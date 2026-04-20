///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DrmDefinitions.h"
#include "IPTVPhysMemMgr.h"
#include "IXDrm.h"
#include <string>

class IAVManager;

class IDrmCallbackSink
{
public:
    virtual ~IDrmCallbackSink() {}
    virtual void OPLCallback(XDRM_OPL_DATA *pOPLData) = 0;
};

class IDrmDecrypter
{
public:
    typedef struct
    {
        uint8_t     CGMS_AFlags;
        uint8_t     MacrovisionFlags;
        uint8_t     FingerprintFlags;
    } SecurityFlags;


    virtual void                Reset() = 0;

    virtual void                SetKeyID(const byte* p, uint32_t len) = 0;
    virtual void                SetSampleID(uint64_t sampleID) = 0;
    virtual void                SetOffset(uint64_t offset) = 0;

    virtual const byte*         GetKeyID(uint32_t &len) = 0;
    virtual uint64_t            GetSampleID() = 0;
    virtual uint64_t            GetOffset() = 0;

    //ManifestDecrypter / ThumbnailDecrypter
    virtual iptv_hal_error      Decrypt(byte* pbData, size_t cbData) = 0;

    virtual iptv_hal_error      Decrypt(byte* pbData, size_t cbData, uint64_t sampleID, uint64_t sampleOffset) = 0;

    //Usually will use TV2SecureCore
    virtual iptv_hal_error      DecryptBufferChain(PIPTV_HAL_BUFFER pIn, PIPTV_HAL_BUFFER pOut) = 0;

    //Generic property API
    virtual iptv_hal_error      GetProperty(const std::string& name, byte* pbPropertyData, size_t* pcbPropertyData) = 0;

    //Need for optimization for Mediaroom decryption
    virtual iptv_hal_error      GetSecurityFlags(SecurityFlags* pFlagsOut) = 0;

    //Handle to share license info for multiple contexts
    virtual uint32_t            GetLicenseHandle() = 0;

    virtual DrmEncryptionType   GetEncryptionType() = 0;

    //Dispose of the decrypter
    virtual void                Dispose() = 0;

protected:

    //Protected destructor.
    //The application is expected to call IDrmDecrypter::Dispose when
    //it's done with the object.
    virtual                     ~IDrmDecrypter() {}
};

class IDrmManager
{
public:
    // Class Factory methods
    static IDrmManager*         Create(IAVManager* pAVManager);
    static void                 Dispose(IDrmManager* pDrmManager);

public:
    virtual ~IDrmManager() {}
    virtual bool                IsDecrypterOnDemand(DrmDecryptionMode mode, size_t cbHdr, const byte* pbHdr) = 0;
    // Create new DrmContext
    virtual IDrmDecrypter*      GetDecrypter(DrmDecryptionMode mode) = 0;
    virtual IDrmDecrypter*      GetDecrypter(DrmDecryptionMode mode, size_t cbHdr, const byte* pbHdr, size_t cbKeyId, const byte* pbKeyId, IDrmCallbackSink* pCallback = NULL) = 0;
    virtual IDrmDecrypter*      GetDecrypter(DrmDecryptionMode mode, GUID& guid) = 0;

    // Fetch new DrmContext using handle from pre-existing object (in case license is shared)
    // Each DrmContext will track sampleID / offset independently
    virtual IDrmDecrypter*      GetDecrypter(DrmDecryptionMode mode, uint32_t hLicense) = 0;
    virtual void                ReleaseDecrypter(IDrmDecrypter* pContext) = 0;

    virtual pkRESULT OverrideCustomData( const char * pResponseCustomData ) = 0;

    virtual pkRESULT OverrideLicenseServerUri( const char * pszLicenseServerUri) = 0;

    virtual pkRESULT OverrideCertPath( const char * pszCertPath) = 0;

};
