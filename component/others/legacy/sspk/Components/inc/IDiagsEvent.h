///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "pkExecutive.h"

// ===============================================================================================================
// Various enums driving the diagnostics engine behavior
// ===============================================================================================================

enum
{
    //Numeric representation for FULLSCREEN pipe id
    kDiagsPipeIdN_FullScreen = 0x46554c4c,

    //Maximum size of the diags event name string
    kDiagsEventMessageLen = 256,
    //Maximum size of the diags data string
    kDiagsEventDataLen = 4096,
    //Maximum size of the binary serialized diags data array
    kDiagsEventBinaryDataLen = 496,
    //Maximum number of filters available
    kDiagsEventBinaryFiltersNum = 400,
};

//The storage mode of the event/counters
//Please be very careful with marking events/counters as Persist. Be sure your
//health model has been reviewed. In the future they will be persisted to flash
//so it should not be done too often as there is risk of reducing the lifetime
//of the flash device
enum LogStorageMode
{
    kLogStorageMode_Persist = 0,
    kLogStorageMode_StoreIfNotConnected = 1,
    kLogStorageMode_DoNotStoreIfNotConnected = 2,
};

//Diagnostic category (normally it should be client subsystems but it must be approved
//during the Health model review of your component as you may want to  group certain
//events in something that is not a "subsystem")
//
//Please only add your new category here after approval of your health model
enum LogCategory
{
    kLogCategory_AV1 = 0,
    kLogCategory_AV2 = 1,
    kLogCategory_DVR = 2,
    kLogCategory_Diagnostics = 3,
};

//Subcategories per category. Each category owner will define hers.
enum LogSubcategory
{
    kLogSubcategory_FIRST = 0,
    kLogSubcategory_LAST = 7,
};

//Priority level
enum LogPriority
{
    kLogPriority_NoLogging = 0,
    kLogPriority_Highest = 1,
    kLogPriority_High = 3,
    kLogPriority_MediumHigh = 5,
    kLogPriority_Medium = 7,
    kLogPriority_MediumLow = 9,
    kLogPriority_Low = 11,
    kLogPriority_Lowest = 15,
};

//Diagnostic Logging modes
enum LogMode
{
    kLogMode_Debug = 0,
    kLogMode_Information = 1,
    kLogMode_Warning = 2,
    kLogMode_Error = 3,
    kLogMode_Perf = 4,
};


// ===============================================================================================================
// Defines the framework for different types of enums used for signaling different types of diagnostics events
// originated from diffent AV System component.  The following allows us to support 256 (higher order byte)
// different system components and 256 events per system component (low order byte) which should be sufficient
// for any needs that I see across Av pipeline
// ===============================================================================================================

