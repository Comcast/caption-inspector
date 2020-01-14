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
#include "dtvcc_output.h"
#include "dtvcc_decode.h"
#include "output_utils.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

const char* normalizedC0CmdSetShortName[33] = {
    "NUL", "RSV", "RSV", "ETX", "RSV", "RSV", "RSV", "RSV", "BS", "RSV", "RSV", "RSV",
    "FF", "CR", "HCR", "RSV", "EXT1", "RSV", "RSV", "RSV", "RSV", "RSV", "RSV",
    "RSV", "P16", "RSV", "RSV", "RSV", "RSV", "RSV", "RSV", "RSV", "UNK"
};

const char* normalizedC1CmdSetShortName[33] = {
    "CW0", "CW1", "CW2", "CW3", "CW4", "CW5", "CW6", "CW7", "CLW", "DSW", "HDW", "TGW",
    "DLW", "DLY", "DLC", "RST", "SPA", "SPC", "SPL", "RSV93", "RSV94", "RSV95", "RSV96",
    "SWA", "DF0", "DF1", "DF2", "DF3", "DF4", "DF5", "DF6", "DF7", "UNK"
};

const char* normalizedG0CharSet[96] = {
    " ", "!", "\"", "#", "$", "%", "&", "\'", "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "♩"
};

const char* normalizedG1CharSet[96] = {
    " ",  "¡",  "￠",  "￡",  "¤",  "￥",  "¦",  "§",  "¨",  "©",  "ª",  "«",  "￢", "-",  "®",  "￣",
    "°",  "±",  "²",  "³",   "´",  "µ",   "¶",  "·",  "¸",  "¹",  "º",  "»",  "¼",  "½",  "¾",  "¿",
    "À",  "Á",  "Â",  "Ã",   "Ä",  "Å",   "Æ",  "Ç",  "È",  "É",  "Ê",  "Ë",  "Ì",  "Í",  "Î",  "Ï",
    "Ð",  "Ñ",  "Ò",  "Ó",   "Ô",  "Õ",   "Ö",  "×",  "Ø",  "Ù",  "Ú",  "Û",  "Ü",  "Ý",  "Þ",  "ß",
    "à",  "á",  "â",  "ã",   "ä",  "å",   "æ",  "ç",  "è",  "é",  "ê",  "ë",  "ì",  "í",  "î",  "ï",
    "ð",  "ñ",  "ò",  "ó",   "ô",  "õ",   "÷",  "ø",  "ù",  "ú",  "ü",  "û",  "ü",  "ý",  "þ",  "ÿ"
};

const char* textTags[17] = {
    "Dialog", "Source or Speaker ID", "Electronically Reproduced Voice", "Dialog in Other Language",
    "Voiceover", "Audible Translation", "Subtitle Translation", "Voice Quality Description",
    "Song Lyrics", "Sound Effect Description", "Musical Score Description", "Oath", "Undefined 0",
    "Undefined 1", "Undefined 2", "Invisible", "Unknown"
};

const char* penOffset[5] = {
    "Subscript", "Normal", "Superscript", "Illegal Value", "Unknown"
};

const char* penSize[5] = {
    "Small", "Standard", "Large", "Illegal Value", "Unknown"
};

const char* predefPenStyle[8] = {
    "Default", "MonoSerif", "ProportSerif", "MonoSanSerif", "ProportSanSerif", "MonoSanSerif-TransBG", "ProportSanSerif-TransBG", "Unknown"
};

const char* edgeType[9] = {
    "None", "Raised", "Depressed", "Uniform", "Left Drop Shadow", "Right Drop Shadow",
    "Illegal Value 0", "Illegal Value 1", "Unknown"
};

const char* fontTag[9] = {
    "Default", "Monospaced Serif", "Proportional Serif", "Monospaced Sans-Serif",
    "Proportional Sans-Serif", "Casual", "Cursive", "Small Caps", "Unknown"
};

const char* opacity[5] = {
    "Solid", "Flash", "Translucent", "Transparent", "Unknown"
};

const char* borderType[7] = {
    "None", "Raised", "Depressed", "Uniform", "Shadow Left", "Shadow Right", "Unknown"
};

