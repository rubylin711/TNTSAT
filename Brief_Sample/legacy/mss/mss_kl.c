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
#include "mss_cmd_utils.h"
#include "mss_tee_client.h"

struct mss_crypto_kldata
{
    unsigned int rootkey;
    unsigned int level;
    unsigned int algo;
    unsigned int op;
    unsigned char const_keys[16*3];
    unsigned char signature[32];

    unsigned char iv[16];

    unsigned int length;
    unsigned char plain[32];
    unsigned char cipher_aes_ecb[32];
    unsigned char cipher_tdes_cbc[32];
}__attribute__((aligned(4)));

struct mss_crypto_kldata mss_crypto_kl_testcase[] = {
{
    // the following test data from kdf_chip.py
    // CASID:0xfa MODULEID:0xb
    // testchip: #105

    // sck0: 1a45e2665bdfc8b769bd3cdb9b60b7b3
    // clearkey: 65b04a00928a0b91f098090c3b4ac3c1
    .rootkey = MT_CIPHER_KEYLADDER_SCK_0,
    .level = 3,
    .algo = MT_CIPHER_ALG_TDES,
    .op = MT_CIPHER_OPERATION_DECRYPT,
    .const_keys = {
        0xE4,0x21,0x13,0xA7,0x99,0xCA,0xF4,0xC7,0xE5,0xC5,0x0A,0xC9,0xD5,0xA5,0x03,0x11,
        0xE4,0x21,0x13,0xA7,0x99,0xCA,0xF4,0xC7,0xE5,0xC5,0x0A,0xC9,0xD5,0xA5,0x03,0x13,
        0x2D,0x08,0x60,0xDA,0xE7,0xFD,0xB0,0xBD,0x4B,0xFA,0xB1,0x11,0xF6,0x15,0x22,0x7A},
    .signature = {
        // SCK0, 3, TDES
        0xE2,0x5B,0xCA,0x62,0xB0,0xAF,0xDE,0xEC,0x9B,0xA1,0x7A,0x97,0x1A,0x49,0x72,0x75,
        0xE4,0xD7,0x31,0xFE,0x5D,0x74,0x6B,0x86,0xC5,0xF5,0xCB,0x3F,0x62,0xAA,0x3B,0xAB},
    .iv = {0x06,0xf1,0x7c,0x62,0x09,0xb7,0xb5,0xd8,0xee,0xfc,0x1f,0xdd,0x1d,0xc3,0x3e,0x7c},
    .length = 16,
    .plain = {0x06,0xf1,0x7c,0x62,0x09,0xb7,0xb5,0xd8,0xee,0xfc,0x1f,0xdd,0x1d,0xc3,0x3e,0x7c},
    .cipher_aes_ecb = {0xf5,0xa2,0x37,0x6b,0xa8,0xb0,0x79,0x72,0x95,0x89,0xf1,0x00,0xc2,0xe0,0x94,0x3e},
    .cipher_tdes_cbc = {0x06,0xf1,0x7c,0x62,0x09,0xb7,0xb5,0xd8,0xee,0xfc,0x1f,0xdd,0x1d,0xc3,0x3e,0x7c},
},
};

static mt_s32 mss_kl_key_set(mt_u32 keyslot, struct mss_crypto_kldata *testcase)
{
    mt_s32 ret = 0;
    mt_handle p_keyladder = 0;
    MT_CIPHER_CTRL_S p_ctrl = {0,};
    mt_u32 algo;
    mt_u32 op;
    mt_u32 rootsck;
    mt_u32 level;
    mt_u8 *signature;
    mt_u8 *keys;
    mt_u32 swap = 0;
    mt_u32 i;

    algo = testcase->algo;
    op = testcase->op;
    rootsck = testcase->rootkey;
    level = testcase->level;
    signature = testcase->signature;
    keys = testcase->const_keys;

    printf("rootsck:%d level:%d algo:%d op:%d\n", rootsck, level, algo, op);

    p_ctrl.operation = op;
    p_ctrl.algorithm = algo;

    ret = mt_unf_cipher_keyladder_create(0, &p_keyladder);
    MT_ASSERT(ret == MT_SUCCESS);

    /* set rootkey */
    ret = mt_unf_cipher_keyladder_start(p_keyladder, rootsck);
    MT_ASSERT(ret == MT_SUCCESS);

    /* if OTP_KLCmdAuthEn != 1, need set signature */
    if (1) {
        mss_dump("signature", signature, 0x20);
        ret = mt_unf_cipher_keyladder_set_signature(p_keyladder, signature);
        MT_ASSERT(ret == MT_SUCCESS);
    }

    for (i = 0; i < level; i++) {
        mss_dump("link key", keys + i * 16, 16);
        ret = mt_unf_cipher_keyladder_link(p_keyladder, &p_ctrl, keys + i * 16, 16);
        MT_ASSERT(ret == MT_SUCCESS);
    }

    /* enable swap mode: swap msb 64 bits and lsb 64 bits */
    if (swap) {
        ret = mt_unf_cipher_keyladder_extra_infor(p_keyladder, MT_CIPHER_KL_ADDT_EXPORTSWAP, MT_CIPHER_ENABLE);
        MT_ASSERT(ret == MT_SUCCESS);
    }

    /* export to keyslot */
    ret = mt_unf_cipher_keyladder_end(p_keyladder, keyslot);
    MT_ASSERT(ret == MT_SUCCESS);

    /* disable swap mode */
    if (swap) {
        ret = mt_unf_cipher_keyladder_extra_infor(p_keyladder, MT_CIPHER_KL_ADDT_EXPORTSWAP, MT_CIPHER_DISABLE);
        MT_ASSERT(ret == MT_SUCCESS);
    }

    ret = mt_unf_cipher_keyladder_destroy(p_keyladder);

    mt_unf_cipher_keyslot_info(keyslot);

    return ret;
}

