/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mt_common.h"
#include "mt_unf_cipher_v2.h"
#include "mt_mpi_bn.h"
#include "mt_unf_ecc.h"

void eccdump(const char *tag, mt_u8 *buffer, mt_u32 len);

#define ECC_ECDH_DEBUG 0

#if ECC_ECDH_DEBUG
#define dump(str, buf, len) {\
	eccdump(str, buf, len); }
#else
#define dump(str, buf, len) {;}
#endif

void eccdump(const char *tag, mt_u8 *buffer, mt_u32 len)
{
    mt_u32 i = 0;
    printf("\n%s:[%d]\n", tag, len);
    for(i = 0; i < len; i++)
    {
        if((i%16) == 0 && i != 0)
            printf("\n");
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}

/* y^2 = x^3 +ax + b  */
int mt_unf_ecc_point_check(MT_CIPHER_EC_PARAMS_S *xParams, mt_u8 *px, mt_u8 *py)
{
    int ret = MT_FAILURE;
    mt_handle handle_mod = MT_INVALID_HANDLE;
    mt_u8 left[32];
    mt_u8 right[32];
    mt_u8 t[32];
    mt_u8 e;

    if (!xParams || !px || !py)
        return MT_CIPHER_ERR_BAD_PARAMETERS;


    dump("check x", px, 32);
    dump("check y", py, 32);

    ret = mt_unf_cipher_bn_create(&handle_mod);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* left = (y * y) mod n */
    ret = mt_unf_cipher_bn_mod_mul(handle_mod, left, py, py, xParams->q, 32, 32, 32);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* t =  x^3 mod n */
    e = 3;
    ret = mt_unf_cipher_bn_mod_exp(handle_mod, t, px, &e, xParams->q, 32, 1, 32);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* r = (a * x) mod n */
    ret = mt_unf_cipher_bn_mod_mul(handle_mod, right, xParams->a, px, xParams->q, 32, 32, 32);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* r =  (x^3 + ax) mod n */
    ret = mt_unf_cipher_bn_mod_add(handle_mod, right, right, t, xParams->q, 32, 32, 32);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* r =  (x^3 + ax + b) mod n */
    ret = mt_unf_cipher_bn_mod_add(handle_mod, right, right, xParams->b, xParams->q, 32, 32, 32);
    if (ret != MT_SUCCESS)
        goto EXIT;

    dump("left", left, 32);
    dump("right", right, 32);

    ret = MT_FAILURE;
    if (mt_bn_ucmp(left, 32, right, 32) == 0) {
        ret = MT_SUCCESS;
    }

EXIT:
    if (handle_mod != MT_INVALID_HANDLE)
        mt_unf_cipher_bn_destroy(handle_mod);
    return ret;
}

int mt_unf_ecc_ecdh_gen_keypair(MT_CIPHER_EC_PARAMS_S *xParams,
    mt_u8 *priKey, mt_u8 *pubKeyX, mt_u8 *pubKeyY)
{
    int ret = MT_FAILURE;
    mt_handle p_handle;
    MT_CIPHER_EC_POINT_S pubKeyP;

    if (!priKey || !pubKeyX || !pubKeyY  || !xParams)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    pubKeyP.X = pubKeyX;
    pubKeyP.Y = pubKeyY;

    /* generate private key d */
    ret = mt_unf_cipher_get_random_number(xParams->keySize, priKey);
    if (ret != MT_SUCCESS)
        return ret;

    if (xParams->keySize == 21)
        priKey[0] = 0;

    /* generate public key P = d * G */
    ret = mt_unf_cipher_ecp_create(&p_handle);
    if (ret != MT_SUCCESS)
        return ret;

    ret = mt_unf_cipher_ecp_mul(p_handle, *xParams, &pubKeyP, priKey, NULL, NULL);
    if (ret != MT_SUCCESS) {
        mt_unf_cipher_ecp_destroy(p_handle);
        return ret;
    }

    mt_unf_cipher_ecp_destroy(p_handle);

    return ret;
}

int mt_unf_ecc_ecdh_gen_pubkey(MT_CIPHER_EC_PARAMS_S *xParams,
    mt_u8 *priKey, mt_u8 *pubKeyX, mt_u8 *pubKeyY)
{
    int ret = MT_FAILURE;
    mt_handle p_handle;
    MT_CIPHER_EC_POINT_S pubKeyP;

    if (!priKey || !pubKeyX || !pubKeyY || !xParams)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    pubKeyP.X = pubKeyX;
    pubKeyP.Y = pubKeyY;

    /* generate public key P = d * G */
    ret = mt_unf_cipher_ecp_create(&p_handle);
    if (ret != MT_SUCCESS)
        return ret;

    ret = mt_unf_cipher_ecp_mul(p_handle, *xParams, &pubKeyP, priKey, NULL, NULL);
    if (ret != MT_SUCCESS) {
        mt_unf_cipher_ecp_destroy(p_handle);
        return ret;
    }

    mt_unf_cipher_ecp_destroy(p_handle);

    return ret;
}

int mt_unf_ecc_ecdh_gen_sharekey(MT_CIPHER_EC_PARAMS_S *xParams,
    mt_u8 *priKeyA, mt_u8 *pubKeyBX, mt_u8 *pubKeyBY,
    mt_u8 *shareKeyX, mt_u8 *shareKeyY)
{
    int ret = MT_FAILURE;
    mt_handle p_handle;
    MT_CIPHER_EC_POINT_S pubKeyPb;
    MT_CIPHER_EC_POINT_S shareKeyP;

    if (!xParams || !priKeyA || !pubKeyBX || !pubKeyBY)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (!shareKeyX || !shareKeyY)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    pubKeyPb.X = pubKeyBX;
    pubKeyPb.Y = pubKeyBY;

    shareKeyP.X = shareKeyX;
    shareKeyP.Y = shareKeyY;

    /* generate shareKey P = priKeyA * pubKeyB */
    ret = mt_unf_cipher_ecp_create(&p_handle);
    if (ret != MT_SUCCESS)
        return ret;

    ret = mt_unf_cipher_ecp_mul(p_handle, *xParams, &shareKeyP, NULL, &pubKeyPb, priKeyA);
    if (ret != MT_SUCCESS) {
        mt_unf_cipher_ecp_destroy(p_handle);
        return ret;
    }

    mt_unf_cipher_ecp_destroy(p_handle);

    return ret;
}

