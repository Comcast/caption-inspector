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

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#ifdef COMPILE_GPAC
#include "gpac/isomedia.h"
#endif

#include "debug.h"

#include "mov_file.h"
#include "pipeline_utils.h"
#include "buffer_utils.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/
#ifdef COMPILE_GPAC
static char* GpacDebugSectionText[GF_LOG_TOOL_MAX] = {
    "GF_LOG_CORE",       // Log message from the core library (init, threads, network calls, etc)
    "GF_LOG_CODING",     // Log message from a raw media parser (BIFS, LASeR, A/V formats)
    "GF_LOG_CONTAINER",  // Log message from a bitstream parser (IsoMedia, MPEG-2 TS, OGG, ...)
    "GF_LOG_NETWORK",    // Log message from the network/service stack (messages & co)
    "GF_LOG_RTP",        // Log message from the RTP/RTCP stack (TS info) and packet structure & hinting (debug)
    "GF_LOG_AUTHOR",     // Log message from authoring subsystem (file manip, import/export)
    "GF_LOG_SYNC",       // Log message from the sync layer of the terminal
    "GF_LOG_CODEC",      // Log message from a codec
    "GF_LOG_PARSER",     // Log message from any XML parser (context loading, etc)
    "GF_LOG_MEDIA",      // Log message from the terminal/compositor, indicating media object state
    "GF_LOG_SCENE",      // Log message from the scene graph/scene manager (handling of nodes and attribute modif, DOM core)
    "GF_LOG_SCRIPT",     // Log message from the scripting engine APIs - does not cover alert() in the script code itself
    "GF_LOG_INTERACT",   // Log message from event handling
    "GF_LOG_COMPOSE",    // Log message from compositor
    "GF_LOG_CACHE",      // Log for video object cache
    "GF_LOG_MMIO",       // Log message from multimedia I/O devices (audio/video input/output, ...)
    "GF_LOG_RTI",        // Log for runtime info (times, memory, CPU usage)
    "GF_LOG_SMIL",       // Log for SMIL timing and animation
    "GF_LOG_MEMORY",     // Log for memory tracker
    "GF_LOG_AUDIO",      // Log for audio compositor
    "GF_LOG_MODULE",     // Generic Log for modules
    "GF_LOG_MUTEX",      // Log for threads and mutexes
    "GF_LOG_CONDITION",  // Log for threads and condition
    "GF_LOG_DASH",       // Log for all HTTP streaming
    "GF_LOG_CONSOLE",    // Log for all messages coming from GF_Terminal or script alert()
    "GF_LOG_APP",        // Log for all messages coming the application, not used by libgpac or the modules
    "GF_LOG_SCHEDULER"   // Log for all messages coming from the scheduler
};

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

static uint8* ccdpFindData(uint8*, uint32, uint8*);
static void closeMovFile( MovFileCtx* );
static void loggingCallback( void*, GF_LOG_Level, GF_LOG_Tool, const char*, va_list );
#endif

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    MovFileInitialize()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    fileNameStr - The name of the source file.
 |    overrideDropframe - Use passed in dropframe value rather than calculated.
 |    isDropframe - Passed in dropframe value.
 |    bailAfterMins - Whether or not to stop processing at x mins if no text found.
 |
 | RETURN VALUES:
 |    boolean - Was this call successful.
 |
 | DESCRIPTION:
 -------------------------------------------------------------------------------*/
