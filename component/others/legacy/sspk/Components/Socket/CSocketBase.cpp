///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDiagsManager.h"
#include "CSocketBase.h"
#include "IReceiver.h"
#include "Trace.h"

//#define SOCKET_SPEW
#if defined(SOCKET_SPEW)
#define SOCKET_TRACE(x) TRACE(x)
#else
#define SOCKET_TRACE(x)
#endif

// ===============================================================================================================
// CSocketBase class
// Knows how to handle interaction between all types of receivers and sockets
// ===============================================================================================================

CSocketBase::CSocketBase(IAVManager* avManager, uint32 pipeIdN, const CTuneRequest& tuneRequest, eSocketType socketType)
    : _pSocketAVManager(avManager)
    , _socketPipeIdN(pipeIdN)
    , _eSocketType(socketType)
    , _socketTuneRequest(tuneRequest)
    , _socketReceiverPlayback(NULL)
    , _socketReceiverNum(0)
    , _socketRunning(true)
    , _socketConnected(true)
    , _socketRetryAllowed(true)
    , _socketResetOnStall(false)
    , _socketBufferDelay(0)
    , _socketClockMode(eClockMode_None)
    , _socketTunerError(eTunerErrorNone)
    , _socketError(eSocketErrorNone)
    , _socketPKResult(pkS_OK)
    , _socketHttpResponse(0)
    , _socketConnectingTime(0)
    , _socketConnectedTime(0)
{
    _socketDefaultBufferDelay = gClockConfiguration[eDefaultBufferDelay_ms];

    _pSocketDiagsManager = _pSocketAVManager->GetDiagsManager();
    _pSocketBufferPoolFactory = _pSocketAVManager->GetBufferPoolFactory();

    _socketStreamingAd = _socketTuneRequest.GetBool(TUNE_REQUEST_ISADSTREAM);
    _socketMapTimeValid = _socketTuneRequest.GetBool(TUNE_REQUEST_MAPTIMEVALID);
    _socketMapFromPcr = _socketTuneRequest.GetUInt64(TUNE_REQUEST_MAPFROMPCR, INVALID_TIME);
    _socketMapFromNtp = _socketTuneRequest.GetUInt64(TUNE_REQUEST_MAPFROMNTP, INVALID_TIME);
    _socketMapToPcr = _socketTuneRequest.GetUInt64(TUNE_REQUEST_MAPTOPCR, INVALID_TIME);
    _socketMapToNtp = _socketTuneRequest.GetUInt64(TUNE_REQUEST_MAPTONTP, INVALID_TIME);

    DiagsReset();
}

CSocketBase::~CSocketBase()
{
    //Sanity check on state of the socket on final cleanup
    ASSERT(_socketConnected == false);
    ASSERT(_socketReceiverPlayback == NULL);
    ASSERT(_socketReceiverNum == 0);
}

//Note note that the Connect and Recv are done on the socket
//thread and a Close call is initiated on an app thread and
//it may come before the socket thread had a chance to start
//connecting to the stream specified in tune request.
//
//In other words heavy lifting of connect and stream is done on
//socket thread which allows for faster responses on teh app thread
//when it is doing fast tunes and detunes while the user is doing
//cvhannel surfing for example.
//
//So always check first if the socket is already closed on the
//socket thread to release control and let the thread start the
//clean up process.
//
//Please also note that the Connect/Close APIs are expected
//to use the _socketLock appropriately to keep the thread
//synchronization so that all relevent data member are protected.
bool CSocketBase::Connect(void)
{
    //Quit if the socket has already been closed
    if (!_socketConnected)
    {
        _socketError = eSocketErrorAlreadyClosed;
        return false;
    }
    return true;
}

bool CSocketBase::Close(void)
{
    //We are now disconnected
    _socketConnected = false;
    return true;
}

