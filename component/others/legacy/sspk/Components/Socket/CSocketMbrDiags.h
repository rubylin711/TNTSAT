///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CSocketDiags.h"
#include "OttStatistics.h"
#include <strsafe.h>

// ===============================================================================================================
// MBR socket update events
// ==============================================================================================================

class CDiagsSocketMbrUpdateEvent : public CDiagsSocketUpdateEvent
{
public:
    CDiagsSocketMbrUpdateEvent()
        : CDiagsSocketUpdateEvent()
    {}
    virtual ~CDiagsSocketMbrUpdateEvent() {}

public:
    __override void DiagsGetEventData(void);
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_SS_Update; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return CDiagsSocketUpdateEvent::DiagsSerializeEventDataLength(version) + sizeof(CMbrDiagnosticStats);
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        CDiagsSocketUpdateEvent::DiagsSerializeEventData(data);

        DiagsPackUint32(data, Stats.RetryCount);
        DiagsPackUint32(data, Stats.TotalQualityLevel);
        DiagsPackUint32(data, Stats.CurrentQualityLevel);
        DiagsPackUint32(data, Stats.TotalBps);
        DiagsPackUint32(data, Stats.MinVideoBps);
        DiagsPackUint32(data, Stats.MaxVideoBps);
        DiagsPackUint32(data, Stats.CurrentVideoBps);
        DiagsPackUint64(data, Stats.LastQualityChangeTs);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        CDiagsSocketUpdateEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackUint32(data, Stats.RetryCount);
        DiagsUnpackUint32(data, Stats.TotalQualityLevel);
        DiagsUnpackUint32(data, Stats.CurrentQualityLevel);
        DiagsUnpackUint32(data, Stats.TotalBps);
        DiagsUnpackUint32(data, Stats.MinVideoBps);
        DiagsUnpackUint32(data, Stats.MaxVideoBps);
        DiagsUnpackUint32(data, Stats.CurrentVideoBps);
        DiagsUnpackUint64(data, Stats.LastQualityChangeTs);
        return true;
    }

public:
    CMbrDiagnosticStats Stats;
};

// ===============================================================================================================
// SmoothStreaming socket events
// ===============================================================================================================

class CDiagsSSQualityChange : public CDiagsSocketEvent
{
public:
    CDiagsSSQualityChange()
        : CDiagsSocketEvent()
        , MediaType(0)
        , StreamId(0)
        , QualityLevel(0)
        , CurBitRate(0)
        , ThreadCpu(0)
        , TotalCpu(0)
    {}
    CDiagsSSQualityChange(uint32 mediaType, uint32 streamId, uint32 qualityLevel, uint32 bitRate, uint32 threadCpu, uint32 totalCpu)
        : CDiagsSocketEvent()
        , MediaType(mediaType)
        , StreamId(streamId)
        , QualityLevel(qualityLevel)
        , CurBitRate(bitRate)
        , ThreadCpu(threadCpu)
        , TotalCpu(totalCpu)
    {}
    virtual ~CDiagsSSQualityChange() {}

public:
    __override void DiagsGetEventData(void);
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.SS.QualityChange"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_SS_QualityChange; }
    __override bool DiagsIsError(void) { return false; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return CDiagsSocketEvent::DiagsSerializeEventDataLength(version) + 6*4;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        CDiagsSocketEvent::DiagsSerializeEventData(data);

        DiagsPackUint32(data, MediaType);
        DiagsPackUint32(data, StreamId);
        DiagsPackUint32(data, QualityLevel);
        DiagsPackUint32(data, CurBitRate);
        DiagsPackUint32(data, ThreadCpu);
        DiagsPackUint32(data, TotalCpu);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        CDiagsSocketEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackUint32(data, MediaType);
        DiagsUnpackUint32(data, StreamId);
        DiagsUnpackUint32(data, QualityLevel);
        DiagsUnpackUint32(data, CurBitRate);
        DiagsUnpackUint32(data, ThreadCpu);
        DiagsUnpackUint32(data, TotalCpu);
        return true;
    }

public:
    uint32 MediaType;
    uint32 StreamId;
    uint32 QualityLevel;
    uint32 CurBitRate;
    uint32 ThreadCpu;
    uint32 TotalCpu;
};

// ==============================================================================================================
// ==============================================================================================================

