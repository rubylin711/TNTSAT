/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2014                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
* File:				mt_fe_dmd_ct8k_T.h
*
* Current version:	00.40
*
* Description:
*
* Log:	Description		Version		Date			Author
---------------------------------------------------------------------
*		Create			00.00		2014.01.18		BJ.Wang
*		Modify			00.10		2014.05.26		BJ.Wang
*		Modify			00.20		2014.08.06		BJ.Wang
*		Modify			00.40		2017.01.18		BJ.Wang
****************************************************************************************************/
#ifndef __MT_FE_DEF_CT8K_T_H__
#define __MT_FE_DEF_CT8K_T_H__

//#include <stdio.h>
#include "mt_fe_def.h"

#ifdef __cplusplus
extern "C" {
#endif

MT_FE_RET mt_fe_dmd_get_driver_version_ct8k_t(U8* p_version);
MT_FE_RET _mt_fe_dmd_download_fw_ct8k_t(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_bw_ct8k_t(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_init_ct8k_t(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_demod_ct8k_t(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_connect_ct8k_t(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_hard_reset_ct8k_t(void);
MT_FE_RET _mt_fe_dmd_soft_reset_ct8k_t(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_output_mode_ct8k_t(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_get_lock_state_ct8k_t(MT_FE_CT8K_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_get_ber_ct8k_t(MT_FE_CT8K_Device_Handle handle, U32 *p_err_cnt, U32 *p_total_cnt);
MT_FE_RET _mt_fe_dmd_get_snr_ct8k_t(MT_FE_CT8K_Device_Handle handle, U16 *p_snr);
MT_FE_RET _mt_fe_dmd_get_quality_ct8k_t(MT_FE_CT8K_Device_Handle handle,U8 *quality);
MT_FE_RET _mt_fe_dmd_get_strength_ct8k_t(MT_FE_CT8K_Device_Handle handle, U8 *ssi_percent);
MT_FE_RET _mt_fe_dmd_get_quality_nordig_ct8k_t(MT_FE_CT8K_Device_Handle handle, U8 *quality);
MT_FE_RET _mt_fe_dmd_get_cell_info_ct8k_t(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_get_tps_info_ct8k_t(MT_FE_CT8K_Device_Handle handle, MT_FE_T_TPS_INFO *tps_info);


#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_DEF_CT8K_T_H__ */

