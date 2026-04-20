/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
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
#include "sd_enc_aria_reg.h"
//#include "disp_ap_aria_reg.h"

/*!
  Write 32 bits register
  
  \param[in] addr register address
  \param[in] p_addr data to write
  */
static inline void hal_put_u32(volatile unsigned long *p_addr, 
                                                 unsigned long data)
{
    
  *((volatile mt_u32 *)(p_addr)) = data;
  
}

void reg_aria_sd_enc_set_sd_enc_mode(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_SD_ENC_SD_ENC_MODE, data);
}

mt_u32  reg_aria_sd_enc_get_sd_enc_mode(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_SD_ENC_SD_ENC_MODE);
}

void reg_aria_sd_enc_set_sd_enc_mode_vid_mode(mt_u8 data)
{
    reg_aria_sd_enc_sd_enc_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_SD_ENC_SD_ENC_MODE;
    d.bitc.vid_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_SD_ENC_SD_ENC_MODE, d.all);
}

mt_u8   reg_aria_sd_enc_get_sd_enc_mode_vid_mode(void)
{
    return (*(volatile reg_aria_sd_enc_sd_enc_mode_t *)
    REG_ARIA_SD_ENC_SD_ENC_MODE).bitc.vid_mode;
}


/*!
  register ARIA_SD_ENC_SD_ENC_CFG6 (read/write)
  */
void reg_aria_sd_enc_set_cfg6(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_SD_ENC_CFIG6, data);
}

mt_u32  reg_aria_sd_enc_get_cfg6(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_SD_ENC_CFIG6);
}

void reg_aria_sd_enc_set_cfg6_video_fmt(mt_u8 data)
{
    reg_aria_sd_enc_sd_enc_cfg6_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_SD_ENC_CFIG6;
    d.bitc.video_fmt = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_SD_ENC_CFIG6, d.all);
}

mt_u8   reg_aria_sd_enc_get_cfg6_video_fmt(void)
{
    return (*(volatile reg_aria_sd_enc_sd_enc_cfg6_t *)
    REG_ARIA_SD_ENC_CFIG6).bitc.video_fmt;
}


/*!
  register ARIA_SD_ENC_SD_ENC_DACNUM (read/write)
  */
void reg_aria_sd_enc_set_sd_enc_dacnum(mt_u32 data)
{
   hal_put_u32((volatile unsigned long *)REG_ARIA_SD_ENC_SD_ENC_DACNUM, data);
}

mt_u32  reg_aria_sd_enc_get_sd_enc_dacnum(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_SD_ENC_SD_ENC_DACNUM);
}

void reg_aria_sd_enc_set_sd_enc_dacnum_dac_switch(mt_u8 data)
{
    reg_aria_sd_enc_sd_enc_dacnum_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_SD_ENC_SD_ENC_DACNUM;
    d.bitc.dac_switch_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_SD_ENC_SD_ENC_DACNUM, d.all);
}

mt_u8   reg_aria_sd_enc_get_sd_enc_dacnum_dac_switch(void)
{
    return (*(volatile reg_aria_sd_enc_sd_enc_dacnum_t *)
    REG_ARIA_SD_ENC_SD_ENC_DACNUM).bitc.dac_switch_en;
}

void reg_aria_sd_encoder_set_gama_correction_gama_coef(mt_u8 data)
{
    reg_aria_sd_enc_sd_enc_curve_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_SD_ENC_SD_ENC_CURVE;
    d.bitc.gamma_amp = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_SD_ENC_SD_ENC_CURVE, d.all);
}

void reg_aria_sd_encoder_set_gama_correction_gama_pole(mt_u8 data)
{
    reg_aria_sd_enc_sd_enc_curve_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_SD_ENC_SD_ENC_CURVE;
    d.bitc.gamma_pole = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_SD_ENC_SD_ENC_CURVE, d.all);
}

void reg_aria_sd_encoder_set_gama_correction_gama_enable(mt_u8 data)
{
    reg_aria_sd_enc_sd_enc_curve_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_SD_ENC_SD_ENC_CURVE;
    d.bitc.gamma_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_SD_ENC_SD_ENC_CURVE, d.all);
}

/*!
  init function
  */
void reg_aria_sd_enc_init(void)
{
    reg_aria_sd_enc_set_sd_enc_mode((mt_u32)0x00000000);
    reg_aria_sd_enc_set_cfg6((mt_u32)0x00000000);
    reg_aria_sd_enc_set_sd_enc_dacnum((mt_u32)0x00000000);
    /* read read-clear registers in order to set mirror variables */
}

/*****************************************************************/


/*!
  register SYMPHONY_SD_ENCODER_MODE (read/write)
  */
void reg_symphony_sd_encoder_set_mode(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_MODE, data);
}

mt_u32  reg_symphony_sd_encoder_get_mode(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_MODE);
}

void reg_symphony_sd_encoder_set_mode_vid_mode(mt_u8 data)
{
    reg_symphony_sd_encoder_mode_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_MODE;
    d.bitc.vid_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_MODE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_mode_vid_mode(void)
{
    return (*(volatile reg_symphony_sd_encoder_mode_t *)REG_SYMPHONY_SD_ENCODER_MODE).bitc.vid_mode;
}

void reg_symphony_sd_encoder_set_mode_colorbar_sel(mt_u8 data)
{
    reg_symphony_sd_encoder_mode_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_MODE;
    d.bitc.colorbar_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_MODE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_mode_colorbar_sel(void)
{
    return (*(volatile reg_symphony_sd_encoder_mode_t *)REG_SYMPHONY_SD_ENCODER_MODE).bitc.colorbar_sel;
}

void reg_symphony_sd_encoder_set_mode_sync_switch(mt_u8 data)
{
    reg_symphony_sd_encoder_mode_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_MODE;
    d.bitc.sync_switch = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_MODE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_mode_sync_switch(void)
{
    return (*(volatile reg_symphony_sd_encoder_mode_t *)REG_SYMPHONY_SD_ENCODER_MODE).bitc.sync_switch;
}

void reg_symphony_sd_encoder_set_mode_vid_test_mode(mt_u8 data)
{
    reg_symphony_sd_encoder_mode_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_MODE;
    d.bitc.vid_test_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_MODE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_mode_vid_test_mode(void)
{
    return (*(volatile reg_symphony_sd_encoder_mode_t *)REG_SYMPHONY_SD_ENCODER_MODE).bitc.vid_test_mode;
}

void reg_symphony_sd_encoder_set_mode_dac_test_en(mt_u8 data)
{
    reg_symphony_sd_encoder_mode_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_MODE;
    d.bitc.dac_test_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_MODE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_mode_dac_test_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_mode_t *)REG_SYMPHONY_SD_ENCODER_MODE).bitc.dac_test_en;
}

void reg_symphony_sd_encoder_set_mode_dac0_sync_en(mt_u8 data)
{
    reg_symphony_sd_encoder_mode_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_MODE;
    d.bitc.dac0_sync_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_MODE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_mode_dac0_sync_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_mode_t *)REG_SYMPHONY_SD_ENCODER_MODE).bitc.dac0_sync_en;
}

void reg_symphony_sd_encoder_set_mode_cable_detect_en(mt_u8 data)
{
    reg_symphony_sd_encoder_mode_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_MODE;
    d.bitc.cable_detect_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_MODE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_mode_cable_detect_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_mode_t *)REG_SYMPHONY_SD_ENCODER_MODE).bitc.cable_detect_en;
}

void reg_symphony_sd_encoder_set_mode_vid_insert_mask_en(mt_u8 data)
{
    reg_symphony_sd_encoder_mode_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_MODE;
    d.bitc.vid_insert_mask_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_MODE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_mode_vid_insert_mask_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_mode_t *)REG_SYMPHONY_SD_ENCODER_MODE).bitc.vid_insert_mask_en;
}


/*!
  register SYMPHONY_SD_ENCODER_CURVE (read/write)
  */
void reg_symphony_sd_encoder_set_curve(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CURVE, data);
}

mt_u32  reg_symphony_sd_encoder_get_curve(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CURVE);
}

void reg_symphony_sd_encoder_set_curve_trap_en(mt_u8 data)
{
    reg_symphony_sd_encoder_curve_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CURVE;
    d.bitc.trap_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CURVE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_curve_trap_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_curve_t *)REG_SYMPHONY_SD_ENCODER_CURVE).bitc.trap_en;
}

