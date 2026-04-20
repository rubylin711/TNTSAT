///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CSocketMbrDiags.h"
#include "Trace.h"

// ===============================================================================================================
// MBR socket update events
// ==============================================================================================================

void CDiagsSocketMbrUpdateEvent::DiagsGetEventData(void)
{
    //Base class stuff
    CDiagsSocketUpdateEvent::DiagsGetEventData();

    DiagsLogValue(L"RetryCount", Stats.RetryCount);
    DiagsLogValue(L"TotalQualityLevel", Stats.TotalQualityLevel);
    DiagsLogValue(L"CurrentQualityLevel", Stats.CurrentQualityLevel);
    DiagsLogValue(L"TotalBps", Stats.TotalBps);
    DiagsLogValue(L"MinVideoBps", Stats.MinVideoBps);
    DiagsLogValue(L"MaxVideoBps", Stats.MaxVideoBps);
    DiagsLogValue(L"CurrentVideoBps", Stats.CurrentVideoBps);
    DiagsLogValue64(L"LastQualityChangeTs", Stats.LastQualityChangeTs);
}

// ===============================================================================================================
// SmoothStreaming socket events
// ===============================================================================================================

void CDiagsSSQualityChange::DiagsGetEventData(void)
{
    //Base class stuff so far
    CDiagsSocketEvent::DiagsGetEventData();

    DiagsLogValue(L"MediaType", MediaType);
    DiagsLogValue(L"StreamId", StreamId);
    DiagsLogValue(L"QualityLevel", QualityLevel);
    DiagsLogValue(L"CurrentBitRate", CurBitRate);
    DiagsLogValue(L"ThreadCpu", ThreadCpu);
    DiagsLogValue(L"TotalCpu", TotalCpu);
}

// ==============================================================================================================
// ==============================================================================================================

void CDiagsSSStreamInfo::DiagsGetEventData(void)
{
    //Base class stuff so far
    CDiagsSocketEvent::DiagsGetEventData();

    DiagsLogValue(L"MediaType", MediaType);
    DiagsLogValue(L"StreamId", StreamId);
    DiagsLogValue(L"QualityLevelCount", QualityLevelCount);
    DiagsLogValue(L"CodecFourCC", CodecFourCC, true);
    DiagsLogValue(L"ChunkCount", ChunkCount);
    DiagsLogValue(L"FirstChunkIndex", FirstChunkIndex);
    DiagsLogValue(L"LastChunkIndex", LastChunkIndex);
    DiagsLogValue(L"TunedToChunkIndex", TunedToChunkIndex);
}

// ==============================================================================================================
// ==============================================================================================================

void CDiagsSSSocketInfo::DiagsGetEventData(void)
{
    //Base class stuff so far
    CDiagsSocketEvent::DiagsGetEventData();

    DiagsLogValue(L"StreamCount", StreamCount);
    DiagsLogValue(L"CpuLimit", CpuLimit);
    DiagsLogValue(L"ChunkBufferLengthInSecond", ChunkBufferLengthInSecond);
    DiagsLogValue(L"IsLive", IsLive);
}

// ===============================================================================================================
// ===============================================================================================================
void CDiagsSSChunkInfo::DiagsGetEventData(void)
{
    //Base class stuff so far
    CDiagsSocketEvent::DiagsGetEventData();

    DiagsLogValue(L"StreamID", StreamID);
    DiagsLogValue(L"ChunkIndex", ChunkIndex);
    DiagsLogValue(L"RequestToChunkComplete", RequestToChunkComplete);
    DiagsLogValue(L"ToResponseHeader", ToResponseHeader);
    DiagsLogValue(L"ToResponseData", ToResponseData);
    DiagsLogValue(L"ToPayload", ToPayload);
    DiagsLogValue(L"TimeToDownloadChunk", TimeToDownloadChunk);
    DiagsLogValue(L"QualityLevel", QualityLevel);
    DiagsLogValue(L"RateControlSleep", RateControlSleep);
    DiagsLogValue(L"ProcessedBytes", ProcessedBytes);
    DiagsLogValue(L"RemainingBytes", RemainingBytes);
}

// ===============================================================================================================
// ===============================================================================================================