boolean MovFileInitialize( Context* rootCtxPtr, char* fileNameStr, boolean overrideDropframe, boolean isDropframe, uint8 bailAfterMins ) {
#ifdef COMPILE_GPAC
    ASSERT(fileNameStr);
    ASSERT(rootCtxPtr);
    ASSERT(!rootCtxPtr->movFileCtxPtr);

    rootCtxPtr->movFileCtxPtr = malloc(sizeof(MovFileCtx));
    MovFileCtx* ctxPtr = rootCtxPtr->movFileCtxPtr;

    ctxPtr->movFile = NULL;

    gf_log_set_callback(NULL, loggingCallback);

    LOG(DEBUG_LEVEL_INFO, DBG_MOV_FILE, "Opening \'%s\'", fileNameStr);

    ctxPtr->movFile = gf_isom_open(fileNameStr, GF_ISOM_OPEN_READ, NULL);
    if( ctxPtr->movFile == NULL ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_MOV_FILE, "failed to open");
        free(ctxPtr);
        rootCtxPtr->movFileCtxPtr = NULL;
        return FALSE;
    }

    ctxPtr->trackCount = gf_isom_get_track_count(ctxPtr->movFile);
    ctxPtr->currentTrack = 0;
    ctxPtr->currentSample = 0;
    ctxPtr->avcTrackCount = 0;
    ctxPtr->ccTrackCount = 0;
    ctxPtr->framerateOneshot = FALSE;
    ctxPtr->overrideDropframe = overrideDropframe;
    ctxPtr->isDropframe = isDropframe;
    ctxPtr->bailNoCaptions = bailAfterMins;

    for(int loop = 0; loop < ctxPtr->trackCount; loop++) {
        const u32 type = gf_isom_get_media_type(ctxPtr->movFile, loop + 1);
        const u32 subtype = gf_isom_get_media_subtype(ctxPtr->movFile, loop + 1, 1);
        LOG(DEBUG_LEVEL_INFO, DBG_MOV_FILE, "Track %d, type=%c%c%c%c subtype=%c%c%c%c", loop+1, (unsigned char) (type>>24%0x100),
            (unsigned char) ((type>>16)%0x100),(unsigned char) ((type>>8)%0x100),(unsigned char) (type%0x100),
            (unsigned char) (subtype>>24%0x100),
            (unsigned char) ((subtype>>16)%0x100),(unsigned char) ((subtype>>8)%0x100),(unsigned char) (subtype%0x100));
        if ((type == GF_ISOM_MEDIA_CAPTIONS && subtype == GF_ISOM_SUBTYPE_C608) ||
            (type == GF_ISOM_MEDIA_CAPTIONS && subtype == GF_ISOM_SUBTYPE_C708))
            ctxPtr->ccTrackCount++;
        if (type == GF_ISOM_MEDIA_VISUAL && subtype == GF_ISOM_SUBTYPE_AVC_H264)
            ctxPtr->avcTrackCount++;
    }

    LOG(DEBUG_LEVEL_INFO, DBG_MOV_FILE, "MP4: found %u tracks: %u avc and %u cc", ctxPtr->trackCount, ctxPtr->avcTrackCount, ctxPtr->ccTrackCount);

    InitSinks(&ctxPtr->sinks, MPG_FILE___CC_DATA);
#endif
    return TRUE;
}  // MovFileInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    MovFileAddSink()
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
boolean MovFileAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    ASSERT(rootCtxPtr);
    ASSERT(rootCtxPtr->movFileCtxPtr);

#ifdef COMPILE_GPAC
    if( rootCtxPtr->movFileCtxPtr->movFile == NULL ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_MOV_FILE, "Can't Set a Source Connection without Open File");
        return FALSE;
    }
#endif

    if( linkInfo.sourceType != DATA_TYPE_CC_DATA ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_MOV_FILE, "Can't Add Incompatible Sink %d vs. %d", DATA_TYPE_CC_DATA, linkInfo.sourceType);
        return FALSE;
    }

    return AddSink(&rootCtxPtr->movFileCtxPtr->sinks, &linkInfo);
}  // MovFileAddSink()

