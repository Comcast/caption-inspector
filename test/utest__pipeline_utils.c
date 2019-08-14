//
// Created by Rob Taylor on 2019-04-16.
//

#include "test_engine.h"
#include "../src/utils/pipeline_utils.c"

/*----------------------------------------------------------------------------*/
/*--                          Support for Stubs                             --*/
/*----------------------------------------------------------------------------*/

uint8 CcDataOutInitializeCalled;
uint8 DetermineDropFrameCalled;
uint8 DtvccDecodeAddSinkCalled;
uint8 DtvccDecodeInitializeCalled;
uint8 DtvccOutInitializeCalled;
uint8 Line21DecodeAddSinkCalled;
uint8 Line21DecodeInitializeCalled;
uint8 Line21OutInitializeCalled;
uint8 MccDecodeAddSinkCalled;
uint8 MccDecodeInitializeCalled;
uint8 MccEncodeAddSinkCalled;
uint8 MccEncodeInitializeCalled;
uint8 MccFileAddSinkCalled;
uint8 MccFileInitializeCalled;
uint8 MccOutInitializeCalled;
uint8 MovFileAddSinkCalled;
uint8 MovFileInitializeCalled;
uint8 MpCoreFileAddSinkCalled;
uint8 MpCoreFileInitializeCalled;
uint8 MpegFileAddSinkCalled;
uint8 MpegFileInitializeCalled;
uint8 SccEncodeAddSinkCalled;
uint8 SccEncodeInitializeCalled;
uint8 SccFileAddSinkCalled;
uint8 SccFileInitializeCalled;
uint8 SeiDecodeAddSinkCalled;
uint8 SeiDecodeInitializeCalled;
uint8 AddReaderCalled;
uint8 StubNextBufferFunctionCalled;
uint8 StubShutdownFunctionCalled;
uint8 StubDriveCounter;

Buffer* AddReaderBuffPtr;
void* StubNextBufferFunctionRootCtxPtr;
Buffer* StubNextBufferFunctionInBuffer;
boolean FailStubNextBufferFunction__Call1;
boolean FailStubNextBufferFunction__Call2;
void* StubShutdownFunctionRootCtxPtr;
boolean FailStubShutdownFunction__Call;
char* SccFileInitializeFileNameStr;
uint32 SccFileInitializeFramerate;
char* Line21OutInitializeFileNameStr;
char* MccFileInitializeFileNameStr;
char* DtvccOutInitializeFileNameStr;
char* CcDataOutInitializeFileNameStr;
char* MccOutInitializeFileNameStr;
char* MpegFileInitializeFileNameStr;
char* DetermineDropFrameInputFilename;
char* DetermineDropFrameArtifactPath;
boolean DetermineDropFrameSaveArtifacts;
boolean DetermineDropFrame__isDropFrame;
boolean MpegFileInitializeisDropframe;
boolean DetermineDropFrame__wasSuccessful;
boolean MpegFileInitializeOverrideDf;

void InitStubs( void ) {
    CcDataOutInitializeCalled = 0;
    DetermineDropFrameCalled = 0;
    DtvccDecodeAddSinkCalled = 0;
    DtvccDecodeInitializeCalled = 0;
    DtvccOutInitializeCalled = 0;
    Line21DecodeAddSinkCalled = 0;
    Line21DecodeInitializeCalled = 0;
    Line21OutInitializeCalled = 0;
    MccDecodeAddSinkCalled = 0;
    MccDecodeInitializeCalled = 0;
    MccEncodeAddSinkCalled = 0;
    MccEncodeInitializeCalled = 0;
    MccFileAddSinkCalled = 0;
    MccFileInitializeCalled = 0;
    MccOutInitializeCalled = 0;
    MovFileAddSinkCalled = 0;
    MovFileInitializeCalled = 0;
    MpCoreFileAddSinkCalled = 0;
    MpCoreFileInitializeCalled = 0;
    MpegFileAddSinkCalled = 0;
    MpegFileInitializeCalled = 0;
    SccEncodeAddSinkCalled = 0;
    SccEncodeInitializeCalled = 0;
    SccFileAddSinkCalled = 0;
    SccFileInitializeCalled = 0;
    SeiDecodeAddSinkCalled = 0;
    SeiDecodeInitializeCalled = 0;
    AddReaderCalled = 0;
    StubNextBufferFunctionCalled = 0;
    StubShutdownFunctionCalled = 0;
    StubDriveCounter = 0;

    AddReaderBuffPtr = NULL;
    StubNextBufferFunctionRootCtxPtr = NULL;
    StubNextBufferFunctionInBuffer = NULL;
    FailStubNextBufferFunction__Call1 = FALSE;
    FailStubNextBufferFunction__Call2 = FALSE;
    StubShutdownFunctionRootCtxPtr = NULL;
    FailStubShutdownFunction__Call = FALSE;
    SccFileInitializeFileNameStr = NULL;
    SccFileInitializeFramerate = 0;
    Line21OutInitializeFileNameStr = NULL;
    MccFileInitializeFileNameStr = NULL;
    DtvccOutInitializeFileNameStr = NULL;
    CcDataOutInitializeFileNameStr = NULL;
    MccOutInitializeFileNameStr = NULL;
    MpegFileInitializeFileNameStr = NULL;
    DetermineDropFrameInputFilename = NULL;
    DetermineDropFrameArtifactPath = NULL;
    DetermineDropFrameSaveArtifacts = FALSE;
    DetermineDropFrame__isDropFrame = FALSE;
    MpegFileInitializeisDropframe = FALSE;
    DetermineDropFrame__wasSuccessful = FALSE;
    MpegFileInitializeOverrideDf = FALSE;
}

boolean AnySpuriousFunctionsCalled( void ) {
    if( (CcDataOutInitializeCalled = 0) ||
        (DetermineDropFrameCalled != 0) ||
        (DtvccDecodeAddSinkCalled != 0) ||
        (DtvccDecodeInitializeCalled != 0) ||
        (DtvccOutInitializeCalled != 0) ||
        (Line21DecodeAddSinkCalled != 0) ||
        (Line21DecodeInitializeCalled != 0) ||
        (Line21OutInitializeCalled != 0) ||
        (MccDecodeAddSinkCalled != 0) ||
        (MccDecodeInitializeCalled != 0) ||
        (MccEncodeAddSinkCalled != 0) ||
        (MccEncodeInitializeCalled != 0) ||
        (MccFileAddSinkCalled != 0) ||
        (MccFileInitializeCalled != 0) ||
        (MccOutInitializeCalled != 0) ||
        (MovFileAddSinkCalled != 0) ||
        (MovFileInitializeCalled != 0) ||
        (MpCoreFileAddSinkCalled != 0) ||
        (MpCoreFileInitializeCalled != 0) ||
        (MpegFileAddSinkCalled != 0) ||
        (MpegFileInitializeCalled != 0) ||
        (SccEncodeAddSinkCalled != 0) ||
        (SccEncodeInitializeCalled != 0) ||
        (SccFileAddSinkCalled != 0) ||
        (SccFileInitializeCalled != 0) ||
        (SeiDecodeAddSinkCalled != 0) ||
        (SeiDecodeInitializeCalled != 0) ||
        (AddReaderCalled != 0) ||
        (StubNextBufferFunctionCalled != 0) ||
        (StubShutdownFunctionCalled != 0) ) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/*----------------------------------------------------------------------------*/
/*--                           Stub Functions                               --*/
/*----------------------------------------------------------------------------*/

LinkInfo CcDataOutInitialize( Context* rootCtxPtr, char* outputFileNameStr ) {
    LinkInfo linkInfo;
    linkInfo.sourceType = 1;

    CcDataOutInitializeCalled++;
    CcDataOutInitializeFileNameStr = outputFileNameStr;

    return linkInfo;
}

boolean DetermineDropFrame( char* fileNameStr, boolean saveMediaInfo, char* artifactPath, boolean* isDropFramePtr ) {

    DetermineDropFrameCalled++;
    DetermineDropFrameInputFilename = fileNameStr;
    if( saveMediaInfo == TRUE ) {
        DetermineDropFrameArtifactPath = artifactPath;
    }
    DetermineDropFrameSaveArtifacts = saveMediaInfo;
    *isDropFramePtr = DetermineDropFrame__isDropFrame;

    return DetermineDropFrame__wasSuccessful;
}

boolean DtvccDecodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    boolean retval = TRUE;

    DtvccDecodeAddSinkCalled++;

    return retval;
}

