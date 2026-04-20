///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CReceiverCommon.h"
#include <string>
#include <vector>

// ===============================================================================================================
// CDecoderDiagnostics class
//
// Encapsulates diagnostics information for video, audio, and audio description decoders
// ===============================================================================================================

class CReceiverDiagnostics;

class CDecoderDiagnostics
{
    friend class CReceiverDiagnostics;

public:
    //Constructor
    CDecoderDiagnostics(bool isVideo, bool isAudioDescription, CReceiverDiagnostics& diagnostics);

    //Reset all counters
    void                    OnReset(void);

    //Initialize decoder parameters
    void                    OnInitialize(int streamType, int streamId);
    //Called when decoder detects errors while pushing data
    void                    OnDecoderError(int numbytes, int errorCode/* = 0*/);
    //Called when decoder detects DRM errors
    void                    OnDRMError(int numbytes, int errorCode = 0);
    //Called when decoder acquisition fails
    void                    OnFailed(int errorCode = 0);
    //Called when decoder successfully acquired
    void                    OnAcquired(LPVOID context);
    //Called when decoder successfully released
    void                    OnReleased(void);
    //Called when successfully processing data
    void                    OnProcessing(int numbytes, uint64 pts);
    //Called when decoder is dropping data silently
    void                    OnDropping(int numbytes);
    //Called when decoder is flushed
    void                    OnFlushed(void);
    //Called when decoder detects doscontinuity in stream
    void                    OnDiscontinuity(int numbytes);
    //Called when decoder has unblocked AV due to parental control
    void                    OnUnBlocked(void);
    //Called when decoder has blocking AV due to parental control
    void                    OnBlocked(void);

    //Check if throughput has changed, indicating that we're processing this stream
    bool                    IsStreamActive(void);

    //Log last frame pts
    void                    LogPts(uint64 halPts, uint64 bufferPts);
private:
    //Keep track of current buffer status and it's trend
    //Note that we do not want to send too many events so return true only
    //when the buffer status changes by certain threshold in either direction
    bool                    LogStc(uint64 stc);
    //Reset buffer status tracking parameters
    void                    ResetBufferStatus(void);

private:
    //Receiver diagnostics to which thsi belongs
    CReceiverDiagnostics&   Diagnostics;
    //Whether this is audio or video decoder
    bool                    IsVideo;
    //Whether it is audio description stream
    bool                    IsAudioDescription;

    //Elementary stream type
    int                     StreamType;
    //Pid number for this stream
    int                     StreamId;

    //Tracks decoder status
    DecoderStatus           Status;
    //Last valid pts seen
    uint64                  LastPts;

    //HAL Decoder context
    LPVOID                  Context;

    //Data throughput
    struct
    {
        //Number fo bytes dropped due to various reasons
        uint64              Dropped;
        //Number of bytes sent to the HAL decoders
        uint64              Throughput;
        //Checking throughput to the decoders
        uint64              LastThroughput;
    } Data;

    //Decoder related errors
    struct
    {
        //Number of decoder errors reported by decoder HAL
        uint32              Errors;
        //Last decoder HAL error code
        uint32              Code;
    } Decode;

    //DRM related counters
    struct
    {
        //Number of DRM errors that were reported by secure core
        uint32              Errors;
        //Last crypto error code
        uint32              Code;
    } Crypto;

    //Access control related counters
    struct
    {
        //How many times content was blocked
        uint32              BlockedCounter;
        //How many times content was unblocked
        uint32              UnblockedCounter;
    } AccessControl;

    //Buffer status
    struct
    {
        //Whether we are tracking valid buffer
        bool                Valid;
        //Pst of last frame sent to HAL
        uint64              HalPts;
        //Pts of last frame that arrived over the wire
        uint64              LastPts;
        //Current pts/stc delta in the HAL
        int32               CurrentHal;
        //Current pts/stc delta in av-engine
        int32               Current;
        //Minimum pts/stc delta
        int32               Minimum;
        //Maximum pts/stc delta
        int32               Maximum;
        //The last pts/stc delta posted via diag event.
        int32               Posted;
    } BufferStatus;
};

// ===============================================================================================================
// Receiver diagnostics counters
// ===============================================================================================================

class  IReceiverControl;
class  IDiagsManager;
class  CRendererState;
class  CStreamInfo;
class  IDiagsEvent;
class  CDiagsReceiverEvent;
class  CTuneRequest;
struct CReceiverNotificationData;

