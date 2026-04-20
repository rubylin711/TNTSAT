///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDiagsManager.h"
#include "CReceiverDiagnostics.h"
#include "CReceiverNotification.h"
#include "CReceiverConfiguration.h"
#include "CReceiverDiags.h"
#include "IReceiverControl.h"
#include "CRendererState.h"
#include "IDecoder.h"
#include "CTuneRequest.h"
#include "Trace.h"
#include <string>
#include <vector>
using namespace std;

//#define RECEIVER_SPEW
#if defined(RECEIVER_SPEW)
#define RECEIVER_MSG(x) TRACE(x)
#else
#define RECEIVER_MSG(x)
#endif

// ===============================================================================================================
// Managing buffer status for active audio/video streams
// ===============================================================================================================

CDecoderDiagnostics::CDecoderDiagnostics(bool isVideo, bool isAudioDescription, CReceiverDiagnostics& diagnostics)
    : Diagnostics(diagnostics)
    , IsVideo(isVideo)
    , IsAudioDescription(isAudioDescription)
{
}

void CDecoderDiagnostics::OnReset(void)
{
    //Elementary stream type
    StreamType = 0;
    //Pid number for this stream
    StreamId = 0;

    //Tracks decoder status
    Status = kDecoderStatus_Idle;
    //Last valid pts seen
    LastPts = INVALID_TIME;

    //HAL Decoder context
    Context = NULL;

    //Data throughput
    Data.Dropped = 0;
    Data.Throughput = 0;
    Data.LastThroughput = 0;

    //Decode error
    Decode.Errors = 0;
    Decode.Code = 0;

    //DRM related counters
    Crypto.Errors = 0;
    Crypto.Code = 0;

    //Access control related counters
    AccessControl.BlockedCounter = 0;
    AccessControl.UnblockedCounter = 0;

    //Buffer status
    ResetBufferStatus();
}

void CDecoderDiagnostics::OnInitialize(int streamType, int streamId)
{
    //Elementary stream type
    StreamType = streamType;
    //Pid number for this stream
    StreamId = streamId;

    //Tracks decoder status
    Status = kDecoderStatus_Idle;
    //Last valid pts seen
    LastPts = INVALID_TIME;
}

void CDecoderDiagnostics::OnDecoderError(int numbytes, int errorCode/* = 0*/)
{
    //Keep track of number of bytes dropped
    Data.Dropped += numbytes;
    //Keep track of number of decoder errors
    Decode.Errors++;

    //Send an event when streaming state changes
    if (Status != kDecoderStatus_DecoderError/* || Decode.Code != errorCode*/)
    {
        //Save decode error code
        Decode.Code = errorCode;
        //Save new state
        Status = kDecoderStatus_DecoderError;
        //Send an event to signal a decoder error
        Diagnostics.PostEvent(new CDiagsReceiverDecoderEvent(IsVideo, IsAudioDescription, Status, StreamType, StreamId, LastPts, errorCode));
    }
}

void CDecoderDiagnostics::OnDRMError(int numbytes, int errorCode/* = 0*/)
{
    //Keep track of number of bytes dropped
    Data.Dropped += numbytes;
    //Keep track of number fo crypto errors
    Crypto.Errors++;

    //Send an event when streaming state changes
    if (Status != kDecoderStatus_DRMError/* || Crypto.Code != errorCode*/)
    {
        //Keep track of crypto error code
        Crypto.Code = errorCode;
        //Save new state
        Status = kDecoderStatus_DRMError;
        //Send an event to signal a DRM error has occurred
        Diagnostics.PostEvent(new CDiagsReceiverDecoderEvent(IsVideo, IsAudioDescription, Status, StreamType, StreamId, LastPts, errorCode));
    }
}

void CDecoderDiagnostics::OnFailed(int errorCode/* = 0*/)
{
    //Keep track of number of decoder errors
    Decode.Errors++;

    //Send an event when streaming state changes
    if (Status != kDecoderStatus_Failed)
    {
        //Save decode error code
        Decode.Code = errorCode;
        //Save new state
        Status = kDecoderStatus_Failed;
        //Send an event to signal decoder acquisition has failed
        Diagnostics.PostEvent(new CDiagsReceiverDecoderEvent(IsVideo, IsAudioDescription, Status, StreamType, StreamId, LastPts, errorCode));
    }
}

void CDecoderDiagnostics::OnAcquired(LPVOID context)
{
    //HAL Decoder context
    Context = context;
    //Clear decode error code
    Decode.Code = 0;
    //Clear crypto error code
    Crypto.Code = 0;

    //Send an event when streaming state changes
    if (Status != kDecoderStatus_Acquired)
    {
        //Save new state
        Status = kDecoderStatus_Acquired;
        //Send streaming started event - used for SeaMonkey automation tests
        Diagnostics.OnDecoderActive();
        //Send decoder acquisition event
        Diagnostics.PostEvent(new CDiagsReceiverDecoderEvent(IsVideo, IsAudioDescription, Status, StreamType, StreamId, LastPts, 0));
    }
}

void CDecoderDiagnostics::OnReleased(void)
{
    //HAL Decoder context
    Context = NULL;
    //Clear decode error code
    Decode.Code = 0;
    //Clear crypto error code
    Crypto.Code = 0;

    //Send an event when streaming state changes
    if (Status != kDecoderStatus_Released)
    {
        //Save new state
        Status = kDecoderStatus_Released;
        //Send streaming stopped event - used for SeaMonkey automation tests
        Diagnostics.PostEvent(new CDiagsReceiverDecoderEvent(IsVideo, IsAudioDescription, Status, StreamType, StreamId, LastPts, 0));
        //Send decoder release event
        Diagnostics.OnDecoderActive();
    }
}

