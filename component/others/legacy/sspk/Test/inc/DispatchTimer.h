///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "pkExecutive.h"
#include "CEvent.h"
#include "Thread.h"
#include "ThreadTime.h"
#include "AutoLock.h"
#include <list>

class DispatchTimer : public IRunnable
{
public:
    ~DispatchTimer();
    void Init(DWORD dwInterval );

    void SetInterval( DWORD dwInterval );

    void Start();
    void Stop();
    void OnThreadRun();
    void OnThreadStop(){}
    bool IsTimerActive();

protected:

    virtual void OnTick() = 0;

private:
    DWORD m_dwInterval;
    Thread* m_Thread;
    CEvent* m_tEvent;
    bool m_isActive;
};

