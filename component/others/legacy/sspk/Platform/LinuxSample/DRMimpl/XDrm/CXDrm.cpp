///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

// Stub implementation of IXDrm

// NOTE: This is a stub implementation for the purpose of verifying code flow
//       with *NO DECRYPTION* actually taking place.

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

#ifdef DEBUG
#define CXDRM_TRACE_ENABLE
#endif
// #define CXDRM_TRACE_VERBOSE

#if defined(CXDRM_TRACE_ENABLE)
#define CXDRM_LOG_MESSAGE( printf_exp ) ( Executive_DebugPrintf printf_exp )
#else
#define CXDRM_LOG_MESSAGE( printf_exp )   ( void ) 0
#endif

#if defined(CXDRM_TRACE_VERBOSE)
#define CXDRM_LOG_MESSAGE_VERBOSE( printf_exp ) ( Executive_DebugPrintf printf_exp )
#else
#define CXDRM_LOG_MESSAGE_VERBOSE( printf_exp )   ( void ) 0
#endif

static const uint32_t DRM_TRANSACTION_LOOP_DELAY = 200;

static const uint32_t MAXIMUM_LICACQ_RESPONSE_BODY_SIZE = 100000;

static const size_t c_MaxDrmHeaderSize = 4000;

#define ChkBOOL(fExpr,err){  \
            if (!(fExpr))            \
            {\
                dr = (err);\
                goto ErrorExit;     \
            }\
        }

#define ChkMem(expr) {               \
            if ( NULL == (expr) )    \
            {                        \
                dr = pkE_OUTOFMEMORY; \
                goto ErrorExit;     \
            }                       \
        }

// ================================================================
// Normally defined in DRM porting kit
// ================================================================

#define Oem_MemAlloc( size ) Executive_Alloc(size, false)

#define Oem_MemFree( ptr ) Executive_Free(ptr)

#define DRM_ID_SIZE  16

typedef struct _tagDRM_ID
{
    uint8_t rgb [DRM_ID_SIZE];
} DRM_ID;

// ================================================================

// Stub Decrypt context handling

typedef struct
{
    DRM_ID oKID;
    uint32_t crc32PSSH;
} _DRM_DECRYPT_CONTEXT;

struct CompareDRMID
{
    bool operator() ( const DRM_ID &lhs, const DRM_ID &rhs ) const
    {
        return memcmp( &lhs, &rhs, sizeof( DRM_ID ) ) < 0;
    }
};

typedef std::map<DRM_ID, _DRM_DECRYPT_CONTEXT*, CompareDRMID> DecryptorMap;

// singleton Decryptor map
static DecryptorMap g_oDecryptorMap;

static uint32_t g_crc32LatestPSSH;

// Key rotation detection:

static uint32_t DRM_DECRYPTORSETUP_ONDEMAND = 1;

static bool g_isOnDemand = false;

static std::string g_LicenseServerURI;

// ================================================================
// Local helpers
// ================================================================

#define CRC32_POLYNOMIAL    0x04c11db7
#define CRC32_INITIALIZE    0xffffffff
//
// Update the running CRC 32 with a new value.
//
// Arguments:
// [crc]    Current CRC 32 value.
// [value]  The new value to be applied to the current CRC 32.
//
// Returns:
// An updated CRC 32 value.
//
static uint32_t UpdateCRC32(
    uint32_t crc,
    uint8_t value )
{
    uint32_t masking, carry;

    masking = 1 << 8;
    while ( ( masking >>= 1 ) )
    {
        carry = crc & 0x80000000;
        crc <<= 1;
        if( ! carry ^ ! ( value & masking ) )
		{
			crc ^= CRC32_POLYNOMIAL;
		}
    }
    return crc;
}

