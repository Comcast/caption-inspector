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
#include <time.h>

#include "debug.h"

#include "cc_data_output.h"
#include "output_utils.h"
#include "cc_utils.h"
#include "mcc_encode.h"
#include "line21_decode.h"
#include "line21_output.h"
#include "dtvcc_decode.h"
#include "dtvcc_output.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

static const char* ccTypeStr[4] = { "1", "2", "D", "S" };

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

static uint16 add708Error( CcDataOutputCtx*, char* );
static void decode608Pair( CcDataOutputCtx*, uint8, uint8, uint8, CcdElemOut*, TextString* );
static void decodePacketStart( CcDataOutputCtx*, uint8, uint8, CcdElemOut* );
static void decodePacketData( CcDataOutputCtx*, uint8, char*, char*, TextString*, char* );
static void decodeC0CmdCode( CcDataOutputCtx*, uint8, char*, char*, char* );
static void decodeC1CmdCode( CcDataOutputCtx*, uint8, char*, char*, char* );
static void decodeExtCmdCode( CcDataOutputCtx*, uint8, char*, char*, TextString*, char* );

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    CcDataOutInitialize()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    outputFileNameStr - The name of the file to write the data to.
 |    suppressFill - Whether or not to output fill packets.
 |
 | RETURN VALUES:
 |    LinkInfo -  Information about this element of the pipeline, such that it can
 |                be chained to elements that can consume the specific type of data
 |                that it produces.
 |
 | DESCRIPTION:
 |    This initializes this element of the pipeline.
 -------------------------------------------------------------------------------*/
LinkInfo CcDataOutInitialize( Context* rootCtxPtr, char* outputFileNameStr ) {
    ASSERT(rootCtxPtr);
    ASSERT(!rootCtxPtr->ccDataOutputCtxPtr);
    char tempFilename[MAX_FILE_NAME_LEN];

    strncpy(tempFilename, outputFileNameStr, MAX_FILE_NAME_LEN-1);
    tempFilename[MAX_FILE_NAME_LEN-1] = '\0';
    strncat(tempFilename, ".ccd", (MAX_FILE_NAME_LEN - strlen(tempFilename) - 1));

    rootCtxPtr->ccDataOutputCtxPtr = malloc(sizeof(CcDataOutputCtx));
    CcDataOutputCtx* ctxPtr = rootCtxPtr->ccDataOutputCtxPtr;

    ctxPtr->fp = NULL;
    ctxPtr->wasHeaderWritten = FALSE;
    ctxPtr->currentChannel[0] = UNKOWN_CHANNEL;
    ctxPtr->currentChannel[1] = UNKOWN_CHANNEL;
    ctxPtr->currentService = UNKNOWN_SERVICE;
    ctxPtr->cea708ErrNum = 0;
    ctxPtr->cea708State = CEA708_STATE_UNKNOWN;
    ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
    ctxPtr->cea708BytesRemaining = 0;
    strncpy(ctxPtr->ccdFileName, tempFilename, MAX_FILE_NAME_LEN-1);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___TEXT_FILE;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_CC_DATA_TXT_FILE;
    linkInfo.NextBufferFnPtr = &CcDataOutProcNextBuffer;
    linkInfo.ShutdownFnPtr = &CcDataOutShutdown;
    return linkInfo;
}  // CcDataOutInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    CcDataOutProcNextBuffer()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Context Pointer.
 |    buffPtr - Pointer to the buffer to process.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method processes an incoming buffer, writing the contents into an
 |    ASCII file.
 -------------------------------------------------------------------------------*/