void CDecoderDiagnostics::OnProcessing(int numbytes, uint64 pts)
{
    //Keep track of number of bytes sent
    Data.Throughput += numbytes;

    //Called to save last PTS
    if (IS_VALID_TIME(pts))
    {
        LastPts = pts;
    }

    //Send an event when streaming state changes
    if (Status != kDecoderStatus_Processing)
    {
        //Clear decode error code
        Decode.Code = 0;
        //Clear crypto error code
        Crypto.Code = 0;
        //Save new state
        Status = kDecoderStatus_Processing;
        //Send successfully processing data event
        Diagnostics.PostEvent(new CDiagsReceiverDecoderEvent(IsVideo, IsAudioDescription, Status, StreamType, StreamId, LastPts, 0));
    }
}

void CDecoderDiagnostics::OnDropping(int numbytes)
{
    //Keep track of number of bytes dropped
    Data.Dropped += numbytes;

    //Send an event when streaming state changes
    if (Status != kDecoderStatus_Dropping)
    {
        //Save new state
        Status = kDecoderStatus_Dropping;
        //Send event to signal data is being dropped silently
        Diagnostics.PostEvent(new CDiagsReceiverDecoderEvent(IsVideo, IsAudioDescription, Status, StreamType, StreamId, LastPts, 0));
    }
}

void CDecoderDiagnostics::OnFlushed(void)
{
    //Send an event when streaming state changes
    if (Status != kDecoderStatus_Flushed)
    {
        //Save new state
        Status = kDecoderStatus_Flushed;
        //Send an event signalling decoder has been flushed
        Diagnostics.PostEvent(new CDiagsReceiverDecoderEvent(IsVideo, IsAudioDescription, Status, StreamType, StreamId, LastPts, 0));
    }
}

void CDecoderDiagnostics::OnDiscontinuity(int numbytes)
{
    //Keep track of number of bytes dropped
    Data.Dropped += numbytes;

    //Send an event when streaming state changes
    if (Status != kDecoderStatus_Discontinuity)
    {
        //Save new state
        Status = kDecoderStatus_Discontinuity;
        //Send an event to signal a discontinuity was detected
        Diagnostics.PostEvent(new CDiagsReceiverDecoderEvent(IsVideo, IsAudioDescription, Status, StreamType, StreamId, LastPts, 0));
    }
}

void CDecoderDiagnostics::OnUnBlocked(void)
{
    //Keep track of how many times thsi stream was unblocked due to parental control
    AccessControl.UnblockedCounter++;
    //Send an event signalling AV has been unblocked due to parental control
    Diagnostics.PostEvent(new CDiagsReceiverDecoderEvent(IsVideo, IsAudioDescription, kDecoderStatus_UnBlocked, StreamType, StreamId, LastPts, 0));
}

void CDecoderDiagnostics::OnBlocked(void)
{
    //Keep track of how many times thsi stream was blocked due to parental control
    AccessControl.BlockedCounter++;
    //Send an event signalling AV has been blocked due to parental control
    Diagnostics.PostEvent(new CDiagsReceiverDecoderEvent(IsVideo, IsAudioDescription, kDecoderStatus_Blocked, StreamType, StreamId, LastPts, 0));
}

bool CDecoderDiagnostics::IsStreamActive(void)
{
    //Whether data is flowing to the decoders
    bool isProcessing = (Data.Throughput != Data.LastThroughput);
    Data.LastThroughput = Data.Throughput;
    return isProcessing;
}

void CDecoderDiagnostics::LogPts(uint64 halPts, uint64 bufferPts)
{
    //Pts's are now valid
    BufferStatus.Valid = true;
    //Save last pts sent to HAL ES buffers
    BufferStatus.HalPts = halPts;
    //Save pts of last frame buffered
    BufferStatus.LastPts = bufferPts;
}

bool CDecoderDiagnostics::LogStc(uint64 stc)
{
    if (BufferStatus.Valid)
    {
        //Log HAL buffer status
        BufferStatus.CurrentHal = (int32)(((int64)BufferStatus.HalPts - (int64)stc) / 90);
        //Log the current buffer status in milliseconds
        BufferStatus.Current = (int32)(((int64)BufferStatus.LastPts - (int64)stc) / 90);

        //Update the trend with latest delta value
        if (BufferStatus.Current < BufferStatus.Minimum) BufferStatus.Minimum = BufferStatus.Current;
        else
        if (BufferStatus.Current > BufferStatus.Maximum) BufferStatus.Maximum = BufferStatus.Current;

        //Send an event when the buffer status has changed more than given threshold in either direction.
        if (BufferStatus.Current <= BufferStatus.Posted - gReceiverConfiguration.PtsDeltaThreshold)
            return true;
        else
        if (BufferStatus.Current >= BufferStatus.Posted + gReceiverConfiguration.PtsDeltaThreshold)
            return true;
    }
    return false;
}

void CDecoderDiagnostics::ResetBufferStatus(void)
{
    BufferStatus.Valid = false;
    BufferStatus.HalPts = 0;
    BufferStatus.LastPts = 0;
    BufferStatus.CurrentHal = 0;
    BufferStatus.Current = 0;
    BufferStatus.Minimum = 0;
    BufferStatus.Maximum = 0;
    BufferStatus.Posted = 0;
}

// ===============================================================================================================
// ===============================================================================================================

//Global diagnostics counters
//
//Total number of times the decoder has underrun/rebuffered, since box reboot
uint32 CReceiverDiagnostics::TotalNumUnderrun = 0;
//Total number of packets missing in all discontinuities so far, since box reboot
uint32 CReceiverDiagnostics::TotalDiscontinuitiesPackets = 0;

