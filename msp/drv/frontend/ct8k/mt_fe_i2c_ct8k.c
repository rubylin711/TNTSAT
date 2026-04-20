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
*	FILE:				mt_fe_i2c_ct8k.c
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

#include "port_ct8k.h"

#include "mt_fe_def.h"
#include "mt_fe_i2c_ct8k.h"



extern int g_i2c_ct8k;

#define CT8K_APB_I2C			1

#define CT8K_APB_BASE_ADDRESS	(0xBF5B0000)
#define CT8K_APB_ADDR_J83B		(CT8K_APB_BASE_ADDRESS + 0x0000)
#define CT8K_APB_ADDR_DVBC		(CT8K_APB_BASE_ADDRESS + 0x0800)
#define CT8K_APB_ADDR_SS2		(CT8K_APB_BASE_ADDRESS + 0x0900)
#define CT8K_APB_ADDR_DVBT		(CT8K_APB_BASE_ADDRESS + 0x0A00)
#define CT8K_APB_ADDR_DVBT2		(CT8K_APB_BASE_ADDRESS + 0x0B00)
#define CT8K_APB_ADDR_SAR		(CT8K_APB_BASE_ADDRESS + 0x0C00)


static MT_FE_RET _mt_fe_i2c_write(U8 dev_addr, U8 *w_buf, U16 w_byte)
{
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;
	int ret = 0;

	i2c = i2c_get_adapter(g_i2c_ct8k);
	msg->addr = dev_addr;
	msg->flags = I2C_M_TEN;
	msg->buf = w_buf;
	msg->len = w_byte;
	mt_msg.rlen = 0;
	mt_msg.wlen = w_byte;
	mt_msg.slave_type = I2C_SLAVE_DEV_SOC_INTER_DEVICE1;
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

	i2c = i2c_get_adapter(g_i2c_ct8k);
	//printk("[%s ] line:%d i2c_bus 0x%08x\n", __func__, __LINE__, i2c);
	msg->addr = dev_addr;
	msg->flags = I2C_M_STD_RD;
	//msg->buf = r_buf;
	msg->buf = buffer;
	msg->len = r_byte;
	mt_msg.rlen = r_byte;
	mt_msg.wlen = w_byte;
	mt_msg.slave_type = I2C_SLAVE_DEV_SOC_INTER_DEVICE1;
	ret = i2c_transfer(i2c, msg, 1);
	if (ret < 0)
	{
		//printk("_mt_fe_i2c_read() failed, dev_addr = 0x%02x, ret %d\n", dev_addr, ret);
		return MtFeErr_I2cErr;
	}

	memcpy(r_buf, buffer, r_byte);

	return MtFeErr_Ok;
}


