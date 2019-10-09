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

#ifndef pipeline_utils_h
#define pipeline_utils_h

#include "types.h"
#include "context.h"
#include "autodetect_file.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

#define PIPELINE_FAILURE                                FALSE
#define PIPELINE_SUCCESS                                TRUE
#define FIRST_TEXT_FOUND                                2

#define DATA_TYPE_UNKNOWN                               0
#define DATA_TYPE_SCC_DATA                              1
#define DATA_TYPE_MCC_DATA                              2
#define DATA_TYPE_CC_DATA                               3
#define DATA_TYPE_SEI_DATA                              4
#define DATA_TYPE_DECODED_608                           5
#define DATA_TYPE_DECODED_708                           6
#define DATA_TYPE_CC_DATA_TXT_FILE                      7
#define DATA_TYPE_SEI_DATA_TXT_FILE                     8
#define DATA_TYPE_608_TXT_FILE                          9
#define DATA_TYPE_708_TXT_FILE                         10
#define DATA_TYPE_MCC_DATA_TXT_FILE                    11
#define DATA_TYPE_EXTERNAL_ADAPTOR                     12
#define MAX_DATA_TYPE                                  13

// WARNING: These defines need to match the text in pipeline_utils.c

// Sources:
#define UNKNOWN_LINK_TYPE                               0
#define SCC_FILE___SCC_DATA                             1
#define MCC_FILE___MCC_DATA                             2
#define MPG_FILE___CC_DATA                              3

// Transforms:
#define MCC_DATA___CC_DATA                              4
#define SCC_DATA___CC_DATA                              5
#define CC_DATA___MCC_DATA                              6
#define CC_DATA___LINE21_DATA                           7
#define CC_DATA___DTVCC_DATA                            8

// Sinks:
#define SEI_DATA___TEXT_FILE                            9
#define CC_DATA___TEXT_FILE                            10
#define MCC_DATA___TEXT_FILE                           11
#define LINE21_DATA___TEXT_FILE                        12
#define DTVCC_DATA___TEXT_FILE                         13
#define LINE21_DATA___EXTRNL_ADPTR                     14
#define DTVCC_DATA___EXTRNL_ADPTR                      15

#define MAX_LINK_TYPE                                  16

/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

#define PassToSinks(ctx, buf, snk) _PassToSinks(__FILE__, __LINE__, ctx, buf, snk)
#define ShutdownSinks(ctx, snk) _ShutdownSinks(__FILE__, __LINE__, ctx, snk)

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/

void InitSinks(Sinks*, uint8);
boolean AddSink(Sinks*, LinkInfo*);
uint8 _PassToSinks(char*, int, Context*, Buffer*, Sinks*);
uint8 _ShutdownSinks(char*, int, Context*, Sinks*);
boolean PlumbSccPipeline(Context*, char*, char*, uint32, boolean);
boolean PlumbMccPipeline(Context*, char*, char*, boolean);
boolean PlumbMpegPipeline(Context*, char*, char*, boolean, char*, uint8);
boolean PlumbMovPipeline(Context*, char*, char*, boolean, char*, uint8);
void DrivePipeline(FileType, Context*);

#endif /* pipeline_utils_h */
