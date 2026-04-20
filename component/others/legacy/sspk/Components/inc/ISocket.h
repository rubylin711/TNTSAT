///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IDiagsProvider.h"
#include "CReceiverNotification.h"
#include "CSocketDefinitions.h"
#include "SmoothErrorDefinitions.h"
#include "Thread.h"
#include <string>
#include <vector>

// ===============================================================================================================
// External classes
// ===============================================================================================================

class IReceiver;
class CTuneRequest;

// ===============================================================================================================
// ISocket interfaces
// ===============================================================================================================

class ISocket : public IRunnable, public IDiagsProvider
{
public:
    //Destructor
    virtual ~ISocket() {}

    //Returns socket type
    virtual eSocketType     GetSocketType(void) = 0;

    //Get access to the tune rqeuest
    virtual CTuneRequest&   GetTuneRequest(void) = 0;

    //Returns true if the socket is already tuned to the given stream (tune request)
    virtual bool            IsSharable(const CTuneRequest& tuneRequest) = 0;

    //Connect socket to the given tune request
    virtual bool            Connect(void) = 0;
    //Refresh a tune url in the socket - used only for VOD at this time
    virtual bool            ConnectRefresh(const std::string& url) = 0;
    //Stop current tune and start cleaning up socket
    virtual bool            Close(void) = 0;
    //Returns the recently measured network bandwidth
    virtual uint32          GetMeasuredNetworkBitsPerSec(void) = 0;
    //Attach given receiver to this socket
    virtual eTunerError     AddReceiver(IReceiver* receiver) = 0;
    //Start streaming on the given socket
    virtual bool            StartStreaming(void) = 0;
    //Detach given receiver from this socket
    virtual bool            RemoveReceiver(IReceiver* receiver, bool forceDetune) = 0;
    //Stop streaming on the given socket
    virtual bool            StopStreaming(void) = 0;

    //Pause the stream
    virtual bool            Pause(void) = 0;
    //Start playing the stream
    virtual bool            Play(void) = 0;
    //Select audio stream by pid / stream index (pid == -1 -> select all audio streams)
    virtual bool            SetAudioLanguage(int pid) = 0;
    //Select subtitle stream by pid / stream index (pid == -1 -> select all subtitle streams)
    virtual bool            SetSubtitleLanguage(int pid) = 0;
    //Is socket currently connected
    virtual bool            IsConnected(void) const = 0;
    //Generalization of different errors encountered during streaming
    virtual eTunerError     GetTunerError(void) const = 0;
    //Get socket specific error
    virtual eSocketError    GetSocketError(void) const = 0;
    //Get pkResult if any
    virtual pkRESULT        GetPKResult(void) const = 0;
    //Get http response if any
    virtual int             GetHttpResponse(void) const = 0;

    //Generic interface to send custom commands
    virtual bool            Command(const std::string& command, const std::vector<std::string>& args) = 0;

    //Send given notification to all receivers
    virtual void            SendNotification(ReceiverNotificationType notificationType, CReceiverNotificationData* notificationData) = 0;

protected:
    //Whether to stream all tracks for audio/subtitles
    static const int        SETLANGUAGE_SELECTALL = -1;
};

// ===============================================================================================================
// ISocketFactory interface
// ===============================================================================================================

class ISocketFactory
{
public:
    //Destructor
    virtual ~ISocketFactory() {}

    //Creates a socket and associating it with given receiver. It can choose to connect IReceiver
    //to an existing socket if one is altready tuned to the same service
    virtual ISocket*        AcquireSocket(_In_ uint32 pipeIdN, 
                                          _In_ const CTuneRequest& tuneRequest, 
                                          _In_ IReceiver* receiver, 
                                          _Out_ eTunerError& tunerError, 
                                          _In_ uint32 initialNetworkBitsPerSec) = 0;    // the presumed available network bandwidth to initalize
                                                                                        // the socket since network history does not persist once the 
                                                                                        // socket is disposed, if zero the value is not used

    //Disassociate given socket from a given receiver, detune immediately if so requested or
    //otherwise the socket may have been shared with other receivers and will be detuned when
    //those receivers go away...
    virtual void            DisposeSocket(_In_ uint32 pipeIdN, 
                                          _In_ ISocket* socket, 
                                          _In_ IReceiver* receiver, 
                                          _In_ bool forceDetune, 
                                          _Out_opt_ uint32* pMeasuredNetworkBitsPerSec = NULL) = 0; // at the time the socket is disposed, the recently measured
                                                                                        // network bandwidth is returned in this parameter so that it
                                                                                        // may be used to initalize the socket on an acquire

    //Factory API to create a socket without a receiver attached.  Socket will spin up and
    //wait for receiver to be connected before using packets downstream...
    virtual ISocket*        AcquireSocket(_In_ uint32 pipeIdN, 
                                          _In_ const CTuneRequest& tuneRequest,  
                                          _Out_ eTunerError& tunerError) = 0;

    //Facory API to dispose a socket created stand alone using the above API.
    virtual void            DisposeSocket(_In_ uint32 pipeIdN, 
                                          _In_ ISocket* socket) = 0;

    //Returns string name given the socket type
    static const char*      SocketNameFromType(_In_ eSocketType socketType);
};

// ===============================================================================================================
// ===============================================================================================================