#ifdef CONFIG_MT_TEE_SUPPORT
static mt_s32 mss_kl_key_set_tee(mt_u32 keyslot, struct mss_crypto_kldata *testcase)
{
    mt_s32 ret = 0;
    mt_u32 p_keyladder = 0;
    MT_CIPHER_CTRL_S p_ctrl = {0,};
    mt_u32 algo;
    mt_u32 op;
    mt_u32 rootsck;
    mt_u32 level;
    mt_u8 *signature;
    mt_u8 *keys;
    mt_u32 i;

    algo = testcase->algo;
    op = testcase->op;
    rootsck = testcase->rootkey;
    level = testcase->level;
    signature = testcase->signature;
    keys = testcase->const_keys;

    printf("rootsck:%d level:%d algo:%d op:%d\n", rootsck, level, algo, op);

    p_ctrl.operation = op;
    p_ctrl.algorithm = algo;

    ret = tee_keyladder_create(0, &p_keyladder);
    MT_ASSERT(ret == MT_SUCCESS);

    /* set rootkey */
    ret = tee_keyladder_start(p_keyladder, rootsck);
    MT_ASSERT(ret == MT_SUCCESS);

    /* if OTP_KLCmdAuthEn != 1, need set signature */
    if (1) {
        mss_dump("signature", signature, 0x20);
        ret = tee_keyladder_set_signature(p_keyladder, signature, 0x20);
        MT_ASSERT(ret == MT_SUCCESS);
    }

    for (i = 0; i < level; i++) {
        mss_dump("link key", keys + i * 16, 16);
        ret = tee_keyladder_link(p_keyladder, &p_ctrl, keys + i * 16, 16);
        MT_ASSERT(ret == MT_SUCCESS);
    }

    /* export to keyslot */
    ret = tee_keyladder_end(p_keyladder, keyslot);
    MT_ASSERT(ret == MT_SUCCESS);

    ret = tee_keyladder_destroy(p_keyladder);

    mt_unf_cipher_keyslot_info(keyslot);

    return ret;
}
#else
static mt_s32 mss_kl_key_set_tee(mt_u32 keyslot, struct mss_crypto_kldata *testcase)
{
    printf("not support tee\n");
    return MT_FAILURE;
}
#endif

static mt_s32 mss_kl_crypto_test(struct mss_crypto_kldata *testcase)
{
    mt_s32 ret = 0;
    mt_u32 keyslot;
    MT_CIPHER_CTRL_S cipher_ctrl = {0,};
    mt_handle p_cipher = MT_NULL;
    mt_u8 output[32];

    ret = mt_unf_cipher_keyslot_request(&keyslot);
    MT_ASSERT(ret == MT_SUCCESS);

    /* set kl key*/
    ret = mss_kl_key_set(keyslot, testcase);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* set keyslot iv*/
    ret = mt_unf_cipher_keyslot_set_iv(keyslot, testcase->iv, 16);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* crypto test with kl */
    {
        cipher_ctrl.core = MT_CIPHER_CORE_M2M_RAW;
        cipher_ctrl.operation = MT_CIPHER_OPERATION_ENCRYPT;
        cipher_ctrl.algorithm = MT_CIPHER_ALG_AES;
        cipher_ctrl.work_mode = MT_CIPHER_WORK_MODE_ECB;

        /* REE uses channel 0 always. */
        ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &p_cipher);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = mt_unf_cipher_crypto_config(p_cipher, &cipher_ctrl, keyslot);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = mt_unf_cipher_crypto_process(p_cipher, testcase->plain, output, testcase->length);
        if (ret != MT_SUCCESS)
            goto EXIT;

        mt_unf_cipher_crypto_destroy(p_cipher);
        p_cipher = MT_NULL;

        mss_dump("clear", testcase->plain, testcase->length);
        mss_dump("output cipher", output, testcase->length);

        if (memcmp(output, testcase->cipher_aes_ecb, testcase->length) == 0) {
            printf("========== SUCCESS ==========\n");
            ret = MT_SUCCESS;
        } else {
            mss_dump("expect cipher", testcase->cipher_aes_ecb, testcase->length);
        }
    }

