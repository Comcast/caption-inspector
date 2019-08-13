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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <execinfo.h>
#include <signal.h>

#include "debug.h"
#include "types.h"
#include "context.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

const char* DebugLevelText[MAX_DEBUG_LEVEL] = {
        "UNKNOWN_DEBUG_LEVEL",
        "VERBOSE",
        "INFO",
        "WARN",
        "ERROR",
        "ASSERT",
        "FATAL"
};

// WARNING: This array needs to match the defines in debug.h
const char* DebugSectionText[MAX_DEBUG_SECTION] = {
        "UNKNOWN_DEBUG_SECTION",
        "DBG_GENERAL",
        "DBG_PIPELINE",
        "DBG_BUFFER",
        "DBG_FILE_IN",
        "DBG_FILE_OUT",
        "DBG_FF_MPEG",
        "DBG_MPEG_FILE",
        "DBG_GPAC_MP4",
        "DBG_MOV_FILE",
        "DBG_MCC_DEC",
        "DBG_MCC_ENC",
        "DBG_SCC_ENC",
        "DBG_608_DEC",
        "DBG_708_DEC",
        "DBG_608_OUT",
        "DBG_708_OUT",
        "DBG_CCD_OUT",
        "DBG_EXT_ADPT"
};

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

static boolean isInitialized = FALSE;

static FILE* fpLog = NULL;
static EXTERNAL_DBG_FN_PTR DebugExternal = NULL;

static uint8 dbgFileWriteLevel[MAX_DEBUG_SECTION] = {
    DEBUG_LEVEL_INFO,   // UNKNOWN_DEBUG_SECTION
    DEBUG_LEVEL_INFO,   // DBG_GENERAL
    DEBUG_LEVEL_INFO,   // DBG_PIPELINE
    DEBUG_LEVEL_INFO,   // DBG_BUFFER
    DEBUG_LEVEL_INFO,   // DBG_FILE_IN
    DEBUG_LEVEL_INFO,   // DBG_FILE_OUT
    DEBUG_LEVEL_INFO,   // DBG_FF_MPEG
    DEBUG_LEVEL_INFO,   // DBG_MPEG_FILE
    DEBUG_LEVEL_INFO,   // DBG_GPAC_MP4
    DEBUG_LEVEL_INFO,   // DBG_MOV_FILE
    DEBUG_LEVEL_INFO,   // DBG_MCC_DEC
    DEBUG_LEVEL_INFO,   // DBG_MCC_ENC
    DEBUG_LEVEL_INFO,   // DBG_SCC_ENC
    DEBUG_LEVEL_INFO,   // DBG_608_DEC
    DEBUG_LEVEL_INFO,   // DBG_708_DEC
    DEBUG_LEVEL_INFO,   // DBG_608_OUT
    DEBUG_LEVEL_INFO,   // DBG_708_OUT
    DEBUG_LEVEL_INFO,   // DBG_CCD_OUT
    DEBUG_LEVEL_INFO    // DBG_EXTERNL_ADAPT
};
static uint8 dbgStdoutWriteLevel[MAX_DEBUG_SECTION] = {
    DEBUG_LEVEL_INFO,   // UNKNOWN_DEBUG_SECTION
    DEBUG_LEVEL_INFO,   // DBG_GENERAL
    DEBUG_LEVEL_INFO,   // DBG_PIPELINE
    DEBUG_LEVEL_INFO,   // DBG_BUFFER
    DEBUG_LEVEL_INFO,   // DBG_FILE_IN
    DEBUG_LEVEL_INFO,   // DBG_FILE_OUT
    DEBUG_LEVEL_INFO,   // DBG_FF_MPEG
    DEBUG_LEVEL_INFO,   // DBG_MPEG_FILE
    DEBUG_LEVEL_INFO,   // DBG_GPAC_MP4
    DEBUG_LEVEL_INFO,   // DBG_MOV_FILE
    DEBUG_LEVEL_INFO,   // DBG_MCC_DEC
    DEBUG_LEVEL_INFO,   // DBG_MCC_ENC
    DEBUG_LEVEL_INFO,   // DBG_SCC_ENC
    DEBUG_LEVEL_INFO,   // DBG_608_DEC
    DEBUG_LEVEL_INFO,   // DBG_708_DEC
    DEBUG_LEVEL_INFO,   // DBG_608_OUT
    DEBUG_LEVEL_INFO,   // DBG_708_OUT
    DEBUG_LEVEL_INFO,   // DBG_CCD_OUT
    DEBUG_LEVEL_INFO    // DBG_EXTERNL_ADAPT
};
static uint16 numWarnings = 0;
static uint16 numErrors = 0;
static uint16 numFatalErrors = 0;

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

void printStackTrace(void);
void handleSignal(int);

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    DebugInit()
 |
 | INPUT PARAMETERS:
 |    writeToFile - Whether or not to write to a file.
 |    filePathStr - File Path, or NULL if we generate here and save local.
 |    extDbgFn - External Debug Function to route Debug info into.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method initializes the debugging module in the code. It opens up the
 |    appropriate log file for writing.
 -------------------------------------------------------------------------------*/
