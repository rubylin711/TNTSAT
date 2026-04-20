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
#include "STUtilities.h"
#include "SSPKTimeSpan.h"
#include <math.h>

using namespace SSPK;
using namespace SSPKTest;

#define PLAYSTATE_ITERATIONS_LIVE_OD \
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault") \
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault") \
        PKTEST_PROPERTY("Data:ODProtectedPiff1.1", "ODProtectedPiff1.1") \
        PKTEST_PROPERTY("Data:LiveProtectedPiff1.3ShortKR", "LiveProtectedPiff1.3ShortKR") \
        PKTEST_PROPERTY("Data:ODProtectedPiff1.3KROnEveryFragment", "ODProtectedPiff1.3KROnEveryFragment") \

#define PLAYSTATE_ITERATIONS_LIVE_OD_LONG \
        PKTEST_PROPERTY("Data:H264ODMultiAudio", "H264ODMultiAudio") \
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault") \
        PKTEST_PROPERTY("Data:ODProtectedPiff1.1", "ODProtectedPiff1.1") \
        PKTEST_PROPERTY("Data:LiveProtectedPiff1.3ShortKR", "LiveProtectedPiff1.3ShortKR") \
        PKTEST_PROPERTY("Data:ODProtectedPiff1.3KROnEveryFragment", "ODProtectedPiff1.3KROnEveryFragment") \

#define PLAYSTATE_ITERATIONS_LIVE \
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault") \
        PKTEST_PROPERTY("Data:LiveProtectedPiff1.3ShortKR", "LiveProtectedPiff1.3ShortKR") \

#define PLAYSTATE_ITERATIONS_OD \
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault") \
        PKTEST_PROPERTY("Data:ODProtectedPiff1.1", "ODProtectedPiff1.1") \
        PKTEST_PROPERTY("Data:ODProtectedPiff1.3KROnEveryFragment", "ODProtectedPiff1.3KROnEveryFragment") \

