/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#define DRM_BUILDING_OEMAESMULTI_C
#include <drmtypes.h>
#include <drmmathsafe.h>
#include <byteorder.h>
#include <oem.h>
#include <oemaesmulti.h>
#include <oembroker.h>
#include <drmprofile.h>
#include <drmlastinclude.h>
#include <oemteecryptointernaltypes.h>
#include <openssl/aes.h>

ENTER_PK_NAMESPACE_CODE;

/*********************************************************************************************
** Function:  Oem_Aes_CbcEncryptData
**
** Synopsis:  Does AES CBC-Mode encryption on a buffer of data
**
** Arguments: [f_pKey]      : The AES secret key used to encrypt the buffer
**                            - If this API is called inside the TEE. Then OEM_AES_KEY_CONTEXT will 
**                              be pointing to an OEM_TEE_KEY_AES.
**                              OEM can convert it to a DRM_AES_KEY using 
**                              OEM_TEE_INTERNAL_CONVERT_OEM_TEE_KEY_AES128_TO_DRM_AES_KEY.
**                            - If this API is called from the normal world, then OEM_AES_KEY_CONTEXT
**                              will be pointing to a DRM_AES_KEY. A simple cast to a DRM_AES_KEY* 
**                              will be sufficient.
**            [f_pbData]    : The buffer to encrypt ( in place )
**            [f_cbData]    : The number of bytes to encrypt
**            [f_rgbIV]     : The initialization vector to use for encryption
**
** Returns:   DRM_SUCCESS
**              Success
**            DRM_E_INVALIDARG
**               One of the pointers was NULL, or the byte count is 0
**               or not a multiple of DRM_AES_BLOCKLEN
**            DRM_E_CRYPTO_FAILED
**              The encrypt operation failed
**********************************************************************************************/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Oem_Aes_CbcEncryptData(
    __in_ecount( 1 )                const OEM_AES_KEY_CONTEXT   *f_pKey,
    __inout_bcount( f_cbData )            DRM_BYTE              *f_pbData,
    __in                                  DRM_DWORD              f_cbData,
    __in_bcount( DRM_AES_BLOCKLEN ) const DRM_BYTE               f_rgbIV[ DRM_AES_BLOCKLEN ] )
{
    CLAW_AUTO_RANDOM_CIPHER
    DRM_RESULT dr           =  DRM_SUCCESS;
    DRM_DWORD  cbDataLeft   = f_cbData;
    DRM_DWORD  ibDataOutCur = 0;

    DRM_PROFILING_ENTER_SCOPE( PERF_MOD_DRMAES, PERF_FUNC_Oem_Aes_CbcEncryptData );

    ChkArg( f_pbData != NULL );
    ChkArg( f_cbData >= DRM_AES_BLOCKLEN );
    ChkArg( f_cbData % DRM_AES_BLOCKLEN ==  0 );

    /*
    ** The first block is a special case: Use the IV to XOR
    */
    DRM_XOR( f_pbData, f_rgbIV, DRM_AES_BLOCKLEN );
//    dr = Oem_Broker_Aes_EncryptOneBlock( f_pKey, f_pbData );
    dr = Oem_Aes_EncryptOne( f_pKey, f_pbData );
    ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );
    cbDataLeft -= DRM_AES_BLOCKLEN;

    while( cbDataLeft > 0 )
    {
        __analysis_assume( ibDataOutCur + DRM_AES_BLOCKLEN + DRM_AES_BLOCKLEN < f_cbData );
        DRM_XOR( &( f_pbData[ ibDataOutCur + DRM_AES_BLOCKLEN ] ), &( f_pbData[ ibDataOutCur ] ), DRM_AES_BLOCKLEN );
        ibDataOutCur += DRM_AES_BLOCKLEN;
        //dr = Oem_Broker_Aes_EncryptOneBlock( f_pKey, &f_pbData[ibDataOutCur] );
        dr = Oem_Aes_EncryptOne( f_pKey, &f_pbData[ibDataOutCur] );
        ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );
        cbDataLeft -= DRM_AES_BLOCKLEN;
    }

ErrorExit:
    DRM_PROFILING_LEAVE_SCOPE;
    return dr;
}