class CReceiverDiagnostics
{
    friend class CDecoderDiagnostics;

public:
    CReceiverDiagnostics(IReceiverControl* receiverControl, uint32 pipeIdN);
    ~CReceiverDiagnostics();

    //Called to fulfill common parameters and posting of each receiver diagnostics event
    void                    PostEvent(CDiagsReceiverEvent* diagsEvent);

private:
    //Called when at least one decoder becomes active
    void                    OnDecoderActive(void);

public:
    //Called when receiver is reset
    void                    OnReset(void);
    //Called to retrieve periodic update event
    IDiagsEvent*            OnRetrieve(void);
    //Called when receiver is tuned
    void                    OnTune(bool channelChange, CTuneRequest& tuneRequest, int action, const int* timings);
    //Called when receiver is detuned
    void                    OnDetune(void);
    //Called when receiver receives packets
    bool                    OnReceivePacket(uint32 numbytes);
    //Called when a new stream specific buffer delay is set
    void                    OnBufferDelay(int bufferDelay);

    //Send video decoder parameters
    void                    OnVideoDecoderParametersEvent(uint32 fourCC, uint32 width = 0, uint32 height = 0, uint32 pixelAspectX = 0, uint32 pixelAspectY = 0);
    //Send audio decoder parameters
    void                    OnAudioDecoderParametersEvent(uint32 fourCC, uint32 versionNumber = 0, uint32 subband = 0, uint32 samplingRate = 0, uint16 channel = 0, uint32 bytePerSec = 0, uint32 blockSize = 0, uint16 encodeOption = 0, uint16 playerOption = 0);

    //Called when an unsupported codec is being requested
    void                    OnUnsupportedCodec(const CStreamInfo& si, bool supported);

    //Called when receivers detects signal loss
    void                    OnSignalStatusEvent(bool lost);
    //Called when stream is near end of completion
    void                    OnNearEnd(void);
    //Called when a socket error is detecetd
    void                    OnSocketError(int tunerError, int socketError, pkRESULT pkResult, int httpResponse);

    //Called when receiver is paused
    void                    OnPause(uint64 currentTime);
    //Called when receiver is plying again
    void                    OnPlay(void);

    //Called when bitrate becomes available
    void                    OnBitrate(int bitrate);

    //Called when socket is connected
    void                    OnConnected(CReceiverNotificationData* notificationData);
    //Called to report thread timings by sockets
    void                    OnThreadTiming(CReceiverNotificationData* notificationData);
    //Called when DRM license becomes available
    void                    OnDrmLicense(CReceiverNotificationData* notificationData);
    //Called when DRM protection level becomes available
    void                    OnDrmOutputProtectionLevel(CReceiverNotificationData* notificationData);
    //Called when discontinuities detected by sockets
    void                    OnDiscontinuity(CReceiverNotificationData* notificationData);
    //Called when trick state changes in sockets (WMS)
    void                    OnStateChange(CReceiverNotificationData* notificationData);

    //Called when first packet arrives
    void                    OnFirstPacket(void);
    void                    OnFirstPacket(CReceiverNotificationData* notificationData);
    //Called when firrt IFrame is received
    void                    OnFirstIFrameTime(void);
    //Called when the clock is starting
    bool                    OnClockStart(void);
    //Keep tracks of last clock slew (drift)
    void                    OnDrift(int drift) { Drift = drift; }

    //Called when a program is selected to be rendered
    void                    OnProgramId(uint32 programId) { ProgramId = programId; }
    //Caleed when subtitle is being rendererd
    void                    OnSubtitles(uint32 numbytes) { SubtitlesCharsRendered = numbytes; }
    //Called when closed captioning is being rendered
    void                    OnClosedCaption(uint32 numbytes) { BytesCCReceived += numbytes; }
    //Called when closed caotioning is turned on or off
    void                    OnClosedCaptionState(bool state, bool isCC708, int service);

    //Called whenever AV engine goes into recovery mode
    uint32                  OnRebuffer(SyncReason reason, bool sync, bool cleanStall);
    //Called when discontinuity is detected
    void                    OnRtpDiscontinuity(uint32 numPackets);
    //Called when error parsing payload
    void                    OnRtpPayloadError(void) { RtpPayloadError++; }
    //Called when packets are being dropped to reach the next IFrame
    void                    OnRtpDropPacketsUntilRap(uint32 numPacketsDropped) { DroppedPacketsUntilRap += numPacketsDropped; }
    //Called when ECM packets can not be parsed
    void                    OnEcmParseError(void) { EcmParseErrors++; }
    //Called when ECP mapping for Av frames is incorrect
    void                    OnEcmLookupError(void) { EcmLookupError++; }

