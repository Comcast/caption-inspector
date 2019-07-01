//
// Created by Rob Taylor on 2019-04-18.
//

#include "test_engine.h"

#include "debug.h"
#include "pipeline_utils.h"

/*----------------------------------------------------------------------------*/
/*--                          Support for Stubs                             --*/
/*----------------------------------------------------------------------------*/

#define TEST_ONE                         1
// Test Case: Two Element Pipeline - Pass a Buffer down the Pipeline Successfully.
// Test Case: Two Element Pipeline - Shutdown Successfully.

#define TEST_TWO                         2
// Test Case: Two Element Pipeline - Split a Buffer Once in the Pipeline.
// Test Case: Two Element Pipeline - Spawn a Buffer during Shutdown.

#define TEST_THREE                       3
// Test Case: Two Element Pipeline - Pass a Buffer down the Pipeline UnSuccessfully.
// Test Case: Two Element Pipeline - Shutdown UnSuccessfully.

#define TEST_FOUR                        4
// Test Case: Five Element Pipeline - Pass a Buffer down the Pipeline Successfully.
// Test Case: Five Element Pipeline - Shutdown Successfully.

#define TEST_FIVE                        5
// Test Case: Five Element Pipeline - Split a Buffer Once in the Pipeline.

#define TEST_SIX                         6
// Test Case: Five Element Pipeline - Split a Buffer Multiple Times in the Pipeline.
// Test Case: Five Element Pipeline - Spawn a Buffer during Shutdown.

#define TEST_SEVEN                       7
// Test Case: Five Element Pipeline - Pass a Buffer down the Pipeline UnSuccessfully.
// Test Case: Five Element Pipeline - Shutdown UnSuccessfully.

#define TEST_EIGHT                       8
// Test Case: Two Split Pipeline - Pass a Buffer down the Pipeline Successfully.
// Test Case: Two Split Pipeline - Shutdown Successfully.

#define TEST_NINE                        9
// Test Case: Two Split Pipeline - Split a Buffer Once in the Pipeline.
// Test Case: Two Split Pipeline - Spawn a Buffer during Shutdown.

#define TEST_TEN                        10
// Test Case: Two Split Pipeline - Pass a Buffer down the Pipeline UnSuccessfully.
// Test Case: Two Split Pipeline - Shutdown UnSuccessfully.

#define TEST_ELEVEN                     11
// Test Case: Three Split Pipeline - Pass a Buffer down the Pipeline Successfully.
// Test Case: Three Split Pipeline - Shutdown Successfully.

#define TEST_TWELVE                     12
// Test Case: Three Split Pipeline - Split a Buffer Once in the Pipeline.

#define TEST_THIRTEEN                   13
// Test Case: Three Split Pipeline - Split a Buffer Multiple Times in the Pipeline.
// Test Case: Three Split Pipeline - Spawn a Buffer during Shutdown.

#define TEST_FOURTEEN                   14
// Test Case: Three Split Pipeline - Pass a Buffer down the Pipeline UnSuccessfully.
// Test Case: Three Split Pipeline - Shutdown UnSuccessfully.

#define TEST_FIFTEEN                    15
// Test Case: Five Split Pipeline - Pass a Buffer down the Pipeline Successfully.
// Test Case: Five Split Pipeline - Shutdown Successfully.

#define TEST_SIXTEEN                    16
// Test Case: Five Split Pipeline - Split a Buffer Once in the Pipeline.
// Test Case: Five Split Pipeline - Spawn a Buffer during Shutdown.

#define TEST_SEVENTEEN                  17
// Test Case: Five Split Pipeline - Pass a Buffer down the Pipeline UnSuccessfully.
// Test Case: Five Split Pipeline - Shutdown UnSuccessfully.

uint8 whichTest = 0;
uint8 numEndpointsHit = 0;

boolean dtvccDecodePassSuccessfully;
boolean line21OutputShutdownSuccessfully;
boolean mccOutputShutdownSuccessfully;

uint8 mccDecodeBufferReceived;
uint8 mccEncodeBufferReceived;
uint8 line21DecodeBufferReceived;
uint8 dtvccDecodeBufferReceived;
uint8 ccDataOutBufferReceived;
uint8 mccOutBufferReceived;
uint8 line21OutBufferReceived;
uint8 dtvccOutBufferReceived;

uint8 mccDecodeShutdownReceived;
uint8 mccEncodeShutdownReceived;
uint8 line21DecodeShutdownReceived;
uint8 dtvccDecodeShutdownReceived;
uint8 ccDataOutShutdownReceived;
uint8 mccOutShutdownReceived;
uint8 line21OutShutdownReceived;
uint8 dtvccOutShutdownReceived;

void resetMetrics( void ) {
    mccDecodeBufferReceived = 0;
    mccEncodeBufferReceived = 0;
    line21DecodeBufferReceived = 0;
    dtvccDecodeBufferReceived = 0;
    ccDataOutBufferReceived = 0;
    mccOutBufferReceived = 0;
    line21OutBufferReceived = 0;
    dtvccOutBufferReceived = 0;

    mccDecodeShutdownReceived = 0;
    mccEncodeShutdownReceived = 0;
    line21DecodeShutdownReceived = 0;
    dtvccDecodeShutdownReceived = 0;
    ccDataOutShutdownReceived = 0;
    mccOutShutdownReceived = 0;
    line21OutShutdownReceived = 0;
    dtvccOutShutdownReceived = 0;
}

#define splitBuffer(c, s)  _splitBuffer(__FILE__, __LINE__, c, s)
boolean _splitBuffer( char* file, int line, void* rootCtxPtr, Sinks* sinks ) {
    Buffer* outBuffer;
    boolean wasSuccessful;

    outBuffer = _NewBuffer(file, line, BUFFER_TYPE_BYTES, 100);
    wasSuccessful = _PassToSinks(file, line, rootCtxPtr, outBuffer, sinks);
    outBuffer = _NewBuffer(file, line, BUFFER_TYPE_BYTES, 200);
    if( (wasSuccessful == TRUE) && (TRUE == _PassToSinks(file, line, rootCtxPtr, outBuffer, sinks)) ) {
        return TRUE;
    } else {
        return FALSE;
    }
}

#define passBuffer(c, s)  _passBuffer(__FILE__, __LINE__, c, s)
boolean _passBuffer( char* file, int line, void* rootCtxPtr, Sinks* sinks ) {
    Buffer* outBuffer = _NewBuffer(file, line, BUFFER_TYPE_BYTES, 100);
    return _PassToSinks(file, line, rootCtxPtr, outBuffer, sinks);
}

#define passBufferUnsuccessfully(c, s)  _passBufferUnsuccessfully(__FILE__, __LINE__, c, s)
boolean _passBufferUnsuccessfully( char* file, int line, void* rootCtxPtr, Sinks* sinks ) {
    Buffer* outBuffer = _NewBuffer(file, line, BUFFER_TYPE_BYTES, 100);
    _PassToSinks(file, line, rootCtxPtr, outBuffer, sinks);
    return FALSE;
}

#define spawnBufferInShutdown(c, s)  _spawnBufferInShutdown(__FILE__, __LINE__, c, s)
void _spawnBufferInShutdown( char* file, int line, void* rootCtxPtr, Sinks* sinks ) {
    Buffer* outBuffer = _NewBuffer(file, line, BUFFER_TYPE_BYTES, 100);
    boolean retval = _PassToSinks(file, line, rootCtxPtr, outBuffer, sinks);
    TEST_ASSERT(retval == TRUE);
}

void dbgdumpBufferPool( uint8 );

/*----------------------------------------------------------------------------*/
/*--                            Test Drivers                                --*/
/*----------------------------------------------------------------------------*/

//***********************//
//**--  Element One  --**//
//***********************//

boolean ElementOneProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    TEST_ASSERT(inBuffer);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->sccFileCtxPtr);

    FreeBuffer(inBuffer);

    switch( whichTest ) {
        case TEST_ONE:
        case TEST_THIRTEEN:
        case TEST_SIXTEEN:
            return splitBuffer( rootCtxPtr, &((Context*)rootCtxPtr)->sccFileCtxPtr->sinks );
        case TEST_TWO:
        case TEST_THREE:
        case TEST_FOUR:
        case TEST_FIVE:
        case TEST_SIX:
        case TEST_SEVEN:
        case TEST_EIGHT:
        case TEST_NINE:
        case TEST_TEN:
        case TEST_ELEVEN:
        case TEST_TWELVE:
        case TEST_FOURTEEN:
        case TEST_FIFTEEN:
        case TEST_SEVENTEEN:
            return passBuffer( rootCtxPtr, &((Context*)rootCtxPtr)->sccFileCtxPtr->sinks );
        default:
            TEST_ASSERT(0);
            return TRUE;
    }
}  // ElementOneProcNextBuffer()

boolean ElementOneShutdown( void* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->sccFileCtxPtr);
    SccFileCtx* ctxPtr = ((Context*)rootCtxPtr)->sccFileCtxPtr;
    Sinks sinks = ctxPtr->sinks;

    switch( whichTest ) {
        case TEST_TWO:
        case TEST_NINE:
        case TEST_SIXTEEN:
            spawnBufferInShutdown( rootCtxPtr, &sinks );
            break;
        default:
            break;
    }

    free(ctxPtr);
    ((Context*)rootCtxPtr)->sccFileCtxPtr = NULL;

    return ShutdownSinks(rootCtxPtr, &sinks);
}  // ElementOneShutdown()

LinkInfo ElementOneInit( Context* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);

    rootCtxPtr->sccFileCtxPtr = malloc(sizeof(SccFileCtx));
    SccFileCtx* ctxPtr = rootCtxPtr->sccFileCtxPtr;

    InitSinks(&ctxPtr->sinks, CC_DATA___DTVCC_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___DTVCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_DECODED_708;
    linkInfo.NextBufferFnPtr = &ElementOneProcNextBuffer;
    linkInfo.ShutdownFnPtr = &ElementOneShutdown;
    return linkInfo;
}  // ElementOneInit()

boolean ElementOneAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    TEST_ASSERT(rootCtxPtr);

    return AddSink(&rootCtxPtr->sccFileCtxPtr->sinks, &linkInfo);
}  // ElementOneAddSink()

//***********************//
//**--  Element Two  --**//
//***********************//

