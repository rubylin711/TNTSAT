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
#include "mt_unf_ecc.h"
#include "mss_cmd_utils.h"
#include "mss_tee_client.h"
#include "mss_pka.h"

struct pka_sm2_signverify_data
{
    unsigned char id[16];
    
    unsigned char prikey[32];
    unsigned char pubkey[64];  /* (x, y) */

    unsigned int msg_length;
    unsigned char msg[256];

    unsigned char sign[64]; /* r + s */
}__attribute__((aligned(4)));

struct pka_sm2_signverify_data pka_sm2_signverify_testcase[] = {
{
    .id = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08},
    .prikey = {
        0xa5,0xe1,0x79,0x66,0xa4,0xeb,0x0b,0xd0,0x43,0x03,0x73,0xd3,0x4d,0xd1,0xe5,0x4c,
        0xc8,0xe0,0xda,0x9f,0x38,0xa3,0x8d,0xf1,0xd1,0x3f,0x7b,0x49,0xb6,0x43,0x99,0xfb
    },
    .pubkey = {
        /* x */
        0xc5,0x35,0x4a,0x59,0x5f,0x0d,0x73,0xe8,0x9d,0x4d,0xac,0x24,0xaa,0x1a,0x90,0xb4,
        0x34,0xb0,0x00,0x21,0xcd,0x6c,0x68,0x59,0x55,0xa7,0xc0,0x39,0xa4,0xf0,0x0f,0xbc,
        /* y */
        0xe9,0x00,0x5a,0xbc,0x3d,0x50,0x7a,0x14,0x6d,0x2a,0x23,0x5c,0x90,0x9c,0xff,0x6a,
        0xef,0x8a,0xe0,0x06,0x97,0xce,0x23,0x90,0x70,0x15,0x80,0x62,0xc7,0xfd,0xb7,0xe6,
    },
    .msg_length = 256,
    .msg = {
        0x39,0xa9,0x60,0xef,0x55,0x4a,0x57,0xac,0xb4,0x51,0xba,0xac,0x9e,0x18,0xcc,0x66,
        0xd5,0x0f,0x18,0x96,0xcb,0x6d,0xc0,0x44,0x7a,0xae,0xc7,0x53,0xaf,0xb2,0xe6,0x2d,
        0x8f,0x92,0x42,0x4a,0xd6,0xaa,0x05,0x2b,0x94,0x35,0x35,0x95,0xb2,0xda,0x3f,0xba,
        0x69,0x2c,0x75,0x40,0x00,0xa3,0x33,0xbb,0x70,0x8a,0x42,0xc0,0x3f,0xaf,0x53,0x8e,
        0x7d,0x47,0xca,0x0e,0x6a,0xf4,0xf7,0x56,0x07,0xdf,0x90,0x18,0x0e,0xcc,0x44,0xec,
        0x91,0x8a,0x98,0x31,0xca,0x62,0x94,0xe8,0xf6,0x90,0x02,0x38,0xb0,0xb0,0xf1,0x90,
        0x01,0x5a,0x07,0xac,0x1c,0x32,0x66,0xf1,0xae,0x68,0x57,0xf6,0x91,0x56,0x45,0xa4,
        0xa5,0x6c,0x9f,0x01,0xce,0xa5,0x11,0xad,0xc2,0x3c,0x51,0x7f,0xfa,0xbe,0xd1,0x42,
        0xec,0xf3,0x3c,0x31,0x53,0x8d,0x0b,0x70,0x38,0x5d,0x08,0x0a,0x38,0xbf,0x87,0x25,
        0x84,0x6a,0xe4,0xcf,0xaa,0x96,0x2e,0xfb,0xdc,0xb6,0x0e,0x4b,0x6c,0x72,0xf4,0xc9,
        0x7d,0x26,0x6b,0x07,0x10,0xbc,0x56,0x83,0x17,0xb9,0x5d,0x6a,0xb5,0x94,0xff,0xaf,
        0x65,0xb1,0xab,0x1d,0x2e,0x95,0x18,0x65,0x81,0xd9,0x3f,0x88,0x75,0x90,0xa1,0x25,
        0xa2,0x63,0x2c,0xc1,0x16,0xe9,0xc1,0x37,0xd8,0x81,0x0d,0xb0,0xf7,0xe1,0x6e,0x3e,
        0xfa,0x24,0x4f,0x8f,0x27,0x4a,0x08,0x88,0xa9,0x57,0x66,0x7f,0x18,0x0d,0xca,0x57,
        0x8d,0x27,0x33,0x49,0x7d,0x59,0x3b,0xca,0x0f,0x95,0x63,0x6d,0xa9,0xe0,0x3d,0xb9,
        0x2d,0xc7,0x16,0xf8,0xf8,0xc8,0x5c,0xe5,0xc2,0x08,0x04,0x9f,0x40,0x7e,0xd6,0x6b
    },
    .sign = {
        /* r */
        0x17,0x27,0xf9,0xaa,0x42,0xac,0x23,0x9a,0x22,0x56,0x3f,0xcb,0xcc,0x55,0xd6,0xa7,
        0x49,0x76,0xff,0x93,0xb7,0xca,0x1b,0x1a,0x91,0x64,0x8b,0xb5,0x3a,0xe7,0xd0,0xbb,
        /* s */
        0x85,0x3e,0x1b,0x4a,0xc7,0xe7,0x0d,0x84,0xbe,0xb0,0xb6,0x42,0xb7,0x4e,0x6d,0xa7,
        0x5e,0x21,0x64,0x1e,0x5c,0x09,0x43,0xf9,0xf2,0x2d,0x05,0xaf,0xe2,0x62,0x18,0xe1,
    },
},
};

