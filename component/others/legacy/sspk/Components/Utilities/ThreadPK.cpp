///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "pkExecutive.h"
#include "Thread.h"

//=================================================================
//=================================================================

IRunnable::~IRunnable()
{
}

void IRunnable::OnThreadRun()
{
    ASSERT(false);
}

void IRunnable::OnThreadStop()
{
}

//=================================================================
//=================================================================

Thread::Thread()
    : JoinTimeout(EXEC_WAIT_INFINITE)
    , _thread(NULL)
    , _runnable(NULL)
    , _bRunning(FALSE)
{
}

Thread::~Thread()
{
    Stop();
}

uint32_t pkAPI s_ThreadFunc(LPVOID lpParam)
{
    IRunnable* pRunnable = (IRunnable*) lpParam;
    pRunnable->OnThreadRun();
    return 0;
}

void Thread::Start(IRunnable* pRunnable)
{
    pkRESULT pkResult;

    AutoLock lock(&_lock);

    ASSERT(pRunnable);
    ASSERT(_thread == NULL);
    if (_bRunning)
    {
        return;
    }

    pkResult = Executive_CreateThread
        (
            s_ThreadFunc, // THREAD_ENTRY,
            pRunnable,    // void *pParam,
            0,            // uint32_t stackSize,
            &_thread      // pkHANDLE *pHandle
        );

    if (pkFAILED(pkResult))
    {
        pkASSERT(false);
    }
    else
    {
        _bRunning = TRUE;
        _runnable = pRunnable;

        Executive_SetThreadPriority(_thread, pkEXECUTIVE_THREAD_PRIORITY_HIGH);
    }
}

void Thread::Stop()
{
    AutoLock lock(&_lock);

    if (_thread == NULL)
        return;

    if(_bRunning)
    {
        _bRunning = FALSE;    // Guard to keep _runnable->Stop() from going into infinite recursive loop

        ASSERT(_runnable);

        if(_runnable != 0)
        {
            _runnable->OnThreadStop();
        }
        Join();
    }

    Executive_CloseThread(_thread);
    _thread = NULL;
    _runnable = NULL;
}

void Thread::Join()
{
    if(_thread)
    {
        Executive_WaitForThread(_thread, JoinTimeout);
    }
}


