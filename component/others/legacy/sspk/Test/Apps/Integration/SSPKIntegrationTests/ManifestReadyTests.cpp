///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h" // includes pkPAL.h
#include "pkSockets.h"
#include "pkExecutive.h"
#include "pkTestFramework.h"
#include "PKTestSuite.h"
#include "PKTestSuiteUtils.h"
#include "SSPKHelpers.h"

#include "ISmoothTransport.h"
#include "STWrapper.h"
#include "ManifestReadyCallbackImpl.h"
#include "StringUtils.h"

using namespace SSPKTest;
using namespace SSPK;


PKTEST_GROUP( ManifestReadyTests )
{
    STWrapper               *m_pSTWObject;
    CXMLElementsList        m_Manifests;

    ManifestReadyTests()
    {
        CXMLElement xTestData = PKTest_GetDataTable( "IntegrationTestSources" );
        if( !xTestData.IsNull() )
        {
             m_Manifests = xTestData.Elements(L"source");
        }
    }

    bool TestSetup()
    {
        PKTEST_ASSERT_MSG_EXIT(m_Manifests.Length() > 0,"Couldn't find any elements with name 'source'");
        
        m_pSTWObject = NEW_NO_THROW STWrapper();
        PKTEST_ASSERT_MSG_EXIT( NULL != m_pSTWObject, "Unable to start smooth streaming object");

        return true;
exit:
        return false;
    }

    bool TestCleanup()
    {
        PKTEST_ASSERT_EXIT(NULL != m_pSTWObject);

        m_pSTWObject->SetManifestCallback(NULL);
        
        if(!m_pSTWObject->IsClosed())
        {
            m_pSTWObject->Close();
        }

        delete m_pSTWObject;
        m_pSTWObject = NULL;

        return true;
exit:
        return false;
    }

    ////////////////////////////////////
    //  Manifest Ready Callbacks
    ////////////////////////////////////

    ///Verification: Tuning and Playback does not happen until after manifest ready callback is done.
    ///
    void ManifestReadyTestAutoPlayOn(IManifest* pManifest, pkRESULT hr)
    {
        PKTEST_MSG( "[%.3f] Successfully get the manifest ready Event on AutoPlayOn.\n", 
                    (double)Executive_GetTickCount() / 1000.0 );

        if( m_pSTWObject->GetStatusUpdatesList().Tuning ||
            m_pSTWObject->GetStatusUpdatesList().Playing )
        {
            PKTEST_HRESULT_MSG_EXIT( E_FAIL, 
                                     "[%.3f] Failed, AutoPlayOn, Playing on ManifestReady!", 
                                     (double)Executive_GetTickCount() / 1000.0);
        }
        else
        {
            PKTEST_MSG("[%.3f] Successful. AutoPlayOn \n",
                        (double)Executive_GetTickCount() / 1000.0);
        }

    exit:
        return ;
    }

    ///Verification: Tuning and Playback does not happen until after manifest ready callback is done.
    ///
    void ManifestReadyTestAutoPlayOff(IManifest* pManifest, pkRESULT hr)
    {
        PKTEST_MSG(  "[%.3f] Successfully get the manifest ready Event on AutoPlayOff.\n",
                    (double)Executive_GetTickCount() / 1000.0
                 );

        if( m_pSTWObject->GetStatusUpdatesList().Tuning ||
            m_pSTWObject->GetStatusUpdatesList().Playing )
        {
            PKTEST_HRESULT_MSG_EXIT( E_FAIL,
                                     "[%.3f] Failed, AutoPlayOff, Tuning, Playing on ManifestReady !! \n",
                                     (double)Executive_GetTickCount() / 1000.0);
        }
        else
        {
            PKTEST_MSG("[%.3f] Successful. AutoPlayOff. \n",
                        (double)Executive_GetTickCount() / 1000.0);
        }
    exit:
        return ;
    }

    ///Verification: No Status Update On Manifest Ready
    ///
    void NoStatusUpdateOnManifestReady(IManifest* pManifest, pkRESULT hr)
    {
        if( m_pSTWObject->GetStatusUpdatesList().Tuning ||
            m_pSTWObject->GetStatusUpdatesList().Playing ||
            m_pSTWObject->GetStatusUpdatesList().Paused ||
            m_pSTWObject->GetStatusUpdatesList().MediaEnded ||
            m_pSTWObject->GetStatusUpdatesList().Detuned ||
            m_pSTWObject->GetStatusUpdatesList().Closed )
        {
            PKTEST_HRESULT_MSG_EXIT( E_FAIL,
                                     "[%.3f] Failed, Unexpected status updates on ManifestReady! \n",
                                     (double)Executive_GetTickCount() / 1000.0 );
        }
        else
        {
            PKTEST_MSG("[%.3f] Successful. No status updates on ManifestReady. \n",
                        (double)Executive_GetTickCount() / 1000.0);
        }

    exit:
        return ;
    }

    ///Verification: Available Streams on Manifest Ready
    ///
    void ManifestReadyTestAvailableStreams(IManifest* pManifest, pkRESULT hr)
    {
        VectorOfStreams availableStreams;

        PKTEST_HRESULT_EXIT( pManifest->GetAvailableStreams( &availableStreams ) );
        PKTEST_ASSERT_EXIT( availableStreams.size() != 0 );

        PKTEST_FUNC_EXIT( SSPKHelpers::CheckStreamCountByType(availableStreams, 2, 2, 0) );

    exit:
        return ;
    }

    ///Verification: Selected Streams on Manifest Ready
    ///
    void ManifestReadyTestSelectedStreams(IManifest* pManifest, pkRESULT hr)
    {
        VectorOfStreams selectedStreams;

        PKTEST_HRESULT_EXIT( pManifest->GetSelectedStreams( &selectedStreams ) );
        PKTEST_ASSERT_EXIT( selectedStreams.size() != 0 );

        PKTEST_FUNC_EXIT( SSPKHelpers::CheckStreamCountByType( selectedStreams, 1, 1, 0 ) );
exit:
        return ;
    }

    ////////////////////////////////////
    //  Test Cases
    ////////////////////////////////////

    PKTEST_METHOD_EX( CheckManifestReadyFiredBeforePlaybackAutoPlayOff,
        PKTEST_PROPERTY("Priority", "0")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &ManifestReadyTests::ManifestReadyTestAutoPlayOff );

        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODDefault") );

        PKTEST_FUNC_EXIT( m_pSTWObject->OpenVideoAndValidate( urlString, false, true ) );

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD_EX( CheckManifestReadyFiredBeforePlaybackAutoPlayOn,
        PKTEST_PROPERTY("Priority", "0")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &ManifestReadyTests::ManifestReadyTestAutoPlayOn );

        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODDefault") );

        PKTEST_FUNC_EXIT( m_pSTWObject->OpenVideoAndValidate( urlString, true, true) );

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD_EX( CheckNoStatusUpdateOnManifestReadyAutoPlayOff,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &ManifestReadyTests::NoStatusUpdateOnManifestReady );

        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODDefault") );

        PKTEST_FUNC_EXIT( m_pSTWObject->OpenVideoAndValidate( urlString, false, true) );
    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD_EX( CheckNoStatusUpdateOnManifestReadyAutoPlayOn,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &ManifestReadyTests::NoStatusUpdateOnManifestReady );
        
        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"H264ODDefault") );

        PKTEST_FUNC_EXIT( m_pSTWObject->OpenVideoAndValidate( urlString, true, true) );

    exit:
        return;
    }

    PKTEST_METHOD_EX( VerifyAvailableStreamsAutoPlayOff,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &ManifestReadyTests::ManifestReadyTestAvailableStreams );
        
        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"VC1ODMultiAudioMultiVideo") );

        PKTEST_FUNC_EXIT( m_pSTWObject->OpenVideoAndValidate( urlString, false, true ) );
        
    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD_EX( VerifyAvailableStreamsAutoPlayOn,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &ManifestReadyTests::ManifestReadyTestAvailableStreams );
        
        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"VC1ODMultiAudioMultiVideo") );

        PKTEST_FUNC_EXIT( m_pSTWObject->OpenVideoAndValidate( urlString, true, true) );

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD_EX( VerifySelectedStreamsAutoPlayOff,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &ManifestReadyTests::ManifestReadyTestSelectedStreams );
        
        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"VC1ODMultiAudioMultiVideo") );

        PKTEST_FUNC_EXIT( m_pSTWObject->OpenVideoAndValidate( urlString, false, true) );

    exit:
        return;
    }

    PKTEST_METHOD_EX( VerifySelectedStreamsAutoPlayOn,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        m_pSTWObject->SetManifestReadyMethod(this, &ManifestReadyTests::ManifestReadyTestSelectedStreams );
        
        PKTEST_FUNC_EXIT( string urlString = GetUrl(L"VC1ODMultiAudioMultiVideo") );

        PKTEST_FUNC_EXIT( m_pSTWObject->OpenVideoAndValidate( urlString, true, true) );

    exit:
        return;
    }

    //Passing Wrong URL to Open should NOT raise ManifestReady Event and should Throw Error
    PKTEST_METHOD_EX( VerifyWrongURLShouldNOTRaiseManifestReady,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        
        m_pSTWObject->SetManifestReadyMethod(this, &ManifestReadyTests::WrongURLShouldNOTRaiseManifestReady );
        
        m_pSTWObject->OpenVideo("WrongUrl", true, false);
        
        PKTEST_ASSERT_MSG_EXIT(!m_pSTWObject->WaitForManifestReady(5000), "should not raise ManifestReady for error case" );

        smoothErrorVector = m_pSTWObject->GetErrorCallbackVector();
        smoothStatusVector = m_pSTWObject->GetStatusCallbackVector();

        PKTEST_ASSERT_MSG_EXIT(smoothStatusVector.size() == 0, "There should not be any status updates when Manifestparsing error occurred." );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_SocketOpenFailed, "SocketOpenFailed expected, but observed some other error: %s.", m_pSTWObject->GetErrorName(smoothErrorVector[0]._errorCode));
        
    exit:
        return;
    }

    void WrongURLShouldNOTRaiseManifestReady(IManifest* pManifest, pkRESULT hr)
    {
        //Do Nothing
        PKTEST_HRESULT_MSG_EXIT(E_UNEXPECTED, "Unexpected manifest ready Event." );
    exit:
        return;
    }

