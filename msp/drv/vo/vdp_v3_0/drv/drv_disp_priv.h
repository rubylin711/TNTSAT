
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_priv.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_DISP_PRIV_H__
#define __DRV_DISP_PRIV_H__

#include "mt_type.h"
#include "mt_drv_video.h"
#include "mt_drv_disp.h"
#include "drv_disp_hal.h"
#include "drv_disp_Symphony_reg.h"

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
#ifdef CONFIG_MT_VO_SL_HDR
#include "SL_api.h"
#endif
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
#include "../../hdmi20/drv_hdmi.h"
#else
#include "../../hdmi/mta_hdmi_hdmi.h"
#endif

// if HDR function code is ready, enable this Macro to enable the HDR function
// HDR function only used in sym2 and sym4 chip, sym1 chip is not supported.

#if defined(CONFIG_MT_CHIP_SYMPHONY4)  || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
#define SUPPORT_HDR
#else
#undef SUPPORT_HDR
#endif



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


typedef struct
{
  u32 transfer_characteristics;  //ITU-T H.265 Table E.4
  u32 colour_primaries;  //ITU-T H.265 Table E.3
#ifdef SUPPORT_HDR
  //metadata for PQ10
  fw_mastering_disp_colour_volume_t colour_volume;
  fw_content_light_level_info_t light_level;
#endif
}PicHdrInfo_t;

typedef struct
{
  u32 transparent_mode;
  u32 hdr_display_Brightness;
  u32 sdr_display_Brightness;  //for vivid
  u32 target_display_adaptation_tuning_level;
  u32 display_OETF;
}sl_hdr_para_t;

typedef struct
{
    /*!
    All the layer info.
    */
    layer_info_t layer_info[DISP_LAYER_ID_MAX];
    /*!
    The display input info. Only one input
    */
    disp_info_t disp_in_info;
    /*!
    The display output info. Two outputs
    */
    disp_info_t disp_out_info[DISP_CHANNEL_MAX];
    /*!
    The rate convert info.
    */
    rc_info_t rc_info[DISP_CHANNEL_MAX];
    /*!
    The video encoder info.
    */
    venc_info_t venc_info;
    /*!
    The deinterlace info.
    */
    di_info_t di_info;
#if 0
  /*!
    The coeff tables.
    */
    coeff_t coeff_table[SCALE_COEFF_TABLE_MAX];
#endif
    /*!
    The coeff tables.
    */
    disp_coeff_s coeff_table[DISP_SCALE_COEFF_TABLE_MAX];
    /*!
     the coeff tables be loaded
    */
    MT_BOOL b_table_load;
    /*!
    The chip version
    */
    //chip_rev_t chip_rev;
    /*!
      The chip version
      */
    /*!
      The osd horizontal scale enable
      */
    MT_BOOL b_osd_hscale;
    /*!
      The osd vertical scale enable
      */
    MT_BOOL b_osd_vscale;
    /*!
      hdmi video cfg
      */
    hdmi_video_config_t hdmi_vcfg;
    /*!
      hdmi video cfg flag, true need to cfg, false do not need
      */
    MT_BOOL hdmi_vcfg_flag;
    /*!
      display pp mode
      */
    disp_pp_mode_t pp_mode;
    /*!
      The graphic size changed
      */
    MT_BOOL b_gra_size_change;
    /*!
      The graphic vertical size changed
      */
    MT_BOOL b_gra_height_change;
    /*!
    The prescale off to on force turn off video
    */
    MT_BOOL b_prescale_on_force_vid_off;
    /*!
      The prescale off to on delay show cnt
      */
    mt_u32 prescale_on_delay_cnt;
    /*!
      The prescale on to off force turn off video
      */
    MT_BOOL b_prescale_off_force_vid_off;
    /*!
      The prescale on to off delay show cnt
      */
    mt_u32 prescale_off_delay_cnt;
    /*!
      csc info
      */
    csc_info_t csc_info;
    /*!
      osd/still default buf
      */
    void *p_default_layerbuf;
    /*!
    pre scale flag
    */
    MT_BOOL b_unuse_prescale;
    /*!
      half scale flag
      */
    MT_BOOL b_use_halfscale;

    /*!
    denosie info
    */
    denoise_info_t denoise_info;

    /*!
       shared memory used by av and ap cpu
    */
    mt_u32 av_ap_shared_mem;
    /*!
      shared memory size
    */
    mt_u32 shared_mem_size;
    /*!
    for font text scroll
    */
    scroll_info_t scroll[MAX_SCROLL_NUM];
    /*!
    tasklet for display
    */
    struct tasklet_struct tasklet;
    /*!
    flag for tasklet
    */
    MT_BOOL tasklet_flag;
    MT_BOOL b_uninit;
    vdac_type_t dacMode;
    MT_BOOL usr_force_3d;
    MT_BOOL cur_3d_flag;
    MT_DRV_DISP_STEREO_E cur_mode_3d;
    hdmi_3d_ext_data_t cur_extpara_3d;
    MT_BOOL old_3d_flag;
    MT_DRV_DISP_STEREO_E old_mode_3d;
    hdmi_3d_ext_data_t old_extpara_3d;
    MT_BOOL b_wrback_422;

    struct work_struct work_hdmi_cb;
    struct work_struct work_hdmi_preformat;
    struct work_struct work_hdmi_setformat;
    /*!
      hdmi info
      */
    hdmi_info_t hdmi_info;

    /*!
      vdac0 plug in/out status
      */
    MT_BOOL vdac0_in;
    /*!
      vdac3 plug in/out status
      */
    MT_BOOL vdac3_in;
    
    MT_BOOL clock_1001_enable;

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
    sl_hdr_para_t hdr_para;
#endif
  PicHdrInfo_t hdr_info;
} disp_priv_t;