boolean CcDataOutProcNextBuffer( void* rootCtxPtr, Buffer* buffPtr ) {
    ASSERT(buffPtr);
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->ccDataOutputCtxPtr);
    CcDataOutputCtx* ctxPtr = ((Context*)rootCtxPtr)->ccDataOutputCtxPtr;
    int len;

    if( ctxPtr->fp == NULL ) {
        LOG(DEBUG_LEVEL_INFO, DBG_CCD_OUT, "Creating new CCD File for Output: %s", ctxPtr->ccdFileName);
        ctxPtr->fp = fileOutputInit(ctxPtr->ccdFileName);
    }

    if( ctxPtr->wasHeaderWritten == FALSE ) {
        char dateStr[50];
        char timeStr[30];
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        ASSERT(tm.tm_wday < 7);
        ASSERT(tm.tm_mon < 12);
        sprintf(dateStr, "Creation Date=%s, %s %d, %d", DayOfWeekStr[tm.tm_wday], MonthStr[tm.tm_mon], tm.tm_mday, tm.tm_year + 1900);
        sprintf(timeStr, "Creation Time=%d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
        writeToFile(ctxPtr->fp, "File Format=Comcast CC Data File\n\n");
        writeToFile(ctxPtr->fp, "/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
        writeToFile(ctxPtr->fp, " | Caption Converter\n");
        writeToFile(ctxPtr->fp, " | Closed Caption Data File\n");
        writeToFile(ctxPtr->fp, " |\n");
        writeToFile(ctxPtr->fp, " | This file contains a decoded binary representation of Closed Captioning Data. Each grouping of lines immediately following a timestamp represents the Closed | Captioning Data - cc_data() that\n");
        writeToFile(ctxPtr->fp, " | would be associated with a frame of video, or a line from an MacCaption (MCC) file. The number of Closed Caption Constructs is determined by the framerate of the asset that they are associated\n");
        writeToFile(ctxPtr->fp, " | with, per the CEA-708-D Specification. For the purposes of this documentation, each grouping of lines that immediately follow a timestamp will be referred to as a 'line'.\n");
        writeToFile(ctxPtr->fp, " |\n");
        writeToFile(ctxPtr->fp, " | Example Line:\n");
        writeToFile(ctxPtr->fp, " |    00:00:36:04   F1:9420 PD:8CFF XD:0000  ____________ ____________ ____________\n");
        writeToFile(ctxPtr->fp, " |    \\         /   \\                     /  \\                                    /\n");
        writeToFile(ctxPtr->fp, " |     Timestamp     ----- Hex Data ------    ------------ Decoded Data ----------\n");
        writeToFile(ctxPtr->fp, " |\n");
        writeToFile(ctxPtr->fp, " | Timestamp - The timestamp has three different formats. Each of the formats start with Hour:Minute:Second, but end with Millisecond or Frame, depending on the punctuation preceeding the last number.\n");
        writeToFile(ctxPtr->fp, " |\n");
        writeToFile(ctxPtr->fp, " | 00:01:22:14  - Frame Number with No Drop Frame (1 Hour, 22 Seconds, 14 Frames)\n");
        writeToFile(ctxPtr->fp, " | 00:01:22;05  - Frame Number with Drop Frame (1 Hour, 22 Seconds, 5 Frames)\n");
        writeToFile(ctxPtr->fp, " | 00:01:22,197 - Time in Milliseconds (1 Hour, 22 Seconds, 197 Milliseconds)\n");
        writeToFile(ctxPtr->fp, " |\n");
        writeToFile(ctxPtr->fp, " | Hex Data - The Hex Data representation of the Closed Captioning Construct is intended to be the most direct representation of the cc_data() structure, which is comprised of the following fields:\n");
        writeToFile(ctxPtr->fp, " |\n");
        writeToFile(ctxPtr->fp, " |            {cc_valid}{cc_type}:{cc_data_1}{cc_data_2}\n");
        writeToFile(ctxPtr->fp, " |\n");
        writeToFile(ctxPtr->fp, " |            cc_valid - This field determines whether the subsequent fields should considered valid and be interperated. This field will either be 'F' for Valid Line 21, 'P' for Valid DTVCC, or 'X' for Invalid.\n");
        writeToFile(ctxPtr->fp, " |            cc_type - This field determines the type of data that will be in cc_data_1 and cc_data_2. This field can have the following values: '1' Line 21 Field 1 Data, '2' Line 21 Field 2 Data,\n");
        writeToFile(ctxPtr->fp, " |                      'S' DTVCC Channel Packet Data Start, 'D' DTVCC Channel Packet Data. \n");
        writeToFile(ctxPtr->fp, " |            cc_data_1 / cc_data_2 - These fields are the actual two bytes of payload data, which are represented in hexidecimal.\n");
        writeToFile(ctxPtr->fp, " |\n");
        writeToFile(ctxPtr->fp, " | Decoded Data - The Decoded Data is an inline decode of Hex Data in the corresponding Closed Captioning Construct. This is a partial decode of the DTVCC / Line 21 repesentations. The following\n");
        writeToFile(ctxPtr->fp, " |                tables can be used to understand the decoding.\n");
        writeToFile(ctxPtr->fp, " |\n");
        writeToFile(ctxPtr->fp, " ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/\n");
        writeToFile(ctxPtr->fp, "%s\n%s\n", dateStr, timeStr);
        if( (buffPtr->captionTime.frameRatePerSecTimesOneHundred % 100) == 0 ) {
            writeToFile(ctxPtr->fp, "Frame Rate=%d\n", (buffPtr->captionTime.frameRatePerSecTimesOneHundred / 100));
        } else {
            writeToFile(ctxPtr->fp, "Frame Rate=%d.%02d\n", (buffPtr->captionTime.frameRatePerSecTimesOneHundred / 100),
                        (buffPtr->captionTime.frameRatePerSecTimesOneHundred % 100));
        }
        if( buffPtr->captionTime.dropframe == TRUE ) {
            writeToFile(ctxPtr->fp, "Drop Frame=True\n\n");
        } else {
            writeToFile(ctxPtr->fp, "Drop Frame=False\n\n");
        }
        ctxPtr->wasHeaderWritten = TRUE;
    }

    if( buffPtr->captionTime.source == CAPTION_TIME_PTS_NUMBERING ) {
        writeToFile(ctxPtr->fp, "%02d:%02d:%02d,%03d  ", buffPtr->captionTime.hour, buffPtr->captionTime.minute,
                    buffPtr->captionTime.second, buffPtr->captionTime.millisecond);
    } else if( buffPtr->captionTime.source == CAPTION_TIME_FRAME_NUMBERING ) {
        if( buffPtr->captionTime.dropframe == FALSE ) {
            writeToFile(ctxPtr->fp, "%02d:%02d:%02d:%02d   ", buffPtr->captionTime.hour, buffPtr->captionTime.minute,
                        buffPtr->captionTime.second, buffPtr->captionTime.frame);
        } else {
            writeToFile(ctxPtr->fp, "%02d:%02d:%02d;%02d   ", buffPtr->captionTime.hour, buffPtr->captionTime.minute,
                        buffPtr->captionTime.second, buffPtr->captionTime.frame);
        }
    }

    ASSERT((buffPtr->numElements % 3) == 0);
    CcdLineOut lineOut;
    lineOut.numElements = 0;
    boolean printNewline = FALSE;
    for( int loop = 0; loop < NUM_608_CHANNELS; loop++ ) {
        lineOut.txtStr.txtStr608[loop][0] = '\0';
    }
    for( int loop = 0; loop < NUM_708_SERVICES; loop++ ) {
        lineOut.txtStr.txtStr708[loop][0] = '\0';
    }
    char errorStr[CEA708_ERROR_STR_SIZE];
    errorStr[0] = '\0';

    for( int loop = 0; loop < buffPtr->numElements; loop = loop + 3 ) {
        boolean ccValid = ((buffPtr->dataPtr[loop] & 0x04) >> 2);
        uint8 ccType = (buffPtr->dataPtr[loop] & 0x03);
        uint8 ccData1 = buffPtr->dataPtr[loop+1];
        uint8 ccData2 = buffPtr->dataPtr[loop+2];
        len = snprintf(lineOut.element[lineOut.numElements].tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "_________" );
        ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE-1));
        len = snprintf(lineOut.element[lineOut.numElements].decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_________________" );
        ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE-1));
        if( ccValid == TRUE ) {
            ASSERT(ccType < 4 );
            if( (ccType == CC_DATA_TYPE__FIELD_1) || (ccType == CC_DATA_TYPE__FIELD_2) ) {
                sprintf(lineOut.element[lineOut.numElements].hexStr, "F%s:%02X%02X", ccTypeStr[ccType], ccData1, ccData2);
            } else {
                sprintf(lineOut.element[lineOut.numElements].hexStr, "P%s:%02X%02X", ccTypeStr[ccType], ccData1, ccData2);
            }
            switch( ccType ) {
                case CC_DATA_TYPE__FIELD_1:
                case CC_DATA_TYPE__FIELD_2:
                    decode608Pair( ctxPtr, ccType, ccData1, ccData2, &lineOut.element[lineOut.numElements], &lineOut.txtStr );
                    lineOut.numElements = lineOut.numElements + 1;
                    break;
                case CC_DATA_TYPE__PACKET_START:
                    decodePacketStart( ctxPtr, ccData1, ccData2, &lineOut.element[lineOut.numElements] );
                    lineOut.numElements = lineOut.numElements + 1;
                    break;
                case CC_DATA_TYPE__PACKET_DATA:
                    decodePacketData( ctxPtr, ccData1, lineOut.element[lineOut.numElements].tagStr, lineOut.element[lineOut.numElements].decStr, &lineOut.txtStr, errorStr );
                    lineOut.element[lineOut.numElements].tagStr[(CC_DATA_ELEMENT_TAG_STR_SIZE/2)-1] = '|';
                    lineOut.element[lineOut.numElements].decStr[(CC_DATA_ELEMENT_DEC_STR_SIZE/2)-1] = '|';
                    decodePacketData( ctxPtr, ccData2, &lineOut.element[lineOut.numElements].tagStr[CC_DATA_ELEMENT_TAG_STR_SIZE/2],
                                      &lineOut.element[lineOut.numElements].decStr[CC_DATA_ELEMENT_DEC_STR_SIZE/2], &lineOut.txtStr, errorStr );
                    lineOut.numElements = lineOut.numElements + 1;
                    break;
                default:
                    ASSERT(0);
            }
        } else {
            ASSERT(ccType < 4 );
            sprintf(lineOut.element[lineOut.numElements].hexStr, "X%s:%02X%02X", ccTypeStr[ccType], ccData1, ccData2);
            lineOut.numElements = lineOut.numElements + 1;
        }

        if( lineOut.numElements >= NUM_CC_DATA_ELEMENTS_PER_LINE ) {
            if( printNewline == TRUE ) {
                writeToFile(ctxPtr->fp, "\n              ");
            }
            for( int iloop = 0; iloop < lineOut.numElements; iloop++ ) {
                writeToFile(ctxPtr->fp, "%s  ", lineOut.element[iloop].hexStr);
            }
            writeToFile(ctxPtr->fp, "  ");
            for( int iloop = 0; iloop < lineOut.numElements; iloop++ ) {
                writeToFile(ctxPtr->fp, "%s  ", lineOut.element[iloop].tagStr);
            }
            writeToFile(ctxPtr->fp, "  ");
            for( int iloop = 0; iloop < lineOut.numElements; iloop++ ) {
                writeToFile(ctxPtr->fp, "%s  ", lineOut.element[iloop].decStr);
            }
            lineOut.numElements = 0;
            printNewline = TRUE;
        }
    }

    boolean anyTextWritten = FALSE;
    for( int loop = 0; loop < NUM_608_CHANNELS; loop++ ) {
        if( strlen(lineOut.txtStr.txtStr608[loop]) > 0 ) {
            if( anyTextWritten == FALSE ) {
                writeToFile(ctxPtr->fp, "\nTEXT: ");
                anyTextWritten = TRUE;
            }
            writeToFile(ctxPtr->fp, "Ch%d - \"%s\" ", loop+1, lineOut.txtStr.txtStr608[loop]);
            lineOut.txtStr.txtStr608[loop][0] = '\0';
        }
    }

    for( int loop = 0; loop < NUM_708_SERVICES; loop++ ) {
        if( strlen(lineOut.txtStr.txtStr708[loop]) > 0 ) {
            if (anyTextWritten == FALSE) {
                writeToFile(ctxPtr->fp, "\nTEXT: ");
                anyTextWritten = TRUE;
            }
            writeToFile(ctxPtr->fp, "Svc%d - \"%s\" ", loop + 1, lineOut.txtStr.txtStr708[loop]);
            lineOut.txtStr.txtStr708[loop][0] = '\0';
        }
    }

    if( strlen(errorStr) > 0 ) {
        writeToFile(ctxPtr->fp, "\nERRORS: %s", errorStr);
        errorStr[0] = '\0';
    }

    ASSERT(lineOut.numElements == 0 );
    writeToFile(ctxPtr->fp, "\n\n");

    FreeBuffer(buffPtr);
    return TRUE;
} // CcDataOutProcNextBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    CcDataOutShutdown()
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
boolean CcDataOutShutdown( void* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->ccDataOutputCtxPtr);

    closeFile(((Context*)rootCtxPtr)->ccDataOutputCtxPtr->fp);

    free(((Context*)rootCtxPtr)->ccDataOutputCtxPtr);
    ((Context*)rootCtxPtr)->ccDataOutputCtxPtr = NULL;
    return TRUE;
} // CcDataOutShutdown()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    decode608Pair()
 |
 | DESCRIPTION:
 |    This function decodes a CEA-608 Closed Captioning Construct.
 -------------------------------------------------------------------------------*/
