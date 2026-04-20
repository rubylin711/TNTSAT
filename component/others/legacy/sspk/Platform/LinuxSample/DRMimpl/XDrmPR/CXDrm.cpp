///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

// Implementation of IXDrm for LinuxSample

// NOTE: A Linux-compatible version of WMDRM-MD headers is required for compilation 
//       and a Linux build of a WMDRM-MD library must be available for linking.

#include "pkPAL.h"
#include "pkExecutive.h"

#include "strsafe.h"

#include <new>
#include <map>
#include <string>


#include "IPTVDecoderHal.h"

#include "AutoLock.h"

#include "CXHttp.h"

#include "CXDrm.h"

#include <drmbuild_linux.h>
#include <drmsal.h>
#include <drmmanager.h>
#include <drmcdmiimpl.h>
#include <drmsoapxmlutility.h>

#include <drmbytemanip.h>
#include <drmtypes.h>
#include <drmconstants.h>
#include <drmrevocation.h>
#include <drmxmlparser.h>
#include <drmbytemanip.h>
#include <oemcocktail.h>

//#define DEBUG
#ifdef DEBUG
//#define CXDRM_TRACE_ENABLE
#endif

#if defined(CXDRM_TRACE_ENABLE)
#define CXDRM_LOG_MESSAGE( printf_exp ) ( Executive_DebugPrintf printf_exp )
#else
#define CXDRM_LOG_MESSAGE( printf_exp )   ( void ) 0
#endif

#define CXDRM_LOG_MESSAGE_ERROR( printf_exp ) ( Executive_DebugPrintf printf_exp )

#if defined(CXDRM_TRACE_VERBOSE)
#define CXDRM_LOG_MESSAGE_VERBOSE( printf_exp ) ( Executive_DebugPrintf printf_exp )
#else
#define CXDRM_LOG_MESSAGE_VERBOSE( printf_exp )   ( void ) 0
#endif

#define SIZEOF( x ) ( sizeof ( x ) )

static const uint32_t DRM_TRANSACTION_LOOP_DELAY = 200;

static const uint32_t MAXIMUM_LICACQ_RESPONSE_BODY_SIZE = 1000000;

// ~100 KB to start * 64 (2^6) ~= 6.4 MB, don't allocate more than ~6.4 MB
#define MAXIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE ( 64 * MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE )

#define CXDRM_LICENSE_STORAGE_FILE "/wmdrmpd/drmstore.dat"

//used to set drm cert and key path
//exaple bleafcert.dat zlpriv_protected.dat loacted 
//in ../../general/sample/,
//Oem_Set_DrmPath("../../general/sample/")
#ifdef __cplusplus
extern "C"{
#endif

    extern DRM_RESULT Oem_Set_DrmPath(DRM_CHAR *f_drmPath);

#ifdef __cplusplus
}
#endif
// ================================================================

struct CompareDRMID
{
    bool operator() ( const DRM_ID &lhs, const DRM_ID &rhs ) const
    { 
        return memcmp( &lhs, &rhs, sizeof( DRM_ID ) ) < 0; 
    }
};

typedef std::map<DRM_ID, DRM_DECRYPT_CONTEXT*, CompareDRMID> DecryptorMap;

// singleton Decryptor map 
static DecryptorMap g_oDecryptorMap;

// The following GUIDs are defined by the PlayReady compliance rule document.
DRM_DEFINE_GUID( ANALOG_VIDEO_AGC_OUTPUT_ID,                         0xC3FD11C6, 0xF8B7, 0x4D20, 0xB0, 0x08, 0x1D, 0xB1, 0x7D, 0x61, 0xF2, 0xDA );
DRM_DEFINE_GUID( ANALOG_VIDEO_EXPLICIT_OUTPUT_ID,                    0x2098DE8D, 0x7DDD, 0x4BAB, 0x96, 0xC6, 0x32, 0xEB, 0xB6, 0xFA, 0xBE, 0xA3 );
DRM_DEFINE_GUID( BEST_EFFORT_CGMSA_ID,                               0x225CD36F, 0xF132, 0x49EF, 0xBA, 0x8C, 0xC9, 0x1E, 0xA2, 0x8E, 0x43, 0x69 );
DRM_DEFINE_GUID( ANALOG_VIDEO_COMPONENT_OUTPUT_ID,                   0x811C5110, 0x46C8, 0x4C6E, 0x81, 0x63, 0xC0, 0x48, 0x2A, 0x15, 0xD4, 0x7E );
DRM_DEFINE_GUID( ANALOG_VIDEO_COMPUTER_MONITOR_OUTPUT_ID,            0xD783A191, 0xE083, 0x4BAF, 0xB2, 0xDA, 0xE6, 0x9F, 0x91, 0x0B, 0x37, 0x72 );
DRM_DEFINE_GUID( DIGITIAL_AUDIO_SCMS_ID,                             0x6D5CFA59, 0xC250, 0x4426, 0x93, 0x0E, 0xFA, 0xC7, 0x2C, 0x8F, 0xCF, 0xA6 );

extern "C" const char g_rchUrlDefault[] = "http://go.microsoft.com/fwlink/?LinkID=59833";

// ================================================================
// Class statics
// ================================================================

// lock for method operation:
Lockable CXDrm::s_OperationLock;

// Singleton operation:
Lockable CXDrm::s_FactoryLock;
int CXDrm::s_nRefCount = 0;
IXDrm* CXDrm::s_poXDrm = NULL;

//
// Factory method that creates an IXDrm instance. XDrm is implemented
// as a singleton so if it has already been created before, a reference
// to the extsing instance is returned and the reference count is
// incremented.
//
//
IXDRM_HRESULT CXDrm::_CreateInstance( IXDrm **ppXDrm )
{
    AutoLock lock(&s_FactoryLock);
    
    if ( s_poXDrm == NULL )
    {
        s_poXDrm = NEW_NO_THROW CXDrm;
        if (NULL == s_poXDrm)
        {
            return (IXDRM_HRESULT) pkE_UNEXPECTED;
        }
    }

    s_nRefCount++;
    *ppXDrm = s_poXDrm;
    
    return (IXDRM_HRESULT) pkS_OK;
}

//
// Destroy an IXDrm instance. First the reference count is decremented
// and if is reaches 0, the singleton XDrm instance is destroyed,
// otherwise the singleton XDrm instance is left alone.
//
IXDRM_HRESULT CXDrm::_DestroyInstance( IXDrm *pXDrm )
{
    AutoLock lock(&s_FactoryLock);
    
    if( pXDrm == NULL || pXDrm != s_poXDrm )
    {
        return (IXDRM_HRESULT) pkE_UNEXPECTED;
    }

    s_nRefCount--;
    if ( s_nRefCount == 0 )
    {
        delete s_poXDrm;
        s_poXDrm = NULL;
    }

    return (IXDRM_HRESULT) pkS_OK;
}

// ================================================================
// Private methods
// ================================================================

// Constructor/destructor

CXDrm::CXDrm() 
    : m_poAppContext( NULL )
    , m_pbOpaqueBuffer( NULL )
    , m_cbOpaqueBuffer( MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE )
    , m_pbRevocationBuffer( NULL )
    , m_pfnXDrmOPLCallback( NULL )
    , m_pvXDrmOPLCallbackContext( NULL )
    , m_fInTransaction(false)
    , m_fInit(false)
    , m_fInitDRM(false)
{ 
    CXDRM_LOG_MESSAGE( ("CXDrm::CXDrm object CREATED\n" ) );
}