static int mss_pka_sm2_sign_verify(struct pka_sm2_signverify_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 *key;
    mt_u8 *p_id, *p_msg;
    mt_u8 *signature;
    mt_u32 msg_len;
    mt_u8 *priKey;
    mt_u8 signbuf[64];

    printf("\n=====================================\n");
    printf("%s IN\n", __FUNCTION__);

    p_id = testcase->id;
    key = testcase->pubkey;
    p_msg = testcase->msg;
    msg_len = testcase->msg_length;
    signature = testcase->sign;
    priKey = testcase->prikey;

    mss_dump("ID", p_id, 16);
    mss_dump("priKey", priKey, 32);
    mss_dump("pubkey", key, 64);
    mss_dump("msg", p_msg, msg_len);

    ret  = mt_unf_ecc_sm2_verify(p_id, &key[0], &key[32], p_msg, msg_len, &signature[0], &signature[32]);
    if(MT_SUCCESS == ret) {
        printf("SM2 verify test Success!\n");
        ret = MT_SUCCESS;
    } else {
        printf("SM2 verify test Failed!\n");
        return MT_FAILURE;
    }

    ret = mt_unf_ecc_sm2_sign(p_id, priKey, p_msg, msg_len, &signbuf[0], &signbuf[32]);
    if(MT_SUCCESS != ret) {
        printf("SM2 sign test Failed!\n");
        return MT_FAILURE;
    }

    ret  = mt_unf_ecc_sm2_verify(p_id, &key[0], &key[32], p_msg, msg_len, &signbuf[0], &signbuf[32]);
    if(MT_SUCCESS == ret) {
        printf("SM2 sign-verify test ##### SUCCESS\n");
        ret = MT_SUCCESS;
    } else {
        printf("SM2 sign-verify test ##### FAIL\n");
        return MT_FAILURE;
    }

    return ret;
}

#ifdef CONFIG_MT_TEE_SUPPORT
static int mss_pka_sm2_sign_verify_tee(struct pka_sm2_signverify_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 *key;
    mt_u8 *p_id, *p_msg;
    mt_u8 *signature;
    mt_u32 msg_len;
    mt_u8 *priKey;
    mt_u8 signbuf[64];

    printf("\n=====================================\n");
    printf("%s IN\n", __FUNCTION__);

    p_id = testcase->id;
    key = testcase->pubkey;
    p_msg = testcase->msg;
    msg_len = testcase->msg_length;
    signature = testcase->sign;
    priKey = testcase->prikey;

    mss_dump("ID", p_id, 16);
    mss_dump("priKey", priKey, 32);
    mss_dump("pubkey", key, 64);
    mss_dump("msg", p_msg, msg_len);

    ret  = tee_sm2_verify(p_id, &key[0], &key[32], p_msg, msg_len, &signature[0], &signature[32]);
    if(MT_SUCCESS == ret) {
        printf("SM2 verify test Success!\n");
        ret = MT_SUCCESS;
    } else {
        printf("SM2 verify test Failed!\n");
        return MT_FAILURE;
    }

    ret = tee_sm2_sign(p_id, priKey, p_msg, msg_len, &signbuf[0], &signbuf[32]);
    if(MT_SUCCESS != ret) {
        printf("SM2 sign test Failed!\n");
        return MT_FAILURE;
    }

    ret  = tee_sm2_verify(p_id, &key[0], &key[32], p_msg, msg_len, &signbuf[0], &signbuf[32]);
    if(MT_SUCCESS == ret) {
        printf("SM2 sign-verify test ##### SUCCESS\n");
        ret = MT_SUCCESS;
    } else {
        printf("SM2 sign-verify test ##### FAIL\n");
        return MT_FAILURE;
    }

    return ret;
}
#else
static int mss_pka_sm2_sign_verify_tee(struct pka_sm2_signverify_data *testcase)
{
    printf("not support tee\n");
    return MT_FAILURE;
}
#endif

struct pka_sm2_encdec_data
{
    unsigned char prikey[32];
    unsigned char pubkey[64];  /* (x, y) */

    unsigned int msg_length;
    unsigned char msg[20];
    unsigned char cipher[20+64+32];
}__attribute__((aligned(4)));