static void decode608Pair( CcDataOutputCtx* ctxPtr, uint8 ccType, uint8 ccData1, uint8 ccData2, CcdElemOut* outputPtr, TextString* txtStr ){
    ccData1 = ccData1 & LINE_21_PARITY_MASK;
    ccData2 = ccData2 & LINE_21_PARITY_MASK;
    uint8 channelNum = 0;
    int len;
    ASSERT(txtStr);
    ASSERT(outputPtr);

    if( (ccData1 == 0) && (ccData2 == 0) ) {
        if( ccType == CC_DATA_TYPE__FIELD_1 ) {
            len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "F1 - NULL" );
            ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE-1));
            len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "608: Field 1 NULL" );
            ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
        } else {
            len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "F2 - NULL" );
            ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE-1));
            len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "608: Field 2 NULL" );
            ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
        }
    } else if( ((ccData1 == GLOBAL_CTRL_CODE_CC1) || (ccData1 == GLOBAL_CTRL_CODE_CC2) ||
                (ccData1 == GLOBAL_CTRL_CODE_CC3) || (ccData1 == GLOBAL_CTRL_CODE_CC4)) &&
               ((ccData2 & GLOBAL_CTRL_CMD_MASK) == ccData2) ) {
        switch( ccData1 ) {
            case GLOBAL_CTRL_CODE_CC1:
                channelNum = 1;
                break;
            case GLOBAL_CTRL_CODE_CC2:
                channelNum = 2;
                break;
            case GLOBAL_CTRL_CODE_CC3:
                channelNum = 3;
                break;
            case GLOBAL_CTRL_CODE_CC4:
                channelNum = 4;
                break;
            default:
                ASSERT(0);
                break;
        }

        if( ((channelNum == 1) && (ccType == CC_DATA_TYPE__FIELD_2)) ||
            ((channelNum == 2) && (ccType == CC_DATA_TYPE__FIELD_2)) ||
            ((channelNum == 3) && (ccType == CC_DATA_TYPE__FIELD_1)) ||
            ((channelNum == 4) && (ccType == CC_DATA_TYPE__FIELD_1)) ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_CCD_OUT, "Channel/Field Mismatch: Chan - %d Field - %s Code - %s", channelNum, ccTypeStr[ccType], Line21GlobalCtrlCodeToString(ccData2));
        }

        ctxPtr->currentChannel[ccType] = channelNum;
        if( strlen(Line21GlobalCtrlCodeToString(ccData2)) == 3 ) {
            len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "Ch%d {%s}", channelNum, Line21GlobalCtrlCodeToString(ccData2));
            ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE - 1));
        } else {
            len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "Ch%d {%s} ", channelNum, Line21GlobalCtrlCodeToString(ccData2));
            ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE - 1));
        }

        switch( ccData2 ) {
            case GLOBAL_CTRL_CODE__RCL:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "ResumeCaptLoading" );
                break;
            case GLOBAL_CTRL_CODE__BS:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "____Backspace____" );
                break;
            case GLOBAL_CTRL_CODE__AOF:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "____Alarm Off____" );
                break;
            case GLOBAL_CTRL_CODE__AON:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_____Alarm On____" );
                break;
            case GLOBAL_CTRL_CODE__DER:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Delete Entire Row" );
                break;
            case GLOBAL_CTRL_CODE__RU2:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Roll Up -  2 Rows" );
                break;
            case GLOBAL_CTRL_CODE__RU3:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Roll Up -  3 Rows" );
                break;
            case GLOBAL_CTRL_CODE__RU4:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Roll Up -  4 Rows" );
                break;
            case GLOBAL_CTRL_CODE__FON:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_____Flash On____" );
                break;
            case GLOBAL_CTRL_CODE__RDC:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "ResumeDirectCaptn" );
                break;
            case GLOBAL_CTRL_CODE__TR:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "___Text Restart__" );
                break;
            case GLOBAL_CTRL_CODE__RTD:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "ResumeTextDisplay" );
                break;
            case GLOBAL_CTRL_CODE__EDM:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "EraseDisplayedMem" );
                break;
            case GLOBAL_CTRL_CODE__CR:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_Carriage Return_" );
                break;
            case GLOBAL_CTRL_CODE__ENM:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Erase NonDisp Mem" );
                break;
            case GLOBAL_CTRL_CODE__EOC:
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "__End of Caption_" );
                break;
            default:
                ASSERT(0);
                break;
        }
        ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
    } else if( ((ccData1 == CMD_MIDROW_BG_CHAN_1_3) || (ccData1 == CMD_MIDROW_FG_CHAN_1_3) ||
                (ccData1 == CMD_MIDROW_BG_CHAN_2_4) || (ccData1 == CMD_MIDROW_FG_CHAN_2_4)) &&
               ((ccData2 & MIDROW_CODE_MASK) == ccData2) ) {
        if( (ccData1 == CMD_MIDROW_BG_CHAN_1_3) || (ccData1 == CMD_MIDROW_FG_CHAN_1_3) ) {
            if( ccType == CC_DATA_TYPE__FIELD_1 ) {
                channelNum = 1;
            } else {
                channelNum = 3;
            }
        } else {
            if( ccType == CC_DATA_TYPE__FIELD_1 ) {
                channelNum = 2;
            } else {
                channelNum = 4;
            }
        }
        ASSERT(channelNum == ctxPtr->currentChannel[ccType]);

        len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "Ch%d - MDR", channelNum );
        ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE-1));

        if( (ccData1 & MIDROW_BG_FG_MASK) == MIDROW_FOREGROUND_STYLE ) {
            switch( ccData2 & MIDROW_FG_STYLE_MASK ) {
                case MIDROW_FG_STYLE_WHITE:
                    if ((ccData2 & MIDROW_TRANSPARENCY_MASK) == MIDROW_PARTIALLY_TRANSPARENT) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "FG:White - Transp");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_____FG:White____");
                    }
                    break;
                case MIDROW_FG_STYLE_GREEN:
                    if ((ccData2 & MIDROW_TRANSPARENCY_MASK) == MIDROW_PARTIALLY_TRANSPARENT) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "FG:Green - Transp");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_____FG:Green____");
                    }
                    break;
                case MIDROW_FG_STYLE_BLUE:
                    if ((ccData2 & MIDROW_TRANSPARENCY_MASK) == MIDROW_PARTIALLY_TRANSPARENT) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "FG:Blue -  Transp");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_____FG:Blue_____");
                    }
                    break;
                case MIDROW_FG_STYLE_CYAN:
                    if ((ccData2 & MIDROW_TRANSPARENCY_MASK) == MIDROW_PARTIALLY_TRANSPARENT) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "FG:Cyan -  Transp");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_____FG:Cyan_____");
                    }
                    break;
                case MIDROW_FG_STYLE_RED:
                    if ((ccData2 & MIDROW_TRANSPARENCY_MASK) == MIDROW_PARTIALLY_TRANSPARENT) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "FG:Red  -  Transp");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "______FG:Red_____");
                    }
                    break;
                case MIDROW_FG_STYLE_YELLOW:
                    if ((ccData2 & MIDROW_TRANSPARENCY_MASK) == MIDROW_PARTIALLY_TRANSPARENT) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "FG:Yellw - Transp");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "____FG:Yellow____");
                    }
                    break;
                case MIDROW_FG_STYLE_MAGENTA:
                    if ((ccData2 & MIDROW_TRANSPARENCY_MASK) == MIDROW_PARTIALLY_TRANSPARENT) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "FG:Mgnta - Transp");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "____FG:Magenta___");
                    }
                    break;
                case MIDROW_FG_STYLE_ITALIC_WHITE:
                    if ((ccData2 & MIDROW_TRANSPARENCY_MASK) == MIDROW_PARTIALLY_TRANSPARENT) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "FG:ItWht - Transp");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "FG:Italics  White");
                    }
                    break;
                default:
                    ASSERT(0);
                    break;
            }
            ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
        } else {
            switch(ccData2 & MIDROW_BG_COLOR_MASK) {
                case MIDROW_BG_COLOR_WHITE:
                    if( (ccData2 & MIDROW_UNDERLINE_MASK) == MIDROW_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "BG:White - Undrln");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_____BG:White____");
                    }
                    break;
                case MIDROW_BG_COLOR_GREEN:
                    if( (ccData2 & MIDROW_UNDERLINE_MASK) == MIDROW_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "BG:Green - Undrln");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_____BG:Green____");
                    }
                    break;
                case MIDROW_BG_COLOR_BLUE:
                    if( (ccData2 & MIDROW_UNDERLINE_MASK) == MIDROW_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "BG:Blue -  Undrln");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_____BG:Blue_____");
                    }
                    break;
                case MIDROW_BG_COLOR_CYAN:
                    if( (ccData2 & MIDROW_UNDERLINE_MASK) == MIDROW_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "BG:Cyan -  Undrln");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_____BG:Cyan_____");
                    }
                    break;
                case MIDROW_BG_COLOR_RED:
                    if( (ccData2 & MIDROW_UNDERLINE_MASK) == MIDROW_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "BG:Red  -  Undrln");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "______BG:Red_____");
                    }
                    break;
                case MIDROW_BG_COLOR_YELLOW:
                    if( (ccData2 & MIDROW_UNDERLINE_MASK) == MIDROW_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "BG:Yellw - Undrln");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "____BG:Yellow____");
                    }
                    break;
                case MIDROW_BG_COLOR_MAGENTA:
                    if( (ccData2 & MIDROW_UNDERLINE_MASK) == MIDROW_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "BG:Mgnta - Undrln");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "____BG:Magenta___");
                    }
                    break;
                case MIDROW_BG_COLOR_BLACK:
                    if( (ccData2 & MIDROW_UNDERLINE_MASK) == MIDROW_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "BG:Black - Undrln");
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_____BG:Black____");
                    }
                    break;
                default:
                    ASSERT(0);
                    break;
            }
            ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
        }
    } else if( ((ccData1 == CMD_TAB_HI_CC_1_3) || (ccData1 == CMD_TAB_HI_CC_2_4)) && ((ccData2 & TAB_CMD_MASK) == ccData2) ) {
        if( ccData1 == CMD_TAB_HI_CC_1_3 ) {
            if( ccType == CC_DATA_TYPE__FIELD_1 ) {
                channelNum = 1;
            } else {
                channelNum = 3;
            }
        } else {
            if( ccType == CC_DATA_TYPE__FIELD_1 ) {
                channelNum = 2;
            } else {
                channelNum = 4;
            }
        }
        ASSERT(channelNum == ctxPtr->currentChannel[ccType]);

        len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "Ch%d - TAB", channelNum );
        ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE-1));
        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "__Tab-Offset: %d__", (ccData2 & 0x0F) );
        ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
    } else if( (ccData1 & PAC_CC1_MASK) == ccData1 ) {
        if( (ccData1 & PAC_CHANNEL_MASK) == PAC_CHANNEL_1_3 ) {
            if( ccType == CC_DATA_TYPE__FIELD_1 ) {
                channelNum = 1;
            } else {
                channelNum = 3;
            }
        } else {
            if( ccType == CC_DATA_TYPE__FIELD_1 ) {
                channelNum = 2;
            } else {
                channelNum = 4;
            }
        }
// TODO - HACK - This needs to be throttled on crappy captions.
//        ASSERT(channelNum == ctxPtr->currentChannel[ccType]);

        len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "Ch%d - PAC", channelNum );
        ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE-1));
        uint8 rowNumber = Line21PacDecodeRow(ccData1,ccData2);
        if( (ccData2 & PAC_STYLE_ADDR_MASK) == PAC_ADDRESS_SET ) {
            switch(ccData2 & PAC_CURSOR_ADDRESS_MASK) {
                case PAC_CURSOR_COLUMN_0:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Col:00-UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Column:00", rowNumber);
                    }
                    break;
                case PAC_CURSOR_COLUMN_4:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Col:04-UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Column:04", rowNumber);
                    }
                    break;
                case PAC_CURSOR_COLUMN_8:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Col:08-UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Column:08", rowNumber);
                    }
                    break;
                case PAC_CURSOR_COLUMN_12:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Col:12-UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Column:12", rowNumber);
                    }
                    break;
                case PAC_CURSOR_COLUMN_16:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Col:60-UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Column:16", rowNumber);
                    }
                    break;
                case PAC_CURSOR_COLUMN_20:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Col:20-UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Column:20", rowNumber);
                    }
                    break;
                case PAC_CURSOR_COLUMN_24:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Col:24-UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Column:24", rowNumber);
                    }
                    break;
                case PAC_CURSOR_COLUMN_28:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Col:28-UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Column:28", rowNumber);
                    }
                    break;
                default:
                    ASSERT(0);
                    break;
            }
            ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
        } else {
            switch(ccData2 & PAC_COLOR_STYLE_MASK) {
                case FG_COLOR_WHITE:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d White - UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_Row:%02d -  White_", rowNumber);
                    }
                    break;
                case FG_COLOR_GREEN:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d Green - UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_Row:%02d -  Green_", rowNumber);
                    }
                    break;
                case FG_COLOR_BLUE:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Blue - UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "__Row:%02d - Blue__", rowNumber);
                    }
                    break;
                case FG_COLOR_CYAN:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d  Cyan - UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "__Row:%02d - Cyan__", rowNumber);
                    }
                    break;
                case FG_COLOR_RED:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_Row:%02d Red - UL_", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "__Row:%02d - Red___", rowNumber);
                    }
                    break;
                case FG_COLOR_YELLOW:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Rw:%02d Yellow - UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_Row:%02d - Yellow_", rowNumber);
                    }
                    break;
                case FG_COLOR_MAGENTA:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Rw:%02d Magnta - UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d -  Magenta", rowNumber);
                    }
                    break;
                case FG_COLOR_ITALIC_WHITE:
                    if( (ccData2 & PAC_UNDERLINE_MASK) == PAC_UNDERLINE ) {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Rw:%02d ItlWht - UL", rowNumber);
                    } else {
                        len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Row:%02d - ItlacWht", rowNumber);
                    }
                    break;
                default:
                    ASSERT(0);
                    break;
            }
            ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
        }
    } else if( ((ccData1 == SPCL_NA_CHAR_SET_CH_1_3) || (ccData1 == SPCL_NA_CHAR_SET_CH_2_4)) &&
               ((ccData2 & SPCL_NA_CHAR_SET_MASK) == ccData2) ) {
        if( ccData1 == SPCL_NA_CHAR_SET_CH_1_3 ) {
            if( ccType == CC_DATA_TYPE__FIELD_1 ) {
                channelNum = 1;
            } else {
                channelNum = 3;
            }
        } else {
            if( ccType == CC_DATA_TYPE__FIELD_1 ) {
                channelNum = 2;
            } else {
                channelNum = 4;
            }
        }
        ASSERT(channelNum == ctxPtr->currentChannel[ccType]);

        snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "Ch%d  \"%s\"", channelNum, Line21DecodeSpecialCharacter(ccData2 & SPCL_NA_CHAR_MASK) );
        if( strlen(outputPtr->tagStr) == (CC_DATA_ELEMENT_TAG_STR_SIZE-2) ) {
            strcat(outputPtr->tagStr, " ");
        }
        ASSERT(strlen(outputPtr->tagStr) == (CC_DATA_ELEMENT_TAG_STR_SIZE-1));
        strcat(txtStr->txtStr608[ctxPtr->currentChannel[ccType]-1], Line21DecodeSpecialCharacter(ccData2 & SPCL_NA_CHAR_MASK));
    } else if( ((ccData1 == EXT_W_EURO_CHAR_SET_CH_1_3_SF) || (ccData1 == EXT_W_EURO_CHAR_SET_CH_1_3_FG) ||
                (ccData1 == EXT_W_EURO_CHAR_SET_CH_2_4_SF) || (ccData1 == EXT_W_EURO_CHAR_SET_CH_2_4_FG)) &&
               ((ccData2 & EXT_W_EURO_CHAR_SET_MASK) == ccData2) ) {

        if( (ccData1 == EXT_W_EURO_CHAR_SET_CH_1_3_SF) || (ccData1 == EXT_W_EURO_CHAR_SET_CH_2_4_SF) ) {
            if( (ccData2 & EXT_W_EURO_CHAR_SET_TOGGLE_MASK) == EXT_W_EURO_CHAR_SET_TOGGLE_MASK ) {
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_French Language_");
            } else {
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_Spanish or Misc_");
            }
            ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
        } else {
            if( (ccData2 & EXT_W_EURO_CHAR_SET_TOGGLE_MASK) == EXT_W_EURO_CHAR_SET_TOGGLE_MASK ) {
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_German / Danish_");
            } else {
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "_Portuguese Lang_");
            }
            ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
        }

        if( (ccData1 == EXT_W_EURO_CHAR_SET_CH_1_3_SF) || (ccData1 == EXT_W_EURO_CHAR_SET_CH_1_3_FG) ) {
            if( ccType == CC_DATA_TYPE__FIELD_1 ) {
                channelNum = 1;
            } else {
                channelNum = 3;
            }
            ASSERT(channelNum == ctxPtr->currentChannel[ccType]);
            snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "Ch%d  \"%s\"", channelNum, Line21DecodeExtendedCharacter(EXT_W_EURO_CHAR_SET_SPANISH_FRENCH, ccData2));
            if( strlen(outputPtr->tagStr) == (CC_DATA_ELEMENT_TAG_STR_SIZE-2) ) {
                strcat(outputPtr->tagStr, " ");
            }
            ASSERT(strlen(outputPtr->tagStr) == (CC_DATA_ELEMENT_TAG_STR_SIZE-1));
            strcat(txtStr->txtStr608[ctxPtr->currentChannel[ccType]-1], Line21DecodeExtendedCharacter(EXT_W_EURO_CHAR_SET_SPANISH_FRENCH, ccData2));
        } else {
            if( ccType == CC_DATA_TYPE__FIELD_1 ) {
                channelNum = 2;
            } else {
                channelNum = 4;
            }
            ASSERT(channelNum == ctxPtr->currentChannel[ccType]);
            snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "Ch%d  \"%s\"", channelNum, Line21DecodeExtendedCharacter(EXT_W_EURO_CHAR_SET_DUTCH_GERMAN, ccData2));
            if( strlen(outputPtr->tagStr) == (CC_DATA_ELEMENT_TAG_STR_SIZE-2) ) {
                strcat(outputPtr->tagStr, " ");
            }
            ASSERT(strlen(outputPtr->tagStr) == (CC_DATA_ELEMENT_TAG_STR_SIZE-1));
            strcat(txtStr->txtStr608[ctxPtr->currentChannel[ccType]-1], Line21DecodeExtendedCharacter(EXT_W_EURO_CHAR_SET_DUTCH_GERMAN, ccData2));
        }
    } else if( (((ccData1 >= FIRST_BASIC_CHAR) && (ccData1 <= LAST_BASIC_CHAR)) || (ccData1 == NULL_BASIC_CHAR)) &&
               (((ccData2 >= FIRST_BASIC_CHAR) && (ccData2 <= LAST_BASIC_CHAR)) || (ccData2 == NULL_BASIC_CHAR)) ) {
// TODO - HACK - This needs to be throttled. It is to verbose on cruddy captions...
//        ASSERT(ctxPtr->currentChannel[ccType]);
        if( (ccData1 != 0) && (ccData2 != 0) ) {
            if( (strlen(Line21DecodeBasicNACharacter(ccData1)) == 1) && (strlen(Line21DecodeBasicNACharacter(ccData2)) == 1) ) {
                len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "Ch%d: \"%s%s\"", ctxPtr->currentChannel[ccType],
                               Line21DecodeBasicNACharacter(ccData1), Line21DecodeBasicNACharacter(ccData2));
                ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE - 1));
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Chan-%d:  \"%s\"  \"%s\"", ctxPtr->currentChannel[ccType],
                               Line21DecodeBasicNACharacter(ccData1), Line21DecodeBasicNACharacter(ccData2));
                ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
            } else if( (strlen(Line21DecodeBasicNACharacter(ccData1)) == 2) && (strlen(Line21DecodeBasicNACharacter(ccData2)) == 2) ) {
                len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "Ch%d\"%s%s\"", ctxPtr->currentChannel[ccType],
                               Line21DecodeBasicNACharacter(ccData1), Line21DecodeBasicNACharacter(ccData2));
                ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE - 1));
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Chan-%d: \"%s\" \"%s\"", ctxPtr->currentChannel[ccType],
                               Line21DecodeBasicNACharacter(ccData1), Line21DecodeBasicNACharacter(ccData2));
                ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
            } else {
                len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "Ch%d \"%s%s\"", ctxPtr->currentChannel[ccType],
                               Line21DecodeBasicNACharacter(ccData1), Line21DecodeBasicNACharacter(ccData2));
                ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE - 1));
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Chan-%d:  \"%s\" \"%s\"", ctxPtr->currentChannel[ccType],
                               Line21DecodeBasicNACharacter(ccData1), Line21DecodeBasicNACharacter(ccData2));
                ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
            }
            strcat(txtStr->txtStr608[ctxPtr->currentChannel[ccType]-1], Line21DecodeBasicNACharacter(ccData1));
            strcat(txtStr->txtStr608[ctxPtr->currentChannel[ccType]-1], Line21DecodeBasicNACharacter(ccData2));
        } else if( (ccData1 != 0) && (ccData2 == 0) ) {
            if( strlen(Line21DecodeBasicNACharacter(ccData1)) == 1 ) {
                len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "Ch%d - \"%s\"", ctxPtr->currentChannel[ccType], Line21DecodeBasicNACharacter(ccData1));
                ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE - 1));
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Channel - %d:  \"%s\"", ctxPtr->currentChannel[ccType],
                               Line21DecodeBasicNACharacter(ccData1));
                ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
            } else {
                len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "Ch%d: \"%s\"", ctxPtr->currentChannel[ccType], Line21DecodeBasicNACharacter(ccData1));
                ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE - 1));
                len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "Channel - %d: \"%s\"", ctxPtr->currentChannel[ccType],
                               Line21DecodeBasicNACharacter(ccData1));
                ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));
            }
            strcat(txtStr->txtStr608[ctxPtr->currentChannel[ccType]-1], Line21DecodeBasicNACharacter(ccData1));
        } else {
            ASSERT(0);
        }
    } else {
        LOG(DEBUG_LEVEL_ERROR, DBG_CCD_OUT, "Unable to decode CC Construct: 0x%02X 0x%02X", ccData1, ccData2);
    }
} // decode608Pair()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodePacketStart()
 |
 | DESCRIPTION:
 |    This function decodes a CEA-708 Packet Start
 -------------------------------------------------------------------------------*/
