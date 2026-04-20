///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSPKHelpers.h"
#include "PKTestSuite.h"
#include "PKTestSuiteUtils.h"
#include "StringUtils.h"
#include "DRMAcquireLicense.h"

using namespace SSPKTest;

static const int32_t HTTP_PROTOCOL_INDEX = 7;
static const int32_t MANIFEST_POSTFIX_SIZE = 8;

const string SSPKHelpers::GetUrlFromList(CXMLElementsList xManifests, const wchar_t* nameAttribute, bool ignoreLicenseAcquisition)
{
    
    if(!ignoreLicenseAcquisition)
    {
        AcquireLicenseForContent(xManifests, nameAttribute);
    }

    string url = GetAttributeValueFromList(xManifests, nameAttribute, L"url");
    LogTestComment("URL: %s", url.c_str());

    return url;

}

const string SSPKHelpers::GetUrlFromList(CXMLElementsList xManifests, const char* nameAttribute, bool ignoreLicenseAcquisition)
{
    return GetUrlFromList(xManifests, Str2WStr(nameAttribute).c_str(), ignoreLicenseAcquisition);
}

const bool SSPKHelpers::AcquireLicenseForContent(CXMLElementsList xManifests, const wchar_t* nameAttribute)
{
        string licenseServerURL = "";
        string keyID = "";
        string customData = "";

        if(AttributeValueExistsInList( xManifests, nameAttribute, L"licenseServerURL"))
        {
            licenseServerURL = GetAttributeValueFromList( xManifests, nameAttribute, L"licenseServerURL");

            //Custom data (optional)
            customData = GetAttributeValueFromList( xManifests, nameAttribute, L"customData");
            if("" != customData)
            {
                customData = unescape(customData);
            }

            //Key ID required
            keyID = SSPKHelpers::GetAttributeValueFromList( xManifests, nameAttribute, L"licenseKeyID");
        }

        //Try to acquire license only if the necessary fields are present
        if (licenseServerURL != "" && keyID != "")
        {
            LogTestComment("Attempting to acquire license, as the key ID and the license server URL are given(keyID= %s, license server= %s", keyID.c_str(), licenseServerURL.c_str());
            PKTEST_ASSERT_MSG_EXIT( AcquireLicense( licenseServerURL, keyID, customData ), "Failed to Get RootLicense" );
            return true;
        }

    exit:
        return false;
}

const bool SSPKHelpers::AttributeValueExistsInList(CXMLElementsList xManifests, const wchar_t* nameAttribute, const wchar_t* attribute)
{
    for(int32_t i = 0; i < xManifests.Length(); i++)
    {
        CXMLAttributesList currentElementAttributes = xManifests[i].Attributes();
        if(wcscmp(currentElementAttributes[L"name"].Value(), nameAttribute) == 0)
        {
            if( "" != escapeSpaces( wstring_to_string( currentElementAttributes[attribute].Value())))
            {
                return true;
            }
        }
    }

    return false;
}

const bool SSPKHelpers::AcquireLicense(string licenseServerURL, string keyID, string customData)
{
    //Create and fill in license acquirer
    CLicenseAcquirer licenseAcquirer;
    HRESULT hr = licenseAcquirer.Initialize();
    if (FAILED(hr))
    {
        LogTestComment("FAILED to initialize LicenseAcquirer: 0x%X\n", hr);
        return false;

    }
    licenseAcquirer.ChallengeCustomData = customData;

    //Try to acquire license
    string response = "";
    hr = licenseAcquirer.AcquireLicense(licenseServerURL.c_str(), keyID.c_str(), &response);
    if (FAILED(hr))
    {
        LogTestComment("FAILED to acquire license from server: %s: 0x%X\n", licenseServerURL.c_str(), hr);
        return false;
    }
    else
    {
        LogTestComment("License acquired");
        if (0 != response.length())
        {
            LogTestComment("AcquireLicense response custom data:\n%s\n", response.c_str());
        }
    }

    return true;
}

const string SSPKHelpers::GetAttributeValueFromList(CXMLElementsList xManifests, const wchar_t* nameAttribute, const wchar_t* attribute)
{
    string attributeValue = "";
    for(int32_t i = 0; i < xManifests.Length(); i++)
    {
        CXMLAttributesList currentElementAttributes = xManifests[i].Attributes();
        if(wcscmp(currentElementAttributes[L"name"].Value(), nameAttribute) == 0)
        {
            attributeValue = escapeSpaces( wstring_to_string( currentElementAttributes[attribute].Value() ) );
            break;
        }
    }

    PKTEST_ASSERT_MSG_EXIT(attributeValue != "", "Failed to find the attribute %ls for %ls in the xml file", attribute, nameAttribute);
    
exit:
    return attributeValue;
}

string SSPKHelpers::GetLocalUrl(string url)
{
        url = url.substr(HTTP_PROTOCOL_INDEX, url.size() - HTTP_PROTOCOL_INDEX );
        if(toLower(url).find("manifest") != string::npos)
        {
            url = url.substr(0, url.size() - (MANIFEST_POSTFIX_SIZE + 1));
        }
        int32_t index = url.find_first_of('/');
        return url.substr(index, url.size() - index);
}

