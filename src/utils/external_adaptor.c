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

#include "debug.h"

#include "external_adaptor.h"
#include "cc_utils.h"
#include "buffer_utils.h"
#include "autodetect_file.h"
#include "mpeg_file.h"
#include "scc_encode.h"
#include "version.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

static char versionStr[100];

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

EXTERNAL_608_FN_PTR external608DataCallbackFn = NULL;
EXTERNAL_708_FN_PTR external708DataCallbackFn = NULL;
EXTERNAL_END_FN_PTR externalEndOfDataCallbackFn = NULL;

Context rootContext;

LinkInfo linkInfo608;
LinkInfo linkInfo708;

uint8 numberOfShutdowns = 0;
FileType fileType = UNK_CAPTIONS_FILE;

boolean pipelineEstablished = FALSE;

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    ExtrnlAdptrGetVersion()
 |
 | INPUT PARAMETERS:
 |    None.
 |
 | RETURN VALUES:
 |    char* - Pointer to memory with the Version Number.
 |
 | DESCRIPTION:
 |    This method returns a pointer to ASCII containing the version number.
 -------------------------------------------------------------------------------*/
char* ExtrnlAdptrGetVersion( void ) {
    sprintf(versionStr, "Version: %s (%s)", VERSION, BUILD);
    return versionStr;
}  // ExtrnlAdptrGetVersion()

/*------------------------------------------------------------------------------
 | NAME:
 |    ExtrnlAdptrInitialize()
 |
 | INPUT PARAMETERS:
 |    cbFn608Data - Callback for the decoded CEA-608 Data.
 |    cbFn708Data - Callback for the decoded CEA-708 Data.
 |    cbFnEnd - Callback for the end of decoded data.
 |    filename - Input Filename (with path, if needed)
 |    framerate - Specified framerate, or 0 for autodetect.
 |
 | RETURN VALUES:
 |    boolean - TRUE is Successful and FALSE is a Failure
 |
 | DESCRIPTION:
 |    This adds another sink in the pipeline after this element.
 -------------------------------------------------------------------------------*/
