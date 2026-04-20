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

#define ARRAY_SIZE(x)   (sizeof(x)/sizeof(x[0]))

#define MSS_NOT_INITIALIZED 0xFFFFFFFF

struct mss_crypto_raw_data
{
    mt_u32 algo;
    mt_u32 mode;
    mt_u8 key[16];
    mt_u8 iv[16];
    mt_u32 data_len;
    mt_u32 crypto_blocks;
    mt_u32 skip_blocks;
    mt_u8 clear[256];
    mt_u8 cipher[256];
}__attribute__((aligned(4)));
#include "m2m_raw_testcase.h"

struct mss_crypto_ts_data
{
    mt_u32 algo;
    mt_u32 mode;
    mt_u8 key[16];
    mt_u8 iv[16];
    mt_u8 clearts[188];
    mt_u8 scramblets[188];
}__attribute__((aligned(4)));
#include "m2m_ts_testcase.h"

mt_s32 mss_crypto_test_cmd(int argc, char *const argv[]);

static phys_addr_t vir2phy(mt_u8 *vir)
{
    mt_s32 ret = MT_SUCCESS;
    phys_addr_t phy_addr;
    ulong phy_size;

    ret = mt_mmz_get_phyaddr((mt_void *)vir, &phy_addr, &phy_size);
    if (ret == MT_SUCCESS)
        return (phys_addr_t)phy_addr;
    else
        return (phys_addr_t)0;
}

static void crypto_algo_mode_print(mt_u32 algo, mt_u32 mode)
{
    if (algo == MT_CIPHER_ALG_AES)
        printf("AES");
    else if (algo == MT_CIPHER_ALG_TDES)
        printf("TDES");
    else if (algo == MT_CIPHER_ALG_DES)
        printf("DES");
    else if (algo == MT_CIPHER_ALG_CSA2)
        printf("CSA2");
    else if (algo == MT_CIPHER_ALG_CSA3)
        printf("CSA3");
    else if (algo == MT_CIPHER_ALG_AES256)
        printf("AES256");
    else if (algo == MT_CIPHER_ALG_SM4)
        printf("SM4");

    if (mode == MT_CIPHER_WORK_MODE_ECB)
        printf(" ECB");
    else if (mode == MT_CIPHER_WORK_MODE_CBC)
        printf(" CBC");
    else if (mode == MT_CIPHER_WORK_MODE_CTR)
        printf(" CTR");
    else if (mode == MT_CIPHER_WORK_MODE_CBCDVS042)
        printf(" CBCDVS042");
    else if (mode == MT_CIPHER_WORK_MODE_CBCCTS)
        printf(" CBCCTS");
    else if (mode == MT_CIPHER_WORK_MODE_RCBC)
        printf(" RCBC");
    else if (mode == MT_CIPHER_WORK_MODE_ECBCTS)
        printf(" ECBCTS");

}

static mt_u32 get_crypto_algorithm(char *algo)
{
    if (strncmp(algo, "aes", 3) == 0) {
        return MT_CIPHER_ALG_AES;
    } else if (strncmp(algo, "tdes", 4) == 0) {
        return MT_CIPHER_ALG_TDES;
    } else if (strncmp(algo, "des", 3) == 0) {
        return MT_CIPHER_ALG_DES;
    } else if (strncmp(algo, "sm4", 3) == 0) {
        return MT_CIPHER_ALG_SM4;
    } else {
        printf("unknown algo:%s\n", algo);
        return MSS_NOT_INITIALIZED;
    } 
}

static mt_u32 get_crypto_operation(char *op)
{
    if (strncmp(op, "dec", 3) == 0) {
        return MT_CIPHER_OPERATION_DECRYPT;
    } else if (strncmp(op, "enc", 3) == 0) {
        return MT_CIPHER_OPERATION_ENCRYPT;
    } else {
        printf("unknown op:%s\n", op);
        return MSS_NOT_INITIALIZED;
    }
}