#define DISP_DEVICE_STATE_CLOSE 0
#define DISP_DEVICE_STATE_OPEN 1
#define DISP_DEVICE_STATE_SUSPEND 2

#define DISP_SET_TIMEOUT_THRESHOLD 10

/* default back ground color */
#define DISP_DEFAULT_COLOR_RED 0
#define DISP_DEFAULT_COLOR_GREEN 0
#define DISP_DEFAULT_COLOR_BLUE 0

typedef enum tagDISP_PRIV_STATE_E {
    DISP_PRIV_STATE_DISABLE = 0,
    DISP_PRIV_STATE_WILL_ENABLE,
    DISP_PRIV_STATE_ENABLE,
    DISP_PRIV_STATE_WILL_DISABLE,
    DISP_PRIV_STATE_BUTT
} DISP_PRIV_STATE_E;

typedef struct tagDISP_SETTING_S
{
    mt_u32 u32Version;
    mt_u32 u32BootVersion;
    //MT_BOOL bSelfStart;
    MT_BOOL bGetPDMParam;

    /* output format */
    MT_DRV_DISP_STEREO_E eDispMode;
    MT_BOOL bRightEyeFirst;
    MT_DRV_DISP_FMT_E enFormat;
    MT_BOOL bFmtChanged;

    MT_DRV_DISP_TIMING_S stCustomTimg;

    /* about color */
    MT_DRV_DISP_COLOR_SETTING_S stColor;

    /* background color */
    MT_DRV_DISP_COLOR_S stBgColor;

    //MT_BOOL bCGMSAEnable;
    //MT_DRV_DISP_CGMSA_TYPE_E  eCGMSAType;
    //MT_DRV_DISP_CGMSA_MODE_E  eCGMSAMode;

    //MT_DRV_DISP_MACROVISION_E eMcvnType;

    /* interface setting */
    mt_u32 u32IntfNumber;
    //MT_DRV_DISP_INTF_S stIntf[MT_DRV_DISP_INTF_ID_MAX];
    DISP_INTF_S stIntf[MT_DRV_DISP_INTF_ID_MAX];

    mt_u32 u32LayerNumber;
    MT_DRV_DISP_LAYER_E enLayer[MT_DRV_DISP_LAYER_BUTT]; /* Z-order is from bottom to top */

    /*we define a vitual format size, all the size settings  users can see are referenced to this rect.
     *      * so users can make a fixed setting not according to the real format size such as 1280*720 50hz.
     *           * it's manual-kindly. When setting to devices, we make a transfer according the real resolution.
     *                */
    mt_rect_s stVirtaulScreen;

    /*as a result of cutting off by crt tv, we make a offset setting to make sure
     *      * that the display is complete, not cut by tv.*/
    MT_DRV_DISP_OFFSET_S stOffsetInfo;

    MT_BOOL bCustomRatio;
    mt_u32 u32CustomRatioWidth;
    mt_u32 u32CustomRatioHeight;

    mt_u32 u32Reseve;
    mt_void *pRevData;
} DISP_SETTING_S;