LinkInfo DtvccDecodeInitialize( Context* rootCtxPtr, boolean processOnly ) {
    LinkInfo linkInfo;
    linkInfo.sourceType = 1;

    DtvccDecodeInitializeCalled++;

    return linkInfo;
}

LinkInfo DtvccOutInitialize( Context* rootCtxPtr, char* outputFileNameStr, boolean nullEtxSuppressed, boolean msNotFrame ) {
    LinkInfo linkInfo;
    linkInfo.sourceType = 1;

    DtvccOutInitializeCalled++;
    DtvccOutInitializeFileNameStr = outputFileNameStr;

    return linkInfo;
}

boolean Line21DecodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    boolean retval = TRUE;

    Line21DecodeAddSinkCalled++;

    return retval;
}

LinkInfo Line21DecodeInitialize( Context* rootCtxPtr, boolean processOnly ) {
    LinkInfo linkInfo;
    linkInfo.sourceType = 1;

    Line21DecodeInitializeCalled++;

    return linkInfo;
}

LinkInfo Line21OutInitialize( Context* ctxPtr, char* outputFileNameStr ) {
    LinkInfo linkInfo;
    linkInfo.sourceType = 1;

    Line21OutInitializeCalled++;
    Line21OutInitializeFileNameStr = outputFileNameStr;

    return linkInfo;
}

boolean MccDecodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    boolean retval = TRUE;

    MccDecodeAddSinkCalled++;

    return retval;
}

LinkInfo MccDecodeInitialize( Context* rootCtxPtr ) {
    LinkInfo linkInfo;
    linkInfo.sourceType = 1;

    MccDecodeInitializeCalled++;

    return linkInfo;
}

boolean MccEncodeAddSink( Context* ctxPtr, LinkInfo linkInfo ) {
    boolean retval = TRUE;

    MccEncodeAddSinkCalled++;

    return retval;
}

LinkInfo MccEncodeInitialize( Context* rootCtxPtr ) {
    LinkInfo linkInfo;
    linkInfo.sourceType = 1;

    MccEncodeInitializeCalled++;

    return linkInfo;
}

boolean MccFileAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    boolean retval = TRUE;

    MccFileAddSinkCalled++;

    return retval;
}

boolean MccFileInitialize( Context* rootCtxPtr, char* fileNameStr ) {
    boolean retval = TRUE;

    MccFileInitializeCalled++;
    MccFileInitializeFileNameStr = fileNameStr;

    return retval;
}

boolean MccFileProcNextBuffer( Context* rootCtxPtr, boolean* isDonePtr ) {
    boolean retval = TRUE;

    StubDriveCounter++;

    if(StubDriveCounter == 3) {
        return FALSE;
    }

    if(StubDriveCounter == 13) {
        return FALSE;
    }

    if(StubDriveCounter == 20) {
        *isDonePtr = TRUE;
    }

    return retval;
}

LinkInfo MccOutInitialize( Context* ctxPtr, char* outputFileNameStr ) {
    LinkInfo linkInfo;
    linkInfo.sourceType = 1;

    MccOutInitializeCalled++;
    MccOutInitializeFileNameStr = outputFileNameStr;

    return linkInfo;
}

boolean MpegFileAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    boolean retval = TRUE;

    MpegFileAddSinkCalled++;

    return retval;
}

boolean MpegFileInitialize( Context* rootCtxPtr, char* fileNameStr, boolean overrideDropframe, boolean isDropframe, boolean bailAtTwenty ) {
    boolean retval = TRUE;

    MpegFileInitializeCalled++;
    MpegFileInitializeFileNameStr = fileNameStr;
    MpegFileInitializeisDropframe = isDropframe;
    MpegFileInitializeOverrideDf = overrideDropframe;

    return retval;
}

boolean MpegFileProcNextBuffer( Context* rootCtxPtr, boolean* isDonePtr ) {
    boolean retval = TRUE;

    return retval;
}

boolean SccEncodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    boolean retval = TRUE;

    SccEncodeAddSinkCalled++;

    return retval;
}

LinkInfo SccEncodeInitialize( Context* rootCtxPtr ) {
    LinkInfo linkInfo;
    linkInfo.sourceType = 1;

    SccEncodeInitializeCalled++;

    return linkInfo;
}

boolean SccFileAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    boolean retval = TRUE;

    SccFileAddSinkCalled++;

    return retval;
}

boolean SccFileInitialize( Context* rootCtxPtr, char* fileNameStr, uint32 frTimesOneHundred ) {
    boolean retval = TRUE;

    SccFileInitializeCalled++;
    SccFileInitializeFileNameStr = fileNameStr;
    SccFileInitializeFramerate = frTimesOneHundred;

    return retval;
}

boolean SccFileProcNextBuffer( Context* rootCtxPtr, boolean* isDonePtr ) {
    boolean retval = TRUE;

    StubDriveCounter++;

    if(StubDriveCounter == 10) {
        *isDonePtr = TRUE;
    }

    return retval;
}

boolean isFramerateValid( uint32 frameRatePerSecTimesOneHundred ) {
    if( frameRatePerSecTimesOneHundred == 2400 ) return TRUE;
    else return FALSE;
}

void AddReader( Buffer* buffPtr ) {
    LOG(TEST_DEBUG_LEVEL_INFO, TEST_SECTION, "AddReader(%p) Called", buffPtr);
    AddReaderCalled++;
    AddReaderBuffPtr = buffPtr;
}

boolean StubNextBufferFunction( void* rootCtxPtr, Buffer* inBuffer ) {
    boolean retval = TRUE;

    if( ((FailStubNextBufferFunction__Call1 == TRUE) && (StubNextBufferFunctionCalled == 1)) ||
        ((FailStubNextBufferFunction__Call2 == TRUE) && (StubNextBufferFunctionCalled == 2)) ) {
        retval = FALSE;
    }

    LOG(TEST_DEBUG_LEVEL_INFO, TEST_SECTION, "StubNextBufferFunction(%p, %p) Called - Returning %s", rootCtxPtr, inBuffer, retval?"TRUE":"FALSE");
    StubNextBufferFunctionCalled++;

    StubNextBufferFunctionRootCtxPtr = rootCtxPtr;
    StubNextBufferFunctionInBuffer = inBuffer;

    return retval;
}

boolean StubShutdownFunction( void* rootCtxPtr ) {
    boolean retval = TRUE;

    if( (FailStubShutdownFunction__Call == TRUE) && (StubShutdownFunctionCalled == 2) ) {
        retval = FALSE;
    }

    LOG(TEST_DEBUG_LEVEL_INFO, TEST_SECTION, "StubShutdownFunction(%p) Called - Returning %s", rootCtxPtr, retval?"TRUE":"FALSE");
    StubShutdownFunctionCalled++;

    StubShutdownFunctionRootCtxPtr = rootCtxPtr;

    return retval;
}

/*----------------------------------------------------------------------------*/
/*--                             Test Cases                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: InitSinks()
 |
 | TEST CASES:
 |    1) Successfully Init Sinks.
 |    2) Unsuccessfully Init a NULL Pointer.
 -------------------------------------------------------------------------------*/
void utest__InitSinks( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    Sinks sinks;

    TEST_START("Test Case: InitSinks() - Successfully Init Sinks.");
    sinks.linkType = 0;
    InitSinks( &sinks, 101 );
    ASSERT_EQ(101, sinks.linkType);
    TEST_END

    TEST_START("Test Case: InitSinks() - Unsuccessfully Init a NULL Pointer.");
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    InitSinks( NULL, 101 );
    TEST_END
}  // utest__InitSinks()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: AddSink()
 |
 | TEST CASES:
 |     1) Add a Valid Sink.
 |     2) Add Two Valid Sinks.
 |     3) Add Three Valid Sinks.
 |     4) Add InValid Sink: Bad Link Type in Sink.
 |     5) Add InValid Sink: Bad Link Type.
 |     6) Add InValid Sink: Bad Source Type.
 |     7) Add InValid Sink: Bad Sink Type.
 |     8) Add NULL Sink.
 |     9) Add NULL Link Info.
 |    10) Add Too Many Sinks.
 -------------------------------------------------------------------------------*/