static void decodePacketStart( CcDataOutputCtx* ctxPtr, uint8 ccData1, uint8 ccData2, CcdElemOut* outputPtr ) {
    ASSERT(outputPtr);
    int len;

/*+-------------------------------------------------------------------------+----+
  |                   DTVCC Caption Channel Packet Syntax                   |bits|
  +-------------------------------------------------------------------------+----+
  | caption_channel_packet() {                                              |    |
  |   sequence_number                                                       |  2 |
  |   packet_size_code                                                      |  6 |
  |   for( i = 0; i < packet_data_size; i++ ) {                             |    |
  |     packet_data[i]                                                      |  8 |
  |   }                                                                     |    |
  | }                                                                       |    |
  +-------------------------------------------------------------------------+----+*/

    uint8 seqNum = (ccData1 & PACKET_SEQ_NUM_MASK) >> PACKET_SEQ_SHIFT;
    uint8 pktSizeCode = ccData1 & PACKET_LENGTH_MASK;

    uint8 pktSize;
    if( pktSizeCode == 0 ) {
        pktSize = 128;
    } else {
        pktSize = 2 * pktSizeCode;
    }

/*+-------------------------------------------------------------------------+----+-------+
  |                        Service Block Syntax                             |bits| value |
  +-------------------------------------------------------------------------+----+-------+
  | service_block() {                                                       |    |       |
  |   service_number                                                        |  3 |       |
  |   block_size                                                            |  5 |       |
  |   if( service_number == b'111' && block_size != 0 ) {                   |    |       |
  |     null_fill                                                           |  2 |  '00' |
  |     extended_service_number                                             |  6 |       |
  |   }                                                                     |    |       |
  |   if( service_number != 0 ) {                                           |    |       |
  |     for( i = 0; i < block_size; i++ ) {                                 |    |       |
  |       block_data                                                        |  8 |       |
  |     }                                                                   |    |       |
  |   }                                                                     |    |       |
  | }                                                                       |    |       |
  +-------------------------------------------------------------------------+----+-------+*/

    uint8 srvcNum = (ccData2 & SERVICE_NUMBER_MASK) >> SERVICE_NUMBER_SHIFT;     // 3 more significant bits
    uint8 blkSize = (ccData2 & SERVICE_BLOCK_SIZE_MASK);                         // 5 less significant bits

    if( srvcNum == EXTENDED_SRV_NUM_PATTERN ) {
        ctxPtr->cea708State = CEA708_STATE_EXTENDED_SEQ_NUM;
        len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "<-Srvc:Ex");
        ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE - 1));
    } else {
        ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
        ctxPtr->currentService = srvcNum;
        len = snprintf(outputPtr->tagStr, CC_DATA_ELEMENT_TAG_STR_SIZE, "<-Srvc:%02d", srvcNum);
        ASSERT(len == (CC_DATA_ELEMENT_TAG_STR_SIZE - 1));
    }
    len = snprintf(outputPtr->decStr, CC_DATA_ELEMENT_DEC_STR_SIZE, "<--Seq:%d P%03d-B%02d", seqNum, pktSize, blkSize);
    ASSERT(len == (CC_DATA_ELEMENT_DEC_STR_SIZE - 1));

} // decodePacketStart()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodePacketData()
 |
 | DESCRIPTION:
 |    This function decodes a CEA-708 Packet Data
 -------------------------------------------------------------------------------*/