CReceiverDiagnostics::CReceiverDiagnostics(IReceiverControl* receiverControl, uint32 pipeIdN)
    : mDiagsManager(receiverControl->GetAVManager()->GetDiagsManager())
    , mRendererState(receiverControl->GetRendererState())
    , mPipeIdN(pipeIdN)
{
    pVideo = NEW_NO_THROW CDecoderDiagnostics(true, false, *this);
    ASSERT(pVideo);

    pAudio = NEW_NO_THROW CDecoderDiagnostics(false, false, *this);
    ASSERT(pAudio);

    pAudioDescription = NEW_NO_THROW CDecoderDiagnostics(false, true, *this);
    ASSERT(pAudioDescription);

    //Initialize parameters reset only across channel changes
    OnDetune();
    //Initialize rest of the counters
    OnReset();
}

CReceiverDiagnostics::~CReceiverDiagnostics()
{
    if (pVideo)
    {
        delete pVideo;
    }

    if (pAudio)
    {
        delete pAudio;
    }

    if (pAudioDescription)
    {
        delete pAudioDescription;
    }
}

void CReceiverDiagnostics::PostEvent(CDiagsReceiverEvent* diagsEvent)
{
    //Patches event with interesting information to be sent to trace logs
    if (diagsEvent)
    {
        diagsEvent->PipeIdN = mPipeIdN;
        diagsEvent->MediaTransportId = MediaTransportId;
        diagsEvent->UniqueId = UniqueId;
        diagsEvent->ContentId = ContentId;
        diagsEvent->Channel = Channel;
        mDiagsManager->PostEvent(diagsEvent);
    }
}

void CReceiverDiagnostics::OnDecoderActive(void)
{
    bool wasStreaming = IsStreaming;
    IsStreaming = pVideo->Context || pAudio->Context;
    if (wasStreaming != IsStreaming)
    {
        PostEvent(new CDiagsReceiverStreamingEvent(IsStreaming));
    }
}

void CReceiverDiagnostics::OnReset(void)
{
    //How long it is tuned for
    TunedTime = Executive_GetTickCount();

    //Time taken for last tune
    TimeToTuneTotal = 0;
    TimeToTuneApp = 0;
    TimeToTuneAV = 0;

    //Whether we are currently streaming
    IsStreaming = false;

    //Keep track of number of bytes received
    BytesReceived = 0;
    LastBytesReceived = 0;

    //Program number being processed
    ProgramId = -1;

    //Keep track of various errors when we do rebuffering
    memset(RebufferTimes, 0, sizeof(RebufferTimes));
    //Keeping track of number of decoder stalls
    NumRebuffering = 0;
    //Keep count of number of sequence number discontinuities
    Discontinuities = 0;
    //Keep count of number of packets in all discontinuites
    DiscontinuitiesPackets = 0;
    //Missing RTP payload
    RtpPayloadError = 0;
    //Skipped RTP packets until RAP
    DroppedPacketsUntilRap = 0;
    //Number of times we failed to parse ECM
    EcmParseErrors = 0;
    //Number of frames we failed to do a ECM lookup
    EcmLookupError = 0;

    //Clock drift being applied
    Drift = 0;

    //Decoder related diagnostics
    pVideo->OnReset();
    pAudio->OnReset();
    pAudioDescription->OnReset();

    //Keep track of number of bytes received for CC data
    BytesCCReceived = 0;

    //Keep track of DXFP subtitling data
    SubtitlesCharsRendered = 0;

    //Keeping track of audio and video off by more than given threshold
    AudioVideoOff = 0;
    LastPtsDeltaTicks = Executive_GetTickCount();

    //Thread timings
    CpuLast = 0;
    CpuAverage = 0;
    CpuMaximum = 0;
    CpuTotalUsed = 0;

#ifdef TV2INTERNAL
    //Various test commands
    CommandReset();
#endif
}

