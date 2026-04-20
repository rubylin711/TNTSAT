///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CChunkBuffer.h"
#include "CMbrConfiguration.h"

//#define CHUNKMANIFEST_SPEW
#ifdef CHUNKMANIFEST_SPEW
#include "Trace.h"
#endif

using namespace MBR;

/////////////////////////////////////////////////////////////////////////
// CChunkBuffer
/////////////////////////////////////////////////////////////////////////

CChunkBuffer::CChunkBuffer()
    : m_info(NULL)
    , m_nDefaultDuration(0)
    , m_pDefaultSize(NULL)
    , m_pDefaultQuality(NULL)
    , m_bLive(false)
    , m_dwSize(gMbrConfiguration.ChunklistMaxSize)
    , m_dwQualityLevels(0)
    , m_nextMaxIndex(gMbrConfiguration.ChunklistMaxSize)
    , m_nextMinIndex(gMbrConfiguration.ChunklistMaxSize-1)
{
}

int32 CChunkBuffer::GetMinIndex() 
{
    return (m_nextMaxIndex - m_nextMinIndex > m_dwSize) ?  (m_nextMaxIndex - m_dwSize) : (m_nextMinIndex + 1); 
}

int32 CChunkBuffer::GetMaxIndex() 
{ 
    return m_nextMaxIndex -1; 
}

int32 CChunkBuffer::Count()
{
    if((m_nextMaxIndex == m_dwSize) && (m_nextMinIndex == m_dwSize-1))
    {
        return 0;
    }

    ASSERT(GetMaxIndex() >= GetMinIndex());
    return (GetMaxIndex() - GetMinIndex() + 1); 
}

CChunkBuffer* CChunkBuffer::Create(uint32 dwQualityLevels, bool bLive /* = false */)
{
    CChunkBuffer* pResult = NULL;

    ASSERT( dwQualityLevels > 0);
    if (0 == dwQualityLevels)
    {
        goto done;
    }

    pResult = NEW_NO_THROW CChunkBuffer;
    CHECKNULL_GOTO(pResult, done);
    {
        ASSERT(pResult->m_dwSize > 0);
        if(0 == pResult->m_dwSize)
        {
            goto cleanup;
        }

        pResult->m_dwQualityLevels = dwQualityLevels;
        pResult->m_bLive = bLive;

        pResult->m_info = NEW_NO_THROW CChunkInfo[pResult->m_dwSize];
        CHECKNULL_GOTO(pResult->m_info, cleanup);

        pResult->m_pDefaultSize = NEW_NO_THROW uint16[dwQualityLevels];
        CHECKNULL_GOTO(pResult->m_pDefaultSize, cleanup);
        memset(pResult->m_pDefaultSize, 0, sizeof(uint16) * dwQualityLevels);

        pResult->m_pDefaultQuality = NEW_NO_THROW uint16[dwQualityLevels];
        CHECKNULL_GOTO(pResult->m_pDefaultQuality, cleanup);
        memset(pResult->m_pDefaultQuality, 0, sizeof(uint16) * dwQualityLevels);

        goto done;

    cleanup:
        delete pResult;
        pResult = NULL;
    }

done:
    return pResult;
}

CChunkBuffer::~CChunkBuffer()
{
    SAFE_DELETE_ARRAY(m_info);
    SAFE_DELETE_ARRAY(m_pDefaultSize);
    SAFE_DELETE_ARRAY(m_pDefaultQuality);
}

CChunkInfo* CChunkBuffer::GetInfo(int32 dwIndex)
{
    // Check if index is outside of current buffer range
    if ((dwIndex > GetMaxIndex()) || (dwIndex < GetMinIndex()))
    {
        return NULL;
    }

    ASSERT(m_info);
    return &m_info[dwIndex % m_dwSize];
}

