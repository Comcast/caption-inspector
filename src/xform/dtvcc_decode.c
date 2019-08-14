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

#include <string.h>

#include "debug.h"

#include "dtvcc_decode.h"
#include "dtvcc_output.h"
#include "output_utils.h"
#include "cc_data_output.h"
#include "cc_utils.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

static const char* ccTypeStr[4] = { "F1", "F2", "PD", "PS" };

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

static void processCurrentPacket( Context*, CaptionTime* );
static uint16 countDataPackets( uint8*, uint8 );
static void processServiceBlock( DtvccDecodeCtx*, uint8*, uint8, Buffer*, uint8, uint8 );
static int8 parseC0CmdCode( DtvccDecodeCtx*, uint8*, uint8, DtvccData*, uint8 );
static int8 parseC1CmdCode( DtvccDecodeCtx*, uint8*, uint8, DtvccData*, uint8, CaptionTime* );

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    DtvccDecodeInitialize()
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
LinkInfo DtvccDecodeInitialize( Context* rootCtxPtr, boolean processOnly ) {
    ASSERT(rootCtxPtr);
    ASSERT(!rootCtxPtr->dtvccDecodeCtxPtr);

    rootCtxPtr->dtvccDecodeCtxPtr = malloc(sizeof(DtvccDecodeCtx));
    DtvccDecodeCtx* ctxPtr = rootCtxPtr->dtvccDecodeCtxPtr;

    ctxPtr->processOnly = processOnly;
    ctxPtr->processedFine = TRUE;
    ctxPtr->firstPacket = TRUE;
    ctxPtr->foundText = NO_TEXT_FOUND;
    ctxPtr->numP16Cmds = 0;
    ctxPtr->pktLenMismatches = 0;
    ctxPtr->dtvccPacketLength = 0;
    ctxPtr->lastSequence = DTVCC_NO_LAST_SEQUENCE;
    ctxPtr->activeServices = 0;
    for( int loop = 0; loop < DTVCC_MAX_NUM_SERVICES; loop++ ) {
        ctxPtr->isPopOnCaptioning[loop] = FALSE;
        ctxPtr->isRollUpCaptioning[loop] = FALSE;
        ctxPtr->isTickerCaptioning[loop] = FALSE;
        ctxPtr->captioningChange[loop] = FALSE;
        ctxPtr->captioningStart[loop] = FALSE;
    }

    InitSinks(&ctxPtr->sinks, CC_DATA___DTVCC_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___DTVCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_DECODED_708;
    linkInfo.NextBufferFnPtr = &DtvccDecodeProcNextBuffer;
    linkInfo.ShutdownFnPtr = &DtvccDecodeShutdown;
    return linkInfo;
}  // DtvccDecodeInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    DtvccDecodeAddSink()
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
boolean DtvccDecodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {

    if( rootCtxPtr->dtvccDecodeCtxPtr->processOnly == TRUE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_708_DEC, "Added a Sink after specifying none would be added");
    }

    if( linkInfo.sourceType != DATA_TYPE_DECODED_708 ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_708_DEC, "Attempt to add Sink that cannot process DTVCC / CEA-708 Data");
        return FALSE;
    }
    
    return AddSink(&rootCtxPtr->dtvccDecodeCtxPtr->sinks, &linkInfo);
}  // Line21DecodeAddSink()

/*------------------------------------------------------------------------------
 | NAME:
 |    DtvccDecodeProcNextBuffer()
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
 |    This method processes an incoming buffer, decoding the CC Data per the
 |    CEA-708D Specification.
 -------------------------------------------------------------------------------*/
uint8 DtvccDecodeProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    ASSERT(inBuffer);
    ASSERT(inBuffer->dataPtr);
    ASSERT(inBuffer->numElements);
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->dtvccDecodeCtxPtr);
    DtvccDecodeCtx* ctxPtr = ((Context*)rootCtxPtr)->dtvccDecodeCtxPtr;
    char captionTimeStr[CAPTION_TIME_SCRATCH_BUFFER_SIZE];

    for( int loop = 0; loop < inBuffer->numElements; loop = loop + 3 ) {
        ASSERT((loop+2) < inBuffer->numElements);
        boolean ccValid = ((inBuffer->dataPtr[loop] & CC_CONSTR_CC_VALID_FLAG_MASK) == CC_CONSTR_CC_VALID_FLAG_SET);
        uint8 ccType = inBuffer->dataPtr[loop] & CC_CONSTR_CC_TYPE_MASK;
        
        LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "Construct %d : 0x%02X -> Type = %s Valid = %s : Data = %02X %02X",
            ((loop/3)+1), inBuffer->dataPtr[loop], ccTypeStr[ccType], trueFalseStr[ccValid], inBuffer->dataPtr[loop+1], inBuffer->dataPtr[loop+2] );
        
        if( ccType == DTVCCC_CHANNEL_PACKET_START ) {
            processCurrentPacket(rootCtxPtr, &inBuffer->captionTime);
            ctxPtr->dtvccPacketLength = 0;
            if( ccValid == TRUE ) {
                if( (ctxPtr->dtvccPacketLength + 2) < DTVCC_MAX_PACKET_LENGTH ) {
                    ctxPtr->dtvccPacket[ctxPtr->dtvccPacketLength++] = inBuffer->dataPtr[loop+1];
                    ctxPtr->dtvccPacket[ctxPtr->dtvccPacketLength++] = inBuffer->dataPtr[loop+2];
                } else {
                    encodeTimeCode(&inBuffer->captionTime, captionTimeStr);
                    LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "At %s Max Packet Length Exceeded, skipping new.", captionTimeStr);
                }
            }
        } else if( (ccValid == TRUE) && (ccType == DTVCCC_CHANNEL_PACKET_DATA) ) {
            if( (ctxPtr->dtvccPacketLength + 2) < DTVCC_MAX_PACKET_LENGTH ) {
                ctxPtr->dtvccPacket[ctxPtr->dtvccPacketLength++] = inBuffer->dataPtr[loop+1];
                ctxPtr->dtvccPacket[ctxPtr->dtvccPacketLength++] = inBuffer->dataPtr[loop+2];
            } else {
                encodeTimeCode(&inBuffer->captionTime, captionTimeStr);
                LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "At %s Max Packet Length Exceeded, skipping new.", captionTimeStr);
            }
        }
    }
    
    FreeBuffer(inBuffer);

    if( ctxPtr->foundText == TEXT_FOUND ) {
        ctxPtr->foundText = TEXT_REPORTED;
        if( ctxPtr->processedFine != PIPELINE_SUCCESS ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_708_DEC, "First Text Found eclipsed non Success Response: %d", ctxPtr->processedFine);
        }
        return FIRST_TEXT_FOUND;
    } else {
        return ctxPtr->processedFine;
    }
}  // DtvccDecodeProcNextBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    DtvccDecodeShutdown()
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
 |    that there is no more data coming down the pipline. This element will
 |    perform any necessary actions as a result and pass this call down the
 |    pipeline.
 -------------------------------------------------------------------------------*/