const char* direction[5] = {
    "LtoR", "RtoL", "TtoB", "BtoT", "Unk"
};

const char* justifyDirection[5] = {
    "L/T", "R/B", "C", "F", "Unk"
};

const char* effectSpeed[17] = {
    "0.0", "0.5", "1.0", "1.5", "2.0", "2.5", "3.0", "3.5", "4.0", "4.5", "5.0", "5.5",
    "6.0", "6.5", "7.0", "7.5", "???"
};

const char* dispEffect[4] = {
    "Snap", "Fade", "Wipe", "Unk"
};

const char* anchorPos[10] = {
    "UL", "UC", "UR", "ML", "MC", "MR", "LL", "LC", "LR", "??"
};

const char* predefWinStyle[8] = {
    "608-PopUp", "PopUp-TransBG", "PopUp-Cntrd", "608-RollUp", "RollUp-TransBG", "RollUp-Cntrd", "TickerTape", "Unknown"
};

const char* BARF = "🤮";

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

static void writeDtvccData( DtvccOutputCtx*, CaptionTime*, DtvccData* );
static void displayC1Cmd( DtvccOutputCtx*, CaptionTime*, C1Command*, DtvccData* );
static void writeTimeStamp( DtvccOutputCtx*, CaptionTime*, uint8 );

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
 | NAME:
 |    DtvccOutInitialize()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    outputFileNameStr - The name of the file to write the data to.
 |    nullEtxSuppressed - Whether or not to suppress the annoying NULL or End of Text Marker.
 |
 | RETURN VALUES:
 |    LinkInfo -  Information about this element of the pipeline, such that it can
 |                be chained to elements that can consume the specific type of data
 |                that it produces.
 |
 | DESCRIPTION:
 |    This initializes this element of the pipeline.
 -------------------------------------------------------------------------------*/
LinkInfo DtvccOutInitialize( Context* rootCtxPtr, boolean nullEtxSuppressed, boolean msNotFrame ) {
    ASSERT(rootCtxPtr);
    ASSERT(!rootCtxPtr->dtvccOutputCtxPtr);

    rootCtxPtr->dtvccOutputCtxPtr = malloc(sizeof(DtvccOutputCtx));
    DtvccOutputCtx* ctxPtr = rootCtxPtr->dtvccOutputCtxPtr;

    ctxPtr->suppressEtx = nullEtxSuppressed;
    ctxPtr->suppressNull = nullEtxSuppressed;
    ctxPtr->printMsNotFrame = msNotFrame;
    for( int loop = 0; loop < DTVCC_MAX_NUM_SERVICES; loop++ ) {
        ctxPtr->textStream[loop] = FALSE;
        ctxPtr->fp[loop] = NULL;
    }

    buildOutputPath(rootCtxPtr->config.inputFilename, rootCtxPtr->config.outputDirectory, "708", ctxPtr->outputFileName);

    LinkInfo linkInfo;
    linkInfo.linkType = DTVCC_DATA___TEXT_FILE;
    linkInfo.sourceType = DATA_TYPE_DECODED_708;
    linkInfo.sinkType = DATA_TYPE_708_TXT_FILE;
    linkInfo.NextBufferFnPtr = &DtvccOutProcNextBuffer;
    linkInfo.ShutdownFnPtr = &DtvccOutShutdown;
    return linkInfo;
}  // DtvccOutInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    DtvccOutProcNextBuffer()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Context Pointer.
 |    bufferPtr - Pointer to the buffer to process.
 |
 | RETURN VALUES:
 |    uint8 - Success is TRUE / PIPELINE_SUCCESS, Failure is FALSE / PIPELINE_FAILURE
 |            All other codes specified in header.
 |
 | DESCRIPTION:
 |    This method processes an incoming buffer, writing the contents into an
 |    ASCII file.
 -------------------------------------------------------------------------------*/