static mt_u32 get_crypto_workmode(char *mode)
{
    if (strcmp(mode, "ecb") == 0) {
        return MT_CIPHER_WORK_MODE_ECB;
    } else if (strcmp(mode, "cbc") == 0) {
        return MT_CIPHER_WORK_MODE_CBC;
    } else if (strcmp(mode, "ctr") == 0) {
        return MT_CIPHER_WORK_MODE_CTR;
    } else if (strcmp(mode, "cbcdvs042") == 0) {
        return MT_CIPHER_WORK_MODE_CBCDVS042;
    } else if (strcmp(mode, "cbccts") == 0) {
        return MT_CIPHER_WORK_MODE_CBCCTS;
    } else if (strcmp(mode, "rcbc") == 0) {
        return MT_CIPHER_WORK_MODE_RCBC;
    } else if (strcmp(mode, "ecbcts") == 0) {
        return MT_CIPHER_WORK_MODE_ECBCTS;
    } else if (strcmp(mode, "cfb") == 0) {
        return MT_CIPHER_WORK_MODE_CFB;
    } else if (strcmp(mode, "ofb") == 0) {
        return MT_CIPHER_WORK_MODE_OFB;
    } else if (strcmp(mode, "cbcs") == 0) {
        return MT_CIPHER_WORK_MODE_CBCS;
    } else if (strcmp(mode, "cens") == 0) {
        return MT_CIPHER_WORK_MODE_CENS;
    } else {
        printf("unknown mode:%s\n", mode);
        return MSS_NOT_INITIALIZED;
    }
}

static mt_s32 _sym6_crypto_raw_algo_mode_check(mt_u32 algo, mt_u32 mode)
{
    mt_s32 support = 0;

    if (algo == MT_CIPHER_ALG_AES) {
        switch (mode) {
            case MT_CIPHER_WORK_MODE_ECB:
            case MT_CIPHER_WORK_MODE_CBC:
            case MT_CIPHER_WORK_MODE_CTR:
            case MT_CIPHER_WORK_MODE_CBCDVS042:
            case MT_CIPHER_WORK_MODE_CBCCTS:
            case MT_CIPHER_WORK_MODE_CBCS:
            case MT_CIPHER_WORK_MODE_CENS:
                support = 1;
                break;
            default:
                break;
        }
    } else if (algo == MT_CIPHER_ALG_TDES) {
        switch (mode) {
            case MT_CIPHER_WORK_MODE_ECB:
            case MT_CIPHER_WORK_MODE_CBC:
            case MT_CIPHER_WORK_MODE_CBCDVS042:
            case MT_CIPHER_WORK_MODE_CBCCTS:
            case MT_CIPHER_WORK_MODE_CBCS:
            case MT_CIPHER_WORK_MODE_CENS:
                support = 1;
                break;
            default:
                break;
        }
    } else if (algo == MT_CIPHER_ALG_DES) {
        switch (mode) {
            case MT_CIPHER_WORK_MODE_ECB:
            case MT_CIPHER_WORK_MODE_CBC:
            case MT_CIPHER_WORK_MODE_CBCDVS042:
            case MT_CIPHER_WORK_MODE_CBCCTS:
            case MT_CIPHER_WORK_MODE_CBCS:
            case MT_CIPHER_WORK_MODE_CENS:
                support = 1;
                break;
            default:
                break;
        }
    } else if (algo == MT_CIPHER_ALG_SM4) {
        switch (mode) {
            case MT_CIPHER_WORK_MODE_ECB:
            case MT_CIPHER_WORK_MODE_CBC:
            case MT_CIPHER_WORK_MODE_CTR:
            case MT_CIPHER_WORK_MODE_CBCDVS042:
            case MT_CIPHER_WORK_MODE_CBCCTS:
            case MT_CIPHER_WORK_MODE_CBCS:
            case MT_CIPHER_WORK_MODE_CENS:
                support = 1;
                break;
            default:
                break;
        }
    }

    return support;
}

static mt_s32 _sym6_crypto_ts_algo_mode_check(mt_u32 algo, mt_u32 mode)
{
    mt_s32 support = 0;

    if (algo == MT_CIPHER_ALG_AES) {
        switch (mode) {
            case MT_CIPHER_WORK_MODE_ECB:
            case MT_CIPHER_WORK_MODE_CBC:
            case MT_CIPHER_WORK_MODE_CBCDVS042:
            case MT_CIPHER_WORK_MODE_CBCCTS:
            //case MT_CIPHER_WORK_MODE_RCBC:
            case MT_CIPHER_WORK_MODE_ECBCTS:
                support = 1;
                break;
            default:
                break;
        }
    }
    else if (algo == MT_CIPHER_ALG_TDES) {
        switch (mode) {
            case MT_CIPHER_WORK_MODE_ECB:
            case MT_CIPHER_WORK_MODE_CBC:
            case MT_CIPHER_WORK_MODE_CBCDVS042:
            case MT_CIPHER_WORK_MODE_CBCCTS:
            //case MT_CIPHER_WORK_MODE_RCBC:
            case MT_CIPHER_WORK_MODE_ECBCTS:
                support = 1;
                break;
            default:
                break;
        }
    }
    else if (algo == MT_CIPHER_ALG_DES) {
        switch (mode) {
            case MT_CIPHER_WORK_MODE_ECB:
            case MT_CIPHER_WORK_MODE_CBC:
            case MT_CIPHER_WORK_MODE_CBCDVS042:
            case MT_CIPHER_WORK_MODE_CBCCTS:
            //case MT_CIPHER_WORK_MODE_RCBC:
            case MT_CIPHER_WORK_MODE_ECBCTS:
                support = 1;
                break;
            default:
                break;
        }
    }
    return support;
}

