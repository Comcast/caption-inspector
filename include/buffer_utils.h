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

#ifndef buffer_utils_h
#define buffer_utils_h

#include "types.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

#define MAX_CALLER_FILE_NAME_SIZE                20
#define MAX_BUFFER_ARRAY_SIZE                    10

#define BUFFER_TYPE_UNKNOWN                       0
#define BUFFER_TYPE_BYTES                         1
#define BUFFER_TYPE_LINE_21                       2
#define BUFFER_TYPE_DTVCC                         3
#define MAX_BUFFER_TYPE                           4

#define CAPTION_TIME_SOURCE_UNKNOWN               0
#define CAPTION_TIME_FRAME_NUMBERING              1
#define CAPTION_TIME_PTS_NUMBERING                2

/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

typedef struct {
    uint8 hour;
    uint8 minute;
    uint8 second;
    uint8 frame;
    uint16 millisecond;
    boolean dropframe;
    uint32 frameRatePerSecTimesOneHundred;
    uint8 source;
} CaptionTime;

typedef struct {
    uint8 bufferType;
    CaptionTime captionTime;
    uint8* dataPtr;
    uint16 numElements;
    uint16 maxNumElements;
} Buffer;

typedef struct {
    char callerFileName[MAX_CALLER_FILE_NAME_SIZE];
    int callerFileLine;
    Buffer* bufferPtr;
    uint8 numReaders;
} BufferElement;

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

#define NewBuffer(bt, sz) _NewBuffer(__FILE__, __LINE__, bt, sz)

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/

void BufferPoolInit( void );
Buffer* _NewBuffer( char*, int, uint8, uint16 );
void FreeBuffer( Buffer* );
void AddReader( Buffer* );
uint8 NumAllocatedBuffers( void );
void CaptionTimeFromPts( CaptionTime*, int64 );

#endif /* buffer_utils_h */
