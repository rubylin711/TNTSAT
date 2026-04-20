/********************************************************************************************/
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2020                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
* File:				mt_fe_def_cs8800_B.h
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
#ifndef __MT_FE_DEF_CS8800_B_H__
#define __MT_FE_DEF_CS8800_B_H__

#include "mt_fe_def.h"

#ifdef __cplusplus
extern "C" {
#endif

MT_FE_RET mt_fe_dmd_get_driver_version_cs8800_b(U8* p_version);
MT_FE_RET _mt_fe_dmd_init_cs8800_b(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_demod_cs8800_b(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_symbol_rate_cs8800_b(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_QAM_cs8800_b(MT_FE_CS8800_Device_Handle handle, U16 qam);
MT_FE_RET _mt_fe_dmd_soft_reset_cs8800_b(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_connect_cs8800_b(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_get_statistics_cs8800_b(MT_FE_CS8800_Device_Handle handle, U32 *agc_lock, U32 *timing_lock, U32 *dagc_lock, U32 *carrier_lock, U32 *chip_lock);
MT_FE_RET _mt_fe_dmd_get_lock_state_cs8800_b(MT_FE_CS8800_Device_Handle handle, MT_FE_LOCK_STATE *lock_status);
MT_FE_RET _mt_fe_dmd_get_ber_cs8800_b(MT_FE_CS8800_Device_Handle handle, U32 *error_bits, U32 *total_bits);
MT_FE_RET _mt_fe_dmd_get_post_ber_cs8800_b(MT_FE_CS8800_Device_Handle handle, U32 *error_bits, U32 *total_bits);
MT_FE_RET _mt_fe_dmd_get_accurate_snr_cs8800_b(MT_FE_CS8800_Device_Handle handle, S32 *signal_snr);
MT_FE_RET _mt_fe_dmd_get_snr_cs8800_b(MT_FE_CS8800_Device_Handle handle, U16 *signal_snr);
MT_FE_RET _mt_fe_dmd_get_strength_gain_cs8800_b(MT_FE_CS8800_Device_Handle handle, U8 *signal_strength);
MT_FE_RET _mt_fe_dmd_get_strength_cs8800_b(MT_FE_CS8800_Device_Handle handle, U8 *ssi_percent);
MT_FE_RET _mt_fe_dmd_get_quality_cs8800_b(MT_FE_CS8800_Device_Handle handle, U8 *p_percent);
MT_FE_RET _mt_fe_dmd_get_frames_cs8800_b(MT_FE_CS8800_Device_Handle handle, U8 *good_frames, U8 *bad_frames, U8 *total_frames);

#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_DEF_CS8800_B_H__ */

