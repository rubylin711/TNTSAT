///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CChunkManifest.h"
#include "StringUtils.h"
#include "Trace.h"
#include "StreamTypeTraits.h"
#include "MbrDiagEvents.h"

#include <algorithm>

using namespace MBR;
using namespace std;

//#define CHUNKMANIFEST_SPEW
#ifdef CHUNKMANIFEST_SPEW
#define CHUNKMANIFEST_TRACE(x) TRACE(x)
#else
#define CHUNKMANIFEST_TRACE(x)
#endif

/////////////////////////////////////////////////////////////////////////
template< class PREDICATE >
void CopyToOutputListIf(
    _In_ CChunkManifest::MSDList& inputStreams,
    _In_ PREDICATE pred,
    _Out_ std::vector< AutoRefPtr<IManifestStream> >* pOutputStreams )
{
    pOutputStreams->clear();
    pOutputStreams->reserve( inputStreams.size() );

    for( CChunkManifest::MSDList::iterator itMSD = inputStreams.begin();
         itMSD != inputStreams.end();
         ++itMSD )
    {
        if( pred( *itMSD ) )
        {
            pOutputStreams->push_back( AutoRefPtr<IManifestStream>( *itMSD ) );
        }
    }
}

template< class ITERATOR, class PREDICATE >
bool RangeHasAnyOf( ITERATOR first, ITERATOR last, PREDICATE pred )
{
    return( std::find_if( first, last, pred ) != last );
}

static bool StreamExists( void* )
{
    return( true );
}

static bool StreamIsSelected( _In_ CMediaStreamDescription* pMSD )
{
    return( pMSD->IsSelected() );
}

template< MediaStreamType TYPE >
static bool StreamOfType( _In_ CMediaStreamDescription* pMSD )
{
    return( pMSD->Type() == TYPE );
}

template< MediaStreamType TYPE >
static bool StreamOfTypeIsSelected( _In_ CMediaStreamDescription* pMSD )
{
    return( ( pMSD->Type() == TYPE ) && pMSD->IsSelected() );
}

class StreamParentNotInList
{
public:

    StreamParentNotInList( _In_ const std::vector< AutoRefPtr<IManifestStream> >& list )
        : m_list( list )
    {
    }

    bool operator()( _In_ IManifestStream* pStream )
    {
        AutoRefPtr<IManifestStream> apParentStream;
        pStream->GetParentStream( apParentStream.DerefOutPtr() );

        // Return true if the stream has a parent and the parent is not in the list

        return(
            ( apParentStream != NULL )
            && std::find( m_list.begin(), m_list.end(), apParentStream ) == m_list.end()
            );
    }

private:

    const std::vector< AutoRefPtr<IManifestStream> >& m_list;
};

class StreamHasName
{
public:

    StreamHasName( const std::wstring& strName )
        : m_strName( strName )
    {
    }

    bool operator()( _In_ CMediaStreamDescription* pMSD )
    {
        if( pMSD->Name().empty() )
        {
            return( m_strName == GetStreamTypeTraits( pMSD->Type() )->pszDefaultName );
        }
        else
        {
            return( m_strName == pMSD->Name() );
        }
    }

private:

    const std::wstring& m_strName;
};

/////////////////////////////////////////////////////////////////////////
pkRESULT CChunkManifest::GetAvailableStreams( _Out_ std::vector< AutoRefPtr<IManifestStream> >* pAvailableStreams )
{
    CopyToOutputListIf( m_availableStreams, StreamExists, pAvailableStreams );
    return ( pkS_OK );
}

/////////////////////////////////////////////////////////////////////////
pkRESULT CChunkManifest::GetSelectedStreams( _Out_ std::vector< AutoRefPtr<IManifestStream> >* pSelectedStreams )
{
    CopyToOutputListIf( m_availableStreams, StreamIsSelected, pSelectedStreams );
    return ( pkS_OK );
}

pkRESULT CChunkManifest::SelectStreamsAsync(
    _In_ IStreamsSelectedCallback* pCallback,
    _In_ std::vector< AutoRefPtr<IManifestStream> >& selectedStreams )
{
    pkRESULT pkResult = pkS_OK;

    CHECKNULL_SET_PKRESULT_GOTO(pCallback, pkE_INVALIDARG, exit);

    {
        AutoLock al(&m_streamSelectionLock);

        if (!VerifyStreamSelection(selectedStreams))
        {
            pkResult = pkE_INVALIDARG;
            CHUNKMANIFEST_TRACE(("The stream selection is not valid."));
            goto exit;
        }

        if (m_fStreamSelectionInProcess)
        {
            pkResult = pkE_ABORT;
            CHUNKMANIFEST_TRACE(("There is already a stream selection in process."));
            goto exit;
        }

        m_fStreamSelectionInProcess = true;
    }

    FinishStreamSelection(selectedStreams, pCallback);

exit:

    return (pkResult);
}

