//
//  test_engine.c
//  Caption Converter Testing Driver File
//
//  Created by Rob Taylor on 4/9/19.
//

#include "test_engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>
#include <getopt.h>

#include "debug.h"

/*----------------------------------------------------------------------------*/
/*--                           Global Variables                             --*/
/*----------------------------------------------------------------------------*/

jmp_buf jmpBuf;
int jmpBufNum;

int8 errorExpected = 0;
boolean errorReceived = FALSE;
boolean fatalErrorExpected = FALSE;
uint16 totalNumSuccessfulTests = 0;
uint16 totalNumFailedTests = 0;

static boolean finalizeXmlFile;
static boolean launchBrowser;
static FILE* outputFilePtr;
static char outputFileName[OUTPUT_XML_FILE_NAME_LEN];
static char textBuffer[500000];

static const char* TestDebugLevelText[MAX_TEST_DEBUG_LEVEL] = {
        "UNKNOWN_TEST_DEBUG_LEVEL",
        "TEST_INFO",
        "TEST_ERROR"
};

static const char *optString = "fbh?";

static struct option longOpts[] = {
        { "finalize",     no_argument, NULL, 'f' },
        { "browser",      no_argument, NULL, 'b' },
        { "help",         no_argument, NULL, 'h' },
        { 0, no_argument, NULL, 0 }
};

/*----------------------------------------------------------------------------*/
/*--                           Public Functions                             --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
| NAME:
|    HandleSignal()
|
| INPUT PARAMETERS:
|    sigNum - The number of the signal that was raised. (Ignored)
|
| RETURN VALUES:
|    None.
|
| DESCRIPTION:
|    This method handles a signal error is expected and should be suppressed.
-------------------------------------------------------------------------------*/
void HandleSignal( int sigNum ) {
    if( fatalErrorExpected == TRUE ) {
        fatalErrorExpected = FALSE;
        longjmp(jmpBuf, EXPECTED_RAISED_SIGNAL_LONG_JUMP);
    } else {
        longjmp(jmpBuf, UNEXPECTED_RAISED_SIGNAL_LONG_JUMP);
    }
}  // HandleSignal()

/*------------------------------------------------------------------------------
 | NAME:
 |    ExpectError()
 |
 | INPUT PARAMETERS:
 |    errorNumber - The number of errors to expect.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method suggests that an error is expected and should be suppressed.
 -------------------------------------------------------------------------------*/
void ExpectError( uint8 errorNumber ) {
    errorExpected = errorExpected + errorNumber;
}  // ExpectError()

/*------------------------------------------------------------------------------
 | NAME:
 |    ExpectFatalError()
 |
 | INPUT PARAMETERS:
 |    None.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method suggests that a fatal error is expected and should be suppressed.
 -------------------------------------------------------------------------------*/
void ExpectFatalError( void ) {
    fatalErrorExpected = TRUE;
}  // ExpectFatalError()

/*------------------------------------------------------------------------------
 | NAME:
 |    CheckError()
 |
 | INPUT PARAMETERS:
 |    jmpBufNum - The Number from the Long Jump Call.
 |
 | RETURN VALUES:
 |    boolean - TRUE: All Errors lined up; FALSE: Expected an error but it
 |              didn't happen.
 |
 | DESCRIPTION:
 |    This method ensures that there were no expected errors that didn't happen.
 -------------------------------------------------------------------------------*/
boolean CheckError( int jmpBufNum ) {

    if( (errorExpected != 0) || (errorReceived == TRUE) ) {
        WriteToOutputBuffer("TEST FAILED: Mismatch in Error Count: Outstanding Error Count: %d - Error Received: %s", errorExpected, errorReceived?"True":"False");
        errorReceived = FALSE;
        errorExpected = 0;
        return FALSE;
    }

    if( jmpBufNum == UNEXPECTED_FATAL_ERROR_LONG_JUMP ) {
        WriteToOutputBuffer("TEST FAILED: Unexpected Fatal Error Received!");
        return FALSE;
    }

    if( jmpBufNum == UNEXPECTED_RAISED_SIGNAL_LONG_JUMP ) {
        WriteToOutputBuffer("TEST FAILED: Fatal Signal Received!");
        return FALSE;
    }

    if( fatalErrorExpected == TRUE ) {
        WriteToOutputBuffer("TEST FAILED: Fatal Error was expected, but never encountered!");
        fatalErrorExpected = FALSE;
        return FALSE;
    }

    return TRUE;
}  // CheckError()

