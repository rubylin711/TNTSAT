///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CSmoothTransportStatusHandler.h"
#include "SmoothTuneParameters.h"
#include "StringUtils.h"
#include "Trace.h"

#include <string>
#include <map>
using namespace std;

//#define SmoothTransportSTATUSHANDLER_SPEW
#if defined(SmoothTransportSTATUSHANDLER_SPEW)
#define SmoothTransportHANDLER_TRACE(x) TRACE(x)
#else
#define SmoothTransportHANDLER_TRACE(x)
#endif

//status string is the form of "<argname1>=<argvalue1>&<argname2>=<argvalue2>..."

//arg names
#define STATUS_STR_ARGNAME_STATUS                "status"
#define STATUS_STR_ARGNAME_STATE                 "state"
#define STATUS_STR_ARGNAME_TYPE                  "type"
#define STATUS_STR_ARGNAME_STARTTIME             "starttime"
#define STATUS_STR_ARGNAME_ENDTIME               "endtime"
#define STATUS_STR_ARGNAME_HEARTTIME             "hearttime"
#define STATUS_STR_ARGNAME_CURRENTTIME           "currenttime"
#define STATUS_STR_ARGNAME_TUNERERROR            "tunererror"
#define STATUS_STR_ARGNAME_SOCKETERROR           "socketerror"
#define STATUS_STR_ARGNAME_HTTPRESPONSE          "httpresponse"
#define STATUS_STR_ARGNAME_PKRESULT              "pkresult"
#define STATUS_STR_ARGNAME_BITRATE               "bps"
#define STATUS_STR_ARGNAME_DECODERCODE           "code"
#define STATUS_STR_ARGNAME_URL                   "url"

//arg value following "status=..."
#define STATUS_STR_ARGVALUE_HEARTBEAT            "heartbeat"
#define STATUS_STR_ARGVALUE_TUNER                "tuner"
#define STATUS_STR_ARGVALUE_STREAM               "stream"
#define STATUS_STR_ARGVALUE_FINGERPRINT          "fingerprint"
#define STATUS_STR_ARGVALUE_INBAND               "inband"
#define STATUS_STR_ARGVALUE_ACCESSCONTROL        "accesscontrol"
#define STATUS_STR_ARGVALUE_AFD                  "afd"
#define STATUS_STR_ARGVALUE_SIGNAL               "signal"
#define STATUS_STR_ARGVALUE_STREAMING            "streaming"
#define STATUS_STR_ARGVALUE_RENDERING            "rendering"
#define STATUS_STR_ARGVALUE_UNDERRUN             "underrun"
#define STATUS_STR_ARGVALUE_REBUFFER             "rebuffer"
#define STATUS_STR_ARGVALUE_STARTENDTIME         "startendtime"
#define STATUS_STR_ARGVALUE_DRMSTATE             "drmstate"
#define STATUS_STR_ARGVALUE_CURRENTBITRATE       "currentbitrate"
#define STATUS_STR_ARGVALUE_DECODERERROR         "decodererror"
#define STATUS_STR_ARGVALUE_CHUNKCONNECTHTTPINV  "chunkconnecthttpinvalid"
#define STATUS_STR_ARGVALUE_NEXTCHUNKHTTPINV     "nextchunkhttpinvalid"
#define STATUS_STR_ARGVALUE_CHUNKHDRHTTPINV      "chunkhdrhttpinvalid"
#define STATUS_STR_ARGVALUE_CHUNKHDRERROR        "chunkhdrerror"
#define STATUS_STR_ARGVALUE_ATWINDOWEDGE         "atwindowedge"
#define STATUS_STR_ARGVALUE_OUTSIDEWINDOWEDGE    "outsidewindowedge"
#define STATUS_STR_ARGVALUE_ENDOFLIVE            "endoflive"
#define STATUS_STR_ARGVALUE_SEGMENTMANIFESTERR   "segmentmanifesterror"
#define STATUS_STR_ARGVALUE_DRMINITERR           "drminiterror"

//value following "state=..."
#define STATUS_STR_ARGVALUE_DETUNED              "detuned"
#define STATUS_STR_ARGVALUE_PLAYING              "playing"
#define STATUS_STR_ARGVALUE_PAUSED               "paused"
#define STATUS_STR_ARGVALUE_MEDIAENDED           "mediaended"
#define STATUS_STR_ARGVALUE_CLOSED               "closed"

std::map<std::string, ESmoothTransportState> CSmoothTransportStatusHelper::_stateArgMap;
std::map<std::string, ESmoothTransportStatusUpdate> CSmoothTransportStatusHelper::_statusArgMap;

