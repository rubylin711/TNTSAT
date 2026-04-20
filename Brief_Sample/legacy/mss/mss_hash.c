/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mt_type.h"
#include "mt_common.h"
#include "mt_unf_cipher_v2.h"
#include "mss_cmd_utils.h"
#include "mss_tee_client.h"

struct mss_hash_data
{
    MT_CIPHER_HASH_TYPE_E type;
    unsigned int msg_length;
    unsigned char *msg;
    unsigned int digest_length;
    unsigned char digest[64];
}__attribute__((aligned(4)));

static unsigned char hashmsg[256] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F, 
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F, 
        0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F, 
        0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F, 
        0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F, 
        0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F, 
        0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F, 
        0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F, 
        0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F, 
        0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, 
        0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF, 
        0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, 
        0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF, 
        0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, 
        0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF, 
        0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF};

struct mss_hash_data mss_hash_testcase[] = {
{
    .type = MT_CIPHER_HASH_TYPE_SHA256,
    .msg_length = 222,
    .msg = hashmsg,
    .digest_length = 32,
    .digest = {
        0x3b,0x7a,0x22,0xd9,0xef,0x08,0x9d,0x4a,0xa3,0x82,0xef,0xf3,0xde,0xeb,0xa7,0x3d,
        0x41,0xe4,0xaf,0x58,0xb0,0x96,0x7e,0x9c,0x86,0x03,0xd8,0x60,0x43,0x1c,0x3e,0xc7},
},
{
    .type = MT_CIPHER_HASH_TYPE_SHA1,
    .msg_length = 222,
    .msg = hashmsg,
    .digest_length = 20,
    .digest = {
        0x55,0x24,0x1a,0x34,0x3c,0xa9,0x7a,0x60,0x2f,0x7a,0x6c,0x71,0x18,0x3f,0xe8,0x97,
        0x69,0x99,0x65,0x1f},
},
{
    .type = MT_CIPHER_HASH_TYPE_SHA224,
    .msg_length = 222,
    .msg = hashmsg,
    .digest_length = 28,
    .digest = {
        0x47,0x2b,0x32,0x2f,0x3b,0xfe,0xd6,0x1c,0xe4,0xea,0xab,0xbf,0x6b,0xba,0xf0,0xf0,
        0xfc,0xea,0x2f,0xe7,0x9a,0x81,0x67,0x99,0x80,0x20,0xae,0xea},
},
{
    .type = MT_CIPHER_HASH_TYPE_SHA384,
    .msg_length = 222,
    .msg = hashmsg,
    .digest_length = 48,
    .digest = {
        0xa5,0x81,0x51,0xfe,0x32,0x11,0xc2,0x76,0x51,0x69,0x3b,0x55,0xe6,0x7c,0xde,0x0e,
        0x88,0x6b,0xb0,0xd8,0xf2,0xb6,0xd9,0x06,0x66,0x15,0x12,0x4c,0xf1,0xda,0x40,0x3d,
        0xfa,0x01,0x4c,0x6f,0x19,0xc1,0xb1,0x0d,0xe7,0xd3,0xbb,0xdb,0xd0,0xab,0x98,0x80},
},
{
    .type = MT_CIPHER_HASH_TYPE_SHA512,
    .msg_length = 222,
    .msg = hashmsg,
    .digest_length = 64,
    .digest = {
        0xc9,0xbd,0x62,0xc0,0xfc,0xe4,0x77,0x36,0xad,0xcd,0x92,0x75,0xb4,0x68,0x45,0xe4,
        0xec,0xa2,0x3b,0x73,0x67,0x86,0x93,0xfe,0xb8,0xe2,0x19,0x09,0xeb,0x84,0x05,0xd4,
        0xb0,0x57,0xaf,0x2a,0xff,0xd7,0xe6,0x67,0xe0,0x47,0xa0,0x7e,0x6a,0xcc,0xad,0xc2,
        0xa5,0x8d,0x73,0x60,0xc1,0x76,0x89,0x76,0x9d,0xb0,0x09,0xf0,0xa7,0x79,0x55,0x60},
},
{
    .type = MT_CIPHER_HASH_TYPE_SM3,
    .msg_length = 222,
    .msg = hashmsg,
    .digest_length = 32,
    .digest = {
        0x12,0x3b,0x07,0x14,0xb7,0xb4,0x09,0xed,0x4d,0x04,0xee,0x97,0xd1,0xfb,0x00,0xf0,
        0xd9,0x1d,0x14,0x68,0x12,0x8c,0x2a,0xe5,0xed,0x86,0x7f,0xd8,0xf2,0xe5,0xa2,0x8f},
},
};

