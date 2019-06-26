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
#include <string.h>
#include <stdlib.h>

#include "mcc_decode.h"
#include "cc_utils.h"

#include "debug.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

static Buffer* expandMccLine( Buffer* );
static Buffer* decodeMccLine( Buffer* );
static uint8 mccCharCount( char );
static void addMultipleFaZeroZero( uint8, uint8*, uint16* );
static void expandMccCode( uint8, uint8*, uint16* );
static void decodeFrameRate( cdp_header, CaptionTime* );

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    MccDecodeInitialize()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one
 |
 | RETURN VALUES:
 |    LinkInfo -  Information about this element of the pipeline, such that it can
 |                be chained to elements that can consume the specific type of data
 |                that it produces.
 |
 | DESCRIPTION:
 |    This initializes this element of the pipeline.
 -------------------------------------------------------------------------------*/
LinkInfo MccDecodeInitialize( Context* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(!rootCtxPtr->mccDecodeCtxPtr);

    rootCtxPtr->mccDecodeCtxPtr = malloc(sizeof(MccDecodeCtx));

    InitSinks(&rootCtxPtr->mccDecodeCtxPtr->sinks, MCC_DATA___CC_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = MCC_DATA___CC_DATA;
    linkInfo.sourceType = DATA_TYPE_MCC_DATA;
    linkInfo.sinkType = DATA_TYPE_CC_DATA;
    linkInfo.NextBufferFnPtr = &MccDecodeProcNextBuffer;
    linkInfo.ShutdownFnPtr = &MccDecodeShutdown;
    return linkInfo;
}  // MccDecodeInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    MccDecodeAddSink()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one
 |    linkInfo - Describes the potential next element of the pipeline.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method binds the next element to this element, after validating that
 |    the data it receives is compatible with the data tht this element sends.
 -------------------------------------------------------------------------------*/
boolean MccDecodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    ASSERT(rootCtxPtr);
    ASSERT(rootCtxPtr->mccDecodeCtxPtr);

    if( linkInfo.sourceType != DATA_TYPE_CC_DATA ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Attempt to add Sink that cannot process CC Data");
        return FALSE;
    }
    
    return AddSink(&rootCtxPtr->mccDecodeCtxPtr->sinks, &linkInfo);
}  // MccDecodeAddSink()

/*------------------------------------------------------------------------------
 | NAME:
 |    MccDecodeProcNextBuffer()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one
 |    inBuffer - Pointer to the buffer to process.
 |
 | RETURN VALUES:
 |    boolean - Success is TRUE and Failure is FALSE
 |
 | DESCRIPTION:
 |    This method processes an incoming buffer, expanding and the decoding the
 |    mcc data inside of it.
 -------------------------------------------------------------------------------*/
boolean MccDecodeProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    ASSERT(inBuffer);
    ASSERT(inBuffer->dataPtr);
    ASSERT(inBuffer->numElements);
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->mccDecodeCtxPtr);
    
    Buffer* expandedBuffer = expandMccLine( inBuffer );
    Buffer* decodedBuffer = decodeMccLine( expandedBuffer );
    
    return PassToSinks(rootCtxPtr, decodedBuffer, &((Context*)rootCtxPtr)->mccDecodeCtxPtr->sinks);
}  // MccDecodeProcNextBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    MccDecodeShutdown()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Pointer to all Pipeline Elements Contexts, including this one
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
boolean MccDecodeShutdown( void* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->mccDecodeCtxPtr);
    Sinks sinks = ((Context*)rootCtxPtr)->mccDecodeCtxPtr->sinks;

    free(((Context*)rootCtxPtr)->mccDecodeCtxPtr);
    ((Context*)rootCtxPtr)->mccDecodeCtxPtr = NULL;

    return ShutdownSinks(rootCtxPtr, &sinks);
}  // MccDecodeShutdown()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    expandMccLine()
 |
 | DESCRIPTION:
 |    This method uses the Telestream specified character expansion.
 |
 |    ANC data bytes may be represented by one ASCII character according
 |    to the following schema:
 |      G  FAh 00h 00h
 |      H  2 x (FAh 00h 00h)
 |      I  3 x (FAh 00h 00h)
 |      J  4 x (FAh 00h 00h)
 |      K  5 x (FAh 00h 00h)
 |      L  6 x (FAh 00h 00h)
 |      M  7 x (FAh 00h 00h)
 |      N  8 x (FAh 00h 00h)
 |      O  9 x (FAh 00h 00h)
 |      P  FBh 80h 80h
 |      Q  FCh 80h 80h
 |      R  FDh 80h 80h
 |      S  96h 69h
 |      T  61h 01h
 |      U  E1h 00h 00h 00h
 |      Z  00h
 -------------------------------------------------------------------------------*/
