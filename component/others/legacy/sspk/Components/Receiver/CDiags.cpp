///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CDiags.h"

//#define CDIAGS_TRACE_ENABLE
#ifdef CDIAGS_TRACE_ENABLE
#define CDIAGS_TRACE(x) TRACE(x)
#else
#define CDIAGS_TRACE(x)
#endif

// ===============================================================================================================
// Event used to post thread timings
// ===============================================================================================================

const WCHAR* CDiagsCpuUsageEvent::DiagsGetEventMessage(void)
{
    static const WCHAR* ThreadNameStrings[] =
    {
        L"AV.CpuUsage",
        L"AV.CpuUsage.DiagsThread",
        L"AV.CpuUsage.DvrWriterThread",
        L"AV.CpuUsage.DecoderThread",
    };
    return Name > ThreadName_Unknown && Name < ThreadName_Max ? ThreadNameStrings[Name] : ThreadNameStrings[ThreadName_Unknown];
}

void CDiagsCpuUsageEvent::DiagsGetEventData(void)
{
    if (Name > ThreadName_Unknown && Name < ThreadName_Max)
    {
        DiagsLogValue(L"Last", CpuUsageLast);
        DiagsLogValue(L"Average", CpuUsageAverage);
        DiagsLogValue(L"Maximum", CpuUsageMaximum);
    }
    DiagsLogValue(L"Total", CpuUsageTotalUsed);
}

// ===============================================================================================================
// UDP authentication errors
// ===============================================================================================================

void UdpAuthProtectionFailure::DiagsGetEventData(void)
{
    //Version
    DiagsLogValue(L"Version", Version);
    //Active key id
    DiagsLogGuid(L"ActiveKeyId", ActiveKeyId);
    //Active key length
    DiagsLogValue(L"ActiveKeyLength", ActiveKeyLength);
    //Error code returned
    DiagsLogValue(L"ErrorCode", ErrorCode);
}

void UdpAuthNoActiveKey::DiagsGetEventData(void)
{
    //Version
    DiagsLogValue(L"Version", Version);
    //Error Count
    DiagsLogValue64(L"Count", Count);
}

// ===============================================================================================================
// ===============================================================================================================
