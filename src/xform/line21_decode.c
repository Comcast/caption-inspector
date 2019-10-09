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

#include "line21_decode.h"
#include "cc_utils.h"
#include "line21_output.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

static boolean decodeCaptionData( Line21DecodeCtx*, uint8, uint8, Line21Code*, CaptionTime* );
static boolean decodeGlobalCmd( Line21DecodeCtx*, uint8, uint8, Line21Code*, CaptionTime* );
static boolean decodeMidRowCode( Line21DecodeCtx*, uint8, uint8, Line21Code* );
static boolean decodeTabCtrl( Line21DecodeCtx*, uint8, uint8, Line21Code* );
static boolean decodePAC( Line21DecodeCtx*, uint8, uint8, Line21Code* );
static boolean decodeBasicChars( Line21DecodeCtx*, uint8, uint8, Line21Code*, CaptionTime* );
static boolean decodeSpecialChar( Line21DecodeCtx*, uint8, uint8, Line21Code*, CaptionTime* );
static boolean decodeExtendedChar( Line21DecodeCtx*, uint8, uint8, Line21Code*, CaptionTime* );
static uint8 decodeColumn( uint8 );

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21DecodeInitialize()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    processOnly - Decode only (to get metrics) = TRUE; Decode and Output = FALSE
 |
 | RETURN VALUES:
 |    LinkInfo -  Information about this element of the pipeline, such that it can
 |                be chained to elements that can consume the specific type of data
 |                that it produces.
 |
 | DESCRIPTION:
 |    This initializes this element of the pipeline.
 -------------------------------------------------------------------------------*/
LinkInfo Line21DecodeInitialize( Context* rootCtxPtr, boolean processOnly ) {
    ASSERT(rootCtxPtr);
    ASSERT(!rootCtxPtr->line21DecodeCtxPtr);

    rootCtxPtr->line21DecodeCtxPtr = malloc(sizeof(Line21DecodeCtx));
    Line21DecodeCtx* ctxPtr = rootCtxPtr->line21DecodeCtxPtr;

    ctxPtr->processOnly = processOnly;
    ctxPtr->foundText = NO_TEXT_FOUND;
    ctxPtr->currentChannel[CEA608E_LINE21_FIELD_1_CC] = LINE21_CHANNEL_UNKONWN;
    ctxPtr->currentChannel[CEA608E_LINE21_FIELD_2_CC] = LINE21_CHANNEL_UNKONWN;
    for( int loop = 0; loop <= LINE21_MAX_NUM_CHANNELS; loop++ ) {
        ctxPtr->dataFound[loop] = FALSE;
        ctxPtr->isPopOnCaptioning[loop] = FALSE;
        ctxPtr->isRollUpCaptioning[loop] = FALSE;
        ctxPtr->isPaintOnCaptioning[loop] = FALSE;
        ctxPtr->captioningChange[loop] = FALSE;
        ctxPtr->captioningStart[loop] = FALSE;
    }

    InitSinks(&ctxPtr->sinks, CC_DATA___LINE21_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___LINE21_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_DECODED_608;
    linkInfo.NextBufferFnPtr = &Line21DecodeProcNextBuffer;
    linkInfo.ShutdownFnPtr = &Line21DecodeShutdown;
    return linkInfo;
}  // Line21DecodeInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21DecodeAddSink()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    linkInfo - Describes the potential next element of the pipeline.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method binds the next element to this element, after validating that
 |    the data it receives is compatible with the data tht this element sends.
 -------------------------------------------------------------------------------*/
boolean Line21DecodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    ASSERT(rootCtxPtr);
    ASSERT(rootCtxPtr->line21DecodeCtxPtr);

    if( rootCtxPtr->line21DecodeCtxPtr->processOnly == TRUE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_608_DEC, "Added a Sink after specifying none would be added");
    }

    if( linkInfo.sourceType != DATA_TYPE_DECODED_608 ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_608_DEC, "Attempt to add Sink that cannot process Line 21 / CEA-608 Data");
        return FALSE;
    }
    
    return AddSink(&rootCtxPtr->line21DecodeCtxPtr->sinks, &linkInfo);
}  // Line21DecodeAddSink()

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21DecodeProcNextBuffer()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |    inBuffer - Pointer to the buffer to process.
 |
 | RETURN VALUES:
 |    uint8 - Success is TRUE / PIPELINE_SUCCESS, Failure is FALSE / PIPELINE_FAILURE
 |            All other codes specified in header.
 |
 | DESCRIPTION:
 |    This method processes an incoming buffer, decoding  the byte pair from
 |    field 1 or field 2 of the Line 21 Data for CEA-608.
 -------------------------------------------------------------------------------*/