static int mss_hash_ree_test(struct mss_hash_data *testcase, mt_u32 update_cnt)
{
    mt_s32 ret = 0;
    mt_u8 digest[64];
    mt_handle handle;
    mt_u8 *msg;
    mt_u32 msg_len;
    mt_u32 update_len;
    mt_u8 *expect_digest;
    mt_u32 digest_len;
    MT_CIPHER_HASH_TYPE_E hash_type;
    mt_u8 rand;

    msg = testcase->msg;
    update_len = testcase->msg_length;
    expect_digest = testcase->digest;
    digest_len = testcase->digest_length;
    hash_type = testcase->type;

    printf("\n=====================================\n");

    switch (hash_type) {
    case MT_CIPHER_HASH_TYPE_SHA256:
        printf("SHA256");
        break;
    case MT_CIPHER_HASH_TYPE_SHA1:
        printf("SHA1");
        break;
    case MT_CIPHER_HASH_TYPE_SHA224:
        printf("SHA224");
        break;
    case MT_CIPHER_HASH_TYPE_SHA384:
        printf("SHA384");
        break;
    case MT_CIPHER_HASH_TYPE_SHA512:
        printf("SHA512");
        break;
    case MT_CIPHER_HASH_TYPE_SM3:
        printf("SM3");
        break;
    default:
        printf("UNKNOWN hash type\n");
        return -1;
    }
    printf("  update_cnt: %d\n", update_cnt);

    ret = mt_unf_cipher_hash_create(hash_type, NULL, &handle);

    while (update_cnt > 0) {
        update_cnt--;

        if (update_cnt == 0) {
            msg_len = update_len;
        } else {
            mt_unf_cipher_get_random_number(1, &rand);
            msg_len = rand % (update_len - update_cnt);
        }

        mss_dump("msg", msg, msg_len);
        ret |= mt_unf_cipher_hash_update(handle, msg, msg_len);

        msg += msg_len;
        update_len -= msg_len;
    }

    ret |= mt_unf_cipher_hash_final(handle, digest);
    mss_dump("digest", digest, digest_len);
    if (memcmp(digest, expect_digest, digest_len) != 0) {
        printf("========== FAIL ==========\n");
        mss_dump("expect", expect_digest, digest_len);
        return -1;
    }
    printf("========== SUCCESS ==========\n");
    return 0;
}

#ifdef CONFIG_MT_TEE_SUPPORT
static int mss_hash_tee_test(struct mss_hash_data *testcase, mt_u32 update_cnt)
{
    mt_s32 ret = 0;
    mt_u8 digest[64];
    mt_u32 handle;
    mt_u8 *msg;
    mt_u32 msg_len;
    mt_u32 update_len;
    mt_u8 *expect_digest;
    mt_u32 digest_len;
    MT_CIPHER_HASH_TYPE_E hash_type;
    mt_u8 rand;

    msg = testcase->msg;
    update_len = testcase->msg_length;
    expect_digest = testcase->digest;
    digest_len = testcase->digest_length;
    hash_type = testcase->type;

    printf("\n=====================================\n");

    switch (hash_type) {
    case MT_CIPHER_HASH_TYPE_SHA256:
        printf("SHA256");
        break;
    case MT_CIPHER_HASH_TYPE_SHA1:
        printf("SHA1");
        break;
    case MT_CIPHER_HASH_TYPE_SHA224:
        printf("SHA224");
        break;
    case MT_CIPHER_HASH_TYPE_SHA384:
        printf("SHA384");
        break;
    case MT_CIPHER_HASH_TYPE_SHA512:
        printf("SHA512");
        break;
    case MT_CIPHER_HASH_TYPE_SM3:
        printf("SM3");
        break;
    default:
        printf("UNKNOWN hash type\n");
        return -1;
    }
    printf("  update_cnt: %d\n", update_cnt);

    ret = tee_hash_create(hash_type, &handle);

    while (update_cnt > 0) {
        update_cnt--;

        if (update_cnt == 0) {
            msg_len = update_len;
        } else {
            mt_unf_cipher_get_random_number(1, &rand);
            msg_len = rand % (update_len - update_cnt);
        }
        mss_dump("msg", msg, msg_len);
        ret |= tee_hash_update(handle, msg, msg_len);

        msg += msg_len;
        update_len -= msg_len;
    }

    ret |= tee_hash_final(handle, digest, digest_len);
    mss_dump("digest", digest, digest_len);
    if (memcmp(digest, expect_digest, digest_len) != 0) {
        printf("========== FAIL ==========\n");
        mss_dump("expect", expect_digest, digest_len);
        return -1;
    }
    printf("========== SUCCESS ==========\n");
    return 0;
}
#else
static int mss_hash_tee_test(struct mss_hash_data *testcase, mt_u32 update_cnt)
{
    printf("not support tee\n");
    return MT_FAILURE;
}
#endif