static MT_FE_RET _mt_fe_i2c_tn_write(U8 dev_addr, U8 *w_buf, U16 w_byte)
{
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;
	int ret = 0;

	i2c = i2c_get_adapter(g_i2c_ct8k);
	msg->addr = dev_addr;
	msg->flags = I2C_M_TEN;
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

static MT_FE_RET _mt_fe_i2c_tn_read(U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
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

	i2c = i2c_get_adapter(g_i2c_ct8k);
	//printk("[%s ] line:%d i2c_bus 0x%08x\n", __func__, __LINE__, i2c);
	msg->addr = dev_addr;
	msg->flags = I2C_M_SEQ_RD;
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


void _mt_sleep_ct8k(U32 ms)
{
	/*
		TODO:
			Delay ms.
	*/

	msleep(ms);
}


void _mt_delay_ct8k(U32 ms)
{
	/*
		TODO:
			Delay ms.
	*/

	//mdelay(ms);
	usleep_range(ms * 1000, ms * 1000 + 500);
}


MT_FE_RET _mt_fe_dmd_write_ct8k(U8 dev_addr, U8 *p_buf, U16 n_byte)
{
#if (CT8K_APB_I2C == 1)
	MT_FE_RET ret = MtFeErr_Ok;
	U8 reg_index = p_buf[0], data_index = 0;
	U32 ulAddr = 0, ulData;

	switch(dev_addr & 0xF8)
	{
		case 0xb8:		// J83.B
			ulAddr = 0xBF5B0000 + (0x00 << 8);	// page 0
			break;

		case 0x80:		// SAR
			ulAddr = 0xBF5B0000 + (0x0C << 8);
			break;

		case 0xd0:		// S/S2/S2X
			ulAddr = 0xBF5B0000 + (0x09 << 8);
			break;

		case 0x38:		// C / T
			ulAddr = 0xBF5B0000 + (0x0A << 8);	// T
			break;

		case 0x18:		// Sys / T2
		default:
			ulAddr = 0xBF5B0000 + (0x0B << 8);
			break;
	}

	data_index = 1;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	while(data_index < n_byte)
	{
		U32 ulData = 0;
		//int i = 0;

		ulData = 0;
		ulData |= p_buf[data_index];

		writeb(p_buf[data_index], (volatile void __iomem *)(ulAddr | (reg_index & 0xFF)));

		data_index ++;

		if((dev_addr & 0xF8) != 0x18)		// DVB-T2 firmware
		{
			if(data_index >= n_byte)
			{
				break;
			}

			ulData = 0;
			ulData |= p_buf[data_index];

			writeb(p_buf[data_index], (volatile void __iomem *)(ulAddr | ((reg_index+ 1) & 0xFF)));

			data_index ++;
		}
	}


	return ret;
#else
	MT_FE_RET ret;

	ret = _mt_fe_i2c_write(dev_addr, p_buf, n_byte);

	if (ret != MtFeErr_Ok)
	{
		printk("MT:  _mt_fe_dmd_write_ct8k() FAILED!\n");
		return MtFeErr_I2cErr;
	}

	return MtFeErr_Ok;
#endif
}


MT_FE_RET _mt_fe_ct8k_set_reg(U8 dev_addr, U8 reg_addr, U8 data)
{
#if (CT8K_APB_I2C == 1)
	MT_FE_RET ret = MtFeErr_Ok;
	U32 ulAddr = 0;
	U32 ulData = 0;
	//int i;

	switch(dev_addr & 0xF8)
	{
		case 0xb8:		// J83.B
			ulAddr = 0xBF5B0000 + (0x00 << 8);	// page 0
			break;

		case 0x80:		// SAR
			ulAddr = 0xBF5B0000 + (0x0C << 8);
			break;

		case 0xd0:		// S/S2/S2X
			ulAddr = 0xBF5B0000 + (0x09 << 8);
			break;

		case 0x38:		// C / T
			ulAddr = 0xBF5B0000 + (0x0A << 8);	// T
			break;

		case 0x18:		// Sys / T2
		default:
			ulAddr = 0xBF5B0000 + (0x0B << 8);
			break;
	}


	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	ulData = 0;
	ulData |= data;

#if 0
	for(i = 0; i < 3; i ++)
	{
		ulData <<= 8;
		ulData |= data;
	}
#endif

	writeb(data, (volatile void __iomem *)(ulAddr | (reg_addr & 0xFF)));

	return ret;
#else
	MT_FE_RET ret;
	U8 buf[2];

	buf[0] = reg_addr;
	buf[1] = data;

	//printk("[%s ] line:%d\n", __func__, __LINE__);

	ret = _mt_fe_i2c_write(dev_addr, buf, (U16)2);

	if (ret != MtFeErr_Ok)
	{
		printk("MT:  _mt_fe_ct8k_set_reg() dev_addr = 0x%02x, reg 0x%02x - data 0x%02x FAILED!\n", dev_addr, reg_addr, data);
		return MtFeErr_I2cErr;
	}
#if 0
	else
	{
		printk("_mt_fe_ct8k_set_reg() dev_addr = 0x%02x, reg 0x%02x - data 0x%02x\n", dev_addr, reg_addr, data);
	}
#endif

	return MtFeErr_Ok;
#endif
}

MT_FE_RET _mt_fe_ct8k_get_reg(U8 dev_addr, U8 reg_addr, U8 *reg_data)
{
#if (CT8K_APB_I2C == 1)
	MT_FE_RET ret = MtFeErr_Ok;
	U32 ulAddr = 0;
	U32 ulData = 0;
	//int i;

	switch(dev_addr & 0xF8)
	{
		case 0xb8:		// J83.B
			ulAddr = 0xBF5B0000 + (0x00 << 8);	// page 0
			break;

		case 0x80:		// SAR
			ulAddr = 0xBF5B0000 + (0x0C << 8);
			break;

		case 0xd0:		// S/S2/S2X
			ulAddr = 0xBF5B0000 + (0x09 << 8);
			break;

		case 0x38:		// C / T
			ulAddr = 0xBF5B0000 + (0x0A << 8);	// T
			break;

		case 0x18:		// Sys / T2
		default:
			ulAddr = 0xBF5B0000 + (0x0B << 8);
			break;
	}

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	ulData = readb((volatile void __iomem *)(ulAddr | (reg_addr & 0xFF)));

	//printk("%s() %d: read 0x%08x = 0x%08x\n", __FUNCTION__, __LINE__, ulAddr, ulData);

	*reg_data = (U8)ulData;

	return ret;
#else
	MT_FE_RET ret;
	U8 temp[2] = {0, 0};

	//printk("[%s ] line:%d\n", __func__, __LINE__);

	temp[0] = (U8)reg_addr;
	ret = _mt_fe_i2c_read(dev_addr, temp, (U16)(1), reg_data, (U16)(1));

	if (ret != MtFeErr_Ok)
	{
		printk("MT:	_mt_fe_ct8k_get_reg() dev_addr = 0x%02x, reg 0x%02x FAILED!\n", dev_addr, reg_addr);
		return MtFeErr_I2cErr;
	}
#if 0
	else
	{
		printk("_mt_fe_ct8k_get_reg() dev_addr = 0x%02x, reg 0x%02x - data 0x%02x\n", dev_addr, reg_addr, *reg_data);
	}
#endif

	return MtFeErr_Ok;
#endif
}


MT_FE_RET _mt_fe_dmd_write_fw_ct8k(U8 dev_addr, U8 reg_addr, U8 *p_buf, U16 n_byte)
{
	U8 fw_data[1024];

	fw_data[0] = reg_addr;
	memcpy(&fw_data[1], p_buf, n_byte);

	return _mt_fe_dmd_write_ct8k(dev_addr, fw_data, (U16)(n_byte + 1));
}


MT_FE_RET mt_fe_tn_write_ct8k(U8 dev_addr, U8 *p_buf, U8 n_byte)
{
	MT_FE_RET ret;

	ret = _mt_fe_i2c_tn_write(dev_addr, p_buf, n_byte);

	if (ret != MtFeErr_Ok)
	{
		printk("MT:  mt_fe_tn_write_ct8k() FAILED!\n");
		return MtFeErr_I2cErr;
	}
#if 0
	else
	{
		printk("mt_fe_tn_write_ct8k() dev_addr = 0x%02x, reg 0x%02x - data 0x%02x\n", dev_addr, p_buf[0], p_buf[1]);
	}
#endif

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_read_ct8k(U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
{
	MT_FE_RET ret;

	ret = _mt_fe_i2c_tn_read(dev_addr, w_buf, w_byte, r_buf, r_byte);

	if (ret != MtFeErr_Ok)
	{
		printk("MT:	mt_fe_tn_read_ct8k reg() FAILED!\n");
		return MtFeErr_I2cErr;
	}
#if 0
	else
	{
		printk("mt_fe_tn_read_ct8k() dev_addr = 0x%02x, reg 0x%02x - data 0x%02x\n", dev_addr, w_buf[0], r_buf[0]);
	}
#endif

	return MtFeErr_Ok;
}


MT_FE_RET _mt_fe_write32_ct8k(U32 reg_addr, U32 reg_data)
{
	writel(reg_data, (volatile void __iomem *)reg_addr);

	return MtFeErr_Ok;
}


MT_FE_RET _mt_fe_read32_ct8k(U32 reg_addr, U32 *p_data)
{
	*p_data = readl((volatile void __iomem *)reg_addr);

	return MtFeErr_Ok;
}


MT_FE_RET _mt_fe_dmd_get_reg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = 0;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	ulData = readb((volatile void __iomem *)(CT8K_APB_ADDR_SS2 + reg_addr));

	//printk("%s() %d: read 0x%08x = 0x%08x\n", __FUNCTION__, __LINE__, CT8K_APB_ADDR_SS2 + reg_addr, ulData);

	*p_data = (U8)ulData;

	return MtFeErr_Ok;
#else
	return _mt_fe_ct8k_get_reg(handle->m_device_ss2.demod_dev_addr, reg_addr, p_data);
#endif
}

MT_FE_RET _mt_fe_dmd_set_reg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = reg_data;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	//hal_put_u32((volatile unsigned long *)(CT8K_APB_ADDR_SS2 + reg_addr), ulData);
	writeb(reg_data, (volatile void __iomem *)(CT8K_APB_ADDR_SS2 + reg_addr));

	return MtFeErr_Ok;
#else
	return _mt_fe_ct8k_set_reg(handle->m_device_ss2.demod_dev_addr, reg_addr, reg_data);
#endif
}

MT_FE_RET _mt_fe_tn_get_reg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data)
{
	if(!handle->bTunerBusOn)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
	}

	//return _mt_fe_ct8k_get_reg(handle->m_device_ss2.tuner_cfg.tuner_dev_addr, reg_addr, p_data);
	return mt_fe_tn_read_ct8k(handle->m_device_ss2.tuner_cfg.tuner_dev_addr, &reg_addr, 1, p_data, 1);
}

