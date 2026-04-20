/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HD_ENC_ARIA_REG_H__
#define __HD_ENC_ARIA_REG_H__
#include "mt_type.h"
/*!
 comments
  */
extern ulong p_hdenc_addr;
#define REG_ARIA_HD_ENCODER_BASE_PHY 0xffdc0000

#define REG_ARIA_HD_ENCODER_BASE_VIRT 0xf8dc0000

#define REG_SYMPHONY_HD_ENCODER_BASE 0xbf470000

#if defined(CONFIG_MT_CHIP_ARIA) || defined (SYMPHONY_LINUX)
#define REG_ARIA_HD_ENCODER_BASE (p_hdenc_addr)
#else
#define REG_ARIA_HD_ENCODER_BASE 0xbfd60000
#endif

/*!
  the enum of ARIA_HD_Encoder registers
  */
#define  REG_ARIA_HD_ENCODER_BASIC_CFG                (REG_ARIA_HD_ENCODER_BASE + 0x0)
#define  REG_ARIA_HD_ENCODER_Y_PARAMETER_LEVEL0       (REG_ARIA_HD_ENCODER_BASE + 0x4)
#define  REG_ARIA_HD_ENCODER_Y_SYNC_LEVEL             (REG_ARIA_HD_ENCODER_BASE + 0x8)
#define  REG_ARIA_HD_ENCODER_PBPR_PARAMETER0_LEVEL    (REG_ARIA_HD_ENCODER_BASE + 0xc)
#define  REG_ARIA_HD_ENCODER_PBPR_PARAMETER1_LEVEL    (REG_ARIA_HD_ENCODER_BASE + 0x10)
#define  REG_ARIA_HD_ENCODER_HD_CFG_INFO              (REG_ARIA_HD_ENCODER_BASE + 0x14)
#define  REG_ARIA_HD_ENCODER_HSYNC_PARAMETER0         (REG_ARIA_HD_ENCODER_BASE + 0x18)
#define  REG_ARIA_HD_ENCODER_HSYNC_PARAMETER1         (REG_ARIA_HD_ENCODER_BASE + 0x1c)
#define  REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER0  (REG_ARIA_HD_ENCODER_BASE + 0x20)
#define  REG_ARIA_HD_ENCODER_REG_H_ACTIVE_PARAMETER1  (REG_ARIA_HD_ENCODER_BASE + 0x24)
#define  REG_ARIA_HD_ENCODER_REG_V_PARAMETER0         (REG_ARIA_HD_ENCODER_BASE + 0x28)
#define  REG_ARIA_HD_ENCODER_REG_V_PARAMETER1         (REG_ARIA_HD_ENCODER_BASE + 0x2c)
#define  REG_ARIA_HD_ENCODER_REG_H_OSD_PARAMETER      (REG_ARIA_HD_ENCODER_BASE + 0x30)
#define  REG_ARIA_HD_ENCODER_REG_V_OSD_PARAMETER      (REG_ARIA_HD_ENCODER_BASE + 0x34)
#define  REG_ARIA_HD_ENCODER_REG_H_VIDEO_PARAMETER    (REG_ARIA_HD_ENCODER_BASE + 0x38)
#define  REG_ARIA_HD_ENCODER_REG_V_VIDEO_PARAMETER    (REG_ARIA_HD_ENCODER_BASE + 0x3c)
#define  REG_ARIA_HD_ENCODER_REG_COLOR_BAR            (REG_ARIA_HD_ENCODER_BASE + 0x40)
#define  REG_ARIA_HD_ENCODER_CBAR_SET_DATA            (REG_ARIA_HD_ENCODER_BASE + 0x44)
#define  REG_ARIA_HD_ENCODER_GAMA_CORRECTION          (REG_ARIA_HD_ENCODER_BASE + 0x48)
#define  REG_ARIA_HD_ENCODER_HD_ENCODER_STATUS        (REG_ARIA_HD_ENCODER_BASE + 0x4c)
#define  REG_ARIA_HD_ENCODER_HDMI_H_FPORCHWIDTH       (REG_ARIA_HD_ENCODER_BASE + 0x50)
#define  REG_ARIA_HD_ENCODER_HDMI_H_SYNCWIDTH         (REG_ARIA_HD_ENCODER_BASE + 0x54)
#define  REG_ARIA_HD_ENCODER_HDMI_H_BPORCHWIDTH       (REG_ARIA_HD_ENCODER_BASE + 0x58)
#define  REG_ARIA_HD_ENCODER_HDMI_H_ACTIVEWIDTH       (REG_ARIA_HD_ENCODER_BASE + 0x5c)
#define  REG_ARIA_HD_ENCODER_HDMI_V_FPORCHHEIGHT      (REG_ARIA_HD_ENCODER_BASE + 0x60)
#define  REG_ARIA_HD_ENCODER_HDMI_V_SYNCHEIGHT        (REG_ARIA_HD_ENCODER_BASE + 0x64)
#define  REG_ARIA_HD_ENCODER_HDMI_V_BPORCHHEIGHT      (REG_ARIA_HD_ENCODER_BASE + 0x68)
#define  REG_ARIA_HD_ENCODER_HDMI_V_ACTIVEHEIGHT      (REG_ARIA_HD_ENCODER_BASE + 0x6c)
#define  REG_ARIA_HD_ENCODER_HDMI_H_HALFLINEWIDTH     (REG_ARIA_HD_ENCODER_BASE + 0x70)
#define  REG_ARIA_HD_ENCODER_HDMI_HSYNC_POLA          (REG_ARIA_HD_ENCODER_BASE + 0x74)
#define  REG_ARIA_HD_ENCODER_HDMI_VSYNC_POLA          (REG_ARIA_HD_ENCODER_BASE + 0x78)
#define  REG_ARIA_HD_ENCODER_TOP_START_DEBUG          (REG_ARIA_HD_ENCODER_BASE + 0x7c)
#define  REG_ARIA_HD_ENCODER_TOP_START_NUM            (REG_ARIA_HD_ENCODER_BASE + 0x80)
#define  REG_ARIA_HD_ENCODER_TOP_START_ERR            (REG_ARIA_HD_ENCODER_BASE + 0x84)
#define  REG_ARIA_HD_ENCODER_TOP_START_WIDTH          (REG_ARIA_HD_ENCODER_BASE + 0x88)
#define  REG_ARIA_HD_ENCODER_TOP_START_CNT            (REG_ARIA_HD_ENCODER_BASE + 0x8c)
#define  REG_ARIA_HD_ENCODER_IRQ_STATUS1              (REG_ARIA_HD_ENCODER_BASE + 0x90)
#define  REG_ARIA_HD_ENCODER_DAC_SEL                  (REG_ARIA_HD_ENCODER_BASE + 0x94)
#define  REG_ARIA_HD_ENCODER_COEF_V_MSB               (REG_ARIA_HD_ENCODER_BASE + 0x98)
#define  REG_ARIA_HD_ENCODER_COEF_U_MSB               (REG_ARIA_HD_ENCODER_BASE + 0x9c)
#define  REG_ARIA_HD_ENCODER_COEF_V_ADJ_1             (REG_ARIA_HD_ENCODER_BASE + 0xa0)
#define  REG_ARIA_HD_ENCODER_COEF_V_ADJ_2             (REG_ARIA_HD_ENCODER_BASE + 0xa4)
#define  REG_ARIA_HD_ENCODER_COEF_V_ADJ_3             (REG_ARIA_HD_ENCODER_BASE + 0xa8)
#define  REG_ARIA_HD_ENCODER_COEF_V_ADJ_4             (REG_ARIA_HD_ENCODER_BASE + 0xac)
#define  REG_ARIA_HD_ENCODER_COEF_U_ADJ_1             (REG_ARIA_HD_ENCODER_BASE + 0xb0)
#define  REG_ARIA_HD_ENCODER_COEF_U_ADJ_2             (REG_ARIA_HD_ENCODER_BASE + 0xb4)
#define  REG_ARIA_HD_ENCODER_COEF_U_ADJ_3             (REG_ARIA_HD_ENCODER_BASE + 0xb8)
#define  REG_ARIA_HD_ENCODER_COEF_U_ADJ_4             (REG_ARIA_HD_ENCODER_BASE + 0xbc)
#define  REG_ARIA_HD_ENCODER_DAC_TEST                 (REG_ARIA_HD_ENCODER_BASE + 0xc0)
#define  REG_ARIA_HD_ENCODER_COEF_Y_MSB               (REG_ARIA_HD_ENCODER_BASE + 0xc4)
#define  REG_ARIA_HD_ENCODER_COEF_Y_ADJ_1             (REG_ARIA_HD_ENCODER_BASE + 0xc8)
#define  REG_ARIA_HD_ENCODER_COEF_Y_ADJ_2             (REG_ARIA_HD_ENCODER_BASE + 0xcc)
#define  REG_ARIA_HD_ENCODER_COEF_Y_ADJ_3             (REG_ARIA_HD_ENCODER_BASE + 0xd0)
#define  REG_ARIA_HD_ENCODER_COEF_Y_ADJ_4             (REG_ARIA_HD_ENCODER_BASE + 0xd4)
#define  REG_ARIA_HD_ENCODER_OS_FILTER_CTRL           (REG_ARIA_HD_ENCODER_BASE + 0xd8)
#define  REG_ARIA_HD_ENCODER_SIGN_CTRL                (REG_ARIA_HD_ENCODER_BASE + 0xe0)
#define  REG_ARIA_HD_ENCODER_V_H_CNT                  (REG_ARIA_HD_ENCODER_BASE + 0xe4)
#define  REG_ARIA_HD_ENCODER_V_H_STATE                (REG_ARIA_HD_ENCODER_BASE + 0xe8)
#define  REG_ARIA_HD_ENCODER_VDAC_ADJUSTING_Y         (REG_ARIA_HD_ENCODER_BASE + 0xec)
#define  REG_ARIA_HD_ENCODER_VDAC_ADJUSTING_UV        (REG_ARIA_HD_ENCODER_BASE + 0xf0)
#define  REG_ARIA_HD_ENCODER_MASK_IRQ                 (REG_ARIA_HD_ENCODER_BASE + 0xf4)
#define  REG_ARIA_HD_ENCODER_HDMI_Y_MAP               (REG_ARIA_HD_ENCODER_BASE + 0xf8)
#define  REG_ARIA_HD_ENCODER_HDMI_UV_MAP              (REG_ARIA_HD_ENCODER_BASE + 0xfc)

