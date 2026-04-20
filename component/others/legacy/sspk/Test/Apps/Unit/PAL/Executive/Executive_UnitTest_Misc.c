///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//
// Executive_UnitTest_Misc.c: Defines the entry point for the sample Media Center Extender Platform
// Unit tests for the remaining Executive PAL miscellaneous implementation.
//

//=============================================================================
// I N C L U D E     F I L E S
//=============================================================================
#include "Executive_UnitTest.h"


//=============================================================================
// C O N S T A N T   S T R U C T U R E    D E F I N I T I O N S
//=============================================================================
#define MAX_RANDOM_TEST_ITERATIONS 50
//=============================================================================
// L O C A L    F U N C T I O N    P R O T O T Y P E S
//=============================================================================
static pkRESULT ExecTest_Executive_Sleep(uint32_t sleepTime);
static pkRESULT ExecTest_Executive_Random( uint32_t iterations);


//=============================================================================
// P U B L I C    F U N C T I O N    D E F I N I T I O N S
//=============================================================================
pkRESULT ExecTest_MiscBasicTest(void)
{
    pkRESULT retval = pkE_FAIL, hr;
    uint32_t sleepTime, iterations;
    uint32_t casesPassed = 0, casesFailed = 0;

    //Case Number 1
    //Basic Test
    //Functional Group: Misc
    //Test Title: Sleep for a specified time with verification from Executive_GetTickCount
    if ( TRUE == g_bIsAborted)
    {
        goto exit;
    }
    sleepTime = 113;
    hr = ExecTest_Executive_Sleep( sleepTime );
    if ( pkSUCCEEDED(hr))
    {
        casesPassed++;
    }
    else
    {
        casesFailed++;
        TF_Printf("Case 1 failed with error: %x\n", hr);
    }

exit:
    TF_Printf("*************************************\n");
    TF_Printf("Executive Unit Test: Basic Miscellaneous Test Results\n");
    TF_Printf("Tests Passed: %d\n",casesPassed);
    TF_Printf("Tests Failed: %d\n",casesFailed);
    TF_Printf("*************************************\n");

    if ( 0 == casesFailed )
    {
        retval = pkS_OK;
    }
    else
    {
        retval = pkE_FAIL;
    }

    if ( TRUE == g_bIsAborted)
    {
        retval = pkE_ABORT;
    }

    return retval;

}

pkRESULT ExecTest_MiscExtendedTest(void)
{
    uint32_t casesPassed = 0, casesFailed = 0;
    pkRESULT retval = pkE_FAIL;

    TF_Printf("*************************************\n");
    TF_Printf("Executive Unit Test: Extended Event Test Results\n");
    TF_Printf("Tests Passed: %d\n",casesPassed);
    TF_Printf("Tests Failed: %d\n",casesFailed);
    TF_Printf("*************************************\n");

    if ( 0 == casesFailed )
    {
        retval = pkS_OK;
    }
    else
    {
        retval = pkE_FAIL;
    }

    if ( TRUE == g_bIsAborted)
    {
        retval = pkE_ABORT;
    }
    return retval;

}

//=============================================================================
// L O C A L    F U N C T I O N    D E F I N I T I O N S
//=============================================================================

static pkRESULT ExecTest_Executive_Sleep(uint32_t sleepTime)
{
    pkRESULT retval = pkE_FAIL;
    uint32_t ticksBefore, ticksAfter;

    ticksBefore = Executive_GetTickCount();
    Executive_Sleep(sleepTime);
    ticksAfter = Executive_GetTickCount();
    if ( ( (ticksAfter - ticksBefore) > (sleepTime + ACCEPTABLE_SKEW_MS)) || ( ((double)sleepTime - ACCEPTABLE_SKEW_MS) <= 0 ? FALSE : ( (ticksAfter - ticksBefore) < (sleepTime - ACCEPTABLE_SKEW_MS)) ) )
    {
        TF_Printf("Executive Unit Test: Incorrect Sleep duration.  Expected duration between %d and %d, actual duration was %d\n", ( ((double)sleepTime - ACCEPTABLE_SKEW_MS) <= 0 ? 0 : (sleepTime - ACCEPTABLE_SKEW_MS)), sleepTime + ACCEPTABLE_SKEW_MS, ticksAfter - ticksBefore);
        retval = pkE_FAIL;
    }
    else
    {
        retval = pkS_OK;
    }

    return retval;
}



