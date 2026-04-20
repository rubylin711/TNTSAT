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
#include "ISmoothTransport.h"
#include "ManifestReadyCallbackImpl.h"
#include "StringUtils.h"
#include "SSPKHelpers.h"
#include "STUtilities.h"
#include <sstream>
#include <algorithm>

using namespace SSPK;
using namespace SSPKTest;


PKTEST_GROUP( LiveToVodTests )
{
    static const int32_t MINUTES_TO_MILLISECONDS                = SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
    static const int32_t NUMBER_OF_SEGMENTS_LONG                = 8;
    static const int32_t NUMBER_OF_SEGMENTS_SHORT               = 1;

    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;
    int32_t _segmentCount;
    int32_t _segmentSize;

    LiveToVodTests()
    {
        CXMLElement xTestData = PKTest_GetDataTable( "IntegrationTestSources" );
        if( !xTestData.IsNull() )
        {
             _xManifests = xTestData.Elements(L"source");
        }
    }

    bool TestSetup()
    {
        _segmentCount = 0;
        _segmentSize = 0;

        PKTEST_ASSERT_MSG_EXIT(_xManifests.Length() > 0,"Couldn't find any elements with name 'source'");

        _smoothObject = NEW_NO_THROW STWrapper();
        PKTEST_ASSERT_EXIT(NULL != _smoothObject);

        return true;

    exit:
        return false;
    }

    bool TestCleanup()
    {
        if(!_smoothObject->IsClosed() )
        {
            _smoothObject->Close();
        }
        delete _smoothObject;
        return true;
    }

    bool StartSmoothObjectAndValidate( string sourceName, bool autoPlay, bool isLiveToVod = true)
    {
        string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(sourceName).c_str());
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, autoPlay));

        if(isLiveToVod)
        {
            GetSegmentInfo(sourceName);
        }

        return true;
    exit:
        return false;
    }

    void GetSegmentInfo(string urlSourceString)
    {
        _segmentCount = toInt(SSPKHelpers::GetAttributeValueFromList( _xManifests, Str2WStr(urlSourceString).c_str(), L"segmentCount" ));
        _segmentSize = toInt(SSPKHelpers::GetAttributeValueFromList( _xManifests, Str2WStr(urlSourceString).c_str(), L"segmentLength" )) * MINUTES_TO_MILLISECONDS; // in Milli seconds
    }

    /////////////////// Live To Vod E2E Tests /////////////////////
    ///////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_E2E_Basic, 
        PKTEST_PROPERTY("Priority", "0") )
    {
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate("LiveToVodMultiAudio", true) );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetManifest()->Type() == ManifestType_Segmented, "This should be a Segmented manifest");
        
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_E2E_InitialDVRWindow, 
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodMultiAudio";
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        PKTEST_ASSERT_MSG_EXIT(( _smoothObject->GetCurrentEndTime() - _smoothObject->GetCurrentStartTime() <= TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(_segmentSize * _segmentCount)).Ticks()), "The initial DVR window is larger than 2 segments.");

        _smoothObject->Delay(_segmentCount * _segmentSize);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_E2E_CrossBoundary, 
        PKTEST_PROPERTY("Priority", "0"))
    {
        string urlSourceString = "LiveToVodMultiAudio";
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        _smoothObject->Delay(_segmentCount * _segmentSize);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_E2E_FragmentInfo, 
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodMultiAudio";
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_HRESULT_EXIT(_smoothObject->Pause(VALIDATION_DELAY));
        PKTEST_HRESULT_EXIT( _smoothObject->Play());
        _smoothObject->Delay(_segmentCount * _segmentSize);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        LiveToVod_E2E_PauseForMultipleSegments, 
        PKTEST_PROPERTY("Priority", "0")
        PKTEST_PROPERTY( "Bug", "27511" )
        )
    {
        string urlSourceString = "LiveToVodMultiAudioWithClientCachingEnabled";

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_HRESULT_EXIT(_smoothObject->Pause((_segmentCount + 1) * _segmentSize));
        PKTEST_HRESULT_EXIT( _smoothObject->Play());
        _smoothObject->Delay(_segmentCount * VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_E2E_Rewind, 
        PKTEST_PROPERTY("Priority", "0"))
    {
        string urlSourceString = "LiveToVodMultiAudio";
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        _smoothObject->Delay((_segmentCount * 2) * _segmentSize);
        PKTEST_HRESULT_EXIT( _smoothObject->Rewind() );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsInRewind(), "SSPK should be in rewind state.");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_E2E_FastForward, 
        PKTEST_PROPERTY("Priority", "0"))
    {
        string urlSourceString = "LiveToVodMultiAudio";
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        _smoothObject->Delay((_segmentCount + 1 ) * _segmentSize);
        PKTEST_HRESULT_EXIT(_smoothObject->Skip((-(_segmentCount * 3)) * _segmentSize)); /// skip to go beyond the current segments.
        PKTEST_HRESULT_EXIT( _smoothObject->FastForward() );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsInFastForward(), "SSPK should be in FastForward state.");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        LiveToVod_E2E_PauseMultiSegmentsToLivePosition, 
        PKTEST_PROPERTY("Priority", "1")
        PKTEST_PROPERTY( "Bug", "27511" )
        )
    {
        string urlSourceString = "LiveToVodMultiAudio";

        int64_t livePositionBeforePause = -1;
        int64_t livePositionAfterSkiptoLive = -1;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        livePositionBeforePause = _smoothObject->GetCurrentPlayBackTime();
        PKTEST_HRESULT_EXIT(_smoothObject->Pause((_segmentCount + 1) * _segmentSize));
        // Go to live.
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(10 * _segmentSize));
        PKTEST_HRESULT_EXIT( _smoothObject->Play());
        _smoothObject->Delay(VALIDATION_DELAY);
        livePositionAfterSkiptoLive = _smoothObject->GetCurrentPlayBackTime();
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");
        // all the timestamps are below are converted from milli seconds to 10mhz.
        PKTEST_ASSERT_MSG_EXIT( (livePositionAfterSkiptoLive > ((_segmentCount + 1) * _segmentSize * 10000) + livePositionBeforePause - (VALIDATION_DELAY * 2* 10000)) && (livePositionAfterSkiptoLive < ((_segmentCount + 1) * _segmentSize* 10000) + livePositionBeforePause + (VALIDATION_DELAY * 2* 10000))
            , "new live position should be approximately near pauseTime + old live position =  %lld but is at %lld",( ((_segmentCount + 1) * _segmentSize* 10000) + livePositionBeforePause ), livePositionAfterSkiptoLive );

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_E2E_MultiSegmentsCrossBoundary, 
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodMultiSegments";
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        _smoothObject->Delay(_segmentCount * _segmentSize);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_E2E_EmptySparseChunkList_CrossBoundary, 
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVod2MinSegEmptySparseChunkList";

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        _smoothObject->Delay(_segmentCount * _segmentSize);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_ManifestParsing_VersionLessThan30, 
        PKTEST_PROPERTY("Priority", "2"))
    {
        vector<SmoothTransportError> smoothErrorVector;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveToVodVersionLessThan30") );
        PKTEST_HRESULT_EXIT(_smoothObject->OpenVideo(urlString, true, false));
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT), "Timeout occured waiting for failure or end");

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found multiple or no errors in the list" );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ManifestParseFailed, "ManifestParsingFailed error expected when version number is less than 3.0 for livetovod, but observed %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode) );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_ManifestParsing_NoStartTime, 
        PKTEST_PROPERTY("Priority", "1"))
    {
        vector<SmoothTransportError> smoothErrorVector;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveToVodNoStartTime") );
        PKTEST_HRESULT_EXIT(_smoothObject->OpenVideo(urlString, true, false));
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT), "Timeout occured waiting for failure or end");

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found multiple or no errors in the list" );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ManifestParseFailed, "ManifestParsingFailed error expected when there is no segment starttime , but not observed some other error." );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX(LiveToVod_ManifestParsing_NoSegmentLength,
        PKTEST_PROPERTY("Priority", "1"))
    {
        vector<SmoothTransportError> smoothErrorVector;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, L"LiveToVodNoSegmentLength") );
        PKTEST_HRESULT_EXIT(_smoothObject->OpenVideo(urlString, true, false));
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT), "Timeout occured waiting for failure or end" );

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found multiple or no errors in the list" );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ManifestParseFailed, "ManifestParsingFailed error expected when there is no segment length, but not observed some other error." );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_ManifestParsing_NoSegmentUrl,
        PKTEST_PROPERTY("Priority", "2"))
    {
        vector<SmoothTransportError> smoothErrorVector;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveToVodNoSegmentUrl") );
        PKTEST_HRESULT_EXIT(_smoothObject->OpenVideo(urlString, true, false));
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT), "Timeout occured waiting for failure or end");

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found multiple or no errors in the list" );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ManifestParseFailed, "ManifestParsingFailed error expected when Segment Url is missing from Manifest, but not observed some other error." );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_ManifestParsing_EmptySegmentAttributes,
        PKTEST_PROPERTY("Priority", "2"))
    {
        vector<SmoothTransportError> smoothErrorVector;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveToVodEmptySegmentAttributes") );
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false, false));

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_ManifestParsing_MissingTrackAndAttributes, 
        PKTEST_PROPERTY("Priority", "1") )
    {
        _smoothObject->SetManifestReadyMethod( this, &LiveToVodTests::LiveToVod_ManifestParsing_MissingTrackAndAttributes_Method );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveToVodMissingTracksAndAttributes") );
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString,false));

    exit:
        return;
    }
    void LiveToVod_ManifestParsing_MissingTrackAndAttributes_Method(IManifest* pManifest, pkRESULT hr)
    {
        // This changes when the source changes
        size_t expectedVideoTrackCount = 5;

        VectorOfStreams availableStreams;
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );

        for (uint32_t i = 0; i < availableStreams.size(); i++)
        {
            if( availableStreams[i]->Type() == MediaStreamTypeVideo )
            {
                std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
                availableStreams[i]->GetAvailableTracks( &availableTracks );

                PKTEST_ASSERT_MSG_EXIT(availableTracks.size() == expectedVideoTrackCount, "Track count is different(%d) than expected(%d)", availableTracks.size(), expectedVideoTrackCount);

                break;
            }
        }

    exit:
        return ;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        LiveToVod_ManifestParsing_MultiSegmentChunkListMerging, 
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        CHUNK_INFO chunkInfo;
        
        string urlSourceString = "LiveToVodMultiSegments";

        // These values below are assumed values. If content changed, these numbers might change.
        int32_t chunksPerSegmentApprox = 30;
        int32_t chunksPerSegmentError = 5;
        IManifestStream* parentStream ;
        VectorOfStreams availableStreams;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );

        for(uint32_t i = 0 ; i < availableStreams.size() ; i++)
        {
            int32_t chunkCount = 0;

            ChunkIterator itChunk = availableStreams[i]->GetFirstInCurrentChunkList();

            while( SUCCEEDED( hr = availableStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
                chunkCount++;
            }
            availableStreams[i]->GetParentStream(&parentStream);

            if(!parentStream)
            {
                PKTEST_ASSERT_MSG_EXIT(chunkCount > _segmentCount * (chunksPerSegmentApprox - chunksPerSegmentError) && chunkCount < _segmentCount * (chunksPerSegmentApprox + chunksPerSegmentError), "For stream : %ls , a 2 min segment each should have chunks around 30, but the Total count for %d segments is %d",availableStreams[i]->Name().c_str(), _segmentCount, chunkCount);
            }
        }
    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_AvailableStreams_Basic_DuringManifestReady, 
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod( this, &LiveToVodTests::LiveToVod_AvailableStreams_Basic_DuringManifestReady_Method );

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate("LiveToVodDefault", true) );

    exit:
        return;
    }
    void LiveToVod_AvailableStreams_Basic_DuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        PKTEST_FUNC_EXIT(CheckAvailableStreamCountByType(2,1,1) );

    exit:
        return ;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_SelectStreams_ToggleAudio_DuringManifestReady, 
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod(this, &LiveToVodTests::LiveToVod_SelectStreams_ToggleAudio_DuringManifestReady_Method);
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate("LiveToVodDefault", true) );

    exit:
        return;
    }
    void LiveToVod_SelectStreams_ToggleAudio_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
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
    PKTEST_METHOD_EX( LiveToVod_SelectStreams_ToggleAudio_AfterManifestReady, 
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;
        string urlSourceString = "LiveToVodMultiAudio";
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

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
        _smoothObject->Delay(_segmentCount * _segmentSize);// Continue playing cross boundary
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing after the stream selection during playback.");

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_SelectStreams_ToggleAudio_WhenInDVR, 
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;
        string urlSourceString = "LiveToVodMultiAudio";
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        _smoothObject->Delay((_segmentCount + 1) * _segmentSize);
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(-10 * _segmentSize));// Skipping to begin position (DVR))
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout occured waiting for rendering");

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
        _smoothObject->Delay(_segmentCount * _segmentSize);// Continue playing cross segment boundary.
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing after the stream selection during playback.");

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( LiveToVod_SelectStreams_ToggleAudio_ThenSkipBackToDVR, 
        PKTEST_PROPERTY("Priority", "1"))
    {
        VectorOfStreams proposedSelection;
        HRESULT hr2;
        string urlSourceString = "LiveToVodMultiAudio";
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        _smoothObject->Delay((_segmentCount + 1) * _segmentSize);

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

        PKTEST_HRESULT_EXIT(_smoothObject->Skip(-10 * _segmentSize));// Skipping to begin position (DVR))

        _smoothObject->Delay(_segmentCount * _segmentSize); // Continue playing
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing after the stream selection during playback.");

    exit:
        return;
    }


    //////////////StartOver Scenarios///////////////////
    ////////////////////////////////////////////////////
    /// <summary>
    /// During playback, Call SetPlaybackRange into past segments. Once received the callback, and seek into past segment
    /// </summary>
    /// <priority value=0 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_Basic,
        PKTEST_PROPERTY("Priority", "0"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t oldStartTime = 0, proposedStartTime = 0, newStartTime = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));
        
        newStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(STUtilities::Rand(newStartTime, oldStartTime)) );

    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments. Check we don't move the start Position till we get the callback
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_CheckStartTime,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t oldStartTime = 0, proposedStartTime = 0, startTime = 0;
        vector<SmoothTransportStatus> statusVector; 
        bool newStartTimeFound = false;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        oldStartTime = _smoothObject->GetCurrentStartTime();

        _smoothObject->Delay(VALIDATION_DELAY);

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        _smoothObject->Delay(VALIDATION_DELAY);

        statusVector = _smoothObject->GetStatusCallbackVector();

        startTime = oldStartTime;
        for (uint32_t i = 0; i < statusVector.size(); i++)
        {
            if(!CompareTimeStamps(startTime, TimeSpan_hns::ConvertFrom(statusVector[i]._startTime).Ticks(), TIMESTAMP_COMPARISON_THRESHOLD))
            {
                if(newStartTimeFound)
                {
                    PKTEST_ASSERT_MSG_EXIT(false, "The start Time should move only once.");
                }
                startTime = TimeSpan_hns::ConvertFrom(statusVector[i]._startTime).Ticks();
                newStartTimeFound = true;
            }
        }

    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments during Manifest Ready
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod( this, &LiveToVodTests::LiveToVod_SetPlaybackRange_DuringManifestReady_Method );

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate("LiveToVodMultiAudio", true) );
        
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(STUtilities::Rand(_smoothObject->GetCurrentStartTime(), _smoothObject->GetCurrentEndTime())) );

    exit:
        return;
    }
    void LiveToVod_SetPlaybackRange_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        int64_t oldStartTime = 0, proposedStartTime = 0;

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

    exit:
        return ;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments after Manifest Ready
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t oldStartTime = 0, proposedStartTime = 0;
                
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));
        
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(STUtilities::Rand(_smoothObject->GetCurrentStartTime(), _smoothObject->GetCurrentEndTime())) );
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments multiple times once the previous one finishes. make sure the last one is still honored.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_MultipleCalls,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t oldStartTime = 0, proposedStartTime = 0, newStartTime = 0;
        int32_t numberOfSegments = 10;
                
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(numberOfSegments * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, numberOfSegments * STATECHANGE_TIMEOUT));

        numberOfSegments = 5;
        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime + TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(numberOfSegments * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, numberOfSegments * STATECHANGE_TIMEOUT));

        newStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(STUtilities::Rand(newStartTime, oldStartTime)) );
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, which is less than 10800 chunks(6 hours). Once received the callback, and seek into past segment
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_LessThan10800chunks,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t oldStartTime = 0, proposedStartTime = 0, newStartTime = 0;
        int32_t chunklistDuration = 6 * 60 * SECONDS_PER_MINUTE; //In seconds. Assuming the chunksize is 2 seconds, which means the chunksize 10800 chunks == 6 hours.
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = STUtilities::Rand(_smoothObject->GetCurrentEndTime() - TimeSpan_hns::ConvertFrom(TimeSpan_s::FromTicks(chunklistDuration)).Ticks(), oldStartTime);
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, ((chunklistDuration * MILLISECONDS_PER_SECOND)/_segmentSize) * STATECHANGE_TIMEOUT));

        newStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(STUtilities::Rand(newStartTime, oldStartTime)) );

    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, which is greater than 10800 chunks(6 hours). Make sure the call failed.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_GreaterThan10800chunks,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t oldStartTime = 0, proposedStartTime = 0, oldEndTime = 0;
        pkRESULT setPlaybackRangeResult = S_OK;
        int32_t chunklistDuration = 7 * 60 * SECONDS_PER_MINUTE; //In seconds.
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");

        oldStartTime = _smoothObject->GetCurrentStartTime();
        oldEndTime = _smoothObject->GetCurrentEndTime();

        proposedStartTime = _smoothObject->GetCurrentEndTime() - TimeSpan_hns::ConvertFrom(TimeSpan_s::FromTicks(chunklistDuration)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime) );
        setPlaybackRangeResult = _smoothObject->WaitForSetPlaybackRangeCompleted(((chunklistDuration * MILLISECONDS_PER_SECOND)/_segmentSize) * STATECHANGE_TIMEOUT);

        PKTEST_ASSERT_MSG_EXIT(pkE_NO_MORE_ROOM == setPlaybackRangeResult, "callback should fail with no more room");

        PKTEST_ASSERT_MSG_EXIT((_smoothObject->GetCurrentStartTime() < oldStartTime) && (_smoothObject->GetCurrentStartTime() > proposedStartTime), "the new start time should be less than old start time and greater than proposed time");
        
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(STUtilities::Rand(_smoothObject->GetCurrentStartTime(), oldStartTime)) );
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Change the default Max chunk list to greater than 10800 chunks(6 hours).
    /// Then Call SetPlaybackRange into past segments(more than 6 hours and with in the new chunk list ). Once received the callback, and seek into past segment
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_ChangeDefaultMaxChunklist,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t oldStartTime = 0, proposedStartTime = 0, newStartTime = 0, newEndTime = 0, oldEndTime = 0;
        int32_t proposedChunklistDuration = 7 * 60 * SECONDS_PER_MINUTE; //In seconds.
        int32_t newChunkListCount = 8 * 60 * SECONDS_PER_MINUTE / 2; //Assuming the chunksize is 2 seconds, which means the chunksize 10800 chunks == 6 hours.
        
        string chunkListSizeString = toString(newChunkListCount);
        const char* g_szChunkListMaxSize = chunkListSizeString.c_str();
        _smoothObject->SendExtendedCommand(EXC_CHUNK_LIST_MAX_SIZE, 1, &g_szChunkListMaxSize);


        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");

        oldStartTime = _smoothObject->GetCurrentStartTime();
        oldEndTime = _smoothObject->GetCurrentEndTime();

        proposedStartTime = _smoothObject->GetCurrentEndTime() - TimeSpan_hns::ConvertFrom(TimeSpan_s::FromTicks(proposedChunklistDuration)).Ticks();
        PKTEST_HRESULT_MSG_EXIT(_smoothObject->SetPlaybackRangeAsync(proposedStartTime), "SetPlaybackRange should not fail" );
        PKTEST_HRESULT_EXIT(_smoothObject->WaitForSetPlaybackRangeCompleted( ((proposedChunklistDuration * MILLISECONDS_PER_SECOND)/_segmentSize) * STATECHANGE_TIMEOUT ));

        newStartTime = _smoothObject->GetCurrentStartTime();
        newEndTime = _smoothObject->GetCurrentEndTime(); 

        //PKTEST_ASSERT_MSG_EXIT(CompareTimeStamps(newEndTime - TimeSpan_hns::ConvertFrom(TimeSpan_s::FromTicks(defaultChunklistDuration)).Ticks(), newStartTime, _nonAccurateSeekThreshold ), "Proposed SetPlaybackRange time is different from new ");
        PKTEST_ASSERT_MSG_EXIT(oldEndTime <= newEndTime, "the old Right edge should be lesser or equal to new right edge as new chunks are added to the chunklist");

        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(STUtilities::Rand(newStartTime, oldStartTime)) );
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// During Manifest Ready, Call SetPlaybackRange into the DVR Window and left hand side of current play back. Once received the callback, Check the left Edge
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_WithInDVRWindow_DuringManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        _smoothObject->SetManifestReadyMethod( this, &LiveToVodTests::LiveToVod_SetPlaybackRange_WithInDVRWindow_DuringManifestReady_Method );

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate("LiveToVodMultiAudio", true) );

        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(STUtilities::Rand(_smoothObject->GetCurrentStartTime(), _smoothObject->GetCurrentEndTime())) );

