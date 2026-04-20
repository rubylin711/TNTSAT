///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/// <summary>
///     IHalClock.h - Interface for platform dependent clock functionality
/// </summary>

#pragma once

// ===============================================================================================================
// ===============================================================================================================

class IHalClock
{
public:
    virtual ~IHalClock() {}

    virtual uint32  AddRef(void) = 0;
    virtual uint32  Release(void) = 0;

    virtual LPVOID  GetContext(void) = 0;

    virtual bool    Init(void) = 0;
    virtual bool    SetTime(uint64 stc) = 0;
    virtual uint64  GetTime(void) = 0;
    virtual bool    Start(void) = 0;
    virtual bool    Stop(void) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
