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
//      kDiagChannel_ChunkList
//      kDiagChannel_FragInfo
//      kDiagChannel_Heuristics
//
// Diagnostic events used in this header (make sure they are defined in IDiagsEvent.h):
//
//      kDiagsEvent_CreateChunkDownloader
//      kDiagsEvent_DestroyChunkDownloader
//      kDiagsEvent_StartChunkRequest
//      kDiagsEvent_StartChunkResponseHeader
//      kDiagsEvent_StartChunkResponseBody
//      kDiagsEvent_EndChunkRequest
//      kDiagsEvent_UpdateHeuristics
//      kDiagsEvent_StartFragInfoRequest
//      kDiagsEvent_AddChunk
//
// =======================================================================================

// =======================================================================================
//
// Macros to report the events
//
// =======================================================================================

#define ReportEvent_CreateChunkDownloader( instance, streamType, SID, streamName, rateControlInstance ) \
    {   bool fChannelIsOn = DiagChannelEnabled(Heuristics,High); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_CreateChunkDownloader( fChannelIsOn, instance, streamType, SID, streamName, rateControlInstance ); } }

#define ReportEvent_DestroyChunkDownloader( instance ) \
    {   bool fChannelIsOn = DiagChannelEnabled(Heuristics,High); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_DestroyChunkDownloader( fChannelIsOn, instance ); } }

#define ReportEvent_StartChunkRequest( instance, SID, chunkIndex, qualityLevelIndex, qualityLevelBitrate, chunkSize, chunkUrl ) \
    {   bool fChannelIsOn = DiagChannelEnabled(Heuristics,High); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_StartChunkRequest( fChannelIsOn, instance, SID, chunkIndex, qualityLevelIndex, qualityLevelBitrate, chunkSize, chunkUrl ); } }

#define ReportEvent_StartChunkResponseHeader( instance, SID, chunkIndex ) \
    {   bool fChannelIsOn = DiagChannelEnabled(Heuristics,High); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_StartChunkResponseHeader( fChannelIsOn, instance, SID, chunkIndex ); } }

#define ReportEvent_StartChunkResponseBody( instance, SID, chunkIndex ) \
    {   bool fChannelIsOn = DiagChannelEnabled(Heuristics,High); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_StartChunkResponseBody( fChannelIsOn, instance, SID, chunkIndex ); } }

#define ReportEvent_EndChunkRequest( instance, SID, chunkIndex ) \
    {   bool fChannelIsOn = DiagChannelEnabled(Heuristics,High); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_EndChunkRequest( fChannelIsOn, instance, SID, chunkIndex ); } }

#define ReportEvent_UpdateHeuristics( instance, SID, chunkIndex, sampledBitrate, averageBitrate, videoBufferLevel, audioBufferLevel ) \
    {   bool fChannelIsOn = DiagChannelEnabled(Heuristics,High); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_UpdateHeuristics( fChannelIsOn, instance, SID, chunkIndex, sampledBitrate, averageBitrate, videoBufferLevel, audioBufferLevel ); } }

#define ReportEvent_StartFragInfoRequest( SID, chunkIndex, qualityLevelIndex, qualityLevelBitrate, chunkUrl ) \
    {   bool fChannelIsOn = DiagChannelEnabled(FragInfo,High); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_StartFragInfoRequest( fChannelIsOn, SID, chunkIndex, qualityLevelIndex, qualityLevelBitrate, chunkUrl ); } }

#define ReportEvent_AddChunk( SID, timestamp ) \
    {   bool fChannelIsOn = DiagChannelEnabled(ChunkList,High); \
        if( CDIAGS_TRACE_IS_ON || fChannelIsOn ) { \
            Event_AddChunk( fChannelIsOn, SID, timestamp ); } }


// =======================================================================================
//
// Functions used by the macros to report the events
// (do not call directly, use the macros)
//
// =======================================================================================