IDiagsEvent* CReceiverDiagnostics::OnRetrieve(void)
{
    CDiagsReceiverUpdateEvent* diagsEvent = new CDiagsReceiverUpdateEvent();
    if (diagsEvent == NULL)
        return diagsEvent;

    //Pipe to which this belongs
    diagsEvent->PipeIdN = mPipeIdN;
    //Unique ids associated with this tune
    diagsEvent->MediaTransportId = MediaTransportId;
    diagsEvent->UniqueId = UniqueId;
    diagsEvent->ContentId = ContentId;
    //Channel number this receiver is on currently
    diagsEvent->Channel = Channel;
    //Expected bitrate of currently tuned channel
    diagsEvent->BitRate = BitRate;

    //How long it is tuned for
    diagsEvent->TuneTime = Executive_GetTickCount() - TunedTime;

    //Time taken for last tune
    diagsEvent->TimeToTuneTotal = TimeToTuneTotal;
    diagsEvent->TimeToTuneApp = TimeToTuneApp;
    diagsEvent->TimeToTuneAV = TimeToTuneAV;

    //Whether we are currently streaming
    diagsEvent->IsStreaming = IsStreaming;

    //Keep track of number of bytes received
    diagsEvent->BytesReceived = BytesReceived;

    //Program number being processed
    diagsEvent->ProgramId = ProgramId;

    //Keep track of various errors when we do rebuffering
    memcpy_s(diagsEvent->RebufferTimes, sizeof(diagsEvent->RebufferTimes), RebufferTimes, sizeof(RebufferTimes));
    //Keeping track of number of decoder stalls
    diagsEvent->NumRebuffering = NumRebuffering;
    //Keep count of number of sequence number discontinuities
    diagsEvent->Discontinuities = Discontinuities;
    //Keep count of number of packets in all discontinuites
    diagsEvent->DiscontinuitiesPackets = DiscontinuitiesPackets;
    //Missing RTP payload
    diagsEvent->RtpPayloadError = RtpPayloadError;
    //Skipped RTP packets until RAP
    diagsEvent->DroppedPacketsUntilRap = DroppedPacketsUntilRap;
    //Number of times we failed to parse ECM
    diagsEvent->EcmParseErrors = EcmParseErrors;
    //Number of frames we failed to do a ECM lookup
    diagsEvent->EcmLookupError = EcmLookupError;

    //Clock drift being applied
    diagsEvent->Drift = Drift;

    //pVideo decoder related diagnostics
    diagsEvent->VideoStreamType = pVideo->StreamType;
    diagsEvent->VideoPtsDelta = pVideo->BufferStatus.Current;
    diagsEvent->VideoPtsDeltaMin = pVideo->BufferStatus.Minimum;
    diagsEvent->VideoPtsDeltaMax = pVideo->BufferStatus.Maximum;
    diagsEvent->VideoPtsDeltaHal = pVideo->BufferStatus.CurrentHal;
    diagsEvent->VideoDataDropped = pVideo->Data.Dropped;
    diagsEvent->VideoDataThroughput = pVideo->Data.Throughput;
    diagsEvent->VideoDrmErrors = pVideo->Crypto.Errors;
    diagsEvent->VideoCryptoError = pVideo->Crypto.Code;
    diagsEvent->VideoAccessControlBlockedCounter = pVideo->AccessControl.BlockedCounter;
    diagsEvent->VideoAccessControlUnblockedCounter = pVideo->AccessControl.UnblockedCounter;
    //Get HAL video decoder diagnostics
    if (mRendererState.ActiveVideoDecoder)
    {
        DECODER_VIDEO_DIAGS videoDiags;
        if (mRendererState.ActiveVideoDecoder->IoControl(DECODER_GETVIDEODIAGS, NULL, 0, &videoDiags, sizeof(videoDiags)) == DECODER_ERR_SUCCESS)
        {
            diagsEvent->VideoFramesDecoded = videoDiags.u32FramesDecoded;
            diagsEvent->VideoFramesDropped = videoDiags.u32FramesDropped;
            diagsEvent->VideoDecoderErrors = videoDiags.u32FramesErrors;
            diagsEvent->VideoBufferUnderruns = videoDiags.u32FramesUnderruns;
            diagsEvent->VideoBufferOverflows = videoDiags.u32FramesOverflows;
            diagsEvent->VideoFifoSize = videoDiags.u32FifoSize;
            diagsEvent->VideoFifoRd = videoDiags.u32FifoReadPtr;
            diagsEvent->VideoFifoWr = videoDiags.u32FifoWritePtr;
            memcpy_s(diagsEvent->VideoDSPCrashes, sizeof(diagsEvent->VideoDSPCrashes), videoDiags.szDecoderResets, sizeof(videoDiags.szDecoderResets));
        }
        DECODER_VIDEO_PICTUREINFO videoPictInfo;
        if (mRendererState.ActiveVideoDecoder->IoControl(DECODER_GETPICTUREINFO, NULL, 0, &videoPictInfo, sizeof(videoPictInfo)) == DECODER_ERR_SUCCESS)
        {
            diagsEvent->EncodecPictureWidth = (int)(videoPictInfo.u16Width);
            diagsEvent->EncodecPictureHeight = (int)(videoPictInfo.u16Height);
            diagsEvent->AspectX = (int)(videoPictInfo.u16Aspx);
            diagsEvent->AspectY = (int)videoPictInfo.u16Aspy;
        }
    }

    //pAudio decoder related diagnostics
    diagsEvent->AudioStreamType = pAudio->StreamType;
    diagsEvent->AudioPtsDelta = pAudio->BufferStatus.Current;
    diagsEvent->AudioPtsDeltaMin = pAudio->BufferStatus.Minimum;
    diagsEvent->AudioPtsDeltaMax = pAudio->BufferStatus.Maximum;
    diagsEvent->AudioPtsDeltaHal = pAudio->BufferStatus.CurrentHal;
    diagsEvent->AudioDataDropped = pAudio->Data.Dropped;
    diagsEvent->AudioDataThroughput = pAudio->Data.Throughput;
    diagsEvent->AudioDrmErrors = pAudio->Crypto.Errors;
    diagsEvent->AudioCryptoError = pAudio->Crypto.Code;
    diagsEvent->AudioAccessControlBlockedCounter = pAudio->AccessControl.BlockedCounter;
    diagsEvent->AudioAccessControlUnblockedCounter = pAudio->AccessControl.UnblockedCounter;
    //Get HAL audio decoder diagnostice
    if (mRendererState.ActiveAudioDecoder)
    {
        DECODER_AUDIO_DIAGS audioDiags;
        if (mRendererState.ActiveAudioDecoder->IoControl(DECODER_GETAUDIODIAGS, NULL, 0, &audioDiags, sizeof(audioDiags)) == DECODER_ERR_SUCCESS)
        {
            diagsEvent->AudioSamplesDecoded = audioDiags.u32FramesDecoded;
            diagsEvent->AudioSamplesDropped = audioDiags.u32FramesDropped;
            diagsEvent->AudioDecoderErrors = audioDiags.u32FramesErrors;
            diagsEvent->AudioBufferUnderruns = audioDiags.u32SamplesUnderruns;
            diagsEvent->AudioBufferOverflows = audioDiags.u32FramesOverflows;
            diagsEvent->AudioFifoSize = audioDiags.u32FifoSize;
            diagsEvent->AudioFifoRd = audioDiags.u32FifoReadPtr;
            diagsEvent->AudioFifoWr = audioDiags.u32FifoWritePtr;
            memcpy_s(diagsEvent->AudioDSPCrashes, sizeof(diagsEvent->AudioDSPCrashes), audioDiags.szDecoderResets, sizeof(audioDiags.szDecoderResets));
        }
    }

    //pAudio description decoder related diagnostics
    diagsEvent->AudioDescriptionStreamType = pAudioDescription->StreamType;
    diagsEvent->AudioDescriptionPtsDelta = pAudioDescription->BufferStatus.Current;
    diagsEvent->AudioDescriptionPtsDeltaMin = pAudioDescription->BufferStatus.Minimum;
    diagsEvent->AudioDescriptionPtsDeltaMax = pAudioDescription->BufferStatus.Maximum;
    diagsEvent->AudioDescriptionPtsDeltaHal = pAudioDescription->BufferStatus.CurrentHal;
    diagsEvent->AudioDescriptionDataDropped = pAudioDescription->Data.Dropped;
    diagsEvent->AudioDescriptionDataThroughput = pAudioDescription->Data.Throughput;
    diagsEvent->AudioDescriptionDrmErrors = pAudioDescription->Crypto.Errors;
    diagsEvent->AudioDescriptionCryptoError = pAudioDescription->Crypto.Code;
    diagsEvent->AudioDescriptionAccessControlBlockedCounter = pAudioDescription->AccessControl.BlockedCounter;
    diagsEvent->AudioDescriptionAccessControlUnblockedCounter = pAudioDescription->AccessControl.UnblockedCounter;
    //Get HAL audio description decoder diagnostice
    if (mRendererState.ActiveAudioDescriptionDecoder != NULL)
    {
        DECODER_AUDIO_DIAGS audioDescriptionDiags;
        if (mRendererState.ActiveAudioDescriptionDecoder->IoControl(DECODER_GETAUDIODIAGS, NULL, 0, &audioDescriptionDiags, sizeof(audioDescriptionDiags)) == DECODER_ERR_SUCCESS)
        {
            diagsEvent->AudioDescriptionSamplesDecoded = audioDescriptionDiags.u32FramesDecoded;
            diagsEvent->AudioDescriptionSamplesDropped = audioDescriptionDiags.u32FramesDropped;
            diagsEvent->AudioDescriptionDecoderErrors = audioDescriptionDiags.u32FramesErrors;
            diagsEvent->AudioDescriptionBufferUnderruns = audioDescriptionDiags.u32SamplesUnderruns;
            diagsEvent->AudioDescriptionBufferOverflows = audioDescriptionDiags.u32FramesOverflows;
            diagsEvent->AudioDescriptionFifoSize = audioDescriptionDiags.u32FifoSize;
            diagsEvent->AudioDescriptionFifoRd = audioDescriptionDiags.u32FifoReadPtr;
            diagsEvent->AudioDescriptionFifoWr = audioDescriptionDiags.u32FifoWritePtr;
            memcpy_s(diagsEvent->AudioDescriptionDSPCrashes, sizeof(diagsEvent->AudioDescriptionDSPCrashes), audioDescriptionDiags.szDecoderResets, sizeof(audioDescriptionDiags.szDecoderResets));
        }
    }

    //Keep track of number of bytes received for CC data
    diagsEvent->BytesCCReceived = BytesCCReceived;

    //Keep track of DXFP subtitling data
    diagsEvent->SubtitlesCharsRendered = SubtitlesCharsRendered;

    //Thread timings
    diagsEvent->CpuTotalUsed = CpuTotalUsed;
    diagsEvent->CpuLast = CpuLast;
    diagsEvent->CpuAverage = CpuAverage;
    diagsEvent->CpuMaximum = CpuMaximum;

    //Total number of times the decoder has underrun/rebuffered, since box reboot
    diagsEvent->TotalNumUnderrun = CReceiverDiagnostics::TotalNumUnderrun;
    //Total number of packets missing in all discontinuities so far, since box reboot
    diagsEvent->TotalDiscontinuitiesPackets = CReceiverDiagnostics::TotalDiscontinuitiesPackets;
    return diagsEvent;
}

