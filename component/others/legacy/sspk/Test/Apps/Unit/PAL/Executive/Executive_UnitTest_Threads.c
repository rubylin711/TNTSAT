///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//
// Executive_UnitTest_Threads.c: Defines the entry point for the sample Media Center Extender Platform.
// Unit tests for the Executive PAL Threading implementation.
//
/* Test the following APIs:
pkRESULT pkAPI Executive_CreateThread(
            THREAD_ENTRY pfnThreadEntry,
            void *pParam,
            uint32_t stackSize,
            pkHANDLE *pHandle);

pkRESULT pkAPI Executive_CreateThreadEx(
            THREAD_ENTRY pfnThreadEntry,
            void *pParam,
            uint32_t stackSize,
            pkHANDLE *pHandle,
            THREAD_CATEGORY threadCategory);

pkRESULT pkAPI Executive_WaitForThread(
            pkHANDLE hHandle,
            uint32_t timeOutMs);

pkRESULT pkAPI Executive_TerminateThread(
            pkHANDLE hHandle,
            int32_t exitCode);

pkRESULT pkAPI Executive_CloseThread(
            pkHANDLE hHandle);

pkRESULT pkAPI Executive_GetCurrentThread(
            pkHANDLE *phHandle);

pkRESULT pkAPI Executive_GetCurrentThreadId(
            uint32_t *pThreadId);

pkRESULT pkAPI Executive_GetThreadId(
            pkHANDLE hHandle,
            uint32_t *pThreadId);

pkRESULT pkAPI Executive_GetThreadPriority(
            pkHANDLE hHandle,
            int *pPriority);

pkRESULT pkAPI Executive_SetThreadPriority(
            pkHANDLE hHandle,
            int priority);

*/
//=============================================================================
// I N C L U D E     F I L E S
//=============================================================================
#include "Executive_UnitTest.h"

//=============================================================================
// D E F I N I T I O N S
//=============================================================================
// define 4 threads per priority setting so that there are as many as there
// are CPUs on a 4 proc system so that we can properly "starve" lower priority
// threads
#define NUM_THREADS_PER_PRIORITY_SETTING    4
// x 2 because we are comparing one thread prioity level against another
#define NUM_THREADS                         (NUM_THREADS_PER_PRIORITY_SETTING*2)
#define THREAD_PRIORITY_RUN_TIME            2000 // run each test for two seconds

//=============================================================================
// G L O B A L    V A R I A B L E S
//=============================================================================
static int s_nLine = 0;

//=============================================================================
// L O C A L    F U N C T I O N    P R O T O T Y P E S
//=============================================================================
static uint32_t pkAPI Thread_One_Proc(void * pParam);
static uint32_t pkAPI Thread_Two_Proc(void * pParam);
static uint32_t pkAPI Thread_Three_Proc(void * pParam);
static bool_t Thread_RunAThread(int ThreadNum);
static uint32_t pkAPI ThreadPriorityTestProc(void *pParam);

//=============================================================================
// L O C A L   S T R U C T U R E S
//=============================================================================
typedef struct __tagThreadPriorityInfo
{
    pkHANDLE    hStartEvent;
    uint32_t    uiTimeToStop;
    uint32_t    uiCounter;
} ThreadPriorityInfo;


//=============================================================================
// L O C A L   E N U M S
//=============================================================================
// EThread_Priority_Tests is an enumerations that identifies each different
// thread priority test to run
typedef enum
{
    eThreadPriorityTest_HIGHESTvsTIME_CRITICAL,
    eThreadPriorityTest_HIGHvsHIGHEST,
    eThreadPriorityTest_NORMALvsHIGH,
    eThreadPriorityTest_LOWvsNORMAL,
    eThreadPriorityTest_LOWESTvsLOW,

    eThreadPriorityTest_TOP
} EThread_Priority_Tests;


