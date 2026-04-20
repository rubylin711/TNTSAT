///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CChunkManifest.h"
#include "CManifestChunk.h"
#include <wchar.h>
#include <string>
#include <algorithm>
#include <StringUtils.h>

using namespace MBR;
using namespace std;

//#define MSD_SPEW
#ifdef MSD_SPEW
#define MSD_TRACE(x) TRACE(x)
#else
#define MSD_TRACE(x)
#endif

/////////////////////////////////////////////////////////////////////////
// Auxiliary
/////////////////////////////////////////////////////////////////////////

static int64 ConvertTimeScale(
    int64 duration,
    int64 from,
    int64 to )
{
    if( from != to )
    {
        duration = ( duration / from * to ) + ( duration % from ) * to / from;
    }

    return( duration );
}


/////////////////////////////////////////////////////////////////////////
// CMediaStreamAttributes
/////////////////////////////////////////////////////////////////////////

pkRESULT CMediaStreamAttributes::GetAttributeByName(const WCHAR *pAttrName, wstring& wstrValue)
{
    ASSERT(pAttrName);
    AttributeMap::iterator it = m_AttributeMap.find(wstring(pAttrName));
    if (it == m_AttributeMap.end())
    {
        wstrValue = L"";
        return pkS_FALSE;
    }

    wstrValue = it->second;
    return pkS_OK;
}

/////////////////////////////////////////////////////////////////////////
// CMediaStreamDescription
/////////////////////////////////////////////////////////////////////////
pkRESULT CMediaStreamDescription::SetSubType(const WCHAR* wszSubtype)
{
    wstring wsSubtypeUpper(wszSubtype?wszSubtype:L"");
    for (size_t i = 0; i < wsSubtypeUpper.length(); i++)
    {
        wsSubtypeUpper[i] = towupper(wsSubtypeUpper[i]);
    }

    // subtype required for DFXP subtitles only for now, don't parse the other subtypes yet
    if (wsSubtypeUpper == L"SUBT")
    {
        m_MediaSubType = MediaStreamSubTypeSubtitles;
    }
    else if (wsSubtypeUpper == L"DESC")
    {
        m_MediaSubType = MediaStreamSubTypeDescriptions;
    }
    else if (wsSubtypeUpper == L"CAPT")
    {
        m_MediaSubType = MediaStreamSubTypeCaptions;
    }
    else if ((wsSubtypeUpper == L"H264")
        || (wsSubtypeUpper.substr(0, 3) == L"AVC")) // match AVC*
    {
        m_MediaSubType = MediaStreamSubTypeAVC;
    }
    else if (wsSubtypeUpper == L"WVC1")
    {
        m_MediaSubType = MediaStreamSubTypeWVC1;
    }
    else if (wsSubtypeUpper == L"WMA")
    {
        m_MediaSubType = MediaStreamSubTypeWMA;
    }
    else if (wsSubtypeUpper == L"WMAPRO")
    {
        m_MediaSubType = MediaStreamSubTypeWMAPro;
    }
    else if (wsSubtypeUpper == L"AAC")
    {
        m_MediaSubType = MediaStreamSubTypeAAC;
    }
    else
    {
        m_MediaSubType = MediaStreamSubTypeUnknown;
    }

    // Save the public facing sub type
    m_SubType = wszSubtype;

    return pkS_OK;
}

void CMediaStreamDescription::SnapChunkIndexToCurrentRange( _Inout_ int32* pChunkIndex )
{
    if( *pChunkIndex >= GetDVRMaxChunkIndex() )
    {
        *pChunkIndex = GetDVRMaxChunkIndex();
    }
    else
    {
        int32 firstIndex = GetDVRMinChunkIndex();

        if( *pChunkIndex < firstIndex )
        {
            *pChunkIndex = firstIndex;
        }
    }
}

int64 CMediaStreamDescription::GetChunkStartPosition( int32 chunkIndex )
{
    int64 result;
    AutoLock lock(&m_chunkBufferLock);

    SnapChunkIndexToCurrentRange( &chunkIndex );

    result = m_pChunkBuffer->GetChunkTicks(chunkIndex);

    result = ConvertTimeScale( result, m_llTimeScale, MBR_DEFAULT_TIMESCALE );

    return result;
}

int64 CMediaStreamDescription::GetChunkEndPosition( int32 chunkIndex )
{
    int64 result;
    AutoLock lock(&m_chunkBufferLock);

    SnapChunkIndexToCurrentRange( &chunkIndex );

    if ( chunkIndex == m_pChunkBuffer->GetMaxIndex() )
    {
        // last chunk must use last chunk duration that has been set.
        result = m_pChunkBuffer->GetChunkTicks(chunkIndex) + m_dwLastChunkDuration;
    }
    else
    {
        // use following chunk start position.
        result = m_pChunkBuffer->GetChunkTicks(chunkIndex+1);
    }

    result = ConvertTimeScale( result, m_llTimeScale, MBR_DEFAULT_TIMESCALE );

    return result;
}

uint32 CMediaStreamDescription::GetChunkDuration( int32 chunkIndex )
{
    uint32 result;
    AutoLock lock(&m_chunkBufferLock);

    SnapChunkIndexToCurrentRange( &chunkIndex );

    if ( chunkIndex ==  m_pChunkBuffer->GetMaxIndex() )
    {
        // last chunk must use last chunk duration that has been set.
        result = m_dwLastChunkDuration;
    }
    else
    {
        // get duration from the difference following chunk starting position.
        result = (uint32)(m_pChunkBuffer->GetChunkTicks(chunkIndex+1) - m_pChunkBuffer->GetChunkTicks(chunkIndex));
    }

    result = (uint32)ConvertTimeScale( result, m_llTimeScale, MBR_DEFAULT_TIMESCALE );

    return result;
}

uint32 CMediaStreamDescription::GetChunkSizeInKB(uint32 dwMBR, int32 chunkIndex)
{
    AutoLock lock(&m_chunkBufferLock);

    return m_pChunkBuffer->GetChunkSizeInKB( TrackWeakPtr( dwMBR ), chunkIndex );
}

