///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/// <summary>
///     CHalClock.cpp
///     Provides IHalClock mapping to IPTV_HAL calls
/// </summary>

#include "stdafx.h"
#include "CHalClock.h"
#include "IPTVDecoderHal.h"
#include "Trace.h"

// ===============================================================================================================
// ===============================================================================================================

CHalClock::CHalClock(bool bHardwareClock)
    : m_nRefCount(1)
{
    m_pHalClock = NULL;

    CE_AV_CLOCK_LOG(CE_AV_CLOCK_ACQUIRE);
    IPTV_HAL_Decoder_Clock_Acquire(bHardwareClock, &m_pHalClock);
    CE_AV_CLOCK_LOG(CE_AV_CLOCK_ACQUIRE_DONE);

    CHECK_ALLOC(m_pHalClock);
}

CHalClock::~CHalClock()
{
    ASSERT(!m_pHalClock);
}

uint32 CHalClock::AddRef(void)
{
    return (uint32)InterlockedIncrement(&m_nRefCount);
}

uint32 CHalClock::Release(void)
{
    uint32 count = (uint32)InterlockedDecrement(&m_nRefCount);
    if (!count)
    {
        CE_AV_CLOCK_LOG(CE_AV_CLOCK_RELEASE);
        IPTV_HAL_Decoder_Clock_Release(m_pHalClock);
        CE_AV_CLOCK_LOG(CE_AV_CLOCK_RELEASE_DONE);

        m_pHalClock = NULL;
        delete this;
    }
    return count;
}

LPVOID CHalClock::GetContext(void)
{
    return m_pHalClock;
}

bool CHalClock::Init(void)
{
    CE_AV_CLOCK_LOG(CE_AV_CLOCK_INIT);
    bool retval = IPTV_HAL_ERROR_RET_SUCCESS(IPTV_HAL_Decoder_Clock_Init(m_pHalClock));
    CE_AV_CLOCK_LOG(CE_AV_CLOCK_INIT_DONE);
    return retval;
}

bool CHalClock::SetTime(uint64 stc)
{
    CE_AV_CLOCK_LOG(CE_AV_CLOCK_SETTIME);
    bool retval = IPTV_HAL_ERROR_RET_SUCCESS(IPTV_HAL_Decoder_Clock_SetTime(m_pHalClock, stc));
    CE_AV_CLOCK_LOG(CE_AV_CLOCK_SETTIME_DONE);
    return retval;
}

uint64 CHalClock::GetTime(void)
{
    uint64 stc = 0;
    return IPTV_HAL_ERROR_RET_SUCCESS(IPTV_HAL_Decoder_Clock_GetTime(m_pHalClock, &stc)) ? stc : 0;
}

bool CHalClock::Start(void)
{
    CE_AV_CLOCK_LOG(CE_AV_CLOCK_START);
    bool retval = IPTV_HAL_ERROR_RET_SUCCESS(IPTV_HAL_Decoder_Clock_Start(m_pHalClock));
    CE_AV_CLOCK_LOG(CE_AV_CLOCK_START_DONE);
    return retval;
}

bool CHalClock::Stop(void)
{
    CE_AV_CLOCK_LOG(CE_AV_CLOCK_STOP);
    bool retval = IPTV_HAL_ERROR_RET_SUCCESS(IPTV_HAL_Decoder_Clock_Stop(m_pHalClock));
    CE_AV_CLOCK_LOG(CE_AV_CLOCK_STOP_DONE);
    return retval;
}

// ===============================================================================================================
// ===============================================================================================================
