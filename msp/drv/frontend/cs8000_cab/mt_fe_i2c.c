/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include "mt_fe_def_cs8000_cab.h"
#include "mt_fe_cab_tn_tc2800.h"
#include "mt_fe_cab_tn_tc3800.h"
#include "mt_fe_cab_tn_tc6800.h"
#include "mt_module_debug.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#if 0
extern int i2c_gpio_read(
                    u8 slv_addr, u8 *p_buf, u32 size, u32 param);
extern int i2c_gpio_write(
                          u8 slv_addr, u8 *p_buf, u32 size, u32 param);
extern int i2c_gpio_seq_read(
                                           u8 slv_addr,
                                           u8 *p_buf,
                                           u32 wlen,
                                           u32 rlen,
                                           u32 param);
#endif

extern int g_i2c_cs8k_cab;

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
MT_FE_RET _mt_fe_dmd_set_reg(void *dev_handle, U8 reg_index, U8 data)
{
    int ret = 0;
    struct mt_i2c_msg mt_msg = {0,};
    struct i2c_msg *msg = &mt_msg.msg;
    unsigned char buf[2];
    struct i2c_adapter *i2c = NULL;
    MT_FE_CS8000_CAB_Device_Handle handle = (MT_FE_CS8000_CAB_Device_Handle)dev_handle;

    i2c = i2c_get_adapter(g_i2c_cs8k_cab);
    msg->addr = handle->demod_dev_addr;
    msg->flags = I2C_M_TEN | I2C_M_SALVE_TYPE;
    msg->buf = buf;
    msg->len = 2;
    msg->buf[0] = reg_index;
    msg->buf[1] = data;
    mt_msg.slave_type = (handle->on_board_settings.chip_mode == 1) ? I2C_SLAVE_DEV_SOC_INTER_DEVICE2 : I2C_SLAVE_DEV_SOC_INTER;
    //mtos_sem_take(&reg_rw_mutex, 0);
    ret = i2c_transfer(i2c, msg, 1);
    //mtos_sem_give(&reg_rw_mutex);
    if (ret < 0) {
	printk("ret=%d devaddr=%x reg=%x \n", ret, handle->tuner_settings.tuner_dev_addr, reg_index);
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
MT_FE_RET _mt_fe_dmd_get_reg(void *dev_handle, U8 reg_index, U8 *p_buf)
{
    int ret = 0;
    struct mt_i2c_msg mt_msg = {0,};
    struct i2c_msg *msg = &mt_msg.msg;
    unsigned char buf[2];
    struct i2c_adapter *i2c = NULL;
    MT_FE_CS8000_CAB_Device_Handle handle = (MT_FE_CS8000_CAB_Device_Handle)dev_handle;

    i2c = i2c_get_adapter(g_i2c_cs8k_cab);
    msg->addr = handle->demod_dev_addr;
    msg->flags = I2C_M_STD_RD | I2C_M_SALVE_TYPE;
    msg->buf = buf;
    msg->len = 1;
    mt_msg.rlen = 1;
    mt_msg.wlen = 1;
    msg->buf[0] = reg_index;
    mt_msg.slave_type = (handle->on_board_settings.chip_mode == 1) ? I2C_SLAVE_DEV_SOC_INTER_DEVICE2 : I2C_SLAVE_DEV_SOC_INTER;
    //mtos_sem_take(&reg_rw_mutex, 0);
    ret = i2c_transfer(i2c, msg, 1);
    //mtos_sem_give(&reg_rw_mutex);
    p_buf[0] = msg->buf[0];
    if (ret < 0) {
	MT_ERR_FRONTEND("ret=%d devaddr=%x reg=%x \n",ret, handle->tuner_settings.tuner_dev_addr, reg_index);
	return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}

/* i2c porting layer */
MT_FE_RET _mt_fe_dmd_reg_write_unlock(MT_FE_CS8000_CAB_Device_Handle dev_handle, U8 reg_index, U8 data)
{
    int ret = 0;
    struct mt_i2c_msg mt_msg = {0,};
    struct i2c_msg *msg = &mt_msg.msg;
    unsigned char buf[2];
    struct i2c_adapter *i2c = NULL;
    MT_FE_CS8000_CAB_Device_Handle handle = (MT_FE_CS8000_CAB_Device_Handle)dev_handle;

    i2c = i2c_get_adapter(g_i2c_cs8k_cab);
    msg->addr = handle->demod_dev_addr;
    msg->flags = I2C_M_TEN | I2C_M_SALVE_TYPE;
    msg->buf = buf;
    msg->len = 2;
    msg->buf[0] = reg_index;
    msg->buf[1] = data;
    mt_msg.slave_type = (handle->on_board_settings.chip_mode == 1) ? I2C_SLAVE_DEV_SOC_INTER_DEVICE2 : I2C_SLAVE_DEV_SOC_INTER;
    ret = i2c_transfer(i2c, msg, 1);
    if (ret < 0) {
	MT_ERR_FRONTEND("ret=%d devaddr=%x reg=%x \n",ret, handle->tuner_settings.tuner_dev_addr, reg_index);
	return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_reg_read_unlock(MT_FE_CS8000_CAB_Device_Handle dev_handle, U8 reg_index, U8 *p_buf)
{
    int ret = 0;
    struct mt_i2c_msg mt_msg = {0,};
    struct i2c_msg *msg = &mt_msg.msg;
    unsigned char buf[2];
    struct i2c_adapter *i2c = NULL;
    MT_FE_CS8000_CAB_Device_Handle handle = (MT_FE_CS8000_CAB_Device_Handle)dev_handle;

    i2c = i2c_get_adapter(g_i2c_cs8k_cab);
    msg->addr = handle->demod_dev_addr;
    msg->flags = I2C_M_STD_RD | I2C_M_SALVE_TYPE;
    msg->buf = buf;
    msg->len = 1;
    mt_msg.rlen = 1;
    mt_msg.wlen = 1;
    msg->buf[0] = reg_index;
    mt_msg.slave_type = (handle->on_board_settings.chip_mode == 1) ? I2C_SLAVE_DEV_SOC_INTER_DEVICE2 : I2C_SLAVE_DEV_SOC_INTER;
    ret = i2c_transfer(i2c, msg, 1);
    p_buf[0] = msg->buf[0];
    if (ret < 0) {
	MT_ERR_FRONTEND("ret=%d devaddr=%x reg=%x \n",ret, handle->tuner_settings.tuner_dev_addr, reg_index);
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
MT_FE_RET _mt_fe_tn_set_reg(void *dev_handle, U8 reg_index, U8 data)
{
    int ret = 0;
    U8 value;
    struct mt_i2c_msg mt_msg = {0,};
    struct i2c_msg *msg = &mt_msg.msg;
    unsigned char buf[2];
    struct i2c_adapter *i2c = NULL;
    MT_FE_CS8000_CAB_Device_Handle handle = (MT_FE_CS8000_CAB_Device_Handle)dev_handle;

    i2c = i2c_get_adapter(g_i2c_cs8k_cab);

    if (handle == NULL) {
	return MtFeErr_Uninit;
    }

    /*open I2C repeater*/
    /*Do not care to close the I2C repeater, it will close by itself*/
    if (handle->on_board_settings.chip_mode != 2) {
	_mt_fe_dmd_reg_read_unlock(handle, 0x87, &value);
	value &= 0x0F;
	value |= 0x90;
	/* bit7 = 1, Enable I2C repeater, bit[6:4] = 1, Enable I2C repeater for 1 time */
	_mt_fe_dmd_reg_write_unlock(handle, 0x87, value);
    } else {
	_mt_fe_dmd_reg_read_unlock(handle, 0x86, &value);
	value |= 0x80;
	_mt_fe_dmd_reg_write_unlock(handle, 0x86, value);
    }

    /*Do not sleep any time after I2C repeater is opened.*/
    /*please set tuner register at once.*/
    msg->addr = handle->tuner_settings.tuner_dev_addr;
    msg->flags = I2C_M_TEN | I2C_M_SALVE_TYPE;
    msg->buf = buf;
    msg->len = 2;
    msg->buf[0] = reg_index;
    msg->buf[1] = data;
    mt_msg.slave_type = (handle->on_board_settings.chip_mode == 1) ? I2C_SLAVE_DEV_SOC_INTER_DEVICE2 : I2C_SLAVE_DEV_SOC_INTER;
    ret = i2c_transfer(i2c, msg, 1);
    // i2c_write(handle->tuner_settings.tuner_dev_addr, reg_index, 1, &data, 1);
    if (ret < 0) {
	MT_ERR_FRONTEND("ret=%d devaddr=%x reg=%x \n",ret, handle->tuner_settings.tuner_dev_addr, reg_index);
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
MT_FE_RET _mt_fe_tn_get_reg(void *dev_handle, U8 reg_index, U8 *p_buf)
{
    int ret = 0;
    U8 value;
    struct mt_i2c_msg mt_msg = {0,};
    struct i2c_msg *msg = &mt_msg.msg;
    unsigned char buf[2];
    struct i2c_adapter *i2c = NULL;
    MT_FE_CS8000_CAB_Device_Handle handle = (MT_FE_CS8000_CAB_Device_Handle)dev_handle;

    i2c = i2c_get_adapter(g_i2c_cs8k_cab);

    if (handle == NULL) {
	return MtFeErr_Uninit;
    }

    /*open I2C repeater*/
    /*Do not care to close the I2C repeater, it will close by itself*/

    /*Do not sleep any time after I2C repeater is opened.*/
    /*please read tuner register at once.*/
    //MT_ERR_FRONTEND("tnget 0x%02x@[0x%02x] -> 0x%02x\n", handle->tuner_settings.tuner_dev_addr,reg_index, *p_buf);
    //i2c_seq_read(handle->tuner_settings.tuner_dev_addr, reg_index, 1, p_buf, 1);

    if (handle->on_board_settings.chip_mode != 2) {
	_mt_fe_dmd_reg_read_unlock(handle, 0x87, &value);
	/* bit7 = 1, Enable I2C repeater, bit[6:4] = 1, Enable I2C repeater for 1 time */
	value &= 0x0F;
	value |= 0xa0;
	_mt_fe_dmd_reg_write_unlock(handle, 0x87, value);
	msg->addr = handle->tuner_settings.tuner_dev_addr;
	msg->flags = I2C_M_STD_RD | I2C_M_SALVE_TYPE;
	msg->buf = buf;
	msg->len = 1;
	mt_msg.rlen = 1;
	mt_msg.wlen = 1;
	msg->buf[0] = reg_index;
	mt_msg.slave_type = (handle->on_board_settings.chip_mode == 1) ? I2C_SLAVE_DEV_SOC_INTER_DEVICE2 : I2C_SLAVE_DEV_SOC_INTER;
	ret = i2c_transfer(i2c, msg, 1);
	p_buf[0] = msg->buf[0];
	if (ret < 0) {
	    MT_ERR_FRONTEND("rc=%d devaddr=%x reg=%x \n",ret, handle->tuner_settings.tuner_dev_addr, reg_index);
	    return MtFeErr_I2cErr;
	}
    } else {
	_mt_fe_dmd_reg_read_unlock(handle, 0x86, &value);
	value |= 0x80;
	_mt_fe_dmd_reg_write_unlock(handle, 0x86, value);
	msg->addr = handle->tuner_settings.tuner_dev_addr;
	msg->flags = I2C_M_SEQ_RD | I2C_M_SALVE_TYPE;
	msg->buf = buf;
	msg->len = 1;
	mt_msg.rlen = 1;
	mt_msg.wlen = 1;
	msg->buf[0] = reg_index;
	mt_msg.slave_type = (handle->on_board_settings.chip_mode == 1) ? I2C_SLAVE_DEV_SOC_INTER_DEVICE2 : I2C_SLAVE_DEV_SOC_INTER;
	ret = i2c_transfer(i2c, msg, 1);
	p_buf[0] = msg->buf[0];
	if (ret < 0) {
	    MT_ERR_FRONTEND("ret=%d devaddr=%x reg=%x \n",ret, handle->tuner_settings.tuner_dev_addr, reg_index);
	    return MtFeErr_I2cErr;
	}
    }

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_tn_write(void *dev_handle, U8 *p_buf, U16 n_byte)
{
    int ret = 0;
    U8 value;
    struct mt_i2c_msg mt_msg = {0,};
    struct i2c_msg *msg = &mt_msg.msg;
    struct i2c_adapter *i2c = NULL;
    MT_FE_CS8000_CAB_Device_Handle handle = (MT_FE_CS8000_CAB_Device_Handle)dev_handle;

    if (handle == NULL) {
	return MtFeErr_Uninit;
    }

    /*open I2C repeater*/
    /*Do not care to close the I2C repeater, it will close by itself*/
    if (handle->on_board_settings.chip_mode != 2) {
	_mt_fe_dmd_reg_read_unlock(handle, 0x87, &value);
	value &= 0x0F;
	value |= 0x90;
	/* bit7 = 1, Enable I2C repeater, bit[6:4] = 1, Enable I2C repeater for 1 time */
	_mt_fe_dmd_reg_write_unlock(handle, 0x87, value);
    } else {
	_mt_fe_dmd_reg_read_unlock(handle, 0x86, &value);
	value |= 0x80;
	_mt_fe_dmd_reg_write_unlock(handle, 0x86, value);
    }

    /*Do not sleep any time after I2C repeater is opened.*/
    /*please write N bytes to register at once.*/
    i2c = i2c_get_adapter(g_i2c_cs8k_cab);

    msg->addr = handle->tuner_settings.tuner_dev_addr;
    msg->flags = I2C_M_TEN | I2C_M_SALVE_TYPE;
    msg->buf = p_buf;
    msg->len = n_byte;
    mt_msg.rlen = 0;
    mt_msg.wlen = n_byte;
    mt_msg.slave_type = (handle->on_board_settings.chip_mode == 1) ? I2C_SLAVE_DEV_SOC_INTER_DEVICE2 : I2C_SLAVE_DEV_SOC_INTER;
    ret = i2c_transfer(i2c, msg, 1);
    //i2c_write(handle->tuner_settings.tuner_dev_addr, 0, 0, p_buf, n_byte);
    if (ret < 0) {
	MT_ERR_FRONTEND("ret=%d devaddr=%x\n",ret, handle->tuner_settings.tuner_dev_addr);
	return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_tn_read(void *dev_handle, U8 *p_buf, U16 n_byte)
{
    int ret = 0;
    U8 value;
    struct mt_i2c_msg mt_msg = {0,};
    struct i2c_msg *msg = &mt_msg.msg;
    struct i2c_adapter *i2c = NULL;
    MT_FE_CS8000_CAB_Device_Handle handle = (MT_FE_CS8000_CAB_Device_Handle)dev_handle;

    i2c = i2c_get_adapter(g_i2c_cs8k_cab);
    /*open I2C repeater*/
    /*Do not care to close the I2C repeater, it will close by itself*/
    /*Do not sleep any time after I2C repeater is opened.*/
    /*please read N bytes.*/

    if (handle->on_board_settings.chip_mode != 2) {
	_mt_fe_dmd_reg_read_unlock(handle, 0x87, &value);
	value &= 0x0F;
	value |= 0x90;
	/* bit7 = 1, Enable I2C repeater, bit[6:4] = 1, Enable I2C repeater for 1 time */
	_mt_fe_dmd_reg_write_unlock(handle, 0x87, value);
    } else {
	_mt_fe_dmd_reg_read_unlock(handle, 0x86, &value);
	value |= 0x80;
	_mt_fe_dmd_reg_write_unlock(handle, 0x86, value);
    }

    msg->addr = handle->tuner_settings.tuner_dev_addr;
    msg->flags = I2C_M_SEQ_RD | I2C_M_SALVE_TYPE;
    msg->buf = p_buf;
    msg->len = n_byte;
    mt_msg.rlen = n_byte;
    mt_msg.wlen = 0;
    mt_msg.slave_type = (handle->on_board_settings.chip_mode == 1) ? I2C_SLAVE_DEV_SOC_INTER_DEVICE2 : I2C_SLAVE_DEV_SOC_INTER;
    ret = i2c_transfer(i2c, msg, 1);
    p_buf[0] = msg->buf[0];

    if (ret < 0) {
	MT_ERR_FRONTEND("ret=%d devaddr=%x\n",ret, handle->tuner_settings.tuner_dev_addr);
	return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}

void _mt_cs8k_cab_sleep(U32 ticks_ms)
{
    /*TODO*/
    msleep(ticks_ms);
    /*
		Wait for ticks_ms time, the time unit is millisecond.
	*/
}

MT_FE_RET _mt_fe_write32(U32 reg_addr, U32 reg_data)
{
	HAL_PUT_U32((volatile u32 *)reg_addr, reg_data);
    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_read32(U32 reg_addr, U32 *p_data)
{
	*p_data = HAL_GET_U32((volatile u32 *)reg_addr);
    return MtFeErr_Ok;
}

S32 _mt_fe_tn_get_reg_tc3800(MT_FE_Tuner_Handle_TC3800 handle, U8 reg_addr, U8 *reg_data)
{
    MT_FE_CS8000_CAB_Device_Handle dev_hdl = (MT_FE_CS8000_CAB_Device_Handle)handle->p_private;
    return _mt_fe_tn_get_reg(dev_hdl, reg_addr, reg_data);
}

S32 _mt_fe_tn_set_reg_tc3800(MT_FE_Tuner_Handle_TC3800 handle, U8 reg_addr, U8 reg_data)
{
    MT_FE_CS8000_CAB_Device_Handle dev_hdl = (MT_FE_CS8000_CAB_Device_Handle)handle->p_private;
    return _mt_fe_tn_set_reg(dev_hdl, reg_addr, reg_data);
}

S32 _mt_fe_tn_set_reg_bit_tc3800(MT_FE_Tuner_Handle_TC3800 handle, U8 reg_addr, U8 data, U8 high_bit, U8 low_bit)
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

    _mt_fe_tn_get_reg_tc3800(handle, reg_addr, &value);
    value &= ~tmp;
    value |= data;
    _mt_fe_tn_set_reg_tc3800(handle, reg_addr, value);

    return 0;
}

#if 1
S32 _mt_fe_tn_get_reg_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 *reg_data)
{
    MT_FE_CS8000_CAB_Device_Handle dev_hdl = (MT_FE_CS8000_CAB_Device_Handle)handle->p_private;
    return _mt_fe_tn_get_reg(dev_hdl, reg_addr, reg_data);
}

S32 _mt_fe_tn_set_reg_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 reg_data)
{
    MT_FE_CS8000_CAB_Device_Handle dev_hdl = (MT_FE_CS8000_CAB_Device_Handle)handle->p_private;

    return _mt_fe_tn_set_reg(dev_hdl, reg_addr, reg_data);
}

S32 _mt_fe_tn_write_fw_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 *reg_data, U8 data_len)
{
    S32 ret;
    U8 *p_send_buf = NULL;
    MT_FE_CS8000_CAB_Device_Handle dev_hdl = (MT_FE_CS8000_CAB_Device_Handle)handle->p_private;

    if (dev_hdl == NULL) {
	return -1;
    }

    p_send_buf = kzalloc(data_len + 1, GFP_KERNEL);
    if (p_send_buf == NULL) {
	MT_ERR_FRONTEND("ERROR:_mt_fe_tn_write_fw_tc6800 malloc %d FAILED!\n", data_len);
	return -1;
    }

    p_send_buf[0] = reg_addr;
    memcpy(p_send_buf + 1, reg_data, data_len);
    data_len++;
    ret = _mt_fe_tn_write(dev_hdl, p_send_buf, data_len); //data_len+1
    kfree(p_send_buf);

    if (ret != 0) {
	MT_ERR_FRONTEND("ERROR:_mt_fe_tn_write_fw_tc6800 FAILED!\n");
	return -1;
    }

    return ret;
}

void _mt_sleep_tc6800(U32 ticks_ms)
{
    _mt_cs8k_cab_sleep(ticks_ms);
}
#endif

#if 0
S32 _mt_fe_tn_get_reg_tc3800(MT_FE_Tuner_Handle_TC3800 handle, U8 reg_addr, U8 *reg_data)
{
	int ret = 0;

	MT_FE_CS8000_CAB_Device_Handle h = (MT_FE_CS8000_CAB_Device_Handle)handle->p_private;
	if(handle->dmd == DMD_DC2800_FOR_TC3800)
	{
	    ret = _mt_fe_tn_get_reg(h, reg_addr, reg_data);
	}
	else if(handle->dmd == DMD_DD3K_FOR_TC3800)
	{
	    ret = mt_fe_dd3k_tn_get_reg_tc3800(handle, reg_addr, reg_data);
	}

	return ret;
}


S32 _mt_fe_tn_set_reg_tc3800(MT_FE_Tuner_Handle_TC3800 handle, U8 reg_addr, U8 reg_data)
{
	int ret = 0;
	MT_FE_CS8000_CAB_Device_Handle h = (MT_FE_CS8000_CAB_Device_Handle)handle->p_private;
	U8 data[2] = {0,0};
	data[0] = reg_addr;
	data[1] = reg_data;
	if(handle->dmd == DMD_DC2800_FOR_TC3800)
	{
		ret = _mt_fe_tn_set_reg(h, reg_addr, reg_data);
	}
	else if(handle->dmd == DMD_DD3K_FOR_TC3800)
	{
	    ret = mt_fe_dd3k_tn_set_reg_tc3800(handle, reg_addr, reg_data);
	}

	return ret;
}

S32 _mt_fe_tn_set_reg_bit_tc3800(MT_FE_Tuner_Handle_TC3800 handle, U8 reg_addr, U8 data, U8 high_bit, U8 low_bit)
{
	U8 tmp = 0, value = 0;

	if(high_bit < low_bit)
	{
		return -1;
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

	_mt_fe_tn_get_reg_tc3800(handle, reg_addr, &value);
	value &= ~tmp;
	value |= data;
	_mt_fe_tn_set_reg_tc3800(handle, reg_addr, value);

	return 0;
}

#endif