private:

    void OpenVideo(string url, bool autoPlay, bool manifestReadyExpected)
    {
        m_pSTWObject->OpenVideo(escapeSpaces(url), autoPlay);
        WaitForManifestReady(manifestReadyExpected);

        if( autoPlay )
        {
            m_pSTWObject->Delay( VALIDATION_DELAY );
        }

        PKTEST_CHECK_FAIL_EXIT();

exit:
        return;
    }

    //"expectedResult == true" means manifestready event is expected.
    void WaitForManifestReady(bool expectedResult)
    {
        bool waitResult = m_pSTWObject->WaitForManifestReady( MANIFESTREADY_TIMEOUT );

        if(waitResult)
        {
            PKTEST_MSG("Received manifest ready Event.");
        }
        else 
        {
            PKTEST_MSG("Timeout or some failure happened while waiting for the manifest ready.");
        }

        if(waitResult == expectedResult)
        {
            PKTEST_MSG("Verification Succeeded");
        }
        else
        {
            PKTEST_HRESULT_MSG_EXIT( E_UNEXPECTED, 
                                    "Err! Manifest ready, Verification failed. (waitResult=%d, expectedResult=%d)", waitResult, expectedResult);
        }

exit:
        return;
    }

    string GetUrl(wstring keyName)
    {
        string url = SSPKHelpers::GetUrlFromList( m_Manifests, keyName.c_str() );

        return (url);
    }
};
