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

mt_s32 mss_kt_test(int argc, char *const argv[]);

/* request continuous keytable slots */
static int mss_kt_request(mt_u32 cnt, mt_u32 slot_num)
{
    mt_u32 r[128];
    mt_u8 bitmap[128];
    mt_u32 i, j;
    int ret;

    if (cnt * slot_num > 128) {
        mss_print("request too much\n");
        return -1;
    }

    if (cnt == 0)
        cnt = 128;

    for (i = 0; i < 128; i++) {
        r[i] = MT_CIPHER_KEYSLOT_INVALID;
        bitmap[i] = 0;
    }

    for (i = 0; i < cnt; i++) {
        ret = mt_unf_cipher_keyslot_request_multi(slot_num, &r[i*slot_num]);
        if (ret == MT_SUCCESS) {
            for (j = 0; j < slot_num; j++) {
                mss_print(" r[%d]: ", i*slot_num+j);
                mt_unf_cipher_keyslot_info(r[i*slot_num+j]);
                bitmap[r[i*slot_num+j]] = 1;
            }
            mss_print("\n");
        }
        else {
            //mss_print("%s(%d) request %d fail\n",__FUNCTION__,__LINE__, i*num);
            break;            
        }
    }

    mt_unf_cipher_keyslot_control_status();

    /* list free slot */
    if (cnt == 128) {
        mss_print("\ncan not be requested slots:\n");
        for (i = 1; i < 128; i++) {
            if (bitmap[i] == 0) {
                mt_unf_cipher_keyslot_info(i);
            }
        }
    }

    /* release slot */
    mss_print("\nrelease slots\n");
    for (i = 0; i < 128; i++) {
        if (r[i] != MT_CIPHER_KEYSLOT_INVALID) {
            mt_unf_cipher_keyslot_release(r[i]);
            r[i] = MT_CIPHER_KEYSLOT_INVALID;
        }
    }

    mt_unf_cipher_keyslot_control_status();

    return MT_SUCCESS;
}

#ifdef CONFIG_MT_TEE_SUPPORT
static int mss_kt_request_tee(mt_u32 cnt, mt_u32 slot_num)
{
    mt_u32 r[128];
    mt_u8 bitmap[128];
    mt_u32 i, j;
    int ret;

    if (cnt * slot_num > 128) {
        mss_print("request too much\n");
        return -1;
    }

    if (cnt == 0)
        cnt = 128;

    for (i = 0; i < 128; i++) {
        r[i] = MT_CIPHER_KEYSLOT_INVALID;
        bitmap[i] = 0;
    }

    for (i = 0; i < cnt; i++) {
        ret = tee_keyslot_request(slot_num, &r[i*slot_num]);
        if (ret == MT_SUCCESS) {
            for (j = 0; j < slot_num; j++) {
                mss_print(" r[%d]: ", i*slot_num+j);
                mt_unf_cipher_keyslot_info(r[i*slot_num+j]);
                bitmap[r[i*slot_num+j]] = 1;
            }
            mss_print("\n");
        }
        else {
            //mss_print("%s(%d) request %d fail\n",__FUNCTION__,__LINE__, i*num);
            break;            
        }
    }

    mt_unf_cipher_keyslot_control_status();

    /* list free slot */
    if (cnt == 128) {
        mss_print("\ncan not be requested slots:\n");
        for (i = 1; i < 128; i++) {
            if (bitmap[i] == 0) {
                mt_unf_cipher_keyslot_info(i);
            }
        }
    }

    /* release slot */
    mss_print("\nrelease slots\n");
    for (i = 0; i < 128; i++) {
        if (r[i] != MT_CIPHER_KEYSLOT_INVALID) {
            tee_keyslot_release(r[i]);
            r[i] = MT_CIPHER_KEYSLOT_INVALID;
        }
    }

    mt_unf_cipher_keyslot_control_status();

    return MT_SUCCESS;
}
#else
static int mss_kt_request_tee(mt_u32 cnt, mt_u32 slot_num)
{
    printf("not support tee\n");
    return MT_FAILURE;
}
#endif

static mt_s32 mss_kt_request_test(int argc, char *const argv[])
{
    mt_u32 src = 0;
    mt_u32 cnt;
    mt_u32 slot_num;
    mt_s32 ret;
    mt_u32 idx;

    idx = 0;
    if (strcmp(argv[idx], "-t") == 0) {
        src = 1;
        idx++;
    }

    cnt = (mt_u32)strtol(argv[idx++], NULL, 0);
    slot_num = (mt_u32)strtol(argv[idx], NULL, 0);

    if (src == 1)
        ret = mss_kt_request_tee(cnt, slot_num);
    else
        ret = mss_kt_request(cnt, slot_num);

    return ret;
}