exit:
        return;
    }
    void LiveToVod_SetPlaybackRange_WithInDVRWindow_DuringManifestReady_Method( IManifest* pManifest,  pkRESULT hr )
    {
        int64_t proposedStartTime = 0;
        
        proposedStartTime = ( _smoothObject->GetCurrentStartTime() + _smoothObject->GetCurrentEndTime() ) / 2;
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, STATECHANGE_TIMEOUT));

exit:
        return ;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// After Manifest Ready, Call SetPlaybackRange into with in the DVR Window and left hand side of current play back. Once received the callback, Check the left Edge
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_WithInDVRWindow_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        proposedStartTime = ( _smoothObject->GetCurrentStartTime() + _smoothObject->GetCurrentEndTime() ) / 2;
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, STATECHANGE_TIMEOUT));
        
        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(_smoothObject->GetRandomSeekablePosition()) );
    exit:
        return;
    }


    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into with in the DVR Window and right hand side of current play back. Make sure call fails
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_RightSideOfCurrentTime,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, proposedCurrentTime = 0, oldStartTime = 0;
        pkRESULT result = S_OK;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedCurrentTime = ( _smoothObject->GetCurrentStartTime() + _smoothObject->GetCurrentEndTime() ) / 2;
        PKTEST_FUNC_EXIT(_smoothObject->PlayAtAndValidate(proposedCurrentTime));

        proposedStartTime = ( _smoothObject->GetCurrentPlayBackTime() + _smoothObject->GetCurrentEndTime() ) / 2;
        
        result = _smoothObject->SetPlaybackRangeAsync(proposedStartTime);
        PKTEST_ASSERT_MSG_EXIT(result == pkE_INVALID_REQUEST, "SetPlaybackRangeAsync should fail when proposed start time is right hand side of current Playback.");
        
        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(STUtilities::Rand(oldStartTime, proposedCurrentTime)) );
        PKTEST_ASSERT_MSG_EXIT(oldStartTime == _smoothObject->GetCurrentStartTime(), "startTime should not have moved, when setplaybackrange failed.");
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into right side of DVR window(right side of Live). Make sure call fails
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_RightSideOfLive,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0, oldEndTime = 0;
        pkRESULT result = S_OK;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldEndTime = _smoothObject->GetCurrentEndTime();
        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = _smoothObject->GetCurrentEndTime() + TOTAL_SECONDS_IN_A_DAY;

        result = _smoothObject->SetPlaybackRangeAsync(proposedStartTime);
        PKTEST_ASSERT_MSG_EXIT(result == pkE_INVALID_REQUEST, "SetPlaybackRangeAsync should fail when proposed start time is right hand side of Live position.");

        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate(STUtilities::Rand(oldStartTime, oldEndTime)) );
        PKTEST_ASSERT_MSG_EXIT(oldStartTime == _smoothObject->GetCurrentStartTime(), "startTime should not have moved, when setplaybackrange failed.");
    exit:
        return;
    }
    
    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange right after Open, and before ManifestReady(make sure you use a big manifest, so that it takes time to download the large manifest).
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_AfterOpen_BeforeManifestReady,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0;
        pkRESULT result = S_OK;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(urlSourceString).c_str());
        proposedStartTime = TOTAL_SECONDS_IN_A_DAY;

        PKTEST_HRESULT_EXIT(_smoothObject->OpenVideo(urlString));

        result = _smoothObject->SetPlaybackRangeAsync(proposedStartTime);
        PKTEST_ASSERT_MSG_EXIT(result == S_OK, "SetPlaybackRangeAsync should fail when called before Manifest ready");
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "timeout happened before ManifestReady");

    exit:
        return;
    }
    
    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, and while waiting for the callback, do a skip into current PlaybackRange.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SkipInToCurrentPlaybackRange_During,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t proposedSkip = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = ( _smoothObject->GetCurrentStartTime() + _smoothObject->GetCurrentEndTime() ) / 2;
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        proposedSkip = TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks(_smoothObject->GetCurrentEndTime() - _smoothObject->GetRandomSeekablePosition())).Ticks();
        PKTEST_ASSERT_EXIT(proposedSkip == (int32_t)proposedSkip);
        PKTEST_FUNC_EXIT( _smoothObject->SkipAndValidate( -(int32_t)proposedSkip ) );
        PKTEST_HRESULT_EXIT( _smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, and while waiting for the callback, do a skip into future PlaybackRange(exclude current playback range).
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SkipInToFuturePlaybackRange_During,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t proposedSkip = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        proposedSkip = TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks(_smoothObject->GetCurrentEndTime() - STUtilities::Rand(proposedStartTime, oldStartTime))).Ticks();
        PKTEST_ASSERT_EXIT(proposedSkip == (int32_t)proposedSkip);
        PKTEST_FUNC_EXIT( _smoothObject->SkipAndValidate( -(int32_t)proposedSkip ) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetCurrentPlayBackTime() > oldStartTime, "Before the callback arrives, skip should be using old startTime. ");
        PKTEST_HRESULT_EXIT( _smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, do a skip into new playback range after the callback and while waiting for the callback.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SkipInToFuturePlaybackRange_After,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t proposedSkip = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        oldStartTime = _smoothObject->GetCurrentStartTime();
        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        proposedSkip = TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks(_smoothObject->GetCurrentEndTime() - STUtilities::Rand(proposedStartTime, oldStartTime))).Ticks();
        PKTEST_ASSERT_EXIT(proposedSkip == (int32_t)proposedSkip);
        PKTEST_FUNC_EXIT( _smoothObject->SkipAndValidate( -(int32_t)proposedSkip ) );
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, and while waiting for the callback, do a seek into current PlaybackRange.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SeekInToCurrentPlaybackRange_During,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t proposedSeek = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        proposedSeek = STUtilities::Rand(oldStartTime, _smoothObject->GetCurrentEndTime());
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate( proposedSeek ) );
        PKTEST_HRESULT_EXIT( _smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));
        PKTEST_ASSERT_EXIT(CompareTimeStamps(proposedStartTime, _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold()));
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, and while waiting for the callback, do a seek into future PlaybackRange(exclude current playback range).
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SeekInToFuturePlaybackRange_During,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t proposedSeek = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        proposedSeek = STUtilities::Rand(proposedStartTime, oldStartTime);
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate( proposedSeek ) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetCurrentPlayBackTime() > oldStartTime, "Since setplayback range is not finished, we should snap to old startTime");

        PKTEST_HRESULT_EXIT( _smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetCurrentPlayBackTime() > oldStartTime, "Since setplayback range is not finished, we should snap to old startTime");

    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, do a seek into new playback range after the callback.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SeekInToFuturePlaybackRange_After,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t proposedSeek = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        proposedSeek = STUtilities::Rand(proposedStartTime, oldStartTime);
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate( proposedSeek ) );
    exit:
        return;
    }
    
    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into DVR window, do a seek into dvr window, which is going to be not in the new DVR Window.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SeekInToOldDVR_During,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t proposedSeek = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = ( _smoothObject->GetCurrentStartTime() + _smoothObject->GetCurrentEndTime() ) / 2;
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        proposedSeek = STUtilities::Rand(proposedStartTime, oldStartTime);
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate( proposedSeek ) );
        PKTEST_HRESULT_EXIT( _smoothObject->WaitForSetPlaybackRangeCompleted(STATECHANGE_TIMEOUT));
        PKTEST_ASSERT_MSG_EXIT(CompareTimeStamps(_smoothObject->GetCurrentPlayBackTime(), proposedStartTime, _smoothObject->GetNonAccurateSeekThreshold()), "Since SetPlaybackrange in to DVR is instantaneous. Seek should snap to new leftEdge.");

    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Do a Seek and Call SetPlaybackRange immediately into previous segment(short jump).
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_DuringSeek,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, proposedSeek = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        
        proposedSeek = ( _smoothObject->GetCurrentStartTime() + _smoothObject->GetCurrentEndTime() ) / 2;
        PKTEST_HRESULT_EXIT( _smoothObject->Seek( proposedSeek ) );

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_SHORT * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_SHORT * STATECHANGE_TIMEOUT));

        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Smooth transport should be in playing state.");
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetCurrentPlayBackTime() > proposedSeek, "current play position should be greater than proposed seek.");

    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Do a Seek and Call SetPlaybackRange immediately to right hand side of seek position
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_DuringSeek_RightSideofSeekPosition,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, proposedSeekTime = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        
        proposedSeekTime = (_smoothObject->GetCurrentStartTime() + _smoothObject->GetCurrentEndTime()) / 2;
        PKTEST_HRESULT_EXIT( _smoothObject->Seek( proposedSeekTime ) );

        proposedStartTime = ( proposedSeekTime + _smoothObject->GetCurrentEndTime() ) / 2;
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, STATECHANGE_TIMEOUT));
        PKTEST_ASSERT_EXIT( _smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT));
        PKTEST_ASSERT_MSG_EXIT(CompareTimeStamps(proposedStartTime, _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold() ), "SetPlaybackRange during Seek, with a minTime ahead of the seek time, should force the seek to snap to proposed minTime");

        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Smooth transport should be in playing state.");

    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, do a Seek to left of the new start position, after the callback
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SeekBeyondFutureStart_After,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t proposedSeek = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        proposedSeek = proposedStartTime - DEFAULT_SKIP_10MHZ;
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate( proposedSeek ) );
        PKTEST_ASSERT_MSG_EXIT(CompareTimeStamps(_smoothObject->GetCurrentPlayBackTime(), proposedStartTime, _smoothObject->GetNonAccurateSeekThreshold()), "current play position should be greater than proposedStartTime.");

exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, and while waiting for the callback, do a PlayAt into current PlaybackRange.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_PlayAtInToCurrentPlaybackRange_During,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t proposedPlayAt = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        proposedPlayAt = STUtilities::Rand(oldStartTime, _smoothObject->GetCurrentEndTime());
        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate( proposedPlayAt, NORMAL_SPEED, ZERO_TIMEOUT ) );
        PKTEST_HRESULT_EXIT( _smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, and while waiting for the callback, do a PlayAt into future PlaybackRange(exclude current playback range).
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_PlayAtInToFuturePlaybackRange_During,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t proposedPlayAt = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        proposedPlayAt = STUtilities::Rand(proposedStartTime, oldStartTime);
        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate( proposedPlayAt, NORMAL_SPEED, ZERO_TIMEOUT ) );
        PKTEST_HRESULT_EXIT( _smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetCurrentPlayBackTime() > oldStartTime, "current play position should be greater than oldStartTime.");

    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, do a PlayAt into new playback range after the callback
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_PlayAtInToFuturePlaybackRange_After,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t proposedPlayAt = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        proposedPlayAt = STUtilities::Rand(proposedStartTime, oldStartTime);
        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate( proposedPlayAt ) );
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, do a PlayAt to left of the new start position, after the callback
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_PlayAtBeyondFutureStart_After,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t proposedPlayAt = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        proposedPlayAt = proposedStartTime - DEFAULT_SKIP_10MHZ;
        PKTEST_FUNC_EXIT( _smoothObject->PlayAtAndValidate( proposedPlayAt, NORMAL_SPEED, ZERO_TIMEOUT ) );
        PKTEST_ASSERT_MSG_EXIT(CompareTimeStamps(_smoothObject->GetCurrentPlayBackTime(), proposedStartTime, _smoothObject->GetNonAccurateSeekThreshold()), "current play position should be greater than proposedStartTime.");

exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segments, and while waiting for the callback, do a pause. Do a play after the callback.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_PauseDuringCallback,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_SHORT * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        PKTEST_HRESULT_EXIT( _smoothObject->Pause(0));
        PKTEST_HRESULT_EXIT( _smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_SHORT * STATECHANGE_TIMEOUT));


    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into previous segment(short jump) during Rewind, and expect to see rewind go till new leftEdge and get MediaEnded
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_CallSetPlayBackRange_DuringRewind,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        PKTEST_FUNC_EXIT(_smoothObject->PlayAtAndValidate(( _smoothObject->GetCurrentStartTime() + _smoothObject->GetCurrentEndTime() ) / 2));
        PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(LOW_SPEED, VALIDATION_DELAY));
        
        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_SHORT * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_SHORT * STATECHANGE_TIMEOUT));

        PKTEST_FUNC_EXIT(_smoothObject->WaitForMediaEnded(_segmentSize * _segmentCount));
        PKTEST_ASSERT_EXIT(CompareTimeStamps(proposedStartTime, _smoothObject->GetCurrentPlayBackTime(), _smoothObject->GetNonAccurateSeekThreshold()));

    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into previous segment(short jump), Do a rewind, and expect to see rewind go till new leftEdge and get MediaEnded
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_RewindDuringCallback,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        PKTEST_FUNC_EXIT(_smoothObject->PlayAtAndValidate(( _smoothObject->GetCurrentStartTime() + _smoothObject->GetCurrentEndTime() ) / 2));

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_SHORT * _segmentSize)).Ticks();
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        PKTEST_FUNC_EXIT(_smoothObject->RewindAndValidate(LOW_SPEED, VALIDATION_DELAY));

        PKTEST_FUNC_EXIT(_smoothObject->WaitForMediaEnded(_segmentSize * _segmentCount));
        PKTEST_FUNC_EXIT(_smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_SHORT * STATECHANGE_TIMEOUT));
        PKTEST_ASSERT_EXIT(CompareTimeStamps(proposedStartTime, _smoothObject->GetCurrentPlayBackTime(), _smoothObject->GetNonAccurateSeekThreshold()));
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment(Big jump), Do a rewind, and expect to get MediaEnded before the Callback.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_RewindDuringCallback_MediaEndedBeforeCallback,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0, segmentSize_hns = 0;
        string latencyProfile;
        string urlString;

        urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(urlSourceString).c_str());
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        PKTEST_FUNC_EXIT(_smoothObject->PlayAtAndValidate(_smoothObject->GetCurrentStartTime() - DEFAULT_SKIP_10MHZ));

        segmentSize_hns = TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(_segmentSize)).Ticks();
        oldStartTime = _smoothObject->GetCurrentStartTime();

        for (uint32_t i = 1; i <= NUMBER_OF_SEGMENTS_LONG; i++)
        {
            latencyProfile = SSPKHelpers::GetCR_SegmentManifestLatencyProfile(_smoothObject->GetCurrentStartTime() - (i * segmentSize_hns), segmentSize_hns, SSPKHelpers::GetLocalUrl(urlString), 500);

            SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), latencyProfile);
        }

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        PKTEST_HRESULT_EXIT(_smoothObject->Rewind(HIGH_SPEED));
        PKTEST_FUNC_EXIT(_smoothObject->WaitForMediaEnded(_segmentSize * _segmentCount));
        PKTEST_FUNC_EXIT(_smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        PKTEST_ASSERT_EXIT(CompareTimeStamps(proposedStartTime, _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold()));
        PKTEST_ASSERT_EXIT(CompareTimeStamps(oldStartTime, _smoothObject->GetCurrentPlayBackTime(), _smoothObject->GetNonAccurateSeekThreshold()));
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, after getting a MediaEnded.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_CallSetPlaybackRange_AfterMediaEnded,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0, oldStartTime = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        PKTEST_FUNC_EXIT(_smoothObject->PlayAtAndValidate(_smoothObject->GetCurrentStartTime() - DEFAULT_SKIP_10MHZ));

        PKTEST_HRESULT_EXIT(_smoothObject->Rewind(HIGH_SPEED));
        PKTEST_ASSERT_EXIT(_smoothObject->WaitForMediaEnded(_segmentSize * _segmentCount));

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        PKTEST_FUNC_EXIT(_smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        PKTEST_ASSERT_EXIT(CompareTimeStamps(proposedStartTime, _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold()));
        PKTEST_ASSERT_EXIT(CompareTimeStamps(oldStartTime, _smoothObject->GetCurrentPlayBackTime(), _smoothObject->GetNonAccurateSeekThreshold()));
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into previous segment(short jump) during fastForward
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_CallSetPlayBackRange_DuringFastForward,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        PKTEST_FUNC_EXIT(_smoothObject->PlayAtAndValidate(( _smoothObject->GetCurrentStartTime() + _smoothObject->GetCurrentEndTime() ) / 2));
        PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(LOW_SPEED, VALIDATION_DELAY));

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_SHORT * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_SHORT * STATECHANGE_TIMEOUT));

        PKTEST_FUNC_EXIT(_smoothObject->WaitForMediaEnded(_segmentSize * _segmentCount));
        PKTEST_ASSERT_EXIT(CompareTimeStamps(_smoothObject->GetCurrentEndTime(), _smoothObject->GetCurrentPlayBackTime(), _smoothObject->GetNonAccurateSeekThreshold()));
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into previous segment(short jump), Do a fastForward before the callback.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_FastForwardDuringCallback,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        PKTEST_FUNC_EXIT(_smoothObject->PlayAtAndValidate(( _smoothObject->GetCurrentStartTime() + _smoothObject->GetCurrentEndTime() ) / 2));

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_SHORT * _segmentSize)).Ticks();
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        PKTEST_FUNC_EXIT(_smoothObject->FastForwardAndValidate(LOW_SPEED, VALIDATION_DELAY));

        PKTEST_FUNC_EXIT(_smoothObject->WaitForMediaEnded(_segmentSize * _segmentCount));
        PKTEST_FUNC_EXIT(_smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_SHORT * STATECHANGE_TIMEOUT));
        PKTEST_ASSERT_EXIT(CompareTimeStamps(proposedStartTime, _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold()));
        PKTEST_ASSERT_EXIT(CompareTimeStamps(_smoothObject->GetCurrentEndTime(), _smoothObject->GetCurrentPlayBackTime(), _smoothObject->GetNonAccurateSeekThreshold()));
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment(Big jump), Do a FastForward, and expect to get MediaEnded before the Callback.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_FastForwardDuringCallback_MediaEndedBeforeCallback,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        
        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        PKTEST_HRESULT_EXIT(_smoothObject->FastForward(HIGH_SPEED));
        PKTEST_FUNC_EXIT(_smoothObject->WaitForMediaEnded(_segmentSize * _segmentCount));
        PKTEST_FUNC_EXIT(_smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        PKTEST_ASSERT_EXIT(CompareTimeStamps(proposedStartTime, _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold()));
        PKTEST_ASSERT_EXIT(CompareTimeStamps(_smoothObject->GetCurrentEndTime(), _smoothObject->GetCurrentPlayBackTime(), _smoothObject->GetNonAccurateSeekThreshold()));
    exit:
        return;
    }
    
    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, Do a Close on the SmoothTransport object.
    /// We should expect to see a callback, with abort, before closed Event.
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_CloseSmoothTransport_During,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0;
        pkRESULT result = S_OK;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        PKTEST_HRESULT_EXIT( _smoothObject->Close() );

        result = _smoothObject->WaitForSetPlaybackRangeCompleted(STATECHANGE_TIMEOUT);
        PKTEST_ASSERT_MSG_EXIT(result == E_ABORT, "SetPlaybackRangeCompleted callback should return E_ABORT when smoothTransport is closed");
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, Delete the SmoothTransport object.
    /// We should expect to see a callback, with abort, before closed Event.
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_DestroySmoothTransport_During,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0;
        pkRESULT result = S_OK;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        
        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        _smoothObject->DestroySmoothTransport();

        result = _smoothObject->WaitForSetPlaybackRangeCompleted(STATECHANGE_TIMEOUT);
        PKTEST_ASSERT_MSG_EXIT(result == E_ABORT, "SetPlaybackRangeCompleted callback should return E_ABORT when smoothTransport is deleted");

    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, Open the SmoothTransport object with Same Url.
    /// We should expect to see a callback, with abort, before closed Event. and the new Open should have only 2 segments in its DVR window
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_OpenWithSameUrl_During,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0;
        pkRESULT result = S_OK;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        result = _smoothObject->WaitForSetPlaybackRangeCompleted(STATECHANGE_TIMEOUT);
        PKTEST_ASSERT_MSG_EXIT(result == E_ABORT, "SetPlaybackRangeCompleted callback should return E_ABORT when smoothTransport is closed");
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, Open the SmoothTransport object with different Url.
    /// We should expect to see a callback, with abort, before closed Event. and the new Open should have only 2 segments in its DVR window
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_OpenWithDiffUrl_During,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodDefault";
        string anotherUrlSourceString = "LiveToVodMultiSegments";
        int64_t proposedStartTime = 0;
        pkRESULT result = S_OK;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(anotherUrlSourceString, true) );

        result = _smoothObject->WaitForSetPlaybackRangeCompleted(STATECHANGE_TIMEOUT);
        PKTEST_ASSERT_MSG_EXIT(result == E_ABORT, "SetPlaybackRangeCompleted callback should return E_ABORT when smoothTransport is closed");
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange with -ve StartTime as parameter
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_NegativeStartTime,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodDefault";
        pkRESULT result = S_OK;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        
        result = _smoothObject->SetPlaybackRangeAsync(-PLAYBACK_TIMEOUT);
        PKTEST_ASSERT_MSG_EXIT(result == E_INVALIDARG, "SetPlaybackRangeAsync should fail when proposed start time is negative");

        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsPlaying(), "Smooth transport should be in playing." );
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange with non INT_MAX End time as the parameter
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_NonIntMaxEndTime,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t proposedStartTime = 0;
        pkRESULT result = S_OK;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        proposedStartTime = _smoothObject->GetCurrentStartTime() - DEFAULT_SKIP_10MHZ;

        result = _smoothObject->SetPlaybackRangeAsync(proposedStartTime,_smoothObject->GetCurrentEndTime());
        PKTEST_ASSERT_MSG_EXIT(result == E_INVALIDARG, "SetPlaybackRangeAsync should fail when proposed end time is non IntMax.");

        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsPlaying(), "Smooth transport should be in playing." );
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange on a Non LiveToVod content( manifest version < 3.0)
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_NonLiveToVod,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Data:StandardLive","LiveDefault")
        PKTEST_PROPERTY("Data:OnDemand","H264ODDefault")
        )
    {
        string urlSourceString = PKTest_GetTestData();
        int64_t proposedStartTime = 0;
        pkRESULT result = S_OK;
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true, false) );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetManifest()->Type() == ManifestType_Standard, "This should be a standard manifest");

        proposedStartTime = ( _smoothObject->GetCurrentStartTime() + _smoothObject->GetCurrentEndTime() ) / 2;

        result = _smoothObject->SetPlaybackRangeAsync(proposedStartTime);
        PKTEST_ASSERT_MSG_EXIT(result == pkE_INVALID_REQUEST, "SetPlaybackRangeAsync should fail when called on a non LiveToVod content.");

        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsPlaying(), "Smooth transport should be in playing." );
    exit:
        return;
    }
    
    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, with 404 on one new segment.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_one404Segment,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault", urlString;
        int64_t proposedStartTime = 0, segmentSize_hns = 0;
        string onSegment404Body;
        int32_t totalSegment404s = 0;
        vector<SmoothTransportStatus> smoothStatusVector;

        urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(urlSourceString).c_str());
        PKTEST_FUNC_EXIT(StartSmoothObjectAndValidate(urlSourceString, true));

        segmentSize_hns = TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(_segmentSize)).Ticks();

        onSegment404Body = SSPKHelpers::GetCR_SegmentManifestReturnStatusCodeProfile(_smoothObject->GetCurrentStartTime() - segmentSize_hns, segmentSize_hns, SSPKHelpers::GetLocalUrl(urlString), "404");

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), onSegment404Body);

        _smoothObject->Delay(VALIDATION_DELAY);

        // The delay is just to make sure the first chunk belongs to expected segment, but not to previous one.
        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks() + NON_ACCURATE_SEEK_THRESHOLD;
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        PKTEST_ASSERT_EXIT(CompareTimeStamps(proposedStartTime, _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold()));

        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( SmoothTransportStatus_SegmentManifestError == smoothStatusVector[i]._update ) && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                totalSegment404s++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalSegment404s == 1, "There are %d chunk 404, but we are expecting the chunk 404 count to be 1", totalSegment404s );

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, with 404s on all new segment except one, which proposedStartTime belongs to.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_one404SegmentExceptFirstOne,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault", urlString;
        int64_t proposedStartTime = 0, segmentSize_hns = 0;
        string onSegment404Body;
        int32_t totalSegment404s = 0;
        vector<SmoothTransportStatus> smoothStatusVector;

        urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(urlSourceString).c_str());
        PKTEST_FUNC_EXIT(StartSmoothObjectAndValidate(urlSourceString, true));

        segmentSize_hns = TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(_segmentSize)).Ticks();

        for (uint32_t i = 1; i < NUMBER_OF_SEGMENTS_LONG; i++)
        {
            onSegment404Body = SSPKHelpers::GetCR_SegmentManifestReturnStatusCodeProfile(_smoothObject->GetCurrentStartTime() - (i * segmentSize_hns), segmentSize_hns, SSPKHelpers::GetLocalUrl(urlString), "404");

            SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), onSegment404Body);
        }

        _smoothObject->Delay(VALIDATION_DELAY);

        // The delay is just to make sure the first chunk belongs to expected segment, but not to previous one.
        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks() + NON_ACCURATE_SEEK_THRESHOLD;
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        PKTEST_ASSERT_EXIT(CompareTimeStamps(proposedStartTime, _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold()));

        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( SmoothTransportStatus_SegmentManifestError == smoothStatusVector[i]._update ) && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                totalSegment404s++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalSegment404s == NUMBER_OF_SEGMENTS_LONG - 1, "There are %d chunk 404, but we are expecting the chunk 404 count to be %d", totalSegment404s, NUMBER_OF_SEGMENTS_LONG - 1);
exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, with one 404 on the segment where the proposed new start time belongs to.
    /// </summary>
    /// <priority value=0 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_404OnSegmentStartTimeBelongsTo,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodDefault", urlString;
        int64_t proposedStartTime = 0, segmentSize_hns = 0;
        string onSegment404Body;
        pkRESULT result = S_OK;
        int32_t totalSegment404s = 0;
        vector<SmoothTransportStatus> smoothStatusVector;

        urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(urlSourceString).c_str());
        PKTEST_FUNC_EXIT(StartSmoothObjectAndValidate(urlSourceString, true));

        segmentSize_hns = TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(_segmentSize)).Ticks();

        onSegment404Body = SSPKHelpers::GetCR_SegmentManifestReturnStatusCodeProfile(_smoothObject->GetCurrentStartTime() - (NUMBER_OF_SEGMENTS_LONG * segmentSize_hns), segmentSize_hns, SSPKHelpers::GetLocalUrl(urlString), "404");

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), onSegment404Body);

        _smoothObject->Delay(VALIDATION_DELAY);

        // The delay is just to make sure the first chunk belongs to expected segment, but not to previous one.
        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks() + NON_ACCURATE_SEEK_THRESHOLD;
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));
    
        result = _smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT);
        PKTEST_ASSERT_MSG_EXIT(result == pkE_BEFORE_VALID_RANGE, "SetPlaybackRangeCompleted callback should return pkE_BEFORE_VALID_RANGE when the segments to new start position are 404s");

        PKTEST_ASSERT_EXIT(CompareTimeStamps(proposedStartTime + segmentSize_hns, _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold()));

        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( SmoothTransportStatus_SegmentManifestError == smoothStatusVector[i]._update ) && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                totalSegment404s++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalSegment404s ==  1, "There are %d chunk 404, but we are expecting the chunk 404 count to be 1", totalSegment404s);
    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, with 404 on all new segment.
    /// </summary>
    /// <priority value=0 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_All404Segment,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodDefault", urlString;
        int64_t proposedStartTime = 0, segmentSize_hns = 0, oldStartTime = 0;
        string onSegment404Body;
        int32_t totalSegment404s = 0;
        pkRESULT result = S_OK;
        vector<SmoothTransportStatus> smoothStatusVector;

        urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(urlSourceString).c_str());
        PKTEST_FUNC_EXIT(StartSmoothObjectAndValidate(urlSourceString, true));

        segmentSize_hns = TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(_segmentSize)).Ticks();

        oldStartTime = _smoothObject->GetCurrentStartTime();

        for (uint32_t i = 1; i <= NUMBER_OF_SEGMENTS_LONG; i++)
        {
            onSegment404Body = SSPKHelpers::GetCR_SegmentManifestReturnStatusCodeProfile(_smoothObject->GetCurrentStartTime() - (i * segmentSize_hns), segmentSize_hns, SSPKHelpers::GetLocalUrl(urlString), "404");

            SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), onSegment404Body);
        }

        _smoothObject->Delay(VALIDATION_DELAY);

        // The delay is just to make sure the first chunk belongs to expected segment, but not to previous one.
        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks() + NON_ACCURATE_SEEK_THRESHOLD;
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        result = _smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT);
        PKTEST_ASSERT_MSG_EXIT(result == pkE_BEFORE_VALID_RANGE, "SetPlaybackRangeCompleted callback should return pkE_BEFORE_VALID_RANGE when the segments to new start position are 404s");

        PKTEST_ASSERT_EXIT(CompareTimeStamps(oldStartTime, _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold()));

        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( SmoothTransportStatus_SegmentManifestError == smoothStatusVector[i]._update ) && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                totalSegment404s++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalSegment404s == NUMBER_OF_SEGMENTS_LONG, "There are %d chunk 404, but we are expecting the chunk 404 count to be %d", totalSegment404s, NUMBER_OF_SEGMENTS_LONG);
exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, with 412 on all new segment.
    /// </summary>
    /// <priority value=0 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_All412Segment,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodDefault", urlString;
        int64_t proposedStartTime = 0, segmentSize_hns = 0, oldStartTime = 0;
        string onSegment412Body;
        int32_t totalSegment412s = 0;
        pkRESULT result = S_OK;
        vector<SmoothTransportStatus> smoothStatusVector;

        urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(urlSourceString).c_str());
        PKTEST_FUNC_EXIT(StartSmoothObjectAndValidate(urlSourceString, true));

        segmentSize_hns = TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(_segmentSize)).Ticks();

        oldStartTime = _smoothObject->GetCurrentStartTime();

        for (uint32_t i = 1; i <= NUMBER_OF_SEGMENTS_LONG; i++)
        {
            onSegment412Body = SSPKHelpers::GetCR_SegmentManifestReturnStatusCodeProfile(_smoothObject->GetCurrentStartTime() - (i * segmentSize_hns), segmentSize_hns, SSPKHelpers::GetLocalUrl(urlString), "412");

            SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), onSegment412Body);
        }

        _smoothObject->Delay(VALIDATION_DELAY);

        // The delay is just to make sure the first chunk belongs to expected segment, but not to previous one.
        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks() + NON_ACCURATE_SEEK_THRESHOLD;
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        result = _smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT);
        PKTEST_ASSERT_MSG_EXIT(result == pkE_BEFORE_VALID_RANGE, "SetPlaybackRangeCompleted callback should return pkE_BEFORE_VALID_RANGE when the segments to new start position are 412s");

        PKTEST_ASSERT_EXIT(CompareTimeStamps(oldStartTime, _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold()));

        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( SmoothTransportStatus_SegmentManifestError == smoothStatusVector[i]._update ) && ( smoothStatusVector[i]._httpResponse == 412 ) )
            {
                totalSegment412s++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalSegment412s == NUMBER_OF_SEGMENTS_LONG, "There are %d chunk 412, but we are expecting the chunk 412 count to be %d", totalSegment412s, NUMBER_OF_SEGMENTS_LONG);
    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment multiple Times, and make sure 2nd one fails
    /// </summary>
    /// <priority value=0 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_MultipleCallsSimultaneously,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodDefault";
        int64_t oldStartTime = 0, proposedStartTime = 0, newStartTime = 0;
        pkRESULT result = S_OK;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SSPK should be in playing state.");

        oldStartTime = _smoothObject->GetCurrentStartTime();

        proposedStartTime = oldStartTime - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));
        
        result = _smoothObject->SetPlaybackRangeAsync(proposedStartTime);
        PKTEST_ASSERT_MSG_EXIT(result == pkE_INVALID_REQUEST, "SetPlaybackRangeAsync should fail when called multiple times.");
        PKTEST_HRESULT_EXIT(_smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsPlaying(), "Smooth transport should be in playing." );

        newStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(STUtilities::Rand(newStartTime, oldStartTime)) );
    exit:
        return;
    }


    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, Do a stream selection(audio Switch) before the callback
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_AudioSwitch_During,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodMultiAudio";
        int64_t proposedStartTime = 0;
        pkRESULT audioSwitchResult = S_OK;
        VectorOfStreams proposedSelection;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeAudio);

        audioSwitchResult = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(audioSwitchResult) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

        PKTEST_FUNC_EXIT(_smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));
        PKTEST_ASSERT_EXIT(CompareTimeStamps(proposedStartTime, _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold()));
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, Do a stream selection(Text) before the callback
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_StreamSelection_During,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodWithText";
        int64_t proposedStartTime = 0;
        pkRESULT audioSwitchResult = S_OK;
        VectorOfStreams proposedSelection;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_HRESULT_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        proposedSelection = ToggledStreamByMediaType(MediaStreamTypeText);

        audioSwitchResult = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
        if (SUCCEEDED(audioSwitchResult) )
        {
            PKTEST_FUNC_EXIT( _smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT) );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT(false, "Stream Selection Asynchronous call returned failure." );
        }

        PKTEST_FUNC_EXIT( CompareGivenVectorToSelectedStreams(proposedSelection) );

        PKTEST_FUNC_EXIT(_smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));
        PKTEST_ASSERT_EXIT(CompareTimeStamps(proposedStartTime, _smoothObject->GetCurrentStartTime(), _smoothObject->GetNonAccurateSeekThreshold()));
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment(big jump), and try to get a chunkInfo using TryGetChunkInfo from the future playback range before the callback.
    /// We should get a pkE_BEFORE_VALID_RANGE
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SparseStreams_TryGetChunkInfo_FromFutureRange_During,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodWithSparse";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;
        pkRESULT hr;
        VectorOfStreams sparseStreams;
        
        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        oldStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");
        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( hr ), "We are expecting a sparse chunk at the zero ");
        PKTEST_ASSERT_MSG_EXIT(chunkInfo.chunkTime >= _smoothObject->GetCurrentStartTime(), "chunk time should be greater than currentStartTime");

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        PKTEST_HRESULT_EXIT(itChunk.MovePrev());
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(pkE_BEFORE_VALID_RANGE == hr , "we should not be able to get the chunk info from future range");

        PKTEST_HRESULT_EXIT(_smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment(big jump), and try to get a chunkInfo using TryGetChunkInfo from the future playback range after the callback returned.
    /// We should get a the chunk from new range.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SparseStreams_TryGetChunkInfo_FromFutureRange_After,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodWithSparse";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;
        pkRESULT hr;
        VectorOfStreams sparseStreams;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        oldStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");
        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( hr ), "We are expecting a sparse chunk at the zero ");
        PKTEST_ASSERT_MSG_EXIT(chunkInfo.chunkTime >= _smoothObject->GetCurrentStartTime(), "chunk time should be greater than currentStartTime");

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        PKTEST_HRESULT_EXIT(itChunk.MovePrev());
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( hr ), "We are expecting a sparse chunk at the zero ");
        PKTEST_ASSERT_MSG_EXIT(chunkInfo.chunkTime < oldStartTime, "chunk time should be less than old start time");

exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into DVR window, and try to get a chunkInfo using TryGetChunkInfo from the old playback range after the callback returned, using an unresolved chunk iterator.
    /// We should get pkE_BEFORE_VALID_RANGE
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SparseStreams_TryGetChunkInfo_FromOldRange_UnResolved_After,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Bug","35084"))
    {
        string urlSourceString = "LiveToVodWithSparse";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;
        pkRESULT hr;
        VectorOfStreams sparseStreams, selected, proposed;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        _smoothObject->Delay(_segmentSize * (_segmentCount - 1));

        oldStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");
        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();

        proposedStartTime = _smoothObject->GetCurrentPlayBackTime() - DEFAULT_SKIP_10MHZ;
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, STATECHANGE_TIMEOUT));

        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(pkE_BEFORE_VALID_RANGE == hr , "we should not be able to get the chunk info from future range");

    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into DVR window, and try to get a chunkInfo using TryGetChunkInfo from the old playback range after the callback returned, using an already resolved iterator.
    /// We should get pkE_BEFORE_VALID_RANGE
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SparseStreams_TryGetChunkInfo_FromOldRange_Resolved_After,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Bug","35084"))
    {
        string urlSourceString = "LiveToVodWithSparse";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        CHUNK_INFO chunkInfo,chunkInfo2;
        ChunkIterator itChunk,itChunk2;
        pkRESULT hr;
        VectorOfStreams sparseStreams;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        _smoothObject->Delay(_segmentSize * (_segmentCount - 1));
        oldStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");
        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        LogTestComment("sparse : %lld", chunkInfo.chunkTime);
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( hr ), "We are expecting a sparse chunk at the zero ");
        PKTEST_ASSERT_MSG_EXIT(chunkInfo.chunkTime >= _smoothObject->GetCurrentStartTime(), "chunk time should be greater than currentStartTime");

        proposedStartTime = _smoothObject->GetCurrentPlayBackTime() - DEFAULT_SKIP_10MHZ;
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, STATECHANGE_TIMEOUT));

        itChunk2 = sparseStreams[0]->GetFirstInCurrentChunkList();
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk2, &chunkInfo2 );
        LogTestComment("sparse : %lld", chunkInfo2.chunkTime);
        PKTEST_ASSERT_MSG_EXIT(pkE_BEFORE_VALID_RANGE == hr , "we should not be able to get the chunk info from future range");

exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, and try to get the first chunkInfo using TryGetChunkInfo from the old playback range after the callback returned.
    /// the first chunks got before and after set playback range should be different.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SparseStreams_TryGetChunkInfo_FirstChunkDifferentAfter,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodWithSparse";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t oldFirstChunkTime = 0;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;
        pkRESULT hr;
        VectorOfStreams sparseStreams;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        oldStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");
        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();

        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        oldFirstChunkTime = chunkInfo.chunkTime;
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( hr ), "We are expecting a sparse chunk at the zero ");
        PKTEST_ASSERT_MSG_EXIT(chunkInfo.chunkTime >= _smoothObject->GetCurrentStartTime(), "chunk time should be greater than currentStartTime");

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();

        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( hr ), "We are expecting a sparse chunk at the zero ");
        PKTEST_ASSERT_MSG_EXIT(chunkInfo.chunkTime != oldFirstChunkTime, "the first chunks got before and after SetPlaybackRange should be different");

    exit:
        return;
    }
    
    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment, and try to get the first chunkInfo using TryGetChunkInfo from the old playback range after the callback returned.
    /// the first chunk's timestamp got after the setPlaybackRange should be greater than new playback range startTime.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SparseStreams_TryGetChunkInfo_FirstSparseChunkInWindow,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodWithSparse";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t oldFirstChunkTime = 0;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;
        pkRESULT hr;
        VectorOfStreams sparseStreams;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        oldStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");
        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();

        PKTEST_HRESULT_EXIT( sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo ) );
        oldFirstChunkTime = chunkInfo.chunkTime;

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();

        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( hr ), "We are expecting a sparse chunk at the zero ");
        PKTEST_ASSERT_MSG_EXIT(chunkInfo.chunkTime < oldStartTime, "The new first sparse chunk should be less than old start time");
        PKTEST_ASSERT_MSG_EXIT(chunkInfo.chunkTime != oldFirstChunkTime, "the first chunks got before and after SetPlaybackRange should be different");

exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment(big jump), and try to get a chunkInfo using GetChunkInfoAsync from the future playback range before the callback.
    /// We should get a pkE_BEFORE_VALID_RANGE
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SparseStreams_GetChunkInfoAsync_FromFutureRange_During,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodWithSparse";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t chunkTimestamp = 0;
        ChunkIterator itChunk;
        pkRESULT chunkInfoResult,chunkInfoCallBackResult;
        VectorOfStreams sparseStreams;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        oldStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");
        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();

        chunkInfoResult = sparseStreams[0]->GetChunkInfoAsync(itChunk, _smoothObject );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( chunkInfoResult ), "The GetChunkInfoAsync call should not fail ");

        chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, chunkTimestamp );
        PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( chunkInfoCallBackResult ), "The GetChunkInfoAsync callback should not fail for already available sparse chunks" );
        PKTEST_ASSERT_MSG_EXIT(chunkTimestamp >= _smoothObject->GetCurrentStartTime() - _smoothObject->GetNonAccurateSeekThreshold(), "chunk time should be greater than currentStartTime");
        
        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        PKTEST_HRESULT_EXIT(itChunk.MovePrev());
        chunkInfoResult = sparseStreams[0]->GetChunkInfoAsync(itChunk, _smoothObject );
        PKTEST_ASSERT_MSG_EXIT(pkE_BEFORE_VALID_RANGE == chunkInfoResult , "we should not be able to get the chunk info from future range");

        PKTEST_HRESULT_EXIT(_smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));
    exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment(big jump), and try to get a chunkInfo using GetChunkInfoAsync from the future playback range after the callback.
    /// We should get a the chunk from new range
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SparseStreams_GetChunkInfoAsync_FromFutureRange_After,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodWithSparse";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t chunkTimestamp = 0;
        ChunkIterator itChunk;
        pkRESULT chunkInfoResult,chunkInfoCallBackResult;
        VectorOfStreams sparseStreams;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        oldStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");
        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();

        chunkInfoResult = sparseStreams[0]->GetChunkInfoAsync(itChunk, _smoothObject );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( chunkInfoResult ), "The GetChunkInfoAsync call should not fail ");

        chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, chunkTimestamp );
        PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( chunkInfoCallBackResult ), "The GetChunkInfoAsync callback should not fail for already available sparse chunks" );
        PKTEST_ASSERT_MSG_EXIT(chunkTimestamp >= _smoothObject->GetCurrentStartTime() - _smoothObject->GetNonAccurateSeekThreshold(), "chunk time should be greater than currentStartTime");

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        PKTEST_HRESULT_EXIT(itChunk.MovePrev());
        chunkInfoResult = sparseStreams[0]->GetChunkInfoAsync(itChunk, _smoothObject );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( chunkInfoResult ), "The GetChunkInfoAsync call should not fail ");

        chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, chunkTimestamp );
        PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( chunkInfoCallBackResult ), "The GetChunkInfoAsync callback should not fail for already available sparse chunks" );
        PKTEST_ASSERT_MSG_EXIT(chunkTimestamp >= _smoothObject->GetCurrentStartTime() - _smoothObject->GetNonAccurateSeekThreshold(), "chunk time should be greater than currentStartTime");

exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment(big jump), and get a chunk using DownloadFragmentAsync from the future playback range before the callback.
    /// We should get a pkE_BEFORE_VALID_RANGE
    /// </summary>
    /// <priority value=0 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SparseStreams_DownloadFragmentAsync_FromFutureRange_During,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlSourceString = "LiveToVodWithSparse";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t chunkTimestamp = 0, downloadedBufferSize = 0, totalChunkSize = 0;
        ChunkIterator itChunk;
        pkRESULT downloadResult,downloadCallBackResult;
        VectorOfStreams sparseStreams;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;
        bool isChunkComplete;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        oldStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");
        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();
        sparseStreams[0]->GetSelectedTracks(&selectedTracks);

        downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

        downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
        PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( downloadCallBackResult ), "The Downloadfragment callback should not fail" );
        PKTEST_ASSERT_MSG_EXIT(chunkTimestamp >= _smoothObject->GetCurrentStartTime() - _smoothObject->GetNonAccurateSeekThreshold(), "chunk time should be greater than currentStartTime");

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAsync(proposedStartTime));

        PKTEST_HRESULT_EXIT(itChunk.MovePrev());
        downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
        //PKTEST_ASSERT_MSG_EXIT(pkE_BEFORE_VALID_RANGE == downloadResult , "we should not be able to get the chunk from future range");

        PKTEST_HRESULT_EXIT(_smoothObject->WaitForSetPlaybackRangeCompleted(NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));
exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// Call SetPlaybackRange into past segment(big jump), and get a chunk using DownloadFragmentAsync from the future playback range after the callback.
    /// We should get a the chunk from new range
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SparseStreams_DownloadFragmentAsync_FromFutureRange_After,
        PKTEST_PROPERTY("Priority", "1"))
    {
        string urlSourceString = "LiveToVodWithSparse";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t chunkTimestamp = 0, downloadedBufferSize = 0, totalChunkSize = 0;
        ChunkIterator itChunk;
        pkRESULT downloadResult,downloadCallBackResult;
        VectorOfStreams sparseStreams;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;
        bool isChunkComplete;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        oldStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");
        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();
        sparseStreams[0]->GetSelectedTracks(&selectedTracks);

        downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

        downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
        PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( downloadCallBackResult ), "The Downloadfragment callback should not fail" );
        PKTEST_ASSERT_MSG_EXIT(chunkTimestamp >= _smoothObject->GetCurrentStartTime() - _smoothObject->GetNonAccurateSeekThreshold(), "chunk time should be greater than currentStartTime");

        proposedStartTime = _smoothObject->GetCurrentStartTime() - TimeSpan_hns::ConvertFrom(TimeSpan_ms::FromTicks(NUMBER_OF_SEGMENTS_LONG * _segmentSize)).Ticks();
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, NUMBER_OF_SEGMENTS_LONG * STATECHANGE_TIMEOUT));

        PKTEST_HRESULT_EXIT(itChunk.MovePrev());
        downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

        downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
        PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( downloadCallBackResult ), "The Downloadfragment callback should not fail" );
exit:
        return;
    }

    ////////////////////////////////////////////////////
    /// <summary>
    /// During a DownloadFragmentAsync call to get the first, Call SetPlaybackRange into DVR.
    /// We should get OnFragmentData callback with E_Abort
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( LiveToVod_SetPlaybackRange_SparseStreams_DownloadFragmentAsync_DuringSetPlaybackRangeAsync,
        PKTEST_PROPERTY("Priority", "2")
        PKTEST_PROPERTY("Bug","35026"))
    {
        string urlSourceString = "LiveToVodWithSparse";
        int64_t proposedStartTime = 0, oldStartTime = 0;
        int64_t chunkTimestamp = 0, downloadedBufferSize = 0, totalChunkSize = 0;
        ChunkIterator itChunk;
        pkRESULT downloadResult,downloadCallBackResult;
        VectorOfStreams sparseStreams;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;
        bool isChunkComplete;

        PKTEST_FUNC_EXIT( StartSmoothObjectAndValidate(urlSourceString, true) );
        oldStartTime = _smoothObject->GetCurrentStartTime();

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");
        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();
        sparseStreams[0]->GetSelectedTracks(&selectedTracks);

        downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

        proposedStartTime = (_smoothObject->GetCurrentStartTime() + _smoothObject->GetCurrentEndTime()) / 2;
        PKTEST_FUNC_EXIT( _smoothObject->SetPlaybackRangeAndValidate(proposedStartTime, STATECHANGE_TIMEOUT));

        downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
        PKTEST_ASSERT_MSG_EXIT( FAILED( downloadCallBackResult ), "The Downloadfragment callback should  fail, when playback range changed." );

exit:
        return;
    }


private:
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

    void CompareGivenVectorToSelectedStreams( VectorOfStreams proposedSelection )
    {
        VectorOfStreams selectedStreams;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams( &selectedStreams ) );
        PKTEST_FUNC_EXIT( SSPKHelpers::CompareTwoVectors( proposedSelection, selectedStreams ) );

    exit:
        return;
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

    void CheckAvailableStreamCountByType(int32_t argAudioStreamCount,int32_t argVideoStreamCount,int32_t argOtherStreamCount)
    {
        VectorOfStreams availableStreams;
        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );

        PKTEST_FUNC_EXIT( SSPKHelpers::CheckStreamCountByType(availableStreams, argAudioStreamCount, argVideoStreamCount, argOtherStreamCount) );

    exit:
        return;
    }

    void SelectASparseStream()
    {
        VectorOfStreams availableStreams;
        VectorOfStreams selectedStreams;
        VectorOfStreams proposedSelection;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        if( GetAllSelectedSparseStreams().size() == 0)
        {
            PKTEST_FUNC_EXIT(proposedSelection = SSPKHelpers::AddTextStream(availableStreams, selectedStreams, true));
            PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->SelectStreamsAsync(_smoothObject, proposedSelection) );
            PKTEST_FUNC_EXIT(_smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT));
        }

    exit:
        return;
    }

    VectorOfStreams GetAllSelectedSparseStreams()
    {
        VectorOfStreams selectedStreams;
        VectorOfStreams proposedSelection;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams);
        proposedSelection = SSPKHelpers::GetAllSparseStreams(selectedStreams);

    exit:
        return proposedSelection;
    }

};
