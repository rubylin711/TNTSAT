/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*
 * Filename:        mt_fe_i2c.h
 *
* Current version:	1.00.00
*
* Description:		i2c function prototype for FE module.
*
* Log:	Description			Version			Date			Author
*		---------------------------------------------------------------------
*		Create				1.00.00			2010.09.15		YZ.Huang
*		Modify				1.00.00			2010.09.15		YZ.Huang
****************************************************************************/
#ifndef __MT_FE_I2C_CS8K_SAT_H__
#define __MT_FE_I2C_CS8K_SAT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mt_fe_def_cs8000_sat.h"

extern void _mt_cs8k_sat_sleep(U32 ms);
extern MT_FE_RET _mt_fe_cs8k_sat_dmd_set_reg(MT_FE_CS8000_SAT_Device_Handle handle, U8 reg_index, U8 data);
extern MT_FE_RET _mt_fe_cs8k_sat_dmd_get_reg(MT_FE_CS8000_SAT_Device_Handle handle, U8 reg_index, U8 *p_buf);
extern MT_FE_RET _mt_fe_cs8k_sat_tn_set_reg(MT_FE_CS8000_SAT_Device_Handle handle, U8 reg_index, U8 data);
extern MT_FE_RET _mt_fe_cs8k_sat_tn_get_reg(MT_FE_CS8000_SAT_Device_Handle handle, U8 reg_index, U8 *p_buf);
extern MT_FE_RET _mt_fe_cs8k_sat_tn_write(MT_FE_CS8000_SAT_Device_Handle handle, U8 *p_buf, U16 n_byte);
extern MT_FE_RET _mt_fe_cs8k_sat_write_fw(MT_FE_CS8000_SAT_Device_Handle handle, U8 reg_index, U8 *p_buf, U16 n_byte);
extern MT_FE_RET _mt_fe_cs8k_sat_write32(U32 reg_addr, U32 reg_data);
extern MT_FE_RET _mt_fe_cs8k_sat_read32(U32 reg_addr, U32 *p_data);

extern MT_FE_RET _mt_fe_cs8k_sat_lnb_write(U8 i2c_id, U8 slav_addr, U8 *p_buf, U16 n_byte);
extern MT_FE_RET _mt_fe_cs8k_sat_lnb_read(U8 i2c_id, U8 slav_addr, U8 *p_buf, U16 n_byte);

#ifdef __cplusplus
}
#endif

#endif /* __MT_FE_I2C_H__ */