enum kDiagsEvent
{
    //Receiver events
    kDiagsEvent_Receiver = 0x0000,
    kDiagsEvent_Receiver_Update = kDiagsEvent_Receiver,
    kDiagsEvent_Receiver_TunerError,
    kDiagsEvent_Receiver_Buffering,
    kDiagsEvent_Receiver_OTT_Discontinuity,
    kDiagsEvent_Receiver_Discontinuity,
    kDiagsEvent_Receiver_FoundRap,
    kDiagsEvent_Receiver_MissingRtpPayload,
    kDiagsEvent_Receiver_ProgramSelected,
    kDiagsEvent_Receiver_AudioLanguage,
    kDiagsEvent_Receiver_AudioDescriptionLanguage,
    kDiagsEvent_Receiver_SubtitleLanguage,
    kDiagsEvent_Receiver_EcmParseError,
    kDiagsEvent_Receiver_EcmError,
    kDiagsEvent_Receiver_VideoDecoder,
    kDiagsEvent_Receiver_AudioDecoder,
    kDiagsEvent_Receiver_AudioDescriptionDecoder,
    kDiagsEvent_Receiver_AudioDescriptionVolume,
    kDiagsEvent_Receiver_Streaming,
    kDiagsEvent_Receiver_SetStreamBufferDelay,
    kDiagsEvent_Receiver_AccessControlBadTimeBoundary,
    kDiagsEvent_Receiver_PtsDelta,
    kDiagsEvent_Receiver_TuneTiming,
    kDiagsEvent_Receiver_Tune,
    kDiagsEvent_Receiver_Pause,
    kDiagsEvent_Receiver_Play,
    kDiagsEvent_Receiver_NearEnd,
    kDiagsEvent_Receiver_CC,
    kDiagsEvent_Receiver_SpliceTiming,
    kDiagsEvent_Receiver_SignalLost,
    kDiagsEvent_Receiver_BitRate,
    //DVR Receiver events
    kDiagsEvent_Receiver_DvrUpdate,
    //Decoder events
    kDiagsEvent_Decoder_VideoParameters,
    kDiagsEvent_Decoder_AudioParameters,
    //Hal Decoder events
    kDiagsEvent_Receiver_SDOnlyTune,
    kDiagsEvent_Receiver_KOKTune,
    //Global receiver diags
    kDiagsEvent_Receiver_StaticDiags,
    //OPL
    kDiagsEvent_Receiver_Macrovision_DRM,
    kDiagsEvent_Receiver_Macrovision_ECM,
    kDiagsEvent_Receiver_Macrovision_WSS,
    kDiagsEvent_Receiver_CGMSA_DRM,
    kDiagsEvent_Receiver_CGMSA_ECM,
    kDiagsEvent_Receiver_CGMSA_WSS,
    //Max receiver events
    kDiagsEvent_Receiver_Max,

    //Base socket events
    kDiagsEvent_Socket = 0x0100,
    kDiagsEvent_Socket_Opened = kDiagsEvent_Socket,
    kDiagsEvent_Socket_Closed,
    kDiagsEvent_Socket_Update,
    kDiagsEvent_Socket_Blackout,
    kDiagsEvent_Socket_StaticDiags,
    kDiagsEvent_Socket_Max,

    //Live socket events
    kDiagsEvent_SocketLive = 0x0200,
    kDiagsEvent_SocketLive_Update = kDiagsEvent_SocketLive,
    kDiagsEvent_SocketLive_OverflowReset,
    kDiagsEvent_SocketLive_SequenceDiscontinuity,
    kDiagsEvent_SocketLive_Hole,
    kDiagsEvent_SocketLive_HoleTooLarge,
    kDiagsEvent_SocketLive_HoleDuringBurst,
    kDiagsEvent_SocketLive_HoleWithoutSession,
    kDiagsEvent_SocketLive_HoleExpired,
    kDiagsEvent_SocketLive_FirstSeqNo,
    kDiagsEvent_SocketLive_Ready,
    kDiagsEvent_SocketLive_Join,
    kDiagsEvent_SocketLive_JoinResponse,
    kDiagsEvent_SocketLive_BurstComplete,
    kDiagsEvent_SocketLive_BurstCompleteAck,
    kDiagsEvent_SocketLive_Leave,
    kDiagsEvent_SocketLive_MulticastJoin,
    kDiagsEvent_SocketLive_MulticastLeave,
    kDiagsEvent_SocketLive_Detune,
    kDiagsEvent_SocketLive_FirstPacket,
    kDiagsEvent_SocketLive_FirstMulticastPacket,
    kDiagsEvent_SocketLive_SrcUnavailable,
    kDiagsEvent_SocketLive_CCSend,
    kDiagsEvent_SocketLive_CCError,
    kDiagsEvent_SocketLive_TunerError,
    kDiagsEvent_SocketLive_TunerError_SessionIdMismatch,
    kDiagsEvent_SocketLive_TunerError_ServiceIdMismatch,
    kDiagsEvent_SocketLive_TunerError_Timeout,
    kDiagsEvent_SocketLive_EdgeMap,
    kDiagsEvent_SocketLive_Max,

    //Dvr Socket events
    kDiagsEvent_SocketDvr = 0x0300,
    kDiagsEvent_SocketDvr_Update = kDiagsEvent_SocketDvr,
    kDiagsEvent_SocketDvr_Max,

