/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "pq_hal.h"
#include "mt_type.h"

static inline void hal_put_u32(volatile unsigned long *p_addr, unsigned long data)
{
    /*!
    Write 32 bits register
    */
    *(p_addr) = data;
}

static inline mt_u32 hal_get_u32(volatile mt_u32 *p_addr)
{
    return *(volatile mt_u32 *)(p_addr);
}

/*!
  register ARIA_DISP_hd_video_post_config (read/write)
  */
void reg_aria2_disp_set_hd_video_post_config(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG), data);
}

mt_u32  reg_aria2_disp_get_hd_video_post_config(void)
{
    return (*(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG));
}

void reg_aria2_disp_set_hd_video_post_config_hd_leverage(mt_u8 data)
{
    reg_aria2_disp_hd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG);
    d.bitc.hd_leverage = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG), d.all);
}

mt_u8   reg_aria2_disp_get_hd_video_post_config_hd_leverage(void)
{
    return (*(volatile reg_aria2_disp_hd_video_post_config_t *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG)).bitc.hd_leverage;
}

void reg_aria2_disp_set_hd_video_post_config_hd_hp_enha(mt_u8 data)
{
    reg_aria2_disp_hd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG);
    d.bitc.hd_hp_enha = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG), d.all);
}

mt_u8   reg_aria2_disp_get_hd_video_post_config_hd_hp_enha(void)
{
    return (*(volatile reg_aria2_disp_hd_video_post_config_t *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG)).bitc.hd_hp_enha;
}

void reg_aria2_disp_set_hd_video_post_config_hd_hori_enha(mt_u8 data)
{
    reg_aria2_disp_hd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG);
    d.bitc.hd_hori_enha = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG), d.all);
}

mt_u8   reg_aria2_disp_get_hd_video_post_config_hd_hori_enha(void)
{
    return (*(volatile reg_aria2_disp_hd_video_post_config_t *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG)).bitc.hd_hori_enha;
}

void reg_aria2_disp_set_hd_video_post_config_hd_shoot_cfg(mt_u8 data)
{
    reg_aria2_disp_hd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG);
    d.bitc.hd_shoot_cfg = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG), d.all);
}

mt_u8   reg_aria2_disp_get_hd_video_post_config_hd_shoot_cfg(void)
{
    return (*(volatile reg_aria2_disp_hd_video_post_config_t *)((ulong)REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG)).bitc.hd_shoot_cfg;
}


/*!
  register ARIA_DISP_sd_video_post_config (read/write)
  */
void reg_aria2_disp_set_sd_video_post_config(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG), data);
}

mt_u32  reg_aria2_disp_get_sd_video_post_config(void)
{
    return (*(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG));
}

void reg_aria2_disp_set_sd_video_post_config_sd_leverage(mt_u8 data)
{
    reg_aria2_disp_sd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG);
    d.bitc.sd_leverage = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG), d.all);
}

mt_u8   reg_aria2_disp_get_sd_video_post_config_sd_leverage(void)
{
    return (*(volatile reg_aria2_disp_sd_video_post_config_t *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG)).bitc.sd_leverage;
}

void reg_aria2_disp_set_sd_video_post_config_sd_hp_enha(mt_u8 data)
{
    reg_aria2_disp_sd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG);
    d.bitc.sd_hp_enha = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG), d.all);
}

mt_u8   reg_aria2_disp_get_sd_video_post_config_sd_hp_enha(void)
{
    return (*(volatile reg_aria2_disp_sd_video_post_config_t *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG)).bitc.sd_hp_enha;
}

void reg_aria2_disp_set_sd_video_post_config_sd_hori_enha(mt_u8 data)
{
    reg_aria2_disp_sd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG);
    d.bitc.sd_hori_enha = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG), d.all);
}

mt_u8   reg_aria2_disp_get_sd_video_post_config_sd_hori_enha(void)
{
    return (*(volatile reg_aria2_disp_sd_video_post_config_t *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG)).bitc.sd_hori_enha;
}

void reg_aria2_disp_set_sd_video_post_config_sd_shoot_cfg(mt_u8 data)
{
    reg_aria2_disp_sd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG);
    d.bitc.sd_shoot_cfg = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG), d.all);
}

mt_u8   reg_aria2_disp_get_sd_video_post_config_sd_shoot_cfg(void)
{
    return (*(volatile reg_aria2_disp_sd_video_post_config_t *)((ulong)REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG)).bitc.sd_shoot_cfg;
}


/*!
  register ARIA_DISP_hd_video_effect_coef (read/write)
  */
void reg_aria2_disp_set_hd_video_effect_coef(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_HD_VIDEO_EFFECT_COEF), data);
}