static Buffer* expandMccLine( Buffer* buffPtr ) {
    ASSERT(buffPtr);
    ASSERT(buffPtr->dataPtr);
    ASSERT(buffPtr->numElements);
    char* mccdata = (char*)buffPtr->dataPtr;
    
    // Detrmine how much data to allocate for the expanded line
    size_t size = 0;
    for( int loop = 0; loop < buffPtr->numElements; loop++ ) {
        size = size + mccCharCount(mccdata[loop]);
    }
    
    ASSERT((size%2) == 0);
    Buffer* outputBuffer = NewBuffer(BUFFER_TYPE_BYTES, (size/2));
    outputBuffer->captionTime = buffPtr->captionTime;
    
    for( int loop = 0; loop < buffPtr->numElements; ) {
        if( ((mccdata[loop] >= 'G') && (mccdata[loop] <= 'Z')) || ((mccdata[loop] >= 'g') && (mccdata[loop] <= 'z')) ) {
            expandMccCode( mccdata[loop], outputBuffer->dataPtr, &outputBuffer->numElements );
            loop++;
        } else if( ((mccdata[loop] >= 'A') && (mccdata[loop] <= 'F')) || ((mccdata[loop] >= 'a') && (mccdata[loop] <= 'f')) || ((mccdata[loop] >= '0') && (mccdata[loop] <= '9')) ) {
            if( isHexByteValid(mccdata[loop], mccdata[(loop+1)]) == TRUE ) {
                outputBuffer->dataPtr[outputBuffer->numElements] = byteFromNibbles(mccdata[loop], mccdata[(loop+1)]);
                outputBuffer->numElements++;
            } else  {
                LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "At %02d:%02d:%02d:%02d Unable to parse MCC Values %c %c - %s",
                    buffPtr->captionTime.hour, buffPtr->captionTime.minute, buffPtr->captionTime.second,
                    buffPtr->captionTime.frame, mccdata[loop], mccdata[(loop+1)], &mccdata[loop] );
            }
            loop = loop + 2;
        } else {
            if( (mccdata[loop] != '\r') && (mccdata[loop] != '\n') ) {
                LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "At %02d:%02d:%02d:%02d Ignoring Spurious Character: %c",
                    buffPtr->captionTime.hour, buffPtr->captionTime.minute, buffPtr->captionTime.second,
                    buffPtr->captionTime.frame, mccdata[loop] );
            }
            loop++;
        }
    }
    ASSERT(outputBuffer->numElements <= outputBuffer->maxNumElements);

#if 0
    for( int loop = 0; loop < outputBuffer->numElements; loop++ ) {
        if( loop == 0 ) {
            printf("\n%02d:%02d:%02d;%02d - %02X ",
                   outputBuffer->captionTime.hour, outputBuffer->captionTime.minute,
                   outputBuffer->captionTime.second, outputBuffer->captionTime.frame,
                   outputBuffer->dataPtr[loop]);
        } else {
            printf("%02X ", outputBuffer->dataPtr[loop]);
        }
    }
#endif
    
    FreeBuffer(buffPtr);
    return outputBuffer;
    
#if 0
    char tmpBuffer[512];
    PrintCaptionsLine( tmpBuffer, 512, outputBuffer->dataPtr );
    tmpBuffer[(strlen(tmpBuffer)-3)] = '\0';
    printf("%s -- Size: %lu\n", tmpBuffer, (size/2));
