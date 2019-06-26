//
// Created by Rob Taylor on 2019-04-09.
//

#include "test_engine.h"

#include "buffer_utils.h"

/*----------------------------------------------------------------------------*/
/*--                             Test Cases                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | INTEGRATION TESTS: Buffer Allocation/Free Tests
 |
 | TEST CASES:
 |     1) Allocate and free a Buffer with Zero Readers.
 |     2) Allocate and free a Buffer with One Reader.
 |     3) Allocate and free a Buffer with Two Readers.
 |     4) Allocate and free a Buffer with Three Readers.
 |     5) Allocate and free Two Buffers with One Reader.
 |     6) Allocate and free Two Buffers with Two Readers.
 |     7) Allocate and free Two Buffers with Three Readers.
 |     8) Allocate and free Three Buffers with One Reader.
 |     9) Allocate and free Three Buffers with Two Readers.
 |    10) Allocate and free Three Buffers with Three Readers.
 -------------------------------------------------------------------------------*/
void itest__BufferAllocFreeTests( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    Buffer* buffPtr;
    Buffer* buffTwoPtr;
    Buffer* buffThreePtr;

    TEST_START("Test Case: Sunny Day Buffer Tests - Allocate and free a Buffer with Zero Readers.");
    BufferPoolInit();
    ASSERT_EQ(0, NumAllocatedBuffers());
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Sunny Day Buffer Tests - Allocate and free a Buffer with One Reader.");
    BufferPoolInit();
    ASSERT_EQ(0, NumAllocatedBuffers());
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    AddReader(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Sunny Day Buffer Tests - Allocate and free a Buffer with Two Readers.");
    BufferPoolInit();
    ASSERT_EQ(0, NumAllocatedBuffers());
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    AddReader(buffPtr);
    AddReader(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Sunny Day Buffer Tests - Allocate and free a Buffer with Three Readers.");
    BufferPoolInit();
    ASSERT_EQ(0, NumAllocatedBuffers());
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    AddReader(buffPtr);
    AddReader(buffPtr);
    AddReader(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Sunny Day Buffer Tests - Allocate and free Two Buffers with One Reader.");
    BufferPoolInit();
    ASSERT_EQ(0, NumAllocatedBuffers());
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    AddReader(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    buffTwoPtr = NewBuffer(BUFFER_TYPE_DTVCC, 10);
    AddReader(buffTwoPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffTwoPtr);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Sunny Day Buffer Tests - Allocate and free Two Buffers with Two Readers.");
    BufferPoolInit();
    ASSERT_EQ(0, NumAllocatedBuffers());
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    AddReader(buffPtr);
    AddReader(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    buffTwoPtr = NewBuffer(BUFFER_TYPE_DTVCC, 10);
    AddReader(buffTwoPtr);
    AddReader(buffTwoPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffTwoPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffTwoPtr);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Sunny Day Buffer Tests - Allocate and free Two Buffers with Three Readers.");
    BufferPoolInit();
    ASSERT_EQ(0, NumAllocatedBuffers());
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    AddReader(buffPtr);
    AddReader(buffPtr);
    AddReader(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    buffTwoPtr = NewBuffer(BUFFER_TYPE_DTVCC, 10);
    AddReader(buffTwoPtr);
    AddReader(buffTwoPtr);
    AddReader(buffTwoPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffTwoPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffTwoPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffTwoPtr);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Sunny Day Buffer Tests - Allocate and free Three Buffers with One Reader.");
    BufferPoolInit();
    ASSERT_EQ(0, NumAllocatedBuffers());
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    AddReader(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    buffTwoPtr = NewBuffer(BUFFER_TYPE_DTVCC, 10);
    AddReader(buffTwoPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    buffThreePtr = NewBuffer(BUFFER_TYPE_LINE_21, 30);
    AddReader(buffThreePtr);
    ASSERT_EQ(3, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    FreeBuffer(buffTwoPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffThreePtr);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Sunny Day Buffer Tests - Allocate and free Three Buffers with Two Readers.");
    BufferPoolInit();
    ASSERT_EQ(0, NumAllocatedBuffers());
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    AddReader(buffPtr);
    AddReader(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    buffTwoPtr = NewBuffer(BUFFER_TYPE_DTVCC, 10);
    AddReader(buffTwoPtr);
    AddReader(buffTwoPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    buffThreePtr = NewBuffer(BUFFER_TYPE_LINE_21, 30);
    AddReader(buffThreePtr);
    AddReader(buffThreePtr);
    ASSERT_EQ(3, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(3, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    FreeBuffer(buffTwoPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    FreeBuffer(buffTwoPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffThreePtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffThreePtr);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Sunny Day Buffer Tests - Allocate and free Three Buffers with Three Readers.");
    BufferPoolInit();
    ASSERT_EQ(0, NumAllocatedBuffers());
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    AddReader(buffPtr);
    AddReader(buffPtr);
    AddReader(buffPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    buffTwoPtr = NewBuffer(BUFFER_TYPE_DTVCC, 10);
    AddReader(buffTwoPtr);
    AddReader(buffTwoPtr);
    AddReader(buffTwoPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    buffThreePtr = NewBuffer(BUFFER_TYPE_LINE_21, 30);
    AddReader(buffThreePtr);
    AddReader(buffThreePtr);
    AddReader(buffThreePtr);
    ASSERT_EQ(3, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(3, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(3, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    FreeBuffer(buffTwoPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    FreeBuffer(buffTwoPtr);
    ASSERT_EQ(2, NumAllocatedBuffers());
    FreeBuffer(buffTwoPtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffThreePtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffThreePtr);
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffThreePtr);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END
}  // itest__BufferAllocFreeTests()

/*------------------------------------------------------------------------------
 | INTEGRATION TESTS: Buffer Allocation/Free Exception Tests
 |
 | TEST CASES:
 |    1) Allocate too many buffers.
 |    2) Add 100 Readers to a buffer and Remove them.
 -------------------------------------------------------------------------------*/
void itest__BufferAllocFreeExceptionTests( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    Buffer* buffPtr;
    Buffer* buffPtrArr[11];

    for( int loop = 0; loop < 11; loop++ ) {
        buffPtrArr[loop] = NULL;
    }

    TEST_START("Test Case: Buffer Allocation/Free Exception Tests - Allocate too many buffers.");
    BufferPoolInit();
    ASSERT_EQ(0, NumAllocatedBuffers());
    for( int loop = 0; loop < 10; loop++ ) {
        buffPtrArr[loop] = NewBuffer(BUFFER_TYPE_BYTES, (100 + loop));
        ASSERT_EQ(loop+1, NumAllocatedBuffers());
    }
    ASSERT_EQ(10, NumAllocatedBuffers());
    ERRORS_EXPECTED(61)
    FATAL_ERROR_EXPECTED
    buffPtrArr[10] = NewBuffer(BUFFER_TYPE_BYTES, 110);
    for( int loop = 0; loop < 10; loop++ ) {
        FreeBuffer(buffPtrArr[loop]);
        ASSERT_EQ(9-loop, NumAllocatedBuffers());
    }
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Buffer Allocation/Free Exception Tests - Add 100 Readers to a buffer and Remove them.");
    BufferPoolInit();
    ASSERT_EQ(0, NumAllocatedBuffers());
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    ASSERT_EQ(1, NumAllocatedBuffers());
    for( int loop = 0; loop < 100; loop++ ) {
        AddReader(buffPtr);
    }
    ASSERT_EQ(1, NumAllocatedBuffers());
    for( int loop = 0; loop < 99; loop++ ) {
        FreeBuffer(buffPtr);
    }
    ASSERT_EQ(1, NumAllocatedBuffers());
    FreeBuffer(buffPtr);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

}  // itest__BufferAllocFreeExceptionTests()

/*----------------------------------------------------------------------------*/
/*--                             Test Suite                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 |                          Buffer Integration Tests
 |                          ========================
 | TESTED FILES:
 |    buffer_utils.c
 -------------------------------------------------------------------------------*/
int main( int argc, char* argv[] ) {
    INIT_TEST_FRAMEWORK( argc, argv )

    TEST_SUITE_START("Test Suite: Sunny Day Buffer Allocation/Free Tests.")
    itest__BufferAllocFreeTests( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: Buffer Allocation/Free Exception Tests.")
    itest__BufferAllocFreeExceptionTests( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    SHUTDOWN_TEST_FRAMEWORK
}  // main()