/*------------------------------------------------------------------------------
 | NAME:
 |    MovFileProcNextBuffer()
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
uint8 MovFileProcNextBuffer( Context* rootCtxPtr, boolean* isDonePtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(rootCtxPtr->movFileCtxPtr);
    boolean retval = TRUE;

#ifdef COMPILE_GPAC
    MovFileCtx* ctxPtr = rootCtxPtr->movFileCtxPtr;

    if( ctxPtr->sinks.numSinks == 0 ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_MOV_FILE, "Can't Process Buffers with no Sinks");
        return FALSE;
    }

    if( ctxPtr->movFile == NULL ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_MOV_FILE, "Can't Process Buffers without an Open File");
        return FALSE;
    }

    if( ctxPtr->currentSample == 0 ) {
        ctxPtr->currentType = gf_isom_get_media_type(ctxPtr->movFile, ctxPtr->currentTrack + 1);
        ctxPtr->currentSubtype = gf_isom_get_media_subtype(ctxPtr->movFile, ctxPtr->currentTrack + 1, 1);
    }

    if ( ctxPtr->currentType == GF_ISOM_MEDIA_VISUAL && ctxPtr->currentSubtype == GF_ISOM_SUBTYPE_XDVB) {
        LOG(DEBUG_LEVEL_INFO, DBG_MOV_FILE, "Found XDVB Track");
    }

    if( ctxPtr->currentType == GF_ISOM_MEDIA_VISUAL && ctxPtr->currentSubtype == GF_ISOM_SUBTYPE_AVC_H264) {
        LOG(DEBUG_LEVEL_INFO, DBG_MOV_FILE, "Found H264 Track");
    }

    if( ctxPtr->currentType == GF_ISOM_MEDIA_CAPTIONS &&
        (ctxPtr->currentSubtype == GF_ISOM_SUBTYPE_C608 || ctxPtr->currentSubtype == GF_ISOM_SUBTYPE_C708) ) {

        if( ctxPtr->currentSample == 0 ) {
            ctxPtr->numSamples = gf_isom_get_sample_count(ctxPtr->movFile, ctxPtr->currentTrack+1);

            ctxPtr->processingStreamDescriptionIndex = 0;
            ctxPtr->timescale = gf_isom_get_media_timescale(ctxPtr->movFile, ctxPtr->currentTrack+1);
        }

        u32 streamDescriptionIndex;
        GF_ISOSample *sample= gf_isom_get_sample(ctxPtr->movFile, ctxPtr->currentTrack+1, ctxPtr->currentSample+1, &streamDescriptionIndex);
        if( ctxPtr->processingStreamDescriptionIndex && ctxPtr->processingStreamDescriptionIndex!=streamDescriptionIndex ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_MOV_FILE, "Multi-Track found.");
            ctxPtr->currentSample = 0;
            ctxPtr->currentTrack = ctxPtr->currentTrack + 1;
            if( ctxPtr->currentTrack >= ctxPtr->trackCount ) {
                ctxPtr->currentTrack = 0;
                *isDonePtr = TRUE;
                closeMovFile(ctxPtr);
                free(ctxPtr);
                rootCtxPtr->movFileCtxPtr = NULL;
            }
            return retval;
        }

        if( !ctxPtr->processingStreamDescriptionIndex ) {
            ctxPtr->processingStreamDescriptionIndex = streamDescriptionIndex;
        }

        if( sample != NULL ) {
            uint64 pts = (sample->DTS + sample->CTS_Offset)*1000/ctxPtr->timescale;

            uint32 tmpFramerate;

            if( ctxPtr->timescale > 10000 ) {
                tmpFramerate = ctxPtr->timescale / 10;
            } else {
                tmpFramerate = ctxPtr->timescale;
            }
            if( ctxPtr->framerateOneshot == FALSE ) {
                LOG(DEBUG_LEVEL_INFO, DBG_MOV_FILE, "Framerate = %d.%d", tmpFramerate / 100, tmpFramerate % 100);
                ctxPtr->framerateOneshot = TRUE;
            }

            int atomStart = 0;
            while( atomStart < sample->dataLength ) {

                char *data = sample->data + atomStart;
                unsigned int atomLength = RB32(data);

                if( atomLength < 8 || atomLength > sample->dataLength ) {
                    LOG(DEBUG_LEVEL_ERROR, DBG_MOV_FILE, "Invalid atom length. %d vs. actual - %d", atomLength, sample->dataLength);
                    break;
                }

                data += 4;

                if( !strncmp(data, "cdat", 4) || !strncmp(data, "cdt2", 4) || !strncmp(data, "ccdp", 4) ) {
                    if( ctxPtr->currentSubtype == GF_ISOM_SUBTYPE_C708 ) {
                        uint8 ccCount;
                        data += 4;
                        uint8* ccData = ccdpFindData( (uint8*)data, sample->dataLength - 8, &ccCount );

                        if( !ccData ) {
                            LOG(DEBUG_LEVEL_ERROR, DBG_MOV_FILE, "No CC Data found in CCDP");
                            break;
                        }

                        Buffer* outputBuffer = NewBuffer(BUFFER_TYPE_BYTES, ccCount*3);
                        if( ctxPtr->timescale > 10000 ) {
                            outputBuffer->captionTime.frameRatePerSecTimesOneHundred = ctxPtr->timescale / 10;
                        } else {
                            outputBuffer->captionTime.frameRatePerSecTimesOneHundred = ctxPtr->timescale;
                        }
                        if( ctxPtr->overrideDropframe == FALSE ) {
                            if ((outputBuffer->captionTime.frameRatePerSecTimesOneHundred / 100 == 29) ||
                                (outputBuffer->captionTime.frameRatePerSecTimesOneHundred / 100 == 59)) {
                                outputBuffer->captionTime.dropframe = TRUE;
                            } else {
                                outputBuffer->captionTime.dropframe = FALSE;
                            }
                        } else {
                            outputBuffer->captionTime.dropframe = ctxPtr->isDropframe;
                        }
                        CaptionTimeFromPts(&outputBuffer->captionTime, pts);

                        for( int loop = 0; loop < ccCount; loop++, ccData += 3 ) {
                            outputBuffer->dataPtr[outputBuffer->numElements] = ccData[0];
                            outputBuffer->dataPtr[outputBuffer->numElements+1] = ccData[1];
                            outputBuffer->dataPtr[outputBuffer->numElements+2] = ccData[2];
                            outputBuffer->numElements = outputBuffer->numElements + 3;
                            ASSERT(outputBuffer->numElements <= outputBuffer->maxNumElements);

                            if( ccData[0] == CDP_SECTION_SVC_INFO || ccData[0] == CDP_SECTION_FOOTER ) {
                                LOG(DEBUG_LEVEL_ERROR, DBG_MOV_FILE, "Premature End");
                                break;
                            }
                        }
                        ASSERT(outputBuffer->numElements == outputBuffer->maxNumElements);
                        retval = PassToSinks(rootCtxPtr, outputBuffer, &ctxPtr->sinks);
                        atomStart = sample->dataLength;
                        if( retval == FIRST_TEXT_FOUND ) {
                            if( ctxPtr->bailNoCaptions != 0 ) {
                               ctxPtr->bailNoCaptions = 0;
                            }
                            retval = PIPELINE_SUCCESS;
                        }
                    } else {
                        int ret = 0;
                        int len = atomLength - 8;
                        data += 4;
                        char *tdata = data;
                        do {
                            LOG(DEBUG_LEVEL_ERROR, DBG_MOV_FILE, "Found 608 Subtype: 0x%02X 0x%02X", tdata[0], tdata[1]);
                            len -= ret;
                            tdata += ret;
                        } while (len > 0);
                        LOG(DEBUG_LEVEL_FATAL, DBG_MOV_FILE, "Currently there is no code to process the 608 subtype, because it would take some work and looks to be unnecessary and unused");
                    }
                }
                atomStart += atomLength;
            }
            free(sample->data);
            free(sample);

            if( ctxPtr->bailNoCaptions != 0 ) {
                CaptionTime captionTime;
                CaptionTimeFromPts(&captionTime, pts);
                if (captionTime.minute >= ctxPtr->bailNoCaptions) {
                    LOG(DEBUG_LEVEL_WARN, DBG_MOV_FILE, "Unable to find Captions after %d mins. Abandoning.", captionTime.minute);
                    *isDonePtr = TRUE;
                    closeMovFile(ctxPtr);
                }
            }
        }

        ctxPtr->currentSample = ctxPtr->currentSample + 1;
        if( ctxPtr->currentSample >= ctxPtr->numSamples ) {
            ctxPtr->currentSample = 0;
        }
    }

    if( ctxPtr->currentSample == 0 ) {
        ctxPtr->currentTrack = ctxPtr->currentTrack + 1;
        if( ctxPtr->currentTrack >= ctxPtr->trackCount ) {
            ctxPtr->currentTrack = 0;
            *isDonePtr = TRUE;
            closeMovFile(ctxPtr);
        }
    }

    if( *isDonePtr == TRUE ) {
        Sinks sinks = ctxPtr->sinks;
        free(ctxPtr);
        rootCtxPtr->movFileCtxPtr = NULL;
        retval = retval & ShutdownSinks(rootCtxPtr, &sinks);
    }
#endif

    return retval;
} // MovFileProcNextBuffer()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

#ifdef COMPILE_GPAC
/*------------------------------------------------------------------------------
 | NAME:
 |    ccdpFindData()
 |
 | DESCRIPTION:
 |    This function extracts the CC Data from CCDP.
 -------------------------------------------------------------------------------*/
