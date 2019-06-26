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
#include <stdio.h>

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

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    encodeTimeCode()
 |
 | INPUT PARAMETERS:
 |    timecode - In the formate of HH:MM:SS;FF
 |
 | RETURN VALUES:
 |    captionTimePtr - Caption Time Structure to hold the result.
 |    boolean - Success = TRUE; Failure = FALSE
 |
 | DESCRIPTION:
 |    This method decodes a timestamp from an SCC or MCC File. Closed Captioning
 |    timestamps follow the format: <Hour>:<Minute>:<Second>;<Frame>
 |    The use of ; rather than : in, at a minumum, the separation between
 |    Second and Frame indicates that asset is Drop Frame.
 |
 |    Example: 01:48:21;04 -> 1 Hour, 48 Minute, 21 Seconds, and 4th Frame
 -------------------------------------------------------------------------------*/
void encodeTimeCode( CaptionTime* captionTimePtr, char* scratchBufferPtr ) {
    ASSERT(captionTimePtr);
    ASSERT(scratchBufferPtr);

    sprintf(scratchBufferPtr, "{TIMESTAMP ERROR!}");

    if( (captionTimePtr->minute > 59) || (captionTimePtr->second > 59) ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_GENERAL, "Invalid Minute: %d or Second: %d", captionTimePtr->minute, captionTimePtr->second);
    } else {
        if (captionTimePtr->source == CAPTION_TIME_FRAME_NUMBERING) {
            ASSERT(captionTimePtr->millisecond == 0);
            if (captionTimePtr->dropframe == TRUE) {
                sprintf(scratchBufferPtr, "%02d:%02d:%02d;%02d", captionTimePtr->hour, captionTimePtr->minute,
                        captionTimePtr->second, captionTimePtr->frame);
            } else {
                sprintf(scratchBufferPtr, "%02d:%02d:%02d:%02d", captionTimePtr->hour, captionTimePtr->minute,
                        captionTimePtr->second, captionTimePtr->frame);
            }
        } else if (captionTimePtr->source == CAPTION_TIME_PTS_NUMBERING) {
            if (captionTimePtr->millisecond > 999) {
                LOG(DEBUG_LEVEL_ERROR, DBG_GENERAL, "Invalid Millisecond: %d", captionTimePtr->millisecond);
            } else {
                ASSERT(captionTimePtr->frame == 0);
                sprintf(scratchBufferPtr, "%02d:%02d:%02d,%03d", captionTimePtr->hour, captionTimePtr->minute,
                        captionTimePtr->second, captionTimePtr->millisecond);
            }
        } else {
            LOG(DEBUG_LEVEL_ERROR, DBG_GENERAL, "Unable to determine Caption Time Type");
        }
    }
} // encodeTimeCode()

/*------------------------------------------------------------------------------
 | NAME:
 |    decodeTimeCode()
 |
 | INPUT PARAMETERS:
 |    timecode - In the format of HH:MM:SS;FF
 |
 | RETURN VALUES:
 |    captionTimePtr - Caption Time Structure to hold the result.
 |    boolean - Success = TRUE; Failure = FALSE
 |
 | DESCRIPTION:
 |    This method decodes a timestamp from an SCC or MCC File. Closed Captioning
 |    timestamps follow the format: <Hour>:<Minute>:<Second>;<Frame>
 |    The use of ; rather than : in, at a minumum, the separation between
 |    Second and Frame indicates that asset is Drop Frame.
 |
 |    Example: 01:48:21;04 -> 1 Hour, 48 Minute, 21 Seconds, and 4th Frame
 -------------------------------------------------------------------------------*/
boolean decodeTimeCode( char* timecode, CaptionTime* captionTimePtr ) {
    ASSERT(timecode);
    ASSERT(captionTimePtr);
    
    if( strlen(timecode) == 11 ) {
        if( timecode[8] == ':' ) {
            captionTimePtr->dropframe = FALSE;
        } else if( timecode[8] == ';' ) {
            captionTimePtr->dropframe = TRUE;
        } else {
            LOG(DEBUG_LEVEL_ERROR, DBG_GENERAL, "Unable to determine Dropframe: %s", timecode);
            return FALSE;
        }

        captionTimePtr->hour = ((timecode[0] - '0') * 10) + (timecode[1] - '0');
        captionTimePtr->minute = ((timecode[3] - '0') * 10) + (timecode[4] - '0');
        captionTimePtr->second = ((timecode[6] - '0') * 10) + (timecode[7] - '0');
        captionTimePtr->frame = ((timecode[9] - '0') * 10) + (timecode[10] - '0');

        captionTimePtr->source = CAPTION_TIME_FRAME_NUMBERING;
        return TRUE;
    } else {
        LOG(DEBUG_LEVEL_ERROR, DBG_GENERAL, "Invalid Timecode: %s", timecode);
        return FALSE;
    }
}  // decodeTimeCode()

