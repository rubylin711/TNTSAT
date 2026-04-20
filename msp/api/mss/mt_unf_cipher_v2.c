/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <string.h>
#include <pthread.h>
#include "mt_common.h"
#include "mt_unf_cipher_v2.h"
#include "mt_mpi_kt.h"
#include "mt_mpi_ce.h"
#include "mt_mpi_keyladder.h"
#include "mt_mpi_cipher_v2.h"

#define goto_fail(func)        \
    {                          \
	goto _##func##_failed; \
    }

int mt_unf_cipher_init(void)
{
    return mt_mpi_cipher_init();
}

int mt_unf_cipher_deinit(void)
{
    return mt_mpi_cipher_deinit();
}

/******************************************************************************
 * KeySlot Functions
 *****************************************************************************/
int mt_unf_cipher_keyslot_request(unsigned int *p_keyslot)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kt_handle = MT_INVALID_HANDLE;

    if (mt_mpi_kt_open(&kt_handle)) {
        return MT_CIPHER_ERR_KT_REQUEST_FAILED;
    }

    ret = mt_mpi_kt_slot_request(kt_handle, p_keyslot);

    mt_mpi_kt_close(kt_handle);

    if (ret == SUCCESS) {
        if (*p_keyslot == MT_CIPHER_KEYSLOT_INVALID)
            return MT_CIPHER_ERR_KT_REQUEST_FAILED;
    }

    return ret;
}

int mt_unf_cipher_keyslot_request_multi(unsigned int num, unsigned int *p_keyslot)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kt_handle = MT_INVALID_HANDLE;
    unsigned int i;

    if (mt_mpi_kt_open(&kt_handle)) {
        return MT_CIPHER_ERR_KT_REQUEST_FAILED;
    }

    ret = mt_mpi_kt_slot_request_multi(kt_handle, num, p_keyslot);

    mt_mpi_kt_close(kt_handle);

    if (ret == SUCCESS) {
        for (i = 0; i < num; i++) {
            if (p_keyslot[i] == MT_CIPHER_KEYSLOT_INVALID)
                return MT_CIPHER_ERR_KT_REQUEST_FAILED;
        }
    }

    return ret;
}

int mt_unf_cipher_keyslot_release(unsigned int keyslot)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kt_handle = MT_INVALID_HANDLE;

    if (mt_mpi_kt_open(&kt_handle)) {
        return MT_CIPHER_ERR_KT_RELEASE_FAILED;
    }

    ret = mt_mpi_kt_slot_active(kt_handle, keyslot, MT_KT_SLOT_DO_INACTIVE);
    ret |= mt_mpi_kt_slot_release(kt_handle, keyslot);
    if (ret != MT_SUCCESS)
        ret = MT_CIPHER_ERR_KT_RELEASE_FAILED;

    mt_mpi_kt_close(kt_handle);

    return ret;
}

/*
 * Parameter input format:
 *  p_key == NULL; p_iv != NULL ==> set IV only, for ivsize, specify ALG in p_ctrl
 *  p_ctrl & p_key != NULL; p_iv == NULL ==> set key, no IV 
 * 
 */
int mt_unf_cipher_keyslot_set(unsigned int keyslot, MT_CIPHER_CTRL_S *p_ctrl, unsigned char *p_key, unsigned char *p_iv)
{
    mt_s32 ret = MT_SUCCESS;
    MT_KT_KEY_ATTR_S attr = { 0 };
    mt_handle kt_handle = MT_INVALID_HANDLE;

    if (keyslot == MT_CIPHER_KEYSLOT_INVALID) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if ((p_ctrl == NULL) || (p_key == NULL && p_iv == NULL)) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    ret = mt_mpi_kt_attr_config(p_ctrl, &attr);
    if (ret != MT_SUCCESS)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (mt_mpi_kt_open(&kt_handle)) {
        return MT_CIPHER_ERR_KT_SET_ERR;
    }

    if (p_key) {
        ret = mt_mpi_kt_slot_active(kt_handle, keyslot, MT_KT_SLOT_DO_INACTIVE);

        ret = mt_mpi_kt_write_key(kt_handle, keyslot, p_key, attr.KEY_SIZE);
        if (ret != MT_SUCCESS) {
            goto_fail(set);
        }

        ret = mt_mpi_kt_write_attr(kt_handle, keyslot, attr);
        if (ret != MT_SUCCESS) {
            goto_fail(set);
        }

        ret = mt_mpi_kt_slot_active(kt_handle, keyslot, MT_KT_SLOT_DO_ACTIVE);
    }

    if (p_iv != NULL) {
        ret = mt_mpi_kt_write_iv(kt_handle, keyslot, p_iv, attr.IV_SIZE);
        if (ret != MT_SUCCESS) {
            goto_fail(set);
        }
    }

    mt_mpi_kt_close(kt_handle);

_set_failed:
    return ret;
}

int mt_unf_cipher_keyslot_set_ext(unsigned int keyslot, MT_KT_CTRL_S *p_ctrl, unsigned char *p_key, unsigned char *p_iv)
{
    mt_s32 ret = MT_SUCCESS;
    MT_KT_KEY_ATTR_S attr = { 0 };
    mt_handle kt_handle = MT_INVALID_HANDLE;

    if (keyslot == MT_CIPHER_KEYSLOT_INVALID) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if ((p_ctrl == NULL) || (p_key == NULL && p_iv == NULL)) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    ret = mt_mpi_kt_attr_config_ext(p_ctrl, &attr);
    if (ret != MT_SUCCESS)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (mt_mpi_kt_open(&kt_handle)) {
        return MT_CIPHER_ERR_KT_SET_ERR;
    }

    if (p_key) {
        ret = mt_mpi_kt_slot_active(kt_handle, keyslot, MT_KT_SLOT_DO_INACTIVE);

        ret = mt_mpi_kt_write_key(kt_handle, keyslot, p_key, attr.KEY_SIZE);
        if (ret != MT_SUCCESS) {
            goto_fail(set);
        }

        ret = mt_mpi_kt_write_attr(kt_handle, keyslot, attr);
        if (ret != MT_SUCCESS) {
            goto_fail(set);
        }

        ret = mt_mpi_kt_slot_active(kt_handle, keyslot, MT_KT_SLOT_DO_ACTIVE);
    }

    if (p_iv != NULL) {
        ret = mt_mpi_kt_write_iv(kt_handle, keyslot, p_iv, attr.IV_SIZE);
        if (ret != MT_SUCCESS) {
            goto_fail(set);
        }
    }

    mt_mpi_kt_close(kt_handle);

_set_failed:
    return ret;
}

