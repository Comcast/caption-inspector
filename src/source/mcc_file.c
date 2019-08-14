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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

#include "debug.h"

#include "captions_file.h"
#include "cc_utils.h"
#include "pipeline_utils.h"
#include "buffer_utils.h"

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
 |    MccFileInitialize()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    fileNameStr - The name of the source file.
 |
 | RETURN VALUES:
 |    boolean - TRUE is Successful and FALSE is a Failure
 |
 | DESCRIPTION:
 |    This initializes the Caption File Source part of the pipeline. It will
 |    determine the type of file and report that back to the caller so that the
 |    appropriate pipeline can be determined and plumbed.
 -------------------------------------------------------------------------------*/
boolean MccFileInitialize( Context* rootCtxPtr, char* fileNameStr ) {
    char* line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    long pos = 0;
    boolean captionsStarted = FALSE;

    ASSERT(rootCtxPtr);
    ASSERT(!rootCtxPtr->mccFileCtxPtr);

    rootCtxPtr->mccFileCtxPtr = malloc(sizeof(MccFileCtx));
    MccFileCtx* ctxPtr = rootCtxPtr->mccFileCtxPtr;

    ctxPtr->numCaptionsLinesRead = 0;
    ctxPtr->captionsFilePtr = NULL;
    ctxPtr->frameRateTimesOneHundred = 2400;
    ctxPtr->isDropFrame = FALSE;
    ctxPtr->oneShotWarningFlag = FALSE;

    ctxPtr->numCaptionsLinesRead = 0;
    ASSERT(fileNameStr);
    ctxPtr->captionsFilePtr = fopen(fileNameStr, "r");
    
    if( ctxPtr->captionsFilePtr == NULL ) {
        char filePath[PATH_MAX];
        filePath[0] = '\0';
        getcwd(filePath, sizeof(filePath));
        LOG(DEBUG_LEVEL_FATAL, DBG_FILE_IN, "Unable to Open File: %s (looking here: %s) - [Errno %d] %s", fileNameStr, filePath, errno, strerror(errno));
        free(ctxPtr);
        rootCtxPtr->mccFileCtxPtr = NULL;
        return FALSE;
    }
    
    strncpy(ctxPtr->captionFileName, fileNameStr, MAX_FILE_NAME_LEN);
    ctxPtr->captionFileName[MAX_FILE_NAME_LEN-1] = '\0';

    InitSinks(&ctxPtr->sinks, MCC_FILE___MCC_DATA);

    while( captionsStarted == FALSE ) {
        pos = ftell(ctxPtr->captionsFilePtr);
        read = getline(&line, &len, ctxPtr->captionsFilePtr);
        
        if( strncmp(line, "Scenarist_SCC V1.0", strlen("Scenarist_SCC V1.0")) == 0 ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_FILE_IN, "Spurious line from an SCC File");
            fclose(ctxPtr->captionsFilePtr);
            free(ctxPtr);
            rootCtxPtr->mccFileCtxPtr = NULL;
            return FALSE;
        } else if( strncmp(line, "File Format=MacCaption_MCC V", strlen("File Format=MacCaption_MCC V")) == 0 ) {
            continue;
        } else if( strncmp(line, "0 PREFIX 39", strlen("0 PREFIX 39")) == 0 ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_FILE_IN, "Spurious line from an encoded SEI File");
            fclose(ctxPtr->captionsFilePtr);
            free(ctxPtr);
            rootCtxPtr->mccFileCtxPtr = NULL;
            return FALSE;
        } else if( strncmp(line, "File Format=Comcast CC Data File", strlen("File Format=Comcast CC Data File")) == 0 ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_FILE_IN, "Spurious line from a CC Data File");
            fclose(ctxPtr->captionsFilePtr);
            free(ctxPtr);
            rootCtxPtr->mccFileCtxPtr = NULL;
            return FALSE;
        } else if( (strncmp(line, "//", 2) == 0) || (read < 5) ) {
            continue;
        } else if( strncmp(line, "UUID=", strlen("UUID=")) == 0 ) {
            continue;
        } else if( strncmp(line, "Creation Program=", strlen("Creation Program=")) == 0 ) {
            continue;
        } else if( strncmp(line, "Creation Date=", strlen("Creation Date=")) == 0 ) {
            continue;
        } else if( strncmp(line, "Creation Time=", strlen("Creation Time=")) == 0 ) {
            continue;
        } else if( strncmp(line, "Time Code Rate=", strlen("Time Code Rate=")) == 0 ) {
            char* tmpPtr = &line[(strlen("Time Code Rate="))];
            
            ctxPtr->frameRateTimesOneHundred = (tmpPtr[0] - '0') * 1000;
            ctxPtr->frameRateTimesOneHundred = ctxPtr->frameRateTimesOneHundred + ((tmpPtr[1] - '0') * 100);
            
            LOG(DEBUG_LEVEL_INFO, DBG_FILE_IN, "Frame Rate: %d.%d", (ctxPtr->frameRateTimesOneHundred / 100), (ctxPtr->frameRateTimesOneHundred % 100) );
            
            if( (tmpPtr[2] == 'D') && (tmpPtr[3] == 'F') ) {
                ctxPtr->isDropFrame = TRUE;
            }
        } else {
            captionsStarted = TRUE;
            fseek(ctxPtr->captionsFilePtr,pos,0);
        }
    }
    
    return TRUE;
}  // MccFileInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    MccFileAddSink()
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
boolean MccFileAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    ASSERT(rootCtxPtr);
    ASSERT(rootCtxPtr->mccFileCtxPtr);

    if( linkInfo.sourceType != DATA_TYPE_MCC_DATA ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_FILE_IN, "Can't Add Incompatible Sink %d", linkInfo.sourceType);
        return FALSE;
    }

    return AddSink(&rootCtxPtr->mccFileCtxPtr->sinks, &linkInfo);
}  // MccFileAddSink()

