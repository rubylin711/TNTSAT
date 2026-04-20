/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "mt_unf_cipher_v2.h"
#include "tee_client_api.h"
#include "mss_test_ta.h"
#include "mss_tee_client.h"

static TEEC_Context g_mss_ctx;
static TEEC_Session g_mss_sess;

/*
static void dump(const char *tag, uint8_t *buffer, uint32_t len) __attribute__((unused));
static void dump(const char *tag, uint8_t *buffer, uint32_t len)
{
    uint32_t i = 0;
    printf("\n%s(0x%lx):[%d]\n", tag, (uint64_t)buffer, len);
    for(i = 0; i < len; i++)
    {
        if((i%16) == 0 && i != 0)
            printf("\n");
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}
*/

static int tee_mss_init(void)
{
	TEEC_Result res;
	uint32_t err_origin;
	TEEC_UUID uuid = TA_MSS_TEST_UUID;
	static int inited = 0;

	if (inited)
		return 0;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &g_mss_ctx);
	if (res != TEEC_SUCCESS)
		printf("%s TEEC_InitializeContext failed with code 0x%x \n", __FUNCTION__, res);
	
	/*
	 * Open TA
	 */
	res = TEEC_OpenSession(&g_mss_ctx, &g_mss_sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		printf("%s TEEC_Opensession failed with code 0x%x origin 0x%x \n",
			__FUNCTION__, res, err_origin);
					
	inited = 1;
	return 0;
}

static int tee_mss_deinit(void) __attribute__((unused));
static int tee_mss_deinit(void)
{
	TEEC_CloseSession(&g_mss_sess);

	TEEC_FinalizeContext(&g_mss_ctx);
	
	return 0;
}

static TEEC_Result tee_mss_InvokeCommand(uint32_t commandID, TEEC_Operation *operation)
{
	TEEC_Result res = TEEC_SUCCESS;
	uint32_t err_origin;

	tee_mss_init();

	res = TEEC_InvokeCommand(&g_mss_sess, commandID, operation, &err_origin);
	if (res != TEEC_SUCCESS){
		printf("%s TEEC_InvokeCommand %d failed with code 0x%x origin 0x%x \n",
			__FUNCTION__, commandID, res, err_origin);
		return res;
	}
	return res;
}

int tee_keyslot_request(uint32_t num, uint32_t *p_keyslot)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	unsigned int *slots = NULL;
	unsigned int bufLen;
	unsigned int i;

	if (!p_keyslot)
		return -1;

	bufLen = num * sizeof(unsigned int);
	slots = (unsigned int *)mt_unf_cipher_malloc(bufLen);
	if (!slots)
		return -1;

	op.paramTypes = PARAM_MSS_KT_REQUEST;

	op.params[0].tmpref.buffer = (void *)slots;
	op.params[0].tmpref.size   = bufLen;

	res = tee_mss_InvokeCommand(TA_MSS_KT_REQUEST, &op);
	if (res == TEEC_SUCCESS) {
		for (i = 0; i < num; i++)
			p_keyslot[i] = slots[i];
	}

	if (slots)
		mt_unf_cipher_free((unsigned char *)slots);
	return res;
}

int tee_keyslot_release(uint32_t keyslot)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	op.paramTypes = PARAM_MSS_KT_RELEASE;

	op.params[0].value.a = keyslot;

	res = tee_mss_InvokeCommand(TA_MSS_KT_RELEASE, &op);
	return res;
}

int tee_keyslot_set(uint32_t keyslot, MT_CIPHER_CTRL_S *p_ctrl,
    uint8_t *key,uint32_t key_size, uint8_t *iv, uint32_t iv_size)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!p_ctrl)
		return -1;

	op.paramTypes = PARAM_MSS_KT_SET;

	op.params[0].value.a = keyslot;

	op.params[1].tmpref.buffer = (void *)p_ctrl;
	op.params[1].tmpref.size   = sizeof(*p_ctrl);

	op.params[2].tmpref.buffer = (void *)key;
	op.params[2].tmpref.size   = key_size;

	op.params[3].tmpref.buffer = (void *)iv;
	op.params[3].tmpref.size   = iv_size;

	res = tee_mss_InvokeCommand(TA_MSS_KT_SET, &op);
	return res;
}

int tee_keyslot_set_ext(uint32_t keyslot, MT_KT_CTRL_S *p_ctrl,
    uint8_t *key,uint32_t key_size, uint8_t *iv, uint32_t iv_size)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!p_ctrl)
		return -1;

	op.paramTypes = PARAM_MSS_KT_SET;

	op.params[0].value.a = keyslot;

	op.params[1].tmpref.buffer = (void *)p_ctrl;
	op.params[1].tmpref.size   = sizeof(*p_ctrl);

	op.params[2].tmpref.buffer = (void *)key;
	op.params[2].tmpref.size   = key_size;

	op.params[3].tmpref.buffer = (void *)iv;
	op.params[3].tmpref.size   = iv_size;

	res = tee_mss_InvokeCommand(TA_MSS_KT_SET_EXT, &op);
	return res;
}

int tee_keyslot_set_iv(uint32_t keyslot, uint8_t *iv, uint32_t iv_size)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	op.paramTypes = PARAM_MSS_KT_SET_IV;

	op.params[0].value.a = keyslot;

	op.params[1].tmpref.buffer = (void *)iv;
	op.params[1].tmpref.size   = iv_size;

	res = tee_mss_InvokeCommand(TA_MSS_KT_SET_IV, &op);
	return res;
}

int tee_crypto_create(MT_CIPHER_CRYPTO_CH_E index, uint32_t *p_handle)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!p_handle)
            return -1;

	op.paramTypes = PARAM_MSS_CRYPTO_CREATE;
	op.params[0].value.a = index;

	res = tee_mss_InvokeCommand(TA_MSS_CRYPTO_CREATE, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	*p_handle = op.params[1].value.a;

	return 0;
}

int tee_crypto_config(uint32_t handle, MT_CIPHER_CTRL_S *p_ctrl, uint32_t key_slot)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!p_ctrl)
		return -1;

	op.paramTypes = PARAM_MSS_CRYPTO_CONFIG;

	op.params[0].value.a = handle;
	op.params[0].value.b = key_slot;

	op.params[1].tmpref.buffer = (void *)p_ctrl;
	op.params[1].tmpref.size   = sizeof(*p_ctrl);

	res = tee_mss_InvokeCommand(TA_MSS_CRYPTO_CONFIG, &op);
	return res;
}