class CDiagsSSStreamInfo : public CDiagsSocketEvent
{
public:
    CDiagsSSStreamInfo()
        : CDiagsSocketEvent()
        , MediaType(0)
        , StreamId(0)
        , QualityLevelCount(0)
        , CodecFourCC(0)
        , ChunkCount(0)
        , FirstChunkIndex(0)
        , LastChunkIndex(0)
        , TunedToChunkIndex(0)
    {}
    CDiagsSSStreamInfo(uint32 mediaType, uint32 streamId, uint32 qualityLevelCount, uint32 codecFourCC, uint32 chunkCount, uint32 firstChunkIndex, uint32 lastChunkIndex, uint32 tunedToChunkIndex)
        : CDiagsSocketEvent()
        , MediaType(mediaType)
        , StreamId(streamId)
        , QualityLevelCount(qualityLevelCount)
        , CodecFourCC(codecFourCC)
        , ChunkCount(chunkCount)
        , FirstChunkIndex(firstChunkIndex)
        , LastChunkIndex(lastChunkIndex)
        , TunedToChunkIndex(tunedToChunkIndex)
    {}
    virtual ~CDiagsSSStreamInfo() {}

public:
    __override void DiagsGetEventData(void);
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.SS.StreamInfo"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_SS_StreamInfo; }
    __override bool DiagsIsError(void) { return false; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return CDiagsSocketEvent::DiagsSerializeEventDataLength(version) + 8*4;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        CDiagsSocketEvent::DiagsSerializeEventData(data);

        DiagsPackUint32(data, MediaType);
        DiagsPackUint32(data, StreamId);
        DiagsPackUint32(data, QualityLevelCount);
        DiagsPackUint32(data, CodecFourCC);
        DiagsPackUint32(data, ChunkCount);
        DiagsPackUint32(data, FirstChunkIndex);
        DiagsPackUint32(data, LastChunkIndex);
        DiagsPackUint32(data, TunedToChunkIndex);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        CDiagsSocketEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackUint32(data, MediaType);
        DiagsUnpackUint32(data, StreamId);
        DiagsUnpackUint32(data, QualityLevelCount);
        DiagsUnpackUint32(data, CodecFourCC);
        DiagsUnpackUint32(data, ChunkCount);
        DiagsUnpackUint32(data, FirstChunkIndex);
        DiagsUnpackUint32(data, LastChunkIndex);
        DiagsUnpackUint32(data, TunedToChunkIndex);
        return true;
    }

public:
    uint32 MediaType;
    uint32 StreamId;
    uint32 QualityLevelCount;
    uint32 CodecFourCC;
    uint32 ChunkCount;
    uint32 FirstChunkIndex;
    uint32 LastChunkIndex;
    uint32 TunedToChunkIndex;
};

// ==============================================================================================================
// ==============================================================================================================

class CDiagsSSSocketInfo : public CDiagsSocketEvent
{
public:
    CDiagsSSSocketInfo()
        : CDiagsSocketEvent()
        , StreamCount(0)
        , CpuLimit(0)
        , ChunkBufferLengthInSecond(0)
        , IsLive(false)
    {
    }
    CDiagsSSSocketInfo(uint32 streamCount, uint32 cpuLimit, uint32 maxBitrate, uint32 chunkBufferLengthInSecond, bool isLive)
        : CDiagsSocketEvent()
        , StreamCount(streamCount)
        , CpuLimit(cpuLimit)
        , ChunkBufferLengthInSecond(chunkBufferLengthInSecond)
        , IsLive(isLive)
    {
        BitRate = maxBitrate;
    }
    virtual ~CDiagsSSSocketInfo() {}

public:
    __override void DiagsGetEventData(void);
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.SS.SocketInfo"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_SS_SocketInfo; }
    __override bool DiagsIsError(void) { return false; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return CDiagsSocketEvent::DiagsSerializeEventDataLength(version) + 3*4 + 1;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        CDiagsSocketEvent::DiagsSerializeEventData(data);

        DiagsPackUint32(data, StreamCount);
        DiagsPackUint32(data, CpuLimit);
        DiagsPackUint32(data, ChunkBufferLengthInSecond);
        DiagsPackBool(data, IsLive);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        CDiagsSocketEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackUint32(data, StreamCount);
        DiagsUnpackUint32(data, CpuLimit);
        DiagsUnpackUint32(data, ChunkBufferLengthInSecond);
        DiagsUnpackBool(data, IsLive);
        return true;
    }

public:
    uint32 StreamCount;
    uint32 CpuLimit;
    uint32 ChunkBufferLengthInSecond;
    bool IsLive;
};

