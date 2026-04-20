/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_MSS_RSA_H__
#define __DRV_MSS_RSA_H__

#define MT_RSA_ERR_BAD_PADDING (-100)

/*!
 * Cipher error code
 */
enum _MT_CIPHER_ERR_E {
    MT_CIPHER_ERR_NOT_INITIALIZED       = -255,
    MT_CIPHER_ERR_FEATURE_NOT_SUPPORT,
    MT_CIPHER_ERR_INVALID_HANDLE,
    MT_CIPHER_ERR_MULTIPLE_HANDLE,              /*Only one handle is supported at a time*/
    MT_CIPHER_ERR_BAD_PARAMETERS,
    MT_CIPHER_ERR_BUFFER_ALLOCATE_FAILED,

    MT_CIPHER_ERR_KT_REQUEST_FAILED,        /*request/release key slot failed*/
    MT_CIPHER_ERR_KT_RELEASE_FAILED,
    MT_CIPHER_ERR_KT_SET_ERR,

    MT_CIPHER_ERR_CRYPTO_CREATE,
    MT_CIPHER_ERR_CRYPTO_CONFIG,
    MT_CIPHER_ERR_CRYPTO_PROCESS,

    MT_CIPHER_ERR_RSA_CREATE,
    MT_CIPHER_ERR_RSA_CONFIG,
    MT_CIPHER_ERR_RSA_PROCESS,

    MT_CIPHER_ERR_HASH_CREATE,
    MT_CIPHER_ERR_HASH_UPDATE,
    MT_CIPHER_ERR_HASH_FINAL,

    MT_CIPHER_ERR_HMAC_CREATE,
    MT_CIPHER_ERR_HMAC_UPDATE,
    MT_CIPHER_ERR_HMAC_FINAL,

    MT_CIPHER_ERR_BN_CREATE,
    MT_CIPHER_ERR_BN_DESTROY,
    MT_CIPHER_ERR_BN_MODMOD,
    MT_CIPHER_ERR_BN_MODADD,
    MT_CIPHER_ERR_BN_MODSUB,
    MT_CIPHER_ERR_BN_MODMUL,
    MT_CIPHER_ERR_BN_MODEXP,
    MT_CIPHER_ERR_BN_MODINV,

    MT_CIPHER_ERR_ECP_CREATE,
    MT_CIPHER_ERR_ECP_DESTROY,
    MT_CIPHER_ERR_ECP_ADD,
    MT_CIPHER_ERR_ECP_MUL,

    MT_CIPHER_ERR_BGC_REQUEST,
    MT_CIPHER_ERR_BGC_RELEASE,
};

typedef enum
{
  MT_RSAES_PKCS1_V1_5,
  MT_RSAES_PKCS1_OAEP_MGF1_SHA1,
  MT_RSAES_PKCS1_OAEP_MGF1_SHA256,
  MT_RSA_NOPAD,
  MT_RSASSA_PKCS1_V1_5_SHA1,
  MT_RSASSA_PKCS1_V1_5_SHA256,
  MT_RSASSA_PKCS1_PSS_MGF1_SHA1,
  MT_RSASSA_PKCS1_PSS_MGF1_SHA256,

  MT_LAST_RSA_ALG,
}MT_RSA_ALG;

typedef enum _MT_CIPHER_HASH_TYPE_E
{
    MT_CIPHER_HASH_TYPE_SHA1,
    MT_CIPHER_HASH_TYPE_SHA256,
}MT_CIPHER_HASH_TYPE_E;

struct rsa_public_key {
    unsigned int n_length;
    unsigned char *n;	/* Modulus */

    unsigned int e_length;
    unsigned char *e;	/* Public exponent */
};

int mt_rsa_verify(struct rsa_public_key *key, MT_RSA_ALG algo,
	const unsigned char *msg, unsigned int mlen, const unsigned char *sign, unsigned int slen);

#endif
