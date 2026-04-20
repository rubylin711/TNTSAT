///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pkSockets.h"
#include "pkExecutive.h"
#include "pkTestFramework.h"
#include "PKTestSuite.h"

#include "STUtilities.h"
#include "ISmoothTransport.h"
#include "STWrapper.h"
#include "ManifestReadyCallbackImpl.h"

#include <algorithm>

#include "SSPKHelpers.h"

using namespace SSPKTest;
using namespace SSPK;

#define SKIP_THIS {goto exit;}

PKTEST_GROUP( TrackSelectionTests )
{
    STWrapper               *m_pSTWObject;
    CXMLElementsList        m_Manifests;

    TrackSelectionTests()
    {
        CXMLElement xTestData = PKTest_GetDataTable( "IntegrationTestSources" );
        if( !xTestData.IsNull() )
        {
             m_Manifests = xTestData.Elements(L"source");
        }
    }

    bool TestSetup()
    {
        PKTEST_ASSERT_MSG_EXIT(m_Manifests.Length() > 0,"Couldn't find any elements with name 'source'");

        m_pSTWObject = new STWrapper();
        PKTEST_ASSERT_MSG_EXIT( NULL != m_pSTWObject, "Unable to start smooth streaming object");

        return true;
exit:
        return false;
    }

    bool TestCleanup()
    {
        m_pSTWObject->SetManifestCallback(NULL);


        if(!m_pSTWObject->IsClosed())
        {
            m_pSTWObject->Close();
        }

        delete m_pSTWObject;

        return true;
    }


    ////////////////////////////////////
    //  Manifest Ready Callbacks
    ////////////////////////////////////

    //Verify Available Tracks Equal Selected Tracks By Default On ManifestReady
    //STEPS:
    //  foreach stream in the manifest
    //      Get Available Tracks
    //      Get Selected Tracks
    //      Compare if available == selected
    //      Log Results
    void AvailableTracksEqualSelectedTracksByDefaultOnManifestReady(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );

        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetSelectedTracks( &selectedTracks ) );
            PKTEST_ASSERT_EXIT( availableTracks == selectedTracks );
        }
    exit:
        return ;
    }

    //Verify Restrict Tracks Changes The Available Tracks
    //STEPS
    //  foreach stream in the manifest
    //      Get Available Tracks
    //      Call Restrict Tracks
    //      Get Available Tracks
    //      Compare and Log Results
    void RestrictTracksChangesTheAvailableTracks(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > restrictedTracks;
        std::vector< AutoRefPtr<IManifestTrack> > shuffledRestrictedTracks;

        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );
        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );
        //
        // Restrict the tracks to a subset of the available tracks and verify!
        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            // Repeat a few times for the same stream, each time removing a few tracks
            for( int32_t j = STUtilities::Rand(8); j < 8; ++j )
            {
                PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

                restrictedTracks = availableTracks;

                for( int32_t k = STUtilities::Rand( restrictedTracks.size() ); k > 0; --k )
                {
                    int32_t iTrackToRemove = STUtilities::Rand( restrictedTracks.size() );
                    restrictedTracks.erase( restrictedTracks.begin() + iTrackToRemove );
                }

                shuffledRestrictedTracks = restrictedTracks;

                std::random_shuffle( shuffledRestrictedTracks.begin(), shuffledRestrictedTracks.end() );

                PKTEST_HRESULT_EXIT( availableStreams[iStrm]->RestrictTracks( shuffledRestrictedTracks ) );

                PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

                PKTEST_ASSERT_EXIT( availableTracks == restrictedTracks );

                if( availableTracks.size() == 0 )
                {
                    break;
                }
            }
        }

    exit:
        return ;
    }

    //Verify Calling Restrict Tracks With No Tracks Returns Error
    //STEPS:
    //  Get Available Tracks
    //  Call Restrict Tracks With No Tracks
    //  Trap error and Log Results.
    //  Re-Confirm AvailableTracks was unchanged!
    void CallingRestrictTracksWithNoTracksReturnsError(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks2;
        std::vector< AutoRefPtr<IManifestTrack> > restrictedTracks;

        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );
        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        // Restrict the tracks completely and verify!
        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

            hr = availableStreams[iStrm]->RestrictTracks( restrictedTracks );
            //This should return an error code so we confirm if it did return an error!
            PKTEST_HRESULT_EXIT( FAILED(hr)  );

            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks2 ) );
            PKTEST_ASSERT_EXIT( availableTracks == availableTracks2 );
        }

    exit:
        return ;
    }

    //Verify restrict tracks with all the tracks.
    //STEPS:
    //      Get Available Tracks
    //      Call Restrict Tracks With All the Tracks from available tracks
    //      Get Available Tracks
    //      Compare and Log Results
    void RestrictTracksWithAllTracks(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks2;
        std::vector< AutoRefPtr<IManifestTrack> > restrictedTracks;

        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );

        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );
        //
        // Restrict the tracks completely and verify!
        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

            restrictedTracks = availableTracks;

            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->RestrictTracks( restrictedTracks ) );
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks2 ) );

            PKTEST_ASSERT_EXIT( restrictedTracks == availableTracks2 );
        }

    exit:
        return ;
    }

    //Verify restrict tracks with all the tracks.
    //STEPS:
    //      Get Available Tracks
    //      Call Restrict Tracks With some Tracks of same bitrate and resolution.
    //      Get Available Tracks
    //      Compare and Log Results
    void RestrictTracksWithSomeTracksOfSameBitrateAndResolution(IManifest* pManifest, pkRESULT hr)
    {
        //TODO:
        return ;
    }

    //Verify How Calling Restrict Tracks Behaves On Already Selected Tracks
    //STEPS:
    //  Get Available Tracks
    //  Select Some Tracks/(By Default some are selected!)
    //  Call Restrict tracks on Selected Tracks.
    //  Get Selected Tracks
    //  Verify results and any errors.
    void RestrictTracksOnAlreadySelectedTracks(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;

        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks2;

        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );
        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetSelectedTracks( &selectedTracks ) );

            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->RestrictTracks( selectedTracks ) );

            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

            //Confirm restriction was proper!
            PKTEST_ASSERT_EXIT( availableTracks == selectedTracks );

            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetSelectedTracks( &selectedTracks2 ) );

            //Confirm selection was proper!
            PKTEST_ASSERT_EXIT( selectedTracks2 == selectedTracks );
        }

    exit:
        return ;
    }

    //Verify Multiple Restrict Tracks Calls
    //STEPS:
    //  Get Available Tracks
    //  Call Restrict tracks
    //  Call Restrict tracks Again
    //  Get Available Tracks
    //  Verify results and any errors.
    void MultipleRestrictTracksCalls(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > restrictedTracks;
        std::vector< AutoRefPtr<IManifestTrack> > shuffledRestrictedTracks;

        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );
        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        // Restrict the tracks to a subset of the available tracks and verify!
        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            // Repeat a few times for the same stream, each time removing a few tracks
            for( int32_t j = STUtilities::Rand(8); j < 8; ++j )
            {
                PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

                restrictedTracks = availableTracks;

                for( int32_t k = STUtilities::Rand( restrictedTracks.size() ); k > 0; --k )
                {
                    int32_t iTrackToRemove = STUtilities::Rand( restrictedTracks.size() );
                    restrictedTracks.erase( restrictedTracks.begin() + iTrackToRemove );
                }

                shuffledRestrictedTracks = restrictedTracks;

                std::random_shuffle( shuffledRestrictedTracks.begin(), shuffledRestrictedTracks.end() );

                //I'll just call RestrictTracks couple times with the same param.
                //Note that outer loop already causes multiple RestrictTracks calls but with different param.
                for(int32_t i=0; i<5; i++){
                    PKTEST_HRESULT_EXIT( availableStreams[iStrm]->RestrictTracks( shuffledRestrictedTracks ) );
                }

                PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

                PKTEST_ASSERT_EXIT( availableTracks == restrictedTracks );

                if( availableTracks.size() == 0 )
                {
                    break;
                }
            }
        }

    exit:
        return ;
    }

    ////////////////////////////////////
    //  Get Available Tracks Tests
    ////////////////////////////////////
    //
    //TEST: The Available tracks should be same as the selected tracks at the Manifest ready.
    PKTEST_METHOD_EX( VerifyAvailableTracksEqualSelectedTracksByDefaultOnManifestReady,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &TrackSelectionTests::AvailableTracksEqualSelectedTracksByDefaultOnManifestReady );


        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( m_Manifests, L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );
exit:
        return;
    }

    ////////////////////////////////////
    // Restrict Tracks Tests
    ////////////////////////////////////

    // TEST: Call restrict tracks with a few available tracks, and check to see the available tracks changes accordingly.
    PKTEST_METHOD_EX( VerifyRestrictTracksChangesTheAvailableTracks, 
        PKTEST_PROPERTY("Priority", "0")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &TrackSelectionTests::RestrictTracksChangesTheAvailableTracks );


        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( m_Manifests, L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

exit:
        return;
    }

    // TEST: Call restrict tracks with no tracks, and verify that we have an error.
    PKTEST_METHOD_EX( VerifyCallingRestrictTracksWithNoTracksReturnsError,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &TrackSelectionTests::CallingRestrictTracksWithNoTracksReturnsError );


        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( m_Manifests, L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

exit:
        return;
    }

    // TEST: Call restrict tracks with all the tracks.( restrict all tracks in a stream.?)
    PKTEST_METHOD_EX( VerifyRestrictTracksWithAllTracks,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &TrackSelectionTests::RestrictTracksWithAllTracks );


        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( m_Manifests, L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

exit:
        return;
    }

    // TEST: Call restrict tracks with some tracks of same bitrate and same resolution.
    PKTEST_METHOD_EX( VerifyRestrictTracksWithSomeTracksOfSameBitrateAndResolution,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &TrackSelectionTests::RestrictTracksWithSomeTracksOfSameBitrateAndResolution );


        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( m_Manifests, L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

exit:
        return;
    }

    // TEST: Call restrict tracks after manifest ready and verify that we get back and error.
    // STEPS:
    //      Call RestTracks After ManifestReady!
    PKTEST_METHOD_EX( VerifyRestrictTracksAfterManifestReadyReturnsError,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks2;
        std::vector< AutoRefPtr<IManifestTrack> > restrictedTracks;

        m_pSTWObject->SetManifestReadyMethod(this, &TrackSelectionTests::DoNothingCallback );


        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( m_Manifests, L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

        //Call RestrictTracks and Verify error code

        PKTEST_HRESULT_EXIT(  m_pSTWObject->GetManifest()->GetAvailableStreams( &availableStreams ) );

        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        // Try Restrict tracks here outside manifest ready!
        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

            restrictedTracks = availableTracks;

            //This should actually return an error because Resrict can be called only from inside the manifestready callback!
            HRESULT hr = availableStreams[iStrm]->RestrictTracks( restrictedTracks );
            PKTEST_HRESULT_EXIT( FAILED(hr) );

            //The available tracks should still be the same because RestrictTracks failed!
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks2 ) );
            PKTEST_ASSERT_EXIT( restrictedTracks == availableTracks2 );
        }

