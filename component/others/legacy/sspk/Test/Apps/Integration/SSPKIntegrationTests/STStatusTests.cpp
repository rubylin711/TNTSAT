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

PKTEST_GROUP( STStatusTests )
{

    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;
    static const uint32_t SOCKETERROR_TIMEOUT = 60 * MILLISECONDS_PER_SECOND;

    STStatusTests()
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

    exit:
        return true;
    }

    bool TestCleanup()
    {
        // delete _xManifests


        return true;
    }
    
    int64_t GetMiddleOfDVRWindow()
    {
        return _smoothObject->GetCurrentStartTime() + (_smoothObject->GetCurrentEndTime() - _smoothObject->GetCurrentStartTime()) / 2 ; 
    }

    int64_t GetLowestBitrate()
    {
        int64_t lowestBitrate = 0;

        vector<SmoothTransportStatus> bitRateUpdateList = _smoothObject->GetStatusCallbackVector(SmoothTransportStatus_BitrateChanged);//NEXT!! Figure out how to get the first Playing.  Should it actually be rendering?
        PKTEST_ASSERT_EXIT(bitRateUpdateList.size() > 0);

        lowestBitrate = (atoi(bitRateUpdateList[0]._additionalInfo.c_str()) + 500)/ 1000;

    exit:
        return lowestBitrate;
    }

    /////////////////////// Status Tests //////////////////////////
    ///////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(StatusTests_CloseWithOutOpen,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        _smoothObject = new STWrapper();
        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject, "smoothTransport is not created.");

        PKTEST_HRESULT_EXIT(_smoothObject->Close());
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsClosed(), "smoothTransport should be in closed state.");

    exit:
        delete _smoothObject;
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(StatusTests_DestroyWithoutClose,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        string urlString;
        _smoothObject = new STWrapper();
        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject, "smoothTransport is not created.");
        
        PKTEST_FUNC_EXIT( urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );
        _smoothObject->OpenVideo(urlString,false);

    exit:
        delete _smoothObject;
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(StatusTests_CloseImmediatelyAfterPlay, 
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        string urlString;
        _smoothObject = new STWrapper();
        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject, "smoothTransport is not created.");

        PKTEST_FUNC_EXIT( urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );

        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false, false));
        PKTEST_HRESULT_EXIT(_smoothObject->Play());
        PKTEST_HRESULT_EXIT(_smoothObject->Close());

    exit:
        delete _smoothObject;
        return ;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(StatusTests_SocketTimeoutAsError, 
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        string urlString;
        _smoothObject = new STWrapper();
        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject, "smoothTransport is not created.");

        PKTEST_FUNC_EXIT( urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSocketTimeoutAt10Seconds") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true, false) );

        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_ASSERT_EXIT(_smoothObject->IsPlaying());
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaFailed(SOCKETERROR_TIMEOUT), "Didn't get an error callback for Socket Timeout" );

    exit:
        delete _smoothObject;
        return ;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(StatusTests_PlayAtSpecificValueToCheckForUnderrun,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        string urlString;
        int underrunCount = 0;
        _smoothObject = new STWrapper();

        PKTEST_FUNC_EXIT( urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );

        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true, true) );
        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(60087777) );//This hardcoded value is known to repro an underrun bug

        //Check for underrun
        underrunCount = _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_Underrun);
        PKTEST_ASSERT_MSG_EXIT( 0 == _smoothObject->GetStatusCallbackCount(SmoothTransportStatus_Underrun), "Expected no underruns but found %d", underrunCount );
        
    exit:
        _smoothObject->Close();
        delete _smoothObject;
        return ;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(StatusTests_BitRateDoesNotSinkToLowestAfterSeek,
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        string urlString;
        int64_t postSeekBitrate = 0, preSeekBitrate = 0, lowestBitrate = 0;

        _smoothObject = new STWrapper();

        PKTEST_FUNC_EXIT( urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true, true) );

        PKTEST_FUNC_EXIT(lowestBitrate = GetLowestBitrate());
        LogTestComment("Lowest bitrate : %d", lowestBitrate);

        //Wait for the kBPS to stablize
        _smoothObject->Delay( SHORT_PLAY_DURATION );
        PKTEST_ASSERT_EXIT( _smoothObject->IsPlaying() );
        preSeekBitrate = _smoothObject->GetLatestBitrateKBPS();
        LogTestComment("Pre seek bitrate : %d", preSeekBitrate);

        PKTEST_ASSERT_MSG_EXIT( preSeekBitrate > lowestBitrate, " The bitrate (%d) has not changed since playback began.  Unable to compare pre and post operation bitrates", lowestBitrate );

        //Perform operation and make sure bitrate is not the lowest
        PKTEST_HRESULT_EXIT( _smoothObject->Seek( GetMiddleOfDVRWindow()  ) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media rendered");
        postSeekBitrate = _smoothObject->GetLatestBitrateKBPS();
        LogTestComment("Post seek bitrate : %d", postSeekBitrate);

        //Compare post operation bitrate to lowest bitrate
        PKTEST_ASSERT_MSG_EXIT( postSeekBitrate > lowestBitrate, " The current bitrate (%d) has fallen to the lowest after Seek", lowestBitrate );
    exit:
        _smoothObject->Close();
        delete _smoothObject;
        return;
    }


    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(StatusTests_BitRateDoesNotSinkToLowestAfterFastForward,
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        string urlString;
        int64_t postFastForwardBitrate = 0, preFastForwardBitrate = 0, lowestBitrate = 0;

        _smoothObject = new STWrapper();

        PKTEST_FUNC_EXIT( urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true, true) );

        PKTEST_FUNC_EXIT(lowestBitrate = GetLowestBitrate());
        LogTestComment("Lowest bitrate : %d", lowestBitrate);

        //Wait for the kBPS to stablize
        _smoothObject->Delay( SHORT_PLAY_DURATION );
        PKTEST_ASSERT_EXIT( _smoothObject->IsPlaying() );
        preFastForwardBitrate = _smoothObject->GetLatestBitrateKBPS();
        LogTestComment("Pre FastForward bitrate : %d", preFastForwardBitrate);

        PKTEST_ASSERT_MSG_EXIT( preFastForwardBitrate > lowestBitrate, " The bitrate (%d) has not changed since playback began.  Unable to compare pre and post operation bitrates", lowestBitrate );

        //Perform operation and make sure bitrate is not the lowest
        _smoothObject->MakeRoomForFastForward();
        _smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, STATECHANGE_TIMEOUT);
        postFastForwardBitrate = _smoothObject->GetLatestBitrateKBPS();
        LogTestComment("Post FastForward bitrate : %d", postFastForwardBitrate);

        //Compare post operation bitrate to lowest bitrate
        PKTEST_ASSERT_MSG_EXIT( postFastForwardBitrate > lowestBitrate, " The current bitrate (%d) has fallen to the lowest after FastForward", lowestBitrate );
    exit:
        _smoothObject->Close();
        delete _smoothObject;
        return;
    }

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(StatusTests_BitRateDoesNotSinkToLowestAfterRewind,
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        string urlString;
        int64_t postRewindBitrate = 0, preRewindBitrate = 0, lowestBitrate = 0;

        _smoothObject = new STWrapper();

        PKTEST_FUNC_EXIT( urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true, true) );

        PKTEST_FUNC_EXIT(lowestBitrate = GetLowestBitrate());
        LogTestComment("Lowest bitrate : %d", lowestBitrate);

        //Wait for the kBPS to stablize
        _smoothObject->Delay( SHORT_PLAY_DURATION );
        PKTEST_ASSERT_EXIT( _smoothObject->IsPlaying() );
        preRewindBitrate = _smoothObject->GetLatestBitrateKBPS();
        LogTestComment("Pre Rewind bitrate : %d", preRewindBitrate);

        PKTEST_ASSERT_MSG_EXIT( preRewindBitrate > lowestBitrate, " The bitrate (%d) has not changed since playback began.  Unable to compare pre and post operation bitrates", lowestBitrate );

        //Perform operation and make sure bitrate is not the lowest
        _smoothObject->MakeRoomForRewind();
        _smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, STATECHANGE_TIMEOUT);
        postRewindBitrate = _smoothObject->GetLatestBitrateKBPS();
        LogTestComment("Post Rewind bitrate : %d", postRewindBitrate);

        //Compare post operation bitrate to lowest bitrate
        PKTEST_ASSERT_MSG_EXIT( postRewindBitrate > lowestBitrate, " The current bitrate (%d) has fallen to the lowest after Rewind", lowestBitrate );
    exit:
        _smoothObject->Close();
        delete _smoothObject;
        return;
    }



};
