///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//
// Executive_UnitTest_Semaphores.c: Defines the entry point for the sample Media Center Extender Platform.
// Unit tests for the Executive PAL Semaphore implementation.
//
/* Test the following APIs:
pkRESULT pkAPI Executive_CreateSemaphore(
            LPCTSTR *pszName,
            int32_t initCount,
            int32_t maxCount,
            pkHANDLE *pHandle);

pkRESULT pkAPI Executive_OpenSemaphore(
            LPCTSTR pszName,
            pkHANDLE *pHandle);

pkRESULT pkAPI Executive_ReleaseSemaphore(
            pkHANDLE hHandle,
            int32_t *pPrevCount);

pkRESULT pkAPI Executive_WaitForSemaphore(
            pkHANDLE hHandle,
            uint32_t timeOutMs);

pkRESULT pkAPI Executive_CloseSemaphore(
            SPalHANDLE hHandle);



*/
//=============================================================================
// I N C L U D E     F I L E S
//=============================================================================
#include "Executive_UnitTest.h"


//=============================================================================
// L O C A L    F U N C T I O N    P R O T O T Y P E S
//=============================================================================
static uint32_t pkAPI Thread_One_Proc(void * pParam);
static uint32_t pkAPI Thread_Two_Proc(void * pParam);
static uint32_t pkAPI Thread_Three_Proc(void * pParam);

//=============================================================================
// P U B L I C    F U N C T I O N    D E F I N I T I O N S
//=============================================================================
pkRESULT ExecTest_SemaphoreExtendedTest(void)
{
    uint32_t casesPassed = 0, casesFailed = 0;
    pkRESULT retval = pkE_FAIL;

    TF_Printf("*************************************\n");
    TF_Printf("Executive Unit Test: Extended Semaphore Test Results\n");
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

pkRESULT ExecTest_SemaphoreBasicTest(void)
{
    uint32_t casesPassed = 0, casesFailed = 0;
    pkRESULT retval = pkE_FAIL, result = pkE_FAIL;
    pkHANDLE SemaphoreHandle = NULL;
    pkHANDLE ThreadHandle1 = NULL;
    pkHANDLE ThreadHandle2 = NULL;
    pkHANDLE ThreadHandle3 = NULL;
    int32_t prevCount = 0;

    TF_Printf("Starting Semaphore_TestSemaphoreFunctions \n");

    result = Executive_CreateSemaphore(NULL, 1, 1, &SemaphoreHandle);
    TF_Printf("Executive_CreateSemaphore() 0x%x  result: %x \n", SemaphoreHandle, result);
    if (pkFAILED(result))
    {
        casesFailed++;
    }
    else
    {
        casesPassed++;
    }

    result = Executive_WaitForSemaphore(SemaphoreHandle, 0);
    TF_Printf("Executive_WaitForSemaphore(0x%x)  result: %x \n", SemaphoreHandle, result);
    if (pkFAILED(result))
    {
        casesFailed++;
    }
    else
    {
        casesPassed++;
    }

    result = Executive_CreateThread(Thread_One_Proc, SemaphoreHandle, 0, &ThreadHandle1);
    TF_Printf("Executive_CreateThread(Thread_One_Proc, &SemaphoreHandle, 0, &ThreadHandle1)  result: %x \n", result);

    result = Executive_CreateThread(Thread_Two_Proc, SemaphoreHandle, 0, &ThreadHandle2);
    TF_Printf("Executive_CreateThread(Thread_Two_Proc, &SemaphoreHandle, 0, &ThreadHandle2)  result: %x \n", result);

    result = Executive_CreateThread(Thread_Three_Proc, SemaphoreHandle, 0, &ThreadHandle3);
    TF_Printf("Executive_CreateThread(Thread_Three_Proc, &SemaphoreHandle, 0, &ThreadHandle3)  result: %x \n", result);

    Executive_Sleep(100);

    result = Executive_ReleaseSemaphore(SemaphoreHandle, &prevCount);
    TF_Printf("Executive_ReleaseSemaphore(SemaphoreHandle) prevCount: %x  result: %x \n",prevCount, result);

    Executive_Sleep(100);

    result = Executive_WaitForSemaphore(SemaphoreHandle, 50);
    TF_Printf("Executive_WaitForSemaphore(SemaphoreHandle, 50)  result: %x \n", result);
    if (pkFAILED(result))
    {
        casesFailed++;
    }
    else
    {
        casesPassed++;
    }

    result = Executive_CloseSemaphore(SemaphoreHandle);
    TF_Printf("Executive_CloseSemaphore(SemaphoreHandle)  result: %x \n", result);
    if (pkFAILED(result))
    {
        casesFailed++;
    }
    else
    {
        casesPassed++;
    }

    TF_Printf("*************************************\n");
    TF_Printf("Executive Unit Test: Basic Semaphore Test Results\n");
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
    pkHANDLE SemaphoreHandle = NULL;
    int32_t prevCount = 0;
    pkRESULT result = pkE_FAIL;

    if(NULL != pParam)
    {
        SemaphoreHandle = (pkHANDLE)pParam;

        result = Executive_WaitForSemaphore(SemaphoreHandle, EXEC_WAIT_INFINITE);
        TF_Printf("Thread 1: WaitForSemaphore(0x%x) result: %x \n", SemaphoreHandle, result);

        result = Executive_ReleaseSemaphore(SemaphoreHandle, &prevCount);
        TF_Printf("Thread 1: ReleaseSemaphore prevCount: %x result: %x \n",prevCount, result);
    }

    return 0;
}

static uint32_t pkAPI Thread_Two_Proc(void *pParam)
{
    pkHANDLE SemaphoreHandle = NULL;
    int32_t prevCount = 0;
    pkRESULT result = pkE_FAIL;

    if(NULL != pParam)
    {
        SemaphoreHandle = (pkHANDLE)pParam;

        result = Executive_WaitForSemaphore(SemaphoreHandle, EXEC_WAIT_INFINITE);
        TF_Printf("Thread 2: WaitForSemaphore(0x%x) result: %x \n", SemaphoreHandle, result);

        result = Executive_ReleaseSemaphore(SemaphoreHandle, &prevCount);
        TF_Printf("Thread 2: ReleaseSemaphore prevCount: %x result: %x \n",prevCount, result);
    }

    return 0;
}

static uint32_t pkAPI Thread_Three_Proc(void *pParam)
{
    pkHANDLE SemaphoreHandle = NULL;
    int32_t prevCount = 0;
    pkRESULT result = pkE_FAIL;

    if(NULL != pParam)
    {
        SemaphoreHandle = (pkHANDLE)pParam;

        result = Executive_WaitForSemaphore(SemaphoreHandle, EXEC_WAIT_INFINITE);
        TF_Printf("Thread 3: WaitForSemaphore(0x%x) result: %x \n", SemaphoreHandle, result);

        result = Executive_ReleaseSemaphore(SemaphoreHandle, &prevCount);
        TF_Printf("Thread 3: ReleaseSemaphore prevCount: %x  result: %x \n",prevCount, result);
    }

    return 0;
}

