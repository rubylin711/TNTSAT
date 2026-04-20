///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDiagsProvider.h"
#include "CDiagsManagerConfiguration.h"

//#define IDIAGSPROVIDER_TRACE_ENABLE
#ifdef IDIAGSPROVIDER_TRACE_ENABLE
#define IDIAGSPROVIDER_TRACE(x) TRACE(x)
#else
#define IDIAGSPROVIDER_TRACE(x)
#endif

// ===============================================================================================================
// IDiagsProvider interfaces used by most objects to extract diagnostics information periodically.
// Any object wiching to provide periodic information should inherit from this interface.
// ===============================================================================================================

void IDiagsProvider::DiagsInit(void)
{
    //First event will be sent after 60 seconds
    _diagsNextSendTime = gDiagsManagerConfiguration.DiagsFirstUpdateAfterTuneTime;
}

void IDiagsProvider::DiagsReset(void)
{
    //First event will be sent after 60 seconds
    _diagsNextSendTime = gDiagsManagerConfiguration.DiagsFirstUpdateAfterTuneTime;
}

IDiagsEvent* IDiagsProvider::DiagsGeneratePeriodicEvent(int32 ticks)
{
    _diagsNextSendTime -= ticks;
    if (_diagsNextSendTime <= 0)
    {
        _diagsNextSendTime = gDiagsManagerConfiguration.DiagsPeriodicUpdateAfterTime;
        return DiagsRetrieve();
    }
    return NULL;
}

// ===============================================================================================================
// ===============================================================================================================
