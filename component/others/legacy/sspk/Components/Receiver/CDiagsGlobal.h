///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IDiagsEvent.h"

// ===============================================================================================================
// Global socket counters
// Global parameters across all streams (not reset on retune)
// Static diags if no live tuner avaialble. Retrieves static total counters only.
// ===============================================================================================================

class CDiagsGlobal : public IDiagsEvent
{
public:
    CDiagsGlobal()
        : IDiagsEvent()
    {}
    virtual ~CDiagsGlobal() {}

public:
    __override void DiagsGetEventData(void)
    {
        DiagsLogValue(L"TotalPacketsReceived", TotalPacketsReceived);
        DiagsLogValue(L"TotalPacketsExpired", TotalPacketsExpired);
        DiagsLogValue(L"TotalHolePackets", TotalHolePackets);
    }
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.Update.Tuner.StaticDiags"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Socket_StaticDiags; }
    __override bool DiagsIsError(void) { return false; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return IDiagsEvent::DiagsSerializeEventDataLength(version) + 4+4+4;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        IDiagsEvent::DiagsSerializeEventData(data);

        DiagsPackUint32(data, TotalPacketsReceived);
        DiagsPackUint32(data, TotalPacketsExpired);
        DiagsPackUint32(data, TotalHolePackets);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        IDiagsEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackUint32(data, TotalPacketsReceived);
        DiagsUnpackUint32(data, TotalPacketsExpired);
        DiagsUnpackUint32(data, TotalHolePackets);
        return true;
    }

public:
    //Total number of packets received across all streams since reboot
    static uint32       TotalPacketsReceived;
    //Counters for tracking total number of holes across all streams since reboot
    static uint32       TotalPacketsExpired;
    static uint32       TotalHolePackets;
};

// ===============================================================================================================
// ===============================================================================================================

