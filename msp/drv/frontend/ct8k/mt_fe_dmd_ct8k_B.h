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
* File:				mt_fe_def_ct8k_B.h
*
* Current version:	01
*
* Description:
*
* Log:	Description		Version		Date		Author
*---------------------------------------------------------------------
*		Create			00			2018.06.25	YZ.Huang
*		Modify			01			2018.06.25	YZ.Huang
****************************************************************************/
#ifndef __MT_FE_DEF_CT8K_B_H__
#define __MT_FE_DEF_CT8K_B_H__

//#include <stdio.h>
#include "mt_fe_def.h"

#ifdef __cplusplus
extern "C" {
#endif

MT_FE_RET mt_fe_dmd_get_driver_version_ct8k_b(U8* p_version);
MT_FE_RET _mt_fe_dmd_init_ct8k_b(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_demod_ct8k_b(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_symbol_rate_ct8k_b(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_QAM_ct8k_b(MT_FE_CT8K_Device_Handle handle, U16 qam);
MT_FE_RET _mt_fe_dmd_soft_reset_ct8k_b(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_connect_ct8k_b(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_get_statistics_ct8k_b(MT_FE_CT8K_Device_Handle handle, U32 *agc_lock, U32 *timing_lock, U32 *dagc_lock, U32 *carrier_lock, U32 *chip_lock);
MT_FE_RET _mt_fe_dmd_get_lock_state_ct8k_b(MT_FE_CT8K_Device_Handle handle, MT_FE_LOCK_STATE *lock_status);
MT_FE_RET _mt_fe_dmd_get_ber_ct8k_b(MT_FE_CT8K_Device_Handle handle, U32 *error_bits, U32 *total_bits);
MT_FE_RET _mt_fe_dmd_get_snr_ct8k_b(MT_FE_CT8K_Device_Handle handle, U16 *signal_snr);
MT_FE_RET _mt_fe_dmd_get_strength_gain_ct8k_b(MT_FE_CT8K_Device_Handle handle, U8 *signal_strength);
MT_FE_RET _mt_fe_dmd_get_strength_ct8k_b(MT_FE_CT8K_Device_Handle handle, U8 *ssi_percent);
MT_FE_RET _mt_fe_dmd_get_quality_ct8k_b(MT_FE_CT8K_Device_Handle handle, U8 *p_percent);
MT_FE_RET _mt_fe_dmd_get_frames_ct8k_b(MT_FE_CT8K_Device_Handle handle, U8 *good_frames, U8 *bad_frames, U8 *total_frames);

#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_DEF_CT8K_B_H__ */

