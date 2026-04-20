/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
//
#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/proc_fs.h>

#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <linux/cdev.h>
#include <linux/compiler.h>
#include <linux/device.h>
#include <linux/clocksource.h>
#include <linux/jiffies.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
//#include <delay.h>
#include <linux/kthread.h>
#include "mt_type.h"
#include "hd_enc_aria_reg.h"
//#include "disp_ap_aria_reg.h"

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

/*!
  Write 32 bits register

  \param[in] addr register address
  \param[in] p_addr data to write
  */
static inline void hal_put_u32(volatile unsigned long *p_addr, unsigned long data)
{
  /*!
      Write 32 bits register
    */
  HAL_PUT_U32((u32*)p_addr, (u32)data);
}

static inline mt_u32 hal_get_u32(volatile mt_u32 *p_addr)
{
    return (mt_u32)HAL_GET_U32((u32*)p_addr);
}

void reg_aria_hd_encoder_set_basic_cfg(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_BASIC_CFG, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_basic_cfg);

mt_u32  reg_aria_hd_encoder_get_basic_cfg(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_BASIC_CFG);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_basic_cfg);

void reg_aria_hd_encoder_set_basic_cfg_resolusion(mt_u8 data)
{
    reg_aria_hd_encoder_basic_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_BASIC_CFG;
    d.bitc.resolusion = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_BASIC_CFG, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_basic_cfg_resolusion);

mt_u8   reg_aria_hd_encoder_get_basic_cfg_resolusion(void)
{
    return (*(volatile reg_aria_hd_encoder_basic_cfg_t *)
        REG_ARIA_HD_ENCODER_BASIC_CFG).bitc.resolusion;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_basic_cfg_resolusion);

void reg_aria_hd_encoder_set_basic_cfg_interlace_mode(mt_u8 data)
{
    reg_aria_hd_encoder_basic_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_BASIC_CFG;
    d.bitc.interlace_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_BASIC_CFG, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_basic_cfg_interlace_mode);

mt_u8   reg_aria_hd_encoder_get_basic_cfg_interlace_mode(void)
{
    return (*(volatile reg_aria_hd_encoder_basic_cfg_t *)
        REG_ARIA_HD_ENCODER_BASIC_CFG).bitc.interlace_mode;
}
//EXPORT_SYMBOL(reg_aria_hd_encoder_get_basic_cfg_interlace_mode);

void reg_aria_hd_encoder_set_basic_cfg_start_num(u16 data)
{
    reg_aria_hd_encoder_basic_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_BASIC_CFG;
    d.bitc.start_num = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_BASIC_CFG, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_basic_cfg_start_num);

u16  reg_aria_hd_encoder_get_basic_cfg_start_num(void)
{
    return (*(volatile reg_aria_hd_encoder_basic_cfg_t *)
        REG_ARIA_HD_ENCODER_BASIC_CFG).bitc.start_num;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_basic_cfg_start_num);

void reg_aria_hd_encoder_set_basic_cfg_start_num1(u16 data)
{
    reg_aria_hd_encoder_basic_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_BASIC_CFG;
    d.bitc.start_num1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_BASIC_CFG, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_basic_cfg_start_num1);

u16  reg_aria_hd_encoder_get_basic_cfg_start_num1(void)
{
    return (*(volatile reg_aria_hd_encoder_basic_cfg_t *)
        REG_ARIA_HD_ENCODER_BASIC_CFG).bitc.start_num1;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_basic_cfg_start_num1);

void reg_aria_hd_encoder_set_basic_cfg_clk_auto_gate_en(mt_u8 data)
{
    reg_aria_hd_encoder_basic_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_BASIC_CFG;
    d.bitc.clk_auto_gate_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_BASIC_CFG, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_basic_cfg_clk_auto_gate_en);

mt_u8   reg_aria_hd_encoder_get_basic_cfg_clk_auto_gate_en(void)
{
    return (*(volatile reg_aria_hd_encoder_basic_cfg_t *)REG_ARIA_HD_ENCODER_BASIC_CFG).bitc.clk_auto_gate_en;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_basic_cfg_clk_auto_gate_en);


/*!
  register ARIA_HD_ENCODER_y_parameter_level0 (read/write)
  */
void reg_aria_hd_encoder_set_y_parameter_level0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)
        REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_y_parameter_level0);

mt_u32  reg_aria_hd_encoder_get_y_parameter_level0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_y_parameter_level0);

void reg_aria_hd_encoder_set_y_parameter_level0_y_a_level(u16 data)
{
    reg_aria_hd_encoder_y_parameter_level0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0;
    d.bitc.y_a_level = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_y_parameter_level0_y_a_level);

u16  reg_aria_hd_encoder_get_y_parameter_level0_y_a_level(void)
{
    return (*(volatile reg_aria_hd_encoder_y_parameter_level0_t *)
        REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0).bitc.y_a_level;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_y_parameter_level0_y_a_level);
void reg_aria_hd_encoder_set_y_parameter_level0_is_frame_packing(mt_u8 data)
{
    reg_aria_hd_encoder_y_parameter_level0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0;
    d.bitc.is_frame_packing = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_y_parameter_level0_is_frame_packing);

mt_u8  reg_aria_hd_encoder_get_y_parameter_level0_is_frame_packing(void)
{
    return (*(volatile reg_aria_hd_encoder_y_parameter_level0_t *)
        REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0).bitc.is_frame_packing;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_y_parameter_level0_is_frame_packing);

void reg_aria_hd_encoder_set_y_parameter_level0_de_mode(mt_u8 data)
{
    reg_aria_hd_encoder_y_parameter_level0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0;
    d.bitc.de_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_y_parameter_level0_de_mode);

mt_u8  reg_aria_hd_encoder_get_y_parameter_level0_de_mode(void)
{
    return (*(volatile reg_aria_hd_encoder_y_parameter_level0_t *)
        REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0).bitc.de_mode;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_y_parameter_level0_de_mode);

void reg_aria_hd_encoder_set_y_parameter_level0_y_blank_level(u16 data)
{
    reg_aria_hd_encoder_y_parameter_level0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0;
    d.bitc.y_blank_level = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_y_parameter_level0_y_blank_level);

u16  reg_aria_hd_encoder_get_y_parameter_level0_y_blank_level(void)
{
    return (*(volatile reg_aria_hd_encoder_y_parameter_level0_t *)
        REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0).bitc.y_blank_level;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_y_parameter_level0_y_blank_level);

void reg_aria_hd_encoder_set_y_parameter_level0_cable_dct_en(mt_u8 data)
{
    reg_aria_hd_encoder_y_parameter_level0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0;
    d.bitc.cable_dct_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_y_parameter_level0_cable_dct_en);

mt_u8   reg_aria_hd_encoder_get_y_parameter_level0_cable_dct_en(void)
{
    return (*(volatile reg_aria_hd_encoder_y_parameter_level0_t *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0).bitc.cable_dct_en;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_y_parameter_level0_cable_dct_en);

void reg_aria_hd_encoder_set_y_parameter_level0_hdmi_lock(mt_u8 data)
{
    reg_aria_hd_encoder_y_parameter_level0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0;
    d.bitc.hdmi_lock = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_y_parameter_level0_hdmi_lock);

mt_u8   reg_aria_hd_encoder_get_y_parameter_level0_hdmi_lock(void)
{
    return (*(volatile reg_aria_hd_encoder_y_parameter_level0_t *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0).bitc.hdmi_lock;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_y_parameter_level0_hdmi_lock);

void reg_aria_hd_encoder_set_y_parameter_level0_hd_lock(u8 data)
{
    reg_aria_hd_encoder_y_parameter_level0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0;
    d.bitc.hd_lock = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_y_parameter_level0_hd_lock);

mt_u8   reg_aria_hd_encoder_get_y_parameter_level0_hd_lock(void)
{
    return (*(volatile reg_aria_hd_encoder_y_parameter_level0_t *)REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0).bitc.hd_lock;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_y_parameter_level0_hd_lock);


/*!
  register ARIA_HD_ENCODER_y_sync_level (read/write)
  */
void reg_aria_hd_encoder_set_y_sync_level(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_Y_SYNC_LEVEL, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_y_sync_level);

mt_u32  reg_aria_hd_encoder_get_y_sync_level(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_Y_SYNC_LEVEL);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_y_sync_level);

void reg_aria_hd_encoder_set_y_sync_level_y_hsync_level(u16 data)
{
    reg_aria_hd_encoder_y_sync_level_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_Y_SYNC_LEVEL;
    d.bitc.y_hsync_level = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_Y_SYNC_LEVEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_y_sync_level_y_hsync_level);

u16  reg_aria_hd_encoder_get_y_sync_level_y_hsync_level(void)
{
    return (*(volatile reg_aria_hd_encoder_y_sync_level_t *)
        REG_ARIA_HD_ENCODER_Y_SYNC_LEVEL).bitc.y_hsync_level;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_y_sync_level_y_hsync_level);

void reg_aria_hd_encoder_set_y_sync_level_y_hi_sync_level(u16 data)
{
    reg_aria_hd_encoder_y_sync_level_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_Y_SYNC_LEVEL;
    d.bitc.y_hi_sync_level = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_Y_SYNC_LEVEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_y_sync_level_y_hi_sync_level);

u16  reg_aria_hd_encoder_get_y_sync_level_y_hi_sync_level(void)
{
    return (*(volatile reg_aria_hd_encoder_y_sync_level_t *)
        REG_ARIA_HD_ENCODER_Y_SYNC_LEVEL).bitc.y_hi_sync_level;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_y_sync_level_y_hi_sync_level);


/*!
  register ARIA_HD_ENCODER_pbpr_parameter0_level (read/write)
  */
void reg_aria_hd_encoder_set_pbpr_parameter0_level(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_PBPR_PARAMETER0_LEVEL, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_pbpr_parameter0_level);

mt_u32  reg_aria_hd_encoder_get_pbpr_parameter0_level(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_PBPR_PARAMETER0_LEVEL);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_pbpr_parameter0_level);

