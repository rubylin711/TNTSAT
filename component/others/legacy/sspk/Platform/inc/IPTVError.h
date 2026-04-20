///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/************************************************************************
*                                                                       *
*  iptverror.h --  error code definitions for the IPTV API functions    *
*                                                                       *
************************************************************************/

#ifndef _IPTVERROR_H_
#define _IPTVERROR_H_

// Please always use these macros for an error code check
// If the macro is broken, it is easy to fix it one place.
#define IPTV_HAL_ERROR_RET_FAILED(code) (code != IPTV_HAL_ERROR_SUCCESS)
#define IPTV_HAL_ERROR_RET_SUCCESS(code) (code == IPTV_HAL_ERROR_SUCCESS)

// We have 2047 generic error codes.
// Each funtional block (crypto, graphics, decoders...) can have its own 2048 unique codes
#define IPTV_HAL_ERROR_CATEGORY_START (0)
#define IPTV_HAL_ERROR_CATEGORY_SIZE (2048)

#define IPTV_HAL_ERROR_DECODER_START (IPTV_HAL_ERROR_CATEGORY_START + IPTV_HAL_ERROR_CATEGORY_SIZE)
#define IPTV_HAL_ERROR_GFX_START (IPTV_HAL_ERROR_DECODER_START + IPTV_HAL_ERROR_CATEGORY_SIZE)
#define IPTV_HAL_ERROR_PERIPHERAL_START (IPTV_HAL_ERROR_GFX_START + IPTV_HAL_ERROR_CATEGORY_SIZE)
#define IPTV_HAL_ERROR_CRYPTO_START (IPTV_HAL_ERROR_PERIPHERAL_START + IPTV_HAL_ERROR_CATEGORY_SIZE)
#define IPTV_HAL_ERROR_DEMUX_START (IPTV_HAL_ERROR_CRYPTO_START + IPTV_HAL_ERROR_CATEGORY_SIZE)
#define IPTV_HAL_ERROR_CRYPTO(X, Y) \
        IPTV_HAL_ERROR_CRYPTO_ ## X = (IPTV_HAL_ERROR_CRYPTO_START + Y)