boolean ElementTwoProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    TEST_ASSERT(inBuffer);
    TEST_ASSERT(inBuffer->dataPtr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->mccFileCtxPtr);

    FreeBuffer(inBuffer);

    switch( whichTest ) {
        case TEST_ONE:
        case TEST_TWO:
            return TRUE;
        case TEST_THREE:
            return FALSE;
        case TEST_FIFTEEN:
        case TEST_SIXTEEN:
        case TEST_SEVENTEEN:
            numEndpointsHit++;
            return TRUE;
        case TEST_FOUR:
        case TEST_FIVE:
        case TEST_SEVEN:
        case TEST_EIGHT:
        case TEST_TEN:
        case TEST_ELEVEN:
        case TEST_TWELVE:
            return passBuffer( rootCtxPtr, &((Context*)rootCtxPtr)->mccFileCtxPtr->sinks );
        case TEST_SIX:
        case TEST_NINE: // nice...
        case TEST_THIRTEEN:
            return splitBuffer( rootCtxPtr, &((Context*)rootCtxPtr)->mccFileCtxPtr->sinks );
        case TEST_FOURTEEN:
            return passBufferUnsuccessfully( rootCtxPtr, &((Context*)rootCtxPtr)->mccFileCtxPtr->sinks );
        default:
            TEST_ASSERT(0);
            return TRUE;
    }
}  // ElementTwoProcNextBuffer()

boolean ElementTwoShutdown( void* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->mccFileCtxPtr);
    MccFileCtx* ctxPtr = ((Context*)rootCtxPtr)->mccFileCtxPtr;
    Sinks sinks = ctxPtr->sinks;

    switch( whichTest ) {
        case TEST_ONE:
        case TEST_TWO:
            return TRUE;
        case TEST_THREE:
            return FALSE;
        case TEST_FIFTEEN:
        case TEST_SIXTEEN:
        case TEST_SEVENTEEN:
            numEndpointsHit++;
            return TRUE;
        default:
            break;
    }

    free(ctxPtr);
    ((Context*)rootCtxPtr)->mccFileCtxPtr = NULL;

    return ShutdownSinks(rootCtxPtr, &sinks);
}  // ElementTwoShutdown()

LinkInfo ElementTwoInit( Context* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);

    rootCtxPtr->mccFileCtxPtr = malloc(sizeof(MccFileCtx));
    MccFileCtx* ctxPtr = rootCtxPtr->mccFileCtxPtr;

    InitSinks(&ctxPtr->sinks, CC_DATA___DTVCC_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___DTVCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_DECODED_708;
    linkInfo.NextBufferFnPtr = &ElementTwoProcNextBuffer;
    linkInfo.ShutdownFnPtr = &ElementTwoShutdown;
    return linkInfo;
}  // ElementTwoInit()

boolean ElementTwoAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    TEST_ASSERT(rootCtxPtr);

    return AddSink(&rootCtxPtr->mccFileCtxPtr->sinks, &linkInfo);
}  // ElementTwoAddSink()

//*************************//
//**--  Element Three  --**//
//*************************//

boolean ElementThreeProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    TEST_ASSERT(inBuffer);
    TEST_ASSERT(inBuffer->dataPtr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->ccDataFileCtxPtr);

    FreeBuffer(inBuffer);

    switch( whichTest ) {
        case TEST_EIGHT:
        case TEST_NINE:
        case TEST_TEN:
        case TEST_FIFTEEN:
        case TEST_SIXTEEN:
        case TEST_SEVENTEEN:
            numEndpointsHit++;
            return TRUE;
        case TEST_FIVE:
        case TEST_TWELVE:
            return splitBuffer( rootCtxPtr, &((Context*)rootCtxPtr)->ccDataFileCtxPtr->sinks );
        case TEST_FOUR:
        case TEST_SIX:
        case TEST_SEVEN:
        case TEST_ELEVEN:
        case TEST_THIRTEEN:
        case TEST_FOURTEEN:
            return passBuffer( rootCtxPtr, &((Context*)rootCtxPtr)->ccDataFileCtxPtr->sinks );
        case TEST_ONE:
        case TEST_TWO:
        case TEST_THREE:
        default:
            TEST_ASSERT(0);
            return TRUE;
    }
}  // ElementThreeProcNextBuffer()

boolean ElementThreeShutdown( void* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->ccDataFileCtxPtr);
    CcDataFileCtx* ctxPtr = ((Context*)rootCtxPtr)->ccDataFileCtxPtr;
    Sinks sinks = ctxPtr->sinks;

    switch( whichTest ) {
        case TEST_EIGHT:
        case TEST_FIFTEEN:
        case TEST_SIXTEEN:
        case TEST_SEVENTEEN:
            free(ctxPtr);
            ((Context*)rootCtxPtr)->ccDataFileCtxPtr = NULL;
            numEndpointsHit++;
            return TRUE;
        case TEST_TEN:
            free(ctxPtr);
            ((Context*)rootCtxPtr)->ccDataFileCtxPtr = NULL;
            numEndpointsHit++;
            return FALSE;
        case TEST_THIRTEEN:
            spawnBufferInShutdown( rootCtxPtr, &sinks );
            break;
        default:
            break;
    }

    free(ctxPtr);
    ((Context*)rootCtxPtr)->ccDataFileCtxPtr = NULL;

    return ShutdownSinks(rootCtxPtr, &sinks);
}  // ElementThreeShutdown()

LinkInfo ElementThreeInit( Context* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(!rootCtxPtr->ccDataFileCtxPtr);

    rootCtxPtr->ccDataFileCtxPtr = malloc(sizeof(CcDataFileCtx));
    CcDataFileCtx* ctxPtr = rootCtxPtr->ccDataFileCtxPtr;

    InitSinks(&ctxPtr->sinks, CC_DATA___DTVCC_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___DTVCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_DECODED_708;
    linkInfo.NextBufferFnPtr = &ElementThreeProcNextBuffer;
    linkInfo.ShutdownFnPtr = &ElementThreeShutdown;
    return linkInfo;
}  // ElementThreeInit()

boolean ElementThreeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    TEST_ASSERT(rootCtxPtr);

    return AddSink(&rootCtxPtr->ccDataFileCtxPtr->sinks, &linkInfo);
}  // ElementThreeAddSink()

//************************//
//**--  Element Four  --**//
//************************//

boolean ElementFourProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    TEST_ASSERT(inBuffer);
    TEST_ASSERT(inBuffer->dataPtr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->mpegFileCtxPtr);

    FreeBuffer(inBuffer);

    switch( whichTest ) {
        case TEST_EIGHT:
        case TEST_NINE:
        case TEST_ELEVEN:
        case TEST_TWELVE:
        case TEST_THIRTEEN:
        case TEST_FOURTEEN:
        case TEST_FIFTEEN:
        case TEST_SIXTEEN:
        case TEST_SEVENTEEN:
            numEndpointsHit++;
            return TRUE;
        case TEST_TEN:
            numEndpointsHit++;
            return FALSE;
        case TEST_FOUR:
        case TEST_FIVE:
            return passBuffer( rootCtxPtr, &((Context*)rootCtxPtr)->mpegFileCtxPtr->sinks );
        case TEST_SIX:
            return splitBuffer( rootCtxPtr, &((Context*)rootCtxPtr)->mpegFileCtxPtr->sinks );
        case TEST_SEVEN:
            return passBufferUnsuccessfully( rootCtxPtr, &((Context*)rootCtxPtr)->mpegFileCtxPtr->sinks);
        case TEST_ONE:
        case TEST_TWO:
        case TEST_THREE:
        default:
            TEST_ASSERT(0);
            return TRUE;
    }
}  // ElementFourProcNextBuffer()

boolean ElementFourShutdown( void* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->mpegFileCtxPtr);
    MpegFileCtx* ctxPtr = ((Context*)rootCtxPtr)->mpegFileCtxPtr;
    Sinks sinks = ctxPtr->sinks;

    switch( whichTest ) {
        case TEST_SIX:
            spawnBufferInShutdown( rootCtxPtr, &sinks );
            break;
        case TEST_SEVEN:
            free(ctxPtr);
            ((Context*)rootCtxPtr)->mpegFileCtxPtr = NULL;
            ShutdownSinks(rootCtxPtr, &sinks);
            return FALSE;
        case TEST_EIGHT:
        case TEST_TEN:
        case TEST_ELEVEN:
        case TEST_TWELVE:
        case TEST_THIRTEEN:
        case TEST_FOURTEEN:
        case TEST_FIFTEEN:
        case TEST_SIXTEEN:
        case TEST_SEVENTEEN:
            free(ctxPtr);
            ((Context*)rootCtxPtr)->mpegFileCtxPtr = NULL;
            numEndpointsHit++;
            return TRUE;
        default:
            break;
    }

    free(ctxPtr);
    ((Context*)rootCtxPtr)->mpegFileCtxPtr = NULL;

    return ShutdownSinks(rootCtxPtr, &sinks);
}  // ElementFourShutdown()

LinkInfo ElementFourInit( Context* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(!rootCtxPtr->mpegFileCtxPtr);

    rootCtxPtr->mpegFileCtxPtr = malloc(sizeof(MpegFileCtx));
    MpegFileCtx* ctxPtr = rootCtxPtr->mpegFileCtxPtr;

    InitSinks(&ctxPtr->sinks, CC_DATA___DTVCC_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___DTVCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_DECODED_708;
    linkInfo.NextBufferFnPtr = &ElementFourProcNextBuffer;
    linkInfo.ShutdownFnPtr = &ElementFourShutdown;
    return linkInfo;
}  // ElementFourInit()

boolean ElementFourAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    TEST_ASSERT(rootCtxPtr);

    return AddSink(&rootCtxPtr->mpegFileCtxPtr->sinks, &linkInfo);
}  // ElementFourAddSink()

//************************//
//**--  Element Five  --**//
//************************//

boolean ElementFiveProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    TEST_ASSERT(inBuffer);
    TEST_ASSERT(inBuffer->dataPtr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->mccEncodeCtxPtr);

    FreeBuffer(inBuffer);

    switch( whichTest ) {
        case TEST_ELEVEN:
        case TEST_TWELVE:
        case TEST_THIRTEEN:
        case TEST_FOURTEEN:
        case TEST_FIFTEEN:
        case TEST_SIXTEEN:
            numEndpointsHit++;
            return TRUE;
        case TEST_FOUR:
        case TEST_FIVE:
        case TEST_SIX:
        case TEST_SEVEN:
            return TRUE;
        case TEST_SEVENTEEN:
            numEndpointsHit++;
            return FALSE;
        case TEST_ONE:
        case TEST_TWO:
        case TEST_THREE:
        case TEST_EIGHT:
        case TEST_NINE:
        case TEST_TEN:
        default:
            TEST_ASSERT(0);
            return TRUE;
    }
}  // ElementFiveProcNextBuffer()