static struct mss_crypto_raw_data *mss_m2m_raw_testcase_get(mt_u32 algo, mt_u32 mode)
{
    struct mss_crypto_raw_data *testcase;
    mt_u32 i;

    for (i = 0; i < ARRAY_SIZE(m2m_raw_testcase); i++) {
        testcase = &m2m_raw_testcase[i];
        if (testcase->algo == algo && testcase->mode == mode)
            return testcase;        
        }

    return NULL;
}

static struct mss_crypto_ts_data *mss_m2m_ts_testcase_get(mt_u32 algo, mt_u32 mode)
{
    struct mss_crypto_ts_data *testcase;
    mt_u32 i;

        for (i = 0; i < ARRAY_SIZE(m2m_ts_testcase); i++) {
        testcase = &m2m_ts_testcase[i];
        if (testcase->algo == algo && testcase->mode == mode)
            return testcase;        
        }

    return NULL;
}

static mt_s32 mss_m2m_raw_test(struct mss_crypto_raw_data *data,
    MT_CIPHER_OPERATION_E op, mt_u32 printsimple)
{
    mt_s32 ret = MT_FAILURE;
    mt_u32 keyslot;
    MT_CIPHER_CTRL_S cipher_ctrl = {0,};
    mt_handle p_cipher = MT_NULL;
    unsigned char *p_src_addr = MT_NULL;
    unsigned char *p_dst_addr = MT_NULL;
    unsigned char *expect;

    printf("\n============== ");
    crypto_algo_mode_print(data->algo, data->mode);
    printf(" RAW");
    if (op == MT_CIPHER_OPERATION_ENCRYPT)
        printf(" ENC");
    else
        printf(" DEC");
    printf(" ==============\n");

    /* request keyslot */
    ret = mt_unf_cipher_keyslot_request(&keyslot);
    MT_ASSERT(ret == MT_SUCCESS);

    p_src_addr = mt_unf_cipher_malloc(data->data_len);
    if (p_src_addr == MT_NULL)
        goto EXIT;

    p_dst_addr = mt_unf_cipher_malloc(data->data_len);
    if (p_dst_addr == MT_NULL)
        goto EXIT;

    cipher_ctrl.core = MT_CIPHER_CORE_M2M_RAW;
    cipher_ctrl.operation = op;
    cipher_ctrl.algorithm = data->algo;
    cipher_ctrl.work_mode = data->mode;
    if ((data->mode == MT_CIPHER_WORK_MODE_CBCS) || (data->mode == MT_CIPHER_WORK_MODE_CENS)) {
        cipher_ctrl.cbcs_params.crypt_blocks = data->crypto_blocks;
        cipher_ctrl.cbcs_params.skip_blocks = data->skip_blocks;
    }

    /* crypto enc/dec */
    {
        if (op == MT_CIPHER_OPERATION_ENCRYPT) {
            memcpy(p_src_addr, data->clear, data->data_len);
            expect = data->cipher;
        }
        else {
            memcpy(p_src_addr, data->cipher, data->data_len);
            expect = data->clear;
        }

        /* set keyslot */
        ret = mt_unf_cipher_keyslot_set(keyslot, &cipher_ctrl, data->key, data->iv);
        if (ret != MT_SUCCESS)
            goto EXIT;
        mt_unf_cipher_keyslot_info(keyslot);

        /* REE uses channel 0 always. */
        ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &p_cipher);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = mt_unf_cipher_crypto_config(p_cipher, &cipher_ctrl, keyslot);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = mt_unf_cipher_crypto_process(p_cipher, p_src_addr, p_dst_addr, data->data_len);
        if (ret != MT_SUCCESS)
            goto EXIT;

        mt_unf_cipher_crypto_destroy(p_cipher);
        p_cipher = MT_NULL;

        mss_dump("key", data->key, sizeof(data->key));
        mss_dump("iv", data->iv, sizeof(data->iv));
        mss_dump("src", p_src_addr, data->data_len);
        mss_dump("dst", p_dst_addr, data->data_len);

        if (ret == MT_SUCCESS) {
            if (memcmp(p_dst_addr, expect, data->data_len) == 0) {
                printf("========== CRYPTO SUCCESS ==========\n");
                ret = MT_SUCCESS;
            } else {
                printf("========== CRYPTO FAIL ==========\n");
                mss_dump("expect", expect, data->data_len);

                ret = MT_FAILURE;
                goto EXIT;
            }
        }
    }

    /* crypto dec/enc phy*/
    {
        cipher_ctrl.operation = op == MT_CIPHER_OPERATION_DECRYPT ? MT_CIPHER_OPERATION_ENCRYPT : MT_CIPHER_OPERATION_DECRYPT;
        if (op == MT_CIPHER_OPERATION_ENCRYPT) {
            expect = data->clear;
        } else {
            expect = data->cipher;
        }

        /* set keyslot */
        ret = mt_unf_cipher_keyslot_set(keyslot, &cipher_ctrl, data->key, data->iv);
        if (ret != MT_SUCCESS)
            goto EXIT;
        mt_unf_cipher_keyslot_info(keyslot);

        /* REE uses channel 0 always. */
        ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &p_cipher);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = mt_unf_cipher_crypto_config(p_cipher, &cipher_ctrl, keyslot);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = mt_unf_cipher_crypto_process_phy(p_cipher, vir2phy(p_dst_addr), vir2phy(p_src_addr), data->data_len);
        if (ret != MT_SUCCESS)
            goto EXIT;

        mt_unf_cipher_crypto_destroy(p_cipher);
        p_cipher = MT_NULL;

        mss_dump("key", data->key, 16);
        mss_dump("src", p_dst_addr, data->data_len);
        mss_dump("dst cipher", p_src_addr, data->data_len);

        if (ret == MT_SUCCESS) {
            if (memcmp(p_src_addr, expect, data->data_len) == 0) {
                printf("========== CRYPTO SUCCESS ==========\n");
                ret = MT_SUCCESS;
            } else {
                printf("========== CRYPTO FAIL ==========\n");
                mss_dump("expect", expect, data->data_len);

                ret = MT_FAILURE;
                goto EXIT;
            }
        }
    }