int mt_unf_cipher_keyslot_set_iv(unsigned int keyslot, unsigned char *p_iv, unsigned int ivsize)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kt_handle = MT_INVALID_HANDLE;

    if (mt_mpi_kt_open(&kt_handle)) {
        return MT_CIPHER_ERR_KT_SET_ERR;
    }

    if (keyslot == MT_CIPHER_KEYSLOT_INVALID)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (p_iv == NULL)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (ivsize != MT_KT_SLOT_8B_SIZE && ivsize != MT_KT_SLOT_16B_SIZE)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    ret = mt_mpi_kt_write_iv(kt_handle, keyslot, p_iv, ivsize);
    if (ret != MT_SUCCESS) {
        goto_fail(set);
    }

_set_failed:
    mt_mpi_kt_close(kt_handle);

    return ret;
}

int mt_unf_cipher_keyslot_get_iv(unsigned int keyslot, unsigned char *p_iv, unsigned int ivsize)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kt_handle = MT_INVALID_HANDLE;

    if (mt_mpi_kt_open(&kt_handle)) {
        return MT_CIPHER_ERR_KT_SET_ERR;
    }

    if (keyslot == MT_CIPHER_KEYSLOT_INVALID)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (p_iv == NULL)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (ivsize != MT_KT_SLOT_8B_SIZE && ivsize != MT_KT_SLOT_16B_SIZE)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    ret = mt_mpi_kt_read_iv(kt_handle, keyslot, p_iv, ivsize);
    if (ret != MT_SUCCESS) {
        goto_fail(set);
    }

_set_failed:
    mt_mpi_kt_close(kt_handle);
    return ret;
}

int mt_unf_cipher_keyslot_get_metadata(unsigned int keyslot, unsigned int *p_metadata)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kt_handle = MT_INVALID_HANDLE;

    if (keyslot == MT_CIPHER_KEYSLOT_INVALID)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (!p_metadata)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (mt_mpi_kt_open(&kt_handle)) {
        return MT_CIPHER_ERR_KT_RELEASE_FAILED;
    }

    ret = mt_mpi_kt_read_metadata(kt_handle, keyslot, p_metadata);

    mt_mpi_kt_close(kt_handle);
    return ret;
}

int mt_unf_cipher_keyslot_get_state(unsigned int keyslot, MT_CIPHER_KT_STATE_E *p_state)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kt_handle = MT_INVALID_HANDLE;
    MT_KT_SLOT_STATE_E status;

    if (keyslot == MT_CIPHER_KEYSLOT_INVALID)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (p_state == NULL)
	return MT_CIPHER_ERR_BAD_PARAMETERS;

    if (mt_mpi_kt_open(&kt_handle)) {
        return MT_CIPHER_ERR_KT_RELEASE_FAILED;
    }

    *p_state = MT_CIPHER_KT_UNINIT;
    ret = mt_mpi_kt_get_state(kt_handle, keyslot, &status);
    if (ret == MT_SUCCESS) {
        *p_state = (MT_CIPHER_KT_STATE_E)status;
    }

    mt_mpi_kt_close(kt_handle);
    return ret;
}

int mt_unf_cipher_keyslot_info(unsigned int keyslot)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kt_handle = MT_INVALID_HANDLE;

    if (mt_mpi_kt_open(&kt_handle)) {
        return MT_CIPHER_ERR_KT_RELEASE_FAILED;
    }

    ret = mt_mpi_kt_slot_info(kt_handle, keyslot);
    if (ret != MT_SUCCESS)
        ret = MT_CIPHER_ERR_KT_RELEASE_FAILED;

    mt_mpi_kt_close(kt_handle);

    return ret;
}

int mt_unf_cipher_keyslot_control_status(void)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kt_handle = MT_INVALID_HANDLE;

    if (mt_mpi_kt_open(&kt_handle)) {
        return MT_CIPHER_ERR_KT_RELEASE_FAILED;
    }

    ret = mt_mpi_kt_control_status(kt_handle);

    mt_mpi_kt_close(kt_handle);

    return ret;
}


/******************************************************************************
 * KeyLadder Functions
 *****************************************************************************/
int mt_unf_cipher_keyladder_create(MT_CIPHER_KEYLADDER_TYPE_E index, mt_handle *p_keyladder)
{
    mt_s32 ret = MT_SUCCESS;
    MT_CIPHER_KEYLADDER_TYPE_E kl_type = index;
    mt_handle kl_handle = MT_INVALID_HANDLE;

    if (index != MT_CIPHER_KEYLADDER_0 && index != MT_CIPHER_KEYLADDER_1) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (p_keyladder == NULL) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    ret = mt_mpi_kl_open(kl_type, &kl_handle);
    if (ret != MT_SUCCESS) {
        goto_fail(kl_create);
    }

    *p_keyladder = kl_handle;

_kl_create_failed:

    return ret;
}

int mt_unf_cipher_keyladder_destroy(mt_handle keyladder)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kl_handle = keyladder;

    if (kl_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(kl_destroy);
    }

    /* Timeout blocking, in terms of 20ms */
    mt_mpi_kl_release_sem(kl_handle);

    ret = mt_mpi_kl_close(kl_handle);

_kl_destroy_failed:
    return ret;
}

//Should be called after keyladder_start
//set_signature is CA vendor dependant
int mt_unf_cipher_keyladder_set_signature(mt_handle keyladder, unsigned char *p_sig)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kl_handle = keyladder;

    if (p_sig == NULL) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (kl_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(signature);
    }

    ret = mt_mpi_kl_set_signature(kl_handle, p_sig);
    if (ret != MT_SUCCESS) {
        mt_mpi_kl_unlock(kl_handle);
    }

_signature_failed:
    return ret;
}

