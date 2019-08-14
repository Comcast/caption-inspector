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

#include "debug.h"

#include "pipeline_utils.h"
#include "autodetect_file.h"
#include "cc_utils.h"
#include "captions_file.h"
#include "mpeg_file.h"
#include "dtvcc_decode.h"
#include "line21_decode.h"
#include "mcc_decode.h"
#include "mcc_encode.h"
#include "scc_encode.h"
#include "mcc_output.h"
#include "dtvcc_output.h"
#include "line21_output.h"
#include "cc_data_output.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

// WARNING: This array needs to match the defines in pipeline_utils.h
static const char* LinkTypeText[MAX_LINK_TYPE] = {
    "UNKNOWN_LINK_TYPE",
    "SCC_FILE___SCC_DATA",
    "MCC_FILE___MCC_DATA",
    "MPG_FILE___CC_DATA",
    "MCC_DATA___CC_DATA",
    "SCC_DATA___CC_DATA",
    "SEI_DATA___CC_DATA",    
    "CC_DATA___MCC_DATA",
    "CC_DATA___LINE21_DATA",
    "CC_DATA___DTVCC_DATA",
    "CC_DATA___TEXT_FILE",
    "MCC_DATA___TEXT_FILE",
    "LINE21_DATA___TEXT_FILE",
    "DTVCC_DATA___TEXT_FILE",
    "LINE21_DATA___EXTRNL_ADPTR",
    "DTVCC_DATA___EXTRNL_ADPTR"
};

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    InitSinks()
 |
 | INPUT PARAMETERS:
 |    sinksToInit - Pointer to the sink structure to initialize.
 |    myLinkType - The Caller's link type (for logging).
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method initializes the sink structure in the caller.
 |
 -------------------------------------------------------------------------------*/
void InitSinks( Sinks* sinksToInit, uint8 myLinkType ) {
    ASSERT(sinksToInit);
    sinksToInit->linkType = myLinkType;
    sinksToInit->numSinks = 0;
}  // InitSinks()

/*------------------------------------------------------------------------------
 | NAME:
 |    AddSink()
 |
 | INPUT PARAMETERS:
 |    sinks - Pointer to the sink structure of the caller.
 |    linkInfoPtr - Pointer to information as to how to link the next element.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method adds another sink to the caller.
 -------------------------------------------------------------------------------*/
boolean AddSink( Sinks* sinks, LinkInfo* linkInfoPtr ) {
    ASSERT(sinks);
    ASSERT(sinks->linkType < MAX_LINK_TYPE);
    ASSERT(linkInfoPtr);
    ASSERT(linkInfoPtr->linkType < MAX_LINK_TYPE);
    ASSERT(linkInfoPtr->sourceType < MAX_DATA_TYPE);
    ASSERT(linkInfoPtr->sinkType < MAX_DATA_TYPE);

    if( (sinks->numSinks + 1) <= MAX_NUMBER_OF_SINKS ) {
        sinks->sink[sinks->numSinks] = *linkInfoPtr;
        sinks->numSinks = sinks->numSinks + 1;
        LOG(DEBUG_LEVEL_VERBOSE, DBG_PIPELINE, "Added Sink: %s -> %s", LinkTypeText[sinks->linkType], LinkTypeText[linkInfoPtr->linkType]);
        return TRUE;
    } else {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Ignoring attempt to add too many sinks.");
        return FALSE;
    }
} // AddSink()

/*------------------------------------------------------------------------------
 | NAME:
 |    PassToSinks()/_PassToSinks()
 |
 | INPUT PARAMETERS:
 |    fileNameStr - The name of the calling function.
 |    lineNum - The line number of the calling function.
 |    ctxPtr - Pointer to the Context of the caller.
 |    buffPtr - Pointer to the buffer to pass.
 |    sinks - Pointer to the sink structure of the caller.
 |
 | RETURN VALUES:
 |    uint8 - Success is TRUE / PIPELINE_SUCCESS, Failure is FALSE / PIPELINE_FAILURE
 |            All other codes specified in header.
 |
 | DESCRIPTION:
 |    This method moves a buffer from one element to its sinks. It also adds
 |    references to the buffer, so that it won't be freed until all of the
 |    sinks are done with it.
 -------------------------------------------------------------------------------*/
