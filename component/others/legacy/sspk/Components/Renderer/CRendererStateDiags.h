///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CReceiverDiags.h"
#include "Trace.h"
#include <string>

// ===============================================================================================================
// ===============================================================================================================

extern std::string ISO639ToString(uint32 iso639LanguageCode);

// ===============================================================================================================
// Bad access control parameters
// ===============================================================================================================

class CDiagsReceiverAccessControlBadTimeBoundary : public CDiagsReceiverEvent
{
public:
    CDiagsReceiverAccessControlBadTimeBoundary()
        : CDiagsReceiverEvent()
    {}
    virtual ~CDiagsReceiverAccessControlBadTimeBoundary() {}

public:
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.AC.BadTimeBoundary"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Receiver_AccessControlBadTimeBoundary; }
    __override bool DiagsIsError(void) { return true; }
};

// ===============================================================================================================
// Audio/Subtitle Language change event
// ===============================================================================================================

class CDiagsReceiverLanguageEvent : public CDiagsReceiverEvent
{
public:
    CDiagsReceiverLanguageEvent()
        : CDiagsReceiverEvent()
    {}
    CDiagsReceiverLanguageEvent(int iso639Language, int type, int pid, int mode)
        : CDiagsReceiverEvent()
        , Iso639Language(iso639Language)
        , Type(type)
        , Pid(pid)
        , Mode(mode)
    {}
    virtual ~CDiagsReceiverLanguageEvent() {}

public:
    __override void DiagsGetEventData(void)
    {
        CDiagsReceiverEvent::DiagsGetEventData();

        DiagsLogString(L"Language", ISO639ToString((uint32)Iso639Language).c_str());
        DiagsLogValue(L"Type", Type);
        DiagsLogValue(L"Pid", Pid);
        DiagsLogValue(L"Mode", Mode);
    }
    __override bool DiagsIsError(void) { return false; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return CDiagsReceiverEvent::DiagsSerializeEventDataLength(version) + 16;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        CDiagsReceiverEvent::DiagsSerializeEventData(data);

        DiagsPackInt32(data, Iso639Language);
        DiagsPackInt32(data, Type);
        DiagsPackInt32(data, Pid);
        DiagsPackInt32(data, Mode);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        CDiagsReceiverEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackInt32(data, Iso639Language);
        DiagsUnpackInt32(data, Type);
        DiagsUnpackInt32(data, Pid);
        DiagsUnpackInt32(data, Mode);
        return true;
    }

public:
    int Iso639Language;
    int Type;
    int Pid;
    int Mode;
};

class CDiagsReceiverAudioLanguageEvent : public CDiagsReceiverLanguageEvent
{
public:
    CDiagsReceiverAudioLanguageEvent()
        : CDiagsReceiverLanguageEvent()
    {}
    CDiagsReceiverAudioLanguageEvent(int iso639Language, int type, int pid, int mode)
        : CDiagsReceiverLanguageEvent(iso639Language, type, pid, mode)
    {}
    virtual ~CDiagsReceiverAudioLanguageEvent() {}

public:
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.Language.Audio"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Receiver_AudioLanguage; }
};

class CDiagsReceiverAudioDescriptionLanguageEvent : public CDiagsReceiverLanguageEvent
{
public:
    CDiagsReceiverAudioDescriptionLanguageEvent()
        : CDiagsReceiverLanguageEvent()
    {}
    CDiagsReceiverAudioDescriptionLanguageEvent(int iso639Language, int type, int pid, int mode)
        : CDiagsReceiverLanguageEvent(iso639Language, type, pid, mode)
    {}
    virtual ~CDiagsReceiverAudioDescriptionLanguageEvent() {}

public:
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.Language.AudioDesc"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Receiver_AudioDescriptionLanguage; }
};

class CDiagsReceiverSubtitleLanguageEvent : public CDiagsReceiverLanguageEvent
{
public:
    CDiagsReceiverSubtitleLanguageEvent()
        : CDiagsReceiverLanguageEvent()
    {}
    CDiagsReceiverSubtitleLanguageEvent(int iso639Language, int type, int pid, int mode)
        : CDiagsReceiverLanguageEvent(iso639Language, type, pid, mode)
    {}
    virtual ~CDiagsReceiverSubtitleLanguageEvent() {}

public:
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.Language.Subtitle"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Receiver_SubtitleLanguage; }
};

// ===============================================================================================================
// Audio Description update
// ===============================================================================================================

