/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/****************************************************************************
*Montage Proprietary and Confidential
* Montage Technology (Shanghai) Co., Ltd.
*@2015 Montage Technology Group Limited and/or its affiliated companies
* --------------------------------------------------------------------------
*
*	FILE:				mt_fe_i2c.c
*
*	VERSION:			1.70.00
*
*	DESCRIPTION:
*			Define the i2c driver interfaces to write/read the registers of the
*		tuner and demodulator.Use the print port to simulate the SCL/SDA signal
*		to connect to the front end devices.
*
*	FUNCTION:
*
* Log:	Description						Version		Date			Author
*		---------------------------------------------------------------------
*		Create					1.00.00		2008.06.05		ChenXiaopeng
*		Modified				1.05.02		2009.01.15		HuangYouzhong
*		Modify					1.30.25		2011.01.06		WangBingju
*		Modify					1.70.00		2014.12.20		WangBingju
****************************************************************************/
//#include <string.h>
//#include <time.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/printk.h>

#include "mt_fe_def.h"

extern int g_i2c_dd3k;

static MT_FE_RET _mt_fe_i2c_write_dd3k(U8 dev_addr, U8 *w_buf, U16 w_byte)
{
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;
	int ret = 0;

	i2c = i2c_get_adapter(g_i2c_dd3k);
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
		printk("%s[%d] failed, dev_addr = 0x%02x, ret %d\n", __FUNCTION__, __LINE__, dev_addr, ret);
		return MtFeErr_I2cErr;
	}

	return MtFeErr_Ok;
}

static MT_FE_RET _mt_fe_i2c_read_dd3k(U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
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

	i2c = i2c_get_adapter(g_i2c_dd3k);
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
		printk("%s[%d] failed, dev_addr = 0x%02x, index = 0x%02x, ret %d\n", __FUNCTION__, __LINE__, dev_addr, w_buf[0], ret);
		return MtFeErr_I2cErr;
	}

	memcpy(r_buf, buffer, r_byte);

	return MtFeErr_Ok;
}

void _mt_sleep_dd3k(U32 ms)
{
	/*
		TODO:
			Delay ms.
	*/

	if (ms < 10)
		usleep_range(ms * 1000, ms * 1000 + 500);		//mdelay(ms);
	else
		msleep(ms);
}

MT_FE_RET _mt_fe_dmd_write(MT_FE_DD_Device_Handle handle, U16 reg_addr, U8 *p_buf, U16 n_byte)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U8 device_i2c_addr = 0;
	U32 i = 0;
	U8 write_buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	if (handle->demod_cur_mode == MtFeType_CTTB)
		device_i2c_addr = handle->demod_ct_settings.demod_i2c_addr;
	else
		device_i2c_addr = handle->demod_dc_settings.demod_i2c_addr;

	write_buf[0] = (U8)reg_addr;
	for (i = 1; i <= n_byte; i++)
	{
		write_buf[i] = *(p_buf + i - 1);
	}

	ret = _mt_fe_i2c_write_dd3k(device_i2c_addr, write_buf, (U16)(n_byte + 1));

	return ret;
}

MT_FE_RET _mt_fe_dmd_set_reg_dd3k(MT_FE_DD_Device_Handle handle, U16 reg_addr, U8 reg_data)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U8 device_i2c_addr = 0;
	U8 temp[2] = {0, 0};

	if (handle->demod_cur_mode == MtFeType_CTTB)
	{
		device_i2c_addr = handle->demod_ct_settings.demod_i2c_addr;
		if (handle->demod_id == MtFeDmdId_DD630X)
		{
			temp[0] = (U8)0xfe;
			temp[1] = (U8)(reg_addr >> 8);
			ret = _mt_fe_i2c_write_dd3k(device_i2c_addr, temp, (U16)2);
		}
	}
	else
	{
		if (handle->demod_id == MtFeDmdId_DD630X)
			device_i2c_addr = handle->demod_ct_settings.demod_i2c_addr;
		else
			device_i2c_addr = handle->demod_dc_settings.demod_i2c_addr;
	}

	temp[0] = (U8)reg_addr;
	temp[1] = (U8)reg_data;

	ret = _mt_fe_i2c_write_dd3k(device_i2c_addr, temp, (U16)2);

	return ret;
}

