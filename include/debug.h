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

#ifndef debug_h
#define debug_h

#include "types.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

// WARNING: These defines need to match the text in debug.c and cshim.py
#define UNKNOWN_DEBUG_LEVEL         0
#define DEBUG_LEVEL_VERBOSE         1
#define DEBUG_LEVEL_INFO            2
#define DEBUG_LEVEL_WARN            3
#define DEBUG_LEVEL_ERROR           4
#define DEBUG_LEVEL_ASSERT          5
#define DEBUG_LEVEL_FATAL           6
#define MAX_DEBUG_LEVEL             7

#define UNKNOWN_DEBUG_SECTION       0
#define DBG_GENERAL                 1
#define DBG_PIPELINE                2
#define DBG_BUFFER                  3
#define DBG_FILE_IN                 4
#define DBG_FILE_OUT                5
#define DBG_FF_MPEG                 6
#define DBG_MPEG_FILE               7
#define DBG_GPAC_MP4                8
#define DBG_MOV_FILE                9
#define DBG_MCC_DEC                 10
#define DBG_MCC_ENC                 11
#define DBG_SCC_ENC                 12
#define DBG_SMPTE_TT_ENC            13
#define DBG_608_DEC                 14
#define DBG_708_DEC                 15
#define DBG_608_OUT                 16
#define DBG_708_OUT                 17
#define DBG_CCD_OUT                 18
#define DBG_SMPTE_TT_OUT            19
#define DBG_EXT_ADPT                20
#define MAX_DEBUG_SECTION           21

/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                                Types                                   --*/
/*----------------------------------------------------------------------------*/

typedef void (*EXTERNAL_DBG_FN_PTR)(char*, int, uint8, uint8, char*);

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

#define LOG(dl, ds, ...) DebugLog(dl, ds, __FILE__, __LINE__, __VA_ARGS__)
#define ASSERT(x) if(!(x)) DebugLog(DEBUG_LEVEL_ASSERT, DBG_GENERAL, __FILE__, __LINE__, "ASSERT FAILED!")

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/

extern const char* DebugLevelText[MAX_DEBUG_LEVEL];
extern const char* DebugSectionText[MAX_DEBUG_SECTION];

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/

void DebugInit( boolean, char*, EXTERNAL_DBG_FN_PTR );
void SetStdoutDebugLevel( uint8 );
void SetMinDebugLevel( uint8 );
uint8 GetMinDebugLevel( uint8 );
boolean IsDebugInitialized( void );
boolean IsDebugExternallyRouted( void );
boolean DebugShutdown( void );

void DebugLog( uint8, uint8, char*, int, ... );

#endif /* debug_h */