#endif
}  // expandMccLine()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeMccLine()
 |
 | DESCRIPTION:
 |    This method decodes the Anciallary Data (ANC) Packet, which wraps the Caption
 |    Distribution Packet (CDP), including the Closed Captioning Data (ccdata_section) as
 |    described in the CEA-708 Spec. Below is the list of specs that were leveraged for
 |    this decode:
 |
 |    SMPTE ST 334-1 - Vertical Ancillary Data Mapping of Caption Data and Other Related Data
 |                     (Specifically: SMPTE ST 334-1:2015 - Revision of SMPTE 334-1-2007)
 |    SMPTE ST 334-2 - Caption Distribution Packet (CDP) Definition
 |                     (Specifically: SMPTE ST 334-2:2015 - Revision of SMPTE 334-2-2007)
 |    CEA-708-D - CEA Standard - Digital Television (DTV) Closed Captioning - August 2008
 -------------------------------------------------------------------------------*/
static Buffer* decodeMccLine( Buffer* buffPtr ) {
    ASSERT(buffPtr);
    ASSERT(buffPtr->dataPtr);
    ASSERT(buffPtr->numElements);
    uint8 dataCount;
    boolean timeCodeSectionPresent = FALSE;
    boolean srvcInfoSectionPresent = FALSE;
    
    uint8* tmpPtr = buffPtr->dataPtr;
    
    ANC_packet* ancPacketPtr = (ANC_packet*)tmpPtr;
    
    if( ancPacketPtr->did != ANC_DID_CLOSED_CAPTIONING ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Unexepected DID: 0x%02X vs 0x%02X", ancPacketPtr->did, ANC_DID_CLOSED_CAPTIONING);
    }
    
    if( ancPacketPtr->sdid != ANC_SDID_CEA_708 ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Unexepected SDID: 0x%02X vs 0x%02X (0x%02X is unexpected CEA-608)", ancPacketPtr->sdid, ANC_SDID_CEA_708, ANC_SDID_CEA_608);
    }
    
    dataCount = ancPacketPtr->dc;
    
    tmpPtr = tmpPtr + 3;
    cdp_header captionDataPacketHeader;
    captionDataPacketHeader.cdp_identifier = (tmpPtr[0] << 8) | tmpPtr[1];
    captionDataPacketHeader.cdp_length = tmpPtr[2];
    captionDataPacketHeader.cdp_frame_rate = (tmpPtr[3] & 0xF0) >> 4;
    captionDataPacketHeader.reserved_1 = (tmpPtr[3] & 0x0F);
    captionDataPacketHeader.time_code_present = (tmpPtr[4] & 0x80) >> 7;
    captionDataPacketHeader.ccdata_present = (tmpPtr[4] & 0x40) >> 6;
    captionDataPacketHeader.svcinfo_present = (tmpPtr[4] & 0x20) >> 5;
    captionDataPacketHeader.svc_info_start = (tmpPtr[4] & 0x10) >> 4;
    captionDataPacketHeader.svc_info_change = (tmpPtr[4] & 0x08) >> 3;
    captionDataPacketHeader.svc_info_complete = (tmpPtr[4] & 0x04) >> 2;
    captionDataPacketHeader.caption_service_active = (tmpPtr[4] & 0x02) >> 1;
    captionDataPacketHeader.reserved_2 = (tmpPtr[4] & 0x01);
    captionDataPacketHeader.cdp_hdr_sequence_cntr = (tmpPtr[5] << 8) | tmpPtr[6];
    
    if( captionDataPacketHeader.cdp_identifier != CDP_IDENTIFIER_VALUE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Unexepected CDP Identifier: 0x%04X vs 0x%04X", captionDataPacketHeader.cdp_identifier, CDP_IDENTIFIER_VALUE);
    }
    
    if( captionDataPacketHeader.cdp_length != dataCount ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Mismatch in length: 0x%02X vs 0x%02X", captionDataPacketHeader.cdp_length, dataCount);
    }
    
    decodeFrameRate( captionDataPacketHeader, &buffPtr->captionTime );
    
    if( captionDataPacketHeader.time_code_present == TRUE ) {
        timeCodeSectionPresent = TRUE;
        LOG(DEBUG_LEVEL_INFO, DBG_MCC_DEC, "Time Code Present");
    }
    
    if( captionDataPacketHeader.ccdata_present != TRUE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Missing CC Data");
    }
    
    if( captionDataPacketHeader.svcinfo_present == TRUE ) {
        srvcInfoSectionPresent = TRUE;
    }

    if( captionDataPacketHeader.caption_service_active != TRUE ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Caption Service Active should be True");
    }

    tmpPtr = tmpPtr + 7;
    
    if( timeCodeSectionPresent == TRUE ) {
        time_code_section timeCodeSection;
        timeCodeSection.time_code_section_id = tmpPtr[0];
        
        if( timeCodeSection.time_code_section_id != TIME_CODE_ID ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Invalid Time Code ID: 0x%02X vs 0x%02X", timeCodeSection.time_code_section_id, TIME_CODE_ID);
        }
        
        tmpPtr = tmpPtr + sizeof(time_code_section) - 1;
    }
    
    ccdata_section ccData;
    ccData.ccdata_id = tmpPtr[0];
    ccData.marker_bits = (tmpPtr[1] & 0xE0) >> 5;
    ccData.cc_count = (tmpPtr[1] & 0x1F);
    
    tmpPtr = tmpPtr + 2;
    
    if( ccData.ccdata_id != CC_DATA_ID ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Invalid CCData ID: 0x%02X vs 0x%02X", ccData.ccdata_id, CC_DATA_ID);
    }
    
    uint8 computedNumCcConstructs = numCcConstructsFromFramerate(buffPtr->captionTime.frameRatePerSecTimesOneHundred);
    if( ccData.cc_count != computedNumCcConstructs ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Unexpected Number of CC Constructs: 0x%02X vs 0x%02X", ccData.cc_count, computedNumCcConstructs);
    }
    
    Buffer* outputBuffer = NewBuffer(BUFFER_TYPE_BYTES, (ccData.cc_count * sizeof(cc_construct)));
    outputBuffer->captionTime = buffPtr->captionTime;
    outputBuffer->numElements = (ccData.cc_count * sizeof(cc_construct));
    
    memcpy(outputBuffer->dataPtr, tmpPtr, (ccData.cc_count * sizeof(cc_construct)));
    
    tmpPtr = tmpPtr + (ccData.cc_count * (sizeof(cc_construct)));

    if( srvcInfoSectionPresent == TRUE ) {
        ccsvcinfo_section srvcInfo;
        srvcInfo.ccsvcinfo_id = tmpPtr[0];
        srvcInfo.svc_count = (tmpPtr[1] & 0x0F);
 
        if( srvcInfo.ccsvcinfo_id != CCS_SVCINFO_ID ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Invalid Srvc Info ID: 0x%02X vs 0x%02X", srvcInfo.ccsvcinfo_id, CCS_SVCINFO_ID);
        }

        // 2 Byte Preamble + (Number of Services * 7 Bytes)
        tmpPtr = tmpPtr + 2 + (7 * srvcInfo.svc_count);
    }
    
    cdp_footer cdpFooter;
    cdpFooter.cdp_footer_id = tmpPtr[0];
    
    if( cdpFooter.cdp_footer_id != CDP_FOOTER_ID ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Invalid CDP Footer ID: 0x%02X vs 0x%02X", cdpFooter.cdp_footer_id, CDP_FOOTER_ID);
    }
    
    FreeBuffer(buffPtr);

    ASSERT(outputBuffer->numElements <= outputBuffer->maxNumElements);
    return outputBuffer;
    
#if 0
    char tmpBuffer[512];
    PrintCaptionsLine( tmpBuffer, 512, buffPtr->dataPtr );
    tmpBuffer[(strlen(tmpBuffer)-3)] = '\0';
    printf("%s -- %d CC Constructs\n", tmpBuffer, ccData.cc_count);
#endif
}  // decodeMccLine()

