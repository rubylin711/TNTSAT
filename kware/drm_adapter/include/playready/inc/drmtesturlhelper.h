/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __DRMTESTURLHELPER_H__
#define __DRMTESTURLHELPER_H__

#include <drmtypes.h>
#include <drmsal.h>

PREFAST_PUSH_DISABLE_EXPLAINED( __WARNING_URL_NEEDS_REVIEW_25085, "Test URLs do not need review." )
PREFAST_PUSH_DISABLE_EXPLAINED( __WARNING_USE_SELECT_ANY_25046, "Static variables cannot be __declspec(selectany)" )

/*
** MSFT:11892245:PC: PK4.0: Some scenario tests are blocked from using playready-testserver.azurewebsites.net
** Multiple bugs in the test server are currently being worked around in these test cases.
** As they get fixed, lines of code referencing this error code (in comments or not)
** should be updated as the associated comments specify.
** Once they are ALL fixed, this error code should be removed entirely.
*/
#define DRM_E_WORKAROUND_BUG_MSFT_11892245_IN_TEST_SERVER   DRM_E_NOTIMPL

/*
** Note: set DRMTESTURLHELPER_DEBUG_JSON to 1 to have the test server
** ONLY return the converted JSon which can easily be viewed in fiddler
*/
#define DRMTESTURLHELPER_DEBUG_JSON                        0

/*
** Note: Set one (but not more than one) of these values to force all tests to use the provided server.
*/
/* DRM_E_WORKAROUND_BUG_MSFT_11892245_IN_TEST_SERVER: public server does not yet support all necessary policy - set all of these to 0 when fixed */
#define DRMTESTURLHELPER_DEBUG_USING_PR_STAGING_SERVER     0
#define DRMTESTURLHELPER_DEBUG_USING_EXPERIMENTAL1_SERVER  1

/*
** If you want to use your own server (e.g. localhost), then define
** DDRMTESTURLHELPER_INITIALIZE_URL_FORCE_OVERRIDE_URL yourself as shown in the following example.
** #define DRMTESTURLHELPER_INITIALIZE_URL_FORCE_OVERRIDE_URL "localhost/playready_3_3_4450/rightsmanager.asmx"
*/

/*
** Set this to TRUE if the server being hit uses the legacy XMR builder.
** Set this to FALSE if the server being hit uses the xbinary XMR builder.
*/
#define DRMTESTURLHELPER_SERVER_IS_USING_LEGACY_XMR_BUILDER  FALSE

static DRM_CHAR  s_szLicAcqUrl[ 1024 ] = { 0 };     /* 1 kb is enough for any URL */
static DRM_DWORD s_idxUrl = 0;

#define DRMTESTURLHELPER_USING_LICENSE_SERVER ( s_idxUrl != 0 )

#ifdef DRMTESTURLHELPER_INITIALIZE_URL_FORCE_OVERRIDE_URL
#define DRMTESTURLHELPER_INITIALIZE_URL_FORCE_OVERRIDE( __pszUrl ) __pszUrl = DRMTESTURLHELPER_INITIALIZE_URL_FORCE_OVERRIDE_URL
#else
#if DRMTESTURLHELPER_DEBUG_USING_PR_STAGING_SERVER && DRMTESTURLHELPER_DEBUG_USING_EXPERIMENTAL1_SERVER
#error Cannot set both  DRMTESTURLHELPER_DEBUG_USING_PR_STAGING_SERVER  and  DRMTESTURLHELPER_DEBUG_USING_EXPERIMENTAL1_SERVER
#elif DRMTESTURLHELPER_DEBUG_USING_PR_STAGING_SERVER
#define DRMTESTURLHELPER_INITIALIZE_URL_FORCE_OVERRIDE( __pszUrl ) __pszUrl = "prtsSTAG-rightsmanager.azurewebsites.net/service/rightsmanager.asmx"
#elif DRMTESTURLHELPER_DEBUG_USING_EXPERIMENTAL1_SERVER
#define DRMTESTURLHELPER_INITIALIZE_URL_FORCE_OVERRIDE( __pszUrl ) __pszUrl = "experimental1.azurewebsites.net/rightsmanager.asmx"
#else
#define DRMTESTURLHELPER_INITIALIZE_URL_FORCE_OVERRIDE( __pszUrl )
#endif
#endif  /* DRMTESTURLHELPER_INITIALIZE_URL_FORCE_OVERRIDE_URL */