uint8 DtvccDecodeShutdown( void* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->dtvccDecodeCtxPtr);
    DtvccDecodeCtx* ctxPtr = ((Context*)rootCtxPtr)->dtvccDecodeCtxPtr;
    Sinks sinks = ctxPtr->sinks;
    boolean processOnly = ctxPtr->processOnly;

    if( ctxPtr->numP16Cmds > 6 ) {
        LOG(DEBUG_LEVEL_INFO, DBG_708_DEC, "Suppressed %d additional occurrences of the warning for finding P16 Commands", ctxPtr->numP16Cmds - 6);
    }

    if( ctxPtr->pktLenMismatches > 6 ) {
        LOG(DEBUG_LEVEL_INFO, DBG_708_DEC, "Suppressed %d additional occurrences of the warning for Packet Length Mismatches", ctxPtr->pktLenMismatches - 6);
    }

    uint64 compareBit = 1;
    for( int loop = 0; loop < 64; loop++ ) {
        if( (ctxPtr->activeServices & compareBit) == compareBit ) {
            if( ctxPtr->captioningChange[loop] == FALSE ) {
                if( ctxPtr->isPopOnCaptioning[loop] == TRUE ) {
                    LOG(DEBUG_LEVEL_INFO, DBG_708_DEC, "Found DTVCC PopOn Captioning Data on Service %d", loop+1);
                } else if( ctxPtr->isRollUpCaptioning[loop] == TRUE ) {
                    LOG(DEBUG_LEVEL_INFO, DBG_708_DEC, "Found DTVCC RollUp Captioning on Service %d", loop+1);
                } else if( ctxPtr->isTickerCaptioning[loop] == TRUE ) {
                    LOG(DEBUG_LEVEL_INFO, DBG_708_DEC, "Found DTVCC TickerTape Captioning on Service %d", loop+1);
                } else {
                    LOG(DEBUG_LEVEL_WARN, DBG_708_DEC, "Unset DTVCC Captioning Type on Service %d: PopOn - %d; RollUp - %d; TickerTape - %d",
                        loop+1, ctxPtr->isPopOnCaptioning[loop], ctxPtr->isRollUpCaptioning[loop], ctxPtr->isTickerCaptioning[loop] );
                }
            } else {
                LOG(DEBUG_LEVEL_INFO, DBG_708_DEC, "Ambiguous DTVCC Captioning Type on Service %d: PopOn - %d; RollUp - %d; TickerTape - %d",
                    loop+1, ctxPtr->isPopOnCaptioning[loop], ctxPtr->isRollUpCaptioning[loop], ctxPtr->isTickerCaptioning[loop] );
            }
        }
        compareBit = compareBit << 1;
    }

    free(ctxPtr);
    ((Context*)rootCtxPtr)->dtvccDecodeCtxPtr = NULL;

    if( processOnly ) {
        return PIPELINE_SUCCESS;
    } else {
        return ShutdownSinks(rootCtxPtr, &sinks);
    }
}  // DtvccDecodeShutdown()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    processCurrentPacket()
 |
 | DESCRIPTION:
 |    This function processes the accumulated packet on the start of a new packet,
 |    which indicates the completion of a previous packet.
 -------------------------------------------------------------------------------*/
static void processCurrentPacket( Context* rootCtxPtr, CaptionTime* captionTimePtr ) {
    DtvccDecodeCtx* ctxPtr = rootCtxPtr->dtvccDecodeCtxPtr;
    char captionTimeStr[CAPTION_TIME_SCRATCH_BUFFER_SIZE];

#if 0
    if( (captionTimePtr->hour == 0) && (captionTimePtr->minute == 4) &&
        (captionTimePtr->second == 13) && (captionTimePtr->millisecond == 294) ) {
        printf("BREAKPOINT!\n");
    }
#endif

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
    
    uint8 sequence_number = (ctxPtr->dtvccPacket[0] & PACKET_SEQ_NUM_MASK) >> PACKET_SEQ_SHIFT;
    uint8 packet_size_code = ctxPtr->dtvccPacket[0] & PACKET_LENGTH_MASK;
    
    if( GetMinDebugLevel(DBG_708_DEC) < DEBUG_LEVEL_INFO ) {
        char charArrayBig[1024];
        char charArraySmall[10];
        charArrayBig[0] = '\0';
        for( int loop = 0; loop < ctxPtr->dtvccPacketLength; loop++ ) {
            sprintf(charArraySmall, "%02X ", ctxPtr->dtvccPacket[loop]);
            strcat(charArrayBig, charArraySmall);
        }
        LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "Processing Packet - Seq: %d Size: %d Length: %d -> [ %s]",
            sequence_number, packet_size_code, ctxPtr->dtvccPacketLength, charArrayBig );
    }
    
    if( ctxPtr->dtvccPacketLength  == 0 ) {
        if( ctxPtr->firstPacket == FALSE ) {
            encodeTimeCode(captionTimePtr, captionTimeStr);
            LOG(DEBUG_LEVEL_WARN, DBG_708_DEC, "Zero Packet Length at %s.", captionTimeStr);
        }
        ctxPtr->firstPacket = FALSE;
        return;
    }

    ctxPtr->firstPacket = FALSE;
    uint8 packet_size;
    if( packet_size_code == 0 ) {
        packet_size = 128;
    } else {
        packet_size = 2 * packet_size_code;
    }
    
    // packet_data_size = packet_size - 1
    if( ctxPtr->dtvccPacketLength != packet_size ) {
        encodeTimeCode(captionTimePtr, captionTimeStr);
        if( ctxPtr->pktLenMismatches < 5 ) {
            LOG(DEBUG_LEVEL_WARN, DBG_708_DEC, "Mismatch in Packet length at %s: Advertised %d vs. found %d",
                captionTimeStr, packet_size, ctxPtr->dtvccPacketLength);
        } else if( ctxPtr->pktLenMismatches == 5 ) {
            LOG(DEBUG_LEVEL_WARN, DBG_708_DEC, "Mismatch in Packet length at %s: Advertised %d vs. found %d --- Suppressing Subsequent Warnings.",
                captionTimeStr, packet_size, ctxPtr->dtvccPacketLength);
        } else {
            LOG(DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "Mismatch in Packet length at %s: Advertised %d vs. found %d",
                captionTimeStr, packet_size, ctxPtr->dtvccPacketLength);
        }
        ctxPtr->pktLenMismatches++;
    }
    
    if( (ctxPtr->lastSequence != DTVCC_NO_LAST_SEQUENCE) && (((ctxPtr->lastSequence + 1) % 4) != sequence_number) ) {
        encodeTimeCode(captionTimePtr, captionTimeStr);
        LOG( DEBUG_LEVEL_WARN, DBG_708_DEC, "Unexpected sequence number at %s, it is [%d] but should be [%d]",
             captionTimeStr, sequence_number, ((ctxPtr->lastSequence +1) % 4) );
    }
    ctxPtr->lastSequence = sequence_number;
    
    uint8* packet_data = &ctxPtr->dtvccPacket[1];
    uint8 packetDataIndex = 0;
    
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
    
    while( (packetDataIndex < packet_size) && ((packet_size - packetDataIndex) > 1) ) {               // packet_data_size = packet_size - 1
        uint8 service_number = (packet_data[packetDataIndex] & SERVICE_NUMBER_MASK) >> SERVICE_NUMBER_SHIFT;     // 3 more significant bits
        uint8 block_size = (packet_data[packetDataIndex] & SERVICE_BLOCK_SIZE_MASK);                             // 5 less significant bits
        
        if( GetMinDebugLevel(DBG_708_DEC) < DEBUG_LEVEL_INFO ) {
            char charArrayBig[256];
            char charArraySmall[10];
            sprintf(charArrayBig, "%02X - [ ", packet_data[packetDataIndex]);
            for( int loop = 0; loop < block_size; loop++ ) {
                sprintf(charArraySmall, "%02X ", packet_data[(packetDataIndex+1)+loop]);
                strcat(charArrayBig, charArraySmall);
            }
            LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "Processing Service Block - Srvc: %d Block Size: %d Pos: %d Packet Size: %d -> %s]",
                sequence_number, block_size, packetDataIndex, packet_size, charArrayBig );
        }
        
        if( block_size == 0 ) break;
        
        if( service_number == EXTENDED_SRV_NUM_PATTERN ) {
            ASSERT(block_size);
            packetDataIndex++;
            service_number = (packet_data[packetDataIndex] & EXTENDED_SRV_NUM_MASK);
            if( service_number < 7 ) {
                encodeTimeCode(captionTimePtr, captionTimeStr);
                LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "Illegal service number at %s in extended header: [%d]", captionTimeStr, service_number );
            }
        }
        packetDataIndex++; // Move to service data

        if( service_number == 0 && block_size != 0 ) { // Illegal, but specs say what to do...
            encodeTimeCode(captionTimePtr, captionTimeStr);
            LOG( DEBUG_LEVEL_WARN, DBG_708_DEC, "Data received at %s for service 0, skipping rest of packet.", captionTimeStr);
            packetDataIndex = packet_size; // Move to end
            break;
        }
        uint16 numDataPackets = countDataPackets( &packet_data[packetDataIndex], block_size );
        Buffer* outBuffer = NewBuffer(BUFFER_TYPE_DTVCC, numDataPackets);
        outBuffer->captionTime = *captionTimePtr;

        uint64 onService = 1;
        ctxPtr->activeServices = ctxPtr->activeServices | (onService << (service_number-1));

        processServiceBlock( ctxPtr, &packet_data[packetDataIndex], block_size, outBuffer, sequence_number, service_number );
        packetDataIndex = packetDataIndex + block_size; // Skip data

        if( rootCtxPtr->dtvccDecodeCtxPtr->processOnly == TRUE ) {
            FreeBuffer(outBuffer);
        } else {
            if( PassToSinks(rootCtxPtr, outBuffer, &ctxPtr->sinks ) == FALSE ) {
                ctxPtr->processedFine = FALSE;
            }
        }
    }
}  // dtvccProcessCurrentPacket()

