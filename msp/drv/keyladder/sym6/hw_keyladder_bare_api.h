/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HAL_KEY_LADDER_INTERFACE_H__
#define __HAL_KEY_LADDER_INTERFACE_H__

#include "hw_keyladder_register.h"
#include "mt_mpi_keyladder.h"

#ifdef __cplusplus
extern "C" {
#endif

int bare_kl_init(void **p_bhandle);

int bare_kl_deinit(void *bhandle);

int bare_kl_sem_request(void *bhandle, KL_SEM_CPU_ID cpu_id);

int bare_kl_sem_release(void *bhandle);

int bare_kl_set_signature(void *bhandle, unsigned char *p_signature);

int bare_kl_select_rootkey(void *bhandle, KL_SCK_SOURCE_E rootkey_src);

int bare_kl_link_aes(void *bhandle, unsigned char *p_input, KL_MOVE_ENC_E enc_type);

int bare_kl_link_tdes(void *bhandle, unsigned char *p_input, KL_MOVE_ENC_E enc_type);

int bare_kl_link_sm4(void *bhandle, unsigned char *p_input, KL_MOVE_ENC_E enc_type);
int bare_kl_link_xor(void *bhandle, unsigned char *p_input);

int bare_kl_link_hash(void *bhandle, unsigned char *p_input, KL_HASH_POSITION_E input_data_pos, KL_HASH_POSITION_E output_key_pos);

int bare_kl_link_seedv(void *bhandle, unsigned char *p_input, KL_HARDWIRED_SOURCE_E mask_key, KL_STANDARD_PROFILE_E profile);

int bare_kl_export_key(void *bhandle, KL_EXPORT_DST_E dst, unsigned long slot_id);

int bare_kl_store_key(void *bhandle, KL_STORE_DST_E store_dst);

int bare_kl_input_data(void *bhandle, KL_INPUT_POSITION_E postion, unsigned char *p_input);

int bare_kl_store_cmd(void *bhandle, KL_STORE_SRC_E store_src, KL_STORE_DST_E store_dst, KL_STORE_LOCK_E lock);

int bare_kl_store_cmd2(void *bhandle, KL_STORE_SRC_E store_src, KL_STORE_DST_E store_dst, KL_STORE_LOCK_E lock, unsigned char exec);

int bare_kl_move_cmd(void *bhandle, KL_MOVE_SRC_E move_src, KL_MOVE_DST_E move_dst, KL_MOVE_TRIGGER_E trigger, KL_MOVE_ENC_E enc_type);

int bare_kl_export_cmd(void *bhandle, KL_EXPORT_SOURCE_E key_src, KL_EXPORT_DST_E key_dst, mt_u32 slot_id);

int bare_kl_wait_complete(void *bhandle,  unsigned long *p_error);

int bare_kl_read_key(void *bhandle, unsigned char *p_key_buffer);

int bare_kl_store_tdc(void *bhandle, unsigned char *p_input);

int bare_kl_execute_tdc(void *bhandle, unsigned char *p_input, KL_MOVE_ALGO_E algo_type, KL_MOVE_ENC_E enc_type);

int bare_kl_addt_condition_op(KL_ADDITIONS_E addt, KL_FUNC_ENABLE_E en);

int bare_kl_execute_TDC_AES(void *bhandle, unsigned char *p_input);

int bare_kl_execute_TDC_TDES(void *bhandle, unsigned char *p_input);

int bare_kl_store_tdc(void *bhandle, unsigned char *p_input);

int kl_check_semaphore(void *bhandle);

int kl_check_error_state(void *bhandle);

int kl_set_input(kl_handle_t *handle, unsigned char *p_input);

void bare_kl_select_keypos(void *bhandle, KL_MOVE_SRC_E key_pos);

int bare_kl_read_key_from_keytable(unsigned int slot_id, unsigned char *p_key_buffer);

int bare_kl_ready_for_cpc(void *bhandle);

int bare_kl_set_signature_keysrc(void *bhandle, KL_SIG_CPU_E cpu, KL_SIG_KEYSRC_E keysrc);

int bare_kl_set_signature_lock_keysrc(void *bhandle, KL_SIG_CPU_E cpu, unsigned char lock);

#ifdef __cplusplus
}
#endif

#endif //__HAL_KEY_LADDER_INTERFACE_H__