CXDrm::~CXDrm() 
{ 
    int nDecryptorsFreed = g_oDecryptorMap.size();
    
    if ( m_poAppContext )
    {
        Drm_Uninitialize( ( DRM_APP_CONTEXT * )m_poAppContext );
        SAFE_OEM_FREE( m_poAppContext );
    }

    DecryptorMap::iterator it;
    for ( it = g_oDecryptorMap.begin(); it != g_oDecryptorMap.end(); ++it )
    {
        SAFE_OEM_FREE( it->second );
    }

    g_oDecryptorMap.clear();

    SAFE_OEM_FREE(m_pbOpaqueBuffer);
    SAFE_OEM_FREE(m_pbRevocationBuffer);

    CXDRM_LOG_MESSAGE( ("CXDrm::~CXDrm Decryptor count = %d\n", nDecryptorsFreed) );
}

// Initialize a XDrm instance.
// This performs all initialization EXCEPT Drm_Initialize
// which happens in _InitDRMIfRequired.  This ensures
// that Drm_Initialize, which may end up forcing Activation,
// never happens on a UI thread.
//
// Arguments:   none.
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::_Init()
{
    IXDRM_HRESULT dr = pkS_OK;

    // TODO: Oem_Init_Path??

    // Allocate/initialize the necessary buffers and data structures.
    ChkMem( m_pbOpaqueBuffer = ( uint8_t * )Oem_MemAlloc( m_cbOpaqueBuffer ) );
    ChkMem( m_pbRevocationBuffer = ( uint8_t * )Oem_MemAlloc( REVOCATION_BUFFER_SIZE ) );
    
    ChkMem( m_poAppContext = Oem_MemAlloc( SIZEOF( DRM_APP_CONTEXT ) ) );
    ZEROMEM( ( uint8_t * )m_poAppContext, SIZEOF( DRM_APP_CONTEXT ) );
    
ErrorExit:
    return dr;
}

//
// Initialize PlayReady DRM
// If Drm_Initialize has not yet been called, this function
// calls Drm_Initialize and Drm_Revocation_SetBuffer.
// Otherwise, it does nothing.
// If Drm_Initialize fails because Activation is required,
// synchronously performs Activation and then calls Drm_Initialize again.
//
// Preconditions:
//    Must be called within the operation lock
//
// Arguments:
// [f_pfJustInitialized] On output:
//                       Set to true if Drm_Initialize was called successfully
//                       regardless of whether Activation was performed or not.
//                       Set to false otherwise.
//
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::_InitDRMIfRequired( bool *f_pfJustInitialized )
{
    IXDRM_HRESULT dr = DRM_SUCCESS;
    
    *f_pfJustInitialized = false;

    DRM_WCHAR* wszURL = NULL;
    DRM_BYTE* pbChallenge = NULL;
    char* pszProcResponse = NULL;
    
    if( !m_fInitDRM )
    {
        // The default location of CDM DRM store.
        const DRM_WCHAR g_rgwchCDMDrmStoreName[] = {DRM_WCHAR_CAST('/'),  DRM_WCHAR_CAST('t'), DRM_WCHAR_CAST('m'), DRM_WCHAR_CAST('p'), DRM_WCHAR_CAST('/'),
                                                    DRM_WCHAR_CAST('d'), DRM_WCHAR_CAST('r'), DRM_WCHAR_CAST('m'), DRM_WCHAR_CAST('s'), DRM_WCHAR_CAST('t'),
                                                    DRM_WCHAR_CAST('o'), DRM_WCHAR_CAST('r'), DRM_WCHAR_CAST('e'), DRM_WCHAR_CAST('.'), DRM_WCHAR_CAST('d'),
                                                    DRM_WCHAR_CAST('a'), DRM_WCHAR_CAST('t'), DRM_WCHAR_CAST('\0')};
        const DRM_CONST_STRING g_dstrCDMDrmStoreName = DRM_CREATE_DRM_STRING(g_rgwchCDMDrmStoreName);

        DRM_SUBSTRING dsstrPath;
        
        DRM_CHAR rgchHDSPath[ MAX_PATH ];
        DRM_WCHAR rgwchHDSPath[ MAX_PATH ];
        DRM_CONST_STRING dstrHDSPath = DRM_EMPTY_DRM_STRING;    
        
        ZEROMEM(rgchHDSPath,MAX_PATH);
        
        ChkDR( StringCchPrintfA(
            (char*)rgchHDSPath,
            sizeof(rgchHDSPath),
            CXDRM_LICENSE_STORAGE_FILE) );
        
        // Convert the HDS path to DRM_STRING.
        dsstrPath.m_ich = 0;
        dsstrPath.m_cch = strlen( (char*)rgchHDSPath );
        
        dstrHDSPath.pwszString = rgwchHDSPath;
        dstrHDSPath.cchString = MAX_PATH;
        
        DRM_UTL_PromoteASCIItoUNICODE( rgchHDSPath,
                                       &dsstrPath,
                                       (DRM_STRING*) &dstrHDSPath );
        
        // TODO: perform synchronous activation if Drm_Initialize fails
        
        ChkDR( Drm_Initialize( ( DRM_APP_CONTEXT * )m_poAppContext,
                               NULL,
                               m_pbOpaqueBuffer,
                               m_cbOpaqueBuffer,
                               &g_dstrCDMDrmStoreName ) );

        ChkDR( Drm_Revocation_SetBuffer( ( DRM_APP_CONTEXT * )m_poAppContext,
                                         m_pbRevocationBuffer,
                                         REVOCATION_BUFFER_SIZE ) );
        
        // Perform secure clock setup
        CXDRM_LOG_MESSAGE(("_InitDRMIfRequired performing secure clock setup\n"));
        {
            // Note: Drm_SecureClock_GenerateChallenge returns a URL of http://go.microsoft.com/fwlink/?LinkId=25817
            //       which redirects to https://services.wmdrm.windowsmedia.com/SecureClock/?petition 
            //       So for now, hard code the URL:
            static const char s_szSecureClockURL[] = 
                //"http://services.wmdrm.windowsmedia.com/SecureClock/?Time";
                "http://securetime.playready.microsoft.com/securetime";
            static const char s_szSecureClockContentType[] = 
                "Content-Type: application/x-www-form-urlencoded\r\n";
#if 0
            //DRM_DWORD cchURL = 0;
            DRM_DWORD cbChallenge = 0;
            #if 0
            dr = Drm_SecureTime_GenerateChallenge( ( DRM_APP_CONTEXT * )m_poAppContext,
                &cchURL,
                &cbChallenge );
            
            if ( dr != DRM_E_BUFFERTOOSMALL )
            {
                ChkDR( dr );
                pkASSERT( false );    // Should never succeed on first call
                ChkDR( DRM_E_FAIL );
            }
            else
            {
                CXDRM_LOG_MESSAGE(("_InitDRMIfRequired cchURL: %d, cbChallenge: %d\n", cchURL, cbChallenge));
                
                ChkMem( pbChallenge = ( DRM_BYTE * )Oem_MemAlloc( cbChallenge + 1 ) );
                ZEROMEM( pbChallenge, cbChallenge + 1 );
                ChkMem( wszURL = ( DRM_WCHAR * )Oem_MemAlloc(sizeof(DRM_WCHAR) * cchURL) );
                ZEROMEM( wszURL, sizeof(DRM_WCHAR) * cchURL );
            }
            #endif
            ChkDR( Drm_SecureTime_GenerateChallenge( ( DRM_APP_CONTEXT * )m_poAppContext,
                &cbChallenge,
                &pbChallenge));

            pbChallenge[cbChallenge] = 0;

            // Pack down the wide string into a zero terminated char string in the same memory buffer
            //char* szSecureClockURL = (char*)wszURL;
            //for (size_t cch = 0; (cch < cchURL); cch++)
           // {
           //     szSecureClockURL[cch] = (char)wszURL[cch];
           // }
           // szSecureClockURL[cchURL] = 0;

            CXDRM_LOG_MESSAGE(("_InitDRMIfRequired performing SendHttp:\nURL: %s\nChallenge: %s\n", 
                s_szSecureClockURL, pbChallenge));
            
            ChkDR( SendHttp(
                s_szSecureClockContentType,
                (const char*) pbChallenge,
                &pszProcResponse,
                s_szSecureClockURL)); // szSecureClockURL, // see note above

            // CXDRM_LOG_MESSAGE(("_InitDRMIfRequired performing ProcessResponse:\n%s\n", pszProcResponse));
            
            DRM_RESULT drmResult;
            
            ChkDR( Drm_SecureTime_ProcessResponse( ( DRM_APP_CONTEXT * )m_poAppContext,
                (DRM_DWORD) strlen(pszProcResponse),
                (DRM_BYTE*) pszProcResponse));
#endif
        }
        
        m_fInitDRM = true;
        *f_pfJustInitialized = true;
    }

ErrorExit:

    if (DRM_FAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::_InitDRMIfRequired failed = 0x%x\n", dr ) );
    }    
    else
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::_InitDRMIfRequired succeeded; %s initialized\n", *f_pfJustInitialized ? "Just" : "Already" ) );
    }

    //MemFree(wszURL);
    //MemFree(pbChallenge);
    //MemFree(pszProcResponse);
    
    return dr;
}