uint8 DtvccOutProcNextBuffer( void* rootCtxPtr, Buffer* buffPtr ) {
    ASSERT(buffPtr);
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->dtvccOutputCtxPtr);
    DtvccData* dtvccDataPtr = (DtvccData*)buffPtr->dataPtr;
    DtvccOutputCtx* ctxPtr = ((Context*)rootCtxPtr)->dtvccOutputCtxPtr;

    if( ctxPtr->fp[dtvccDataPtr->serviceNumber-1] == NULL ) {
        char uniqueFileName[MAX_FILE_NAME_LEN];
        strncpy(uniqueFileName, ctxPtr->outputFileName, MAX_FILE_NAME_LEN);
        char* tmpCharPtr = strrchr(uniqueFileName, '.');
        ASSERT(tmpCharPtr);
        *tmpCharPtr = '\0';
        char suffix[10];
        sprintf(suffix, "-S%d.708", dtvccDataPtr->serviceNumber);
        strncat(uniqueFileName, suffix, (MAX_FILE_NAME_LEN - strlen(uniqueFileName)));
        ctxPtr->fp[dtvccDataPtr->serviceNumber-1] = fileOutputInit(uniqueFileName);
        writeToFile(ctxPtr->fp[dtvccDataPtr->serviceNumber-1], "Decoded DTVCC / CEA-708 for Asset: %s - Service: %d", uniqueFileName, dtvccDataPtr->serviceNumber);
        LOG(DEBUG_LEVEL_INFO, DBG_708_OUT, "Creating new DTVCC File for Output: %s", uniqueFileName);
    }

    for( int loop = 0; loop < buffPtr->numElements; loop++ ) {
        writeDtvccData( ctxPtr, &buffPtr->captionTime, &dtvccDataPtr[loop] );
    }
    
    FreeBuffer(buffPtr);
    return PIPELINE_SUCCESS;
} // DtvccOutProcNextBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    DtvccOutShutdown()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Context Pointer.
 |
 | RETURN VALUES:
 |    uint8 - Success is TRUE / PIPELINE_SUCCESS, Failure is FALSE / PIPELINE_FAILURE
 |            All other codes specified in header.
 |
 | DESCRIPTION:
 |    This method is called when the previous element in the pipeline determines
 |    that there is no more data coming down the pipline. This element will
 |    perform any necessary actions as a result and pass this call down the
 |    pipeline.
 -------------------------------------------------------------------------------*/
uint8 DtvccOutShutdown( void* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->dtvccOutputCtxPtr);
    boolean anyFiles = FALSE;

    for( int loop = 0; loop < DTVCC_MAX_NUM_SERVICES; loop++ ) {
        if( ((Context*)rootCtxPtr)->dtvccOutputCtxPtr->fp[loop] != NULL ) {
            closeFile(((Context *) rootCtxPtr)->dtvccOutputCtxPtr->fp[loop]);
            anyFiles = TRUE;
        }
    }

    if( anyFiles == FALSE ) {
        LOG( DEBUG_LEVEL_WARN, DBG_708_OUT, "No 708 Data Outputted. No Decoded 708 File Written." );
    }

    free(((Context*)rootCtxPtr)->dtvccOutputCtxPtr);
    ((Context*)rootCtxPtr)->dtvccOutputCtxPtr = NULL;
    return PIPELINE_SUCCESS;
} // DtvcceOutShutdown()


/*------------------------------------------------------------------------------
 | NAME:
 |    DtvccDecodeG0CharSet()
 |
 | INPUT PARAMETERS:
 |    code - G0 Character Code to decode into a Character.
 |
 | RETURN VALUES:
 |    char* - UTF-8 encoded version of the decoded G0 Character Code.
 |
 | DESCRIPTION:
 |    This method decodes G0 Chars into UTF-8.
 -------------------------------------------------------------------------------*/
const char* DtvccDecodeG0CharSet(uint8 c) {
  if ((c>0x1F) && (c<0x80)) {
    return normalizedG0CharSet[(c - 0x20)];
  } else {
    return BARF;
  }
}  // DecodeG0CharSet()

