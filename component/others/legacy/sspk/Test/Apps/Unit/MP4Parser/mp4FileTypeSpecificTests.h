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
#include "MP4Info.h"
#include "MP4Parser.h"

#include <pkTestFramework.h>

using namespace std;


class MP4ParserTest
{
private:

public:
    FMP4Parser _TestMP4Parser;
    pkHANDLE _hLog;

    uint32 _bytesParsed;

    MP4ParserTest(pkHANDLE hLog/*, void * source, int length*/);

    void Init(pkHANDLE hLog, FILE* myfile, void * source, int length);


    bool MP4ParserAnalysis(uint32 ChunkType, uint32 Encrypted, uint32 FileSize);
};

class UnitMP4Streamer : public MP4Streamer
{
private:
    FILE* _myfile;

public:
    UnitMP4Streamer(FILE* myfile);

    ~UnitMP4Streamer();

    int32 RecvCount(__out_ecount(dstlen) byte* dst, int32 dstlen, int32 dataLength, int32 timeout);

};
