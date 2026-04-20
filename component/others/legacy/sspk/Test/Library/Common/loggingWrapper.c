///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//===========================================================================
// Includes
//===========================================================================
#include <PAL.h>
#include <pkTestFramework.h>
#include <StringSafe.h>
#include "TLCommon.h"


//===========================================================================
// Static variables
//===========================================================================
static bool_t s_TLC_UseVerboseLogging = FALSE;
static bool_t s_TLC_NoLogFile = FALSE;
static bool_t s_TLC_LastTestEnded = TRUE;


static uint32_t s_TLC_FunctionalGroupCasesPassed = 0;
static uint32_t s_TLC_FunctionalGroupCasesFailed = 0;
static uint32_t s_TLC_FunctionalGroupCasesSkipped = 0;
static uint32_t s_TLC_SummaryCasesPassed = 0;
static uint32_t s_TLC_SummaryCasesFailed = 0;
static uint32_t s_TLC_SummaryCasesSkipped = 0;

//===========================================================================
// Basic functions
//===========================================================================

//Used to turn verbose logging on or off
//By default, verbose logging is OFF
void TLC_SetVerboseLogging
    (
        bool_t LoggingSetting
    )
{
    s_TLC_UseVerboseLogging = LoggingSetting;
    return;
}

//Used to turn logging to a log file on or off
//Note: The semantics of the name of the setting and the
//setting used in this set function are opposite.
//One is easier for the caller of the wrapper to understand
//and the other is more clear in the context of the wrapper code

//By default, logging is sent to a file
void TLC_SetLogToFile
    (
        bool_t UseLogFile
    )
{
    s_TLC_NoLogFile=!(UseLogFile);
    return;
}


// Templatized output for logging the beginning of Unit Test execution
pkRESULT TLC_LogUnitTestBegin
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szSuitesSelected,
        const char *szTestPassType
    )
{
    pkRESULT hr, retval;
    char *szFormatString = "\n************Unit Test Begin******\nUnit Test Name: %s\nSuites Selected: %s\nTest Pass Type: %s\n**********************************\n\n";

    //Clear out our variables tracking test statistics
    s_TLC_SummaryCasesPassed = 0;
    s_TLC_SummaryCasesFailed = 0;
    s_TLC_SummaryCasesSkipped = 0;
    s_TLC_FunctionalGroupCasesPassed = 0;
    s_TLC_FunctionalGroupCasesFailed = 0;
    s_TLC_FunctionalGroupCasesSkipped = 0;

    retval = pkE_FAIL;

    //Check to see if hLog matters and is NULL
    if ( NULL == hLog && FALSE == s_TLC_NoLogFile)
    {
        retval = pkE_HANDLE;
        goto bail;
    }

    if ( FALSE == s_TLC_NoLogFile)
    {
        hr = TF_Logging_Printf(hLog, szFormatString, szUnitTestName, szSuitesSelected, szTestPassType);
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }
    }

    hr = TF_Printf(szFormatString, szUnitTestName, szSuitesSelected, szTestPassType);
    if (!pkSUCCEEDED(hr))
    {
        retval=hr;
        goto bail;
    }

    retval = pkS_OK;

bail:
    if ( pkFAILED(retval) )
    {
        TF_Printf("Logging failure in TLC_LogUnitTestBegin, test logs incomplete!\n");
    }
    return retval;

}

