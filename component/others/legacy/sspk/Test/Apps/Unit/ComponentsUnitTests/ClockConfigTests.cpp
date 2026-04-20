///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <PKTestSuite.h>
#include <PKTestSuiteUtils.h>
#include <pkTestFramework.h>

#include <SSPKDefines.h>
#include <vector>
#include <CClockConfiguration.h>    // Components/Renderer

struct
{
    eClockConfigurationPropId propId;
    const char* pszCommand;
    uint32 expectedDefaultValue;
    uint32 valueToSet;
}
static const s_rgClockConfigTestData[] =
{
    { eDefaultBufferDelay_ms,                                   "defaultbufferdelay",                                   900,        15042 },
    { eAVBufferUnderflowThreshold_ms,                           "avbufferunderflowthreshold",                           0,          10250 },
    { eAVRebufferThreshold_ms,                                  "avrebufferthresholdms",                                250,        28491 },
    { eAVRebufferDelayIncrement_ms,                             "avrebufferdelayincrementms",                           1000,       3896 },
    { eAVRebufferMaxBufferDelay_ms,                             "avrebuffermaxbufferdelayms",                           6000,       20406 },
    { eMaxEsFifoLimitInNormalPlayback_90kHz,                    "maxesfifolimitinnormalplayback90kHz",                  90*8000,    13584*90 },
    { eEsFifoDeltaToRequestFragmentInNormalPlayback_90kHz,      "esfifodeltatorequestfragmentinnormalplayback90kHz",    90*2000,    7895*90 },
    { eMaxEsFifoLimitInTrickPlayback_90kHz,                     "maxesfifolimitintrickplayback90kHz",                   90*3000,    8427*90 },
    { eEsFifoDeltaToRequestFragmentInTrickPlayback_90kHz,       "esfifodeltatorequestfragmentintrickplayback90kHz",     90*750,     8907*90 },
    { eStreamChangeTransitionThreshold_ms,                      "streamchangetransitionthreshold",                      2000,       12686 },
};

PKTEST_GROUP( ClockConfiguration )
{
    ////////////////////////////////////
    PKTEST_METHOD( CheckDefaults )
    {
        CClockConfiguration fixture;

        for( size_t i = 0; i < sizeof(s_rgClockConfigTestData)/sizeof(s_rgClockConfigTestData[0]); ++i )
        {
            eClockConfigurationPropId propId = s_rgClockConfigTestData[i].propId;
            uint32_t defaultValue = s_rgClockConfigTestData[i].expectedDefaultValue;

            PKTEST_ASSERT_EXIT( fixture[ propId ] == defaultValue );
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( SetPropMethod )
    {
        CClockConfiguration fixture;

        // Set all to zero

        for( size_t i = 0; i < sizeof(s_rgClockConfigTestData)/sizeof(s_rgClockConfigTestData[0]); ++i )
        {
            fixture.SetProp( s_rgClockConfigTestData[i].propId, 0 );
        }

        // Check all to a new value

        for( size_t i = 0; i < sizeof(s_rgClockConfigTestData)/sizeof(s_rgClockConfigTestData[0]); ++i )
        {
            PKTEST_ASSERT_EXIT( fixture[ s_rgClockConfigTestData[i].propId ] == 0 );

            fixture.SetProp( s_rgClockConfigTestData[i].propId, s_rgClockConfigTestData[i].valueToSet );
        }

        // Ensure all are set to the right value

        for( size_t i = 0; i < sizeof(s_rgClockConfigTestData)/sizeof(s_rgClockConfigTestData[0]); ++i )
        {
            PKTEST_ASSERT_EXIT( fixture[ s_rgClockConfigTestData[i].propId ] == s_rgClockConfigTestData[i].valueToSet );
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( CommandMethod )
    {
        CClockConfiguration fixture;

        // Set all to zero

        for( size_t i = 0; i < sizeof(s_rgClockConfigTestData)/sizeof(s_rgClockConfigTestData[0]); ++i )
        {
            fixture.SetProp( s_rgClockConfigTestData[i].propId, 0 );
        }

        // Check setup through command

        std::string command;

        std::vector< std::string > args;
        args.resize( 1 );

        for( size_t i = 0; i < sizeof(s_rgClockConfigTestData)/sizeof(s_rgClockConfigTestData[0]); ++i )
        {
            char szValue[10];

            PKTEST_HRESULT_EXIT( StringCbPrintfA( szValue, sizeof(szValue), "%d", s_rgClockConfigTestData[i].valueToSet ) );

            command = s_rgClockConfigTestData[i].pszCommand;

            args[0] = szValue;

            PKTEST_ASSERT_EXIT( fixture.Command( command, args ) );
        }

        // Ensure all are set to the right value

        for( size_t i = 0; i < sizeof(s_rgClockConfigTestData)/sizeof(s_rgClockConfigTestData[0]); ++i )
        {
            PKTEST_ASSERT_EXIT( fixture[ s_rgClockConfigTestData[i].propId ] == s_rgClockConfigTestData[i].valueToSet );
        }

    exit:

        return;
    }
};
