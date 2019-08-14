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
#include <string.h>

#include "debug.h"

#include "smpte_output.h"
#include "output_utils.h"

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
 |    SmpteOutInitialize()
 |
 | INPUT PARAMETERS:
 |    ctxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    outputFileNameStr - The name of the file to write the data to.
 |
 | RETURN VALUES:
 |    LinkInfo -  Information about this element of the pipeline, such that it can
 |                be chained to elements that can consume the specific type of data
 |                that it produces.
 |
 | DESCRIPTION:
 |    This initializes this element of the pipeline.
 -------------------------------------------------------------------------------*/
LinkInfo SmpteOutInitialize( Context* ctxPtr, char* outputFileNameStr ) {
    ASSERT(ctxPtr);
    ASSERT(!ctxPtr->smpteOutputCtxPtr);
    char tempFilename[MAX_FILE_NAME_LEN];

    strncpy(tempFilename, outputFileNameStr, MAX_FILE_NAME_LEN-1);
    tempFilename[MAX_FILE_NAME_LEN-1] = '\0';
    strncat(tempFilename, ".smt", (MAX_FILE_NAME_LEN - strlen(tempFilename) - 1));

    ctxPtr->smpteOutputCtxPtr = malloc(sizeof(SmpteOutputCtx));
    ctxPtr->smpteOutputCtxPtr->fp = NULL;
    strncpy(ctxPtr->smpteOutputCtxPtr->smpteFileName, tempFilename, MAX_FILE_NAME_LEN-1);

    LinkInfo linkInfo;
    linkInfo.linkType = SMPTE_TT_DATA___TEXT_FILE;
    linkInfo.sourceType = DATA_TYPE_SMPTE_TT;
    linkInfo.sinkType = DATA_TYPE_SMPTE_TT_TXT_FILE;
    linkInfo.NextBufferFnPtr = &SmpteOutProcNextBuffer;
    linkInfo.ShutdownFnPtr = &SmpteOutShutdown;
    return linkInfo;
}  // SmpteOutInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    SmpteOutProcNextBuffer()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Context Pointer.
 |    inBuffer - Pointer to the buffer to process.
 |
 | RETURN VALUES:
 |    uint8 - Success is TRUE / PIPELINE_SUCCESS, Failure is FALSE / PIPELINE_FAILURE
 |            All other codes specified in header.
 |
 | DESCRIPTION:
 |    This method processes an incoming buffer, writing the contents into an
 |    ASCII file.
 -------------------------------------------------------------------------------*/
uint8 SmpteOutProcNextBuffer( void* rootCtxPtr, Buffer* buffPtr ) {
    ASSERT(buffPtr);
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->smpteOutputCtxPtr);

    if( ((Context*)rootCtxPtr)->smpteOutputCtxPtr->fp == NULL ) {
        LOG(DEBUG_LEVEL_INFO, DBG_SMPTE_TT_OUT, "Creating new SMPTE-TT File for Output: %s", ((Context*)rootCtxPtr)->smpteOutputCtxPtr->smpteFileName);
        ((Context*)rootCtxPtr)->smpteOutputCtxPtr->fp = fileOutputInit(((Context*)rootCtxPtr)->smpteOutputCtxPtr->smpteFileName);
    }

    writeToFile(((Context*)rootCtxPtr)->smpteOutputCtxPtr->fp, "%s\n", buffPtr->dataPtr);
    FreeBuffer(buffPtr);
    return PIPELINE_SUCCESS;
} // SmpteOutProcNextBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    SmpteOutShutdown()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Context Pointer.
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
uint8 SmpteOutShutdown( void* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->smpteOutputCtxPtr);

    if( ((Context*)rootCtxPtr)->smpteOutputCtxPtr->fp != NULL ) {
        closeFile(((Context*)rootCtxPtr)->smpteOutputCtxPtr->fp);
    } else {
        LOG(DEBUG_LEVEL_ERROR, DBG_SMPTE_TT_OUT, "Nothing to place in SMPTE-TT File. File not created!");
    }

    free(((Context*)rootCtxPtr)->smpteOutputCtxPtr);
    ((Context*)rootCtxPtr)->smpteOutputCtxPtr = NULL;
    return PIPELINE_SUCCESS;
} // SmpteOutShutdown()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/
