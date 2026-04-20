///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IDiagsEvent.h"
#include "CSocketDefinitions.h"

#include <strsafe.h>

// ===============================================================================================================
// Base event class used for various socket related events
// Fills in basic information about the current tune parameters and
// common properties for all types of supported sockets
// ===============================================================================================================

class CDiagsSocketEvent : public IDiagsEvent
{
public:
    CDiagsSocketEvent()
        : IDiagsEvent()
        , SocketType(eSocketType_Unknown)
        , PipeIdN(kDiagsPipeIdN_FullScreen)
        , MediaTransportId(0)
        , UniqueId(0)
        , ContentId(GUID_NULL)
        , Channel(-1)
        , BitRate(0)
    {}
    virtual ~CDiagsSocketEvent() {}

public:
    __override void DiagsGetEventData(void);

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return IDiagsEvent::DiagsSerializeEventDataLength(version) + 4*6+16;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        IDiagsEvent::DiagsSerializeEventData(data);

        DiagsPackUint32(data, (uint32)SocketType);
        DiagsPackUint32(data, PipeIdN);
        DiagsPackUint32(data, MediaTransportId);
        DiagsPackUint32(data, UniqueId);
        DiagsPackGuid(data, ContentId);
        DiagsPackInt32(data, Channel);
        DiagsPackUint32(data, BitRate);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        IDiagsEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackUint32(data, SocketType);
        DiagsUnpackUint32(data, PipeIdN);
        DiagsUnpackUint32(data, MediaTransportId);
        DiagsUnpackUint32(data, UniqueId);
        DiagsUnpackGuid(data, ContentId);
        DiagsUnpackInt32(data, Channel);
        DiagsUnpackUint32(data, BitRate);
        return true;
    }

public:
    //Type of socket
    eSocketType SocketType;
    //Pipe Id
    uint32      PipeIdN;
    //Unique ids associated with this tune
    uint32      MediaTransportId;
    uint32      UniqueId;
    GUID        ContentId;
    //Channel number
    int         Channel;
    //Overall bitrates
    uint32      BitRate;
};

// ===============================================================================================================
// Sent when a socket is connected
// ===============================================================================================================

class CDiagsSocketOpenedEvent : public CDiagsSocketEvent
{
public:
    CDiagsSocketOpenedEvent(const char* tuneUrl)
        : CDiagsSocketEvent()
    {
        if (tuneUrl)
        {
            if( pkFAILED( StringCbCopyA( TuneUrl, sizeof(TuneUrl), tuneUrl ) ) )
            {
                // This is a benign assertion.
                // StringCchCopyA truncates the string on error.
                // It's OK to truncate a long URL for the diagnostic event.

                pkASSERT( false );
            }
        }
        else
        {
            TuneUrl[0] = 0;
        }
    }

    virtual ~CDiagsSocketOpenedEvent() {}

public:
    __override void DiagsGetEventData(void)
    {
        //Base class stuff
        CDiagsSocketEvent::DiagsGetEventData();

        DiagsLogString(L"TuneURL", TuneUrl);
    }
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.Tuner.Opened"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Socket_Opened; }
    __override bool DiagsIsError(void) { return false; }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return CDiagsSocketEvent::DiagsSerializeEventDataLength(version) + SOCKET_MAX_URL_LENGTH;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        CDiagsSocketEvent::DiagsSerializeEventData(data);

        DiagsPackChars(data, TuneUrl, SOCKET_MAX_URL_LENGTH);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        CDiagsSocketEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackChars(data, TuneUrl, SOCKET_MAX_URL_LENGTH);
        return true;
    }

public:
    char TuneUrl[SOCKET_MAX_URL_LENGTH];
};

// ===============================================================================================================
// Sent when a socket is closed
// ===============================================================================================================

class CDiagsSocketClosedEvent : public CDiagsSocketEvent
{
public:
    CDiagsSocketClosedEvent()
        : CDiagsSocketEvent()
    {}
    virtual ~CDiagsSocketClosedEvent() {}

public:
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.Tuner.Closed"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Socket_Closed; }
    __override bool DiagsIsError(void) { return false; }
};

// ===============================================================================================================
// Periodic diagnostics event generated by each socket currently active in the system
// ===============================================================================================================

class CDiagsSocketUpdateEvent : public CDiagsSocketEvent
{
public:
    CDiagsSocketUpdateEvent()
        : CDiagsSocketEvent()
    {}
    virtual ~CDiagsSocketUpdateEvent() {}

public:
    __override void DiagsGetEventData(void);
    __override const WCHAR* DiagsGetEventMessage(void) { return L"AV.Update.Tuner"; }
    __override kDiagsEvent DiagsGetEventType(void) { return kDiagsEvent_Socket_Update; }
    __override bool DiagsIsError(void) { return false; }

    __override LogStorageMode DiagsLogStorageMode(void)
    {
        return kLogStorageMode_Persist;
    }
    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        return CDiagsSocketEvent::DiagsSerializeEventDataLength(version) + 2*4;
    }
    __override void DiagsSerializeEventData(byte*& data)
    {
        CDiagsSocketEvent::DiagsSerializeEventData(data);

        DiagsPackUint32(data, PacketsProcessed);
        DiagsPackUint32(data, PacketsWithParseErrors);
    }
    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        CDiagsSocketEvent::DiagsDeSerializeEventData(version, data);

        DiagsUnpackUint32(data, PacketsProcessed);
        DiagsUnpackUint32(data, PacketsWithParseErrors);
        return true;
    }

public:
    //Number of RTP packets received
    uint32      PacketsProcessed;
    //Number of RTP packets with parse errors
    uint32      PacketsWithParseErrors;
};

// ===============================================================================================================
// ===============================================================================================================

