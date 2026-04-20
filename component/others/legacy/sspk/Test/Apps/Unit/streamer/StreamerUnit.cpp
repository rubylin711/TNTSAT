///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

// StreamerUnit.cpp : Defines the entry point for the console application.
//

#include "stdafx.h" // includes pkPAL.h
#include "StreamerHelper.h"
#include "StreamerUnit.h"

#include <pkSockets.h>
#include <pkTestFramework.h>
#include <TLCommon.h>
#include <fstream>

static void s_PrintUsage(bool_t fVerbose)
{
    const char *pszProgName = "StreamerUnit";

    TF_Printf(
        "===========================================================\n"
        "%s Usage:\n%s",
        pszProgName,
        "   -vf <vector file>\n"
        "   -sf <script file>\n"
        "   -lf <log file>\n"
        );
}


pkRESULT Test_GetConfig(int argc, char *argv[], TF_Config *pConfig)
{
    // parse through options...
    bool_t fVerbose;
    pkRESULT pkr = pkS_OK;

    TLC_SetCmdLineArgs(argc, argv);

    if (TLC_IsHelpRequested(&fVerbose))
    {
        s_PrintUsage(fVerbose);
        pkr = pkS_FALSE;
        goto exit;
    }

    // set the configuration
    if (pConfig->size < sizeof(TF_Config))
    {
        pkr = pkE_INVALIDARG;
        goto exit;
    }

    if (pkS_OK != TLC_GetCmdLineArg("-vf", 1, &s_szVectorFileURI) )
    {
        s_PrintUsage(fVerbose);
        pkr = pkE_INVALIDARG;
        goto exit;
    }
    else
    {
        t_AreVectorsAvailable = true;
    }

    if (pkS_OK != TLC_GetCmdLineArg("-sf", 1, &s_szScriptFileURI) )
    {
        s_PrintUsage(fVerbose);
        pkr = pkE_INVALIDARG;
        goto exit;
    }
    else
    {
        t_isScriptAvailable = true;
    }

    if (pkS_OK != TLC_GetCmdLineArg("-lf", 1, &s_szLogFileURI) )
    {
        s_PrintUsage(fVerbose);
        pkr = pkE_INVALIDARG;
        goto exit;
    }
    else
    {
        t_IsLoggingEnabled = true;
    }

exit:
    return pkr;
}

// ======================
// Test vectors I/O

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

static pkRESULT s_RunTestVectorsTest(pkHANDLE hLog)
{
    bool EndOfVectors = false;

    uint8_t s_VectBuffer[2048];
    size_t cbVector = sizeof(tTestVector);

    pkRESULT result = pkS_OK;
    pkRESULT pkresult = pkE_FAIL;
    pkRESULT pkrLog = pkS_OK;

    pkHANDLE hVector = NULL;

    pkresult = TF_TestVectors_Open( &hVector, s_szVectorFileURI );
    if (pkFAILED(pkresult))
    {
        pkrLog = TF_Logging_Printf(hLog, "TF_TestVectors_Open failed 0x%x (%s)\n", pkresult, s_szVectorFileURI);
        goto exit;
    }

    pkrLog = TF_Logging_Printf(hLog, ">>>> TF_TestVectors blocks in file %s\n", s_szVectorFileURI);
    if (pkFAILED(pkrLog))
    {
        goto exit;
    }

    uint32_t cbFileSize;

    pkresult = s_GetFileSize(hVector, &cbFileSize, hLog);
    if (pkFAILED(pkresult))
    {
        goto exit;
    }

//    pkrLog = TF_Logging_Printf(hLog, "Vector file size is %u bytes\n", (unsigned int)cbFileSize);

    if (pkFAILED(pkrLog))
    {
        goto exit;
    }

    while (EndOfVectors == false)
    {
        if (cbFileSize >= VectorSize)
        {
            pkresult = TF_TestVectors_ReadBlock(hVector, s_VectBuffer, &cbVector);
            cbFileSize = cbFileSize - VectorSize;
            result = (SOCKET_SUCCESS == Socket_Startup()) ? pkS_OK : pkE_FAIL;
            pkresult = TF_Printf("Launching Streaming Test ... \n");
            pkresult = ParseScript(hLog, pScriptBuffer, s_VectBuffer);
        }

        if (pkFAILED(pkresult))
        {
            pkrLog = TF_Logging_Printf(hLog, "TF_TestVectors_ReadBlock failed 0x%x\n", pkresult);
            if (pkFAILED(pkrLog))
            {
                goto exit;
            }
            break;
        }

        if (cbFileSize <= 0)
        {
            EndOfVectors = true;
        }
    }

    if (pkFAILED(pkrLog))
    {
        goto exit;
    }

    if (sizeof(s_VectBuffer) != cbVector)
    {
        TF_Printf("TF_TestVectors_ReadBlock only %u bytes so we are done.\n", cbVector);
    }

exit:
    if (NULL != hVector)
    {
        pkRESULT pkrClose = TF_TestVectors_Close(hVector);
        if (pkFAILED(pkrClose))
        {
            pkrLog = TF_Logging_Printf(hLog, "TF_TestVectors_Close returned 0x%x\n", pkrClose);
        }
    }

    if (pkFAILED(pkrLog))
    {
        TF_Printf("s_RunTestVectorsTest TF_Logging_Printf failed 0x%x\n", pkrLog);
    }
    return pkresult;
}


