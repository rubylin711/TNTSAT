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

static mt_u8 sha1_null[20] = { 0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d, 0x32, 0x55, 0xbf, 0xef, 0x95, 0x60, 0x18, 0x90,
                               0xaf, 0xd8, 0x07, 0x09, };
static mt_u8 sha256_null[32] = { 0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
                                 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55, };

static unsigned int constant_time_msb(unsigned int a)
{
    return 0 - (a >> (sizeof(a) * 8 - 1));
}

static unsigned int constant_time_is_zero(unsigned int a)
{
    return constant_time_msb(~a & (a - 1));
}

static inline unsigned int constant_time_eq(unsigned int a, unsigned int b)
{
    return constant_time_is_zero(a ^ b);
}

static inline unsigned int constant_time_select(unsigned int mask,
                                                unsigned int a,
                                                unsigned int b)
{
    return (mask & a) | (~mask & b);
}

static inline int constant_time_select_int(unsigned int mask, int a, int b)
{
    return (int)(constant_time_select(mask, (unsigned)(a), (unsigned)(b)));
}

/*
     DB = HASH(L) + PS + 01 + M
     DBmask = MGF(seed) ^ DB
     seedmask = seed ^ MGF(DBmask)
     EM = 00 + seedmask + DBmask
*/
int PKCS1_OAEP_mgf1_encode(mt_u8 *to, mt_u32 tlen,
    mt_u8 *from, mt_u32 flen, mt_u32 hashtype)
{
    mt_u32 i;
    mt_u8 *db, *seed;
    mt_u8 seedmask[64], dbmask[256];
    mt_u32 emlen, hlen, dblen, pslen;

    if (to == NULL || tlen == 0 || from == NULL || flen == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    emlen = tlen;

    hlen = (MT_CIPHER_HASH_TYPE_SHA1 == hashtype) ? 20 : 32;

    if((flen +  2 * hlen + 2 > emlen) || (emlen < 2 * hlen + 2)) 
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    dblen = emlen -hlen -1;
    pslen = dblen - flen - hlen - 1;

    to[0] = 0;
    seed = to + 1;
    db = to + hlen + 1;

    /* HASH(L) */
    if (MT_CIPHER_HASH_TYPE_SHA1 == hashtype) {
	memcpy(db, sha1_null, hlen);
    } else {
	memcpy(db, sha256_null, hlen);
    }

    /* PS with zero-padded */
    memset(db + hlen, 0, pslen);

    /* 01 */
    db[hlen + pslen] = 0x01;

    /* M */
    memcpy(db + hlen + pslen + 1, from, flen);

    /* seed: random data */
    if (mt_unf_cipher_get_random_number(hlen, seed) < 0) {
	printf("RSA_padding_add_OAEP gen random seed failed\n");
	return MT_FAILURE;
    }

    /* DBmask = MGF(seed) ^ DB */
    MGF1(dbmask, dblen, seed, hlen, hashtype);
    for (i = 0; i < dblen; i++)
	db[i] ^= dbmask[i];

    /* seedmask = seed ^ MGF(DBmask) */
    MGF1(seedmask, hlen, db, dblen, hashtype);
    for (i = 0; i < hlen; i++)
	seed[i] ^= seedmask[i];

    return MT_SUCCESS;
}

/*
     EM = 00 + seedmask + DBmask
     seed = seedmask ^ MGF(DBmask)
     DB = MGF(seed) ^ DBmask
     DB = HASH(L) + PS + 01 + M
*/
int PKCS1_OAEP_mgf1_decode(mt_u8 *to, mt_u32 *tlen,
    mt_u8 *from, mt_u32 flen, mt_u32 num, mt_u32 hashtype)
{
    mt_u32 i, mlen = 0, one_index = 0, msg_index;
    mt_u32 good, found_one_byte;
    mt_u8 *maskedseed, *maskeddb;
    mt_u8 db[256], em[256], seed[64], phash[64];
    mt_u32 emlen, hlen, dblen;
    mt_u32 bad = 0;

    if (to == NULL || tlen == 0 || from == NULL || flen == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (flen <= 0)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    hlen = (MT_CIPHER_HASH_TYPE_SHA1 == hashtype) ? 20 : 32;
    emlen = num;
    dblen = emlen - hlen - 1;

    /*
     * |num| is the length of the modulus; |flen| is the length of the
     * encoded message. Therefore, for any |from| that was obtained by
     * decrypting a ciphertext, we must have |flen| <= |num|. Similarly,
     * num < 2 * mdlen + 2 must hold for the modulus irrespective of
     * the ciphertext, see PKCS #1 v2.2, section 7.1.2.
     * This does not leak any side-channel information.
     */
    if (emlen < flen || emlen < 2 * hlen + 2)
	return MT_RSA_ERR_BAD_PADDING;

    /*
     * Always do this zero-padding copy (even when num == flen) to avoid
     * leaking that information. The copy still leaks some side-channel
     * information, but it's impossible to have a fixed  memory access
     * pattern since we can't read out of the bounds of |from|.
     *
     * TODO(emilia): Consider porting BN_bn2bin_padded from BoringSSL.
     */
    memset(em, 0, emlen);
    memcpy(em + emlen - flen, from, flen);

    /*
     * The first byte must be zero, however we must not leak if this is
     * true. See James H. Manger, "A Chosen Ciphertext  Attack on RSA
     * Optimal Asymmetric Encryption Padding (OAEP) [...]", CRYPTO 2001).
     */
    good = constant_time_is_zero((mt_u32)em[0]);

    maskedseed = em + 1;
    maskeddb = em + 1 + hlen;

    /* seed = seedmask ^ MGF(DBmask) */
    MGF1(seed, hlen, maskeddb, dblen, hashtype);
    for (i = 0; i < hlen; i++) {
	seed[i] ^= maskedseed[i];
    }

    /* DB = MGF(seed) ^ DBmask */
    MGF1(db, dblen, seed, hlen, hashtype);
    for (i = 0; i < dblen; i++) {
	db[i] ^= maskeddb[i];
    }

    if (MT_CIPHER_HASH_TYPE_SHA1 == hashtype) {
	memcpy(phash, sha1_null, (mt_u32)hlen);
    } else {
	memcpy(phash, sha256_null, (mt_u32)hlen);
    }

    good &= constant_time_is_zero((mt_u32)memcmp(db, phash, (mt_u32)hlen));
    for (i = 0; i < hlen; i++) {
	bad |= db[i] ^ phash[i];
	//good &= ~(db[i] ^ phash[i]);
    }

    good &= constant_time_is_zero(bad);

    found_one_byte = 0;
    for (i = hlen; i < dblen; i++) {
	/*
         * Padding consists of a number of 0-bytes, followed by a 1.
         */
	unsigned int equals1 = constant_time_eq(db[i], 1);
	unsigned int equals0 = constant_time_is_zero(db[i]);
	one_index = constant_time_select(~found_one_byte & equals1,
	                                     i, one_index);
	found_one_byte |= equals1;
	good &= (found_one_byte | equals0);
    }

    good &= found_one_byte;

    /*
     * At this point |good| is zero unless the plaintext was valid,
     * so plaintext-awareness ensures timing side-channels are no longer a
     * concern.
     */
    if (!good)
	return MT_RSA_ERR_BAD_PADDING;

    msg_index = one_index + 1;
    mlen = dblen - msg_index;

    memcpy(to, db + msg_index, mlen);
    *tlen = mlen;

    return MT_SUCCESS;
}

