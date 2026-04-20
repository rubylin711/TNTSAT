///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <SSPKDefines.h>
#include <PKTestSuite.h>

#include <vector>
#include <string>
#include <map>

#include "CManifestTrack.h"

#include <AutoRefPtr.h>

////////////////////////////////////////////////////////////////////////////////
PKTEST_GROUP( ManifestTrackTests )
{
    ////////////////////////////////////
    PKTEST_METHOD( Instantiation )
    {
        AutoRefPtr<CManifestTrack> apFixture;

        PKTEST_HRESULT_EXIT( CreateManifestTrack( apFixture.DerefOutPtr() ) );

        PKTEST_ASSERT_EXIT( apFixture != NULL );

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( SortByBitrate )
    {
        uint32 bitrates[] =         { 10, 30, 15, 3, 50, 1 };
        uint32 sortedBitrates[] =   { 1, 3, 10, 15, 30, 50 };

        static const size_t NUM_OF_TRACKS = sizeof(bitrates)/sizeof(bitrates[0]);

        //
        // Initialize the array of tracks
        //

        std::vector< AutoRefPtr<CManifestTrack> > tracks( NUM_OF_TRACKS );

        for( size_t i = 0; i < NUM_OF_TRACKS; ++i )
        {
            PKTEST_HRESULT_EXIT( CreateManifestTrack( tracks[i].DerefOutPtr() ) );

            tracks[i]->Bitrate() = bitrates[i];
        }

        //
        // Call the API to sort the array by bitrate
        //

        SortManifestTracksInAscendingBitrateOrder( &tracks );

        //
        // Check it was sorted
        //

        PKTEST_ASSERT_EXIT( tracks.size() == NUM_OF_TRACKS );

        for( size_t i = 0; i < NUM_OF_TRACKS; ++i )
        {
            if( i > 0 )
            {
                PKTEST_ASSERT_EXIT( sortedBitrates[i-1] < sortedBitrates[i] );
            }

            PKTEST_ASSERT_EXIT( sortedBitrates[i] == tracks[i]->Bitrate() );
        }

    exit:

        return;
    }

        ////////////////////////////////////
    PKTEST_METHOD( CustomAttributesTrack )
    {
        struct customAttributes 
        {
            const wchar_t* pszName;
            const wchar_t* pszValue;
        }
        static const rgData[] =
        {
            {L"foo1", L"bar1"},
            {L"foo2", L"bar2"},
            {L"foo3", L"bar3"},
            {L"foo4", L"bar4"},

        };

        static const size_t NUM_OF_CUSTOM_ATTRIBUTES = sizeof(rgData)/sizeof(rgData[0]);

        AutoRefPtr<CManifestTrack> apTrack;
        std::vector<std::wstring> names;

        //
        // Initialize the track
        //

        PKTEST_HRESULT_EXIT( CreateManifestTrack( apTrack.DerefOutPtr() ) );

        //
        // Call the API to add the custom attributes
        //

        for( size_t i=0; i < NUM_OF_CUSTOM_ATTRIBUTES; i++ )
        {
            apTrack->SetCustomAttribute( rgData[i].pszName,rgData[i].pszValue );
        }

        //
        // Try to get the custom attribute names
        //
        
        apTrack->GetCustomAttributeNames(&names);
        PKTEST_ASSERT_EXIT( names.size() == NUM_OF_CUSTOM_ATTRIBUTES );

        for( size_t i = 0; i < names.size(); i++ )
        {
            PKTEST_ASSERT_EXIT( names[i] == rgData[i].pszName );

            std::wstring value;
            apTrack->GetCustomAttributeValue(names[i],&value);

            PKTEST_ASSERT_EXIT( value == rgData[i].pszValue );
        }

    exit:

        return;
    }
    
    ////////////////////////////////////
    PKTEST_METHOD( SetAndGetManifestAttributes )
    {
        struct
        {
            const wchar_t* pszName;
            const wchar_t* pszValue;
        }
        static const rgEntries[] =
        {
            // Random made up attributes

            { L"speff",   L"wug" },
            { L"tulver",  L"blicket" },
            { L"gazzer",  L"dax" },
            { L"fem",     L"toma" },
            { L"fendle",  L"pimwit" },
            { L"tupa",    L"zav" },
        };

        AutoRefPtr<CManifestTrack> apTrack;

        //
        // Initialize the track
        //

        PKTEST_HRESULT_EXIT( CreateManifestTrack( apTrack.DerefOutPtr() ) );

        // Set all attributes
        for( int i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            PKTEST_HRESULT_EXIT( apTrack->SetAttribute( rgEntries[i].pszName, rgEntries[i].pszValue ) );
        }

        // Verify all attributes were set accordingly
        for( int i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            std::wstring val;

            PKTEST_HRESULT_EXIT(  apTrack->GetAttribute( rgEntries[i].pszName, &val ) );
            PKTEST_ASSERT_EXIT( val == rgEntries[i].pszValue );
        }

        // Set the same attribute again should result in an error
        for (int i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i)
        {
            PKTEST_ASSERT_EXIT( pkFAILED( apTrack->SetAttribute( rgEntries[i].pszName, rgEntries[i].pszValue ) ) )
        }

        // Verify unknown attributes return false
        for( int i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            std::wstring val;

            PKTEST_ASSERT_EXIT( false ==  apTrack->GetAttribute( std::wstring(rgEntries[i].pszName) + L"0", &val ) );
        }

    exit:
        return;
    }
};
