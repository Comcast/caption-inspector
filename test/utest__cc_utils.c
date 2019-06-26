//
// Created by Rob Taylor on 2019-04-02.
//

#include "test_engine.h"
#include "../src/utils/cc_utils.c"

/*----------------------------------------------------------------------------*/
/*--                             Test Cases                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: encodeTimeCode()
 |
 | TEST CASES:
 |     1) Encode a PTS Based Timestamp.
 |     2) Encode the smallest PTS Based Timestamp.
 |     3) Encode the largest PTS Based Timestamp.
 |     4) Encode a Drop Frame Based Timestamp.
 |     5) Encode the smallest DF Based Timestamp.
 |     6) Encode the largest DF Based Timestamp.
 |     7) Encode a No Drop Frame Based Timestamp.
 |     8) Encode the smallest NDF Based Timestamp.
 |     9) Encode the largest NDF Based Timestamp.
 |    10) Encode a NULL Timestamp.
 -------------------------------------------------------------------------------*/
void utest__encodeTimeCode( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    CaptionTime captionTime;
    char scratchBuffer[500];

    TEST_START("Test Case: encodeTimeCode() - Encode a PTS Based Timestamp.");
    captionTime.hour = 11;
    captionTime.minute = 59;
    captionTime.second = 12;
    captionTime.millisecond = 123;
    captionTime.frame = 0;
    captionTime.source = CAPTION_TIME_PTS_NUMBERING;
    encodeTimeCode(&captionTime, scratchBuffer);
    ASSERT_STREQ("11:59:12,123", scratchBuffer);
    TEST_END

    TEST_START("Test Case: encodeTimeCode() - Encode the smallest PTS Based Timestamp.");
    captionTime.hour = 0;
    captionTime.minute = 0;
    captionTime.second = 0;
    captionTime.millisecond = 0;
    captionTime.frame = 0;
    captionTime.source = CAPTION_TIME_PTS_NUMBERING;
    encodeTimeCode(&captionTime, scratchBuffer);
    ASSERT_STREQ("00:00:00,000", scratchBuffer);
    TEST_END

    TEST_START("Test Case: encodeTimeCode() - Encode the largest PTS Based Timestamp.");
    captionTime.hour = 0xFF;
    captionTime.minute = 12;
    captionTime.second = 13;
    captionTime.millisecond = 0xFFFF;
    captionTime.frame = 0;
    captionTime.source = CAPTION_TIME_PTS_NUMBERING;
    ERROR_EXPECTED
    encodeTimeCode(&captionTime, scratchBuffer);
    ASSERT_STREQ("{TIMESTAMP ERROR!}", scratchBuffer);
    TEST_END

    TEST_START("Test Case: encodeTimeCode() - Encode a Drop Frame Based Timestamp.");
    captionTime.hour = 11;
    captionTime.minute = 04;
    captionTime.second = 28;
    captionTime.frame = 12;
    captionTime.millisecond = 0;
    captionTime.dropframe = TRUE;
    captionTime.source = CAPTION_TIME_FRAME_NUMBERING;
    encodeTimeCode(&captionTime, scratchBuffer);
    ASSERT_STREQ("11:04:28;12", scratchBuffer);
    TEST_END

    TEST_START("Test Case: encodeTimeCode() - Encode the smallest DF Based Timestamp.");
    captionTime.hour = 0;
    captionTime.minute = 0;
    captionTime.second = 0;
    captionTime.frame = 0;
    captionTime.millisecond = 0;
    captionTime.dropframe = TRUE;
    captionTime.source = CAPTION_TIME_FRAME_NUMBERING;
    encodeTimeCode(&captionTime, scratchBuffer);
    ASSERT_STREQ("00:00:00;00", scratchBuffer);
    TEST_END

    TEST_START("Test Case: encodeTimeCode() - Encode the largest DF Based Timestamp.");
    captionTime.hour = 0xFF;
    captionTime.minute = 0xFF;
    captionTime.second = 0xFF;
    captionTime.frame = 0xFF;
    captionTime.millisecond = 0;
    captionTime.dropframe = TRUE;
    captionTime.source = CAPTION_TIME_FRAME_NUMBERING;
    ERROR_EXPECTED
    encodeTimeCode(&captionTime, scratchBuffer);
    ASSERT_STREQ("{TIMESTAMP ERROR!}", scratchBuffer);
    TEST_END


    TEST_START("Test Case: encodeTimeCode() - Encode a No Drop Frame Based Timestamp.");
    captionTime.hour = 9;
    captionTime.minute = 12;
    captionTime.second = 32;
    captionTime.frame = 24;
    captionTime.millisecond = 0;
    captionTime.dropframe = FALSE;
    captionTime.source = CAPTION_TIME_FRAME_NUMBERING;
    encodeTimeCode(&captionTime, scratchBuffer);
    ASSERT_STREQ("09:12:32:24", scratchBuffer);
    TEST_END

    TEST_START("Test Case: encodeTimeCode() - Encode the smallest NDF Based Timestamp.");
    captionTime.hour = 0;
    captionTime.minute = 0;
    captionTime.second = 0;
    captionTime.frame = 0;
    captionTime.millisecond = 0;
    captionTime.dropframe = FALSE;
    captionTime.source = CAPTION_TIME_FRAME_NUMBERING;
    encodeTimeCode(&captionTime, scratchBuffer);
    ASSERT_STREQ("00:00:00:00", scratchBuffer);
    TEST_END

    TEST_START("Test Case: encodeTimeCode() - Encode the largest NDF Based Timestamp.");
    captionTime.hour = 0xFF;
    captionTime.minute = 0xFF;
    captionTime.second = 0xFF;
    captionTime.frame = 0xFF;
    captionTime.millisecond = 0;
    captionTime.dropframe = FALSE;
    captionTime.source = CAPTION_TIME_FRAME_NUMBERING;
    ERROR_EXPECTED
    encodeTimeCode(&captionTime, scratchBuffer);
    ASSERT_STREQ("{TIMESTAMP ERROR!}", scratchBuffer);
    TEST_END

    TEST_START("Test Case: encodeTimeCode() - Encode a NULL Timestamp.");
    memset(&captionTime, 0, sizeof(CaptionTime));
    ERROR_EXPECTED
    encodeTimeCode(&captionTime, scratchBuffer);
    ASSERT_STREQ("{TIMESTAMP ERROR!}", scratchBuffer);
    TEST_END

}  // utest__encodeTimeCode()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: decodeTimeCode()
 |
 | TEST CASES:
 |    1) Decode a Dropframe Timestamp.
 |    2) Decode a No Dropframe Timestamp.
 |    3) Decode a Bogus Dropframe Indication.
 |    4) Decode a Bogus Timestamp.
 -------------------------------------------------------------------------------*/
