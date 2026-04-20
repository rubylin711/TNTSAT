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

#include <StringUtils.h>

////////////////////////////////////////////////////////////////////////////////
PKTEST_GROUP( StringUtilsTests )
{
    ////////////////////////////////////
    PKTEST_METHOD( ConvertHexWStrToByteArray )
    {
        const wchar_t szInput[] = L"d3be27f1678a27f859e8045081c3237a11c0";

        const uint8 expected[] = { 0xd3, 0xbe, 0x27, 0xf1, 0x67, 0x8a, 0x27, 0xf8, 0x59, 0xe8, 0x04, 0x50, 0x81, 0xc3, 0x23, 0x7a, 0x11, 0xc0 };

        const size_t cbGuard = 13;

        uint8 targetBuffer[
                    cbGuard                 // room for the head guard
                    + sizeof(expected)      // room for the actual target data
                    + cbGuard               // room for the tail guard
                    ];

        //
        // Make sure HexStrToBytes fails if the target buffer is not big enough
        //

        PKTEST_ASSERT_EXIT(
                    HexStrToBytes(
                        szInput,
                        sizeof(szInput)/sizeof(szInput[0]) - 1,
                        targetBuffer,
                        sizeof(expected) - 1 ) == pkE_FAIL )
                        
        //
        // Initialize the target buffer
        //

        memset( targetBuffer, 0, sizeof(targetBuffer) );

        for( size_t j = 0; j < sizeof(targetBuffer); ++j )
        {
            PKTEST_ASSERT_EXIT( targetBuffer[j] == 0 );
        }

        //
        // Convert
        //

        PKTEST_HRESULT_EXIT(
                    HexStrToBytes(
                        szInput,
                        sizeof(szInput)/sizeof(szInput[0]) - 1,
                        targetBuffer + cbGuard,
                        sizeof(expected) )
                        );

        //
        // Check the result
        //

        PKTEST_ASSERT_EXIT( memcmp( targetBuffer + cbGuard, expected, sizeof(expected) ) == 0 );

        //
        // Check the head and tail guard sequences
        //

        for( size_t j = 0; j < cbGuard; ++j )
        {
            PKTEST_ASSERT_EXIT( targetBuffer[j] == 0 );
            PKTEST_ASSERT_EXIT( targetBuffer[cbGuard + sizeof(expected) + j] == 0 );
        }

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( BadFormatConvertHexWStrToByteArray )
    {
        static const wchar_t* rgEntries[] =
        {
            L"0",
            L"0x10",
            L"000",
            L"fOO",
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            std::wstring input( rgEntries[i] );

            uint8 targetBuffer[256];

            // Just make sure the buffer is big enough or HexStrToBytes won't even
            // parse input string.
            PKTEST_ASSERT_EXIT( sizeof(targetBuffer) > ( input.length() + 1 ) / 2 );

            PKTEST_ASSERT_EXIT(
                        HexStrToBytes(
                            input.c_str(),
                            input.length(),
                            targetBuffer,
                            input.length() / 2 ) == pkE_INVALID_FORMAT );
        }

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( StrStartsWith )
    {
        // Positives
        PKTEST_ASSERT_EXIT( startsWith( "teststring", "t" ) );
        PKTEST_ASSERT_EXIT( startsWith( "stringtest", "string" ) );
        PKTEST_ASSERT_EXIT( startsWith( "1234", "1234" ) );

        // Negatives
        PKTEST_ASSERT_EXIT( !startsWith( "test", "teststring" ) );
        PKTEST_ASSERT_EXIT( !startsWith( "teststring", "string" ) );
        PKTEST_ASSERT_EXIT( !startsWith( "teststring", "eststring" ) );

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( StrEndsWith )
    {
        // Positives
        PKTEST_ASSERT_EXIT( endsWith( "teststring", "g" ) );
        PKTEST_ASSERT_EXIT( endsWith( "stringtest", "test" ) );
        PKTEST_ASSERT_EXIT( endsWith( "1234", "1234" ) );

        // Negatives
        PKTEST_ASSERT_EXIT( !endsWith( "string", "teststring" ) );
        PKTEST_ASSERT_EXIT( !endsWith( "teststring", "test" ) );
        PKTEST_ASSERT_EXIT( !endsWith( "teststring", "teststrin" ) );

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ConvertStrToFourCC )
    {
        struct
        {
            const wchar_t* pch;
            uint32 expectedFourCC;
            HRESULT expectedResult;
        }
        static const rgEntries[] =
        {
            { L"",          MAKEFOURCC(   0,   0,   0,   0 ), pkE_INVALID_FORMAT },
            { L"a",         MAKEFOURCC(   0,   0,   0,   0 ), pkE_INVALID_FORMAT },
            { L"tY",        MAKEFOURCC(   0 ,  0,   0,   0 ), pkE_INVALID_FORMAT },
            { L"!?*",       MAKEFOURCC(   0,   0,   0,   0 ), pkE_INVALID_FORMAT },
            { L"5135",      MAKEFOURCC( '5', '1', '3', '5' ), pkS_OK },
            { L"51356",     MAKEFOURCC(   0,   0,   0,   0 ), pkE_INVALID_FORMAT  },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            uint32 fourCC;

            PKTEST_ASSERT_EXIT( StrToFourCC( rgEntries[i].pch, &fourCC ) == rgEntries[i].expectedResult );
            PKTEST_ASSERT_EXIT( fourCC == rgEntries[i].expectedFourCC );
        }

    exit:
        return;
    }

     ////////////////////////////////////
    PKTEST_METHOD( ConvertFourCCToWStr )
    {
        struct
        {
            uint32 FourCC;
            std::wstring expectedWStr;
            HRESULT expectedResult;
        }
        static const rgEntries[] =
        {
            { MAKEFOURCC(   0,   0,   0,   0 ), L"", pkE_INVALID_FORMAT },
            { MAKEFOURCC( 'A',   0,   0,   0 ), L"", pkE_INVALID_FORMAT },
            { MAKEFOURCC( 'A', 'B',   0,   0 ), L"", pkE_INVALID_FORMAT },
            { MAKEFOURCC( 'A', 'B', 'C',   0 ), L"", pkE_INVALID_FORMAT },
            { MAKEFOURCC( 'A', 'B', 'C', 'D' ), L"ABCD", pkS_OK },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            std::wstring wStrFourCC;

            PKTEST_ASSERT_EXIT( FourCCToStr( rgEntries[i].FourCC, &wStrFourCC ) == rgEntries[i].expectedResult );
            PKTEST_ASSERT_EXIT( wStrFourCC == rgEntries[i].expectedWStr );
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ConvertFourCCToStr )
    {
        struct
        {
            uint32 FourCC;
            std::string expectedStr;
            HRESULT expectedResult;
        }
        static const rgEntries[] =
        {
            { MAKEFOURCC(   0,   0,   0,   0 ), "", pkE_INVALID_FORMAT },
            { MAKEFOURCC( 'A',   0,   0,   0 ), "", pkE_INVALID_FORMAT },
            { MAKEFOURCC( 'A', 'B',   0,   0 ), "", pkE_INVALID_FORMAT },
            { MAKEFOURCC( 'A', 'B', 'C',   0 ), "", pkE_INVALID_FORMAT },
            { MAKEFOURCC( 'A', 'B', 'C', 'D' ), "ABCD", pkS_OK },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            std::string strFourCC;

            PKTEST_ASSERT_EXIT( FourCCToStr( rgEntries[i].FourCC, &strFourCC ) == rgEntries[i].expectedResult );
            PKTEST_ASSERT_EXIT( strFourCC == rgEntries[i].expectedStr );
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ConvertByteArrayToWStr )
    {
        static const uint8 rgSrc[] = { 0x00, 0x01, 0x20, 0xf5, 0x4e };
        static const wchar_t strExpected[] = L"000120F54E";

        std::wstring strOut;

        for( size_t cb = 0; cb <= sizeof(rgSrc); ++cb )
        {
            PKTEST_HRESULT_EXIT( BytesToHexStr( rgSrc, cb, &strOut ) );

            PKTEST_ASSERT_EXIT( strOut.compare( 0, strOut.size(), strExpected, strOut.size() ) == 0 );
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( ReplaceInPlace )
    {
        struct
        {
            const char* pszInput;
            const char* pszFrom;
            const char* pszTo;
            const char* pszExpected;
        }
        static const rgEntries[] =
        {
            { "the {0} fox", "{0}", "brown", "the brown fox" },
            { "replace {this} and {this} with {this}", "{this}", "that", "replace that and that with that" },
            { "the fox %verb% over the dog", "%verb%", "jumped", "the fox jumped over the dog" },
            { "nothing to replace here", "replaces", "", "nothing to replace here" },
            { "delete this", " this", "", "delete" },
        };

        for( size_t iEntry = 0; iEntry < sizeof(rgEntries)/sizeof(rgEntries[0]); ++iEntry )
        {
            std::string str = rgEntries[iEntry].pszInput;

            StrReplaceInPlace( &str, rgEntries[iEntry].pszFrom, rgEntries[iEntry].pszTo );

            PKTEST_ASSERT_EXIT( str == rgEntries[iEntry].pszExpected );
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( Str2WStrConversions )
    {
        PKTEST_ASSERT_EXIT( Str2WStr( "wug" ) == L"wug" );
        PKTEST_ASSERT_EXIT( Str2WStr( "wug" ).length() == 3 );
        PKTEST_ASSERT_EXIT( Str2WStr( "wug" ).size() == 3 );

        PKTEST_ASSERT_EXIT( Str2WStr( "" ) == L"" );
        PKTEST_ASSERT_EXIT( Str2WStr( "" ).length() == 0 );
        PKTEST_ASSERT_EXIT( Str2WStr( "" ).size() == 0 );

        PKTEST_ASSERT_EXIT( Str2WStr( std::string( "blicket" ) ) == L"blicket" );

        PKTEST_ASSERT_EXIT( WStr2Str( L"dax" ) == "dax" );
        PKTEST_ASSERT_EXIT( WStr2Str( L"dax" ).length() == 3 );
        PKTEST_ASSERT_EXIT( WStr2Str( L"dax" ).size() == 3 );

        PKTEST_ASSERT_EXIT( WStr2Str( L"" ) == "" );
        PKTEST_ASSERT_EXIT( WStr2Str( L"" ).length() == 0 );
        PKTEST_ASSERT_EXIT( WStr2Str( L"" ).size() == 0 );

        PKTEST_ASSERT_EXIT( WStr2Str( std::wstring( L"pimwit" ) ) == "pimwit" );

    exit:

        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( StrToLower )
    {        
        PKTEST_ASSERT_EXIT( toLower( "" ) == "" );
        PKTEST_ASSERT_EXIT( toLower( "ABC" ) == "abc" );
        PKTEST_ASSERT_EXIT( toLower( "abc" ) == "abc" );
        PKTEST_ASSERT_EXIT( toLower( "1ABc" ) == "1abc" );
        PKTEST_ASSERT_EXIT( toLower( "-abC1" ) == "-abc1" );

    exit:
        return;
    }

     ////////////////////////////////////
    PKTEST_METHOD( StrToUpper )
    {        
        PKTEST_ASSERT_EXIT( toUpper( "" ) == "" );
        PKTEST_ASSERT_EXIT( toUpper( "ABC" ) == "ABC" );
        PKTEST_ASSERT_EXIT( toUpper( "abc" ) == "ABC" );
        PKTEST_ASSERT_EXIT( toUpper( "1ABc" ) == "1ABC" );
        PKTEST_ASSERT_EXIT( toUpper( "-abC1" ) == "-ABC1" );

    exit:
        return;
    }

};
