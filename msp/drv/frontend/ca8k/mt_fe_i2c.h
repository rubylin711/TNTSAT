/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/*
 * Filename:        mt_fe_i2c.h
 *
 * Description:     2-wire bus function prototype for FE module.
 *
 *
 * Author:          YZ.Huang
 *
 * Version:         1.01.00
 * Date:            2014-12-23

 * History:
 * Description      Version     Date            Author
 *----------------------------------------------------------------------------
 * File Create      1.01.00     2012.02.28      YZ.Huang
 *      Modify      1.03.03     2016.04.14      YZ.Huang
 *----------------------------------------------------------------------------
 *****************************************************************************/

#ifndef __MT_FE_I2C_H__
#define __MT_FE_I2C_H__

#include "mt_fe_def_ca8k.h"

#ifdef __cplusplus
extern "C" {
#endif

MT_FE_RET _mt_fe_dmd_set_reg(void *dev_handle, U8 reg_index, U8 data);
MT_FE_RET _mt_fe_dmd_get_reg(void *dev_handle, U8 reg_index, U8 *p_buf);
MT_FE_RET _mt_fe_tn_set_reg(void *dev_handle, U8 reg_index, U8 data);
MT_FE_RET _mt_fe_tn_get_reg(void *dev_handle, U8 reg_index, U8 *p_buf);
MT_FE_RET _mt_fe_tn_write(void *dev_handle, U8 *p_buf, U16 n_byte);
MT_FE_RET _mt_fe_tn_read(void *dev_handle, U8 *p_buf, U16 n_byte);
MT_FE_RET _mt_fe_write32(U32 reg_addr, U32 reg_data);
MT_FE_RET _mt_fe_read32(U32 reg_addr, U32 *p_data);

void _mt_sleep(U32 ticks_ms);
void _mt_sleep_tc6800(U32 ticks_ms);

#ifdef __cplusplus
};
#endif

#endif /* __MT_FE_I2C_H__ */