#if defined(SmoothTransportSTATUSHANDLER_SPEW)
const char* sStateNames[SmoothTransportTunerState_Max] =
{
    "SmoothTransportTunerState_Unknown",
    "SmoothTransportTunerState_Tuning",
    "SmoothTransportTunerState_Playing",
    "SmoothTransportTunerState_Paused",
    "SmoothTransportTunerState_MediaEnded",
    "SmoothTransportTunerState_Detuned",
    "SmoothTransportTunerState_Closed"
};

const char* sUpdateNames[SmoothTransportStatus_Max] =
{
    "SmoothTransportStatus_Unknown",
    "SmoothTransportStatus_Heartbeat",
    "SmoothTransportStatus_TunerStateChanged",
    "SmoothTransportStatus_Streaming",
    "SmoothTransportStatus_Rendering",
    "SmoothTransportStatus_Underrun",
    "SmoothTransportStatus_Rebuffer",
    "SmoothTransportStatus_StartEndTime",
    "SmoothTransportStatus_DrmStateChanged",
    "SmoothTransportStatus_BitrateChanged",
    "SmoothTransportStatus_DecoderError",
    "SmoothTransportStatus_ChunkConnectHttpInvalid",
    "SmoothTransportStatus_NextChunkHttpInvalid",
    "SmoothTransportStatus_ChunkHdrHttpInvalid",
    "SmoothTransportStatus_ChunkHdrError",
    "SmoothTransportStatus_AtWindowEdge",
    "SmoothTransportStatus_EndOfLive",
    "SmoothTransportStatus_OutsideWindowEdge",
    "SmoothTransportStatus_SegmentManifestFailure",
    "SmoothTransportStatus_DrmInitError"
};

#endif

void CSmoothTransportStatusHelper::Init()
{
    if (_statusArgMap.empty())
    {
        _statusArgMap[STATUS_STR_ARGVALUE_TUNER]              = SmoothTransportStatus_TunerStateChanged;
        _statusArgMap[STATUS_STR_ARGVALUE_HEARTBEAT]          = SmoothTransportStatus_Heartbeat;        //heartbeat
        _statusArgMap[STATUS_STR_ARGVALUE_STREAM]             = SmoothTransportStatus_Heartbeat;        //N/A for mbr
        _statusArgMap[STATUS_STR_ARGVALUE_FINGERPRINT]        = SmoothTransportStatus_Heartbeat;        //N/A for mbr
        _statusArgMap[STATUS_STR_ARGVALUE_INBAND]             = SmoothTransportStatus_Heartbeat;        //N/A for mbr
        _statusArgMap[STATUS_STR_ARGVALUE_ACCESSCONTROL]      = SmoothTransportStatus_Heartbeat;        //N/A for mbr
        _statusArgMap[STATUS_STR_ARGVALUE_AFD]                = SmoothTransportStatus_Heartbeat;        //N/A for mbr
        _statusArgMap[STATUS_STR_ARGVALUE_SIGNAL]             = SmoothTransportStatus_Heartbeat;        //N/A for mbr
        _statusArgMap[STATUS_STR_ARGVALUE_STREAMING]          = SmoothTransportStatus_Streaming;
        _statusArgMap[STATUS_STR_ARGVALUE_RENDERING]          = SmoothTransportStatus_Rendering;
        _statusArgMap[STATUS_STR_ARGVALUE_UNDERRUN]           = SmoothTransportStatus_Underrun;
        _statusArgMap[STATUS_STR_ARGVALUE_REBUFFER]           = SmoothTransportStatus_Rebuffer;
        _statusArgMap[STATUS_STR_ARGVALUE_STARTENDTIME]       = SmoothTransportStatus_StartEndTime;
        _statusArgMap[STATUS_STR_ARGVALUE_DRMSTATE]           = SmoothTransportStatus_DrmStateChanged;
        _statusArgMap[STATUS_STR_ARGVALUE_CURRENTBITRATE]     = SmoothTransportStatus_BitrateChanged;
        _statusArgMap[STATUS_STR_ARGVALUE_DECODERERROR]       = SmoothTransportStatus_DecoderError;
        _statusArgMap[STATUS_STR_ARGVALUE_CHUNKCONNECTHTTPINV]= SmoothTransportStatus_ChunkConnectHttpInvalid;
        _statusArgMap[STATUS_STR_ARGVALUE_NEXTCHUNKHTTPINV]   = SmoothTransportStatus_NextChunkHttpInvalid;
        _statusArgMap[STATUS_STR_ARGVALUE_CHUNKHDRHTTPINV]    = SmoothTransportStatus_ChunkHdrHttpInvalid;
        _statusArgMap[STATUS_STR_ARGVALUE_CHUNKHDRERROR]      = SmoothTransportStatus_ChunkHdrError;
        _statusArgMap[STATUS_STR_ARGVALUE_ATWINDOWEDGE]       = SmoothTransportStatus_AtWindowEdge;
        _statusArgMap[STATUS_STR_ARGVALUE_ENDOFLIVE]          = SmoothTransportStatus_EndOfLive;
        _statusArgMap[STATUS_STR_ARGVALUE_OUTSIDEWINDOWEDGE]  = SmoothTransportStatus_OutsideWindowEdge;
        _statusArgMap[STATUS_STR_ARGVALUE_SEGMENTMANIFESTERR] = SmoothTransportStatus_SegmentManifestError;
        _statusArgMap[STATUS_STR_ARGVALUE_DRMINITERR]         = SmoothTransportStatus_DrmInitError;
    }

    if (_stateArgMap.empty())
    {
        _stateArgMap[STATUS_STR_ARGVALUE_DETUNED]        = SmoothTransportTunerState_Detuned;
        _stateArgMap[STATUS_STR_ARGVALUE_PLAYING]        = SmoothTransportTunerState_Playing;
        _stateArgMap[STATUS_STR_ARGVALUE_PAUSED]         = SmoothTransportTunerState_Paused;
        _stateArgMap[STATUS_STR_ARGVALUE_MEDIAENDED]     = SmoothTransportTunerState_MediaEnded;
        _stateArgMap[STATUS_STR_ARGVALUE_CLOSED]         = SmoothTransportTunerState_Closed;
    }
}