// ================================================================
// IXDrm implementation methods
// ================================================================

//
// Initialize a XDrm instance. Call Drm_Reinitialize if the instance has
// already been initialized before.
//
// Arguments:
// [pfnCallback]    Policy callback.
// [pfnCallbackArg] Parameter to be passed to the policy callback.
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::Init( XDrmOPLCallbackPtr pfnCallback, void* pvCallbackContext )
{
    DRM_RESULT dr = DRM_SUCCESS;
    
    AutoLock lock(&s_OperationLock);

    if ( !m_fInit )
    {
        ChkDR( _Init() );
        m_fInit = true;
    }

    //
    // This function gets called a LOT with NULL callbacks,
    // but we only care about the first valid callback it's called with.
    // However, it is also repeatedly called with different callback args
    // as the key rotates and different CPlayReadyLicense objects are created.
    // Thus, the arg changes even as the callback remains the same.
    //
    if( pfnCallback != NULL )
    {
        if( m_pfnXDrmOPLCallback != NULL )
        {
            //
            // Callback is immutable once set
            //
            pkASSERT( m_pfnXDrmOPLCallback == pfnCallback );
        }
        else
        {
            m_pfnXDrmOPLCallback = pfnCallback;
        }
        m_pvXDrmOPLCallbackContext = pvCallbackContext;
    }
    else
    {
        //
        // Argument should always be null if callback is null
        //
        pkASSERT( pvCallbackContext == NULL );
    }

ErrorExit:
    
    if (DRM_FAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::Init failed = 0x%x\n", dr ) );
    }    
    else
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::Init succeeded\n") );
    }
    return dr;
}

IXDRM_HRESULT CXDrm::SetCertPath( const char*  ppszCertPath )
{
    DRM_RESULT dr                   = DRM_SUCCESS;
    if (ppszCertPath)
    {
        dr = Oem_Set_DrmPath((DRM_CHAR *)ppszCertPath);
        if (DRM_FAILED(dr))
        {
            CXDRM_LOG_MESSAGE( ("CXDrm::Init failed = 0x%x\n", dr ) );
        }
    }

    return dr;
}

//
// Forwards to CXDrm::OPLCallback after reformatting data into XDRM_OPL_DATA structure
//
/* old callback not used in DPK4.0
static DRM_RESULT DRM_CALL CXDrmPolicyCallback(
    IN const DRM_VOID                 *f_pvCallbackData,
    IN       DRM_POLICY_CALLBACK_TYPE  f_dwCallbackType,
    IN const DRM_VOID                 *f_pv )
    */
static DRM_RESULT DRM_CALL CXDrmPolicyCallback(
    __in     const DRM_VOID                 *f_pvCallbackData,
    __in           DRM_POLICY_CALLBACK_TYPE  f_dwCallbackType,
    __in_opt const DRM_KID                  *f_pKID,            /* KID that is being enumerated, i.e. the KID of the leaf-most license in a chain.  Will be NULL for callbacks not dealing with a license. */
    __in_opt const DRM_LID                  *f_pLID,            /* LID of the actual license being called upon, i.e. may be leaf or root license in a chain.  Will be NULL for callbacks not dealing with a license. */
    __in_opt const DRM_VOID                 *f_pv )            /* Void pointer to opaque data passed in alongside the DRMPFNPOLICYCALLBACK parameter which is then passed to the callback, e.g. in Drm_Reader_Bind */
{
    DRM_RESULT dr                   = DRM_SUCCESS;
    void *pvCallbackData            = NULL;

    pkASSERT( f_pv != NULL );
    CXDRM_LOG_MESSAGE(("[%s] new CallbackType = %d f_pv=0x%x\n",__FUNCTION__, f_dwCallbackType, f_pv));
    switch (f_dwCallbackType)
    {
        case DRM_PLAY_OPL_CALLBACK:
        {
            XDRM_OPL_DATA *pOPLData = NULL;

            ChkMem( pOPLData = ( XDRM_OPL_DATA * )Oem_MemAlloc( sizeof( XDRM_OPL_DATA ) ) );
            memset( pOPLData, 0, sizeof( XDRM_OPL_DATA ) );

            pvCallbackData = pOPLData;

            DRM_PLAY_OPL_EX2 *pOPL = ( DRM_PLAY_OPL_EX2 * )f_pvCallbackData;
            DRM_WORD wUncompressedVideoOPL = pOPL->minOPL.wUncompressedDigitalVideo;
            CXDRM_LOG_MESSAGE_ERROR(("CXDrmPolicyCallback minOPL.wCompressedDigitalVideo = %d\n", pOPL->minOPL.wCompressedDigitalVideo ));
            CXDRM_LOG_MESSAGE_ERROR(("CXDrmPolicyCallback minOPL.wUncompressedDigitalVideo = %d\n", pOPL->minOPL.wUncompressedDigitalVideo ));
            CXDRM_LOG_MESSAGE_ERROR(("CXDrmPolicyCallback minOPL.wAnalogVideo = %d\n", pOPL->minOPL.wAnalogVideo ));
            CXDRM_LOG_MESSAGE_ERROR(("CXDrmPolicyCallback minOPL.wCompressedDigitalAudio = %d\n", pOPL->minOPL.wCompressedDigitalAudio ));
            CXDRM_LOG_MESSAGE_ERROR(("CXDrmPolicyCallback minOPL.wUncompressedDigitalAudio = %d\n", pOPL->minOPL.wUncompressedDigitalAudio ));
            if ( wUncompressedVideoOPL <= 250 )
            {
                // No restriction.
            }
            else if ( wUncompressedVideoOPL <= 270 )
            {
                // Allow output to external display but the content has to down-res.
                pOPLData->HDCPAction = XDRM_OPL_ENABLE_DOWN_RES;
            }
            else
            {
                // Does not allow output to external display.
                pOPLData->HDCPAction = XDRM_OPL_ENABLE_ALWAYS;
            }
            
            // Check for explicit analog video/audio protections.
            // There are two categories of analog video/audio protections: One does not allow output if there
            // is an external display and the other down-res when there is an external display.
            // Since the behavior is the same as that of HDCP OPLs we are reusing HDCP bits to achieve the same goal. 
            for ( int i = 0; i < pOPL->vopi.cEntries; i++ )
            {
                if ( MEMCMP( &pOPL->vopi.rgVop[ i ].guidId, &ANALOG_VIDEO_COMPONENT_OUTPUT_ID, SIZEOF( DRM_GUID ) ) == 0 ||
                    MEMCMP( &pOPL->vopi.rgVop[ i ].guidId, &ANALOG_VIDEO_COMPUTER_MONITOR_OUTPUT_ID, SIZEOF( DRM_GUID ) ) == 0 )
                {
                    // Allow output to external display but the content has to down-res.
                    pOPLData->HDCPAction = XDRM_OPL_ENABLE_DOWN_RES;
                }
                else if ( MEMCMP( &pOPL->vopi.rgVop[ i ].guidId, &ANALOG_VIDEO_AGC_OUTPUT_ID, SIZEOF( DRM_GUID ) ) == 0 ||
                        MEMCMP( &pOPL->vopi.rgVop[ i ].guidId, &ANALOG_VIDEO_EXPLICIT_OUTPUT_ID, SIZEOF( DRM_GUID ) ) == 0 )
                {
                    // Does not allow output to external display.
                    pOPLData->HDCPAction = XDRM_OPL_ENABLE_ALWAYS;
                }
                else if ( MEMCMP( &pOPL->vopi.rgVop[ i ].guidId, &BEST_EFFORT_CGMSA_ID, SIZEOF( DRM_GUID ) ) == 0 )
                {
                    // Do nothing for "Best effort".
                }
            }

            for ( int i = 0; i < pOPL->aopi.cEntries; i++ )
            {
                if ( MEMCMP( &pOPL->aopi.rgAop[ i ].guidId, &DIGITIAL_AUDIO_SCMS_ID, SIZEOF( DRM_GUID ) ) == 0 )
                {
                    // Does not allow output to external display.
                    pOPLData->HDCPAction = XDRM_OPL_ENABLE_ALWAYS;
                }
            }
            ChkDR( ((CXDrm*)f_pv)->OPLCallback( pOPLData ) );
            break;
        }
        default:
        {
            //ChkDR( DRM_E_NOTIMPL );
            CXDRM_LOG_MESSAGE_ERROR(("[%s] CallbackType=%d not supported.\n",__FUNCTION__, f_dwCallbackType));
            ChkDR( DRM_SUCCESS );
            break;
        }
    }
    
ErrorExit:
    SAFE_OEM_FREE( pvCallbackData );
    return dr;
}