EXIT:
    if (p_cipher != MT_NULL)
        mt_unf_cipher_crypto_destroy(p_cipher);

    if (p_src_addr)
        mt_unf_cipher_free(p_src_addr);

    if (p_dst_addr)
        mt_unf_cipher_free(p_dst_addr);

    /* release keyslot */
    mt_unf_cipher_keyslot_release(keyslot);

    return ret;
}

static mt_s32 mss_m2m_ts_test(struct mss_crypto_ts_data *data,
    MT_CIPHER_OPERATION_E op, mt_u32 printsimple)
{
    mt_s32 ret = MT_FAILURE;
    mt_u32 keyslot;
    MT_CIPHER_CTRL_S cipher_ctrl = {0,};
    MT_CIPHER_TS_PARA_S ts_para = {0,};
    mt_handle p_cipher = MT_NULL;
    mt_u8 *in_vir_addr;
    mt_u8 *out_vir_addr;
    mt_u8 *expect;

    printf("\n============== ");
    crypto_algo_mode_print(data->algo, data->mode);
    printf(" TS");
    if (op == MT_CIPHER_OPERATION_ENCRYPT)
        printf(" SCRAMBLE");
    else
        printf(" DESCRAMBLE");
    printf(" ==============\n");

    /* request keyslot */
    ret = mt_unf_cipher_keyslot_request(&keyslot);
    MT_ASSERT(ret == MT_SUCCESS);

    in_vir_addr = mt_unf_cipher_malloc(188);
    out_vir_addr = mt_unf_cipher_malloc(188);

    if (op == MT_CIPHER_OPERATION_ENCRYPT) {
        memcpy(in_vir_addr, data->clearts, 188);
        expect = data->scramblets;
    }
    else {
        memcpy(in_vir_addr, data->scramblets, 188);
        expect = data->clearts;
    }

    memset(out_vir_addr, 0, 188);

    /* config ts parameters */
    if (op == MT_CIPHER_OPERATION_ENCRYPT) {
        /* scramble */
        ts_para.ts_enc_ksel = MT_TS_ENC_EVEN_KEY;   // scramble with even key
        //ts_para.ts_force_enc_en = MT_TS_FORCE_ENC_ENABLE;  // Only can be written by FWHost.
    }
    else {
        /* descramble with even key */
        ts_para.ts_dec_ind = MT_TS_DEC_IND_CLEAR;    // clear scramble control
    }
    ts_para.ts_pid0_filt_en = 1;                //enable
    ts_para.ts_pid0_filt_num = 0x01a0;            //pid
    ts_para.ts_short_mode = MT_TS_SHORT_TAIL;

    cipher_ctrl.core = MT_CIPHER_CORE_M2M_TS;
    cipher_ctrl.operation = op;
    cipher_ctrl.algorithm = data->algo;
    cipher_ctrl.work_mode = data->mode;

    {
        /* set keyslot */
        ret = mt_unf_cipher_keyslot_set(keyslot, &cipher_ctrl, data->key, data->iv);
        MT_ASSERT(ret == MT_SUCCESS);
        mt_unf_cipher_keyslot_info(keyslot);

        ret = mt_unf_cipher_ts_create(MT_CIPHER_CRYPTO_CH_0, &p_cipher);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = mt_unf_cipher_ts_config(p_cipher, &cipher_ctrl, &ts_para, keyslot, keyslot);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = mt_unf_cipher_ts_process(p_cipher, vir2phy(in_vir_addr), vir2phy(out_vir_addr), 188);
        if (ret != MT_SUCCESS)
            goto EXIT;

        mt_unf_cipher_ts_destroy(p_cipher);
        p_cipher = MT_NULL;

        mss_dump("key", data->key, 16);
        mss_dump("ints", in_vir_addr, 188);
        mss_dump("outts", out_vir_addr, 188);

        if (ret == MT_SUCCESS) {
            if (memcmp(out_vir_addr, expect, 188) == 0) {
                printf("========== CRYPTOTS SUCCESS ==========\n");
                ret = MT_SUCCESS;
            } else {
                printf("========== CRYPTOTS FAIL ==========\n");
                mss_dump("expect", expect, 188);

                ret = MT_FAILURE;
                goto EXIT;
            }
        }
    }

EXIT:
    if (p_cipher != MT_NULL)
        mt_unf_cipher_ts_destroy(p_cipher);

    /* release keyslot */
    mt_unf_cipher_keyslot_release(keyslot);

    mt_unf_cipher_free(in_vir_addr);
    mt_unf_cipher_free(out_vir_addr);

    return ret;
}