/*------------------------------------------------------------------------------
 | NAME:
 |    MccFileProcNextBuffer()
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
uint8 MccFileProcNextBuffer( Context* rootCtxPtr, boolean* isDonePtr ) {
    char* line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    
    ASSERT(isDonePtr);
    ASSERT(rootCtxPtr);
    ASSERT(rootCtxPtr->mccFileCtxPtr);

    MccFileCtx* ctxPtr = rootCtxPtr->mccFileCtxPtr;

    if( ctxPtr->sinks.numSinks == 0 ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_FILE_IN, "Can't Process Buffers with no Sinks");
        return FALSE;
    }
    
    if( ctxPtr->captionsFilePtr == NULL ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_FILE_IN, "Unable to read from NULL file");
        return FALSE;
    }
    
    while( line == NULL ) {
        read = getline(&line, &len, ctxPtr->captionsFilePtr);
        if( read == -1 ) {
            LOG(DEBUG_LEVEL_INFO, DBG_FILE_IN, "Reached End of Captions File");
            LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Captions File: %s contained %d lines of caption data", ctxPtr->captionFileName, (ctxPtr->numCaptionsLinesRead+1));
            fclose(ctxPtr->captionsFilePtr);
            Sinks sinks = ctxPtr->sinks;
            free(ctxPtr);
            rootCtxPtr->mccFileCtxPtr = NULL;
            *isDonePtr = TRUE;
            return ShutdownSinks(rootCtxPtr, &sinks);
        } else if( read < 5 ) {
            line = NULL;
        }
    }

    char* timecode = strtok(line, "\t");
    char* mccdata = strtok(NULL, "\t");

    ASSERT(timecode);
    ASSERT(mccdata);

// TODO - This is a kludge! Fully support the MCC 2.0 Stuff and remove this
    if( (strlen(timecode) > 11) && (timecode[11] == '.') ) {
        if( ctxPtr->oneShotWarningFlag == FALSE ) {
            ctxPtr->oneShotWarningFlag = TRUE;
            LOG(DEBUG_LEVEL_WARN, DBG_FILE_IN, "Detected MCC 2.0. Handling file (in a kludgy way), but MCC 2.0 is not fully supported.");
        }
        return TRUE;
    }

    ASSERT(strlen(timecode) == 11);
    uint16 bufferSize = strlen(mccdata);
    Buffer* newBufferPtr = NewBuffer( BUFFER_TYPE_BYTES, bufferSize );
    newBufferPtr->numElements = bufferSize;
    memcpy(newBufferPtr->dataPtr, mccdata, bufferSize);

    boolean wasSuccessful = decodeTimeCode( timecode, &newBufferPtr->captionTime );
    if( wasSuccessful == FALSE ) {
        LOG(DEBUG_LEVEL_WARN, DBG_FILE_IN, "Unable to parse timecode on line: %s", line);
        FreeBuffer(newBufferPtr);
        return TRUE;
    }

    newBufferPtr->captionTime.frameRatePerSecTimesOneHundred = ctxPtr->frameRateTimesOneHundred;

    if( ctxPtr->isDropFrame == TRUE ) {
        newBufferPtr->captionTime.dropframe = TRUE;
    }

    LOG(DEBUG_LEVEL_VERBOSE, DBG_FILE_IN,"Raw MCC - %s", mccdata);
    ctxPtr->numCaptionsLinesRead = ctxPtr->numCaptionsLinesRead + 1;

    LOG(DEBUG_LEVEL_VERBOSE, DBG_GENERAL, "Captions File: %s Line: %d - Processed %d bytes of mcc caption data",
        ctxPtr->captionFileName, ctxPtr->numCaptionsLinesRead, newBufferPtr->numElements);

    return PassToSinks(rootCtxPtr, newBufferPtr, &ctxPtr->sinks);
    
} // MccFileProcNextBuffer()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

