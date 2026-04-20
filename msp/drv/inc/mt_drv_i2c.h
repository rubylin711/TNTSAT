/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_DRV_I2C_H__
#define __MT_DRV_I2C_H__

#include "mt_type.h"
#include "mt_debug.h"

#define MT_FATAL_I2C(fmt...) MT_FATAL_PRINT(MT_ID_I2C, fmt)
#define MT_ERR_I2C(fmt...) MT_ERR_PRINT(MT_ID_I2C, fmt)
#define MT_WARN_I2C(fmt...) MT_WARN_PRINT(MT_ID_I2C, fmt)
#define MT_INFO_I2C(fmt...) MT_INFO_PRINT(MT_ID_I2C, fmt)

#define MT_STD_I2C_NUM 2
#define MT_I2C_MAX_NUM_RESERVE 0
#define MT_I2C_MAX_NUM (MT_I2C_MAX_NUM_USER + MT_I2C_MAX_NUM_RESERVE)

#define MT_I2C_MAX_LENGTH 2048

#if 0
mt_s32  mt_drv_i2c_init(mt_void);
mt_void mt_drv_i2c_deinit(mt_void);

mt_s32 mt_drv_i2c_write_config(mt_u32 i2c_id, mt_u8 dev_addr);
mt_s32 mt_drv_i2c_write(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr, mt_u32 reg_addr_byte_num, mt_u8 * p_data,
                        mt_u32 data_len);
mt_s32 mt_drv_i2c_read(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr, mt_u32 reg_addr_byte_num, mt_u8 * p_data,
                       mt_u32 data_len);
mt_s32 mt_drv_i2c_read_silabs(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr, mt_u32 reg_addr_byte_num, mt_u8 *p_data,
                       mt_u32 data_len);
mt_s32 mt_drv_i2c_write_sony(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr, mt_u32 reg_addr_byte_num, mt_u8 *p_data, mt_u32 data_len, mt_u8 mode);
mt_s32 mt_drv_i2c_read_sony(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr, mt_u32 reg_addr_byte_num, mt_u8 *p_data, mt_u32 data_len, mt_u8 mode);
mt_s32 mt_drv_i2c_read_directly(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr, mt_u32 reg_addr_byte_num,
                               mt_u8 *p_data, mt_u32 data_len);
mt_s32 mt_drv_i2c_write_no_stop(mt_u32 i2c_id, mt_u8 dev_addr, mt_u32 reg_addr, mt_u32 reg_addr_byte_num,
                               mt_u8 *p_data, mt_u32 data_len);

mt_s32 mt_drv_i2c_set_rate(mt_u32 i2c_id, mt_u32 i2c_rate);
#endif

#endif