eTunerError CSocketBase::AddReceiver(IReceiver* receiver)
{
    AutoLock lock(&_socketReceiverLock);

    //Add the new receiver to our list of receivers
    if (_socketReceiverPlayback != NULL)
    {
        return eTunerErrorCannotSharePlaybackReceivers;
    }
    _socketReceiverPlayback = receiver;

    //One more receiver attached here
    _socketReceiverNum++;

    //Initialize the new receiver to latest tuner state...
    receiver->Start();
    return eTunerErrorNone;
}

bool CSocketBase::StartStreaming(void)
{
    TRACE(("[%08x] socket [%08x] to start streaming...", _socketPipeIdN, this));

    //Send an event when the socket is connecting
    std::string url = _socketTuneRequest.GetArg(TUNE_REQUEST_SOURCEURL);
    if (url.empty())
        url = _socketTuneRequest.CanonicalUrl;

    url = url.substr(0, url.find("\r\n"));

    SendDiagsEvent(new CDiagsSocketOpenedEvent(url.c_str()));
    return true;
}

bool CSocketBase::RemoveReceiver(IReceiver* receiver, bool forceDetune)
{
    AutoLock lock(&_socketReceiverLock);

    ASSERT(_socketReceiverPlayback == receiver);
    _socketReceiverPlayback = NULL;

    //One less receiver to worry about;
    _socketReceiverNum--;

    //Signal all other active receivers to stop immediately
    //when we are forcefully stopping the tuner.  This will
    //ensure all node status related activities to cease
    //synchronously with this call.
    if (forceDetune || _socketReceiverNum == 0)
    {
        if (_socketReceiverPlayback)
        {
            _socketReceiverPlayback->SignalStop();
        }
        //Signal socket to stop streaming
        _socketRunning = false;
    }
    return _socketReceiverNum == 0;
}

bool CSocketBase::StopStreaming(void)
{
    TRACE(("[%08x] socket [%08x] to stop streaming...", _socketPipeIdN, this));

    //Send an event when the socket is closing
    SendDiagsEvent(new CDiagsSocketClosedEvent());
    return true;
}

void CSocketBase::SendNotification(ReceiverNotificationType notificationType, CReceiverNotificationData* notificationData)
{
    AutoLock lock(&_socketReceiverLock);

    //Send given notification to the receiver
    if (_socketReceiverPlayback)
    {
        _socketReceiverPlayback->Notify(notificationType, notificationData);
    }
}

void CSocketBase::GetDecoderBufferStatus( _Out_ DecoderBufferStatus* pDecoderBufferStatus )
{
    AutoLock lock(&_socketReceiverLock);

    // Query the receiver for the buffer status.
    // If the receiver is not available, make sure the status is initialized with safe values.

    if (_socketReceiverPlayback)
    {
        _socketReceiverPlayback->GetDecoderBufferStatus( pDecoderBufferStatus );
    }
    else
    {
        memset( pDecoderBufferStatus, 0, sizeof(*pDecoderBufferStatus) );
    }
}

bool CSocketBase::CanWaitForFragment()
{
    return( ( _socketReceiverPlayback != NULL ) && ( _socketReceiverPlayback->CanWaitForFragment() ) );
}

uint64 CSocketBase::GetCurrentPlaybackTime(void)
{
    AutoLock lock(&_socketReceiverLock);

    //Queries playback receiver for current time of frame being rendered
    uint64 ntp = INVALID_TIME;

    //It is only safe to call this function when the thread has
    //acquired the Receiver_Lock and not the Renderer_Lock
    if (_socketReceiverPlayback)
    {
        ntp = _socketReceiverPlayback->GetCurrentPlaybackTime();

        if (IS_VALID_TIME(_socketMapToNtp) && IS_VALID_TIME(_socketMapFromNtp))
        {
            if (ntp > _socketMapToNtp)
            {
                //The clock time may not be in the same timeline as this socket/asset
                //due to splicing (ad replacement/insertion).
                //Convert the time back to the timeline of this socket/asset,
                ntp = ntp - _socketMapToNtp + _socketMapFromNtp;
            }
            else
            {
                //If ntp is less than _socketMapToNtp, the current playback time
                //is still less than the starting time of this socket(asset),
                //i.e. the first packet from this socket is still in the decoder buffer.
                //so, the playback time for this socket should still be 0.
                ntp = 0;
            }
        }
    }
    return ntp;
}

