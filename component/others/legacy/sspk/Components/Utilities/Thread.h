///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


#pragma once
#include "AutoLock.h"
//    Don't subclass thread class, use IRunnable instead
class IRunnable;
class Thread
{
public:
        Thread();
virtual    ~Thread();

        void Start(IRunnable* runnable);
        void Stop();
        void Join();
        DWORD JoinTimeout;

private:
        HANDLE    _thread;
        IRunnable* _runnable;
        BOOL _bRunning;
        Lockable _lock;
};


class IRunnable
{
public:
virtual    ~IRunnable();
virtual    void OnThreadRun();        // Called when thread runs
virtual void OnThreadStop();    // Called to terminate thread
};