string SSPKHelpers::GetServerNameFromUrl(string url)
{
        url = url.substr(HTTP_PROTOCOL_INDEX, url.size() - HTTP_PROTOCOL_INDEX);
        int32_t index = url.find_first_of('/');
        return url.substr(0, index);
}

string SSPKHelpers::GetCR_ReturnStatusCodeProfile(const string& localUrl, const string& requestType, const int32_t& statusCode, const string& additionalConditions, const string& additionalResults)
{
    return GetCR_ReturnStatusCodeProfile(localUrl, requestType, "video", statusCode, additionalConditions, additionalResults);
}

string SSPKHelpers::GetCR_ReturnStatusCodeProfile(const string& localUrl, const string& requestType, const string& streamName, const int32_t& statusCode, const string& additionalConditions, const string& additionalResults)
{
    string returnString = "<profile>\n <clientIP address=\"\">\n  <request url=\"" + localUrl + "\" ";
    
     if(requestType.length() > 0)
    {
            returnString += " requestType=\"" + requestType + "\" ";
    }
    
    if(streamName.length() > 0)
    {
            returnString += " fragmentType=\"" + streamName + "\" ";
    }
    
    if (additionalConditions != "")
    {
      returnString += additionalConditions;
    }
    
    returnString += " >\n     <statusCode value=\"" + to_string(statusCode) + "\" />\n  ";

    if (additionalResults != "")
    {
        returnString += "<" + additionalResults + "/>\n";
    }

    returnString += " </request>\n </clientIP>\n</profile>";

    return returnString;
}

string SSPKHelpers::GetCR_SegmentManifestReturnStatusCodeProfile(const int64_t currentStartTime, const int64_t segmentLength, const string& localUrl, const string& returnCode)
{
    string options = "<statusCode value=\"" + returnCode + "\" />";
    return GetCR_SegmentManifestProfile(currentStartTime, segmentLength, localUrl, options);
}

string SSPKHelpers::GetCR_SegmentManifestLatencyProfile(const int64_t currentStartTime, const int64_t segmentLength, const string& localUrl, const int64_t latency)
{
    string options = "<delayInMilliseconds mean=\"" + toString64(latency) + "\" />\n";
    return GetCR_SegmentManifestProfile(currentStartTime, segmentLength, localUrl, options);
}

string SSPKHelpers::GetCR_SegmentManifestProfile(const int64_t currentStartTime, const int64_t segmentLength, const string& localUrl, const string& options)
{
    int64_t segmentTimestamp = 0;
    string returnString;
    segmentTimestamp = currentStartTime - ( currentStartTime % segmentLength );

    returnString ="<profile>\n <clientIP address=\"\">\n <request url=\"" + localUrl + "/Segments(" + toString64(segmentTimestamp) + ")\" requestType=\"manifest\">\n " + options + " </request>\n </clientIP>\n </profile>";
    
    return returnString;
}



string SSPKHelpers::GetCR_SetMaxNetworkCap(const string& localUrl, const int32_t& maxBandWidth )
{
    string returnString = "<profile>\n <clientIP address=\"\">\n  <request url=\"" + localUrl + "\" requestType=\"fragments\" ";
        
    returnString += " >\n     <throttle kbs=\"" + to_string(maxBandWidth) + "\" />\n  ";
    
    returnString += " </request>\n </clientIP>\n</profile>";

    return returnString;
}

void SSPKHelpers::ClientResponderPost(const string& url, const string& server, const string& body)
{
    vector<string> additionalHeaders;

    HttpPost(url, server, additionalHeaders, body);
}

bool SSPKHelpers::CompleteTracePost(const string& url, const string& server, const string& ClientName, const string& testCase, const string& body)
{
    vector<string> additionalHeaders;

    additionalHeaders.push_back("heurprofile:" + testCase);
    additionalHeaders.push_back("clientType:SSPK");

    return HttpPost(url, server, additionalHeaders, body);
}

bool SSPKHelpers::HttpPost(const string& url, const string& server, const vector<string>& additionalHeaders, const string& body)
{
    string httpRequest;

    httpRequest =  "POST " + url + " HTTP/1.1\r\n";
    httpRequest += "Accept: */*\r\n";
    httpRequest += "Accept-Language: en-US\r\n";
    httpRequest += "Content-Length: ";
    httpRequest += toString(body.size());
    httpRequest += "\r\n";
    httpRequest += "Content-Type: text/xml\r\n";
    httpRequest += "Accept-Encoding: gzip, deflate\r\n";
    httpRequest += "User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)\r\n";
    httpRequest += "Host: " + server + "\r\n";
    httpRequest += "Connection: Keep-Alive\r\n";
    for(size_t i = 0; i<additionalHeaders.size(); i++)
    {
        httpRequest += additionalHeaders[i] + "\r\n";
    }
    httpRequest += "Pragma: no-cache\r\n\r\n";

    TestStreamerHttp* streamer = new TestStreamerHttp();

    LogTestComment(httpRequest.c_str());
    LogTestComment(body.c_str());

    if( streamer->HttpRequestResponse(server,httpRequest,body) )
    {
        LogTestComment("POST Succeded.");
        delete streamer;
        return true;
    }
    else
    {
        LogTestError("POST failed");
        delete streamer;
        return false;
    }
}

