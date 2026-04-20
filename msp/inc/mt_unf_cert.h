/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_CERT_H__
#define __MT_UNF_CERT_H__
#include "mt_type.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum {
		CERT_TO_DEFAULT,
		CERT_TO_OTP,
		LAST_CERT_TO
	} CERT_TO_TYPE_E;

	typedef struct _CERT_COMMAND_STRUCT {
		mt_u8 input[32];
		mt_u8 output[32];
		mt_u8 status[4];
		mt_u8 opcodes[4];
		CERT_TO_TYPE_E timeout;
	} CERT_COMMAND_S;

	mt_s32 mt_unf_cert_open(mt_handle * p_handle);
	mt_s32 mt_unf_cert_lock(mt_handle handle);
	mt_s32 mt_unf_cert_unlock(mt_handle handle);
	mt_s32 mt_unf_cert_close(mt_handle handle);
	mt_s32 mt_unf_cert_exchange(mt_handle handle, mt_u32 cmds_num,
				    const CERT_COMMAND_S * p_cmds,
				    mt_u32 * p_processed_num);
	mt_s32 mt_unf_cert_export_key(mt_handle handle, mt_u32 slot_id,
				      mt_u32 ext_attr);
	mt_s32 mt_unf_cert_key_ack(mt_handle handle);
	mt_s32 mt_unf_cert_reset(mt_handle handle);

#ifdef __cplusplus
}
#endif
#endif				//__MT_UNF_CERT_H__