//=============================================================================
// P U B L I C    F U N C T I O N    D E F I N I T I O N S
//=============================================================================
pkRESULT ExecTest_ThreadExtendedTest(void)
{
    uint32_t casesPassed = 0, casesFailed = 0;
    pkRESULT result = pkE_FAIL;
    pkRESULT hr; // for intermediate results checking

    // We are going to test the thread priority functionality to
    // make sure threads designated with a higher priority actually
    // get more CPU time
    //
    // We also will allocate FOUR threads for each priority in
    // order to make sure that the results are consistent even on a
    // four-processor machine

    ThreadPriorityInfo thread_info[NUM_THREADS];
    pkHANDLE hThreads[NUM_THREADS]={0};
    pkHANDLE hStartEvent=NULL;
    EThread_Priority_Tests eTest=eThreadPriorityTest_HIGHESTvsTIME_CRITICAL;

    hr=Executive_CreateEvent(NULL, TRUE, FALSE, &hStartEvent);

    if (pkFAILED(hr))
    {
        TF_Printf("*************************************\n");
        TF_Printf("Executive Unit Test: Extended Thread Test Error\n");
        TF_Printf("Failed to create a \"start\" event: 0x%08X\n", hr);
        TF_Printf("*************************************\n");
        // we have failed
        casesFailed++;
        goto exit;
    }

    // run through each test
    while (eTest != eThreadPriorityTest_TOP)
    {
        // the priorities to use in this test
        int iLoThreadPriority=0;
        int iHiThreadPriority=0;

        // the names of the priorities to use in this test
        const char *pszLoThreadPriority=NULL;
        const char *pszHiThreadPriority=NULL;

        // the total count from each thread running at the same
        // priority
        uint32_t uiCounterLo=0;
        uint32_t uiCounterHi=0;

        // thread counter variable
        int iThread=0;

        // determine when the threads should stop running
        uint32_t uiTimeToStop = Executive_GetTickCount() + THREAD_PRIORITY_RUN_TIME;

        // set up the variables used for a specific test
        switch (eTest)
        {
        case eThreadPriorityTest_HIGHESTvsTIME_CRITICAL:
            iLoThreadPriority=pkEXECUTIVE_THREAD_PRIORITY_HIGHEST;
            iHiThreadPriority=pkEXECUTIVE_THREAD_PRIORITY_TIME_CRITICAL;
            pszLoThreadPriority="HIGHEST";
            pszHiThreadPriority="TIME_CRITICAL";
            eTest=eThreadPriorityTest_HIGHvsHIGHEST;
            break;

        case eThreadPriorityTest_HIGHvsHIGHEST:
            iLoThreadPriority=pkEXECUTIVE_THREAD_PRIORITY_HIGH;
            iHiThreadPriority=pkEXECUTIVE_THREAD_PRIORITY_HIGHEST;
            pszLoThreadPriority="HIGH";
            pszHiThreadPriority="HIGHEST";
            eTest=eThreadPriorityTest_NORMALvsHIGH;
            break;

        case eThreadPriorityTest_NORMALvsHIGH:
            iLoThreadPriority=pkEXECUTIVE_THREAD_PRIORITY_NORMAL;
            iHiThreadPriority=pkEXECUTIVE_THREAD_PRIORITY_HIGH;
            pszLoThreadPriority="NORMAL";
            pszHiThreadPriority="HIGH";
            eTest=eThreadPriorityTest_LOWvsNORMAL;
            break;

        case eThreadPriorityTest_LOWvsNORMAL:
            iLoThreadPriority=pkEXECUTIVE_THREAD_PRIORITY_LOW;
            iHiThreadPriority=pkEXECUTIVE_THREAD_PRIORITY_NORMAL;
            pszLoThreadPriority="LOW";
            pszHiThreadPriority="NORMAL";
            eTest=eThreadPriorityTest_LOWESTvsLOW;
            break;

        case eThreadPriorityTest_LOWESTvsLOW:
            iLoThreadPriority=pkEXECUTIVE_THREAD_PRIORITY_LOWEST;
            iHiThreadPriority=pkEXECUTIVE_THREAD_PRIORITY_LOW;
            pszLoThreadPriority="LOWEST";
            pszHiThreadPriority="LOW";
            eTest=eThreadPriorityTest_TOP;
            break;
        }

        // make sure the event is not signalled so that
        // the threads don't do much
        Executive_ResetEvent(hStartEvent);

        // create the threads and set their priority
        for (iThread=0 ; iThread<NUM_THREADS ; iThread++)
        {
            // initialize the data the threads will use
            thread_info[iThread].hStartEvent    = hStartEvent;
            thread_info[iThread].uiCounter      = 0;
            thread_info[iThread].uiTimeToStop   = uiTimeToStop;

            // create the thread
            hr=Executive_CreateThread(ThreadPriorityTestProc, (void *)&thread_info[iThread], 0, &hThreads[iThread]);

            if (pkFAILED(hr))
            {
                TF_Printf("*************************************\n");
                TF_Printf("Executive Unit Test: Extended Thread Test Error\n");
                TF_Printf("Failed to create a thread: 0x%08X\n", hr);
                TF_Printf("*************************************\n");
                break;
            }

            // set the thread's priority based on the current iThread index
            hr=Executive_SetThreadPriority(hThreads[iThread], iThread < NUM_THREADS_PER_PRIORITY_SETTING ? iLoThreadPriority : iHiThreadPriority);
            if (pkFAILED(hr))
            {
                TF_Printf("*************************************\n");
                TF_Printf("Executive Unit Test: Extended Thread Test Error\n");
                TF_Printf("Failed to set thread priority: 0x%08X\n", hr);
                TF_Printf("*************************************\n");
                break;
            }
        }

        if (iThread<NUM_THREADS)
        {
            // something failed---clean up and return an error
            int i=0;
            for (i=0 ; i<iThread ; i++)
            {
                Executive_WaitForThread(hThreads[iThread], EXEC_WAIT_INFINITE);
                Executive_CloseThread(hThreads[iThread]);
                hThreads[iThread]=NULL;
            }

            Executive_CloseEvent(hStartEvent);
            hStartEvent=NULL;

            // we have failed
            casesFailed++;
            goto exit;
        }

        // okay, all the threads should now be created, initialized, running, and
        // waiting on the thread start event, so let's wake 'em all up and get going!
        Executive_SetEvent(hStartEvent);

        // now, wait for the threads to exit
        for (iThread=0 ; iThread<NUM_THREADS ; iThread++)
        {
            Executive_WaitForThread(hThreads[iThread], EXEC_WAIT_INFINITE);
            Executive_CloseThread(hThreads[iThread]);
            hThreads[iThread]=NULL;
        }

        // report the results
        TF_Printf("*************************************\n");
        TF_Printf("Executive Unit Test: Thread Priority Test\n");

        // now, tally the results from all priority-alike threads
        for (iThread=0 ; iThread<NUM_THREADS ; iThread++)
        {
            if (iThread < NUM_THREADS_PER_PRIORITY_SETTING)
            {
                uiCounterLo += thread_info[iThread].uiCounter;
            }
            else
            {
                uiCounterHi += thread_info[iThread].uiCounter;
            }
            TF_Printf("Thread %d count:        %8u\n", iThread, thread_info[iThread].uiCounter);
        }

        TF_Printf("-------------------------------------\n");
        TF_Printf("Priority %-13s %8u\n", pszLoThreadPriority, uiCounterLo);
        TF_Printf("Priority %-13s %8u\n", pszHiThreadPriority, uiCounterHi);

        // the lower priority count should be less than the hi priority count
        if (uiCounterLo < uiCounterHi)
        {
            casesPassed++;
            TF_Printf("TEST PASSED\n");
        }
        else
        {
            casesFailed++;
            TF_Printf("TEST FAILED: The lower priority thread shouldn't get more\n");
            TF_Printf("processing time than the higher priority thread\n");
        }
        TF_Printf("*************************************\n");
    }

    // close the event used to start the threads
    Executive_CloseEvent(hStartEvent);
    hStartEvent=NULL;

exit:
    TF_Printf("*************************************\n");
    TF_Printf("Executive Unit Test: Extended Thread Test Results\n");
    TF_Printf("Tests Passed: %d\n",casesPassed);
    TF_Printf("Tests Failed: %d\n",casesFailed);
    TF_Printf("*************************************\n");

    if ( 0 == casesFailed )
    {
        result = pkS_OK;
    }
    else
    {
        result = pkE_FAIL;
    }

    return result;
}

