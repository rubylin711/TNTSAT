///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace SSPK
{
// ===============================================================================================================
// Tuner error states
// ===============================================================================================================
enum eTunerError
{
    // Success
    eTunerErrorNone,
    // Socket Errors
    eTunerErrorConnectFailed,
    // Errors during buffer receives
    eTunerErrorReadFailed,
    // EOF was reached, not really an error!
    eTunerErrorEOF,
    // Memory allocation failure
    eTunerErrorAllocationFailure,
    // Cannot share Playback Receivers
    eTunerErrorCannotSharePlaybackReceivers,
    // Chunk data is invalid
    eTunerErrorInvalidChunkData,
    // Drm initialization failure
    eTunerErrorDrmInitFailure,
};

// ===============================================================================================================
// Different types of socket errors
// ===============================================================================================================
enum eSocketError
{
    // Success
    eSocketErrorNone = 0,
    eSocketErrorAlreadyClosed,
    eSocketErrorReadError,

    // HTTP Socket errors
    eSocketErrorHttpOpenFailed,
    eSocketErrorHttpConnectFailed,
    eSocketErrorHttpSendRequestFailed,
    eSocketErrorHttpRecvResponseFailed,
    eSocketErrorHttpParseResponseFailed,
    eSocketErrorHttpInvalidResult,
    eSocketErrorHttpTooManyRedirect,
    eSocketErrorHttpRedirectFailed,
    eSocketErrorHttpRedirectNotAllowed,
    eSocketErrorHttpCreateFailed,
 
    // Manifest socket
    eSocketErrorSSManifestParsingFailed,
    eSocketErrorSSManifestVersionUnsupported,
    eSocketErrorSSManifestInvalid,
    eSocketErrorSSManifestHttpInvalid,

    // Chunk socket
    eSocketErrorSSChunkConnectHttpInvalid,
    eSocketErrorSSNextChunkHttpInvalid,
    eSocketErrorSSChunkHdrParsingFailed,

    // Drm
    eSocketErrorSSDrmInitFailed,
};

}; // namespace SSPK
