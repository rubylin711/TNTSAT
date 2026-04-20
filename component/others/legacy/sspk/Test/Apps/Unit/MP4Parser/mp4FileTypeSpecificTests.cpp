///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "mp4FileTypeSpecificTests.h"
#include "MP4Feed.h"
#include <iostream>
#include <string>
#include <fstream>

#include <pkTestFramework.h>

using namespace std;

    MP4ParserTest::MP4ParserTest(pkHANDLE hLog)
    {
        _hLog = hLog;

    }

    void MP4ParserTest::Init(pkHANDLE hLog, FILE* myfile, void * source, int length)
    {
        uint32 offset = 0;
        UnitMP4Streamer streamer(myfile);
        if (_TestMP4Parser.Prefetch(MP4Atom::eAtomType_mdat, &streamer, (uint8*)source, length, &offset) > 0)
        {
            _TestMP4Parser.Init(MP4Feed::MP4Feed_Memory, source, offset);
        }

    }

    bool MP4ParserTest::MP4ParserAnalysis(uint32 ChunkType, uint32 Encrypted, uint32 FileSize)
    {
        bool result = true;

        if (_TestMP4Parser.Parse() == false)
        {
            if (ChunkType == 5)
            {
                TF_Logging_Printf(_hLog, "EXPECTED: MP4 Parsing to fail for invalid Chunk Type \n");
                TF_Logging_Printf(_hLog, "ACTUAL: MP4 parsing failed\n");
            }
            else
            {
                TF_Logging_Printf(_hLog, "EXPECTED: MP4 Parsing to Pass. \n");
                TF_Logging_Printf(_hLog, "ACTUAL: MP4 parsing failed\n");
                result = false;
            }
        }
        else
        {
                TF_Logging_Printf(_hLog, "EXPECTED: MP4 Parsing to pass \n");
                TF_Logging_Printf(_hLog, "ACTUAL: MP4 Parsing passed \n");
        }

        BaseMP4Info* info = _TestMP4Parser.GetInfo();

        _bytesParsed = info->_bytesParsed;

        if (ChunkType != 5)
        {
            if (_bytesParsed != FileSize)
            {
                TF_Logging_Printf(_hLog, "Bytes parsed does not match chunk file size \n");
                result = false;
            }
        }

        if (result == 1)
        {
            TF_Logging_Printf(_hLog, "TEST PASSED \n");
        }

        if (result == 0)
        {
            TF_Logging_Printf(_hLog, "TEST FAILED \n");
        }

        TF_Logging_Printf(_hLog, "*******************END OF TEST************************* \n");

        return result;
    }

    UnitMP4Streamer::UnitMP4Streamer(FILE* myfile)
    {
        _myfile = myfile;
    }

    UnitMP4Streamer::~UnitMP4Streamer()
    {
    }

    int32 UnitMP4Streamer::RecvCount(__out_ecount(dstlen) byte* dst, int32 dstlen, int32 dataLength, int32 timeout)
    {
        ASSERT(dstlen >= dataLength);

        int retBytes = fread(dst, 1, dataLength, _myfile);

        return retBytes;
    }

