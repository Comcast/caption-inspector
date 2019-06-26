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

#ifndef cc_utils_h
#define cc_utils_h

#include "mcc_decode.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

#define CEA608E_LINE21_FIELD_1_CC           0x00
#define CEA608E_LINE21_FIELD_2_CC           0x01
#define DTVCCC_CHANNEL_PACKET_DATA          0x02
#define DTVCCC_CHANNEL_PACKET_START         0x03

#define CC_CONSTR_CC_VALID_FLAG_MASK        0x04
#define CC_CONSTR_CC_VALID_FLAG_SET         0x04
#define CC_CONSTR_CC_TYPE_MASK              0x03

#define CC_COUNT_MASK                       0x1F

#define VALID_CEA608E_LINE21_FIELD_1_CC     0xFC
#define VALID_CEA608E_LINE21_FIELD_2_CC     0xFD
#define INVALID_DTVCCC_CHANNEL_PACKET_DATA  0xFA
#define CEA608_ZERO_WITH_ODD_PARITY         0x80
#define EMPTY_DTVCC_CHANNEL_PACKET_DATA     0x00

#define CAPTION_TIME_SCRATCH_BUFFER_SIZE      24

#define CC_COUNT_FRAME_RATE_24              25
#define CC_COUNT_FRAME_RATE_25              24
#define CC_COUNT_FRAME_RATE_30              20
#define CC_COUNT_FRAME_RATE_50              12
#define CC_COUNT_FRAME_RATE_60              10

/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/

void encodeTimeCode( CaptionTime*, char* );
boolean decodeTimeCode( char*, CaptionTime* );
boolean isHexByteValid( char, char );
uint8 byteFromNibbles( char, char );
void byteToAscii( uint8, uint8*, uint8* );
uint8 numCcConstructsFromFramerate( uint32 );
uint8 cdpFramerateFromFramerate( uint32 );
boolean isFramerateValid( uint32 frameRatePerSecTimesOneHundred );
void frameToTimeCode( uint32, uint32, CaptionTime* );
uint32 timeCodeToFrame( CaptionTime* );

#endif /* cc_utils_h */
