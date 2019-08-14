//
// Created by Rob Taylor on 2019-04-08.
//

#include "test_engine.h"

#define IsDebugInitialized   fakeIsDebugInitialized
#define IsDebugExternallyRouted   fakeIsDebugExternallyRouted

#include "../src/utils/external_adaptor.c"

#undef IsDebugInitialized
#undef IsDebugExternallyRouted

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

const char* captionFileTypeStr[MAX_FILE_TYPE] = {
        "Unknown Caption File",     // UNK_CAPTIONS_FILE
        "SCC Caption File",         // SCC_CAPTIONS_FILE
        "MCC Caption File",         // MCC_CAPTIONS_FILE
        "Binary MPEG File",         // MPEG_BINARY_FILE
        "Binary MOV File",          // MOV_BINARY_FILE
};

uint8 fakeCtx;
Buffer buffer;
Line21Code line21Codes[3];
DtvccData dtvccDatas[3];

/*----------------------------------------------------------------------------*/
/*--                          Support for Stubs                             --*/
/*----------------------------------------------------------------------------*/

uint8 DetermineDropFrameCalled;
uint8 DtvccDecodeAddSinkCalled;
uint8 DtvccDecodeInitializeCalled;
uint8 Line21DecodeAddSinkCalled;
uint8 Line21DecodeInitializeCalled;
uint8 MccDecodeAddSinkCalled;
uint8 MccDecodeInitializeCalled;
uint8 MccFileAddSinkCalled;
uint8 MccFileInitializeCalled;
uint8 MovFileAddSinkCalled;
uint8 MovFileInitializeCalled;
uint8 MpegFileAddSinkCalled;
uint8 MpegFileInitializeCalled;
uint8 SccEncodeAddSinkCalled;
uint8 SccFileInitializeCalled;
uint8 AddReaderCalled;
uint8 BufferPoolInitCalled;
uint8 FreeBufferCalled;
uint8 stubExternal608CallbackCalled;
uint8 stubExternal708CallbackCalled;
uint8 stubExternalEndDataCallbackCalled;
uint8 PlumbMccPipelineCalled;
uint8 PlumbSccPipelineCalled;
uint8 PlumbMpgPipelineCalled;
uint8 PlumbMovPipelineCalled;
uint8 DrivePipelineCalled;
uint8 DetermineFileTypeCalled;

Buffer* AddReaderBuffPtr;
char* SccFileInitializeFileNameStr;
uint32 SccFileInitializeFramerate;
char* MccFileInitializeFileNameStr;
char* MpegFileInitializeFileNameStr;
char* MovFileInitializeFileNameStr;
char* DetermineDropFrameInputFilename;
char* DetermineDropFrameArtifactPath;
boolean DetermineDropFrameSaveArtifacts;
boolean DetermineDropFrame__isDropFrame;
boolean MpegFileInitializeisDropframe;
boolean MovFileInitializeisDropframe;
boolean DetermineDropFrame__wasSuccessful;
boolean MpegFileInitializeOverrideDf;
boolean MovFileInitializeOverrideDf;
boolean fakeIsDebugInitializedVal;
boolean fakeIsDebugExternallyRoutedVal;
boolean PlumbMccPipelineReturn;
boolean PlumbSccPipelineReturn;
boolean PlumbMpgPipelineReturn;
boolean PlumbMovPipelineReturn;
FileType DetermineFileTypeReturn;
boolean DtvccDecodeAddSinkReturn;
boolean Line21DecodeAddSinkReturn;
boolean MccDecodeAddSinkReturn;
boolean MccFileAddSinkReturn;
boolean MccFileInitializeReturn;
boolean MovFileAddSinkReturn;
boolean MovFileInitializeReturn;
boolean MpegFileAddSinkReturn;
boolean MpegFileInitializeReturn;
boolean SccEncodeAddSinkReturn;
boolean SccFileInitializeReturn;

void InitStubs( void ) {
    DetermineDropFrameCalled = 0;
    DtvccDecodeAddSinkCalled = 0;
    DtvccDecodeInitializeCalled = 0;
    Line21DecodeAddSinkCalled = 0;
    Line21DecodeInitializeCalled = 0;
    MccDecodeAddSinkCalled = 0;
    MccDecodeInitializeCalled = 0;
    MccFileAddSinkCalled = 0;
    MccFileInitializeCalled = 0;
    MovFileAddSinkCalled = 0;
    MovFileInitializeCalled = 0;
    MpegFileAddSinkCalled = 0;
    MpegFileInitializeCalled = 0;
    SccEncodeAddSinkCalled = 0;
    SccFileInitializeCalled = 0;
    AddReaderCalled = 0;
    BufferPoolInitCalled = 0;
    FreeBufferCalled = 0;
    stubExternal608CallbackCalled = 0;
    stubExternal708CallbackCalled = 0;
    stubExternalEndDataCallbackCalled = 0;
    PlumbMccPipelineCalled = 0;
    PlumbSccPipelineCalled = 0;
    PlumbMpgPipelineCalled = 0;
    PlumbMovPipelineCalled = 0;
    DrivePipelineCalled = 0;
    DetermineFileTypeCalled = 0;

    AddReaderBuffPtr = NULL;
    SccFileInitializeFileNameStr = NULL;
    SccFileInitializeFramerate = 0;
    MccFileInitializeFileNameStr = NULL;
    MpegFileInitializeFileNameStr = NULL;
    DetermineDropFrameInputFilename = NULL;
    DetermineDropFrameArtifactPath = NULL;
    DetermineDropFrameSaveArtifacts = FALSE;
    DetermineDropFrame__isDropFrame = FALSE;
    MpegFileInitializeisDropframe = FALSE;
    DetermineDropFrame__wasSuccessful = FALSE;
    MpegFileInitializeOverrideDf = FALSE;
    MovFileInitializeFileNameStr = NULL;
    MovFileInitializeisDropframe = FALSE;
    MovFileInitializeOverrideDf = FALSE;
    fakeIsDebugInitializedVal = TRUE;
    fakeIsDebugExternallyRoutedVal = TRUE;
    PlumbMccPipelineReturn = TRUE;
    PlumbSccPipelineReturn = TRUE;
    PlumbMpgPipelineReturn = TRUE;
    PlumbMovPipelineReturn = TRUE;
    DtvccDecodeAddSinkReturn = TRUE;
    Line21DecodeAddSinkReturn = TRUE;
    MccDecodeAddSinkReturn = TRUE;
    MccFileAddSinkReturn = TRUE;
    MccFileInitializeReturn = TRUE;
    MovFileAddSinkReturn = TRUE;
    MovFileInitializeReturn = TRUE;
    MpegFileAddSinkReturn = TRUE;
    MpegFileInitializeReturn = TRUE;
    SccEncodeAddSinkReturn = TRUE;
    SccFileInitializeReturn = TRUE;
    DetermineFileTypeReturn = UNK_CAPTIONS_FILE;

    line21Codes[0].codeType = LINE21_BASIC_CHARS;
    line21Codes[0].channelNum = 1;
    line21Codes[0].fieldNum = 4;
    line21Codes[1].codeType = LINE21_BASIC_CHARS;
    line21Codes[1].channelNum = 2;
    line21Codes[1].fieldNum = 5;
    line21Codes[2].codeType = LINE21_BASIC_CHARS;
    line21Codes[2].channelNum = 3;
    line21Codes[2].fieldNum = 6;

    dtvccDatas[0].sequenceNumber = 1;
    dtvccDatas[0].serviceNumber = 4;
    dtvccDatas[1].sequenceNumber = 2;
    dtvccDatas[1].serviceNumber = 5;
    dtvccDatas[2].sequenceNumber = 3;
    dtvccDatas[2].serviceNumber = 6;
}

