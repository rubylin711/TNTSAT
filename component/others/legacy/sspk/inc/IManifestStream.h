///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// <summary>
///     IManifestStream.h
///     Provides interface expose the stream object in the client manifest to the application
/// </summary>

#include "IManifestTrack.h"
#include "AutoRefPtr.h"
#include "MediaStreamType.h"
#include "ManifestChunk.h"

#include <string>
#include <vector>

// Class receiving the events should implement this interface
class IFlushCallback
{
public:
    /// <summary>
    /// The actual callback for the Flush event
    /// </summary>
    virtual void FlushEventCallback() = 0;

protected:
    virtual ~IFlushCallback() {};
};

class IManifestStream
{
public:
    /// <summary>
    /// Increment the ref count
    /// </summary>
    virtual void AddRef() = 0;

    /// <summary>
    /// Decrement the ref count
    /// </summary>
    virtual void Release() = 0;
 
    /// <summary>
    /// Get the stream level attributes from the manifest
    /// </summary>
    /// <param name="name">The name of the attribute</param>
    /// <param name="pValue">The value of the attribute</param>
    /// <return> Indicates whether the attribute exists. </return>
    virtual bool GetAttribute(_In_ const std::wstring& name, _Out_ std::wstring* pValue) const = 0;
 
    /// <summary>
    /// Get the available tracks in the current stream
    /// </summary>
    /// <param name="pAvailableTracks">The list of available tracks in the stream</param>
    virtual pkRESULT GetAvailableTracks(_Out_ std::vector< AutoRefPtr<IManifestTrack> >* pAvailableTracks) = 0;
 
    /// <summary>
    /// Get the selected tracks in the current mainfest
    /// </summary>
    /// <param name="pSelectedTracks">The list of selected tracks in the stream</param>
    virtual pkRESULT GetSelectedTracks(_Out_ std::vector< AutoRefPtr<IManifestTrack> >* pSelectedTracks) = 0;

    /// <summary>
    /// Restrict the tracks to a subset of all the available tracks.
    /// This API should be called on Manifest Ready event and it will change AvailableTracks
    /// </summary>
    /// <param name="restrictedTracks">The list of tracks to keep available in the stream. Tracks not in the list will be removed.</param>
    virtual pkRESULT RestrictTracks(_In_ const std::vector< AutoRefPtr<IManifestTrack> >& restrictedTracks) = 0;

    /// <summary>
    /// Select the tracks in a subset of all the available tracks.
    /// This API will change SelectedTracks
    /// </summary>
    /// <param name="selectedTracks">The list of selected tracks in the stream</param>
    virtual pkRESULT SelectTracks(_In_ const std::vector< AutoRefPtr<IManifestTrack> >& selectedTracks) = 0;

    /// <summary>
    /// Get the child (sparse) streams of this stream
    /// </summary>
    /// <param name="pChildStreams">The list of sparse streams that are the children of this stream</param>
    virtual void GetChildStreams(_Out_ std::vector< AutoRefPtr<IManifestStream> >* pChildStreams) = 0;

    /// <summary>
    /// Get the parent stream of this stream. This stream is typically a sparse stream
    /// </summary>
    /// <param name="ppParentStream">The parent stream of this stream</param>
    virtual void GetParentStream(_Deref_out_opt_ IManifestStream ** ppParentStream) = 0;

    /// <summary>
    /// Set or reset the event callback indicating the stream is flushed
    /// </summary>
    /// <param name="pCallback">callback interface, default is null</param>
    /// <returns>void</returns>
    virtual void SetFlushCallback(_In_opt_ IFlushCallback* pCallback = NULL) = 0;

    /// <summary>
    /// Returns a iterator to traverse the chunks in this stream.
    /// </summary>
    /// <param name="minTime">value in timescale units to how far back in the timeline the iterator can go</param>
    /// <param name="time">time the iterator is to start at, in timescale units</param>
    /// <returns>A chunk iterator at the given time position</returns>
    virtual ChunkIterator GetIterator( _In_ int64_t minTime, _In_ int64_t time ) = 0;

    /// <summary>
    /// Returns iterator for the first chunk currently in the DVR window of this stream.
    /// </summary>
    /// <returns>The chunk iterator</returns>
    virtual ChunkIterator GetFirstInCurrentChunkList() = 0;