void reg_symphony_sd_encoder_set_curve_trap_sel(mt_u8 data)
{
    reg_symphony_sd_encoder_curve_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CURVE;
    d.bitc.trap_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CURVE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_curve_trap_sel(void)
{
    return (*(volatile reg_symphony_sd_encoder_curve_t *)REG_SYMPHONY_SD_ENCODER_CURVE).bitc.trap_sel;
}

void reg_symphony_sd_encoder_set_curve_chrom_filter_sel(mt_u8 data)
{
    reg_symphony_sd_encoder_curve_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CURVE;
    d.bitc.chrom_filter_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CURVE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_curve_chrom_filter_sel(void)
{
    return (*(volatile reg_symphony_sd_encoder_curve_t *)REG_SYMPHONY_SD_ENCODER_CURVE).bitc.chrom_filter_sel;
}

void reg_symphony_sd_encoder_set_curve_over_sample_mode(mt_u8 data)
{
    reg_symphony_sd_encoder_curve_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CURVE;
    d.bitc.over_sample_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CURVE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_curve_over_sample_mode(void)
{
    return (*(volatile reg_symphony_sd_encoder_curve_t *)REG_SYMPHONY_SD_ENCODER_CURVE).bitc.over_sample_mode;
}

void reg_symphony_sd_encoder_set_curve_gamma_amp(mt_u8 data)
{
    reg_symphony_sd_encoder_curve_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CURVE;
    d.bitc.gamma_amp = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CURVE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_curve_gamma_amp(void)
{
    return (*(volatile reg_symphony_sd_encoder_curve_t *)REG_SYMPHONY_SD_ENCODER_CURVE).bitc.gamma_amp;
}

void reg_symphony_sd_encoder_set_curve_gamma_pole(mt_u8 data)
{
    reg_symphony_sd_encoder_curve_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CURVE;
    d.bitc.gamma_pole = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CURVE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_curve_gamma_pole(void)
{
    return (*(volatile reg_symphony_sd_encoder_curve_t *)REG_SYMPHONY_SD_ENCODER_CURVE).bitc.gamma_pole;
}

void reg_symphony_sd_encoder_set_curve_gamma_en(mt_u8 data)
{
    reg_symphony_sd_encoder_curve_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CURVE;
    d.bitc.gamma_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CURVE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_curve_gamma_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_curve_t *)REG_SYMPHONY_SD_ENCODER_CURVE).bitc.gamma_en;
}


/*!
  register SYMPHONY_SD_ENCODER_DELAY (read/write)
  */
void reg_symphony_sd_encoder_set_delay(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DELAY, data);
}

mt_u32  reg_symphony_sd_encoder_get_delay(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DELAY);
}

void reg_symphony_sd_encoder_set_delay_y_sync(mt_u8 data)
{
    reg_symphony_sd_encoder_delay_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DELAY;
    d.bitc.y_sync = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DELAY, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_delay_y_sync(void)
{
    return (*(volatile reg_symphony_sd_encoder_delay_t *)REG_SYMPHONY_SD_ENCODER_DELAY).bitc.y_sync;
}

void reg_symphony_sd_encoder_set_delay_cvbs_delay_n(mt_u8 data)
{
    reg_symphony_sd_encoder_delay_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DELAY;
    d.bitc.cvbs_delay_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DELAY, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_delay_cvbs_delay_n(void)
{
    return (*(volatile reg_symphony_sd_encoder_delay_t *)REG_SYMPHONY_SD_ENCODER_DELAY).bitc.cvbs_delay_n;
}

void reg_symphony_sd_encoder_set_delay_cvbs_delay_p(mt_u8 data)
{
    reg_symphony_sd_encoder_delay_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DELAY;
    d.bitc.cvbs_delay_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DELAY, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_delay_cvbs_delay_p(void)
{
    return (*(volatile reg_symphony_sd_encoder_delay_t *)REG_SYMPHONY_SD_ENCODER_DELAY).bitc.cvbs_delay_p;
}

void reg_symphony_sd_encoder_set_delay_cvbs_lum_delay(mt_u8 data)
{
    reg_symphony_sd_encoder_delay_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DELAY;
    d.bitc.cvbs_lum_delay = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DELAY, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_delay_cvbs_lum_delay(void)
{
    return (*(volatile reg_symphony_sd_encoder_delay_t *)REG_SYMPHONY_SD_ENCODER_DELAY).bitc.cvbs_lum_delay;
}


/*!
  register SYMPHONY_SD_ENCODER_BLANK_P (read/write)
  */
void reg_symphony_sd_encoder_set_blank_p(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_BLANK_P, data);
}

mt_u32  reg_symphony_sd_encoder_get_blank_p(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_P);
}

void reg_symphony_sd_encoder_set_blank_p_y_blank_p(mt_u16 data)
{
    reg_symphony_sd_encoder_blank_p_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_P;
    d.bitc.y_blank_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_BLANK_P, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_blank_p_y_blank_p(void)
{
    return (*(volatile reg_symphony_sd_encoder_blank_p_t *)REG_SYMPHONY_SD_ENCODER_BLANK_P).bitc.y_blank_p;
}

void reg_symphony_sd_encoder_set_blank_p_y_black_p(mt_u16 data)
{
    reg_symphony_sd_encoder_blank_p_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_P;
    d.bitc.y_black_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_BLANK_P, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_blank_p_y_black_p(void)
{
    return (*(volatile reg_symphony_sd_encoder_blank_p_t *)REG_SYMPHONY_SD_ENCODER_BLANK_P).bitc.y_black_p;
}


/*!
  register SYMPHONY_SD_ENCODER_BLANK_N (read/write)
  */
void reg_symphony_sd_encoder_set_blank_n(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_BLANK_N, data);
}

mt_u32  reg_symphony_sd_encoder_get_blank_n(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_N);
}

void reg_symphony_sd_encoder_set_blank_n_y_blank_n(mt_u16 data)
{
    reg_symphony_sd_encoder_blank_n_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_N;
    d.bitc.y_blank_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_BLANK_N, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_blank_n_y_blank_n(void)
{
    return (*(volatile reg_symphony_sd_encoder_blank_n_t *)REG_SYMPHONY_SD_ENCODER_BLANK_N).bitc.y_blank_n;
}

void reg_symphony_sd_encoder_set_blank_n_y_black_n(mt_u16 data)
{
    reg_symphony_sd_encoder_blank_n_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_N;
    d.bitc.y_black_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_BLANK_N, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_blank_n_y_black_n(void)
{
    return (*(volatile reg_symphony_sd_encoder_blank_n_t *)REG_SYMPHONY_SD_ENCODER_BLANK_N).bitc.y_black_n;
}


/*!
  register SYMPHONY_SD_ENCODER_CBLANK (read/write)
  */
void reg_symphony_sd_encoder_set_cblank(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CBLANK, data);
}

mt_u32  reg_symphony_sd_encoder_get_cblank(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CBLANK);
}

void reg_symphony_sd_encoder_set_cblank_c_sync(mt_u16 data)
{
    reg_symphony_sd_encoder_cblank_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CBLANK;
    d.bitc.c_sync = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CBLANK, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_cblank_c_sync(void)
{
    return (*(volatile reg_symphony_sd_encoder_cblank_t *)REG_SYMPHONY_SD_ENCODER_CBLANK).bitc.c_sync;
}

void reg_symphony_sd_encoder_set_cblank_c_blank(mt_u16 data)
{
    reg_symphony_sd_encoder_cblank_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CBLANK;
    d.bitc.c_blank = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CBLANK, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_cblank_c_blank(void)
{
    return (*(volatile reg_symphony_sd_encoder_cblank_t *)REG_SYMPHONY_SD_ENCODER_CBLANK).bitc.c_blank;
}


/*!
  register SYMPHONY_SD_ENCODER_VDAC_PCARRY (read/write)
  */
void reg_symphony_sd_encoder_set_vdac_pcarry(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VDAC_PCARRY, data);
}

mt_u32  reg_symphony_sd_encoder_get_vdac_pcarry(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VDAC_PCARRY);
}

void reg_symphony_sd_encoder_set_vdac_pcarry_subcarrier_p(mt_u32 data)
{
    reg_symphony_sd_encoder_vdac_pcarry_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VDAC_PCARRY;
    d.bitc.subcarrier_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VDAC_PCARRY, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_vdac_pcarry_subcarrier_p(void)
{
    return (*(volatile reg_symphony_sd_encoder_vdac_pcarry_t *)REG_SYMPHONY_SD_ENCODER_VDAC_PCARRY).bitc.subcarrier_p;
}


/*!
  register SYMPHONY_SD_ENCODER_VDAC_NCARRY (read/write)
  */
void reg_symphony_sd_encoder_set_vdac_ncarry(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VDAC_NCARRY, data);
}

mt_u32  reg_symphony_sd_encoder_get_vdac_ncarry(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VDAC_NCARRY);
}

void reg_symphony_sd_encoder_set_vdac_ncarry_subcarrier_n(mt_u32 data)
{
    reg_symphony_sd_encoder_vdac_ncarry_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VDAC_NCARRY;
    d.bitc.subcarrier_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VDAC_NCARRY, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_vdac_ncarry_subcarrier_n(void)
{
    return (*(volatile reg_symphony_sd_encoder_vdac_ncarry_t *)REG_SYMPHONY_SD_ENCODER_VDAC_NCARRY).bitc.subcarrier_n;
}


/*!
  register SYMPHONY_SD_ENCODER_CFIG1 (read/write)
  */
void reg_symphony_sd_encoder_set_cfig1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG1, data);
}

mt_u32  reg_symphony_sd_encoder_get_cfig1(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG1);
}

void reg_symphony_sd_encoder_set_cfig1_bluecoef_p(mt_u16 data)
{
    reg_symphony_sd_encoder_cfig1_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG1;
    d.bitc.bluecoef_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG1, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_cfig1_bluecoef_p(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig1_t *)REG_SYMPHONY_SD_ENCODER_CFIG1).bitc.bluecoef_p;
}

void reg_symphony_sd_encoder_set_cfig1_redcoef_p(mt_u16 data)
{
    reg_symphony_sd_encoder_cfig1_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG1;
    d.bitc.redcoef_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG1, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_cfig1_redcoef_p(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig1_t *)REG_SYMPHONY_SD_ENCODER_CFIG1).bitc.redcoef_p;
}


/*!
  register SYMPHONY_SD_ENCODER_CFIG2 (read/write)
  */
void reg_symphony_sd_encoder_set_cfig2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG2, data);
}

mt_u32  reg_symphony_sd_encoder_get_cfig2(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2);
}

void reg_symphony_sd_encoder_set_cfig2_lumcoef_p(mt_u16 data)
{
    reg_symphony_sd_encoder_cfig2_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2;
    d.bitc.lumcoef_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG2, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_cfig2_lumcoef_p(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig2_t *)REG_SYMPHONY_SD_ENCODER_CFIG2).bitc.lumcoef_p;
}

void reg_symphony_sd_encoder_set_cfig2_burstcoef_p(mt_u16 data)
{
    reg_symphony_sd_encoder_cfig2_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2;
    d.bitc.burstcoef_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG2, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_cfig2_burstcoef_p(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig2_t *)REG_SYMPHONY_SD_ENCODER_CFIG2).bitc.burstcoef_p;
}


/*!
  register SYMPHONY_SD_ENCODER_CFIG3 (read/write)
  */
void reg_symphony_sd_encoder_set_cfig3(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG3, data);
}

mt_u32  reg_symphony_sd_encoder_get_cfig3(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG3);
}

