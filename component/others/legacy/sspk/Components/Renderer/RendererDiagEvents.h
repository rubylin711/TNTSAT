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
//      kDiagsEvent_AddVideoSample
//      kDiagsEvent_AddAudioSample
//      kDiagsEvent_LimitVideoFifo
//
// =======================================================================================

// =======================================================================================
//
// Macros to report the events
//
// =======================================================================================

#define ReportEvent_AddVideoSample( instance, pipeIdN, stc, pts, fTrick ) \
    {   bool fChannelIsOn = DiagChannelEnabled(Heuristics,Low); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_AddVideoSample( fChannelIsOn, instance, pipeIdN, stc, pts, fTrick ); } }

#define ReportEvent_AddAudioSample( instance, pipeIdN, stc, pts ) \
    {   bool fChannelIsOn = DiagChannelEnabled(Heuristics,Low); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_AddAudioSample( fChannelIsOn, instance, pipeIdN, stc, pts ); } }

#define ReportEvent_LimitVideoFifo( instance, pipeIdN, level ) \
    {   bool fChannelIsOn = DiagChannelEnabled(Heuristics,Low); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_LimitVideoFifo( fChannelIsOn, instance, pipeIdN, level ); } }


// =======================================================================================
//
// Functions used by the macros to report the events
// (do not call directly, use the macros)
//
// =======================================================================================

//
// Event AddVideoSample
inline void Event_AddVideoSample(
        bool fChannelIsOn,
        const void* instance,
        uint32 pipeIdN,
        int64 stc,             // in 90khz scale
        int64 pts,             // in 90khz scale
        int32 fTrick
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[5];
        SetRecord( rg[0], L"instance", &instance, sizeof(instance), eDiagRecordPtr );
        SetRecord( rg[1], L"pipeIdN", &pipeIdN, sizeof(pipeIdN), eDiagRecordUInt32 );
        SetRecord( rg[2], L"stc", &stc, sizeof(stc), eDiagRecordInt64 );
        SetRecord( rg[3], L"pts", &pts, sizeof(pts), eDiagRecordInt64 );
        SetRecord( rg[4], L"fTrick", &fTrick, sizeof(fTrick), eDiagRecordInt32 );
        ReportDiagEvent( kDiagsEvent_AddVideoSample, L"AddVideoSample", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_AddVideoSample %p, %u, %lld, %lld, %d", \
            instance, pipeIdN, stc, pts, fTrick )); \
}

//
// Event AddAudioSample
inline void Event_AddAudioSample(
        bool fChannelIsOn,
        const void* instance,
        uint32 pipeIdN,
        int64 stc,             // in 90khz scale
        int64 pts              // in 90khz scale
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[4];
        SetRecord( rg[0], L"instance", &instance, sizeof(instance), eDiagRecordPtr );
        SetRecord( rg[1], L"pipeIdN", &pipeIdN, sizeof(pipeIdN), eDiagRecordUInt32 );
        SetRecord( rg[2], L"stc", &stc, sizeof(stc), eDiagRecordInt64 );
        SetRecord( rg[3], L"pts", &pts, sizeof(pts), eDiagRecordInt64 );
        ReportDiagEvent( kDiagsEvent_AddAudioSample, L"AddAudioSample", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_AddAudioSample %p, %u, %lld, %lld", \
            instance, pipeIdN, stc, pts )); \
}

//
// Event LimitVideoFifo
inline void Event_LimitVideoFifo(
        bool fChannelIsOn,
        const void* instance,
        uint32 pipeIdN,
        int64 level            // in 90khz scale
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[3];
        SetRecord( rg[0], L"instance", &instance, sizeof(instance), eDiagRecordPtr );
        SetRecord( rg[1], L"pipeIdN", &pipeIdN, sizeof(pipeIdN), eDiagRecordUInt32 );
        SetRecord( rg[2], L"level", &level, sizeof(level), eDiagRecordInt64 );
        ReportDiagEvent( kDiagsEvent_LimitVideoFifo, L"LimitVideoFifo", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_LimitVideoFifo %p, %u, %lld", \
            instance, pipeIdN, level )); \
}