/*!
  the union of register reg_aria_hd_encoder_basic_cfg
  */
typedef union reg_aria_hd_encoder_basic_cfg
{
    mt_u32 all;
    struct
    {
        mt_u32 resolusion                  : 3;
        mt_u32                             : 1;
        mt_u32 interlace_mode              : 1;
        mt_u32                             : 2;
        mt_u32 top_start_config            : 1;
        mt_u32 start_num                   : 10;
        mt_u32                             : 2;
        mt_u32 start_num1                  : 10;
        mt_u32                             : 1;
        mt_u32 clk_auto_gate_en            : 1;
    } bitc;
} reg_aria_hd_encoder_basic_cfg_t;

/*!
  the union of register reg_aria_hd_encoder_y_parameter_level0
  */
typedef union reg_aria_hd_encoder_y_parameter_level0
{
    mt_u32 all;
    struct
    {
        mt_u32 y_a_level                   : 10;
        mt_u32                             : 2;
        mt_u32 is_frame_packing            : 1;
        mt_u32 de_mode                     : 1;
        mt_u32                             : 2;
        mt_u32 y_blank_level               : 10;
        mt_u32                             : 2;
        mt_u32 cable_dct_en                : 1;
        mt_u32 hdmi_lock                   : 1;
        mt_u32 hd_lock                     : 1;
        mt_u32                             : 1;
    } bitc;
} reg_aria_hd_encoder_y_parameter_level0_t;

/*!
  the union of register reg_aria_hd_encoder_y_sync_level
  */
typedef union reg_aria_hd_encoder_y_sync_level
{
    mt_u32 all;
    struct
    {
        mt_u32 y_hsync_level               : 10;
        mt_u32                             : 6;
        mt_u32 y_hi_sync_level             : 10;
        mt_u32                             : 6;
    } bitc;
} reg_aria_hd_encoder_y_sync_level_t;

/*!
  the union of register reg_aria_hd_encoder_pbpr_parameter0_level
  */