/*------------------------------------------------------------------------------
 | NAME:
 |    countDataPackets()
 |
 | DESCRIPTION:
 |    This function counts the number of data packets in a given service block,
 |    which is done primarily for buffer size allocation.
 -------------------------------------------------------------------------------*/
static uint16 countDataPackets( uint8* dataPtr, uint8 block_size ) {
    uint8 index = 0;
    uint16 numDataPackets = 0;
    
    while( index < block_size ) {
        int8 used = LENGTH_UNKNOWN;
        
        if( dataPtr[index] != DTVCC_C0_EXT1 ) {
            if( (dataPtr[index] >= DTVCC_MIN_C0_CODE) && (dataPtr[index] <= DTVCC_MAX_C0_CODE) ) {
                numDataPackets = numDataPackets + 1;
                if( dataPtr[index] <= 0x0F ) used = 1;
                else if( dataPtr[index] <= 0x17 ) used = 2;
                else if( dataPtr[index] <= DTVCC_MAX_C0_CODE ) used = 3;
                else LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "Impossible Branch %02X", dataPtr[index] );
            } else if( (dataPtr[index] >= DTVCC_MIN_G0_CODE) && (dataPtr[index] <= DTVCC_MAX_G0_CODE) ) {
                numDataPackets = numDataPackets + 1;
                used = 1;
            } else if( (dataPtr[index] >= DTVCC_MIN_C1_CODE) && (dataPtr[index] <= DTVCC_MAX_C1_CODE) ) {
                numDataPackets = numDataPackets + 1;
                if( (dataPtr[index] == DTVCC_C1_CW0) || (dataPtr[index] == DTVCC_C1_CW1) || (dataPtr[index] == DTVCC_C1_CW2) ||
                    (dataPtr[index] == DTVCC_C1_CW3) || (dataPtr[index] == DTVCC_C1_CW4) || (dataPtr[index] == DTVCC_C1_CW5) ||
                    (dataPtr[index] == DTVCC_C1_CW6) || (dataPtr[index] == DTVCC_C1_CW7) || (dataPtr[index] == DTVCC_C1_DLC) ||
                    (dataPtr[index] == DTVCC_C1_RST) || (dataPtr[0] == DTVCC_C1_RSV93)   || (dataPtr[0] == DTVCC_C1_RSV94)   ||
                    (dataPtr[0] == DTVCC_C1_RSV95)   || (dataPtr[0] == DTVCC_C1_RSV96)) {
                    used = 1;
                } else if( (dataPtr[index] == DTVCC_C1_CLW) || (dataPtr[index] == DTVCC_C1_DSW) || (dataPtr[index] == DTVCC_C1_HDW) ||
                           (dataPtr[index] == DTVCC_C1_TGW) || (dataPtr[index] == DTVCC_C1_DLW) || (dataPtr[index] == DTVCC_C1_DLY) ) {
                    used = 2;
                } else if( (dataPtr[index] == DTVCC_C1_SPA) || (dataPtr[index] == DTVCC_C1_SPL) ) {
                    used = 3;
                } else if( dataPtr[index] == DTVCC_C1_SPC ) {
                    used = 4;
                } else if( dataPtr[index] == DTVCC_C1_SWA ) {
                    used = 5;
                } else if( (dataPtr[index] == DTVCC_C1_DF0) || (dataPtr[index] == DTVCC_C1_DF1) || (dataPtr[index] == DTVCC_C1_DF2) ||
                           (dataPtr[index] == DTVCC_C1_DF3) || (dataPtr[index] == DTVCC_C1_DF4) || (dataPtr[index] == DTVCC_C1_DF5) ||
                           (dataPtr[index] == DTVCC_C1_DF6) || (dataPtr[index] == DTVCC_C1_DF7) ) {
                    used = 7;
                } else {
                    LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "Impossible Branch: 0x%02X", dataPtr[0] );
                    used = 1;
                }
            } else {
                numDataPackets = numDataPackets + 1;
                used = 1;
            }
            if( used == LENGTH_UNKNOWN ) {
                LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "There was a problem handling the data. Reseting service decoder" );
                return 25;
            }
        } else {  // Use Extended Set
            if( (dataPtr[index+1] >= DTVCC_MIN_C0_CODE) && (dataPtr[index+1] <= DTVCC_MAX_C0_CODE) ) { // C2: Extended Misc. Control Codes
                numDataPackets = numDataPackets + 1;
                if( dataPtr[index+2] < 0x07 ) { // 00-07 : Single-byte control bytes (0 additional bytes)
                    used = 2;
                } else if( dataPtr[index+2] < 0x0F ) { // 08-0F : Two-byte control codes (1 additional byte)
                    used = 3;
                } else if( dataPtr[index+2] < 0x0F ) { // 10-17 : Three-byte control codes (2 additional bytes)
                    used = 4;
                } else {  // 18-1F : Four-byte control codes (3 additional bytes)
                    used = 5;
                }
                LOG( DEBUG_LEVEL_WARN, DBG_708_DEC, "Skipping C2 Code: 0x%02X", dataPtr[1] );
            } else if( (dataPtr[index+1] >= DTVCC_MIN_G0_CODE) && (dataPtr[index+1] <= DTVCC_MAX_G0_CODE) ) {  // G2: Extended Misc. Characters
                numDataPackets = numDataPackets + 1;
                used = 2;
            } else if( (dataPtr[index+1] >= DTVCC_MIN_C1_CODE) && (dataPtr[index+1] <= DTVCC_MAX_C1_CODE) ) {
                if( (dataPtr[index+2] < 0x80) || (dataPtr[index+2] > 0x9F) ) {
                    LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "Skipping Invalid C3 Cmd: 0x%02X", dataPtr[index+2] );
                } else if( dataPtr[index+2] <= 0x87 ) { // 80-87 : Five-byte control bytes (4 additional bytes)
                    numDataPackets = numDataPackets + 1;
                    used = 6;
                } else if( dataPtr[index+2] <= 0x8F ) { // 88-8F : Six-byte control codes (5 additional byte)
                    numDataPackets = numDataPackets + 1;
                    used = 7;
                } else {
                    // 90-9F : These are variable length commands, that can even span several segments.
                    // They were envisioned for things like downloading fonts and graphics.
                    // We are not supporting this set of data.
                    LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "Likely Data Corruption. Unsupported C3 Data Range: 0x%02X", dataPtr[index+2] );
                    return 25;
                }
            } else {  // G3 Character Set (Basically just the [CC] Symbol).
                numDataPackets = numDataPackets + 1;
                used = 2;
            }
        }
        index = index + used;
    }
    ASSERT(index == block_size);
    return numDataPackets;
}  // countDataPackets()