uint8 Line21DecodeProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    ASSERT(inBuffer);
    ASSERT(inBuffer->dataPtr);
    ASSERT(inBuffer->numElements);
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->line21DecodeCtxPtr);
    Line21DecodeCtx* ctxPtr = ((Context*)rootCtxPtr)->line21DecodeCtxPtr;
    char captionTimeStr[CAPTION_TIME_SCRATCH_BUFFER_SIZE];
    uint8 numOutputElements = 0;

    for( int loop = 0; loop < inBuffer->numElements; loop = loop + 3 ) {
        if( (((inBuffer->dataPtr[loop+1] & LINE_21_PARITY_MASK) != NULL_CAPTION_DATA) ||
             ((inBuffer->dataPtr[loop+2] & LINE_21_PARITY_MASK) != NULL_CAPTION_DATA)) &&
            (((inBuffer->dataPtr[loop] & CC_CONSTR_CC_TYPE_MASK) == CEA608E_LINE21_FIELD_1_CC) ||
             ((inBuffer->dataPtr[loop] & CC_CONSTR_CC_TYPE_MASK) == CEA608E_LINE21_FIELD_2_CC)) &&
            (inBuffer->dataPtr[loop] & CC_CONSTR_CC_VALID_FLAG_MASK) == CC_CONSTR_CC_VALID_FLAG_SET ) {
            numOutputElements++;
        }
    }

    if( numOutputElements == 0) {
        FreeBuffer(inBuffer);
        return PIPELINE_SUCCESS;
    }

    Buffer* outBuffer = NewBuffer(BUFFER_TYPE_LINE_21, numOutputElements);
    Line21Code* codePtr = (Line21Code*)outBuffer->dataPtr;
    outBuffer->captionTime = inBuffer->captionTime;
    outBuffer->numElements = 0;

    for( int loop = 0; loop < inBuffer->numElements; loop = loop + 3 ) {
        if( (((inBuffer->dataPtr[loop+1] & LINE_21_PARITY_MASK) != NULL_CAPTION_DATA) ||
             ((inBuffer->dataPtr[loop+2] & LINE_21_PARITY_MASK) != NULL_CAPTION_DATA)) &&
            (((inBuffer->dataPtr[loop] & CC_CONSTR_CC_TYPE_MASK) == CEA608E_LINE21_FIELD_1_CC) ||
             ((inBuffer->dataPtr[loop] & CC_CONSTR_CC_TYPE_MASK) == CEA608E_LINE21_FIELD_2_CC)) &&
            (inBuffer->dataPtr[loop] & CC_CONSTR_CC_VALID_FLAG_MASK) == CC_CONSTR_CC_VALID_FLAG_SET ) {

            if( (inBuffer->dataPtr[loop] & CC_CONSTR_CC_TYPE_MASK) == CEA608E_LINE21_FIELD_1_CC ) {
                codePtr->fieldNum = CEA608E_LINE21_FIELD_1_CC;
            } else {
                codePtr->fieldNum = CEA608E_LINE21_FIELD_2_CC;
            }

            codePtr->codeType = LINE21_CODE_TYPE_UNKNOWN;

            boolean wasDecoded = decodeCaptionData(ctxPtr, inBuffer->dataPtr[loop+1], inBuffer->dataPtr[loop+2], codePtr, &inBuffer->captionTime);

            if( wasDecoded == TRUE ) {
                outBuffer->numElements = outBuffer->numElements + 1;
                codePtr++;
            } else {
                encodeTimeCode(&inBuffer->captionTime, captionTimeStr);
                LOG(DEBUG_LEVEL_ERROR, DBG_608_DEC, "Unable to decode Line 21 Data Pair at %s (%d): 0x%02X 0x%02X 0x%02X",
                    captionTimeStr, loop/3, inBuffer->dataPtr[loop], inBuffer->dataPtr[loop+1], inBuffer->dataPtr[loop+2]);
            }
        }
    }

    FreeBuffer(inBuffer);
    if( ((Context*)rootCtxPtr)->line21DecodeCtxPtr->processOnly == TRUE ) {
        FreeBuffer(outBuffer);
        if( ctxPtr->foundText == TEXT_FOUND ) {
            ctxPtr->foundText = TEXT_REPORTED;
            return FIRST_TEXT_FOUND;
        }
        return PIPELINE_SUCCESS;
    } else {
        if( ctxPtr->foundText == TEXT_FOUND ) {
            ctxPtr->foundText = TEXT_REPORTED;
            uint8 retval = PassToSinks(rootCtxPtr, outBuffer, &ctxPtr->sinks);
            if( retval != PIPELINE_SUCCESS ) {
                LOG(DEBUG_LEVEL_ERROR, DBG_608_DEC, "First Text Found eclipsed non Success Response: %d", retval);
            }
            return FIRST_TEXT_FOUND;
        } else {
            return PassToSinks(rootCtxPtr, outBuffer, &ctxPtr->sinks);
        }
    }
}  // Line21DecodeProcNextBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21DecodeShutdown()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |
 | RETURN VALUES:
 |    uint8 - Success is TRUE / PIPELINE_SUCCESS, Failure is FALSE / PIPELINE_FAILURE
 |            All other codes specified in header.
 |
 | DESCRIPTION:
 |    This method is called when the previous element in the pipeline determines
 |    that there is no more data coming down the pipeline. This element will
 |    perform any necessary actions as a result and pass this call down the
 |    pipeline.
 -------------------------------------------------------------------------------*/