boolean ExtrnlAdptrInitialize( EXTERNAL_608_FN_PTR cbFn608Data, EXTERNAL_708_FN_PTR cbFn708Data, EXTERNAL_END_FN_PTR cbFnEnd ) {
    
    if( IsDebugInitialized() == FALSE ) {
        LOG( DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Attempt to Initialize the External Adaptor without initializing debugging." );
        return FALSE;
    }

    if( IsDebugExternallyRouted() == FALSE ) {
        LOG( DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Attempt to Initialize the External Adaptor without debugging being piped to that adaptor." );
        return FALSE;
    }

    if( (cbFn608Data == NULL) || (cbFn708Data == NULL) || (cbFnEnd == NULL) ) {
        LOG( DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "608: %p or 708: %p or End: %p Callback not initialized.", cbFn608Data, cbFn708Data, cbFnEnd );
        return FALSE;
    }
 
    external608DataCallbackFn = cbFn608Data;
    external708DataCallbackFn = cbFn708Data;
    externalEndOfDataCallbackFn = cbFnEnd;

    linkInfo608.linkType = LINE21_DATA___EXTRNL_ADPTR;
    linkInfo608.sourceType = DATA_TYPE_DECODED_608;
    linkInfo608.sinkType = DATA_TYPE_EXTERNAL_ADAPTOR;
    linkInfo608.NextBufferFnPtr = &ExtrnlAdptr608OutProcNextBuffer;
    linkInfo608.ShutdownFnPtr = &ExtrnlAdptrShutdown;

    linkInfo708.linkType = DTVCC_DATA___EXTRNL_ADPTR;
    linkInfo708.sourceType = DATA_TYPE_DECODED_708;
    linkInfo708.sinkType = DATA_TYPE_EXTERNAL_ADAPTOR;
    linkInfo708.NextBufferFnPtr = &ExtrnlAdptr708OutProcNextBuffer;
    linkInfo708.ShutdownFnPtr = &ExtrnlAdptrShutdown;

    numberOfShutdowns = 0;

    BufferPoolInit();

    return TRUE;
}

/*------------------------------------------------------------------------------
 | NAME:
 |    ExtrnlAdptr608OutProcNextBuffer()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Unused Context Pointer. Not needed since we can't have
 |                 multiple External Adaptors. Needs to be here to conform to
 |                 standard function Protoptype.
 |    inBuffer - Pointer to the buffer to process.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method processes an incoming buffer, passing it to the External App.
 -------------------------------------------------------------------------------*/
boolean ExtrnlAdptr608OutProcNextBuffer( void* rootCtxPtr, Buffer* buffPtr ) {
    ASSERT(buffPtr);
    ASSERT(buffPtr->dataPtr);
    Line21Code* line21CodePtr = (Line21Code*)buffPtr->dataPtr;

    for( int loop = 0; loop < buffPtr->numElements; loop++ ) {
        if( line21CodePtr[loop].codeType != LINE21_NULL_DATA ) {
            (external608DataCallbackFn)(buffPtr->captionTime, line21CodePtr[loop]);
        }
    }

    FreeBuffer(buffPtr);
    return TRUE;
} // ExtrnlAdptr608OutProcNextBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    ExtrnlAdptr708OutProcNextBuffer()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Unused Context Pointer. Not needed since we can't have
 |                 multiple External Adaptors. Needs to be here to conform to
 |                 standard function Protoptype.
 |    inBuffer - Pointer to the buffer to process.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method processes an incoming buffer, passing it to the External App.
 -------------------------------------------------------------------------------*/
boolean ExtrnlAdptr708OutProcNextBuffer( void* rootCtxPtr, Buffer* buffPtr ) {
    ASSERT(buffPtr);
    ASSERT(buffPtr->dataPtr);
    DtvccData* dtvccDataPtr = (DtvccData*)buffPtr->dataPtr;

    for( int loop = 0; loop < buffPtr->numElements; loop++ ) {
        (external708DataCallbackFn)(buffPtr->captionTime, dtvccDataPtr[loop]);
    }

    FreeBuffer(buffPtr);
    return TRUE;
} // ExtrnlAdptr708OutProcNextBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    ExtrnlAdptrShutdown()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Unused Context Pointer. Not needed since we can't have
 |                 multiple External Adaptors. Needs to be here to conform to
 |                 standard function Protoptype.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method is called when the previous element in the pipeline determines
 |    that there is no more data coming down the pipeline. We know that this
 |    method will be called twice, since there are both a 608 and 708 handler.
 |    Thus we must ensure that we have received both calls before notifying the
 |    External Adaptor.
 -------------------------------------------------------------------------------*/
boolean ExtrnlAdptrShutdown( void* rootCtxPtr ) {

    numberOfShutdowns++;

    if( numberOfShutdowns == 2 ) {
        (externalEndOfDataCallbackFn)();
    }

    return TRUE;
} // ExtrnlAdptrShutdown()

/*------------------------------------------------------------------------------
 | NAME:
 |    ExtrnlAdptrPlumbFileDecodePipeline()
 |
 | INPUT PARAMETERS:
 |    inputFilename - Name of the input file (and root of the output filename).
 |    framerate - Framerate of file. Mandatory for SCC files. Ignored for all others.
 |
 | RETURN VALUES:
 |    Context - Context of this Pipeline
 |
 | DESCRIPTION:
 |    This method plumbs the pipeline to strip CC Data from an MPEG or MOV
 |    file, or process a SCC or MCC file, decoding the Caption Data and passing
 |    it to an external adaptor.
 |
 | PIPELINE:                 +----------------+      +------------------+
 |                     +---> | Line 21 Decode | ---> | External Adaptor |
 |    +-----------+    |     +----------------+      +------------------+
 |    | MPEG File | ---|
 |    +-----------+    |      +--------------+       +------------------+
 |                     +----> | DTVCC Decode | ----> | External Adaptor |
 |                            +--------------+       +------------------+
 |
 |    +----------+      +------------+      +----------------+      +------------------+
 |    | SCC File | ---> | SCC Encode | ---> | Line 21 Decode | ---> | External Adaptor |
 |    +----------+      +------------+      +----------------+      +------------------+
 |
 |                                              +----------------+      +------------------+
 |                                        +---> | Line 21 Decode | ---> | External Adaptor |
 |    +----------+      +------------+    |     +----------------+      +------------------+
 |    | MCC File | ---> | MCC Decode | ---|
 |    +----------+      +------------+    |      +--------------+       +------------------+
 |                                        +----> | DTVCC Decode | ----> | External Adaptor |
 |                                               +--------------+       +------------------+
 -------------------------------------------------------------------------------*/
boolean ExtrnlAdptrPlumbFileDecodePipeline( char* inputFilename, uint32 framerate ) {
#ifndef DONT_COMPILE_FFMPEG
    boolean isDropframe;
#endif
    boolean retval;

    pipelineEstablished = FALSE;

    LOG( DEBUG_LEVEL_INFO, DBG_EXT_ADPT, "Attempting to Plumb a Decode Pipeline for: %s", inputFilename);

    if( IsDebugInitialized() == FALSE ) {
        LOG( DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Attempt to plumb stack from an External Adaptor without initializing debugging." );
        return FALSE;
    }

    if( IsDebugExternallyRouted() == FALSE ) {
        LOG( DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Attempt to plumb stack from an External Adaptor without debugging being piped to that adaptor." );
        return FALSE;
    }

    if( (external608DataCallbackFn == NULL) || (external708DataCallbackFn == NULL) || (externalEndOfDataCallbackFn == NULL) ) {
        LOG( DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "608: %p or 708: %p or End: %p Callback not initialized.", external608DataCallbackFn, external708DataCallbackFn, externalEndOfDataCallbackFn );
        return FALSE;
    }

    if( inputFilename == NULL ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "NULL Input Filename, unable to establish pipeline.");
        return FALSE;
    }

    fileType = DetermineFileType( inputFilename );

    if( (fileType != MPEG_BINARY_FILE) && (fileType != SCC_CAPTIONS_FILE) && (fileType != MCC_CAPTIONS_FILE) ) {
        if( fileType < MAX_FILE_TYPE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Attempt to plumb stack with an unsupported File Type: %s", DECODE_CAPTION_FILE_TYPE(fileType));
        } else {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Attempt to plumb stack with an invalid File Type: %d", fileType);
        }
        return FALSE;
    }

    memset(&rootContext, 0, sizeof(Context));

    if( fileType == MPEG_BINARY_FILE ) {
#ifdef DONT_COMPILE_FFMPEG
        LOG(DEBUG_LEVEL_FATAL, DBG_EXT_ADPT, "Executable was compiled without FFMPEG, unable to process Binary MPEG File");
#else
        boolean wasSuccessful = DetermineDropFrame(inputFilename, FALSE, NULL, &isDropframe);
        retval = MpegFileInitialize(&rootContext, inputFilename, wasSuccessful, isDropframe);
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = MpegFileAddSink(&rootContext, Line21DecodeInitialize(&rootContext, FALSE));
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = MpegFileAddSink(&rootContext, DtvccDecodeInitialize(&rootContext, FALSE));
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }
        retval = Line21DecodeAddSink(&rootContext, linkInfo608);
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = DtvccDecodeAddSink(&rootContext, linkInfo708);
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }
#endif
    } else if( fileType == SCC_CAPTIONS_FILE ) {
        if( isFramerateValid(framerate) == TRUE ) {
            retval = SccFileInitialize(&rootContext, inputFilename, framerate);
            if( retval == FALSE ) {
                LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
                return FALSE;
            }

            retval = SccEncodeAddSink(&rootContext, Line21DecodeInitialize(&rootContext, FALSE));
            if( retval == FALSE ) {
                LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
                return FALSE;
            }

            retval = Line21DecodeAddSink(&rootContext, linkInfo608);
            if( retval == FALSE ) {
                LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
                return FALSE;
            }

            numberOfShutdowns = 1;
        } else {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Attempt to plumb stack for SCC File with an invalid Framerate: %d", framerate);
            return FALSE;
        }
    } else {
        retval = MccFileInitialize(&rootContext, inputFilename);
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = MccFileAddSink(&rootContext, MccDecodeInitialize(&rootContext));
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = MccDecodeAddSink(&rootContext, Line21DecodeInitialize(&rootContext, FALSE));
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = MccDecodeAddSink(&rootContext, DtvccDecodeInitialize(&rootContext, FALSE));
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = Line21DecodeAddSink(&rootContext, linkInfo608);
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }

        retval = DtvccDecodeAddSink(&rootContext, linkInfo708);
        if( retval == FALSE ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Problem Establishing Pipeline, bailing.");
            return FALSE;
        }
    }

    pipelineEstablished = TRUE;

    return TRUE;
} // ExtrnlAdptrPlumbFileDecodePipeline()