static uint32_t pkAPI ThreadPriorityTestProc(void *pParam)
{
    // the param is actually a ThreadPriorityInfo structure
    ThreadPriorityInfo *pi=(ThreadPriorityInfo *)pParam;

    // wait for the tests to be set up and ready to go
    Executive_WaitForEvent(pi->hStartEvent, EXEC_WAIT_INFINITE);

    Executive_Sleep(10); // cause threads to start from coming out of sleep

    // loop, incrementing the counter, as fast as the thread
    // scheduler will let us until it's time to stop
    while ((int32_t)(Executive_GetTickCount() - pi->uiTimeToStop) < 0 && !g_bIsAborted)
    {
        pi->uiCounter++;
    }

    return 0;
}



pkRESULT ExecTest_ThreadBasicTest(void)
{
    uint32_t casesPassed = 0;
    uint32_t casesFailed = 0;
    pkRESULT result = pkE_FAIL;


    TF_Printf("Threads_TestThreadFunctions++  Tick Count: %u \n",Executive_GetTickCount());

    s_nLine = 0;

    if(Thread_RunAThread(1))
    {
        TF_Printf("Thread 1 test finished at Tick Count: %u \n",Executive_GetTickCount());
    }
    else
    {
        TF_Printf("Thread 1 test FAILED at Tick Count: %u \n",Executive_GetTickCount());
    }

    Executive_Sleep(10);

    if(0 != s_nLine)
    {
        TF_Printf("Error in %s on line %d\n", __FILE__, s_nLine);
        s_nLine = 0;
        casesFailed++;
    }
    else
    {
        casesPassed++;
    }

    if (g_bIsAborted)
    {
        goto exit;
    }

    if(Thread_RunAThread(2))
    {
        TF_Printf("Thread 2 test finished at Tick Count: %u \n",Executive_GetTickCount());
    }
    else
    {
        TF_Printf("Thread 2 test FAILED at Tick Count: %u \n",Executive_GetTickCount());
    }

    Executive_Sleep(10);

    if(0 != s_nLine)
    {
        TF_Printf("Error in %s on line %d\n", __FILE__, s_nLine);
        s_nLine = 0;
        casesFailed++;
    }
    else
    {
        casesPassed++;
    }

    if (g_bIsAborted)
    {
        goto exit;
    }

    if(Thread_RunAThread(3))
    {
        TF_Printf("Thread 3 test finished at Tick Count: %u \n",Executive_GetTickCount());
    }
    else
    {
        TF_Printf("Thread 3 test FAILED at Tick Count: %u \n",Executive_GetTickCount());
    }

    Executive_Sleep(10);

    if(0 != s_nLine)
    {
        TF_Printf("Error in %s on line %d\n", __FILE__, s_nLine);
        s_nLine = 0;
        casesFailed++;
    }
    else
    {
        casesPassed++;
    }

exit:

    TF_Printf("Threads_TestThreadFunctions--  Tick Count: %u \n",Executive_GetTickCount());

    TF_Printf("*************************************\n");
    TF_Printf("Executive Unit Test: Basic Thread Test Results\n");
    TF_Printf("Tests Passed: %d\n",casesPassed);
    TF_Printf("Tests Failed: %d\n",casesFailed);
    TF_Printf("*************************************\n");

    if ( 0 == casesFailed )
    {
        result = pkS_OK;
    }
    else
    {
        result = pkE_FAIL;
    }

    return result;
}

