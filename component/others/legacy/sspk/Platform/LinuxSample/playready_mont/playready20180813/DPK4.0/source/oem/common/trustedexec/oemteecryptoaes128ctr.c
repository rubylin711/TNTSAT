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
** This function is used in all PlayReady scenarios.
**
** Synopsis:
**
** This function encrypts/decrypts the given data with an AES key using AES CTR.
**
** Operations Performed:
**
**  1. AES CTR encrypt/decrypt the data using the provided Key.
**
** Arguments:
**
** f_pContext:              (in/out) The TEE context returned from
**                                   OEM_TEE_BASE_AllocTEEContext.
** f_pKey:                      (in) AES Key to encrypt the data.
** f_pbData:                (in/out) Buffer holding the clear/encrypted data and receive the encrypted/clear data.
** f_cbData:                    (in) Length of the data to encrypt/decrypt.
** f_pCtrContext:           (in/out) CTR mode context.
**
** Returns:
**
** On failure, this function should return an explicitly-defined error from
** drmresults.h OR an OEM-defined error code in the range 0x8004dd00 to 0x8004ddff.
**
*/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL OEM_TEE_CRYPTO_AES128_CtrProcessData(
    __inout                                               OEM_TEE_CONTEXT              *f_pContext,
    __in_ecount( 1 )                                const OEM_TEE_KEY                  *f_pKey,
    __inout_bcount( f_cbData )                            DRM_BYTE                     *f_pbData,
    __in                                                  DRM_DWORD                     f_cbData,
    __inout_ecount( 1 )                                   DRM_AES_COUNTER_MODE_CONTEXT *f_pCtrContext )
{
    /*char * tmp_key = OEM_TEE_INTERNAL_CONVERT_OEM_TEE_KEY_AES128_TO_DRM_BYTES( f_pKey );
        printf("key 2: ");
        for(int aa =0; aa < 16; aa ++)
            printf("0x%x ", *(tmp_key+aa)&0xFF);
        printf("\n");*/
    //return Oem_Aes_CtrProcessData( OEM_TEE_INTERNAL_CONVERT_OEM_TEE_KEY_AES128_TO_BROKER( f_pKey ), f_pbData, f_cbData, f_pCtrContext );
    return Oem_Aes_CtrProcessData_Mont( OEM_TEE_INTERNAL_CONVERT_OEM_TEE_KEY_AES128_TO_DRM_BYTES( f_pKey ), DRM_AES_KEYSIZE_128, f_pbData, f_cbData, f_pCtrContext );
}

EXIT_PK_NAMESPACE_CODE;

