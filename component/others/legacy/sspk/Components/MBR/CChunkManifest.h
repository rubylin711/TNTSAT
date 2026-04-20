///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CMediaStreamDescription.h"
#include "CManifest.h"
#include "IManifestUpdate.h"
#include "IManifestUpdateManager.h"
#include <map>
#include <vector>

namespace MBR
{

class CChunkManifest : public CManifest
{
    typedef std::map<uint32, AutoRefPtr<CMediaStreamDescription> > MSDTypeMap;

    CChunkManifest()
        : m_dwCombinedVersion( 0 )
        , m_hnsMarkIn( 0 )
        , m_dwBufferTime( 0 )
        , m_nTotalBps( 0 )
        , m_dwPrimaryStreamId( 0 )
        , m_fManifestParsedAndMerged( false )
        , m_fStreamSelectionInProcess( false )
        , m_pManifestUpdateManager( NULL )
    {
        m_dwMajorVersion = 0;
        m_dwMinorVersion = 0;
        m_llTimeScale = MBR_DEFAULT_TIMESCALE;
        m_llDuration = 0;
        m_fIsLive = false;
        m_dwLookAheadCount = 0;
        m_llDVRWindowLength = 0;
        m_ullStartTime = 0;
    }

public:

    typedef std::vector< AutoRefPtr<CMediaStreamDescription> > MSDList;

    // Creates the default CChunkManifest
    static pkRESULT CreateInstance(_Out_ CChunkManifest** ppChunkManifest);

    ~CChunkManifest()
    {
        m_availableStreams.clear();
    }

    // CManifest methods override
    virtual pkRESULT GetAvailableStreams(_Out_ std::vector< AutoRefPtr<IManifestStream> >* pAvailableStreams);
    virtual pkRESULT GetSelectedStreams(_Out_ std::vector< AutoRefPtr<IManifestStream> >* pSelectedStreams);
    virtual pkRESULT SelectStreamsAsync(_In_ IStreamsSelectedCallback* pCallback, _In_ std::vector< AutoRefPtr<IManifestStream> >& selectedStreams);
    virtual EManifestType Type() const;

    // Other public methods
    CMediaStreamDescription* AddStream();
    CMediaStreamDescription* GetLastStream() { return (m_availableStreams.back()); }
    uint32                   GetStreamCount() { return m_availableStreams.size(); }

    CMediaStreamDescription* GetStreamDescriptionByType( MediaStreamType mType, MediaStreamSubType mSubType = MediaStreamSubTypeUnknown );
    CMediaStreamDescription* GetStreamDescriptionById( uint32 dwStreamId );
    CMediaStreamDescription* GetStreamDescriptionByStream(IManifestStream* pStream);
    CMediaStreamDescription* GetPrimaryStreamDescription() { return GetStreamDescriptionById( m_dwPrimaryStreamId ); }
    uint32                   GetPrimaryStreamId() const { return m_dwPrimaryStreamId; }
    uint32                   GetActiveAudioId();

    pkRESULT                 GetChunkURL( uint32 dwStreamId, int32 chunkIndex, uint32 iMBR, _Out_ std::wstring& wstrURL, bool isTrick = false, bool isChunkInfo = false );
    pkRESULT                 GetLatestChunkInfoURL( uint32 dwStreamId, uint32 iMBR, _Out_ std::wstring& wstrURL );
    pkRESULT                 GetSegmentedManifestURL(_In_ int64 pos, _Out_ std::wstring* wstrURL);
    void                     UpdateManifestStartDuration();
    pkRESULT                 ResolveSparseStreams();
    pkRESULT                 ValidateManifest();
    pkRESULT                 SelectInitialStreams();
    void                     SetManifestIsParsedAndMerged();
    uint32                   GetChunkDuration(MediaStreamType mType, int32 chunkIndex);

    void                     DumpManifest();

    void                     SetManifestUpdateManager(IManifestUpdateManager* pManifestUpdateManager) { m_pManifestUpdateManager = pManifestUpdateManager; };

private:

    bool                     VerifyStreamSelection( _In_ const std::vector< AutoRefPtr<IManifestStream> >& proposedSelection );
    void                     FinishStreamSelection( _In_ const std::vector< AutoRefPtr<IManifestStream> >& proposedSelection, _In_ IStreamsSelectedCallback* pCallback );
    int                      FindStreamIndex( _In_ IManifestStream* pStream );
    void                     ResolveStartTime();

public:

    uint32                   m_dwCombinedVersion;
    int64                    m_hnsMarkIn;
    uint32                   m_dwBufferTime;          // in milliseconds
    uint32                   m_nTotalBps;
    uint32                   m_dwPrimaryStreamId;

    // Map caching streams pointers by StreamType (no ref count)
    // If stream list is modified, make sure to clear this map
    MSDTypeMap               m_StreamTypeCache;

    std::wstring             m_wstrTimeScaleZeroPoint;

    MSDList                  m_availableStreams;

    bool                     ManifestParsedAndMerged()   { return (m_fManifestParsedAndMerged); }
    Lockable&                ManifestLock()              { return (m_manifestLock); }

    void SetManifestUrl( _In_ const wchar_t* pszUrl )           { m_apUrlServices->SetManifestUrl( pszUrl ); }

    bool                IsSegmented() const                 { return m_apUrlServices->IsSegmented(); }
    uint64              ReferenceSegmentStartTime() const   { return m_apUrlServices->ReferenceSegmentStartTime(); }
    uint64              SegmentDuration() const             { return m_apUrlServices->SegmentDuration(); }
    const std::wstring& SegmentUrlTemplate() const          { return m_apUrlServices->SegmentUrlTemplate(); }

    void SetSegmented( _In_ bool f )                            { m_apUrlServices->SetSegmented( f ); }
    void SetReferenceSegmentStartTime( _In_ uint64 v )          { m_apUrlServices->SetReferenceSegmentStartTime( v ); }
    void SetSegmentDuration( _In_ uint64 v )                    { m_apUrlServices->SetSegmentDuration( v ); }
    void SetSegmentUrlTemplate( _In_ const std::wstring& v )    { m_apUrlServices->SetSegmentUrlTemplate( v ); }

private:

    friend pkRESULT CreateInstance(_Out_ CChunkManifest** ppChunkManifest);

    bool                    m_fManifestParsedAndMerged;
    bool                    m_fStreamSelectionInProcess;
    Lockable                m_streamSelectionLock;
    Lockable                m_manifestLock;
    IManifestUpdateManager* m_pManifestUpdateManager;
    AutoRefPtr<IManifestUrlServices> m_apUrlServices;
};

} // namespace MBR