/*------------------------------------------------------------------------------
 | NAME:
 |    isHexByteValid()
 |
 | INPUT PARAMETERS:
 |    msn - Most Significant Nibble (i.e. 0xF0)
 |    lsn - Least Significant Nibble (i.e. 0x0F)
 |
 | RETURN VALUES:
 |    TRUE - Valid; FALSE - Not Valid
 |
 | DESCRIPTION:
 |    This method determines if the two characters which are passed in (presumably
 |    from a byte decoded in a ASCII file) represent the correct ASCII Characters
 |    which would build a Hex Number.
 -------------------------------------------------------------------------------*/
boolean isHexByteValid( char msn, char lsn ) {
    boolean retval = FALSE;
    
    if( ((msn >= 'A') && (msn <= 'F')) || ((msn >= 'a') && (msn <= 'f')) || ((msn >= '0') && (msn <= '9')) ) {
        retval = TRUE;
    }
    
    if( ((lsn >= 'A') && (lsn <= 'F')) || ((lsn >= 'a') && (lsn <= 'f')) || ((lsn >= '0') && (lsn <= '9')) ) {
        return retval;
    }
    
    return FALSE;
}  // isHexByteValid()

/*------------------------------------------------------------------------------
 | NAME:
 |    byteFromNibbles()
 |
 | INPUT PARAMETERS:
 |    msn - Most Significant Nibble (i.e. 0xF0)
 |    lsn - Least Significant Nibble (i.e. 0x0F)
 |
 | RETURN VALUES:
 |    Hex Value specified by ASCII Nibbles.
 |
 | DESCRIPTION:
 |    This method simply converts ASCII Nibbles to Hex and returns the result.
 -------------------------------------------------------------------------------*/
uint8 byteFromNibbles( char msn, char lsn ) {
    uint8 byte;

    ASSERT(isHexByteValid(msn, lsn));
    
    if( msn < 'A' ) byte = msn - '0';
    else if( msn < 'a' ) byte = msn - 'A' + 10;
    else byte = msn - 'a' + 10;
    
    if( lsn < 'A' ) byte = (byte * 0x10) + lsn - '0';
    else if( lsn < 'a' ) byte = (byte * 0x10) + lsn - 'A' + 10;
    else byte = (byte * 0x10) + lsn - 'a' + 10;
    
    return byte;
}  // byteFromNibbles()

/*------------------------------------------------------------------------------
 | NAME:
 |    byteToAscii()
 |
 | INPUT PARAMETERS:
 |    hexByte - The Hex Byte to Convert
 |
 | RETURN VALUES:
 |    msn - Most Significant ASCII Nibble (i.e. 0xF0)
 |    lsn - Least Significant ASCII Nibble (i.e. 0x0F)
 |
 | DESCRIPTION:
 |    This method simply converts a Hex Byte to ASCII Nibbles and returns the result.
 -------------------------------------------------------------------------------*/
void byteToAscii( uint8 hexByte, uint8* msn, uint8* lsn ) {
    ASSERT(msn);
    ASSERT(lsn);

    *msn = (hexByte & 0xF0);
    *msn = *msn >> 4;
    if( *msn < 0x0A ) *msn = *msn + '0';
    else *msn = (*msn - 0x0A) + 'A';

    *lsn = (hexByte & 0x0F);
    if( *lsn < 0x0A ) *lsn = *lsn + '0';
    else *lsn = (*lsn - 0x0A) + 'A';
    
}  // byteToAscii()