boolean ElementFiveShutdown( void* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->mccEncodeCtxPtr);
    MccEncodeCtx* ctxPtr = ((Context*)rootCtxPtr)->mccEncodeCtxPtr;
    Sinks sinks = ctxPtr->sinks;

    switch( whichTest ) {
        case TEST_FOUR:
        case TEST_ELEVEN:
        case TEST_TWELVE:
        case TEST_THIRTEEN:
        case TEST_FOURTEEN:
        case TEST_FIFTEEN:
        case TEST_SIXTEEN:
        case TEST_SEVENTEEN:
            numEndpointsHit++;
            free(ctxPtr);
            ((Context*)rootCtxPtr)->mccEncodeCtxPtr = NULL;
            return TRUE;
        default:
            break;
    }

    free(ctxPtr);
    ((Context*)rootCtxPtr)->mccEncodeCtxPtr = NULL;

    return ShutdownSinks(rootCtxPtr, &sinks);
}  // ElementFiveShutdown()

LinkInfo ElementFiveInit( Context* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(!rootCtxPtr->mccEncodeCtxPtr);

    rootCtxPtr->mccEncodeCtxPtr = malloc(sizeof(MccEncodeCtx));
    MccEncodeCtx* ctxPtr = rootCtxPtr->mccEncodeCtxPtr;

    InitSinks(&ctxPtr->sinks, CC_DATA___DTVCC_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___DTVCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_DECODED_708;
    linkInfo.NextBufferFnPtr = &ElementFiveProcNextBuffer;
    linkInfo.ShutdownFnPtr = &ElementFiveShutdown;
    return linkInfo;
}  // ElementFiveInit()

boolean ElementFiveAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    TEST_ASSERT(rootCtxPtr);

    return AddSink(&rootCtxPtr->mccEncodeCtxPtr->sinks, &linkInfo);
}  // ElementFiveAddSink()

//************************//
//**--  Element Six  --**//
//************************//

boolean ElementSixProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    TEST_ASSERT(inBuffer);
    TEST_ASSERT(inBuffer->dataPtr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->line21DecodeCtxPtr);

    FreeBuffer(inBuffer);

    switch( whichTest ) {
        case TEST_ELEVEN:
        case TEST_TWELVE:
        case TEST_THIRTEEN:
        case TEST_FOURTEEN:
        case TEST_FIFTEEN:
        case TEST_SIXTEEN:
        case TEST_SEVENTEEN:
            numEndpointsHit++;
            return TRUE;
        case TEST_ONE:
        case TEST_TWO:
        case TEST_THREE:
        case TEST_FOUR:
        case TEST_FIVE:
        case TEST_SIX:
        case TEST_SEVEN:
        case TEST_EIGHT:
        case TEST_NINE:
        case TEST_TEN:
        default:
            TEST_ASSERT(0);
            return TRUE;
    }
}  // ElementSixProcNextBuffer()

boolean ElementSixShutdown( void* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->line21DecodeCtxPtr);
    Line21DecodeCtx* ctxPtr = ((Context*)rootCtxPtr)->line21DecodeCtxPtr;
    Sinks sinks = ctxPtr->sinks;

    switch( whichTest ) {
        case TEST_ELEVEN:
        case TEST_TWELVE:
        case TEST_THIRTEEN:
        case TEST_FIFTEEN:
        case TEST_SIXTEEN:
            numEndpointsHit++;
            break;
        case TEST_FOURTEEN:
        case TEST_SEVENTEEN:
            numEndpointsHit++;
            free(ctxPtr);
            ((Context*)rootCtxPtr)->line21DecodeCtxPtr = NULL;
            return FALSE;
        default:
            break;
    }

    free(ctxPtr);
    ((Context*)rootCtxPtr)->line21DecodeCtxPtr = NULL;

    return TRUE;
}  // ElementSixShutdown()

LinkInfo ElementSixInit( Context* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(!rootCtxPtr->line21DecodeCtxPtr);

    rootCtxPtr->line21DecodeCtxPtr = malloc(sizeof(Line21DecodeCtx));
    Line21DecodeCtx* ctxPtr = rootCtxPtr->line21DecodeCtxPtr;

    InitSinks(&ctxPtr->sinks, CC_DATA___DTVCC_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___DTVCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_DECODED_708;
    linkInfo.NextBufferFnPtr = &ElementSixProcNextBuffer;
    linkInfo.ShutdownFnPtr = &ElementSixShutdown;
    return linkInfo;
}  // ElementSixInit()

boolean ElementSixAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    TEST_ASSERT(rootCtxPtr);

    return AddSink(&rootCtxPtr->line21DecodeCtxPtr->sinks, &linkInfo);
}  // ElementSixAddSink()

/*----------------------------------------------------------------------------*/
/*--                    Active Stub Functions and Mocks                     --*/
/*----------------------------------------------------------------------------*/

//***********************//
//**==---------------==**//
//**==--  Sources  --==**//
//**==---------------==**//
//***********************//

//***********************//
//**--  MCC File In  --**//
//***********************//

boolean MccFileInitialize( Context* rootCtxPtr, char* fileNameStr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(!rootCtxPtr->mccFileCtxPtr);

    rootCtxPtr->mccFileCtxPtr = malloc(sizeof(MccFileCtx));
    MccFileCtx* ctxPtr = rootCtxPtr->mccFileCtxPtr;

    strcpy( ctxPtr->captionFileName, fileNameStr );

    InitSinks(&ctxPtr->sinks, MCC_FILE___MCC_DATA);

    return TRUE;
}  // MccFileInitialize()

boolean MccFileAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(rootCtxPtr->mccFileCtxPtr);
    TEST_ASSERT(linkInfo.sourceType == DATA_TYPE_MCC_DATA );

    return AddSink(&rootCtxPtr->mccFileCtxPtr->sinks, &linkInfo);
}  // MccFileAddSink()

boolean MccFileProcNextBuffer( Context* rootCtxPtr, boolean* isDonePtr ) {
    TEST_ASSERT(isDonePtr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(rootCtxPtr->mccFileCtxPtr);

    MccFileCtx* ctxPtr = rootCtxPtr->mccFileCtxPtr;
    TEST_ASSERT( ctxPtr->sinks.numSinks != 0 );

    if( *isDonePtr == FALSE ) {
        Buffer *newBufferPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
        return PassToSinks(rootCtxPtr, newBufferPtr, &ctxPtr->sinks);
    } else {
        return ShutdownSinks(rootCtxPtr , &ctxPtr->sinks);
    }
} // MccFileProcNextBuffer()

//************************//
//**--  MPEG File In  --**//
//************************//

boolean MpegFileInitialize( Context* rootCtxPtr, char* fileNameStr, boolean overrideDropframe, boolean isDropframe ) {
    TEST_ASSERT(fileNameStr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(!rootCtxPtr->mpegFileCtxPtr);

    rootCtxPtr->mpegFileCtxPtr = malloc(sizeof(MpegFileCtx));
    MpegFileCtx* ctxPtr = rootCtxPtr->mpegFileCtxPtr;

    strcpy((char*)ctxPtr->buffer, fileNameStr);

    InitSinks(&ctxPtr->sinks, MPG_FILE___CC_DATA);

    return TRUE;
}  // MpegFileInitialize()

boolean MpegFileAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(rootCtxPtr->mpegFileCtxPtr);
    TEST_ASSERT(linkInfo.sourceType == DATA_TYPE_CC_DATA);

    return AddSink(&rootCtxPtr->mpegFileCtxPtr->sinks, &linkInfo);
}  // MpegFileAddSink()

boolean MpegFileProcNextBuffer( Context* rootCtxPtr, boolean* isDonePtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(rootCtxPtr->mpegFileCtxPtr);

    MpegFileCtx* ctxPtr = rootCtxPtr->mpegFileCtxPtr;
    TEST_ASSERT(ctxPtr->sinks.numSinks != 0);

    if( *isDonePtr == FALSE ) {
        Buffer *newBufferPtr = NewBuffer(BUFFER_TYPE_BYTES, 500);
        return PassToSinks(rootCtxPtr, newBufferPtr, &ctxPtr->sinks);
    } else {
        return ShutdownSinks(rootCtxPtr , &ctxPtr->sinks);
    }
} // MpegFileProcNextBuffer()

//**************************//
//**==------------------==**//
//**==--  Transforms  --==**//
//**==------------------==**//
//**************************//

//************************//
//**--  DTVCC Decode  --**//
//************************//

boolean DtvccDecodeProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    TEST_ASSERT(inBuffer);
    TEST_ASSERT(inBuffer->dataPtr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->dtvccDecodeCtxPtr);
    DtvccDecodeCtx* ctxPtr = ((Context*)rootCtxPtr)->dtvccDecodeCtxPtr;

    dtvccDecodeBufferReceived++;

    FreeBuffer(inBuffer);
    if( ctxPtr->sinks.numSinks != 0 ) {
        Buffer *outBuffer = NewBuffer(BUFFER_TYPE_LINE_21, 100);
        boolean retval = PassToSinks(rootCtxPtr, outBuffer, &ctxPtr->sinks);
        TEST_ASSERT(retval);
    }

    return dtvccDecodePassSuccessfully;
}  // DtvccDecodeProcNextBuffer()

boolean DtvccDecodeShutdown( void* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->dtvccDecodeCtxPtr);
    DtvccDecodeCtx* ctxPtr = ((Context*)rootCtxPtr)->dtvccDecodeCtxPtr;
    Sinks sinks = ctxPtr->sinks;

    dtvccDecodeShutdownReceived++;

    free(ctxPtr);
    ((Context*)rootCtxPtr)->dtvccDecodeCtxPtr = NULL;

    return ShutdownSinks(rootCtxPtr, &sinks);
}  // DtvccDecodeShutdown()

boolean DtvccDecodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(rootCtxPtr->dtvccDecodeCtxPtr);
    TEST_ASSERT(linkInfo.sourceType == DATA_TYPE_DECODED_708);

    return AddSink(&rootCtxPtr->dtvccDecodeCtxPtr->sinks, &linkInfo);
}  // Line21DecodeAddSink()