int tee_crypto_process(uint32_t handle, uint8_t *p_src_addr, uint8_t *p_dest_addr, uint32_t length)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!p_src_addr || !p_dest_addr || length == 0)
		return -1;

	op.paramTypes = PARAM_MSS_CRYPTO_PROCESS;

	op.params[0].value.a = handle;

	op.params[1].tmpref.buffer = (void *)p_src_addr;
	op.params[1].tmpref.size   = length;
	op.params[2].tmpref.buffer = (void *)p_dest_addr;
	op.params[2].tmpref.size   = length;

	res = tee_mss_InvokeCommand(TA_MSS_CRYPTO_PROCESS, &op);
	return res;
}

int tee_crypto_destroy(uint32_t handle)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	op.paramTypes = PARAM_MSS_CRYPTO_DESTROY;
	op.params[0].value.a = handle;

	res = tee_mss_InvokeCommand(TA_MSS_CRYPTO_DESTROY, &op);
	return res;
}

int tee_ts_create(MT_CIPHER_CRYPTO_CH_E index, uint32_t *p_handle)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!p_handle)
            return -1;

	op.paramTypes = PARAM_MSS_CRYPTO_TS_CREATE;
	op.params[0].value.a = index;

	res = tee_mss_InvokeCommand(TA_MSS_CRYPTO_TS_CREATE, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	*p_handle = op.params[1].value.a;

	return 0;
}

int tee_ts_destroy(uint32_t handle)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	op.paramTypes = PARAM_MSS_CRYPTO_TS_DESTROY;
	op.params[0].value.a = handle;

	res = tee_mss_InvokeCommand(TA_MSS_CRYPTO_TS_DESTROY, &op);
	return res;
}

int tee_ts_process(uint32_t handle, uint8_t *p_src_addr, uint8_t *p_dest_addr, uint32_t length)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!p_src_addr || !p_dest_addr || length == 0)
		return -1;

	op.paramTypes = PARAM_MSS_CRYPTO_TS_PROCESS;

	op.params[0].value.a = handle;

	op.params[1].tmpref.buffer = (void *)p_src_addr;
	op.params[1].tmpref.size   = length;
	op.params[2].tmpref.buffer = (void *)p_dest_addr;
	op.params[2].tmpref.size   = length;

	res = tee_mss_InvokeCommand(TA_MSS_CRYPTO_TS_PROCESS, &op);
	return res;
}

int tee_ts_config(uint32_t handle,
	MT_CIPHER_CTRL_S *p_ctrl, MT_CIPHER_TS_PARA_S *p_ts_para,
	uint32_t even_key_slot,	uint32_t odd_key_slot)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!p_ctrl)
		return -1;

	op.paramTypes = PARAM_MSS_CRYPTO_TS_CONFIG;

	op.params[0].value.a = handle;

	op.params[1].tmpref.buffer = (void *)p_ctrl;
	op.params[1].tmpref.size   = sizeof(*p_ctrl);

	op.params[2].tmpref.buffer = (void *)p_ts_para;
	op.params[2].tmpref.size   = sizeof(*p_ts_para);

	op.params[3].value.a = even_key_slot;
	op.params[3].value.b = odd_key_slot;

	res = tee_mss_InvokeCommand(TA_MSS_CRYPTO_TS_CONFIG, &op);
	return res;
}

int tee_hash_create(uint32_t hash_type, uint32_t *p_handle)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!p_handle)
            return -1;

	op.paramTypes = PARAM_MSS_HASH_CREATE;
	op.params[0].value.a = hash_type;

	res = tee_mss_InvokeCommand(TA_MSS_HASH_CREATE, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	*p_handle = op.params[1].value.a;

	return 0;
}

int tee_hash_update(uint32_t handle, uint8_t *data, uint32_t length)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!data || length == 0)
            return -1;

	op.paramTypes = PARAM_MSS_HASH_UPDATE;
	op.params[0].value.a = handle;
	op.params[1].tmpref.buffer = (void *)data;
	op.params[1].tmpref.size   = length;

	res = tee_mss_InvokeCommand(TA_MSS_HASH_UPDATE, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

int tee_hash_final(uint32_t handle, uint8_t *digest, uint32_t digest_length)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!digest || digest_length == 0)
            return -1;

	op.paramTypes = PARAM_MSS_HASH_FINAL;
	op.params[0].value.a = handle;
	op.params[1].tmpref.buffer = (void *)digest;
	op.params[1].tmpref.size   = digest_length;

	res = tee_mss_InvokeCommand(TA_MSS_HASH_FINAL, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

int tee_mac_create(uint32_t mac_type,
    MT_CIPHER_HMAC_ATTS_S *p_hmac_attr, uint32_t *p_handle)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!p_handle || !p_hmac_attr)
            return -1;

	op.paramTypes = PARAM_MSS_MAC_CREATE;
	op.params[0].value.a = mac_type;
        op.params[0].value.b = p_hmac_attr->key_len;
	op.params[1].value.a = p_hmac_attr->key_slot[0];
        op.params[1].value.b = p_hmac_attr->key_slot[1];
	op.params[2].tmpref.buffer = (void *)p_hmac_attr->p_hmac_key;
	if (p_hmac_attr->p_hmac_key)
		op.params[2].tmpref.size   = p_hmac_attr->key_len;
	else
		op.params[2].tmpref.size   = 0;

	res = tee_mss_InvokeCommand(TA_MSS_MAC_CREATE, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	*p_handle = op.params[3].value.a;

printf("%s p_handle:0x%x\n", __FUNCTION__, *p_handle);
	return 0;
}

int tee_mac_update(uint32_t handle, uint8_t *data, uint32_t length)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!data || length == 0)
            return -1;

printf("%s handle:0x%x\n", __FUNCTION__,handle);

	op.paramTypes = PARAM_MSS_MAC_UPDATE;
	op.params[0].value.a = handle;
	op.params[1].tmpref.buffer = (void *)data;
	op.params[1].tmpref.size   = length;

	res = tee_mss_InvokeCommand(TA_MSS_MAC_UPDATE, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

int tee_mac_final(uint32_t handle, uint8_t *digest, uint32_t digest_length)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!digest || digest_length == 0)
            return -1;

printf("%s handle:0x%x\n", __FUNCTION__,handle);

	op.paramTypes = PARAM_MSS_MAC_FINAL;
	op.params[0].value.a = handle;
	op.params[1].tmpref.buffer = (void *)digest;
	op.params[1].tmpref.size   = digest_length;

	res = tee_mss_InvokeCommand(TA_MSS_MAC_FINAL, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

int tee_keyladder_create(MT_CIPHER_KEYLADDER_TYPE_E index, uint32_t *p_handle)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!p_handle)
            return -1;

	op.paramTypes = PARAM_MSS_KL_CREATE;
	op.params[0].value.a = index;

	res = tee_mss_InvokeCommand(TA_MSS_KL_CREATE, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	*p_handle = op.params[1].value.a;

	return 0;
}

