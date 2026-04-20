///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

// MP4Parser : Defines the entry point for the console application.
//

#include "stdafx.h" // includes pkPAL.h
#include "mp4FileTypeSpecificTests.h"
#include "mp4parserhelper.h"

#include <pkSockets.h>
#include <pkTestFramework.h>
#include <TLCommon.h>
#include <fstream>

// TODO: need to replace static memory allocation and global variables
char * s_szLogFileURI;
char* s_szVectorFileURI;
char* s_szScriptFileURI;

char s_ScriptBuffer[2048];
char * pScriptBuffer = &s_ScriptBuffer[0];

bool_t g_IsAbandoned = FALSE;
bool_t isVerboseHelpRequested = FALSE;
bool_t t_isScriptAvailable = FALSE;
bool_t t_AreVectorsAvailable = FALSE;

// TODO: Logging has to done per test or for an entire script. Create an enum to track this
bool t_IsLoggingEnabled = false;

// TODO: Move this API to generic library
static pkRESULT s_GetFileSize(pkHANDLE hFile, uint32_t *cbFileSize, pkHANDLE hLog)
{
    pkRESULT pkresult = pkS_OK;
    pkRESULT pkrLog = pkS_OK;
    int32_t lCurr;
    int32_t offset;

    if (NULL == hFile)
    {
        pkrLog = TF_Logging_Printf(hLog, "s_GetFileSize called with NULL file handle\n");
        pkresult = pkE_HANDLE;
        goto exit;
    }

    pkresult = TF_TestVectors_GetOffset(hFile, &lCurr);
    if (pkFAILED(pkresult))
    {
        pkrLog = TF_Logging_Printf(hLog, "TF_TestVectors_GetOffset lCurr failed 0x%x\n", pkresult);
        goto exit;
    }

    pkresult = TF_TestVectors_Seek(hFile, 0, TF_TestVectors_Seek_End);
    if (pkFAILED(pkresult))
    {
        pkrLog = TF_Logging_Printf(hLog, "TF_TestVectors_Seek End failed 0x%x\n", pkresult);
        goto exit;
    }

    pkresult = TF_TestVectors_GetOffset(hFile, &offset);
    if (pkFAILED(pkresult))
    {
        pkrLog = TF_Logging_Printf(hLog, "TF_TestVectors_GetOffset offset failed 0x%x\n", pkresult);
        goto exit;
    }

    pkresult = TF_TestVectors_Seek(hFile, lCurr, TF_TestVectors_Seek_Set);
    if (pkFAILED(pkresult))
    {
        pkrLog = TF_Logging_Printf(hLog, "TF_TestVectors_Seek Set failed 0x%x\n", pkresult);
        goto exit;
    }

    *cbFileSize = (uint32_t)offset;

exit:
    if (pkFAILED(pkrLog))
    {
        TF_Printf("s_GetFileSize TF_Logging_Printf failed 0x%x\n", pkrLog);
    }
    return pkresult;
}

bool LaunchMP4Parser(pkHANDLE hLog, char * fileType, char * chunkFile, char * chunkData)
{
    bool result = false;
    tTestVector * pTestVector;
    tTestVector testVector;

    pTestVector = &testVector;

    FILE* myfile;
    fopen_s( &myfile, chunkFile, "rb");

    TF_Logging_Printf(hLog, "Vector Filename is %s \n", chunkFile);

    // Parse the vector
    ParseVector(chunkData, pTestVector);

    MP4ParserTest thisMP4ParserTest(hLog);
    const static uint32 size2 = 1024*1024;
    unsigned char memblock[size2];
    thisMP4ParserTest.Init(hLog, myfile, memblock, size2);

    result = thisMP4ParserTest.MP4ParserAnalysis(pTestVector->ChunkType, pTestVector->Encrypted, pTestVector->FileSize);

    fclose(myfile);

    return result;
}


// ========================================================================
// TestFramework entry points
// ========================================================================

static void s_PrintUsage(bool_t fVerbose)
{
    const char *pszProgName = "MP4ParserUnitTest";

    TF_Printf(
        "===========================================================\n"
        "%s Usage:\n%s",
        pszProgName,
        "   -lf <log file>\n"
        "   -vf <vector file>\n"
        "   -sf <script file>\n"
        );

    TF_Printf("fileType can be video, audio, or subtitle \n");
    TF_Printf("filename refers to the Smooth Streaming chunks \n");
}

pkRESULT pkAPI Test_GetConfig(int argc, char *argv[], TF_Config *pConfig)
{
    pkRESULT result = pkS_OK;

    TLC_SetCmdLineArgs(argc, argv);

    // help can be requested anywhere in command line
    if (TLC_IsHelpRequested(&isVerboseHelpRequested))
    {
        TF_Printf("Usage: MP4ParserUnit [fileType] [filename]\n");
        if (isVerboseHelpRequested)
        {
            s_PrintUsage(isVerboseHelpRequested);
        }
    }

    // set the scripts
    if (pkS_OK != TLC_GetCmdLineArg("-sf", 1, &s_szScriptFileURI) )
    {
        TF_Printf("Usage: MP4ParserUnit -sf argument failed \n");
        s_PrintUsage(isVerboseHelpRequested);
        result = pkE_INVALIDARG;
    }
    else
    {
        t_isScriptAvailable = true;
    }

    // set the vectors
    if (pkS_OK != TLC_GetCmdLineArg("-vf", 1, &s_szVectorFileURI) )
    {
        TF_Printf("Usage: MP4ParserUnit -vf argument failed \n");
        s_PrintUsage(isVerboseHelpRequested);
        result = pkE_INVALIDARG;
    }
    else
    {
        t_AreVectorsAvailable = true;
    }


    // set the logging
    if (pkS_OK != TLC_GetCmdLineArg("-lf", 1, &s_szLogFileURI) )
    {
        TF_Printf("Usage: MP4ParserUnit -lf argument failed \n");
        s_PrintUsage(isVerboseHelpRequested);
        result = pkE_INVALIDARG;
    }
    else
    {
        t_IsLoggingEnabled = true;
    }

    return result;
}