/*------------------------------------------------------------------------------
 | NAME:
 |    processServiceBlock()
 |
 | DESCRIPTION:
 |    This function processes the service blocks that were located in the
 |    packet.
 -------------------------------------------------------------------------------*/
static void processServiceBlock( DtvccDecodeCtx* ctxPtr, uint8* dataPtr, uint8 block_size,
                                 Buffer* outBuffer, uint8 seqNum, uint8 srvcNum ) {
    uint8 index = 0;
    DtvccData* dtvccDataPtr = NULL;
    DtvccData* dtvccBufferDataPtr = (DtvccData*)outBuffer->dataPtr;
    ASSERT(outBuffer->numElements == 0);
    
    while( index < block_size ) {
        int8 used = LENGTH_UNKNOWN;
        
        ASSERT(outBuffer->numElements < outBuffer->maxNumElements);
        
        dtvccDataPtr = &(dtvccBufferDataPtr[outBuffer->numElements]);
        dtvccDataPtr->dtvccType = DTVCC_UNKNOWN;
        dtvccDataPtr->sequenceNumber = seqNum;
        dtvccDataPtr->serviceNumber = srvcNum;
        outBuffer->numElements = outBuffer->numElements + 1;
        
        if( dataPtr[index] != DTVCC_C0_EXT1 ) {
            if( (dataPtr[index] >= DTVCC_MIN_C0_CODE) && (dataPtr[index] <= DTVCC_MAX_C0_CODE) ) {
                dtvccDataPtr->dtvccType = DTVCC_C0_CMD;
                used = parseC0CmdCode( ctxPtr, &dataPtr[index], (block_size - index), dtvccDataPtr, dtvccDataPtr->serviceNumber );
            } else if( (dataPtr[index] >= DTVCC_MIN_G0_CODE) && (dataPtr[index] <= DTVCC_MAX_G0_CODE) ) {
                dtvccDataPtr->dtvccType = DTVCC_G0_CHAR;
                dtvccDataPtr->data.g0char = dataPtr[index];
                LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "G0: [%02X] '%s'", dataPtr[index], DtvccDecodeG0CharSet(dtvccDataPtr->data.g0char) );
                used = 1;
            } else if( (dataPtr[index] >= DTVCC_MIN_C1_CODE) && (dataPtr[index] <= DTVCC_MAX_C1_CODE) ) {
                dtvccDataPtr->dtvccType = DTVCC_C1_CMD;
                used = parseC1CmdCode( ctxPtr, &dataPtr[index], (block_size - index), dtvccDataPtr, dtvccDataPtr->serviceNumber, &outBuffer->captionTime );
            } else {
                dtvccDataPtr->dtvccType = DTVCC_G1_CHAR;
                dtvccDataPtr->data.g1char = dataPtr[index];
                LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "G1: [%02X] '%s'", dataPtr[index], DtvccDecodeG1CharSet(dtvccDataPtr->data.g0char) );
                used = 1;
                if( ctxPtr->foundText == NO_TEXT_FOUND ) {
                    ctxPtr->foundText = TEXT_FOUND;
                }
            }
            if( used == LENGTH_UNKNOWN ) {
                LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "There was a problem handling the data. Reseting service decoder" );
                return;
            }
        } else {  // Use Extended Set
            if( (dataPtr[index+1] >= DTVCC_MIN_C0_CODE) && (dataPtr[index+1] <= DTVCC_MAX_C0_CODE) ) { // C2: Extended Misc. Control Codes
                dtvccDataPtr->dtvccType = DTVCC_C2_CMD;
                /* This section is for future codes. While by definition we can't do any work on them, we must advance */
                /* however many bytes would be consumed if these codes were supported, as defined in the specs.        */
                // WARN: This code is completely untested due to lack of samples. Just following specs!
                if( dataPtr[index+2] < 0x07 ) { // 00-07 : Single-byte control bytes (0 additional bytes)
                    used = 2;
                } else if( dataPtr[index+2] <0x0F ) { // 08-0F : Two-byte control codes (1 additional byte)
                    used = 3;
                } else if( dataPtr[index+2] <0x0F ) { // 10-17 : Three-byte control codes (2 additional bytes)
                    used = 4;
                } else {  // 18-1F : Four-byte control codes (3 additional bytes)
                    used = 5;
                }
                LOG( DEBUG_LEVEL_WARN, DBG_708_DEC, "Skipping C2 Code: 0x%02X", dataPtr[1] );
            } else if( (dataPtr[index+1] >= DTVCC_MIN_G0_CODE) && (dataPtr[index+1] <= DTVCC_MAX_G0_CODE) ) {  // G2: Extended Misc. Characters
                dtvccDataPtr->dtvccType = DTVCC_G2_CHAR;
                if( (dataPtr[index+2] == 0x20) || (dataPtr[index+2] == 0x21) || (dataPtr[index+2] == 0x25) || (dataPtr[index+2] == 0x2A) ||
                    (dataPtr[index+2] == 0x2C) || ((dataPtr[index+2] >= 0x30) && (dataPtr[index+2] <= 0x35)) || (dataPtr[index+2] == 0x39) ||
                    (dataPtr[index+2] == 0x3A) || (dataPtr[index+2] == 0x3C) || (dataPtr[index+2] == 0x3D) || (dataPtr[index+2] == 0x3F) ||
                    ((dataPtr[index+2] >= 0x76) && (dataPtr[index+2] <= 0x7F)) ) {
                    dtvccDataPtr->data.g2char = dataPtr[index+2];
                    LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "G2: '%s'", DtvccDecodeG2CharSet(dtvccDataPtr->data.g2char) );
                    if( ctxPtr->foundText == NO_TEXT_FOUND ) {
                        ctxPtr->foundText = TEXT_FOUND;
                    }
                } else {
                    LOG( DEBUG_LEVEL_WARN, DBG_708_DEC, "Skipping Unknown G2 Char: 0x%02X", dataPtr[index+2] );
                    dtvccDataPtr->data.g2char = DTVCC_UNKNOWN_G2_CHAR;
                }
                used = 2;
            } else if( (dataPtr[index+1] >= DTVCC_MIN_C1_CODE) && (dataPtr[index+1] <= DTVCC_MAX_C1_CODE) ) {
                dtvccDataPtr->dtvccType = DTVCC_C3_CMD;
                if( (dataPtr[index+2] < 0x80) || (dataPtr[index+2] > 0x9F) ) {
                    LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "Skipping Invalid C3 Cmd: 0x%02X", dataPtr[index+2] );
                } else if( dataPtr[index+2] <= 0x87 ) { // 80-87 : Five-byte control bytes (4 additional bytes)
                    used = 6;
                } else if( dataPtr[index+2] <= 0x8F ) { // 88-8F : Six-byte control codes (5 additional byte)
                    used = 7;
                } else {
                    // 90-9F : These are variable length commands, that can even span several segments.
                    // They were envisioned for things like downloading fonts and graphics.
                    // We are not supporting this set of data.
                    LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "Likely Data Corruption. Unsupported C3 Data Range: 0x%02X", dataPtr[index+2] );
                }
            } else {  // G3 Character Set (Basically just the [CC] Symbol).
                dtvccDataPtr->dtvccType = DTVCC_G3_CHAR;
                if( dataPtr[index+2] != DTVCC_G3_CC_ICON ) {
                    dtvccDataPtr->data.g3char = dataPtr[index+2];
                    LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "G3: '%s'", DtvccDecodeG2CharSet(dtvccDataPtr->data.g3char) );
                    if( ctxPtr->foundText == NO_TEXT_FOUND ) {
                        ctxPtr->foundText = TEXT_FOUND;
                    }
                } else {
                    LOG( DEBUG_LEVEL_WARN, DBG_708_DEC, "Skipping Unknown G3 Char: 0x%02X", dataPtr[index+2] );
                    dtvccDataPtr->data.g3char = DTVCC_UNKNOWN_G3_CHAR;
                }
                used = 2;
            }
        }
        
        if( dtvccDataPtr->dtvccType == DTVCC_UNKNOWN ) {
            LOG( DEBUG_LEVEL_WARN, DBG_708_DEC, "Unexpectedly Skipping a buffer element" );
            outBuffer->numElements = outBuffer->numElements - 1;
        }
        
        index = index + used;
    }
    ASSERT(index == block_size);
    ASSERT(outBuffer->numElements == outBuffer->maxNumElements);
}  // processServiceBlock()

