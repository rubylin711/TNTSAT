///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include "PKTestSuiteUtils.h"
#include "PKTestSuite.h"
#include "StringUtils.h"
#include "ISmoothTransport.h"
#include "TestStreamerHttp.h"
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

template <class T>
inline std::string to_string (const T& t)
{
std::stringstream ss;
ss << t;
return ss.str();
}

static const string ADDPROFILE_STR = "/clientresponder(addprofile)";
static const string DELETEPROFILE_STR = "/clientresponder(deleteprofile)";


namespace SSPKTest
{
    using namespace std;
    using namespace SSPK;

    typedef std::vector< AutoRefPtr<IManifestStream> > VectorOfStreams;
    typedef std::vector< AutoRefPtr<IManifestTrack> > VectorOfTracks;

    class SSPKHelpers
    {
    private:
        static void CheckManifestAttributeValue( AutoRefPtr<IManifest> pManifest, const wstring& name, int64 expectedValue, bool presentInManifest );
        static void CheckManifestAttributeValue( AutoRefPtr<IManifest> pManifest, const wstring& name, const wstring& expectedValue, bool presentInManifest );
        static void CheckStreamAttributeValue( AutoRefPtr<IManifestStream> currentStream, const wstring& name, int64 expectedValue, bool presentInManifest );
        static void CheckStreamAttributeValue( AutoRefPtr<IManifestStream> currentStream, const wstring& name, const wstring& expectedValue, bool presentInManifest );
        static void CheckTrackAttributeValue( AutoRefPtr<IManifestTrack> currentTrack, const wstring& name, int64 expectedValue, bool presentInManifest);
        static void CheckTrackAttributeValue( AutoRefPtr<IManifestTrack> currentTrack, const wstring& name, const wstring& expectedValue, bool presentInManifest);

        const static bool AcquireLicenseForContent(CXMLElementsList xManifests, const wchar_t* nameAttribute);
        const static bool AttributeValueExistsInList(CXMLElementsList xManifests, const wchar_t* nameAttribute, const wchar_t* attribute);


    public:
        const static string GetUrlFromList( CXMLElementsList xManifests, const wchar_t* nameAttribute, bool ignoreLicenseAcquisition = false );
        const static string GetUrlFromList(CXMLElementsList xManifests, const char* nameAttribute, bool ignoreLicenseAcquisition = false);
        const static string GetAttributeValueFromList(CXMLElementsList xManifests, const wchar_t* nameAttribute, const wchar_t* attribute);
        const static bool AcquireLicense(string licenseServerURL, string keyID, string customData);

        static string GetLocalUrl(string url);
        static string GetServerNameFromUrl(string url);

        static string GetCR_ReturnStatusCodeProfile(const string& localUrl, const string& requestType, const int32_t& statusCode, const string& additionalConditions = "", const string& additionalResults = "");

        static string GetCR_ReturnStatusCodeProfile(const string& localUrl, const string& requestType, const string& streamName, const int32_t& statusCode, const string& additionalConditions = "", const string& additionalResults = "");

        static string GetCR_SetMaxNetworkCap(const string& localUrl, const int32_t& maxBandWidth );

        static string GetCR_SegmentManifestReturnStatusCodeProfile(const int64_t currentStartTime, const int64_t segmentLength, const string& localUrl, const string& returnCode);

        static string GetCR_SegmentManifestLatencyProfile(const int64_t currentStartTime, const int64_t segmentLength, const string& localUrl, const int64_t latency);

        static string GetCR_SegmentManifestProfile(const int64_t currentStartTime, const int64_t segmentLength, const string& localUrl, const string& options);

        static void ClientResponderPost(const string& url, const string& server, const string& body);

        static bool CompleteTracePost(const string& url, const string& server, const string& ClientName, const string& testCase, const string& body);

        static bool HttpPost(const string& url, const string& server, const vector<string>& additionalHeader, const string& body);

        static void CheckStreamCountByType( const VectorOfStreams& givenStreamVector, int32_t argAudioStreamCount,int32_t argVideoStreamCount,int32_t argOtherStreamCount );

        static void CheckManifestAttributes( AutoRefPtr<IManifest> pManifest, uint32_t majorVersion, uint32_t minorVersion, int64_t timeScale, int64_t duration, bool isLive, uint32_t lookAheadCount, int64 dvrWindowLength );

        static void CheckStreamAttributes( AutoRefPtr<IManifestStream> currentStream, int64_t timeScaleArg, const string& languageArg, uint32_t maxWidthArg, uint32_t maxHeightArg, uint32_t displayWidthArg, uint32_t displayHeightArg, const string& urlArg, const string& nameArg, MediaStreamType typeArg, const string& subTypeArg );

        static void CheckVideoTrackAttributes( AutoRefPtr<IManifestTrack> track, uint32_t audioTag, uint32_t bitrate, const wstring& fourCC, uint32_t hardwareProfile, uint32_t maxHeight, uint32_t maxWidth, uint32_t trackIndex, uint32_t nominalBitrate,  const wstring& codecPrivateData );

        static void CheckAudioTrackAttributes( AutoRefPtr<IManifestTrack> track, uint32_t bitrate, uint32_t audioTag, const wstring& codecPrivateData );
        
        static void CheckStreamByName( const VectorOfStreams& givenStreamVector, MediaStreamType type, const string& name );

        static VectorOfStreams SelectionStreamByIndex( const VectorOfStreams& completeStreamVector, int32_t indices[], int32_t size );

        static VectorOfStreams ToggledStreamByMediaType( const VectorOfStreams& completeStreamVector, const VectorOfStreams& givenStreamVector, MediaStreamType type );

        static VectorOfStreams SelectNextAudioStream( const VectorOfStreams& completeStreamVector, const VectorOfStreams& givenStreamVector, bool isPrevious = false );

        static VectorOfStreams SelectAllStreamsByMediaType( const VectorOfStreams& completeStreamVector, MediaStreamType type );

        static VectorOfStreams GetAllSparseStreams(const VectorOfStreams& givenStreamVector);

        static VectorOfStreams SelectFirstStreamOfAudioAndVideo(const VectorOfStreams& completeStreamVector );

        static VectorOfStreams AddTextStream(const VectorOfStreams& completeStreamVector, const VectorOfStreams& givenStreamVector, bool isChild);

        static void CheckDefaultStreamSelection( const VectorOfStreams& completeStreamVector, const VectorOfStreams& givenStreamVector );

        static void CompareTwoVectors( const VectorOfStreams& fistVector, const VectorOfStreams& secondVector );
        
        static void CheckTracksCount( const AutoRefPtr<IManifestStream> pManifest, size_t expectedTracksCount );

    };
};
