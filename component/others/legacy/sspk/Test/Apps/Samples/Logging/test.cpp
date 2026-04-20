///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include <pkPAL.h>
#include <pkExecutive.h>
#include <pkTestFramework.h>
#include <TLCommon.h>

static void s_PrintUsage(bool_t fVerbose)
{
    const char *pszProgName = "Logging";

    TF_Printf(
        "===========================================================\n"
        "%s Usage:\n%s",
        pszProgName,
        "   -vf <vector file>\n"
        "   -sf <script file>\n"
        "   -lf <log file>\n"
        );
}


static char* s_szVectorFileURI;
static char* s_szScriptFileURI;
static char* s_szLogFileURI;

static bool s_IsAbandoned;


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

    if (pkS_OK != TLC_GetCmdLineArg("-sf", 1, &s_szScriptFileURI) )
    {
        s_PrintUsage(fVerbose);
        pkr = pkE_INVALIDARG;
        goto exit;
    }

    if (pkS_OK != TLC_GetCmdLineArg("-lf", 1, &s_szLogFileURI) )
    {
        s_PrintUsage(fVerbose);
        pkr = pkE_INVALIDARG;
        goto exit;
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
    static uint8_t s_VectBuffer[16];
    static char s_VectChars[17];

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

    pkrLog = TF_Logging_Printf(hLog, "Vector file size is %u bytes\n", (unsigned int)cbFileSize);
    if (pkFAILED(pkrLog))
    {
        goto exit;
    }

    s_IsAbandoned = false;

    while (!s_IsAbandoned)
    {
        unsigned int i;
        size_t cbVector = sizeof(s_VectBuffer);

        pkresult = TF_TestVectors_ReadBlock(hVector, s_VectBuffer, &cbVector);
        if (pkFAILED(pkresult))
        {
            pkrLog = TF_Logging_Printf(hLog, "TF_TestVectors_ReadBlock failed 0x%x\n", pkresult);
            if (pkFAILED(pkrLog))
            {
                goto exit;
            }
            break;
        }

        for (i=0; i < cbVector; i++)
        {
            pkrLog = TF_Logging_Printf(hLog, "%02x ", (unsigned int)s_VectBuffer[i]);
            if (pkFAILED(pkrLog))
            {
                goto exit;
            }

            if (s_VectBuffer[i] >= 32 && s_VectBuffer[i] <= 127)
            {
                s_VectChars[i] = (char)s_VectBuffer[i];
            }
            else
            {
                s_VectChars[i] = '.';
            }
        }
        s_VectChars[i] = '\0';

        pkrLog = TF_Logging_Printf(hLog, "%s\n", s_VectChars);
        if (pkFAILED(pkrLog))
        {
            goto exit;
        }

        if (sizeof(s_VectBuffer) != cbVector)
        {
            TF_Printf("TF_TestVectors_ReadBlock only %u bytes so we are done.\n", cbVector);
            break;
        }

        // Back up a little bit to test out relative seeking
        pkresult = TF_TestVectors_Seek(hVector, -4, TF_TestVectors_Seek_Cur);
        if (pkFAILED(pkresult))
        {
            pkrLog = TF_Logging_Printf(hLog, "TF_TestVectors_Seek Cur -10 failed 0x%x\n", pkresult);
            if (pkFAILED(pkrLog))
            {
                goto exit;
            }
        }
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
    static char s_ScriptBuffer[250];

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

    s_IsAbandoned = false;

    while (!s_IsAbandoned)
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

    pkresult = TF_Printf("Logging Test running ...\n");
    if (pkFAILED(pkresult))
    {
        goto bailout;
    }
    // Note: Since TF_Printf worked the first time we don't check it after this.

    pkresult = TF_Logging_Open( &hLog, s_szLogFileURI );
    if (pkFAILED(pkresult))
    {
        TF_Printf("TF_Logging_Open failed 0x%x (%s)\n", pkresult, s_szLogFileURI);
        goto exit;
    }

    pkresult = TF_Logging_Printf(hLog, "=============== Logging sample test output ============== \n");
    if (pkFAILED(pkresult))
    {
        TF_Printf("Test_Run TF_Logging_Printf failed 0x%x\n", pkresult);
        goto exit;
    }

    pkresult = s_RunTestVectorsTest(hLog);
    if (pkFAILED(pkresult))
    {
        goto exit;
    }

    pkresult = s_RunTestScriptingTest(hLog);
    if (pkFAILED(pkresult))
    {
        goto exit;
    }

    if (s_IsAbandoned)
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
    s_IsAbandoned = true;
    return pkS_OK;
}

