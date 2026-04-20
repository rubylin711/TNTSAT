///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CManifestTrack.h"
#include "ManifestChunk.h"

namespace MBR
{

enum EBufferDirection
{
    eBufferDirection_Forward,
    eBufferDirection_Backward
};

struct CChunkInfo
{
    CChunkInfo()
        : m_timescaleStartPos( 0 )
        , m_pwSize( NULL )
        , m_pwQuality( NULL )
    {
    }

    ~CChunkInfo()
    {
        SAFE_DELETE_ARRAY(m_pwSize);
        SAFE_DELETE_ARRAY(m_pwQuality);
    }

    int64   m_timescaleStartPos; // start position in terms of timescale duration

    uint16 *m_pwSize;            // each bitrate chunk has different size (in KB)
    uint16 *m_pwQuality;         // each bitrate chunk has different quality level

    std::vector< AutoRefPtr<IRefBuffer> > m_fragments;
};

class CChunkBuffer
{
protected:
    CChunkBuffer();

public:
    static CChunkBuffer* Create(uint32 dwQualityLevels = 1, bool bLive = false);

    ~CChunkBuffer();

    int32        GetMinIndex();
    int32        GetMaxIndex();
    int32        Count();
    int32        Size() const { return m_dwSize; }
    CChunkInfo*  GetInfo(int32 dwIndex);
    CChunkInfo*  Add(_In_ int64 pos, _In_ EBufferDirection direction = eBufferDirection_Forward);

    uint32       GetDefaultChunkDuration() { return m_nDefaultDuration; }
    void         SetDefaultChunkDuration(uint32 dwDuration) { m_nDefaultDuration  = dwDuration; }

    int64        GetChunkTicks( uint32 dwIndex );
    uint32       GetChunkSizeInKB( _In_ const CManifestTrack* pTrack, _In_ uint32 iChunk );
    uint16       GetQuality( _In_ const CManifestTrack* pTrack, _In_ uint32 iChunk );
    pkRESULT     SetChunkSizeInKB( _In_ const CManifestTrack* pTrack, _In_ uint32 iChunk, _In_ uint16 wSize );
    pkRESULT     SetQuality( _In_ const CManifestTrack* pTrack, _In_ uint32 iChunk, _In_ uint16 wQuality );
    pkRESULT     SetFragmentData( _In_ const CManifestTrack* pTrack, _In_ uint32 iChunk, _In_ IRefBuffer* pFragmentData );

    void         SetDefaultChunkSizeInKB( _In_ CManifestTrack* pTrack, _In_ uint16 wSize )  { m_pDefaultSize[ pTrack->OriginalIndex() ] = wSize; }
    uint32       GetDefaultChunkSizeInKB( _In_ CManifestTrack* pTrack )                     { return( m_pDefaultSize[ pTrack->OriginalIndex() ] ); }
    void         SetDefaultQuality( _In_ CManifestTrack* pTrack, _In_ uint16 wQuality )     { m_pDefaultQuality[pTrack->OriginalIndex()] = wQuality; }

    bool         IsLive() const { return( m_bLive ); }

    void         DumpChunkList( const char* szLinePrefix );

private:

    CChunkInfo*  m_info;

    uint32       m_nDefaultDuration;
    uint16*      m_pDefaultSize;
    uint16*      m_pDefaultQuality;

    bool         m_bLive;
    int32        m_dwSize;
    uint32       m_dwQualityLevels;

    int32        m_nextMaxIndex;
    int32        m_nextMinIndex;
};

} // namespace MBR