EManifestType CChunkManifest::Type() const 
{ 
    return m_apUrlServices->IsSegmented() ? ManifestType_Segmented : ManifestType_Standard; 
}

CMediaStreamDescription* CChunkManifest::AddStream()
{
    AutoRefPtr<CMediaStreamDescription> apStreamInfo;
    CMediaStreamDescription* pStreamInfo = NULL;

    if (pkFAILED( CMediaStreamDescription::CreateInstance( apStreamInfo.DerefOutPtr() ) ) )
    {
        // Allocation failure
        ASSERT( false );
        return NULL;
    }

    apStreamInfo->SetManifestUrlServices( m_apUrlServices );

    // Add new MSD to the available stream list
    m_availableStreams.push_back( apStreamInfo );

    // m_StreamList[index] (0-based) => m_dwStreamID - 1 (1-based)
    apStreamInfo->m_dwStreamID = m_availableStreams.size();

    // Stream list has changed so clear cache
    m_StreamTypeCache.clear();

    // The caller doesn't call Release when it's done with the pointer this method returns.
    // Instead, the caller expects the reference to be held internally by this object.
    // Therefore, we must return the pointer without calling AddRef on it (the reference is held
    // by the m_availableStreams list).

    pStreamInfo = apStreamInfo;

    return (pStreamInfo);
}

CMediaStreamDescription* CChunkManifest::GetStreamDescriptionByType(
    MediaStreamType mType, MediaStreamSubType mSubType )
{
    // First check if cached reference exists for this type
    MSDTypeMap::iterator it = m_StreamTypeCache.find(MEDIASTREAM_TYPE_HASH(mType, mSubType));
    if (it != m_StreamTypeCache.end())
    {
        return it->second;
    }

    for (uint32 i = 0; i < m_availableStreams.size(); i++)
    {
        CMediaStreamDescription* pStreamInfo = m_availableStreams[i];
        if ( (mType    == pStreamInfo->Type()   )
            && ((mSubType == MediaStreamSubTypeUnknown      )
            ||  (mSubType == pStreamInfo->m_MediaSubType)) )
        {
            // Store stream type in cache and return
            m_StreamTypeCache[MEDIASTREAM_TYPE_HASH(mType, mSubType)].Set(pStreamInfo);
            return pStreamInfo;
        }
    }

    return NULL;
}

// Find MSD by StreamId (1-based)
CMediaStreamDescription* CChunkManifest::GetStreamDescriptionById(uint32 dwStreamId)
{
    CMediaStreamDescription* pMSD = NULL;

    if (dwStreamId && dwStreamId <= m_availableStreams.size())
    {
        pMSD = m_availableStreams[dwStreamId - 1];
    }

    return (pMSD);
}

CMediaStreamDescription* CChunkManifest::GetStreamDescriptionByStream(IManifestStream* pStream)
{
    CMediaStreamDescription* pMSD = NULL;

    MSDList::iterator itMSD = std::find( m_availableStreams.begin(), m_availableStreams.end(), pStream );

    if( itMSD != m_availableStreams.end() )
    {
        pMSD = *itMSD;
    }

    return( pMSD );
}

uint32 CChunkManifest::GetActiveAudioId()
{
    AutoLock al(&m_manifestLock);

    MSDList::iterator itMSD =
                        std::find_if(
                                m_availableStreams.begin(),
                                m_availableStreams.end(),
                                StreamOfTypeIsSelected<MediaStreamTypeAudio> );

    return( ( itMSD != m_availableStreams.end() ) ? (*itMSD)->m_dwStreamID : 0 );
}

