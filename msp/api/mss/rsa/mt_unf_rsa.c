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
#include "mt_unf_rsa.h"
#include "pkcs1_internel.h"
#include "rsa_internel.h"

#ifdef MT_RSA_ENCDEC_ON
int mt_unf_rsa_public_encrypt(struct rsa_public_key *key,
        unsigned char *src, unsigned int src_len, unsigned char *dst,
        MT_RSA_ALG algo)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 buf[64];
    mt_u8 *input = (mt_u8 *)buf;
    mt_u32 input_len;

    if (NULL == key || NULL == src || NULL == dst)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (NULL == key->e || NULL == key->n)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (src_len > key->n_length)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    input_len = key->n_length;

    if (algo == MT_RSAES_PKCS1_OAEP_MGF1_SHA1)
        ret = PKCS1_OAEP_mgf1_encode(input, input_len, src, src_len,
                    MT_CIPHER_HASH_TYPE_SHA1);
    else if (algo == MT_RSAES_PKCS1_OAEP_MGF1_SHA256)
        ret = PKCS1_OAEP_mgf1_encode(input, input_len, src, src_len,
                    MT_CIPHER_HASH_TYPE_SHA256);
    else if (algo == MT_RSAES_PKCS1_V1_5)
         ret = PKCS1_V1_5_encode(input, input_len, src, src_len,
                    MT_RSA_CRYPT);
    else if (algo == MT_RSA_NOPAD)
    {
        input = (mt_u8 *)src;
        input_len = src_len;
    }
    else
    {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (ret != MT_SUCCESS)
        return ret;

    ret = rsa_crypto_public(key, input, input_len, dst);

    return ret;
}