CChunkInfo* CMediaStreamDescription::AddChunk(_In_ int64 pos, _In_opt_ EBufferDirection direction /*= eBufferDirection_Forward*/)
{
    CChunkInfo* pChunkInfo = NULL;
    {
        AutoLock lock(&m_chunkBufferLock);
        pChunkInfo = m_pChunkBuffer->Add(pos, direction);
    }

    if(pChunkInfo)
    {
        CheckChunkInfoRequest();
    }
    return pChunkInfo;
}

bool CMediaStreamDescription::AddChunks(_In_ const ChunkInfoVector& chunkInfo, _In_opt_ EBufferDirection direction /*= eBufferDirection_Forward*/)
{
    MSD_TRACE(("AddChunks %d %s, [%lld, %lld]", 
        chunkInfo.size(),
        eBufferDirection_Forward == direction ? "forward" : "backward",
        chunkInfo.size() ? chunkInfo.front().timestamp : 0, 
        chunkInfo.size() ? chunkInfo.back().timestamp : 0));

    {
        AutoLock lock(&m_chunkBufferLock);

        for(size_t i = 0; i < chunkInfo.size(); i++)
        {
            size_t index = (eBufferDirection_Forward == direction) ? i : chunkInfo.size()-(i+1);
            if(NULL == m_pChunkBuffer->Add(chunkInfo[index].timestamp, direction))
            {
                goto done;
            }

            // add fragment data
            uint32 bufferIndex = (eBufferDirection_Forward == direction) ? m_pChunkBuffer->GetMaxIndex() : m_pChunkBuffer->GetMinIndex();
            for(size_t j = 0; j < chunkInfo[i].FragmentMetadata.size(); j++)
            {
                if(NULL == chunkInfo[i].FragmentMetadata[j].pTrack)
                {
                    continue;
                }
                
                m_pChunkBuffer->SetChunkSizeInKB(chunkInfo[index].FragmentMetadata[j].pTrack,
                                                 bufferIndex,
                                                 chunkInfo[index].FragmentMetadata[j].chunkSizeInKB);

                if(chunkInfo[i].FragmentMetadata[j].quality)
                {
                    m_pChunkBuffer->SetQuality(chunkInfo[index].FragmentMetadata[j].pTrack,
                                               bufferIndex,
                                               chunkInfo[index].FragmentMetadata[j].quality);
                }

                if(chunkInfo[i].FragmentMetadata[j].apFragmentData)
                {
                    m_pChunkBuffer->SetFragmentData(chunkInfo[index].FragmentMetadata[j].pTrack,
                                                    bufferIndex,
                                                    chunkInfo[index].FragmentMetadata[j].apFragmentData);
                }
            }
        }
    }
done:
    CheckChunkInfoRequest();
    return true;
}

bool CMediaStreamDescription::SetDVRMinTime(_In_ TimeSpan_hns newTime)
{
    bool found = false;
    
    AutoLock lock(&m_chunkBufferLock);

    ASSERT(m_pChunkBuffer);
    ASSERT(m_DVRMinIndex >= 0);
    int32 newDVRMinIndex = m_DVRMinIndex;

    if(newTime == TimeSpan_hns::FromTicks(TimeSpan_hns::MIN_TICKS))
    {
        newDVRMinIndex = m_pChunkBuffer->GetMinIndex();
        found = true;
        goto done;
    }

    if(newTime.Ticks() > GetChunkStartPosition(m_DVRMinIndex))
    {
        while(newDVRMinIndex+1 < GetDVRMaxChunkIndex())
        {
            TimeSpan_hns tempMinTime = TimeSpan_hns::ConvertFrom(m_pChunkBuffer->GetChunkTicks(newDVRMinIndex+1), m_llTimeScale);
            if(tempMinTime > newTime)
            {
                found = true;
                goto done;
            }
            newDVRMinIndex++;
        }
    }
    else
    {
        while(newDVRMinIndex >= m_pChunkBuffer->GetMinIndex())
        {
            TimeSpan_hns tempMinTime = TimeSpan_hns::ConvertFrom(m_pChunkBuffer->GetChunkTicks(newDVRMinIndex), m_llTimeScale);
            if(tempMinTime <= newTime)
            {
                found = true;
                goto done;
            }
            newDVRMinIndex--;
        }
    }

done:
    if(found)
    {
        m_DVRMinIndex = newDVRMinIndex;
    }

    return found;
}

void CMediaStreamDescription::UpdateDVRMinTime()
{
    AutoLock lock(&m_chunkBufferLock);

    ASSERT(m_pChunkBuffer);

    if( -1 == m_DVRMinIndex || (0 == m_DVRWindowLength && m_pChunkBuffer->Count() == m_pChunkBuffer->Size()))
    {
        m_DVRMinIndex = m_pChunkBuffer->GetMinIndex();
    }

    // Update the DVR left edge
    if(0 != m_DVRWindowLength)
    {   
        bool found = false;
        int32 newDVRMinIndex = m_DVRMinIndex;

        while(newDVRMinIndex+1 < GetDVRMaxChunkIndex())
        {
            int64 dvrLen = GetChunkEndPosition(GetDVRMaxChunkIndex()) - GetChunkStartPosition(newDVRMinIndex+1);
            if(dvrLen < m_DVRWindowLength)
            {
                found = true;
                dvrLen = GetChunkEndPosition(GetDVRMaxChunkIndex()) - GetChunkStartPosition(newDVRMinIndex);
                break;
            }
            newDVRMinIndex++;
        }

        if (found)
        {
            if(m_DVRMinIndex != newDVRMinIndex)
            {
                MSD_TRACE(("[%d] m_DVRMinIndex updated: index(%d, %d) [%lld, %lld] DVRLen %lld",
                            m_dwStreamID, 
                            newDVRMinIndex, 
                            GetDVRMaxChunkIndex(), 
                            GetChunkStartPosition(newDVRMinIndex), 
                            GetChunkEndPosition(GetDVRMaxChunkIndex()), 
                            m_DVRWindowLength));
            
                m_DVRMinIndex = newDVRMinIndex;
            }
        }
        else
        {
            TRACE_ERROR(("[%d] m_DVRMinIndex failed to update [%d-%d] [%lld-%lld] DVRLen %lld",
                          m_dwStreamID, 
                          m_DVRMinIndex,
                          GetDVRMaxChunkIndex(),
                          GetChunkStartPosition(m_DVRMinIndex),
                          GetChunkEndPosition(GetDVRMaxChunkIndex()),
                          m_DVRWindowLength));
        }
    }
}

