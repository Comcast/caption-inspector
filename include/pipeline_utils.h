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
#define DATA_TYPE_SMPTE_TT                              7
#define DATA_TYPE_CC_DATA_TXT_FILE                      8
#define DATA_TYPE_SEI_DATA_TXT_FILE                     9
#define DATA_TYPE_608_TXT_FILE                         10
#define DATA_TYPE_708_TXT_FILE                         11
#define DATA_TYPE_MCC_DATA_TXT_FILE                    12
#define DATA_TYPE_SMPTE_TT_TXT_FILE                    13
#define DATA_TYPE_EXTERNAL_ADAPTOR                     14
#define MAX_DATA_TYPE                                  15

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
#define LINE21_DATA___SMPTE_TT_DATA                     9

// Sinks:
#define SEI_DATA___TEXT_FILE                           10
#define CC_DATA___TEXT_FILE                            11
#define MCC_DATA___TEXT_FILE                           12
#define LINE21_DATA___TEXT_FILE                        13
#define DTVCC_DATA___TEXT_FILE                         14
#define SMPTE_TT_DATA___TEXT_FILE                      15
#define LINE21_DATA___EXTRNL_ADPTR                     16
#define DTVCC_DATA___EXTRNL_ADPTR                      17

#define MAX_LINK_TYPE                                  18

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
boolean PlumbSccPipeline(Context*, char*, char*);
boolean PlumbMccPipeline(Context*, char*, char*);
boolean PlumbMpegPipeline(Context*, char*, char*);
boolean PlumbMovPipeline(Context*, char*, char*);
void DrivePipeline(FileType, Context*);

#endif /* pipeline_utils_h */
