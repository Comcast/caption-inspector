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

#include "smpte_encode.h"
#include "line21_decode.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    SmpteEncodeInitialize()
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
LinkInfo SmpteEncodeInitialize( Context* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(!rootCtxPtr->smpteEncodeCtxPtr);

    rootCtxPtr->smpteEncodeCtxPtr = malloc(sizeof(SmpteEncodeCtx));
    SmpteEncodeCtx* ctxPtr = rootCtxPtr->smpteEncodeCtxPtr;

// TODO - Add any initialization of the Context here.
//        This gets run once when the pipeline is plumbed.

    InitSinks(&ctxPtr->sinks, LINE21_DATA___SMPTE_TT_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = LINE21_DATA___SMPTE_TT_DATA;
    linkInfo.sourceType = DATA_TYPE_DECODED_608;
    linkInfo.sinkType = DATA_TYPE_SMPTE_TT;
    linkInfo.NextBufferFnPtr = &SmpteEncodeProcNextBuffer;
    linkInfo.ShutdownFnPtr = &SmpteEncodeShutdown;
    return linkInfo;
}  // SmpteEncodeInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    SmpteEncodeAddSink()
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
boolean SmpteEncodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    ASSERT(rootCtxPtr);
    ASSERT(rootCtxPtr->smpteEncodeCtxPtr);

    if( linkInfo.sourceType != DATA_TYPE_SMPTE_TT ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_SMPTE_TT_ENC, "Attempt to add Sink that cannot process SMPTE-TT Data");
        return FALSE;
    }
    
    return AddSink(&rootCtxPtr->smpteEncodeCtxPtr->sinks, &linkInfo);
}  // SmpteEncodeAddSink()

/*------------------------------------------------------------------------------
 | NAME:
 |    SmpteEncodeProcNextBuffer()
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
 |    This method processes an incoming buffer, converting the CEA-608 to SMPTE-TT
 |    2052 Preserved Mode.
 -------------------------------------------------------------------------------*/
uint8 SmpteEncodeProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    ASSERT(inBuffer);
    ASSERT(inBuffer->dataPtr);
    ASSERT(inBuffer->numElements);
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->smpteEncodeCtxPtr);
    SmpteEncodeCtx* ctxPtr = ((Context*)rootCtxPtr)->smpteEncodeCtxPtr;
    Line21Code* line21CodePtr = (Line21Code*)inBuffer->dataPtr;

// TODO - For an example, I am going to allocate a buffer and pass
//        something to the next stage of the pipeline.

    Buffer* outBuffer = NewBuffer(BUFFER_TYPE_BYTES, 1024);
    outBuffer->captionTime = inBuffer->captionTime;
    *outBuffer->dataPtr = '\0';

    for( int loop = 0; loop < inBuffer->numElements; loop++ ) {

// TODO - This loop processes each caption code that is passed to it.
//        line21CodePtr[loop] has the data and it fits the structure
//        Line21Code.

        if( inBuffer->captionTime.source == CAPTION_TIME_FRAME_NUMBERING ) {
            sprintf((char *) &outBuffer->dataPtr[strlen((char *) outBuffer->dataPtr)],
                    "Timecode: %02d:%02d:%02d:%02d\n", inBuffer->captionTime.hour, inBuffer->captionTime.minute,
                    inBuffer->captionTime.second, inBuffer->captionTime.frame);
        } else {
            sprintf((char *) &outBuffer->dataPtr[strlen((char *) outBuffer->dataPtr)],
                    "Timecode: %02d:%02d:%02d,%03d\n", inBuffer->captionTime.hour, inBuffer->captionTime.minute,
                    inBuffer->captionTime.second, inBuffer->captionTime.millisecond);
        }

        switch( line21CodePtr[loop].codeType ) {
            case LINE21_NULL_DATA:
                sprintf( (char*)&outBuffer->dataPtr[strlen((char*)outBuffer->dataPtr)], "It's Null Data -- Chan: %d Field %d\n", line21CodePtr[loop].channelNum, line21CodePtr[loop].fieldNum );
                break;
            case LINE21_BASIC_CHARS:
                sprintf( (char*)&outBuffer->dataPtr[strlen((char*)outBuffer->dataPtr)], "Basic Characters -- Chan: %d Field %d\n", line21CodePtr[loop].channelNum, line21CodePtr[loop].fieldNum );
                break;
            case LINE21_SPECIAL_CHAR:
                sprintf( (char*)&outBuffer->dataPtr[strlen((char*)outBuffer->dataPtr)], "Special Characters -- Chan: %d Field %d\n", line21CodePtr[loop].channelNum, line21CodePtr[loop].fieldNum );
                break;
            case LINE21_EXTENDED_CHAR:
                sprintf( (char*)&outBuffer->dataPtr[strlen((char*)outBuffer->dataPtr)], "Extended Characters -- Chan: %d Field %d\n", line21CodePtr[loop].channelNum, line21CodePtr[loop].fieldNum );
                break;
            case LINE21_GLOBAL_CONTROL_CODE:
                sprintf( (char*)&outBuffer->dataPtr[strlen((char*)outBuffer->dataPtr)], "Global Control Code -- Chan: %d Field %d\n", line21CodePtr[loop].channelNum, line21CodePtr[loop].fieldNum );
                break;
            case LINE21_PREAMBLE_ACCESS_CODE:
                sprintf( (char*)&outBuffer->dataPtr[strlen((char*)outBuffer->dataPtr)], "PAC -- Chan: %d Field %d\n", line21CodePtr[loop].channelNum, line21CodePtr[loop].fieldNum );
                break;
            case LINE21_TAB_CONTROL_CODE:
                sprintf( (char*)&outBuffer->dataPtr[strlen((char*)outBuffer->dataPtr)], "Tab -- Chan: %d Field %d\n", line21CodePtr[loop].channelNum, line21CodePtr[loop].fieldNum );
                break;
            case LINE21_MID_ROW_CONTROL_CODE:
                sprintf( (char*)&outBuffer->dataPtr[strlen((char*)outBuffer->dataPtr)], "Midrow -- Chan: %d Field %d\n", line21CodePtr[loop].channelNum, line21CodePtr[loop].fieldNum );
                break;
            default:
                ASSERT(0);
                break;
        }
    }

    FreeBuffer(inBuffer);

    outBuffer->numElements = strlen((char*)outBuffer->dataPtr) + 1;
    ASSERT(outBuffer->numElements < 1024 );

    return PassToSinks(rootCtxPtr, outBuffer, &ctxPtr->sinks);
}  // SmpteEncodeProcNextBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21DecodeShutdown()
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
 |    that there is no more data coming down the pipeline. This element will
 |    perform any necessary actions as a result and pass this call down the
 |    pipeline.
 -------------------------------------------------------------------------------*/
uint8 SmpteEncodeShutdown( void* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->smpteEncodeCtxPtr);
    SmpteEncodeCtx* ctxPtr = ((Context*)rootCtxPtr)->smpteEncodeCtxPtr;
    Sinks sinks = ctxPtr->sinks;

// TODO - This is the end of the input file. Free any allocated memory
//        or print out any statistics you want at the end here.

    free(ctxPtr);
    ((Context*)rootCtxPtr)->smpteEncodeCtxPtr = NULL;

    return ShutdownSinks(rootCtxPtr, &sinks);
}  // SmpteEncodeShutdown()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/