static int mss_kt_iv_test(void)
{
    mt_u32 slot;
    mt_u8 iv[16] = {0xe0,0x3a,0x0c,0x22,0xb9,0x9a,0xa9,0x05,0xea,0xd4,0xaf,0x02,0xf9,0x7a,0xfe,0x24};
    mt_u8 iv_out[16];

    mss_print("\n%s START ========\n", __FUNCTION__);

#ifdef CONFIG_MT_TEE_SUPPORT
    mss_print("tee request ktslot:\n");
    tee_keyslot_request(1, &slot);
#else
    mss_print("ree request ktslot:\n");
    mt_unf_cipher_keyslot_request(&slot);
#endif

    mt_unf_cipher_keyslot_info(slot);
    mt_unf_cipher_keyslot_get_iv(slot, iv_out, 16);
    mss_dump("iv0", iv_out, 16);

    mss_print("\nree update iv test:\n");
    mt_unf_cipher_keyslot_set_iv(slot, iv, 16);
    mt_unf_cipher_keyslot_get_iv(slot, iv_out, 16);
    mss_dump("iv1", iv_out, 16);
    if (memcmp(iv_out, iv, 16) == 0) {
        mss_print("ree can update iv\n");
    } else {
        mss_print("ree can not update iv\n");
    }

#ifdef CONFIG_MT_TEE_SUPPORT
    mss_print("\ntee release tee ktslot:\n");
    tee_keyslot_release(slot);
#else
    mss_print("\nree release ktslot:\n");
    mt_unf_cipher_keyslot_release(slot);
#endif

    mt_unf_cipher_keyslot_info(slot);

    mss_print("%s END ========\n", __FUNCTION__);

    return 0;
}

static int mss_kt_set_test(void)
{
    mt_u32 slot;
    mt_u8 key[16] = {0xb7,0x08,0xfd,0xfc,0xfe,0xe9,0x10,0x72,0xe5,0xb2,0xd6,0xb6,0xb0,0x36,0x50,0xb3};
    mt_u8 iv[16] = {0xe0,0x3a,0x0c,0x22,0xb9,0x9a,0xa9,0x05,0xea,0xd4,0xaf,0x02,0xf9,0x7a,0xfe,0x24};
    MT_CIPHER_CTRL_S ctrl;

    mss_print("\n%s START ========\n", __FUNCTION__);

    mss_print("ree request ktslot:\n");
    mt_unf_cipher_keyslot_request(&slot);
    mt_unf_cipher_keyslot_info(slot);

    mss_print("\nree set key:\n");
    ctrl.core = MT_CIPHER_CORE_M2M_RAW;
    ctrl.operation = MT_CIPHER_OPERATION_ENCRYPT;
    ctrl.algorithm = MT_CIPHER_ALG_TDES;
    mt_unf_cipher_keyslot_set(slot, &ctrl, key, iv);
    mt_unf_cipher_keyslot_info(slot);

#ifdef CONFIG_MT_TEE_SUPPORT
    mss_print("\ntee set key:\n");
    ctrl.core = MT_CIPHER_CORE_M2M_RAW;
    ctrl.operation = MT_CIPHER_OPERATION_DECRYPT;
    ctrl.algorithm = MT_CIPHER_ALG_AES;
    tee_keyslot_set(slot, &ctrl, key, 16, NULL, 0);
    mt_unf_cipher_keyslot_info(slot);
#endif

    mss_print("\nree release ktslot:\n");
    mt_unf_cipher_keyslot_release(slot);
    mt_unf_cipher_keyslot_info(slot);

    mss_print("%s END ========\n", __FUNCTION__);

    return 0;
}

static mt_s32 mss_kt_auto_test(int argc, char *const argv[])
{
    mss_kt_iv_test();

    mss_kt_set_test();

    return 0;
}

static void mss_kt_cmd_help(void)
{
    mss_print("Usage:\n");

    mss_print("\nkt info <slotid>\n");
    mss_print("\t print slot info\n");

    mss_print("\nkt request [-t] <group_cnt> <group_size>\n");
    mss_print("\t kt request test.\n");
    mss_print("\t [-t]: request tee kt slot\n");
    mss_print("\t <group_cnt>: count of groups for keyslot\n");
    mss_print("\t <group_size>: keyslot group size\n");
    mss_print("\t example: kt request 1 2 -- request one group, each with 2 keyslots\n");

    mss_print("\nkt auto\n");
    mss_print("\t kt key & iv set test\n");

}

mt_s32 mss_kt_test(int argc, char *const argv[])
{
    mt_s32 ret = -1;
    char *cmd;
    mt_u32 i;
    mt_u32 keyslot;

    if (0 == strncmp("-h", argv[0], 2)){
        mss_kt_cmd_help();
        return 0;
    }

    for (i = 0; i < argc; i++)
        printf("%s [%d]:%s\n", __FUNCTION__, i, argv[i]);

    cmd = argv[0];

    if (0 == strncmp("info", cmd, 4)){
        if (argc < 2)
            return -1;

        keyslot = (mt_u32)strtol(argv[1], NULL, 0);
        ret = mt_unf_cipher_keyslot_info(keyslot);
    }
    else if (0 == strncmp("request", cmd, 7)){
        if (argc < 3)
            return -1;

        ret = mss_kt_request_test(argc - 1, argv + 1);
    }
    else if (0 == strncmp("auto", cmd, 4)){
        ret = mss_kt_auto_test(argc - 1, argv + 1);
    }

    if (ret)
        mss_kt_cmd_help();

    return ret;
}