//
// Forwards to m_pfnXDrmOPLCallback 
//
IXDRM_HRESULT CXDrm::OPLCallback( XDRM_OPL_DATA *f_pOPLData )
{
    if(m_pfnXDrmOPLCallback == NULL)
    {
        CXDRM_LOG_MESSAGE_ERROR(("[%s]err:cxdrm call m_pfnXDrmOPLCallback,but it don't register m_pfnXDrmOPLCallback and return drm_success\n",__FUNCTION__));
        return( DRM_SUCCESS );
    }
    pkASSERT( m_pfnXDrmOPLCallback != NULL );
    return m_pfnXDrmOPLCallback( f_pOPLData, m_pvXDrmOPLCallbackContext );
}

//
// Set the rights to be performed.
//
// Arguments:
// [ulRights]   The rights to be performed.
//
// Returns:
// DRM_SUCCESS.
//
IXDRM_HRESULT CXDrm::SetRights(
    unsigned long ulRights )
{
    // Always succeed.
    return( DRM_SUCCESS );
}

//
// Set the current DRM header.
//
// Arguments:
// [cbHdr]      Size of the DRM header.
// [pbHdr]      Pointer to a buffer containing the DRM header.
// [cbKeyID]    Size of an optional key id.
// [pbKeyID]    (Optional) pointer to a buffer containing the key id.
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::SetEnhancedData(
        size_t cbHdr,
        const uint8_t* pbHdr,
        size_t cbKeyID,
        const uint8_t* pbKeyID ) 
{ 
    IXDRM_HRESULT dr = pkS_OK;

    bool fJustInitialized = false;
    DRM_WCHAR *pwchB64KeyID     = NULL;
    
    CXDRM_LOG_MESSAGE( ("CXDrm::SetEnhancedData cbHdr %u, pbHdr %p, cbKeyID %u, pbKeyID %p\n", 
        cbHdr, pbHdr, cbKeyID, pbKeyID) );
    
    _RequireTransaction();

    AutoLock lock(&s_OperationLock);

    ChkDR( _InitDRMIfRequired( &fJustInitialized ) );

    if( !fJustInitialized )
    {
        // Need to call Reinitialize to enable setting a new header.
        ChkDR( Drm_Reinitialize( ( DRM_APP_CONTEXT * )m_poAppContext ) );
    }

    if ( pbKeyID != NULL && cbKeyID > 0 )
    {
        DRM_CSP_PLAYREADY_OBJ_WITH_KID_DATA oData = { 0 };

        DRM_DWORD cchB64KeyID = CCH_BASE64_EQUIV( cbKeyID );

        ChkMem( pwchB64KeyID = ( DRM_WCHAR * )Oem_MemAlloc( cchB64KeyID * SIZEOF( DRM_WCHAR ) ) );
        ChkDR( DRM_B64_EncodeW( pbKeyID,
                                cbKeyID,
                                pwchB64KeyID,
                                &cchB64KeyID,
                                0 ) );

        oData.pbKeyID = ( DRM_BYTE * )pwchB64KeyID;
        oData.cbKeyID = cchB64KeyID * SIZEOF( DRM_WCHAR );
        oData.pbHeaderData = pbHdr;
        oData.cbHeaderData = cbHdr;

        ChkDR( Drm_Content_SetProperty(
                    ( DRM_APP_CONTEXT * )m_poAppContext,
                    DRM_CSP_PLAYREADY_OBJ_WITH_KID,
                    ( DRM_BYTE * )&oData,
                    SIZEOF( oData ) ) );                    

    }
    else
    {
        ChkDR( Drm_Content_SetProperty(
                    ( DRM_APP_CONTEXT * )m_poAppContext,
                    DRM_CSP_AUTODETECT_HEADER,
                    pbHdr,
                    cbHdr ) );
    }

ErrorExit:
    if (DRM_FAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::SetEnhancedData Drm_Content_SetProperty failed = 0x%x\n", dr ) );
    }    
    return dr;
}

