/********************************************************************************************/
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
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
* File:				mt_fe_dmd_dm6k_T.h
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
#ifndef __MT_FE_DEF_DM6K_T_H__
#define __MT_FE_DEF_DM6K_T_H__

//#include <stdio.h>
#include "mt_fe_def.h"

#ifdef __cplusplus
extern "C" {
#endif

MT_FE_RET mt_fe_dmd_get_driver_version_dm6k_t(U8* p_version);
MT_FE_RET _mt_fe_dmd_download_fw_dm6k_t(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_bw_dm6k_t(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_init_dm6k_t(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_connect_dm6k_t(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_hard_reset_dm6k_t(void);
MT_FE_RET _mt_fe_dmd_soft_reset_dm6k_t(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_output_mode_dm6k_t(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_get_lock_state_dm6k_t(MT_FE_DM6K_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_get_ber_dm6k_t(MT_FE_DM6K_Device_Handle handle, U32 *p_err_cnt, U32 *p_total_cnt);
MT_FE_RET _mt_fe_dmd_get_snr_dm6k_t(MT_FE_DM6K_Device_Handle handle, U8 *p_snr);
MT_FE_RET _mt_fe_dmd_get_quality_dm6k_t(MT_FE_DM6K_Device_Handle handle,U8 *quality);
MT_FE_RET _mt_fe_dmd_get_strength_dm6k_t(MT_FE_DM6K_Device_Handle handle, U8 *ssi_percent);
MT_FE_RET _mt_fe_dmd_get_quality_nordig_dm6k_t(MT_FE_DM6K_Device_Handle handle, U8 *quality);
MT_FE_RET _mt_fe_dmd_get_cell_info_dm6k_t(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_get_tps_info_dm6k_t(MT_FE_DM6K_Device_Handle handle, MT_FE_T_TPS_INFO *tps_info);


#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_DEF_DM6K_T_H__ */

