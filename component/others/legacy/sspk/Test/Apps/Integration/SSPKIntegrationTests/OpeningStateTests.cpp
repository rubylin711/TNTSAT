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
#include "STUtilities.h"
#include "ManifestReadyCallbackImpl.h"
#include "StringUtils.h"
#include "SSPKHelpers.h"
#include <sstream>
#include <algorithm>

using namespace SSPK;
using namespace SSPKTest;

PKTEST_GROUP( OpeningStateTests )
{

    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;

    OpeningStateTests()
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

    ////////////// Actions During ManifestReady //////////////
    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( OpenDuringManifestReady,
        PKTEST_PROPERTY( "Priority", "2" )
        PKTEST_PROPERTY("Ignore", "true")
        PKTEST_PROPERTY("Bug", "27080")
        )
    {
        _smoothObject->SetManifestReadyMethod( this, &OpeningStateTests::OpenDuringManifestReady_Method );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void OpenDuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveDefault") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return ;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        PlayDuringManifestReady,
        PKTEST_PROPERTY( "Priority", "2" )
        )
    {
        _smoothObject->SetManifestReadyMethod( this, &OpeningStateTests::PlayDuringManifestReady_Method );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void PlayDuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        pkRESULT pkResult = _smoothObject->Play();
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( PauseDuringManifestReady,
        PKTEST_PROPERTY( "Priority", "2" )
        )
    {
        _smoothObject->SetManifestReadyMethod( this, &OpeningStateTests::PauseDuringManifestReady_Method );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void PauseDuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        pkRESULT pkResult = _smoothObject->Pause(0);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( PlayAtDuringManifestReady,
        PKTEST_PROPERTY( "Priority", "2" )
        )
    {
        _smoothObject->SetManifestReadyMethod( this, &OpeningStateTests::PlayAtDuringManifestReady_Method );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void PlayAtDuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        int64_t playAtTime = _smoothObject->GetRandomSeekablePosition();
        
        pkRESULT pkResult = _smoothObject->PlayAt(playAtTime);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( SkipDuringManifestReady,
        PKTEST_PROPERTY( "Priority", "2" )
        )
    {
        _smoothObject->SetManifestReadyMethod( this, &OpeningStateTests::SkipDuringManifestReady_Method );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void SkipDuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        pkRESULT pkResult = _smoothObject->Skip(RAND_SKIP_SECONDS);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( SeekDuringManifestReady,
        PKTEST_PROPERTY( "Priority", "2" )
        )
    {
        _smoothObject->SetManifestReadyMethod( this, &OpeningStateTests::SeekDuringManifestReady_Method );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void SeekDuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        int64_t proposedPlayBackTime = _smoothObject->GetRandomSeekablePosition();
        LogTestComment("Seeking to : %.3f ", (double)(proposedPlayBackTime)/ TIMESCALE_10MHZ);
        pkRESULT pkResult = _smoothObject->Seek( proposedPlayBackTime );
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( CloseDuringManifestReady,
        PKTEST_PROPERTY( "Priority", "2" )
        PKTEST_PROPERTY("Ignore", "true")
        PKTEST_PROPERTY("Bug", "27080")
        )
    {
        _smoothObject->SetManifestReadyMethod( this, &OpeningStateTests::CloseDuringManifestReady_Method );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void CloseDuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        _smoothObject->Close();
        //We should add appropriate expected error.
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( GetCurrentTimeDuringManifestReady,
        PKTEST_PROPERTY( "Priority", "2" )
        PKTEST_PROPERTY("Ignore", "true")
        PKTEST_PROPERTY("Bug", "27080")
        )
    {
        _smoothObject->SetManifestReadyMethod( this, &OpeningStateTests::GetCurrentTimeDuringManifestReady_Method );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

    exit:
        return;
    }
    void GetCurrentTimeDuringManifestReady_Method(IManifest* pManifest, pkRESULT hr)
    {
        PKTEST_ASSERT_MSG_EXIT( 0 == _smoothObject->GetCurrentPlayBackTime(), "CurrentTime during ManifestReady should be 0");

    exit:
        return;
    
    }


    ////////////// Actions WithOut Open //////////////////////
    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        PlayWithOutOpen,
        PKTEST_PROPERTY( "Priority", "2" )
        )
    {
        pkRESULT pkResult = pkS_OK;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        pkResult = _smoothObject->Play();
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( PauseWithOutOpen,
        PKTEST_PROPERTY( "Priority", "2" )
        )
    {
        pkRESULT pkResult = pkS_OK;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        pkResult = _smoothObject->Pause(0);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( PlayAtWithOutOpen,
        PKTEST_PROPERTY( "Priority", "2" )
        )
    {
        pkRESULT pkResult = pkS_OK;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        pkResult = _smoothObject->PlayAt(0);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( SkipWithOutOpen,
        PKTEST_PROPERTY( "Priority", "2" )
        )
    {
        pkRESULT pkResult = pkS_OK;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        
        pkResult = _smoothObject->Skip(RAND_SKIP_SECONDS);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( SeekWithOutOpen,
        PKTEST_PROPERTY( "Priority", "2" )
        )
    {
        pkRESULT pkResult = pkS_OK;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        pkResult = _smoothObject->Seek(0);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( CloseWithOutOpen,
        PKTEST_PROPERTY( "Priority", "1" )
        )
    {
        pkRESULT pkResult = pkS_OK;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        pkResult = _smoothObject->Close();
        PKTEST_ASSERT_MSG_EXIT(pkS_OK == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( GetCurrentTimeWithOutOpen,
        PKTEST_PROPERTY( "Priority", "1" )
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        PKTEST_ASSERT_MSG_EXIT( 0 == _smoothObject->GetCurrentPlayBackTime(), "CurrentTime without open should be 0");

    exit:
        return;
    }


    ////////// Actions After Open Without AutoPlay ///////////
    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        PlayAfterOpenWithAutoPlayFalse,
        PKTEST_PROPERTY( "Priority", "0" )
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

        PKTEST_FUNC_EXIT( _smoothObject->PlayAndValidate(VALIDATION_DELAY) );

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( PauseAfterOpenWithAutoPlayFalse,
        PKTEST_PROPERTY( "Priority", "1" )
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

        PKTEST_FUNC_EXIT( _smoothObject->PauseAndValidate(VALIDATION_DELAY) );

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( PlayAtAfterOpenWithAutoPlayFalseOD,
        PKTEST_PROPERTY( "Priority", "0" )
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        )
    {
        int64_t proposedPlaybackTime, currentPlaybackTime, diff;
        
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

        //Use manifest version because clock has not started, GetRandomSeekablePosition will always return 0
        proposedPlaybackTime = _smoothObject->GetRandomSeekablePositionFromManifest();

        PKTEST_HRESULT_EXIT( _smoothObject->PlayAt(proposedPlaybackTime) );
        
        //Validate in the test since PlayAtAndValidate assumes that playback has already begun.
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingStarted( STATECHANGE_TIMEOUT ), "Timeout occured waiting for media rendering" );
        currentPlaybackTime = _smoothObject->GetCurrentPlayBackTime();
        diff = currentPlaybackTime > proposedPlaybackTime ? currentPlaybackTime - proposedPlaybackTime: proposedPlaybackTime - currentPlaybackTime;
        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(proposedPlaybackTime, currentPlaybackTime, _smoothObject->GetNonAccurateSeekThreshold()), "Current time After PlayAt (%lld) does not match given PlayAt time (%lld) with Threshold : %lld.  Actual difference is %lld (%.3f seconds)", proposedPlaybackTime, proposedPlaybackTime, _smoothObject->GetNonAccurateSeekThreshold(), diff, TimeSpan_hns::FromTicks(diff).ToSeconds() );

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( PlayAtAfterOpenWithAutoPlayFalseLive,
        PKTEST_PROPERTY( "Priority", "0" )
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault")
        )
    {
        int64_t proposedPlaybackTime, currentPlaybackTime, diff;
        
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

        //Use manifest version because clock has not started, GetRandomSeekablePosition will always return 0
        proposedPlaybackTime = _smoothObject->GetRandomSeekablePositionFromManifest();

        PKTEST_HRESULT_EXIT( _smoothObject->PlayAt(proposedPlaybackTime) );        
        
        //Validate in the test since PlayAtAndValidate assumes that playback has already begun.
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingStarted( STATECHANGE_TIMEOUT ), "Timeout occured waiting for media rendering" );
        currentPlaybackTime = _smoothObject->GetCurrentPlayBackTime();
        diff = currentPlaybackTime > proposedPlaybackTime ? currentPlaybackTime - proposedPlaybackTime: proposedPlaybackTime - currentPlaybackTime;
        PKTEST_ASSERT_MSG_EXIT( CompareTimeStamps(proposedPlaybackTime, currentPlaybackTime, _smoothObject->GetNonAccurateSeekThreshold()), "Current time After PlayAt (%lld) does not match given PlayAt time (%lld) with Threshold : %lld.  Actual difference is %lld (%.3f seconds)", proposedPlaybackTime, proposedPlaybackTime, _smoothObject->GetNonAccurateSeekThreshold(), diff, TimeSpan_hns::FromTicks(diff).ToSeconds() );


    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( SkipAfterOpenWithAutoPlayFalse,
        PKTEST_PROPERTY( "Priority", "1" )
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault")
        )
    {
        pkRESULT pkResult = pkS_OK;
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );
        
        pkResult = _smoothObject->Skip(RAND_SKIP_SECONDS);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);
    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( SeekAfterOpenWithAutoPlayFalse,
        PKTEST_PROPERTY( "Priority", "1" )
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault")
        )
    {
        int64_t proposedPlayBackTime;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

        proposedPlayBackTime = _smoothObject->GetRandomSeekablePosition();

        PKTEST_FUNC_EXIT( _smoothObject->SeekAndValidate(proposedPlayBackTime) );

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( CloseAfterOpenWithAutoPlayFalse,
        PKTEST_PROPERTY( "Priority", "1" )
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

        PKTEST_FUNC_EXIT( _smoothObject->CloseAndValidate(VALIDATION_DELAY) );

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( GetCurrentTimeAfterOpenWithAutoPlayFalse,
        PKTEST_PROPERTY( "Priority", "1" )
        PKTEST_PROPERTY("Data:H264ODDefault", "H264ODDefault")
        PKTEST_PROPERTY("Data:LiveDefault", "LiveDefault")
        )
    {
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData()) );
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, false) );

        PKTEST_ASSERT_MSG_EXIT( 0 == _smoothObject->GetCurrentPlayBackTime(), "CurrentTime without open should be 0");

    exit:
        return;
    }
    
};
