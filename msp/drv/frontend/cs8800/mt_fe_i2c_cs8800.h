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
*	FILE:				mt_fe_i2c_cs8800.h
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
#ifndef __MT_FE_I2C_CS8800_H__
#define __MT_FE_I2C_CS8800_H__

#include "mt_fe_def.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void mt_i2c_init(void);

MT_FE_RET _mt_fe_cs8800_set_reg(U8 dev_addr, U8 reg_addr, U8 data);
MT_FE_RET _mt_fe_cs8800_get_reg(U8 dev_addr, U8 reg_addr, U8 *reg_data);
MT_FE_RET _mt_fe_dmd_write_cs8800(U8 dev_addr, U8 *p_buf, U16 n_byte);
MT_FE_RET _mt_fe_dmd_write_fw_cs8800(U8 dev_addr, U8 reg_addr, U8 *p_buf, U16 n_byte);
MT_FE_RET mt_fe_tn_write_cs8800(U8 dev_addr, U8 *p_buf, U8 n_byte);
MT_FE_RET mt_fe_tn_read_cs8800(U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte);

MT_FE_RET _mt_fe_dmd_get_reg_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, U8 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_dmd_set_reg_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, U8 reg_addr, U8 reg_data);
MT_FE_RET _mt_fe_tn_get_reg_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, U8 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_tn_set_reg_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, U8 reg_addr, U8 reg_data);
MT_FE_RET _mt_fe_tn_write_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, U8 *p_buf, U16 n_byte);


MT_FE_RET _mt_fe_dmd_get_reg_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_dmd_set_reg_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 reg_addr, U8 reg_data);

MT_FE_RET _mt_fe_dmd_get_page_reg_cs8800(MT_FE_CS8800_Device_Handle handle, U16 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_dmd_set_page_reg_cs8800(MT_FE_CS8800_Device_Handle handle, U16 reg_addr, U8 reg_data);

MT_FE_RET _mt_fe_dmd_get_reg_c_cs8800(MT_FE_CS8800_Device_Handle handle, U8 reg_addr, U8 *p_data);
MT_FE_RET _mt_fe_dmd_set_reg_c_cs8800(MT_FE_CS8800_Device_Handle handle, U8 reg_addr, U8 reg_data);

MT_FE_RET _mt_fe_sar_set_reg_cs8800(MT_FE_CS8800_Device_Handle handle, U8 reg_index, U8 reg_data);
MT_FE_RET _mt_fe_sar_get_reg_cs8800(MT_FE_CS8800_Device_Handle handle, U8 reg_index, U8 *p_data);

MT_FE_RET _mt_fe_tn_get_reg_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 reg_addr, U8 *reg_data);
MT_FE_RET _mt_fe_tn_set_reg_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 reg_addr, U8 reg_data);

MT_FE_RET _mt_fe_write32_cs8800(U32 reg_addr, U32 reg_data);
MT_FE_RET _mt_fe_read32_cs8800(U32 reg_addr, U32 *p_data);


MT_FE_RET mt_fe_i2c_repeat_enable_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_i2c_repeat_disable_cs8800(MT_FE_CS8800_Device_Handle handle);

void _mt_sleep_cs8800(U32 ms);
void _mt_delay_cs8800(U32 ms);
void _mt_delayus_cs8800(U32 us);

#ifdef __cplusplus
}
#endif
#endif /* __MT_FE_I2C_CS8800_H__ */

