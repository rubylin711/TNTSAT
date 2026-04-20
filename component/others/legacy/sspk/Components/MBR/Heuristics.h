///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/*
 * Heuristics.h
 *
 * This file describes the functionality of the Heuristics module
 */

#pragma once
#include "DecoderBufferStatus.h"
#include "ManifestParser.h"
#include "ThreadTime.h"

namespace MBR
{
#define    CFG_MaxBandwidth                (100*1000*1000)  // bits per second
#define    CFG_LowThreshold                5                // Seconds
#define    CFG_HighThreshold               15               // Seconds
#define    CFG_LookForwardDuration         120              // Seconds
#define    CFG_LookUpFactor                0.8              // limit to 80% target bitrate
#define    CFG_TrickModeBitrateFactor      ( 1 / 1.3 )      // limit the trick mode bitrate
#define    CFG_EnableMBRSwitch             true             // do mbr switching
#define    CFG_CpuCheckPeriod              6000             // minimum period between collection of CPU data

#define UNKNOWNBANDWIDTH                0
#define TARGETQUALITYACCURACY           1                   // when binary search target quality, stop when quality difference reaches this target
#define MAXHISTORYCOUNT                 5

class CHeuristicsMBR
{
    // simpler moving average
    struct CBandwidthHistory
    {
    public:
        CBandwidthHistory()
        {
            Clear();
        }

        pkRESULT Add( uint32 dwBandwidth )
        {
            _last = (_last + 1) % MAXHISTORYCOUNT;
            _sum -= _entry[_last];
            _sum += dwBandwidth;
            _entry[_last] = dwBandwidth;
            if (_count < MAXHISTORYCOUNT) _count++;
            return pkS_OK;
        }

        uint32 GetAverageBandwidth() const
        {
            return _count  ? (_sum / _count) : UNKNOWNBANDWIDTH;
        }

        uint32 GetAverageBandwidthCount() const
        {
            return _count;
        }

        uint32 GetLatestBandwidth() const
        {
            return _count ? _entry[_last] : UNKNOWNBANDWIDTH;
        }

        void Clear()
        {
            memset(_entry, 0, sizeof(_entry));
            _sum = _last = _count = 0;
        }

        void ClearTillLastOne()
        {
            if( _count > 0 )
            {
                _sum = _entry[_last];
                memset(_entry, 0, sizeof(_entry));
                _entry[_last] = _sum;
                _count = 1;
            }
        }

    private:
        uint32 _entry[MAXHISTORYCOUNT];
        uint32 _sum;
        uint8  _last;
        uint8  _count;
    };

public:
    CHeuristicsMBR( CChunkManifest *pManifest );
    virtual ~CHeuristicsMBR();

    pkRESULT  Init(bool isTrick, uint32 pctLimit);

    pkRESULT  GetNextChunk( _In_ uint32 dwStreamId,
                            _In_ uint32 dwChunkIndex,
                            _In_ uint32 dwCurrentBitrateIndex,
                            _Out_ uint32* pdwBitrateIndex );

    bool      ForceQualityNextChunk( _In_ uint32 streamId, _In_ uint32 dwCurrentBitrateIndex, _In_ EQualityDirection direction );
    pkRESULT  AddBandwidth( _In_ uint32 dwBandwidthBitrate );
    void      UpdateBufferStatus( _In_ const DecoderBufferStatus& decoderBufferStatus );

    uint32    GetAverageBandwidth() const { return m_BWHistory.GetAverageBandwidth(); }
    uint32    GetAverageBandwidthCount() const { return m_BWHistory.GetAverageBandwidthCount(); }
    uint32    GetLatestBandwidth() const { return m_BWHistory.GetLatestBandwidth(); }
    uint32    GetThreadCpu() { return m_pctCpuLast; }
    uint32    GetTotalCpu() { return m_pctCpuTotal; }

private:

    pkRESULT GetLeakyBucket(
        int32 chunkIndex,
        uint32 dwBandwidth,
        uint32 dwQuality,
        int64 hnsLookForward,
        _Out_ int64 * phnsLeakyBuffer,
        _Out_ int64 * phnsEndBuffer
        );

    uint32 GetNearestQualityMBRIndex( int32 chunkIndex, uint32 dwBandwidth, uint32 dwQuality );

    uint32 FindTargetQuality(
        uint32 dwBandwidth,
        int32 chunkIndex,
        int64 hnsLookForward);

    int64 PredictBufferAfterNextChunk(
        uint32 dwBandwidth,
        uint32 dwChunkIndex,
        uint32 dwMBRIndex );

    bool ShouldSwitchDownInEmergency( MediaStreamType mediaType, uint32 dwChunkIndex ) const
    {
        return (mediaType == MediaStreamTypeVideo) && (m_hnsBufferTime < (CFG_LowThreshold * TIMESCALE_10MHZ));
    }

private:
    CBandwidthHistory        m_BWHistory;

    CChunkManifest*          m_pManifest;
    CMediaStreamDescription* m_pVSD;            // video stream description

    uint64                   m_hnsStartTime;    // source start time
    uint64                   m_hnsBufferTime;   // source buffer time

    ThreadTime               m_threadTime;      // tracks cpu usage
    uint32                   m_pctCpuLimit;     // max total cpu allowed before downgrade (0==disabled)
    uint32                   m_pctCpuResume;    // cpu at which we'll allow quality level to move up again
    uint32                   m_dwResumeIndex;   // quality level index at which we forced downgrade

    uint32                   m_pctCpuLast;
    uint32                   m_pctCpuTotal;

    int                      m_downloadedChunk; // how many chunks have been downloaded
    bool                     m_isTrick;
    EQualityDirection        m_forcedQualityDirectionNextChunk; // force to step down/up to next selected quality for the next chunk
};

} // namespace MBR