void CSocketBase::ReceiversOnClockMode(void)
{
    //Send the clock related parameters connected playback receiver
    if (_socketReceiverPlayback)
    {
        //Whether the receiver can retry packets to the decoder
        CReceiverNotificationData canRetry;
        canRetry.Retry.CanRetry = _socketRetryAllowed;
        canRetry.Retry.ResetOnStall = _socketResetOnStall;
        _socketReceiverPlayback->Notify(kReceiverNotificationType_CanRetry, &canRetry);

        //Set the buffer delay
        CReceiverNotificationData streamBufferDelay;
        streamBufferDelay.StreamBuffer.Delay = _socketBufferDelay;
        _socketReceiverPlayback->Notify(kReceiverNotificationType_StreamBuffer, &streamBufferDelay);

        //Default wall clock buffer delay to use when no other signaling present
        CReceiverNotificationData streamBufferDefaultDelay;
        streamBufferDefaultDelay.StreamBufferDefault.Delay = _socketDefaultBufferDelay;
        _socketReceiverPlayback->Notify(kReceiverNotificationType_StreamBufferDefault, &streamBufferDefaultDelay);

        //Update the clock mode in receivers
        CReceiverNotificationData clockMode;
        clockMode.Clock.Mode = _socketClockMode;
        _socketReceiverPlayback->Notify(kReceiverNotificationType_ClockMode, &clockMode);
    }
}

void CSocketBase::ReceiversOnConnected(void)
{
    //Called when socket is successfully connected to the source
    AutoLock lock(&_socketReceiverLock);

    //Update the clock mode in receivers
    ReceiversOnClockMode();

    //Instrumentation for connect times
    CReceiverNotificationData connectTimes;
    connectTimes.ConnectTimes.ConnectingTime = _socketConnectingTime;
    connectTimes.ConnectTimes.ConnectedTime = _socketConnectedTime;

    //Send the events to all connected playback receiver
    if (_socketReceiverPlayback)
    {
        //Instrumentation for connect times
        _socketReceiverPlayback->Notify(kReceiverNotificationType_OnConnected, &connectTimes);
        //Let the receiver know we are ready to play
        _socketReceiverPlayback->Notify(kReceiverNotificationType_DataReady, NULL);
    }
}

bool CSocketBase::ReceiversOnPacket(IPacket& packet)
{
    IReceiver* pSocketReceiverPlayback = NULL;

    {
        AutoLock lock(&_socketReceiverLock);

        //Process the packet on downstream receivers
        //
        //One more packet received
        _socketDiagsPacketsProcessed++;

        //Used to remap the timestamps coming from this socket
        //based on request from the receiver at tune time
        SocketPacketReceived(packet);

        //Process packets which may have some inband information
        if (SocketPacketInbandInfo(packet))
            return true;

        // save the receiver in case RemoveReceiver is called while
        // outside of the lock below
        pSocketReceiverPlayback = _socketReceiverPlayback;
    }

    bool bWritePlayback = true;

    if (pSocketReceiverPlayback)
    {
        bWritePlayback = pSocketReceiverPlayback->WritePacket(packet);
    }
    return bWritePlayback;
}

void CSocketBase::ReceiversOnError(void)
{
    AutoLock lock(&_socketReceiverLock);
    bool reportError = true;

    //Called when an error detected from socket while connecting or during streaming
    //
    //We now know the exact nature of error while tuning or reading data from the
    //socket. Notify downstream component of specific state of affairs (for example,
    //we have reached live or we have reached end of file for DVR or VOD etc).
    TRACE(("[%08x] TunerError=%d SocketError=%d pkResult=0x%08x httpResponse=%d",
        _socketPipeIdN, _socketTunerError, _socketError, _socketPKResult, _socketHttpResponse));

    //Normal termination of Connect call due to a retune?
    if (_socketTunerError == eTunerErrorConnectFailed && _socketError == eSocketErrorNone)
    {
        _socketTunerError = eTunerErrorNone;
    }

    if ( (_socketTunerError == eTunerErrorNone) 
        || (_socketTunerError == eTunerErrorEOF) 
        || (_socketTunerError == eTunerErrorDrmInitFailure))
    {
        reportError = false;
    }

    //Handle error conditions
    if (_socketReceiverPlayback)
    {
        _socketReceiverPlayback->SignalStop();
        if (reportError)
        {
            _socketReceiverPlayback->Error(_socketTunerError, _socketError, _socketPKResult, _socketHttpResponse);
        }
        else
        {
            _socketReceiverPlayback->Stop(false, _socketTunerError);
        }
    }
    _socketRunning = false;
}

