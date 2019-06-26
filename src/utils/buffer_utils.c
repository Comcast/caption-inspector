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

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "buffer_utils.h"
#include "line21_decode.h"
#include "dtvcc_decode.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

static BufferElement* bufferArray[MAX_BUFFER_ARRAY_SIZE];

static const char* BufferTypeText[MAX_BUFFER_TYPE] = {
    "BUFFER_TYPE_UNKNOWN",
    "BUFFER_TYPE_BYTES",
    "BUFFER_TYPE_LINE_21",
    "BUFFER_TYPE_DTVCC"
};

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/
void dbgdumpBufferPool( uint8 );

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    BufferPoolInit()
 |
 | INPUT PARAMETERS:
 |    None.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method initializes the buffer pool.
 -------------------------------------------------------------------------------*/
void BufferPoolInit( void ) {
    for( int loop = 0; loop < MAX_BUFFER_ARRAY_SIZE; loop++ ) {
        bufferArray[loop] = NULL;
    }
}  // BufferPoolInit()

/*------------------------------------------------------------------------------
 | NAME:
 |    NewBuffer()/_NewBuffer()
 |
 | INPUT PARAMETERS:
 |    fileNameStr - The name of the calling function.
 |    lineNum - The line number of the calling function.
 |    bufferType - The type of buffer requested.
 |    size - The size of buffer requested (dependent upon type).
 |
 | RETURN VALUES:
 |    The Caption File Object, which contains metadata about the file, including
 |    the FILE pointer.
 |
 | DESCRIPTION:
 |    This method opens the Scenarist Closed Captioning (SCC) or MacCaption (MCC)
 |    Closed Captioning File. It will parse the header and in the case of MCC
 |    extract the metadata about the file.
 -------------------------------------------------------------------------------*/
Buffer* _NewBuffer( char* fileNameStr, int lineNum, uint8 bufferType, uint16 size ) {
    ASSERT(size);
    ASSERT(bufferType);
    ASSERT(bufferType < MAX_BUFFER_TYPE);
    
    char* basename = fileNameStr;
    basename = strrchr(fileNameStr, '/');
    basename = basename ? basename+1 : fileNameStr;
    
    Buffer* newBuffer = malloc(sizeof(Buffer));
    if( newBuffer == NULL ) LOG(DEBUG_LEVEL_FATAL, DBG_BUFFER, "Malloc Failed");
    LOG(DEBUG_LEVEL_VERBOSE, DBG_BUFFER, "Buffer [%p] Allocated by {%s:%d} %s - %d", newBuffer, basename, lineNum, BufferTypeText[bufferType], size);

    newBuffer->numElements = 0;
    newBuffer->maxNumElements = size;
    newBuffer->bufferType = bufferType;
    
    switch( bufferType ) {
        case BUFFER_TYPE_BYTES:
            newBuffer->dataPtr = malloc(newBuffer->maxNumElements);
            break;
        case BUFFER_TYPE_LINE_21:
            newBuffer->dataPtr = malloc(newBuffer->maxNumElements * sizeof(Line21Code));
            break;
        case BUFFER_TYPE_DTVCC:
            newBuffer->dataPtr = malloc(newBuffer->maxNumElements * sizeof(DtvccData));
            break;
        default:
            LOG(DEBUG_LEVEL_FATAL, DBG_BUFFER, "Unreachable Branch %d", bufferType);
    }
    if( newBuffer->dataPtr == NULL ) LOG(DEBUG_LEVEL_FATAL, DBG_BUFFER, "Malloc Failed");

    newBuffer->captionTime.hour = 0;
    newBuffer->captionTime.minute = 0;
    newBuffer->captionTime.second = 0;
    newBuffer->captionTime.millisecond = 0;
    newBuffer->captionTime.frame = 0;
    newBuffer->captionTime.frameRatePerSecTimesOneHundred = 0;
    newBuffer->captionTime.dropframe = FALSE;

    BufferElement* listElementPtr = malloc(sizeof(BufferElement));
    if( listElementPtr == NULL ) LOG(DEBUG_LEVEL_FATAL, DBG_BUFFER, "Malloc Failed");
    listElementPtr->bufferPtr = newBuffer;
    listElementPtr->numReaders = 0;

    strncpy(listElementPtr->callerFileName, basename, MAX_CALLER_FILE_NAME_SIZE);
    listElementPtr->callerFileName[MAX_CALLER_FILE_NAME_SIZE-1] = '\0';
    listElementPtr->callerFileLine = lineNum;
    
    for( int loop = 0; loop < MAX_BUFFER_ARRAY_SIZE; loop++ ) {
        if( bufferArray[loop] == NULL ) {
            bufferArray[loop] = listElementPtr;
            return newBuffer;
        }
    }

    dbgdumpBufferPool( DEBUG_LEVEL_ERROR );
    LOG(DEBUG_LEVEL_FATAL, DBG_BUFFER, "Unable to add new buffer to array");

    return newBuffer;
}  // _NewBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    FreeBuffer()
 |
 | INPUT PARAMETERS:
 |    bufferToFreePtr - Pointer to the buffer to be freed.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method checks to see if anyone else is reading the buffer. If there
 |    are no other readers the buffer is freed. If there are other readers, this
 |    reader is removed.
 -------------------------------------------------------------------------------*/