uint8 Line21DecodeShutdown( void* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->line21DecodeCtxPtr);
    Line21DecodeCtx* ctxPtr = ((Context*)rootCtxPtr)->line21DecodeCtxPtr;
    Sinks sinks = ctxPtr->sinks;
    boolean processOnly = ctxPtr->processOnly;

    for( int loop = 1; loop <= LINE21_MAX_NUM_CHANNELS; loop++ ) {
        if( ctxPtr->dataFound[loop] == TRUE ) {
            if( ctxPtr->captioningChange[loop] == FALSE ) {
                if( ctxPtr->isPopOnCaptioning[loop] == TRUE ) {
                    LOG(DEBUG_LEVEL_INFO, DBG_608_DEC, "Found Line 21 PopOn Captioning on Channel %d", loop);
                } else if( ctxPtr->isRollUpCaptioning[loop] == TRUE ) {
                    LOG(DEBUG_LEVEL_INFO, DBG_608_DEC, "Found Line 21 RollUp Captioning on Channel %d", loop);
                } else if( ctxPtr->isPaintOnCaptioning[loop] == TRUE ) {
                    LOG(DEBUG_LEVEL_INFO, DBG_608_DEC, "Found Line 21 PaintOn Captioning on Channel %d", loop);
                } else {
                    ASSERT(0);
                }
            } else {
                LOG(DEBUG_LEVEL_WARN, DBG_608_DEC, "Ambiguous Line 21 Captioning Type on Channel %d: PopOn - %d; RollUp - %d; PaintOn - %d",
                    loop, ctxPtr->isPopOnCaptioning[loop], ctxPtr->isRollUpCaptioning[loop], ctxPtr->isPaintOnCaptioning[loop] );
            }
        }
    }

    free(ctxPtr);
    ((Context*)rootCtxPtr)->line21DecodeCtxPtr = NULL;

    if( processOnly ) {
        return PIPELINE_SUCCESS;
    } else {
        return ShutdownSinks(rootCtxPtr, &sinks);
    }
}  // Line21DecodeShutdown()

/*------------------------------------------------------------------------------
 | NAME:
 |    Line21PacDecodeRow()
 |
 | INPUT PARAMETERS:
 |    ccData1 - First Byte of CC Data.
 |    ccData2 - Second Byte of CC Data.
 |
 | RETURN VALUES:
 |    uint8 - Row Number
 |
 | DESCRIPTION:
 |    This method decodes a Line 21 Tab Offset Code and passes back a null
 |    terminated reference to the string in memory
 -------------------------------------------------------------------------------*/
uint8 Line21PacDecodeRow( uint8 ccData1, uint8 ccData2 ) {
    uint8 rowCode = ((ccData1 & PAC_ROW_POS_MASK) << 1) | ((ccData2 & PAC_WHICH_ROW_MASK) >> 5);
    uint8 retval = 0;

    switch(rowCode) {
        case PAC_ROW_1:
            retval = 1;
            break;
        case PAC_ROW_2:
            retval = 2;
            break;
        case PAC_ROW_3:
            retval = 3;
            break;
        case PAC_ROW_4:
            retval = 4;
            break;
        case PAC_ROW_5:
            retval = 5;
            break;
        case PAC_ROW_6:
            retval = 6;
            break;
        case PAC_ROW_7:
            retval = 7;
            break;
        case PAC_ROW_8:
            retval = 8;
            break;
        case PAC_ROW_9:
            retval = 9;
            break;
        case PAC_ROW_10:
            retval = 10;
            break;
        case PAC_ROW_11:
            retval = 11;
            break;
        case PAC_ROW_12:
            retval = 12;
            break;
        case PAC_ROW_13:
            retval = 13;
            break;
        case PAC_ROW_14:
            retval = 14;
            break;
        case PAC_ROW_15:
            retval = 15;
            break;
        default:
            LOG( DEBUG_LEVEL_ERROR, DBG_608_OUT, "Invalid PAC Row, Using 0: 0x%02X 0x%02X %d", ccData1, ccData2, rowCode );
            break;
    }

    return retval;
} // Line21PacDecodeRow()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeCaptionData()
 |
 | DESCRIPTION:
 |    This function decodes the Line 21 Caption Data by calling lower funcitons.
 -------------------------------------------------------------------------------*/
static boolean decodeCaptionData( Line21DecodeCtx* ctxPtr, uint8 firstByte, uint8 secondByte, Line21Code* codePtr, CaptionTime* captionTimePtr ) {
    boolean wasDecoded;

    firstByte = firstByte & LINE_21_PARITY_MASK;
    secondByte = secondByte & LINE_21_PARITY_MASK;

    wasDecoded = decodeGlobalCmd(ctxPtr, firstByte, secondByte, codePtr, captionTimePtr);
    if (wasDecoded == FALSE) wasDecoded = decodeMidRowCode(ctxPtr, firstByte, secondByte, codePtr);
    if (wasDecoded == FALSE) wasDecoded = decodeTabCtrl(ctxPtr, firstByte, secondByte, codePtr);
    if (wasDecoded == FALSE) wasDecoded = decodePAC(ctxPtr, firstByte, secondByte, codePtr);
    if (wasDecoded == FALSE) wasDecoded = decodeSpecialChar(ctxPtr, firstByte, secondByte, codePtr, captionTimePtr);
    if (wasDecoded == FALSE) wasDecoded = decodeExtendedChar(ctxPtr, firstByte, secondByte, codePtr, captionTimePtr);
    if (wasDecoded == FALSE) wasDecoded = decodeBasicChars(ctxPtr, firstByte, secondByte, codePtr, captionTimePtr);

    return wasDecoded;
}  // decodeCaptionData()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeGlobalCmd()
 |
 | DESCRIPTION:
 |    This function decodes the Line 21 Global Command per the CEA-608 Spec.
 -------------------------------------------------------------------------------*/
