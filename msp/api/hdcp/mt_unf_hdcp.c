/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "mt_type.h"
#include "mt_common.h"
#include "mt_unf_hdcp.h"
#include "mt_module_debug.h"
#ifdef CONFIG_MT_CHIP_SYMPHONY1
#include "hal_cipher.h"
#include "mt_unf_cipher.h"
#else
#include "mt_unf_cipher_v2.h"
#endif

#include "mt_unf_otp.h"
#include "mt_unf_misc.h"

#ifdef CONFIG_MT_CHIP_SYMPHONY6
#include "tee/tee_client_api.h"
#endif

//#define HDCP_KEY_DEBUG 1

#ifdef HDCP_KEY_DEBUG
static void dump(mt_u8 *buffer, mt_u32 len)
{
    printf("\n==================================\n");
    for(mt_u32 i = 0; i < len; i ++) {
        if((i % 16) == 0 && i != 0)
            printf("\n");
        printf("%02X ", buffer[i]);
    }
    printf("\n==================================\n");
}
#endif

#ifdef CONFIG_MT_CHIP_SYMPHONY1
static mt_s32 __encrypt_hdcpkey_for_sym1(MT_UNF_HDCP_HDCPKEY_S st_hdcpkey, mt_u8 out_encrypted_key[304])
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle cipher_handle;
    mt_handle keyladder_handle;
    MT_CIPHER_KEY_LADDER_SOURCE_E keyladder_source;
    MT_CIPHER_CTRL_S info;
    //mt_u32 data_length = 304;

    if (out_encrypted_key == NULL)
        return MT_FAILURE;

    if (st_hdcpkey.enc_flag == MT_FALSE) {
        keyladder_source = MT_CIPHER_KEY_LADDER_HDCP;
        ret = MT_UNF_CIPHER_keyladder_create(keyladder_source, 0, &keyladder_handle);
        if (ret != MT_SUCCESS)
            return ret;

        ret = MT_UNF_CIPHER_create(keyladder_handle, &cipher_handle);
        if (ret != MT_SUCCESS) {
            MT_UNF_CIPHER_keyladder_destroy(keyladder_handle);
            return ret;
        }

        memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));
        info.operation = MT_CIPHER_OPERATION_ENCRYPT;
        info.algorithm = MT_CIPHER_ALG_AES;
        info.parameter.aes_para.work_mode = MT_CIPHER_WORK_MODE_ECB;
        info.parameter.aes_para.key_length = 16;
        ret = MT_UNF_CIPHER_config(cipher_handle, &info);
        if (ret != MT_SUCCESS) {
            MT_UNF_CIPHER_keyladder_destroy(keyladder_handle);
            MT_UNF_CIPHER_destroy(cipher_handle);
            return ret;
        }

        ret = MT_UNF_CIPHER_process(cipher_handle, st_hdcpkey.hdcpkey, out_encrypted_key, 304);

        MT_UNF_CIPHER_keyladder_destroy(keyladder_handle);
        MT_UNF_CIPHER_destroy(cipher_handle);

    } else {
        /*If encrypted, the encrypt key is the below default one. You should use HDCP-key tool from Montage to encrypt your hdcp-key*/
        mt_u8 default_key[16] = {0x2C,0x96,0xB4,0xE0,0x3E,0xBB,0xA4,0x95,0xD0,0xC0,0x6F,0x1C,0x24,0x34,0x53,0x75};
        mt_u8 *decrypted_key = malloc(304);

        if (decrypted_key != NULL)
        {
        	memset(decrypted_key, 0, 304);
        }
        else
        {
			return MT_FAILURE;
        }

        //Step1:aes-ecb decrypt use default_key
        ret = MT_UNF_CIPHER_create((mt_handle)NULL, &cipher_handle);
        if (ret != MT_SUCCESS) {
			if (decrypted_key != NULL)
				free(decrypted_key);

            return ret;
        }

        memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));
        info.operation = MT_CIPHER_OPERATION_DECRYPT;
        info.algorithm = MT_CIPHER_ALG_AES;
        info.parameter.aes_para.work_mode = MT_CIPHER_WORK_MODE_ECB;
        info.parameter.aes_para.key_length = 16;
        info.parameter.aes_para.p_key = default_key;
        ret = MT_UNF_CIPHER_config(cipher_handle, &info);
        if (ret != MT_SUCCESS) {
            MT_UNF_CIPHER_destroy(cipher_handle);

			if (decrypted_key != NULL)
				free(decrypted_key);

            return ret;
        }

        ret = MT_UNF_CIPHER_process(cipher_handle, st_hdcpkey.hdcpkey, decrypted_key, 304);
        MT_UNF_CIPHER_destroy(cipher_handle);
        if (ret != MT_SUCCESS)
        {
			if (decrypted_key != NULL)
				free(decrypted_key);

            return ret;
		}

        //Step2: encrypt with hdcp protection key
        keyladder_source = MT_CIPHER_KEY_LADDER_HDCP;
        ret = MT_UNF_CIPHER_keyladder_create(keyladder_source, 0, (mt_handle*)&keyladder_handle);
        if (ret != MT_SUCCESS)
        {
			if (decrypted_key != NULL)
				free(decrypted_key);

            return ret;
        }

        ret = MT_UNF_CIPHER_create(keyladder_handle, &cipher_handle);
        if (ret != MT_SUCCESS) {
            MT_UNF_CIPHER_keyladder_destroy(keyladder_handle);

			if (decrypted_key != NULL)
				free(decrypted_key);

            return ret;
        }

        memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));
        info.operation = MT_CIPHER_OPERATION_ENCRYPT;
        info.algorithm = MT_CIPHER_ALG_AES;
        info.parameter.aes_para.work_mode = MT_CIPHER_WORK_MODE_ECB;
        info.parameter.aes_para.key_length = 16;
        ret = MT_UNF_CIPHER_config(cipher_handle, &info);
        if (ret != MT_SUCCESS) {
            MT_UNF_CIPHER_keyladder_destroy(keyladder_handle);
            MT_UNF_CIPHER_destroy(cipher_handle);

			if (decrypted_key != NULL)
				free(decrypted_key);

            return ret;
        }

        ret = MT_UNF_CIPHER_process(cipher_handle, decrypted_key, out_encrypted_key, 304);

        MT_UNF_CIPHER_keyladder_destroy(keyladder_handle);
        MT_UNF_CIPHER_destroy(cipher_handle);
        free(decrypted_key);

    }

    return ret;
}
#elif defined(CONFIG_MT_CHIP_SYMPHONY2)
static mt_s32 __encrypt_hdcpkey_for_sym2(MT_UNF_HDCP_HDCPKEY_S st_hdcpkey, mt_u8 out_encrypted_key[304])
{
	mt_s32 ret = MT_SUCCESS;
	mt_u32 slot_id = MT_CIPHER_KEYSLOT_INVALID;
	unsigned int kl_handle = 0;
	MT_CIPHER_CTRL_S info;
	mt_handle p_cipher;
	if (out_encrypted_key == NULL)
		return MT_FAILURE;
	if (st_hdcpkey.enc_flag == MT_FALSE) {
		mt_unf_cipher_keyslot_request(&slot_id);
		ret = mt_unf_cipher_keyladder_create(MT_CIPHER_KEYLADDER_0, (mt_handle*) &kl_handle);
		if (ret != 0) {
			printf("keyladder create error\n");
			ret = MT_FAILURE;
			goto free_keyslot1;
		}
		ret = mt_unf_cipher_keyladder_start(kl_handle, MT_CIPHER_KEYLADDER_SCK_8);
		if (ret != 0) {
			printf("keyladder start error\n");
			ret = MT_FAILURE;
			goto free_keyladder1;
		}
		mt_unf_cipher_keyladder_end(kl_handle, slot_id);
		mt_unf_cipher_keyladder_destroy(kl_handle);
		memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));
		info.operation = MT_CIPHER_OPERATION_ENCRYPT;
		info.algorithm = MT_CIPHER_ALG_AES;
		info.work_mode = MT_CIPHER_WORK_MODE_ECB;
		ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &p_cipher);
		ret |= mt_unf_cipher_crypto_config(p_cipher, &info, slot_id);
		ret |= mt_unf_cipher_crypto_process(p_cipher, st_hdcpkey.hdcpkey, out_encrypted_key, 304);
		if (ret != MT_SUCCESS) {
			printf("aes error\n");
			goto free_crypto1;
		}
	free_crypto1:
		if (p_cipher != MT_INVALID_HANDLE)
			mt_unf_cipher_crypto_destroy(p_cipher);
	free_keyladder1:
		if (kl_handle != 0)
			mt_unf_cipher_keyladder_destroy(kl_handle);
	free_keyslot1:
		if (slot_id != MT_CIPHER_KEYSLOT_INVALID)
			 mt_unf_cipher_keyslot_release(slot_id);
		return ret;
	} else {
		mt_u8 default_key[16] = {0x2C,0x96,0xB4,0xE0,0x3E,0xBB,0xA4,0x95,0xD0,0xC0,0x6F,0x1C,0x24,0x34,0x53,0x75};
		mt_u8 *decrypted_key = malloc(304);
		memset(decrypted_key, 0, 304);
		mt_unf_cipher_keyslot_request(&slot_id);
    		memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));
		info.operation = MT_CIPHER_OPERATION_DECRYPT;
		info.algorithm = MT_CIPHER_ALG_AES;
		info.work_mode = MT_CIPHER_WORK_MODE_ECB;
		mt_unf_cipher_keyslot_set(slot_id, &info, default_key, NULL);
		ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &p_cipher);
		ret |= mt_unf_cipher_crypto_config(p_cipher, &info, slot_id);
		ret |= mt_unf_cipher_crypto_process(p_cipher, st_hdcpkey.hdcpkey, decrypted_key, 304);
		if (p_cipher != MT_INVALID_HANDLE)
			mt_unf_cipher_crypto_destroy(p_cipher);
		if (slot_id != MT_CIPHER_KEYSLOT_INVALID)
			mt_unf_cipher_keyslot_release(slot_id);
		if (ret != MT_SUCCESS)
			return ret;
		mt_unf_cipher_keyslot_request(&slot_id);
		ret = mt_unf_cipher_keyladder_create(MT_CIPHER_KEYLADDER_0, (mt_handle*)&kl_handle);
		if (ret != 0) {
			printf("keyladder create error\n");
			ret = MT_FAILURE;
			goto free_keyslot2;
		}
		ret = mt_unf_cipher_keyladder_start(kl_handle, MT_CIPHER_KEYLADDER_SCK_8);
		if (ret != 0) {
			printf("keyladder start error\n");
			ret = MT_FAILURE;
			goto free_keyladder2;
		}
		mt_unf_cipher_keyladder_end(kl_handle, slot_id);
		mt_unf_cipher_keyladder_destroy(kl_handle);
		memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));
		info.operation = MT_CIPHER_OPERATION_ENCRYPT;
		info.algorithm = MT_CIPHER_ALG_AES;
		info.work_mode = MT_CIPHER_WORK_MODE_ECB;
		ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &p_cipher);
		ret |= mt_unf_cipher_crypto_config(p_cipher, &info, slot_id);
		ret |= mt_unf_cipher_crypto_process(p_cipher, decrypted_key, out_encrypted_key, 304);
		if (ret != MT_SUCCESS) {
			printf("aes error\n");
			goto free_crypto2;
		}
	free_crypto2:
		if (p_cipher != MT_INVALID_HANDLE)
			mt_unf_cipher_crypto_destroy(p_cipher);
	free_keyladder2:
		if (kl_handle != 0)
			mt_unf_cipher_keyladder_destroy(kl_handle);
	free_keyslot2:
		if (slot_id != MT_CIPHER_KEYSLOT_INVALID)
			 mt_unf_cipher_keyslot_release(slot_id);
		return ret;
    }
}

