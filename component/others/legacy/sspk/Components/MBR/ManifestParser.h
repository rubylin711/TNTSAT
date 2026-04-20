///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CXmlParser.h"
#include "CChunkManifest.h"

namespace MBR
{
//Attribute names

#define MBR_MS_UINT64_PLAY_DURATION         L"MBR_MS_UINT64_DURATION_HNS"
#define MBR_MS_UINT64_STARTPOSITION         L"MBR_MS_UINT64_STARTPOSITION_HNS"
#define MBR_MS_BLOB_PLAYREADY_OBJECT        L"MBR_MS_BLOB_PLAYREADY_OBJECT"
#define MBR_MS_BLOB_WMDRM_OBJECT            L"MBR_MS_BLOB_WMDRM_OBJECT"

enum EManifestMode
{
    eManifestMode_Initial,
    eManifestMode_Subsequent
};

// Simple parser class to hide CManifestParsingCallback and CXmlParser
class CManifestParser
{
public:
    CManifestParser(CChunkManifest* pChunkManifest)
        : m_apChunkManifest(pChunkManifest)
    { }

    pkRESULT Parse(_In_ HANDLE hStream, 
                   _In_ PFN_READCB readCallback, 
                   _In_opt_ EManifestMode mode = eManifestMode_Initial, 
                   _Out_opt_ CXmlParser::EEncodingType* pEncodingType = NULL);

private:
    AutoRefPtr<CChunkManifest> m_apChunkManifest;
};

} // namespace MBR
