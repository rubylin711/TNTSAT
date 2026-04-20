///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/// <summary>
///     CHalClock.h
///     Provides simple IHalClock mapping to IPTV_HAL calls
/// </summary>

#pragma once

#include "IHalClock.h"

// ===============================================================================================================
// ===============================================================================================================

class CHalClock : public IHalClock
{
public:
    CHalClock(bool bHardwareClock);
    virtual ~CHalClock();

    __override uint32   AddRef(void);
    __override uint32   Release(void);

    __override LPVOID   GetContext(void);

    __override bool     Init(void);
    __override bool     SetTime(uint64 stc);
    __override uint64   GetTime(void);
    __override bool     Start(void);
    __override bool     Stop(void);
    __override bool     SetSlew(uint32 numerator, uint32 denominator);
    __override bool     SetSpeed(uint32 numerator, uint32 denominator);

private:
    long    m_nRefCount;
    LPVOID  m_pHalClock;
};

// ===============================================================================================================
// ===============================================================================================================