void reg_aria_hd_encoder_set_pbpr_parameter0_level_uv_pbpr_center(u16 data)
{
    reg_aria_hd_encoder_pbpr_parameter0_level_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_PBPR_PARAMETER0_LEVEL;
    d.bitc.v_pbpr_center = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_PBPR_PARAMETER0_LEVEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_pbpr_parameter0_level_uv_pbpr_center);

u16  reg_aria_hd_encoder_get_pbpr_parameter0_level_uv_pbpr_center(void)
{
    return (*(volatile reg_aria_hd_encoder_pbpr_parameter0_level_t *)
        REG_ARIA_HD_ENCODER_PBPR_PARAMETER0_LEVEL).bitc.v_pbpr_center;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_pbpr_parameter0_level_uv_pbpr_center);


/*!
  register ARIA_HD_ENCODER_pbpr_parameter1_level (read/write)
  */
void reg_aria_hd_encoder_set_pbpr_parameter1_level(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_PBPR_PARAMETER1_LEVEL, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_pbpr_parameter1_level);

mt_u32  reg_aria_hd_encoder_get_pbpr_parameter1_level(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_PBPR_PARAMETER1_LEVEL);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_pbpr_parameter1_level);

void reg_aria_hd_encoder_set_pbpr_parameter1_level_uv_pbpr_b(u16 data)
{
    reg_aria_hd_encoder_pbpr_parameter1_level_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_PBPR_PARAMETER1_LEVEL;
    d.bitc.v_pbpr_b = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_PBPR_PARAMETER1_LEVEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_pbpr_parameter1_level_uv_pbpr_b);

u16  reg_aria_hd_encoder_get_pbpr_parameter1_level_uv_pbpr_b(void)
{
    return (*(volatile reg_aria_hd_encoder_pbpr_parameter1_level_t *)
        REG_ARIA_HD_ENCODER_PBPR_PARAMETER1_LEVEL).bitc.v_pbpr_b;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_pbpr_parameter1_level_uv_pbpr_b);


/*!
  register ARIA_HD_ENCODER_hd_cfg_info (read/write)
  */
void reg_aria_hd_encoder_set_hd_cfg_info(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info);

mt_u32  reg_aria_hd_encoder_get_hd_cfg_info(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info);

void reg_aria_hd_encoder_set_hd_cfg_info_reg_field_mode(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.reg_field_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_reg_field_mode);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.reg_field_mode;
}
//EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode);

void reg_aria_hd_encoder_set_hd_cfg_info_frame_rate(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.frame_rate = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_frame_rate);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_frame_rate(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.frame_rate;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_frame_rate);

void reg_aria_hd_encoder_set_hd_cfg_info_reg_hdtv_mode(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.reg_hdtv_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_reg_hdtv_mode);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_reg_hdtv_mode(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.reg_hdtv_mode;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_reg_hdtv_mode);

void reg_aria_hd_encoder_set_hd_cfg_info_comp_mode_on(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.comp_mode_on = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_comp_mode_on);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_comp_mode_on(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.comp_mode_on;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_comp_mode_on);

void reg_aria_hd_encoder_set_hd_cfg_info_video_format(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.video_format = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_video_format);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_video_format(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.video_format;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_video_format);

void reg_aria_hd_encoder_set_hd_cfg_info_hi_sync_enable(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.hi_sync_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_hi_sync_enable);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_hi_sync_enable(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.hi_sync_enable;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_hi_sync_enable);

void reg_aria_hd_encoder_set_hd_cfg_info_pbpr_data_offset_enable(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.pbpr_data_offset_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_pbpr_data_offset_enable);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_pbpr_data_offset_enable(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.pbpr_data_offset_enable;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_pbpr_data_offset_enable);

void reg_aria_hd_encoder_set_hd_cfg_info_delay_y(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.delay_y = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_delay_y);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_delay_y(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.delay_y;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_delay_y);

void reg_aria_hd_encoder_set_hd_cfg_info_py_limit_enable(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.py_limit_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_py_limit_enable);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_py_limit_enable(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.py_limit_enable;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_py_limit_enable);

void reg_aria_hd_encoder_set_hd_cfg_info_pbpr_limit_enablet(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.pbpr_limit_enablet = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_pbpr_limit_enablet);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_pbpr_limit_enablet(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.pbpr_limit_enablet;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_pbpr_limit_enablet);

void reg_aria_hd_encoder_set_hd_cfg_info_y_limit_enable(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.y_limit_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_y_limit_enable);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_y_limit_enable(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.y_limit_enable;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_y_limit_enable);

void reg_aria_hd_encoder_set_hd_cfg_info_cbcr_limit_enable(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.cbcr_limit_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_cbcr_limit_enable);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_cbcr_limit_enable(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.cbcr_limit_enable;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_cbcr_limit_enable);

void reg_aria_hd_encoder_set_hd_cfg_info_enable_1080p(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.enable_1080p = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_enable_1080p);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_enable_1080p(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.enable_1080p;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_enable_1080p);

void reg_aria_hd_encoder_set_hd_cfg_info_vsync_update_timing_enable(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.vsync_update_timing_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_vsync_update_timing_enable);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_vsync_update_timing_enable(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.vsync_update_timing_enable;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_vsync_update_timing_enable);

void reg_aria_hd_encoder_set_hd_cfg_info_h_timing_update_start(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.h_timing_update_start = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_h_timing_update_start);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_h_timing_update_start(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.h_timing_update_start;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_h_timing_update_start);

void reg_aria_hd_encoder_set_hd_cfg_info_reg_disp_on(mt_u8 data)
{
    reg_aria_hd_encoder_hd_cfg_info_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_CFG_INFO;
    d.bitc.reg_disp_on = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_CFG_INFO, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_cfg_info_reg_disp_on);

mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_reg_disp_on(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_cfg_info_t *)
        REG_ARIA_HD_ENCODER_HD_CFG_INFO).bitc.reg_disp_on;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_cfg_info_reg_disp_on);


/*!
  register ARIA_HD_ENCODER_hsync_parameter0 (read/write)
  */
void reg_aria_hd_encoder_set_hsync_parameter0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HSYNC_PARAMETER0, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hsync_parameter0);

mt_u32  reg_aria_hd_encoder_get_hsync_parameter0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HSYNC_PARAMETER0);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hsync_parameter0);

void reg_aria_hd_encoder_set_hsync_parameter0_reg_h_sync_fporchwidth(u16 data)
{
    reg_aria_hd_encoder_hsync_parameter0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HSYNC_PARAMETER0;
    d.bitc.reg_h_sync_fporchwidth = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HSYNC_PARAMETER0, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hsync_parameter0_reg_h_sync_fporchwidth);

