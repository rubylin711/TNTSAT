///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/// <summary>
///     IPTVCryptoHal.h
///        This file contains definitions of Crypto Core APIs that need to be implemented by
///        the mediaprocessor silicon vendor in order to interface with Microsoft MediaRoom.
/// </summary>

#pragma once

#include "IPTVError.h"
#include "IPTVPhysMemMgr.h"
/// <topic name="DataStructs" displayname="Crypto HAL Data Structures"> </topic>

/// <summary>
/// Cipher Mode Types.
/// </summary>
/// <remarks>
/// IPTV_HAL_CRYPTO_CIPHER_MODE_ECB: The ECB (Electronic Code Book) cipher mode of AES.
/// IPTV_HAL_CRYPTO_CIPHER_MODE_CBC: The CBC (Cipher Block Chaining) cipher mode of AES.
/// IPTV_HAL_CRYPTO_CIPHER_MODE_CTR: The CTR (CounTeR) cipher mode of AES.
/// </remarks>
typedef enum _IPTV_HAL_CRYPTO_CIPHER_MODE
{
    IPTV_HAL_CRYPTO_CIPHER_MODE_ECB            = 0,
    IPTV_HAL_CRYPTO_CIPHER_MODE_CBC            = 1,
    IPTV_HAL_CRYPTO_CIPHER_MODE_CTR            = 2,
} IPTV_HAL_CRYPTO_CIPHER_MODE;

/// <summary>
/// Key Register Index
/// </summary>
/// <remarks>
/// The following type is used to select a key register. Some values are defined to indicate special behavior of the function.
/// </remarks>
typedef int IPTV_HAL_CRYPTO_KEY_REGISTER_INDEX;
#define IPTV_HAL_CRYPTO_KEY_REGISTER_PASSTHRU (-1)

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
iptv_hal_error IPTV_HAL_Crypto_DecryptAVPayload(PIPTV_HAL_BUFFER inBufList,
                                                PIPTV_HAL_BUFFER outBufList,
                                                IPTV_HAL_CRYPTO_KEY_REGISTER_INDEX sourceBoundaryKeyRegisterId,
                                                UINT64 sampleID,
                                                UINT64 sampleByteOffset,
                                                PBYTE keyID,
                                                UINT32 keyIDLength);

//.End