typedef union reg_aria_hd_encoder_pbpr_parameter0_level
{
    mt_u32 all;
    struct
    {
        mt_u32 v_pbpr_center               : 10;
        mt_u32                             : 2;
        mt_u32 u_pbpr_center               : 10;
        mt_u32                             : 10;
    } bitc;
} reg_aria_hd_encoder_pbpr_parameter0_level_t;

/*!
  the union of register reg_aria_hd_encoder_pbpr_parameter1_level
  */
typedef union reg_aria_hd_encoder_pbpr_parameter1_level
{
    mt_u32 all;
    struct
    {
        mt_u32 v_pbpr_b                    : 10;
        mt_u32                             : 2;
        mt_u32 u_pbpr_b                    : 10;
        mt_u32                             : 10;
    } bitc;
} reg_aria_hd_encoder_pbpr_parameter1_level_t;

/*!
  the union of register reg_aria_hd_encoder_hd_cfg_info
  */
typedef union reg_aria_hd_encoder_hd_cfg_info
{
    mt_u32 all;
    struct
    {
        mt_u32 reg_field_mode              : 1;
        mt_u32 frame_rate                  : 1;
        mt_u32 reg_hdtv_mode               : 1;
        mt_u32 comp_mode_on                : 1;
        mt_u32 video_format                : 1;
        mt_u32 hi_sync_enable              : 1;
        mt_u32 pbpr_data_offset_enable     : 1;
        mt_u32                             : 1;
        mt_u32 delay_y                     : 3;
        mt_u32                             : 1;
        mt_u32 py_limit_enable             : 1;
        mt_u32 pbpr_limit_enablet          : 1;
        mt_u32 y_limit_enable              : 1;
        mt_u32 cbcr_limit_enable           : 1;
        mt_u32 enable_1080p                : 1;
        mt_u32                             : 3;
        mt_u32 pbpr_filter_en              : 1;
        mt_u32                             : 1;
        mt_u32 ycbcr2rgb_en                : 1;
        mt_u32 r_sync_en                   : 1;
        mt_u32 g_sync_en                   : 1;
        mt_u32 b_sync_en                   : 1;
        mt_u32                             : 2;
        mt_u32 timing_reg_write_finish     : 1;
        mt_u32 vsync_update_timing_enable  : 1;
        mt_u32 h_timing_update_start       : 1;
        mt_u32 reg_disp_on                 : 1;
    } bitc;
} reg_aria_hd_encoder_hd_cfg_info_t;

/*!
  the union of register reg_aria_hd_encoder_hsync_parameter0
  */
typedef union reg_aria_hd_encoder_hsync_parameter0
{
    mt_u32 all;
    struct
    {
        mt_u32 reg_h_sync_fporchwidth      : 12;
        mt_u32 delay_pr                    : 3;
        mt_u32                             : 1;
        mt_u32 reg_h_syncwidth             : 9;
        mt_u32                             : 3;
        mt_u32 delay_pb                    : 3;
        mt_u32                             : 1;
    } bitc;
} reg_aria_hd_encoder_hsync_parameter0_t;

/*!
  the union of register reg_aria_hd_encoder_hsync_parameter1
  */
typedef union reg_aria_hd_encoder_hsync_parameter1
{
    mt_u32 all;
    struct
    {
        mt_u32 reg_h_sync_bporch           : 9;
        mt_u32                             : 7;
        mt_u32 reg_h_sync_activewidth      : 11;
        mt_u32                             : 5;
    } bitc;
} reg_aria_hd_encoder_hsync_parameter1_t;

/*!
  the union of register reg_aria_hd_encoder_reg_h_active_parameter0
  */
typedef union reg_aria_hd_encoder_reg_h_active_parameter0
{
    mt_u32 all;
    struct
    {
        mt_u32 reg_h_active_fporch         : 12;
        mt_u32                             : 4;
        mt_u32 reg_h_active_bporch         : 9;
        mt_u32                             : 7;
    } bitc;
} reg_aria_hd_encoder_reg_h_active_parameter0_t;

/*!
  the union of register reg_aria_hd_encoder_reg_h_active_parameter1
  */
typedef union reg_aria_hd_encoder_reg_h_active_parameter1
{
    mt_u32 all;
    struct
    {
        mt_u32 reg_h_active_activewidth    : 13;
        mt_u32                             : 3;
        mt_u32 reg_vsyncpixelcnt           : 11;
        mt_u32                             : 5;
    } bitc;
} reg_aria_hd_encoder_reg_h_active_parameter1_t;

/*!
  the union of register reg_aria_hd_encoder_reg_v_parameter0
  */
typedef union reg_aria_hd_encoder_reg_v_parameter0
{
    mt_u32 all;
    struct
    {
        mt_u32 reg_v_fporchheight          : 9;
        mt_u32                             : 7;
        mt_u32 reg_v_syncheight            : 9;
        mt_u32                             : 7;
    } bitc;
} reg_aria_hd_encoder_reg_v_parameter0_t;

/*!
  the union of register reg_aria_hd_encoder_reg_v_parameter1
  */
typedef union reg_aria_hd_encoder_reg_v_parameter1
{
    mt_u32 all;
    struct
    {
        mt_u32 reg_v_bporchheight          : 9;
        mt_u32                             : 7;
        mt_u32 reg_v_activeheight          : 12;
        mt_u32                             : 4;
    } bitc;
} reg_aria_hd_encoder_reg_v_parameter1_t;

/*!
  the union of register reg_aria_hd_encoder_reg_h_osd_parameter
  */
typedef union reg_aria_hd_encoder_reg_h_osd_parameter
{
    mt_u32 all;
    struct
    {
        mt_u32 reg_h_osd_start             : 13;
        mt_u32 frame_rate                  : 3;
        mt_u32 reg_h_osd_end               : 13;
        mt_u32                             : 3;
    } bitc;
} reg_aria_hd_encoder_reg_h_osd_parameter_t;

/*!
  the union of register reg_aria_hd_encoder_reg_v_osd_parameter
  */
typedef union reg_aria_hd_encoder_reg_v_osd_parameter
{
    mt_u32 all;
    struct
    {
        mt_u32 reg_v_osd_start             : 12;
        mt_u32                             : 4;
        mt_u32 reg_v_osd_end               : 12;
        mt_u32                             : 4;
    } bitc;
} reg_aria_hd_encoder_reg_v_osd_parameter_t;

