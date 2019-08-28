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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "debug.h"
#include "autodetect_file.h"
#include "mpeg_file.h"
#include "mov_file.h"
#include "output_utils.h"
#include "getopt.h"
#include "version.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

static struct timespec startTime;

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

static void printHelp( char* );
static void printVersion();

struct globalArgs_t {
    char* cmd;
    char outputDirectory[MAX_FILE_NAME_LEN];    // -o option
    uint32 passedInFramerate;                   // -f option
    boolean bailNoCaptions;                     // -b --bail_no_captions
    boolean debugFile;                          // --no-debug option
    boolean artifacts;                          // --no-artifacts option
    char* inputFilename;                        // input file
} globalArgs;
 
static const char *optString = "o:f:hvb?";

static struct option longOpts[] = {
    { "output",           required_argument, NULL, 'o' },
    { "framerate",        required_argument, NULL, 'f' },
    { "no-debug",         no_argument,       NULL, 0 },
    { "no-artifacts",     no_argument,       NULL, 0 },
    { "help",             no_argument,       NULL, 'h' },
    { "version",          no_argument,       NULL, 'v' },
    { "bail_no_captions", no_argument,       NULL, 'b' },
    { 0, no_argument, NULL, 0 }
};

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    main()
 |
 | INPUT PARAMETERS:
 |    argc - Standard argument count to the executable.
 |    argv - Standard argument array for the executable.
 |
 | RETURN VALUES:
 |    Standard UNIX return code.
 |
 | DESCRIPTION:
 |    This is the main method of the Caption Converter program. It processes the
 |    command line arguments, opens the source file, decodes the contents, and
 |    writes the results to the destination file.
 -------------------------------------------------------------------------------*/