void CReceiverDiagnostics::OnTune(bool channelChange, CTuneRequest& tuneRequest, int action, const int* timings)
{
    //Save tune timing if any was passed
    TimingsValid = (action != TimingAction_Unknown) && timings;
    if (TimingsValid)
    {
        TimingAction = action;
        memcpy_s(ManagedTimings, TimingSnapshotAt_MaxManaged * sizeof(int), timings, TimingSnapshotAt_MaxManaged * sizeof(int));
    }

    //Unique ids associated with this tune
    MediaTransportId = tuneRequest.MediaTransportId;
    UniqueId = tuneRequest.UniqueId;
    ContentId = tuneRequest.ContentId;
    //Save additional channel change related state
    if (channelChange)
    {
        Channel = tuneRequest.Channel;
        BitRate = tuneRequest.NetworkBandwidthUsage();
    }

    //Send an event signifying the reason for tune
    if (action != 0)
    {
        PostEvent(new CDiagsReceiverTuneEvent(action, tuneRequest.Speed, tuneRequest.Rap));
    }
}

void CReceiverDiagnostics::OnDetune(void)
{
    //Cleat the stc
    Stc = 0;

    //Unique ids associated with this tune
    MediaTransportId = 0;
    UniqueId = 0;
    memset(&ContentId, 0, sizeof(ContentId));
    //Channel number this receiver is on currently
    Channel = INVALID_UINT32;
    //Clear the bitrate on detune
    BitRate = 0;

    //Tune timings
    TimingsValid = false;
    TimingAction = TimingAction_Unknown;
    memset(ManagedTimings, 0, sizeof(ManagedTimings));

    //Clear last unsupported codec tracking parameters
    VideoLastUnsupportedPid = -1;
    AudioLastUnsupportedPid = -1;
    AudioDescriptionLastUnsupportedPid = -1;
}