static void decodePacketData( CcDataOutputCtx* ctxPtr, uint8 ccData, char* tagStr, char* decStr, TextString* txtStr, char* errStr ) {
    int len;

    char* halfDecStr = decStr;

    if( ctxPtr->cea708State  == CEA708_STATE_EXTENDED_SEQ_NUM ) {
        ctxPtr->currentService = ccData & 0x3F;
        ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
        len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "S:%02d", ctxPtr->currentService);
        ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "ExSvc:%02d", ctxPtr->currentService);
        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
    } else if( ctxPtr->cea708State  == CEA708_STATE_DATA_WAIT ) {
        if( ccData == 0 ) {
// HACK - TODO - What happens in the other decode for this?
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "?00?");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "?\?\?-0x00");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
// HACK - Error, or no? There are lots in the bad asset...
//            LOG(DEBUG_LEVEL_WARN, DBG_CCD_OUT, "{%X} - Ignoring Spurious NULL on Srvc: %d", add708Error(ctxPtr, errStr), ctxPtr->currentService);
        } else if( ccData != DTVCC_C0_EXT1 ) {
            if ((ccData >= DTVCC_MIN_C0_CODE) && (ccData <= DTVCC_MAX_C0_CODE)) {
                decodeC0CmdCode(ctxPtr, ccData, tagStr, decStr, errStr);
            } else if ((ccData >= DTVCC_MIN_G0_CODE) && (ccData <= DTVCC_MAX_G0_CODE)) {
                if( strlen(DtvccDecodeG0CharSet(ccData)) == 1 ) {
                    len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "G0:%s", DtvccDecodeG0CharSet(ccData));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "G0Svc:%02d", ctxPtr->currentService);
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                } else {
                    len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "G%s", DtvccDecodeG0CharSet(ccData));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "S:%02d %s", ctxPtr->currentService, DtvccDecodeG0CharSet(ccData));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                }
                strcat(txtStr->txtStr708[ctxPtr->currentService-1], DtvccDecodeG0CharSet(ccData));
            } else if ((ccData >= DTVCC_MIN_C1_CODE) && (ccData <= DTVCC_MAX_C1_CODE)) {
                decodeC1CmdCode(ctxPtr, ccData, tagStr, decStr, errStr);
            } else {
                if( strlen(DtvccDecodeG1CharSet(ccData)) == 1 ) {
                    len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "G1:%s", DtvccDecodeG1CharSet(ccData));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "G1 - \"%s\"", DtvccDecodeG1CharSet(ccData));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                } else {
                    len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "G:%s", DtvccDecodeG1CharSet(ccData));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "G1 -\"%s\"", DtvccDecodeG1CharSet(ccData));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                }
                strcat(txtStr->txtStr708[ctxPtr->currentService-1], DtvccDecodeG1CharSet(ccData));
            }
        } else {  // Use Extended Set
            ctxPtr->cea708State = CEA708_STATE_EXTENDED_CODE;
            ctxPtr->cea708Code = CEA708_CODE_EXTENDED_CODE_UNKNOWN;
            ctxPtr->cea708BytesRemaining = 0;
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "EXT1");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "Extended");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
        }
    } else {
        switch( ctxPtr->cea708State ) {
            case CEA708_STATE_UNKNOWN:
                break;
            case CEA708_STATE_C0_CODE:
                decodeC0CmdCode(ctxPtr, ccData, tagStr, decStr, errStr);
                break;
            case CEA708_STATE_C1_CODE:
                decodeC1CmdCode(ctxPtr, ccData, tagStr, decStr, errStr);
                break;
            case CEA708_STATE_EXTENDED_CODE:
                decodeExtCmdCode(ctxPtr, ccData, tagStr, decStr, txtStr, errStr);
                break;
            default:
                LOG(DEBUG_LEVEL_ERROR, DBG_CCD_OUT, "{%X} - Unknown CEA-708 State: 0x%02X", add708Error(ctxPtr, errStr), ctxPtr->cea708State);
                break;
        }
    }
} // decodePacketData()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeC0CmdCode()
 |
 | DESCRIPTION:
 |    This function decodes C0 Commands, per the spec.
 -------------------------------------------------------------------------------*/
