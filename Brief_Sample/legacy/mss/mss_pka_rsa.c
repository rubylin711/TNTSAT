/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "mt_type.h"
#include "mt_common.h"
#include "mt_unf_cipher_v2.h"
#include "mt_unf_rsa.h"
#include "mss_cmd_utils.h"
#include "mss_tee_client.h"
#include "mss_pka.h"

struct pka_rsa_data
{
	unsigned int n_length;
	unsigned char n[256];  /* Modulus */

	unsigned int e_length;
	unsigned char e[256];  /* Public exponent */

	unsigned int d_length;
	unsigned char d[256];  /* Private exponent */

        /* Optional CRT parameters (all NULL if unused) */
	unsigned int p_length;
	unsigned char p[256];  /* N = pq */

	unsigned int q_length;
	unsigned char q[256];

	unsigned int qInv_length;
	unsigned char qInv[256];  /* 1/q mod p */

	unsigned int dp_length;
	unsigned char dp[256];  /* d mod (p-1) */

	unsigned int dq_length;
	unsigned char dq[256];  /* d mod (q-1) */

	unsigned int plain_length;
	unsigned char plain[256];

	unsigned char cipher[256];
	unsigned char cipher_oaep_sha256[256];
	unsigned char cipher_pkcs1v15[256];
	unsigned char sign_pkcs1v15_sha1[256];
	unsigned char sign_pss_sha256[256];
}__attribute__((aligned(4)));
#include "pka_rsa_testcase.h"

