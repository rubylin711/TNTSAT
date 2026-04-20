///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <AutoLock.h>

#include "CFragmentDownloader.h"

//#define FRAGMENTFETCHER_SPEW
#ifdef FRAGMENTFETCHER_SPEW
#define FRAGMENTFETCHER_TRACE(x) TRACE(x)
#else
#define FRAGMENTFETCHER_TRACE(x)
#endif

////////////////////////////////////////////////////////////////////////////////
//
// CFragmentFetcher - default implementation of IInternalFragmentFetcher
//
////////////////////////////////////////////////////////////////////////////////

class CFragmentFetcher
    : public IInternalFragmentFetcher
    , public ChunkIterator::ContextAccessor
{
public:

    CFragmentFetcher()
        : m_wpOuterCache( NULL )
        , m_hWorkThread( NULL )
        , m_hWorkSignal( NULL )
        , m_pCallback( NULL )
        , m_fBusy( false )
    {
    }

    ~CFragmentFetcher()
    {
        ShutDown();
    }

    //
    // Implementation
    //

    pkRESULT Initialize(
                _In_ IInternalFragmentCache* pOuterCache,
                _In_ IManifestUrlServices* pUrlServices );

    void ShutDown();

    void Execute();

    bool ContinueWork();

    //
    // IInternalFragmentFetcher
    //

    __override
    pkRESULT FetchFragmentAsync(
                _In_ const ChunkIterator& itChunk,
                _In_ IManifestTrack* pTrack,
                _In_ size_t cbMaxBufferLength,
                _In_ IFragmentCallback* pCallback );

    __override
    pkRESULT Abort()
    {
        // :TODO: must implement
        return( pkE_NOTIMPL );
    }

    __override
    void CacheUpdatePoke();

private:

    IInternalFragmentCache* m_wpOuterCache;
    AutoRefPtr<IManifestUrlServices> m_apUrlServices;
    pkHANDLE m_hWorkThread;
    pkHANDLE m_hWorkSignal;

    IFragmentCallback* m_pCallback;
    AutoRefPtr<IManifestTrack> m_apTrack;
    ChunkIterator m_itChunk;
    size_t m_cbMaxBufferLength;

    Lockable m_Lock;

    LONG m_fBusy;

    static uint32_t pkAPI _WorkThreadEntryPoint( void* pParam )
    {
        reinterpret_cast<CFragmentFetcher*>( pParam )->Execute();
        return( 0 );
    }

    bool ExchangeBusy( bool fCurrent, bool fValue )
    {
        return( InterlockedCompareExchange( &m_fBusy, fValue, fCurrent ) != 0 );
    }

    IFragmentCallback* FinishCallback()
    {
        IFragmentCallback* pCallback = m_pCallback;

        m_pCallback = NULL;
        m_apTrack.Release();

        ExchangeBusy( true, false );    // reset busy

        return( pCallback );
    }
};

//////////////////////////////////////////////////////////////////////
pkRESULT CFragmentFetcher::Initialize(
    _In_ IInternalFragmentCache* pOuterCache,
    _In_ IManifestUrlServices* pUrlServices )
{
    pkRESULT pkResult = pkS_OK;

    // The cache aggregates the fetcher,
    // so a weak reference works OK
    m_wpOuterCache = pOuterCache;

    m_apUrlServices.Set( pUrlServices );

    pkResult = Executive_CreateEvent( NULL, false, false, &m_hWorkSignal );
    if( pkFAILED(pkResult) )
    {
        goto exit;
    }

    pkResult = Executive_CreateThread( _WorkThreadEntryPoint, this, 0, &m_hWorkThread );
    if( pkFAILED(pkResult) )
    {
        goto exit;
    }

exit:

    return( pkResult );
}

