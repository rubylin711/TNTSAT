/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#include <oemtee.h>
#include <oemteecrypto.h>
#include <oemteecryptointernaltypes.h>
#include <drmlastinclude.h>

ENTER_PK_NAMESPACE_CODE;

/*
** OEM_OPTIONAL:
** You do not need to replace this function implementation. However, this function MUST
** have a valid implementation for your PlayReady port if this function is still called
** by other OEM_TEE functions once your port is complete.
**
** Any reimplementation MUST exactly match the behavior of the default PK implementation.
**
** This function is only used by remote provisioning.
**
** Synopsis:
**
** Does AES CBC-Mode encryption on a buffer of data.
**
** Operations Performed:
**
**  1. Encrypt the given data with the provided AES key.
**
** Arguments:
**
** f_pContextAllowNULL: (in/out) The TEE context returned from
**                               OEM_TEE_BASE_AllocTEEContext.
**                               This function may receive NULL.
**                               This function may receive an
**                               OEM_TEE_CONTEXT where
**                               cbUnprotectedOEMData == 0 and
**                               pbUnprotectedOEMData == NULL.
** f_pKey:                  (in) The AES secret key used to encrypt the buffer.
** f_pbData:            (in/out) The data to be encrypted.
** f_cbData:                (in) The size (in bytes) of the data to be encrypted.
** f_rgbIV:                 (in) The AES CBC IV.
**
** Returns:
**
** On failure, this function should return an explicitly-defined error from
** drmresults.h OR an OEM-defined error code in the range 0x8004dd00 to 0x8004ddff.
**
*/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL OEM_TEE_CRYPTO_AES128_CbcEncryptData(
    __inout_opt                                           OEM_TEE_CONTEXT              *f_pContextAllowNULL,
    __in_ecount( 1 )                                const OEM_TEE_KEY                  *f_pKey,
    __inout_bcount( f_cbData )                            DRM_BYTE                     *f_pbData,
    __in                                                  DRM_DWORD                     f_cbData,
    __in_bcount( DRM_AES_BLOCKLEN )                 const DRM_BYTE                      f_rgbIV[ DRM_AES_BLOCKLEN ] )
{
    //return Oem_Aes_CbcEncryptData( OEM_TEE_INTERNAL_CONVERT_OEM_TEE_KEY_AES128_TO_BROKER( f_pKey ), f_pbData, f_cbData, f_rgbIV );
    return Oem_Aes_CbcEncryptData_Mont( OEM_TEE_INTERNAL_CONVERT_OEM_TEE_KEY_AES128_TO_DRM_BYTES( f_pKey ), DRM_AES_KEYSIZE_128, f_pbData, f_cbData, f_rgbIV );
}

/*
** OEM_OPTIONAL:
** You do not need to replace this function implementation. However, this function MUST
** have a valid implementation for your PlayReady port if this function is still called
** by other OEM_TEE functions once your port is complete.
**
** Any reimplementation MUST exactly match the behavior of the default PK implementation.
**
** This function is only used by remote provisioning.
**
** Synopsis:
**
** Does AES CBC-Mode decryption on a buffer of data.
**
** Operations Performed:
**
**  1. Decrypt the given data with the provided AES key.
**
** Arguments:
**
** f_pContextAllowNULL: (in/out) The TEE context returned from
**                               OEM_TEE_BASE_AllocTEEContext.
**                               This function may receive NULL.
**                               This function may receive an
**                               OEM_TEE_CONTEXT where
**                               cbUnprotectedOEMData == 0 and
**                               pbUnprotectedOEMData == NULL.
** f_pKey:                  (in) The AES secret key used to decrypt the buffer.
** f_pbData:            (in/out) The data to be decrypted.
** f_cbData:                (in) The size (in bytes) of the data to be decrypted.
** f_rgbIV:                 (in) The AES CBC IV.
**
** Returns:
**
** On failure, this function should return an explicitly-defined error from
** drmresults.h OR an OEM-defined error code in the range 0x8004dd00 to 0x8004ddff.
**
*/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL OEM_TEE_CRYPTO_AES128_CbcDecryptData(
    __inout_opt                                           OEM_TEE_CONTEXT              *f_pContextAllowNULL,
    __in_ecount( 1 )                                const OEM_TEE_KEY                  *f_pKey,
    __inout_bcount( f_cbData )                            DRM_BYTE                     *f_pbData,
    __in                                                  DRM_DWORD                     f_cbData,
    __in_bcount( DRM_AES_BLOCKLEN )                 const DRM_BYTE                      f_rgbIV[ DRM_AES_BLOCKLEN ] )
{
    //return Oem_Aes_CbcDecryptData( OEM_TEE_INTERNAL_CONVERT_OEM_TEE_KEY_AES128_TO_BROKER( f_pKey ), f_pbData, f_cbData, f_rgbIV );
    return Oem_Aes_CbcDecryptData_Mont( OEM_TEE_INTERNAL_CONVERT_OEM_TEE_KEY_AES128_TO_DRM_BYTES( f_pKey ), DRM_AES_KEYSIZE_128, f_pbData, f_cbData, f_rgbIV );
}

EXIT_PK_NAMESPACE_CODE;