pkRESULT pkAPI Test_Run(void)
{
    // Result Codes
    pkRESULT pkrLog = pkS_OK;
    pkRESULT pkresult = pkE_FAIL;

    // File Handles
    pkHANDLE hLog = NULL;
    pkHANDLE hVector = NULL;
    pkHANDLE hScript = NULL;

    // Vector File Size
    uint32_t vectorFileSize = 0;
    uint32_t *cbFileSize = &vectorFileSize;
    uint32_t fileSize;

    // Vector Block Size
    uint8_t vectorBuffer[VectorSize];
    size_t vectorSize = (size_t)VectorSize;

    bool EndOfVectors = false;

    pkresult = TF_Printf("MP4Parser Unit Test running ...\n");

    // Check the Test Framework Print once
    if (pkFAILED(pkresult))
    {
        goto exit;
    }

    // Open the log file
    pkresult = TF_Logging_Open( &hLog, s_szLogFileURI );
    if (pkFAILED(pkresult))
    {
        TF_Printf("TF_Logging_Open failed 0x%x (%s)\n", pkresult, s_szLogFileURI);
        goto exit;
    }

    pkresult = TF_Logging_Printf(hLog, "=============== MP4Parser Unit Test output ============== \n");
    if (pkFAILED(pkresult))
    {
        TF_Printf("TF_Logging_Printf failed 0x%x (%s)\n", pkresult, s_szLogFileURI);
        goto exit;
    }

    if (t_isScriptAvailable == TRUE)
    {

        // Open the Script File
        size_t cbScript = sizeof(s_ScriptBuffer);
        pkresult =     TF_TestScripting_Open(&hScript, s_szScriptFileURI);
        if (pkFAILED(pkresult))
        {
            pkresult = TF_Logging_Printf(hLog, " Unable to open the script file \n");
            goto exit;
        }

        // TODO: need to modify to read multi-line scripts
        // Read it to a buffer
        pkresult =     TF_TestScripting_ReadString(hScript, s_ScriptBuffer, cbScript);
        if (pkFAILED(pkresult))
        {
            pkresult = TF_Logging_Printf(hLog, " Unable to read the scripting file \n");
            goto exit;
        }

        // Close the Script File
        pkresult = TF_TestScripting_Close(hScript);
        if (pkFAILED(pkresult))
        {
            pkresult = TF_Logging_Printf(hLog, " Unable to open the close the scripting file \n");
            goto exit;
        }

    }

    if (t_AreVectorsAvailable == TRUE)
    {
        // Open Vectors File
        pkresult = TF_TestVectors_Open(&hVector, s_szVectorFileURI );

        if (pkFAILED(pkresult))
        {
            pkrLog = TF_Logging_Printf(hLog, "TF_TestVectors_Open failed 0x%x (%s)\n", pkresult, s_szVectorFileURI);
            goto exit;
        }

        pkresult = s_GetFileSize(hVector, cbFileSize , hLog);
        fileSize = *cbFileSize;

        // Read the vectors
        // TODO: Vectors are in binary data in form of blocks. Need to read text vectors
        while (EndOfVectors == false)
        {
            if (fileSize >= VectorSize)
            {
                pkresult = TF_TestVectors_ReadBlock(hVector, vectorBuffer, &vectorSize);
                fileSize = fileSize - VectorSize;
                pkresult = ParseScript(hLog, pScriptBuffer, vectorBuffer);
            }

            if (fileSize <= 0)
            {
                EndOfVectors = true;
            }
        }

        // Close the vectors
        pkresult = TF_TestVectors_Close(hVector);

        if (pkFAILED(pkresult))
        {
            pkresult = TF_Logging_Printf(hLog, " Unable to close the vector file \n");
            goto exit;
        }
    }


    // Parse the script
    if ((t_isScriptAvailable == TRUE) && (t_AreVectorsAvailable == FALSE))
    {
        pkresult = ParseScript(hLog, pScriptBuffer, NULL);
    }


exit:
    // exit after file cleanup
    if (hLog != NULL)
    {
        pkRESULT pkrClose = TF_Logging_Close(hLog);
        if (pkFAILED(pkrClose))
        {
            TF_Printf("TF_Logging_Close returned 0x%x\n", pkrClose);
        }
    }

    if (pkFAILED(pkresult))
    {
        TF_Printf("Test_Run returning error 0x%x\n", pkresult);
    }

    return pkS_OK;

}

pkRESULT pkAPI Test_Abandon(void)
{
    TF_Printf("Abandonment requested. Please hit Enter key.\n");
    g_IsAbandoned = true;
    return pkS_OK;
}