u16  reg_aria_hd_encoder_get_hsync_parameter0_reg_h_sync_fporchwidth(void)
{
    return (*(volatile reg_aria_hd_encoder_hsync_parameter0_t *)
        REG_ARIA_HD_ENCODER_HSYNC_PARAMETER0).bitc.reg_h_sync_fporchwidth;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hsync_parameter0_reg_h_sync_fporchwidth);

void reg_aria_hd_encoder_set_hsync_parameter0_reg_h_syncwidth(u16 data)
{
    reg_aria_hd_encoder_hsync_parameter0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HSYNC_PARAMETER0;
    d.bitc.reg_h_syncwidth = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HSYNC_PARAMETER0, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hsync_parameter0_reg_h_syncwidth);

u16  reg_aria_hd_encoder_get_hsync_parameter0_reg_h_syncwidth(void)
{
    return (*(volatile reg_aria_hd_encoder_hsync_parameter0_t *)
        REG_ARIA_HD_ENCODER_HSYNC_PARAMETER0).bitc.reg_h_syncwidth;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hsync_parameter0_reg_h_syncwidth);


/*!
  register ARIA_HD_ENCODER_hsync_parameter1 (read/write)
  */
void reg_aria_hd_encoder_set_hsync_parameter1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HSYNC_PARAMETER1, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hsync_parameter1);

mt_u32  reg_aria_hd_encoder_get_hsync_parameter1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HSYNC_PARAMETER1);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hsync_parameter1);

void reg_aria_hd_encoder_set_hsync_parameter1_reg_h_sync_bporch(u16 data)
{
    reg_aria_hd_encoder_hsync_parameter1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HSYNC_PARAMETER1;
    d.bitc.reg_h_sync_bporch = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HSYNC_PARAMETER1, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hsync_parameter1_reg_h_sync_bporch);

u16  reg_aria_hd_encoder_get_hsync_parameter1_reg_h_sync_bporch(void)
{
    return (*(volatile reg_aria_hd_encoder_hsync_parameter1_t *)
        REG_ARIA_HD_ENCODER_HSYNC_PARAMETER1).bitc.reg_h_sync_bporch;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hsync_parameter1_reg_h_sync_bporch);

void reg_aria_hd_encoder_set_hsync_parameter1_reg_h_sync_activewidth(u16 data)
{
    reg_aria_hd_encoder_hsync_parameter1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HSYNC_PARAMETER1;
    d.bitc.reg_h_sync_activewidth = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HSYNC_PARAMETER1, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hsync_parameter1_reg_h_sync_activewidth);

u16  reg_aria_hd_encoder_get_hsync_parameter1_reg_h_sync_activewidth(void)
{
    return (*(volatile reg_aria_hd_encoder_hsync_parameter1_t *)
        REG_ARIA_HD_ENCODER_HSYNC_PARAMETER1).bitc.reg_h_sync_activewidth;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hsync_parameter1_reg_h_sync_activewidth);


/*!
  register ARIA_HD_ENCODER_reg_H_active_parameter0 (read/write)
  */
void reg_aria_hd_encoder_set_reg_h_active_parameter0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER0, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_h_active_parameter0);

mt_u32  reg_aria_hd_encoder_get_reg_h_active_parameter0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER0);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_h_active_parameter0);

void reg_aria_hd_encoder_set_reg_h_active_parameter0_reg_h_active_fporch(u16 data)
{
    reg_aria_hd_encoder_reg_h_active_parameter0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER0;
    d.bitc.reg_h_active_fporch = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER0, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_h_active_parameter0_reg_h_active_fporch);

u16  reg_aria_hd_encoder_get_reg_h_active_parameter0_reg_h_active_fporch(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_h_active_parameter0_t *)
        REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER0).bitc.reg_h_active_fporch;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_h_active_parameter0_reg_h_active_fporch);

void reg_aria_hd_encoder_set_reg_h_active_parameter0_reg_h_active_bporch(u16 data)
{
    reg_aria_hd_encoder_reg_h_active_parameter0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER0;
    d.bitc.reg_h_active_bporch = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER0, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_h_active_parameter0_reg_h_active_bporch);

u16  reg_aria_hd_encoder_get_reg_h_active_parameter0_reg_h_active_bporch(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_h_active_parameter0_t *)
        REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER0).bitc.reg_h_active_bporch;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_h_active_parameter0_reg_h_active_bporch);


/*!
  register ARIA_HD_ENCODER_reg_H_active_parameter1 (read/write)
  */
void reg_aria_hd_encoder_set_reg_h_active_parameter1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER1, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_h_active_parameter1);

mt_u32  reg_aria_hd_encoder_get_reg_h_active_parameter1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER1);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_h_active_parameter1);

void reg_aria_hd_encoder_set_reg_h_active_parameter1_reg_h_active_activewidth(u16 data)
{
    reg_aria_hd_encoder_reg_h_active_parameter1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER1;
    d.bitc.reg_h_active_activewidth = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER1, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_h_active_parameter1_reg_h_active_activewidth);

u16  reg_aria_hd_encoder_get_reg_h_active_parameter1_reg_h_active_activewidth(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_h_active_parameter1_t *)
        REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER1).bitc.reg_h_active_activewidth;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_h_active_parameter1_reg_h_active_activewidth);

void reg_aria_hd_encoder_set_reg_h_active_parameter1_reg_vsyncpixelcnt(u16 data)
{
    reg_aria_hd_encoder_reg_h_active_parameter1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER1;
    d.bitc.reg_vsyncpixelcnt = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER1, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_h_active_parameter1_reg_vsyncpixelcnt);

u16  reg_aria_hd_encoder_get_reg_h_active_parameter1_reg_vsyncpixelcnt(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_h_active_parameter1_t *)
        REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER1).bitc.reg_vsyncpixelcnt;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_h_active_parameter1_reg_vsyncpixelcnt);


/*!
  register ARIA_HD_ENCODER_reg_V_parameter0 (read/write)
  */
void reg_aria_hd_encoder_set_reg_v_parameter0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_V_PARAMETER0, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_v_parameter0);

mt_u32  reg_aria_hd_encoder_get_reg_v_parameter0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_V_PARAMETER0);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_v_parameter0);

void reg_aria_hd_encoder_set_reg_v_parameter0_reg_v_fporchheight(u16 data)
{
    reg_aria_hd_encoder_reg_v_parameter0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_V_PARAMETER0;
    d.bitc.reg_v_fporchheight = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_V_PARAMETER0, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_v_parameter0_reg_v_fporchheight);

u16  reg_aria_hd_encoder_get_reg_v_parameter0_reg_v_fporchheight(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_v_parameter0_t *)
        REG_ARIA_HD_ENCODER_REG_V_PARAMETER0).bitc.reg_v_fporchheight;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_v_parameter0_reg_v_fporchheight);

void reg_aria_hd_encoder_set_reg_v_parameter0_reg_v_syncheight(u16 data)
{
    reg_aria_hd_encoder_reg_v_parameter0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_V_PARAMETER0;
    d.bitc.reg_v_syncheight = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_V_PARAMETER0, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_v_parameter0_reg_v_syncheight);

u16  reg_aria_hd_encoder_get_reg_v_parameter0_reg_v_syncheight(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_v_parameter0_t *)
        REG_ARIA_HD_ENCODER_REG_V_PARAMETER0).bitc.reg_v_syncheight;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_v_parameter0_reg_v_syncheight);


/*!
  register ARIA_HD_ENCODER_reg_V_parameter1 (read/write)
  */
void reg_aria_hd_encoder_set_reg_v_parameter1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_V_PARAMETER1, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_v_parameter1);

mt_u32  reg_aria_hd_encoder_get_reg_v_parameter1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_V_PARAMETER1);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_v_parameter1);

void reg_aria_hd_encoder_set_reg_v_parameter1_reg_v_bporchheight(u16 data)
{
    reg_aria_hd_encoder_reg_v_parameter1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_V_PARAMETER1;
    d.bitc.reg_v_bporchheight = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_V_PARAMETER1, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_v_parameter1_reg_v_bporchheight);

u16  reg_aria_hd_encoder_get_reg_v_parameter1_reg_v_bporchheight(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_v_parameter1_t *)
        REG_ARIA_HD_ENCODER_REG_V_PARAMETER1).bitc.reg_v_bporchheight;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_v_parameter1_reg_v_bporchheight);