void SSPKHelpers::CheckStreamCountByType( const VectorOfStreams& givenStreamVector, int32_t argAudioStreamCount, int32_t argVideoStreamCount, int32_t argOtherStreamCount )
{
    int32_t audioStreamCount = 0;
    int32_t videoStreamCount = 0;
    int32_t otherStreamCount = 0;

    for(uint32_t i=0;i<givenStreamVector.size();i++)
    {
        if((givenStreamVector[i])->Type() == MediaStreamTypeAudio)
        {
            audioStreamCount++;
        }
        else if((givenStreamVector[i])->Type() == MediaStreamTypeVideo)
        {
            videoStreamCount++;
        }
        else
        {
            otherStreamCount++;
        }
    }
    PKTEST_ASSERT_MSG_EXIT( audioStreamCount == argAudioStreamCount, "Audio stream count(%d) is different from expected(%d)", audioStreamCount, argAudioStreamCount );
    PKTEST_ASSERT_MSG_EXIT( videoStreamCount == argVideoStreamCount, "Video stream count(%d) is different from expected(%d)", videoStreamCount, argVideoStreamCount );
    PKTEST_ASSERT_MSG_EXIT( otherStreamCount == argOtherStreamCount, "Other stream count(%d) is different from expected(%d)", otherStreamCount, argOtherStreamCount );

exit:
    return;
}

void SSPKHelpers::CheckTracksCount( const AutoRefPtr<IManifestStream> apManifestStream, size_t expectedTracksCount )
{
    VectorOfTracks tracks;
    size_t numTracks;

    PKTEST_HRESULT_EXIT( apManifestStream->GetAvailableTracks(&tracks) );

    numTracks = tracks.size();
    
    switch(apManifestStream->Type())
    {
        case MediaStreamTypeAudio:
            PKTEST_ASSERT_MSG_EXIT( numTracks == expectedTracksCount, "Audio tracks count(%d) is different from expected(%d)", numTracks, expectedTracksCount );
            break;

        case MediaStreamTypeVideo:
            PKTEST_ASSERT_MSG_EXIT( numTracks == expectedTracksCount, "Video tracks count(%d) is different from expected(%d)", numTracks, expectedTracksCount );
            break;

        case MediaStreamTypeText:
            PKTEST_ASSERT_MSG_EXIT( numTracks == expectedTracksCount, "Text tracks count(%d) is different from expected(%d)", numTracks, expectedTracksCount );
            break;

        case MediaStreamTypeBinary:
            PKTEST_ASSERT_MSG_EXIT( numTracks == expectedTracksCount, "Binary tracks count(%d) is different from expected(%d)", numTracks, expectedTracksCount );
            break;

        default:
            PKTEST_ASSERT_MSG_EXIT( numTracks == expectedTracksCount, "Unknown tracks count(%d) is different from expected(%d)", numTracks, expectedTracksCount );
            break;
    }

exit:
    return;
}

void SSPKHelpers::CheckManifestAttributes( AutoRefPtr<IManifest> pManifest, uint32_t majorVersion, uint32_t minorVersion, int64_t timeScale, int64_t duration, bool isLive, uint32_t lookAheadCount, int64_t dvrWindowLength )
{
    PKTEST_ASSERT_MSG_EXIT( pManifest->MajorVersion() == majorVersion, "Major manifest version(%d) is different from expected(%d)", pManifest->MajorVersion(), majorVersion );
    PKTEST_ASSERT_MSG_EXIT( pManifest->MinorVersion() == minorVersion,  "Minor manifest version(%d) is different from expected(%d)", pManifest->MinorVersion(), minorVersion );
    PKTEST_ASSERT_MSG_EXIT( pManifest->TimeScale() == timeScale,  "Timescale(%lld) is different from expected(%lld)", pManifest->TimeScale(), timeScale );
    PKTEST_ASSERT_MSG_EXIT( pManifest->Duration() == duration,  "Duration of the presentation(%lld) is different from expected(%lld)", pManifest->Duration(), duration );
    PKTEST_ASSERT_MSG_EXIT( pManifest->IsLive() == isLive, "Available audio stream count(%d) is different from expected(%d)", pManifest->IsLive(), isLive );
    PKTEST_ASSERT_MSG_EXIT( pManifest->LookAheadCount() == lookAheadCount,  "Look ahead count in manifest(%d) is different from expected(%d)", pManifest->LookAheadCount(), lookAheadCount );
    PKTEST_ASSERT_MSG_EXIT( pManifest->DVRWindowLength() == dvrWindowLength,  "length of the DVR window(%lld) is different from expected(%lld)", pManifest->DVRWindowLength(), dvrWindowLength );

exit:
    return;
}