static boolean decodeGlobalCmd( Line21DecodeCtx* ctxPtr, uint8 ccData1, uint8 ccData2, Line21Code* codePtr, CaptionTime* captionTimePtr ) {
    char captionTimeStr[CAPTION_TIME_SCRATCH_BUFFER_SIZE];
    
    if( (ccData1 != GLOBAL_CTRL_CODE_CC1) && (ccData1 != GLOBAL_CTRL_CODE_CC2) &&
       (ccData1 != GLOBAL_CTRL_CODE_CC3) && (ccData1 != GLOBAL_CTRL_CODE_CC4) ) {
        return FALSE;
    }
    
    if( (ccData2 & GLOBAL_CTRL_CMD_MASK) != ccData2 ) {
        return FALSE;
    }
    
    ASSERT(codePtr);
    codePtr->codeType = LINE21_GLOBAL_CONTROL_CODE;
    
    switch( ccData1 ) {
        case GLOBAL_CTRL_CODE_CC1:
            codePtr->channelNum = LINE21_CHANNEL_1;
            ctxPtr->dataFound[LINE21_CHANNEL_1] = TRUE;
            codePtr->code.globalControl.chan = LINE21_CHANNEL_1;
            ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_1;
            break;
        case GLOBAL_CTRL_CODE_CC2:
            codePtr->channelNum = LINE21_CHANNEL_2;
            ctxPtr->dataFound[LINE21_CHANNEL_2] = TRUE;
            codePtr->code.globalControl.chan = LINE21_CHANNEL_2;
            ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_2;
            break;
        case GLOBAL_CTRL_CODE_CC3:
            codePtr->channelNum = LINE21_CHANNEL_3;
            ctxPtr->dataFound[LINE21_CHANNEL_3] = TRUE;
            codePtr->code.globalControl.chan = LINE21_CHANNEL_3;
            ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_3;
            break;
        case GLOBAL_CTRL_CODE_CC4:
            codePtr->channelNum = LINE21_CHANNEL_4;
            ctxPtr->dataFound[LINE21_CHANNEL_4] = TRUE;
            codePtr->code.globalControl.chan = LINE21_CHANNEL_4;
            ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_4;
            break;
        default:
            codePtr->code.globalControl.chan = LINE21_CHANNEL_UNKONWN;
            LOG(DEBUG_LEVEL_ERROR, DBG_608_DEC, "Impossible Code Branch: 0x%02X", ccData1);
            break;
    }

    if( ((codePtr->channelNum == LINE21_CHANNEL_1) && (codePtr->fieldNum == CEA608E_LINE21_FIELD_2_CC)) ||
        ((codePtr->channelNum == LINE21_CHANNEL_2) && (codePtr->fieldNum == CEA608E_LINE21_FIELD_2_CC)) ||
        ((codePtr->channelNum == LINE21_CHANNEL_3) && (codePtr->fieldNum == CEA608E_LINE21_FIELD_1_CC)) ||
        ((codePtr->channelNum == LINE21_CHANNEL_4) && (codePtr->fieldNum == CEA608E_LINE21_FIELD_1_CC)) ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_608_DEC, "Channel/Field Mismatch: Chan - %d Field - %d", codePtr->channelNum, codePtr->fieldNum);
    }
    
    codePtr->code.globalControl.cmd = ccData2;

    if( (ccData2 == GLOBAL_CTRL_CODE__RCL) || (ccData2 == GLOBAL_CTRL_CODE__EOC) ) {
        if( (ccData2 != GLOBAL_CTRL_CODE__RCL) && (ctxPtr->captioningStart[codePtr->code.globalControl.chan] == FALSE) ) {
            ctxPtr->captioningStart[codePtr->code.globalControl.chan] = TRUE;
            encodeTimeCode(captionTimePtr, captionTimeStr);
            LOG(DEBUG_LEVEL_INFO, DBG_608_DEC, "Line 21 PopOn Captioning on Channel %d started at %s", codePtr->code.globalControl.chan, captionTimeStr);
        }
        if( ctxPtr->captioningChange[codePtr->code.globalControl.chan] == FALSE ) {
            ctxPtr->isPopOnCaptioning[codePtr->code.globalControl.chan] = TRUE;
            if (ctxPtr->isRollUpCaptioning[codePtr->code.globalControl.chan] == TRUE) {
                LOG(DEBUG_LEVEL_WARN, DBG_608_DEC, "Caption Format change on Channel %d: RollUp -> PopUp", codePtr->code.globalControl.chan);
                ctxPtr->captioningChange[codePtr->code.globalControl.chan] = TRUE;
            }
            if (ctxPtr->isPaintOnCaptioning[codePtr->code.globalControl.chan] == TRUE) {
                LOG(DEBUG_LEVEL_WARN, DBG_608_DEC, "Caption Format change on Channel %d: PaintOn -> PopUp", codePtr->code.globalControl.chan);
                ctxPtr->captioningChange[codePtr->code.globalControl.chan] = TRUE;
            }
        }
    } else if( (ccData2 == GLOBAL_CTRL_CODE__RDC) || (ccData2 == GLOBAL_CTRL_CODE__DER) ) {
        if( (ccData2 != GLOBAL_CTRL_CODE__DER) && (ctxPtr->captioningStart[codePtr->code.globalControl.chan] == FALSE) ) {
            ctxPtr->captioningStart[codePtr->code.globalControl.chan] = TRUE;
            encodeTimeCode(captionTimePtr, captionTimeStr);
            LOG(DEBUG_LEVEL_INFO, DBG_608_DEC, "Line 21 PaintOn Captioning on Channel %d started at %s", codePtr->code.globalControl.chan, captionTimeStr);
        }
        if( ctxPtr->captioningChange[codePtr->code.globalControl.chan] == FALSE ) {
            ctxPtr->isPaintOnCaptioning[codePtr->code.globalControl.chan] = TRUE;
            if( ctxPtr->isPopOnCaptioning[codePtr->code.globalControl.chan] == TRUE ) {
                LOG(DEBUG_LEVEL_WARN, DBG_608_DEC, "Caption Format change on Channel %d: PopUp -> PaintOn", codePtr->code.globalControl.chan);
                ctxPtr->captioningChange[codePtr->code.globalControl.chan] = TRUE;
            }
            if( ctxPtr->isRollUpCaptioning[codePtr->code.globalControl.chan] == TRUE ) {
                LOG(DEBUG_LEVEL_WARN, DBG_608_DEC, "Caption Format change on Channel %d: RollUp -> PaintOn", codePtr->code.globalControl.chan);
                ctxPtr->captioningChange[codePtr->code.globalControl.chan] = TRUE;
            }
        }
    } else if( (ccData2 == GLOBAL_CTRL_CODE__RU2) || (ccData2 == GLOBAL_CTRL_CODE__RU3) ||
               (ccData2 == GLOBAL_CTRL_CODE__RU4) || (ccData2 == GLOBAL_CTRL_CODE__CR) ) {
        if( (ccData2 != GLOBAL_CTRL_CODE__CR) && (ctxPtr->captioningStart[codePtr->code.globalControl.chan] == FALSE) ) {
            ctxPtr->captioningStart[codePtr->code.globalControl.chan] = TRUE;
            encodeTimeCode(captionTimePtr, captionTimeStr);
            LOG(DEBUG_LEVEL_INFO, DBG_608_DEC, "Line 21 RollUp Captioning on Channel %d started at %s", codePtr->code.globalControl.chan, captionTimeStr);
        }
        if( ctxPtr->captioningChange[codePtr->code.globalControl.chan] == FALSE ) {
            ctxPtr->isRollUpCaptioning[codePtr->code.globalControl.chan] = TRUE;
            if( ctxPtr->isPopOnCaptioning[codePtr->code.globalControl.chan] == TRUE ) {
                LOG(DEBUG_LEVEL_WARN, DBG_608_DEC, "Caption Format change on Channel %d: PopUp -> RollUp", codePtr->code.globalControl.chan);
                ctxPtr->captioningChange[codePtr->code.globalControl.chan] = TRUE;
            }
            if( ctxPtr->isPaintOnCaptioning[codePtr->code.globalControl.chan] == TRUE ) {
                LOG(DEBUG_LEVEL_WARN, DBG_608_DEC, "Caption Format change on Channel %d: PaintOn -> RollUp", codePtr->code.globalControl.chan);
                ctxPtr->captioningChange[codePtr->code.globalControl.chan] = TRUE;
            }
        }
    }

    return TRUE;
}  // decodeGlobalCmd()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeMidRowCode()
 |
 | DESCRIPTION:
 |    This function decodes the Line 21 Mid-Row Code per the CEA-608 Spec.
 -------------------------------------------------------------------------------*/