//
// Calculate a CRC 32 value of a byte buffer.
//
// Arguments:
// [dwCRC]      Starting value of the CRC 32 value (start with CRC32_INITIALIZE)
// [pbBuffer]   The byte buffer used to calculate the CRC 32.
// [dwLength]   Size of the buffer mentioned above.
//
// Returns:
// The CRC 32 value of the buffer.
//
static uint32_t ComputeCRC32(
    uint32_t dwCRC,
    const uint8_t* pbBuffer,
    size_t dwLength )
{
    const uint8_t* pbEnd = pbBuffer + dwLength;

    while ( pbBuffer != pbEnd )
    {
        dwCRC = UpdateCRC32( dwCRC, *pbBuffer++ );
    }

    return dwCRC;
}

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
    : m_pfnXDrmPolicyCallback(NULL)
    , m_pfnXDrmPolicyCallbackContext(NULL)
    , m_fInTransaction(false)
    , m_fInit(false)
    , m_fInitDRM(false)
    , m_poIXDrmDiagDelegate(NULL)
{
    CXDRM_LOG_MESSAGE( ("CXDrm::CXDrm IXDrm object CREATED\n" ) );
}

CXDrm::~CXDrm()
{
    CXDRM_LOG_MESSAGE( ("CXDrm::~CXDrm Decryptor count = %d\n", g_oDecryptorMap.size() ) );

    DecryptorMap::iterator it;
    for ( it = g_oDecryptorMap.begin(); it != g_oDecryptorMap.end(); ++it )
    {
        Oem_MemFree( it->second );
    }

    g_oDecryptorMap.clear();
}

// Initialize a XDrm instance.
// This performs all initialization EXCEPT DRM porting kit Initialize
// which happens in _InitDRMIfRequired.
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

    // stub does nothing and returns OK

    return dr;
}

//
// Initialize PlayReady DRM
// If DRM porting kit Initialize has not yet been called, this function
// calls it and Revocation Set Buffer.
// Otherwise, it does nothing.
// If DRM Initialize fails because Activation is required, it should
// synchronously performs Activation and then call Initialize again.
//
// Preconditions:
//    Must be called within the operation lock
//
// Arguments:
// [f_pfJustInitialized] On output:
//                       Set to true if DRM Initialize was called successfully
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
    IXDRM_HRESULT dr = pkS_OK;

    *f_pfJustInitialized = false;

    if( !m_fInitDRM )
    {
        // Note: should perform DRM porting kit Initialize and Revocation Set Buffer

        m_fInitDRM = true;
        *f_pfJustInitialized = true;
    }

    return dr;
}

// ================================================================
// IXDrm implementation methods
// ================================================================

IXDRM_HRESULT CXDrm::Init( XDrmOPLCallbackPtr pfnCallback, void* pvCallbackContext )
{
    IXDRM_HRESULT dr = pkS_OK;

    AutoLock lock(&s_OperationLock);

    if (!m_fInit)
    {
        dr = _Init();
        if (pkFAILED(dr))
        {
            CXDRM_LOG_MESSAGE( ("CXDrm::_Init FAILED: 0x%X\n", dr ) );
            goto errExit;
        }
        m_fInit = true;
    }

    //
    // This function gets called a LOT with NULL callbacks,
    // but we only care about the first valid callback it's called with.
    // However, it is also repeatedly called with different callback args
    // as the key rotates and different CPlayReadyLicense objects are created.
    // Thus, the arg changes even as the callback remains the same.
    //
    if( NULL == pfnCallback )
    {
        //
        // Argument should always be null if callback is null
        //
        pkASSERT( pvCallbackContext == NULL );
    }
    else
    {
        if( m_pfnXDrmPolicyCallback != NULL )
        {
            //
            // Callback is immutable once set
            //
            pkASSERT( m_pfnXDrmPolicyCallback == pfnCallback );
        }
        else
        {
            m_pfnXDrmPolicyCallback = pfnCallback;
        }
        m_pfnXDrmPolicyCallbackContext = pvCallbackContext;
    }

errExit:
    return dr;
}

IXDRM_HRESULT CXDrm::SetRights( unsigned long ulRights )
{
return pkS_OK; // ALWAYS SUCCEED
}