void reg_aria_hd_encoder_set_reg_v_parameter1_reg_v_activeheight(u16 data)
{
    reg_aria_hd_encoder_reg_v_parameter1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_V_PARAMETER1;
    d.bitc.reg_v_activeheight = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_V_PARAMETER1, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_v_parameter1_reg_v_activeheight);

u16  reg_aria_hd_encoder_get_reg_v_parameter1_reg_v_activeheight(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_v_parameter1_t *)
        REG_ARIA_HD_ENCODER_REG_V_PARAMETER1).bitc.reg_v_activeheight;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_v_parameter1_reg_v_activeheight);


/*!
  register ARIA_HD_ENCODER_reg_H_OSD_parameter (read/write)
  */
void reg_aria_hd_encoder_set_reg_h_osd_parameter(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_H_OSD_PARAMETER, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_h_osd_parameter);

mt_u32  reg_aria_hd_encoder_get_reg_h_osd_parameter(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_H_OSD_PARAMETER);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_h_osd_parameter);

void reg_aria_hd_encoder_set_reg_h_osd_parameter_reg_h_osd_start(u16 data)
{
    reg_aria_hd_encoder_reg_h_osd_parameter_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_H_OSD_PARAMETER;
    d.bitc.reg_h_osd_start = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_H_OSD_PARAMETER, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_h_osd_parameter_reg_h_osd_start);

u16  reg_aria_hd_encoder_get_reg_h_osd_parameter_reg_h_osd_start(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_h_osd_parameter_t *)
        REG_ARIA_HD_ENCODER_REG_H_OSD_PARAMETER).bitc.reg_h_osd_start;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_h_osd_parameter_reg_h_osd_start);
#if 0
void reg_aria_hd_encoder_set_reg_h_osd_parameter_reg_frame_rate(u16 data)
{
    reg_aria_hd_encoder_reg_h_osd_parameter_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_H_OSD_PARAMETER;
    d.bitc.frame_rate = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_H_OSD_PARAMETER, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_h_osd_parameter_reg_frame_rate);

u16  reg_aria_hd_encoder_get_reg_h_osd_parameter_reg_frame_rate(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_h_osd_parameter_t *)
        REG_ARIA_HD_ENCODER_REG_H_OSD_PARAMETER).bitc.frame_rate;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_h_osd_parameter_reg_frame_rate);
#endif

/*!
  register ARIA_HD_ENCODER_reg_V_OSD_parameter (read/write)
  */
void reg_aria_hd_encoder_set_reg_v_osd_parameter(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_V_OSD_PARAMETER, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_v_osd_parameter);

mt_u32  reg_aria_hd_encoder_get_reg_v_osd_parameter(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_V_OSD_PARAMETER);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_v_osd_parameter);

void reg_aria_hd_encoder_set_reg_v_osd_parameter_reg_v_osd_start(u16 data)
{
    reg_aria_hd_encoder_reg_v_osd_parameter_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_V_OSD_PARAMETER;
    d.bitc.reg_v_osd_start = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_V_OSD_PARAMETER, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_v_osd_parameter_reg_v_osd_start);

u16  reg_aria_hd_encoder_get_reg_v_osd_parameter_reg_v_osd_start(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_v_osd_parameter_t *)
        REG_ARIA_HD_ENCODER_REG_V_OSD_PARAMETER).bitc.reg_v_osd_start;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_v_osd_parameter_reg_v_osd_start);


/*!
  register ARIA_HD_ENCODER_reg_H_Video_parameter (read/write)
  */
void reg_aria_hd_encoder_set_reg_h_video_parameter(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_H_VIDEO_PARAMETER, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_h_video_parameter);

mt_u32  reg_aria_hd_encoder_get_reg_h_video_parameter(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_H_VIDEO_PARAMETER);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_h_video_parameter);

void reg_aria_hd_encoder_set_reg_h_video_parameter_reg_h_video_start(u16 data)
{
    reg_aria_hd_encoder_reg_h_video_parameter_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_H_VIDEO_PARAMETER;
    d.bitc.reg_h_video_start = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_H_VIDEO_PARAMETER, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_h_video_parameter_reg_h_video_start);

u16  reg_aria_hd_encoder_get_reg_h_video_parameter_reg_h_video_start(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_h_video_parameter_t *)
        REG_ARIA_HD_ENCODER_REG_H_VIDEO_PARAMETER).bitc.reg_h_video_start;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_h_video_parameter_reg_h_video_start);

void reg_aria_hd_encoder_set_reg_h_video_parameter_reg_h_video_end(u16 data)
{
    reg_aria_hd_encoder_reg_h_video_parameter_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_H_VIDEO_PARAMETER;
    d.bitc.reg_h_video_end = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_H_VIDEO_PARAMETER, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_h_video_parameter_reg_h_video_end);

u16  reg_aria_hd_encoder_get_reg_h_video_parameter_reg_h_video_end(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_h_video_parameter_t *)
        REG_ARIA_HD_ENCODER_REG_H_VIDEO_PARAMETER).bitc.reg_h_video_end;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_h_video_parameter_reg_h_video_end);


/*!
  register ARIA_HD_ENCODER_reg_V_Video_parameter (read/write)
  */
void reg_aria_hd_encoder_set_reg_v_video_parameter(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_V_VIDEO_PARAMETER, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_v_video_parameter);

mt_u32  reg_aria_hd_encoder_get_reg_v_video_parameter(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_V_VIDEO_PARAMETER);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_v_video_parameter);

void reg_aria_hd_encoder_set_reg_v_video_parameter_reg_v_video_start(u16 data)
{
    reg_aria_hd_encoder_reg_v_video_parameter_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_V_VIDEO_PARAMETER;
    d.bitc.reg_v_video_start = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_V_VIDEO_PARAMETER, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_v_video_parameter_reg_v_video_start);

u16  reg_aria_hd_encoder_get_reg_v_video_parameter_reg_v_video_start(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_v_video_parameter_t *)
        REG_ARIA_HD_ENCODER_REG_V_VIDEO_PARAMETER).bitc.reg_v_video_start;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_v_video_parameter_reg_v_video_start);

void reg_aria_hd_encoder_set_reg_v_video_parameter_reg_v_video_end(u16 data)
{
    reg_aria_hd_encoder_reg_v_video_parameter_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_V_VIDEO_PARAMETER;
    d.bitc.reg_v_video_end = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_V_VIDEO_PARAMETER, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_v_video_parameter_reg_v_video_end);

u16  reg_aria_hd_encoder_get_reg_v_video_parameter_reg_v_video_end(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_v_video_parameter_t *)
        REG_ARIA_HD_ENCODER_REG_V_VIDEO_PARAMETER).bitc.reg_v_video_end;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_v_video_parameter_reg_v_video_end);


/*!
  register ARIA_HD_ENCODER_reg_color_bar (read/write)
  */
void reg_aria_hd_encoder_set_reg_color_bar(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_COLOR_BAR, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_color_bar);

mt_u32  reg_aria_hd_encoder_get_reg_color_bar(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_COLOR_BAR);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_color_bar);

void reg_aria_hd_encoder_set_reg_color_bar_cbar_width(mt_u8 data)
{
    reg_aria_hd_encoder_reg_color_bar_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_COLOR_BAR;
    d.bitc.cbar_width = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_COLOR_BAR, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_color_bar_cbar_width);

mt_u8   reg_aria_hd_encoder_get_reg_color_bar_cbar_width(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_color_bar_t *)
        REG_ARIA_HD_ENCODER_REG_COLOR_BAR).bitc.cbar_width;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_color_bar_cbar_width);

void reg_aria_hd_encoder_set_reg_color_bar_cbar_enable(mt_u8 data)
{
    reg_aria_hd_encoder_reg_color_bar_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_REG_COLOR_BAR;
    d.bitc.cbar_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_REG_COLOR_BAR, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_reg_color_bar_cbar_enable);

mt_u8   reg_aria_hd_encoder_get_reg_color_bar_cbar_enable(void)
{
    return (*(volatile reg_aria_hd_encoder_reg_color_bar_t *)
        REG_ARIA_HD_ENCODER_REG_COLOR_BAR).bitc.cbar_enable;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_reg_color_bar_cbar_enable);


/*!
  register ARIA_HD_ENCODER_cbar_set_data (read/write)
  */
void reg_aria_hd_encoder_set_cbar_set_data(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_CBAR_SET_DATA, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_cbar_set_data);

mt_u32  reg_aria_hd_encoder_get_cbar_set_data(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_CBAR_SET_DATA);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_cbar_set_data);

