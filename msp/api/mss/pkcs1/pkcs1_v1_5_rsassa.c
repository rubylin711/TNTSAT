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

static mt_u8 sha1_digestinfo[15] = {0x30,0x21,0x30,0x09,0x06,0x05,0x2b,0x0e,0x03,0x02,0x1a,0x05,0x00,0x04,0x14};
static mt_u8 sha256_digestinfo[19] = {0x30,0x31,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0x04,0x20};

int PKCS1_V1_5_sign(mt_u8 *msg, mt_u32 mlen,
        mt_u8 *sign, mt_u32 slen, mt_u32 hashtype)
{
    int ret;
    mt_handle hash_handle;
    mt_u8 digestinfo[64];
    mt_u32 digestinfoLen;
    mt_u8 *digest;

    digestinfoLen = (MT_CIPHER_HASH_TYPE_SHA1 == hashtype) ? 35 : 51;

    if (MT_CIPHER_HASH_TYPE_SHA1 == hashtype)
    {
        digestinfoLen = 35;
        memcpy(digestinfo, sha1_digestinfo, 15);
        digest = digestinfo + 15;
    }
    else if (MT_CIPHER_HASH_TYPE_SHA256 == hashtype)
    {
        digestinfoLen = 51;
        memcpy(digestinfo, sha256_digestinfo, 19);
        digest = digestinfo + 19;
    }
    else
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    /* digest = HASH(msg) */
    ret = mt_unf_cipher_hash_create(hashtype, NULL, &hash_handle);
    ret |= mt_unf_cipher_hash_update(hash_handle, msg, mlen);
    ret |= mt_unf_cipher_hash_final(hash_handle, digest);

    PKCS1_V1_5_encode(sign, slen, digestinfo, digestinfoLen, MT_RSA_SIGN);

    return MT_SUCCESS;
}

int PKCS1_V1_5_verify(mt_u8 *msg, mt_u32 mlen,
        mt_u8 *sign, mt_u32 slen, mt_u32 hashtype)
{
    int ret;
    mt_handle hash_handle;
    mt_u8 digestinfo[64];
    mt_u32 digestinfoLen;
    mt_u8 *digest;
    mt_u8 hash[32];
    mt_u32 hlen;

    ret = PKCS1_V1_5_decode(digestinfo, &digestinfoLen, sign, slen, MT_RSA_SIGN);
    if (ret != MT_SUCCESS)
        return ret;

    if (MT_CIPHER_HASH_TYPE_SHA1 == hashtype)
    {
        if (digestinfoLen != 35)
            return MT_RSA_ERR_BAD_PADDING;

        if (memcmp(digestinfo, sha1_digestinfo, 15))
            return MT_RSA_ERR_BAD_PADDING;

        digest = digestinfo + 15;
        hlen = 20;
    }
    else if (MT_CIPHER_HASH_TYPE_SHA256 == hashtype)
    {
        if (digestinfoLen != 51)
            return MT_RSA_ERR_BAD_PADDING;

        if (memcmp(digestinfo, sha256_digestinfo, 19))
            return MT_RSA_ERR_BAD_PADDING;

        digest = digestinfo + 19;
        hlen = 32;
    }
    else
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    /* digest = HASH(msg) */
    ret = mt_unf_cipher_hash_create(hashtype, NULL, &hash_handle);
    ret |= mt_unf_cipher_hash_update(hash_handle, msg, mlen);
    ret |= mt_unf_cipher_hash_final(hash_handle, hash);

    if(memcmp(digest, hash, hlen) != 0)
        return MT_RSA_ERR_BAD_PADDING;

    return MT_SUCCESS;
}