/*------------------------------------------------------------------------------
 | NAME:
 |    mccCharCount()
 |
 | DESCRIPTION:
 |    This function determines the resulting number of charcters from expansion
 |    of a specific character.
 |
 |    Per the description in the MCC File:
 |
 |    ANC data bytes may be represented by one ASCII character according
 |    to the following schema:
 |      G  FAh 00h 00h
 |      H  2 x (FAh 00h 00h)
 |      I  3 x (FAh 00h 00h)
 |      J  4 x (FAh 00h 00h)
 |      K  5 x (FAh 00h 00h)
 |      L  6 x (FAh 00h 00h)
 |      M  7 x (FAh 00h 00h)
 |      N  8 x (FAh 00h 00h)
 |      O  9 x (FAh 00h 00h)
 |      P  FBh 80h 80h
 |      Q  FCh 80h 80h
 |      R  FDh 80h 80h
 |      S  96h 69h
 |      T  61h 01h
 |      U  E1h 00h 00h 00h
 |      Z  00h
 -------------------------------------------------------------------------------*/
static uint8 mccCharCount( char character ) {
    uint8 retval;
    
    switch( character ) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            retval = 1;
            break;
        case 'G':  // G  FAh 00h 00h
        case 'g':  // G  FAh 00h 00h
            retval = 6;
            break;
        case 'H':  // H  2 x (FAh 00h 00h)
        case 'h':  // H  2 x (FAh 00h 00h)
            retval = 12;
            break;
        case 'I':  // I  3 x (FAh 00h 00h)
        case 'i':  // I  3 x (FAh 00h 00h)
            retval = 18;
            break;
        case 'J':  // J  4 x (FAh 00h 00h)
        case 'j':  // J  4 x (FAh 00h 00h)
            retval = 24;
            break;
        case 'K':  // K  5 x (FAh 00h 00h)
        case 'k':  // K  5 x (FAh 00h 00h)
            retval = 30;
            break;
        case 'L':  // L  6 x (FAh 00h 00h)
        case 'l':  // L  6 x (FAh 00h 00h)
            retval = 36;
            break;
        case 'M':  // M  7 x (FAh 00h 00h)
        case 'm':  // M  7 x (FAh 00h 00h)
            retval = 42;
            break;
        case 'N':  // N  8 x (FAh 00h 00h)
        case 'n':  // N  8 x (FAh 00h 00h)
            retval = 48;
            break;
        case 'O':  // O  9 x (FAh 00h 00h)
        case 'o':  // O  9 x (FAh 00h 00h)
            retval = 54;
            break;
        case 'P':  // P  FBh 80h 80h
        case 'p':  // P  FBh 80h 80h
        case 'Q':  // Q  FCh 80h 80h
        case 'q':  // Q  FCh 80h 80h
        case 'R':  // R  FDh 80h 80h
        case 'r':  // R  FDh 80h 80h
            retval = 6;
            break;
        case 'S':  // S  96h 69h
        case 's':  // S  96h 69h
        case 'T':  // T  61h 01h
        case 't':  // T  61h 01h
            retval = 4;
            break;
        case 'U':  // U  E1h 00h 00h 00h
        case 'u':  // U  E1h 00h 00h 00h
            retval = 8;
            break;
        case 'Z':  // Z  00h
        case 'z':  // Z  00h
            retval = 2;
            break;
        default:
            retval = 0;
            break;
    }
    return retval;
}  // mccCharCount()

