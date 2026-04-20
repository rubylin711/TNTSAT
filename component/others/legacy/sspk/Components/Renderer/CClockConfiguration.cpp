///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CClockConfiguration.h"
#include "CStreamInfo.h"
#include "Trace.h"
#include <string>
#include <vector>
using namespace std;

// ===============================================================================================================
// Auxiliary local macros and functions
// ===============================================================================================================

#define FROM_ms_TO_90kHz( v )     ((v)*90)

// ===============================================================================================================
// Table of descriptors for the CClockConfiguration's properties. It maps the property ID to the command name
// and default value. The entries must be in ascending order of property ID.
// ===============================================================================================================

struct
{
    eClockConfigurationPropId propId;
    const char* pszCommand;
    uint32 defaultValue;
}
static const s_rgClockConfigPropDescriptors[] =
{
    { eDefaultBufferDelay_ms,                                   "defaultbufferdelay",                                   900 },
    { eAVBufferUnderflowThreshold_ms,                           "avbufferunderflowthreshold",                           0 },
    { eAVRebufferThreshold_ms,                                  "avrebufferthresholdms",                                250 },
    { eAVRebufferDelayIncrement_ms,                             "avrebufferdelayincrementms",                           1000 },
    { eAVRebufferMaxBufferDelay_ms,                             "avrebuffermaxbufferdelayms",                           6000 },
    { eMaxEsFifoLimitInNormalPlayback_90kHz,                    "maxesfifolimitinnormalplayback90kHz",                  FROM_ms_TO_90kHz( 8000 ) },
    { eEsFifoDeltaToRequestFragmentInNormalPlayback_90kHz,      "esfifodeltatorequestfragmentinnormalplayback90kHz",    FROM_ms_TO_90kHz( 2000 ) },
    { eMaxEsFifoLimitInTrickPlayback_90kHz,                     "maxesfifolimitintrickplayback90kHz",                   FROM_ms_TO_90kHz( 3000 ) },
    { eEsFifoDeltaToRequestFragmentInTrickPlayback_90kHz,       "esfifodeltatorequestfragmentintrickplayback90kHz",     FROM_ms_TO_90kHz( 750 ) },
    { eStreamChangeTransitionThreshold_ms,                      "streamchangetransitionthreshold",                      2000 },
};


// ===============================================================================================================
// Global configuration parameters controlling clock behavior
// ===============================================================================================================

CClockConfiguration::CClockConfiguration()
{
    for(
        size_t i = 0;
        i < sizeof(s_rgClockConfigPropDescriptors)/sizeof(s_rgClockConfigPropDescriptors[0]);
        ++i
        )
    {
        ASSERT( s_rgClockConfigPropDescriptors[i].propId == (eClockConfigurationPropId) i );

        SetProp( s_rgClockConfigPropDescriptors[i].propId, s_rgClockConfigPropDescriptors[i].defaultValue );
    }
}

//Process commands
bool CClockConfiguration::Command(
    _In_ const std::string& command,
    _In_ const std::vector<std::string>& args )
{
    bool fFound = false;

    for(
        size_t i = 0;
        i < sizeof(s_rgClockConfigPropDescriptors)/sizeof(s_rgClockConfigPropDescriptors[0]);
        ++i
        )
    {
        if( command == s_rgClockConfigPropDescriptors[i].pszCommand )
        {
            if( args.size() == 1 )
            {
                uint32 value = atoi( args[0].c_str() );

                SetProp(
                    s_rgClockConfigPropDescriptors[i].propId,
                    value
                    );

                TRACE(("%s = %u", command.c_str(), value ));
            }

            fFound = true;

            break;
        }
    }

    return fFound;
}

// ===============================================================================================================
// ===============================================================================================================

CClockConfiguration gClockConfiguration;

// ===============================================================================================================
// ===============================================================================================================
