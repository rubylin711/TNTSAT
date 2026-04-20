/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef MSS_TEE_CLIENT_H
#define MSS_TEE_CLIENT_H

#include <stdint.h>

#include "mt_unf_cipher_v2.h"
#include "mt_unf_rsa.h"
#include "mt_unf_cert.h"

int tee_keyslot_request(uint32_t num, uint32_t *p_keyslot);
int tee_keyslot_release(uint32_t keyslot);
int tee_keyslot_set(uint32_t keyslot, MT_CIPHER_CTRL_S *p_ctrl,
    uint8_t *key,uint32_t key_size, uint8_t *iv, uint32_t iv_size);
int tee_keyslot_set_ext(uint32_t keyslot, MT_KT_CTRL_S *p_ctrl,
    uint8_t *key,uint32_t key_size, uint8_t *iv, uint32_t iv_size);
int tee_keyslot_set_iv(uint32_t keyslot, uint8_t *iv, uint32_t iv_size);

int tee_crypto_create(MT_CIPHER_CRYPTO_CH_E index, uint32_t *p_handle);
int tee_crypto_config(uint32_t handle, MT_CIPHER_CTRL_S *p_ctrl, uint32_t key_slot);
int tee_crypto_process(uint32_t handle, uint8_t *p_src_addr, uint8_t *p_dest_addr, uint32_t length);
int tee_crypto_destroy(uint32_t handle);
int tee_ts_create(MT_CIPHER_CRYPTO_CH_E index, uint32_t *p_handle);
int tee_ts_config(uint32_t handle,
	MT_CIPHER_CTRL_S *p_ctrl, MT_CIPHER_TS_PARA_S *p_ts_para,
	uint32_t even_key_slot,	uint32_t odd_key_slot);
int tee_ts_process(uint32_t handle, uint8_t *p_src_addr, uint8_t *p_dest_addr, uint32_t length);
int tee_ts_destroy(uint32_t handle);

int tee_hash_create(uint32_t hash_type, uint32_t *p_handle);
int tee_hash_update(uint32_t handle, uint8_t *data, uint32_t length);
int tee_hash_final(uint32_t handle, uint8_t *digest, uint32_t digest_length);

int tee_mac_create(uint32_t mac_type,
    MT_CIPHER_HMAC_ATTS_S *p_hmac_attr, uint32_t *p_handle);
int tee_mac_update(uint32_t handle, uint8_t *data, uint32_t length);
int tee_mac_final(uint32_t handle, uint8_t *digest, uint32_t digest_length);

int tee_keyladder_create(MT_CIPHER_KEYLADDER_TYPE_E index, uint32_t *p_handle);
int tee_keyladder_destroy(uint32_t handle);
int tee_keyladder_set_signature(uint32_t handle, uint8_t *p_sig, uint32_t sigLen);
int tee_keyladder_start(uint32_t handle, MT_CIPHER_KEYLADDER_SOURCE_E keysrc);
int tee_keyladder_link(uint32_t handle, MT_CIPHER_CTRL_S *p_ctrl, uint8_t *input, uint32_t length);
int tee_keyladder_end(uint32_t handle, uint32_t keyslot);

int tee_rsa_public_encrypt(struct rsa_public_key *key,
	uint8_t *src, uint32_t src_len, uint8_t *dst, MT_RSA_ALG algo);
int tee_rsa_private_decrypt(struct rsa_keypair *key,
	uint8_t *src, uint32_t src_len, uint8_t *dst, uint32_t *dst_len, MT_RSA_ALG algo);
int tee_rsa_private_encrypt(struct rsa_keypair *key,
	uint8_t *src, uint32_t src_len, uint8_t *dst, MT_RSA_ALG algo);
int tee_rsa_public_decrypt(struct rsa_public_key *key,
	uint8_t *src, uint32_t src_len, uint8_t *dst, uint32_t *dst_len, MT_RSA_ALG algo);
