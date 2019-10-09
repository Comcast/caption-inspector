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

#ifndef context_h
#define context_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef DONT_COMPILE_FFMPEG
#include <libavformat/avformat.h>
#endif

#ifdef COMPILE_GPAC
#include "gpac/isomedia.h"
#endif

#include "types.h"
#include "pipeline_structs.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

/* Quick Found Text State Machine */
#define NO_TEXT_FOUND                                        0
#define TEXT_FOUND                                           1
#define TEXT_REPORTED                                        2

/* Size Related Defines for the Specific Contexts */
#define MAX_FILE_NAME_LEN                                  256
#define LINE21_MAX_NUM_CHANNELS                              4
#define DTVCC_MAX_NUM_SERVICES                              63
#define LINE21_NUMBER_OF_FIELDS                              2

/* Bytes we can always look ahead without going out of limits. Buffer Size is 2 Mb plus the safety pass  */
#define ONEPASS                                            120
#define BUFSIZE                                            (2048*1024+ONEPASS)

#define DTVCC_MAX_PACKET_LENGTH                            128

/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

/* Sources */

typedef struct {
    Sinks sinks;
    char captionFileName[MAX_FILE_NAME_LEN];
    uint32 numCaptionsLinesRead;
    FILE* captionsFilePtr;
    uint32 frameRateTimesOneHundred;
} SccFileCtx;

typedef struct {
    Sinks sinks;
    char captionFileName[MAX_FILE_NAME_LEN];
    uint32 numCaptionsLinesRead;
    FILE* captionsFilePtr;
    uint32 frameRateTimesOneHundred;
    boolean isDropFrame;
// TODO - This is a kludge! Fully support the MCC 2.0 Stuff and remove this
    boolean oneShotWarningFlag;
} MccFileCtx;

typedef struct {
    Sinks sinks;
    char captionFileName[MAX_FILE_NAME_LEN];
    uint32 numCaptionsLinesRead;
    FILE* captionsFilePtr;
    uint32 frameRateTimesOneHundred;
    boolean isDropFrame;
} CcDataFileCtx;

#ifndef DONT_COMPILE_FFMPEG
typedef struct {
    Sinks sinks;
    AVFormatContext* formatContext;
    AVCodecContext* decoderContext;
    AVFrame* frame;
    int streamIndex;
    boolean isFileOpen;
    uint32 frameRatePerSecTimesOneHundred;
    boolean overrideDropframe;
    boolean isDropframe;
    boolean bailNoCaptions;
    int64 fileSize;
    uint8 buffer[BUFSIZE];
    uint32 len;
    int64 firstPts;
    uint32 ccCountMismatchErrors;
} MpegFileCtx;
#endif

typedef struct {
    Sinks sinks;
#ifdef COMPILE_GPAC
    GF_ISOFile* movFile;
#endif
    uint32 trackCount;
    uint32 currentTrack;
    uint32 currentSample;
    uint32 avcTrackCount;
    uint32 ccTrackCount;
    uint32 currentType;
    uint32 currentSubtype;
    uint32 numSamples;
    uint32 processingStreamDescriptionIndex;
    uint32 timescale;
    boolean framerateOneshot;
    boolean overrideDropframe;
    boolean isDropframe;
    boolean bailNoCaptions;
} MovFileCtx;

/* Transforms */

typedef struct {
    Sinks sinks;
    boolean processOnly;
    uint8 foundText;
    uint8 decodeWhichChannels;
    uint8 currentChannel[LINE21_NUMBER_OF_FIELDS+1];
    boolean dataFound[LINE21_MAX_NUM_CHANNELS+1];
    boolean isPopOnCaptioning[LINE21_MAX_NUM_CHANNELS+1];
    boolean isRollUpCaptioning[LINE21_MAX_NUM_CHANNELS+1];
    boolean isPaintOnCaptioning[LINE21_MAX_NUM_CHANNELS+1];
    boolean captioningChange[LINE21_MAX_NUM_CHANNELS+1];
    boolean captioningStart[LINE21_MAX_NUM_CHANNELS+1];
} Line21DecodeCtx;