int tee_keyladder_destroy(uint32_t handle)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	op.paramTypes = PARAM_MSS_KL_DESTROY;
	op.params[0].value.a = handle;

	res = tee_mss_InvokeCommand(TA_MSS_KL_DESTROY, &op);
	return res;
}

int tee_keyladder_set_signature(uint32_t handle, uint8_t *p_sig, uint32_t sigLen)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!p_sig)
		return -1;

	op.paramTypes = PARAM_MSS_KL_SET_SIGNATURE;

	op.params[0].value.a = handle;

	op.params[1].tmpref.buffer = (void *)p_sig;
	op.params[1].tmpref.size   = sigLen;

	res = tee_mss_InvokeCommand(TA_MSS_KL_SET_SIGNATURE, &op);
	return res;
}

int tee_keyladder_start(uint32_t handle, MT_CIPHER_KEYLADDER_SOURCE_E keysrc)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	op.paramTypes = PARAM_MSS_KL_START;

	op.params[0].value.a = handle;
	op.params[0].value.b = keysrc;

	res = tee_mss_InvokeCommand(TA_MSS_KL_START, &op);
	return res;
}

int tee_keyladder_link(uint32_t handle, MT_CIPHER_CTRL_S *p_ctrl, uint8_t *input, uint32_t length)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!p_ctrl || !input)
		return -1;

	op.paramTypes = PARAM_MSS_KL_LINK;

	op.params[0].value.a = handle;

	op.params[1].tmpref.buffer = (void *)p_ctrl;
	op.params[1].tmpref.size   = sizeof(*p_ctrl);
	op.params[2].tmpref.buffer = (void *)input;
	op.params[2].tmpref.size   = length;

	res = tee_mss_InvokeCommand(TA_MSS_KL_LINK, &op);
	return res;
}

int tee_keyladder_end(uint32_t handle, uint32_t keyslot)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	op.paramTypes = PARAM_MSS_KL_END;

	op.params[0].value.a = handle;
	op.params[0].value.b = keyslot;

	res = tee_mss_InvokeCommand(TA_MSS_KL_END, &op);
	return res;
}

typedef struct {
	uint32_t n_length;
	uint8_t n[256];	/* Modulus */

	uint32_t e_length;
	uint8_t e[256];	/* Public exponent */

	uint32_t d_length;
	uint8_t d[256];	/* Private exponent */

	/* Optional CRT parameters (all NULL if unused) */
	uint32_t p_length;
	uint8_t p[128];	/* N = pq */

	uint32_t q_length;
	uint8_t q[128];

	uint32_t qInv_length;
	uint8_t qInv[128];	/* 1/q mod p */

	uint32_t dp_length;
	uint8_t dp[128];	/* d mod (p-1) */

	uint32_t dq_length;
	uint8_t dq[128];	/* d mod (q-1) */
}MSS_RSA_Keypair;

typedef struct {
	uint32_t n_length;
	uint8_t n[256];	/* Modulus */

	uint32_t e_length;
	uint8_t e[256];	/* Public exponent */
}MSS_RSA_PubKey;

static TEEC_Result tee_rsa_public_key_config(
	struct rsa_public_key *key, MSS_RSA_PubKey *msskey)
{
	if (!key || !msskey)
		return TEEC_ERROR_BAD_PARAMETERS;

	if (key->n_length > 256 || key->e_length > 256)
		return TEEC_ERROR_BAD_PARAMETERS;

	memset(msskey, 0, sizeof(*msskey));

	if (key->n) {
		memcpy(msskey->n, key->n,  key->n_length);
		msskey->n_length = key->n_length;
	}
	if (key->e) {
		memcpy(msskey->e, key->e,  key->e_length);
		msskey->e_length = key->e_length;
	}

	return TEEC_SUCCESS;
}

static TEEC_Result tee_rsa_private_key_config(
	struct rsa_keypair *key, MSS_RSA_Keypair *msskey)
{
	if (!key || !msskey)
		return TEEC_ERROR_BAD_PARAMETERS;

	if (key->n_length > 256
		|| key->e_length > 256
		|| key->d_length > 256
		|| key->p_length > 128
		|| key->q_length > 128
		|| key->dp_length > 128
		|| key->dq_length > 128
		|| key->qInv_length > 128)
		return TEEC_ERROR_BAD_PARAMETERS;

	memset(msskey, 0, sizeof(*msskey));

	if (key->n) {
		memcpy(msskey->n, key->n,  key->n_length);
		msskey->n_length = key->n_length;
	}
	if (key->e) {
		memcpy(msskey->e, key->e,  key->e_length);
		msskey->e_length = key->e_length;
	}
	if (key->d) {
		memcpy(msskey->d, key->d,  key->d_length);
		msskey->d_length = key->d_length;
	}
	if (key->p) {
		memcpy(msskey->p, key->p,  key->p_length);
		msskey->p_length = key->p_length;
	}
	if (key->q) {
		memcpy(msskey->q, key->q,  key->q_length);
		msskey->q_length = key->q_length;
	}
	if (key->dp) {
		memcpy(msskey->dp, key->dp,  key->dp_length);
		msskey->dp_length = key->dp_length;
	}
	if (key->dq) {
		memcpy(msskey->dq, key->dq,  key->dq_length);
		msskey->dq_length = key->dq_length;
	}
	if (key->qInv) {
		memcpy(msskey->qInv, key->qInv,  key->qInv_length);
		msskey->qInv_length = key->qInv_length;
	}

	return TEEC_SUCCESS;
}

int tee_rsa_public_encrypt(struct rsa_public_key *key,
	uint8_t *src, uint32_t src_len, uint8_t *dst, MT_RSA_ALG algo)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_RSA_PubKey *msskey = NULL;

	if (!key || !src || !dst)
            return -1;

	op.paramTypes = PARAM_MSS_RSA_PUBLIC_ENCRYPT;

	msskey = (MSS_RSA_PubKey *)mt_unf_cipher_malloc(sizeof(MSS_RSA_PubKey));
	if (!msskey)
		return -1;

	res = tee_rsa_public_key_config(key, msskey);
	if (res != TEEC_SUCCESS) {
		mt_unf_cipher_free((uint8_t *)msskey);
		return -1;
	}

	op.params[0].tmpref.buffer = (void *)msskey;
	op.params[0].tmpref.size   = sizeof(*msskey);
	op.params[1].tmpref.buffer = (void *)src;
	op.params[1].tmpref.size   = src_len;
	op.params[2].tmpref.buffer = (void *)dst;
	op.params[2].tmpref.size   = msskey->n_length;
	op.params[3].value.a = algo;

	res = tee_mss_InvokeCommand(TA_MSS_RSA_PUBLIC_ENCRYPT, &op);
	mt_unf_cipher_free((uint8_t *)msskey);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