void SSPKHelpers::CheckManifestAttributeValue( AutoRefPtr<IManifest> pManifest, const wstring& name, int64_t expectedValue, bool presentInManifest )
{
    SSPKHelpers::CheckManifestAttributeValue( pManifest, name, toWString(expectedValue), presentInManifest );
}

void SSPKHelpers::CheckManifestAttributeValue( AutoRefPtr<IManifest> pManifest, const wstring& name, const wstring& expectedValue, bool presentInManifest )
{
    wstring value;

    if(presentInManifest)
    {
        PKTEST_ASSERT_MSG_EXIT( pManifest->GetAttribute( name, &value), 
                                "%ls attribute not found in manifest", 
                                name.c_str() );

        PKTEST_ASSERT_MSG_EXIT( value == expectedValue , 
                                "%ls=%ls . Manifest Attribute value is not as expected(%ls)", 
                                name.c_str(), value.c_str(), expectedValue.c_str() );
    }
    else
    {
        PKTEST_ASSERT_MSG_EXIT( !pManifest->GetAttribute( name, &value), 
                                "%ls attribute should not be found in manifest", 
                                name.c_str() );
    }
exit:
    return;
}


void SSPKHelpers::CheckStreamAttributes( AutoRefPtr<IManifestStream> currentStream, 
                                         int64_t timeScaleArg, 
                                         const string& languageArg, 
                                         uint32_t maxWidthArg, 
                                         uint32_t maxHeightArg, 
                                         uint32_t displayWidthArg, 
                                         uint32_t displayHeightArg, 
                                         const string& urlArg, 
                                         const string& nameArg, 
                                         MediaStreamType typeArg, 
                                         const string& subTypeArg )
{
    wstring expectedtype;

    //Comparing the Get Attribute.
    if ( typeArg == MediaStreamTypeVideo )
    {
        expectedtype = L"VIDEO";
    }
    else if ( typeArg == MediaStreamTypeAudio )
    {
        expectedtype = L"AUDIO";
    }
    else if ( typeArg == MediaStreamTypeText )
    {
        expectedtype = L"TEXT";
    }
    else if ( typeArg == MediaStreamTypeBinary )
    {
        expectedtype = L"BINARY";
    }
    
    PKTEST_ASSERT_MSG_EXIT( currentStream->TimeScale() == timeScaleArg, "Timescale(%lld) is different from expected(%lld)", currentStream->TimeScale(), timeScaleArg );
    PKTEST_ASSERT_MSG_EXIT( (currentStream->Language().compare(Str2WStr(languageArg) ) == 0),  "language(%ls) is different from expected(%s)", currentStream->Language().c_str(), languageArg.c_str() );
    PKTEST_ASSERT_MSG_EXIT( currentStream->MaxWidth() == maxWidthArg,  "The Max width(%d) is different from expected(%d)", currentStream->MaxWidth(), maxWidthArg );
    PKTEST_ASSERT_MSG_EXIT( currentStream->MaxHeight() == maxHeightArg,  "The Max height(%d) is different from expected(%d)", currentStream->MaxHeight(), maxHeightArg );
    PKTEST_ASSERT_MSG_EXIT( currentStream->DisplayWidth() == displayWidthArg,  "The display width(%d) is different from expected(%d)", currentStream->DisplayWidth(), displayWidthArg) ;
    PKTEST_ASSERT_MSG_EXIT( currentStream->DisplayHeight() == displayHeightArg,  "The display height(%d) is different from expected(%d)", currentStream->DisplayHeight(), displayHeightArg) ;
    PKTEST_ASSERT_MSG_EXIT( (currentStream->Url().compare(Str2WStr(urlArg) ) == 0),  "Url(%ls) is different from expected(%s)", currentStream->Url().c_str(), urlArg.c_str() );
    PKTEST_ASSERT_MSG_EXIT( (currentStream->Name().compare(Str2WStr(nameArg) ) == 0),  "Stream Name(%ls) is different from expected(%s)", currentStream->Name().c_str(), nameArg.c_str() );
    PKTEST_ASSERT_MSG_EXIT( currentStream->Type() == typeArg,  "Stream Type is different from expected" );
    PKTEST_ASSERT_MSG_EXIT( (currentStream->SubType().compare(Str2WStr(subTypeArg) ) == 0),  "Stream subtype(%ls) is different from expected(%s)", currentStream->SubType().c_str(), subTypeArg.c_str() );

    CheckStreamAttributeValue(currentStream, L"TimeScale", timeScaleArg, true);
    CheckStreamAttributeValue(currentStream, L"Language", Str2WStr(languageArg), true);
    CheckStreamAttributeValue(currentStream, L"MaxWidth", maxWidthArg, true);
    CheckStreamAttributeValue(currentStream, L"MaxHeight", maxHeightArg, true);
    CheckStreamAttributeValue(currentStream, L"DisplayWidth", displayWidthArg, true);
    CheckStreamAttributeValue(currentStream, L"DisplayHeight", displayHeightArg, true);
    CheckStreamAttributeValue(currentStream, L"Url", Str2WStr(urlArg), true);
    CheckStreamAttributeValue(currentStream, L"Name", Str2WStr(nameArg), true);
    CheckStreamAttributeValue(currentStream, L"Type", expectedtype, true);
    CheckStreamAttributeValue(currentStream, L"Subtype", Str2WStr(subTypeArg), true);
    CheckStreamAttributeValue(currentStream, L"Foo", 0, false);

exit:
    return;
}