struct pka_sm2_encdec_data pka_sm2_encdec_testcase[] = {
{
    .prikey = {
        0x39,0x45,0x20,0x8F,0x7B,0x21,0x44,0xB1,0x3F,0x36,0xE3,0x8A,0xC6,0xD3,0x9F,0x95,
        0x88,0x93,0x93,0x69,0x28,0x60,0xB5,0x1A,0x42,0xFB,0x81,0xEF,0x4D,0xF7,0xC5,0xB8},
    .pubkey = {
        0x09,0xF9,0xDF,0x31,0x1E,0x54,0x21,0xA1,0x50,0xDD,0x7D,0x16,0x1E,0x4B,0xC5,0xC6,
        0x72,0x17,0x9F,0xAD,0x18,0x33,0xFC,0x07,0x6B,0xB0,0x8F,0xF3,0x56,0xF3,0x50,0x20,
        0xCC,0xEA,0x49,0x0C,0xE2,0x67,0x75,0xA5,0x2D,0xC6,0xEA,0x71,0x8C,0xC1,0xAA,0x60,
        0x0A,0xED,0x05,0xFB,0xF3,0x5E,0x08,0x4A,0x66,0x32,0xF6,0x07,0x2D,0xA9,0xAD,0x13},
    .msg_length = 19,
    .msg = {
        0x65,0x6E,0x63,0x72,0x79,0x70,0x74,0x69,0x6F,0x6E,0x20,0x73,0x74,0x61,0x6E,0x64,
        0x61,0x72,0x64},
    .cipher = {
        0x04,0xEB,0xFC,0x71,0x8E,0x8D,0x17,0x98,0x62,0x04,0x32,0x26,0x8E,0x77,0xFE,0xB6,
        0x41,0x5E,0x2E,0xDE,0x0E,0x07,0x3C,0x0F,0x4F,0x64,0x0E,0xCD,0x2E,0x14,0x9A,0x73,
        0xE8,0x58,0xF9,0xD8,0x1E,0x54,0x30,0xA5,0x7B,0x36,0xDA,0xAB,0x8F,0x95,0x0A,0x3C,
        0x64,0xE6,0xEE,0x6A,0x63,0x09,0x4D,0x99,0x28,0x3A,0xFF,0x76,0x7E,0x12,0x4D,0xF0,
        0x21,0x88,0x6C,0xA9,0x89,0xCA,0x9C,0x7D,0x58,0x08,0x73,0x07,0xCA,0x93,0x09,0x2D,
        0x65,0x1E,0xFA,0x59,0x98,0x3C,0x18,0xF8,0x09,0xE2,0x62,0x92,0x3C,0x53,0xAE,0xC2,
        0x95,0xD3,0x03,0x83,0xB5,0x4E,0x39,0xD6,0x09,0xD1,0x60,0xAF,0xCB,0x19,0x08,0xD0,
        0xBD,0x87,0x66},
},
};

static int mss_pka_sm2_encdec(struct pka_sm2_encdec_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 *pubKey;
    mt_u8 *priKey;
    mt_u8 *msg;
    mt_u32 msg_len;
    mt_u8 *cipher;
    mt_u32 cipher_len;

    mt_u8 cipherout[64+32+20];
    mt_u8 msgout[20];
    mt_u32 cipheroutlen = 64+32+20;
    mt_u32 msgoutlen = 20;

    pubKey = testcase->pubkey;
    priKey = testcase->prikey;
    msg = testcase->msg;
    msg_len = testcase->msg_length;
    cipher = testcase->cipher;
    cipher_len = msg_len + 64 + 32;


    mss_dump("sm2 cipher", cipher, cipher_len);
    ret = mt_unf_ecc_sm2_decrypt(priKey, cipher, cipher_len, msgout, &msgoutlen);
    if(ret != MT_SUCCESS) {
        printf("tee_sm2_decrypt Failed!\n");
        return MT_FAILURE;
    }
    mss_dump("msgout", msgout, msgoutlen);

    if (memcmp(msgout, msg, msgoutlen) == 0) {
        printf("SM2 decrypt test ##### SUCCESS\n");
        ret = MT_SUCCESS;
    } else {
        printf("SM2 decrypt test Failed!\n");
        return MT_FAILURE;
    }

    mss_dump("sm2 msg", msg, msg_len);
    ret = mt_unf_ecc_sm2_encrypt(&pubKey[0], &pubKey[32], msg, msg_len, cipherout, &cipheroutlen);
    if(ret != MT_SUCCESS) {
        printf("tee_sm2_encrypt Failed!\n");
        return MT_FAILURE;
    }
    mss_dump("cipherout", cipherout, cipheroutlen);

    ret = mt_unf_ecc_sm2_decrypt(priKey, cipherout, cipheroutlen, msgout, &msgoutlen);
    if(ret != MT_SUCCESS) {
        printf("tee_sm2_decrypt Failed!\n");
        return MT_FAILURE;
    }
    mss_dump("msgout", msgout, msgoutlen);

    if (memcmp(msgout, msg, msgoutlen) == 0) {
        printf("SM2 encrypt-decrypt test ##### SUCCESS\n");
        ret = MT_SUCCESS;
    } else {
        printf("SM2 encrypt-decrypt test Failed!\n");
        return MT_FAILURE;
    }

    return ret;
}

