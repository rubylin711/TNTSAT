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

#include "ISmoothTransport.h"
#include "STWrapper.h"
#include "ManifestReadyCallbackImpl.h"

#include <algorithm>
#include "SSPKHelpers.h"
//#include "PKTestSuiteUtils.h"

#include "StringUtils.h"

using namespace SSPKTest;
using namespace SSPK;

PKTEST_GROUP( TrackAttributesTests )
{
    STWrapper               *m_pSTWObject;
    CXMLElementsList        m_Manifests;

    TrackAttributesTests()
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
        PKTEST_ASSERT_EXIT(m_pSTWObject != NULL);

        m_pSTWObject->SetManifestCallback(NULL);
        
        if(!m_pSTWObject->IsClosed())
        {
            m_pSTWObject->Close();
        }

        delete m_pSTWObject;
        m_pSTWObject = NULL;

        return true;
exit:

        return false;
    }

    ////////////////////////////////////
    //  Manifest Ready Callbacks
    ////////////////////////////////////
    void DoNothingCallback(IManifest* pManifest, pkRESULT hr)
    {
        //Do Nothing.
        return ;
    }

    //Attribute Testing
    PKTEST_METHOD_EX( VerifyTrackAttributes,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Bug", "28746"))
        
    {
        m_pSTWObject->SetManifestReadyMethod(this, &TrackAttributesTests::TrackAttributesCallback );

        PKTEST_FUNC_EXIT( string urlSting = GetUrl(L"VC1ODMultiAudioMultiVideoDifferentAttributes") );

        PKTEST_FUNC_EXIT( m_pSTWObject->OpenVideoAndValidate(urlSting, false, true ) );
    exit:
        return;
    }

    PKTEST_METHOD_EX( TrackSelection_CustomAttributes_CameraAnglePerTrack_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "0"))
    {
        m_pSTWObject->SetManifestReadyMethod( this, &TrackAttributesTests::TrackSelection_CustomAttributes_CameraAnglePerTrack_DuringManifestReady_Method );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( m_Manifests,L"H264ODCameraAnglePerTrack") );
        m_pSTWObject->OpenVideoAndValidate(urlString,false);
        
    exit:
        return;
    }
    void TrackSelection_CustomAttributes_CameraAnglePerTrack_DuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< wstring > customAttributeNames, cameraAngleVector;
        wstring value;
        bool videoStreamFound = false;

        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );
        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );
        
        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            if( availableStreams[iStrm]->Type() == MediaStreamTypeVideo )
            {
                videoStreamFound = true;
                PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

                for ( uint32_t j = 0 ;j < availableTracks.size(); j++ )
                {
                    availableTracks[j]->GetCustomAttributeNames(&customAttributeNames);
                    PKTEST_ASSERT_MSG_EXIT(customAttributeNames.size() == 1, "we are expecting only one customAttribute" );
                    PKTEST_ASSERT_MSG_EXIT(customAttributeNames[0] == L"CameraAngle", "we are expecting only one customAttribute, that is CameraAngle" );

                    PKTEST_ASSERT_MSG_EXIT(availableTracks[j]->GetCustomAttributeValue(customAttributeNames[0], &value), "No Custom Attribute value for CameraAngle Attribute Found");
                    std::vector< wstring >::const_iterator it = std::find( cameraAngleVector.begin(), cameraAngleVector.end(), value );

                    if ( it == cameraAngleVector.end() )
                    {
                        cameraAngleVector.push_back(value);
                    }
                    else
                    {
                        PKTEST_ASSERT_MSG_EXIT(false, "this customAttributeValue:%ls of name %ls is a duplicate.", value.c_str(), customAttributeNames[0].c_str());
                    }
                }
            }
        }
        PKTEST_ASSERT_MSG_EXIT(videoStreamFound,"cannot find a video Stream" );

    exit:
        return ;
    }

    PKTEST_METHOD_EX( TrackSelection_CustomAttributes_NoCustomAttributes_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
         m_pSTWObject->SetManifestReadyMethod( this, &TrackAttributesTests::TrackSelection_CustomAttributes_NoCustomAttributes_DuringManifestReady_Method );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( m_Manifests,L"H264ODDefault") );
        m_pSTWObject->OpenVideoAndValidate(urlString,false);
        
    exit:
        return;
    }
    void TrackSelection_CustomAttributes_NoCustomAttributes_DuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::vector< wstring > customAttributeNames, cameraAngleVector;
        wstring value;
        bool videoStreamFound = false;

        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );
        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );
        
        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            if( availableStreams[iStrm]->Type() == MediaStreamTypeVideo )
            {
                videoStreamFound = true;
                PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

                for ( uint32_t j = 0 ;j < availableTracks.size(); j++ )
                {
                    availableTracks[j]->GetCustomAttributeNames(&customAttributeNames);
                    PKTEST_ASSERT_MSG_EXIT(customAttributeNames.size() == 0, "we are expecting no customAttributes" );
                    
                    PKTEST_ASSERT_MSG_EXIT(!availableTracks[j]->GetCustomAttributeValue(L"CameraAngle", &value), "Custom Attribute value for CameraAngle Attribute Found");
                    PKTEST_ASSERT_MSG_EXIT(value == L"","value for a non existing custom attribute should be emtpy");
                }
            }
        }
        PKTEST_ASSERT_MSG_EXIT(videoStreamFound,"cannont find a video Stream" );

    exit:
        return ;
    }


    void TrackAttributesCallback(IManifest* pManifest, pkRESULT hr)
    {
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        std::wstring streamName;
        int32_t totalTracks = 0;
        
        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );
        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        //This manifest should have 2 audio and 2 video streams, verify it!
        PKTEST_FUNC_EXIT( SSPKHelpers::CheckStreamCountByType(availableStreams, 2, 2, 0) );

        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {            
            PKTEST_HRESULT_EXIT( availableStreams[iStrm]->GetAvailableTracks( &availableTracks ) );

            streamName = availableStreams[iStrm]->Name();

            //Verify attributes for individual tracks.
            if( streamName == L"video_eng" )
            {
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckTracksCount(availableStreams[iStrm], 5) );

                PKTEST_FUNC_EXIT( SSPKHelpers::CheckVideoTrackAttributes(availableTracks[0], 0, 331000, L"WVC1", 0, 160, 212, 4, 0, L"250000010FC3CA06904F8A069813E80C90808A1950CF400000010E5A67F840") );
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckVideoTrackAttributes(availableTracks[1], 0, 688000, L"WVC1", 0, 256, 340, 3, 0, L"250000010FC3D40A907F8A0A981FE80C908114FED3FBC00000010E5A67F840") );
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckVideoTrackAttributes(availableTracks[2], 0, 991000, L"WVC1", 0, 336, 448, 2, 0, L"250000010FCBDE0DF0A78A0DF829E80C90811E3DF8F8400000010E5A67F840") );
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckVideoTrackAttributes(availableTracks[3], 0, 1427000, L"WVC1", 0, 432, 576, 1, 0, L"250000010FCBEC11F0D78A11F835E80C9081AB8BD718400000010E5A67F840") );
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckVideoTrackAttributes(availableTracks[4], 0, 2962000, L"WVC1", 0, 720, 960, 0, 0, L"250000010FD3FE1DF1678A1DF859E80C90825A645A64400000010E5A67F840") );

                totalTracks += 5;
            }
            else if( streamName == L"video_fra" )
            {
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckTracksCount(availableStreams[iStrm], 5) );

                PKTEST_FUNC_EXIT( SSPKHelpers::CheckVideoTrackAttributes(availableTracks[0], 0, 331000, L"WVC1", 0, 150, 202, 4, 0, L"250000010FC3CA06904F8A069813E80C90808A1950CF400000010E5A67F830") );
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckVideoTrackAttributes(availableTracks[1], 0, 688000, L"WVC1", 0, 246, 330, 3, 0, L"250000010FC3D40A907F8A0A981FE80C908114FED3FBC00000010E5A67F830") );
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckVideoTrackAttributes(availableTracks[2], 0, 991000, L"WVC1", 0, 326, 438, 2, 0, L"250000010FCBDE0DF0A78A0DF829E80C90811E3DF8F8400000010E5A67F830") );
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckVideoTrackAttributes(availableTracks[3], 0, 1427000, L"WVC1", 0, 422, 566, 1, 0, L"250000010FCBEC11F0D78A11F835E80C9081AB8BD718400000010E5A67F830") );
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckVideoTrackAttributes(availableTracks[4], 0, 2962000, L"WVC1", 0, 710, 950, 0, 0, L"250000010FD3FE1DF1678A1DF859E80C90825A645A64400000010E5A67F830") );

                totalTracks += 5;
            }
            else if( streamName == L"audio_fra" )
            {
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckTracksCount(availableStreams[iStrm], 1) );
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckAudioTrackAttributes(availableTracks[0], 64000, 354, L"1000030000000000000000000000E00042C0") );

                totalTracks += 1;
            }
            else if( streamName == L"audio_eng" )
            {
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckTracksCount(availableStreams[iStrm], 1) );
                PKTEST_FUNC_EXIT( SSPKHelpers::CheckAudioTrackAttributes(availableTracks[0], 64000, 354, L"1000030000000000000000000000E00042C0") );

                totalTracks += 1;
            }
            else
            {
                PKTEST_HRESULT_EXIT(E_UNEXPECTED);
                //This means possibly incorrect test data!
            }
        }
        //This test manifest, video +Audio!
        PKTEST_ASSERT_EXIT(12 == totalTracks);

    exit:
        return ;
    }



