///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <pkExecutive.h>

#include "CManifestStream.h"

////////////////////////////////////////////////////////////////////////////////
//
// The implemenation of CManifestStream
//
////////////////////////////////////////////////////////////////////////////////

CManifestStream::CManifestStream()
    : m_cExternalRefs(1)
    , m_cTotalRefs(1)
{
    m_ElementConnection.Initialize( this );
}

void CManifestStream::AddRef()
{
    // AddRef adds one external reference.

    InnerAddRef();
    Executive_InterlockedIncrement( &m_cExternalRefs );
}

void CManifestStream::Release()
{
    // Release subtracts one external reference.
    // When all external references go away, the object tears down
    // its relationship with contained tracks and child streams,
    // but it doesn't get deleted until all references (internal and
    // external) go away. This prevents a race condition in which
    // a different thread may acquire a pointer to this object
    // after it already decided to delete itself, in which case
    // that thread would get a pointer to invalid memory.

    if( Executive_InterlockedDecrement( &m_cExternalRefs ) == 0 )
    {
        TearDownCircularReferences();
    }

    InnerRelease();
}

bool CManifestStream::GetAttribute(
    _In_ const std::wstring& name,
    _Out_ std::wstring* pValue ) const
{
    bool fSuccess = false;
    pkASSERT(NULL != pValue);
    pValue->clear();

    std::map<std::wstring, std::wstring>::const_iterator it = m_attributes.find( name );

    if (it != m_attributes.end())
    {
        *pValue = it->second;
        fSuccess = true;
    }

    return (fSuccess);
}

pkRESULT CManifestStream::GetAvailableTracks( _Out_ std::vector< AutoRefPtr<IManifestTrack> >* pAvailableTracks )
{
    pkRESULT pkResult = pkE_NOTIMPL;

    /// This should be never called at this level. The actual implemenation class will implement this
    pkASSERT( false );

    return (pkResult);
}

pkRESULT CManifestStream::GetSelectedTracks( _Out_ std::vector< AutoRefPtr<IManifestTrack> >* pSelectedTracks )
{
    pkRESULT pkResult = pkE_NOTIMPL;

    /// This should be never called at this level. The actual implemenation class will implement this
    pkASSERT( false );

    return (pkResult);
}

pkRESULT CManifestStream::RestrictTracks( _In_ const std::vector< AutoRefPtr<IManifestTrack> >& restrictedTracks )
{
    pkRESULT pkResult = pkE_NOTIMPL;

    /// This should be never called at this level. The actual implemenation class will implement this
    pkASSERT( false );

    return (pkResult);
}

pkRESULT CManifestStream::SelectTracks( _In_ const std::vector< AutoRefPtr<IManifestTrack> >& selectedTracks )
{
    pkRESULT pkResult = pkE_NOTIMPL;

    /// This should be never called at this level. The actual implemenation class will implement this
    pkASSERT( false );

    return (pkResult);
}

void CManifestStream::GetChildStreams( _Out_ std::vector< AutoRefPtr<IManifestStream> >* pChildStreams )
{
    /// This should be never called at this level. The actual implemenation class will implement this
    pkASSERT( false );
}

void CManifestStream::GetParentStream( _Deref_out_opt_ IManifestStream ** ppParentStream )
{
    /// This should be never called at this level. The actual implemenation class will implement this
    pkASSERT( false );
}

void CManifestStream::SetFlushCallback( _In_opt_ IFlushCallback* pCallback )
{
    /// TODO: implement this.
    pkASSERT( false );
}

ChunkIterator CManifestStream::GetIterator( _In_ int64_t minTime, _In_ int64_t time )
{
    /// This should be never called at this level. The actual implemenation class will implement this
    pkASSERT( false );
    return( ChunkIterator() );
}

ChunkIterator CManifestStream::GetFirstInCurrentChunkList()
{
    /// This should be never called at this level. The actual implemenation class will implement this
    pkASSERT( false );
    return( ChunkIterator() );
}

ChunkIterator CManifestStream::GetLastInCurrentChunkList()
{
    /// This should be never called at this level. The actual implemenation class will implement this
    pkASSERT( false );
    return( ChunkIterator() );
}

pkRESULT CManifestStream::TryGetChunkInfo( _In_ const ChunkIterator& itChunk, _Out_ CHUNK_INFO* pChunk )
{
    /// This should be never called at this level. The actual implemenation class will implement this
    pkASSERT( false );
    return( pkE_NOTIMPL );
}

pkRESULT CManifestStream::GetChunkInfoAsync( _In_ const ChunkIterator& itChunk, _In_ IChunkInfoCallback* pCallback )
{
    /// This should be never called at this level. The actual implemenation class will implement this
    pkASSERT( false );
    return( pkE_NOTIMPL );
}

pkRESULT CManifestStream::AbortChunkInfoAsync()
{
    /// This should be never called at this level. The actual implemenation class will implement this
    pkASSERT( false );
    return( pkE_NOTIMPL );
}

pkRESULT CManifestStream::DownloadFragmentAsync(
    _In_ const ChunkIterator& itChunk,
    _In_ IManifestTrack* pTrack,
    _In_ size_t cbMaxBufferLength,
    _In_ IFragmentCallback* pCallback )
{
    /// This should be never called at this level. The actual implemenation class will implement this
    pkASSERT( false );
    return( pkE_NOTIMPL );
}

void CManifestStream::InnerAddRef()
{
    Executive_InterlockedIncrement( &m_cTotalRefs );
}

void CManifestStream::InnerRelease()
{
    if( Executive_InterlockedDecrement( &m_cTotalRefs ) == 0 )
    {
        delete this;
    }
}

void CManifestStream::AddChildStream( _In_ CManifestStream* pChild )
{
    pChild->m_apParentStreamConnection.Set( &m_ElementConnection );
    m_childStreams.push_back( AutoRefPtr<CManifestStream>( pChild ) );
}

pkRESULT CManifestStream::SetAttribute(
    _In_ const std::wstring& name,
    _In_ const std::wstring& value )
{
    pkRESULT pkResult = pkS_OK;

    if (m_attributes.end() != m_attributes.find( name ))
    {
        pkResult = pkE_INVALIDARG;
    }
    else
    {
        m_attributes[name] = value;
    }

    return (pkResult);
}

pkRESULT CManifestStream::CreateInstance( _Out_ CManifestStream** ppManifestStream )
{
    *ppManifestStream = NEW_NO_THROW CManifestStream();

    return (NULL == (*ppManifestStream)) ? pkE_OUTOFMEMORY : pkS_OK;
}

