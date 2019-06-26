//
// Created by Rob Taylor on 2019-04-04.
//

#include "test_engine.h"
#include "../src/utils/output_utils.c"

/*----------------------------------------------------------------------------*/
/*--                             Test Cases                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: uint8toBitArray()
 |
 | TEST CASES:
 |    1) Valid Hex Byte.
 |    2) Another Valid Hex Byte.
 |    3) Smallest Hex Byte.
 |    4) Largest Hex Byte.
 -------------------------------------------------------------------------------*/
void utest__uint8toBitArray( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    char* retval;

    TEST_START("Test Case: uint8toBitArray() - Valid Hex Byte.");
    retval = (char*)uint8toBitArray( 0xBE );
    ASSERT_STREQ("10111110", retval);
    TEST_END

    TEST_START("Test Case: uint8toBitArray() - Another Valid Hex Byte.");
    retval = (char*)uint8toBitArray( 0x5C );
    ASSERT_STREQ("01011100", retval);
    TEST_END

    TEST_START("Test Case: uint8toBitArray() - Smallest Hex Byte.");
    retval = (char*)uint8toBitArray( 0x00 );
    ASSERT_STREQ("00000000", retval);
    TEST_END

    TEST_START("Test Case: uint8toBitArray() - Largest Hex Byte.");
    retval = (char*)uint8toBitArray( 0xFF );
    ASSERT_STREQ("11111111", retval);
    TEST_END
}  // utest__uint8toBitArray()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: buildOutputPath()
 |
 | TEST CASES:
 |     1) No Output Directory. Input in working.
 |     2) No Output Directory. Input below working.
 |     3) No Output Directory. Input two below working.
 |     4) No Output Directory. Absolute Path to Input One Deep.
 |     5) No Output Directory. Absolute Path to Input Two Deep.
 |     6) Output Directory '.' Input in working.
 |     7) Output Directory '.' Input below working.
 |     8) Output Directory '.' Input two below working.
 |     9) Output Directory '.' Absolute Path to Input One Deep.
 |    10) Output Directory '.' Absolute Path to Input Two Deep.
 |    11) Output Directory Below Working. Input in working.
 |    12) Output Directory Below Working. Input below working.
 |    13) Output Directory Below Working. Input two below working.
 |    14) Output Directory Below Working. Absolute Path to Input One Deep.
 |    15) Output Directory Below Working. Absolute Path to Input Two Deep.
 |    16) Output Directory Below Working (no slash). Input in working.
 |    17) Output Directory Below Working (no slash). Input below working.
 |    18) Output Directory Below Working (no slash). Input two below working.
 |    19) Output Directory Below Working (no slash). Absolute Path to Input One Deep.
 |    20) Output Directory Below Working (no slash). Absolute Path to Input Two Deep.
 |    21) Output Directory Two Below Working. Input in working.
 |    22) Output Directory Two Below Working. Input below working.
 |    23) Output Directory Two Below Working. Input two below working.
 |    24) Output Directory Two Below Working. Absolute Path to Input One Deep.
 |    25) Output Directory Two Below Working. Absolute Path to Input Two Deep.
 |    26) Output Directory Two Below Working (no slash). Input in working.
 |    27) Output Directory Two Below Working (no slash). Input below working.
 |    28) Output Directory Two Below Working (no slash). Input two below working.
 |    29) Output Directory Two Below Working (no slash). Absolute Path to Input One Deep.
 |    30) Output Directory Two Below Working (no slash). Absolute Path to Input Two Deep.
 |    31) Output Directory Absolute Path One Deep. Input in working.
 |    32) Output Directory Absolute Path One Deep. Input below working.
 |    33) Output Directory Absolute Path One Deep. Input two below working.
 |    34) Output Directory Absolute Path One Deep. Absolute Path to Input One Deep.
 |    35) Output Directory Absolute Path One Deep. Absolute Path to Input Two Deep.
 |    36) Output Directory Absolute Path Two Deep. Input in working.
 |    37) Output Directory Absolute Path Two Deep. Input below working.
 |    38) Output Directory Absolute Path Two Deep. Input two below working.
 |    39) Output Directory Absolute Path Two Deep. Absolute Path to Input One Deep.
 |    40) Output Directory Absolute Path Two Deep. Absolute Path to Input Two Deep.
 |    41) Output Directory has a super long name that is not too long. Input in working.
 |    42) Output Directory has a super long name that is too long. Input in working.
 -------------------------------------------------------------------------------*/
