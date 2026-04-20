/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <asm/atomic.h>

#ifdef CONFIG_MT_CHIP_ARIA
#include <mach/aria_io.h>
#else
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif

#include "mt_kernel_adapt.h"
#include "mt_type.h"

#include "mt_fe_def_ca8k.h"
#include "mt_fe_tn_tc6800.h"
#include "mt_module_debug.h"
//#include "mtos_task.h"
//#include "hal_base.h"
//#include "mt_fe_ca8k_tn_tc3800.h"

#define NIM_BASE_ADDRESS (0xffd00000)

enum {
    I2C_PARAM_DEV_SOC_TUNER = 0,
    I2C_PARAM_DEV_SOC_DEMOD
};

MT_DECLARE_MUTEX(g_fe_i2c_mutex);

//extern void ca8k_dmd_reg_write(MT_FE_CA8K_CAB_Device_Handle p_handle, u8 reg, u8 data);
//extern u8 ca8k_dmd_reg_read(MT_FE_CA8K_CAB_Device_Handle p_handle, u8 reg);
//extern void ca8k_tn_reg_write(MT_FE_CA8K_CAB_Device_Handle p_handle, u8 reg, u8 data);
//extern u8 ca8k_tn_reg_read(MT_FE_CA8K_CAB_Device_Handle p_handle, u8 reg);
//extern s32 ca8k_tn_seq_write(MT_FE_CA8K_CAB_Device_Handle p_handle, u8 *p_buf, u32 len);
//extern s32 ca8k_tn_seq_read(MT_FE_CA8K_CAB_Device_Handle p_handle, u8 *p_buf, u32 wlen, u32 rlen);

MT_FE_RET ahb_write_dmd_reg(U8 reg_addr, U8 *p_buf, U32 len)
{
    U32 i = 0;
	U32 sysctrl_base;
	sysctrl_base = aria_get_demod_base();

    for (i = 0; i < len; i++) {
		//(*(volatile U32 *)(NIM_BASE_ADDRESS + ((U32)reg_addr + i) * 4)) = *(p_buf + i);
		HAL_PUT_U32((volatile u32 *)(sysctrl_base + ((U32)reg_addr + i) * 4), *(p_buf + i));
		//printk("reg0x%08x = 0x%08x\n", (NIM_BASE_ADDRESS + ((u32)reg_addr + i) * 4), *(p_buf + i));
    }

    return MtFeErr_Ok;
}