/*------------------------------------------------------------------------------
 | NAME:
 |    expandMccCode()
 |
 | DESCRIPTION:
 |    This is just a conveinece function to save from cut and paste.
 -------------------------------------------------------------------------------*/
static void addMultipleFaZeroZero( uint8 number, uint8* lineBytesPtr, uint16* numLineBytesPtr ) {
    
    for( int loop = 0; loop < number; loop++ ) {
        lineBytesPtr[(*numLineBytesPtr)++] = 0xFA;
        lineBytesPtr[(*numLineBytesPtr)++] = 0x00;
        lineBytesPtr[(*numLineBytesPtr)++] = 0x00;
    }
}  // addMultipleFaZeroZero()

/*------------------------------------------------------------------------------
 | NAME:
 |    expandMccCode()
 |
 | DESCRIPTION:
 |    This function determines the resulting number of charcters from expansion
 |    of a specific character.
 |
 |    Per the description in the MCC File:
 |
 |    ANC data bytes may be represented by one ASCII character according
 |    to the following schema:
 |      G  FAh 00h 00h
 |      H  2 x (FAh 00h 00h)
 |      I  3 x (FAh 00h 00h)
 |      J  4 x (FAh 00h 00h)
 |      K  5 x (FAh 00h 00h)
 |      L  6 x (FAh 00h 00h)
 |      M  7 x (FAh 00h 00h)
 |      N  8 x (FAh 00h 00h)
 |      O  9 x (FAh 00h 00h)
 |      P  FBh 80h 80h
 |      Q  FCh 80h 80h
 |      R  FDh 80h 80h
 |      S  96h 69h
 |      T  61h 01h
 |      U  E1h 00h 00h 00h
 |      Z  00h
 -------------------------------------------------------------------------------*/