boolean AnySpuriousFunctionsCalled( void ) {
    if( (DetermineDropFrameCalled != 0) ||
        (DtvccDecodeAddSinkCalled != 0) ||
        (DtvccDecodeInitializeCalled != 0) ||
        (Line21DecodeAddSinkCalled != 0) ||
        (Line21DecodeInitializeCalled != 0) ||
        (MccDecodeAddSinkCalled != 0) ||
        (MccDecodeInitializeCalled != 0) ||
        (MccFileAddSinkCalled != 0) ||
        (MccFileInitializeCalled != 0) ||
        (MovFileAddSinkCalled != 0) ||
        (MovFileInitializeCalled != 0) ||
        (MpegFileAddSinkCalled != 0) ||
        (MpegFileInitializeCalled != 0) ||
        (SccEncodeAddSinkCalled != 0) ||
        (SccFileInitializeCalled != 0) ||
        (BufferPoolInitCalled != 0) ||
        (FreeBufferCalled != 0) ||
        (stubExternal608CallbackCalled != 0) ||
        (stubExternal708CallbackCalled != 0) ||
        (stubExternalEndDataCallbackCalled != 0) ||
        (PlumbMccPipelineCalled != 0) ||
        (PlumbSccPipelineCalled != 0) ||
        (PlumbMpgPipelineCalled != 0) ||
        (PlumbMovPipelineCalled != 0) ||
        (DrivePipelineCalled != 0) ||
        (DetermineFileTypeCalled != 0) ||
        (AddReaderCalled != 0) ) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/*----------------------------------------------------------------------------*/
/*--                           Stub Functions                               --*/
/*----------------------------------------------------------------------------*/

const char* VERSION = "v0.0";
const char* BUILD = "v0.0-0-g9a0959c";

boolean DtvccDecodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    DtvccDecodeAddSinkCalled++;

    return DtvccDecodeAddSinkReturn;
}

LinkInfo DtvccDecodeInitialize( Context* rootCtxPtr, boolean processOnly ) {
    LinkInfo linkInfo;
    linkInfo.sourceType = 1;

    DtvccDecodeInitializeCalled++;

    return linkInfo;
}

boolean Line21DecodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    Line21DecodeAddSinkCalled++;

    return Line21DecodeAddSinkReturn;
}

LinkInfo Line21DecodeInitialize( Context* rootCtxPtr, boolean processOnly ) {
    LinkInfo linkInfo;
    linkInfo.sourceType = 1;

    Line21DecodeInitializeCalled++;

    return linkInfo;
}

boolean MccDecodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    MccDecodeAddSinkCalled++;

    return MccDecodeAddSinkReturn;
}

LinkInfo MccDecodeInitialize( Context* rootCtxPtr ) {
    LinkInfo linkInfo;
    linkInfo.sourceType = 1;

    MccDecodeInitializeCalled++;

    return linkInfo;
}

boolean MccFileAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    MccFileAddSinkCalled++;

    return MccFileAddSinkReturn;
}

boolean MccFileInitialize( Context* rootCtxPtr, char* fileNameStr ) {
    MccFileInitializeCalled++;
    MccFileInitializeFileNameStr = fileNameStr;

    return MccFileInitializeReturn;
}

boolean MovFileAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    MovFileAddSinkCalled++;

    return MovFileAddSinkReturn;
}

boolean MovFileInitialize( Context* rootCtxPtr, char* fileNameStr, boolean overrideDropframe, boolean isDropframe, boolean bailAtTwenty ) {
    MovFileInitializeCalled++;
    MovFileInitializeFileNameStr = fileNameStr;
    MovFileInitializeisDropframe = isDropframe;
    MovFileInitializeOverrideDf = overrideDropframe;

    return MovFileInitializeReturn;
}

boolean MpegFileAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    MpegFileAddSinkCalled++;

    return MpegFileAddSinkReturn;
}

boolean MpegFileInitialize( Context* rootCtxPtr, char* fileNameStr, boolean overrideDropframe, boolean isDropframe, boolean bailAtTwenty ) {
    MpegFileInitializeCalled++;
    MpegFileInitializeFileNameStr = fileNameStr;
    MpegFileInitializeisDropframe = isDropframe;
    MpegFileInitializeOverrideDf = overrideDropframe;

    return MpegFileInitializeReturn;
}

boolean SccEncodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    SccEncodeAddSinkCalled++;

    return SccEncodeAddSinkReturn;
}

boolean SccFileInitialize( Context* rootCtxPtr, char* fileNameStr, uint32 frTimesOneHundred ) {
    SccFileInitializeCalled++;
    SccFileInitializeFileNameStr = fileNameStr;
    SccFileInitializeFramerate = frTimesOneHundred;

    return SccFileInitializeReturn;
}

