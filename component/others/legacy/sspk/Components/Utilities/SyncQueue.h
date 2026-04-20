///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include <list>
#include "AutoLock.h"

class SyncQueue {
public:
    SyncQueue();
    ~SyncQueue();

    void    Enqueue(void* data);
    void*    Dequeue(int timeout = INFINITE);
    size_t  size();

private:
    std::list<void*> _queue;
    Lockable _queueLock;
    HANDLE _semaphore;
    bool _bClosing;

    SyncQueue(const SyncQueue& q);
    SyncQueue &operator=(const SyncQueue& q);
};
