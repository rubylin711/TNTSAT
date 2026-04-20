///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IAVManager.h"
#include "ITimeslice.h"
#include "AutoLock.h"
#include "Thread.h"
#include "ThreadTime.h"
#include <list>

// ===============================================================================================================
// A thread to provide a time slice to all register objects
// ===============================================================================================================

class CTimesliceManager : public ITimesliceManager, public IRunnable
{
public:
    CTimesliceManager(IAVManager* avManager);
    virtual ~CTimesliceManager();

private:
    //ITimesliceManager APIs
    __override bool         Command(const std::string& command, const std::vector<std::string>& args);
    __override void         Register(ITimeslice* decoder);
    __override void         UnRegister(ITimeslice* decoder);

    //IRunnable APIs
    __override void         OnThreadRun();
    __override void         OnThreadStop();

private:
    void                    Tick(void);

private:
    //Factory of factories
    IAVManager*             mAVManager;

    Lockable                mTimesliceLock;

    DWORD                   mLastCoFreeUnusedTicks;

    bool                    mThreadStopped;
    Thread                  mThread;
    ThreadTime              mThreadTime;

    std::list<ITimeslice*>  mList;
};

// ===============================================================================================================
// ===============================================================================================================