/////////////////////////////////////////////////////////////////////////
pkRESULT CChunkManifest::GetChunkURL(
    uint32 dwStreamId,
    int32 chunkIndex,
    uint32 iMBR,
    _Out_ wstring& wstrURL,
    bool  isTrick,
    bool  isChunkInfo)
{
    pkRESULT pkResult = pkS_OK;
    CMediaStreamDescription* pSD = NULL;
    CManifestTrack* pTrack = NULL;

    pSD = GetStreamDescriptionById( dwStreamId ); // no addref
    CHECKNULL_SET_PKRESULT_GOTO( pSD, pkE_UNEXPECTED, done);

    pTrack = pSD->TrackWeakPtr( iMBR );
    CHECKNULL_SET_PKRESULT_GOTO( pTrack, pkE_INVALIDARG, done );

    if ( chunkIndex > pSD->GetDVRMaxChunkIndex() )
    {
        CHECK_PKRESULT_GOTO( pkE_INVALIDARG, done );
    }

    pkResult = m_apUrlServices->FormatURL(
                        pSD->Url().c_str(),
                        pTrack,
                        chunkIndex,
                        pTrack->HardwareProfile(),
                        pSD->GetChunkTimeScaleStartPosition(chunkIndex),
                        &wstrURL );
    CHECK_PKRESULT_GOTO( pkResult, done );

    if (isTrick)
    {
        StrReplaceInPlace( &wstrURL, L"Fragments", L"KeyFrames");
    }
    else if (isChunkInfo)
    {
        StrReplaceInPlace( &wstrURL, L"Fragments", L"FragmentInfo");
    }

done:

    return pkResult;
}

/////////////////////////////////////////////////////////////////////////
pkRESULT CChunkManifest::GetLatestChunkInfoURL(uint32 dwStreamId, uint32 iMBR, _Out_ wstring& wstrURL)
{
    pkRESULT pkResult = pkS_OK;
    CMediaStreamDescription* pSD = NULL;
    CManifestTrack* pTrack = NULL;

    pSD = GetStreamDescriptionById( dwStreamId ); // no addref
    CHECKNULL_SET_PKRESULT_GOTO( pSD, pkE_UNEXPECTED, done );

    pTrack = pSD->TrackWeakPtr( iMBR );
    CHECKNULL_SET_PKRESULT_GOTO( pTrack, pkE_INVALIDARG, done );

    pkResult = m_apUrlServices->FormatURL(
                            pSD->Url().c_str(),
                            pTrack,
                            0,
                            pTrack->HardwareProfile(),
                            (uint64) -1,
                            &wstrURL );

    StrReplaceInPlace( &wstrURL, L"Fragments", L"FragmentInfo");

done:

    return pkResult;
}

pkRESULT CChunkManifest::GetSegmentedManifestURL(_In_ int64 pos, _Out_ std::wstring* wstrURL)
{
    return m_apUrlServices->FormatSegmentManifestURL(
                            pos,
                            wstrURL );
}
/////////////////////////////////////////////////////////////////////////
pkRESULT CChunkManifest::SelectInitialStreams()
{
    pkRESULT pkResult = pkS_OK;
    AutoLock al(&m_manifestLock);

    // Add the all the streams to selected streams
    // If there are multiple audio/video streams, only the last one will be added

    bool fFoundVideo = false;
    bool fFoundAudio = false;

    size_t i = m_availableStreams.size();
    while( i > 0 )
    {
        --i;

        CMediaStreamDescription* pMSD = m_availableStreams[i];

        if( !fFoundVideo && ( MediaStreamTypeVideo == pMSD->Type() ) )
        {
            pMSD->SetSelection( true );
            fFoundVideo = true;
        }
        else if( !fFoundAudio && ( MediaStreamTypeAudio == pMSD->Type() ) )
        {
            pMSD->SetSelection( true );
            fFoundAudio = true;
        }
        else
        {
            pMSD->SetSelection( false );
        }
    }

    if (!fFoundAudio && !fFoundVideo)
    {
        // Return pkS_FALSE here to be consistent with the case where there are no available streams
        pkResult = pkS_FALSE;
        CHUNKMANIFEST_TRACE(("There is no video or audio stream in the available stream list."));
    }

    return (pkResult);
}