    //Socket Set
    kDiagsEvent_SocketSet = 0x0400,
    kDiagsEvent_SocketSet_AddSocket = kDiagsEvent_SocketSet,
    kDiagsEvent_SocketSet_AddSocketFail,
    kDiagsEvent_SocketSet_CloseSocket,
    kDiagsEvent_SocketSet_Max,

    //HTTP Socket events
    kDiagsEvent_SocketHttp = 0x0500,
    kDiagsEvent_SocketHttp_Max,

    //VOD Socket events
    kDiagsEvent_SocketVod = 0x0600,
    kDiagsEvent_SocketVod_Max,

    //DVB socket events
    kDiagsEvent_SocketDvb = 0x0700,
    kDiagsEvent_SocketDvb_RtpPacketizerRestart = kDiagsEvent_SocketDvb,
    kDiagsEvent_SocketDvb_Max,

    //Flex socket events
    kDiagsEvent_SocketFlex = 0x0800,
    kDiagsEvent_SocketFlex_Max,

    //Bypass socket events
    kDiagsEvent_SocketSbp = 0x0900,
    kDiagsEvent_SocketSbp_Max,

    //UDP socket events
    kDiagsEvent_SocketUdp = 0x0A00,
    kDiagsEvent_SocketUdp_Max,

    //Multicast socket events
    kDiagsEvent_SocketMulticast = 0x0B00,
    kDiagsEvent_SocketMulticast_Max,

    //File socket events
    kDiagsEvent_SocketFile = 0x0C00,
    kDiagsEvent_SocketFile_Max,

    //WHDVR streamer events
    kDiagsEvent_RemoteSocket = 0x0D00,
    kDiagsEvent_RemoteSocket_Streaming = kDiagsEvent_RemoteSocket,
    kDiagsEvent_RemoteSocket_StreamingDone,
    kDiagsEvent_RemoteSocket_StreamingFailed,
    kDiagsEvent_RemoteSocket_Max,

    //WMS events
    kDiagsEvent_WMS = 0x0E00,
    kDiagsEvent_WMS_PlayingItem = kDiagsEvent_WMS,
    kDiagsEvent_WMS_Max,

    //Splicing events
    kDiagsEvent_Splice = 0x0F00,
    kDiagsEvent_Splice_HeadsUp = kDiagsEvent_Splice,
    kDiagsEvent_Splice_Now,
    kDiagsEvent_Splice_Error,
    kDiagsEvent_Splice_Max,

    //Smooth Streaming events
    kDiagsEvent_SS = 0x1000,
    kDiagsEvent_SS_Update = kDiagsEvent_SS,
    kDiagsEvent_SS_QualityChange,
    kDiagsEvent_SS_StreamInfo,
    kDiagsEvent_SS_SocketInfo,
    kDiagsEvent_SS_ChunkInfo,

    //Playready related events
    kDiagsEvent_PlayReady = 0x2000,
    kDiagsEvent_PlayReady_OPL = kDiagsEvent_PlayReady,

    //Heuristic related events
    kDiagsEvent_CreateChunkDownloader = 0x3000,
    kDiagsEvent_DestroyChunkDownloader,
    kDiagsEvent_StartChunkRequest,
    kDiagsEvent_StartChunkResponseHeader,
    kDiagsEvent_StartChunkResponseBody,
    kDiagsEvent_EndChunkRequest,
    kDiagsEvent_UpdateHeuristics,
    kDiagsEvent_RateMeasurement,
    kDiagsEvent_AddVideoSample,
    kDiagsEvent_AddAudioSample,
    kDiagsEvent_LimitVideoFifo,

    //Manifest related events
    kDiagsEvent_StreamInManifest = 0x4000,

    //Chunklist related events
    kDiagsEvent_AddChunk = 0x5000,

    //FragInfo related events
    kDiagsEvent_StartFragInfoRequest = 0x6000,

