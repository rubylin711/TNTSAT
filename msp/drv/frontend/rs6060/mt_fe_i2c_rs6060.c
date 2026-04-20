/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/****************************************************************************
* MONTAGE PROPRIETARY AND CONFIDENTIAL
* Montage Technology (Shanghai) Inc.
* All Rights Reserved
* --------------------------------------------------------------------------
*
* File:				mt_fe_i2c.c
*
* Current version:	1.00.00
*
* Description:		Define all i2c function for FE module.
*
* Log:	Description			Version		Date		Author
*		---------------------------------------------------------------------
*		Create				1.00.00		2010.09.15	YZ.Huang
*		Modify				1.00.00		2010.09.15	YZ.Huang
****************************************************************************/
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/io.h>

#include "port_rs6060.h"

#include "mt_fe_def.h"
#include "mt_fe_i2c_rs6060.h"

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif



extern int g_i2c_rs6060;


static MT_FE_RET _mt_fe_i2c_write(U8 dev_addr, U8 *w_buf, U16 w_byte)
{
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;
	int ret = 0;

	i2c = i2c_get_adapter(g_i2c_rs6060);
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

	i2c = i2c_get_adapter(g_i2c_rs6060);
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


void _mt_sleep_rs6060(U32 ms)
{
	/*
		TODO:
			Delay ms.
	*/

	if (ms < 10)
	{
		usleep_range(ms * 1000, ms * 1000 + 500);
	}
	else
	{
		msleep(ms);
	}
}

void _mt_delayus_rs6060(U32 us)
{
	usleep_range(us / 100 * 100 + 50, us / 100 * 100 + 100);
}

MT_FE_RET _mt_fe_write32_rs6060(U32 reg_addr, U32 reg_data)
{
	HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(reg_addr), reg_data);

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_read32_rs6060(U32 reg_addr, U32 *p_data)
{
	*p_data = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(reg_addr));

	return MtFeErr_Ok;
}

/*****************************************************************
** Function: _mt_fe_dmd_set_reg
**
**
** Description:	write data to demod register
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	reg_index		U8			register index
**	data			U8			value to write
**
**
** Outputs:
**
**
*****************************************************************/
MT_FE_RET _mt_fe_dmd_set_reg_rs6060(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 data)
{
	/*
		TODO:
			Obtain the i2c mutex
	*/



	/*
		TODO:
			write data to demodulator register
	*/

	MT_FE_RET ret;
	U8 buf[2];

	buf[0] = reg_index;
	buf[1] = data;

	//printk("[%s ] line:%d\n", __func__, __LINE__);

	ret = _mt_fe_i2c_write(handle->demod_dev_addr, buf, (U16)2);

	if (ret != MtFeErr_Ok)
	{
		printk("MT:  _mt_fe_dmd_set_reg_rs6060() dev_addr = 0x%02x, reg 0x%02x - data 0x%02x FAILED!\n", handle->demod_dev_addr, reg_index, data);
		return MtFeErr_I2cErr;
	}

	/*
		TODO:
			Release the i2c mutex
	*/



	return MtFeErr_Ok;
}


/*****************************************************************
** Function: _mt_fe_dmd_get_reg
**
**
** Description:	read data from demod register
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	reg_index		U8			register index
**
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	p_buf			U8*			register data
**
**
*****************************************************************/
MT_FE_RET _mt_fe_dmd_get_reg_rs6060(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 *p_buf)
{
	/*
		TODO:
			Obtain the i2c mutex
	*/



	/*
		TODO:
			read demodulator register value
	*/


	MT_FE_RET ret;
	U8 temp[2] = {0, 0};

	//printk("[%s ] line:%d\n", __func__, __LINE__);

	temp[0] = (U8)reg_index;
	ret = _mt_fe_i2c_read(handle->demod_dev_addr, temp, (U16)(1), p_buf, (U16)(1));

	if (ret != MtFeErr_Ok)
	{
		printk("MT:	_mt_fe_dmd_get_reg_rs6060() dev_addr = 0x%02x, reg 0x%02x FAILED!\n", handle->demod_dev_addr, reg_index);
		return MtFeErr_I2cErr;
	}


	/*
		TODO:
			Release the i2c mutex
	*/


	return MtFeErr_Ok;
}