void reg_aria_hd_encoder_set_cbar_set_data_y_cbar_set_data(u16 data)
{
    reg_aria_hd_encoder_cbar_set_data_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_CBAR_SET_DATA;
    d.bitc.y_cbar_set_data = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_CBAR_SET_DATA, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_cbar_set_data_y_cbar_set_data);

u16  reg_aria_hd_encoder_get_cbar_set_data_y_cbar_set_data(void)
{
    return (*(volatile reg_aria_hd_encoder_cbar_set_data_t *)
        REG_ARIA_HD_ENCODER_CBAR_SET_DATA).bitc.y_cbar_set_data;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_cbar_set_data_y_cbar_set_data);

void reg_aria_hd_encoder_set_cbar_set_data_pb_cbar_set_data(u16 data)
{
    reg_aria_hd_encoder_cbar_set_data_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_CBAR_SET_DATA;
    d.bitc.pb_cbar_set_data = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_CBAR_SET_DATA, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_cbar_set_data_pb_cbar_set_data);

u16  reg_aria_hd_encoder_get_cbar_set_data_pb_cbar_set_data(void)
{
    return (*(volatile reg_aria_hd_encoder_cbar_set_data_t *)
        REG_ARIA_HD_ENCODER_CBAR_SET_DATA).bitc.pb_cbar_set_data;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_cbar_set_data_pb_cbar_set_data);

void reg_aria_hd_encoder_set_cbar_set_data_pr_cbar_set_data(u16 data)
{
    reg_aria_hd_encoder_cbar_set_data_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_CBAR_SET_DATA;
    d.bitc.pr_cbar_set_data = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_CBAR_SET_DATA, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_cbar_set_data_pr_cbar_set_data);

u16  reg_aria_hd_encoder_get_cbar_set_data_pr_cbar_set_data(void)
{
    return (*(volatile reg_aria_hd_encoder_cbar_set_data_t *)
        REG_ARIA_HD_ENCODER_CBAR_SET_DATA).bitc.pr_cbar_set_data;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_cbar_set_data_pr_cbar_set_data);

void reg_aria_hd_encoder_set_cbar_set_data_cbar_set_enable(mt_u8 data)
{
    reg_aria_hd_encoder_cbar_set_data_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_CBAR_SET_DATA;
    d.bitc.cbar_set_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_CBAR_SET_DATA, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_cbar_set_data_cbar_set_enable);

mt_u8   reg_aria_hd_encoder_get_cbar_set_data_cbar_set_enable(void)
{
    return (*(volatile reg_aria_hd_encoder_cbar_set_data_t *)
        REG_ARIA_HD_ENCODER_CBAR_SET_DATA).bitc.cbar_set_enable;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_cbar_set_data_cbar_set_enable);


/*!
  register ARIA_HD_ENCODER_gama_correction (read/write)
  */
void reg_aria_hd_encoder_set_gama_correction(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_GAMA_CORRECTION, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_gama_correction);

mt_u32  reg_aria_hd_encoder_get_gama_correction(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_GAMA_CORRECTION);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_gama_correction);

void reg_aria_hd_encoder_set_gama_correction_gama_coef(mt_u8 data)
{
    reg_aria_hd_encoder_gama_correction_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_GAMA_CORRECTION;
    d.bitc.gama_coef = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_GAMA_CORRECTION, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_gama_correction_gama_coef);

mt_u8   reg_aria_hd_encoder_get_gama_correction_gama_coef(void)
{
    return (*(volatile reg_aria_hd_encoder_gama_correction_t *)
        REG_ARIA_HD_ENCODER_GAMA_CORRECTION).bitc.gama_coef;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_gama_correction_gama_coef);

void reg_aria_hd_encoder_set_gama_correction_gama_pole(mt_u8 data)
{
    reg_aria_hd_encoder_gama_correction_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_GAMA_CORRECTION;
    d.bitc.gama_pole = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_GAMA_CORRECTION, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_gama_correction_gama_pole);

mt_u8   reg_aria_hd_encoder_get_gama_correction_gama_pole(void)
{
    return (*(volatile reg_aria_hd_encoder_gama_correction_t *)
        REG_ARIA_HD_ENCODER_GAMA_CORRECTION).bitc.gama_pole;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_gama_correction_gama_pole);

void reg_aria_hd_encoder_set_gama_correction_gama_enable(mt_u8 data)
{
    reg_aria_hd_encoder_gama_correction_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_GAMA_CORRECTION;
    d.bitc.gama_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_GAMA_CORRECTION, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_gama_correction_gama_enable);

mt_u8   reg_aria_hd_encoder_get_gama_correction_gama_enable(void)
{
    return (*(volatile reg_aria_hd_encoder_gama_correction_t *)
        REG_ARIA_HD_ENCODER_GAMA_CORRECTION).bitc.gama_enable;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_gama_correction_gama_enable);


/*!
  register ARIA_HD_ENCODER_hd_encoder_status (read)
  */
 void reg_aria_hd_encoder_set_hd_encoder_status(u16 data)
{
    reg_aria_hd_encoder_hd_encoder_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_ENCODER_STATUS;
    d.bitc.empty_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HD_ENCODER_STATUS, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hd_encoder_status);
mt_u32  reg_aria_hd_encoder_get_hd_encoder_status(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HD_ENCODER_STATUS);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_encoder_status);
mt_u8   reg_aria_hd_encoder_get_hd_encoder_status_new_addr_flag(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_encoder_status_t *)
        REG_ARIA_HD_ENCODER_HD_ENCODER_STATUS).bitc.new_addr_flag;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_encoder_status_new_addr_flag);
mt_u8   reg_aria_hd_encoder_get_hd_encoder_status_o_even_field(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_encoder_status_t *)
        REG_ARIA_HD_ENCODER_HD_ENCODER_STATUS).bitc.o_even_field;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_encoder_status_o_even_field);
mt_u8   reg_aria_hd_encoder_get_hd_encoder_status_o_top_field(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_encoder_status_t *)
        REG_ARIA_HD_ENCODER_HD_ENCODER_STATUS).bitc.o_top_field;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_encoder_status_o_top_field);
mt_u8   reg_aria_hd_encoder_get_hd_encoder_status_update_disp_h_timing(void)
{
    return (*(volatile reg_aria_hd_encoder_hd_encoder_status_t *)
        REG_ARIA_HD_ENCODER_HD_ENCODER_STATUS).bitc.update_disp_h_timing;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hd_encoder_status_update_disp_h_timing);

/*!
  register ARIA_HD_ENCODER_HDMI_H_FPorchWidth (read/write)
  */
void reg_aria_hd_encoder_set_hdmi_h_fporchwidth(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_H_FPORCHWIDTH, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_h_fporchwidth);

mt_u32  reg_aria_hd_encoder_get_hdmi_h_fporchwidth(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_H_FPORCHWIDTH);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_h_fporchwidth);

void reg_aria_hd_encoder_set_hdmi_h_fporchwidth_hdmi_h_fporchwidth(u16 data)
{
    reg_aria_hd_encoder_hdmi_h_fporchwidth_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_H_FPORCHWIDTH;
    d.bitc.hdmi_h_fporchwidth = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_H_FPORCHWIDTH, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_h_fporchwidth_hdmi_h_fporchwidth);

u16  reg_aria_hd_encoder_get_hdmi_h_fporchwidth_hdmi_h_fporchwidth(void)
{
    return (*(volatile reg_aria_hd_encoder_hdmi_h_fporchwidth_t *)
        REG_ARIA_HD_ENCODER_HDMI_H_FPORCHWIDTH).bitc.hdmi_h_fporchwidth;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_h_fporchwidth_hdmi_h_fporchwidth);


/*!
  register ARIA_HD_ENCODER_HDMI_H_SyncWidth (read/write)
  */
void reg_aria_hd_encoder_set_hdmi_h_syncwidth(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_H_SYNCWIDTH, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_h_syncwidth);

mt_u32  reg_aria_hd_encoder_get_hdmi_h_syncwidth(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_H_SYNCWIDTH);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_h_syncwidth);

void reg_aria_hd_encoder_set_hdmi_h_syncwidth_hdmi_h_syncwidth(u16 data)
{
    reg_aria_hd_encoder_hdmi_h_syncwidth_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_H_SYNCWIDTH;
    d.bitc.hdmi_h_syncwidth = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_H_SYNCWIDTH, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_h_syncwidth_hdmi_h_syncwidth);