/////////////////////////////////////////////////////////////////////////
pkRESULT CChunkManifest::ResolveSparseStreams()
{
    pkRESULT pkResult = pkS_OK;
    MSDList::iterator itMSD;
    uint32 streamID = 1;    // stream IDs start at '1'

    AutoLock al(&m_manifestLock);

    //
    // Find the sparse streams and link them to their parent streams.
    // Discard the ones whose parent is not present.
    //

    itMSD = m_availableStreams.begin();

    while( itMSD != m_availableStreams.end() )
    {
        if( !(*itMSD)->ParentStreamName().empty() )
        {
            MSDList::iterator itParentMSD
                                = std::find_if(
                                        m_availableStreams.begin(),
                                        m_availableStreams.end(),
                                        StreamHasName( (*itMSD)->ParentStreamName() ) );

            if( itParentMSD == m_availableStreams.end() )
            {
                // Remove child streams whose parents are not in the manifest
                TRACE_ERROR(("Parent stream '%ls' not found. Remove child stream '%ls'",
                            (*itMSD)->ParentStreamName().c_str(),
                            (*itMSD)->Name().c_str() ));

                itMSD = m_availableStreams.erase( itMSD );

                continue;
            }

            if( !(*itParentMSD)->ParentStreamName().empty() )
            {
                // According to IIS Smooth Streaming File/Wire Format Specification,
                // a parent stream can't be sparse, therefore it can't have a parent of its own

                TRACE_ERROR(("Sparse streams can't be parent streams: '%ls' parent of '%ls'", (*itParentMSD)->Name().c_str(), (*itMSD)->Name().c_str() ));

                CHECK_PKRESULT_GOTO( pkE_INVALID_FORMAT, exit );
            }

            (*itParentMSD)->AddChildStream( *itMSD );
        }

        // Keep this stream in the list and move to the next

        (*itMSD)->m_dwStreamID = streamID;
        ++streamID;
        ++itMSD;
    }

exit:

    return( pkResult );
}

/////////////////////////////////////////////////////////////////////////
pkRESULT CChunkManifest::ValidateManifest()
{
    pkRESULT pkResult = pkS_OK;
    uint32 cMBRTracks = 0;
    uint16 iMBR = 0;
    CMediaStreamDescription *pMSD = NULL;

    // we'll calculate maximum total bitrate, audio/video/text
    uint32 stream_type_bps[MEDIASTREAMTYPECOUNT] = {0, 0, 0, 0};
    uint32 stream_type_id[MEDIASTREAMTYPECOUNT] = {0, 0, 0, 0};

    AutoLock al(&m_manifestLock);

    pkResult = ResolveSparseStreams();
    CHECK_PKRESULT_GOTO( pkResult, done );

    if (m_availableStreams.size() == 0)
    {
        pkResult = pkS_FALSE;
        goto done;
    }

    for (size_t iStream = 0; iStream < m_availableStreams.size(); iStream++)
    {
        pMSD = m_availableStreams[iStream]; // no AddRef

        cMBRTracks = pMSD->NumberOfAvailableTracks();

        if (cMBRTracks >= MBR_MAX_BITRATECOUNT
            || cMBRTracks == 0 )
        {
            CHECK_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
        }

        // non-sparse streams need to have at least one chunk
        if( !pMSD->HasParent() && (pMSD->GetDVRCount() == 0) )
        {
            CHECK_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
        }

        if (pMSD->Type() < MEDIASTREAMTYPECOUNT)
        {
            uint32 iMBRMax = cMBRTracks - 1;

            uint32 bitrate = MAX(
                                pMSD->TrackWeakPtr( iMBRMax )->NominalBitrate(),
                                pMSD->TrackWeakPtr( iMBRMax )->Bitrate()
                                );

            if (stream_type_bps[pMSD->Type()] < bitrate)
            {
                stream_type_bps[pMSD->Type()] = bitrate;
                stream_type_id[pMSD->Type()] = pMSD->m_dwStreamID;
            }
        }

        // sparse streams may not have any chunks in the manifest, create the chunk buffer
        if ( ( NULL == pMSD->m_pChunkBuffer ) && pMSD->HasParent() )
        {
            static const uint32 DEFAULT_CHUNK_DURATION_SECS = 2;

            pMSD->m_pChunkBuffer = CChunkBuffer::Create(pMSD->m_Tracks.size(), IsLive());
            CHECKNULL_SET_PKRESULT_GOTO( pMSD->m_pChunkBuffer, pkE_OUTOFMEMORY, done );
            
            pMSD->m_pChunkBuffer->SetDefaultChunkDuration(DEFAULT_CHUNK_DURATION_SECS * TIMESCALE_10MHZ);
        }

        CHECKNULL_SET_PKRESULT_GOTO( pMSD->m_pChunkBuffer, pkE_UNEXPECTED, done );

        AutoLock lock(&(pMSD->m_chunkBufferLock));
        
        uint32 defaultDuration = pMSD->m_pChunkBuffer->GetDefaultChunkDuration();

        if (0 == defaultDuration)
        {
            defaultDuration = pMSD->GetChunkDuration(pMSD->m_pChunkBuffer->GetMinIndex());
            pMSD->m_pChunkBuffer->SetDefaultChunkDuration(defaultDuration);
        }

        // Note: Only default chunk sizes are set and *not* sizes for each chunk based on duration and bitrate
        //       because explicit chunk duration is not tracked in the chunk buffer and computing chunk 
        //       duration from start time differences does not work when discontinuities are present.

        for ( iMBR=0; iMBR < cMBRTracks; iMBR++ )
        {
            // set default chunk quality
            pMSD->m_pChunkBuffer->SetDefaultQuality( pMSD->TrackWeakPtr( iMBR ), (iMBR + 1) * 10);

            // set the default chunk size
            if (0 == pMSD->m_pChunkBuffer->GetDefaultChunkSizeInKB( pMSD->TrackWeakPtr( iMBR )))
            {
                double dblSize = (double)pMSD->TrackWeakPtr( iMBR )->Bitrate() * ((double)defaultDuration / TIMESCALE_10MHZ) / (8 * 1024);
                
                if (dblSize > (MBR_MAX_CHUNK_SIZE / 1024) || dblSize > 0xFFFF)
                {
                    CHECK_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
                }
            
                pMSD->m_pChunkBuffer->SetDefaultChunkSizeInKB( pMSD->TrackWeakPtr( iMBR ), (uint16)dblSize);
            }
        }

        pMSD->SetDVRWindowLength(m_llDVRWindowLength);
        ReportEvent_StreamInManifest( pMSD->GetStreamID(),
                                      GetStreamTypeTraits( pMSD->Type() )->pszDefaultName,
                                      pMSD->Name().c_str() );
    }

    // designate video streamId w/max bitrate as primary index to be used
    // for timing purposes (if no video is available, then use audio)
    if (stream_type_id[MediaStreamTypeVideo] > 0)
    {
        m_dwPrimaryStreamId = stream_type_id[MediaStreamTypeVideo];
    }
    else if (stream_type_id[MediaStreamTypeAudio] > 0)
    {
        //TODO: Take into account whether audio stream is enabled
        m_dwPrimaryStreamId = stream_type_id[MediaStreamTypeAudio];

        // Currently SSPK does not support Audio only
        CHECK_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
    }
    else
    {
        m_dwPrimaryStreamId = 0;
    }

    m_nTotalBps = 0;
    for (int i = 0; i < (int)MEDIASTREAMTYPECOUNT; ++i)
    {
        m_nTotalBps += stream_type_bps[i];
    }

    // set the start time and duration based on the chunk list of the primary stream
    UpdateManifestStartDuration();

done:

    return pkResult;
}

