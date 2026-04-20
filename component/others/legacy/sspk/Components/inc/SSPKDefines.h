///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

// SSPKDefines.h - common typedef and define declarations used by SSPK source code

#pragma once

#include "pkPAL.h"
#include "pkSockets.h"
#include "SmoothErrorDefinitions.h"

using namespace SSPK;

#ifdef ENABLE_ALL_COMPONENTS_SPEW
#include "enableAllSpew.h"
#endif

#define SSPKLIB

#ifndef uchar
typedef unsigned char   uchar;
#endif
#ifndef uint
typedef unsigned int    uint;
#endif
#ifndef ushort
typedef unsigned short  ushort;
#endif
#ifndef ulong
typedef unsigned long   ulong;
#endif

typedef int8_t          int8;
typedef uint8_t         uint8;
typedef int16_t         int16;
typedef uint16_t        uint16;
typedef int32_t         int32;
typedef uint32_t        uint32;
typedef int64_t         int64;
typedef uint64_t        uint64;

// ===============================================================================================================
// Generic uint32 constant for invalid values
// ===============================================================================================================

#define INVALID_UINT32 ((uint32) 0xFFFFFFFF)

// ===============================================================================================================
// Extracting different size types from a data stream
// ===============================================================================================================

#define MAKE_U16(d)             (((uint32)(d)[0] << 8) | (d)[1])
#define MAKE_U32(d)             (((uint32)(d)[0] << 24) | ((uint32)(d)[1] << 16) | ((uint32)(d)[2] << 8) | (d)[3])
#define MAKE_U64(d)             (((uint64)MAKE_U32(d) << 32) | MAKE_U32(&(d)[4]))

// ===============================================================================================================
// Minimum and maximum time in 64 bit unit
// Note: time is expressed interchangeably between uint64 and int64, assume max is int64
// ===============================================================================================================

#define MIN_TIME64              (0)
#define MAX_TIME64              ((int64)0x7FFFFFFFFFFFFFFFLL)

// ===============================================================================================================
// Timeline validity related macros.  Used for various different timelines like PTS, NTP, NPT etc.
// ===============================================================================================================

//Invalid streaming timing
#define INVALID_TIME            ((uint64)0xFFFFFFFFFFFFFFFFLL)
//Is given time invalid
#define IS_INVALID_TIME(x)      ((x) == INVALID_TIME)
//Is given time valid
#define IS_VALID_TIME(x)        ((x) != INVALID_TIME)

// ===============================================================================================================
// NTP time units (seconds in 32.32 fixed binary point) conversions (inline to prevent parameter re-evaluation)
// ===============================================================================================================

//Conversion between 90KHz to 32.32FP units
inline uint64
NTP_UINT64TO90KHZ(uint64 x) { return   ((((x) >> 32) * 90000) + ((((x) & 0xFFFFFFFF) * 90000) >> 32)); }
inline uint64
NTP_90KHZTOUINT64(uint64 x) { return  ((((x) / 90000) << 32) + (uint32)((((x) % 90000) << 32) / 90000)); }

//Conversion between 10MHz and 32.32FP units
inline uint64
NTP_UINT64TO10MHZ(uint64 x) { return   ((((x) >> 32) * 10000000) + ((((x) & 0xFFFFFFFF) * 10000000) >> 32)); }
inline uint64
NTP_10MHZTOUINT64(uint64 x) { return   ((((x) / 10000000) << 32) + (uint32)((((x) % 10000000) << 32) / 10000000)); }

//Conversion from 32.32FP to 1KHz units
inline uint64
NTP_UINT64TO1KHZ(uint64 x) { return   ((((x) >> 32) * 1000) + ((((x) & 0xFFFFFFFF) * 1000) >> 32)); }

// ===============================================================================================================
// Time unit mappings
// ===============================================================================================================

//Converts from 90KHz to 10MHz time scale
inline uint64
PTS_90KHZTO10MHZ(uint64 x) { return   (((x) * 1000) / 9); }

//Converts from 10MHZ to 90KHz time scale
inline uint64
PTS_10MHZTO90KHZ(uint64 x) { return    (((x) * 9) / 1000); }

//Converts from 10MHZ to 1KHz time scale
inline uint64
PTS_10MHZTO1KHZ(uint64 x) { return    (x / 10000); }

//Converts 32-bit 90KHz duration to 1KHz
inline uint32
DUR_90KHZTO1KHZ(uint32 x) { return   (x  / 90); }

//Converts 32-bit 10MHZ duration to 1Khz
inline uint32
DUR_10MHZTO1KHZ(uint32 x) { return    (x / 10000); }

// ===============================================================================================================
// Units per second for different timescales
// ===============================================================================================================
#define TIMESCALE_10MHZ         10000000
#define TIMESCALE_90KHZ         90000
#define TIMESCALE_NTP          (0x100000000LL)

// ===============================================================================================================
// Define max url len to avoid dependency on <wininet.h>
//
// #define INTERNET_MAX_PATH_LENGTH        2048
// #define INTERNET_MAX_SCHEME_LENGTH      32          // longest protocol name length
// #define INTERNET_MAX_URL_LENGTH         (INTERNET_MAX_SCHEME_LENGTH + sizeof("://") + INTERNET_MAX_PATH_LENGTH)
// ===============================================================================================================

#define SOCKET_MAX_URL_LENGTH (2083)

// ===============================================================================================================
// Max RTP Packet size supported
// ===============================================================================================================

#define MAX_RTP_PACKET_LENGTH (1472)

// ===============================================================================================================
// Maximum number of SSRCs to expect in a stream
// ===============================================================================================================