    //All other miscllaneous events
    kDiagsEvent_Misc = 0xFF00,
    kDiagsEvent_CpuUsage = kDiagsEvent_Misc,
    kDiagsEvent_UdpAuthNoActiveKey,
    kDiagsEvent_UdpAuthProtectionFailure,
    kDiagsEvent_Misc_Max,

    //Undefined event type - should not happen
    kDiagsEvent_Unknown = 0xFFFF,
};

// ===============================================================================================================
// Diagnostic events
// ===============================================================================================================

/// <summary>
/// Represents diagnostic events that can be posted to the diagnostic manager
/// </summary>
class IDiagsEvent
{
public:
    /// <summary>
    /// IDiagsEvent
    /// </summary>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    IDiagsEvent() { TimeStamp = Executive_GetTickCount(); NextEvent = NULL; DataPtr = DataEnd = NULL; }
    /// <summary>
    /// ~IDiagsEvent
    /// </summary>
    /// <return>
    /// <para>Returns virtual type</para>
    /// </return>
    virtual ~IDiagsEvent() {}

public:
    /// <summary>
    /// Helper APIs to set the buffer where the event data string would be built
    /// Make sure the buffer is set before any of the follwing APIs are called
    /// </summary>
    /// <param name="diagsData">[IN] wchar representing data to log</param>
    /// <param name="diagsData">[IN] length of wchar buffer</param>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    void DiagsSetBuffer(__out_ecount(diagsDataLen) WCHAR* diagsData, int diagsDataLen) { DataPtr = diagsData; DataEnd = diagsData + diagsDataLen; }
    /// <summary>
    /// Build common parameters for events in the output string
    /// </summary>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    void DiagsPreamble(void) { DiagsLogValue(L"Timestamp", TimeStamp); }
    /// <summary>
    /// Called to finalize the event once the data event string has been built
    /// </summary>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    void DiagsFinalize(void) { *DataPtr = 0; }
protected:
    /// <summary>
    /// Helper APIs to process the event data
    /// </summary>
    /// <param name="diagsData">[IN] wchar representing data to log</param>
    /// <param name="diagsLabel">[IN] label to associate with data</param>
    /// <param name="diagsValue">[IN] integer value to associate with data</param>
    /// <param name="isHex">[IN] whether the data should be printed in Hex format</param>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    void DiagsLogValue(const WCHAR* diagsLabel, int diagsValue, bool isHex = false);
    /// <summary>
    /// Helper APIs to process the event data
    /// </summary>
    /// <param name="diagsData">[IN] wchar representing data to log</param>
    /// <param name="diagsLabel">[IN] label to associate with data</param>
    /// <param name="diagsValue">[IN] integer 64 byte value to associate with data</param>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    void DiagsLogValue64(const WCHAR* diagsLabel, int64 diagsValue);
    /// <summary>
    /// Helper APIs to process the event data
    /// </summary>
    /// <param name="diagsData">[IN] wchar representing data to log</param>
    /// <param name="diagsLabel">[IN] label to associate with data</param>
    /// <param name="guid">[IN] GUID to associate with data</param>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    void DiagsLogGuid(const WCHAR* diagsLabel, const GUID& guid);
    /// <summary>
    /// Helper APIs to process the event data
    /// </summary>
    /// <param name="diagsData">[IN] wchar representing data to log</param>
    /// <param name="diagsLabel">[IN] label to associate with data</param>
    /// <param name="diagsString">[IN] string to associate with data</param>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    void DiagsLogString(const WCHAR* diagsLabel, const char* diagsString);
    /// <summary>
    /// Helper APIs to process the event data
    /// </summary>
    /// <param name="diagsData">[IN] wchar representing data to log</param>
    /// <param name="diagsLabel">[IN] label to associate with data</param>
    /// <param name="diagsWString">[IN] unicode string to associate with data</param>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    void DiagsLogWString(const WCHAR* diagsLabel,  const WCHAR* diagsWString);
    /// <summary>
    /// Helper APIs to process the event data
    /// </summary>
    /// <param name="diagsData">[IN] wchar representing data to log</param>
    /// <param name="diagsLabel">[IN] label to associate with data</param>
    /// <param name="guid">[IN] GUID to associate with data</param>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    void DiagsLogPipeId(const WCHAR* diagsLabel, uint32 pipeIdN);
private:
    /// <summary>
    /// Helper field to build the events as wide character strings
    /// as expected by the trace log framework...
    ///
    /// Note that the following fields are used while building the
    /// events into the format expected by the logging framework and
    /// those methods are called only from the singleton diags manager
    /// thread.  So it will be OK to have them as static fields
    /// </summary>
    WCHAR* DataPtr;
    WCHAR* DataEnd;

protected:
    //Pack bolean
    static void DiagsPackBool(byte*& data, bool diagsValue)
    {
        *data++ = (byte)diagsValue;
    }
    //Unpack boolean
    static void DiagsUnpackBool(byte*& data, bool& diagsValue)
    {
        diagsValue = *data++ != 0;
    }
    //Pack 8 bit data
    static void DiagsPackByte(byte*& data, byte diagsValue)
    {
        *data++ = diagsValue;
    }
    //Unpack 8 bit data
    static void DiagsUnpackByte(byte*& data, byte& diagsValue)
    {
        diagsValue = *data++;
    }
    //Pack 8 bit data
    static void DiagsPackChars(byte*& data, const char* diags_array, int n)
    {
        const char* end = diags_array + n;
        while (diags_array < end)
        {
            *data++ = (byte)*diags_array++;
        }
    }
    //Unpack 8 bit data
    static void DiagsUnpackChars(byte*& data, __out_ecount(n)char* diags_array, int n)
    {
        const char* end = diags_array + n;
        while (diags_array < end)
        {
            *diags_array++ = (char)*data++;
        }
    }
    //Pack 16 bit chars
    static void DiagsPackWChars(byte*& data, const wchar_t* diags_array, int n)
    {
        const wchar_t* end = diags_array + n;
        while (diags_array < end)
        {
            *data++ = (byte)(*diags_array >> 8);
            *data++ = (byte)(*diags_array);

            ++diags_array;
        }
    }
    //Unpack 16 bit chars
    static void DiagsUnpackWChars(byte*& data, __out_ecount(n)wchar_t* diags_array, int n)
    {
        const wchar_t* end = diags_array + n;
        while (diags_array < end)
        {
            *diags_array  = (wchar_t)(*data++) << 8;
            *diags_array |= (wchar_t)(*data++);

            ++diags_array;
        }
    }
    //Pack 16 bit data
    static void DiagsPackUint16(byte*& data, uint16 diagsValue)
    {
        *data++ = (byte)(diagsValue >>  8);
        *data++ = (byte)(diagsValue      );
    }
    //Unpack 16 bit data
    static void DiagsUnpackUint16(byte*& data, uint16& diagsValue)
    {
        diagsValue  = (uint16)(*data++) << 8;
        diagsValue |= (uint16)(*data++);
    }
    //Pack 32 bit signed data
    static void DiagsPackInt32(byte*& data, int diagsValue)
    {
        *data++ = (byte)(diagsValue >> 24);
        *data++ = (byte)(diagsValue >> 16);
        *data++ = (byte)(diagsValue >>  8);
        *data++ = (byte)(diagsValue      );
    }
    //Unpack 32 bit signed data
    static void DiagsUnpackInt32(byte*& data, int& diagsValue)
    {
        diagsValue  = (uint32)(*data++) << 24;
        diagsValue |= (uint32)(*data++) << 16;
        diagsValue |= (uint32)(*data++) <<  8;
        diagsValue |= (uint32)(*data++);
    }
    //Pack 32 bit unsigned data
    static void DiagsPackUint32(byte*& data, uint32 diagsValue)
    {
        *data++ = (byte)(diagsValue >> 24);
        *data++ = (byte)(diagsValue >> 16);
        *data++ = (byte)(diagsValue >>  8);
        *data++ = (byte)(diagsValue      );
    }
    //Unpack 32 bit unsigned data
    template <class T>
    static void DiagsUnpackUint32(byte*& data, T& diagsValue)
    {
        
        uint32 tempValue  = (uint32)(*data++) << 24;
        tempValue |= (uint32)(*data++) << 16;
        tempValue |= (uint32)(*data++) <<  8;
        tempValue |= (uint32)(*data++);

        diagsValue = (T)tempValue;
    }
    //Pack 64 bit unsigned data
    static void DiagsPackUint64(byte*& data, uint64 diagsValue)
    {
        *data++ = (byte)(diagsValue >> 56);
        *data++ = (byte)(diagsValue >> 48);
        *data++ = (byte)(diagsValue >> 40);
        *data++ = (byte)(diagsValue >> 32);
        *data++ = (byte)(diagsValue >> 24);
        *data++ = (byte)(diagsValue >> 16);
        *data++ = (byte)(diagsValue >>  8);
        *data++ = (byte)(diagsValue      );
    }
    //Unpack 64 bit unsigned data
    static void DiagsUnpackUint64(byte*& data, uint64& diagsValue)
    {
        diagsValue  = (uint64)(*data++) << 56;
        diagsValue |= (uint64)(*data++) << 48;
        diagsValue |= (uint64)(*data++) << 40;
        diagsValue |= (uint64)(*data++) << 32;
        diagsValue |= (uint64)(*data++) << 24;
        diagsValue |= (uint64)(*data++) << 16;
        diagsValue |= (uint64)(*data++) <<  8;
        diagsValue |= (uint64)(*data++);
    }
    //Pack GUID
    static void DiagsPackGuid(byte*& data, const GUID& diagsValue)
    {
        *data++ = (byte)(diagsValue.Data1 >> 24);
        *data++ = (byte)(diagsValue.Data1 >> 16);
        *data++ = (byte)(diagsValue.Data1 >>  8);
        *data++ = (byte)(diagsValue.Data1      );

        *data++ = (byte)(diagsValue.Data2 >>  8);
        *data++ = (byte)(diagsValue.Data2      );

        *data++ = (byte)(diagsValue.Data3 >>  8);
        *data++ = (byte)(diagsValue.Data3      );

        *data++ = (byte)(diagsValue.Data4[0]   );
        *data++ = (byte)(diagsValue.Data4[1]   );
        *data++ = (byte)(diagsValue.Data4[2]   );
        *data++ = (byte)(diagsValue.Data4[3]   );
        *data++ = (byte)(diagsValue.Data4[4]   );
        *data++ = (byte)(diagsValue.Data4[5]   );
        *data++ = (byte)(diagsValue.Data4[6]   );
        *data++ = (byte)(diagsValue.Data4[7]   );
    }
    //Unpack GUID
    static void DiagsUnpackGuid(byte*& data, GUID& diagsValue)
    {
        diagsValue.Data1    = (uint32)(*data++) << 24;
        diagsValue.Data1   |= (uint32)(*data++) << 16;
        diagsValue.Data1   |= (uint32)(*data++) << 8;
        diagsValue.Data1   |= (uint32)(*data++);

        diagsValue.Data2    = (uint16)(*data++) << 8;
        diagsValue.Data2   |= (uint16)(*data++);

        diagsValue.Data3    = (uint16)(*data++) << 8;
        diagsValue.Data3   |= (uint16)(*data++);

        diagsValue.Data4[0] = *data++;
        diagsValue.Data4[1] = *data++;
        diagsValue.Data4[2] = *data++;
        diagsValue.Data4[3] = *data++;
        diagsValue.Data4[4] = *data++;
        diagsValue.Data4[5] = *data++;
        diagsValue.Data4[6] = *data++;
        diagsValue.Data4[7] = *data++;
    }

public:
    /// <summary>
    /// Called by diagnostics manager to prepare event data string
    /// </summary>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    virtual void DiagsGetEventData(void) = 0;
    /// <summary>
    /// Called by diagnostics manager to retrieve the event message string
    /// </summary>
    /// <return>
    /// <para>Returns WCHAR* type</para>
    /// </return>
    virtual const WCHAR* DiagsGetEventMessage(void) = 0;
    /// <summary>
    /// Called by diags manager to extract the event type
    /// </summary>
    /// <return>
    /// <para>Returns event type</para>
    /// </return>
    virtual kDiagsEvent DiagsGetEventType(void) = 0;
    /// <summary>
    /// Called by diagnostics manager to retrieve the event type (whether it is error or information type).
    /// </summary>
    /// <return>
    /// <para>Returns bool type</para>
    /// </return>
    virtual bool DiagsIsError(void) = 0;

