///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDiagsManager.h"
#include "CTimesliceManager.h"
#include "CDiags.h"
#include "Trace.h"
#include <list>
using namespace std;

//#define TIMESLICE_SPEW
#if defined(TIMESLICE_SPEW)
#define TIMESLICE_MSG(x) TRACE(x)
#else
#define TIMESLICE_MSG(x)
#endif

// ===============================================================================================================
// ===============================================================================================================

#define COFREEUNUSEDPERIOD  (5*60*1000) //Check every 5 minutes
#define CPUDIAGSPERIOD      (60*1000)   //Every 1 minute

// ===============================================================================================================
// ===============================================================================================================

CTimesliceManager::CTimesliceManager(IAVManager* avManager)
    : mAVManager(avManager)
{
    mLastCoFreeUnusedTicks = Executive_GetTickCount();
    mThreadStopped = false;
    mThread.Start(this);
}

CTimesliceManager::~CTimesliceManager()
{
    mThread.Stop();
    ASSERT(mThreadStopped);
}

bool CTimesliceManager::Command(const string& command, const vector<string>& args)
{
    //Number of arguments
    uint32 numargs = (uint32)args.size();

    //Periodic CPU usage sampling rate update to diagnostics
    if (command == "cpuupdateperiod")
    {
        if (numargs == 1)
        {
            AutoLock lock(&mTimesliceLock);
            uint32 value = atoi(args[0].c_str());
            mThreadTime.SetCheckPeriod(value);
            TRACE(("Changing periodic CPU diagnostics update time to [%d]", value));
        }
        return true;
    }

    return false;
}

void CTimesliceManager::Register(ITimeslice* decoder)
{
    AutoLock lock(&mTimesliceLock);
    mList.push_back(decoder);
}

void CTimesliceManager::UnRegister(ITimeslice* decoder)
{
    AutoLock lock(&mTimesliceLock);
    mList.remove(decoder);
}

void CTimesliceManager::Tick(void)
{
    AutoLock lock(&mTimesliceLock);
    for (list<ITimeslice*>::iterator it = mList.begin(); !mThreadStopped && it != mList.end(); ++it)
    {
        (*it)->OnTimeslice();
    }
}

void CTimesliceManager::OnThreadRun()
{
    //Initialize CPU time tracking
    mThreadTime.Init();
    //Check CPU Usage every 1 min
    mThreadTime.SetCheckPeriod(CPUDIAGSPERIOD);

    //The thread always runs to provide timeslices to subscribers
    while (!mThreadStopped)
    {
        //Timeslicing starts
        //DWORD timeSlicingStartsAtTicks = Executive_GetTickCount();

        //Lets do some work for all registered subscribers...
        if (mTimesliceLock.TryLock())
        {
            Tick();

            //Collect CPU diagnstics
            if (mThreadTime.Collect())
            {
                TIMESLICE_MSG(("ThreadTimes[CTimeslice][%d]: Last=%d, Average=%d, Maximum=%d", mThreadTime.TotalUsed, mThreadTime.Last, mThreadTime.Average, mThreadTime.Maximum));
                mAVManager->GetDiagsManager()->PostEvent(new CDiagsCpuUsageEvent(ThreadName_Unknown, mThreadTime.Last, mThreadTime.Average, mThreadTime.Maximum, mThreadTime.TotalUsed));
            }

            mTimesliceLock.Unlock();
        }

        //Timeslicing done
        DWORD timeSliceingDoneAtTicks = Executive_GetTickCount();

        //This needs to be called periodically to free up cached DLLs,
        //which will only unload after 10 minutes of not being used
        if (timeSliceingDoneAtTicks - mLastCoFreeUnusedTicks >= COFREEUNUSEDPERIOD)
        {
            mLastCoFreeUnusedTicks = timeSliceingDoneAtTicks;
        }

        //Sleep a little bit
        Executive_Sleep(33);
    }
}

void CTimesliceManager::OnThreadStop()
{
    mThreadStopped = true;
}

// ===============================================================================================================
// ===============================================================================================================