int mt_unf_cipher_keyladder_start(mt_handle keyladder, MT_CIPHER_KEYLADDER_SOURCE_E keyladder_source)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kl_handle = keyladder;

    if (kl_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(kl_start);
    }

    if (keyladder_source >= MT_CIPHER_KEYLADDER_SCK_UNKNOWN) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        goto_fail(kl_start);
    }

    ret = mt_mpi_kl_lock(kl_handle);
    if (ret != MT_SUCCESS) {
        goto_fail(kl_start);
    }

    ret = mt_mpi_kl_request_sem(kl_handle);
    if (ret != MT_SUCCESS) {
        mt_mpi_kl_unlock(kl_handle);
        goto_fail(kl_start);
    }

    ret = mt_mpi_kl_select_rootkey(kl_handle, keyladder_source);
    if (ret != MT_SUCCESS) {
        mt_mpi_kl_unlock(kl_handle);
    }

_kl_start_failed:

    return ret;
}

int mt_unf_cipher_keyladder_link(mt_handle keyladder, MT_CIPHER_CTRL_S *p_ctrl, unsigned char *input, unsigned int length)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kl_handle = keyladder;

    if (kl_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(kl_link);
    }

    if ((p_ctrl == NULL) || (input == NULL) || (length != 16)) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        goto_fail(kl_link);
    }

    if (p_ctrl->algorithm == MT_CIPHER_ALG_TDES) {
        ret = mt_mpi_kl_link_tdes(kl_handle, input, (KL_MOVE_ENC_E)p_ctrl->operation);
    } else {
        ret = mt_mpi_kl_link_aes(kl_handle, input, (KL_MOVE_ENC_E)p_ctrl->operation);
    }

_kl_link_failed:
    if (ret != MT_SUCCESS && ret != MT_CIPHER_ERR_INVALID_HANDLE) {
        mt_mpi_kl_unlock(kl_handle);
    }

    return ret;
}

int mt_unf_cipher_keyladder_end(mt_handle keyladder, unsigned int key_slot)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kl_handle = keyladder;
    mt_s32 kl_err_code;

    if (kl_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(kl_end);
    }

    //export to keytable
    ret = mt_mpi_kl_export_key(kl_handle, KL_EXPORT_DST_KT, key_slot);
    if (ret != MT_SUCCESS) {
        goto_fail(kl_end);
    }

    ret = mt_mpi_kl_wait_complete(kl_handle, &kl_err_code);
    if (kl_err_code != 0) {
        ret = MT_FAILURE;
    }

_kl_end_failed:
    if (ret != MT_CIPHER_ERR_INVALID_HANDLE) {
        //must unlock
        mt_mpi_kl_unlock(kl_handle);
    }

    return ret;
}

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
int mt_unf_cipher_keyladder_extr_tdc(mt_handle keyladder, unsigned char *tdc_data)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kl_handle = keyladder;
	
    if (kl_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(kl_extr_tdc);
    }

    /* use TASK to decrypt cipher_TDC, and HW will verify the hash of TDC, get TDC if verified pass */
    ret = mt_mpi_kl_extr_tdc(kl_handle, tdc_data);

_kl_extr_tdc_failed:
    if ((ret != MT_SUCCESS) && (ret != MT_CIPHER_ERR_INVALID_HANDLE)) {
        mt_mpi_kl_unlock(kl_handle);
    }

    return ret;	
}

int mt_unf_cipher_keyladder_run_tdc(mt_handle keyladder, unsigned char *inupt, MT_CIPHER_CTRL_S *enc_ctrl)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kl_handle = keyladder;
    KL_MOVE_ALGO_E invt_algo = KL_MOVE_ALGO_TDES;
	
    if ((kl_handle == MT_INVALID_HANDLE)) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(kl_run_tdc);
    }
    if ((inupt == NULL) || (enc_ctrl == NULL)) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        goto_fail(kl_run_tdc);
    }

    if (enc_ctrl->algorithm == MT_CIPHER_ALG_TDES) {
        invt_algo = KL_MOVE_ALGO_TDES;
    } else if (enc_ctrl->algorithm == MT_CIPHER_ALG_AES) {
        invt_algo = KL_MOVE_ALGO_AES;
    }

    /* use Invt algo to generate cipher_CW, use CWSK to decrypt cipher_CW, get CW */
    ret = mt_mpi_kl_run_tdc(kl_handle, inupt, invt_algo, KL_MOVE_ADDs_DEC);

_kl_run_tdc_failed:
    if ((ret != MT_SUCCESS) && (ret != MT_CIPHER_ERR_INVALID_HANDLE)) {
        mt_mpi_kl_unlock(kl_handle);
    }

    return ret;	
}

int mt_unf_cipher_keyladder_store(mt_handle keyladder, MT_CIPHER_KEYLADDER_STORE_E store_dst)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kl_handle = keyladder;

    if ((kl_handle == MT_INVALID_HANDLE)) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(kl_store);
    }

    ret = mt_mpi_kl_storeKey(kl_handle, store_dst);

_kl_store_failed:
    if ((ret != MT_SUCCESS) && (ret != MT_CIPHER_ERR_INVALID_HANDLE)) {
        mt_mpi_kl_unlock(kl_handle);
    }

    return ret; 
}

int mt_unf_cipher_keyladder_kdf_seedv(mt_handle keyladder, unsigned char *vendor_id, MT_CIPHER_STANDARD_PROFILE_S kdf_profile, MT_CIPHER_HARDWIRED_SOURCE_E hw_key)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kl_handle = keyladder;
    mt_u8 *kl_input = vendor_id;

    if (kl_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(kl_kdf);
    }

    if (kl_input == NULL) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        goto_fail(kl_kdf);
    }

    ret = mt_mpi_kl_LinkSeedv(kl_handle, kl_input, (KL_HARDWIRED_SOURCE_E)hw_key, kdf_profile);

_kl_kdf_failed:
    if (ret != MT_SUCCESS && ret != MT_CIPHER_ERR_INVALID_HANDLE) {
        mt_mpi_kl_unlock(kl_handle);
    }

    return ret;
}

int mt_unf_cipher_keyladder_extra_infor(mt_handle keyladder, MT_CIPHER_KL_ADDITIONS_E additions, MT_CIPHER_ENABLE_E en)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle kl_handle = keyladder;

    if (kl_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(kl_extra);
    }

    if (additions >= MT_CIPHER_KL_ADDT_MAXNUM) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        goto_fail(kl_extra);
    }

    ret = mt_mpi_kl_extra(kl_handle, additions, en);