void reg_symphony_sd_encoder_set_cfig3_bluecoef_n(mt_u16 data)
{
    reg_symphony_sd_encoder_cfig3_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG3;
    d.bitc.bluecoef_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG3, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_cfig3_bluecoef_n(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig3_t *)REG_SYMPHONY_SD_ENCODER_CFIG3).bitc.bluecoef_n;
}

void reg_symphony_sd_encoder_set_cfig3_redcoef_n(mt_u16 data)
{
    reg_symphony_sd_encoder_cfig3_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG3;
    d.bitc.redcoef_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG3, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_cfig3_redcoef_n(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig3_t *)REG_SYMPHONY_SD_ENCODER_CFIG3).bitc.redcoef_n;
}


/*!
  register SYMPHONY_SD_ENCODER_CFIG4 (read/write)
  */
void reg_symphony_sd_encoder_set_cfig4(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG4, data);
}

mt_u32  reg_symphony_sd_encoder_get_cfig4(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG4);
}

void reg_symphony_sd_encoder_set_cfig4_lumcoef_n(mt_u16 data)
{
    reg_symphony_sd_encoder_cfig4_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG4;
    d.bitc.lumcoef_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG4, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_cfig4_lumcoef_n(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig4_t *)REG_SYMPHONY_SD_ENCODER_CFIG4).bitc.lumcoef_n;
}

void reg_symphony_sd_encoder_set_cfig4_burstcoef_n(mt_u16 data)
{
    reg_symphony_sd_encoder_cfig4_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG4;
    d.bitc.burstcoef_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG4, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_cfig4_burstcoef_n(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig4_t *)REG_SYMPHONY_SD_ENCODER_CFIG4).bitc.burstcoef_n;
}


/*!
  register SYMPHONY_SD_ENCODER_CFIG5 (read/write)
  */
void reg_symphony_sd_encoder_set_cfig5(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG5, data);
}

mt_u32  reg_symphony_sd_encoder_get_cfig5(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG5);
}

void reg_symphony_sd_encoder_set_cfig5_white_black_cfg(mt_u8 data)
{
    reg_symphony_sd_encoder_cfig5_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG5;
    d.bitc.white_black_cfg = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG5, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_cfig5_white_black_cfg(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig5_t *)REG_SYMPHONY_SD_ENCODER_CFIG5).bitc.white_black_cfg;
}

void reg_symphony_sd_encoder_set_cfig5_burst_rst_phase(mt_u16 data)
{
    reg_symphony_sd_encoder_cfig5_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG5;
    d.bitc.burst_rst_phase = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG5, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_cfig5_burst_rst_phase(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig5_t *)REG_SYMPHONY_SD_ENCODER_CFIG5).bitc.burst_rst_phase;
}


/*!
  register SYMPHONY_SD_ENCODER_CFIG6 (read/write)
  */
void reg_symphony_sd_encoder_set_cfig6(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG6, data);
}

mt_u32  reg_symphony_sd_encoder_get_cfig6(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6);
}

void reg_symphony_sd_encoder_set_cfig6_video_format(mt_u8 data)
{
    reg_symphony_sd_encoder_cfig6_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6;
    d.bitc.video_format = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG6, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_cfig6_video_format(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig6_t *)REG_SYMPHONY_SD_ENCODER_CFIG6).bitc.video_format;
}

void reg_symphony_sd_encoder_set_cfig6_burst_lmt_en(mt_u8 data)
{
    reg_symphony_sd_encoder_cfig6_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6;
    d.bitc.burst_lmt_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG6, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_cfig6_burst_lmt_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig6_t *)REG_SYMPHONY_SD_ENCODER_CFIG6).bitc.burst_lmt_en;
}

void reg_symphony_sd_encoder_set_cfig6_os_bypass(mt_u8 data)
{
    reg_symphony_sd_encoder_cfig6_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6;
    d.bitc.os_bypass = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG6, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_cfig6_os_bypass(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig6_t *)REG_SYMPHONY_SD_ENCODER_CFIG6).bitc.os_bypass;
}

void reg_symphony_sd_encoder_set_cfig6_dac_chal_sel(mt_u8 data)
{
    reg_symphony_sd_encoder_cfig6_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6;
    d.bitc.dac_chal_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG6, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_cfig6_dac_chal_sel(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig6_t *)REG_SYMPHONY_SD_ENCODER_CFIG6).bitc.dac_chal_sel;
}


/*!
  register SYMPHONY_SD_ENCODER_CFIG7 (read/write)
  */
void reg_symphony_sd_encoder_set_cfig7(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG7, data);
}

mt_u32  reg_symphony_sd_encoder_get_cfig7(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG7);
}

void reg_symphony_sd_encoder_set_cfig7_bypass_filter(mt_u8 data)
{
    reg_symphony_sd_encoder_cfig7_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG7;
    d.bitc.bypass_filter = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG7, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_cfig7_bypass_filter(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig7_t *)REG_SYMPHONY_SD_ENCODER_CFIG7).bitc.bypass_filter;
}

void reg_symphony_sd_encoder_set_cfig7_field_line(mt_u16 data)
{
    reg_symphony_sd_encoder_cfig7_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG7;
    d.bitc.field_line = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG7, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_cfig7_field_line(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig7_t *)REG_SYMPHONY_SD_ENCODER_CFIG7).bitc.field_line;
}

void reg_symphony_sd_encoder_set_cfig7_field_cnt(mt_u8 data)
{
    reg_symphony_sd_encoder_cfig7_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG7;
    d.bitc.field_cnt = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG7, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_cfig7_field_cnt(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig7_t *)REG_SYMPHONY_SD_ENCODER_CFIG7).bitc.field_cnt;
}


/*!
  register SYMPHONY_SD_ENCODER_SET (read/write)
  */
void reg_symphony_sd_encoder_set_set(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET, data);
}

mt_u32  reg_symphony_sd_encoder_get_set(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET);
}

void reg_symphony_sd_encoder_set_set_burst_rstmode(mt_u8 data)
{
    reg_symphony_sd_encoder_set_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET;
    d.bitc.burst_rstmode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_set_burst_rstmode(void)
{
    return (*(volatile reg_symphony_sd_encoder_set_t *)REG_SYMPHONY_SD_ENCODER_SET).bitc.burst_rstmode;
}

void reg_symphony_sd_encoder_set_set_burst_phase_change(mt_u8 data)
{
    reg_symphony_sd_encoder_set_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET;
    d.bitc.burst_phase_change = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_set_burst_phase_change(void)
{
    return (*(volatile reg_symphony_sd_encoder_set_t *)REG_SYMPHONY_SD_ENCODER_SET).bitc.burst_phase_change;
}

void reg_symphony_sd_encoder_set_set_ire_7p5_en(mt_u8 data)
{
    reg_symphony_sd_encoder_set_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET;
    d.bitc.ire_7p5_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_set_ire_7p5_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_set_t *)REG_SYMPHONY_SD_ENCODER_SET).bitc.ire_7p5_en;
}

void reg_symphony_sd_encoder_set_set_pal_seq_mode(mt_u8 data)
{
    reg_symphony_sd_encoder_set_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET;
    d.bitc.pal_seq_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_set_pal_seq_mode(void)
{
    return (*(volatile reg_symphony_sd_encoder_set_t *)REG_SYMPHONY_SD_ENCODER_SET).bitc.pal_seq_mode;
}

void reg_symphony_sd_encoder_set_set_dac_test_mode(mt_u8 data)
{
    reg_symphony_sd_encoder_set_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET;
    d.bitc.dac_test_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_set_dac_test_mode(void)
{
    return (*(volatile reg_symphony_sd_encoder_set_t *)REG_SYMPHONY_SD_ENCODER_SET).bitc.dac_test_mode;
}


/*!
  register SYMPHONY_SD_ENCODER_DAC0_PARA (read/write)
  */
void reg_symphony_sd_encoder_set_dac0_para(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA, data);
}

mt_u32  reg_symphony_sd_encoder_get_dac0_para(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA);
}

void reg_symphony_sd_encoder_set_dac0_para_df_coef_b(mt_u16 data)
{
    reg_symphony_sd_encoder_dac0_para_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA;
    d.bitc.df_coef_b = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_dac0_para_df_coef_b(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac0_para_t *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA).bitc.df_coef_b;
}

void reg_symphony_sd_encoder_set_dac0_para_df_coef_a(mt_u16 data)
{
    reg_symphony_sd_encoder_dac0_para_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA;
    d.bitc.df_coef_a = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_dac0_para_df_coef_a(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac0_para_t *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA).bitc.df_coef_a;
}

void reg_symphony_sd_encoder_set_dac0_para_df_sign_b(mt_u8 data)
{
    reg_symphony_sd_encoder_dac0_para_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA;
    d.bitc.df_sign_b = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_dac0_para_df_sign_b(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac0_para_t *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA).bitc.df_sign_b;
}

void reg_symphony_sd_encoder_set_dac0_para_df_en(mt_u8 data)
{
    reg_symphony_sd_encoder_dac0_para_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA;
    d.bitc.df_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_dac0_para_df_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac0_para_t *)REG_SYMPHONY_SD_ENCODER_DAC0_PARA).bitc.df_en;
}


/*!
  register SYMPHONY_SD_ENCODER_DRCOEF (read/write)
  */
void reg_symphony_sd_encoder_set_drcoef(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DRCOEF, data);
}

mt_u32  reg_symphony_sd_encoder_get_drcoef(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DRCOEF);
}

void reg_symphony_sd_encoder_set_drcoef_df_coef(mt_u32 data)
{
    reg_symphony_sd_encoder_drcoef_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DRCOEF;
    d.bitc.df_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DRCOEF, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_drcoef_df_coef(void)
{
    return (*(volatile reg_symphony_sd_encoder_drcoef_t *)REG_SYMPHONY_SD_ENCODER_DRCOEF).bitc.df_coef;
}


/*!
  register SYMPHONY_SD_ENCODER_DBCOEF (read/write)
  */
void reg_symphony_sd_encoder_set_dbcoef(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DBCOEF, data);
}