#ifdef CONFIG_MT_TEE_SUPPORT
static int mss_pka_sm2_encdec_tee(struct pka_sm2_encdec_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 *pubKey;
    mt_u8 *priKey;
    mt_u8 *msg;
    mt_u32 msg_len;
    mt_u8 *cipher;
    mt_u32 cipher_len;

    mt_u8 cipherout[64+32+20];
    mt_u8 msgout[20];
    mt_u32 cipheroutlen = 64+32+20;
    mt_u32 msgoutlen = 20;

    pubKey = testcase->pubkey;
    priKey = testcase->prikey;
    msg = testcase->msg;
    msg_len = testcase->msg_length;
    cipher = testcase->cipher;
    cipher_len = msg_len + 64 + 32;


    mss_dump("sm2 cipher", cipher, cipher_len);
    ret = tee_sm2_decrypt(priKey, cipher, cipher_len, msgout, &msgoutlen);
    if(ret != MT_SUCCESS) {
        printf("tee_sm2_decrypt Failed!\n");
        return MT_FAILURE;
    }
    mss_dump("msgout", msgout, msgoutlen);

    if (memcmp(msgout, msg, msgoutlen) == 0) {
        printf("SM2 decrypt test ##### SUCCESS\n");
        ret = MT_SUCCESS;
    } else {
        printf("SM2 decrypt test Failed!\n");
        return MT_FAILURE;
    }

    mss_dump("sm2 msg", msg, msg_len);
    ret = tee_sm2_encrypt(&pubKey[0], &pubKey[32], msg, msg_len, cipherout, &cipheroutlen);
    if(ret != MT_SUCCESS) {
        printf("tee_sm2_encrypt Failed!\n");
        return MT_FAILURE;
    }
    mss_dump("cipherout", cipherout, cipheroutlen);

    ret = tee_sm2_decrypt(priKey, cipherout, cipheroutlen, msgout, &msgoutlen);
    if(ret != MT_SUCCESS) {
        printf("tee_sm2_decrypt Failed!\n");
        return MT_FAILURE;
    }
    mss_dump("msgout", msgout, msgoutlen);

    if (memcmp(msgout, msg, msgoutlen) == 0) {
        printf("SM2 encrypt-decrypt test ##### SUCCESS\n");
        ret = MT_SUCCESS;
    } else {
        printf("SM2 encrypt-decrypt test Failed!\n");
        return MT_FAILURE;
    }

    return ret;
}
#else
static int mss_pka_sm2_encdec_tee(struct pka_sm2_encdec_data *testcase)
{
    printf("not support tee\n");
    return MT_FAILURE;
}
#endif

static int mss_pka_sm2_signverify_test(int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i;
    struct pka_sm2_signverify_data *testcase = NULL;
    MSS_CPU_MODE_E cpu_mode = MSS_APCPU;

    if (argc > 0 && strcmp(argv[0], "-t") == 0) {
        cpu_mode = MSS_SECCPU;
            argc--;
            argv++;
    }

    for (i = 0; i < ARRAY_SIZE(pka_sm2_signverify_testcase); i++) {
        testcase =  &pka_sm2_signverify_testcase[i];
        if (cpu_mode == MSS_SECCPU)
            ret = mss_pka_sm2_sign_verify_tee(testcase);
        else
            ret = mss_pka_sm2_sign_verify(testcase);
        if (ret != MT_SUCCESS) {
            printf("%s[%d] FAIL!\n", __FUNCTION__,i);
            break;
        }
    }

    return ret;
}

static int mss_pka_sm2_encdec_test(int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i;
    struct pka_sm2_encdec_data *testcase = NULL;
    MSS_CPU_MODE_E cpu_mode = MSS_APCPU;

    if (argc > 0 && strcmp(argv[0], "-t") == 0) {
        cpu_mode = MSS_SECCPU;
            argc--;
            argv++;
    }

    for (i = 0; i < ARRAY_SIZE(pka_sm2_encdec_testcase); i++) {
        testcase =  &pka_sm2_encdec_testcase[i];
        if (cpu_mode == MSS_SECCPU)
            ret = mss_pka_sm2_encdec_tee(testcase);
        else
            ret = mss_pka_sm2_encdec(testcase);
        if (ret != MT_SUCCESS) {
            printf("%s[%d] FAIL!\n", __FUNCTION__,i);
            break;
        }
    }

    return ret;
}

int mss_pka_sm2_test(int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;

    if (argc == 0)
        return MT_FAILURE;

    if (0 == strcmp("encdec", argv[0])){
        ret = mss_pka_sm2_encdec_test(--argc, ++argv);
    } else if (0 == strcmp("sign", argv[0])){
        ret =mss_pka_sm2_signverify_test(--argc, ++argv);
    }

    return ret;
}

struct ecc_curve
{
    char name[16];
    uint32_t key_size;
    uint8_t p[32];
    uint8_t a[32];
    uint8_t b[32];
    uint8_t n[32];
    uint8_t gx[32];
    uint8_t gy[32];
}__attribute__((aligned(4)));

