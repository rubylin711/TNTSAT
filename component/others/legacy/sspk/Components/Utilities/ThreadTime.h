///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// ====================================================================================================================
// Measuring Thread CPU usage
// ====================================================================================================================

class ThreadTime
{
public:
    ThreadTime():CheckPeriod(15000){}

    void        Init(void);
    bool        Collect(void);
    void        SetCheckPeriod(DWORD period){CheckPeriod = period;}

private:
    HANDLE        ThreadHandle;
    DWORD        LastChecked;
    DWORD        CheckPeriod;

private:
    DWORD         StartTime;
    LARGE_INTEGER TimeBase;
    LARGE_INTEGER LastNumerator;
    DWORD         LastDenominator;
public:
    DWORD         Last;
    DWORD         Average;
    DWORD         Maximum;

private:
    DWORD         TotalLastTicks;
    DWORD         TotalLastUsed;
public:
    DWORD         TotalUsed;
};

// ====================================================================================================================
// ====================================================================================================================
