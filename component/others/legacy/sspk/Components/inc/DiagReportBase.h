///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

//
// DiagReportBase.h - internal header that defines base elements for reporting generic diagnostic events
//

#pragma once

#include <IDiagsManager.h>  // from inc
#include <IDiagsEvent.h>    // from Components/inc
//
// These strings are always passed by reference and assumed to
// be in the code segment of the program. Never pass a temporary
// string as a label.
//
typedef const wchar_t* DIAG_EVENT_LABEL;

//
// Types of records
//
enum DiagRecordType
{
    eDiagRecordPtr,
    eDiagRecordInt32,
    eDiagRecordUInt32,
    eDiagRecordInt64,
    eDiagRecordUInt64,
    eDiagRecordStr,
    eDiagRecordWStr,
};

//
// The diagnostic record defines a field of a diagnostic event
//
struct DIAG_EVENT_RECORD
{
    DIAG_EVENT_LABEL pszLabel;
    const void* pData;
    size_t cbData;
    DiagRecordType type;
};

inline void SetRecord(
    _Inout_ DIAG_EVENT_RECORD& r,
    _In_ DIAG_EVENT_LABEL pszLabel,
    _In_bytecount_(cb) const void* p,
    _In_ size_t cb,
    _In_ DiagRecordType type )
{
    r.pszLabel = pszLabel;
    r.pData = p;
    r.cbData = cb;
    r.type = type;
}

//
// Array of flags that tell if a given diagnostic channel is disabled
//

extern int g_rgDiagChannelPriorities[ kDiagChannel_CountOf ];

#define DiagChannelEnabled(Name,Priority)   ( g_rgDiagChannelPriorities[ kDiagChannel_##Name ] >= kDiagsPriority_##Priority )


//
// Function to report a generic event
//

extern void ReportDiagEvent(
    _In_ kDiagsEvent eEventType,
    _In_ DIAG_EVENT_LABEL pszEventLabel,
    _In_count_(cRecords) const DIAG_EVENT_RECORD* prgRecords,
    _In_ size_t cRecords );