mt_u32  reg_symphony_sd_encoder_get_dbcoef(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DBCOEF);
}

void reg_symphony_sd_encoder_set_dbcoef_db_coef(mt_u32 data)
{
    reg_symphony_sd_encoder_dbcoef_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DBCOEF;
    d.bitc.db_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DBCOEF, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_dbcoef_db_coef(void)
{
    return (*(volatile reg_symphony_sd_encoder_dbcoef_t *)REG_SYMPHONY_SD_ENCODER_DBCOEF).bitc.db_coef;
}


/*!
  register SYMPHONY_SD_ENCODER_CFIG8 (read/write)
  */
void reg_symphony_sd_encoder_set_cfig8(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG8, data);
}

mt_u32  reg_symphony_sd_encoder_get_cfig8(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG8);
}

void reg_symphony_sd_encoder_set_cfig8_scm_camp_coef(mt_u16 data)
{
    reg_symphony_sd_encoder_cfig8_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG8;
    d.bitc.scm_camp_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_CFIG8, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_cfig8_scm_camp_coef(void)
{
    return (*(volatile reg_symphony_sd_encoder_cfig8_t *)REG_SYMPHONY_SD_ENCODER_CFIG8).bitc.scm_camp_coef;
}


/*!
  register SYMPHONY_SD_ENCODER_SCARRYDR (read/write)
  */
void reg_symphony_sd_encoder_set_scarrydr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SCARRYDR, data);
}

mt_u32  reg_symphony_sd_encoder_get_scarrydr(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SCARRYDR);
}

void reg_symphony_sd_encoder_set_scarrydr_mid_freqwdr(mt_u32 data)
{
    reg_symphony_sd_encoder_scarrydr_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SCARRYDR;
    d.bitc.mid_freqwdr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SCARRYDR, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_scarrydr_mid_freqwdr(void)
{
    return (*(volatile reg_symphony_sd_encoder_scarrydr_t *)REG_SYMPHONY_SD_ENCODER_SCARRYDR).bitc.mid_freqwdr;
}


/*!
  register SYMPHONY_SD_ENCODER_SCARRYDB (read/write)
  */
void reg_symphony_sd_encoder_set_scarrydb(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SCARRYDB, data);
}

mt_u32  reg_symphony_sd_encoder_get_scarrydb(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SCARRYDB);
}

void reg_symphony_sd_encoder_set_scarrydb_mid_freqwdb(mt_u32 data)
{
    reg_symphony_sd_encoder_scarrydb_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SCARRYDB;
    d.bitc.mid_freqwdb = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SCARRYDB, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_scarrydb_mid_freqwdb(void)
{
    return (*(volatile reg_symphony_sd_encoder_scarrydb_t *)REG_SYMPHONY_SD_ENCODER_SCARRYDB).bitc.mid_freqwdb;
}


/*!
  register SYMPHONY_SD_ENCODER_PINCREMENT (read/write)
  */
void reg_symphony_sd_encoder_set_pincrement(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_PINCREMENT, data);
}

mt_u32  reg_symphony_sd_encoder_get_pincrement(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_PINCREMENT);
}

void reg_symphony_sd_encoder_set_pincrement_phase_line_increment_p(mt_u32 data)
{
    reg_symphony_sd_encoder_pincrement_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_PINCREMENT;
    d.bitc.phase_line_increment_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_PINCREMENT, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_pincrement_phase_line_increment_p(void)
{
    return (*(volatile reg_symphony_sd_encoder_pincrement_t *)REG_SYMPHONY_SD_ENCODER_PINCREMENT).bitc.phase_line_increment_p;
}


/*!
  register SYMPHONY_SD_ENCODER_NINCREMENT (read/write)
  */
void reg_symphony_sd_encoder_set_nincrement(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_NINCREMENT, data);
}

mt_u32  reg_symphony_sd_encoder_get_nincrement(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_NINCREMENT);
}

void reg_symphony_sd_encoder_set_nincrement_phase_line_increment_n(mt_u32 data)
{
    reg_symphony_sd_encoder_nincrement_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_NINCREMENT;
    d.bitc.phase_line_increment_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_NINCREMENT, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_nincrement_phase_line_increment_n(void)
{
    return (*(volatile reg_symphony_sd_encoder_nincrement_t *)REG_SYMPHONY_SD_ENCODER_NINCREMENT).bitc.phase_line_increment_n;
}


/*!
  register SYMPHONY_SD_ENCODER_GCONTROL (read/write)
  */
void reg_symphony_sd_encoder_set_gcontrol(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_GCONTROL, data);
}

mt_u32  reg_symphony_sd_encoder_get_gcontrol(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_GCONTROL);
}

void reg_symphony_sd_encoder_set_gcontrol_gain_control_0(mt_u16 data)
{
    reg_symphony_sd_encoder_gcontrol_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_GCONTROL;
    d.bitc.gain_control_0 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_GCONTROL, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_gcontrol_gain_control_0(void)
{
    return (*(volatile reg_symphony_sd_encoder_gcontrol_t *)REG_SYMPHONY_SD_ENCODER_GCONTROL).bitc.gain_control_0;
}

void reg_symphony_sd_encoder_set_gcontrol_gain_control_123(mt_u16 data)
{
    reg_symphony_sd_encoder_gcontrol_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_GCONTROL;
    d.bitc.gain_control_123 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_GCONTROL, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_gcontrol_gain_control_123(void)
{
    return (*(volatile reg_symphony_sd_encoder_gcontrol_t *)REG_SYMPHONY_SD_ENCODER_GCONTROL).bitc.gain_control_123;
}

void reg_symphony_sd_encoder_set_gcontrol_gain_control_0_en(mt_u8 data)
{
    reg_symphony_sd_encoder_gcontrol_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_GCONTROL;
    d.bitc.gain_control_0_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_GCONTROL, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_gcontrol_gain_control_0_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_gcontrol_t *)REG_SYMPHONY_SD_ENCODER_GCONTROL).bitc.gain_control_0_en;
}

void reg_symphony_sd_encoder_set_gcontrol_gain_control_1_en(mt_u8 data)
{
    reg_symphony_sd_encoder_gcontrol_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_GCONTROL;
    d.bitc.gain_control_1_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_GCONTROL, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_gcontrol_gain_control_1_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_gcontrol_t *)REG_SYMPHONY_SD_ENCODER_GCONTROL).bitc.gain_control_1_en;
}

void reg_symphony_sd_encoder_set_gcontrol_gain_control_2_en(mt_u8 data)
{
    reg_symphony_sd_encoder_gcontrol_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_GCONTROL;
    d.bitc.gain_control_2_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_GCONTROL, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_gcontrol_gain_control_2_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_gcontrol_t *)REG_SYMPHONY_SD_ENCODER_GCONTROL).bitc.gain_control_2_en;
}

void reg_symphony_sd_encoder_set_gcontrol_gain_control_3_en(mt_u8 data)
{
    reg_symphony_sd_encoder_gcontrol_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_GCONTROL;
    d.bitc.gain_control_3_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_GCONTROL, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_gcontrol_gain_control_3_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_gcontrol_t *)REG_SYMPHONY_SD_ENCODER_GCONTROL).bitc.gain_control_3_en;
}


/*!
  register SYMPHONY_SD_ENCODER_DACNUM (read/write)
  */
void reg_symphony_sd_encoder_set_dacnum(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DACNUM, data);
}

mt_u32  reg_symphony_sd_encoder_get_dacnum(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DACNUM);
}

void reg_symphony_sd_encoder_set_dacnum_soft_set(mt_u8 data)
{
    reg_symphony_sd_encoder_dacnum_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DACNUM;
    d.bitc.soft_set = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DACNUM, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_dacnum_soft_set(void)
{
    return (*(volatile reg_symphony_sd_encoder_dacnum_t *)REG_SYMPHONY_SD_ENCODER_DACNUM).bitc.soft_set;
}

void reg_symphony_sd_encoder_set_dacnum_dac_switch_en(mt_u8 data)
{
    reg_symphony_sd_encoder_dacnum_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DACNUM;
    d.bitc.dac_switch_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DACNUM, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_dacnum_dac_switch_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_dacnum_t *)REG_SYMPHONY_SD_ENCODER_DACNUM).bitc.dac_switch_en;
}

void reg_symphony_sd_encoder_set_dacnum_ypbpr_same_set(mt_u8 data)
{
    reg_symphony_sd_encoder_dacnum_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DACNUM;
    d.bitc.ypbpr_same_set = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DACNUM, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_dacnum_ypbpr_same_set(void)
{
    return (*(volatile reg_symphony_sd_encoder_dacnum_t *)REG_SYMPHONY_SD_ENCODER_DACNUM).bitc.ypbpr_same_set;
}


/*!
  register SYMPHONY_SD_ENCODER_INSTM (read/write)
  */
void reg_symphony_sd_encoder_set_instm(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_INSTM, data);
}

mt_u32  reg_symphony_sd_encoder_get_instm(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_INSTM);
}

void reg_symphony_sd_encoder_set_instm_video_instm_p(mt_u16 data)
{
    reg_symphony_sd_encoder_instm_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_INSTM;
    d.bitc.video_instm_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_INSTM, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_instm_video_instm_p(void)
{
    return (*(volatile reg_symphony_sd_encoder_instm_t *)REG_SYMPHONY_SD_ENCODER_INSTM).bitc.video_instm_p;
}

void reg_symphony_sd_encoder_set_instm_video_instm_n(mt_u16 data)
{
    reg_symphony_sd_encoder_instm_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_INSTM;
    d.bitc.video_instm_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_INSTM, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_instm_video_instm_n(void)
{
    return (*(volatile reg_symphony_sd_encoder_instm_t *)REG_SYMPHONY_SD_ENCODER_INSTM).bitc.video_instm_n;
}

void reg_symphony_sd_encoder_set_instm_digital_time_en(mt_u8 data)
{
    reg_symphony_sd_encoder_instm_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_INSTM;
    d.bitc.digital_time_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_INSTM, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_instm_digital_time_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_instm_t *)REG_SYMPHONY_SD_ENCODER_INSTM).bitc.digital_time_en;
}