uint8 _PassToSinks( char* fileNameStr, int lineNum, Context* ctxPtr, Buffer* buffPtr, Sinks* sinks ) {
    ASSERT(ctxPtr);
    ASSERT(buffPtr);
    ASSERT(sinks);
    ASSERT(sinks->numSinks <= MAX_NUMBER_OF_SINKS);
    boolean retval = TRUE;

    char* basename = fileNameStr;
    basename = strrchr(fileNameStr, '/');
    basename = basename ? basename+1 : fileNameStr;

    if( sinks->numSinks == 0 ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Attempted to pass a buffer to no sinks in {%s:%d}.", basename, lineNum);
    }

    for( int loop = 0; loop < sinks->numSinks; loop++ ) {
        AddReader(buffPtr);
    }
    
    for( int loop = 0; loop < sinks->numSinks; loop++ ) {
        ASSERT(sinks->linkType < MAX_LINK_TYPE);
        ASSERT(sinks->sink[loop].linkType < MAX_LINK_TYPE);
        ASSERT(sinks->sink[loop].NextBufferFnPtr);
        LOG(DEBUG_LEVEL_VERBOSE, DBG_PIPELINE, "Passed Buffer [%p] to Sink: %s -> %s in {%s:%d}", buffPtr, LinkTypeText[sinks->linkType], LinkTypeText[sinks->sink[loop].linkType], basename, lineNum);
        if( sinks->sink[loop].NextBufferFnPtr(ctxPtr, buffPtr) == FALSE ) {
            retval = FALSE;
        }
    }
    return retval;
} // PassToSinks()

/*------------------------------------------------------------------------------
 | NAME:
 |    ShutdownSinks()/_ShutdownSinks()
 |
 | INPUT PARAMETERS:
 |    fileNameStr - The name of the calling function.
 |    lineNum - The line number of the calling function.
 |    ctxPtr - Pointer to the Context of the caller.
 |    sinks - Pointer to the sink structure of the caller.
 |
 | RETURN VALUES:
 |    uint8 - Success is TRUE / PIPELINE_SUCCESS, Failure is FALSE / PIPELINE_FAILURE
 |            All other codes specified in header.
 |
 | DESCRIPTION:
 |    This method tells the sinks that there is no more data in the pipeline.
 -------------------------------------------------------------------------------*/
uint8 _ShutdownSinks( char* fileNameStr, int lineNum, Context* ctxPtr, Sinks* sinks ) {
    ASSERT(sinks);
    ASSERT(ctxPtr);
    ASSERT(sinks->numSinks <= MAX_NUMBER_OF_SINKS);
    
    boolean retval = TRUE;

    char* basename = fileNameStr;
    basename = strrchr(fileNameStr, '/');
    basename = basename ? basename+1 : fileNameStr;

    if( sinks->numSinks == 0 ) {
        LOG(DEBUG_LEVEL_WARN, DBG_PIPELINE, "Unexpected Call to ShutdownSinks() from {%s:%d} when no sinks exist: %s", basename, lineNum, LinkTypeText[sinks->linkType]);
    }
    
    for( int loop = 0; loop < sinks->numSinks; loop++ ) {
        ASSERT(sinks->linkType < MAX_LINK_TYPE);
        ASSERT(sinks->sink[loop].linkType < MAX_LINK_TYPE);
        ASSERT(sinks->sink[loop].ShutdownFnPtr);
        LOG(DEBUG_LEVEL_VERBOSE, DBG_PIPELINE, "Call from {%s:%d} to Shut Down Sink: %s -> %s", basename, lineNum, LinkTypeText[sinks->linkType], LinkTypeText[sinks->sink[loop].linkType]);
        if( sinks->sink[loop].ShutdownFnPtr(ctxPtr) == FALSE ) {
            retval = FALSE;
        }
    }
    return retval;
} // ShutdownSinks()