void CSmoothTransportStatusHelper::DumpStatus(const SmoothTransportStatus& status, const SmoothTransportError& errorInfo)
{
#if defined(SmoothTransportSTATUSHANDLER_SPEW)

    TRACE(("------------Status----------------"));
    TRACE(("_currentState  :%s", sStateNames[status._currentState]));
    TRACE(("_currentTime   :%.3f", status._currentTime.ToSeconds()));
    TRACE(("_startTime     :%.3f", status._startTime.ToSeconds()));
    TRACE(("_endTime       :%.3f", status._endTime.ToSeconds()));
    TRACE(("_speed         :%f", status._speed));
    TRACE(("_update        :%s", sUpdateNames[status._update]));
    TRACE(("_additionalInfo:%s", status._additionalInfo.c_str()));
    TRACE(("_pkResult      :0x%08x", status._pkResult));
    TRACE(("_httpResponse  :%d", status._httpResponse));

    if (status._currentState == SmoothTransportTunerState_Detuned)
    {
        TRACE(("Error:%d, pkResult: %d, httpResponse: %d, %s",
            errorInfo._errorCode,
            errorInfo._pkResult,
            errorInfo._httpResponse,
            errorInfo._message.c_str()));
    }

    TRACE(("----------------------------------"));
#endif
}

SmoothTransportStatusInternal::SmoothTransportStatusInternal()
{
    Reset();
}

void SmoothTransportStatusInternal::Reset()
{
    AutoLock lock(&_lock);

    _status._currentState = SmoothTransportTunerState_Unknown;
    _status._currentTime.ResetTicks();
    _status._startTime.ResetTicks();
    _status._endTime.ResetTicks();
    _status._speed = 0.0;
    _status._update = SmoothTransportStatus_Heartbeat;
    _status._additionalInfo = "";
    _status._clockStarted = false;
    _status._pkResult = pkS_OK;
    _status._httpResponse = 0;

}

void SmoothTransportStatusInternal::Tune(float speed)
{
    AutoLock lock(&_lock);

    _status._currentState = SmoothTransportTunerState_Tuning;
    _status._clockStarted = false;
    _status._speed = speed;
}

void SmoothTransportStatusInternal::Pause()
{
    AutoLock lock(&_lock);

    _status._currentState = SmoothTransportTunerState_Paused;
}

void SmoothTransportStatusInternal::Resume()
{
    AutoLock lock(&_lock);

    _status._currentState = SmoothTransportTunerState_Playing;
}

void SmoothTransportStatusInternal::SetStartEndTime( TimeSpan_NTP startTime, TimeSpan_NTP endTime)
{
    AutoLock lock(&_lock);

    _status._startTime = startTime;
    _status._endTime = endTime;
}