int tee_rsa_private_decrypt(struct rsa_keypair *key,
	uint8_t *src, uint32_t src_len, uint8_t *dst, uint32_t *dst_len, MT_RSA_ALG algo)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_RSA_Keypair *msskey = NULL;

	if (!key || !src || !dst)
            return -1;

	op.paramTypes = PARAM_MSS_RSA_PRIVATE_DECRYPT;

	msskey = (MSS_RSA_Keypair *)mt_unf_cipher_malloc(sizeof(MSS_RSA_Keypair));
	if (!msskey)
		return -1;

	res = tee_rsa_private_key_config(key, msskey);
	if (res != TEEC_SUCCESS) {
		mt_unf_cipher_free((uint8_t *)msskey);
		return -1;
	}

	op.params[0].tmpref.buffer = (void *)msskey;
	op.params[0].tmpref.size   = sizeof(*msskey);
	op.params[1].tmpref.buffer = (void *)src;
	op.params[1].tmpref.size   = src_len;
	op.params[2].tmpref.buffer = (void *)dst;
	op.params[2].tmpref.size   = msskey->n_length;
	op.params[3].value.a = algo;

	res = tee_mss_InvokeCommand(TA_MSS_RSA_PRIVATE_DECRYPT, &op);
	mt_unf_cipher_free((uint8_t *)msskey);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	*dst_len = op.params[2].tmpref.size;
	return 0;
}

int tee_rsa_private_encrypt(struct rsa_keypair *key,
	uint8_t *src, uint32_t src_len, uint8_t *dst, MT_RSA_ALG algo)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_RSA_Keypair *msskey = NULL;

	if (!key || !src || !dst)
            return -1;

	op.paramTypes = PARAM_MSS_RSA_PRIVATE_ENCRYPT;

	msskey = (MSS_RSA_Keypair *)mt_unf_cipher_malloc(sizeof(MSS_RSA_Keypair));
	if (!msskey)
		return -1;

	res = tee_rsa_private_key_config(key, msskey);
	if (res != TEEC_SUCCESS) {
		mt_unf_cipher_free((uint8_t *)msskey);
		return -1;
	}

	op.params[0].tmpref.buffer = (void *)msskey;
	op.params[0].tmpref.size   = sizeof(*msskey);
	op.params[1].tmpref.buffer = (void *)src;
	op.params[1].tmpref.size   = src_len;
	op.params[2].tmpref.buffer = (void *)dst;
	op.params[2].tmpref.size   = msskey->n_length;
	op.params[3].value.a = algo;

	res = tee_mss_InvokeCommand(TA_MSS_RSA_PRIVATE_ENCRYPT, &op);
	mt_unf_cipher_free((uint8_t *)msskey);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

int tee_rsa_public_decrypt(struct rsa_public_key *key,
	uint8_t *src, uint32_t src_len, uint8_t *dst, uint32_t *dst_len, MT_RSA_ALG algo)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_RSA_PubKey *msskey = NULL;

	if (!key || !src || !dst)
            return -1;

	op.paramTypes = PARAM_MSS_RSA_PUBLIC_DECRYPT;

	msskey = (MSS_RSA_PubKey *)mt_unf_cipher_malloc(sizeof(MSS_RSA_PubKey));
	if (!msskey)
		return -1;

	res = tee_rsa_public_key_config(key, msskey);
	if (res != TEEC_SUCCESS) {
		mt_unf_cipher_free((uint8_t *)msskey);
		return -1;
	}

	op.params[0].tmpref.buffer = (void *)msskey;
	op.params[0].tmpref.size   = sizeof(*msskey);
	op.params[1].tmpref.buffer = (void *)src;
	op.params[1].tmpref.size   = src_len;
	op.params[2].tmpref.buffer = (void *)dst;
	op.params[2].tmpref.size   = msskey->n_length;
	op.params[3].value.a = algo;

	res = tee_mss_InvokeCommand(TA_MSS_RSA_PUBLIC_DECRYPT, &op);
	mt_unf_cipher_free((uint8_t *)msskey);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	*dst_len = op.params[2].tmpref.size;
	return 0;
}

int tee_rsa_sign(struct rsa_keypair *key, MT_RSA_ALG algo,
	uint8_t*msg, uint32_t mlen, uint8_t *sign, uint32_t *slen)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_RSA_Keypair *msskey = NULL;

	if (!key || !msg || !sign || !slen)
            return -1;

	op.paramTypes = PARAM_MSS_RSA_SIGN;

	msskey = (MSS_RSA_Keypair *)mt_unf_cipher_malloc(sizeof(MSS_RSA_Keypair));
	if (!msskey)
		return -1;

	res = tee_rsa_private_key_config(key, msskey);
	if (res != TEEC_SUCCESS) {
		mt_unf_cipher_free((uint8_t *)msskey);
		return -1;
	}

	op.params[0].tmpref.buffer = (void *)msskey;
	op.params[0].tmpref.size   = sizeof(*msskey);
	op.params[1].tmpref.buffer = (void *)msg;
	op.params[1].tmpref.size   = mlen;
	op.params[2].tmpref.buffer = (void *)sign;
	op.params[2].tmpref.size   = msskey->n_length;
	op.params[3].value.a = algo;

	res = tee_mss_InvokeCommand(TA_MSS_RSA_SIGN, &op);
	mt_unf_cipher_free((uint8_t *)msskey);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	*slen = op.params[2].tmpref.size;
	return 0;
}

