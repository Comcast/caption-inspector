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

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <uuid/uuid.h>
#include "debug.h"

#include "mcc_encode.h"
#include "cc_utils.h"

//#define DEBUG_CAPTION_TIMING

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

const char* MonthStr[12] = { "January", "February", "March", "April",
                             "May", "June", "July", "August", "September",
                             "October", "November", "December" };

const char* DayOfWeekStr[7] = { "Sunday", "Monday", "Tuesday", "Wednesday",
                                "Thursday", "Friday", "Saturday" };

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

static boolean generateMccHeader( Context*, CaptionTime* );
static void addFillPacket( Context*, CaptionTime* );
static uint8 handleSkew( Context*, CaptionTime* );
static CaptionTime convertCaptionTime( Context*, Buffer* );
static boolean sendMccText( Context*, char*, CaptionTime* );
static Buffer* addBoilerplate( MccEncodeCtx*, Buffer* );
static uint16 countChars( uint8*, uint16 );
static void compressData( uint8*, uint16, Buffer* );

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    MccEncodeInitialize()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |
 | RETURN VALUES:
 |    LinkInfo -  Information about this element of the pipeline, such that it can
 |                be chained to elements that can consume the specific type of data
 |                that it produces.
 |
 | DESCRIPTION:
 |    This initializes this element of the pipeline.
 -------------------------------------------------------------------------------*/
LinkInfo MccEncodeInitialize( Context* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(!rootCtxPtr->mccEncodeCtxPtr);

    rootCtxPtr->mccEncodeCtxPtr = malloc(sizeof(MccEncodeCtx));
    MccEncodeCtx* ctxPtr = rootCtxPtr->mccEncodeCtxPtr;

    if( rootCtxPtr->config.matchPtsTime == TRUE ) {
        LOG(DEBUG_LEVEL_INFO, DBG_MCC_ENC, "Configured to Match PTS Time to MCC Time");
    }

    ctxPtr->matchPtsTime = rootCtxPtr->config.matchPtsTime;
    ctxPtr->headerPrinted = FALSE;
    ctxPtr->cdpHeaderSequence = 0;
    ctxPtr->numFillFrames = 0;
    ctxPtr->maxPositiveDelta = 0;
    ctxPtr->maxNegativeDelta = 0;
    ctxPtr->fullSecondFound = FALSE;
    ctxPtr->totalSkew = 0;

    for( int loop = 0; loop < 60; loop++ ) {
        ctxPtr->framesPerSec[loop] = 0;
    }

    InitSinks(&ctxPtr->sinks, CC_DATA___MCC_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___MCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_MCC_DATA;
    linkInfo.NextBufferFnPtr = &MccEncodeProcNextBuffer;
    linkInfo.ShutdownFnPtr = &MccEncodeShutdown;
    return linkInfo;
}  // MccEncodeInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    MccEncodeAddSink()
 |
 | INPUT PARAMETERS:
 |    ctxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    linkInfo - Describes the potential next element of the pipeline.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method binds the next element to this element, after validating that
 |    the data it receives is compatible with the data tht this element sends.
 -------------------------------------------------------------------------------*/
boolean MccEncodeAddSink( Context* ctxPtr, LinkInfo linkInfo ) {
    ASSERT(ctxPtr);
    ASSERT(ctxPtr->mccEncodeCtxPtr);

    if( linkInfo.sourceType != DATA_TYPE_MCC_DATA ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_ENC, "Attempt to add Sink that cannot process MCC Data: %d vs. %d", linkInfo.sourceType, DATA_TYPE_MCC_DATA);
        return FALSE;
    }
    
    return AddSink(&ctxPtr->mccEncodeCtxPtr->sinks, &linkInfo);
}  // MccEncodeAddSink()

/*------------------------------------------------------------------------------
 | NAME:
 |    MccEncodeProcNextBuffer()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    inBuffer - Pointer to the buffer to process.
 |
 | RETURN VALUES:
 |    uint8 - Success is TRUE / PIPELINE_SUCCESS, Failure is FALSE / PIPELINE_FAILURE
 |            All other codes specified in header.
 |
 | DESCRIPTION:
 |    This method processes an incoming buffer, encoding line 21 byte pairs
 |    into CC Data.
 -------------------------------------------------------------------------------*/