MT_FE_RET _mt_fe_tn_set_reg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
	U8 p_buf[2] = {0, 0};

	p_buf[0] =  reg_addr;
	p_buf[1] =  reg_data;

	if(!handle->bTunerBusOn)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
	}

	//return _mt_fe_ct8k_set_reg(handle->m_device_ss2.tuner_cfg.tuner_dev_addr, reg_addr, reg_data);
	return mt_fe_tn_write_ct8k(handle->m_device_ss2.tuner_cfg.tuner_dev_addr, p_buf, 2);
}

MT_FE_RET _mt_fe_tn_write_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *p_buf, U16 n_byte)
{
	if(!handle->bTunerBusOn)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
	}

	//return _mt_fe_dmd_write_ct8k(handle->m_device_ss2.tuner_cfg.tuner_dev_addr, p_buf, 2);
	return mt_fe_tn_write_ct8k(handle->m_device_ss2.tuner_cfg.tuner_dev_addr, p_buf, (U8)n_byte);
}

MT_FE_RET _mt_fe_dmd_get_reg_t_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = 0;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	ulData = readb((volatile void __iomem *)(CT8K_APB_ADDR_DVBT + reg_addr));

	//printk("%s() %d: read 0x%08x = 0x%08x\n", __FUNCTION__, __LINE__, CT8K_APB_ADDR_DVBT + reg_addr, ulData);

	*p_data = (U8)ulData;

	return MtFeErr_Ok;
