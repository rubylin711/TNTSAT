///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ThreadTime.h"
#include "pkExecutive.h"

// ====================================================================================================================
// Measuring Thread CPU usage
// ====================================================================================================================

void ThreadTime::Init(void)
{
    ThreadHandle = GetCurrentThread();
    LastChecked = 0;
    StartTime = 0;
    TimeBase.QuadPart = 0;
    LastNumerator.QuadPart = 0;
    LastDenominator = 0;
    Last = 0;
    Average = 0;
    Maximum = 0;
    TotalLastTicks = 0;
    TotalLastUsed = 0;
    TotalUsed = 0;
}

bool ThreadTime::Collect(void)
{
    //First check if it is time to check CPU usage
    DWORD currentTime = Executive_GetTickCount();
    if ((currentTime - LastChecked) < CheckPeriod)
        return false;
    LastChecked = currentTime;

    //No handle no CPU time checking
    if (ThreadHandle == NULL)
        return false;

    //Calculate CPU usage for this decoder thread
    LARGE_INTEGER liThreadTime;
    {
        FILETIME creationTime, exitTime, kernelTime, userTime;
        if (!GetThreadTimes(ThreadHandle, &creationTime, &exitTime, &kernelTime, &userTime))
            return false;

        LARGE_INTEGER liUserTime;
        liUserTime.u.HighPart = userTime.dwHighDateTime;
        liUserTime.u.LowPart = userTime.dwLowDateTime;

        LARGE_INTEGER liKernelTime;
        liKernelTime.u.HighPart = kernelTime.dwHighDateTime;
        liKernelTime.u.LowPart = kernelTime.dwLowDateTime;

        liThreadTime.QuadPart = liUserTime.QuadPart + liKernelTime.QuadPart;
    }

    //Get current CPU idle time

    //Calculate the average CPU usage of the decoder thread so far
    if ((StartTime == 0) || (currentTime <= StartTime))
    {
        StartTime = currentTime;
        TimeBase.QuadPart = liThreadTime.QuadPart;
    }
    else
    {
        Average = (DWORD)((liThreadTime.QuadPart - TimeBase.QuadPart) / (ULONGLONG)(currentTime - StartTime));
    }

    //Calculate CPU usage since last time this method was called
    if ((LastDenominator > 0) && (currentTime != LastDenominator))
    {
        Last = (DWORD)((liThreadTime.QuadPart - LastNumerator.QuadPart) / (ULONGLONG)(currentTime - LastDenominator));
    }
    else
    {
        Last = 0;
    }

    LastDenominator = currentTime;
    LastNumerator.QuadPart = liThreadTime.QuadPart;

    //Now save the maximum time taken between different calls to this method
    if (Last > Maximum) Maximum = Last;
    return true;
}

// ====================================================================================================================
// ====================================================================================================================
