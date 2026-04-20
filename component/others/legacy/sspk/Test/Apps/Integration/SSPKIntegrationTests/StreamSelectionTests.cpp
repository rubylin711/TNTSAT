///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PKTestSuite.h"
#include "PKTestSuiteUtils.h"
#include "STWrapper.h"
#include "ManifestReadyCallbackImpl.h"
#include "StringUtils.h"
#include "SSPKHelpers.h"
#include <sstream>
#include <algorithm>

using namespace SSPK;
using namespace SSPKTest;

PKTEST_GROUP( StreamSelectionTests )
{

    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;

    StreamSelectionTests()
    {
        CXMLElement xTestData = PKTest_GetDataTable( "IntegrationTestSources" );
        if( !xTestData.IsNull() )
        {
             _xManifests = xTestData.Elements(L"source");
        }
    }

    bool TestSetup()
    {
        PKTEST_ASSERT_MSG_EXIT(_xManifests.Length() > 0,"Couldn't find any elements with name 'source'");

        _smoothObject = NEW_NO_THROW STWrapper();
        PKTEST_ASSERT_EXIT(NULL != _smoothObject);
        return true;

    exit:
        return false;
    }

    bool TestCleanup()
    {
        pkRESULT pkResult = pkS_OK;

        _smoothObject->SetManifestCallback(NULL);
        if(!_smoothObject->IsClosed() )
        {
            pkResult = _smoothObject->Close();
        }
        delete _smoothObject;
        return SUCCEEDED(pkResult);
    }

    bool ToggleAudioAndValidate()
    {
        VectorOfStreams proposedSelection;
        HRESULT hr;

        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);

        hr = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure.");
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

        return true;
    exit:
        return false;
    }

    //**********************************************************************************************************************************

    ////////////// AvailableStreams Tests //////////////
    ////////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_AvailableStreams_MultipleAudio_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "0"))
    {
        _smoothObject->SetManifestReadyMethod( this, &StreamSelectionTests::StreamSel_AvailableStreams_MultipleAudio_DuringManifestReady_Method );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_AvailableStreams_MultipleAudio_DuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        PKTEST_FUNC_EXIT(CheckAvailableStreamCountByType(3,1,1) );

    exit:
        return ;
    }

    ////////////////////////////////////////////////////
    PKTEST_METHOD_EX(StreamSel_AvailableStreams_MultipleAudioMultiText_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "0"))
    {
        _smoothObject->SetManifestReadyMethod( this, &StreamSelectionTests::StreamSel_AvailableStreams_MultipleAudioMultiText_DuringManifestReady_Method );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudioMultiText") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_AvailableStreams_MultipleAudioMultiText_DuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        PKTEST_FUNC_EXIT(CheckAvailableStreamCountByType(2,1,2) );

    exit:
        return ;
    }

    PKTEST_METHOD_EX( StreamSel_AvailableStreams_AudioOnly,
        PKTEST_PROPERTY("Priority", "2"))
    {
        vector<SmoothTransportError> smoothErrorVector;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODAudioOnly") );
        PKTEST_HRESULT_EXIT( _smoothObject->OpenVideo(urlString, false, false) );
        PKTEST_ASSERT_MSG_EXIT(!_smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "should not raise ManifestReady for error case" );

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found multiple or no errors in the list" );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ManifestInvalid, "ManifestInvalid expected, but not observed some other error." );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_AvailableStreams_NonExistingParent_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::SparseStreams_AvailableStreams_NonExistingParent_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODMultiSparseParentedWithAudioAndVideo") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void SparseStreams_AvailableStreams_NonExistingParent_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams availableStreams;
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );

        for( uint32_t i = 0; i<availableStreams.size(); i++ )
        {
            if( availableStreams[i]->Type() == MediaStreamTypeText )
            {
                IManifestStream* parentStream ;
                availableStreams[i]->GetParentStream(&parentStream);
                if(NULL != parentStream)
                {
                    VectorOfStreams::const_iterator it = std::find( availableStreams.begin(), availableStreams.end(), parentStream);
                    PKTEST_ASSERT_MSG_EXIT( it !=availableStreams.end(), "A child Stream with non existing parent is found in available streams" );
                }
            }
        }

    exit:
        return ;
    }

    //*********************************************************************************************************************************

    //////////////// SelectedStreams Tests /////////////
    ////////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectedStreams_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "0"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectedStreams_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"VC1ODMultiAudioMultiVideo") );
        PKTEST_HRESULT_EXIT( _smoothObject->OpenVideo(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectedStreams_DuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return ;
    }

    ////////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectedStreams_MultipleAudio_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectedStreams_MultipleAudio_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_HRESULT_EXIT( _smoothObject->OpenVideo(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectedStreams_MultipleAudio_DuringManifestReady_Method(IManifest* pManifest,  pkRESULT hr)
    {
        // still need to discuss about if the text streams selected by default of not.
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return ;
    }

    ////////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectedStreams_MultipleAudioMultiText_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "0"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectedStreams_MultipleAudioMultiText_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudioMultiText") );
        PKTEST_HRESULT_EXIT( _smoothObject->OpenVideo(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectedStreams_MultipleAudioMultiText_DuringManifestReady_Method(IManifest* pManifest,  pkRESULT hr)
    {
        // still need to discuss about if the text streams selected by default of not.
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return ;
    }

    ////////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectedStreams_DIffOrder_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "2"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectedStreams_DiffOrder_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefaultDiffOrder") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectedStreams_DiffOrder_DuringManifestReady_Method(IManifest* pManifest,  pkRESULT hr)
    {
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        StreamSel_SelectedStreams_AudioOnly_AfterManifestReady_DefaultSelection,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY( "Ignore", "true" )
        PKTEST_PROPERTY( "Bug", "25608" )
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODAudioOnly") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsPlaying(), "The content is not playing." );

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection());

    exit:
        return;
    }

    ////////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectedStreams_MultiVideo_MultipleAudio_AfterManifestReady_DefaultSelection,
        PKTEST_PROPERTY("Priority", "1"))
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"VC1ODMultiAudioMultiVideo") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsPlaying(), "The content is not playing." );

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return;
    }

    //*********************************************************************************************************************************

    //////////////// SelectStreams Tests //////////////
    ///////////////////////////////////////////////////

    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleAudio_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "0"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectStreams_ToggleAudio_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectStreams_ToggleAudio_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;
        vector<StreamChangedEventArgs> streamSelectionArgs;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );
        streamSelectionArgs = _smoothObject->GetStreamSelectionArgs();
        PKTEST_ASSERT_MSG_EXIT(streamSelectionArgs.size() == 2, "Expecting 2 stream selected args but observed different." );
        
        PKTEST_ASSERT_MSG_EXIT(streamSelectionArgs[0].Action == StreamChangedEventArgs::StreamDeselected, "Unexpected stream selection action encountered for stream : %ls", streamSelectionArgs[0].pStream->Name().c_str() );
        PKTEST_ASSERT_MSG_EXIT(streamSelectionArgs[1].Action == StreamChangedEventArgs::StreamSelected, "Unexpected stream selection action encountered for stream : %ls", streamSelectionArgs[1].pStream->Name().c_str() );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleAudio_AfterManifestReady, 
        PKTEST_PROPERTY("Priority", "0") )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        StreamSel_SelectStreams_ToggleAllAudio,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1ODMultiAudio", "VC1ODMultiAudio" )
        PKTEST_PROPERTY("Data:VC1LiveMultiAudio", "VC1LiveMultiAudio" )
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio" )
        PKTEST_PROPERTY("Data:H264LiveMultiAudioTextWithLargeChunkDuration", "H264LiveMultiAudioTextWithLargeChunkDuration" )
        PKTEST_PROPERTY("Data:H264ODMultiAudioMultiText", "H264ODMultiAudioMultiText" )
        PKTEST_PROPERTY("Data:H264ODMultiAudio2", "H264ODMultiAudio2" )
        PKTEST_PROPERTY("Data:VC1ODMultiAudioMultiVideo", "VC1ODMultiAudioMultiVideo" )
        PKTEST_PROPERTY("Data:ODMultiSparseParentedWithAudioAndVideo", "ODMultiSparseParentedWithAudioAndVideo" )
        PKTEST_PROPERTY("Data:ODMultiSparseParentedWithAudio", "ODMultiSparseParentedWithAudio" )
        PKTEST_PROPERTY("Data:LiveNonSparseTextStream", "LiveNonSparseTextStream" )
        PKTEST_PROPERTY("Data:LiveToVodDefault", "LiveToVodDefault" )
        PKTEST_PROPERTY("Data:LiveToVodMultiSegments", "LiveToVodMultiSegments" )
        )
    {
        VectorOfStreams availableStreams;
        VectorOfStreams selectedStreams;
        VectorOfStreams proposedSelection;
        int32_t audioStreamCount = 0;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData() ) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        
        for(size_t i = 0; i < availableStreams.size(); i++ )
        {
            if( availableStreams[i]->Type() == MediaStreamTypeAudio )
            {
                audioStreamCount++;
            }
        }

        for(int32_t audioIndex = 0; audioIndex < audioStreamCount; audioIndex++)
        {
            PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

            PKTEST_FUNC_EXIT( proposedSelection =  SSPKHelpers::SelectNextAudioStream( availableStreams, selectedStreams ) );
            
            hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
            if (SUCCEEDED(hr2) )
            {
                PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
            }
            else
            {
                PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
            }

            PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );
            _smoothObject->Delay(VALIDATION_DELAY);

            PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsPlaying(), "should be in playing state after audio change.");
        }


    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleVideo_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "2") )
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectStreams_ToggleVideo_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"VC1ODMultiAudioMultiVideo") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectStreams_ToggleVideo_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeVideo);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        // Change the code below based on where the error shows up.
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }
        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( 
        StreamSel_SelectStreams_ToggleVideo_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY( "Bug", "25710")
        )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"VC1ODMultiAudioMultiVideo") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeVideo);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        // Change the code below based on where the error shows up.
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Toggling video streams should fail, but did not." );
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_SelectionCallbackCount,
        PKTEST_PROPERTY("Priority", "0"))
    {

        VectorOfStreams proposedSelection;
        HRESULT hr2;
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectStreams_SelectionCallbackCount_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->GetStreamSelectionCount() == 1, "Stream selection callback event count is not 1" );

        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

        PKTEST_ASSERT_MSG_EXIT( _smoothObject->GetStreamSelectionCount() == 2, "Stream selection callback event count is not 2" );

    exit:
        return;
    }
    void StreamSel_SelectStreams_SelectionCallbackCount_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        CheckDefaultStreamSelection();
        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleAudio_AfterManifestReady_Playing,
        PKTEST_PROPERTY("Priority", "0"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264LiveMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

        PKTEST_CHECK_FAIL_EXIT();
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        ManifestParser_LiveTextStreamWithlargeChunkDuration,
        PKTEST_PROPERTY("Priority", "2")
        )
    {

        VectorOfStreams proposedSelection;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264LiveMultiAudioTextWithLargeChunkDuration") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

    exit:
        return;
    }

    /////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleAudio_AfterManifestReady_Paused,
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );

        PKTEST_CHECK_FAIL_EXIT();
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleAudio_AfterManifestReady_FastForward,
        PKTEST_PROPERTY("Priority", "2"))
    {
        VectorOfStreams proposedSelection;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );
        PKTEST_HRESULT_EXIT( _smoothObject->FastForward() );
        _smoothObject->Delay(3000);

        PKTEST_CHECK_FAIL_EXIT();
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "First audio toggle failed" );

        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "Second audio toggle failed" );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleAudio_AfterManifestReady_Rewind,
        PKTEST_PROPERTY("Priority", "2"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY*5) );
        PKTEST_HRESULT_EXIT( _smoothObject->Rewind() );
        _smoothObject->Delay(3000);
        PKTEST_CHECK_FAIL_EXIT();

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure.") ;
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( 
        StreamSel_SelectStreams_ToggleAudio_AfterManifestReady_Closed,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY( "Bug", "25789" )
        )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264LiveMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

        PKTEST_CHECK_FAIL_EXIT();
        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);

        PKTEST_HRESULT_EXIT( _smoothObject->Close() );
        _smoothObject->Delay(VALIDATION_DELAY);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Select Stream Async on a closed smoothtransport should fail");

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_MultiAudio_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectStreams_MultiAudio_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectStreams_MultiAudio_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = SelectAllStreamsByMediaType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Selecting multiple audio streams should fail, but did not.");


        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_MultiAudio_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_CHECK_FAIL_EXIT();

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = SelectAllStreamsByMediaType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Selecting multiple audio streams should fail, but did not.");
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_MultiVideo_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectStreams_MultiVideo_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"VC1ODMultiAudioMultiVideo") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectStreams_MultiVideo_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = SelectAllStreamsByMediaType(MediaStreamTypeVideo);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Selecting multiple video streams should fail, but did not." );

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return ;
    }

        ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleAudio_AfterManifestReady_MultipleAudioSwitchesDuringFastForward,
        PKTEST_PROPERTY("Priority", "2"))
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_HRESULT_EXIT( _smoothObject->OpenVideo(urlString,true) );

        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The first audio stream selection failed");

        _smoothObject->MakeRoomForFastForward();
        PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(LOW_SPEED, VALIDATION_DELAY) );

        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The first audio stream selection after fast forward failed");
        _smoothObject->Delay( VALIDATION_DELAY );
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The second audio stream selection after fast forward failed");

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleAudio_AfterManifestReady_MultipleAudioSwitchesDuringRewind,
        PKTEST_PROPERTY("Priority", "2"))
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_HRESULT_EXIT( _smoothObject->OpenVideo(urlString,true) );

        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

        PKTEST_CHECK_FAIL_EXIT();
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The first audio stream selection failed");

        _smoothObject->MakeRoomForRewind();
        PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(LOW_SPEED, VALIDATION_DELAY) );

        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The first audio stream selection after rewind failed");
        _smoothObject->Delay( VALIDATION_DELAY );
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The second audio stream selection after rewind failed");

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleAudio_AfterManifestReady_MultipleAudioSwitchesAfterFastForward,
        PKTEST_PROPERTY("Priority", "2"))
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_HRESULT_EXIT( _smoothObject->OpenVideo(urlString,true) );

        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

        PKTEST_CHECK_FAIL_EXIT();
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The first audio stream selection failed");

        _smoothObject->MakeRoomForFastForward();
        PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY) );
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The first audio stream selection after fast forward failed");
        _smoothObject->Delay( VALIDATION_DELAY );
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The second audio stream selection after fast forward failed");

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleAudio_AfterManifestReady_MultipleAudioSwitchesAfterRewind,
        PKTEST_PROPERTY("Priority", "2"))
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_HRESULT_EXIT( _smoothObject->OpenVideo(urlString,true) );

        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

        PKTEST_CHECK_FAIL_EXIT();
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The first audio stream selection failed");

        _smoothObject->MakeRoomForRewind();
        PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY) );
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The first audio stream selection after rewind failed");
        _smoothObject->Delay( VALIDATION_DELAY );
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The second audio stream selection after rewind failed");

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleAudio_AfterManifestReady_MultipleAudioSwitchesDuringPlayAfterMediaEnded,
        PKTEST_PROPERTY("Priority", "2"))
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_HRESULT_EXIT( _smoothObject->OpenVideo(urlString,true) );

        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

        PKTEST_CHECK_FAIL_EXIT();
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The first audio stream selection failed");

        //Rewind to beginning
        PKTEST_HRESULT_EXIT( _smoothObject->Rewind() );
        PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, VALIDATION_DELAY));

        //Play again and make sure that audio switching still works
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The first audio stream selection after media ended failed");
        _smoothObject->Delay( VALIDATION_DELAY );
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The second audio stream selection after media ended failed");

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleAudio_AfterManifestReady_MultipleAudioSwitchesAfterMediaEnded,
        PKTEST_PROPERTY("Priority", "2"))
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_HRESULT_EXIT( _smoothObject->OpenVideo(urlString,true) );

        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

        PKTEST_CHECK_FAIL_EXIT();
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The first audio stream selection failed");

        //Rewind to beginning
        PKTEST_HRESULT_EXIT( _smoothObject->Rewind() );
        PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, VALIDATION_DELAY));

        //Make sure that audio switching still works
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The first audio stream selection after media ended failed");
        _smoothObject->Delay( VALIDATION_DELAY );
        PKTEST_ASSERT_MSG_EXIT( ToggleAudioAndValidate(), "The second audio stream selection after media ended failed");

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_MultiVideo_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"VC1ODMultiAudioMultiVideo") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_CHECK_FAIL_EXIT();

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = SelectAllStreamsByMediaType(MediaStreamTypeVideo);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Selecting multiple video streams should fail, but did not.");
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_Live_DuringManifest,
        PKTEST_PROPERTY("Priority", "0"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectStreams_Live_DuringManifest_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264LiveMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectStreams_Live_DuringManifest_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_Live_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "0"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264LiveMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_CHECK_FAIL_EXIT();

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_AlreadySelected_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectStreams_AlreadySelected_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectStreams_AlreadySelected_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;
        vector<StreamChangedEventArgs> streamSelectionArgs;
        VectorOfStreams selectedStreams;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, selectedStreams);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        streamSelectionArgs = _smoothObject->GetStreamSelectionArgs();
        PKTEST_ASSERT_MSG_EXIT(streamSelectionArgs.size() == 0, "When selected already selected streams, the stream selection args count should be 0" );

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_AlreadySelected_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;
        vector<StreamChangedEventArgs> streamSelectionArgs;
        VectorOfStreams selectedStreams;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, selectedStreams);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        streamSelectionArgs = _smoothObject->GetStreamSelectionArgs();
        PKTEST_ASSERT_MSG_EXIT(streamSelectionArgs.size() == 0, "When selected already selected streams, the stream selection args count should be 0" );

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return;
    }

        ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_StreamSelectionArgsOrder_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectStreams_StreamSelectionArgsOrder_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"VC1ODMultiAudioMultiVideo") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectStreams_StreamSelectionArgsOrder_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;
        vector<StreamChangedEventArgs> streamSelectionArgs;
        proposedSelection = SelectFirstStreamOfAudioAndVideo();

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        // Change the code below based on where the error shows up.
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        streamSelectionArgs = _smoothObject->GetStreamSelectionArgs();
        PKTEST_ASSERT_MSG_EXIT(streamSelectionArgs.size() == 4, "Expecting 4 stream selected args but observed different." );
        PKTEST_ASSERT_MSG_EXIT(streamSelectionArgs[0].Action == StreamChangedEventArgs::StreamDeselected, "Unexpected stream selection action encountered for stream : %ls", streamSelectionArgs[0].pStream->Name().c_str() );
        PKTEST_ASSERT_MSG_EXIT(streamSelectionArgs[1].Action == StreamChangedEventArgs::StreamDeselected, "Unexpected stream selection action encountered for stream : %ls", streamSelectionArgs[1].pStream->Name().c_str() );
        PKTEST_ASSERT_MSG_EXIT(streamSelectionArgs[2].Action == StreamChangedEventArgs::StreamSelected, "Unexpected stream selection action encountered for stream : %ls", streamSelectionArgs[2].pStream->Name().c_str() );
        PKTEST_ASSERT_MSG_EXIT(streamSelectionArgs[3].Action == StreamChangedEventArgs::StreamSelected, "Unexpected stream selection action encountered for stream : %ls", streamSelectionArgs[3].pStream->Name().c_str() );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_StreamSelectionArgsOrder_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;
        vector<StreamChangedEventArgs> streamSelectionArgs;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"VC1ODMultiAudioMultiVideo") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);

        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);
        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);

        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        streamSelectionArgs = _smoothObject->GetStreamSelectionArgs();
        PKTEST_ASSERT_MSG_EXIT(streamSelectionArgs.size() == 2, "Expecting 2 stream selected args but observed different." );
        PKTEST_ASSERT_MSG_EXIT(streamSelectionArgs[0].Action == StreamChangedEventArgs::StreamDeselected, "Unexpected stream selection action encountered for stream : %ls", streamSelectionArgs[0].pStream->Name().c_str() );
        PKTEST_ASSERT_MSG_EXIT(streamSelectionArgs[1].Action == StreamChangedEventArgs::StreamSelected, "Unexpected stream selection action encountered for stream : %ls", streamSelectionArgs[1].pStream->Name().c_str() );
    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_SelectAudioOnly_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "2"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectStreams_SelectAudioOnly_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectStreams_SelectAudioOnly_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = SelectAllStreamsByMediaType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Selecting just a audio stream from a content with video stream too should fail." );
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_SelectAudioOnly_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "2"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_CHECK_FAIL_EXIT();

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = SelectAllStreamsByMediaType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Selecting just a audio stream from a content with video stream too should fail." );
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_SelectVideoOnly_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectStreams_SelectVideoOnly_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectStreams_SelectVideoOnly_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = SelectAllStreamsByMediaType(MediaStreamTypeVideo);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Selecting just a video stream from a content with audio stream too should fail." );
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_SelectVideoOnly_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_CHECK_FAIL_EXIT();

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = SelectAllStreamsByMediaType(MediaStreamTypeVideo);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Selecting just a video stream from a content with audio stream too should fail." );

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_NullVector_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "2"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectStreams_NullVector_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectStreams_NullVector_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Calling select streams with null input should fail." );
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_NullVector_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "2"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_CHECK_FAIL_EXIT();
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Calling select streams with null input should fail.");
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return;
    }

    /////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_NullStream_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "2"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectStreams_NullStream_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectStreams_NullStream_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = SelectFirstStreamOfAudioAndVideo();
        proposedSelection.push_back(AutoRefPtr<IManifestStream>(NULL) );

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Calling select streams with null stream should fail." );
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_NullStream_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "2"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_CHECK_FAIL_EXIT();

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = SelectFirstStreamOfAudioAndVideo();
        proposedSelection.push_back(AutoRefPtr<IManifestStream>(NULL) );

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Calling select streams with null stream should fail.") ;
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_DeselectAll_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "0"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_SelectStreams_DeselectAll_DuringManifestReady_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_SelectStreams_DeselectAll_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = SelectFirstStreamOfAudioAndVideo();
        proposedSelection.clear();

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Calling select streams with empty streams should fail.");
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_DeselectAll_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_CHECK_FAIL_EXIT();

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = SelectFirstStreamOfAudioAndVideo();
        proposedSelection.clear();

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Calling select streams with empty streams should fail." );
        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

    exit:
        return;
    }
    
    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_SelectNonSparseTextStream,
        PKTEST_PROPERTY("Priority", "0"))
    {
        VectorOfStreams proposedSelection;
        IManifestStream* proposedTextStream = NULL;
        HRESULT hr, hr2;
        ChunkIterator itChunk;
        CHUNK_INFO chunkInfo;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveNonSparseTextStream") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_CHECK_FAIL_EXIT();

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        proposedSelection = AddTextStream(false);

        for( uint32_t i = 0; i< proposedSelection.size(); i++ )
        {
            if( MediaStreamTypeText == proposedSelection[i]->Type() )
            {
                proposedTextStream = proposedSelection[i];
            }
        }

        PKTEST_ASSERT_MSG_EXIT(NULL != proposedTextStream, "No Text streams are proposed(Probably no Non-Sparse Text Streams are found in the mainfest)");

        itChunk = proposedTextStream->GetFirstInCurrentChunkList();
        hr = proposedTextStream->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr), "TryGetChunkInfo should not work for the non selected text stream.");

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

        itChunk = proposedTextStream->GetFirstInCurrentChunkList();
        hr = proposedTextStream->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED(hr), "TryGetChunkInfo should work for the selected text stream.");

    exit:
        return;
    }
    
    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_SelectStreams_ToggleAudio_AfterManifestReadyNoDelay,
        PKTEST_PROPERTY("Priority", "0"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr = S_OK;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);

        hr = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

        exit:
            return;
    }

    //*********************************************************************************************************************************

    //////////////// GetAttribute Tests ///////////////
    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_GetAttribute_Basic, 
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Bug", "28746"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_GetAttribute_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"VC1ODMultiAudioMultiVideoDifferentAttributes") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_GetAttribute_Method(IManifest* pManifest,  pkRESULT hr)
    {

        VectorOfStreams availableStreams;

        PKTEST_FUNC_EXIT(CheckAvailableStreamCountByType(2,2,0) );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );

        PKTEST_FUNC_EXIT( SSPKHelpers::CheckStreamAttributes( availableStreams[0], TIMESCALE_10MHZ, "", 960,720,960,720, "QualityLevels({bitrate})/Fragments(video_eng={start time})", "video_eng", MediaStreamTypeVideo, "") );
        PKTEST_FUNC_EXIT( SSPKHelpers::CheckStreamAttributes( availableStreams[1], TIMESCALE_10MHZ, "", 950,710,950,710, "QualityLevels({bitrate})/Fragments(video_fra={start time})", "video_fra", MediaStreamTypeVideo, "") );
        PKTEST_FUNC_EXIT( SSPKHelpers::CheckStreamAttributes( availableStreams[2], TIMESCALE_10MHZ, "fra", 0,0,0,0,"QualityLevels({bitrate})/Fragments(audio_fra={start time})","audio_fra",MediaStreamTypeAudio,"") );
        PKTEST_FUNC_EXIT( SSPKHelpers::CheckStreamAttributes( availableStreams[3], TIMESCALE_10MHZ, "eng", 0,0,0,0,"QualityLevels({bitrate})/Fragments(audio_eng={start time})","audio_eng",MediaStreamTypeAudio,"") );


    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_GetAttribute_NoName, 
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Bug", "28746"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_GetAttribute_NoName_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODStreamNoname") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_GetAttribute_NoName_Method(IManifest* pManifest,  pkRESULT hr)
    {
        VectorOfStreams availableStreams;
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_FUNC_EXIT(SSPKHelpers::CheckStreamAttributes(availableStreams[0],TIMESCALE_10MHZ,"" ,852,480,852,480,"QualityLevels({bitrate})/Fragments(video={start time})","",MediaStreamTypeVideo,"") );
    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_GetAttribute_NoWidth, 
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Bug", "28746"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_GetAttribute_NoWidth_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODStreamNoWidth") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_GetAttribute_NoWidth_Method(IManifest* pManifest,  pkRESULT hr)
    {
        VectorOfStreams availableStreams;
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_FUNC_EXIT(SSPKHelpers::CheckStreamAttributes(availableStreams[0],TIMESCALE_10MHZ,"" ,852,480,852,480,"QualityLevels({bitrate})/Fragments(video={start time})","video",MediaStreamTypeVideo,"") );
    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_GetAttribute_NoType, 
        PKTEST_PROPERTY("Priority", "2"))
    {
        vector<SmoothTransportError> smoothErrorVector;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODStreamNoType") );
        PKTEST_HRESULT_EXIT( _smoothObject->OpenVideo(urlString, false, false) );
        PKTEST_ASSERT_MSG_EXIT(!_smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "should not raise ManifestReady for error case" );

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found multiple or no errors in the list" );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ManifestParseFailed, "ManifestParseFailed expected, but not observed some other error." );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_GetAttribute_NoUrl, 
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Bug", "28746"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_GetAttribute_NoUrl_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODStreamNoUrl") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_GetAttribute_NoUrl_Method(IManifest* pManifest,  pkRESULT hr)
    {
        VectorOfStreams availableStreams;
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_FUNC_EXIT(SSPKHelpers::CheckStreamAttributes(availableStreams[0],TIMESCALE_10MHZ,"" ,852,480,852,480,"","video",MediaStreamTypeVideo,"") );
    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( StreamSel_GetAttribute_AudWithWidth, 
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Bug", "28746"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::StreamSel_GetAttribute_AudWithWidth_Method);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODStreamAudWithWidth") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void StreamSel_GetAttribute_AudWithWidth_Method(IManifest* pManifest,  pkRESULT hr)
    {
        VectorOfStreams availableStreams;
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_FUNC_EXIT(SSPKHelpers::CheckStreamAttributes(availableStreams[1],TIMESCALE_10MHZ,"" ,0,0,0,0,"QualityLevels({bitrate})/Fragments(audio={start time})","audio",MediaStreamTypeAudio,"") );
    exit:
        return ;
    }


    //*********************************************************************************************************************************

    //////////////// Sparse Streams Selection Tests ///////////////
    ///////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_SelectStreams_DuringManifestReady_Basic, 
        PKTEST_PROPERTY("Priority", "0") )
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::SparseStreams_SelectStreams_DuringManifestReady_Basic_Method);

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamDefault");
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void SparseStreams_SelectStreams_DuringManifestReady_Basic_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;
        vector<StreamChangedEventArgs> streamSelectionArgs;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );
        proposedSelection = AddTextStream(true);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_SelectStreams_AfterManifestReady_Basic,
        PKTEST_PROPERTY("Priority", "0"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamDefault");
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        proposedSelection = AddTextStream(true);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_SelectStreams_SelectParentWithOutChild_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::SparseStreams_SelectStreams_SelectParentWithOutChild_DuringManifestReady_Method);

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamDefault");
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void SparseStreams_SelectStreams_SelectParentWithOutChild_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        // select parent with out selecting child
        proposedSelection = SelectFirstStreamOfAudioAndVideo();

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_SelectStreams_SelectParentWithOutChild_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamDefault");
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        // select parent with out selecting child
        proposedSelection = SelectFirstStreamOfAudioAndVideo();

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_SelectStreams_SelectChildWithOutParent_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::SparseStreams_SelectStreams_SelectChildWithOutParent_DuringManifestReady_Method);

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODMultiSparseParentedWithAudioAndVideo");
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void SparseStreams_SelectStreams_SelectChildWithOutParent_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        proposedSelection = AddTextStream(false);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Stream Selection Asynchronous should return failure." );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_SelectStreams_SelectChildWithOutParent_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODMultiSparseParentedWithAudioAndVideo");
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        proposedSelection = AddTextStream(false);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Stream Selection Asynchronous should return failure." );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_SelectStreams_SelectMultipleChildWithSameParent_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::SparseStreams_SelectStreams_SelectMultipleChildWithSameParent_DuringManifestReady_Method);

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODMultiSparseParentedWithAudio");
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void SparseStreams_SelectStreams_SelectMultipleChildWithSameParent_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        proposedSelection = SelectSparseStreamsByParentType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_SelectStreams_SelectMultipleChildWithSameParent_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODMultiSparseParentedWithAudio");
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        proposedSelection = SelectSparseStreamsByParentType(MediaStreamTypeAudio);

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_SelectStreams_DeselectChildWithOutDeselectingParent_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::SparseStreams_SelectStreams_DeselectChildWithOutDeselectingParent_DuringManifestReady_Method);

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODMultiSparseParentedWithAudio");
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void SparseStreams_SelectStreams_DeselectChildWithOutDeselectingParent_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        //select all text streams.
        proposedSelection = AddTextStream(true);
        _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );

        //Deselect child sparse streams.
        proposedSelection = SelectFirstStreamOfAudioAndVideo();

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_SelectStreams_DeselectChildWithOutDeselectingParent_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODMultiSparseParentedWithAudioAndVideo");
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        //select all text streams.
        proposedSelection = AddTextStream(true);
        _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );

        //Delesect child sparse streams.
        proposedSelection = SelectFirstStreamOfAudioAndVideo();

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(hr2) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_SelectStreams_DeSelectParentWithOutChild_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod(this, &StreamSelectionTests::SparseStreams_SelectStreams_DeSelectParentWithOutChild_DuringManifestReady_Method);

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODMultiSparseParentedWithAudioAndVideo");
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void SparseStreams_SelectStreams_DeSelectParentWithOutChild_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        VectorOfStreams availableStreams;
        VectorOfStreams selectedStreams;
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        //ADD the child streams for current audio
        proposedSelection = SelectSparseStreamsByParentType(MediaStreamTypeAudio);
        _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );

        //change the audio
        proposedSelection.clear();

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        for(uint32_t i = 0; i < selectedStreams.size(); i++)
        {
            if( selectedStreams[i]->Type() != MediaStreamTypeAudio ) 
            {
                proposedSelection.push_back(selectedStreams[i]);
            }
        }

        for(uint32_t i = 0; i < availableStreams.size(); i++)
        {
            if( availableStreams[i]->Type() == MediaStreamTypeAudio ) 
            {
                VectorOfStreams::const_iterator it = std::find( selectedStreams.begin(), selectedStreams.end(), availableStreams[i] );
                if ( it == selectedStreams.end() )
                {
                    proposedSelection.push_back(availableStreams[i]);
                    break;
                }
            }
        }

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Stream Selection Asynchronous should return failure." );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_SelectStreams_DeSelectParentWithOutChild_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;

        VectorOfStreams availableStreams;
        VectorOfStreams selectedStreams;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODMultiSparseParentedWithAudioAndVideo");
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( CheckDefaultStreamSelection() );

        //ADD the child streams for current audio
        proposedSelection = SelectSparseStreamsByParentType(MediaStreamTypeAudio);
        _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );

        //change the audio
        proposedSelection.clear();

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        for(uint32_t i = 0; i < selectedStreams.size(); i++)
        {
            if( selectedStreams[i]->Type() != MediaStreamTypeAudio ) 
            {
                proposedSelection.push_back(selectedStreams[i]);
            }
        }

        for(uint32_t i = 0; i < availableStreams.size(); i++)
        {
            if( availableStreams[i]->Type() == MediaStreamTypeAudio ) 
            {
                VectorOfStreams::const_iterator it = std::find( selectedStreams.begin(), selectedStreams.end(), availableStreams[i] );
                if ( it == selectedStreams.end() )
                {
                    proposedSelection.push_back(availableStreams[i]);
                    break;
                }
            }
        }

        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        PKTEST_ASSERT_MSG_EXIT(FAILED(hr2), "Stream Selection Asynchronous should return failure." );

    exit:
        return;
    }


    //*********************************************************************************************************************************
    