CChunkInfo* CChunkBuffer::Add(_In_ int64 pos, _In_ EBufferDirection direction /*= eBufferDirection_Forward */)
{
    if (!m_bLive && (Count() >= m_dwSize))
    {
        return NULL;
    }

    ASSERT(m_info);
    CChunkInfo *pChunkInfo = NULL;
    if(eBufferDirection_Forward == direction)
    {
        // Check for duplicate / retrograde chunks
        if ((Count() > 0) && (pos <= m_info[GetMaxIndex() % m_dwSize].m_timescaleStartPos))
        {
            return NULL;
        }

        pChunkInfo = &m_info[m_nextMaxIndex % m_dwSize];

        m_nextMaxIndex++;
    }
    else
    {
        // adding to the back must not overwrite the front
        if(m_nextMaxIndex - m_nextMinIndex > m_dwSize)
        {
            return NULL;
        }

        // Check for duplicate / retrograde chunks
        if ((Count() > 0) && (pos >= m_info[GetMinIndex() % m_dwSize].m_timescaleStartPos))
        {
            return NULL;
        }

        pChunkInfo = &m_info[m_nextMinIndex % m_dwSize];
        m_nextMinIndex--;
    }

    if (m_bLive && Count() >= m_dwSize)
    {
        // If we've wrapped around, we need to clear chunk arrays (if allocated)
        if (pChunkInfo->m_pwSize)
        {
            memset(pChunkInfo->m_pwSize, 0, sizeof(uint16) * m_dwQualityLevels);
        }
        if (pChunkInfo->m_pwQuality)
        {
            memset(pChunkInfo->m_pwQuality, 0, sizeof(uint16) * m_dwQualityLevels);
        }
    }

    pChunkInfo->m_timescaleStartPos = pos;

    return pChunkInfo;
}

int64 CChunkBuffer::GetChunkTicks(uint32 dwIndex)
{
    CChunkInfo* pChunkInfo = GetInfo(dwIndex);

    return pChunkInfo ? pChunkInfo->m_timescaleStartPos : 0;
}

uint32 CChunkBuffer::GetChunkSizeInKB( _In_ const CManifestTrack* pTrack, _In_ uint32 iChunk )
{
    uint32 dwMBR = pTrack->OriginalIndex();
    CChunkInfo* pChunkInfo = GetInfo( iChunk );

    if (pChunkInfo && pChunkInfo->m_pwSize && pChunkInfo->m_pwSize[dwMBR])
    {
        return pChunkInfo->m_pwSize[dwMBR];
    }

    ASSERT(m_pDefaultSize);
    return m_pDefaultSize[dwMBR];
}

uint16 CChunkBuffer::GetQuality( _In_ const CManifestTrack* pTrack, _In_ uint32 iChunk )
{
    uint32 dwMBR = pTrack->OriginalIndex();
    CChunkInfo* pChunkInfo = GetInfo(iChunk);

    if (pChunkInfo && pChunkInfo->m_pwQuality && pChunkInfo->m_pwQuality[dwMBR])
    {
        return pChunkInfo->m_pwQuality[dwMBR];
    }

    ASSERT(m_pDefaultQuality);
    return m_pDefaultQuality[dwMBR];
}

pkRESULT CChunkBuffer::SetChunkSizeInKB( _In_ const CManifestTrack* pTrack, _In_ uint32 iChunk, _In_ uint16 wSize )
{
    pkRESULT pkResult = pkS_OK;

    uint32 dwMBR = pTrack->OriginalIndex();
    CChunkInfo* pChunkInfo = GetInfo(iChunk);
    CHECKNULL_SET_PKRESULT_GOTO(pChunkInfo, pkE_INVALIDARG, exit);

    ASSERT(dwMBR < m_dwQualityLevels);
    if (dwMBR < m_dwQualityLevels)
    {
        // Allocate array if it doesn't exist already
        if (NULL == pChunkInfo->m_pwSize)
        {
            pChunkInfo->m_pwSize = NEW_NO_THROW uint16[m_dwQualityLevels];
            CHECKNULL_SET_PKRESULT_GOTO(pChunkInfo->m_pwSize, pkE_OUTOFMEMORY, exit);

            memset(pChunkInfo->m_pwSize, 0, sizeof(uint16) * m_dwQualityLevels);
        }
        pChunkInfo->m_pwSize[dwMBR] = wSize;
    }
exit:
    return pkResult;
}