void utest__AddSink( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    Sinks sinks;
    boolean retval;
    LinkInfo linkInfo;
    LinkInfo linkInfoTwo;
    LinkInfo linkInfoThree;

    TEST_START("Test Case: AddSink() - Add a Valid Sink.");
    sinks.numSinks = 0;
    sinks.linkType = MCC_FILE___MCC_DATA;
    linkInfo.linkType = MCC_DATA___CC_DATA;
    linkInfo.sinkType = DATA_TYPE_MCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    retval = AddSink( &sinks, &linkInfo );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, sinks.numSinks);
    TEST_END

    TEST_START("Test Case: AddSink() - Add Two Valid Sinks.");
    sinks.numSinks = 0;
    sinks.linkType = MCC_FILE___MCC_DATA;
    linkInfo.linkType = MCC_DATA___CC_DATA;
    linkInfo.sinkType = DATA_TYPE_MCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    retval = AddSink( &sinks, &linkInfo );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, sinks.numSinks);
    ASSERT_STRUCTEQ(&linkInfo, &sinks.sink[0], sizeof(LinkInfo));
    linkInfoTwo.linkType = LINE21_DATA___TEXT_FILE;
    linkInfoTwo.sinkType = DATA_TYPE_DECODED_608;
    linkInfoTwo.sourceType = DATA_TYPE_608_TXT_FILE;
    retval = AddSink( &sinks, &linkInfoTwo );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(2, sinks.numSinks);
    ASSERT_STRUCTEQ(&linkInfoTwo, &sinks.sink[1], sizeof(LinkInfo));
    TEST_END

    TEST_START("Test Case: AddSink() - Add Three Valid Sinks.");
    sinks.numSinks = 0;
    sinks.linkType = MCC_FILE___MCC_DATA;
    linkInfo.linkType = MCC_DATA___CC_DATA;
    linkInfo.sinkType = DATA_TYPE_MCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    retval = AddSink( &sinks, &linkInfo );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, sinks.numSinks);
    ASSERT_STRUCTEQ(&linkInfo, &sinks.sink[0], sizeof(LinkInfo));
    linkInfoTwo.linkType = LINE21_DATA___TEXT_FILE;
    linkInfoTwo.sinkType = DATA_TYPE_DECODED_608;
    linkInfoTwo.sourceType = DATA_TYPE_608_TXT_FILE;
    retval = AddSink( &sinks, &linkInfoTwo );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(2, sinks.numSinks);
    ASSERT_STRUCTEQ(&linkInfoTwo, &sinks.sink[1], sizeof(LinkInfo));
    linkInfoThree.linkType = SEI_DATA___TEXT_FILE;
    linkInfoThree.sinkType = DATA_TYPE_SEI_DATA;
    linkInfoThree.sourceType = DATA_TYPE_SEI_DATA_TXT_FILE;
    retval = AddSink( &sinks, &linkInfoThree );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(3, sinks.numSinks);
    ASSERT_STRUCTEQ(&linkInfoThree, &sinks.sink[2], sizeof(LinkInfo));
    TEST_END

    TEST_START("Test Case: AddSinks() - Add InValid Sink: Bad Link Type in Sink.");
    sinks.numSinks = 0;
    sinks.linkType = MAX_LINK_TYPE;
    linkInfo.linkType = SEI_DATA___TEXT_FILE;
    linkInfo.sinkType = DATA_TYPE_MCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    FATAL_ERROR_EXPECTED
    retval = AddSink( &sinks, &linkInfo );
    TEST_END

    TEST_START("Test Case: AddSinks() - Add InValid Sink: Bad Link Type.");
    sinks.numSinks = 0;
    sinks.linkType = MCC_FILE___MCC_DATA;
    linkInfo.linkType = MAX_LINK_TYPE + 5;
    linkInfo.sinkType = DATA_TYPE_MCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    retval = AddSink( &sinks, &linkInfo );
    TEST_END

    TEST_START("Test Case: AddSink() - Add InValid Sink: Bad Source Type.");
    sinks.numSinks = 0;
    sinks.linkType = MCC_FILE___MCC_DATA;
    linkInfo.linkType = MCC_DATA___CC_DATA;
    linkInfo.sinkType = DATA_TYPE_MCC_DATA;
    linkInfo.sourceType = MAX_DATA_TYPE + 10;
    FATAL_ERROR_EXPECTED
    retval = AddSink( &sinks, &linkInfo );
    TEST_END

    TEST_START("Test Case: AddSink() - Add InValid Sink: Bad Sink Type.");
    sinks.numSinks = 0;
    sinks.linkType = MCC_FILE___MCC_DATA;
    linkInfo.linkType = MCC_DATA___CC_DATA;
    linkInfo.sinkType = MAX_DATA_TYPE;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    FATAL_ERROR_EXPECTED
    retval = AddSink( &sinks, &linkInfo );
    TEST_END

    TEST_START("Test Case: AddSink() - Add NULL Sink.");
    sinks.numSinks = 0;
    sinks.linkType = MCC_FILE___MCC_DATA;
    linkInfo.linkType = MCC_DATA___CC_DATA;
    linkInfo.sinkType = MAX_DATA_TYPE;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    retval = AddSink( NULL, &linkInfo );
    TEST_END

    TEST_START("Test Case: AddSink() - Add NULL Link Info.");
    sinks.numSinks = 0;
    sinks.linkType = MCC_FILE___MCC_DATA;
    linkInfo.linkType = MCC_DATA___CC_DATA;
    linkInfo.sinkType = MAX_DATA_TYPE;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    retval = AddSink( &sinks, NULL );
    TEST_END

    TEST_START("Test Case: AddSink() - Add Too Many Sink.");
    sinks.numSinks = MAX_NUMBER_OF_SINKS;
    sinks.linkType = MCC_FILE___MCC_DATA;
    linkInfo.linkType = MCC_DATA___CC_DATA;
    linkInfo.sinkType = DATA_TYPE_MCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    ERROR_EXPECTED
    retval = AddSink( &sinks, &linkInfo );
    ASSERT_EQ(FALSE, retval);
    TEST_END
}  // utest__AddSink()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: PassToSinks()
 |
 | TEST CASES:
 |      1) Pass to One Sink.
 |      2) Pass to Two Sinks.
 |      3) Pass to Max Number of Sinks.
 |      4) Pass to Multiple Sinks with Different Return Values.
 |      5) Pass to No Sinks.
 |      6) Pass to Too Many Sinks.
 |      7) Pass a NULL Context.
 |      8) Pass a NULL Buffer.
 |      9) Pass a NULL Sinks List.
 |     10) Pass a NULL Sink.
 |     11) Pass an Invalid Link Type in Sinks.
 |     12) Pass an Invalid Sink Link Type in Sinks.
 -------------------------------------------------------------------------------*/
