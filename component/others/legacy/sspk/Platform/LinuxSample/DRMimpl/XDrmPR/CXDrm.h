///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include "IXDrm.h"

class CXDrm : public IXDrm
{
private:
    CXDrm();
    ~CXDrm();

public:
    static IXDRM_HRESULT _CreateInstance( IXDrm **ppXDrm );
    static IXDRM_HRESULT _DestroyInstance( IXDrm *pXDrm );
        
// IXDrm implementation:

    IXDRM_HRESULT Init( XDrmOPLCallbackPtr pfnCallback, void* pvCallbackContext );

    IXDRM_HRESULT SetCertPath( const char*  ppszCertPath );

    IXDRM_HRESULT SetRights( unsigned long ulRights );

    IXDRM_HRESULT SetEnhancedData(
            size_t cbHdr,
            const uint8_t* pbHdr,
            size_t cbKeyID,
            const uint8_t* pbKeyID );

    IXDRM_HRESULT AcquireDecryptContext(
            size_t cbKeyID,
            const uint8_t* pbKeyID,
            void **ppvDecryptContext );

    IXDRM_HRESULT ReleaseDecryptContext(
            size_t cbKeyID,
            const uint8_t* pbKeyID );

    IXDRM_HRESULT CanDecrypt(
            void *pvDecryptContext,
            bool fAbortPlayback );

    IXDRM_HRESULT Decrypt(
            void *pvDecryptContext,
            uint8_t* pbData,
            size_t cbData,
            bool fIsAES,
            uint64_t qwSampleID,
            uint64_t qwOffset );

    IXDRM_HRESULT DecryptBufferChain(
            void *pvDecryptContext,
            void* pInBufList,
            void* pOutBufList,
            bool fIsAES,
            uint64_t qwSampleID,
            uint64_t qwOffset );

    IXDRM_HRESULT Commit();

    IXDRM_HRESULT GenerateChallenge(
            char** ppszUrl,
            const char* pszCustomData,
            char** ppszChallenge,
            bool fAllowCustomDataOverride );

    IXDRM_HRESULT ProcessResponse( const char* pszResponse );
    
    IXDRM_HRESULT SendHttp(
            const char* szContentType,
            const char* szRequest,
            char** pszResponse,
            const char* szURL );

    void Reset();
    void MemFree( void* ptr );

    XDRM_VERSION_CODE GetDRMVersionCode();

    IXDRM_HRESULT GetContentProperty(
            XDRM_CONTENT_PROPERTY eProperty,
            uint8_t* pbProperty,
            uint32_t* pcbProperty );

    IXDRM_HRESULT GetPropertyFromResponse(
            const uint8_t *pbResponse,
            size_t cbResponse,
            XDRM_RESPONSE_PROPERTY eProperty,
            char **ppszPropertyData );

    void BeginTransaction();
    void EndTransaction();

    // The following are not defined in IXDrm.
    
    IXDRM_HRESULT OPLCallback(
            XDRM_OPL_DATA *f_pOPLData );

private: // methods

    IXDRM_HRESULT _Init();
    IXDRM_HRESULT _InitDRMIfRequired( bool *f_pfJustInitialized );
    void _RequireTransaction();

private: // state
    void *m_poAppContext;
    uint8_t *m_pbOpaqueBuffer;
    uint32_t m_cbOpaqueBuffer;
    uint8_t *m_pbRevocationBuffer;

    XDrmOPLCallbackPtr m_pfnXDrmOPLCallback;
    void *m_pvXDrmOPLCallbackContext;

    bool m_fInTransaction;
    bool m_fInit;
    bool m_fInitDRM;

    //
    // Singleton reference tracking state:
    //
    static Lockable s_FactoryLock;

    static int s_nRefCount;
    static IXDrm *s_poXDrm;

    // Method operation lock
    static Lockable s_OperationLock;
    
};

