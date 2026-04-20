/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

/*
** oemaes.c
**
** This file implements AES encryption of a single block sized buffer
** Each OEM is expected to implement its own version of these routines.
*/


#define DRM_BUILDING_OEMAES_C
#include <string.h>

#include <oemaes.h>
#include <openssl/aes.h>
#include "aes128.h"
#include <oem.h>
#include <drmprofile.h>
#include <drmlastinclude.h>




ENTER_PK_NAMESPACE_CODE;

/*********************************************************************************************
** Function:  Oem_Aes_ZeroKey
**
** Synopsis:  Zeroes out an AES secret key
**
** Arguments: [f_pKey]  : The key to zero
**
** Returns:   DRM_SUCCESS
**              Success
**            DRM_E_INVALIDARG
**              f_pKey was NULL
**********************************************************************************************/
DRM_API DRM_RESULT DRM_CALL Oem_Aes_ZeroKey(
    __inout_ecount( 1 )  DRM_AES_KEY  *f_pKey )
{
    DRM_RESULT            dr           = DRM_SUCCESS;

    ChkArg( f_pKey != NULL );

    /*
    ** Clear the AES key itself for security reasons
    */
    OEM_SECURE_ZERO_MEMORY( f_pKey, sizeof( DRM_AES_KEY ) );

ErrorExit:
    return dr;
}


/*********************************************************************************************
** Function:  Oem_Aes_SetKey
**
** Synopsis:  Initializes an AES secret key given the value of the key.
**
** Arguments: [f_rgbKey]  : Buffer containing the key data
**            [f_pAesKey] : Returns the AES key structure
**
** Returns:   DRM_SUCCESS
**               Success
**            DRM_E_INVALIDARG
**               One of the pointers was NULL or f_cbKey was an invalid size
**********************************************************************************************/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Oem_Aes_SetKey(
    __in_bcount( DRM_AES_KEYSIZE_128 ) const DRM_BYTE      f_rgbKey[ DRM_AES_KEYSIZE_128 ],
    __out_ecount( 1 )                        DRM_AES_KEY  *f_pAesKey )
{
    CLAW_AUTO_RANDOM_CIPHER
    DRM_RESULT            dr           = DRM_SUCCESS;
    INTERNAL_DRM_AES_KEY *pInternalKey = DRM_REINTERPRET_CAST( INTERNAL_DRM_AES_KEY, f_pAesKey );

    DRM_PROFILING_ENTER_SCOPE( PERF_MOD_OEMAES, PERF_FUNC_Oem_Aes_SetKey );

    ChkArg ( f_pAesKey != NULL );
    ChkArg ( f_rgbKey   != NULL );

    /*
    ** Copy the actual key into the buffer
    */
    OEM_SECURE_MEMCPY( pInternalKey->rgbKey, f_rgbKey, DRM_AES_KEYSIZE_128 );

    Aes_CreateKey128( &pInternalKey->AESTable, pInternalKey->rgbKey );

ErrorExit:
    DRM_PROFILING_LEAVE_SCOPE;
    return dr;
}

/*********************************************************************************************
** Function:  Oem_Aes_EncryptOne
**
** Synopsis:  AES encrypts one block in-place.
**
** Arguments: [f_pKey]    : A pointer to the opaque key structure
**            [f_rgbData] : Specifies the data to encrypt (in place)
**
** Returns:   DRM_SUCCESS
**                success
**            DRM_E_INVALIDARG
**                one of the arguments was NULL or improperly initialized
**            DRM_E_CIPHER_NOT_INITIALIZED
**                The aes key has not been properly initialized with Oem_Aes_SetKey
**********************************************************************************************/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Oem_Aes_EncryptOne(
    __in_ecount( 1 )             const DRM_AES_KEY  *f_pKey,
    __inout_bcount( DRM_AES_BLOCKLEN ) DRM_BYTE      f_rgbData[ DRM_AES_BLOCKLEN ] )
{
    CLAW_AUTO_RANDOM_CIPHER
    DRM_RESULT                  dr           = DRM_SUCCESS;
    const INTERNAL_DRM_AES_KEY *pInternalKey = DRM_REINTERPRET_CONST_CAST( const INTERNAL_DRM_AES_KEY, f_pKey );

    /*
    DRM_PROFILING_ENTER_SCOPE( PERF_MOD_OEMAES, PERF_FUNC_Oem_Aes_EncryptOne );
    */

    ChkArg( f_pKey != NULL );

    #if 0
    /*
    ** Simply use the system's AES routine
    */
     Aes_Encrypt128( f_rgbData, f_rgbData, pInternalKey->AESTable.keytabenc );
    #else
    AES_KEY wctx;
    //unsigned char tmp[16];
    //memcpy ( tmp, f_rgbData, 16 );
    AES_set_encrypt_key(pInternalKey->rgbKey, 128, &wctx);
    AES_encrypt(f_rgbData, f_rgbData, &wctx);

    #endif

ErrorExit:
    /*
    DRM_PROFILING_LEAVE_SCOPE;
    */
    return dr;
}


