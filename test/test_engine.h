//
//  test_engine.h
//  Caption Converter Testing Driver File
//
//  Created by Rob Taylor on 4/9/19.
//

#ifndef test_engine_h
#define test_engine_h

#include <stdio.h>
#include <setjmp.h>

#include "types.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

#define OUTPUT_XML_FILE_NAME_LEN                          100

#define NO_LONG_JUMP_OCCURRED                               0
#define EXPECTED_FATAL_ERROR_LONG_JUMP                      1
#define UNEXPECTED_FATAL_ERROR_LONG_JUMP                    2
#define EXPECTED_RAISED_SIGNAL_LONG_JUMP                    3
#define UNEXPECTED_RAISED_SIGNAL_LONG_JUMP                  4

#define TEST_DEBUG_LEVEL_INFO            (MAX_DEBUG_LEVEL + 1)
#define TEST_DEBUG_LEVEL_ERROR           (MAX_DEBUG_LEVEL + 2)
#define MAX_TEST_DEBUG_LEVEL                                3
#define TEST_SECTION                                        0

/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                                Types                                   --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

#define TEST_ASSERT(x) if(!(x)) DebugLog(TEST_DEBUG_LEVEL_ERROR, DBG_GENERAL, __FILE__, __LINE__, "ASSERT FAILED IN TEST CODE!")

#define ERROR_EXPECTED ExpectError(1);
#define ERRORS_EXPECTED(num) ExpectError(num);
#define FATAL_ERROR_EXPECTED ExpectFatalError();

#define INIT_TEST_FRAMEWORK(argc, argv) uint16 tmpNumSuccessfulTests; uint16 tmpNumFailedTests; \
                                        uint16 totalNumSuccessfulTests = 0; uint16 totalNumFailedTests = 0; InitTestFramework(argc, argv);
#define SHUTDOWN_TEST_FRAMEWORK ShutdownTestFramework(); printf("Total: Succeeded - %d - Failed - %d\n", totalNumSuccessfulTests, totalNumFailedTests); return totalNumFailedTests;

#define TEST_SUITE_START(name) tmpNumSuccessfulTests = 0; tmpNumFailedTests = 0; printf("Running - %s", name); \
                               WriteToOutputFile("    <testsuite name=\"%s\">\n", name);
#define TEST_SUITE_PASSED_ARGUMENTS &tmpNumSuccessfulTests, &tmpNumFailedTests
#define TEST_SUITE_RECEIVED_ARGUMENTS uint16* numSuccessfulPtr, uint16* numFailedPtr
#define TEST_SUITE_END printf("  ---  Passed - %d Failed - %d\n", tmpNumSuccessfulTests, tmpNumFailedTests); \
                       totalNumSuccessfulTests = totalNumSuccessfulTests + tmpNumSuccessfulTests; \
                       totalNumFailedTests = totalNumFailedTests + tmpNumFailedTests; \
                       tmpNumSuccessfulTests = tmpNumFailedTests = 0; WriteToOutputFile("    </testsuite>\n");

#define TEST_INITIALIZE  boolean isSuccessful;
#define TEST_START(name) WriteToOutputFile("        <testcase name=\"%s\"", name); isSuccessful = TRUE; \
                         jmpBufNum = setjmp(jmpBuf); if( jmpBufNum == NO_LONG_JUMP_OCCURRED ) {
#define TEST_END } \
                 if( CheckError(jmpBufNum) == FALSE ) isSuccessful = FALSE; \
                 if( isSuccessful == TRUE ) (*numSuccessfulPtr)++; \
                 else (*numFailedPtr)++; LogTestCase( isSuccessful );

#define ASSERT_STREQ(expected, actual) if( AssertStringEqual(__FILE__, __LINE__, expected, actual) == FALSE) isSuccessful = FALSE
#define ASSERT_PTREQ(expected, actual) if( AssertPointerEqual(__FILE__, __LINE__, expected, actual) == FALSE) isSuccessful = FALSE
#define ASSERT_STRUCTEQ(expected, actual, size) if( AssertStructEqual(__FILE__, __LINE__, expected, actual, size) == FALSE) isSuccessful = FALSE
#define ASSERT_EQ(expected, actual) if( AssertEqual(__FILE__, __LINE__, expected, actual) == FALSE) isSuccessful = FALSE
#define ASSERT_NEQ(expected, actual) if( AssertNotEqual(__FILE__, __LINE__, expected, actual) == FALSE) isSuccessful = FALSE

#define ASSERT_STREQ_MSG(expected, actual, ...) if( AssertStringEqualMsg(__FILE__, __LINE__, expected, actual, __VA_ARGS__) == FALSE) isSuccessful = FALSE
#define ASSERT_PTREQ_MSG(expected, actual, ...) if( AssertPointerEqualMsg(__FILE__, __LINE__, expected, actual, __VA_ARGS__) == FALSE) isSuccessful = FALSE
#define ASSERT_STRUCTEQ_MSG(expected, actual, size, ...) if( AssertStructEqualMsg(__FILE__, __LINE__, expected, actual, size, __VA_ARGS__) == FALSE) isSuccessful = FALSE
#define ASSERT_EQ_MSG(expected, actual, ...) if( AssertEqualMsg(__FILE__, __LINE__, expected, actual, __VA_ARGS__) == FALSE) isSuccessful = FALSE
#define ASSERT_NEQ_MSG(expected, actual, ...) if( AssertNotEqualMsg(__FILE__, __LINE__, expected, actual, __VA_ARGS__) == FALSE) isSuccessful = FALSE

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/

extern jmp_buf jmpBuf;
extern int jmpBufNum;

extern int8 errorExpected;
extern boolean errorReceived;
extern boolean fatalErrorExpected;

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/

void HandleSignal( int );
void ExpectError( uint8 );
void ExpectFatalError( void );
boolean CheckError( int );
void LogTestCase( boolean );
void DebugLogHandler( char*, int, uint8, uint8, char* );

void InitTestFramework( int argc, char* argv[] );
void ShutdownTestFramework( void );

void CreateOutputFile( const char* );
void WriteToOutputFile( char*, ... );
void WriteToOutputBuffer( char*, ... );
void FlushBufferToFile( void );
void CloseOutputFile( void );

boolean AssertStringEqual( char*, int, char*, char* );
boolean AssertPointerEqual( char*, int, void*, void* );
boolean AssertStructEqual( char*, int, void*, void*, size_t );
boolean AssertEqual( char*, int, uint64, uint64 );
boolean AssertNotEqual( char*, int, uint64, uint64 );

boolean AssertStringEqualMsg( char*, int, char*, char*, ... );
boolean AssertPointerEqualMsg( char*, int, void*, void*, ... );
boolean AssertStructEqualMsg( char*, int, void*, void*, size_t, ... );
boolean AssertEqualMsg( char*, int, uint64, uint64, ... );
boolean AssertNotEqualMsg( char*, int, uint64, uint64, ... );

#endif /* test_engine_h */