exit:
        return;
    }

    // TEST: Call restrict tracks, restricting the tracks which are already selected.
    PKTEST_METHOD_EX( VerifyRestrictTracksOnAlreadySelectedTracks,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &TrackSelectionTests::RestrictTracksOnAlreadySelectedTracks );


        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( m_Manifests, L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );
exit:
        return;
    }

    // TEST: Multiple Restrict Tracks Calls!
    PKTEST_METHOD_EX( VerifyMultipleRestrictTracksCalls,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &TrackSelectionTests::MultipleRestrictTracksCalls );


        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( m_Manifests, L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );
exit:
        return;
    }


    ////////////////////////////////////
    // Get Selected Tracks Tests
    ////////////////////////////////////
    //
    //Test: The Available tracks should be same as the selected tracks outside Manifest ready callback.
    //STEPS:
    //  foreach stream in the manifest
    //      Get Available Tracks
    //      Get Selected Tracks
    //      Compare if available == selected
    //      Log Results.
    PKTEST_METHOD_EX( VerifyAvailableTracksEqualSelectedTracksByDefaultOutsideManifestReady,
        PKTEST_PROPERTY("Priority", "0")
        )
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        m_pSTWObject->SetManifestReadyMethod(this, &TrackSelectionTests::DoNothingCallback );


        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( m_Manifests, L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

        //Try GetSelectedTracks here. ManifestReady calllback should be done by now.
        PKTEST_HRESULT_EXIT( m_pSTWObject->GetManifest()->GetAvailableStreams( &availableStreams ) );

        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetSelectedTracks( &selectedTracks ) );
            PKTEST_ASSERT_EXIT( availableTracks == selectedTracks );
        }

exit:
        return;
    }

    //Test:  Verify selected tracks can be called inside manifestready callback.
    PKTEST_METHOD_EX( CallSelectedTracksInsideManifestReady,
        PKTEST_PROPERTY("Priority", "0")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &TrackSelectionTests::SelectedTracksInsideManifestReady );


        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( m_Manifests, L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );
exit:
        return;
    }

    //STEPS:
    //  foreach stream in the manifest
    //      Get Selected Tracks
    //      Log Results.
    void SelectedTracksInsideManifestReady(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        PKTEST_HRESULT_EXIT( m_pSTWObject->GetManifest()->GetAvailableStreams( &availableStreams ) );

        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            //Comparing against availableTracks would validate the return results for GetSelectedTracks!
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetSelectedTracks( &selectedTracks ) );
            PKTEST_ASSERT_EXIT( availableTracks == selectedTracks );
        }

    exit:
        return ;
    }

    //Test:  Verify selected tracks can be called outside manifestready callback.
    //STEPS:
    //  foreach stream in the manifest
    //      Get Selected Tracks
    //      Log Results.
    PKTEST_METHOD_EX( CallSelectedTracksOutsideManifestReady, 
        PKTEST_PROPERTY("Priority", "0")
        )
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        m_pSTWObject->SetManifestReadyMethod(this, &TrackSelectionTests::DoNothingCallback );


        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( m_Manifests, L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

        PKTEST_HRESULT_EXIT( m_pSTWObject->GetManifest()->GetAvailableStreams( &availableStreams ) );

        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            //Comparing against availableTracks would validate the return results for GetSelectedTracks!
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetSelectedTracks( &selectedTracks ) );
            PKTEST_ASSERT_EXIT( availableTracks == selectedTracks );
        }
exit:
        return;
    }

    void DoNothingCallback(IManifest* pManifest, pkRESULT hr)
    {
        //Do Nothing.
        return ;
    }

};