int tee_rsa_verify(struct rsa_public_key *key, MT_RSA_ALG algo,
	uint8_t*msg, uint32_t mlen, uint8_t *sign, uint32_t slen)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_RSA_PubKey *msskey = NULL;

	if (!key || !msg || !sign)
            return -1;

	op.paramTypes = PARAM_MSS_RSA_VERIFY;

	msskey = (MSS_RSA_PubKey *)mt_unf_cipher_malloc(sizeof(MSS_RSA_PubKey));
	if (!msskey)
		return -1;

	res = tee_rsa_public_key_config(key, msskey);
	if (res != TEEC_SUCCESS) {
		mt_unf_cipher_free((uint8_t *)msskey);
		return -1;
	}

	op.params[0].tmpref.buffer = (void *)msskey;
	op.params[0].tmpref.size   = sizeof(*msskey);
	op.params[1].tmpref.buffer = (void *)msg;
	op.params[1].tmpref.size   = mlen;
	op.params[2].tmpref.buffer = (void *)sign;
	op.params[2].tmpref.size   = slen;
	op.params[3].value.a = algo;

	res = tee_mss_InvokeCommand(TA_MSS_RSA_VERIFY, &op);
	mt_unf_cipher_free((uint8_t *)msskey);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

// r = d mod m
int tee_mod_mod(uint8_t *d, uint32_t d_length, uint8_t *m, uint32_t m_length, uint8_t *r, uint32_t r_length)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!d || d_length == 0)
            return -1;

	if (!m || m_length == 0)
            return -1;

	if (!r || r_length == 0)
            return -1;

	op.paramTypes = PARAM_MSS_MOD_MOD;

	op.params[0].tmpref.buffer = (void *)d;
	op.params[0].tmpref.size   = d_length;

	op.params[1].tmpref.buffer = (void *)m;
	op.params[1].tmpref.size   = m_length;

	op.params[2].tmpref.buffer = (void *)r;
	op.params[2].tmpref.size   = r_length;

	res = tee_mss_InvokeCommand(TA_MSS_MOD_MOD, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

// r = (a * b) mod m
int tee_mod_mul(uint8_t *a, uint32_t a_length, uint8_t *b, uint32_t b_length,
    uint8_t *m, uint32_t m_length, uint8_t *r, uint32_t r_length)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!a || a_length == 0)
            return -1;

	if (!b || b_length == 0)
            return -1;

	if (!m || m_length == 0)
            return -1;

	if (!r || r_length == 0)
            return -1;

	op.paramTypes = PARAM_MSS_MOD_MUL;

	op.params[0].tmpref.buffer = (void *)a;
	op.params[0].tmpref.size   = a_length;

	op.params[1].tmpref.buffer = (void *)b;
	op.params[1].tmpref.size   = b_length;

	op.params[2].tmpref.buffer = (void *)m;
	op.params[2].tmpref.size   = m_length;

	op.params[3].tmpref.buffer = (void *)r;
	op.params[3].tmpref.size   = r_length;

	res = tee_mss_InvokeCommand(TA_MSS_MOD_MUL, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

// r = (a + b) mod m
int tee_mod_add(uint8_t *a, uint32_t a_length, uint8_t *b, uint32_t b_length,
    uint8_t *m, uint32_t m_length, uint8_t *r, uint32_t r_length)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!a || a_length == 0)
            return -1;

	if (!b || b_length == 0)
            return -1;

	if (!m || m_length == 0)
            return -1;

	if (!r || r_length == 0)
            return -1;

	op.paramTypes = PARAM_MSS_MOD_ADD;

	op.params[0].tmpref.buffer = (void *)a;
	op.params[0].tmpref.size   = a_length;

	op.params[1].tmpref.buffer = (void *)b;
	op.params[1].tmpref.size   = b_length;

	op.params[2].tmpref.buffer = (void *)m;
	op.params[2].tmpref.size   = m_length;

	op.params[3].tmpref.buffer = (void *)r;
	op.params[3].tmpref.size   = r_length;

	res = tee_mss_InvokeCommand(TA_MSS_MOD_ADD, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

// r = (a - b) mod m
int tee_mod_sub(uint8_t *a, uint32_t a_length, uint8_t *b, uint32_t b_length,
    uint8_t *m, uint32_t m_length, uint8_t *r, uint32_t r_length)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!a || a_length == 0)
            return -1;

	if (!b || b_length == 0)
            return -1;

	if (!m || m_length == 0)
            return -1;

	if (!r || r_length == 0)
            return -1;

	op.paramTypes = PARAM_MSS_MOD_SUB;

	op.params[0].tmpref.buffer = (void *)a;
	op.params[0].tmpref.size   = a_length;

	op.params[1].tmpref.buffer = (void *)b;
	op.params[1].tmpref.size   = b_length;

	op.params[2].tmpref.buffer = (void *)m;
	op.params[2].tmpref.size   = m_length;

	op.params[3].tmpref.buffer = (void *)r;
	op.params[3].tmpref.size   = r_length;

	res = tee_mss_InvokeCommand(TA_MSS_MOD_SUB, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

// r = (1/d) mod m
int tee_mod_inv(uint8_t *d, uint32_t d_length, uint8_t *m, uint32_t m_length, uint8_t *r, uint32_t r_length)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!d || d_length == 0)
            return -1;

	if (!m || m_length == 0)
            return -1;

	if (!r || r_length == 0)
            return -1;

	op.paramTypes = PARAM_MSS_MOD_INV;

	op.params[0].tmpref.buffer = (void *)d;
	op.params[0].tmpref.size   = d_length;

	op.params[1].tmpref.buffer = (void *)m;
	op.params[1].tmpref.size   = m_length;

	op.params[2].tmpref.buffer = (void *)r;
	op.params[2].tmpref.size   = r_length;

	res = tee_mss_InvokeCommand(TA_MSS_MOD_INV, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

// r = (a^p) mod m
int tee_mod_exp(uint8_t *a, uint32_t a_length, uint8_t *p, uint32_t p_length,
    uint8_t *m, uint32_t m_length, uint8_t *r, uint32_t r_length)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!a || a_length == 0)
            return -1;

	if (!p || p_length == 0)
            return -1;

	if (!m || m_length == 0)
            return -1;

	if (!r || r_length == 0)
            return -1;

	op.paramTypes = PARAM_MSS_MOD_EXP;

	op.params[0].tmpref.buffer = (void *)a;
	op.params[0].tmpref.size   = a_length;

	op.params[1].tmpref.buffer = (void *)p;
	op.params[1].tmpref.size   = p_length;

	op.params[2].tmpref.buffer = (void *)m;
	op.params[2].tmpref.size   = m_length;

	op.params[3].tmpref.buffer = (void *)r;
	op.params[3].tmpref.size   = r_length;

	res = tee_mss_InvokeCommand(TA_MSS_MOD_EXP, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

typedef struct {
	uint8_t q[64];
	uint8_t a[64];
	uint8_t b[64];
	uint8_t n[64];
	uint8_t h[64];
	uint8_t GX[64];
	uint8_t GY[64];
	uint32_t key_size;
	uint32_t hashType;
} MSS_ECC_Params;

typedef struct {
	uint8_t x[64];
	uint8_t y[64];
} MSS_ECC_Point;

static int mt_mss_eccparams_config(
	MT_CIPHER_EC_PARAMS_S *params, MSS_ECC_Params *mss_params)
{
	if (!params || !mss_params)
		return -1;

	memset(mss_params, 0, sizeof(*mss_params));
	mss_params->key_size = params->keySize;

	if (params->keySize > 64)
		return -1;

	if (params->q)
		memcpy(mss_params->q, params->q, params->keySize);
	if (params->a)
		memcpy(mss_params->a, params->a, params->keySize);
	if (params->b)
		memcpy(mss_params->b, params->b, params->keySize);
	if (params->n)
		memcpy(mss_params->n, params->n, params->keySize);
	if (params->h)
		memcpy(mss_params->h, params->h, params->keySize);
	if (params->GX)
		memcpy(mss_params->GX, params->GX, params->keySize);
	if (params->GY)
		memcpy(mss_params->GY, params->GY, params->keySize);

	return 0;
}

int tee_ecc_point_check(MT_CIPHER_EC_PARAMS_S *xParams,
	uint8_t *px, uint8_t *py)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_ECC_Params *eccParam = NULL;
	MSS_ECC_Point *eccPoint = NULL;

	if (!xParams || !px || !py)
            return -1;

	eccParam = (MSS_ECC_Params *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Params));
	if (!eccParam)
		goto EXIT;

	eccPoint = (MSS_ECC_Point *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Point));
	if (!eccPoint)
		goto EXIT;

	if (mt_mss_eccparams_config(xParams, eccParam) != 0)
		goto EXIT;

	memcpy(eccPoint->x, px, eccParam->key_size);
	memcpy(eccPoint->y, py, eccParam->key_size);

	op.paramTypes = PARAM_MSS_ECC_POINT_CHECK;

	op.params[0].tmpref.buffer = (void *)eccParam;
	op.params[0].tmpref.size   = sizeof(MSS_ECC_Params);
	op.params[1].tmpref.buffer = (void *)eccPoint;
	op.params[1].tmpref.size   = sizeof(MSS_ECC_Point);

	res = tee_mss_InvokeCommand(TA_MSS_ECC_POINT_CHECK, &op);

