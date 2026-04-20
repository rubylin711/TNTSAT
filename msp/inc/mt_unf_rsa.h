/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_RSA_H__
#define __MT_UNF_RSA_H__

#define MT_RSA_ERR_BAD_PADDING (-100)

#define MT_RSA_ENCDEC_ON
#define MT_RSA_SIGNVERIFY_ON

struct rsa_keypair {
    unsigned int n_length;
    unsigned char *n;	/* Modulus */

    unsigned int e_length;
    unsigned char *e;	/* Public exponent */

    unsigned int d_length;
    unsigned char *d;	/* Private exponent */

    /* Optional CRT parameters (all NULL if unused) */
    unsigned int p_length;
    unsigned char *p;	/* N = pq */

    unsigned int q_length;
    unsigned char *q;

    unsigned int qInv_length;
    unsigned char *qInv;	/* 1/q mod p */

    unsigned int dp_length;
    unsigned char *dp;	/* d mod (p-1) */

    unsigned int dq_length;
    unsigned char *dq;	/* d mod (q-1) */
};

struct rsa_public_key {
    unsigned int n_length;
    unsigned char *n;	/* Modulus */

    unsigned int e_length;
    unsigned char *e;	/* Public exponent */
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

#ifdef MT_RSA_ENCDEC_ON
int mt_unf_rsa_public_encrypt(struct rsa_public_key *key,
        unsigned char *src, unsigned int src_len, unsigned char *dst,
        MT_RSA_ALG algo);

int mt_unf_rsa_private_decrypt(struct rsa_keypair *key,
        unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len,
        MT_RSA_ALG algo);

int mt_unf_rsa_public_decrypt(struct rsa_public_key *key,
        unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len,
        MT_RSA_ALG algo);

int mt_unf_rsa_private_encrypt(struct rsa_keypair *key,
        mt_u8*src, mt_u32 src_len, mt_u8 *dst,
        MT_RSA_ALG algo);
#endif

#ifdef MT_RSA_SIGNVERIFY_ON
int mt_unf_rsa_sign(struct rsa_keypair *key, MT_RSA_ALG algo,
	unsigned char *msg, unsigned int mlen, unsigned char *sign, unsigned int *slen);

int mt_unf_rsa_verify(struct rsa_public_key *key, MT_RSA_ALG algo,
	unsigned char *msg, unsigned int mlen, unsigned char *sign, unsigned int slen);
#endif

int mt_unf_rsa_gen_crt_params(unsigned int keysize, unsigned int E,
    const unsigned char *P, const unsigned char *Q,
    unsigned char *DP, unsigned char *DQ, unsigned char *QInv);

#endif