void utest__PassToSinks( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    boolean retval;
    Context ctx;
    Buffer buff;
    Sinks sinks;

    TEST_START("Test Case: PassToSinks() - Pass to One Sink.");
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 1;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = PassToSinks(&ctx, &buff, &sinks);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, AddReaderCalled);
    ASSERT_PTREQ(&buff, AddReaderBuffPtr);
    ASSERT_EQ(1, StubNextBufferFunctionCalled);
    ASSERT_PTREQ(&ctx, StubNextBufferFunctionRootCtxPtr);
    ASSERT_PTREQ(&buff, StubNextBufferFunctionInBuffer);
    AddReaderCalled = 0;
    StubNextBufferFunctionCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PassToSinks() - Pass to Two Sinks.");
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 2;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[1].linkType = MCC_DATA___CC_DATA;
    sinks.sink[1].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[1].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[1].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[1].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = PassToSinks(&ctx, &buff, &sinks);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(2, AddReaderCalled);
    ASSERT_PTREQ(&buff, AddReaderBuffPtr);
    ASSERT_EQ(2, StubNextBufferFunctionCalled);
    ASSERT_PTREQ(&ctx, StubNextBufferFunctionRootCtxPtr);
    ASSERT_PTREQ(&buff, StubNextBufferFunctionInBuffer);
    AddReaderCalled = 0;
    StubNextBufferFunctionCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PassToSinks() - Pass to Max Number of Sinks.");
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 5;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[1].linkType = MCC_DATA___CC_DATA;
    sinks.sink[1].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[1].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[1].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[1].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[2].linkType = MCC_DATA___CC_DATA;
    sinks.sink[2].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[2].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[2].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[2].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[3].linkType = MCC_DATA___CC_DATA;
    sinks.sink[3].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[3].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[3].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[3].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[4].linkType = MCC_DATA___CC_DATA;
    sinks.sink[4].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[4].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[4].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[4].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = PassToSinks(&ctx, &buff, &sinks);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(5, AddReaderCalled);
    ASSERT_PTREQ(&buff, AddReaderBuffPtr);
    ASSERT_EQ(5, StubNextBufferFunctionCalled);
    ASSERT_PTREQ(&ctx, StubNextBufferFunctionRootCtxPtr);
    ASSERT_PTREQ(&buff, StubNextBufferFunctionInBuffer);
    AddReaderCalled = 0;
    StubNextBufferFunctionCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PassToSinks() - Pass to Multiple Sinks with Different Return Values.");
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 4;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[1].linkType = MCC_DATA___CC_DATA;
    sinks.sink[1].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[1].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[1].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[1].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[2].linkType = MCC_DATA___CC_DATA;
    sinks.sink[2].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[2].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[2].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[2].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[3].linkType = MCC_DATA___CC_DATA;
    sinks.sink[3].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[3].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[3].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[3].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    FailStubNextBufferFunction__Call1 = TRUE;
    FailStubNextBufferFunction__Call2 = TRUE;
    retval = PassToSinks(&ctx, &buff, &sinks);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(4, AddReaderCalled);
    ASSERT_PTREQ(&buff, AddReaderBuffPtr);
    ASSERT_EQ(4, StubNextBufferFunctionCalled);
    ASSERT_PTREQ(&ctx, StubNextBufferFunctionRootCtxPtr);
    ASSERT_PTREQ(&buff, StubNextBufferFunctionInBuffer);
    AddReaderCalled = 0;
    StubNextBufferFunctionCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PassToSinks() - Pass to No Sinks.");
    ERROR_EXPECTED
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 0;
    InitStubs();
    retval = PassToSinks(&ctx, &buff, &sinks);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PassToSinks() - Pass to Too Many Sinks.");
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = MAX_NUMBER_OF_SINKS + 1;
    InitStubs();
    retval = PassToSinks(&ctx, &buff, &sinks);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PassToSinks() - Pass a NULL Context.");
    FATAL_ERROR_EXPECTED
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 1;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = PassToSinks(NULL, &buff, &sinks);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, AddReaderCalled);
    ASSERT_PTREQ(&buff, AddReaderBuffPtr);
    ASSERT_EQ(1, StubNextBufferFunctionCalled);
    ASSERT_PTREQ(NULL, StubNextBufferFunctionRootCtxPtr);
    ASSERT_PTREQ(&buff, StubNextBufferFunctionInBuffer);
    AddReaderCalled = 0;
    StubNextBufferFunctionCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PassToSinks() - Pass a NULL Buffer.");
    FATAL_ERROR_EXPECTED
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 1;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = PassToSinks(&ctx, NULL, &sinks);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, AddReaderCalled);
    ASSERT_PTREQ(NULL, AddReaderBuffPtr);
    ASSERT_EQ(1, StubNextBufferFunctionCalled);
    ASSERT_PTREQ(&ctx, StubNextBufferFunctionRootCtxPtr);
    ASSERT_PTREQ(NULL, StubNextBufferFunctionInBuffer);
    AddReaderCalled = 0;
    StubNextBufferFunctionCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PassToSinks() - Pass a NULL Sinks List.");
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    InitStubs();
    retval = PassToSinks(&ctx, &buff, NULL);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PassToSinks() - Pass a NULL Sink.");
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 1;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = NULL;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = PassToSinks(&ctx, &buff, &sinks);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PassToSinks() - Pass an Invalid Link Type in Sinks.");
    FATAL_ERROR_EXPECTED
    sinks.linkType = MAX_LINK_TYPE;
    sinks.numSinks = 1;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = PassToSinks(&ctx, &buff, &sinks);
    TEST_END

    TEST_START("Test Case: PassToSinks() - Pass an Invalid Sink Link Type in Sinks.");
    FATAL_ERROR_EXPECTED
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 1;
    sinks.sink[0].linkType = MAX_LINK_TYPE + 1;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = PassToSinks(&ctx, &buff, &sinks);
    TEST_END
}  // utest__PassToSinks()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: ShutdownSinks()
 |
 | TEST CASES:
 |      1) Shutdown One Sink.
 |      2) Shutdown Two Sinks.
 |      3) Shutdown Max Number of Sinks.
 |      4) Shutdown Multiple Sinks with Different Return Values.
 |      5) Shutdown No Sinks.
 |      6) Pass a NULL Context.
 |      7) Pass a NULL Sinks List.
 |      8) Pass a NULL Sink.
 |      9) Pass an Invalid Link Type in Sinks.
 |     10) Pass an Invalid Sink Link Type in Sinks.
 -------------------------------------------------------------------------------*/
void utest__ShutdownSinks( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    boolean retval;
    Context ctx;
    Sinks sinks;

    TEST_START("Test Case: ShutdownSinks() - Shutdown One Sink.")
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 1;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = ShutdownSinks(&ctx, &sinks);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, StubShutdownFunctionCalled);
    ASSERT_PTREQ(&ctx, StubShutdownFunctionRootCtxPtr);
    StubShutdownFunctionCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ShutdownSinks() - Shutdown Two Sinks.")
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 2;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[1].linkType = MCC_DATA___CC_DATA;
    sinks.sink[1].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[1].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[1].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[1].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = ShutdownSinks(&ctx, &sinks);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(2, StubShutdownFunctionCalled);
    ASSERT_PTREQ(&ctx, StubShutdownFunctionRootCtxPtr);
    StubShutdownFunctionCalled = 0;
    ASSERT_EQ(FALSE, AnySpuriousFunctionsCalled());
    TEST_END

    TEST_START("Test Case: ShutdownSinks() - Shutdown Max Number of Sinks.")
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 5;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[1].linkType = MCC_DATA___CC_DATA;
    sinks.sink[1].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[1].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[1].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[1].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[2].linkType = MCC_DATA___CC_DATA;
    sinks.sink[2].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[2].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[2].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[2].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[3].linkType = MCC_DATA___CC_DATA;
    sinks.sink[3].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[3].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[3].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[3].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[4].linkType = MCC_DATA___CC_DATA;
    sinks.sink[4].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[4].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[4].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[4].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = ShutdownSinks(&ctx, &sinks);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(5, StubShutdownFunctionCalled);
    ASSERT_PTREQ(&ctx, StubShutdownFunctionRootCtxPtr);
    StubShutdownFunctionCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ShutdownSinks() - Shutdown Multiple Sinks with Different Return Values.")
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 4;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[1].linkType = MCC_DATA___CC_DATA;
    sinks.sink[1].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[1].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[1].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[1].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[2].linkType = MCC_DATA___CC_DATA;
    sinks.sink[2].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[2].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[2].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[2].ShutdownFnPtr = StubShutdownFunction;
    sinks.sink[3].linkType = MCC_DATA___CC_DATA;
    sinks.sink[3].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[3].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[3].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[3].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    FailStubShutdownFunction__Call = TRUE;
    retval = ShutdownSinks(&ctx, &sinks);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(4, StubShutdownFunctionCalled);
    ASSERT_PTREQ(&ctx, StubShutdownFunctionRootCtxPtr);
    StubShutdownFunctionCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ShutdownSinks() - Shutdown No Sinks.");
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 0;
    InitStubs();
    retval = ShutdownSinks(&ctx, &sinks);
    ASSERT_EQ(FALSE, AnySpuriousFunctionsCalled());
    TEST_END

    TEST_START("Test Case: ShutdownSinks() - Shutdown Too Many Sinks.");
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = MAX_NUMBER_OF_SINKS + 1;
    InitStubs();
    retval = ShutdownSinks(&ctx, &sinks);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ShutdownSinks() - Pass a NULL Context.")
    FATAL_ERROR_EXPECTED
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 1;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = ShutdownSinks(NULL, &sinks);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, StubShutdownFunctionCalled);
    ASSERT_PTREQ(NULL, StubShutdownFunctionRootCtxPtr);
    StubShutdownFunctionCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ShutdownSinks() - Pass a NULL Sinks List.")
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    InitStubs();
    retval = ShutdownSinks(&ctx, NULL);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ShutdownSinks() - Pass a NULL Sink.");
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 1;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = NULL;
    InitStubs();
    retval = ShutdownSinks(&ctx, &sinks);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ShutdownSinks() - Pass an Invalid Link Type in Sinks.");
    FATAL_ERROR_EXPECTED
    sinks.linkType = MAX_LINK_TYPE;
    sinks.numSinks = 1;
    sinks.sink[0].linkType = MCC_DATA___CC_DATA;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = ShutdownSinks(&ctx, &sinks);
    TEST_END

    TEST_START("Test Case: ShutdownSinks() - Pass an Invalid Sink Link Type in Sinks.");
    FATAL_ERROR_EXPECTED
    sinks.linkType = MCC_FILE___MCC_DATA;
    sinks.numSinks = 1;
    sinks.sink[0].linkType = MAX_LINK_TYPE + 1;
    sinks.sink[0].sourceType = DATA_TYPE_MCC_DATA;
    sinks.sink[0].sinkType = DATA_TYPE_CC_DATA;
    sinks.sink[0].NextBufferFnPtr = StubNextBufferFunction;
    sinks.sink[0].ShutdownFnPtr = StubShutdownFunction;
    InitStubs();
    retval = ShutdownSinks(&ctx, &sinks);
    TEST_END
}  // utest__ShutdownSinks()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: PlumbSccPipeline()
 |
 | TEST CASES:
 |    1) Successfully Plumb SCC Pipeline with Artifacts.
 |    2) Successfully Plumb SCC Pipeline without Artifacts.
 |    2) Pass a NULL Input Filename.
 |    3) Pass a NULL Output Filename.
 |    4) Pass an Invalid Framerate.
 -------------------------------------------------------------------------------*/
