/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/io.h>

#include "mt_fe_def_cs8000_sat.h"
#include "mt_fe_sat_tn_montage_ts6011.h"
#include "port_m88cs8k_sat.h"
#include "mt_module_debug.h"

extern int g_i2c_cs8k_sat;

MT_FE_RET _mt_fe_lnb_i2c_read(U8 i2c_id, U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
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
    struct mt_i2c_msg mt_msg = { 0, };
    struct i2c_msg *msg = &mt_msg.msg;
    struct i2c_adapter *i2c = NULL;
    int ret = 0;

    //*r_buf = *w_buf;

    i2c = i2c_get_adapter((int)i2c_id);
    //printk("[%s ] line:%d i2c_bus 0x%08x\n", __func__, __LINE__, i2c);
    msg->addr = dev_addr;
    msg->flags = I2C_M_RD | I2C_M_SALVE_TYPE;
    msg->buf = r_buf;
    msg->len = r_byte;
    mt_msg.rlen = r_byte;
    mt_msg.wlen = w_byte;
    mt_msg.slave_type = I2C_SLAVE_DEV_SOC_EXTER_DEVICE1;
    ret = i2c_transfer(i2c, msg, 1);
    if (ret < 0) {
	MT_ERR_FRONTEND("_mt_fe_lnb_i2c_read failed, ret %d", ret);
	return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_lnb_i2c_write(U8 i2c_id, U8 dev_addr, U8 *w_buf, U16 w_byte)
{
    struct mt_i2c_msg mt_msg = { 0, };
    struct i2c_msg *msg = &mt_msg.msg;
    struct i2c_adapter *i2c = NULL;
    int ret = 0;

    i2c = i2c_get_adapter((int)i2c_id);
    msg->addr = dev_addr;
    msg->flags = I2C_M_TEN | I2C_M_SALVE_TYPE;
    msg->buf = w_buf;
    msg->len = w_byte;
    mt_msg.rlen = 0;
    mt_msg.wlen = w_byte;
    mt_msg.slave_type = I2C_SLAVE_DEV_SOC_EXTER_DEVICE1;
    ret = i2c_transfer(i2c, msg, 1);
    if (ret < 0) {
	MT_ERR_FRONTEND("_mt_fe_lnb_i2c_write failed, ret %d", ret);
	return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_i2c_write(U8 dev_addr, U8 *w_buf, U16 w_byte)
{
    struct mt_i2c_msg mt_msg = { 0, };
    struct i2c_msg *msg = &mt_msg.msg;
    struct i2c_adapter *i2c = NULL;
    int ret = 0;

    i2c = i2c_get_adapter(g_i2c_cs8k_sat);
    msg->addr = dev_addr;
    msg->flags = I2C_M_TEN | I2C_M_SALVE_TYPE;
    msg->buf = w_buf;
    msg->len = w_byte;
    mt_msg.rlen = 0;
    mt_msg.wlen = w_byte;
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
    mt_msg.slave_type = I2C_SLAVE_DEV_SOC_INTER;
#else
    mt_msg.slave_type = I2C_SLAVE_DEV_SOC_INTER_DEVICE1;
#endif
    ret = i2c_transfer(i2c, msg, 1);
    if (ret < 0) {
	MT_ERR_FRONTEND("_mt_fe_i2c_write failed, ret %d", ret);
	return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_i2c_std_read(U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
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
    struct mt_i2c_msg mt_msg = { 0, };
    struct i2c_msg *msg = &mt_msg.msg;
    struct i2c_adapter *i2c = NULL;
    int ret = 0;
    U8 buffer[32];

    //*r_buf = *w_buf;
    memcpy(buffer, w_buf, w_byte);

    i2c = i2c_get_adapter(g_i2c_cs8k_sat);
    //printk("[%s ] line:%d i2c_bus 0x%08x\n", __func__, __LINE__, i2c);
    msg->addr = dev_addr;
    msg->flags = I2C_M_STD_RD | I2C_M_SALVE_TYPE;
    //msg->buf = r_buf;
    msg->buf = buffer;
    msg->len = r_byte;
    mt_msg.rlen = r_byte;
    mt_msg.wlen = w_byte;
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
    mt_msg.slave_type = I2C_SLAVE_DEV_SOC_INTER;
#else
    mt_msg.slave_type = I2C_SLAVE_DEV_SOC_INTER_DEVICE1;
#endif
    ret = i2c_transfer(i2c, msg, 1);
    if (ret < 0) {
	MT_ERR_FRONTEND("_mt_fe_i2c_read failed, ret %d", ret);
	return MtFeErr_I2cErr;
    }

    memcpy(r_buf, buffer, r_byte);

    return MtFeErr_Ok;
}

static MT_FE_RET _cs8k_sat_dmd_set_reg_no_lock(MT_FE_CS8000_SAT_Device_Handle handle, U8 reg_index, U8 data)
{
    MT_FE_RET ret;
    U8 buf[2];
    buf[0] = reg_index;
    buf[1] = data;

    ret = _mt_fe_i2c_write(handle->demod_dev_addr, buf, (U16)2);

    if (ret != MtFeErr_Ok) {
	MT_ERR_FRONTEND("MT:	_cs8k_sat_dmd_set_reg_no_lock reg %x data %x FAILED!\n", reg_index, data);
	return MtFeErr_I2cErr;
    }
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
MT_FE_RET _mt_fe_cs8k_sat_dmd_set_reg(MT_FE_CS8000_SAT_Device_Handle handle, U8 reg_index, U8 data)
{
    MT_FE_RET ret;
    U8 buf[2];

    buf[0] = reg_index;
    buf[1] = data;

    //printk("[%s ] line:%d\n", __func__, __LINE__);

    ret = _mt_fe_i2c_write(handle->demod_dev_addr, buf, (U16)2);

    if (ret != MtFeErr_Ok) {
	MT_ERR_FRONTEND("MT:  _mt_fe_cs8k_sat_dmd_set_reg reg %x data %x FAILED!\n", reg_index, data);
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
MT_FE_RET _mt_fe_cs8k_sat_dmd_get_reg(MT_FE_CS8000_SAT_Device_Handle handle, U8 reg_index, U8 *p_buf)
{
    MT_FE_RET ret;
    U8 temp[2] = { 0, 0 };

    //printk("[%s ] line:%d\n", __func__, __LINE__);

    temp[0] = (U8)reg_index;
    ret = _mt_fe_i2c_std_read(handle->demod_dev_addr, temp, (U16)(1), p_buf, (U16)(1));

    if (ret != MtFeErr_Ok) {
	MT_ERR_FRONTEND("MT:	_mt_fe_cs8k_dmd_get_reg reg %x FAILED!\n", reg_index);
	return MtFeErr_I2cErr;
    }

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
MT_FE_RET _mt_fe_cs8k_sat_tn_set_reg(MT_FE_CS8000_SAT_Device_Handle handle, U8 reg_index, U8 data)
{
    MT_FE_RET ret;
    U8 val;
    U8 buf[2];

    //printk("[%s ] line:%d\n", __func__, __LINE__);

    buf[0] = reg_index;
    buf[1] = data;

    val = 0x11;
    ret = _cs8k_sat_dmd_set_reg_no_lock(handle, 0x03, val);
    if (ret != MtFeErr_Ok) {
	return ret;
    }

    ret = _mt_fe_i2c_write(handle->tuner_cfg.tuner_dev_addr, buf, (U16)2);
    if (ret < 0) {
	MT_ERR_FRONTEND("_mt_fe_cs8k_tn_set_reg reg %x Failure!!!\n", reg_index);
	return MtFeErr_I2cErr;
    }

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
MT_FE_RET _mt_fe_cs8k_sat_tn_get_reg(MT_FE_CS8000_SAT_Device_Handle handle, U8 reg_index, U8 *p_buf)
{
    MT_FE_RET ret;
    U8 val;
    U8 temp[2] = { 0, 0 };

    //printk("[%s ] line:%d\n", __func__, __LINE__);

    val = 0x12;
    ret = _cs8k_sat_dmd_set_reg_no_lock(handle, 0x03, val);
    if (ret != MtFeErr_Ok) {
	return ret;
    }

    temp[0] = (U8)reg_index;
    ret = _mt_fe_i2c_std_read(handle->tuner_cfg.tuner_dev_addr, temp, (U16)(1), p_buf, (U16)(1));
    if (ret < 0) {
	MT_ERR_FRONTEND("_mt_fe_cs8k_tn_get_reg reg %x Failure!!!\n", reg_index);
	return MtFeErr_I2cErr;
    }

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
MT_FE_RET _mt_fe_cs8k_sat_write_fw(MT_FE_CS8000_SAT_Device_Handle handle, U8 reg_index, U8 *p_buf, U16 n_byte)
{
    MT_FE_RET ret;
    U8 *p_send_buf = (U8 *)kmalloc(n_byte + 1, GFP_KERNEL);

    if (p_send_buf == NULL) {
		MT_ERR_FRONTEND("MT:_mt_fe_cs8k_write_fw malloc %d FAILED!\n", n_byte);
		return MtFeErr_NoMemory;
    }

    p_send_buf[0] = reg_index;
    memcpy(p_send_buf + 1, p_buf, n_byte);
    n_byte++;

    //printk("[%s ] line:%d\n", __func__, __LINE__);

    ret = _mt_fe_i2c_write(handle->demod_dev_addr, p_send_buf, n_byte);

    kfree(p_send_buf);
    if (ret != MtFeErr_Ok) {
	MT_ERR_FRONTEND("MT:_mt_fe_cs8k_write_fw FAILED!\n");
	return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}

void _mt_cs8k_sat_sleep(U32 ticks_ms)
{
    /*TODO*/

    /*
		Wait for ticks_ms time, the time unit is millisecond.
	*/
    msleep(ticks_ms);
}

S32 _mt_fe_sat_tn_get_reg_ts6011(MT_FE_Tuner_Handle_TS6011 handle, U8 reg_addr, U8 *reg_data)
{
    MT_FE_RET ret = MtFeErr_Ok;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = g_cs8k_sat_priv->cs8k_handle;
    cs8k_handle->tn_get_reg(cs8k_handle, reg_addr, reg_data);

    return (ret == MtFeErr_Ok) ? 0 : -1;
}

S32 _mt_fe_sat_tn_set_reg_ts6011(MT_FE_Tuner_Handle_TS6011 handle, U8 reg_addr, U8 reg_data)
{
    MT_FE_RET ret = MtFeErr_Ok;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = g_cs8k_sat_priv->cs8k_handle;

    cs8k_handle->tn_set_reg(cs8k_handle, reg_addr, reg_data);

    return (ret == MtFeErr_Ok) ? 0 : -1;
}

S32 _mt_fe_sat_tn_set_reg_bit_ts6011(MT_FE_Tuner_Handle_TS6011 handle, U8 reg_addr, U8 data, U8 high_bit, U8 low_bit)
{
    U8 tmp = 0, value = 0;

    if (high_bit < low_bit) {
	tmp = high_bit;
	high_bit = low_bit;
	low_bit = tmp;
    }

    data <<= (7 + low_bit - high_bit);
    data &= 0xFF;
    data >>= (7 - high_bit);
    data &= 0xFF;

    tmp = 0xFF;
    tmp <<= (7 + low_bit - high_bit);
    tmp &= 0xFF;
    tmp >>= (7 - high_bit);
    tmp &= 0xFF;

    _mt_fe_sat_tn_get_reg_ts6011(handle, reg_addr, &value);
    value &= ~tmp;
    value |= data;
    _mt_fe_sat_tn_set_reg_ts6011(handle, reg_addr, value);

    return 0;
}

MT_FE_RET _mt_fe_cs8k_sat_lnb_write(U8 i2c_id, U8 slav_addr, U8 *p_buf, U16 n_byte)
{
    MT_FE_RET ret;

    //MT_INFO_FRONTEND("_mt_fe_cs8k_sat_lnb_write\n");

    ret = _mt_fe_lnb_i2c_write(i2c_id, slav_addr, p_buf, n_byte);
    if (ret != MtFeErr_Ok) {
	MT_ERR_FRONTEND("MT:  _mt_fe_cs8k_sat_lnb_write i2c_id %d slav_addr %x FAILED!\n", i2c_id, slav_addr);
	return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_cs8k_sat_lnb_read(U8 i2c_id, U8 slav_addr, U8 *p_buf, U16 n_byte)
{
    MT_FE_RET ret;

    //MT_INFO_FRONTEND("_mt_fe_cs8k_sat_lnb_read\n");

    ret = _mt_fe_lnb_i2c_read(i2c_id, slav_addr, NULL, 0, p_buf, n_byte);

    if (ret != MtFeErr_Ok) {
	MT_ERR_FRONTEND("MT:	_mt_fe_cs8k_sat_lnb_read slav_addr %x FAILED!\n", slav_addr);
	return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_cs8k_sat_write32(U32 reg_addr, U32 reg_data)
{
    //(volatile unsigned long *)reg_addr = reg_data;
    writel(reg_data, (volatile void __iomem *)reg_addr);
    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_cs8k_sat_read32(U32 reg_addr, U32 *p_data)
{
    //*p_data = (volatile unsigned long *)(reg_addr);
    *p_data = readl((volatile void __iomem *)reg_addr);
    return MtFeErr_Ok;
}