// ================================================
// Templatized output for logging the summary results of a Unit Test execution
pkRESULT TLC_LogUnitTestSummary
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szSuiteName,
        const char *szTestPassType,
        uint32_t* passCount,
        uint32_t* failCount,
        uint32_t* skipCount

    )
{
    pkRESULT hr, retval;
    char *szHeaderFormatString = "\n************Unit Test Summary******\nUnit Test: %s\nSuites Run: %s\nTest Pass Type: %s\n";
    char *szDataFormatString = "Tests Passed: %d | Tests Failed: %d | Tests Skipped: %d\n";
    char *szFooterFormatString = "**********************************\n\n";

    retval = pkE_FAIL;

    // Check out our pointers
    // If all are null it is fine, since they are optional
    // If just some are null, we should return an error
    if ( ( NULL == passCount ) || ( NULL == failCount ) || ( NULL == skipCount ) )
    {
        if  ( ( NULL == passCount ) && ( NULL == failCount ) && ( NULL == skipCount ) )
        {
            //All pointers are null
        }
        else
        {
            //Some, but not all pointers are null which is OK
            return pkE_POINTER;
            goto bail;
        }
    }


    //Check to see if hLog matters and is NULL
    if ( NULL == hLog && FALSE == s_TLC_NoLogFile)
    {
        retval = pkE_HANDLE;
        goto bail;
    }

    if ( FALSE == s_TLC_NoLogFile)
    {
        hr = TF_Logging_Printf(hLog, szHeaderFormatString, szUnitTestName, szSuiteName, szTestPassType);
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }
        hr = TF_Logging_Printf(hLog, szDataFormatString, s_TLC_SummaryCasesPassed, s_TLC_SummaryCasesFailed, s_TLC_SummaryCasesSkipped);
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }
        hr = TF_Logging_Printf(hLog, szFooterFormatString );
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }
    }

    hr = TF_Printf(szHeaderFormatString, szUnitTestName, szSuiteName, szTestPassType);
    if (!pkSUCCEEDED(hr))
    {
        retval=hr;
        goto bail;
    }

    hr = TF_Printf(szDataFormatString, s_TLC_SummaryCasesPassed, s_TLC_SummaryCasesFailed, s_TLC_SummaryCasesSkipped);
    if (!pkSUCCEEDED(hr))
    {
        retval=hr;
        goto bail;
    }

    hr = TF_Printf(szFooterFormatString);
    if (!pkSUCCEEDED(hr))
    {
        retval=hr;
        goto bail;
    }

    retval = pkS_OK;
    if  ( !(( NULL == passCount ) && ( NULL == failCount ) && ( NULL == skipCount )) )
    {
        *passCount = s_TLC_SummaryCasesPassed;
        *failCount = s_TLC_SummaryCasesFailed;
        *skipCount = s_TLC_SummaryCasesSkipped;
    }

bail:
    if ( pkFAILED(retval) )
    {
        TF_Printf("Logging failure in TLC_LogUnitTestSummary, test logs incomplete!\n");
    }
    return retval;

}

// ================================================
// Templatized output for logging the beginning of a group of related tests or a test suite
pkRESULT TLC_LogTestGroupBegin
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szFunctionalGroupName

    )
{
    pkRESULT hr, retval;
    char *szHeaderFormatString = "\n************%s Group Begin******\nFunctional Group: %s\n**********************************\n\n";

    retval = pkE_FAIL;

    //Check to see if hLog matters and is NULL
    if ( NULL == hLog && FALSE == s_TLC_NoLogFile)
    {
        retval = pkE_HANDLE;
        goto bail;
    }

    //Reset count of functional group cases
    s_TLC_FunctionalGroupCasesPassed = 0;
    s_TLC_FunctionalGroupCasesFailed = 0;
    s_TLC_FunctionalGroupCasesSkipped = 0;


    if ( FALSE == s_TLC_LastTestEnded)
    {
        TF_Printf("%s: WARNING: TLC_LogTestBegin called without prior TLC_LogTestEnd.  Old statistics lost.\n", szUnitTestName);
        s_TLC_LastTestEnded = TRUE;
    }

    if ( FALSE == s_TLC_NoLogFile)
    {
        hr = TF_Logging_Printf(hLog, szHeaderFormatString, szUnitTestName, szFunctionalGroupName );
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }
    }

    hr = TF_Printf( szHeaderFormatString, szUnitTestName, szFunctionalGroupName );
    if (!pkSUCCEEDED(hr))
    {
        retval=hr;
        goto bail;
    }

    retval = pkS_OK;