#ifdef CONFIG_MT_TEE_SUPPORT
static mt_s32 mss_m2m_raw_tee_test(struct mss_crypto_raw_data *data,
    MT_CIPHER_OPERATION_E op, mt_u32 printsimple)
{
    mt_s32 ret = MT_FAILURE;
    mt_u32 keyslot;
    MT_CIPHER_CTRL_S cipher_ctrl = {0,};
    mt_u32 p_cipher = MT_NULL;
    mt_u8 output[32] = {0,};
    unsigned char *p_src_addr = MT_NULL;
    unsigned char *expect;

    printf("\n============== ");
    crypto_algo_mode_print(data->algo, data->mode);
    printf(" RAW TEE");
    if (op == MT_CIPHER_OPERATION_ENCRYPT)
        printf(" ENC");
    else
        printf(" DEC");
    printf(" ==============\n");

    /* request keyslot */
    ret = tee_keyslot_request(1, &keyslot);
    MT_ASSERT(ret == MT_SUCCESS);

    if (op == MT_CIPHER_OPERATION_ENCRYPT) {
        p_src_addr = data->clear;
        expect = data->cipher;
    }
    else {
        p_src_addr = data->cipher;
        expect = data->clear;
    }

    cipher_ctrl.core = MT_CIPHER_CORE_M2M_RAW;
    cipher_ctrl.operation = op;
    cipher_ctrl.algorithm = data->algo;
    cipher_ctrl.work_mode = data->mode;

    {
        /* set keyslot */
        ret = tee_keyslot_set(keyslot, &cipher_ctrl, data->key, 16, data->iv, 16);
        if (ret != MT_SUCCESS)
            goto EXIT;
        mt_unf_cipher_keyslot_info(keyslot);

        /* TEE uses channel 1 always. */
        ret = tee_crypto_create(MT_CIPHER_CRYPTO_CH_1, &p_cipher);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = tee_crypto_config(p_cipher, &cipher_ctrl, keyslot);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = tee_crypto_process(p_cipher, p_src_addr, output, data->data_len);
        if (ret != MT_SUCCESS)
            goto EXIT;

        tee_crypto_destroy(p_cipher);
        p_cipher = MT_NULL;

        mss_dump("key", data->key, 16);
        mss_dump("src", p_src_addr, data->data_len);
        mss_dump("output", output, data->data_len);

        if (ret == MT_SUCCESS) {
            if (memcmp(output, expect, data->data_len) == 0) {
                printf("========== CRYPTO TEE SUCCESS ==========\n");
                ret = MT_SUCCESS;
            } else {
                printf("========== CRYPTO TEE FAIL ==========\n");
                mss_dump("expect", expect, data->data_len);

                ret = MT_FAILURE;
                goto EXIT;
            }
        }
    }

EXIT:
    if (p_cipher != MT_NULL)
        tee_crypto_destroy(p_cipher);

    /* release keyslot */
    tee_keyslot_release(keyslot);

    return ret;
}

