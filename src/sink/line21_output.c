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

#include "debug.h"

#include "line21_output.h"
#include "line21_decode.h"
#include "cc_utils.h"
#include "output_utils.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/
static char* BARF = "🤮"; // used for unknown char response

static char* normalizedBasicNaCharSet[96] = { // int not char for Special Characters
        " ",  "!",  "\"", "#",  "$",  "%", "&", "\'", "(", ")",  "á",  "+",  ",",
        "-",  ".",  "/",  "0",  "1",  "2", "3", "4",  "5", "6",  "7",  "8",  "9",
        ":",  ";",  "<",  "=",  ">",  "?", "@", "A",  "B", "C",  "D",  "E",  "F",
        "G",  "H",  "I",  "J",  "K",  "L", "M", "N",  "O", "P",  "Q",  "R",  "S",
        "T",  "U",  "V",  "W",  "X",  "Y", "Z", "[",  "é", "]",  "í",  "ó",  "ú",
        "a",  "b",  "c",  "d",  "e",  "f", "g", "h",  "i", "j",  "k",  "l",  "m",
        "n",  "o",  "p",  "q",  "r",  "s", "t", "u",  "v", "w",  "x",  "y",  "z",
        "ç",  "÷",  "Ñ",  "ñ",  "■"
};

static char* normalizedSpecialNaCharSet[16] = { // int not char for Special Characters
        "®",  "°",  "½",  "¿",  "™",  "¢",  "£",  "♪",
        "à",  "☐",  "è",  "â",  "ê",  "î",  "ô",  "û"
};

static char* normalizedExtendedWeCharSet1[32] = { // int not char for Special Characters
        "Á",  "É",  "Ó",  "Ú",  "Ü",  "ü",  "´",  "¡",  // Spanish
        "*", "\"",  "-",  "©",  "℠",  "·", "\"", "\"",  // Miscellanous
        "À",  "Â",  "Ç",  "È",  "Ê",  "Ë",  "ë",  "Î",  // French
        "Ï",  "ï",  "Ô",  "Ù",  "ù",  "Û",  "«",  "»"
};

static char* normalizedExtendedWeCharSet2[32] = { // int not char for Special Characters
        "Ã",  "ã",  "Í",  "Ì",  "ì",  "Ò",  "ò",  "Õ",  // Portuguese
        "õ",  "{",  "}", "\\",  "^",  "_",  "|",  "~",
        "Ä",  "ä",  "Ö",  "ö",  "ß",  "¥",  "¤",  "|",  // German
        "Å",  "å",  "Ø",  "ø",  "+",  "+",  "+",  "+"   // Danish
};

static char* controlCodeShortName[17] = {
        "RCL", "BS", "AOF", "AON", "DER", "RU2", "RU3", "RU4",
        "FON", "RDC", "TR", "RTD", "EDM", "CR", "ENM", "EOC", "ERR"
};

static char* tabControlCodeDescrShort[4] = {
        "ERR", "TO1", "TO2", "TO3"
};

static char* styleNameFull[9] = {
        "White", "Green", "Blue", "Cyan", "Red", "Yellow", "Magenta", "Italic-White", "Unknown"
};

static char* colorNameFull[9] = {
        "White", "Green", "Blue", "Cyan", "Red", "Yellow", "Magenta", "Black", "Unknown"
};

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

static char* Line21StyleCodeToString( uint8 );
static char* Line21ColorCodeToString( uint8 );
static void writeLine21codeToText( Line21OutputCtx*, Line21Code, CaptionTime* );

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21OutInitialize()
 |
 | INPUT PARAMETERS:
 |    ctxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    outputFileNameStr - The name of the file to write the data to.
 |
 | RETURN VALUES:
 |    LinkInfo -  Information about this element of the pipeline, such that it can
 |                be chained to elements that can consume the specific type of data
 |                that it produces.
 |
 | DESCRIPTION:
 |    This initializes this element of the pipeline.
 -------------------------------------------------------------------------------*/