int tee_rsa_sign(struct rsa_keypair *key, MT_RSA_ALG algo,
	uint8_t*msg, uint32_t mlen, uint8_t *sign, uint32_t *slen);
int tee_rsa_verify(struct rsa_public_key *key, MT_RSA_ALG algo,
	uint8_t*msg, uint32_t mlen, uint8_t *sign, uint32_t slen);

int tee_mod_mod(uint8_t *d, uint32_t d_length, uint8_t *m, uint32_t m_length, uint8_t *r, uint32_t r_length);
int tee_mod_mul(uint8_t *a, uint32_t a_length, uint8_t *b, uint32_t b_length,
    uint8_t *m, uint32_t m_length, uint8_t *r, uint32_t r_length);
int tee_mod_add(uint8_t *a, uint32_t a_length, uint8_t *b, uint32_t b_length,
    uint8_t *m, uint32_t m_length, uint8_t *r, uint32_t r_length);
int tee_mod_sub(uint8_t *a, uint32_t a_length, uint8_t *b, uint32_t b_length,
    uint8_t *m, uint32_t m_length, uint8_t *r, uint32_t r_length);
int tee_mod_inv(uint8_t *d, uint32_t d_length, uint8_t *m, uint32_t m_length, uint8_t *r, uint32_t r_length);
int tee_mod_exp(uint8_t *a, uint32_t a_length, uint8_t *p, uint32_t p_length,
    uint8_t *m, uint32_t m_length, uint8_t *r, uint32_t r_length);

int tee_ecc_point_check(MT_CIPHER_EC_PARAMS_S *xParams,
	uint8_t *px, uint8_t *py);
int tee_ecdh_gen_keypair(MT_CIPHER_EC_PARAMS_S *xParams,
	uint8_t *priKey, uint8_t *pubKeyX, uint8_t *pubKeyY);
int tee_ecdh_gen_pubkey(MT_CIPHER_EC_PARAMS_S *xParams,
	uint8_t *priKey, uint8_t *pubKeyX, uint8_t *pubKeyY);
int tee_ecdh_gen_sharekey(MT_CIPHER_EC_PARAMS_S *xParams,
	uint8_t *priKeyA, uint8_t *pubKeyBX, uint8_t *pubKeyBY,
	uint8_t *shareKeyX, uint8_t *shareKeyY);

int tee_ecdsa_sign(MT_CIPHER_EC_PARAMS_S *xParams,
	MT_CIPHER_HASH_TYPE_E xHashType, uint8_t *priKey,
	uint8_t *msg, uint32_t mlen, uint8_t *r, uint8_t *s);
int tee_ecdsa_verify(MT_CIPHER_EC_PARAMS_S *xParams,
	MT_CIPHER_HASH_TYPE_E xHashType, uint8_t *pubKeyX, uint8_t *pubKeyY,
	uint8_t *msg, uint32_t mlen, uint8_t *r, uint8_t *s);

int tee_sm2_sign(uint8_t *id, uint8_t *priKey,
	uint8_t *msg, uint32_t mlen, uint8_t *r, uint8_t *s);
int tee_sm2_verify(uint8_t *id, uint8_t *pubKeyX, uint8_t *pubKeyY,
	uint8_t *msg, uint32_t mlen, uint8_t *r, uint8_t *s);
int tee_sm2_encrypt(uint8_t *pubKeyX, uint8_t *pubKeyY,
	uint8_t *msg, uint32_t mlen, uint8_t *out, uint32_t *outlen);
int tee_sm2_decrypt(uint8_t *priKey,
	uint8_t *cipher, uint32_t clen, uint8_t *out, uint32_t *outlen);

int tee_cert_lock(void);
int tee_cert_unlock(void);
int tee_cert_exchange(uint32_t cmds_num,
			CERT_COMMAND_S *p_cmds, uint32_t *p_processed_num);
int tee_cert_reset(void);
int tee_cert_export_key(uint32_t slot_id, uint32_t ext_attr);
int tee_cert_key_ack(void);

#endif /*MSS_TEE_CLIENT_H*/