void AddReader( Buffer* buffPtr ) {
    LOG(TEST_DEBUG_LEVEL_INFO, TEST_SECTION, "AddReader(%p) Called", buffPtr);
    AddReaderCalled++;
    AddReaderBuffPtr = buffPtr;
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

boolean isFramerateValid( uint32 frameRatePerSecTimesOneHundred ) {
    if( frameRatePerSecTimesOneHundred == 2400 ) return TRUE;
    else return FALSE;
}

void BufferPoolInit( void ) {
    BufferPoolInitCalled++;
}

FileType DetermineFileType( char* fileNameStr ) {
    DetermineFileTypeCalled++;
    return DetermineFileTypeReturn;
}

void DrivePipeline( FileType sourceType, Context* ctxPtr ) {
    DrivePipelineCalled++;
}

void FreeBuffer( Buffer* bufferToFreePtr ) {
    FreeBufferCalled++;
    TEST_ASSERT(bufferToFreePtr == &buffer);
}

boolean PlumbMccPipeline( Context* ctxPtr, char* inputFilename, char* outputFilename, boolean artifacts ) {
    PlumbMccPipelineCalled++;
    return PlumbMccPipelineReturn;
}

boolean PlumbSccPipeline( Context* ctxPtr, char* inputFilename, char* outputFilename, uint32 framerate, boolean artifacts ) {
    PlumbSccPipelineCalled++;
    return PlumbSccPipelineReturn;
}

boolean PlumbMpegPipeline( Context* ctxPtr, char* inputFilename, char* outputFilename, boolean artifacts, char* artifactPath, boolean bailAtTwenty ) {
    PlumbMpgPipelineCalled++;
    return PlumbMpgPipelineReturn;
}

boolean PlumbMovPipeline( Context* ctxPtr, char* inputFilename, char* outputFilename, boolean artifacts, char* artifactPath, boolean bailAtTwenty ) {
    PlumbMovPipelineCalled++;
    return PlumbMovPipelineReturn;
}

void stubExternal608Callback( CaptionTime captionTime, Line21Code line21Code ) {
    stubExternal608CallbackCalled++;
    TEST_ASSERT( memcmp(&captionTime, &buffer.captionTime, sizeof(CaptionTime)) == 0 );
    TEST_ASSERT( (memcmp(&line21Code, &line21Codes[0], sizeof(Line21Code)) == 0) || (memcmp(&line21Code, &line21Codes[1], sizeof(Line21Code)) == 0) || (memcmp(&line21Code, &line21Codes[2], sizeof(Line21Code)) == 0) );
}

void stubExternal708Callback( CaptionTime captionTime, DtvccData dtvccData ) {
    stubExternal708CallbackCalled++;
    TEST_ASSERT( memcmp(&captionTime, &buffer.captionTime, sizeof(CaptionTime)) == 0 );
    TEST_ASSERT( (memcmp(&dtvccData, &dtvccDatas[0], sizeof(DtvccData)) == 0) || (memcmp(&dtvccData, &dtvccDatas[1], sizeof(DtvccData)) == 0) || (memcmp(&dtvccData, &dtvccDatas[2], sizeof(DtvccData)) == 0) );
}

void stubExternalEndDataCallback( void ) {
    stubExternalEndDataCallbackCalled++;
}

boolean fakeIsDebugInitialized( void ) { return fakeIsDebugInitializedVal; }

boolean fakeIsDebugExternallyRouted( void ) { return fakeIsDebugExternallyRoutedVal; }

/*----------------------------------------------------------------------------*/
/*--                             Test Cases                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: ExtrnlAdptrInitialize()
 |
 | TEST CASES:
 |    1) Initialize the External Adaptor.
 |    2) Uninitialized Debug Service.
 |    3) Debug Service not Externally Routed.
 |    4) Missing CEA-608 Callback Function.
 |    5) Missing CEA-708 Callback Function.
 |    6) Missing Data End Callback Function.
 -------------------------------------------------------------------------------*/
void utest__ExtrnlAdptrInitialize( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    boolean retval;

    TEST_START("Test Case: ExtrnlAdptrInitialize() - Initialize the External Adaptor.")
    InitStubs();
    retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, BufferPoolInitCalled);
    BufferPoolInitCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ExtrnlAdptrInitialize() - Uninitialized Debug Service.")
    InitStubs();
    ERROR_EXPECTED
    fakeIsDebugInitializedVal = FALSE;
    retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ExtrnlAdptrInitialize() - Debug Service not Externally Routed.")
    InitStubs();
    ERROR_EXPECTED
    fakeIsDebugExternallyRoutedVal = FALSE;
    retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ExtrnlAdptrInitialize() - Missing CEA-608 Callback Function.")
    InitStubs();
    ERROR_EXPECTED
    retval = ExtrnlAdptrInitialize( NULL, stubExternal708Callback, stubExternalEndDataCallback );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ExtrnlAdptrInitialize() - Missing CEA-708 Callback Function.")
    InitStubs();
    ERROR_EXPECTED
    retval = ExtrnlAdptrInitialize( stubExternal608Callback, NULL, stubExternalEndDataCallback );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ExtrnlAdptrInitialize() - Missing Data End Callback Function.")
    InitStubs();
    ERROR_EXPECTED
    retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, NULL );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END
}  // utest__ExtrnlAdptrInitialize()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: ExtrnlAdptr608OutProcNextBuffer()
 |
 | TEST CASES:
 |    1) Pass Line21 Elements to Sink.
 |    2) Pass a NULL Context.
 |    3) Pass a NULL Buffer.
 |    4) Pass NULL Data in Buffer.
 -------------------------------------------------------------------------------*/
