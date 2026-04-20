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
static const float TRICK_PLAY_SPEED_MEDIUM = 4; //For ensuring that current time snapped to correct position

PKTEST_GROUP( MediaEndedTests )
{
    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;

        //Setup
    MediaEndedTests()
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

    //After Media ended
    ///Pause
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedPauseCurrentTime,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedPausedTime, actualPausedTime;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        expectedPausedTime = _smoothObject->GetCurrentPlayBackTime();
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );
        actualPausedTime = _smoothObject->GetCurrentPlayBackTime();

        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(expectedPausedTime, actualPausedTime, MIN_TIME64), "Current paused time (%lld) does not match expected Pause time (%lld) with Threshold : %d", actualPausedTime, expectedPausedTime, MIN_TIME64 );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedPause,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedPausePlay,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );
        PKTEST_HRESULT_EXIT( _smoothObject->Play() );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout happened before the rendering started." );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowStart(), "PlayAt past left Edge should snap to left Edge" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedPauseCurrentTime,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedPausedTime, actualPausedTime;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        expectedPausedTime = _smoothObject->GetCurrentPlayBackTime();
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );
        actualPausedTime = _smoothObject->GetCurrentPlayBackTime();

        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(expectedPausedTime, actualPausedTime, MIN_TIME64), "Current paused time (%lld) does not match expected Pause time (%lld) with Threshold : %d", actualPausedTime, expectedPausedTime, MIN_TIME64 );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedPause,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );

    exit:
        return;
    }

    ///Play
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedPlay,
        PKTEST_PROPERTY("Priority", "0")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        PKTEST_HRESULT_EXIT(_smoothObject->Play());
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        //Should snap to left edge
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowStart(), "Play past left Edge should snap to left Edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "should be in playing state" );

        //Make sure there is no extra mediaEnded update; there should be only the one for reaching the beginning of the dvr window
        PKTEST_ASSERT_MSG_EXIT( 1 == _smoothObject->GetStatusUpdatesList().MediaEnded, " Expected one MediaEnded for every time the beginning of the DVR window was reached: expected count 1, actual count %d", _smoothObject->GetStatusUpdatesList().MediaEnded);
    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedPlayLive,
        PKTEST_PROPERTY("Priority", "0")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        PKTEST_HRESULT_EXIT(_smoothObject->Play());
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        //Should snap to right edge
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "Play after media ended on right edge should start at right edge");
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "should be in playing state" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedLongWaitPlayLive,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        int64_t waitTime = 0;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        waitTime = TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks(_smoothObject->GetCurrentEndTime() - _smoothObject->GetCurrentStartTime()) ).Ticks();
        PKTEST_ASSERT_EXIT(waitTime == (int32_t)waitTime);
        _smoothObject->Delay((int32_t)waitTime);

        PKTEST_HRESULT_EXIT(_smoothObject->Play());
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowStart(), "Play past left Edge should snap to left Edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "should be in playing state" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedPlayOD,
        PKTEST_PROPERTY("Priority", "0")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        PKTEST_HRESULT_EXIT(_smoothObject->Play());
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, IMMEDIATE_STATECHANGE_TIMEOUT), "Play past right on VOD should get media Ended Immediately" );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "Play past Right Edge should snap to right Edge" );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Play past Right Edge should get mediaEnded" );

    exit:
        return;
    }

    ///PlayAt
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedPlayAtInDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedPlayTime = 0;
        
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        expectedPlayTime = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(expectedPlayTime, 1.0, VALIDATION_DELAY) );
        
    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedPlayAtInDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedPlayTime = 0;
        
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        expectedPlayTime = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(expectedPlayTime, 1.0,  VALIDATION_DELAY) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedPlayAtBeforeDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t beforeWindow = 0;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        if(_smoothObject->GetCurrentStartTime() > DEFAULT_SKIP_10MHZ)
        {
            beforeWindow = _smoothObject->GetCurrentStartTime() - DEFAULT_SKIP_10MHZ;
        }

        PKTEST_FUNC_EXIT(_smoothObject->PlayAt(beforeWindow));
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");
        
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowStart(), "PlayAt past left Edge should snap to left Edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "should be in playing state" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedPlayAtBeforeDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t beforeWindow = 0;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        if(_smoothObject->GetCurrentStartTime() > DEFAULT_SKIP_10MHZ)
        {
            beforeWindow = _smoothObject->GetCurrentStartTime() - DEFAULT_SKIP_10MHZ;
        }

        PKTEST_FUNC_EXIT(_smoothObject->PlayAt(beforeWindow));
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");
        
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowStart(), "PlayAt past left Edge should snap to left Edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "should be in playing state" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedPlayAtAfterDVRWindowLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        int64_t afterWindow = 0;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        afterWindow = _smoothObject->GetCurrentEndTime() + DEFAULT_SKIP_10MHZ;

        PKTEST_FUNC_EXIT(_smoothObject->PlayAt(afterWindow));
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");
        
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "PlayAt past Right Edge should snap to right Edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "should be in playing state" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedPlayAtAfterDVRWindowOD,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t afterWindow = 0;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        //Play to the left of DVR window and make sure the timestamp is correct
        afterWindow = _smoothObject->GetCurrentEndTime() + DEFAULT_SKIP_10MHZ;
        PKTEST_HRESULT_EXIT(_smoothObject->PlayAt(afterWindow));
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, IMMEDIATE_STATECHANGE_TIMEOUT), "PlayAt past right on VOD should get media Ended Immediately" );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "PlayAt past Right Edge should snap to right Edge" );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "PlayAt past Right Edge should get mediaEnded" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedPlayAtAfterDVRWindowLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        int64_t afterWindow = 0;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        afterWindow = _smoothObject->GetCurrentEndTime() + DEFAULT_SKIP_10MHZ;
        PKTEST_FUNC_EXIT(_smoothObject->PlayAtAndValidate(afterWindow));
        
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "PlayAt past Right Edge should snap to right Edge" );
    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedPlayAtAfterDVRWindowOD,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t afterWindow = 0;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        //Play to the left of DVR window and make sure the timestamp is correct
        afterWindow = _smoothObject->GetCurrentEndTime() + DEFAULT_SKIP_10MHZ;
        PKTEST_HRESULT_EXIT(_smoothObject->PlayAt(afterWindow));
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, IMMEDIATE_STATECHANGE_TIMEOUT), "PlayAt past right on VOD should get media Ended Immediately" );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "PlayAt past Right Edge should snap to right Edge" );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "PlayAt past Right Edge should get mediaEnded" );
    exit:
        return;
    }

    ///Seek
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedSeekInDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedTimeStamp = 0;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        expectedTimeStamp = _smoothObject->GetRandomSeekablePosition();
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(expectedTimeStamp) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedSeekInDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedTimeStamp = 0;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        expectedTimeStamp = _smoothObject->GetRandomSeekablePosition();
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(expectedTimeStamp) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedSeekBeforeDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedTimeStamp = 0;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        if(_smoothObject->GetCurrentStartTime() > DEFAULT_SKIP_10MHZ)
        {
            expectedTimeStamp = _smoothObject->GetCurrentStartTime() - DEFAULT_SKIP_10MHZ;
        }
        PKTEST_HRESULT_EXIT( _smoothObject->Seek(expectedTimeStamp) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowStart(), "Seek past left Edge should snap to left Edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "should be in playing state" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedSeekBeforeDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedTimeStamp = 0;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        if(_smoothObject->GetCurrentStartTime() > DEFAULT_SKIP_10MHZ)
        {
            expectedTimeStamp = _smoothObject->GetCurrentStartTime() - DEFAULT_SKIP_10MHZ;
        }
        PKTEST_HRESULT_EXIT( _smoothObject->Seek(expectedTimeStamp) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowStart(), "Seek past left Edge should snap to left Edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "should be in playing state" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedSeekAfterDVRWindowOD,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedTimeStamp = 0;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        expectedTimeStamp = (_smoothObject->GetCurrentEndTime() + DEFAULT_SKIP_10MHZ);
        PKTEST_HRESULT_EXIT( _smoothObject->Seek(expectedTimeStamp) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, IMMEDIATE_STATECHANGE_TIMEOUT), "Seek past right on VOD should get media Ended Immediately" );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "Seek past Right Edge should snap to right Edge" );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Seek past Right Edge should get mediaEnded" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedSeekAfterDVRWindowLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        int64_t expectedTimeStamp = 0;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        expectedTimeStamp = (_smoothObject->GetCurrentEndTime() + DEFAULT_SKIP_10MHZ);
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(expectedTimeStamp) );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "Seek past Right Edge should snap to right Edge" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedSeekAfterDVRWindowOD,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedTimeStamp = 0;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        expectedTimeStamp = (_smoothObject->GetCurrentEndTime() + DEFAULT_SKIP_10MHZ);
        PKTEST_HRESULT_EXIT( _smoothObject->Seek(expectedTimeStamp) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, IMMEDIATE_STATECHANGE_TIMEOUT), "Seek past right on VOD should get media Ended Immediately" );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "Seek past Right Edge should snap to right Edge" );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Seek past Right Edge should get mediaEnded" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedSeekAfterDVRWindowLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        int64_t expectedTimeStamp = 0;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        expectedTimeStamp = (_smoothObject->GetCurrentEndTime() + DEFAULT_SKIP_10MHZ);
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(expectedTimeStamp) );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "PlayAt past Right Edge should snap to right Edge" );

    exit:
        return;
    }

    ///Skip
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedSkipInDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedTimeStamp = 0, currentTime = 0, skipSeconds = 0;
        pkRESULT pkResult;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        //Try to seek to the middle of DVR window.  It should fail.
        expectedTimeStamp = _smoothObject->GetRandomSeekablePosition();
        currentTime = _smoothObject->GetCurrentPlayBackTime();
        
        skipSeconds = ( (expectedTimeStamp > currentTime) ? expectedTimeStamp - currentTime : currentTime - expectedTimeStamp ) / TIMESCALE_10MHZ;
        
        //Try to skip into the middle of DVR window. It should fail
        PKTEST_ASSERT_EXIT(skipSeconds == (int32_t)skipSeconds)
        pkResult = _smoothObject->Skip((int32_t)skipSeconds);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);
        PKTEST_ASSERT_EXIT(_smoothObject->IsMediaEnded());

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedSkipInDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t expectedTimeStamp = 0, currentTime = 0, skipSeconds = 0;
        pkRESULT pkResult;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        //Try to seek to the middle of DVR window.  It should fail.
        expectedTimeStamp = _smoothObject->GetRandomSeekablePosition();
        currentTime = _smoothObject->GetCurrentPlayBackTime();
        
        skipSeconds = ( (expectedTimeStamp > currentTime) ? expectedTimeStamp - currentTime : currentTime - expectedTimeStamp ) / TIMESCALE_10MHZ;

        //Try to skip into the middle of DVR window. It should fail
        PKTEST_ASSERT_EXIT(skipSeconds == (int32_t)skipSeconds)
        pkResult = _smoothObject->Skip((int32_t)skipSeconds);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);
        PKTEST_ASSERT_EXIT(_smoothObject->IsMediaEnded());

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedSkipBeforeDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        pkRESULT pkResult;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));
        
        //Try to skip to time before DVR window. It should fail
        pkResult = _smoothObject->Skip(-TOTAL_SECONDS_IN_A_DAY);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);
        PKTEST_ASSERT_EXIT(_smoothObject->IsMediaEnded());

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedSkipBeforeDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        pkRESULT pkResult;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));
        
        //Try to skip to time before DVR window. It should fail
        pkResult = _smoothObject->Skip(-TOTAL_SECONDS_IN_A_DAY);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);
        PKTEST_ASSERT_EXIT(_smoothObject->IsMediaEnded());

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedSkipAfterDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        pkRESULT pkResult;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));
        
        //Try to skip to time after DVR window. It should fail
        pkResult = _smoothObject->Skip(TOTAL_SECONDS_IN_A_DAY);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);
        PKTEST_ASSERT_EXIT(_smoothObject->IsMediaEnded());

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedSkipAfterDVRWindow,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        pkRESULT pkResult;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));
        
        //Try to skip to time after DVR window. It should fail
        pkResult = _smoothObject->Skip(TOTAL_SECONDS_IN_A_DAY);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);
        PKTEST_ASSERT_EXIT(_smoothObject->IsMediaEnded());

    exit:
        return;
    }

    ///Close
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedClose,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        PKTEST_FUNC_EXIT( _smoothObject->CloseAndValidate(VALIDATION_DELAY) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedClose,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        PKTEST_FUNC_EXIT( _smoothObject->CloseAndValidate(VALIDATION_DELAY) );

    exit:
        return;
    }

    ///FastForward
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedFastForward,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        PKTEST_HRESULT_EXIT( _smoothObject->FastForward(TRICK_PLAY_SPEED_MEDIUM) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");
        
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsAtDVRWindowStart(), "FastForward after mediaEnded should snap to left edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsInFastForward(), "Should be in fastforward" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedFastForwardLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        PKTEST_HRESULT_EXIT( _smoothObject->FastForward(TRICK_PLAY_SPEED_MEDIUM) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");
        
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsAtDVRWindowEnd(), "FastForward after mediaEnded should snap to righ edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsInFastForward(), "Should be in fastforward" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedFastForwardOD,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        PKTEST_HRESULT_EXIT( _smoothObject->FastForward(TRICK_PLAY_SPEED_MEDIUM) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, IMMEDIATE_STATECHANGE_TIMEOUT), "Fastforward past right on VOD should get media Ended Immediately" );

        //Should snap to right edge
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowEnd(), "Fastforward Past Right Edge should snap to right Edge" );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Fastforward Past Right Edge should get mediaEnded" );
       
    exit:
        return;
    }

    ///Rewind
    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeMediaEndedRewind,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true));

        PKTEST_HRESULT_EXIT( _smoothObject->Rewind(TRICK_PLAY_SPEED_MEDIUM) );

        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, IMMEDIATE_STATECHANGE_TIMEOUT), "Rewind past left should get media Ended Immediately" );

        //Should snap to left edge
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowStart(), "Rewind Past Left Edge should snap to Left Edge" );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Rewind Past Left Edge should get mediaEnded" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeMediaEndedRewind,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false));

        PKTEST_HRESULT_EXIT( _smoothObject->Rewind(TRICK_PLAY_SPEED_MEDIUM) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");
        
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsAtDVRWindowEnd(), "Rewind after mediaEnded should snap to right edge" );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsInRewind(), "Should be in Rewind" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_RightEdgeStartPlaybackFromMediaEndedAtSameSpeedUsedToReachMediaEnded,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(false, TRICK_PLAY_SPEED_LOW));
        
        //Make sure that trying to play again at the same speed does not fail
        PKTEST_HRESULT_EXIT(_smoothObject->Play(TRICK_PLAY_SPEED_LOW));

    exit:
        return;
    }

    /////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(MediaEndedTests_LeftEdgeStartPlaybackFromMediaEndedAtSameSpeedUsedToReachMediaEnded,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Data:VC1LiveDefault", "VC1LiveDefault")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        PKTEST_FUNC_EXIT(ReachMediaEnded(true, TRICK_PLAY_SPEED_LOW));
        
        //Make sure that trying to play again at the same speed does not fail
        PKTEST_HRESULT_EXIT(_smoothObject->Play(TRICK_PLAY_SPEED_LOW));

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////////////
    bool ReachMediaEnded(bool leftEdge = false, float speed = TRICK_PLAY_SPEED_LOW)
    {
        if(leftEdge)
        {
            PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(_smoothObject->GetCurrentStartTime() + TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(VALIDATION_DELAY)).Ticks() + LIVE_LEFT_BACKOFF_TIME ));
            PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(speed, VALIDATION_DELAY * 2) );
        }
        else
        {
            PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate( _smoothObject->GetCurrentEndTime() - ( TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(VALIDATION_DELAY)).Ticks() + LIVE_BACKOFF_TIME )));
            PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(speed, VALIDATION_DELAY * 2) );
        }

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaEnded( STATECHANGE_TIMEOUT ), "TimeOut happened while waiting for the MediaEnded");

        return true;
    exit:
        return false;
    }
};