#else
	return _mt_fe_ct8k_get_reg(handle->m_device_ctt2.dmd_dev_addr, reg_addr, p_data);
#endif
}

MT_FE_RET _mt_fe_dmd_set_reg_t_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = reg_data;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	//hal_put_u32((volatile unsigned long *)(CT8K_APB_ADDR_SS2 + reg_addr), ulData);
	writeb(reg_data, (volatile void __iomem *)(CT8K_APB_ADDR_DVBT + reg_addr));

	return MtFeErr_Ok;
#else
	return _mt_fe_ct8k_set_reg(handle->m_device_ctt2.dmd_dev_addr, reg_addr, reg_data);
#endif
}

MT_FE_RET _mt_fe_dmd_get_reg_t2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = 0;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	ulData = readb((volatile void __iomem *)(CT8K_APB_ADDR_DVBT2 + reg_addr));

	//printk("%s() %d: read 0x%08x = 0x%08x\n", __FUNCTION__, __LINE__, CT8K_APB_ADDR_DVBT2 + reg_addr, ulData);

	*p_data = (U8)ulData;

	return MtFeErr_Ok;
#else
#if 0
	MT_FE_RET ret = MtFeErr_Ok;

	ret = _mt_fe_ct8k_get_reg(handle->sys_dev_addr, reg_addr, p_data);
	printk("%s() %d: 0x%02x = 0x%02x\n", __FUNCTION__, __LINE__, reg_addr, p_data[0]);

	return ret;
#else
	return _mt_fe_ct8k_get_reg(handle->sys_dev_addr, reg_addr, p_data);
#endif
#endif
}

MT_FE_RET _mt_fe_dmd_set_reg_t2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = reg_data;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	//hal_put_u32((volatile unsigned long *)(CT8K_APB_ADDR_SS2 + reg_addr), ulData);
	writeb(reg_data, (volatile void __iomem *)(CT8K_APB_ADDR_DVBT2 + reg_addr));

	return MtFeErr_Ok;
#else
#if 0
	MT_FE_RET ret = MtFeErr_Ok;

	ret = _mt_fe_ct8k_set_reg(handle->sys_dev_addr, reg_addr, reg_data);
	printk("%s() %d: 0x%02x = 0x%02x\n", __FUNCTION__, __LINE__, reg_addr, reg_data);

	return ret;
#else
	return _mt_fe_ct8k_set_reg(handle->sys_dev_addr, reg_addr, reg_data);
#endif
#endif
}

MT_FE_RET _mt_fe_dmd_get_reg_b_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = 0;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	ulData = readb((volatile void __iomem *)(CT8K_APB_ADDR_J83B + reg_addr));

	//printk("%s() %d: read 0x%08x = 0x%08x\n", __FUNCTION__, __LINE__, CT8K_APB_ADDR_J83B + reg_addr, ulData);

	*p_data = (U8)ulData;

	return MtFeErr_Ok;
