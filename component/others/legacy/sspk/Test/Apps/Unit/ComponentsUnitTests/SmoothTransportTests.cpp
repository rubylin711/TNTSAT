///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <pkSockets.h>
#include <PKTestSuite.h>
#include <PKTestSuiteUtils.h>
#include <pkTestFramework.h>

#include <IAVManager.h>
#include <ISmoothTransport.h>

#include <AutoLock.h>
#include <CEvent.h>

#include "SmoothTransportDriver.h"


////////////////////////////////////////////////////////////////////////////////
PKTEST_GROUP( MediaTransportTests )
{
    ////////////////////////////////////
    PKTEST_METHOD( OpenPlayStopOnDemandURLs )
    {
        bool fSocketsInitialized = false;

        CXMLElement xTable = PKTest_GetDataTable( "OnDemandUrls" );
        CXMLElementsList xOnDemandUrls = xTable.Elements( L"url" );

        PKTEST_ASSERT_EXIT( !xTable.IsNull() );

        PKTEST_ASSERT_EXIT( SOCKET_SUCCESS == Socket_Startup() );
        fSocketsInitialized = true;

        for( int iUrl = 0; iUrl < xOnDemandUrls.Length(); ++iUrl )
        {
            CXMLAttribute xUrl = xOnDemandUrls[iUrl].Attributes()[L"value"];

            PKTEST_ASSERT_EXIT( !xUrl.IsNull() );

            PKTEST_FUNC_EXIT( RunOpenPlayStopOnDemandURL( wstring_to_string( xUrl.Value() ).c_str() ) );
        }

    exit:

        if( fSocketsInitialized )
        {
            Socket_Cleanup();
        }
    }

    ////////////////////////////////////
    PKTEST_METHOD( OpenAndDumpChunkInfoFromLocalCache )
    {
        bool fSocketsInitialized = false;

        CXMLElement xTable = PKTest_GetDataTable( "DumpChunkInfo" );
        CXMLElementsList xOnDemandUrls = xTable.Elements( L"url" );

        PKTEST_ASSERT_EXIT( !xTable.IsNull() );

        PKTEST_ASSERT_EXIT( SOCKET_SUCCESS == Socket_Startup() );
        fSocketsInitialized = true;

        for( int iUrl = 0; iUrl < xOnDemandUrls.Length(); ++iUrl )
        {
            CXMLAttribute xUrl = xOnDemandUrls[iUrl].Attributes()[L"value"];

            PKTEST_ASSERT_EXIT( !xUrl.IsNull() );

            PKTEST_FUNC_EXIT( ConsumeChunkInfoFromLocalCache( wstring_to_string( xUrl.Value() ).c_str() ) );
        }

    exit:

        if( fSocketsInitialized )
        {
            Socket_Cleanup();
        }
    }

    ////////////////////////////////////
    // Event sink used by most tests in
    // this test group
    struct LocalSink : public CSmoothTransportDriver::EventSink
    {
        bool m_fError;
        bool m_fPlaying;
        CSmoothTransportDriver::AutoEvent m_Signal;
        AutoRefPtr<IManifest> m_apManifest;

        void Reset()
        {
            m_fError = false;
            m_fPlaying = false;
            m_apManifest.Release();
            m_Signal.Reset();
        }

        void OnError( _In_ CSmoothTransportDriver& drv, _In_ SSPK::SmoothTransportError& error )
        {
            m_fError = true;
            m_Signal.Set();
        }

        void OnStatus( _In_ CSmoothTransportDriver& drv, _In_ SSPK::SmoothTransportStatus& status )
        {
            LogTestComment( "   %s, %s",
                GetSmoothTransportTunerStateDesc( status._currentState ),
                GetSmoothTransportStatusName( status._update ) );

            if( status._currentState == SSPK::SmoothTransportTunerState_Playing
                && status._update == SSPK::SmoothTransportStatus_Rendering )
            {
                m_fPlaying = true;
                m_Signal.Set();
            }
        }

        void OnManifestReady(
                    _In_ CSmoothTransportDriver& drv,
                    _In_ IManifest* pManifest,
                    _In_ HRESULT result)
        {
            m_fError = false;
            m_apManifest.Set( pManifest );
            m_Signal.Set();
        }
    };

    ////////////////////////////////////
    // Fragment callback used by
    // this test group
    struct LocalFragmentCallback : public IFragmentCallback
    {
        CSmoothTransportDriver::AutoEvent m_Signal;
        AutoRefPtr<IRefBuffer> m_apFragmentData;
        HRESULT m_hrResult;
        CHUNK_INFO m_chunkInfo;

        __override
        HRESULT OnFragmentData(
                    _In_ HRESULT hrResult,
                    _In_ CHUNK_INFO* pChunkInfo,
                    _In_opt_ IRefBuffer* pBuffer,
                    _In_ bool fFinalBuffer,
                    _In_ size_t cbTotalSize )
        {
            m_hrResult = hrResult;
            m_apFragmentData.Set( pBuffer );
            m_chunkInfo = *pChunkInfo;
            m_Signal.Set();
            return( S_OK );
        }
    };

    ////////////////////////////////////
    void RunOpenPlayStopOnDemandURL( _In_ const char* pszUrl )
    {
        CSmoothTransportDriver fixture;

        LogTestComment( "URL = %s", pszUrl );

        LocalSink localEventSink;

        //
        // Open the URL and wait for the ManifestReady event
        //

        PKTEST_HRESULT_EXIT( fixture.Create() );

        fixture.SetErrorSink( &localEventSink );
        fixture.SetStausSink( &localEventSink );
        fixture.SetManifestReadySink( &localEventSink );

        fixture->Open( pszUrl, SSPK::SmoothTransportProtocol_Mbr, false );

        PKTEST_ASSERT_EXIT( localEventSink.m_Signal.Wait( 5000 ) == CEvent::eWaitSignaled );

        PKTEST_ASSERT_EXIT( !localEventSink.m_fError );
        PKTEST_ASSERT_EXIT( localEventSink.m_apManifest != NULL );

        localEventSink.m_Signal.Reset();

        fixture->Play();

        PKTEST_ASSERT_EXIT( localEventSink.m_Signal.Wait( 5000 ) == CEvent::eWaitSignaled );
        PKTEST_ASSERT_EXIT( localEventSink.m_fPlaying );

        fixture->Close();

        PKTEST_ASSERT_EXIT( !localEventSink.m_fError );

    exit:

        fixture->Close();

        fixture.Dispose();
    }

    ////////////////////////////////////
    void ConsumeChunkInfoFromLocalCache( _In_ const char* pszUrl )
    {
        CSmoothTransportDriver fixture;
        std::vector< AutoRefPtr<IManifestStream> > streams;

        LogTestComment( "URL = %s", pszUrl );

        LocalSink localEventSink;
        LocalFragmentCallback localFragmentCallback;

        //
        // Open the URL and wait for the ManifestReady event
        //

        PKTEST_HRESULT_EXIT( fixture.Create() );

        fixture.SetErrorSink( &localEventSink );
        fixture.SetStausSink( &localEventSink );
        fixture.SetManifestReadySink( &localEventSink );

        fixture->Open( pszUrl, SSPK::SmoothTransportProtocol_Mbr , false);

        PKTEST_ASSERT_EXIT( localEventSink.m_Signal.Wait( 5000 ) == CEvent::eWaitSignaled );

        PKTEST_ASSERT_EXIT( !localEventSink.m_fError );
        PKTEST_ASSERT_EXIT( localEventSink.m_apManifest != NULL );

        localEventSink.m_Signal.Reset();

        //
        // Take all chunk info in the cache for all available streams using TryGetChunkInfo
        //

        localEventSink.m_apManifest->GetSelectedStreams( &streams );

        for( std::vector< AutoRefPtr<IManifestStream> >::iterator itStream = streams.begin();
            itStream != streams.end();
            ++itStream )
        {
            int64_t currentTime = 0;
            CHUNK_INFO chunkInfo;
            HRESULT hr = S_OK;
            size_t cChunks = 0;
            std::vector< AutoRefPtr<IManifestTrack> > tracks;

            (*itStream)->GetSelectedTracks( &tracks );

            ChunkIterator itChunk = (*itStream)->GetIterator( 0, 0 );   // from the beginning

            while( SUCCEEDED( hr = (*itStream)->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                //
                // Get all the fragments for the first 2 chunks in the stream
                //

                if( cChunks < 2 )
                {
                    LogTestComment( "  Stream '%ls'", (*itStream)->Name().c_str() );

                    for( std::vector< AutoRefPtr<IManifestTrack> >::iterator itTrack = tracks.begin();
                        itTrack != tracks.end();
                        ++itTrack )
                    {
                        PKTEST_HRESULT_EXIT( (*itStream)->DownloadFragmentAsync( itChunk, *itTrack, (size_t)-1, &localFragmentCallback ) );

                        PKTEST_ASSERT_EXIT( localFragmentCallback.m_Signal.Wait( EXEC_WAIT_INFINITE ) == CEvent::eWaitSignaled );

                        LogTestComment(
                                "    [%d] t = %lld, fragmentSize = %d",
                                (*itTrack)->TrackIndex(),
                                localFragmentCallback.m_chunkInfo.chunkTime,
                                localFragmentCallback.m_apFragmentData->Length() );
                    }
                }

                //
                // Move to the next chunk
                //

                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );

                if( cChunks++ > 0 )
                {
                    // Chunks must progress in time as the iterator moves to the next chunk
                    PKTEST_ASSERT_EXIT( chunkInfo.chunkTime > currentTime );
                }

                currentTime = chunkInfo.chunkTime;
            }

            LogTestComment( "  Stream '%ls', chunks %d", (*itStream)->Name().c_str(), cChunks );

            if( localEventSink.m_apManifest->IsLive() )
            {
                PKTEST_ASSERT_EXIT( hr == pkE_PENDING );
            }
            else
            {
                PKTEST_ASSERT_EXIT( hr == pkE_NO_MORE_ITEMS );
            }
        }

    exit:

        fixture->Close();

        fixture.Dispose();
    }
};