//
// Retrieve the decrypt context based on key id. If key id
// is NULL then a blank key id is internally used. A new
// decrypt context is allocated if currently there is not one
// that matches the requested key id.
//
// Arguments:
// [cbKeyID]    Size of a key id.
// [pbKeyID]    Pointer to a buffer containing the key id.
// [ppvDecryptContext]  Pointer to a pointer to receive the decrypt context.
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::AcquireDecryptContext(
        size_t cbKeyID,
        const uint8_t* pbKeyID,
        void **ppvDecryptContext ) 
{ 
    IXDRM_HRESULT dr = pkS_OK;
    
    DecryptorMap::iterator it;
    DRM_ID oKID;

    AutoLock lock(&s_OperationLock);

    pkASSERT(m_fInitDRM);

    if ( pbKeyID != NULL )
    {
        ChkArg( cbKeyID == SIZEOF( DRM_ID ) );
        MEMCPY( &oKID, pbKeyID, cbKeyID );
    }
    else
    {
        ZEROMEM( &oKID, SIZEOF( DRM_ID ) );
    }

    it = g_oDecryptorMap.find( oKID );

    if ( it != g_oDecryptorMap.end() )
    {
        DRM_DECRYPT_CONTEXT *poContext = it->second;
        ChkBOOL( poContext != NULL, DRM_E_FAIL );
        *ppvDecryptContext = poContext;
    }
    else
    {
        DRM_DECRYPT_CONTEXT *poContext = NULL;
        ChkMem( poContext = ( DRM_DECRYPT_CONTEXT * )Oem_MemAlloc( SIZEOF( DRM_DECRYPT_CONTEXT ) ) );
        ZEROMEM( poContext, SIZEOF( DRM_DECRYPT_CONTEXT ) );
        g_oDecryptorMap.insert( std::make_pair( oKID, poContext ) );
        *ppvDecryptContext = poContext;

        if( pbKeyID )
        {
            DRM_GUID *pguid = (DRM_GUID *)pbKeyID;
            CXDRM_LOG_MESSAGE( ("CXDrm::AcquireDecryptContext KID = {%08X-%04X-%04X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X}\n",
                                  pguid->Data1, pguid->Data2, pguid->Data3,
                                  pguid->Data4[0], pguid->Data4[1], pguid->Data4[2], pguid->Data4[3], 
                                  pguid->Data4[4], pguid->Data4[5], pguid->Data4[6], pguid->Data4[7] ) );
        }
        CXDRM_LOG_MESSAGE( ("CXDrm::AcquireDecryptContext Decryptor count = %d\n", g_oDecryptorMap.size() ) );
    }

ErrorExit:
    return dr;
}

//
// Release the decrypt context related to a key id (created in
// AcquireDecryptContext).
//
// Arguments:
// [cbKeyID]    Size of a key id.
// [pbKeyID]    Pointer to a buffer containing the key id.
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::ReleaseDecryptContext(
        size_t cbKeyID,
        const uint8_t* pbKeyID ) 
{ 
    IXDRM_HRESULT dr = pkS_OK;
    
    DecryptorMap::iterator it;
    DRM_ID oKID;
    
    AutoLock lock(&s_OperationLock);

    if ( !m_fInitDRM )
    {
        goto ErrorExit;
    }


    if ( pbKeyID != NULL )
    {
        DRM_GUID *pguid = (DRM_GUID *)pbKeyID;

        ChkArg( cbKeyID == SIZEOF( DRM_ID ) );
        MEMCPY( &oKID, pbKeyID, cbKeyID );

        CXDRM_LOG_MESSAGE( ("CXDrm::ReleaseDecryptContext KID = {%08X-%04X-%04X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X}\n",
                              pguid->Data1, pguid->Data2, pguid->Data3,
                              pguid->Data4[0], pguid->Data4[1], pguid->Data4[2], pguid->Data4[3], 
                              pguid->Data4[4], pguid->Data4[5], pguid->Data4[6], pguid->Data4[7] ) );
    }
    else
    {
        ZEROMEM( &oKID, SIZEOF( DRM_ID ) );
    }

    it = g_oDecryptorMap.find( oKID );

    if ( it != g_oDecryptorMap.end() )
    {
        DRM_DECRYPT_CONTEXT *poContext = it->second;
        Oem_MemFree( poContext );
        g_oDecryptorMap.erase( it );

        CXDRM_LOG_MESSAGE( ("CXDrm::ReleaseDecryptContext Decryptor count = %d\n", g_oDecryptorMap.size() ) );
    }
    else
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::ReleaseDecryptContext FAILED TO FIND KID [%02X%02X%02X%02X]\n", 
            oKID.rgb[3], oKID.rgb[2], oKID.rgb[1], oKID.rgb[0] ) );
    }

ErrorExit:
    return dr;
}


//
// Perform the DRM bind function to locate a license of a
// piece of content with a specific rights.
//
// Arguments:
// [pvDecryptContext]   Pointer to a decrypt context to use.
// [fAbortPlayback]     Flag indicating whether playback should be aborted if error occurrs.
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::CanDecrypt( void *pvDecryptContext, bool fAbortPlayback )
{
    DRM_RESULT dr = DRM_SUCCESS;
    uint8_t *pbNewOpaqueBuffer = NULL;
    const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrDRM_RIGHT_PLAYBACK };

    _RequireTransaction();

    AutoLock lock(&s_OperationLock);
    
    pkASSERT( m_fInitDRM );

    pkASSERT( m_cbOpaqueBuffer >= MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE ); // sanity check

    ChkArg( pvDecryptContext != NULL );
    CXDRM_LOG_MESSAGE( ("CXDrm::CanDecrypt this= 0x%x\n", this ) );
    CXDRM_LOG_MESSAGE( ("CXDrm::CanDecrypt m_pfnXDrmOPLCallback= 0x%x\n", this->m_pfnXDrmOPLCallback ) );
    CXDRM_LOG_MESSAGE( ("CXDrm::CanDecrypt m_pvXDrmOPLCallbackContext= 0x%x\n", this->m_pvXDrmOPLCallbackContext ) );
    CXDRM_LOG_MESSAGE( ("CXDrm::CanDecrypt Drm_Reader_Bind start\n") );

    while
    (
        ( dr = Drm_Reader_Bind(
                ( DRM_APP_CONTEXT * )m_poAppContext,
                rgstrRights,
                1,
                //(m_pfnXDrmOPLCallback != NULL) ? (DRMPFNPOLICYCALLBACK)CXDrmPolicyCallback : NULL,
                //(m_pfnXDrmOPLCallback != NULL) ? this : NULL,
                (DRMPFNPOLICYCALLBACK)CXDrmPolicyCallback,
                (DRM_VOID *)this,
                (DRM_DECRYPT_CONTEXT*) pvDecryptContext )
        ) == DRM_E_BUFFERTOOSMALL
    )
    {
        uint32_t cbNewOpaqueBuffer = m_cbOpaqueBuffer * 2;
        pkASSERT( cbNewOpaqueBuffer > m_cbOpaqueBuffer ); // overflow check
        
        if( cbNewOpaqueBuffer > MAXIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE )
        {
            ChkDR( DRM_E_OUTOFMEMORY );
        }

        ChkMem( pbNewOpaqueBuffer = ( uint8_t* )Oem_MemAlloc( cbNewOpaqueBuffer ) );

        ChkDR( Drm_ResizeOpaqueBuffer(
            ( DRM_APP_CONTEXT* )m_poAppContext,
            pbNewOpaqueBuffer,
            cbNewOpaqueBuffer ) );

        //
        // Free the old buffer and then transfer the new buffer ownership
        // Free must happen after Drm_ResizeOpaqueBuffer because that
        // function assumes the existing buffer is still valid
        //
        SAFE_OEM_FREE( m_pbOpaqueBuffer );
        m_cbOpaqueBuffer = cbNewOpaqueBuffer;
        m_pbOpaqueBuffer = pbNewOpaqueBuffer;
        pbNewOpaqueBuffer = NULL;
    }

    if ( DRM_FAILED( dr ) )
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::CanDecrypt Drm_Reader_Bind failed = 0x%x\n", dr ) );
        goto ErrorExit;
    }
    else
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::CanDecrypt Drm_Reader_Bind succeeded.\n") );
    }

