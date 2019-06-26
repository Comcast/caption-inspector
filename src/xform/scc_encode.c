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

#include "debug.h"

#include "scc_encode.h"
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

static Buffer* generateNewCcDataPacket( uint8, uint8, uint8, uint32, uint32 );

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    SccEncodeInitialize()
 |
 | INPUT PARAMETERS:
 |    ctxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |
 | RETURN VALUES:
 |    LinkInfo -  Information about this element of the pipeline, such that it can
 |                be chained to elements that can consume the specific type of data
 |                that it produces.
 |
 | DESCRIPTION:
 |    This initializes this element of the pipeline.
 -------------------------------------------------------------------------------*/
LinkInfo SccEncodeInitialize( Context* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(!rootCtxPtr->sccEncodeCtxPtr);

    rootCtxPtr->sccEncodeCtxPtr = malloc(sizeof(SccEncodeCtx));
    SccEncodeCtx* ctxPtr = rootCtxPtr->sccEncodeCtxPtr;

    ctxPtr->nextFrameNum = 0;
    ctxPtr->hourAdjust = 0;
    ctxPtr->sccFrameRate = 0;

    InitSinks(&ctxPtr->sinks, SCC_DATA___CC_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = SCC_DATA___CC_DATA;
    linkInfo.sourceType = DATA_TYPE_SCC_DATA;
    linkInfo.sinkType = DATA_TYPE_CC_DATA;
    linkInfo.NextBufferFnPtr = &SccEncodeProcNextBuffer;
    linkInfo.ShutdownFnPtr = &SccEncodeShutdown;
    return linkInfo;
}  // SccEncodeInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    SccEncodeAddSink()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    linkInfo - Describes the potential next element of the pipeline.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method binds the next element to this element, after validating that
 |    the data it receives is compatible with the data tht this element sends.
 -------------------------------------------------------------------------------*/
boolean SccEncodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    ASSERT(rootCtxPtr);
    ASSERT(rootCtxPtr->sccEncodeCtxPtr);

    if( linkInfo.sourceType != DATA_TYPE_CC_DATA ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_SCC_ENC, "Attempt to add Sink that cannot process CC Data");
        return FALSE;
    }
    
    return AddSink(&rootCtxPtr->sccEncodeCtxPtr->sinks, &linkInfo);
}  // SccEncodeAddSink()

/*------------------------------------------------------------------------------
 | NAME:
 |    SccEncodeProcNextBuffer()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    inBuffer - Pointer to the buffer to process.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method processes an incoming buffer, encoding line 21 byte pairs
 |    into CC Data.
 -------------------------------------------------------------------------------*/
boolean SccEncodeProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    ASSERT(inBuffer);
    ASSERT(inBuffer->dataPtr);
    ASSERT(inBuffer->numElements);
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->sccEncodeCtxPtr);
    SccEncodeCtx* ctxPtr = ((Context*)rootCtxPtr)->sccEncodeCtxPtr;

    uint32 frame;
    boolean retval = TRUE;
    
    ASSERT(inBuffer->captionTime.frameRatePerSecTimesOneHundred);
    uint8 ccCount = numCcConstructsFromFramerate(inBuffer->captionTime.frameRatePerSecTimesOneHundred);
    
    if( ctxPtr->nextFrameNum == 0 ) {
        ctxPtr->sccFrameRate = inBuffer->captionTime.frameRatePerSecTimesOneHundred;
        if( inBuffer->captionTime.hour == 1 ) {
            ctxPtr->hourAdjust = 1;
            LOG(DEBUG_LEVEL_WARN, DBG_SCC_ENC, "First Caption 1 hour into film, normalizing.");
        }
    }
    
    ASSERT(ctxPtr->sccFrameRate == inBuffer->captionTime.frameRatePerSecTimesOneHundred);
    
    frame = ((((inBuffer->captionTime.hour - ctxPtr->hourAdjust) * 60 * 60) +
             (inBuffer->captionTime.minute * 60) + inBuffer->captionTime.second) * inBuffer->captionTime.frameRatePerSecTimesOneHundred) / 100;
    frame = frame + inBuffer->captionTime.frame;
    
    if( (inBuffer->numElements % 2) != 0 ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_SCC_ENC, "Odd Number of Bytes: %d - %s", inBuffer->numElements, inBuffer->dataPtr);
    }
    
    if( (frame < ctxPtr->nextFrameNum) && ((frame != 0) || (ctxPtr->nextFrameNum != 0 )) ) {
        LOG(DEBUG_LEVEL_WARN, DBG_SCC_ENC, "Alignment Offset at Timecode: %02d:%02d:%02d:%02d with SCC File: %d vs. %d delta: %d frames",
            inBuffer->captionTime.hour, inBuffer->captionTime.minute, inBuffer->captionTime.second, inBuffer->captionTime.frame,
            frame, ctxPtr->nextFrameNum, (ctxPtr->nextFrameNum - frame));
    }
    
    LOG(DEBUG_LEVEL_VERBOSE, DBG_SCC_ENC, "SCC Encode: Received %d bytes of data starting at frame %d with %d CC Constructs - Current frame %d means %d frames of pad and %d frames of data.",
        inBuffer->numElements, frame, ccCount, ctxPtr->nextFrameNum, (ctxPtr->nextFrameNum - frame), (inBuffer->numElements/2) );
    
    // Pad to Current Frame
    for( uint32 loop = ctxPtr->nextFrameNum; loop < frame; loop++ ) {
        Buffer* outputBufPtr = generateNewCcDataPacket( CEA608_ZERO_WITH_ODD_PARITY, CEA608_ZERO_WITH_ODD_PARITY, ccCount, ctxPtr->nextFrameNum, ctxPtr->sccFrameRate );
        LOG(DEBUG_LEVEL_VERBOSE, DBG_SCC_ENC, "SCC Encode: Passing %d bytes of pad at frame %d ", outputBufPtr->numElements, ctxPtr->nextFrameNum );

        if( PassToSinks(rootCtxPtr, outputBufPtr, &ctxPtr->sinks) == FALSE ) {
            retval = FALSE;
        }
        
        ctxPtr->nextFrameNum = ctxPtr->nextFrameNum + 1;
    }
    
    // Break up payload into multiple, contiguous frames
    for( int loop = 0; loop < (inBuffer->numElements / 2); loop++ ) {
        Buffer* outputBufPtr = generateNewCcDataPacket( inBuffer->dataPtr[(loop*2)], inBuffer->dataPtr[((loop*2)+1)], ccCount, ctxPtr->nextFrameNum, ctxPtr->sccFrameRate );
        LOG(DEBUG_LEVEL_VERBOSE, DBG_SCC_ENC, "SCC Encode: Passing %d bytes of data {%02X, %02X} at frame %d ",
            outputBufPtr->numElements, inBuffer->dataPtr[(loop*2)], inBuffer->dataPtr[((loop*2)+1)], ctxPtr->nextFrameNum );
        
        if( PassToSinks(rootCtxPtr, outputBufPtr, &ctxPtr->sinks) == FALSE ) {
            retval = FALSE;
        }
        
        ctxPtr->nextFrameNum = ctxPtr->nextFrameNum + 1;
    }
    
    FreeBuffer(inBuffer);
    return retval;
}  // SccEncodeProcNextBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    SccEncodeShutdown()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method is called when the previous element in the pipeline determines
 |    that there is no more data coming down the pipline. This element will
 |    perform any necessary actions as a result and pass this call down the
 |    pipeline.
 -------------------------------------------------------------------------------*/