struct ecc_curve curves[] = {
{
    .name = "secp256k1",
    .key_size = 32,
    .p = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0xff,0xff,0xfc,0x2f},
    .a = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    .b = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07},
    .n = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0xba,0xae,0xdc,0xe6,0xaf,0x48,0xa0,0x3b,0xbf,0xd2,0x5e,0x8c,0xd0,0x36,0x41,0x41},
    .gx = {0x79,0xbe,0x66,0x7e,0xf9,0xdc,0xbb,0xac,0x55,0xa0,0x62,0x95,0xce,0x87,0x0b,0x07,0x02,0x9b,0xfc,0xdb,0x2d,0xce,0x28,0xd9,0x59,0xf2,0x81,0x5b,0x16,0xf8,0x17,0x98},
    .gy = {0x48,0x3a,0xda,0x77,0x26,0xa3,0xc4,0x65,0x5d,0xa4,0xfb,0xfc,0x0e,0x11,0x08,0xa8,0xfd,0x17,0xb4,0x48,0xa6,0x85,0x54,0x19,0x9c,0x47,0xd0,0x8f,0xfb,0x10,0xd4,0xb8},
},
{
    .name = "secp256r1",
    .key_size = 32,
    .p = {0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
    .a = {0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfc},
    .b = {0x5a,0xc6,0x35,0xd8,0xaa,0x3a,0x93,0xe7,0xb3,0xeb,0xbd,0x55,0x76,0x98,0x86,0xbc,0x65,0x1d,0x06,0xb0,0xcc,0x53,0xb0,0xf6,0x3b,0xce,0x3c,0x3e,0x27,0xd2,0x60,0x4b},
    .n = {0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBC,0xE6,0xFA,0xAD,0xA7,0x17,0x9E,0x84,0xF3,0xB9,0xCA,0xC2,0xFC,0x63,0x25,0x51},
    .gx = {0x6b,0x17,0xd1,0xf2,0xe1,0x2c,0x42,0x47,0xf8,0xbc,0xe6,0xe5,0x63,0xa4,0x40,0xf2,0x77,0x03,0x7d,0x81,0x2d,0xeb,0x33,0xa0,0xf4,0xa1,0x39,0x45,0xd8,0x98,0xc2,0x96},
    .gy = {0x4f,0xe3,0x42,0xe2,0xfe,0x1a,0x7f,0x9b,0x8e,0xe7,0xeb,0x4a,0x7c,0x0f,0x9e,0x16,0x2b,0xce,0x33,0x57,0x6b,0x31,0x5e,0xce,0xcb,0xb6,0x40,0x68,0x37,0xbf,0x51,0xf5},
},
};

typedef enum {
	secp256k1 = 0,
        secp256r1,
} CURVE_TYPE;

struct pka_ecdh_data
{
    struct ecc_curve *curve;
    uint8_t priKeyA[32];
    uint8_t pubKeyA[64];
    uint8_t priKeyB[32];
    uint8_t pubKeyB[64];
}__attribute__((aligned(4)));

struct pka_ecdh_data pka_ecdh_testcase[] = {
{
    .curve = &curves[secp256r1],
    .priKeyA = {
        0x70,0x83,0x09,0xa7,0x44,0x9e,0x15,0x6b,0x0d,0xb7,0x0e,0x5b,0x52,0xe6,0x06,0xc7,
        0xe0,0x94,0xed,0x67,0x6c,0xe8,0x95,0x3b,0xf6,0xc1,0x47,0x57,0xc8,0x26,0xf5,0x90},
    .pubKeyA = {
        0x29,0x57,0x8c,0x7a,0xb6,0xce,0x0d,0x11,0x49,0x3c,0x95,0xd5,0xea,0x05,0xd2,0x99,
        0xd5,0x36,0x80,0x1c,0xa9,0xcb,0xd5,0x0e,0x99,0x24,0xe4,0x3b,0x73,0x3b,0x83,0xab,
        0x08,0xc8,0x04,0x98,0x79,0xc6,0x27,0x8b,0x22,0x73,0x34,0x84,0x74,0x15,0x85,0x15,
        0xac,0xca,0xa3,0x83,0x44,0x10,0x6e,0xf9,0x68,0x03,0xc5,0xa0,0x5a,0xdc,0x48,0x00},
    .priKeyB = {
        0x90,0xc5,0x38,0x61,0x00,0xb1,0x37,0xa7,0x5b,0x0b,0xb4,0x95,0x00,0x2b,0x28,0x69,
        0x7a,0x45,0x1a,0xdd,0x2f,0x1f,0x22,0xcb,0x65,0xf7,0x35,0xe8,0xaa,0xea,0xce,0x98},
     .pubKeyB = {
        0x4a,0x92,0x39,0x6f,0xf7,0x93,0x0b,0x1d,0xa9,0xa8,0x73,0xa4,0x79,0xa2,0x8a,0x98,
        0x96,0xaf,0x6c,0xc3,0xd3,0x93,0x45,0xb9,0x49,0xb7,0x26,0xdc,0x3c,0xd9,0x78,0xb5,
        0x47,0x5a,0xbb,0x18,0xea,0xed,0x94,0x88,0x79,0xb9,0xc1,0x45,0x3e,0x3e,0xf2,0x75,
        0x5d,0xd9,0x0f,0x77,0x51,0x9e,0xc7,0xb6,0xa3,0x02,0x97,0xaa,0xd0,0x8e,0x49,0x31},
},
};

