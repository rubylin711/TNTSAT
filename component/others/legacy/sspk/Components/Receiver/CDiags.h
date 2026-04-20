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
// Extracing HAL Decoder diagnostics
// ===============================================================================================================

typedef struct _decoderDiagnostics
{
    int iFramesDecoded;
    int iFramesDropped;
    int iFrameErrors;
    int iBufferUnderruns;
    int iSamplesDecoded;
    int iSamplesDropped;
    int iSampleErrors;
} DECODER_DIAGNOSTICS;

// ===============================================================================================================
// Event used to post thread timings
// ===============================================================================================================

enum ThreadName
{
    ThreadName_Unknown,
    ThreadName_DiagsThread,
    ThreadName_DvrWriterThread,
    ThreadName_DecoderThread,
    ThreadName_Max,
};

class CDiagsCpuUsageEvent : public IDiagsEvent
{
public:
    CDiagsCpuUsageEvent()
        : IDiagsEvent()
    {}
    CDiagsCpuUsageEvent(ThreadName name, DWORD last, DWORD average, DWORD maximum, DWORD total)
        : IDiagsEvent()
        , Name(name)
        , CpuUsageLast(last)
        , CpuUsageAverage(average)
        , CpuUsageMaximum(maximum)
        , CpuUsageTotalUsed(total)
    {}
    virtual ~CDiagsCpuUsageEvent() {}

public:
    __override void DiagsGetEventData(void);
    __override const WCHAR* DiagsGetEventMessage(void);
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_CpuUsage; }
    __override bool DiagsIsError(void) { return false; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return IDiagsEvent::DiagsSerializeEventDataLength(version) + 2 + 4 * 4;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        IDiagsEvent::DiagsSerializeEventData(data);

        DiagsPackUint16(data, (uint16) Name);
        DiagsPackUint32(data, CpuUsageLast);
        DiagsPackUint32(data, CpuUsageAverage);
        DiagsPackUint32(data, CpuUsageMaximum);
        DiagsPackUint32(data, CpuUsageTotalUsed);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        if (!IDiagsEvent::DiagsDeSerializeEventData(version, data))
            return false;

        uint16 name;
        DiagsUnpackUint16(data, name); Name = (ThreadName)name;
        DiagsUnpackUint32(data, CpuUsageLast);
        DiagsUnpackUint32(data, CpuUsageAverage);
        DiagsUnpackUint32(data, CpuUsageMaximum);
        DiagsUnpackUint32(data, CpuUsageTotalUsed);
        return true;
    }

public:
    //Thread posting this event
    ThreadName Name;
    //Time used by the thread posting this event
    uint32     CpuUsageLast;
    uint32     CpuUsageAverage;
    uint32     CpuUsageMaximum;
    //Total CPU used
    uint32     CpuUsageTotalUsed;
};

// ===============================================================================================================
// UDP authentication errors
// ===============================================================================================================

class UdpAuthNoActiveKey : public IDiagsEvent
{
public:
    UdpAuthNoActiveKey()
        : IDiagsEvent()
    {}
    UdpAuthNoActiveKey(uint32 version, uint64 count)
        : IDiagsEvent()
        , Version(version)
        , Count(count)
    {}
    virtual ~UdpAuthNoActiveKey() {}

public:
    __override void DiagsGetEventData(void);
    __override const WCHAR* DiagsGetEventMessage(void) { return (WCHAR*) L"UdpAuthProtectionFailure"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_UdpAuthNoActiveKey; }
    __override bool DiagsIsError(void) { return true; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return IDiagsEvent::DiagsSerializeEventDataLength(version) + 4 + 8;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        IDiagsEvent::DiagsSerializeEventData(data);

        DiagsPackUint32(data, Version);
        DiagsPackUint64(data, Count);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        if (!IDiagsEvent::DiagsDeSerializeEventData(version, data))
            return false;

        DiagsUnpackUint32(data, Version);
        DiagsUnpackUint64(data, Count);
        return true;
    }

public:
    uint32 Version;
    uint64 Count;
};

class UdpAuthProtectionFailure : public IDiagsEvent
{
public:
    UdpAuthProtectionFailure()
        : IDiagsEvent()
    {}
    UdpAuthProtectionFailure(uint32 version, const void* keyId, int32 keyLength, int32 errorCode)
        : IDiagsEvent()
        , Version(version)
        , ActiveKeyLength(keyLength)
        , ErrorCode(errorCode)
    {
        memcpy_s(&ActiveKeyId, sizeof(GUID), keyId, sizeof(ActiveKeyId));
    }
    UdpAuthProtectionFailure(uint32 version, int32 keyLength, int32 errorCode)
        : IDiagsEvent()
        , Version(version)
        , ActiveKeyLength(keyLength)
        , ErrorCode(errorCode)
    {
        memset(&ActiveKeyId, 0, sizeof(ActiveKeyId));
    }
    virtual ~UdpAuthProtectionFailure() {}

public:
    __override void DiagsGetEventData(void);
    __override const WCHAR* DiagsGetEventMessage(void) { return (WCHAR*) L"UdpAuthProtectionFailure"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_UdpAuthProtectionFailure; }
    __override bool DiagsIsError(void) { return true; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return IDiagsEvent::DiagsSerializeEventDataLength(version) + 4 + 16 + 2*4;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        IDiagsEvent::DiagsSerializeEventData(data);

        DiagsPackUint32(data, Version);
        DiagsPackGuid(data, ActiveKeyId);
        DiagsPackInt32(data, ActiveKeyLength);
        DiagsPackInt32(data, ErrorCode);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        if (!IDiagsEvent::DiagsDeSerializeEventData(version, data))
            return false;

        DiagsUnpackUint32(data, Version);
        DiagsUnpackGuid(data, ActiveKeyId);
        DiagsUnpackInt32(data, ActiveKeyLength);
        DiagsUnpackInt32(data, ErrorCode);
        return true;
    }

public:
    uint32 Version;
    GUID   ActiveKeyId;
    int32  ActiveKeyLength;
    int32  ErrorCode;
};

// ===============================================================================================================
// ===============================================================================================================