#define MAX_SSRC_COUNT (4)

// ===============================================================================================================
// Size of key id - used everywhere in AV Engine...
// ===============================================================================================================

#define DRM_KEYID_SIZE (128)

// ===============================================================================================================
// Sampld id
// ===============================================================================================================

//Invalid sample id
#define INVALID_SAMPLEID        (0xFFFFFFFFFFFFFFFFULL)
//Whether a given sample id is valid
#define IS_VALID_SAMPLEID(x)    ((x) != INVALID_SAMPLEID)

// ===============================================================================================================
// Invalid codec type
// ===============================================================================================================

#define INVALID_CODEC -1

// ===============================================================================================================
// Default frames per second played back during tricks
// ===============================================================================================================

#define DEFAULT_FPS_DURING_TRICKS (4)

// ===============================================================================================================
// ===============================================================================================================

#ifndef ASSERT
#define ASSERT assert
#endif

// ===============================================================================================================
// ===============================================================================================================

#define MIN(_a,_b) (((_a) < (_b)) ? (_a) : (_b))
#define MAX(_a,_b) (((_a) > (_b)) ? (_a) : (_b))

#define ABS(x)     ((x) < 0 ? -(x) : (x))

// ===============================================================================================================
// ===============================================================================================================

#ifndef CHECK_ALLOC
#define CHECK_ALLOC(_alloced_p) do {((_alloced_p) || (RaiseException(STATUS_NO_MEMORY, 0, 0, NULL), 0));__analysis_assume(!!(_alloced_p));} while(0)
#endif

// ===============================================================================================================
// ===============================================================================================================

#define SAFE_ADDREF(a)          { if(a) { (a)->AddRef(); } }
#define SAFE_RELEASE(a)         { if(a) { (a)->Release(); a = NULL; } }

#define SAFE_DELETE(a)          { if(a) { delete a; a = NULL; } }

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(a)    { if(a) { delete[] a; a = NULL; } }
#endif

// ===============================================================================================================
// HTTP Response Status Codes:
// ===============================================================================================================

#ifndef HTTP_STATUS_NOT_FOUND

#define HTTP_STATUS_CONTINUE            100 // OK to continue with request
#define HTTP_STATUS_SWITCH_PROTOCOLS    101 // server has switched protocols in upgrade header

#define HTTP_STATUS_OK                  200 // request completed
#define HTTP_STATUS_CREATED             201 // object created, reason = new URI
#define HTTP_STATUS_ACCEPTED            202 // async completion (TBS)
#define HTTP_STATUS_PARTIAL             203 // partial completion
#define HTTP_STATUS_NO_CONTENT          204 // no info to return
#define HTTP_STATUS_RESET_CONTENT       205 // request completed, but clear form
#define HTTP_STATUS_PARTIAL_CONTENT     206 // partial GET furfilled

#define HTTP_STATUS_AMBIGUOUS           300 // server couldn't decide what to return
#define HTTP_STATUS_MOVED               301 // object permanently moved
#define HTTP_STATUS_REDIRECT            302 // object temporarily moved
#define HTTP_STATUS_REDIRECT_METHOD     303 // redirection w/ new access method
#define HTTP_STATUS_NOT_MODIFIED        304 // if-modified-since was not modified
#define HTTP_STATUS_USE_PROXY           305 // redirection to proxy, location header specifies proxy to use
#define HTTP_STATUS_REDIRECT_KEEP_VERB  307 // HTTP/1.1: keep same verb

#define HTTP_STATUS_BAD_REQUEST         400 // invalid syntax
#define HTTP_STATUS_DENIED              401 // access denied
#define HTTP_STATUS_PAYMENT_REQ         402 // payment required
#define HTTP_STATUS_FORBIDDEN           403 // request forbidden
#define HTTP_STATUS_NOT_FOUND           404 // object not found
#define HTTP_STATUS_BAD_METHOD          405 // method is not allowed
#define HTTP_STATUS_NONE_ACCEPTABLE     406 // no response acceptable to client found
#define HTTP_STATUS_PROXY_AUTH_REQ      407 // proxy authentication required
#define HTTP_STATUS_REQUEST_TIMEOUT     408 // server timed out waiting for request
#define HTTP_STATUS_CONFLICT            409 // user should resubmit with more info
#define HTTP_STATUS_GONE                410 // the resource is no longer available
#define HTTP_STATUS_LENGTH_REQUIRED     411 // the server refused to accept request w/o a length
#define HTTP_STATUS_PRECOND_FAILED      412 // precondition given in request failed
#define HTTP_STATUS_REQUEST_TOO_LARGE   413 // request entity was too large
#define HTTP_STATUS_URI_TOO_LONG        414 // request URI too long
#define HTTP_STATUS_UNSUPPORTED_MEDIA   415 // unsupported media type
#define HTTP_STATUS_RETRY_WITH          449 // retry after doing the appropriate action.

#define HTTP_STATUS_SERVER_ERROR        500 // internal server error
#define HTTP_STATUS_NOT_SUPPORTED       501 // required not supported
#define HTTP_STATUS_BAD_GATEWAY         502 // error response received from gateway
#define HTTP_STATUS_SERVICE_UNAVAIL     503 // temporarily overloaded
#define HTTP_STATUS_GATEWAY_TIMEOUT     504 // timed out waiting for gateway
#define HTTP_STATUS_VERSION_NOT_SUP     505 // HTTP version not supported

#define HTTP_STATUS_FIRST               HTTP_STATUS_CONTINUE
#define HTTP_STATUS_LAST                HTTP_STATUS_VERSION_NOT_SUP

#endif

// ===============================================================================================================
// ===============================================================================================================
