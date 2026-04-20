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

#define ECC_SM2_DEBUG 0

#if ECC_SM2_DEBUG
#define dump(str, buf, len) {\
	eccdump(str, buf, len); }
#else
#define dump(str, buf, len) {;}
#endif

typedef struct _EC_PARAMS_S {
    mt_u8 p[32];
    mt_u8 a[32];
    mt_u8 b[32];
    mt_u8 n[32];
    mt_u8 h[32];
    mt_u8 gx[32];
    mt_u8 gy[32];
}MT_ECC_PARAMS_S;

static int bn_is_zero(mt_u8 *data, mt_u32 size)
{
    mt_u32 i;

    for (i = 0; i < size; i++) {
        if (0 != data[i])
            return 0;
    }

    return 1;
}

static MT_ECC_PARAMS_S ecc_sm2p256v1 = {
    .p = {
        0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
    .a = {
        0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC},
    .b = {
        0x28,0xE9,0xFA,0x9E,0x9D,0x9F,0x5E,0x34,0x4D,0x5A,0x9E,0x4B,0xCF,0x65,0x09,0xA7,
        0xF3,0x97,0x89,0xF5,0x15,0xAB,0x8F,0x92,0xDD,0xBC,0xBD,0x41,0x4D,0x94,0x0E,0x93},
    .n = {
        0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0x72,0x03,0xDF,0x6B,0x21,0xC6,0x05,0x2B,0x53,0xBB,0xF4,0x09,0x39,0xD5,0x41,0x23},
    .h = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
    .gx = {
        0x32,0xC4,0xAE,0x2C,0x1F,0x19,0x81,0x19,0x5F,0x99,0x04,0x46,0x6A,0x39,0xC9,0x94,
        0x8F,0xE3,0x0B,0xBF,0xF2,0x66,0x0B,0xE1,0x71,0x5A,0x45,0x89,0x33,0x4C,0x74,0xC7},
    .gy = {
        0xBC,0x37,0x36,0xA2,0xF4,0xF6,0x77,0x9C,0x59,0xBD,0xCE,0xE3,0x6B,0x69,0x21,0x53,
        0xD0,0xA9,0x87,0x7C,0xC6,0x2A,0x47,0x40,0x02,0xDF,0x32,0xE5,0x21,0x39,0xF0,0xA0},
};

static mt_u8 ENTL[2] = {0x00,0x80};
static MT_CIPHER_HASH_TYPE_E sm2_hash_type = MT_CIPHER_HASH_TYPE_SM3;

static int sm2_e(mt_u8 *id, MT_CIPHER_EC_POINT_S *pubkey,
    mt_u8 *msg, mt_u32 mlen, mt_u8 *e)
{
    mt_handle p_hash;
    mt_u8 Z[32];
    int ret;

    ret = mt_unf_cipher_hash_create(sm2_hash_type, NULL, &p_hash);
    ret |= mt_unf_cipher_hash_update(p_hash, ENTL, 2);
    ret |= mt_unf_cipher_hash_update(p_hash, id, 16);
    ret |= mt_unf_cipher_hash_update(p_hash, ecc_sm2p256v1.a, 32);
    ret |= mt_unf_cipher_hash_update(p_hash, ecc_sm2p256v1.b, 32);
    ret |= mt_unf_cipher_hash_update(p_hash, ecc_sm2p256v1.gx, 32);
    ret |= mt_unf_cipher_hash_update(p_hash, ecc_sm2p256v1.gy, 32);
    ret |= mt_unf_cipher_hash_update(p_hash, pubkey->X, 32);
    ret |= mt_unf_cipher_hash_update(p_hash, pubkey->Y, 32);
    ret |= mt_unf_cipher_hash_final(p_hash, Z);
    if (ret != MT_SUCCESS)
        return ret;

    //dump("Z", Z, 32);

    ret = mt_unf_cipher_hash_create(sm2_hash_type, NULL, &p_hash);
    ret |= mt_unf_cipher_hash_update(p_hash, Z, 32);
    ret |= mt_unf_cipher_hash_update(p_hash, msg, mlen);
    ret |= mt_unf_cipher_hash_final(p_hash, e);

    //dump("e", e, 32);

    return ret;
}

/*
SM2 sign:
ZA=H (ENTLA ||IDA||a||b|| xG|| yG||xA||yA)£¬ Hash:SM3
1. M*=ZA||M,  e = H(M*)
2. generate random k (0 < k < n)
3. P1=k * G= (x1, y1)
4. r=(e+x1) mod n, if r=0 or r+k=n, goto 2. 
5. s=(1+ dA)^(-1)*(k -r*dA) mod n, if s=0, then goto 2.
6. M signature: (r, s)
*/
int mt_unf_ecc_sm2_sign(mt_u8 *id, mt_u8 *priKey,
	mt_u8 *msg, mt_u32 mlen, mt_u8 *r, mt_u8 *s)
{
    mt_handle handle_ecp = MT_NULL;
    mt_handle handle_mod = MT_NULL;
    MT_CIPHER_EC_POINT_S pubKey;
    MT_CIPHER_EC_POINT_S P1;
    MT_CIPHER_EC_PARAMS_S xParams;
    mt_u8 k[32] = {
        0x59,0x27,0x6E,0x27,0xD5,0x06,0x86,0x1A,0x16,0x68,0x0F,0x3A,0xD9,0xC0,0x2D,0xCC,
        0xEF,0x3C,0xC1,0xFA,0x3C,0xDB,0xE4,0xCE,0x6D,0x54,0xB8,0x0D,0xEA,0xC1,0xBC,0x21};
    mt_u8 e[32];
    mt_u8 x[64] = {0};
    mt_u8 *y = &x[32];
    mt_u8 rk[32];
    mt_u8 d_1[32];
    int ret;

    if (!id || !priKey || !msg || !r || !s)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    //dump("pubKey.X", pubKey.X, 32);
    //dump("pubKey.Y", pubKey.Y, 32);

    memset(&xParams, 0, sizeof(xParams));
    xParams.q = ecc_sm2p256v1.p;
    xParams.a = ecc_sm2p256v1.a;
    xParams.b = ecc_sm2p256v1.b;
    xParams.GX = ecc_sm2p256v1.gx;
    xParams.GY = ecc_sm2p256v1.gy;
    xParams.n = ecc_sm2p256v1.n;
    xParams.h = ecc_sm2p256v1.h;
    xParams.keySize = 32;

    pubKey.X = x;
    pubKey.Y = y;
    ret = mt_unf_ecc_ecdh_gen_pubkey(&xParams, priKey, pubKey.X, pubKey.Y);
    if (ret != MT_SUCCESS)
        goto EXIT;

    P1.X = x;
    P1.Y = y;

    /* 1. M*=ZA||M,  e = H(M*) */
    ret = sm2_e(id, &pubKey, msg, mlen, e);
    if (ret != MT_SUCCESS)
        goto EXIT;

    ret = mt_unf_cipher_ecp_create(&handle_ecp);
    if (ret != MT_SUCCESS)
        goto EXIT;

    ret = mt_unf_cipher_bn_create(&handle_mod);
    if (ret != MT_SUCCESS)
        goto EXIT;

    while (1) {
        /* 2. generate random k (0 < k < n) */
        ret = mt_unf_cipher_get_random_number(32, k);
        if (ret != MT_SUCCESS)
            goto EXIT;

        /* check k:  0 < k < n */
        if (bn_is_zero(k, 32) || mt_bn_ucmp(k, 32, xParams.n, 32) >= 0)
            continue;
        //dump("k", k, 32);

        /* 3. P1 = k * G */
        ret |= mt_unf_cipher_ecp_mul(handle_ecp, xParams, &P1, k, NULL, NULL);
        if (ret != MT_SUCCESS)
            goto EXIT;
        //dump("x1", P1.X, 32);
        //dump("y1", P1.Y, 32);

        /* 4. r = (e + x1) mod n */
        ret = mt_unf_cipher_bn_mod_add(handle_mod, r, e, P1.X, xParams.n, 32, 32, 32);
        if (ret != MT_SUCCESS)
            goto EXIT;
        //dump("r", r, 32);

        /* 5. s=(1+ dA)^(-1)*(k -r*dA) mod n */
        /* rk = (r + k) mod n */
        ret = mt_unf_cipher_bn_mod_add(handle_mod, rk, r, k, xParams.n, 32, 32, 32);
        if (ret != MT_SUCCESS)
            goto EXIT;

        /* check r != 0 && r+k != 0 */
        if (bn_is_zero(r, 32) || bn_is_zero(rk, 32))
            continue;

        /* d_1 = (1 + priKey)^(-1) mod n */
        memset(d_1, 0, 32);
        d_1[31] = 1;
        ret = mt_unf_cipher_bn_mod_add(handle_mod, d_1, d_1, priKey, xParams.n, 32, 32, 32);
        if (ret != MT_SUCCESS)
            goto EXIT;
        ret = mt_unf_cipher_bn_mod_inv(handle_mod, d_1, d_1, xParams.n, 32, 32);
        if (ret != MT_SUCCESS)
            goto EXIT;
        //dump("(1+d)^-1", d_1, 32);

        /* s = (1+priKey)^-1 * (k - r * priKey) mode n =  ((1+priKey)^-1 * (k + r) - r) mode n */
        /* s = (rk * d_1 -r ) mod n */
        ret = mt_unf_cipher_bn_mod_mul(handle_mod, s, rk, d_1, xParams.n, 32, 32, 32);
        if (ret != MT_SUCCESS)
            goto EXIT;
        ret = mt_unf_cipher_bn_mod_sub(handle_mod, s, s, r, xParams.n, 32, 32, 32);
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
SM2 verify:
1. check 0 < r < n, otherwise verify fail
2. check 0 < s < n, otherwise verify fail
3. M*=ZA||M,  e= H(M*)
4. t= (r+s) mod n, if t=0, verify fail
5. (x1,y1)= s*G + t*PA
6. v=(e+x1) mod n, if v=r, verify success; otherwise fail
*/
int mt_unf_ecc_sm2_verify(mt_u8 *id, mt_u8 *pubKeyX, mt_u8 *pubKeyY,
	mt_u8 *msg, mt_u32 mlen, mt_u8 *r, mt_u8 *s)
{
    mt_handle handle_ecp = MT_NULL;
    mt_handle handle_mod = MT_NULL;
    MT_CIPHER_EC_POINT_S key;
    MT_CIPHER_EC_POINT_S P1, P2;
    MT_CIPHER_EC_PARAMS_S xParams;
    mt_u8 e[32];
    mt_u8 t[32];
    mt_u8 x[32] = {0};
    mt_u8 y[32] = {0};
    mt_u8 x2[32] = {0};
    mt_u8 y2[32] = {0};
    int ret;

    if (!id || !pubKeyX || !pubKeyY || !msg || !r || !s)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    key.X = pubKeyX;
    key.Y = pubKeyY;

    memset(&xParams, 0, sizeof(xParams));
    xParams.q = ecc_sm2p256v1.p;
    xParams.a = ecc_sm2p256v1.a;
    xParams.b = ecc_sm2p256v1.b;
    xParams.GX = ecc_sm2p256v1.gx;
    xParams.GY = ecc_sm2p256v1.gy;
    xParams.n = ecc_sm2p256v1.n;
    xParams.h = ecc_sm2p256v1.h;
    xParams.keySize = 32;

    P1.X = x;
    P1.Y = y;
    P2.X = x2;
    P2.Y = y2;

    /* 1. check r: 1 <= r <= n-1 */
    if (bn_is_zero(r, 32) || mt_bn_ucmp(r, 32, xParams.n, 32) >= 0)
        return -1;

    /* 2. check s: 1 <= s <= n-1 */
    if (bn_is_zero(s, 32) || mt_bn_ucmp(s, 32, xParams.n, 32) >= 0)
        return -1;

    /* 3. M*=ZA||M,  e= H(M*) */
    ret = sm2_e(id, &key, msg, mlen, e);
    if (ret != MT_SUCCESS)
        goto EXIT;
    //dump("e", e, 32);

    ret = mt_unf_cipher_bn_create(&handle_mod);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* 4. t = (r + s) mod n */
    ret = mt_unf_cipher_bn_mod_add(handle_mod, t, r, s, xParams.n, 32, 32, 32);
    if (ret != MT_SUCCESS)
        goto EXIT;
    //dump("t", t, 32);

    /* check t != 0 */
    if (bn_is_zero(t, 32))
        goto EXIT;

    /* 5. (x1,y1)= s*G + t*PA */
    ret = mt_unf_cipher_ecp_create(&handle_ecp);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* P1 = s * G */
    ret |= mt_unf_cipher_ecp_mul(handle_ecp, xParams, &P1, s, NULL, NULL);
    if (ret != MT_SUCCESS)
        goto EXIT;
    //dump("x1", P1.X, 32);
    //dump("y1", P1.Y, 32);

    /* P2 = t * pubKey */
    ret |= mt_unf_cipher_ecp_mul(handle_ecp, xParams, &P2, NULL, &key, t);
    if (ret != MT_SUCCESS)
        goto EXIT;
    //dump("x2", P2.X, 32);
    //dump("y2", P2.Y, 32);

    /* P1 = s * G + t * pubKey */
    ret |= mt_unf_cipher_ecp_add(handle_ecp, xParams, &P1, &P1, &P2);
    //dump("x", P1.X, 32);
    //dump("y", P1.Y, 32);

    /* 6. v = (e + x1) mod n */
    ret = mt_unf_cipher_bn_mod_add(handle_mod, t, e, P1.X, xParams.n, 32, 32, 32);
    if (ret != MT_SUCCESS)
        goto EXIT;
    //dump("v", t, 32);

    //dump("r", r, 32);

    /* check v == r */
    if (mt_bn_ucmp(t, 32, r, 32) != 0)
        ret = MT_CIPHER_ERR_ECC_VERIFY_FAILED;

EXIT:
    if (handle_mod)
        mt_unf_cipher_bn_destroy(handle_mod);

    if (handle_ecp)
        mt_unf_cipher_ecp_destroy(handle_ecp);

    return ret;
}

static int KDF(mt_u8 *seed, mt_u32 seedlen, mt_u8 *out, mt_u32 outlen)
{
    int ret = MT_FAILURE;
    mt_handle hash_handle = MT_INVALID_HANDLE;
    mt_u32 i;
    mt_u32 len = 0;
    mt_u8 cnt[4];
    mt_u8 dgst[32];
    mt_u32 mdlen;

    if (!out || !seed)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    mdlen = 32;

    for (i = 1; len < outlen; i++) {
        cnt[0] = (mt_u8)((i >> 24) & 0xFF);
        cnt[1] = (mt_u8)((i >> 16) & 0xFF);
        cnt[2] = (mt_u8)((i >> 8) & 0xFF);
        cnt[3] = (mt_u8)(i & 0xFF);

        ret = mt_unf_cipher_hash_create(MT_CIPHER_HASH_TYPE_SM3, NULL, &hash_handle);
        ret |= mt_unf_cipher_hash_update(hash_handle, seed, seedlen);
        ret |= mt_unf_cipher_hash_update(hash_handle, cnt, 4);
        ret |= mt_unf_cipher_hash_final(hash_handle, dgst);
        if (ret != MT_SUCCESS)
            break;

        if (len + mdlen <= outlen) {
            memcpy(out + len, dgst, mdlen);
            len += mdlen;
        } else {
            memcpy(out + len, dgst, outlen - len);
            len = outlen;
            return MT_SUCCESS;
        }
    }

    return MT_FAILURE;
}

int mt_unf_ecc_sm2_encrypt(mt_u8 *pubKeyX, mt_u8 *pubKeyY,
	mt_u8 *msg, mt_u32 mlen, mt_u8 *out, mt_u32 *outlen)
{
    int ret = MT_FAILURE;
    mt_handle handle_hash = MT_INVALID_HANDLE;
    mt_handle handle_ecp = MT_INVALID_HANDLE;
    mt_u8 k[32] = {
        0x59,0x27,0x6E,0x27,0xD5,0x06,0x86,0x1A,0x16,0x68,0x0F,0x3A,0xD9,0xC0,0x2D,0xCC,
        0xEF,0x3C,0xC1,0xFA,0x3C,0xDB,0xE4,0xCE,0x6D,0x54,0xB8,0x0D,0xEA,0xC1,0xBC,0x21};
    MT_CIPHER_EC_PARAMS_S xParams;
    MT_CIPHER_EC_POINT_S keyB;
    MT_CIPHER_EC_POINT_S P1;
    MT_CIPHER_EC_POINT_S P2;
    mt_u8 buf[64] = {0};
    mt_u8 *C1, *C2, *C3;
    mt_u32 i;

    if (!pubKeyX || !pubKeyY || !msg || !out || !outlen)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (mlen + 64 + 32 > *outlen)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    keyB.X = pubKeyX;
    keyB.Y = pubKeyY;

    C1 = &out[0];
    C2 = &out[64];
    C3 = &out[64 + mlen];

    P1.X = &C1[0];
    P1.Y = &C1[32];

    P2.X = &buf[0];
    P2.Y = &buf[32];

    memset(&xParams, 0, sizeof(xParams));
    xParams.q = ecc_sm2p256v1.p;
    xParams.a = ecc_sm2p256v1.a;
    xParams.b = ecc_sm2p256v1.b;
    xParams.GX = ecc_sm2p256v1.gx;
    xParams.GY = ecc_sm2p256v1.gy;
    xParams.n = ecc_sm2p256v1.n;
    xParams.h = ecc_sm2p256v1.h;
    xParams.keySize = 32;

    ret = mt_unf_cipher_ecp_create(&handle_ecp);
    if (ret != MT_SUCCESS)
        goto EXIT;

    while (1) {
        /* 1. generate random k (0 < k < n) */
        ret = mt_unf_cipher_get_random_number(32, k);
        if (ret != MT_SUCCESS)
            goto EXIT;

        /* check k:  0 < k < n */
        if (bn_is_zero(k, 32) || mt_bn_ucmp(k, 32, xParams.n, 32) >= 0)
            continue;
        dump("k", k, 32);

        /* 2. C1 = k * G */
        ret = mt_unf_cipher_ecp_mul(handle_ecp, xParams, &P1, k, NULL, NULL);
        if (ret != MT_SUCCESS)
            goto EXIT;
        dump("x1", P1.X, 32);
        dump("y1", P1.Y, 32);

        /* 3. check keyB */
        // pointmul not support * 1

        /* 4. P2(x2, y2) = k * keyB */
        ret = mt_unf_cipher_ecp_mul(handle_ecp, xParams, &P2, NULL, &keyB, k);
        if (ret != MT_SUCCESS)
            goto EXIT;
        dump("x2", P2.X, 32);
        dump("y2", P2.Y, 32);

        /* 5. t = KDF(x2||y2, mlen) */
        ret = KDF(buf, 64, C2, mlen);
        if (ret != MT_SUCCESS)
            goto EXIT;
        if (bn_is_zero(C2, mlen))
            continue;
        dump("t", C2, mlen);

        /* 6. C2 = msg ^ t */
        for (i = 0; i < mlen; i++) {
            C2[i] ^= msg[i];
        }
        dump("C2", C2, mlen);

        /* 7. C3 = Hash(x2||M||y2) */
        ret = mt_unf_cipher_hash_create(MT_CIPHER_HASH_TYPE_SM3, NULL, &handle_hash);
        ret |= mt_unf_cipher_hash_update(handle_hash, P2.X, 32);
        ret |= mt_unf_cipher_hash_update(handle_hash, msg, mlen);
        ret |= mt_unf_cipher_hash_update(handle_hash, P2.Y, 32);
        ret |= mt_unf_cipher_hash_final(handle_hash, C3);
        if (ret != MT_SUCCESS)
            break;
        dump("C3", C3, 32);

        break;
    }

EXIT:
    if (handle_ecp != MT_INVALID_HANDLE)
        mt_unf_cipher_ecp_destroy(handle_ecp);

    if (ret == MT_SUCCESS) {
        *outlen = mlen + 64 + 32;
        dump("cipher", out, *outlen);
    }

    return ret;
}

int mt_unf_ecc_sm2_decrypt(mt_u8 *priKey,
	mt_u8 *cipher, mt_u32 clen, mt_u8 *out, mt_u32 *outlen)
{
    int ret = MT_FAILURE;
    mt_handle handle_hash = MT_INVALID_HANDLE;
    mt_handle handle_ecp = MT_INVALID_HANDLE;
    MT_CIPHER_EC_PARAMS_S xParams;
    MT_CIPHER_EC_POINT_S P1;
    MT_CIPHER_EC_POINT_S P2;
    mt_u8 buf[64] = {0};
    mt_u8 dgst[32] = {0};
    mt_u8 *C1, *C2, *C3;
    mt_u32 mlen;
    mt_u32 i;

    if (!priKey || !cipher || !out || !outlen)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (clen <= 64 + 32)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (clen > *outlen + 64 + 32)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    mlen = clen - 64 - 32;

    C1 = &cipher[0];
    C2 = &cipher[64];
    C3 = &cipher[64 + mlen];
    dump("C1", C1, 64);
    dump("C2", C2, mlen);
    dump("C3", C3, 32);

    memset(&xParams, 0, sizeof(xParams));
    xParams.q = ecc_sm2p256v1.p;
    xParams.a = ecc_sm2p256v1.a;
    xParams.b = ecc_sm2p256v1.b;
    xParams.GX = ecc_sm2p256v1.gx;
    xParams.GY = ecc_sm2p256v1.gy;
    xParams.n = ecc_sm2p256v1.n;
    xParams.h = ecc_sm2p256v1.h;
    xParams.keySize = 32;

    P1.X = &C1[0];
    P1.Y = &C1[32];

    P2.X = &buf[0];
    P2.Y = &buf[32];

    /* 1. check C1 */
    if (mt_unf_ecc_point_check(&xParams, P1.X, P1.Y))
        return MT_FAILURE;

    ret = mt_unf_cipher_ecp_create(&handle_ecp);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* 2. check h*C1 */
    // pointmul not support * 1

    /* 3. P2(x2, y2) = priKey * C1 */
    ret = mt_unf_cipher_ecp_mul(handle_ecp, xParams, &P2, NULL, &P1, priKey);
    if (ret != MT_SUCCESS)
        goto EXIT;
    dump("x2", P2.X, 32);
    dump("y2", P2.Y, 32);

    /* 4. t = KDF(x2||y2, mlen) */
    ret = KDF(buf, 64, out, mlen);
    if (ret != MT_SUCCESS)
        goto EXIT;
    if (bn_is_zero(out, mlen))
        goto EXIT;
    dump("t", out, mlen);

    /* 5. M = C2 ^ t */
    for (i = 0; i < mlen; i++) {
        out[i] ^= C2[i];
    }
    dump("M", out, mlen);

    /* 6. dgst = Hash(x2||M||y2) */
    ret = mt_unf_cipher_hash_create(MT_CIPHER_HASH_TYPE_SM3, NULL, &handle_hash);
    ret |= mt_unf_cipher_hash_update(handle_hash, P2.X, 32);
    ret |= mt_unf_cipher_hash_update(handle_hash, out, mlen);
    ret |= mt_unf_cipher_hash_update(handle_hash, P2.Y, 32);
    ret |= mt_unf_cipher_hash_final(handle_hash, dgst);
    if (ret != MT_SUCCESS)
        goto EXIT;
    dump("dgst", dgst, 32);

    /* dgst == C3 ? */
    ret = MT_FAILURE;
    if (mt_bn_ucmp(dgst, 32, C3, 32) == 0) {
        *outlen = mlen;
        ret = MT_SUCCESS;
    }

    EXIT:
    if (handle_ecp != MT_INVALID_HANDLE)
        mt_unf_cipher_ecp_destroy(handle_ecp);

    return ret;
}