void CMediaStreamDescription::CheckChunkInfoRequest()
{
    IChunkInfoCallback* pCallback = NULL;
    CHUNK_INFO ci = { 0 };
    pkRESULT pkResultChunkGet = pkS_OK;
    
    {
        AutoLock lock(&m_chunkBufferLock);

        //
        // If there's a asynchronous request waiting for a chunk info,
        // see if the new chunk fulfills the request.
        //

        if( m_pCurrentChunkRequestCallback != NULL )
        {
            pkResultChunkGet = TryGetChunkInfo( m_itCurrentChunkRequest, &ci );
            if( pkResultChunkGet != pkE_PENDING )
            {
                // Either the chunk info was in the local cache,
                // does not exist at all, or the operation failed
                // for some other reason. Either way, it's time
                // to invoke the callback with the result.
                // Important: never invoke a callback from within
                // a object's main lock or it will cause a
                // dead-lock with the application's code.

                pCallback = m_pCurrentChunkRequestCallback;
                m_pCurrentChunkRequestCallback = NULL;
            }
        }

        //
        // If there's a fragment fetcher attatched to this
        // stream, give it a nudge in case it's waiting for
        // the chunk
        //

        if( m_apFragmentFetcher != NULL )
        {
            m_apFragmentFetcher->CacheUpdatePoke();
        }
    }

    if( pCallback != NULL )
    {
        // :TODO: this should be put in a work queue thread instead of
        // calling directly from here to avoid reentrancy problems.

        pCallback->OnChunkInfo( pkResultChunkGet, &ci );
    }
}

void CMediaStreamDescription::AddSparseChildChunkInfo(
    _In_ const std::string& strStreamName,
    _In_ int64 chunkTime )
{
    std::wstring strName = Str2WStr( strStreamName );

    MSD_TRACE(("@%p: child '%s', chunk %lld", this, strStreamName.c_str(), chunkTime ));

    AutoLock al( &m_TracksLock );

    for( size_t i = 0; i < m_childStreams.size(); ++i )
    {
        if( m_childStreams[i]->Name() == strName )
        {
            CManifestStream* pStrm = m_childStreams[i];

            static_cast<CMediaStreamDescription*>( pStrm )->AddChunk( chunkTime );

            break;
        }
    }
}

void CMediaStreamDescription::SetLookAheadCount(int32 lookAheadCount)
{
    AutoLock lock(&m_chunkBufferLock);
    m_lookAheadCount = lookAheadCount;
}

void CMediaStreamDescription::SetLastChunkDuration( _In_ uint32 dwLastChunkDuration) 
{ 
    m_dwLastChunkDuration = dwLastChunkDuration;
    UpdateDVRMinTime();
}

int64 CMediaStreamDescription::GetChunkTimeScaleStartPosition( int32 chunkIndex )
{
    AutoLock lock(&m_chunkBufferLock);

    SnapChunkIndexToCurrentRange( &chunkIndex );

    return m_pChunkBuffer->GetChunkTicks(chunkIndex);
}


pkRESULT CMediaStreamDescription::FindPositionByTime(
    _In_ int64 hnsTime,
    _In_ bool floorIt,
    _Out_ CMediaPosition* pPos )
{
    AutoLock lock(&m_chunkBufferLock);

    int32 iStart = GetDVRMinChunkIndex();
    int32 iEnd = GetDVRMaxChunkIndex();
    int32 iMiddle = iStart;
    int32 iChunkIndex = iStart;

    int64 scaledTime = ConvertTimeScale( hnsTime, MBR_DEFAULT_TIMESCALE, m_llTimeScale );

    // binary search accordingly to chunk start position
    while ( true )
    {
        if ((iStart >= iEnd) || (scaledTime <= m_pChunkBuffer->GetChunkTicks(iStart)))
        {
            iChunkIndex = iStart;
            break;
        }

        if ( scaledTime >= m_pChunkBuffer->GetChunkTicks(iEnd) )
        {
            iChunkIndex = iEnd;
            break;
        }

        iMiddle = ( iStart + iEnd ) / 2;

        if (  scaledTime < m_pChunkBuffer->GetChunkTicks(iMiddle) )
        {
            iEnd = iMiddle - 1;
        }
        else if (  scaledTime >= m_pChunkBuffer->GetChunkTicks(iMiddle+1) )
        {
            iStart = iMiddle + 1;
        }
        else
        {
            iChunkIndex = iMiddle;
            break;
        }
    }

    int64 hnsChunkStartTime = GetChunkStartPosition( iChunkIndex );

    // take the ceiling if we can, when requested
    if (!floorIt && (iChunkIndex < GetDVRMaxChunkIndex()) && (hnsTime > hnsChunkStartTime ))
    {
        iChunkIndex++;

        hnsChunkStartTime = GetChunkStartPosition( iChunkIndex );
    }

    *pPos = CMediaPosition( hnsChunkStartTime, iChunkIndex );

    return pkS_OK;
}

uint32 CMediaStreamDescription::GetFourCC(uint32 nQualityLevel)
{
    uint32 fourCC = 0;

    if( nQualityLevel < m_Tracks.size() )
    {
        fourCC = m_Tracks[nQualityLevel]->FourCC();
    }

    return fourCC;
}

uint32 CMediaStreamDescription::GetHeight(uint32 nQualityLevel)
{
    uint32 height = 0;

    if (m_Tracks.size() > nQualityLevel)
    {
        height = m_Tracks[nQualityLevel]->MaxHeight();
    }

    return height;
}

uint32 CMediaStreamDescription::GetWidth(uint32 nQualityLevel)
{
    uint32 width = 0;

    if (m_Tracks.size() > nQualityLevel)
    {
        width = m_Tracks[nQualityLevel]->MaxWidth();
    }

    return width;
}

byte* CMediaStreamDescription::GetCodecBlob(uint32 nQualityLevel, uint32* pnBlobLen)
{
    byte* pbCodecBlob = NULL;
    *pnBlobLen = 0;

    if (m_Tracks.size() > nQualityLevel)
    {
        pbCodecBlob = m_Tracks[nQualityLevel]->CodecPrivateData().data();
        *pnBlobLen = m_Tracks[nQualityLevel]->CodecPrivateData().size();
    }

    return pbCodecBlob;
}