LinkInfo DtvccDecodeInitialize( Context* rootCtxPtr, boolean processOnly ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(!rootCtxPtr->dtvccDecodeCtxPtr);

    rootCtxPtr->dtvccDecodeCtxPtr = malloc(sizeof(DtvccDecodeCtx));
    DtvccDecodeCtx* ctxPtr = rootCtxPtr->dtvccDecodeCtxPtr;

    InitSinks(&ctxPtr->sinks, CC_DATA___DTVCC_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___DTVCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_DECODED_708;
    linkInfo.NextBufferFnPtr = &DtvccDecodeProcNextBuffer;
    linkInfo.ShutdownFnPtr = &DtvccDecodeShutdown;
    return linkInfo;
}  // DtvccDecodeInitialize()

//**************************//
//**--  Line 21 Decode  --**//
//**************************//

boolean Line21DecodeProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    TEST_ASSERT(inBuffer);
    TEST_ASSERT(inBuffer->dataPtr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->line21DecodeCtxPtr);
    Line21DecodeCtx* ctxPtr = ((Context*)rootCtxPtr)->line21DecodeCtxPtr;

    line21DecodeBufferReceived++;

    FreeBuffer(inBuffer);

    if( ctxPtr->sinks.numSinks != 0 ) {
        Buffer *outBuffer = NewBuffer(BUFFER_TYPE_LINE_21, 100);
        return PassToSinks(rootCtxPtr, outBuffer, &ctxPtr->sinks);
    } else {
        return TRUE;
    }
}  // Line21DecodeProcNextBuffer()

boolean Line21DecodeShutdown( void* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->line21DecodeCtxPtr);
    Line21DecodeCtx* ctxPtr = ((Context*)rootCtxPtr)->line21DecodeCtxPtr;
    Sinks sinks = ctxPtr->sinks;

    line21DecodeShutdownReceived++;

    free(ctxPtr);
    ((Context*)rootCtxPtr)->line21DecodeCtxPtr = NULL;
    return ShutdownSinks(rootCtxPtr, &sinks);
}  // Line21DecodeShutdown()

boolean Line21DecodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(rootCtxPtr->line21DecodeCtxPtr);
    TEST_ASSERT(linkInfo.sourceType == DATA_TYPE_DECODED_608);

    return AddSink(&rootCtxPtr->line21DecodeCtxPtr->sinks, &linkInfo);
}  // Line21DecodeAddSink()

LinkInfo Line21DecodeInitialize( Context* rootCtxPtr, boolean processOnly ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(!rootCtxPtr->line21DecodeCtxPtr);

    rootCtxPtr->line21DecodeCtxPtr = malloc(sizeof(Line21DecodeCtx));
    Line21DecodeCtx* ctxPtr = rootCtxPtr->line21DecodeCtxPtr;

    InitSinks(&ctxPtr->sinks, CC_DATA___LINE21_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___LINE21_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_DECODED_608;
    linkInfo.NextBufferFnPtr = &Line21DecodeProcNextBuffer;
    linkInfo.ShutdownFnPtr = &Line21DecodeShutdown;
    return linkInfo;
}  // Line21DecodeInitialize()

//**********************//
//**--  MCC Decode  --**//
//**********************//

boolean MccDecodeProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    TEST_ASSERT(inBuffer);
    TEST_ASSERT(inBuffer->dataPtr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->mccDecodeCtxPtr);

    mccDecodeBufferReceived++;

    Buffer* outBuffer = NewBuffer(BUFFER_TYPE_LINE_21, 100);
    FreeBuffer(inBuffer);

    return PassToSinks(rootCtxPtr, outBuffer, &((Context*)rootCtxPtr)->mccDecodeCtxPtr->sinks);
}  // MccDecodeProcNextBuffer()

boolean MccDecodeShutdown( void* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->mccDecodeCtxPtr);
    Sinks sinks = ((Context*)rootCtxPtr)->mccDecodeCtxPtr->sinks;

    mccDecodeShutdownReceived++;

    free(((Context*)rootCtxPtr)->mccDecodeCtxPtr);
    ((Context*)rootCtxPtr)->mccDecodeCtxPtr = NULL;

    return ShutdownSinks(rootCtxPtr, &sinks);
}  // MccDecodeShutdown()

boolean MccDecodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(rootCtxPtr->mccDecodeCtxPtr);
    TEST_ASSERT(linkInfo.sourceType == DATA_TYPE_CC_DATA);

    return AddSink(&rootCtxPtr->mccDecodeCtxPtr->sinks, &linkInfo);
}  // MccDecodeAddSink()

LinkInfo MccDecodeInitialize( Context* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(!rootCtxPtr->mccDecodeCtxPtr);

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

//**********************//
//**--  MCC Encode  --**//
//**********************//

boolean MccEncodeAddSink( Context* ctxPtr, LinkInfo linkInfo ) {
    TEST_ASSERT(ctxPtr);
    TEST_ASSERT(ctxPtr->mccEncodeCtxPtr);
    TEST_ASSERT(linkInfo.sourceType == DATA_TYPE_MCC_DATA);

    return AddSink(&ctxPtr->mccEncodeCtxPtr->sinks, &linkInfo);
}  // MccEncodeAddSink()

boolean MccEncodeProcNextBuffer( void* rootCtxPtr, Buffer* inBuffer ) {
    TEST_ASSERT(inBuffer);
    TEST_ASSERT(inBuffer->dataPtr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->mccEncodeCtxPtr);
    MccEncodeCtx* ctxPtr = ((Context*)rootCtxPtr)->mccEncodeCtxPtr;

    mccEncodeBufferReceived++;

    Buffer* outBuffer = NewBuffer(BUFFER_TYPE_LINE_21, 100);
    FreeBuffer(inBuffer);

    return PassToSinks(rootCtxPtr, outBuffer, &ctxPtr->sinks);
}  // MccEncodeProcNextBuffer()

boolean MccEncodeShutdown( void* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->mccEncodeCtxPtr);
    Sinks sinks = ((Context*)rootCtxPtr)->mccEncodeCtxPtr->sinks;

    mccEncodeShutdownReceived++;

    free(((Context*)rootCtxPtr)->mccEncodeCtxPtr);
    ((Context*)rootCtxPtr)->mccEncodeCtxPtr = NULL;

    return ShutdownSinks(rootCtxPtr, &sinks);
}  // MccEncodeShutdown()

LinkInfo MccEncodeInitialize( Context* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(!rootCtxPtr->mccEncodeCtxPtr);

    rootCtxPtr->mccEncodeCtxPtr = malloc(sizeof(MccEncodeCtx));
    MccEncodeCtx* ctxPtr = rootCtxPtr->mccEncodeCtxPtr;

    InitSinks(&ctxPtr->sinks, CC_DATA___MCC_DATA);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___MCC_DATA;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_MCC_DATA;
    linkInfo.NextBufferFnPtr = &MccEncodeProcNextBuffer;
    linkInfo.ShutdownFnPtr = &MccEncodeShutdown;
    return linkInfo;
}  // MccEncodeInitialize()

//*********************//
//**==-------------==**//
//**==--  Sinks  --==**//
//**==-------------==**//
//*********************//

//***********************//
//**--  CC Data Out  --**//
//***********************//

boolean CcDataOutProcNextBuffer( void* rootCtxPtr, Buffer* buffPtr ) {
    TEST_ASSERT(buffPtr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->ccDataOutputCtxPtr);
    CcDataOutputCtx* ctxPtr = ((Context*)rootCtxPtr)->ccDataOutputCtxPtr;

    ccDataOutBufferReceived++;

    FreeBuffer(buffPtr);
    return TRUE;
} // CcDataOutProcNextBuffer()

boolean CcDataOutShutdown( void* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->ccDataOutputCtxPtr);

    ccDataOutShutdownReceived++;

    free(((Context*)rootCtxPtr)->ccDataOutputCtxPtr);
    ((Context*)rootCtxPtr)->ccDataOutputCtxPtr = NULL;
    return TRUE;
} // CcDataOutShutdown()

LinkInfo CcDataOutInitialize( Context* rootCtxPtr, char* outputFileNameStr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(!rootCtxPtr->ccDataOutputCtxPtr);

    rootCtxPtr->ccDataOutputCtxPtr = malloc(sizeof(CcDataOutputCtx));
    CcDataOutputCtx* ctxPtr = rootCtxPtr->ccDataOutputCtxPtr;

    strcpy(ctxPtr->ccdFileName, outputFileNameStr);

    LinkInfo linkInfo;
    linkInfo.linkType = CC_DATA___TEXT_FILE;
    linkInfo.sourceType = DATA_TYPE_CC_DATA;
    linkInfo.sinkType = DATA_TYPE_CC_DATA_TXT_FILE;
    linkInfo.NextBufferFnPtr = &CcDataOutProcNextBuffer;
    linkInfo.ShutdownFnPtr = &CcDataOutShutdown;
    return linkInfo;
} // CcDataOutInitialize()

//*********************//
//**--  DTVCC Out  --**//
//*********************//

boolean DtvccOutProcNextBuffer( void* rootCtxPtr, Buffer* buffPtr ) {
    TEST_ASSERT(buffPtr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->dtvccOutputCtxPtr);
    DtvccOutputCtx* ctxPtr = ((Context*)rootCtxPtr)->dtvccOutputCtxPtr;

    dtvccOutBufferReceived++;

    FreeBuffer(buffPtr);
    return TRUE;
} // DtvccOutProcNextBuffer()

boolean DtvccOutShutdown( void* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->dtvccOutputCtxPtr);

    dtvccOutShutdownReceived++;

    free(((Context*)rootCtxPtr)->dtvccOutputCtxPtr);
    ((Context*)rootCtxPtr)->dtvccOutputCtxPtr = NULL;
    return TRUE;
} // DtvcceOutShutdown()

LinkInfo DtvccOutInitialize( Context* rootCtxPtr, char* outputFileNameStr, boolean nullEtxSuppressed, boolean msNotFrame ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(!rootCtxPtr->dtvccOutputCtxPtr);

    rootCtxPtr->dtvccOutputCtxPtr = malloc(sizeof(DtvccOutputCtx));
    DtvccOutputCtx* ctxPtr = rootCtxPtr->dtvccOutputCtxPtr;

    strcpy(ctxPtr->baseFileName, outputFileNameStr);

    LinkInfo linkInfo;
    linkInfo.linkType = DTVCC_DATA___TEXT_FILE;
    linkInfo.sourceType = DATA_TYPE_DECODED_708;
    linkInfo.sinkType = DATA_TYPE_708_TXT_FILE;
    linkInfo.NextBufferFnPtr = &DtvccOutProcNextBuffer;
    linkInfo.ShutdownFnPtr = &DtvccOutShutdown;
    return linkInfo;
}  // DtvccOutInitialize()