int main( int argc, char* argv[] ) {
    char tempOutputPath[MAX_FILE_NAME_LEN];
    int opt;
    int longIndex = 0;

    globalArgs.cmd = argv[0];
    globalArgs.passedInFramerate = 0;
    globalArgs.debugFile = TRUE;
    globalArgs.artifacts = TRUE;
    globalArgs.bailNoCaptions = FALSE;

    if( argv[1] == NULL ) {
        printHelp(globalArgs.cmd);
        exit(EXIT_FAILURE);
    }
    
    globalArgs.outputDirectory[0] = '\0';

    while ((opt = getopt_long(argc, argv, optString, longOpts, &longIndex )) != -1) {
        switch (opt) {
             case 'o' :
                 strncpy(globalArgs.outputDirectory, optarg, MAX_FILE_NAME_LEN);
                 if( globalArgs.outputDirectory[strlen(globalArgs.outputDirectory)-1] != '/' ) {
                     strncat(globalArgs.outputDirectory, "/", (MAX_FILE_NAME_LEN - strlen(globalArgs.outputDirectory)));
                 }
                 break;
             case 'f' :
                 globalArgs.passedInFramerate = (uint16)strtol(optarg, NULL, 10);
                 break;
            case 0:
                if( strcmp( "no-debug", longOpts[longIndex].name ) == 0 ) {
                    globalArgs.debugFile = FALSE;
                } else if( strcmp( "no-artifacts", longOpts[longIndex].name ) == 0 ) {
                    globalArgs.artifacts = FALSE;
                } else {
                    printHelp(globalArgs.cmd);
                    exit(EXIT_FAILURE);
                }
                break;
             case 'h' :
                 printHelp(globalArgs.cmd);
                 exit(EXIT_SUCCESS);
             case 'v' :
                 printVersion();
                 exit(EXIT_SUCCESS);
            case 'b' :
                 globalArgs.bailNoCaptions = TRUE;
                 break;
            default:
                 printHelp(globalArgs.cmd);
                 exit(EXIT_FAILURE);
        }
    }

    if( (argc - optind) >= 1 ) {
        globalArgs.inputFilename = argv[optind];
    }

    if( globalArgs.inputFilename == NULL ) {
        printf("ERROR: Please supply an input file name!\n");
        printHelp(globalArgs.cmd);
        exit(EXIT_FAILURE);
    }

    buildOutputPath( globalArgs.inputFilename, globalArgs.outputDirectory, tempOutputPath );

    DebugInit(globalArgs.debugFile, ((tempOutputPath[0] == '\0')?NULL:tempOutputPath),  NULL);
    BufferPoolInit();
    LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Version: %s (%s)", VERSION, BUILD);

    if( globalArgs.passedInFramerate != 0 ) {
        LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Passed in Framerate: %d", globalArgs.passedInFramerate);
    }

    if( globalArgs.bailNoCaptions == TRUE ) {
        LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Bail if no Captions before 20 mins of Asset");
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);

    FileType sourceType = DetermineFileType(globalArgs.inputFilename);

    LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Processing input file %s of type %s", globalArgs.inputFilename, DECODE_CAPTION_FILE_TYPE(sourceType));
    LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Writing Output To: %s.???", tempOutputPath);

    if( globalArgs.artifacts == TRUE ) {
        LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Writing all Artifacts.");
    }

    Context ctx;
    boolean retval = FALSE;

    switch(sourceType) {
        case SCC_CAPTIONS_FILE:
            retval = PlumbSccPipeline(&ctx, globalArgs.inputFilename, tempOutputPath, globalArgs.passedInFramerate, globalArgs.artifacts);
            break;
        case MCC_CAPTIONS_FILE:
            retval = PlumbMccPipeline(&ctx, globalArgs.inputFilename, tempOutputPath, globalArgs.artifacts);
            break;
        case MPEG_BINARY_FILE:
#ifdef DONT_COMPILE_FFMPEG
            LOG(DEBUG_LEVEL_FATAL, DBG_GENERAL, "Executable was compiled without FFMPEG, unable to process Binary MPEG File");
#else
            retval = PlumbMpegPipeline(&ctx, globalArgs.inputFilename, tempOutputPath, globalArgs.artifacts, tempOutputPath, globalArgs.bailNoCaptions);
#endif
            break;
        case MOV_BINARY_FILE:
            retval = PlumbMovPipeline(&ctx, globalArgs.inputFilename, tempOutputPath, TRUE, tempOutputPath, globalArgs.bailNoCaptions);
            break;
        default:
            LOG(DEBUG_LEVEL_ERROR, DBG_GENERAL, "Impossible Branch - %d", sourceType);
    }

    if( retval == TRUE ) {
        DrivePipeline(sourceType, &ctx);
    } else {
        LOG(DEBUG_LEVEL_FATAL, DBG_GENERAL, "Unable to establish pipeline. Unable to proceed.");
    }
    Shutdown();

    return EXIT_SUCCESS;
}  // main()

/*------------------------------------------------------------------------------
 | NAME:
 |    Shutdown()
 |
 | INPUT PARAMETERS:
 |    None.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method cleans up any loose ends in preparation for stopping the
 |    program. (e.g. Closing Files.)
 -------------------------------------------------------------------------------*/
void Shutdown( void ) {
    struct timespec endTime;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime);

    uint8 secDuration = (endTime.tv_sec - startTime.tv_sec) % 60;
    uint8 minDuration = ((endTime.tv_sec - startTime.tv_sec) / 60) % 60;
    uint8 hourDuration = ((endTime.tv_sec - startTime.tv_sec) / 3600);
    ASSERT(((endTime.tv_sec - startTime.tv_sec) / 3600) < 5);

    if( hourDuration > 0 ) {
        LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Total Runtime: %d hours %d minutes %d seconds", hourDuration, minDuration, secDuration);
    } else if( minDuration > 0 ) {
        LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Total Runtime: %d minutes %d seconds", minDuration, secDuration);
    } else {
        LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Total Runtime: %d seconds", secDuration);
    }
    
    ASSERT(areAnyFilesOpen() == FALSE);
    ASSERT(NumAllocatedBuffers() == 0);
    DebugShutdown();
}  // Shutdown()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    printHelp()
 |
 | DESCRIPTION:
 |    This help message with command line options for this program.
 -------------------------------------------------------------------------------*/
