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

#define EXIT_SUCCESS              0
#define EXIT_FAILURE              1
#define EXIT_NO_CAPTIONS_FOUND   10

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

static void printHelp( void );
static void printVersion();

static char* executibleName;
 
static const char *optString = "o:f:b:d:hv?";

static struct option longOpts[] = {
    { "output",           required_argument, NULL, 'o' },
    { "framerate",        required_argument, NULL, 'f' },
    { "no-debug",         no_argument,       NULL, 0 },
    { "no-artifacts",     no_argument,       NULL, 0 },
    { "help",             no_argument,       NULL, 'h' },
    { "version",          no_argument,       NULL, 'v' },
    { "match-pts",        no_argument,       NULL, 'p' },
    { "bail_no_captions", required_argument, NULL, 'b' },
    { "dropframe",        required_argument, NULL, 'd' },
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
    Context ctx;
    char tempOutputPath[MAX_FILE_NAME_LEN];
    int opt;
    int longIndex = 0;

    executibleName = strrchr(argv[0], '/');
    executibleName = executibleName ? executibleName+1 : argv[0];

    memset(&ctx, 0, sizeof(Context));
    ctx.config.passedInFramerate = 0;
    ctx.config.debugFile = TRUE;
    ctx.config.artifacts = TRUE;
    ctx.config.matchPtsTime = FALSE;
    ctx.config.bailAfterMins = 0;
    ctx.config.forceDropframe = FALSE;

    ctx.stats.captionText608Found = FALSE;
    ctx.stats.captionText708Found = FALSE;
    ctx.stats.valid608CaptionsFound = FALSE;
    ctx.stats.valid708CaptionsFound = FALSE;

    if( argv[1] == NULL ) {
        printHelp();
        exit(EXIT_FAILURE);
    }

    ctx.config.outputDirectory[0] = '\0';