void CSocketBase::DiagsReset(void)
{
    //Base class thingy
    IDiagsProvider::DiagsReset();
    //Number of RTP packets received
    _socketDiagsPacketsProcessed = 0;
    //Number of RTP packets with parse errors
    _socketDiagsPacketsWithParseErrors = 0;
}

void CSocketBase::DiagsRetrieve(IDiagsEvent* diagsEvent)
{
    CDiagsSocketUpdateEvent* diagsSocketEvent = (CDiagsSocketUpdateEvent*)diagsEvent;
    if (diagsSocketEvent)
    {
        diagsSocketEvent->SocketType = _eSocketType;
        diagsSocketEvent->PipeIdN = _socketPipeIdN;
        diagsSocketEvent->MediaTransportId = _socketTuneRequest.MediaTransportId;
        diagsSocketEvent->UniqueId = _socketTuneRequest.UniqueId;
        diagsSocketEvent->ContentId = _socketTuneRequest.ContentId;
        diagsSocketEvent->Channel = _socketTuneRequest.Channel;
        diagsSocketEvent->BitRate = _socketTuneRequest.BitRate;

        diagsSocketEvent->PacketsProcessed = _socketDiagsPacketsProcessed + _socketDiagsPacketsWithParseErrors;
        diagsSocketEvent->PacketsWithParseErrors = _socketDiagsPacketsWithParseErrors;
    }
}

IDiagsEvent* CSocketBase::DiagsRetrieve(void)
{
    CDiagsSocketUpdateEvent* diagsSocketEvent = new CDiagsSocketUpdateEvent();
    if (diagsSocketEvent)
    {
        DiagsRetrieve(diagsSocketEvent);
    }
    return diagsSocketEvent;
}

void CSocketBase::SendDiagsEvent(CDiagsSocketEvent* diagsEvent)
{
    if (diagsEvent)
    {
        diagsEvent->SocketType = _eSocketType;
        diagsEvent->PipeIdN = _socketPipeIdN;
        diagsEvent->MediaTransportId = _socketTuneRequest.MediaTransportId;
        diagsEvent->UniqueId = _socketTuneRequest.UniqueId;
        diagsEvent->ContentId = _socketTuneRequest.ContentId;
        diagsEvent->Channel = _socketTuneRequest.Channel;
        diagsEvent->BitRate = _socketTuneRequest.BitRate;
        _pSocketDiagsManager->PostEvent(diagsEvent);
    }
}

// ===============================================================================================================
// Maps socket recv errors to generic tuner errors
//
// This determines whether there is a reportable error.  The socket determines whether to be closed based on the return
// value of ReceivePacket on the socket thread
// ===============================================================================================================

eTunerError CSocketBase::MapRecvErrorToTunerError(eSocketRecvError recvError)
{
    switch (recvError)
    {
        case eRecvErrorUnknown:     return eTunerErrorReadFailed;
        case eRecvErrorReadFailed:  return eTunerErrorReadFailed;
        case eRecvErrorBadData:     return eTunerErrorInvalidChunkData;
        case eRecvErrorEOF:         return eTunerErrorEOF;
        case eRecvErrorDrmInit:     return eTunerErrorDrmInitFailure;
        default:                    return eTunerErrorNone;
    }
}

// ===============================================================================================================
// ===============================================================================================================