LinkInfo Line21OutInitialize( Context* ctxPtr, char* outputFileNameStr ) {
    ASSERT(ctxPtr);
    ASSERT(!ctxPtr->line21OutputCtxPtr);
    char tempFilename[MAX_FILE_NAME_LEN];

    strncpy(tempFilename, outputFileNameStr, MAX_FILE_NAME_LEN-1);
    tempFilename[MAX_FILE_NAME_LEN-1] = '\0';
    strncat(tempFilename, ".608", (MAX_FILE_NAME_LEN - strlen(tempFilename) - 1));

    ctxPtr->line21OutputCtxPtr = malloc(sizeof(Line21OutputCtx));
    for( int loop = 0; loop < LINE21_MAX_NUM_CHANNELS; loop++ ) {
        ctxPtr->line21OutputCtxPtr->textStream[loop] = FALSE;
        ctxPtr->line21OutputCtxPtr->fp[loop] = NULL;
    }
    strncpy(ctxPtr->line21OutputCtxPtr->baseFileName, tempFilename, MAX_FILE_NAME_LEN-1);

    LinkInfo linkInfo;
    linkInfo.linkType = LINE21_DATA___TEXT_FILE;
    linkInfo.sourceType = DATA_TYPE_DECODED_608;
    linkInfo.sinkType = DATA_TYPE_608_TXT_FILE;
    linkInfo.NextBufferFnPtr = &Line21OutProcNextBuffer;
    linkInfo.ShutdownFnPtr = &Line21OutShutdown;
    return linkInfo;
}  // Line21OutInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21OutProcNextBuffer()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Context Pointer.
 |    inBuffer - Pointer to the buffer to process.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method processes an incoming buffer, writing the contents into an
 |    ASCII file. There are two options, writing as SCC, which has the format:
 |    <Timestamp>    <Decoded Byte Pair> <Decoded Byte Pair> ... <Decoded Byte Pair>
 |    Or writing as SEI, which has the format:
 |    <Frame Number> - <Decoded Field 1 - Line 21 - Byte Pair>
 -------------------------------------------------------------------------------*/
boolean Line21OutProcNextBuffer( void* rootCtxPtr, Buffer* buffPtr ) {
    ASSERT(buffPtr);
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->line21OutputCtxPtr);
    Line21OutputCtx* ctxPtr = ((Context*)rootCtxPtr)->line21OutputCtxPtr;
    Line21Code* line21CodePtr = (Line21Code*)buffPtr->dataPtr;

    for( int loop = 0; loop < buffPtr->numElements; loop++ ) {
        ASSERT(line21CodePtr[loop].channelNum);

        if( ctxPtr->fp[line21CodePtr[loop].channelNum-1] == NULL ) {
            char uniqueFileName[MAX_FILE_NAME_LEN];
            strncpy(uniqueFileName, ctxPtr->baseFileName, MAX_FILE_NAME_LEN);
            char* tmpCharPtr = strrchr(uniqueFileName, '.');
            ASSERT(tmpCharPtr);
            *tmpCharPtr = '\0';
            char suffix[10];
            sprintf(suffix, "-C%d.608", line21CodePtr[loop].channelNum);
            strncat(uniqueFileName, suffix, (MAX_FILE_NAME_LEN - strlen(uniqueFileName)));
            ctxPtr->fp[line21CodePtr[loop].channelNum-1] = fileOutputInit(uniqueFileName);
            LOG(DEBUG_LEVEL_INFO, DBG_608_OUT, "Creating new Line-21 File for Output: %s", uniqueFileName);
            tmpCharPtr = strrchr(uniqueFileName, '.');
            ASSERT(tmpCharPtr);
            *tmpCharPtr = '\0';
            writeToFile(ctxPtr->fp[line21CodePtr[loop].channelNum-1], "Decoded Line 21 / CEA-608 for Asset: %s - Channel: %d", uniqueFileName, line21CodePtr[loop].channelNum);
        }

        writeLine21codeToText(ctxPtr, line21CodePtr[loop], &buffPtr->captionTime);
    }
    
    FreeBuffer(buffPtr);
    return TRUE;
} // Line21OutProcNextBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21OutShutdown()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Context Pointer.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method is called when the previous element in the pipeline determines
 |    that there is no more data coming down the pipline. This element will
 |    perform any necessary actions as a result and pass this call down the
 |    pipeline.
 -------------------------------------------------------------------------------*/
boolean Line21OutShutdown( void* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->line21OutputCtxPtr);
    Line21OutputCtx* ctxPtr = ((Context*)rootCtxPtr)->line21OutputCtxPtr;
    boolean anyFiles = FALSE;

    for( int loop = 0; loop < LINE21_MAX_NUM_CHANNELS; loop++ ) {
        if( ctxPtr->fp[loop] != NULL ) {
            closeFile(ctxPtr->fp[loop]);
            anyFiles = TRUE;
        }
    }

    if( anyFiles == FALSE ) {
        LOG( DEBUG_LEVEL_WARN, DBG_608_OUT, "No 608 Data Outputted. No Decoded 608 File Written." );
    }

    free(ctxPtr);
    ((Context*)rootCtxPtr)->line21OutputCtxPtr = NULL;
    return TRUE;
} // Line21OutShutdown()

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21DecodeBasicNACharacter()
 |
 | INPUT PARAMETERS:
 |    basicCharCode - Basic North American Character Code
 |
 | RETURN VALUES:
 |    char* - Decoded UTF-8 Character
 |
 | DESCRIPTION:
 |    This method decodes a Line 21 Basic North American Character and returns it
 -------------------------------------------------------------------------------*/