mt_u32  reg_aria2_disp_get_hd_video_effect_coef(void)
{
    return (*(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_HD_VIDEO_EFFECT_COEF));
}

void reg_aria2_disp_set_hd_video_effect_coef_bright_coeff(mt_u8 data)
{
    reg_aria2_disp_hd_video_effect_coef_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_HD_VIDEO_EFFECT_COEF);
    d.bitc.bright_coeff = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_HD_VIDEO_EFFECT_COEF), d.all);
}

mt_u8   reg_aria2_disp_get_hd_video_effect_coef_bright_coeff(void)
{
    return (*(volatile reg_aria2_disp_hd_video_effect_coef_t *)((ulong)REG_ARIA2_DISP_HD_VIDEO_EFFECT_COEF)).bitc.bright_coeff;
}

void reg_aria2_disp_set_hd_video_effect_coef_contrast_coeff(mt_u8 data)
{
    reg_aria2_disp_hd_video_effect_coef_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_HD_VIDEO_EFFECT_COEF);
    d.bitc.contrast_coeff = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_HD_VIDEO_EFFECT_COEF), d.all);
}

mt_u8   reg_aria2_disp_get_hd_video_effect_coef_contrast_coeff(void)
{
    return (*(volatile reg_aria2_disp_hd_video_effect_coef_t *)((ulong)REG_ARIA2_DISP_HD_VIDEO_EFFECT_COEF)).bitc.contrast_coeff;
}

void reg_aria2_disp_set_hd_video_effect_coef_saturation_coeff(mt_u8 data)
{
    reg_aria2_disp_hd_video_effect_coef_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_HD_VIDEO_EFFECT_COEF);
    d.bitc.saturation_coeff = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_HD_VIDEO_EFFECT_COEF), d.all);
}

mt_u8   reg_aria2_disp_get_hd_video_effect_coef_saturation_coeff(void)
{
    return (*(volatile reg_aria2_disp_hd_video_effect_coef_t *)((ulong)REG_ARIA2_DISP_HD_VIDEO_EFFECT_COEF)).bitc.saturation_coeff;
}


/*!
  register ARIA_DISP_hd_video_hue_adjust (read/write)
  */
void reg_aria2_disp_set_hd_video_hue_adjust(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_HD_VIDEO_HUE_ADJUST), data);
}

mt_u32  reg_aria2_disp_get_hd_video_hue_adjust(void)
{
    return (*(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_HD_VIDEO_HUE_ADJUST));
}

void reg_aria2_disp_set_hd_video_hue_adjust_hd_sina(mt_u16 data)
{
    reg_aria2_disp_hd_video_hue_adjust_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_HD_VIDEO_HUE_ADJUST);
    d.bitc.hd_sina = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_HD_VIDEO_HUE_ADJUST), d.all);
}

mt_u16  reg_aria2_disp_get_hd_video_hue_adjust_hd_sina(void)
{
    return (*(volatile reg_aria2_disp_hd_video_hue_adjust_t *)((ulong)REG_ARIA2_DISP_HD_VIDEO_HUE_ADJUST)).bitc.hd_sina;
}

void reg_aria2_disp_set_hd_video_hue_adjust_hd_cosa(mt_u16 data)
{
    reg_aria2_disp_hd_video_hue_adjust_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_HD_VIDEO_HUE_ADJUST);
    d.bitc.hd_cosa = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_HD_VIDEO_HUE_ADJUST), d.all);
}

mt_u16  reg_aria2_disp_get_hd_video_hue_adjust_hd_cosa(void)
{
    return (*(volatile reg_aria2_disp_hd_video_hue_adjust_t *)((ulong)REG_ARIA2_DISP_HD_VIDEO_HUE_ADJUST)).bitc.hd_cosa;
}

void reg_aria2_disp_set_hd_video_hue_adjust_hd_hue_adjust_en(mt_u8 data)
{
    reg_aria2_disp_hd_video_hue_adjust_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_HD_VIDEO_HUE_ADJUST);
    d.bitc.hd_hue_adjust_en = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_HD_VIDEO_HUE_ADJUST), d.all);
}

mt_u8   reg_aria2_disp_get_hd_video_hue_adjust_hd_hue_adjust_en(void)
{
    return (*(volatile reg_aria2_disp_hd_video_hue_adjust_t *)((ulong)REG_ARIA2_DISP_HD_VIDEO_HUE_ADJUST)).bitc.hd_hue_adjust_en;
}

/*!
  register ARIA_DISP_sd_video_hue_adjust (read/write)
  */
void reg_aria2_disp_set_sd_video_hue_adjust(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_SD_VIDEO_HUE_ADJUST), data);
}