static bool _CheckHeaderForOnDemand( size_t cbHdr, const uint8_t* pbHdr )
{
    // NOTE: This is a stub implementation just for detecting KeyRotation in use.
    //       It assumes working on a little endian machine and requires little endian UTF16.
    //       Search the DRM header for UTF16 "WRMHEADER" and if present search for "ONDEMAND"
    //       If found, set the global g_isOnDemand flag.

    bool isXML = false;

    static const size_t cbXmlOffset = 8; // offset to 16-bit size of the XML blob

    if (NULL != pbHdr && c_MaxDrmHeaderSize > cbHdr && (cbXmlOffset + 4) < cbHdr)
    {
        uint16_t* pUTF16 = (uint16_t*) &pbHdr[cbXmlOffset];

        size_t cchHeader = *pUTF16++ / 2;

        if (cchHeader != (cbHdr - (cbXmlOffset + 2)) / 2)
        {
            pUTF16 = (uint16_t*)pbHdr;
            cchHeader = cbHdr / 2;

            if (!('<' == pUTF16[0] && 'W' == pUTF16[1] && 'R' == pUTF16[2] && 'M' == pUTF16[3]))
            {
                CXDRM_LOG_MESSAGE( ("CXDrm::SetEnhancedData _CheckHeaderForOnDemand IGNORE content: XML length doesn't match data length\n"));
                goto okExit;
            }
        }

        if (0xFEFF == *pUTF16) // Big endian Byte Order Mark
        {
            CXDRM_LOG_MESSAGE( ("CXDrm::SetEnhancedData _CheckHeaderForOnDemand IGNORE content: big endian Byte Order Mark found\n"));
            goto okExit;
        }

        if (0xFFFE == *pUTF16) // Little endian Byte Order Mark
        {
            // Just skip it
            cchHeader--;
            pUTF16++;
        }

        // At this point we might have some XML so search for tags
        {
            char* szBuffer = NEW_NO_THROW char[cchHeader + 1]; // room for zero terminator

            if (NULL != szBuffer)
            {
                size_t ix;

                for ( ix = 0; ix < cchHeader && (0 != (*pUTF16 & 0xff)); ix++ )
                {
                    if (0 != (*pUTF16 & 0xff00)) // non-ASCII code page
                    {
                        szBuffer[ix] = '.'; // just substitute period character
                    }
                    else
                    {
                        szBuffer[ix] = (char) *pUTF16++;
                    }
                }
                szBuffer[ix] = '\0';

                isXML = NULL != strstr(szBuffer, "WRMHEADER");

                if (isXML)
                {
                    char* szLA_URL = strstr(szBuffer, "<LA_URL>");
                    if (szLA_URL)
                    {
                        szLA_URL += 8; // strlen("<LA_URL>");
                        char* szLA_URLend = strstr(szLA_URL, "</LA_URL>");
                        if (szLA_URLend && szLA_URLend > szLA_URL)
                        {
                            g_LicenseServerURI.assign(szLA_URL, szLA_URLend - szLA_URL);
                        }
                    }

                    g_isOnDemand = (NULL != strstr(szBuffer, "ONDEMAND"));

                    CXDRM_LOG_MESSAGE( ("CXDrm::SetEnhancedData _CheckHeaderForOnDemand %s cbHdr:%u\nXML: %s\n",
                        g_isOnDemand ? "*ONDEMAND*" : "NOT ONDEMAND", cbHdr, szBuffer ) );
                }
                delete szBuffer;
            }
        }
    }

okExit:
    return isXML;
}


IXDRM_HRESULT CXDrm::SetEnhancedData(
        size_t cbHdr,
        const uint8_t* pbHdr,
        size_t cbKeyID,
        const uint8_t* pbKeyID )
{
    IXDRM_HRESULT dr = pkS_OK;

    bool fJustInitialized = false;

    _RequireTransaction();

    AutoLock lock(&s_OperationLock);

    dr = _InitDRMIfRequired( &fJustInitialized );
    if (pkFAILED(dr))
    {
        goto errExit;
    }

    if( !fJustInitialized )
    {
        // Note: should call DRM porting kit Reinitialize to enable setting a new header.
    }

    if ( pbKeyID != NULL && cbKeyID > 0 )
    {
        // Note: should call DRM porting kit content set property based on key id
    }
    else
    {
        // Note: should call DRM porting kit content set property in auto-detect mode
    }

    // The following is just for purposes of stub operation
    if (!_CheckHeaderForOnDemand( cbHdr, pbHdr )
        && NULL != pbHdr
        && 4000 > cbHdr
        && 0 < cbHdr)
    {
        g_crc32LatestPSSH = ComputeCRC32(CRC32_INITIALIZE, pbHdr, cbHdr);
    }

errExit:
    return dr;
}