//***********************//
//**--  Line 21 Out  --**//
//***********************//

boolean Line21OutProcNextBuffer( void* rootCtxPtr, Buffer* buffPtr ) {
    TEST_ASSERT(buffPtr);
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->line21OutputCtxPtr);
    Line21OutputCtx* ctxPtr = ((Context*)rootCtxPtr)->line21OutputCtxPtr;

    line21OutBufferReceived++;

    FreeBuffer(buffPtr);
    return TRUE;
} // Line21OutProcNextBuffer()

boolean Line21OutShutdown( void* rootCtxPtr ) {
    TEST_ASSERT(rootCtxPtr);
    TEST_ASSERT(((Context*)rootCtxPtr)->line21OutputCtxPtr);
    Line21OutputCtx* ctxPtr = ((Context*)rootCtxPtr)->line21OutputCtxPtr;

    line21OutShutdownReceived++;

    free(ctxPtr);
    ((Context*)rootCtxPtr)->line21OutputCtxPtr = NULL;
    return line21OutputShutdownSuccessfully;
} // Line21OutShutdown()

LinkInfo Line21OutInitialize( Context* ctxPtr, char* outputFileNameStr ) {
    TEST_ASSERT(ctxPtr);
    TEST_ASSERT(!ctxPtr->line21OutputCtxPtr);

    ctxPtr->line21OutputCtxPtr = malloc(sizeof(Line21OutputCtx));
    strcpy(ctxPtr->line21OutputCtxPtr->baseFileName, outputFileNameStr);

    LinkInfo linkInfo;
    linkInfo.linkType = LINE21_DATA___TEXT_FILE;
    linkInfo.sourceType = DATA_TYPE_DECODED_608;
    linkInfo.sinkType = DATA_TYPE_608_TXT_FILE;
    linkInfo.NextBufferFnPtr = &Line21OutProcNextBuffer;
    linkInfo.ShutdownFnPtr = &Line21OutShutdown;
    return linkInfo;
}  // Line21OutInitialize()

//*******************//
//**--  MCC Out  --**//
//*******************//

boolean MccOutProcNextBuffer( void* rootCtxPtr, Buffer* buffPtr ) {
    ASSERT(buffPtr);
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->mccOutputCtxPtr);

    mccOutBufferReceived++;

    FreeBuffer(buffPtr);
    return TRUE;
} // MccOutProcNextBuffer()

boolean MccOutShutdown( void* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->mccOutputCtxPtr);

    mccOutShutdownReceived++;

    free(((Context*)rootCtxPtr)->mccOutputCtxPtr);
    ((Context*)rootCtxPtr)->mccOutputCtxPtr = NULL;
    return mccOutputShutdownSuccessfully;
} // MccOutShutdown()

LinkInfo MccOutInitialize( Context* ctxPtr, char* outputFileNameStr ) {
    ASSERT(ctxPtr);
    ASSERT(!ctxPtr->mccOutputCtxPtr);

    ctxPtr->mccOutputCtxPtr = malloc(sizeof(MccOutputCtx));
    strcpy(ctxPtr->mccOutputCtxPtr->mccFileName, outputFileNameStr);

    LinkInfo linkInfo;
    linkInfo.linkType = MCC_DATA___TEXT_FILE;
    linkInfo.sourceType = DATA_TYPE_MCC_DATA;
    linkInfo.sinkType = DATA_TYPE_MCC_DATA_TXT_FILE;
    linkInfo.NextBufferFnPtr = &MccOutProcNextBuffer;
    linkInfo.ShutdownFnPtr = &MccOutShutdown;
    return linkInfo;
}  // MccOutInitialize()

/*----------------------------------------------------------------------------*/
/*--                        Passive Stub Functions                          --*/
/*----------------------------------------------------------------------------*/

boolean DetermineDropFrame( char* fileNameStr, boolean saveMediaInfo, char* artifactPath, boolean* isDropFramePtr ) { return TRUE; }
boolean SccEncodeAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) { return TRUE; }
LinkInfo SccEncodeInitialize( Context* rootCtxPtr ) { LinkInfo linkInfo; return linkInfo; }
boolean SccFileProcNextBuffer( Context* rootCtxPtr, boolean* isDonePtr ) { return TRUE; }
boolean SccFileAddSink( Context* rootCtxPtr, LinkInfo linkInfo ) { return TRUE; }
boolean SccFileInitialize( Context* rootCtxPtr, char* fileNameStr, uint32 frTimesOneHundred ) { return TRUE; }

/*----------------------------------------------------------------------------*/
/*--                             Test Cases                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | INTEGRATION TESTS: Testing a Pipeline with Two Elements.
 |
 | PIPELINE:
 |    +-------------+      +-------------+
 |    | Element One | ---> | Element Two |
 |    +-------------+      +-------------+
 |
 | TEST CASES:
 |    1) Establish the Pipeline.
 |    2) Pass a Buffer down the Pipeline Successfully.
 |    3) Shutdown Successfully.
 |    4) Split a Buffer Once in the Pipeline.
 |    5) Spawn a Buffer during Shutdown.
 |    6) Pass a Buffer down the Pipeline UnSuccessfully.
 |    7) Shutdown UnSuccessfully.
 -------------------------------------------------------------------------------*/
