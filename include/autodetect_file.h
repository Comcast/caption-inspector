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

#ifndef autodetect_file_h
#define autodetect_file_h

#include "types.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

// WARNING: These defines need to match the text in autodetect_file.c
typedef enum {
    UNK_CAPTIONS_FILE     = 0,
    SCC_CAPTIONS_FILE     = 1,
    MCC_CAPTIONS_FILE     = 2,
    MPEG_BINARY_FILE      = 3,
    MOV_BINARY_FILE       = 4,
    MAX_FILE_TYPE         = 5
} FileType;

/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

#define DECODE_CAPTION_FILE_TYPE(X)     (X < MAX_FILE_TYPE) ? (captionFileTypeStr[X]) : (captionFileTypeStr[0])

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/

extern const char* captionFileTypeStr[MAX_FILE_TYPE];

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/
FileType DetermineFileType( char* );
boolean DetermineDropFrame( char*, boolean, char*, boolean* );

#endif /* autodetect_file_h */