IXDRM_HRESULT CXDrm::AcquireDecryptContext(
        size_t cbKeyID,
        const uint8_t* pbKeyID,
        void **ppvDecryptContext )
{
    IXDRM_HRESULT dr = pkS_OK;

    DecryptorMap::iterator it;
    DRM_ID oKID;

    _DRM_DECRYPT_CONTEXT *poContext = NULL;

    AutoLock lock(&s_OperationLock);

    pkASSERT(m_fInitDRM);

    if ( pbKeyID != NULL )
    {
        if (sizeof(DRM_ID) != cbKeyID)
        {
            dr = pkE_UNEXPECTED;
            goto errExit;
        }
        memcpy( &oKID, pbKeyID, cbKeyID );
    }
    else
    {
        memset( &oKID, 0, sizeof(DRM_ID) );
    }

    it = g_oDecryptorMap.find( oKID );

    if ( it != g_oDecryptorMap.end() )
    {
        poContext = it->second;
        if (NULL == poContext)
        {
            dr = pkE_UNEXPECTED;
            goto errExit;
        }
        *ppvDecryptContext = poContext;
    }
    else // allocate a new decrypt context
    {
        poContext = (_DRM_DECRYPT_CONTEXT*) Oem_MemAlloc( sizeof(_DRM_DECRYPT_CONTEXT) );
        if (NULL == poContext)
        {
            dr = pkE_UNEXPECTED;
            goto errExit;
        }

        poContext->crc32PSSH = g_crc32LatestPSSH;

        memcpy( &(poContext->oKID), pbKeyID, cbKeyID );

        g_oDecryptorMap.insert( std::make_pair( oKID, poContext ) );

        *ppvDecryptContext = poContext;

        CXDRM_LOG_MESSAGE( ("CXDrm::AcquireDecryptContext [%02X%02X%02X%02X] Decryptor count now: %d\n",
            oKID.rgb[3],
            oKID.rgb[2],
            oKID.rgb[1],
            oKID.rgb[0],
            g_oDecryptorMap.size() ) );
    }

errExit:
    return dr;
}

IXDRM_HRESULT CXDrm::ReleaseDecryptContext(
        size_t cbKeyID,
        const uint8_t* pbKeyID )
{
    IXDRM_HRESULT dr = pkS_OK;

    DecryptorMap::iterator it;
    DRM_ID oKID;

    AutoLock lock(&s_OperationLock);

    pkASSERT(m_fInitDRM);

    if ( pbKeyID != NULL )
    {
        if (sizeof(DRM_ID) != cbKeyID)
        {
            dr = pkE_UNEXPECTED;
            goto errExit;
        }
        memcpy( &oKID, pbKeyID, cbKeyID );
    }
    else
    {
        memset( &oKID, 0, sizeof(DRM_ID) );
    }

    it = g_oDecryptorMap.find( oKID );

    if ( it != g_oDecryptorMap.end() )
    {
        _DRM_DECRYPT_CONTEXT *poContext = it->second;
        Oem_MemFree( poContext );
        g_oDecryptorMap.erase( it );

        CXDRM_LOG_MESSAGE( ("CXDrm::ReleaseDecryptContext [%02X%02X%02X%02X] Decryptor count remaining: %d\n",
            oKID.rgb[3],
            oKID.rgb[2],
            oKID.rgb[1],
            oKID.rgb[0],
            g_oDecryptorMap.size() ) );
    }
    else
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::ReleaseDecryptContext FAILED TO FIND KID [%02X%02X%02X%02X]\n",
            oKID.rgb[3], oKID.rgb[2], oKID.rgb[1], oKID.rgb[0] ) );
    }

errExit:
    return dr;
}

IXDRM_HRESULT CXDrm::CanDecrypt( void *pvDecryptContext, bool fAbortPlayback )
{
    IXDRM_HRESULT dr = pkS_OK;

    _RequireTransaction();

    AutoLock lock(&s_OperationLock);

    pkASSERT( m_fInitDRM );


    // Note: should call DRM porting kit reader bind function

    return dr;
}

