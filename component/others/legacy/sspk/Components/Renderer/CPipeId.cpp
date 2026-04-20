///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CPipeId.h"
#include <string>
using namespace std;

// ===============================================================================================================
// Managing Pipe Ids in AV Engine
// ===============================================================================================================

//Static pipe ids in integer forms
//To avoid doing the math everytime we need them
const uint32 CPipeId::PipeIdN_Fullscreen    = PipeId_StringToU32(string("FULLSCREEN"));
const uint32 CPipeId::PipeIdN_FPip          = PipeId_StringToU32(string("FPIP"));
const uint32 CPipeId::PipeIdN_Pip           = PipeId_StringToU32(string("PIP"));
const uint32 CPipeId::PipeIdN_CC708b        = PipeId_StringToU32(string("CC708b"));
const uint32 CPipeId::PipeIdN_Subtitles     = PipeId_StringToU32(string("SUBTITLES"));
const uint32 CPipeId::PipeIdN_Teletext      = PipeId_StringToU32(string("TELETEXT"));
const uint32 CPipeId::PipeIdN_DVR           = PipeId_StringToU32(string("DVR"));
const uint32 CPipeId::PipeIdN_WHDVR         = PipeId_StringToU32(string("RDVR"));

uint32 PipeId_StringToU32(const string& pipeId)
{
    uint32 value = 0;
    for (uint32 i=0; i<4 && i<pipeId.length(); i++)
    {
        value |= (pipeId[i] & 0xff) << (8 * (3-i));
    }
    return value;
}

uint32 PipeId_XMLStringToU32(const wstring& pipeId)
{
    uint32 value = 0;
    for (uint32 i=0; i<4 && i<pipeId.length(); i++)
    {
        value |= (pipeId[i] & 0xff) << (8 * (3-i));
    }
    return value;
}

// ===============================================================================================================
// ===============================================================================================================