MT_FE_RET ahb_read_dmd_reg(U8 reg_addr, U8 *p_buf, U32 rlen)
{
    U32 i = 0;
	U32 sysctrl_base;
	U32 data = 0;
	sysctrl_base = aria_get_demod_base();

    for (i = 0; i < rlen; i++)
	{
		//*(p_buf + i) = ((*(volatile U32 *)(NIM_BASE_ADDRESS + ((U32)reg_addr + i) * 4)) & 0xFF);
		*(p_buf + i) = (HAL_GET_U32((volatile u32 *)(sysctrl_base + ((U32)reg_addr + i) * 4)) & 0xff);
	}

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_set_reg(void *dev_handle, U8 reg_index, U8 data)
{
    MT_FE_RET ret;
    U8 buf[2];
    //MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

    buf[0] = reg_index;
    buf[1] = data;

    down_interruptible(&g_fe_i2c_mutex);
    ret = ahb_write_dmd_reg(reg_index, &data, 1);
    up(&g_fe_i2c_mutex);

    if (ret != MT_SUCCESS) {
	MT_ERR_FRONTEND("_mt_fe_dmd_set_reg reg %x data %x FAILED!\n", reg_index, data);
	return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_get_reg(void *dev_handle, U8 reg_index, U8 *p_buf)
{
    MT_FE_RET ret;
    //MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

    down_interruptible(&g_fe_i2c_mutex);
    ret = ahb_read_dmd_reg(reg_index, p_buf, 1);
    up(&g_fe_i2c_mutex);

    if (ret != MT_SUCCESS) {
	MT_ERR_FRONTEND("_mt_fe_dmd_get_reg reg %x FAILED!\n", reg_index);
	return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_i2c_read(U8 i2c_id, U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
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

    *r_buf = *w_buf;

    i2c = i2c_get_adapter(i2c_id);
    msg->addr = dev_addr;
    msg->flags = I2C_M_STD_RD;
    msg->buf = r_buf;
    msg->len = r_byte;
    mt_msg.rlen = r_byte;
    mt_msg.wlen = w_byte;
    ret = i2c_transfer(i2c, msg, 1);

    //  printk("%s, dev_addr=0x%x, *w_buf=0x%x, w_byte=0x%x, *r_buf=0x%x, r_byte=0x%x\n",
    //    __FUNCTION__, dev_addr, *w_buf, w_byte, *r_buf, r_byte);
	if (ret < 0)
	{
		MT_ERR_FRONTEND("_mt_fe_i2c_read failed, ret %d", ret);
		return MtFeErr_I2cErr;
	}

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_i2c_write(U8 i2c_id, U8 dev_addr, U8 *w_buf, U16 w_byte)
{
    struct mt_i2c_msg mt_msg = {0,};
    struct i2c_msg *msg = &mt_msg.msg;
    struct i2c_adapter *i2c = NULL;
	int ret = 0;

    i2c = i2c_get_adapter(i2c_id);
    msg->addr = dev_addr;
    msg->flags = I2C_M_TEN;
    msg->buf = w_buf;
    msg->len = w_byte;
    mt_msg.rlen = 0;
    mt_msg.wlen = w_byte;
    ret = i2c_transfer(i2c, msg, 1);
	if (ret < 0)
	{
		MT_ERR_FRONTEND("_mt_fe_i2c_write failed, ret %d", ret);
		return MtFeErr_I2cErr;
	}

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_tn_set_reg(void *dev_handle, U8 reg_index, U8 data)
{
    MT_FE_RET ret;
    U8 buf[2];
    U8 i2c_id = 0;
    U8 slv_addr = 0;
    MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
    //MT_FE_RET ret = MtFeErr_I2cErr;

    if (0) //handle == NULL)
    {
	return MtFeErr_Uninit;
    }

    //printk("_mt_fe_tn_set_reg tun_addr %02x reg %02x !!!\n", dev_addr, reg_index);

    slv_addr = handle->tuner_settings.tuner_dev_addr;
    i2c_id = handle->tuner_i2c_id;

    buf[0] = reg_index;
    buf[1] = data;

    //down_interruptible(&g_fe_i2c_mutex);
    ret = _mt_fe_i2c_write(i2c_id, slv_addr, buf, 2);
    //up(&g_fe_i2c_mutex);

    if (ret < 0) {
	MT_ERR_FRONTEND("_mt_fe_tn_set_reg tun_addr %02x reg %x Failure!!!\n", slv_addr, reg_index);
	return MtFeErr_I2cErr;
    }

    //printk("_s2sa_tn_set_reg tun_addr %02x reg %x!!!\n", p_nim_cfg->tun_addr, reg_index);

    return MtFeErr_Ok;
}

//MT_FE_RET   (*tn_set_reg)(U8 i2c_id, U8 dev_addr, U8 reg_index, U8 data);
//MT_FE_RET   (*tn_get_reg)(U8 i2c_id, U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte);
//MT_FE_RET	(*tn_get_reg)(void *handle, U8 reg_index, U8 *p_buf);
//MT_FE_RET _mt_fe_tn_get_reg(void *dev_handle, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)

MT_FE_RET _mt_fe_tn_get_reg(void *dev_handle, U8 reg_index, U8 *p_buf)
{
    MT_FE_RET ret;
    U8 i2c_id = 0;
    U8 slv_addr = 0;
    MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

    if (0) //handle == NULL)
    {
	return MtFeErr_Uninit;
    }

    slv_addr = handle->tuner_settings.tuner_dev_addr;
    i2c_id = handle->tuner_i2c_id;

    //down_interruptible(&g_fe_i2c_mutex);
    ret = _mt_fe_i2c_read(i2c_id, slv_addr, &reg_index, 1, p_buf, 1);
    //MT_FE_RET _mt_fe_i2c_read(U8 i2c_id, U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
    //up(&g_fe_i2c_mutex);

    if (ret < 0) {
	//printk("_mt_fe_tn_get_reg bus[0x%x], addr[0x%x], reg %x Failure!!!\n", i2c_id, dev_addr);
	MT_ERR_FRONTEND("_mt_fe_tn_get_reg bus[0x%x], slv_addr[0x%x] Failure!!!\n", i2c_id, slv_addr);
	return MtFeErr_I2cErr;
    }

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_tn_write(void *dev_handle, U8 *p_buf, U16 n_byte)
{
    //MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_tn_read(void *dev_handle, U8 *p_buf, U16 n_byte)
{
    //MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_write32(U32 reg_addr, U32 reg_data)
{
    //hal_put_u32((volatile unsigned long *)reg_addr, reg_data);
#if 1
	//U32 data = 0;
	U32 sysctrl_base;
	sysctrl_base = mt_get_sys_ctrl_base();
	//data = HAL_GET_U32((volatile u32 *)(sysctrl_base + reg_addr));
	//data = ((data & 0xffffefff) | 0x80000000);
	HAL_PUT_U32((volatile u32 *)(sysctrl_base + reg_addr), reg_data);
#endif

	return MtFeErr_Ok;
}


MT_FE_RET _mt_fe_read32(U32 reg_addr, U32 *p_data)
{
    //*p_data = hal_get_u32((volatile unsigned long *)reg_addr);
	U32 data = 0;

#if 1
 	U32 sysctrl_base;
	sysctrl_base = mt_get_sys_ctrl_base();
	data = HAL_GET_U32((volatile u32 *)(sysctrl_base + reg_addr));
#endif

    *p_data = data;

	return MtFeErr_Ok;
}

S32 _mt_fe_tn_get_reg_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 *reg_data)
{
#if 1
	MT_FE_CA8K_CAB_Device_Handle dev_handle = (MT_FE_CA8K_CAB_Device_Handle)handle->p_private;

	return _mt_fe_tn_get_reg((void *)dev_handle, reg_addr, reg_data);
#else
	U8 slv_addr = 0;
	U8 i2c_id = 0;
	MT_FE_CA8K_CAB_Device_Handle dev_handle = (MT_FE_CA8K_CAB_Device_Handle)handle->p_private;

	slv_addr = dev_handle->tuner_settings.tuner_dev_addr;
	i2c_id = dev_handle->tuner_i2c_id;

	return _mt_fe_tn_get_reg(i2c_id, slv_addr, &reg_addr, reg_data);
#endif
}


S32 _mt_fe_tn_set_reg_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 reg_data)
{
#if 1
	MT_FE_CA8K_CAB_Device_Handle dev_handle = (MT_FE_CA8K_CAB_Device_Handle)handle->p_private;

	return _mt_fe_tn_set_reg((void *)dev_handle, reg_addr, reg_data);
#else
	U8 slv_addr = 0;
	U8 i2c_id = 0;
	MT_FE_CA8K_CAB_Device_Handle dev_handle = (MT_FE_CA8K_CAB_Device_Handle)handle->p_private;

	slv_addr = dev_handle->tuner_settings.tuner_dev_addr;
	i2c_id = dev_handle->tuner_i2c_id;

	return _mt_fe_tn_set_reg(i2c_id, slv_addr, reg_addr, reg_data);
#endif
}

S32 _mt_fe_tn_write_fw_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 *reg_data, U8 data_len)
{
    MT_FE_RET ret;
	U8 *p_send_buf = NULL;
	U8 slv_addr = 0;
	U8 i2c_id = 0;
	MT_FE_CA8K_CAB_Device_Handle dev_handle;

    if (handle == NULL)
    {
        return -1;
    }

	dev_handle = (MT_FE_CA8K_CAB_Device_Handle)handle->p_private;
	slv_addr = dev_handle->tuner_settings.tuner_dev_addr;
	i2c_id = dev_handle->tuner_i2c_id;

	p_send_buf = kzalloc(data_len+1, GFP_KERNEL);
	if (p_send_buf == NULL)
	{
		MT_ERR_FRONTEND("ERROR:_mt_fe_tn_write_fw_tc6800 malloc %d FAILED!\n", data_len);
		return MtFeErr_NoMemory;
	}

	p_send_buf[0] = reg_addr;
	memcpy(p_send_buf+1, reg_data, data_len);
	data_len++;
	ret = _mt_fe_i2c_write(i2c_id, slv_addr, p_send_buf, data_len);
	kfree(p_send_buf);

	if (ret != MT_SUCCESS)
	{
		MT_ERR_FRONTEND("ERROR:_mt_fe_tn_write_fw_tc6800 FAILED!\n");
		return MtFeErr_I2cErr;
	}

	return ret;
}



void _mt_sleep(U32 ticks_ms)
{
	msleep(ticks_ms);
}

void _mt_sleep_tc6800(U32 ticks_ms)
{
	msleep(ticks_ms);
}


