/********************************************************************************************/
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
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
*	FILE:				mt_fe_i2c_dm6k.c
*
*	VERSION:			0.60.00
*
*	DESCRIPTION:
*			Define the i2c driver interfaces to write/read the registers of the
*		tuner and demodulator.Use the print port to simulate the SCL/SDA signal
*		to connect to the front end devices.
*
*	FUNCTION:
*
* Log:	Description	Version		Date			Author
*---------------------------------------------------------------------
*		Create		0.00.00		2014.06.05		BJ.Wang
*		Modify		0.60.00		2014.08.06		BJ.Wang
*************************************************************************************************/
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/io.h>

#include "port_dm6k.h"

#include "mt_fe_def.h"
#include "mt_fe_i2c_dm6k.h"



extern int g_i2c_dm6k;


static MT_FE_RET _mt_fe_i2c_write(U8 dev_addr, U8 *w_buf, U16 w_byte)
{
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;
	int ret = 0;

	i2c = i2c_get_adapter(g_i2c_dm6k);
	msg->addr = dev_addr;
	msg->flags = I2C_M_TEN | I2C_M_SALVE_TYPE;
	msg->buf = w_buf;
	msg->len = w_byte;
	mt_msg.rlen = 0;
	mt_msg.wlen = w_byte;
	mt_msg.slave_type = I2C_SLAVE_DEV_SOC_EXTER;
	ret = i2c_transfer(i2c, msg, 1);
	if (ret < 0)
	{
		//printk("_mt_fe_i2c_write() failed, dev_addr = 0x%02x, ret %d\n", dev_addr, ret);
		return MtFeErr_I2cErr;
	}

	return MtFeErr_Ok;
}