/*!
  register SYMPHONY_SD_ENCODER_COMPRESS (read/write)
  */
void reg_symphony_sd_encoder_set_compress(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COMPRESS, data);
}

mt_u32  reg_symphony_sd_encoder_get_compress(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COMPRESS);
}

void reg_symphony_sd_encoder_set_compress_cr_compress(mt_u8 data)
{
    reg_symphony_sd_encoder_compress_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COMPRESS;
    d.bitc.cr_compress = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COMPRESS, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_compress_cr_compress(void)
{
    return (*(volatile reg_symphony_sd_encoder_compress_t *)REG_SYMPHONY_SD_ENCODER_COMPRESS).bitc.cr_compress;
}

void reg_symphony_sd_encoder_set_compress_cb_compress(mt_u8 data)
{
    reg_symphony_sd_encoder_compress_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COMPRESS;
    d.bitc.cb_compress = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COMPRESS, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_compress_cb_compress(void)
{
    return (*(volatile reg_symphony_sd_encoder_compress_t *)REG_SYMPHONY_SD_ENCODER_COMPRESS).bitc.cb_compress;
}

void reg_symphony_sd_encoder_set_compress_y_compress(mt_u8 data)
{
    reg_symphony_sd_encoder_compress_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COMPRESS;
    d.bitc.y_compress = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COMPRESS, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_compress_y_compress(void)
{
    return (*(volatile reg_symphony_sd_encoder_compress_t *)REG_SYMPHONY_SD_ENCODER_COMPRESS).bitc.y_compress;
}


/*!
  register SYMPHONY_SD_ENCODER_SET1 (read/write)
  */
void reg_symphony_sd_encoder_set_set1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET1, data);
}

mt_u32  reg_symphony_sd_encoder_get_set1(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET1);
}

void reg_symphony_sd_encoder_set_set1_cvbs_chrom_delay(mt_u8 data)
{
    reg_symphony_sd_encoder_set1_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET1;
    d.bitc.cvbs_chrom_delay = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET1, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_set1_cvbs_chrom_delay(void)
{
    return (*(volatile reg_symphony_sd_encoder_set1_t *)REG_SYMPHONY_SD_ENCODER_SET1).bitc.cvbs_chrom_delay;
}

void reg_symphony_sd_encoder_set_set1_ypp_chrom_delay(mt_u8 data)
{
    reg_symphony_sd_encoder_set1_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET1;
    d.bitc.ypp_chrom_delay = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET1, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_set1_ypp_chrom_delay(void)
{
    return (*(volatile reg_symphony_sd_encoder_set1_t *)REG_SYMPHONY_SD_ENCODER_SET1).bitc.ypp_chrom_delay;
}


/*!
  register SYMPHONY_SD_ENCODER_SET2 (read/write)
  */
void reg_symphony_sd_encoder_set_set2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET2, data);
}

mt_u32  reg_symphony_sd_encoder_get_set2(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET2);
}

void reg_symphony_sd_encoder_set_set2_ypp_lum_delay(mt_u8 data)
{
    reg_symphony_sd_encoder_set2_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET2;
    d.bitc.ypp_lum_delay = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET2, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_set2_ypp_lum_delay(void)
{
    return (*(volatile reg_symphony_sd_encoder_set2_t *)REG_SYMPHONY_SD_ENCODER_SET2).bitc.ypp_lum_delay;
}

void reg_symphony_sd_encoder_set_set2_y_sync_n(mt_u8 data)
{
    reg_symphony_sd_encoder_set2_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET2;
    d.bitc.y_sync_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET2, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_set2_y_sync_n(void)
{
    return (*(volatile reg_symphony_sd_encoder_set2_t *)REG_SYMPHONY_SD_ENCODER_SET2).bitc.y_sync_n;
}


/*!
  register SYMPHONY_SD_ENCODER_SET3 (read/write)
  */
void reg_symphony_sd_encoder_set_set3(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET3, data);
}

mt_u32  reg_symphony_sd_encoder_get_set3(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET3);
}

void reg_symphony_sd_encoder_set_set3_video_end_num_p(mt_u16 data)
{
    reg_symphony_sd_encoder_set3_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET3;
    d.bitc.video_end_num_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET3, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_set3_video_end_num_p(void)
{
    return (*(volatile reg_symphony_sd_encoder_set3_t *)REG_SYMPHONY_SD_ENCODER_SET3).bitc.video_end_num_p;
}

void reg_symphony_sd_encoder_set_set3_video_end_num_n(mt_u16 data)
{
    reg_symphony_sd_encoder_set3_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SET3;
    d.bitc.video_end_num_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SET3, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_set3_video_end_num_n(void)
{
    return (*(volatile reg_symphony_sd_encoder_set3_t *)REG_SYMPHONY_SD_ENCODER_SET3).bitc.video_end_num_n;
}


/*!
  register SYMPHONY_SD_ENCODER_LUM_DLY_108M (read/write)
  */
void reg_symphony_sd_encoder_set_lum_dly_108m(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M, data);
}

mt_u32  reg_symphony_sd_encoder_get_lum_dly_108m(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M);
}

void reg_symphony_sd_encoder_set_lum_dly_108m_cvbs_lum_delay_108m(mt_u8 data)
{
    reg_symphony_sd_encoder_lum_dly_108m_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M;
    d.bitc.cvbs_lum_delay_108m = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_lum_dly_108m_cvbs_lum_delay_108m(void)
{
    return (*(volatile reg_symphony_sd_encoder_lum_dly_108m_t *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M).bitc.cvbs_lum_delay_108m;
}

void reg_symphony_sd_encoder_set_lum_dly_108m_ypp_lum_delay_108m(mt_u8 data)
{
    reg_symphony_sd_encoder_lum_dly_108m_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M;
    d.bitc.ypp_lum_delay_108m = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_lum_dly_108m_ypp_lum_delay_108m(void)
{
    return (*(volatile reg_symphony_sd_encoder_lum_dly_108m_t *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M).bitc.ypp_lum_delay_108m;
}

void reg_symphony_sd_encoder_set_lum_dly_108m_vid_mask_begin_cnt(mt_u8 data)
{
    reg_symphony_sd_encoder_lum_dly_108m_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M;
    d.bitc.vid_mask_begin_cnt = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_lum_dly_108m_vid_mask_begin_cnt(void)
{
    return (*(volatile reg_symphony_sd_encoder_lum_dly_108m_t *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M).bitc.vid_mask_begin_cnt;
}

void reg_symphony_sd_encoder_set_lum_dly_108m_vid_mask_end_cnt(mt_u8 data)
{
    reg_symphony_sd_encoder_lum_dly_108m_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M;
    d.bitc.vid_mask_end_cnt = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_lum_dly_108m_vid_mask_end_cnt(void)
{
    return (*(volatile reg_symphony_sd_encoder_lum_dly_108m_t *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M).bitc.vid_mask_end_cnt;
}

void reg_symphony_sd_encoder_set_lum_dly_108m_vid_mask_en(mt_u8 data)
{
    reg_symphony_sd_encoder_lum_dly_108m_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M;
    d.bitc.vid_mask_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_lum_dly_108m_vid_mask_en(void)
{
    return (*(volatile reg_symphony_sd_encoder_lum_dly_108m_t *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M).bitc.vid_mask_en;
}

void reg_symphony_sd_encoder_set_lum_dly_108m_first_line_later(mt_u8 data)
{
    reg_symphony_sd_encoder_lum_dly_108m_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M;
    d.bitc.first_line_later = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_lum_dly_108m_first_line_later(void)
{
    return (*(volatile reg_symphony_sd_encoder_lum_dly_108m_t *)REG_SYMPHONY_SD_ENCODER_LUM_DLY_108M).bitc.first_line_later;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF11 (read/write)
  */
void reg_symphony_sd_encoder_set_coef11(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF11, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef11(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF11);
}

void reg_symphony_sd_encoder_set_coef11_coef11(mt_u16 data)
{
    reg_symphony_sd_encoder_coef11_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF11;
    d.bitc.coef11 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF11, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef11_coef11(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef11_t *)REG_SYMPHONY_SD_ENCODER_COEF11).bitc.coef11;
}

void reg_symphony_sd_encoder_set_coef11_coef11_y(mt_u16 data)
{
    reg_symphony_sd_encoder_coef11_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF11;
    d.bitc.coef11_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF11, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef11_coef11_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef11_t *)REG_SYMPHONY_SD_ENCODER_COEF11).bitc.coef11_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF10 (read/write)
  */
void reg_symphony_sd_encoder_set_coef10(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF10, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef10(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF10);
}

void reg_symphony_sd_encoder_set_coef10_coef10(mt_u16 data)
{
    reg_symphony_sd_encoder_coef10_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF10;
    d.bitc.coef10 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF10, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef10_coef10(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef10_t *)REG_SYMPHONY_SD_ENCODER_COEF10).bitc.coef10;
}

void reg_symphony_sd_encoder_set_coef10_coef10_y(mt_u16 data)
{
    reg_symphony_sd_encoder_coef10_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF10;
    d.bitc.coef10_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF10, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef10_coef10_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef10_t *)REG_SYMPHONY_SD_ENCODER_COEF10).bitc.coef10_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF9 (read/write)
  */
void reg_symphony_sd_encoder_set_coef9(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF9, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef9(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF9);
}

void reg_symphony_sd_encoder_set_coef9_coef9(mt_u16 data)
{
    reg_symphony_sd_encoder_coef9_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF9;
    d.bitc.coef9 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF9, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef9_coef9(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef9_t *)REG_SYMPHONY_SD_ENCODER_COEF9).bitc.coef9;
}

void reg_symphony_sd_encoder_set_coef9_coef9_y(mt_u16 data)
{
    reg_symphony_sd_encoder_coef9_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF9;
    d.bitc.coef9_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF9, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef9_coef9_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef9_t *)REG_SYMPHONY_SD_ENCODER_COEF9).bitc.coef9_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF8 (read/write)
  */
void reg_symphony_sd_encoder_set_coef8(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF8, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef8(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF8);
}

void reg_symphony_sd_encoder_set_coef8_coef8(mt_u16 data)
{
    reg_symphony_sd_encoder_coef8_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF8;
    d.bitc.coef8 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF8, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef8_coef8(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef8_t *)REG_SYMPHONY_SD_ENCODER_COEF8).bitc.coef8;
}

void reg_symphony_sd_encoder_set_coef8_coef8_y(mt_u16 data)
{
    reg_symphony_sd_encoder_coef8_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF8;
    d.bitc.coef8_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF8, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef8_coef8_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef8_t *)REG_SYMPHONY_SD_ENCODER_COEF8).bitc.coef8_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF7 (read/write)
  */
void reg_symphony_sd_encoder_set_coef7(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF7, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef7(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF7);
}

void reg_symphony_sd_encoder_set_coef7_coef7(mt_u16 data)
{
    reg_symphony_sd_encoder_coef7_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF7;
    d.bitc.coef7 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF7, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef7_coef7(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef7_t *)REG_SYMPHONY_SD_ENCODER_COEF7).bitc.coef7;
}

void reg_symphony_sd_encoder_set_coef7_coef7_y(mt_u16 data)
{
    reg_symphony_sd_encoder_coef7_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF7;
    d.bitc.coef7_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF7, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef7_coef7_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef7_t *)REG_SYMPHONY_SD_ENCODER_COEF7).bitc.coef7_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF6 (read/write)
  */
void reg_symphony_sd_encoder_set_coef6(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF6, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef6(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF6);
}

void reg_symphony_sd_encoder_set_coef6_coef6(mt_u16 data)
{
    reg_symphony_sd_encoder_coef6_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF6;
    d.bitc.coef6 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF6, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef6_coef6(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef6_t *)REG_SYMPHONY_SD_ENCODER_COEF6).bitc.coef6;
}

void reg_symphony_sd_encoder_set_coef6_coef6_y(mt_u16 data)
{
    reg_symphony_sd_encoder_coef6_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF6;
    d.bitc.coef6_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF6, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef6_coef6_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef6_t *)REG_SYMPHONY_SD_ENCODER_COEF6).bitc.coef6_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF5 (read/write)
  */
