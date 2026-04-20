///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
//  File:       Heuristics.cpp
//  Heuristics module
//
//////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Heuristics.h"

//#define HEURISTICS_SPEW
#ifdef HEURISTICS_SPEW
#include "Utils.h"
#define HEURISTICS_TRACE(x) TRACE(x)
#else
#define HEURISTICS_TRACE(x)
#endif

// #define HEURISTICS_EXTRA_SPEW
#ifdef HEURISTICS_SPEW_EXTRA
#include "Utils.h"
#define HEURISTICS_TRACE_EXTRA(x) TRACE(x)
#else
#define HEURISTICS_TRACE_EXTRA(x)
#endif

#ifndef fabs
#define fabs(x) ((x >= 0.0) ? (x) : (-1.0 * (x)))
#endif

using namespace MBR;

static inline DWORD maxdw( DWORD a, DWORD b)
{
    return (a > b) ? a : b;
}

static inline DWORD mindw( DWORD a, DWORD b)
{
    return (a < b) ? a : b;
}

/////////////////////////////////////////////////////////////////////////

CHeuristicsMBR::CHeuristicsMBR( CChunkManifest *pManifest )
    : m_pManifest(pManifest)
    , m_pVSD(NULL)
    , m_hnsStartTime(0)
    , m_hnsBufferTime(0)
    , m_pctCpuLimit(0)
    , m_pctCpuResume(0)
    , m_dwResumeIndex(0)
    , m_pctCpuLast(0)
    , m_pctCpuTotal(0)
    , m_downloadedChunk(0)
    , m_isTrick(false)
    , m_forcedQualityDirectionNextChunk(eQualityDirection_None)
{
}

/////////////////////////////////////////////////////////////////////////
CHeuristicsMBR::~CHeuristicsMBR()
{
    SAFE_RELEASE(m_pVSD);
}

/////////////////////////////////////////////////////////////////////////
pkRESULT CHeuristicsMBR::Init(bool isTrick, uint32 pctLimit)
{
    pkRESULT pkResult = pkS_OK;

    SAFE_RELEASE(m_pVSD);
    m_pVSD = m_pManifest->GetStreamDescriptionByType(MediaStreamTypeVideo);
    CHECKNULL_SET_PKRESULT_GOTO( m_pVSD, pkE_UNEXPECTED, done );
    SAFE_ADDREF(m_pVSD);

    m_downloadedChunk = 0;
    m_isTrick = isTrick;
    m_forcedQualityDirectionNextChunk = eQualityDirection_None;

    //Initialize CPU tracking for heuristics
    m_threadTime.Init();
    m_threadTime.SetCheckPeriod(CFG_CpuCheckPeriod);
    m_pctCpuResume = 0;
    m_dwResumeIndex = 0;
    m_pctCpuLast = 0;
    m_pctCpuTotal = 0;

    m_pctCpuLimit = pctLimit;

done:
    return pkResult;
}

bool CHeuristicsMBR::ForceQualityNextChunk(_In_ uint32 streamId, 
                                           _In_ uint32 currentQualityIndex, 
                                           _In_ EQualityDirection direction) 
{
    CMediaStreamDescription *pSD = m_pManifest->GetStreamDescriptionById(streamId);
    CHECKNULL_GOTO( pSD, done );

    if( MediaStreamTypeVideo == pSD->Type() 
        && currentQualityIndex != m_pVSD->GetSelectedNextQualityIndex(currentQualityIndex, direction))
    {
        m_forcedQualityDirectionNextChunk = direction; 
        return true;
    }

done:
    return false;
}

pkRESULT CHeuristicsMBR::AddBandwidth( _In_ uint32 dwBandwidth )
{
    HEURISTICS_TRACE(("@%p: bandwidth %d", this, dwBandwidth));
    ++m_downloadedChunk;
    return m_BWHistory.Add(dwBandwidth);
}

// for the given chunk and quality, find the nearest stream index not exceeding (bandwidth * lookupfactor)
uint32 CHeuristicsMBR::GetNearestQualityMBRIndex(
    int32 chunkIndex,
    uint32 dwBandwidth,
    uint32 dwQuality)
{
    uint32 dwTargetMBRIndex = m_pVSD->GetIndexOfFirstSelectedTrack();

    CHECKNULL_GOTO( m_pVSD, done );

    ASSERT( chunkIndex <= m_pVSD->GetDVRMaxChunkIndex() );

    // Optimized loop as it is the most called inside heuristics (90% of time is spent in the following code)
    if (dwQuality > (m_pVSD->GetQualityRangeOfSelectedTracks()._min + TARGETQUALITYACCURACY) )
    {
        uint32 dwMaxBitrate = (uint32)(dwBandwidth * CFG_LookUpFactor);

        if ( dwMaxBitrate > CFG_MaxBandwidth )
        {
            dwMaxBitrate = CFG_MaxBandwidth;
        }

        if( m_isTrick )
        {
            // assume trick mode bitrate is a factor of normal playback bitrate
            dwMaxBitrate = (uint32)( dwMaxBitrate * CFG_TrickModeBitrateFactor );
        }

        dwTargetMBRIndex = m_pVSD->GetSelectedTrackWithNearestQuality( chunkIndex, dwQuality, dwMaxBitrate );
    }

    HEURISTICS_TRACE_EXTRA(("GetNearestQualityMBRIndex chunk=%d bw=%u qual=%u; result=%u",
        chunkIndex, dwBandwidth, dwQuality, dwTargetMBRIndex ));
done:
    return dwTargetMBRIndex;
}


