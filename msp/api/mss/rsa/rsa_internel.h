/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __RSA_INTERNEL_H__
#define __RSA_INTERNEL_H__

int rsa_crypto_public(struct rsa_public_key *key,
        unsigned char *src, unsigned int src_len, unsigned char *dst);

int rsa_crypto_private(struct rsa_keypair *key,
        unsigned char *src, unsigned int src_len, unsigned char *dst);

int rsa_crypto_private_crt(struct rsa_keypair *key,
        unsigned char *src, unsigned int src_len, unsigned char *dst);

#endif