// ======================
// Test scripting I/O

static pkRESULT s_RunTestScriptingTest(pkHANDLE hLog)
{
    pkRESULT pkresult = pkE_FAIL;
    pkRESULT pkrLog = pkS_OK;

    pkHANDLE hScript = NULL;

    pkresult = TF_TestScripting_Open( &hScript, s_szScriptFileURI );
    if (pkFAILED(pkresult))
    {
        pkrLog = TF_Logging_Printf(hLog, "TF_TestScripting_Open failed 0x%x (%s)\n", pkresult, s_szScriptFileURI);
        goto exit;
    }

    pkrLog = TF_Logging_Printf(hLog, ">>>> TF_TestScripting strings in file %s\n", s_szScriptFileURI);
    if (pkFAILED(pkrLog))
    {
        goto exit;
    }

    g_IsAbandoned = false;

    while (!g_IsAbandoned)
    {
        size_t cbScript = sizeof(s_ScriptBuffer);

        pkresult = TF_TestScripting_ReadString(hScript, s_ScriptBuffer, cbScript);
        if (pkFAILED(pkresult))
        {
            pkrLog = TF_Logging_Printf(hLog, "TF_TestScripting_ReadString failed 0x%x\n", pkresult);
            if (pkFAILED(pkrLog))
            {
                goto exit;
            }
            break;
        }

        if (pkS_FALSE == pkresult)
        {
            TF_Printf("TF_TestScripting_ReadString EOF so we are done.\n");
            break;
        }

        pkrLog = TF_Logging_Printf(hLog, "%s", s_ScriptBuffer);
        if (pkFAILED(pkrLog))
        {
            goto exit;
        }
    }

exit:
    if (NULL != hScript)
    {
        pkRESULT pkrClose = TF_TestScripting_Close(hScript);
        if (pkFAILED(pkrClose))
        {
            pkrLog = TF_Logging_Printf(hLog, "TF_TestScripting_Close returned 0x%x\n", pkrClose);
        }
    }

    if (pkFAILED(pkrLog))
    {
        TF_Printf("s_RunTestScriptingTest TF_Logging_Printf failed 0x%x\n", pkrLog);
    }
    return pkresult;

}


// ======================
// Main test program
// ======================

pkRESULT Test_Run(void)
{
    pkRESULT pkresult = pkE_FAIL;

    pkHANDLE hLog = NULL;

    pkresult = TF_Printf("Streamer Unit Test running ...\n");
    if (pkFAILED(pkresult))
    {
        goto bailout;
    }

    pkresult = TF_Logging_Open( &hLog, s_szLogFileURI );
    if (pkFAILED(pkresult))
    {
        TF_Printf("TF_Logging_Open failed 0x%x (%s)\n", pkresult, s_szLogFileURI);
        goto exit;
    }

    pkresult = TF_Logging_Printf(hLog, "=============== StreamerUnit test output ============== \n");
    if (pkFAILED(pkresult))
    {
        TF_Printf("Test_Run TF_Logging_Printf failed 0x%x\n", pkresult);
        goto exit;
    }

    if (t_isScriptAvailable == TRUE)
    {
        pkresult = s_RunTestScriptingTest(hLog);
    }

    if (pkFAILED(pkresult))
    {
        goto exit;
    }

    if (t_AreVectorsAvailable == TRUE)
    {
        pkresult = s_RunTestVectorsTest(hLog);
    }

    if (pkFAILED(pkresult))
    {
        goto exit;
    }

    if (g_IsAbandoned)
    {
        TF_Printf("Abandoned writing %s\n", s_szLogFileURI);
    }
    else
    {
        TF_Printf("Finished writing %s\n", s_szLogFileURI);
    }

exit: // exit after file cleanup
    if (NULL != hLog)
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

bailout:
    return pkresult;
}


pkRESULT Test_Abandon(void)
{
    g_IsAbandoned = true;
    return pkS_OK;
}