void utest__decodeTimeCode( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    CaptionTime captionTime;
    boolean retval;

    TEST_START("Test Case: decodeTimeCode() - Decode a Dropframe Timestamp.");
    retval = decodeTimeCode("01:02:03;04", &captionTime);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, captionTime.hour);
    ASSERT_EQ(2, captionTime.minute);
    ASSERT_EQ(3, captionTime.second);
    ASSERT_EQ(4, captionTime.frame);
    ASSERT_EQ(CAPTION_TIME_FRAME_NUMBERING, captionTime.source);
    ASSERT_EQ(TRUE, captionTime.dropframe);
    TEST_END

    TEST_START("Test Case: decodeTimeCode() - Decode a No Dropframe Timestamp.");
    retval = decodeTimeCode("05:16:41:66", &captionTime);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(5, captionTime.hour);
    ASSERT_EQ(16, captionTime.minute);
    ASSERT_EQ(41, captionTime.second);
    ASSERT_EQ(66, captionTime.frame);
    ASSERT_EQ(CAPTION_TIME_FRAME_NUMBERING, captionTime.source);
    ASSERT_EQ(FALSE, captionTime.dropframe);
    TEST_END

    TEST_START("Test Case: decodeTimeCode() - Decode a Bogus Dropframe Indication.");
    ERROR_EXPECTED
    retval = decodeTimeCode("05:16:41,66", &captionTime);
    ASSERT_EQ(FALSE, retval);
    TEST_END

    TEST_START("Test Case: decodeTimeCode() - Decode a Bogus Timestamp.");
    ERROR_EXPECTED
    retval = decodeTimeCode("This ain't a timestamp", &captionTime);
    ASSERT_EQ(FALSE, retval);
    TEST_END

}  // utest__decodeTimeCode()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: isHexByteValid()
 |
 | TEST CASES:
 |    1) Valid Hex Byte (Uppercase).
 |    2) Valid Hex Byte (Lowercase).
 |    3) Invalid High Nibble, Valid Low Nibble.
 |    4) Valid High Nibble, Invalid Low Nibble.
 |    5) Invalid Hex Byte.
 -------------------------------------------------------------------------------*/
void utest__isHexByteValid( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    boolean retval;

    TEST_START("Test Case: isHexByteValid() - Valid Hex Byte (Uppercase).");
    retval = isHexByteValid('6', 'A');
    ASSERT_EQ(TRUE, retval);
    TEST_END

    TEST_START("Test Case: isHexByteValid() - Valid Hex Byte (Lowercase).");
    retval = isHexByteValid('b', '5');
    ASSERT_EQ(TRUE, retval);
    TEST_END

    TEST_START("Test Case: isHexByteValid() - Invalid High Nibble, Valid Low Nibble.");
    retval = isHexByteValid('x', '2');
    ASSERT_EQ(FALSE, retval);
    TEST_END

    TEST_START("Test Case: isHexByteValid() - Valid High Nibble, Invalid Low Nibble.");
    retval = isHexByteValid('3', 'z');
    ASSERT_EQ(FALSE, retval);
    TEST_END

    TEST_START("Test Case: isHexByteValid() - Invalid Hex Byte.");
    retval = isHexByteValid('R', 't');
    ASSERT_EQ(FALSE, retval);
    TEST_END

}  // utest__isHexByteValid()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: byteFromNibbles()
 |
 | TEST CASES:
 |    1) Valid Uppercase Hex Byte.
 |    2) Valid Lowercase Hex Byte.
 |    3) Invalid Hex Byte.
 -------------------------------------------------------------------------------*/
