///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "pkExecutive.h"

#include "Threadpool.h"

//    BUGBUG Not very happy with this implementation
//    Really want something that will not generate lock convoys
//    An IOCP implementation would be nice but it won't run on CE

static uint32_t pkAPI s_ThreadFunc(LPVOID lpParam)
{
    ThreadPool::RunningQueue *pRunningQueue = (ThreadPool::RunningQueue *)lpParam;

    while (pRunningQueue->_running)
    {
        IRunnable* pWorkItem = static_cast<IRunnable*>(pRunningQueue->_queue.Dequeue());
        // Note: Dequeue blocks until there is something enqueued

        if(pWorkItem != NULL)
        {
            pWorkItem->OnThreadRun();
        }
    }
    return 0;
}

ThreadPool::ThreadPool(int count, int priority)
    : _count(count)
{
    pkRESULT pkResult;

    _threads = new HANDLE[_count];

    if (NULL == _threads)
    {
        //TRACE_ERROR(("ThreadPool failed to allocate array of %d handles", count));
        pkASSERT(FALSE);
        _count = 0;
        _runningQueue._running = false;
        return;
    }

    _runningQueue._running = true;

    for (int i=0; i < count; i++)
    {
        pkResult = Executive_CreateThread(s_ThreadFunc, &_runningQueue, 0, &_threads[i]);
        if (pkFAILED(pkResult))
        {
            //TRACE_ERROR(("ThreadPool CreateThread failed [0x%x] after %d created", pkResult, i));
            pkASSERT(FALSE);
            _count = i;
            break;
        }

        if( priority != 0 )
        {
            pkResult = Executive_SetThreadPriority(_threads[i], priority);
            if (pkFAILED(pkResult))
            {
                //TRACE_ERROR(("ThreadPool SetThreadPriority failed [0x%x] for thread %d", pkResult, i));
                pkASSERT(FALSE);
                _count = i;
            }
        }
    }
}

ThreadPool::~ThreadPool()
{
    pkRESULT pkResult;

    _runningQueue._running = false;

    // Queue up NULL items to force threads out of loop
    for (int i = 0; i < _count; i++)
    {
        _runningQueue._queue.Enqueue(NULL);
    }

    // Wait for the threads to exit and then close them
    for (int i = 0; i < _count; i++)
    {
        pkResult = Executive_WaitForThread(_threads[i], PK_THREAD_TIMEOUT);
        if (pkResult == pkS_FALSE)
        {
            //If we have to, force termination of thread else we'll crash on next loop of ThreadFunc
            pkASSERT(FALSE);
            pkResult = Executive_TerminateThread(_threads[i], 0);
        }
        Executive_CloseThread(_threads[i]);
    }

    delete [] _threads;
}

void ThreadPool::QueueWorkItem(IRunnable* workItem)
{
    _runningQueue._queue.Enqueue(workItem);
    // TODO: add dynamic pool sizing based on the count of outstanding queue items
    //TRACE_ERROR(("ThreadPool queue has %d items", _queue.size()));
}

