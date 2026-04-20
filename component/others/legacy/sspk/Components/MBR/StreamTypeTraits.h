///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

struct STREAM_TYPE_TRAITS
{
    bool isValid;
    const wchar_t* pszDefaultName;
    bool canBeSparse;
    bool requiresFourCC;
    bool requiresBitrate;
};

const STREAM_TYPE_TRAITS* GetStreamTypeTraits( _In_ MediaStreamType type );