void utest__ExtrnlAdptr608OutProcNextBuffer( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    boolean retval;

    TEST_START("Test Case: ExtrnlAdptr608OutProcNextBuffer() - Pass Line21 Elements to Sink.")
    retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
    ASSERT_EQ(TRUE, retval);
    InitStubs();
    buffer.numElements = 3;
    buffer.dataPtr = (uint8*)line21Codes;
    retval = ExtrnlAdptr608OutProcNextBuffer( &fakeCtx, &buffer );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(3, stubExternal608CallbackCalled);
    ASSERT_EQ(1, FreeBufferCalled);
    FreeBufferCalled = 0;
    stubExternal608CallbackCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ExtrnlAdptr608OutProcNextBuffer() - Pass a NULL Context.")
    retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
    ASSERT_EQ(TRUE, retval);
    InitStubs();
    buffer.numElements = 3;
    buffer.dataPtr = (uint8*)line21Codes;
    retval = ExtrnlAdptr608OutProcNextBuffer( NULL, &buffer );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(3, stubExternal608CallbackCalled);
    ASSERT_EQ(1, FreeBufferCalled);
    FreeBufferCalled = 0;
    stubExternal608CallbackCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ExtrnlAdptr608OutProcNextBuffer() - Pass a NULL Buffer.")
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    InitStubs();
    retval = ExtrnlAdptr608OutProcNextBuffer( &fakeCtx, NULL );
    TEST_END

    TEST_START("Test Case: ExtrnlAdptr608OutProcNextBuffer() - Pass NULL Data in Buffer.")
    FATAL_ERROR_EXPECTED
    InitStubs();
    buffer.numElements = 0;
    buffer.dataPtr = NULL;
    retval = ExtrnlAdptr608OutProcNextBuffer( &fakeCtx, &buffer );
    TEST_END
}  // utest__ExtrnlAdptr608OutProcNextBuffer()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: ExtrnlAdptr708OutProcNextBuffer()
 |
 | TEST CASES:
 |    1) Pass DTVCC Elements to Sink.
 |    2) Pass a NULL Context.
 |    3) Pass a NULL Buffer.
 |    4) Pass NULL Data in Buffer.
 -------------------------------------------------------------------------------*/
void utest__ExtrnlAdptr708OutProcNextBuffer( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    boolean retval;

    TEST_START("Test Case: ExtrnlAdptr708OutProcNextBuffer() - Pass DTVCC Elements to Sink.")
    retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
    ASSERT_EQ(TRUE, retval);
    InitStubs();
    buffer.numElements = 3;
    buffer.dataPtr = (uint8*)dtvccDatas;
    retval = ExtrnlAdptr708OutProcNextBuffer( &fakeCtx, &buffer );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(3, stubExternal708CallbackCalled);
    ASSERT_EQ(1, FreeBufferCalled);
    FreeBufferCalled = 0;
    stubExternal708CallbackCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ExtrnlAdptr708OutProcNextBuffer() - Pass a NULL Context.")
    retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
    ASSERT_EQ(TRUE, retval);
    InitStubs();
    buffer.numElements = 3;
    buffer.dataPtr = (uint8*)dtvccDatas;
    retval = ExtrnlAdptr708OutProcNextBuffer( NULL, &buffer );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(3, stubExternal708CallbackCalled);
    ASSERT_EQ(1, FreeBufferCalled);
    FreeBufferCalled = 0;
    stubExternal708CallbackCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ExtrnlAdptr708OutProcNextBuffer() - Pass a NULL Buffer.")
    ERROR_EXPECTED
    FATAL_ERROR_EXPECTED
    InitStubs();
    retval = ExtrnlAdptr708OutProcNextBuffer( &fakeCtx, NULL );
    TEST_END

    TEST_START("Test Case: ExtrnlAdptr708OutProcNextBuffer() - Pass NULL Data in Buffer.")
    FATAL_ERROR_EXPECTED
    InitStubs();
    buffer.numElements = 0;
    buffer.dataPtr = NULL;
    retval = ExtrnlAdptr708OutProcNextBuffer( &fakeCtx, &buffer );
    TEST_END
}  // utest__ExtrnlAdptr708OutProcNextBuffer()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: ExtrnlAdptrShutdown()
 |
 | TEST CASES:
 |    1) Shutdown Successfully.
 |    2) Pass a NULL Context.
 -------------------------------------------------------------------------------*/
void utest__ExtrnlAdptrShutdown( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    boolean retval;

    TEST_START("Test Case: ExtrnlAdptrShutdown() - Shutdown Successfully.")
    retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
    ASSERT_EQ(TRUE, retval);
    InitStubs();
    retval = ExtrnlAdptrShutdown( &fakeCtx );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, stubExternalEndDataCallbackCalled);
    retval = ExtrnlAdptrShutdown( &fakeCtx );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, stubExternalEndDataCallbackCalled);
    stubExternalEndDataCallbackCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: ExtrnlAdptrShutdown() - Pass a NULL Context.")
    retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
    ASSERT_EQ(TRUE, retval);
    InitStubs();
    retval = ExtrnlAdptrShutdown( NULL );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, stubExternalEndDataCallbackCalled);
    retval = ExtrnlAdptrShutdown( NULL );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, stubExternalEndDataCallbackCalled);
    stubExternalEndDataCallbackCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END
}  // utest__ExtrnlAdptrShutdown()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: ExtrnlAdptrPlumbSccPipeline()
 |
 | TEST CASES:
 |    1) Plumb Pipeline Successfully.
 |    2) Plumb Pipeline UnSuccessfully.
 |    3) Pass a NULL Input Filename.
 |    4) Pass a NULL Output Filename.
 -------------------------------------------------------------------------------*/
void utest__ExtrnlAdptrPlumbSccPipeline( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    boolean retval;

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbSccPipeline() - Plumb Pipeline Successfully.")
    InitStubs();
    PlumbSccPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbSccPipeline( "input", "output", 2400 );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(SCC_CAPTIONS_FILE, fileType);
    ASSERT_EQ(1, PlumbSccPipelineCalled);
    PlumbSccPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbSccPipeline() - Plumb Pipeline UnSuccessfully.")
    InitStubs();
    PlumbSccPipelineReturn = FALSE;
    retval = ExtrnlAdptrPlumbSccPipeline( "input", "output", 2400 );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(SCC_CAPTIONS_FILE, fileType);
    ASSERT_EQ(1, PlumbSccPipelineCalled);
    PlumbSccPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbSccPipeline() - Pass a NULL Input Filename.")
    InitStubs();
    PlumbSccPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbSccPipeline( NULL, "output", 2400 );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(SCC_CAPTIONS_FILE, fileType);
    ASSERT_EQ(1, PlumbSccPipelineCalled);
    PlumbSccPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbSccPipeline() - Pass a NULL Output Filename.")
    InitStubs();
    PlumbSccPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbSccPipeline( "input", NULL, 2400 );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(SCC_CAPTIONS_FILE, fileType);
    ASSERT_EQ(1, PlumbSccPipelineCalled);
    PlumbSccPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END
}  // utest__ExtrnlAdptrPlumbSccPipeline()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: ExtrnlAdptrPlumbMccPipeline()
 |
 | TEST CASES:
 |    1) Plumb Pipeline Successfully.
 |    2) Plumb Pipeline UnSuccessfully.
 |    3) Pass a NULL Input Filename.
 |    4) Pass a NULL Output Filename.
 -------------------------------------------------------------------------------*/