void utest__PlumbSccPipeline( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    char* inputFilename = "Who";
    char* outputFilename = "Cares?";
    boolean retval;
    Context ctx;

    TEST_START("Test Case: PlumbSccPipeline() - Successfully Plumb SCC Pipeline with Artifacts.")
    InitStubs();
    retval = PlumbSccPipeline(&ctx, inputFilename, outputFilename, 2400, TRUE);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, SccFileInitializeCalled);
    ASSERT_EQ(1, SccFileAddSinkCalled);
    ASSERT_EQ(3, SccEncodeAddSinkCalled);
    ASSERT_EQ(1, Line21DecodeAddSinkCalled);
    ASSERT_EQ(1, SccEncodeInitializeCalled);
    ASSERT_EQ(1, Line21DecodeInitializeCalled);
    ASSERT_EQ(1, Line21OutInitializeCalled);
    ASSERT_EQ(1, MccEncodeInitializeCalled);
    ASSERT_EQ(1, MccEncodeAddSinkCalled);
    ASSERT_EQ(1, MccOutInitializeCalled);
    ASSERT_EQ(1, CcDataOutInitializeCalled);
    ASSERT_PTREQ(inputFilename, SccFileInitializeFileNameStr);
    ASSERT_EQ(2400, SccFileInitializeFramerate);
    ASSERT_PTREQ(outputFilename, Line21OutInitializeFileNameStr);
    SccFileInitializeCalled = 0;
    SccFileAddSinkCalled = 0;
    SccEncodeAddSinkCalled = 0;
    Line21DecodeAddSinkCalled = 0;
    SccEncodeInitializeCalled = 0;
    Line21DecodeInitializeCalled = 0;
    Line21OutInitializeCalled = 0;
    MccEncodeInitializeCalled = 0;
    MccOutInitializeCalled = 0;
    MccEncodeAddSinkCalled = 0;
    CcDataOutInitializeCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbSccPipeline() - Successfully Plumb SCC Pipeline without Artifacts.")
    InitStubs();
    retval = PlumbSccPipeline(&ctx, inputFilename, outputFilename, 2400, FALSE);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, SccFileInitializeCalled);
    ASSERT_EQ(1, SccFileAddSinkCalled);
    ASSERT_EQ(1, SccEncodeAddSinkCalled);
    ASSERT_EQ(1, SccEncodeInitializeCalled);
    ASSERT_EQ(1, Line21DecodeInitializeCalled);
    ASSERT_PTREQ(inputFilename, SccFileInitializeFileNameStr);
    ASSERT_EQ(2400, SccFileInitializeFramerate);
    ASSERT_PTREQ(Line21OutInitializeFileNameStr, NULL);
    SccFileInitializeCalled = 0;
    SccFileAddSinkCalled = 0;
    SccEncodeAddSinkCalled = 0;
    SccEncodeInitializeCalled = 0;
    Line21DecodeInitializeCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbSccPipeline() - Pass a NULL Input Filename.")
    ERROR_EXPECTED
    InitStubs();
    retval = PlumbSccPipeline(&ctx, NULL, outputFilename, 2400, FALSE);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, SccFileInitializeCalled);
    ASSERT_EQ(0, SccFileAddSinkCalled);
    ASSERT_EQ(0, SccEncodeAddSinkCalled);
    ASSERT_EQ(0, Line21DecodeAddSinkCalled);
    ASSERT_EQ(0, SccEncodeInitializeCalled);
    ASSERT_EQ(0, Line21DecodeInitializeCalled);
    ASSERT_EQ(0, Line21OutInitializeCalled);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbSccPipeline() - Pass a NULL Output Filename.")
    ERROR_EXPECTED
    InitStubs();
    retval = PlumbSccPipeline(&ctx, inputFilename, NULL, 2400, FALSE);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, SccFileInitializeCalled);
    ASSERT_EQ(0, SccFileAddSinkCalled);
    ASSERT_EQ(0, SccEncodeAddSinkCalled);
    ASSERT_EQ(0, Line21DecodeAddSinkCalled);
    ASSERT_EQ(0, SccEncodeInitializeCalled);
    ASSERT_EQ(0, Line21DecodeInitializeCalled);
    ASSERT_EQ(0, Line21OutInitializeCalled);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbSccPipeline() - Pass an Invalid Framerate.")
    FATAL_ERROR_EXPECTED
    InitStubs();
    retval = PlumbSccPipeline(&ctx, inputFilename, outputFilename, 2600, FALSE);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, SccFileInitializeCalled);
    ASSERT_EQ(0, SccFileAddSinkCalled);
    ASSERT_EQ(0, SccEncodeAddSinkCalled);
    ASSERT_EQ(0, Line21DecodeAddSinkCalled);
    ASSERT_EQ(0, SccEncodeInitializeCalled);
    ASSERT_EQ(0, Line21DecodeInitializeCalled);
    ASSERT_EQ(0, Line21OutInitializeCalled);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END
}  // utest__PlumbSccPipeline()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: PlumbMccPipeline()
 |
 | TEST CASES:
 |    1) Successfully Plumb MCC Pipeline with Artifacts.
 |    2) Successfully Plumb MCC Pipeline without Artifacts.
 |    3) Pass a NULL Input Filename.
 |    4) Pass a NULL Output Filename.
 -------------------------------------------------------------------------------*/