static int mss_pka_ecdh(struct pka_ecdh_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    MT_CIPHER_EC_PARAMS_S xParams;
    mt_u8 *priKeyA;
    mt_u8 *pubKeyA;
    mt_u8 *priKeyB;
    mt_u8 *pubKeyB;
    mt_u8 keyout1[64];
    mt_u8 keyout2[64];

    memset(&xParams, 0, sizeof(xParams));
    xParams.q = testcase->curve->p;
    xParams.a = testcase->curve->a;
    xParams.b = testcase->curve->b;
    xParams.n = testcase->curve->n;
    xParams.GX = testcase->curve->gx;
    xParams.GY = testcase->curve->gy;
    xParams.keySize = testcase->curve->key_size;

    priKeyA = testcase->priKeyA;
    pubKeyA = testcase->pubKeyA;
    priKeyB = testcase->priKeyB;
    pubKeyB = testcase->pubKeyB;

    /*ecdh gen keypair test */
    {
        ret = mt_unf_ecc_ecdh_gen_keypair(&xParams, keyout2, &keyout1[0], &keyout1[32]);
        if(ret != MT_SUCCESS) {
            printf("tee_ecdh_gen_pubkey Failed!\n");
            return MT_FAILURE;
        }
        mss_dump("priKey", keyout2, 32);
        mss_dump("pubKey out", keyout1, 64);

        /*ecc point check test */
        ret = mt_unf_ecc_point_check(&xParams, &keyout1[0], &keyout1[32]);
        if(ret == MT_SUCCESS) {
            printf("ecc point check test ##### SUCCESS\n");
        } else {
            printf("ecc point check test Failed!\n");
            return MT_FAILURE;
        }
    }

    /*ecdh gen pubkey test */
    {
        mss_dump("priKeyA", priKeyA, 32);

        ret = mt_unf_ecc_ecdh_gen_pubkey(&xParams, priKeyA, &keyout1[0], &keyout1[32]);
        if(ret != MT_SUCCESS) {
            printf("tee_ecdh_gen_pubkey Failed!\n");
            return MT_FAILURE;
        }
        mss_dump("pubKeyA out", keyout1, 64);
        if (memcmp(keyout1, pubKeyA, 64) == 0) {
            printf("ecdh gen pubkey test ##### SUCCESS\n");
            ret = MT_SUCCESS;
        } else {
            printf("ecdh gen pubkey test Failed!\n");
            return MT_FAILURE;
        }
    }

    /*ecdh gen sharekey test */
    {
        ret = mt_unf_ecc_ecdh_gen_sharekey(&xParams, priKeyA, &pubKeyB[0], &pubKeyB[32],
                    &keyout1[0], &keyout1[32]);
        if(ret != MT_SUCCESS) {
            printf("tee_ecdh_gen_sharekey Failed!\n");
            return MT_FAILURE;
        }
        mss_dump("shareKey1", keyout1, 64);

        ret = mt_unf_ecc_ecdh_gen_sharekey(&xParams, priKeyB, &pubKeyA[0], &pubKeyA[32],
                    &keyout2[0], &keyout2[32]);
        if(ret != MT_SUCCESS) {
            printf("tee_ecdh_gen_sharekey Failed!\n");
            return MT_FAILURE;
        }
        mss_dump("shareKey2", keyout2, 64);

        if (memcmp(keyout1, keyout2, 64) == 0) {
            printf("ecdh gen sharekey test ##### SUCCESS\n");
            ret = MT_SUCCESS;
        } else {
            printf("ecdh gen sharekey test Failed!\n");
            return MT_FAILURE;
        }
    }
    return ret;
}

#ifdef CONFIG_MT_TEE_SUPPORT
static int mss_pka_ecdh_tee(struct pka_ecdh_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    MT_CIPHER_EC_PARAMS_S xParams;
    mt_u8 *priKeyA;
    mt_u8 *pubKeyA;
    mt_u8 *priKeyB;
    mt_u8 *pubKeyB;
    mt_u8 keyout1[64];
    mt_u8 keyout2[64];

    memset(&xParams, 0, sizeof(xParams));
    xParams.q = testcase->curve->p;
    xParams.a = testcase->curve->a;
    xParams.b = testcase->curve->b;
    xParams.n = testcase->curve->n;
    xParams.GX = testcase->curve->gx;
    xParams.GY = testcase->curve->gy;
    xParams.keySize = testcase->curve->key_size;

    priKeyA = testcase->priKeyA;
    pubKeyA = testcase->pubKeyA;
    priKeyB = testcase->priKeyB;
    pubKeyB = testcase->pubKeyB;

    /*ecdh gen keypair test */
    {
        ret = tee_ecdh_gen_keypair(&xParams, keyout2, &keyout1[0], &keyout1[32]);
        if(ret != MT_SUCCESS) {
            printf("tee_ecdh_gen_pubkey Failed!\n");
            return MT_FAILURE;
        }
        mss_dump("priKey", keyout2, 32);
        mss_dump("pubKey out", keyout1, 64);

        /*ecc point check test */
        ret = tee_ecc_point_check(&xParams, &keyout1[0], &keyout1[32]);
        if(ret == MT_SUCCESS) {
            printf("ecc point check test ##### SUCCESS\n");
        } else {
            printf("ecc point check test Failed!\n");
            return MT_FAILURE;
        }
    }

    /*ecdh gen pubkey test */
    {
        mss_dump("priKeyA", priKeyA, 32);

        ret = tee_ecdh_gen_pubkey(&xParams, priKeyA, &keyout1[0], &keyout1[32]);
        if(ret != MT_SUCCESS) {
            printf("tee_ecdh_gen_pubkey Failed!\n");
            return MT_FAILURE;
        }
        mss_dump("pubKeyA out", keyout1, 64);
        if (memcmp(keyout1, pubKeyA, 64) == 0) {
            printf("ecdh gen pubkey test ##### SUCCESS\n");
            ret = MT_SUCCESS;
        } else {
            printf("ecdh gen pubkey test Failed!\n");
            return MT_FAILURE;
        }
    }

    /*ecdh gen sharekey test */
    {
        ret = tee_ecdh_gen_sharekey(&xParams, priKeyA, &pubKeyB[0], &pubKeyB[32],
                    &keyout1[0], &keyout1[32]);
        if(ret != MT_SUCCESS) {
            printf("tee_ecdh_gen_sharekey Failed!\n");
            return MT_FAILURE;
        }
        mss_dump("shareKey1", keyout1, 64);

        ret = tee_ecdh_gen_sharekey(&xParams, priKeyB, &pubKeyA[0], &pubKeyA[32],
                    &keyout2[0], &keyout2[32]);
        if(ret != MT_SUCCESS) {
            printf("tee_ecdh_gen_sharekey Failed!\n");
            return MT_FAILURE;
        }
        mss_dump("shareKey2", keyout2, 64);

        if (memcmp(keyout1, keyout2, 64) == 0) {
            printf("ecdh gen sharekey test ##### SUCCESS\n");
            ret = MT_SUCCESS;
        } else {
            printf("ecdh gen sharekey test Failed!\n");
            return MT_FAILURE;
        }
    }
    return ret;
}
#else
static int mss_pka_ecdh_tee(struct pka_ecdh_data *testcase)
{
    printf("not support tee\n");
    return MT_FAILURE;
}
#endif

