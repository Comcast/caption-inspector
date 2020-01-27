/**
 * Copyright 2019 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "debug.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "mpeg_file.h"
#include "pipeline_utils.h"
#include "buffer_utils.h"
#include "cc_utils.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

#ifndef DONT_COMPILE_FFMPEG
static void loggingCallback( void*, int, const char*, va_list );
#endif

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    MpegFileInitialize()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    overrideDropframe - Use passed in dropframe value rather than calculated.
 |    isDropframe - Passed in dropframe value.
 |    bailAfterMins - Whether or not to stop processing at x mins if no text found.
 |
 | RETURN VALUES:
 |    boolean - Was this call successful.
 |
 | DESCRIPTION:
 -------------------------------------------------------------------------------*/
boolean MpegFileInitialize( Context* rootCtxPtr, uint8 bailAfterMins ) {
#ifndef DONT_COMPILE_FFMPEG
    ASSERT(rootCtxPtr);
    ASSERT(rootCtxPtr->config.inputFilename);
    ASSERT(!rootCtxPtr->mpegFileCtxPtr);

    int ret = 0;
    int stream_index = 0;
    AVCodec *dec = NULL;

    rootCtxPtr->mpegFileCtxPtr = malloc(sizeof(MpegFileCtx));
    MpegFileCtx* ctxPtr = rootCtxPtr->mpegFileCtxPtr;

    ctxPtr->firstPts = 0;
    ctxPtr->ccCountMismatchErrors = 0;

    ctxPtr->fileSize = 0;
    ctxPtr->isFileOpen = FALSE;

    if( rootCtxPtr->config.forceDropframe == TRUE ) {
        ctxPtr->isDropframe = rootCtxPtr->config.forcedDropframe;
    } else {
        ctxPtr->isDropframe = DetermineDropFrame(rootCtxPtr->config.inputFilename, rootCtxPtr->config.artifacts, rootCtxPtr->config.outputDirectory);
    }

    ctxPtr->bailNoCaptions = bailAfterMins;

    int fdesc = open(rootCtxPtr->config.inputFilename, O_RDONLY);

    if( fdesc == -1 ) {
        char filePath[PATH_MAX];
        filePath[0] = '\0';
        getcwd(filePath, sizeof(filePath));
        switch (errno)
        {
            case ENOENT:
                LOG(DEBUG_LEVEL_FATAL, DBG_MPEG_FILE, "EXIT_NO_INPUT_FILES --- Failed to open file: File does not exist. --- Looking Here: %s", filePath);
            case EACCES:
                LOG(DEBUG_LEVEL_FATAL, DBG_MPEG_FILE, "EXIT_READ_ERROR --- Failed to open file: Unable to access. --- Looking Here: %s", filePath);
            case EINVAL:
                LOG(DEBUG_LEVEL_FATAL, DBG_MPEG_FILE, "EXIT_READ_ERROR --- Failed to open file: Invalid opening flag. --- Looking Here: %s", filePath);
            case EMFILE:
                LOG(DEBUG_LEVEL_FATAL, DBG_MPEG_FILE, "EXIT_NO_INPUT_FILES --- Failed to open file: File does not exist. --- Looking Here: %s", filePath);
            default:
                LOG(DEBUG_LEVEL_FATAL, DBG_MPEG_FILE, "EXIT_READ_ERROR --- Failed to open file: Reason unknown. --- Looking Here: %s", filePath);
        }
        free(ctxPtr);
        rootCtxPtr->mpegFileCtxPtr = NULL;
        return FALSE;
    }

    int64 current = lseek( fdesc, 0, SEEK_CUR ) ;
    int64 length = lseek( fdesc, 0, SEEK_END );
    if( current < 0 || length < 0 ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_MPEG_FILE, "Error in File Seek %d %d", current, length );
    }

    if( lseek(fdesc, current, SEEK_SET) < 0 ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_MPEG_FILE, "Error in File Seek Set" );
    }

    ctxPtr->fileSize = length;
    close(fdesc);

    avcodec_register_all();
    av_register_all();

    switch( GetMinDebugLevel(DBG_FF_MPEG) ) {
        case DEBUG_LEVEL_FATAL:
            av_log_set_level(AV_LOG_FATAL);
            break;
        case DEBUG_LEVEL_ERROR:
            av_log_set_level(AV_LOG_ERROR);
            break;
        case DEBUG_LEVEL_WARN:
            av_log_set_level(AV_LOG_WARNING);
            break;
        case DEBUG_LEVEL_INFO:
            av_log_set_level(AV_LOG_INFO);
            break;
        case DEBUG_LEVEL_VERBOSE:
        default:
            av_log_set_level(AV_LOG_DEBUG);
    }

    av_log_set_callback(loggingCallback);

    /**
     * Initialize decoder according to the name of input
     */
    ret = avformat_open_input(&ctxPtr->formatContext, rootCtxPtr->config.inputFilename, NULL, NULL);
    if( ret < 0 ) {
        char filePath[PATH_MAX];
        filePath[0] = '\0';
        getcwd(filePath, sizeof(filePath));
        av_log(NULL,AV_LOG_ERROR,"could not open input(%s) format. Looking Here: %s\n", rootCtxPtr->config.inputFilename, filePath);
        free(ctxPtr);
        rootCtxPtr->mpegFileCtxPtr = NULL;
        return FALSE;
    }

    ret = avformat_find_stream_info(ctxPtr->formatContext,NULL);
    if( ret < 0 ) {
        av_log(NULL,AV_LOG_ERROR,"could not find any stream\n");
        free(ctxPtr);
        rootCtxPtr->mpegFileCtxPtr = NULL;
        return FALSE;
    }

    /* first search in strean if not found search the video stream */
    ret = av_find_best_stream(ctxPtr->formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if( ret < 0 ) {
        av_log(NULL, AV_LOG_ERROR, "no suitable subtitle or caption\n");
        free(ctxPtr);
        rootCtxPtr->mpegFileCtxPtr = NULL;
        return FALSE;
    }

    stream_index = ret;
    ctxPtr->decoderContext = ctxPtr->formatContext->streams[stream_index]->codec;
    ctxPtr->streamIndex = stream_index;
    ret = avcodec_open2(ctxPtr->decoderContext, dec, NULL);
    if( ret < 0 ) {
        av_log(NULL,AV_LOG_ERROR,"unable to open codec\n");
        free(ctxPtr);
        rootCtxPtr->mpegFileCtxPtr = NULL;
        return FALSE;
    }

    //Initialize frame where input frame will be stored
    ctxPtr->frame = av_frame_alloc();

    AVRational retval = av_guess_frame_rate(ctxPtr->formatContext, ctxPtr->formatContext->streams[stream_index], ctxPtr->frame);
    ctxPtr->frameRatePerSecTimesOneHundred = ((retval.num * 100)/retval.den);

    LOG(DEBUG_LEVEL_INFO, DBG_MPEG_FILE, "Framerate = %d/%d - %d.%d", retval.num, retval.den, ctxPtr->frameRatePerSecTimesOneHundred / 100, ctxPtr->frameRatePerSecTimesOneHundred % 100 );
    if( rootCtxPtr->config.forceDropframe == TRUE ) {
        if( rootCtxPtr->config.forcedDropframe == TRUE ) {
            ASSERT(ctxPtr->isDropframe);
            LOG(DEBUG_LEVEL_INFO, DBG_MPEG_FILE, "Dropframe forced to TRUE");
            if( (ctxPtr->frameRatePerSecTimesOneHundred / 100 != 29) && (ctxPtr->frameRatePerSecTimesOneHundred / 100 != 30) && (ctxPtr->frameRatePerSecTimesOneHundred / 100 != 59) && (ctxPtr->frameRatePerSecTimesOneHundred / 100 != 60) ) {
                LOG(DEBUG_LEVEL_ERROR, DBG_MPEG_FILE, "Likely invalid combination of Forced Dropframe and Framerate");
            }
        } else {
            ASSERT(!ctxPtr->isDropframe);
            LOG(DEBUG_LEVEL_INFO, DBG_MPEG_FILE, "Dropframe forced to FALSE");
        }
    } else {
        if( ctxPtr->isDropframe ) {
            LOG(DEBUG_LEVEL_INFO, DBG_MPEG_FILE, "Dropframe = TRUE");
        } else {
            LOG(DEBUG_LEVEL_INFO, DBG_MPEG_FILE, "Dropframe = FALSE");
        }
    }

    ctxPtr->isFileOpen = TRUE;

    InitSinks(&ctxPtr->sinks, MPG_FILE___CC_DATA);

    return TRUE;
#else
    LOG(DEBUG_LEVEL_FATAL, DBG_MPEG_FILE,"Impossible Branch.");
    return TRUE;
#endif
}  // MpegFileInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    MpegFileAddSink()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    linkInfo - Information of the next element in the pipeline.
 |
 | RETURN VALUES:
 |    boolean - TRUE is Successful and FALSE is a Failure
 |
 | DESCRIPTION:
 |    This adds another sink in the pipeline after this element.
 -------------------------------------------------------------------------------*/