_kl_extra_failed:
    if (ret != MT_SUCCESS && ret != MT_CIPHER_ERR_INVALID_HANDLE) {
        mt_mpi_kl_unlock(kl_handle);
    }

    return ret;
}
#endif

/******************************************************************************
 * Crypto Functions
 *****************************************************************************/
int mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_E index, mt_handle *p_crypto)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle ce_handle = MT_INVALID_HANDLE;

    if (index >= MT_CIPHER_CRYPTO_CH_UNKNOWN || p_crypto == NULL) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_CRYPTO_CREATE;
    }

    ret = mt_mpi_crypto_create(index, ce_fd, &ce_handle);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_CREATE;
        goto_fail(crypto_create);
    }

    *p_crypto = ce_handle;

_crypto_create_failed:
    mt_mpi_crypto_close(ce_fd);

    return ret;
}

int mt_unf_cipher_crypto_config(mt_handle crypto, MT_CIPHER_CTRL_S *p_ctrl, unsigned int key_slot)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle ce_handle = crypto;

    if (ce_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(crypto_config);
    }

    if ((p_ctrl == NULL) || key_slot >= MT_CIPHER_KEYSLOT_INVALID) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        goto_fail(crypto_config);
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_CRYPTO_CONFIG;
    }

    ret = mt_mpi_crypto_config(ce_fd, ce_handle, p_ctrl, key_slot);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_CONFIG;
        goto_fail(crypto_config);
    }

_crypto_config_failed:
    mt_mpi_crypto_close(ce_fd);

    return ret;
}

int mt_unf_cipher_crypto_destroy(mt_handle crypto)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle ce_handle = crypto;

    if (ce_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(crypto_destroy);
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_FAILURE;
    }

    ret = mt_mpi_crypto_destroy(ce_fd, ce_handle);

_crypto_destroy_failed:
    mt_mpi_crypto_close(ce_fd);

    return ret;
}

int mt_unf_cipher_crypto_process(mt_handle crypto, unsigned char *p_src_addr, unsigned char *p_dest_addr, unsigned int length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle ce_handle = crypto;


    if (ce_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(crypto_process);
    }

    if (p_src_addr == NULL || p_dest_addr == NULL || length == 0) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        goto_fail(crypto_process);
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_CRYPTO_PROCESS;
    }

    ret = mt_mpi_crypto_process(ce_fd, ce_handle, p_src_addr, p_dest_addr, length);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_PROCESS;
        goto_fail(crypto_process);
    }

_crypto_process_failed:
    mt_mpi_crypto_close(ce_fd);
    return ret;
}

int mt_unf_cipher_crypto_process_phy(mt_handle crypto, phys_addr_t p_src_phyaddr, phys_addr_t p_dest_phyaddr, unsigned int length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle ce_handle = crypto;

    if (ce_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(crypto_process);
    }

    if (p_src_phyaddr == 0 || p_dest_phyaddr == 0 || length == 0) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        goto_fail(crypto_process);
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_CRYPTO_PROCESS;
    }

    ret = mt_mpi_crypto_process_phy(ce_fd, ce_handle, p_src_phyaddr, p_dest_phyaddr, 0, length);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_PROCESS;
        goto_fail(crypto_process);
    }

_crypto_process_failed:
    mt_mpi_crypto_close(ce_fd);
    return ret;
}

int mt_unf_cipher_crypto_process_phy_ext(mt_handle crypto, phys_addr_t p_src_phyaddr, phys_addr_t p_dest_phyaddr, unsigned int clear_length, unsigned int cipher_length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle ce_handle = crypto;

    if (ce_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(crypto_process);
    }

    if (p_src_phyaddr == 0 || p_dest_phyaddr == 0) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        goto_fail(crypto_process);
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_CRYPTO_PROCESS;
    }

    ret = mt_mpi_crypto_process_phy(ce_fd, ce_handle, p_src_phyaddr, p_dest_phyaddr, clear_length, cipher_length);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_PROCESS;
        goto_fail(crypto_process);
    }

_crypto_process_failed:
    mt_mpi_crypto_close(ce_fd);
    return ret;
}

mt_s32 mt_unf_cipher_crypto_async_request(mt_handle crypto, const mt_u8 *src, mt_u8 *dst, mt_u32 count, mt_u32 clear_length, mt_u32 protected_length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle ce_handle = crypto;

    if (ce_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(crypto_process);
    }

    if ((src == NULL) || (dst == NULL) || (count == 0) || ((clear_length == 0) && (protected_length == 0))) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        goto_fail(crypto_process);
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_CRYPTO_PROCESS;
    }

    ret = mt_mpi_cipher_crypto_async_request(ce_fd, ce_handle, src, dst, count, clear_length, protected_length);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_PROCESS;
        goto_fail(crypto_process);
    }

_crypto_process_failed:
    mt_mpi_crypto_close(ce_fd);
    return ret;
}

mt_s32 mt_unf_cipher_crypto_async_start(mt_handle crypto)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle ce_handle = crypto;

    if (ce_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(crypto_process);
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_CRYPTO_PROCESS;
    }

    ret = mt_mpi_cipher_crypto_async_start(ce_fd, ce_handle);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_PROCESS;
        goto_fail(crypto_process);
    }

_crypto_process_failed:
    mt_mpi_crypto_close(ce_fd);
    return ret;
}

mt_s32 mt_unf_cipher_crypto_async_wait(mt_handle crypto)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle ce_handle = crypto;

    if (ce_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(crypto_process);
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_CRYPTO_PROCESS;
    }

    ret = mt_mpi_cipher_crypto_async_wait(ce_fd, ce_handle);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_PROCESS;
        goto_fail(crypto_process);
    }

_crypto_process_failed:
    mt_mpi_crypto_close(ce_fd);
    return ret;
}

/******************************************************************************
 * TS Functions
 *****************************************************************************/