bail:
    if ( pkFAILED(retval) )
    {
        TF_Printf("Logging failure in TLC_LogTestBegin, test logs incomplete!\n");
    }
    return retval;

}

// ================================================
// Templatized output for logging the ending of a group of related tests or a test suite
pkRESULT TLC_LogTestGroupEnd
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szFunctionalGroupName,
        uint32_t* passCount,
        uint32_t* failCount,
        uint32_t* skipCount
    )
{
    pkRESULT hr, retval;
    char *szHeaderFormatString = "\n************%s Group End******\nTest Group: %s\n";
    char *szDataFormatString = "Tests Passed: %d | Tests Failed: %d | Tests Skipped: %d\n";
    char *szFooterFormatString = "**********************************\n\n";

    retval = pkE_FAIL;

    // Check out our pointers
    // If all are null it is fine, since they are optional
    // If just some are null, we should return an error
    if ( ( NULL == passCount ) || ( NULL == failCount ) || ( NULL == skipCount ) )
    {
        if  ( ( NULL == passCount ) && ( NULL == failCount ) && ( NULL == skipCount ) )
        {
            //All pointers are null
        }
        else
        {
            //Some, but not all pointers are null
            return pkE_POINTER;
            goto bail;
        }
    }

    //Check to see if hLog matters and is NULL
    if ( NULL == hLog && FALSE == s_TLC_NoLogFile)
    {
        retval = pkE_HANDLE;
        goto bail;
    }

    if ( FALSE == s_TLC_NoLogFile)
    {
        hr = TF_Logging_Printf(hLog, szHeaderFormatString, szUnitTestName, szFunctionalGroupName);
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }
        hr = TF_Logging_Printf(hLog, szDataFormatString, s_TLC_FunctionalGroupCasesPassed, s_TLC_FunctionalGroupCasesFailed, s_TLC_FunctionalGroupCasesSkipped);
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }
        hr = TF_Logging_Printf(hLog, szFooterFormatString );
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }
    }

    hr = TF_Printf( szHeaderFormatString, szUnitTestName, szFunctionalGroupName );
    if (!pkSUCCEEDED(hr))
    {
        retval=hr;
        goto bail;
    }
    hr = TF_Printf( szDataFormatString, s_TLC_FunctionalGroupCasesPassed, s_TLC_FunctionalGroupCasesFailed, s_TLC_FunctionalGroupCasesSkipped );
    if (!pkSUCCEEDED(hr))
    {
        retval=hr;
        goto bail;
    }
    hr = TF_Printf( szFooterFormatString );
    if (!pkSUCCEEDED(hr))
    {
        retval=hr;
        goto bail;
    }

    retval = pkS_OK;
    if  ( !(( NULL == passCount ) && ( NULL == failCount ) && ( NULL == skipCount )) )
    {
        *passCount = s_TLC_FunctionalGroupCasesPassed;
        *failCount = s_TLC_FunctionalGroupCasesFailed;
        *skipCount = s_TLC_FunctionalGroupCasesSkipped;
    }

    s_TLC_LastTestEnded = TRUE;

bail:
    if ( pkFAILED(retval) )
    {
        TF_Printf("Logging failure in TLC_LogTestEnd, test logs incomplete!\n");
    }
    return retval;

}

// ================================================
// Templatized output for logging a test case pass result
pkRESULT TLC_LogTestCasePass
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szTestCaseName,
        uint32_t testCaseNumber
    )
{
    pkRESULT hr, retval;
    char *szFormatString = "%s: Case PASS - %d: %s\n";

    retval = pkE_FAIL;
    if ( (NULL == szUnitTestName) || (NULL == szTestCaseName) )
    {
        retval = pkE_POINTER;
        goto bail;
    }

    s_TLC_FunctionalGroupCasesPassed++;
    s_TLC_SummaryCasesPassed++;

    //Check to see if hLog matters and is NULL
    if ( NULL == hLog && FALSE == s_TLC_NoLogFile)
    {
        retval = pkE_HANDLE;
        goto bail;
    }

    if ( FALSE == s_TLC_NoLogFile)
    {
        hr = TF_Logging_Printf(hLog, szFormatString, szUnitTestName, testCaseNumber, szTestCaseName);
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }
    }

    else
    {
        hr = TF_Printf( szFormatString, szUnitTestName, testCaseNumber, szTestCaseName);
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }
    }

    retval = pkS_OK;

