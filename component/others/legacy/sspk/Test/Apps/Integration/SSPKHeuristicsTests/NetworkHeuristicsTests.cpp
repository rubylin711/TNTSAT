///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "STWrapper.h"
#include "ISmoothTransport.h"
#include "ManifestReadyCallbackImpl.h"
#include "StringUtils.h"
#include "SSPKHelpers.h"
#include <sstream>
#include <algorithm>

using namespace SSPK;
using namespace SSPKTest;

#define GRADUAL_PROFILE \
                 "<profile>\n" \
                 "   <clientIP address=\"\">\n" \
                 "     <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"45\" >\n" \
                 "         <throttle kbs=\"700\" />\n" \
                 "     </request>\n" \
                 "     <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"90\" >\n" \
                 "       <throttle kbs=\"3000\" />\n" \
                 "     </request>\n" \
                 "     <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"135\" >\n" \
                 "       <throttle kbs=\"2000\" />\n" \
                 "     </request>\n" \
                 "     <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"180\" >\n" \
                 "       <throttle kbs=\"4500\" />\n" \
                 "     </request>\n" \
                 "     <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"250\" >\n" \
                 "       <throttle kbs=\"2000\" />\n" \
                 "     </request>\n" \
                 "   </clientIP>\n" \
                 " </profile>\n"

#define SUDDEN_PROFILE \
                 "<profile>\n" \
                 "   <clientIP address="">\n" \
                 "      <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"45\" >\n" \
                 "        <throttle kbs=\"2000\" />\n" \
                 "      </request>\n" \
                 "      <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"50\" >\n" \
                 "        <throttle kbs=\"4500\" />\n" \
                 "      </request>\n" \
                 "      <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"90\" >\n" \
                 "        <throttle kbs=\"2000\" />\n" \
                 "      </request>\n" \
                 "      <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"95\" >\n" \
                 "        <throttle kbs=\"1000\" />\n" \
                 "      </request>\n" \
                 "      <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"135\" >\n" \
                 "        <throttle kbs=\"2000\" />\n" \
                 "      </request>\n" \
                 "      <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"140\" >\n" \
                 "        <throttle kbs=\"4000\" />\n" \
                 "      </request>\n" \
                 "      <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"145\" >\n" \
                 "        <throttle kbs=\"1000\" />\n" \
                 "      </request>\n" \
                 "      <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"185\" >\n" \
                 "        <throttle kbs=\"2000\" />\n" \
                 "      </request>\n" \
                 "      <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"190\" >\n" \
                 "        <throttle kbs=\"3000\" />\n" \
                 "      </request>\n" \
                 "      <request url=\"{0}\" requestType=\"fragments\" wallClockInSeconds=\"250\" >\n" \
                 "        <throttle kbs=\"2000\" />\n" \
                 "      </request>\n" \
                 "    </clientIP>\n" \
                 "  </profile>\n"

#define LATENCY_PROFILE \
                 "<profile>\n" \
                 "   <clientIP address="">\n" \
                 "     <request url=\"{0}\" requestType=\"fragments\" fragmentType=\"video\" count=15:20 >\n" \
                 "         <delayInMilliseconds mean=\"2000\" />\n" \
                 "      </request>\n" \
                 "    </clientIP>\n" \
                 "  </profile>\n"