char* Line21DecodeBasicNACharacter( uint8 basicCharCode ) {
    if( (basicCharCode > 0x1F) && (basicCharCode < 0x80) ) {
        return normalizedBasicNaCharSet[(basicCharCode - 0x20)];
    } else {
        LOG( DEBUG_LEVEL_ERROR, DBG_608_OUT, "Invalid Basic NA Character Code: 0x%02X", basicCharCode );
        return BARF;
    }
} // Line21DecodeBasicNACharacter()

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21DecodeSpecialCharacter()
 |
 | INPUT PARAMETERS:
 |    specialCharCode - Special Character Code to decode.
 |
 | RETURN VALUES:
 |    char* - Decoded UTF-8 Special Character
 |
 | DESCRIPTION:
 |    This method decodes a Line 21 Special Character and returns it
 -------------------------------------------------------------------------------*/
char* Line21DecodeSpecialCharacter( uint8 specialCharCode ) {
    if( (specialCharCode > 0x2F) && (specialCharCode < 0x40) ) {
        return normalizedSpecialNaCharSet[(specialCharCode - 0x30)];
    } else {
        LOG( DEBUG_LEVEL_ERROR, DBG_608_OUT, "Invalid Special Character Code: 0x%02X", specialCharCode );
        return BARF;
    }
} // Line21DecodeSpecialCharacter()

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21DecodeExtendedCharacter()
 |
 | INPUT PARAMETERS:
 |    charSet - Which Character Set
 |    extendedCharCode - Extended Character Code to decode.
 |
 | RETURN VALUES:
 |    char* - Decoded UTF-8 Extended Character
 |
 | DESCRIPTION:
 |    This method decodes a Line 21 Extended Western Europeon Character and returns it
 -------------------------------------------------------------------------------*/
char* Line21DecodeExtendedCharacter( uint8 charSet, uint8 extendedCharCode ) {
    if( ((charSet != EXT_W_EURO_CHAR_SET_SPANISH_FRENCH) && (charSet != EXT_W_EURO_CHAR_SET_DUTCH_GERMAN)) ||
        !((charSet>0x1F) && (charSet<0x40)) ) {
        LOG( DEBUG_LEVEL_ERROR, DBG_608_OUT, "Invalid Extended Character Code: 0x%02X 0x%02X", extendedCharCode );
        return BARF;
    }

    if( charSet == EXT_W_EURO_CHAR_SET_SPANISH_FRENCH ) {
        return normalizedExtendedWeCharSet1[(extendedCharCode - 0x20)];
    } else {
        return normalizedExtendedWeCharSet2[(extendedCharCode - 0x20)];
    }
} // Line21DecodeExtendedCharacter()

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21GlobalCtrlCodeToString()
 |
 | INPUT PARAMETERS:
 |    globalCtrlCode - Global Control Code to decode.
 |
 | RETURN VALUES:
 |    char* - Pointer to the decoded Global Control Code
 |
 | DESCRIPTION:
 |    This method decodes a Line 21 Global Control Code and passes back a null
 |    terminated reference to the string in memory
 -------------------------------------------------------------------------------*/
char* Line21GlobalCtrlCodeToString( uint8 globalCtrlCode ) {
    if( (globalCtrlCode > 0x1F) && (globalCtrlCode < 0x30) ) {
        return controlCodeShortName[(globalCtrlCode & 0x0F)];
    } else {
        LOG( DEBUG_LEVEL_ERROR, DBG_608_OUT, "Invalid Global Control Code: 0x%02X", globalCtrlCode );
        return controlCodeShortName[16];
    }
} // Line21GlobalCtrlCodeToString()

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21TabOffsetCodeToString()
 |
 | INPUT PARAMETERS:
 |    tabOffsetCode - Tab Offset Code to decode.
 |
 | RETURN VALUES:
 |    char* - Pointer to the decoded Tab Offset
 |
 | DESCRIPTION:
 |    This method decodes a Line 21 Tab Offset Code and passes back a null
 |    terminated reference to the string in memory
 -------------------------------------------------------------------------------*/
