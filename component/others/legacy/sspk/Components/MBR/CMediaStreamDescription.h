///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AutoLock.h"
#include "SSPKTimeSpan.h"
#include "CManifestTrack.h"
#include "CManifestChunk.h"
#include "CChunkBuffer.h"
#include "CStreamInfo.h"
#include "MarshallingUtils.h"
#include "MediaStreamDefinitions.h"
#include "wmaudiofmt.h"
#include "CManifestStream.h"
#include <map>
#include <vector>

#include "MbrLocalInterfaces.h"

#include <AutoRefPtr.h>

namespace MBR
{
// ===============================================================================================================
// MBR constants
// ===============================================================================================================

#define MBR_MAX_CODEC_BLOB_LEN        2048
#define MBR_MAX_FOURCC_STR_LEN        5
#define MBR_DEFAULT_TIMESCALE         TIMESCALE_10MHZ // Hundreds of nanoseconds

#define MBR_MAX_CHUNK_SIZE            100000000       // max CHUNK size is 100M

#define MBR_OBFUSCATIONSEED_LEN       16

#define MBR_MAX_BITRATECOUNT          256
#define MBR_MAX_STREAMCOUNT           64

#define MBR_INVALID_INDEX             ((uint32)-1)

// ===============================================================================================================
// ===============================================================================================================

class CMediaStreamAttributes
{
    typedef std::map<std::wstring,std::wstring> AttributeMap;

public:
    uint32 GetAttributeCount() { return m_AttributeMap.size(); }
    pkRESULT AddAttribute(const WCHAR* pAttrName, std::wstring& attrValue) { m_AttributeMap[std::wstring(pAttrName)] = attrValue; return pkS_OK; }
    pkRESULT GetAttributeByName(const WCHAR *pAttrName, std::wstring& wstrValue);

private:
    AttributeMap m_AttributeMap;
};

class IQualityLevelComparer
{
public:
    virtual ~IQualityLevelComparer() {};
    virtual int Compare(
                _In_ CManifestTrack* pt1,
                _In_ CManifestTrack* pt2 ) = 0;
};

class CBitrateQualityLevelComparer : public IQualityLevelComparer
{
public:
    __override int Compare(
                _In_ CManifestTrack* pt1,
                _In_ CManifestTrack* pt2 )
    {
        return (int)pt1->Bitrate() - (int)pt2->Bitrate();
    }
};

/// <summary>
/// CMediaPosition combines the chunk index with a presentation time stamp to
/// allow moving the chunk index up and down by a fraction of the chunks duration,
/// which is required to ensure the specified rate during trick mode.
/// </summary>
class CMediaPosition
{
public:

    CMediaPosition(
        _In_ int64 hnsTime = 0,
        _In_ int32 chunkIndex = 0 )
        : _hnsTime( hnsTime )
        , _chunkIndex( chunkIndex )
    {
    }

    int64 Time_hns() const      { return( _hnsTime ); }
    int32 ChunkIndex() const    { return( _chunkIndex ); }

private:

    int64 _hnsTime;
    int32 _chunkIndex;
};

struct SFragmentMetadata
{
    CManifestTrack* pTrack;
    uint16 chunkSizeInKB;
    uint16 quality;
    AutoRefPtr<IRefBuffer> apFragmentData;

    SFragmentMetadata(CManifestTrack* track)
        : pTrack(track)
        , chunkSizeInKB(0)
        , quality(0)
    {
    };
};

struct SChunkInfo
{
    int64 timestamp; 
    std::vector<SFragmentMetadata> FragmentMetadata;

