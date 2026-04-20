///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IDrmManager.h"
#include "IXDrm.h"
#include "IDrmLicense.h"


/// <summary>
/// Implements IDrmDecrypter for PlayReady
/// </summary>

class CPlayReadyDecrypter : public IDrmDecrypter
{
public:

    CPlayReadyDecrypter( _In_ IDrmLicense* pLicense );

    __override void                Reset(void);
    __override void                SetKeyID(const byte* p, uint32 len);
    __override void                SetSampleID(uint64 sampleID) { _sampleID = sampleID; }
    __override void                SetOffset(uint64 offset) { _sampleOffset = offset; }

    __override const byte*         GetKeyID(uint32_t &len) { len = _keyID.Size(); return _keyID.Data(); }
    __override uint64_t            GetSampleID() { return _sampleID; }
    __override uint64_t            GetOffset() { return _sampleOffset; }

    __override iptv_hal_error      Decrypt(byte* pbData, size_t cbData);
    __override iptv_hal_error      Decrypt(byte* pbData, size_t cbData, uint64 sampleID, uint64 sampleOffset);
    __override iptv_hal_error      DecryptBufferChain(PIPTV_HAL_BUFFER pIn, PIPTV_HAL_BUFFER pOut);

    //Get property APIs
    __override iptv_hal_error      GetProperty(const std::string& name, byte* pbPropertyData, size_t* pcbPropertyData);
    __override iptv_hal_error      GetSecurityFlags(SecurityFlags* pFlagsOut);
    __override uint32              GetLicenseHandle(void);
    __override DrmEncryptionType   GetEncryptionType() { return DrmEncryptionType_PlayReady; }

    __override void Dispose()      { delete this; }

private:

    virtual ~CPlayReadyDecrypter();

    IDrmLicense*  _license;
    IXDrm*        _pIXDrm;

    uint64        _sampleID;
    uint64        _sampleOffset;

    CByteBuffer   _keyID;
};