/*------------------------------------------------------------------------------
 | NAME:
 |    PlumbSccPipeline()
 |
 | INPUT PARAMETERS:
 |    inputFilename - Name of the input file (and root of the output filename).
 |    outputFilename - Name of the output file
 |    framerate - Framerate of the asset described in the SCC file.
 |
 | RETURN VALUES:
 |    Context - Context of this Pipeline
 |
 | DESCRIPTION:
 |    This method plumbs the pipeline to decode a SCC file and leave it's
 |    decoded text in a file that is <inputFilename>.608
 |
 | PIPELINE:                                      +----------------+      +----------------+
 |                                           +--> | Line 21 Decode | -?-> | Line 21 Output |
 |                                           |    +----------------+      +----------------+
 |                                           |
 |    +--------------+      +------------+   |     +------------+         +------------+
 |    | Caption File | ---> | SCC Encode | --+-?-> | MCC Encode | ------> | MCC Output |
 |    +--------------+      +------------+   |     +------------+         +------------+
 |                                           |
 |                                           |     +----------------+
 |                                           +-?-> | CC Data Output |
 |                                                 +----------------+
 -------------------------------------------------------------------------------*/
boolean PlumbSccPipeline( Context* ctxPtr, char* inputFilename, char* outputFilename, uint32 framerate, boolean artifacts ) {
    ASSERT(ctxPtr);
    memset(ctxPtr, 0, sizeof(Context));
    boolean retval;

    if( inputFilename == NULL ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "NULL Input Filename, unable to establish pipeline.");
        return FALSE;
    }

    if( outputFilename == NULL ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "NULL Output Filename, unable to establish pipeline.");
        return FALSE;
    }

    if( isFramerateValid(framerate) == FALSE ) {
        LOG(DEBUG_LEVEL_FATAL, DBG_PIPELINE, "Invalid Framerate: %d, unable to establish pipeline.", framerate);
        return FALSE;
    }

    retval = SccFileInitialize(ctxPtr, inputFilename, framerate);
    if( retval == FALSE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
        return FALSE;
    }

    retval = SccFileAddSink(ctxPtr, SccEncodeInitialize(ctxPtr));
    if( retval == FALSE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
        return FALSE;
    }

    retval = SccEncodeAddSink(ctxPtr, Line21DecodeInitialize(ctxPtr, (artifacts == FALSE)));
    if( retval == FALSE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
        return FALSE;
    }

    if( artifacts == TRUE ) {
        retval = Line21DecodeAddSink(ctxPtr, Line21OutInitialize(ctxPtr, outputFilename));
        if (retval == FALSE) {
            LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = SccEncodeAddSink(ctxPtr, MccEncodeInitialize(ctxPtr));
        if (retval == FALSE) {
            LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = MccEncodeAddSink(ctxPtr, MccOutInitialize(ctxPtr, outputFilename));
        if (retval == FALSE) {
            LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = SccEncodeAddSink(ctxPtr, CcDataOutInitialize(ctxPtr, outputFilename));
        if (retval == FALSE) {
            LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }
    }

    return TRUE;
} // PlumbSccPipeline()

/*------------------------------------------------------------------------------
 | NAME:
 |    PlumbMccPipeline()
 |
 | INPUT PARAMETERS:
 |    inputFilename - Name of the input file (and root of the output filename).
 |    outputFilename - Name of the output file
 |
 | RETURN VALUES:
 |    Context - Context of this Pipeline
 |
 | DESCRIPTION:
 |    This method plumbs the pipeline to decode a MCC file and leave it's
 |    decoded text in a file that is <inputFilename>.608 and <inputFilename>.708
 |
 | PIPELINE:                                      +----------------+      +----------------+
 |                                           +--> | Line 21 Decode | -?-> | Line 21 Output |
 |                                           |    +----------------+      +----------------+
 |                                           |
 |    +--------------+      +------------+   |     +--------------+        +--------------+
 |    | Caption File | ---> | MCC Decode | --+---> | DTVCC Decode | --?--> | DTVCC Output |
 |    +--------------+      +------------+   |     +--------------+        +--------------+
 |                                           |
 |                                           |     +----------------+
 |                                           +-?-> | CC Data Output |
 |                                                 +----------------+
 -------------------------------------------------------------------------------*/
boolean PlumbMccPipeline( Context* ctxPtr, char* inputFilename, char* outputFilename, boolean artifacts ) {
    ASSERT(ctxPtr);
    memset(ctxPtr, 0, sizeof(Context));
    boolean retval;

    if( inputFilename == NULL ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "NULL Input Filename, unable to establish pipeline.");
        return FALSE;
    }

    if( outputFilename == NULL ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "NULL Output Filename, unable to establish pipeline.");
        return FALSE;
    }

    retval = MccFileInitialize(ctxPtr, inputFilename);
    if( retval == FALSE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
        return FALSE;
    }

    retval = MccFileAddSink(ctxPtr, MccDecodeInitialize(ctxPtr));
    if( retval == FALSE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
        return FALSE;
    }

    retval = MccDecodeAddSink(ctxPtr, DtvccDecodeInitialize(ctxPtr, (artifacts == FALSE)));
    if( retval == FALSE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
        return FALSE;
    }

    retval = MccDecodeAddSink(ctxPtr, Line21DecodeInitialize(ctxPtr, (artifacts == FALSE)));
    if (retval == FALSE) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
        return FALSE;
    }

    if( artifacts == TRUE ) {
        retval = DtvccDecodeAddSink(ctxPtr, DtvccOutInitialize(ctxPtr, outputFilename, TRUE, FALSE));
        if (retval == FALSE) {
            LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = Line21DecodeAddSink(ctxPtr, Line21OutInitialize(ctxPtr, outputFilename));
        if (retval == FALSE) {
            LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = MccDecodeAddSink(ctxPtr, CcDataOutInitialize(ctxPtr, outputFilename));
        if (retval == FALSE) {
            LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }
    }

    return TRUE;
} // PlumbMccPipeline()

/*------------------------------------------------------------------------------
 | NAME:
 |    PlumbMpegPipeline()
 |
 | INPUT PARAMETERS:
 |    inputFilename - Name of the input file (and root of the output filename).
 |    outputFilename - Name of the output file
 |    artifacts - Whether or not to save artifacts along with the MCC File.
 |    artifactPath - Path to save the artifacts (if configured).
 |    bailAtTwenty - Whether or not to stop processing at 20 mins if no text found.
 |
 | RETURN VALUES:
 |    Context - Context of this Pipeline
 |
 | DESCRIPTION:
 |    This method plumbs the pipeline to strip CC Data from an MPEG File and
 |    convert it to an MCC File. Additionally, if specified, it will decode
 |    the CC Data that is found in the asset and leave the decoded text in
 |    files that are <inputFilename>.608, <inputFilename>.708, and
 |    <inputFilename>.ccd. The output goes into the file <inputFilename>.mcc.
 |
 | PIPELINE:                 +------------+          +------------+
 |                     +---> | MCC Encode | -------> | MCC Output |
 |                     |     +------------+          +------------+
 |                     |
 |                     |     +----------------+      +----------------+
 |                     |---> | Line 21 Decode | -?-> | Line 21 Output |
 |     +-----------+   |     +----------------+      +----------------+
 |     | MPEG File | --|
 |     +-----------+   |     +--------------+        +--------------+
 |                     |---> | DTVCC Decode | --?--> | DTVCC Output |
 |                     |     +--------------+        +--------------+
 |                     |
 |                     |     +----------------+
 |                     +-?-> | CC Data Output |
 |                           +----------------+
 -------------------------------------------------------------------------------*/
boolean PlumbMpegPipeline( Context* ctxPtr, char* inputFilename, char* outputFilename, boolean artifacts, char* artifactPath, boolean bailAtTwenty ) {
    ASSERT(ctxPtr);
    memset(ctxPtr, 0, sizeof(Context));
    boolean retval;

    if( inputFilename == NULL ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "NULL Input Filename, unable to establish pipeline.");
        return FALSE;
    }

    if( outputFilename == NULL ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "NULL Output Filename, unable to establish pipeline.");
        return FALSE;
    }

    if( (artifacts == TRUE) && (artifactPath == NULL) ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "NULL Artifact Path, unable to establish pipeline.");
        return FALSE;
    }

    boolean isDropframe;
    boolean wasSuccessful = DetermineDropFrame(inputFilename, artifacts, artifactPath, &isDropframe);

    retval = MpegFileInitialize(ctxPtr, inputFilename, wasSuccessful, isDropframe, bailAtTwenty);
    if( retval == FALSE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
        return FALSE;
    }

    retval = MpegFileAddSink(ctxPtr, MccEncodeInitialize(ctxPtr));
    if( retval == FALSE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
        return FALSE;
    }

    retval = MpegFileAddSink(ctxPtr, DtvccDecodeInitialize(ctxPtr, (artifacts == FALSE)));
    if( retval == FALSE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
        return FALSE;
    }

    retval = MpegFileAddSink(ctxPtr, Line21DecodeInitialize(ctxPtr, (artifacts == FALSE)));
    if( retval == FALSE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
        return FALSE;
    }

    retval = MccEncodeAddSink(ctxPtr, MccOutInitialize(ctxPtr, outputFilename));
    if( retval == FALSE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
        return FALSE;
    }

    if( artifacts == TRUE ) {
        retval = DtvccDecodeAddSink(ctxPtr, DtvccOutInitialize(ctxPtr, artifactPath, TRUE, TRUE));
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = Line21DecodeAddSink(ctxPtr, Line21OutInitialize(ctxPtr, artifactPath));
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = MpegFileAddSink(ctxPtr, CcDataOutInitialize(ctxPtr, artifactPath));
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_PIPELINE, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }
    }

    return TRUE;
} // PlumbMpegPipeline()

