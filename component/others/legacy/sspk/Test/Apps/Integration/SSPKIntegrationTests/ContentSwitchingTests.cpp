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
#include "STUtilities.h"

using namespace SSPK;
using namespace SSPKTest;

PKTEST_GROUP( ContentSwitchingTests )
{

    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;

    ContentSwitchingTests()
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

    PKTEST_METHOD_EX(
        ContentSwitchSourcesWithDestroy,
        PKTEST_PROPERTY("Data:NonExistentManifest-H264Vod", "NonExistentManifest--H264ODDefault" )
        PKTEST_PROPERTY("Data:H264Vod-NonExistentManifest", "H264ODDefault--NonExistentManifest" )
        PKTEST_PROPERTY("Data:H264Vod-H264Vod", "H264ODDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:H264Vod-H264Live", "H264ODDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:H264Live-H264Vod", "LiveDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:H264Live-H264Live", "LiveDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-VC1Vod", "VC1ODDefault--VC1ODDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-VC1Live", "VC1ODDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:VC1Live-VC1Vod", "VC1LiveDefault--VC1ODDefault" )
        PKTEST_PROPERTY("Data:VC1Live-VC1Live","VC1LiveDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:H264Live-VC1Vod", "LiveDefault--VC1ODDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-H264Vod", "VC1ODDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-H264Live", "VC1ODDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:H264Vod-VC1Live", "H264ODDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:VC1Live-H264Vod", "VC1LiveDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:VC1Live-H264Live", "VC1LiveDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:H264Live-VC1Live", "LiveDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:H264Vod-H264LiveWithText", "H264ODDefault--H264LiveMultiAudioTextWithLargeChunkDuration" )
        PKTEST_PROPERTY("Data:H264LiveWithText-H264VodMultiVideo", "H264LiveMultiAudioTextWithLargeChunkDuration--H264LiveMultiAudioTextWithLargeChunkDuration" )
        PKTEST_PROPERTY("Data:H264ChangeSourcesWithDifferentAudioBitrates", "LiveDefault--H264ODMultiAudio2")
        PKTEST_PROPERTY("Data:H264ODMultiAudio-H264Live", "H264ODMultiAudio--LiveDefault")
        PKTEST_PROPERTY("Data:DigitalRapids-H264Live", "H264ODDigitalRapids--LiveDefault")
        PKTEST_PROPERTY("Data:H264Live-H24Envivio", "LiveDefault--H264ODEnvivio")
        PKTEST_PROPERTY("Data:LiveProtected-ProtectedVod", "LiveProtectedPiff1.3ShortKR--ODProtectedPiff1.3KROnEveryFragment")
        PKTEST_PROPERTY("Data:LiveProtected-H264Vod", "LiveProtectedPiff1.3ShortKR--H264ODDefault")
        PKTEST_PROPERTY("Data:LiveProtected-H264Live", "LiveProtectedPiff1.3ShortKR--LiveDefault")
        PKTEST_PROPERTY("Data:ProtectedVod-ProtectedLive", "ODProtectedPiff1.3KROnEveryFragment--LiveProtectedPiff1.3ShortKR")
        PKTEST_PROPERTY("Data:ProtectedVod-H264Vod", "ODProtectedPiff1.3KROnEveryFragment--H264ODDefault")
        PKTEST_PROPERTY("Data:ProtectedVod-H264Live", "ODProtectedPiff1.3KROnEveryFragment--LiveDefault")
        PKTEST_PROPERTY("Data:H264Vod-ProtectedVod", "LiveDefault--ODProtectedPiff1.3KROnEveryFragment")
        PKTEST_PROPERTY("Data:H264Live-ProtectedLive", "LiveDefault--LiveProtectedPiff1.3ShortKR")
        PKTEST_PROPERTY("Data:H264OD-ProtectedOD", "H264ODDefault--ODProtectedPiff1.3KROnEveryFragment")
        PKTEST_PROPERTY("Data:H264OD-ProtectedLive", "H264ODDefault--LiveProtectedPiff1.3ShortKR")
        PKTEST_PROPERTY("Keyword", "ChannelSwitch")
        )
    {
        uint64_t startTime = 0;
        uint64_t endTime = 0;
        string firstSource;
        string secondSource;
        bool firstSourceWasPlaying = false;

        PKTEST_FUNC_EXIT( GetSourceStrings(PKTest_GetTestData(), &firstSource, &secondSource) );
        
        //not using this startTime
        PKTEST_FUNC_EXIT( OpenMediaAndPlay(firstSource, &startTime) );

        firstSourceWasPlaying = ( 0 != _smoothObject->GetStatusUpdatesList().Playing );

        startTime = STUtilities::GetSystemTime();
        PKTEST_HRESULT_EXIT( _smoothObject->Close() );

        _smoothObject->DestroySmoothTransport();
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetStatusUpdatesList().Closed == (firstSourceWasPlaying ? 1 : 0), "There should be %d Closed Event, but got %d", (firstSourceWasPlaying ? 1 : 0), _smoothObject->GetStatusUpdatesList().Closed);

        delete _smoothObject;

        _smoothObject = NEW_NO_THROW STWrapper();
        PKTEST_ASSERT_EXIT(NULL != _smoothObject);
        
        PKTEST_FUNC_EXIT( OpenMediaAndPlay(secondSource, &endTime) );
        


        LogTestComment("Time took for channel switch is : %.3f", ((double)(endTime - startTime)) / TIMESCALE_10MHZ);

    exit:
        return;
    }

    PKTEST_METHOD_EX(
        ContentSwitchSourcesWithDestroyWithoutClose,
        PKTEST_PROPERTY("Data:NonExistentManifest-H264Vod", "NonExistentManifest--H264ODDefault" )
        PKTEST_PROPERTY("Data:H264Vod-NonExistentManifest", "H264ODDefault--NonExistentManifest" )
        PKTEST_PROPERTY("Data:H264Vod-H264Vod", "H264ODDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:H264Vod-H264Live", "H264ODDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:H264Live-H264Vod", "LiveDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:H264Live-H264Live", "LiveDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-VC1Vod", "VC1ODDefault--VC1ODDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-VC1Live", "VC1ODDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:VC1Live-VC1Vod", "VC1LiveDefault--VC1ODDefault" )
        PKTEST_PROPERTY("Data:VC1Live-VC1Live","VC1LiveDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:H264Live-VC1Vod", "LiveDefault--VC1ODDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-H264Vod", "VC1ODDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-H264Live", "VC1ODDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:H264Vod-VC1Live", "H264ODDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:VC1Live-H264Vod", "VC1LiveDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:VC1Live-H264Live", "VC1LiveDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:H264Live-VC1Live", "LiveDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:H264Vod-H264LiveWithText", "H264ODDefault--H264LiveMultiAudioTextWithLargeChunkDuration" )
        PKTEST_PROPERTY("Data:H264LiveWithText-H264VodMultiVideo", "H264LiveMultiAudioTextWithLargeChunkDuration--H264LiveMultiAudioTextWithLargeChunkDuration" )
        PKTEST_PROPERTY("Data:H264ChangeSourcesWithDifferentAudioBitrates", "LiveDefault--H264ODMultiAudio2")
        PKTEST_PROPERTY("Data:H264ODMultiAudio-H264Live", "H264ODMultiAudio--LiveDefault")
        PKTEST_PROPERTY("Data:DigitalRapids-H264Live", "H264ODDigitalRapids--LiveDefault")
        PKTEST_PROPERTY("Data:H264Live-H24Envivio", "LiveDefault--H264ODEnvivio")
        PKTEST_PROPERTY("Data:LiveProtected-ProtectedVod", "LiveProtectedPiff1.3ShortKR--ODProtectedPiff1.3KROnEveryFragment")
        PKTEST_PROPERTY("Data:LiveProtected-H264Vod", "LiveProtectedPiff1.3ShortKR--H264ODDefault")
        PKTEST_PROPERTY("Data:LiveProtected-H264Live", "LiveProtectedPiff1.3ShortKR--LiveDefault")
        PKTEST_PROPERTY("Data:ProtectedVod-ProtectedLive", "ODProtectedPiff1.3KROnEveryFragment--LiveProtectedPiff1.3ShortKR")
        PKTEST_PROPERTY("Data:ProtectedVod-H264Vod", "ODProtectedPiff1.3KROnEveryFragment--H264ODDefault")
        PKTEST_PROPERTY("Data:ProtectedVod-H264Live", "ODProtectedPiff1.3KROnEveryFragment--LiveDefault")
        PKTEST_PROPERTY("Data:H264Vod-ProtectedVod", "LiveDefault--ODProtectedPiff1.3KROnEveryFragment")
        PKTEST_PROPERTY("Data:H264Live-ProtectedLive", "LiveDefault--LiveProtectedPiff1.3ShortKR")
        PKTEST_PROPERTY("Data:H264OD-ProtectedOD", "H264ODDefault--ODProtectedPiff1.3KROnEveryFragment")
        PKTEST_PROPERTY("Data:H264OD-ProtectedLive", "H264ODDefault--LiveProtectedPiff1.3ShortKR")
        PKTEST_PROPERTY("Keyword", "ChannelSwitch"))
        {
        uint64_t startTime = 0;
        uint64_t endTime = 0;
        string firstSource;
        string secondSource;
        bool firstSourceWasPlaying = false;

        PKTEST_FUNC_EXIT( GetSourceStrings(PKTest_GetTestData(), &firstSource, &secondSource) );
        
        //not using this startTime
        PKTEST_FUNC_EXIT( OpenMediaAndPlay(firstSource, &startTime) );
        
        firstSourceWasPlaying = ( 0 != _smoothObject->GetStatusUpdatesList().Playing );

        startTime = STUtilities::GetSystemTime();

        _smoothObject->DestroySmoothTransport();
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetStatusUpdatesList().Closed == (firstSourceWasPlaying ? 1 : 0), "There should be %d Closed Event, but got %d", (firstSourceWasPlaying ? 1 : 0), _smoothObject->GetStatusUpdatesList().Closed);

        delete _smoothObject;
        

        _smoothObject = NEW_NO_THROW STWrapper();
        PKTEST_ASSERT_EXIT(NULL != _smoothObject);
        
        PKTEST_FUNC_EXIT( OpenMediaAndPlay(secondSource, &endTime) );

        LogTestComment("Time took for channel switch is : %.3f", ((double)(endTime - startTime)) / TIMESCALE_10MHZ);

    exit:
        return;
    }

    PKTEST_METHOD_EX(
        ContentSwitchSourcesWithClose,
        PKTEST_PROPERTY("Data:NonExistentManifest-H264Vod", "NonExistentManifest--H264ODDefault" )
        PKTEST_PROPERTY("Data:H264Vod-NonExistentManifest", "H264ODDefault--NonExistentManifest" )
        PKTEST_PROPERTY("Data:H264Vod-H264Vod", "H264ODDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:H264Vod-H264Live", "H264ODDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:H264Live-H264Vod", "LiveDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:H264Live-H264Live", "LiveDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-VC1Vod", "VC1ODDefault--VC1ODDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-VC1Live", "VC1ODDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:VC1Live-VC1Vod", "VC1LiveDefault--VC1ODDefault" )
        PKTEST_PROPERTY("Data:VC1Live-VC1Live","VC1LiveDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:H264Live-VC1Vod", "LiveDefault--VC1ODDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-H264Vod", "VC1ODDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-H264Live", "VC1ODDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:H264Vod-VC1Live", "H264ODDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:VC1Live-H264Vod", "VC1LiveDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:VC1Live-H264Live", "VC1LiveDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:H264Live-VC1Live", "LiveDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:H264Vod-H264LiveWithText", "H264ODDefault--H264LiveMultiAudioTextWithLargeChunkDuration" )
        PKTEST_PROPERTY("Data:H264LiveWithText-H264VodMultiVideo", "H264LiveMultiAudioTextWithLargeChunkDuration--H264LiveMultiAudioTextWithLargeChunkDuration" )
        PKTEST_PROPERTY("Data:H264ChangeSourcesWithDifferentAudioBitrates", "LiveDefault--H264ODMultiAudio2")
        PKTEST_PROPERTY("Data:H264ODMultiAudio-H264Live", "H264ODMultiAudio--LiveDefault")
        PKTEST_PROPERTY("Data:DigitalRapids-H264Live", "H264ODDigitalRapids--LiveDefault")
        PKTEST_PROPERTY("Data:H264Live-H24Envivio", "LiveDefault--H264ODEnvivio")
        PKTEST_PROPERTY("Data:LiveProtected-ProtectedVod", "LiveProtectedPiff1.3ShortKR--ODProtectedPiff1.3KROnEveryFragment")
        PKTEST_PROPERTY("Data:LiveProtected-H264Vod", "LiveProtectedPiff1.3ShortKR--H264ODDefault")
        PKTEST_PROPERTY("Data:LiveProtected-H264Live", "LiveProtectedPiff1.3ShortKR--LiveDefault")
        PKTEST_PROPERTY("Data:ProtectedVod-ProtectedLive", "ODProtectedPiff1.3KROnEveryFragment--LiveProtectedPiff1.3ShortKR")
        PKTEST_PROPERTY("Data:ProtectedVod-H264Vod", "ODProtectedPiff1.3KROnEveryFragment--H264ODDefault")
        PKTEST_PROPERTY("Data:ProtectedVod-H264Live", "ODProtectedPiff1.3KROnEveryFragment--LiveDefault")
        PKTEST_PROPERTY("Data:H264Vod-ProtectedVod", "LiveDefault--ODProtectedPiff1.3KROnEveryFragment")
        PKTEST_PROPERTY("Data:H264Live-ProtectedLive", "LiveDefault--LiveProtectedPiff1.3ShortKR")
        PKTEST_PROPERTY("Data:H264OD-ProtectedOD", "H264ODDefault--ODProtectedPiff1.3KROnEveryFragment")
        PKTEST_PROPERTY("Data:H264OD-ProtectedLive", "H264ODDefault--LiveProtectedPiff1.3ShortKR")
        PKTEST_PROPERTY("Keyword", "ChannelSwitch")
        )
    {
        uint64_t startTime = 0;
        uint64_t endTime = 0;
        string firstSource;
        string secondSource;
        bool firstSourceWasPlaying = false;

        PKTEST_FUNC_EXIT( GetSourceStrings(PKTest_GetTestData(), &firstSource, &secondSource) );

        PKTEST_FUNC_EXIT( OpenMediaAndPlay(firstSource, &startTime) );
        
        firstSourceWasPlaying = ( 0 != _smoothObject->GetStatusUpdatesList().Playing );

        startTime = STUtilities::GetSystemTime();

        PKTEST_HRESULT_EXIT( _smoothObject->Close() );

        PKTEST_FUNC_EXIT( OpenMediaAndPlay(secondSource, &endTime) );
        
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetStatusUpdatesList().Closed == (firstSourceWasPlaying ? 1 : 0), "There should be %d Closed Event, but got %d", (firstSourceWasPlaying ? 1 : 0), _smoothObject->GetStatusUpdatesList().Closed);

        LogTestComment("Time took for channel switch is : %.3f", ((double)(endTime - startTime)) / TIMESCALE_10MHZ);

    exit:
        return;
    }

    PKTEST_METHOD_EX(
        ContentSwitchSourcesWithOutClose,
        PKTEST_PROPERTY("Data:NonExistentManifest-H264Vod", "NonExistentManifest--H264ODDefault" )
        PKTEST_PROPERTY("Data:H264Vod-NonExistentManifest", "H264ODDefault--NonExistentManifest" )
        PKTEST_PROPERTY("Data:H264Vod-H264Vod", "H264ODDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:H264Vod-H264Live", "H264ODDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:H264Live-H264Vod", "LiveDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:H264Live-H264Live", "LiveDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-VC1Vod", "VC1ODDefault--VC1ODDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-VC1Live", "VC1ODDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:VC1Live-VC1Vod", "VC1LiveDefault--VC1ODDefault" )
        PKTEST_PROPERTY("Data:VC1Live-VC1Live","VC1LiveDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:H264Live-VC1Vod", "LiveDefault--VC1ODDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-H264Vod", "VC1ODDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:VC1Vod-H264Live", "VC1ODDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:H264Vod-VC1Live", "H264ODDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:VC1Live-H264Vod", "VC1LiveDefault--H264ODDefault" )
        PKTEST_PROPERTY("Data:VC1Live-H264Live", "VC1LiveDefault--LiveDefault" )
        PKTEST_PROPERTY("Data:H264Live-VC1Live", "LiveDefault--VC1LiveDefault" )
        PKTEST_PROPERTY("Data:H264Vod-H264LiveWithText", "H264ODDefault--H264LiveMultiAudioTextWithLargeChunkDuration" )
        PKTEST_PROPERTY("Data:H264LiveWithText-H264VodMultiVideo", "H264LiveMultiAudioTextWithLargeChunkDuration--H264LiveMultiAudioTextWithLargeChunkDuration" )
        PKTEST_PROPERTY("Data:H264ChangeSourcesWithDifferentAudioBitrates", "LiveDefault--H264ODMultiAudio2")
        PKTEST_PROPERTY("Data:H264ODMultiAudio-H264Live", "H264ODMultiAudio--LiveDefault")
        PKTEST_PROPERTY("Data:DigitalRapids-H264Live", "H264ODDigitalRapids--LiveDefault")
        PKTEST_PROPERTY("Data:H264Live-H24Envivio", "LiveDefault--H264ODEnvivio")
        PKTEST_PROPERTY("Data:LiveProtected-ProtectedVod", "LiveProtectedPiff1.3ShortKR--ODProtectedPiff1.3KROnEveryFragment")
        PKTEST_PROPERTY("Data:LiveProtected-H264Vod", "LiveProtectedPiff1.3ShortKR--H264ODDefault")
        PKTEST_PROPERTY("Data:LiveProtected-H264Live", "LiveProtectedPiff1.3ShortKR--LiveDefault")
        PKTEST_PROPERTY("Data:ProtectedVod-ProtectedLive", "ODProtectedPiff1.3KROnEveryFragment--LiveProtectedPiff1.3ShortKR")
        PKTEST_PROPERTY("Data:ProtectedVod-H264Vod", "ODProtectedPiff1.3KROnEveryFragment--H264ODDefault")
        PKTEST_PROPERTY("Data:ProtectedVod-H264Live", "ODProtectedPiff1.3KROnEveryFragment--LiveDefault")
        PKTEST_PROPERTY("Data:H264Vod-ProtectedVod", "LiveDefault--ODProtectedPiff1.3KROnEveryFragment")
        PKTEST_PROPERTY("Data:H264Live-ProtectedLive", "LiveDefault--LiveProtectedPiff1.3ShortKR")
        PKTEST_PROPERTY("Data:H264OD-ProtectedOD", "H264ODDefault--ODProtectedPiff1.3KROnEveryFragment")
        PKTEST_PROPERTY("Data:H264OD-ProtectedLive", "H264ODDefault--LiveProtectedPiff1.3ShortKR")
        PKTEST_PROPERTY("Keyword", "ChannelSwitch")
        )
    {
        uint64_t startTime = 0;
        uint64_t endTime = 0;
        string firstSource;
        string secondSource;
        bool firstSourceWasPlaying = false;

        PKTEST_FUNC_EXIT( GetSourceStrings(PKTest_GetTestData(), &firstSource, &secondSource) );
        
        PKTEST_FUNC_EXIT( OpenMediaAndPlay(firstSource, &startTime) );
       
        firstSourceWasPlaying = ( 0 != _smoothObject->GetStatusUpdatesList().Playing );

        startTime = STUtilities::GetSystemTime();

        PKTEST_FUNC_EXIT( OpenMediaAndPlay(secondSource, &endTime) );

        PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetStatusUpdatesList().Closed == (firstSourceWasPlaying ? 1 : 0), "There should be %d Closed Event, but got %d", (firstSourceWasPlaying ? 1 : 0), _smoothObject->GetStatusUpdatesList().Closed);

        LogTestComment("Time took for channel switch is : %.3f", ((double)(endTime - startTime)) / TIMESCALE_10MHZ);

    exit:
        return;
    }


    //*********************************************************************************************************************************
    
    void OpenMediaAndPlay(const string& url, uint64_t* systemTimeAfterOpen )
    {
        bool canPlay = wcscmp(Str2WStr(SSPKHelpers::GetAttributeValueFromList(_xManifests, Str2WStr(url).c_str(), L"canPlay")).c_str(), L"True") == 0;
        PKTEST_FUNC_EXIT(OpenMedia(url));

        *systemTimeAfterOpen  = STUtilities::GetSystemTime();

        _smoothObject->Delay(2*VALIDATION_DELAY);
        if(canPlay)
        {
            PKTEST_ASSERT_MSG_EXIT( _smoothObject->IsPlaying(), "We should be in playing state for source %s.", url.c_str() );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT( !_smoothObject->IsPlaying(), "We should not be in playing state for source %s.", url.c_str() );
        }

    exit:
        return;
    }

    void OpenMedia(const string& url)
    {
        //Find out if the content should play
        bool canPlay = wcscmp(Str2WStr(SSPKHelpers::GetAttributeValueFromList(_xManifests, Str2WStr(url).c_str(), L"canPlay")).c_str(), L"True") == 0;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(url).c_str()) );

        PKTEST_ASSERT_MSG_EXIT(!urlString.empty(), "given Url is empty or not found");

        LogTestComment("Source url is : %s", urlString.c_str());
        PKTEST_HRESULT_EXIT(_smoothObject->OpenVideo(urlString, true, canPlay));
        if( canPlay )
        {
            PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout while waiting for the manifest ready for source %s.", url.c_str() );
            PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingStarted(PLAYBACK_STARTUP_TIMEOUT), "Timeout or some failure happened while waiting for media to render for source %s", url.c_str());
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT( !_smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Expected a timeout while waiting for the manifest ready for source %s, but did not encounter one", url.c_str() );
            PKTEST_ASSERT_MSG_EXIT( !_smoothObject->WaitForMediaRenderingStarted(PLAYBACK_STARTUP_TIMEOUT), "Expected timeout or some failure while waiting for media to render for source %s, but encountered neither", url.c_str());
        }

    exit:
        return;
    }

    void GetSourceStrings(string testData, string* firstSource, string* secondSource)
    {
        const string delimiter = "--";

        size_t index = testData.find(delimiter);
        
        PKTEST_ASSERT_MSG_EXIT(index != string::npos, "DataDrivenTest data doesn't have the delimiter.");
        *firstSource = testData.substr(0,index);
        *secondSource = testData.substr(index + delimiter.length());

    exit:
        return;
    }
};