IXDRM_HRESULT CXDrm::Decrypt(
        void *pvDecryptContext,
        uint8_t* pbData,
        size_t cbData,
        bool fIsAES,
        uint64_t qwSampleID,
        uint64_t qwOffset )
{
    IXDRM_HRESULT dr = pkS_OK;

    CXDRM_LOG_MESSAGE_VERBOSE( ("CXDrm::Decrypt [%p] cbData: %u iv:%llu offset:%llu\n",
        pvDecryptContext, cbData, qwSampleID, qwOffset ) );

    AutoLock lock(&s_OperationLock);

    if (!fIsAES)
    {
        dr = pkE_FAIL;
        goto errExit;
    }

    if (NULL == pvDecryptContext || NULL == pbData)
    {
        dr = pkE_INVALIDARG;
        goto errExit;
    }

    pkASSERT( m_fInitDRM );

    // Note: should call DRM porting kit reader decrypt function

    if (m_poIXDrmDiagDelegate)
    {
        _DRM_DECRYPT_CONTEXT* poContext = (_DRM_DECRYPT_CONTEXT*)pvDecryptContext;

        IXDrmDiagDelegate::SDecryptInfo sDecryptInfo;

        sDecryptInfo.cbKeyID = sizeof(poContext->oKID);
        sDecryptInfo.pbKeyID = poContext->oKID.rgb;
        sDecryptInfo.fIsAES = fIsAES;
        sDecryptInfo.fEndOfFrame = true;
        sDecryptInfo.qwSampleID = qwSampleID;
        sDecryptInfo.qwOffset = qwOffset;
        sDecryptInfo.crc32PSSH = poContext->crc32PSSH;

        m_poIXDrmDiagDelegate->OnDecrypt(sDecryptInfo);
    }

errExit:
    if (pkFAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::Decrypt [%p] FAILED [0x%X]\n",
            pvDecryptContext, dr ));
    }
    return dr;
}

IXDRM_HRESULT CXDrm::DecryptBufferChain(
        void *pvDecryptContext,
        void* pInBufList,
        void* pOutBufList,
        bool fIsAES,
        uint64_t qwSampleID,
        uint64_t qwOffset )
{
    IXDRM_HRESULT dr = pkS_OK;

    CXDRM_LOG_MESSAGE_VERBOSE( ("CXDrm::DecryptBufferChain [%p] in:%p out:%p iv:%llu offset:%llu\n",
        pvDecryptContext, pInBufList, pOutBufList, qwSampleID, qwOffset ) );

    AutoLock lock(&s_OperationLock);

    if (!fIsAES)
    {
        dr = pkE_FAIL;
        goto errExit;
    }

    if (NULL == pvDecryptContext || NULL == pInBufList)
    {
        dr = pkE_INVALIDARG;
        goto errExit;
    }

    pkASSERT( m_fInitDRM );

    // loop through the input buffer chain
    {
        bool isAnyEncrypted = false;
        bool endOfFrame = false;

        uint64_t qwOffsetAccum = qwOffset;

        PIPTV_HAL_BUFFER pBufListItem = (PIPTV_HAL_BUFFER) pInBufList;

        while (NULL != pBufListItem)
        {
            uint32_t cbSize = pBufListItem->u32DataEnd - pBufListItem->u32DataStart;

            bool isEncrypted = ( pBufListItem->u32Flags & IPTV_HAL_BUFFER_FLAG_DECRYPT ) != 0;

            if (isEncrypted)
            {
                // Note: should call DRM porting kit reader decrypt function
                //    qwSampleID for the initialization vector value
                //    (qwOffsetAccum / 16) for the block offset
                //    (qwOffsetAccum % 16) for the byte offset in the block
                //    (pBufListItem->pBuf + pBufListItem->u32DataStart) for the data pointer
                //    cbSize for the size of the data to encrypt
                isAnyEncrypted = true;
            }

            bool itemIsEndOfFrame = ( pBufListItem->u32Flags & IPTV_HAL_BUFFER_FLAG_ENDFRAME ) != 0;

            if( itemIsEndOfFrame )
            {
                endOfFrame = true;
            }

            CXDRM_LOG_MESSAGE_VERBOSE( ("- buf:%p start:%u size:%u offset:%llu %s%s\n",
                pBufListItem,
                pBufListItem->u32DataStart,
                cbSize,
                qwOffsetAccum,
                isEncrypted ? "Crypt" : "Clear",
                itemIsEndOfFrame ? "" : " <more>" ) );

            if ( isEncrypted )
            {
                qwOffsetAccum += cbSize;
            }

            pBufListItem = pBufListItem->pNext;
        }

        if (isAnyEncrypted)
        {
            if (m_poIXDrmDiagDelegate)
            {
                _DRM_DECRYPT_CONTEXT* poContext = (_DRM_DECRYPT_CONTEXT*)pvDecryptContext;

                IXDrmDiagDelegate::SDecryptInfo sDecryptInfo;

                sDecryptInfo.cbKeyID = sizeof(poContext->oKID);
                sDecryptInfo.pbKeyID = poContext->oKID.rgb;
                sDecryptInfo.fIsAES = fIsAES;
                sDecryptInfo.fEndOfFrame = endOfFrame;
                sDecryptInfo.qwSampleID = qwSampleID;
                sDecryptInfo.qwOffset = qwOffsetAccum;
                sDecryptInfo.crc32PSSH = poContext->crc32PSSH;

                m_poIXDrmDiagDelegate->OnDecryptBufferChain(sDecryptInfo);
            }
        }
    }

errExit:
    if (pkFAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::DecryptBufferChain [%p] FAILED [0x%X]\n",
            pvDecryptContext, dr ));
    }
    return dr;
}

