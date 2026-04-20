///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

// ===============================================================================================================
// Various clock modes - sockets to provide a bitmap of different clock modes to the clock
// ===============================================================================================================

enum eClockMode
{
    //Clear/initialzie clock mode
    eClockMode_None                         = 0x00000000,
    //Need to take buffer delay into account
    eClockMode_UseBufferDelay               = 0x00000001,
    //Enforce additional buffer delay
    eClockMode_EnforceBufferDelay           = 0x00000002,
    //Auto audio primary detection
    eClockMode_AutoAudioPrimaryDetection    = 0x00000004,
    //Limit ES FIFO based on time
    eClockMode_TimeLimitFifo                = 0x00000008,
    //Allow rebuffering
    eClockMode_RebufferWhenUnderrun         = 0x00000010,
    //Sync when under running
    eClockMode_SyncOnUnderrun               = 0x00000020,
    //Turn off ICC mode when syncing while in fast start
    eClockMode_TurnFastStartOffWhenSyncing  = 0x00000040,
    //Fix base tiem when no explicit timing available
    eClockMode_FixBaseTime                  = 0x00000080,
};

// ===============================================================================================================
// Clock timing mode - App could request stream time depending upon the transport
// and tune type.  Native tune will be provided with a flag so that it can report
// time appropriately.
// ===============================================================================================================

enum eClockTimingMode
{
    //Unknown timing mode
    eClockTimingMode_Unknown,

    //Send time in NTP units
    eClockTimingMode_NTPUnit,

    //Tming in raw 90 kHz units
    eClockTimingMode_STCUnit,

    //Max timing modes
    eClockTimingMode_Max,
};

/// <summary>
/// CONFIG: list of configuration properties for the clock
/// </summary>
enum eClockConfigurationPropId
{
    /// <summary>
    /// Default value for how much content is buffered before the presentation clock starts.
    /// </summary>
    eDefaultBufferDelay_ms = 0,

    /// <summary>
    /// How low the buffer level needs to get before a "buffer underrun" is signaled.
    /// The default value is chosen as roughly 1/2 the duration of a frame (for a NTSC frame frequency)
    /// </summary>
    eAVBufferUnderflowThreshold_ms,

    /// <summary>
    /// How low the buffer level needs to get to trigger a re-buffer attempt
    /// </summary>
    eAVRebufferThreshold_ms,

    /// <summary>
    /// How much the buffer delay is incremented when there's a re-buffer.
    /// The increment "sticks" for the duration of the connection.
    /// </summary>
    eAVRebufferDelayIncrement_ms,

    /// <summary>
    /// Maximum value the buffer delay can get to when there's an increment due to a re-buffer.
    /// </summary>
    eAVRebufferMaxBufferDelay_ms,

    /// <summary>
    /// Maximum buffer level for normal playback (buffer fullness).
    /// </summary>
    eMaxEsFifoLimitInNormalPlayback_90kHz,

    /// <summary>
    /// How much the buffer has to be below the max level (eMaxEsFifoLimitInNormalPlayback_90kHz)
    /// before the downloader sends a request for another fragment.
    /// </summary>
    eEsFifoDeltaToRequestFragmentInNormalPlayback_90kHz,

    /// <summary>
    /// Maximum buffer level for trick playback (buffer fullness).
    /// </summary>
    eMaxEsFifoLimitInTrickPlayback_90kHz,

    /// <summary>
    /// How much the buffer has to be below the max level (eMaxEsFifoLimitInTrickPlayback_90kHz)
    /// before the downloader sends a request for another fragment.
    /// </summary>
    eEsFifoDeltaToRequestFragmentInTrickPlayback_90kHz,

    /// <summary>
    /// Time after a stream change to allow PTS delta to settle.
    /// During this interval the underflow detection will be off.
    /// </summary>
    eStreamChangeTransitionThreshold_ms,

    eClockConfigurationNumOfProperties
};

// ===============================================================================================================
// Global configuration parameters controlling clock behavior
// ===============================================================================================================

class CClockConfiguration
{
public:

    CClockConfiguration();

    //Process commands
    bool Command(
            _In_ const std::string& command,
            _In_ const std::vector<std::string>& args );


    uint32 operator[]( _In_ eClockConfigurationPropId propId ) const
    {
        ASSERT( 0 <= propId && propId < eClockConfigurationNumOfProperties );
        return( m_rgProperties[propId] );
    }

    void SetProp( _In_ eClockConfigurationPropId propId, _In_ uint32 value )
    {
        ASSERT( 0 <= propId && propId < eClockConfigurationNumOfProperties );
        m_rgProperties[propId] = value;
    }

private:

    uint32 m_rgProperties[eClockConfigurationNumOfProperties];
};

// ===============================================================================================================
// ===============================================================================================================

extern CClockConfiguration gClockConfiguration;

// ===============================================================================================================
// ===============================================================================================================