pkRESULT CHeuristicsMBR::GetLeakyBucket(
    int32 chunkIndex,               // starting this chunk
    uint32 dwBandwidth,             // at this bandwidth
    uint32 dwQuality,               // at this target quality
    int64 hnsLookForward,           // simulate download over 2 minutes
    _Out_ int64 * phnsLeakyBuffer,  // get smallest time buffered within look forward period
    _Out_ int64 * phnsEndBuffer )   // get time buffered after look forward period
{
    pkRESULT pkResult = pkS_OK;

    int64 hnsDuration = 0;

    *phnsLeakyBuffer = 0;
    *phnsEndBuffer = 0;

    CHECKNULL_SET_PKRESULT_GOTO( m_pVSD, pkE_UNEXPECTED, done );

    if ( 0 == dwBandwidth )
    {
        ASSERT( dwBandwidth > 0 );
        pkResult = pkE_INVALIDARG;
    }
    else
    {
        AutoLock lock(&m_pVSD->m_chunkBufferLock);
        uint32 dwMBRIndex = GetNearestQualityMBRIndex( chunkIndex, dwBandwidth, dwQuality );
        uint32 chunkDuration = 0;
        uint32 dwChunkSize = 0;

        if (chunkIndex <= m_pVSD->GetDVRMaxChunkIndex())
        {
            // find the nearest quality in all MBR streams
            for ( int32 iChunk = chunkIndex;
                hnsDuration <= hnsLookForward /*&& iChunk <= m_pVSD->GetLastChunkIndex()*/ ;
                iChunk ++ )
            {
                chunkDuration = 0;
                if (iChunk <= m_pVSD->GetDVRMaxChunkIndex())
                {
                    chunkDuration = m_pVSD->GetChunkDuration( iChunk );
                }

                if (chunkDuration == 0)
                {
                    chunkDuration = m_pVSD->m_pChunkBuffer->GetDefaultChunkDuration();
                }

                dwChunkSize = 0;

                if (iChunk <= m_pVSD->GetDVRMaxChunkIndex())
                {
                    dwChunkSize = m_pVSD->m_pChunkBuffer->GetChunkSizeInKB( m_pVSD->TrackWeakPtr( dwMBRIndex ), iChunk) * 1024;
                }

                if (dwChunkSize == 0)
                {
                    dwChunkSize = m_pVSD->m_pChunkBuffer->GetDefaultChunkSizeInKB( m_pVSD->TrackWeakPtr( dwMBRIndex ) ) * 1024;
                }

                //uint32 dwMBRIndex = GetNearestQualityMBRIndex( iChunk, dwBandwidth, dwQuality );
                hnsDuration += chunkDuration;
                int64 xferTime = ((int64)dwChunkSize * 8 * TIMESCALE_10MHZ/dwBandwidth);
                *phnsEndBuffer += (int64)chunkDuration - xferTime;

                HEURISTICS_TRACE_EXTRA(("dur=%lld size=%d bandwidth=%d xfer=%lld\n",
                    m_pVSD->m_pChunkBuffer->GetInfo(iChunk)->m_hnsDuration, dwChunkSize, dwBandwidth, xferTime));

                if ( iChunk == chunkIndex // first chunk
                    || *phnsEndBuffer < *phnsLeakyBuffer )
                {
                    *phnsLeakyBuffer = *phnsEndBuffer;
                }
            }
        }
    }
done:
    return pkResult;
}

// predict time in buffer after downloading the given chunk/index at this bitrate
int64 CHeuristicsMBR::PredictBufferAfterNextChunk(
    uint32 dwBandwidth,
    uint32 dwChunkIndex,
    uint32 dwMBRIndex )
{
    int64 hnsCurBuffer = m_hnsBufferTime; //orig:m_spChunkSource->GetBufferTime( MediaStreamTypeVideo );

    if (0 == dwBandwidth)
    {
        ASSERT( dwBandwidth > 0 );
        goto done;
    }

    hnsCurBuffer +=
        ( (uint64)m_pVSD->GetChunkDuration( dwChunkIndex )
        - (uint64)m_pVSD->m_pChunkBuffer->GetChunkSizeInKB( m_pVSD->TrackWeakPtr( dwMBRIndex ), dwChunkIndex) * 1024 * 8 * TIMESCALE_10MHZ / dwBandwidth );

done:
    return hnsCurBuffer;
}