PKTEST_GROUP( NetworkHeuristicsTests )
{

    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;
    AutoRefPtr<IManifest> _pManifest;

    //Seek Times in seconds
    int32_t _timeForSeek;
    int32_t _seekOffset;

    

    NetworkHeuristicsTests() : _timeForSeek(60), _seekOffset(30)
    {


        CXMLElement xTestData = PKTest_GetDataTable( "HeuristicsTestSources" );
        if( !xTestData.IsNull() )
        {
            _xManifests = xTestData.Elements(L"source");
        }


    }

    ~NetworkHeuristicsTests()
    {
    }

    bool TestSetup()
    {
        vector<DiagsChannel> channelList;

        PKTEST_ASSERT_MSG_EXIT(_xManifests.Length() > 0, "Couldn't find any elements with name 'source'");

        _smoothObject = NEW_NO_THROW STWrapper();
        PKTEST_ASSERT_EXIT(NULL != _smoothObject);

        channelList.push_back(kDiagChannel_Heuristics);
        _smoothObject->RegisterDiagEvents(channelList, kDiagsPriority_Medium);
        return true;

    exit:
        return false;
    }

    bool TestCleanup()
    {
        pkRESULT pkResult = pkS_OK;
        bool tracePostResult = true;
        if(!_smoothObject->IsClosed() )
        {
            pkResult = _smoothObject->Close();
        }

        string allTraces = _smoothObject->GetCompleteTraces();
        
        string name = PKTest_GetTestFullName();
        size_t index = name.find("::");
        if(index != string::npos)
        {
            name = name.substr(index+2);
        }


        delete _smoothObject;

        return SUCCEEDED(pkResult) && tracePostResult;
    }

    ////////////// Network Heuristics Stream Tests ////////////////
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX
        ( NetworkHeuristics_FixedNetworkCap_ODSeek,
        PKTEST_PROPERTY("Data:700", "700")
        PKTEST_PROPERTY("Data:1000", "1000")
        PKTEST_PROPERTY("Data:2000", "2000")
        PKTEST_PROPERTY("Data:3000", "3000")
        PKTEST_PROPERTY("Data:4500", "4500")
        )
    {
        string profileString;
        int64_t currentPlayBackTime;
        int64_t proposedPlayBackTime;

        _smoothObject->SetManifestReadyMethod( this, &NetworkHeuristicsTests::ManifestReadyPassThrough );

        int32_t maxBandwidth = toInt(PKTest_GetTestData()); // change to bytes

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );

        profileString = SSPKHelpers::GetCR_SetMaxNetworkCap(SSPKHelpers::GetLocalUrl(urlString), maxBandwidth);
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), profileString);
        
        _smoothObject->OpenVideo(urlString, true );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );

        _smoothObject->Delay(_timeForSeek * 1000);
        // Commenting out the verification part as they dont really work now, until we get the diagevents and populate the chunklist vector.
        //PKTEST_FUNC_EXIT( _smoothObject->CheckNetworkHeuristicBasicBehaviour(_pManifest, maxBandwidth * BITS_PER_KILOBITS) );

        _smoothObject->Skip(_seekOffset);
        _smoothObject->Delay(_timeForSeek * 1000);
        //PKTEST_FUNC_EXIT( _smoothObject->CheckNetworkHeuristicBasicBehaviour(_pManifest, maxBandwidth * BITS_PER_KILOBITS) );

        currentPlayBackTime = _smoothObject->GetCurrentPlayBackTime();
        proposedPlayBackTime = currentPlayBackTime + ( _seekOffset * TIMESCALE_10MHZ );
        
        _smoothObject->Seek( proposedPlayBackTime );
        _smoothObject->Delay(_timeForSeek * 1000);
        //PKTEST_FUNC_EXIT( _smoothObject->CheckNetworkHeuristicBasicBehaviour(_pManifest, maxBandwidth * BITS_PER_KILOBITS) );


    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    ////////////// Network Heuristics Stream Tests ////////////////
    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX
        ( NetworkHeuristics_FixedNetworkCap_LiveSeek,
        PKTEST_PROPERTY("Data:700", "700")
        PKTEST_PROPERTY("Data:1000", "1000")
        PKTEST_PROPERTY("Data:2000", "2000")
        PKTEST_PROPERTY("Data:3000", "3000")
        PKTEST_PROPERTY("Data:4500", "4500")
        )
    {
        string profileString;
        int64_t currentPlayBackTime;
        int64_t proposedPlayBackTime;

        _smoothObject->SetManifestReadyMethod( this, &NetworkHeuristicsTests::ManifestReadyPassThrough );

        int32_t maxBandwidth = toInt(PKTest_GetTestData()); // change to bytes

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveDefault") );

        profileString = SSPKHelpers::GetCR_SetMaxNetworkCap(SSPKHelpers::GetLocalUrl(urlString), maxBandwidth);
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), profileString);
        
        _smoothObject->OpenVideo(urlString, true );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );

        _smoothObject->Delay(_timeForSeek * 1000);
        // Commenting out the verification part as they dont really work now, until we get the diagevents and populate the chunklist vector.
        //PKTEST_FUNC_EXIT( _smoothObject->CheckNetworkHeuristicBasicBehaviour(_pManifest, maxBandwidth * BITS_PER_KILOBITS) );

        _smoothObject->Skip(_seekOffset);
        _smoothObject->Delay(_timeForSeek * 1000);
        //PKTEST_FUNC_EXIT( _smoothObject->CheckNetworkHeuristicBasicBehaviour(_pManifest, maxBandwidth * BITS_PER_KILOBITS) );

        currentPlayBackTime = _smoothObject->GetCurrentPlayBackTime();
        proposedPlayBackTime = currentPlayBackTime + ( _seekOffset * TIMESCALE_10MHZ );
        
        _smoothObject->Seek( proposedPlayBackTime );
        _smoothObject->Delay(_timeForSeek * 1000);
        //PKTEST_FUNC_EXIT( _smoothObject->CheckNetworkHeuristicBasicBehaviour(_pManifest, maxBandwidth * BITS_PER_KILOBITS) );


    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        NetworkHeuristics_NetworkChange_ODSeek,
        PKTEST_PROPERTY("Data:Gradual", GRADUAL_PROFILE)
        PKTEST_PROPERTY("Data:Sudden", SUDDEN_PROFILE)
        PKTEST_PROPERTY("Data:Latency", LATENCY_PROFILE)
        )
    {
        int64_t currentPlayBackTime;
        int64_t proposedPlayBackTime;
        size_t index = string::npos;

        string currentCRProfile = PKTest_GetTestData();

        _smoothObject->SetManifestReadyMethod( this, &NetworkHeuristicsTests::ManifestReadyPassThrough );

        string replaceString = "{0}";
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );
        
        while((index = currentCRProfile.find(replaceString)) != string::npos)
        {
            currentCRProfile.replace(index, replaceString.length(), SSPKHelpers::GetLocalUrl(urlString));
        }

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), currentCRProfile);
        
        _smoothObject->OpenVideo(urlString, true );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );

        _smoothObject->Delay(_timeForSeek * 1000);

        _smoothObject->Skip(_seekOffset);
        _smoothObject->Delay(_timeForSeek * 1000);

        currentPlayBackTime = _smoothObject->GetCurrentPlayBackTime();
        proposedPlayBackTime = currentPlayBackTime + ( _seekOffset * TIMESCALE_10MHZ );
        
        _smoothObject->Seek( proposedPlayBackTime );
        _smoothObject->Delay(_timeForSeek * 1000);

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        NetworkHeuristics_NetworkChange_LiveSeek,
        PKTEST_PROPERTY("Data:Gradual", GRADUAL_PROFILE)
        PKTEST_PROPERTY("Data:Sudden", SUDDEN_PROFILE)
        PKTEST_PROPERTY("Data:Latency", LATENCY_PROFILE)
        )
    {
        int64_t currentPlayBackTime;
        int64_t proposedPlayBackTime;
        size_t index = string::npos;

        string currentCRProfile = PKTest_GetTestData();

        _smoothObject->SetManifestReadyMethod( this, &NetworkHeuristicsTests::ManifestReadyPassThrough );

        string replaceString = "{0}";
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveDefault") );
        
        while((index = currentCRProfile.find(replaceString)) != string::npos)
        {
            currentCRProfile.replace(index, replaceString.length(), SSPKHelpers::GetLocalUrl(urlString));
        }

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), currentCRProfile);
        
        _smoothObject->OpenVideo(urlString, true );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );

        _smoothObject->Delay(_timeForSeek * 1000);

        _smoothObject->Skip(_seekOffset);
        _smoothObject->Delay(_timeForSeek * 1000);

        currentPlayBackTime = _smoothObject->GetCurrentPlayBackTime();
        proposedPlayBackTime = currentPlayBackTime + ( _seekOffset * TIMESCALE_10MHZ );
        
        _smoothObject->Seek( proposedPlayBackTime );
        _smoothObject->Delay(_timeForSeek * 1000);

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        NetworkHeuristics_NetworkErrors_OD,
        PKTEST_PROPERTY("Data:404", "404")
        PKTEST_PROPERTY("Data:412", "412")
        )
    {
        int64_t currentPlayBackTime;
        int64_t proposedPlayBackTime;

        string currentCRProfile;
        int32_t networkErrorCode = toInt(PKTest_GetTestData());

        _smoothObject->SetManifestReadyMethod( this, &NetworkHeuristicsTests::ManifestReadyPassThrough );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );

        currentCRProfile = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", networkErrorCode,"count=\"10:15\"" );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), currentCRProfile);

        _smoothObject->OpenVideo(urlString, true );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );

        _smoothObject->Delay(_timeForSeek * 1000);

        _smoothObject->Skip(_seekOffset);
        _smoothObject->Delay(_timeForSeek * 1000);

        currentPlayBackTime = _smoothObject->GetCurrentPlayBackTime();
        proposedPlayBackTime = currentPlayBackTime + ( _seekOffset * TIMESCALE_10MHZ );
        
        _smoothObject->Seek( proposedPlayBackTime );
        _smoothObject->Delay(_timeForSeek * 1000);

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        NetworkHeuristics_NetworkErrors_Live,
        PKTEST_PROPERTY("Data:404", "404")
        PKTEST_PROPERTY("Data:412", "412")
        )
    {
        int64_t currentPlayBackTime;
        int64_t proposedPlayBackTime;

        string currentCRProfile;
        int32_t networkErrorCode = toInt(PKTest_GetTestData());

        _smoothObject->SetManifestReadyMethod( this, &NetworkHeuristicsTests::ManifestReadyPassThrough );

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveDefault") );

        currentCRProfile = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", networkErrorCode,"count=\"10:15\"" );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), currentCRProfile);

        _smoothObject->OpenVideo(urlString, true );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );

        _smoothObject->Delay(_timeForSeek * 1000);

        _smoothObject->Skip(_seekOffset);
        _smoothObject->Delay(_timeForSeek * 1000);

        currentPlayBackTime = _smoothObject->GetCurrentPlayBackTime();
        proposedPlayBackTime = currentPlayBackTime + ( _seekOffset * TIMESCALE_10MHZ );
        
        _smoothObject->Seek( proposedPlayBackTime );
        _smoothObject->Delay(_timeForSeek * 1000);

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }
    
    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        NetworkHeuristics_NetworkChange_ODTrickPlay,
        PKTEST_PROPERTY("Data:4500", "4500")
        )
    {
        string currentCRProfile = PKTest_GetTestData();

        _smoothObject->SetManifestReadyMethod( this, &NetworkHeuristicsTests::ManifestReadyPassThrough );

        int32_t maxBandwidth = toInt(PKTest_GetTestData()); // change to bytes
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );

        currentCRProfile = SSPKHelpers::GetCR_SetMaxNetworkCap(SSPKHelpers::GetLocalUrl(urlString), maxBandwidth);
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), currentCRProfile);
        
        _smoothObject->OpenVideo(urlString, true );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media renderered");

        _smoothObject->Delay(2 * VALIDATION_DELAY);

        _smoothObject->FastForwardAndValidate(LOW_SPEED, VALIDATION_DELAY);

        _smoothObject->Delay(_timeForSeek * 1000);

        _smoothObject->RewindAndValidate(LOW_SPEED, VALIDATION_DELAY);
        _smoothObject->Delay(_timeForSeek * 1000);

        _smoothObject->PlayAndValidate(VALIDATION_DELAY);
        _smoothObject->Delay(_timeForSeek * 1000);


    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        NetworkHeuristics_NetworkChange_LiveTrickPlay,
        PKTEST_PROPERTY("Data:4500", "4500")
        )
    {
        string currentCRProfile = PKTest_GetTestData();

        _smoothObject->SetManifestReadyMethod( this, &NetworkHeuristicsTests::ManifestReadyPassThrough );

        int32_t maxBandwidth = toInt(PKTest_GetTestData()); // change to bytes
        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveDefault") );

        currentCRProfile = SSPKHelpers::GetCR_SetMaxNetworkCap(SSPKHelpers::GetLocalUrl(urlString), maxBandwidth);
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), currentCRProfile);

        _smoothObject->OpenVideo(urlString, true );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );
        PKTEST_ASSERT_MSG_EXIT(_smoothObject->WaitForMediaRenderingStarted(STATECHANGE_TIMEOUT), "Timeout before media renderered");

        _smoothObject->Delay(2 * VALIDATION_DELAY);
        _smoothObject->RewindAndValidate(LOW_SPEED, VALIDATION_DELAY);
        
        _smoothObject->Delay(_timeForSeek * 1000);

        _smoothObject->FastForwardAndValidate(LOW_SPEED, VALIDATION_DELAY);  
        _smoothObject->Delay(_timeForSeek * 1000);

        _smoothObject->PlayAndValidate(VALIDATION_DELAY);
        _smoothObject->Delay(_timeForSeek * 1000);

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }


    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        NetworkHeuristics_ExtendedCommands_OD,
        PKTEST_PROPERTY("Data:MaxWaterMark18", "maxesfifolimitinnormalplayback90kHz--1620000")
        PKTEST_PROPERTY("Data:Max18LiveDelay12", "maxesfifolimitinnormalplayback90kHz--1620000;ssliveplaybackoffsetsec--12")
        )
    {
        int64_t currentPlayBackTime;
        int64_t proposedPlayBackTime;

        _smoothObject->SetManifestReadyMethod( this, &NetworkHeuristicsTests::ManifestReadyPassThrough );

        list< pair<string,string>> commands;
        string testData = PKTest_GetTestData();
        ExtendedCommandNameValue(testData, commands);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"H264ODMultiAudio") );

        for ( list< pair<string,string>>::const_iterator currentCommand = commands.begin(); currentCommand != commands.end(); ++currentCommand )
        {
            const char* value = currentCommand->second.c_str();
            PKTEST_FUNC_EXIT( _smoothObject->SendExtendedCommand(currentCommand->first.c_str(), 1, &value ) );
        }
        
        _smoothObject->OpenVideo(urlString, true );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );

        _smoothObject->Delay(_timeForSeek * 1000);

        _smoothObject->Skip( -_seekOffset);
        _smoothObject->Delay(_timeForSeek * 1000);

        currentPlayBackTime = _smoothObject->GetCurrentPlayBackTime();
        proposedPlayBackTime = currentPlayBackTime + ( _seekOffset * TIMESCALE_10MHZ );
        
        _smoothObject->Seek( proposedPlayBackTime );
        _smoothObject->Delay(_timeForSeek * 1000);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        NetworkHeuristics_ExtendedCommands_Live,
        PKTEST_PROPERTY("Data:MaxWaterMark18", "maxesfifolimitinnormalplayback90kHz--1620000")
        PKTEST_PROPERTY("Data:Max18LiveDelay12", "maxesfifolimitinnormalplayback90kHz--1620000;ssliveplaybackoffsetsec--12")
        )
    {
        int64_t currentPlayBackTime;
        int64_t proposedPlayBackTime;

        _smoothObject->SetManifestReadyMethod( this, &NetworkHeuristicsTests::ManifestReadyPassThrough );

        list< pair<string,string>> commands;
        string testData = PKTest_GetTestData();
        ExtendedCommandNameValue(testData, commands);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveDefault") );

        for ( list< pair<string,string>>::const_iterator currentCommand = commands.begin(); currentCommand != commands.end(); ++currentCommand )
        {
            const char* value = currentCommand->second.c_str();
            PKTEST_FUNC_EXIT( _smoothObject->SendExtendedCommand(currentCommand->first.c_str(), 1, &value ) );
        }
        
        _smoothObject->OpenVideo(urlString, true );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );

        _smoothObject->Delay(_timeForSeek * 1000);

        currentPlayBackTime = _smoothObject->GetCurrentPlayBackTime();
        proposedPlayBackTime = currentPlayBackTime - ( _seekOffset * TIMESCALE_10MHZ );
        
        _smoothObject->Seek( proposedPlayBackTime );
        _smoothObject->Delay(_timeForSeek * 1000);

        _smoothObject->Skip( TOTAL_SECONDS_IN_A_DAY );
        _smoothObject->Delay(_timeForSeek * 1000);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        NetworkHeuristics_ExtendedCommandsLIVE_again_Live,
        PKTEST_PROPERTY("Data:Min18LiveDelay5", "maxesfifolimitinnormalplayback90kHz--1620000;ssliveplaybackoffsetsec--5")
        )
    {

        _smoothObject->SetManifestReadyMethod( this, &NetworkHeuristicsTests::ManifestReadyPassThrough );

        list< pair<string,string>> commands;
        string testData = PKTest_GetTestData();
        ExtendedCommandNameValue(testData, commands);

        PKTEST_FUNC_EXIT( string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveDefault") );

        for ( list< pair<string,string>>::const_iterator currentCommand = commands.begin(); currentCommand != commands.end(); ++currentCommand )
        {
            const char* value = currentCommand->second.c_str();
            PKTEST_FUNC_EXIT( _smoothObject->SendExtendedCommand(currentCommand->first.c_str(), 1, &value ) );
        }
        
        _smoothObject->OpenVideo(urlString, true );
        PKTEST_ASSERT_MSG_EXIT( _smoothObject->WaitForManifestReady(MANIFESTREADY_TIMEOUT), "Timeout or some failure happened while waiting for the manifest ready." );

        _smoothObject->Delay(VALIDATION_DELAY * 2);

        _smoothObject->Skip( TOTAL_SECONDS_IN_A_DAY );
        _smoothObject->Delay(VALIDATION_DELAY * 2);

        
        _smoothObject->Skip( TOTAL_SECONDS_IN_A_DAY );
        _smoothObject->Delay(VALIDATION_DELAY * 2);
        
        _smoothObject->Skip( TOTAL_SECONDS_IN_A_DAY );
        _smoothObject->Delay(VALIDATION_DELAY * 2);
        
        _smoothObject->Skip( TOTAL_SECONDS_IN_A_DAY );
        _smoothObject->Delay(VALIDATION_DELAY * 2);

        _smoothObject->Skip( TOTAL_SECONDS_IN_A_DAY );
        _smoothObject->Delay(VALIDATION_DELAY * 2);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////
    void ManifestReadyPassThrough(IManifest* pManifest,pkRESULT hr)
    {
        _pManifest.Set(pManifest);
        _smoothObject->SetIsLive(_pManifest->IsLive());
    }

    void ExtendedCommandNameValue(string& testData, list< pair<string,string>>& commands )
    {
        const string delimiter = "--";
        const string commandDelimiter = ";";

        size_t commandIndex = 0;
        do
        {
            commandIndex = testData.find(commandDelimiter);
            
            string currentCommand = testData;

            if(commandIndex != string::npos)
            {
                currentCommand = testData.substr(0, commandIndex);
            }
            size_t index = currentCommand.find(delimiter);
        
            PKTEST_ASSERT_MSG_EXIT(index != string::npos, "DataDrivenTest data doesn't have the delimiter '--'.");

            commands.push_back( make_pair( currentCommand.substr(0, index), currentCommand.substr(index + delimiter.length()) ) );
            testData = testData.substr(commandIndex + commandDelimiter.length());

        }while(commandIndex != string::npos);

    exit:
        return;
    }
    
};