ErrorExit:
    SAFE_OEM_FREE( pbNewOpaqueBuffer );

    return dr;
}


//
// Decrypt a subsample buffer.
//
// Arguments:
// [pvDecryptContext]   Pointer to a decrypt context to use.
// [pbData]     Pointer to a buffer containing the subsample data.
// [cbData]     Size of the buffer mentioned above.
// [fIsAES]     Currently it must be true.
// [qwSampleID] The Sample ID associated with the subsample.
// [qwOffset]   Offset of the subsample within the sample it belongs to.
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::Decrypt(
        void *pvDecryptContext,
        uint8_t* pbData,
        size_t cbData,
        bool fIsAES,
        uint64_t qwSampleID,
        uint64_t qwOffset ) 
{ 
    IXDRM_HRESULT dr = pkS_OK;
    DRM_AES_COUNTER_MODE_CONTEXT ctrModeCtx;
    
    CXDRM_LOG_MESSAGE_VERBOSE( ("CXDrm::Decrypt [%p] cbData: %u iv:%llu offset:%llu\n", 
        pvDecryptContext, cbData, qwSampleID, qwOffset ) );

    AutoLock lock(&s_OperationLock);
    
    pkASSERT( m_fInitDRM );

    ChkArg( pvDecryptContext != NULL );

    ChkBOOL( fIsAES, DRM_E_FAIL );

    memcpy( &ctrModeCtx.qwInitializationVector, ( DRM_BYTE * )&qwSampleID, sizeof( uint64_t ) );
    ctrModeCtx.qwBlockOffset = (uint64_t)qwOffset / 16;
    ctrModeCtx.bByteOffset = qwOffset % 16;

    ChkDR( Drm_Reader_DecryptLegacy ( (DRM_DECRYPT_CONTEXT*)pvDecryptContext,
                               &ctrModeCtx,
                               pbData,
                               cbData ) );

ErrorExit:
    if (pkFAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::Decrypt [%p] FAILED [0x%X]\n", 
            pvDecryptContext, dr )); 
    }
    return dr;
}

//
// Decrypt a chain of subsamples. The list entry is in the form of
// IPTV_HAL_BUFFER. The implementation only uses the input list to
// decrypt the data in place.
//
// Arguments:
// [pvDecryptContext]   Pointer to a decrypt context to use.
// [pInBufList]     Input list of subsample buffers to be decrypted.
// [pOutBufList]    Output list of subsample buffers after being decrypted.
// [fIsAES]         Currently it must be true.
// [qwSampleID]     The Sample ID associated with the subsample.
// [qwOffset]       Offset of the subsample within the sample it belongs to.
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::DecryptBufferChain(
        void *pvDecryptContext,
        void* pInBufList,
        void* pOutBufList,
        bool fIsAES,
        uint64_t qwSampleID,
        uint64_t qwOffset ) 
{ 
    IXDRM_HRESULT dr = pkS_OK;
    DRM_AES_COUNTER_MODE_CONTEXT ctrModeCtx;
    uint64_t qwOffset1 = qwOffset;    
    DRM_DECRYPT_CONTEXT *poContext = ( DRM_DECRYPT_CONTEXT * )pvDecryptContext;
    
    CXDRM_LOG_MESSAGE_VERBOSE( ("CXDrm::DecryptBufferChain [%p] in:%p out:%p iv:%llu offset:%llu\n", 
        pvDecryptContext, pInBufList, pOutBufList, qwSampleID, qwOffset ) );
    
    AutoLock lock(&s_OperationLock);
    
    pkASSERT( m_fInitDRM );

    ChkArg( pvDecryptContext != NULL );

    ChkArg( pInBufList != NULL && pOutBufList != NULL );
    
    ChkBOOL( fIsAES, DRM_E_FAIL );

    // loop through the input buffer chain
    {
        PIPTV_HAL_BUFFER pBufListItem = (PIPTV_HAL_BUFFER) pInBufList;

        while (NULL != pBufListItem)
        {
            if ( ( ( pBufListItem->u32Flags & IPTV_HAL_BUFFER_FLAG_DECRYPT ) != 0 ) )
            {
                uint32_t cbSize;
        
                memcpy( &ctrModeCtx.qwInitializationVector, ( DRM_BYTE * )&qwSampleID, sizeof( uint64_t ) );
                ctrModeCtx.qwBlockOffset =  (uint64_t)qwOffset1 / 16;
                ctrModeCtx.bByteOffset = qwOffset1 % 16;
        
                cbSize = pBufListItem->u32DataEnd - pBufListItem->u32DataStart;
        
                ChkDR( Drm_Reader_DecryptLegacy ( poContext,
                                           &ctrModeCtx,
                                           pBufListItem->pBuf + pBufListItem->u32DataStart,
                                           cbSize ) );
        
                qwOffset1 += cbSize;
            }
        
            pBufListItem = pBufListItem->pNext;
        }
    }

ErrorExit:
    if (pkFAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::DecryptBufferChain [%p] FAILED [0x%X]\n", 
            pvDecryptContext, dr )); 
    }
    return dr;
}

//
// Commit the bind operation.
//
// Arguments:   none.
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::Commit() 
{ 
    IXDRM_HRESULT dr = pkS_OK;
    
    CXDRM_LOG_MESSAGE( ("CXDrm::Commit\n" ) ); 
    
    _RequireTransaction();

    AutoLock lock(&s_OperationLock);

    pkASSERT( m_fInitDRM );

    ChkDR( Drm_Reader_Commit( ( DRM_APP_CONTEXT * )m_poAppContext, NULL, NULL ) );

ErrorExit:
    if (pkFAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::Commit [%p] FAILED [0x%X]\n", 
            m_poAppContext, dr )); 
    }
    return dr;
}

