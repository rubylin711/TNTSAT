///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

// Test PKTestDefines.h - common typedef and define declarations used by SSPK Test source code

#pragma once

#include "pkPAL.h"
#include "SSPKTimeSpan.h"

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
// Units per second for different timescales
// ===============================================================================================================
#define TIMESCALE_10MHZ             10000000
#define TIMESCALE_90KHZ             90000
#define MILLISECONDS_PER_SECOND     1000
#define SECONDS_PER_MINUTE          60

// ===============================================================================================================
// Max and Min values
// ===============================================================================================================
#define INT_MAX                     2147483647    /* maximum (signed) int value */
#define INT_MIN                     (-2147483647 - 1) /* minimum (signed) int value */
#define MIN_TIME64                  (0)
#define MAX_TIME64                  ((int64)0x7FFFFFFFFFFFFFFFLL)

#define TOTAL_SECONDS_IN_A_DAY      86400

// ===============================================================================================================
// Timeline validity related macros.  Used for various different timelines like PTS, NTP, NPT etc.
// ===============================================================================================================

//Invalid streaming timing
#define INVALID_TIME                ((uint64)0xFFFFFFFFFFFFFFFFLL)
//Is given time invalid
#define IS_INVALID_TIME(x)          ((x) == INVALID_TIME)
//Is given time valid
#define IS_VALID_TIME(x)            ((x) != INVALID_TIME)

// ===============================================================================================================
// NTP time units (seconds in 32.32 fixed binary point) conversions (inline to prevent parameter re-evaluation)
// ===============================================================================================================

//Conversion between 90KHz to 32.32FP units
inline uint64
NTP_UINT64TO90KHZ(uint64_t x) { return   ((((x) >> 32) * 90000) + ((((x) & 0xFFFFFFFF) * 90000) >> 32)); }
inline uint64
NTP_90KHZTOUINT64(uint64_t x) { return  ((((x) / 90000) << 32) + (uint32)((((x) % 90000) << 32) / 90000)); }

//Conversion between 10MHz and 32.32FP units
inline uint64
NTP_UINT64TO10MHZ(uint64_t x) { return   ((((x) >> 32) * 10000000) + ((((x) & 0xFFFFFFFF) * 10000000) >> 32)); }
inline uint64
NTP_10MHZTOUINT64(uint64_t x) { return   ((((x) / 10000000) << 32) + (uint32)((((x) % 10000000) << 32) / 10000000)); }

//Conversion from 32.32FP to 1KHz units
inline uint64
NTP_UINT64TO1KHZ(uint64_t x) { return   ((((x) >> 32) * 1000) + ((((x) & 0xFFFFFFFF) * 1000) >> 32)); }


namespace SSPKTest
{
// ===============================================================================================================
// Timeout values for the tests
// ===============================================================================================================

    static const int32_t STATECHANGE_TIMEOUT                            = 20 * MILLISECONDS_PER_SECOND;
    static const int32_t VALIDATION_DELAY                               = 5 * MILLISECONDS_PER_SECOND;
    static const int32_t DEFAULT_CHUNK_DOWNLOAD_LIMIT                   = 10 * MILLISECONDS_PER_SECOND;
    static const int32_t STREAMSELECTION_TIMEOUT                        = 5 * MILLISECONDS_PER_SECOND;
    static const int32_t PLAYBACK_TIMEOUT                               = 2 * 60 * MILLISECONDS_PER_SECOND;
    static const int32_t PLAYBACK_STARTUP_TIMEOUT                       = 10 * MILLISECONDS_PER_SECOND;
    static const int32_t MANIFESTREADY_TIMEOUT                          = 10 * MILLISECONDS_PER_SECOND;
    static const int32_t LIVE_SPARSE_STREAM_DEFAULT_TIMEOUT             = 30 * MILLISECONDS_PER_SECOND;
    static const int32_t SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT          = 30 * MILLISECONDS_PER_SECOND;
    static const int32_t CLOSE_TIMEOUT                                  = 2 * MILLISECONDS_PER_SECOND;
    static const int32_t IMMEDIATE_STATECHANGE_TIMEOUT                  = 3 * MILLISECONDS_PER_SECOND;
    static const int32_t SHORT_PLAY_DURATION                            = 10 * MILLISECONDS_PER_SECOND;
    static const int32_t HALF_SECOND_IN_MILLISECONDS                    = MILLISECONDS_PER_SECOND / 2;
    static const int32_t ZERO_TIMEOUT                                   = 0;

