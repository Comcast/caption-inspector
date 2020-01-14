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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "debug.h"
#include "output_utils.h"
#include "context.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

const char* trueFalseStr[2] = { "False", "True" };

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

static uint8 numFilePtrs = 0;
static FILE* fpArray[MAX_NUM_OUTPUT_FILES];

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    fileOutputInit()
 |
 | INPUT PARAMETERS:
 |    outputFileNameStr - Name of Output File.
 |
 | RETURN VALUES:
 |    File Pointer or NULL if no file was opened.
 |
 | DESCRIPTION:
 |    This method opens a file for output and keeps track of it.
 |
 -------------------------------------------------------------------------------*/
FILE* fileOutputInit( char* outputFileNameStr ) {
    FILE* fp = NULL;
    
    if( outputFileNameStr == NULL ) {
        return NULL;
    }
    
    if( numFilePtrs == 0 ) {
        for( uint8 loop = 0; loop < MAX_NUM_OUTPUT_FILES; loop++ ) {
            fpArray[loop] = NULL;
        }
    }
    
    if( numFilePtrs == MAX_NUM_OUTPUT_FILES ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_FILE_OUT, "Attempted to open too many files: %d", numFilePtrs );
        return NULL;
    }
    
    fp = fopen( outputFileNameStr, "w" );
    if( fp == NULL ) LOG(DEBUG_LEVEL_FATAL, DBG_FILE_OUT, "Unable to Open File: %s - [Errno %d] %s", outputFileNameStr, errno, strerror(errno));
    fpArray[numFilePtrs] = fp;
    numFilePtrs++;

    return fp;
}  // FileOutputInit()

/*------------------------------------------------------------------------------
 | NAME:
 |    writeToFile()
 |
 | INPUT PARAMETERS:
 |    fout - File Pointer of the File to write to, or NULL to write to screen.
 |    fmt, ... - C-Formatted Text with variable length arguments.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method will write a C-Formatted data to a file or to the screen.
 |
 -------------------------------------------------------------------------------*/
void writeToFile( FILE* fout, char* fmt, ... ) {
    va_list args;
    va_start(args, fmt);
    if( fout != NULL ) vfprintf( fout, fmt, args );
    else vprintf( fmt, args );
    va_end(args);
}  // WriteToFile()

/*------------------------------------------------------------------------------
 | NAME:
 |    closeFile()
 |
 | INPUT PARAMETERS:
 |    myFp - File Pointer of file we want closed.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method will close a specific file.
 |
 -------------------------------------------------------------------------------*/
void closeFile( FILE* myFp ) {
    if( myFp == FALSE ) return;
    
    for( uint8 loop = 0; loop < MAX_NUM_OUTPUT_FILES; loop++ ) {
        if( fpArray[loop] == myFp ) {
            fclose(fpArray[loop]);
            fpArray[loop] = NULL;
            numFilePtrs = numFilePtrs - 1;
            return;
        }
    }
}  // closeFile()

/*------------------------------------------------------------------------------
 | NAME:
 |    closeAllFiles()
 |
 | INPUT PARAMETERS:
 |    None.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method will close all open files.
 |
 -------------------------------------------------------------------------------*/
void closeAllFiles( void ) {
    for( uint8 loop = 0; loop < numFilePtrs; loop++ ) {
        if( fpArray[loop] != NULL ) {
            fclose(fpArray[loop]);
            fpArray[loop] = NULL;
        }
    }
    numFilePtrs = 0;
}  // closeAllFiles()

/*------------------------------------------------------------------------------
 | NAME:
 |    areAnyFilesOpen()
 |
 | INPUT PARAMETERS:
 |    None.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method tell if any files remain open.
 |
 -------------------------------------------------------------------------------*/
boolean areAnyFilesOpen( void ) {
    return (numFilePtrs != 0);
}  // areAnyFilesOpen()

/*------------------------------------------------------------------------------
 | NAME:
 |    uint8toBitArray()
 |
 | INPUT PARAMETERS:
 |    byteToBits
 |
 | RETURN VALUES:
 |    const char* - Contains the bit array for printf()ing
 |
 | DESCRIPTION:
 |    This method will write a uint8 in base 2 into a string.
 |    i.e. 0xA7 -> 10100111
 |
 -------------------------------------------------------------------------------*/
const char* uint8toBitArray( uint8 byteToBits )
{
    static char bitArray[9];

    for( int loop = 0; loop < 8; loop++ ) {
        if( (byteToBits & 0x80) == 0x80 ) {
            bitArray[loop] = '1';
        } else {
            bitArray[loop] = '0';
        }
        byteToBits = byteToBits << 1;
    }
    bitArray[8] = '\0';

    return bitArray;
}  // uint8toBitArray()

/*------------------------------------------------------------------------------
 | NAME:
 |    buildOutputPath()
 |
 | INPUT PARAMETERS:
 |    inputFileName - The input file name and path (maybe on the path).
 |    outputDir - Output Directory, if specified, otherwise null.
 |    extension - Extension to add to the base input file name.
 |
 | RETURN VALUES:
 |    artifactPath - The filename and path to write the specific artifact.
 |
 | DESCRIPTION:
 |    This function will piece together a filename and path using the following:
 |    artifactPath = <Output Directory>/<Input Filename Base>.<Extension>
 |
 -------------------------------------------------------------------------------*/
void buildOutputPath(char* inputFilename, char* outputDir, char* extension, char* artifactPath ) {
    ASSERT(inputFilename);
    ASSERT(outputDir);
    ASSERT(artifactPath);
    ASSERT(extension);

    char *tmpCharPtr;
    char baseFilename[MAX_FILE_NAME_LEN];

    artifactPath[0] = '\0';

    tmpCharPtr = strrchr(inputFilename, '/');
    if (tmpCharPtr == NULL) {
        strncpy(baseFilename, inputFilename, MAX_FILE_NAME_LEN);
        baseFilename[MAX_FILE_NAME_LEN - 1] = '\0';
    } else {
        strncpy(baseFilename, tmpCharPtr + 1, MAX_FILE_NAME_LEN);
        baseFilename[MAX_FILE_NAME_LEN - 1] = '\0';
    }

    tmpCharPtr = strrchr(baseFilename, '.');
    if( tmpCharPtr != NULL ) *tmpCharPtr = '\0';

    if( outputDir[0] == '\0' ) {
        char tmpInputFilename[MAX_FILE_NAME_LEN];
        memcpy(tmpInputFilename, inputFilename, MAX_FILE_NAME_LEN);
        tmpInputFilename[MAX_FILE_NAME_LEN-1] = '\0';
        tmpCharPtr = strrchr(tmpInputFilename, '/');
        if( tmpCharPtr == NULL ) {
            sprintf(artifactPath, "%s.%s", baseFilename, extension);
        } else {
            *tmpCharPtr = '\0';
            sprintf(artifactPath, "%s/%s.%s", tmpInputFilename, baseFilename, extension);
        }
    } else {
        if( outputDir[strlen(outputDir)-1] == '/') outputDir[strlen(outputDir)-1] = '\0';
        sprintf(artifactPath, "%s/%s.%s", outputDir, baseFilename, extension );
    }

    ASSERT(strlen(artifactPath) < MAX_FILE_NAME_LEN);
}  // buildOutputPath()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/