//
// Generate a license acquisition challenge. Both the generated Url and challenge strings are
// NULL terminated.  Also cleans up stale licenses.
//
// Arguments:
// [ppszUrl]    Pointer to a pointer of a buffer to store the license acquisition Url.
//              It's caller's responsibility to release the buffer via MemFree after usage.
// [pszCustomData]  Pointer to a custom data string to be sent along with the challenge.
// [ppszChallenge]  Pointer to a pointer of a buffer to store the license acquisition challenge.
//              It's caller's responsibility to release the buffer via MemFree after usage.
// [fAllowCustomDataOverride] Boolean to indicate whether to retrieve the custom data from the player. Not Used.
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::GenerateChallenge(
    char** ppszUrl,
    const char* pszCustomData,
    char** ppszChallenge,
    bool fAllowCustomDataOverride )
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_BYTE *pbChallenge = NULL;
    DRM_CHAR *pchURL = NULL;
    DRM_CHAR *pszCustomDataUsed = NULL;
    DRM_DWORD cchCustomDataUsed = 0;
    DRM_DWORD cbChallenge = 0;
    DRM_DWORD cchURL = 0;

    _RequireTransaction();

    AutoLock lock(&s_OperationLock);
    
    pkASSERT( m_fInitDRM );

    ChkArg( ppszUrl != NULL && ppszChallenge != NULL );

    if ((NULL != pszCustomData) && (0 != pszCustomData[0]))
    {
        pszCustomDataUsed = ( DRM_CHAR * )pszCustomData;
        cchCustomDataUsed = (DRM_DWORD) strlen(pszCustomData);
    }

    dr = Drm_LicenseAcq_GenerateChallenge(
            ( DRM_APP_CONTEXT * )m_poAppContext,
            NULL,
            0,
            NULL,
            pszCustomDataUsed,
            cchCustomDataUsed,
            NULL,
            &cchURL,
            NULL,
            NULL,
            NULL,
            &cbChallenge,
            NULL );

    if ( dr == DRM_E_BUFFERTOOSMALL )
    {
        ChkMem( pchURL = ( DRM_CHAR * )Oem_MemAlloc( cchURL + 1 ) );
        ZEROMEM( pchURL, cchURL + 1 );

        ChkMem( pbChallenge = ( DRM_BYTE * )Oem_MemAlloc( cbChallenge + 1 ) );
        ZEROMEM( pbChallenge, cbChallenge + 1 );

        ChkDR( Drm_LicenseAcq_GenerateChallenge(
                    ( DRM_APP_CONTEXT * )m_poAppContext,
                    NULL,
                    0,
                    NULL,
                    pszCustomDataUsed,
                    pszCustomDataUsed != NULL ? strlen( (char*)pszCustomDataUsed ) : 0,
                    pchURL,
                    &cchURL,
                    NULL,
                    NULL,
                    pbChallenge,
                    &cbChallenge,
                    NULL ) );

        pbChallenge[ cbChallenge ] = 0;
    }
    else
    {
        ChkDR( dr );
        pkASSERT( false );    // Should never succeed on first call
        ChkDR( DRM_E_FAIL );
    }

    // Cleanup stale licenses.
    ChkDR( Drm_StoreMgmt_CleanupStore( ( DRM_APP_CONTEXT * )m_poAppContext,
                                       DRM_STORE_CLEANUP_DELETE_REMOVAL_DATE_LICENSES,
                                       NULL,
                                       0,
                                       NULL ) );

    *ppszUrl = (char*)pchURL;
    pchURL = NULL;
    *ppszChallenge = (char*)pbChallenge;
    pbChallenge = NULL;

ErrorExit:
    SAFE_OEM_FREE( pchURL );
    SAFE_OEM_FREE( pbChallenge );
    
    if (pkFAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::GenerateChallenge [%p] FAILED [0x%X]\n", 
            m_poAppContext, dr )); 
    }
    return dr;
}


//
// Process a license acquisition response.
//
// Arguments:
// [pszResponse]    Pointer to buffer containing the license acquisition response.
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::ProcessResponse(
    const char* pszResponse )
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_LICENSE_RESPONSE oResponse;
    int nLen;

    //
    // NOTE: This method does not use the wmrm header
    //       therefore _RequireTransaction should not be called
    //
    AutoLock lock(&s_OperationLock);
    
    pkASSERT( m_fInitDRM );

    ChkArg( pszResponse != NULL );

    nLen = strlen( pszResponse );

    ZEROMEM( &oResponse, SIZEOF( DRM_LICENSE_RESPONSE ) );

    ChkDR( Drm_LicenseAcq_ProcessResponse( ( DRM_APP_CONTEXT * )m_poAppContext,
                                           //DRM_PROCESS_LIC_RESPONSE_SIGNATURE_NOT_REQUIRED,
                                           //NULL,
                                           //NULL,
                                           DRM_PROCESS_LIC_RESPONSE_NO_FLAGS,
                                           ( uint8_t * )pszResponse,
                                           nLen,
                                           &oResponse ) );

ErrorExit:
    if (pkFAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::ProcessResponse [%p] FAILED [0x%X]\n%s\n", 
            m_poAppContext, dr, pszResponse )); 
    }
    return dr;
}

IXDRM_HRESULT CXDrm::SendHttp(
        const char* szContentType,
        const char* szRequest,
        char** pszResponse,
        const char* szURL) 
{ 
    DRM_RESULT dr = DRM_SUCCESS;

    CXHttp* poHttp = NULL;
    
    std::string httpBody = szRequest;
    std::string httpServer;
    std::string resource = szURL;
    std::string httpRequest;

    CXDRM_LOG_MESSAGE( ("CXDrm::SendHttp URL: %s\n", szURL ) ); 
    
    // NOTE: does NOT _RequireTransaction();

    // Parse out the httpServer and resource strings
    {
        size_t ixProto = resource.find("https://");
        printf("find https:// resource:%ld\n",ixProto);
        if(ixProto == 0)
        {
         resource = resource.substr(ixProto+8, resource.size() - (ixProto+8));
        }
        else
        {
         size_t ixProto = resource.find("http://");
         printf("find http:// resource:%ld\n",ixProto);
         if(ixProto == 0)
           {
             resource = resource.substr(ixProto+7, resource.size() - (ixProto+7));
           }
           else
           {
             printf("cxdrm err:SendHttp resource url is invalid!,url:%s\n",szURL);
             ChkBOOL(0 == ixProto, DRM_E_INVALIDARG);
             return DRM_E_INVALIDARG;
           }
        }
        
        size_t ixUri = resource.find_first_of('/');

        ChkBOOL(std::string::npos != ixUri, DRM_E_INVALIDARG);
        httpServer = resource.substr(0, ixUri);
        resource = resource.substr(ixUri, resource.size()-(ixUri));
    }
    
    // Create the request header string
    {
        const int uintStrSize = 12; // max string size of %u
        char contentSizeStr[uintStrSize];
        
        StringCbPrintfA(contentSizeStr, uintStrSize, "%u", (unsigned int) httpBody.size());
        
        httpRequest =  "POST " + resource + " HTTP/1.1\r\n";
        httpRequest += 
            "Accept: */*\r\n"
            "Accept-Language: en-US\r\n"
            "Content-Length: "
            ;
        httpRequest += contentSizeStr;
        httpRequest += 
            "\r\n"
            //"User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)\r\n"
            //"Pragma: no-cache\r\n"
            ;
        httpRequest += "Host: " + httpServer + "\r\n";
        
        if (NULL != szContentType)
        {
            httpRequest += szContentType;
        }
        else
        {
            httpRequest += "Content-Type: text/xml; charset=utf-8\r\n";
        }

        // finally, a second cr/lf in a row signals end of headers
        httpRequest += "\r\n"; 
    }

    // Perform the HTTP transaction
    {
        char *pchResponseBody = NULL;
        
        poHttp = NEW_NO_THROW CXHttp;
        ChkMem( poHttp );
        
        std::string responseHeader;

        bool isResponseOk = poHttp->HttpRequestResponse(httpServer, httpRequest, httpBody, responseHeader);

        if (isResponseOk)
        {
            CXDRM_LOG_MESSAGE_VERBOSE( ("CXDrm::SendHttp challenge(%d):\n%s\n===RESPONSE HEADER===(%d)\n%s\n", httpBody.length(),
                httpBody.c_str(), responseHeader.length(), responseHeader.c_str()) ); 
        }

        ChkBOOL( isResponseOk, DRM_E_FAIL );

        uint32_t cbContent = poHttp->GetContentLength();

        ChkBOOL( cbContent < MAXIMUM_LICACQ_RESPONSE_BODY_SIZE, DRM_E_BUFFERTOOSMALL );

        ChkMem( pchResponseBody = (char*)Oem_MemAlloc( cbContent + 1 ) );

        // Recv response body into buffer
        
        uint32_t ixContent = 0;
        while (ixContent < cbContent)
        {
            int rc = poHttp->Recv((byte*)&pchResponseBody[ixContent], cbContent - ixContent);
            if (rc <= 0)
            {
                CXDRM_LOG_MESSAGE( ("Receiving response body failed rc: %d len: %d received: %d contentLen: %d\n", 
                    rc, 
                    cbContent - ixContent, 
                    ixContent, 
                    poHttp->GetContentLength()));
                
                MemFree(pchResponseBody);
                dr = DRM_E_FAIL;
                goto ErrorExit;
            }
            ixContent += rc;
        }
        pkASSERT(ixContent == cbContent);

        pchResponseBody[cbContent] = 0; // null terminate the response body

        CXDRM_LOG_MESSAGE_VERBOSE( ("CXDrm::SendHttp response (%u):\n%s\n", cbContent, pchResponseBody ) ); 

        *pszResponse = pchResponseBody;
        // Note: *pszResponse must be freed by caller using IXDrm::MemFree
    }
    
ErrorExit:

    delete poHttp;
    
    if(!pkSUCCEEDED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::SendHttp failed: 0x%X\n", dr ) ); 
    }
    return dr;
}

