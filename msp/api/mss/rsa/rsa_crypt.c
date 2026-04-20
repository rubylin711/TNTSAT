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
#include "rsa_internel.h"

int rsa_crypto_public(struct rsa_public_key *key,
        unsigned char *src, unsigned int src_len, unsigned char *dst)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle p_cipher;
    MT_CIPHER_RSA_CTRL_S info;
    memset(&info, 0, sizeof(MT_CIPHER_RSA_CTRL_S));

    info.operation = MT_CIPHER_OPERATION_ENCRYPT;
    info.rsa_para.p_e = key->e;
    info.rsa_para.p_m = key->n;
    info.rsa_para.key_length = key->n_length;
    info.rsa_para.exp_length = key->e_length;

    ret = mt_unf_cipher_rsa_create(&p_cipher);
    ret |= mt_unf_cipher_rsa_config(p_cipher, &info);
    ret |= mt_unf_cipher_rsa_process(p_cipher, src, dst, src_len);
    mt_unf_cipher_rsa_destroy(p_cipher);

    if (ret != MT_SUCCESS) {
        if (ret == MT_CIPHER_ERR_FEATURE_NOT_SUPPORT)
            printf("[RSA Feature Not Support!!!, len=%d]\n", src_len);
        return ret;
    }

    return ret;
}

int rsa_crypto_private(struct rsa_keypair *key,
        unsigned char *src, unsigned int src_len, unsigned char *dst)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle p_cipher;
    MT_CIPHER_RSA_CTRL_S info;

    memset(&info, 0, sizeof(MT_CIPHER_RSA_CTRL_S));

    info.operation = MT_CIPHER_OPERATION_DECRYPT;
    info.rsa_para.p_e = key->d;
    info.rsa_para.p_m = key->n;
    info.rsa_para.key_length = key->n_length;
    info.rsa_para.exp_length = key->d_length;

    ret = mt_unf_cipher_rsa_create(&p_cipher);
    ret |= mt_unf_cipher_rsa_config(p_cipher, &info);
    ret |= mt_unf_cipher_rsa_process(p_cipher, src, dst, src_len);
    mt_unf_cipher_rsa_destroy(p_cipher);

    if (ret != MT_SUCCESS) {
        if (ret == MT_CIPHER_ERR_FEATURE_NOT_SUPPORT)
            printf("[RSA Feature Not Support!!!, len=%d]\n", src_len);
        return ret;
    }

    return ret;
}

int rsa_crypto_private_crt(struct rsa_keypair *key,
        unsigned char *src, unsigned int src_len, unsigned char *dst)
{
    mt_handle bn_handle = 0;
    int ret = 0;
    unsigned char tmpbuf1[256];
    unsigned char tmpbuf2[256];
    unsigned char tmpbuf3[256];
    unsigned char *m1;
    unsigned char *m2;
    unsigned char *m1_m2;
    unsigned char *h;
    unsigned char *hq;
    unsigned char *m;

    if (NULL == key  || NULL == key->n)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (NULL == key->p  || NULL == key->q || NULL == key->qInv || NULL == key->dp || NULL == key->dq)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (NULL == src || NULL == dst)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (mt_unf_cipher_bn_create(&bn_handle))
        return MT_CIPHER_ERR_BN_CREATE;

    /* m1 =  src exp(dP) mod P */
    m1 = tmpbuf1;
    if (mt_unf_cipher_bn_mod_exp(bn_handle, m1,
        src, key->dp, key->p, src_len, key->dp_length, key->p_length))
    {
        ret = MT_CIPHER_ERR_BN_MODEXP;
        goto EXIT;
    }

    /* m2 =  src exp(dQ) mod Q */
    m2 = tmpbuf2;
    if (mt_unf_cipher_bn_mod_exp(bn_handle, m2,
        src, key->dq, key->q, src_len, key->dq_length, key->q_length))
    {
        ret = MT_CIPHER_ERR_BN_MODEXP;
        goto EXIT;
    }

    /* h =  (m1 - m2) * qInv mod P */
    m1_m2 = tmpbuf3;
    if (mt_unf_cipher_bn_mod_sub(bn_handle, m1_m2,
        m1, m2, key->p, key->p_length, key->p_length, key->p_length))
    {
        ret = MT_CIPHER_ERR_BN_MODSUB;
        goto EXIT;
    }
    h = tmpbuf1;
    if (mt_unf_cipher_bn_mod_mul(bn_handle, h,
        key->qInv, m1_m2, key->p, key->p_length, key->p_length, key->p_length))
    {
        ret = MT_CIPHER_ERR_BN_MODMUL;
        goto EXIT;
    }

    /* m = (m2 + h*Q) mod pxN */
    hq = tmpbuf3;
    if (mt_unf_cipher_bn_mod_mul(bn_handle, hq,
        h, key->q, key->n, key->p_length, key->q_length, key->n_length))
    {
        ret = MT_CIPHER_ERR_BN_MODMUL;
        goto EXIT;
    }
    m = dst;
    if (mt_unf_cipher_bn_mod_add(bn_handle, m,
        m2, hq, key->n, key->q_length, key->n_length, key->n_length))
    {
        ret = MT_CIPHER_ERR_BN_MODADD;
        goto EXIT;
    }

EXIT:
    mt_unf_cipher_bn_destroy(bn_handle);

    return ret;
}

