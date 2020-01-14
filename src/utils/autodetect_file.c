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
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <zconf.h>

#include "debug.h"

#include "autodetect_file.h"
#include "cc_utils.h"
#include "buffer_utils.h"
#ifdef COMPILE_GPAC
#include "gpac/isomedia.h"
#endif
#include "output_utils.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

// WARNING: This array needs to match the defines in autodetect_file.h
const char* captionFileTypeStr[MAX_FILE_TYPE] = {
        "Unknown Caption File",     // UNK_CAPTIONS_FILE
        "SCC Caption File",         // SCC_CAPTIONS_FILE
        "MCC Caption File",         // MCC_CAPTIONS_FILE
        "Binary MPEG File",         // MPEG_BINARY_FILE
        "Binary MOV File",          // MOV_BINARY_FILE
};

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

char* mediaInfoInvokeStr = "/usr/local/bin/mediainfo";
char* mediaInfoVerStr = "--Version";
char* mediaInfoFullStr = "-full";
char* dropframeTagStr = "Time code of first frame";

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

#ifdef COMPILE_GPAC
static void gpacLoggingSink( void*, GF_LOG_Level, GF_LOG_Tool, const char*, va_list );
#endif

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    DetermineDropFrame()
 |
 | INPUT PARAMETERS:
 |    fileNameStr - The name of the source file.
 |    saveMediaInfo - Save the MediaInfo output into a file: <fileNameStr>.inf
 |    outputPath - Path to save the MediaInfo output, or NULL to save in
 |                  the input dir.
 |
 | RETURN VALUES:
 |    boolean - Was the Call Successful: Successful = TRUE; Failure = FALSE
 |    isDropFramePtr - Dropframe = TRUE; No DropFrame = FALSE
 |
 | DESCRIPTION:
 |    This determine whether this is likely a MPEG file or likely a Caption
 |    file. This is done by examining the first few bytes of the file.
 ------------------------------------------------------------------------------*/
boolean DetermineDropFrame( char* fileNameStr, boolean saveMediaInfo, char* outputPath ) {
    FILE* filePtr;
    FILE* outFilePtr = NULL;
    char invocationString[MAX_FILE_NAME_LEN*2];
    char buffer[1035];
    boolean dfFound = FALSE;
    boolean isDropFrame = FALSE;

    if( saveMediaInfo == TRUE ) {
        char tempFilename[MAX_FILE_NAME_LEN];

        buildOutputPath(fileNameStr, outputPath, "inf", tempFilename);
        outFilePtr = fileOutputInit(tempFilename);

        sprintf(invocationString, "%s %s", mediaInfoInvokeStr, mediaInfoVerStr);

        filePtr = popen(invocationString, "r");
        if( filePtr == NULL ) {
            char filePath[PATH_MAX];
            filePath[0] = '\0';
            getcwd(filePath, sizeof(filePath));
            LOG(DEBUG_LEVEL_FATAL, DBG_FILE_IN, "Unable to Open File: %s (looking here: %s) - [Errno %d] %s", fileNameStr, filePath, errno, strerror(errno));
            return FALSE;
        }

        writeToFile(outFilePtr, "Media Info for File: %s\n\n", fileNameStr );
        while( fgets(buffer, sizeof(buffer)-1, filePtr) != NULL ) {
            writeToFile(outFilePtr, buffer);
            for( int loop = 0; (loop < sizeof(buffer)-1) && (buffer[loop] != '\0'); loop++ ) {
                if( (buffer[loop] == '\n') || (buffer[loop] == '\r') ) {
                    buffer[loop] = ' ';
                }
            }
            LOG(DEBUG_LEVEL_INFO, DBG_FILE_IN, "Using --- %s", buffer);
        }
        writeToFile(outFilePtr, "==========================\n" );
        pclose(filePtr);
    }

    sprintf(invocationString, "%s %s %s", mediaInfoInvokeStr, fileNameStr, mediaInfoFullStr);

    filePtr = popen(invocationString, "r");
    if( filePtr == NULL ) {
        char filePath[PATH_MAX];
        filePath[0] = '\0';
        getcwd(filePath, sizeof(filePath));
        LOG(DEBUG_LEVEL_FATAL, DBG_FILE_IN, "Unable to Open File: %s (looking here: %s) - [Errno %d] %s", fileNameStr, filePath, errno, strerror(errno));
        return FALSE;
    }

    while( fgets(buffer, sizeof(buffer)-1, filePtr) != NULL ) {
        if( strncmp(buffer, dropframeTagStr, strlen(dropframeTagStr)) == 0 ) {
            char* tmpCharPtr;
            tmpCharPtr = strchr(buffer, ':');
            tmpCharPtr = tmpCharPtr+2;
            if( tmpCharPtr[8] == ';' ) {
                dfFound = TRUE;
                isDropFrame = TRUE;
                LOG(DEBUG_LEVEL_INFO, DBG_FILE_IN, "File: %s is determined to be dropframe", fileNameStr );
            } else if( tmpCharPtr[8] == ':' ) {
                dfFound = TRUE;
                isDropFrame = FALSE;
                LOG(DEBUG_LEVEL_INFO, DBG_FILE_IN, "File: %s is determined to be non-dropframe", fileNameStr );
            } else {
                LOG(DEBUG_LEVEL_ERROR, DBG_FILE_IN, "Unable to determine DropFrame: %s %s", tmpCharPtr, buffer );
            }
        }
        if( saveMediaInfo == TRUE ) {
            ASSERT(outFilePtr);
            writeToFile(outFilePtr, buffer);
        }
    }
    pclose(filePtr);

    if( saveMediaInfo == TRUE ) {
        ASSERT(outFilePtr);
        closeFile(outFilePtr);
    }

    if( dfFound == TRUE ) {
        return isDropFrame;
    } else {
        LOG(DEBUG_LEVEL_WARN, DBG_FILE_IN, "Unable to determine DropFrame from MediaInfo.");
        return FALSE;
    }
}  // DetermineDropFrame()

