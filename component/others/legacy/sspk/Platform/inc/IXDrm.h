///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// File:   IXDrm.h
//
// Microsoft Digital Rights Management
//
// Description:
//
// Interface class for the DRM platform abstraction
//
//////////////////////////////////////////////////////////////////////////

#pragma once

// DRM Version Enum
typedef enum {
    XDRM_VERSIONCODE_UNKNOWN,
    XDRM_VERSIONCODE_PLAYREADY,
} XDRM_VERSION_CODE;

typedef enum
{
    XDRM_PROP_DECRYPTORSETUP
} XDRM_CONTENT_PROPERTY;


typedef enum XDRM_RIGHTS
{
    XDRM_RIGHT_PLAYBACK  = 0x1,
} XDRM_RIGHTS;

typedef enum
{
    XDRM_RESPONSE_CUSTOM_DATA,
    XDRM_RESPONSE_REDIRECT_URL
} XDRM_RESPONSE_PROPERTY;

typedef enum
{
    XDRM_POLICY_CALLBACK_UNKNOWN = 0x0,
    XDRM_POLICY_CALLBACK_PLAY_OPL = 0x1,
    XDRM_POLICY_CALLBACK_COPY_OPL = 0x2,
    XDRM_POLICY_CALLBACK_INCLUSION_LIST = 0x3,
    XDRM_POLICY_CALLBACK_EXTENDED_RESTRICTION_CONDITION = 0x4,
    XDRM_POLICY_CALLBACK_EXTENDED_RESTRICTION_ACTION = 0x5,
    XDRM_POLICY_CALLBACK_EXTENDED_RESTRICTION_QUERY = 0x6,
    XDRM_POLICY_CALLBACK_SECURE_STATE_TOKEN_RESOLVE = 0x7,
    XDRM_POLICY_CALLBACK_RESTRICTED_SOURCEID = 0x8,
    XDRM_POLICY_CALLBACK_OEM_KEY_INFO = 0x9,
    XDRM_POLICY_CALLBACK_MAX,
} XDRM_POLICY_CALLBACK;

typedef enum
{
    XDRM_OPL_UNKNOWN = 0x0,
    XDRM_OPL_DISABLE = 0x1,
    XDRM_OPL_ENABLE_DOWN_RES = 0x2,
    XDRM_OPL_ENABLE_ALWAYS = 0x3
} XDRM_OPL_ACTION;

typedef struct
{
    uint32_t CGMSALevel;
    uint32_t MacrovisionLevel;
    XDRM_OPL_ACTION HDCPAction;
    bool DisableComponentVideo;
} XDRM_OPL_DATA;

// Note: IXDRM_HRESULT must be type compatible with Windows HRESULT
typedef int32_t IXDRM_HRESULT;

typedef IXDRM_HRESULT (*XDrmOPLCallbackPtr)( XDRM_OPL_DATA *f_pOPLData,
                                             const void *f_pvContext );

#define XDRM_IS_OPL_DATA_EMPTY( pOPLData )  ( pOPLData->CGMSALevel == 0 && pOPLData->MacrovisionLevel == 0 && pOPLData->HDCPAction == XDRM_OPL_UNKNOWN && !(pOPLData->DisableComponentVideo) )

#define PLAYREADY_GETLICENSE_CONTENT_TYPE  "Content-Type: text/xml; charset=utf-8\r\nSOAPAction: \"http://schemas.microsoft.com/DRM/2007/03/protocols/AcquireLicense\"\r\n"

#ifdef __cplusplus

class IXDrmDiagDelegate
{
public:
    virtual ~IXDrmDiagDelegate() {};

    struct SDecryptInfo
    {
        size_t          cbKeyID;
        const uint8_t*  pbKeyID;
        bool            fIsAES;
        bool            fEndOfFrame;
        uint64_t        qwSampleID;
        uint64_t        qwOffset;
        uint32_t        crc32PSSH;
    };