void CChunkManifest::UpdateManifestStartDuration()
{
    // set the start time and duration based on the chunk list of the primary stream
    CMediaStreamDescription* pMSD = GetStreamDescriptionById(m_dwPrimaryStreamId);
    if(pMSD)
    {
        m_ullStartTime = NTP_10MHZTOUINT64(pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex()));
        m_llDuration = pMSD->GetChunkEndPosition(pMSD->GetDVRMaxChunkIndex()) - pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex());
    }

    ResolveStartTime();
}

void CChunkManifest::ResolveStartTime()
{
    int64 minStartTime = MAX_TIME64;

    // the minimum start time of video/audio stream is the start time of the presentation
    for (size_t i = 0; i < m_availableStreams.size(); i++)
    {
        CMediaStreamDescription *pMSD = m_availableStreams[i];

        if ( MediaStreamTypeVideo == pMSD->Type() || MediaStreamTypeAudio == pMSD->Type() )
        {
            if( pMSD->GetDVRCount() > 0 )
            {
                uint32 firstIndex = pMSD->GetDVRMinChunkIndex();
                int64 startTime = pMSD->GetChunkStartPosition(firstIndex);
                if( startTime < minStartTime )
                {
                    minStartTime = startTime;
                }
            }
        }
    }

    if ( minStartTime != MAX_TIME64 )
    {
        m_hnsMarkIn = minStartTime;
    }
}

void CChunkManifest::SetManifestIsParsedAndMerged()
{
    for( size_t i = 0; i < m_availableStreams.size(); ++i )
    {
        m_availableStreams[i]->SetManifestIsParsedAndMerged();
    }

    m_fManifestParsedAndMerged = true;
}

