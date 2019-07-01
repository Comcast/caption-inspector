//
// Created by Rob Taylor on 2019-04-08.
//

#include "test_engine.h"

#define _STDLIB_H_
#define _STDLIB_H
static void* malloc( uint64 );
static void free( void* );

#include "../src/utils/buffer_utils.c"

/*----------------------------------------------------------------------------*/
/*--                          Support for Stubs                             --*/
/*----------------------------------------------------------------------------*/

#define STUB_NUM_ELEMENTS         35
#define STUB_BUFFER_MEMORY        50

typedef struct {
    boolean allocated;
    uint32 size;
    uint8 memory[STUB_BUFFER_MEMORY];
} MallocStubElement;

static boolean anyMallocErrors = FALSE;
static boolean failNextMalloc = FALSE;
static MallocStubElement mallocStubElementArray[STUB_NUM_ELEMENTS];

/*----------------------------------------------------------------------------*/
/*--                           Stub Functions                               --*/
/*----------------------------------------------------------------------------*/

static void* malloc( uint64 size ) {
    if( failNextMalloc == TRUE ) {
        failNextMalloc = FALSE;
        return 0;
    }
    for( int loop = 0; loop < STUB_NUM_ELEMENTS; loop++ ) {
        if( mallocStubElementArray[loop].allocated == FALSE ) {
            mallocStubElementArray[loop].allocated = TRUE;
            mallocStubElementArray[loop].size = size;
            return mallocStubElementArray[loop].memory;
        }
    }
    anyMallocErrors = TRUE;
    return NULL;
} // Stub: malloc()

static void free( void* ptr ) {
    for( int loop = 0; loop < STUB_NUM_ELEMENTS; loop++ ) {
        if( mallocStubElementArray[loop].memory == ptr ) {
            if( mallocStubElementArray[loop].allocated == FALSE ) {
                anyMallocErrors = TRUE;
            }
            mallocStubElementArray[loop].allocated = FALSE;
            mallocStubElementArray[loop].size = 0;
            for( int loopTwo = 0; loopTwo < STUB_BUFFER_MEMORY; loopTwo++ ) {
                mallocStubElementArray[loop].memory[loopTwo] = 0;
            }
            return;
        }
    }
    anyMallocErrors = TRUE;
} // Stub: free()

static void stubInitMallocStubElems( void ) {
    anyMallocErrors = FALSE;
    for( int loop = 0; loop < STUB_NUM_ELEMENTS; loop++ ) {
        mallocStubElementArray[loop].allocated = FALSE;
        mallocStubElementArray[loop].size = 0;
        for( int loopTwo = 0; loopTwo < STUB_BUFFER_MEMORY; loopTwo++ ) {
            mallocStubElementArray[loop].memory[loopTwo] = 0;
        }
    }
} // Stub Helper: stubInitMallocStubElems()

static MallocStubElement* stubGetMallocStubElem( void* ptr ) {
    for( int loop = 0; loop < STUB_NUM_ELEMENTS; loop++ ) {
        if( mallocStubElementArray[loop].memory == ptr ) {
            return &mallocStubElementArray[loop];
        }
    }
    return NULL;
} // Stub Helper: stubGetMallocStubElem()

static uint8 stubCountMallocedElements( void ) {
    uint8 retval = 0;

    for( int loop = 0; loop < STUB_NUM_ELEMENTS; loop++ ) {
        if( mallocStubElementArray[loop].allocated == TRUE ) {
            retval++;
        }
    }
    return retval;
} // Stub Helper: stubCountMallocedElements()

/*----------------------------------------------------------------------------*/
/*--                             Test Cases                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: _NewBuffer()
 |
 | TEST CASES:
 |    1) Allocate a Buffer of Bytes.
 |    2) Allocate a Buffer of Line 21 Data.
 |    3) Allocate a Buffer of DTVCC Data.
 |    4) Allocate a Buffer of Invalid Type.
 |    5) Allocate a Buffer with Zero Length.
 |    6) Allocate three Buffers.
 |    7) Allocate too many Buffers.
 |    8) Fail a malloc call.
 -------------------------------------------------------------------------------*/