PKTEST_GROUP( PlayStateTests )
{

    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;

    static const int32_t PLAYBACK_VALIDATION_DURATION = 7000; //7 SECONDS
    static const int32_t LONG_PLAYBACK_TIMEOUT = 40000;//40 SECONDS
    static const int32_t SHORT_PLAY_DURATION = 10000;//10 SECONDS
    static const int32_t LONG_WAIT = 60000; //60 SECONDS
    static const int32_t NORMAL_WAIT = 10000; //10 SECONDS
    static const int32_t SHORT_WAIT = 1500; //1.5 SECONDS
    static const int32_t PLAYBACK_SPEED_CHANGE_WAIT = 2000; //2 SECONDS
    static const int32_t REPEATED_ACTION_COUNT = 5;
    static const int32_t REPEATED_ACTION_COUNT_DOUBLE = REPEATED_ACTION_COUNT * 2;
    static const int32_t REPEATED_ACTION_COUNT_MEDIUM = REPEATED_ACTION_COUNT * 5;

    static const int32_t REPEATED_ACTION_COUNT_HIGH = 100;
    static const int32_t STATUS_CALLBACK_VERIFY_COUNT = 10;

    class StatusUpdateMonitor
    {
    public:
        StatusUpdateMonitor(ESmoothTransportStatusUpdate updateType, STWrapper* wrapper):
            m_updateToLookFor(updateType),
            m_wrapper(wrapper)
        {
            m_statusListSearchBegin = m_wrapper->GetStatusCallbackVector().size();//The first index to look at will be 1 more than the last one
        }

        bool UpdateOfTypeOccurred()
        {
            uint32_t end = m_wrapper->GetStatusCallbackVector().size();
            for(uint32_t i = m_statusListSearchBegin; i < end; ++i)
            {
                if(m_updateToLookFor == m_wrapper->GetStatusCallbackVector()[i]._update)
                {
                    return true;
                }
            }
            return false;
        }

        uint32_t m_statusListSearchBegin;
        ESmoothTransportStatusUpdate m_updateToLookFor;
        STWrapper* m_wrapper;
    };
    //
    static const int32_t TRICK_PLAY_STEP = 8;//Because of Bug 30302, only speeds that are a multiple 8 work

#ifdef _WINDOWS_
    static const int32_t SOCKET_TIMEOUT = 180000; //According to something??
#else
    static const int32_t SOCKET_TIMEOUT = 65000; //According to tcp man the default timeout is 60 seconds on Ubuntu
#endif

    PlayStateTests()
    {
        CXMLElement xTestData = PKTest_GetDataTable( "IntegrationTestSources" );
        if( !xTestData.IsNull() )
        {
             _xManifests = xTestData.Elements(L"source");
        }
    }

    //Setup
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
        return SUCCEEDED(pkResult);
    }
    
    bool StartSmoothObject( string sourceName, bool autoPlay, bool expectNoManifestReady = false)
    {
        string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(sourceName).c_str());
        PKTEST_HRESULT_EXIT( _smoothObject->OpenVideo(urlString, autoPlay, !expectNoManifestReady) );

        if(expectNoManifestReady)
        {
            PKTEST_ASSERT_MSG_EXIT( !_smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure expected while waiting for the manifest ready but did not happen." );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );
        }

        if(autoPlay)
        {
            PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");
            _smoothObject->Delay(VALIDATION_DELAY);
        }

        return true;
    exit:
        return false;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    //Utility Methods
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////
    //Values often retrieved
    int64_t GetCurrentPlaybackTimeMilliseconds()
    {
        return TimestampToMilliseconds(_smoothObject->GetCurrentPlayBackTime());
    }

    int64_t DurationInMilliseconds()
    {
        return TimestampToMilliseconds(_smoothObject->GetManifest()->Duration());
    }

    double DurationInSeconds()
    {
        return MillisecondsToSeconds(TimestampToMilliseconds(_smoothObject->GetManifest()->Duration()));
    }

    ///////////////////////////////////////////////////////////////
    //Operations often used

    bool PlayToMediaEndedState()
    {
        PKTEST_FUNC_EXIT( _smoothObject->SeekFromEnd(PLAYBACK_VALIDATION_DURATION) );
        PKTEST_HRESULT_EXIT( _smoothObject->Play() );
        PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaEnded(PLAYBACK_VALIDATION_DURATION + STATECHANGE_TIMEOUT) );
        return _smoothObject->IsMediaEnded();
    exit:
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //Play Tests
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayFromOpen,
        PKTEST_PROPERTY( "Priority", "0" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayToEnd,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_OD
        )
    {
        int64_t expectedDuration;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        expectedDuration = DurationInMilliseconds();
        PKTEST_MSG("Expected Duration: %lld milliseconds", expectedDuration);

        //SHORT_WAIT is to allow a few seconds for rounding error in conversion
        PKTEST_ASSERT_EXIT((int32)(expectedDuration + SHORT_WAIT) == (expectedDuration + SHORT_WAIT));
        PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaEnded( (int32)(expectedDuration + SHORT_WAIT) ) );
        PKTEST_ASSERT_EXIT( _smoothObject->IsMediaEnded() );
    exit:

        return;
    }

    ////////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAfterLongTimePause,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(SOCKET_TIMEOUT + LONG_WAIT) );
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );

    exit:

        return;
    }

    ////////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAfterShortTimePause,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(SHORT_WAIT) );
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayContentWithLongVideoChunkAndShortAudioChunk,
        PKTEST_PROPERTY( "Priority", "1" )
        )
    {
        PKTEST_FUNC_EXIT( StartSmoothObject("LongVideoChunkAndShortAudioChunk", false) );
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate( NORMAL_WAIT ) );

        //Seek from end
        PKTEST_FUNC_EXIT( _smoothObject->SeekFromEnd(LONG_WAIT) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate( NORMAL_WAIT ) );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayContentWhichHasSampleSizeDefined,
        PKTEST_PROPERTY( "Priority", "1" )
        )
    {
        PKTEST_FUNC_EXIT( StartSmoothObject("SamplesizeDefinedFromSixthAudiosample", false) );
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );
   
   exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayContentWhichHasIncorrectlyDefinedBaseDataOffset,
        PKTEST_PROPERTY( "Priority", "1" )
    )
    {
        PKTEST_FUNC_EXIT( StartSmoothObject("IncorrectlyDefinedBaseDataOffset", false) );
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayFromMediaEnded,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_OD
        PKTEST_PROPERTY("Bug","30637")
        )
    {
        int64_t currentTime = 0;
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        PKTEST_FUNC_EXIT( PlayToMediaEndedState() );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Media has not MediaEnded as expected");

        //Try to play again, should start from the beginning
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );
        currentTime = GetCurrentPlaybackTimeMilliseconds();
        PKTEST_ASSERT_MSG_EXIT(currentTime <= (int64_t )(VALIDATION_DELAY + SHORT_WAIT), "Current playback time should be within the range [0,%lld), but is %lld", (int64_t )(VALIDATION_DELAY + SHORT_WAIT) , currentTime );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayFromFastForward,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );
        PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY) );
        //Make sure speed returns to normal
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );
        PKTEST_ASSERT_MSG_EXIT(!_smoothObject->IsInFastForward() && !_smoothObject->IsInRewind(), "Playback speed is outside of the expected range");

    exit:
        return;
    }


    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayFromRewind,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
        PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY) );
        //Make sure speed returns to normal
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );
    exit:

        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayFromAutoPlay,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        {
            StatusUpdateMonitor statusCheck( SmoothTransportStatus_TunerStateChanged, _smoothObject);
            PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );

            PKTEST_ASSERT_MSG_EXIT( !statusCheck.UpdateOfTypeOccurred(), "Did not expect status TunerStateChanged update but one occurred");
        }
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayFromPlayRepeatedly,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );
        {
            StatusUpdateMonitor statusCheck( SmoothTransportStatus_TunerStateChanged, _smoothObject);

            for(int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
            {
                PKTEST_FUNC_EXIT(_smoothObject->PlayAndValidate(SHORT_WAIT));
            }

            PKTEST_ASSERT_MSG_EXIT( !statusCheck.UpdateOfTypeOccurred(), "Did not expect status TunerStateChanged update but one occurred");
        }
    exit:
        return;
    }

    PKTEST_METHOD_EX(PlayStateTests_PlayWithSpeed0,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT( StartSmoothObject(PKTest_GetTestData(), false) );
        
        PKTEST_FUNC_EXIT( _smoothObject->Play(0) );
        PKTEST_FUNC_EXIT( _smoothObject->Delay(VALIDATION_DELAY) );
        PKTEST_ASSERT_MSG_EXIT( !_smoothObject->IsPlaying(), "should not be in playing state" );

    exit:
        return;
    }

    PKTEST_METHOD_EX(PlayStateTests_PlayWithDecimalSpeed,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    { 
        const float decimalRate = 0.7F;
        
        PKTEST_FUNC_EXIT( StartSmoothObject(PKTest_GetTestData(), true) );
        for(int32_t i = 1; i <= REPEATED_ACTION_COUNT; i++)
        {
            pkRESULT pkResult = pkS_OK;
        
            pkResult = _smoothObject->Play(i * decimalRate);
            PKTEST_ASSERT_MSG_EXIT(pkE_INVALIDARG == pkResult, "Expected pkE_INVALIDARG, but got 0x%X", pkResult);

            pkResult = _smoothObject->Play(-i * decimalRate);
            PKTEST_ASSERT_MSG_EXIT(pkE_INVALIDARG == pkResult, "Expected pkE_INVALIDARG, but got 0x%X", pkResult);
        }

    exit:
        return;
    }

    PKTEST_METHOD_EX(PlayStateTests_PlayWithJaggedEdge,
        PKTEST_PROPERTY("Priority", "0")
        PKTEST_PROPERTY("Data:ODH264JaggedVideoStartFirst", "ODH264JaggedVideoStartFirst")
        PKTEST_PROPERTY("Data:ODH264JaggedAudioStartFirst", "ODH264JaggedAudioStartFirst")
        PKTEST_PROPERTY("Data:ODH264JaggedVideoEndFirst", "ODH264JaggedVideoEndFirst")
        PKTEST_PROPERTY("Data:ODH264JaggedAudioEndirst", "ODH264JaggedAudioEndFirst")
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );

        PKTEST_FUNC_EXIT( _smoothObject->SeekFromEnd(2 * VALIDATION_DELAY, true) );
        
        PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaEnded(3 * STATECHANGE_TIMEOUT) );

    exit:
        return;

    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    //PlayAt Tests
    //////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtBeyondLeftEdge,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t startTime = 0, seekTime = 0;
        vector<SmoothTransportStatus> statusUpdates;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        statusUpdates = _smoothObject->GetStatusCallbackVector();

        startTime = TimeSpan_hns::ConvertFrom(statusUpdates[statusUpdates.size() - 1]._startTime).Ticks();

        if(startTime > DEFAULT_SKIP_10MHZ)
        {
            seekTime = startTime - DEFAULT_SKIP_10MHZ;
        }

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(seekTime) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtBeyondLeftEdgeWithRewind,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t startTime = 0, seekTime = 0;
        vector<SmoothTransportStatus> statusUpdates;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        statusUpdates = _smoothObject->GetStatusCallbackVector();

        startTime = TimeSpan_hns::ConvertFrom(statusUpdates[statusUpdates.size() - 1]._startTime).Ticks();

        if(startTime > DEFAULT_SKIP_10MHZ)
        {
            seekTime = startTime - DEFAULT_SKIP_10MHZ;
        }

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(seekTime, -DEFAULT_TRICK_PLAY_SPEED) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtBeyondLeftEdgeWithFastForward,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t startTime = 0, seekTime = 0;
        vector<SmoothTransportStatus> statusUpdates;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        statusUpdates = _smoothObject->GetStatusCallbackVector();

        startTime = TimeSpan_hns::ConvertFrom(statusUpdates[statusUpdates.size() - 1]._startTime).Ticks();

        if(startTime > DEFAULT_SKIP_10MHZ)
        {
            seekTime = startTime - DEFAULT_SKIP_10MHZ;
        }

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(seekTime, DEFAULT_TRICK_PLAY_SPEED) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtBeyondRightEdgeOD,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_OD
        )
    {
        int64_t endTime = 0;
        vector<SmoothTransportStatus> statusUpdates;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        statusUpdates = _smoothObject->GetStatusCallbackVector();

        endTime = TimeSpan_hns::ConvertFrom(statusUpdates[statusUpdates.size() - 1]._endTime).Ticks();

        PKTEST_HRESULT_EXIT( _smoothObject->PlayAt(endTime + DEFAULT_SKIP_10MHZ) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, STATECHANGE_TIMEOUT ), "Timeout occured waiting for MediaEnded" );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtBeyondRightEdgeLive,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE
        )
    {
        int64_t endTime = 0;
        vector<SmoothTransportStatus> statusUpdates;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        statusUpdates = _smoothObject->GetStatusCallbackVector();

        endTime = TimeSpan_hns::ConvertFrom(statusUpdates[statusUpdates.size() - 1]._endTime).Ticks();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(endTime + DEFAULT_SKIP_10MHZ) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtBeyondRightEdgeWithRewind,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t endTime = 0;
        vector<SmoothTransportStatus> statusUpdates;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        statusUpdates = _smoothObject->GetStatusCallbackVector();

        endTime = TimeSpan_hns::ConvertFrom(statusUpdates[statusUpdates.size() - 1]._endTime).Ticks();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(endTime + DEFAULT_SKIP_10MHZ, -DEFAULT_TRICK_PLAY_SPEED) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtBeyondRightEdgeWithFastForward,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t endTime = 0;
        vector<SmoothTransportStatus> statusUpdates;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        statusUpdates = _smoothObject->GetStatusCallbackVector();

        endTime = TimeSpan_hns::ConvertFrom(statusUpdates[statusUpdates.size() - 1]._endTime).Ticks();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(endTime + DEFAULT_SKIP_10MHZ, DEFAULT_TRICK_PLAY_SPEED) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtWithInDVR,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t playAtPosition = 0;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        playAtPosition = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(playAtPosition) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtWithInDVRWithRewind,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t playAtPosition = 0;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        playAtPosition = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(playAtPosition, -DEFAULT_TRICK_PLAY_SPEED) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtWithInDVRWithFastForward,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t playAtPosition = 0;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        playAtPosition = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(playAtPosition, DEFAULT_TRICK_PLAY_SPEED) );

    exit:
        return;
    }

    ////////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtAfterLongTimePause,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t playAtPosition = 0;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(SOCKET_TIMEOUT + LONG_WAIT) );

        playAtPosition = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(playAtPosition) );

    exit:

        return;
    }

    /////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtDuringPause,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t playAtPosition = 0;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(SHORT_WAIT) );

        playAtPosition = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(playAtPosition) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtDuringPauseWithRewind,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t playAtPosition = 0;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(SHORT_WAIT) );

        playAtPosition = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(playAtPosition, -DEFAULT_TRICK_PLAY_SPEED) );

    exit:
        return;
    }

    /////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtDuringPauseWithFastForward,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t playAtPosition = 0;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(SHORT_WAIT) );

        playAtPosition = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(playAtPosition, DEFAULT_TRICK_PLAY_SPEED) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtDuringFastForward,
        PKTEST_PROPERTY( "Priority", "0" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t playAtPosition = 0;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );
        PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(LOW_SPEED, VALIDATION_DELAY) );

        playAtPosition = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(playAtPosition) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtDuringFastForwardWithRewind,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t playAtPosition = 0;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );
        PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(LOW_SPEED, VALIDATION_DELAY) );

        playAtPosition = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(playAtPosition, -DEFAULT_TRICK_PLAY_SPEED) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtDuringFastForwardWithFastForward,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t playAtPosition = 0;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );
        PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(LOW_SPEED, VALIDATION_DELAY) );

        playAtPosition = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(playAtPosition, DEFAULT_TRICK_PLAY_SPEED) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtDuringRewind,
        PKTEST_PROPERTY( "Priority", "0" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t playAtPosition = 0;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
        PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(LOW_SPEED, VALIDATION_DELAY) );

        playAtPosition = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(playAtPosition) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtDuringRewindWithRewind,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t playAtPosition = 0;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
        PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(LOW_SPEED, VALIDATION_DELAY) );

        playAtPosition = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(playAtPosition, -DEFAULT_TRICK_PLAY_SPEED) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PlayAtDuringRewindWithFastForward,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t playAtPosition = 0;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
        PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(LOW_SPEED, VALIDATION_DELAY) );

        playAtPosition = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(playAtPosition, DEFAULT_TRICK_PLAY_SPEED) );

    exit:
        return;
    }

    PKTEST_METHOD_EX(PlayStateTests_PlayAtWithSpeed0,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        PKTEST_PROPERTY("Bug", "31748")
        )
    {
        int64_t playAtPosition = 0;
        PKTEST_FUNC_EXIT( StartSmoothObject(PKTest_GetTestData(), true) );
        
        playAtPosition = _smoothObject->GetRandomSeekablePosition();
        PKTEST_FUNC_EXIT( _smoothObject->PlayAt(playAtPosition, 0) );
        PKTEST_FUNC_EXIT( _smoothObject->Delay(VALIDATION_DELAY) );
        PKTEST_ASSERT_MSG_EXIT( !_smoothObject->IsPlaying(), "should not be in playing state" );

    exit:
        return;
    }

    PKTEST_METHOD_EX(PlayStateTests_PlayAtWithDecimalSpeed,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    { 
        int64_t playAtPosition = 0;
        const float decimalRate = 0.7F;
        
        PKTEST_FUNC_EXIT( StartSmoothObject(PKTest_GetTestData(), true) );
        for(int32_t i = 1; i <= REPEATED_ACTION_COUNT; i++)
        {
            pkRESULT pkResult = pkS_OK;

            playAtPosition = _smoothObject->GetRandomSeekablePosition();

            pkResult = _smoothObject->PlayAt(playAtPosition, i * decimalRate);
            PKTEST_ASSERT_MSG_EXIT(pkE_INVALIDARG == pkResult, "Expected pkE_INVALIDARG, but got 0x%X", pkResult);

            pkResult = _smoothObject->PlayAt(playAtPosition, -i * decimalRate);
            PKTEST_ASSERT_MSG_EXIT(pkE_INVALIDARG == pkResult, "Expected pkE_INVALIDARG, but got 0x%X", pkResult);
        }

    exit:
        return;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    //Close Tests
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////
     PKTEST_METHOD_EX(PlayStateTests_CloseFromPlay,
        PKTEST_PROPERTY( "Priority", "0" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(VALIDATION_DELAY));

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_CloseRepeatedlyFromPlay,
        PKTEST_PROPERTY( "Priority", "1" )
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            PKTEST_HRESULT_EXIT( _smoothObject->Play() );
            _smoothObject->Delay(VALIDATION_DELAY);
            PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(VALIDATION_DELAY));
            PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_CloseRepeatedlyFromFastForward,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            PKTEST_FUNC_EXIT(_smoothObject->PlayAndValidate(VALIDATION_DELAY));
            PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY));
            PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(VALIDATION_DELAY));
            PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_CloseRepeatedlyFromRewind,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            PKTEST_FUNC_EXIT(_smoothObject->PlayAndValidate(VALIDATION_DELAY));
            PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY));
            PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(VALIDATION_DELAY));
            PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );
        }
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_CloseRepeatedlyFromClosed,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );
        PKTEST_FUNC_EXIT( _smoothObject->CloseAndValidate(VALIDATION_DELAY) );
        for(int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(SHORT_WAIT));
        }

    exit:
        return;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    //Pause Tests
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PauseFromPlay,
        PKTEST_PROPERTY("Priority", "0")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT(_smoothObject->PauseAndValidate(VALIDATION_DELAY));

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PauseNearTheEnd,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->SeekFromEnd(PLAYBACK_VALIDATION_DURATION) );
        _smoothObject->Delay(SHORT_WAIT);
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PauseRepeatedlyFromPlay,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            PKTEST_FUNC_EXIT(_smoothObject->PlayAndValidate(SHORT_PLAY_DURATION));
            PKTEST_FUNC_EXIT(_smoothObject->PauseAndValidate(VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PauseRepeatedlyFromFastForward,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );
            PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY));
            PKTEST_FUNC_EXIT(_smoothObject->PauseAndValidate(VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PauseRepeatedlyFromRewindOD,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        //Start and seek to position near the end so that there is room to rewind without hitting the beginning
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
            PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY));
            PKTEST_FUNC_EXIT(_smoothObject->PauseAndValidate(VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PauseRepeatedlyFromRewindLive,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
            PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY));
            PKTEST_FUNC_EXIT(_smoothObject->PauseAndValidate(VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_PauseRepeatedly,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        _smoothObject->Delay(NORMAL_WAIT);
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );
        {
            for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
            {
                PKTEST_FUNC_EXIT(_smoothObject->PauseAndValidate(VALIDATION_DELAY));
            }
        }
    exit:
        return;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //Fast Forward Tests
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardFromPlay,
        PKTEST_PROPERTY("Priority", "0")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );
        PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardFromPause,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );

        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );
        PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY));


    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardFromRewind,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->SeekFromEnd(PLAYBACK_VALIDATION_DURATION, true) );
        PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, NORMAL_WAIT) );
        PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardRepeatedlyChangingSpeed,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        float rates[] = {2.0f, 5.0f, 15.0f, 20.0f, 30.0f};
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        
        for(int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );
            PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(rates[i % (sizeof( rates ) / sizeof( rates[0] ))], VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardAndRewindAlternating,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD_LONG
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        for(int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
            PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, PLAYBACK_VALIDATION_DURATION));
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );
            PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, PLAYBACK_VALIDATION_DURATION));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardToEndOD,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_OD
        )
    {
        int64_t duration = 0;
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(SHORT_PLAY_DURATION) );

        PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY) );
        duration = (DurationInMilliseconds());
        PKTEST_ASSERT_EXIT((DWORD)(duration) == duration);
        PKTEST_ASSERT_EXIT(_smoothObject->WaitForMediaEnded((DWORD)((duration) / DEFAULT_TRICK_PLAY_SPEED + STATECHANGE_TIMEOUT)) );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Should be in MediaEnded state after fast forwarding to the end, but is not.");

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardBeyondLive,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE
        )
    {
        int64_t duration = 0;
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        PKTEST_HRESULT_EXIT( _smoothObject->FastForward(DEFAULT_TRICK_PLAY_SPEED) );
        duration = DurationInMilliseconds();
        PKTEST_ASSERT_EXIT(duration == (int32)duration);
        PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaEnded((int32)duration) );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Should be in MediaEnded state after fast forwarding to the end of live, but is not.");

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardToEndFromPause,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_OD
        )
    {
        int64_t duration = 0;
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );
        PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY) );
        duration = DurationInMilliseconds();
        PKTEST_ASSERT_EXIT(duration == (int32)duration);
        PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaEnded((int32)duration) );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Should be in MediaEnded state after fast forwarding to the end, but is not.");

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardSameSpeedRepeatedly,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD_LONG
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        {
            for(int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
            {
                PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );
                PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY));
            }
        }
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardDifferentLowSpeeds,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        for(int32_t i = 1; i < REPEATED_ACTION_COUNT_DOUBLE; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );

            //Set speed
            float newSpeed = i * 2.0f;
            LogTestComment("Speed is set to %.2f", newSpeed);
            PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(newSpeed, VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardDifferentSpeedsSmallStep,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD)
    {
        float step = 1;
        float speed = 1;

        //Start normally
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        
        //Slowly change speed
        for(int32_t i = 1; i < REPEATED_ACTION_COUNT_DOUBLE; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );

            //Set speed
            speed += step;
            LogTestComment("Speed is set to %.2f", speed);
            PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(speed, VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardMediumSpeeds,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD_LONG
        )
    {
        float step = 2;
        float speed = MEDIUM_SPEED;

        //Start normally
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        
        //Slowly change speed
        for(int32_t i = 1; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );

            //Set speed
            speed += step;
            LogTestComment("Speed is set to %.2f", speed);
            PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(speed, VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardHighSpeeds,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD_LONG
        )
    {
        float step = 10;
        float speed = HIGH_SPEED;

        //Start normally
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        //Slowly change speed
        for(int32_t i = 1; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );

            //Set speed
            speed += step;
            LogTestComment("Speed is set to %.2f", speed);
            PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(speed, VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardWorkingSpeeds,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD_LONG
        )
    {
        float step = TRICK_PLAY_STEP;
        float speed = TRICK_PLAY_STEP;

        //Start normally
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        
        //Try several speeds that should work (speeds that are multiples of 8).
        while( speed < (TRICK_PLAY_STEP * REPEATED_ACTION_COUNT) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );

            //Set speed
            speed += step;
            LogTestComment("Speed is set to %.2f", speed);
            PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(speed, VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //Rewind Tests
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_RewindFromPlay,
        PKTEST_PROPERTY("Priority", "0")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT(_smoothObject->PlayAndValidate(VALIDATION_DELAY) );
        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
        PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_RewindRepeatedlyFromPause,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        for(int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
            PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );
            PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_RewindFromFastForward,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD_LONG
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );

        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );
        PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY) );
        PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_RewindRepeatedlyChangingSpeed,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD_LONG
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        for(int32_t i = 1; i <= REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
            PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(LOW_SPEED * i, VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_RewindToStart,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        
        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
        //Rewind until start of DVR window
        PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(HIGH_SPEED, VALIDATION_DELAY) );
        PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaEnded( (uint32_t) TimeSpan_ms::ConvertFrom( TimeSpan_s::FromTicks( _smoothObject->GetManifest()->DVRWindowLength() / (int64_t)DEFAULT_TRICK_PLAY_SPEED ) ).Ticks() + STATECHANGE_TIMEOUT ) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Should be in MediaEnded state after rewinding to start, but is not.");

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_RewindToStartFromPause,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );

        //Rewind to start
        PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(HIGH_SPEED, VALIDATION_DELAY) );
        PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaEnded( (uint32_t) TimeSpan_ms::ConvertFrom( TimeSpan_s::FromTicks( _smoothObject->GetManifest()->DVRWindowLength() / (int64_t)DEFAULT_TRICK_PLAY_SPEED ) ).Ticks() + STATECHANGE_TIMEOUT ) );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Should be in MediaEnded state after rewinding to start, but is not.");

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_RewindDifferentLowSpeeds,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD_LONG
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        for(int32_t i = 1; i < REPEATED_ACTION_COUNT_DOUBLE; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );

            //Set speed
            float newSpeed = i * 2.0f;
            LogTestComment("Speed is set to %.2f", newSpeed);
            PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(newSpeed, VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_RewindDifferentSpeedsSmallStep,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD_LONG
        )
    {
        float step = 1;
        float speed = -1;

        //Start normally
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        //Slowly change speed
        for(int32_t i = 1; i < REPEATED_ACTION_COUNT_DOUBLE; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );

            //Set speed
            speed -= step;
            LogTestComment("Speed is set to %.2f", speed);
            PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(speed, VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_RewindMediumSpeeds,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD_LONG
        )
    {
        float step = 2;
        float speed = -MEDIUM_SPEED;

        //Start normally
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        //Slowly change speed
        for(int32_t i = 1; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
            
            //Set speed
            speed -= step;
            LogTestComment("Speed is set to %.2f", speed);
            PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(speed, VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_RewindHighSpeeds,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD_LONG
        )
    {
        float step = 10;
        float speed = -HIGH_SPEED;

        //Start normally
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        //Slowly change speed
        for(int32_t i = 1; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );

            //Set speed
            speed -= step;
            LogTestComment("Speed is set to %.2f", speed);
            PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(speed, VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_RewindWorkingSpeeds,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        float step = TRICK_PLAY_STEP;
        float speed = -TRICK_PLAY_STEP;

        //Start normally
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        //Try several speeds that should work (speeds that are multiples of 8).
        for(int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );

            //Set speed
            speed -= step;
            LogTestComment("Speed is set to %.2f", speed);
            PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(speed, VALIDATION_DELAY));
        }

    exit:
        return;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //Seek Tests
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SeekWhenPlaying,
        PKTEST_PROPERTY("Priority", "0")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t seekTime = 0;


        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT(_smoothObject->SeekAndValidate(seekTime));
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SeekRepeatedlyWhenPlaying,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t baseTime;
        int64_t seekToPosition[] = { 4000, 9000, 23000, 101000, 35000 };
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        baseTime = TimestampToMilliseconds(_smoothObject->GetCurrentPlayBackTime());
        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            if(_smoothObject->GetManifest()->IsLive())
            {
                PKTEST_FUNC_EXIT(_smoothObject->SeekMilliseconds(baseTime - seekToPosition[i%(sizeof(seekToPosition)/sizeof(seekToPosition[0]))], true));

            }
            else
            {
                PKTEST_FUNC_EXIT(_smoothObject->SeekMilliseconds(baseTime + seekToPosition[i%(sizeof(seekToPosition)/sizeof(seekToPosition[0]))], true));
            }
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SeekRepeatedlyWhenPaused,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE_OD)
    {
        int64_t baseTime;
        int64_t seekToPosition[] = { 4000, 9000, 23000, 101000, 35000 };
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        baseTime = TimestampToMilliseconds(_smoothObject->GetCurrentPlayBackTime());

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            PKTEST_FUNC_EXIT(_smoothObject->SeekMilliseconds(baseTime + seekToPosition[i % (sizeof(seekToPosition)/sizeof(seekToPosition[0]))], true));
            _smoothObject->Delay(VALIDATION_DELAY);
            PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Expected the player to still be in playing state after seeking in the paused state, but it is not" );
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SeekToEnd,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );

        PKTEST_FUNC_EXIT( _smoothObject->SeekFromEnd(MIN_TIME64, false) );
        PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaEnded(STATECHANGE_TIMEOUT) );

        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(_smoothObject->GetManifest()->Duration(), _smoothObject->GetCurrentPlayBackTime(), TIMESTAMP_COMPARISON_THRESHOLD), "Seeked to end but playback time (%lld) does not match duration (%lld)",_smoothObject->GetCurrentPlayBackTime() ,_smoothObject->GetManifest()->Duration());
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsMediaEnded(), "Media seeked to end but did not enter MediaEnded state",_smoothObject->GetCurrentPlayBackTime() ,_smoothObject->GetManifest()->Duration());

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SeekPastEndOD,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );

        PKTEST_FUNC_EXIT( _smoothObject->SeekMilliseconds(TOTAL_SECONDS_IN_A_DAY * MILLISECONDS_PER_SECOND ) );

        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(_smoothObject->GetManifest()->Duration() , _smoothObject->GetCurrentPlayBackTime(), TIMESTAMP_COMPARISON_THRESHOLD), "Seeked past end but playback time (%lld) does not match duration (%lld)",_smoothObject->GetCurrentPlayBackTime() ,_smoothObject->GetManifest()->Duration());
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsMediaEnded(), "Seeked past end but did not enter MediaEnded state");

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SeekPastEndLive,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );

        PKTEST_FUNC_EXIT( _smoothObject->SeekMilliseconds(_smoothObject->GetCurrentEndTime() + DEFAULT_SKIP_10MHZ, true ) );
        _smoothObject->Delay(VALIDATION_DELAY);
        
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsPlaying(), "Seeked past end but did not start playing");

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SeekToStart,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t baseTime;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );
        baseTime = TimestampToMilliseconds(_smoothObject->GetCurrentPlayBackTime());
        PKTEST_FUNC_EXIT( _smoothObject->SeekMilliseconds(baseTime, true) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SeekToSamePositionRepeatedly,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t seekTime;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate( PLAYBACK_VALIDATION_DURATION ) );

        seekTime = GetCurrentPlaybackTimeMilliseconds();
        _smoothObject->Delay(PLAYBACK_VALIDATION_DURATION);

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            _smoothObject->Delay(SHORT_WAIT);
            PKTEST_FUNC_EXIT(_smoothObject->SeekMilliseconds(seekTime, true));
        }

    exit:

        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SeekCloseToLive,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE
        )
    {
        int64_t startTime, endTime;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        _smoothObject->GetCurrentPlayablePosition(&startTime, &endTime);
        
        PKTEST_FUNC_EXIT( _smoothObject->SeekMilliseconds(endTime) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SeekToLiveFromPlay,
        PKTEST_PROPERTY("Priority", "0")
        PLAYSTATE_ITERATIONS_LIVE
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );
        PKTEST_FUNC_EXIT( _smoothObject->SeekToLive());

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SeekToLiveFromPause,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE)
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(NORMAL_WAIT) );
        PKTEST_FUNC_EXIT( _smoothObject->SeekToLive(true) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SeekPastEndWhilePausedOD,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );
        //Seek past end
        PKTEST_FUNC_EXIT( _smoothObject->SeekMilliseconds(TOTAL_SECONDS_IN_A_DAY * MILLISECONDS_PER_SECOND, false ) );
        PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaEnded(STATECHANGE_TIMEOUT) );
        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(_smoothObject->GetManifest()->Duration() , _smoothObject->GetCurrentPlayBackTime(), TIMESTAMP_COMPARISON_THRESHOLD), "Skipped to end but playback time (%lld) does not match duration (%lld)",_smoothObject->GetCurrentPlayBackTime() ,_smoothObject->GetManifest()->Duration());
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsMediaEnded(), "Media skipped to end but did not enter MediaEnded state" );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SeekPastEndWhilePausedLive,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );
        //Seek past end
        PKTEST_FUNC_EXIT( _smoothObject->SeekMilliseconds(TOTAL_SECONDS_IN_A_DAY * MILLISECONDS_PER_SECOND ) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Expected the player to be in playing after Seeking past end in the paused state, but it is no longer playing" );

    exit:
        return;
    }
    

    ////////////////////////////////////////////////////////////////
    //Skip Tests
    ////////////////////////////////////////////////////////////////
    static const int32_t SHORT_SKIP = 1;
    static const int32_t MEDIUM_SKIP = 3;
    static const int32_t LARGE_SKIP = 10;
    static const int32_t TEN_MINUTE_SKIP = 600;

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipForward,
        PKTEST_PROPERTY("Priority", "0")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT(_smoothObject->SkipAndValidate(SHORT_SKIP));

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipBackward,
        PKTEST_PROPERTY("Priority", "0")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_FUNC_EXIT(_smoothObject->SkipAndValidate(-SHORT_SKIP));

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipBackPastStart,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        //One large skip that should be past the start
        PKTEST_FUNC_EXIT( _smoothObject->SkipAndValidate(-1 * TOTAL_SECONDS_IN_A_DAY) );
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );

        PKTEST_ASSERT_EXIT( _smoothObject->IsPlaying() );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipForwardPastEndOD,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        //One large skip that should be past the end
        PKTEST_FUNC_EXIT( _smoothObject->Skip(TOTAL_SECONDS_IN_A_DAY) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingAndEnded(false, STATECHANGE_TIMEOUT), "After skip to end, we should get a MediaEnded within 3 seconds");
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Expected the player to be in MediaEnded state after skipping past End, but it is not");
        PKTEST_ASSERT_MSG_EXIT(CompareTimeStamps( _smoothObject->GetManifest()->Duration(), _smoothObject->GetCurrentPlayBackTime(), TIMESTAMP_COMPARISON_THRESHOLD ), "Skipped past end but playback time (%lld) does not match duration (%lld)",_smoothObject->GetCurrentPlayBackTime() ,_smoothObject->GetManifest()->Duration());

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipForwardPastEndLive,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        //One large skip that should be past the end
        PKTEST_FUNC_EXIT( _smoothObject->SkipAndValidate(TOTAL_SECONDS_IN_A_DAY) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Expected the player to be in playing after Skipping beyond End, but it is no longer playing" );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipForwardRepeatedly,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        for(int32_t i = 0; i < REPEATED_ACTION_COUNT_HIGH; ++i)
        {
            if(_smoothObject->IsMediaEnded())
            {
                break;
            }
            PKTEST_FUNC_EXIT(_smoothObject->SkipAndValidate(SHORT_SKIP));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipForwardRepeatedlyWhenPlaying,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() ); // using this so that we are ready for skip fronts
        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            _smoothObject->Delay(SHORT_WAIT);
            PKTEST_FUNC_EXIT(_smoothObject->SkipAndValidate(SHORT_SKIP));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipBackwardRepeatedlyWhenPlaying,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() ); // using this so that we are ready for skip backs
        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            PKTEST_FUNC_EXIT(_smoothObject->SkipAndValidate(-1 * LARGE_SKIP));
            _smoothObject->Delay(VALIDATION_DELAY);
            PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsPlaying(), "Expected the object to be playing but it is not");
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipRepeatedlyWhenPlaying,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT * 2; i++)
        {
            //Skip back and forth
            PKTEST_FUNC_EXIT(_smoothObject->SkipAndValidate( ((i % 2)?1:-1)* SHORT_SKIP));
            _smoothObject->Delay(SHORT_WAIT);
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipToEndOD,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_OD
        )
    {
        double diff = 0;
        int32_t skip = 0;
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        skip = (int32_t) ceil( (float)(DurationInSeconds() - TimeSpan_hns::FromTicks(_smoothObject->GetCurrentPlayBackTime()).ToSeconds()) ); //ceil(double) does not work on Linux
        PKTEST_HRESULT_EXIT( _smoothObject->Skip( skip ));
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, STATECHANGE_TIMEOUT), "Timeout happened before Rendering or media Ended.");

        diff = (DurationInSeconds() - TimeSpan_hns::FromTicks(_smoothObject->GetCurrentPlayBackTime()).ToSeconds() );
        
        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(_smoothObject->GetManifest()->Duration(), _smoothObject->GetCurrentPlayBackTime(), _smoothObject->GetNonAccurateSeekThreshold()), "Skipped to end but playback time (%lld) does not match duration (%lld).  A difference of %.3f seconds.",_smoothObject->GetCurrentPlayBackTime(), _smoothObject->GetManifest()->Duration(), diff );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsMediaEnded(), "Media skipped to end but did not enter MediaEnded state");

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipToEndLive,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->SkipAndValidate((int32_t) TimeSpan_hns::FromTicks(_smoothObject->GetCurrentEndTime() - _smoothObject->GetCurrentPlayBackTime() ).ToSeconds()) );        
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsAtDVRWindowEnd(), "Skipped to end but playback time (%lld) is not at the live edge",_smoothObject->GetCurrentPlayBackTime() ,_smoothObject->GetManifest()->Duration());		

    exit:
        return;
    }


    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipPastEndOD,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_HRESULT_EXIT( _smoothObject->Skip(TOTAL_SECONDS_IN_A_DAY) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, STATECHANGE_TIMEOUT), "Timeout happened before Rendering or media Ended.");

        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(_smoothObject->GetManifest()->Duration() , _smoothObject->GetCurrentPlayBackTime(), TIMESTAMP_COMPARISON_THRESHOLD), "Seeked past end but playback time (%lld) does not match duration (%lld)",_smoothObject->GetCurrentPlayBackTime() ,_smoothObject->GetManifest()->Duration());
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsMediaEnded(), "Seeked past end but did not enter Media ended");

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipPastEndLive,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->SkipAndValidate(TOTAL_SECONDS_IN_A_DAY) );

        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsPlaying(), "Seeked past end but did not continue playing");

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipToStart,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );
        PKTEST_FUNC_EXIT( _smoothObject->SkipAndValidate(-1 * (int32_t)MillisecondsToSeconds(PLAYBACK_VALIDATION_DURATION)) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipToSamePositionRepeatedly,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            _smoothObject->Delay(SHORT_WAIT);
            PKTEST_FUNC_EXIT(_smoothObject->SkipAndValidate(-SHORT_WAIT));
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    //While paused

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipRepeatedlyWhenPaused,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; i++)
        {
            PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(SHORT_WAIT) );
            //Skip back and forth
            PKTEST_FUNC_EXIT(_smoothObject->SkipAndValidate( ((i % 2)?-1:1)* SHORT_SKIP));
            _smoothObject->Delay(VALIDATION_DELAY);
            PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Expected the player to be in playing after skipping in the paused state, but it is no longer playing" );
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipPastStartWhilePaused,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );
        //Skip past start
        PKTEST_FUNC_EXIT( _smoothObject->SkipAndValidate( -1 * (int32_t)DurationInSeconds()) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Expected the player to be in playing after skipping in the paused state, but it is no longer playing" );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipPastEndWhilePausedOD,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );
        //Skip past end
        PKTEST_HRESULT_EXIT( _smoothObject->Skip( TOTAL_SECONDS_IN_A_DAY ) );
        PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaRenderingAndEnded(false, STATECHANGE_TIMEOUT) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsMediaEnded(), "Expected the player to be in MediaEnded state after skipping beyond End in the paused state, but it is not");
        PKTEST_ASSERT_MSG_EXIT(CompareTimeStamps( _smoothObject->GetManifest()->Duration(), _smoothObject->GetCurrentPlayBackTime(), TIMESTAMP_COMPARISON_THRESHOLD ), "Skipped past end but playback time (%lld) does not match duration (%lld)",_smoothObject->GetCurrentPlayBackTime() ,_smoothObject->GetManifest()->Duration());

    exit:
        return;
    }

     ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipPastEndWhilePausedLive,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION) );
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );
        //Skip past end
        PKTEST_FUNC_EXIT( _smoothObject->SkipAndValidate( TOTAL_SECONDS_IN_A_DAY ) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Expected the player to be in playing after Skipping beyond End in the paused state, but it is no longer playing" );

    exit:
        return;
    }


    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipBackPastStartFromPause,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );
        //Skip past end
        PKTEST_FUNC_EXIT( _smoothObject->SkipAndValidate( -TOTAL_SECONDS_IN_A_DAY ) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Expected the player to be in playing after skipping in the paused state, but it is no longer playing" );

    exit:
        return;
    }


    ///////////////////////////////////////////////////////////////
    //Play Speed

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipBackwardRepeatedlyWhileInFastForward,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD_LONG
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        for(int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );
            PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(LOW_SPEED, VALIDATION_DELAY) );

            PKTEST_FUNC_EXIT( _smoothObject->SkipAndValidate(-1 * LARGE_SKIP) );
            _smoothObject->Delay(VALIDATION_DELAY);
            PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsInFastForward(), "Expected the object to be fastforward but it is not");
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipForwardRepeatedlyWhileInRewind,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        for(int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
            PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(LOW_SPEED, VALIDATION_DELAY) );

            PKTEST_FUNC_EXIT(_smoothObject->SkipAndValidate(SHORT_SKIP));
            _smoothObject->Delay(VALIDATION_DELAY);
            PKTEST_ASSERT_MSG_EXIT( !_smoothObject->IsPlaying() && _smoothObject->IsInRewind(), "Expected the object to be Rewind but it is not");
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipForwardRepeatedlyWhileInFastForward,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        //Move forward until the end
        for(int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );
            PKTEST_FUNC_EXIT( _smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY) );

            PKTEST_FUNC_EXIT(_smoothObject->SkipAndValidate(SHORT_SKIP));
            _smoothObject->Delay(VALIDATION_DELAY);
            PKTEST_ASSERT_MSG_EXIT( !_smoothObject->IsPlaying() && _smoothObject->IsInFastForward(), "Expected the object to be Fast forward but it is not");
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipBackwardRepeatedlyWhileInRewind,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        //Skip back until near the start
        for(int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
            PKTEST_FUNC_EXIT( _smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY) );

            PKTEST_FUNC_EXIT(_smoothObject->SkipAndValidate(-1 * SHORT_SKIP));
            _smoothObject->Delay(VALIDATION_DELAY);
            PKTEST_ASSERT_MSG_EXIT( !_smoothObject->IsPlaying() && _smoothObject->IsInRewind(), "Expected the object to be Rewind but it is not");
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SkipBackAndForwardRepeatedly,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));
        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(LONG_PLAYBACK_TIMEOUT) );

        for(int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT(_smoothObject->SkipAndValidate( ((i % 2)? -1 : 1) * SHORT_SKIP));
            _smoothObject->Delay(VALIDATION_DELAY);
            PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(false), "Expected to still be playing.");
        }

    exit:
        return;
    }

    ////////////////////////////////////////////////////////////////
    //Open Tests
    ////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_OpenWithoutAutoplayRepeatedlyFromFastForward,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );

            PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY));
            PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

            //Check that calling open worked
            PKTEST_ASSERT_MSG_EXIT(GetCurrentPlaybackTimeMilliseconds() == 0 && !_smoothObject->IsInFastForward(), "Expected that calling open would reset current playback time and take smooth transport out of Fast forward state, but it did not" );
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_OpenWithAutoPlayRepeatedlyFromFastForward,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));


        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForFastForward() );

            PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(LOW_SPEED, VALIDATION_DELAY));
            PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
            
            if(!_smoothObject->IsLive())
            {
                PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsAtDVRWindowStart(), "Expected that calling open would reset current playback time, but it did not");
            }
            PKTEST_ASSERT_MSG_EXIT(!_smoothObject->IsInFastForward(false), "Expected that calling open would stop fast forwarding, but it did not" );
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_OpenWithoutAutoPlayRepeatedlyFromRewind,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        
        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );

            //Rewind to Open
            PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY));
            PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );
            
            //Check that calling open worked
            PKTEST_ASSERT_MSG_EXIT(GetCurrentPlaybackTimeMilliseconds() == 0 && !_smoothObject->IsPlaying(),"Expected that calling open would reset currentPlayback time and stop playback, but it did not" );

        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_OpenWithAutoPlayRepeatedlyFromRewind,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT( _smoothObject->MakeRoomForRewind() );
            PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY));
            PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );
            PKTEST_ASSERT_MSG_EXIT(!_smoothObject->IsInRewind() && GetCurrentPlaybackTimeMilliseconds() < VALIDATION_DELAY, "Expected call to open to stop rewinding and reset media time but it did not");
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_OpenWithoutAutoPlayRepeatedlyFromPlay,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT(_smoothObject->PlayAndValidate(PLAYBACK_VALIDATION_DURATION));
            PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_OpenWithoutAutoPlayRepeatedlyFromPause,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        for (int32_t i = 0; i < REPEATED_ACTION_COUNT; ++i)
        {
            PKTEST_FUNC_EXIT(_smoothObject->PauseAndValidate(VALIDATION_DELAY));
            PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );
        }

    exit:
        return;
    }

    ////////////////////////////////////////////////////////////////
    //Miscellaneous Tests
    ////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_SlowMotionSpeeds,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t nearEnd;
        float step = .1f;
        float speed = 1.0f;

        //Start normally
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        _smoothObject->Delay(SHORT_WAIT);
        nearEnd = DurationInMilliseconds() - SHORT_PLAY_DURATION;

        //Slowly change speed
        while (speed > -1.0)
        {
            //Start over so that there is room to fast forward or rewind without hitting the end of the content
            if(!_smoothObject->GetManifest()->IsLive())
            {
                if(speed > 0 &&
                    GetCurrentPlaybackTimeMilliseconds() >= nearEnd)
                {
                    PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(TIMESCALE_10MHZ) );
                }
                else if (speed < 0 &&
                    GetCurrentPlaybackTimeMilliseconds() <= SHORT_PLAY_DURATION)
                {
                    PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(TIMESCALE_10MHZ) );
                }
            }
            //Set speed
            speed -= step;
            LogTestComment("Speed is set to %.2f", speed);
            
            if(speed == 0)
            {
                PKTEST_HRESULT_EXIT( _smoothObject->SlowMotion(speed));
                _smoothObject->Delay(VALIDATION_DELAY);
                PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPaused(), "Speed was set to 0 so the player should be paused, but is not");
            }
            else
            {
                PKTEST_ASSERT_MSG_EXIT( pkE_INVALIDARG == _smoothObject->SlowMotion(speed), "Expected attempt to play at slow motion speed to fail but it did not return the expected value" );
                _smoothObject->Delay(VALIDATION_DELAY);
                PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying() && !_smoothObject->IsInFastForward() && !_smoothObject->IsInRewind(), "Speed was set to a value between -1 and 1 so the player should not be in fast forward or rewind");
            }
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(PlayStateTests_FastForwardAndRewindLowSpeedsSmallStep,
        PKTEST_PROPERTY("Priority", "1")
        PLAYSTATE_ITERATIONS_LIVE_OD)
    {
        int64_t nearstart = 0, nearend=0;
        int64_t middleOfContent;
        float step = 1;
        float speed = 1;

        //Start normally
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));
        _smoothObject->Delay(SHORT_WAIT);


        //Slowly change speed
        for(int32_t i = 1; i < REPEATED_ACTION_COUNT_DOUBLE; ++i)
        {
            
            _smoothObject->GetCurrentPlayablePosition(&nearstart, &nearend);
            int64_t currentTime = _smoothObject->GetCurrentPlayBackTime(); 
            //Start over so that there is room to fast forward or rewind without hitting the end of the content
            if(currentTime >= nearend - LIVE_BACKOFF_TIME ||
               currentTime <= nearstart + LIVE_BACKOFF_TIME)
            {
                middleOfContent = ( nearstart / 2 ) + ( nearend / 2 );
                PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(middleOfContent) );
            }

            //Set speed
            speed += step;
            LogTestComment("Speed is set to %.2f", speed);
            //Either fast forward rewind
            if(i % 2)
            {
                PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(speed, VALIDATION_DELAY));
            }
            else
            {
                PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(speed, VALIDATION_DELAY));
            }
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        PlayStateTests_CurrentTimeUpdatedInStatusCallbacks,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault")
        )
    {
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT*2;
        vector<SmoothTransportStatus> statusVector;
        TimeSpan_NTP lastTimestamp;
        int32_t firstRenderingIndex = -1;
        double timestampDiff;
        int32_t statusVectorSize;

        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), true));

        //Normal playback
        PKTEST_FUNC_EXIT(_smoothObject->PlayAndValidate(playbackTime));
        //Check status updates for change in current time, which should be increasing because it should be playing
        statusVector =  _smoothObject->GetStatusCallbackVector();
        PKTEST_ASSERT_MSG_EXIT(statusVector.size() > 1, "Insufficient status callbacks for testing whether current time is updated in each callback.  Callback count = %d", statusVector.size());
        statusVectorSize = statusVector.size();

        //Find index of first rendering status callback.  After this one all of them should be increasing
        lastTimestamp = statusVector[statusVector.size() - 1]._currentTime;
        for(int i = 0; i < statusVectorSize; ++i)
        {
            if(statusVector[i]._update == SmoothTransportStatus_Rendering)
            {
                firstRenderingIndex = i;
                break;
            }
        }
        PKTEST_ASSERT_MSG_EXIT(firstRenderingIndex != -1, "There was no rendering callback");
        
        for(int32_t i = firstRenderingIndex; i < statusVectorSize - 1; ++i)
        {
            timestampDiff = statusVector[i + 1]._currentTime.ToSeconds() - statusVector[i]._currentTime.ToSeconds();
            PKTEST_ASSERT_MSG_EXIT(timestampDiff > 0, "Expected the later status to have a later currentTime, but it did not.  Earlier (%d) status timestamp: %.3f  Later (%d) status timestamp: %.3f  Diff: %.3f", i, statusVector[i]._currentTime.ToSeconds(), i + 1, statusVector[i + 1]._currentTime.ToSeconds(), timestampDiff);
        }

    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        PlayStateTests_StaysPausedDuringBPSChange,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        vector<SmoothTransportStatus> statusVector;
        uint32_t firstPausedCallback = 0;
        uint32_t prePauseBPSChangeCount = 0;
        bool shouldBePaused = false;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        prePauseBPSChangeCount = _smoothObject->GetStatusCallbackVector( SmoothTransportStatus_BitrateChanged).size();
        //Pause. There should be BPS changes, wait for them
        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(PLAYBACK_STARTUP_TIMEOUT * 2) );
        //_smoothObject->Delay(PLAYBACK_STARTUP_TIMEOUT * 2);
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->GetStatusCallbackVector( SmoothTransportStatus_BitrateChanged).size() > prePauseBPSChangeCount, "There were no BPS changes after Pause command");
        
        //Check status callbacks to make sure that the status stays paused after BPS changes
        statusVector =  _smoothObject->GetStatusCallbackVector();
        PKTEST_ASSERT_MSG_EXIT(statusVector.size() > 1, "Insufficient status callbacks for testing whether status was changed from Pause to Play by a BPS chane.  Callback count = %d", statusVector.size());
        
        for(uint32_t i = 0; i < statusVector.size(); ++i)
        {
            //Looking for first Paused status callback
            if(!shouldBePaused)
            {
                if(statusVector[i]._currentState == SmoothTransportTunerState_Paused)
                {
                    firstPausedCallback = i;
                    shouldBePaused = true;
                }
            }
            else
            //Making sure the state is 'Paused' on all the rest
            {
                PKTEST_ASSERT_MSG_EXIT(statusVector[i]._currentState == SmoothTransportTunerState_Paused, "All status callbacks after #%d should have a currentState of Paused, but #%d had %s", firstPausedCallback, i,_smoothObject->GetTunerStateName(statusVector[i]._currentState));
            }
        }

    exit:
        return;
    }

    PKTEST_METHOD_EX( PlayStateTests_Rebuffering,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalOutOfWndwEvents = 0;
        string profileString = "";
        int32_t bandWidthCap = 100; //Kbps

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        profileString = SSPKHelpers::GetCR_SetMaxNetworkCap(SSPKHelpers::GetLocalUrl(urlString), bandWidthCap);

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), profileString);

        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );
        _smoothObject->Delay(4 * VALIDATION_DELAY);
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( smoothStatusVector[i]._update == SmoothTransportStatus_Rebuffer )
            {
                 totalOutOfWndwEvents++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalOutOfWndwEvents > 0 , " We are expecting 1 OutOfWndw Callback, but there are %d events", totalOutOfWndwEvents );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 0, "We are not expecting any errors, but found %d errors in the list", smoothErrorVector.size() );
        

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        PlayStateTests_GetCurrentPlaybackTimeAlwaysIncreasesAfterStartingRendering,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE_OD
        )
    {
        int64_t currentPlaybackTime = 0, lastPlaybackTime = 0;
        
        PKTEST_FUNC_EXIT(StartSmoothObject(PKTest_GetTestData(), false));

        //Wait for rendering
        _smoothObject->Play();
        PKTEST_ASSERT_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT));

        _smoothObject->Delay( HALF_SECOND_IN_MILLISECONDS );

        //Start checking the playback time
        for(int32_t i = 0; i < REPEATED_ACTION_COUNT_MEDIUM; i++)
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

    PKTEST_METHOD_EX(
        PlayStateTests_CloseImmediatelyAfterMediaOpen,
        PKTEST_PROPERTY("Priority", "2")
        PLAYSTATE_ITERATIONS_LIVE_OD      
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_HRESULT_EXIT(_smoothObject->OpenVideo(urlString, false));
        PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(0));
    
    exit:
        return;
    }
};
