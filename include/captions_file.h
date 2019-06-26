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

#ifndef captions_file_h
#define captions_file_h

#include "types.h"
#include "pipeline_utils.h"
#include "context.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/

boolean MccFileInitialize( Context*, char* );
boolean SccFileInitialize( Context*, char*, uint32 );

boolean MccFileAddSink( Context*, LinkInfo );
boolean SccFileAddSink( Context*, LinkInfo );

boolean MccFileProcNextBuffer( Context*, boolean* );
boolean SccFileProcNextBuffer( Context*, boolean* );

#endif /* captions_file_h */
