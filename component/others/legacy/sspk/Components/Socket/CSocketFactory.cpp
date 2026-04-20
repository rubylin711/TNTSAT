///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CSocketFactory.h"
#include "CSocketMbr.h"
#include "Thread.h"
#include "Trace.h"
#include <string>
#include <list>
#include "pkSockets.h"
using namespace std;

//#define SOCKET_SPEW
#if defined(SOCKET_SPEW)
#define SOCKET_TRACE(x) TRACE(x)
#else
#define SOCKET_TRACE(x)
#endif

// ===============================================================================================================
// Socket Thread
// ===============================================================================================================

class CSocketThread : public IRunnable
{
public:
    CSocketThread(ISocket* socket) : _socket(socket) {}
    virtual ~CSocketThread() {}

    //Queue up request in thread pool
    void StartSocketThread(ThreadPool* threadPool)
    {
        //Start the work on tuner thread
        threadPool->QueueWorkItem(this);
    }

    //IRunnable interface
    __override void OnThreadRun(void)
    {
        _socket->OnThreadRun();
        delete this;
    }

private:
    //Socket to read the data from
    ISocket* _socket;
};

// ===============================================================================================================
// ===============================================================================================================

CSocketFactory::CSocketFactory(IAVManager* avManager)
    : mAVManager(avManager)
{
    //Allocate a thread pool to run sockets
    mThreadPool = new ThreadPool(SOCKET_THREADPOOL_SIZE, SOCKET_THREAD_PRIORITY);
    CHECK_ALLOC(mThreadPool);
}

CSocketFactory::~CSocketFactory()
{
    //Sanity check
    ASSERT(mSocketsList.size() == 0);

    //Delete the thread pool
    delete mThreadPool;
}

// ===============================================================================================================
// Factory methods for acquiring and disposing a socket
// We mainatin a list of all active sockets which allow us to share sockets among multiple receivers.
// It is upto sockets to allow whether sharing is possible
// ===============================================================================================================

//Factory APIs to create a socket and associating it with given receiver. It can choose
//to connect receiver to an existing socket is one is altready tuned to the same service
ISocket* CSocketFactory::AcquireSocket(_In_ uint32 pipeIdN, 
                                       _In_ const CTuneRequest& tuneRequest, 
                                       _In_ IReceiver* receiver, 
                                       _Out_ eTunerError& tunerError, 
                                       _In_ uint32 initialNetworkBitsPerSec)
{
    AutoLock lock(&mSocketsLock);

    //Check if we can find a socket to share
    ISocket* socket = NULL;
    for (list<ISocket*>::iterator it = mSocketsList.begin(); it != mSocketsList.end(); ++it)
    {
        if ((*it)->IsSharable(tuneRequest))
        {
            socket = *it;
            break;
        }
    }

    //Could not find an existing socket - create a new one
    if (socket == NULL)
    {
        //Create a socket based on the given tune request
        socket = NewSocket(pipeIdN, tuneRequest, initialNetworkBitsPerSec);
        //Failed to create a socket?
        if (socket == NULL)
        {
            TRACE(("[%08x] Failed to create socket [%08x] for receiver[%08x] url %s", pipeIdN, socket, receiver, tuneRequest.TunerUrl.c_str()));

            //Failure
            tunerError = eTunerErrorAllocationFailure;
        }
        else
        //New socket is created
        {
            TRACE(("[%08x] New socket [%08x] for receiver [%08x] with url %s", pipeIdN, socket, receiver, tuneRequest.TunerUrl.c_str()));

            //Associate the given receiver to this socket
            socket->AddReceiver(receiver);
            //And let the socket start streaming
            if (socket->StartStreaming())
            {
                //Start the socket thread
                CSocketThread* socketThread = new CSocketThread(socket);
                socketThread->StartSocketThread(mThreadPool);
                tunerError = eTunerErrorNone;
            }
            else
            {
                tunerError = eTunerErrorConnectFailed;
            }
            //Save the new socket to active list
            mSocketsList.push_back(socket);
        }
    }
    else
    //Share an existing socket
    {
        //Make the association between existing socket and the supplied receiver
        tunerError = socket->AddReceiver(receiver);
        if (tunerError != eTunerErrorNone)
        {
            //Sharing refused by socket
            TRACE(("[%08x] Failed to share socket [%08x] for receiver [%08x] with url %s", pipeIdN, socket, receiver, tuneRequest.TunerUrl.c_str()));
            socket = NULL;
        }
        else
        {
            //Sharing the socket
            TRACE(("[%08x] Share socket [%08x] for receiver [%08x] with url %s", pipeIdN, socket, receiver, tuneRequest.TunerUrl.c_str()));
        }
    }
    return socket;
}