// return the duration of the given chunk.
uint32 CChunkManifest::GetChunkDuration(MediaStreamType mType, int32 chunkIndex)
{
    CMediaStreamDescription *pMSD = GetStreamDescriptionByType(mType);

    if (pMSD)
    {
        return pMSD->GetChunkDuration(chunkIndex);
    }

    return 0;
}

pkRESULT CChunkManifest::CreateInstance( _Out_ CChunkManifest** ppChunkManifest )
{
    pkRESULT pkResult = pkS_OK;

    *ppChunkManifest = NULL;

    AutoRefPtr<CChunkManifest> apObj;

    apObj.AdoptRef( NEW_NO_THROW CChunkManifest() );

    if( apObj == NULL )
    {
        pkResult = pkE_OUTOFMEMORY;
        goto exit;
    }

    pkResult = DefaultManifestUrlServices::CreateInstance( apObj->m_apUrlServices.DerefOutPtr() );
    if( pkFAILED(pkResult) )
    {
        goto exit;
    }

    *ppChunkManifest = apObj.HandOffRef();

exit:

    return (pkResult);
}

bool CChunkManifest::VerifyStreamSelection( _In_ const std::vector< AutoRefPtr<IManifestStream> >& proposedSelection )
{
    bool fResult = false;
    int videoStreams = 0;
    int audioStreams = 0;

    AutoLock al(&m_manifestLock);

    // Verify that all the proposed selections are actually in the list of available streams
    for (size_t i = 0; i < proposedSelection.size(); ++i)
    {
        IManifestStream* pStream = proposedSelection[i];

        if( pStream == NULL )
        {
            TRACE_ERROR(( "Error: unexpected null pointer in the list" ));
            goto exit;
        }

        if( FindStreamIndex( pStream ) < 0 )
        {
            TRACE_ERROR(( "Error: Stream [%p] %ls not found", pStream, pStream->Name().c_str() ));
            goto exit;
        }
    }

    // Confirm there are no more than 1 video and audio streams
    for (size_t i = 0; i < proposedSelection.size(); ++i)
    {
        if (MediaStreamTypeVideo == proposedSelection[i]->Type())
        {
            ++videoStreams;
        }
        else if (MediaStreamTypeAudio == proposedSelection[i]->Type())
        {
            ++audioStreams;
        }
    }

    if (1 < audioStreams || 1 < videoStreams)
    {
        TRACE_ERROR(("Error: there are %d video streams and %d audio streams in the selection. There cannot be more than 1 video and 1 audio streams selected.",
            videoStreams, audioStreams));
        goto exit;
    }

    // Confirm that there is at least 1 audio or 1 video stream
    if (0 == audioStreams && 0 == videoStreams)
    {
        TRACE_ERROR(("Error: there is no video and audio streams selected."));
        goto exit;
    }

    // If audio stream is missing, confirm that this is a video only stream
    if (0 == audioStreams)
    {
        bool fFoundAudio
                = RangeHasAnyOf(
                        m_availableStreams.begin(),
                        m_availableStreams.end(),
                        StreamOfType<MediaStreamTypeAudio> );

        if (fFoundAudio)
        {
            TRACE_ERROR(("Error: selection must specify an audio stream for content with audio."));
            goto exit;
        }
    }

    // If video stream is missing, confirm that this is an audio only stream
    if (0 == videoStreams)
    {
        bool fFoundVideo
                = RangeHasAnyOf(
                        m_availableStreams.begin(),
                        m_availableStreams.end(),
                        StreamOfType<MediaStreamTypeVideo> );
        if (fFoundVideo)
        {
            TRACE_ERROR(("Error: selection must specify a video stream for content with video."));
            goto exit;
        }
    }

    // If a child stream is selected, its parent must be selected as well

    if( RangeHasAnyOf( proposedSelection.begin(), proposedSelection.end(), StreamParentNotInList( proposedSelection ) ) )
    {
        TRACE_ERROR(("Error: selection of a child stream must include its parent stream too."));
        goto exit;
    }

    // If we reached this point, everything looks valid
    fResult = true;

exit:

    return fResult;
}

int CChunkManifest::FindStreamIndex( _In_ IManifestStream* pStream )
{
    int ret = -1;

    for( size_t i = 0; i < m_availableStreams.size(); ++i )
    {
        if( m_availableStreams[i] == pStream )
        {
            ret = i;
            break;
        }
    }

    return( ret );
}


