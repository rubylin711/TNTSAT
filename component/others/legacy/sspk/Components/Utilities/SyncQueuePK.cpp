///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "AutoLock.h"
#include "pkExecutive.h"

#include "SyncQueue.h"
#include "Utils.h"

// An arbitrary large number
#define MAX_QUEUE_ENTRIES        0x7fffffff

SyncQueue::SyncQueue()
{
    _bClosing = false;
    pkRESULT pkResult = Executive_CreateSemaphore
    (
        NULL,                 // const char *pszIgnored,
        0,                     // int32_t initCount,
        MAX_QUEUE_ENTRIES,     // int32_t maxCount,
        &_semaphore         // pkHANDLE *pHandle
    );

    (void)pkResult; // prevent unused warning in release build
    ASSERT(pkSUCCEEDED(pkResult));
}

SyncQueue::~SyncQueue()
{
    _bClosing = true;
    _queue.empty();

    int32_t prevCount = 0;
    pkRESULT pkResult;

    do // Release and close semaphore first to free up any threads waiting on semaphore
    {
        pkResult = Executive_ReleaseSemaphore(_semaphore, &prevCount);
        if (pkFAILED(pkResult))
        {
            break;
        }
    } while (prevCount <= 1);

    Executive_CloseSemaphore(_semaphore);
}

void SyncQueue::Enqueue(void* data)
{
    if (_bClosing)
        return;

    _queueLock.Lock();
    _queue.push_back(data);
    CE_AV_SYNCQUEUE_LOG(CE_AV_SYNCQUEUE_ENQUEUE, _queue.size());
    _queueLock.Unlock();
    Executive_ReleaseSemaphore(_semaphore, NULL);
}

void* SyncQueue::Dequeue(int timeout)
{
    if (_bClosing)
    {
        return NULL;
    }

    pkRESULT pkResult = Executive_WaitForSemaphore(_semaphore, timeout);

    if (_bClosing || pkResult != pkS_OK) //pkResult == pkS_FALSE on timeout
    {
        return NULL;
    }

    void* data = NULL;

    _queueLock.Lock();

    if (_queue.size() == 0)
    {
        // This should not happen unless the semaphore has been closed and
        // we are shutting down
        CE_AV_SYNCQUEUE_LOG(CE_AV_SYNCQUEUE_EMPTY, 0);
    }
    else
    {
        data = _queue.front();
        _queue.pop_front();
        CE_AV_SYNCQUEUE_LOG(CE_AV_SYNCQUEUE_DEQUEUE, _queue.size());
    }

    _queueLock.Unlock();
    return data;
}

size_t  SyncQueue::size()
{
    size_t result;

    _queueLock.Lock();
    result = _queue.size();
    _queueLock.Unlock();

    return result;
}
