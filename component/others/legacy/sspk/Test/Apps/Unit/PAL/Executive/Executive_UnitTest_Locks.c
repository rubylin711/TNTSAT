///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//
// Executive_UnitTest_Locks.c: Defines the entry point for the sample Media Center Extender Platform.
// Unit tests for the Executive PAL Lock implementation.
//
/* Test the following APIs:

pkRESULT pkAPI Executive_CreateLock(
                pkHANDLE *pHandle);

pkRESULT pkAPI Executive_EnterLock(
                pkHANDLE hHandle);

pkRESULT pkAPI Executive_ExitLock(
                pkHANDLE hHandle);

pkRESULT pkAPI Executive_DeleteLock(
                pkHANDLE hHandle);

bool_t pkAPI Executive_IsLockedByCurrentThread(
                pkHANDLE hHandle);

pkRESULT pkAPI Executive_TryEnterLock(
                pkHANDLE hHandle);

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
pkRESULT ExecTest_LockBasicTest(void)
{
    pkRESULT retval = pkE_FAIL;
    uint32_t casesPassed = 0, casesFailed = 0;

    pkHANDLE LockHandle = NULL;
    pkHANDLE ThreadHandle1 = NULL;
    pkHANDLE ThreadHandle2 = NULL;
    pkHANDLE ThreadHandle3 = NULL;

    TF_Printf("Main thread: Create lock.\n");
    Executive_CreateLock(&LockHandle);
    TF_Printf("Main thread: Try to acquire the lock.\n");
    Executive_TryEnterLock(LockHandle);

    TF_Printf("Main thread: Check if the lock is currently owned by this thread.\n");
    if(Executive_IsLockedByCurrentThread(LockHandle))
    {
        TF_Printf("Main thread: Acquired the lock at tick %u.\n", Executive_GetTickCount());
        Executive_ExitLock(LockHandle);
    }
    else
    {
        TF_Printf("Executive Unit Test: TEST FAIL - Expected Main thread to own the lock, but it didn't.\n");
        casesFailed++;
        goto exit;
    }


    Executive_CreateThread(Thread_One_Proc, LockHandle, 0, &ThreadHandle1);
    TF_Printf("Main thread: Created Thread 1 Handle: 0x%x\n", ThreadHandle1);
    Executive_CreateThread(Thread_Two_Proc, LockHandle, 0, &ThreadHandle2);
    TF_Printf("Main thread: Created Thread 2 Handle: 0x%x\n", ThreadHandle2);
    Executive_CreateThread(Thread_Three_Proc, LockHandle, 0, &ThreadHandle3);
    TF_Printf("Main thread: Created Thread 3 Handle: 0x%x\n", ThreadHandle3);

    Executive_Sleep(150);

    TF_Printf("Main Thread: Try to acquire the lock.\n");
    Executive_EnterLock(LockHandle);
    TF_Printf("Main thread: Acquired the lock at tick %u.\n", Executive_GetTickCount());
    casesPassed++;

exit:
    Executive_ExitLock(LockHandle);
    Executive_DeleteLock(LockHandle);
    TF_Printf("Main thread: Deleted lock\n");

    TF_Printf("*************************************\n");
    TF_Printf("Executive Unit Test: Extended Lock Test Results\n");
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

pkRESULT ExecTest_LockExtendedTest(void)
{
    uint32_t casesPassed, casesFailed;
    pkRESULT retval;

    casesPassed = casesFailed = 0;
    retval = pkE_FAIL;

    TF_Printf("*************************************\n");
    TF_Printf("Executive Unit Test: Extended Lock Test Results\n");
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
    pkHANDLE handle = NULL;

    if(NULL != pParam)
    {
        handle = (pkHANDLE)pParam;

        Executive_Sleep(10);
        TF_Printf("Thread 1: Try to acquire the lock.\n");
        Executive_EnterLock(handle);
        TF_Printf("Thread 1: Acquired the lock at tick %u.\n", Executive_GetTickCount());
        Executive_Sleep(30);
        TF_Printf("Thread 1: Exit lock.\n");
        Executive_ExitLock(handle);
    }

    return 0;
}

static uint32_t pkAPI Thread_Two_Proc(void *pParam)
{
    pkHANDLE * handle = NULL;

    if(NULL != pParam)
    {
        handle = (pkHANDLE)pParam;

        Executive_Sleep(20);
        TF_Printf("Thread 2: Try to acquire the lock.\n");
        Executive_EnterLock(handle);
        TF_Printf("Thread 2: Acquired the lock at tick %u.\n", Executive_GetTickCount());
        Executive_Sleep(50);
        TF_Printf("Thread 2: Exit lock.\n");
        Executive_ExitLock(handle);
    }

    return 0;
}

static uint32_t pkAPI Thread_Three_Proc(void *pParam)
{
    pkHANDLE * handle = NULL;

    if(NULL != pParam)
    {
        handle = (pkHANDLE)pParam;

        Executive_Sleep(30);
        TF_Printf("Thread 3: Try to acquire the lock.\n");
        while (Executive_TryEnterLock(handle) != pkS_OK)
        {
            TF_Printf("Thread 3: Try to exit lock. This should fail since this thread doesn't own this lock yet!\n");
            Executive_ExitLock(handle);
            Executive_Sleep(5);
            TF_Printf("Thread 3: Try to acquire the lock.\n");
        }
        TF_Printf("Thread 3: Acquired the lock at tick %u.\n", Executive_GetTickCount());
        Executive_Sleep(10);
        TF_Printf("Thread 3: Exit lock.\n");
        Executive_ExitLock(handle);
    }

    return 0;
}