/*****************************************************************
** Function: _mt_fe_tn_set_reg
**
**
** Description:	write data to tuner register
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	reg_index		U8			register index
**	data			U8			value to write
**
**
** Outputs:
**
**
*****************************************************************/
MT_FE_RET _mt_fe_tn_set_reg_rs6060(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 data)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U8 val;
	U8 buf[2];


	/*
		TODO:
			Obtain the i2c mutex
	*/


	/*open I2C repeater*/
	/*Do not care to close the I2C repeater, it will close by itself*/
	val = 0x11;
	ret = _mt_fe_dmd_set_reg_rs6060(handle, 0x03, val);
	if (ret != MtFeErr_Ok)
		return ret;


	/*Do not sleep any time after I2C repeater is opened.*/
	/*please set tuner register at once.*/


	/*
		TODO:
			write value to tuner register
	*/


	buf[0] = reg_index;
	buf[1] = data;

	//printk("[%s ] line:%d\n", __func__, __LINE__);

	ret = _mt_fe_i2c_write(handle->tuner_cfg.tuner_dev_addr, buf, (U16)2);

	if (ret != MtFeErr_Ok)
	{
		printk("MT:  _mt_fe_tn_set_reg_rs6060() dev_addr = 0x%02x, reg 0x%02x - data 0x%02x FAILED!\n", handle->tuner_cfg.tuner_dev_addr, reg_index, data);
		return MtFeErr_I2cErr;
	}


	/*
		TODO:
			Release the i2c mutex
	*/



	return MtFeErr_Ok;
}


/*****************************************************************
** Function: _mt_fe_tn_get_reg
**
**
** Description:	get tuner register data
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	register		U8			register address
**
**
** Outputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	p_buf			U8*			register data
**
**
*****************************************************************/
MT_FE_RET _mt_fe_tn_get_reg_rs6060(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 *p_buf)
{
	MT_FE_RET ret;
	U8 val;
	U8 temp[2] = {0, 0};

	/*
		TODO:
			Obtain the i2c mutex
	*/


	/*open I2C repeater*/
	/*Do not care to close the I2C repeater, it will close by itself*/
	val = 0x12;
		// IMPORTANT:
		// This value can be 0x11 or 0x12.
		// It depends on the sum of I2C_STOP flags in a whole I2CRead operation flow.
		// 0x11 means there's only ONE I2C_STOP flag.
		// 0x12 means there're two I2C_STOP flags.
		// Please refer to the application notes for detail descriptions
	ret = _mt_fe_dmd_set_reg_rs6060(handle, 0x03, val);
	if (ret != MtFeErr_Ok)
		return ret;

	/*Do not sleep any time after I2C repeater is opened.*/
	/*please read tuner register at once.*/


	/*
		TODO:
			read tuner register value
	*/


	//printk("[%s ] line:%d\n", __func__, __LINE__);

	temp[0] = (U8)reg_index;
	ret = _mt_fe_i2c_read(handle->tuner_cfg.tuner_dev_addr, temp, (U16)(1), p_buf, (U16)(1));

	if (ret != MtFeErr_Ok)
	{
		printk("MT:	_mt_fe_tn_get_reg_rs6060() dev_addr = 0x%02x, reg 0x%02x FAILED!\n", handle->tuner_cfg.tuner_dev_addr, reg_index);
		return MtFeErr_I2cErr;
	}

	/*
		TODO:
			Release the i2c mutex
	*/


	return MtFeErr_Ok;
}

/*****************************************************************
** Function: _mt_fe_iic_write
**
**
** Description:	write n bytes via iic to device
**
**
** Inputs:
**
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	p_buf			U8*			pointer of the tuner data
**	n_byte			U16			the data length
**
**
** Outputs:		none
**
**
**
*****************************************************************/
MT_FE_RET _mt_fe_write_fw_rs6060(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 *p_buf, U16 n_byte)
{

	/*
		TODO:
			Obtain the i2c mutex
	*/
	MT_FE_RET ret;
	U8 buf[130];
	int i = 0;

	buf[0] = reg_index;

	for (i = 0; i < n_byte; i ++)
	{
		buf[i + 1] = p_buf[i];
	}



	/*
		TODO:
			write n bytes to demodulator
	*/


	//printk("[%s ] line:%d\n", __func__, __LINE__);

	ret = _mt_fe_i2c_write(handle->demod_dev_addr, buf, (U16)(n_byte + 1));

	if (ret != MtFeErr_Ok)
	{
		printk("MT:  _mt_fe_write_fw_rs6060() dev_addr = 0x%02x, reg 0x%02x FAILED!\n", handle->demod_dev_addr, reg_index);
		return MtFeErr_I2cErr;
	}


	/*
		TODO:
			Release the i2c mutex
	*/

	return MtFeErr_Ok;
}