void DebugInit( boolean writeToFile, char* filePathStr, EXTERNAL_DBG_FN_PTR extDbgFn ) {
    char fileNameStr[MAX_FILE_NAME_LEN];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    DebugExternal = extDbgFn;
    
    numWarnings = 0;
    numErrors = 0;
    numFatalErrors = 0;
    
    if( writeToFile == TRUE ) {
        if( filePathStr != NULL ) {
            strncpy(fileNameStr, filePathStr, MAX_FILE_NAME_LEN-1);
            fileNameStr[MAX_FILE_NAME_LEN-1] = '\0';
            strncat(fileNameStr, ".dbg", (MAX_FILE_NAME_LEN - strlen(fileNameStr) - 1));
            fpLog = fopen(fileNameStr, "w");
            if(fpLog == NULL) LOG(DEBUG_LEVEL_FATAL, DBG_GENERAL, "Unable to Open File: %s - [Errno %d] %s", fileNameStr, errno, strerror(errno));
        } else {
            sprintf(fileNameStr, "%d_%d_%d__%d_%d_%d__debug_log.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            fpLog = fopen(fileNameStr, "w");
            if(fpLog == NULL) LOG(DEBUG_LEVEL_FATAL, DBG_GENERAL, "Unable to Open File: %s - [Errno %d] %s", fileNameStr, errno, strerror(errno));
        }
    }

    if( extDbgFn == NULL ) {
        signal(SIGFPE , handleSignal);
        signal(SIGSEGV, handleSignal);
    }
    
    isInitialized = TRUE;
}  // DebugInit()

/*------------------------------------------------------------------------------
 | NAME:
 |    SetStdoutDebugLevel()
 |
 | INPUT PARAMETERS:
 |    minLevel - Minimum Debug Level which will be printed out.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method sets the minimum level (defaults to Warning) which will be
 |    printed into the log file.
 -------------------------------------------------------------------------------*/
void SetStdoutDebugLevel( uint8 minLevel ) {
    for( int loop = 1; loop < MAX_DEBUG_SECTION; loop++ ) {
        dbgStdoutWriteLevel[loop] = minLevel;
    }
}  // SetStdoutDebugLevel()

/*------------------------------------------------------------------------------
 | NAME:
 |    SetMinDebugLevel()
 |
 | INPUT PARAMETERS:
 |    minLevel - Minimum Debug Level which will be printed out.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method sets the minimum level (defaults to Warning) which will be
 |    printed into the log file.
 -------------------------------------------------------------------------------*/
void SetMinDebugLevel( uint8 minLevel ) {
    for( int loop = 1; loop < MAX_DEBUG_SECTION; loop++ ) {
        dbgFileWriteLevel[loop] = minLevel;
    }
}  // SetMinDebugLevel()

/*------------------------------------------------------------------------------
 | NAME:
 |    GetMinDebugLevel()
 |
 | INPUT PARAMETERS:
 |    section - Debug Section to look at.
 |
 | RETURN VALUES:
 |    Minimum Debug Level which will be printed out.
 |
 | DESCRIPTION:
 |    This method returns the minimum level (defaults to Warning) which will be
 |    printed into the log file.
 -------------------------------------------------------------------------------*/
uint8 GetMinDebugLevel( uint8 section ) {
    return dbgFileWriteLevel[section];
}  // GetMinDebugLevel()

/*------------------------------------------------------------------------------
 | NAME:
 |    DebugLog()
 |
 | INPUT PARAMETERS:
 |    level - The severity of the logging message as defined in debug.h
 |    section - The logical section of the code which made the logging statement.
 |    file - The file name of the function that made the logging statement. This
 |           file name is added by the LOG() Macro.
 |    line - The line number of the function that made the logging statement. This
 |           line number is added by the LOG() Macro.
 |    ... - The string to log and the variable argument array associated with that
 |          string.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method formats the log and writes it out to the log file. This function
 |    is invoked by the LOG() Macro, which adds some relevant information into the
 |    text being logged.
 -------------------------------------------------------------------------------*/
void DebugLog( uint8 level, uint8 section, char* file, int line, ... ) {
    va_list args;
    char message[1024];
    
    if( level == DEBUG_LEVEL_FATAL ) {
        numFatalErrors++;
    } else if( (level == DEBUG_LEVEL_ERROR) || (level == DEBUG_LEVEL_ASSERT) ) {
        numErrors++;
    } else if( level == DEBUG_LEVEL_WARN ) {
        numWarnings++;
    }
    
    if( (level < dbgFileWriteLevel[section]) && ((level < dbgStdoutWriteLevel[section]) && (DebugExternal == NULL)) ) {
        return;
    }
    
    va_start(args, line);
    char* fmt = va_arg(args, char*);
    vsprintf(message, fmt, args);
    va_end(args);
    
    char* basename = strrchr(file, '/');
    basename = basename ? basename+1 : file;

    if( message[(strlen(message)-1)] == '\n' ) message[(strlen(message)-1)] = '\0';
    
    if( DebugExternal == NULL ) {
        if( level >= dbgFileWriteLevel[section] ) {
            if( fpLog != NULL ) {
                fprintf(fpLog, "%s %s [%s:%d] - %s\n", DebugLevelText[level], DebugSectionText[section], basename, line, message);
            } else {
                printf("%s %s [%s:%d] - %s\n", DebugLevelText[level], DebugSectionText[section], basename, line, message);
            }
        }

        if( level == DEBUG_LEVEL_FATAL ) {
            if( fpLog != NULL ) {
                fprintf(fpLog, "\nDumping Call Stack Based on Fatal Error.\n");
                printf("%s %s [%s:%d] - %s\n", DebugLevelText[level], DebugSectionText[section], basename, line, message);
            } else {
                printf("\nDumping Call Stack Based on Fatal Error.\n");
            }
            printStackTrace();
            DebugShutdown();
            exit(1);
        }
    } else {
        (DebugExternal)(basename, line, level, section, message);
    }
    
}  // DebugLog()

/*------------------------------------------------------------------------------
 | NAME:
 |    IsDebugInitialized()
 |
 | INPUT PARAMETERS:
 |    None.
 |
 | RETURN VALUES:
 |    boolean - TRUE = Initialized; FALSE = Uninitialized
 |
 | DESCRIPTION:
 |    This method states whether or not the debugging has been initialized.
 -------------------------------------------------------------------------------*/
boolean IsDebugInitialized( void ) {
    return isInitialized;
}  // IsDebugInitialized()

/*------------------------------------------------------------------------------
 | NAME:
 |    IsDebugExternallyRouted()
 |
 | INPUT PARAMETERS:
 |    None.
 |
 | RETURN VALUES:
 |    boolean - TRUE = Routed Externally; FALSE = Internal only.
 |
 | DESCRIPTION:
 |    This method states whether or not the debugging is going to be routed
 |    externally.
 -------------------------------------------------------------------------------*/
boolean IsDebugExternallyRouted( void ) {
    return (DebugExternal != NULL);
}  // IsDebugInitialized()

/*------------------------------------------------------------------------------
 | NAME:
 |    DebugShutdown()
 |
 | INPUT PARAMETERS:
 |    None.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method terminates the debugging module in the code. It closes the
 |    logging file which has been created.
 -------------------------------------------------------------------------------*/
void DebugShutdown( void ) {
    
    if( DebugExternal == NULL ) {
        if( numFatalErrors > 0 ) {
            printf("\n%d FATAL ERROR(s) Detected! Outputs are very suspect.\n", numFatalErrors);
        }

        printf("\nApplication finished with: %d Warnings and %d Errors\n", numWarnings,
               (numErrors + numFatalErrors) );
        
        char filePath[PATH_MAX];
        if (getcwd(filePath, sizeof(filePath)) != NULL) {
            printf("Logfile Written: %s\n", filePath);
        }
        
        if( fpLog != NULL ) {
            fclose(fpLog);
        }
    }
}  // DebugShutdown()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    handleSignal()
 |
 | DESCRIPTION:
 |    This function is called when we hit a Segmentation Fault or we attempt
 |    to do something evil and unwholesome, like dividing by zero.
 -------------------------------------------------------------------------------*/
void handleSignal( int sigNum ) {
    if( sigNum == SIGSEGV ) {
        if (fpLog != NULL) {
            fprintf(fpLog, "\n*** SIGSEGV Received! Program Bailing\n");
        } else {
            printf("\n*** SIGSEGV Received! Program Bailing\n");
        }
    } else {
        if (fpLog != NULL) {
            fprintf(fpLog, "\n*** SIGFPE Received! Program Bailing\n");
        } else {
            printf("\n*** SIGFPE Received! Program Bailing\n");
        }
    }
    printStackTrace();
    DebugShutdown();
    exit(1);
}  // handleSignal()

/*------------------------------------------------------------------------------
 | NAME:
 |    printStackTrace()
 |
 | DESCRIPTION:
 |    This function will print out the stack trace into the file or onto the
 |    the shell window.
 -------------------------------------------------------------------------------*/
void printStackTrace( void ) {
    void* callstack[128];
    int frames;
    char** strs;

    frames = backtrace(callstack, 128);
    strs = backtrace_symbols(callstack, frames);

    if( fpLog != NULL ) {
        fprintf(fpLog, "\nStack Trace:\n------------\n");
    } else {
        printf("\nStack Trace:\n------------\n");
    }
    for( int loop = 0; loop < frames; ++loop) {
        if( fpLog != NULL ) {
            fprintf(fpLog, "%s\n", strs[loop]);
        } else {
            printf("%s\n", strs[loop]);
        }
    }
    free(strs);
}  // printStackTrace()
