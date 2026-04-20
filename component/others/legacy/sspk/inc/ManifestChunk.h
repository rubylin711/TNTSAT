///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// <summary>
///     ManifestChunk.h
/// </summary>

/// <summary>
/// ChunkIterator: iterator to traverse the chunks of a stream
/// <summary>

class ChunkIterator
{
public:

    ChunkIterator();

    ChunkIterator( const ChunkIterator& that );

    ChunkIterator& operator = ( const ChunkIterator & that );

    pkRESULT MoveNext();

    pkRESULT MovePrev();

public:

    struct ContextAccessor;

    struct CONTEXT
    {
        void* m_pToken;
        int64_t m_minTime;
        union
        {
            int64_t m_time;
            int32_t m_iChunk;
        };
        bool m_fByIndex;
    };

private:

    ChunkIterator(
        _In_ void* pToken,
        _In_ int64_t minTime,
        _In_ int64_t refTime );

    mutable CONTEXT m_Ctx;
};

/// <summary>
/// CHUNK_INFO: structure with information about a chunk
/// <summary>

struct CHUNK_INFO
{
    int64_t chunkTime;
};

/// <summary>
/// IChunkInfoCallback: interface exposed by the application to receive the
/// completion of a call to IManifestStream::GetChunkInfoAsync
/// <summary>

class IChunkInfoCallback
{
public:

    /// <summary>
    /// Receives the completion of a call to IManifestStream::GetChunkInfoAsync
    /// </summary>
    /// <param name="hrResult">result of the operation
    /// <para>pkS_OK: success</para>
    /// <para>pkE_NO_MORE_ITEMS: no chunks found at the required iterator position</para>
    /// <para>pkE_ABORT: the operation was aborted</para>
    /// </param>

    virtual void OnChunkInfo(
                        _In_ pkRESULT hrResult,
                        _In_ const CHUNK_INFO* pChunk ) = 0;

protected:

    virtual ~IChunkInfoCallback() {};
};

/// <summary>
/// IRefBuffer: interface to an object representing a reference-counted array of bytes
/// <summary>

class IRefBuffer
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
    /// Get the length of the data
    /// </summary>
    /// <return> The length of the data. </return>
    virtual size_t Length() = 0;

    /// <summary>
    /// Get the actual bytes of the data
    /// </summary>
    /// <return> The actual bytes of the data. </return>
    virtual byte* Data() = 0;

protected:
    // make destructor protected to avoid direct destroy of the object without calling Release()
    virtual ~IRefBuffer() {};
};

/// <summary>
/// Creates the default reference-counted buffer exposing the IRefBuffer interface
/// <param name="cbLength">length of the buffer to create</param>
/// <param name="ppBuffer">returns a pointer to the buffer object</param>
/// </param>
/// <summary>

pkRESULT CreateRefBuffer( _In_ size_t cbLength, _Deref_out_ IRefBuffer** ppBuffer );

/// <summary>
/// IFragmentCallback: interface exposed by the application to receive the
/// completion of a call to IManifestTrack::DownloadFragmentAsync
/// <summary>

class IFragmentCallback
{
public:

    /// <summary>
    /// Receives the completion of a call to IManifestStream::DownloadFragmentAsync
    /// </summary>
    /// <param name="hrResult">result of the asynchronous operation
    ///     <para>pkS_OK: success</para>
    ///     <para>pkE_NO_MORE_ITEMS: no chunk found for the iterator given to IManifestTrack::DownloadFragmentAsync</para>
    ///     <para>pkE_NOT_FOUND: no fragment was found in the track for the stream chunk</para>
    ///     <para>pkE_ABORT: the operation was aborted</para>
    /// </param>
    /// <param name="pChunkInfo">information about the stream chunk corresponding to the fragment</param>
    /// <param name="pBuffer">buffer with the fragment's data</param>
    /// <param name="fFinalBuffer">
    ///     <para>true: this is the final piece of the fragment</para>
    ///     <para>false: this is part of fragment, but there's more</para>
    /// </param>
    /// <param name="cbTotalLength">total length of the fragment (it can be more than what's passed in the buffer object)</param>

    virtual pkRESULT OnFragmentData(
                        _In_ pkRESULT hrResult,
                        _In_ CHUNK_INFO* pChunkInfo,
                        _In_opt_ IRefBuffer* pBuffer,
                        _In_ bool fFinalBuffer,
                        _In_ size_t cbTotalLength ) = 0;

protected:

    virtual ~IFragmentCallback() {};
};