//
// Event CreateChunkDownloader
inline void Event_CreateChunkDownloader(
        bool fChannelIsOn,
        const void* instance,
        const wchar_t* streamType,
        uint32 SID,
        const wchar_t* streamName,
        const void* rateControlInstance   // instance of rate control used by this instance
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[5];
        SetRecord( rg[0], L"instance", &instance, sizeof(instance), eDiagRecordPtr );
        SetRecord( rg[1], L"streamType", streamType, ( wcslen( streamType ) + 1 ) * sizeof(wchar_t), eDiagRecordWStr );
        SetRecord( rg[2], L"SID", &SID, sizeof(SID), eDiagRecordUInt32 );
        SetRecord( rg[3], L"streamName", streamName, ( wcslen( streamName ) + 1 ) * sizeof(wchar_t), eDiagRecordWStr );
        SetRecord( rg[4], L"rateControlInstance", &rateControlInstance, sizeof(rateControlInstance), eDiagRecordPtr );
        ReportDiagEvent( kDiagsEvent_CreateChunkDownloader, L"CreateChunkDownloader", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_CreateChunkDownloader %p, '%ls', %u, '%ls', %p", \
            instance, streamType, SID, streamName, rateControlInstance )); \
}

//
// Event DestroyChunkDownloader
inline void Event_DestroyChunkDownloader(
        bool fChannelIsOn,
        const void* instance
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[1];
        SetRecord( rg[0], L"instance", &instance, sizeof(instance), eDiagRecordPtr );
        ReportDiagEvent( kDiagsEvent_DestroyChunkDownloader, L"DestroyChunkDownloader", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_DestroyChunkDownloader %p", \
            instance )); \
}

//
// Event StartChunkRequest
inline void Event_StartChunkRequest(
        bool fChannelIsOn,
        const void* instance,
        uint32 SID,
        int32 chunkIndex,
        int32 qualityLevelIndex,
        int32 qualityLevelBitrate,        // bps
        uint32 chunkSize,                 // KB
        const wchar_t* chunkUrl
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[7];
        SetRecord( rg[0], L"instance", &instance, sizeof(instance), eDiagRecordPtr );
        SetRecord( rg[1], L"SID", &SID, sizeof(SID), eDiagRecordUInt32 );
        SetRecord( rg[2], L"chunkIndex", &chunkIndex, sizeof(chunkIndex), eDiagRecordInt32 );
        SetRecord( rg[3], L"qualityLevelIndex", &qualityLevelIndex, sizeof(qualityLevelIndex), eDiagRecordInt32 );
        SetRecord( rg[4], L"qualityLevelBitrate", &qualityLevelBitrate, sizeof(qualityLevelBitrate), eDiagRecordInt32 );
        SetRecord( rg[5], L"chunkSize", &chunkSize, sizeof(chunkSize), eDiagRecordUInt32 );
        SetRecord( rg[6], L"chunkUrl", chunkUrl, ( wcslen( chunkUrl ) + 1 ) * sizeof(wchar_t), eDiagRecordWStr );
        ReportDiagEvent( kDiagsEvent_StartChunkRequest, L"StartChunkRequest", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_StartChunkRequest %p, %u, %d, %d, %d, %u, '%ls'", \
            instance, SID, chunkIndex, qualityLevelIndex, qualityLevelBitrate, chunkSize, chunkUrl )); \
}

//
// Event StartChunkResponseHeader
inline void Event_StartChunkResponseHeader(
        bool fChannelIsOn,
        const void* instance,
        uint32 SID,
        int32 chunkIndex
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[3];
        SetRecord( rg[0], L"instance", &instance, sizeof(instance), eDiagRecordPtr );
        SetRecord( rg[1], L"SID", &SID, sizeof(SID), eDiagRecordUInt32 );
        SetRecord( rg[2], L"chunkIndex", &chunkIndex, sizeof(chunkIndex), eDiagRecordInt32 );
        ReportDiagEvent( kDiagsEvent_StartChunkResponseHeader, L"StartChunkResponseHeader", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_StartChunkResponseHeader %p, %u, %d", \
            instance, SID, chunkIndex )); \
}

//
// Event StartChunkResponseBody
inline void Event_StartChunkResponseBody(
        bool fChannelIsOn,
        const void* instance,
        uint32 SID,
        int32 chunkIndex
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[3];
        SetRecord( rg[0], L"instance", &instance, sizeof(instance), eDiagRecordPtr );
        SetRecord( rg[1], L"SID", &SID, sizeof(SID), eDiagRecordUInt32 );
        SetRecord( rg[2], L"chunkIndex", &chunkIndex, sizeof(chunkIndex), eDiagRecordInt32 );
        ReportDiagEvent( kDiagsEvent_StartChunkResponseBody, L"StartChunkResponseBody", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_StartChunkResponseBody %p, %u, %d", \
            instance, SID, chunkIndex )); \
}

//
// Event EndChunkRequest
inline void Event_EndChunkRequest(
        bool fChannelIsOn,
        const void* instance,
        uint32 SID,
        int32 chunkIndex
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[3];
        SetRecord( rg[0], L"instance", &instance, sizeof(instance), eDiagRecordPtr );
        SetRecord( rg[1], L"SID", &SID, sizeof(SID), eDiagRecordUInt32 );
        SetRecord( rg[2], L"chunkIndex", &chunkIndex, sizeof(chunkIndex), eDiagRecordInt32 );
        ReportDiagEvent( kDiagsEvent_EndChunkRequest, L"EndChunkRequest", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_EndChunkRequest %p, %u, %d", \
            instance, SID, chunkIndex )); \
}

//
// Event UpdateHeuristics
inline void Event_UpdateHeuristics(
        bool fChannelIsOn,
        const void* instance,
        uint32 SID,
        int32 chunkIndex,
        uint32 sampledBitrate,            // bps
        uint32 averageBitrate,            // bps
        int64 videoBufferLevel,           // 90khz
        int64 audioBufferLevel            // 90khz
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[7];
        SetRecord( rg[0], L"instance", &instance, sizeof(instance), eDiagRecordPtr );
        SetRecord( rg[1], L"SID", &SID, sizeof(SID), eDiagRecordUInt32 );
        SetRecord( rg[2], L"chunkIndex", &chunkIndex, sizeof(chunkIndex), eDiagRecordInt32 );
        SetRecord( rg[3], L"sampledBitrate", &sampledBitrate, sizeof(sampledBitrate), eDiagRecordUInt32 );
        SetRecord( rg[4], L"averageBitrate", &averageBitrate, sizeof(averageBitrate), eDiagRecordUInt32 );
        SetRecord( rg[5], L"videoBufferLevel", &videoBufferLevel, sizeof(videoBufferLevel), eDiagRecordInt64 );
        SetRecord( rg[6], L"audioBufferLevel", &audioBufferLevel, sizeof(audioBufferLevel), eDiagRecordInt64 );
        ReportDiagEvent( kDiagsEvent_UpdateHeuristics, L"UpdateHeuristics", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_UpdateHeuristics %p, %u, %d, %u, %u, %lld, %lld", \
            instance, SID, chunkIndex, sampledBitrate, averageBitrate, videoBufferLevel, audioBufferLevel )); \
}

//
// Event StartFragInfoRequest
inline void Event_StartFragInfoRequest(
        bool fChannelIsOn,
        uint32 SID,
        int32 chunkIndex,
        int32 qualityLevelIndex,
        int32 qualityLevelBitrate,        // bps
        const wchar_t* chunkUrl
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[5];
        SetRecord( rg[0], L"SID", &SID, sizeof(SID), eDiagRecordUInt32 );
        SetRecord( rg[1], L"chunkIndex", &chunkIndex, sizeof(chunkIndex), eDiagRecordInt32 );
        SetRecord( rg[2], L"qualityLevelIndex", &qualityLevelIndex, sizeof(qualityLevelIndex), eDiagRecordInt32 );
        SetRecord( rg[3], L"qualityLevelBitrate", &qualityLevelBitrate, sizeof(qualityLevelBitrate), eDiagRecordInt32 );
        SetRecord( rg[4], L"chunkUrl", chunkUrl, ( wcslen( chunkUrl ) + 1 ) * sizeof(wchar_t), eDiagRecordWStr );
        ReportDiagEvent( kDiagsEvent_StartFragInfoRequest, L"StartFragInfoRequest", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_StartFragInfoRequest %u, %d, %d, %d, '%ls'", \
            SID, chunkIndex, qualityLevelIndex, qualityLevelBitrate, chunkUrl )); \
}

//
// Event AddChunk
inline void Event_AddChunk(
        bool fChannelIsOn,
        uint32 SID,
        uint64 timestamp                  // 10MHz
        )
{
    if( fChannelIsOn )
    {
        DIAG_EVENT_RECORD rg[2];
        SetRecord( rg[0], L"SID", &SID, sizeof(SID), eDiagRecordUInt32 );
        SetRecord( rg[1], L"timestamp", &timestamp, sizeof(timestamp), eDiagRecordUInt64 );
        ReportDiagEvent( kDiagsEvent_AddChunk, L"AddChunk", rg, sizeof(rg)/sizeof(rg[0]) );
    }

    CDIAGS_TRACE(( "Event_AddChunk %u, %llu", \
            SID, timestamp )); \
}

