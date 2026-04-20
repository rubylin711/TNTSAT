/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _PQ_HAL_H
#define _PQ_HAL_H

//#include "../../../../../commpn/inc/mt_type.h"
#include "mt_type.h"

extern mt_u32 p_pq_addr;

#define REG_ARIA2_DISP_BASE_PHY 0xffd20000

#define REG_ARIA2_DISP_BASE_VIRT 0xf8d20000
#define REG_ARIA2_DISP_BASE (p_pq_addr)
#if defined(CONFIG_MT_CHIP_ARIA)
/*!
  the enum of ARIA_DISP registers
  */
#define REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG   (REG_ARIA2_DISP_BASE+0x0128)
#define REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG   (REG_ARIA2_DISP_BASE+0x012c)
#define REG_ARIA2_DISP_HD_VIDEO_EFFECT_COEF   (REG_ARIA2_DISP_BASE+0x0130)
#define REG_ARIA2_DISP_HD_VIDEO_HUE_ADJUST    (REG_ARIA2_DISP_BASE+0x0134)
#define REG_ARIA2_DISP_SD_VIDEO_HUE_ADJUST    (REG_ARIA2_DISP_BASE+0x7040)
#define REG_ARIA2_DISP_SD_VIDEO_EFFECT_COEF   (REG_ARIA2_DISP_BASE+0x7044)

#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
	/*!
  the enum of ARIA_DISP registers
  */
#define REG_ARIA2_DISP_HD_VIDEO_POST_CONFIG   (REG_ARIA2_DISP_BASE+0x00cc)
#define REG_ARIA2_DISP_SD_VIDEO_POST_CONFIG   (REG_ARIA2_DISP_BASE+0x00d0)
#define REG_ARIA2_DISP_HD_VIDEO_EFFECT_COEF   (REG_ARIA2_DISP_BASE+0x00d4)
#define REG_ARIA2_DISP_HD_VIDEO_HUE_ADJUST    (REG_ARIA2_DISP_BASE+0x3068)
#define REG_ARIA2_DISP_SD_VIDEO_HUE_ADJUST    (REG_ARIA2_DISP_BASE+0x306c)
#define REG_ARIA2_DISP_SD_VIDEO_EFFECT_COEF   (REG_ARIA2_DISP_BASE+0x00d8)

#endif

/*!
  the union of register reg_aria2_disp_hd_video_post_config
  */
typedef union reg_aria2_disp_hd_video_post_config
{
    mt_u32 all;
    struct
    {
        mt_u32 hd_leverage                 : 8;
        mt_u32 hd_hp_enha                  : 8;
        mt_u32 hd_hori_enha                : 8;
        mt_u32 hd_shoot_cfg                : 8;
    } bitc;
} reg_aria2_disp_hd_video_post_config_t;

/*!
  the union of register reg_aria2_disp_sd_video_post_config
  */
typedef union reg_aria2_disp_sd_video_post_config
{
    mt_u32 all;
    struct
    {
        mt_u32 sd_leverage                 : 8;
        mt_u32 sd_hp_enha                  : 8;
        mt_u32 sd_hori_enha                : 8;
        mt_u32 sd_shoot_cfg                : 8;
    } bitc;
} reg_aria2_disp_sd_video_post_config_t;

/*!
  the union of register reg_aria2_disp_hd_video_effect_coef
  */
typedef union reg_aria2_disp_hd_video_effect_coef
{
    mt_u32 all;
    struct
    {
        mt_u32 bright_coeff                : 8;
        mt_u32 contrast_coeff              : 8;
        mt_u32 saturation_coeff            : 8;
        mt_u32                             : 8;
    } bitc;
} reg_aria2_disp_hd_video_effect_coef_t;

/*!
  the union of register reg_aria2_disp_hd_video_hue_adjust
  */
typedef union reg_aria2_disp_hd_video_hue_adjust
{
    mt_u32 all;
    struct
    {
        mt_u32 hd_sina                     : 11;
        mt_u32                             : 5;
        mt_u32 hd_cosa                     : 11;
        mt_u32                             : 4;
        mt_u32 hd_hue_adjust_en            : 1;
    } bitc;
} reg_aria2_disp_hd_video_hue_adjust_t;

/*!
  the union of register reg_aria2_disp_sd_video_hue_adjust
  */
typedef union reg_aria2_disp_sd_video_hue_adjust
{
    mt_u32 all;
    struct
    {
        mt_u32 sd_sina                     : 11;
        mt_u32                             : 5;
        mt_u32 sd_cosa                     : 11;
        mt_u32                             : 4;
        mt_u32 sd_hue_adjust_en            : 1;
    } bitc;
} reg_aria2_disp_sd_video_hue_adjust_t;

/*!
  the union of register reg_aria2_disp_sd_video_effect_coef
  */
typedef union reg_aria2_disp_sd_video_effect_coef
{
    mt_u32 all;
    struct
    {
        mt_u32 sd_bright_coeff             : 8;
        mt_u32 sd_contrast_coeff           : 8;
        mt_u32 sd_saturation_coeff         : 8;
        mt_u32                             : 8;
    } bitc;
} reg_aria2_disp_sd_video_effect_coef_t;



