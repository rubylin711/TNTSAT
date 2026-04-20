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
#include <sstream>
#include <algorithm>

using namespace SSPK;
using namespace SSPKTest;

PKTEST_GROUP( FieldDiagnosticCallbackTests )
{
    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;
    static const int32_t MAX_NONFATAL_CHUNK404S = 50;
    static const int32_t MAX_NONFATAL_CHUNK412S = 50;

    /// Because of Bug 28816
    static const int32_t MAX_NONFATAL_CHUNK404S_FOR_NEXTCHUNK = 51;

    FieldDiagnosticCallbackTests()
    {
        CXMLElement xTestData = PKTest_GetDataTable( "IntegrationTestSources" );
        if( !xTestData.IsNull() )
        {
             _xManifests = xTestData.Elements(L"source");
        }
    }

    pkRESULT StartSmoothObject(string url, bool waitForManifestReady, bool autoPlay = true)
    {
        pkRESULT result;

        result = _smoothObject->OpenVideo(url, autoPlay, false );
        if(waitForManifestReady)
        {
            PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );
        }
        else
        {
            PKTEST_ASSERT_MSG_EXIT( !_smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Expected there to be no manifest ready, but there was one" );
        }

        return result;
    exit:
        return pkE_TIMEOUT;
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
        if(!_smoothObject->IsClosed() )
        {
            _smoothObject->Close();
        }
        delete _smoothObject;
        return true;
    }

    PKTEST_METHOD_EX( FieldDiag_ManifestError_ParserError,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODStreamNoType") );
        PKTEST_FUNC_EXIT(StartSmoothObject(urlString, false, true));
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT ), "Timeout occurred waiting for media to fail");
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();

        PKTEST_ASSERT_MSG_EXIT(smoothStatusVector.size() == 0, "There should not be any status updates when Manifestparsing error occurred." );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ManifestParseFailed, "ManifestParseFailed expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));

    exit:
        return;
    }

    PKTEST_METHOD_EX( FieldDiag_ManifestError_Manifest404_AutoPlayTrue,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"NonExistentManifest") );
        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, false) );
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();

        PKTEST_ASSERT_MSG_EXIT(smoothStatusVector.size() == 0, "There should not be any status updates when Manifestparsing error occurred." );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ManifestHttpInvalidResult, "ManifestHttpInvalidResult expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._httpResponse == 404, "Expecting 404, but not found" );

    exit:
        return;
    }

    PKTEST_METHOD_EX(FieldDiag_ManifestError_Manifest404_AutoPlayFalse,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"NonExistentManifest") );
        PKTEST_FUNC_EXIT(StartSmoothObject(urlString, false, false));
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT ), "Timeout occurred waiting for media to fail");
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();

        PKTEST_ASSERT_MSG_EXIT(smoothStatusVector.size() == 0, "There should not be any status updates when Manifestparsing error occurred." );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ManifestHttpInvalidResult, "ManifestHttpInvalidResult expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._httpResponse == 404, "Expecting 404, but not found" );

    exit:
        return;
    }

    ///////////////////////////////////////////////////
    PKTEST_METHOD_EX( FieldDiag_ManifestError_AudioOnly_AfterManifestReady,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODAudioOnly") );
        PKTEST_FUNC_EXIT(StartSmoothObject(urlString, false, true));
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT ), "Timeout occurred waiting for media to fail");

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();

        PKTEST_ASSERT_MSG_EXIT(smoothStatusVector.size() == 0, "There should not be any status updates when Manifestparsing error occurred." );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ManifestInvalid, "ManifestInvalid expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));

    exit:
        return;
    }

    PKTEST_METHOD_EX( FieldDiag_ODAllVideoFragments404,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk404Count = 0;
        string AllVideoFragments404Body = "";

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoFragments404Body = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", 404 );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoFragments404Body);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT ), "Timeout occurred waiting for media to fail");
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( smoothStatusVector[i]._update == SmoothTransportStatus_ChunkConnectHttpInvalid ) && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                 totalChunk404Count++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalChunk404Count == MAX_NONFATAL_CHUNK404S, "There are %d chunk 404, but we are expecting the chunk 404 count to be %d", totalChunk404Count, MAX_NONFATAL_CHUNK404S );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ChunkConnectHttpInvalidResult, "ChunkConnectHttpInvalidResult expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._httpResponse == 404, "Expecting 404 but not found.");


    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_ODAllVideoFragments404After5Sec,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk404Count = 0;
        string AllVideoFragments404Body = "";

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoFragments404Body = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", 404, "wallClockInSeconds=\"5:50\"" );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoFragments404Body);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingStarted(PLAYBACK_STARTUP_TIMEOUT), "Timeout waiting for media rendered" );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT ), "Timeout occurred waiting for media to fail");
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( SmoothTransportStatus_NextChunkHttpInvalid == smoothStatusVector[i]._update ) && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                 totalChunk404Count++;
            }
        }

        //Below statement should change when bug 28816 is fixed.
        PKTEST_ASSERT_MSG_EXIT(totalChunk404Count == MAX_NONFATAL_CHUNK404S_FOR_NEXTCHUNK, "There are %d chunk 404, but we are expecting the chunk 404 count to be %d", totalChunk404Count, MAX_NONFATAL_CHUNK404S );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ChunkNextHttpInvalidResult, "ChunkNextHttpInvalidResult expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._httpResponse == 404, "Expecting 404 but not found.");


    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_ODAllVideoFragments404For7Sec,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk404Count = 0;
        string AllVideoFragments404BodyFor7Sec = "";

        PKTEST_FUNC_EXIT(string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoFragments404BodyFor7Sec = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", 404, "offset=\"50000000:120000000\" ");

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoFragments404BodyFor7Sec);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingStarted(PLAYBACK_STARTUP_TIMEOUT), "Timeout waiting for media rendered" );
        
        _smoothObject->Delay(2 * PLAYBACK_STARTUP_TIMEOUT);

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();

        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( SmoothTransportStatus_NextChunkHttpInvalid == smoothStatusVector[i]._update ) && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                 totalChunk404Count++;
            }
        }

        //Below statement should change when bug 28816 is fixed.
        PKTEST_ASSERT_MSG_EXIT(totalChunk404Count < MAX_NONFATAL_CHUNK404S_FOR_NEXTCHUNK, "There are %d chunk 404, but we are expecting the chunk 404 count to be less than %d", totalChunk404Count, MAX_NONFATAL_CHUNK404S );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 0, "We are not expecting any error, but found %d errors in the list", smoothErrorVector.size() );

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_ODAllVideoKeyFrames404After5Sec,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk404Count = 0;
        string AllVideoKeyFrames404Body = "";

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoKeyFrames404Body = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "KeyFrames", 404, "wallClockInSeconds=\"5:500\"" );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoKeyFrames404Body);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(PLAYBACK_STARTUP_TIMEOUT), "Timeout waiting for media rendered");
        PKTEST_HRESULT_EXIT(_smoothObject->FastForward());
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT), "Timeout waiting for media rendered");

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( SmoothTransportStatus_NextChunkHttpInvalid == smoothStatusVector[i]._update ) && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                 totalChunk404Count++;
            }
        }

        //Below statement should change when bug 28816 is fixed.
        PKTEST_ASSERT_MSG_EXIT(totalChunk404Count == MAX_NONFATAL_CHUNK404S_FOR_NEXTCHUNK, "There are %d chunk 404, but we are expecting the chunk 404 count to be %d", totalChunk404Count, MAX_NONFATAL_CHUNK404S );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ChunkNextHttpInvalidResult, "ChunkNextHttpInvalidResult expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._httpResponse == 404, "Expecting 404 but not found.");


    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_ODAllVideoKeyFrames404For7Sec,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk404Count = 0;
        string AllVideoKeyFrames404BodyFor7Sec = "";

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODDefault") );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoKeyFrames404BodyFor7Sec = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "KeyFrames", 404, "offset=\"50000000:120000000\" ");

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoKeyFrames404BodyFor7Sec);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingStarted(PLAYBACK_STARTUP_TIMEOUT), "Timeout waiting for media rendered" );
        PKTEST_HRESULT_EXIT(_smoothObject->FastForward());
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingStarted(PLAYBACK_STARTUP_TIMEOUT), "Timeout waiting for media rendered" );
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( SmoothTransportStatus_NextChunkHttpInvalid == smoothStatusVector[i]._update ) && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                 totalChunk404Count++;
            }
        }

        //Below statement should change when bug 28816 is fixed.
        PKTEST_ASSERT_MSG_EXIT(totalChunk404Count < MAX_NONFATAL_CHUNK404S_FOR_NEXTCHUNK, "There are %d chunk 404, but we are expecting the chunk 404 count to be less than %d", totalChunk404Count, MAX_NONFATAL_CHUNK404S );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 0, "We are  not expecting any error, but found %d errors in the list", smoothErrorVector.size() );

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    PKTEST_METHOD_EX( FieldDiag_ODAllHighBitrateVideoFragments404,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk404Count = 0;
        string AllVideoFragments404Body = "";

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoFragments404Body = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", 404, "qualityLevels=\"991000:2962000\"" );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoFragments404Body);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingStarted(PLAYBACK_STARTUP_TIMEOUT ), "Timeout occured waiting rendering");
        
        //Need to wait until BPS gets to high bitrates
		_smoothObject->Delay(PLAYBACK_STARTUP_TIMEOUT);		

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( ( SmoothTransportStatus_ChunkConnectHttpInvalid == smoothStatusVector[i]._update ) || ( SmoothTransportStatus_NextChunkHttpInvalid == smoothStatusVector[i]._update ) )
                && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                 totalChunk404Count++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(0 < totalChunk404Count && totalChunk404Count < MAX_NONFATAL_CHUNK404S, "There are %d chunk 404, but we are expecting the chunk 404 count to be between 0 and %d", totalChunk404Count, MAX_NONFATAL_CHUNK404S );
        PKTEST_ASSERT_MSG_EXIT(0 == smoothErrorVector.size(), "We are expecting no errors, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "404 only on high bitrates, retrying lower bitrates is recoverable, we should continue playing.");
    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_LiveAllVideoFragments404,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk404Count = 0;
        string AllVideoFragments404Body = "";

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, L"LiveDefault") );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoFragments404Body = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", 404 );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoFragments404Body);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT ), "Timeout occurred waiting for media to fail");
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( smoothStatusVector[i]._update == SmoothTransportStatus_ChunkConnectHttpInvalid ) && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                totalChunk404Count++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalChunk404Count > 0, "We are expecting the chunk 404 count to be greater than 0" );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ChunkConnectHttpInvalidResult, "ChunkConnectHttpInvalidResult expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._httpResponse == 404, "Expecting 404 but not found.");


    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_LiveAllVideoFragments404_skipBackToCheckMax404Count,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk404Count = 0;
        string AllVideoFragments404Body = "";

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveDefault") );

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_HRESULT_EXIT(_smoothObject->Skip(-TOTAL_SECONDS_IN_A_DAY));

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoFragments404Body = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", 404 );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoFragments404Body);

        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT ), "Timeout occurred waiting for media to fail");
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( smoothStatusVector[i]._update == SmoothTransportStatus_NextChunkHttpInvalid ) && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                totalChunk404Count++;
            }
        }
        
        //Below statement should change when bug 28816 is fixed.
        PKTEST_ASSERT_MSG_EXIT(totalChunk404Count == MAX_NONFATAL_CHUNK404S_FOR_NEXTCHUNK, "There are %d chunk 404, but we are expecting the chunk 404 count to be %d", totalChunk404Count, MAX_NONFATAL_CHUNK404S );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ChunkNextHttpInvalidResult, "ChunkNextHttpInvalidResult expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._httpResponse == 404, "Expecting 404 but not found.");


    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_LiveAllVideoFragments404After5Sec,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk404Count = 0;
        string AllVideoFragments404Body = "";

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveDefault") );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoFragments404Body = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", 404, "wallClockInSeconds=\"5:50\"" );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoFragments404Body);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingStarted(PLAYBACK_STARTUP_TIMEOUT), "Timeout waiting for media rendered" );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT), "Timeout occurred waiting for media to fail");
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( SmoothTransportStatus_NextChunkHttpInvalid == smoothStatusVector[i]._update ) && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                 totalChunk404Count++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(0 < totalChunk404Count && totalChunk404Count < MAX_NONFATAL_CHUNK404S, "There are %d chunk 404, but we are expecting the chunk 404 count to be between 0 and %d", totalChunk404Count, MAX_NONFATAL_CHUNK404S );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ChunkNextHttpInvalidResult, "ChunkNextHttpInvalidResult expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._httpResponse == 404, "Expecting 404 but not found.");


    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_VideoFragment1chunk404,
        PKTEST_PROPERTY("Priority", "0")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk404Count = 0;
        int32_t maxExpectedChunk404Count = 10; //Even though only 1 video chunk should return 404, playing at higher bitrates there may be a few retries for the lower bitrates.

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests, L"ODVideoFragment1chunk404") );
        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY * 2); //Give the average playrate time to flatten out the outlier first measurement
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( ( SmoothTransportStatus_ChunkConnectHttpInvalid == smoothStatusVector[i]._update ) || ( SmoothTransportStatus_NextChunkHttpInvalid == smoothStatusVector[i]._update ) )
                && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                 totalChunk404Count++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(0 < totalChunk404Count && maxExpectedChunk404Count > totalChunk404Count, "There are %d chunk 404, but we are expecting the chunk 404 count to be between 0 and %d", totalChunk404Count, maxExpectedChunk404Count);
        PKTEST_ASSERT_MSG_EXIT(0 == smoothErrorVector.size(), "We are expecting no errors, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "single chunk failure is recoverable, we should continue playing.");

    exit:
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_LiveFragmentInfo412s,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk412Count = 0;
        string AllVideoFragments412Body = "";
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveDefault") );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        AllVideoFragments412Body = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragmentinfo", 412 );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoFragments412Body);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_HRESULT_EXIT(_smoothObject->Pause(2 * VALIDATION_DELAY));
        PKTEST_HRESULT_EXIT( _smoothObject->Play());
        _smoothObject->Delay(2 * VALIDATION_DELAY);
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( smoothStatusVector[i]._update == SmoothTransportStatus_ChunkHdrHttpInvalid ) && ( smoothStatusVector[i]._httpResponse == 412 ) )
            {
                totalChunk412Count++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(0 < totalChunk412Count, "Expecting the chunk 412 count to be greater 0" );
        PKTEST_ASSERT_MSG_EXIT(0 == smoothErrorVector.size(), "We are expecting no errors, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Failure to download fragmentInfo, we should continue playing.");

    exit:

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    PKTEST_METHOD_EX( FieldDiag_LiveAllVideoFragments412,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk412Count = 0;
        string AllVideoFragments412Body = "";

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264LiveMultiAudio");
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoFragments412Body = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", 412 );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoFragments412Body);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );

        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT * 2 ), "Timeout occurred waiting for media to fail");

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( smoothStatusVector[i]._update == SmoothTransportStatus_ChunkConnectHttpInvalid ) && ( smoothStatusVector[i]._httpResponse == 412 ) )
            {
                totalChunk412Count++;
            }
        }
        PKTEST_ASSERT_MSG_EXIT(totalChunk412Count == MAX_NONFATAL_CHUNK412S, "There are %d chunk 412, but we are expecting the chunk 412 count to be %d", totalChunk412Count, MAX_NONFATAL_CHUNK412S );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ChunkConnectHttpInvalidResult, "ChunkConnectHttpInvalidResult expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._httpResponse == 412, "Expecting 412 but not found.");

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_LiveAllFragments412,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk412Count = 0;
        string AllFragments412Body = "";

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264LiveMultiAudio");
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllFragments412Body = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments","", 412 );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllFragments412Body);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT * 2 ), "Timeout occurred waiting for media to fail");
        
        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( smoothStatusVector[i]._update == SmoothTransportStatus_ChunkConnectHttpInvalid ) && ( smoothStatusVector[i]._httpResponse == 412 ) )
            {
                totalChunk412Count++;
            }
        }
        PKTEST_ASSERT_MSG_EXIT(totalChunk412Count == MAX_NONFATAL_CHUNK412S, "There are %d chunk 412, but we are expecting the chunk 412 count to be %d", totalChunk412Count, MAX_NONFATAL_CHUNK412S );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ChunkConnectHttpInvalidResult, "ChunkConnectHttpInvalidResult expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._httpResponse == 412, "Expecting 412 but not found.");

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    PKTEST_METHOD_EX( FieldDiag_LiveFragmentInfo404s,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunk404Count = 0;
        string AllVideoFragments404Body = "";

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveDefault") );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoFragments404Body = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragmentInfo", 404 );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoFragments404Body);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_HRESULT_EXIT(_smoothObject->Pause(VALIDATION_DELAY));
        PKTEST_HRESULT_EXIT( _smoothObject->Play());
        _smoothObject->Delay(2*VALIDATION_DELAY);

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( ( smoothStatusVector[i]._update == SmoothTransportStatus_ChunkHdrHttpInvalid ) && ( smoothStatusVector[i]._httpResponse == 404 ) )
            {
                totalChunk404Count++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalChunk404Count > 0, "Expecting the chunk 404 count to be greater 0" );
        PKTEST_ASSERT_MSG_EXIT(0 == smoothErrorVector.size(), "We are expecting no errors, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "Failure to get fragmentInfo is not fatal, we should continue playing.");

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_AllVideoInvalidFragmentHeader,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunkHrdWarningCount = 0;
        string AllVideoInvalidFragmentBody = "";

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoInvalidFragmentBody = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", 200, "", "body file=\"F:\\ClientTestMedia\\Automation\\H264\\BadContent\\Badchunk2.txt\"" );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoInvalidFragmentBody);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();

        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( smoothStatusVector[i]._update == SmoothTransportStatus_ChunkHdrError )
            {
                totalChunkHrdWarningCount++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalChunkHrdWarningCount > 0, "There should be any ChunkHdrError warnings, but none found." );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ChunkHdrParseFailed, "ChunkHdrParseFailed expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_AllVideoInvalidFragmentHeaderAfter5Sec,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunkHrdWarningCount = 0;
        string AllVideoInvalidFragmentBody = "";

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoInvalidFragmentBody = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", 200, "wallClockInSeconds=\"5:50\"", "body file=\"F:\\ClientTestMedia\\Automation\\H264\\BadContent\\Badchunk2.txt\"" );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoInvalidFragmentBody);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaRenderingStarted(PLAYBACK_STARTUP_TIMEOUT), "Timeout waiting for media rendered" );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaFailed( PLAYBACK_STARTUP_TIMEOUT ), "Timeout occurred waiting for media to fail");

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();
        
        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( smoothStatusVector[i]._update == SmoothTransportStatus_ChunkHdrError )
            {
                totalChunkHrdWarningCount++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalChunkHrdWarningCount > 0, "There should be any ChunkHdrError warnings, but none found." );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ChunkHdrParseFailed, "ChunkHdrParseFailed expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_LiveAllVideoInvalidFragmentHeader,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunkHrdWarningCount = 0;
        string AllVideoInvalidFragmentBody = "";

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveDefault") );
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoInvalidFragmentBody = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", 200, "", "body file=\"D:\\Live\\Badchunk2.txt\"" );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoInvalidFragmentBody);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForMediaFailed(PLAYBACK_STARTUP_TIMEOUT ), "Timeout occurred waiting for media to fail");

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();

        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( smoothStatusVector[i]._update == SmoothTransportStatus_ChunkHdrError )
            {
                totalChunkHrdWarningCount++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalChunkHrdWarningCount > 0, "There should be any ChunkHdrError warnings, but none found." );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector.size() == 1, "We are expecting only one error, but found %d errors in the list", smoothErrorVector.size() );
        PKTEST_ASSERT_MSG_EXIT(smoothErrorVector[0]._errorCode == SmoothTransportError_ChunkHdrParseFailed, "ChunkHdrParseFailed expected, but observed some other error: %s.", _smoothObject->GetErrorName(smoothErrorVector[0]._errorCode));

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }

    PKTEST_METHOD_EX( FieldDiag_LiveAllVideoInvalidFragmentInfoHeader,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        vector<SmoothTransportError> smoothErrorVector;
        vector<SmoothTransportStatus> smoothStatusVector;
        int32_t totalChunkHrdWarningCount = 0;
        string AllVideoInvalidFragmentBody = "";

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveDefault") );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        AllVideoInvalidFragmentBody = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragmentInfo", 200, "", "body file=\"D:\\Live\\Badchunk2.txt\"" );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), AllVideoInvalidFragmentBody);

        PKTEST_HRESULT_EXIT( StartSmoothObject(urlString, true) );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_HRESULT_EXIT(_smoothObject->Pause(VALIDATION_DELAY));
        PKTEST_HRESULT_EXIT( _smoothObject->Play());
        _smoothObject->Delay(2 * VALIDATION_DELAY);

        smoothErrorVector = _smoothObject->GetErrorCallbackVector();
        smoothStatusVector = _smoothObject->GetStatusCallbackVector();

        for( size_t i = 0; i < smoothStatusVector.size(); i++ )
        {
            if( smoothStatusVector[i]._update == SmoothTransportStatus_ChunkHdrError )
            {
                totalChunkHrdWarningCount++;
            }
        }

        PKTEST_ASSERT_MSG_EXIT(totalChunkHrdWarningCount > 0, "There should be any ChunkHdrError warnings, but none found." );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->IsPlaying(), "SmoothTransportStatus_ChunkHdrError is not fatal, we should continue playing.");


    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;

    }
};