void CChunkManifest::FinishStreamSelection(
    _In_ const std::vector< AutoRefPtr<IManifestStream> >& proposedSelection,
    _In_ IStreamsSelectedCallback* pCallback )
{
    StreamSelectedEventArgs args;

    std::vector< bool > updatedStreamsSelections;

    {
        AutoLock al(&m_manifestLock);

        updatedStreamsSelections.assign( m_availableStreams.size(), false );

        // Create a list to update the stream selection flags

        for( size_t i = 0; i < proposedSelection.size(); ++i )
        {
            int iStrm = FindStreamIndex( proposedSelection[i] );
            ASSERT( iStrm >= 0 );
            updatedStreamsSelections[iStrm] = true;
        }

        args.StreamChanges.reserve( m_availableStreams.size() );
        args.Result = pkS_OK;

        // Process the streams to be deselected

        for( size_t i = 0; i < updatedStreamsSelections.size(); ++i )
        {
            if( !updatedStreamsSelections[i] && m_availableStreams[i]->IsSelected() )
            {
                m_availableStreams[i]->SetSelection( false );

                StreamChangedEventArgs streamChangedArgs;
                streamChangedArgs.Action = StreamChangedEventArgs::StreamDeselected;
                streamChangedArgs.pStream = m_availableStreams[i];
                streamChangedArgs.Result = pkS_OK;

                /// TODO: Timestamp of stream is not implemented yet: streamChangedArgs.Timestamp = ?

                args.StreamChanges.push_back( streamChangedArgs );
            }
        }

        // Process the streams to select

        for( size_t i = 0; i < updatedStreamsSelections.size(); ++i )
        {
            if( updatedStreamsSelections[i] && !m_availableStreams[i]->IsSelected() )
            {
                m_availableStreams[i]->SetSelection( true );

                StreamChangedEventArgs streamChangedArgs;
                streamChangedArgs.Action = StreamChangedEventArgs::StreamSelected;
                streamChangedArgs.pStream = m_availableStreams[i];
                streamChangedArgs.Result = pkS_OK;

                /// TODO: Timestamp of stream is not implemented yet: streamChangedArgs.Timestamp = ?

                args.StreamChanges.push_back( streamChangedArgs );
            }
        }

        m_fStreamSelectionInProcess = false;

        args.Result = pkS_OK;
    }

    if (NULL != m_pManifestUpdateManager)
    {
        // Notify CSocketMbr to do the actual switching
        for (size_t i = 0; i < args.StreamChanges.size(); ++i)
        {
            AutoRefPtr<IManifestStream> apParentStream;
            args.StreamChanges[i].pStream->GetParentStream( apParentStream.DerefOutPtr() );

            // do the switching on non-sparse streams only
            if( NULL == apParentStream )
            {
                if (StreamChangedEventArgs::StreamDeselected == args.StreamChanges[i].Action)
                {
                    m_pManifestUpdateManager->DeselectStream(args.StreamChanges[i].pStream);
                }
                else
                {
                    ASSERT(StreamChangedEventArgs::StreamSelected == args.StreamChanges[i].Action);
                    m_pManifestUpdateManager->SelectStream(args.StreamChanges[i].pStream);
                }
            }
        }
    }

    // Make the callback
    if (NULL != pCallback)
    {
        pCallback->StreamSelectedCallback(&args);
    }
}