void utest__PlumbMccPipeline( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    char* inputFilename = "Who";
    char* outputFilename = "Cares?";
    boolean retval;
    Context ctx;

    TEST_START("Test Case: PlumbMccPipeline() - Successfully Plumb MCC Pipeline with Artifacts.");
    InitStubs();
    retval = PlumbMccPipeline( &ctx, inputFilename, outputFilename, TRUE );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, MccFileInitializeCalled);
    ASSERT_EQ(1, MccFileAddSinkCalled);
    ASSERT_EQ(1, MccDecodeInitializeCalled);
    ASSERT_EQ(3, MccDecodeAddSinkCalled);
    ASSERT_EQ(1, DtvccDecodeInitializeCalled);
    ASSERT_EQ(1, DtvccDecodeAddSinkCalled);
    ASSERT_EQ(1, Line21DecodeInitializeCalled);
    ASSERT_EQ(1, Line21DecodeAddSinkCalled);
    ASSERT_EQ(1, DtvccOutInitializeCalled);
    ASSERT_EQ(1, Line21OutInitializeCalled);
    ASSERT_EQ(1, CcDataOutInitializeCalled);
    ASSERT_PTREQ(inputFilename, MccFileInitializeFileNameStr);
    ASSERT_PTREQ(outputFilename, DtvccOutInitializeFileNameStr);
    ASSERT_PTREQ(outputFilename, Line21OutInitializeFileNameStr);
    ASSERT_PTREQ(outputFilename, CcDataOutInitializeFileNameStr);
    MccFileInitializeCalled = 0;
    MccFileAddSinkCalled = 0;
    MccDecodeInitializeCalled = 0;
    MccDecodeAddSinkCalled = 0;
    DtvccDecodeInitializeCalled = 0;
    DtvccDecodeAddSinkCalled = 0;
    Line21DecodeInitializeCalled = 0;
    Line21DecodeAddSinkCalled = 0;
    DtvccOutInitializeCalled = 0;
    Line21OutInitializeCalled = 0;
    CcDataOutInitializeCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbMccPipeline() - Successfully Plumb MCC Pipeline with Artifacts.");
    InitStubs();
    retval = PlumbMccPipeline( &ctx, inputFilename, outputFilename, FALSE );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, MccFileInitializeCalled);
    ASSERT_EQ(1, MccFileAddSinkCalled);
    ASSERT_EQ(1, MccDecodeInitializeCalled);
    ASSERT_EQ(2, MccDecodeAddSinkCalled);
    ASSERT_EQ(1, DtvccDecodeInitializeCalled);
    ASSERT_EQ(1, Line21DecodeInitializeCalled);
    ASSERT_PTREQ(inputFilename, MccFileInitializeFileNameStr);
    ASSERT_PTREQ(DtvccOutInitializeFileNameStr, NULL);
    ASSERT_PTREQ(Line21OutInitializeFileNameStr, NULL);
    ASSERT_PTREQ(CcDataOutInitializeFileNameStr, NULL);
    MccFileInitializeCalled = 0;
    MccFileAddSinkCalled = 0;
    MccDecodeInitializeCalled = 0;
    MccDecodeAddSinkCalled = 0;
    DtvccDecodeInitializeCalled = 0;
    Line21DecodeInitializeCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbMccPipeline() - Pass a NULL Input Filename.");
    ERROR_EXPECTED
    InitStubs();
    retval = PlumbMccPipeline( &ctx, NULL, outputFilename, FALSE );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, MccFileInitializeCalled);
    ASSERT_EQ(0, MccFileAddSinkCalled);
    ASSERT_EQ(0, MccDecodeInitializeCalled);
    ASSERT_EQ(0, MccDecodeAddSinkCalled);
    ASSERT_EQ(0, DtvccDecodeInitializeCalled);
    ASSERT_EQ(0, DtvccDecodeAddSinkCalled);
    ASSERT_EQ(0, Line21DecodeInitializeCalled);
    ASSERT_EQ(0, Line21DecodeAddSinkCalled);
    ASSERT_EQ(0, DtvccOutInitializeCalled);
    ASSERT_EQ(0, Line21OutInitializeCalled);
    ASSERT_EQ(0, CcDataOutInitializeCalled);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbMccPipeline() - Pass a NULL Output Filename.");
    ERROR_EXPECTED
    InitStubs();
    retval = PlumbMccPipeline( &ctx, inputFilename, NULL, FALSE );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, MccFileInitializeCalled);
    ASSERT_EQ(0, MccFileAddSinkCalled);
    ASSERT_EQ(0, MccDecodeInitializeCalled);
    ASSERT_EQ(0, MccDecodeAddSinkCalled);
    ASSERT_EQ(0, DtvccDecodeInitializeCalled);
    ASSERT_EQ(0, DtvccDecodeAddSinkCalled);
    ASSERT_EQ(0, Line21DecodeInitializeCalled);
    ASSERT_EQ(0, Line21DecodeAddSinkCalled);
    ASSERT_EQ(0, DtvccOutInitializeCalled);
    ASSERT_EQ(0, Line21OutInitializeCalled);
    ASSERT_EQ(0, CcDataOutInitializeCalled);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END
}  // utest__PlumbMccPipeline()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: PlumbMpegPipeline()
 |
 | TEST CASES:
 |    1) Successfully Plumb MPEG DF Pipeline with Artifacts.
 |    2) Successfully Plumb MPEG NDF Pipeline with Artifacts.
 |    3) Successfully Plumb MPEG DF Pipeline without Artifacts.
 |    4) Successfully Plumb MPEG NDF Pipeline without Artifacts.
 |    5) Successfully Plumb MPEG Ambiguous Dropframe Pipeline without Artifacts.
 |    6) Successfully Plumb MPEG without Artifacts with a NULL Artifact Path.
 |    7) Pass a NULL Input Filename.
 |    8) Pass a NULL Output Filename.
 |    9) Pass a NULL Artifact Path.
 -------------------------------------------------------------------------------*/