/*------------------------------------------------------------------------------
 | NAME:
 |    numCcConstructsFromFramerate()
 |
 | INPUT PARAMETERS:
 |    frameRatePerSecTimesOneHundred - (Framerate / Sec) * 100.
 |
 | RETURN VALUES:
 |    Number of CC Constructs
 |
 | DESCRIPTION:
 |    This method simply returns the appropiate number of Closed Captioning
 |    Constructs that would be found in a cc_data structure, per the CEA-708 Spec.
 -------------------------------------------------------------------------------*/
uint8 numCcConstructsFromFramerate( uint32 frameRatePerSecTimesOneHundred ) {
    uint8 retval = 0;
    
    switch (frameRatePerSecTimesOneHundred) {
        case 2397:
        case 2400:
            retval = CC_COUNT_FRAME_RATE_24;
            break;
        case 2500:
            retval = CC_COUNT_FRAME_RATE_25;
            break;
        case 2997:
        case 3000:
            retval = CC_COUNT_FRAME_RATE_30;
            break;
        case 5000:
            retval = CC_COUNT_FRAME_RATE_50;
            break;
        case 5994:
        case 6000:
            retval = CC_COUNT_FRAME_RATE_60;
            break;
        default:
            LOG(DEBUG_LEVEL_ERROR, DBG_GENERAL,"Unknown Framerate: %d", frameRatePerSecTimesOneHundred);
            break;
    }
    return retval;
}  // numCcConstructsFromFramerate()

/*------------------------------------------------------------------------------
 | NAME:
 |    cdpFramerateFromFramerate()
 |
 | INPUT PARAMETERS:
 |    frameRatePerSecTimesOneHundred - (Framerate / Sec) * 100.
 |
 | RETURN VALUES:
 |    CDP Framerate
 |
 | DESCRIPTION:
 |    This method simply returns the appropiate CDP Framerate, per the spec:
 |    Caption Distribution Packet (CDP) Definition - SMPTE ST 334-2:2015
 -------------------------------------------------------------------------------*/
uint8 cdpFramerateFromFramerate( uint32 frameRatePerSecTimesOneHundred ) {
    uint8 retval = 0;
    
    switch (frameRatePerSecTimesOneHundred) {
        case 2397:
            retval = CDP_FRAME_RATE_23_976;
            break;
        case 2400:
            retval = CDP_FRAME_RATE_24;
            break;
        case 2500:
            retval = CDP_FRAME_RATE_25;
            break;
        case 2997:
            retval = CDP_FRAME_RATE_29_97;
            break;
        case 3000:
            retval = CDP_FRAME_RATE_30;
            break;
        case 5000:
            retval = CDP_FRAME_RATE_50;
            break;
        case 5994:
            retval = CDP_FRAME_RATE_59_94;
            break;
        case 6000:
            retval = CDP_FRAME_RATE_60;
            break;
        default:
            LOG(DEBUG_LEVEL_ERROR, DBG_GENERAL,"Unknown Framerate: %d", frameRatePerSecTimesOneHundred);
            break;
    }
    
    return retval;
}  // cdpFramerateFromFramerate()

/*------------------------------------------------------------------------------
 | NAME:
 |    isFramerateValid()
 |
 | INPUT PARAMETERS:
 |    frameRatePerSecTimesOneHundred - (Framerate / Sec) * 100.
 |
 | RETURN VALUES:
 |    TRUE - Valid Framerate; FALSE - Invalid Framerate
 |
 | DESCRIPTION:
 |    This method simply returns whether or not a framerate is recognized as valid.
 -------------------------------------------------------------------------------*/
boolean isFramerateValid( uint32 frameRatePerSecTimesOneHundred ) {

    switch (frameRatePerSecTimesOneHundred) {
        case 2397:
        case 2400:
        case 2500:
        case 2997:
        case 3000:
        case 5000:
        case 5994:
        case 6000:
            return TRUE;
        default:
            return FALSE;
    }
}  // isFramerateValid()

/*------------------------------------------------------------------------------
 | NAME:
 |    printCaptionsLine()
 |
 | INPUT PARAMETERS:
 |    buffer - The pre-allocated buffer to write the decoded line into.
 |    size - The amount of space allocated in the pre-allocated buffer.
 |    dataPtr - Pointer to the data to print out.
 |    dataLen - Length of data to print out.
 |    captionsTimePtr - Pointer to the timing data to print or NULL if none.
 |
 | RETURN VALUES:
 |    The ASCII Text representation of the Caption Line is written into the
 |    passed in buffer.
 |
 | DESCRIPTION:
 |    This prints the specific Caption Line into a passed in buffer for debugging.
 -------------------------------------------------------------------------------*/
