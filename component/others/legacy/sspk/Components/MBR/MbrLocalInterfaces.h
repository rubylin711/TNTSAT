///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// <summary>
/// IManifestUrlServices provides services for formatting URLs
/// <summary>
class IManifestUrlServices
{
public:

    virtual void AddRef() = 0;

    virtual void Release() = 0;

    virtual pkRESULT FormatURL(
                        _In_ const wchar_t* pszBaseUrl,
                        _In_ IManifestTrack* pTrack,
                        _In_ uint32 dwChunkIndex,
                        _In_ uint32 dwHardwareProfile,
                        _In_ uint64 chunkStartTime,
                        _Out_ std::wstring* pOutUrl ) = 0;

    virtual pkRESULT FormatSegmentManifestURL(
                        _In_ int64 chunkStartTime,
                        _Out_ std::wstring* pOutUrl ) = 0;

    const std::wstring& ManifestUrl() const
    {
        return( m_strManifestUrl );
    }

    void SetManifestUrl( _In_ const wchar_t* psz )
    {
        m_strManifestUrl = psz;
    }

    bool IsSegmented() const
    {
        return( m_isSegmented );
    }

    uint64 ReferenceSegmentStartTime() const
    {
        return( m_referenceSegmentStartTime );
    }

    uint64 SegmentDuration() const
    {
        return( m_segmentDuration );
    }

    const std::wstring& SegmentUrlTemplate() const
    {
        return( m_strSegmentUrlTemplate );
    }

    void SetSegmented( _In_ bool f )
    {
        m_isSegmented = f;
    }

    void SetReferenceSegmentStartTime( _In_ uint64 v )
    {
        m_referenceSegmentStartTime = v;
    }

    void SetSegmentDuration( _In_ uint64 v )
    {
        m_segmentDuration = v;
    }

    void SetSegmentUrlTemplate( _In_ const std::wstring& v )
    {
        m_strSegmentUrlTemplate = v;
    }

protected:

    std::wstring m_strManifestUrl;
    bool m_isSegmented;
    uint64 m_referenceSegmentStartTime;
    uint64 m_segmentDuration;
    std::wstring m_strSegmentUrlTemplate;

    virtual ~IManifestUrlServices() {};
};

/// <summary>
/// Instantiates the default IManifestUrlServices
/// <summary>

namespace DefaultManifestUrlServices
{
    extern pkRESULT CreateInstance( _Out_ IManifestUrlServices** ppObj );
}

/// <summary>
/// Used by IInternalFragmentFetcher to request fragments to the local cache
/// <summary>

struct IInternalFragmentCache
{
    virtual const std::wstring& BaseUrl() = 0;

    virtual pkRESULT TryGetFragment(
                _In_ const ChunkIterator& itChunk,
                _In_ uint32 trackIndex,
                _Out_ CHUNK_INFO* pChunkInfo,
                _Deref_out_opt_ IRefBuffer** ppFragmentData ) = 0;
};

/// <summary>
/// Fetches fragments
/// <summary>

struct IInternalFragmentFetcher
{
    virtual void AddRef() = 0;

    virtual void Release() = 0;

    virtual pkRESULT FetchFragmentAsync(
                _In_ const ChunkIterator& itChunk,
                _In_ IManifestTrack* pTrack,
                _In_ size_t cbMaxBufferLength,
                _In_ IFragmentCallback* pCallback ) = 0;

    virtual pkRESULT Abort() = 0;

    virtual void CacheUpdatePoke() = 0;
};

/// <summary>
/// Instantiates the default IInternalFragmentFetcher
/// <summary>

namespace DefaultFragmentFetcher
{
    extern pkRESULT CreateInstance(
            _In_ IInternalFragmentCache* pOuterCache,   // owns the fetcher
            _In_ IManifestUrlServices* pUrlServices,
            _Out_ IInternalFragmentFetcher** ppFetcher
            );
}
