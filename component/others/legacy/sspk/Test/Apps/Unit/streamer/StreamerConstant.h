///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <string>

using namespace std;

string HttpHeaderString = "Accept-Encoding: deflate\r\n";
string HttpOkString = "1.1 200 OK";
string HttpContentTypeString = "Content-Type: ";
string HttpContentLengthString = "Content-Length: ";
string HttpContentEncodingString = "Content-Encoding:";

uint HttpContentTypeStringLength = HttpContentTypeString.length();
uint HttpContentLengthStringLength = HttpContentLengthString.length();
uint HttpContentEncodingStringLength = HttpContentEncodingString.length();

string HttpTextXmlContentString = "text/xml";
string HttpVideoMP4ContentString = "video/mp4";
string HttpAudioMP4ContentString = "audio/mp4";