    virtual void OnDecrypt( const SDecryptInfo& sDecryptInfo ) {}

    virtual void OnDecryptBufferChain( const SDecryptInfo& sDecryptInfo ) {}
};

class IXDrm
{
public:
    virtual ~IXDrm() {};

    virtual IXDRM_HRESULT Init( XDrmOPLCallbackPtr pfnCallback, void* pvCallbackContext ) = 0;
    
    virtual IXDRM_HRESULT SetCertPath( const char*  ppszCertPath ) = 0;

    virtual IXDRM_HRESULT SetRights( unsigned long ulRights ) = 0;

    virtual IXDRM_HRESULT SetEnhancedData(
            size_t cbHdr,
            const uint8_t* pbHdr,
            size_t cbKeyID,
            const uint8_t* pbKeyID ) = 0;

    virtual IXDRM_HRESULT AcquireDecryptContext(
            size_t cbKeyID,
            const uint8_t* pbKeyID,
            void **ppvDecryptContext ) = 0;

    virtual IXDRM_HRESULT ReleaseDecryptContext(
            size_t cbKeyID,
            const uint8_t* pbKeyID ) = 0;

    virtual IXDRM_HRESULT CanDecrypt(
            void *pvDecryptContext,
            bool fAbortPlayback ) = 0;

    virtual IXDRM_HRESULT Decrypt(
            void *pvDecryptContext,
            uint8_t* pbData,
            size_t cbData,
            bool fIsAES,
            uint64_t qwSampleID,
            uint64_t qwOffset ) = 0;

    virtual IXDRM_HRESULT DecryptBufferChain(
            void *pvDecryptContext,
            void* pInBufList,
            void* pOutBufList,
            bool fIsAES,
            uint64_t qwSampleID,
            uint64_t qwOffset ) = 0;

    virtual IXDRM_HRESULT Commit() = 0;

    virtual IXDRM_HRESULT GenerateChallenge(
            char** ppszUrl,
            const char* pszCustomData,
            char** ppszChallenge,
            bool fAllowCustomDataOverride = true ) = 0;

    virtual IXDRM_HRESULT ProcessResponse(
            const char* pszResponse ) = 0;

    virtual IXDRM_HRESULT SendHttp(
            const char* szContentType,
            const char* szRequest,
            char** pszResponse,
            const char* szURL ) = 0;

    virtual void Reset() = 0;
    virtual void MemFree( void* ptr ) = 0;

    virtual XDRM_VERSION_CODE GetDRMVersionCode() = 0;

    virtual IXDRM_HRESULT GetContentProperty(
            XDRM_CONTENT_PROPERTY eProperty,
            uint8_t* pbProperty,
            uint32_t* pcbProperty ) = 0;

    virtual IXDRM_HRESULT GetPropertyFromResponse(
            const uint8_t *pbResponse,
            size_t cbResponse,
            XDRM_RESPONSE_PROPERTY eProperty,
            char **ppszPropertyData ) = 0;

    virtual void BeginTransaction() = 0;
    virtual void EndTransaction() = 0;

    virtual bool SetDiagDelegate( IXDrmDiagDelegate* poIXDrmDiagDelegate ) { return false; }
    // SetDelegate overrides any current setting and should be called with NULL before deleting the delegate.

};

extern "C" __declspec(dllexport) IXDRM_HRESULT WINAPI XDRM_CreateInstance( IXDrm **ppXDrm );
extern "C" __declspec(dllexport) IXDRM_HRESULT WINAPI XDRM_DestroyInstance( IXDrm *pXDrm );

typedef IXDRM_HRESULT (WINAPI* pfn_XDRM_CreateInstance)( IXDrm **ppXDrm );
typedef IXDRM_HRESULT (WINAPI* pfn_XDRM_DestroyInstance)( IXDrm *pXDrm );

#endif  // __cplusplus


