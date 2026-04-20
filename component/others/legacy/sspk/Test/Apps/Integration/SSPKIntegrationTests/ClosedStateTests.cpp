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

PKTEST_GROUP( ClosedStateTests )
{

    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;

    ClosedStateTests()
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

    void StartSmoothObject( string sourceName, bool autoPlay)
    {
        string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(sourceName).c_str());
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, autoPlay) );

    exit:
        return;
    }

    /////////////// Actions After Close //////////////////////
    //////////////////////////////////////////////////////////
    /// <summary>
    /// Open the source again after close successfully.
    /// </summary>
    /// <priority value=0 /> 
    PKTEST_METHOD_EX(
        OpenAfterManualClose,
        PKTEST_PROPERTY( "Priority", "0" )
        )
    {
        PKTEST_FUNC_EXIT( StartSmoothObject("H264ODMultiAudio", true) );
        PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(CLOSE_TIMEOUT) );
        _smoothObject->Delay(VALIDATION_DELAY);
        
        PKTEST_FUNC_EXIT( StartSmoothObject("H264ODMultiAudio", true) );

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    /// <summary>
    /// Play the source at beginning after close successfully.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX(
        PlayAfterManualClose,
        PKTEST_PROPERTY( "Priority", "1" )
        )
    {
        pkRESULT pkResult = pkS_OK;
        PKTEST_FUNC_EXIT( StartSmoothObject("H264ODMultiAudio", true) );
        PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(CLOSE_TIMEOUT) );
        _smoothObject->Delay(VALIDATION_DELAY);

        pkResult = _smoothObject->Play();
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    /// <summary>
    /// Pause the source after close successfully.
    /// </summary>
    /// <priority value=2 /> 
    PKTEST_METHOD_EX( PauseAfterManualClose,
        PKTEST_PROPERTY( "Priority", "2" )
        )
    {
        pkRESULT pkResult = pkS_OK;
        PKTEST_FUNC_EXIT( StartSmoothObject("H264ODMultiAudio", true) );
        PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(CLOSE_TIMEOUT) );
        _smoothObject->Delay(VALIDATION_DELAY);

        pkResult = _smoothObject->Pause(0);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    /// <summary>
    /// Play the source at a seekable position after close successfully.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( PlayAtAfterManualClose,
        PKTEST_PROPERTY( "Priority", "1" )
        )
    {
        int64_t playAtTime;
        pkRESULT pkResult = pkS_OK;
        PKTEST_FUNC_EXIT( StartSmoothObject("H264ODMultiAudio", true) );
        PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(CLOSE_TIMEOUT) );
        _smoothObject->Delay(VALIDATION_DELAY);

        playAtTime = _smoothObject->GetRandomSeekablePosition();
        pkResult = _smoothObject->PlayAt(playAtTime);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    /// <summary>
    /// Skip the source by 10 seconds after close successfully.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( SkipAfterManualClose,
        PKTEST_PROPERTY( "Priority", "1" )
        )
    {
        pkRESULT pkResult = pkS_OK;
        PKTEST_FUNC_EXIT( StartSmoothObject("H264ODMultiAudio", true) );
        PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(CLOSE_TIMEOUT) );
        _smoothObject->Delay(VALIDATION_DELAY);

        pkResult = _smoothObject->Skip(RAND_SKIP_SECONDS);
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

exit:
        return;
    }

    //////////////////////////////////////////////////////////
    /// <summary>
    /// Seek to a random seekable position after close successfully.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( SeekAfterManualClose,
        PKTEST_PROPERTY( "Priority", "1" )
        )
    {
        int64_t proposedPlayBackTime;
        pkRESULT pkResult = pkS_OK;
        PKTEST_FUNC_EXIT( StartSmoothObject("H264ODMultiAudio", true) );
        PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(CLOSE_TIMEOUT) );
        _smoothObject->Delay(VALIDATION_DELAY);

        proposedPlayBackTime = _smoothObject->GetRandomSeekablePosition();
        LogTestComment("Seeking to : %.3f ", (double)proposedPlayBackTime/ TIMESCALE_10MHZ);
        pkResult = _smoothObject->Seek( proposedPlayBackTime );
        PKTEST_ASSERT_MSG_EXIT(pkE_INVALID_REQUEST == pkResult, "Expected pkE_INVALID_REQUEST, but got 0x%X", pkResult);

exit:
        return;
    }

    //////////////////////////////////////////////////////////
    /// <summary>
    /// Close the source which is already closed successfully.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( CloseAfterManualClose,
        PKTEST_PROPERTY( "Priority", "1" )
        )
    {
        PKTEST_FUNC_EXIT( StartSmoothObject("H264ODMultiAudio", true) );
        PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(CLOSE_TIMEOUT) );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(CLOSE_TIMEOUT) );

    exit:
        return;
    }

    //////////////////////////////////////////////////////////
    /// <summary>
    /// Validate the PlayBackTime is 0 after close successfully.
    /// </summary>
    /// <priority value=1 /> 
    PKTEST_METHOD_EX( GetCurrentTimeAfterManualClose,
        PKTEST_PROPERTY( "Priority", "1" )
        )
    {
        PKTEST_FUNC_EXIT( StartSmoothObject("H264ODMultiAudio", true) );
        PKTEST_FUNC_EXIT(_smoothObject->CloseAndValidate(CLOSE_TIMEOUT) );
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_ASSERT_MSG_EXIT( 0 == _smoothObject->GetCurrentPlayBackTime(), "CurrentTime without open should be 0");

    exit:
        return;
    }

};