char* Line21TabOffsetCodeToString( uint8 tabOffsetCode ) {
    if( (tabOffsetCode > 0x1F) && (tabOffsetCode < 0x24) ) {
        return tabControlCodeDescrShort[(tabOffsetCode & 0x0F)];
    } else {
        LOG( DEBUG_LEVEL_ERROR, DBG_608_OUT, "Invalid Tab Offset Code: 0x%02X", tabOffsetCode );
        return tabControlCodeDescrShort[0];
    }
} // Line21TabOffsetCodeToString()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21StyleCodeToString()
 |
 | DESCRIPTION:
 |    This function decodes the Style Code into Text.
 -------------------------------------------------------------------------------*/
static char* Line21StyleCodeToString( uint8 styleCode ) {
    if( ((styleCode & 0x01) == 0x00) && (styleCode < 0x0F) ) {
        return styleNameFull[(styleCode >> 0x01)];
    } else {
        LOG( DEBUG_LEVEL_ERROR, DBG_608_OUT, "Invalid PAC Style Code: 0x%02X", styleCode );
        return styleNameFull[8];
    }
} // Line21StyleCodeToString()

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21ColorCodeToString()
 |
 | DESCRIPTION:
 |    This function decodes the Color Code into Text.
 -------------------------------------------------------------------------------*/
static char* Line21ColorCodeToString( uint8 colorCode ) {
    if( ((colorCode & 0x01) == 0x00) && (colorCode < 0x0F) ) {
        return colorNameFull[(colorCode >> 0x01)];
    } else {
        LOG( DEBUG_LEVEL_ERROR, DBG_608_OUT, "Invalid PAC Color Code: 0x%02X", colorCode );
        return colorNameFull[8];
    }
} // Line21ColorCodeToString()

/*------------------------------------------------------------------------------
 | NAME:
 |    line21codeToText()
 |
 | DESCRIPTION:
 |    This function formats the text for output and writes it to the file/screen.
 -------------------------------------------------------------------------------*/
