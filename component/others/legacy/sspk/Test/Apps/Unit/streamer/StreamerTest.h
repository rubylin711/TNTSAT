///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <iostream>
#include <string>

#include <pkTestFramework.h>

using namespace std;

#include "IStreamerHttp.h"
#include "CTuneRequest.h"
#include <string>

using namespace std;

class StreamerTest
{
private:

public:
    pkHANDLE _hLog;    // logging file

    IStreamerHttp*          _pChunkSocket;    // Streamer

    int        _HttpStatusCode;

    string _ContentType;
    int _ContentLength;

    StreamerTest(pkHANDLE hLog);

    bool Init(string SendMsg);

    bool VerifyStreaming(string SendMsg);

    bool CheckManifest(string MsgSent);

    bool DecidePassOrFail(int length, int type);
};