//Disassociate given socket from given receiver, detune immediately if so requested or
//when all receivers have been taken away from the socket
void CSocketFactory::DisposeSocket(_In_ uint32 pipeIdN, 
                                   _In_ ISocket* socket, 
                                   _In_ IReceiver* receiver, 
                                   _In_ bool forceDetune, 
                                   _Out_opt_ uint32* pMeasuredNetworkBitsPerSec /* = NULL */)
{
    AutoLock lock(&mSocketsLock);

    TRACE(("[%08x] Detach receiver [%08x] from socket [%08x] ForceDetune[%s]", pipeIdN, receiver, socket, forceDetune ? "true" : "false"));

    //Signal the receiver to stop processing data asap
    receiver->SignalStop();
    //Detach receiver from socket
    bool stopStreaming = socket->RemoveReceiver(receiver, forceDetune);
    //And stop the receiver
    receiver->Stop(true);
    ::Socket_SetExitNetworkFlag(1);//add for network retry function

    if(pMeasuredNetworkBitsPerSec)
    {
        *pMeasuredNetworkBitsPerSec = socket->GetMeasuredNetworkBitsPerSec();
    }

    //Close the socket immediately if so requested or no more ceceivers attached to it
    if (forceDetune || stopStreaming)
    {
        TRACE(("[%08x] Stopping socket [%08x]", pipeIdN, socket));

        //This will unblock any recv that may be pending
        socket->Close();
        //Remove this socket from our active list of sockets
        mSocketsList.remove(socket);
        //Stop streaming when all receivers are taken away.
        if (stopStreaming)
        {
            socket->StopStreaming();
            //NOTE: ***** IMPORTANT *****
            //Socket could be deleted any time after streaming is stopped
            //So do not try to access the object from this point on...
        }
    }
}

//Factory API to create a socket without a receiver attached
ISocket* CSocketFactory::AcquireSocket(uint32 pipeIdN, const CTuneRequest& tuneRequest, eTunerError& tunerError)
{
    //Create a socket based on the given tune request
    ISocket* socket = NewSocket(pipeIdN, tuneRequest);
    //Failed to create a socket?
    if (socket == NULL)
    {
        TRACE(("[%08x] Failed to create socket [%08x]", pipeIdN, socket));

        //Failure
        tunerError = eTunerErrorAllocationFailure;
    }
    else
    //New socket is created
    {
        TRACE(("[%08x] New socket [%08x]", pipeIdN, socket));

        //Adds a request for network profile of a new socket
        //Start the socket thread
        if (socket->StartStreaming())
        {
            //Start the socket thread
            CSocketThread* socketThread = new CSocketThread(socket);
            socketThread->StartSocketThread(mThreadPool);
            tunerError = eTunerErrorNone;
        }
        else
        {
            tunerError = eTunerErrorConnectFailed;
        }
    }
    return socket;
}

//Facory API to dispose a socket without a receiver.
//This should be sued when sockets were instantiated standalone using the
//above factory API
void CSocketFactory::DisposeSocket(uint32 pipeIdN, ISocket* socket)
{
    TRACE(("[%08x] Dispose socket [%08x]", pipeIdN, socket));

    //Close the socket immediately
    //This will unblock any recv that may be pending
    socket->Close();
    //Stop streaming on the socket right away
    socket->StopStreaming();
    //NOTE: ***** IMPORTANT *****
    //Socket could be deleted any time after streaming is stopped
    //So do not try to access the object from this point on...
}

// ===============================================================================================================
// ===============================================================================================================

//Create a socket based on the given tune request
ISocket* CSocketFactory::NewSocket(_In_ uint32 pipeIdN, 
                                   _In_ const CTuneRequest& tuneRequest, 
                                   _In_opt_ uint32 initialNetworkBitsPerSec /*= 0*/)
{
    if (tuneRequest.IsMbr())
        return new CSocketMbr(mAVManager, pipeIdN, tuneRequest, initialNetworkBitsPerSec);

    return NULL;
}

// ===============================================================================================================
// Returns string name given the socket type
// ===============================================================================================================

const char* ISocketFactory::SocketNameFromType(eSocketType socketType)
{
    const char* gSocketTypeStrings[] =
    {
        "Unknown",                   //eSocketType_Unknown,
        "SetHttpRateControlled",     //eSocketType_SetHttpRateControlled,
        "Live",                      //eSocketType_Live,
        "DVR",                       //eSocketType_Dvr,
        "DvrRateControlled",         //eSocketType_DvrRateControlled,
        "VOD",                       //eSocketType_Vod,
        "Flexible",                  //eSocketType_Flexible,
        "Dvb",                       //eSocketType_Dvb,
        "WMS",                       //eSocketType_Wms,
        "MP3",                       //eSocketType_Mp3,
        "TS",                        //eSocketType_TS,
        "MP4",                       //eSocketType_Mp4,
        "MBR",                       //eSocketType_Mbr,
        "MBRChunk",                  //eSocketType_MbrChunk,
        "SmoothStreaming",           //eSocketType_SS,
        "SmoothStreamingChunk",      //eSocketType_SSChunk,
        "MpegTS",                    //eSocketType_MpegTS
        "RtpCarousel",               //eSocketType_RtpCarousel
    };

    ASSERT(socketType < eSocketType_Max);
    return gSocketTypeStrings[socketType < eSocketType_Max ? socketType : 0];
}

// ===============================================================================================================
// ===============================================================================================================