SmoothTransportStatus SmoothTransportStatusInternal::OnStatusEvent( ArgsMap& args, TimeSpan_NTP currentTime, CSmoothTuneParameters& smoothTuneParameters)
{
    AutoLock lock(&_lock);

    if (!args[STATUS_STR_ARGNAME_STATE].empty())
    {
        _status._currentState = CSmoothTransportStatusHelper::_stateArgMap[args[STATUS_STR_ARGNAME_STATE]];

        //if the key is not found in the map then zero is returned, which is reserved as SmoothTransportTunerState_Unknown
        ASSERT(SmoothTransportTunerState_Unknown != _status._currentState);

        // store the start/end time to report an accurate current playback time since the start/end time can change for live
        if (SmoothTransportTunerState_MediaEnded == _status._currentState)
        {
            smoothTuneParameters.SetMediaEndedTime();
        }
    }
    if (!args[STATUS_STR_ARGNAME_STATUS].empty())
    {
        _status._update = CSmoothTransportStatusHelper::_statusArgMap[args[STATUS_STR_ARGNAME_STATUS]];

        //if the key is not found in the map then zero is returned, which is reserved as SmoothTransportStatus_Unknown
        ASSERT(SmoothTransportStatus_Unknown != _status._update);

        // store that the playback has been over taken by the moving dvr
        if (SmoothTransportStatus_OutsideWindowEdge == _status._update)
        {
            smoothTuneParameters.SetPlaybackOverTaken();
        }
    }

    _status._additionalInfo = "";
    _status._pkResult = toULong(args[STATUS_STR_ARGNAME_PKRESULT]);
    _status._httpResponse = toULong(args[STATUS_STR_ARGNAME_HTTPRESPONSE]);

    if (_status._update == SmoothTransportStatus_Underrun || _status._update == SmoothTransportStatus_Rebuffer)
    {
        _status._additionalInfo = args[STATUS_STR_ARGNAME_TYPE];
    }
    else if (_status._update == SmoothTransportStatus_StartEndTime)
    {
        if (!args[STATUS_STR_ARGNAME_STARTTIME].empty())
        {
            _status._startTime = TimeSpan_NTP::FromTicks( toUInt64(args[STATUS_STR_ARGNAME_STARTTIME]) );
            smoothTuneParameters.SetStartTime(_status._startTime);
        }
        if (!args[STATUS_STR_ARGNAME_ENDTIME].empty())
        {
            _status._endTime = TimeSpan_NTP::FromTicks( toUInt64(args[STATUS_STR_ARGNAME_ENDTIME]) );
            smoothTuneParameters.SetEndTime(_status._endTime);
        }
    }
    else if (_status._update == SmoothTransportStatus_Rendering)
    {
        _status._clockStarted = true;
    }
    else if (_status._update == SmoothTransportStatus_BitrateChanged)
    {
        _status._additionalInfo = args[STATUS_STR_ARGNAME_BITRATE];
    }
    else if ((_status._update == SmoothTransportStatus_ChunkConnectHttpInvalid) ||
        (_status._update == SmoothTransportStatus_NextChunkHttpInvalid) ||
        (_status._update == SmoothTransportStatus_ChunkHdrHttpInvalid) ||
        (_status._update == SmoothTransportStatus_ChunkHdrError) ||
        (_status._update == SmoothTransportStatus_SegmentManifestError) )
    {
        _status._additionalInfo = args[STATUS_STR_ARGNAME_URL];
    }

    if (_status._currentState == SmoothTransportTunerState_Detuned)
    {
        _status._update = SmoothTransportStatus_TunerStateChanged;
    }

    if (!args[STATUS_STR_ARGNAME_HEARTTIME].empty())
    {
        _status._currentTime = TimeSpan_NTP::FromTicks( toUInt64(args[STATUS_STR_ARGNAME_HEARTTIME]) );
    }
    else if (!args[STATUS_STR_ARGNAME_CURRENTTIME].empty())
    {
        _status._currentTime = TimeSpan_NTP::FromTicks( toUInt64(args[STATUS_STR_ARGNAME_CURRENTTIME]) );
    }

    if ((-1 == _status._currentTime.Ticks()) || args[STATUS_STR_ARGNAME_CURRENTTIME].empty())
    {
        _status._currentTime = currentTime;
    }

    return _status;
}

ESmoothTransportState SmoothTransportStatusInternal::GetStatus_CurrentState(_Out_opt_ bool* pClockStarted)
{
    AutoLock lock(&_lock);

    if(pClockStarted)
    {
        *pClockStarted = _status._clockStarted;
    }

    return _status._currentState;
}