#if DRMTESTURLHELPER_DEBUG_JSON
#define DRMTESTURLHELPER_INITIALIZE_URL_QUERY_STRING   "?debug=json&cfg="
#else
#define DRMTESTURLHELPER_INITIALIZE_URL_QUERY_STRING   "?cfg="
#endif

#define DRMTESTURLHELPER_CLEAR_LACQ_URL() DRM_DO {                  \
    ChkVOID( ZEROMEM( s_szLicAcqUrl, sizeof( s_szLicAcqUrl ) ) );   \
    s_idxUrl = 0;                                                   \
} DRM_WHILE_FALSE

#define DRMTESTURLHELPER_INITIALIZE_LACQ_URL( __pszUrl ) DRM_DO {                                      \
    DRMTESTURLHELPER_APPENDURLINTERNAL_ALWAYS( "http://" );                                            \
    DRMTESTURLHELPER_INITIALIZE_URL_FORCE_OVERRIDE( __pszUrl );                                        \
    DRMTESTURLHELPER_APPENDURLINTERNAL_ALWAYS( __pszUrl );                                             \
    DRMTESTURLHELPER_APPENDURLINTERNAL_ALWAYS( DRMTESTURLHELPER_INITIALIZE_URL_QUERY_STRING );         \
} DRM_WHILE_FALSE

#define DRMTESTURLHELPER_APPENDURLINTERNAL_ALWAYS( __str ) DRM_DO {                 \
    if( __str[ 0 ] != '\0' )                                                        \
    {                                                                               \
        DRM_DWORD __cch = 0;                                                        \
        ChkDR( DRM_SizeTToDWord( DRMCRT_strlen( __str ), &__cch ) );                \
        AssertChkBOOL( s_idxUrl + __cch + 1 < sizeof( s_szLicAcqUrl ) );            \
        ChkVOID( MEMCPY( &s_szLicAcqUrl[ s_idxUrl ], __str, __cch ) );              \
        s_idxUrl += __cch;                                                          \
    }                                                                               \
} DRM_WHILE_FALSE

#define DRMTESTURLHELPER_APPENDURLINTERNAL_STRING( __str ) DRM_DO {                 \
    if( DRMTESTURLHELPER_USING_LICENSE_SERVER )                                     \
    {                                                                               \
        DRMTESTURLHELPER_APPENDURLINTERNAL_ALWAYS( __str );                         \
    }                                                                               \
} DRM_WHILE_FALSE

/* Base-10, no padding */
#define DRMTESTURLHELPER_APPENDURLINTERNAL_DWORD( __value ) DRM_DO {                \
    if( DRMTESTURLHELPER_USING_LICENSE_SERVER )                                     \
    {                                                                               \
        DRM_CHAR __rgch[ DRM_MAX_CCH_BASE10_DWORD_STRING + 1 ] = { 0 };             \
        ChkDR( DRM_UTL_NumberToStringA(                                             \
            __value, __rgch, DRM_NO_OF( __rgch ), 0, 10, NULL ) );                  \
        DRMTESTURLHELPER_APPENDURLINTERNAL_STRING( __rgch );                        \
    }                                                                               \
} DRM_WHILE_FALSE

/* Base-16, pad to two character, i.e. 00 to FF */
#define DRMTESTURLHELPER_APPENDURLINTERNAL_BYTE( __value ) DRM_DO {                 \
    if( DRMTESTURLHELPER_USING_LICENSE_SERVER )                                     \
    {                                                                               \
        DRMASSERT( __value <= 0xFF );                                               \
        DRM_CHAR __rgch[ DRM_MAX_CCH_BASE10_DWORD_STRING + 1 ] = { 0 };             \
        ChkDR( DRM_UTL_NumberToStringA(                                             \
            __value, __rgch, DRM_NO_OF( __rgch ), 2, 16, NULL ) );                  \
        DRMTESTURLHELPER_APPENDURLINTERNAL_STRING( __rgch );                        \
    }                                                                               \
} DRM_WHILE_FALSE