void SSPKHelpers::CheckStreamAttributeValue( AutoRefPtr<IManifestStream> currentStream, const wstring& name, int64_t expectedValue, bool presentInManifest )
{
    SSPKHelpers::CheckStreamAttributeValue( currentStream, name, toWString(expectedValue), presentInManifest );
}

void SSPKHelpers::CheckStreamAttributeValue( AutoRefPtr<IManifestStream> currentStream, const wstring& name, const wstring& expectedValue, bool presentInManifest )
{
    wstring value;

    if(presentInManifest)
    {
        PKTEST_ASSERT_MSG_EXIT( currentStream->GetAttribute( name, &value), 
                                "%ls attribute not found in stream", 
                                name.c_str() );

        PKTEST_ASSERT_MSG_EXIT( value == expectedValue , 
                                "%ls=%ls . Stream Attribute value is not as expected(%ls)", 
                                name.c_str(), value.c_str(), expectedValue.c_str() );
    }
    else
    {
        PKTEST_ASSERT_MSG_EXIT( !currentStream->GetAttribute( name, &value), 
                                "%ls attribute should not be found in stream", 
                                name.c_str() );
    }
exit:
    return;
}

void SSPKHelpers::CheckVideoTrackAttributes(  AutoRefPtr<IManifestTrack> track, 
                                uint32_t audioTag,
                                uint32_t bitrate,
                                const wstring& fourCC,
                                uint32_t hardwareProfile,
                                uint32_t maxHeight,
                                uint32_t maxWidth,
                                uint32_t trackIndex,
                                uint32_t nominalBitrate, 
                                const wstring& expectedCodecPrivateData )
{
    uint32_t iFourCC = 0;

    CodecPrivateDataBuffer vExpectedCodecPrivateData;
    wstring codecPrivateDataVal;
    
    PKTEST_HRESULT_EXIT( BytesToHexStr( track->CodecPrivateData().data(), track->CodecPrivateData().size(), &codecPrivateDataVal) );

    PKTEST_HRESULT_EXIT( vExpectedCodecPrivateData.ParseFromHexStr( expectedCodecPrivateData.c_str(), expectedCodecPrivateData.length(), 0 ) );

    PKTEST_HRESULT_EXIT( StrToFourCC(fourCC.c_str(), &iFourCC) );

    PKTEST_ASSERT_MSG_EXIT( audioTag == track->AudioTag(), "AudioTag(%d) is different from expected(%d)", track->AudioTag(), audioTag);
    PKTEST_ASSERT_MSG_EXIT( bitrate == track->Bitrate(),  "Bitrate(%d) is different from expected(%d)", track->Bitrate(), bitrate);

    PKTEST_ASSERT_MSG_EXIT( iFourCC == track->FourCC(),  "FourCC(%d) is different from expected(%d)", track->FourCC(), iFourCC);

    PKTEST_ASSERT_MSG_EXIT( hardwareProfile == track->HardwareProfile(),  "HardwareProfile(%d) is different from expected(%d)", track->HardwareProfile(), hardwareProfile);
    PKTEST_ASSERT_MSG_EXIT( maxHeight == track->MaxHeight(),  "MaxHeight(%d) is different from expected(%d)", track->MaxHeight(), maxHeight);
    PKTEST_ASSERT_MSG_EXIT( maxWidth == track->MaxWidth(),  "MaxWidth(%d) is different from expected(%d)", track->MaxWidth(), maxWidth);
    PKTEST_ASSERT_MSG_EXIT( trackIndex == track->TrackIndex(),  "TrackIndex(%d) is different from expected(%d)", track->TrackIndex(), trackIndex);
    PKTEST_ASSERT_MSG_EXIT( nominalBitrate == track->NominalBitrate(),  "NominalBitrate(%d) is different from expected(%d)", track->NominalBitrate(), nominalBitrate);

    PKTEST_ASSERT_MSG_EXIT( vExpectedCodecPrivateData == track->CodecPrivateData(),  
                            "CodecPrivateData(%ls) is different from expected(%ls)", 
                            codecPrivateDataVal.c_str(), expectedCodecPrivateData.c_str() );

    CheckTrackAttributeValue(track, L"AudioTag", audioTag, false);
    CheckTrackAttributeValue(track, L"Bitrate", bitrate, true);
    CheckTrackAttributeValue(track, L"FourCC", fourCC, true);
    CheckTrackAttributeValue(track, L"HardwareProfile", hardwareProfile, false);
    CheckTrackAttributeValue(track, L"MaxHeight", maxHeight, true);
    CheckTrackAttributeValue(track, L"MaxWidth", maxWidth, true);
    CheckTrackAttributeValue(track, L"Index", trackIndex, true);
    CheckTrackAttributeValue(track, L"NominalBitrate", nominalBitrate, false);
    CheckTrackAttributeValue(track, L"CodecPrivateData", expectedCodecPrivateData, true);

exit:
    return;
}