/*------------------------------------------------------------------------------
 | NAME:
 |    DtvccDecodeG1CharSet()
 |
 | INPUT PARAMETERS:
 |    code - G1 Character Code to decode into a Character.
 |
 | RETURN VALUES:
 |    char* - UTF-8 encoded version of the decoded G1 Character Code.
 |
 | DESCRIPTION:
 |    This method decodes G1 Chars into UTF-8.
 -------------------------------------------------------------------------------*/
const char* DtvccDecodeG1CharSet(uint8 c) {
  if ((c>0x9F) && (c<0xFF)) {
    return normalizedG1CharSet[(c - 0xA0)];
  } else {
    return BARF;
  }
}  // DecodeG1CharSet()

/*------------------------------------------------------------------------------
 | NAME:
 |    DtvccDecodeG2CharSet()
 |
 | INPUT PARAMETERS:
 |    code - G2 Character Code to decode into a Character.
 |
 | RETURN VALUES:
 |    char* - 'Long Char' of the decoded G2 Character Code.
 |
 | DESCRIPTION:
 |    This method decodes G2 Chars. It differs from the G0/G1 functions because
 |    the lack of contiguous characters makes this representation the easiest
 |    way to write the code.
 -------------------------------------------------------------------------------*/
const char* DtvccDecodeG2CharSet( uint8 code ) {
    switch( code ) {
        case 0x20:
            return " ";  // Transparent Space
        case 0x21:
            return " ";  // Non-Breaking Transparent Space
        case 0x25:
            return "…";
        case 0x2A:
            return "Š";
        case 0x2C:
            return "Œ";
        case 0x30:
            return "█";  // Solid Block which fills the entire Characater Block
        case 0x31:
            return "\'";
        case 0x32:
            return "\'";
        case 0x33:
            return "“";
        case 0x34:
            return "”";
        case 0x35:
            return "•";
        case 0x39:
            return "™";
        case 0x3A:
            return "š";
        case 0x3C:
            return "œ";
        case 0x3D:
            return "℠";
        case 0x3F:
            return "Ÿ";
        case 0x76:
            return "⅛";
        case 0x77:
            return "⅜";
        case 0x78:
            return "⅝";
        case 0x79:
            return "⅞";
        case 0x7A:
            return "│";
        case 0x7B:
            return "┐";
        case 0x7C:
            return "└";
        case 0x7D:
            return "─";
        case 0x7E:
            return "┘";
        case 0x7F:
            return "┌";
    }
    return BARF;
}  // DecodeG2CharSet()

/*------------------------------------------------------------------------------
 | NAME:
 |    DtvccDecodeG3CharSet()
 |
 | INPUT PARAMETERS:
 |    code - G3 Character Code to decode into a Character.
 |
 | RETURN VALUES:
 |    char* - 'Long Char' of the decoded G3 Character Code.
 |
 | DESCRIPTION:
 |    This method decodes G3 Chars. It differs from the G0/G1 functions because
 |    there is only one G3 character.
 -------------------------------------------------------------------------------*/
const char* DtvccDecodeG3CharSet( uint8 code ) {
    if( code == DTVCC_G3_CC_ICON ) {
        return "㏄";  // Supposed to be '[㏄]' but there is no char for that.
    } else {
        return BARF;
    }
}  // DecodeG3CharSet()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    writeDtvccData()
 |
 | DESCRIPTION:
 |    This method takes decoded DTVCC Data and writes it to the file or the
 |    screen.
 -------------------------------------------------------------------------------*/
