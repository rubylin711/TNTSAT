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
#include <openssl/aes.h>


#include <stdio.h>
#include <stdlib.h>

//#include <string.h>
//#include <openssl/aes.h>

ENTER_PK_NAMESPACE_CODE;

/*********************************************************************************************
** Function:  Oem_Aes_CtrProcessData
**
** Synopsis:  Does AES Counter-Mode encryption or decryption on a buffer of data
**
** Arguments: [f_pKey]      : The AES secret key used to encrypt or decrypt buffer
**                            - If this API is called inside the TEE. Then OEM_AES_KEY_CONTEXT will
**                              be pointing to an OEM_TEE_KEY_AES.
**                              OEM can convert it to a DRM_AES_KEY using
**                              OEM_TEE_INTERNAL_CONVERT_OEM_TEE_KEY_AES128_TO_DRM_AES_KEY.
**                            - If this API is called from the normal world, then OEM_AES_KEY_CONTEXT
**                              will be pointing to a DRM_AES_KEY. A simple cast to a DRM_AES_KEY*
**                              will be sufficient.
**            [f_pbData]    : The buffer to encrypt or decrypt ( in place )
**            [f_cbData]    : The number of bytes to encrypt or decrypt
**            [f_pCtrContext] : Contains the initialization vector and offset data. Will be updated
**
** Returns:   DRM_SUCCESS
**              Success
**            DRM_E_INVALIDARG
**              One of the pointers was NULL, or the byte count is 0, or neither the
**              block ID or offset are 0 ( one must == 0 ).
**            DRM_E_CRYPTO_FAILED
**              The encrypt/decrypt operation failed
**********************************************************************************************/
DRM_API DRM_RESULT DRM_CALL Oem_Aes_CtrProcessData(
    __in_ecount( 1 )            const OEM_AES_KEY_CONTEXT          *f_pKey,
    __inout_bcount( f_cbData )        DRM_BYTE                     *f_pbData,
    __in                              DRM_DWORD                     f_cbData,
    __inout_ecount( 1 )               DRM_AES_COUNTER_MODE_CONTEXT *f_pCtrContext )
{
#if DRM_BUILD_PROFILE == DRM_BUILD_PROFILE_WP8_1
    CLAW_AUTO_RANDOM_CIPHER
#endif /* DRM_BUILD_PROFILE == DRM_BUILD_PROFILE_WP8_1 */
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_UINT64 qwIVCorrectEndianness;
    DRM_UINT64 rllDataOut[ 2 ];//2  //DRM_AES_BLOCKLEN /sizeof(DRM_UINT64)
    DRM_DWORD  cbDataLeft   = f_cbData;
    DRM_DWORD  ibDataOutCur = 0;
    DRM_DWORD  cbDataToUse  = 0;
    //AES_KEY wctx;
    //int aa = 0;
    //DRM_PROFILING_ENTER_SCOPE( PERF_MOD_DRMAES, PERF_FUNC_Oem_Aes_CtrProcessData );

    /*ChkArg( f_pbData      != NULL );
    ChkArg( f_cbData      >  0 );
    ChkArg( f_pCtrContext != NULL );
    ChkArg( f_pKey        != NULL );
    ChkArg( f_pCtrContext->bByteOffset <= DRM_AES_BLOCKLEN );*/

    //printf("iv2 0x%llx 0x%llx \n", f_pCtrContext->qwInitializationVector, f_pCtrContext->qwBlockOffset);
    qwIVCorrectEndianness = f_pCtrContext->qwInitializationVector;
    //ChkVOID( FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( qwIVCorrectEndianness ) );
    FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( qwIVCorrectEndianness );

    if( f_pCtrContext->bByteOffset > 0 )
    {
        /*
        ** The data is in the middle of a block.  Handle the special case first
        */
        //printf(" f_pCtrContext->bByteOffset %d, data is in the middle of a block!\n", f_pCtrContext->bByteOffset);
        cbDataToUse = (DRM_DWORD)( DRM_AES_BLOCKLEN - f_pCtrContext->bByteOffset );
        cbDataToUse = DRM_MIN( cbDataLeft, cbDataToUse );

        rllDataOut[1] = f_pCtrContext->qwBlockOffset;
        //ChkVOID( FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( rllDataOut[ 1 ] ) );
        FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( rllDataOut[ 1 ] );
        rllDataOut[ 0 ] = qwIVCorrectEndianness;

        dr = Oem_Broker_Aes_EncryptOneBlock( f_pKey, (DRM_BYTE*) rllDataOut );
        //ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );

        DRM_XOR( &( f_pbData[ ibDataOutCur ] ), &( ( (DRM_BYTE*)rllDataOut )[ f_pCtrContext->bByteOffset ] ), cbDataToUse );

        ibDataOutCur += cbDataToUse;

        //ChkDR( DRM_DWordSub( cbDataLeft, cbDataToUse, &cbDataLeft ) );
        DRM_DWordSub( cbDataLeft, cbDataToUse, &cbDataLeft );

        /*
        ** If we used all of the bytes in the current block, then the block offset needs to be increased by one.
        */
        if( f_pCtrContext->bByteOffset + cbDataToUse == DRM_AES_BLOCKLEN )
        {
            /*
            ** Overflow is expected/required for CTR.  The block offset is used as the counter
            ** value, which is allowed to reset to zero.
            */
            f_pCtrContext->qwBlockOffset = DRM_UI64Add( f_pCtrContext->qwBlockOffset, DRM_UI64( 1 ) );
        }
    }



    //printf("cbDataLeft %d\n", cbDataLeft);
    while( cbDataLeft >= DRM_AES_BLOCKLEN )
    {
        rllDataOut[ 1 ] = f_pCtrContext->qwBlockOffset;
        //ChkVOID( FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( rllDataOut[ 1 ] ) );
        FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( rllDataOut[ 1 ] );
        rllDataOut[ 0 ] = qwIVCorrectEndianness;

        //char *tmp_key = (char *)pInternalKey->rgbKey;
        //char *tmp = (char *)rllDataOut;
        //printf("key: ");
        //for(aa =0; aa < 16; aa ++)
        //    printf("0x%x ", *(tmp_key+aa)&0xFF);
        //printf("\n");
        /*printf("cbDataLeft %d\n", cbDataLeft);
        printf("orig: ");
        for(aa =0; aa < 16; aa ++)
            printf("0x%x ", *(tmp+aa)&0xFF);
        printf("\n");*/
        dr = Oem_Broker_Aes_EncryptOneBlock( f_pKey, (DRM_BYTE*)rllDataOut );
        //ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );

//PREFAST_PUSH_DISABLE_EXPLAINED( __WARNING_WRITE_OVERRUN_6386, "Writing to f_pbData cant overrun because ibDataOutCur + DRM_AES_BLOCKLEN is ensured to be less than f_cbData" )
        DRM_XOR( &( f_pbData[ ibDataOutCur ] ), (DRM_BYTE*)rllDataOut, DRM_AES_BLOCKLEN );
//PREFAST_POP /* __WARNING_WRITE_OVERRUN_6386 */

        /*printf("ctr: ");
        for(aa =0; aa < 16; aa ++)
            printf("0x%x ", *(tmp+aa)&0xFF);
        printf("\n");*/
        ibDataOutCur += DRM_AES_BLOCKLEN;
        cbDataLeft   -= DRM_AES_BLOCKLEN;

        /*
        ** Overflow is expected/required for CTR.  The block offset is used as the counter
        ** value, which is allowed to reset to zero.
        */
        f_pCtrContext->qwBlockOffset = DRM_UI64Add( f_pCtrContext->qwBlockOffset, DRM_UI64( 1 ) );

    }

    if( cbDataLeft > 0 ) /* at this point it is strictly less than DRM_AES_BLOCKLEN */
    {
        cbDataToUse = cbDataLeft;

        rllDataOut[ 1 ] = f_pCtrContext->qwBlockOffset;
        //ChkVOID( FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( rllDataOut[ 1 ] ) );
        FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( rllDataOut[ 1 ] );
        rllDataOut[ 0 ] = qwIVCorrectEndianness;

        dr = Oem_Broker_Aes_EncryptOneBlock( f_pKey, (DRM_BYTE*)rllDataOut );
        //ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );

        DRM_XOR( &( f_pbData[ ibDataOutCur ] ), (DRM_BYTE*)rllDataOut, cbDataToUse );
    }

    f_pCtrContext->bByteOffset = ( f_pCtrContext->bByteOffset + f_cbData ) % DRM_AES_BLOCKLEN;


ErrorExit:
    //DRM_PROFILING_LEAVE_SCOPE;
    return dr;
}