void SSPKHelpers::CheckAudioTrackAttributes(    AutoRefPtr<IManifestTrack> track, 
                                        uint32_t bitrate,
                                        uint32_t audioTag,
                                        const wstring& codecPrivateData )
{
    wstring codecPrivateDataVal;
    int32_t codecPrivateDataPosition;

    PKTEST_ASSERT_MSG_EXIT( bitrate == track->Bitrate(),  "Bitrate(%d) is different from expected(%d)", track->Bitrate(), bitrate);
    PKTEST_ASSERT_MSG_EXIT( audioTag == track->AudioTag(),  "AudioTag(%d) is different from expected(%d)", track->AudioTag(), audioTag);

    PKTEST_HRESULT_EXIT( BytesToHexStr( track->CodecPrivateData().data(), track->CodecPrivateData().size(), &codecPrivateDataVal) );
        
    codecPrivateDataPosition = codecPrivateDataVal.find(codecPrivateData);
    PKTEST_ASSERT_MSG_EXIT( codecPrivateDataPosition != -1,  
                            "Audio CodecPrivateData (%ls) does not contain expected (%ls)", 
                            codecPrivateDataVal.c_str(), codecPrivateData.c_str() );

    CheckTrackAttributeValue(track, L"AudioTag", audioTag, false);
    CheckTrackAttributeValue(track, L"Bitrate", bitrate, true);
    CheckTrackAttributeValue(track, L"FourCC", L"", false);
    CheckTrackAttributeValue(track, L"HardwareProfile", 0, false);
    CheckTrackAttributeValue(track, L"MaxHeight", 0, false);
    CheckTrackAttributeValue(track, L"MaxWidth", 0, false);
    CheckTrackAttributeValue(track, L"Index", 0, false);
    CheckTrackAttributeValue(track, L"NominalBitrate", 0, false);

exit:
    return;
}

void SSPKHelpers::CheckTrackAttributeValue( AutoRefPtr<IManifestTrack> currentTrack, const wstring& name, int64_t expectedValue, bool presentInManifest )
{
    SSPKHelpers::CheckTrackAttributeValue( currentTrack, name, toWString(expectedValue), presentInManifest );
}

void SSPKHelpers::CheckTrackAttributeValue( AutoRefPtr<IManifestTrack> currentTrack, const wstring& name, const wstring& expectedValue, bool presentInManifest )
{
    wstring value;

    if(presentInManifest)
    {
        PKTEST_ASSERT_MSG_EXIT( currentTrack->GetAttribute( name, &value), 
                                "%ls attribute not found in track", 
                                name.c_str() );

        PKTEST_ASSERT_MSG_EXIT( value == expectedValue , 
                                "%ls=%ls . Track Attribute value is not as expected(%ls)", 
                                name.c_str(), value.c_str(), expectedValue.c_str() );
    }
    else
    {
        PKTEST_ASSERT_MSG_EXIT( !currentTrack->GetAttribute( name, &value), 
                                "%ls attribute shoul not be found in track", 
                                name.c_str() );
    }
exit:
    return;
}

void SSPKHelpers::CheckStreamByName( const VectorOfStreams& givenStreamVector, MediaStreamType type, const string& name )
{
    bool streamFound = false;

    for(uint32_t i=0;i<givenStreamVector.size();i++)
    {
        if(givenStreamVector[i]->Type() != type)
        {
            continue;
        }
        else
        {
            if(givenStreamVector[i]->Name().compare(Str2WStr(name) ) == 0)
            {
                streamFound = true;
                break;
            }


        }
    }
    PKTEST_ASSERT_MSG_EXIT(streamFound, "Couldn't find any Stream with given name(%s) for given type", name.c_str() );
exit:
    return;
}

VectorOfStreams SSPKHelpers::SelectionStreamByIndex( const VectorOfStreams& completeStreamVector, int32_t indices[], int32_t size )
{
    VectorOfStreams proposedSelection;

    for (int32_t i = 0; i < size; ++i)
    {
        proposedSelection.push_back( completeStreamVector[indices[i]] );
    }

    return proposedSelection;
}

