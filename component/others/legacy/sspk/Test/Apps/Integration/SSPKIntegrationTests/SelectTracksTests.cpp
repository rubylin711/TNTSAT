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

PKTEST_GROUP( SelectTracksTests )
{
    STWrapper               *m_pSTWObject;
    CXMLElementsList        m_Manifests;

    SelectTracksTests()
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

        m_pSTWObject = NEW_NO_THROW STWrapper();
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
    void DoNothingCallback(IManifest* pManifest, pkRESULT hr)
    {
        //Do Nothing.
        return ;
    }

    //STEPS:
    //  foreach stream in the manifest.
    //      Get Available Tracks
    //      Call Select Tracks
    //      Get Selected Tracks
    //      Compare and Log Results
    void SelectTracksChangesSelectedTracks(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracksBefore;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracksAfter;
        std::vector< AutoRefPtr<IManifestTrack> > shuffledSelectedTracks;

        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );
        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            // Repeat a few times for the same stream, each time removing a few tracks
            for( int32_t j = STUtilities::Rand(8); j < 8; ++j )
            {
                PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

                selectedTracksBefore = availableTracks;

                for( int32_t k = STUtilities::Rand( selectedTracksBefore.size() ); k > 0; --k )
                {
                    int32_t iTrackToRemove = STUtilities::Rand( selectedTracksBefore.size() );
                    selectedTracksBefore.erase( selectedTracksBefore.begin() + iTrackToRemove );
                }

                shuffledSelectedTracks = selectedTracksBefore;

                std::random_shuffle( shuffledSelectedTracks.begin(), shuffledSelectedTracks.end() );

                PKTEST_HRESULT_EXIT( availableStreams[iStrm]->SelectTracks( shuffledSelectedTracks ) );
                PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetSelectedTracks( &selectedTracksAfter ) );

                PKTEST_ASSERT_EXIT( selectedTracksAfter == selectedTracksBefore );

                if( availableTracks.size() == 0 )
                {
                    break;
                }
            }
        }

    exit:
        return ;
    }

    //STEPS:
    //  foreach stream in the manifest.
    //      Get Available Tracks
    //      Call Select Tracks with no tracks
    //      Trap errors
    //      Get Selected Tracks
    //      Compare and Log Results
    void SelectTracksWithNoTracks(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracksBefore;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracksAfter;
        std::vector< AutoRefPtr<IManifestTrack> > selectTracks;

        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );
        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetSelectedTracks( &selectedTracksBefore ) );

            hr = availableStreams[iStrm]->SelectTracks( selectTracks ); //Empty argument!
            //Should have failed.
            PKTEST_ASSERT_EXIT( FAILED(hr) );

            //Reset error code.
            hr = S_OK;

            //Should not have changed anything
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetSelectedTracks( &selectedTracksAfter ) );
            PKTEST_ASSERT_EXIT( selectedTracksAfter == selectedTracksBefore );
        }

    exit:
        return ;
    }

    //STEPS:
    //  foreach stream in the manifest.
    //      Get Available Tracks
    //      Call Select Tracks with All available tracks
    //      Get Selected Tracks
    //      Compare and Log Results
    void SelectTracksWithAllTracks(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;

        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );
        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->SelectTracks( availableTracks ) );

            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetSelectedTracks( &selectedTracks ) );

            PKTEST_ASSERT_EXIT( selectedTracks == availableTracks );
        }

    exit:
        return ;
    }

    //STEPS:
    //  foreach stream in the manifest.
    //      Get Selected Tracks
    //      Call Select Tracks with All selected tracks
    //      Get Selected Tracks
    //      Compare and Log Results
    void SelectTracksWithAlreadySelectedTracks(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracksBefore;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracksAfter;

        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );
        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetSelectedTracks( &selectedTracksBefore ) );

            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->SelectTracks( selectedTracksBefore ) );

            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetSelectedTracks( &selectedTracksAfter ) );

            PKTEST_ASSERT_EXIT( selectedTracksAfter == selectedTracksBefore );
        }

    exit:
        return ;
    }

    void SelectTracksWithRandomCameraAngle(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > selectedStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracksBefore;
        std::vector< AutoRefPtr<IManifestTrack> > proposedTracks;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracksAfter;
        std::vector< wstring > customAttributeNames, cameraAngleVector;
        wstring value;

        PKTEST_HRESULT_EXIT( pManifest->GetSelectedStreams( &selectedStreams ) );
        PKTEST_ASSERT_EXIT( selectedStreams.size() != 0 );

        for( size_t iStrm = 0; iStrm < selectedStreams.size(); ++iStrm )
        {
            if( selectedStreams[iStrm]->Type() == MediaStreamTypeVideo)
            {
                PKTEST_HRESULT_EXIT( selectedStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

                for (uint32_t i = 0; i < availableTracks.size(); i++)
                {
                    availableTracks[i]->GetCustomAttributeNames(&customAttributeNames);
                    PKTEST_ASSERT_MSG_EXIT(customAttributeNames.size() == 1, "we are expecting only one customAttribute" );
                    PKTEST_ASSERT_MSG_EXIT(customAttributeNames[0] == L"CameraAngle", "we are expecting only one customAttribute, that is CameraAngle" );
                    
                    PKTEST_ASSERT_MSG_EXIT(availableTracks[i]->GetCustomAttributeValue(L"CameraAngle", &value), "Custom Attribute value for CameraAngle Attribute not Found");
                    
                    vector<wstring>::const_iterator it = std::find( cameraAngleVector.begin(), cameraAngleVector.end(), value );
                    if ( it == cameraAngleVector.end() )
                    {
                        cameraAngleVector.push_back(value);
                    }
                }

                wstring selectedCameraAngle = cameraAngleVector[STUtilities::RandomIndex(cameraAngleVector.size())];

                for (uint32_t i = 0; i < availableTracks.size(); i++)
                {
                    availableTracks[i]->GetCustomAttributeNames(&customAttributeNames);
                    
                    PKTEST_ASSERT_MSG_EXIT(availableTracks[i]->GetCustomAttributeValue(L"CameraAngle", &value), "Custom Attribute value for CameraAngle Attribute not Found");
                    
                    if(value == selectedCameraAngle)
                    {
                        proposedTracks.push_back(availableTracks[i]);
                    }
                }

                PKTEST_HRESULT_EXIT( selectedStreams[iStrm]->SelectTracks( proposedTracks ) );
                PKTEST_HRESULT_EXIT( selectedStreams[iStrm]->GetSelectedTracks( &selectedTracksAfter ) );

                PKTEST_ASSERT_EXIT( selectedTracksAfter == proposedTracks );

            }
        }

    exit:
        return ;
    }

    ////////////////////////////////////////////////////
    //  SelectTracks Inside ManifestReady Tests
    ////////////////////////////////////////////////////

    //TEST: Select tracks with only few of the available tracks. And check to see the selected tracks change accordingly.
    PKTEST_METHOD_EX( VerifySelectTracksChangesSelectedTracks,
        PKTEST_PROPERTY("Priority", "0")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &SelectTracksTests::SelectTracksChangesSelectedTracks );

        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

exit:
        return;
    }

    //TEST: Call select tracks with none of the tracks selected. And verify that we have an error.
    PKTEST_METHOD_EX( VerifySelectTracksWithNoTracks,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &SelectTracksTests::SelectTracksWithNoTracks );

        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

