///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include <DiagReportBase.h>

#ifdef CDIAGS_TRACE_ENABLE
#define CDIAGS_TRACE(x)         TRACE(x)
#define CDIAGS_TRACE_IS_ON      true
#else
#define CDIAGS_TRACE(x)
#define CDIAGS_TRACE_IS_ON      false
#endif

// =======================================================================================
//
// Diagnostic channels used in this header (make sure they are defined in IDiagsManager.h):
//
//      kDiagChannel_Manifest
//
// Diagnostic events used in this header (make sure they are defined in IDiagsEvent.h):
//
//      kDiagsEvent_StreamInManifest
//
// =======================================================================================

// =======================================================================================
//
// Macros to report the events
//
// =======================================================================================

#define ReportEvent_StreamInManifest( SID, streamType, streamName ) \
    {   bool fChannelIsOn = DiagChannelEnabled(Manifest,High); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_StreamInManifest( fChannelIsOn, SID, streamType, streamName ); } }


// =======================================================================================
//
// Functions used by the macros to report the events
// (do not call directly, use the macros)
//
// =======================================================================================

//
// Event StreamInManifest
inline void Event_StreamInManifest(
        bool fChannelIsOn,
        uint32 SID,
        const wchar_t* streamType,
        const wchar_t* streamName
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[3];
        SetRecord( rg[0], L"SID", &SID, sizeof(SID), eDiagRecordUInt32 );
        SetRecord( rg[1], L"streamType", streamType, ( wcslen( streamType ) + 1 ) * sizeof(wchar_t), eDiagRecordWStr );
        SetRecord( rg[2], L"streamName", streamName, ( wcslen( streamName ) + 1 ) * sizeof(wchar_t), eDiagRecordWStr );
        ReportDiagEvent( kDiagsEvent_StreamInManifest, L"StreamInManifest", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_StreamInManifest %u, '%ls', '%ls'", \
            SID, streamType, streamName )); \
}