u16  reg_aria_hd_encoder_get_hdmi_h_syncwidth_hdmi_h_syncwidth(void)
{
    return (*(volatile reg_aria_hd_encoder_hdmi_h_syncwidth_t *)
        REG_ARIA_HD_ENCODER_HDMI_H_SYNCWIDTH).bitc.hdmi_h_syncwidth;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_h_syncwidth_hdmi_h_syncwidth);


/*!
  register ARIA_HD_ENCODER_HDMI_H_BPorchWidth (read/write)
  */
void reg_aria_hd_encoder_set_hdmi_h_bporchwidth(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_H_BPORCHWIDTH, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_h_bporchwidth);

mt_u32  reg_aria_hd_encoder_get_hdmi_h_bporchwidth(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_H_BPORCHWIDTH);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_h_bporchwidth);

void reg_aria_hd_encoder_set_hdmi_h_bporchwidth_hdmi_h_bporchwidth(u16 data)
{
    reg_aria_hd_encoder_hdmi_h_bporchwidth_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_H_BPORCHWIDTH;
    d.bitc.hdmi_h_bporchwidth = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_H_BPORCHWIDTH, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_h_bporchwidth_hdmi_h_bporchwidth);

u16  reg_aria_hd_encoder_get_hdmi_h_bporchwidth_hdmi_h_bporchwidth(void)
{
    return (*(volatile reg_aria_hd_encoder_hdmi_h_bporchwidth_t *)
        REG_ARIA_HD_ENCODER_HDMI_H_BPORCHWIDTH).bitc.hdmi_h_bporchwidth;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_h_bporchwidth_hdmi_h_bporchwidth);


/*!
  register ARIA_HD_ENCODER_HDMI_H_ActiveWidth (read/write)
  */
void reg_aria_hd_encoder_set_hdmi_h_activewidth(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_H_ACTIVEWIDTH, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_h_activewidth);

mt_u32  reg_aria_hd_encoder_get_hdmi_h_activewidth(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_H_ACTIVEWIDTH);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_h_activewidth);

void reg_aria_hd_encoder_set_hdmi_h_activewidth_hdmi_h_activewidth(u16 data)
{
    reg_aria_hd_encoder_hdmi_h_activewidth_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_H_ACTIVEWIDTH;
    d.bitc.hdmi_h_activewidth = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_H_ACTIVEWIDTH, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_h_activewidth_hdmi_h_activewidth);

u16  reg_aria_hd_encoder_get_hdmi_h_activewidth_hdmi_h_activewidth(void)
{
    return (*(volatile reg_aria_hd_encoder_hdmi_h_activewidth_t *)
        REG_ARIA_HD_ENCODER_HDMI_H_ACTIVEWIDTH).bitc.hdmi_h_activewidth;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_h_activewidth_hdmi_h_activewidth);


/*!
  register ARIA_HD_ENCODER_HDMI_V_FPorchHeight (read/write)
  */
void reg_aria_hd_encoder_set_hdmi_v_fporchheight(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_V_FPORCHHEIGHT, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_v_fporchheight);

mt_u32  reg_aria_hd_encoder_get_hdmi_v_fporchheight(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_V_FPORCHHEIGHT);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_v_fporchheight);

void reg_aria_hd_encoder_set_hdmi_v_fporchheight_hdmi_v_fporchheight(u16 data)
{
    reg_aria_hd_encoder_hdmi_v_fporchheight_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_V_FPORCHHEIGHT;
    d.bitc.hdmi_v_fporchheight = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_V_FPORCHHEIGHT, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_v_fporchheight_hdmi_v_fporchheight);

u16  reg_aria_hd_encoder_get_hdmi_v_fporchheight_hdmi_v_fporchheight(void)
{
    return (*(volatile reg_aria_hd_encoder_hdmi_v_fporchheight_t *)
        REG_ARIA_HD_ENCODER_HDMI_V_FPORCHHEIGHT).bitc.hdmi_v_fporchheight;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_v_fporchheight_hdmi_v_fporchheight);


/*!
  register ARIA_HD_ENCODER_HDMI_V_SyncHeight (read/write)
  */
void reg_aria_hd_encoder_set_hdmi_v_syncheight(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_V_SYNCHEIGHT, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_v_syncheight);

mt_u32  reg_aria_hd_encoder_get_hdmi_v_syncheight(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_V_SYNCHEIGHT);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_v_syncheight);

void reg_aria_hd_encoder_set_hdmi_v_syncheight_hdmi_v_syncheight(u16 data)
{
    reg_aria_hd_encoder_hdmi_v_syncheight_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_V_SYNCHEIGHT;
    d.bitc.hdmi_v_syncheight = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_V_SYNCHEIGHT, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_v_syncheight_hdmi_v_syncheight);

u16  reg_aria_hd_encoder_get_hdmi_v_syncheight_hdmi_v_syncheight(void)
{
    return (*(volatile reg_aria_hd_encoder_hdmi_v_syncheight_t *)
        REG_ARIA_HD_ENCODER_HDMI_V_SYNCHEIGHT).bitc.hdmi_v_syncheight;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_v_syncheight_hdmi_v_syncheight);


/*!
  register ARIA_HD_ENCODER_HDMI_V_BPorchHeight (read/write)
  */
void reg_aria_hd_encoder_set_hdmi_v_bporchheight(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_V_BPORCHHEIGHT, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_v_bporchheight);

mt_u32  reg_aria_hd_encoder_get_hdmi_v_bporchheight(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_V_BPORCHHEIGHT);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_v_bporchheight);

void reg_aria_hd_encoder_set_hdmi_v_bporchheight_hdmi_v_bporchheight(u16 data)
{
    reg_aria_hd_encoder_hdmi_v_bporchheight_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_V_BPORCHHEIGHT;
    d.bitc.hdmi_v_bporchheight = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_V_BPORCHHEIGHT, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_v_bporchheight_hdmi_v_bporchheight);

u16  reg_aria_hd_encoder_get_hdmi_v_bporchheight_hdmi_v_bporchheight(void)
{
    return (*(volatile reg_aria_hd_encoder_hdmi_v_bporchheight_t *)
        REG_ARIA_HD_ENCODER_HDMI_V_BPORCHHEIGHT).bitc.hdmi_v_bporchheight;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_v_bporchheight_hdmi_v_bporchheight);


/*!
  register ARIA_HD_ENCODER_HDMI_V_ActiveHeight (read/write)
  */
void reg_aria_hd_encoder_set_hdmi_v_activeheight(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_V_ACTIVEHEIGHT, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_v_activeheight);

mt_u32  reg_aria_hd_encoder_get_hdmi_v_activeheight(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_V_ACTIVEHEIGHT);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_v_activeheight);

void reg_aria_hd_encoder_set_hdmi_v_activeheight_hdmi_v_activeheight(u16 data)
{
    reg_aria_hd_encoder_hdmi_v_activeheight_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_V_ACTIVEHEIGHT;
    d.bitc.hdmi_v_activeheight = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_V_ACTIVEHEIGHT, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_v_activeheight_hdmi_v_activeheight);

u16  reg_aria_hd_encoder_get_hdmi_v_activeheight_hdmi_v_activeheight(void)
{
    return (*(volatile reg_aria_hd_encoder_hdmi_v_activeheight_t *)
        REG_ARIA_HD_ENCODER_HDMI_V_ACTIVEHEIGHT).bitc.hdmi_v_activeheight;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_v_activeheight_hdmi_v_activeheight);


/*!
  register ARIA_HD_ENCODER_HDMI_H_HalfLineWidth (read/write)
  */
void reg_aria_hd_encoder_set_hdmi_h_halflinewidth(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_H_HALFLINEWIDTH, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_h_halflinewidth);

mt_u32  reg_aria_hd_encoder_get_hdmi_h_halflinewidth(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_H_HALFLINEWIDTH);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_h_halflinewidth);

void reg_aria_hd_encoder_set_hdmi_h_halflinewidth_hdmi_h_halflinewidth(u16 data)
{
    reg_aria_hd_encoder_hdmi_h_halflinewidth_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_H_HALFLINEWIDTH;
    d.bitc.hdmi_h_halflinewidth = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_H_HALFLINEWIDTH, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_h_halflinewidth_hdmi_h_halflinewidth);