void reg_symphony_sd_encoder_set_coef5(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF5, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef5(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF5);
}

void reg_symphony_sd_encoder_set_coef5_coef5(mt_u16 data)
{
    reg_symphony_sd_encoder_coef5_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF5;
    d.bitc.coef5 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF5, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef5_coef5(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef5_t *)REG_SYMPHONY_SD_ENCODER_COEF5).bitc.coef5;
}

void reg_symphony_sd_encoder_set_coef5_coef5_y(mt_u16 data)
{
    reg_symphony_sd_encoder_coef5_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF5;
    d.bitc.coef5_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF5, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef5_coef5_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef5_t *)REG_SYMPHONY_SD_ENCODER_COEF5).bitc.coef5_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF4 (read/write)
  */
void reg_symphony_sd_encoder_set_coef4(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF4, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef4(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF4);
}

void reg_symphony_sd_encoder_set_coef4_coef4(mt_u16 data)
{
    reg_symphony_sd_encoder_coef4_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF4;
    d.bitc.coef4 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF4, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef4_coef4(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef4_t *)REG_SYMPHONY_SD_ENCODER_COEF4).bitc.coef4;
}

void reg_symphony_sd_encoder_set_coef4_coef4_y(mt_u16 data)
{
    reg_symphony_sd_encoder_coef4_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF4;
    d.bitc.coef4_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF4, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef4_coef4_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef4_t *)REG_SYMPHONY_SD_ENCODER_COEF4).bitc.coef4_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF3 (read/write)
  */
void reg_symphony_sd_encoder_set_coef3(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF3, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef3(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF3);
}

void reg_symphony_sd_encoder_set_coef3_coef3(mt_u16 data)
{
    reg_symphony_sd_encoder_coef3_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF3;
    d.bitc.coef3 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF3, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef3_coef3(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef3_t *)REG_SYMPHONY_SD_ENCODER_COEF3).bitc.coef3;
}

void reg_symphony_sd_encoder_set_coef3_coef3_y(mt_u16 data)
{
    reg_symphony_sd_encoder_coef3_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF3;
    d.bitc.coef3_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF3, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef3_coef3_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef3_t *)REG_SYMPHONY_SD_ENCODER_COEF3).bitc.coef3_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF2 (read/write)
  */
void reg_symphony_sd_encoder_set_coef2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF2, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef2(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF2);
}

void reg_symphony_sd_encoder_set_coef2_coef2(mt_u16 data)
{
    reg_symphony_sd_encoder_coef2_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF2;
    d.bitc.coef2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF2, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef2_coef2(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef2_t *)REG_SYMPHONY_SD_ENCODER_COEF2).bitc.coef2;
}

void reg_symphony_sd_encoder_set_coef2_coef2_y(mt_u16 data)
{
    reg_symphony_sd_encoder_coef2_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF2;
    d.bitc.coef2_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF2, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef2_coef2_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef2_t *)REG_SYMPHONY_SD_ENCODER_COEF2).bitc.coef2_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF1 (read/write)
  */
void reg_symphony_sd_encoder_set_coef1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF1, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef1(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF1);
}

void reg_symphony_sd_encoder_set_coef1_coef1(mt_u16 data)
{
    reg_symphony_sd_encoder_coef1_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF1;
    d.bitc.coef1 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF1, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef1_coef1(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef1_t *)REG_SYMPHONY_SD_ENCODER_COEF1).bitc.coef1;
}

void reg_symphony_sd_encoder_set_coef1_coef1_y(mt_u16 data)
{
    reg_symphony_sd_encoder_coef1_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF1;
    d.bitc.coef1_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF1, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef1_coef1_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef1_t *)REG_SYMPHONY_SD_ENCODER_COEF1).bitc.coef1_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF0 (read/write)
  */
void reg_symphony_sd_encoder_set_coef0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF0, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef0(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF0);
}

void reg_symphony_sd_encoder_set_coef0_coef0(mt_u16 data)
{
    reg_symphony_sd_encoder_coef0_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF0;
    d.bitc.coef0 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF0, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef0_coef0(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef0_t *)REG_SYMPHONY_SD_ENCODER_COEF0).bitc.coef0;
}

void reg_symphony_sd_encoder_set_coef0_coef0_y(mt_u16 data)
{
    reg_symphony_sd_encoder_coef0_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF0;
    d.bitc.coef0_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF0, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_coef0_coef0_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef0_t *)REG_SYMPHONY_SD_ENCODER_COEF0).bitc.coef0_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF_C_LUT3_U (read/write)
  */
void reg_symphony_sd_encoder_set_coef_c_lut3_u(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT3_U, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut3_u(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT3_U);
}

