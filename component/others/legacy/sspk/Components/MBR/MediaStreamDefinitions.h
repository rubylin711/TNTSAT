///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IManifestStream.h"

enum MediaStreamSubType
{
    MediaStreamSubTypeUnknown      = 0,
    MediaStreamSubTypeScript       = 1,
    MediaStreamSubTypeCaptions     = 2,
    MediaStreamSubTypeSubtitles    = 3,
    MediaStreamSubTypeDescriptions = 4,
    MediaStreamSubTypeWMV7         = 0x00800001,
    MediaStreamSubTypeWMV8         = 0x00800002,
    MediaStreamSubTypeWMV9         = 0x00800003,
    MediaStreamSubTypeWVC1         = 0x00800004,
    MediaStreamSubTypeAVC          = 0x00800005,
    MediaStreamSubTypeWMA          = 0x00400001,
    MediaStreamSubTypeWMAPro       = 0x00400002,
    MediaStreamSubTypeMP3          = 0x00400003,
    MediaStreamSubTypeAAC          = 0x00400004,
    MEDIASTREAMSUBTYPE_FORCE_DWORD = 0xffffffff
};

//Combine type and subtype into 32-bit uint
#define MEDIASTREAM_TYPE_HASH(type,subtype) ((type << 24) | subtype)

enum MediaStreamSourceDiagonosticKind
{
    BufferLevelInMilliseconds                       = 1,
    DownloadProgressInPrecent                       = 2,
    MEDIASTREAMSOURCEDIAGONOSTICKIND_FORCE_DWORD    = 0xffffffff
};
