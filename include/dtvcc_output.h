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

#ifndef dtvcc_output_h
#define dtvcc_output_h

#include <stdio.h>

#include "types.h"
#include "dtvcc_decode.h"
#include "pipeline_utils.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

#define DtvccDecodeC0Cmd(X)               (X<32) ? (normalizedC0CmdSetShortName[X]) : (normalizedC0CmdSetShortName[32])
#define DtvccDecodeC1Cmd(X)               ((X>0x7F) && (X<0xA0)) ? (normalizedC1CmdSetShortName[(X - 0x80)]) : (normalizedC1CmdSetShortName[32])
#define DECODE_TEXT_TAG(X)                (X<16) ? (textTags[X]) : (textTags[16])
#define DECODE_PEN_OFFSET(X)              (X<4) ? (penOffset[X]) : (penOffset[4])
#define DECODE_PEN_SIZE(X)                (X<4) ? (penSize[X]) : (penSize[4])
#define DECODE_EDGE_TYPE(X)               (X<8) ? (edgeType[X]) : (edgeType[8])
#define DECODE_FONT_TAG(X)                (X<8) ? (fontTag[X]) : (fontTag[8])
#define DECODE_OPACITY(X)                 (X<4) ? (opacity[X]) : (opacity[4])
#define DECODE_BORDER_TYPE(X)             (X<6) ? (borderType[X]) : (borderType[6])
#define DECODE_DIRECTION(X)               (X<4) ? (direction[X]) : (direction[4])
#define DECODE_JUSTIFY_DIRECTION(X)       (X<4) ? (justifyDirection[X]) : (justifyDirection[4])
#define DECODE_EFFECT_SPEED(X)            (X<16) ? (effectSpeed[X]) : (effectSpeed[16])
#define DECODE_DISPLAY_EFFECT(X)          (X<3) ? (dispEffect[X]) : (dispEffect[3])
#define DECODE_ANCOR_POSITION(X)          (X<9) ? (anchorPos[X]) : (anchorPos[9])
#define DECODE_PREDEF_PEN_STYLE(X)        (X<7) ? (predefPenStyle[X]) : (predefPenStyle[7])
#define DECODE_PREDEF_WIN_STYLE(X)        (X<7) ? (predefWinStyle[X]) : (predefWinStyle[7])

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/


extern const char* normalizedC0CmdSetShortName[33];
extern const char* normalizedC1CmdSetShortName[33];
extern const char* normalizedG0CharSet[96];
extern const char* normalizedG1CharSet[96];

extern const char* textTags[17];
extern const char* penOffset[5];
extern const char* penSize[5];
extern const char* predefPenStyle[8];
extern const char* edgeType[9];
extern const char* fontTag[9];
extern const char* opacity[5];
extern const char* borderType[7];
extern const char* direction[5];
extern const char* justifyDirection[5];
extern const char* effectSpeed[17];
extern const char* dispEffect[4];
extern const char* anchorPos[10];
extern const char* predefWinStyle[8];

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/

LinkInfo DtvccOutInitialize( Context*, char*, boolean, boolean );
boolean DtvccOutProcNextBuffer( void*, Buffer* );
boolean DtvccOutShutdown( void* );

const char* DtvccDecodeG0CharSet( uint8 );
const char* DtvccDecodeG1CharSet( uint8 );
const char* DtvccDecodeG2CharSet( uint8 );
const char* DtvccDecodeG3CharSet( uint8 );

#endif /* dtvcc_output_h */
