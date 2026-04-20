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

PKTEST_GROUP( ContentPlayingTests )
{

    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;
    
    static const int32_t PLAYBACK_VALIDATION_DURATION = 10 * 1000; //10 SECONDS
    
    ContentPlayingTests()
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

        return true;
    exit:
        return false;
    }

    PKTEST_METHOD_EX(
        PlayContentFor10s,
        PKTEST_PROPERTY("Priority","1")
        )
    {
        string attributeValue = "";
        for(int32_t index = 0; index < _xManifests.Length(); index++)
        {
            CXMLAttributesList currentElementAttributes = _xManifests[index].Attributes();
            if(wcscmp(currentElementAttributes[L"canPlay"].Value(), L"True") == 0)
            {
                attributeValue = escapeSpaces( wstring_to_string( currentElementAttributes[L"url"].Value() ) );
                
                PlayContentAndExit(attributeValue, index);

            }

        }
    }

    void PlayContentAndExit(string urlString, int32_t index)
    {   
        CXMLAttributesList currentElementAttributes;
        const char *extendedCommandArgBuffer;
        const wchar_t* wProxyValue;
        _smoothObject = NEW_NO_THROW STWrapper();
        string proxyString;
        PKTEST_ASSERT_MSG_EXIT( NULL != _smoothObject, "Unable to start smooth streaming object");

        //Check for other attributes that may be needed for setup
        currentElementAttributes = _xManifests[index].Attributes();
        wProxyValue = currentElementAttributes[L"proxy"].Value();
        proxyString = WStr2Str(wProxyValue);
        extendedCommandArgBuffer = proxyString.c_str();
        _smoothObject->SendExtendedCommand(EXC_HTTPPROXY, 1, &extendedCommandArgBuffer);

        PKTEST_MSG("Opening urlString %s", urlString.c_str()); 
        PKTEST_FUNC_EXIT( _smoothObject->OpenVideoAndValidate(urlString, true) );

        _smoothObject->Delay(PLAYBACK_VALIDATION_DURATION);
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Url %s is not in playing state", urlString.c_str());

    exit:

        if(_smoothObject != NULL)
        {
            _smoothObject->Close();
            delete _smoothObject;
        }
        return;
    }

};