/*!
  the union of register reg_aria_hd_encoder_reg_h_video_parameter
  */
typedef union reg_aria_hd_encoder_reg_h_video_parameter
{
    mt_u32 all;
    struct
    {
        mt_u32 reg_h_video_start           : 13;
        mt_u32                             : 3;
        mt_u32 reg_h_video_end             : 13;
        mt_u32                             : 3;
    } bitc;
} reg_aria_hd_encoder_reg_h_video_parameter_t;

/*!
  the union of register reg_aria_hd_encoder_reg_v_video_parameter
  */
typedef union reg_aria_hd_encoder_reg_v_video_parameter
{
    mt_u32 all;
    struct
    {
        mt_u32 reg_v_video_start           : 12;
        mt_u32                             : 4;
        mt_u32 reg_v_video_end             : 12;
        mt_u32                             : 4;
    } bitc;
} reg_aria_hd_encoder_reg_v_video_parameter_t;

/*!
  the union of register reg_aria_hd_encoder_reg_color_bar
  */
typedef union reg_aria_hd_encoder_reg_color_bar
{
    mt_u32 all;
    struct
    {
        mt_u32 cbar_width                  : 10;
        mt_u32                             : 21;
        mt_u32 cbar_enable                 : 1;
    } bitc;
} reg_aria_hd_encoder_reg_color_bar_t;

/*!
  the union of register reg_aria_hd_encoder_cbar_set_data
  */
typedef union reg_aria_hd_encoder_cbar_set_data
{
    mt_u32 all;
    struct
    {
        mt_u32 y_cbar_set_data             : 10;
        mt_u32 pb_cbar_set_data            : 10;
        mt_u32 pr_cbar_set_data            : 10;
        mt_u32                             : 1;
        mt_u32 cbar_set_enable             : 1;
    } bitc;
} reg_aria_hd_encoder_cbar_set_data_t;

/*!
  the union of register reg_aria_hd_encoder_gama_correction
  */
typedef union reg_aria_hd_encoder_gama_correction
{
    mt_u32 all;
    struct
    {
        mt_u32 gama_coef                   : 7;
        mt_u32                             : 23;
        mt_u32 gama_pole                   : 1;
        mt_u32 gama_enable                 : 1;
    } bitc;
} reg_aria_hd_encoder_gama_correction_t;

/*!
  the union of register reg_aria_hd_encoder_hd_encoder_status
  */
typedef union reg_aria_hd_encoder_hd_encoder_status
{
    mt_u32 all;
    struct
    {
        mt_u32 new_addr_flag               : 1;
        mt_u32 o_even_field                : 1;
        mt_u32 o_top_field                 : 1;
        mt_u32 update_disp_h_timing        : 1;
        mt_u32 bot_field                   : 1;
        mt_u32                             : 19;
        mt_u32 empty_flag                  : 1;
        mt_u32                             : 3;
        mt_u32 sync_mode                   : 4;
    } bitc;
} reg_aria_hd_encoder_hd_encoder_status_t;

/*!
  the union of register reg_aria_hd_encoder_hdmi_h_fporchwidth
  */
typedef union reg_aria_hd_encoder_hdmi_h_fporchwidth
{
    mt_u32 all;
    struct
    {
        mt_u32 hdmi_h_fporchwidth          : 12;
        mt_u32                             : 20;
    } bitc;
} reg_aria_hd_encoder_hdmi_h_fporchwidth_t;

/*!
  the union of register reg_aria_hd_encoder_hdmi_h_syncwidth
  */
typedef union reg_aria_hd_encoder_hdmi_h_syncwidth
{
    mt_u32 all;
    struct
    {
        mt_u32 hdmi_h_syncwidth            : 9;
        mt_u32                             : 23;
    } bitc;
} reg_aria_hd_encoder_hdmi_h_syncwidth_t;

/*!
  the union of register reg_aria_hd_encoder_hdmi_h_bporchwidth
  */
typedef union reg_aria_hd_encoder_hdmi_h_bporchwidth
{
    mt_u32 all;
    struct
    {
        mt_u32 hdmi_h_bporchwidth          : 9;
        mt_u32                             : 23;
    } bitc;
} reg_aria_hd_encoder_hdmi_h_bporchwidth_t;

/*!
  the union of register reg_aria_hd_encoder_hdmi_h_activewidth
  */
typedef union reg_aria_hd_encoder_hdmi_h_activewidth
{
    mt_u32 all;
    struct
    {
        mt_u32 hdmi_h_activewidth          : 13;
        mt_u32                             : 19;
    } bitc;
} reg_aria_hd_encoder_hdmi_h_activewidth_t;

/*!
  the union of register reg_aria_hd_encoder_hdmi_v_fporchheight
  */
typedef union reg_aria_hd_encoder_hdmi_v_fporchheight
{
    mt_u32 all;
    struct
    {
        mt_u32 hdmi_v_fporchheight         : 9;
        mt_u32                             : 23;
    } bitc;
} reg_aria_hd_encoder_hdmi_v_fporchheight_t;

/*!
  the union of register reg_aria_hd_encoder_hdmi_v_syncheight
  */
typedef union reg_aria_hd_encoder_hdmi_v_syncheight
{
    mt_u32 all;
    struct
    {
        mt_u32 hdmi_v_syncheight           : 9;
        mt_u32                             : 23;
    } bitc;
} reg_aria_hd_encoder_hdmi_v_syncheight_t;

/*!
  the union of register reg_aria_hd_encoder_hdmi_v_bporchheight
  */
typedef union reg_aria_hd_encoder_hdmi_v_bporchheight
{
    mt_u32 all;
    struct
    {
        mt_u32 hdmi_v_bporchheight         : 9;
        mt_u32                             : 23;
    } bitc;
} reg_aria_hd_encoder_hdmi_v_bporchheight_t;

/*!
  the union of register reg_aria_hd_encoder_hdmi_v_activeheight
  */
typedef union reg_aria_hd_encoder_hdmi_v_activeheight
{
    mt_u32 all;
    struct
    {
        mt_u32 hdmi_v_activeheight         : 12;
        mt_u32                             : 20;
    } bitc;
} reg_aria_hd_encoder_hdmi_v_activeheight_t;

/*!
  the union of register reg_aria_hd_encoder_hdmi_h_halflinewidth
  */
