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

extern void eccdump(const char *tag, mt_u8 *buffer, mt_u32 len);

#define ECC_ECDSA_DEBUG 0

#if ECC_ECDSA_DEBUG
#define dump(str, buf, len) {\
	eccdump(str, buf, len); }
#else
#define dump(str, buf, len) {;}
#endif

static int bn_is_zero(mt_u8 *data, mt_u32 size)
{
    mt_u32 i;

    for (i = 0; i < size; i++) {
        if (0 != data[i])
            return 0;
    }

    return 1;
}

static mt_u32 hash_digest_size(mt_u32 algo)
{
	switch (algo) {
		case MT_CIPHER_HASH_TYPE_SHA1:
			return 20;
		case MT_CIPHER_HASH_TYPE_SHA256:
			return 32;
		default:
			break;
	}

	return 0;
}

/*
ecdsa sign:
1. e = H(M),    Hash: SHA256
2. generate random k (0 < k < n)
3. P1=k * G= (x1, y1)
4. r=x1 mod n, if r=0 or, goto 2.
5. s=k^(-1)*(e +d*r) mod n, if s=0, then goto 2.
6. M signature: (r, s)
*/
int mt_unf_ecc_ecdsa_sign(MT_CIPHER_EC_PARAMS_S *xParams,
    MT_CIPHER_HASH_TYPE_E xHashType, mt_u8 *priKey,
    mt_u8 *msg, mt_u32 mlen, mt_u8 *r, mt_u8 *s)
{
    mt_handle handle_hash = MT_NULL;
    mt_handle handle_ecp = MT_NULL;
    mt_handle handle_mod = MT_NULL;
    mt_u32 keysize;
    mt_u32 digestsize;
    MT_CIPHER_EC_POINT_S P1;
    mt_u8 k[32];
    mt_u8 e[32];
    mt_u8 x[32] = {0};
    mt_u8 y[32] = {0};
    mt_u8 k_1[32];
    int ret;

    if (!xParams || !priKey || !msg || !r || !s)
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    if (!xParams->n)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    keysize = xParams->keySize;
    if (keysize > 32)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    digestsize = hash_digest_size(xHashType);

    P1.X = x;
    P1.Y = y;

    /* 1. e = H(M) */
    memset(e, 0, keysize);
    ret = mt_unf_cipher_hash_create(xHashType, NULL, &handle_hash);
    ret |= mt_unf_cipher_hash_update(handle_hash, msg, mlen);
    ret |= mt_unf_cipher_hash_final(handle_hash, e + keysize - digestsize);
    if (ret != MT_SUCCESS)
        return ret;

    ret = mt_unf_cipher_ecp_create(&handle_ecp);
    if (ret != MT_SUCCESS)
        goto EXIT;

    ret = mt_unf_cipher_bn_create(&handle_mod);
    if (ret != MT_SUCCESS)
        goto EXIT;

    while (1) {
        /* 2. generate random k (0 < k < n) */
        ret = mt_unf_cipher_get_random_number(xParams->keySize, k);
        if (ret != MT_SUCCESS)
            goto EXIT;

        /* check k:  0 < k < n */
        if (bn_is_zero(k, keysize) || mt_bn_ucmp(k, keysize, xParams->n, keysize) >= 0)
            continue;
        //dump("k", k, keysize);

        /* 3. P1 = k * G */
        ret |= mt_unf_cipher_ecp_mul(handle_ecp, *xParams, &P1, k, NULL, NULL);
        if (ret != MT_SUCCESS)
            goto EXIT;
        //dump("x1", P1.X, keysize);
        //dump("y1", P1.Y, keysize);

        /* 4. r=x1 mod n */
        ret = mt_unf_cipher_bn_mod_mod(handle_mod, r, P1.X, xParams->n, keysize, keysize);
        if (ret != MT_SUCCESS)
            goto EXIT;
        /* check r != 0 */
        if (bn_is_zero(r, keysize))
            continue;
        //dump("r", r, keysize);

        /* 5. s=k^(-1)*(e +d*r) mod n */
        /* k_1 = k^(-1) */
        ret = mt_unf_cipher_bn_mod_inv(handle_mod, k_1, k, xParams->n, keysize, keysize);
        if (ret != MT_SUCCESS)
            goto EXIT;
        //dump("k^-1", k_1, keysize);
        /* t=(e +d*r) mod n */
        ret = mt_unf_cipher_bn_mod_mul(handle_mod, s, priKey, r, xParams->n, keysize, keysize, keysize);
        if (ret != MT_SUCCESS)
            goto EXIT;
        ret = mt_unf_cipher_bn_mod_add(handle_mod, s, e, s, xParams->n, keysize, keysize, keysize);
        if (ret != MT_SUCCESS)
            goto EXIT;
        /* s=k^(-1)*(e +d*r) mod n */
        ret = mt_unf_cipher_bn_mod_mul(handle_mod, s, k_1, s, xParams->n, keysize, keysize, keysize);
        if (ret != MT_SUCCESS)
            goto EXIT;

        if (bn_is_zero(s, 32))
            continue;
        //dump("s", s, 32);

        break;
    }

EXIT:
    if (handle_mod)
        mt_unf_cipher_bn_destroy(handle_mod);

    if (handle_ecp)
        mt_unf_cipher_ecp_destroy(handle_ecp);

    return ret;
}