#else
	return _mt_fe_ct8k_get_reg(handle->m_device_ctt2.dmd_dev_addr, reg_addr, p_data);
#endif
}

MT_FE_RET _mt_fe_dmd_set_reg_b_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = reg_data;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	//hal_put_u32((volatile unsigned long *)(CT8K_APB_ADDR_SS2 + reg_addr), ulData);
	writeb(reg_data, (volatile void __iomem *)(CT8K_APB_ADDR_J83B + reg_addr));

	return MtFeErr_Ok;
#else
	return _mt_fe_ct8k_set_reg(handle->m_device_ctt2.dmd_dev_addr, reg_addr, reg_data);
#endif
}

MT_FE_RET _mt_fe_dmd_get_page_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U16 reg_addr, U8 *p_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = 0;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	ulData = readb((volatile void __iomem *)(CT8K_APB_ADDR_J83B + reg_addr));

	//printk("%s() %d: read 0x%08x = 0x%08x\n", __FUNCTION__, __LINE__, CT8K_APB_ADDR_J83B + reg_addr, ulData);

	*p_data = (U8)ulData;

	return MtFeErr_Ok;
#else
	U8 page_no = (U8)(reg_addr >> 8);
	U8 reg_index = (U8)(reg_addr & 0xFF);

	if(page_no != handle->m_device_ctt2.m_iPageNo)
	{
		_mt_fe_dmd_set_reg_b_ct8k(handle, 0xFE, page_no);
		handle->m_device_ctt2.m_iPageNo = page_no;
	}

	return _mt_fe_dmd_get_reg_b_ct8k(handle, reg_index, p_data);
#endif
}

MT_FE_RET _mt_fe_dmd_set_page_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U16 reg_addr, U8 reg_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = reg_data;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	//hal_put_u32((volatile unsigned long *)(CT8K_APB_ADDR_SS2 + reg_addr), ulData);
	writeb(reg_data, (volatile void __iomem *)(CT8K_APB_ADDR_J83B + reg_addr));

	return MtFeErr_Ok;
#else
	U8 page_no = (U8)(reg_addr >> 8);
	U8 reg_index = (U8)(reg_addr & 0xFF);

	if(page_no != handle->m_device_ctt2.m_iPageNo)
	{
		_mt_fe_dmd_set_reg_b_ct8k(handle, 0xFE, page_no);
		handle->m_device_ctt2.m_iPageNo = page_no;
	}

	return _mt_fe_dmd_set_reg_b_ct8k(handle, reg_index, reg_data);
#endif
}

MT_FE_RET _mt_fe_dmd_get_reg_c_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = 0;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	ulData = readb((volatile void __iomem *)(CT8K_APB_ADDR_DVBC + reg_addr));

	//printk("%s() %d: read 0x%08x = 0x%08x\n", __FUNCTION__, __LINE__, CT8K_APB_ADDR_DVBC + reg_addr, ulData);

	*p_data = (U8)ulData;

	return MtFeErr_Ok;
#else
	return _mt_fe_ct8k_get_reg(handle->m_device_ctt2.dmd_dev_addr, reg_addr, p_data);
#endif
}

MT_FE_RET _mt_fe_dmd_set_reg_c_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = reg_data;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	//hal_put_u32((volatile unsigned long *)(CT8K_APB_ADDR_SS2 + reg_addr), ulData);
	writeb(reg_data, (volatile void __iomem *)(CT8K_APB_ADDR_DVBC + reg_addr));

	return MtFeErr_Ok;
#else
	return _mt_fe_ct8k_set_reg(handle->m_device_ctt2.dmd_dev_addr, reg_addr, reg_data);
#endif
}

MT_FE_RET _mt_fe_sar_get_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = 0;

	ulData = readb((volatile void __iomem *)(CT8K_APB_ADDR_SAR + reg_addr));

	//printk("%s() %d: read 0x%08x = 0x%08x\n", __FUNCTION__, __LINE__, CT8K_APB_ADDR_SAR + reg_addr, ulData);

	*p_data = (U8)ulData;

	return MtFeErr_Ok;
#else
	return _mt_fe_ct8k_get_reg(handle->sar_dev_addr, reg_addr, p_data);
#endif
}