typedef union reg_aria_hd_encoder_hdmi_h_halflinewidth
{
    mt_u32 all;
    struct
    {
        mt_u32 hdmi_h_halflinewidth        : 11;
        mt_u32                             : 21;
    } bitc;
} reg_aria_hd_encoder_hdmi_h_halflinewidth_t;

/*!
  the union of register reg_aria_hd_encoder_hdmi_hsync_pola
  */
typedef union reg_aria_hd_encoder_hdmi_hsync_pola
{
    mt_u32 all;
    struct
    {
        mt_u32 hdmi_hsync_pola             : 1;
        mt_u32                             : 31;
    } bitc;
} reg_aria_hd_encoder_hdmi_hsync_pola_t;

/*!
  the union of register reg_aria_hd_encoder_hdmi_vsync_pola
  */
typedef union reg_aria_hd_encoder_hdmi_vsync_pola
{
    mt_u32 all;
    struct
    {
        mt_u32 hdmi_vsync_pola             : 1;
        mt_u32                             : 31;
    } bitc;
} reg_aria_hd_encoder_hdmi_vsync_pola_t;

/*!
  the union of register reg_aria_hd_encoder_dac_sel
  */
typedef union reg_aria_hd_encoder_dac_sel
{
    mt_u32 all;
    struct
    {
        mt_u32 dac_3_mode                  : 2;
        mt_u32 dac_2_mode                  : 2;
        mt_u32 dac_1_mode                  : 2;
        mt_u32 dac_0_mode                  : 2;
        mt_u32 dac_3_hdsd_sel              : 1;
        mt_u32 dac_2_hdsd_sel              : 1;
        mt_u32 dac_1_hdsd_sel              : 1;
        mt_u32 dac_0_hdsd_sel              : 1;
        mt_u32                             : 4;
        mt_u32 hdmi_422_444                : 1;
        mt_u32 cbcr_order                  : 1;
        mt_u32                             : 14;
    } bitc;
} reg_aria_hd_encoder_dac_sel_t;

/*!
  the union of register reg_aria_hd_encoder_os_filter_ctrl
  */
typedef union reg_aria_hd_encoder_os_filter_ctrl
{
    mt_u32 all;
    struct
    {
        mt_u32 mux_sdp_4_3                 : 2;
        mt_u32 mux_sdp_6_5                 : 2;
        mt_u32 mux_sdp_7                   : 1;
        mt_u32 mux_sdp_8                   : 1;
        mt_u32                             : 2;
        mt_u32 mux_sdp_0                   : 1;
        mt_u32 mux_sdp_2_1                 : 2;
        mt_u32                             : 5;
        mt_u32 os_en                       : 1;
        mt_u32                             : 12;
        mt_u32 sample_ctrl                 : 1;
        mt_u32 sync_filter_en              : 1;
        mt_u32 int_state_rd_clr_en         : 1;
    } bitc;
} reg_aria_hd_encoder_os_filter_ctrl_t;

/*!
  the union of register reg_aria_hd_encoder_sign_ctrl
  */
typedef union reg_aria_hd_encoder_sign_ctrl
{
    u32 all;
    struct
    {
        u32 sign_ctrl                   : 12;
        u32                             : 4;
        u32 irq_cable_mask0_dct_fall_dac0: 1;
        u32 irq_cable_mask0_dct_rise_dac0: 1;
        u32 irq_cable_mask0_dct_fall_dac1: 1;
        u32 irq_cable_mask0_dct_rise_dac1: 1;
        u32 irq_cable_mask0_dct_fall_dac2: 1;
        u32 irq_cable_mask0_dct_rise_dac2: 1;
        u32 irq_cable_mask0_dct_fall_dac3: 1;
        u32 irq_cable_mask0_dct_rise_dac3: 1;
        u32 irq_cable_mask1_dct_fall_dac0: 1;
        u32 irq_cable_mask1_dct_rise_dac0: 1;
        u32 irq_cable_mask1_dct_fall_dac1: 1;
        u32 irq_cable_mask1_dct_rise_dac1: 1;
        u32 irq_cable_mask1_dct_fall_dac2: 1;
        u32 irq_cable_mask1_dct_rise_dac2: 1;
        u32 irq_cable_mask1_dct_fall_dac3: 1;
        u32 irq_cable_mask1_dct_rise_dac3: 1;
    } bitc;
} reg_aria_hd_encoder_sign_ctrl_t;

/*!
  the union of register reg_symphony_hdvenc_mask_riq
  */
typedef union reg_aria_hdvenc_mask_riq
{
    u32 all;
    struct
    {
        u32 line_cnt                : 12;
        u32 cable_dct_fall_dac0_irq               : 1;
        u32 cable_dct_rise_dac0_irq               : 1;
        u32 cable_dct_fall_dac1_irq               : 1;
        u32 cable_dct_rise_dac1_irq               : 1;
        u32 bot_field_irq                 : 1;
        u32 top_field_irq                 : 1;
        u32 bot_start_irq                 : 1;
        u32 top_start_irq                 : 1;
        u32 cable_dct_fall_dac2_irq               : 1;
        u32 cable_dct_rise_dac2_irq               : 1;
        u32 cable_dct_fall_dac3_irq               : 1;
        u32 cable_dct_rise_dac3_irq               : 1;
        u32                             : 8;
    } bitc;
} reg_aria_hdvenc_mask_riq_t;

