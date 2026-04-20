///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CReceiverCommon.h"
#include "IAVManager.h"
#include <string>

// ===============================================================================================================
// IReceiverControl API
// ===============================================================================================================

class CRendererState;
class CReceiverDiagnostics;
class Clock;
class ISpliceControl;
class IPacketMonitor;
class ITimeslice;

class IReceiverControl
{
public:
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~IReceiverControl() {}
    /// <summary>
    /// Returns the factory of factory
    /// </summary>
    virtual IAVManager*            GetAVManager(void) = 0;
    /// <summary>
    /// Returns the persistent renderer state used by downstream components
    /// </summary>
    virtual CRendererState&        GetRendererState(void) = 0;
    /// <summary>
    /// Returns the diagnostics being sued
    /// </summary>
    virtual CReceiverDiagnostics&  GetDiagnostics(void) = 0;
    /// <summary>
    /// Returns the clock being used
    /// </summary>
    virtual Clock&                 GetClock(void) = 0;
    /// <summary>
    /// Returns the splcie control API
    /// </summary>
    virtual ISpliceControl*        GetSpliceControl(void) = 0;
    /// <summary>
    /// Set a packet monitor to track packets - only one packet monitor supported
    /// </summary>
    virtual void                   SetPacketMonitor(IPacketMonitor* packetMonitor) = 0;
    /// <summary>
    /// Whether still streaming from socket
    /// </summary>
    virtual bool                   IsStreaming(void) const = 0;
    /// <summary>
    /// Whether the receiver has been stopped
    /// </summary>
    virtual bool                   IsRunning(void) const = 0;
    /// <summary>
    /// Whether we're in a VOD/DVR trick mode
    /// </summary>
    virtual bool                   IsTrickMode(void) const = 0;
    /// <summary>
    /// Synchronize to next RAP point
    /// </summary>
    virtual void                   Sync(SyncReason reason, bool sync, bool cleanStall) = 0;
    /// <summary>
    /// Called when clock is rebuffering or sync'ing
    /// </summary>
    virtual void                   OnRebuffer(SyncReason reason, bool sync, bool cleanStall) = 0;
    /// <summary>
    /// Send events to managed side
    /// </summary>
    virtual void                   NotifyStatus(const std::string& status) = 0;
    /// <summary>
    /// Registering internal components for timeslice
    /// </summary>
    virtual void                   RegisterForTimeslice(ITimeslice* timeslice) = 0;
    /// <summary>
    /// Unregistering internal components for timeslice
    /// </summary>
    virtual void                   UnRegisterForTimeslice(ITimeslice* timeslice) = 0;
    /// <summary>
    /// Called to update the splice-in and splice-out timings
    ///
    /// This is a temporary API for measuring the length of ad streams since they
    /// are not reliable.  The length of the ad stream is determined based on
    /// the original PTS of these stream as they are being rendered.  Which means
    /// RTP Ad streams need to be played back once in normal playback mode to allow
    /// for seamless splicing while tricking.
    /// </summary>
    virtual void                   NewFrameArrived(uint64 originalPts) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