typedef struct {
    Sinks sinks;
    boolean processOnly;
    boolean processedFine;
    boolean firstPacket;
    uint8 foundText;
    uint32 numP16Cmds;
    uint32 pktLenMismatches;
    uint8 dtvccPacket[DTVCC_MAX_PACKET_LENGTH];
    uint8 dtvccPacketLength;
    int8 lastSequence;
    uint64 activeServices;
    boolean isPopOnCaptioning[DTVCC_MAX_NUM_SERVICES];
    boolean isRollUpCaptioning[DTVCC_MAX_NUM_SERVICES];
    boolean isTickerCaptioning[DTVCC_MAX_NUM_SERVICES];
    boolean captioningChange[DTVCC_MAX_NUM_SERVICES];
    boolean captioningStart[DTVCC_MAX_NUM_SERVICES];
} DtvccDecodeCtx;

typedef struct {
    Sinks sinks;
    uint32 numCcCountMismatches;
} MccDecodeCtx;

typedef struct {
    Sinks sinks;
    boolean headerPrinted;
    uint16 cdpHeaderSequence;
    CaptionTime nextCaptionTime;
} MccEncodeCtx;

typedef struct {
    Sinks sinks;
    uint32 nextFrameNum;
    uint32 hourAdjust;
    uint32 sccFrameRate;
} SccEncodeCtx;

/* Sinks */

typedef struct {
    Sinks sinks;
} ExtrnlAdptrCtx;

typedef struct {
    FILE* fp;
    boolean wasHeaderWritten;
    uint8 currentChannel[LINE21_NUMBER_OF_FIELDS];
    uint8 currentService;
    uint16 cea708ErrNum;
    uint8 cea708State;
    uint8 cea708Code;
    uint8 cea708BytesRemaining;
    char ccdFileName[MAX_FILE_NAME_LEN];
} CcDataOutputCtx;

typedef struct {
    FILE* fp[LINE21_MAX_NUM_CHANNELS];
    boolean textStream[LINE21_MAX_NUM_CHANNELS];
    uint8 outputWhichChannel;
    char baseFileName[MAX_FILE_NAME_LEN];
} Line21OutputCtx;

typedef struct {
    FILE* fp[DTVCC_MAX_NUM_SERVICES];
    boolean textStream[DTVCC_MAX_NUM_SERVICES];
    boolean suppressEtx;
    boolean suppressNull;
    boolean printMsNotFrame;
    uint8 outputWhichService;
    char baseFileName[MAX_FILE_NAME_LEN];
} DtvccOutputCtx;

typedef struct {
    FILE* fp;
    char mccFileName[MAX_FILE_NAME_LEN];
} MccOutputCtx;

typedef struct {
    SccFileCtx* sccFileCtxPtr;
    MccFileCtx* mccFileCtxPtr;
    CcDataFileCtx* ccDataFileCtxPtr;
#ifndef DONT_COMPILE_FFMPEG
    MpegFileCtx* mpegFileCtxPtr;
#endif
    MovFileCtx* movFileCtxPtr;
    Line21DecodeCtx* line21DecodeCtxPtr;
    DtvccDecodeCtx* dtvccDecodeCtxPtr;
    MccDecodeCtx* mccDecodeCtxPtr;
    MccEncodeCtx* mccEncodeCtxPtr;
    SccEncodeCtx* sccEncodeCtxPtr;
    ExtrnlAdptrCtx* extrnlAdptrCtxPtr;
    CcDataOutputCtx* ccDataOutputCtxPtr;
    Line21OutputCtx* line21OutputCtxPtr;
    DtvccOutputCtx* dtvccOutputCtxPtr;
    MccOutputCtx* mccOutputCtxPtr;
} Context;

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/

#endif /* context_h */
