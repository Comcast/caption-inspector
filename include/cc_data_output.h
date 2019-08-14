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

#ifndef cc_data_output_h
#define cc_data_output_h

#include "types.h"
#include "pipeline_utils.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/
#define UNKOWN_CHANNEL                                           0
#define UNKNOWN_SERVICE                                          0
#define CC_DATA_TYPE__FIELD_1                                    0
#define CC_DATA_TYPE__FIELD_2                                    1
#define CC_DATA_TYPE__PACKET_DATA                                2
#define CC_DATA_TYPE__PACKET_START                               3

#define CC_DATA_ELEMENT_HEX_STR_SIZE                            10
#define CC_DATA_ELEMENT_TAG_STR_SIZE                            10
#define CC_DATA_ELEMENT_HALF_TAG_STR_SIZE                        5
#define CC_DATA_ELEMENT_DEC_STR_SIZE                            18
#define CC_DATA_ELEMENT_HALF_DEC_STR_SIZE                        9

#define NUM_CC_DATA_ELEMENTS_PER_LINE                            5

#define NUM_608_CHANNELS                                         4
#define NUM_708_SERVICES                                        16
#define MAX_CHARS_PER_LINE_PER_CHANNEL                          64

#define CEA708_ERROR_STR_SIZE                                   25

#define CEA708_STATE_UNKNOWN                                     0
#define CEA708_STATE_EXTENDED_SEQ_NUM                            1
#define CEA708_STATE_DATA_WAIT                                   2
#define CEA708_STATE_C0_CODE                                     3
#define CEA708_STATE_C1_CODE                                     4
#define CEA708_STATE_EXTENDED_CODE                               5

#define CEA708_CODE_UNKNOWN                                      0
#define CEA708_CODE_UNKNOWN_C0_1                                 1
#define CEA708_CODE_UNKNOWN_C0_2                                 2
#define CEA708_CODE_P16_C0                                       3
#define CEA708_CODE_EXTENDED_CODE_UNKNOWN                        4
#define CEA708_CODE_C2                                           5
#define CEA708_CODE_G2                                           6
#define CEA708_CODE_C3                                           7
#define CEA708_CODE_G3                                           8
#define CEA708_CODE_C1_BITMAP                                    9
#define CEA708_CODE_C1_DLY                                      10
#define CEA708_CODE_C1_SPA                                      11
#define CEA708_CODE_C1_SPC                                      12
#define CEA708_CODE_C1_SPL                                      13
#define CEA708_CODE_C1_SWA                                      14
#define CEA708_CODE_C1_DFW                                      15


/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

typedef struct {
    char hexStr[CC_DATA_ELEMENT_HEX_STR_SIZE];
    char tagStr[CC_DATA_ELEMENT_TAG_STR_SIZE];
    char decStr[CC_DATA_ELEMENT_DEC_STR_SIZE];
} CcdElemOut;

typedef struct {
    char txtStr608[NUM_608_CHANNELS][MAX_CHARS_PER_LINE_PER_CHANNEL];
    char txtStr708[NUM_708_SERVICES][MAX_CHARS_PER_LINE_PER_CHANNEL];
} TextString;

typedef struct {
    uint8 numElements;
    CcdElemOut element[NUM_CC_DATA_ELEMENTS_PER_LINE];
    TextString txtStr;
} CcdLineOut;

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/

LinkInfo CcDataOutInitialize( Context*, char* );
uint8 CcDataOutProcNextBuffer( void*, Buffer* );
uint8 CcDataOutShutdown( void* );

#endif /* cc_data_output_h */