/*********************************************************************************************
** Function:  Oem_Aes_DecryptOne
**
** Synopsis:  AES decrypts one block in-place.
**
** Arguments: [f_pKey]    : A pointer to the opaque key structure
**            [f_rgbData] : Specifies the data to decrypt (in place)
**
** Returns:   DRM_SUCCESS
**                success
**            DRM_E_INVALIDARG
**                one of the arguments was NULL or improperly initialized
**            DRM_E_CIPHER_NOT_INITIALIZED
**                The aes key has not been properly initialized with Oem_Aes_SetKey
**********************************************************************************************/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Oem_Aes_DecryptOne(
    __in_ecount( 1 )             const DRM_AES_KEY  *f_pKey,
    __inout_bcount( DRM_AES_BLOCKLEN ) DRM_BYTE      f_rgbData[ DRM_AES_BLOCKLEN ] )
{
    CLAW_AUTO_RANDOM_CIPHER
    DRM_RESULT                  dr           = DRM_SUCCESS;
    const INTERNAL_DRM_AES_KEY *pInternalKey = DRM_REINTERPRET_CONST_CAST( const INTERNAL_DRM_AES_KEY, f_pKey );
    DRM_BYTE                    rgbTemp[ DRM_AES_BLOCKLEN ];

    /*
    DRM_PROFILING_ENTER_SCOPE( PERF_MOD_OEMAES, PERF_FUNC_Oem_Aes_DecryptOne );
    */

    ChkArg( f_pKey != NULL );

    /*
    ** Simply use the system's AES routine
    */
#if 0
    Aes_Decrypt128( rgbTemp, f_rgbData, pInternalKey->AESTable.keytabdec );
    OEM_SECURE_MEMCPY( f_rgbData, rgbTemp, DRM_AES_BLOCKLEN );
#else
    AES_KEY wctx;
    AES_set_decrypt_key(pInternalKey->rgbKey, 128, &wctx);
    AES_decrypt(f_rgbData, f_rgbData, &wctx);
#endif

ErrorExit:
    /*
    DRM_PROFILING_LEAVE_SCOPE;
    */
    return dr;
}


void  Oem_Mont_Aes_Setencryptkey(
    DRM_BYTE       *f_pKey,
    DRM_WCHAR32     len,
    void           *ptr)
{
    AES_KEY  *p_wctx = (AES_KEY  *)ptr;
    AES_set_encrypt_key(f_pKey, len, p_wctx);
}

void  Oem_Mont_Aes_Setdecryptkey(
    DRM_BYTE       *f_pKey,
    DRM_WCHAR32     len,
    void           *ptr)
{
    AES_KEY  *p_wctx = (AES_KEY  *)ptr;
    AES_set_decrypt_key(f_pKey, len, p_wctx);
}


void  Oem_Mont_Aes_EncryptOne(
    DRM_BYTE      f_rgbData[ DRM_AES_BLOCKLEN ] ,
    void          *ptr)
{
    AES_KEY  *p_wctx = (AES_KEY  *)ptr;
    AES_encrypt(f_rgbData, f_rgbData, p_wctx);
}

void  Oem_Mont_Aes_DecryptOne(
    DRM_BYTE      f_rgbData[ DRM_AES_BLOCKLEN ] ,
    void          *ptr)
{
    AES_KEY  *p_wctx = (AES_KEY  *)ptr;
    AES_decrypt(f_rgbData, f_rgbData, p_wctx);
}

EXIT_PK_NAMESPACE_CODE;
