///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IReferenceClock.h"

// ===============================================================================================================
// CReferenceClock - provides a common reference clock for all streams (NTP in this implementation)
// ===============================================================================================================

class CReferenceClock : public IReferenceClock
{
public:
    //Constructor
    CReferenceClock() {}
    //Destructor
    virtual ~CReferenceClock() {}
    //Returns time in 10Mhz unit
    __override uint64 GetTime(void)
    {
        // NOTE: offset to 1970 to prevent overflow when converting to NTP 32-bit seconds value
        static const uint64 fileTime1970 = 116444736000000000LL; // 134774 days
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        return (((uint64) ft.dwHighDateTime << 32) | ft.dwLowDateTime) - fileTime1970;
    }

    //returns time in 32.32 fixed binary point (NTP) seconds
    __override uint64 GetTimeEx(void)
    {
        return NTP_10MHZTOUINT64(GetTime());
    }
};

// ===============================================================================================================
// ===============================================================================================================