/*------------------------------------------------------------------------------
 | NAME:
 |    parseC0CmdCode()
 |
 | DESCRIPTION:
 |    This function decodes C0 Commands, per the spec.
 -------------------------------------------------------------------------------*/
static int8 parseC0CmdCode( DtvccDecodeCtx* ctxPtr, uint8* dataPtr, uint8 blockSize, DtvccData* dtvccDataPtr, uint8 service ) {
    int8 len = LENGTH_UNKNOWN;
    C0Command* c0CmdPtr = &dtvccDataPtr->data.c0cmd;
    
    // These commands have a known length even if they are reserved.
    if( dataPtr[0] <= 0x0F ) {
        if( (dataPtr[0] == DTVCC_C0_NUL) || (dataPtr[0] == DTVCC_C0_ETX) || (dataPtr[0] == DTVCC_C0_BS) ||
            (dataPtr[0] == DTVCC_C0_FF) || (dataPtr[0] == DTVCC_C0_CR) || (dataPtr[0] == DTVCC_C0_HCR) ) {
            c0CmdPtr->c0CmdCode = dataPtr[0];
            LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "C0: [%02X] {%s} Srvc: %d", dataPtr[0], DtvccDecodeC0Cmd(c0CmdPtr->c0CmdCode), service);
        } else {
            LOG( DEBUG_LEVEL_WARN, DBG_708_DEC, "Ignoring Unknown Command: 0x%02X Srvc: %d", dataPtr[0], service );
            dtvccDataPtr->dtvccType = DTVCC_IGNORE_CMD_OR_CHAR;
        }
        len = 1;
    } else if( dataPtr[0] <= 0x17 ) {
        // Note that 0x10 is actually EXT1 and is dealt with somewhere else. Rest is undefined as per CEA-708-D
        LOG( DEBUG_LEVEL_WARN, DBG_708_DEC, "Ignoring Unknown Command: 0x%02X Srvc: %d", dataPtr[0], service );
        dtvccDataPtr->dtvccType = DTVCC_IGNORE_CMD_OR_CHAR;
        len = 2;
    } else if( dataPtr[0] <= DTVCC_MAX_C0_CODE ) {
        if( dataPtr[0] == DTVCC_C0_P16 ) { // PE16
            c0CmdPtr->c0CmdCode = dataPtr[0];
            c0CmdPtr->pe16sym1 = dataPtr[1];
            c0CmdPtr->pe16sym2 = dataPtr[2];
            if( ctxPtr->numP16Cmds < 5 ) {
                LOG(DEBUG_LEVEL_WARN, DBG_708_DEC, "Found a P16 Command: 0x%02X%02X Srvc: %d", c0CmdPtr->pe16sym1,
                    c0CmdPtr->pe16sym2, service);
            } else if( ctxPtr->numP16Cmds == 5 ) {
                LOG(DEBUG_LEVEL_WARN, DBG_708_DEC, "Found a P16 Command: 0x%02X%02X Srvc: %d --- Suppressing Subsequent Warnings.", c0CmdPtr->pe16sym1,
                    c0CmdPtr->pe16sym2, service);
            } else {
                LOG(DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "Found a P16 Command: 0x%02X%02X Srvc: %d", c0CmdPtr->pe16sym1,
                    c0CmdPtr->pe16sym2, service);
            }
            ctxPtr->numP16Cmds++;
        } else {
            LOG( DEBUG_LEVEL_WARN, DBG_708_DEC, "Ignoring Unknown Command: 0x%02X Srvc: %d", dataPtr[0], service );
            dtvccDataPtr->dtvccType = DTVCC_IGNORE_CMD_OR_CHAR;
        }
        len = 3;
    }
    
    if( len == LENGTH_UNKNOWN ) {
        LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "Command is unknown: 0x%02X Srvc: %d", dataPtr[0], service );
        return LENGTH_UNKNOWN;
    }
    
    if( len > blockSize ) {
        LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "Command is %d bytes long but we only have %d Srvc: %d", len, blockSize, service );
        return LENGTH_UNKNOWN;
    }
    
    return len;
}  // parseC0CmdCode()