static boolean decodeMidRowCode( Line21DecodeCtx* ctxPtr, uint8 ccData1, uint8 ccData2, Line21Code* codePtr ) {

    if( (ccData1 != CMD_MIDROW_BG_CHAN_1_3) && (ccData1 != CMD_MIDROW_FG_CHAN_1_3) &&
       (ccData1 != CMD_MIDROW_BG_CHAN_2_4) && (ccData1 != CMD_MIDROW_FG_CHAN_2_4) ) {
        return FALSE;
    }
    
    if( (ccData2 & MIDROW_CODE_MASK) != ccData2 ) {
        return FALSE;
    }
    
    ASSERT(codePtr);
    codePtr->codeType = LINE21_MID_ROW_CONTROL_CODE;
    
    switch( ccData1 ) {
        case CMD_MIDROW_BG_CHAN_1_3:
        case CMD_MIDROW_FG_CHAN_1_3:
            codePtr->code.midRow.chan = LINE21_CHANNEL_1_3;
            if( codePtr->fieldNum == CEA608E_LINE21_FIELD_1_CC ) {
                codePtr->channelNum = LINE21_CHANNEL_1;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_1;
            } else {
                codePtr->channelNum = LINE21_CHANNEL_3;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_3;
            }
            break;
        case CMD_MIDROW_BG_CHAN_2_4:
        case CMD_MIDROW_FG_CHAN_2_4:
            codePtr->code.midRow.chan = LINE21_CHANNEL_2_4;
            if( codePtr->fieldNum == CEA608E_LINE21_FIELD_1_CC ) {
                codePtr->channelNum = LINE21_CHANNEL_2;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_2;
            } else {
                codePtr->channelNum = LINE21_CHANNEL_4;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_4;
            }
            break;
        default:
            codePtr->code.midRow.chan = LINE21_CHANNEL_UNKONWN;
            LOG(DEBUG_LEVEL_ERROR, DBG_608_DEC, "Impossible Code Branch: 0x%02X", ccData1);
            break;
    }
    
    if( (ccData1 & MIDROW_BG_FG_MASK) == MIDROW_FOREGROUND_STYLE ) {
        codePtr->code.midRow.backgroundOrForeground = MIDROW_FOREGROUND_STYLE;
        codePtr->code.midRow.isPartiallyTransparent = ccData2 & MIDROW_TRANSPARENCY_MASK;
        codePtr->code.midRow.backgroundForgroundData.style = ccData2 & MIDROW_FG_STYLE_MASK;
    } else {
        codePtr->code.midRow.backgroundOrForeground = MIDROW_BACKGROUND_COLOR;
        codePtr->code.midRow.isUnderlined = ccData2 & MIDROW_UNDERLINE_MASK;
        codePtr->code.midRow.backgroundForgroundData.color = ccData2 & MIDROW_BG_COLOR_MASK;
    }
    
    return TRUE;
}  // decodeMidRowCode()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeTabCtrl()
 |
 | DESCRIPTION:
 |    This function decodes a Tab Control Code in Line 21 Data per the CEA-608 Spec.
 -------------------------------------------------------------------------------*/