private:

    //"expectedResult == true" means manifestready event is expected.
    void WaitForManifestReady(bool expectedResult)
    {
        bool waitResult = m_pSTWObject->WaitForManifestReady( MANIFESTREADY_TIMEOUT );

        if(waitResult)
        {
            PKTEST_MSG("Received manifest ready Event.");
        }
        else 
        {
            PKTEST_MSG("Timeout or some failure happened while waiting for the manifest ready.");
        }

        if(waitResult == expectedResult)
        {
            PKTEST_MSG("Verification Succeeded");
        }
        else
        {
            PKTEST_HRESULT_MSG_EXIT( E_UNEXPECTED, 
                                    "Err! Manifest ready, Verification failed. (waitResult=%d, expectedResult=%d)", waitResult, expectedResult);
        }

exit:
        return;
    }

    string GetUrl(wstring keyName)
    {
        string url = SSPKHelpers::GetUrlFromList( m_Manifests, keyName.c_str() ) ;

        return (url);
    }

    wstring VerifyTrackAttribute(AutoRefPtr<IManifestTrack> track, wstring name, uint32_t expectedValue)
    {
        return VerifyTrackAttribute( track, name, toWString(expectedValue) );
    }

    wstring VerifyTrackAttribute(AutoRefPtr<IManifestTrack> track, wstring name, wstring expectedValue)
    {
        wstring value ;

        PKTEST_ASSERT_MSG_EXIT( track->GetAttribute(name, &value), 
                                "%ls . Attribute not found in track!", 
                                name.c_str() );

        PKTEST_ASSERT_MSG_EXIT( value == expectedValue , 
                                "%ls=%ls . Attribute value is not as expected(%s)", 
                                name.c_str(), value.c_str(), expectedValue.c_str() );
exit:
        return value;
    }



};