SmoothTransportErrorInternal::SmoothTransportErrorInternal()
{
    Reset();
}

void SmoothTransportErrorInternal::Reset()
{
    AutoLock lock(&_lock);

    _errorInfo._errorCode = SmoothTransportError_None;
    _errorInfo._pkResult = pkS_OK;
    _errorInfo._httpResponse = 0;
}

SmoothTransportError SmoothTransportErrorInternal::OnStatusEvent(ArgsMap& args)
{
    AutoLock lock(&_lock);

    _errorInfo._errorCode = MapError((eTunerError)toInt(args[STATUS_STR_ARGNAME_TUNERERROR]),
        (eSocketError)toInt(args[STATUS_STR_ARGNAME_SOCKETERROR]));
    _errorInfo._pkResult = toULong(args[STATUS_STR_ARGNAME_PKRESULT]);
    _errorInfo._httpResponse = toULong(args[STATUS_STR_ARGNAME_HTTPRESPONSE]);

    return _errorInfo;
}

ESmoothTransportError SmoothTransportErrorInternal::MapError(eTunerError tunerError, eSocketError socketError)
{
    switch (tunerError)
    {
        case eTunerErrorNone:
        case eTunerErrorEOF:
            return SmoothTransportError_None;

        case eTunerErrorAllocationFailure:
            return SmoothTransportError_TunerAllocationFailure;

        case eTunerErrorCannotSharePlaybackReceivers:
            return SmoothTransportError_TunerSharedReceivers;

        case eTunerErrorInvalidChunkData:
            return SmoothTransportError_ChunkInvalidData;

        case eTunerErrorDrmInitFailure:
            return SmoothTransportError_DrmInitFailed;

        case eTunerErrorConnectFailed:
        case eTunerErrorReadFailed:
            // let socketError define the error
            switch (socketError)
            {
                case eSocketErrorNone:
                    return SmoothTransportError_Unknown;

                case eSocketErrorAlreadyClosed:
                    return SmoothTransportError_SocketAlreadyClosed;

                case eSocketErrorReadError:
                    return SmoothTransportError_SocketReadError;

                case eSocketErrorHttpOpenFailed:
                    return SmoothTransportError_SocketOpenFailed;

                case eSocketErrorHttpConnectFailed:
                    return SmoothTransportError_SocketConnectFailed;

                case eSocketErrorHttpSendRequestFailed:
                    return SmoothTransportError_SocketSendFailed;

                case eSocketErrorHttpRecvResponseFailed:
                    return SmoothTransportError_SocketRecvFailed;

                case eSocketErrorHttpParseResponseFailed:
                    return SmoothTransportError_HttpParseResponseFailed;

                case eSocketErrorHttpInvalidResult:
                    return SmoothTransportError_HttpInvalidResult;

                case eSocketErrorHttpTooManyRedirect:
                    return SmoothTransportError_HttpTooManyRedirect;

                case eSocketErrorHttpRedirectFailed:
                    return SmoothTransportError_HttpRedirectFailed;

                case eSocketErrorHttpRedirectNotAllowed:
                    return SmoothTransportError_HttpRedirectNotAllowed;

                case eSocketErrorHttpCreateFailed:
                    return SmoothTransportError_HttpCreateFailed;

                case eSocketErrorSSManifestParsingFailed:
                    return SmoothTransportError_ManifestParseFailed;

                case eSocketErrorSSManifestVersionUnsupported:
                    return SmoothTransportError_ManifestVersionUnsupported;

                case eSocketErrorSSManifestInvalid:
                    return SmoothTransportError_ManifestInvalid;

                case eSocketErrorSSManifestHttpInvalid:
                    return SmoothTransportError_ManifestHttpInvalidResult;

                case eSocketErrorSSChunkConnectHttpInvalid:
                    return SmoothTransportError_ChunkConnectHttpInvalidResult;

                case eSocketErrorSSNextChunkHttpInvalid:
                    return SmoothTransportError_ChunkNextHttpInvalidResult;

                case eSocketErrorSSChunkHdrParsingFailed:
                    return SmoothTransportError_ChunkHdrParseFailed;

                case eSocketErrorSSDrmInitFailed:
                    return SmoothTransportError_DrmInitFailed;
                default:
                    break;
            }
        default:
            break;
    }

    TRACE_ERROR(("Failed to MapError tunerError:%d, socketError:%d", tunerError, socketError));
    return SmoothTransportError_Unknown;
}