void FreeBuffer( Buffer* bufferToFreePtr ) {
    ASSERT(bufferToFreePtr);
    
    for( int loop = 0; loop < MAX_BUFFER_ARRAY_SIZE; loop++ ) {
        if( (bufferArray[loop] != NULL) && (bufferArray[loop]->bufferPtr == bufferToFreePtr) ) {
            LOG(DEBUG_LEVEL_VERBOSE, DBG_BUFFER, "Attempt to free Buffer [%p] with %d readers.", bufferToFreePtr, bufferArray[loop]->numReaders);
            
            if( bufferArray[loop]->numReaders > 1 ) {
                bufferArray[loop]->numReaders = bufferArray[loop]->numReaders - 1;
                LOG(DEBUG_LEVEL_VERBOSE, DBG_BUFFER, "Decremented Readers for Buffer [%p] now %d", bufferToFreePtr, bufferArray[loop]->numReaders);
                return;
            }
            
            LOG(DEBUG_LEVEL_VERBOSE, DBG_BUFFER, "Buffer [%p] Freed", bufferToFreePtr);
            
            free(bufferArray[loop]);
            bufferArray[loop] = NULL;
            
            ASSERT(bufferToFreePtr->dataPtr);
            free(bufferToFreePtr->dataPtr);
            free(bufferToFreePtr);
            return;
        }
    }
    LOG(DEBUG_LEVEL_ERROR, DBG_BUFFER, "Unable to find or free Buffer [%p]", bufferToFreePtr );
    
}  // FreeBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    AddReader()
 |
 | INPUT PARAMETERS:
 |    buffPtr - Pointer to the buffer which needs an additional reader.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method reserves the buffer for an additional reader, ensuring that
 |    we do not actually free the buffer until no one is reading it.
 -------------------------------------------------------------------------------*/
void AddReader( Buffer* buffPtr ) {
    ASSERT(buffPtr);
    
    for( int loop = 0; loop < MAX_BUFFER_ARRAY_SIZE; loop++ ) {
        if( (bufferArray[loop] != NULL) && (bufferArray[loop]->bufferPtr == buffPtr) ) {
            bufferArray[loop]->numReaders = bufferArray[loop]->numReaders + 1;
            LOG(DEBUG_LEVEL_VERBOSE, DBG_BUFFER, "Reader added to Buffer [%p] - Total %d", buffPtr, bufferArray[loop]->numReaders);
            return;
        }
    }
    
    LOG(DEBUG_LEVEL_ERROR, DBG_BUFFER, "Unable to find Buffer %p", buffPtr);
} // AddReader()

/*------------------------------------------------------------------------------
 | NAME:
 |    NumAllocatedBuffers()
 |
 | INPUT PARAMETERS:
 |    None.
 |
 | RETURN VALUES:
 |    uint8 - Number of Buffers that are not freed
 |
 | DESCRIPTION:
 |    This method initializes the buffer pool.
 -------------------------------------------------------------------------------*/