int mss_pka_ecdh_test(int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i;
    struct pka_ecdh_data *testcase = NULL;
    MSS_CPU_MODE_E cpu_mode = MSS_APCPU;

    if (argc > 0 && strcmp(argv[0], "-t") == 0) {
        cpu_mode = MSS_SECCPU;
            argc--;
            argv++;
    }

    for (i = 0; i < ARRAY_SIZE(pka_ecdh_testcase); i++) {
        testcase =  &pka_ecdh_testcase[i];
        if (cpu_mode == MSS_SECCPU)
            ret = mss_pka_ecdh_tee(testcase);
        else
            ret = mss_pka_ecdh(testcase);
        if (ret != MT_SUCCESS) {
            printf("%s[%d] FAIL!\n", __FUNCTION__,i);
            break;
        }
    }

    return ret;
}

struct pka_ecdsa_data
{
    struct ecc_curve *curve;
    uint32_t hashtype;
    uint8_t priKey[32];
    uint8_t pubKey[64];
    uint32_t msg_len;
    uint8_t msg[32];
    uint8_t signature[64];
}__attribute__((aligned(4)));

struct pka_ecdsa_data pka_ecdsa_testcase[] = {
{
    .curve = &curves[secp256k1],
    .hashtype = MT_CIPHER_HASH_TYPE_SHA256,
    .priKey = {
        0x83,0x5c,0x16,0x15,0x64,0x5e,0xe9,0xb7,0xe4,0x80,0xd1,0xfe,0x2a,0xce,0x9d,0x4c,
        0x94,0xe5,0x99,0xb0,0x5a,0xa4,0xff,0xba,0x34,0x30,0xc2,0xc7,0xcf,0xc7,0xe5,0x46},
     .pubKey = {
        0x36,0x49,0x26,0xc7,0x62,0x50,0x65,0xc8,0x1d,0x29,0xe5,0x50,0x21,0x85,0x7b,0x2d,
        0x0e,0x4b,0xfa,0x5d,0x62,0xde,0x95,0x63,0xb9,0x0e,0xc5,0x55,0xdc,0x71,0x6c,0x39,
        0xfc,0xd5,0x1e,0xd1,0x35,0x36,0xf3,0x55,0xfe,0x9f,0xef,0x0b,0xf8,0xd7,0x28,0xbe,
        0xae,0x28,0x5c,0xc1,0xb3,0x53,0x43,0x7a,0xb3,0x9f,0xef,0x4f,0xe0,0x27,0xca,0x1c},
    .msg_len = 8,
    .msg = {
        0x48,0x65,0x6c,0x6c,0x6f,0x31,0x32,0x33},
    .signature = {
        0x6e,0x0c,0xef,0x3b,0xa0,0x10,0x0b,0x85,0x18,0xd0,0xf9,0x8e,0xce,0x7c,0x5b,0x16,
        0x02,0xdf,0xa4,0xb2,0x64,0x57,0xd8,0x83,0x46,0xb6,0x58,0xfa,0x5a,0x55,0x47,0x1a,
        0xaf,0x7f,0xec,0x42,0x6e,0x21,0xf8,0xb3,0xb0,0x18,0x25,0x9f,0x41,0x0e,0x85,0xc9,
        0x17,0xf5,0x49,0xa1,0x61,0x34,0x13,0x5a,0x93,0x79,0xb5,0xf1,0xc5,0x59,0x16,0xc5},
},
};