    //Reset current buffer status
    void                    ResetBufferStatus(uint64 stc);
    //Log buffer status to diagnostics
    void                    LogBuffer(uint64 stc);

    //Whether the stream is still actively flowing
    bool                    IsStreamActive(void);

    //Get global diagnostics
    static IDiagsEvent*     OnRetrieveGlobal(void);

    //Generic command interface
    bool                    Command(const std::string& command, const std::vector<std::string>& args);

private:
    //Diags manager
    IDiagsManager*          mDiagsManager;
    //Renderer state to use
    CRendererState&         mRendererState;
    //Pipe id
    uint32                  mPipeIdN;

    //Unique ids associated with this tune
    uint32                  MediaTransportId;
    uint32                  UniqueId;
    GUID                    ContentId;
    //Channel number this receiver is on currently
    uint32                  Channel;
    //Expected bitrate of currently tuned channel
    uint32                  BitRate;

    //Keeping track of last stc - assuming that stc is being updated regularly
    //through timeslise thread or by other means
    uint64                  Stc;

    //How long it is tuned for
    uint32                  TunedTime;

    //Time taken for last tune
    uint32                  TimeToTuneTotal;
    uint32                  TimeToTuneApp;
    uint32                  TimeToTuneAV;

    //Are the codecs associated with our current stream types supported?
    int                     VideoLastUnsupportedPid;
    int                     AudioLastUnsupportedPid;
    int                     AudioDescriptionLastUnsupportedPid;

    //Whether we are currently streaming
    bool                    IsStreaming;

    //Keep track of number of bytes received
    uint64                  BytesReceived;
    uint64                  LastBytesReceived;

    //Program number being processed
    int                     ProgramId;

    //Keep track of various errors when we do rebuffering
    uint32                  RebufferTimes[kSyncReason_Max];
    //Keeping track of number of decoder stalls
    uint32                  NumRebuffering;
    //Keep count of number of sequence number discontinuities
    uint32                  Discontinuities;
    //Keep count of number of packets in all discontinuites
    uint32                  DiscontinuitiesPackets;
    //Missing RTP payload
    uint32                  RtpPayloadError;
    //Skipped RTP packets until RAP
    uint32                  DroppedPacketsUntilRap;
    //Number of times we failed to parse ECM
    uint32                  EcmParseErrors;
    //Number of frames we failed to do a ECM lookup
    uint32                  EcmLookupError;

    //Clock drift being applied
    int                     Drift;

public:
    //Decoder related diagnostics
    CDecoderDiagnostics*    pVideo;
    CDecoderDiagnostics*    pAudio;
    CDecoderDiagnostics*    pAudioDescription;

private:
    //Keep track of number of bytes received for CC data
    uint32                  BytesCCReceived;

    //Number of DXFP subtitling characters rendered for subtitles
    uint32                  SubtitlesCharsRendered;

    //Keeping track of audio and video off by more than given threshold
    int                     AudioVideoOff;
    //Keep track of last PtsDelta capture
    uint32                  LastPtsDeltaTicks;

    //Keep track of tune timings
    bool                    TimingsValid;
    int                     TimingAction;
    int                     ManagedTimings[TimingSnapshotAt_Max];

    //Thread timings
    uint32                  CpuLast;
    uint32                  CpuAverage;
    uint32                  CpuMaximum;
    uint32                  CpuTotalUsed;

    //Global diagnostics counters
    //
    //Total number of times the decoder has underrun/rebuffered, since box reboot
    static uint32           TotalNumUnderrun;
    //Total number of packets missing in all discontinuities so far, since box reboot
    static uint32           TotalDiscontinuitiesPackets;

#ifdef TV2INTERNAL
public:
    //Various custom commands for testing and diagnostics
    void                    CommandReset(void);
    bool                    ShouldCorruptPES(void);
private:
    //Used for profiling the system and component level timings
    bool                    DropPackets;
    //Used to force discontinuity in Rtp Packet Stream
    uint32                  DropNumPackets;
    //Use this to introduce latency in packet delivery to decoder
    uint32                  PacketLatency;
    //Parameter to cause Audio/Video DSP Crashes
    bool                    CorruptPES;
#endif
};

// ===============================================================================================================
// ===============================================================================================================