bool CReceiverDiagnostics::OnReceivePacket(uint32 numbytes)
{
#ifdef TV2INTERNAL
    if (DropPackets)
    {
        return false;
    }
    if (DropNumPackets > 0)
    {
        DropNumPackets--;
        return false;
    }
    if (PacketLatency > 0)
    {
        Executive_Sleep(PacketLatency);
        PacketLatency = 0;
    }
#endif

    //Keep track of number of bytes received
    BytesReceived += numbytes;
    return true;
}

void CReceiverDiagnostics::OnBufferDelay(int bufferDelay)
{
    PostEvent(new CDiagsSetStreamBufferDelay(bufferDelay));
}

void CReceiverDiagnostics::OnVideoDecoderParametersEvent(uint32 fourCC, uint32 width/* = 0*/, uint32 height/* = 0*/, uint32 pixelAspectX/* = 0*/, uint32 pixelAspectY/* = 0*/)
{
    PostEvent(new CDiagsVideoDecoderParametersEvent(fourCC, width, height, pixelAspectX, pixelAspectY));
}

void CReceiverDiagnostics::OnAudioDecoderParametersEvent(uint32 fourCC, uint32 versionNumber/* = 0*/, uint32 subband/* = 0*/, uint32 samplingRate/* = 0*/, uint16 channel/* = 0*/, uint32 bytePerSec/* = 0*/, uint32 blockSize/* = 0*/, uint16 encodeOption/* = 0*/, uint16 playerOption/* = 0*/)
{
    PostEvent(new CDiagsAudioDecoderParametersEvent(fourCC, versionNumber, subband, samplingRate, channel, bytePerSec, blockSize, encodeOption, playerOption));
}

void CReceiverDiagnostics::OnUnsupportedCodec(const CStreamInfo& si, bool supported)
{
    if (si.IsVideo())
    {
        if (supported)
        {
            VideoLastUnsupportedPid = -1;
        }
        else if (!supported && VideoLastUnsupportedPid != si.StreamId)
        {
            VideoLastUnsupportedPid = si.StreamId;
            PostEvent(new CDiagsReceiverDecoderEvent(si.IsVideo(), si.IsAudioDescription(), kDecoderStatus_Unsupported, si.Format, si.StreamId, INVALID_TIME));
        }
    }
    else
    if (si.IsAudioDescription())
    {
        //Post a diag the first time we discover this codec type is not supported
        if (supported)
        {
            AudioDescriptionLastUnsupportedPid = -1;
        }
        else if (!supported && AudioDescriptionLastUnsupportedPid != si.StreamId)
        {
            AudioDescriptionLastUnsupportedPid = si.StreamId;
            PostEvent(new CDiagsReceiverDecoderEvent(si.IsVideo(), si.IsAudioDescription(), kDecoderStatus_Unsupported, si.Format, si.StreamId, INVALID_TIME));
        }
    }
    else
    {
        if (supported)
        {
            AudioLastUnsupportedPid = -1;
        }
        else if (!supported && AudioLastUnsupportedPid != si.StreamId)
        {
            AudioLastUnsupportedPid = si.StreamId;
            PostEvent(new CDiagsReceiverDecoderEvent(si.IsVideo(), si.IsAudioDescription(), kDecoderStatus_Unsupported, si.Format, si.StreamId, INVALID_TIME));
        }
    }
}

void CReceiverDiagnostics::OnSignalStatusEvent(bool signalLost)
{
    PostEvent(new CDiagsSignalLostEvent(signalLost));
}

void CReceiverDiagnostics::OnNearEnd(void)
{
    PostEvent(new CDiagsNearEndEvent());
}

void CReceiverDiagnostics::OnSocketError(int tunerError, int socketError, pkRESULT pkResult, int httpResponse)
{
    PostEvent(new CDiagsReceiverTunerErrorEvent(tunerError, socketError, pkResult, httpResponse));
}

void CReceiverDiagnostics::OnPause(uint64 currentTime)
{
    PostEvent(new CDiagsReceiverPauseEvent(currentTime));
}

void CReceiverDiagnostics::OnPlay(void)
{
    PostEvent(new CDiagsReceiverPlayEvent());
}

void CReceiverDiagnostics::OnBitrate(int bitrate)
{
    if (bitrate)
    {
        BitRate = bitrate;
    }
    PostEvent(new CDiagsReceiverOTTBitRateEvent(bitrate));
}

void CReceiverDiagnostics::OnConnected(CReceiverNotificationData* notificationData)
{
    ManagedTimings[TimingSnapshotAt_Connecting] = notificationData->ConnectTimes.ConnectingTime;
    ManagedTimings[TimingSnapshotAt_Connected] = notificationData->ConnectTimes.ConnectedTime;
}

void CReceiverDiagnostics::OnThreadTiming(CReceiverNotificationData* notificationData)
{
    CpuLast = notificationData->ThreadTimes.DiagsCpuUsageLast;
    CpuAverage = notificationData->ThreadTimes.DiagsCpuUsageAverage;
    CpuMaximum = notificationData->ThreadTimes.DiagsCpuUsageMaximum;
    CpuTotalUsed = notificationData->ThreadTimes.DiagsCpuUsageTotalUsed;

    TRACE(("ThreadTimes[%08x][%d]: Last=%d, Average=%d, Maximum=%d", mPipeIdN, CpuTotalUsed, CpuLast, CpuAverage, CpuMaximum));
}