void utest__ExtrnlAdptrPlumbMccPipeline( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    boolean retval;

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMccPipeline() - Plumb Pipeline Successfully.")
    InitStubs();
    PlumbMccPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbMccPipeline( "input", "output" );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(MCC_CAPTIONS_FILE, fileType);
    ASSERT_EQ(1, PlumbMccPipelineCalled);
    PlumbMccPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMccPipeline() - Plumb Pipeline UnSuccessfully.")
    InitStubs();
    PlumbMccPipelineReturn = FALSE;
    retval = ExtrnlAdptrPlumbMccPipeline( "input", "output" );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(MCC_CAPTIONS_FILE, fileType);
    ASSERT_EQ(1, PlumbMccPipelineCalled);
    PlumbMccPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMccPipeline() - Pass a NULL Input Filename.")
    InitStubs();
    PlumbMccPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbMccPipeline( NULL, "output" );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(MCC_CAPTIONS_FILE, fileType);
    ASSERT_EQ(1, PlumbMccPipelineCalled);
    PlumbMccPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMccPipeline() - Pass a NULL Output Filename.")
    InitStubs();
    PlumbMccPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbMccPipeline( "input", NULL );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(MCC_CAPTIONS_FILE, fileType);
    ASSERT_EQ(1, PlumbMccPipelineCalled);
    PlumbMccPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END
}  // utest__ExtrnlAdptrPlumbMccPipeline()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: ExtrnlAdptrPlumbMpgPipeline()
 |
 | TEST CASES:
 |    1) Plumb Pipeline Successfully.
 |    2) Plumb Pipeline UnSuccessfully.
 |    3) Pass a NULL Input Filename.
 |    4) Pass a NULL Output Filename.
 |    5) Pass a NULL Artifact Path.
 -------------------------------------------------------------------------------*/
void utest__ExtrnlAdptrPlumbMpgPipeline( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    boolean retval;

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMpgPipeline() - Plumb Pipeline Successfully.")
    InitStubs();
    PlumbMpgPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbMpegPipeline( "input", "output", TRUE, "artifact" );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(MPEG_BINARY_FILE, fileType);
    ASSERT_EQ(1, PlumbMpgPipelineCalled);
    PlumbMpgPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMpgPipeline() - Plumb Pipeline UnSuccessfully.")
    InitStubs();
    PlumbMpgPipelineReturn = FALSE;
    retval = ExtrnlAdptrPlumbMpegPipeline( "input", "output", TRUE, "artifact" );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(MPEG_BINARY_FILE, fileType);
    ASSERT_EQ(1, PlumbMpgPipelineCalled);
    PlumbMpgPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMpgPipeline() - Pass a NULL Input Filename.")
    InitStubs();
    PlumbMpgPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbMpegPipeline( NULL, "output", TRUE, "artifact" );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(MPEG_BINARY_FILE, fileType);
    ASSERT_EQ(1, PlumbMpgPipelineCalled);
    PlumbMpgPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMpgPipeline() - Pass a NULL Output Filename.")
    InitStubs();
    PlumbMpgPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbMpegPipeline( "input", NULL, TRUE, "artifact" );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(MPEG_BINARY_FILE, fileType);
    ASSERT_EQ(1, PlumbMpgPipelineCalled);
    PlumbMpgPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMpgPipeline() - Pass a NULL Artifact Path.")
    InitStubs();
    PlumbMpgPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbMpegPipeline( "input", "output", TRUE, NULL );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(MPEG_BINARY_FILE, fileType);
    ASSERT_EQ(1, PlumbMpgPipelineCalled);
    PlumbMpgPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END
}  // utest__ExtrnlAdptrPlumbMpgPipeline()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: ExtrnlAdptrPlumbMovPipeline()
 |
 | TEST CASES:
 |    1) Plumb Pipeline Successfully.
 |    2) Plumb Pipeline UnSuccessfully.
 |    3) Pass a NULL Input Filename.
 |    4) Pass a NULL Output Filename.
 |    5) Pass a NULL Artifact Path.
 -------------------------------------------------------------------------------*/
void utest__ExtrnlAdptrPlumbMovPipeline( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    boolean retval;

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMovPipeline() - Plumb Pipeline Successfully.")
    InitStubs();
    PlumbMovPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbMovPipeline( "input", "output", TRUE, "artifact" );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(MOV_BINARY_FILE, fileType);
    ASSERT_EQ(1, PlumbMovPipelineCalled);
    PlumbMovPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMovPipeline() - Plumb Pipeline UnSuccessfully.")
    InitStubs();
    PlumbMovPipelineReturn = FALSE;
    retval = ExtrnlAdptrPlumbMovPipeline( "input", "output", TRUE, "artifact" );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(MOV_BINARY_FILE, fileType);
    ASSERT_EQ(1, PlumbMovPipelineCalled);
    PlumbMovPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMovPipeline() - Pass a NULL Input Filename.")
    InitStubs();
    PlumbMovPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbMovPipeline( NULL, "output", TRUE, "artifact" );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(MOV_BINARY_FILE, fileType);
    ASSERT_EQ(1, PlumbMovPipelineCalled);
    PlumbMovPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMovPipeline() - Pass a NULL Output Filename.")
    InitStubs();
    PlumbMovPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbMovPipeline( "input", NULL, TRUE, "artifact" );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(MOV_BINARY_FILE, fileType);
    ASSERT_EQ(1, PlumbMovPipelineCalled);
    PlumbMovPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbMovPipeline() - Pass a NULL Artifact Path.")
    InitStubs();
    PlumbMovPipelineReturn = TRUE;
    retval = ExtrnlAdptrPlumbMovPipeline( "input", "output", TRUE, NULL );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(MOV_BINARY_FILE, fileType);
    ASSERT_EQ(1, PlumbMovPipelineCalled);
    PlumbMovPipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END
}  // utest__ExtrnlAdptrPlumbMovPipeline()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: ExtrnlAdptrDriveDecodePipeline()
 |
 | TEST CASES:
 |    1) Drive Pipeline Successfully.
 |    2) Try to Drive an Unestablished Pipeline.
 -------------------------------------------------------------------------------*/