int mt_unf_cipher_ts_create(MT_CIPHER_CRYPTO_CH_E index, mt_handle *p_ts_handle)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle ts_handle = MT_INVALID_HANDLE;

    if (index >= MT_CIPHER_CRYPTO_CH_UNKNOWN || p_ts_handle == NULL) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_CRYPTO_CREATE;
    }

    ret = mt_mpi_crypto_ts_create(index, ce_fd, &ts_handle);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_CREATE;
        goto_fail(ts_create);
    }

    *p_ts_handle = ts_handle;

_ts_create_failed:
    mt_mpi_crypto_close(ce_fd);
    return ret;
}

int mt_unf_cipher_ts_destroy(mt_handle ts_handle)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle ce_handle = ts_handle;

    if (ce_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        goto_fail(ts_destroy);
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_FAILURE;
    }

    ret = mt_mpi_crypto_ts_destroy(ce_fd, ts_handle);

_ts_destroy_failed:
    mt_mpi_crypto_close(ce_fd);
    return ret;
}

int mt_unf_cipher_ts_config(mt_handle ts_handle, MT_CIPHER_CTRL_S *p_ctrl, MT_CIPHER_TS_PARA_S *p_ts_para, 
        unsigned int even_key_slot, unsigned int odd_key_slot)
{
    mt_s32 ret = MT_FAILURE;
    int ce_fd = -1;
    MT_CE_TS_CTRL_S ts_ctrl = {0};
    mt_handle ce_handle = ts_handle;

    if (ce_handle == MT_INVALID_HANDLE) {
        ret = MT_CIPHER_ERR_INVALID_HANDLE;
        return ret;
    }

    if ((p_ctrl == NULL) || p_ts_para == NULL 
            || even_key_slot >= MT_CIPHER_KEYSLOT_INVALID
            || odd_key_slot >= MT_CIPHER_KEYSLOT_INVALID) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        return ret;
    }

    if (p_ctrl->operation == MT_CIPHER_OPERATION_DECRYPT)
        ts_ctrl.operation = MT_CE_TS_OPERATION_DESCRAMBLE;
    else
        ts_ctrl.operation = MT_CE_TS_OPERATION_SCRAMBLE;

    ts_ctrl.ts_para.even_key_slot = even_key_slot;
    ts_ctrl.ts_para.odd_key_slot = odd_key_slot;

    switch (p_ctrl->algorithm) {
    case MT_CIPHER_ALG_DES:
        ts_ctrl.ts_para.algo_mode = MT_CE_TS_ALGO_MODE_DES;
    break;
    case MT_CIPHER_ALG_TDES:
        ts_ctrl.ts_para.algo_mode = MT_CE_TS_ALGO_MODE_TDES_ABA;
    break;
    case MT_CIPHER_ALG_AES:
        ts_ctrl.ts_para.algo_mode = MT_CE_TS_ALGO_MODE_AES128;
    break;
    default:
        //unkown or not support algo
        return MT_CIPHER_ERR_FEATURE_NOT_SUPPORT;
    break;
    }

    switch (p_ctrl->work_mode) {
    case MT_CIPHER_WORK_MODE_ECB:
        ts_ctrl.ts_para.work_mode = MT_CE_TS_WORK_MODE_ADES_ECB;
    break;
    case MT_CIPHER_WORK_MODE_CBC:
        ts_ctrl.ts_para.work_mode = MT_CE_TS_WORK_MODE_ADES_CBC;
    break;
    case MT_CIPHER_WORK_MODE_CBCDVS042:
        ts_ctrl.ts_para.work_mode = MT_CE_TS_WORK_MODE_ADES_CBCDVS042;
    break;
    case MT_CIPHER_WORK_MODE_CBCCTS:
        ts_ctrl.ts_para.work_mode = MT_CE_TS_WORK_MODE_ADES_CBCCTS;
    break;
    case MT_CIPHER_WORK_MODE_CTR:
        if (p_ctrl->algorithm != MT_CIPHER_ALG_AES)
            return MT_CIPHER_ERR_FEATURE_NOT_SUPPORT;

        ts_ctrl.ts_para.work_mode = MT_CE_TS_WORK_MODE_AES_CTR;
    break;
    case MT_CIPHER_WORK_MODE_RCBC:
        ts_ctrl.ts_para.work_mode = MT_CE_TS_WORK_MODE_ADES_RCBCCTS;
    break;
    case MT_CIPHER_WORK_MODE_ECBCTS:
        ts_ctrl.ts_para.work_mode = MT_CE_TS_WORK_MODE_ADES_ECBCTS;
    break;
    default:
        return MT_CIPHER_ERR_FEATURE_NOT_SUPPORT;
    break;
    }

    ts_ctrl.ts_para.ts_enc_ksel = p_ts_para->ts_enc_ksel;
    ts_ctrl.ts_para.ts_dec_ind = p_ts_para->ts_dec_ind;
    ts_ctrl.ts_para.ts_pkt_len = p_ts_para->ts_pkt_len;
    ts_ctrl.ts_para.ts_ive_mode = p_ts_para->ts_ive_mode;
    ts_ctrl.ts_para.ts_short_mode = p_ts_para->ts_short_mode;
    ts_ctrl.ts_para.ts_small_mode = p_ts_para->ts_small_mode;

    ts_ctrl.ts_para.ts_pid0_filt_en =  p_ts_para->ts_pid0_filt_en;
    ts_ctrl.ts_para.ts_pid1_filt_en =  p_ts_para->ts_pid1_filt_en;
    ts_ctrl.ts_para.ts_pid2_filt_en =  p_ts_para->ts_pid2_filt_en;
    ts_ctrl.ts_para.ts_pid3_filt_en =  p_ts_para->ts_pid3_filt_en;
    ts_ctrl.ts_para.ts_pid4_filt_en =  p_ts_para->ts_pid4_filt_en;
    ts_ctrl.ts_para.ts_pid5_filt_en =  p_ts_para->ts_pid5_filt_en;
    ts_ctrl.ts_para.ts_pid6_filt_en =  p_ts_para->ts_pid6_filt_en;
    ts_ctrl.ts_para.ts_pid7_filt_en =  p_ts_para->ts_pid7_filt_en;
    ts_ctrl.ts_para.ts_pid0_filt_num = p_ts_para->ts_pid0_filt_num;
    ts_ctrl.ts_para.ts_pid1_filt_num = p_ts_para->ts_pid1_filt_num;
    ts_ctrl.ts_para.ts_pid2_filt_num = p_ts_para->ts_pid2_filt_num;
    ts_ctrl.ts_para.ts_pid3_filt_num = p_ts_para->ts_pid3_filt_num;
    ts_ctrl.ts_para.ts_pid4_filt_num = p_ts_para->ts_pid4_filt_num;
    ts_ctrl.ts_para.ts_pid5_filt_num = p_ts_para->ts_pid5_filt_num;
    ts_ctrl.ts_para.ts_pid6_filt_num = p_ts_para->ts_pid6_filt_num;
    ts_ctrl.ts_para.ts_pid7_filt_num = p_ts_para->ts_pid7_filt_num;

    ts_ctrl.ts_para.ts_force_enc_en =  p_ts_para->ts_force_enc_en;

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_CRYPTO_CONFIG;
    }

    ret = mt_mpi_crypto_ts_config(ce_fd, ts_handle, &ts_ctrl);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_CONFIG;
        goto_fail(ts_config);
    }

