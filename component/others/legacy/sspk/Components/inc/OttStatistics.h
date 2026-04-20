///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// ===============================================================================================================
// Discontinuity statistics for WMS/MBR
// ===============================================================================================================

class COttDiscontinuityStats
{
public:
    COttDiscontinuityStats()
        : AudioDiscontinuityCount(0)
        , VideoDiscontinuityCount(0)
        , LastReportTick(0)
        , CurTick(0)
    {}

    void Reset()
    {
        AudioDiscontinuityCount = 0;
        VideoDiscontinuityCount = 0;
        LastReportTick = CurTick;
    }

    // return true if we should fire the discontinuity report
    bool AddDiscontinuityCount(bool isVideo, uint32 curTick)
    {
        CurTick = curTick;

        if (isVideo)
            ++VideoDiscontinuityCount;
        else
            ++AudioDiscontinuityCount;

        return (CurTick - LastReportTick > cReportInternal);
    }

    uint32 GetTicks()
    {
        return LastReportTick ? (CurTick - LastReportTick) : 0;
    }

    // do not report discontinuity event more
    // frequently than once per second
    static const uint32 cReportInternal = 1000;

    uint32 AudioDiscontinuityCount;
    uint32 VideoDiscontinuityCount;
    uint32 LastReportTick;
    uint32 CurTick;
};

// ===============================================================================================================
// Diagnostics information for MBR
// ===============================================================================================================

class CMbrDiagnosticStats
{
public:
    CMbrDiagnosticStats()
    {
        Init();
    }

    void Init()
    {
        RetryCount = 0;
        TotalQualityLevel = 0;
        CurrentQualityLevel = 0;
        TotalBps = 0;
        MinVideoBps = 0;
        MaxVideoBps = 0;
        CurrentVideoBps = 0;
        LastQualityChangeTs = 0;
    }

public:
    uint32 RetryCount;
    uint32 TotalQualityLevel;
    uint32 CurrentQualityLevel;
    uint32 TotalBps;
    uint32 MinVideoBps;
    uint32 MaxVideoBps;
    uint32 CurrentVideoBps;
    uint64 LastQualityChangeTs; // in 100ns
};