void reg_symphony_sd_encoder_set_coef_c_lut3_u_coef_c_lut3_u(mt_u32 data)
{
    reg_symphony_sd_encoder_coef_c_lut3_u_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT3_U;
    d.bitc.coef_c_lut3_u = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT3_U, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut3_u_coef_c_lut3_u(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef_c_lut3_u_t *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT3_U).bitc.coef_c_lut3_u;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF_C_LUT2_U (read/write)
  */
void reg_symphony_sd_encoder_set_coef_c_lut2_u(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT2_U, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut2_u(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT2_U);
}

void reg_symphony_sd_encoder_set_coef_c_lut2_u_coef_c_lut2_u(mt_u32 data)
{
    reg_symphony_sd_encoder_coef_c_lut2_u_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT2_U;
    d.bitc.coef_c_lut2_u = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT2_U, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut2_u_coef_c_lut2_u(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef_c_lut2_u_t *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT2_U).bitc.coef_c_lut2_u;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF_C_LUT1_U (read/write)
  */
void reg_symphony_sd_encoder_set_coef_c_lut1_u(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT1_U, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut1_u(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT1_U);
}

void reg_symphony_sd_encoder_set_coef_c_lut1_u_coef_c_lut1_u(mt_u32 data)
{
    reg_symphony_sd_encoder_coef_c_lut1_u_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT1_U;
    d.bitc.coef_c_lut1_u = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT1_U, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut1_u_coef_c_lut1_u(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef_c_lut1_u_t *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT1_U).bitc.coef_c_lut1_u;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF_C_LUT0_U (read/write)
  */
void reg_symphony_sd_encoder_set_coef_c_lut0_u(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT0_U, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut0_u(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT0_U);
}

void reg_symphony_sd_encoder_set_coef_c_lut0_u_coef_c_lut0_u(mt_u32 data)
{
    reg_symphony_sd_encoder_coef_c_lut0_u_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT0_U;
    d.bitc.coef_c_lut0_u = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT0_U, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut0_u_coef_c_lut0_u(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef_c_lut0_u_t *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT0_U).bitc.coef_c_lut0_u;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF_C_LUT3_V (read/write)
  */
void reg_symphony_sd_encoder_set_coef_c_lut3_v(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT3_V, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut3_v(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT3_V);
}

void reg_symphony_sd_encoder_set_coef_c_lut3_v_coef_c_lut3_v(mt_u32 data)
{
    reg_symphony_sd_encoder_coef_c_lut3_v_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT3_V;
    d.bitc.coef_c_lut3_v = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT3_V, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut3_v_coef_c_lut3_v(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef_c_lut3_v_t *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT3_V).bitc.coef_c_lut3_v;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF_C_LUT2_V (read/write)
  */
void reg_symphony_sd_encoder_set_coef_c_lut2_v(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT2_V, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut2_v(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT2_V);
}

void reg_symphony_sd_encoder_set_coef_c_lut2_v_coef_c_lut2_v(mt_u32 data)
{
    reg_symphony_sd_encoder_coef_c_lut2_v_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT2_V;
    d.bitc.coef_c_lut2_v = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT2_V, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut2_v_coef_c_lut2_v(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef_c_lut2_v_t *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT2_V).bitc.coef_c_lut2_v;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF_C_LUT1_V (read/write)
  */
void reg_symphony_sd_encoder_set_coef_c_lut1_v(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT1_V, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut1_v(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT1_V);
}

void reg_symphony_sd_encoder_set_coef_c_lut1_v_coef_c_lut1_v(mt_u32 data)
{
    reg_symphony_sd_encoder_coef_c_lut1_v_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT1_V;
    d.bitc.coef_c_lut1_v = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT1_V, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut1_v_coef_c_lut1_v(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef_c_lut1_v_t *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT1_V).bitc.coef_c_lut1_v;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF_C_LUT0_V (read/write)
  */
void reg_symphony_sd_encoder_set_coef_c_lut0_v(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT0_V, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut0_v(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT0_V);
}

void reg_symphony_sd_encoder_set_coef_c_lut0_v_coef_c_lut0_v(mt_u32 data)
{
    reg_symphony_sd_encoder_coef_c_lut0_v_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT0_V;
    d.bitc.coef_c_lut0_v = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT0_V, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_coef_c_lut0_v_coef_c_lut0_v(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef_c_lut0_v_t *)REG_SYMPHONY_SD_ENCODER_COEF_C_LUT0_V).bitc.coef_c_lut0_v;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF_LUT3_Y (read/write)
  */
void reg_symphony_sd_encoder_set_coef_lut3_y(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_LUT3_Y, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef_lut3_y(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_LUT3_Y);
}

void reg_symphony_sd_encoder_set_coef_lut3_y_coef_lut3_y(mt_u32 data)
{
    reg_symphony_sd_encoder_coef_lut3_y_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_LUT3_Y;
    d.bitc.coef_lut3_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_LUT3_Y, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_coef_lut3_y_coef_lut3_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef_lut3_y_t *)REG_SYMPHONY_SD_ENCODER_COEF_LUT3_Y).bitc.coef_lut3_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF_LUT2_Y (read/write)
  */
void reg_symphony_sd_encoder_set_coef_lut2_y(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_LUT2_Y, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef_lut2_y(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_LUT2_Y);
}

void reg_symphony_sd_encoder_set_coef_lut2_y_coef_lut2_y(mt_u32 data)
{
    reg_symphony_sd_encoder_coef_lut2_y_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_LUT2_Y;
    d.bitc.coef_lut2_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_LUT2_Y, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_coef_lut2_y_coef_lut2_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef_lut2_y_t *)REG_SYMPHONY_SD_ENCODER_COEF_LUT2_Y).bitc.coef_lut2_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF_LUT1_Y (read/write)
  */
void reg_symphony_sd_encoder_set_coef_lut1_y(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_LUT1_Y, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef_lut1_y(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_LUT1_Y);
}

void reg_symphony_sd_encoder_set_coef_lut1_y_coef_lut1_y(mt_u32 data)
{
    reg_symphony_sd_encoder_coef_lut1_y_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_LUT1_Y;
    d.bitc.coef_lut1_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_LUT1_Y, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_coef_lut1_y_coef_lut1_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef_lut1_y_t *)REG_SYMPHONY_SD_ENCODER_COEF_LUT1_Y).bitc.coef_lut1_y;
}


/*!
  register SYMPHONY_SD_ENCODER_COEF_LUT0_Y (read/write)
  */
void reg_symphony_sd_encoder_set_coef_lut0_y(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_LUT0_Y, data);
}

mt_u32  reg_symphony_sd_encoder_get_coef_lut0_y(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_LUT0_Y);
}

void reg_symphony_sd_encoder_set_coef_lut0_y_coef_lut0_y(mt_u32 data)
{
    reg_symphony_sd_encoder_coef_lut0_y_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_COEF_LUT0_Y;
    d.bitc.coef_lut0_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_COEF_LUT0_Y, d.all);
}

mt_u32  reg_symphony_sd_encoder_get_coef_lut0_y_coef_lut0_y(void)
{
    return (*(volatile reg_symphony_sd_encoder_coef_lut0_y_t *)REG_SYMPHONY_SD_ENCODER_COEF_LUT0_Y).bitc.coef_lut0_y;
}


/*!
  register SYMPHONY_SD_ENCODER_SIGN_CTL (read/write)
  */
void reg_symphony_sd_encoder_set_sign_ctl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SIGN_CTL, data);
}

mt_u32  reg_symphony_sd_encoder_get_sign_ctl(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SIGN_CTL);
}

void reg_symphony_sd_encoder_set_sign_ctl_sign_ctl(mt_u16 data)
{
    reg_symphony_sd_encoder_sign_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SIGN_CTL;
    d.bitc.sign_ctl = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_SIGN_CTL, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_sign_ctl_sign_ctl(void)
{
    return (*(volatile reg_symphony_sd_encoder_sign_ctl_t *)REG_SYMPHONY_SD_ENCODER_SIGN_CTL).bitc.sign_ctl;
}


/*!
  register SYMPHONY_SD_ENCODER_DAC_OFFSET (read/write)
  */
void reg_symphony_sd_encoder_set_dac_offset(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET, data);
}

mt_u32  reg_symphony_sd_encoder_get_dac_offset(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET);
}

void reg_symphony_sd_encoder_set_dac_offset_dac0_offset(mt_u8 data)
{
    reg_symphony_sd_encoder_dac_offset_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET;
    d.bitc.dac0_offset = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_dac_offset_dac0_offset(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac_offset_t *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET).bitc.dac0_offset;
}

void reg_symphony_sd_encoder_set_dac_offset_field1_other_vbi_first(mt_u8 data)
{
    reg_symphony_sd_encoder_dac_offset_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET;
    d.bitc.field1_other_vbi_first = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_dac_offset_field1_other_vbi_first(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac_offset_t *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET).bitc.field1_other_vbi_first;
}

void reg_symphony_sd_encoder_set_dac_offset_field2_other_vbi_first(mt_u8 data)
{
    reg_symphony_sd_encoder_dac_offset_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET;
    d.bitc.field2_other_vbi_first = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_dac_offset_field2_other_vbi_first(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac_offset_t *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET).bitc.field2_other_vbi_first;
}

void reg_symphony_sd_encoder_set_dac_offset_vbi_priority_mode(mt_u8 data)
{
    reg_symphony_sd_encoder_dac_offset_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET;
    d.bitc.vbi_priority_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_dac_offset_vbi_priority_mode(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac_offset_t *)REG_SYMPHONY_SD_ENCODER_DAC_OFFSET).bitc.vbi_priority_mode;
}


/*!
  register SYMPHONY_SD_ENCODER_DAC123_PARA (read/write)
  */
void reg_symphony_sd_encoder_set_dac123_para(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA, data);
}

mt_u32  reg_symphony_sd_encoder_get_dac123_para(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA);
}

void reg_symphony_sd_encoder_set_dac123_para_df_coef_b(mt_u16 data)
{
    reg_symphony_sd_encoder_dac123_para_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA;
    d.bitc.df_coef_b = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_dac123_para_df_coef_b(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac123_para_t *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA).bitc.df_coef_b;
}

void reg_symphony_sd_encoder_set_dac123_para_df_coef_a(mt_u16 data)
{
    reg_symphony_sd_encoder_dac123_para_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA;
    d.bitc.df_coef_a = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_dac123_para_df_coef_a(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac123_para_t *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA).bitc.df_coef_a;
}

void reg_symphony_sd_encoder_set_dac123_para_df_sign_b(mt_u8 data)
{
    reg_symphony_sd_encoder_dac123_para_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA;
    d.bitc.df_sign_b = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_dac123_para_df_sign_b(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac123_para_t *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA).bitc.df_sign_b;
}

void reg_symphony_sd_encoder_set_dac123_para_df_en_1(mt_u8 data)
{
    reg_symphony_sd_encoder_dac123_para_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA;
    d.bitc.df_en_1 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_dac123_para_df_en_1(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac123_para_t *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA).bitc.df_en_1;
}

void reg_symphony_sd_encoder_set_dac123_para_df_en_2(mt_u8 data)
{
    reg_symphony_sd_encoder_dac123_para_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA;
    d.bitc.df_en_2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_dac123_para_df_en_2(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac123_para_t *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA).bitc.df_en_2;
}

void reg_symphony_sd_encoder_set_dac123_para_df_en_3(mt_u8 data)
{
    reg_symphony_sd_encoder_dac123_para_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA;
    d.bitc.df_en_3 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_dac123_para_df_en_3(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac123_para_t *)REG_SYMPHONY_SD_ENCODER_DAC123_PARA).bitc.df_en_3;
}


/*!
  register SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF1_0 (read/write)
  */
void reg_symphony_sd_encoder_set_dac0_anti_coef1_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF1_0, data);
}

mt_u32  reg_symphony_sd_encoder_get_dac0_anti_coef1_0(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF1_0);
}

void reg_symphony_sd_encoder_set_dac0_anti_coef1_0_dac0_anti_coef0(mt_u16 data)
{
    reg_symphony_sd_encoder_dac0_anti_coef1_0_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF1_0;
    d.bitc.dac0_anti_coef0 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF1_0, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_dac0_anti_coef1_0_dac0_anti_coef0(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac0_anti_coef1_0_t *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF1_0).bitc.dac0_anti_coef0;
}

