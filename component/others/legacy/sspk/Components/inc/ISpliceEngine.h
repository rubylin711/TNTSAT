///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CReceiverCommon.h"
#include "CSocketDefinitions.h"

// ===============================================================================================================
// ISpliceEngine APIs
// ===============================================================================================================

class  IPacket;
class  CTuneRequest;
struct CSpliceMessage;
struct CReceiverNotificationData;

class ISpliceEngine
{
public:
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~ISpliceEngine() {}
    /// <summary>
    /// Registration with splice engine when the app changes channel
    /// </summary>
    virtual bool   Tune(CTuneRequest& tuneRequest) = 0;
    /// <summary>
    /// UnRegister with splice engine when the app changes channel
    /// </summary>
    virtual void   Detune(bool channelChange) = 0;
    /// <summary>
    /// Trim the splice session  based on current buffer length
    /// </summary>
    virtual void   SetBufferLength(uint64 startTime, uint64 endTime) = 0;
    /// <summary>
    /// Process incoming signal for splicing later
    /// </summary>
    virtual bool   HandleSpliceLaterSignal(const CSpliceMessage* spliceMessage) = 0;
    /// <summary>
    /// Process incoming signal for splicing
    /// </summary>
    virtual bool   HandleSpliceSignal(const CSpliceMessage* spliceMessage) = 0;
    /// <summary>
    /// Process OTT signal (ad insertion in OTT content like SS)
    /// </summary>
    virtual bool   HandleOttSpliceSignal(const CSpliceMessage* spliceMessage) = 0;
    /// <summary>
    /// Process packet received for splice signalling
    /// </summary>
    virtual bool   ProcessPacket(const IPacket& packet) = 0;
    /// <summary>
    /// Called to update the splice-in and splice-out timings
    /// </summary>
    virtual void   SpliceOnGoing(uint64 originalPts) = 0;
    /// <summary>
    /// Called when a stream was joined at transition points
    /// </summary>
    virtual void   StreamJoined(CReceiverNotificationData* notificationData) = 0;
    /// <summary>
    /// Called when the secondary stream has stopped
    /// </summary>
    virtual bool   IsSecondaryStreamEnd(void) = 0;
    /// <summary>
    /// Handle while streaming secondary stream
    /// </summary>
    virtual bool   IsSecondaryStreamError(eTunerError tunerError, eSocketError socketError, int wsaError) = 0;
    /// <summary>
    /// Handle switching between secondary and primary streams
    /// </summary>
    virtual void   OnTimeslice(void) = 0;
    /// <summary>
    /// Retrieve secondary stream url
    /// </summary>
    virtual bool   PreflightNextAd(void) = 0;
};

// ===============================================================================================================
// ===============================================================================================================

class IReceiverControl;

class ISpliceEngineFactory
{
public:
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~ISpliceEngineFactory() {}
    /// <summary>
    /// Acquires a splice engine
    /// </summary>
    /// <param name="receiverControl">[IN] </param>
    /// <return>
    /// <para>Returns a splice engine instance</para>
    /// </return>
    virtual ISpliceEngine* AcquireSpliceEngine(IReceiverControl* receiverControl) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