private:
    void CheckAvailableStreamCountByType(int32_t argAudioStreamCount,int32_t argVideoStreamCount,int32_t argOtherStreamCount)
    {
        VectorOfStreams availableStreams;
        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );

        PKTEST_FUNC_EXIT( SSPKHelpers::CheckStreamCountByType(availableStreams, argAudioStreamCount, argVideoStreamCount, argOtherStreamCount) );

    exit:
        return;
    }

    void CheckSelectedStreamCountByType(int32_t argAudioStreamCount,int32_t argVideoStreamCount,int32_t argOtherStreamCount)
    {
        VectorOfStreams selectedStreams;
        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        PKTEST_FUNC_EXIT( SSPKHelpers::CheckStreamCountByType(selectedStreams, argAudioStreamCount, argVideoStreamCount, argOtherStreamCount) );

    exit:
        return;
    }

    void CheckSelectedStreamByName(MediaStreamType type, string name)
    {
        VectorOfStreams selectedStreams;
        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        PKTEST_FUNC_EXIT( SSPKHelpers::CheckStreamByName(selectedStreams, type, name) );

    exit:
        return;
    }

    VectorOfStreams SelectionStreamByIndex(int32_t indices[], int32_t size)
    {
        VectorOfStreams availableStreams;
        VectorOfStreams proposedSelection;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );

        PKTEST_FUNC_EXIT( proposedSelection =  SSPKHelpers::SelectionStreamByIndex(availableStreams, indices, size) );

    exit:
        return proposedSelection;
    }

    VectorOfStreams ToggledStreamByMediaType(MediaStreamType type)
    {
        VectorOfStreams availableStreams;
        VectorOfStreams selectedStreams;
        VectorOfStreams proposedSelection;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        PKTEST_FUNC_EXIT( proposedSelection =  SSPKHelpers::ToggledStreamByMediaType(availableStreams, selectedStreams, type) );

    exit:
        return proposedSelection;
    }

    VectorOfStreams SelectAllStreamsByMediaType(MediaStreamType type)
    {
        VectorOfStreams availableStreams;
        VectorOfStreams proposedSelection;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );

        PKTEST_FUNC_EXIT( proposedSelection =  SSPKHelpers::SelectAllStreamsByMediaType(availableStreams, type) );

    exit:
        return proposedSelection;
    }

    VectorOfStreams SelectFirstStreamOfAudioAndVideo()
    {
        VectorOfStreams availableStreams;
        VectorOfStreams proposedSelection;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );

        PKTEST_FUNC_EXIT( proposedSelection =  SSPKHelpers::SelectFirstStreamOfAudioAndVideo(availableStreams ));

    exit:
        return proposedSelection;
    }

    void CheckDefaultStreamSelection()
    {
        VectorOfStreams availableStreams;
        VectorOfStreams selectedStreams;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        PKTEST_FUNC_EXIT( SSPKHelpers::CheckDefaultStreamSelection( availableStreams, selectedStreams ) );

    exit:
        return;
    }

    void CompareGivenVectorToSelectedStreams( VectorOfStreams proposedSelection )
    {
        VectorOfStreams selectedStreams;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams( &selectedStreams ) );
        PKTEST_FUNC_EXIT( SSPKHelpers::CompareTwoVectors( proposedSelection, selectedStreams ) );

    exit:
        return;
    }

    VectorOfStreams AddTextStream(bool isChild)
    {
        VectorOfStreams availableStreams;
        VectorOfStreams selectedStreams;
        VectorOfStreams proposedSelection;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        PKTEST_FUNC_EXIT(proposedSelection = SSPKHelpers::AddTextStream(availableStreams, selectedStreams, isChild));

    exit:
        return proposedSelection;
    }

    VectorOfStreams SelectSparseStreamsByParentType(MediaStreamType type)
    {
        VectorOfStreams availableStreams;
        VectorOfStreams selectedStreams;
        VectorOfStreams proposedSelection;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        for(uint32_t j=0; j < selectedStreams.size(); j++)
        {
            if( selectedStreams[j]->Type() != MediaStreamTypeText)
            {
                proposedSelection.push_back(selectedStreams[j]);
            }
        }
        for( uint32_t i = 0; i<availableStreams.size(); i++ )
        {
            if( availableStreams[i]->Type() == MediaStreamTypeText )
            {
                IManifestStream* parentStream ;
                availableStreams[i]->GetParentStream(&parentStream);
                if( NULL != parentStream)
                {
                    if(parentStream->Type() == type)
                    {
                        VectorOfStreams::const_iterator it = std::find( selectedStreams.begin(), selectedStreams.end(), parentStream);
                        if ( it != selectedStreams.end() )
                        {
                            proposedSelection.push_back(availableStreams[i]);
                        }
                    }
                }
            }
        }

    exit:
        return proposedSelection;
    }

};
