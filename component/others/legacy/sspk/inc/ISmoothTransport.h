///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// <summary>
/// ISmoothTransport.h
///
/// Notes:
/// - According to SSME MSDN article, "stream" refers to video / audio / binary level contained by StreamIndex tag in manifest
/// while "track" refers to different levels available for stream (i.e. QualityLevel / camera angle)
/// </summary>

#include "IManifest.h"
#include "SSPKTimeSpan.h"

#include <string>
#include <vector>

namespace SSPK
{
enum ESmoothTransportState
{
    SmoothTransportTunerState_Unknown = 0,
    SmoothTransportTunerState_Tuning,             // Tuning
    SmoothTransportTunerState_Playing,            // Playback started
    SmoothTransportTunerState_Paused,             // Playback paused
    SmoothTransportTunerState_MediaEnded,         // Presentation has completed
    SmoothTransportTunerState_Detuned,            // Detuned due to an error
    SmoothTransportTunerState_Closed,             // Transport is closed, open is required
    SmoothTransportTunerState_Max
};

enum ESmoothTransportStatusUpdate
{
    SmoothTransportStatus_Unknown = 0,
    SmoothTransportStatus_Heartbeat,
    SmoothTransportStatus_TunerStateChanged,      // Tuner State changed
    SmoothTransportStatus_Streaming,              // Data started to come in from the socket
    SmoothTransportStatus_Rendering,              // Clock started
    SmoothTransportStatus_Underrun,               // There was a underrun event
    SmoothTransportStatus_Rebuffer,               // There was a rebuffering event
    SmoothTransportStatus_StartEndTime,           // The start/end time changed
    SmoothTransportStatus_DrmStateChanged,        // Drm status changed
    SmoothTransportStatus_BitrateChanged,         // Bit rate changed
    SmoothTransportStatus_DecoderError,           // Decoder error
    SmoothTransportStatus_ChunkConnectHttpInvalid,// http <200 or >=400 reponse on connect
    SmoothTransportStatus_NextChunkHttpInvalid,   // http <200 or >=400 reponse on next chunk
    SmoothTransportStatus_ChunkHdrHttpInvalid,    // http <200 or >=400 reponse on chunk hdr
    SmoothTransportStatus_ChunkHdrError,          // chunk header parser error
    SmoothTransportStatus_AtWindowEdge,           // Playback position is at or past DVR window edge
    SmoothTransportStatus_EndOfLive,              // Live presentation is no longer live
    SmoothTransportStatus_OutsideWindowEdge,      // Download position is outside of DVR window
    SmoothTransportStatus_SegmentManifestError,   // Error while getting a segment manifest
    SmoothTransportStatus_DrmInitError,           // Error while initializing DRM
    SmoothTransportStatus_Max
};

enum ESmoothTransportProtocol
{
    SmoothTransportProtocol_Mbr = 0                // Currently only Multi-Bitrate is supported
};

// Note: Do not delete or change any of the existing ESmoothTransportError
// values.  To add a new value, add it to the end of the appropriate group
// This is to keep the same values for backwards compatibility
enum ESmoothTransportError
{
    SmoothTransportError_None                           = 0,

    // Generic errors
    SmoothTransportError_Unknown                        = 100,

    // Tuner errors
    SmoothTransportError_TunerAllocationFailure         = 200,
    SmoothTransportError_TunerSharedReceivers,

    // Manifest errors
    SmoothTransportError_ManifestParseFailed            = 300,
    SmoothTransportError_ManifestVersionUnsupported,
    SmoothTransportError_ManifestInvalid,
    SmoothTransportError_ManifestHttpInvalidResult,

    // Socket errors
    SmoothTransportError_SocketAlreadyClosed            = 400,
    SmoothTransportError_SocketReadError,
    SmoothTransportError_SocketOpenFailed,
    SmoothTransportError_SocketConnectFailed,
    SmoothTransportError_SocketSendFailed,
    SmoothTransportError_SocketRecvFailed,

    //HTTP errors
    SmoothTransportError_HttpParseResponseFailed        = 500,
    SmoothTransportError_HttpInvalidResult,
    SmoothTransportError_HttpTooManyRedirect,
    SmoothTransportError_HttpRedirectFailed,
    SmoothTransportError_HttpRedirectNotAllowed,
    SmoothTransportError_HttpCreateFailed,

    // Chunk errors
    SmoothTransportError_ChunkConnectHttpInvalidResult  = 600,
    SmoothTransportError_ChunkNextHttpInvalidResult,
    SmoothTransportError_ChunkHdrParseFailed,
    SmoothTransportError_ChunkInvalidData,