typedef struct
{
    volatile reg_aria_hd_encoder_basic_cfg_t               basic_cfg    ; /* 0x0 */
    volatile reg_aria_hd_encoder_y_parameter_level0_t    y_parameter_level0 ; /* 0x4 */
    volatile reg_aria_hd_encoder_y_sync_level_t          y_sync_level      ; /* 0x8 */
    volatile reg_aria_hd_encoder_pbpr_parameter0_level_t      pbpr_parameter0_level ; /* 0xc */
    volatile reg_aria_hd_encoder_pbpr_parameter1_level_t      pbpr_parameter1_level ; /* 0x10 */
    volatile reg_aria_hd_encoder_hd_cfg_info_t            hd_cfg_info   ; /* 0x14 */
    volatile reg_aria_hd_encoder_hsync_parameter0_t   hsync_parameter0   ; /* 0x18 */
    volatile reg_aria_hd_encoder_hsync_parameter1_t    hsync_parameter1 ; /* 0x1c */
    volatile reg_aria_hd_encoder_reg_h_active_parameter0_t   h_active_parameter0 ; /* 0x20 */
    volatile reg_aria_hd_encoder_reg_h_active_parameter1_t   h_active_parameter1 ; /* 0x24 */
    volatile reg_aria_hd_encoder_reg_v_parameter0_t           v_parameter0  ; /* 0x28 */
    volatile reg_aria_hd_encoder_reg_v_parameter1_t          v_parameter1 ; /* 0x2c */
    volatile reg_aria_hd_encoder_reg_h_osd_parameter_t     h_osd_parameter ; /* 0x30 */
    volatile reg_aria_hd_encoder_reg_v_osd_parameter_t     v_osd_parameter  ; /* 0x34 */
    volatile reg_aria_hd_encoder_reg_h_video_parameter_t   h_video_parameter ; /* 0x38 */
    volatile reg_aria_hd_encoder_reg_v_video_parameter_t   v_video_parameter ; /* 0x3c */
    volatile reg_aria_hd_encoder_reg_color_bar_t           color_bar   ; /* 0x40 */
    volatile reg_aria_hd_encoder_cbar_set_data_t           cbar_set_data    ; /* 0x44 */
    volatile reg_aria_hd_encoder_gama_correction_t        gama_correction    ; /* 0x48 */
    volatile reg_aria_hd_encoder_hd_encoder_status_t      hd_encoder_status  ; /* 0x4c */
    volatile reg_aria_hd_encoder_hdmi_h_fporchwidth_t      hdmi_h_fporchwidth   ; /* 0x50 */
    volatile reg_aria_hd_encoder_hdmi_h_syncwidth_t        hdmi_h_syncwidth   ; /* 0x54 */
    volatile reg_aria_hd_encoder_hdmi_h_bporchwidth_t     hdmi_h_bporchwidth ; /* 0x58 */
    volatile reg_aria_hd_encoder_hdmi_h_activewidth_t       hdmi_h_activewidth  ; /* 0x5c */
    volatile reg_aria_hd_encoder_hdmi_v_fporchheight_t    hdmi_v_fporchheight ; /* 0x60 */
    volatile reg_aria_hd_encoder_hdmi_v_syncheight_t       hdmi_v_syncheight ; /* 0x64 */
    volatile reg_aria_hd_encoder_hdmi_v_bporchheight_t    hdmi_v_bporchheight ; /* 0x68 */
    volatile reg_aria_hd_encoder_hdmi_v_activeheight_t    hdmi_v_activeheight  ; /* 0x6c */
    volatile reg_aria_hd_encoder_hdmi_h_halflinewidth_t    hdmi_h_halflinewidth ; /* 0x70 */
    volatile reg_aria_hd_encoder_hdmi_hsync_pola_t         hdmi_hsync_pola ; /* 0x74 */
    volatile reg_aria_hd_encoder_hdmi_vsync_pola_t       hdmi_vsync_pola ; /* 0x78 */
    volatile mt_u32 fld_debug[5];/*0x7c-0x8c*/
    volatile mt_u32 irq_status1; /* 0x90*/
    volatile reg_aria_hd_encoder_dac_sel_t               dac_channel_sel ; /* 0x94*/
    volatile mt_u32 hd_coeff_adj[16]; /* 0x98-0xd4*/
    volatile mt_u32 oversample_ctrl; /* 0xd8*/
    volatile mt_u32 sign_ctrl; /* 0xe0*/
    volatile mt_u32 pixel_vh_cnt; /* 0xe4*/
    volatile mt_u32 pixel_vh_state; /* 0xe8*/
    volatile mt_u32 vdac_adjust_y; /* 0xec*/
    volatile mt_u32 vdac_adjust_uv; /* 0xf0*/
    volatile mt_u32 irq_mask; /* 0xf4*/
    volatile mt_u32 hdmi_y_map; /* 0xf8*/
    volatile mt_u32 hdmi_uv_map; /* 0xfc*/
}aria_regs_hdenc_t;