void utest__NewBuffer( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    MallocStubElement* mallocedElemPtr;
    Buffer* buffPtr;
    Buffer* buffTwoPtr;
    Buffer* buffThreePtr;

    TEST_START("Test Case: _NewBuffer() - Allocate a Buffer of Bytes.");
    stubInitMallocStubElems();
    BufferPoolInit();
    ASSERT_EQ(0, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_BYTES, 123);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(0, buffPtr->captionTime.hour);
    ASSERT_EQ(0, buffPtr->captionTime.minute);
    ASSERT_EQ(0, buffPtr->captionTime.second);
    ASSERT_EQ(0, buffPtr->captionTime.millisecond);
    ASSERT_EQ(0, buffPtr->captionTime.frame);
    ASSERT_EQ(0, buffPtr->captionTime.frameRatePerSecTimesOneHundred);
    ASSERT_EQ(0, buffPtr->numElements);
    ASSERT_EQ(123, buffPtr->maxNumElements);
    ASSERT_EQ(3, stubCountMallocedElements());
    mallocedElemPtr = stubGetMallocStubElem(buffPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ASSERT_EQ(sizeof(Buffer), mallocedElemPtr->size);
    mallocedElemPtr = stubGetMallocStubElem(buffPtr->dataPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ASSERT_EQ(123, mallocedElemPtr->size);
    FreeBuffer(buffPtr);
    ASSERT_EQ(0, stubCountMallocedElements());
    ASSERT_EQ(FALSE, anyMallocErrors);
    TEST_END

    TEST_START("Test Case: _NewBuffer() - Allocate a Buffer of Line 21 Data.");
    stubInitMallocStubElems();
    BufferPoolInit();
    ASSERT_EQ(0, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_LINE_21, 13);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(BUFFER_TYPE_LINE_21, buffPtr->bufferType);
    ASSERT_EQ(0, buffPtr->captionTime.hour);
    ASSERT_EQ(0, buffPtr->captionTime.minute);
    ASSERT_EQ(0, buffPtr->captionTime.second);
    ASSERT_EQ(0, buffPtr->captionTime.millisecond);
    ASSERT_EQ(0, buffPtr->captionTime.frame);
    ASSERT_EQ(0, buffPtr->captionTime.frameRatePerSecTimesOneHundred);
    ASSERT_EQ(0, buffPtr->numElements);
    ASSERT_EQ(13, buffPtr->maxNumElements);
    ASSERT_EQ(3, stubCountMallocedElements());
    mallocedElemPtr = stubGetMallocStubElem(buffPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ASSERT_EQ(sizeof(Buffer), mallocedElemPtr->size);
    mallocedElemPtr = stubGetMallocStubElem(buffPtr->dataPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ASSERT_EQ(13 * sizeof(Line21Code), mallocedElemPtr->size);
    FreeBuffer(buffPtr);
    ASSERT_EQ(0, stubCountMallocedElements());
    ASSERT_EQ(FALSE, anyMallocErrors);
    TEST_END

    TEST_START("Test Case: _NewBuffer() - Allocate a Buffer of DTVCC Data.");
    stubInitMallocStubElems();
    BufferPoolInit();
    ASSERT_EQ(0, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_DTVCC, 45);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(BUFFER_TYPE_DTVCC, buffPtr->bufferType);
    ASSERT_EQ(0, buffPtr->captionTime.hour);
    ASSERT_EQ(0, buffPtr->captionTime.minute);
    ASSERT_EQ(0, buffPtr->captionTime.second);
    ASSERT_EQ(0, buffPtr->captionTime.millisecond);
    ASSERT_EQ(0, buffPtr->captionTime.frame);
    ASSERT_EQ(0, buffPtr->captionTime.frameRatePerSecTimesOneHundred);
    ASSERT_EQ(0, buffPtr->numElements);
    ASSERT_EQ(45, buffPtr->maxNumElements);
    ASSERT_EQ(3, stubCountMallocedElements());
    mallocedElemPtr = stubGetMallocStubElem(buffPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ASSERT_EQ(sizeof(Buffer), mallocedElemPtr->size);
    mallocedElemPtr = stubGetMallocStubElem(buffPtr->dataPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ASSERT_EQ(45 * sizeof(DtvccData), mallocedElemPtr->size);
    FreeBuffer(buffPtr);
    ASSERT_EQ(0, stubCountMallocedElements());
    ASSERT_EQ(FALSE, anyMallocErrors);
    TEST_END

    TEST_START("Test Case: _NewBuffer() - Allocate a Buffer of Invalid Type.");
    stubInitMallocStubElems();
    BufferPoolInit();
    ASSERT_EQ(0, stubCountMallocedElements());
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    buffPtr = _NewBuffer("filename", 42, 0, 45);
    TEST_END

    TEST_START("Test Case: _NewBuffer() - Allocate a Buffer with Zero Length.");
    stubInitMallocStubElems();
    BufferPoolInit();
    ASSERT_EQ(0, stubCountMallocedElements());
    FATAL_ERROR_EXPECTED
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_DTVCC, 0);
    TEST_END

    TEST_START("Test Case: _NewBuffer() - Allocate three Buffers.");
    stubInitMallocStubElems();
    BufferPoolInit();
    ASSERT_EQ(0, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_BYTES, 321);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(0, buffPtr->captionTime.hour);
    ASSERT_EQ(0, buffPtr->captionTime.minute);
    ASSERT_EQ(0, buffPtr->captionTime.second);
    ASSERT_EQ(0, buffPtr->captionTime.millisecond);
    ASSERT_EQ(0, buffPtr->captionTime.frame);
    ASSERT_EQ(0, buffPtr->captionTime.frameRatePerSecTimesOneHundred);
    ASSERT_EQ(0, buffPtr->numElements);
    ASSERT_EQ(321, buffPtr->maxNumElements);
    ASSERT_EQ(3, stubCountMallocedElements());
    mallocedElemPtr = stubGetMallocStubElem(buffPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ASSERT_EQ(sizeof(Buffer), mallocedElemPtr->size);
    mallocedElemPtr = stubGetMallocStubElem(buffPtr->dataPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ASSERT_EQ(321, mallocedElemPtr->size);
    ASSERT_EQ(3, stubCountMallocedElements());
    buffTwoPtr = _NewBuffer("filename", 42, BUFFER_TYPE_LINE_21, 31);
    ASSERT_NEQ((uint64)NULL, (uint64)buffTwoPtr);
    ASSERT_EQ(BUFFER_TYPE_LINE_21, buffTwoPtr->bufferType);
    ASSERT_EQ(0, buffTwoPtr->captionTime.hour);
    ASSERT_EQ(0, buffTwoPtr->captionTime.minute);
    ASSERT_EQ(0, buffTwoPtr->captionTime.second);
    ASSERT_EQ(0, buffTwoPtr->captionTime.millisecond);
    ASSERT_EQ(0, buffTwoPtr->captionTime.frame);
    ASSERT_EQ(0, buffTwoPtr->captionTime.frameRatePerSecTimesOneHundred);
    ASSERT_EQ(0, buffTwoPtr->numElements);
    ASSERT_EQ(31, buffTwoPtr->maxNumElements);
    ASSERT_EQ(6, stubCountMallocedElements());
    mallocedElemPtr = stubGetMallocStubElem(buffTwoPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ASSERT_EQ(sizeof(Buffer), mallocedElemPtr->size);
    mallocedElemPtr = stubGetMallocStubElem(buffTwoPtr->dataPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ASSERT_EQ(31 * sizeof(Line21Code), mallocedElemPtr->size);
    ASSERT_EQ(6, stubCountMallocedElements());
    buffThreePtr = _NewBuffer("filename", 42, BUFFER_TYPE_DTVCC, 54);
    ASSERT_NEQ((uint64)NULL, (uint64)buffThreePtr);
    ASSERT_EQ(BUFFER_TYPE_DTVCC, buffThreePtr->bufferType);
    ASSERT_EQ(0, buffThreePtr->captionTime.hour);
    ASSERT_EQ(0, buffThreePtr->captionTime.minute);
    ASSERT_EQ(0, buffThreePtr->captionTime.second);
    ASSERT_EQ(0, buffThreePtr->captionTime.millisecond);
    ASSERT_EQ(0, buffThreePtr->captionTime.frame);
    ASSERT_EQ(0, buffThreePtr->captionTime.frameRatePerSecTimesOneHundred);
    ASSERT_EQ(0, buffThreePtr->numElements);
    ASSERT_EQ(54, buffThreePtr->maxNumElements);
    ASSERT_EQ(9, stubCountMallocedElements());
    mallocedElemPtr = stubGetMallocStubElem(buffThreePtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ASSERT_EQ(sizeof(Buffer), mallocedElemPtr->size);
    mallocedElemPtr = stubGetMallocStubElem(buffThreePtr->dataPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ASSERT_EQ(54 * sizeof(DtvccData), mallocedElemPtr->size);
    FreeBuffer(buffPtr);
    ASSERT_EQ(6, stubCountMallocedElements());
    FreeBuffer(buffTwoPtr);
    ASSERT_EQ(3, stubCountMallocedElements());
    FreeBuffer(buffThreePtr);
    ASSERT_EQ(0, stubCountMallocedElements());
    ASSERT_EQ(FALSE, anyMallocErrors);
    TEST_END

    TEST_START("Test Case: _NewBuffer() - Allocate too many Buffers.");
    stubInitMallocStubElems();
    BufferPoolInit();
    ASSERT_EQ(0, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_BYTES, 567);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(3, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_LINE_21, 89);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(6, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_DTVCC, 10);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(9, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_BYTES, 437);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(12, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_LINE_21, 82);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(15, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_DTVCC, 16);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(18, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_BYTES, 375);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(21, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_LINE_21, 22);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(24, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_DTVCC, 88);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(27, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_LINE_21, 33);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(30, stubCountMallocedElements());
    ERRORS_EXPECTED(61)
    FATAL_ERROR_EXPECTED
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_DTVCC, 55);
    ASSERT_EQ(FALSE, anyMallocErrors);
    TEST_END

    TEST_START("Test Case: _NewBuffer() - Fail a malloc call.");
    stubInitMallocStubElems();
    BufferPoolInit();
    ASSERT_EQ(0, stubCountMallocedElements());
    failNextMalloc = TRUE;
    FATAL_ERROR_EXPECTED
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_LINE_21, 13);
    ASSERT_EQ(FALSE, anyMallocErrors);
    TEST_END
}  // utest__NewBuffer()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: FreeBuffer()
 |
 | TEST CASES:
 |    1) Free a Buffer.
 |    2) Free an Invalid Buffer.
 |    3) Free a NULL Buffer.
 -------------------------------------------------------------------------------*/
void utest__FreeBuffer( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    Buffer* buffPtr;
    MallocStubElement* mallocedElemPtr;

    TEST_START("Test Case: FreeBuffer() - Free a Buffer.");
    stubInitMallocStubElems();
    BufferPoolInit();
    ASSERT_EQ(0, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_BYTES, 123);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(3, stubCountMallocedElements());
    mallocedElemPtr = stubGetMallocStubElem(buffPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    mallocedElemPtr = stubGetMallocStubElem(buffPtr->dataPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    FreeBuffer(buffPtr);
    ASSERT_EQ(0, stubCountMallocedElements());
    ASSERT_EQ(FALSE, anyMallocErrors);
    TEST_END

    TEST_START("Test Case: FreeBuffer() - Free an Invalid Buffer.");
    stubInitMallocStubElems();
    BufferPoolInit();
    ASSERT_EQ(0, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_BYTES, 123);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(3, stubCountMallocedElements());
    mallocedElemPtr = stubGetMallocStubElem(buffPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    mallocedElemPtr = stubGetMallocStubElem(buffPtr->dataPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ERROR_EXPECTED
    uint8 foo;
    FreeBuffer((Buffer*)&foo);
    ASSERT_EQ(3, stubCountMallocedElements());
    ASSERT_EQ(FALSE, anyMallocErrors);
    TEST_END

    TEST_START("Test Case: FreeBuffer() - Free a NULL Buffer.");
    stubInitMallocStubElems();
    BufferPoolInit();
    ASSERT_EQ(0, stubCountMallocedElements());
    buffPtr = _NewBuffer("filename", 42, BUFFER_TYPE_BYTES, 123);
    ASSERT_NEQ((uint64)NULL, (uint64)buffPtr);
    ASSERT_EQ(3, stubCountMallocedElements());
    mallocedElemPtr = stubGetMallocStubElem(buffPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    mallocedElemPtr = stubGetMallocStubElem(buffPtr->dataPtr);
    ASSERT_NEQ((uint64)NULL, (uint64)mallocedElemPtr);
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    FreeBuffer(NULL);
    ASSERT_EQ(3, stubCountMallocedElements());
    ASSERT_EQ(FALSE, anyMallocErrors);
    TEST_END
}  // utest__FreeBuffer()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: AddReader()
 |
 | TEST CASES:
 |    1) Add Reader to an Invalid Buffer/Pointer.
 |    2) Add Reader to a NULL Pointer.
 -------------------------------------------------------------------------------*/
void utest__AddReader( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    uint8 dummyAddress;

    TEST_START("Test Case: FreeBuffer() - Add Reader to an Invalid Buffer/Pointer.");
    ERROR_EXPECTED
    AddReader( (Buffer*)&dummyAddress );
    TEST_END

    TEST_START("Test Case: FreeBuffer() - Add Reader to a NULL Pointer.");
    FATAL_ERROR_EXPECTED
    AddReader( NULL );
    TEST_END
}  // utest__AddReader()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: CaptionTimeFromPts()
 |
 | TEST CASES:
 |    1) Valid Presentation Time Stamp.
 |    2) Smallest Presentation Time Stamp.
 |    3) Largest Valid Presentation Time Stamp.
 |    4) Largest Presentation Time Stamp.
 |    5) Negative Presentation Time Stamp.
 -------------------------------------------------------------------------------*/
void utest__CaptionTimeFromPts( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    CaptionTime captionTime;

    TEST_START("Test Case: CaptionTimeFromPts() - Valid Presentation Time Stamp.");
    CaptionTimeFromPts( &captionTime, 20645280 );
    ASSERT_EQ(20645280, (5 * (1000*60*60)) + (44 * (1000*60)) + (5 * 1000) + 280);
    ASSERT_EQ(5, captionTime.hour);
    ASSERT_EQ(44, captionTime.minute);
    ASSERT_EQ(5, captionTime.second);
    ASSERT_EQ(280, captionTime.millisecond);
    ASSERT_EQ(CAPTION_TIME_PTS_NUMBERING, captionTime.source);
    TEST_END

    TEST_START("Test Case: CaptionTimeFromPts() - Smallest Presentation Time Stamp.");
    CaptionTimeFromPts( &captionTime, 0 );
    ASSERT_EQ(0, captionTime.hour);
    ASSERT_EQ(0, captionTime.minute);
    ASSERT_EQ(0, captionTime.second);
    ASSERT_EQ(0, captionTime.millisecond);
    ASSERT_EQ(CAPTION_TIME_PTS_NUMBERING, captionTime.source);
    TEST_END

    TEST_START("Test Case: CaptionTimeFromPts() - Largest Valid Presentation Time Stamp.");
    CaptionTimeFromPts( &captionTime, 86399999 );
    ASSERT_EQ(86399999, (23 * (1000*60*60)) + (59 * (1000*60)) + (59 * 1000) + 999);
    ASSERT_EQ(59, captionTime.minute);
    ASSERT_EQ(59, captionTime.second);
    ASSERT_EQ(999, captionTime.millisecond);
    ASSERT_EQ(CAPTION_TIME_PTS_NUMBERING, captionTime.source);
    TEST_END

    TEST_START("Test Case: CaptionTimeFromPts() - Largest Presentation Time Stamp.");
    FATAL_ERROR_EXPECTED
    CaptionTimeFromPts( &captionTime, 0x7FFFFFFFFFFFFFFF );
    TEST_END

    TEST_START("Test Case: CaptionTimeFromPts() - Negative Presentation Time Stamp.");
    FATAL_ERROR_EXPECTED
    CaptionTimeFromPts( &captionTime, -1 );
    TEST_END
}  // utest__CaptionTimeFromPts()

/*----------------------------------------------------------------------------*/
/*--                             Test Suite                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | TESTED FUNCTIONS:
 |    _NewBuffer()
 |    FreeBuffer()
 |    AddReader()
 |    CaptionTimeFromPts()
 |
 | UNTESTED FUNCTIONS:
 |    BufferPoolInit()          -- Tested in Integration Testing
 |    NumAllocatedBuffers()     -- Tested in Integration Testing
 |    dumpBufferPool()          -- Untested Debug Function
 -------------------------------------------------------------------------------*/
int main( int argc, char* argv[] ) {
    INIT_TEST_FRAMEWORK( argc, argv )

    TEST_SUITE_START("Test Suite: buffer_utils.c -- _NewBuffer()");
    utest__NewBuffer( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: buffer_utils.c -- FreeBuffer()");
    utest__FreeBuffer( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: buffer_utils.c -- AddReader()");
    utest__AddReader( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: buffer_utils.c -- CaptionTimeFromPts()");
    utest__CaptionTimeFromPts( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    SHUTDOWN_TEST_FRAMEWORK
}  // main()