void CChunkManifest::DumpManifest()
{
#ifdef CHUNKMANIFEST_SPEW
    CHUNKMANIFEST_TRACE(("-----------------Begin Manifest Dump--------------------------"));
    CHUNKMANIFEST_TRACE(("m_dwMajorVersion: %d", m_dwMajorVersion));
    CHUNKMANIFEST_TRACE(("m_dwMinorVersion: %d", m_dwMinorVersion));
    CHUNKMANIFEST_TRACE(("m_dwCombinedVersion: %d", m_dwCombinedVersion));
    CHUNKMANIFEST_TRACE(("m_llTimeScale: %lld", m_llTimeScale));
    CHUNKMANIFEST_TRACE(("m_llDVRWindowLength: %lld", m_llDVRWindowLength));
    CHUNKMANIFEST_TRACE(("m_llDuration: %lld", m_llDuration));
    CHUNKMANIFEST_TRACE(("ManifestURL: %ls", ( m_apUrlServices == NULL ) ? L"<null>" : m_apUrlServices->ManifestUrl().c_str() ));

    for (size_t i = 0; i < m_availableStreams.size(); i++)
    {
        CMediaStreamDescription* pMSD = m_availableStreams[i];

        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_Type: %d", i, pMSD->Type()));
        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_MediaSubType: %d", i, pMSD->m_MediaSubType));
        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_wstrUrl: %ls", i, pMSD->Url().c_str()));
        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_wstrLanguage: %ls", i, pMSD->Language().c_str()));
        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_dwManifestChunkCount: %d", i, pMSD->m_dwManifestChunkCount));
        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_dwStreamID: %d", i, pMSD->m_dwStreamID));
        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_llTimeScale: %lld", i, pMSD->TimeScale()));
        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_dwMaxHeight: %d", i, pMSD->MaxHeight()));
        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_dwMaxWidth: %d", i, pMSD->MaxWidth()));
        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_dwDisplayHeight: %d", i, pMSD->DisplayHeight()));
        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_dwDisplayWidth: %d", i, pMSD->DisplayWidth()));
        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_dwDefaultQL: %d", i, pMSD->m_dwDefaultQL));
        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_bManifestOutput: %s", i, pMSD->m_bManifestOutput?"yes":"no"));
        CHUNKMANIFEST_TRACE(("pMSD[%d]->m_Tracks.size(): %d", i, pMSD->m_Tracks.size()));

        for (uint32 iMBR=0; iMBR<pMSD->m_Tracks.size(); ++iMBR)
        {
            CHUNKMANIFEST_TRACE(("pMSD[%d]->m_Tracks[%d]->ID: %d", i, iMBR, pMSD->m_Tracks[iMBR]->TrackIndex()));
            CHUNKMANIFEST_TRACE(("pMSD[%d]->m_Tracks[%d]->dwBitrate: %d", i, iMBR, pMSD->m_Tracks[iMBR]->Bitrate()));
            CHUNKMANIFEST_TRACE(("pMSD[%d]->m_Tracks[%d]->dwNominalBitrate: %d", i, iMBR, pMSD->m_Tracks[iMBR]->NominalBitrate()));
            CHUNKMANIFEST_TRACE(("pMSD[%d]->m_Tracks[%d]->dwWidth: %d", i, iMBR, pMSD->m_Tracks[iMBR]->MaxWidth()));
            CHUNKMANIFEST_TRACE(("pMSD[%d]->m_Tracks[%d]->dwHeight: %d", i, iMBR, pMSD->m_Tracks[iMBR]->MaxHeight()));
            CHUNKMANIFEST_TRACE(("pMSD[%d]->m_Tracks[%d]->dwHardwareProfile: %d", i, iMBR, pMSD->m_Tracks[iMBR]->HardwareProfile()));
            CHUNKMANIFEST_TRACE(("pMSD[%d]->m_Tracks[%d]->dwNalUnitLength: %d", i, iMBR, pMSD->m_Tracks[iMBR]->NALUnitLength()));
            CHUNKMANIFEST_TRACE(("pMSD[%d]->m_Tracks[%d]->dwCodecBlobSize: %d", i, iMBR, pMSD->m_Tracks[iMBR]->CodecPrivateData().size()));
            CHUNKMANIFEST_TRACE(("pMSD[%d]->m_Tracks[%d]->FourCC:%d", i, iMBR, pMSD->m_Tracks[iMBR]->FourCC()));
        }

        // Dump the chunk list
        {
            char szPrefix[32];
            StringCbPrintfA(szPrefix, sizeof(szPrefix), "pMSD[%d]->m_pChunkBuffer->", i);
            pMSD->m_pChunkBuffer->DumpChunkList(szPrefix);
        }

        CHUNKMANIFEST_TRACE(("m_dwLastChunkDuration: %d", pMSD->m_dwLastChunkDuration));
    }

    CHUNKMANIFEST_TRACE(("\nm_hnsMarkIn: %lld", m_hnsMarkIn));
    CHUNKMANIFEST_TRACE(("m_dwBufferTime: %d", m_dwBufferTime));
    CHUNKMANIFEST_TRACE(("m_nTotalBps: %d", m_nTotalBps));
    CHUNKMANIFEST_TRACE(("m_fIsLive: %s", m_fIsLive?"yes":"no"));
    CHUNKMANIFEST_TRACE(("m_dwLookAheadCount: %d", m_dwLookAheadCount));
    CHUNKMANIFEST_TRACE(("m_wstrTimeScaleZeroPoint: %ls", m_wstrTimeScaleZeroPoint.c_str()));
    CHUNKMANIFEST_TRACE(("-----------------End Manifest Dump----------------------------"));
#endif
}
