///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CMbrConfiguration.h"
#include "StringUtils.h"
#include "Trace.h"
#include <string>
#include <vector>
using namespace std;

static const int32 c_ChunklistDefaultSize = 10800; // 6 hours of 2 second chunks
static const int32 c_ChunklistMaxSize = 86400;     // 48 hours of 2 second chunks
static const int32 c_ChunklistMinSize = 900;       // 30 min of 2 second chunks
// ===============================================================================================================
// Global configuration parameters controlling MBR behavior
// ===============================================================================================================

CMbrConfiguration::CMbrConfiguration()
    : ChunklistMaxSize(c_ChunklistDefaultSize)
{
}

//Process commands
bool CMbrConfiguration::Command(const string& command, const vector<string>& args)
{
    //Number of arguments
    size_t numargs = args.size();

    //Max chunklist size
    if (command == "chunklistmaxsize")
    {
        if (numargs == 1)
        {
            if(atoi(args[0].c_str()) >= c_ChunklistMinSize && atoi(args[0].c_str()) <= c_ChunklistMaxSize)
            {
                ChunklistMaxSize = atoi(args[0].c_str());
                TRACE(("ChunklistMaxSize=%d", ChunklistMaxSize));
                return true;
            }
            else
            {
                TRACE_ERROR(("Invalid chunklistmaxsize=%d, valid range (%d,%d)", 
                    atoi(args[0].c_str()),
                    c_ChunklistMinSize,
                    c_ChunklistMaxSize));
            }
        }
    }

    return false;
}

// ===============================================================================================================
// ===============================================================================================================

CMbrConfiguration gMbrConfiguration;

// ===============================================================================================================
// ===============================================================================================================