static void expandMccCode( uint8 code, uint8* lineBytesPtr, uint16* numLineBytesPtr ) {
    
    switch( code ) {
        case 'G':  // G  FAh 00h 00h
        case 'g':  // G  FAh 00h 00h
            lineBytesPtr[(*numLineBytesPtr)++] = 0xFA;
            lineBytesPtr[(*numLineBytesPtr)++] = 0x00;
            lineBytesPtr[(*numLineBytesPtr)++] = 0x00;
            break;
        case 'H':  // H  2 x (FAh 00h 00h)
        case 'h':  // H  2 x (FAh 00h 00h)
            addMultipleFaZeroZero(2, lineBytesPtr, numLineBytesPtr);
            break;
        case 'I':  // I  3 x (FAh 00h 00h)
        case 'i':  // I  3 x (FAh 00h 00h)
            addMultipleFaZeroZero(3, lineBytesPtr, numLineBytesPtr);
            break;
        case 'J':  // J  4 x (FAh 00h 00h)
        case 'j':  // J  4 x (FAh 00h 00h)
            addMultipleFaZeroZero(4, lineBytesPtr, numLineBytesPtr);
            break;
        case 'K':  // K  5 x (FAh 00h 00h)
        case 'k':  // K  5 x (FAh 00h 00h)
            addMultipleFaZeroZero(5, lineBytesPtr, numLineBytesPtr);
            break;
        case 'L':  // L  6 x (FAh 00h 00h)
        case 'l':  // L  6 x (FAh 00h 00h)
            addMultipleFaZeroZero(6, lineBytesPtr, numLineBytesPtr);
            break;
        case 'M':  // M  7 x (FAh 00h 00h)
        case 'm':  // M  7 x (FAh 00h 00h)
            addMultipleFaZeroZero(7, lineBytesPtr, numLineBytesPtr);
            break;
        case 'N':  // N  8 x (FAh 00h 00h)
        case 'n':  // N  8 x (FAh 00h 00h)
            addMultipleFaZeroZero(8, lineBytesPtr, numLineBytesPtr);
            break;
        case 'O':  // O  9 x (FAh 00h 00h)
        case 'o':  // O  9 x (FAh 00h 00h)
            addMultipleFaZeroZero(9, lineBytesPtr, numLineBytesPtr);
            break;
        case 'P':  // P  FBh 80h 80h
        case 'p':  // P  FBh 80h 80h
            lineBytesPtr[(*numLineBytesPtr)++] = 0xFB;
            lineBytesPtr[(*numLineBytesPtr)++] = 0x80;
            lineBytesPtr[(*numLineBytesPtr)++] = 0x80;
            break;
        case 'Q':  // Q  FCh 80h 80h
        case 'q':  // Q  FCh 80h 80h
            lineBytesPtr[(*numLineBytesPtr)++] = 0xFC;
            lineBytesPtr[(*numLineBytesPtr)++] = 0x80;
            lineBytesPtr[(*numLineBytesPtr)++] = 0x80;
            break;
        case 'R':  // R  FDh 80h 80h
        case 'r':  // R  FDh 80h 80h
            lineBytesPtr[(*numLineBytesPtr)++] = 0xFD;
            lineBytesPtr[(*numLineBytesPtr)++] = 0x80;
            lineBytesPtr[(*numLineBytesPtr)++] = 0x80;
            break;
        case 'S':  // S  96h 69h
        case 's':  // S  96h 69h
            lineBytesPtr[(*numLineBytesPtr)++] = 0x96;
            lineBytesPtr[(*numLineBytesPtr)++] = 0x69;
            break;
        case 'T':  // T  61h 01h
        case 't':  // T  61h 01h
            lineBytesPtr[(*numLineBytesPtr)++] = 0x61;
            lineBytesPtr[(*numLineBytesPtr)++] = 0x01;
            break;
        case 'U':  // U  E1h 00h 00h 00h
        case 'u':  // U  E1h 00h 00h 00h
            lineBytesPtr[(*numLineBytesPtr)++] = 0xE1;
            lineBytesPtr[(*numLineBytesPtr)++] = 0x00;
            lineBytesPtr[(*numLineBytesPtr)++] = 0x00;
            lineBytesPtr[(*numLineBytesPtr)++] = 0x00;
            break;
        case 'Z':  // Z  00h
        case 'z':  // Z  00h
            lineBytesPtr[(*numLineBytesPtr)++] = 0x00;
            break;
        default:
            LOG(DEBUG_LEVEL_ERROR, DBG_FILE_IN, "Invalid Replacement Char: %c", code);
            break;
    }
}  // expandMccCode()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeFrameRate()
 |
 | DESCRIPTION:
 |    This function decodes the cdp_frame_rate from the Caption Distribution Packet
 |    (CDP) Defintion. SMPTE ST 334-2:2015 - Revision of SMPTE 334-2-2007
 -------------------------------------------------------------------------------*/