    /// <summary>
    /// Called by diagnostics manager to retrieve the storage mode of the event
    /// </summary>
    /// <return>
    /// <para>Returns storage mode</para>
    /// </return>
    virtual LogStorageMode DiagsLogStorageMode(void)
    {
        return kLogStorageMode_StoreIfNotConnected;
    }
    /// <summary>
    /// Called by diagnostics manager to retrieve the category of the event
    /// </summary>
    /// <return>
    /// <para>Returns category to which this event belongs</para>
    /// </return>
    virtual LogCategory DiagsLogCategory(void)
    {
        return kLogCategory_AV1;
    }
    /// <summary>
    /// Called by diagnostics manager to retrieve the sub-category of the event
    /// </summary>
    /// <return>
    /// <para>Returns sub-category to which this event belongs</para>
    /// </return>
    virtual LogSubcategory DiagsLogSubcategory(void)
    {
        return kLogSubcategory_FIRST;
    }
    /// <summary>
    /// Called by diagnostics manager to retrieve the priority of the event
    /// </summary>
    /// <return>
    /// <para>Returns diags priority</para>
    /// </return>
    virtual LogPriority DiagsLogPriority(void)
    {
        return kLogPriority_Highest;
    }
    /// <summary>
    /// Called by diagnostics manager to retrieve the logging mode of the event
    /// </summary>
    /// <return>
    /// <para>Returns loging mode</para>
    /// </return>
    virtual LogMode DiagsLogMode(void)
    {
        return kLogMode_Information;
    }
    /// <summary>
    /// Called by diagnostics manager to get the version of the event
    /// </summary>
    /// <return>
    /// <para>Returns version type</para>
    /// </return>
    virtual byte DiagsEventVersion(void)
    {
        return 1;
    }
    /// <summary>
    /// Called by diagnostics manager to serialize event data in binary format
    /// </summary>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    virtual uint16 DiagsSerializeEventDataLength(byte version)
    {
        return 4;
    }
    /// <summary>
    /// Called by diagnostics manager to serialize event data in binary format
    /// </summary>
    /// <return>
    /// <para>Returns void type</para>
    /// </return>
    virtual void DiagsSerializeEventData(byte*& data)
    {
        //Prepare the event header
        //
        byte version = DiagsEventVersion();
        //Type of event - 2 bytes
        DiagsPackUint16(data, (uint16)DiagsGetEventType());
        //Version information - 1 bytes
        DiagsPackByte(data, version);
        //Reserved 1 byte
        DiagsPackByte(data, 0);
        //Length - 2 bytes
        DiagsPackUint16(data, (uint16)DiagsSerializeEventDataLength(version));
        //Paking 2 bytes (place holder)
        DiagsPackUint16(data, 0);

        //Build out rest of the event
        //
        //Time stamp - 4 bytes
        DiagsPackUint32(data, TimeStamp);
    }
    /// <summary>
    /// Called by the parser tool to deserialize event data from binary format into the fields
    /// </summary>
    /// <return>
    /// <para>Returns true if there is enough data to deserialize the event</para>
    /// </return>
    virtual bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        DiagsUnpackUint32(data, TimeStamp);
        return true;
    }

public:
    /// <summary>
    /// Timestamp at which the event was generated
    /// </summary>
    uint32       TimeStamp;
    IDiagsEvent* NextEvent;
};

// ===============================================================================================================
// ===============================================================================================================
