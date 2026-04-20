///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "StreamerTest.h"
#include "StreamerConstant.h"
#include <iostream>
#include <fstream>

#include <pkTestFramework.h>

StreamerTest::StreamerTest(pkHANDLE hLog)
{
    _hLog = hLog;
}

bool StreamerTest::Init(string MsgSent)
{
    bool result = true;
    pkRESULT pkresult;

    _HttpStatusCode = 0;
    _ContentType = "";
    _ContentLength = 0;

    _pChunkSocket = IStreamerHttp::CreateStreamerHttp();

    if (NULL == _pChunkSocket)
    {
        pkresult = TF_Logging_Printf(_hLog, "Unable to create HttpStreamer \n");
        return false;
    }

    return result;

}

bool StreamerTest::VerifyStreaming(string SendMsg)
{
    CTuneRequest tuneRequest;
    string httpHeader;
    string response;

    pkRESULT pkresult;

    int position;
    int positionContentType;
    int positionContentEncoding;
    int positionContentLength;

    tuneRequest.ParseUrl(SendMsg);

    bool ret = _pChunkSocket->Connect( &tuneRequest.TunerUrl, httpHeader, &response, IStreamerHttp::eRedirectEnable);

    if (ret == 0)
    {
        return false;
    }

// TODO: bool CStreamerHttp::ParseResponse(string& response, int& http_result)

    position = (response.find("1.1 200 OK", 0));

    if (position > 0)
    {
        _HttpStatusCode = 200;
        pkresult = TF_Logging_Printf(_hLog, "_HttpStatusCode %d \n", _HttpStatusCode);
    }

    positionContentType = (response.find(HttpContentTypeString, 0));
    positionContentLength = (response.find(HttpContentLengthString, 0));
    positionContentEncoding = (response.find(HttpContentEncodingString, 0));

    position = response.length();

    // Add 14 which is number of characters in Content-Type:
    _ContentType = response.substr(positionContentType + HttpContentTypeStringLength, positionContentEncoding - positionContentType - HttpContentTypeStringLength);

    // Add 15 which is number of characters in Content-Length:
    string ContentLength = response.substr(positionContentLength + HttpContentLengthStringLength, position - positionContentLength - HttpContentLengthStringLength);
    _ContentLength = atoi(ContentLength.data());

    pkresult = TF_Logging_Printf(_hLog, "ContentType %s \n", _ContentType.c_str());
    pkresult = TF_Logging_Printf(_hLog, "ContentLength %s \n", ContentLength.c_str());

    return ret;
}


bool StreamerTest::DecidePassOrFail(int ExpectedLength, int ExpectedType)
{
    string TypeToCompare = "";

// TODO: Remove ExpectedType

    // Compare Content Length to expected Value
    if (_ContentLength != ExpectedLength)
    {
        return false;
    }

    if (ExpectedType == 1)
    {
        TypeToCompare = HttpTextXmlContentString;
    }

    if (ExpectedType == 2)
    {
        TypeToCompare = HttpVideoMP4ContentString;
    }

    if (ExpectedType == 3)
    {
        TypeToCompare = HttpAudioMP4ContentString;
    }


    if (_ContentType != TypeToCompare)
    {
        return false;
    }

    return true;
}
