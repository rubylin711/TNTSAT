///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "SmoothErrorDefinitions.h"
#include "CReceiverNotification.h"
#include "DecoderBufferStatus.h"
#include <string>

// ===============================================================================================================
// IReceiver interface - sockets use these to communicate with receivers
// ===============================================================================================================

class  ISocket;
class  IPacket;

class IReceiver
{
public:
    IReceiver() {}
    virtual ~IReceiver() {}
public:
    virtual bool        AttachSocketToDvrReceiver( const std::string& path, ISocket* socket ) = 0;
    virtual bool        DetachSocketFromDvrReceiver( const std::string& path, ISocket* socket ) = 0;
    virtual void        Start(void) = 0;
    virtual void        SignalStop(void) = 0;
    virtual void        Stop( _In_ bool bForced, _In_opt_ eTunerError tunerError = eTunerErrorNone ) = 0;
    virtual void        Error( eTunerError tunerError, eSocketError socketError, pkRESULT pkResult, int httpResponse) = 0;
    virtual bool        WritePacket( IPacket& packet ) = 0;
    virtual void        Notify( ReceiverNotificationType notificationType, CReceiverNotificationData* notificationData ) = 0;
    virtual uint64      GetCurrentPlaybackTime(void) = 0;
    virtual void        GetDecoderBufferStatus( _Out_ DecoderBufferStatus* pDecoderBufferStatus ) = 0;
    virtual bool        CanWaitForFragment() = 0;
};

// ===============================================================================================================
// ===============================================================================================================
