/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_KEY_LADDER_H__
#define __DRV_KEY_LADDER_H__

#include "mt_mpi_keyladder.h"

mt_s32 DRV_KEYLADDER_Init(mt_void);

mt_s32 DRV_KEYLADDER_DeInit(mt_void);

mt_s32 DRV_KEYLADDER_Open(KEYLADDER_TYPE_E type, mt_handle *p_handle);

mt_s32 DRV_KEYLADDER_Close(mt_handle handle);

mt_s32 DRV_KEYLADDER_ReqSem(mt_handle handle);

mt_s32 DRV_KEYLADDER_RlsSem(mt_handle handle);

mt_s32 DRV_KEYLADDER_Lock(mt_handle handle);

mt_s32 DRV_KEYLADDER_Unlock(mt_handle handle);

mt_s32 DRV_KEYLADDER_SetSignature(mt_handle handle, mt_u8 *p_signature);

mt_s32 DRV_KEYLADDER_SelectRootkey(mt_handle handle, KL_SCK_SOURCE_E rootkey_source);

mt_s32 DRV_KEYLADDER_LinkAES(mt_handle handle, mt_u8 *p_input, KL_MOVE_ENC_E enc_type);

mt_s32 DRV_KEYLADDER_LinkTDES(mt_handle handle, mt_u8 *p_input, KL_MOVE_ENC_E enc_type);

mt_s32 DRV_KEYLADDER_LinkXOR(mt_handle handle, mt_u8 *p_input);

mt_s32 DRV_KEYLADDER_LinkHash(mt_handle handle, mt_u8 *p_input, KL_HASH_POSITION_E input_data_pos, KL_HASH_POSITION_E output_key_pos);

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 DRV_KEYLADDER_LinkSeedv(mt_handle handle, mt_u8 *p_input, KL_HARDWIRED_SOURCE_E mask_key, KL_STANDARD_PROFILE_E profile);
#else
mt_s32 DRV_KEYLADDER_LinkSeedv(mt_handle handle, KL_HARDWIRED_SOURCE_E mask_key, KL_STANDARD_PROFILE_E profile);
#endif

mt_s32 DRV_KEYLADDER_StoreKey(mt_handle handle, KL_STORE_DST_E store_dst);

mt_s32 DRV_KEYLADDER_ExportKey(mt_handle handle, KL_EXPORT_DST_E export_dst, mt_u32 slot_id);

mt_s32 DRV_KEYLADDER_InputData(mt_handle handle, KL_INPUT_POSITION_E postion, mt_u8 *p_input);

mt_s32 DRV_KEYLADDER_MoveCmd(mt_handle handle, KL_MOVE_SRC_E move_src, KL_MOVE_DST_E move_dst, KL_MOVE_TRIGGER_E trigger, KL_MOVE_ENC_E enc_type);

mt_s32 DRV_KEYLADDER_StoreCmd(mt_handle handle, KL_STORE_SRC_E store_src, KL_STORE_DST_E store_dst, KL_STORE_LOCK_E lock);

mt_s32 DRV_KEYLADDER_ExportCmd(mt_handle handle, KL_EXPORT_SOURCE_E key_src, KL_EXPORT_DST_E export_dst, mt_u32 slot_id);

mt_s32 DRV_KEYLADDER_WaitComplete(mt_handle handle, mt_u32 *p_error);

mt_s32 DRV_KEYLADDER_ReadKey(mt_handle handle, mt_u8 *key_buffer);

mt_s32 DRV_KEYLADDER_SetSigSrc(mt_handle handle, KL_SIG_CPU_E cpu, KL_SIG_KEYSRC_E keysrc);

mt_s32 DRV_KEYLADDER_LockSigSrc(mt_handle handle, KL_SIG_CPU_E cpu, unsigned char lock);

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 DRV_KEYLADDER_ExtrTDC(mt_handle handle, mt_u8 *p_input);

mt_s32 DRV_KEYLADDER_RunTDC(mt_handle handle, mt_u8 *p_input, KL_MOVE_ALGO_E algo_type, KL_MOVE_ENC_E enc_type);

mt_s32 DRV_KEYLADDER_Additions(mt_handle handle, KL_ADDITIONS_E addt, KL_FUNC_ENABLE_E en);
#endif

#endif  //__DRV_KEY_LADDER_H__