static void decodeC0CmdCode( CcDataOutputCtx* ctxPtr, uint8 ccData, char* tagStr, char* decStr, char* errStr ) {
    ASSERT(ctxPtr->currentService);
    int len;

    if( ctxPtr->cea708State == CEA708_STATE_DATA_WAIT ) {
        if (ccData <= 0x0F) {
            if ((ccData == DTVCC_C0_NUL) || (ccData == DTVCC_C0_ETX) || (ccData == DTVCC_C0_BS) ||
                (ccData == DTVCC_C0_FF) || (ccData == DTVCC_C0_CR) || (ccData == DTVCC_C0_HCR)) {
                if( strlen(DtvccDecodeC0Cmd(ccData)) == 2 ) {
                    len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "%s  ", DtvccDecodeC0Cmd(ccData));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C0: %s  ", DtvccDecodeC0Cmd(ccData));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                } else {
                    len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "%s ", DtvccDecodeC0Cmd(ccData));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C0: %s ", DtvccDecodeC0Cmd(ccData));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                }
            } else {
                len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C0:?");
                ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C0:%02X???", ccData);
                ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                LOG(DEBUG_LEVEL_WARN, DBG_CCD_OUT, "{%X} - Ignoring Unknown Command: 0x%02X Srvc: %d", add708Error(ctxPtr, errStr), ccData, ctxPtr->currentService);
            }
        } else if (ccData <= 0x17) {
            // Note that 0x10 is actually EXT1 and is dealt with somewhere else. Rest is undefined as per CEA-708-D
            ctxPtr->cea708State = CEA708_STATE_C0_CODE;
            ctxPtr->cea708Code = CEA708_CODE_UNKNOWN_C0_1;
            ctxPtr->cea708BytesRemaining = 1;
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C0:?");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C0:%02X?E1", ccData);
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
            LOG(DEBUG_LEVEL_WARN, DBG_CCD_OUT, "{%X} - Ignoring Unknown Command: 0x%02X Srvc: %d", add708Error(ctxPtr, errStr), ccData, ctxPtr->currentService);
        } else if( ccData <= DTVCC_MAX_C0_CODE ) {
            if( ccData == DTVCC_C0_P16 ) { // PE16
                ctxPtr->cea708State = CEA708_STATE_C0_CODE;
                ctxPtr->cea708Code = CEA708_CODE_P16_C0;
                ctxPtr->cea708BytesRemaining = 2;
                len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "P16?");
                ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "P16:UNH?");
                ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
            } else {
                ctxPtr->cea708State = CEA708_STATE_C0_CODE;
                ctxPtr->cea708Code = CEA708_CODE_UNKNOWN_C0_2;
                ctxPtr->cea708BytesRemaining = 2;
                len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C0:?");
                ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C0:%02X?X1", ccData);
                ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                LOG(DEBUG_LEVEL_WARN, DBG_CCD_OUT, "{%X} - Ignoring Unknown Command: 0x%02X Srvc: %d", add708Error(ctxPtr, errStr), ccData, ctxPtr->currentService);
            }
        }
    } else {
        switch( ctxPtr->cea708Code ) {
            case CEA708_CODE_P16_C0:
                len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "P16?");
                ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                if( ctxPtr->cea708BytesRemaining == 2 ) {
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "P16:%02X:1", ccData);
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                    ctxPtr->cea708BytesRemaining = 1;
                } else {
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "P16:%02X:2", ccData);
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                    ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                    ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                    ctxPtr->cea708BytesRemaining = 0;
                }
                break;
            case CEA708_CODE_UNKNOWN_C0_1:
                len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C0:?");
                ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C0:%02X?E2", ccData);
                ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                ctxPtr->cea708BytesRemaining = 0;
                break;
            case CEA708_CODE_UNKNOWN_C0_2:
                if( ctxPtr->cea708BytesRemaining == 2 ) {
                    len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C0:?");
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C0:%02X?X2", ccData);
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                    ctxPtr->cea708BytesRemaining = 1;
                } else {
                    len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C0:?");
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C0:%02X?X3", ccData);
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                    ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                    ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                    ctxPtr->cea708BytesRemaining = 0;
                }
                break;
            default:
                break;
        }
    }
} // decodeC0CmdCode()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeC1CmdCode()
 |
 | DESCRIPTION:
 |    This function decodes C1 Commands, per the spec.
 -------------------------------------------------------------------------------*/