uint8 MccEncodeProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    ASSERT(inBuffer);
    ASSERT(inBuffer->dataPtr);
    ASSERT(inBuffer->numElements);
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->mccEncodeCtxPtr);
    MccEncodeCtx* ctxPtr = ((Context*)rootCtxPtr)->mccEncodeCtxPtr;

    if( ctxPtr->headerPrinted == FALSE ) {
        LOG(DEBUG_LEVEL_VERBOSE, DBG_MCC_ENC, "Sending MCC Header.");
        if( generateMccHeader(rootCtxPtr, &inBuffer->captionTime) == FALSE ) {
            return FALSE;
        }
        ctxPtr->headerPrinted = TRUE;
        ctxPtr->nextCaptionTime.hour = inBuffer->captionTime.hour;
        ctxPtr->nextCaptionTime.minute = inBuffer->captionTime.minute;
        ctxPtr->nextCaptionTime.second = inBuffer->captionTime.second;

        ctxPtr->lastCaptionTime.hour = inBuffer->captionTime.hour;
        ctxPtr->lastCaptionTime.minute = inBuffer->captionTime.minute;
        ctxPtr->lastCaptionTime.second = inBuffer->captionTime.second;

        uint64 frameNumber = inBuffer->captionTime.millisecond * inBuffer->captionTime.frameRatePerSecTimesOneHundred;
        frameNumber = frameNumber / 100000;
        if( frameNumber > (inBuffer->captionTime.frameRatePerSecTimesOneHundred / 100) ) {
            LOG(DEBUG_LEVEL_WARN, DBG_MCC_ENC, "Normalized Frame Number %d to %d", frameNumber, (inBuffer->captionTime.frameRatePerSecTimesOneHundred / 100));
            frameNumber = inBuffer->captionTime.frameRatePerSecTimesOneHundred / 100;
        }
        ctxPtr->nextCaptionTime.frame = frameNumber;
        LOG(DEBUG_LEVEL_INFO, DBG_MCC_ENC, "Captions start at: %02d:%02d:%02d:%02d / %02d:%02d:%02d,%03d",
            inBuffer->captionTime.hour, inBuffer->captionTime.minute, inBuffer->captionTime.second, frameNumber,
            inBuffer->captionTime.hour, inBuffer->captionTime.minute, inBuffer->captionTime.second, inBuffer->captionTime.millisecond);
    }

    LOG(DEBUG_LEVEL_VERBOSE, DBG_MCC_ENC, "MCC Encoding %d byte packet at time: %02d:%02d:%02d;%02d",
        inBuffer->numElements, inBuffer->captionTime.hour, inBuffer->captionTime.minute,
        inBuffer->captionTime.second, inBuffer->captionTime.frame);
    Buffer* withBoilerPlateBuffer = addBoilerplate(ctxPtr, inBuffer);
    if( inBuffer->captionTime.source == CAPTION_TIME_PTS_NUMBERING ) {
        withBoilerPlateBuffer->captionTime = convertCaptionTime(rootCtxPtr, inBuffer);
    } else {
        withBoilerPlateBuffer->captionTime = inBuffer->captionTime;
    }
    uint16 numCharsNeeded = countChars( withBoilerPlateBuffer->dataPtr, withBoilerPlateBuffer->numElements );
    LOG(DEBUG_LEVEL_VERBOSE, DBG_MCC_ENC, "With CDP Boiler Plate %d byte packet at time: %02d:%02d:%02d;%02d requires %d bytes compressed",
        withBoilerPlateBuffer->numElements, withBoilerPlateBuffer->captionTime.hour, withBoilerPlateBuffer->captionTime.minute,
        withBoilerPlateBuffer->captionTime.second, withBoilerPlateBuffer->captionTime.frame, numCharsNeeded);
    Buffer* outputBuffer = NewBuffer(BUFFER_TYPE_BYTES, (numCharsNeeded + 13));
    outputBuffer->captionTime = withBoilerPlateBuffer->captionTime;
    sprintf((char*)outputBuffer->dataPtr, "%02d:%02d:%02d:%02d\t", withBoilerPlateBuffer->captionTime.hour,
            withBoilerPlateBuffer->captionTime.minute, withBoilerPlateBuffer->captionTime.second,
            withBoilerPlateBuffer->captionTime.frame);
    outputBuffer->numElements = 13;
    compressData( withBoilerPlateBuffer->dataPtr, withBoilerPlateBuffer->numElements, outputBuffer );
    FreeBuffer(withBoilerPlateBuffer);
    LOG(DEBUG_LEVEL_VERBOSE, DBG_MCC_ENC, "Sending Compressed %d byte packet at time: %02d:%02d:%02d;%02d",
        outputBuffer->numElements, outputBuffer->captionTime.hour, outputBuffer->captionTime.minute,
        outputBuffer->captionTime.second, outputBuffer->captionTime.frame);
    
    return PassToSinks(rootCtxPtr, outputBuffer, &ctxPtr->sinks);
}  // MccEncodeProcNextBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    MccEncodeShutdown()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |
 | RETURN VALUES:
 |    uint8 - Success is TRUE / PIPELINE_SUCCESS, Failure is FALSE / PIPELINE_FAILURE
 |            All other codes specified in header.
 |
 | DESCRIPTION:
 |    This method is called when the previous element in the pipeline determines
 |    that there is no more data coming down the pipline. This element will
 |    perform any necessary actions as a result and pass this call down the
 |    pipeline.
 -------------------------------------------------------------------------------*/
uint8 MccEncodeShutdown( void* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->mccEncodeCtxPtr);
    Sinks sinks = ((Context*)rootCtxPtr)->mccEncodeCtxPtr->sinks;
    char scratchBuffer[256];
    scratchBuffer[0] = '\0';

    for( int loop = 0; loop < 60; loop++ ) {
        if( ((Context*)rootCtxPtr)->mccEncodeCtxPtr->framesPerSec[loop] == 0 ) {
            break;
        }
        sprintf(&scratchBuffer[strlen(scratchBuffer)], "%d  ", ((Context*)rootCtxPtr)->mccEncodeCtxPtr->framesPerSec[loop]);
        if(loop == 29 ) {
            sprintf(&scratchBuffer[strlen(scratchBuffer)], "\n");
            LOG(DEBUG_LEVEL_INFO, DBG_MCC_ENC, "%s", scratchBuffer);
            scratchBuffer[0] = '\0';
        }
    }
    if( scratchBuffer[0] != '\0' ) {
        LOG(DEBUG_LEVEL_INFO, DBG_MCC_ENC, "%s", scratchBuffer);
    }

    LOG(DEBUG_LEVEL_VERBOSE, DBG_MCC_ENC, "Shutting down MCC Encode pipeline element.");

    if( ((Context*)rootCtxPtr)->mccEncodeCtxPtr->totalSkew != 0 ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_ENC, "Adding %d frames to correct skew from missing frames in source", ((Context*)rootCtxPtr)->mccEncodeCtxPtr->totalSkew);
    }
    if( ((Context*)rootCtxPtr)->config.matchPtsTime == TRUE ) {
        LOG(DEBUG_LEVEL_INFO, DBG_MCC_ENC, "Added %ld Fill Frames to match PTS Time", ((Context *) rootCtxPtr)->mccEncodeCtxPtr->numFillFrames);
    }
    if( ((Context*)rootCtxPtr)->config.matchPtsTime == TRUE ) {
        LOG(DEBUG_LEVEL_INFO, DBG_MCC_ENC, "Added %ld Fill Frames to match PTS Time", ((Context *) rootCtxPtr)->mccEncodeCtxPtr->numFillFrames);
    }
    LOG(DEBUG_LEVEL_INFO, DBG_MCC_ENC, "Frame Time Ahead High Water Mark - %lld ms", ((Context*)rootCtxPtr)->mccEncodeCtxPtr->maxPositiveDelta);
    LOG(DEBUG_LEVEL_INFO, DBG_MCC_ENC, "Frame Time Behind High Water Mark - %lld ms", ((Context*)rootCtxPtr)->mccEncodeCtxPtr->maxNegativeDelta);

    free(((Context*)rootCtxPtr)->mccEncodeCtxPtr);
    ((Context*)rootCtxPtr)->mccEncodeCtxPtr = NULL;

    return ShutdownSinks(rootCtxPtr, &sinks);
}  // MccEncodeShutdown()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    generateMccHeader()
 |
 | DESCRIPTION:
 |    This function will build the standard MCC Header and send it line by line
 |    to the next component in the pipeline.
 -------------------------------------------------------------------------------*/
