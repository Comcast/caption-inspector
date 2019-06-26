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

#ifndef pipeline_structs_h
#define pipeline_structs_h

#include "types.h"

#include "buffer_utils.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

#define MAX_NUMBER_OF_SINKS                             5

/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

typedef struct {
    uint8 linkType;
    uint8 sourceType;
    uint8 sinkType;
    boolean (*NextBufferFnPtr)(void*, Buffer*);
    boolean (*ShutdownFnPtr)(void*);
} LinkInfo;

typedef struct {
    uint8 linkType;
    uint8 numSinks;
    LinkInfo sink[MAX_NUMBER_OF_SINKS];
} Sinks;

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/

#endif /* pipeline_structs_h */