void utest__PlumbMpegPipeline( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    char* inputFilename = "Who";
    char* outputFilename = "Cares?";
    char* artifactPath = "NotMe";
    boolean retval;
    Context ctx;

    TEST_START("Test Case: PlumbMpegPipeline() - Successfully Plumb MPEG DF Pipeline with Artifacts.");
    InitStubs();
    DetermineDropFrame__isDropFrame = TRUE;
    DetermineDropFrame__wasSuccessful = TRUE;
    retval = PlumbMpegPipeline( &ctx, inputFilename, outputFilename, TRUE, artifactPath, FALSE );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, DetermineDropFrameCalled);
    ASSERT_EQ(1, MpegFileInitializeCalled);
    ASSERT_EQ(4, MpegFileAddSinkCalled);
    ASSERT_EQ(1, MccEncodeInitializeCalled);
    ASSERT_EQ(1, MccEncodeAddSinkCalled);
    ASSERT_EQ(1, DtvccDecodeInitializeCalled);
    ASSERT_EQ(1, DtvccDecodeAddSinkCalled);
    ASSERT_EQ(1, Line21DecodeInitializeCalled);
    ASSERT_EQ(1, Line21DecodeAddSinkCalled);
    ASSERT_EQ(1, DtvccOutInitializeCalled);
    ASSERT_EQ(1, Line21OutInitializeCalled);
    ASSERT_EQ(1, CcDataOutInitializeCalled);
    ASSERT_EQ(1, MccOutInitializeCalled);
    ASSERT_PTREQ(inputFilename, MpegFileInitializeFileNameStr);
    ASSERT_PTREQ(inputFilename, DetermineDropFrameInputFilename);
    ASSERT_PTREQ(outputFilename, MccOutInitializeFileNameStr);
    ASSERT_PTREQ(artifactPath, DtvccOutInitializeFileNameStr);
    ASSERT_PTREQ(artifactPath, Line21OutInitializeFileNameStr);
    ASSERT_PTREQ(artifactPath, CcDataOutInitializeFileNameStr);
    ASSERT_PTREQ(artifactPath, DetermineDropFrameArtifactPath);
    ASSERT_EQ(TRUE, DetermineDropFrameSaveArtifacts);
    ASSERT_EQ(TRUE, MpegFileInitializeisDropframe);
    DetermineDropFrameCalled = 0;
    MpegFileInitializeCalled = 0;
    MpegFileAddSinkCalled = 0;
    MccEncodeInitializeCalled = 0;
    MccEncodeAddSinkCalled = 0;
    DtvccDecodeInitializeCalled = 0;
    DtvccDecodeAddSinkCalled = 0;
    Line21DecodeInitializeCalled = 0;
    Line21DecodeAddSinkCalled = 0;
    DtvccOutInitializeCalled = 0;
    Line21OutInitializeCalled = 0;
    CcDataOutInitializeCalled = 0;
    MccOutInitializeCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbMpegPipeline() - Successfully Plumb MPEG NDF Pipeline with Artifacts.");
    InitStubs();
    DetermineDropFrame__isDropFrame = FALSE;
    DetermineDropFrame__wasSuccessful = TRUE;
    retval = PlumbMpegPipeline( &ctx, inputFilename, outputFilename, TRUE, artifactPath, FALSE );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, DetermineDropFrameCalled);
    ASSERT_EQ(1, MpegFileInitializeCalled);
    ASSERT_EQ(4, MpegFileAddSinkCalled);
    ASSERT_EQ(1, MccEncodeInitializeCalled);
    ASSERT_EQ(1, MccEncodeAddSinkCalled);
    ASSERT_EQ(1, DtvccDecodeInitializeCalled);
    ASSERT_EQ(1, DtvccDecodeAddSinkCalled);
    ASSERT_EQ(1, Line21DecodeInitializeCalled);
    ASSERT_EQ(1, Line21DecodeAddSinkCalled);
    ASSERT_EQ(1, DtvccOutInitializeCalled);
    ASSERT_EQ(1, Line21OutInitializeCalled);
    ASSERT_EQ(1, CcDataOutInitializeCalled);
    ASSERT_EQ(1, MccOutInitializeCalled);
    ASSERT_PTREQ(inputFilename, MpegFileInitializeFileNameStr);
    ASSERT_PTREQ(inputFilename, DetermineDropFrameInputFilename);
    ASSERT_PTREQ(outputFilename, MccOutInitializeFileNameStr);
    ASSERT_PTREQ(artifactPath, DtvccOutInitializeFileNameStr);
    ASSERT_PTREQ(artifactPath, Line21OutInitializeFileNameStr);
    ASSERT_PTREQ(artifactPath, CcDataOutInitializeFileNameStr);
    ASSERT_PTREQ(artifactPath, DetermineDropFrameArtifactPath);
    ASSERT_EQ(TRUE, DetermineDropFrameSaveArtifacts);
    ASSERT_EQ(FALSE, MpegFileInitializeisDropframe);
    ASSERT_EQ(TRUE, MpegFileInitializeOverrideDf);
    DetermineDropFrameCalled = 0;
    MpegFileInitializeCalled = 0;
    MpegFileAddSinkCalled = 0;
    MccEncodeInitializeCalled = 0;
    MccEncodeAddSinkCalled = 0;
    DtvccDecodeInitializeCalled = 0;
    DtvccDecodeAddSinkCalled = 0;
    Line21DecodeInitializeCalled = 0;
    Line21DecodeAddSinkCalled = 0;
    DtvccOutInitializeCalled = 0;
    Line21OutInitializeCalled = 0;
    CcDataOutInitializeCalled = 0;
    MccOutInitializeCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbMpegPipeline() - Successfully Plumb MPEG DF Pipeline without Artifacts.");
    InitStubs();
    DetermineDropFrame__isDropFrame = TRUE;
    DetermineDropFrame__wasSuccessful = TRUE;
    retval = PlumbMpegPipeline( &ctx, inputFilename, outputFilename, FALSE, artifactPath, FALSE );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, DetermineDropFrameCalled);
    ASSERT_EQ(1, MpegFileInitializeCalled);
    ASSERT_EQ(3, MpegFileAddSinkCalled);
    ASSERT_EQ(1, MccEncodeInitializeCalled);
    ASSERT_EQ(1, MccEncodeAddSinkCalled);
    ASSERT_EQ(1, DtvccDecodeInitializeCalled);
    ASSERT_EQ(1, Line21DecodeInitializeCalled);
    ASSERT_EQ(1, MccOutInitializeCalled);
    ASSERT_PTREQ(inputFilename, MpegFileInitializeFileNameStr);
    ASSERT_PTREQ(inputFilename, DetermineDropFrameInputFilename);
    ASSERT_PTREQ(outputFilename, MccOutInitializeFileNameStr);
    ASSERT_PTREQ(NULL, DtvccOutInitializeFileNameStr);
    ASSERT_PTREQ(NULL, Line21OutInitializeFileNameStr);
    ASSERT_PTREQ(NULL, CcDataOutInitializeFileNameStr);
    ASSERT_PTREQ(NULL, DetermineDropFrameArtifactPath);
    ASSERT_EQ(FALSE, DetermineDropFrameSaveArtifacts);
    ASSERT_EQ(TRUE, MpegFileInitializeisDropframe);
    ASSERT_EQ(TRUE, MpegFileInitializeOverrideDf);
    DetermineDropFrameCalled = 0;
    MpegFileInitializeCalled = 0;
    MpegFileAddSinkCalled = 0;
    MccEncodeInitializeCalled = 0;
    MccEncodeAddSinkCalled = 0;
    DtvccDecodeInitializeCalled = 0;
    Line21DecodeInitializeCalled = 0;
    MccOutInitializeCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbMpegPipeline() - Successfully Plumb MPEG Ambiguous Dropframe Pipeline without Artifacts.");
    InitStubs();
    DetermineDropFrame__isDropFrame = FALSE;
    DetermineDropFrame__wasSuccessful = FALSE;
    retval = PlumbMpegPipeline( &ctx, inputFilename, outputFilename, FALSE, artifactPath, FALSE );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, DetermineDropFrameCalled);
    ASSERT_EQ(1, MpegFileInitializeCalled);
    ASSERT_EQ(3, MpegFileAddSinkCalled);
    ASSERT_EQ(1, MccEncodeInitializeCalled);
    ASSERT_EQ(1, MccEncodeAddSinkCalled);
    ASSERT_EQ(1, DtvccDecodeInitializeCalled);
    ASSERT_EQ(1, Line21DecodeInitializeCalled);
    ASSERT_EQ(1, MccOutInitializeCalled);
    ASSERT_PTREQ(inputFilename, MpegFileInitializeFileNameStr);
    ASSERT_PTREQ(inputFilename, DetermineDropFrameInputFilename);
    ASSERT_PTREQ(outputFilename, MccOutInitializeFileNameStr);
    ASSERT_PTREQ(NULL, DtvccOutInitializeFileNameStr);
    ASSERT_PTREQ(NULL, Line21OutInitializeFileNameStr);
    ASSERT_PTREQ(NULL, CcDataOutInitializeFileNameStr);
    ASSERT_PTREQ(NULL, DetermineDropFrameArtifactPath);
    ASSERT_EQ(FALSE, DetermineDropFrameSaveArtifacts);
    ASSERT_EQ(FALSE, MpegFileInitializeisDropframe);
    ASSERT_EQ(FALSE, MpegFileInitializeOverrideDf);
    DetermineDropFrameCalled = 0;
    MpegFileInitializeCalled = 0;
    MpegFileAddSinkCalled = 0;
    MccEncodeInitializeCalled = 0;
    MccEncodeAddSinkCalled = 0;
    DtvccDecodeInitializeCalled = 0;
    Line21DecodeInitializeCalled = 0;
    MccOutInitializeCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbMpegPipeline() - Successfully Plumb MPEG without Artifacts with a NULL Artifact Path.");
    InitStubs();
    DetermineDropFrame__isDropFrame = FALSE;
    DetermineDropFrame__wasSuccessful = FALSE;
    retval = PlumbMpegPipeline( &ctx, inputFilename, outputFilename, FALSE, NULL, FALSE );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, DetermineDropFrameCalled);
    ASSERT_EQ(1, MpegFileInitializeCalled);
    ASSERT_EQ(3, MpegFileAddSinkCalled);
    ASSERT_EQ(1, MccEncodeInitializeCalled);
    ASSERT_EQ(1, MccEncodeAddSinkCalled);
    ASSERT_EQ(1, DtvccDecodeInitializeCalled);
    ASSERT_EQ(1, Line21DecodeInitializeCalled);
    ASSERT_EQ(1, MccOutInitializeCalled);
    ASSERT_PTREQ(inputFilename, MpegFileInitializeFileNameStr);
    ASSERT_PTREQ(inputFilename, DetermineDropFrameInputFilename);
    ASSERT_PTREQ(outputFilename, MccOutInitializeFileNameStr);
    ASSERT_PTREQ(NULL, DtvccOutInitializeFileNameStr);
    ASSERT_PTREQ(NULL, Line21OutInitializeFileNameStr);
    ASSERT_PTREQ(NULL, CcDataOutInitializeFileNameStr);
    ASSERT_PTREQ(NULL, DetermineDropFrameArtifactPath);
    ASSERT_EQ(FALSE, DetermineDropFrameSaveArtifacts);
    ASSERT_EQ(FALSE, MpegFileInitializeisDropframe);
    ASSERT_EQ(FALSE, MpegFileInitializeOverrideDf);
    DetermineDropFrameCalled = 0;
    MpegFileInitializeCalled = 0;
    MpegFileAddSinkCalled = 0;
    MccEncodeInitializeCalled = 0;
    MccEncodeAddSinkCalled = 0;
    DtvccDecodeInitializeCalled = 0;
    Line21DecodeInitializeCalled = 0;
    MccOutInitializeCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbMpegPipeline() - Pass a NULL Input Filename.");
    InitStubs();
    DetermineDropFrame__isDropFrame = TRUE;
    DetermineDropFrame__wasSuccessful = TRUE;
    ERROR_EXPECTED
    retval = PlumbMpegPipeline( &ctx, NULL, outputFilename, TRUE, artifactPath, FALSE );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, DetermineDropFrameCalled);
    ASSERT_EQ(0, MpegFileInitializeCalled);
    ASSERT_EQ(0, MpegFileAddSinkCalled);
    ASSERT_EQ(0, MccEncodeInitializeCalled);
    ASSERT_EQ(0, MccEncodeAddSinkCalled);
    ASSERT_EQ(0, DtvccDecodeInitializeCalled);
    ASSERT_EQ(0, DtvccDecodeAddSinkCalled);
    ASSERT_EQ(0, Line21DecodeInitializeCalled);
    ASSERT_EQ(0, Line21DecodeAddSinkCalled);
    ASSERT_EQ(0, DtvccOutInitializeCalled);
    ASSERT_EQ(0, Line21OutInitializeCalled);
    ASSERT_EQ(0, CcDataOutInitializeCalled);
    ASSERT_EQ(0, MccOutInitializeCalled);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbMpegPipeline() - Pass a NULL Output Filename.");
    InitStubs();
    DetermineDropFrame__isDropFrame = TRUE;
    DetermineDropFrame__wasSuccessful = TRUE;
    ERROR_EXPECTED
    retval = PlumbMpegPipeline( &ctx, inputFilename, NULL, TRUE, artifactPath, FALSE );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, DetermineDropFrameCalled);
    ASSERT_EQ(0, MpegFileInitializeCalled);
    ASSERT_EQ(0, MpegFileAddSinkCalled);
    ASSERT_EQ(0, MccEncodeInitializeCalled);
    ASSERT_EQ(0, MccEncodeAddSinkCalled);
    ASSERT_EQ(0, DtvccDecodeInitializeCalled);
    ASSERT_EQ(0, DtvccDecodeAddSinkCalled);
    ASSERT_EQ(0, Line21DecodeInitializeCalled);
    ASSERT_EQ(0, Line21DecodeAddSinkCalled);
    ASSERT_EQ(0, DtvccOutInitializeCalled);
    ASSERT_EQ(0, Line21OutInitializeCalled);
    ASSERT_EQ(0, CcDataOutInitializeCalled);
    ASSERT_EQ(0, MccOutInitializeCalled);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: PlumbMpegPipeline() - Pass a NULL Artifact Directory.");
    InitStubs();
    DetermineDropFrame__isDropFrame = TRUE;
    DetermineDropFrame__wasSuccessful = TRUE;
    ERROR_EXPECTED
    retval = PlumbMpegPipeline( &ctx, inputFilename, outputFilename, TRUE, NULL, FALSE );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, DetermineDropFrameCalled);
    ASSERT_EQ(0, MpegFileInitializeCalled);
    ASSERT_EQ(0, MpegFileAddSinkCalled);
    ASSERT_EQ(0, MccEncodeInitializeCalled);
    ASSERT_EQ(0, MccEncodeAddSinkCalled);
    ASSERT_EQ(0, DtvccDecodeInitializeCalled);
    ASSERT_EQ(0, DtvccDecodeAddSinkCalled);
    ASSERT_EQ(0, Line21DecodeInitializeCalled);
    ASSERT_EQ(0, Line21DecodeAddSinkCalled);
    ASSERT_EQ(0, DtvccOutInitializeCalled);
    ASSERT_EQ(0, Line21OutInitializeCalled);
    ASSERT_EQ(0, CcDataOutInitializeCalled);
    ASSERT_EQ(0, MccOutInitializeCalled);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END
}  // utest__PlumbMpegPipeline()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: DrivePipeline()
 |
 | TEST CASES:
 |    1) Successfully Drive the Pipeline.
 |    2) Receive Two Errors driving the pipeline.
 |    3) Pass a NULL Context.
 |    4) Pass an Invalid File type.
 -------------------------------------------------------------------------------*/
