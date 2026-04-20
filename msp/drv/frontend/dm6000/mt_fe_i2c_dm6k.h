/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/*
*	FILE:				mt_fe_i2c_dm6k.h
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
#ifndef __MT_FE_I2C_DM6K_H__
#define __MT_FE_I2C_DM6K_H__

#include "mt_fe_def.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void mt_i2c_init(void);
MT_FE_RET _mt_fe_dm6k_set_reg(U8 dev_addr, U8 reg_addr, U8 data);
MT_FE_RET _mt_fe_dm6k_get_reg(U8 dev_addr, U8 reg_addr, U8 *reg_data);
MT_FE_RET _mt_fe_dmd_write_dm6k(U8 dev_addr, U8 *p_buf, U16 n_byte);
MT_FE_RET _mt_fe_dmd_write_fw_dm6k(U8 dev_addr, U8 reg_addr, U8 *p_buf, U16 n_byte);
MT_FE_RET mt_fe_tn_write_dm6k(U8 dev_addr, U8 *p_buf, U8 n_byte);
MT_FE_RET mt_fe_tn_read_dm6k(U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte);
MT_FE_RET _mt_fe_dmd_set_reg_dm6k(MT_FE_DM6K_Device_Handle handle, U8 reg_addr, U8 reg_data);
MT_FE_RET _mt_fe_dmd_get_reg_dm6k(MT_FE_DM6K_Device_Handle handle, U8 reg_addr, U8 *p_data);

MT_FE_RET _mt_fe_dmd_get_reg_ss2(MT_FE_DM6K_Device_Handle handle, U8 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_dmd_set_reg_ss2(MT_FE_DM6K_Device_Handle handle, U8 reg_addr, U8 reg_data);
MT_FE_RET _mt_fe_tn_get_reg_ss2(MT_FE_DM6K_Device_Handle handle, U8 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_tn_set_reg_ss2(MT_FE_DM6K_Device_Handle handle, U8 reg_addr, U8 reg_data);
MT_FE_RET _mt_fe_tn_write_ss2(MT_FE_DM6K_Device_Handle handle, U8 *p_buf, U16 n_byte);

MT_FE_RET _mt_fe_sys_set_reg(MT_FE_DM6K_Device_Handle handle, U8 reg_index, U8 reg_data);
MT_FE_RET _mt_fe_sys_get_reg(MT_FE_DM6K_Device_Handle handle, U8 reg_index, U8 *p_data);



void _mt_sleep_dm6k(U32 ms);

#ifdef __cplusplus
}
#endif
#endif /* __MT_FE_I2C_DM6K_H__ */