u16  reg_aria_hd_encoder_get_hdmi_h_halflinewidth_hdmi_h_halflinewidth(void)
{
    return (*(volatile reg_aria_hd_encoder_hdmi_h_halflinewidth_t *)
        REG_ARIA_HD_ENCODER_HDMI_H_HALFLINEWIDTH).bitc.hdmi_h_halflinewidth;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_h_halflinewidth_hdmi_h_halflinewidth);


/*!
  register ARIA_HD_ENCODER_HDMI_HSync_Pola (read/write)
  */
void reg_aria_hd_encoder_set_hdmi_hsync_pola(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_HSYNC_POLA, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_hsync_pola);

mt_u32  reg_aria_hd_encoder_get_hdmi_hsync_pola(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_HSYNC_POLA);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_hsync_pola);

void reg_aria_hd_encoder_set_hdmi_hsync_pola_hdmi_hsync_pola(mt_u8 data)
{
    reg_aria_hd_encoder_hdmi_hsync_pola_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_HSYNC_POLA;
    d.bitc.hdmi_hsync_pola = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_HSYNC_POLA, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_hsync_pola_hdmi_hsync_pola);

mt_u8   reg_aria_hd_encoder_get_hdmi_hsync_pola_hdmi_hsync_pola(void)
{
    return (*(volatile reg_aria_hd_encoder_hdmi_hsync_pola_t *)
        REG_ARIA_HD_ENCODER_HDMI_HSYNC_POLA).bitc.hdmi_hsync_pola;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_hsync_pola_hdmi_hsync_pola);


/*!
  register ARIA_HD_ENCODER_HDMI_VSync_Pola (read/write)
  */
void reg_aria_hd_encoder_set_hdmi_vsync_pola(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_VSYNC_POLA, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_vsync_pola);

mt_u32  reg_aria_hd_encoder_get_hdmi_vsync_pola(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_VSYNC_POLA);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_vsync_pola);

void reg_aria_hd_encoder_set_hdmi_vsync_pola_hdmi_vsync_pola(mt_u8 data)
{
    reg_aria_hd_encoder_hdmi_vsync_pola_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_VSYNC_POLA;
    d.bitc.hdmi_vsync_pola = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_HDMI_VSYNC_POLA, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_hdmi_vsync_pola_hdmi_vsync_pola);

mt_u8   reg_aria_hd_encoder_get_hdmi_vsync_pola_hdmi_vsync_pola(void)
{
    return (*(volatile reg_aria_hd_encoder_hdmi_vsync_pola_t *)
        REG_ARIA_HD_ENCODER_HDMI_VSYNC_POLA).bitc.hdmi_vsync_pola;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_hdmi_vsync_pola_hdmi_vsync_pola);


/*!
  register ARIA_HD_ENCODER_DAC_SEL (read/write)
  */
void reg_aria_hd_encoder_set_dac_sel(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_DAC_SEL, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_dac_sel);

mt_u32  reg_aria_hd_encoder_get_dac_sel(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_DAC_SEL);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_dac_sel);

void reg_aria_hd_encoder_set_dac_sel_dac_3_mode(mt_u8 data)
{
    reg_aria_hd_encoder_dac_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_DAC_SEL;
    d.bitc.dac_3_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_DAC_SEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_dac_sel_dac_3_mode);

mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_3_mode(void)
{
    return (*(volatile reg_aria_hd_encoder_dac_sel_t *)
        REG_ARIA_HD_ENCODER_DAC_SEL).bitc.dac_3_mode;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_dac_sel_dac_3_mode);

void reg_aria_hd_encoder_set_dac_sel_dac_2_mode(mt_u8 data)
{
    reg_aria_hd_encoder_dac_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_DAC_SEL;
    d.bitc.dac_2_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_DAC_SEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_dac_sel_dac_2_mode);

mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_2_mode(void)
{
    return (*(volatile reg_aria_hd_encoder_dac_sel_t *)
        REG_ARIA_HD_ENCODER_DAC_SEL).bitc.dac_2_mode;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_dac_sel_dac_2_mode);

void reg_aria_hd_encoder_set_dac_sel_dac_1_mode(mt_u8 data)
{
    reg_aria_hd_encoder_dac_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_DAC_SEL;
    d.bitc.dac_1_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_DAC_SEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_dac_sel_dac_1_mode);

mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_1_mode(void)
{
    return (*(volatile reg_aria_hd_encoder_dac_sel_t *)
        REG_ARIA_HD_ENCODER_DAC_SEL).bitc.dac_1_mode;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_dac_sel_dac_1_mode);

void reg_aria_hd_encoder_set_dac_sel_dac_0_mode(mt_u8 data)
{
    reg_aria_hd_encoder_dac_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_DAC_SEL;
    d.bitc.dac_0_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_DAC_SEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_dac_sel_dac_0_mode);

mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_0_mode(void)
{
    return (*(volatile reg_aria_hd_encoder_dac_sel_t *)
        REG_ARIA_HD_ENCODER_DAC_SEL).bitc.dac_0_mode;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_dac_sel_dac_0_mode);

void reg_aria_hd_encoder_set_dac_sel_dac_3_hdsd_sel(mt_u8 data)
{
    reg_aria_hd_encoder_dac_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_DAC_SEL;
    d.bitc.dac_3_hdsd_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_DAC_SEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_dac_sel_dac_3_hdsd_sel);

mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_3_hdsd_sel(void)
{
    return (*(volatile reg_aria_hd_encoder_dac_sel_t *)
        REG_ARIA_HD_ENCODER_DAC_SEL).bitc.dac_3_hdsd_sel;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_dac_sel_dac_3_hdsd_sel);

void reg_aria_hd_encoder_set_dac_sel_dac_2_hdsd_sel(mt_u8 data)
{
    reg_aria_hd_encoder_dac_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_DAC_SEL;
    d.bitc.dac_2_hdsd_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_DAC_SEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_dac_sel_dac_2_hdsd_sel);

mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_2_hdsd_sel(void)
{
    return (*(volatile reg_aria_hd_encoder_dac_sel_t *)
        REG_ARIA_HD_ENCODER_DAC_SEL).bitc.dac_2_hdsd_sel;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_dac_sel_dac_2_hdsd_sel);

void reg_aria_hd_encoder_set_dac_sel_dac_1_hdsd_sel(mt_u8 data)
{
    reg_aria_hd_encoder_dac_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_DAC_SEL;
    d.bitc.dac_1_hdsd_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_DAC_SEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_dac_sel_dac_1_hdsd_sel);

mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_1_hdsd_sel(void)
{
    return (*(volatile reg_aria_hd_encoder_dac_sel_t *)
        REG_ARIA_HD_ENCODER_DAC_SEL).bitc.dac_1_hdsd_sel;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_dac_sel_dac_1_hdsd_sel);

void reg_aria_hd_encoder_set_dac_sel_dac_0_hdsd_sel(mt_u8 data)
{
    reg_aria_hd_encoder_dac_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_DAC_SEL;
    d.bitc.dac_0_hdsd_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_DAC_SEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_dac_sel_dac_0_hdsd_sel);

mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_0_hdsd_sel(void)
{
    return (*(volatile reg_aria_hd_encoder_dac_sel_t *)
        REG_ARIA_HD_ENCODER_DAC_SEL).bitc.dac_0_hdsd_sel;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_dac_sel_dac_0_hdsd_sel);

void reg_aria_hd_encoder_set_dac_sel_hdmi_mode(mt_u8 data)
{
    reg_aria_hd_encoder_dac_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_DAC_SEL;
    d.bitc.hdmi_422_444 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_DAC_SEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_dac_sel_hdmi_mode);

mt_u8   reg_aria_hd_encoder_get_dac_sel_hdmi_mode(void)
{
    return (*(volatile reg_aria_hd_encoder_dac_sel_t *)
        REG_ARIA_HD_ENCODER_DAC_SEL).bitc.hdmi_422_444;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_dac_sel_hdmi_mode);

void reg_aria_hd_encoder_set_dac_sel_cbcr_order(mt_u8 data)
{
    reg_aria_hd_encoder_dac_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_DAC_SEL;
    d.bitc.cbcr_order = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_DAC_SEL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_dac_sel_cbcr_order);

mt_u8   reg_aria_hd_encoder_get_dac_sel_cbcr_order(void)
{
    return (*(volatile reg_aria_hd_encoder_dac_sel_t *)
        REG_ARIA_HD_ENCODER_DAC_SEL).bitc.cbcr_order;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_dac_sel_cbcr_order);