// Returns 0-based quality level index
uint32 CMediaStreamDescription::GetDefaultQualityLevel()
{
    uint32 qlBest = 0;

    // Use cached QL, if available
    if (m_dwDefaultQL != INVALID_UINT32)
        return (uint32) m_dwDefaultQL;

    // If no comparer currently, use bitrate comparer
    if (NULL == m_pQLComparer)
    {
        m_pQLComparer = NEW_NO_THROW CBitrateQualityLevelComparer();
        CHECKNULL_GOTO( m_pQLComparer, exit );
    }

    for (size_t ql = 1; ql < m_Tracks.size(); ql ++)
    {
        if (m_pQLComparer->Compare( m_Tracks[ql], m_Tracks[qlBest] ) > 0 )
        {
            qlBest = ql;
        }
    }

exit:
    m_dwDefaultQL = qlBest;

    return qlBest;
}

void CMediaStreamDescription::SetComparer(IQualityLevelComparer *pComparer)
{
    SAFE_DELETE(m_pQLComparer);

    m_pQLComparer = pComparer;

    // Reset cached QL value
    m_dwDefaultQL = INVALID_UINT32;
}

void CMediaStreamDescription::SetDVRWindowLength( _In_ int64 lenSec )
{
    m_DVRWindowLength = lenSec * m_llTimeScale;
    UpdateDVRMinTime();
}

int32 CMediaStreamDescription::GetDVRCount()
{
    AutoLock lock(&m_chunkBufferLock);
    return( ( m_pChunkBuffer != NULL ) ? GetDVRMaxChunkIndex()-GetDVRMinChunkIndex()+1 : 0 );
}

TimeSpan_hns CMediaStreamDescription::TimeRemainingInDVR()
{
    AutoLock lock(&m_chunkBufferLock);

    TimeSpan_hns timeRemaining;

    //check if DVR window is full
    if((m_DVRWindowLength != 0 && (GetChunkEndPosition(GetDVRMaxChunkIndex()) - GetChunkStartPosition(GetDVRMinChunkIndex())) >= m_DVRWindowLength)
        || m_pChunkBuffer->Count() == m_pChunkBuffer->Size())
    {
        return timeRemaining;
    }
    
    // calculate the time left if the dvr window will fit in the chunklist
    if(m_DVRWindowLength != 0 && (m_pChunkBuffer->Size() * m_pChunkBuffer->GetDefaultChunkDuration() > m_DVRWindowLength))
    {
        int64 currentDvrWindowLength = GetChunkEndPosition(GetDVRMaxChunkIndex()) - GetChunkStartPosition(GetDVRMinChunkIndex());
        return timeRemaining.ConvertFrom(m_DVRWindowLength - currentDvrWindowLength, m_llTimeScale);
    }
    else
    {
        int32 chunksRemaining = m_pChunkBuffer->Size() - m_pChunkBuffer->Count();
        return timeRemaining.ConvertFrom(chunksRemaining * m_pChunkBuffer->GetDefaultChunkDuration(), m_llTimeScale);
    }
}

int32 CMediaStreamDescription::GetDVRMinChunkIndex()
{
    AutoLock lock(&m_chunkBufferLock);
    return ( m_pChunkBuffer != NULL ) ? m_DVRMinIndex : 0;
}

int32 CMediaStreamDescription::GetDVRMaxChunkIndex()
{
    int32 lastChunkIndex = 0;
    
    AutoLock lock(&m_chunkBufferLock);

    if (m_pChunkBuffer)
    {
        lastChunkIndex = m_pChunkBuffer->GetMaxIndex();

        if (lastChunkIndex > m_lookAheadCount)
        {
            lastChunkIndex -= m_lookAheadCount;
        }
        else
        {
            lastChunkIndex = 0;
        }
    }

    return lastChunkIndex;
}

bool CMediaStreamDescription::ContainsChunkIndex( int32 chunkIndex )
{
    return( GetDVRMinChunkIndex() <= chunkIndex && chunkIndex <= GetDVRMaxChunkIndex() );
}

bool CMediaStreamDescription::ChunkListContainsTime(TimeSpan_hns time)
{
    AutoLock lock(&m_chunkBufferLock);

    if( NULL == m_pChunkBuffer || 0 == m_pChunkBuffer->Count() )
    {
        return false;
    }

    TimeSpan_hns startTime = TimeSpan_hns::ConvertFrom(m_pChunkBuffer->GetChunkTicks(m_pChunkBuffer->GetMinIndex()), m_llTimeScale);

    return (time >= startTime && time.Ticks() <= GetChunkEndPosition(GetDVRMaxChunkIndex()));
}

uint32 CMediaStreamDescription::GetNominalBitrate(uint32 nQualityLevel)
{
    if (m_Tracks.size() > nQualityLevel)
    {
        return m_Tracks[nQualityLevel]->NominalBitrate() ? m_Tracks[nQualityLevel]->NominalBitrate() : m_Tracks[nQualityLevel]->Bitrate();
    }

    ASSERT(false);
    return 0;
}

pkRESULT CMediaStreamDescription::AddTrack( _Deref_out_ CManifestTrack** ppTrack )
{
    pkRESULT pkResult = pkS_OK;
    AutoRefPtr<CManifestTrack> apTrack;

    *ppTrack = NULL;

    pkResult = CreateManifestTrack( apTrack.DerefOutPtr() );
    if( pkFAILED(pkResult) )
    {
        goto exit;
    }

    // Tracks are added selected by default

    m_iLastSelectedTrack = m_Tracks.size();
    apTrack->OriginalIndex() = m_iLastSelectedTrack;

    // Add it to the list of available tracks in this stream

    m_Tracks.push_back( apTrack );

    //
    // Give the track a connection reference back to this stream.
    // A connection reference allows the track to get back to the stream
    // without producing a circular reference that would cause the stream
    // object to leak.
    //

    apTrack->SetStreamConnection( &m_ElementConnection );

    *ppTrack = apTrack.HandOffRef();

exit:

    return( pkResult );
}



