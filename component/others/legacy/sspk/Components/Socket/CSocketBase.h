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
#include "IPacket.h"
#include "DecoderBufferStatus.h"
#include "CTuneRequest.h"
#include "Buffer.h"
#include "CClockConfiguration.h"
#include "CSocketDiags.h"
#include "AutoLock.h"

// ===============================================================================================================
// CSocketBase class
// Knows how to handle interaction between all types of receivers and sockets
// ===============================================================================================================

class CDiagsSocketEvent;
class CSocketBase : public ISocket
{
public:
    //Constructor
    CSocketBase(IAVManager* avManager, uint32 pipeIdN, const CTuneRequest& tuneRequest, eSocketType socketType);
    //Destructor
    virtual ~CSocketBase();

public:
    //ISocket interfaces
    //
    //Returns socket type
    __override eSocketType      GetSocketType(void) { return _eSocketType; }

    //Get access to the tune rqeuest
    __override CTuneRequest&    GetTuneRequest(void) { return _socketTuneRequest; }

    //Returns true if the socket is already tuned to the given stream (tune request)
    __override bool             IsSharable(const CTuneRequest& tuneRequest) { return false; }

    //Connect socket to the given tune request
    __override bool             Connect(void);
    //Refresh a tune url in the socket - used only for VOD at this time
    __override bool             ConnectRefresh(const std::string& url) { return true; }
    //Stop current tune and start cleaning up socket
    __override bool             Close(void);

    //Returns the recently measured network bandwidth
    __override uint32          GetMeasuredNetworkBitsPerSec(void) { return 0; }

    //Attach given receiver to this socket
    __override eTunerError      AddReceiver(IReceiver* receiver);
    //Start streaming on this socket
    __override bool             StartStreaming(void);
    //Detach given receiver from this socket
    __override bool             RemoveReceiver(IReceiver* receiver, bool forceDetune);
    //Stop streaming on the this socket
    __override bool             StopStreaming(void);

    //Pause the stream
    __override bool             Pause(void) { return true; }
    //Start playing the stream
    __override bool             Play(void) { return true; }
    //Select audio stream by pid / stream index
    __override bool             SetAudioLanguage(int pid) { return false; }
    //Select audio stream by pid / stream index
    __override bool             SetSubtitleLanguage(int pid) { return false; }
    
    //Is socket currently connected
    __override bool             IsConnected(void) const { return _socketConnected; }

    //Generalization of different errors encountered during streaming
    __override eTunerError      GetTunerError(void) const { return _socketTunerError; }
    //Get socket specific error
    __override eSocketError     GetSocketError(void) const { return _socketError; }
    //Get http response if any
    __override int              GetHttpResponse(void) const { return _socketHttpResponse; }
    //Get pkResult if any
    __override pkRESULT         GetPKResult(void) const { return _socketPKResult; }

    //Generic interface to send custom commands
    __override bool             Command(const std::string& command, const std::vector<std::string>& args) { return false; }

    //Send given notification to all receivers
    __override void             SendNotification(ReceiverNotificationType notificationType, CReceiverNotificationData* notificationData);

    //IRunnable interface
    __override void             OnThreadRun(void) { ASSERT(false); }

    //IDiagsProvider interfaces
    __override void             DiagsReset(void);
    __override void             DiagsRetrieve(IDiagsEvent* diagsEvent);
    __override IDiagsEvent*     DiagsRetrieve(void);

    //Helper method to send socket event
    void                        SendDiagsEvent(CDiagsSocketEvent* diagsEvent);

protected:
    //Queries the playback receivers if the decoder buffers are full
    //and returns current audio and video buffer levels
    void                        GetDecoderBufferStatus( _Out_ DecoderBufferStatus* pDecoderBufferStatus );

    // Checks if the receiver can wait while the socket stalls before
    // requesting another fragment
    bool                        CanWaitForFragment();

    //Queries playback receiver for current time of frame being rendered
    uint64                      GetCurrentPlaybackTime(void);

    //Update the clock mode in receivers
    virtual void                ReceiversOnClockMode(void);
    //Called when socket is successfully connected to the source
    virtual void                ReceiversOnConnected(void);
    //Process packets and pass on to receivers
    virtual bool                ReceiversOnPacket(IPacket& packet);
    //Called when an error detected from socket while connecting or during streaming
    //
    //We now know the exact nature of error while tuning or reading data from the
    //socket. Notify downstream component of specific state of affairs (for example,
    //we have reached live or we have reached end of file for DVR or VOD etc).
    virtual void                ReceiversOnError(void);

    //Called to pre-process the packet before it ias passed on to the receiver
    virtual void                SocketPacketReceived(IPacket& packet) {}
    //Called to process inband signalling for pcakets
    virtual bool                SocketPacketInbandInfo(IPacket& packet) { return false; }

    //Maps socket recv errors to generic tuner errors
    static eTunerError          MapRecvErrorToTunerError(eSocketRecvError recvError);

protected:
    //Factory of factories
    IAVManager*                 _pSocketAVManager;
    //Diagnostics manager
    IDiagsManager*              _pSocketDiagsManager;
    //Buffer pool factory to use
    IBufferPoolFactory*         _pSocketBufferPoolFactory;
    //Numeric representation of pipeId to which this socket is attached
    uint32                      _socketPipeIdN;

    //Type of socket
    eSocketType                 _eSocketType;

    //Current tune request
    CTuneRequest                _socketTuneRequest;

    //A generic lock used by various sockets
    mutable Lockable            _socketLock;

    //Managing receivers connected to this tuner
    Lockable                    _socketReceiverLock;
    IReceiver*                  _socketReceiverPlayback;
    int                         _socketReceiverNum;

    //Whether the socket processing is still on
    bool                        _socketRunning;

    //Whether it is currently connected or stopped
    //
    //Sockets always start life with this flag set to true
    //and socket Close() should set it to false when we are
    //done using it
    bool                        _socketConnected;

    //Socket allows retrying of packets in receivers
    bool                        _socketRetryAllowed;
    bool                        _socketResetOnStall;
    //Represents the buffer model to use for this socket
    // -1 => start with no buffering (ICC)
    //  0 => start using stream defined PCRPTS
    //  x => start when x stream units have expired
    int                         _socketBufferDelay;
    //Default wall clock buffer delay to use when no other signaling present
    int                         _socketDefaultBufferDelay;
    //Customizing clock behavior based on sockets
    uint32                      _socketClockMode;

    //Generalization of different errors encountered during streaming
    eTunerError                 _socketTunerError;
    //Socket specific error
    eSocketError                _socketError;
    //platform error if any
    pkRESULT                    _socketPKResult;
    //http response code if any
    int                         _socketHttpResponse;
    //Whether socket is playing ad stream
    bool                        _socketStreamingAd;
    //Whether we need to map PCR/NTP time line
    bool                        _socketMapTimeValid;
    //Map from given PCR/NTP time pair
    uint64                      _socketMapFromPcr;
    uint64                      _socketMapFromNtp;
    //Map to given PCR/NTP time pair
    uint64                      _socketMapToPcr;
    uint64                      _socketMapToNtp;

    //Keep track of timings
    uint32                      _socketConnectingTime;
    uint32                      _socketConnectedTime;

    //Number of RTP packets received
    uint32                      _socketDiagsPacketsProcessed;
    //Number of RTP packets with parse errors
    uint32                      _socketDiagsPacketsWithParseErrors;
};

// ===============================================================================================================
// ===============================================================================================================