static void decodeFrameRate( cdp_header captionDataPacketHeader, CaptionTime* captionsTimePtr ) {
    ASSERT(captionsTimePtr);
    uint32 frameRatePerSecTimesOneThousand;
    
    switch( captionDataPacketHeader.cdp_frame_rate ) {
        case CDP_FRAME_RATE_23_976:
            frameRatePerSecTimesOneThousand = 2397;
            break;
        case CDP_FRAME_RATE_24:
            frameRatePerSecTimesOneThousand = 2400;
            break;
        case CDP_FRAME_RATE_25:
            frameRatePerSecTimesOneThousand = 2500;
            break;
        case CDP_FRAME_RATE_29_97:
            frameRatePerSecTimesOneThousand = 2997;
            break;
        case CDP_FRAME_RATE_30:
            frameRatePerSecTimesOneThousand = 3000;
            break;
        case CDP_FRAME_RATE_50:
            frameRatePerSecTimesOneThousand = 5000;
            break;
        case CDP_FRAME_RATE_59_94:
            frameRatePerSecTimesOneThousand = 5994;
            break;
        case CDP_FRAME_RATE_60:
            frameRatePerSecTimesOneThousand = 6000;
            break;
        default:
            LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Frame Rate Not Decoded: 0x%02X", captionDataPacketHeader.cdp_frame_rate );
            return;
    }
    
    if( captionsTimePtr->frameRatePerSecTimesOneHundred == 0 ) {
        captionsTimePtr->frameRatePerSecTimesOneHundred = frameRatePerSecTimesOneThousand;
        LOG(DEBUG_LEVEL_INFO, DBG_MCC_DEC, "Frame Rate: %d.%d", (frameRatePerSecTimesOneThousand / 100), (frameRatePerSecTimesOneThousand % 100) );
    } else if( captionsTimePtr->frameRatePerSecTimesOneHundred == frameRatePerSecTimesOneThousand ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_MCC_DEC, "Frame Rate Mismatch: %d.%d vs. %d.%d", (frameRatePerSecTimesOneThousand / 100), (frameRatePerSecTimesOneThousand % 100),
            (captionsTimePtr->frameRatePerSecTimesOneHundred / 100), (captionsTimePtr->frameRatePerSecTimesOneHundred % 100) );
    }
}  // decodeFrameRate()