    //Drm error
    SmoothTransportError_DrmInitFailed                  = 700,
};

struct SmoothTransportStatus
{
    ESmoothTransportState        _currentState;    // Current status
    TimeSpan_NTP                 _currentTime;     // Current displayed media time in playback stream
    TimeSpan_NTP                 _startTime;       // Start displayable media time of playback stream
    TimeSpan_NTP                 _endTime;         // End displayable media time of playback stream
    float                        _speed;           // Current playback rate
    ESmoothTransportStatusUpdate _update;          // Reason for this update
    std::string                  _additionalInfo;  // Any additional info that may go with the reason
    bool                         _clockStarted;    // Is clock started?
    pkRESULT                     _pkResult;        // pkRESULT code, if any
    int                          _httpResponse;    // http response code, if any
};

struct SmoothTransportError
{
    ESmoothTransportError       _errorCode;       // Overall error code
    pkRESULT                    _pkResult;        // pkRESULT code, if any
    int                         _httpResponse;    // http response code, if any
    std::string                 _message;         // Error message
};

/// <summary>
/// Callback interface to notify the application of transport events.
/// <summary>
class ISmoothTransportStatusSink
{
public:
    virtual void StatusCallback(SmoothTransportStatus& status) = 0;

protected:
    virtual ~ISmoothTransportStatusSink() {}
};

/// <summary>
/// Callback interface to notify the application when asynchronous
/// and out-of-band errors occurred.
/// <summary>
class ISmoothTransportErrorSink
{
public:
    virtual void ErrorCallback(SmoothTransportError& error) = 0;

protected:
    virtual ~ISmoothTransportErrorSink() {}
};

/// <summary>
/// Callback interface to notify the application that a manifest is
/// ready to play.
/// <summary>
class IManifestReadyCallback
{
public:
    /// <summary>
    /// The actual callback for the manifest ready event
    /// </summary>
    /// <param name="pManifest">The parsed manifest</param>
    /// <param name="result">The pkRESULT for getting and parsing the manifest</param>
    virtual void ManifestReadyCallback(_In_ IManifest* pManifest, pkRESULT result) = 0;

protected:
    virtual ~IManifestReadyCallback() {};
};

/// <summary>
/// Callback interface to notify the application the result of SetPlaybackRangeAsync
/// <summary>
class ISetPlaybackRangeCallback
{
public:
    /// <summary>
    /// The actual callback for the set playback range request
    /// </summary>
    /// <param name="pManifest">The updated manifest</param>
    /// <param name="result">The pkRESULT of setting the new playback range</param>
    virtual void SetPlaybackRangeCallback(_In_ IManifest* pManifest, pkRESULT result) = 0;

protected:
    virtual ~ISetPlaybackRangeCallback() {};
};

/// <summary>
/// Interface for transport and playback operations of a Smooth Streaming presentation.
/// <summary>
class ISmoothTransport
{
public:
    // Specifies target for command
    enum ECommandTarget
    {
        CommandTarget_Global,
        CommandTarget_Transport,
        CommandTarget_Socket,
        CommandTarget_Receiver,
        CommandTarget_Renderer
    };

public:
    /// <summary>
    /// Static function used to create an instance of ISmoothTransport for the default instance Id
    /// </summary>
    /// <returns>ISmoothTransport*</returns>
    static ISmoothTransport* CreateSmoothTransport();

    /// <summary>
    /// Static function used to create an instance of ISmoothTransport for a given Id to enable multiple instances
    /// </summary>
    /// <param name="pipeId">[IN] pipe Id of the current instance</param>
    /// <returns>ISmoothTransport*</returns>
    static ISmoothTransport* CreateSmoothTransport(_In_ const std::string& pipeId);

    /// <summary>
    /// Static function used to destroy the instance of ISmoothTransport
    /// </summary>
    /// <param name="SmoothTransport">[IN] pointer of SmoothTransport instance to be destroyed</param>
    /// <returns>void</returns>
    static void DestroySmoothTransport(_In_ ISmoothTransport* smoothTransport);

    /// <summary>
    /// Initialize SmoothTransport
    /// </summary>
    /// <returns>pkRESULT</returns>
    virtual pkRESULT Init(void) = 0;

    /// <summary>
    /// Tunes to tuneUrl (optionally starting up playback).  Results in a IManifestReadyCallback.
    /// </summary>
    /// <param name="tuneUrl">[IN] tuneUrl</param>
    /// <param name="protocol">[IN] protocol type, if not contained in tuneUrl</param>
    /// <param name="bAutoPlay">[IN] true = start playing immediately, false = wait for Play() call</param>
    /// <return>pkRESULT</return>
    virtual pkRESULT Open(_In_ const std::string& tuneUrl, _In_ ESmoothTransportProtocol protocol, _In_ bool bAutoPlay = false) = 0;

    /// <summary>
    /// Closes the transport, requires Open.
    /// </summary>
    /// <returns>pkRESULT</returns>
    virtual pkRESULT Close(void) = 0;

