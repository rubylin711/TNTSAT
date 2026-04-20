///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////
//
// Tests to test the test suite
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Verifies that the TestSetup and TestCleanup methods work

#if 0

PKTEST_GROUP( TestCleanupSetup )
{
    int m_cSetups;
    int m_cCleanups;

    TestCleanupSetup()
        : m_cSetups( 0 )
        , m_cCleanups( 0 )
    {
    }

    bool TestSetup()
    {
        ++m_cSetups;
        return( m_cSetups > 1 );
    }

    bool TestCleanup()
    {
        ++m_cCleanups;
        return( true );
    }

    ////////////////////////////////////
    PKTEST_METHOD( DoNotExecuteIfTestSetupReturnsFail )
    {
        // TestSetup returns 'false' for the first
        // test, so this test should never run
        PKTEST_ASSERT_EXIT( FALSE );

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( SecondTestWasSetup )
    {
        PKTEST_ASSERT_EXIT( m_cSetups == 2 );
        PKTEST_ASSERT_EXIT( m_cCleanups == 0 );

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ThirdTestWasSetup )
    {
        PKTEST_ASSERT_EXIT( m_cSetups == 3 );
        PKTEST_ASSERT_EXIT( m_cCleanups == 1 );

    exit:
        return;
    }
};

#endif

////////////////////////////////////////////////////////////////////////////////
// Tests for the read-only XML mini DOM

PKTEST_GROUP( XmlMiniReadOnlyDom )
{
    ////////////////////////////////////
    PKTEST_METHOD( NavigateEmptyElement )
    {
        CXMLElement fixture;

        PKTEST_ASSERT_EXIT( fixture.IsNull() );
        PKTEST_ASSERT_EXIT( fixture.Name()[0] == L'\0' );

        PKTEST_ASSERT_EXIT( fixture.Attributes().Length() == 0 );
        PKTEST_ASSERT_EXIT( fixture.Elements().Length() == 0 );
        PKTEST_ASSERT_EXIT( fixture.Elements(L"Foo").Length() == 0 );

        {
            CXMLAttribute attribute;

            CXMLAttributesList list = fixture.Attributes();

            PKTEST_ASSERT_EXIT( list.Length() == 0 );
            PKTEST_ASSERT_EXIT( list[0].IsNull() );

            attribute = list[0];

            PKTEST_ASSERT_EXIT( attribute.IsNull() );

            PKTEST_ASSERT_EXIT( attribute.Name()[0] == L'\0' );
            PKTEST_ASSERT_EXIT( attribute.Value()[0] == L'\0' );
        }

        {
            CXMLElement child;

            CXMLElementsList list = fixture.Elements();

            PKTEST_ASSERT_EXIT( list.Length() == 0 );
            PKTEST_ASSERT_EXIT( list[0].IsNull() );

            child = list[0];

            PKTEST_ASSERT_EXIT( child.IsNull() );
            PKTEST_ASSERT_EXIT( child.Name()[0] == L'\0' );

            PKTEST_ASSERT_EXIT( child.Attributes().Length() == 0 );
            PKTEST_ASSERT_EXIT( child.Elements().Length() == 0 );
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ParseSingleElementDoc1 )
    {
        const char szData[] = "<doc></doc>";

        CXMLDocument fixture;
        CXMLElement element;

        PKTEST_HRESULT_EXIT( fixture.LoadFromBuffer( szData, sizeof(szData) ) );

        element = fixture.RootElement();

        PKTEST_ASSERT_EXIT( !element.IsNull() );
        PKTEST_ASSERT_EXIT( wcscmp( element.Name(), L"doc" ) == 0 );
        PKTEST_ASSERT_EXIT( wcscmp( element.Name(), L"bar" ) != 0 );

        PKTEST_ASSERT_EXIT( element.Attributes().Length() == 0 );
        PKTEST_ASSERT_EXIT( element.Elements().Length() == 0 );
        PKTEST_ASSERT_EXIT( element.Elements(L"sub").Length() == 0 );

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ParseSingleElementDoc2 )
    {
        const char szData[] = "<doc/>";

        CXMLDocument fixture;
        CXMLElement element;

        PKTEST_HRESULT_EXIT( fixture.LoadFromBuffer( szData, sizeof(szData) ) );

        element = fixture.RootElement();

        PKTEST_ASSERT_EXIT( !element.IsNull() );
        PKTEST_ASSERT_EXIT( wcscmp( element.Name(), L"doc" ) == 0 );
        PKTEST_ASSERT_EXIT( wcscmp( element.Name(), L"bar" ) != 0 );

        PKTEST_ASSERT_EXIT( element.Attributes().Length() == 0 );
        PKTEST_ASSERT_EXIT( element.Elements().Length() == 0 );
        PKTEST_ASSERT_EXIT( element.Elements(L"sub").Length() == 0 );

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ParseSingleElementWithAttributes )
    {
        const char szData[] = "<doc id=\"foo\" comment=\"baroo\">";

        CXMLDocument fixture;
        CXMLAttributesList list;
        CXMLElement element;

        PKTEST_HRESULT_EXIT( fixture.LoadFromBuffer( szData, sizeof(szData) ) );

        element = fixture.RootElement();

        PKTEST_ASSERT_EXIT( list.Length() == 0 );

        list = element.Attributes();

        PKTEST_ASSERT_EXIT( list.Length() == 2 );

        {
            CXMLAttribute xid = list[0];
            PKTEST_ASSERT_EXIT( !xid.IsNull() );
            PKTEST_ASSERT_EXIT( wcscmp( xid.Name(), L"id" ) == 0 );
            PKTEST_ASSERT_EXIT( wcscmp( xid.Value(), L"foo" ) == 0 );
        }

        {
            CXMLAttribute xcomment = list[1];
            PKTEST_ASSERT_EXIT( !xcomment.IsNull() );
            PKTEST_ASSERT_EXIT( wcscmp( xcomment.Name(), L"comment" ) == 0 );
            PKTEST_ASSERT_EXIT( wcscmp( xcomment.Value(), L"baroo" ) == 0 );
        }

        {
            CXMLAttribute xnil = list[2];
            PKTEST_ASSERT_EXIT( xnil.IsNull() );
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ParseElementWithChildren )
    {
        const char szData[] =
            "<doc>"
                "<bar id=\"1\"/>"
                "<foo id=\"1\"/>"
                "<foo id=\"2\">"
                    "<bar comment=\"nested, must not return in root's Elements\"/>"
                "</foo>"
                "<bar id=\"2\"/>"
                "<bar id=\"3\"/>"
            "</doc>";

        struct
        {
            const WCHAR* pszElemName;
            const WCHAR* pszIdValue;
        }
        static const rgExpectedElements[] =
        {
            { L"bar", L"1" },
            { L"foo", L"1" },
            { L"foo", L"2" },
            { L"bar", L"2" },
            { L"bar", L"3" },
        };

        struct
        {
            const WCHAR* pszElementName;
            int cElements;
        }
        static const rgExpectedCounters[] =
        {
            { L"bar", 3 },
            { L"foo", 2 },
        };

        CXMLDocument fixture;
        CXMLElement element;

        PKTEST_HRESULT_EXIT( fixture.LoadFromBuffer( szData, sizeof(szData) ) );

        element = fixture.RootElement();

        {
            CXMLElementsList allChildren = element.Elements();

            PKTEST_ASSERT_EXIT( allChildren.Length() == sizeof(rgExpectedElements)/sizeof(rgExpectedElements[0]) );

            for( int i = 0; i < allChildren.Length(); ++i )
            {
                CXMLElement e = allChildren[i];

                PKTEST_ASSERT_EXIT( wcscmp( e.Name(), rgExpectedElements[i].pszElemName ) == 0 );
                PKTEST_ASSERT_EXIT( wcscmp( e.Attributes()[L"id"].Value(), rgExpectedElements[i].pszIdValue ) == 0 );
            }
        }

        //
        // Check that selective Elements( "name"  ) works
        //

        for( int iGroup = 0; iGroup < sizeof(rgExpectedCounters)/sizeof(rgExpectedCounters[0]); ++iGroup )
        {
            CXMLElementsList list = element.Elements( rgExpectedCounters[iGroup].pszElementName );

            PKTEST_HRESULT_EXIT( list.Length() == rgExpectedCounters[iGroup].cElements );

            for( int i = 0; i < list.Length(); ++i )
            {
                PKTEST_HRESULT_EXIT( wcscmp( list[i].Name(), rgExpectedCounters[iGroup].pszElementName ) == 0 );
            }
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ParseBadData1 )
    {
        const char szData[] =
            "<doc>"
            "</bok>";

        CXMLDocument fixture;

        PKTEST_ASSERT_EXIT( FAILED( fixture.LoadFromBuffer( szData, sizeof(szData) ) ) );

    exit:
        return;
    }
};

////////////////////////////////////////////////////////////////////////////////
// Tests for the test suite auxiliary class CContiguousBufferBuilder, used
// by the XML mini DOM

PKTEST_GROUP( ContiguousBufferBuilderTests )
{
    ////////////////////////////////////
    PKTEST_METHOD(BuildSimpleBuffer)
    {
        CContiguousBufferBuilder fixture;
        size_t cPasses = 0;

        char* pb1 = NULL;
        char* pb2 = NULL;
        char* pbAll = NULL;
        pkRESULT hr = pkS_OK;

        do
        {
            PKTEST_HRESULT_EXIT( fixture.Append( sizeof(long), (void**)&pb1 ) );
            PKTEST_HRESULT_EXIT( fixture.Append( sizeof(long), (void**)&pb2 ) );

            PKTEST_ASSERT_EXIT( cPasses < 2 );

            if( cPasses == 0 )
            {
                //
                // In pass 0, the size of the buffer is computed,
                // but nothing is allocated
                //
                PKTEST_ASSERT_EXIT( pb1 == NULL );
                PKTEST_ASSERT_EXIT( pb2 == NULL );
            }
            else
            {
                PKTEST_ASSERT_EXIT( ( pb2 - pb1 ) == sizeof(long) );

                *reinterpret_cast<long*>(pb1) = 0;
                *reinterpret_cast<long*>(pb2) = 0;
            }

            ++cPasses;
        }
        while( ( hr = fixture.Loop() ) == S_OK );

        PKTEST_HRESULT_EXIT( hr );

        pbAll = fixture.DetachBuffer();

        PKTEST_ASSERT_EXIT( pbAll != NULL );
        PKTEST_ASSERT_EXIT( pbAll == pb1 );

    exit:

        delete pbAll;

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD(BuildBufferWithCopy)
    {
        CContiguousBufferBuilder fixture;
        size_t cPasses = 0;

        static const unsigned char pattern1[] =
        {
            0x95, 0xc8, 0x8b, 0x7e, 0xd9, 0x2d, 0x4b, 0x3b, 0x9c, 0xcd, 0x35, 0x26, 0xdd, 0x14, 0xf6, 0x58
        };

        static const unsigned char pattern2[] =
        {
            0xf7, 0x2e, 0x94, 0xdd, 0xa5, 0xb0, 0x02, 0xfc, 0x74, 0x8e, 0xb5, 0xaa, 0xb5, 0xf9, 0x85, 0x78,
            0x66, 0xdc, 0x9d, 0xa3, 0xe3, 0x2a, 0x99, 0xf6, 0x89, 0x4c, 0xd2, 0x09, 0x75, 0x44, 0x04, 0x65,
            0x04, 0x8a, 0xc2, 0xaf, 0x3a, 0x8f, 0x9c, 0x12
        };

        char* pb1 = NULL;
        char* pb2 = NULL;
        pkRESULT hr = pkS_OK;

        do
        {
            PKTEST_HRESULT_EXIT( fixture.AppendCopy( pattern1, sizeof(pattern1), (void**)&pb1 ) );
            PKTEST_HRESULT_EXIT( fixture.AppendCopy( pattern2, sizeof(pattern2), (void**)&pb2 ) );

            PKTEST_ASSERT_EXIT( cPasses < 2 );

            if( cPasses == 0 )
            {
                //
                // In pass 0, the size of the buffer is computed,
                // but nothing is allocated
                //
                PKTEST_ASSERT_EXIT( pb1 == NULL );
                PKTEST_ASSERT_EXIT( pb2 == NULL );
            }
            else
            {
                PKTEST_ASSERT_EXIT( memcmp( pb1, pattern1, sizeof(pattern1) ) == 0 );
                PKTEST_ASSERT_EXIT( memcmp( pb2, pattern2, sizeof(pattern2) ) == 0 );
            }

            ++cPasses;
        }
        while( ( hr = fixture.Loop() ) == S_OK );

        PKTEST_HRESULT_EXIT( hr );

    exit:

        return;
    }
};

////////////////////////////////////////////////////////////////////////////////
// Tests for the test suite logger auxiliary function EscapeXmlStringInPlace
//

PKTEST_GROUP( EscapeXmlStringInPlaceTests )
{
    ////////////////////////////////////
    PKTEST_METHOD(NoCharsToEscape)
    {
        const char szExpected[] = "no characters to escape";

        char szWorkingBuffer[sizeof(szExpected)];

        memcpy( szWorkingBuffer, szExpected, sizeof(szExpected) );

        PKTEST_HRESULT_EXIT( EscapeXmlStringInPlace( szWorkingBuffer, sizeof(szWorkingBuffer) ) );
        PKTEST_ASSERT_EXIT( strcmp( szExpected, szWorkingBuffer ) == 0 );

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD(AllCharsToEscape)
    {
        const char szOriginal[]  = "escape < and > and \" and & and that's it";
        const char szExpected[]  = "escape &lt; and &gt; and &quot; and &amp; and that's it";

        static const size_t cchGuardSequence = 10;

        char workingBuffer[ cchGuardSequence + sizeof(szExpected) + cchGuardSequence ];
        char guardSequence[ cchGuardSequence ];

        for( char chGuard = 1; chGuard <= 2; ++chGuard )
        {
            // Set working buffer to [guard][original][guard]

            memset( guardSequence, chGuard, sizeof(guardSequence) );
            memset( workingBuffer, chGuard, sizeof(workingBuffer) );

            memcpy( workingBuffer + cchGuardSequence, szOriginal, sizeof(szOriginal) );

            PKTEST_ASSERT_EXIT( memcmp( workingBuffer, guardSequence, cchGuardSequence ) == 0 );
            PKTEST_ASSERT_EXIT( memcmp( workingBuffer + cchGuardSequence, szOriginal, sizeof(szOriginal) ) == 0 );
            PKTEST_ASSERT_EXIT( memcmp( workingBuffer + sizeof(workingBuffer) - cchGuardSequence, guardSequence, cchGuardSequence ) == 0 );

            // Call the fixture to escape the XML chars

            PKTEST_HRESULT_EXIT( EscapeXmlStringInPlace( workingBuffer + cchGuardSequence, sizeof(szExpected) ) );

            // Check the string has been escaped in the working buffer and it didn't touch the guards

            PKTEST_ASSERT_EXIT( memcmp( workingBuffer, guardSequence, cchGuardSequence ) == 0 );
            PKTEST_ASSERT_EXIT( memcmp( workingBuffer + cchGuardSequence, szExpected, sizeof(szExpected) ) == 0 );
            PKTEST_ASSERT_EXIT( memcmp( workingBuffer + sizeof(workingBuffer) - cchGuardSequence, guardSequence, cchGuardSequence ) == 0 );
        }

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD(FailsIfBufferNotBigEnough)
    {
        const char szOriginal[]  = "escape < and > and \" and & and that's it";
        const char szExpected[]  = "escape &lt; and &gt; and &quot; and &amp; and that's it";

        static const size_t cchGuardSequence = 10;

        char workingBuffer[ cchGuardSequence + sizeof(szExpected) + cchGuardSequence ];
        char guardSequence[ cchGuardSequence ];

        for( char chGuard = 1; chGuard <= 2; ++chGuard )
        {
            // Set working buffer to [guard][original][guard]

            memset( guardSequence, chGuard, sizeof(guardSequence) );
            memset( workingBuffer, chGuard, sizeof(workingBuffer) );

            memcpy( workingBuffer + cchGuardSequence, szOriginal, sizeof(szOriginal) );

            PKTEST_ASSERT_EXIT( memcmp( workingBuffer, guardSequence, cchGuardSequence ) == 0 );
            PKTEST_ASSERT_EXIT( memcmp( workingBuffer + cchGuardSequence, szOriginal, sizeof(szOriginal) ) == 0 );
            PKTEST_ASSERT_EXIT( memcmp( workingBuffer + sizeof(workingBuffer) - cchGuardSequence, guardSequence, cchGuardSequence ) == 0 );

            // Call the fixture to escape the XML chars but
            // with a buffer 1 char too short

            pkRESULT pkr = EscapeXmlStringInPlace( workingBuffer + cchGuardSequence, sizeof(szExpected) - 1 );

            PKTEST_ASSERT_EXIT( pkr == pkE_INVALIDARG );        // must fail
            PKTEST_ASSERT_EXIT( workingBuffer[cchGuardSequence] == '\0' );  // empty buffer on failure

            // Check the operation didn't touch the guards

            PKTEST_ASSERT_EXIT( memcmp( workingBuffer, guardSequence, cchGuardSequence ) == 0 );
            PKTEST_ASSERT_EXIT( memcmp( workingBuffer + sizeof(workingBuffer) - cchGuardSequence, guardSequence, cchGuardSequence ) == 0 );
        }

    exit:

        return;
    }
};