void CMediaStreamDescription::SortQualityLevels()
{
    SortManifestTracksInAscendingBitrateOrder( &m_Tracks );
}


void CMediaStreamDescription::SetManifestIsParsedAndMerged()
{
    AutoLock al( &m_TracksLock );

    m_fManifestIsParsedAndMerged = true;
}


pkRESULT CMediaStreamDescription::GetAvailableTracks( _Out_ std::vector< AutoRefPtr<IManifestTrack> >* pAvailableTracks )
{
    pkRESULT pkResult = pkS_OK;

    AutoLock al( &m_TracksLock );

    pAvailableTracks->resize( m_Tracks.size() );

    for( size_t i = 0; i < m_Tracks.size(); ++i )
    {
        (*pAvailableTracks)[i].Set( m_Tracks[i] );
    }

    return (pkResult);
}

pkRESULT CMediaStreamDescription::GetSelectedTracks( _Out_ std::vector< AutoRefPtr<IManifestTrack> >* pSelectedTracks )
{
    pkRESULT pkResult = pkS_OK;

    AutoLock al( &m_TracksLock );

    pSelectedTracks->clear();
    pSelectedTracks->reserve( m_Tracks.size() );

    for( size_t i = 0; i < m_Tracks.size(); ++i )
    {
        if( m_Tracks[i]->IsSelected() )
        {
            pSelectedTracks->push_back( AutoRefPtr<IManifestTrack>( m_Tracks[i] ) );
        }
    }

    return (pkResult);
}

pkRESULT CMediaStreamDescription::RestrictTracks( _In_ const std::vector< AutoRefPtr<IManifestTrack> >& restrictedTracks )
{
    pkRESULT pkResult = pkS_OK;
    TrackVector::iterator itIn;
    TrackVector::iterator itOut;

    std::vector<bool> tracksToKeep;

    AutoLock al( &m_TracksLock );

    if( m_fManifestIsParsedAndMerged )
    {
        MSD_TRACE(("stream %p: cannot restrict tracks past manifest ready", this));
        pkResult = pkE_ACCESSDENIED;
        goto exit;
    }

    if( restrictedTracks.size() == 0 )
    {
        pkResult = pkE_INVALIDARG;
        goto exit;
    }

    //
    // Mark the tracks to keep
    //

    tracksToKeep.assign( m_Tracks.size(), false );   // all to remove by default

    for( size_t i = 0; i < restrictedTracks.size(); ++i )
    {
        itIn = std::find( m_Tracks.begin(), m_Tracks.end(), restrictedTracks[i] );

        if( itIn == m_Tracks.end() )
        {
            // Fail if the requested list has an unknown track
            MSD_TRACE(("stream %p: can't find track %p in the list of available tracks", this, (IManifestTrack*)restrictedTracks[i] ));
            pkResult = pkE_INVALIDARG;
            goto exit;
        }

        // This is a keeper
        tracksToKeep[ itIn - m_Tracks.begin() ] = true;
    }

    //
    // Walk the list and remove the tracks not marked for keeping.
    // Use move semantics to avoid AddRef/Release on the tracks.
    // Preserve the tracks' order.
    //

    itIn = m_Tracks.begin();
    itOut = m_Tracks.begin();

    for( size_t i = 0; i < tracksToKeep.size(); ++i )
    {
        if( tracksToKeep[i] )
        {
            itIn->AdoptRef( itOut->HandOffRef() );

            (*itIn)->IsSelected() = true;   // restricted tracks get selected by default

            ++itIn;
        }

        ++itOut;
    }

    // Trim the list to the restricted ones
    m_Tracks.erase( itIn, m_Tracks.end() );

    ASSERT( NumberOfAvailableTracks() );

    m_iLastSelectedTrack = ( NumberOfAvailableTracks() - 1 );

exit:

    return (pkResult);
}

pkRESULT CMediaStreamDescription::SelectTracks( _In_ const std::vector< AutoRefPtr<IManifestTrack> >& selectedTracks )
{
    pkRESULT pkResult = pkS_OK;

    std::vector<bool> tracksToSelect;

    AutoLock al( &m_TracksLock );

    if( selectedTracks.size() == 0 )
    {
        pkResult = pkE_INVALIDARG;
        goto exit;
    }

    //
    // Mark the tracks to select
    //

    tracksToSelect.assign( m_Tracks.size(), false );    // all to deselect by default

    for( size_t i = 0; i < selectedTracks.size(); ++i )
    {
        TrackVector::iterator it = std::find( m_Tracks.begin(), m_Tracks.end(), selectedTracks[i] );

        // Fail if the requested list has an unknown track
        if( it == m_Tracks.end() )
        {
            MSD_TRACE(("stream %p: can't find track %p in the list of available tracks", this, (IManifestTrack*)selectedTracks[i] ));
            pkResult = pkE_INVALIDARG;
            goto exit;
        }

        // This will be selected
        tracksToSelect[ it - m_Tracks.begin() ] = true;
    }

    //
    // Select the tracks
    //

    m_iLastSelectedTrack = 0;
    m_iFirstSelectedTrack = m_Tracks.size();

    ASSERT( tracksToSelect.size() == m_Tracks.size() );

    for( size_t i = 0; i < m_Tracks.size(); ++i )
    {
        m_Tracks[i]->IsSelected() = tracksToSelect[i];

        if( m_Tracks[i]->IsSelected() )
        {
            m_iLastSelectedTrack = MAX( m_iLastSelectedTrack, i );
            m_iFirstSelectedTrack = MIN( m_iFirstSelectedTrack, i );
        }
    }

    // There should be at least one track selected in
    // the stream
    ASSERT( m_iFirstSelectedTrack <= m_iLastSelectedTrack );
    ASSERT( m_iLastSelectedTrack < m_Tracks.size() );

exit:

    return (pkResult);
}

void CMediaStreamDescription::GetChildStreams( _Out_ std::vector< AutoRefPtr<IManifestStream> >* pChildStreams )
{
    AutoLock al( &m_TracksLock );

    pChildStreams->reserve( m_childStreams.size() );
    pChildStreams->clear();

    for(
        std::vector< AutoRefPtr<CManifestStream> >::iterator it = m_childStreams.begin();
        it != m_childStreams.end();
        ++it )
    {
        pChildStreams->push_back( AutoRefPtr<IManifestStream>( *it ) );
    }
}