static boolean decodeTabCtrl( Line21DecodeCtx* ctxPtr, uint8 ccData1, uint8 ccData2, Line21Code* codePtr ) {
    
    if( (ccData1 != CMD_TAB_HI_CC_1_3) && (ccData1 != CMD_TAB_HI_CC_2_4) ) {
        return FALSE;
    }
    
    if( (ccData2 & TAB_CMD_MASK) != ccData2 ) {
        return FALSE;
    }
    
    ASSERT(codePtr);
    codePtr->codeType = LINE21_TAB_CONTROL_CODE;
    
    switch( ccData1 ) {
        case CMD_TAB_HI_CC_1_3:
            codePtr->code.tabControl.chan = LINE21_CHANNEL_1_3;
            if( codePtr->fieldNum == CEA608E_LINE21_FIELD_1_CC ) {
                codePtr->channelNum = LINE21_CHANNEL_1;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_1;
            } else {
                codePtr->channelNum = LINE21_CHANNEL_3;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_3;
            }
            break;
        case CMD_TAB_HI_CC_2_4:
            codePtr->code.tabControl.chan = LINE21_CHANNEL_2_4;
            if( codePtr->fieldNum == CEA608E_LINE21_FIELD_1_CC ) {
                codePtr->channelNum = LINE21_CHANNEL_2;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_2;
            } else {
                codePtr->channelNum = LINE21_CHANNEL_4;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_4;
            }
            break;
        default:
            codePtr->code.tabControl.chan = LINE21_CHANNEL_UNKONWN;
            LOG(DEBUG_LEVEL_ERROR, DBG_608_DEC, "Impossible Code Branch: 0x%02X", ccData1);
            break;
    }
    
    codePtr->code.tabControl.tabOffset = ccData2;
    
    return TRUE;
}  // decodeTabCtrl()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodePAC()
 |
 | DESCRIPTION:
 |    This function decodes the Line 21 Preamble Access Code per the CEA-608 Spec.
 -------------------------------------------------------------------------------*/
static boolean decodePAC( Line21DecodeCtx* ctxPtr, uint8 ccData1, uint8 ccData2, Line21Code* codePtr ) {
    
    if( (ccData1 & PAC_CC1_MASK) != ccData1 ) {
        return FALSE;
    }
    
    ASSERT(codePtr);
    codePtr->codeType = LINE21_PREAMBLE_ACCESS_CODE;
    
    switch( ccData1 & PAC_CHANNEL_MASK ) {
        case PAC_CHANNEL_1_3:
            codePtr->code.pac.chan = LINE21_CHANNEL_1_3;
            if( codePtr->fieldNum == CEA608E_LINE21_FIELD_1_CC ) {
                codePtr->channelNum = LINE21_CHANNEL_1;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_1;
            } else {
                codePtr->channelNum = LINE21_CHANNEL_3;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_3;
            }
            break;
        case PAC_CHANNEL_2_4:
            codePtr->code.pac.chan = LINE21_CHANNEL_2_4;
            if( codePtr->fieldNum == CEA608E_LINE21_FIELD_1_CC ) {
                codePtr->channelNum = LINE21_CHANNEL_2;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_2;
            } else {
                codePtr->channelNum = LINE21_CHANNEL_4;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_4;
            }
            break;
        default:
            codePtr->code.pac.chan = LINE21_CHANNEL_UNKONWN;
            LOG(DEBUG_LEVEL_ERROR, DBG_608_DEC, "Impossible Code Branch: 0x%02X", ccData1);
            break;
    }
    
    codePtr->code.pac.rowNumber = Line21PacDecodeRow(ccData1,ccData2);
    
    if( (ccData2 & PAC_STYLE_ADDR_MASK) == PAC_ADDRESS_SET ) {
        codePtr->code.pac.styleAddr = PAC_ADDRESS_CODE;
        codePtr->code.pac.styleAddrData.cursor = decodeColumn(ccData2);
    } else {
        codePtr->code.pac.styleAddr = PAC_STYLE_CODE;
        codePtr->code.pac.styleAddrData.color = ccData2 & PAC_COLOR_STYLE_MASK;
    }
    
    codePtr->code.pac.isUnderlined = ccData2 & PAC_UNDERLINE_MASK;
    
    return TRUE;
}  // decodePAC()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeBasicChars()
 |
 | DESCRIPTION:
 |    This function decodes Basic Characters in Line 21 Data per the CEA-608 Spec.
 -------------------------------------------------------------------------------*/