static void writeDtvccData( DtvccOutputCtx* ctxPtr, CaptionTime* captionTimePtr, DtvccData* dtvccDataPtr ) {
    ASSERT(dtvccDataPtr);
    ASSERT(captionTimePtr);
    ASSERT(ctxPtr);
    uint8 fileIdx = dtvccDataPtr->serviceNumber - 1;
    
    boolean isChar = ((dtvccDataPtr->dtvccType == DTVCC_G0_CHAR) || (dtvccDataPtr->dtvccType == DTVCC_G1_CHAR) ||
                      (dtvccDataPtr->dtvccType == DTVCC_G2_CHAR) || (dtvccDataPtr->dtvccType == DTVCC_G3_CHAR));
    
    if( (dtvccDataPtr->dtvccType == DTVCC_C0_CMD) && (ctxPtr->textStream[dtvccDataPtr->serviceNumber] == TRUE) &&
        (dtvccDataPtr->data.c0cmd.c0CmdCode == DTVCC_C0_ETX) && (ctxPtr->suppressEtx == TRUE) ) {
        isChar = TRUE;
    }

    if( (dtvccDataPtr->dtvccType == DTVCC_C0_CMD) && (ctxPtr->textStream[dtvccDataPtr->serviceNumber] == TRUE) &&
        (dtvccDataPtr->data.c0cmd.c0CmdCode == DTVCC_C0_NUL) && (ctxPtr->suppressNull == TRUE) ) {
        isChar = TRUE;
    }

    if( (isChar == TRUE) && (ctxPtr->textStream[dtvccDataPtr->serviceNumber] == FALSE) ) {
        writeToFile(ctxPtr->fp[fileIdx], " \"");
        ctxPtr->textStream[dtvccDataPtr->serviceNumber] = TRUE;
    } else if( (isChar == FALSE) && (ctxPtr->textStream[dtvccDataPtr->serviceNumber] == TRUE) ) {
        writeToFile(ctxPtr->fp[fileIdx], "\"");
        ctxPtr->textStream[dtvccDataPtr->serviceNumber] = FALSE;
    }
    
    switch(dtvccDataPtr->dtvccType) {
        case DTVCC_C0_CMD:
            if( (dtvccDataPtr->data.c0cmd.c0CmdCode == DTVCC_C0_NUL) ||
               (dtvccDataPtr->data.c0cmd.c0CmdCode == DTVCC_C0_ETX) ||
               (dtvccDataPtr->data.c0cmd.c0CmdCode == DTVCC_C0_BS)  ||
               (dtvccDataPtr->data.c0cmd.c0CmdCode == DTVCC_C0_FF)  ||
               (dtvccDataPtr->data.c0cmd.c0CmdCode == DTVCC_C0_CR)  ||
               (dtvccDataPtr->data.c0cmd.c0CmdCode == DTVCC_C0_HCR) ) {
                if( (((dtvccDataPtr->data.c0cmd.c0CmdCode != DTVCC_C0_ETX) || (ctxPtr->suppressEtx == FALSE)) &&
                     ((dtvccDataPtr->data.c0cmd.c0CmdCode != DTVCC_C0_NUL) || (ctxPtr->suppressNull == FALSE))) ||
                    (ctxPtr->textStream[dtvccDataPtr->serviceNumber] == FALSE) ) {
                    writeToFile(ctxPtr->fp[fileIdx], " {%s}", DtvccDecodeC0Cmd(dtvccDataPtr->data.c0cmd.c0CmdCode));
                }
            } else if( dtvccDataPtr->data.c0cmd.c0CmdCode == DTVCC_C0_P16 ) {
                writeToFile(ctxPtr->fp[fileIdx], " {P16:0x%02X%02X}", dtvccDataPtr->data.c0cmd.pe16sym1, dtvccDataPtr->data.c0cmd.pe16sym2);
            }
            break;
        case DTVCC_C1_CMD:
            displayC1Cmd(ctxPtr, captionTimePtr, &dtvccDataPtr->data.c1cmd, dtvccDataPtr);
            break;
        case DTVCC_C2_CMD:
            writeToFile(ctxPtr->fp[fileIdx], " {C2Cmd}");
            break;
        case DTVCC_C3_CMD:
            writeToFile(ctxPtr->fp[fileIdx], " {C3Cmd}");
            break;
        case DTVCC_G0_CHAR:
            writeToFile(ctxPtr->fp[fileIdx], "%s", DtvccDecodeG0CharSet(dtvccDataPtr->data.g0char));
            break;
        case DTVCC_G1_CHAR:
            writeToFile(ctxPtr->fp[fileIdx], "%s", DtvccDecodeG1CharSet(dtvccDataPtr->data.g1char));
            break;
        case DTVCC_G2_CHAR:
            writeToFile(ctxPtr->fp[fileIdx], "%s", DtvccDecodeG2CharSet(dtvccDataPtr->data.g2char));
            break;
        case DTVCC_G3_CHAR:
            writeToFile(ctxPtr->fp[fileIdx], "%s", DtvccDecodeG3CharSet(dtvccDataPtr->data.g3char));
            break;
        case DTVCC_IGNORE_CMD_OR_CHAR:
            break;
        default:
            LOG( DEBUG_LEVEL_ERROR, DBG_708_OUT, "Unreachable Branch", dtvccDataPtr->dtvccType );
            break;
    }
    
} // writeDtvccData()