DRM_API DRM_RESULT DRM_CALL Oem_Aes_CtrProcessData_Mont(
    __in_ecount( f_keyData )          DRM_BYTE                     *f_pKey,
    __in                              DRM_DWORD                     f_keyData,
    __inout_bcount( f_cbData )        DRM_BYTE                     *f_pbData,
    __in                              DRM_DWORD                     f_cbData,
    __inout_ecount( 1 )               DRM_AES_COUNTER_MODE_CONTEXT *f_pCtrContext )
{
#if DRM_BUILD_PROFILE == DRM_BUILD_PROFILE_WP8_1
    CLAW_AUTO_RANDOM_CIPHER
#endif /* DRM_BUILD_PROFILE == DRM_BUILD_PROFILE_WP8_1 */
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_UINT64 qwIVCorrectEndianness;
    DRM_UINT64 rllDataOut[ 2 ];//2  //DRM_AES_BLOCKLEN /sizeof(DRM_UINT64)
    DRM_DWORD  cbDataLeft   = f_cbData;
    DRM_DWORD  ibDataOutCur = 0;
    DRM_DWORD  cbDataToUse  = 0;
    //AES_KEY wctx;

    /*int aa = 0;
      char *tmp_key = (char *)f_pKey;
        printf("key 2: ");
        for(aa =0; aa < 16; aa ++)
            printf("0x%x ", *(tmp_key+aa)&0xFF);
        printf("\n");*/

    AES_KEY  wctx;
    Oem_Mont_Aes_Setencryptkey(f_pKey, 128, (void *)&wctx);
    //DRM_PROFILING_ENTER_SCOPE( PERF_MOD_DRMAES, PERF_FUNC_Oem_Aes_CtrProcessData );

    /*ChkArg( f_pbData      != NULL );
    ChkArg( f_cbData      >  0 );
    ChkArg( f_pCtrContext != NULL );
    ChkArg( f_pKey        != NULL );
    ChkArg( f_pCtrContext->bByteOffset <= DRM_AES_BLOCKLEN );*/

    //printf("iv2 0x%llx 0x%llx \n", f_pCtrContext->qwInitializationVector, f_pCtrContext->qwBlockOffset);
    qwIVCorrectEndianness = f_pCtrContext->qwInitializationVector;
    //ChkVOID( FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( qwIVCorrectEndianness ) );
    FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( qwIVCorrectEndianness );

    if( f_pCtrContext->bByteOffset > 0 )
    {
        /*
        ** The data is in the middle of a block.  Handle the special case first
        */
        //printf(" f_pCtrContext->bByteOffset %d, data is in the middle of a block!\n", f_pCtrContext->bByteOffset);
        cbDataToUse = (DRM_DWORD)( DRM_AES_BLOCKLEN - f_pCtrContext->bByteOffset );
        cbDataToUse = DRM_MIN( cbDataLeft, cbDataToUse );

        rllDataOut[1] = f_pCtrContext->qwBlockOffset;
        //ChkVOID( FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( rllDataOut[ 1 ] ) );
        FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( rllDataOut[ 1 ] );
        rllDataOut[ 0 ] = qwIVCorrectEndianness;

        Oem_Mont_Aes_EncryptOne((DRM_BYTE*) rllDataOut, (void *)&wctx );
        //dr = Oem_Broker_Aes_EncryptOneBlock( f_pKey, (DRM_BYTE*) rllDataOut );
        //ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );

        DRM_XOR( &( f_pbData[ ibDataOutCur ] ), &( ( (DRM_BYTE*)rllDataOut )[ f_pCtrContext->bByteOffset ] ), cbDataToUse );

        ibDataOutCur += cbDataToUse;

        //ChkDR( DRM_DWordSub( cbDataLeft, cbDataToUse, &cbDataLeft ) );
        DRM_DWordSub( cbDataLeft, cbDataToUse, &cbDataLeft );

        /*
        ** If we used all of the bytes in the current block, then the block offset needs to be increased by one.
        */
        if( f_pCtrContext->bByteOffset + cbDataToUse == DRM_AES_BLOCKLEN )
        {
            /*
            ** Overflow is expected/required for CTR.  The block offset is used as the counter
            ** value, which is allowed to reset to zero.
            */
            f_pCtrContext->qwBlockOffset = DRM_UI64Add( f_pCtrContext->qwBlockOffset, DRM_UI64( 1 ) );
        }
    }



    //printf("cbDataLeft %d\n", cbDataLeft);
    while( cbDataLeft >= DRM_AES_BLOCKLEN )
    {
        rllDataOut[ 1 ] = f_pCtrContext->qwBlockOffset;
        //ChkVOID( FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( rllDataOut[ 1 ] ) );
        FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( rllDataOut[ 1 ] );
        rllDataOut[ 0 ] = qwIVCorrectEndianness;

        //char *tmp_key = (char *)pInternalKey->rgbKey;
        char *tmp = (char *)rllDataOut;
        //printf("key: ");
        //for(aa =0; aa < 16; aa ++)
        //    printf("0x%x ", *(tmp_key+aa)&0xFF);
        //printf("\n");
        /*printf("cbDataLeft %d\n", cbDataLeft);
        printf("orig: ");
        for(aa =0; aa < 16; aa ++)
            printf("0x%x ", *(tmp+aa)&0xFF);
        printf("\n");*/
        Oem_Mont_Aes_EncryptOne((DRM_BYTE*) rllDataOut, (void *)&wctx );
        //dr = Oem_Broker_Aes_EncryptOneBlock( f_pKey, (DRM_BYTE*)rllDataOut );
        //ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );

//PREFAST_PUSH_DISABLE_EXPLAINED( __WARNING_WRITE_OVERRUN_6386, "Writing to f_pbData cant overrun because ibDataOutCur + DRM_AES_BLOCKLEN is ensured to be less than f_cbData" )
        DRM_XOR( &( f_pbData[ ibDataOutCur ] ), (DRM_BYTE*)rllDataOut, DRM_AES_BLOCKLEN );
//PREFAST_POP /* __WARNING_WRITE_OVERRUN_6386 */

        /*printf("ctr: ");
        for(aa =0; aa < 16; aa ++)
            printf("0x%x ", *(tmp+aa)&0xFF);
        printf("\n");*/
        ibDataOutCur += DRM_AES_BLOCKLEN;
        cbDataLeft   -= DRM_AES_BLOCKLEN;

        /*
        ** Overflow is expected/required for CTR.  The block offset is used as the counter
        ** value, which is allowed to reset to zero.
        */
        f_pCtrContext->qwBlockOffset = DRM_UI64Add( f_pCtrContext->qwBlockOffset, DRM_UI64( 1 ) );

    }

    if( cbDataLeft > 0 ) /* at this point it is strictly less than DRM_AES_BLOCKLEN */
    {
        cbDataToUse = cbDataLeft;

        rllDataOut[ 1 ] = f_pCtrContext->qwBlockOffset;
        //ChkVOID( FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( rllDataOut[ 1 ] ) );
        FORMAT_QWORD_AS_BIG_ENDIAN_BYTES_INPLACE( rllDataOut[ 1 ] );
        rllDataOut[ 0 ] = qwIVCorrectEndianness;

        Oem_Mont_Aes_EncryptOne((DRM_BYTE*) rllDataOut, (void *)&wctx );
        //dr = Oem_Broker_Aes_EncryptOneBlock( f_pKey, (DRM_BYTE*)rllDataOut );
        //ChkBOOL( DRM_SUCCEEDED( dr ), DRM_E_CRYPTO_FAILED );

        DRM_XOR( &( f_pbData[ ibDataOutCur ] ), (DRM_BYTE*)rllDataOut, cbDataToUse );
    }

    f_pCtrContext->bByteOffset = ( f_pCtrContext->bByteOffset + f_cbData ) % DRM_AES_BLOCKLEN;


ErrorExit:
    //DRM_PROFILING_LEAVE_SCOPE;
    return dr;
}

EXIT_PK_NAMESPACE_CODE;