static boolean generateMccHeader( Context* ctxPtr, CaptionTime* captionTimePtr ) {
    uuid_t binuuid;
    char uuidStr[50];
    char dateStr[50];
    char timeStr[30];
    char tcrString[25];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    sprintf(uuidStr, "UUID=");
    uuid_generate_random(binuuid);
    uuid_unparse_upper(binuuid, &uuidStr[5]);
    uuidStr[41] = '\0';
    
    ASSERT(tm.tm_wday < 7);
    ASSERT(tm.tm_mon < 12);
    sprintf(dateStr, "Creation Date=%s, %s %d, %d", DayOfWeekStr[tm.tm_wday], MonthStr[tm.tm_mon], tm.tm_mday, tm.tm_year + 1900);
    sprintf(timeStr, "Creation Time=%d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
    
    switch( numCcConstructsFromFramerate(captionTimePtr->frameRatePerSecTimesOneHundred) ) {
        case CC_COUNT_FRAME_RATE_24:
            sprintf(tcrString, "Time Code Rate=24\n");
            break;
        case CC_COUNT_FRAME_RATE_25:
            sprintf(tcrString, "Time Code Rate=25\n");
            break;
        case CC_COUNT_FRAME_RATE_30:
            if( captionTimePtr->dropframe == TRUE ) {
                sprintf(tcrString, "Time Code Rate=30DF\n");
            } else {
                sprintf(tcrString, "Time Code Rate=30\n");
            }
            break;
        case CC_COUNT_FRAME_RATE_50:
            sprintf(tcrString, "Time Code Rate=50\n");
            break;
        case CC_COUNT_FRAME_RATE_60:
            if( captionTimePtr->dropframe == TRUE ) {
                sprintf(tcrString, "Time Code Rate=60DF\n");
            } else {
                sprintf(tcrString, "Time Code Rate=60\n");
            }
            break;
        default:
            break;
    }
    
    if( sendMccText(ctxPtr, "File Format=MacCaption_MCC V1.0\n", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "///////////////////////////////////////////////////////////////////////////////////", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "// Telestream, LLC", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "// Ancillary Data Packet Transfer File", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//\n// Permission to generate this format is granted provided that", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//   1. This ANC Transfer file format is used on an as-is basis and no warranty is given, and", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//   2. This entire descriptive information text is included in a generated .mcc file.", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//\n// General file format:", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//   HH:MM:SS:FF(tab)[Hexadecimal ANC data in groups of 2 characters]", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     Hexadecimal data starts with the Ancillary Data Packet DID (Data ID defined in S291M)", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//       and concludes with the Check Sum following the User Data Words.", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     Each time code line must contain at most one complete ancillary data packet.", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     To transfer additional ANC Data successive lines may contain identical time code.", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     Time Code Rate=[24, 25, 30, 30DF, 50, 60, 60DF]\n//", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//   ANC data bytes may be represented by one ASCII character according to the following schema:", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     G  FAh 00h 00h", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     H  2 x (FAh 00h 00h)", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     I  3 x (FAh 00h 00h)", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     J  4 x (FAh 00h 00h)", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     K  5 x (FAh 00h 00h)", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     L  6 x (FAh 00h 00h)", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     M  7 x (FAh 00h 00h)", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     N  8 x (FAh 00h 00h)", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     O  9 x (FAh 00h 00h)", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     P  FBh 80h 80h", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     Q  FCh 80h 80h", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     R  FDh 80h 80h", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     S  96h 69h", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     T  61h 01h", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     U  E1h 00h 00h 00h", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     Q  FCh 80h 80h", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     Q  FCh 80h 80h", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "//     Z  00h\n//", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "///////////////////////////////////////////////////////////////////////////////////\n", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, uuidStr, captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, "Creation Program=Caption Inspector", captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, dateStr, captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, timeStr, captionTimePtr) == FALSE ) return FALSE;
    if( sendMccText(ctxPtr, tcrString, captionTimePtr) == FALSE ) return FALSE;

    return TRUE;
} // generateMccHeader()

/*------------------------------------------------------------------------------
 | NAME:
 |    addFillPacket()
 |
 | DESCRIPTION:
 |    This function will generate a packet of fill and send it to the next
 |    stage in the pipeline. This is needed when the PTS timestamp from the
 |    asset trails the Frame Timestamp in the MCC file.
 ------------------------------------------------------------------------------*/
static void addFillPacket( Context* rootCtxPtr, CaptionTime* captionTimePtr ) {
    MccEncodeCtx* ctxPtr = rootCtxPtr->mccEncodeCtxPtr;
    uint8 ccCount = numCcConstructsFromFramerate(captionTimePtr->frameRatePerSecTimesOneHundred);

    Buffer* fillBuffer = NewBuffer(BUFFER_TYPE_BYTES, (ccCount * 3));
    fillBuffer->numElements = fillBuffer->maxNumElements;
    fillBuffer->captionTime = *captionTimePtr;

    fillBuffer->dataPtr[0] = VALID_CEA608E_LINE21_FIELD_1_CC;
    fillBuffer->dataPtr[1] = CEA608_ZERO_WITH_ODD_PARITY;
    fillBuffer->dataPtr[2] = CEA608_ZERO_WITH_ODD_PARITY;
    fillBuffer->dataPtr[3] = VALID_CEA608E_LINE21_FIELD_2_CC;
    fillBuffer->dataPtr[4] = CEA608_ZERO_WITH_ODD_PARITY;
    fillBuffer->dataPtr[5] = CEA608_ZERO_WITH_ODD_PARITY;
    uint8* tmpPtr = &fillBuffer->dataPtr[6];
    for( int loop = 2; loop < ccCount; loop++ ) {
        tmpPtr[0] = INVALID_DTVCCC_CHANNEL_PACKET_DATA;
        tmpPtr[1] = EMPTY_DTVCC_CHANNEL_PACKET_DATA;
        tmpPtr[2] = EMPTY_DTVCC_CHANNEL_PACKET_DATA;
        tmpPtr = &tmpPtr[3];
    }

    Buffer* withBoilerPlateBuffer = addBoilerplate(ctxPtr, fillBuffer);
    withBoilerPlateBuffer->captionTime = *captionTimePtr;
    uint16 numCharsNeeded = countChars( withBoilerPlateBuffer->dataPtr, withBoilerPlateBuffer->numElements );
    LOG(DEBUG_LEVEL_VERBOSE, DBG_MCC_ENC, "FILL: With CDP Boiler Plate %d byte packet at time: %02d:%02d:%02d;%02d requires %d bytes compressed",
        withBoilerPlateBuffer->numElements, withBoilerPlateBuffer->captionTime.hour, withBoilerPlateBuffer->captionTime.minute,
        withBoilerPlateBuffer->captionTime.second, withBoilerPlateBuffer->captionTime.frame, numCharsNeeded);
    Buffer* outputBuffer = NewBuffer(BUFFER_TYPE_BYTES, (numCharsNeeded + 13));
    outputBuffer->captionTime = withBoilerPlateBuffer->captionTime;
    sprintf((char*)outputBuffer->dataPtr, "%02d:%02d:%02d:%02d\t", withBoilerPlateBuffer->captionTime.hour,
            withBoilerPlateBuffer->captionTime.minute, withBoilerPlateBuffer->captionTime.second,
            withBoilerPlateBuffer->captionTime.frame);
    outputBuffer->numElements = 13;
    compressData( withBoilerPlateBuffer->dataPtr, withBoilerPlateBuffer->numElements, outputBuffer );
    FreeBuffer(withBoilerPlateBuffer);
    LOG(DEBUG_LEVEL_VERBOSE, DBG_MCC_ENC, "Sending Compressed %d byte FILL packet at time: %02d:%02d:%02d;%02d",
        outputBuffer->numElements, outputBuffer->captionTime.hour, outputBuffer->captionTime.minute,
        outputBuffer->captionTime.second, outputBuffer->captionTime.frame);

    if( PassToSinks(rootCtxPtr, outputBuffer, &ctxPtr->sinks) != PIPELINE_SUCCESS ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_ENC, "Attempt to add Fill Frame Failed. Error was suppressed.");
    }

} // addFillPacket()

/*------------------------------------------------------------------------------
 | NAME:
 |    handleSkew()
 |
 | DESCRIPTION:
 |    This function will determine if skew is being added as a result of missing
 |    closed captioning frames. If it is, this function will notify the calling
 |    function to add fill packets to compensate. All of the results of this
 |    function will be logged.
 ------------------------------------------------------------------------------*/
static uint8 handleSkew( Context* rootCtxPtr, CaptionTime* inCaptionTimePtr ) {
    uint8 framerateLow = 0;
    uint8 framerateHigh = 0;
    MccEncodeCtx* ctxPtr = rootCtxPtr->mccEncodeCtxPtr;
    uint8 retval = 0;

    if( ((inCaptionTimePtr->frameRatePerSecTimesOneHundred / 100) == 29) || ((inCaptionTimePtr->frameRatePerSecTimesOneHundred / 100) == 30) ) {
        framerateHigh = 30;
        framerateLow = 29;
    } else if( ((inCaptionTimePtr->frameRatePerSecTimesOneHundred / 100) == 59) || ((inCaptionTimePtr->frameRatePerSecTimesOneHundred / 100) == 60) ) {
        framerateHigh = 60;
        framerateLow = 59;
    } else if( ((inCaptionTimePtr->frameRatePerSecTimesOneHundred / 100) == 23) || ((inCaptionTimePtr->frameRatePerSecTimesOneHundred / 100) == 24) ) {
        framerateHigh = 24;
        framerateLow = 23;
    } else if( (inCaptionTimePtr->frameRatePerSecTimesOneHundred / 100) == 25 ) {
        framerateHigh = 25;
        framerateLow = 25;
    } else {
        LOG(DEBUG_LEVEL_FATAL, DBG_MCC_ENC, "Unexpected and unhandled framerate: %ld", inCaptionTimePtr->frameRatePerSecTimesOneHundred);
    }

    if( (ctxPtr->lastCaptionTime.minute == inCaptionTimePtr->minute) && (ctxPtr->lastCaptionTime.second == inCaptionTimePtr->second) ) {
        ctxPtr->framesPerSec[inCaptionTimePtr->second]++;
    } else if( ctxPtr->lastCaptionTime.minute != inCaptionTimePtr->minute ) {
        uint32 minuteSkew = 0;
        uint32 minuteFrames = 0;
        char scratchBuffer[256];
        scratchBuffer[0] = '\0';
        for( int loop = 0; loop < 60; loop++ ) {
            if( (ctxPtr->framesPerSec[loop] != framerateLow) && (ctxPtr->framesPerSec[loop] != framerateHigh) ) {
                sprintf(&scratchBuffer[strlen(scratchBuffer)], "%02d* ", ctxPtr->framesPerSec[loop]);
                ASSERT(ctxPtr->framesPerSec[loop] < framerateHigh);
                minuteSkew = minuteSkew + (framerateHigh - ctxPtr->framesPerSec[loop]);
            } else if( ctxPtr->framesPerSec[loop] == framerateLow ) {
                sprintf(&scratchBuffer[strlen(scratchBuffer)], "%02d! ", ctxPtr->framesPerSec[loop]);
            } else {
                sprintf(&scratchBuffer[strlen(scratchBuffer)], "%02d  ", ctxPtr->framesPerSec[loop]);
            }
            minuteFrames = minuteFrames + ctxPtr->framesPerSec[loop];
            ctxPtr->framesPerSec[loop] = 0;
            if(loop == 29 ) {
                sprintf(&scratchBuffer[strlen(scratchBuffer)], "\n");
                LOG(DEBUG_LEVEL_INFO, DBG_MCC_ENC, "%s", scratchBuffer);
                scratchBuffer[0] = '\0';
            }
        }
        if( minuteSkew == 0 ) {
            sprintf(&scratchBuffer[strlen(scratchBuffer)], "%d:%02d -> %d:%02d = %ld\n",
                    ctxPtr->lastCaptionTime.hour, ctxPtr->lastCaptionTime.minute, inCaptionTimePtr->hour,
                    inCaptionTimePtr->minute, minuteFrames);
        } else {
            sprintf(&scratchBuffer[strlen(scratchBuffer)], "%d:%02d -> %d:%02d = %ld  --- Skew %ld\n",
                    ctxPtr->lastCaptionTime.hour, ctxPtr->lastCaptionTime.minute, inCaptionTimePtr->hour,
                    inCaptionTimePtr->minute, minuteFrames, minuteSkew);
        }
        LOG(DEBUG_LEVEL_INFO, DBG_MCC_ENC, "%s", scratchBuffer);
        ctxPtr->totalSkew = ctxPtr->totalSkew + minuteSkew;
        ctxPtr->framesPerSec[inCaptionTimePtr->second]++;
    } else if( ctxPtr->lastCaptionTime.second != inCaptionTimePtr->second ) {
        ctxPtr->framesPerSec[inCaptionTimePtr->second]++;
        if( (ctxPtr->framesPerSec[ctxPtr->lastCaptionTime.second] == framerateLow) || (ctxPtr->framesPerSec[ctxPtr->lastCaptionTime.second] == framerateHigh) ) {
            ctxPtr->fullSecondFound = TRUE;
        } else if( ctxPtr->fullSecondFound == TRUE ) {
            ASSERT(ctxPtr->framesPerSec[ctxPtr->lastCaptionTime.second] < framerateHigh);
            retval = framerateHigh - ctxPtr->framesPerSec[ctxPtr->lastCaptionTime.second];
            LOG(DEBUG_LEVEL_ERROR, DBG_MCC_ENC, "Adding %d frames to correct skew from missing frames at: %02d:%02d:%02d.%03d", retval,
                inCaptionTimePtr->hour, inCaptionTimePtr->minute, inCaptionTimePtr->second, inCaptionTimePtr->millisecond);
        } else {
            LOG(DEBUG_LEVEL_WARN, DBG_MCC_ENC, "Missing %d frames in the front of the asset: %02d:%02d:%02d.%03d Ignoring.",
                (framerateHigh - ctxPtr->framesPerSec[ctxPtr->lastCaptionTime.second]), inCaptionTimePtr->hour,
                inCaptionTimePtr->minute, inCaptionTimePtr->second, inCaptionTimePtr->millisecond);
        }
    } else {
        ASSERT(0);
    }

    ctxPtr->lastCaptionTime.hour = inCaptionTimePtr->hour;
    ctxPtr->lastCaptionTime.minute = inCaptionTimePtr->minute;
    ctxPtr->lastCaptionTime.second = inCaptionTimePtr->second;

    return retval;
} // handleSkew()

/*------------------------------------------------------------------------------
 | NAME:
 |    convertCaptionTime()
 |
 | DESCRIPTION:
 |    This function will convert the time derived from PTS (in Milliseconds)
 |    to frame number, which matches the time specified in SCC or MCC.
 ------------------------------------------------------------------------------*/
static CaptionTime convertCaptionTime( Context* rootCtxPtr, Buffer* inBufferPtr ) {
    MccEncodeCtx* ctxPtr = rootCtxPtr->mccEncodeCtxPtr;
    CaptionTime* inCaptionTimePtr = &inBufferPtr->captionTime;
    CaptionTime captionTime;
    boolean timeSynch = FALSE;
    int64 actualTimeInMs = 0;
    int64 frameTimeInMs = 0;
    int64 deltaInMs;
    int64 frameSizeMs = (100000 / inCaptionTimePtr->frameRatePerSecTimesOneHundred) + 1;
    uint8 skewFillPackets = 0;

    while( timeSynch == FALSE ) {
        captionTime.frameRatePerSecTimesOneHundred = inCaptionTimePtr->frameRatePerSecTimesOneHundred;
        captionTime.dropframe = inCaptionTimePtr->dropframe;

        captionTime.hour = ctxPtr->nextCaptionTime.hour;
        captionTime.minute = ctxPtr->nextCaptionTime.minute;
        captionTime.second = ctxPtr->nextCaptionTime.second;
        captionTime.frame = ctxPtr->nextCaptionTime.frame;
        captionTime.millisecond = 0;
        captionTime.source = CAPTION_TIME_FRAME_NUMBERING;

        ctxPtr->nextCaptionTime.frame = ctxPtr->nextCaptionTime.frame + 1;

        uint8 frameRollOver = captionTime.frameRatePerSecTimesOneHundred / 100;
        if ((captionTime.frameRatePerSecTimesOneHundred % 100) > 75) frameRollOver++;

        if (ctxPtr->nextCaptionTime.frame >= frameRollOver) {
            ctxPtr->nextCaptionTime.frame = 0;
            ctxPtr->nextCaptionTime.second = ctxPtr->nextCaptionTime.second + 1;
        }

        if (ctxPtr->nextCaptionTime.second >= 60) {
            ctxPtr->nextCaptionTime.second = 0;
            ctxPtr->nextCaptionTime.minute = ctxPtr->nextCaptionTime.minute + 1;
        }

        if (ctxPtr->nextCaptionTime.minute >= 60) {
            ctxPtr->nextCaptionTime.minute = 0;
            ctxPtr->nextCaptionTime.hour = ctxPtr->nextCaptionTime.hour + 1;
        }

        if( (captionTime.dropframe == TRUE) && (ctxPtr->nextCaptionTime.second == 0) &&
            (ctxPtr->nextCaptionTime.frame == 0) && ((ctxPtr->nextCaptionTime.minute % 10) != 0) ){
            ctxPtr->nextCaptionTime.frame = 2;
        }

        actualTimeInMs = (((inCaptionTimePtr->hour * 3600) + (inCaptionTimePtr->minute * 60) +
                           (inCaptionTimePtr->second)) * 1000) + inCaptionTimePtr->millisecond;
        frameTimeInMs = (((captionTime.hour * 3600) + (captionTime.minute * 60) + (captionTime.second)) * 1000) +
                        ((captionTime.frame * 100000) / captionTime.frameRatePerSecTimesOneHundred);

#ifdef DEBUG_CAPTION_TIMING
        if( actualTimeInMs > frameSizeMs ) {
            deltaInMs = actualTimeInMs - frameTimeInMs;
            printf("%02d:%02d:%02d:%03d, %02d:%02d:%02d:%02d, %lld, %ld\n", inCaptionTimePtr->hour, inCaptionTimePtr->minute,
                   inCaptionTimePtr->second, inCaptionTimePtr->millisecond, captionTime.hour, captionTime.minute,
                   captionTime.second, captionTime.frame, deltaInMs, ctxPtr->numFillFrames);
        } else if( actualTimeInMs < frameSizeMs ) {
            deltaInMs = frameTimeInMs - actualTimeInMs;
            printf("%02d:%02d:%02d:%03d, %02d:%02d:%02d:%02d, -%lld, %ld\n", inCaptionTimePtr->hour, inCaptionTimePtr->minute,
                   inCaptionTimePtr->second, inCaptionTimePtr->millisecond, captionTime.hour, captionTime.minute,
                   captionTime.second, captionTime.frame, deltaInMs, ctxPtr->numFillFrames);
        }
#endif

        if( ctxPtr->matchPtsTime == TRUE ) {
            deltaInMs = actualTimeInMs - frameTimeInMs;

            if (deltaInMs > frameSizeMs) {
                addFillPacket(rootCtxPtr, &captionTime);
                ctxPtr->numFillFrames = ctxPtr->numFillFrames + 1;
            } else if (deltaInMs < -frameSizeMs) {
                LOG(DEBUG_LEVEL_ERROR, DBG_MCC_ENC, "Code Missing to remove Fill Frame! Time will be skewed: %lld",
                    deltaInMs);
                timeSynch = TRUE;
            } else {
                timeSynch = TRUE;
            }
        } else {
            if( skewFillPackets == 0 ) {
#if 0
                printf("%02d:%02d:%02d:%03d | %02d:%02d:%02d:%02d - ", inCaptionTimePtr->hour, inCaptionTimePtr->minute,
                       inCaptionTimePtr->second, inCaptionTimePtr->millisecond, captionTime.hour, captionTime.minute,
                       captionTime.second, captionTime.frame);
                for( int loop = 0; loop < inBufferPtr->numElements; loop++ ) {
                    uint8 msn, lsn;
                    byteToAscii(inBufferPtr->dataPtr[loop], &msn, &lsn );
                    printf("%c%c ", msn, lsn);
                }
                printf("\n");
#endif
                skewFillPackets = handleSkew(rootCtxPtr, inCaptionTimePtr);
                if( skewFillPackets == 0 ) {
                    timeSynch = TRUE;
                }
            } else {
                addFillPacket(rootCtxPtr, &captionTime);
                skewFillPackets = skewFillPackets - 1;
                if( skewFillPackets == 0 ) {
                    timeSynch = TRUE;
                }
            }
        }
    }

    if( actualTimeInMs > frameSizeMs ) {
        deltaInMs = actualTimeInMs - frameTimeInMs;
        if( ctxPtr->maxPositiveDelta < deltaInMs ) {
            ctxPtr->maxPositiveDelta = deltaInMs;
        }
    } else if( actualTimeInMs < frameSizeMs ) {
        deltaInMs = frameTimeInMs - actualTimeInMs;
        if( ctxPtr->maxNegativeDelta < deltaInMs ) {
            ctxPtr->maxNegativeDelta = deltaInMs;
        }
    }

    LOG(DEBUG_LEVEL_VERBOSE, DBG_MCC_ENC, "Converting Caption Time: %02d:%02d:%02d,%03d - %02d:%02d:%02d:%02d",
        inCaptionTimePtr->hour, inCaptionTimePtr->minute, inCaptionTimePtr->second, inCaptionTimePtr->millisecond,
        captionTime.hour, captionTime.minute, captionTime.second, captionTime.frame );

    return captionTime;
}  // convertCaptionTime()

/*------------------------------------------------------------------------------
 | NAME:
 |    sendMccText()
 |
 | DESCRIPTION:
 |    This function sends a line of text to the next component in the pipeline.
 -------------------------------------------------------------------------------*/
static boolean sendMccText( Context* ctxPtr, char* textStr, CaptionTime* captionTimePtr ) {
    Buffer* outputBuffer = NewBuffer(BUFFER_TYPE_BYTES, (strlen(textStr)+1));
    strcpy((char*)outputBuffer->dataPtr, textStr);

    outputBuffer->captionTime = *captionTimePtr;
    outputBuffer->numElements = outputBuffer->maxNumElements;
    return PassToSinks(ctxPtr, outputBuffer, &ctxPtr->mccEncodeCtxPtr->sinks);
}  // sendMccText()

/*------------------------------------------------------------------------------
 | NAME:
 |    addBoilerplate()
 |
 | DESCRIPTION:
 |    This method encodes the Anciallary Data (ANC) Packet, which wraps the Caption
 |    Distribution Packet (CDP), including the Closed Captioning Data (ccdata_section) as
 |    described in the CEA-708 Spec. Below is the list of specs that were leveraged for
 |    this encode:
 |
 |    SMPTE ST 334-1 - Vertical Ancillary Data Mapping of Caption Data and Other Related Data
 |                     (Specifically: SMPTE ST 334-1:2015 - Revision of SMPTE 334-1-2007)
 |    SMPTE ST 334-2 - Caption Distribution Packet (CDP) Definition
 |                     (Specifically: SMPTE ST 334-2:2015 - Revision of SMPTE 334-2-2007)
 |    CEA-708-D - CEA Standard - Digital Television (DTV) Closed Captioning - August 2008
 -------------------------------------------------------------------------------*/
static Buffer* addBoilerplate( MccEncodeCtx* ctxPtr, Buffer* inBufferPtr ) {
    ASSERT(inBufferPtr->maxNumElements == inBufferPtr->numElements);
    ASSERT(!(inBufferPtr->maxNumElements % 3));
    uint8 ccCount = inBufferPtr->maxNumElements / 3;
    Buffer* outBufferPtr = NewBuffer(BUFFER_TYPE_BYTES, (inBufferPtr->numElements + 17));
    outBufferPtr->numElements = inBufferPtr->numElements + 17;
    uint8* dataPtr = outBufferPtr->dataPtr;
    
    dataPtr[0] = ANC_DID_CLOSED_CAPTIONING;
    dataPtr[1] = ANC_SDID_CEA_708;
    dataPtr[2] = inBufferPtr->numElements + 13;
    dataPtr[3] = CDP_IDENTIFIER_VALUE_HIGH;
    dataPtr[4] = CDP_IDENTIFIER_VALUE_LOW;
    dataPtr[5] = inBufferPtr->numElements + 13;
    dataPtr[6] = ((cdpFramerateFromFramerate(inBufferPtr->captionTime.frameRatePerSecTimesOneHundred) << 4) | 0x0F);
    dataPtr[7] = 0x43;  // Timecode not Present; Service Info not Present; Captions Present
    dataPtr[8] = (uint8)((ctxPtr->cdpHeaderSequence & 0xF0) >> 8);
    dataPtr[9] = (uint8)(ctxPtr->cdpHeaderSequence & 0x0F);
    dataPtr[10] = CC_DATA_ID;
    dataPtr[11] = ccCount | 0xE0;
    memcpy( &dataPtr[12], inBufferPtr->dataPtr, inBufferPtr->numElements );
    dataPtr = &dataPtr[(12+inBufferPtr->numElements)];
    dataPtr[0] = CDP_FOOTER_ID;
    dataPtr[1] = (uint8)((ctxPtr->cdpHeaderSequence & 0xF0) >> 8);
    dataPtr[2] = (uint8)(ctxPtr->cdpHeaderSequence & 0x0F);
    dataPtr[3] = 0; // cdp checksum
    dataPtr[4] = 0; // vanc checksum

    //the arithmetic sum of the entire packet (first byte of cdp_identifier to packet_checksum, inclusive) modulo 256 equal zero.
    for( int loop = 3; loop < (inBufferPtr->numElements + 15); loop++ ) {
        dataPtr[3] = dataPtr[3] + outBufferPtr->dataPtr[loop];
    }

    dataPtr[3] = (~dataPtr[3]) + 1;  //Two's complement value is the checksum

    for( int loop = 0; loop < 3; loop++ ) {
        dataPtr[4] = dataPtr[4] + outBufferPtr->dataPtr[loop];
    }
    
    FreeBuffer( inBufferPtr );
    ctxPtr->cdpHeaderSequence++;
    
    return outBufferPtr;
} // addBoilerplate()

/*------------------------------------------------------------------------------
 | NAME:
 |    countChars()
 |
 | DESCRIPTION:
 |    This function counts the chars needed for the output text stream.
 |
 |    Per the description in the MCC File:
 |
 |    ANC data bytes may be represented by one ASCII character according
 |    to the following schema:
 |      G  FAh 00h 00h
 |      H  2 x (FAh 00h 00h)
 |      I  3 x (FAh 00h 00h)
 |      J  4 x (FAh 00h 00h)
 |      K  5 x (FAh 00h 00h)
 |      L  6 x (FAh 00h 00h)
 |      M  7 x (FAh 00h 00h)
 |      N  8 x (FAh 00h 00h)
 |      O  9 x (FAh 00h 00h)
 |      P  FBh 80h 80h
 |      Q  FCh 80h 80h
 |      R  FDh 80h 80h
 |      S  96h 69h
 |      T  61h 01h
 |      U  E1h 00h 00h 00h
 |      Z  00h
 -------------------------------------------------------------------------------*/
static uint16 countChars( uint8* dataPtr, uint16 numElements ) {
    uint16 numChars = 1;
    
    while( numElements > 0 ) {
        switch( dataPtr[0] ) {
            case 0xFA:
                if( (numElements >= 3) && (dataPtr[1] == 0x00) && (dataPtr[2] == 0x00) ) {
                    uint8 numFaoos = 0;
                    while( (numElements >= 3) && (dataPtr[0] == 0xFA) && (dataPtr[1] == 0x00) &&
                           (dataPtr[2] == 0x00) && (numFaoos < 9) ) {
                        dataPtr = &dataPtr[3];
                        numElements = numElements - 3;
                        numFaoos++;
                    }
                    numChars = numChars + 1;
                } else {
                    dataPtr = &dataPtr[1];
                    numElements = numElements - 1;
                    numChars = numChars + 2;
                }
                break;
            case 0xFB:
            case 0xFC:
            case 0xFD:
                if( (numElements >= 3) && (dataPtr[1] == 0x80) && (dataPtr[2] == 0x80) ) {
                    dataPtr = &dataPtr[3];
                    numElements = numElements - 3;
                    numChars = numChars + 1;
                } else {
                    dataPtr = &dataPtr[1];
                    numElements = numElements - 1;
                    numChars = numChars + 2;
                }
                break;
            case 0x96:
                if( (numElements >= 2) && dataPtr[1] == 0x69 ) {
                    dataPtr = &dataPtr[2];
                    numElements = numElements - 2;
                    numChars = numChars + 1;
                } else {
                    dataPtr = &dataPtr[1];
                    numElements = numElements - 1;
                    numChars = numChars + 2;
                }
                break;
            case 0x61:
                if( (numElements >= 2) && dataPtr[1] == 0x01 ) {
                    dataPtr = &dataPtr[2];
                    numElements = numElements - 2;
                    numChars = numChars + 1;
                } else {
                    dataPtr = &dataPtr[1];
                    numElements = numElements - 1;
                    numChars = numChars + 2;
                }
                break;
            case 0xE1:
                if( (numElements >= 4) && (dataPtr[1] == 0x00) && (dataPtr[2] == 0x00) && (dataPtr[3] == 0x00) ) {
                    dataPtr = &dataPtr[4];
                    numElements = numElements - 4;
                    numChars = numChars + 1;
                } else {
                    dataPtr = &dataPtr[1];
                    numElements = numElements - 1;
                    numChars = numChars + 2;
                }
                break;
            case 0x00:
                dataPtr = &dataPtr[1];
                numElements = numElements - 1;
                numChars = numChars + 1;
                break;
            default:
                dataPtr = &dataPtr[1];
                numElements = numElements - 1;
                numChars = numChars + 2;
                break;
        }
    }
    return numChars;
} // countChars()

/*------------------------------------------------------------------------------
 | NAME:
 |    compressData()
 |
 | DESCRIPTION:
 |    This function uses the MCC byte replacment to compress the data.
 |
 |    Per the description in the MCC File:
 |
 |    ANC data bytes may be represented by one ASCII character according
 |    to the following schema:
 |      G  FAh 00h 00h
 |      H  2 x (FAh 00h 00h)
 |      I  3 x (FAh 00h 00h)
 |      J  4 x (FAh 00h 00h)
 |      K  5 x (FAh 00h 00h)
 |      L  6 x (FAh 00h 00h)
 |      M  7 x (FAh 00h 00h)
 |      N  8 x (FAh 00h 00h)
 |      O  9 x (FAh 00h 00h)
 |      P  FBh 80h 80h
 |      Q  FCh 80h 80h
 |      R  FDh 80h 80h
 |      S  96h 69h
 |      T  61h 01h
 |      U  E1h 00h 00h 00h
 |      Z  00h
 -------------------------------------------------------------------------------*/
static void compressData( uint8* dataPtr, uint16 numElements, Buffer* outBufPtr ) {
    uint8* outDataPtr = &outBufPtr->dataPtr[12];
    
    while( numElements > 0 ) {
        switch( dataPtr[0] ) {
            case 0xFA:
                if( (numElements >= 3) && (dataPtr[1] == 0x00) && (dataPtr[2] == 0x00) ) {
                    uint8 numFaoos = 0;
                    while( (numElements >= 3) && (dataPtr[0] == 0xFA) && (dataPtr[1] == 0x00) &&
                          (dataPtr[2] == 0x00) && (numFaoos < 9) ) {
                        dataPtr = &dataPtr[3];
                        numElements = numElements - 3;
                        numFaoos++;
                    }
                    switch( numFaoos ) {
                        case 1:
                            *outDataPtr = 'G';
                            break;
                        case 2:
                            *outDataPtr = 'H';
                            break;
                        case 3:
                            *outDataPtr = 'I';
                            break;
                        case 4:
                            *outDataPtr = 'J';
                            break;
                        case 5:
                            *outDataPtr = 'K';
                            break;
                        case 6:
                            *outDataPtr = 'L';
                            break;
                        case 7:
                            *outDataPtr = 'M';
                            break;
                        case 8:
                            *outDataPtr = 'N';
                            break;
                        case 9:
                            *outDataPtr = 'O';
                            break;
                        default:
                            LOG(DEBUG_LEVEL_ERROR, DBG_MCC_ENC, "Invalid Branch: %d", numFaoos);
                    }
                    outDataPtr = &outDataPtr[1];
                    outBufPtr->numElements = outBufPtr->numElements + 1;
                    ASSERT(outBufPtr->numElements <= outBufPtr->maxNumElements);
                } else {
                    byteToAscii( dataPtr[0], &outDataPtr[0], &outDataPtr[1] );
                    outDataPtr = &outDataPtr[2];
                    outBufPtr->numElements = outBufPtr->numElements + 2;
                    ASSERT(outBufPtr->numElements <= outBufPtr->maxNumElements);
                    dataPtr = &dataPtr[1];
                    numElements = numElements - 1;
                }
                break;
            case 0xFB:
            case 0xFC:
            case 0xFD:
                if( (numElements >= 3) && (dataPtr[1] == 0x80) && (dataPtr[2] == 0x80) ) {
                    switch( dataPtr[0] ) {
                        case 0xFB:
                            *outDataPtr = 'P';
                            break;
                        case 0xFC:
                            *outDataPtr = 'Q';
                            break;
                        case 0xFD:
                            *outDataPtr = 'R';
                            break;
                        default:
                            ASSERT(0);
                    }
                    dataPtr = &dataPtr[3];
                    numElements = numElements - 3;
                    outDataPtr = &outDataPtr[1];
                    outBufPtr->numElements = outBufPtr->numElements + 1;
                    ASSERT(outBufPtr->numElements <= outBufPtr->maxNumElements);
                } else {
                    byteToAscii( dataPtr[0], &outDataPtr[0], &outDataPtr[1] );
                    outDataPtr = &outDataPtr[2];
                    outBufPtr->numElements = outBufPtr->numElements + 2;
                    ASSERT(outBufPtr->numElements <= outBufPtr->maxNumElements);
                    dataPtr = &dataPtr[1];
                    numElements = numElements - 1;
                }
                break;
            case 0x96:
                if( (numElements >= 2) && dataPtr[1] == 0x69 ) {
                    dataPtr = &dataPtr[2];
                    numElements = numElements - 2;
                    *outDataPtr = 'S';
                    outDataPtr = &outDataPtr[1];
                    outBufPtr->numElements = outBufPtr->numElements + 1;
                    ASSERT(outBufPtr->numElements <= outBufPtr->maxNumElements);
                } else {
                    byteToAscii( dataPtr[0], &outDataPtr[0], &outDataPtr[1] );
                    outDataPtr = &outDataPtr[2];
                    outBufPtr->numElements = outBufPtr->numElements + 2;
                    ASSERT(outBufPtr->numElements <= outBufPtr->maxNumElements);
                    dataPtr = &dataPtr[1];
                    numElements = numElements - 1;
                }
                break;
            case 0x61:
                if( (numElements >= 2) && dataPtr[1] == 0x01 ) {
                    dataPtr = &dataPtr[2];
                    numElements = numElements - 2;
                    *outDataPtr = 'T';
                    outDataPtr = &outDataPtr[1];
                    outBufPtr->numElements = outBufPtr->numElements + 1;
                    ASSERT(outBufPtr->numElements <= outBufPtr->maxNumElements);
                } else {
                    byteToAscii( dataPtr[0], &outDataPtr[0], &outDataPtr[1] );
                    outDataPtr = &outDataPtr[2];
                    outBufPtr->numElements = outBufPtr->numElements + 2;
                    ASSERT(outBufPtr->numElements <= outBufPtr->maxNumElements);
                    dataPtr = &dataPtr[1];
                    numElements = numElements - 1;
                }
                break;
            case 0xE1:
                if( (numElements >= 4) && (dataPtr[1] == 0x00) && (dataPtr[2] == 0x00) && (dataPtr[3] == 0x00) ) {
                    dataPtr = &dataPtr[4];
                    numElements = numElements - 4;
                    *outDataPtr = 'U';
                    outDataPtr = &outDataPtr[1];
                    outBufPtr->numElements = outBufPtr->numElements + 1;
                    ASSERT(outBufPtr->numElements <= outBufPtr->maxNumElements);
                } else {
                    byteToAscii( dataPtr[0], &outDataPtr[0], &outDataPtr[1] );
                    outDataPtr = &outDataPtr[2];
                    outBufPtr->numElements = outBufPtr->numElements + 2;
                    ASSERT(outBufPtr->numElements <= outBufPtr->maxNumElements);
                    dataPtr = &dataPtr[1];
                    numElements = numElements - 1;
                }
                break;
            case 0x00:
                dataPtr = &dataPtr[1];
                numElements = numElements - 1;
                *outDataPtr = 'Z';
                outDataPtr = &outDataPtr[1];
                outBufPtr->numElements = outBufPtr->numElements + 1;
                ASSERT(outBufPtr->numElements <= outBufPtr->maxNumElements);
                break;
            default:
                byteToAscii( dataPtr[0], &outDataPtr[0], &outDataPtr[1] );
                outDataPtr = &outDataPtr[2];
                outBufPtr->numElements = outBufPtr->numElements + 2;
                ASSERT(outBufPtr->numElements <= outBufPtr->maxNumElements);
                dataPtr = &dataPtr[1];
                numElements = numElements - 1;
                break;
        }
    }
    *outDataPtr = '\0';
    outBufPtr->numElements = outBufPtr->numElements + 1;
    ASSERT(outBufPtr->numElements == outBufPtr->maxNumElements);
} // compressData()