EXIT:
	if (eccPoint)
		mt_unf_cipher_free((uint8_t *)eccPoint);
	if (eccParam)
		mt_unf_cipher_free((uint8_t *)eccParam);

	return res;
}

int tee_ecdh_gen_keypair(MT_CIPHER_EC_PARAMS_S *xParams,
	uint8_t *priKey, uint8_t *pubKeyX, uint8_t *pubKeyY)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_ECC_Params *eccParam = NULL;
	MSS_ECC_Point *pubPoint = NULL;

	if (!xParams || !priKey || !pubKeyX || !pubKeyY)
            return -1;

	eccParam = (MSS_ECC_Params *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Params));
	if (!eccParam)
		goto EXIT;

	pubPoint = (MSS_ECC_Point *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Point));
	if (!pubPoint)
		goto EXIT;

	if (mt_mss_eccparams_config(xParams, eccParam) != 0)
		goto EXIT;

	op.paramTypes = PARAM_MSS_ECDH_GEN_KEYPAIR;

	op.params[0].tmpref.buffer = (void *)eccParam;
	op.params[0].tmpref.size   = sizeof(MSS_ECC_Params);
	op.params[1].tmpref.buffer = (void *)priKey;
	op.params[1].tmpref.size   = eccParam->key_size;
	op.params[2].tmpref.buffer = (void *)pubPoint;
	op.params[2].tmpref.size   = sizeof(MSS_ECC_Point);

	res = tee_mss_InvokeCommand(TA_MSS_ECDH_GEN_KEYPAIR, &op);
	if (res == TEEC_SUCCESS) {
		memcpy(pubKeyX, pubPoint->x, eccParam->key_size);
		memcpy(pubKeyY, pubPoint->y, eccParam->key_size);
	}

EXIT:
	if (pubPoint)
		mt_unf_cipher_free((uint8_t *)pubPoint);
	if (eccParam)
		mt_unf_cipher_free((uint8_t *)eccParam);

	return res;
}

int tee_ecdh_gen_pubkey(MT_CIPHER_EC_PARAMS_S *xParams,
	uint8_t *priKey, uint8_t *pubKeyX, uint8_t *pubKeyY)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_ECC_Params *eccParam = NULL;
	MSS_ECC_Point *pubPoint = NULL;

	if (!xParams || !priKey || !pubKeyX || !pubKeyY)
            return -1;

	eccParam = (MSS_ECC_Params *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Params));
	if (!eccParam)
		goto EXIT;

	pubPoint = (MSS_ECC_Point *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Point));
	if (!pubPoint)
		goto EXIT;

	if (mt_mss_eccparams_config(xParams, eccParam) != 0)
		return -1;

	op.paramTypes = PARAM_MSS_ECDH_GEN_PUBKEY;

	op.params[0].tmpref.buffer = (void *)eccParam;
	op.params[0].tmpref.size   = sizeof(MSS_ECC_Params);
	op.params[1].tmpref.buffer = (void *)priKey;
	op.params[1].tmpref.size   = eccParam->key_size;
	op.params[2].tmpref.buffer = (void *)pubPoint;
	op.params[2].tmpref.size   = sizeof(MSS_ECC_Point);

	res = tee_mss_InvokeCommand(TA_MSS_ECDH_GEN_PUBKEY, &op);
	if (res == TEEC_SUCCESS) {
		memcpy(pubKeyX, pubPoint->x, eccParam->key_size);
		memcpy(pubKeyY, pubPoint->y, eccParam->key_size);
	}

EXIT:
	if (pubPoint)
		mt_unf_cipher_free((uint8_t *)pubPoint);
	if (eccParam)
		mt_unf_cipher_free((uint8_t *)eccParam);

	return res;
}