MT_FE_RET _mt_fe_dmd_get_reg_dd3k(MT_FE_DD_Device_Handle handle, U16 reg_addr, U8 *p_buf)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U8 device_i2c_addr = 0;
	U8 temp[2] = {0, 0};

	if (handle->demod_cur_mode == MtFeType_CTTB)
	{
		device_i2c_addr = handle->demod_ct_settings.demod_i2c_addr;
		if (handle->demod_id != MtFeDmdId_DD3X0X)
		{
			temp[0] = (U8)0xfe;
			temp[1] = (U8)(reg_addr >> 8);
			ret = _mt_fe_i2c_write_dd3k(device_i2c_addr, temp, (U16)2);
		}
	}
	else
	{
		if (handle->demod_id == MtFeDmdId_DD630X)
			device_i2c_addr = handle->demod_ct_settings.demod_i2c_addr;
		else
			device_i2c_addr = handle->demod_dc_settings.demod_i2c_addr;
	}

	temp[0] = (U8)reg_addr;
	ret = _mt_fe_i2c_read_dd3k(device_i2c_addr, temp, (U16)(1), p_buf, (U16)(1));

	return ret;
}

MT_FE_RET _mt_fe_tn_write_dd3k(MT_FE_DD_Device_Handle handle, U8 *p_buf, U16 n_byte)
{
	MT_FE_RET ret = MtFeErr_I2cErr;
	U8 data = 0, num = 0;
	MT_FE_TYPE last_mode;
	U8 device_i2c_addr = 0;

	if ((handle->demod_id == MtFeDmdId_DD630X) && (handle->demod_cur_mode != MtFeType_CTTB))
	{
		_mt_fe_dmd_get_reg_dd3k(handle, 0x0087, &data);
		data = (U8)((data & 0x0f) | 0x90);
						// bit7		 = 1, Enable I2C repeater
						// bit[6:4]	 = 1, Enable I2C repeater for 1 time
		ret = _mt_fe_dmd_set_reg_dd3k(handle, 0x0087, data);
		if (ret != MtFeErr_Ok)
			return ret;
		device_i2c_addr = handle->tuner_dc_settings.tuner_dev_addr;
	}
	else
	{
		last_mode = handle->demod_cur_mode;
		handle->demod_cur_mode = MtFeType_CTTB;
		_mt_fe_dmd_get_reg_dd3k(handle, 0x0003, &data);
		data = (U8)((data & 0xf8) | 0x11);
		ret = _mt_fe_dmd_set_reg_dd3k(handle, 0x0003, data); //open demod IIC repeater 1 time!
		handle->demod_cur_mode = last_mode;					 //,and hardware close the repeater automatically
		if (ret != MtFeErr_Ok)
			return ret;

		if (handle->demod_cur_mode == MtFeType_CTTB)
			device_i2c_addr = handle->tuner_ct_settings.tuner_dev_addr;
		else
			device_i2c_addr = handle->tuner_dc_settings.tuner_dev_addr;
	}

	ret = _mt_fe_i2c_write_dd3k(device_i2c_addr, p_buf, (U16)n_byte);

	return ret;
}

MT_FE_RET _mt_fe_tn_read_dd3k(MT_FE_DD_Device_Handle handle, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
{
	MT_FE_RET ret = MtFeErr_I2cErr;
	U8 data = 0;
	MT_FE_TYPE last_mode;
	U8 device_i2c_addr = 0;

	if ((handle->demod_id == MtFeDmdId_DD630X) && (handle->demod_cur_mode != MtFeType_CTTB))
	{
		_mt_fe_dmd_get_reg_dd3k(handle, 0x0087, &data);
		data = (U8)((data & 0x0f) | 0xA0);
						// bit7		 = 1, Enable I2C repeater
						// bit[6:4]	 = 1, Enable I2C repeater for 1 time
		ret = _mt_fe_dmd_set_reg_dd3k(handle, 0x0087, data);
		if (ret != MtFeErr_Ok)
			return ret;

		if (handle->demod_cur_mode == MtFeType_CTTB)
			device_i2c_addr = handle->tuner_ct_settings.tuner_dev_addr;
		else
			device_i2c_addr = handle->tuner_dc_settings.tuner_dev_addr;
	}
	else
	{
		last_mode = handle->demod_cur_mode;
		handle->demod_cur_mode = MtFeType_CTTB;
		_mt_fe_dmd_get_reg_dd3k(handle, 0x0003, &data);
		data = (U8)((data & 0xf8) | 0x12);
		ret = _mt_fe_dmd_set_reg_dd3k(handle, 0x0003, data); //open demod IIC repeater 2 time!
		handle->demod_cur_mode = last_mode;					 //,and hardware close the repeater automatically
		if (ret != MtFeErr_Ok)
			return ret;

		if (handle->demod_cur_mode == MtFeType_CTTB)
			device_i2c_addr = handle->tuner_ct_settings.tuner_dev_addr;
		else
			device_i2c_addr = handle->tuner_dc_settings.tuner_dev_addr;
	}

	ret = _mt_fe_i2c_read_dd3k(device_i2c_addr, w_buf, w_byte, r_buf, r_byte);

	return ret;
}

