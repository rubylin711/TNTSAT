///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "STUtilities.h"
#include "StringUtils.h"
#include <time.h>

using namespace SSPKTest;
using namespace SSPK;

const int64_t STUtilities::GetSystemTime()
{
    FILETIME currentTime;
    GetSystemTimeAsFileTime(&currentTime);
    return (((int64_t) currentTime.dwHighDateTime << 32) | currentTime.dwLowDateTime);
}

const string STUtilities::ConvertToTimeString(int64_t time)
{
    int64_t currentTimeMilli = TimeSpan_ms::ConvertFrom(TimeSpan_NTP::FromTicks(time)).Ticks();

    int64_t milliSeconds = currentTimeMilli % MILLISECONDS_PER_SECOND;

    int64_t currentTimeSeconds = currentTimeMilli / MILLISECONDS_PER_SECOND;
    
    return (toString64(currentTimeSeconds / 3600) + ":" + toString64(((currentTimeSeconds % 3600) / 60)) + ":" + toString64((currentTimeSeconds % 60))) + "." + toString64(milliSeconds);
}