static void writeLine21codeToText( Line21OutputCtx* ctxPtr, Line21Code codeToDecode, CaptionTime* captionTimePtr ) {
    ASSERT(codeToDecode.channelNum);
    uint8 fileIdx = codeToDecode.channelNum - 1;
    ASSERT(ctxPtr->fp[fileIdx]);

    if( codeToDecode.codeType == LINE21_BASIC_CHARS ) {
        if( ctxPtr->textStream[fileIdx] == FALSE ) {
            writeToFile(ctxPtr->fp[fileIdx], " \"");
            ctxPtr->textStream[fileIdx] = TRUE;
        }
    } else {
        if( ctxPtr->textStream[fileIdx] == TRUE ) {
            writeToFile(ctxPtr->fp[fileIdx], "\"");
            ctxPtr->textStream[fileIdx] = FALSE;
        }
    }
    
    switch( codeToDecode.codeType ) {
        case LINE21_BASIC_CHARS:
            if( (codeToDecode.code.basicChars.charOne != 0) && (codeToDecode.code.basicChars.charTwo != 0) ) {
                writeToFile(ctxPtr->fp[fileIdx], "%s%s", Line21DecodeBasicNACharacter(codeToDecode.code.basicChars.charOne), Line21DecodeBasicNACharacter(codeToDecode.code.basicChars.charTwo));
            } else if( (codeToDecode.code.basicChars.charOne == 0) && (codeToDecode.code.basicChars.charTwo != 0) ) {
                writeToFile(ctxPtr->fp[fileIdx], "%s", Line21DecodeBasicNACharacter(codeToDecode.code.basicChars.charTwo));
            } else if( (codeToDecode.code.basicChars.charOne != 0) && (codeToDecode.code.basicChars.charTwo == 0) ) {
                writeToFile(ctxPtr->fp[fileIdx], "%s", Line21DecodeBasicNACharacter(codeToDecode.code.basicChars.charOne));
            } else {
                ASSERT(0);
            }
            break;
        case LINE21_SPECIAL_CHAR:
            writeToFile(ctxPtr->fp[fileIdx], "%s", Line21DecodeSpecialCharacter(codeToDecode.code.specialChar.spChar));
            break;
        case LINE21_EXTENDED_CHAR:
            writeToFile(ctxPtr->fp[fileIdx], "%s", Line21DecodeExtendedCharacter(codeToDecode.code.extendedChar.charSet, codeToDecode.code.extendedChar.exChar));
            break;
        case LINE21_GLOBAL_CONTROL_CODE:
            if( (codeToDecode.code.globalControl.cmd == GLOBAL_CTRL_CODE__RU2) ||
                (codeToDecode.code.globalControl.cmd == GLOBAL_CTRL_CODE__RU3) ||
                (codeToDecode.code.globalControl.cmd == GLOBAL_CTRL_CODE__RU4) ||
                (codeToDecode.code.globalControl.cmd == GLOBAL_CTRL_CODE__RDC) ||
                (codeToDecode.code.globalControl.cmd == GLOBAL_CTRL_CODE__RCL) ||
                (codeToDecode.code.globalControl.cmd == GLOBAL_CTRL_CODE__EDM) ||
                (codeToDecode.code.globalControl.cmd == GLOBAL_CTRL_CODE__EOC) ) {
                if( captionTimePtr->source == CAPTION_TIME_PTS_NUMBERING ) {
                    writeToFile(ctxPtr->fp[fileIdx], "\n%02d:%02d:%02d,%03d -", captionTimePtr->hour, captionTimePtr->minute,
                                captionTimePtr->second, captionTimePtr->millisecond);
                } else {
                    writeToFile(ctxPtr->fp[fileIdx], "\n%02d:%02d:%02d:%02d -", captionTimePtr->hour, captionTimePtr->minute,
                                captionTimePtr->second, captionTimePtr->frame);
                }
            }
            writeToFile(ctxPtr->fp[fileIdx], " {%s}", Line21GlobalCtrlCodeToString(codeToDecode.code.globalControl.cmd));
            break;
        case LINE21_PREAMBLE_ACCESS_CODE:
            writeToFile(ctxPtr->fp[fileIdx], " {R%d:", codeToDecode.code.pac.rowNumber);
            if( codeToDecode.code.pac.styleAddr == PAC_ADDRESS_CODE ) {
                writeToFile(ctxPtr->fp[fileIdx], "C%d", codeToDecode.code.pac.styleAddrData.cursor);
            } else if( codeToDecode.code.pac.styleAddr == PAC_STYLE_CODE ) {
                writeToFile(ctxPtr->fp[fileIdx], "%s", Line21StyleCodeToString(codeToDecode.code.pac.styleAddrData.color));
            } else {
                writeToFile(ctxPtr->fp[fileIdx], "???");
                LOG(DEBUG_LEVEL_WARN, DBG_608_OUT, "Style or Address??? - %d", codeToDecode.code.pac.styleAddr);
            }
            if( codeToDecode.code.pac.isUnderlined == TRUE ) {
                writeToFile(ctxPtr->fp[fileIdx], ":UL}");
            } else {
                writeToFile(ctxPtr->fp[fileIdx], "}");
            }
            break;
        case LINE21_TAB_CONTROL_CODE:
            writeToFile(ctxPtr->fp[fileIdx], " {%s}", Line21TabOffsetCodeToString(codeToDecode.code.tabControl.tabOffset));
            break;
        case LINE21_MID_ROW_CONTROL_CODE:
            if( codeToDecode.code.midRow.backgroundOrForeground == MIDROW_FOREGROUND_STYLE ) {
                writeToFile(ctxPtr->fp[fileIdx], " {FG-%s", Line21StyleCodeToString(codeToDecode.code.midRow.backgroundForgroundData.style));
            } else {
                writeToFile(ctxPtr->fp[fileIdx], " {BG-%s", Line21ColorCodeToString(codeToDecode.code.midRow.backgroundForgroundData.color));
            }
            if( (codeToDecode.code.midRow.isPartiallyTransparent == TRUE) && (codeToDecode.code.midRow.isUnderlined == TRUE) ) {
                writeToFile(ctxPtr->fp[fileIdx], ":PT:UL}");
            } else if( (codeToDecode.code.midRow.isPartiallyTransparent == TRUE) && (codeToDecode.code.midRow.isUnderlined == FALSE) ) {
                writeToFile(ctxPtr->fp[fileIdx], "PT}");
            } else if( (codeToDecode.code.midRow.isPartiallyTransparent == FALSE) && (codeToDecode.code.midRow.isUnderlined == TRUE) ) {
                writeToFile(ctxPtr->fp[fileIdx], "UL}");
            } else {
                writeToFile(ctxPtr->fp[fileIdx], "}");
            }
            break;
        case LINE21_NULL_DATA:
        case LINE21_CODE_TYPE_UNKNOWN:
        default:
            LOG(DEBUG_LEVEL_WARN, DBG_608_OUT, "Unexpected Code Type: %d", codeToDecode.codeType);
            break;
    }
}  // line21codeToText()
