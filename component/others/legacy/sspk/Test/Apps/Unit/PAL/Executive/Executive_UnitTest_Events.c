///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//
// Executive_UnitTest_Events.c: Defines the entry point for the sample Media Center Extender Platform.
// Unit tests for the Executive PAL Event implementation.
//
/* Test the following APIs:
pkRESULT pkAPI Executive_CreateEvent(
            LPCTSTR *pszName,
            bool_t isManualReset,
            bool_t isSignaled,
            pkHANDLE *pHandle);

pkRESULT pkAPI Executive_OpenEvent(
            LPCTSTR pszName,
            pkHANDLE *pHandle);

pkRESULT pkAPI Executive_SetEvent(
            pkHANDLE hHandle);

pkRESULT pkAPI Executive_ResetEvent(
            pkHANDLE hHandle);

pkRESULT pkAPI Executive_WaitForEvent(
            pkHANDLE hHandle,
            uint32_t timeOutMs);


*/
//=============================================================================
// I N C L U D E     F I L E S
//=============================================================================
#include "Executive_UnitTest.h"


//=============================================================================
// L O C A L    F U N C T I O N    P R O T O T Y P E S
//=============================================================================
static uint32_t pkAPI Thread_One_Proc(void *pParam);
static uint32_t pkAPI Thread_Two_Proc(void *pParam);
static uint32_t pkAPI Thread_Three_Proc(void *pParam);

//=============================================================================
// P U B L I C    F U N C T I O N    D E F I N I T I O N S
//=============================================================================
pkRESULT ExecTest_EventBasicTest(void)
{
    uint32_t casesPassed = 0, casesFailed = 0;
    pkRESULT retval = pkE_FAIL, result = pkE_FAIL;
    pkHANDLE EventHandle= NULL;
    pkHANDLE ThreadHandle1 = NULL;
    pkHANDLE ThreadHandle2 = NULL;
    pkHANDLE ThreadHandle3 = NULL;

    TF_Printf("Starting ExecTest_TestEventFunctions \n");

    result = Executive_CreateEvent(NULL, TRUE, TRUE, &EventHandle);
    TF_Printf("Executive_CreateEvent(NULL, TRUE, &EventHandle)  result: %x \n", result);
    if (pkFAILED(result))
    {
        casesFailed++;
    }
    else
    {
        casesPassed++;
    }


    result = Executive_WaitForEvent(EventHandle, 0);
    TF_Printf("Executive_WaitForEvent(EventHandle, 0)  result: %x \n", result);
    if (pkFAILED(result))
    {
        casesFailed++;
    }
    else
    {
        casesPassed++;
    }

    result = Executive_CreateThread(Thread_One_Proc, EventHandle, 0, &ThreadHandle1);
    TF_Printf("Executive_CreateThread(Thread_One_Proc, &EventHandle, 0, &ThreadHandle1)  result: %x \n", result);

    result = Executive_CreateThread(Thread_Two_Proc, EventHandle, 0, &ThreadHandle2);
    TF_Printf("Executive_CreateThread(Thread_Two_Proc, &EventHandle, 0, &ThreadHandle2)  result: %x \n", result);

    result = Executive_CreateThread(Thread_Three_Proc, EventHandle, 0, &ThreadHandle3);
    TF_Printf("Executive_CreateThread(Thread_Three_Proc, &EventHandle, 0, &ThreadHandle3)  result: %x \n", result);

    Executive_Sleep(100);

    result = Executive_SetEvent(EventHandle); //Should trigger one threads
    TF_Printf("Executive_SetEvent(EventHandle)  result: %x \n", result);

    Executive_Sleep(100);

    result = Executive_SetEvent(EventHandle); //Should trigger the second
    TF_Printf("Executive_SetEvent(EventHandle)  result: %x \n", result);

    Executive_Sleep(100);

    result = Executive_SetEvent(EventHandle); //Should trigger the last
    TF_Printf("Executive_SetEvent(EventHandle)  result: %x \n", result);

    Executive_Sleep(100);

    result = Executive_CloseEvent(EventHandle);
    TF_Printf("Executive_CloseEvent(EventHandle)  result: %x \n", result);
    if (pkFAILED(result))
    {
        casesFailed++;
    }
    else
    {
        casesPassed++;
    }


    TF_Printf("*************************************\n");
    TF_Printf("Executive Unit Test: Basic Event Test Results\n");
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

pkRESULT ExecTest_EventExtendedTest(void)
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
static uint32_t pkAPI Thread_One_Proc(void *pParam)
{
    pkHANDLE EventHandle = NULL;

    pkRESULT result = pkE_FAIL;

    if(NULL != pParam)
    {
        EventHandle = (pkHANDLE )pParam;

        result = Executive_WaitForEvent(EventHandle, EXEC_WAIT_INFINITE);
        TF_Printf("Thread 1: WaitForEvent result: %x \n", result);
    }

    return 0;
}

static uint32_t pkAPI Thread_Two_Proc(void *pParam)
{
    pkHANDLE EventHandle = NULL;
    pkRESULT result = pkE_FAIL;

    if(NULL != pParam)
    {
        EventHandle = (pkHANDLE )pParam;

        result = Executive_WaitForEvent(EventHandle, EXEC_WAIT_INFINITE);
        TF_Printf("Thread 2: WaitForEvent result: %x \n", result);
    }

    return 0;
}

static uint32_t pkAPI Thread_Three_Proc(void *pParam)
{
    pkHANDLE EventHandle = NULL;
    pkRESULT result = pkE_FAIL;

    if(NULL != pParam)
    {
        EventHandle = (pkHANDLE )pParam;

        result = Executive_WaitForEvent(EventHandle, EXEC_WAIT_INFINITE);
        TF_Printf("Thread 3: WaitForEvent result: %x \n", result);
    }

    return 0;
}