/*------------------------------------------------------------------------------
 | NAME:
 |    DetermineFileType()
 |
 | INPUT PARAMETERS:
 |    fileNameStr - The name of the source file.
 |
 | RETURN VALUES:
 |    uint8 - The source type of the file.
 |
 | DESCRIPTION:
 |    This determine whether this is likely a MPEG file or likely a Caption
 |    file. This is done by examining the first few bytes of the file.
 ------------------------------------------------------------------------------*/
FileType DetermineFileType( char* fileNameStr ) {
    FILE* filePtr = fopen(fileNameStr, "r");

    if( filePtr == NULL ) {
        char filePath[PATH_MAX];
        filePath[0] = '\0';
        getcwd(filePath, sizeof(filePath));
        LOG(DEBUG_LEVEL_FATAL, DBG_FILE_IN, "Unable to Open File: %s (looking here: %s) - [Errno %d] %s", fileNameStr, filePath, errno, strerror(errno));
        return UNK_CAPTIONS_FILE;
    }

    uint8 buffer[100];

    size_t total = fread(buffer, 1, 100, filePtr);

    if( total != 100 ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_FILE_IN, "Unable to read a hundred bytes from File: %s - %d", fileNameStr, total);
        fclose(filePtr);
        return UNK_CAPTIONS_FILE;
    }

    for( int loop = 0; loop < 100; loop++ ) {
        if( !((buffer[loop] >= ' ') && (buffer[loop] <= '~')) &&
             (buffer[loop] != 0) && (buffer[loop] != 9) &&
             (buffer[loop] != 10) && (buffer[loop] != 13) ) {
            fclose(filePtr);
#ifdef COMPILE_GPAC
            gf_log_set_callback(NULL, gpacLoggingSink);
            GF_ISOFile* movFilePtr = gf_isom_open(fileNameStr, GF_ISOM_OPEN_READ, NULL);
            if( movFilePtr != NULL ) {
                gf_isom_close(movFilePtr);
                LOG(DEBUG_LEVEL_INFO, DBG_FILE_IN, "Determined %s is an MOV File", fileNameStr);
                return MOV_BINARY_FILE;
            } else {
                LOG(DEBUG_LEVEL_INFO, DBG_FILE_IN, "Determined %s is an MPEG File", fileNameStr);
                return MPEG_BINARY_FILE;
            }
#else
            LOG(DEBUG_LEVEL_INFO, DBG_FILE_IN, "Determined %s is an MPEG File", fileNameStr);
            return MPEG_BINARY_FILE;
#endif
        }
    }

    rewind(filePtr);

    char* line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    FileType retval = UNK_CAPTIONS_FILE;
    boolean wasSuccessful;
    boolean firstCaptionTimeSet = FALSE;
    CaptionTime firstCaptionTime;
    CaptionTime lastCaptionTime;