// ==============================================================================================================
// ==============================================================================================================

class CDiagsSSChunkInfo : public CDiagsSocketEvent
{
public:
    CDiagsSSChunkInfo()
        : CDiagsSocketEvent()
        , StreamID(0)
        , ChunkIndex(0)
        , RequestToChunkComplete(0)
        , ToResponseHeader(0)
        , ToResponseData(0)
        , ToPayload(0)
        , TimeToDownloadChunk(0)
        , QualityLevel(0)
        , RateControlSleep(0)
        , ProcessedBytes(0)
        , RemainingBytes(0)
    {
    }
    CDiagsSSChunkInfo(uint32 streamID, uint32 chunkIndex, uint32 time1, uint32 time2, uint32 time3, uint32 time4, uint32 time5, uint32 qualityLevel, uint32 rateControlSleep, uint32 processedBytes, uint32 remainingBytes)
        : CDiagsSocketEvent()
        , StreamID(streamID)
        , ChunkIndex(chunkIndex)
        , RequestToChunkComplete(time1)
        , ToResponseHeader(time2)
        , ToResponseData(time3)
        , ToPayload(time4)
        , TimeToDownloadChunk(time5)
        , QualityLevel(qualityLevel)
        , RateControlSleep(rateControlSleep)
        , ProcessedBytes(processedBytes)
        , RemainingBytes(remainingBytes)
    {
    }
    virtual ~CDiagsSSChunkInfo() {}

public:
    __override void DiagsGetEventData(void);
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.SS.ChunkInfo"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_SS_ChunkInfo; }
    __override bool DiagsIsError(void) { return false; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return CDiagsSocketEvent::DiagsSerializeEventDataLength(version) + 44;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        CDiagsSocketEvent::DiagsSerializeEventData(data);

        DiagsPackUint32(data, StreamID);
        DiagsPackUint32(data, ChunkIndex);
        DiagsPackUint32(data, RequestToChunkComplete);
        DiagsPackUint32(data, ToResponseHeader);
        DiagsPackUint32(data, ToResponseData);
        DiagsPackUint32(data, ToPayload);
        DiagsPackUint32(data, TimeToDownloadChunk);
        DiagsPackUint32(data, QualityLevel);
        DiagsPackUint32(data, RateControlSleep);
        DiagsPackUint32(data, ProcessedBytes);
        DiagsPackUint32(data, RemainingBytes);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        CDiagsSocketEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackUint32(data, StreamID);
        DiagsUnpackUint32(data, ChunkIndex);
        DiagsUnpackUint32(data, RequestToChunkComplete);
        DiagsUnpackUint32(data, ToResponseHeader);
        DiagsUnpackUint32(data, ToResponseData);
        DiagsUnpackUint32(data, ToPayload);
        DiagsUnpackUint32(data, TimeToDownloadChunk);
        DiagsUnpackUint32(data, QualityLevel);
        DiagsUnpackUint32(data, RateControlSleep);
        DiagsUnpackUint32(data, ProcessedBytes);
        DiagsUnpackUint32(data, RemainingBytes);
        return true;
    }

public:
    uint32 StreamID;
    uint32 ChunkIndex;                //index of the chunk
    uint32 RequestToChunkComplete;    //1. MS elapsed from sending out request referenced by <ChunkIndex+1> to finishing reading of chunk referenced by <ChunkIndex>
    uint32 ToResponseHeader;        //2. MS elapsed from end of 1 to start of reading the http response header for <ChunkIndex+1>
    uint32 ToResponseData;            //3. MS elapsed from end of 2 to start of reading the http response data (f-mp4 header) for <ChunkIndex+1>
    uint32 ToPayload;                //4. MS elapsed from end of 3 to start of reading the payload data for <ChunkIndex+1>
    uint32 TimeToDownloadChunk;        //5. MS spent for downloading the chunk referenced by <ChunkIndex>
    uint32 QualityLevel;            //quality level of <ChunkIndex>
    uint32 RateControlSleep;         //MS slept by rate control. this is reset on each quality level change.
    uint32 ProcessedBytes;            //number of bytes of the chunk <ChunkIndex> processed when sending out request for chunk <ChunkIndex+1>
    uint32 RemainingBytes;            //number of bytes of the chunk <ChunkIndex> remaining when sending out request for chunk <ChunkIndex+1>
};

// ===============================================================================================================
// ===============================================================================================================