////////////////////////////////////////////////////////////////////////////////
void CFragmentFetcher::ShutDown()
{
    pkHANDLE hWorkThread = m_hWorkThread;
    m_hWorkThread = NULL;

    if( hWorkThread != NULL )
    {
        // Wake up the thread to detect a shutdown
        Executive_SetEvent( m_hWorkSignal );
        Executive_WaitForThread( hWorkThread, EXEC_WAIT_INFINITE );
        Executive_CloseThread( hWorkThread );
    }

    if( m_hWorkSignal != NULL )
    {
        Executive_CloseEvent( m_hWorkSignal );
        m_hWorkSignal = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
void CFragmentFetcher::Execute()
{
    pkRESULT pkResult = pkS_OK;
    CHUNK_INFO chunkInfo = { 0 };

    while( ContinueWork() )
    {
        if( m_pCallback == NULL )
        {
            // Woke up on false alarm, no requests pending.
            // Ignore.
            continue;
        }

        //
        // First, retrieve the chunk info to obtain the timestamp and/or index of the fragment
        //

        AutoRefPtr<IRefBuffer> apFragmentData;
        size_t cbFragmentLength;

        pkResult = m_wpOuterCache->TryGetFragment( m_itChunk, m_apTrack->TrackIndex(), &chunkInfo, apFragmentData.DerefOutPtr() );
        if( pkResult == pkE_PENDING )
        {
            continue;
        }

        cbFragmentLength = ( apFragmentData != NULL ) ? apFragmentData->Length() : 0;

        if( pkFAILED(pkResult) || ( apFragmentData != NULL ) )
        {
            //
            // Complete the request if the operation failed for anything other
            // than a cache miss, or if the fragment data was already in the
            // cache (i.e., ManifestOutput=true)
            //

            FinishCallback()->OnFragmentData( pkResult, &chunkInfo, apFragmentData, true, cbFragmentLength );

            continue;
        }

        //
        // Second, if the fragment was not available in the cache (from the manifest),
        // download it from the server
        //

        {
            ChunkIterator::CONTEXT ctx = CtxOf( m_itChunk );
            std::wstring strChunkUrl;
            size_t cbFragmentLength = 0;
            size_t iDownload = 0;
            size_t cbToDownload = 0;
            CFragmentDownloader downloader;

            //
            // Compose the URL to download the fragment
            //

            pkResult = m_apUrlServices->FormatURL(
                                    m_wpOuterCache->BaseUrl().c_str(),
                                    m_apTrack,
                                    ctx.m_iChunk,
                                    m_apTrack->HardwareProfile(),
                                    chunkInfo.chunkTime,
                                    &strChunkUrl );

            //
            // Dowload the fragment
            //

            if( pkSUCCEEDED(pkResult) )
            {
                FRAGMENTFETCHER_TRACE(("@%p: Download '%ls'", this, strChunkUrl.c_str() ));

                pkResult = downloader.RequestFragment( strChunkUrl );
            }

            if( pkSUCCEEDED(pkResult) )
            {
                pkResult = downloader.ReceiveHeader( &cbFragmentLength );
            }

            if( pkSUCCEEDED(pkResult) )
            {
                cbToDownload = min( cbFragmentLength, m_cbMaxBufferLength );

                pkResult = CreateRefBuffer( cbToDownload, apFragmentData.DerefOutPtr() );
            }

            //
            // :TODO:
            // This does not support multi-part downloads yet. It's return the
            // first buffer and finish the callback.
            //

            while( iDownload < cbToDownload )
            {
                size_t cbRead = 0;

                pkResult = downloader.ReceiveFragmentData(
                                    apFragmentData->Data() + iDownload,
                                    cbToDownload - iDownload,
                                    &cbRead );

                if( pkFAILED(pkResult) )
                {
                    break;
                }

                iDownload += cbRead;
            }

            FinishCallback()->OnFragmentData( pkResult, &chunkInfo, apFragmentData, ( iDownload == cbFragmentLength ), cbFragmentLength );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
bool CFragmentFetcher::ContinueWork()
{
    bool fContinue = true;
    IFragmentCallback* pCallback = NULL;

    Executive_WaitForEvent( m_hWorkSignal, EXEC_WAIT_INFINITE );

    {
        AutoLock al( &m_Lock );

        if( m_hWorkThread == NULL )
        {
            // Shutdown required.
            // Mark as busy so no new requests may come.

            ExchangeBusy( false, true );

            pCallback = m_pCallback;
            m_pCallback = NULL;
            m_apTrack.Release();

            fContinue = false;
        }
    }

    if( pCallback != NULL )
    {
        CHUNK_INFO chunkInfo = { 0 };

        pCallback->OnFragmentData( pkE_ABORT, &chunkInfo, NULL, false, 0 );
    }

    return( fContinue );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CFragmentFetcher::FetchFragmentAsync(
    _In_ const ChunkIterator& itChunk,
    _In_ IManifestTrack* pTrack,
    _In_ size_t cbMaxBufferLength,
    _In_ IFragmentCallback* pCallback )
{
    pkRESULT pkResult = pkS_OK;

    {
        AutoLock al( &m_Lock );

        bool fBusy = ExchangeBusy( false, false );

        if( fBusy )
        {
            pkResult = pkE_SINGLE_INSTANCE_OP;
            goto exit;
        }

        m_itChunk = itChunk;
        m_pCallback = pCallback;
        m_apTrack.Set( pTrack );
        m_cbMaxBufferLength = cbMaxBufferLength;

        fBusy = ExchangeBusy( false, true );
        ASSERT( !fBusy );

        Executive_SetEvent( m_hWorkSignal );
    }

exit:

    return( pkResult );
}

////////////////////////////////////////////////////////////////////////////////
void CFragmentFetcher::CacheUpdatePoke()
{
    Executive_SetEvent( m_hWorkSignal );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT DefaultFragmentFetcher::CreateInstance(
    _In_ IInternalFragmentCache* pOuterCache,   // owns the fetcher
    _In_ IManifestUrlServices* pUrlServices,
    _Out_ IInternalFragmentFetcher** ppFetcher
    )
{
    pkRESULT pkResult = pkS_OK;

    *ppFetcher = NULL;

    AutoRefPtr<CFragmentFetcher> apObj;

    apObj.AdoptRef( NEW_NO_THROW CRefCountedObj<CFragmentFetcher>() );

    if( apObj == NULL )
    {
        pkResult = pkE_OUTOFMEMORY;
        goto exit;
    }

    pkResult = apObj->Initialize( pOuterCache, pUrlServices );
    if( pkFAILED(pkResult) )
    {
        goto exit;
    }

    *ppFetcher = apObj.HandOffRef();

exit:

    return( pkResult );
}