int tee_ecdh_gen_sharekey(MT_CIPHER_EC_PARAMS_S *xParams,
	uint8_t *priKeyA, uint8_t *pubKeyBX, uint8_t *pubKeyBY,
	uint8_t *shareKeyX, uint8_t *shareKeyY)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_ECC_Params *eccParam = NULL;
	MSS_ECC_Point *pubPointB = NULL;
	MSS_ECC_Point *sharePoint = NULL;

	if (!xParams || !priKeyA || !pubKeyBX || !pubKeyBY)
		return -1;
	if (!shareKeyX || !shareKeyY)
		return -1;

	eccParam = (MSS_ECC_Params *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Params));
	if (!eccParam)
		goto EXIT;

	pubPointB = (MSS_ECC_Point *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Point));
	if (!pubPointB)
		goto EXIT;

	sharePoint = (MSS_ECC_Point *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Point));
	if (!sharePoint)
		goto EXIT;

	if (mt_mss_eccparams_config(xParams, eccParam) != 0)
		return -1;

	memcpy(pubPointB->x, pubKeyBX, eccParam->key_size);
	memcpy(pubPointB->y, pubKeyBY, eccParam->key_size);

	op.paramTypes = PARAM_MSS_ECDH_GEN_SHAREKEY;

	op.params[0].tmpref.buffer = (void *)eccParam;
	op.params[0].tmpref.size   = sizeof(MSS_ECC_Params);
	op.params[1].tmpref.buffer = (void *)priKeyA;
	op.params[1].tmpref.size   = eccParam->key_size;
	op.params[2].tmpref.buffer = (void *)pubPointB;
	op.params[2].tmpref.size   = sizeof(MSS_ECC_Point);
	op.params[3].tmpref.buffer = (void *)sharePoint;
	op.params[3].tmpref.size   = sizeof(MSS_ECC_Point);

	res = tee_mss_InvokeCommand(TA_MSS_ECDH_GEN_SHAREKEY, &op);
	if (res == TEEC_SUCCESS) {
		memcpy(shareKeyX, sharePoint->x, eccParam->key_size);
		memcpy(shareKeyY, sharePoint->y, eccParam->key_size);
	}

EXIT:
	if (pubPointB)
		mt_unf_cipher_free((uint8_t *)pubPointB);
	if (sharePoint)
		mt_unf_cipher_free((uint8_t *)sharePoint);
	if (eccParam)
		mt_unf_cipher_free((uint8_t *)eccParam);

	return res;
}

int tee_ecdsa_sign(MT_CIPHER_EC_PARAMS_S *xParams,
	MT_CIPHER_HASH_TYPE_E xHashType, uint8_t *priKey,
	uint8_t *msg, uint32_t mlen, uint8_t *r, uint8_t *s)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_ECC_Params *eccParam = NULL;
	MSS_ECC_Point *sigPoint = NULL;

	if (!xParams || !priKey || !msg || !r || !s)
            return -1;

	eccParam = (MSS_ECC_Params *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Params));
	if (!eccParam)
		goto EXIT;

	sigPoint = (MSS_ECC_Point *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Point));
	if (!sigPoint)
		goto EXIT;

	if (mt_mss_eccparams_config(xParams, eccParam) != 0)
		return -1;
	eccParam->hashType = xHashType;

	op.paramTypes = PARAM_MSS_ECDSA_SIGN;

	op.params[0].tmpref.buffer = (void *)eccParam;
	op.params[0].tmpref.size   = sizeof(MSS_ECC_Params);
	op.params[1].tmpref.buffer = (void *)priKey;
	op.params[1].tmpref.size   = eccParam->key_size;
	op.params[2].tmpref.buffer = (void *)msg;
	op.params[2].tmpref.size   = mlen;
	op.params[3].tmpref.buffer = (void *)sigPoint;
	op.params[3].tmpref.size   = sizeof(MSS_ECC_Point);

	res = tee_mss_InvokeCommand(TA_MSS_ECDSA_SIGN, &op);
	if (res == TEEC_SUCCESS) {
		memcpy(r, sigPoint->x, eccParam->key_size);
		memcpy(s, sigPoint->y, eccParam->key_size);
	}

EXIT:
	if (sigPoint)
		mt_unf_cipher_free((uint8_t *)sigPoint);
	if (eccParam)
		mt_unf_cipher_free((uint8_t *)eccParam);

	return res;
}

int tee_ecdsa_verify(MT_CIPHER_EC_PARAMS_S *xParams,
	MT_CIPHER_HASH_TYPE_E xHashType, uint8_t *pubKeyX, uint8_t *pubKeyY,
	uint8_t *msg, uint32_t mlen, uint8_t *r, uint8_t *s)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_ECC_Params *eccParam = NULL;
	MSS_ECC_Point *pubPoint = NULL;
	MSS_ECC_Point *sigPoint = NULL;

	if (!xParams || !msg || !pubKeyX || !pubKeyY || !r || !s)
            return -1;

	eccParam = (MSS_ECC_Params *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Params));
	if (!eccParam)
		return -1;

	pubPoint = (MSS_ECC_Point *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Point));
	if (!pubPoint)
		goto EXIT;

	sigPoint = (MSS_ECC_Point *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Point));
	if (!sigPoint)
		goto EXIT;

	if (mt_mss_eccparams_config(xParams, eccParam) != 0)
		return -1;
	eccParam->hashType = xHashType;

	memcpy(pubPoint->x, pubKeyX, eccParam->key_size);
	memcpy(pubPoint->y, pubKeyY, eccParam->key_size);

	memcpy(sigPoint->x, r, eccParam->key_size);
	memcpy(sigPoint->y, s, eccParam->key_size);

	op.paramTypes = PARAM_MSS_ECDSA_VERIFY;

	op.params[0].tmpref.buffer = (void *)eccParam;
	op.params[0].tmpref.size   = sizeof(MSS_ECC_Params);
	op.params[1].tmpref.buffer = (void *)pubPoint;
	op.params[1].tmpref.size   = sizeof(MSS_ECC_Point);
	op.params[2].tmpref.buffer = (void *)msg;
	op.params[2].tmpref.size   = mlen;
	op.params[3].tmpref.buffer = (void *)sigPoint;
	op.params[3].tmpref.size   = sizeof(MSS_ECC_Point);

	res = tee_mss_InvokeCommand(TA_MSS_ECDSA_VERIFY, &op);

EXIT:
	if (pubPoint)
		mt_unf_cipher_free((uint8_t *)pubPoint);
	if (sigPoint)
		mt_unf_cipher_free((uint8_t *)sigPoint);
	if (eccParam)
		mt_unf_cipher_free((uint8_t *)eccParam);

	return res;
}