static int mss_pka_ecdsa(struct pka_ecdsa_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    MT_CIPHER_EC_PARAMS_S xParams;
    MT_CIPHER_HASH_TYPE_E hashtype;
    mt_u8 *priKey;
    mt_u8 *pubKey;
    mt_u32 msg_len;
    mt_u8 *msg;
    mt_u8 *signature;
    mt_u8 sign_out[64];

    memset(&xParams, 0, sizeof(xParams));
    xParams.q = testcase->curve->p;
    xParams.a = testcase->curve->a;
    xParams.b = testcase->curve->b;
    xParams.n = testcase->curve->n;
    xParams.GX = testcase->curve->gx;
    xParams.GY = testcase->curve->gy;
    xParams.keySize = testcase->curve->key_size;

    priKey = testcase->priKey;
    pubKey = testcase->pubKey;
    msg_len = testcase->msg_len;
    msg = testcase->msg;
    signature = testcase->signature;

    hashtype = testcase->hashtype;

    /* ecdsa verify test */
    {
        mss_dump("pubKey", pubKey, xParams.keySize * 2);
        mss_dump("msg", msg, msg_len);

        ret = mt_unf_ecc_ecdsa_verify(&xParams, hashtype, &pubKey[0], &pubKey[32],
                    msg, msg_len, &signature[0], &signature[32]);
        if(ret == MT_SUCCESS) {
            printf("ecdsa verify test ##### SUCCESS\n");
        } else {
            printf("ecdsa verify test Failed!\n");
            return MT_FAILURE;
        }
    }

    /* ecdsa sign-verify test */
    {
        ret = mt_unf_ecc_ecdsa_sign(&xParams, hashtype, priKey, msg, msg_len, &sign_out[0], &sign_out[32]);
        if(ret != MT_SUCCESS) {
            printf("tee_ecdsa_sign Failed!\n");
            return MT_FAILURE;
        }
        mss_dump("sign_out", sign_out, xParams.keySize * 2);

        ret = mt_unf_ecc_ecdsa_verify(&xParams, hashtype, &pubKey[0], &pubKey[32],
                    msg, msg_len, &sign_out[0], &sign_out[32]);
        if(ret == MT_SUCCESS) {
            printf("ecdsa sign-verify test ##### SUCCESS\n");
        } else {
            printf("ecdsa sign-verify test Failed!\n");
            return MT_FAILURE;
        }
    }
    return ret;
}

#ifdef CONFIG_MT_TEE_SUPPORT
static int mss_pka_ecdsa_tee(struct pka_ecdsa_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    MT_CIPHER_EC_PARAMS_S xParams;
    MT_CIPHER_HASH_TYPE_E hashtype;
    mt_u8 *priKey;
    mt_u8 *pubKey;
    mt_u32 msg_len;
    mt_u8 *msg;
    mt_u8 *signature;
    mt_u8 sign_out[64];

    memset(&xParams, 0, sizeof(xParams));
    xParams.q = testcase->curve->p;
    xParams.a = testcase->curve->a;
    xParams.b = testcase->curve->b;
    xParams.n = testcase->curve->n;
    xParams.GX = testcase->curve->gx;
    xParams.GY = testcase->curve->gy;
    xParams.keySize = testcase->curve->key_size;

    priKey = testcase->priKey;
    pubKey = testcase->pubKey;
    msg_len = testcase->msg_len;
    msg = testcase->msg;
    signature = testcase->signature;

    hashtype = testcase->hashtype;

    /* ecdsa verify test */
    {
        mss_dump("pubKey", pubKey, xParams.keySize * 2);
        mss_dump("msg", msg, msg_len);

        ret = tee_ecdsa_verify(&xParams, hashtype, &pubKey[0], &pubKey[32],
                    msg, msg_len, &signature[0], &signature[32]);
        if(ret == MT_SUCCESS) {
            printf("ecdsa verify test ##### SUCCESS\n");
        } else {
            printf("ecdsa verify test Failed!\n");
            return MT_FAILURE;
        }
    }

    /* ecdsa sign-verify test */
    {
        ret = tee_ecdsa_sign(&xParams, hashtype, priKey, msg, msg_len, &sign_out[0], &sign_out[32]);
        if(ret != MT_SUCCESS) {
            printf("tee_ecdsa_sign Failed!\n");
            return MT_FAILURE;
        }
        mss_dump("sign_out", sign_out, xParams.keySize * 2);

        ret = tee_ecdsa_verify(&xParams, hashtype, &pubKey[0], &pubKey[32],
                    msg, msg_len, &sign_out[0], &sign_out[32]);
        if(ret == MT_SUCCESS) {
            printf("ecdsa sign-verify test ##### SUCCESS\n");
        } else {
            printf("ecdsa sign-verify test Failed!\n");
            return MT_FAILURE;
        }
    }
    return ret;
}
#else
static int mss_pka_ecdsa_tee(struct pka_ecdsa_data *testcase)
{
    printf("not support tee\n");
    return MT_FAILURE;
}
#endif

int mss_pka_ecdsa_test(int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i;
    struct pka_ecdsa_data *testcase = NULL;
    MSS_CPU_MODE_E cpu_mode = MSS_APCPU;

    if (argc > 0 && strcmp(argv[0], "-t") == 0) {
        cpu_mode = MSS_SECCPU;
            argc--;
            argv++;
    }

    for (i = 0; i < ARRAY_SIZE(pka_ecdsa_testcase); i++) {
        testcase =  &pka_ecdsa_testcase[i];
        if (cpu_mode == MSS_SECCPU)
            ret = mss_pka_ecdsa_tee(testcase);
        else
            ret = mss_pka_ecdsa(testcase);
        if (ret != MT_SUCCESS) {
            printf("%s[%d] FAIL!\n", __FUNCTION__,i);
            break;
        }
    }

    return ret;
}

