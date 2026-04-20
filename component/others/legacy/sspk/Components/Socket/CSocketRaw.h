///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CSocket.h"
#include "CRawPacket.h"

// ===============================================================================================================
// CSocketRaw class
// ===============================================================================================================

class CSocketRaw : public CSocket
{
public:
    CSocketRaw(IAVManager* avManager, uint32 pipeIdN, const CTuneRequest& tuneRequest, eSocketType socketType);
    virtual ~CSocketRaw();

protected:
    //CSocket interfaces
    //
    //Instantiating a packet to be used for processing on socket thread
    __override IPacket*     CreatePacket(void);
    //Receiving a packet from socket
    __override int          ReceivePacket(IPacket& packet);

    //CSocketRaw interfaces
    //
    //Read a raw packet from the socket
    //Derived classes to provide implementation
    virtual int             Recv(CRawPacket& rawPacket);

    //CSocketBase interfaces
    //
    //Called to pre-process the packet before it ias passed on to the receiver
    __override void         SocketPacketReceived(IPacket& packet);
};

// ===============================================================================================================
// ===============================================================================================================