static uint8* ccdpFindData(uint8* ccdpAtomContent, uint32 len, uint8* ccCount) {
    uint8* data = ccdpAtomContent;

    if( len < 4 ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MOV_FILE, "Unexpected cdp size 4 vs. %d", len);
        return NULL;
    }

    uint16 cdp_id = (data[0] << 8) | data[1];
    if( cdp_id != 0x9669 ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MOV_FILE, "Unexpected Header: %02X %02X", data[0], data[1]);
        return NULL;
    }

    data = data + 2;
    len = len - 2;

    uint8 cdp_data_count = data[0];
    if( cdp_data_count != len + 2 ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MOV_FILE, "Unexpected data length %d vs. %d", cdp_data_count, len + 2);
        return NULL;
    }

    data = data + 2;
    len = len - 2;

    uint8 cdp_flags = data[0];

    data = data + 3;
    len = len - 3;

    uint8 cdp_timecode_added = (cdp_flags & 0x80) >> 7;
    uint8 cdp_data_added = (cdp_flags & 0x40) >> 6;

    if( cdp_data_added == FALSE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MOV_FILE, "No Data in Packet");
        return NULL;
    }

    if( cdp_timecode_added == TRUE ) {
        data = data + 4;
        len = len - 4;
    }

    if( data[0] != CDP_SECTION_DATA ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MOV_FILE, "cdp_data_section byte not found");
        return NULL;
    }

    *ccCount = (uint8)(data[1] & 0x1F);

    if( (*ccCount != 10) && (*ccCount != 20) && (*ccCount != 25) && (*ccCount != 30) ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MOV_FILE, "Unexpected cc_count %u", *ccCount);
        return NULL;
    }

    data = data + 2;
    len = len - 2;

    if( (*ccCount) * 3 > len ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MOV_FILE, "Not enough bytes left (%d) in buffer to carry %d*3 bytes", len, *ccCount);
        return NULL;
    }

    return data;
} // ccdpFindData()