uint8 NumAllocatedBuffers( void ) {
    uint8 retval = 0;
    
    for( int loop = 0; loop < MAX_BUFFER_ARRAY_SIZE; loop++ ) {
        if( bufferArray[loop] != NULL ) {
            retval = retval + 1;
        }
    }
    return retval;
}  // NumAllocatedBuffers()

/*------------------------------------------------------------------------------
 | NAME:
 |    CaptionTimeFromPts()
 |
 | INPUT PARAMETERS:
 |    int64 - The Presentation Timestamp
 |
 | RETURN VALUES:
 |    captionTimePtr - Updated Caption Value, Referenced by this Pointer
 |
 | DESCRIPTION:
 |    This method converts the PTS into our internal Caption Time Structure.
 -------------------------------------------------------------------------------*/
void CaptionTimeFromPts( CaptionTime* captionTimePtr, int64 pts ) {

    ASSERT(((pts / (1000*60*60)) / 24) == 0);
    ASSERT(pts >= 0);
    captionTimePtr->hour = (uint8)((pts / (1000*60*60)) % 24);
    captionTimePtr->minute = (uint8)((pts / (1000*60)) % 60);
    captionTimePtr->second = (uint8)((pts / 1000) % 60);
    captionTimePtr->millisecond = (uint16)(pts % 1000);
    captionTimePtr->source = CAPTION_TIME_PTS_NUMBERING;

}  // CaptionTimeFromPts()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    dbgdumpBufferPool()
 |
 | DESCRIPTION:
 |    This function logs buffer pool related information.
 -------------------------------------------------------------------------------*/
void dbgdumpBufferPool( uint8 level ) {

    LOG(level, DBG_BUFFER, "Dumping all Buffers:");

    for( int loop = 0; loop < MAX_BUFFER_ARRAY_SIZE; loop++ ) {
        if( bufferArray[loop] != NULL ) {
            LOG(level, DBG_BUFFER, "%d) Buffer [%p] - Allocated [%s:%d]", loop, bufferArray[loop]->bufferPtr,
                bufferArray[loop]->callerFileName, bufferArray[loop]->callerFileLine);
            LOG(level, DBG_BUFFER, " - Number of Readers = %d", bufferArray[loop]->numReaders);
            if( bufferArray[loop]->bufferPtr == NULL ) {
                LOG(DEBUG_LEVEL_ERROR, DBG_BUFFER, " - No Buffer Pointer!");
            } else {
                if( bufferArray[loop]->bufferPtr->captionTime.dropframe == TRUE ) {
                    LOG(level, DBG_BUFFER, " - Timestamp = %02d:%02d:%02d;%02d",
                        bufferArray[loop]->bufferPtr->captionTime.hour,
                        bufferArray[loop]->bufferPtr->captionTime.minute,
                        bufferArray[loop]->bufferPtr->captionTime.second,
                        bufferArray[loop]->bufferPtr->captionTime.frame);
                } else {
                    LOG(level, DBG_BUFFER, " - Timestamp = %02d:%02d:%02d:%02d",
                        bufferArray[loop]->bufferPtr->captionTime.hour,
                        bufferArray[loop]->bufferPtr->captionTime.minute,
                        bufferArray[loop]->bufferPtr->captionTime.second,
                        bufferArray[loop]->bufferPtr->captionTime.frame);
                }
                LOG(level, DBG_BUFFER, " - Framerate = %d.%d fps",
                    (bufferArray[loop]->bufferPtr->captionTime.frameRatePerSecTimesOneHundred / 100),
                    (bufferArray[loop]->bufferPtr->captionTime.frameRatePerSecTimesOneHundred % 100));
                LOG(level, DBG_BUFFER, " - Type = %s", BufferTypeText[bufferArray[loop]->bufferPtr->bufferType]);
                LOG(level, DBG_BUFFER, " - Elements = [%d of %d]", bufferArray[loop]->bufferPtr->numElements,
                    bufferArray[loop]->bufferPtr->maxNumElements);
            }
        } else {
            LOG(level, DBG_BUFFER, "Buffer[%d]\n  - Empty", loop);
        }
    }
}