void utest__ExtrnlAdptrDriveDecodePipeline( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE

    TEST_START("Test Case: utest__ExtrnlAdptrDriveDecodePipeline() - Drive Pipeline Successfully.")
    InitStubs();
    pipelineEstablished = TRUE;
    ExtrnlAdptrDriveDecodePipeline();
    ASSERT_EQ(1, DrivePipelineCalled);
    DrivePipelineCalled = 0;
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrDriveDecodePipeline() - Drive Pipeline Successfully.")
    InitStubs();
    pipelineEstablished = FALSE;
    ERROR_EXPECTED
    ExtrnlAdptrDriveDecodePipeline();
    ASSERT_EQ(0, DrivePipelineCalled);
    ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END
}  // utest__ExtrnlAdptrDriveDecodePipeline()

/*------------------------------------------------------------------------------
 | FUNCTION UNDER TEST: ExtrnlAdptrPlumbFileDecodePipeline()
 |
 | TEST CASES:
 |     1) Successfully Plumb a Decode Pipeline for an MPEG File.
 |     2) Successfully Plumb a Decode Pipeline for an MOV File.
 |     3) Successfully Plumb a Decode Pipeline for an SCC File.
 |     4) Successfully Plumb a Decode Pipeline for an MCC File.
 |     5) Fail Plumbing because Debug is not Initialized.
 |     6) Fail Plumbing because Debug is not Routed Externally.
 |     7) Fail Plumbing because No External 608 Data Callback.
 |     8) Fail Plumbing because No External 708 Data Callback.
 |     9) Fail Plumbing because No External End Data Callback.
 |    10) Fail Plumbing because No Input File.
 |    11) Fail Plumbing because of Indeterminate File Type.
 |    12) Fail Plumbing because of Invalid File Type.
 |    13) Fail Plumbing because of Failed Mpeg File Initialize.
 |    14) Fail Plumbing because of Failed Mpeg Sink Addition.
 |    15) Fail Plumbing because of Failed Mov File Initialize.
 |    16) Fail Plumbing because of Failed Mov Sink Addition.
 |    17) Fail Plumbing because of Failed Line21 Sink Addition. (MOV)
 |    18) Fail Plumbing because of Failed Dtvcc Sink Addition. (MPEG)
 |    19) Fail Plumbing because of Failed Scc File Initialize.
 |    20) Fail Plumbing because of Failed Scc Sink Addition.
 |    21) Fail Plumbing because of Failed Line21 Sink Addition.
 |    22) Fail Plumbing because of Invalid Framerate.
 |    23) Fail Plumbing because of Failed Mcc File Initialize.
 |    24) Fail Plumbing because of Failed Mcc Sink Addition.
 |    25) Fail Plumbing because of Failed Line21 Sink Addition.
 |    26) Fail Plumbing because of Failed Dtvcc Sink Addition.
 -------------------------------------------------------------------------------*/