/*------------------------------------------------------------------------------
 | NAME:
 |    closeMovFile()
 |
 | DESCRIPTION:
 |    This function closes an open media file.
 -------------------------------------------------------------------------------*/
static void closeMovFile( MovFileCtx* ctxPtr ) {
    LOG(DEBUG_LEVEL_INFO, DBG_MOV_FILE, "Closing media.");

    gf_isom_close(ctxPtr->movFile);
    ctxPtr->movFile = NULL;

    if( ctxPtr->avcTrackCount == 0 ) {
        LOG(DEBUG_LEVEL_INFO, DBG_MOV_FILE, "Found no AVC track(s). ", ctxPtr->movFile);
    } else {
        LOG(DEBUG_LEVEL_INFO, DBG_MOV_FILE, "Found %d AVC track(s). ", ctxPtr->avcTrackCount);
    }
    if( ctxPtr->ccTrackCount )
        LOG(DEBUG_LEVEL_INFO, DBG_MOV_FILE, "Found %d CC track(s).", ctxPtr->ccTrackCount);
    else
        LOG(DEBUG_LEVEL_INFO, DBG_MOV_FILE, "Found no dedicated CC track(s).");
} // closeMovFile()

/*------------------------------------------------------------------------------
 | NAME:
 |    LoggingCallback()
 |
 | DESCRIPTION:
 |    This function provides a callback from GPAC MP4 into the CTTP Logging
 |    Mechanism.
 -------------------------------------------------------------------------------*/
static void loggingCallback( void *cbck, GF_LOG_Level log_level, GF_LOG_Tool log_tool, const char* fmt, va_list vlist ) {
    uint8 dbgLevel;
    char message[1024];

    switch( log_level ) {
        case GF_LOG_ERROR:
            dbgLevel = DEBUG_LEVEL_ERROR;
            break;
        case GF_LOG_WARNING:
            dbgLevel = DEBUG_LEVEL_WARN;
            break;
        case GF_LOG_INFO:
            dbgLevel = DEBUG_LEVEL_INFO;
            break;
        case GF_LOG_DEBUG:
        default:
            dbgLevel = DEBUG_LEVEL_VERBOSE;
    }

    vsprintf(message, fmt, vlist);

    if( log_tool < GF_LOG_TOOL_MAX ) {
        DebugLog(dbgLevel, DBG_GPAC_MP4, GpacDebugSectionText[log_tool], 0, message);
    } else {
        DebugLog(dbgLevel, DBG_GPAC_MP4, "GPAC", 0, message);
    }
} // loggingCallback()
#endif