/*------------------------------------------------------------------------------
 | NAME:
 |    LogTestCase()
 |
 | INPUT PARAMETERS:
 |    wasSuccessful - Was Test Case Successful? True - Successful; False - Failure
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method writes the result of the test case out to file, following the
 |    correct boiler plate and adding the relevant data.
 -------------------------------------------------------------------------------*/
void LogTestCase( boolean wasSuccessful ) {
    if( wasSuccessful == TRUE ) {
        if( textBuffer[0] == '\0' ) {
            WriteToOutputFile("/>\n");
        } else {
            WriteToOutputFile(">\n            <passed>");
            FlushBufferToFile();
            WriteToOutputFile("</passed>\n        </testcase>\n");
        }
    } else {
        WriteToOutputFile(">\n            <failure>");
        FlushBufferToFile();
        WriteToOutputFile("</failure>\n\n        </testcase>\n");
    }
}  // LogTestCase()

/*------------------------------------------------------------------------------
 | NAME:
 |    DebugLogHandler()
 |
 | INPUT PARAMETERS:
 |    file - The file name of the function that made the logging statement. This
 |           file name is added by the LOG() Macro.
 |    line - The line number of the function that made the logging statement. This
 |           line number is added by the LOG() Macro.
 |    level - The severity of the logging message as defined in debug.h
 |    section - The logical section of the code which made the logging statement.
 |    message - The string to log array associated with the error, etc.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method formats the log and saves it into a buffer for writing to the
 |    output file. This method also takes into account whether an error is expected
 |    and whether or not an error is fatal (in which case it exits the test).
 -------------------------------------------------------------------------------*/
void DebugLogHandler(char* file, int line, uint8 level, uint8 section, char* message) {

    char* basename = strrchr(file, '/');
    basename = basename ? basename+1 : file;

    if( message[(strlen(message)-1)] == '\n' ) message[(strlen(message)-1)] = '\0';

    if( (level > DEBUG_LEVEL_WARN) && (level < DEBUG_LEVEL_FATAL) && (errorExpected > 0) ) {
        errorExpected--;
        WriteToOutputBuffer("EXPECTED --> {%s} %s [%s:%d] - %s\n", DebugLevelText[level], DebugSectionText[section], basename, line, message);
    } else if( ((level == DEBUG_LEVEL_FATAL) || (level == DEBUG_LEVEL_ASSERT)) && (fatalErrorExpected == TRUE) ) {
        WriteToOutputBuffer("EXPECTED --> {%s} %s [%s:%d] - %s\n", DebugLevelText[level], DebugSectionText[section], basename, line, message);
    } else if( level >= MAX_DEBUG_LEVEL ) {
        if( level == TEST_DEBUG_LEVEL_ERROR ) {
            errorReceived = TRUE;
        }
        WriteToOutputBuffer("{%s} [%s:%d] - %s\n", TestDebugLevelText[(level - MAX_DEBUG_LEVEL)], basename, line, message);
    } else {
        if( level > DEBUG_LEVEL_WARN ) {
            errorReceived = TRUE;
        }
        WriteToOutputBuffer("{%s} %s [%s:%d] - %s\n", DebugLevelText[level], DebugSectionText[section], basename, line, message);
    }

    if( (level == DEBUG_LEVEL_FATAL) || (level == DEBUG_LEVEL_ASSERT) ) {
        if( fatalErrorExpected == TRUE ) {
            fatalErrorExpected = FALSE;
            longjmp(jmpBuf, EXPECTED_FATAL_ERROR_LONG_JUMP);
        } else {
            longjmp(jmpBuf, UNEXPECTED_FATAL_ERROR_LONG_JUMP);
        }
    }

}  // DebugLogHandler()

/*------------------------------------------------------------------------------
 | NAME:
 |    printHelp()
 |
 | DESCRIPTION:
 |    This help message with command line options for this program.
 -------------------------------------------------------------------------------*/