void CReceiverDiagnostics::OnDrmLicense(CReceiverNotificationData* notificationData)
{
    // Make sure that we have enough space to copy times
    ASSERT(DrmLicenseTimes_Count <= TimingSnapshotAt_TuneStep9 - TimingSnapshotAt_TuneStep2);
    for (int i=0; i<DrmLicenseTimes_Count; i++)
    {
        ManagedTimings[TimingSnapshotAt_TuneStep3 + i] = notificationData->DrmLicense.Timing[i];
    }
}

void CReceiverDiagnostics::OnDrmOutputProtectionLevel(CReceiverNotificationData* notificationData)
{
    //Send a diagnostic event for Play Ready OPL for all streams
    PostEvent(new CDiagsPlayReadyOPLEvent
        (
        notificationData->DrmOutputProtectionLevel.CGMSALevel,
        notificationData->DrmOutputProtectionLevel.MacrovisionLevel,
        notificationData->DrmOutputProtectionLevel.HDCPAction,
        notificationData->DrmOutputProtectionLevel.DisableComponentVideo
        ));
}

void CReceiverDiagnostics::OnDiscontinuity(CReceiverNotificationData* notificationData)
{
    if (notificationData->DiscontinuityStats == NULL)
        return;

    PostEvent(new CDiagsReceiverOTTDiscontinuityEvent
        (
        notificationData->DiscontinuityStats->AudioDiscontinuityCount,
        notificationData->DiscontinuityStats->VideoDiscontinuityCount,
        notificationData->DiscontinuityStats->GetTicks()
        ));

    //Keep a cumulative counter of all OTT discontinuities
    if (notificationData->DiscontinuityStats->AudioDiscontinuityCount) Discontinuities++;
    DiscontinuitiesPackets += notificationData->DiscontinuityStats->AudioDiscontinuityCount;
    if (notificationData->DiscontinuityStats->VideoDiscontinuityCount) Discontinuities++;
    DiscontinuitiesPackets += notificationData->DiscontinuityStats->VideoDiscontinuityCount;
}

void CReceiverDiagnostics::OnStateChange(CReceiverNotificationData* notificationData)
{
    PostEvent(new CDiagsReceiverTuneEvent(notificationData->StateChange.Action, notificationData->StateChange.Speed, 0));
}

void CReceiverDiagnostics::OnFirstPacket(void)
{
    ManagedTimings[TimingSnapshotAt_ReceivedFirstPacket] = ManagedTimings[TimingSnapshotAt_ReceivedFirstIFrame] = Executive_GetTickCount();
}

void CReceiverDiagnostics::OnFirstPacket(CReceiverNotificationData* notificationData)
{
    ManagedTimings[TimingSnapshotAt_ReceivedFirstPacket] = ManagedTimings[TimingSnapshotAt_ReceivedFirstIFrame] = notificationData->PacketTime.Tick;
    TRACE(("FirstPacketTime[%08x]: %d", mPipeIdN, notificationData->PacketTime.Tick));
}

void CReceiverDiagnostics::OnFirstIFrameTime(void)
{
    if (TimingsValid)
    {
        ManagedTimings[TimingSnapshotAt_ReceivedFirstIFrame] = Executive_GetTickCount();
    }
}

bool CReceiverDiagnostics::OnClockStart(void)
{
    if (TimingsValid)
    {
        ManagedTimings[TimingSnapshotAt_ClockStarted] = Executive_GetTickCount();

        TimeToTuneTotal = ManagedTimings[TimingSnapshotAt_ClockStarted] - ManagedTimings[TimingSnapshotAt_Instantiate];
        TimeToTuneApp   = ManagedTimings[TimingSnapshotAt_TuneFromApp ] - ManagedTimings[TimingSnapshotAt_Instantiate];
        TimeToTuneAV    = ManagedTimings[TimingSnapshotAt_ClockStarted] - ManagedTimings[TimingSnapshotAt_TuneFromApp];

        PostEvent(new CDiagsReceiverTuneTimingEvent(TimingAction, ManagedTimings));

        TimingsValid = false;
        TimingAction = TimingAction_Unknown;
        return true;
    }
    return false;
}

void CReceiverDiagnostics::OnClosedCaptionState(bool state, bool isCC708, int service)
{
    PostEvent(new CDtvCCDiags(state, isCC708, (byte) service));
}

uint32 CReceiverDiagnostics::OnRebuffer(SyncReason reason, bool sync, bool cleanStall)
{
    if ((reason == kSyncReason_VideoBufferUnderrun) || (reason == kSyncReason_AudioBufferUnderrun))
    {
        //Total number of times the decoder has underrun/rebuffered, since box reboot
        InterlockedIncrement((LONG*)&CReceiverDiagnostics::TotalNumUnderrun);
    }

    //Send a diagnostics event
    PostEvent(new CDiagsReceiverBufferingEvent(reason, sync, cleanStall));
    //Keep track of number of buffer overruns
    RebufferTimes[reason]++;
    NumRebuffering++;
    return RebufferTimes[reason];
}

void CReceiverDiagnostics::OnRtpDiscontinuity(uint32 numPackets)
{
    Discontinuities++;
    DiscontinuitiesPackets += numPackets;
    InterlockedExchangeAdd((LONG*)&CReceiverDiagnostics::TotalDiscontinuitiesPackets, numPackets);
}