    /// <summary>
    /// Registers a status callback
    /// </summary>
    /// <param name="callback">[IN] callback interface</param>
    /// <param name="bRegister">[IN] bool - true = register, false = unregister</param>
    /// <returns>pkRESULT</returns>
    virtual pkRESULT RegisterStatusCallback(_In_opt_ ISmoothTransportStatusSink* callback, _In_ bool bRegister = true) = 0;

    /// <summary>
    /// Registers an error callback
    /// </summary>
    /// <param name="callback">[IN] callback interface</param>
    /// <param name="bRegister">[IN] bool - true = register, false = unregister</param>
    /// <return>pkRESULT</return>
    virtual pkRESULT RegisterErrorCallback(_In_opt_ ISmoothTransportErrorSink* callback, _In_ bool bRegister = true) = 0;

    /// <summary>
    /// Set or reset the manifest ready event callback
    /// </summary>
    /// <param name="pCallback">callback interface. Ignoring the parameter sets it to null</param>
    /// <returns>pkRESULT</returns>
    virtual pkRESULT SetManifestCallback(_In_opt_ IManifestReadyCallback* pCallback = NULL) = 0;

    /// <summary>
    /// Get the current playback time, in NTP units
    /// </summary>
    /// <param name="pTime">[OUT] current playback time</param>
    /// <return>void</return>
    virtual void GetCurrentPlaybackTime( _Out_ TimeSpan_NTP* pTime ) = 0;

    /// <summary>
    /// Start playback with the specified speed.
    /// </summary>
    /// <param name="speed">[IN] playback speed</param>
    /// <return>pkRESULT</return>
    virtual pkRESULT Play(_In_ float speed = 1.0) = 0;

    /// <summary>
    /// Start playback at the specified time with the specified speed
    /// </summary>
    /// <param name="timestamp">[IN] timestamp (in NTP units) from which the playback starts</param>
    /// <param name="speed">[IN] playback speed</param>
    /// <return>pkRESULT</return>
    virtual pkRESULT PlayAt(_In_ TimeSpan_NTP timestamp, _In_ float speed = 1.0) = 0; // timestamp: 0 = beginning, int64_t.Max = live/end

    /// <summary>
    /// Pause playback
    /// </summary>
    /// <return>pkRESULT</return>
    virtual pkRESULT Pause(void) = 0;

    /// <summary>
    /// Seek to the specified time, and play at 1x speed
    /// </summary>
    /// <param name="timestamp">[IN] specified time in NTP units</param>
    /// <return>pkRESULT</return>
    virtual pkRESULT Seek(_In_ TimeSpan_NTP timestamp) = 0;

    /// <summary>
    /// Skip forward or backward by a specified duration.
    /// If the playback is paused, it'll start playing from the new position at 1x.
    /// For other speeds, the speed will be preserved.
    /// </summary>
    /// <param name="duration">[IN] duration to skip, positive for forward and negative for backwards</param>
    /// <return>pkRESULT</return>
    virtual pkRESULT Skip(_In_ TimeSpan_NTP duration) = 0;

    /// <summary>
    /// Request to set the playback range. This api is only supported for manifest of type ManifestType_Segmented. 
    /// </summary>
    /// <param name="pCallback">[IN] The callback interface</param>
    /// <param name="minTime">[IN] the min time to set the playback range</param>
    /// <param name="maxTime">[IN] the max time to set the playback range, currently only live is support which must be max value (TimeSpan_NTP::MAX_TICKS)</param>
    virtual pkRESULT SetPlaybackRangeAsync(_In_ ISetPlaybackRangeCallback* pCallback, _In_ TimeSpan_NTP minTime, _In_ TimeSpan_NTP maxTime = TimeSpan_NTP::FromTicks(TimeSpan_NTP::MAX_TICKS) ) = 0;

    /// <summary>
    /// Send a command to SmoothTransport (as string vector)
    /// </summary>
    /// <param name="command">[IN] command</param>
    /// <param name="args">[IN] list of arguments</param>
    /// <param name="target">[IN] target of the command</param>
    /// <return>pkRESULT</return>
    virtual pkRESULT SendExtendedCommand(_In_ const std::string& command, _In_ const std::vector<std::string>& args, _In_ ECommandTarget target = CommandTarget_Global) = 0;

    /// <summary>
    /// Send a command to SmoothTransport (as char* array)
    /// </summary>
    /// <param name="command">[IN] command</param>
    /// <param name="argc">[IN] number of arguments</param>
    /// <param name="argv">[IN] array of arguments</param>
    /// <param name="target">[IN] target of the command</param>
    /// <return>pkRESULT</return>
    virtual pkRESULT SendExtendedCommand(_In_ const char* command, _In_ int argc, _In_ const char* argv[], _In_ ECommandTarget target = CommandTarget_Global) = 0;

protected:
    // make ctor and dtor protected to force using the
    // CreateSmoothTransport/DestroySmoothTransport api
    ISmoothTransport() {};
    virtual ~ISmoothTransport() {};
};

}; // namespace SSPK
