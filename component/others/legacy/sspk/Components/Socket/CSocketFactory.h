///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IAVManager.h"
#include "ISocket.h"
#include "IReceiver.h"
#include "CTuneRequest.h"
#include "Threadpool.h"
#include "AutoLock.h"
#include <string>
#include <list>

// ===============================================================================================================
// CSocketFactory
// ===============================================================================================================

class CSocketFactory : public ISocketFactory
{
public:
    //Constructor
    CSocketFactory(IAVManager* avManager);
    //Destructor
    virtual ~CSocketFactory();

private:
    //Factory of factories
    IAVManager*             mAVManager;

    //Thread pool to be used for socket threads
    ThreadPool*             mThreadPool;

    //Number of threads in socket thread pool
    static const uint32     SOCKET_THREADPOOL_SIZE = 8;
    static const uint32     SOCKET_THREAD_PRIORITY = pkEXECUTIVE_THREAD_PRIORITY_HIGH;


private:
    //Global lock used managing list of active tuners
    Lockable                mSocketsLock;
    //Managing active tuners list (used to create shared tuner scenario)
    std::list<ISocket*>     mSocketsList;
public:
    //Factory APIs to create a socket and associating it with given receiver. It can choose
    //to connect IReceiver to an existing socket is one is already tuned to the same service
    __override ISocket*     AcquireSocket(_In_ uint32 pipeIdN, 
                                          _In_ const CTuneRequest& tuneRequest, 
                                          _In_ IReceiver* receiver, 
                                          _Out_ eTunerError& tunerError, 
                                          _In_ uint32 initialNetworkBitsPerSec);

    //Disassociate given socket from given receiver, detune immediately if so requested or
    //when all receivers have been taken away from the socket
    __override void         DisposeSocket(_In_ uint32 pipeIdN, 
                                          _In_ ISocket* socket, 
                                          _In_ IReceiver* receiver, 
                                          _In_ bool forceDetune, 
                                          _Out_opt_ uint32* pMeasuredNetworkBitsPerSec = NULL);

    //Factory API to create a socket without a receiver attached
    __override ISocket*     AcquireSocket(_In_ uint32 pipeIdN, 
                                          _In_ const CTuneRequest& tuneRequest,  
                                          _Out_ eTunerError& tunerError);

    //Facory API to dispose a socket created stand alone using the above API.
    __override void         DisposeSocket(_In_ uint32 pipeIdN, 
                                          _In_ ISocket* socket);

private:
    //Factory APIs to create a new socket
    ISocket*                NewSocket(_In_ uint32 pipeIdN, 
                                      _In_ const CTuneRequest& tuneRequest, 
                                      _In_opt_ uint32 initialNetworkBitsPerSec = 0);
};

// ===============================================================================================================
// ===============================================================================================================
