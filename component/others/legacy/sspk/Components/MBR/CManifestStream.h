///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include <IManifestStream.h>

#include <RefCountObj.h>

////////////////////////////////////////////////////////////////////////////////
//
// CManifestStream - basic IManifestStream implementation
//
////////////////////////////////////////////////////////////////////////////////

class CManifestStream : public IManifestStream
{
protected:

    CManifestStream();

public:

    // Creates the default CManifestStream
    static pkRESULT CreateInstance(_Out_ CManifestStream** ppManifestStream);

    //
    // Overloads the attribute accessors to allow writing
    //
    int64& TimeScale() { return (m_llTimeScale); };
    std::wstring& Language() { return (m_wstrLanguage); };
    uint32& MaxWidth() { return (m_dwMaxWidth); };
    uint32& MaxHeight() { return (m_dwMaxHeight); };
    uint32& DisplayWidth() { return (m_dwDisplayWidth); };
    uint32& DisplayHeight() { return (m_dwDisplayHeight); };
    std::wstring& Url() { return (m_wstrUrl); };
    std::wstring& Name() { return (m_wstrName); };
    MediaStreamType& Type() { return (m_Type); };
    std::wstring& SubType() { return (m_SubType); };
    //
    // IManifestStream - implementation
    //

    virtual void AddRef();

    virtual void Release();

    bool GetAttribute(_In_ const std::wstring& name, _Out_ std::wstring* pValue) const;

    pkRESULT GetAvailableTracks(_Out_ std::vector< AutoRefPtr<IManifestTrack> >* pAvailableTracks);

    pkRESULT GetSelectedTracks(_Out_ std::vector< AutoRefPtr<IManifestTrack> >* pSelectedTracks);

    pkRESULT RestrictTracks(_In_ const std::vector< AutoRefPtr<IManifestTrack> >& restrictedTracks);

    pkRESULT SelectTracks(_In_ const std::vector< AutoRefPtr<IManifestTrack> >& selectedTracks);

    void GetChildStreams(_Out_ std::vector< AutoRefPtr<IManifestStream> >* pChildStreams);

    void GetParentStream(_Deref_out_opt_ IManifestStream ** ppParentStream);

    void SetFlushCallback(_In_opt_ IFlushCallback* pCallback = NULL);

    ChunkIterator GetIterator( _In_ int64_t minTime, _In_ int64_t time );

    ChunkIterator GetFirstInCurrentChunkList();

    ChunkIterator GetLastInCurrentChunkList();

    pkRESULT TryGetChunkInfo( _In_ const ChunkIterator& itChunk, _Out_ CHUNK_INFO* pChunk );

    pkRESULT GetChunkInfoAsync( _In_ const ChunkIterator& itChunk, _In_ IChunkInfoCallback* pCallback );

    pkRESULT AbortChunkInfoAsync();

    pkRESULT DownloadFragmentAsync( _In_ const ChunkIterator& itChunk, _In_ IManifestTrack* pTrack, _In_ size_t cbMaxBufferLength, _In_ IFragmentCallback* pCallback );

    //
    // Other public methods
    //

    void InnerAddRef();

    void InnerRelease();

    void AddChildStream( _In_ CManifestStream* pChild );

    std::wstring& ParentStreamName()    { return (m_wstrParentStreamName); }

    bool HasParent() const              { return( m_apParentStreamConnection != NULL ); }

    bool HasChildStreams() const        { return( !m_childStreams.empty() ); }

    pkRESULT SetAttribute(_In_ const std::wstring& name, _In_ const std::wstring& value);

protected:

    int32 m_cExternalRefs;
    int32 m_cTotalRefs;

    std::map<std::wstring,std::wstring> m_attributes;
    std::vector< AutoRefPtr<IManifestTrack> > m_selectedTracks;
    std::vector< AutoRefPtr<CManifestStream> > m_childStreams;
    std::wstring m_wstrParentStreamName;
    AutoRefPtr< IElementToContainerConnection<IManifestStream> > m_apParentStreamConnection;
    CElementToContainerConnection< IManifestStream, CManifestStream > m_ElementConnection;

    virtual void TearDownCircularReferences() {};
};