boolean MpegFileAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
#ifndef DONT_COMPILE_FFMPEG
    ASSERT(rootCtxPtr);
    ASSERT(rootCtxPtr->mpegFileCtxPtr);

    if( rootCtxPtr->mpegFileCtxPtr->isFileOpen == FALSE ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_MPEG_FILE, "Can't Set a Source Connection without Open File");
        return FALSE;
    }
    
    if( linkInfo.sourceType != DATA_TYPE_CC_DATA ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_MPEG_FILE, "Can't Add Incompatible Sink %d vs. %d", DATA_TYPE_CC_DATA, linkInfo.sourceType);
        return FALSE;
    }
    
    return AddSink(&rootCtxPtr->mpegFileCtxPtr->sinks, &linkInfo);
#else
    LOG(DEBUG_LEVEL_FATAL, DBG_MPEG_FILE,"Impossible Branch.");
    return TRUE;
#endif
}  // MpegFileAddSink()

/*------------------------------------------------------------------------------
 | NAME:
 |    MpegFileProcNextBuffer()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |
 | RETURN VALUES:
 |    isDonePtr - Whether or not the file has been completely read.
 |    uint8 - Success is TRUE / PIPELINE_SUCCESS, Failure is FALSE / PIPELINE_FAILURE
 |            All other codes specified in header.
 |
 | DESCRIPTION:
 |    This method creates the next buffer of data and passes it down the pipeline.
 -------------------------------------------------------------------------------*/