IXDRM_HRESULT CXDrm::Commit()
{
    IXDRM_HRESULT dr = pkS_OK;

    _RequireTransaction();

    AutoLock lock(&s_OperationLock);

    pkASSERT( m_fInitDRM );

    // Note: should call DRM porting kit reader commit function

    CXDRM_LOG_MESSAGE( ("CXDrm::Commit\n" ) );

    return dr;
}

IXDRM_HRESULT CXDrm::GenerateChallenge(
        char** ppszUrl,
        const char* pszCustomData,
        char** ppszChallenge,
        bool fAllowCustomDataOverride )
{
    IXDRM_HRESULT dr = pkS_OK;

    _RequireTransaction();

    AutoLock lock(&s_OperationLock);

    pkASSERT( m_fInitDRM );

    // Note: should call DRM porting kit to generate the challenge and license server URL

    // For the stub, use LA_URL from the _checkHeaderForOnDemand and empty SOAP message
    {
        if (0 == g_LicenseServerURI.length())
        {
            g_LicenseServerURI = "http://example.net/test";
        }
        *ppszUrl = (char*)Oem_MemAlloc(g_LicenseServerURI.length()+1);
        strncpy(*ppszUrl, g_LicenseServerURI.c_str(), g_LicenseServerURI.length()+1);

        static const char pszNullChallenge[] =
            "<?xml version=\"1.0\" encoding=\"utf-8\" ?><soap:Envelope \
            xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \
            xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" \
            xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"> \
            <soap:Body><AcquireLicense></AcquireLicense></soap:Body></soap:Envelope>";

        *ppszChallenge = (char*)Oem_MemAlloc(sizeof(pszNullChallenge));
        strncpy(*ppszChallenge, pszNullChallenge, sizeof(pszNullChallenge));
    }
    CXDRM_LOG_MESSAGE( ("CXDrm::GenerateChallenge\n" ) );

    return dr;
}

IXDRM_HRESULT CXDrm::ProcessResponse( const char* pszResponse )
{
    IXDRM_HRESULT dr = pkS_OK;

    // NOTE: does NOT _RequireTransaction();

    AutoLock lock(&s_OperationLock);

    // Note: should call DRM porting kit to process response

    CXDRM_LOG_MESSAGE( ("CXDrm::ProcessResponse\n" ) );

    return dr;
}