static void decodeC1CmdCode( CcDataOutputCtx* ctxPtr, uint8 ccData, char* tagStr, char* decStr, char* errStr ) {
    ASSERT(ctxPtr->currentService);
    int len;

    if( ctxPtr->cea708State == CEA708_STATE_DATA_WAIT ) {
        // --- Set Current Window {CWx} --- DelayCancel {DLC} --- Reset {RST} ---
        if( (ccData == DTVCC_C1_CW0) || (ccData == DTVCC_C1_CW1) || (ccData == DTVCC_C1_CW2) ||
            (ccData == DTVCC_C1_CW3) || (ccData == DTVCC_C1_CW4) || (ccData == DTVCC_C1_CW5) ||
            (ccData == DTVCC_C1_CW6) || (ccData == DTVCC_C1_CW7) || (ccData == DTVCC_C1_DLC) ||
            (ccData == DTVCC_C1_RST) ) {
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "%s ", DtvccDecodeC1Cmd(ccData));
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C1:{%s}", DtvccDecodeC1Cmd(ccData));
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
            // --- Clear Windows {CLW} --- Display Windows {DSW} --- HideWindows {HDW} ---
            // --- ToggleWindows {TGW} --- Delete Windows {DLW} ---
        } else if( (ccData == DTVCC_C1_CLW) || (ccData == DTVCC_C1_DSW) || (ccData == DTVCC_C1_HDW) ||
                   (ccData == DTVCC_C1_TGW) || (ccData == DTVCC_C1_DLW) ) {
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "%s ", DtvccDecodeC1Cmd(ccData));
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C1:{%s}", DtvccDecodeC1Cmd(ccData));
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
            ctxPtr->cea708State = CEA708_STATE_C1_CODE;
            ctxPtr->cea708Code = CEA708_CODE_C1_BITMAP;
            ctxPtr->cea708BytesRemaining = 1;
            // --- Delay {DLY} in 100ms ---
        } else if( ccData == DTVCC_C1_DLY ) {
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "DLY ");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C1:{DLY}");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
            ctxPtr->cea708State = CEA708_STATE_C1_CODE;
            ctxPtr->cea708Code = CEA708_CODE_C1_DLY;
            ctxPtr->cea708BytesRemaining = 1;
            // --- Set Pen Attributes {SPA} ---
        } else if( ccData == DTVCC_C1_SPA ) {
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "SPA ");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C1:{SPA}");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
            ctxPtr->cea708State = CEA708_STATE_C1_CODE;
            ctxPtr->cea708Code = CEA708_CODE_C1_SPA;
            ctxPtr->cea708BytesRemaining = 2;
            // --- Set Pen Color {SPC} ---
        } else if( ccData == DTVCC_C1_SPC ) {
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "SPC ");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C1:{SPC}");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
            ctxPtr->cea708State = CEA708_STATE_C1_CODE;
            ctxPtr->cea708Code = CEA708_CODE_C1_SPC;
            ctxPtr->cea708BytesRemaining = 3;
            // --- Set Pen Location {SPL} ---
        } else if( ccData == DTVCC_C1_SPL ) {
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "SPL ");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C1:{SPL}");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
            ctxPtr->cea708State = CEA708_STATE_C1_CODE;
            ctxPtr->cea708Code = CEA708_CODE_C1_SPL;
            ctxPtr->cea708BytesRemaining = 2;
            // --- Reserved {RSVxx} ---
        } else if( (ccData == DTVCC_C1_RSV93) || (ccData == DTVCC_C1_RSV94) || (ccData == DTVCC_C1_RSV95) ||
                   (ccData == DTVCC_C1_RSV96) ) {
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "RSV ");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "{RSV-%X} ", ccData);
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
            // --- Set Window Attributes {SWA} ---
        } else if( ccData == DTVCC_C1_SWA ) {
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "SWA ");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C1:{SWA}");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
            ctxPtr->cea708State = CEA708_STATE_C1_CODE;
            ctxPtr->cea708Code = CEA708_CODE_C1_SWA;
            ctxPtr->cea708BytesRemaining = 4;
            // --- Define Window {DFWx} ---
        } else if( (ccData == DTVCC_C1_DF0) || (ccData == DTVCC_C1_DF1) || (ccData == DTVCC_C1_DF2) ||
                   (ccData == DTVCC_C1_DF3) || (ccData == DTVCC_C1_DF4) || (ccData == DTVCC_C1_DF5) ||
                   (ccData == DTVCC_C1_DF6) || (ccData == DTVCC_C1_DF7) ) {
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "%s ", DtvccDecodeC1Cmd(ccData));
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C1:{%s}", DtvccDecodeC1Cmd(ccData));
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
            ctxPtr->cea708State = CEA708_STATE_C1_CODE;
            ctxPtr->cea708Code = CEA708_CODE_C1_DFW;
            ctxPtr->cea708BytesRemaining = 6;
        } else {
            ASSERT(0);
        }
    } else {
        switch (ctxPtr->cea708Code) {
            case CEA708_CODE_C1_BITMAP:
                len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "BITM");
                ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "%s", uint8toBitArray(ccData));
                ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                ctxPtr->cea708BytesRemaining = 0;
                break;
            case CEA708_CODE_C1_DLY:
                len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "DLY1");
                ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                if( ccData < 10 ) {
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "_%d00 ms_", ccData);
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                } else if( ccData < 100 ) {
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "%d00  ms", ccData);
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                } else {
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "%d00 ms", ccData);
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                }
                ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                ctxPtr->cea708BytesRemaining = 0;
                break;
            case CEA708_CODE_C1_SPA:
                len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "SPA%d", (3 - ctxPtr->cea708BytesRemaining) );
                ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                if( ctxPtr->cea708BytesRemaining == 2 ) {
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, " %d %d %02d ", (ccData & PEN_SIZE_MASK),
                                   ((ccData & PEN_OFFSET_MASK) >> PEN_OFFSET_NORM_SHIFT), ((ccData & TEXT_TAG_MASK) >> TEST_TAG_NORM_SHIFT));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                    ctxPtr->cea708BytesRemaining = 1;
                } else {
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, " %d %d %c %c", (ccData & FONT_TAG_MASK),
                                   ((ccData & EDGE_TYPE_MASK) >> EDGE_TYPE_NORM_SHIFT),
                                   (((ccData & PA_UNDERLINE_TOGGLE_MASK) == PA_UNDERLINE_TOGGLE_SET)?'T':'F'),
                                   (((ccData & PA_ITALIC_TOGGLE_MASK) == PA_ITALIC_TOGGLE_SET)?'T':'F'));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                    ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                    ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                    ctxPtr->cea708BytesRemaining = 0;
                }
                break;
            case CEA708_CODE_C1_SPC:
                len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "SPC%d", (4 - ctxPtr->cea708BytesRemaining) );
                ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                switch( ctxPtr->cea708BytesRemaining ) {
                    case 3:
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "%d  %d:%d:%d", ((ccData & FORGROUND_OPACITY_MASK) >> FORGROUND_OPACITY_NORM_SHIFT),
                                       ((ccData & FORGROUND_RED_COLOR_CMP_MASK) >> FORGROUND_RED_COLOR_CMP_NORM_SHIFT),
                                       ((ccData & FORGROUND_GREEN_COLOR_CMP_MASK) >> FORGROUND_GREEN_COLOR_CMP_NORM_SHIFT), (ccData & FORGROUND_BLUE_COLOR_CMP_MASK) );
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 2;
                        break;
                    case 2:
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "%d  %d:%d:%d",
                                       ((ccData & BACKGROUND_OPACITY_MASK) >> BACKGROUND_OPACITY_NORM_SHIFT),
                                       ((ccData & BACKGROUND_RED_COLOR_CMP_MASK) >> BACKGROUND_RED_COLOR_CMP_NORM_SHIFT),
                                       ((ccData & BACKGROUND_GREEN_COLOR_CMP_MASK) >> BACKGROUND_GREEN_COLOR_CMP_NORM_SHIFT),
                                       (ccData & BACKGROUND_BLUE_COLOR_CMP_MASK) );
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 1;
                        break;
                    case 1:
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "RGB%d:%d:%d",
                                       ((ccData & EDGE_RED_COLOR_CMP_MASK) >> EDGE_RED_COLOR_CMP_NORM_SHIFT),
                                       ((ccData & EDGE_GREEN_COLOR_CMP_MASK) >> EDGE_GREEN_COLOR_CMP_NORM_SHIFT),
                                       (ccData & EDGE_BLUE_COLOR_CMP_MASK));
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                        ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                        ctxPtr->cea708BytesRemaining = 0;
                        break;
                    default:
                        ASSERT(0);
                }
                break;
            case CEA708_CODE_C1_SPL:
                len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "SPL%d", (3 - ctxPtr->cea708BytesRemaining) );
                ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                if( ctxPtr->cea708BytesRemaining == 2 ) {
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, " Row-%02d ", (ccData & PEN_LOC_ROW_MASK));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                    ctxPtr->cea708BytesRemaining = 1;
                } else {
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, " Col-%02d ", (ccData & PEN_LOC_COLUMN_MASK));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                    ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                    ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                    ctxPtr->cea708BytesRemaining = 0;
                }
                break;
            case CEA708_CODE_C1_SWA:
                len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "SWA%d", (5 - ctxPtr->cea708BytesRemaining) );
                ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                switch( ctxPtr->cea708BytesRemaining ) {
                    case 4:
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "%d  %d:%d:%d",
                                       ((ccData & FILL_OPACITY_MASK) >> FILL_OPACITY_NORM_SHIFT),
                                       ((ccData & FILL_RED_COLOR_COMP_MASK) >> FILL_RED_COLOR_COMP_SHIFT),
                                       ((ccData & FILL_GREEN_COLOR_COMP_MASK) >> FILL_GREEN_COLOR_COMP_SHIFT),
                                       (ccData & FILL_BLUE_COLOR_COMP_MASK));
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 3;
                        break;
                    case 3:
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "%d  %d:%d:%d",
                                       ((ccData & BORDER_TYPE_LOWER_BITS_MASK) >> BORDER_TYPE_LB_NORM_SHIFT),
                                       ((ccData & BORDER_RED_COLOR_COMP_MASK) >> BORDER_RED_COLOR_COMP_SHIFT),
                                       ((ccData & BORDER_GREEN_COLOR_COMP_MASK) >> BORDER_GREEN_COLOR_COMP_SHIFT),
                                       (ccData & BORDER_BLUE_COLOR_COMP_MASK));
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 2;
                        break;
                    case 2:
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "%d %d %d %d%c",
                                       ((ccData & BORDER_TYPE_HIGH_BIT_MASK) >> BORDER_TYPE_HB_NORM_SHIFT),
                                       ((ccData & PRINT_DIRECTION_MASK) >> PRINT_DIRECTION_NORM_SHIFT),
                                       ((ccData & SCROLL_DIRECTION_MASK) >> SCROLL_DIRECTION_NORM_SHIFT),
                                       (ccData & JUSTIFY_DIRECTION_MASK),
                                       ((ccData & WORD_WRAP_TOGGLE_MASK) == WORD_WRAP_TOGGLE_SET)?'T':'F');
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 1;
                        break;
                    case 1:
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "%d %ss %d",
                                       (ccData & DISPLAY_EFFECT_MASK),
                                       DECODE_EFFECT_SPEED(((ccData & EFFECT_SPEED_MASK) >> EFFECT_SPEED_NORM_SHIFT)),
                                       ((ccData & EFFECT_DIRECTION_MASK) >> EFFECT_DIRECTION_NORM_SHIFT));
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                        ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                        ctxPtr->cea708BytesRemaining = 2;
                        break;
                    default:
                        ASSERT(0);
                }
                break;
            case CEA708_CODE_C1_DFW:
                len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "DFW%d", (7 - ctxPtr->cea708BytesRemaining) );
                ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                switch( ctxPtr->cea708BytesRemaining ) {
                    case 6:
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "P%d %c %c %c",
                                       (ccData & PRIORITY_MASK),
                                       ((ccData & VISIBLE_TOGGLE_MASK) == VISIBLITY_SET)?'T':'F',
                                       ((ccData & ROW_LOCK_TOGGLE_MASK) == ROW_LOCK_TOGGLE_SET)?'T':'F',
                                       ((ccData & COLUMN_LOCK_TOGGLE_MASK) == COLUMN_LOCK_TOGGLE_SET)?'T':'F');
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 5;
                        break;
                    case 5:
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "VA-%03d %c", (ccData & VERTICAL_ANCHOR_MASK),
                                       ((ccData & RELATIVE_POSITIONING_MASK) == RELATIVE_POSITIONING_SET)?'T':'F');
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 4;
                        break;
                    case 4:
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "HrzA-%03d", ccData);
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 3;
                        break;
                    case 3:
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "RN-%02d %s", (ccData & ROW_COUNT_MASK),
                                       DECODE_ANCOR_POSITION(((ccData & ANCHOR_ID_MASK) >> ANCHOR_ID_NORM_SHIFT)));
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 2;
                        break;
                    case 2:
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "ColN-%03d", (ccData & COLUMN_COUNT_MASK));
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 1;
                        break;
                    case 1:
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "WS%d  PS%d", ((ccData & WINDOW_STYLE_MASK) >> WINDOW_STYLE_NORM_SHIFT), (ccData & PEN_STYLE_MASK));
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                        ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                        ctxPtr->cea708BytesRemaining = 0;
                        break;
                    default:
                        ASSERT(0);
                }
                break;
            default:
                ASSERT(0);
                break;
        }
    }
}  // decodeC1CmdCode()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeExtCmdCode()
 |
 | DESCRIPTION:
 |    This function decodes C0 Commands, per the spec.
 -------------------------------------------------------------------------------*/