void utest__byteFromNibbles( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    uint8 retval;

    TEST_START("Test Case: isHexByteValid() - Valid Uppercase Hex Byte.");
    retval = byteFromNibbles('6', 'A');
    ASSERT_EQ(0x6A, retval);
    TEST_END

    TEST_START("Test Case: isHexByteValid() - Valid Lowercase Hex Byte.");
    retval = byteFromNibbles('b', 'd');
    ASSERT_EQ(0xBD, retval);
    TEST_END

    TEST_START("Test Case: isHexByteValid() - Invalid Hex Byte.");
    FATAL_ERROR_EXPECTED
    retval = byteFromNibbles('x', 'y');
    TEST_END

}  // utest__byteFromNibbles()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: byteToAscii()
 |
 | TEST CASES:
 |    1) Valid Hex Byte.
 -------------------------------------------------------------------------------*/
void utest__byteToAscii( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    uint8 msn;
    uint8 lsn;

    TEST_START("Test Case: byteToAscii() - Valid Hex Byte.");
    byteToAscii( 0xA7, &msn, &lsn);
    ASSERT_EQ('A', msn);
    ASSERT_EQ('7', lsn);
    TEST_END

}  // utest__byteToAscii()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: frameToTimeCode()
 |
 | TEST CASES:
 |    1) Valid Frame and Framerate
 -------------------------------------------------------------------------------*/
void utest__frameToTimeCode( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    CaptionTime captionTime;

    TEST_START("Test Case: frameToTimeCode() - Valid Frame and Framerate.");
    frameToTimeCode( 455280, 2400, &captionTime );
    ASSERT_EQ(5, captionTime.hour);
    ASSERT_EQ(16, captionTime.minute);
    ASSERT_EQ(10, captionTime.second);
    ASSERT_EQ(0, captionTime.frame);
    ASSERT_EQ(2400, captionTime.frameRatePerSecTimesOneHundred);
    TEST_END

}  // utest__frameToTimeCode()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: timeCodeToFrame()
 |
 | TEST CASES:
 |    1) Valid Timecode
 -------------------------------------------------------------------------------*/
void utest__timeCodeToFrame( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    CaptionTime captionTime;
    uint32 retval;

    TEST_START("Test Case: timeCodeToFrame() - Valid Timecode.");
    captionTime.hour = 5;
    captionTime.minute = 16;
    captionTime.second = 10;
    captionTime.frame = 0;
    captionTime.frameRatePerSecTimesOneHundred = 2400;
    retval = timeCodeToFrame( &captionTime );
    ASSERT_EQ(455280, retval);
    TEST_END

}  // utest__timeCodeToFrame()

/*----------------------------------------------------------------------------*/
/*--                             Test Suite                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | TESTED FUNCTIONS:
 |    encodeTimeCode()
 |    decodeTimeCode()
 |    isHexByteValid()
 |    byteFromNibbles()
 |    byteToAscii()
 |    frameToTimeCode()
 |    timeCodeToFrame()
 |
 | UNTESTED FUNCTIONS:
 |    printCaptionsLine()             --  Used only for Debug
 |    numCcConstructsFromFramerate()  --  Too simple to test
 |    cdpFramerateFromFramerate()     --  Too simple to test
 -------------------------------------------------------------------------------*/
int main( int argc, char* argv[] ) {
    INIT_TEST_FRAMEWORK( argc, argv )

    TEST_SUITE_START("Test Suite: cc_utils.c -- encodeTimeCode()");
    utest__encodeTimeCode( &tmpNumSuccessfulTests, &tmpNumFailedTests );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: cc_utils.c -- decodeTimeCode()");
    utest__decodeTimeCode( &tmpNumSuccessfulTests, &tmpNumFailedTests );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: cc_utils.c -- isHexByteValid()");
    utest__isHexByteValid( &tmpNumSuccessfulTests, &tmpNumFailedTests );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: cc_utils.c -- byteFromNibbles()");
    utest__byteFromNibbles( &tmpNumSuccessfulTests, &tmpNumFailedTests );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: cc_utils.c -- byteToAscii()");
    utest__byteToAscii( &tmpNumSuccessfulTests, &tmpNumFailedTests );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: cc_utils.c -- frameToTimeCode()");
    utest__frameToTimeCode( &tmpNumSuccessfulTests, &tmpNumFailedTests );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: cc_utils.c -- timeCodeToFrame()");
    utest__timeCodeToFrame( &tmpNumSuccessfulTests, &tmpNumFailedTests );
    TEST_SUITE_END

    SHUTDOWN_TEST_FRAMEWORK
}  // main()