_ts_config_failed:
    mt_mpi_crypto_close(ce_fd);

    return ret;
    
}

int mt_unf_cipher_ts_process(mt_handle ts_handle, phys_addr_t p_src_addr, phys_addr_t p_dest_addr, unsigned int length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle ce_handle = ts_handle;

    if (ce_handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (p_src_addr == 0 || p_dest_addr == 0 || length == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_CRYPTO_PROCESS;
    }

    ret = mt_mpi_crypto_ts_process(ce_fd, ce_handle, p_src_addr, p_dest_addr, length);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_CRYPTO_PROCESS;
        goto_fail(ts_process);
    }

_ts_process_failed:
    mt_mpi_crypto_close(ce_fd);

    return ret;
}

/******************************************************************************
 * Hash Functions
 *****************************************************************************/
int mt_unf_cipher_hash_create(MT_CIPHER_HASH_TYPE_E hash_type, MT_CIPHER_HMAC_ATTS_S *p_hmac_attr, mt_handle *p_hash)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle hash_handle = MT_INVALID_HANDLE;

    UNUSED(p_hmac_attr);

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_HASH_CREATE;
    }

    ret = mt_mpi_hash_create(hash_type, ce_fd, &hash_handle);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_HASH_CREATE;
        goto_fail(hash_create);
    }

    *p_hash = hash_handle;

_hash_create_failed:
    mt_mpi_crypto_close(ce_fd);

    return ret;
}

int mt_unf_cipher_hash_update(mt_handle hash, unsigned char *p_data, unsigned int length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle hash_handle = hash;

    if (p_data == NULL || length == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (hash_handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }
    
    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_HASH_UPDATE;
    }

    ret = mt_mpi_hash_update(ce_fd, hash_handle, p_data, length);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_HASH_UPDATE;
        goto_fail(hash_update);
    }

_hash_update_failed:
    mt_mpi_crypto_close(ce_fd);

    return ret;
}

int mt_unf_cipher_hash_final(mt_handle hash, unsigned char *p_output_hash)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle hash_handle = hash;

    if (hash_handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (p_output_hash == NULL) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_HASH_FINAL;
    }

    ret = mt_mpi_hash_final(ce_fd, hash_handle, p_output_hash);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_HASH_FINAL;
        goto_fail(hash_final);
    }

_hash_final_failed:
    mt_mpi_crypto_close(ce_fd);

    return ret;
}

/******************************************************************************
 * HMAC/CMAC/CBCMAC Functions
 *****************************************************************************/
int mt_unf_cipher_mac_create(MT_CIPHER_MAC_TYPE_E mac_type, MT_CIPHER_HMAC_ATTS_S *p_hmac_attr, mt_handle *p_mac)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle mac_handle = MT_INVALID_HANDLE;

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_MAC_CREATE;
    }

    if (!p_hmac_attr) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        goto_fail(mac_create);
    }

    if (p_hmac_attr->p_hmac_key == NULL
        && p_hmac_attr->key_slot[0] == MT_CIPHER_KEYSLOT_INVALID) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        goto_fail(mac_create);
    }

    ret = mt_mpi_mac_create(mac_type, p_hmac_attr, ce_fd, &mac_handle);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_MAC_CREATE;
        goto_fail(mac_create);
    }

    *p_mac = mac_handle;

_mac_create_failed:
    mt_mpi_crypto_close(ce_fd);

    return ret;
}

int mt_unf_cipher_mac_update(mt_handle mac, unsigned char *p_data, unsigned int length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle mac_handle = mac;

    if (p_data == NULL || length == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mac_handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }
    
    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_MAC_UPDATE;
    }

    if (mt_mpi_mac_update(ce_fd, mac_handle, p_data, length)) {
        ret = MT_CIPHER_ERR_MAC_UPDATE;
    }

    mt_mpi_crypto_close(ce_fd);

    return ret;
}

int mt_unf_cipher_mac_final(mt_handle mac, unsigned char *p_output_mac)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle mac_handle = mac;

    if (mac_handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (p_output_mac == NULL) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_MAC_FINAL;
    }

    if (mt_mpi_mac_final(ce_fd, mac_handle, p_output_mac)) {
        ret = MT_CIPHER_ERR_MAC_FINAL;
    }

    mt_mpi_crypto_close(ce_fd);

    return ret;
}

/******************************************************************************
 * RSA Functions
 *****************************************************************************/
int mt_unf_cipher_rsa_create(mt_handle *p_rsa_handle)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;
    mt_handle rsa_handle = MT_INVALID_HANDLE;

    if (p_rsa_handle == NULL) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_RSA_CREATE;
    }

    ret = mt_mpi_crypto_rsa_create(ce_fd, &rsa_handle);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_RSA_CREATE;
        goto_fail(rsa_create);
    }

    *p_rsa_handle = rsa_handle;

_rsa_create_failed:
    mt_mpi_crypto_close(ce_fd);

    return ret;
}

int mt_unf_cipher_rsa_destroy(mt_handle rsa_handle)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (rsa_handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_RSA_CREATE;
    }
    ret = mt_mpi_crypto_rsa_destroy(ce_fd, rsa_handle);

    mt_mpi_crypto_close(ce_fd);

    return ret;
}

