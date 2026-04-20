///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSPKDefines.h"
#include "PKTestSuite.h"
#include "PKTestSuiteUtils.h"

#include "CManifestTrack.h"             // Components/MBR
#include "CManifestChunk.h"             // Components/MBR
#include "ManifestParser.h"             // Components/MBR
#include "CMediaStreamDescription.h"
#include "CMbrConfiguration.h"

#include "StringUtils.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
PKTEST_GROUP( ManifestParserTests )
{
    ////////////////////////////////////
    PKTEST_METHOD( InstantiateParser )
    {
        AutoRefPtr<MBR::CChunkManifest> apManifest;
        MBR::CManifestParser fixture( apManifest );
    }

    struct Attributes
    {
        const wchar_t* pszName;
        const wchar_t* pszValue;
    };

    static
    void
    DumpManifestToTextFile(
        _In_ CTextFileWriter& wrt,
        _In_ IManifest* pManifest );

    static
    void
    CommonParseManifestFromFile(
        _In_ const wchar_t* pszFileName,
        _Out_ MBR::CChunkManifest** ppManifest );
    
    static
    void
    SubsequentParseManifestFromFile(
        _In_ const wchar_t* pszFileName,
        _In_ MBR::CChunkManifest* pManifest );

    static
    void
    VerifyRelationships(
        _In_ IManifest* pManifest );

    template< class storeT >
    static
    void
    VerifyAttributes(
        AutoRefPtr<storeT> storePtr,
        _In_count_(cEntries) const Attributes* pAttrib,
        _In_ size_t cEntries );

    static
    uint32
    Rand( uint32 nMax )
    {
        double d = std::rand();

        d = ( d / RAND_MAX ) * ( nMax - MIN( nMax, 1 ) );

        return( (uint32)d );
    }

    ////////////////////////////////////
    PKTEST_METHOD( ParseAndDumpManifests )
    {
        //
        // The intention of this test is to exercise the manifest parser and provide
        // a little bit of protection against regressions in its code.
        //
        // The input test file must follow this format:
        //
        //      <tables>
        //          <table name="DumpManifestFromOM">
        //              <manifest
        //                  src="<src-file>"
        //                  baseline="<baseline-file>"
        //                  output="<output-file>" />
        //              ...
        //              <manifest ... >
        //              ...
        //          </table>
        //      </tables>
        //
        //      Where:
        //          src-file        : path to ISMC file
        //          baseline-file   : [optional] path to the baseline dump file
        //          output-file     : path to the output dump file
        //
        // For all the manifests referenced in the test table:
        // - Using MBR::CManifestParser, parse the manifest to a MBR::CChunkManifest object.
        // - Then using MBR::CChunkManifest's IManifest interface, dump all the streams and
        //   tracks in the manifest to an output file
        // - Finally, if a baseline file is present, compare it to the output file and fail
        //   if they're different
        //

        CXMLElement xTestData = PKTest_GetDataTable( "DumpManifestFromOM" );
        CXMLElementsList xManifests;

        PKTEST_ASSERT_EXIT( !xTestData.IsNull() );

        xManifests = xTestData.Elements(L"manifest");

        for( int iManifest = 0; iManifest < xManifests.Length(); ++iManifest )
        {
            //
            // Parse the manifest
            //

            AutoRefPtr<MBR::CChunkManifest> apManifest;

            CXMLElement xManifest = xManifests[iManifest];
            CXMLAttribute xSrc = xManifest.Attributes()[L"src"];
            CXMLAttribute xOutput = xManifest.Attributes()[L"output"];
            CXMLAttribute xBaseline = xManifest.Attributes()[L"baseline"];

            PKTEST_ASSERT_EXIT( !xManifest.IsNull() );

            LogTestComment( "%ls", xSrc.Value() );

            PKTEST_FUNC_EXIT( CommonParseManifestFromFile( xSrc.Value(), apManifest.DerefOutPtr() ) );

            if( xOutput.IsNull() )
            {
                continue;
            }

            //
            // Dump the manifest to the output file
            //

            {
                CTextFileWriter wrt;

                PKTEST_HRESULT_EXIT( wrt.CreateAlways( wstring_to_string( xOutput.Value() ).c_str() ) );

                PKTEST_FUNC_EXIT( DumpManifestToTextFile( wrt, apManifest ) );

                wrt.Close();

                if( !xBaseline.IsNull() )
                {
                    PKTEST_ASSERT_EXIT(
                                    S_OK == FilesAreEqual(
                                                wstring_to_string( xOutput.Value() ).c_str(),
                                                wstring_to_string( xBaseline.Value() ).c_str()
                                                )
                                        );
                }
            }

            //
            // Verify Container/Contained relationships
            //

            PKTEST_FUNC_EXIT( VerifyRelationships( apManifest ) );
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( LiveToVODManifests )
    {
        CXMLElement xTestData = PKTest_GetDataTable( "LiveToVOD" );
        CXMLElementsList xManifests;

        PKTEST_ASSERT_EXIT( !xTestData.IsNull() );

        xManifests = xTestData.Elements(L"manifest");

        for( int iManifest = 0; iManifest < xManifests.Length(); ++iManifest )
        {
            //
            // Parse the manifest
            //

            AutoRefPtr<MBR::CChunkManifest> apManifest;
            std::vector< AutoRefPtr<IManifestStream> > availableStreams;
            std::vector< AutoRefPtr<IManifestTrack> > availableTracks;

            CXMLElement xManifest = xManifests[iManifest];
            CXMLAttribute xSrc = xManifest.Attributes()[L"src"];

            PKTEST_ASSERT_EXIT( !xManifest.IsNull() );

            LogTestComment( "%ls", xSrc.Value() );

            PKTEST_FUNC_EXIT( CommonParseManifestFromFile( xSrc.Value(), apManifest.DerefOutPtr() ) );

            //
            // Check the number of streams
            //

            apManifest->GetAvailableStreams( &availableStreams );

            //
            // Check the number of tracks
            //

            for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
            {
                availableStreams[iStrm]->GetAvailableTracks( &availableTracks );

                if (MediaStreamTypeVideo == availableStreams[iStrm]->Type())
                {
                    PKTEST_ASSERT_EXIT( availableTracks.size() == 6 );
                }
                else if (MediaStreamTypeAudio == availableStreams[iStrm]->Type())
                {
                    PKTEST_ASSERT_EXIT( availableTracks.size() == 1 );
                }
            }

            // check that there are only 2 streams
            PKTEST_ASSERT_EXIT( 2 == availableStreams.size() );

            // check if the segment information has been stored
            PKTEST_ASSERT_EXIT( apManifest->IsSegmented() );
            PKTEST_ASSERT_EXIT( !apManifest->SegmentUrlTemplate().empty() );
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( StartOverManifests )
    {
        CXMLElement xTestData = PKTest_GetDataTable( "StartOver" );
        CXMLElementsList xManifests;
        AutoRefPtr<MBR::CChunkManifest> apManifest;
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;

        PKTEST_ASSERT_EXIT( !xTestData.IsNull() );

        xManifests = xTestData.Elements(L"manifest");

        for( int iManifest = 0; iManifest < xManifests.Length(); ++iManifest )
        {
            //
            // Parse the manifest
            //
            CXMLElement xManifest = xManifests[iManifest];
            CXMLAttribute xSrc = xManifest.Attributes()[L"src"];

            PKTEST_ASSERT_EXIT( !xManifest.IsNull() );

            LogTestComment( "%ls", xSrc.Value() );

            if( 0 == iManifest)
            {
                PKTEST_FUNC_EXIT( CommonParseManifestFromFile( xSrc.Value(), apManifest.DerefOutPtr() ) );
            }
            else
            {
                PKTEST_FUNC_EXIT( SubsequentParseManifestFromFile( xSrc.Value(), apManifest ) );
            }

            apManifest->GetAvailableStreams( &availableStreams );

            for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
            {
                MBR::CMediaStreamDescription* pMSD = apManifest->GetStreamDescriptionByStream(availableStreams[iStrm]);
                availableStreams[iStrm]->GetAvailableTracks( &availableTracks );

                //
                // Check the left edge stays the same
                //
                PKTEST_ASSERT_EXIT( pMSD->GetDVRMinChunkIndex() == gMbrConfiguration.ChunklistMaxSize );
                
                //
                // Check the number of tracks, chunks in the dvr, right edge, and left start position don't change
                //
                if (MediaStreamTypeVideo == availableStreams[iStrm]->Type())
                {
                    PKTEST_ASSERT_EXIT( availableTracks.size() == wideToUInt32(xManifests[0].Attributes()[L"videoTracks"].Value() ) );
                    PKTEST_ASSERT_EXIT( pMSD->GetDVRCount() == wideToUInt32(xManifests[0].Attributes()[L"videoChunks"].Value() ) );
                    PKTEST_ASSERT_EXIT( pMSD->GetDVRMaxChunkIndex() == (gMbrConfiguration.ChunklistMaxSize + wideToUInt32(xManifests[0].Attributes()[L"videoChunks"].Value()) - 1) );
                    PKTEST_ASSERT_EXIT( pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex()) == wideToUInt64(xManifests[0].Attributes()[L"videoStartPos"].Value() ) );
                }
                else if (MediaStreamTypeAudio == availableStreams[iStrm]->Type())
                {
                    PKTEST_ASSERT_EXIT( availableTracks.size() == wideToUInt32(xManifests[0].Attributes()[L"audioTracks"].Value() ) );
                    PKTEST_ASSERT_EXIT( pMSD->GetDVRCount() == wideToUInt32(xManifests[0].Attributes()[L"audioChunks"].Value() ) );
                    PKTEST_ASSERT_EXIT( pMSD->GetDVRMaxChunkIndex() == (gMbrConfiguration.ChunklistMaxSize + wideToUInt32(xManifests[0].Attributes()[L"audioChunks"].Value()) - 1) );
                    PKTEST_ASSERT_EXIT( pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex()) == ((pMSD->GetStreamID() == 2) 
                                                                                                ? wideToUInt64(xManifests[0].Attributes()[L"audio1StartPos"].Value() ) 
                                                                                                : wideToUInt64(xManifests[0].Attributes()[L"audio2StartPos"].Value() )));
                }
                else if (MediaStreamTypeText == availableStreams[iStrm]->Type())
                {
                    PKTEST_ASSERT_EXIT( availableTracks.size() == wideToUInt32(xManifests[0].Attributes()[L"textTracks"].Value() ) );
                    PKTEST_ASSERT_EXIT( pMSD->GetDVRCount() == wideToUInt32(xManifests[0].Attributes()[L"textChunks"].Value() ) );
                    PKTEST_ASSERT_EXIT( pMSD->GetDVRMaxChunkIndex() == (gMbrConfiguration.ChunklistMaxSize + wideToUInt32(xManifests[0].Attributes()[L"textChunks"].Value()) - 1) );
                    PKTEST_ASSERT_EXIT( pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex()) == wideToUInt64(xManifests[0].Attributes()[L"textStartPos"].Value() ) );
                }
            }

            PKTEST_ASSERT_EXIT( wideToUInt32(xManifests[0].Attributes()[L"totalStreams"].Value() ) == availableStreams.size() );

            // check if the segment information has been stored
            PKTEST_ASSERT_EXIT( apManifest->IsSegmented() );
            PKTEST_ASSERT_EXIT( !apManifest->SegmentUrlTemplate().empty() );
        }

        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            MBR::CMediaStreamDescription* pMSD = apManifest->GetStreamDescriptionByStream(availableStreams[iStrm]);

            //
            // Set the left edge
            //
            PKTEST_ASSERT_EXIT( pMSD->SetDVRMinTime(TimeSpan_hns::FromTicks(wideToUInt64(xManifests[1].Attributes()[L"videoStartPos"].Value()))) );
                
            //
            // Check that the dvr count has increased, the right edge has stayed the same, the left edge start
            // time has changed, left edge has moved back
            //
            if (MediaStreamTypeVideo == availableStreams[iStrm]->Type())
            {
                PKTEST_ASSERT_EXIT( pMSD->GetDVRMinChunkIndex() == gMbrConfiguration.ChunklistMaxSize - wideToUInt32(xManifests[1].Attributes()[L"videoChunks"].Value() ) );
                PKTEST_ASSERT_EXIT( pMSD->GetDVRCount() == wideToUInt32(xManifests[0].Attributes()[L"videoChunks"].Value()) 
                                                            + wideToUInt32(xManifests[1].Attributes()[L"videoChunks"].Value()) );

                PKTEST_ASSERT_EXIT( pMSD->GetDVRMaxChunkIndex() == (gMbrConfiguration.ChunklistMaxSize 
                                                            + wideToUInt32(xManifests[0].Attributes()[L"videoChunks"].Value()) - 1) );

                PKTEST_ASSERT_EXIT( pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex()) == wideToUInt64(xManifests[1].Attributes()[L"videoStartPos"].Value() ) );
            }
            else if (MediaStreamTypeAudio == availableStreams[iStrm]->Type())
            {
                PKTEST_ASSERT_EXIT( pMSD->GetDVRMinChunkIndex() == gMbrConfiguration.ChunklistMaxSize - wideToUInt32(xManifests[1].Attributes()[L"audioChunks"].Value() ) );
                PKTEST_ASSERT_EXIT( pMSD->GetDVRCount() == wideToUInt32(xManifests[0].Attributes()[L"audioChunks"].Value()) 
                                                            + wideToUInt32(xManifests[1].Attributes()[L"audioChunks"].Value()) );

                PKTEST_ASSERT_EXIT( pMSD->GetDVRMaxChunkIndex() == (gMbrConfiguration.ChunklistMaxSize 
                                                            + wideToUInt32(xManifests[0].Attributes()[L"audioChunks"].Value()) - 1) );

                PKTEST_ASSERT_EXIT( pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex()) == ((pMSD->GetStreamID() == 2) 
                                                                                            ? wideToUInt64(xManifests[1].Attributes()[L"audio1StartPos"].Value() ) 
                                                                                            : wideToUInt64(xManifests[1].Attributes()[L"audio2StartPos"].Value() )));
            }
            else if (MediaStreamTypeText == availableStreams[iStrm]->Type())
            {
                PKTEST_ASSERT_EXIT( pMSD->GetDVRMinChunkIndex() == gMbrConfiguration.ChunklistMaxSize - wideToUInt32(xManifests[1].Attributes()[L"textChunks"].Value() ) );
                PKTEST_ASSERT_EXIT( pMSD->GetDVRCount() == wideToUInt32(xManifests[0].Attributes()[L"textChunks"].Value()) 
                                                            + wideToUInt32(xManifests[1].Attributes()[L"textChunks"].Value()) );

                PKTEST_ASSERT_EXIT( pMSD->GetDVRMaxChunkIndex() == (gMbrConfiguration.ChunklistMaxSize 
                                                            + wideToUInt32(xManifests[0].Attributes()[L"textChunks"].Value()) - 1) );

                PKTEST_ASSERT_EXIT( pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex()) == wideToUInt64(xManifests[1].Attributes()[L"textStartPos"].Value() ) );
            }
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( StartOverSparseManifests )
    {
        CXMLElement xTestData = PKTest_GetDataTable( "StartOverSparse" );
        CXMLElementsList xManifests;
        AutoRefPtr<MBR::CChunkManifest> apManifest;
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;

        PKTEST_ASSERT_EXIT( !xTestData.IsNull() );

        xManifests = xTestData.Elements(L"manifest");

        for( int iManifest = 0; iManifest < xManifests.Length(); ++iManifest )
        {
            //
            // Parse the manifest
            //
            CXMLElement xManifest = xManifests[iManifest];
            CXMLAttribute xSrc = xManifest.Attributes()[L"src"];

            PKTEST_ASSERT_EXIT( !xManifest.IsNull() );

            LogTestComment( "%ls", xSrc.Value() );

            if( 0 == iManifest)
            {
                PKTEST_FUNC_EXIT( CommonParseManifestFromFile( xSrc.Value(), apManifest.DerefOutPtr() ) );
            }
            else
            {
                PKTEST_FUNC_EXIT( SubsequentParseManifestFromFile( xSrc.Value(), apManifest ) );
            }

            apManifest->GetAvailableStreams( &availableStreams );

            for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
            {
                MBR::CMediaStreamDescription* pMSD = apManifest->GetStreamDescriptionByStream(availableStreams[iStrm]);
                availableStreams[iStrm]->GetAvailableTracks( &availableTracks );
                                
                //
                // Check the left edge stays the same
                //
                PKTEST_ASSERT_EXIT( pMSD->GetDVRMinChunkIndex() == gMbrConfiguration.ChunklistMaxSize );

                //
                // Check the number of tracks, chunks in the dvr, right edge, and left start position don't change
                //
                if (MediaStreamTypeVideo == availableStreams[iStrm]->Type())
                {
                    PKTEST_ASSERT_EXIT( availableTracks.size() == wideToUInt32(xManifests[0].Attributes()[L"videoTracks"].Value() ) );
                    PKTEST_ASSERT_EXIT( pMSD->GetDVRCount() == wideToUInt32(xManifests[0].Attributes()[L"videoChunks"].Value() ) );
                    PKTEST_ASSERT_EXIT( pMSD->GetDVRMaxChunkIndex() == (gMbrConfiguration.ChunklistMaxSize + wideToUInt32(xManifests[0].Attributes()[L"videoChunks"].Value()) - 1) );
                    PKTEST_ASSERT_EXIT( pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex()) == wideToUInt64(xManifests[0].Attributes()[L"videoStartPos"].Value() ) );
                }
                else if (MediaStreamTypeAudio == availableStreams[iStrm]->Type())
                {
                    PKTEST_ASSERT_EXIT( availableTracks.size() == wideToUInt32(xManifests[0].Attributes()[L"audioTracks"].Value() ) );
                    PKTEST_ASSERT_EXIT( pMSD->GetDVRCount() == wideToUInt32(xManifests[0].Attributes()[L"audioChunks"].Value() ) );
                    PKTEST_ASSERT_EXIT( pMSD->GetDVRMaxChunkIndex() == (gMbrConfiguration.ChunklistMaxSize + wideToUInt32(xManifests[0].Attributes()[L"audioChunks"].Value()) - 1) );
                    PKTEST_ASSERT_EXIT( pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex()) == wideToUInt64(xManifests[0].Attributes()[L"audioStartPos"].Value() ) );
                }
                else if (MediaStreamTypeText == availableStreams[iStrm]->Type())
                {
                    PKTEST_ASSERT_EXIT( availableTracks.size() == wideToUInt32(xManifests[0].Attributes()[L"textTracks"].Value() ) );
                    PKTEST_ASSERT_EXIT( pMSD->GetDVRCount() == wideToUInt32(xManifests[0].Attributes()[L"textChunks"].Value() ) );
                    PKTEST_ASSERT_EXIT( pMSD->GetDVRMaxChunkIndex() == (gMbrConfiguration.ChunklistMaxSize + wideToUInt32(xManifests[0].Attributes()[L"textChunks"].Value()) - 1) );
                    PKTEST_ASSERT_EXIT( pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex()) == wideToUInt64(xManifests[0].Attributes()[L"textStartPos"].Value() ) );
                }
            }

            PKTEST_ASSERT_EXIT( 3 == availableStreams.size() );

            // check if the segment information has been stored
            PKTEST_ASSERT_EXIT( apManifest->IsSegmented() );
            PKTEST_ASSERT_EXIT( !apManifest->SegmentUrlTemplate().empty() );
        }

        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            MBR::CMediaStreamDescription* pMSD = apManifest->GetStreamDescriptionByStream(availableStreams[iStrm]);

            //
            // Set the left edge
            //
            PKTEST_ASSERT_EXIT( pMSD->SetDVRMinTime(TimeSpan_hns::FromTicks(wideToUInt64(xManifests[1].Attributes()[L"audioStartPos"].Value()))) );
            
            //
            // Check that the dvr count has increased, the right edge has stayed the same, the left edge and 
            // left edge start time has changed
            //
            if (MediaStreamTypeVideo == availableStreams[iStrm]->Type())
            {
                PKTEST_ASSERT_EXIT( pMSD->GetDVRMinChunkIndex() == gMbrConfiguration.ChunklistMaxSize - wideToUInt32(xManifests[1].Attributes()[L"videoChunks"].Value() ) );
                PKTEST_ASSERT_EXIT( pMSD->GetDVRCount() == wideToUInt32(xManifests[0].Attributes()[L"videoChunks"].Value()) 
                                                            + wideToUInt32(xManifests[1].Attributes()[L"videoChunks"].Value()) );

                PKTEST_ASSERT_EXIT( pMSD->GetDVRMaxChunkIndex() == (gMbrConfiguration.ChunklistMaxSize 
                                                            + wideToUInt32(xManifests[0].Attributes()[L"videoChunks"].Value()) - 1) );

                PKTEST_ASSERT_EXIT( pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex()) == wideToUInt64(xManifests[1].Attributes()[L"videoStartPos"].Value() ) );
            }
            else if (MediaStreamTypeAudio == availableStreams[iStrm]->Type())
            {
                PKTEST_ASSERT_EXIT( pMSD->GetDVRMinChunkIndex() == gMbrConfiguration.ChunklistMaxSize - wideToUInt32(xManifests[1].Attributes()[L"audioChunks"].Value() ) );
                PKTEST_ASSERT_EXIT( pMSD->GetDVRCount() == wideToUInt32(xManifests[0].Attributes()[L"audioChunks"].Value()) 
                                                            + wideToUInt32(xManifests[1].Attributes()[L"audioChunks"].Value()) );

                PKTEST_ASSERT_EXIT( pMSD->GetDVRMaxChunkIndex() == (gMbrConfiguration.ChunklistMaxSize 
                                                            + wideToUInt32(xManifests[0].Attributes()[L"audioChunks"].Value()) - 1) );

                PKTEST_ASSERT_EXIT( pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex()) == wideToUInt64(xManifests[1].Attributes()[L"audioStartPos"].Value() ));
            }
            else if (MediaStreamTypeText == availableStreams[iStrm]->Type())
            {
                PKTEST_ASSERT_EXIT( pMSD->GetDVRMinChunkIndex() == gMbrConfiguration.ChunklistMaxSize - wideToUInt32(xManifests[1].Attributes()[L"textChunks"].Value() ) );
                PKTEST_ASSERT_EXIT( pMSD->GetDVRCount() == wideToUInt32(xManifests[0].Attributes()[L"textChunks"].Value()) 
                                                            + wideToUInt32(xManifests[1].Attributes()[L"textChunks"].Value()) );

                PKTEST_ASSERT_EXIT( pMSD->GetDVRMaxChunkIndex() == (gMbrConfiguration.ChunklistMaxSize 
                                                            + wideToUInt32(xManifests[0].Attributes()[L"textChunks"].Value()) - 1) );

                PKTEST_ASSERT_EXIT( pMSD->GetChunkStartPosition(pMSD->GetDVRMinChunkIndex()) == wideToUInt64(xManifests[1].Attributes()[L"textStartPos"].Value() ) );
            }
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( RestrictTracksAtManifestLevel )
    {
        bool fRestrictionToInvalidTrackTested = false;
        bool fRestrictionToSubsetTested = false;
        bool fRestrictionToEmptyListTested = false;

        CXMLElement xTestData = PKTest_GetDataTable( "TrackRestriction" );
        CXMLElementsList xManifests;

        PKTEST_ASSERT_EXIT( !xTestData.IsNull() );

        xManifests = xTestData.Elements(L"manifest");

        for( int iManifest = 0; iManifest < xManifests.Length(); ++iManifest )
        {
            //
            // Parse the manifest
            //

            AutoRefPtr<MBR::CChunkManifest> apManifest;
            std::vector< AutoRefPtr<IManifestStream> > availableStreams;
            std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
            std::vector< AutoRefPtr<IManifestTrack> > restrictedTracks;
            std::vector< AutoRefPtr<IManifestTrack> > shuffledRestrictedTracks;

            CXMLElement xManifest = xManifests[iManifest];
            CXMLAttribute xSrc = xManifest.Attributes()[L"src"];

            PKTEST_ASSERT_EXIT( !xManifest.IsNull() );

            LogTestComment( "%ls", xSrc.Value() );

            PKTEST_FUNC_EXIT( CommonParseManifestFromFile( xSrc.Value(), apManifest.DerefOutPtr() ) );

            //
            // Get the list of streams to test track restriction
            //

            apManifest->GetAvailableStreams( &availableStreams );

            //
            // Reflexive: restricting to the existing set results in the same set
            //

            for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
            {
                availableStreams[iStrm]->GetAvailableTracks( &availableTracks );

                shuffledRestrictedTracks = restrictedTracks = availableTracks;

                std::random_shuffle( shuffledRestrictedTracks.begin(), shuffledRestrictedTracks.end() );

                PKTEST_HRESULT_EXIT( availableStreams[iStrm]->RestrictTracks( shuffledRestrictedTracks ) );

                availableStreams[iStrm]->GetAvailableTracks( &availableTracks );

                PKTEST_ASSERT_EXIT( availableTracks == restrictedTracks );
            }

            //
            // Trying to restrict to a track that is not in the current list must fail
            //

            for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
            {
                availableStreams[iStrm]->GetAvailableTracks( &availableTracks );

                shuffledRestrictedTracks = restrictedTracks = availableTracks;

                // Remove one track from the list

                shuffledRestrictedTracks.erase( shuffledRestrictedTracks.begin() + Rand( shuffledRestrictedTracks.size() ) );

                PKTEST_ASSERT_EXIT( shuffledRestrictedTracks.size() == restrictedTracks.size() - 1 );

                if( shuffledRestrictedTracks.size() == 0 )
                {
                    // Restricting to an empty list should fail
                    PKTEST_ASSERT_EXIT( FAILED( availableStreams[iStrm]->RestrictTracks( shuffledRestrictedTracks ) ) );

                    fRestrictionToEmptyListTested = true;

                    continue;
                }

                // Restrict to that list

                PKTEST_HRESULT_EXIT( availableStreams[iStrm]->RestrictTracks( shuffledRestrictedTracks ) );

                availableStreams[iStrm]->GetAvailableTracks( &availableTracks );

                PKTEST_ASSERT_EXIT( availableTracks == shuffledRestrictedTracks );

                // Now try to restrict to the list that still has the track that was initially
                // removed. It must fail.

                PKTEST_ASSERT_EXIT( FAILED( availableStreams[iStrm]->RestrictTracks( restrictedTracks ) ) );

                availableStreams[iStrm]->GetAvailableTracks( &availableTracks );

                PKTEST_ASSERT_EXIT( availableTracks == shuffledRestrictedTracks );

                fRestrictionToInvalidTrackTested = true;
            }

            //
            // Restric the tracks to a subset of the available tracks
            //

            for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
            {
                //
                // Repeat a few times for the same stream, each time removing a few tracks
                //

                for( int j = Rand(8); j < 8; ++j )
                {
                    availableStreams[iStrm]->GetAvailableTracks( &availableTracks );

                    restrictedTracks = availableTracks;

                    for( int k = Rand( restrictedTracks.size() ); k > 0; --k )
                    {
                        int iTrackToRemove = Rand( restrictedTracks.size() );

                        restrictedTracks.erase( restrictedTracks.begin() + iTrackToRemove );
                    }

                    shuffledRestrictedTracks = restrictedTracks;

                    std::random_shuffle( shuffledRestrictedTracks.begin(), shuffledRestrictedTracks.end() );

                    if( shuffledRestrictedTracks.size() == 0 )
                    {
                        // Restricting to an empty list should fail
                        PKTEST_ASSERT_EXIT( FAILED( availableStreams[iStrm]->RestrictTracks( shuffledRestrictedTracks ) ) );

                        fRestrictionToEmptyListTested = true;

                        break;
                    }

                    PKTEST_HRESULT_EXIT( availableStreams[iStrm]->RestrictTracks( shuffledRestrictedTracks ) );

                    availableStreams[iStrm]->GetAvailableTracks( &availableTracks );

                    PKTEST_ASSERT_EXIT( availableTracks == restrictedTracks );

                    fRestrictionToSubsetTested = true;
                }
            }
        }

        PKTEST_ASSERT_EXIT( fRestrictionToInvalidTrackTested && fRestrictionToSubsetTested && fRestrictionToEmptyListTested );

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( CustomAttributesManifests )
    {
        CXMLElement xTestData = PKTest_GetDataTable( "CustomAttributes" );
        CXMLElementsList xManifests;

        PKTEST_ASSERT_EXIT( !xTestData.IsNull() );

        xManifests = xTestData.Elements(L"manifest");

        for( int iManifest = 0; iManifest < xManifests.Length(); ++iManifest )
        {
            //
            // Parse the manifest
            //

            AutoRefPtr<MBR::CChunkManifest> apManifest;
            std::vector< AutoRefPtr<IManifestStream> > availableStreams;
            std::vector< AutoRefPtr<IManifestTrack> > availableTracks;

            CXMLElement xManifest = xManifests[iManifest];
            CXMLAttribute xSrc = xManifest.Attributes()[L"src"];

            PKTEST_ASSERT_EXIT( !xManifest.IsNull() );

            LogTestComment( "%ls", xSrc.Value() );

            PKTEST_FUNC_EXIT( CommonParseManifestFromFile( xSrc.Value(), apManifest.DerefOutPtr() ) );


            apManifest->GetAvailableStreams( &availableStreams );

            //
            // Check the custom attributes
            //

            for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
            {
                if (MediaStreamTypeVideo == availableStreams[iStrm]->Type())
                {
                    availableStreams[iStrm]->GetAvailableTracks( &availableTracks );
                    for( size_t iTrk = 0; iTrk < availableTracks.size(); ++iTrk )
                    {
                        std::wstring value;
                        uint32 index = availableTracks[iTrk]->TrackIndex();

                        std::vector<std::wstring> names;
                        availableTracks[iTrk]->GetCustomAttributeNames(&names);

                        PKTEST_ASSERT_EXIT( names.size() == 1 );
                        PKTEST_ASSERT_EXIT( names[0] == L"CameraAngle" );

                        availableTracks[iTrk]->GetCustomAttributeValue(names[0],&value);                        
                        PKTEST_ASSERT_EXIT( value == (L"Camera" + toWString((index/10) + 1)) );
                    }
                }
            }
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( AttributesManifests )
    {
        static const Attributes ManifestAttrib[] =
        {
            { L"MajorVersion",              L"2" },
            { L"MinorVersion",              L"2" },
            { L"TimeScale",                 L"10000000" },
            { L"IsLive",                    L"TRUE" },
            { L"Duration",                  L"0" },
            { L"LookAheadFragmentCount",    L"30" },
            { L"DVRWindowLength",           L"36000000000"},
        };

        static const Attributes VideoStreamAttrib[] =
        {   
            { L"Type",          L"video" },
            { L"Name",          L"video" },
            { L"Subtype",       L"" },
            { L"Chunks",        L"0" },
            { L"TimeScale",     L"10000000" },
            { L"Url",           L"QualityLevels({bitrate})/Fragments(video={start time})" }
        };

        static const Attributes AudioStreamAttrib[] =
        {
            { L"Type",          L"audio" },
            { L"Name",          L"audio" },
            { L"Language",      L"eng" },
            { L"Subtype",       L"" },
            { L"Chunks",        L"0" },
            { L"TimeScale",     L"10000000" },
            { L"Url",           L"QualityLevels({bitrate})/Fragments(audio={start time})" }
        };

        static const Attributes TextStreamAttrib[] =
        {
            { L"Type",          L"text" },
            { L"Name",          L"captions" },
            { L"Language",      L"eng" },
            { L"Subtype",       L"CAPT" },
            { L"Chunks",        L"0" },
            { L"TimeScale",     L"10000000" },
            { L"Url",           L"QualityLevels({bitrate})/Fragments(captions={start time})" }
        };

        static const Attributes VideoTrackAttrib[3][6] =
        {   
            {
                { L"Index",             L"0" },
                { L"Bitrate",           L"300000" },
                { L"CodecPrivateData",  L"000000016742801e9656090ad80a0400000fa40003a98388800927000c987f18e0ed0a15700000000168ca8d48" },
                { L"FourCC",            L"AVC1" },
                { L"MaxWidth",          L"288" },
                { L"MaxHeight",         L"160" }
            },

            {
                { L"Index",             L"1" },
                { L"Bitrate",           L"1500000" },
                { L"CodecPrivateData",  L"00000001674d401f965602802dd80a0400000fa40003a983888002dc60003efc7f18e0ed0a15700000000168ca8d48" },
                { L"FourCC",            L"AVC1" },
                { L"MaxWidth",          L"1280" },
                { L"MaxHeight",         L"720" }
            },

            {
                { L"Index",             L"2" },
                { L"Bitrate",           L"800000" },
                { L"CodecPrivateData",  L"000000016742801e96560f047f580a0400000fa40003a9838880061a80008661fc6383b42855c00000000168ca8d48" },
                { L"FourCC",            L"AVC1" },
                { L"MaxWidth",          L"480" },
                { L"MaxHeight",         L"270" }
            },
        };

        static const Attributes AudioTrackAttrib[] =
        {
            { L"Index",             L"0" },
            { L"Bitrate",           L"64000" },
            { L"CodecPrivateData",  L"" },
            { L"FourCC",            L"AACL" },
            { L"AudioTag",          L"255" },
            { L"Channels",          L"2" },
            { L"SamplingRate",      L"48000" },
            { L"BitsPerSample",     L"16" },
            { L"PacketSize",        L"1"}
        };

        static const Attributes TextTrackAttrib[] =
        {
            { L"Index",             L"0" },
            { L"Bitrate",           L"1000" },
            { L"CodecPrivateData",  L"" },
            { L"FourCC",            L"TTML" }
        };

        CXMLElement xTestData = PKTest_GetDataTable( "Attributes" );
        CXMLElementsList xManifests;
        AutoRefPtr<MBR::CChunkManifest> apManifest;
        std::vector< AutoRefPtr<IManifestStream> > availableStreams;
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
        CXMLElement xManifest;
        CXMLAttribute xSrc;

        PKTEST_ASSERT_EXIT( !xTestData.IsNull() );
        xManifests = xTestData.Elements(L"manifest");

        xManifest = xManifests[0];
        xSrc = xManifest.Attributes()[L"src"];

        PKTEST_ASSERT_EXIT( !xManifest.IsNull() );

        LogTestComment( "%ls", xSrc.Value() );
        
        //
        // Parse the manifest
        //
        PKTEST_FUNC_EXIT( CommonParseManifestFromFile( xSrc.Value(), apManifest.DerefOutPtr() ) );

        //
        // Check manifest attributes
        //
        PKTEST_FUNC_EXIT( VerifyAttributes( apManifest, 
                                            ManifestAttrib, 
                                            sizeof(ManifestAttrib)/sizeof(ManifestAttrib[0]) ) );
            
        apManifest->GetAvailableStreams( &availableStreams );

        //
        // Check the stream and track attributes
        //   
        for( size_t iStrm = 0; iStrm < availableStreams.size(); ++iStrm )
        {
            const Attributes* pVerifyAttrib;
            size_t verifySize = 0;
            if( MediaStreamTypeVideo == availableStreams[iStrm]->Type() )
            {
                pVerifyAttrib = VideoStreamAttrib;
                verifySize = sizeof(VideoStreamAttrib)/sizeof(VideoStreamAttrib[0]);
            }
            else if( MediaStreamTypeAudio == availableStreams[iStrm]->Type() ) 
            {
                pVerifyAttrib = AudioStreamAttrib;
                verifySize = sizeof(AudioStreamAttrib)/sizeof(AudioStreamAttrib[0]);
            }
            else
            {
                pVerifyAttrib = TextStreamAttrib;
                verifySize = sizeof(TextStreamAttrib)/sizeof(TextStreamAttrib[0]);
            }

            PKTEST_FUNC_EXIT( VerifyAttributes( availableStreams[iStrm], 
                                                pVerifyAttrib, 
                                                verifySize ) );
                
            availableStreams[iStrm]->GetAvailableTracks( &availableTracks );
            for( size_t iTrk = 0; iTrk < availableTracks.size(); ++iTrk )
            {
                if ( MediaStreamTypeVideo == availableStreams[iStrm]->Type() )
                {
                    uint32 index = availableTracks[iTrk]->TrackIndex();
                    PKTEST_FUNC_EXIT( VerifyAttributes( availableTracks[iTrk],
                                                        VideoTrackAttrib[index],
                                                        sizeof(VideoTrackAttrib[index])/sizeof(VideoTrackAttrib[index][0]) ) );
                }
                else if( MediaStreamTypeAudio == availableStreams[iStrm]->Type() ) 
                {
                    PKTEST_FUNC_EXIT( VerifyAttributes( availableTracks[iTrk],
                                                        AudioTrackAttrib,
                                                        sizeof(AudioTrackAttrib)/sizeof(AudioTrackAttrib[0]) ) );
                }
                else
                {
                    PKTEST_FUNC_EXIT( VerifyAttributes( availableTracks[iTrk],
                                                        TextTrackAttrib,
                                                        sizeof(TextTrackAttrib)/sizeof(TextTrackAttrib[0]) ) );
                }
            }
        }
        

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( UTFEncoding )
    {
        CXMLElement xTestData = PKTest_GetDataTable( "UTFEncoding" );
        CXMLElementsList xManifests;

        PKTEST_ASSERT_EXIT( !xTestData.IsNull() );

        xManifests = xTestData.Elements(L"manifest");

        for( int iManifest = 0; iManifest < xManifests.Length(); ++iManifest )
        {
            //
            // Parse the manifest
            //

            AutoRefPtr<MBR::CChunkManifest> apManifest;
            CXMLElement xManifest = xManifests[iManifest];
            CXMLAttribute xSrc = xManifest.Attributes()[L"src"];
            CXmlParser::EEncodingType expectedEncodingType = (CXmlParser::EEncodingType)wideToUInt32(xManifests[iManifest].Attributes()[L"encodingType"].Value());
            CXmlParser::EEncodingType actualEncodingType = CXmlParser::eEncodingType_invalid;

            PKTEST_ASSERT_EXIT( !xManifest.IsNull() );

            LogTestComment( "%ls", xSrc.Value() );

            PKTEST_HRESULT_EXIT( MBR::CChunkManifest::CreateInstance( apManifest.DerefOutPtr() ));

            MBR::CManifestParser parser( apManifest );

            std::string strSrc = wstring_to_string( xSrc.Value() );

            CTextFileReader reader;

            PKTEST_HRESULT_EXIT( reader.OpenExisting( strSrc.c_str() ) );

            pkRESULT pkR = parser.Parse( reader.AsHandle(), reader.ReadCallback, MBR::eManifestMode_Initial, &actualEncodingType );

            //
            // Confirm the encoding type is correct
            //
            PKTEST_ASSERT_EXIT( actualEncodingType == expectedEncodingType );
        }

    exit:
        return;
    }
};

////////////////////////////////////////////////////////////////////////////////
/*static*/ void
ManifestParserTests::DumpManifestToTextFile(
    _In_ CTextFileWriter& wrt,
    _In_ IManifest* pManifest )
{
    std::vector< AutoRefPtr<IManifestStream> > availableStreams;
    std::vector< AutoRefPtr<IManifestTrack> > availableTracks;
    std::vector< AutoRefPtr<IManifestStream> > selectedStreams;

    std::vector< AutoRefPtr<IManifestStream> >::iterator itStream;
    std::vector< AutoRefPtr<IManifestTrack> >::iterator itTrack;

    int cStreams = 0;
    int cTracks = 0;

    //
    // Dump the manifest level attributes
    //

    PKTEST_HRESULT_EXIT(
        wrt.WriteFormat(
            "[ManifestLevel]\n"
            "   MajorVersion = %d\n"
            "   MinorVersion = %d\n"
            "      TimeScale = %lld\n"
            "       Duration = %lld\n"
            "        IsLive  = %d\n"
            "\n",
            pManifest->MajorVersion(),
            pManifest->MinorVersion(),
            pManifest->TimeScale(),
            pManifest->Duration(),
            pManifest->IsLive()
            ) );

    //
    // Dump the streams
    //

    pManifest->GetAvailableStreams( &availableStreams );
    pManifest->GetSelectedStreams( &selectedStreams );

    for( itStream = availableStreams.begin(); itStream != availableStreams.end(); ++itStream )
    {
        PKTEST_HRESULT_EXIT(
            wrt.WriteFormat(
                "[Stream_%d]\n"
                "       TimeScale = %llu\n"
                "        Language = %ls\n"
                "        MaxWidth = %d\n"
                "       MaxHeight = %d\n"
                "    DisplayWidth = %d\n"
                "   DisplayHeigth = %d\n",
                cStreams,
                (*itStream)->TimeScale(),
                (*itStream)->Language().c_str(),
                (*itStream)->MaxWidth(),
                (*itStream)->MaxHeight(),
                (*itStream)->DisplayWidth(),
                (*itStream)->DisplayHeight()
                ));

        PKTEST_HRESULT_EXIT(
            wrt.WriteFormat(
                "             Url = %ls\n"
                "            Name = %ls\n"
                "            Type = %d\n"
                "          SubTye = %ls\n"
                "     _IsSelected = %d\n"
                "\n",
                (*itStream)->Url().c_str(),
                (*itStream)->Name().c_str(),
                (*itStream)->Type(),
                (*itStream)->SubType().c_str(),
                std::find( selectedStreams.begin(), selectedStreams.end(), (*itStream) ) != selectedStreams.end()
                ));

        (*itStream)->GetAvailableTracks( &availableTracks );

        cTracks = 0;

        for( itTrack = availableTracks.begin(); itTrack != availableTracks.end(); ++itTrack )
        {
            std::string strCodecPrivateData;

            PKTEST_HRESULT_EXIT( BytesToHexStr(
                                    (*itTrack)->CodecPrivateData().data(),
                                    (*itTrack)->CodecPrivateData().size(),
                                    &strCodecPrivateData ) );
            PKTEST_HRESULT_EXIT(
                wrt.WriteFormat(
                    "[Track_%d_%d]\n"
                    "         TrackIndex = %d\n"
                    "            Bitrate = %d\n"
                    "     NominalBitrate = %d\n"
                    "           MaxWidth = %d\n"
                    "          MaxHeight = %d\n"
                    "    HardwareProfile = %d\n"
                    "      NALUnitLength = %d\n"
                    "           AudioTag = %d\n"
                    "             FourCC = 0x%x\n",
                    cStreams,
                    cTracks,
                    (*itTrack)->TrackIndex(),
                    (*itTrack)->Bitrate(),
                    (*itTrack)->NominalBitrate(),
                    (*itTrack)->MaxWidth(),
                    (*itTrack)->MaxHeight(),
                    (*itTrack)->HardwareProfile(),
                    (*itTrack)->NALUnitLength(),
                    (*itTrack)->AudioTag(),
                    (*itTrack)->FourCC()
                    ));

            PKTEST_HRESULT_EXIT(
                wrt.WriteFormat(
                    "   CodecPrivateData = %s\n"
                    "\n",
                    strCodecPrivateData.c_str()
                    ));

            ++cTracks;
        }

        ++cStreams;
    }

exit:

    return;
}

////////////////////////////////////////////////////////////////////////////////
/*static*/ void
ManifestParserTests::CommonParseManifestFromFile(
    _In_ const wchar_t* pszFileName,
    _Out_ MBR::CChunkManifest** ppManifest )
{
    *ppManifest = NULL;

    PKTEST_HRESULT_EXIT( MBR::CChunkManifest::CreateInstance( ppManifest ) );

    {
        MBR::CManifestParser parser( *ppManifest );

        std::string strSrc = wstring_to_string( pszFileName );

        CTextFileReader reader;
        PKTEST_HRESULT_EXIT( reader.OpenExisting( strSrc.c_str() ) );

        PKTEST_HRESULT_EXIT( parser.Parse( reader.AsHandle(), reader.ReadCallback ) );

        PKTEST_HRESULT_EXIT( (*ppManifest)->ValidateManifest() );

        PKTEST_HRESULT_EXIT( (*ppManifest)->SelectInitialStreams() );
    }

exit:
    return;
}

////////////////////////////////////////////////////////////////////////////////
/*static*/ void
ManifestParserTests::SubsequentParseManifestFromFile(
    _In_ const wchar_t* pszFileName,
    _In_ MBR::CChunkManifest* pManifest )
{
    
    MBR::CManifestParser parser( pManifest );

    std::string strSrc = wstring_to_string( pszFileName );

    CTextFileReader reader;
    PKTEST_HRESULT_EXIT( reader.OpenExisting( strSrc.c_str() ) );

    PKTEST_HRESULT_EXIT( parser.Parse( reader.AsHandle(), reader.ReadCallback, MBR::eManifestMode_Subsequent ) );

exit:
    return;
}

////////////////////////////////////////////////////////////////////////////////
/*static*/ void
ManifestParserTests::VerifyRelationships(
    _In_ IManifest* pManifest )
{
    std::vector< AutoRefPtr<IManifestStream> > availableStreams;

    std::vector< AutoRefPtr<IManifestStream> >::iterator itStream;
    std::vector< AutoRefPtr<IManifestStream> >::iterator itChildStream;
    std::vector< AutoRefPtr<IManifestTrack> >::iterator itTrack;

    pManifest->GetAvailableStreams( &availableStreams );

    //
    // Verify that parent -> child relationships are reflexive
    //

    for( itStream = availableStreams.begin(); itStream != availableStreams.end(); ++itStream )
    {
        std::vector< AutoRefPtr<IManifestStream> > childStreams;

        (*itStream)->GetChildStreams( &childStreams );

        for( itChildStream = childStreams.begin(); itChildStream != childStreams.end(); ++itChildStream )
        {
            AutoRefPtr<IManifestStream> apParentStream;

            (*itChildStream)->GetParentStream( apParentStream.DerefOutPtr() );

            PKTEST_ASSERT_EXIT( apParentStream == (*itStream) );
        }
    }

    //
    // Verify that child -> parent relationships are reflexive
    //

    for( itStream = availableStreams.begin(); itStream != availableStreams.end(); ++itStream )
    {
        AutoRefPtr<IManifestStream> apParentStream;

        (*itStream)->GetParentStream( apParentStream.DerefOutPtr() );

        if( apParentStream != NULL )
        {
            std::vector< AutoRefPtr<IManifestStream> > childStreams;

            apParentStream->GetChildStreams( &childStreams );

            bool fFound = ( std::find( childStreams.begin(), childStreams.end(), (*itStream) ) != childStreams.end() );

            PKTEST_ASSERT_EXIT( fFound );
        }
    }

    //
    // Verify that stream -> track relationships are reflexive
    //

    for( itStream = availableStreams.begin(); itStream != availableStreams.end(); ++itStream )
    {
        std::vector< AutoRefPtr<IManifestTrack> > availableTracks;

        (*itStream)->GetAvailableTracks( &availableTracks );

        for( itTrack = availableTracks.begin(); itTrack != availableTracks.end(); ++itTrack )
        {
            AutoRefPtr<IManifestStream> apOwner;

            (*itTrack)->GetStream( apOwner.DerefOutPtr() );

            PKTEST_ASSERT_EXIT( apOwner == (*itStream) );
        }
    }

exit:

    return;
}

////////////////////////////////////////////////////////////////////////////////
template< class storeT >
/*static*/ void
ManifestParserTests::VerifyAttributes(AutoRefPtr<storeT> storePtr, _In_count_(cEntries) const Attributes* VerifyAttrib, size_t cEntries )
{
    for( size_t i = 0; i < cEntries; ++i )
    {
        std::wstring val;
        PKTEST_HRESULT_EXIT( storePtr->GetAttribute( VerifyAttrib[i].pszName, &val ) );
        PKTEST_ASSERT_EXIT( val ==  VerifyAttrib[i].pszValue );
    }

exit:
    return;
}