#define DRMTESTURLHELPER_APPENDURLINTERNAL_CHAR( __char ) DRM_DO {                  \
    if( DRMTESTURLHELPER_USING_LICENSE_SERVER )                                     \
    {                                                                               \
        AssertChkBOOL( s_idxUrl + 1 < sizeof( s_szLicAcqUrl ) );                    \
        s_szLicAcqUrl[ s_idxUrl++ ] = ( __char );                                   \
    }                                                                               \
} DRM_WHILE_FALSE

/* Removes the {} created by DRM_UTL_GuidToString during the call to DRM_UTL_DemoteUNICODEtoASCII */
#define DRMTESTURLHELPER_APPENDURLINTERNAL_GUID( __pguid ) DRM_DO {                 \
    if( DRMTESTURLHELPER_USING_LICENSE_SERVER )                                     \
    {                                                                               \
        DRM_WCHAR __rgwch[ DRM_GUID_STRING_LEN + 1 ] = { 0 };                       \
        DRM_CHAR  __rgch[ DRM_GUID_STRING_LEN - 1 ]  = { 0 };                       \
        ChkDR( DRM_UTL_GuidToString(                                                \
            DRM_REINTERPRET_CAST( const DRM_GUID, __pguid ), __rgwch ) );           \
        ChkVOID( DRM_UTL_DemoteUNICODEtoASCII(                                      \
            &__rgwch[ 1 ], __rgch, DRM_GUID_STRING_LEN - 2 ) );                     \
        DRMTESTURLHELPER_APPENDURLINTERNAL_STRING( __rgch );                        \
    }                                                                               \
} DRM_WHILE_FALSE

#define DRMTESTURLHELPER_APPENDURLINTERNAL_STARTPOLICY( __str ) DRM_DO {            \
    if( DRMTESTURLHELPER_USING_LICENSE_SERVER )                                     \
    {                                                                               \
        AssertChkBOOL( s_idxUrl > 0 );                                              \
        DRMTESTURLHELPER_APPENDURLPOLICY_CONTINUEARRAY_FRIENDLY_NAMED( "..." );     \
        DRMTESTURLHELPER_APPENDURLINTERNAL_STRING( __str );                         \
    }                                                                               \
} DRM_WHILE_FALSE

#define DRMTESTURLHELPER_APPENDURLPOLICY_STRING( __param, __value ) DRM_DO {        \
    if( DRMTESTURLHELPER_USING_LICENSE_SERVER )                                     \
    {                                                                               \
        DRMTESTURLHELPER_APPENDURLINTERNAL_STARTPOLICY( __param );                  \
        DRMTESTURLHELPER_APPENDURLINTERNAL_CHAR( ':' );                             \
        DRMTESTURLHELPER_APPENDURLINTERNAL_STRING( __value );                       \
    }                                                                               \
} DRM_WHILE_FALSE

/* Examples:  0  ,  1  ,  17  ,  326  ,  4000000001 */
#define DRMTESTURLHELPER_APPENDURLPOLICY_DWORD( __param, __value ) DRM_DO {         \
    if( DRMTESTURLHELPER_USING_LICENSE_SERVER )                                     \
    {                                                                               \
        DRMTESTURLHELPER_APPENDURLINTERNAL_STARTPOLICY( __param );                  \
        DRMTESTURLHELPER_APPENDURLINTERNAL_CHAR( ':' );                             \
        DRMTESTURLHELPER_APPENDURLINTERNAL_DWORD( __value );                        \
    }                                                                               \
} DRM_WHILE_FALSE

#define DRMTESTURLHELPER_APPENDURLPOLICY_BOOL( __param, __value ) DRM_DO {          \
    if( DRMTESTURLHELPER_USING_LICENSE_SERVER )                                     \
    {                                                                               \
        DRMTESTURLHELPER_APPENDURLINTERNAL_STARTPOLICY( __param );                  \
        DRMTESTURLHELPER_APPENDURLINTERNAL_CHAR( ':' );                             \
        if ( __value  )                                                             \
        {                                                                           \
            DRMTESTURLHELPER_APPENDURLINTERNAL_STRING( "true" );                    \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            DRMTESTURLHELPER_APPENDURLINTERNAL_STRING( "false" );                   \
        }                                                                           \
    }                                                                               \
} DRM_WHILE_FALSE