static mt_s32 mss_m2m_ts_tee_test(struct mss_crypto_ts_data *data,
    MT_CIPHER_OPERATION_E op, mt_u32 printsimple)
{
    mt_s32 ret = MT_FAILURE;
    mt_u32 keyslot;
    MT_CIPHER_CTRL_S cipher_ctrl = {0,};
    MT_CIPHER_TS_PARA_S ts_para = {0,};
    mt_u32 p_cipher = MT_NULL;
    mt_u8 *in_vir_addr;
    mt_u8 *out_vir_addr;
    mt_u8 *expect;

    printf("\n============== ");
    crypto_algo_mode_print(data->algo, data->mode);
    printf(" TS");
    if (op == MT_CIPHER_OPERATION_ENCRYPT)
        printf(" SCRAMBLE");
    else
        printf(" DESCRAMBLE");
    printf(" ==============\n");

    /* request keyslot */
    ret = tee_keyslot_request(1, &keyslot);
    MT_ASSERT(ret == MT_SUCCESS);

    in_vir_addr = mt_unf_cipher_malloc(188);
    out_vir_addr = mt_unf_cipher_malloc(188);

    if (op == MT_CIPHER_OPERATION_ENCRYPT) {
        memcpy(in_vir_addr, data->clearts, 188);
        expect = data->scramblets;
    }
    else {
        memcpy(in_vir_addr, data->scramblets, 188);
        expect = data->clearts;
    }

    memset(out_vir_addr, 0, 188);

    /* config ts parameters */
    if (op == MT_CIPHER_OPERATION_ENCRYPT) {
        /* scramble */
        ts_para.ts_enc_ksel = MT_TS_ENC_EVEN_KEY;   // scramble with even key
        //ts_para.ts_force_enc_en = MT_TS_FORCE_ENC_ENABLE;  // Only can be written by FWHost.
    }
    else {
        /* descramble with even key */
        ts_para.ts_dec_ind = MT_TS_DEC_IND_CLEAR;    // clear scramble control
    }
    ts_para.ts_pid0_filt_en = 1;                //enable
    ts_para.ts_pid0_filt_num = 0x01a0;            //pid
    ts_para.ts_short_mode = MT_TS_SHORT_TAIL;

    cipher_ctrl.core = MT_CIPHER_CORE_M2M_TS;
    cipher_ctrl.operation = op;
    cipher_ctrl.algorithm = data->algo;
    cipher_ctrl.work_mode = data->mode;

    {
        /* set keyslot */
        ret = tee_keyslot_set(keyslot, &cipher_ctrl, data->key, 16, data->iv, 16);
        MT_ASSERT(ret == MT_SUCCESS);
        mt_unf_cipher_keyslot_info(keyslot);

        ret = tee_ts_create(MT_CIPHER_CRYPTO_CH_1, &p_cipher);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = tee_ts_config(p_cipher, &cipher_ctrl, &ts_para, keyslot, keyslot);
        if (ret != MT_SUCCESS)
            goto EXIT;

        ret = tee_ts_process(p_cipher, in_vir_addr, out_vir_addr, 188);
        if (ret != MT_SUCCESS)
            goto EXIT;

        tee_ts_destroy(p_cipher);
        p_cipher = MT_NULL;

        mss_dump("key", data->key, 16);
        mss_dump("ints", in_vir_addr, 188);
        mss_dump("outts", out_vir_addr, 188);

        if (ret == MT_SUCCESS) {
            if (memcmp(out_vir_addr, expect, 188) == 0) {
                printf("========== CRYPTOTS TEE SUCCESS ==========\n");
                ret = MT_SUCCESS;
            } else {
                printf("========== CRYPTOTS TEE FAIL ==========\n");
                mss_dump("expect", expect, 188);

                ret = MT_FAILURE;
                goto EXIT;
            }
        }
    }

EXIT:
    if (p_cipher != MT_NULL)
        tee_ts_destroy(p_cipher);

    /* release keyslot */
    tee_keyslot_release(keyslot);

    mt_unf_cipher_free(in_vir_addr);
    mt_unf_cipher_free(out_vir_addr);

    return ret;
}
#else
static mt_s32 mss_m2m_raw_tee_test(struct mss_crypto_raw_data *data,
    MT_CIPHER_OPERATION_E op, mt_u32 printsimple)
{
    printf("not support tee\n");
    return MT_FAILURE;
}