void printCaptionsLine( char* buffer, uint16 size, uint8* dataPtr, uint16 dataLen, CaptionTime* captionsTimePtr ) {
    ASSERT(buffer);
    ASSERT(dataPtr);
    ASSERT(captionsTimePtr);
    
    if( size < 20 ) {
        LOG(DEBUG_LEVEL_ERROR, DBG_GENERAL,"Buffer too small. Unable to dump line %d", size);
        return;
    }
    
    if( captionsTimePtr != NULL ) {
        if( captionsTimePtr->dropframe == TRUE ) {
            sprintf(buffer, "%02d:%02d:%02d;%02d\t", captionsTimePtr->hour, captionsTimePtr->minute,
                    captionsTimePtr->second, captionsTimePtr->frame);
        } else {
            sprintf(buffer, "%02d:%02d:%02d:%02d\t", captionsTimePtr->hour, captionsTimePtr->minute,
                    captionsTimePtr->second, captionsTimePtr->frame);
        }
        size = size - 12;
        buffer = &buffer[(strlen(buffer))];
    }
    
    for( int loop = 0; loop < dataLen; loop = loop + 2 ) {
        if( size < 10 ) {
            LOG(DEBUG_LEVEL_ERROR, DBG_GENERAL,"Buffer too small. Line Trunkated");
            sprintf(buffer, "... \n\n");
            return;
        }
        sprintf(buffer, "%02X%02X ", dataPtr[loop], dataPtr[(loop+1)]);
        size = size - 5;
        buffer = &buffer[(strlen(buffer))];
    }
    sprintf(buffer, "\n\n");
}  // printCaptionsLine()

/*------------------------------------------------------------------------------
 | NAME:
 |    frameToTimeCode()
 |
 | INPUT PARAMETERS:
 |    frameNum - The frame number index into the asset.
 |    frameRate - (Framerate / Sec) * 100.
 |
 | RETURN VALUES:
 |    captionsTimePtr - The HH:MM:SS:FF representation
 |
 | DESCRIPTION:
 |    This converts from frame number to time code. Additionally, this will save
 |    the frame rate in the same output structure.
 -------------------------------------------------------------------------------*/
void frameToTimeCode( uint32 frameNum, uint32 frameRate, CaptionTime* captionTimePtr ) {
    ASSERT(captionTimePtr);
    
    captionTimePtr->frame = ((frameNum * 100) % frameRate) / 100;
    uint32 tmpFrameNum = frameNum - captionTimePtr->frame;
    tmpFrameNum = (tmpFrameNum * 100) / frameRate;
    captionTimePtr->hour = tmpFrameNum / 3600;
    captionTimePtr->minute = (tmpFrameNum % 3600) / 60;
    captionTimePtr->second = tmpFrameNum % 60;
    captionTimePtr->frameRatePerSecTimesOneHundred = frameRate;
    captionTimePtr->source = CAPTION_TIME_FRAME_NUMBERING;
}  // frameToTimeCode()

/*------------------------------------------------------------------------------
 | NAME:
 |    timeCodeToFrame()
 |
 | INPUT PARAMETERS:
 |    captionsTimePtr - The HH:MM:SS:FF representation, including Framerate
 |
 | RETURN VALUES:
 |    frameNum - The frame number index into the asset.
 |
 | DESCRIPTION:
 |    This converts from frame number to time code. Additionally, this will save
 |    the frame rate in the same output structure.
 -------------------------------------------------------------------------------*/
uint32 timeCodeToFrame( CaptionTime* captionTimePtr ) {
    ASSERT(captionTimePtr);
    ASSERT(captionTimePtr->frameRatePerSecTimesOneHundred);
    
    uint32 frame = (((captionTimePtr->hour * 60 * 60) +
                     (captionTimePtr->minute * 60) + captionTimePtr->second) *
                    captionTimePtr->frameRatePerSecTimesOneHundred) / 100;
    frame = frame + captionTimePtr->frame;
    return frame;
}

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/

