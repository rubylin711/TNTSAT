///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PKTestSuite.h"
#include "STWrapper.h"
#include "ManifestReadyCallbackImpl.h"
#include "SSPKHelpers.h"

using namespace SSPK;
using namespace SSPKTest;

static const float TRICK_PLAY_SPEED = 96; //For moving past DVR Window's edges quickly
static const float TRICK_PLAY_SPEED_LOW = 2; //For ensuring that current time snapped to correct position

static const uint32_t LIVE_BACKOFF_VALUES[] = {10,15,65};


PKTEST_GROUP( DVRWindowTests )
{
    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;

    //Utility
    bool CheckForMediaEnded()
    {
        return _smoothObject->GetStatusUpdatesList().MediaEnded > 0;
    }

    int32_t GetMediaEndedCount()
    {
        return _smoothObject->GetStatusUpdatesList().MediaEnded;
    }

    int32_t GetDetuneCount()
    {
        return _smoothObject->GetStatusUpdatesList().Detuned;
    }

    int32_t GetTuningCount()
    {
        return _smoothObject->GetStatusUpdatesList().Tuning;
    }
    
    bool CheckForAtWindowEdge()
    {
        vector<SmoothTransportStatus> statusList = _smoothObject->GetStatusCallbackVector();

        for(int32_t i = statusList.size() - 1; i >= 0; i--)
        {
            if(statusList[i]._update == SmoothTransportStatus_AtWindowEdge)
            {
                return true;
            }
        }
        
        return false;
    }

    bool SelectOtherStreams(MediaStreamType streamType)
    {
        VectorOfStreams availableStreams;
        VectorOfStreams selectedStreams;
        VectorOfStreams proposedSelection;
        HRESULT selectResult;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        PKTEST_FUNC_EXIT( proposedSelection =  SSPKHelpers::ToggledStreamByMediaType(availableStreams, selectedStreams, streamType) );
        selectResult = _smoothObject->GetManifest()->SelectStreamsAsync(_smoothObject, proposedSelection);
        if (pkSUCCEEDED(selectResult) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure value 0x%08x", selectResult) ;
        }
        

        return true;
    exit:
        return false;

    }

    //Setup
    DVRWindowTests()
    {
        CXMLElement xTestData = PKTest_GetDataTable( "IntegrationTestSources" );
        if( !xTestData.IsNull() )
        {
             _xManifests = xTestData.Elements(L"source");
        }
    }

    bool TestSetup()
    {
        PKTEST_ASSERT_MSG_EXIT(_xManifests.Length() > 0, "Couldn't find any elements with name 'source'");

        _smoothObject = NEW_NO_THROW STWrapper();
        PKTEST_ASSERT_MSG_EXIT( NULL != _smoothObject, "Unable to start smooth streaming object");

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
        return pkSUCCEEDED(pkResult);
    }
    
    bool LetDVRWindowPassPausedPosition()
    {
        int64_t seekToNearDVRStart = 0, prePauseTime = 0, postPauseTime = 0;

        seekToNearDVRStart = _smoothObject->GetCurrentStartTime() + DEFAULT_SKIP_10MHZ;
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(seekToNearDVRStart) );
        
        prePauseTime = _smoothObject->GetCurrentPlayBackTime();
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(2 * (int32_t) TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks(DEFAULT_SKIP_10MHZ) ).Ticks() ));
        postPauseTime = _smoothObject->GetCurrentPlayBackTime();

        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps( postPauseTime, prePauseTime, TIMESCALE_10MHZ), "currentTime(%lld) is not same as the previous time(%lld).  They differ by %.3f seconds", postPauseTime, prePauseTime, TimeSpan_hns::FromTicks(postPauseTime - prePauseTime).ToSeconds());
        PKTEST_ASSERT_MSG_EXIT(0 == GetMediaEndedCount(), "we should not get any mediaEnded, but we did ");
        PKTEST_ASSERT_MSG_EXIT(CheckForAtWindowEdge(), "we should get WindowEdge status, but we did not ");
        PKTEST_ASSERT_MSG_EXIT(postPauseTime < _smoothObject->GetCurrentStartTime() , "DVR Window did not pass paused position, paused position is %.3f seconds past DVR window", TimeSpan_hns::FromTicks(postPauseTime - _smoothObject->GetCurrentStartTime()).ToSeconds());
        return true;
    exit:
        LogTestComment("Unable to seek to near left edge and pause");
        return false;
    }

    
    //Tests
    //Skip past edge
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_SkipPastRightEdgeLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(TOTAL_SECONDS_IN_A_DAY));
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        //Should snap to right edge
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsAtDVRWindowEnd(), "Skip Past Right Edge on live should snap to right edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Skip Past Right Edge on live should autoplay" );
    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_SkipPastRightEdgeOD,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(TOTAL_SECONDS_IN_A_DAY));
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, IMMEDIATE_STATECHANGE_TIMEOUT), "skip past right on VOD should get media Ended Immediately" );

        //Should snap to right edge
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "Skip Past Right Edge should snap to right Edge" );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Skip Past Right Edge should get mediaEnded" );
    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_SkipPastRightEdgeODNonZeroStartTime,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Data:ODStartTimeNot0", "ODStartTimeNot0")
        PKTEST_PROPERTY("Bug", "31907")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(TOTAL_SECONDS_IN_A_DAY));
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaEnded(IMMEDIATE_STATECHANGE_TIMEOUT), "skip past right on VOD should get media Ended Immediately" );

        //Should snap to right edge
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "Skip Past Right Edge should snap to right Edge" );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Skip Past Right Edge should get mediaEnded" );
    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_SkipPastLeftEdge,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(-TOTAL_SECONDS_IN_A_DAY));
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        //Should snap to right edge
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsAtDVRWindowStart(), "Skip Past Left Edge  should snap to left edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Skip Past Left Edge should autoplay" );

    exit:
        return;
    }

    //Seek past edge
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_SeekPastLeftEdge,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedTimeStamp = 0;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        
        if(_smoothObject->GetCurrentStartTime() > DEFAULT_SKIP_10MHZ)
        {
            expectedTimeStamp = _smoothObject->GetCurrentStartTime() - DEFAULT_SKIP_10MHZ;
        }
        PKTEST_HRESULT_EXIT( _smoothObject->Seek(expectedTimeStamp) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        //Should snap to right edge
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsAtDVRWindowStart(), "Seek Past Left Edge  should snap to left edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Seek Past Left Edge should autoplay" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_SeekPastRightEdgeLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        int64_t expectedTimeStamp = 0;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        
        expectedTimeStamp = _smoothObject->GetCurrentEndTime() + DEFAULT_SKIP_10MHZ;
        PKTEST_HRESULT_EXIT( _smoothObject->Seek(expectedTimeStamp) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        //Should snap to right edge
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsAtDVRWindowEnd(), "Seek Past Right Edge  should snap to Right edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Seek Past Right Edge on live should autoplay" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_SeekPastRightEdgeOD,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedTimeStamp = 0;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        
        expectedTimeStamp = _smoothObject->GetCurrentEndTime() + DEFAULT_SKIP_10MHZ;
        PKTEST_HRESULT_EXIT( _smoothObject->Seek(expectedTimeStamp) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, IMMEDIATE_STATECHANGE_TIMEOUT), "skip past right on VOD should get media Ended Immediately" );

        //Should snap to right edge
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "Seek Past Right Edge should snap to right Edge" );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Seek Past Right Edge should get mediaEnded" );

    exit:
        return;
    }


    //Trick play past edge
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_FastForwardPastRightEdge,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
       
        PKTEST_FUNC_EXIT(_smoothObject->SeekMilliseconds( _smoothObject->GetCurrentEndTime() - ( VALIDATION_DELAY + LIVE_BACKOFF_TIME)));
        PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(TRICK_PLAY_SPEED, VALIDATION_DELAY) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaEnded( STATECHANGE_TIMEOUT ), "TimeOut happened while waiting for the MediaEnded");
        PKTEST_ASSERT_MSG_EXIT( !CheckForAtWindowEdge(), "Expected smoothObject not to have gotten AtWindowEdge notification but it did.");

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_RewindPastLeftEdge,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
       
        PKTEST_FUNC_EXIT( _smoothObject->SeekMilliseconds(_smoothObject->GetCurrentStartTime() + VALIDATION_DELAY + LIVE_LEFT_BACKOFF_TIME));
        PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(TRICK_PLAY_SPEED, VALIDATION_DELAY) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaEnded( STATECHANGE_TIMEOUT ), "TimeOut happened while waiting for the MediaEnded");
        PKTEST_ASSERT_MSG_EXIT( !CheckForAtWindowEdge(), "Expected smoothObject not to have gotten AtWindowEdge notification but it did.");

        //Should not get 'out of range' for OD content
        if(!_smoothObject->IsLive())
        {
            PKTEST_ASSERT_MSG_EXIT( 0 == _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_OutsideWindowEdge), "There should be no outside window edge status updates for OD content");
        }

    exit:
        return;
    }

    //After DVRWindow has passed current (paused) position
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_PlayAfterWindowPasses,
        PKTEST_PROPERTY("Priority", "0")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );

        PKTEST_FUNC_EXIT( LetDVRWindowPassPausedPosition() );

        PKTEST_HRESULT_EXIT(_smoothObject->Play());
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        PKTEST_ASSERT_EXIT(_smoothObject->IsAtDVRWindowStart());
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "should be in playing state" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_CloseAfterWindowPasses,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );

        PKTEST_FUNC_EXIT( LetDVRWindowPassPausedPosition() );

        PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(VALIDATION_DELAY));

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_FastForwardAfterWindowPasses,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );

        PKTEST_FUNC_EXIT( LetDVRWindowPassPausedPosition() );

        PKTEST_HRESULT_EXIT( _smoothObject->FastForward(TRICK_PLAY_SPEED_LOW) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsAtDVRWindowStart(), "fastforward from beyond left edge should snap to left edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsInFastForward(), "We should be in Fastforward" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_RewindAfterWindowPasses,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );

        PKTEST_FUNC_EXIT( LetDVRWindowPassPausedPosition() );

        PKTEST_HRESULT_EXIT( _smoothObject->Rewind(TRICK_PLAY_SPEED_LOW) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, IMMEDIATE_STATECHANGE_TIMEOUT), "Trying to rewind at left edge should get MediaEnded immediately" );

        //Should snap to left edge
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowStart(), "Rewind from beyond left edge should snap to left edge" );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Rewind from LeftEdge should cause MediaEnded" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_SwitchAudioAfterWindowPasses,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveMultiAudio", "VC1LiveMultiAudio")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );

        PKTEST_FUNC_EXIT( LetDVRWindowPassPausedPosition() );
        
        PKTEST_ASSERT_MSG_EXIT( SelectOtherStreams(MediaStreamTypeAudio), "Expected audio track to change when paused outside of DVR window but it did not");

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_SwitchTextAfterWindowPasses,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveMultiAudio", "VC1LiveMultiAudio")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );

        PKTEST_FUNC_EXIT( LetDVRWindowPassPausedPosition() );
        
        PKTEST_ASSERT_MSG_EXIT( SelectOtherStreams(MediaStreamTypeText), "Expected text track to change when paused outside of DVR window but it did not");

    exit:
        return;
    }
    
    //AtWndwEdge
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_PlayAtWindowEdge,
        PKTEST_PROPERTY("Priority", "0")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        int64_t expectedTimeStamp = 0;
        int32_t detuneCount = 0, tuningCount = 0;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );

        expectedTimeStamp = _smoothObject->GetCurrentStartTime() + DEFAULT_SKIP_10MHZ;
        PKTEST_HRESULT_EXIT( _smoothObject->Seek(expectedTimeStamp) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");
        
        PKTEST_HRESULT_EXIT(_smoothObject->Pause(MIN_TIME64));
        PKTEST_FUNC_EXIT( _smoothObject->WaitForAtWndwEdge(3 * STATECHANGE_TIMEOUT) );

        //Make sure there are no extraneous detune/tune when playing at window edge notification
        detuneCount = GetDetuneCount();
        tuningCount = GetTuningCount();
        PKTEST_FUNC_EXIT(_smoothObject->PlayAndValidate(VALIDATION_DELAY));
        PKTEST_ASSERT_MSG_EXIT( detuneCount == GetDetuneCount(), " Expected no detuning when playing at window edge but it did occur: expected count %d, actual count %d", detuneCount, GetDetuneCount());
        PKTEST_ASSERT_MSG_EXIT( tuningCount == GetTuningCount(), " Expected no Tuning when playing at window edge but it did occur: expected count %d, actual count %d", tuningCount, GetTuningCount());
        

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_NonIntegerSecondsChunkDuration,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Data:LiveChunkSizeNonIntegerSeconds", "LiveChunkSizeNonIntegerSeconds")
        )
    {
        vector<SmoothTransportStatus> statusList;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(PLAYBACK_STARTUP_TIMEOUT);

        statusList = _smoothObject->GetStatusCallbackVector(SmoothTransportStatus_StartEndTime);
        PKTEST_ASSERT_MSG_EXIT( statusList.size() > 1, "There are only %d StartEndTime status callbacks, not enough to validate.", statusList.size() );

        //Make sure that the start time changes for every StartEnd update
        for(uint32_t i = 1; i < statusList.size() ; ++i)
        {
            PKTEST_ASSERT_MSG_EXIT( statusList[i - 1]._startTime < statusList[i]._startTime, "Timestamp for #%d status update ( %.3f seconds ) is not greater than that of #%d status update ( %.3f seconds )", 
                                        i, statusList[i]._startTime.ToSeconds(), i - 1, statusList[i - 1]._startTime.ToSeconds());
        }
        
    exit:
        return;
    }

    PKTEST_METHOD_EX( DVRWindowTests_PlayPosition_TakenOverBy_LeftEdge_OutOfWindow,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalOutOfWndwEvents = 0;
        string profileString = "";
        int32_t bandWidthCap = 100; //Kbps

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"VC1LiveDefault") );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        profileString = SSPKHelpers::GetCR_SetMaxNetworkCap(SSPKHelpers::GetLocalUrl(urlString), bandWidthCap);

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), profileString);

        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_HRESULT_EXIT( _smoothObject->Skip(-TOTAL_SECONDS_IN_A_DAY) );
        _smoothObject->Delay(4 * VALIDATION_DELAY);
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( smoothStatusVector[i]._update == SmoothTransportStatus_OutsideWindowEdge )
            {
                 totalOutOfWndwEvents++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalOutOfWndwEvents == 1 , " We are expecting 1 OutOfWndw Callback, but there are %d events", totalOutOfWndwEvents );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 0, "We are not expecting any errors, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_FUNC_EXIT(_smoothObject->WaitForMediaEnded(DEFAULT_BUFFER_SIZE + VALIDATION_DELAY));

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        PKTEST_FUNC_EXIT(_smoothObject->PlayAndValidate(VALIDATION_DELAY));
        PKTEST_ASSERT_EXIT(_smoothObject->IsAtDVRWindowStart());

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(StatusTests_ActualDVRWindowLengthMatchesManifestValue,
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        PKTEST_PROPERTY("Data:LiveProtectedPiff1.3DefaultKR", "LiveProtectedPiff1.3DefaultKR")
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        string urlString;
        int64_t manifestDVRWindowLength = 0, actualDVRWindowLength = 0, diff = 0;

        PKTEST_FUNC_EXIT( urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true, true) );

        actualDVRWindowLength = _smoothObject->GetCurrentEndTime() - _smoothObject->GetCurrentStartTime();
        manifestDVRWindowLength = _smoothObject->GetManifest()->Duration();
        diff = actualDVRWindowLength > manifestDVRWindowLength ? actualDVRWindowLength - manifestDVRWindowLength: manifestDVRWindowLength - actualDVRWindowLength;
        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(actualDVRWindowLength, manifestDVRWindowLength, TIMESTAMP_COMPARISON_THRESHOLD), " The actual DVR window length (%lld) does not match the length specified in the manifest (%lld).  The difference is %lld", actualDVRWindowLength, manifestDVRWindowLength, diff);

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_OD_StartPosition_DontChange,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlString;
        int64_t startTime = 0;
        vector<SmoothTransportStatus> statusVector; 


        PKTEST_FUNC_EXIT( urlString = SSPKHelpers::GetUrlFromList( _xManifests, L"H264ODDefault") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true, true) );
        startTime = _smoothObject->GetCurrentStartTime();

        _smoothObject->Delay(VALIDATION_DELAY);

        statusVector = _smoothObject->GetStatusCallbackVector();

        for (uint32_t i = 0; i < statusVector.size(); i++)
        {
            PKTEST_ASSERT_MSG_EXIT(CompareTimeStamps(startTime, TimeSpan_hns::ConvertFrom(statusVector[i]._startTime).Ticks(), TIMESTAMP_COMPARISON_THRESHOLD), "start Time should not be moving for OD content" );
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_OD_ContentBiggerThan6hours,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlString;
        vector<SmoothTransportStatus> smoothStatusVector; 
        vector<SmoothTransportError> smoothErrorVector; 


        PKTEST_FUNC_EXIT( urlString = SSPKHelpers::GetUrlFromList( _xManifests, L"H264ODLongerThan6Hours") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideo(urlString, true, false) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT ), "Timeout occurred waiting for media to fail");

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();

        PKTEST_ASSERT_MSG_EXIT(smoothStatusVector.size() == 0, "There should not be any status updates when Manifestparsing error occurred." );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ManifestParseFailed, "ManifestParseFailed expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_OD_ChangeChunkListSize_ContentBiggerThan6hours,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlString;
        vector<SmoothTransportStatus> statusVector; 
        int32_t newChunkListCount = 16 * 60 * SECONDS_PER_MINUTE / 2; //Assuming the chunksize is 2 seconds.

        string chunkListSizeString = toString(newChunkListCount);
        const char* g_szChunkListMaxSize = chunkListSizeString.c_str();
        PKTEST_FUNC_EXIT( _smoothObject->SendExtendedCommand(EXC_CHUNK_LIST_MAX_SIZE, 1, &g_szChunkListMaxSize) );

        PKTEST_FUNC_EXIT( urlString = SSPKHelpers::GetUrlFromList( _xManifests, L"H264ODLongerThan6Hours") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true, true) );
        
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_ASSERT_EXIT(CompareTimeStamps( _smoothObject->GetManifest()->Duration(), _smoothObject->GetCurrentEndTime() - _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold()));


    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_LeftEdgeStartPlaybackFromMediaEndedCurrentTimeMovesForward,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        int64_t currentPlaybackTime = 0, lastPlaybackTime = 0;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_ASSERT_EXIT( LetDVRWindowPassPausedPosition() );
        lastPlaybackTime = _smoothObject->GetCurrentPlayBackTime();

        //Make sure that when playing, the time does not jump backwards
        PKTEST_HRESULT_EXIT(_smoothObject->Play());
        PKTEST_ASSERT_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT));

        //Make sure that when playing, the time does not jump backwards
        currentPlaybackTime = _smoothObject->GetCurrentPlayBackTime();
        PKTEST_ASSERT_EXIT(lastPlaybackTime < currentPlaybackTime);
        _smoothObject->Delay( HALF_SECOND_IN_MILLISECONDS );

        //Start checking the playback time
        for(int32_t i = 0; i < 10; i++)
        {
            lastPlaybackTime = currentPlaybackTime;
            currentPlaybackTime = _smoothObject->GetCurrentPlayBackTime();
            
            PKTEST_ASSERT_MSG_EXIT( currentPlaybackTime > lastPlaybackTime, "Current playback time on iteration %d is not more than the previous iteration. current = %lld, previous = %lld, difference = %lld", i, currentPlaybackTime, lastPlaybackTime, lastPlaybackTime - currentPlaybackTime);
            LogTestComment("Current = %lld    Last = %lld    Diff = %lld", currentPlaybackTime, lastPlaybackTime, currentPlaybackTime - lastPlaybackTime);

            _smoothObject->Delay( HALF_SECOND_IN_MILLISECONDS );
        }

    exit:
        return;
    }

    
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_SetLivePlaybackOffset,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault")
        PKTEST_PROPERTY("Data:LiveProtectedPiff1.3DefaultKR", "LiveProtectedPiff1.3DefaultKR")
        )
    {
        int64_t currentPlaybackTime = 0, liveTime = 0;
        string valueAsString;
        const char *extendedCommandArgBuffer = NULL;
        string urlString;
        uint32_t liveBackoffInSeconds = (uint32_t) TimestampToSeconds( DEFAULT_LIVE_BACKOFF);

        PKTEST_FUNC_EXIT( urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );

        for(int32_t i = 0; i < sizeof(LIVE_BACKOFF_VALUES) / sizeof(uint32_t); ++i)
        {
            //Set live backoff
            valueAsString = uint32ToString(LIVE_BACKOFF_VALUES[i]);
            extendedCommandArgBuffer = valueAsString.c_str();
            PKTEST_FUNC_EXIT( _smoothObject->SendExtendedCommand(EXC_LIVE_PLAYBACK_OFFSET, 1, &extendedCommandArgBuffer) );

            //Open and play
            PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        
            //Make sure that playback offset is correct
            currentPlaybackTime = _smoothObject->GetCurrentPlayBackTime();
            liveTime = _smoothObject->GetCurrentEndTime();
            PKTEST_ASSERT_MSG_EXIT(CompareTimeStamps(currentPlaybackTime, liveTime - TimeSpan_hns::ConvertFrom(TimeSpan_s::FromTicks(LIVE_BACKOFF_VALUES[i] + liveBackoffInSeconds) ).Ticks(), _smoothObject->GetNonAccurateSeekThreshold()),
                                        "Expected playback time (%lld) to be %d seconds from the live edge (%lld), but it is %.3f seconds behind the live edge instead.  The live backoff is %d seconds.",
                                        currentPlaybackTime, LIVE_BACKOFF_VALUES[i] + liveBackoffInSeconds, liveTime, TimestampToSeconds(liveTime - currentPlaybackTime), liveBackoffInSeconds );

            //Shut down
            PKTEST_FUNC_EXIT( _smoothObject->CloseAndValidate(VALIDATION_DELAY) );
        }

    exit:
        //Set live backoff back to default
        valueAsString = uint32ToString(liveBackoffInSeconds);
        extendedCommandArgBuffer = valueAsString.c_str();
        _smoothObject->SendExtendedCommand(EXC_LIVE_BACKOFF, 1, &extendedCommandArgBuffer);
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_SetLiveBackoff,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault")
        PKTEST_PROPERTY("Data:LiveProtectedPiff1.3DefaultKR", "LiveProtectedPiff1.3DefaultKR")
        )
    {
        int64_t currentPlaybackTime = 0, liveTime = 0;
        string valueAsString;
        const char *extendedCommandArgBuffer = NULL;
        string urlString;
        uint32_t livePlaybackOffsetInSeconds = (uint32_t) TimestampToSeconds( DEFAULT_LIVE_PLAYBACK_OFFSET);

        PKTEST_FUNC_EXIT(urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );

        for(int32_t i = 0; i < sizeof(LIVE_BACKOFF_VALUES) / sizeof(uint32_t); ++i)
        {
            //Set live backoff
            valueAsString = uint32ToString(LIVE_BACKOFF_VALUES[i]);
            extendedCommandArgBuffer = valueAsString.c_str();
            PKTEST_FUNC_EXIT( _smoothObject->SendExtendedCommand(EXC_LIVE_BACKOFF, 1, &extendedCommandArgBuffer) );

            //Open and play
            PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        
            //Make sure that backoff is correct
            currentPlaybackTime = _smoothObject->GetCurrentPlayBackTime();
            liveTime = _smoothObject->GetCurrentEndTime();
            PKTEST_ASSERT_MSG_EXIT(CompareTimeStamps(currentPlaybackTime, liveTime - TimeSpan_hns::ConvertFrom(TimeSpan_s::FromTicks(LIVE_BACKOFF_VALUES[i] + livePlaybackOffsetInSeconds) ).Ticks(), _smoothObject->GetNonAccurateSeekThreshold()),
                                        "Expected playback time (%lld) to be %d seconds from the live edge (%lld), but it is %.3f seconds behind the live edge instead.  The live playback offset is %d seconds.",
                                        currentPlaybackTime, LIVE_BACKOFF_VALUES[i] + livePlaybackOffsetInSeconds, liveTime, TimestampToSeconds(liveTime - currentPlaybackTime), livePlaybackOffsetInSeconds);

            //Shut down
            PKTEST_FUNC_EXIT( _smoothObject->CloseAndValidate(VALIDATION_DELAY) );
        }

    exit:
        //Set live playback offset back to default
        valueAsString = uint32ToString(livePlaybackOffsetInSeconds);
        extendedCommandArgBuffer = valueAsString.c_str();
        _smoothObject->SendExtendedCommand(EXC_LIVE_PLAYBACK_OFFSET, 1, &extendedCommandArgBuffer);
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(DVRWindowTests_SetLiveMinTimeBufferBackoff,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault")
        PKTEST_PROPERTY("Data:LiveProtectedPiff1.3DefaultKR", "LiveProtectedPiff1.3DefaultKR")
        )
    {
        int64_t currentPlaybackTime = 0, minTime = 0;
        string valueAsString;
        const char *extendedCommandArgBuffer = NULL;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );

        for(int32_t i = 0; i < sizeof(LIVE_BACKOFF_VALUES) / sizeof(uint32_t); ++i)
        {
            //Set live backoff
            valueAsString = uint32ToString(LIVE_BACKOFF_VALUES[i]);
            extendedCommandArgBuffer = valueAsString.c_str();
            PKTEST_FUNC_EXIT( _smoothObject->SendExtendedCommand(EXC_LIVE_BEGIN_EDGE_BACKOFF, 1, &extendedCommandArgBuffer) );

            //Open, play and move to beginning
            PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
            minTime = _smoothObject->GetCurrentStartTime();
            PKTEST_HRESULT_EXIT( _smoothObject->Seek(minTime) );
            PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT) );

            //Make sure that backoff is correct
            currentPlaybackTime = _smoothObject->GetCurrentPlayBackTime();
            minTime = _smoothObject->GetCurrentStartTime();
            PKTEST_ASSERT_MSG_EXIT(CompareTimeStamps(currentPlaybackTime, TimeSpan_hns::ConvertFrom(TimeSpan_s::FromTicks(LIVE_BACKOFF_VALUES[i]) ).Ticks() + minTime, _smoothObject->GetNonAccurateSeekThreshold()),
                                        "Expected playback time (%lld) to be %d seconds from the starting edge (%lld), but it is %.3f seconds ahead of the live edge instead.",
                                        currentPlaybackTime, LIVE_BACKOFF_VALUES[i], minTime, TimestampToSeconds(currentPlaybackTime - minTime));

            //Shut down
            PKTEST_FUNC_EXIT( _smoothObject->CloseAndValidate(VALIDATION_DELAY) );
        }

    exit:
        return;
    }
};