#ifdef __cplusplus
extern "C" {
#endif

/*!
  register REGARIA_DISP_hd_video_post_config (read/write)
  */
void reg_aria2_disp_set_hd_video_post_config(mt_u32 data);
mt_u32  reg_aria2_disp_get_hd_video_post_config(void);
void reg_aria2_disp_set_hd_video_post_config_hd_leverage(mt_u8 data);
mt_u8   reg_aria2_disp_get_hd_video_post_config_hd_leverage(void);
void reg_aria2_disp_set_hd_video_post_config_hd_hp_enha(mt_u8 data);
mt_u8   reg_aria2_disp_get_hd_video_post_config_hd_hp_enha(void);
void reg_aria2_disp_set_hd_video_post_config_hd_hori_enha(mt_u8 data);
mt_u8   reg_aria2_disp_get_hd_video_post_config_hd_hori_enha(void);
void reg_aria2_disp_set_hd_video_post_config_hd_shoot_cfg(mt_u8 data);
mt_u8   reg_aria2_disp_get_hd_video_post_config_hd_shoot_cfg(void);

/*!
  register REGARIA_DISP_sd_video_post_config (read/write)
  */
void reg_aria2_disp_set_sd_video_post_config(mt_u32 data);
mt_u32  reg_aria2_disp_get_sd_video_post_config(void);
void reg_aria2_disp_set_sd_video_post_config_sd_leverage(mt_u8 data);
mt_u8   reg_aria2_disp_get_sd_video_post_config_sd_leverage(void);
void reg_aria2_disp_set_sd_video_post_config_sd_hp_enha(mt_u8 data);
mt_u8   reg_aria2_disp_get_sd_video_post_config_sd_hp_enha(void);
void reg_aria2_disp_set_sd_video_post_config_sd_hori_enha(mt_u8 data);
mt_u8   reg_aria2_disp_get_sd_video_post_config_sd_hori_enha(void);
void reg_aria2_disp_set_sd_video_post_config_sd_shoot_cfg(mt_u8 data);
mt_u8   reg_aria2_disp_get_sd_video_post_config_sd_shoot_cfg(void);

/*!
  register REGARIA_DISP_hd_video_effect_coef (read/write)
  */
void reg_aria2_disp_set_hd_video_effect_coef(mt_u32 data);
mt_u32  reg_aria2_disp_get_hd_video_effect_coef(void);
void reg_aria2_disp_set_hd_video_effect_coef_bright_coeff(mt_u8 data);
mt_u8   reg_aria2_disp_get_hd_video_effect_coef_bright_coeff(void);
void reg_aria2_disp_set_hd_video_effect_coef_contrast_coeff(mt_u8 data);
mt_u8   reg_aria2_disp_get_hd_video_effect_coef_contrast_coeff(void);
void reg_aria2_disp_set_hd_video_effect_coef_saturation_coeff(mt_u8 data);
mt_u8   reg_aria2_disp_get_hd_video_effect_coef_saturation_coeff(void);

/*!
  register REGARIA_DISP_hd_video_hue_adjust (read/write)
  */
void reg_aria2_disp_set_hd_video_hue_adjust(mt_u32 data);
mt_u32  reg_aria2_disp_get_hd_video_hue_adjust(void);
void reg_aria2_disp_set_hd_video_hue_adjust_hd_sina(mt_u16 data);
mt_u16  reg_aria2_disp_get_hd_video_hue_adjust_hd_sina(void);
void reg_aria2_disp_set_hd_video_hue_adjust_hd_cosa(mt_u16 data);
mt_u16  reg_aria2_disp_get_hd_video_hue_adjust_hd_cosa(void);
void reg_aria2_disp_set_hd_video_hue_adjust_hd_hue_adjust_en(mt_u8 data);
mt_u8   reg_aria2_disp_get_hd_video_hue_adjust_hd_hue_adjust_en(void);

/*!
  register REGARIA_DISP_sd_video_hue_adjust (read/write)
  */
void reg_aria2_disp_set_sd_video_hue_adjust(mt_u32 data);
mt_u32  reg_aria2_disp_get_sd_video_hue_adjust(void);
void reg_aria2_disp_set_sd_video_hue_adjust_sd_sina(mt_u16 data);
mt_u16  reg_aria2_disp_get_sd_video_hue_adjust_sd_sina(void);
void reg_aria2_disp_set_sd_video_hue_adjust_sd_cosa(mt_u16 data);
mt_u16  reg_aria2_disp_get_sd_video_hue_adjust_sd_cosa(void);
void reg_aria2_disp_set_sd_video_hue_adjust_sd_hue_adjust_en(mt_u8 data);
mt_u8   reg_aria2_disp_get_sd_video_hue_adjust_sd_hue_adjust_en(void);

/*!
  register REGARIA_DISP_sd_video_effect_coef (read/write)
  */
void reg_aria2_disp_set_sd_video_effect_coef(mt_u32 data);
mt_u32  reg_aria2_disp_get_sd_video_effect_coef(void);
void reg_aria2_disp_set_sd_video_effect_coef_sd_bright_coeff(mt_u8 data);
mt_u8   reg_aria2_disp_get_sd_video_effect_coef_sd_bright_coeff(void);
void reg_aria2_disp_set_sd_video_effect_coef_sd_contrast_coeff(mt_u8 data);
mt_u8   reg_aria2_disp_get_sd_video_effect_coef_sd_contrast_coeff(void);
void reg_aria2_disp_set_sd_video_effect_coef_sd_saturation_coeff(mt_u8 data);
mt_u8   reg_aria2_disp_get_sd_video_effect_coef_sd_saturation_coeff(void);


#ifdef __cplusplus
}
#endif

#endif /* _PQ_HAL_H */