    static const int64_t NON_ACCURATE_SEEK_THRESHOLD                    = 4 * TIMESCALE_10MHZ;
    static const int64_t TIMESTAMP_COMPARISON_THRESHOLD                 = 500;
    static const int32_t RAND_SKIP_SECONDS                              = 10;
    static const double SECONDS_EPSILON                                 = .0001;

    static const float TRICK_PLAY_RATE_THRESHOLD                        = 1.0f;
    static const float TRICK_PLAY_ERROR_RATE                            = 0.15f;

    static const int32_t BITS_PER_KILOBITS                              = 1000;

    static const int64_t DEFAULT_CHUNK_DURATION                         = 2 * TIMESCALE_10MHZ;
    static const int64_t DEFAULT_BUFFER_SIZE                            = 6 * TIMESCALE_10MHZ;

    // _endTime in status callbacks also include the last chunk duration.
    static const int64_t LIVE_BACKOFF_TIME                              = 10 * TIMESCALE_10MHZ + DEFAULT_CHUNK_DURATION;
    static const int64_t DEFAULT_LIVE_PLAYBACK_OFFSET                   = 7 * TIMESCALE_10MHZ;
    static const int64_t DEFAULT_LIVE_BACKOFF                           = 3 * TIMESCALE_10MHZ;

    static const int64_t LIVE_LEFT_BACKOFF_TIME                         = 6 * TIMESCALE_10MHZ;
    static const int64_t DEFAULT_SKIP_10MHZ                             = 10 * TIMESCALE_10MHZ;
    static const int64_t LIVE_TO_FRAGINFO_START_TIME                    = 6 * TIMESCALE_10MHZ;
    static const float DEFAULT_TRICK_PLAY_SPEED                         = 8;
    static const int64_t SHORT_PLAY_DURATION_10MHZ                      = 10 * TIMESCALE_10MHZ;

    //Extended Command Strings
    static const char EXC_HTTPPROXY[]                                   = "httpproxyhost";
    static const char EXC_AUDIOHAL_BUFFER_THRESHOLD[]                   = "audiohalbufferingthreshold";
    static const char EXC_FIFO_LIMIT_NORMAL_PLAYBACK[]                  = "maxesfifolimitinnormalplayback90kHz";
    static const char EXC_FIFO_LIMIT_TRICK_PLAYBACK[]                   = "maxesfifolimitintrickplayback90kHz";
    static const char EXC_SSLIVE_DELAY[]                                = "sslivedelay";
    static const char EXC_CHUNK_LIST_MAX_SIZE[]                         = "chunklistmaxsize";
    static const char EXC_LIVE_BACKOFF[]                                = "sslivebackoffsec";
    static const char EXC_LIVE_PLAYBACK_OFFSET[]                        = "ssliveplaybackoffsetsec";
    static const char EXC_LIVE_BEGIN_EDGE_BACKOFF[]                     = "sslivemintimebuffersec";

    //For FF and RW
    static const float NORMAL_SPEED                                     = 1.0f;
    static const float LOW_SPEED                                        = 2.0f;
    static const float MEDIUM_SPEED                                     = 15.0f;
    static const float HIGH_SPEED                                       = 300.0f;


    //Useful conversions
    inline double MillisecondsToSeconds(int64_t milliseconds)
    {
        return SSPK::TimeSpan_ms::FromTicks(milliseconds).ToSeconds();
    }

    inline int64_t SecondsToMilliseconds(double seconds)
    {
        return SSPK::TimeSpan_ms::ConvertFrom(SSPK::TimeSpan_s::FromTicks((int64_t)seconds)).Ticks();;
    }

    inline int64_t TimestampToMilliseconds(int64_t timestamp)
    {
        return SSPK::TimeSpan_ms::ConvertFrom(SSPK::TimeSpan_hns::FromTicks(timestamp)).Ticks();
    }

    inline int64_t ToMilliseconds( SSPK::TimeSpan_NTP time )
    {
        return( SSPK::TimeSpan_ms::ConvertFrom( time ).Ticks() );
    }

    inline double TimestampToSeconds(int64_t timestamp)
    {
        return SSPK::TimeSpan_hns::FromTicks(timestamp).ToSeconds();
    }

    inline bool CompareTimeStamps(int64_t left, int64_t right, int64_t threshold)
    {
        if(left > right)
            return (left - right) <= threshold;
        else
            return (right - left) <= threshold;
    }

};