int mt_unf_rsa_private_decrypt(struct rsa_keypair *key,
        unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len,
        MT_RSA_ALG algo)
{
    mt_s32 ret = MT_SUCCESS;

    if (NULL == key || NULL == src || NULL == dst || NULL == dst_len)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (NULL == key->n)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (NULL != key->d)
    {
         ret = rsa_crypto_private(key, src, src_len, dst);
    }
    else
    {
        ret = rsa_crypto_private_crt(key, src, src_len, dst);
    }

    if (ret != MT_SUCCESS)
        return ret;

    if (algo == MT_RSAES_PKCS1_OAEP_MGF1_SHA1)
        ret = PKCS1_OAEP_mgf1_decode(dst, dst_len, dst, src_len,
                    key->n_length, MT_CIPHER_HASH_TYPE_SHA1);
    else if (algo == MT_RSAES_PKCS1_OAEP_MGF1_SHA256)
        ret = PKCS1_OAEP_mgf1_decode(dst, dst_len, dst, src_len,
                    key->n_length, MT_CIPHER_HASH_TYPE_SHA256);
    else if (algo == MT_RSAES_PKCS1_V1_5)
        ret = PKCS1_V1_5_decode(dst, dst_len, dst, src_len,
                    MT_RSA_CRYPT);
    else if (algo == MT_RSA_NOPAD)
    {
        *dst_len = key->n_length;
    }
    else
    {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    return ret;
}

int mt_unf_rsa_public_decrypt(struct rsa_public_key *key,
        unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len,
        MT_RSA_ALG algo)
{
    mt_s32 ret = MT_SUCCESS;

    if (NULL == key || NULL == src || NULL == dst || NULL == dst_len)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (NULL == key->e || NULL == key->n)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    ret = rsa_crypto_public(key, src, src_len, dst);
    if (ret != MT_SUCCESS)
        return ret;

    if (algo == MT_RSAES_PKCS1_OAEP_MGF1_SHA1)
        ret = PKCS1_OAEP_mgf1_decode(dst, dst_len, dst, src_len,
                    key->n_length, MT_CIPHER_HASH_TYPE_SHA1);
    else if (algo == MT_RSAES_PKCS1_OAEP_MGF1_SHA256)
        ret = PKCS1_OAEP_mgf1_decode(dst, dst_len, dst, src_len,
                    key->n_length, MT_CIPHER_HASH_TYPE_SHA256);
    else if (algo == MT_RSAES_PKCS1_V1_5)
        ret = PKCS1_V1_5_decode(dst, dst_len, dst, src_len,
                    MT_RSA_CRYPT);
    else if (algo == MT_RSA_NOPAD)
    {
        *dst_len = key->n_length;
    }
    else
    {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    return ret;
}

int mt_unf_rsa_private_encrypt(struct rsa_keypair *key,
        mt_u8*src, mt_u32 src_len, mt_u8 *dst,
        MT_RSA_ALG algo)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 buf[64];
    mt_u8 *input = (mt_u8 *)buf;
    mt_u32 input_len;

    if (NULL == key || NULL == src || NULL == dst )
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (NULL == key->n)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (src_len > key->n_length)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

     input_len = key->n_length;

    if (algo == MT_RSAES_PKCS1_OAEP_MGF1_SHA1)
        ret = PKCS1_OAEP_mgf1_encode(input, input_len, src, src_len,
                    MT_CIPHER_HASH_TYPE_SHA1);
    else if (algo == MT_RSAES_PKCS1_OAEP_MGF1_SHA256)
        ret = PKCS1_OAEP_mgf1_encode(input, input_len, src, src_len,
                    MT_CIPHER_HASH_TYPE_SHA256);
    else if (algo == MT_RSAES_PKCS1_V1_5)
         ret = PKCS1_V1_5_encode(input, input_len, src, src_len,
                    MT_RSA_CRYPT);
    else if (algo == MT_RSA_NOPAD)
    {
        input = (mt_u8 *)src;
        input_len = src_len;
    }
    else
    {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (ret != MT_SUCCESS)
        return ret;

    if (NULL != key->d)
    {
        ret = rsa_crypto_private(key, input, input_len, dst);
    }
    else
    {
        ret = rsa_crypto_private_crt(key, input, input_len, dst);
    }

    return ret;
}
#endif

#ifdef MT_RSA_SIGNVERIFY_ON
int mt_unf_rsa_sign(struct rsa_keypair *key, MT_RSA_ALG algo,
	unsigned char *msg, unsigned int mlen, unsigned char *sign, unsigned int *slen)
{
    int ret;
    mt_u32 buf[64];
    mt_u8 *signaure;

    signaure = (mt_u8 *)buf;

    if (NULL == key || NULL == msg || NULL == sign)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (NULL == key->n)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (algo == MT_RSASSA_PKCS1_PSS_MGF1_SHA1)
        ret = PKCS1_PSS_sign(msg, mlen, key->n_length*8-1, signaure, key->n_length,
                        MT_CIPHER_HASH_TYPE_SHA1);
    else if (algo == MT_RSASSA_PKCS1_PSS_MGF1_SHA256)
        ret = PKCS1_PSS_sign(msg, mlen, key->n_length*8-1, signaure, key->n_length,
                        MT_CIPHER_HASH_TYPE_SHA256);
    else if (algo == MT_RSASSA_PKCS1_V1_5_SHA1)
        ret = PKCS1_V1_5_sign(msg, mlen, signaure, key->n_length,
                        MT_CIPHER_HASH_TYPE_SHA1);
    else if (algo == MT_RSASSA_PKCS1_V1_5_SHA256)
        ret = PKCS1_V1_5_sign(msg, mlen, signaure, key->n_length,
                        MT_CIPHER_HASH_TYPE_SHA256);
    else
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (ret != MT_SUCCESS)
        return ret;

    if (NULL != key->d)
    {
        ret = rsa_crypto_private(key, signaure, key->n_length, sign);
    }
    else
    {
        ret = rsa_crypto_private_crt(key, signaure, key->n_length, sign);
    }

    *slen = key->n_length;

    return ret;
}

int mt_unf_rsa_verify(struct rsa_public_key *key, MT_RSA_ALG algo,
	unsigned char *msg, unsigned int mlen, unsigned char *sign, unsigned int slen)
{
    int ret;
    mt_u32 buf[64];
    mt_u8 *signaure;

    signaure = (mt_u8 *)buf;

    if (NULL == key || NULL == msg || NULL == sign)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (NULL == key->e || NULL == key->n)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (slen != key->n_length)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    ret = rsa_crypto_public(key, sign, slen, signaure);

    if (ret != MT_SUCCESS)
        return ret;

    if (algo == MT_RSASSA_PKCS1_PSS_MGF1_SHA1)
        ret = PKCS1_PSS_verify(msg, mlen, key->n_length*8-1, signaure, slen,
                        MT_CIPHER_HASH_TYPE_SHA1);
    else if (algo == MT_RSASSA_PKCS1_PSS_MGF1_SHA256)
        ret = PKCS1_PSS_verify(msg, mlen, key->n_length*8-1, signaure, slen,
                        MT_CIPHER_HASH_TYPE_SHA256);
    else if (algo == MT_RSASSA_PKCS1_V1_5_SHA1)
        ret = PKCS1_V1_5_verify(msg, mlen, signaure, slen,
                        MT_CIPHER_HASH_TYPE_SHA1);
    else if (algo == MT_RSASSA_PKCS1_V1_5_SHA256)
        ret = PKCS1_V1_5_verify(msg, mlen, signaure, slen,
                        MT_CIPHER_HASH_TYPE_SHA256);
    else
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    return ret;
}
#endif

static int _MinusOne(const unsigned char * input, unsigned char * result, unsigned int size)
{
    unsigned int i;

    if (!input || !result || 0 == size) {
        return -1;
    }

    memcpy(result, input, size);

    for (i = size - 1; i >= 0; i--) {
        if (result[i]) {
            result[i] -= 1;
            break;
        } else {
            result[i] = 0xff;
            continue;
        }
    }

    if (i < 0) {
        memset(result, 0xFF, size);
    }

    return 0;
}

/*
DP = E**(-1) mod (P-1)
DQ = E**(-1) mod (Q-1)
QInv = Q**(-1) mod P
*/
int mt_unf_rsa_gen_crt_params(unsigned int keysize, unsigned int E,
    const unsigned char *P, const unsigned char *Q,
    unsigned char *DP, unsigned char *DQ, unsigned char *QInv)
{
    int ret;
    mt_handle bn_handle;
    mt_u32 length = keysize / 2;
    mt_u8 e[4];
    mt_u8 P_1[128];
    mt_u8 Q_1[128];

    if (!P || !Q || !DP || !DQ || !QInv)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (keysize > 256 || keysize < 64)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    memset(e, 0, 4);
    if (E == 3) {
        e[3] = 0x3;
    } else if (E == 17) {
        e[3] = 0x11;
    } else if (E == 65537) {
        e[3] = 0x1;
        e[1] = 0x1;
    } else {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    ret = _MinusOne(P, P_1, length);
    ret = _MinusOne(Q, Q_1, length);

    ret = mt_unf_cipher_bn_create(&bn_handle);
    if (ret != MT_SUCCESS)
        return MT_CIPHER_ERR_BN_CREATE;

    /* DP = e**(-1) mod (P-1) */
    ret = mt_unf_cipher_bn_mod_inv(bn_handle, DP, e, P_1, 4, length);
    if (ret != MT_SUCCESS) {
        goto EXIT;
    }

    /* DQ = e**(-1) mod (Q-1) */
    ret = mt_unf_cipher_bn_mod_inv(bn_handle, DQ, e, P_1, 4, length);
    if (ret != MT_SUCCESS) {
        goto EXIT;
    }

    /* QInv = Q**(-1) mod P */
    ret = mt_unf_cipher_bn_mod_inv(bn_handle, QInv, Q, P, length, length);
    if (ret != MT_SUCCESS) {
        goto EXIT;
    }

EXIT:
    mt_unf_cipher_bn_destroy(bn_handle);
    return ret;
}