void CReceiverDiagnostics::ResetBufferStatus(uint64 stc)
{
    //Send the last buffer status out before resetting
    if (stc != INVALID_TIME)
    {
        //Update buffer status
        pVideo->LogStc(stc);
        pAudio->LogStc(stc);
        pAudioDescription->LogStc(stc);
        //And send it out
        PostEvent(new CDiagsReceiverPtsDelta(
                            pVideo->BufferStatus.Current,
                            pVideo->BufferStatus.Minimum,
                            pVideo->BufferStatus.Maximum,
                            pVideo->BufferStatus.CurrentHal,
                            pAudio->BufferStatus.Current,
                            pAudio->BufferStatus.Minimum,
                            pAudio->BufferStatus.Maximum,
                            pAudio->BufferStatus.CurrentHal,
                            pAudioDescription->BufferStatus.Current,
                            pAudioDescription->BufferStatus.Minimum,
                            pAudioDescription->BufferStatus.Maximum,
                            pAudioDescription->BufferStatus.CurrentHal,
                            0,
                            stc
                        ));
    }
    //Reset the buffer status now
    pVideo->ResetBufferStatus();
    pAudio->ResetBufferStatus();
    pAudioDescription->ResetBufferStatus();
    AudioVideoOff = 0;
    LastPtsDeltaTicks = Executive_GetTickCount();
}

void CReceiverDiagnostics::LogBuffer(uint64 stc)
{
    bool sendEvent = pVideo->LogStc(stc) || pAudio->LogStc(stc) || pAudioDescription->LogStc(stc);

    //Save the latest stc value
    Stc = stc;

    //Periodic events
    uint32 ticks = Executive_GetTickCount();
    uint32 tickDelta = ticks - LastPtsDeltaTicks;

    //Checks whether audio and video streams are off by a given threshold in either direction.
    int avOff = pVideo->BufferStatus.Current - pAudio->BufferStatus.Current;

    //Send an event only if the buufer status changed or if audio
    //and video buffer status delta changed by given thresholds
    if (sendEvent ||
        tickDelta >= gReceiverConfiguration.PtsDeltaPeriodMS ||
        avOff <= AudioVideoOff - gReceiverConfiguration.AVOffThreshold ||
        avOff >= AudioVideoOff + gReceiverConfiguration.AVOffThreshold)
    {
        PostEvent(new CDiagsReceiverPtsDelta(
                            pVideo->BufferStatus.Current,
                            pVideo->BufferStatus.Minimum,
                            pVideo->BufferStatus.Maximum,
                            pVideo->BufferStatus.CurrentHal,
                            pAudio->BufferStatus.Current,
                            pAudio->BufferStatus.Minimum,
                            pAudio->BufferStatus.Maximum,
                            pAudio->BufferStatus.CurrentHal,
                            pAudioDescription->BufferStatus.Current,
                            pAudioDescription->BufferStatus.Minimum,
                            pAudioDescription->BufferStatus.Maximum,
                            pAudioDescription->BufferStatus.CurrentHal,
                            0,
                            stc
                        ));

        pVideo->BufferStatus.Posted = pVideo->BufferStatus.Current;
        pAudio->BufferStatus.Posted = pAudio->BufferStatus.Current;
        pAudioDescription->BufferStatus.Posted = pAudioDescription->BufferStatus.Current;
        AudioVideoOff = avOff;
        LastPtsDeltaTicks = ticks;
    }
}

bool CReceiverDiagnostics::IsStreamActive(void)
{
    //Whether we are receiving data
    if (BytesReceived != LastBytesReceived)
    {
        LastBytesReceived = BytesReceived;
        return true;
    }

    //Simple algorithm to determine whether we are currently streaming on this renderer
    return pVideo->IsStreamActive() || pAudio->IsStreamActive() || pAudioDescription->IsStreamActive();
}

IDiagsEvent* CReceiverDiagnostics::OnRetrieveGlobal(void)
{
    CReceiverGlobalDiags* diagsEvent = new CReceiverGlobalDiags();
    if (diagsEvent != NULL)
    {
        diagsEvent->TotalNumUnderrun = CReceiverDiagnostics::TotalNumUnderrun;
        diagsEvent->TotalDiscontinuitiesPackets = CReceiverDiagnostics::TotalDiscontinuitiesPackets;
    }
    return diagsEvent;
}

bool CReceiverDiagnostics::Command(const string& command, const vector<string>& args)
{
#ifdef TV2INTERNAL
    if (command == "rendererperf")
    {
        DropPackets = !DropPackets;
        TRACE(("[%08x] Drop packets = %s", mPipeIdN, DropPackets ? "yes" : "no"));
        return true;
    }

    if (command == "discontinuity")
    {
        const string& param1 = args[0];

        if (param1.empty())
            return true;

        int numpackets = atoi(param1.c_str());
        DropNumPackets = numpackets < 0 ? 0 : numpackets;
        TRACE(("[%8x] Will drop %d packets now...", mPipeIdN, DropNumPackets));
        return true;
    }

    if (command == "latency")
    {
        const string& param1 = args[0];

        if (param1.empty())
            return true;

        int latency = atoi(param1.c_str());
        PacketLatency = latency < 0 ? 0 : latency;
        TRACE(("[%08x] Introduce packet latency of %d milliseconds...", mPipeIdN, PacketLatency));
        return true;
    }

    if (command == "corruptpesdata")
     {
        CorruptPES = !CorruptPES;
        return true;
     }

    if (command == "resetall")
    {
        CommandReset();
        //Let it fall through so that other componets can handle this
    }
#endif

    return false;
}

#ifdef TV2INTERNAL
void CReceiverDiagnostics::CommandReset(void)
{
    DropPackets = false;
    DropNumPackets = 0;
    PacketLatency = 0;
    CorruptPES = false;
}

bool CReceiverDiagnostics::ShouldCorruptPES(void)
{
    return CorruptPES;
}
#endif

// ===============================================================================================================
// ===============================================================================================================
