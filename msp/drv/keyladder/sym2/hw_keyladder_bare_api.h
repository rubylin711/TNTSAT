/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __KEY_LADDER_BARE_API_H__
#define __KEY_LADDER_BARE_API_H__

#include "mt_mpi_keyladder.h"

#ifdef __cplusplus
extern "C" {
#endif

int bare_kl_init(void **p_bhandle, KEYLADDER_TYPE_E type);

int bare_kl_set_signature(void *bhandle, unsigned char *p_signature);

int bare_kl_select_rootkey(void *bhandle, KL_SCK_SOURCE_E rootkey_source);

int bare_kl_link_aes(void *bhandle, unsigned char *p_input);

int bare_kl_link_tdes(void *bhandle, unsigned char *p_input);

int bare_kl_link_xor(void *bhandle, unsigned char *p_input);

int bare_kl_link_hash(void *bhandle, unsigned char *p_input, KL_HASH_POSITION_E input_data_pos, KL_HASH_POSITION_E output_key_pos);

int bare_kl_link_seedv(void *bhandle, KL_HARDWIRED_SOURCE_E mask_key, KL_STANDARD_PROFILE_E profile);

int bare_kl_export_key(void *bhandle, KL_EXPORT_DST_E dst, unsigned long slot_id);

int bare_kl_store_key(void *bhandle, KL_STORE_DST_E store_dst);

int bare_kl_input_data(void *bhandle, KL_INPUT_POSITION_E postion, unsigned char *p_input);

int bare_kl_store_cmd(void *bhandle, KL_STORE_SRC_E store_src, KL_STORE_DST_E store_dst, KL_STORE_LOCK_E lock);

int bare_kl_move_cmd(void *bhandle, KL_MOVE_SRC_E move_src, KL_MOVE_DST_E move_dst, KL_MOVE_TRIGGER_E trigger);

int bare_kl_export_cmd(void *bhandle, KL_EXPORT_SOURCE_E key_src, KL_EXPORT_DST_E dst, unsigned long slot_id);

int bare_kl_wait_complete(void *bhandle,  unsigned long *p_error);

int bare_kl_read_key(void *bhandle, unsigned char *p_key_buffer);

#ifdef __cplusplus
}
#endif

#endif //__KEY_LADDER_H__