class CDiagsReceiverAudioDescriptionVolumeEvent : public CDiagsReceiverEvent
{
public:
    CDiagsReceiverAudioDescriptionVolumeEvent()
        : CDiagsReceiverEvent()
    {}
    CDiagsReceiverAudioDescriptionVolumeEvent(int volume)
        : CDiagsReceiverEvent()
        , Volume(volume)
    {}
    virtual ~CDiagsReceiverAudioDescriptionVolumeEvent() {}

public:
    __override void DiagsGetEventData(void)
    {
        CDiagsReceiverEvent::DiagsGetEventData();

        DiagsLogValue(L"Volume", Volume);
    }
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.AudioDescVolume"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Receiver_AudioDescriptionVolume; }
    __override bool DiagsIsError(void) { return false; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return CDiagsReceiverEvent::DiagsSerializeEventDataLength(version) + 4;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        CDiagsReceiverEvent::DiagsSerializeEventData(data);

        DiagsPackInt32(data, Volume);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        CDiagsReceiverEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackInt32(data, Volume);
        return true;
    }

public:
    int Volume;
};

// ===============================================================================================================
// ===============================================================================================================

class CDiagsReceiverOplEvent : public CDiagsReceiverEvent
{
public:
    CDiagsReceiverOplEvent()
        : CDiagsReceiverEvent()
    {}
    CDiagsReceiverOplEvent(uint32 level)
        : CDiagsReceiverEvent()
        , Level(level)
    {}
    virtual ~CDiagsReceiverOplEvent() {}

public:
    __override void DiagsGetEventData(void)
    {
        CDiagsReceiverEvent::DiagsGetEventData();

        DiagsLogValue(L"Level", Level);
    }
    __override bool DiagsIsError(void) { return false; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return CDiagsReceiverEvent::DiagsSerializeEventDataLength(version) + 4;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        CDiagsReceiverEvent::DiagsSerializeEventData(data);

        DiagsPackUint32(data, Level);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        CDiagsReceiverEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackUint32(data, Level);
        return true;
    }

public:
    uint32 Level;
};

class CDiagsReceiverMacrovisionDRMEvent : public CDiagsReceiverOplEvent
{
public:
    CDiagsReceiverMacrovisionDRMEvent()
        : CDiagsReceiverOplEvent()
    {}
    CDiagsReceiverMacrovisionDRMEvent(uint32 level)
        : CDiagsReceiverOplEvent(level)
    {}
    virtual ~CDiagsReceiverMacrovisionDRMEvent() {}

public:
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.OPL.DRM.Macrovision"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Receiver_Macrovision_DRM; }
};

class CDiagsReceiverMacrovisionECMEvent : public CDiagsReceiverOplEvent
{
public:
    CDiagsReceiverMacrovisionECMEvent()
        : CDiagsReceiverOplEvent()
    {}
    CDiagsReceiverMacrovisionECMEvent(uint32 level)
        : CDiagsReceiverOplEvent(level)
    {}
    virtual ~CDiagsReceiverMacrovisionECMEvent() {}

public:
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.OPL.ECM.Macrovision"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Receiver_Macrovision_ECM; }
};

class CDiagsReceiverMacrovisionWSSEvent : public CDiagsReceiverOplEvent
{
public:
    CDiagsReceiverMacrovisionWSSEvent()
        : CDiagsReceiverOplEvent()
    {}
    CDiagsReceiverMacrovisionWSSEvent(uint32 level)
        : CDiagsReceiverOplEvent(level)
    {}
    virtual ~CDiagsReceiverMacrovisionWSSEvent() {}

public:
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.OPL.WSS.Macrovision"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Receiver_Macrovision_WSS; }
};

class CDiagsReceiverCgmsaDRMEvent : public CDiagsReceiverOplEvent
{
public:
    CDiagsReceiverCgmsaDRMEvent()
        : CDiagsReceiverOplEvent()
    {}
    CDiagsReceiverCgmsaDRMEvent(uint32 level)
        : CDiagsReceiverOplEvent(level)
    {}
    virtual ~CDiagsReceiverCgmsaDRMEvent() {}

public:
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.OPL.DRM.CGMSA"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Receiver_CGMSA_DRM; }
};

class CDiagsReceiverCgmsaECMEvent : public CDiagsReceiverOplEvent
{
public:
    CDiagsReceiverCgmsaECMEvent()
        : CDiagsReceiverOplEvent()
    {}
    CDiagsReceiverCgmsaECMEvent(uint32 level)
        : CDiagsReceiverOplEvent(level)
    {}
    virtual ~CDiagsReceiverCgmsaECMEvent() {}

public:
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.OPL.ECM.CGMSA"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Receiver_CGMSA_ECM; }
};

class CDiagsReceiverCgmsaWSSEvent : public CDiagsReceiverOplEvent
{
public:
    CDiagsReceiverCgmsaWSSEvent()
        : CDiagsReceiverOplEvent()
    {}
    CDiagsReceiverCgmsaWSSEvent(uint32 level)
        : CDiagsReceiverOplEvent(level)
    {}
    virtual ~CDiagsReceiverCgmsaWSSEvent() {}

public:
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.OPL.WSS.CGMSA"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Receiver_CGMSA_WSS; }
};

// ===============================================================================================================
// ===============================================================================================================