/*
ecdsa verify:
1. check 0 < r < n, otherwise verify fail
2. check 0 < s < n, otherwise verify fail
3. e= H(M)
4. w = s^-1 mod n
5. u1 = e*w, u2 = r*w
6. (x1,y1)= u1*G + u2*PA
7. if x1=r, verify success; otherwise fail
*/
int mt_unf_ecc_ecdsa_verify(MT_CIPHER_EC_PARAMS_S *xParams,
    MT_CIPHER_HASH_TYPE_E xHashType, mt_u8 *pubKeyX, mt_u8 *pubKeyY,
    mt_u8 *msg, mt_u32 mlen, mt_u8 *r, mt_u8 *s)
{
    mt_handle handle_hash = MT_NULL;
    mt_handle handle_ecp = MT_NULL;
    mt_handle handle_mod = MT_NULL;
    MT_CIPHER_EC_POINT_S key;
    MT_CIPHER_EC_POINT_S P1, P2;
    mt_u32 keysize;
    mt_u32 digestsize;
    mt_u8 e[32];
    mt_u8 w[32];
    mt_u8 x[32] = {0};
    mt_u8 y[32] = {0};
    mt_u8 x2[32] = {0};
    mt_u8 y2[32] = {0};
    mt_u8 u1[32];
    mt_u8 u2[32];
    int ret = 0;

    if (!xParams || !pubKeyX || !pubKeyY || !msg || !r | !s)
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    if (!xParams->n)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    P1.X = x;
    P1.Y = y;
    P2.X = x2;
    P2.Y = y2;

    keysize = xParams->keySize;
    if (keysize > 32)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    key.X = pubKeyX;
    key.Y = pubKeyY;

    digestsize = hash_digest_size(xHashType);

    /* 1. check r: 1 <= r <= n-1 */
    if (bn_is_zero(r, keysize) || mt_bn_ucmp(r, keysize, xParams->n, keysize) >= 0)
        return -1;

    /* 2. check s: 1 <= s <= n-1 */
    if (bn_is_zero(s, keysize) || mt_bn_ucmp(s, keysize, xParams->n, keysize) >= 0)
        return -1;

    /* 3. e = H(M) */
    memset(e, 0, keysize);
    ret = mt_unf_cipher_hash_create(xHashType, NULL, &handle_hash);
    ret |= mt_unf_cipher_hash_update(handle_hash, msg, mlen);
    ret |= mt_unf_cipher_hash_final(handle_hash, e + keysize - digestsize);

    if (ret != MT_SUCCESS)
        return ret;
    //dump("e", e, keysize);

    ret = mt_unf_cipher_bn_create(&handle_mod);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* 4. w = s^-1 mod n */
    ret = mt_unf_cipher_bn_mod_inv(handle_mod, w, s, xParams->n, keysize, keysize);
    if (ret != MT_SUCCESS)
        goto EXIT;
    //dump("w", w, keysize);

    /* 5. u1 = e*w mod n, u2 = r*w mod n */
    ret = mt_unf_cipher_bn_mod_mul(handle_mod, u1, e, w, xParams->n, keysize, keysize, keysize);
    if (ret != MT_SUCCESS)
        goto EXIT;
    ret = mt_unf_cipher_bn_mod_mul(handle_mod, u2, r, w, xParams->n, keysize, keysize, keysize);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* 6. (x1,y1)= u1*G + u2*PA */
    ret = mt_unf_cipher_ecp_create(&handle_ecp);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* P1 = u1 * G */
    ret |= mt_unf_cipher_ecp_mul(handle_ecp, *xParams, &P1, u1, NULL, NULL);
    if (ret != MT_SUCCESS)
        goto EXIT;
    //dump("x1", P1.X, keysize);
    //dump("y1", P1.Y, keysize);

    /* P2 = u2 * pubKey */
    ret |= mt_unf_cipher_ecp_mul(handle_ecp, *xParams, &P2, NULL, &key, u2);
    if (ret != MT_SUCCESS)
        goto EXIT;
    //dump("x2", P2.X, keysize);
    //dump("y2", P2.Y, keysize);

    /* (x1,y1)= u1*G + u2*PA */
    ret |= mt_unf_cipher_ecp_add(handle_ecp, *xParams, &P1, &P1, &P2);
    if (ret != MT_SUCCESS)
        goto EXIT;
    //dump("x", P1.X, keysize);
    //dump("y", P1.Y, keysize);

    /* check x1 == r */
    if (mt_bn_ucmp(P1.X, keysize, r, keysize) != 0)
        ret = MT_CIPHER_ERR_ECC_VERIFY_FAILED;

EXIT:
    if (handle_mod)
        mt_unf_cipher_bn_destroy(handle_mod);

    if (handle_ecp)
        mt_unf_cipher_ecp_destroy(handle_ecp);

    return ret;
}