static mt_s32 mss_m2m_ts_tee_test(struct mss_crypto_ts_data *data,
    MT_CIPHER_OPERATION_E op, mt_u32 printsimple)
{
    printf("not support tee\n");
    return MT_FAILURE;
}
#endif

static mt_s32 mss_crypto_raw(int argc, char *const argv[])
{
    mt_s32 ret = 0;
    struct mss_crypto_raw_data *testcase;
    mt_u32 algo = MSS_NOT_INITIALIZED;
    mt_u32 mode = MSS_NOT_INITIALIZED;
    mt_u32 op = MSS_NOT_INITIALIZED;
    mt_u32 i;
    MSS_CPU_MODE_E cpu_mode = MSS_APCPU;
    mt_u32 printsimple = 0;

    if (argc > 0 && strcmp(argv[0], "-t") == 0) {
        cpu_mode = MSS_SECCPU;
        argv++;
        argc--;
    }

    if (argc == 0) {
        for (i = 0; i < ARRAY_SIZE(m2m_raw_testcase); i++) {
            testcase = &m2m_raw_testcase[i];
            if (_sym6_crypto_raw_algo_mode_check(testcase->algo, testcase->mode) == 0) {
                crypto_algo_mode_print(testcase->algo, testcase->mode);
                printf(" raw not support\n");
                continue;
            }

            if (cpu_mode == MSS_SECCPU) {
                ret = mss_m2m_raw_tee_test(testcase, MT_CIPHER_OPERATION_ENCRYPT, printsimple);
                if (ret != MT_SUCCESS)
                    return ret;

                ret = mss_m2m_raw_tee_test(testcase, MT_CIPHER_OPERATION_DECRYPT, printsimple);
                if (ret != MT_SUCCESS)
                    return ret;
            } else {
                ret = mss_m2m_raw_test(testcase, MT_CIPHER_OPERATION_ENCRYPT, printsimple);
                if (ret != MT_SUCCESS)
                    return ret;

                ret = mss_m2m_raw_test(testcase, MT_CIPHER_OPERATION_DECRYPT, printsimple);
                if (ret != MT_SUCCESS)
                    return ret;
            }
        }
    } else if (argc >= 3) {
        op = get_crypto_operation(argv[0]);
        algo = get_crypto_algorithm(argv[1]);
        mode = get_crypto_workmode(argv[2]);

        if (op == MSS_NOT_INITIALIZED || algo == MSS_NOT_INITIALIZED || mode == MSS_NOT_INITIALIZED)
            return 0;

        if (_sym6_crypto_raw_algo_mode_check(algo, mode) == 0) {
            crypto_algo_mode_print(algo, mode);
            printf(" raw not support\n");
            return 0;
        }

        testcase =  mss_m2m_raw_testcase_get(algo, mode);

        if (cpu_mode == MSS_SECCPU) {
            ret = mss_m2m_raw_tee_test(testcase, op, printsimple);
            if (ret != MT_SUCCESS)
                return ret;
        }
        else {
            ret = mss_m2m_raw_test(testcase, op, printsimple);
            if (ret != MT_SUCCESS)
                return ret;
        }
    } else {
        return -1;
    }

    return 0;
}