/* Example:  E296B58E-1866-6EF6-FECF-8DB44AD5FC24 */
#define DRMTESTURLHELPER_APPENDURLPOLICY_GUID( __param, __value ) DRM_DO {          \
    if( DRMTESTURLHELPER_USING_LICENSE_SERVER )                                     \
    {                                                                               \
        DRMTESTURLHELPER_APPENDURLINTERNAL_STARTPOLICY( __param );                  \
        DRMTESTURLHELPER_APPENDURLINTERNAL_CHAR( ':' );                             \
        DRMTESTURLHELPER_APPENDURLINTERNAL_GUID( __value );                         \
    }                                                                               \
} DRM_WHILE_FALSE

/* Example 16 bytes:  04_CC_EC_12_41_42_45_16_47_34_A0_B3_A2_D0_00_36 */
#define DRMTESTURLHELPER_APPENDURLPOLICY_BYTES( __param, __value, __size ) DRM_DO { \
    if( DRMTESTURLHELPER_USING_LICENSE_SERVER )                                     \
    {                                                                               \
        DRM_DWORD __idx;                                                            \
        DRMTESTURLHELPER_APPENDURLINTERNAL_STARTPOLICY( __param );                  \
        DRMTESTURLHELPER_APPENDURLINTERNAL_CHAR( ':' );                             \
        for( __idx = 0; __idx < ( __size ) - 1; __idx++ )                           \
        {                                                                           \
            DRMTESTURLHELPER_APPENDURLINTERNAL_BYTE( ( __value )[ __idx ] );        \
            DRMTESTURLHELPER_APPENDURLINTERNAL_CHAR( '_' );                         \
        }                                                                           \
        DRMTESTURLHELPER_APPENDURLINTERNAL_BYTE( ( __value )[ __idx ] );            \
    }                                                                               \
} DRM_WHILE_FALSE

/* Example Jan 5th, 2017: 20170105 */
#define DRMTESTURLHELPER_APPENDURLPOLICY_SYSTEMTIME( __param, __value ) DRM_DO {    \
    if( DRMTESTURLHELPER_USING_LICENSE_SERVER )                                     \
    {                                                                               \
        DRMTESTURLHELPER_APPENDURLPOLICY_DWORD( __param, __value.wYear );           \
        if( __value.wMonth < 10 )                                                   \
        {                                                                           \
            DRMTESTURLHELPER_APPENDURLINTERNAL_DWORD( 0 );                          \
        }                                                                           \
        DRMTESTURLHELPER_APPENDURLINTERNAL_DWORD( __value.wMonth );                 \
        if( __value.wDay < 10 )                                                     \
        {                                                                           \
            DRMTESTURLHELPER_APPENDURLINTERNAL_DWORD( 0 );                          \
        }                                                                           \
        DRMTESTURLHELPER_APPENDURLINTERNAL_DWORD( __value.wDay );                   \
    }                                                                               \
} DRM_WHILE_FALSE

#define DRMTESTURLHELPER_APPENDURLPOLICY_OPENARRAY_FRIENDLY_NAMED( __param, __friendlyname ) DRM_DO {   \
    if( __param[ 0 ] != '\0' )                                                                          \
    {                                                                                                   \
        DRMTESTURLHELPER_APPENDURLPOLICY_STRING( __param, "" );                                         \
    }                                                                                                   \
    DRMTESTURLHELPER_APPENDURLINTERNAL_CHAR( '(' );                                                     \
} DRM_WHILE_FALSE

#define DRMTESTURLHELPER_APPENDURLPOLICY_CONTINUEARRAY_FRIENDLY_NAMED( __friendlyname ) DRM_DO {        \
    AssertChkBOOL( s_idxUrl > 0 );                                                                      \
    if( s_szLicAcqUrl[ s_idxUrl - 1 ] != '(' )                                                          \
    {                                                                                                   \
        DRMTESTURLHELPER_APPENDURLINTERNAL_CHAR( ',' );                                                 \
    }                                                                                                   \
} DRM_WHILE_FALSE

#define DRMTESTURLHELPER_APPENDURLPOLICY_CLOSEARRAY_FRIENDLY_NAMED( __friendlyname ) DRMTESTURLHELPER_APPENDURLINTERNAL_CHAR( ')' )

PREFAST_POP  /* __WARNING_USE_SELECT_ANY_25046 */
PREFAST_POP  /* __WARNING_URL_NEEDS_REVIEW_25085 */

#endif  /* __DRMTESTURLHELPER_H__ */