void itest__TwoElementPipelineTests( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    Context ctx;
    Buffer* buffPtr;
    boolean retval;

    memset(&ctx, 0, sizeof(Context));

    whichTest = TEST_ONE;

    TEST_START("Test Case: Two Element Pipeline - Establish the Pipeline.")
    BufferPoolInit();
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    TEST_END

    TEST_START("Test Case: Two Element Pipeline - Pass a Buffer down the Pipeline Successfully.")
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(100, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Two Element Pipeline - Shutdown Successfully.")
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    whichTest = TEST_TWO;

    TEST_START("Test Case: Two Element Pipeline - Split a Buffer Once in the Pipeline.")
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 200);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(200, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Two Element Pipeline - Spawn a Buffer during Shutdown.")
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    whichTest = TEST_THREE;

    TEST_START("Test Case: Two Element Pipeline - Pass a Buffer down the Pipeline UnSuccessfully.")
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(100, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Two Element Pipeline - Shutdown UnSuccessfully.")
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

}  // itest__TwoElementPipelineTests()

/*------------------------------------------------------------------------------
 | INTEGRATION TESTS: Testing a Pipeline with Five Elements.
 |
 | PIPELINE:
 |    +---------+      +---------+      +---------+      +---------+      +---------+
 |    | Elmnt 1 | ---> | Elmnt 2 | ---> | Elmnt 3 | ---> | Elmnt 4 | ---> | Elmnt 5 |
 |    +---------+      +---------+      +---------+      +---------+      +---------+
 |
 | TEST CASES:
 |    1) Establish the Pipeline.
 |    2) Pass a Buffer down the Pipeline Successfully.
 |    3) Shutdown Successfully.
 |    4) Split a Buffer Once in the Pipeline.
 |    5) Split a Buffer Multiple Times in the Pipeline.
 |    6) Spawn a Buffer during Shutdown.
 |    7) Pass a Buffer down the Pipeline UnSuccessfully.
 |    8) Shutdown UnSuccessfully.
 -------------------------------------------------------------------------------*/
void itest__FiveElementPipelineTests( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    Context ctx;
    Buffer* buffPtr;
    boolean retval;

    memset(&ctx, 0, sizeof(Context));

    whichTest = TEST_FOUR;

    TEST_START("Test Case: Five Element Pipeline - Establish the Pipeline.")
    BufferPoolInit();
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementTwoAddSink(&ctx, ElementThreeInit(&ctx));
    ElementThreeAddSink(&ctx, ElementFourInit(&ctx));
    ElementFourAddSink(&ctx, ElementFiveInit(&ctx));
    TEST_END

    TEST_START("Test Case: Five Element Pipeline - Pass a Buffer down the Pipeline Successfully.")
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(100, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Five Element Pipeline - Shutdown Successfully.")
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    whichTest = TEST_FIVE;

    TEST_START("Test Case: Five Element Pipeline - Split a Buffer Once in the Pipeline.")
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementTwoAddSink(&ctx, ElementThreeInit(&ctx));
    ElementThreeAddSink(&ctx, ElementFourInit(&ctx));
    ElementFourAddSink(&ctx, ElementFiveInit(&ctx));
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 200);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(200, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    whichTest = TEST_SIX;

    TEST_START("Test Case: Five Element Pipeline - Split a Buffer Multiple Times in the Pipeline.")
    memset(&ctx, 0, sizeof(Context));
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementTwoAddSink(&ctx, ElementThreeInit(&ctx));
    ElementThreeAddSink(&ctx, ElementFourInit(&ctx));
    ElementFourAddSink(&ctx, ElementFiveInit(&ctx));
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 200);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(200, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Five Element Pipeline - Spawn a Buffer during Shutdown.")
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    whichTest = TEST_SEVEN;

    TEST_START("Test Case: Five Element Pipeline - Pass a Buffer down the Pipeline UnSuccessfully.")
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementTwoAddSink(&ctx, ElementThreeInit(&ctx));
    ElementThreeAddSink(&ctx, ElementFourInit(&ctx));
    ElementFourAddSink(&ctx, ElementFiveInit(&ctx));
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(100, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Five Element Pipeline - Shutdown UnSuccessfully.")
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END
}  // itest__FiveElementPipelineTests()

/*------------------------------------------------------------------------------
 | INTEGRATION TESTS: Testing a Pipeline with Two Splits.
 |
 | PIPELINE:                                     +---------------+
 |                                           +-> | Element Three |
 |    +-------------+      +-------------+   |   +---------------+
 |    | Element One | ---> | Element Two | --|
 |    +-------------+      +-------------+   |   +--------------+
 |                                           +-> | Element Four |
 |                                               +--------------+
 | TEST CASES:
 |    1) Establish the Pipeline.
 |    2) Pass a Buffer down the Pipeline Successfully.
 |    3) Shutdown Successfully.
 |    4) Split a Buffer Once in the Pipeline.
 |    5) Spawn a Buffer during Shutdown.
 |    6) Pass a Buffer down the Pipeline UnSuccessfully.
 |    7) Shutdown UnSuccessfully.
 -------------------------------------------------------------------------------*/
void itest__TwoSplitPipelineTests( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    Context ctx;
    Buffer* buffPtr;
    boolean retval;

    memset(&ctx, 0, sizeof(Context));

    whichTest = TEST_EIGHT;

    TEST_START("Test Case: Two Split Pipeline - Establish the Pipeline.")
    BufferPoolInit();
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementTwoAddSink(&ctx, ElementThreeInit(&ctx));
    ElementTwoAddSink(&ctx, ElementFourInit(&ctx));
    TEST_END

    TEST_START("Test Case: Two Split Pipeline - Pass a Buffer down the Pipeline Successfully.")
    numEndpointsHit = 0;
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(100, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(2, numEndpointsHit);
    TEST_END

    TEST_START("Test Case: Two Split Pipeline - Shutdown Successfully.")
    numEndpointsHit = 0;
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(2, numEndpointsHit);
    TEST_END

    whichTest = TEST_NINE;

    TEST_START("Test Case: Two Split Pipeline - Split a Buffer Once in the Pipeline.")
    numEndpointsHit = 0;
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementTwoAddSink(&ctx, ElementThreeInit(&ctx));
    ElementTwoAddSink(&ctx, ElementFourInit(&ctx));
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 200);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(200, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(4, numEndpointsHit);
    TEST_END

    TEST_START("Test Case: Two Split Pipeline - Spawn a Buffer during Shutdown.")
    numEndpointsHit = 0;
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(4, numEndpointsHit);
    TEST_END

    whichTest = TEST_TEN;

    TEST_START("Test Case: Two Split Pipeline - Pass a Buffer down the Pipeline UnSuccessfully.")
    numEndpointsHit = 0;
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementTwoAddSink(&ctx, ElementThreeInit(&ctx));
    ElementTwoAddSink(&ctx, ElementFourInit(&ctx));
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(100, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(2, numEndpointsHit);
    TEST_END

    TEST_START("Test Case: Two Split Pipeline - Shutdown UnSuccessfully.")
    numEndpointsHit = 0;
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(2, numEndpointsHit);
    TEST_END
}  // itest__TwoSplitPipelineTests()

/*------------------------------------------------------------------------------
 | INTEGRATION TESTS: Testing a Pipeline with Three Splits.
 |
 | PIPELINE:                 +-------------+      +--------------+
 |                      +--> | Element Two | ---> | Element Five |
 |                      |    +-------------+      +--------------+
 |                      |
 |    +-------------+   |   +---------------+      +-------------+
 |    | Element One | --+-> | Element Three | ---> | Element Six |
 |    +-------------+   |   +---------------+      +-------------+
 |                      |
 |                      |    +--------------+
 |                      +--> | Element Four |
 |                           +--------------+
 | TEST CASES:
 |    1) Establish the Pipeline.
 |    2) Pass a Buffer down the Pipeline Successfully.
 |    3) Shutdown Successfully.
 |    4) Split a Buffer Once in the Pipeline.
 |    5) Split a Buffer Multiple Times in the Pipeline.
 |    6) Spawn a Buffer during Shutdown.
 |    7) Pass a Buffer down the Pipeline UnSuccessfully.
 |    8) Shutdown UnSuccessfully.
 -------------------------------------------------------------------------------*/
void itest__ThreeSplitPipelineTests( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    Context ctx;
    Buffer* buffPtr;
    boolean retval;

    memset(&ctx, 0, sizeof(Context));

    whichTest = TEST_ELEVEN;

    TEST_START("Test Case: Three Split Pipeline - Establish the Pipeline.")
    BufferPoolInit();
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementTwoAddSink(&ctx, ElementFiveInit(&ctx));
    ElementOneAddSink(&ctx, ElementThreeInit(&ctx));
    ElementThreeAddSink(&ctx, ElementSixInit(&ctx));
    ElementOneAddSink(&ctx, ElementFourInit(&ctx));
    TEST_END

    TEST_START("Test Case: Three Split Pipeline - Pass a Buffer down the Pipeline Successfully.")
    numEndpointsHit = 0;
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(100, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(3, numEndpointsHit);
    TEST_END

    TEST_START("Test Case: Three Split Pipeline - Shutdown Successfully.")
    numEndpointsHit = 0;
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(3, numEndpointsHit);
    TEST_END

    whichTest = TEST_TWELVE;

    TEST_START("Test Case: Three Split Pipeline - Split a Buffer Once in the Pipeline.")
    numEndpointsHit = 0;
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementTwoAddSink(&ctx, ElementFiveInit(&ctx));
    ElementOneAddSink(&ctx, ElementThreeInit(&ctx));
    ElementThreeAddSink(&ctx, ElementSixInit(&ctx));
    ElementOneAddSink(&ctx, ElementFourInit(&ctx));
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 200);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(200, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(4, numEndpointsHit);
    TEST_END

    whichTest = TEST_THIRTEEN;

    TEST_START("Test Case: Three Split Pipeline - Split a Buffer Multiple Times in the Pipeline.")
    memset(&ctx, 0, sizeof(Context));
    numEndpointsHit = 0;
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementTwoAddSink(&ctx, ElementFiveInit(&ctx));
    ElementOneAddSink(&ctx, ElementThreeInit(&ctx));
    ElementThreeAddSink(&ctx, ElementSixInit(&ctx));
    ElementOneAddSink(&ctx, ElementFourInit(&ctx));
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 200);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(200, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(8, numEndpointsHit);
    TEST_END

    TEST_START("Test Case: Three Split Pipeline - Spawn a Buffer during Shutdown.")
    numEndpointsHit = 0;
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(4, numEndpointsHit);
    TEST_END

    whichTest = TEST_FOURTEEN;

    TEST_START("Test Case: Three Split Pipeline - Pass a Buffer down the Pipeline UnSuccessfully.")
    numEndpointsHit = 0;
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementTwoAddSink(&ctx, ElementFiveInit(&ctx));
    ElementOneAddSink(&ctx, ElementThreeInit(&ctx));
    ElementThreeAddSink(&ctx, ElementSixInit(&ctx));
    ElementOneAddSink(&ctx, ElementFourInit(&ctx));
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(100, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(3, numEndpointsHit);
    TEST_END

    TEST_START("Test Case: Three Split Pipeline - Shutdown UnSuccessfully.")
    numEndpointsHit = 0;
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(3, numEndpointsHit);
    TEST_END
}  // itest__ThreeSplitPipelineTests()

/*------------------------------------------------------------------------------
 | INTEGRATION TESTS: Testing a Pipeline with Five Splits.
 |
 | PIPELINE:                    +-------------+
 |                      +-----> | Element Two |
 |                      |       +-------------+
 |                      |
 |                      |     +---------------+
 |                      +---> | Element Three |
 |                      |     +---------------+
 |                      |
 |    +-------------+   |      +--------------+
 |    | Element One | --+----> | Element Four |
 |    +-------------+   |      +--------------+
 |                      |
 |                      |      +--------------+
 |                      +----> | Element Five |
 |                      |      +--------------+
 |                      |
 |                      |       +-------------+
 |                      +-----> | Element Six |
 |                              +-------------+
 | TEST CASES:
 |    1) Establish the Pipeline.
 |    2) Fail to add another Split to the Pipeline.
 |    3) Pass a Buffer down the Pipeline Successfully.
 |    4) Shutdown Successfully.
 |    5) Split a Buffer Once in the Pipeline.
 |    6) Spawn a Buffer during Shutdown.
 |    7) Pass a Buffer down the Pipeline UnSuccessfully.
 |    8) Shutdown UnSuccessfully.
 -------------------------------------------------------------------------------*/
void itest__FiveSplitPipelineTests( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    Context ctx;
    Buffer* buffPtr;
    boolean retval;
    LinkInfo dummyLI;

    memset(&ctx, 0, sizeof(Context));

    whichTest = TEST_FIFTEEN;

    TEST_START("Test Case: Five Split Pipeline - Establish the Pipeline.")
    BufferPoolInit();
    dummyLI = ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementOneAddSink(&ctx, ElementThreeInit(&ctx));
    ElementOneAddSink(&ctx, ElementFourInit(&ctx));
    ElementOneAddSink(&ctx, ElementFiveInit(&ctx));
    ElementOneAddSink(&ctx, ElementSixInit(&ctx));
    TEST_END

    TEST_START("Test Case: Five Split Pipeline - Fail to add another Split to the Pipeline.")
    ERROR_EXPECTED
    ElementOneAddSink(&ctx, dummyLI);
    TEST_END

    TEST_START("Test Case: Five Split Pipeline - Pass a Buffer down the Pipeline Successfully.")
    numEndpointsHit = 0;
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(100, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(5, numEndpointsHit);
    TEST_END

    TEST_START("Test Case: Five Split Pipeline - Shutdown Successfully.")
    numEndpointsHit = 0;
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(5, numEndpointsHit);
    TEST_END

    whichTest = TEST_SIXTEEN;

    TEST_START("Test Case: Five Split Pipeline - Split a Buffer Once in the Pipeline.")
    numEndpointsHit = 0;
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementOneAddSink(&ctx, ElementThreeInit(&ctx));
    ElementOneAddSink(&ctx, ElementFourInit(&ctx));
    ElementOneAddSink(&ctx, ElementFiveInit(&ctx));
    ElementOneAddSink(&ctx, ElementSixInit(&ctx));
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 200);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(200, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(10, numEndpointsHit);
    TEST_END

    TEST_START("Test Case: Five Split Pipeline - Spawn a Buffer during Shutdown.")
    numEndpointsHit = 0;
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(10, numEndpointsHit);
    TEST_END

    whichTest = TEST_SEVENTEEN;

    TEST_START("Test Case: Five Split Pipeline - Pass a Buffer down the Pipeline UnSuccessfully.")
    numEndpointsHit = 0;
    ElementOneInit( &ctx );
    ElementOneAddSink(&ctx, ElementTwoInit(&ctx));
    ElementOneAddSink(&ctx, ElementThreeInit(&ctx));
    ElementOneAddSink(&ctx, ElementFourInit(&ctx));
    ElementOneAddSink(&ctx, ElementFiveInit(&ctx));
    ElementOneAddSink(&ctx, ElementSixInit(&ctx));
    buffPtr = NewBuffer(BUFFER_TYPE_BYTES, 100);
    ASSERT_EQ(1, NumAllocatedBuffers());
    ASSERT_EQ(BUFFER_TYPE_BYTES, buffPtr->bufferType);
    ASSERT_EQ(100, buffPtr->maxNumElements);
    ASSERT_EQ(0, buffPtr->numElements);
    retval = ElementOneProcNextBuffer(&ctx, buffPtr);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(5, numEndpointsHit);
    TEST_END

    TEST_START("Test Case: Five Split Pipeline - Shutdown UnSuccessfully.")
    numEndpointsHit = 0;
    retval = ElementOneShutdown(&ctx);
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(0, NumAllocatedBuffers());
    ASSERT_EQ(5, numEndpointsHit);
    TEST_END
}  // itest__FiveSplitPipelineTests()

/*------------------------------------------------------------------------------
 | INTEGRATION TESTS: Testing a Mock MCC Pipeline
 |
 | PIPELINE:                                     +----------------+      +----------------+
 |                                           +-> | Line 21 Decode | ---> | Line 21 Output |
 |                                           |   +----------------+      +----------------+
 |                                           |
 |    +--------------+      +------------+   |    +--------------+        +--------------+
 |    | Caption File | ---> | MCC Decode | --+--> | DTVCC Decode | -----> | DTVCC Output |
 |    +--------------+      +------------+   |    +--------------+        +--------------+
 |                                           |
 |                                           |    +----------------+
 |                                           +--> | CC Data Output |
 |                                                +----------------+
 | TEST CASES:
 |    1) Establish the Pipeline.
 |    2) Pass two Buffers Successfully.
 |    3) Successfully Shut down the Pipeline.
 |    4) Pass two Buffers Successfully and one Buffer UnSuccessfully.
 |    5) UnSuccessfully Shut down the Pipeline.
 -------------------------------------------------------------------------------*/
void itest__MockMccPipelineTests( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    Context ctx;
    boolean retval;
    boolean isDone;

    memset(&ctx, 0, sizeof(Context));

    TEST_START("Test Case: Mock MCC Pipeline - Establish the Pipeline.")
    BufferPoolInit();
    retval = PlumbMccPipeline(&ctx, "Test", "MCC", TRUE );
    ASSERT_EQ(TRUE, retval);
    ASSERT_STREQ("Test", ctx.mccFileCtxPtr->captionFileName);
    ASSERT_STREQ("MCC", ctx.line21OutputCtxPtr->baseFileName);
    ASSERT_STREQ("MCC", ctx.dtvccOutputCtxPtr->baseFileName);
    ASSERT_STREQ("MCC", ctx.ccDataOutputCtxPtr->ccdFileName);
    TEST_END

    TEST_START("Test Case: Mock MCC Pipeline - Pass two Buffers Successfully.")
    isDone = FALSE;
    dtvccDecodePassSuccessfully = TRUE;
    resetMetrics();
    retval = MccFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, mccDecodeBufferReceived);
    ASSERT_EQ(1, line21DecodeBufferReceived);
    ASSERT_EQ(1, dtvccDecodeBufferReceived);
    ASSERT_EQ(1, ccDataOutBufferReceived);
    ASSERT_EQ(1, line21OutBufferReceived);
    ASSERT_EQ(1, dtvccOutBufferReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    retval = MccFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(2, mccDecodeBufferReceived);
    ASSERT_EQ(2, line21DecodeBufferReceived);
    ASSERT_EQ(2, dtvccDecodeBufferReceived);
    ASSERT_EQ(2, ccDataOutBufferReceived);
    ASSERT_EQ(2, line21OutBufferReceived);
    ASSERT_EQ(2, dtvccOutBufferReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Mock MCC Pipeline - Successfully Shut down the Pipeline.")
    isDone = TRUE;
    line21OutputShutdownSuccessfully = TRUE;
    retval = MccFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(TRUE, mccDecodeShutdownReceived);
    ASSERT_EQ(TRUE, line21DecodeShutdownReceived);
    ASSERT_EQ(TRUE, dtvccDecodeShutdownReceived);
    ASSERT_EQ(TRUE, ccDataOutShutdownReceived);
    ASSERT_EQ(TRUE, line21OutShutdownReceived);
    ASSERT_EQ(TRUE, dtvccOutShutdownReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Mock MCC Pipeline - Pass two Buffers Successfully and one UnSuccessfully.")
    isDone = FALSE;
    dtvccDecodePassSuccessfully = TRUE;
    resetMetrics();
    retval = PlumbMccPipeline( &ctx, "Test", "MCC", TRUE );
    ASSERT_EQ(TRUE, retval);
    retval = MccFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, mccDecodeBufferReceived);
    ASSERT_EQ(1, line21DecodeBufferReceived);
    ASSERT_EQ(1, dtvccDecodeBufferReceived);
    ASSERT_EQ(1, ccDataOutBufferReceived);
    ASSERT_EQ(1, line21OutBufferReceived);
    ASSERT_EQ(1, dtvccOutBufferReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    retval = MccFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(2, mccDecodeBufferReceived);
    ASSERT_EQ(2, line21DecodeBufferReceived);
    ASSERT_EQ(2, dtvccDecodeBufferReceived);
    ASSERT_EQ(2, ccDataOutBufferReceived);
    ASSERT_EQ(2, line21OutBufferReceived);
    ASSERT_EQ(2, dtvccOutBufferReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    dtvccDecodePassSuccessfully = FALSE;
    retval = MccFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(3, mccDecodeBufferReceived);
    ASSERT_EQ(3, line21DecodeBufferReceived);
    ASSERT_EQ(3, dtvccDecodeBufferReceived);
    ASSERT_EQ(3, ccDataOutBufferReceived);
    ASSERT_EQ(3, line21OutBufferReceived);
    ASSERT_EQ(3, dtvccOutBufferReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Mock MCC Pipeline - UnSuccessfully Shut down the Pipeline.")
    isDone = TRUE;
    line21OutputShutdownSuccessfully = FALSE;
    retval = MccFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(TRUE, mccDecodeShutdownReceived);
    ASSERT_EQ(TRUE, line21DecodeShutdownReceived);
    ASSERT_EQ(TRUE, dtvccDecodeShutdownReceived);
    ASSERT_EQ(TRUE, ccDataOutShutdownReceived);
    ASSERT_EQ(TRUE, line21OutShutdownReceived);
    ASSERT_EQ(TRUE, dtvccOutShutdownReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END
}  // itest__MockMccPipelineTests()

/*------------------------------------------------------------------------------
 | INTEGRATION TESTS: Testing a Mock MPEG Pipeline
 |
 | PIPELINE:                 +------------+          +------------+
 |                     +---> | MCC Encode | -------> | MCC Output |
 |                     |     +------------+          +------------+
 |                     |
 |                     |     +----------------+      +----------------+
 |                     |---> | Line 21 Decode | -?-> | Line 21 Output |
 |     +-----------+   |     +----------------+      +----------------+
 |     | MPEG File | --|
 |     +-----------+   |     +--------------+        +--------------+
 |                     |---> | DTVCC Decode | --?--> | DTVCC Output |
 |                     |     +--------------+        +--------------+
 |                     |
 |                     |     +----------------+
 |                     +-?-> | CC Data Output |
 |                           +----------------+
 | TEST CASES:
 |     1) Establish the Pipeline with Artifacts.
 |     2) Pass two Buffers Successfully to Pipeline with Artifacts.
 |     3) Successfully Shut down the Pipeline with Artifacts.
 |     4) Pass two Buffers Successfully and one Buffer UnSuccessfully (w/Artifacts).
 |     5) UnSuccessfully Shut down the Pipeline with Artifacts.
 |     6) Establish the Pipeline without Artifacts.
 |     7) Pass two Buffers Successfully to Pipeline without Artifacts.
 |     8) Successfully Shut down the Pipeline without Artifacts.
 |     9) Pass two Buffers Successfully and one Buffer UnSuccessfully (wo/Artifacts).
 |    10) UnSuccessfully Shut down the Pipeline without Artifacts.
 -------------------------------------------------------------------------------*/
void itest__MockMpegPipelineTests( TEST_SUITE_RECEIVED_ARGUMENTS ) {
    TEST_INITIALIZE
    Context ctx;
    boolean retval;
    boolean isDone;

    memset(&ctx, 0, sizeof(Context));

    TEST_START("Test Case: Mock MPEG Pipeline - Establish the Pipeline with Artifacts.")
    BufferPoolInit();
    retval = PlumbMpegPipeline( &ctx, "Test", "MPEG", TRUE, "Pipeline" );
    ASSERT_EQ(TRUE, retval);
    ASSERT_STREQ("Test", (char*)ctx.mpegFileCtxPtr->buffer);
    ASSERT_STREQ("MPEG", ctx.mccOutputCtxPtr->mccFileName);
    ASSERT_STREQ("Pipeline", ctx.line21OutputCtxPtr->baseFileName);
    ASSERT_STREQ("Pipeline", ctx.dtvccOutputCtxPtr->baseFileName);
    ASSERT_STREQ("Pipeline", ctx.ccDataOutputCtxPtr->ccdFileName);
    TEST_END

    TEST_START("Test Case: Mock MPEG Pipeline - Pass two Buffers Successfully to Pipeline with Artifacts.")
    isDone = FALSE;
    dtvccDecodePassSuccessfully = TRUE;
    resetMetrics();
    retval = MpegFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, mccEncodeBufferReceived);
    ASSERT_EQ(1, mccOutBufferReceived);
    ASSERT_EQ(1, line21DecodeBufferReceived);
    ASSERT_EQ(1, dtvccDecodeBufferReceived);
    ASSERT_EQ(1, ccDataOutBufferReceived);
    ASSERT_EQ(1, line21OutBufferReceived);
    ASSERT_EQ(1, dtvccOutBufferReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    retval = MpegFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(2, mccEncodeBufferReceived);
    ASSERT_EQ(2, mccOutBufferReceived);
    ASSERT_EQ(2, line21DecodeBufferReceived);
    ASSERT_EQ(2, dtvccDecodeBufferReceived);
    ASSERT_EQ(2, ccDataOutBufferReceived);
    ASSERT_EQ(2, line21OutBufferReceived);
    ASSERT_EQ(2, dtvccOutBufferReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Mock MPEG Pipeline - Successfully Shut down the Pipeline with Artifacts.")
    isDone = TRUE;
    line21OutputShutdownSuccessfully = TRUE;
    mccOutputShutdownSuccessfully = TRUE;
    retval = MpegFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(TRUE, mccEncodeShutdownReceived);
    ASSERT_EQ(TRUE, mccOutShutdownReceived);
    ASSERT_EQ(TRUE, line21DecodeShutdownReceived);
    ASSERT_EQ(TRUE, dtvccDecodeShutdownReceived);
    ASSERT_EQ(TRUE, ccDataOutShutdownReceived);
    ASSERT_EQ(TRUE, line21OutShutdownReceived);
    ASSERT_EQ(TRUE, dtvccOutShutdownReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Mock MPEG Pipeline - Pass two Buffers Successfully and one Buffer UnSuccessfully (w/Artifacts).")
    isDone = FALSE;
    dtvccDecodePassSuccessfully = TRUE;
    resetMetrics();
    retval = PlumbMpegPipeline(&ctx, "Test", "MPEG", TRUE, "Pipeline" );
    ASSERT_EQ(TRUE, retval);
    retval = MpegFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, mccEncodeBufferReceived);
    ASSERT_EQ(1, mccOutBufferReceived);
    ASSERT_EQ(1, line21DecodeBufferReceived);
    ASSERT_EQ(1, dtvccDecodeBufferReceived);
    ASSERT_EQ(1, ccDataOutBufferReceived);
    ASSERT_EQ(1, line21OutBufferReceived);
    ASSERT_EQ(1, dtvccOutBufferReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    retval = MpegFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(2, mccEncodeBufferReceived);
    ASSERT_EQ(2, mccOutBufferReceived);
    ASSERT_EQ(2, line21DecodeBufferReceived);
    ASSERT_EQ(2, dtvccDecodeBufferReceived);
    ASSERT_EQ(2, ccDataOutBufferReceived);
    ASSERT_EQ(2, line21OutBufferReceived);
    ASSERT_EQ(2, dtvccOutBufferReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    dtvccDecodePassSuccessfully = FALSE;
    retval = MpegFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(3, mccEncodeBufferReceived);
    ASSERT_EQ(3, mccOutBufferReceived);
    ASSERT_EQ(3, line21DecodeBufferReceived);
    ASSERT_EQ(3, dtvccDecodeBufferReceived);
    ASSERT_EQ(3, ccDataOutBufferReceived);
    ASSERT_EQ(3, line21OutBufferReceived);
    ASSERT_EQ(3, dtvccOutBufferReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Mock MPEG Pipeline - UnSuccessfully Shut down the Pipeline with Artifacts.")
    isDone = TRUE;
    line21OutputShutdownSuccessfully = FALSE;
    mccOutputShutdownSuccessfully = TRUE;
    retval = MpegFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(FALSE, retval);
    ASSERT_EQ(TRUE, mccEncodeShutdownReceived);
    ASSERT_EQ(TRUE, mccOutShutdownReceived);
    ASSERT_EQ(TRUE, line21DecodeShutdownReceived);
    ASSERT_EQ(TRUE, dtvccDecodeShutdownReceived);
    ASSERT_EQ(TRUE, ccDataOutShutdownReceived);
    ASSERT_EQ(TRUE, line21OutShutdownReceived);
    ASSERT_EQ(TRUE, dtvccOutShutdownReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Mock MPEG Pipeline - Establish the Pipeline without Artifacts.")
    BufferPoolInit();
    retval = PlumbMpegPipeline( &ctx, "Test", "MPEG", FALSE, "Pipeline" );
    ASSERT_EQ(TRUE, retval);
    ASSERT_STREQ("Test", (char*)ctx.mpegFileCtxPtr->buffer);
    ASSERT_STREQ("MPEG", ctx.mccOutputCtxPtr->mccFileName);
    TEST_END

    TEST_START("Test Case: Mock MPEG Pipeline - Pass two Buffers Successfully to Pipeline without Artifacts.")
    isDone = FALSE;
    dtvccDecodePassSuccessfully = TRUE;
    resetMetrics();
    retval = MpegFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(1, mccEncodeBufferReceived);
    ASSERT_EQ(1, mccOutBufferReceived);
    ASSERT_EQ(1, line21DecodeBufferReceived);
    ASSERT_EQ(1, dtvccDecodeBufferReceived);
    ASSERT_EQ(0, ccDataOutBufferReceived);
    ASSERT_EQ(0, line21OutBufferReceived);
    ASSERT_EQ(0, dtvccOutBufferReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    retval = MpegFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(2, mccEncodeBufferReceived);
    ASSERT_EQ(2, mccOutBufferReceived);
    ASSERT_EQ(2, line21DecodeBufferReceived);
    ASSERT_EQ(2, dtvccDecodeBufferReceived);
    ASSERT_EQ(0, ccDataOutBufferReceived);
    ASSERT_EQ(0, line21OutBufferReceived);
    ASSERT_EQ(0, dtvccOutBufferReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Mock MPEG Pipeline - Successfully Shut down the Pipeline without Artifacts.")
    isDone = TRUE;
    line21OutputShutdownSuccessfully = TRUE;
    retval = MpegFileProcNextBuffer( &ctx, &isDone );
    ASSERT_EQ(TRUE, retval);
    ASSERT_EQ(TRUE, mccEncodeShutdownReceived);
    ASSERT_EQ(TRUE, mccOutShutdownReceived);
    ASSERT_EQ(TRUE, line21DecodeShutdownReceived);
    ASSERT_EQ(TRUE, dtvccDecodeShutdownReceived);
    ASSERT_EQ(FALSE, ccDataOutShutdownReceived);
    ASSERT_EQ(FALSE, line21OutShutdownReceived);
    ASSERT_EQ(FALSE, dtvccOutShutdownReceived);
    ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Mock MPEG Pipeline - Pass two Buffers Successfully and one Buffer UnSuccessfully (wo/Artifacts).")
        isDone = FALSE;
        dtvccDecodePassSuccessfully = TRUE;
        resetMetrics();
        retval = PlumbMpegPipeline( &ctx, "Test", "MPEG", FALSE, "Pipeline" );
        ASSERT_EQ(TRUE, retval);
        retval = MpegFileProcNextBuffer( &ctx, &isDone );
        ASSERT_EQ(TRUE, retval);
        ASSERT_EQ(1, mccEncodeBufferReceived);
        ASSERT_EQ(1, mccOutBufferReceived);
        ASSERT_EQ(1, line21DecodeBufferReceived);
        ASSERT_EQ(1, dtvccDecodeBufferReceived);
        ASSERT_EQ(0, ccDataOutBufferReceived);
        ASSERT_EQ(0, line21OutBufferReceived);
        ASSERT_EQ(0, dtvccOutBufferReceived);
        ASSERT_EQ(0, NumAllocatedBuffers());
        retval = MpegFileProcNextBuffer( &ctx, &isDone );
        ASSERT_EQ(TRUE, retval);
        ASSERT_EQ(2, mccEncodeBufferReceived);
        ASSERT_EQ(2, mccOutBufferReceived);
        ASSERT_EQ(2, line21DecodeBufferReceived);
        ASSERT_EQ(2, dtvccDecodeBufferReceived);
        ASSERT_EQ(0, ccDataOutBufferReceived);
        ASSERT_EQ(0, line21OutBufferReceived);
        ASSERT_EQ(0, dtvccOutBufferReceived);
        ASSERT_EQ(0, NumAllocatedBuffers());
        dtvccDecodePassSuccessfully = FALSE;
        retval = MpegFileProcNextBuffer( &ctx, &isDone );
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(3, mccEncodeBufferReceived);
        ASSERT_EQ(3, mccOutBufferReceived);
        ASSERT_EQ(3, line21DecodeBufferReceived);
        ASSERT_EQ(3, dtvccDecodeBufferReceived);
        ASSERT_EQ(0, ccDataOutBufferReceived);
        ASSERT_EQ(0, line21OutBufferReceived);
        ASSERT_EQ(0, dtvccOutBufferReceived);
        ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END

    TEST_START("Test Case: Mock MPEG Pipeline - UnSuccessfully Shut down the Pipeline without Artifacts.")
        isDone = TRUE;
        mccOutputShutdownSuccessfully = FALSE;
        retval = MpegFileProcNextBuffer( &ctx, &isDone );
        ASSERT_EQ(FALSE, retval);
        ASSERT_EQ(TRUE, mccEncodeShutdownReceived);
        ASSERT_EQ(TRUE, mccOutShutdownReceived);
        ASSERT_EQ(TRUE, line21DecodeShutdownReceived);
        ASSERT_EQ(TRUE, dtvccDecodeShutdownReceived);
        ASSERT_EQ(FALSE, ccDataOutShutdownReceived);
        ASSERT_EQ(FALSE, line21OutShutdownReceived);
        ASSERT_EQ(FALSE, dtvccOutShutdownReceived);
        ASSERT_EQ(0, NumAllocatedBuffers());
    TEST_END
}  // itest__MockMpegPipelineTests()

/*----------------------------------------------------------------------------*/
/*--                             Test Suite                                 --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 |                         Pipeline Integration Tests
 |                         ==========================
 | TESTED FILES:
 |    pipeline_utils.c
 |    buffer_utils.c
 -------------------------------------------------------------------------------*/
int main( int argc, char* argv[] ) {
    INIT_TEST_FRAMEWORK( argc, argv )

    TEST_SUITE_START("Test Suite: Testing a Pipeline with Two Elements.")
    itest__TwoElementPipelineTests( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: Testing a Pipeline with Five Elements.")
    itest__FiveElementPipelineTests( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: Testing a Pipeline with Two Splits.")
    itest__TwoSplitPipelineTests( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: Testing a Pipeline with Three Splits.")
    itest__ThreeSplitPipelineTests( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: Testing a Pipeline with Five Splits.")
    itest__FiveSplitPipelineTests( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: Testing a Mock MCC Pipeline.")
    itest__MockMccPipelineTests( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    TEST_SUITE_START("Test Suite: Testing a Mock MPEG Pipeline.")
    itest__MockMpegPipelineTests( TEST_SUITE_PASSED_ARGUMENTS );
    TEST_SUITE_END

    SHUTDOWN_TEST_FRAMEWORK
}  // main()