static MT_FE_RET _mt_fe_i2c_read(U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
{
	// 8 bit Register Read Protocol:
	// +------+-+-----+-+-+----------+-+
	// |MASTER|S|SADDR|W|  |RegAddr   |
	// +------+-+-----+-+-+-----------+-+
	// |SLAVE |                          |A|               |A| |
	// +------+-+-----+-+-+-----------+-+
	// +------+-+-----+-+-+-----+--+-+
	// |MASTER|S|SADDR|R| |     |MN|P|
	// +------+-+-----+-+-+-----+--+-+
	// |SLAVE |         |A|Data |  | |
	// +------+---------+-+-----+--+-+
	// Legends: SADDR(I2c slave address), S(Start condition), MA(Master Ack), MN(Master NACK),
	// P(Stop condition)
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;
	int ret = 0;
	U8 buffer[32];

	//*r_buf = *w_buf;
	memcpy(buffer, w_buf, w_byte);

	i2c = i2c_get_adapter(g_i2c_dm6k);
	//printk("[%s ] line:%d i2c_bus 0x%08x\n", __func__, __LINE__, i2c);
	msg->addr = dev_addr;
	msg->flags = I2C_M_STD_RD | I2C_M_SALVE_TYPE;
	//msg->buf = r_buf;
	msg->buf = buffer;
	msg->len = r_byte;
	mt_msg.rlen = r_byte;
	mt_msg.wlen = w_byte;
	mt_msg.slave_type = I2C_SLAVE_DEV_SOC_EXTER;
	ret = i2c_transfer(i2c, msg, 1);
	if (ret < 0)
	{
		//printk("_mt_fe_i2c_read() failed, dev_addr = 0x%02x, ret %d\n", dev_addr, ret);
		return MtFeErr_I2cErr;
	}

	memcpy(r_buf, buffer, r_byte);

	return MtFeErr_Ok;
}


void _mt_sleep_dm6k(U32 ms)
{
	/*
		TODO:
			Delay ms.
	*/

	msleep(ms);
}

MT_FE_RET _mt_fe_dmd_write_dm6k(U8 dev_addr, U8 *p_buf, U16 n_byte)
{
	MT_FE_RET ret;

	ret = _mt_fe_i2c_write(dev_addr, p_buf, n_byte);

	if (ret != MtFeErr_Ok)
	{
		printk("MT:  _mt_fe_dmd_write_dm6k() FAILED!\n");
		return MtFeErr_I2cErr;
	}

	return MtFeErr_Ok;
}


MT_FE_RET _mt_fe_dm6k_set_reg(U8 dev_addr, U8 reg_addr, U8 data)
{
	MT_FE_RET ret;
	U8 buf[2];

	buf[0] = reg_addr;
	buf[1] = data;

	//printk("[%s ] line:%d\n", __func__, __LINE__);

	ret = _mt_fe_i2c_write(dev_addr, buf, (U16)2);

	if (ret != MtFeErr_Ok)
	{
		printk("MT:  _mt_fe_dm6k_set_reg() dev_addr = 0x%02x, reg 0x%02x - data 0x%02x FAILED!\n", dev_addr, reg_addr, data);
		return MtFeErr_I2cErr;
	}

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dm6k_get_reg(U8 dev_addr, U8 reg_addr, U8 *reg_data)
{
	MT_FE_RET ret;
	U8 temp[2] = {0, 0};

	//printk("[%s ] line:%d\n", __func__, __LINE__);

	temp[0] = (U8)reg_addr;
	ret = _mt_fe_i2c_read(dev_addr, temp, (U16)(1), reg_data, (U16)(1));

	if (ret != MtFeErr_Ok)
	{
		printk("MT:	_mt_fe_dm6k_get_reg() dev_addr = 0x%02x, reg 0x%02x FAILED!\n", dev_addr, reg_addr);
		return MtFeErr_I2cErr;
	}

	return MtFeErr_Ok;
}


MT_FE_RET _mt_fe_dmd_write_fw_dm6k(U8 dev_addr, U8 reg_addr, U8 *p_buf, U16 n_byte)
{
	U8 fw_data[1024];

	fw_data[0] = reg_addr;
	memcpy(&fw_data[1], p_buf, n_byte);

	return _mt_fe_dmd_write_dm6k(dev_addr, fw_data, (U16)(n_byte + 1));
}



MT_FE_RET mt_fe_tn_write_dm6k(U8 dev_addr, U8 *p_buf, U8 n_byte)
{
	MT_FE_RET ret;

	ret = _mt_fe_i2c_write(dev_addr, p_buf, n_byte);

	if (ret != MtFeErr_Ok)
	{
		printk("MT:  mt_fe_tn_write_dm6k() FAILED!\n");
		return MtFeErr_I2cErr;
	}

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_read_dm6k(U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
{
	MT_FE_RET ret;

	ret = _mt_fe_i2c_read(dev_addr, w_buf, w_byte, r_buf, r_byte);

	if (ret != MtFeErr_Ok)
	{
		printk("MT:	mt_fe_tn_read_dm6k reg() FAILED!\n");
		return MtFeErr_I2cErr;
	}

	return MtFeErr_Ok;
}



MT_FE_RET _mt_fe_dmd_get_reg_dm6k(MT_FE_DM6K_Device_Handle handle, U8 reg_addr, U8 *p_data)
{
	return _mt_fe_dm6k_get_reg(handle->sys_dev_addr, reg_addr, p_data);
}

MT_FE_RET _mt_fe_dmd_set_reg_dm6k(MT_FE_DM6K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
	return _mt_fe_dm6k_set_reg(handle->sys_dev_addr, reg_addr, reg_data);
}

MT_FE_RET _mt_fe_sys_set_reg(MT_FE_DM6K_Device_Handle handle, U8 reg_index, U8 reg_data)
{
	MT_FE_RET ret = MtFeErr_Ok;

	ret = _mt_fe_dm6k_set_reg(handle->sys_dev_addr, 0x1E, reg_index);
	ret |= _mt_fe_dm6k_set_reg(handle->sys_dev_addr, 0x1F, reg_data);

	return ret;
}

MT_FE_RET _mt_fe_sys_get_reg(MT_FE_DM6K_Device_Handle handle, U8 reg_index, U8 *p_data)
{
	MT_FE_RET ret = MtFeErr_Ok;

	ret = _mt_fe_dm6k_set_reg(handle->sys_dev_addr, 0x1E, reg_index);
	ret |= _mt_fe_dm6k_get_reg(handle->sys_dev_addr, 0x1F, p_data);

	return ret;
}