uint8 MpegFileProcNextBuffer( Context* rootCtxPtr, boolean* isDonePtr ) {
#ifndef DONT_COMPILE_FFMPEG
    ASSERT(rootCtxPtr);
    ASSERT(rootCtxPtr->mpegFileCtxPtr);

    MpegFileCtx* ctxPtr = rootCtxPtr->mpegFileCtxPtr;

    if( ctxPtr->sinks.numSinks == 0 ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_MPEG_FILE, "Can't Process Buffers with no Sinks");
        return FALSE;
    }
    
    while( TRUE ) {
        int retval = 0;
        int got_frame;
        AVPacket packet;
        int64 pts = 0;

        ctxPtr->len = 0;
        
        retval = av_read_frame(ctxPtr->formatContext, &packet);
        if( retval == AVERROR_EOF ) {
            *isDonePtr = TRUE;
            Sinks sinks = ctxPtr->sinks;
            free(ctxPtr);
            rootCtxPtr->mpegFileCtxPtr = NULL;
            return ShutdownSinks(rootCtxPtr, &sinks);
        } else if( retval < 0 ) {
            av_log(NULL, AV_LOG_ERROR, "not able to read the packet\n");
            return FALSE;
        } else if( packet.stream_index != ctxPtr->streamIndex ) {
            continue;
        }
        
        retval = avcodec_decode_video2( ctxPtr->decoderContext, ctxPtr->frame, &got_frame, &packet );
        if( ctxPtr->firstPts == 0 ) {
// TODO - Need to account for rollover
            ctxPtr->firstPts = packet.pts;
        }

        if( retval < 0 ) {
            av_log(NULL,AV_LOG_ERROR,"unable to decode packet\n");
            return FALSE;
        } else if( !got_frame ) {
            continue;
        }
        
        for( int i = 0; i < ctxPtr->frame->nb_side_data; i++ ) {
            if(ctxPtr->frame->side_data[i]->type == AV_FRAME_DATA_A53_CC) {
                ctxPtr->frame->pts = av_frame_get_best_effort_timestamp(ctxPtr->frame);

                pts = (((ctxPtr->frame->pts - ctxPtr->firstPts) * ctxPtr->formatContext->streams[ctxPtr->streamIndex]->time_base.num)) /
                       (ctxPtr->formatContext->streams[ctxPtr->streamIndex]->time_base.den / 1000);

                if(ctxPtr->frame->side_data[i]->size > BUFSIZE) {
                    av_log(NULL,AV_LOG_ERROR,"Please consider increasing length of data\n");
                } else {
                    memcpy(ctxPtr->buffer, ctxPtr->frame->side_data[i]->data,
                           ctxPtr->frame->side_data[i]->size);
                    ctxPtr->len = ctxPtr->frame->side_data[i]->size;
                }
            }
        }

        if( ctxPtr->bailNoCaptions != 0 ) {
            CaptionTime captionTime;
            CaptionTimeFromPts(&captionTime, pts);
            if (captionTime.minute >= ctxPtr->bailNoCaptions) {
                LOG(DEBUG_LEVEL_WARN, DBG_MPEG_FILE, "Unable to find Captions after %d mins. Abandoning.", captionTime.minute);
                *isDonePtr = TRUE;
                Sinks sinks = ctxPtr->sinks;
                free(ctxPtr);
                rootCtxPtr->mpegFileCtxPtr = NULL;
                return ShutdownSinks(rootCtxPtr, &sinks);
            }
        }

        if( ctxPtr->len != 0 ) {
            ASSERT(!(ctxPtr->len % 3));
            uint8 ccCount = numCcConstructsFromFramerate(ctxPtr->frameRatePerSecTimesOneHundred);
            if( ccCount != (ctxPtr->len / 3) ) {
                ctxPtr->ccCountMismatchErrors++;
                if( ctxPtr->ccCountMismatchErrors < 5) {
                    LOG(DEBUG_LEVEL_WARN, DBG_MPEG_FILE, "Mismatch in CC Count Expected: %d vs Actual: %d", ccCount, (ctxPtr->len / 3));
                } else if( ctxPtr->ccCountMismatchErrors == 5) {
                    LOG(DEBUG_LEVEL_WARN, DBG_MPEG_FILE, "Mismatch in CC Count Expected: %d vs Actual: %d. Suppressing Subsequent Error Messages.", ccCount, (ctxPtr->len / 3));
                }
            }

            Buffer* outputBuffer = NewBuffer(BUFFER_TYPE_BYTES, ctxPtr->len);
            outputBuffer->captionTime.frameRatePerSecTimesOneHundred = ctxPtr->frameRatePerSecTimesOneHundred;
            outputBuffer->captionTime.dropframe = ctxPtr->isDropframe;
            CaptionTimeFromPts(&outputBuffer->captionTime, pts);
            outputBuffer->numElements = outputBuffer->maxNumElements;
            memcpy(outputBuffer->dataPtr, ctxPtr->buffer, ctxPtr->len);

            uint8 returnval = PassToSinks(rootCtxPtr, outputBuffer, &ctxPtr->sinks);
            if( returnval == FIRST_TEXT_FOUND ) {
                if( ctxPtr->bailNoCaptions != 0 ) {
                    ctxPtr->bailNoCaptions = 0;
                }
                returnval = PIPELINE_SUCCESS;
            }
            return returnval;
        }
    }