void reg_aria_hd_encoder_set_oversample(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_oversample);

void reg_aria_hd_encoder_set_vdac_adjusting_y(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_VDAC_ADJUSTING_Y, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_vdac_adjusting_y);

void reg_aria_hd_encoder_set_vdac_adjusting_uv(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_VDAC_ADJUSTING_UV, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_vdac_adjusting_uv);

/*!
  register SYMPHONY_HD_ENCODER_OS_FILTER_CTRL (read/write)
  */
void reg_aria_hd_encoder_set_os_filter_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_os_filter_ctrl);

mt_u32  reg_aria_hd_encoder_get_os_filter_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_os_filter_ctrl);

void reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_4_3(mt_u8 data)
{
    reg_aria_hd_encoder_os_filter_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL;
    d.bitc.mux_sdp_4_3 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_4_3);

mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_4_3(void)
{
    return (*(volatile reg_aria_hd_encoder_os_filter_ctrl_t *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL).bitc.mux_sdp_4_3;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_4_3);

void reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_6_5(mt_u8 data)
{
    reg_aria_hd_encoder_os_filter_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL;
    d.bitc.mux_sdp_6_5 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_6_5);

mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_6_5(void)
{
    return (*(volatile reg_aria_hd_encoder_os_filter_ctrl_t *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL).bitc.mux_sdp_6_5;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_6_5);

void reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_7(u8 data)
{
    reg_aria_hd_encoder_os_filter_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL;
    d.bitc.mux_sdp_7 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_7);

mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_7(void)
{
    return (*(volatile reg_aria_hd_encoder_os_filter_ctrl_t *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL).bitc.mux_sdp_7;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_7);

void reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_8(mt_u8 data)
{
    reg_aria_hd_encoder_os_filter_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL;
    d.bitc.mux_sdp_8 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_8);

mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_8(void)
{
    return (*(volatile reg_aria_hd_encoder_os_filter_ctrl_t *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL).bitc.mux_sdp_8;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_8);

void reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(mt_u8 data)
{
    reg_aria_hd_encoder_os_filter_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL;
    d.bitc.mux_sdp_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0);

mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_0(void)
{
    return (*(volatile reg_aria_hd_encoder_os_filter_ctrl_t *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL).bitc.mux_sdp_0;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_0);

void reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(u8 data)
{
    reg_aria_hd_encoder_os_filter_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL;
    d.bitc.mux_sdp_2_1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1);

mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_2_1(void)
{
    return (*(volatile reg_aria_hd_encoder_os_filter_ctrl_t *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL).bitc.mux_sdp_2_1;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_2_1);

void reg_aria_hd_encoder_set_os_filter_ctrl_os_en(mt_u8 data)
{
    reg_aria_hd_encoder_os_filter_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL;
    d.bitc.os_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_os_filter_ctrl_os_en);

mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_os_en(void)
{
    return (*(volatile reg_aria_hd_encoder_os_filter_ctrl_t *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL).bitc.os_en;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_os_filter_ctrl_os_en);

void reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(mt_u8 data)
{
    reg_aria_hd_encoder_os_filter_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL;
    d.bitc.sample_ctrl = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl);

mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_sample_ctrl(void)
{
    return (*(volatile reg_aria_hd_encoder_os_filter_ctrl_t *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL).bitc.sample_ctrl;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_os_filter_ctrl_sample_ctrl);

void reg_aria_hd_encoder_set_os_filter_ctrl_sync_filter_en(mt_u8 data)
{
    reg_aria_hd_encoder_os_filter_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL;
    d.bitc.sync_filter_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_os_filter_ctrl_sync_filter_en);

mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_sync_filter_en(void)
{
    return (*(volatile reg_aria_hd_encoder_os_filter_ctrl_t *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL).bitc.sync_filter_en;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_os_filter_ctrl_sync_filter_en);

void reg_aria_hd_encoder_set_os_filter_ctrl_int_state_rd_clr_en(u8 data)
{
    reg_aria_hd_encoder_os_filter_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL;
    d.bitc.int_state_rd_clr_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL, d.all);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_os_filter_ctrl_int_state_rd_clr_en);

mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_int_state_rd_clr_en(void)
{
    return (*(volatile reg_aria_hd_encoder_os_filter_ctrl_t *)REG_ARIA_HD_ENCODER_OS_FILTER_CTRL).bitc.int_state_rd_clr_en;
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_os_filter_ctrl_int_state_rd_clr_en);

void reg_aria_hd_encoder_set_sign_ctrl(u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_SIGN_CTRL, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_sign_ctrl);

mt_u32  reg_aria_hd_encoder_get_sign_ctrl(void)
{
    return (*(volatile u32 *)REG_ARIA_HD_ENCODER_SIGN_CTRL);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_sign_ctrl);

void reg_aria_hd_encoder_set_mask_irq(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_HD_ENCODER_MASK_IRQ, data);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_set_mask_irq);

mt_u32  reg_aria_hd_encoder_get_mask_irq(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_HD_ENCODER_MASK_IRQ);
}
EXPORT_SYMBOL(reg_aria_hd_encoder_get_mask_irq);

/*!
  init function
  */
void reg_aria_hd_encoder_init(void)
{
    reg_aria_hd_encoder_set_basic_cfg((mt_u32)0x00500a10);
    reg_aria_hd_encoder_set_y_parameter_level0((mt_u32)0x00fc0281);
    reg_aria_hd_encoder_set_y_sync_level((mt_u32)0x01e80010);
    reg_aria_hd_encoder_set_pbpr_parameter0_level((mt_u32)0x00000200);
    reg_aria_hd_encoder_set_pbpr_parameter1_level((mt_u32)0x00000281);
    reg_aria_hd_encoder_set_hd_cfg_info((mt_u32)0xa000002f);
    reg_aria_hd_encoder_set_hsync_parameter0((mt_u32)0x0058002c);
    reg_aria_hd_encoder_set_hsync_parameter1((mt_u32)0x03700058);
    reg_aria_hd_encoder_set_reg_h_active_parameter0((mt_u32)0x0058002c);
    reg_aria_hd_encoder_set_reg_h_active_parameter1((mt_u32)0x000507bc);
    reg_aria_hd_encoder_set_reg_v_parameter0((mt_u32)0x000a0004);
    reg_aria_hd_encoder_set_reg_v_parameter1((mt_u32)0x0438001e);
    reg_aria_hd_encoder_set_reg_h_osd_parameter((mt_u32)0x07800001);
    reg_aria_hd_encoder_set_reg_v_osd_parameter((mt_u32)0x04380005);
    reg_aria_hd_encoder_set_reg_h_video_parameter((mt_u32)0x07800001);
    reg_aria_hd_encoder_set_reg_v_video_parameter((mt_u32)0x0438000a);
    reg_aria_hd_encoder_set_reg_color_bar((mt_u32)0x600000f0);
    reg_aria_hd_encoder_set_cbar_set_data((mt_u32)0x200802d1);
    reg_aria_hd_encoder_set_gama_correction((mt_u32)0x4000003b);
    reg_aria_hd_encoder_set_hdmi_h_fporchwidth((mt_u32)0x00000210);
    reg_aria_hd_encoder_set_hdmi_h_syncwidth((mt_u32)0x0000002c);
    reg_aria_hd_encoder_set_hdmi_h_bporchwidth((mt_u32)0x00000094);
    reg_aria_hd_encoder_set_hdmi_h_activewidth((mt_u32)0x00000780);
    reg_aria_hd_encoder_set_hdmi_v_fporchheight((mt_u32)0x00000002);
    reg_aria_hd_encoder_set_hdmi_v_syncheight((mt_u32)0x00000005);
    reg_aria_hd_encoder_set_hdmi_v_bporchheight((mt_u32)0x0000000f);
    reg_aria_hd_encoder_set_hdmi_v_activeheight((mt_u32)0x0000021c);
    reg_aria_hd_encoder_set_hdmi_h_halflinewidth((mt_u32)0x00000468);
    reg_aria_hd_encoder_set_hdmi_hsync_pola((mt_u32)0x00000000);
    reg_aria_hd_encoder_set_hdmi_vsync_pola((mt_u32)0x00000000);
    reg_aria_hd_encoder_set_dac_sel((mt_u32)0x00000006);
    /* read read-clear registers in order to set mirror variables */
}
EXPORT_SYMBOL(reg_aria_hd_encoder_init);

/*!
  end of file
  */