exit:
        return;
    }

    //Call select tracks with all the tracks selected.
    PKTEST_METHOD_EX( VerifySelectTracksWithAllTracks,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &SelectTracksTests::SelectTracksWithAllTracks );


        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

exit:
        return;
    }

    //Call select tracks with the tracks selected which are already selected.
    PKTEST_METHOD_EX( VerifySelectTracksWithAlreadySelectedTracks,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &SelectTracksTests::SelectTracksWithAlreadySelectedTracks );


        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

exit:
        return;
    }

    //Call select tracks with the tracks with random Camera Angle
    PKTEST_METHOD_EX( VerifySelectTracksWithRandomCameraAngle,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &SelectTracksTests::SelectTracksWithRandomCameraAngle );


        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODMultiCamera") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

exit:
        return;
    }

    ////////////////////////////////////////////////////
    //  SelectTracks Outside ManifestReady Tests
    ////////////////////////////////////////////////////

    //TEST: Select tracks with only few of the available tracks. And check to see the selected tracks change accordingly.
    PKTEST_METHOD_EX( VerifySelectTracksChangesSelectedTracksOutsideMR,
        PKTEST_PROPERTY("Priority", "0")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &SelectTracksTests::DoNothingCallback );


        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

        PKTEST_FUNC_EXIT( SelectTracksChangesSelectedTracks(m_pSTWObject->GetManifest(), S_OK) );
exit:
        return;
    }

    //TEST: Call select tracks with none of the tracks selected. And verify that we have an error.
    PKTEST_METHOD_EX( VerifySelectTracksWithNoTracksOutsideMR,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &SelectTracksTests::DoNothingCallback );


        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

        PKTEST_FUNC_EXIT( SelectTracksWithNoTracks(m_pSTWObject->GetManifest(), S_OK) );

exit:
        return;
    }

    //Call select tracks with all the tracks selected.
    PKTEST_METHOD_EX( VerifySelectTracksWithAllTracksOutsideMR,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &SelectTracksTests::DoNothingCallback );


        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

        PKTEST_FUNC_EXIT( SelectTracksWithAllTracks(m_pSTWObject->GetManifest(), S_OK) );

exit:
        return;
    }

    //Call select tracks with the tracks selected which are already selected.
    PKTEST_METHOD_EX( VerifySelectTracksWithAlreadySelectedTracksOutsideMR,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &SelectTracksTests::DoNothingCallback );


        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODDefault") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

        PKTEST_FUNC_EXIT( SelectTracksWithAlreadySelectedTracks(m_pSTWObject->GetManifest(), S_OK) );

exit:
        return;
    }

    //Call select tracks with the tracks with random Camera Angle.
    PKTEST_METHOD_EX( VerifySelectTracksWithRandomCameraAngleOutsideMR,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &SelectTracksTests::DoNothingCallback );


        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODMultiCamera") );

        m_pSTWObject->OpenVideo(urlString, false);

        PKTEST_ASSERT_EXIT( m_pSTWObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT) );

        PKTEST_FUNC_EXIT( SelectTracksWithRandomCameraAngle(m_pSTWObject->GetManifest(), S_OK) );

exit:
        return;
    }
    
    string GetUrl(wstring keyName)
    {
        string url = SSPKHelpers::GetUrlFromList( m_Manifests, keyName.c_str() );

        return (url);
    }
};