void CMediaStreamDescription::GetParentStream( _Deref_out_opt_ IManifestStream ** ppParentStream )
{
    if( m_apParentStreamConnection != NULL )
    {
        m_apParentStreamConnection->GetContainer( ppParentStream );
    }
    else
    {
        *ppParentStream = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
ChunkIterator CMediaStreamDescription::GetIterator( _In_ int64_t minTime, _In_ int64_t time )
{
    return( ChunkIterator::ContextAccessor::MakeChunkIterator( this, minTime, time ) );
}

////////////////////////////////////////////////////////////////////////////////
ChunkIterator CMediaStreamDescription::GetFirstInCurrentChunkList()
{
    AutoLock al( &m_chunkBufferLock );

    ChunkIterator it;
    ChunkIterator::CONTEXT& ctx = CtxOf( it );

    ctx.m_pToken = this;    // mark this iterator came from this object
    ctx.m_minTime = 0;
    ctx.m_fByIndex = true;
    ctx.m_iChunk = GetDVRMinChunkIndex();

    return( it );
}

////////////////////////////////////////////////////////////////////////////////
ChunkIterator CMediaStreamDescription::GetLastInCurrentChunkList()
{
    AutoLock al( &m_chunkBufferLock );

    ChunkIterator it;
    ChunkIterator::CONTEXT& ctx = CtxOf( it );

    ctx.m_pToken = this;    // mark this iterator came from this object
    ctx.m_minTime = 0;
    ctx.m_fByIndex = true;
    ctx.m_iChunk = GetDVRMaxChunkIndex();

    return( it );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CMediaStreamDescription::TryGetChunkInfo(
    _In_ const ChunkIterator& itChunk,
    _Out_ CHUNK_INFO* pChunk )
{
    AutoLock al( &m_chunkBufferLock );

    pkRESULT pkResult = pkS_OK;

    memset( pChunk, 0, sizeof(CHUNK_INFO) );

    ChunkIterator::CONTEXT& ctx = CtxOf( itChunk );

    int32 iFirstChunk = GetDVRMinChunkIndex();
    int32 iLastChunk = GetDVRMaxChunkIndex();

    pkResult = _ValidateChunkIterator( itChunk );
    if( pkFAILED(pkResult) )
    {
        goto exit;
    }

    //
    // Try now if the iterator hasn't been synchronized to the cache yet
    //

    if( !ctx.m_fByIndex && ( iFirstChunk <= iLastChunk ) )
    {
        bool fGetBestMatchFromCache = false;

        if( !m_pChunkBuffer->IsLive() )
        {
            // In On-Demand we always look for the best match in the
            // cache because the cache contains all the chunks of the content.
            // If the iterator refers to a time beyond the end of the cache it'll
            // snap back to the last chunk, which is OK because there can't be
            // chunks after that.

            fGetBestMatchFromCache = true;
        }
        else
        {
            // In Live we only look for the best match in the cache
            // if the required time is within the cache range.
            // If the iterator refers to a time beyond the end of the cache it
            // can't be resolved at this moment.

            int64_t lastTimeInCache = 0;

            lastTimeInCache = m_pChunkBuffer->GetInfo( iLastChunk )->m_timescaleStartPos;
            lastTimeInCache += m_dwLastChunkDuration;

            fGetBestMatchFromCache = ( ctx.m_time < lastTimeInCache );
        }

        if( fGetBestMatchFromCache )
        {
            int32 iChunk = iLastChunk;

            while( iFirstChunk <= iChunk )
            {
                CChunkInfo* pChunkInfo = m_pChunkBuffer->GetInfo( iChunk );

                if( pChunkInfo->m_timescaleStartPos < ctx.m_minTime )
                {
                    TRACE_ERROR(("Range [%lld..%lld] doesn't include any chunks", ctx.m_minTime, ctx.m_time));
                    pkResult = pkE_NO_MORE_ITEMS;
                    goto exit;
                }
                else if( pChunkInfo->m_timescaleStartPos <= ctx.m_time )
                {
                    ctx.m_iChunk = iChunk;
                    ctx.m_fByIndex = true;
                    break;
                }

                --iChunk;
            }
        }
    }

    //
    // Check if the iterator refers to a chunk before the valid range
    //

    if( ctx.m_fByIndex && ( ctx.m_iChunk < iFirstChunk ) )
    {
        // Note: The chunk cache can't go backwards to
        // retrieve past chunks when they fall out of the window,
        // even if the content supports it (e.g., live to VOD).
        if( !m_pChunkBuffer->IsLive() )
        {
            pkResult = pkE_NO_MORE_ITEMS;
            goto exit;
        }
        else
        {
            pkResult = pkE_BEFORE_VALID_RANGE;
            goto exit;
        }
    }

    if( !ctx.m_fByIndex || ( iLastChunk < ctx.m_iChunk ) )
    {
        if( !m_pChunkBuffer->IsLive() )
        {
            // In purely VOD scenario all the chunks should be in the
            // cache already from parsing the manifest.
            pkResult = pkE_NO_MORE_ITEMS;
            goto exit;
        }
        else
        {
            // In live scenario a chunk not in cache may be yet
            // to come.
            pkResult = pkE_PENDING;
            goto exit;
        }
    }

    //
    // Iterator hits the cache
    //

    pChunk->chunkTime = m_pChunkBuffer->GetInfo( ctx.m_iChunk )->m_timescaleStartPos;

exit:

    if( pkFAILED(pkResult) )
    {
        if( ctx.m_fByIndex )
        {
            TRACE_ERROR(("Error 0x%x on iterator i=%d", pkResult, ctx.m_fByIndex));
        }
        else
        {
            TRACE_ERROR(("Error 0x%x on iterator t=[%lld..%lld]", pkResult, ctx.m_minTime, ctx.m_time));
        }
    }

    return( pkResult );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CMediaStreamDescription::GetChunkInfoAsync(
    _In_ const ChunkIterator& itChunk,
    _In_ IChunkInfoCallback* pCallback )
{
    pkRESULT pkResult = pkS_OK;
    CHUNK_INFO chunkInfo = { 0 };
    bool fFromCache = false;

    ASSERT( pCallback );

    {
        AutoLock al( &m_chunkBufferLock );

        if( m_pCurrentChunkRequestCallback != NULL )
        {
            TRACE_ERROR(("@:%p can't execute multiple GetChunkInfoAsync", this));
            pkResult = pkE_SINGLE_INSTANCE_OP;
            goto exit;
        }

        //
        // Try to get the chunk immediately in case it has just arrived.
        // Then if it's not in the local cache, keep track of the request
        // for when the chunk arrives.
        //

        pkResult = TryGetChunkInfo( itChunk, &chunkInfo );

        if( pkE_PENDING == pkResult )
        {
            m_itCurrentChunkRequest = itChunk;
            m_pCurrentChunkRequestCallback = pCallback;
            pkResult = pkS_OK;
            goto exit;
        }

        if( pkFAILED(pkResult) )
        {
            TRACE_ERROR(("@:%p 0x%x", this, pkResult ));
            goto exit;
        }

        fFromCache = true;
    }

exit:

    if( fFromCache )
    {
        // The chunk was in the local cache. Invoke the callback.

        // :TODO: this should be put in a work queue thread instead of
        // calling directly from here to avoid reentrancy problems.

        pCallback->OnChunkInfo( pkResult, &chunkInfo );
    }

    return( pkResult );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CMediaStreamDescription::AbortChunkInfoAsync()
{
    pkRESULT pkResult = pkS_OK;
    IChunkInfoCallback* pCallback = NULL;
    CHUNK_INFO dummy = { 0 };

    {
        AutoLock al( &m_chunkBufferLock );

        pCallback = m_pCurrentChunkRequestCallback;
        m_pCurrentChunkRequestCallback = NULL;
    }

    // :TODO: this should use a WorkThread to avoid reentrancy problems
    if( pCallback != NULL )
    {
        pCallback->OnChunkInfo( pkE_ABORT, &dummy );
        pkResult = pkS_OK;
    }
    else
    {
        pkResult = pkS_FALSE;
    }

    return( pkResult );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CMediaStreamDescription::DownloadFragmentAsync(
    _In_ const ChunkIterator& itChunk,
    _In_ IManifestTrack* pTrack,
    _In_ size_t cbMaxBufferLength,
    _In_ IFragmentCallback* pCallback )
{
    pkRESULT pkResult = pkS_OK;
    AutoRefPtr<IManifestStream> apStream;

    pkResult = _ValidateChunkIterator( itChunk );
    if( pkFAILED(pkResult) )
    {
        goto exit;
    }

    if( ( pCallback == NULL )
        || ( pTrack == NULL ) )
    {
        pkResult = pkE_INVALIDARG;
        goto exit;
    }

    pTrack->GetStream( apStream.DerefOutPtr() );

    if( apStream != this )
    {
        TRACE_ERROR(("@%p: not the track's [%p] stream", this, (IManifestStream*)apStream ));
        pkResult = pkE_INVALIDARG;
        goto exit;
    }

    {
        AutoLock al( &m_chunkBufferLock );

        if( m_apFragmentFetcher == NULL )
        {
            pkResult = DefaultFragmentFetcher::CreateInstance( this, m_apUrlServices, m_apFragmentFetcher.DerefOutPtr() );
            if( pkFAILED(pkResult) )
            {
                goto exit;
            }
        }

        pkResult = m_apFragmentFetcher->FetchFragmentAsync( itChunk, pTrack, cbMaxBufferLength, pCallback );
        if( pkFAILED(pkResult) )
        {
            goto exit;
        }
    }

exit:

    return( pkResult );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CMediaStreamDescription::TryGetFragment(
    _In_ const ChunkIterator& itChunk,
    _In_ uint32 trackIndex,
    _Out_ CHUNK_INFO* pChunkInfo,
    _Deref_out_opt_ IRefBuffer** ppFragmentData )
{
    pkRESULT pkResult = pkS_OK;
    CChunkInfo* pInfo = NULL;
    ChunkIterator::CONTEXT& ctx = CtxOf( itChunk );

    *ppFragmentData = NULL;

    AutoLock al( &m_chunkBufferLock );

    pkResult = TryGetChunkInfo( itChunk, pChunkInfo );
    if( pkFAILED(pkResult) )
    {
        goto exit;
    }

    ASSERT( ctx.m_fByIndex );

    pInfo = m_pChunkBuffer->GetInfo( ctx.m_iChunk );

    ASSERT( pInfo != NULL );

    if( trackIndex < pInfo->m_fragments.size() )
    {
        //
        // If the fragment data was in the manifest,
        // return it with the chunk info
        //
        *ppFragmentData =
            AutoRefPtr<IRefBuffer>(
                pInfo->m_fragments[ trackIndex ]
                ).HandOffRef();
    }

exit:

    return( pkResult );
}

CMediaStreamDescription::Range CMediaStreamDescription::GetBitrateRangeOfSelectedTracks()
{
    Range rg;
    rg._min = 0;
    rg._max = 0;

    AutoLock al( &m_TracksLock );

    // There should be at least one track selected in
    // the stream
    ASSERT( m_iFirstSelectedTrack <= m_iLastSelectedTrack );
    ASSERT( m_iLastSelectedTrack < m_Tracks.size() );

    rg._min = m_Tracks[m_iFirstSelectedTrack]->Bitrate();
    rg._max = m_Tracks[m_iLastSelectedTrack]->Bitrate();

    ASSERT( rg._min <= rg._max );

    return( rg );
}

CMediaStreamDescription::Range CMediaStreamDescription::GetQualityRangeOfSelectedTracks()
{
    Range rg;
    rg._min = (uint32)-1;
    rg._max = 0;

    // NOTE: the order of the following locks is important
    AutoLock al1( &m_TracksLock );
    AutoLock al2( &m_chunkBufferLock );

    // There should be at least one track selected in
    // the stream
    ASSERT( m_iFirstSelectedTrack <= m_iLastSelectedTrack );
    ASSERT( m_iLastSelectedTrack < m_Tracks.size() );

    for( size_t iTrack = m_iFirstSelectedTrack; iTrack <= m_iLastSelectedTrack; ++iTrack )
    {
        if( !m_Tracks[iTrack]->IsSelected() )
        {
            continue;
        }

        for( int32 iChunk = GetDVRMinChunkIndex(); iChunk <= GetDVRMaxChunkIndex(); ++iChunk )
        {
            CChunkInfo* pChunkInfo = m_pChunkBuffer->GetInfo( iChunk );
            if( pChunkInfo != NULL )
            {
                uint16 quality = m_pChunkBuffer->GetQuality( m_Tracks[iTrack], iChunk );

                rg._max = MAX( rg._max, quality );
                rg._min = MIN( rg._min, quality );
            }
        }
    }

    ASSERT( rg._min <= rg._max );

    // For safety ensure that rg._min <= rg._max
    rg._min = MIN( rg._min, rg._max );

    return( rg );
}

uint32 CMediaStreamDescription::GetIndexOfLastSelectedTrack()
{
    AutoLock al( &m_TracksLock );

    // There should be at least one track selected in
    // the stream
    ASSERT( m_iFirstSelectedTrack <= m_iLastSelectedTrack );
    ASSERT( m_iLastSelectedTrack < m_Tracks.size() );

    return( m_iLastSelectedTrack );
}

uint32 CMediaStreamDescription::GetIndexOfFirstSelectedTrack()
{
    AutoLock al( &m_TracksLock );

    // There should be at least one track selected in
    // the stream
    ASSERT( m_iFirstSelectedTrack <= m_iLastSelectedTrack );
    ASSERT( m_iFirstSelectedTrack < m_Tracks.size() );

    return( m_iFirstSelectedTrack );
}

uint32 CMediaStreamDescription::GetSelectedTrackWithNearestQuality(
        _In_ uint32 iChunk,
        _In_ uint32 quality,
        _In_ uint32 maxBitrate )
{
    uint32 iBestTrack = 0;
    uint32 bestDelta = (uint32)-1;

    // NOTE: the order of the following locks is important
    AutoLock al1( &m_TracksLock );
    AutoLock al2( &m_chunkBufferLock );

    //
    // Use the first selected track by default.
    // There must be at least one selected track in the list.
    //

    iBestTrack = m_iFirstSelectedTrack;

    //
    // Now try to find the best track
    //

    for( uint32 iTrack = iBestTrack; iTrack <= m_iLastSelectedTrack; ++iTrack )
    {
        if( !m_Tracks[iTrack]->IsSelected() )
        {
            continue;
        }

        if( m_Tracks[iTrack]->Bitrate() > maxBitrate )
        {
            break;
        }

        uint32 delta = abs( (int32)( m_pChunkBuffer->GetQuality( TrackWeakPtr( iTrack ), iChunk ) - quality ) );

        if( delta > bestDelta )
        {
            // Quality is ordered (in principle),
            // so stop when deltas start to grow bigger
            break;
        }

        iBestTrack = iTrack;
        bestDelta = delta;
    }

    return( iBestTrack );
}

uint32 CMediaStreamDescription::GetSelectedNextQualityIndex( _In_ uint32 quality, _In_ EQualityDirection direction )
{
    uint32 iNextQuality = quality;
    if (eQualityDirection_None == direction)
    {
        return( iNextQuality );
    }

    AutoLock al( &m_TracksLock );
    if ( eQualityDirection_Down == direction )
    {
        if ( 0 == iNextQuality )
        {
            return( iNextQuality );
        }

        for(uint32 iIndxQuality = iNextQuality-1; iIndxQuality >= m_iFirstSelectedTrack; --iIndxQuality)
        {
            if( m_Tracks[iIndxQuality]->IsSelected() )
            {
                iNextQuality = iIndxQuality;
                break;
            }
        }
    }
    else
    {
        for(uint32 iIndxQuality = iNextQuality+1; iIndxQuality <= m_iLastSelectedTrack; ++iIndxQuality)
        {
            if( m_Tracks[iIndxQuality]->IsSelected() )
            {
                iNextQuality = iIndxQuality;
                break;
            }
        }
    }

    return( iNextQuality );

}
pkRESULT CMediaStreamDescription::_ValidateChunkIterator( _In_ const ChunkIterator& itChunk )
{
    pkRESULT pkResult = pkS_OK;

    ChunkIterator::CONTEXT& ctx = CtxOf( itChunk );

    if( ctx.m_pToken != this )
    {
        TRACE_ERROR(("wrong iterator stream"));
        pkResult = pkE_INVALIDARG;
        goto exit;
    }

    if( !ctx.m_fByIndex && ( ctx.m_time < ctx.m_minTime ) )
    {
        TRACE_ERROR(("invalid iterator time range %lld..%lld", ctx.m_minTime, ctx.m_time ));
        pkResult = pkE_INVALIDARG;
        goto exit;
    }

    if( !m_bIsSelected )
    {
        TRACE_ERROR(("cannot iterate on unselected stream"));
        pkResult = pkE_INVALID_REQUEST;
        goto exit;
    }

exit:

    return( pkResult );
}

pkRESULT CMediaStreamDescription::CreateInstance( _Out_ CMediaStreamDescription** ppMediaStreamDescription )
{
    pkRESULT pkResult = pkS_OK;

    *ppMediaStreamDescription = NEW_NO_THROW CMediaStreamDescription();

    CHECKNULL_SET_PKRESULT_GOTO((*ppMediaStreamDescription), pkE_OUTOFMEMORY, exit);

exit:
    return pkResult;
}

void CMediaStreamDescription::TearDownCircularReferences()
{
    IChunkInfoCallback* pCallback = NULL;

    {
        AutoLock al( &m_TracksLock );

        m_selectedTracks.clear();
        m_childStreams.clear();
        m_Tracks.clear();

        pCallback = m_pCurrentChunkRequestCallback;
        m_pCurrentChunkRequestCallback = NULL;
    }

    if( pCallback != NULL )
    {
        // :TODO: this should be put in a work queue thread instead of
        // calling directly from here to avoid reentrancy problems.

        CHUNK_INFO chunkInfo = { 0 };

        pCallback->OnChunkInfo( pkE_ABORT, &chunkInfo );
    }
}

// test hook to setup the chunklist buffer, should only be used by unit tests
void CMediaStreamDescription::UnitTest_SetChunkBuffer( _In_ CChunkBuffer* pChunkBuffer )
{
    m_pChunkBuffer = pChunkBuffer;
}
