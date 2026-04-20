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
//      kDiagChannel_Heuristics
//
// Diagnostic events used in this header (make sure they are defined in IDiagsEvent.h):
//
//      kDiagsEvent_RateMeasurement
//
// =======================================================================================

// =======================================================================================
//
// Macros to report the events
//
// =======================================================================================

#define ReportEvent_RateMeasurement( instance, byteCount, timeSpent, bitrate, timeToMeasure ) \
    {   bool fChannelIsOn = DiagChannelEnabled(Heuristics,Low); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_RateMeasurement( fChannelIsOn, instance, byteCount, timeSpent, bitrate, timeToMeasure ); } }


// =======================================================================================
//
// Functions used by the macros to report the events
// (do not call directly, use the macros)
//
// =======================================================================================

//
// Event RateMeasurement
inline void Event_RateMeasurement(
        bool fChannelIsOn,
        const void* instance,
        uint32 byteCount,      // bytes
        uint32 timeSpent,      // ms
        uint32 bitrate,        // bps
        uint32 timeToMeasure   // ms
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[5];
        SetRecord( rg[0], L"instance", &instance, sizeof(instance), eDiagRecordPtr );
        SetRecord( rg[1], L"byteCount", &byteCount, sizeof(byteCount), eDiagRecordUInt32 );
        SetRecord( rg[2], L"timeSpent", &timeSpent, sizeof(timeSpent), eDiagRecordUInt32 );
        SetRecord( rg[3], L"bitrate", &bitrate, sizeof(bitrate), eDiagRecordUInt32 );
        SetRecord( rg[4], L"timeToMeasure", &timeToMeasure, sizeof(timeToMeasure), eDiagRecordUInt32 );
        ReportDiagEvent( kDiagsEvent_RateMeasurement, L"RateMeasurement", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_RateMeasurement %p, %u, %u, %u, %u", \
            instance, byteCount, timeSpent, bitrate, timeToMeasure )); \
}

