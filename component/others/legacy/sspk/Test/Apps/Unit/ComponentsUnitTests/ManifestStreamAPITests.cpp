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

#include <IManifestTrack.h>

#include "CManifestChunk.h"
#include "CMediaStreamDescription.h"

#include <algorithm>

using namespace MBR;

static const int64 c_dvrLenSec = 60;

//
// Unit tests for the new IManifestStream APIs
//

PKTEST_GROUP( ManifestStreamAPITests )
{
    ////////////////////////////////////
    PKTEST_METHOD( Instantiation )
    {
        AutoRefPtr<CManifestStream> apManifestStream;
        AutoRefPtr<CMediaStreamDescription> apMSD;

        PKTEST_HRESULT_EXIT( CManifestStream::CreateInstance( apManifestStream.DerefOutPtr() ) );
        PKTEST_ASSERT_EXIT( NULL != apManifestStream );

        PKTEST_HRESULT_EXIT( CMediaStreamDescription::CreateInstance( apMSD.DerefOutPtr() ) );
        PKTEST_ASSERT_EXIT( NULL != apMSD );

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

        AutoRefPtr<CManifestStream> apManifestStream;
        AutoRefPtr<CMediaStreamDescription> apMSD;

        PKTEST_HRESULT_EXIT( CManifestStream::CreateInstance( apManifestStream.DerefOutPtr() ) );
        PKTEST_ASSERT_EXIT( NULL != apManifestStream );

        PKTEST_HRESULT_EXIT( CMediaStreamDescription::CreateInstance( apMSD.DerefOutPtr() ) );
        PKTEST_ASSERT_EXIT( NULL != apMSD );

        // Set all attributes
        for( int i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            PKTEST_HRESULT_EXIT( apManifestStream->SetAttribute( rgEntries[i].pszName, rgEntries[i].pszValue ) );
            PKTEST_HRESULT_EXIT( apMSD->SetAttribute( rgEntries[i].pszName, rgEntries[i].pszValue ) );
        }

        // Verify all attributes were set accordingly
        for( int i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            std::wstring val;

            PKTEST_HRESULT_EXIT( apManifestStream->GetAttribute( rgEntries[i].pszName, &val ) );
            PKTEST_ASSERT_EXIT( val == rgEntries[i].pszValue );

            PKTEST_HRESULT_EXIT( apMSD->GetAttribute( rgEntries[i].pszName, &val ) );
            PKTEST_ASSERT_EXIT( val == rgEntries[i].pszValue );
        }

        // Set the same attribute again should result in an error
        for (int i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i)
        {
            PKTEST_ASSERT_EXIT( pkFAILED( apManifestStream->SetAttribute( rgEntries[i].pszName, rgEntries[i].pszValue ) ) )
            PKTEST_ASSERT_EXIT( pkFAILED( apMSD->SetAttribute( rgEntries[i].pszName, rgEntries[i].pszValue ) ) )
        }

        // Verify unknown attributes return false
        for( int i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            std::wstring val;

            PKTEST_ASSERT_EXIT( false == apManifestStream->GetAttribute( std::wstring(rgEntries[i].pszName) + L"0", &val ) );
            PKTEST_ASSERT_EXIT( false == apMSD->GetAttribute( std::wstring(rgEntries[i].pszName) + L"0", &val ) );

        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( MSD_AddChunks_ZeroDVR )
    {
        int64 lastTimeStamp = 0;
        AutoRefPtr<CMediaStreamDescription> apMSD;
        CChunkBuffer* pChunkBuffer =  CChunkBuffer::Create(2, true);

        PKTEST_ASSERT_EXIT( NULL != pChunkBuffer );
        PKTEST_HRESULT_EXIT( CMediaStreamDescription::CreateInstance(apMSD.DerefOutPtr()) );

        pChunkBuffer->SetDefaultChunkDuration(1);
        apMSD->UnitTest_SetChunkBuffer(pChunkBuffer);

        // Fill up chunk buffer to size-1
        for(int32 i = 0; i < pChunkBuffer->Size()-1; ++i )
        {
            PKTEST_ASSERT_EXIT( NULL != apMSD->AddChunk(i));
            lastTimeStamp = i;
        }
        
        apMSD->SetDVRWindowLength(0);

        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMinChunkIndex()) == 0); 
        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMaxChunkIndex()) == lastTimeStamp); 
        PKTEST_ASSERT_EXIT(apMSD->GetDVRCount() == pChunkBuffer->Size() - 1);
        PKTEST_ASSERT_EXIT(apMSD->TimeRemainingInDVR() > TimeSpan_hns::FromTicks(0));

        PKTEST_ASSERT_EXIT( NULL != apMSD->AddChunk(++lastTimeStamp));
        apMSD->SetLastChunkDuration(1);

        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMinChunkIndex()) == 0); 
        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMaxChunkIndex()) == lastTimeStamp); 
        PKTEST_ASSERT_EXIT(apMSD->GetDVRCount() == pChunkBuffer->Size());
        PKTEST_ASSERT_EXIT(apMSD->TimeRemainingInDVR() == TimeSpan_hns::FromTicks(0));

        PKTEST_ASSERT_EXIT( NULL != apMSD->AddChunk(++lastTimeStamp));
        apMSD->SetLastChunkDuration(1);

        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMinChunkIndex()) == 1); 
        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMaxChunkIndex()) == lastTimeStamp); 
        PKTEST_ASSERT_EXIT(apMSD->GetDVRCount() == pChunkBuffer->Size());
        PKTEST_ASSERT_EXIT(apMSD->TimeRemainingInDVR() == TimeSpan_hns::FromTicks(0));

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( MSD_AddChunks_DVR )
    {
        int64 lastTimeStamp = 0;
        AutoRefPtr<CMediaStreamDescription> apMSD;
        CChunkBuffer* pChunkBuffer =  CChunkBuffer::Create(2, true);

        PKTEST_ASSERT_EXIT( NULL != pChunkBuffer );
        PKTEST_HRESULT_EXIT( CMediaStreamDescription::CreateInstance(apMSD.DerefOutPtr()) );

        pChunkBuffer->SetDefaultChunkDuration(MBR_DEFAULT_TIMESCALE);
        apMSD->UnitTest_SetChunkBuffer(pChunkBuffer);

        // Fill up entire chunk buffer
        for(int64 i = 0; i < pChunkBuffer->Size(); ++i )
        {
            PKTEST_ASSERT_EXIT( NULL != apMSD->AddChunk(i*MBR_DEFAULT_TIMESCALE));
            lastTimeStamp = i*MBR_DEFAULT_TIMESCALE;
        }
        
        apMSD->SetDVRWindowLength(c_dvrLenSec);
        apMSD->SetLastChunkDuration(MBR_DEFAULT_TIMESCALE);

        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMaxChunkIndex()) == lastTimeStamp); 
        PKTEST_ASSERT_EXIT(apMSD->GetChunkEndPosition(apMSD->GetDVRMaxChunkIndex()) - apMSD->GetChunkStartPosition(apMSD->GetDVRMinChunkIndex()) == c_dvrLenSec*MBR_DEFAULT_TIMESCALE); 
        PKTEST_ASSERT_EXIT(apMSD->GetDVRCount() == c_dvrLenSec);
        PKTEST_ASSERT_EXIT(apMSD->TimeRemainingInDVR() == TimeSpan_hns::FromTicks(0));

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( MSD_AddChunks_BackVector )
    {
        AutoRefPtr<CMediaStreamDescription> apMSD;
        int64 lastTimeStamp = 0;
        std::vector<SChunkInfo> reverseTimeVector;
        CChunkBuffer* pChunkBuffer =  CChunkBuffer::Create(2, true);

        PKTEST_ASSERT_EXIT( NULL != pChunkBuffer );
        PKTEST_HRESULT_EXIT( CMediaStreamDescription::CreateInstance(apMSD.DerefOutPtr()) );

        pChunkBuffer->SetDefaultChunkDuration(MBR_DEFAULT_TIMESCALE);
        apMSD->UnitTest_SetChunkBuffer(pChunkBuffer);

        static const int64 c_baseTime = (int64)pChunkBuffer->Size()*MBR_DEFAULT_TIMESCALE;

        // Fill up half of the chunk buffer from time 10800
        for(int64 i = 0; i < pChunkBuffer->Size()/2; ++i )
        {
            PKTEST_ASSERT_EXIT( NULL != apMSD->AddChunk(c_baseTime + i*MBR_DEFAULT_TIMESCALE));
            lastTimeStamp = c_baseTime + i*MBR_DEFAULT_TIMESCALE;
        }
        
        apMSD->SetLastChunkDuration(MBR_DEFAULT_TIMESCALE);

        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMinChunkIndex()) == c_baseTime); 
        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMaxChunkIndex()) == lastTimeStamp); 

        // create reverse timestamps
        for(int64 j = 0; j < pChunkBuffer->Size()/2; ++j )
        {
            SChunkInfo chunkInfo(c_baseTime/2 + j*MBR_DEFAULT_TIMESCALE);
            reverseTimeVector.push_back(chunkInfo);
        }

        PKTEST_ASSERT_EXIT( apMSD->AddChunks(reverseTimeVector, eBufferDirection_Backward));

        // Confirm that the DVR window is unchanged
        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMinChunkIndex()) == c_baseTime); 
        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMaxChunkIndex()) == lastTimeStamp); 
        PKTEST_ASSERT_EXIT(apMSD->GetDVRCount() == pChunkBuffer->Size()/2);

        // Set the DVR left edge
        PKTEST_ASSERT_EXIT(apMSD->SetDVRMinTime(TimeSpan_hns::FromTicks(reverseTimeVector[0].timestamp)));
        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMinChunkIndex()) == reverseTimeVector[0].timestamp); 
        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMaxChunkIndex()) == lastTimeStamp); 
        PKTEST_ASSERT_EXIT(apMSD->GetDVRCount() == pChunkBuffer->Size());

        // Try to set to an invalid timestamp
        PKTEST_ASSERT_EXIT(!apMSD->SetDVRMinTime(TimeSpan_hns::FromTicks(reverseTimeVector[0].timestamp-1)));

        // Set the DVR left edge to a later time
        PKTEST_ASSERT_EXIT(apMSD->SetDVRMinTime(TimeSpan_hns::FromTicks(reverseTimeVector[0].timestamp + 10 * MBR_DEFAULT_TIMESCALE)));
        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMinChunkIndex()) == reverseTimeVector[0].timestamp + 10 * MBR_DEFAULT_TIMESCALE); 
        PKTEST_ASSERT_EXIT(apMSD->GetChunkStartPosition(apMSD->GetDVRMaxChunkIndex()) == lastTimeStamp); 
        PKTEST_ASSERT_EXIT(apMSD->GetDVRCount() == pChunkBuffer->Size() - 10);

    exit:
        return;
    }
    ////////////////////////////////////

    PKTEST_METHOD( AddAndVerifyAvailableTracks )
    {
        /// TODO: To be implemented

        PKTEST_ASSERT_EXIT( true);

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ChangeAndVerifySelectedTracks )
    {
        /// TODO: To be implemented

        PKTEST_ASSERT_EXIT( true);

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( InvalidSelectionForSelectedTracks )
    {
        /// TODO: To be implemented

        PKTEST_ASSERT_EXIT( true);

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( RestrickTracksAndVerifyAvailableAndSelectedTracks )
    {
        /// TODO: To be implemented

        PKTEST_ASSERT_EXIT( true);

    exit:

        return;
    }
};
