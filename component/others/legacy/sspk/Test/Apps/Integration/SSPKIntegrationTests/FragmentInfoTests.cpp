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
#include <math.h>

using namespace SSPK;
using namespace SSPKTest;

PKTEST_GROUP( FragmentInfoTests )
{
    static const uint32_t FRAG_INFO_COUNT_TOLERANCE = 3;
    static const uint32_t CALLBACK_COUNT_TOLERANCE = 3;

    typedef std::map<int32_t,int32_t> MapOfChunkInfoDiffs;

    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;

    bool CompareCallbackCount(int32_t actual, float expected)
    {
        int32_t minExpected = (int32_t) max((double)expected - CALLBACK_COUNT_TOLERANCE, 0.0);
        int32_t maxExpected = (int32_t) ceil(expected + CALLBACK_COUNT_TOLERANCE);

        if(( minExpected > actual || 
            maxExpected < actual))
            {
                LogTestComment("Expected between %d and %d callbacks, but found %d", minExpected, maxExpected, actual);
                return false;
            }
        else
        {
            LogTestComment("Expected between %d and %d callbacks, and found %d", minExpected, maxExpected, actual);
        }
        return true;
    }

    float ExpectedStartEndCallbackFrequency()
    {
        return  (float)TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks(_smoothObject->GetNonAccurateSeekThreshold() / 2)).Ticks();  //NonAccurateSeekThreshold is double the duration of a chunk
    }

    bool SwitchStreams()
    {
        VectorOfStreams selectedStreams, availableStreams, proposedSelection;
        pkRESULT pkr;

        _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams);
        _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams);
        proposedSelection = SSPKHelpers::SelectNextAudioStream(availableStreams, selectedStreams);

        pkr = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (pkSUCCEEDED(pkr) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        return true;
    exit:
        return false;
    }

    MapOfChunkInfoDiffs CompareCurrentChunkInfoToSnapshot( MapOfChunkInfoVectors& snapshot)
    {
        MapOfChunkInfoVectors current = _smoothObject->GetFragInfos();
        MapOfChunkInfoDiffs diff;

        //Calculate the number of fragInfos that are part of the current one but not the snapshot
        for(MapOfChunkInfoVectors::const_iterator cur = current.begin(); cur != current.end(); ++cur)
        {
            int32_t streamID = cur->first;
            int32_t countDiff = cur->second.size() - snapshot[streamID].size();
            diff[streamID] = countDiff;
        }
        return diff;
    }

    bool ValidateChunkInfoDiff(MapOfChunkInfoDiffs& diff, uint64_t elapsedMilliseconds, vector<int32_t> selectedStreamIDs, bool fragInfoForSelectedStreams)
    {
        
        double elapsedSeconds = TimeSpan_ms::FromTicks(elapsedMilliseconds).ToSeconds();
        //The name of the stream may be more useful than the id, which is not actually a part of the stream object.
        VectorOfStreams availableStreams;
        _smoothObject->GetManifest()->GetAvailableStreams( &availableStreams );

        for(MapOfChunkInfoDiffs::const_iterator cur = diff.begin(); cur != diff.end(); ++cur)
        {
            int32_t streamID = cur->first;
            bool checkCount = false;
            if(selectedStreamIDs.end() != find(selectedStreamIDs.begin(), selectedStreamIDs.end(), streamID))
            {
                //It was selected, should not have gotten any fragInfos
                if(fragInfoForSelectedStreams)
                {
                    checkCount = true;
                }
                else
                {
                    if(0 != diff[streamID])
                    {
                        LogTestComment("Expected no fragmentInfos for the selected stream with ID %d (%ls), but found %d fragmentInfos for %.3f seconds", streamID, availableStreams[streamID - 1]->Name().c_str(), diff[streamID], elapsedSeconds);
                        return false;
                    }
                    else
                    {
                        LogTestComment("Expected no fragmentInfos for the selected stream with ID %d (%ls) and found none", streamID, availableStreams[streamID - 1]->Name().c_str());
                    }
                }
            }
            else
            {
                //Check for whether this is a child stream.  If it is, then it will have no fragInfos. 
                IManifestStream* parentStream;
                availableStreams[streamID - 1]->GetParentStream(&parentStream);
                if(parentStream)
                {
                    if(0 != diff[streamID])
                    {
                        LogTestComment("Expected no fragmentInfos for the stream with ID %d (%ls), as it has a parent stream, but found %d fragmentInfos for %.3f seconds", streamID, availableStreams[streamID - 1]->Name().c_str(), diff[streamID], elapsedSeconds);
                        return false;
                    }
                }
                else
                {
                    checkCount = true;
                }
            }
            //Make sure the correct number of fragInfos were received
            if(checkCount)
            {
                //Text streams without parents behave differently, just figure out whether there are more fragInfos or not.
                if(MediaStreamTypeText == availableStreams[streamID - 1]->Type())
                {
                    //0 or close to it
                    if(elapsedSeconds == 0.0 ||
                        fabs(elapsedSeconds) < SECONDS_EPSILON)
                    {
                        if(diff[streamID] != 0)
                        {
                            LogTestComment("Expected no more fragmentInfos for the stream with ID %d (%ls) but found %d for %.3f seconds", streamID, availableStreams[streamID - 1]->Name().c_str(), diff[streamID], elapsedSeconds);
                            return false;
                        }
                    }
                    else
                    {
                        if(diff[streamID] <= 0)
                        {
                            LogTestComment("Expected an increase in the number of fragmentInfos for the stream with ID %d (%ls) but found none for %.3f seconds", streamID, availableStreams[streamID - 1]->Name().c_str(), elapsedSeconds);
                            return false;
                        }
                    }
                }
                else
                //Normal stream
                {
                    int32_t expectedFragInfoCount, minExpected, maxExpected;
                    expectedFragInfoCount = (int32_t)ceil((float)(elapsedSeconds / TimeSpan_hns::FromTicks(_smoothObject->GetNonAccurateSeekThreshold() / 2).ToSeconds() ));//cast to float to avoid error on Linux.  Also NonAccurateSeekThreshold is double the duration of a chunk
                    
                    minExpected = (int32_t) max( (double)expectedFragInfoCount - FRAG_INFO_COUNT_TOLERANCE, 0.0);
                    maxExpected = expectedFragInfoCount + FRAG_INFO_COUNT_TOLERANCE;

                     if(( minExpected > diff[streamID] || 
                            maxExpected < diff[streamID]))
                    {
                        LogTestComment("Expected between %d and %d fragmentInfos for the stream with ID %d (%ls), but found %d fragmentInfos for %.3f seconds", minExpected, maxExpected, streamID, availableStreams[streamID - 1]->Name().c_str(), diff[streamID], elapsedSeconds);
                        return false;
                    }
                    else
                    {
                        LogTestComment("Expected between %d and %d fragmentInfos for the stream with ID %d (%ls) and found %d fragmentInfos for %.3f seconds", minExpected, maxExpected, streamID, availableStreams[streamID - 1]->Name().c_str(), diff[streamID], elapsedSeconds);
                    }
                }
            }
        }
        return true;
    }

    vector<int32_t> GetSelectedStreamIDs()
    {
        VectorOfStreams selectedStreams;
        VectorOfStreams availableStreams;
        vector<int32_t> selectedStreamIDs;

        PKTEST_HRESULT_EXIT(_smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams));
        PKTEST_HRESULT_EXIT(_smoothObject->GetManifest()->GetAvailableStreams(&availableStreams)); //Used to get the position of the streams to figure out the IDs

        for(size_t i=0; i < selectedStreams.size(); ++i)
        {
            int32_t streamID = 0;
            //A stream's ID specified in the diag event is its position in the available streams + 1
            VectorOfStreams::const_iterator streamInAvailableVector = find(availableStreams.begin(), availableStreams.end(), selectedStreams[i]);
            if(availableStreams.end() == streamInAvailableVector)
            {
                PKTEST_ASSERT_MSG_EXIT(false, "A selected stream, numbered %d in the list of selected streams, was not in the list of available streams", i);
            }

            streamID = (int32_t)(streamInAvailableVector - availableStreams.begin()) + 1;
            LogTestComment("Stream with ID %d is selected", streamID);

            selectedStreamIDs.push_back(streamID);
        }
        exit:
        return selectedStreamIDs;
    }

    FragmentInfoTests()
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

        if(_smoothObject != NULL)
        {
            _smoothObject->Close();
            delete _smoothObject;
        }
        return true;
    }
    
    bool StartSmoothObject(string url)
    {
        //Filter by channel
        vector<DiagsChannel> channelList;
        channelList.push_back(kDiagChannel_Manifest);
        channelList.push_back(kDiagChannel_FragInfo);
        //Register for events (manifest parsing and fraginfo)
        _smoothObject->RegisterDiagEvents(channelList, kDiagsPriority_Medium);
        
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(url, false));

        PKTEST_HRESULT_EXIT(_smoothObject->Play());

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");

        return true;
    exit:
        return false;
    }

    //Tests
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_FragInfoBasic,
        PKTEST_PROPERTY("Priority", "0")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        PKTEST_PROPERTY("Data:VC1LiveMultiAudio", "VC1LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        //Calculate and validate differences
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);
        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY) / (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, false) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_NotAtLive,
        PKTEST_PROPERTY("Priority", "0")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;

        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Wait so that it is no longer at live and play
        PKTEST_HRESULT_EXIT( _smoothObject->Pause(VALIDATION_DELAY) );
        cumulativeTime += VALIDATION_DELAY;
        PKTEST_HRESULT_EXIT( _smoothObject->Play() );
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        //Calculate and validate differences
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);
        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, true) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_FastForward,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;

        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);
        PKTEST_FUNC_EXIT(_smoothObject->MakeRoomForFastForward());

        PKTEST_HRESULT_EXIT( _smoothObject->FastForward(DEFAULT_TRICK_PLAY_SPEED) );
        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        diff = CompareCurrentChunkInfoToSnapshot(snapshot);
        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, true) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_Rewind,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;

        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        PKTEST_HRESULT_EXIT( _smoothObject->Rewind(DEFAULT_TRICK_PLAY_SPEED) );
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        diff = CompareCurrentChunkInfoToSnapshot(snapshot);
        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, true) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_MediaEnded,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        float trickPlaySpeed = DEFAULT_TRICK_PLAY_SPEED * 10;
        int32_t waitTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Get media ended then check for fragInfos
        PKTEST_HRESULT_EXIT( _smoothObject->Rewind(trickPlaySpeed) );
        PKTEST_FUNC_EXIT( _smoothObject->WaitForMediaEnded( STATECHANGE_TIMEOUT ));

        PKTEST_ASSERT_EXIT( _smoothObject->GetStatusUpdatesList().MediaEnded > 0);
        snapshot = _smoothObject->GetFragInfos();
        
        _smoothObject->Delay(waitTime);
        cumulativeTime += waitTime;

        diff = CompareCurrentChunkInfoToSnapshot(snapshot);
        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY ) / (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, waitTime, selectedStreamIDs, true) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///Switch streams
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_SwitchStreamsAtLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT( StartSmoothObject(urlString) );
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Normal playback
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        //Switch stream during playback
        snapshot = _smoothObject->GetFragInfos();
        SwitchStreams();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        diff = CompareCurrentChunkInfoToSnapshot(snapshot);
        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY ) / (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, false) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_SwitchStreamsNotAtLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT( StartSmoothObject(urlString) );
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Normal playback (not at live)
        PKTEST_HRESULT_EXIT( _smoothObject->Pause(playbackTime * 2) );
        cumulativeTime += playbackTime * 2;
        PKTEST_HRESULT_EXIT( _smoothObject->Play() );
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        PKTEST_ASSERT_EXIT(!_smoothObject->IsAtDVRWindowEnd());

        //Switch stream during playback
        snapshot = _smoothObject->GetFragInfos();
        SwitchStreams();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);

        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY * 2)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, true) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_SwitchStreamsWhilePaused,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        PKTEST_PROPERTY("Bug", "31813")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT( StartSmoothObject(urlString) );
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Normal playback to paused
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        PKTEST_HRESULT_EXIT( _smoothObject->Pause(VALIDATION_DELAY) );
        cumulativeTime += VALIDATION_DELAY;
        SwitchStreams();

        //Start playing again (at live)
        PKTEST_HRESULT_EXIT( _smoothObject->Play() );
        
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(TOTAL_SECONDS_IN_A_DAY));
        _smoothObject->Delay(VALIDATION_DELAY * 2);
        cumulativeTime += VALIDATION_DELAY;
        PKTEST_ASSERT_EXIT(_smoothObject->IsAtDVRWindowEnd() );

        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);

        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, false) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///Seek
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_SeekLiveToNonLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Normal playback
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        //Seek to some non-live time
        PKTEST_HRESULT_EXIT(_smoothObject->Seek(_smoothObject->GetCurrentStartTime()));
        _smoothObject->Delay(2 * VALIDATION_DELAY);
        cumulativeTime += 2 * VALIDATION_DELAY;

        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);

        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, true) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_SeekNonLiveToLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Normal playback at non live
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(-RAND_SKIP_SECONDS));
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        //Seek to live time
        PKTEST_HRESULT_EXIT(_smoothObject->Seek(_smoothObject->GetCurrentEndTime()));
        _smoothObject->Delay(VALIDATION_DELAY);
        cumulativeTime += VALIDATION_DELAY;

        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);

        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        //removing the 5 seconds in which fraginfo thread is not started when seeked to live.
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, false) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_SeekNonLiveToNonLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Normal playback at non live
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(-RAND_SKIP_SECONDS));
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        //Seek to another non live time
        PKTEST_HRESULT_EXIT(_smoothObject->Seek( (_smoothObject->GetCurrentPlayBackTime() - DEFAULT_SKIP_10MHZ ) ));
        _smoothObject->Delay(VALIDATION_DELAY*2);

        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);

        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, true) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///PlayAt
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_PlayAtLiveToNonLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Normal playback
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        //Play at some non-live time
        PKTEST_FUNC_EXIT(_smoothObject->PlayAtAndValidate(_smoothObject->GetCurrentStartTime()));
        _smoothObject->Delay(VALIDATION_DELAY);
        cumulativeTime += VALIDATION_DELAY;

        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);

        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, true) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_PlayAtNonLiveToLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Normal playback at non live
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(-RAND_SKIP_SECONDS));
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        //PlayAt live time
        PKTEST_HRESULT_EXIT(_smoothObject->PlayAt(_smoothObject->GetCurrentEndTime()));
        _smoothObject->Delay(VALIDATION_DELAY * 2);
        cumulativeTime += VALIDATION_DELAY;

        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);

        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, false) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

        ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_PlayAtNonLiveToNonLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Normal playback at non live
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(-RAND_SKIP_SECONDS));
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        //Play at another non live time
        PKTEST_HRESULT_EXIT(_smoothObject->PlayAt( (_smoothObject->GetCurrentPlayBackTime() - DEFAULT_SKIP_10MHZ ) ));
        _smoothObject->Delay(VALIDATION_DELAY * 2);

        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);

        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, true) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///Skip
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_SkipLiveToNonLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Normal playback
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        //Seek to some non-live time
        PKTEST_HRESULT_EXIT(_smoothObject->Skip( (int32_t)(-_smoothObject->GetManifest()->DVRWindowLength() / 2)));
        _smoothObject->Delay(VALIDATION_DELAY);
        cumulativeTime += VALIDATION_DELAY;

        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);

        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, true) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_SkipNonLiveToLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);
        //Normal playback at non live
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(-RAND_SKIP_SECONDS));
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        //Skip to live time
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(TOTAL_SECONDS_IN_A_DAY));
        _smoothObject->Delay(VALIDATION_DELAY);
        cumulativeTime += VALIDATION_DELAY;

        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);

        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        //removing the 5 seconds in which fraginfo thread is not started when skipped to live.
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, false) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_SkipNonLiveToNonLive,
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Normal playback at non live
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(-RAND_SKIP_SECONDS));
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        //Seek to another non live time
        PKTEST_HRESULT_EXIT(_smoothObject->Skip( -RAND_SKIP_SECONDS ));
        _smoothObject->Delay(VALIDATION_DELAY * 2);

        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);

        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime )/ (ExpectedStartEndCallbackFrequency());

        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, playbackTime, selectedStreamIDs, true) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///After close
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_FragInfoStopAfterClose,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Data:H264LiveMultiAudio", "H264LiveMultiAudio")
        )
    {
        MapOfChunkInfoVectors snapshot;
        MapOfChunkInfoDiffs diff;
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, cumulativeTime = 0;
        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;
        PKTEST_HRESULT_EXIT( _smoothObject->Close() );
        _smoothObject->Delay(VALIDATION_DELAY);
        snapshot = _smoothObject->GetFragInfos();
        _smoothObject->Delay(playbackTime);

        //Calculate and validate differences
        diff = CompareCurrentChunkInfoToSnapshot(snapshot);
        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY) / (ExpectedStartEndCallbackFrequency());
        
        //There should be no difference, so this should pass with a playback time of 0
        PKTEST_FUNC_EXIT( selectedStreamIDs = GetSelectedStreamIDs() );
        PKTEST_ASSERT_EXIT( ValidateChunkInfoDiff(diff, 0, selectedStreamIDs, false) );
        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }

    ///StartEndCallbacks
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        FragmentInfoTests_GetStartEndCallbacksInPause,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault")
        )
    {
        int32_t playbackTime = PLAYBACK_STARTUP_TIMEOUT, pauseTime = 20 * MILLISECONDS_PER_SECOND, cumulativeTime = 0;

        int32_t startEndCountDiff, startEndCount;
        float expectedStartEndCountDiff;
        vector<int32_t> selectedStreamIDs;
        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str()));

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        startEndCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime);

        //Normal playback 
        _smoothObject->Delay(playbackTime);
        cumulativeTime += playbackTime;

        //Pause
        PKTEST_HRESULT_EXIT(_smoothObject->Pause( pauseTime ));
        cumulativeTime += pauseTime;

        startEndCountDiff = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_StartEndTime) - startEndCount;
        expectedStartEndCountDiff = (cumulativeTime - VALIDATION_DELAY)/ (ExpectedStartEndCallbackFrequency());

        PKTEST_ASSERT_MSG_EXIT( CompareCallbackCount(startEndCountDiff, expectedStartEndCountDiff), "The number of StartEnd status callbacks, %d does not match the expected %d", startEndCountDiff, (int32_t)expectedStartEndCountDiff );
    exit:
        return;
    }
};