static void printHelp( char* nameAndPath ) {
    char* programName;
    programName = strrchr(nameAndPath, '/');
    programName = programName ? programName+1 : nameAndPath;

    printf("\n                 ,ad8888ba,                                    88\n");
    printf("                d8\"'    `\"8b                            ,d     \"\"\n");
    printf("               d8'                                      88\n");
    printf("               88             ,adPPYYba,  8b,dPPYba,  MM88MMM  88   ,adPPYba,   8b,dPPYba,\n");
    printf("               88             \"\"     `Y8  88P'    \"8a   88     88  a8\"     \"8a  88P'   `\"8a\n");
    printf("               Y8,            ,adPPPPP88  88       d8   88     88  8b       d8  88       88\n");
    printf("                Y8a.    .a8P  88,    ,88  88b,   ,a8\"   88,    88  \"8a,   ,a8\"  88       88\n");
    printf("                 `\"Y8888Y\"'   `\"8bbdP\"Y8  88`YbbdP\"'    \"Y888  88   `\"YbbdP\"'   88       88\n");
    printf("  ,ad8888ba,                              88\n");
    printf(" d8\"'    `\"8b                             88                                    ,d\n");
    printf("d8'                                                                             88\n");
    printf("88              ,adPPYba,   8b,dPPYba,   8b       d8   ,adPPYba,  8b,dPPYba,  MM88MMM  ,adPPYba,  8b,dPPYba,\n");
    printf("88             a8\"     \"8a  88P'   `\"8a  `8b     d8'  a8P_____88  88P'   \"Y8    88    a8P_____88  88P'   \"Y8\n");
    printf("Y8,            8b       d8  88       88   `8b   d8'   8PP\"\"\"\"\"\"\"  88            88    8PP\"\"\"\"\"\"\"  88\n");
    printf(" Y8a.    .a8P  \"8a,   ,a8\"  88       88    `8b,d8'    \"8b,   ,aa  88            88,   \"8b,   ,aa  88\n");
    printf("  `\"Y8888Y\"'    `\"YbbdP\"'   88       88      \"8\"       `\"Ybbd8\"'  88            \"Y888  `\"Ybbd8\"'  88\n\n");
    printf("               d888888b d88888b .d8888. d888888b      .d8888. db    db d888888b d888888b d88888b\n");
    printf("               `~~88~~' 88'     88'  YP `~~88~~'      88'  YP 88    88   `88'   `~~88~~' 88'\n");
    printf("                  88    88ooooo `8bo.      88         `8bo.   88    88    88       88    88ooooo\n");
    printf("                  88    88~~~~~   `Y8b.    88           `Y8b. 88    88    88       88    88~~~~~\n");
    printf("                  88    88.     db   8D    88         db   8D 88b  d88   .88.      88    88.\n");
    printf("                  YP    Y88888P `8888Y'    YP         `8888Y' ~Y8888P' Y888888P    YP    Y88888P \n\n");
    printf("\nUsage: %s [options] [XML Filename]\n", programName);
    printf("\nOptions:\n");
    printf("    -h|--help     : Display this help message.\n");
    printf("    -f|--finalize : Finalizes the XML File and convert it to HTML.\n");
    printf("    -b|--browser  : Finalizes the XML File, convert it to HTML, and launched into a browser.\n");
    printf("\nRunning without an Optional XML Filename will cause a new, unique file to be generated, finalized, converted to HTML, and launched in browser.\n\n");
}  // printHelp()

/*------------------------------------------------------------------------------
 | NAME:
 |    InitTestFramework()
 |
 | INPUT PARAMETERS:
 |    argc - Standard argument count to the executable.
 |    argv - Standard argument array for the executable.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This function initializes the test framework. Specifically, it initializes
 |    the signal handlers, reroutes the debug log, creates the output file, and
 |    writes the initial boiler plate in the file.
 -------------------------------------------------------------------------------*/