pkRESULT CChunkBuffer::SetQuality( _In_ const CManifestTrack* pTrack, _In_ uint32 iChunk, _In_ uint16 wQuality )
{
    pkRESULT pkResult = pkS_OK;

    uint32 dwMBR = pTrack->OriginalIndex();
    CChunkInfo* pChunkInfo = GetInfo(iChunk);
    CHECKNULL_SET_PKRESULT_GOTO(pChunkInfo, pkE_INVALIDARG, exit);

    ASSERT(dwMBR < m_dwQualityLevels);
    if(dwMBR < m_dwQualityLevels)
    {
        // Allocate array if it doesn't exist already
        if (NULL == pChunkInfo->m_pwQuality)
        {
            pChunkInfo->m_pwQuality = NEW_NO_THROW uint16[m_dwQualityLevels];
            CHECKNULL_SET_PKRESULT_GOTO(pChunkInfo->m_pwQuality, pkE_OUTOFMEMORY, exit);

            memset(pChunkInfo->m_pwQuality, 0, sizeof(uint16) * m_dwQualityLevels);
        }
        pChunkInfo->m_pwQuality[dwMBR] = wQuality;
    }
exit:
    return pkResult;
}

pkRESULT CChunkBuffer::SetFragmentData(
    _In_ const CManifestTrack* pTrack,
    _In_ uint32 iChunk,
    _In_ IRefBuffer* pFragmentData )
{
    pkRESULT pkResult = pkS_OK;

    uint32 dwMBR = pTrack->OriginalIndex();
    CChunkInfo* pChunkInfo = GetInfo( iChunk );
    CHECKNULL_SET_PKRESULT_GOTO( pChunkInfo, pkE_INVALIDARG, exit );

    ASSERT(dwMBR < m_dwQualityLevels);

    if(dwMBR < m_dwQualityLevels)
    {
        pChunkInfo->m_fragments.resize( m_dwQualityLevels );

        pChunkInfo->m_fragments[ dwMBR ].Set( pFragmentData );
    }

exit:

    return( pkResult );
}


void CChunkBuffer::DumpChunkList( const char* szLinePrefix )
{
#ifdef CHUNKMANIFEST_SPEW

    TRACE(("%sm_nDefaultDuration: %d", szLinePrefix, m_nDefaultDuration));

    for (uint32 j=0; j<m_dwQualityLevels; ++j)
    {
        TRACE(("%sm_pDefaultSize[%d]: %d", szLinePrefix, j, m_pDefaultSize[j]));
        TRACE(("%sm_pDefaultQuality[%d]: %d", szLinePrefix, j, m_pDefaultQuality[j]));
    }

    TRACE(("%sm_bLive: %s", szLinePrefix, m_bLive?"yes":"no"));
    TRACE(("%sm_dwSize: %d", szLinePrefix, m_dwSize));
    TRACE(("%sm_dwQualityLevels: %d", szLinePrefix, m_dwQualityLevels));
    TRACE(("%sCount: %d", szLinePrefix, Count()));

    for (int32 nCID=0; nCID<m_dwSize; ++nCID)
    {
        for (uint32 nQID=0; nQID<m_dwQualityLevels; ++nQID)
        {
            if (m_info[nCID].m_pwQuality)
            {
                TRACE(("%sm_info[%d]->m_pwQuality[%d]: %d", szLinePrefix, nCID, nQID, m_info[nCID].m_pwQuality[nQID]));
            }
            if (m_info[nCID].m_pwSize)
            {
                TRACE(("%sm_info[%d]->m_pwSize[%d]: %d", szLinePrefix, nCID, nQID, m_info[nCID].m_pwSize[nQID]));
            }
        }
    }
#endif
}