static boolean decodeBasicChars( Line21DecodeCtx* ctxPtr, uint8 ccData1, uint8 ccData2, Line21Code* codePtr, CaptionTime* captionTimePtr ) {
    ASSERT(codePtr);

    if( (ctxPtr->currentChannel[codePtr->fieldNum] < 1) || (ctxPtr->currentChannel[codePtr->fieldNum] > 4) ) {
        LOG(DEBUG_LEVEL_WARN, DBG_608_DEC, "Dropping Chars for unknown channel : %d - 0x%02X 0x%02X",
            ctxPtr->currentChannel[codePtr->fieldNum], ccData1, ccData2);
        return FALSE;
    }
    
    if( (((ccData1 >= FIRST_BASIC_CHAR) && (ccData1 <= LAST_BASIC_CHAR)) || (ccData1 == NULL_BASIC_CHAR)) &&
        (((ccData2 >= FIRST_BASIC_CHAR) && (ccData2 <= LAST_BASIC_CHAR)) || (ccData2 == NULL_BASIC_CHAR)) ) {
        codePtr->codeType = LINE21_BASIC_CHARS;
        
        codePtr->code.basicChars.charOne = ccData1;
        codePtr->code.basicChars.charTwo = ccData2;

        codePtr->channelNum = ctxPtr->currentChannel[codePtr->fieldNum];

        if( ctxPtr->foundText == NO_TEXT_FOUND ) {
            ctxPtr->foundText = TEXT_FOUND;
            char captionTimeStr[CAPTION_TIME_SCRATCH_BUFFER_SIZE];
            encodeTimeCode(captionTimePtr, captionTimeStr);
            LOG(DEBUG_LEVEL_INFO, DBG_608_DEC, "Line 21 First Character of text found on Channel %d at %s", codePtr->channelNum, captionTimeStr);
        }

        return TRUE;
    }
    return FALSE;
}  // decodeBasicChars()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeSpecialChar()
 |
 | DESCRIPTION:
 |    This function decodes a Special Character in Line 21 Data per the CEA-608 Spec.
 -------------------------------------------------------------------------------*/
static boolean decodeSpecialChar( Line21DecodeCtx* ctxPtr, uint8 ccData1, uint8 ccData2, Line21Code* codePtr, CaptionTime* captionTimePtr ) {
    
    if( (ccData1 != SPCL_NA_CHAR_SET_CH_1_3) && (ccData1 != SPCL_NA_CHAR_SET_CH_2_4) ) {
        return FALSE;
    }
    
    if( (ccData2 & SPCL_NA_CHAR_SET_MASK) != ccData2 ) {
        return FALSE;
    }
    
    ASSERT(codePtr);
    codePtr->codeType = LINE21_SPECIAL_CHAR;
    
    switch( ccData1 ) {
        case SPCL_NA_CHAR_SET_CH_1_3:
            codePtr->code.specialChar.chan = LINE21_CHANNEL_1_3;
            if( codePtr->fieldNum == CEA608E_LINE21_FIELD_1_CC ) {
                codePtr->channelNum = LINE21_CHANNEL_1;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_1;
            } else {
                codePtr->channelNum = LINE21_CHANNEL_3;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_3;
            }
            break;
        case SPCL_NA_CHAR_SET_CH_2_4:
            codePtr->code.specialChar.chan = LINE21_CHANNEL_2_4;
            if( codePtr->fieldNum == CEA608E_LINE21_FIELD_1_CC ) {
                codePtr->channelNum = LINE21_CHANNEL_2;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_2;
            } else {
                codePtr->channelNum = LINE21_CHANNEL_4;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_4;
            }
            break;
        default:
            codePtr->code.specialChar.chan = LINE21_CHANNEL_UNKONWN;
            LOG(DEBUG_LEVEL_ERROR, DBG_608_DEC, "Impossible Code Branch: 0x%02X", ccData1);
            break;
    }
    
    codePtr->code.specialChar.spChar = ccData2 & SPCL_NA_CHAR_MASK;

    if( ctxPtr->foundText == NO_TEXT_FOUND ) {
        ctxPtr->foundText = TEXT_FOUND;
        char captionTimeStr[CAPTION_TIME_SCRATCH_BUFFER_SIZE];
        encodeTimeCode(captionTimePtr, captionTimeStr);
        LOG(DEBUG_LEVEL_INFO, DBG_608_DEC, "Line 21 First Character of text found on Channel %d at %s", codePtr->channelNum, captionTimeStr);
    }
    
    return TRUE;
}  // decodeSpecialChar()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeExtendedChar()
 |
 | DESCRIPTION:
 |    This function decodes a Extended Character in Line 21 Data per the CEA-608 Spec.
 -------------------------------------------------------------------------------*/