/*------------------------------------------------------------------------------
 | NAME:
 |    ExtrnlAdptrPlumbSccPipeline()
 |
 | INPUT PARAMETERS:
 |    inputFilename - Name of the input file (and root of the output filename).
 |    outputFilename - Name of the output file
 |    framerate - Framerate of the asset described in the SCC file.
 |
 | RETURN VALUES:
 |    boolean - Successful Call - TRUE; Failure - FALSE
 |
 | DESCRIPTION:
 |    This method plumbs the pipeline to decode a SCC file on behalf of an
 |    external entity.
 -------------------------------------------------------------------------------*/
boolean ExtrnlAdptrPlumbSccPipeline( char* inputFilename, char* outputFilename, uint32 framerate ) {
    memset(&rootContext, 0, sizeof(Context));
    pipelineEstablished = PlumbSccPipeline( &rootContext, inputFilename, outputFilename, framerate, TRUE );
    fileType = SCC_CAPTIONS_FILE;
    return pipelineEstablished;
} // ExtrnlAdptrPlumbSccPipeline()

/*------------------------------------------------------------------------------
 | NAME:
 |    ExtrnlAdptrPlumbMccPipeline()
 |
 | INPUT PARAMETERS:
 |    inputFilename - Name of the input file (and root of the output filename).
 |    outputFilename - Name of the output file
 |
 | RETURN VALUES:
 |    boolean - Successful Call - TRUE; Failure - FALSE
 |
 | DESCRIPTION:
 |    This method plumbs the pipeline to decode a MCC file on behalf of an
 |    external entity.
 -------------------------------------------------------------------------------*/