boolean SccEncodeShutdown( void* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->sccEncodeCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->sccEncodeCtxPtr->sccFrameRate);
    SccEncodeCtx* ctxPtr = ((Context*)rootCtxPtr)->sccEncodeCtxPtr;

    Sinks sinks = ctxPtr->sinks;

    boolean retval = TRUE;
    uint8 ccCount = numCcConstructsFromFramerate(ctxPtr->sccFrameRate);
    
    Buffer* outputBufPtr = generateNewCcDataPacket( CEA608_ZERO_WITH_ODD_PARITY, CEA608_ZERO_WITH_ODD_PARITY, ccCount,
                                                    ctxPtr->nextFrameNum, ctxPtr->sccFrameRate );

    free(ctxPtr);
    ((Context*)rootCtxPtr)->sccEncodeCtxPtr = NULL;
    
    if( PassToSinks(rootCtxPtr, outputBufPtr, &sinks) == FALSE ) {
        retval = FALSE;
    }
    
    if( ShutdownSinks(rootCtxPtr, &sinks) == FALSE ) {
        retval = FALSE;
    }
    
    return retval;
}  // SccEncodeShutdown()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    generateNewCcDataPacket()
 |
 | DESCRIPTION:
 |    This method allocates a new SEI Packet and fills it out with the appropriate
 |    boiler plate, as well as the passed in data.
 |
 |    The SEI Specification is described in: ANSI/SCTE 128-1 2013
 |    AVC Video Constraints for Cable Television -- Part 1- Coding
 |
 | MEMORY ALLOCATION:
 |    This function allocates memory that will be freed once the data is converted
 |    to Base64 in the method: base64EncodeSeiPacket()
 -------------------------------------------------------------------------------*/
static Buffer* generateNewCcDataPacket( uint8 firstByte, uint8 secondByte, uint8 ccCount, uint32 frameNum, uint32 frameRate ) {
    
    Buffer* outputBuffer = NewBuffer(BUFFER_TYPE_BYTES, (3 * ccCount));
    frameToTimeCode( frameNum, frameRate, &outputBuffer->captionTime );
    outputBuffer->numElements = outputBuffer->maxNumElements;
    
    outputBuffer->dataPtr[0] = VALID_CEA608E_LINE21_FIELD_1_CC;
    outputBuffer->dataPtr[1] = firstByte;
    outputBuffer->dataPtr[2] = secondByte;
    outputBuffer->dataPtr[3] = VALID_CEA608E_LINE21_FIELD_2_CC;
    outputBuffer->dataPtr[4] = CEA608_ZERO_WITH_ODD_PARITY;
    outputBuffer->dataPtr[5] = CEA608_ZERO_WITH_ODD_PARITY;
    uint8* tmpPtr = &outputBuffer->dataPtr[6];
    for( int loop = 2; loop < ccCount; loop++ ) {
        tmpPtr[0] = INVALID_DTVCCC_CHANNEL_PACKET_DATA;
        tmpPtr[1] = EMPTY_DTVCC_CHANNEL_PACKET_DATA;
        tmpPtr[2] = EMPTY_DTVCC_CHANNEL_PACKET_DATA;
        tmpPtr = &tmpPtr[3];
    }
    
    return outputBuffer;
}  // generateNewCcDataPacket()

