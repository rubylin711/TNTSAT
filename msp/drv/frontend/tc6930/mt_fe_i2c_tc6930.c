/********************************************************************************************/
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/*
 * Filename:        mt_fe_i2c_tc6930.c
 *
 * Description:     2-wire bus functions for FE module.
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
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/io.h>

#include "port_tc6930.h"


#include "mt_fe_common_tc6930.h"
#include "mt_fe_def_tc6930.h"
#include "mt_fe_i2c_tc6930.h"


#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif


extern int g_i2c_tc6930;


static MT_FE_RET _mt_fe_i2c_write(void *dev_handle,U8 dev_addr, U8 *w_buf, U16 w_byte)
{
	//MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;
	int ret = 0;

	i2c = i2c_get_adapter(g_i2c_tc6930);
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

static MT_FE_RET _mt_fe_i2c_read(void *dev_handle,U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
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
	//MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;
	int ret = 0;
	U8 buffer[32];

	//*r_buf = *w_buf;
	memcpy(buffer, w_buf, w_byte);

	i2c = i2c_get_adapter(g_i2c_tc6930);
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


MT_FE_RET _mt_fe_sym1_dmd_set_reg(U8 dev_addr, U8 reg_index, U8 data)
{
    int ret = 0;
    struct mt_i2c_msg mt_msg = {0,};
    struct i2c_msg *msg = &mt_msg.msg;
    unsigned char buf[2];
    struct i2c_adapter *i2c = NULL;

    i2c = i2c_get_adapter(g_i2c_tc6930);
    msg->addr = dev_addr;
    msg->flags = I2C_M_TEN | I2C_M_SALVE_TYPE;
    msg->buf = buf;
    msg->len = 2;
    msg->buf[0] = reg_index;
    msg->buf[1] = data;
    mt_msg.slave_type = I2C_SLAVE_DEV_SOC_INTER_DEVICE2;
    //mtos_sem_take(&reg_rw_mutex, 0);
    ret = i2c_transfer(i2c, msg, 1);
    //mtos_sem_give(&reg_rw_mutex);
    if (ret < 0)
	{
		printk("ret = %d dev_addr = %02x, reg = %02x\n", ret, dev_addr, reg_index);
		return MtFeErr_I2cErr;
    }

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
MT_FE_RET _mt_fe_sym1_dmd_get_reg(U8 dev_addr, U8 reg_index, U8 *p_buf)
{
    int ret = 0;
    struct mt_i2c_msg mt_msg = {0,};
    struct i2c_msg *msg = &mt_msg.msg;
    unsigned char buf[2];
    struct i2c_adapter *i2c = NULL;

    i2c = i2c_get_adapter(g_i2c_tc6930);
    msg->addr = dev_addr;
    msg->flags = I2C_M_STD_RD | I2C_M_SALVE_TYPE;
    msg->buf = buf;
    msg->len = 1;
    mt_msg.rlen = 1;
    mt_msg.wlen = 1;
    msg->buf[0] = reg_index;
    mt_msg.slave_type = I2C_SLAVE_DEV_SOC_INTER_DEVICE2;
    //mtos_sem_take(&reg_rw_mutex, 0);
    ret = i2c_transfer(i2c, msg, 1);
    //mtos_sem_give(&reg_rw_mutex);
    p_buf[0] = msg->buf[0];
    if (ret < 0)
	{
		printk("ret = %d dev_addr = %02x, reg = %02x\n",ret, dev_addr, reg_index);
		return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}


#define CS8800_APB_BASE_ADDRESS	(mt_get_demod_base())
#define CS8800_APB_ADDR_J83B	(CS8800_APB_BASE_ADDRESS + 0x0000)
#define CS8800_APB_ADDR_DVBC	(CS8800_APB_BASE_ADDRESS + 0x0800)
#define CS8800_APB_ADDR_SS2		(CS8800_APB_BASE_ADDRESS + 0x0900)
#define CS8800_APB_ADDR_SAR		(CS8800_APB_BASE_ADDRESS + 0x0C00)

MT_FE_RET _mt_fe_sym4_sar_get_reg(U8 reg_addr, U8 *p_data)
{
	*p_data = HAL_GET_U8((volatile u8 *)(CS8800_APB_ADDR_SAR + reg_addr));

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_sym4_sar_set_reg(U8 reg_addr, U8 reg_data)
{
	HAL_PUT_U8((volatile u8 *)(CS8800_APB_ADDR_SAR + reg_addr), reg_data);

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_sym4_c_dmd_get_reg(U8 reg_addr, U8 *p_data)
{
	U8 tmp = 0;

	tmp = HAL_GET_U8((volatile u8 *)CS8800_APB_ADDR_SAR);

	*p_data = HAL_GET_U8((volatile u8 *)(CS8800_APB_ADDR_DVBC + reg_addr));

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_sym4_c_dmd_set_reg(U8 reg_addr, U8 reg_data)
{
	U8 tmp = 0;

	tmp = HAL_GET_U8((volatile u8 *)CS8800_APB_ADDR_SAR);

	HAL_PUT_U8((volatile u8 *)(CS8800_APB_ADDR_DVBC + reg_addr), reg_data);

	return MtFeErr_Ok;
}

// Basic I2C functions

// Read several bytes
// chipAddr	 ---- Slave chip address of I2C device
// pData	 ---- Pointer to store the read data
// ulLength	 ---- Bytes to read
MT_BOOL I2cRead(void *dev_handle,U8 chipAddr, U8 *pData, U32 ulLength)
{
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	_mt_fe_i2c_read(handle,chipAddr, pData, 0, pData, (U16)ulLength);

	return TRUE;
}


// Write ulLength bytes
// chipAddr	 ---- Slave chip address of I2C device
// pData	 ---- Pointer of data to be written
// ulLength	 ---- Bytes to read
MT_BOOL I2cWrite(void *dev_handle,U8 chipAddr, U8 *pData, U32 ulLength)
{
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	_mt_fe_i2c_write(handle,chipAddr, pData, (U16)ulLength);

	return TRUE;
}


// About TC6930 Debug I2C

#define TC6930_ID				0xAC // TC6930's I2C debug port slave ID.


// Read 32-bit data from 32-bit address
U32 TC6930_ReadChipReg32(void *dev_handle,U32 RegAddress)
{
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	U8	ReadData[4];
	U8	WriteData[5];
	U8	ucByte;
	U32	unResult;

	//RegAddress &= 0x1fffffff;

	// Special flag for write operation.
	WriteData[0] = 0x1e;

	// Write address, MSB first.
	ucByte = (U8)(RegAddress >> 24);
	WriteData[1] = ucByte;

	ucByte = (U8)(RegAddress >> 16);
	WriteData[2] = ucByte;

	ucByte = (U8)(RegAddress >> 8);
	WriteData[3] = ucByte;

	ucByte = (U8)(RegAddress >> 0);
	WriteData[4] = ucByte;

	//_mt_i2c_tx_with_stop(SLAVE_ID, WriteData, 5, 0);
	I2cWrite(handle,TC6930_ID, WriteData, 5);

	// Read the data out.

	//_mt_i2c_rx_with_stop(SLAVE_ID, ReadData, 4, 0);
	I2cRead(handle,TC6930_ID, ReadData, 4);

	// Return.
	unResult  = 0xff000000 & (((U32)ReadData[0]) << 24);
	unResult |= 0x00ff0000 & (((U32)ReadData[1]) << 16);
	unResult |= 0x0000ff00 & (((U32)ReadData[2]) << 8);
	unResult |= 0x000000ff & (((U32)ReadData[3]) << 0);

	return unResult;
}


// Write 32-bit data to 32-bit address
void TC6930_WriteChipReg32(void *dev_handle,U32 RegAddress, U32 WData)
{
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	U8	WriteData[9];
	U8	ucByte;

	//RegAddress &= 0x1fffffff;

	// Special flag for write operation.
	WriteData[0] = 0x1f;

	// Write address, MSB first.
	ucByte = (U8)(RegAddress >> 24);
	WriteData[1] = ucByte;

	ucByte = (U8)(RegAddress >> 16);
	WriteData[2] = ucByte;

	ucByte = (U8)(RegAddress >> 8);
	WriteData[3] = ucByte;

	ucByte = (U8)(RegAddress >> 0);
	WriteData[4] = ucByte;

	// Write data.
	ucByte = (U8)(WData >> 24);
	WriteData[5] = ucByte;

	ucByte = (U8)(WData >> 16);
	WriteData[6] = ucByte;

	ucByte = (U8)(WData >> 8);
	WriteData[7] = ucByte;

	ucByte = (U8)(WData >> 0);
	WriteData[8] = ucByte;

	I2cWrite(handle,TC6930_ID, WriteData, 9);

	return;
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
MT_FE_RET _mt_fe_dmd_set_reg_tc6930(void *dev_handle, U8 demod_index, U8 reg_index, U8 data)
{
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;

	/*
		TODO:
			Obtain the i2c mutex
	*/



	/*
		TODO:
			write data to demodulator register
	*/


	if (demod_index == 0)
		TC6930_WriteChipReg32(handle,0x003b000 + reg_index * 4, data);
	else if(demod_index == 1)
		TC6930_WriteChipReg32(handle,0x003c000 + reg_index * 4, data);
	else
	{
		demod_index = 2;

		if (handle->demod_device[demod_index].demod_type == MT_FE_DEMOD_CS8800)
		{	// Sym4
			_mt_fe_sym4_c_dmd_set_reg(reg_index, data);
		}
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
MT_FE_RET _mt_fe_dmd_get_reg_tc6930(void *dev_handle, U8 demod_index, U8 reg_index, U8 *p_buf)
{
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	U32 ulTmp = 0;

	/*
		TODO:
			Obtain the i2c mutex
	*/



	/*
		TODO:
			read demodulator register value
	*/

	if (demod_index == 0)
		ulTmp = TC6930_ReadChipReg32(handle,0x003b000 + reg_index * 4);
	else if (demod_index == 1)
		ulTmp = TC6930_ReadChipReg32(handle,0x003c000 + reg_index * 4);
	else
	{
		demod_index = 2;

		if (handle->demod_device[demod_index].demod_type == MT_FE_DEMOD_CS8800)
		{	// Sym4
			_mt_fe_sym4_c_dmd_get_reg(reg_index, p_buf);
		}
	}


	if(demod_index < 2)
		*p_buf = (U8)ulTmp;

	/*
		TODO:
			Release the i2c mutex
	*/


	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_tn_write_tc6930(void *dev_handle, U8 iIndex, U8 *p_buf, U16 n_byte)
{
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	//MT_FE_RET	ret = MtFeErr_Ok;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	/*
		TODO:
			Obtain the i2c mutex
	*/


	/*
		TODO:
			write N bytes to tuner
	*/


	/*
		TODO:
			Release the i2c mutex
	*/



	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_write32_tc6930(U32 reg_addr, U32 reg_data)
{
	HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(reg_addr), reg_data);

	return MtFeErr_Ok;
}


MT_FE_RET _mt_fe_read32_tc6930(U32 reg_addr, U32 *p_data)
{
	*p_data = HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(reg_addr));

	return MtFeErr_Ok;
}


MT_FE_RET _mt_fe_dsp_get_reg_tc6930(void *dev_handle,U8 iIndex, U8 reg_addr, U8 *reg_data)
{
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	U32 ulTmp;
	/*
		TODO:
			Obtain the i2c mutex
	*/


	/*
		TODO:
			read tuner register value
	*/

	if(iIndex == 0)
		ulTmp = TC6930_ReadChipReg32(handle,0x0038000 + reg_addr * 4);
	else if(iIndex == 1)
		ulTmp = TC6930_ReadChipReg32(handle,0x0039000 + reg_addr * 4);
	else
		ulTmp = TC6930_ReadChipReg32(handle,0x003a000 + reg_addr * 4);

	*reg_data = (U8)ulTmp;


	/*
		TODO:
			Release the i2c mutex
	*/


	return MtFeErr_Ok;
}


S32 _mt_fe_dsp_set_reg_tc6930(void *dev_handle,U8 iIndex, U8 reg_addr, U8 reg_data)
{
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	/*
		TODO:
			Obtain the i2c mutex
	*/


	/*
		TODO:
			write value to tuner register
	*/
	if(iIndex == 0)
		TC6930_WriteChipReg32(handle,0x0038000 + reg_addr * 4, reg_data);
	else if(iIndex == 1)
		TC6930_WriteChipReg32(handle,0x0039000 + reg_addr * 4, reg_data);
	else
		TC6930_WriteChipReg32(handle,0x003a000 + reg_addr * 4, reg_data);


	/*
		TODO:
			Release the i2c mutex
	*/



	return MtFeErr_Ok;
}


S32 _mt_fe_tn_get_reg_16bit_tc6930(void *dev_handle,U16 reg_addr, U16 *reg_data)
{
	U32 ulTmp;
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	/*
		TODO:
			Obtain the i2c mutex
	*/


	/*
		TODO:
			read tuner register value
	*/

	ulTmp = TC6930_ReadChipReg32(handle,reg_addr * 4 + 0x20000);

	*reg_data = (U16)ulTmp;

	/*
		TODO:
			Release the i2c mutex
	*/


	return 0;
}


S32 _mt_fe_tn_set_reg_16bit_tc6930(void *dev_handle,U16 reg_addr, U16 reg_data)
{
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	/*
		TODO:
			Obtain the i2c mutex
	*/


	/*
		TODO:
			write value to tuner register
	*/
	TC6930_WriteChipReg32(handle,reg_addr * 4 + 0x20000, reg_data);

	/*
		TODO:
			Release the i2c mutex
	*/



	return MtFeErr_Ok;
}


S32 _mt_fe_tc6930_get_reg_32bit(void *dev_handle,U32 reg_addr, U32 *reg_data)
{
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	/*
		TODO:
			Obtain the i2c mutex
	*/


	/*
		TODO:
			read tuner register value
	*/

	*reg_data = TC6930_ReadChipReg32(handle,reg_addr * 4 + 0x20000);

	/*
		TODO:
			Release the i2c mutex
	*/


	return 0;
}


S32 _mt_fe_tc6930_set_reg_32bit(void *dev_handle,U32 reg_addr, U32 reg_data)
{
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	/*
		TODO:
			Obtain the i2c mutex
	*/


	/*
		TODO:
			write value to tuner register
	*/

	TC6930_WriteChipReg32(handle,reg_addr * 4 + 0x20000, reg_data);

	/*
		TODO:
			Release the i2c mutex
	*/



	return MtFeErr_Ok;
}


S32 _mt_fe_tc6930_set_reg_32bit_bits(void *dev_handle,U32 ulAddr, U32 ulData, U8 ucBitHigh, U8 ucBitLow)
{
	MT_FE_TC6930_Device_Handle handle = (MT_FE_TC6930_Device_Handle)dev_handle;
	U32 ulTmp = 0, ulVal = 0;

	U8 ucTmp;

	if((ucBitHigh > 31) || (ucBitLow > 31))
	{
		return -1;
	}

	if(ucBitHigh < ucBitLow)
	{
		ucTmp = ucBitHigh;
		ucBitHigh = ucBitLow;
		ucBitLow = ucTmp;
	}

	ulData <<= (31 + ucBitLow - ucBitHigh);
	ulData &= 0xFFFFFFFF;
	ulData >>= (31 - ucBitHigh);
	ulData &= 0xFFFFFFFF;

	ulTmp = 0xFFFFFFFF;
	ulTmp <<= (31 + ucBitLow - ucBitHigh);
	ulTmp &= 0xFFFFFFFF;
	ulTmp >>= (31 - ucBitHigh);
	ulTmp &= 0xFFFFFFFF;

	_mt_fe_tc6930_get_reg_32bit(handle,ulAddr, &ulVal);
	ulVal &= ~ulTmp;
	ulVal |= ulData;
	_mt_fe_tc6930_set_reg_32bit(handle,ulAddr, ulVal);

	return 0;
}

void _mt_sleep_tc6930(U32 ticks_ms)
{
	if(ticks_ms < 15)
		usleep_range(ticks_ms * 1000, ticks_ms * 1000 + 500);	//mdelay(ticks_ms);
	else
		msleep(ticks_ms);
}


