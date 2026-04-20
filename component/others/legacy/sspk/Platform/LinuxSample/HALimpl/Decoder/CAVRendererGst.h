///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include <pkPAL.h>

#include "CAVRendererDefs.h"

namespace CAVRendererGst
{

pkRESULT Startup();
pkRESULT Shutdown();

pkRESULT Open(pkHANDLE *ctx);
pkRESULT Close(pkHANDLE ctx);

pkRESULT OpenDecoder
(
    pkHANDLE hRenderer,
    pkAV_STREAM_DESCRIPTOR *pStreamDescriptor,
    pkBUFFER_POOL_DESCRIPTOR *pBufferPoolDesc,
    pkHANDLE *phDecoder
);

pkRESULT CloseDecoder(pkHANDLE ctx, pkHANDLE hDecoder);
pkRESULT QueryDecoderBufferPool
(
    pkHANDLE ctx,
    pkHANDLE hDecoder,
    uint32_t *pNumFree,
    uint32_t *pLastBufferTransferred
);
pkRESULT GetDecoderBuffer
(
    pkHANDLE ctx,
    pkHANDLE hDecoder,
    uint8_t **ppBuffer,
    uint32_t *pBufSize,
    uint32_t timeoutMillisecs
);
pkRESULT SubmitDecoderBuffer
(
    pkHANDLE ctx,
    pkHANDLE hDecoder,
    uint8_t *pBuffer,
    uint32_t bufLength,
    uint32_t sampleFlags,
    pkSAMPLE_FRAGMENT_TYPE fragmentType,
    int64_t timeStamp
);
pkRESULT Start(pkHANDLE ctx, uint32_t startFlags, uint64_t iStartTime );
pkRESULT Stop(pkHANDLE ctx, uint32_t stopFlags);
pkRESULT Pause(pkHANDLE ctx);
pkRESULT NotifyPrerollStart(pkHANDLE ctx);
pkRESULT NotifyPrerollComplete(pkHANDLE ctx);
pkRESULT Resume(pkHANDLE ctx);

pkRESULT SetPCR(pkHANDLE ctx, uint64_t uiPCR);
pkRESULT SetClockRate(pkHANDLE ctx, int32_t uiPPM);
pkRESULT GetCurrentPTS(pkHANDLE hRenderer, uint64_t *pCurrentPTShns);

pkRESULT SetVolumeLevel(pkHANDLE ctx, uint32_t uiVolLevel);
pkRESULT GetVolumeLevel(pkHANDLE ctx, uint32_t *puiVolLevel);
pkRESULT SetMuteState(pkHANDLE ctx, bool_t isMuted);
pkRESULT GetMuteState(pkHANDLE ctx, bool_t *pIsMuted);

pkRESULT SetPlayRate(pkHANDLE ctx, int32_t iPlayRate);

pkRESULT SetEndOfStream(pkHANDLE hRenderer, pkHANDLE hDecoder, bool isEoS);
pkRESULT GetRenderingDone(pkHANDLE hRenderer, pkHANDLE hDecoder, bool *isDone);
pkRESULT GetDecoderStalled(pkHANDLE hRenderer, pkHANDLE hDecoder, uint64_t systemTime90KHz, bool *isStalled);

pkRESULT FlushDecoder(pkHANDLE ctx, pkHANDLE hDecoder);

}// end namespace CAVRendererGst
