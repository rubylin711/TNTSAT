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
#include "CChunkManifest.h"
#include "CEvent.h"

#include <algorithm>

using namespace MBR;

//
// Unit tests for the new IManifest APIs
//

PKTEST_GROUP( ManifestAPITests ) , public IStreamsSelectedCallback
{
    StreamSelectedEventArgs m_args;
    CEvent m_streamSelectedEvent;

    void StreamSelectedCallback(_In_ StreamSelectedEventArgs* pEventArgs)
    {
        m_args = *pEventArgs;
        m_streamSelectedEvent.Set();
    }

    ManifestAPITests() : m_streamSelectedEvent(CEvent::eResetModeAuto)
    {
    }

    ////////////////////////////////////
    PKTEST_METHOD( Instantiation )
    {
        AutoRefPtr<CChunkManifest> apChunkManifest;

        PKTEST_HRESULT_EXIT( CChunkManifest::CreateInstance( apChunkManifest.DerefOutPtr() ) );
        PKTEST_ASSERT_EXIT( NULL != apChunkManifest );

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

        AutoRefPtr<CChunkManifest> apChunkManifest;

        PKTEST_HRESULT_EXIT( CChunkManifest::CreateInstance( apChunkManifest.DerefOutPtr() ) );
        PKTEST_ASSERT_EXIT( NULL != apChunkManifest );

        // Set all attributes
        for( int i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            PKTEST_HRESULT_EXIT( apChunkManifest->SetAttribute( rgEntries[i].pszName, rgEntries[i].pszValue ) );
        }

        // Verify all attributes were set accordingly
        for( int i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            std::wstring val;

            PKTEST_HRESULT_EXIT( apChunkManifest->GetAttribute( rgEntries[i].pszName, &val ) );
            PKTEST_ASSERT_EXIT( val == rgEntries[i].pszValue );
        }

        // Set the same attribute again should result in an error
        for (int i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i)
        {
            PKTEST_ASSERT_EXIT( pkFAILED( apChunkManifest->SetAttribute( rgEntries[i].pszName, rgEntries[i].pszValue ) ) )
        }

        // Verify unknown attributes return false
        for( int i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            std::wstring val;

            PKTEST_ASSERT_EXIT( false == apChunkManifest->GetAttribute( std::wstring(rgEntries[i].pszName) + L"0", &val ) );
        }
    exit:
        return;
    }



    struct STREAM_ENTRY
    {
        const wchar_t* pszName;
        const wchar_t* pszParentName;
        MediaStreamType type;
    };

    struct SELECTION_ENTRY
    {
        const wchar_t* pszName;
    };

    ////////////////////////////////////
    void PopupateManifest_Helper(
            _In_ CChunkManifest* pChunkManifest,
            _In_count_(cEntries) const STREAM_ENTRY* prgEntries,
            _In_                 size_t cEntries )
    {
        for( size_t i = 0; i < cEntries; ++i )
        {
            CMediaStreamDescription* pStreamInfo = pChunkManifest->AddStream();

            pStreamInfo->Name()             = prgEntries[i].pszName;
            pStreamInfo->Type()             = prgEntries[i].type;

            if( prgEntries[i].pszParentName != NULL )
            {
                pStreamInfo->ParentStreamName() = prgEntries[i].pszParentName;
            }
        }

        PKTEST_HRESULT_EXIT( pChunkManifest->ResolveSparseStreams() );

        PKTEST_HRESULT_EXIT( pChunkManifest->SelectInitialStreams() );

    exit:
        return;
    }

    ////////////////////////////////////
    void VerifyAvailableStreams_Helper(
            _In_ const std::vector< AutoRefPtr<IManifestStream> >& availableStreams,
            _In_count_(cEntries) const STREAM_ENTRY* prgEntries,
            _In_                 size_t cEntries )
    {
        PKTEST_ASSERT_EXIT( availableStreams.size() == cEntries );

        for( size_t i = 0; i < cEntries; ++i )
        {
            PKTEST_ASSERT_EXIT( availableStreams[i]->Name() == prgEntries[i].pszName );
            PKTEST_ASSERT_EXIT( availableStreams[i]->Type() == prgEntries[i].type );
        }

    exit:
        return;
    }

    ////////////////////////////////////
    void VerifySelectedStreams_Helper(
            _In_ const std::vector< AutoRefPtr<IManifestStream> >& selectedStreams,
            _In_count_(cEntries) const SELECTION_ENTRY* prgEntries,
            _In_                 size_t cEntries )
    {
        PKTEST_ASSERT_EXIT( selectedStreams.size() == cEntries );

        for( size_t i = 0; i < cEntries; ++i )
        {
            PKTEST_ASSERT_EXIT( selectedStreams[i]->Name() == prgEntries[i].pszName );
        }

    exit:
        return;
    }

    ////////////////////////////////////
    void PrepareSelection_Helper(
        _In_ const std::vector< AutoRefPtr<IManifestStream> >& inputList,
        _Inout_ std::vector< AutoRefPtr<IManifestStream> >&    outputList,
        _In_count_(cEntries) const SELECTION_ENTRY* prgEntries,
        _In_                 size_t cEntries )
    {
        outputList.clear();
        outputList.reserve( cEntries );

        for( size_t i = 0; i < cEntries; ++i )
        {
            bool fFound = false;

            for( size_t j = 0; j < inputList.size(); ++j )
            {
                if( inputList[j]->Name() == prgEntries[i].pszName )
                {
                    fFound = true;
                    outputList.push_back( inputList[j] );
                    break;
                }
            }

            PKTEST_ASSERT_EXIT( fFound );
        }

    exit:
        return;
    }


    ////////////////////////////////////
    PKTEST_METHOD( AddAndVerifyAvailableStreams )
    {
        static const STREAM_ENTRY rgEntries[] =
        {
            // Random made up attributes

            { L"video1", NULL, MediaStreamTypeVideo },
            { L"video2", NULL, MediaStreamTypeVideo },
            { L"audio1", NULL, MediaStreamTypeAudio },
            { L"audio2", NULL, MediaStreamTypeAudio },
            { L"text1", NULL, MediaStreamTypeText },
            { L"text2", NULL, MediaStreamTypeText },
        };

        AutoRefPtr<CChunkManifest> apChunkManifest;
        std::vector< AutoRefPtr <IManifestStream> > availableStreams;

        // Create manifest object

        PKTEST_HRESULT_EXIT( CChunkManifest::CreateInstance( apChunkManifest.DerefOutPtr() ) );
        PKTEST_ASSERT_EXIT( NULL != apChunkManifest );

        PKTEST_FUNC_EXIT( PopupateManifest_Helper( apChunkManifest, rgEntries, sizeof(rgEntries)/sizeof(rgEntries[0]) ) );

        // Verify the streams show up in the list of available streams

        PKTEST_HRESULT_EXIT( apChunkManifest->GetAvailableStreams(&availableStreams) );

        PKTEST_FUNC_EXIT( VerifyAvailableStreams_Helper( availableStreams, rgEntries, sizeof(rgEntries)/sizeof(rgEntries[0]) ) );

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( DefaultSelectedStreams )
    {
        static const STREAM_ENTRY rgEntries[] =
        {
            // Random made up streams

            { L"video1", NULL, MediaStreamTypeVideo },
            { L"video2", NULL, MediaStreamTypeVideo },
            { L"audio1", NULL, MediaStreamTypeAudio },
            { L"audio2", NULL, MediaStreamTypeAudio },
            { L"text1",  NULL, MediaStreamTypeText  },
            { L"text2",  NULL, MediaStreamTypeText  },
        };

        // By default only the last video and audio are selected
        static const SELECTION_ENTRY rgDefaultSelections[] =
        {
            { L"video2" },
            { L"audio2" },
        };

        AutoRefPtr<CChunkManifest> apChunkManifest;
        std::vector< AutoRefPtr <IManifestStream> > availableStreams;
        std::vector< AutoRefPtr <IManifestStream> > selectedStreams;

        // Create manifest object

        PKTEST_HRESULT_EXIT( CChunkManifest::CreateInstance( apChunkManifest.DerefOutPtr() ) );
        PKTEST_ASSERT_EXIT( NULL != apChunkManifest );

        // Populate with streams

        PKTEST_FUNC_EXIT( PopupateManifest_Helper( apChunkManifest, rgEntries, sizeof(rgEntries)/sizeof(rgEntries[0]) ) );

        // Verify the streams show up in the list of available streams

        PKTEST_HRESULT_EXIT( apChunkManifest->GetAvailableStreams(&availableStreams) );

        PKTEST_FUNC_EXIT( VerifyAvailableStreams_Helper( availableStreams, rgEntries, sizeof(rgEntries)/sizeof(rgEntries[0]) ) );

        // Get the selected stream list and verify the default selection

        PKTEST_HRESULT_EXIT( apChunkManifest->GetSelectedStreams(&selectedStreams) );

        PKTEST_FUNC_EXIT( VerifySelectedStreams_Helper( selectedStreams, rgDefaultSelections, sizeof(rgDefaultSelections)/sizeof(rgDefaultSelections[0]) ) );

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( InvalidSelectionForSelectedStreams )
    {
        static const STREAM_ENTRY rgEntries[] =
        {
            // Random made up streams, make sure there are multiple audio/video/text streams

            { L"video1", NULL, MediaStreamTypeVideo },
            { L"video2", NULL, MediaStreamTypeVideo },
            { L"audio1", NULL, MediaStreamTypeAudio },
            { L"audio2", NULL, MediaStreamTypeAudio },
            { L"text1",  NULL, MediaStreamTypeText },
            { L"text2",  NULL, MediaStreamTypeText },
        };

        // Selection doesn't include audio and/or video
        static const SELECTION_ENTRY rgBadSelections1[] =
        {
            { L"text1" },
            { L"text2" },
        };

        // Selection includes 2 video streams
        static const SELECTION_ENTRY rgBadSelections2[] =
        {
            { L"video1" },
            { L"video2" },
            { L"audio1" },
        };

        AutoRefPtr<CChunkManifest> apChunkManifest;
        std::vector< AutoRefPtr <IManifestStream> > availableStreams;
        std::vector< AutoRefPtr <IManifestStream> > selectedStreams;
        std::vector< AutoRefPtr <IManifestStream> > proposedSelection;

        AutoRefPtr<CMediaStreamDescription> apMSD;
        AutoRefPtr<IManifestStream> apStream;

        // Create manifest object

        PKTEST_HRESULT_EXIT( CChunkManifest::CreateInstance( apChunkManifest.DerefOutPtr() ) );
        PKTEST_ASSERT_EXIT( NULL != apChunkManifest );

        // Populate with streams

        PKTEST_FUNC_EXIT( PopupateManifest_Helper( apChunkManifest, rgEntries, sizeof(rgEntries)/sizeof(rgEntries[0]) ) );

        // Verify the streams show up in the list of available streams

        PKTEST_HRESULT_EXIT( apChunkManifest->GetAvailableStreams(&availableStreams) );

        PKTEST_FUNC_EXIT( VerifyAvailableStreams_Helper( availableStreams, rgEntries, sizeof(rgEntries)/sizeof(rgEntries[0]) ) );

        // Try to select only the text streams.
        // Expect it to fail.

        PKTEST_FUNC_EXIT( PrepareSelection_Helper( availableStreams, proposedSelection, rgBadSelections1, sizeof(rgBadSelections1)/sizeof(rgBadSelections1[0]) ) );

        PKTEST_ASSERT_EXIT( FAILED(apChunkManifest->SelectStreamsAsync(this, proposedSelection)) );

        // Try to select 2 video streams.
        // Expect it to fail.

        PKTEST_FUNC_EXIT( PrepareSelection_Helper( availableStreams, proposedSelection, rgBadSelections2, sizeof(rgBadSelections2)/sizeof(rgBadSelections2[0]) ) );

        PKTEST_ASSERT_EXIT( FAILED(apChunkManifest->SelectStreamsAsync(this, proposedSelection)) );

        // Select streams not belong to the manifest and expect error
        proposedSelection.clear();
        PKTEST_HRESULT_EXIT( apChunkManifest->GetSelectedStreams(&proposedSelection) );

        PKTEST_HRESULT_EXIT( CMediaStreamDescription::CreateInstance( apMSD.DerefOutPtr() ) );
        PKTEST_ASSERT_EXIT( NULL != apMSD );
        apMSD->Type() = MediaStreamTypeText;
        apMSD->Name() = L"SomeText";
        apStream.Set( apMSD );
        proposedSelection.push_back( apStream );

        PKTEST_ASSERT_EXIT( FAILED(apChunkManifest->SelectStreamsAsync(this, proposedSelection)) );

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ChangeAndVerifySelectedStreams )
    {
        static const STREAM_ENTRY rgEntries[] =
        {
            // Random made up streams, make sure there are multiple audio/video/text streams

            { L"video1", NULL, MediaStreamTypeVideo },
            { L"video2", NULL, MediaStreamTypeVideo },
            { L"audio1", NULL, MediaStreamTypeAudio },
            { L"audio2", NULL, MediaStreamTypeAudio },
            { L"text1",  NULL, MediaStreamTypeText },
            { L"text2",  NULL, MediaStreamTypeText },
            { L"text3",  NULL, MediaStreamTypeText },
        };

        // By default only the last video and audio are selected
        static const SELECTION_ENTRY rgDefaultSelections[] =
        {
            { L"video2" },
            { L"audio2" },
        };

        static const SELECTION_ENTRY rgSelections[] =
        {
            { L"video1" },
            { L"audio1" },
            { L"text1" },
            { L"text3" },
        };

        struct
        {
            const wchar_t* pszName;
            StreamChangedEventArgs::StreamChangedAction action;
        }
        static const rgSelResults[] =
        {
            { L"video2", StreamChangedEventArgs::StreamDeselected },
            { L"audio2", StreamChangedEventArgs::StreamDeselected },
            { L"video1", StreamChangedEventArgs::StreamSelected },
            { L"audio1", StreamChangedEventArgs::StreamSelected },
            { L"text1",  StreamChangedEventArgs::StreamSelected },
            { L"text3",  StreamChangedEventArgs::StreamSelected },
        };

        AutoRefPtr<CChunkManifest> apChunkManifest;
        std::vector< AutoRefPtr <IManifestStream> > availableStreams;
        std::vector< AutoRefPtr <IManifestStream> > selectedStreams;
        std::vector< AutoRefPtr <IManifestStream> > proposedSelection;

        AutoRefPtr<CMediaStreamDescription> apMSD;
        AutoRefPtr<IManifestStream> apStream;

        PKTEST_HRESULT_EXIT( CChunkManifest::CreateInstance( apChunkManifest.DerefOutPtr() ) );
        PKTEST_ASSERT_EXIT( NULL != apChunkManifest );

        // Add these streams to the available stream list
        PKTEST_FUNC_EXIT( PopupateManifest_Helper( apChunkManifest, rgEntries, sizeof(rgEntries)/sizeof(rgEntries[0]) ) );

        // Get the list of available streams
        PKTEST_HRESULT_EXIT( apChunkManifest->GetAvailableStreams( &availableStreams ) );
        PKTEST_FUNC_EXIT( VerifyAvailableStreams_Helper( availableStreams, rgEntries, sizeof(rgEntries)/sizeof(rgEntries[0]) ) );

        // Verify the default selection
        PKTEST_HRESULT_EXIT( apChunkManifest->GetSelectedStreams( &selectedStreams ) );
        PKTEST_FUNC_EXIT( VerifySelectedStreams_Helper( selectedStreams, rgDefaultSelections, sizeof(rgDefaultSelections)/sizeof(rgDefaultSelections[0]) ) );

        // Apply the selection
        PKTEST_FUNC_EXIT( PrepareSelection_Helper( availableStreams, proposedSelection, rgSelections, sizeof(rgSelections)/sizeof(rgSelections[0]) ) );

        // Do the actual selection and wait for the event to finish
        PKTEST_HRESULT_EXIT( apChunkManifest->SelectStreamsAsync( this, proposedSelection ) );

        m_streamSelectedEvent.Wait();

        // Verify the callback result
        PKTEST_HRESULT_EXIT( m_args.Result );
        PKTEST_ASSERT_EXIT( m_args.StreamChanges.size() == sizeof(rgSelResults)/sizeof(rgSelResults[0]) );

        for( size_t i = 0; i < m_args.StreamChanges.size(); ++i )
        {
            PKTEST_HRESULT_EXIT( m_args.StreamChanges[i].Result );
            PKTEST_ASSERT_EXIT( m_args.StreamChanges[i].Action == rgSelResults[i].action );
            PKTEST_ASSERT_EXIT( m_args.StreamChanges[i].pStream->Name() == rgSelResults[i].pszName );
        }

        // Verify the the selected stream list is updated

        PKTEST_HRESULT_EXIT( apChunkManifest->GetSelectedStreams( &selectedStreams ) );

        PKTEST_FUNC_EXIT( VerifySelectedStreams_Helper( selectedStreams, rgSelections, sizeof(rgSelections)/sizeof(rgSelections[0]) ) );

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ChildStreamSelection )
    {
        static const STREAM_ENTRY rgEntries[] =
        {
            { L"video1",        NULL,      MediaStreamTypeVideo },
            { L"video1_child",  L"video1", MediaStreamTypeText },
            { L"audio1",        NULL,      MediaStreamTypeAudio },
            { L"audio1_child",  L"audio1", MediaStreamTypeText },
            { L"video2",        NULL,      MediaStreamTypeVideo },
            { L"audio2",        NULL,      MediaStreamTypeAudio },
        };

        static const SELECTION_ENTRY rgParentsOnly[] =
        {
            { L"video1" },
            { L"audio1" },
        };

        static const SELECTION_ENTRY rgParentsAndOneChild[] =
        {
            { L"video1" },
            { L"audio1" },
            { L"video1_child" },
        };

        static const SELECTION_ENTRY rgParentsAndAnotherChild[] =
        {
            { L"video1" },
            { L"audio1" },
            { L"video1_child" },
            { L"audio1_child" },
        };

        static const SELECTION_ENTRY rgParentsAndChildren[] =
        {
            { L"video1" },
            { L"audio1" },
            { L"audio1_child" },
        };

        static const SELECTION_ENTRY rgFullStreamsWithNoChildren[] =
        {
            { L"video2" },
            { L"audio2" },
        };

        static const SELECTION_ENTRY rgChildrenNoParents1[] =
        {
            { L"video2" },
            { L"audio2" },
            { L"video1_child" },
        };

        static const SELECTION_ENTRY rgChildrenNoParents2[] =
        {
            { L"video1" },
            { L"audio2" },
            { L"video1_child" },
            { L"audio1_child" },
        };

        AutoRefPtr<CChunkManifest> apChunkManifest;
        std::vector< AutoRefPtr <IManifestStream> > availableStreams;
        std::vector< AutoRefPtr <IManifestStream> > proposedSelection;

        PKTEST_HRESULT_EXIT( CChunkManifest::CreateInstance( apChunkManifest.DerefOutPtr() ) );
        PKTEST_ASSERT_EXIT( NULL != apChunkManifest );

        // Add these streams to the available stream list
        PKTEST_FUNC_EXIT( PopupateManifest_Helper( apChunkManifest, rgEntries, sizeof(rgEntries)/sizeof(rgEntries[0]) ) );

        PKTEST_HRESULT_EXIT( apChunkManifest->GetAvailableStreams( &availableStreams ) );

        // Can select just the parents
        PKTEST_FUNC_EXIT( PrepareSelection_Helper( availableStreams, proposedSelection, rgParentsOnly, sizeof(rgParentsOnly)/sizeof(rgParentsOnly[0]) ) );
        PKTEST_ASSERT_EXIT( S_OK == apChunkManifest->SelectStreamsAsync( this, proposedSelection ) );
        m_streamSelectedEvent.Wait();
        PKTEST_HRESULT_EXIT( m_args.Result );

        // Can select all parents and one child
        PKTEST_FUNC_EXIT( PrepareSelection_Helper( availableStreams, proposedSelection, rgParentsAndOneChild, sizeof(rgParentsAndOneChild)/sizeof(rgParentsAndOneChild[0]) ) );
        PKTEST_ASSERT_EXIT( S_OK == apChunkManifest->SelectStreamsAsync( this, proposedSelection ) );
        m_streamSelectedEvent.Wait();
        PKTEST_HRESULT_EXIT( m_args.Result );

        // Can select all parents and another child
        PKTEST_FUNC_EXIT( PrepareSelection_Helper( availableStreams, proposedSelection, rgParentsAndAnotherChild, sizeof(rgParentsAndAnotherChild)/sizeof(rgParentsAndAnotherChild[0]) ) );
        PKTEST_ASSERT_EXIT( S_OK == apChunkManifest->SelectStreamsAsync( this, proposedSelection ) );
        m_streamSelectedEvent.Wait();
        PKTEST_HRESULT_EXIT( m_args.Result );

        // Can select all parents and all children
        PKTEST_FUNC_EXIT( PrepareSelection_Helper( availableStreams, proposedSelection, rgParentsAndChildren, sizeof(rgParentsAndChildren)/sizeof(rgParentsAndChildren[0]) ) );
        PKTEST_ASSERT_EXIT( S_OK == apChunkManifest->SelectStreamsAsync( this, proposedSelection ) );
        m_streamSelectedEvent.Wait();
        PKTEST_HRESULT_EXIT( m_args.Result );

        // Can select streams that have no parents nor children
        PKTEST_FUNC_EXIT( PrepareSelection_Helper( availableStreams, proposedSelection, rgFullStreamsWithNoChildren, sizeof(rgFullStreamsWithNoChildren)/sizeof(rgFullStreamsWithNoChildren[0]) ) );
        PKTEST_ASSERT_EXIT( S_OK == apChunkManifest->SelectStreamsAsync( this, proposedSelection ) );
        m_streamSelectedEvent.Wait();
        PKTEST_HRESULT_EXIT( m_args.Result );

        // Can't select a child without its parent
        PKTEST_FUNC_EXIT( PrepareSelection_Helper( availableStreams, proposedSelection, rgChildrenNoParents1, sizeof(rgChildrenNoParents1)/sizeof(rgChildrenNoParents1[0]) ) );
        PKTEST_ASSERT_EXIT( E_INVALIDARG == apChunkManifest->SelectStreamsAsync( this, proposedSelection ) );

        // Can't select any child without that child's parent
        PKTEST_FUNC_EXIT( PrepareSelection_Helper( availableStreams, proposedSelection, rgChildrenNoParents2, sizeof(rgChildrenNoParents2)/sizeof(rgChildrenNoParents2[0]) ) );
        PKTEST_ASSERT_EXIT( E_INVALIDARG == apChunkManifest->SelectStreamsAsync( this, proposedSelection ) );

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( AbsentParentChildRemoval )
    {
        static const STREAM_ENTRY rgEntries[] =
        {
            { L"video1",        NULL,               MediaStreamTypeVideo },
            { L"video1_child",  L"video1",          MediaStreamTypeText },
            { L"orphan1",       L"absentParent1",   MediaStreamTypeText },      // this must be removed from available streams
            { L"audio1",        NULL,               MediaStreamTypeAudio },
            { L"audio1_child",  L"audio1",          MediaStreamTypeText },
            { L"orphan2",       L"absentParent2",   MediaStreamTypeText },      // this must be removed from available streams
        };

        static const STREAM_ENTRY rgExpectedAvailables[] =
        {
            { L"video1",        NULL,               MediaStreamTypeVideo },
            { L"video1_child",  L"video1",          MediaStreamTypeText },
            { L"audio1",        NULL,               MediaStreamTypeAudio },
            { L"audio1_child",  L"audio1",          MediaStreamTypeText },
        };

        AutoRefPtr<CChunkManifest> apChunkManifest;
        std::vector< AutoRefPtr <IManifestStream> > availableStreams;
        std::vector< AutoRefPtr <IManifestStream> > proposedSelection;

        PKTEST_HRESULT_EXIT( CChunkManifest::CreateInstance( apChunkManifest.DerefOutPtr() ) );
        PKTEST_ASSERT_EXIT( NULL != apChunkManifest );

        // Add these streams to the available stream list
        PKTEST_FUNC_EXIT( PopupateManifest_Helper( apChunkManifest, rgEntries, sizeof(rgEntries)/sizeof(rgEntries[0]) ) );

        PKTEST_HRESULT_EXIT( apChunkManifest->GetAvailableStreams( &availableStreams ) );

        PKTEST_FUNC_EXIT( VerifyAvailableStreams_Helper( availableStreams, rgExpectedAvailables, sizeof(rgExpectedAvailables)/sizeof(rgExpectedAvailables[0]) ) );

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ManifestReadyCallback )
    {
        /// TODO: To be implemented

        PKTEST_ASSERT_EXIT( true);

    exit:

        return;
    }
};