int tee_sm2_sign(uint8_t *id, uint8_t *priKey,
	uint8_t *msg, uint32_t mlen, uint8_t *r, uint8_t *s)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_ECC_Point *sigPoint = NULL;

	if (!id || !priKey || !msg || !r || !s)
            return -1;

	sigPoint = (MSS_ECC_Point *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Point));
	if (!sigPoint)
		goto EXIT;

	op.paramTypes = PARAM_MSS_SM2_SIGN;

	op.params[0].tmpref.buffer = (void *)id;
	op.params[0].tmpref.size   = 16;
	op.params[1].tmpref.buffer = (void *)priKey;
	op.params[1].tmpref.size   = 32;
	op.params[2].tmpref.buffer = (void *)msg;
	op.params[2].tmpref.size   = mlen;
	op.params[3].tmpref.buffer = (void *)sigPoint;
	op.params[3].tmpref.size   = sizeof(MSS_ECC_Point);

	res = tee_mss_InvokeCommand(TA_MSS_SM2_SIGN, &op);
	if (res == TEEC_SUCCESS) {
		memcpy(r, sigPoint->x, 32);
		memcpy(s, sigPoint->y, 32);
	}

EXIT:
	if (sigPoint)
		mt_unf_cipher_free((uint8_t *)sigPoint);

	return res;
}

int tee_sm2_verify(uint8_t *id, uint8_t *pubKeyX, uint8_t *pubKeyY,
	uint8_t *msg, uint32_t mlen, uint8_t *r, uint8_t *s)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_ECC_Point *pubPoint = NULL;
	MSS_ECC_Point *sigPoint = NULL;

	if (!id || !pubKeyX || !pubKeyY || !msg || !r || !s)
            return -1;

	pubPoint = (MSS_ECC_Point *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Point));
	if (!pubPoint)
		goto EXIT;

	sigPoint = (MSS_ECC_Point *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Point));
	if (!sigPoint)
		goto EXIT;

	memcpy(pubPoint->x, pubKeyX, 32);
	memcpy(pubPoint->y, pubKeyY, 32);

	memcpy(sigPoint->x, r, 32);
	memcpy(sigPoint->y, s, 32);

	op.paramTypes = PARAM_MSS_SM2_VERIFY;

	op.params[0].tmpref.buffer = (void *)id;
	op.params[0].tmpref.size   = 16;
	op.params[1].tmpref.buffer = (void *)pubPoint;
	op.params[1].tmpref.size   = sizeof(MSS_ECC_Point);
	op.params[2].tmpref.buffer = (void *)msg;
	op.params[2].tmpref.size   = mlen;
	op.params[3].tmpref.buffer = (void *)sigPoint;
	op.params[3].tmpref.size   = sizeof(MSS_ECC_Point);

	res = tee_mss_InvokeCommand(TA_MSS_SM2_VERIFY, &op);

EXIT:
	if (sigPoint)
		mt_unf_cipher_free((uint8_t *)sigPoint);
	if (pubPoint)
		mt_unf_cipher_free((uint8_t *)pubPoint);

	return res;
}

int tee_sm2_encrypt(uint8_t *pubKeyX, uint8_t *pubKeyY,
	uint8_t *msg, uint32_t mlen, uint8_t *out, uint32_t *outlen)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;
	MSS_ECC_Point *pubPoint = NULL;

	if (!msg || !pubKeyX || !pubKeyY || !out || !outlen)
            return -1;

	pubPoint = (MSS_ECC_Point *)mt_unf_cipher_malloc(sizeof(MSS_ECC_Point));
	if (!pubPoint)
		goto EXIT;

	memcpy(pubPoint->x, pubKeyX, 32);
	memcpy(pubPoint->y, pubKeyY, 32);

	op.paramTypes = PARAM_MSS_SM2_ENCRYPT;

	op.params[0].tmpref.buffer = (void *)pubPoint;
	op.params[0].tmpref.size   = sizeof(MSS_ECC_Point);;
	op.params[1].tmpref.buffer = (void *)msg;
	op.params[1].tmpref.size   = mlen;
	op.params[2].tmpref.buffer = (void *)out;
	op.params[2].tmpref.size   = *outlen;

	res = tee_mss_InvokeCommand(TA_MSS_SM2_ENCRYPT, &op);
	if (res == TEEC_SUCCESS){
		*outlen = op.params[2].tmpref.size;
	}

EXIT:
	if (pubPoint)
		mt_unf_cipher_free((uint8_t *)pubPoint);

	return res;
}

int tee_sm2_decrypt(uint8_t *priKey,
	uint8_t *cipher, uint32_t clen, uint8_t *out, uint32_t *outlen)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	if (!priKey || !cipher || !out || !outlen)
            return -1;

	op.paramTypes = PARAM_MSS_SM2_DECRYPT;

	op.params[0].tmpref.buffer = (void *)priKey;
	op.params[0].tmpref.size   = 32;
	op.params[1].tmpref.buffer = (void *)cipher;
	op.params[1].tmpref.size   = clen;
	op.params[2].tmpref.buffer = (void *)out;
	op.params[2].tmpref.size   = *outlen;

	res = tee_mss_InvokeCommand(TA_MSS_SM2_DECRYPT, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	*outlen = op.params[2].tmpref.size;

	return 0;
}


int tee_cert_lock(void)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	op.paramTypes = PARAM_MSS_CERT_LOCK;

	res = tee_mss_InvokeCommand(TA_MSS_CERT_LOCK, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}
	return 0;
}

int tee_cert_unlock(void)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	op.paramTypes = PARAM_MSS_CERT_UNLOCK;

	res = tee_mss_InvokeCommand(TA_MSS_CERT_UNLOCK, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

int tee_cert_exchange(uint32_t cmds_num,
			CERT_COMMAND_S *p_cmds, uint32_t *p_processed_num)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	op.paramTypes = PARAM_MSS_CERT_EXCHANGE;
	op.params[0].tmpref.buffer = (void *)p_cmds;
	op.params[0].tmpref.size   = sizeof(CERT_COMMAND_S) * cmds_num;

	res = tee_mss_InvokeCommand(TA_MSS_CERT_EXCHANGE, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	*p_processed_num = op.params[1].value.a;
	return 0;
}

int tee_cert_reset(void)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	op.paramTypes = PARAM_MSS_CERT_RESET;

	res = tee_mss_InvokeCommand(TA_MSS_CERT_RESET, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

int tee_cert_export_key(uint32_t slot_id, uint32_t ext_attr)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	op.paramTypes = PARAM_MSS_CERT_EXPORT;
	op.params[0].value.a = slot_id;
	op.params[0].value.b = ext_attr;

	res = tee_mss_InvokeCommand(TA_MSS_CERT_EXPORT, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

int tee_cert_key_ack(void)
{
	TEEC_Operation op = { 0 };
	TEEC_Result res = 0;

	op.paramTypes = PARAM_MSS_CERT_ACK;

	res = tee_mss_InvokeCommand(TA_MSS_CERT_ACK, &op);
	if (res != TEEC_SUCCESS){
		return -1;
	}

	return 0;
}