VectorOfStreams SSPKHelpers::ToggledStreamByMediaType( const VectorOfStreams& completeStreamVector, const VectorOfStreams& givenStreamVector, MediaStreamType type )
{
    VectorOfStreams proposedSelection;
    VectorOfStreams selectedTogglingStreams;
    IManifestStream* parentStream ;

    for(uint32_t j=0; j<givenStreamVector.size(); j++)
    {
        givenStreamVector[j]->GetParentStream(&parentStream);
        if(givenStreamVector[j]->Type() != type)
        {
            if(NULL == parentStream)
            {
                proposedSelection.push_back(givenStreamVector[j]);
            }
            else
            {
                if( parentStream->Type() != type )
                {
                    proposedSelection.push_back(givenStreamVector[j]);
                }
            }
        }
    }
    parentStream = NULL;

    for( uint32_t i = 0; i<completeStreamVector.size(); i++ )
    {
        if( completeStreamVector[i]->Type() == type )
        {
            VectorOfStreams::const_iterator it = std::find( givenStreamVector.begin(), givenStreamVector.end(), completeStreamVector[i] );
            if ( it == givenStreamVector.end() )
            {
                // If its audio or video, it doesn't make sense to add multiple streams to the list. So when we toggle, we actually chose one stream for each audio and video which was not chose earlier.
                if (completeStreamVector[i]->Type() == MediaStreamTypeAudio || completeStreamVector[i]->Type() == MediaStreamTypeVideo)
                {
                    proposedSelection.push_back(completeStreamVector[i]);
                    break;
                }
                else
                {
                    completeStreamVector[i]->GetParentStream(&parentStream);
                    if(NULL == parentStream)
                    {
                        proposedSelection.push_back(completeStreamVector[i]);
                    }
                    else
                    {
                        VectorOfStreams::const_iterator parentIt = std::find( proposedSelection.begin(), proposedSelection.end(), completeStreamVector[i] );
                        if ( parentIt != proposedSelection.end() )
                        {
                            proposedSelection.push_back(completeStreamVector[i]);
                        }
                    }
                }
            }
        }
    }

    return proposedSelection;
}

VectorOfStreams SSPKHelpers::SelectNextAudioStream( const VectorOfStreams& completeStreamVector, const VectorOfStreams& givenStreamVector, bool isPrevious )
{
    VectorOfStreams proposedSelection;
    AutoRefPtr<IManifestStream> apSelectedAudio;
    AutoRefPtr<IManifestStream> apNextAudio;

    // Count how many audio streams we currently have
    int32_t iNumAudio = 0;
    for (size_t i = 0; i < completeStreamVector.size(); ++i)
    {
        if (MediaStreamTypeAudio == completeStreamVector[i]->Type())
        {
            ++iNumAudio;
        }
    }

    if (iNumAudio < 2)
    {
        // There is nothing to switch, so return selected Streams
        return givenStreamVector;
    }
    else
    {
        // Find the current selected audio stream
        for (size_t i = 0; i < givenStreamVector.size(); ++i)
        {
            if (MediaStreamTypeAudio == givenStreamVector[i]->Type())
            {
                apSelectedAudio.Set(givenStreamVector[i]);
            }
            else
            {
                proposedSelection.push_back(givenStreamVector[i]);
            }
        }

        // Find the index of the current selected audio in the available stream list
        size_t iCurrentIndex = 0;
        for (; iCurrentIndex < completeStreamVector.size(); ++iCurrentIndex)
        {
            if (apSelectedAudio == completeStreamVector[iCurrentIndex])
            {
                break;
            }
        }

        if(!isPrevious)
        {
            // Find the next audio stream in the available stream list
            apNextAudio = apSelectedAudio;
            for (size_t i = iCurrentIndex + 1; i < completeStreamVector.size(); ++i)
            {
                if (MediaStreamTypeAudio == completeStreamVector[i]->Type())
                {
                    apNextAudio.Set(completeStreamVector[i]);
                    break;
                }
            }

            if (apNextAudio == apSelectedAudio)
            {
                for (size_t i = 0; i < iCurrentIndex; ++i)
                {
                    if (MediaStreamTypeAudio == completeStreamVector[i]->Type())
                    {
                        apNextAudio.Set(completeStreamVector[i]);
                        break;
                    }
                }
            }
        }
        else
        {
            apNextAudio = apSelectedAudio;
            for (int32_t i = iCurrentIndex - 1; i >= 0 ; --i)
            {
                if (MediaStreamTypeAudio == completeStreamVector[i]->Type())
                {
                    apNextAudio.Set(completeStreamVector[i]);
                    break;
                }
            }

            if (apNextAudio == apSelectedAudio)
            {
                for (size_t i = completeStreamVector.size() - 1; i > iCurrentIndex; --i)
                {
                    if (MediaStreamTypeAudio == completeStreamVector[i]->Type())
                    {
                        apNextAudio.Set(completeStreamVector[i]);
                        break;
                    }
                }
            }
        }

        ASSERT(apNextAudio != apSelectedAudio);
        proposedSelection.push_back(apNextAudio);

        return proposedSelection;
    }
}

VectorOfStreams SSPKHelpers::SelectAllStreamsByMediaType( const VectorOfStreams& completeStreamVector, MediaStreamType type )
{
    VectorOfStreams proposedSelection;

    for( uint32_t i = 0; i<completeStreamVector.size(); i++ )
    {
        if(completeStreamVector[i]->Type() == type)
        {
            proposedSelection.push_back(completeStreamVector[i]);
        }
    }

    return proposedSelection;
}