    while ((opt = getopt_long(argc, argv, optString, longOpts, &longIndex )) != -1) {
        switch (opt) {
            case 'o' :
                 strncpy(ctx.config.outputDirectory, optarg, MAX_FILE_NAME_LEN);
                 if( ctx.config.outputDirectory[strlen(ctx.config.outputDirectory)-1] != '/' ) {
                     strncat(ctx.config.outputDirectory, "/", (MAX_FILE_NAME_LEN - strlen(ctx.config.outputDirectory)));
                 }
                 break;
            case 'f' :
                 ctx.config.passedInFramerate = (uint16)strtol(optarg, NULL, 10);
                 break;
            case 0:
                if( strcmp( "no-debug", longOpts[longIndex].name ) == 0 ) {
                    ctx.config.debugFile = FALSE;
                } else if( strcmp( "no-artifacts", longOpts[longIndex].name ) == 0 ) {
                    ctx.config.artifacts = FALSE;
                } else {
                    printHelp();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'd' :
                if( strcmp( "true", optarg ) == 0 ) {
                    ctx.config.forceDropframe = TRUE;
                    ctx.config.forcedDropframe = TRUE;
                } else if( strcmp( "false", optarg ) == 0 ) {
                    ctx.config.forceDropframe = TRUE;
                    ctx.config.forcedDropframe = FALSE;
                } else {
                    printHelp();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h' :
                 printHelp();
                 exit(EXIT_SUCCESS);
            case 'v' :
                 printVersion();
                 exit(EXIT_SUCCESS);
            case 'p' :
                ctx.config.matchPtsTime = TRUE;
                break;
            case 'b' :
                 ctx.config.bailAfterMins = (uint8)strtol(optarg, NULL, 10);
                 break;
            default:
                 printHelp();
                 exit(EXIT_FAILURE);
        }
    }

    if( (argc - optind) >= 1 ) {
        ctx.config.inputFilename = argv[optind];
    }

    if( ctx.config.inputFilename == NULL ) {
        printf("ERROR: Please supply an input file name!\n");
        printHelp();
        exit(EXIT_FAILURE);
    }

    buildOutputPath( ctx.config.inputFilename, ctx.config.outputDirectory, tempOutputPath );

    DebugInit(ctx.config.debugFile, ((tempOutputPath[0] == '\0')?NULL:tempOutputPath),  NULL);
    BufferPoolInit();
    LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Version: %s (%s)", VERSION, BUILD);

    if( ctx.config.passedInFramerate != 0 ) {
        LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Passed in Framerate: %d", ctx.config.passedInFramerate);
    }

    if( ctx.config.bailAfterMins != 0 ) {
        LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Bail if no Captions before %d mins of Asset", ctx.config.bailAfterMins);
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);

    FileType sourceType = DetermineFileType(ctx.config.inputFilename);

    LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Processing input file %s of type %s", ctx.config.inputFilename, DECODE_CAPTION_FILE_TYPE(sourceType));
    LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Writing Output To: %s.???", tempOutputPath);

    if( ctx.config.artifacts == TRUE ) {
        LOG(DEBUG_LEVEL_INFO, DBG_GENERAL, "Writing all Artifacts.");
    }

    boolean retval = FALSE;

    switch(sourceType) {
        case SCC_CAPTIONS_FILE:
            retval = PlumbSccPipeline(&ctx, ctx.config.inputFilename, ((tempOutputPath[0] == '\0') ? NULL : tempOutputPath) );
            break;
        case MCC_CAPTIONS_FILE:
            retval = PlumbMccPipeline(&ctx, ctx.config.inputFilename, ((tempOutputPath[0] == '\0') ? NULL : tempOutputPath));
            break;
        case MPEG_BINARY_FILE:
#ifdef DONT_COMPILE_FFMPEG
            LOG(DEBUG_LEVEL_FATAL, DBG_GENERAL, "Executable was compiled without FFMPEG, unable to process Binary MPEG File");
#else
            retval = PlumbMpegPipeline(&ctx, ctx.config.inputFilename, ((tempOutputPath[0] == '\0') ? NULL : tempOutputPath));
#endif
            break;
        case MOV_BINARY_FILE:
            retval = PlumbMovPipeline(&ctx, ctx.config.inputFilename, ((tempOutputPath[0] == '\0') ? NULL : tempOutputPath));
            break;
        default:
            LOG(DEBUG_LEVEL_ERROR, DBG_GENERAL, "Impossible Branch - %d", sourceType);
    }

    if( retval == TRUE ) {
        DrivePipeline(sourceType, &ctx);
    } else {
        LOG(DEBUG_LEVEL_FATAL, DBG_GENERAL, "Unable to establish pipeline. Unable to proceed.");
    }
    if( Shutdown() == TRUE ) {
        printf("%s Completed with Errors.\n", executibleName);
        return EXIT_FAILURE;
    }

    if( (ctx.stats.valid608CaptionsFound == FALSE) && (ctx.stats.valid708CaptionsFound == FALSE) ) {
        printf("%s Completed with No Captions Found.\n", executibleName);
        return EXIT_NO_CAPTIONS_FOUND;
    }

    printf("%s Completed Successfully.\n", executibleName);
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
 |    boolean - True: Fatal Error(s) found; False: No Fatal Errors.
 |
 | DESCRIPTION:
 |    This method cleans up any loose ends in preparation for stopping the
 |    program. (e.g. Closing Files.)
 -------------------------------------------------------------------------------*/
boolean Shutdown( void ) {
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
    return DebugShutdown();
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
static void printHelp( void ) {
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
    printf("\nUsage: %s [options] <input-file>\n", executibleName);
    printf("\nOptions:\n");
    printf("    -h|--help                    : Display this help message.\n");
    printf("    -v|--version                 : Display version and build information.\n");
    printf("    -p|--match_pts_time          : When encoding an MCC file, match the PTS time found in the asset.\n");
    printf("    -o|--output <dir>            : Directory to save output files. If this is not set files are saved in the directory of the input file.\n");
    printf("    -f|--framerate <num>         : Framerate * 100 (e.g. 3000, 2997). This is a requirement for SCC Files.\n");
    printf("    -b|--bail_no_captions <mins> : Bail if no captions are found x minutes into the asset.\n");
    printf("    -d|--dropframe <true/false>  : Force asset to be either dropframe or no dropframe.\n");
    printf("    --no-debug                   : Don't create a debug file.\n");
    printf("    --no-artifacts               : Don't create artifact files.\n");
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
