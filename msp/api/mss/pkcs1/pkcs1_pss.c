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

/*
     M' = pad + mHash + salt
     H = hash(M')
     DB = PS + 01 + salt
     maskedDB = DB ^ MGF(H)
     EM = maskedDB + H + 0xBC
*/
int PKCS1_PSS_sign(mt_u8 *msg, mt_u32 mlen, mt_u32 emBits,
        mt_u8 *sign, mt_u32 slen, mt_u32 hashtype)
{
    int ret, i;
    mt_handle hash_handle;
    mt_u32 hlen, saltlen;
    mt_u32 emlen, dblen;
    mt_u8 M1[32+32+8];
    mt_u8 mask[256];
    mt_u8 *mHash, *salt;
    mt_u8 *DB;
    mt_u8 *em, *H, *BC;
    mt_u8 lmask;

    hlen = (MT_CIPHER_HASH_TYPE_SHA1 == hashtype) ? 20 : 32;

    em = sign;
    emlen = (emBits+7)/8;

    lmask = 0;
    for (i=0; i<emlen*8-emBits; i++)
        lmask = (lmask>>1) | 0x80;

    if(emlen >= hlen + hlen + 2)
        saltlen = hlen;
    else if(emlen >= hlen + hlen)
        saltlen = emlen - hlen - 2;
    else
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    /* M' = pad + mHash + salt */

    /* pad with zero-padded */
    memset(M1, 0, 8 + hlen + saltlen);

    mHash = M1 + 8;
    salt = mHash + hlen;

    /* mHash */
    ret = mt_unf_cipher_hash_create(hashtype, NULL, &hash_handle);
    ret |= mt_unf_cipher_hash_update(hash_handle, msg, mlen);
    ret |= mt_unf_cipher_hash_final(hash_handle, mHash);

    /* salt: random data */
    mt_unf_cipher_get_random_number(saltlen, salt);		

    /* H = hash(M') */
    H = em + emlen - hlen - 1;

    ret = mt_unf_cipher_hash_create(hashtype, NULL, &hash_handle);
    ret |= mt_unf_cipher_hash_update(hash_handle, M1, 8 + hlen + saltlen);
    ret |= mt_unf_cipher_hash_final(hash_handle, H);

    /* DB = PS + 01 + salt */
    DB = em;
    dblen = emlen - hlen - 1;
    memset(DB, 0, dblen - saltlen -1);
    DB[dblen - saltlen -1] = 0x01;
    memcpy(DB + dblen - saltlen, salt, saltlen);

    /* maskedDB = DB ^ MGF(H) */
    MGF1(mask, dblen, H, hlen, hashtype);
    for (i = 0; i < dblen; i++)
        DB[i] ^= mask[i];

    DB[0] &= (mt_u8)~lmask;
    
    /* 0xBC */
    BC = H + hlen;
    *BC = 0xBC;

    return MT_SUCCESS;
}

/*
     EM = maskedDB + H + 'BC' ==> maskDB & H
     DB = maskedDB ^ MGF(H)
     DB = PS + 01 + salt ==> salt
     M' = pad + mHash + salt
     H' = hash(M')
     compare H & H'
*/
int PKCS1_PSS_verify(mt_u8 *msg, mt_u32 mlen, mt_u32 emBits,
        mt_u8 *sign, mt_u32 slen, mt_u32 hashtype)
{
    int ret, i;
    mt_handle hash_handle;
    mt_u32 hlen, saltlen;
    mt_u32 emlen, dblen;
    mt_u8 M1[32+32+8];
    mt_u8 mask[256];
    mt_u8 H1[32];
    mt_u8 *mHash, *salt;
    mt_u8 *DB;
    mt_u8 *em, *maskedDB, *H;
    mt_u8 *p;
    mt_u8 lmask;

    hlen = (MT_CIPHER_HASH_TYPE_SHA1 == hashtype) ? 20 : 32;

    em = sign;
    emlen = (emBits+7)/8;

    lmask = 0;
    for (i=0; i<emlen*8-emBits; i++)
        lmask = (lmask>>1) | 0x80;

    /* BC */
    if (em[emlen - 1] != 0xBC)
        return MT_RSA_ERR_BAD_PADDING;

    /* maskedDB */
    maskedDB = em;
    dblen = emlen - hlen -1;

    /* H */
    H = em + dblen;

    /* DB = maskedDB ^ MGF(H) */
    DB = maskedDB;

    MGF1(mask, dblen, H, hlen, hashtype);

    for (i = 0; i < dblen; i++)
    	DB[i] ^= mask[i];

    DB[0] &= (mt_u8)~lmask;

    /* DB = PS + 01 + salt ==> salt */
    p = DB;
    while( p < H - 1 && *p == 0 )
        p++;

    if( *p++ != 0x01 )
        return MT_RSA_ERR_BAD_PADDING;

    salt = p;
    saltlen = (mt_u32)(H - salt);

    /* M' = pad + mHash + salt */

    /* pad with zero-padded */
    memset(M1, 0, 8);

    mHash = M1 + 8;

    /* mHash */
    ret = mt_unf_cipher_hash_create(hashtype, NULL, &hash_handle);
    ret |= mt_unf_cipher_hash_update(hash_handle, msg, mlen);
    ret |= mt_unf_cipher_hash_final(hash_handle, mHash);

    /* salt */
    memcpy(mHash + hlen, salt, saltlen);

    /* H' = hash(M') */
    H = em + emlen - hlen - 1;

    ret = mt_unf_cipher_hash_create(hashtype, NULL, &hash_handle);
    ret |= mt_unf_cipher_hash_update(hash_handle, M1, 8 + hlen + saltlen);
    ret |= mt_unf_cipher_hash_final(hash_handle, H1);

    if(memcmp(H, H1, hlen) != 0)
        return MT_RSA_ERR_BAD_PADDING;

    return MT_SUCCESS;
}