#ifdef __cplusplus
extern "C" {
#endif

/*!
  register REGARIA_HD_ENCODER_basic_cfg (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_basic_cfg(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_basic_cfg(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_basic_cfg_resolusion(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_basic_cfg_resolusion(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_basic_cfg_interlace_mode(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_basic_cfg_interlace_mode(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_basic_cfg_start_num(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_basic_cfg_start_num(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_basic_cfg_start_num1(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_basic_cfg_start_num1(void);
void reg_aria_hd_encoder_set_basic_cfg_clk_auto_gate_en(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_basic_cfg_clk_auto_gate_en(void);

/*!
  register REGARIA_HD_ENCODER_y_parameter_level0 (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_y_parameter_level0(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_y_parameter_level0(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_y_parameter_level0_y_a_level(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_y_parameter_level0_y_a_level(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_y_parameter_level0_is_frame_packing(mt_u8 data);
/*!
 comments
  */
mt_u8  reg_aria_hd_encoder_get_y_parameter_level0_is_frame_packing(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_y_parameter_level0_de_mode(mt_u8 data);
/*!
 comments
  */
mt_u8  reg_aria_hd_encoder_get_y_parameter_level0_de_mode(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_y_parameter_level0_y_blank_level(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_y_parameter_level0_y_blank_level(void);
void reg_aira_hd_encoder_set_y_parameter_level0_cable_dct_en(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_y_parameter_level0_cable_dct_en(void);
void reg_aria_hd_encoder_set_y_parameter_level0_hdmi_lock(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_y_parameter_level0_hdmi_lock(void);
void reg_aria_hd_encoder_set_y_parameter_level0_hd_lock(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_y_parameter_level0_hd_lock(void);

/*!
  register REGARIA_HD_ENCODER_y_sync_level (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_y_sync_level(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_y_sync_level(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_y_sync_level_y_hsync_level(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_y_sync_level_y_hsync_level(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_y_sync_level_y_hi_sync_level(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_y_sync_level_y_hi_sync_level(void);

/*!
  register REGARIA_HD_ENCODER_pbpr_parameter0_level (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_pbpr_parameter0_level(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_pbpr_parameter0_level(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_pbpr_parameter0_level_uv_pbpr_center(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_pbpr_parameter0_level_uv_pbpr_center(void);

/*!
  register REGARIA_HD_ENCODER_pbpr_parameter1_level (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_pbpr_parameter1_level(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_pbpr_parameter1_level(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_pbpr_parameter1_level_uv_pbpr_b(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_pbpr_parameter1_level_uv_pbpr_b(void);

/*!
  register REGARIA_HD_ENCODER_hd_cfg_info (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hd_cfg_info(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_reg_field_mode(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_frame_rate(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_frame_rate(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_reg_hdtv_mode(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_reg_hdtv_mode(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_comp_mode_on(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_comp_mode_on(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_video_format(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_video_format(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_hi_sync_enable(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_hi_sync_enable(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_pbpr_data_offset_enable(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_pbpr_data_offset_enable(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_delay_y(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_delay_y(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_py_limit_enable(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_py_limit_enable(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_pbpr_limit_enablet(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_pbpr_limit_enablet(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_y_limit_enable(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_y_limit_enable(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_cbcr_limit_enable(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_cbcr_limit_enable(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_enable_1080p(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_enable_1080p(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_vsync_update_timing_enable(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_vsync_update_timing_enable(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_h_timing_update_start(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_h_timing_update_start(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hd_cfg_info_reg_disp_on(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_cfg_info_reg_disp_on(void);

/*!
  register REGARIA_HD_ENCODER_hsync_parameter0 (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hsync_parameter0(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hsync_parameter0(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hsync_parameter0_reg_h_sync_fporchwidth(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_hsync_parameter0_reg_h_sync_fporchwidth(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hsync_parameter0_reg_h_syncwidth(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_hsync_parameter0_reg_h_syncwidth(void);

/*!
  register REGARIA_HD_ENCODER_hsync_parameter1 (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hsync_parameter1(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hsync_parameter1(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hsync_parameter1_reg_h_sync_bporch(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_hsync_parameter1_reg_h_sync_bporch(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hsync_parameter1_reg_h_sync_activewidth(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_hsync_parameter1_reg_h_sync_activewidth(void);

/*!
  register REGARIA_HD_ENCODER_reg_H_active_parameter0 (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_h_active_parameter0(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_reg_h_active_parameter0(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_h_active_parameter0_reg_h_active_fporch(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_h_active_parameter0_reg_h_active_fporch(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_h_active_parameter0_reg_h_active_bporch(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_h_active_parameter0_reg_h_active_bporch(void);

/*!
  register REGARIA_HD_ENCODER_reg_H_active_parameter1 (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_h_active_parameter1(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_reg_h_active_parameter1(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_h_active_parameter1_reg_h_active_activewidth(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_h_active_parameter1_reg_h_active_activewidth(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_h_active_parameter1_reg_vsyncpixelcnt(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_h_active_parameter1_reg_vsyncpixelcnt(void);

/*!
  register REGARIA_HD_ENCODER_reg_V_parameter0 (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_v_parameter0(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_reg_v_parameter0(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_v_parameter0_reg_v_fporchheight(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_v_parameter0_reg_v_fporchheight(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_v_parameter0_reg_v_syncheight(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_v_parameter0_reg_v_syncheight(void);

/*!
  register REGARIA_HD_ENCODER_reg_V_parameter1 (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_v_parameter1(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_reg_v_parameter1(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_v_parameter1_reg_v_bporchheight(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_v_parameter1_reg_v_bporchheight(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_v_parameter1_reg_v_activeheight(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_v_parameter1_reg_v_activeheight(void);

/*!
  register REGARIA_HD_ENCODER_reg_H_OSD_parameter (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_h_osd_parameter(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_reg_h_osd_parameter(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_h_osd_parameter_reg_h_osd_start(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_h_osd_parameter_reg_h_osd_start(void);

#if 0
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_h_osd_parameter_reg_frame_rate(mt_u16 data);
/*!
 comments
  */
mt_u16 reg_aria_hd_encoder_get_reg_h_osd_parameter_reg_frame_rate(void);
#endif
/*!
  register REGARIA_HD_ENCODER_reg_V_OSD_parameter (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_v_osd_parameter(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_reg_v_osd_parameter(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_v_osd_parameter_reg_v_osd_start(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_v_osd_parameter_reg_v_osd_start(void);

/*!
  register REGARIA_HD_ENCODER_reg_H_Video_parameter (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_h_video_parameter(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_reg_h_video_parameter(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_h_video_parameter_reg_h_video_start(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_h_video_parameter_reg_h_video_start(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_h_video_parameter_reg_h_video_end(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_h_video_parameter_reg_h_video_end(void);

/*!
  register REGARIA_HD_ENCODER_reg_V_Video_parameter (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_v_video_parameter(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_reg_v_video_parameter(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_v_video_parameter_reg_v_video_start(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_v_video_parameter_reg_v_video_start(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_v_video_parameter_reg_v_video_end(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_reg_v_video_parameter_reg_v_video_end(void);

/*!
  register REGARIA_HD_ENCODER_reg_color_bar (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_color_bar(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_reg_color_bar(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_color_bar_cbar_width(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_reg_color_bar_cbar_width(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_reg_color_bar_cbar_enable(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_reg_color_bar_cbar_enable(void);

/*!
  register REGARIA_HD_ENCODER_cbar_set_data (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_cbar_set_data(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_cbar_set_data(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_cbar_set_data_y_cbar_set_data(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_cbar_set_data_y_cbar_set_data(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_cbar_set_data_pb_cbar_set_data(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_cbar_set_data_pb_cbar_set_data(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_cbar_set_data_pr_cbar_set_data(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_cbar_set_data_pr_cbar_set_data(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_cbar_set_data_cbar_set_enable(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_cbar_set_data_cbar_set_enable(void);

/*!
  register REGARIA_HD_ENCODER_gama_correction (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_gama_correction(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_gama_correction(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_gama_correction_gama_coef(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_gama_correction_gama_coef(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_gama_correction_gama_pole(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_gama_correction_gama_pole(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_gama_correction_gama_enable(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_gama_correction_gama_enable(void);

/*!
  register REGARIA_HD_ENCODER_hd_encoder_status (read)
  */
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hd_encoder_status(void);

void reg_aria_hd_encoder_set_hd_encoder_status(mt_u16 data);

/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_encoder_status_new_addr_flag(void);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_encoder_status_o_even_field(void);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_encoder_status_o_top_field(void);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hd_encoder_status_update_disp_h_timing(void);

/*!
  register REGARIA_HD_ENCODER_HDMI_H_FPorchWidth (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_h_fporchwidth(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hdmi_h_fporchwidth(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_h_fporchwidth_hdmi_h_fporchwidth(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_hdmi_h_fporchwidth_hdmi_h_fporchwidth(void);

/*!
  register REGARIA_HD_ENCODER_HDMI_H_SyncWidth (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_h_syncwidth(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hdmi_h_syncwidth(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_h_syncwidth_hdmi_h_syncwidth(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_hdmi_h_syncwidth_hdmi_h_syncwidth(void);

/*!
  register REGARIA_HD_ENCODER_HDMI_H_BPorchWidth (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_h_bporchwidth(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hdmi_h_bporchwidth(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_h_bporchwidth_hdmi_h_bporchwidth(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_hdmi_h_bporchwidth_hdmi_h_bporchwidth(void);

/*!
  register REGARIA_HD_ENCODER_HDMI_H_ActiveWidth (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_h_activewidth(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hdmi_h_activewidth(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_h_activewidth_hdmi_h_activewidth(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_hdmi_h_activewidth_hdmi_h_activewidth(void);

/*!
  register REGARIA_HD_ENCODER_HDMI_V_FPorchHeight (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_v_fporchheight(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hdmi_v_fporchheight(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_v_fporchheight_hdmi_v_fporchheight(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_hdmi_v_fporchheight_hdmi_v_fporchheight(void);

/*!
  register REGARIA_HD_ENCODER_HDMI_V_SyncHeight (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_v_syncheight(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hdmi_v_syncheight(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_v_syncheight_hdmi_v_syncheight(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_hdmi_v_syncheight_hdmi_v_syncheight(void);

/*!
  register REGARIA_HD_ENCODER_HDMI_V_BPorchHeight (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_v_bporchheight(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hdmi_v_bporchheight(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_v_bporchheight_hdmi_v_bporchheight(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_hdmi_v_bporchheight_hdmi_v_bporchheight(void);

/*!
  register REGARIA_HD_ENCODER_HDMI_V_ActiveHeight (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_v_activeheight(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hdmi_v_activeheight(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_v_activeheight_hdmi_v_activeheight(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_hdmi_v_activeheight_hdmi_v_activeheight(void);

/*!
  register REGARIA_HD_ENCODER_HDMI_H_HalfLineWidth (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_h_halflinewidth(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hdmi_h_halflinewidth(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_h_halflinewidth_hdmi_h_halflinewidth(mt_u16 data);
/*!
 comments
  */
mt_u16  reg_aria_hd_encoder_get_hdmi_h_halflinewidth_hdmi_h_halflinewidth(void);

/*!
  register REGARIA_HD_ENCODER_HDMI_HSync_Pola (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_hsync_pola(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hdmi_hsync_pola(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_hsync_pola_hdmi_hsync_pola(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hdmi_hsync_pola_hdmi_hsync_pola(void);

/*!
  register REGARIA_HD_ENCODER_HDMI_VSync_Pola (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_vsync_pola(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_hdmi_vsync_pola(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_hdmi_vsync_pola_hdmi_vsync_pola(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_hdmi_vsync_pola_hdmi_vsync_pola(void);

/*!
  register REGARIA_HD_ENCODER_DAC_SEL (read/write)
  */
/*!
 comments
  */
void reg_aria_hd_encoder_set_dac_sel(mt_u32 data);
/*!
 comments
  */
mt_u32  reg_aria_hd_encoder_get_dac_sel(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_dac_sel_dac_3_mode(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_3_mode(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_dac_sel_dac_2_mode(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_2_mode(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_dac_sel_dac_1_mode(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_1_mode(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_dac_sel_dac_0_mode(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_0_mode(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_dac_sel_dac_3_hdsd_sel(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_3_hdsd_sel(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_dac_sel_dac_2_hdsd_sel(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_2_hdsd_sel(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_dac_sel_dac_1_hdsd_sel(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_1_hdsd_sel(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_dac_sel_dac_0_hdsd_sel(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_dac_sel_dac_0_hdsd_sel(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_dac_sel_hdmi_mode(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_dac_sel_hdmi_mode(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_dac_sel_cbcr_order(mt_u8 data);
/*!
 comments
  */
mt_u8   reg_aria_hd_encoder_get_dac_sel_cbcr_order(void);

/*!
  ARIA_HD_Encoder reg init function
  */
void reg_aria_hd_encoder_init(void);
/*!
 comments
  */
void reg_aria_hd_encoder_set_oversample(mt_u32 data);

/*!
  register REGARIA_HD_ENCODER_OS_FILTER_CTRL (read/write)
  */
void reg_aria_hd_encoder_set_os_filter_ctrl(mt_u32 data);
mt_u32  reg_aria_hd_encoder_get_os_filter_ctrl(void);
void reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_4_3(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_4_3(void);
void reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_6_5(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_6_5(void);
void reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_7(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_7(void);
void reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_8(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_8(void);
void reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_0(void);
void reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_mux_sdp_2_1(void);
void reg_aria_hd_encoder_set_os_filter_ctrl_os_en(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_os_en(void);
void reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_sample_ctrl(void);
void reg_aria_hd_encoder_set_os_filter_ctrl_sync_filter_en(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_sync_filter_en(void);
void reg_aria_hd_encoder_set_os_filter_ctrl_int_state_rd_clr_en(mt_u8 data);
mt_u8   reg_aria_hd_encoder_get_os_filter_ctrl_int_state_rd_clr_en(void);
void reg_aria_hd_encoder_set_sign_ctrl(u32 data);
mt_u32  reg_aria_hd_encoder_get_sign_ctrl(void);
void reg_aria_hd_encoder_set_y_parameter_level0_cable_dct_en(mt_u8 data);
void reg_aria_hd_encoder_set_mask_irq(mt_u32 data);
mt_u32  reg_aria_hd_encoder_get_mask_irq(void);

/*!
 comments
  */
void reg_aria_hd_encoder_set_vdac_adjusting_y(mt_u32 data);
/*!
 comments
  */
void reg_aria_hd_encoder_set_vdac_adjusting_uv(mt_u32 data);
#ifdef __cplusplus
}
#endif

#endif

