///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// ===============================================================================================================
// Various reasons for rebuffering...
// ===============================================================================================================

enum SyncReason
{
    kSyncReason_None = 0,
    kSyncReason_TimelineDiscontinuity,
    kSyncReason_VideoBufferUnderrun,
    kSyncReason_AudioBufferUnderrun,
    kSyncReason_BufferOverrun,
    kSyncReason_DecoderStall,
    kSyncReason_PmtChanged,
    kSyncReason_SelectComponentVideo,
    kSyncReason_SelectComponentAudio,
    kSyncReason_Max,
};

// ===============================================================================================================
// Active decoder status
// ===============================================================================================================

enum DecoderStatus
{
    kDecoderStatus_Idle = 0,
    kDecoderStatus_Unsupported,
    kDecoderStatus_DecoderError,
    kDecoderStatus_DRMError,
    kDecoderStatus_Failed,
    kDecoderStatus_Acquired,
    kDecoderStatus_Released,
    kDecoderStatus_Processing,
    kDecoderStatus_Dropping,
    kDecoderStatus_Flushed,
    kDecoderStatus_Discontinuity,
    kDecoderStatus_UnBlocked,
    kDecoderStatus_Blocked,
    kDecoderStatus_Max,
};

// ===============================================================================================================
// Tune timing event
// ===============================================================================================================

//Enumerations for handling timings for different actions
//
//***** IMPORTANT *****
//Please make sure that the enumerations defined in managed code
//mimic the following if anything changes here
//
//Also the string representation of the following enums in CReceiverDiags.cpp
//should mimic the following as well
enum TimingAction
{
    TimingAction_Unknown = 0,
    TimingAction_Pause,
    TimingAction_Play,
    TimingAction_Step,
    TimingAction_FastForward,
    TimingAction_Rewind,
    TimingAction_SkipForward,
    TimingAction_SkipBackward,
    TimingAction_Scan,
    TimingAction_SkipPlaylistItem,
    TimingAction_PlayFromBeginning,
    TimingAction_PlayLive,
    TimingAction_PlayAt,
    TimingAction_Stop,
    TimingAction_FrontTruncation,
    TimingAction_TuneToLive,
    TimingAction_TuneToDvb,
    TimingAction_TuneToDvr,
    TimingAction_TuneToVod,
    TimingAction_TuneToWMS,
    TimingAction_TuneToMP3,
    TimingAction_TuneToMP4,
    TimingAction_TuneToTimeShiftLive,
    TimingAction_TuneToTimeShiftRange,
    TimingAction_TuneToMpegTS,
    TimingAction_TuneToTS,
    TimingAction_TuneToSmoothStreaming,
    TimingAction_Max,
};

//Enumerations for handling timing snapsot during tune process in managed media transports
//
//***** IMPORTANT *****
//Please make sure that the enumerations defined in managed code
//mimic the following if anything changes here
//
enum TimingSnapshotAt
{
    //Managed timings
    TimingSnapshotAt_Instantiate = 0,
    TimingSnapshotAt_TuneFromApp,
    TimingSnapshotAt_TuneStep1,
    TimingSnapshotAt_TuneStep2,
    TimingSnapshotAt_TuneStep3,
    TimingSnapshotAt_TuneStep4,
    TimingSnapshotAt_TuneStep5,
    TimingSnapshotAt_TuneStep6,
    TimingSnapshotAt_TuneStep7,
    TimingSnapshotAt_TuneStep8,
    TimingSnapshotAt_TuneStep9,
    TimingSnapshotAt_BeforePreflight,
    TimingSnapshotAt_AfterPreflight,
    TimingSnapshotAt_DetuneTunerSession,
    TimingSnapshotAt_SetupTunerSession,
    TimingSnapshotAt_TuneTunerSession,
    TimingSnapshotAt_MaxManaged,

    //Native timings
    TimingSnapshotAt_Connecting = TimingSnapshotAt_MaxManaged,
    TimingSnapshotAt_Connected,
    TimingSnapshotAt_ReceivedFirstPacket,
    TimingSnapshotAt_ReceivedFirstIFrame,
    TimingSnapshotAt_ClockStarted,
    TimingSnapshotAt_Max,
};

// ===============================================================================================================
// Splice timings...
// ===============================================================================================================

enum SpliceTiming
{
    kSpliceTiming_ReceivedHeadsUp = 0,
    kSpliceTiming_Signalled,
    kSpliceTiming_QuerySecondaryUrl,
    kSpliceTiming_SecondaryUrlReceived,
    kSpliceTiming_DetuneCurrent,
    kSpliceTiming_TuneToNext,
    kSpliceTiming_NextTuned,
    kSpliceTiming_ToConnect,
    kSpliceTiming_ToConnected,
    kSpliceTiming_ResetRenderer,
    kSpliceTiming_RendererReset,
    kSpliceTiming_NextJoined,
    kSpliceTiming_Max,
};

// ===============================================================================================================
// ===============================================================================================================