//
// Reset the XDrm instance.
//
// Arguments:   none.
//
// Returns: none.
//
void CXDrm::Reset() 
{ 
    /* do nothing */ 
}

//
// Release a buffer allocated by Oem_MemAlloc or other compatible ways.
//
// Arguments:
// [ptr]    Pointer to the buffer being released.
//
// Returns: none.
//
void CXDrm::MemFree( void* ptr )
{
    SAFE_OEM_FREE( ptr );
}

//
// Returns the DRM version code.
//
// Arguments:   none.
//
// Returns: XDRM_VERSIONCODE_PLAYREADY.
//
XDRM_VERSION_CODE CXDrm::GetDRMVersionCode() 
{ 
    return XDRM_VERSIONCODE_PLAYREADY;
}

//
// Retrive various properties from the content header.
// Currently only XDRM_PROP_DECRYPTORSETUP is supported.
//
// Arguments:
// [eProperty]      Type of property to be retrieved.
// [pbProperty]     Pointer to a buffer to receive the requested property.
// [pcbProperty]    Pointer to a variable that contains the size of the buffer
//                  during input and receives the actual size of the buffer used
//                  during output.
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::GetContentProperty(
    XDRM_CONTENT_PROPERTY eProperty,
    uint8_t* pbProperty,
    uint32_t* pcbProperty )
{
    IXDRM_HRESULT dr = pkE_FAIL;

    _RequireTransaction();

    AutoLock lock(&s_OperationLock);
    
    pkASSERT( m_fInitDRM );

    switch ( eProperty )
    {
        case XDRM_PROP_DECRYPTORSETUP:
            
            dr = Drm_Content_GetProperty(
                    ( DRM_APP_CONTEXT * )m_poAppContext,
                    DRM_CGP_DECRYPTORSETUP,
                    pbProperty,
                    pcbProperty );
            if ( dr == DRM_E_CH_INVALID_HEADER || dr == DRM_E_XMLNOTFOUND )
            {
                // Simply means there is no such property.
                dr = DRM_SUCCESS;
            }
            else
            {
                ChkDR( dr );
            }
            break;

        default:
            dr = pkE_FAIL;
    }

ErrorExit:
    return( dr );
}


//
// Extract the property data containing in a DRM protocol response
// (e.g. license response).
//
// Arguments:
// [pbResponse] Pointer to a buffer containing the DRM protocol response.
// [cbResponse] Size of the buffer mentioned above.
// [eProperty]  Type of property data to retrieve.
// [ppszCustomData] Pointer to a pointer of a buffer that receives the property
//              data in the response. The data in the buffer is a NULL terminated
//              string. If there is no property data in the response the pointer to
//              the buffer is NULL.
//
// Returns:
// DRM_SUCCESS if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::GetPropertyFromResponse(
    const uint8_t *pbResponse,
    size_t cbResponse,
    XDRM_RESPONSE_PROPERTY eProperty,
    char **ppszPropertyData )
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_CHAR *pchData = NULL;
    DRM_DWORD cchData = 0;
    DRM_DWORD dwType = DRM_GARD_CUSTOM_DATA;

    //
    // This method does not use the wmrm header
    // so _RequireTransaction should not be called
    //
    AutoLock lock(&s_OperationLock);
    
    pkASSERT( m_fInitDRM );

    ChkArg( pbResponse != NULL && cbResponse > 0 );
    ChkArg( ppszPropertyData != NULL );

    *ppszPropertyData = NULL;
    
    switch ( eProperty )
    {
        case XDRM_RESPONSE_CUSTOM_DATA:
            dwType = DRM_GARD_CUSTOM_DATA;
            break;

        case XDRM_RESPONSE_REDIRECT_URL:
            dwType = DRM_GARD_REDIRECT_URL;
            break;

        default:
            ChkDR( DRM_E_INVALIDARG );
            break;
    }
    
    dr = Drm_GetAdditionalResponseData(
        ( DRM_APP_CONTEXT * )m_poAppContext,
        pbResponse,
        cbResponse,
        dwType,
        NULL,
        &cchData );

    if ( dr == DRM_E_BUFFERTOOSMALL )
    {
        ChkMem( pchData = ( DRM_CHAR * )Oem_MemAlloc( cchData + 1 ) );
        ZEROMEM( pchData, cchData + 1 );

        ChkDR( Drm_GetAdditionalResponseData(
                ( DRM_APP_CONTEXT * )m_poAppContext,
                pbResponse,
                cbResponse,
                dwType,
                pchData,
                &cchData ) );

        pchData[ cchData ] = 0;
    }
    else if ( dr == DRM_E_XMLNOTFOUND )
    {
        dr = DRM_SUCCESS;
    }
    else
    {
        ChkDR( dr );
    }
    *ppszPropertyData = (char*)pchData;

    pchData = NULL;

ErrorExit:
    SAFE_OEM_FREE( pchData );
    
    return( dr );
}


void CXDrm::BeginTransaction()
{
    //
    // Wait for any existing transaction to complete before starting another one
    //
    s_OperationLock.Lock();

    // TODO: should there be some kind of failure if this wait goes on too long?
    while( m_fInTransaction )
    {
        s_OperationLock.Unlock();
        Executive_Sleep( DRM_TRANSACTION_LOOP_DELAY );
        s_OperationLock.Lock();
    }
    m_fInTransaction = true;
    
    s_OperationLock.Unlock();
}

void CXDrm::EndTransaction()
{
    s_OperationLock.Lock();
    
    if( !m_fInTransaction )
    {
        pkASSERT( false );
    }
    m_fInTransaction = false;
    
    s_OperationLock.Unlock();
}

void CXDrm::_RequireTransaction()
{
    bool fInTransaction = false;
    s_OperationLock.Lock();
    fInTransaction = m_fInTransaction;
    s_OperationLock.Unlock();
    if( !fInTransaction )
    {
        pkASSERT( false );
    }
}

//
// ========================================================================
// IXDrm factory API implementation
// ========================================================================

IXDRM_HRESULT WINAPI XDRM_CreateInstance( IXDrm **ppXDrm )
{
    return CXDrm::_CreateInstance(ppXDrm);
}

//
// Destroy an IXDrm instance. First the reference count is decremented
// and if is reaches 0, the singleton XDrm instance is destroyed,
// otherwise the singleton XDrm instance is left alone.
//
IXDRM_HRESULT WINAPI XDRM_DestroyInstance( IXDrm *pXDrm )
{
    return CXDrm::_DestroyInstance(pXDrm);
}

