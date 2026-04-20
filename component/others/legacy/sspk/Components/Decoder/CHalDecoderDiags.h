///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CDiags.h"

// ===============================================================================================================
// SD only tunes flag set/clear by the application
// ===============================================================================================================

class CDiagsSDOnlyTuneEvent : public IDiagsEvent
{
public:
    CDiagsSDOnlyTuneEvent()
        : IDiagsEvent()
    {}
    CDiagsSDOnlyTuneEvent(bool isSDOnlyTune)
        : IDiagsEvent()
        , IsSDOnlyTune(isSDOnlyTune)
    {}
    virtual ~CDiagsSDOnlyTuneEvent() {}

public:
    __override void DiagsGetEventData(void) {}
    __override const WCHAR* DiagsGetEventMessage(void) { return IsSDOnlyTune ? L"AV.SDOnlyTunes.ON" : L"AV.SDOnlyTunes.OFF"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Receiver_SDOnlyTune; }
    __override bool DiagsIsError(void) { return false; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return IDiagsEvent::DiagsSerializeEventDataLength(version) + 1;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        IDiagsEvent::DiagsSerializeEventData(data);

        DiagsPackBool(data, IsSDOnlyTune);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        IDiagsEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackBool(data, IsSDOnlyTune);
        return true;
    }

public:
    bool IsSDOnlyTune;
};

// ===============================================================================================================
// SD only tunes flag set/clear by the application
// ===============================================================================================================

class CDiagsKOKTuneEvent : public IDiagsEvent
{
public:
    CDiagsKOKTuneEvent()
        : IDiagsEvent()
    {}
    CDiagsKOKTuneEvent(bool isKOKOn)
        : IDiagsEvent()
        , IsKOKOn(isKOKOn)
    {}
    virtual ~CDiagsKOKTuneEvent() {}

public:
    __override void DiagsGetEventData(void) {}
    __override const WCHAR* DiagsGetEventMessage(void) { return IsKOKOn ? L"AV.KOKTunes.ON" : L"AV.KOKTunes.OFF"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Receiver_KOKTune; }
    __override bool DiagsIsError(void) { return false; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return IDiagsEvent::DiagsSerializeEventDataLength(version) + 1;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        IDiagsEvent::DiagsSerializeEventData(data);

        DiagsPackBool(data, IsKOKOn);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        IDiagsEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackBool(data, IsKOKOn);
        return true;
    }

public:
    bool IsKOKOn;
};

// ===============================================================================================================
// ===============================================================================================================