/*------------------------------------------------------------------------------
 | NAME:
 |    DrivePipeline()
 |
 | INPUT PARAMETERS:
 |    sourceType - File Type feeding the pipeline.
 |    ctxPtr - Pointer to the Context.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method drives the pipeline, calling the source until there is nothing
 |    left to pass.
 -------------------------------------------------------------------------------*/
void DrivePipeline( FileType sourceType, Context* ctxPtr ) {
    boolean areWeDone = FALSE;
    boolean wasSuccessful;
    ASSERT(ctxPtr);

    while( areWeDone == FALSE ) {
        switch(sourceType) {
            case SCC_CAPTIONS_FILE:
                wasSuccessful = SccFileProcNextBuffer(ctxPtr, &areWeDone);
                break;
            case MCC_CAPTIONS_FILE:
                wasSuccessful = MccFileProcNextBuffer(ctxPtr, &areWeDone);
                break;
            case MPEG_BINARY_FILE:
                wasSuccessful = MpegFileProcNextBuffer(ctxPtr, &areWeDone);
                break;
            default:
                LOG(DEBUG_LEVEL_ERROR, DBG_GENERAL, "Impossible Branch - %d", sourceType);
                return;
        }

        if( wasSuccessful == PIPELINE_FAILURE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_GENERAL, "Error in Pipeline!");
        }
    }
} // DrivePipeline()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

