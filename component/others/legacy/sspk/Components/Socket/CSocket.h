///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CSocketBase.h"
#include "CEvent.h"
#include "ThreadTime.h"

// ===============================================================================================================
// CSocket class
// ===============================================================================================================

class CSocket : public CSocketBase
{
public:
    CSocket(IAVManager* avManager, uint32 pipeIdN, const CTuneRequest& tuneRequest, eSocketType socketType);
    virtual ~CSocket();

protected:
    //CSocket interfaces
    //
    //Instantiating a packet to be used for processing on socket thread
    virtual IPacket*        CreatePacket(void) = 0;
    //Receiving a packet from socket
    virtual int             ReceivePacket(IPacket& packet) = 0;

public:
    //ISocket interfaces
    //
    //Attach given receiver to this socket
    __override eTunerError  AddReceiver(IReceiver* receiver);
    //Stop streaming on the this socket
    __override bool         StopStreaming(void);
    //Wait for the socket to be closed
    __override bool         Close(void);
    //Generic interface to send custom commands
    __override bool         Command(const std::string& command, const std::vector<std::string>& args);

    //IRunnable interface
    __override void         OnThreadRun(void);

private:
    //Managing streaming data to receivers
    CEvent                  _socketStreamingEvent;
    //Managing receivers connected to this tuner
    CEvent                  _socketThreadCleanupEvent;
    //Let SocketFactory know when the socket has exited
    CEvent                  _socketThreadExitedEvent;
    //Thread instrumentation
    ThreadTime              _socketThreadTime;

#ifdef TV2INTERNAL
    //Force socket to return a faked error
    eSocketError            _diagsFakeSocketReadError;
#endif
};

// ===============================================================================================================
// ===============================================================================================================