void InitTestFramework( int argc, char* argv[] ) {
    int opt;
    int longIndex = 0;
    char* outputFilename = NULL;

    while ((opt = getopt_long(argc, argv, optString, longOpts, &longIndex )) != -1) {
        switch (opt) {
            case 'h' :
                printHelp(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'f' :
                finalizeXmlFile = TRUE;
                break;
            case 'b' :
                launchBrowser = TRUE;
                break;
            default:
                printHelp((char*)argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if( (argc - optind) >= 1 ) {
        outputFilename = argv[optind];
    } else {
        finalizeXmlFile = TRUE;
        launchBrowser = TRUE;
    }

    CreateOutputFile(outputFilename);

    DebugInit( FALSE, NULL, DebugLogHandler );

    signal(SIGFPE , HandleSignal);
    signal(SIGSEGV, HandleSignal);

    if( finalizeXmlFile == TRUE ) {
        WriteToOutputFile("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        WriteToOutputFile("<testsuites name=\"Caption Converter Tests\">\n");
    }
}  // InitTestFramework()

/*------------------------------------------------------------------------------
 | NAME:
 |    ShutdownTestFramework()
 |
 | INPUT PARAMETERS:
 |    None.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This function writes the last of the boiler plate and closes the file.
 -------------------------------------------------------------------------------*/
void ShutdownTestFramework( void ) {
    char newFileName[OUTPUT_XML_FILE_NAME_LEN];
    char commandString[1024];
    char* tmpCharPtr;

    if( finalizeXmlFile == TRUE ) {
        WriteToOutputFile("</testsuites>\n\n");
    } else {
        WriteToOutputFile("\n");
    }

    CloseOutputFile();

    printf("Testing Complete: Results written to file - %s\n", outputFileName);

    if( finalizeXmlFile == TRUE ) {
        strcpy(newFileName, outputFileName);

        tmpCharPtr = strrchr(newFileName, '.');
        *tmpCharPtr = '\0';
        strcat(newFileName, ".html");
        sprintf(commandString, "xunit-viewer --results=%s --output=%s --title=\"Caption Converter C Test Suite\"\n",
                outputFileName, newFileName);
        system(commandString);

        printf("XML Results file: %s converted to HTML: %s\n", outputFileName, newFileName);
    }

    if( launchBrowser == TRUE ) {
        sprintf(commandString, "open %s\n", newFileName);
        system(commandString);
    }
}  // ShutdownTestFramework()

/*------------------------------------------------------------------------------
 | NAME:
 |    CreateOutputFile()
 |
 | INPUT PARAMETERS:
 |    fileNameStr - Filename of the existing file.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This function creates a new xml file, or appends an existing xml file,
 |    depending on whether an existing file's name is passed in.
 -------------------------------------------------------------------------------*/
void CreateOutputFile( const char* fileNameStr ) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    if( fileNameStr == NULL ) {
        sprintf(outputFileName, "%d_%d_%d__%d_%d_%d__test_output.xml", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    } else {
        strcpy(outputFileName, fileNameStr);
    }

    printf("Writing to New Existing Output File: %s\n", outputFileName);
    outputFilePtr = fopen(outputFileName, "w");

    textBuffer[0] = '\0';
}  // CreateOutputFile()

/*------------------------------------------------------------------------------
 | NAME:
 |    WriteToOutputFile()
 |
 | INPUT PARAMETERS:
 |    fmt, ... - C-Formatted Text with variable length arguments.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method will write a C-Formatted data to a file or to the screen.
 |
 -------------------------------------------------------------------------------*/
void WriteToOutputFile( char* fmt, ... ) {
    va_list args;
    va_start( args, fmt );
    vfprintf( outputFilePtr, fmt, args );
    va_end( args );
}  // WriteToOutputFile()

/*------------------------------------------------------------------------------
 | NAME:
 |    WriteToOutputBuffer()
 |
 | INPUT PARAMETERS:
 |    text - Text to append to the output buffer.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method will append text to the output buffer.
 |
 -------------------------------------------------------------------------------*/
void WriteToOutputBuffer( char* fmt, ... ) {
    va_list args;
    char message[1024];

    va_start(args, fmt);
    vsprintf(message, fmt, args);
    va_end(args);

    strcat(textBuffer, message);
}  // WriteToOutputBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    FlushBufferToFile()
 |
 | INPUT PARAMETERS:
 |    None.
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method will dump the text buffer to an output file.
 |
 -------------------------------------------------------------------------------*/
void FlushBufferToFile( void ) {
    if( textBuffer[(strlen(textBuffer)-1)] == '\n' ) textBuffer[(strlen(textBuffer)-1)] = '\0';
    fprintf( outputFilePtr, "%s", textBuffer );
    textBuffer[0] = '\0';
}  // FlushBufferToFile()

/*------------------------------------------------------------------------------
 | NAME:
 |    CloseOutputFile()
 |
 | INPUT PARAMETERS:
 |    None.
 |
 | RETURN VALUES:
 |    None.
 |
 | DESCRIPTION:
 |    This method will close a specific file.
 |
 -------------------------------------------------------------------------------*/
void CloseOutputFile( void ) {
    fclose( outputFilePtr );
}  // CloseOutputFile()

/*------------------------------------------------------------------------------
 | NAME:
 |    AssertStringEqual()
 |
 | INPUT PARAMETERS:
 |    file - Filename of the calling file.
 |    line - Line Number of the calling file.
 |    expectedStr - The Expected String.
 |    actualStr - The Actual String.
 |
 | RETURN VALUES:
 |    boolean - TRUE = Success; FALSE = Failure
 |
 | DESCRIPTION:
 |    This function verifies that the Expected String matches the Actual String.
 -------------------------------------------------------------------------------*/
boolean AssertStringEqual( char* file, int line, char* expectedStr, char* actualStr ) {
    if( strcmp(expectedStr, actualStr) != 0 ) {
        char* basename = strrchr(file, '/');
        basename = basename ? basename+1 : file;
        WriteToOutputBuffer("TEST FAILED: [%s:%d] %s does not match %s\n", basename, line, expectedStr, actualStr);
        return FALSE;
    }
    return TRUE;
}  // AssertStringEqual()

/*------------------------------------------------------------------------------
 | NAME:
 |    AssertPointerEqual()
 |
 | INPUT PARAMETERS:
 |    file - Filename of the calling file.
 |    line - Line Number of the calling file.
 |    expectedPtr - The Expected Pointer.
 |    actualPtr - The Actual Pointer.
 |
 | RETURN VALUES:
 |    boolean - TRUE = Success; FALSE = Failure
 |
 | DESCRIPTION:
 |    This function verifies that the Expected String matches the Actual String.
 -------------------------------------------------------------------------------*/
boolean AssertPointerEqual( char* file, int line, void* expectedPtr, void* actualPtr ) {
    if( expectedPtr != actualPtr ) {
        char* basename = strrchr(file, '/');
        basename = basename ? basename+1 : file;
        WriteToOutputBuffer("TEST FAILED: [%s:%d] %p does not match %p\n", basename, line, expectedPtr, actualPtr);
        return FALSE;
    }
    return TRUE;
}  // AssertPointerEqual()

/*------------------------------------------------------------------------------
 | NAME:
 |    AssertStructEqual()
 |
 | INPUT PARAMETERS:
 |    file - Filename of the calling file.
 |    line - Line Number of the calling file.
 |    expectedStructPtr - The Expected Structure Pointer.
 |    actualStructPtr - The Actual Structure Pointer.
 |    size - Size of the Structure to compare.
 |
 | RETURN VALUES:
 |    boolean - TRUE = Success; FALSE = Failure
 |
 | DESCRIPTION:
 |    This function verifies that the Expected Structure matches the Actual Structure.
 -------------------------------------------------------------------------------*/
boolean AssertStructEqual( char* file, int line, void* expectedStructPtr, void* actualStructPtr, size_t size ) {
    if( memcmp(expectedStructPtr, actualStructPtr, size) != 0 ) {
        char tmpExpBuffer[(size*3)+10];
        char tmpActBuffer[(size*3)+10];
        tmpExpBuffer[0] = tmpActBuffer[0] = '\0';
        for( int loop = 0; loop < size; loop++ ) {
            char tmpBuffer[5];
            sprintf(tmpBuffer, "%02X ", ((uint8*)expectedStructPtr)[loop]);
            strcat(tmpExpBuffer, tmpBuffer);
            sprintf(tmpBuffer, "%02X ", ((uint8*)actualStructPtr)[loop]);
            strcat(tmpActBuffer, tmpBuffer);
        }
        char* basename = strrchr(file, '/');
        basename = basename ? basename+1 : file;
        WriteToOutputBuffer("TEST FAILED: [%s:%d] {%s} does not match {%s}\n", basename, line, tmpExpBuffer, tmpActBuffer);

        return FALSE;
    }
    return TRUE;
}  // AssertStructEqual()

/*------------------------------------------------------------------------------
 | NAME:
 |    AssertEqual()
 |
 | INPUT PARAMETERS:
 |    file - Filename of the calling file.
 |    line - Line Number of the calling file.
 |    expected - The Expected Value.
 |    actual - The Actual Value.
 |
 | RETURN VALUES:
 |    boolean - TRUE = Success; FALSE = Failure
 |
 | DESCRIPTION:
 |    This function verifies that the Expected Value matches the Actual Value.
 -------------------------------------------------------------------------------*/
boolean AssertEqual( char* file, int line, uint64 expected, uint64 actual ) {
    if( expected != actual ) {
        char* basename = strrchr(file, '/');
        basename = basename ? basename+1 : file;
        WriteToOutputBuffer("TEST FAILED: [%s:%d] %d does not match %d\n", basename, line, expected, actual);
        return FALSE;
    }
    return TRUE;
}  // AssertEqual()

/*------------------------------------------------------------------------------
 | NAME:
 |    AssertNotEqual()
 |
 | INPUT PARAMETERS:
 |    file - Filename of the calling file.
 |    line - Line Number of the calling file.
 |    expected - The Expected Value not to be...
 |    actual - The Actual Value.
 |
 | RETURN VALUES:
 |    boolean - TRUE = Success; FALSE = Failure
 |
 | DESCRIPTION:
 |    This function verifies that the Expected Value matches the Actual Value.
 -------------------------------------------------------------------------------*/
boolean AssertNotEqual( char* file, int line, uint64 expected, uint64 actual ) {
    if( expected == actual ) {
        char* basename = strrchr(file, '/');
        basename = basename ? basename+1 : file;
        WriteToOutputBuffer("TEST FAILED: [%s:%d] %d  matches %d\n", basename, line, expected, actual);
        return FALSE;
    }
    return TRUE;
}  // AssertNotEqual()

/*------------------------------------------------------------------------------
 | NAME:
 |    AssertStringEqualMsg()
 |
 | INPUT PARAMETERS:
 |    file - Filename of the calling file.
 |    line - Line Number of the calling file.
 |    expectedStr - The Expected String.
 |    actualStr - The Actual String.
 |    ... - The variable argument array for the text
 |
 | RETURN VALUES:
 |    boolean - TRUE = Success; FALSE = Failure
 |
 | DESCRIPTION:
 |    This function verifies that the Expected String matches the Actual String.
 -------------------------------------------------------------------------------*/
boolean AssertStringEqualMsg( char* file, int line, char* expectedStr, char* actualStr, ... ) {
    va_list args;
    char msgStr[1024];

    va_start(args, actualStr);
    char* fmt = va_arg(args, char*);
    vsprintf(msgStr, fmt, args);
    va_end(args);

    if( strcmp(expectedStr, actualStr) != 0 ) {
        char* basename = strrchr(file, '/');
        basename = basename ? basename+1 : file;
        WriteToOutputBuffer("TEST FAILED: [%s:%d] %s does not match %s - %s\n", basename, line, expectedStr, actualStr, msgStr);
        return FALSE;
    }
    return TRUE;
}  // AssertStringEqualMsg()

/*------------------------------------------------------------------------------
 | NAME:
 |    AssertPointerEqualMsg()
 |
 | INPUT PARAMETERS:
 |    file - Filename of the calling file.
 |    line - Line Number of the calling file.
 |    expectedStr - The Expected Pointer.
 |    actualStr - The Actual Pointer.
 |    ... - The variable argument array for the text
 |
 | RETURN VALUES:
 |    boolean - TRUE = Success; FALSE = Failure
 |
 | DESCRIPTION:
 |    This function verifies that the Expected Pointer matches the Actual Pointer.
 -------------------------------------------------------------------------------*/
boolean AssertPointerEqualMsg( char* file, int line, void* expectedPtr, void* actualPtr, ... ) {
    va_list args;
    char msgStr[1024];

    va_start(args, actualPtr);
    char* fmt = va_arg(args, char*);
    vsprintf(msgStr, fmt, args);
    va_end(args);

    if( expectedPtr != actualPtr ) {
        char* basename = strrchr(file, '/');
        basename = basename ? basename+1 : file;
        WriteToOutputBuffer("TEST FAILED: [%s:%d] %p does not match %p - %s\n", basename, line, expectedPtr, actualPtr, msgStr);
        return FALSE;
    }
    return TRUE;
}  // AssertPointerEqualMsg()

/*------------------------------------------------------------------------------
 | NAME:
 |    AssertStructEqualMsg()
 |
 | INPUT PARAMETERS:
 |    file - Filename of the calling file.
 |    line - Line Number of the calling file.
 |    expectedStructPtr - The Expected Structure Pointer.
 |    actualStructPtr - The Actual Structure Pointer.
 |    size - Size of the Structure to compare.
 |    ... - The variable argument array for the text
 |
 | RETURN VALUES:
 |    boolean - TRUE = Success; FALSE = Failure
 |
 | DESCRIPTION:
 |    This function verifies that the Expected Structure matches the Actual Structure.
 -------------------------------------------------------------------------------*/
boolean AssertStructEqualMsg( char* file, int line, void* expectedStructPtr, void* actualStructPtr, size_t size, ... ) {
    va_list args;
    char msgStr[1024];

    va_start(args, size);
    char* fmt = va_arg(args, char*);
    vsprintf(msgStr, fmt, args);
    va_end(args);

    if( memcmp(expectedStructPtr, actualStructPtr, size) != 0 ) {
        char tmpExpBuffer[(size*3)+10];
        char tmpActBuffer[(size*3)+10];
        tmpExpBuffer[0] = tmpActBuffer[0] = '\0';
        for( int loop = 0; loop < size; loop++ ) {
            char tmpBuffer[5];
            sprintf(tmpBuffer, "%02X ", ((uint8*)expectedStructPtr)[loop]);
            strcat(tmpExpBuffer, tmpBuffer);
            sprintf(tmpBuffer, "%02X ", ((uint8*)actualStructPtr)[loop]);
            strcat(tmpActBuffer, tmpBuffer);
        }
        char* basename = strrchr(file, '/');
        basename = basename ? basename+1 : file;
        WriteToOutputBuffer("TEST FAILED: [%s:%d] {%s} does not match {%s} - %s\n", basename, line, tmpExpBuffer, tmpActBuffer, msgStr);

        return FALSE;
    }
    return TRUE;
}  // AssertStructEqualMsg()

/*------------------------------------------------------------------------------
 | NAME:
 |    AssertEqualMsg()
 |
 | INPUT PARAMETERS:
 |    file - Filename of the calling file.
 |    line - Line Number of the calling file.
 |    expected - The Expected Value.
 |    actual - The Actual Value.
 |    ... - The variable argument array for the text
 |
 | RETURN VALUES:
 |    boolean - TRUE = Success; FALSE = Failure
 |
 | DESCRIPTION:
 |    This function verifies that the Expected Value matches the Actual Value.
 -------------------------------------------------------------------------------*/
boolean AssertEqualMsg( char* file, int line, uint64 expected, uint64 actual, ... ) {
    va_list args;
    char msgStr[1024];

    va_start(args, actual);
    char* fmt = va_arg(args, char*);
    vsprintf(msgStr, fmt, args);
    va_end(args);

    if( expected != actual ) {
        char* basename = strrchr(file, '/');
        basename = basename ? basename+1 : file;
        WriteToOutputBuffer("TEST FAILED: [%s:%d] %d does not match %d - %s\n", basename, line, expected, actual, msgStr);
        return FALSE;
    }
    return TRUE;
}  // AssertEqualMsg()

/*------------------------------------------------------------------------------
 | NAME:
 |    AssertNotEqualMsg()
 |
 | INPUT PARAMETERS:
 |    file - Filename of the calling file.
 |    line - Line Number of the calling file.
 |    expected - The Expected Value.
 |    actual - The Actual Value.
 |    ... - The variable argument array for the text
 |
 | RETURN VALUES:
 |    boolean - TRUE = Success; FALSE = Failure
 |
 | DESCRIPTION:
 |    This function verifies that the Expected Value does not match the Actual Value.
 -------------------------------------------------------------------------------*/
boolean AssertNotEqualMsg( char* file, int line, uint64 expected, uint64 actual, ... ) {
    va_list args;
    char msgStr[1024];

    va_start(args, actual);
    char* fmt = va_arg(args, char*);
    vsprintf(msgStr, fmt, args);
    va_end(args);

    if( expected == actual ) {
        char* basename = strrchr(file, '/');
        basename = basename ? basename+1 : file;
        WriteToOutputBuffer("TEST FAILED: [%s:%d] %d matches %d - %s\n", basename, line, expected, actual, msgStr);
        return FALSE;
    }
    return TRUE;
}  // AssertNotEqualMsg()





