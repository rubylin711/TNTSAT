/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2018                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
*	FILE:				mt_fe_i2c_ct8k.h
*
*	VERSION:			0.10.00
*
*	DESCRIPTION:
*
*Log:	Description			Version		Date			Author
*---------------------------------------------------------------------
*		Create				0.01.00		2014.05.18		BJ.Wang
*		Modify				0.10.00		2014.06.06		BJ.Wang
*********************************************************************************************************/
#ifndef __MT_FE_I2C_CT8K_H__
#define __MT_FE_I2C_CT8K_H__

#include "mt_fe_def.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void mt_i2c_init(void);
MT_FE_RET _mt_fe_ct8k_set_reg(U8 dev_addr, U8 reg_addr, U8 data);
MT_FE_RET _mt_fe_ct8k_get_reg(U8 dev_addr, U8 reg_addr, U8 *reg_data);
MT_FE_RET _mt_fe_dmd_write_ct8k(U8 dev_addr, U8 *p_buf, U16 n_byte);
MT_FE_RET _mt_fe_dmd_write_fw_ct8k(U8 dev_addr, U8 reg_addr, U8 *p_buf, U16 n_byte);
MT_FE_RET mt_fe_tn_write_ct8k(U8 dev_addr, U8 *p_buf, U8 n_byte);
MT_FE_RET mt_fe_tn_read_ct8k(U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte);
MT_FE_RET _mt_fe_dmd_set_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data);
MT_FE_RET _mt_fe_dmd_get_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data);

MT_FE_RET _mt_fe_dmd_get_reg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_dmd_set_reg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data);
MT_FE_RET _mt_fe_tn_get_reg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_tn_set_reg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data);
MT_FE_RET _mt_fe_tn_write_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *p_buf, U16 n_byte);


MT_FE_RET _mt_fe_dmd_get_reg_t_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_dmd_set_reg_t_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data);

MT_FE_RET _mt_fe_dmd_get_reg_t2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_dmd_set_reg_t2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data);

MT_FE_RET _mt_fe_dmd_get_reg_b_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_dmd_set_reg_b_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data);

MT_FE_RET _mt_fe_dmd_get_page_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U16 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_dmd_set_page_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U16 reg_addr, U8 reg_data);

MT_FE_RET _mt_fe_dmd_get_reg_c_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_dmd_set_reg_c_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data);

MT_FE_RET _mt_fe_sar_set_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_index, U8 reg_data);
MT_FE_RET _mt_fe_sar_get_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_index, U8 *p_data);

MT_FE_RET _mt_fe_sys_set_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_index, U8 reg_data);
MT_FE_RET _mt_fe_sys_get_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_index, U8 *p_data);

MT_FE_RET _mt_fe_tn_get_reg_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *reg_data);
MT_FE_RET _mt_fe_tn_set_reg_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data);

MT_FE_RET _mt_fe_write32_ct8k(U32 reg_addr, U32 reg_data);
MT_FE_RET _mt_fe_read32_ct8k(U32 reg_addr, U32 *p_data);


MT_FE_RET mt_fe_i2c_repeat_enable_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_i2c_repeat_disable_ct8k(MT_FE_CT8K_Device_Handle handle);


void _mt_sleep_ct8k(U32 ms);
void _mt_delay_ct8k(U32 ms);

#ifdef __cplusplus
}
#endif
#endif /* __MT_FE_I2C_CT8K_H__ */

