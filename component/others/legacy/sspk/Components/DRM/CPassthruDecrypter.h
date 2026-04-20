///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "IDrmManager.h"
#include "IPTVCryptoHal.h"

class CPassthruDecrypter : public IDrmDecrypter
{
public:

    virtual void                Reset() {}

    virtual void                SetKeyID(const byte* p, uint32 len) {}
    virtual void                SetSampleID(uint64 sampleID) {}
    virtual void                SetOffset(uint64 offset) {}

    virtual const byte*         GetKeyID(uint32_t &len) { len = 0; return NULL; }
    virtual uint64_t            GetSampleID() { return 0; }
    virtual uint64_t            GetOffset() { return 0; }

    virtual iptv_hal_error      Decrypt(byte* pbData, size_t cbData) { return IPTV_HAL_ERROR_SUCCESS; }
    virtual iptv_hal_error      Decrypt(byte* pbData, size_t cbData, uint64 sampleID, uint64 sampleOffset) { return IPTV_HAL_ERROR_SUCCESS; }
    virtual iptv_hal_error      DecryptBufferChain(PIPTV_HAL_BUFFER pIn, PIPTV_HAL_BUFFER pOut)
    {
        return IPTV_HAL_Crypto_DecryptAVPayload(pIn, pOut, IPTV_HAL_CRYPTO_KEY_REGISTER_PASSTHRU, 0, 0, NULL, 0);
    }
    virtual iptv_hal_error      GetProperty(const std::string& name, byte* pbPropertyData, size_t* pcbPropertyData) { return IPTV_HAL_ERROR_NOT_SUPPORTED; }
    virtual iptv_hal_error      GetSecurityFlags(SecurityFlags* pFlagsOut) { return IPTV_HAL_ERROR_NOT_SUPPORTED; }
    virtual uint32              GetLicenseHandle() { return 0; }
    virtual DrmEncryptionType   GetEncryptionType() { return DrmEncryptionType_Unknown; }

    virtual void Dispose()      { delete this; }

protected:

    //Destructor
    virtual ~CPassthruDecrypter() {}
};