typedef enum _IPTV_HAL_ERROR
{
    //-------------------------------------------------------------//
    // Generic Error codes
     IPTV_HAL_ERROR_SUCCESS = IPTV_HAL_ERROR_CATEGORY_START,
    IPTV_HAL_ERROR_INVALID_PARAMETER,
    IPTV_HAL_ERROR_NOT_INITIALIZED,
    IPTV_HAL_ERROR_NOT_SUPPORTED,
    IPTV_HAL_ERROR_BUFFER_TOO_SMALL,
    IPTV_HAL_ERROR_ALREADY_INITIALIZED,
    IPTV_HAL_ERROR_OUT_OF_MEMORY,
    IPTV_HAL_ERROR_TIMEOUT,
    IPTV_HAL_ERROR_DEVICEERROR,
    //-------------------------------------------------------------//

    IPTV_HAL_ERROR_FAILED = 0x8000000,

      // Error codes specific to HAL implementations

      // DecoderHAL specific Error Codes
    IPTV_HAL_ERROR_DECODER_NO_MORE_DECODERS = IPTV_HAL_ERROR_DECODER_START,
    IPTV_HAL_ERROR_DECODER_ESFIFO_FULL,
    IPTV_HAL_ERROR_DECODER_DECRYPT_FAILED,

    //-------------------------------------------------------------//

      // DemuxHAL specific Error Codes
    IPTV_HAL_ERROR_DEMUX_XXX = IPTV_HAL_ERROR_DEMUX_START,
    ///<summary>DemuxHAL No Error.</summary>
    IPTV_HAL_DEMUX_ERROR_NONE = 0,

    ///<summary>DemuxHAL Generic Error - no further info available.</summary>
    IPTV_HAL_DEMUX_ERROR_FAIL,

    ///<summary>DemuxHAL Invalid demux instance index passed to the driver.</summary>
    IPTV_HAL_DEMUX_ERROR_INVALID_INDEX,

    ///<summary>DemuxHAL Invalid handle passed to the driver.</summary>
    IPTV_HAL_DEMUX_ERROR_INVALID_HANDLE,

    ///<summary>DemuxHAL Invalid command passed to the driver.</summary>
    IPTV_HAL_DEMUX_ERROR_INVALID_COMMAND,

    ///<summary>DemuxHAL Driver unable to execute the command.</summary>
    IPTV_HAL_DEMUX_ERROR_COMMAND_FAILURE,

    ///<summary>DemuxHAL Invalid pointer</summary>
    IPTV_HAL_DEMUX_ERROR_INVALID_POINTER,

    ///<summary>DemuxHAL Out of memory or not enough buffers left.</summary>
    IPTV_HAL_DEMUX_ERROR_OUT_OF_MEMORY,

    ///<summary>DemuxHAL Out of demux hardware resources, PID filter or section filter.</summary>
    IPTV_HAL_DEMUX_ERROR_OUT_OF_RESOURCES,

    ///<summary>DemuxHAL PID value specified is illegal (must be in the MPEG defined ranage of 0-0x1FFF).</summary>
    IPTV_HAL_DEMUX_ERROR_INVALID_PID,

    ///<summary>DemuxHAL Invalid length parameter passed.</summary>
    IPTV_HAL_DEMUX_ERROR_INVALID_LENGTH,

    ///<summary>DemuxHAL Invalid cipher type parameter passed.</summary>
    IPTV_HAL_DEMUX_ERROR_INVALID_CIPHER_TYPE,

    ///<summary>DemuxHAL The PID has not been associated with a cipher with IPTV_HAL_Demux_Cipher_Add.</summary>
    IPTV_HAL_DEMUX_ERROR_CIPHER_NO_PID,

    ///<summary>DemuxHAL Invalid section size specified must be either 1024 or 4096.</summary>
    IPTV_HAL_DEMUX_ERROR_INVALID_SECTION_SIZE,

    //-------------------------------------------------------------//
      // Graphics HAL specific Error Codes
    IPTV_HAL_ERROR_GFX_INVALID_HANDLE = IPTV_HAL_ERROR_GFX_START,
    //-------------------------------------------------------------//

    // Crypto HAL specific Error Codes
    ///<summary>CryptoHAL: The function succeeded.</summary>
    IPTV_HAL_ERROR_CRYPTO_SUCCESS = IPTV_HAL_ERROR_SUCCESS,
    ///<summary>CryptoHAL: Non-specific error.</summary>
    IPTV_HAL_ERROR_CRYPTO(FAILED,1),
    ///<summary>CryptoHAL: The signature of the encrypted boundary key structure fails to verify.</summary>
    IPTV_HAL_ERROR_CRYPTO(BAD_BOUNDARY_KEY_SIGNATURE,2),
    ///<summary>CryptoHAL: The signature of the key identifier structure fails to verify.</summary>
    IPTV_HAL_ERROR_CRYPTO(BAD_KEY_ID_SIGNATURE,3),
    ///<summary>CryptoHAL: The key is not one of the permissible lengths (128, 192, 256 bits).</summary>
    IPTV_HAL_ERROR_CRYPTO(BAD_KEY_LENGTH,4),
    ///<summary>CryptoHAL: The key register index is not in the permitted range.</summary>
    IPTV_HAL_ERROR_CRYPTO(BAD_KEY_REGISTER_INDEX,5),
    ///<summary>CryptoHAL: The message length is inappropriate for the requested operation, e.g. AES-CBS encryption of a non-integral number of 16-byte blocks, RSA encryption of a message larger than the modulus, etc.</summary>
    IPTV_HAL_ERROR_CRYPTO(BAD_MESSAGE_LENGTH,6),
    ///<summary>CryptoHAL: A pointer is null.</summary>
    IPTV_HAL_ERROR_CRYPTO(BAD_POINTER,7),
    ///<summary>CryptoHAL: The encrypted boundary key structure is too small to be valid.</summary>
    IPTV_HAL_ERROR_CRYPTO(BOUNDARY_KEY_BUFFER_TOO_SMALL,8),
    ///<summary>CryptoHAL: Attempt to use a boundary key after its expiration time [not required to be checked in this version].</summary>
    IPTV_HAL_ERROR_CRYPTO(BOUNDARY_KEY_EXPIRED,9),
    ///<summary>CryptoHAL: The buffer passed into one of the functions exceeds an implementation limit. </summary>
    IPTV_HAL_ERROR_CRYPTO(BUFFER_TOO_LARGE,10),
    ///<summary>CryptoHAL: The buffer passed into one of the functions is too small to fulfil its role./summary>
    IPTV_HAL_ERROR_CRYPTO(BUFFER_TOO_SMALL,11),
    ///<summary>CryptoHAL: The content cannot be decrypted because it is subject to a blackout.</summary>
    IPTV_HAL_ERROR_CRYPTO(CONTENT_BLACKED_OUT,12),
    ///<summary>CryptoHAL: Unused.</summary>
    IPTV_HAL_ERROR_CRYPTO(CREATE_EVENT_FAILED,13),
    ///<summary>CryptoHAL: Unused.</summary>
    IPTV_HAL_ERROR_CRYPTO(INCONSISTENT_KEY,14),
    ///<summary>CryptoHAL: Unused.</summary>
    IPTV_HAL_ERROR_CRYPTO(INCORRECT_KEY,15),
    ///<summary>CryptoHAL: A non-specific error caused by some inconsistent internal behavior.</summary>
    IPTV_HAL_ERROR_CRYPTO(INTERNAL_ERROR,16),
    ///<summary>CryptoHAL: The key identifier structure is too small to be valid.</summary>
    IPTV_HAL_ERROR_CRYPTO(KEY_ID_BUFFER_TOO_SMALL,17),
    ///<summary>CryptoHAL: Unused.</summary>
    IPTV_HAL_ERROR_CRYPTO(MISSING_DRM_KEY_RING,18),
    ///<summary>CryptoHAL: Unused.</summary>
    IPTV_HAL_ERROR_CRYPTO(MISSING_SESSION_KEY,19),
    ///<summary>CryptoHAL: Unused.</summary>
    IPTV_HAL_ERROR_CRYPTO(NO_KEY,20),
    ///<summary>CryptoHAL: A key register referenced in a function parameter has not been loaded with a key.</summary>
    IPTV_HAL_ERROR_CRYPTO(NO_KEY_IN_REGISTER,21),
    ///<summary>CryptoHAL: The function is not implemented in this version.</summary>
    IPTV_HAL_ERROR_CRYPTO(NOT_IMPL,22),
    ///<summary>CryptoHAL: The cryptocore has not been initialized.</summary>
    IPTV_HAL_ERROR_CRYPTO(NOT_INITIALIZED,23),
    ///<summary>CryptoHAL: A request within the cryptocore has failed to allocate memory required to fulfill the function.</summary>
    IPTV_HAL_ERROR_CRYPTO(OUT_OF_MEMORY,24),
    ///<summary>CryptoHAL: Unused.</summary>
    IPTV_HAL_ERROR_CRYPTO(OUT_OF_KEY_REGISTERS,25),
    ///<summary>CryptoHAL: The sample is not within the permitted range for the use of this key identifier.</summary>
    IPTV_HAL_ERROR_CRYPTO(SAMPLE_ID_OUT_OF_RANGE,26),
    ///<summary>CryptoHAL: An RSA or OMAC-1 signature verification failed.</summary>
    IPTV_HAL_ERROR_CRYPTO(SIGNATURE_CHECK_FAILED,27),
    ///<summary>CryptoHAL: The permitted maximum number of blackout ids for "legacy blackouts" was exceeded.</summary>
    IPTV_HAL_ERROR_CRYPTO(TOO_MANY_BLACKOUT_IDS,28),
    ///<summary>CryptoHAL: Unused.</summary>
    IPTV_HAL_ERROR_CRYPTO(UNLOAD_ERROR_KEY_IS_IN_USE,29),
    ///<summary>CryptoHAL: Unused.</summary>
    IPTV_HAL_ERROR_CRYPTO(UNLOAD_ERROR_KEY_IS_LOADING,30),
    ///<summary>CryptoHAL: The mode parameter is not one of those permitted.</summary>
    IPTV_HAL_ERROR_CRYPTO(UNRECOGNIZED_MODE,31),
    ///<summary>CryptoHAL: The version field of the encrypted boundary key structure does not contain one of those supported.</summary>
    IPTV_HAL_ERROR_CRYPTO(UNSUPPORTED_BOUNDARY_KEY_VERSION,32),
    ///<summary>CryptoHAL: The hash type parameter is not one of those permitted.</summary>
    IPTV_HAL_ERROR_CRYPTO(UNSUPPORTED_HASH_TYPE,33),
    ///<summary>CryptoHAL: The version field of the key identifier structure does not contain one of those supported.</summary>
    IPTV_HAL_ERROR_CRYPTO(UNSUPPORTED_KEY_ID_VERSION,34),
    ///<summary>CryptoHAL: The key length, e.g.of an RSA public key, is not supported.</summary>
    IPTV_HAL_ERROR_CRYPTO(UNSUPPORTED_KEY_LENGTH,35),
    ///<summary>CryptoHAL: The key type argument of the function is not one of the permitted values.</summary>
    IPTV_HAL_ERROR_CRYPTO(UNSUPPORTED_KEY_TYPE,36),
    ///<summary>CryptoHAL: The boundary key loaded in a register and referenced in a decryption operation is not the one specified in the key identifier.</summary>
    IPTV_HAL_ERROR_CRYPTO(WRONG_BOUNDARY_KEY,37),
    ///<summary>CryptoHAL: An internal inter-process communication taking place as part of the implementation of a function has timed out.</summary>
    IPTV_HAL_ERROR_CRYPTO(TIMEOUT,38),
    ///<summary>CryptoHAL: The signature scheme parameter is not one of those permitted.</summary>
    IPTV_HAL_ERROR_CRYPTO(UNSUPPORTED_SIGNATURE_SCHEME,39),
    ///<summary>CryptoHAL: Unused.</summary>
    IPTV_HAL_ERROR_CRYPTO(BAD_ENCODING,40),
    ///<summary>CryptoHAL: Unused.</summary>
    IPTV_HAL_ERROR_CRYPTO(NO_KEY_QUEUE,41),
    ///<summary>CryptoHAL: Unused.</summary>
    IPTV_HAL_ERROR_CRYPTO(NO_RIGHTS_FOR_QUEUE,42),
    ///<summary>CryptoHAL: Unused.</summary>
    IPTV_HAL_ERROR_CRYPTO(NTP_NOT_SYNCHRONIZED,43),
    ///<summary>CryptoHAL: The GRC membership token structure is too small to be valid.</summary>
    IPTV_HAL_ERROR_CRYPTO(GRC_MEMBERSHIP_TOKEN_BUFFER_TOO_SMALL,44),
    ///<summary>CryptoHAL: The GRC membership token structure is too large to be valid.</summary>
    IPTV_HAL_ERROR_CRYPTO(GRC_MEMBERSHIP_TOKEN_BUFFER_TOO_LARGE,45),
    ///<summary>CryptoHAL: The version field of the GRC membership token structure does not contain one of those supported.</summary>
    IPTV_HAL_ERROR_CRYPTO(UNSUPPORTED_GRC_MEMBERSHIP_TOKEN_VERSION,46),
    ///<summary>CryptoHAL: The signature of the GRC membership token structure fails to verify.</summary>
    IPTV_HAL_ERROR_CRYPTO(BAD_GRC_MEMBERSHIP_TOKEN_SIGNATURE,47),
    ///<summary>CryptoHAL: The blackout vector in the Version 5 key identifier is larger than the maximum permitted.</summary>
    IPTV_HAL_ERROR_CRYPTO(BLACKOUT_VECTOR_TOO_LARGE,48),
    ///<summary>CryptoHAL: The public key used to validate an AV session key, or the AV session key used to validate a GRC membership token was not itself validated.</summary>
    IPTV_HAL_ERROR_CRYPTO(KEY_NOT_VALIDATED,49),
    ///<summary>CryptoHAL: A secure memory access was attempted by the main CPU.</summary>
    IPTV_HAL_ERROR_CRYPTO(SECURE_MEMORY_VIOLATION,50),

    //-------------------------------------------------------------//
} iptv_hal_error;

#endif
