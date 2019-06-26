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

#ifndef external_adaptor_h
#define external_adaptor_h

#include "line21_decode.h"
#include "dtvcc_decode.h"
#include "pipeline_utils.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                                Types                                   --*/
/*----------------------------------------------------------------------------*/

typedef void (*EXTERNAL_608_FN_PTR)(CaptionTime, Line21Code);
typedef void (*EXTERNAL_708_FN_PTR)(CaptionTime, DtvccData);
typedef void (*EXTERNAL_END_FN_PTR)(void);

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

boolean ExtrnlAdptrInitialize( EXTERNAL_608_FN_PTR, EXTERNAL_708_FN_PTR, EXTERNAL_END_FN_PTR );
boolean ExtrnlAdptr608OutProcNextBuffer( void*, Buffer* );
boolean ExtrnlAdptr708OutProcNextBuffer( void*, Buffer* );
boolean ExtrnlAdptrShutdown( void* );
boolean ExtrnlAdptrPlumbFileDecodePipeline( char*, uint32 );
boolean ExtrnlAdptrPlumbSccPipeline(char*, char*, uint32);
boolean ExtrnlAdptrPlumbMccPipeline(char*, char*);
boolean ExtrnlAdptrPlumbMpCorePipeline(char*, char*, uint32);
boolean ExtrnlAdptrPlumbMpegPipeline(char*, char*, boolean, char*);
boolean ExtrnlAdptrPlumbMovPipeline(char*, char*, boolean, char*);
void ExtrnlAdptrDriveDecodePipeline( void );

#endif /* external_adaptor_h */