int mt_unf_cipher_rsa_config(mt_handle rsa_handle, MT_CIPHER_RSA_CTRL_S *p_ctrl)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (rsa_handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_RSA_CONFIG;
    }
    ret = mt_mpi_crypto_rsa_config(ce_fd, rsa_handle, p_ctrl);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_RSA_CONFIG;
        goto_fail(rsa_config);
    }

_rsa_config_failed:
    mt_mpi_crypto_close(ce_fd);

    return ret;
}

int mt_unf_cipher_rsa_process(mt_handle rsa_handle, unsigned char *p_src_addr, unsigned char *p_dst_addr, unsigned int length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (rsa_handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (p_src_addr == NULL || p_dst_addr == NULL || length == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_RSA_PROCESS;
    }
    ret = mt_mpi_crypto_rsa_process(ce_fd, rsa_handle, p_src_addr, p_dst_addr, length);
    if (ret != MT_SUCCESS) {
        if (ret == CE_NOT_SUPPORT)
            ret = MT_CIPHER_ERR_FEATURE_NOT_SUPPORT;
        else
            ret = MT_CIPHER_ERR_RSA_PROCESS;
        goto_fail(rsa_process);
    }

_rsa_process_failed:
    mt_mpi_crypto_close(ce_fd);

    return ret;
}

/******************************************************************************
 * BN Functions
 *****************************************************************************/
/*!
  Obtain a bn handle

  \param[out] (p_bn_handle) create bn handle
  
  \return SUCCESS, else fail
*/
int mt_unf_cipher_bn_create(mt_handle *p_bn_handle)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (p_bn_handle == NULL) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_BN_CREATE;
    }

    ret = mt_mpi_crypto_bn_create(ce_fd, p_bn_handle);

    mt_mpi_crypto_close(ce_fd);
    return ret;
}

/*!
  Destroy the existing cipher handle
  
  \param[in] (handle) the bn handle
  
  \return SUCCESS, else fail
  */
int mt_unf_cipher_bn_destroy(mt_handle handle)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_BN_DESTROY;
    }

    ret = mt_mpi_crypto_bn_destroy(ce_fd, handle);

    mt_mpi_crypto_close(ce_fd);
    return ret;
}

/**
 * @brief modular
 *
 * @param[in] handle:
 * @param[out] r:   remainder 
 * @param[in]  d:   in data  
 * @param[in]  m:   modulo
 * @param[in]  d_length:    data length
 * @param[in]  m_length:    modulo length
 *
 * @return SUCCESS, else fail
 */
int mt_unf_cipher_bn_mod_mod(mt_handle handle, unsigned char *r, unsigned char *d, unsigned char *m, unsigned int d_length, unsigned int m_length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (d == NULL || r == NULL || m == NULL || d_length == 0 || m_length == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_BN_MODMOD;
    }

    ret = mt_mpi_crypto_bn_mod_mod(ce_fd, handle, r, d, m, d_length, m_length);

    mt_mpi_crypto_close(ce_fd);
    return ret;
}

/**
 * @brief modular multiply
 *
 * @param[in] handle:
 * @param[out] r:   remainder 
 * @param[in]  a:   input a
 * @param[in]  b:   input b
 * @param[in]  m:   modulo
 * @param[in]  a_length:    data length
 * @param[in]  b_length:    data length
 * @param[in]  m_length:    modulo length
 *
 * @return SUCCESS, else fail
 */
int mt_unf_cipher_bn_mod_mul(mt_handle handle, unsigned char *r, unsigned char *a, unsigned char *b, unsigned char *m, unsigned int a_length, unsigned int b_length, unsigned int m_length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (r == NULL || a == NULL || b == NULL || m == NULL || a_length == 0 || b_length == 0 || m_length == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_BN_MODMUL;
    }

    ret = mt_mpi_crypto_bn_mod_mul(ce_fd, handle, r, a, b, m, a_length, b_length, m_length);

    mt_mpi_crypto_close(ce_fd);
    return ret;
}

/**
 * @brief modular addition
 *
 * @param[in] handle:
 * @param[out] r:   remainder 
 * @param[in]  a:   input a
 * @param[in]  b:   input b
 * @param[in]  m:   modulo
 * @param[in]  a_length:    data length
 * @param[in]  b_length:    data length
 * @param[in]  m_length:    modulo length
 *
 * @return SUCCESS, else fail
 */
int mt_unf_cipher_bn_mod_add(mt_handle handle, unsigned char *r, unsigned char *a, unsigned char *b, unsigned char *m, unsigned int a_length, unsigned int b_length, unsigned int m_length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (r == NULL || a == NULL || b == NULL || m == NULL || a_length == 0 || b_length == 0 || m_length == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_BN_MODADD;
    }

    ret = mt_mpi_crypto_bn_mod_add(ce_fd, handle, r, a, b, m, a_length, b_length, m_length);

    mt_mpi_crypto_close(ce_fd);
    return ret;
}
/**
 * @brief modular subtraction
 *
 * @param[in] handle:
 * @param[out] r:   remainder 
 * @param[in]  a:   input a
 * @param[in]  b:   input b
 * @param[in]  m:   modulo
 * @param[in]  a_length:    data length
 * @param[in]  b_length:    data length
 * @param[in]  m_length:    modulo length
 *
 * @return SUCCESS, else fail
 */
int mt_unf_cipher_bn_mod_sub(mt_handle handle, unsigned char *r, unsigned char *a, unsigned char *b, unsigned char *m, unsigned int a_length, unsigned int b_length, unsigned int m_length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (r == NULL || a == NULL || b == NULL || m == NULL || a_length == 0 || b_length == 0 || m_length == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_BN_MODSUB;
    }
    ret = mt_mpi_crypto_bn_mod_sub(ce_fd, handle, r, a, b, m, a_length, b_length, m_length);

    mt_mpi_crypto_close(ce_fd);

    return ret;
}

/**
 * @brief modular inverse
 *
 * @param[in] handle:
 * @param[out] r:   remainder 
 * @param[in]  d:   input data
 * @param[in]  m:   modulo
 * @param[in]  d_length:    data length
 * @param[in]  m_length:    modulo length
 *
 * @return SUCCESS, else fail
 */
