///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

enum MediaStreamType
{
    MediaStreamTypeAudio        = 0,
    MediaStreamTypeVideo        = 1,
    MediaStreamTypeText         = 2,
    MediaStreamTypeBinary       = 3,
    MEDIASTREAMTYPECOUNT        = 4,

    MediaStreamTypeUnknown      = 0xff,

    MEDIASTREAMTYPE_FORCE_DWORD = 0xffffffff
};