static boolean decodeExtendedChar( Line21DecodeCtx* ctxPtr, uint8 ccData1, uint8 ccData2, Line21Code* codePtr, CaptionTime* captionTimePtr ) {
    
    if( (ccData1 != EXT_W_EURO_CHAR_SET_CH_1_3_SF) && (ccData1 != EXT_W_EURO_CHAR_SET_CH_1_3_FG) &&
       (ccData1 != EXT_W_EURO_CHAR_SET_CH_2_4_SF) && (ccData1 != EXT_W_EURO_CHAR_SET_CH_2_4_FG) ) {
        return FALSE;
    }
    
    if( (ccData2 & EXT_W_EURO_CHAR_SET_MASK) != ccData2 ) {
        return FALSE;
    }
    
    ASSERT(codePtr);
    codePtr->codeType = LINE21_EXTENDED_CHAR;
    
    switch( ccData1 ) {
        case EXT_W_EURO_CHAR_SET_CH_1_3_SF:
        case EXT_W_EURO_CHAR_SET_CH_1_3_FG:
            codePtr->code.extendedChar.chan = LINE21_CHANNEL_1_3;
            codePtr->code.extendedChar.charSet = EXT_W_EURO_CHAR_SET_SPANISH_FRENCH;
            codePtr->code.extendedChar.exChar = ccData2;
            if( codePtr->fieldNum == CEA608E_LINE21_FIELD_1_CC ) {
                codePtr->channelNum = LINE21_CHANNEL_1;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_1;
            } else {
                codePtr->channelNum = LINE21_CHANNEL_3;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_3;
            }
            break;
        case EXT_W_EURO_CHAR_SET_CH_2_4_SF:
        case EXT_W_EURO_CHAR_SET_CH_2_4_FG:
            codePtr->code.extendedChar.chan = LINE21_CHANNEL_2_4;
            codePtr->code.extendedChar.charSet = EXT_W_EURO_CHAR_SET_DUTCH_GERMAN;
            codePtr->code.extendedChar.exChar = ccData2;
            if( codePtr->fieldNum == CEA608E_LINE21_FIELD_1_CC ) {
                codePtr->channelNum = LINE21_CHANNEL_2;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_2;
            } else {
                codePtr->channelNum = LINE21_CHANNEL_4;
                ctxPtr->currentChannel[codePtr->fieldNum] = LINE21_CHANNEL_4;
            }
            break;
        default:
            codePtr->code.extendedChar.chan = LINE21_CHANNEL_UNKONWN;
            LOG(DEBUG_LEVEL_ERROR, DBG_608_DEC, "Impossible Code Branch: 0x%02X", ccData1);
            break;
    }

    if( ctxPtr->foundText == NO_TEXT_FOUND ) {
        ctxPtr->foundText = TEXT_FOUND;
        char captionTimeStr[CAPTION_TIME_SCRATCH_BUFFER_SIZE];
        encodeTimeCode(captionTimePtr, captionTimeStr);
        LOG(DEBUG_LEVEL_INFO, DBG_608_DEC, "Line 21 First Character of text found on Channel %d at %s", codePtr->channelNum, captionTimeStr);
    }
    
    return TRUE;
}  // decodeExtendedChar()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeColumn()
 |
 | DESCRIPTION:
 |    This method decodes a Line 21 PAC Cursor Column.
 -------------------------------------------------------------------------------*/
static uint8 decodeColumn( uint8 ccData ) {
    uint8 columnCode = ccData & PAC_CURSOR_ADDRESS_MASK;
    uint8 retval = 0;

    switch( columnCode ){
        case PAC_CURSOR_COLUMN_0:
            retval = 0;
            break;
        case PAC_CURSOR_COLUMN_4:
            retval = 4;
            break;
        case PAC_CURSOR_COLUMN_8:
            retval = 8;
            break;
        case PAC_CURSOR_COLUMN_12:
            retval = 12;
            break;
        case PAC_CURSOR_COLUMN_16:
            retval = 16;
            break;
        case PAC_CURSOR_COLUMN_20:
            retval = 20;
            break;
        case PAC_CURSOR_COLUMN_24:
            retval = 24;
            break;
        case PAC_CURSOR_COLUMN_28:
            retval = 28;
            break;
        default:
            LOG(DEBUG_LEVEL_ERROR, DBG_608_DEC, "Invalid Colum Code: 0x%02X 0x%02X", ccData, columnCode);
            break;
    }

    return retval;
}
