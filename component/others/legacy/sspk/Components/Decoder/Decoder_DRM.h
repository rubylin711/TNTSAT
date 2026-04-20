///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Decoder.h"
#include "IDrmManager.h"

// ===============================================================================================================
// ===============================================================================================================

class DecoderDRM : public Decoder
{
public:
    DecoderDRM(IReceiverControl* receiverControl, const CStreamInfo& si);
    virtual ~DecoderDRM();

    __override bool            Acquire(void);
    __override void            Release(void);
    __override void            OnSync(bool sync, bool teardownPicture, bool cleanStall);
    __override void            SetDRMHandle(uint32 drmHandle);

protected:
    void                       SetCryptoKeyId(__in_bcount(len) const byte* signedKeyId, uint32 len);
    void                       SetCryptoSampleId(uint64 sampleId);
    Buffer*                    WriteHALDecoder(Buffer* chain, uint64 pts);
    void                       HeartBeatEvent(uint64 curPts);

    virtual void               SetPassThroughAfterDecryption(void) {}

private:
    void                       DrmErrorEvent(uint32 cryptoError);
    static iptv_hal_error      DecryptCB(PIPTV_HAL_BUFFER pInBufList, PIPTV_HAL_BUFFER pOutBufList, PVOID pDecryptContext);

protected:
    //Whether this is audio or video decoder
    bool                       mIsVideo;
    //Whether it is audio description stream
    bool                       mIsAudioDescription;
    //Codec type
    int                        mCodec;
    //HAL Decoder context
    LPVOID                     mDecoderContext;

    //Whether the HAl decoder is already in flushed state
    bool                       mDecoderFlushed;

    //DRM context (Mediaroom or PlayReady)
    IDrmDecrypter*             mDRMContext;

    //Tracks DRM errors
    uint32                     mLastDrmError;
    //Decoder diagnostics
    CDecoderDiagnostics&       mDecoderDiagnostics;
};

// ===============================================================================================================
// ===============================================================================================================