#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
#define OTP_CHIPUIDH_BIT_OFFSET	 		(0xeae << 5)
#define OTP_CHIPUIDH_BIT_SIZE   		32
#define OTP_CHIPUIDL_BIT_OFFSET 		(0xead << 5)
#define OTP_CHIPUIDL_BIT_SIZE   		32
#define HDCP_KEY_SIZE					304
#define KEYLADDER_CMD_SIG_SIZE			32
#define KEYLADDER_HWSCK3_CMD_SIG_FILE	"/usr/local/stbdata/hdcp_kl_sig.bin"

static mt_s32 hdcp_read_file(const char *file, mt_u8 *buf, mt_u32 len)
{
	mt_s32 fd = 0;
	mt_s32 r_len = 0;

	if (!file || !buf || !len) {
		printf("Error parameter!");
		return MT_FAILURE;
	}
	fd = open(file, O_RDONLY);
	if (fd < 0) {
		printf("File: %s open failed!\n", file);
		return MT_FAILURE;
	}

	r_len = read(fd, buf, len);
	if (r_len != len) {
		printf("Read failed: read len: %d, len: %d\n", r_len, len);
		close(fd);
		return MT_FAILURE;
	}
	close(fd);
	return MT_SUCCESS;
}

static mt_s32 hdcp_get_kl_sig_from_file(mt_u8 *buf, mt_u32 len)
{
	return hdcp_read_file(KEYLADDER_HWSCK3_CMD_SIG_FILE, buf, len);
}