int mt_unf_cipher_bn_mod_inv(mt_handle handle, unsigned char *r, unsigned char *d, unsigned char *m, unsigned int d_length, unsigned int m_length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (r == NULL || d == NULL || m == NULL || d_length == 0 || m_length == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (m_length >= 256) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_BN_MODINV;
    }

    ret = mt_mpi_crypto_bn_mod_inv(ce_fd, handle, r, d, m, d_length, m_length);

    mt_mpi_crypto_close(ce_fd);
    return ret;
}

/**
 * @brief modular exponentiation
 *
 * @param[in] handle:
 * @param[out] r:   remainder 
 * @param[in]  a:   input data
 * @param[in]  p:   input data
 * @param[in]  m:   modulo
 * @param[in]  a_length:    data length
 * @param[in]  p_length:    data length
 * @param[in]  m_length:    modulo length
 *
 * @return SUCCESS, else fail
 */
int mt_unf_cipher_bn_mod_exp(mt_handle handle, unsigned char *r, unsigned char *a, unsigned char *p, unsigned char *m, unsigned int a_length, unsigned int p_length, unsigned int m_length)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (r == NULL || a == NULL || p == NULL || m == NULL || a_length == 0 || p_length == 0 || m_length == 0) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_BN_MODEXP;
    }

    ret = mt_mpi_crypto_bn_mod_exp(ce_fd, handle, r, a, p, m, a_length, p_length, m_length);

    mt_mpi_crypto_close(ce_fd);
    return ret;
}

/******************************************************************************
 * ECPoint Functions
 *****************************************************************************/
int mt_unf_cipher_ecp_create(mt_handle *p_handle)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (p_handle == NULL) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_ECP_CREATE;
    }

    ret = mt_mpi_crypto_ec_point_create(ce_fd, p_handle);

    mt_mpi_crypto_close(ce_fd);
    return ret;
}

int mt_unf_cipher_ecp_destroy(mt_handle handle)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_ECP_DESTROY;
    }
    ret = mt_mpi_crypto_ec_point_destroy(ce_fd, handle);

    mt_mpi_crypto_close(ce_fd);
    return ret;
}

int mt_unf_cipher_ecp_mul(
    mt_handle handle,
    MT_CIPHER_EC_PARAMS_S xParams,
    MT_CIPHER_EC_POINT_S *r,
    const unsigned char *g_scalar,
    MT_CIPHER_EC_POINT_S *p_point,
    const unsigned char *p_scalar)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (r == NULL || (g_scalar == NULL && p_scalar == NULL)) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_ECP_MUL;
    }
    ret = mt_mpi_crypto_ec_point_mul(ce_fd, handle, xParams, r, g_scalar, p_point, p_scalar);

    mt_mpi_crypto_close(ce_fd);
    return ret;
}

int mt_unf_cipher_ecp_add(
    mt_handle handle,
    MT_CIPHER_EC_PARAMS_S xParams,
    MT_CIPHER_EC_POINT_S *r,
    const MT_CIPHER_EC_POINT_S *a,
    const MT_CIPHER_EC_POINT_S *b)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (handle == MT_INVALID_HANDLE) {
        return MT_CIPHER_ERR_INVALID_HANDLE;
    }

    if (r == NULL || a == NULL || b == NULL) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_ECP_ADD;
    }
    ret = mt_mpi_crypto_ec_point_add(ce_fd, handle, xParams, r, a, b);

    mt_mpi_crypto_close(ce_fd);
    return ret;
}

/******************************************************************************
 * MISC Functions
 *****************************************************************************/
unsigned char *mt_unf_cipher_malloc(unsigned int length)
{
    return (unsigned char *)mt_mpi_cipher_malloc(length);
}

int mt_unf_cipher_free(unsigned char *p)
{
    return mt_mpi_cipher_free((mt_void *)p);
}

int mt_unf_cipher_get_random_number(unsigned int bytes_to_get, unsigned char *p_random_number)
{
    mt_s32 ret = MT_SUCCESS;

    if (bytes_to_get == 0 || p_random_number == NULL) {
        ret = MT_CIPHER_ERR_BAD_PARAMETERS;
        goto_fail(random_number);
    }

    ret = mt_mpi_cipher_get_random_number(bytes_to_get, p_random_number);

_random_number_failed:

    return ret;
}

/******************************************************************************
 * Background check Functions
 *****************************************************************************/
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 mt_unf_cipher_bgc_request(mt_u32 *bgc_slot, \
            MT_CIPHER_BGC_SEM_REQ_S time_out, const mt_u8 *bgc_addr, mt_u32 size, \
            MT_CIPHER_BGC_DELAY delay, mt_u8 *golden_hash, MT_CIPHER_BGC_LOCK_S lock)
{
    mt_s32 ret = MT_FAILURE;
    int ce_fd = -1;
    mt_handle bgc_slot_req;

    if (bgc_slot == NULL) {
        return MT_CIPHER_ERR_BAD_PARAMETERS;
    }

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_BGC_REQUEST;
    }

    ret = mt_mpi_cipher_bgc_request(ce_fd, &bgc_slot_req, time_out);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_BGC_REQUEST;
        goto_fail(bgc_request);
    }

    ret = mt_mpi_cipher_bgc_setup(ce_fd, bgc_slot_req, bgc_addr, size, delay, golden_hash, lock);
    if (ret == MT_SUCCESS) {
        *bgc_slot = bgc_slot_req;
    } else {
        ret = MT_CIPHER_ERR_BGC_REQUEST;
    }
    
_bgc_request_failed:
    mt_mpi_crypto_close(ce_fd);
    return ret;
}

mt_s32 mt_unf_cipher_bgc_release(mt_handle bgc_slot, MT_CIPHER_BGC_SEM_REQ_S time_out)
{
    mt_s32 ret = MT_SUCCESS;
    int ce_fd = -1;

    if (mt_mpi_crypto_open(&ce_fd)) {
        return MT_CIPHER_ERR_BGC_RELEASE;
    }

    ret = mt_mpi_cipher_bgc_release(ce_fd, bgc_slot, time_out);
    if (ret != MT_SUCCESS) {
        ret = MT_CIPHER_ERR_BGC_RELEASE;
    }

    mt_mpi_crypto_close(ce_fd);
    return ret;
}
#endif