/////////////////////////////////////////////////////////////////////////
// binary search all quality levels to find highest sustainable quality for given bandwidth
uint32 CHeuristicsMBR::FindTargetQuality(
    uint32 dwBandwidth,
    int32 chunkIndex,
    int64 hnsLookForward )
{
    pkRESULT pkResult = pkS_OK;

    int64 hnsLeakyBuffer = 0;
    int64 hnsEndBuffer = 0;
    uint32 dwMBRIndex = 0;
    int64 hnsCurBuffer = 0;
    int64 hnsNextBuffer = 0;
    uint32 dwLastBandwidth = 0;
    uint32 dwUpperQuality = 0;
    uint32 dwLowerQuality = 0;
    uint32 dwMidQuality = 0;

    CHECKNULL_GOTO( m_pVSD, done );

    if ( chunkIndex > m_pVSD->GetDVRMaxChunkIndex() || dwBandwidth == 0 )
    {
        return 0;
    }

    dwLastBandwidth = m_BWHistory.GetLatestBandwidth();
    dwUpperQuality = m_pVSD->GetQualityRangeOfSelectedTracks()._max;
    dwLowerQuality = m_pVSD->GetQualityRangeOfSelectedTracks()._min;

    // do a binary search, find the highest quality level that current bandwidth can sustain.
    while ( dwUpperQuality - dwLowerQuality > TARGETQUALITYACCURACY )
    {
        dwMidQuality = ( dwLowerQuality + dwUpperQuality ) / 2;
        CHECK_PKRESULT_GOTO( GetLeakyBucket(
            chunkIndex,
            dwBandwidth,
            dwMidQuality,
            hnsLookForward,
            &hnsLeakyBuffer,
            &hnsEndBuffer ), done );

        dwMBRIndex = GetNearestQualityMBRIndex( chunkIndex, dwLastBandwidth, dwMidQuality );
        hnsCurBuffer = m_hnsBufferTime; //orig:m_spChunkSource->GetBufferTime( MediaStreamTypeVideo );
        hnsNextBuffer = PredictBufferAfterNextChunk( dwLastBandwidth, chunkIndex, dwMBRIndex );

        if (m_isTrick)
        {
            // apply a much simpler rule for trick mode
            if (hnsNextBuffer >= hnsCurBuffer)
            {
                dwLowerQuality = dwMidQuality;
            }
            else
            {
                dwUpperQuality = dwMidQuality;
            }
        }
        else
        {
            // a target quality is sustainable if:
            // c1) leaky buffer is higher than LowThreshold
            //     OR
            // c2) leaky buffer is large than 0 AND current buffer is less than lowThreshold( low buffering mode )
            // AND
            // c3) end buffer is higher than highthread hold.

            bool c1 = ( hnsLeakyBuffer + hnsNextBuffer > CFG_LowThreshold * TIMESCALE_10MHZ );
            bool c2 = ( hnsLeakyBuffer > 0 &&  hnsCurBuffer < ( CFG_LowThreshold * TIMESCALE_10MHZ ) );
            bool c3 = ( hnsEndBuffer + hnsCurBuffer > CFG_HighThreshold * TIMESCALE_10MHZ );
            HEURISTICS_TRACE_EXTRA(("%s> %d %d %d  hnsEndBuffer=%lld hnsLeakyBuffer=%lld hnsNextBuffer=%lld hnsCurBuffer=%lld dwBandwidth=%d i=%d (%f %f %f)\n",
                ( ( c1 || c2 ) && c3 ) ? "Higher" : "Lower",
                c1, c2, c3,
                hnsEndBuffer,
                hnsLeakyBuffer,
                hnsNextBuffer,
                hnsCurBuffer,
                dwBandwidth,
                dwMBRIndex,
                dwLowerQuality,
                dwMidQuality,
                dwUpperQuality));

            if ( ( c1 || c2 ) && c3 )
            {
                dwLowerQuality =  dwMidQuality;
            }
            else
            {
                dwUpperQuality =  dwMidQuality;
            }
        }
    }

done:
    return dwLowerQuality;
}


