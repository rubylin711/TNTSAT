///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

// ===============================================================================================================
// Managing Pipe Ids in AV Engine
// ===============================================================================================================

class CPipeId
{
public:
    //Static const pipe ids in numerical form
    //To avoid doing the conversion from string to numerical values everytime we need them
    static const uint32 PipeIdN_Fullscreen;
    static const uint32 PipeIdN_FPip;
    static const uint32 PipeIdN_Pip;
    static const uint32 PipeIdN_CC708b;
    static const uint32 PipeIdN_Subtitles;
    static const uint32 PipeIdN_Teletext;
    static const uint32 PipeIdN_DVR;
    static const uint32 PipeIdN_WHDVR;
};

// ===============================================================================================================
// ===============================================================================================================

uint32 PipeId_StringToU32(const std::string& pipeId);
uint32 PipeId_XMLStringToU32(const std::wstring& pipeId);

// ===============================================================================================================
// ===============================================================================================================