void utest__buildOutputPath( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    char outputPath[MAX_FILE_NAME_LEN];
    char tmpStr[MAX_FILE_NAME_LEN];

    TEST_START("Test Case: buildOutputPath() - No Output Directory. Input in working.");
    tmpStr[0] = '\0';
    buildOutputPath( "filename.mpg", tmpStr, outputPath );
    ASSERT_STREQ("filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - No Output Directory. Input below working.");
    tmpStr[0] = '\0';
    buildOutputPath( "path/filename.mpg", tmpStr, outputPath );
    ASSERT_STREQ("path/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - No Output Directory. Input two below working.");
    tmpStr[0] = '\0';
    buildOutputPath( "path/two/filename.mpg", tmpStr, outputPath );
    ASSERT_STREQ("path/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - No Output Directory. Absolute Path to Input One Deep.");
    tmpStr[0] = '\0';
    buildOutputPath( "/path/filename.mpg", tmpStr, outputPath );
    ASSERT_STREQ("/path/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - No Output Directory. Absolute Path to Input Two Deep.");
    tmpStr[0] = '\0';
    buildOutputPath( "/path/two/filename.mpg", tmpStr, outputPath );
    ASSERT_STREQ("/path/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory '.' Input in working.");
    buildOutputPath( "filename.mpg", ".", outputPath );
    ASSERT_STREQ("./filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory '.' Input below working.");
    buildOutputPath( "path/filename.mpg", ".", outputPath );
    ASSERT_STREQ("./filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory '.' Input two below working.");
    buildOutputPath( "path/two/filename.mpg", ".", outputPath );
    ASSERT_STREQ("./filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory '.' Absolute Path to Input One Deep.");
    buildOutputPath( "/path/filename.mpg", ".", outputPath );
    ASSERT_STREQ("./filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory '.' Absolute Path to Input Two Deep.");
    buildOutputPath( "/path/two/filename.mpg", ".", outputPath );
    ASSERT_STREQ("./filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Below Working. Input in working.");
    buildOutputPath( "filename.mpg", "outpath/", outputPath );
    ASSERT_STREQ("outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Below Working. Input below working.");
    buildOutputPath( "path/filename.mpg", "outpath/", outputPath );
    ASSERT_STREQ("outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Below Working. Input two below working.");
    buildOutputPath( "path/two/filename.mpg", "outpath/", outputPath );
    ASSERT_STREQ("outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Below Working. Absolute Path to Input One Deep.");
    buildOutputPath( "/path/filename.mpg", "outpath/", outputPath );
    ASSERT_STREQ("outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Below Working. Absolute Path to Input Two Deep.");
    buildOutputPath( "/path/two/filename.mpg", "outpath/", outputPath );
    ASSERT_STREQ("outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Below Working (no slash). Input in working.");
    buildOutputPath( "filename.mpg", "outpath", outputPath );
    ASSERT_STREQ("outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Below Working (no slash). Input below working.");
    buildOutputPath( "path/filename.mpg", "outpath", outputPath );
    ASSERT_STREQ("outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Below Working (no slash). Input two below working.");
    buildOutputPath( "path/two/filename.mpg", "outpath", outputPath );
    ASSERT_STREQ("outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Below Working (no slash). Absolute Path to Input One Deep.");
    buildOutputPath( "/path/filename.mpg", "outpath", outputPath );
    ASSERT_STREQ("outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Below Working (no slash). Absolute Path to Input Two Deep.");
    buildOutputPath( "/path/two/filename.mpg", "outpath", outputPath );
    ASSERT_STREQ("outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Two Below Working. Input in working.");
    buildOutputPath( "filename.mpg", "outpath/two/", outputPath );
    ASSERT_STREQ("outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Two Below Working. Input below working.");
    buildOutputPath( "path/filename.mpg", "outpath/two/", outputPath );
    ASSERT_STREQ("outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Two Below Working. Input two below working.");
    buildOutputPath( "path/two/filename.mpg", "outpath/two/", outputPath );
    ASSERT_STREQ("outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Two Below Working. Absolute Path to Input One Deep.");
    buildOutputPath( "/path/filename.mpg", "outpath/two/", outputPath );
    ASSERT_STREQ("outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Two Below Working. Absolute Path to Input Two Deep.");
    buildOutputPath( "/path/two/filename.mpg", "outpath/two/", outputPath );
    ASSERT_STREQ("outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Two Below Working (no slash). Input in working.");
    buildOutputPath( "filename.mpg", "outpath/two", outputPath );
    ASSERT_STREQ("outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Two Below Working (no slash). Input below working.");
    buildOutputPath( "path/filename.mpg", "outpath/two", outputPath );
    ASSERT_STREQ("outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Two Below Working (no slash). Input two below working.");
    buildOutputPath( "path/two/filename.mpg", "outpath/two", outputPath );
    ASSERT_STREQ("outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Two Below Working (no slash). Absolute Path to Input One Deep.");
    buildOutputPath( "/path/filename.mpg", "outpath/two", outputPath );
    ASSERT_STREQ("outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory Two Below Working (no slash). Absolute Path to Input Two Deep.");
    buildOutputPath( "/path/two/filename.mpg", "outpath/two", outputPath );
    ASSERT_STREQ("outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Absolute Path One Deep. Input in working.");
    buildOutputPath( "filename.mpg", "/outpath/", outputPath );
    ASSERT_STREQ("/outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Absolute Path One Deep. Input below working.");
    buildOutputPath( "path/filename.mpg", "/outpath/", outputPath );
    ASSERT_STREQ("/outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Absolute Path One Deep. Input two below working.");
    buildOutputPath( "path/two/filename.mpg", "/outpath/", outputPath );
    ASSERT_STREQ("/outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Absolute Path One Deep. Absolute Path to Input One Deep.");
    buildOutputPath( "/path/filename.mpg", "/outpath/", outputPath );
    ASSERT_STREQ("/outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Absolute Path One Deep. Absolute Path to Input Two Deep.");
    buildOutputPath( "/path/two/filename.mpg", "/outpath/", outputPath );
    ASSERT_STREQ("/outpath/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Absolute Path Two Deep. Input in working.");
    buildOutputPath( "filename.mpg", "/outpath/two/", outputPath );
    ASSERT_STREQ("/outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Absolute Path Two Deep. Input below working.");
    buildOutputPath( "path/filename.mpg", "/outpath/two/", outputPath );
    ASSERT_STREQ("/outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Absolute Path Two Deep. Input two below working.");
    buildOutputPath( "path/two/filename.mpg", "/outpath/two/", outputPath );
    ASSERT_STREQ("/outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Absolute Path Two Deep. Absolute Path to Input One Deep.");
    buildOutputPath( "/path/filename.mpg", "/outpath/two/", outputPath );
    ASSERT_STREQ("/outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Absolute Path Two Deep. Absolute Path to Input Two Deep.");
    buildOutputPath( "/path/two/filename.mpg", "/outpath/two/", outputPath );
    ASSERT_STREQ("/outpath/two/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory has a super long name that is not too long. Input in working.");
    buildOutputPath( "filename.mpg", "/outpath/is/incredibly/long/probably/not/a/full/256/bytes/long/but/certainly/longer/than/anything/that/is/reasonable/or/is/likely/to/come/up/in/production/", outputPath );
    ASSERT_STREQ("/outpath/is/incredibly/long/probably/not/a/full/256/bytes/long/but/certainly/longer/than/anything/that/is/reasonable/or/is/likely/to/come/up/in/production/filename", outputPath);
    TEST_END

    TEST_START("Test Case: buildOutputPath() - Output Directory has a super long name that is long. Input in working.");
    buildOutputPath( "filename.mpg", "/outpath/is/incredibly/long/probably/not/a/full/256/bytes/long/but/certainly/longer/than/anything/that/is/reasonable/or/is/likely/to/come/up/in/production/ok/now/it/is/even/longer/this/would/cause/real/problems/if/it/happened/but/look/how/absurd/and/unlikely/it/is/", outputPath );
    ASSERT_STREQ("/outpath/is/incredibly/long/probably/not/a/full/256/bytes/long/but/certainly/longer/than/anything/that/is/reasonable/or/is/likely/to/come/up/in/production/ok/now/it/is/even/longer/this/would/cause/real/problems/if/it/happened/but/look/how/abs/filename", outputPath);
    TEST_END
}  // utest__buildOutputPath()

/*----------------------------------------------------------------------------*/
/*--                             Test Suite                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | TESTED FUNCTIONS:
 |    uint8toBitArray()
 |    buildOutputPath()
 |
 | UNTESTED FUNCTIONS:
 |    fileOutputInit()             --  Tested with Integration Tests
 |    writeToFile()                --  Tested with Integration Tests
 |    closeFile()                  --  Tested with Integration Tests
 |    closeAllFiles()              --  Too simple to test
 |    areAnyFilesOpen()            --  Too simple to test
 |    writeHexLine()               --  Currently Unused
 -------------------------------------------------------------------------------*/
int main( int argc, char* argv[] ) {
    INIT_TEST_FRAMEWORK( argc, argv )

    TEST_SUITE_START("Test Suite: output_utils.c -- uint8toBitArray()");
    utest__uint8toBitArray( &tmpNumSuccessfulTests, &tmpNumFailedTests );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: output_utils.c -- buildOutputPath()");
    utest__buildOutputPath( &tmpNumSuccessfulTests, &tmpNumFailedTests );
    TEST_SUITE_END

    SHUTDOWN_TEST_FRAMEWORK
}  // main()