static int mss_pka_rsa_privenc_pubdec(struct pka_rsa_data *testcase,
    int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 buf1[256];
    mt_u8 buf2[256];
    mt_u8 *input;
    mt_u8 *output;
    mt_u32 inlen;
    mt_u32 outlen;
    mt_u32 oaep_sha256_flag = 1;
    mt_u32 pkcs1v15_flag = 1;

    struct rsa_public_key pubkey;
    struct rsa_keypair privkey;
    struct rsa_keypair privkey_crt;

    printf("\n=====================================\n");
    printf("%s IN\n", __FUNCTION__);

    if (argc > 0) {
        if (0 == strcmp("oaep_sha256", argv[0])){
            oaep_sha256_flag = 1;
            pkcs1v15_flag = 0;
        } else if (0 == strcmp("pkcs1v15", argv[0])){
            oaep_sha256_flag = 0;
            pkcs1v15_flag = 1;
        }
    }

    memset(&pubkey, 0, sizeof(struct rsa_public_key));
    pubkey.e_length = testcase->e_length;
    pubkey.e = testcase->e;
    pubkey.n_length = testcase->n_length;
    pubkey.n = testcase->n;

    memset(&privkey, 0, sizeof(struct rsa_keypair));
    privkey.d_length = testcase->d_length;
    privkey.d = testcase->d;
    privkey.n_length = testcase->n_length;
    privkey.n = testcase->n;

    memset(&privkey_crt, 0, sizeof(struct rsa_keypair));
    privkey_crt.e_length = testcase->e_length;
    privkey_crt.e = testcase->e;
    privkey_crt.n_length = testcase->n_length;
    privkey_crt.n = testcase->n;
    privkey_crt.p_length = testcase->p_length;
    privkey_crt.p = testcase->p;
    privkey_crt.q_length = testcase->q_length;
    privkey_crt.q = testcase->q;
    privkey_crt.qInv_length = testcase->qInv_length;
    privkey_crt.qInv = testcase->qInv;
    privkey_crt.dp_length = testcase->dp_length;
    privkey_crt.dp = testcase->dp;
    privkey_crt.dq_length = testcase->dq_length;
    privkey_crt.dq = testcase->dq;

    /* rsa public deccrypt with OAEP SHA256 PADDING */
    if (oaep_sha256_flag)
    {
        printf("\nSTART RSA OAEP SHA256 PADDING test\n");

        input = buf2;
        memcpy(input, testcase->plain, testcase->plain_length);
        inlen = testcase->plain_length;
        output = buf1;

        mss_dump("Input", input, inlen);
        ret = mt_unf_rsa_private_encrypt(&privkey_crt, input, inlen, output,
                        MT_RSAES_PKCS1_OAEP_MGF1_SHA256);
        if (ret != MT_SUCCESS) {
            printf("OAEP SHA256 PRIVENC CRT Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("OAEP SHA256 PRIVENC CRT  Output", output, pubkey.n_length);

        input = output;
        inlen = pubkey.n_length;
        output = buf2;

        ret = mt_unf_rsa_public_decrypt(&pubkey, input, inlen, output, &outlen,
                        MT_RSAES_PKCS1_OAEP_MGF1_SHA256);
        if (ret != MT_SUCCESS) {
            printf("OAEP SHA256 PUBDEC Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("OAEP SHA256 PUBDEC Output", output, outlen);

        if((outlen == testcase->plain_length)
            && memcmp(output, testcase->plain, outlen) == 0) {
          printf("RSA OAEP SHA256 PRIVENC PUBDEC test ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA OAEP SHA256 PRIVENC PUBDEC test ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }

    /* rsa public deccrypt with PKCS1 V1.5 PADDING */
    if (pkcs1v15_flag)
    {
        printf("\nSTART RSA PKCS1 V1.5 PADDING test\n");

        input = buf2;
        memcpy(input, testcase->plain, testcase->plain_length);
        inlen = testcase->plain_length;
        output = buf1;

        mss_dump("Input", input, inlen);
        ret = mt_unf_rsa_private_encrypt(&privkey, input, inlen, output,
                        MT_RSAES_PKCS1_V1_5);
        if (ret != MT_SUCCESS) {
            printf("PKCS1 V1.5 PRIVENC CRT Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("PKCS1 V1.5 PRIVENC Output", output, pubkey.n_length);

        input = output;
        inlen = pubkey.n_length;
        output = buf2;

        ret = mt_unf_rsa_public_decrypt(&pubkey, input, inlen, output, &outlen,
                        MT_RSAES_PKCS1_V1_5);
        if (ret != MT_SUCCESS) {
            printf("PKCS1 V1.5 PUBDEC Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("PKCS1 V1.5 PUBDEC Output", output, outlen);

        if((outlen == testcase->plain_length)
            && memcmp(output, testcase->plain, outlen) == 0) {
          printf("RSA PKCS1 V1.5 PRIVENC PUBDEC test ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA PKCS1 V1.5 PRIVENC PUBDEC test ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }
    return ret;
}

static int mss_pka_rsa_pubenc_privdec(struct pka_rsa_data *testcase,
    int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 buf1[256];
    mt_u8 buf2[256];
    mt_u8 *input;
    mt_u8 *output;
    mt_u32 inlen;
    mt_u32 outlen;
    mt_u32 oaep_sha256_flag = 1;
    mt_u32 pkcs1v15_flag = 1;

    struct rsa_public_key pubkey;
    struct rsa_keypair privkey;
    struct rsa_keypair privkey_crt;

    printf("\n=====================================\n");
    printf("%s IN\n", __FUNCTION__);

    if (argc > 0) {
        if (0 == strcmp("oaep_sha256", argv[0])){
            oaep_sha256_flag = 1;
            pkcs1v15_flag = 0;
        } else if (0 == strcmp("pkcs1v15", argv[0])){
            oaep_sha256_flag = 0;
            pkcs1v15_flag = 1;
        }
    }

    memset(&pubkey, 0, sizeof(struct rsa_public_key));
    pubkey.e_length = testcase->e_length;
    pubkey.e = testcase->e;
    pubkey.n_length = testcase->n_length;
    pubkey.n = testcase->n;

    memset(&privkey, 0, sizeof(struct rsa_keypair));
    privkey.d_length = testcase->d_length;
    privkey.d = testcase->d;
    privkey.n_length = testcase->n_length;
    privkey.n = testcase->n;

    memset(&privkey_crt, 0, sizeof(struct rsa_keypair));
    privkey_crt.e_length = testcase->e_length;
    privkey_crt.e = testcase->e;
    privkey_crt.n_length = testcase->n_length;
    privkey_crt.n = testcase->n;
    privkey_crt.p_length = testcase->p_length;
    privkey_crt.p = testcase->p;
    privkey_crt.q_length = testcase->q_length;
    privkey_crt.q = testcase->q;
    privkey_crt.qInv_length = testcase->qInv_length;
    privkey_crt.qInv = testcase->qInv;
    privkey_crt.dp_length = testcase->dp_length;
    privkey_crt.dp = testcase->dp;
    privkey_crt.dq_length = testcase->dq_length;
    privkey_crt.dq = testcase->dq;

    /* rsa public deccrypt with OAEP SHA256 PADDING */
    if (oaep_sha256_flag)
    {
        printf("\nSTART RSA OAEP SHA256 PADDING test\n");

        input = buf2;
        memcpy(input, testcase->plain, testcase->plain_length);
        inlen = testcase->plain_length;
        output = buf1;

        mss_dump("Input", input, inlen);
        ret = mt_unf_rsa_public_encrypt(&pubkey, input, inlen, output,
                        MT_RSAES_PKCS1_OAEP_MGF1_SHA256);
        if (ret != MT_SUCCESS) {
            printf("OAEP PUBVENC Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("OAEP PUBVENC Output", output, pubkey.n_length);

        input = output;
        inlen = pubkey.n_length;
        output = buf2;

        ret = mt_unf_rsa_private_decrypt(&privkey_crt, input, inlen, output, &outlen,
                        MT_RSAES_PKCS1_OAEP_MGF1_SHA256);
        if (ret != MT_SUCCESS) {
            printf("OAEP PRIVVDEC CRTFailed!\n");
            return MT_FAILURE;
        }

        mss_dump("OAEP PRIVVDEC CRT Output", output, outlen);

        if((outlen == testcase->plain_length)
            && memcmp(output, testcase->plain, outlen) == 0) {
          printf("RSA OAEP SHA256 PADDING test ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA OAEP SHA256 PADDING test ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }

    /* rsa public deccrypt with PKCS1 V1.5 PADDING */
    if (pkcs1v15_flag)
    {
        printf("\nSTART RSA PKCS1 V1.5 PADDING test\n");

        input = buf2;
        memcpy(input, testcase->plain, testcase->plain_length);
        inlen = testcase->plain_length;
        output = buf1;

        mss_dump("Input", input, inlen);
        ret = mt_unf_rsa_public_encrypt(&pubkey, input, inlen, output,
                        MT_RSAES_PKCS1_V1_5);
        if (ret != MT_SUCCESS) {
            printf("PKCS1 V1.5 PUBVENC Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("PKCS1 V1.5 PUBVENC Output", output, pubkey.n_length);

        input = output;
        inlen = pubkey.n_length;
        output = buf2;

        ret = mt_unf_rsa_private_decrypt(&privkey, input, inlen, output, &outlen,
                        MT_RSAES_PKCS1_V1_5);
        if (ret != MT_SUCCESS) {
            printf("PKCS1 V1.5 PRIVDEC Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("PKCS1 V1.5 PRIVDEC Output", output, outlen);

        if((outlen == testcase->plain_length)
            && memcmp(output, testcase->plain, outlen) == 0) {
          printf("RSA PKCS1 V1.5 PADDING test ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA PKCS1 V1.5 PADDING test ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }

    return ret;
}

static int mss_pka_rsa_privatedec(struct pka_rsa_data *testcase,
    int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 output[256];
    mt_u8 input[256];
    mt_u32 inlen;
    mt_u32 outlen;
    mt_u32 oaep_sha256_flag = 1;
    mt_u32 pkcs1v15_flag = 1;
    mt_u32 nopadding_flag = 1;

    struct rsa_keypair privkey;
    struct rsa_keypair privkey_crt;
    printf("\n=====================================\n");
    printf("%s IN\n", __FUNCTION__);

    if (argc > 0) {
        if (0 == strcmp("oaep_sha256", argv[0])){
            oaep_sha256_flag = 1;
            pkcs1v15_flag = 0;
            nopadding_flag = 0;
        } else if (0 == strcmp("pkcs1v15", argv[0])){
            oaep_sha256_flag = 0;
            pkcs1v15_flag = 1;
            nopadding_flag = 0;
        } else if (0 == strcmp("nopadding", argv[0])){
            oaep_sha256_flag = 0;
            pkcs1v15_flag = 0;
            nopadding_flag = 1;
        }
    }

    memset(&privkey, 0, sizeof(struct rsa_keypair));
    privkey.d_length = testcase->d_length;
    privkey.d = testcase->d;
    privkey.n_length = testcase->n_length;
    privkey.n = testcase->n;

    memset(&privkey_crt, 0, sizeof(struct rsa_keypair));
    privkey_crt.e_length = testcase->e_length;
    privkey_crt.e = testcase->e;
    privkey_crt.n_length = testcase->n_length;
    privkey_crt.n = testcase->n;
    privkey_crt.p_length = testcase->p_length;
    privkey_crt.p = testcase->p;
    privkey_crt.q_length = testcase->q_length;
    privkey_crt.q = testcase->q;
    privkey_crt.qInv_length = testcase->qInv_length;
    privkey_crt.qInv = testcase->qInv;
    privkey_crt.dp_length = testcase->dp_length;
    privkey_crt.dp = testcase->dp;
    privkey_crt.dq_length = testcase->dq_length;
    privkey_crt.dq = testcase->dq;

    /* rsa public deccrypt with NO PADDING */
    if (nopadding_flag)
    {
        printf("\nSTART RSA PRIVATE DECRYPT with NO PADDING\n");

        inlen = privkey.n_length;
        memcpy(input, testcase->cipher, inlen);

        mss_dump("Input", input, inlen);

        ret = mt_unf_rsa_private_decrypt(&privkey, input, privkey.n_length, output, &outlen,
                        MT_RSA_NOPAD);
        if (ret != MT_SUCCESS) {
            printf("NO PADDING PRIVDEC Failed!(%d)\n", ret);
            return MT_FAILURE;
        }

        mss_dump("Output", output, outlen);

        if(memcmp(output+outlen-testcase->plain_length, testcase->plain, testcase->plain_length) == 0) {
          printf("RSA PRIVATE DEC NO PADDING ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA PRIVATE DEC NO PADDING ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }

    /* rsa public deccrypt with OAEP SHA256 PADDING */
    if (oaep_sha256_flag)
    {
        printf("\nSTART RSA PRIVATE DECRYPT with OAEP SHA256 PADDING\n");

        inlen = privkey.n_length;
        memcpy(input, testcase->cipher_oaep_sha256, inlen);

        mss_dump("Input", input, inlen);

        ret = mt_unf_rsa_private_decrypt(&privkey, input, privkey.n_length, output, &outlen,
                        MT_RSAES_PKCS1_OAEP_MGF1_SHA256);
        if (ret != MT_SUCCESS) {
            printf("OAEP SHA256 PRIVDEC Failed!(%d)\n", ret);
            return MT_FAILURE;
        }

        mss_dump("Output", output, outlen);

        if((outlen == testcase->plain_length)
            && memcmp(output, testcase->plain, outlen) == 0) {
          printf("RSA PRIVATE DEC OAEP SHA256 PADDING ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA PRIVATE DEC OAEP SHA256 PADDING ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }

    /* rsa public deccrypt with PKCS1 V1.5 PADDING */
    if (pkcs1v15_flag)
    {
        printf("\nSTART RSA PRIVATE DECRYPT with PKCS1 V1.5 PADDING\n");

        inlen = privkey_crt.n_length;
        memcpy(input, testcase->cipher_pkcs1v15, inlen);

        mss_dump("Input", input, inlen);

        ret = mt_unf_rsa_private_decrypt(&privkey_crt, input, privkey_crt.n_length, output, &outlen,
                    MT_RSAES_PKCS1_V1_5);
        if (ret != MT_SUCCESS) {
            printf("PKCS1 V1.5 PRIVDEC Failed!(%d)\n", ret);
            return MT_FAILURE;
        }

        mss_dump("Output", output, outlen);

        if((outlen == testcase->plain_length)
            && memcmp(output, testcase->plain, outlen) == 0) {
          printf("RSA PRIVATE DEC PKCS1 V1.5 PADDING ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA PRIVATE DEC PKCS1 V1.5 PADDING ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }

    return ret;
}

static int mss_pka_rsa_sign_verify(struct pka_rsa_data *testcase,
    int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 buf1[256];
    mt_u8 *sign;
    mt_u32 slen;
    mt_u32 pkcs1v15_sha1_flag = 1;
    mt_u32 pss_sha256_flag = 1;
    struct rsa_public_key pubkey;
    struct rsa_keypair privkey;
    struct rsa_keypair privkey_crt;

    printf("\n=====================================\n");
    printf("%s IN\n", __FUNCTION__);

    if (argc > 0) {
        if (0 == strcmp("pkcs1v15_sha1", argv[0])){
            pkcs1v15_sha1_flag = 1;
            pss_sha256_flag = 0;
        } else if (0 == strcmp("pss_sha256", argv[0])){
            pkcs1v15_sha1_flag = 0;
            pss_sha256_flag = 1;
        }
    }

    memset(&pubkey, 0, sizeof(struct rsa_public_key));
    pubkey.e_length = testcase->e_length;
    pubkey.e = testcase->e;
    pubkey.n_length = testcase->n_length;
    pubkey.n = testcase->n;

    memset(&privkey, 0, sizeof(struct rsa_keypair));
    privkey.d_length = testcase->d_length;
    privkey.d = testcase->d;
    privkey.n_length = testcase->n_length;
    privkey.n = testcase->n;

    memset(&privkey_crt, 0, sizeof(struct rsa_keypair));
    privkey_crt.e_length = testcase->e_length;
    privkey_crt.e = testcase->e;
    privkey_crt.n_length = testcase->n_length;
    privkey_crt.n = testcase->n;
    privkey_crt.p_length = testcase->p_length;
    privkey_crt.p = testcase->p;
    privkey_crt.q_length = testcase->q_length;
    privkey_crt.q = testcase->q;
    privkey_crt.qInv_length = testcase->qInv_length;
    privkey_crt.qInv = testcase->qInv;
    privkey_crt.dp_length = testcase->dp_length;
    privkey_crt.dp = testcase->dp;
    privkey_crt.dq_length = testcase->dq_length;
    privkey_crt.dq = testcase->dq;

    sign = buf1;

    /* rsassa with PSS SHA256 */
    if (pss_sha256_flag)
    {
        printf("\nSTART RSA verify with PSS SHA256 PADDING\n");

        ret = mt_unf_rsa_verify(&pubkey, MT_RSASSA_PKCS1_PSS_MGF1_SHA256,
            testcase->plain, testcase->plain_length, testcase->sign_pss_sha256, testcase->n_length);
        if(MT_SUCCESS == ret)
        {
            printf("RSASSA PSS SHA256 verify test Success!\n");
            ret = MT_SUCCESS;
        }
        else
        {
            printf("RSASSA PSS SHA256 verify test Failed!\n");
            return MT_FAILURE;
        }

        mt_unf_rsa_sign(&privkey_crt, MT_RSASSA_PKCS1_PSS_MGF1_SHA256,
            testcase->plain, testcase->plain_length, sign, &slen);

        ret = mt_unf_rsa_verify(&pubkey, MT_RSASSA_PKCS1_PSS_MGF1_SHA256,
                    testcase->plain, testcase->plain_length, sign, slen);

        if(MT_SUCCESS == ret)
        {
            printf("RSASSA PSS SHA256 CRT sign test ##### SUCCESS\n");
            ret = MT_SUCCESS;
        }
        else
        {
            printf("RSASSA PSS SHA256 CRT sign test ##### FAIL\n");
            return MT_FAILURE;
        }
    }

    /* rsassa with PKCS1 V1.5 SHA1 */
    if (pkcs1v15_sha1_flag)
    {
        printf("\nSTART RSA verify with PKCS1 V1.5 SHA1 PADDING\n");

        ret = mt_unf_rsa_verify(&pubkey, MT_RSASSA_PKCS1_V1_5_SHA1,
            testcase->plain, testcase->plain_length, testcase->sign_pkcs1v15_sha1, testcase->n_length);
        if(MT_SUCCESS == ret)
        {
            printf("RSASSA PKCS1 V1.5 SHA1 verify test Success!\n");
            ret = MT_SUCCESS;
        }
        else
        {
            printf("RSASSA PKCS1 V1.5 SHA1 verify test Failed!\n");
            return MT_FAILURE;
        }

        mt_unf_rsa_sign(&privkey, MT_RSASSA_PKCS1_V1_5_SHA1,
            testcase->plain, testcase->plain_length, sign, &slen);

        ret = mt_unf_rsa_verify(&pubkey, MT_RSASSA_PKCS1_V1_5_SHA1,
            testcase->plain, testcase->plain_length, sign, slen);

        if(MT_SUCCESS == ret)
        {
            printf("RSASSA PKCS1 V1.5 SHA1 sign test ##### SUCCESS\n");
            ret = MT_SUCCESS;
        }
        else
        {
            printf("RSASSA PKCS1 V1.5 SHA1 sign test ##### FAIL\n");
            return MT_FAILURE;
        }
    }

    return 0;
}

#ifdef CONFIG_MT_TEE_SUPPORT
static int mss_pka_rsa_privenc_pubdec_tee(struct pka_rsa_data *testcase,
    int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 buf1[256];
    mt_u8 buf2[256];
    mt_u8 *input;
    mt_u8 *output;
    mt_u32 inlen;
    mt_u32 outlen;
    mt_u32 oaep_sha256_flag = 1;
    mt_u32 pkcs1v15_flag = 1;

    struct rsa_public_key pubkey;
    struct rsa_keypair privkey;
    struct rsa_keypair privkey_crt;

    printf("\n=====================================\n");
    printf("%s IN\n", __FUNCTION__);

    if (argc > 0) {
        if (0 == strcmp("oaep_sha256", argv[0])){
            oaep_sha256_flag = 1;
            pkcs1v15_flag = 0;
        } else if (0 == strcmp("pkcs1v15", argv[0])){
            oaep_sha256_flag = 0;
            pkcs1v15_flag = 1;
        }
    }

    memset(&pubkey, 0, sizeof(struct rsa_public_key));
    pubkey.e_length = testcase->e_length;
    pubkey.e = testcase->e;
    pubkey.n_length = testcase->n_length;
    pubkey.n = testcase->n;

    memset(&privkey, 0, sizeof(struct rsa_keypair));
    privkey.d_length = testcase->d_length;
    privkey.d = testcase->d;
    privkey.n_length = testcase->n_length;
    privkey.n = testcase->n;

    memset(&privkey_crt, 0, sizeof(struct rsa_keypair));
    privkey_crt.e_length = testcase->e_length;
    privkey_crt.e = testcase->e;
    privkey_crt.n_length = testcase->n_length;
    privkey_crt.n = testcase->n;
    privkey_crt.p_length = testcase->p_length;
    privkey_crt.p = testcase->p;
    privkey_crt.q_length = testcase->q_length;
    privkey_crt.q = testcase->q;
    privkey_crt.qInv_length = testcase->qInv_length;
    privkey_crt.qInv = testcase->qInv;
    privkey_crt.dp_length = testcase->dp_length;
    privkey_crt.dp = testcase->dp;
    privkey_crt.dq_length = testcase->dq_length;
    privkey_crt.dq = testcase->dq;

    /* rsa public deccrypt with OAEP SHA256 PADDING */
    if (oaep_sha256_flag)
    {
        printf("\nSTART RSA OAEP SHA256 PADDING TEST\n");

        input = buf2;
        memcpy(input, testcase->plain, testcase->plain_length);
        inlen = testcase->plain_length;
        output = buf1;

        mss_dump("Input", input, inlen);
        ret = tee_rsa_private_encrypt(&privkey_crt, input, inlen, output,
                        MT_RSAES_PKCS1_OAEP_MGF1_SHA256);
        if (ret != MT_SUCCESS) {
            printf("OAEP SHA256 PRIVENC CRT Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("OAEP SHA256 PRIVENC CRT  Output", output, pubkey.n_length);

        input = output;
        inlen = pubkey.n_length;
        output = buf2;

        ret = tee_rsa_public_decrypt(&pubkey, input, inlen, output, &outlen,
                        MT_RSAES_PKCS1_OAEP_MGF1_SHA256);
        if (ret != MT_SUCCESS) {
            printf("OAEP SHA256 PUBDEC Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("OAEP SHA256 PUBDEC Output", output, outlen);

        if((outlen == testcase->plain_length)
            && memcmp(output, testcase->plain, outlen) == 0) {
          printf("RSA OAEP SHA256 PRIVENC PUBDEC test ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA OAEP SHA256 PRIVENC PUBDEC test ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }

    /* rsa public deccrypt with PKCS1 V1.5 PADDING */
    if (pkcs1v15_flag)
    {
        printf("\nSTART RSA PKCS1 V1.5 PADDING test\n");

        input = buf2;
        memcpy(input, testcase->plain, testcase->plain_length);
        inlen = testcase->plain_length;
        output = buf1;

        mss_dump("Input", input, inlen);
        ret = tee_rsa_private_encrypt(&privkey, input, inlen, output,
                        MT_RSAES_PKCS1_V1_5);
        if (ret != MT_SUCCESS) {
            printf("PKCS1 V1.5 PRIVENC CRT Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("PKCS1 V1.5 PRIVENC Output", output, pubkey.n_length);

        input = output;
        inlen = pubkey.n_length;
        output = buf2;

        ret = tee_rsa_public_decrypt(&pubkey, input, inlen, output, &outlen,
                        MT_RSAES_PKCS1_V1_5);
        if (ret != MT_SUCCESS) {
            printf("PKCS1 V1.5 PUBDEC Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("PKCS1 V1.5 PUBDEC Output", output, outlen);

        if((outlen == testcase->plain_length)
            && memcmp(output, testcase->plain, outlen) == 0) {
          printf("RSA PKCS1 V1.5 PRIVENC PUBDEC test ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA PKCS1 V1.5 PRIVENC PUBDEC test ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }
    return ret;
}

static int mss_pka_rsa_pubenc_privdec_tee(struct pka_rsa_data *testcase,
    int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 buf1[256];
    mt_u8 buf2[256];
    mt_u8 *input;
    mt_u8 *output;
    mt_u32 inlen;
    mt_u32 outlen;
    mt_u32 oaep_sha256_flag = 1;
    mt_u32 pkcs1v15_flag = 1;

    struct rsa_public_key pubkey;
    struct rsa_keypair privkey;
    struct rsa_keypair privkey_crt;

    printf("\n=====================================\n");
    printf("%s IN\n", __FUNCTION__);

    if (argc > 0) {
        if (0 == strcmp("oaep_sha256", argv[0])){
            oaep_sha256_flag = 1;
            pkcs1v15_flag = 0;
        } else if (0 == strcmp("pkcs1v15", argv[0])){
            oaep_sha256_flag = 0;
            pkcs1v15_flag = 1;
        }
    }

    memset(&pubkey, 0, sizeof(struct rsa_public_key));
    pubkey.e_length = testcase->e_length;
    pubkey.e = testcase->e;
    pubkey.n_length = testcase->n_length;
    pubkey.n = testcase->n;

    memset(&privkey, 0, sizeof(struct rsa_keypair));
    privkey.d_length = testcase->d_length;
    privkey.d = testcase->d;
    privkey.n_length = testcase->n_length;
    privkey.n = testcase->n;

    memset(&privkey_crt, 0, sizeof(struct rsa_keypair));
    privkey_crt.e_length = testcase->e_length;
    privkey_crt.e = testcase->e;
    privkey_crt.n_length = testcase->n_length;
    privkey_crt.n = testcase->n;
    privkey_crt.p_length = testcase->p_length;
    privkey_crt.p = testcase->p;
    privkey_crt.q_length = testcase->q_length;
    privkey_crt.q = testcase->q;
    privkey_crt.qInv_length = testcase->qInv_length;
    privkey_crt.qInv = testcase->qInv;
    privkey_crt.dp_length = testcase->dp_length;
    privkey_crt.dp = testcase->dp;
    privkey_crt.dq_length = testcase->dq_length;
    privkey_crt.dq = testcase->dq;

    /* rsa public deccrypt with OAEP SHA256 PADDING */
    if (oaep_sha256_flag)
    {
        printf("\nSTART RSA OAEP SHA256 PADDING test\n");

        input = buf2;
        memcpy(input, testcase->plain, testcase->plain_length);
        inlen = testcase->plain_length;
        output = buf1;

        mss_dump("Input", input, inlen);
        ret = tee_rsa_public_encrypt(&pubkey, input, inlen, output,
                        MT_RSAES_PKCS1_OAEP_MGF1_SHA256);
        if (ret != MT_SUCCESS) {
            printf("OAEP PUBVENC Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("OAEP PUBVENC Output", output, pubkey.n_length);

        input = output;
        inlen = pubkey.n_length;
        output = buf2;

        ret = tee_rsa_private_decrypt(&privkey_crt, input, inlen, output, &outlen,
                        MT_RSAES_PKCS1_OAEP_MGF1_SHA256);
        if (ret != MT_SUCCESS) {
            printf("OAEP PRIVVDEC CRTFailed!\n");
            return MT_FAILURE;
        }

        mss_dump("OAEP PRIVVDEC CRT Output", output, outlen);

        if((outlen == testcase->plain_length)
            && memcmp(output, testcase->plain, outlen) == 0) {
          printf("RSA OAEP SHA256 PADDING test ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA OAEP SHA256 PADDING test ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }

    /* rsa public deccrypt with PKCS1 V1.5 PADDING */
    if (pkcs1v15_flag)
    {
        printf("\nSTART RSA PKCS1 V1.5 PADDING TEST\n");

        input = buf2;
        memcpy(input, testcase->plain, testcase->plain_length);
        inlen = testcase->plain_length;
        output = buf1;

        mss_dump("Input", input, inlen);
        ret = tee_rsa_public_encrypt(&pubkey, input, inlen, output,
                        MT_RSAES_PKCS1_V1_5);
        if (ret != MT_SUCCESS) {
            printf("PKCS1 V1.5 PUBVENC Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("PKCS1 V1.5 PUBVENC Output", output, pubkey.n_length);

        input = output;
        inlen = pubkey.n_length;
        output = buf2;

        ret = tee_rsa_private_decrypt(&privkey, input, inlen, output, &outlen,
                        MT_RSAES_PKCS1_V1_5);
        if (ret != MT_SUCCESS) {
            printf("PKCS1 V1.5 PRIVDEC Failed!\n");
            return MT_FAILURE;
        }

        mss_dump("PKCS1 V1.5 PRIVDEC Output", output, outlen);

        if((outlen == testcase->plain_length)
            && memcmp(output, testcase->plain, outlen) == 0) {
          printf("RSA PKCS1 V1.5 PADDING test ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA PKCS1 V1.5 PADDING test ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }

    return ret;
}

static int mss_pka_rsa_privatedec_tee(struct pka_rsa_data *testcase,
    int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 output[256];
    mt_u8 input[256];
    mt_u32 inlen;
    mt_u32 outlen;
    mt_u32 oaep_sha256_flag = 1;
    mt_u32 pkcs1v15_flag = 1;
    mt_u32 nopadding_flag = 1;

    struct rsa_keypair privkey;
    struct rsa_keypair privkey_crt;
    printf("\n=====================================\n");
    printf("%s IN\n", __FUNCTION__);

    if (argc > 0) {
        if (0 == strcmp("oaep_sha256", argv[0])){
            oaep_sha256_flag = 1;
            pkcs1v15_flag = 0;
            nopadding_flag = 0;
        } else if (0 == strcmp("pkcs1v15", argv[0])){
            oaep_sha256_flag = 0;
            pkcs1v15_flag = 1;
            nopadding_flag = 0;
        } else if (0 == strcmp("nopadding", argv[0])){
            oaep_sha256_flag = 0;
            pkcs1v15_flag = 0;
            nopadding_flag = 1;
        }
    }

    memset(&privkey, 0, sizeof(struct rsa_keypair));
    privkey.d_length = testcase->d_length;
    privkey.d = testcase->d;
    privkey.n_length = testcase->n_length;
    privkey.n = testcase->n;

    memset(&privkey_crt, 0, sizeof(struct rsa_keypair));
    privkey_crt.e_length = testcase->e_length;
    privkey_crt.e = testcase->e;
    privkey_crt.n_length = testcase->n_length;
    privkey_crt.n = testcase->n;
    privkey_crt.p_length = testcase->p_length;
    privkey_crt.p = testcase->p;
    privkey_crt.q_length = testcase->q_length;
    privkey_crt.q = testcase->q;
    privkey_crt.qInv_length = testcase->qInv_length;
    privkey_crt.qInv = testcase->qInv;
    privkey_crt.dp_length = testcase->dp_length;
    privkey_crt.dp = testcase->dp;
    privkey_crt.dq_length = testcase->dq_length;
    privkey_crt.dq = testcase->dq;

    /* rsa public deccrypt with NO PADDING */
    if (nopadding_flag)
    {
        printf("\nSTART RSA PRIVATE DECRYPT with NO PADDING\n");

        inlen = privkey.n_length;
        memcpy(input, testcase->cipher, inlen);

        mss_dump("Input", input, inlen);

        ret = tee_rsa_private_decrypt(&privkey, input, privkey.n_length, output, &outlen,
                        MT_RSA_NOPAD);
        if (ret != MT_SUCCESS) {
            printf("NO PADDING PRIVDEC Failed!(%d)\n", ret);
            return MT_FAILURE;
        }

        mss_dump("Output", output, outlen);

        if(memcmp(output+outlen-testcase->plain_length, testcase->plain, testcase->plain_length) == 0) {
          printf("RSA PRIVATE DEC NO PADDING ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA PRIVATE DEC NO PADDING ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }

    /* rsa public deccrypt with OAEP SHA256 PADDING */
    if (oaep_sha256_flag)
    {
        printf("\nSTART RSA PRIVATE DECRYPT with OAEP SHA256 PADDING\n");

        inlen = privkey.n_length;
        memcpy(input, testcase->cipher_oaep_sha256, inlen);

        mss_dump("Input", input, inlen);

        ret = tee_rsa_private_decrypt(&privkey, input, privkey.n_length, output, &outlen,
                        MT_RSAES_PKCS1_OAEP_MGF1_SHA256);
        if (ret != MT_SUCCESS) {
            printf("OAEP SHA256 PRIVDEC Failed!(%d)\n", ret);
            return MT_FAILURE;
        }

        mss_dump("Output", output, outlen);

        if((outlen == testcase->plain_length)
            && memcmp(output, testcase->plain, outlen) == 0) {
          printf("RSA PRIVATE DEC OAEP SHA256 PADDING ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA PRIVATE DEC OAEP SHA256 PADDING ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }

    /* rsa public deccrypt with PKCS1 V1.5 PADDING */
    if (pkcs1v15_flag)
    {
        printf("\nSTART RSA PRIVATE DECRYPT with PKCS1 V1.5 PADDING\n");

        inlen = privkey_crt.n_length;
        memcpy(input, testcase->cipher_pkcs1v15, inlen);

        mss_dump("Input", input, inlen);

        ret = tee_rsa_private_decrypt(&privkey_crt, input, privkey_crt.n_length, output, &outlen,
                    MT_RSAES_PKCS1_V1_5);
        if (ret != MT_SUCCESS) {
            printf("PKCS1 V1.5 PRIVDEC Failed!(%d)\n", ret);
            return MT_FAILURE;
        }

        mss_dump("Output", output, outlen);

        if((outlen == testcase->plain_length)
            && memcmp(output, testcase->plain, outlen) == 0) {
          printf("RSA PRIVATE DEC PKCS1 V1.5 PADDING ##### SUCCESS\n");
          ret = MT_SUCCESS;
        }
        else {
          printf("RSA PRIVATE DEC PKCS1 V1.5 PADDING ##### FAIL\n");
          ret = MT_FAILURE;
        }
    }

    return ret;
}

static int mss_pka_rsa_sign_verify_tee(struct pka_rsa_data *testcase,
    int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 buf1[256];
    mt_u8 *sign;
    mt_u32 slen;
    mt_u32 pkcs1v15_sha1_flag = 1;
    mt_u32 pss_sha256_flag = 1;
    struct rsa_public_key pubkey;
    struct rsa_keypair privkey;
    struct rsa_keypair privkey_crt;

    printf("\n=====================================\n");
    printf("%s IN\n", __FUNCTION__);

    if (argc > 0) {
        if (0 == strcmp("pkcs1v15_sha1", argv[0])){
            pkcs1v15_sha1_flag = 1;
            pss_sha256_flag = 0;
        } else if (0 == strcmp("pss_sha256", argv[0])){
            pkcs1v15_sha1_flag = 0;
            pss_sha256_flag = 1;
        }
    }

    memset(&pubkey, 0, sizeof(struct rsa_public_key));
    pubkey.e_length = testcase->e_length;
    pubkey.e = testcase->e;
    pubkey.n_length = testcase->n_length;
    pubkey.n = testcase->n;

    memset(&privkey, 0, sizeof(struct rsa_keypair));
    privkey.d_length = testcase->d_length;
    privkey.d = testcase->d;
    privkey.n_length = testcase->n_length;
    privkey.n = testcase->n;

    memset(&privkey_crt, 0, sizeof(struct rsa_keypair));
    privkey_crt.e_length = testcase->e_length;
    privkey_crt.e = testcase->e;
    privkey_crt.n_length = testcase->n_length;
    privkey_crt.n = testcase->n;
    privkey_crt.p_length = testcase->p_length;
    privkey_crt.p = testcase->p;
    privkey_crt.q_length = testcase->q_length;
    privkey_crt.q = testcase->q;
    privkey_crt.qInv_length = testcase->qInv_length;
    privkey_crt.qInv = testcase->qInv;
    privkey_crt.dp_length = testcase->dp_length;
    privkey_crt.dp = testcase->dp;
    privkey_crt.dq_length = testcase->dq_length;
    privkey_crt.dq = testcase->dq;

    sign = buf1;

    /* rsassa with PSS SHA256 */
    if (pss_sha256_flag)
    {
        printf("\nSTART RSA VERITY with PSS SHA256 PADDING\n");

        ret = tee_rsa_verify(&pubkey, MT_RSASSA_PKCS1_PSS_MGF1_SHA256,
            testcase->plain, testcase->plain_length, testcase->sign_pss_sha256, testcase->n_length);
        if(MT_SUCCESS == ret)
        {
            printf("RSASSA PSS SHA256 verify test Success!\n");
            ret = MT_SUCCESS;
        }
        else
        {
            printf("RSASSA PSS SHA256 verify test Failed!\n");
            return MT_FAILURE;
        }

        tee_rsa_sign(&privkey_crt, MT_RSASSA_PKCS1_PSS_MGF1_SHA256,
            testcase->plain, testcase->plain_length, sign, &slen);

        ret = tee_rsa_verify(&pubkey, MT_RSASSA_PKCS1_PSS_MGF1_SHA256,
                    testcase->plain, testcase->plain_length, sign, slen);

        if(MT_SUCCESS == ret)
        {
            printf("RSASSA PSS SHA256 CRT sign test ##### SUCCESS\n");
            ret = MT_SUCCESS;
        }
        else
        {
            printf("RSASSA PSS SHA256 CRT sign test ##### FAIL\n");
            return MT_FAILURE;
        }
    }

    /* rsassa with PKCS1 V1.5 SHA1 */
    if (pkcs1v15_sha1_flag)
    {
        printf("\nSTART RSA VERITY with PKCS1 V1.5 SHA1 PADDING\n");

        ret = tee_rsa_verify(&pubkey, MT_RSASSA_PKCS1_V1_5_SHA1,
            testcase->plain, testcase->plain_length, testcase->sign_pkcs1v15_sha1, testcase->n_length);
        if(MT_SUCCESS == ret)
        {
            printf("RSASSA PKCS1 V1.5 SHA1 verify test Success!\n");
            ret = MT_SUCCESS;
        }
        else
        {
            printf("RSASSA PKCS1 V1.5 SHA1 verify test Failed!\n");
            return MT_FAILURE;
        }

        tee_rsa_sign(&privkey, MT_RSASSA_PKCS1_V1_5_SHA1,
            testcase->plain, testcase->plain_length, sign, &slen);

        ret = tee_rsa_verify(&pubkey, MT_RSASSA_PKCS1_V1_5_SHA1,
            testcase->plain, testcase->plain_length, sign, slen);

        if(MT_SUCCESS == ret)
        {
            printf("RSASSA PKCS1 V1.5 SHA1 sign test ##### SUCCESS\n");
            ret = MT_SUCCESS;
        }
        else
        {
            printf("RSASSA PKCS1 V1.5 SHA1 sign test ##### FAIL\n");
            return MT_FAILURE;
        }
    }

    return 0;
}
#else
static int mss_pka_rsa_privenc_pubdec_tee(struct pka_rsa_data *testcase,
    int argc, char *const argv[])
{
    printf("not support tee\n");
    return MT_FAILURE;
}

static int mss_pka_rsa_pubenc_privdec_tee(struct pka_rsa_data *testcase,
    int argc, char *const argv[])
{
    printf("not support tee\n");
    return MT_FAILURE;
}

static int mss_pka_rsa_privatedec_tee(struct pka_rsa_data *testcase,
    int argc, char *const argv[])
{
    printf("not support tee\n");
    return MT_FAILURE;
}

static int mss_pka_rsa_sign_verify_tee(struct pka_rsa_data *testcase,
    int argc, char *const argv[])
{
    printf("not support tee\n");
    return MT_FAILURE;
}
#endif

int mss_pka_rsa_test(int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i;
    struct pka_rsa_data *testcase = NULL;
    mt_u32 encdec_flag = 1;
    mt_u32 sign_flag = 1;
    MSS_CPU_MODE_E cpu_mode = MSS_APCPU;

    if (argc > 0) {
        if (0 == strcmp("encdec", argv[0])){
            encdec_flag = 1;
            sign_flag = 0;
            argc--;
            argv++;
        } else if (0 == strcmp("sign", argv[0])){
            encdec_flag = 0;
            sign_flag = 1;
            argc--;
            argv++;
        }
    }

    if (argc > 0 && strcmp(argv[0], "-t") == 0) {
        cpu_mode = MSS_SECCPU;
            argc--;
            argv++;
    }

    for (i = 0; i < ARRAY_SIZE(pka_rsa_testcase); i++) {
        testcase =  &pka_rsa_testcase[i];
        if (encdec_flag) {
            if (cpu_mode == MSS_SECCPU) {
                ret |= mss_pka_rsa_privatedec_tee(testcase, argc, argv);
                ret |= mss_pka_rsa_privenc_pubdec_tee(testcase, argc, argv);
                ret |= mss_pka_rsa_pubenc_privdec_tee(testcase, argc, argv);
            } else {
                ret |= mss_pka_rsa_privatedec(testcase, argc, argv);
                ret |= mss_pka_rsa_privenc_pubdec(testcase, argc, argv);
                ret |= mss_pka_rsa_pubenc_privdec(testcase, argc, argv);
            }
        }
        if (sign_flag) {
            if (cpu_mode == MSS_SECCPU)
                ret |= mss_pka_rsa_sign_verify_tee(testcase, argc, argv);
            else
                ret |= mss_pka_rsa_sign_verify(testcase, argc, argv);
        }
        if (ret != MT_SUCCESS) {
            printf("pka_rsa_testcase[%d] FAIL!\n", i);
            break;
        }
    }

    return ret;
}