void utest__DrivePipeline( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    Context ctx;

    TEST_START("Test Case: DrivePipeline() - Successfully Drive the Pipeline.");
    InitStubs();
    DrivePipeline(SCC_CAPTIONS_FILE, &ctx);
    TEST_END

    TEST_START("Test Case: DrivePipeline() - Receive Two Errors driving the pipeline.");
    InitStubs();
    ERRORS_EXPECTED(2)
    DrivePipeline(MCC_CAPTIONS_FILE, &ctx);
    TEST_END

    TEST_START("Test Case: DrivePipeline() - Pass a NULL Context.");
    FATAL_ERROR_EXPECTED
    InitStubs();
    DrivePipeline(SCC_CAPTIONS_FILE, NULL);
    TEST_END

    TEST_START("Test Case: DrivePipeline() - Pass an Invalid File type.");
    ERROR_EXPECTED
    InitStubs();
    DrivePipeline(MAX_FILE_TYPE+1, &ctx);
    TEST_END
}  // utest__DrivePipeline()

/*----------------------------------------------------------------------------*/
/*--                             Test Suite                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | TESTED FILES:
 |    pipeline_utils.c
 |
 | TESTED FUNCTIONS:
 |    pipeline_utils.c - InitSinks()
 |    pipeline_utils.c - AddSink()
 |    pipeline_utils.c - PassToSinks()
 |    pipeline_utils.c - ShutdownSinks()
 |    pipeline_utils.c - PlumbSccPipeline()
 |    pipeline_utils.c - PlumbMccPipeline()
 |    pipeline_utils.c - PlumbMpegPipeline()
 |    pipeline_utils.c - DrivePipeline()
 |
 | UNTESTED FUNCTIONS:
 -------------------------------------------------------------------------------*/
int main( int argc, char* argv[] ) {
    INIT_TEST_FRAMEWORK( argc, argv )

    TEST_SUITE_START("Test Suite: pipeline_utils.c -- PassToSinks()");
    utest__PassToSinks( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: pipeline_utils.c -- ShutdownSinks()");
    utest__ShutdownSinks( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: pipeline_utils.c -- InitSinks()");
    utest__InitSinks( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: pipeline_utils.c -- AddSinks()");
    utest__AddSink( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: pipeline_utils.c -- PlumbSccPipeline()");
    utest__PlumbSccPipeline( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: pipeline_utils.c -- PlumbMccPipeline()");
    utest__PlumbMccPipeline( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: pipeline_utils.c -- PlumbMpegPipeline()");
    utest__PlumbMpegPipeline( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: pipeline_utils.c -- DrivePipeline()");
    utest__DrivePipeline( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    SHUTDOWN_TEST_FRAMEWORK
}  // main()
