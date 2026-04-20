///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DrmDefinitions.h"
#include "OttStatistics.h"
#include "IXDrm.h"

// ===============================================================================================================
// Various notifications from sockets to receivers
// ===============================================================================================================

enum ReceiverNotificationType
{
    kReceiverNotificationType_Unknown,

    kReceiverNotificationType_Stopped,
    kReceiverNotificationType_DataReady,
    kReceiverNotificationType_PlayingItem,
    kReceiverNotificationType_Paused,
    kReceiverNotificationType_Detuned,
    kReceiverNotificationType_DecodersPurged,

    kReceiverNotificationType_Reset,
    kReceiverNotificationType_EndOfStream,
    kReceiverNotificationType_Error,

    kReceiverNotificationType_CanRetry,
    kReceiverNotificationType_StreamBuffer,
    kReceiverNotificationType_StreamBufferDefault,
    kReceiverNotificationType_ClockMode,
    kReceiverNotificationType_BitRate,
    kReceiverNotificationType_CurrentBitRate,
    kReceiverNotificationType_AudioLanguages,
    kReceiverNotificationType_StreamDescUpdate,
    kReceiverNotificationType_SmoothStreamingManifest,
    kReceiverNotificationType_DrmOutputProtectionLevel,
    kReceiverNotificationType_MediaTransportEvent,

    kReceiverNotificationType_OnConnected,
    kReceiverNotificationType_OnFirstPacket,
    kReceiverNotificationType_Discontinuity,
    kReceiverNotificationType_StateChangeEvent,
    kReceiverNotificationType_ThreadTimes,
    kReceiverNotificationType_DrmLicenseTimes,

    kReceiverNotificationType_SpliceSignalLater,
    kReceiverNotificationType_SpliceSignal,
    kReceiverNotificationType_SpliceSignalOTT,
};

// ===============================================================================================================
// Extracting the WMS client state
// ===============================================================================================================

class CReceiverParams
{
public:
    CReceiverParams()
        : CurrentTime(0)
        , StartTime(0)
        , EndTime(0)
        , Speed(0)
        , PlayListIndex(0)
        , PlayListCount(0)
    {
    }

    uint64 CurrentTime;
    uint64 StartTime;
    uint64 EndTime;
    int32  Speed;
    int32  PlayListIndex;
    int32  PlayListCount;
};

// ===============================================================================================================
// SCTE 35 signaling parsed from the special header extension for now
// ===============================================================================================================

struct CSpliceMessage
{
public:
    uint32 AvailId;
    uint32 TimeStamp;
    uint64 PCR;
    uint64 NTP;
    uint64 Pts;
    uint64 Duration;
    uint16 Flags;
};

// ===============================================================================================================
// Availability of streams (smooth streaming manifest for example)
// ===============================================================================================================

class CStreamInfoList;

// ===============================================================================================================
// Data associated with various notifications
// ===============================================================================================================

#define WMS_MAX_LANGUAGE_COUNT 4

struct CReceiverNotificationData
{
public:
    CReceiverNotificationData()
    {
        memset(&PlayParams, 0, sizeof(PlayParams));
    }

public:
    CReceiverParams PlayParams;
    union
    {
        struct
        {
            bool    IsVideo;
        } DataReady;

        struct
        {
            WCHAR*  SourceUrl;
        } PlayingItem;

        struct
        {
            bool    IsIFrameOnlyMode;
            int32   Speed;
            uint64  Offset;
        } Reset;

        struct
        {
            uint32  Result;
            bool    Direction;
        } EndOfStream;

        struct
        {
            bool    CanRetry;
            bool    ResetOnStall;
        } Retry;

        struct
        {
            int32   Delay;
        } StreamBuffer;

        struct
        {
            int32   Delay;
        } StreamBufferDefault;

        struct
        {
            uint32  Mode;
        } Clock;

        struct
        {
            uint32  BPS;
        } BitRate;

        struct
        {
            uint32  LanguageCount;
            uint32  LCID[WMS_MAX_LANGUAGE_COUNT];
        } AudioLanguages;

        struct
        {
            CStreamInfoList* StreamInfo;
            //CReceiver will fill out the following three fields
            //in response to the callback from socket.
            //See bug# 226428
            uint32 AudioPID;
            uint32 SubtitlePID;
            bool   InitialSelection;
        } StreamDescUpdate;

        struct
        {
            const byte* Manifest;
            uint32 ManifestLength;
        } SmoothStreamingManifest;

        XDRM_OPL_DATA DrmOutputProtectionLevel;

        struct
        {
            uint32  ConnectingTime;
            uint32  ConnectedTime;
        } ConnectTimes;

        struct
        {
            uint32  Tick;
        } PacketTime;

        COttDiscontinuityStats* DiscontinuityStats;

        //State change event
        struct
        {
            int     Action;
            int     Speed;
        } StateChange;

        struct
        {
            uint32  DiagsCpuUsageLast;
            uint32  DiagsCpuUsageAverage;
            uint32  DiagsCpuUsageMaximum;
            uint32  DiagsCpuUsageTotalUsed;
        } ThreadTimes;

        struct
        {
            DrmLicenseTiming Timing; // tick counts at different stages of DRM license acquisition
        } DrmLicense;

        struct
        {
            const char* EventStringPtr;
        } MediaTransportEvent;

        //SCTE 35 signaling parsed from the special header extension for now
        CSpliceMessage* SpliceMessage;
    };
};

// ===============================================================================================================
// ===============================================================================================================