mt_u32  reg_aria2_disp_get_sd_video_hue_adjust(void)
{
    return (*(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_SD_VIDEO_HUE_ADJUST));
}

void reg_aria2_disp_set_sd_video_hue_adjust_sd_sina(mt_u16 data)
{
    reg_aria2_disp_sd_video_hue_adjust_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_SD_VIDEO_HUE_ADJUST);
    d.bitc.sd_sina = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_SD_VIDEO_HUE_ADJUST), d.all);
}

mt_u16  reg_aria2_disp_get_sd_video_hue_adjust_sd_sina(void)
{
    return (*(volatile reg_aria2_disp_sd_video_hue_adjust_t *)((ulong)REG_ARIA2_DISP_SD_VIDEO_HUE_ADJUST)).bitc.sd_sina;
}

void reg_aria2_disp_set_sd_video_hue_adjust_sd_cosa(mt_u16 data)
{
    reg_aria2_disp_sd_video_hue_adjust_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_SD_VIDEO_HUE_ADJUST);
    d.bitc.sd_cosa = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_SD_VIDEO_HUE_ADJUST), d.all);
}

mt_u16  reg_aria2_disp_get_sd_video_hue_adjust_sd_cosa(void)
{
    return (*(volatile reg_aria2_disp_sd_video_hue_adjust_t *)((ulong)REG_ARIA2_DISP_SD_VIDEO_HUE_ADJUST)).bitc.sd_cosa;
}

void reg_aria2_disp_set_sd_video_hue_adjust_sd_hue_adjust_en(mt_u8 data)
{
    reg_aria2_disp_sd_video_hue_adjust_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_SD_VIDEO_HUE_ADJUST);
    d.bitc.sd_hue_adjust_en = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_SD_VIDEO_HUE_ADJUST), d.all);
}

mt_u8   reg_aria2_disp_get_sd_video_hue_adjust_sd_hue_adjust_en(void)
{
    return (*(volatile reg_aria2_disp_sd_video_hue_adjust_t *)((ulong)REG_ARIA2_DISP_SD_VIDEO_HUE_ADJUST)).bitc.sd_hue_adjust_en;
}


/*!
  register ARIA_DISP_sd_video_effect_coef (read/write)
  */
void reg_aria2_disp_set_sd_video_effect_coef(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_SD_VIDEO_EFFECT_COEF), data);
}

mt_u32  reg_aria2_disp_get_sd_video_effect_coef(void)
{
    return (*(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_SD_VIDEO_EFFECT_COEF));
}

void reg_aria2_disp_set_sd_video_effect_coef_sd_bright_coeff(mt_u8 data)
{
    reg_aria2_disp_sd_video_effect_coef_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_SD_VIDEO_EFFECT_COEF);
    d.bitc.sd_bright_coeff = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_SD_VIDEO_EFFECT_COEF), d.all);
}

mt_u8   reg_aria2_disp_get_sd_video_effect_coef_sd_bright_coeff(void)
{
    return (*(volatile reg_aria2_disp_sd_video_effect_coef_t *)((ulong)REG_ARIA2_DISP_SD_VIDEO_EFFECT_COEF)).bitc.sd_bright_coeff;
}

void reg_aria2_disp_set_sd_video_effect_coef_sd_contrast_coeff(mt_u8 data)
{
    reg_aria2_disp_sd_video_effect_coef_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_SD_VIDEO_EFFECT_COEF);
    d.bitc.sd_contrast_coeff = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_SD_VIDEO_EFFECT_COEF), d.all);
}

mt_u8   reg_aria2_disp_get_sd_video_effect_coef_sd_contrast_coeff(void)
{
    return (*(volatile reg_aria2_disp_sd_video_effect_coef_t *)((ulong)REG_ARIA2_DISP_SD_VIDEO_EFFECT_COEF)).bitc.sd_contrast_coeff;
}

void reg_aria2_disp_set_sd_video_effect_coef_sd_saturation_coeff(mt_u8 data)
{
    reg_aria2_disp_sd_video_effect_coef_t d;
    d.all = *(volatile mt_u32 *)((ulong)REG_ARIA2_DISP_SD_VIDEO_EFFECT_COEF);
    d.bitc.sd_saturation_coeff = data;
    hal_put_u32((volatile unsigned long *)((ulong)REG_ARIA2_DISP_SD_VIDEO_EFFECT_COEF), d.all);
}

mt_u8   reg_aria2_disp_get_sd_video_effect_coef_sd_saturation_coeff(void)
{
    return (*(volatile reg_aria2_disp_sd_video_effect_coef_t *)((ulong)REG_ARIA2_DISP_SD_VIDEO_EFFECT_COEF)).bitc.sd_saturation_coeff;
}

/*!  end of file  */
