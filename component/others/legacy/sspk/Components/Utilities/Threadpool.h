///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "SyncQueue.h"
#include "Thread.h"

//    Base class for poolable work items
class WorkItem {
public:
    virtual void Work() = 0;
    virtual ~WorkItem() {};
};

class ThreadPool {
public:
            ThreadPool(int count, int priority = 0);
            ~ThreadPool();

    void    QueueWorkItem(IRunnable *workItem);

    struct RunningQueue // for internal use
    {
        bool        _running;
        SyncQueue    _queue;
    };

private:
    ThreadPool(const ThreadPool &);
    ThreadPool &operator=(const ThreadPool &);

    int            _count;
    HANDLE*        _threads;

    RunningQueue _runningQueue;
};