/*------------------------------------------------------------------------------
 | NAME:
 |    parseC1CmdCode()
 |
 | DESCRIPTION:
 |    This function decodeds C1 Commands, per the spec.
 -------------------------------------------------------------------------------*/
static int8 parseC1CmdCode( DtvccDecodeCtx* ctxPtr, uint8* dataPtr, uint8 blockSize, DtvccData* dtvccDataPtr, uint8 service, CaptionTime* captionTimePtr ) {
    ASSERT(service < 64);
    int8 len = LENGTH_UNKNOWN;
    C1Command* c1CmdPtr = &dtvccDataPtr->data.c1cmd;
    char captionTimeStr[CAPTION_TIME_SCRATCH_BUFFER_SIZE];
    
    // --- Set Current Window {CWx} --- DelayCancel {DLC} --- Reset {RST} ---
    if( (dataPtr[0] == DTVCC_C1_CW0) || (dataPtr[0] == DTVCC_C1_CW1) || (dataPtr[0] == DTVCC_C1_CW2) ||
        (dataPtr[0] == DTVCC_C1_CW3) || (dataPtr[0] == DTVCC_C1_CW4) || (dataPtr[0] == DTVCC_C1_CW5) ||
        (dataPtr[0] == DTVCC_C1_CW6) || (dataPtr[0] == DTVCC_C1_CW7) || (dataPtr[0] == DTVCC_C1_DLC) ||
        (dataPtr[0] == DTVCC_C1_RST) ) {
        c1CmdPtr->c1CmdCode = dataPtr[0];
        LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "C1: [%02X] {%s}", dataPtr[0], DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode));
        len = 1;
    // --- Clear Windows {CLW} --- Display Windows {DSW} --- HideWindows {HDW} ---
    // --- ToggleWindows {TGW} --- Delete Windows {DLW} ---
    } else if( (dataPtr[0] == DTVCC_C1_CLW) || (dataPtr[0] == DTVCC_C1_DSW) || (dataPtr[0] == DTVCC_C1_HDW) ||
               (dataPtr[0] == DTVCC_C1_TGW) || (dataPtr[0] == DTVCC_C1_DLW) ) {
        if( (((dataPtr[0] == DTVCC_C1_TGW) || (dataPtr[0] == DTVCC_C1_DSW)) && (dataPtr[1] != 0)) && (ctxPtr->captioningStart[service] == FALSE) ) {
            ctxPtr->captioningStart[service] = TRUE;
            encodeTimeCode(captionTimePtr, captionTimeStr);
            LOG(DEBUG_LEVEL_INFO, DBG_608_DEC, "DTVCC Captioning on Service %d started at %s", service, captionTimeStr);
        }
        c1CmdPtr->c1CmdCode = dataPtr[0];
        c1CmdPtr->cmdData.windowBitmap = dataPtr[1];
        LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "C1: [%02X %02X] {%s:%s}", dataPtr[0], dataPtr[1],
             DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode), uint8toBitArray(c1CmdPtr->cmdData.windowBitmap));
        len = 2;
    // --- Delay {DLY} in 100ms ---
    } else if( dataPtr[0] == DTVCC_C1_DLY ) {
        c1CmdPtr->c1CmdCode = dataPtr[0];
        c1CmdPtr->cmdData.tenthsOfaSec = dataPtr[1];
        LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "C1: [%02X %02X] {%s:%d00ms}", dataPtr[0], dataPtr[1],
             DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode), c1CmdPtr->cmdData.tenthsOfaSec);
        len = 2;
    // --- Set Pen Attributes {SPA} ---
    } else if( dataPtr[0] == DTVCC_C1_SPA ) {
        c1CmdPtr->c1CmdCode = dataPtr[0];
        c1CmdPtr->cmdData.penAttributes.penSize = dataPtr[1] & PEN_SIZE_MASK;
        c1CmdPtr->cmdData.penAttributes.penOffset = (dataPtr[1] & PEN_OFFSET_MASK) >> PEN_OFFSET_NORM_SHIFT;
        c1CmdPtr->cmdData.penAttributes.textTag = (dataPtr[1] & TEXT_TAG_MASK) >> TEST_TAG_NORM_SHIFT;
        c1CmdPtr->cmdData.penAttributes.fontTag = (dataPtr[2] & FONT_TAG_MASK);
        c1CmdPtr->cmdData.penAttributes.edgeType = (dataPtr[2] & EDGE_TYPE_MASK) >> EDGE_TYPE_NORM_SHIFT;
        c1CmdPtr->cmdData.penAttributes.isUnderlined = ((dataPtr[2] & PA_UNDERLINE_TOGGLE_MASK) == PA_UNDERLINE_TOGGLE_SET);
        c1CmdPtr->cmdData.penAttributes.isItalic = ((dataPtr[2] & PA_ITALIC_TOGGLE_MASK) == PA_ITALIC_TOGGLE_SET);
        LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "C1: [%02X %02X %02X] {%s:Pen-[Size:%s,Offset:%s]:TextTag-%s:FontTag-%s:EdgeType-%s:Underline-%s:Italics-%s}",
            dataPtr[0], dataPtr[1], dataPtr[2],
            DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode),
            DECODE_PEN_SIZE(c1CmdPtr->cmdData.penAttributes.penSize),
            DECODE_PEN_OFFSET(c1CmdPtr->cmdData.penAttributes.penOffset),
            DECODE_TEXT_TAG(c1CmdPtr->cmdData.penAttributes.textTag),
            DECODE_FONT_TAG(c1CmdPtr->cmdData.penAttributes.fontTag),
            DECODE_EDGE_TYPE(c1CmdPtr->cmdData.penAttributes.edgeType),
            trueFalseStr[c1CmdPtr->cmdData.penAttributes.isUnderlined],
            trueFalseStr[c1CmdPtr->cmdData.penAttributes.isItalic]);
        len = 3;
    // --- Set Pen Color {SPC} ---
    } else if( dataPtr[0] == DTVCC_C1_SPC ) {
        c1CmdPtr->c1CmdCode = dataPtr[0];
        c1CmdPtr->cmdData.penColor.fgOpacity = (dataPtr[1] & FORGROUND_OPACITY_MASK) >> FORGROUND_OPACITY_NORM_SHIFT;
        c1CmdPtr->cmdData.penColor.fgRed = (dataPtr[1] & FORGROUND_RED_COLOR_CMP_MASK) >> FORGROUND_RED_COLOR_CMP_NORM_SHIFT;
        c1CmdPtr->cmdData.penColor.fgGreen = (dataPtr[1] & FORGROUND_GREEN_COLOR_CMP_MASK) >> FORGROUND_GREEN_COLOR_CMP_NORM_SHIFT;
        c1CmdPtr->cmdData.penColor.fgBlue = dataPtr[1] & FORGROUND_BLUE_COLOR_CMP_MASK;
        c1CmdPtr->cmdData.penColor.bgOpacity = (dataPtr[2] & BACKGROUND_OPACITY_MASK) >> BACKGROUND_OPACITY_NORM_SHIFT;
        c1CmdPtr->cmdData.penColor.bgRed = (dataPtr[2] & BACKGROUND_RED_COLOR_CMP_MASK) >> BACKGROUND_RED_COLOR_CMP_NORM_SHIFT;
        c1CmdPtr->cmdData.penColor.bgGreen = (dataPtr[2] & BACKGROUND_GREEN_COLOR_CMP_MASK) >> BACKGROUND_GREEN_COLOR_CMP_NORM_SHIFT;
        c1CmdPtr->cmdData.penColor.bgBlue = dataPtr[2] & BACKGROUND_BLUE_COLOR_CMP_MASK;
        c1CmdPtr->cmdData.penColor.edgeRed = (dataPtr[3] & EDGE_RED_COLOR_CMP_MASK) >> EDGE_RED_COLOR_CMP_NORM_SHIFT;
        c1CmdPtr->cmdData.penColor.edgeGreen = (dataPtr[3] & EDGE_GREEN_COLOR_CMP_MASK) >> EDGE_GREEN_COLOR_CMP_NORM_SHIFT;
        c1CmdPtr->cmdData.penColor.edgeBlue = dataPtr[3] & EDGE_BLUE_COLOR_CMP_MASK;
        LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "C1: [%02X %02X %02X %02X] {%s:FG-%s-R%dG%dB%d:BG-%s-R%dG%dB%d:Edge-R%dG%dB%d}",
            dataPtr[0], dataPtr[1], dataPtr[2], dataPtr[3],
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
        len = 4;
    // --- Set Pen Location {SPL} ---
    } else if( dataPtr[0] == DTVCC_C1_SPL ) {
        c1CmdPtr->c1CmdCode = dataPtr[0];
        c1CmdPtr->cmdData.penLocation.row = dataPtr[1] & PEN_LOC_ROW_MASK;
        c1CmdPtr->cmdData.penLocation.column = dataPtr[2] & PEN_LOC_COLUMN_MASK;
        LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "C1: [%02X %02X %02X] {%s:R%d-C%d}",
            dataPtr[0], dataPtr[1], dataPtr[2], DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode),
            c1CmdPtr->cmdData.penLocation.row, c1CmdPtr->cmdData.penLocation.column);
        len = 3;
    // --- Reserved {RSVxx} ---
    } else if( (dataPtr[0] == DTVCC_C1_RSV93) || (dataPtr[0] == DTVCC_C1_RSV94) || (dataPtr[0] == DTVCC_C1_RSV95) ||
               (dataPtr[0] == DTVCC_C1_RSV96) ) {
        c1CmdPtr->c1CmdCode = dataPtr[0];
        LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "C1: [%02X] {%s}", dataPtr[0], DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode) );
        LOG( DEBUG_LEVEL_WARN, DBG_708_DEC, "Found Reserved codes, ignored." );
        len = 1;
    // --- Set Window Attributes {SWA} ---
    } else if( dataPtr[0] == DTVCC_C1_SWA ) {
        c1CmdPtr->c1CmdCode = dataPtr[0];
        c1CmdPtr->cmdData.winAttributes.fillOpacity = (dataPtr[1] & FILL_OPACITY_MASK) >> FILL_OPACITY_NORM_SHIFT;
        c1CmdPtr->cmdData.winAttributes.fillRedColorComp = (dataPtr[1] & FILL_RED_COLOR_COMP_MASK) >> FILL_RED_COLOR_COMP_SHIFT;
        c1CmdPtr->cmdData.winAttributes.fillGreenColorComp = (dataPtr[1] & FILL_GREEN_COLOR_COMP_MASK) >> FILL_GREEN_COLOR_COMP_SHIFT;
        c1CmdPtr->cmdData.winAttributes.fillBlueColorComp = dataPtr[1] & FILL_BLUE_COLOR_COMP_MASK;
        c1CmdPtr->cmdData.winAttributes.borderType = ((dataPtr[3] & BORDER_TYPE_HIGH_BIT_MASK) >> BORDER_TYPE_HB_NORM_SHIFT) ||
                                                     ((dataPtr[2] & BORDER_TYPE_LOWER_BITS_MASK) >> BORDER_TYPE_LB_NORM_SHIFT);
        c1CmdPtr->cmdData.winAttributes.borderRed = (dataPtr[2] & BORDER_RED_COLOR_COMP_MASK) >> BORDER_RED_COLOR_COMP_SHIFT;
        c1CmdPtr->cmdData.winAttributes.borderGreen = (dataPtr[2] & BORDER_GREEN_COLOR_COMP_MASK) >> BORDER_GREEN_COLOR_COMP_SHIFT;
        c1CmdPtr->cmdData.winAttributes.borderBlue = dataPtr[2] & BORDER_BLUE_COLOR_COMP_MASK;
        c1CmdPtr->cmdData.winAttributes.isWordWrapped = ((dataPtr[3] & WORD_WRAP_TOGGLE_MASK) == WORD_WRAP_TOGGLE_SET);
        c1CmdPtr->cmdData.winAttributes.printDirection = (dataPtr[3] & PRINT_DIRECTION_MASK) >> PRINT_DIRECTION_NORM_SHIFT;
        c1CmdPtr->cmdData.winAttributes.scrollDirection = (dataPtr[3] & SCROLL_DIRECTION_MASK) >> SCROLL_DIRECTION_NORM_SHIFT;
        c1CmdPtr->cmdData.winAttributes.justifyDirection = dataPtr[3] & JUSTIFY_DIRECTION_MASK;
        c1CmdPtr->cmdData.winAttributes.effectSpeed = (dataPtr[4] & EFFECT_SPEED_MASK) >> EFFECT_SPEED_NORM_SHIFT;
        c1CmdPtr->cmdData.winAttributes.effectDirection = (dataPtr[4] & EFFECT_DIRECTION_MASK) >> EFFECT_DIRECTION_NORM_SHIFT;
        c1CmdPtr->cmdData.winAttributes.displayEffect = dataPtr[4] & DISPLAY_EFFECT_MASK;
        LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "C1: [%02X %02X %02X %02X %02X] {%s:Fill-%s-R%dG%dB%d:Brdr-%s-R%dG%dB%d:PD-%s:SD-%s:JD-%s:%s-%ssec-%s:WordWrap-%s}",
            dataPtr[0], dataPtr[1], dataPtr[2], dataPtr[3], dataPtr[4],
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
            DECODE_DIRECTION(c1CmdPtr->cmdData.winAttributes.effectDirection),
            trueFalseStr[c1CmdPtr->cmdData.winAttributes.isWordWrapped]);
        len = 5;
    // --- Define Window {DFWx} ---
    } else if( (dataPtr[0] == DTVCC_C1_DF0) || (dataPtr[0] == DTVCC_C1_DF1) || (dataPtr[0] == DTVCC_C1_DF2) ||
               (dataPtr[0] == DTVCC_C1_DF3) || (dataPtr[0] == DTVCC_C1_DF4) || (dataPtr[0] == DTVCC_C1_DF5) ||
               (dataPtr[0] == DTVCC_C1_DF6) || (dataPtr[0] == DTVCC_C1_DF7) ) {
        c1CmdPtr->c1CmdCode = dataPtr[0];
        c1CmdPtr->cmdData.winDefinition.isVisible = ((dataPtr[1] & VISIBLE_TOGGLE_MASK) == VISIBLITY_SET);
        c1CmdPtr->cmdData.winDefinition.isRowLocked = ((dataPtr[1] & ROW_LOCK_TOGGLE_MASK) == ROW_LOCK_TOGGLE_SET);
        c1CmdPtr->cmdData.winDefinition.isColumnLocked = ((dataPtr[1] & COLUMN_LOCK_TOGGLE_MASK) == COLUMN_LOCK_TOGGLE_SET);
        c1CmdPtr->cmdData.winDefinition.priority = dataPtr[1] & PRIORITY_MASK;
        c1CmdPtr->cmdData.winDefinition.isRelativePosition = ((dataPtr[2] & RELATIVE_POSITIONING_MASK) == RELATIVE_POSITIONING_SET);
        c1CmdPtr->cmdData.winDefinition.verticalAnchor = dataPtr[2] & VERTICAL_ANCHOR_MASK;
        c1CmdPtr->cmdData.winDefinition.horizontalAnchor = dataPtr[3];
        c1CmdPtr->cmdData.winDefinition.anchorId = (dataPtr[4] & ANCHOR_ID_MASK) >> ANCHOR_ID_NORM_SHIFT;
        c1CmdPtr->cmdData.winDefinition.rowCount = dataPtr[4] & ROW_COUNT_MASK;
        c1CmdPtr->cmdData.winDefinition.columnCount = dataPtr[5] & COLUMN_COUNT_MASK;
        c1CmdPtr->cmdData.winDefinition.windowStyle = (dataPtr[6] & WINDOW_STYLE_MASK) >> WINDOW_STYLE_NORM_SHIFT;
        c1CmdPtr->cmdData.winDefinition.penStyle = dataPtr[6] & PEN_STYLE_MASK;
        LOG( DEBUG_LEVEL_VERBOSE, DBG_708_DEC, "C1 [%02X %02X %02X %02X %02X %02X %02X] {%s:%s:R%d-C:%d:Anchor-%s-V%d-H%d:Pen-%s:Pr-%d:Visible-%s:RowLocked-%s:ColumnLocked-%s:RelativePos-%s}",
            dataPtr[0], dataPtr[1], dataPtr[2], dataPtr[3], dataPtr[4], dataPtr[5], dataPtr[6],
            DtvccDecodeC1Cmd(c1CmdPtr->c1CmdCode),
            DECODE_PREDEF_WIN_STYLE(c1CmdPtr->cmdData.winDefinition.windowStyle),
            c1CmdPtr->cmdData.winDefinition.rowCount,
            c1CmdPtr->cmdData.winDefinition.columnCount,
            DECODE_ANCOR_POSITION(c1CmdPtr->cmdData.winDefinition.anchorId),
            c1CmdPtr->cmdData.winDefinition.verticalAnchor,
            c1CmdPtr->cmdData.winDefinition.horizontalAnchor,
            DECODE_PREDEF_PEN_STYLE(c1CmdPtr->cmdData.winDefinition.penStyle),
            c1CmdPtr->cmdData.winDefinition.priority,
            trueFalseStr[c1CmdPtr->cmdData.winDefinition.isVisible],
            trueFalseStr[c1CmdPtr->cmdData.winDefinition.isRowLocked],
            trueFalseStr[c1CmdPtr->cmdData.winDefinition.isColumnLocked],
            trueFalseStr[c1CmdPtr->cmdData.winDefinition.isRelativePosition]);
        len = 7;

        if( ctxPtr->captioningChange[service-1] == FALSE ) {
            if( (c1CmdPtr->cmdData.winDefinition.windowStyle == WINDOW_STYLE_608_POPUP) ||
                (c1CmdPtr->cmdData.winDefinition.windowStyle == WINDOW_STYLE_POPUP_TRANS_BG) ||
                (c1CmdPtr->cmdData.winDefinition.windowStyle == WINDOW_STYLE_POPUP_CENTERED) ) {
                ctxPtr->isPopOnCaptioning[service-1] = TRUE;
                if( ctxPtr->isRollUpCaptioning[service-1] == TRUE ) {
                    LOG(DEBUG_LEVEL_WARN, DBG_708_DEC, "Caption Format change on Service %d: RollUp -> PopUp", service-1);
                    ctxPtr->captioningChange[service-1] = TRUE;
                }
                if( ctxPtr->isTickerCaptioning[service-1] == TRUE ) {
                    LOG(DEBUG_LEVEL_WARN, DBG_708_DEC, "Caption Format change on Service %d: TickerTape -> PopUp", service-1);
                    ctxPtr->captioningChange[service-1] = TRUE;
                }
            } else if( (c1CmdPtr->cmdData.winDefinition.windowStyle == WINDOW_STYLE_608_ROLLUP) ||
                       (c1CmdPtr->cmdData.winDefinition.windowStyle == WINDOW_STYLE_ROLLUP_TRANS_BG) ||
                       (c1CmdPtr->cmdData.winDefinition.windowStyle == WINDOW_STYLE_ROLLUP_CENTERED) ) {
                ctxPtr->isRollUpCaptioning[service-1] = TRUE;
                if( ctxPtr->isPopOnCaptioning[service-1] == TRUE ) {
                    LOG(DEBUG_LEVEL_WARN, DBG_708_DEC, "Caption Format change on Service %d: PopUp -> RollUp", service-1);
                    ctxPtr->captioningChange[service-1] = TRUE;
                }
                if( ctxPtr->isTickerCaptioning[service-1] == TRUE ) {
                    LOG(DEBUG_LEVEL_WARN, DBG_708_DEC, "Caption Format change on Service %d: TickerTape -> RollUp", service-1);
                    ctxPtr->captioningChange[service-1] = TRUE;
                }
            } else if( c1CmdPtr->cmdData.winDefinition.windowStyle == WINDOW_STYLE_TICKER_TAPE ) {
                ctxPtr->isTickerCaptioning[service-1] = TRUE;
                if( ctxPtr->isPopOnCaptioning[service-1] == TRUE ) {
                    LOG(DEBUG_LEVEL_WARN, DBG_708_DEC, "Caption Format change on Service %d: PopUp -> TickerTape", service-1);
                    ctxPtr->captioningChange[service-1] = TRUE;
                }
                if( ctxPtr->isRollUpCaptioning[service-1] == TRUE ) {
                    LOG(DEBUG_LEVEL_WARN, DBG_708_DEC, "Caption Format change on Service %d: RollUp -> TickerTape", service-1);
                    ctxPtr->captioningChange[service-1] = TRUE;
                }
            } else {
                LOG(DEBUG_LEVEL_ERROR, DBG_708_DEC, "Unexpected Window Style on Service %d: %d", c1CmdPtr->cmdData.winDefinition.windowStyle, service-1);
            }
        }
    } else {
        LOG( DEBUG_LEVEL_ERROR, DBG_708_DEC, "Impossible Branch: 0x%02X", dataPtr[0] );
        dtvccDataPtr->dtvccType = DTVCC_IGNORE_CMD_OR_CHAR;
        len = 1;
    }
    return len;
}  // parseC1CmdCode()