static void decodeExtCmdCode( CcDataOutputCtx* ctxPtr, uint8 ccData, char* tagStr, char* decStr, TextString* txtStr, char* errStr ) {
    int len;

    if( ctxPtr->cea708Code == CEA708_CODE_EXTENDED_CODE_UNKNOWN ) {
        if( (ccData >= DTVCC_MIN_C0_CODE) && (ccData <= DTVCC_MAX_C0_CODE) ) { // C2: Extended Misc. Control Codes
            ctxPtr->cea708Code = CEA708_CODE_C2;
            ctxPtr->cea708BytesRemaining = 4;
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C2:1");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C2:1-%02X?", ccData);
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
            LOG(DEBUG_LEVEL_WARN, DBG_CCD_OUT, "{%X} - Skipping C2 Code: 0x%02X Srvc: %d", add708Error(ctxPtr, errStr), ccData, ctxPtr->currentService);
        } else if( (ccData >= DTVCC_MIN_G0_CODE) && (ccData <= DTVCC_MAX_G0_CODE)) {  // G2: Extended Misc. Characters
            ctxPtr->cea708Code = CEA708_CODE_G2;
            ctxPtr->cea708BytesRemaining = 1;
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "G2:1");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "G2:1-%02X ", ccData);
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
        } else if( (ccData >= DTVCC_MIN_C1_CODE) && (ccData <= DTVCC_MAX_C1_CODE) ) {
            ctxPtr->cea708Code = CEA708_CODE_C3;
            ctxPtr->cea708BytesRemaining = 6;
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C3:1");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C3:1-%02X?", ccData);
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
            LOG(DEBUG_LEVEL_WARN, DBG_CCD_OUT, "{%X} - Skipping C3 Code: 0x%02X Srvc: %d", add708Error(ctxPtr, errStr), ccData, ctxPtr->currentService);
        } else {  // G3 Character Set (Basically just the [CC] Symbol).
            ctxPtr->cea708Code = CEA708_CODE_G3;
            ctxPtr->cea708BytesRemaining = 1;
            len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "G3:1");
            ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
            len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "G3:1-%02X ", ccData);
            ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
        }
    } else {
        switch (ctxPtr->cea708Code) {
            case CEA708_CODE_C2:
                switch (ctxPtr->cea708BytesRemaining) {
                    case 4:
                        /* This section is for future codes. While by definition we can't do any work on them, we must advance */
                        /* however many bytes would be consumed if these codes were supported, as defined in the specs.        */
                        // WARN: This code is completely untested due to lack of samples. Just following specs!
                        if (ccData < 0x07) { // 00-07 : Single-byte control bytes (0 additional bytes)
                            ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                            ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                            ctxPtr->cea708BytesRemaining = 0;
                        } else if (ccData < 0x0F) { // 08-0F : Two-byte control codes (1 additional byte)
                            ctxPtr->cea708BytesRemaining = 1;
                        } else if (ccData < 0x0F) { // 10-17 : Three-byte control codes (2 additional bytes)
                            ctxPtr->cea708BytesRemaining = 2;
                        } else {  // 18-1F : Four-byte control codes (3 additional bytes)
                            ctxPtr->cea708BytesRemaining = 3;
                        }
                        len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C2:2");
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C2:2-%02X?", ccData);
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        break;
                    case 3:
                        len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C2:3");
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C2:3-%02X?", ccData);
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 2;
                        break;
                    case 2:
                        len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C2:4");
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C2:4-%02X?", ccData);
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 1;
                        break;
                    case 1:
                        len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C2:5");
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C2:5-%02X?", ccData);
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                        ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                        ctxPtr->cea708BytesRemaining = 0;
                        break;
                    default:
                        ASSERT(0);
                        break;
                }
                break;
            case CEA708_CODE_G2:
                ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                ctxPtr->cea708BytesRemaining = 0;
                if ((ccData == 0x20) || (ccData == 0x21) || (ccData == 0x25) || (ccData == 0x2A) ||
                    (ccData == 0x2C) || ((ccData >= 0x30) && (ccData <= 0x35)) || (ccData == 0x39) ||
                    (ccData == 0x3A) || (ccData == 0x3C) || (ccData == 0x3D) || (ccData == 0x3F) ||
                    ((ccData >= 0x76) && (ccData <= 0x7F))) {
                    if( strlen(DtvccDecodeG2CharSet(ccData)) == 2 ) {
                        len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "G2%s", DtvccDecodeG2CharSet(ccData));
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "G2-'%s' ", DtvccDecodeG2CharSet(ccData));
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                    } else {
                        len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "G2:%s", DtvccDecodeG2CharSet(ccData));
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "G2 - '%s'", DtvccDecodeG2CharSet(ccData));
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                    }
                    strcat(txtStr->txtStr708[ctxPtr->currentService-1], DtvccDecodeG2CharSet(ccData));
                } else {
                    len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "G2:2");
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "G2:2-%02X?", ccData);
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                    LOG(DEBUG_LEVEL_WARN, DBG_CCD_OUT, "{%X} - Skipping Unknown G2 Char: 0x%02X Srvc: %d",
                        add708Error(ctxPtr, errStr), ccData, ctxPtr->currentService);
                }
                break;
            case CEA708_CODE_C3:
                switch (ctxPtr->cea708BytesRemaining) {
                    case 6:
                        if ((ccData < 0x80) || (ccData > 0x9F)) {
                            ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                            ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                            ctxPtr->cea708BytesRemaining = 0;
                        } else if (ccData <= 0x87) { // 80-87 : Five-byte control bytes (4 additional bytes)
                            ctxPtr->cea708BytesRemaining = 4;
                        } else if (ccData <= 0x8F) { // 88-8F : Six-byte control codes (5 additional byte)
                            ctxPtr->cea708BytesRemaining = 5;
                        } else {
                            // 90-9F : These are variable length commands, that can even span several segments.
                            // They were envisioned for things like downloading fonts and graphics.
                            // We are not supporting this set of data.
                            ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                            ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                            ctxPtr->cea708BytesRemaining = 0;
                            LOG(DEBUG_LEVEL_WARN, DBG_CCD_OUT,
                                "{%X} - SLikely Data Corruption. Unsupported C3 Data Range: 0x%02X %d",
                                add708Error(ctxPtr, errStr), ccData, ctxPtr->currentService);
                        }
                        len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C3:2");
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C3:2-%02X?", ccData);
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        break;
                    case 5:
                        len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C3:3");
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C3:3-%02X?", ccData);
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 4;
                        break;
                    case 4:
                        len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C3:4");
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C3:4-%02X?", ccData);
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 3;
                        break;
                    case 3:
                        len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C3:5");
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C3:5-%02X?", ccData);
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 2;
                        break;
                    case 2:
                        len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C3:6");
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C3:6-%02X?", ccData);
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708BytesRemaining = 1;
                        break;
                    case 1:
                        len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "C3:7");
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                        len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "C3:7-%02X?", ccData);
                        ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                        ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                        ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                        ctxPtr->cea708BytesRemaining = 0;
                        break;
                    default:
                        ASSERT(0);
                        break;
                }
                break;
            case CEA708_CODE_G3:
                ctxPtr->cea708State = CEA708_STATE_DATA_WAIT;
                ctxPtr->cea708Code = CEA708_CODE_UNKNOWN;
                ctxPtr->cea708BytesRemaining = 0;
                if( ccData != DTVCC_G3_CC_ICON ) {
                    len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "G3%s", DtvccDecodeG3CharSet(ccData));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "G3: '%s'", DtvccDecodeG3CharSet(ccData));
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                    strcat(txtStr->txtStr708[ctxPtr->currentService-1], DtvccDecodeG3CharSet(ccData));
                } else {
                    len = snprintf(tagStr, CC_DATA_ELEMENT_HALF_TAG_STR_SIZE, "G3:2");
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_TAG_STR_SIZE - 1));
                    len = snprintf(decStr, CC_DATA_ELEMENT_HALF_DEC_STR_SIZE, "G3:2-%02X?", ccData);
                    ASSERT(len == (CC_DATA_ELEMENT_HALF_DEC_STR_SIZE - 1));
                    LOG(DEBUG_LEVEL_WARN, DBG_CCD_OUT, "{%X} - Skipping Unknown G3 Char: 0x%02X Srvc: %d",
                        add708Error(ctxPtr, errStr), ccData, ctxPtr->currentService);
                }
                break;
            default:
                ASSERT(0);
                break;
        }
    }
}  // decodeExtCmdCode()

/*------------------------------------------------------------------------------
 | NAME:
 |    add708Error()
 |
 | DESCRIPTION:
 |    This function adds a logged error into the CEA-708 Strings to the right.
 -------------------------------------------------------------------------------*/
static uint16 add708Error( CcDataOutputCtx* ctxPtr, char* errStr ) {
    ctxPtr->cea708ErrNum = ctxPtr->cea708ErrNum + 1;
    if( strlen(errStr) == 0 ) {
        snprintf(errStr, CEA708_ERROR_STR_SIZE + 1, "*** {%X}", ctxPtr->cea708ErrNum );
    } else {
        char tmpErrStr[CEA708_ERROR_STR_SIZE];
        snprintf(tmpErrStr, CEA708_ERROR_STR_SIZE - strlen(errStr) + 1, " {%X}", ctxPtr->cea708ErrNum );
        strcat(errStr, tmpErrStr);
// HACK - TODO - Trunkating Errors?
    }
    return ctxPtr->cea708ErrNum;
}  // add708Error()
