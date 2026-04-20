/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_ECC_H__
#define __MT_UNF_ECC_H__

#define MT_CIPHER_ERR_ECC_VERIFY_FAILED 0xEECC0001

int mt_unf_ecc_point_check(MT_CIPHER_EC_PARAMS_S *xParams, mt_u8 *px, mt_u8 *py);

int mt_unf_ecc_ecdh_gen_keypair(MT_CIPHER_EC_PARAMS_S *xParams,
    mt_u8 *priKey, mt_u8 *pubKeyX, mt_u8 *pubKeyY);
int mt_unf_ecc_ecdh_gen_pubkey(MT_CIPHER_EC_PARAMS_S *xParams,
    mt_u8 *priKey, mt_u8 *pubKeyX, mt_u8 *pubKeyY);
int mt_unf_ecc_ecdh_gen_sharekey(MT_CIPHER_EC_PARAMS_S *xParams,
    mt_u8 *priKeyA, mt_u8 *pubKeyBX, mt_u8 *pubKeyBY,
    mt_u8 *shareKeyX, mt_u8 *shareKeyY);

int mt_unf_ecc_ecdsa_sign(MT_CIPHER_EC_PARAMS_S *xParams,
    MT_CIPHER_HASH_TYPE_E xHashType, mt_u8 *priKey,
    mt_u8 *msg, mt_u32 mlen, mt_u8 *r, mt_u8 *s);
int mt_unf_ecc_ecdsa_verify(MT_CIPHER_EC_PARAMS_S *xParams,
    MT_CIPHER_HASH_TYPE_E xHashType, mt_u8 *pubKeyX, mt_u8 *pubKeyY,
    mt_u8 *msg, mt_u32 mlen, mt_u8 *r, mt_u8 *s);

int mt_unf_ecc_sm2_sign(mt_u8 *id, mt_u8 *priKey,
	mt_u8 *msg, mt_u32 mlen, mt_u8 *r, mt_u8 *s);
int mt_unf_ecc_sm2_verify(mt_u8 *id, mt_u8 *pubKeyX, mt_u8 *pubKeyY,
	mt_u8 *msg, mt_u32 mlen, mt_u8 *r, mt_u8 *s);

int mt_unf_ecc_sm2_encrypt(mt_u8 *pubKeyX, mt_u8 *pubKeyY,
	mt_u8 *msg, mt_u32 mlen, mt_u8 *out, mt_u32 *outlen);
int mt_unf_ecc_sm2_decrypt(mt_u8 *priKey,
	mt_u8 *cipher, mt_u32 clen, mt_u8 *out, mt_u32 *outlen);

#endif