/*------------------------------------------------------------------------------
 | NAME:
 |    displayC1Cmd()
 |
 | DESCRIPTION:
 |    This function renders the decoded C1 Command and displays it in stdio
 |    or writes it to a file, depending on if a file was specified.
 -------------------------------------------------------------------------------*/
static void displayC1Cmd( DtvccOutputCtx* ctxPtr, CaptionTime* captionTimePtr, C1Command* c1CmdPtr, DtvccData* dtvccDataPtr ) {
    ASSERT(dtvccDataPtr);
    ASSERT(c1CmdPtr);
    ASSERT(captionTimePtr);
    ASSERT(ctxPtr);
    uint8 fileIdx = dtvccDataPtr->serviceNumber - 1;

    // --- Set Current Window {CWx} --- DelayCancel {DLC} --- Reset {RST} --- Reserved {RSVxx} ---
    if( (c1CmdPtr->c1CmdCode == DTVCC_C1_CW0)   || (c1CmdPtr->c1CmdCode == DTVCC_C1_CW1)   ||
        (c1CmdPtr->c1CmdCode == DTVCC_C1_CW2)   || (c1CmdPtr->c1CmdCode == DTVCC_C1_CW3)   ||
        (c1CmdPtr->c1CmdCode == DTVCC_C1_CW4)   || (c1CmdPtr->c1CmdCode == DTVCC_C1_CW5)   ||
        (c1CmdPtr->c1CmdCode == DTVCC_C1_CW6)   || (c1CmdPtr->c1CmdCode == DTVCC_C1_CW7)   ||
        (c1CmdPtr->c1CmdCode == DTVCC_C1_DLC)   || (c1CmdPtr->c1CmdCode == DTVCC_C1_RST)   ||
        (c1CmdPtr->c1CmdCode == DTVCC_C1_RSV93) || (c1CmdPtr->c1CmdCode == DTVCC_C1_RSV94) ||
        (c1CmdPtr->c1CmdCode == DTVCC_C1_RSV95) || (c1CmdPtr->c1CmdCode == DTVCC_C1_RSV96) ) {
        writeToFile(ctxPtr->fp[fileIdx], " {%s}", DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode));
    // --- Clear Windows {CLW} --- Display Windows {DSW} --- HideWindows {HDW} ---
    // --- ToggleWindows {TGW} --- Delete Windows {DLW} ---
    } else if( (c1CmdPtr->c1CmdCode == DTVCC_C1_CLW) || (c1CmdPtr->c1CmdCode == DTVCC_C1_DSW) ||
               (c1CmdPtr->c1CmdCode == DTVCC_C1_HDW) || (c1CmdPtr->c1CmdCode == DTVCC_C1_TGW) ||
               (c1CmdPtr->c1CmdCode == DTVCC_C1_DLW) ) {
        writeTimeStamp(ctxPtr, captionTimePtr, dtvccDataPtr->serviceNumber);
        writeToFile(ctxPtr->fp[fileIdx], " {%s:%s}", DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode), uint8toBitArray(c1CmdPtr->cmdData.windowBitmap));
    // --- Delay {DLY} in 100ms ---
    } else if( c1CmdPtr->c1CmdCode == DTVCC_C1_DLY ) {
        writeToFile(ctxPtr->fp[fileIdx], " {%s:%d00ms}", DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode), c1CmdPtr->cmdData.tenthsOfaSec);
    // --- Set Pen Attributes {SPA} ---
    } else if( c1CmdPtr->c1CmdCode == DTVCC_C1_SPA ) {
        writeToFile(ctxPtr->fp[fileIdx], " {%s:Pen-[Size:%s,Offset:%s]:TextTag-%s:FontTag-%s:EdgeType-%s",
                    DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode),
                    DECODE_PEN_SIZE(c1CmdPtr->cmdData.penAttributes.penSize),
                    DECODE_PEN_OFFSET(c1CmdPtr->cmdData.penAttributes.penOffset),
                    DECODE_TEXT_TAG(c1CmdPtr->cmdData.penAttributes.textTag),
                    DECODE_FONT_TAG(c1CmdPtr->cmdData.penAttributes.fontTag),
                    DECODE_EDGE_TYPE(c1CmdPtr->cmdData.penAttributes.edgeType));
        if( c1CmdPtr->cmdData.penAttributes.isUnderlined == TRUE ) writeToFile(ctxPtr->fp[fileIdx], ":UL");
        if( c1CmdPtr->cmdData.penAttributes.isItalic == TRUE ) writeToFile(ctxPtr->fp[fileIdx], ":IT");
        writeToFile(ctxPtr->fp[fileIdx], "}");
    // --- Set Pen Color {SPC} ---
    } else if( c1CmdPtr->c1CmdCode == DTVCC_C1_SPC ) {
        writeToFile(ctxPtr->fp[fileIdx], " {%s:FG-%s-R%dG%dB%d:BG-%s-R%dG%dB%d:Edg-R%dG%dB%d}",
                    DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode),
                    DECODE_OPACITY(c1CmdPtr->cmdData.penColor.fgOpacity),
                    c1CmdPtr->cmdData.penColor.fgRed,
                    c1CmdPtr->cmdData.penColor.fgGreen,
                    c1CmdPtr->cmdData.penColor.fgBlue,
                    DECODE_OPACITY(c1CmdPtr->cmdData.penColor.bgOpacity),
                    c1CmdPtr->cmdData.penColor.bgRed,
                    c1CmdPtr->cmdData.penColor.bgGreen,
                    c1CmdPtr->cmdData.penColor.bgBlue,
                    c1CmdPtr->cmdData.penColor.edgeRed,
                    c1CmdPtr->cmdData.penColor.edgeGreen,
                    c1CmdPtr->cmdData.penColor.edgeBlue);
    // --- Set Pen Location {SPL} ---
    } else if( c1CmdPtr->c1CmdCode == DTVCC_C1_SPL ) {
        writeToFile(ctxPtr->fp[fileIdx], " {%s:R%d-C%d}", DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode),
                    c1CmdPtr->cmdData.penLocation.row, c1CmdPtr->cmdData.penLocation.column);
    // --- Set Window Attributes {SWA} ---
    } else if( c1CmdPtr->c1CmdCode == DTVCC_C1_SWA ) {
        writeToFile(ctxPtr->fp[fileIdx], " {%s:Fill-%s-R%dG%dB%d:Brdr-%s-R%dG%dB%d:PD-%s:SD-%s:JD-%s:%s-%ssec-%s",
                    DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode),
                    DECODE_OPACITY(c1CmdPtr->cmdData.winAttributes.fillOpacity),
                    c1CmdPtr->cmdData.winAttributes.fillRedColorComp,
                    c1CmdPtr->cmdData.winAttributes.fillGreenColorComp,
                    c1CmdPtr->cmdData.winAttributes.fillBlueColorComp,
                    DECODE_BORDER_TYPE(c1CmdPtr->cmdData.winAttributes.borderType),
                    c1CmdPtr->cmdData.winAttributes.borderRed,
                    c1CmdPtr->cmdData.winAttributes.borderGreen,
                    c1CmdPtr->cmdData.winAttributes.borderBlue,
                    DECODE_DIRECTION(c1CmdPtr->cmdData.winAttributes.printDirection),
                    DECODE_DIRECTION(c1CmdPtr->cmdData.winAttributes.scrollDirection),
                    DECODE_DIRECTION(c1CmdPtr->cmdData.winAttributes.justifyDirection),
                    DECODE_DISPLAY_EFFECT(c1CmdPtr->cmdData.winAttributes.displayEffect),
                    DECODE_EFFECT_SPEED(c1CmdPtr->cmdData.winAttributes.effectSpeed),
                    DECODE_DIRECTION(c1CmdPtr->cmdData.winAttributes.effectDirection));
        if( c1CmdPtr->cmdData.winAttributes.isWordWrapped == TRUE ) writeToFile(ctxPtr->fp[fileIdx], ":WW");
        writeToFile(ctxPtr->fp[fileIdx], "}");
    // --- Define Window {DFx} ---
    } else if( (c1CmdPtr->c1CmdCode == DTVCC_C1_DF0) || (c1CmdPtr->c1CmdCode == DTVCC_C1_DF1) ||
               (c1CmdPtr->c1CmdCode == DTVCC_C1_DF2) || (c1CmdPtr->c1CmdCode == DTVCC_C1_DF3) ||
               (c1CmdPtr->c1CmdCode == DTVCC_C1_DF4) || (c1CmdPtr->c1CmdCode == DTVCC_C1_DF5) ||
               (c1CmdPtr->c1CmdCode == DTVCC_C1_DF6) || (c1CmdPtr->c1CmdCode == DTVCC_C1_DF7) ) {
        writeToFile(ctxPtr->fp[fileIdx], " {%s:%s:R%d-C%d:Anchor-%s-V%d-H%d:Pen-%s:Pr-%d",
                    DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode),
                    DECODE_PREDEF_WIN_STYLE(c1CmdPtr->cmdData.winDefinition.windowStyle),
                    c1CmdPtr->cmdData.winDefinition.rowCount,
                    c1CmdPtr->cmdData.winDefinition.columnCount,
                    DECODE_ANCOR_POSITION(c1CmdPtr->cmdData.winDefinition.anchorId),
                    c1CmdPtr->cmdData.winDefinition.verticalAnchor,
                    c1CmdPtr->cmdData.winDefinition.horizontalAnchor,
                    DECODE_PREDEF_PEN_STYLE(c1CmdPtr->cmdData.winDefinition.penStyle),
                    c1CmdPtr->cmdData.winDefinition.priority);
        if( c1CmdPtr->cmdData.winDefinition.isVisible == TRUE ) writeToFile(ctxPtr->fp[fileIdx], ":VIS");
        if( c1CmdPtr->cmdData.winDefinition.isRowLocked == TRUE ) writeToFile(ctxPtr->fp[fileIdx], ":RL");
        if( c1CmdPtr->cmdData.winDefinition.isColumnLocked == TRUE ) writeToFile(ctxPtr->fp[fileIdx], ":CL");
        if( c1CmdPtr->cmdData.winDefinition.isRelativePosition == TRUE ) writeToFile(ctxPtr->fp[fileIdx], ":RP");
        writeToFile(ctxPtr->fp[fileIdx], "}");
    } else {
        LOG( DEBUG_LEVEL_ERROR, DBG_708_OUT, "Impossible Branch: 0x%02X", c1CmdPtr->c1CmdCode );
    }
} // displayC1Cmd()

/*------------------------------------------------------------------------------
 | NAME:
 |    writeTimeStamp()
 |
 | DESCRIPTION:
 |    This function writes out the timestamp to STDOUT or a file.
 -------------------------------------------------------------------------------*/
static void writeTimeStamp( DtvccOutputCtx* ctxPtr, CaptionTime* timestampPtr, uint8 serviceNumber ) {
    ASSERT(ctxPtr);

    if( ctxPtr->printMsNotFrame == TRUE ) {
        writeToFile(ctxPtr->fp[serviceNumber-1], "\n%02d:%02d:%02d,%03d -", timestampPtr->hour,
                    timestampPtr->minute, timestampPtr->second, timestampPtr->millisecond);
    } else {
        writeToFile(ctxPtr->fp[serviceNumber-1], "\n%02d:%02d:%02d:%02d -", timestampPtr->hour,
                    timestampPtr->minute, timestampPtr->second, timestampPtr->frame);
    }
}  // writeTimeStamp()