void reg_symphony_sd_encoder_set_dac0_anti_coef1_0_dac0_anti_coef1(mt_u16 data)
{
    reg_symphony_sd_encoder_dac0_anti_coef1_0_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF1_0;
    d.bitc.dac0_anti_coef1 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF1_0, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_dac0_anti_coef1_0_dac0_anti_coef1(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac0_anti_coef1_0_t *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF1_0).bitc.dac0_anti_coef1;
}


/*!
  register SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF3_2 (read/write)
  */
void reg_symphony_sd_encoder_set_dac0_anti_coef3_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF3_2, data);
}

mt_u32  reg_symphony_sd_encoder_get_dac0_anti_coef3_2(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF3_2);
}

void reg_symphony_sd_encoder_set_dac0_anti_coef3_2_dac0_anti_coef2(mt_u16 data)
{
    reg_symphony_sd_encoder_dac0_anti_coef3_2_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF3_2;
    d.bitc.dac0_anti_coef2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF3_2, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_dac0_anti_coef3_2_dac0_anti_coef2(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac0_anti_coef3_2_t *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF3_2).bitc.dac0_anti_coef2;
}

void reg_symphony_sd_encoder_set_dac0_anti_coef3_2_dac0_anti_coef3(mt_u16 data)
{
    reg_symphony_sd_encoder_dac0_anti_coef3_2_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF3_2;
    d.bitc.dac0_anti_coef3 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF3_2, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_dac0_anti_coef3_2_dac0_anti_coef3(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac0_anti_coef3_2_t *)REG_SYMPHONY_SD_ENCODER_DAC0_ANTI_COEF3_2).bitc.dac0_anti_coef3;
}


/*!
  register SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF1_0 (read/write)
  */
void reg_symphony_sd_encoder_set_dac123_anti_coef1_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF1_0, data);
}

mt_u32  reg_symphony_sd_encoder_get_dac123_anti_coef1_0(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF1_0);
}

void reg_symphony_sd_encoder_set_dac123_anti_coef1_0_dac123_anti_coef0(mt_u16 data)
{
    reg_symphony_sd_encoder_dac123_anti_coef1_0_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF1_0;
    d.bitc.dac123_anti_coef0 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF1_0, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_dac123_anti_coef1_0_dac123_anti_coef0(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac123_anti_coef1_0_t *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF1_0).bitc.dac123_anti_coef0;
}

void reg_symphony_sd_encoder_set_dac123_anti_coef1_0_dac123_anti_coef1(mt_u16 data)
{
    reg_symphony_sd_encoder_dac123_anti_coef1_0_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF1_0;
    d.bitc.dac123_anti_coef1 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF1_0, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_dac123_anti_coef1_0_dac123_anti_coef1(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac123_anti_coef1_0_t *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF1_0).bitc.dac123_anti_coef1;
}


/*!
  register SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF3_2 (read/write)
  */
void reg_symphony_sd_encoder_set_dac123_anti_coef3_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF3_2, data);
}

mt_u32  reg_symphony_sd_encoder_get_dac123_anti_coef3_2(void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF3_2);
}

void reg_symphony_sd_encoder_set_dac123_anti_coef3_2_dac123_anti_coef2(mt_u16 data)
{
    reg_symphony_sd_encoder_dac123_anti_coef3_2_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF3_2;
    d.bitc.dac123_anti_coef2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF3_2, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_dac123_anti_coef3_2_dac123_anti_coef2(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac123_anti_coef3_2_t *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF3_2).bitc.dac123_anti_coef2;
}

void reg_symphony_sd_encoder_set_dac123_anti_coef3_2_dac123_anti_coef3(mt_u16 data)
{
    reg_symphony_sd_encoder_dac123_anti_coef3_2_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF3_2;
    d.bitc.dac123_anti_coef3 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF3_2, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_dac123_anti_coef3_2_dac123_anti_coef3(void)
{
    return (*(volatile reg_symphony_sd_encoder_dac123_anti_coef3_2_t *)REG_SYMPHONY_SD_ENCODER_DAC123_ANTI_COEF3_2).bitc.dac123_anti_coef3;
}


/*!
  init function
  */
void reg_symphony_sd_encoder_init(void)
{
    reg_symphony_sd_encoder_set_mode((mt_u32)0x10000000);
    reg_symphony_sd_encoder_set_curve((mt_u32)0x013b0000);
    reg_symphony_sd_encoder_set_delay((mt_u32)0x0431212a);
    reg_symphony_sd_encoder_set_blank_p((mt_u32)0x01180118);
    reg_symphony_sd_encoder_set_blank_n((mt_u32)0x0133010d);
    reg_symphony_sd_encoder_set_cblank((mt_u32)0x02000108);
    reg_symphony_sd_encoder_set_vdac_pcarry((mt_u32)0x0a8262b2);
    reg_symphony_sd_encoder_set_vdac_ncarry((mt_u32)0x087c1f07);
    reg_symphony_sd_encoder_set_cfig1((mt_u32)0x015d00f5);
    reg_symphony_sd_encoder_set_cfig2((mt_u32)0x00780118);
    reg_symphony_sd_encoder_set_cfig3((mt_u32)0x014600e8);
    reg_symphony_sd_encoder_set_cfig4((mt_u32)0x00730114);
    reg_symphony_sd_encoder_set_cfig5((mt_u32)0x012e0000);
    reg_symphony_sd_encoder_set_cfig6((mt_u32)0x00c60010);
    reg_symphony_sd_encoder_set_cfig7((mt_u32)0x00000000);
    reg_symphony_sd_encoder_set_set((mt_u32)0x00000011);
    reg_symphony_sd_encoder_set_dac0_para((mt_u32)0x11a40320);
    reg_symphony_sd_encoder_set_drcoef((mt_u32)0x0000e263);
    reg_symphony_sd_encoder_set_dbcoef((mt_u32)0x0000b998);
    reg_symphony_sd_encoder_set_cfig8((mt_u32)0x00000041);
    reg_symphony_sd_encoder_set_scarrydr((mt_u32)0x0a71c71c);
    reg_symphony_sd_encoder_set_scarrydb((mt_u32)0x0a12f685);
    reg_symphony_sd_encoder_set_pincrement((mt_u32)0xc068db8c);
    reg_symphony_sd_encoder_set_nincrement((mt_u32)0x80000000);
    reg_symphony_sd_encoder_set_gcontrol((mt_u32)0x062b062b);
    reg_symphony_sd_encoder_set_dacnum((mt_u32)0x00010000);
    reg_symphony_sd_encoder_set_instm((mt_u32)0x00f0011b);
    reg_symphony_sd_encoder_set_compress((mt_u32)0x008d8a88);
    reg_symphony_sd_encoder_set_set1((mt_u32)0x00000016);
    reg_symphony_sd_encoder_set_set2((mt_u32)0x00002001);
    reg_symphony_sd_encoder_set_set3((mt_u32)0x0691069a);
    reg_symphony_sd_encoder_set_lum_dly_108m((mt_u32)0x01260a00);
    reg_symphony_sd_encoder_set_coef11((mt_u32)0x00010206);
    reg_symphony_sd_encoder_set_coef10((mt_u32)0x02040205);
    reg_symphony_sd_encoder_set_coef9((mt_u32)0x02010004);
    reg_symphony_sd_encoder_set_coef8((mt_u32)0x00080012);
    reg_symphony_sd_encoder_set_coef7((mt_u32)0x02030013);
    reg_symphony_sd_encoder_set_coef6((mt_u32)0x020f0207);
    reg_symphony_sd_encoder_set_coef5((mt_u32)0x000e022d);
    reg_symphony_sd_encoder_set_coef4((mt_u32)0x00170235);
    reg_symphony_sd_encoder_set_coef3((mt_u32)0x0229000a);
    reg_symphony_sd_encoder_set_coef2((mt_u32)0x021d008c);
    reg_symphony_sd_encoder_set_coef1((mt_u32)0x009f0111);
    reg_symphony_sd_encoder_set_coef0((mt_u32)0x01200148);
    reg_symphony_sd_encoder_set_coef_c_lut3_u((mt_u32)0x00000000);
    reg_symphony_sd_encoder_set_coef_c_lut2_u((mt_u32)0x55555555);
    reg_symphony_sd_encoder_set_coef_c_lut1_u((mt_u32)0x55555555);
    reg_symphony_sd_encoder_set_coef_c_lut0_u((mt_u32)0x00000000);
    reg_symphony_sd_encoder_set_coef_c_lut3_v((mt_u32)0x00000000);
    reg_symphony_sd_encoder_set_coef_c_lut2_v((mt_u32)0x55555555);
    reg_symphony_sd_encoder_set_coef_c_lut1_v((mt_u32)0x55555555);
    reg_symphony_sd_encoder_set_coef_c_lut0_v((mt_u32)0x00000000);
    reg_symphony_sd_encoder_set_coef_lut3_y((mt_u32)0x00000000);
    reg_symphony_sd_encoder_set_coef_lut2_y((mt_u32)0x00000000);
    reg_symphony_sd_encoder_set_coef_lut1_y((mt_u32)0x00000000);
    reg_symphony_sd_encoder_set_coef_lut0_y((mt_u32)0x00000000);
    reg_symphony_sd_encoder_set_sign_ctl((mt_u32)0x000000cc);
    reg_symphony_sd_encoder_set_dac_offset((mt_u32)0x80808080);
    reg_symphony_sd_encoder_set_dac123_para((mt_u32)0x71a40320);
    reg_symphony_sd_encoder_set_dac0_anti_coef1_0((mt_u32)0x01f000c0);
    reg_symphony_sd_encoder_set_dac0_anti_coef3_2((mt_u32)0x01ff0004);
    reg_symphony_sd_encoder_set_dac123_anti_coef1_0((mt_u32)0x01f000c0);
    reg_symphony_sd_encoder_set_dac123_anti_coef3_2((mt_u32)0x01ff0004);
    /* read read-clear registers in order to set mirror variables */
}


/*!
  end of file
  */

