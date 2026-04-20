///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CPlayReadyDecrypter.h"
#include "CPlayReadyLicense.h"
#include "IPTVCryptoHal.h"
#include "IXDrm.h"
#include "Trace.h"

//#define PRDECRYPTER_SPEW
#if defined(PRDECRYPTER_SPEW)
#define PRDECRYPTER_MSG(x) TRACE(x)
#else
#define PRDECRYPTER_MSG(x)
#endif

CPlayReadyDecrypter::CPlayReadyDecrypter( _In_ IDrmLicense* pLicense )
    : _sampleID(0)
    , _sampleOffset(0)
{
    CHECK_ALLOC(pLicense);
    _license = pLicense;

    _pIXDrm = _license->GetIXDrm();
    CHECK_ALLOC(_pIXDrm);

    uint32 len = 0;
    const byte* pKeyID = _license->GetDefaultKeyID(&len);
    if (pKeyID && len)
    {
        SetKeyID(pKeyID, len);
    }
}

CPlayReadyDecrypter::~CPlayReadyDecrypter()
{
    _license = NULL;
    _keyID.Clear();
}

void CPlayReadyDecrypter::Reset(void)
{
    _sampleOffset = 0;
}

void CPlayReadyDecrypter::SetKeyID(const byte* p, uint32 len)
{
    _keyID.Set( p, len );
}

iptv_hal_error CPlayReadyDecrypter::Decrypt(byte* pbData, size_t cbData)
{
    pkRESULT pkResult = pkS_OK;
    void *pvDecryptContext = NULL;

    if (!_pIXDrm)
    {
        return IPTV_HAL_ERROR_CRYPTO_NOT_INITIALIZED;
    }

    pkResult = _pIXDrm->AcquireDecryptContext( _keyID.Size(), _keyID.Data(), &pvDecryptContext);
    if (pkFAILED(pkResult))
    {
        return IPTV_HAL_ERROR_FAILED;
    }

    pkResult = _pIXDrm->Decrypt(pvDecryptContext, pbData, cbData, true, _sampleID, _sampleOffset);
    if (pkFAILED(pkResult))
    {
        return IPTV_HAL_ERROR_FAILED;
    }

    _sampleOffset += cbData;
    return IPTV_HAL_ERROR_SUCCESS;
}

iptv_hal_error CPlayReadyDecrypter::Decrypt(byte* pbData, size_t cbData, uint64 sampleID, uint64 sampleOffset)
{
    pkRESULT pkResult = pkS_OK;
    void *pvDecryptContext = NULL;

    if (!_pIXDrm)
    {
        return IPTV_HAL_ERROR_CRYPTO_NOT_INITIALIZED;
    }

    pkResult = _pIXDrm->AcquireDecryptContext(_keyID.Size(), _keyID.Data(), &pvDecryptContext);
    if (pkFAILED(pkResult))
    {
        return IPTV_HAL_ERROR_FAILED;
    }

    pkResult = _pIXDrm->Decrypt(pvDecryptContext, pbData, cbData, true, sampleID, sampleOffset);
    if (pkFAILED(pkResult))
    {
        return IPTV_HAL_ERROR_FAILED;
    }

    return IPTV_HAL_ERROR_SUCCESS;
}

#define IS_ENCRYPTED(b) ((b->u32Flags & IPTV_HAL_BUFFER_FLAG_DECRYPT) != 0)
#define IS_ENDFRAME(b)  ((b->u32Flags & IPTV_HAL_BUFFER_FLAG_ENDFRAME) != 0)

iptv_hal_error CPlayReadyDecrypter::DecryptBufferChain(PIPTV_HAL_BUFFER pIn, PIPTV_HAL_BUFFER pOut)
{
    pkRESULT pkResult = pkS_OK;
    PIPTV_HAL_BUFFER pBuf;
    void *pvDecryptContext = NULL;

    if (!_pIXDrm)
    {
        TRACE_ERROR(("CPlayReadyDecrypter::DecryptBufferChain(), NOT_INITIALIZED!\n"));
        return IPTV_HAL_ERROR_CRYPTO_NOT_INITIALIZED;
    }

    pkResult = _pIXDrm->AcquireDecryptContext(_keyID.Size(), _keyID.Data(), &pvDecryptContext);
    if (pkFAILED(pkResult))
    {
        TRACE_ERROR(("CPlayReadyDecrypter::DecryptBufferChain(), AcquireDecryptContext failed, pvDecryptContext=0x%x", pvDecryptContext));
        return IPTV_HAL_ERROR_FAILED;
    }

    pkResult =  _pIXDrm->DecryptBufferChain(pvDecryptContext, pIn, pIn, true, _sampleID, _sampleOffset);

    if (pkFAILED(pkResult))
    {
        ASSERT(false);
        // TODO: Convert pkRESULT to IPTV_HAL_ERROR
        TRACE_ERROR(("CPlayReadyDecrypter::DecryptBufferChain(), DecryptBufferChain failed, pvDecryptContext=0x%x", pvDecryptContext));
        return IPTV_HAL_ERROR_FAILED;
    }
    else
    {
        for (pBuf = pIn; ; pBuf = pBuf->pNext)
        {
            if (IS_ENCRYPTED(pBuf))
            {
                UINT32 offset = pBuf->u32DataStart;    // Buffer::PayloadStart
                int size = pBuf->u32DataEnd - offset;
                _sampleOffset += size;
            }

            if (pBuf->pNext == NULL)
                break;
        }
    }
    if (IS_ENDFRAME(pBuf))
    {
        _sampleOffset = 0;
    }

    // Afterwards, go through normal pass-through codepath
    //return IPTV_HAL_Crypto_DecryptAVPayload(pIn, pOut, IPTV_HAL_CRYPTO_KEY_REGISTER_PASSTHRU, 0, 0, NULL, 0);

    //As there are same for input and output buffer, no need to operate the output buffer again. So just return.
    return IPTV_HAL_ERROR_SUCCESS;
}

iptv_hal_error CPlayReadyDecrypter::GetProperty(const std::string& name, byte* pbPropertyData, size_t* pcbPropertyData)
{
    if (name == DRM_PROPERTY_LICENSETIMES)
    {
        if (!pbPropertyData || (*pcbPropertyData < sizeof(DrmLicenseTiming)))
        {
            *pcbPropertyData = sizeof(DrmLicenseTiming);
            return IPTV_HAL_ERROR_INVALID_PARAMETER;
        }

        _license->GetLicenseTimes((DrmLicenseTiming*) pbPropertyData);
        return IPTV_HAL_ERROR_SUCCESS;
    }

    return IPTV_HAL_ERROR_NOT_SUPPORTED;
}

iptv_hal_error CPlayReadyDecrypter::GetSecurityFlags(SecurityFlags* pFlagsOut)
{
    // Currently, PlayReady security flags stored in CRendererState
    return IPTV_HAL_ERROR_NOT_SUPPORTED;
}

uint32 CPlayReadyDecrypter::GetLicenseHandle(void)
{
    return _license->GetHandle();
}