//=============================================================================
// L O C A L    F U N C T I O N    D E F I N I T I O N S
//=============================================================================
static uint32_t pkAPI Thread_One_Proc(void * pParam)
{
    TF_Printf("%u: %s start\n", Executive_GetTickCount(), __FUNCTION__);

    // Exit immediately

    TF_Printf("%u: %s exit\n", Executive_GetTickCount(), __FUNCTION__);
    return 0;
}

#define MAX_THREAD_2_ITERATIONS 5

static uint32_t pkAPI Thread_Two_Proc(void * pParam)
{
    int count = 0;

    TF_Printf("%u: %s start\n", Executive_GetTickCount(), __FUNCTION__);

    for(count = 0; count < MAX_THREAD_2_ITERATIONS; count++)
    {
        TF_Printf("%u: %s loop iteration %d\n", Executive_GetTickCount(), __FUNCTION__, count + 1);
        Executive_Sleep(10); // 10 ms.per loop
    }

    TF_Printf("%u: %s sleeping for 10 seconds ...\n", Executive_GetTickCount(), __FUNCTION__);
    Executive_Sleep(10000); // 10 sec.

    TF_Printf("%u: %s exit\n", Executive_GetTickCount(), __FUNCTION__);
    return 0;
}

static uint32_t pkAPI Thread_Three_Proc(void * pParam)
{

    TF_Printf("%u: %s start\n", Executive_GetTickCount(), __FUNCTION__);

    //This threads purpose in life is to wait
    //for a while and then exit.  Used for
    //testing WaitForThread functionality.

    TF_Printf("%u: %s sleeping for 1 second ...\n", Executive_GetTickCount(), __FUNCTION__);
    Executive_Sleep(1000);
    TF_Printf("%u: %s exit\n", Executive_GetTickCount(), __FUNCTION__);
    return(0);
}

