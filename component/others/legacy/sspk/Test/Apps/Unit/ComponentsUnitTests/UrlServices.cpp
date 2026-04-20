///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <PKTestSuite.h>

#include <IManifestTrack.h>
#include <SSPKDefines.h>
#include <ManifestChunk.h>
#include <CManifestTrack.h>
#include <MbrLocalInterfaces.h> // for IManifestUrlServices
#include <vector>
#include <StringUtils.h>

////////////////////////////////////////////////////////////////////////////////
// Example of a Test Group
//
PKTEST_GROUP( ManifestUrlServices )
{
    ////////////////////////////////////////
    PKTEST_METHOD( BasicManifestUrlServices )
    {
        enum Command
        {
            cmdResetFixture,
            cmdSetBaseUrl,
            cmdSetChunkStartTime,
            cmdSetBitrate,
            cmdSetCustomAttribute,
            cmdSetReferenceSegmentStartTime,
            cmdSetSegmentDuration,
            cmdSetSegmentUrlTemplate,
            cmdVerifySegmentedManifestUrl,
            cmdVerifyFormatUrl, 
        };

        struct
        {
            Command cmd;
            const wchar_t* pszValue;
            uint64 value;
        }
        static const rgCmds[] =
        {
            // ULR:1
            { cmdResetFixture, L"http://localhost/SSCSDK/Media.ism/Manifest", 0 },
            { cmdSetBaseUrl, L"QualityLevels({bitrate},{CustomAttributes})/Fragments(video={start time})", 0 },
            { cmdVerifyFormatUrl, L"http://localhost/SSCSDK/Media.ism/QualityLevels(0)/Fragments(video=0)", 0 },

            { cmdSetChunkStartTime, NULL, 12345 },
            { cmdVerifyFormatUrl, L"http://localhost/SSCSDK/Media.ism/QualityLevels(0)/Fragments(video=12345)", 0 },

            { cmdSetBitrate, NULL, 7890 },
            { cmdVerifyFormatUrl, L"http://localhost/SSCSDK/Media.ism/QualityLevels(7890)/Fragments(video=12345)", 0 },

            { cmdSetCustomAttribute, L"CameraAngle=Camera1", 0 },
            { cmdVerifyFormatUrl, L"http://localhost/SSCSDK/Media.ism/QualityLevels(7890,CameraAngle=Camera1)/Fragments(video=12345)", 0 },

            { cmdResetFixture, L"http://localhost/content.ism/Manifest?test=foo/bar1+bar2/**&v=40", 0 },
            { cmdSetBaseUrl, L"QualityLevels({bitrate})/Fragments(video={start time})", 0 },
            { cmdVerifyFormatUrl, L"http://localhost/content.ism/QualityLevels(0)/Fragments(video=0)", 0 },

            { cmdResetFixture, L"http://localhost/Content.ism/Manifest", 0 },
            { cmdSetBaseUrl, L"Segments({starttime})/QualityLevels({bitrate})/Fragments(video={start time})", 0 },

            { cmdSetReferenceSegmentStartTime, NULL, 0 },
            { cmdSetSegmentDuration, NULL, 1000 },
            { cmdVerifyFormatUrl, L"http://localhost/Content.ism/Segments(0)/QualityLevels(0)/Fragments(video=0)", 0 },

            { cmdSetChunkStartTime, NULL, 2500 },
            { cmdVerifyFormatUrl, L"http://localhost/Content.ism/Segments(2000)/QualityLevels(0)/Fragments(video=2500)", 0 },

            { cmdSetSegmentUrlTemplate, L"Segments({starttime})/manifest", 0 },
            { cmdVerifySegmentedManifestUrl, L"http://localhost/Content.ism/Segments(2000)/manifest", 0 },

            { cmdSetReferenceSegmentStartTime, NULL, 100 },
            { cmdVerifyFormatUrl, L"http://localhost/Content.ism/Segments(2100)/QualityLevels(0)/Fragments(video=2500)", 0 },
            { cmdVerifySegmentedManifestUrl, L"http://localhost/Content.ism/Segments(2100)/manifest", 0 },
        };

        AutoRefPtr<IManifestUrlServices> apFixture;
        std::wstring strBaseUrl;
        uint32 dwBitrate = 0;
        uint32 dwChunkIndex = 0;
        uint32 dwHardwareProfile = 0;
        uint64 chunkStartTime = 0;
        std::wstring strResult;
        AutoRefPtr<CManifestTrack> apTrack;
        std::vector<std::wstring> customAttribute;

        for( int i = 0; i < sizeof(rgCmds)/sizeof(rgCmds[0]); ++i )
        {
            switch( rgCmds[i].cmd )
            {
            case cmdResetFixture:

                apFixture.Release();
                apTrack.Release();

                PKTEST_HRESULT_EXIT( DefaultManifestUrlServices::CreateInstance( apFixture.DerefOutPtr() ) );
                PKTEST_HRESULT_EXIT( CreateManifestTrack( apTrack.DerefOutPtr()));

                apFixture->SetManifestUrl( rgCmds[i].pszValue );

                PKTEST_ASSERT_EXIT( apFixture->ManifestUrl() == rgCmds[i].pszValue );

                strBaseUrl.clear();
                strResult.clear();
                customAttribute.clear();
                dwBitrate = 0;
                dwChunkIndex = 0;
                dwHardwareProfile = 0;
                chunkStartTime = 0;
                break;

            case cmdSetBaseUrl:

                strBaseUrl = rgCmds[i].pszValue;
                break;

            case cmdSetChunkStartTime:

                chunkStartTime = rgCmds[i].value;
                break;

            case cmdSetBitrate:

                apTrack->Bitrate() = (uint32)rgCmds[i].value;
                break;

            case cmdSetCustomAttribute:
  
                wsplit(rgCmds[i].pszValue, customAttribute, L"=");
                PKTEST_ASSERT_EXIT(2 == customAttribute.size());
                apTrack->SetCustomAttribute(customAttribute[0].c_str(),customAttribute[1].c_str());
                break;

            case cmdSetReferenceSegmentStartTime:

                apFixture->SetSegmented( true );
                apFixture->SetReferenceSegmentStartTime( rgCmds[i].value );
                break;

            case cmdSetSegmentDuration:

                apFixture->SetSegmented( true );
                apFixture->SetSegmentDuration( rgCmds[i].value );
                break;

            case cmdSetSegmentUrlTemplate:

                apFixture->SetSegmented( true );
                apFixture->SetSegmentUrlTemplate( rgCmds[i].pszValue );
                break;

            case cmdVerifySegmentedManifestUrl:
                PKTEST_HRESULT_EXIT(
                        apFixture->FormatSegmentManifestURL(
                                            chunkStartTime,
                                            &strResult
                                            ) );

                PKTEST_ASSERT_MSG_EXIT(
                        strResult == rgCmds[i].pszValue,
                        "'%ls' != '%ls'",
                        strResult.c_str(),
                        rgCmds[i].pszValue );
                break;

            case cmdVerifyFormatUrl:

                PKTEST_HRESULT_EXIT(
                        apFixture->FormatURL(
                                        strBaseUrl.c_str(),
                                        apTrack,
                                        dwChunkIndex,
                                        dwHardwareProfile,
                                        chunkStartTime,
                                        &strResult
                                        ) );

                PKTEST_ASSERT_MSG_EXIT(
                        strResult == rgCmds[i].pszValue,
                        "'%ls' != '%ls'",
                        strResult.c_str(),
                        rgCmds[i].pszValue );
                        
                break;
            }
        }

    exit:

        return;
    }
};