VectorOfStreams SSPKHelpers::SelectFirstStreamOfAudioAndVideo(const VectorOfStreams& completeStreamVector )
{
    VectorOfStreams proposedSelection;

    bool fFoundVideo = false;
    bool fFoundAudio = false;

    for (size_t i = 0; i < completeStreamVector.size(); ++i)
    {
        if (MediaStreamTypeVideo == completeStreamVector[i]->Type() )
        {
            if (!fFoundVideo)
            {
                proposedSelection.push_back( completeStreamVector[i] );
                fFoundVideo = true;
            }
        }
        else if (MediaStreamTypeAudio == completeStreamVector[i]->Type() )
        {
            if (!fFoundAudio)
            {
                proposedSelection.push_back( completeStreamVector[i] );
                fFoundAudio = true;
            }
        }
    }

    return proposedSelection;
}

VectorOfStreams SSPKHelpers::GetAllSparseStreams(const VectorOfStreams& givenStreamVector)
{
    VectorOfStreams sparseStreams;

    for( size_t i = 0; i<givenStreamVector.size(); i++ )
    {
        if( givenStreamVector[i]->Type() == MediaStreamTypeText )
        {
            IManifestStream* parentStream ;
            givenStreamVector[i]->GetParentStream(&parentStream);

            VectorOfStreams::const_iterator it = std::find( givenStreamVector.begin(), givenStreamVector.end(), parentStream);
            if ( it != givenStreamVector.end() )
            {
                sparseStreams.push_back(givenStreamVector[i]);
            }
        }
    }

    return sparseStreams;
}

VectorOfStreams SSPKHelpers::AddTextStream(const VectorOfStreams& completeStreamVector, const VectorOfStreams& givenStreamVector, bool isChild)
{
    VectorOfStreams proposedSelection;

    for(uint32_t j=0; j < givenStreamVector.size(); j++)
    {
        proposedSelection.push_back(givenStreamVector[j]);
    }
    for( uint32_t i = 0; i < completeStreamVector.size(); i++ )
    {
        if( completeStreamVector[i]->Type() == MediaStreamTypeText )
        {
            VectorOfStreams::const_iterator duplicateIterator = std::find( proposedSelection.begin(), proposedSelection.end(), completeStreamVector[i]);
            if ( duplicateIterator == proposedSelection.end() )
            {
                IManifestStream* parentStream ;
                completeStreamVector[i]->GetParentStream(&parentStream);
                VectorOfStreams::const_iterator it = std::find( givenStreamVector.begin(), givenStreamVector.end(), parentStream);
                if(isChild )
                {
                    if ( it != givenStreamVector.end() )
                    {
                        proposedSelection.push_back(completeStreamVector[i]);
                        break;
                    }
                }
                else
                {
                    if ( it == givenStreamVector.end() )
                    {
                        proposedSelection.push_back(completeStreamVector[i]);
                        break;
                    }
                }
            }
        }
    }

    return proposedSelection;
}

void SSPKHelpers::CheckDefaultStreamSelection( const VectorOfStreams& completeStreamVector, const VectorOfStreams& givenStreamVector )
{
    AutoRefPtr<IManifestStream> videoLastStream;
    AutoRefPtr<IManifestStream> audioLastStream;
    VectorOfStreams::const_iterator it;

    for( uint32_t i = 0; i < completeStreamVector.size(); i++ )
    {
        if( completeStreamVector[i]->Type() == MediaStreamTypeVideo )
        {
            videoLastStream = completeStreamVector[i];
        }
        else if( completeStreamVector[i]->Type() == MediaStreamTypeAudio )
        {
            audioLastStream = completeStreamVector[i];
        }
        else
        {
            it = std::find( givenStreamVector.begin(), givenStreamVector.end(), completeStreamVector[i] );
            PKTEST_ASSERT_MSG_EXIT( it == givenStreamVector.end(),  "One of the text or binary stream is selected by default."  );
        }
    }

    PKTEST_ASSERT_MSG_EXIT(givenStreamVector.size() <= 2, "The count of streams selected by default at manifest ready should not be more than 2(one audio and one video) ");

    if(NULL != videoLastStream)
    {
        it = std::find( givenStreamVector.begin(), givenStreamVector.end(), videoLastStream );
        PKTEST_ASSERT_MSG_EXIT( it != givenStreamVector.end(),  "The Last Video stream in the available streams is not  present in the selected streams."  );
    }

    if( NULL != audioLastStream )
    {
        it = std::find( givenStreamVector.begin(), givenStreamVector.end(), audioLastStream );
        PKTEST_ASSERT_MSG_EXIT( it != givenStreamVector.end(),  "The Last Audio stream in the available streams is not  present in the selected streams."  );
    }

exit:
    return;
}

void SSPKHelpers::CompareTwoVectors( const VectorOfStreams& fistVector, const VectorOfStreams& secondVector )
{
    VectorOfStreams::const_iterator it;

    PKTEST_ASSERT_MSG_EXIT( fistVector.size() == secondVector.size(),  "The count of the two vectors is differnt."  );
    for( uint32_t i = 0; i<fistVector.size(); i++ )
    {
        it = std::find( secondVector.begin(), secondVector.end(), fistVector[i] );
        PKTEST_ASSERT_MSG_EXIT( it != secondVector.end(),  "The vectors are not equal."  );
    }

exit:
    return;
}
