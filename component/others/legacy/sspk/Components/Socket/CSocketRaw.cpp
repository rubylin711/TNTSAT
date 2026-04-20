///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CSocketRaw.h"
#include "Trace.h"

//#define SOCKET_SPEW
#if defined(SOCKET_SPEW)
#define SOCKET_TRACE(x) TRACE(x)
#else
#define SOCKET_TRACE(x)
#endif

// ===============================================================================================================
// CSocketRaw class
// ===============================================================================================================

CSocketRaw::CSocketRaw(IAVManager* avManager, uint32 pipeIdN, const CTuneRequest& tuneRequest, eSocketType socketType)
    : CSocket(avManager, pipeIdN, tuneRequest, socketType)
{
}

CSocketRaw::~CSocketRaw()
{
}

IPacket* CSocketRaw::CreatePacket(void)
{
    return new CRawPacket(IPacket::kPacketType_Raw);
}

int CSocketRaw::ReceivePacket(IPacket& packet)
{
    return Recv((CRawPacket&)packet);
}

//Reads raw elementary stream data from sockets
//Derived classes to provide implementation
int CSocketRaw::Recv(CRawPacket& rawPacket)
{
    ASSERT(false);
    return eRecvErrorUnknown;
}

void CSocketRaw::SocketPacketReceived(IPacket& packet)
{
    //Map timing coming out of this socket to the one requested by the receiver
    //This is done for seamless splicing of streams
    if (IS_VALID_TIME(packet.NTP))
    {
        if (_socketMapTimeValid)
        {
            packet.PCR = (_socketMapToPcr + (packet.PCR - _socketMapFromPcr));
            packet.NTP = (_socketMapToNtp + (packet.NTP - _socketMapFromNtp));
        }

        //Update the timing used for mapping into DVR and also used for splicing
        packet.PCRForDvr = packet.PCR;
        packet.NTPForDvr = packet.NTP;
    }
}

// ===============================================================================================================
// ===============================================================================================================