/*
hash [-t] auto level
hash [-t] sha256 level
*/
static int mss_hash_test_auto(int argc, char *const argv[])
{
    mt_s32 ret = 0;
    MSS_CPU_MODE_E cpu_mode = MSS_APCPU;
    mt_u32 level;
    mt_u32 idx;
    mt_u32 type = 0xff;
    char *cmd;
    mt_u32 i;
    struct mss_hash_data *testcase;

    idx = 0;
    if (strcmp(argv[idx], "-t") == 0) {
        cpu_mode = MSS_SECCPU;
        idx++;
    }

    cmd = argv[idx++];
    if (0 == strncmp("auto", cmd, 4)){
        type = 0xff;
    } else if (0 == strncmp("sha256", cmd, 6)) {
        type = MT_CIPHER_HASH_TYPE_SHA256;
    } else if (0 == strncmp("sha1", cmd, 4)) {
        type = MT_CIPHER_HASH_TYPE_SHA1;
    } else if (0 == strncmp("sha224", cmd, 6)) {
        type = MT_CIPHER_HASH_TYPE_SHA224;
    } else if (0 == strncmp("sha384", cmd, 6)) {
        type = MT_CIPHER_HASH_TYPE_SHA384;
    } else if (0 == strncmp("sha512", cmd, 6)) {
        type = MT_CIPHER_HASH_TYPE_SHA512;
    } else if (0 == strncmp("sm3", cmd, 3)) {
        type = MT_CIPHER_HASH_TYPE_SM3;
    }

    level = (mt_u32)strtol(argv[idx++], NULL, 0);

    for (i = 0; i < ARRAY_SIZE(mss_hash_testcase); i++) {
        testcase = &mss_hash_testcase[i];

        if (type == 0xff || type == testcase->type) {
            if (cpu_mode == MSS_SECCPU)
                ret = mss_hash_tee_test(testcase, level);
            else
                ret = mss_hash_ree_test(testcase, level);

            if (ret != 0)
                return ret;
        }
    }

    return 0;
}

static void mss_hash_cmd_help(void)
{
    mss_print("Usage:\n");

    mss_print("\nhash [-t] <type> <count>\n");
    mss_print("\t [-t]: tee hash test\n");
    mss_print("\t <type>: auto|sha256|sha1|sha224|sha384|sha512|sm3\n");
    mss_print("\t <count>: msg update count\n");
}

mt_s32 mss_hash_test(int argc, char *const argv[]);

mt_s32 mss_hash_test(int argc, char *const argv[])
{
    int ret;
    mt_u32 i;

    if (0 == strncmp("-h", argv[0], 2)){
        mss_hash_cmd_help();
        return 0;
    }

    for (i = 0; i < argc; i++)
        printf("%s [%d]:%s\n", __FUNCTION__, i, argv[i]);

    ret = mss_hash_test_auto(argc, argv);
    if (ret != 0) {
        printf("\nmss_hash_test FAIL\n");
        return ret;
    }

    printf("\nmss_hash_test SUCCESS\n");

    return 0;
}