static bool_t Thread_RunAThread(int ThreadNum)
{
    pkRESULT result;
    bool_t retval = TRUE;

    switch(ThreadNum)
    {
        case 1:
        {
            pkHANDLE handle1;

            result = Executive_CreateThread(Thread_One_Proc, NULL, 0, &handle1);
            if(pkFAILED(result))
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_CreateThread 1 failed: %x\n",result);
                retval = FALSE;
                break;
            }
            TF_Printf("%u: Call Executive_CreateThread 1 succeeded\n", Executive_GetTickCount());
            Executive_Sleep(100); // let thread exit

            result = Executive_CloseThread(handle1);
            if(pkFAILED(result))
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_CloseThread 1 failed: %x\n",result);
                retval = FALSE;
                break;
            }
            TF_Printf("%u: Call Executive_CloseThread 1 succeeded\n", Executive_GetTickCount());
            break;
        }

        case 2:
        {
            pkHANDLE handle2;

            int priority;

            result = Executive_CreateThread(Thread_Two_Proc, NULL, 0, &handle2);
            if(pkFAILED(result))
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_CreateThread 2 failed: %x\n",result);
                retval = FALSE;
                break;
            }
            TF_Printf("%u: Call Executive_CreateThread 2 succeeded\n", Executive_GetTickCount());
            Executive_Sleep(5);

            result = Executive_SetThreadPriority(handle2, pkEXECUTIVE_THREAD_PRIORITY_LOW);
            if(pkFAILED(result))
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_SetThreadPriority (%d) on thread 2 failed: %x\n",
                    pkEXECUTIVE_THREAD_PRIORITY_LOW, result);
                retval = FALSE;
                goto close2;
            }
            TF_Printf("%u: Call Executive_SetThreadPriority on thread 2 succeeded.\n", Executive_GetTickCount());
            Executive_Sleep(20);

            result = Executive_GetThreadPriority(handle2, &priority);
            if(pkFAILED(result))
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_GetThreadPriority on thread 2 failed: %x\n",result);
                retval = FALSE;
                goto close2;
            }
            TF_Printf("%u: Call Executive_GetThreadPriority on thread 2 got value: %d\n", Executive_GetTickCount(), priority);

            if (pkEXECUTIVE_THREAD_PRIORITY_LOW != priority)
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_GetThreadPriority on thread 2 failed to get expected value: %d\n",
                    pkEXECUTIVE_THREAD_PRIORITY_LOW);
                retval = FALSE;
                goto close2;
            }

            // Wait long enough for iterations to complete but *not* for thread to exit
            Executive_Sleep(10 * (MAX_THREAD_2_ITERATIONS+1));

            result = Executive_TerminateThread(handle2, 1); // exit code 1
            if (pkE_NOTIMPL == result)
            {
                TF_Printf("%u: Call Executive_TerminateThread 2  NOT IMPLEMENTED.\n", Executive_GetTickCount());
            }
            else if (pkFAILED(result))
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_TerminateThread 2 failed: %x\n",result);
                retval = FALSE;
                goto close2;
            }
            else
            {
                TF_Printf("%u: Call Executive_TerminateThread 2 succeeded.\n", Executive_GetTickCount());
            }

        close2:

            result = Executive_WaitForThread(handle2, EXEC_WAIT_INFINITE);
            if(pkFAILED(result))
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_WaitForThread 2 exit failed: %x\n",result);
                retval = FALSE;
                // continue to CloseThread
            }
            result = Executive_CloseThread(handle2);
            if(pkFAILED(result))
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_CloseThread 2 exit failed: %x\n",result);
                retval = FALSE;
                break;
            }
            TF_Printf("%u: Call Executive_CloseThread 2 exit succeeded\n", Executive_GetTickCount());
            break;
        }

        case 3:
        {
            pkHANDLE handle3;

            result = Executive_CreateThread(Thread_Three_Proc, NULL, 0, &handle3);
            if(pkFAILED(result))
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_CreateThread 3 failed: %x\n",result);
                retval = FALSE;
                break;
            }
            TF_Printf("%u: Call Executive_CreateThread 3 succeeded\n", Executive_GetTickCount());
            Executive_Sleep(20);

            // This test should return pkS_FALSE code since the thread is still running at this time.

            TF_Printf("%u: Call Executive_WaitForThread 3 timeout 100 ...\n", Executive_GetTickCount());
            result = Executive_WaitForThread(handle3, 100);
            if(pkFAILED(result))
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_WaitForThread 3 timeout 100 failed: %x\n",result);
                retval = FALSE;
                goto close3;
            }

            if (pkS_FALSE != result)
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_WaitForThread 3 timeout 100 did not indicate timeout result: 0x%08x\n",result);
                retval = FALSE;
                goto close3;
            }
            TF_Printf("%u: Call Executive_WaitForThread 3 timeout 100 finished\n", Executive_GetTickCount());

            // This test should return a pkS_OK

            TF_Printf("%u: Call Executive_WaitForThread 3 timeout infinite ...\n", Executive_GetTickCount());
            result = Executive_WaitForThread(handle3, EXEC_WAIT_INFINITE);
            if(pkFAILED(result))
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_WaitForThread 3 timeout infinite failed: %x\n",result);
                retval = FALSE;
                goto close3;
            }

            if (pkS_OK != result)
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_WaitForThread 3 timeout infinite failed to indicate non-timeout result: 0x%08x\n",result);
                retval = FALSE;
                goto close3;
            }
            TF_Printf("%u: Call Executive_WaitForThread 3 finished\n", Executive_GetTickCount());

        close3:

            result = Executive_WaitForThread(handle3, EXEC_WAIT_INFINITE);
            if(pkFAILED(result))
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_WaitForThread 3 exit failed: %x\n",result);
                retval = FALSE;
                // continue to CloseThread
            }
            result = Executive_CloseThread(handle3);
            if(pkFAILED(result))
            {
                s_nLine = __LINE__;
                TF_Printf("Call Executive_CloseThread 3 exit failed: %x\n",result);
                retval = FALSE;
                break;
            }

            TF_Printf("%u: Call Executive_CloseThread 3 exit succeeded\n", Executive_GetTickCount());
            break;
        }

        default:
            s_nLine = __LINE__;
            TF_Printf("%s invalid thread number: %d\n", __FUNCTION__, ThreadNum);
            retval = FALSE;
            break;
    }

    return retval;
}






