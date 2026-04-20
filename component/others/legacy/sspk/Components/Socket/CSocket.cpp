///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDiagsManager.h"
#include "CSocket.h"
#include "Trace.h"
#include <string>
#include <map>
using namespace std;

//#define SOCKET_SPEW
#if defined(SOCKET_SPEW)
#define SOCKET_TRACE(x) TRACE(x)
#else
#define SOCKET_TRACE(x)
#endif

// ===============================================================================================================
// CSocket class
// ===============================================================================================================

CSocket::CSocket(IAVManager* avManager, uint32 pipeIdN, const CTuneRequest& tuneRequest, eSocketType socketType)
    : CSocketBase(avManager, pipeIdN, tuneRequest, socketType)
    , _socketStreamingEvent(CEvent::eResetModeManual, false)
    , _socketThreadCleanupEvent(CEvent::eResetModeManual, false)
    , _socketThreadExitedEvent(CEvent::eResetModeManual, true)
{
#ifdef TV2INTERNAL
    //Force socket to return a faked error
    _diagsFakeSocketReadError = eSocketErrorNone;
#endif
}

CSocket::~CSocket()
{
}

eTunerError CSocket::AddReceiver(IReceiver* receiver)
{
    //Let go of the base class first
    eTunerError tunerError = CSocketBase::AddReceiver(receiver);
    if (tunerError == eTunerErrorNone)
    {
        //Let the socket start streaming because a receiver is now connected
        _socketStreamingEvent.Set();
    }
    return tunerError;
}

bool CSocket::StopStreaming(void)
{
    //Do base class stuff
    CSocketBase::StopStreaming();
    //Release the thread in case it has not yet started to stream
    //because a receiver was never or not yet connected
    _socketStreamingEvent.Set();
    //This should be the last thing we do so that
    //the socket thread can do the final cleanup of
    //the socket object and delete all resources
    _socketThreadCleanupEvent.Set();
    return true;
}
bool CSocket::Close(void)
{
    bool bClose = CSocketBase::Close();

    //wait for the thread to actually exit
    SOCKET_TRACE(("socketShutdownEvent Waiting..."));
    _socketThreadExitedEvent.Wait();
    SOCKET_TRACE(("socketShutdownEvent Waiting...DONE"));

    return bClose;
}

void CSocket::OnThreadRun(void)
{
    SOCKET_TRACE(("[%08x] socket [%08x] thread starting...", _socketPipeIdN, this));

    IPacket* packet = NULL;

    // reset the event since the thread is now running
    _socketThreadExitedEvent.Reset();

    //We would like to calculate CPU usage on per tuner thread basis
    _socketThreadTime.Init();

    //Do the initialization
    CE_AV_TUNERTHREAD_LOG(CE_AV_TUNERTHREAD_START);

    //Starting a tune
    if (_socketRunning)
    {
        //Now connect the socket
        _socketConnectingTime = Executive_GetTickCount();
        bool connected = Connect();
        _socketConnectedTime = Executive_GetTickCount();

        //Connection failed
        if (!connected)
        {
            _socketTunerError = eTunerErrorConnectFailed;
            goto exit;
        }

        //Wait for at least one receiver to connect
        _socketStreamingEvent.Wait();

        //Now that we have connected the socket
        //let the receivers know about the clock start mode
        ReceiversOnConnected();
    }

    //Register for IDiagsProvider - attach the socket as a diags provider
    _pSocketDiagsManager->Attach(this);

    CE_AV_TUNERTHREAD_LOG(CE_AV_TUNERTHREAD_CONNECTED);

    SOCKET_TRACE(("[%08x] socket [%08x] connected...", _socketPipeIdN, this));

    packet = CreatePacket();
    if (!packet)
    {
        ASSERT(false);
        TRACE_ERROR(("[%08x] CreatePacket() FAILED", _socketPipeIdN));
    }
    else
    {
        bool closeSocket = false;
        while (_socketRunning)
        {
            //Any error during processing packets?
            //Give the socket a chance to handle the error, if so
            if((_socketTunerError != eTunerErrorNone) || ( closeSocket ))
            {
                ReceiversOnError();
            }

            //Keep processing data on successful tune
            while (_socketRunning)
            {
#ifdef TV2INTERNAL
                //Check CPU timings periodically
                if (_socketThreadTime.Collect())
                {
                    CReceiverNotificationData notificationData;
                    notificationData.ThreadTimes.DiagsCpuUsageLast = _socketThreadTime.Last;
                    notificationData.ThreadTimes.DiagsCpuUsageAverage = _socketThreadTime.Average;
                    notificationData.ThreadTimes.DiagsCpuUsageMaximum = _socketThreadTime.Maximum;
                    notificationData.ThreadTimes.DiagsCpuUsageTotalUsed = _socketThreadTime.TotalUsed;
                    SendNotification(kReceiverNotificationType_ThreadTimes, &notificationData);
                }
#endif

                //Read next packet from the socket
                int lenError = ReceivePacket(*packet);

#ifdef TV2INTERNAL
                if (_diagsFakeSocketReadError != eSocketErrorNone)
                {
                    lenError = _diagsFakeSocketReadError;
                    _diagsFakeSocketReadError = eSocketErrorNone;
                    SOCKET_TRACE(("[%08x] force the faked recvive error:%d\n", _socketPipeIdN, lenError));
                }
#endif
                if (lenError <= 0)
                {
                    closeSocket = true;
                    _socketTunerError = MapRecvErrorToTunerError((eSocketRecvError)lenError);
                    break;
                }

                //Otherwise process packet downstream
                ReceiversOnPacket(*packet);
            }
        }
        //Done with packet - the thread are stopping now
        delete packet;
    }

    SOCKET_TRACE(("[%08x] socket [%08x] stopping...", _socketPipeIdN, this));

    //And do the final cleanup
    CE_AV_TUNERTHREAD_LOG(CE_AV_TUNERTHREAD_STOPPING);

    //Unregister for IDiagsProvider - remove socket as diags provider
    _pSocketDiagsManager->Detach(this);

exit:
    // Signal that socket thread is shutting down
    SOCKET_TRACE(("Setting socketThreadExitedEvent"));
    _socketThreadExitedEvent.Set();

    //And then wait for all receivers to be taken away
    _socketThreadCleanupEvent.Wait();

    SOCKET_TRACE(("[%08x] socket [%08x] stopped...", _socketPipeIdN, this));

    CE_AV_TUNERTHREAD_LOG(CE_AV_TUNERTHREAD_STOPPED);
    
    //Finally delete the socket itself
    delete this;
}

bool CSocket::Command(const string& command, const vector<string>& args)
{
#ifdef TV2INTERNAL
    //Force a fake socket read error
    if (command == "fakesocketreaderror")
    {
        const string& error = args[0];
        _diagsFakeSocketReadError = error.empty() ? eSocketErrorReadError : (eSocketError)atoi(error.c_str());

        SOCKET_TRACE(("[%08x] Fake socket read error to %d", _socketPipeIdN, _diagsFakeSocketReadError));
        return true;
    }
#endif
    return CSocketBase::Command(command, args);
}

// ===============================================================================================================
// ===============================================================================================================