    SChunkInfo(int64 pos)
        : timestamp(pos)
    {
    }
};

enum EQualityDirection
{
    eQualityDirection_None,
    eQualityDirection_Down,
    eQualityDirection_Up
};

// CMediaStreamDescription - Manages information for a stream
class CMediaStreamDescription
    : public CManifestStream
    , public ChunkIterator::ContextAccessor
    , public IInternalFragmentCache
{
    friend class CManifestParsingCallback;
    friend class CChunkManifest;
    friend class CHeuristicsMBR;

public:

    typedef std::vector< AutoRefPtr<CManifestTrack> > TrackVector;
    typedef std::vector< SChunkInfo > ChunkInfoVector;

protected:
    CMediaStreamDescription()
        : m_MediaSubType(MediaStreamSubTypeUnknown)
        , m_dwManifestChunkCount(0)
        , m_dwDefaultQL(INVALID_UINT32)
        , m_bManifestOutput(false)
        , m_bIsSelected(false)
        , m_pQLComparer(NULL)
        , m_iFirstSelectedTrack( 0 )
        , m_iLastSelectedTrack( 0 )
        , m_pChunkBuffer(NULL)
        , m_fManifestIsParsedAndMerged(false)
        , m_lookAheadCount(0)
        , m_dwLastChunkDuration(0)
        , m_DVRWindowLength(0)
        , m_DVRMinIndex(-1)
        , m_pCurrentChunkRequestCallback( NULL )
    {
        m_llTimeScale = MBR_DEFAULT_TIMESCALE;
        m_dwMaxHeight = 0;
        m_dwMaxWidth = 0;
        m_dwDisplayHeight = 0;
        m_dwDisplayWidth = 0;
        m_Type = MediaStreamTypeUnknown;
    }

    __override void TearDownCircularReferences();

public:

    // Creates the default CMediaStreamDescription
    static pkRESULT CreateInstance( _Out_ CMediaStreamDescription** ppMediaStreamDescription );

    ~CMediaStreamDescription()
    {
        SAFE_DELETE(m_pChunkBuffer);
        SAFE_DELETE(m_pQLComparer);

        m_Tracks.clear();
    }

    pkRESULT               SetSubType(const WCHAR* wszSubtype);
    MediaStreamSubType     GetSubType() const { return m_MediaSubType; }
    uint32                 GetStreamID() const { return m_dwStreamID; }


    int64                  GetChunkStartPosition( int32 chunkIndex );
    int64                  GetChunkEndPosition( int32 chunkIndex );
    uint32                 GetChunkDuration( int32 chunkIndex );
    uint32                 GetChunkSizeInKB(uint32 dwMBR, int32 chunkIndex);
    void                   SetLastChunkDuration( _In_ uint32 dwLastChunkDuration);
    int64                  GetChunkTimeScaleStartPosition( int32 chunkIndex );

    pkRESULT               FindPositionByTime(
                                _In_ int64 hnsTime,
                                _In_ bool floorIt,
                                _Out_ CMediaPosition* pPos );

    uint32                 GetFourCC(uint32 nQualityLevel);
    uint32                 GetHeight(uint32 nQualityLevel);
    uint32                 GetWidth(uint32 nQualityLevel);
    byte*                  GetCodecBlob(uint32 nQualityLevel, uint32* pnBlobLen);
    uint32                 GetDefaultQualityLevel();
    void                   SetComparer(IQualityLevelComparer *pComparer);

    CChunkInfo*            AddChunk(_In_ int64 pos, _In_opt_ EBufferDirection direction = eBufferDirection_Forward);
    bool                   AddChunks(_In_ const ChunkInfoVector& chunkInfo, _In_opt_ EBufferDirection direction = eBufferDirection_Forward);
    void                   AddSparseChildChunkInfo( _In_ const std::string& strStreamName, _In_ int64 chunkTime );
    void                   SetLookAheadCount(int32 lookAheadCount);
    int32                  GetDVRCount();
    TimeSpan_hns           TimeRemainingInDVR();
    int32                  GetDVRMinChunkIndex();
    int32                  GetDVRMaxChunkIndex();
    bool                   SetDVRMinTime(_In_ TimeSpan_hns time);
    bool                   ContainsChunkIndex( int32 chunkIndex );
    bool                   ChunkListContainsTime(TimeSpan_hns time);
    uint32                 GetNominalBitrate(uint32 nQualityLevel);
    pkRESULT               AddTrack( _Deref_out_ CManifestTrack** ppTrack );
    void                   SortQualityLevels();
    void                   SetManifestIsParsedAndMerged();
    void                   SetDVRWindowLength( _In_ int64 lenSec );

    // test hook to setup the chunklist buffer, should only be used by unit tests
    void                   UnitTest_SetChunkBuffer( _In_ CChunkBuffer* pChunkBuffer );

    //
    // CManifestStream implementation
    //

    __override
    pkRESULT GetAvailableTracks(_Out_ std::vector< AutoRefPtr<IManifestTrack> >* pAvailableTracks);

    __override
    pkRESULT GetSelectedTracks(_Out_ std::vector< AutoRefPtr<IManifestTrack> >* pSelectedTracks);

    __override
    pkRESULT RestrictTracks(_In_ const std::vector< AutoRefPtr<IManifestTrack> >& restrictedTracks);

    __override
    pkRESULT SelectTracks(_In_ const std::vector< AutoRefPtr<IManifestTrack> >& selectedTracks);

    __override
    void GetChildStreams( _Out_ std::vector< AutoRefPtr<IManifestStream> >* pChildStreams );

    __override
    void GetParentStream(_Deref_out_opt_ IManifestStream ** ppParentStream);

    __override
    ChunkIterator GetIterator( _In_ int64_t minTime, _In_ int64_t time );

    __override
    ChunkIterator GetFirstInCurrentChunkList();

    __override
    ChunkIterator GetLastInCurrentChunkList();

    __override
    pkRESULT TryGetChunkInfo(
                _In_ const ChunkIterator& itChunk,
                _Out_ CHUNK_INFO* pChunk );

    __override
    pkRESULT GetChunkInfoAsync(
                _In_ const ChunkIterator& itChunk,
                _In_ IChunkInfoCallback* pCallback );

    __override
    pkRESULT AbortChunkInfoAsync();

    __override
    pkRESULT DownloadFragmentAsync(
                _In_ const ChunkIterator& itChunk,
                _In_ IManifestTrack* pTrack,
                _In_ size_t cbMaxBufferLength,
                _In_ IFragmentCallback* pCallback );

    //
    // IInternalFragmentCache
    //

    __override
    const std::wstring& BaseUrl() { return( Url() ); }

    __override
    pkRESULT TryGetFragment(
                _In_ const ChunkIterator& itChunk,
                _In_ uint32 trackIndex,
                _Out_ CHUNK_INFO* pChunkInfo,
                _Deref_out_opt_ IRefBuffer** ppFragmentData );

    //
    // Implementation
    //

    struct Range
    {
        uint32 _min;
        uint32 _max;
    };

    Range GetBitrateRangeOfSelectedTracks();

    Range GetQualityRangeOfSelectedTracks();

    uint32 GetIndexOfLastSelectedTrack();

    uint32 GetIndexOfFirstSelectedTrack();

    uint32 GetSelectedTrackWithNearestQuality( _In_ uint32 iChunk, _In_ uint32 quality, _In_ uint32 maxBitrate );

    uint32 GetSelectedNextQualityIndex( _In_ uint32 quality, _In_ EQualityDirection direction );

    CManifestTrack* TrackWeakPtr( size_t iTrack ) const
    {
        return( ( iTrack < m_Tracks.size() )
                    ? (CManifestTrack*)m_Tracks[iTrack]
                    : NULL );
    }

    uint32 NumberOfAvailableTracks() const
    {
        return( m_Tracks.size() );
    }

    void SetManifestUrlServices( _In_ IManifestUrlServices* pObj )
    {
        m_apUrlServices.Set( pObj );
    }

    bool IsSelected() const     { return( m_bIsSelected ); }

    void SetSelection( bool b ) { m_bIsSelected = b; }

private:

    pkRESULT _ValidateChunkIterator( _In_ const ChunkIterator& itChunk );
    void CheckChunkInfoRequest();
    void UpdateDVRMinTime();

private:

    void                   SnapChunkIndexToCurrentRange( _Inout_ int32* pChunkIndex );

    MediaStreamSubType     m_MediaSubType;
    uint32                 m_dwManifestChunkCount;
    uint32                 m_dwStreamID;
    uint32                 m_dwDefaultQL;
    bool                   m_bManifestOutput;
    bool                   m_bIsSelected;
    IQualityLevelComparer* m_pQLComparer;

    TrackVector            m_Tracks;
    uint32                 m_iFirstSelectedTrack;
    uint32                 m_iLastSelectedTrack;

    CMediaStreamAttributes m_StreamAttributes;
    CChunkBuffer*          m_pChunkBuffer;
    Lockable               m_chunkBufferLock;
    Lockable               m_TracksLock;
    bool                   m_fManifestIsParsedAndMerged;

    int32                  m_lookAheadCount;
    uint32                 m_dwLastChunkDuration; // in timescale unit
    int64                  m_DVRWindowLength;     // in timescale unit
    int32                  m_DVRMinIndex;

private:

    AutoRefPtr<IManifestUrlServices>     m_apUrlServices;
    AutoRefPtr<IInternalFragmentFetcher> m_apFragmentFetcher;

    IChunkInfoCallback*     m_pCurrentChunkRequestCallback;
    ChunkIterator           m_itCurrentChunkRequest;
};
} // namespace MBR
