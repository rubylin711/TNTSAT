///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

/// <summary>
///     IPTVCryptoHal.cpp
///        This file contains sample/stub implementation of Crypto Core APIs
/// </summary>

#include "pkPAL.h"

#include "IPTVCryptoHal.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Decrypts an AV payload using the key in the indicated boundary key register, and places the result in a secure memory area where it can be
/// accessed by the AV decoder. The AV payload is to be decrypted using AES in counter mode. The initial counter value is established based on
/// the sample id and byte offset, as specified below.
///
/// A "passthru mode" (where no boundary key is indicated) is provided - in this case no decryption takes place and the input data is copied
/// directly into the secure memory.
///
/// It is also possible for the caller to set flags so as to selectively passthru individual buffers. (This can be used in cases where the
/// caller needs to intersperse cleartext information, such as protocol headers, with the encrypted AV payload).
///
/// If (and only if) permitted by a flag in the signed key identifier supplied, the decrypted data may be placed in CPU-accessible memory.
///
/// A sequence of checks MUST be performed before decryption is permitted. This sequence is specified in the Crypto HAL API document.
///
/// </summary>
/// <param name="inBufList">[IN] The buffer(s) containing the AV payload to be decrypted. The data type for this parameter is IPTV_HAL_BUFFER as defined in section 5.2 The decrypt flag of a particular buffer (IPTV_HAL_BUFFER_FLAG_DECRYPT) is set to indicate that the data in the corresponding buffer is to be decyrpted. Otherwise, if the flag is clear the data should be passed through without decryption, even if the sourceBoundaryKeyRegisterId value does not itself indicate the passthru value.</param>
/// <param name="outBufList">[IN] The buffer(s) into which the decrypted data is to be placed. The total length of all the buffers shall match that of the inBufList. The flags field in each buffer is ignored. The function shall ensure that the buffers are entirely within the memory area protected from access by the application CPU unless the CpuTgtOK bit in the keyID structure is set to 1.</param>
/// <param name="sourceBoundaryKeyRegisterId">[IN] The index of the register holding the boundary key to use for decryption of the source data. The value IPTV_HAL_CRYPTO_KEY_REGISTER_PASSTHRU (-1) indicates "passthru mode" wherein the payload is plaintext and is not to be decrypted.</param>
/// <param name="sampleID">[IN] A 64 bit integer specifying the sample ID of the sample to be decrypted and decoded.  This sample ID is used to initialize the upper 64 bits of the 128 bit counter used when decrypting the sample ES.  (The lower 64 bits of the counter are initialized according to the sampleByteOffset field as described below).  The sampleID MUST be within the range indicated by the key identifier structure. Specifically:
///
///    (sampleID - keyID.StartingSampleID) < keyID.SampleIDLength
///
/// if the sampleID lies outside of the permitted range, no decryption shall be performed and an appropriate error code must be returned.</param>
/// <param name="sampleByteOffset">[IN] A 64 bit integer specifying the byte offset (from the start of the sample) of the first byte of the payload. This value, divided by 16, is used to initialize the lower half of the 128 bit counter used when decrypting the sample ES. If the sampleByteOffset modulo 16 has a value N != 0, then the first N bytes of counter mode keystream must be discarded before XORing it with the encrypted payload. (Equivalently, N arbitrary bytes can be prepended to the encrypted payload and the first N bytes of the plaintext discarded). </param>
/// <param name="keyID">[IN] a structure identifying the key to be used to decrypt the sample in question. Its layout is specified in section 6.1. In addition to the GUID of the boundary key to use in order to decrypt the AV data, the keyID structure contains important sample specific information, such as the Macrovision level, CGMS-A flags, and blackouts to be associated with the decoded sample.  The keyID structure also indicates a range of sampleIDs that these sample specific flags apply to.  If the sampleID parameter lies outside of the range indicated by the keyID structure, no decryption operation may be performed and an appropriate error code MUST be returned. If there is blackout information in the key identifier, then decryption MUST only take place if the boundary key or GRC membership token has the appropriate settings, as detailed in the Crypto HAL API document.</param>
/// <param name="keyIDLength">[IN] The length of the keyID buffer in bytes.</param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors.</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_BAD_KEY_ID_SIGNATURE:</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_BAD_KEY_REGISTER_INDEX:</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_BAD_POINTER:</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_BUFFER_TOO_LARGE:</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_BUFFER_TOO_SMALL:</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_CONTENT_BLACKED_OUT:</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_KEY_ID_BUFFER_TOO_SMALL:</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_NO_KEY_IN_REGISTER:</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_NOT_INITIALIZED:</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_OUT_OF_MEMORY:</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_SAMPLE_ID_OUT_OF_RANGE:</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_UNSUPPORTED_KEY_ID_VERSION:</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_WRONG_BOUNDARY_KEY:</para>
/// <para>IPTV_HAL_ERROR_CRYPTO_BLACKOUT_VECTOR_TOO_LARGE:</para>
/// </returns>
iptv_hal_error IPTV_HAL_Crypto_DecryptAVPayload(PIPTV_HAL_BUFFER pInBufList,
                                                PIPTV_HAL_BUFFER pOutBufList,
                                                IPTV_HAL_CRYPTO_KEY_REGISTER_INDEX sourceBoundaryKeyRegisterId,
                                                UINT64 sampleID,
                                                UINT64 sampleByteOffset,
                                                PBYTE keyID,
                                                UINT32 keyIDLength)
{
    iptv_hal_error retval = IPTV_HAL_ERROR_NOT_SUPPORTED;

    if (IPTV_HAL_CRYPTO_KEY_REGISTER_PASSTHRU == sourceBoundaryKeyRegisterId)
    {
        retval = IPTV_HAL_ERROR_SUCCESS;

        PIPTV_HAL_BUFFER pInBuffer = pInBufList;
        UINT32 cbTakenFromInBuffer = 0;

        // For each buffer in the output chain

        for( PIPTV_HAL_BUFFER pOutBuffer = pOutBufList; pOutBuffer != NULL; pOutBuffer = pOutBuffer->pNext )
        {
            // Initialize the output buffer

            pOutBuffer->u32DataStart = 0;
            pOutBuffer->u32DataEnd = 0;

            // While there's room in the output buffer and data
            // in the input chain, copy as much data as possible

            while( ( pOutBuffer->u32DataEnd < pOutBuffer->u32Size ) && ( pInBuffer != NULL ) )
            {
                if( pInBuffer->u32DataEnd < pInBuffer->u32DataStart )
                {
                    retval = IPTV_HAL_ERROR_INVALID_PARAMETER;
                    break;
                }

                UINT32 cbCapacity = pOutBuffer->u32Size - pOutBuffer->u32DataEnd;
                UINT32 cbCopy = pInBuffer->u32DataEnd - pInBuffer->u32DataStart - cbTakenFromInBuffer;

                cbCopy = ( cbCapacity < cbCopy ) ? cbCapacity : cbCopy;

                memcpy(
                    &pOutBuffer->pBuf[ pOutBuffer->u32DataEnd ],
                    &pInBuffer->pBuf[ pInBuffer->u32DataStart + cbTakenFromInBuffer ],
                    cbCopy );

                pOutBuffer->u32DataEnd += cbCopy;

                cbTakenFromInBuffer += cbCopy;

                if( cbTakenFromInBuffer >= pInBuffer->u32DataEnd - pInBuffer->u32DataStart )
                {
                    // Current input buffer fully copied.
                    // Move to the next.

                    cbTakenFromInBuffer = 0;

                    pInBuffer = pInBuffer->pNext;
                }
            }
        }

        if( ( retval == IPTV_HAL_ERROR_SUCCESS ) && ( pInBuffer != NULL ) )
        {
            // Unexpected failure. There's data left in the input chain.
            // The output chain was not large enough.
            retval = IPTV_HAL_ERROR_DECODER_DECRYPT_FAILED;
        }
    }

    return retval;
}

//.End