EXIT:
    if (p_cipher != MT_NULL)
        mt_unf_cipher_crypto_destroy(p_cipher);

    mt_unf_cipher_keyslot_release(keyslot);

    if (ret != MT_SUCCESS)
        printf("========== FAIL ==========\n");

    return ret;
}

#ifdef CONFIG_MT_TEE_SUPPORT
static mt_s32 mss_kl_crypto_tee_test(struct mss_crypto_kldata *testcase)
{
    mt_s32 ret = 0;
    mt_u32 keyslot;
    MT_CIPHER_CTRL_S cipher_ctrl = {0,};
    mt_u32 p_cipher = 0;
    mt_u8 output[32];

    ret = tee_keyslot_request(1, &keyslot);
    MT_ASSERT(ret == MT_SUCCESS);

    /* set kl key*/
    ret = mss_kl_key_set_tee(keyslot, testcase);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* set keyslot iv*/
    ret = mt_unf_cipher_keyslot_set_iv(keyslot, testcase->iv, 16);
    if (ret != MT_SUCCESS)
        goto EXIT;

    /* crypto test with kl */
    {
        cipher_ctrl.core = MT_CIPHER_CORE_M2M_RAW;
        cipher_ctrl.operation = MT_CIPHER_OPERATION_ENCRYPT;
        cipher_ctrl.algorithm = MT_CIPHER_ALG_TDES;
        cipher_ctrl.work_mode = MT_CIPHER_WORK_MODE_CBC;

        ret = tee_crypto_create(MT_CIPHER_CRYPTO_CH_1, &p_cipher);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = tee_crypto_config(p_cipher, &cipher_ctrl, keyslot);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = tee_crypto_process(p_cipher, testcase->plain, output, testcase->length);
        if (ret != MT_SUCCESS)
            goto EXIT;

        tee_crypto_destroy(p_cipher);
        p_cipher = MT_NULL;

        mss_dump("clear", testcase->plain, testcase->length);
        mss_dump("output cipher", output, testcase->length);

        if (memcmp(output, testcase->cipher_aes_ecb, testcase->length) == 0) {
            printf("========== SUCCESS ==========\n");
            ret = MT_SUCCESS;
        } else {
            mss_dump("expect cipher", testcase->cipher_aes_ecb, testcase->length);
        }
    }

EXIT:
    if (p_cipher != MT_NULL)
        tee_crypto_destroy(p_cipher);

    tee_keyslot_release(keyslot);

    if (ret != MT_SUCCESS)
        printf("========== FAIL ==========\n");

    return ret;
}
#else
static mt_s32 mss_kl_crypto_tee_test(struct mss_crypto_kldata *testcase)
{
    printf("not support tee\n");
    return MT_FAILURE;
}
#endif

static int mss_kl_test_auto(int argc, char *const argv[])
{
    mt_s32 ret = 0;
    MSS_CPU_MODE_E cpu_mode = MSS_APCPU;
    mt_u32 idx;
    mt_u32 i;
    struct mss_crypto_kldata *testcase;

    idx = 0;
    if (argc > 0 && strcmp(argv[0], "-t") == 0) {
        cpu_mode = MSS_SECCPU;
        idx++;
    }

    for (i = 0; i < ARRAY_SIZE(mss_crypto_kl_testcase); i++) {
        testcase = &mss_crypto_kl_testcase[i];

        if (cpu_mode == MSS_SECCPU)
            ret = mss_kl_crypto_tee_test(testcase);
        else
            ret = mss_kl_crypto_test(testcase);

        if (ret != 0)
            return ret;
    }

    return 0;
}

static void mss_kl_cmd_help(void)
{
    mss_print("Usage:\n");

    mss_print("\nkl [-t]\n");
    mss_print("\t [-t]: tee kl test\n");
}

mt_s32 mss_kl_test(int argc, char *const argv[]);

mt_s32 mss_kl_test(int argc, char *const argv[])
{
    int ret;
    mt_u32 i;

    if (0 == strncmp("-h", argv[0], 2)){
        mss_kl_cmd_help();
        return 0;
    }

    for (i = 0; i < argc; i++)
        printf("%s [%d]:%s\n", __FUNCTION__, i, argv[i]);

    ret = mss_kl_test_auto(argc, argv);
    if (ret != 0) {
        printf("\nmss_kl_test_auto FAIL\n");
        return ret;
    }

    printf("\nmss_kl_test_auto SUCCESS\n");

    return 0;
}