static void printHelp( char* nameAndPath ) {
    
    char* programName;
    programName = strrchr(nameAndPath, '/');
    programName = programName ? programName+1 : nameAndPath;

    printf("\n         .oooooo.                            .    o8o\n");
    printf("        d8P'  `Y8b                         .o8    `\"'\n");
    printf("       888           .oooo.   oo.ooooo.  .o888oo oooo   .ooooo.  ooo. .oo.\n");
    printf("       888          `P  )88b   888' `88b   888   `888  d88' `88b `888P\"Y88b\n");
    printf("       888           .oP\"888   888   888   888    888  888   888  888   888\n");
    printf("       `88b    ooo  d8(  888   888   888   888 .  888  888   888  888   888\n");
    printf("        `Y8bood8P'  `Y888\"\"8o  888bod8P'   \"888\" o888o `Y8bod8P' o888o o888o\n");
    printf("                               888\n");
    printf("ooooo                         o888o                           .\n");
    printf("`888'                                                       .o8\n");
    printf(" 888  ooo. .oo.    .oooo.o oo.ooooo.   .ooooo.   .ooooo.  .o888oo  .ooooo.  oooo d8b\n");
    printf(" 888  `888P\"Y88b  d88(  \"8  888' `88b d88' `88b d88' `\"Y8   888   d88' `88b `888\"\"8P\n");
    printf(" 888   888   888  `\"Y88b.   888   888 888ooo888 888         888   888   888  888\n");
    printf(" 888   888   888  o.  )88b  888   888 888    .o 888   .o8   888 . 888   888  888\n");
    printf("o888o o888o o888o 8\"\"888P'  888bod8P' `Y8bod8P' `Y8bod8P'   \"888\" `Y8bod8P' d888b\n");
    printf("                            888\n");
    printf("                            o888o\n");
    printVersion();
    printf("\nUsage: %s [options] <input-file>\n", programName);
    printf("\nOptions:\n");
    printf("    -h|--help                : Display this help message.\n");
    printf("    -v|--version             : Display version and build information.\n");
    printf("    -o|--output <dir>        : Directory to save output files. If this is not set files are saved in the directory of the input file.\n");
    printf("    -f|--framerate <num>     : Framerate * 100 (e.g. 3000, 2997). This is a requirement for SCC Files.\n");
    printf("    -b|--bail_no_captions    : Bail if no captions are found 20 minutes into the asset.\n");
    printf("    --no-debug               : Don't create a debug file.\n");
    printf("    --no-artifacts           : Don't create artifact files.\n");
}  // printHelp()

/*------------------------------------------------------------------------------
 | NAME:
 |    printVersion()
 |
 | DESCRIPTION:
 |    Display the version and build. Version is generated by `git describe` and
 |    is the latest "v[0-9]*" tag found. The build is also generated by
 |    `get describe` and is in the format "<version>-<num>-g<abbrev>" where
 |    _<num>_ is the number of commits since the version tag, and _<abbrev>_ is
 |    the 6 most significant digits of the commit identifier.
 |
 |    See the include/version.h target in ../Makefile for details
 -------------------------------------------------------------------------------*/
static void printVersion( void ) {
    printf("\nVersion: %s (%s) ", VERSION, BUILD);
#ifdef DONT_COMPILE_FFMPEG
    printf("No FFMPEG ");
#else
    printf("+FFMPEG ");
#endif
#ifdef COMPILE_GPAC
    printf("+GPAC ");
#else
    printf("No GPAC ");
#endif
    printf("\n");
}