/*********************************************************************************************
** Function:  Oem_Aes_CbcDecryptData
**
** Synopsis:  Does AES CBC-Mode decryption on a buffer of data
**
** Arguments: [f_pKey]      : The AES secret key used to decrypt the buffer
**                            - If this API is called inside the TEE. Then OEM_AES_KEY_CONTEXT will 
**                              be pointing to an OEM_TEE_KEY_AES.
**                              OEM can convert it to a DRM_AES_KEY using 
**                              OEM_TEE_INTERNAL_CONVERT_OEM_TEE_KEY_AES128_TO_DRM_AES_KEY.
**                            - If this API is called from the normal world, then OEM_AES_KEY_CONTEXT
**                              will be pointing to a DRM_AES_KEY. A simple cast to a DRM_AES_KEY* 
**                              will be sufficient.
**            [f_pbData]    : The buffer to decrypt ( in place )
**            [f_cbData]    : The number of bytes to decrypt
**            [f_rgbIV]     : The initialization vector to use for decryption
**
** Returns:   DRM_SUCCESS
**              Success
**            DRM_E_INVALIDARG
**               One of the pointers was NULL, or the byte count is 0
**               or not a multiple of DRM_AES_BLOCKLEN
**            DRM_E_CRYPTO_FAILED
**              The decrypt operation failed
**********************************************************************************************/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Oem_Aes_CbcDecryptData(
    __in_ecount( 1 )                const OEM_AES_KEY_CONTEXT   *f_pKey,
    __inout_bcount( f_cbData )            DRM_BYTE              *f_pbData,
    __in                                  DRM_DWORD              f_cbData,
    __in_bcount( DRM_AES_BLOCKLEN ) const DRM_BYTE               f_rgbIV[ DRM_AES_BLOCKLEN ] )
{
    CLAW_AUTO_RANDOM_CIPHER
    DRM_RESULT dr           =  DRM_SUCCESS;
    DRM_DWORD  ibDataOutCur = 0;
    DRM_DWORD  i            = 0;
    DRM_BYTE   rgbTemp1[ DRM_AES_BLOCKLEN ];
    DRM_BYTE   rgbTemp2[ DRM_AES_BLOCKLEN ];
    int aa= 0;

    DRM_PROFILING_ENTER_SCOPE( PERF_MOD_DRMAES, PERF_FUNC_Oem_Aes_CbcDecryptData );

    ChkArg( f_pbData != NULL );
    ChkArg( f_cbData >= DRM_AES_BLOCKLEN );
    ChkArg( f_cbData % DRM_AES_BLOCKLEN ==  0 );

    //OEM_TEE_KEY_AES *tmp2 = (OEM_TEE_KEY_AES *)f_pKey;

    if(0)
    {
        printf("pInternalKey->rgbKey ");
        const INTERNAL_DRM_AES_KEY *pInternalKey = DRM_REINTERPRET_CONST_CAST( const INTERNAL_DRM_AES_KEY, f_pKey);
        for(aa=0; aa< DRM_AES_KEYSIZE_128; aa++)
            printf("0x%x ", pInternalKey->rgbKey[aa]);
        //printf(", 0x%x\n", tmp2->oKey);
        //printf("tmp2->rgbRawKey ");
        //for(aa=0; aa< DRM_AES_KEYSIZE_128; aa++)
        //    printf("0x%x ", tmp2->rgbRawKey[aa]);
        printf("\n");
        printf("f_pbData ");
        for(aa=0; aa< DRM_AES_BLOCKLEN; aa++)
        {
            printf("0x%x ",*(f_pbData+aa) );
        }
        printf("\n");
    }
    //zxcbc_num++;
    /*
    ** The first block is a special case: Use the IV to XOR
    */
    OEM_SECURE_MEMCPY( rgbTemp1, f_pbData, DRM_AES_BLOCKLEN );
    //dr = Oem_Broker_Aes_DecryptOneBlock( f_pKey, f_pbData );
    dr = Oem_Aes_DecryptOne( f_pKey, f_pbData );
    DRM_XOR( f_pbData, f_rgbIV, DRM_AES_BLOCKLEN );
    ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );
    ibDataOutCur += DRM_AES_BLOCKLEN;

    for( ; ibDataOutCur < f_cbData; i++, ibDataOutCur += DRM_AES_BLOCKLEN )
    {
        OEM_SECURE_MEMCPY( ( i % 2 == 0? rgbTemp2 : rgbTemp1 ), f_pbData + ibDataOutCur, DRM_AES_BLOCKLEN );
        //dr = Oem_Broker_Aes_DecryptOneBlock( f_pKey, &f_pbData[ibDataOutCur] );
        dr = Oem_Aes_DecryptOne( f_pKey, &f_pbData[ibDataOutCur] );
        ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );

        __analysis_assume( ibDataOutCur + DRM_AES_BLOCKLEN < f_cbData );
        DRM_XOR( &( f_pbData[ ibDataOutCur ] ), ( i % 2 == 0 ? rgbTemp1 : rgbTemp2 ), DRM_AES_BLOCKLEN );
    }

    if(0)
    {
        printf("Decrypt f_pbData ");
        for(aa=0; aa< DRM_AES_BLOCKLEN; aa++)
            printf("0x%x ", *(f_pbData+aa) );
        printf("\n");
    }
ErrorExit:
    DRM_PROFILING_LEAVE_SCOPE;
    return dr;
}


DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Oem_Aes_CbcEncryptData_Mont(
    __in_ecount( f_keyData )              DRM_BYTE              *f_pKey,
    __in                                  DRM_DWORD              f_keyData,
    __inout_bcount( f_cbData )            DRM_BYTE              *f_pbData,
    __in                                  DRM_DWORD              f_cbData,
    __in_bcount( DRM_AES_BLOCKLEN ) const DRM_BYTE               f_rgbIV[ DRM_AES_BLOCKLEN ] )
{
    CLAW_AUTO_RANDOM_CIPHER
    DRM_RESULT dr           =  DRM_SUCCESS;
    DRM_DWORD  cbDataLeft   = f_cbData;
    DRM_DWORD  ibDataOutCur = 0;

    //DRM_PROFILING_ENTER_SCOPE( PERF_MOD_DRMAES, PERF_FUNC_Oem_Aes_CbcEncryptData );

    AES_KEY  wctx;
    Oem_Mont_Aes_Setencryptkey(f_pKey, 128, (void *)&wctx);
    //ChkArg( f_pbData != NULL );
    //ChkArg( f_cbData >= DRM_AES_BLOCKLEN );
    //ChkArg( f_cbData % DRM_AES_BLOCKLEN ==  0 );

    /*
    ** The first block is a special case: Use the IV to XOR
    */
    DRM_XOR( f_pbData, f_rgbIV, DRM_AES_BLOCKLEN );
//    dr = Oem_Broker_Aes_EncryptOneBlock( f_pKey, f_pbData );
    Oem_Mont_Aes_EncryptOne( f_pbData, (void *)&wctx);
    //ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );
    cbDataLeft -= DRM_AES_BLOCKLEN;

    while( cbDataLeft > 0 )
    {
        __analysis_assume( ibDataOutCur + DRM_AES_BLOCKLEN + DRM_AES_BLOCKLEN < f_cbData );
        DRM_XOR( &( f_pbData[ ibDataOutCur + DRM_AES_BLOCKLEN ] ), &( f_pbData[ ibDataOutCur ] ), DRM_AES_BLOCKLEN );
        ibDataOutCur += DRM_AES_BLOCKLEN;
        Oem_Mont_Aes_EncryptOne( &f_pbData[ibDataOutCur], (void *)&wctx );
        //ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );
        cbDataLeft -= DRM_AES_BLOCKLEN;
    }

ErrorExit:
    //DRM_PROFILING_LEAVE_SCOPE;
    return dr;
}

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Oem_Aes_CbcDecryptData_Mont(
    __in_ecount( f_keyData )              DRM_BYTE              *f_pKey,
    __in                                  DRM_DWORD              f_keyData,
    __inout_bcount( f_cbData )            DRM_BYTE              *f_pbData,
    __in                                  DRM_DWORD              f_cbData,
    __in_bcount( DRM_AES_BLOCKLEN ) const DRM_BYTE               f_rgbIV[ DRM_AES_BLOCKLEN ] )
{
    CLAW_AUTO_RANDOM_CIPHER
    DRM_RESULT dr           =  DRM_SUCCESS;
    DRM_DWORD  ibDataOutCur = 0;
    DRM_DWORD  i            = 0;
    DRM_BYTE   rgbTemp1[ DRM_AES_BLOCKLEN ];
    DRM_BYTE   rgbTemp2[ DRM_AES_BLOCKLEN ];
    int aa= 0;

    //DRM_PROFILING_ENTER_SCOPE( PERF_MOD_DRMAES, PERF_FUNC_Oem_Aes_CbcDecryptData );

    //ChkArg( f_pbData != NULL );
    //ChkArg( f_cbData >= DRM_AES_BLOCKLEN );
    //ChkArg( f_cbData % DRM_AES_BLOCKLEN ==  0 );
    AES_KEY  wctx;
    Oem_Mont_Aes_Setdecryptkey(f_pKey, 128, (void *)&wctx);

    //zxcbc_num++;
    /*
    ** The first block is a special case: Use the IV to XOR
    */
    OEM_SECURE_MEMCPY( rgbTemp1, f_pbData, DRM_AES_BLOCKLEN );

    Oem_Mont_Aes_DecryptOne( f_pbData, (void *)&wctx );

    DRM_XOR( f_pbData, f_rgbIV, DRM_AES_BLOCKLEN );
    //ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );
    ibDataOutCur += DRM_AES_BLOCKLEN;

    for( ; ibDataOutCur < f_cbData; i++, ibDataOutCur += DRM_AES_BLOCKLEN )
    {
        OEM_SECURE_MEMCPY( ( i % 2 == 0? rgbTemp2 : rgbTemp1 ), f_pbData + ibDataOutCur, DRM_AES_BLOCKLEN );

        Oem_Mont_Aes_DecryptOne( &f_pbData[ibDataOutCur], (void *)&wctx );

        //ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );

        __analysis_assume( ibDataOutCur + DRM_AES_BLOCKLEN < f_cbData );
        DRM_XOR( &( f_pbData[ ibDataOutCur ] ), ( i % 2 == 0 ? rgbTemp1 : rgbTemp2 ), DRM_AES_BLOCKLEN );
    }

    if(0)
    {
        printf("Decrypt f_pbData ");
        for(aa=0; aa< DRM_AES_BLOCKLEN; aa++)
            printf("0x%x ", *(f_pbData+aa) );
        printf("\n");
    }
ErrorExit:
    //DRM_PROFILING_LEAVE_SCOPE;
    return dr;
}

EXIT_PK_NAMESPACE_CODE;