/////////////////////////////////////////////////////////////////////////
pkRESULT CHeuristicsMBR::GetNextChunk(
    _In_ uint32 dwStreamId,
    _In_ uint32 dwChunkIndex,
    _In_ uint32 dwCurrentBitrateIndex,
    _Out_ uint32* pdwBitrateIndex )
{
    MediaStreamType mediaType;
    pkRESULT pkResult = pkS_OK;
    uint32 dwTargetQuality = 0;
    float flRate = 1.0;
    uint32 dwBitrateIndex;


    *pdwBitrateIndex = dwCurrentBitrateIndex;

    uint32 dwBandwidthInUse = 0;
    int64 hnsLookForward = CFG_LookForwardDuration * TIMESCALE_10MHZ;

    CMediaStreamDescription *pSD = m_pManifest->GetStreamDescriptionById(dwStreamId);
    CHECKNULL_SET_PKRESULT_GOTO( pSD, pkE_UNEXPECTED, done );

    mediaType = pSD->Type();
    if ( mediaType !=  MediaStreamTypeVideo || !CFG_EnableMBRSwitch )
    {
        // return the max bitrate index to audio stream or dynamic switch is disabled
        *pdwBitrateIndex = pSD->GetDefaultQualityLevel();
        goto done;
    }

    if (m_forcedQualityDirectionNextChunk != eQualityDirection_None)
    {
        *pdwBitrateIndex = m_pVSD->GetSelectedNextQualityIndex(dwCurrentBitrateIndex, m_forcedQualityDirectionNextChunk);

        HEURISTICS_TRACE(("@%p: %sgrade qualityIndex %d->%d for chunk %d", this,
            m_forcedQualityDirectionNextChunk == eQualityDirection_Down ? "down" : "up",
            dwCurrentBitrateIndex,
            *pdwBitrateIndex,
            dwChunkIndex));

        m_threadTime.Init();
        m_threadTime.Collect();
        m_pctCpuResume = 0;
        m_dwResumeIndex = 0;

        m_forcedQualityDirectionNextChunk = eQualityDirection_None;

        goto done;
    }

    {
        // ShouldSwitchDownInEmergency
        if ( m_downloadedChunk == 0 || ShouldSwitchDownInEmergency(mediaType, dwChunkIndex) )
        {
            // first chunk after seek. clear bandwidth history except the latest one for fast responding
            m_BWHistory.ClearTillLastOne();
        }
        if ( m_isTrick  )
        {
            CHECKNULL_SET_PKRESULT_GOTO( m_pVSD, pkE_UNEXPECTED, done );

            // do not use too much bandwidth for smooth FF/RW.
            uint32 dwHighestBitrate =  m_pVSD->GetBitrateRangeOfSelectedTracks()._max;

            dwBandwidthInUse =  mindw( m_BWHistory.GetAverageBandwidth(), dwHighestBitrate );
        }
        else
        {
            dwBandwidthInUse =  (uint32)( m_BWHistory.GetAverageBandwidth() / fabs(flRate) );
        }

        if ( m_downloadedChunk < 2 )
        {
            // use 1/4 of bandwidth for the first chunk and 1/2 bandwidth for the seconds
            // to ensure source to build up some buffer
            dwBandwidthInUse = ( dwBandwidthInUse * ( m_downloadedChunk + 1 ) / 4 );
        }
    }

    dwTargetQuality =  FindTargetQuality( dwBandwidthInUse, dwChunkIndex, hnsLookForward );

    // We are taking min here because if avg is too high we can underrun our FIFO on
    // the next chunk.  Note also the simulation is run with latest for next chunk, not avg.

    dwBitrateIndex = GetNearestQualityMBRIndex(
            dwChunkIndex,
            (uint32)mindw(m_BWHistory.GetLatestBandwidth(), dwBandwidthInUse), //orig:dwBandwidthInUse,
            dwTargetQuality);

    // check if we need to force lower quality level if CPU usage is too high
    if (m_pctCpuLimit)
    {
        if (m_threadTime.Collect())
        {
            HEURISTICS_TRACE(("GetNextChunk(%d): CPU last %d / total %d (limit %d)",
                dwChunkIndex, m_threadTime.Last / 100, m_threadTime.TotalUsed, m_pctCpuLimit));
        }

    }

    if (*pdwBitrateIndex != dwBitrateIndex)
    {
        HEURISTICS_TRACE(("GetNextChunk(%d): QL %d -> %d", dwChunkIndex, dwCurrentBitrateIndex, dwBitrateIndex));

        *pdwBitrateIndex = dwBitrateIndex;

        m_pctCpuLast = m_threadTime.Last / 100;
        m_pctCpuTotal = m_threadTime.TotalUsed;

        // reset CPU usage if we change quality levels
        m_threadTime.Init();
        m_threadTime.Collect();
    }

done:

    return pkResult;
}

void CHeuristicsMBR::UpdateBufferStatus( _In_ const DecoderBufferStatus& decoderBufferStatus )
{
    m_hnsBufferTime = PTS_90KHZTO10MHZ((uint64)decoderBufferStatus.videoBufferLevel90kHz);
    HEURISTICS_TRACE(("@%p: bufferTime %lld", this, m_hnsBufferTime));
}