    /// <summary>
    /// Returns iterator for the last chunk currently in the DVR window of this stream.
    /// </summary>
    /// <returns>The chunk iterator</returns>
    virtual ChunkIterator GetLastInCurrentChunkList() = 0;

    /// <summary>
    /// Tries to get the chunk information from the local cache. If the information is not available in
    /// the cache, the method fails and in that case the application can try to get the chunk asynchronously
    /// with IManifestStream::GetChunkInfoAsync.
    /// </summary>
    /// <param name="itChunk">iterator to the desired chunk</param>
    /// <param name="pChunk">pointer to struct to receive the information about the chunk</param>
    /// <returns>
    /// <para>pkS_OK: Success</para>
    /// <para>pkE_DATA_NOT_READY: the chunk is not available in the in the local cache.</para>
    /// <para>pkE_NO_MORE_ITEMS: the iterator reached the end of the stream</para>
    /// <para>pkE_INVALIDARG: invalid iterator</para>
    /// </returns>
    virtual pkRESULT TryGetChunkInfo( _In_ const ChunkIterator& itChunk, _Out_ CHUNK_INFO* pChunk ) = 0;

    /// <summary>
    /// Gets the chunk information asynchronously.
    /// </summary>
    /// <param name="itChunk">iterator to the desired chunk</param>
    /// <param name="pCallback">pointer to a callback provided by the application that will be called once the chunk information becomes available</param>
    /// <returns>
    /// <para>pkS_OK: the operation will complete asynchronously through the callback</para>
    /// <para>pkE_SINGLE_INSTANCE_OP: there's an instance of this operation already in progress, and it must complete before another can start</para>
    /// </returns>
    virtual pkRESULT GetChunkInfoAsync( _In_ const ChunkIterator& itChunk, _In_ IChunkInfoCallback* pCallback ) = 0;

    /// <summary>
    /// Aborts a pending IManifestStream::GetChunkInfoAsync, if any.
    /// </summary>
    /// <returns>
    /// <para>pkS_OK: The operation has been aborted and the callback will be invoked with pkE_ABORT.</para>
    /// <para>pkS_FALSE: No operation was pending.</para>
    /// </returns>
    virtual pkRESULT AbortChunkInfoAsync() = 0;

    /// <summary>
    /// Downloads a fragment asynchronously.
    /// </summary>
    /// <param name="itChunk">iterator pointing to the chunk that contains the fragment to download</param>
    /// <param name="pTrack">pointer to the stream's track that contains the fragment</param>
    /// <param name="cbMaxBufferLength">maximum length of the buffer to read the fragment to</param>
    /// <param name="pCallback">pointer to a callback provided by the application that will be called when fragment data is available</param>
    /// <para>pkS_OK: the operation will complete asynchronously through the callback</para>
    /// <para>pkE_SINGLE_INSTANCE_OP: there's an instance of this operation already in progress, and it must complete before another can start</para>
    /// </returns>
    virtual pkRESULT DownloadFragmentAsync( _In_ const ChunkIterator& itChunk, _In_ IManifestTrack* pTrack, _In_ size_t cbMaxBufferLength, _In_ IFragmentCallback* pCallback ) = 0;

    // Public properties
    int64_t TimeScale() const               { return (m_llTimeScale); };
    const std::wstring& Language() const    { return (m_wstrLanguage); };
    uint32_t MaxWidth() const               { return (m_dwMaxWidth); };
    uint32_t MaxHeight() const              { return (m_dwMaxHeight); };
    uint32_t DisplayWidth() const           { return (m_dwDisplayWidth); };
    uint32_t DisplayHeight() const          { return (m_dwDisplayHeight); };
    const std::wstring& Url() const         { return (m_wstrUrl); };
    const std::wstring& Name() const        { return (m_wstrName); };
    MediaStreamType Type() const            { return (m_Type); };
    const std::wstring& SubType() const     { return (m_SubType); };

protected:
    // make destructor protected to avoid direct destroy of the object without calling Release()
    virtual ~IManifestStream() {};

    // Internal back store of the properties
    int64_t m_llTimeScale;
    std::wstring m_wstrLanguage;
    uint32_t m_dwMaxWidth;
    uint32_t m_dwMaxHeight;
    uint32_t m_dwDisplayWidth;
    uint32_t m_dwDisplayHeight;
    std::wstring m_wstrUrl;
    std::wstring m_wstrName;
    MediaStreamType m_Type;
    std::wstring m_SubType;
};