mt_u8 default_hwsck3_kdcmd_sig[KEYLADDER_CMD_SIG_SIZE] = {
	0x09,0xF1,0x32,0xCE,0xA9,0x98,0x51,0x67,0x08,0xB3,0x0A,0x35,0x7F,0x51,0x22,0xF6,0xCE,0x4A,0xAF,0x5B,0xE0,0xD2,0x99,0x7D,0x15,0x43,0xD1,0x8D,0x43,0x16,0xB3,0xFF,
};

static mt_s32 hdcp_set_3lvl_tdes_keyladder(mt_u8 *key1, mt_u8 *key2, mt_u8 *key3,
		mt_u8 sig[KEYLADDER_CMD_SIG_SIZE], mt_u32 slotID)
{
	mt_s32 ret = MT_FAILURE;
	mt_u32 p_keyladder = 0;
	MT_CIPHER_CTRL_S p_ctrl = {0,};

	if (key1 == NULL || key2 == NULL || key3 == NULL || sig == NULL) {
		printf("Bad parameters input!!\n");
		return MT_FAILURE;
	}

	p_ctrl.core = MT_CIPHER_CORE_M2M_RAW;
	p_ctrl.work_mode = MT_CIPHER_WORK_MODE_CBC;
	p_ctrl.operation = MT_CIPHER_OPERATION_DECRYPT;
	p_ctrl.algorithm = MT_CIPHER_ALG_TDES;

	ret = mt_unf_cipher_keyladder_create(0, &p_keyladder);
	if(ret != MT_SUCCESS) {
		printf("keyladder create error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

	ret = mt_unf_cipher_keyladder_start(p_keyladder,MT_CIPHER_KEYLADDER_HWSCK_3);
	if(ret != MT_SUCCESS) {
		printf("keyladder start error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

	ret = mt_unf_cipher_keyladder_set_signature(p_keyladder, sig);
	if(ret != MT_SUCCESS) {
		printf("keyladder set_signature error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

	ret = mt_unf_cipher_keyladder_link(p_keyladder, &p_ctrl, key1, 16);
	if(ret != MT_SUCCESS) {
		printf("keyladder link key1 error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

	ret = mt_unf_cipher_keyladder_link(p_keyladder, &p_ctrl, key2, 16);
	if(ret != MT_SUCCESS) {
		printf("keyladder link key2 error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

	ret = mt_unf_cipher_keyladder_link(p_keyladder, &p_ctrl, key3, 16);
	if(ret != MT_SUCCESS) {
		printf("keyladder link key3 error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

	ret = mt_unf_cipher_keyladder_end(p_keyladder, slotID);
	if(ret != MT_SUCCESS) {
		printf("keyladder end error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

	if (p_keyladder != 0)
	{
		ret = mt_unf_cipher_keyladder_destroy(p_keyladder);
		p_keyladder = 0;
	}

	return ret;
}

static mt_s32 __encrypt_hdcpkey_for_sym4(MT_UNF_HDCP_HDCPKEY_S st_hdcpkey, mt_u8 out_encrypted_key[HDCP_KEY_SIZE])
{
	mt_s32 ret = MT_FAILURE;
	mt_u32 slot_id = MT_CIPHER_KEYSLOT_INVALID;
	mt_handle p_cipher = MT_INVALID_HANDLE;
	mt_u8 *hdcp_input = NULL;
	mt_u8 *clear_hdcpkey = NULL;
	mt_u8 *encrypted_hdcpkey = NULL;
	mt_u8 *kl_sig = NULL;
	mt_u32 input0[4] = {0};
	mt_u32 input1[4] = {0};
	mt_u32 input2[4] = {0};
	MT_CIPHER_CTRL_S info;
	mt_u8 default_dec_key[16] = {0x2C, 0x96, 0xB4, 0xE0, 0x3E, 0xBB, 0xA4, 0x95, 0xD0, 0xC0, 0x6F, 0x1C, 0x24, 0x34, 0x53, 0x75};

	if (out_encrypted_key == NULL || st_hdcpkey.hdcpkey == NULL) {
		printf("Bad hdcp encrypt input parameters!\n");
		return MT_FAILURE;
	}

	hdcp_input = mt_unf_cipher_malloc(HDCP_KEY_SIZE);
	if (hdcp_input == NULL) {
		printf("Error malloc!\n");
		goto free_buffer;
	}
	memset(hdcp_input, 0, HDCP_KEY_SIZE);
	memcpy(hdcp_input, st_hdcpkey.hdcpkey, HDCP_KEY_SIZE);

	encrypted_hdcpkey = mt_unf_cipher_malloc(HDCP_KEY_SIZE);
	if (encrypted_hdcpkey == NULL) {
		printf("error malloc\n");
		goto free_buffer;
	}
	memset(encrypted_hdcpkey, 0, HDCP_KEY_SIZE);

	clear_hdcpkey = mt_unf_cipher_malloc(HDCP_KEY_SIZE);
	if (clear_hdcpkey == NULL) {
		printf("error malloc\n");
		goto free_buffer;
	}
	memset(clear_hdcpkey, 0, HDCP_KEY_SIZE);

	kl_sig = mt_unf_cipher_malloc(KEYLADDER_CMD_SIG_SIZE);
	if (kl_sig == NULL) {
		printf("error malloc\n");
		goto free_buffer;
	}
	memset(kl_sig, 0, KEYLADDER_CMD_SIG_SIZE);

	if (st_hdcpkey.enc_flag)
	{
		ret = mt_unf_cipher_keyslot_request(&slot_id);
		if (ret != MT_SUCCESS) {
			ret = MT_FAILURE;
			goto free_keyslot;
		}

		memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));
		info.operation = MT_CIPHER_OPERATION_DECRYPT;
		info.algorithm = MT_CIPHER_ALG_AES;
		info.work_mode = MT_CIPHER_WORK_MODE_ECB;
		ret = mt_unf_cipher_keyslot_set(slot_id, &info, default_dec_key, NULL);
		if (ret != MT_SUCCESS) {
			ret = MT_FAILURE;
			goto free_crypto;
		}

		ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &p_cipher);
		ret |= mt_unf_cipher_crypto_config(p_cipher, &info, slot_id);
		ret |= mt_unf_cipher_crypto_process(p_cipher, hdcp_input, clear_hdcpkey, HDCP_KEY_SIZE);
		if (ret != MT_SUCCESS) {
			ret = MT_FAILURE;
			goto free_crypto;
		}

		if (p_cipher != MT_INVALID_HANDLE)
			mt_unf_cipher_crypto_destroy(p_cipher);

		if (slot_id != MT_CIPHER_KEYSLOT_INVALID)
			mt_unf_cipher_keyslot_release(slot_id);
	} else {
		memcpy(clear_hdcpkey, hdcp_input, HDCP_KEY_SIZE);
	}

	ret= mt_unf_cipher_keyslot_request(&slot_id);
	if (ret != MT_SUCCESS) {
		goto free_keyslot;
	}

	input0[0] = 0x19190729;
	input0[1] = 0x2019ab0e;

	MT_UNF_OTP_Init();
	MT_UNF_OTP_read(OTP_CHIPUIDH_BIT_OFFSET, OTP_CHIPUIDH_BIT_SIZE, (MT_U32 *)&input0[2]);
	MT_UNF_OTP_read(OTP_CHIPUIDL_BIT_OFFSET, OTP_CHIPUIDL_BIT_SIZE, (MT_U32 *)&input0[3]);

	input1[0] = 0x136489ac;
	input1[1] = 0x2587eb0f;
	input1[2] = 0x3465879a;
	input1[3] = 0xed752423;

	input2[0] = 0x88663d9a;
	input2[1] = 0xbb98c2e1;
	input2[2] = 0xcc7612da;
	input2[3] = 0xad072829;

	if (hdcp_get_kl_sig_from_file(kl_sig, KEYLADDER_CMD_SIG_SIZE) != MT_SUCCESS)
		memcpy(kl_sig, default_hwsck3_kdcmd_sig, KEYLADDER_CMD_SIG_SIZE);

	hdcp_set_3lvl_tdes_keyladder((mt_u8 *)&input0, (mt_u8 *)&input1, (mt_u8 *)&input2, kl_sig, slot_id);

	memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));
	info.operation = MT_CIPHER_OPERATION_ENCRYPT;
	info.algorithm = MT_CIPHER_ALG_AES;
	info.work_mode = MT_CIPHER_WORK_MODE_ECB;
	info.core = MT_CIPHER_CORE_M2M_RAW;

	ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &p_cipher);
	ret |= mt_unf_cipher_crypto_config(p_cipher, &info, slot_id);
	ret |= mt_unf_cipher_crypto_process(p_cipher,clear_hdcpkey, encrypted_hdcpkey, HDCP_KEY_SIZE);
	if (ret != MT_SUCCESS) {
		printf("aes error!!, ret: %d\n", ret);
		goto free_crypto;
	}

	memcpy(out_encrypted_key, encrypted_hdcpkey, HDCP_KEY_SIZE);

free_crypto:
	if (p_cipher != MT_INVALID_HANDLE)
		mt_unf_cipher_crypto_destroy(p_cipher);

free_keyslot:
	if (slot_id != MT_CIPHER_KEYSLOT_INVALID)
		mt_unf_cipher_keyslot_release(slot_id);

free_buffer:
	if (kl_sig != NULL) {
		mt_unf_cipher_free(kl_sig);
		kl_sig = NULL;
	}
	if (hdcp_input != NULL) {
		mt_unf_cipher_free(hdcp_input);
		hdcp_input = NULL;
	}
	if (clear_hdcpkey != NULL) {
		mt_unf_cipher_free(clear_hdcpkey);
		clear_hdcpkey = NULL;
	}
	if (encrypted_hdcpkey != NULL) {
		mt_unf_cipher_free(encrypted_hdcpkey);
		encrypted_hdcpkey = NULL;
	}

	return ret;
}
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
#define OTP_CHIPUIDH_BIT_OFFSET	 		(0xeae << 5)
#define OTP_CHIPUIDH_BIT_SIZE   		32
#define OTP_CHIPUIDL_BIT_OFFSET 		(0xead << 5)
#define OTP_CHIPUIDL_BIT_SIZE   		32
#define HDCP_KEY_SIZE					304
#define KEYLADDER_CMD_SIG_SIZE			32
#define KEYLADDER_HWSCK3_CMD_SIG_FILE	"/usr/local/stbdata/hdcp_kl_sig.bin"

static mt_s32 hdcp_read_file(const char *file, mt_u8 *buf, mt_u32 len)
{
	mt_s32 fd = 0;
	mt_s32 r_len = 0;

	if (!file || !buf || !len) {
		printf("Error parameter!");
		return MT_FAILURE;
	}
	fd = open(file, O_RDONLY);
	if (fd < 0) {
		printf("File: %s open failed!\n", file);
		return MT_FAILURE;
	}

	r_len = read(fd, buf, len);
	if (r_len != len) {
		printf("Read failed: read len: %d, len: %d\n", r_len, len);
		close(fd);
		return MT_FAILURE;
	}
	close(fd);
	return MT_SUCCESS;
}

static mt_s32 hdcp_get_kl_sig_from_file(mt_u8 *buf, mt_u32 len)
{
	return hdcp_read_file(KEYLADDER_HWSCK3_CMD_SIG_FILE, buf, len);
}

mt_u8 default_hwsck3_kdcmd_sig[KEYLADDER_CMD_SIG_SIZE] = {
	0x09,0xF1,0x32,0xCE,0xA9,0x98,0x51,0x67,0x08,0xB3,0x0A,0x35,0x7F,0x51,0x22,0xF6,0xCE,0x4A,0xAF,0x5B,0xE0,0xD2,0x99,0x7D,0x15,0x43,0xD1,0x8D,0x43,0x16,0xB3,0xFF,
};

static mt_s32 hdcp_set_3lvl_tdes_keyladder(mt_u8 *key1, mt_u8 *key2, mt_u8 *key3,
		mt_u8 sig[KEYLADDER_CMD_SIG_SIZE], mt_u32 slotID)
{
	mt_s32 ret = MT_FAILURE;
	mt_handle p_keyladder = 0;
	MT_CIPHER_CTRL_S p_ctrl = {0,};

	if (key1 == NULL || key2 == NULL || key3 == NULL || sig == NULL) {
		printf("Bad parameters input!!\n");
		return MT_FAILURE;
	}

	p_ctrl.core = MT_CIPHER_CORE_M2M_RAW;
	p_ctrl.work_mode = MT_CIPHER_WORK_MODE_ECB;
	p_ctrl.operation = MT_CIPHER_OPERATION_DECRYPT;
	p_ctrl.algorithm = MT_CIPHER_ALG_TDES;

	ret = mt_unf_cipher_keyladder_create(0, &p_keyladder);
	if(ret != MT_SUCCESS) {
		printf("keyladder create error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

	ret = mt_unf_cipher_keyladder_start(p_keyladder,MT_CIPHER_KEYLADDER_HWSCK_3);
	if(ret != MT_SUCCESS) {
		printf("keyladder start error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

	ret = mt_unf_cipher_keyladder_set_signature(p_keyladder, sig);
	if(ret != MT_SUCCESS) {
		printf("keyladder set_signature error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

	ret = mt_unf_cipher_keyladder_link(p_keyladder, &p_ctrl, key1, 16);
	if(ret != MT_SUCCESS) {
		printf("keyladder link key1 error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

	ret = mt_unf_cipher_keyladder_link(p_keyladder, &p_ctrl, key2, 16);
	if(ret != MT_SUCCESS) {
		printf("keyladder link key2 error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

	ret = mt_unf_cipher_keyladder_link(p_keyladder, &p_ctrl, key3, 16);
	if(ret != MT_SUCCESS) {
		printf("keyladder link key3 error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

	ret = mt_unf_cipher_keyladder_end(p_keyladder, slotID);
	if(ret != MT_SUCCESS) {
		printf("keyladder end error! ret: %d\n", ret);
		ret = MT_FAILURE;
	}

    if (p_keyladder != 0) {
        ret = mt_unf_cipher_keyladder_destroy(p_keyladder);
        p_keyladder = 0;
    }

    return ret;
}

static void dump_data(mt_u8 *data, mt_u32 len)
{
    int i;

    for (i = 0; i < len; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");
}


static mt_s32 __encrypt_hdcpkey_for_sym6(MT_UNF_HDCP_HDCPKEY_S st_hdcpkey, mt_u8 out_encrypted_key[HDCP_KEY_SIZE])
{
	mt_s32 ret = MT_FAILURE;
	mt_u32 slot_id = MT_CIPHER_KEYSLOT_INVALID;
	mt_handle p_cipher = MT_INVALID_HANDLE;
	mt_u8 *hdcp_input = NULL;
	mt_u8 *clear_hdcpkey = NULL;
	mt_u8 *encrypted_hdcpkey = NULL;
	mt_u8 *kl_sig = NULL;

    MT_CIPHER_CTRL_S info;
    mt_u8 default_dec_key[16] = {0x2C, 0x96, 0xB4, 0xE0, 0x3E, 0xBB, 0xA4, 0x95, 0xD0, 0xC0, 0x6F, 0x1C, 0x24, 0x34, 0x53, 0x75};
    uint8_t hdcp_intermediate_key[48] = {
        0xd4, 0x89, 0x4f, 0x72, 0x4b, 0x64, 0x1d, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xa0, 0x69, 0xfa, 0x5b, 0x87, 0x1b, 0x47, 0x2d, 0x96, 0x30, 0xbd, 0xa2, 0x7e, 0x10, 0xf6, 0xf2,
        0xf9, 0x92, 0x6a, 0x44, 0xd5, 0xbe, 0xdc, 0x07, 0x6b, 0xe6, 0x18, 0x63, 0x35, 0x41, 0xa3, 0xe5,
    };

	if (out_encrypted_key == NULL || st_hdcpkey.hdcpkey == NULL) {
		printf("Bad hdcp encrypt input parameters!\n");
		return MT_FAILURE;
	}

    hdcp_input = mt_unf_cipher_malloc(HDCP_KEY_SIZE);
    if (hdcp_input == NULL) {
        printf("Error malloc!\n");
        goto free_buffer;
    }
    memset(hdcp_input, 0, HDCP_KEY_SIZE);
    memcpy(hdcp_input, st_hdcpkey.hdcpkey, HDCP_KEY_SIZE);

    encrypted_hdcpkey = mt_unf_cipher_malloc(HDCP_KEY_SIZE);
    if (encrypted_hdcpkey == NULL) {
        printf("error malloc\n");
        goto free_buffer;
    }
    memset(encrypted_hdcpkey, 0, HDCP_KEY_SIZE);

    clear_hdcpkey = mt_unf_cipher_malloc(HDCP_KEY_SIZE);
    if (clear_hdcpkey == NULL) {
        printf("error malloc\n");
        goto free_buffer;
    }
    memset(clear_hdcpkey, 0, HDCP_KEY_SIZE);

    kl_sig = mt_unf_cipher_malloc(KEYLADDER_CMD_SIG_SIZE);
    if (kl_sig == NULL) {
        printf("error malloc\n");
        goto free_buffer;
    }
    memset(kl_sig, 0, KEYLADDER_CMD_SIG_SIZE);

    if (st_hdcpkey.enc_flag) {
        ret = mt_unf_cipher_keyslot_request(&slot_id);
        if (ret != MT_SUCCESS) {
            ret = MT_FAILURE;
            goto free_keyslot;
        }

        memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));
        info.operation = MT_CIPHER_OPERATION_DECRYPT;
        info.algorithm = MT_CIPHER_ALG_AES;
        info.work_mode = MT_CIPHER_WORK_MODE_ECB;
        ret = mt_unf_cipher_keyslot_set(slot_id, &info, default_dec_key, NULL);
        if (ret != MT_SUCCESS) {
            ret = MT_FAILURE;
            goto free_crypto;
        }

        ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &p_cipher);
        ret |= mt_unf_cipher_crypto_config(p_cipher, &info, slot_id);
        ret |= mt_unf_cipher_crypto_process(p_cipher, hdcp_input, clear_hdcpkey, HDCP_KEY_SIZE);
        if (ret != MT_SUCCESS) {
            ret = MT_FAILURE;
            goto free_crypto;
        }

        if (p_cipher != MT_INVALID_HANDLE)
            mt_unf_cipher_crypto_destroy(p_cipher);

        if (slot_id != MT_CIPHER_KEYSLOT_INVALID)
            mt_unf_cipher_keyslot_release(slot_id);
    } else {
        memcpy(clear_hdcpkey, hdcp_input, HDCP_KEY_SIZE);
    }

    ret = mt_unf_cipher_keyslot_request(&slot_id);
    if (ret != MT_SUCCESS) {
        goto free_keyslot;
    }
    MT_UNF_OTP_Init();
    //OTP_ChipUIDH and OTP_ChipUIDL are reversed in TEE-loader operation
    MT_UNF_OTP_read(OTP_CHIPUIDH_BIT_OFFSET, OTP_CHIPUIDH_BIT_SIZE, (MT_U32 *)&hdcp_intermediate_key[8]);
    MT_UNF_OTP_read(OTP_CHIPUIDL_BIT_OFFSET, OTP_CHIPUIDL_BIT_SIZE, (MT_U32 *)&hdcp_intermediate_key[12]);

    dump_data(&hdcp_intermediate_key[0], 16);



    if (hdcp_get_kl_sig_from_file(kl_sig, KEYLADDER_CMD_SIG_SIZE) != MT_SUCCESS)
        memcpy(kl_sig, default_hwsck3_kdcmd_sig, KEYLADDER_CMD_SIG_SIZE);

    dump_data(kl_sig, 16);
    hdcp_set_3lvl_tdes_keyladder((mt_u8 *)&hdcp_intermediate_key[0], (mt_u8 *)&hdcp_intermediate_key[16], (mt_u8 *)&hdcp_intermediate_key[32], kl_sig, slot_id);

    memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));
    info.operation = MT_CIPHER_OPERATION_ENCRYPT;
    info.algorithm = MT_CIPHER_ALG_AES;
    info.work_mode = MT_CIPHER_WORK_MODE_CBC;
    info.core = MT_CIPHER_CORE_M2M_RAW;

    ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &p_cipher);
    ret |= mt_unf_cipher_crypto_config(p_cipher, &info, slot_id);
    ret |= mt_unf_cipher_crypto_process(p_cipher, clear_hdcpkey, encrypted_hdcpkey, HDCP_KEY_SIZE);
    if (ret != MT_SUCCESS) {
        printf("aes error!!, ret: %d\n", ret);
        goto free_crypto;
    }

    memcpy(out_encrypted_key, encrypted_hdcpkey, HDCP_KEY_SIZE);

free_crypto:
    if (p_cipher != MT_INVALID_HANDLE)
        mt_unf_cipher_crypto_destroy(p_cipher);

free_keyslot:
    if (slot_id != MT_CIPHER_KEYSLOT_INVALID)
        mt_unf_cipher_keyslot_release(slot_id);

free_buffer:
    if (kl_sig != NULL) {
        mt_unf_cipher_free(kl_sig);
        kl_sig = NULL;
    }
    if (hdcp_input != NULL) {
        mt_unf_cipher_free(hdcp_input);
        hdcp_input = NULL;
    }
    if (clear_hdcpkey != NULL) {
        mt_unf_cipher_free(clear_hdcpkey);
        clear_hdcpkey = NULL;
    }
    if (encrypted_hdcpkey != NULL) {
        mt_unf_cipher_free(encrypted_hdcpkey);
        encrypted_hdcpkey = NULL;
    }

    return ret;
}

#define HDCP_PROCESS_UUID { 0x7a2b9c4d, 0x3f8e, 0x41c9, \
        { 0xb2, 0x5d, 0x7e, 0x1a, 0xc3, 0x8f, 0x6b, 0x0d } }

#define PTA_SYMPHONY_DEC_AND_LOAD_HDCP_KEY      0x00000000
#define PTA_SYMPHONY_ENC_HDCP_KEY       			 0x00000001

static mt_s32 __load_encrypted_hdcpkey_for_sym6_by_tee(mt_u8 in_encrypted_key[HDCP_KEY_SIZE])
{
	mt_s32 ret = MT_FAILURE;
    
#if defined(CONFIG_MT_TEE_SUPPORT)
	TEEC_Result res = TEEC_ERROR_GENERIC;

    TEEC_Session sess;
    TEEC_UUID uuid = HDCP_PROCESS_UUID;
    TEEC_Operation op;
    uint32_t err_origin;
    unsigned char *buffer = NULL;
    TEEC_Context g_ctx;

    res = TEEC_InitializeContext(NULL, &g_ctx);
    if (res != TEEC_SUCCESS) {
        //printf("%s-%d\n", __func__, __LINE__);
        goto exit;
    }

    buffer = (unsigned char *)mt_unf_cipher_malloc(HDCP_KEY_SIZE);
    if(buffer == NULL) {
        //printf("%s-%d malloc failed.\n", __func__, __LINE__);
        goto exit;
    }

    memcpy(buffer, in_encrypted_key, HDCP_KEY_SIZE);

    res = TEEC_OpenSession(&g_ctx, &sess, &uuid,
                           TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS) {
        //printf("%s-%d  TEEC_OpenSession failed. err code: 0x%x err_origin:0x%x\n", __func__, __LINE__, res, err_origin);
        goto exit;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
                                     TEEC_NONE, TEEC_NONE);

    op.params[0].tmpref.buffer = buffer;
    op.params[0].tmpref.size = HDCP_KEY_SIZE;

    res = TEEC_InvokeCommand(&sess, PTA_SYMPHONY_DEC_AND_LOAD_HDCP_KEY, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        //printf("%s-%d TEEC_InvokeCommand failed.\n", __func__, __LINE__);
        goto exit;
    }
	ret = MT_SUCCESS;

exit:
    if(buffer) {
        mt_unf_cipher_free(buffer);
		buffer = NULL;
	}

    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&g_ctx);
#endif
    return ret;
}

static mt_s32 __encrypted_hdcpkey_for_sym6_by_tee(MT_UNF_HDCP_HDCPKEY_S st_hdcpkey, mt_u8 out_encrypted_key[HDCP_KEY_SIZE])
{
	mt_s32 ret = MT_FAILURE;
#if defined(CONFIG_MT_TEE_SUPPORT)
    TEEC_Result res = TEEC_ERROR_GENERIC;

    TEEC_Session sess;
    TEEC_UUID uuid = HDCP_PROCESS_UUID;
    TEEC_Operation op;
    uint32_t err_origin;
    unsigned char *buffer = NULL;
    TEEC_Context g_ctx;

    res = TEEC_InitializeContext(NULL, &g_ctx);
    if (res != TEEC_SUCCESS) {
        printf("%s-%d\n", __func__, __LINE__);
        goto exit;
    }

    buffer = (unsigned char *)mt_unf_cipher_malloc(HDCP_KEY_SIZE);
    if(buffer == NULL) {
        printf("%s-%d malloc failed.\n", __func__, __LINE__);
        goto exit;
    }

    memcpy(buffer, &st_hdcpkey.hdcpkey, HDCP_KEY_SIZE);

    res = TEEC_OpenSession(&g_ctx, &sess, &uuid,
                           TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS) {
        printf("%s-%d  TEEC_OpenSession failed. err code: 0x%x err_origin:0x%x\n", __func__, __LINE__, res, err_origin);
        goto exit;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT, TEEC_NONE,
                                     TEEC_NONE, TEEC_NONE);

    op.params[0].tmpref.buffer = buffer;
    op.params[0].tmpref.size = HDCP_KEY_SIZE;

    res = TEEC_InvokeCommand(&sess, PTA_SYMPHONY_ENC_HDCP_KEY, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        printf("%s-%d TEEC_InvokeCommand failed.\n", __func__, __LINE__);
        goto exit;
    } else {
        memcpy(out_encrypted_key, buffer, HDCP_KEY_SIZE);
    }
	ret = MT_SUCCESS;

exit:
    if(buffer) {
        mt_unf_cipher_free(buffer);
		buffer = NULL;
	}

    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&g_ctx);
#endif

    return ret;
}

#endif
mt_s32 MT_UNF_HDCP_encrypt_hdcpkey(MT_UNF_HDCP_HDCPKEY_S st_hdcpkey, mt_u8 out_encrypted_key[304])
{
	mt_s32 ret = MT_FAILURE;
#ifdef CONFIG_MT_CHIP_SYMPHONY1
    ret = __encrypt_hdcpkey_for_sym1(st_hdcpkey, out_encrypted_key);
#elif defined(CONFIG_MT_CHIP_SYMPHONY2)
    ret = __encrypt_hdcpkey_for_sym2(st_hdcpkey, out_encrypted_key);
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
    ret = __encrypt_hdcpkey_for_sym4(st_hdcpkey, out_encrypted_key);
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
    ret = __encrypt_hdcpkey_for_sym6(st_hdcpkey, out_encrypted_key);
#endif
    return ret;
}

mt_s32 MT_UNF_HDCP_load_hdcpkey(mt_u8 in_encrypted_key[304])
{
	mt_s32 ret = MT_FAILURE;
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    ret = __load_encrypted_hdcpkey_for_sym6_by_tee(in_encrypted_key);
#endif
    return ret;
}

mt_s32 MT_UNF_HDCP_encrypt_hdcpkey_tee(MT_UNF_HDCP_HDCPKEY_S st_hdcpkey, mt_u8 out_encrypted_key[304])
{
	mt_s32 ret = MT_FAILURE;
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    ret = __encrypted_hdcpkey_for_sym6_by_tee(st_hdcpkey, out_encrypted_key);
#endif
    return ret;
}