static mt_s32 mss_crypto_ts(int argc, char *const argv[])
{
    mt_s32 ret = 0;
    struct mss_crypto_ts_data *testcase;
    mt_u32 algo = MSS_NOT_INITIALIZED;
    mt_u32 mode = MSS_NOT_INITIALIZED;
    mt_u32 op = MSS_NOT_INITIALIZED;
    mt_u32 i;
    MSS_CPU_MODE_E cpu_mode = MSS_APCPU;
    mt_u32 printsimple = 0;

    if (argc > 0 && strcmp(argv[0], "-t") == 0) {
        cpu_mode = MSS_SECCPU;
        argv++;
        argc--;
    }

    if (argc == 0) {
        for (i = 0; i < ARRAY_SIZE(m2m_ts_testcase); i++) {
            testcase = &m2m_ts_testcase[i];
            if (_sym6_crypto_ts_algo_mode_check(testcase->algo, testcase->mode) == 0) {
                crypto_algo_mode_print(testcase->algo, testcase->mode);
                printf(" ts not support\n");
                continue;
            }

            if (cpu_mode == MSS_SECCPU) {
                ret = mss_m2m_ts_tee_test(testcase, MT_CIPHER_OPERATION_DECRYPT, printsimple);
                if (ret != MT_SUCCESS)
                    return ret;

                ret = mss_m2m_ts_tee_test(testcase, MT_CIPHER_OPERATION_ENCRYPT, printsimple);
                if (ret != MT_SUCCESS)
                    return ret;
            } else {
                ret = mss_m2m_ts_test(testcase, MT_CIPHER_OPERATION_DECRYPT, printsimple);
                if (ret != MT_SUCCESS)
                    return ret;

                ret = mss_m2m_ts_test(testcase, MT_CIPHER_OPERATION_ENCRYPT, printsimple);
                if (ret != MT_SUCCESS)
                    return ret;
            }
        }
    } else if (argc >= 3) {
        op = get_crypto_operation(argv[0]);
        algo = get_crypto_algorithm(argv[1]);
        mode = get_crypto_workmode(argv[2]);

        if (op == MSS_NOT_INITIALIZED || algo == MSS_NOT_INITIALIZED || mode == MSS_NOT_INITIALIZED)
            return 0;

        if (_sym6_crypto_ts_algo_mode_check(algo, mode) == 0) {
            crypto_algo_mode_print(algo, mode);
            printf(" ts not support\n");
            return 0;
        }

        testcase =  mss_m2m_ts_testcase_get(algo, mode);

        if (cpu_mode == MSS_SECCPU) {
            ret = mss_m2m_ts_tee_test(testcase, op, printsimple);
            if (ret != MT_SUCCESS)
                return ret;
        }
        else {
            ret = mss_m2m_ts_test(testcase, op, printsimple);
            if (ret != MT_SUCCESS)
                return ret;
        }
    } else {
        return -1;
    }

    return 0;
}

static void mss_crypto_cmd_help(void)
{
    mss_print("Usage:\n");

    mss_print("\ncrypto raw [-t] [<op> <algo> <mode>]\n");
    mss_print("\t crypto raw test\n");
    mss_print("\t [-t]: tee crypto raw test\n");
    mss_print("\t <op>: enc|dec\n");
    mss_print("\t <algo>: aes|tdes|des|sm4\n");
    mss_print("\t <mode>: ecb|cbc|ctr|cbcdvs042|cbccts|cbcs|cens\n");

    mss_print("\ncrypto ts [-t] [<op> <algo> <mode>]\n");
    mss_print("\t crypto ts test\n");
    mss_print("\t [-t]: tee crypto ts test\n");
    mss_print("\t <op>: enc|dec\n");
    mss_print("\t <algo>: aes|tdes|des|sm4\n");
    mss_print("\t <mode>: ecb|cbc|cbcdvs042|cbccts|ecbcts\n");

}

mt_s32 mss_crypto_test(int argc, char *const argv[]);

mt_s32 mss_crypto_test(int argc, char *const argv[])
{
    mt_s32 ret = 0;
    char *cmd;

    if (0 == strncmp("-h", argv[0], 2)){
        mss_crypto_cmd_help();
        return 0;
    }

    cmd = argv[0];
    if (0 == strncmp("raw", cmd, 3)) {
        ret = mss_crypto_raw(argc - 1, argv + 1);
    }
    else if (0 == strncmp("ts", cmd, 2)){
        ret = mss_crypto_ts(argc - 1, argv + 1);
    }

    return ret;
}