#else
    LOG(DEBUG_LEVEL_FATAL, DBG_MPEG_FILE,"Impossible Branch.");
    return TRUE;
#endif
} // MpegFileProcNextBuffer()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

#ifndef DONT_COMPILE_FFMPEG
/*------------------------------------------------------------------------------
 | NAME:
 |    LoggingCallback()
 |
 | DESCRIPTION:
 |    This function provides a callback from FFMPEG into the CTTP Logging
 |    Mechanism.
 -------------------------------------------------------------------------------*/
static void loggingCallback( void* ptr, int level, const char* fmt, va_list vl ) {
    uint8 dbgLevel;
    char message[1024];
    
    if( level > av_log_get_level() )
        return;
    
    switch( level ) {
        case AV_LOG_PANIC:
        case AV_LOG_FATAL:
            dbgLevel = DEBUG_LEVEL_FATAL;
            break;
        case AV_LOG_ERROR:
            dbgLevel = DEBUG_LEVEL_ERROR;
            break;
        case AV_LOG_WARNING:
            dbgLevel = DEBUG_LEVEL_WARN;
            break;
        case AV_LOG_INFO:
            dbgLevel = DEBUG_LEVEL_INFO;
            break;
        case AV_LOG_DEBUG:
        default:
            dbgLevel = DEBUG_LEVEL_VERBOSE;
    }

    vsprintf(message, fmt, vl);

    DebugLog( dbgLevel, DBG_FF_MPEG, "FFMPEG", 0, message );
}  // loggingCallback()
#endif