bail:
    if ( pkFAILED(retval) )
    {
        TF_Printf("Logging failure in TLC_LogTestCasePass, test logs incomplete!\n");
    }
    return retval;
}

// ================================================
// Templatized output for logging a test case fail result
// szFailureReason and failureCode can be Null and 0 respectively
// if they are not known.
pkRESULT TLC_LogTestCaseFail
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szTestCaseName,
        uint32_t testCaseNumber,
        const char *szFailureReason,
        pkRESULT failureCode
    )
{
    pkRESULT hr, retval=pkE_FAIL;
    int err = 0;
    bool_t bPrintReason = FALSE;
    char szFailedString[250];
    char *szFormatString = "%s: Case FAILED - %u: %s\n";
    char *szFormatStringWithCode = "%s: Case FAILED - %u: %s with code 0x%x\n";
    char *szReasonString = "%s: Failure information: %s\n";

    if ( (NULL == szUnitTestName) || (NULL == szTestCaseName) )
    {
        retval = pkE_POINTER;
        goto bail;
    }

    s_TLC_FunctionalGroupCasesFailed++;
    s_TLC_SummaryCasesFailed++;

    if ( NULL != szFailureReason)
    {
        bPrintReason = TRUE;
    }

    if ( 0 == failureCode )
    {
        hr = StringSafe_CbPrintfA(szFailedString, sizeof(szFailedString), szFormatString, szUnitTestName, testCaseNumber, szTestCaseName );
        if ( pkFAILED(hr))
        {
            retval = hr;
            goto bail;
        }
    }
    else
    {
        hr = StringSafe_CbPrintfA(szFailedString, sizeof(szFailedString), szFormatStringWithCode, szUnitTestName, testCaseNumber, szTestCaseName, failureCode);
        if ( pkFAILED(hr))
        {
            retval = hr;
            goto bail;
        }
    }

    //Check to see if hLog matters and is NULL
    if ( NULL == hLog && FALSE == s_TLC_NoLogFile)
    {
        retval = pkE_HANDLE;
        goto bail;
    }

    if ( FALSE == s_TLC_NoLogFile)
    {
        hr = TF_Logging_Printf(hLog, szFailedString);
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }

        if ( TRUE == bPrintReason)
        {
            hr = TF_Logging_Printf(hLog, szReasonString, szUnitTestName, szFailureReason);
            if (!pkSUCCEEDED(hr))
            {
                retval=hr;
                goto bail;
            }
        }


    }

    hr = TF_Printf( szFailedString );
    if (!pkSUCCEEDED(hr))
    {
        retval=hr;
        goto bail;
    }
    if ( TRUE == bPrintReason)
    {
        hr = TF_Logging_Printf(hLog, szReasonString, szUnitTestName, szFailureReason);
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }
    }


    retval = pkS_OK;

bail:
    if ( pkFAILED(retval) )
    {
        TF_Printf("Logging failure in TLC_LogTestCaseFail, test logs incomplete!\n");
    }
    return retval;
}