MT_FE_RET _mt_fe_sar_set_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
#if (CT8K_APB_I2C == 1)
	//U32 ulData = reg_data;

	//hal_put_u32((volatile unsigned long *)(CT8K_APB_ADDR_SS2 + reg_addr), ulData);
	writeb(reg_data, (volatile void __iomem *)(CT8K_APB_ADDR_SAR + reg_addr));

	return MtFeErr_Ok;
#else
	return _mt_fe_ct8k_set_reg(handle->sar_dev_addr, reg_addr, reg_data);
#endif
}

MT_FE_RET _mt_fe_dmd_get_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = 0;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	ulData = readb((volatile void __iomem *)(CT8K_APB_ADDR_DVBT2 + reg_addr));

	*p_data = (U8)ulData;

	return MtFeErr_Ok;
#else
#if 0
	MT_FE_RET ret = MtFeErr_Ok;

	ret = _mt_fe_ct8k_get_reg(handle->sys_dev_addr, reg_addr, p_data);
	printk("%s() %d: 0x%02x = 0x%02x\n", __FUNCTION__, __LINE__, reg_addr, p_data[0]);

	return ret;
#else
	return _mt_fe_ct8k_get_reg(handle->sys_dev_addr, reg_addr, p_data);
#endif
#endif
}

MT_FE_RET _mt_fe_dmd_set_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
#if (CT8K_APB_I2C == 1)
	U32 ulData = reg_data;

	ulData = readb((volatile void __iomem *)0xBF5B0C00);

	//hal_put_u32((volatile unsigned long *)(CT8K_APB_ADDR_SS2 + reg_addr), ulData);
	writeb(reg_data, (volatile void __iomem *)(CT8K_APB_ADDR_DVBT2 + reg_addr));

	return MtFeErr_Ok;
#else
#if 0
	MT_FE_RET ret = MtFeErr_Ok;

	ret = _mt_fe_ct8k_set_reg(handle->sys_dev_addr, reg_addr, reg_data);
	printk("%s() %d: 0x%02x = 0x%02x\n", __FUNCTION__, __LINE__, reg_addr, reg_data);

	return ret;
#else
	return _mt_fe_ct8k_set_reg(handle->sys_dev_addr, reg_addr, reg_data);
#endif
#endif
}

MT_FE_RET _mt_fe_sys_set_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_index, U8 reg_data)
{
	MT_FE_RET ret = MtFeErr_Ok;

	ret = _mt_fe_ct8k_set_reg(handle->sys_dev_addr, 0x1E, reg_index);
	ret |= _mt_fe_ct8k_set_reg(handle->sys_dev_addr, 0x1F, reg_data);

	return ret;
}

MT_FE_RET _mt_fe_sys_get_reg_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_index, U8 *p_data)
{
	MT_FE_RET ret = MtFeErr_Ok;

	ret = _mt_fe_ct8k_set_reg(handle->sys_dev_addr, 0x1E, reg_index);
	ret |= _mt_fe_ct8k_get_reg(handle->sys_dev_addr, 0x1F, p_data);

	return ret;
}

MT_FE_RET _mt_fe_tn_get_reg_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *reg_data)
{
	if(!handle->bTunerBusOn)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
	}

	return mt_fe_tn_read_ct8k(handle->m_device_ctt2.tuner_cfg.tuner_dev_addr, &reg_addr, 1, reg_data, 1);
}

MT_FE_RET _mt_fe_tn_set_reg_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
	U8 p_buf[2] = {0, 0};

	p_buf[0] =  reg_addr;
	p_buf[1] =  reg_data;

	if(!handle->bTunerBusOn)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
	}

	return mt_fe_tn_write_ct8k(handle->m_device_ctt2.tuner_cfg.tuner_dev_addr, p_buf, 2);
}


MT_FE_RET mt_fe_i2c_repeat_enable_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;
#if 0
	U8	data = 0;

	ret =  _mt_fe_dmd_get_reg_ct8k(handle, 0x03, &data);
	data = (U8)(data | 0x10);
	ret =  _mt_fe_dmd_set_reg_ct8k(handle, 0x03, data);
#endif
	handle->bTunerBusOn = TRUE;

	return ret;
}

MT_FE_RET mt_fe_i2c_repeat_disable_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;
#if 0
	U8	data = 0;

	ret =  _mt_fe_dmd_get_reg_ct8k(handle, 0x03, &data);
	data = (U8)(data & 0xef);
	ret =  _mt_fe_dmd_set_reg_ct8k(handle, 0x03, data);
#endif
	handle->bTunerBusOn = FALSE;

	return ret;
}