IXDRM_HRESULT CXDrm::SendHttp(
        const char* szContentType,
        const char* szRequest,
        char** pszResponse,
        const char* szURL)
{
    IXDRM_HRESULT dr = pkS_OK;

    CXHttp* poHttp = NULL;

    std::string httpBody = szRequest;
    std::string httpServer;
    std::string resource = szURL;
    std::string httpRequest;

    CXDRM_LOG_MESSAGE( ("CXDrm::SendHttp URL: %s\n", szURL ) );

    // NOTE: does NOT _RequireTransaction();

    ChkBOOL( strcmp( szContentType, PLAYREADY_GETLICENSE_CONTENT_TYPE ) == 0, pkE_FAIL );

    // Parse out the httpServer and resource strings
    {
        size_t ixProto = resource.find("http://");

        ChkBOOL(0 == ixProto, pkE_INVALIDARG);
        resource = resource.substr(ixProto+7, resource.size() - (ixProto+7));

        size_t ixUri = resource.find_first_of('/');

        ChkBOOL(std::string::npos != ixUri, pkE_INVALIDARG);
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
            "User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)\r\n"
            "Pragma: no-cache\r\n"
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
            CXDRM_LOG_MESSAGE_VERBOSE( ("CXDrm::SendHttp challenge:\n%s\n===RESPONSE HEADER===\n%s\n",
                httpBody.c_str(), responseHeader.c_str()) );
        }
        else
        {
            CXDRM_LOG_MESSAGE( ("CXDrm::SendHttp challenge FAILED with response header\n%s\n",
                responseHeader.c_str()) );
        }

        ChkBOOL( isResponseOk, pkE_FAIL );

        uint32_t cbContent = poHttp->GetContentLength();

        ChkBOOL( cbContent < MAXIMUM_LICACQ_RESPONSE_BODY_SIZE, pkE_INSUFFICIENT_BUFFER);

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
                dr = pkE_FAIL;
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

void CXDrm::Reset()
{
    /* do nothing */
}

void CXDrm::MemFree( void* ptr )
{
    if (ptr)
    {
        Oem_MemFree(ptr);
    }
}

XDRM_VERSION_CODE CXDrm::GetDRMVersionCode()
{
    return XDRM_VERSIONCODE_PLAYREADY;
}

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

            // Note: should call DRM porting kit to get property

            if (sizeof(uint32_t) == *pcbProperty)
            {
                *((uint32_t*)pbProperty) = g_isOnDemand ? DRM_DECRYPTORSETUP_ONDEMAND : 0;

                CXDRM_LOG_MESSAGE( ("CXDrm::GetContentProperty XDRM_PROP_DECRYPTORSETUP: isOnDemand: %d\n", g_isOnDemand ) );

                dr = pkS_OK;
            }
            break;

        default:
            dr = pkE_FAIL;
    }

    return( dr );
}


IXDRM_HRESULT CXDrm::GetPropertyFromResponse(
            const uint8_t *pbResponse,
            size_t cbResponse,
            XDRM_RESPONSE_PROPERTY eProperty,
            char **ppszPropertyData )
{
    IXDRM_HRESULT dr = pkS_OK;

    // NOTE: does NOT _RequireTransaction();

    AutoLock lock(&s_OperationLock);

    pkASSERT( m_fInitDRM );

    if( NULL == pbResponse || 0 == cbResponse || NULL == ppszPropertyData )
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::GetCustomDataFromResponse: invalid params: %p, %u, %p\n",
            pbResponse, cbResponse, ppszPropertyData ) );

        dr = pkE_INVALIDARG;
        goto errExit;
    }

    *ppszPropertyData = NULL;

    switch ( eProperty )
    {
        case XDRM_RESPONSE_CUSTOM_DATA:
            // map type to DRM porting kit type
            break;

        case XDRM_RESPONSE_REDIRECT_URL:
            // map type to DRM porting kit type
            break;

        default:
            dr = pkE_INVALIDARG;
            goto errExit;
    }

    // Note: should call DRM porting kit to get additional response data based on type

    CXDRM_LOG_MESSAGE( ("CXDrm::GetCustomDataFromResponse\n" ) );

errExit:
    return dr;
}


void CXDrm::BeginTransaction()
{
    //
    // Wait for any existing transaction to complete before starting another one
    //
    s_OperationLock.Lock();

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

bool CXDrm::SetDiagDelegate( IXDrmDiagDelegate* poIXDrmDiagDelegate )
{
    s_OperationLock.Lock();
    m_poIXDrmDiagDelegate = poIXDrmDiagDelegate;
    s_OperationLock.Unlock();

    return true;
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