// ================================================
// Templatized output for logging a skipped test case
pkRESULT TLC_LogTestCaseSkip
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szTestCaseName,
        uint32_t testCaseNumber,
        const char *szSkipReason
    )
{
    pkRESULT hr, retval;
    char *szFormatString = "%s: Case SKIPPED - %d: %s because: %s\n";

    retval = pkE_FAIL;

    if ( (NULL == szUnitTestName) || (NULL == szTestCaseName) )
    {
        retval = pkE_POINTER;
        goto bail;
    }

    s_TLC_FunctionalGroupCasesSkipped++;
    s_TLC_SummaryCasesSkipped++;

    //Check to see if hLog matters and is NULL
    if ( NULL == hLog && FALSE == s_TLC_NoLogFile)
    {
        retval = pkE_HANDLE;
        goto bail;
    }

    if ( FALSE == s_TLC_NoLogFile)
    {
        hr = TF_Logging_Printf(hLog, szFormatString, szUnitTestName, testCaseNumber, szTestCaseName, szSkipReason);
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }
    }

    hr = TF_Printf(hLog, szFormatString, szUnitTestName, testCaseNumber, szTestCaseName, szSkipReason);
    if (!pkSUCCEEDED(hr))
    {
        retval=hr;
        goto bail;
    }

    retval = pkS_OK;

bail:
    if ( pkFAILED(retval) )
    {
        TF_Printf("Logging failure in TLC_LogTestCaseSkip, test logs incomplete!\n");
    }
    return retval;
}

pkRESULT TLC_LogWarning
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char *szWarningText
    )
{
    pkRESULT hr, retval;
    char *szFormatString = "%s: WARNING: %s\n";

    retval = pkE_FAIL;

    if ( (NULL == szUnitTestName) )
    {
        retval = pkE_POINTER;
        goto bail;
    }

    //Check to see if hLog matters and is NULL
    if ( NULL == hLog && FALSE == s_TLC_NoLogFile)
    {
        retval = pkE_HANDLE;
        goto bail;
    }

    if ( FALSE == s_TLC_NoLogFile)
    {
        hr = TF_Logging_Printf(hLog, szFormatString, szUnitTestName, szWarningText );
        if (!pkSUCCEEDED(hr))
        {
            retval=hr;
            goto bail;
        }
    }

    hr = TF_Printf(szFormatString, szUnitTestName, szWarningText );
    if (!pkSUCCEEDED(hr))
    {
        retval=hr;
        goto bail;
    }

    retval = pkS_OK;

bail:
    if ( pkFAILED(retval) )
    {
        TF_Printf("Logging failure in TLC_LogWarning, test logs incomplete!\n");
    }
    return retval;
}

// ================================================
// Templatized output for logging verbose output as appropriate
// This is a no-op if verbose logging is turned off (default)
pkRESULT TLC_LogVerbose
    (
        pkHANDLE hLog,
        const char *szUnitTestName,
        const char* szFormatString,
        ...
    )
{
    pkRESULT hr, retval;
    va_list arglist;
    char buffer[250]; //use same buffer size as TF_Logging_Printf
    char* iterator = buffer + 4;
    size_t cchDest = 0;

    if (FALSE == s_TLC_UseVerboseLogging)
    {
        retval = pkS_FALSE;
        goto bail;
    }

    StringSafe_CchCopyA(buffer, sizeof(buffer), "%s: ");

    if ( (NULL == szUnitTestName) || (NULL == szFormatString) )
    {
        retval = pkE_POINTER;
        goto bail;
    }

    cchDest = (sizeof(buffer) - 4)/sizeof(char);
    va_start(arglist,szFormatString);
    hr = StringSafe_CchVPrintfA(iterator, sizeof(buffer) - 4, szFormatString, arglist );
    va_end(arglist);

    if (pkFAILED(hr))
    {
        retval = hr;
        goto bail;
    }


    if ( FALSE == s_TLC_NoLogFile )
    {
        hr = TF_Logging_Printf(hLog, buffer, szUnitTestName );
        if (pkFAILED(hr))
        {
            retval=hr;
            goto bail;
        }
    }

    else
    {
        hr = TF_Printf( buffer, szUnitTestName );
        if (pkFAILED(hr))
        {
            retval=hr;
            goto bail;
        }
    }

    retval = pkS_OK;



bail:

    if ( pkFAILED(retval) )
    {
        TF_Printf("Logging failure in TLC_LogVerbose, test logs incomplete!\n");
    }
    return retval;

}