// TODO - Do I Care?
    firstCaptionTime.dropframe = FALSE;
    lastCaptionTime.dropframe = FALSE;

    while( (read = getline(&line, &len, filePtr)) != -1 ) {

        if( strncmp(line, "Scenarist_SCC V1.0", strlen("Scenarist_SCC V1.0")) == 0 ) {
            LOG(DEBUG_LEVEL_INFO, DBG_FILE_IN, "Determined %s is an SCC File", fileNameStr);
            retval = SCC_CAPTIONS_FILE;
        } else if( strncmp(line, "File Format=MacCaption_MCC V", strlen("File Format=MacCaption_MCC V")) == 0 ) {
            LOG(DEBUG_LEVEL_INFO, DBG_FILE_IN, "Determined %s is an MCC File", fileNameStr);
            retval = MCC_CAPTIONS_FILE;
        } else if( (strncmp(line, "//", 2) == 0) || (read < 5) ) {
            continue;
        } else if( strncmp(line, "UUID=", strlen("UUID=")) == 0 ) {
            continue;
        } else if( strncmp(line, "Creation Program=", strlen("Creation Program=")) == 0 ) {
            continue;
        } else if( strncmp(line, "Creation Date=", strlen("Creation Date=")) == 0 ) {
            continue;
        } else if( strncmp(line, "Creation Time=", strlen("Creation Time=")) == 0 ) {
            continue;
        } else if( strncmp(line, "Time Code Rate=", strlen("Time Code Rate=")) == 0 ) {
            continue;
        } else {
            char* timecode = strtok(line, "\t");
            if( firstCaptionTimeSet == FALSE ) {
                wasSuccessful = decodeTimeCode(timecode, &firstCaptionTime);
                firstCaptionTimeSet = TRUE;
            } else {
                wasSuccessful = decodeTimeCode(timecode, &lastCaptionTime);
            }

            if( wasSuccessful == FALSE ) {
                LOG(DEBUG_LEVEL_WARN, DBG_FILE_IN, "Unable to parse timecode on line: %s", line);
                fclose(filePtr);
                return UNK_CAPTIONS_FILE;
            }
        }
    }

    fclose(filePtr);

    LOG(DEBUG_LEVEL_INFO, DBG_FILE_IN, "Caption Window for File: %s --- %02d:%02d:%02d;%02d to %02d:%02d:%02d;%02d ", fileNameStr,
        firstCaptionTime.hour, firstCaptionTime.minute, firstCaptionTime.second, firstCaptionTime.frame,
        lastCaptionTime.hour, lastCaptionTime.minute, lastCaptionTime.second, lastCaptionTime.frame);

    return retval;
} // DetermineFileType()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

#ifdef COMPILE_GPAC
/*------------------------------------------------------------------------------
 | NAME:
 |    gpacLoggingSink()
 |
 | DESCRIPTION:
 |    This function merely suppresses logging from the GPAC MP4 Code.
 -------------------------------------------------------------------------------*/
static void gpacLoggingSink( void *cbck, GF_LOG_Level log_level, GF_LOG_Tool log_tool, const char* fmt, va_list vlist ) { }
#endif