typedef struct tagDISP_S
{
    MT_DRV_DISPLAY_E enDisp;

    //state
    MT_BOOL bBaseExist;
    MT_BOOL bOpen;
    MT_BOOL bEnable;
    MT_BOOL bStateBackup;

    /* for attach display */
    MT_BOOL bIsMaster;
    MT_BOOL bIsSlave;
    MT_DRV_DISPLAY_E enAttachedDisp;

    DISP_SETTING_S stSetting;
    MT_BOOL bDispSettingChange;

    volatile DISP_PRIV_STATE_E eState;
    mt_u32 u32Underflow;
    mt_u32 u32StartTime;

    // for other module get
    //MT_BOOL bDispInfoValid;
    MT_DISP_DISPLAY_INFO_S stDispInfo;

    //mirrorcast
    mt_handle hCast;
    mt_handle Cast_ptr;

    //algrithm operation
    //mt_handle hAlgOpt;

    //component operation
    DISP_INTF_OPERATION_S *pstIntfOpt;
    disp_priv_t *p_dp;
    mt_u32 screen_width;
    mt_u32 screen_height;
    mt_u32 aspect;
    mt_u8 percent;
    mt_u32 pole;
    mt_u32 color;
    mt_u32 alpha;
    mt_u32 v_aspect;
    mt_u32 ar_mode;
    mt_u32 p_align;
    mt_u32 grp_id;
    mt_u32 layer;
    mt_u32 b_on;
    mt_u32 ch;
    mt_u8 coef;
    mt_u32 p_size;
    mt_u32 type;
    rect_vsb_t cur_rect;
    rect_vsb_t old_rect;
    void *p_region;
    void *p_buf;
    pos_t p_pos;
    DISP_BUF disp_buf;
    mt_u32 update_flag;
    mt_u32 vid_fmt;
    vdac_type_t dacMode;

    cvbs_dac_t cvbs_dac[CVBS_GRP_MAX];
    comp_dac_t comp_dac[COMPONENT_GRP_MAX];
    svideo_dac_t svideo_dac[SVIDEO_GRP_MAX];

	MT_DRV_DISP_MACROVISION_E enMacrovisionMode;

} DISP_S;

typedef struct tagDISP_ATTACH_ID_S
{
    MT_DRV_DISPLAY_E eMaster;
    MT_DRV_DISPLAY_E eSlave;
} DISP_ATTACH_ID_S;

typedef struct tagDISP_DEV_S
{
    MT_BOOL bHwReseted;
    DISP_S stDisp[MT_DRV_DISPLAY_BUTT + 1];

    MT_BOOL bAttachEnable;
    DISP_ATTACH_ID_S stAttchDisp;
    DISP_INTF_OPERATION_S stIntfOpt;
} DISP_DEV_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*  __DRV_DISP_PRIV_H__  */
