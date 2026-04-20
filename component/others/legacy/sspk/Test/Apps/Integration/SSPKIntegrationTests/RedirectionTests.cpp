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

PKTEST_GROUP( RedirectionTests )
{

    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;
    
    static const int32_t PLAYBACK_VALIDATION_DURATION = 10 * 1000; //10 SECONDS
    
    RedirectionTests()
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
    
    bool StartSmoothObject(string url)
    {
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(url, true));
        
        return true;
    exit:
        return false;
    }

    //Tests
    PKTEST_METHOD_EX(
        PlayRedirectionContent,
        PKTEST_PROPERTY("Priority", "0")
        PKTEST_PROPERTY("Data:RegularManifestWithQueryStringNoRedirect", "RegularManifestWithQueryStringNoRedirect")
        PKTEST_PROPERTY("Data:RegularManifestWithQueryStringRedirectNoQueryString", "RegularManifestWithQueryStringRedirectNoQueryString")
        PKTEST_PROPERTY("Data:RegularManifestWithQuerystringRedirectWithQueryString", "RegularManifestWithQuerystringRedirectWithQueryString")
        PKTEST_PROPERTY("Data:RegularManifestNotISMOrISMLFile", "RegularManifestNotISMOrISMLFile")
        PKTEST_PROPERTY("Data:RedirectedLiveSource", "RedirectedLiveSource")
        )
    {
        string urlString = SSPKHelpers::GetUrlFromList( _xManifests, Str2WStr(PKTest_GetTestData()).c_str());
        LogTestComment("URL: %s", urlString.c_str());

        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        _smoothObject->Delay(PLAYBACK_VALIDATION_DURATION);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Url %s is not in playing state", urlString.c_str());

    exit:
        return;
    }

    PKTEST_METHOD_EX(
        FastForwardOnRedirectedLiveSource,
        PKTEST_PROPERTY("Priority", "2"))
    {
        string urlString = SSPKHelpers::GetUrlFromList( _xManifests, L"RedirectedLiveSource");
        LogTestComment("URL: %s", urlString.c_str());
        PKTEST_FUNC_EXIT(StartSmoothObject(urlString));
        _smoothObject->RewindAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY);
        _smoothObject->FastForwardAndValidate(DEFAULT_TRICK_PLAY_SPEED, VALIDATION_DELAY);
    exit:
        return;
    }
};