void utest__ExtrnlAdptrPlumbFileDecodePipeline( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    boolean retval;

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Successfully Plumb a Decode Pipeline for an MPEG File.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MPEG_BINARY_FILE;
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mpeg filename", 2600);
        ASSERT_EQ(TRUE, retval);
        ASSERT_EQ(TRUE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, DetermineDropFrameCalled);
        ASSERT_EQ(1, MpegFileInitializeCalled);
        ASSERT_EQ(2, MpegFileAddSinkCalled);
        ASSERT_EQ(1, Line21DecodeInitializeCalled);
        ASSERT_EQ(1, Line21DecodeAddSinkCalled);
        ASSERT_EQ(1, DtvccDecodeInitializeCalled);
        ASSERT_EQ(1, DtvccDecodeAddSinkCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        DetermineDropFrameCalled = 0;
        MpegFileInitializeCalled = 0;
        MpegFileAddSinkCalled = 0;
        Line21DecodeInitializeCalled = 0;
        Line21DecodeAddSinkCalled = 0;
        DtvccDecodeInitializeCalled = 0;
        DtvccDecodeAddSinkCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Successfully Plumb a Decode Pipeline for an MOV File.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MOV_BINARY_FILE;
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mov filename", 2600);
        ASSERT_EQ(TRUE, retval);
        ASSERT_EQ(TRUE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, DetermineDropFrameCalled);
        ASSERT_EQ(1, MovFileInitializeCalled);
        ASSERT_EQ(2, MovFileAddSinkCalled);
        ASSERT_EQ(1, Line21DecodeInitializeCalled);
        ASSERT_EQ(1, Line21DecodeAddSinkCalled);
        ASSERT_EQ(1, DtvccDecodeInitializeCalled);
        ASSERT_EQ(1, DtvccDecodeAddSinkCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        DetermineDropFrameCalled = 0;
        MovFileInitializeCalled = 0;
        MovFileAddSinkCalled = 0;
        Line21DecodeInitializeCalled = 0;
        Line21DecodeAddSinkCalled = 0;
        DtvccDecodeInitializeCalled = 0;
        DtvccDecodeAddSinkCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Successfully Plumb a Decode Pipeline for an SCC File.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = SCC_CAPTIONS_FILE;
        retval = ExtrnlAdptrPlumbFileDecodePipeline("scc filename", 2400);
        ASSERT_EQ(TRUE, retval);
        ASSERT_EQ(TRUE, pipelineEstablished);
        ASSERT_EQ(1, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, SccFileInitializeCalled);
        ASSERT_EQ(1, SccEncodeAddSinkCalled);
        ASSERT_EQ(1, Line21DecodeInitializeCalled);
        ASSERT_EQ(1, Line21DecodeAddSinkCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        SccFileInitializeCalled = 0;
        SccEncodeAddSinkCalled = 0;
        Line21DecodeInitializeCalled = 0;
        Line21DecodeAddSinkCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Successfully Plumb a Decode Pipeline for an MCC File.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MCC_CAPTIONS_FILE;
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mcc filename", 2600);
        ASSERT_EQ(TRUE, retval);
        ASSERT_EQ(TRUE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, MccFileInitializeCalled);
        ASSERT_EQ(1, MccDecodeInitializeCalled);
        ASSERT_EQ(1, MccFileAddSinkCalled);
        ASSERT_EQ(2, MccDecodeAddSinkCalled);
        ASSERT_EQ(1, Line21DecodeInitializeCalled);
        ASSERT_EQ(1, Line21DecodeAddSinkCalled);
        ASSERT_EQ(1, DtvccDecodeInitializeCalled);
        ASSERT_EQ(1, DtvccDecodeAddSinkCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        MccFileInitializeCalled = 0;
        MccDecodeInitializeCalled = 0;
        MccFileAddSinkCalled = 0;
        MccDecodeAddSinkCalled = 0;
        Line21DecodeInitializeCalled = 0;
        Line21DecodeAddSinkCalled = 0;
        DtvccDecodeInitializeCalled = 0;
        DtvccDecodeAddSinkCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because Debug is not Initialized.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        fakeIsDebugInitializedVal = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mcc filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        BufferPoolInitCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because Debug is not Routed Externally.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        fakeIsDebugExternallyRoutedVal = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mcc filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        BufferPoolInitCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because No External 608 Data Callback.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        external608DataCallbackFn = NULL;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mcc filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        BufferPoolInitCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because No External 708 Data Callback.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        external708DataCallbackFn = NULL;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mcc filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        BufferPoolInitCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because No External End Data Callback.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        externalEndOfDataCallbackFn = NULL;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mcc filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        BufferPoolInitCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because No Input File.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        externalEndOfDataCallbackFn = NULL;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline(NULL, 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        BufferPoolInitCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because Indeterminate File Type.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = UNK_CAPTIONS_FILE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mcc filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because Invalid File Type.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MAX_FILE_TYPE + 1;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mcc filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Failed Mpeg File Initialize.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MPEG_BINARY_FILE;
        MpegFileInitializeReturn = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mpeg filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, DetermineDropFrameCalled);
        ASSERT_EQ(1, MpegFileInitializeCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        DetermineDropFrameCalled = 0;
        MpegFileInitializeCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Failed Mpeg Sink Addition.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MPEG_BINARY_FILE;
        MpegFileAddSinkReturn = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mpeg filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, DetermineDropFrameCalled);
        ASSERT_EQ(1, MpegFileInitializeCalled);
        ASSERT_EQ(1, MpegFileAddSinkCalled);
        ASSERT_EQ(1, Line21DecodeInitializeCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        DetermineDropFrameCalled = 0;
        MpegFileInitializeCalled = 0;
        MpegFileAddSinkCalled = 0;
        Line21DecodeInitializeCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Failed Mov File Initialize.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MOV_BINARY_FILE;
        MovFileInitializeReturn = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mov filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, DetermineDropFrameCalled);
        ASSERT_EQ(1, MovFileInitializeCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        DetermineDropFrameCalled = 0;
        MovFileInitializeCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Failed Mov Sink Addition.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MOV_BINARY_FILE;
        MovFileAddSinkReturn = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mov filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, DetermineDropFrameCalled);
        ASSERT_EQ(1, MovFileInitializeCalled);
        ASSERT_EQ(1, MovFileAddSinkCalled);
        ASSERT_EQ(1, Line21DecodeInitializeCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        DetermineDropFrameCalled = 0;
        MovFileInitializeCalled = 0;
        MovFileAddSinkCalled = 0;
        Line21DecodeInitializeCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Failed Line21 Sink Addition. (MOV)")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MOV_BINARY_FILE;
        Line21DecodeAddSinkReturn = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mov filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, DetermineDropFrameCalled);
        ASSERT_EQ(1, MovFileInitializeCalled);
        ASSERT_EQ(2, MovFileAddSinkCalled);
        ASSERT_EQ(1, Line21DecodeInitializeCalled);
        ASSERT_EQ(1, Line21DecodeAddSinkCalled);
        ASSERT_EQ(1, DtvccDecodeInitializeCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        DetermineDropFrameCalled = 0;
        MovFileInitializeCalled = 0;
        MovFileAddSinkCalled = 0;
        Line21DecodeInitializeCalled = 0;
        Line21DecodeAddSinkCalled = 0;
        DtvccDecodeInitializeCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Failed Dtvcc Sink Addition. (MPEG).")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MPEG_BINARY_FILE;
        DtvccDecodeAddSinkReturn = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mpeg filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, DetermineDropFrameCalled);
        ASSERT_EQ(1, MpegFileInitializeCalled);
        ASSERT_EQ(2, MpegFileAddSinkCalled);
        ASSERT_EQ(1, Line21DecodeInitializeCalled);
        ASSERT_EQ(1, Line21DecodeAddSinkCalled);
        ASSERT_EQ(1, DtvccDecodeInitializeCalled);
        ASSERT_EQ(1, DtvccDecodeAddSinkCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        DetermineDropFrameCalled = 0;
        MpegFileInitializeCalled = 0;
        MpegFileAddSinkCalled = 0;
        Line21DecodeInitializeCalled = 0;
        Line21DecodeAddSinkCalled = 0;
        DtvccDecodeInitializeCalled = 0;
        DtvccDecodeAddSinkCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Failed Scc File Initialize.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = SCC_CAPTIONS_FILE;
        SccFileInitializeReturn = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("scc filename", 2400);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, SccFileInitializeCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        SccFileInitializeCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Failed Scc Sink Addition.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = SCC_CAPTIONS_FILE;
        SccEncodeAddSinkReturn = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("scc filename", 2400);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, SccFileInitializeCalled);
        ASSERT_EQ(1, SccEncodeAddSinkCalled);
        ASSERT_EQ(1, Line21DecodeInitializeCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        SccFileInitializeCalled = 0;
        SccEncodeAddSinkCalled = 0;
        Line21DecodeInitializeCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Failed Line21 Sink Addition.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = SCC_CAPTIONS_FILE;
        Line21DecodeAddSinkReturn = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("scc filename", 2400);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, SccFileInitializeCalled);
        ASSERT_EQ(1, SccEncodeAddSinkCalled);
        ASSERT_EQ(1, Line21DecodeInitializeCalled);
        ASSERT_EQ(1, Line21DecodeAddSinkCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        SccFileInitializeCalled = 0;
        SccEncodeAddSinkCalled = 0;
        Line21DecodeInitializeCalled = 0;
        Line21DecodeAddSinkCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Invalid Framerate.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = SCC_CAPTIONS_FILE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("scc filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Failed Mcc File Initialize.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MCC_CAPTIONS_FILE;
        MccFileInitializeReturn = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mcc filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, MccFileInitializeCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        MccFileInitializeCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Failed Mcc Sink Addition.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MCC_CAPTIONS_FILE;
        MccFileAddSinkReturn = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mcc filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, MccFileInitializeCalled);
        ASSERT_EQ(1, MccDecodeInitializeCalled);
        ASSERT_EQ(1, MccFileAddSinkCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        MccFileInitializeCalled = 0;
        MccDecodeInitializeCalled = 0;
        MccFileAddSinkCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Failed Line21 Sink Addition.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MCC_CAPTIONS_FILE;
        Line21DecodeAddSinkReturn = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mcc filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, MccFileInitializeCalled);
        ASSERT_EQ(1, MccDecodeInitializeCalled);
        ASSERT_EQ(1, MccFileAddSinkCalled);
        ASSERT_EQ(2, MccDecodeAddSinkCalled);
        ASSERT_EQ(1, Line21DecodeInitializeCalled);
        ASSERT_EQ(1, Line21DecodeAddSinkCalled);
        ASSERT_EQ(1, DtvccDecodeInitializeCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        MccFileInitializeCalled = 0;
        MccDecodeInitializeCalled = 0;
        MccFileAddSinkCalled = 0;
        MccDecodeAddSinkCalled = 0;
        Line21DecodeInitializeCalled = 0;
        Line21DecodeAddSinkCalled = 0;
        DtvccDecodeInitializeCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END

    TEST_START("Test Case: utest__ExtrnlAdptrPlumbFileDecodePipeline() - Fail Plumbing because of Failed Dtvcc Sink Addition.")
        InitStubs();
        retval = ExtrnlAdptrInitialize( stubExternal608Callback, stubExternal708Callback, stubExternalEndDataCallback );
        ASSERT_EQ(TRUE, retval);
        DetermineFileTypeReturn = MCC_CAPTIONS_FILE;
        DtvccDecodeAddSinkReturn = FALSE;
        ERROR_EXPECTED
        retval = ExtrnlAdptrPlumbFileDecodePipeline("mcc filename", 2600);
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(FALSE, pipelineEstablished);
        ASSERT_EQ(0, numberOfShutdowns);
        ASSERT_EQ(1, DetermineFileTypeCalled);
        ASSERT_EQ(1, MccFileInitializeCalled);
        ASSERT_EQ(1, MccDecodeInitializeCalled);
        ASSERT_EQ(1, MccFileAddSinkCalled);
        ASSERT_EQ(2, MccDecodeAddSinkCalled);
        ASSERT_EQ(1, Line21DecodeInitializeCalled);
        ASSERT_EQ(1, Line21DecodeAddSinkCalled);
        ASSERT_EQ(1, DtvccDecodeInitializeCalled);
        ASSERT_EQ(1, DtvccDecodeAddSinkCalled);
        BufferPoolInitCalled = 0;
        DetermineFileTypeCalled = 0;
        MccFileInitializeCalled = 0;
        MccDecodeInitializeCalled = 0;
        MccFileAddSinkCalled = 0;
        MccDecodeAddSinkCalled = 0;
        Line21DecodeInitializeCalled = 0;
        Line21DecodeAddSinkCalled = 0;
        DtvccDecodeInitializeCalled = 0;
        DtvccDecodeAddSinkCalled = 0;
        ASSERT_EQ_MSG(FALSE, AnySpuriousFunctionsCalled(), "Unexpected Functions Called.");
    TEST_END
}  // utest__ExtrnlAdptrPlumbFileDecodePipeline()

/*----------------------------------------------------------------------------*/
/*--                             Test Suite                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | TESTED FUNCTIONS:
 |    ExtrnlAdptrInitialize()
 |    ExtrnlAdptr608OutProcNextBuffer()
 |    ExtrnlAdptr708OutProcNextBuffer()
 |    ExtrnlAdptrShutdown()
 |    ExtrnlAdptrPlumbSccPipeline()
 |    ExtrnlAdptrPlumbMccPipeline()
 |    ExtrnlAdptrPlumbMpegPipeline()
 |    ExtrnlAdptrPlumbMovPipeline()
 |    ExtrnlAdptrDriveDecodePipeline()
 |    ExtrnlAdptrPlumbFileDecodePipeline()
 -------------------------------------------------------------------------------*/
int main( int argc, char* argv[] ) {
    INIT_TEST_FRAMEWORK( argc, argv )

    TEST_SUITE_START("Test Suite: external_adaptor.c -- ExtrnlAdptrInitialize()");
    utest__ExtrnlAdptrInitialize( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: external_adaptor.c -- ExtrnlAdptr608OutProcNextBuffer()");
    utest__ExtrnlAdptr608OutProcNextBuffer( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: external_adaptor.c -- ExtrnlAdptr708OutProcNextBuffer()");
    utest__ExtrnlAdptr708OutProcNextBuffer( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: external_adaptor.c -- ExtrnlAdptrShutdown()");
    utest__ExtrnlAdptrShutdown( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: external_adaptor.c -- ExtrnlAdptrPlumbSccPipeline()");
    utest__ExtrnlAdptrPlumbSccPipeline( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: external_adaptor.c -- ExtrnlAdptrPlumbMccPipeline()");
    utest__ExtrnlAdptrPlumbMccPipeline( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: external_adaptor.c -- ExtrnlAdptrPlumbMpgPipeline()");
    utest__ExtrnlAdptrPlumbMpgPipeline( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: external_adaptor.c -- ExtrnlAdptrPlumbMovPipeline()");
    utest__ExtrnlAdptrPlumbMovPipeline( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: external_adaptor.c -- ExtrnlAdptrDriveDecodePipeline()");
    utest__ExtrnlAdptrDriveDecodePipeline( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: external_adaptor.c -- ExtrnlAdptrPlumbFileDecodePipeline()");
    utest__ExtrnlAdptrPlumbFileDecodePipeline( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    SHUTDOWN_TEST_FRAMEWORK
}  // main()