boolean ExtrnlAdptrPlumbMccPipeline( char* inputFilename, char* outputFilename ) {
    memset(&rootContext, 0, sizeof(Context));
    pipelineEstablished = PlumbMccPipeline( &rootContext, inputFilename, outputFilename, TRUE );
    fileType = MCC_CAPTIONS_FILE;
    return pipelineEstablished;
} // ExtrnlAdptrPlumbMccPipeline()

/*------------------------------------------------------------------------------
 | NAME:
 |    ExtrnlAdptrPlumbMpegPipeline()
 |
 | INPUT PARAMETERS:
 |    inputFilename - Name of the input file (and root of the output filename).
 |    outputFilename - Name of the output file
 |    artifacts - Whether or not to save artifacts along with the MCC File.
 |    artifactPath - Path to save the artifacts (if configured).
 |
 | RETURN VALUES:
 |    boolean - Successful Call - TRUE; Failure - FALSE
 |
 | DESCRIPTION:
 |    This method plumbs the pipeline to strip CC Data from an MPEG File and
 |    convert it to an MCC File on behalf of an external entity.
 -------------------------------------------------------------------------------*/
boolean ExtrnlAdptrPlumbMpegPipeline( char* inputFilename, char* outputFilename, boolean artifacts, char* artifactPath ) {
    memset(&rootContext, 0, sizeof(Context));
    pipelineEstablished = PlumbMpegPipeline( &rootContext, inputFilename, outputFilename, artifacts, artifactPath );
    fileType = MPEG_BINARY_FILE;
    return pipelineEstablished;
} // ExtrnlAdptrPlumbMpegPipeline()

/*------------------------------------------------------------------------------
 | NAME:
 |    ExtrnlAdptrDriveDecodePipeline()
 |
 | INPUT PARAMETERS:
 |    None.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method will drive the pipeline that was configured by the external
 |    entity.
 -------------------------------------------------------------------------------*/
void ExtrnlAdptrDriveDecodePipeline( void ) {
    if( pipelineEstablished == FALSE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_EXT_ADPT, "Attempt to drive a pipeline that is not successfully established");
    } else {
        DrivePipeline(fileType, &rootContext);
    }
} // ExtrnlAdptrDriveDecodePipeline()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

