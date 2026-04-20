/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************
  File Name     : mt_drv_disp.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2015/12/25
Description   :
History       :
1.Date        : 2015/12/25
Author      : 
Modification: 

 *********************************************************************************************/
#ifndef __MT_DRV_DISP_H__
#define __MT_DRV_DISP_H__

#include "mt_type.h"
#include "mt_common.h"
#include "mt_debug.h"
#include "mt_drv_video.h"
#include "drv_pq_define.h"
#include "drv_pq_ext.h"
#include "mt_unf_hdmi.h"

extern int disp_dbg_enable;
//#define DEBUG_OPEN
#ifdef DEBUG_OPEN 
#define DISP_DEBUGK printk
#define WIN_DEBUGK printk
#define MTFB_DEBUGK printk

#define DISP_DEBUGF printf
#define WIN_DEBUGF printf

#else
#define DISP_DEBUGF(fmt,...)
#define WIN_DEBUGF(fmt,...)

#define DISP_DEBUGK(fmt, ...) \
	do { \
		if (disp_dbg_enable) { \
			printk(KERN_CRIT   fmt, ## __VA_ARGS__); \
		} \
	} while (0)
#define WIN_DEBUGK(fmt, ...) \
	do { \
		if (disp_dbg_enable) { \
			printk(KERN_CRIT  fmt, ## __VA_ARGS__); \
		} \
	} while (0)
#define MTFB_DEBUGK(fmt, ...) \
	do { \
		if (disp_dbg_enable) { \
			printk(KERN_CRIT  fmt, ## __VA_ARGS__); \
		} \
	} while (0)
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif


/*!
  The width of CIF video layer  (in pixels).
  */
#define DISP_VID_FULLSCR_CIF_WIDTH            (352)
/*!
  The height of full screen for CIF in PAL standard (in pixels).
  */
#define DISP_VID_FULLSCR_CIF_HEIGHT       (288)
/*!
  The width of full screen for SD video layer  (in pixels).
  */
#define DISP_VID_FULLSCR_SD_WIDTH            (720)
/*!
  The height of full screen for video layer in PAL standard (in pixels).
  */
#define DISP_VID_FULLSCR_PAL_HEIGHT       (576)
/*!
  The height of half full screen for video layer in PAL standard (in pixels).
  */
#define DISP_VID_FULLSCR_PAL_HEIGHT_HALF       (288)
/*!
  The height of full screen for video layer in NTSC standard (in pixels).
  */
#define DISP_VID_FULLSCR_NTSC_HEIGHT      (480)
/*!
  The height of half full screen for video layer in NTSC standard (in pixels).
  */
#define DISP_VID_FULLSCR_NTSC_HEIGHT_HALF      (240)
/*!
  The width of full screen for 720P video layer  (in pixels).
  */
#define DISP_VID_FULLSCR_720P_WIDTH            (1280)
/*!
  The height of full screen for video layer in 720P(in pixels).
  */
#define DISP_VID_FULLSCR_720P_HEIGHT      (720)
/*!
  The width of full screen for 1080I/P video layer  (in pixels).
  */
#define DISP_VID_FULLSCR_1080_WIDTH            (1920)
/*!
  The height of full screen for video layer in 1080I/P(in pixels).
  */
#define DISP_VID_FULLSCR_1080_HEIGHT      (1080)
/*!
  The height of full screen for video layer in 1080I/P(in pixels).
  */
#define DISP_VID_FULLSCR_1080_HEIGHT_1088      (1088)

#define DISP_VID_FULLSCR_3840_WIDTH            (3840)
#define DISP_VID_FULLSCR_4096_WIDTH            (4096)
#define DISP_VID_FULLSCR_2160_HEIGHT            (2160)
#define UPDATE_FLAG_TVSYS       0x0001
/*!
  Zoom flag.
  */
#define UPDATE_FLAG_ZOOM       0x0002
/*!
  Rate convertion flag.
  */
#define UPDATE_FLAG_RATE_COV    0x0004
/*!
  Plane Alpha flag.
  */
#define UPDATE_FLAG_ALPHA       0x0008
/*!
  Colorkey flag.
  */
#define UPDATE_FLAG_COLORKEY    0x0010
/*!
  Show flag.
  */
#define UPDATE_FLAG_ONOFF       0x0020
/*!
  Region flag.Add, Delete, Move
  */
#define UPDATE_FLAG_REGION      0x0040
/*!
  Aspect ration flag.
  */
#define UPDATE_FLAG_AR          0x0080
/*!
  Zoom exit flag.
  */
#define UPDATE_FLAG_ZOOM_EXIT       0x0100

#define MAX_WIN_NUM 2


/* display ID */
typedef enum mtDRV_DISPLAY_E
{
    MT_DRV_DISPLAY_0 = 0, //seemed as sd
    MT_DRV_DISPLAY_1,  //seemed as hd
    MT_DRV_DISPLAY_2,
    MT_DRV_DISPLAY_BUTT
}MT_DRV_DISPLAY_E;

typedef enum mtDRV_DISP_LAYER_E
{
    MT_DRV_DISP_LAYER_NONE = 0,
    MT_DRV_DISP_LAYER_VIDEO,
    MT_DRV_DISP_LAYER_GFX,
    MT_DRV_DISP_LAYER_BUTT
}MT_DRV_DISP_LAYER_E;


typedef enum mtDRV_DISP_STEREO_MODE_E
{
    MT_DRV_DISP_STEREO_NONE = 0, /* 2D Mode */
    MT_DRV_DISP_STEREO_FRAME_PACKING,
    MT_DRV_DISP_STEREO_SBS_HALF,
    MT_DRV_DISP_STEREO_TAB,
    MT_DRV_DISP_STEREO_FIELD_ALT,
    MT_DRV_DISP_STEREO_LINE_ALT,
    MT_DRV_DISP_STEREO_SBS_FULL,
    MT_DRV_DISP_STEREO_L_DEPTH,
    MT_DRV_DISP_STEREO_L_DEPTH_G_DEPTH,
    MT_DRV_DISP_STEREO_MODE_BUTT
}MT_DRV_DISP_STEREO_MODE_E;

typedef enum mtDRV_DISP_ZORDER_E
{
    MT_DRV_DISP_ZORDER_MOVETOP = 0,
    MT_DRV_DISP_ZORDER_MOVEUP,
    MT_DRV_DISP_ZORDER_MOVEBOTTOM,
    MT_DRV_DISP_ZORDER_MOVEDOWN,
    MT_DRV_DISP_ZORDER_BUTT
}MT_DRV_DISP_ZORDER_E;

typedef enum mtDRV_DISP_ZORDER_ABS_E
{
    MT_DRV_DISP_ZORDER_ABS_OSD0_OSD1_SUB= 0,
    MT_DRV_DISP_ZORDER_ABS_OSD1_OSD0_SUB ,
    MT_DRV_DISP_ZORDER_ABS_SUB_OSD1_OSD0 ,
    MT_DRV_DISP_ZORDER_ABS_OSD1_SUB_OSD0 ,
    MT_DRV_DISP_ZORDER_ABS_SUB_OSD0_OSD1 ,
    MT_DRV_DISP_ZORDER_ABS_OSD0_SUB_OSD1 ,
    MT_DRV_DISP_ZORDER_ABS_BUTT
}MT_DRV_DISP_ZORDER_ABS_E;

typedef enum mtDRV_DISP_FMT_E
{
    MT_DRV_DISP_FMT_1080P_60 = 0,     /**<1080p 60 Hz*/
    MT_DRV_DISP_FMT_1080P_59_94,  /**<1080p 59.94 Hz*/
    MT_DRV_DISP_FMT_1080P_50,         /**<1080p 50 Hz*/
    MT_DRV_DISP_FMT_1080P_30,         /**<1080p 30 Hz*/
    MT_DRV_DISP_FMT_1080P_29_97,      /**<1080p 29.97 Hz*/
    MT_DRV_DISP_FMT_1080P_25,         /**<1080p 25 Hz*/
    MT_DRV_DISP_FMT_1080P_24,         /**<1080p 24 Hz*/

    MT_DRV_DISP_FMT_1080i_60,         /**<1080i 60 Hz*/
    MT_DRV_DISP_FMT_1080i_59_94,      /**<1080i 59.94 Hz*/
    MT_DRV_DISP_FMT_1080i_50,         /**<1080i 60 Hz*/

    MT_DRV_DISP_FMT_720P_60,          /**<720p 60 Hz*/
    MT_DRV_DISP_FMT_720P_59_94,       /**<720p 59.94 Hz*/
    MT_DRV_DISP_FMT_720P_50,          /**<720p 50 Hz */

    MT_DRV_DISP_FMT_576P_50,          /**<576p 50 Hz*/
    MT_DRV_DISP_FMT_480P_60,          /**<480p 60 Hz*/

    MT_DRV_DISP_FMT_PAL,              /* B D G H I PAL */
    MT_DRV_DISP_FMT_PAL_B,            /* B PAL， Australia */
    MT_DRV_DISP_FMT_PAL_B1,           /* B1 PAL, Hungary */
    MT_DRV_DISP_FMT_PAL_D,            /* D PAL, China */
    MT_DRV_DISP_FMT_PAL_D1,           /* D1 PAL， Poland */
    MT_DRV_DISP_FMT_PAL_G,            /* G PAL， Europe */
    MT_DRV_DISP_FMT_PAL_H,            /* H PAL， Europe */
    MT_DRV_DISP_FMT_PAL_K,            /* K PAL， Europe */
    MT_DRV_DISP_FMT_PAL_I,            /* I PAL，U.K. */
    MT_DRV_DISP_FMT_PAL_N,            /* N PAL, Jamaica/Uruguay */
    MT_DRV_DISP_FMT_PAL_Nc,           /* Nc PAL, Argentina:21 */

    MT_DRV_DISP_FMT_PAL_M,            /* M PAL, 525 lines */
    MT_DRV_DISP_FMT_PAL_60,           /* 60 PAL */
    MT_DRV_DISP_FMT_NTSC,             /* (M)NTSC       */
    MT_DRV_DISP_FMT_NTSC_J,           /* NTSC-J        */
    MT_DRV_DISP_FMT_NTSC_443,          /* (M)PAL        */

    MT_DRV_DISP_FMT_SECAM_SIN,      /**< SECAM_SIN*/
    MT_DRV_DISP_FMT_SECAM_COS,      /**< SECAM_COS*/
    MT_DRV_DISP_FMT_SECAM_L,        /**< France*/
    MT_DRV_DISP_FMT_SECAM_B,        /**< Middle East*/
    MT_DRV_DISP_FMT_SECAM_G,        /**< Middle East*/
    MT_DRV_DISP_FMT_SECAM_D,        /**< Eastern Europe*/
    MT_DRV_DISP_FMT_SECAM_K,        /**< Eastern Europe*/
    MT_DRV_DISP_FMT_SECAM_H,        /**< Line SECAM:34*/

    MT_DRV_DISP_FMT_1440x576i_50,
    MT_DRV_DISP_FMT_1440x480i_60, /*sequnce:36*/

    MT_DRV_DISP_FMT_1080P_24_FP,
    MT_DRV_DISP_FMT_720P_60_FP,
    MT_DRV_DISP_FMT_720P_50_FP,

    MT_DRV_DISP_FMT_861D_640X480_60,
    MT_DRV_DISP_FMT_VESA_800X600_60,
    MT_DRV_DISP_FMT_VESA_1024X768_60,
    MT_DRV_DISP_FMT_VESA_1280X720_60,
    MT_DRV_DISP_FMT_VESA_1280X800_60,
    MT_DRV_DISP_FMT_VESA_1280X1024_60,
    MT_DRV_DISP_FMT_VESA_1360X768_60,         //Rowe
    MT_DRV_DISP_FMT_VESA_1366X768_60,
    MT_DRV_DISP_FMT_VESA_1400X1050_60,        //Rowe
    MT_DRV_DISP_FMT_VESA_1440X900_60,
    MT_DRV_DISP_FMT_VESA_1440X900_60_RB,
    MT_DRV_DISP_FMT_VESA_1600X900_60_RB,
    MT_DRV_DISP_FMT_VESA_1600X1200_60,
    MT_DRV_DISP_FMT_VESA_1680X1050_60,       //Rowe
    MT_DRV_DISP_FMT_VESA_1680X1050_60_RB,       //Rowe
    MT_DRV_DISP_FMT_VESA_1920X1080_60,
    MT_DRV_DISP_FMT_VESA_1920X1200_60,
    MT_DRV_DISP_FMT_VESA_1920X1440_60,
    MT_DRV_DISP_FMT_VESA_2048X1152_60,
    MT_DRV_DISP_FMT_VESA_2560X1440_60_RB,
    MT_DRV_DISP_FMT_VESA_2560X1600_60_RB, /*sequence:60*/

    MT_DRV_DISP_FMT_3840X2160_24,/*reserve for extention*/
    MT_DRV_DISP_FMT_3840X2160_25,
    MT_DRV_DISP_FMT_3840X2160_29_97,
    MT_DRV_DISP_FMT_3840X2160_30,
    MT_DRV_DISP_FMT_3840X2160_50,
    MT_DRV_DISP_FMT_3840X2160_59_94,
    MT_DRV_DISP_FMT_3840X2160_60,
    MT_DRV_DISP_FMT_4096X2160_24,
    MT_DRV_DISP_FMT_4096X2160_25,
    MT_DRV_DISP_FMT_4096X2160_29_97,
    MT_DRV_DISP_FMT_4096X2160_30,
    MT_DRV_DISP_FMT_4096X2160_50,
    MT_DRV_DISP_FMT_4096X2160_59_94,
    MT_DRV_DISP_FMT_4096X2160_60,

    //ADD
    MT_DRV_DISP_FMT_CUSTOM,
    MT_DRV_DISP_FMT_BUTT
}MT_DRV_DISP_FMT_E;

typedef enum mtDRV_DISP_VDAC_SIGNAL_E
{
    MT_DRV_DISP_VDAC_NONE = 0,
    MT_DRV_DISP_VDAC_CVBS,
    MT_DRV_DISP_VDAC_Y,
    MT_DRV_DISP_VDAC_PB,
    MT_DRV_DISP_VDAC_PR,
    MT_DRV_DISP_VDAC_SV_Y,
    MT_DRV_DISP_VDAC_SV_C,
    MT_DRV_DISP_VDAC_R,
    MT_DRV_DISP_VDAC_G,
    MT_DRV_DISP_VDAC_B,
    MT_DRV_DISP_VDAC_G_NOSYNC,
    MT_DRV_DISP_VDAC_SIGNAL_BUTT
}MT_DRV_DISP_VDAC_SIGNAL_E;

typedef enum mtDRV_DISP_INTF_ID_E
{
    MT_DRV_DISP_INTF_YPBPR0 = 0,
    MT_DRV_DISP_INTF_RGB0,
    MT_DRV_DISP_INTF_SVIDEO0,
    MT_DRV_DISP_INTF_CVBS0,
    MT_DRV_DISP_INTF_VGA0,

    MT_DRV_DISP_INTF_HDMI0,
    MT_DRV_DISP_INTF_HDMI1,
    MT_DRV_DISP_INTF_HDMI2,

    MT_DRV_DISP_INTF_BT656_0,
    MT_DRV_DISP_INTF_BT656_1,
    MT_DRV_DISP_INTF_BT656_2,

    MT_DRV_DISP_INTF_BT1120_0,
    MT_DRV_DISP_INTF_BT1120_1,
    MT_DRV_DISP_INTF_BT1120_2,

    MT_DRV_DISP_INTF_LCD0,
    MT_DRV_DISP_INTF_LCD1,
    MT_DRV_DISP_INTF_LCD2,

    MT_DRV_DISP_INTF_ID_MAX
}MT_DRV_DISP_INTF_ID_E;

typedef enum
{
    /*!
      background layer
      */
    DISP_LAYER_ID_BACKGROUND = 0,
    /*!
      still layer SD 1
      */
    DISP_LAYER_ID_STILL_SD,  
    /*!
      still layer HD 2
      */
    DISP_LAYER_ID_STILL_HD,  
    /*!
      video layer SD 3
      */
    DISP_LAYER_ID_VIDEO_SD,
    /*!
      video layer HD 4
      */
    DISP_LAYER_ID_VIDEO_HD,
    /*!
      osd layer 0  5
      */
    DISP_LAYER_ID_OSD0,
    /*!
      osd layer 1  6
      */
    DISP_LAYER_ID_OSD1,
    /*!
      sub layer 7
      */
    DISP_LAYER_ID_SUBTITL,
    /*!
      max 
      */
    DISP_LAYER_ID_MAX
} MT_DRV_DISP_LAYER_ID_E;

/*!
  The indext video DACs
  */
typedef enum
{
    /*!
      Video DAC 0
      */
    DAC_0 = 0,
    /*!
      Video DAC 1
      */
    DAC_1,
    /*!
      Video DAC 2
      */
    DAC_2,
    /*!
      Video DAC 3
      */
    DAC_3
}dac_index_t;

/*!
  This structure defines the supported output formats of video DAC.
  */
typedef enum
{
    /*!
      Red component
      */
    DAC_R = 0,
    /*!
      Green component
      */
    DAC_G,
    /*!
      Blue component
      */
    DAC_B,
    /*!
      Y component
      */
    DAC_Y,
    /*!
      U component
      */
    DAC_U,
    /*!
      V component
      */
    DAC_V,
    /*!
      Composite video
      */
    DRV_DAC_CVBS,
    /*!
      Luminance component
      */
    DAC_LUMA,
    /*!
      Chrominace componet
      */
    DAC_CHROMA
}dac_fmt_t;


/*!
  Define cvbs dac group
  */
typedef enum
{
    /*!
      CVBS Group 0 use dac 0
      */
    CVBS_GRP0 = 0,
    /*!
      CVBS Group 1 use dac 1
      */
    CVBS_GRP1,
    /*!
      CVBS Group 2 use dac 2
      */
    CVBS_GRP2,
    /*!
      CVBS Group 3 use dac 3
      */
    CVBS_GRP3,
    /*!
      CVBS Group MAX
      */
    CVBS_GRP_MAX
}cvbs_dacgrp_t;

/*!
  Define component dac group
  */
typedef enum
{
    /*!
      Component Group 0 SD use dac 1,2,3, yuv,gbr
      */
    COMPONENT_GRP0 = 0,
    /*!
      Component Group 1 HD use dac 1,2,3,yuv
      */
    COMPONENT_GRP1,
    /*!
      Component Group 2 HD use dac 1,0,2,yuv, fpga
      */
    COMPONENT_GRP2,
    /*!
      Component Group 3 HD use dac 1,3,2,yuv
      */
    COMPONENT_GRP3,
    /*!
      Component Group 4 HD use dac 1,2,0,yuv
      */
    COMPONENT_GRP4,
    /*!
      Component Group 5 HD use dac 1,0,3,yuv
      */
    COMPONENT_GRP5,
    /*!
      Component Group 6 HD use dac 1,3,0,yuv
      */
    COMPONENT_GRP6,
    /*!
      Component Group 7 HD use dac 0,1,2,yuv
      */
    COMPONENT_GRP7,
    /*!
      Component Group 8 HD use dac 0,1,3,yuv
      */
    COMPONENT_GRP8,
    /*!
      Component Group 9 HD use dac 0,2,1,yuv
      */
    COMPONENT_GRP9,
    /*!
      Component Group 10 HD use dac 0,2,3,yuv
      */
    COMPONENT_GRP10,
    /*!
      Component Group 11 HD use dac 0,3,1,yuv
      */
    COMPONENT_GRP11,
    /*!
      Component Group 12 HD use dac 0,3,2,yuv
      */
    COMPONENT_GRP12,
    /*!
      Component Group 13 HD use dac 2,0,1,yuv
      */
    COMPONENT_GRP13,
    /*!
      Component Group 14 HD use dac 2,0,3,yuv
      */
    COMPONENT_GRP14, 
    /*!
      Component Group 15 HD use dac 2,1,0,yuv
      */
    COMPONENT_GRP15,
    /*!
      Component Group 16 HD use dac 2,1,3,yuv
      */
    COMPONENT_GRP16,
    /*!
      Component Group 17 HD use dac 2,3,0,yuv
      */
    COMPONENT_GRP17,
    /*!
      Component Group 18 HD use dac 2,3,1,yuv
      */
    COMPONENT_GRP18,
    /*!
      Component Group 19 HD use dac 3,0,1,yuv
      */
    COMPONENT_GRP19,
    /*!
      Component Group 20 HD use dac 3,0,2,yuv
      */
    COMPONENT_GRP20,
    /*!
      Component Group 21 HD use dac 3,1,0,yuv
      */
    COMPONENT_GRP21,
    /*!
      Component Group 22 HD use dac 3,1,2,yuv
      */
    COMPONENT_GRP22,
    /*!
      Component Group 23 HD use dac 3,2,0,yuv
      */
    COMPONENT_GRP23,
    /*!
      Component Group 24 HD use dac 3,2,1,yuv
      */
    COMPONENT_GRP24,
    /*!
      Component Group 25 SD use dac 1,0,2, yuv,gbr
      */
    COMPONENT_GRP25,    
    /*!
      Component Group MAX 
      */
    COMPONENT_GRP_MAX
}component_dacgrp_t;

/*!
  Define Svideo dac group
  */
typedef enum
{
    /*!
      Svideo Group 0 SD use dac 2,3
      */
    SVIDEO_GRP0 = 0,
    /*!
      Svideo Group 1
      */
    SVIDEO_GRP1,
    /*!
      Svideo Group 2
      */
    SVIDEO_GRP2,
    /*!
      Svideo Group 3
      */
    SVIDEO_GRP3,
    /*!
      Svideo Group MAX
      */
    SVIDEO_GRP_MAX
}svideo_dacgrp_t;

/*!
  This structure defines the colorspace
  */
typedef enum
{
    /*!
      rgb
      */
    COLOR_RGB,
    /*!
      yuv
      */
    COLOR_YUV,
    /*!
      jazz: cvbs+s-video
      */  
    COLOR_SVID_CVBS,
    /*!
      jazz: cvbs+YUV
      */  
    COLOR_CVBS_YUV,
    /*!
      jazz: cvbs+RGB
      */  
    COLOR_CVBS_RGB,
    /*!
      jazz: 2cvbs+s-video
      */  
    COLOR_2CVBS_SVID,
} colorspace_t;
/*!
  This structure defines the types of aspect ratio
  */
typedef enum
{
    /*!
      PanScan mode
      */
    VID_ASPECT_MODE_PANSCAN,
    /*!
      LetterBox mode
      */
    VID_ASPECT_MODE_LETTERBOX,
    /*!
      Keep the orignal video aspect ratio, centerlized.
      */
    VID_ASPECT_MODE_ORIG,
    /*!
      Auto mode
      */
    VID_ASPECT_MODE_AUTO
}disp_vid_aspect_mode_t;

/*!
  The output channels
  */
typedef enum
{
    /*!
      SD channel
      */
    DISP_CHANNEL_SD,
    /*!
      HD channel
      */
    DISP_CHANNEL_HD,
    /*!
      MAX channel
      */
    DISP_CHANNEL_MAX
}disp_channel_t;

/*!
  The supported gamma correction mode
  */
typedef enum
{
    /*!
      arc like
      */
    DISP_GAMMA_POLE_ARC,
    /*!
      S like
      */
    DISP_GAMMA_POLE_S
}disp_gamma_pole_t;


/*!
  @~english define the sigal type of DAC output
  @~chinese 定义支持的DAC信号组合
  */
typedef enum 
{
    DISP_DAC_CVBS_RGB = 0,           //CVBS + RGB输出,dac0:cvbs, dac1:sd G, dac2:sd B, dac3:sd R
    DISP_DAC_SING_CVBS,           //!<@~english dac0:cvbs, dac1~dac3:off @~chinese dac0:cvbs, dac1~dac3:关闭
    DISP_DAC_SING_CVBS_LOW_POWER,           //!<@~english dac3:cvbs, dac0~dac2:off @~chinese dac3:cvbs, dac0~dac2:关闭
    DISP_DAC_DULE_CVBS,        //!<@~english dac1:cvbs, dac0~dac2:off, dac3:cvbs @~chinese dac1:cvbs, dac0~dac2:off, dac3:cvbs
    DISP_DAC_CVBS_SVIDEO,  //CVBS + S_VIDEO, dac0:cvbs, dac1~2:s-video, dac3:off
    DISP_DAC_SVIDEO_CVBS,  //S_VIDEO + CVBS, dac0:off, dac1~2:s-video, dac3:cvbs
    DISP_DAC_DULE_CVBS_SVIDEO,  //S_VIDEO + 2CVBS, dac0:cvbs, dac1~2:s-video, dac3:cvbs
    DISP_DAC_CVBS_YPBPR_SD,  //CVBS + sd YPBPR输出，dac0:cvbs, dac1:sd y, dac2:sd pb, dac3:sd pr
    DISP_DAC_CVBS_YPBPR_HD,  //CVBS + hd YPBPR输出，dac0:cvbs, dac1:hd y, dac2:hd pb, dac3:hd pr
    DISP_DAC_TYPE_BUTT
}vdac_type_t;



/*!
  This structure defines component dac structures. 
  */
typedef struct
{
    /*! 
      The index of component group.
      */
    component_dacgrp_t comp_grp_id;
    /*! 
      The component colorspace.
      */
    colorspace_t comp_colorspace;  
    /*! 
      The dac on/off flag.
      */
    MT_BOOL b_on;  
} comp_dac_t;

/*!
  This structure defines cvbs dac structures. 
  */
typedef struct
{
    /*! 
      The index of cvbs group.
      */
    cvbs_dacgrp_t cvbs_grp_id;
    /*! 
      The dac on/off flag.
      */
    MT_BOOL b_on;  
} cvbs_dac_t;

/*!
  This structure defines svideo dac structures. 
  */
typedef struct
{
    /*! 
      The index of svideo group.
      */
    svideo_dacgrp_t svideo_grp_id;
    /*! 
      The dac on/off flag.
      */
    MT_BOOL b_on;  
} svideo_dac_t;



/*!
  This structure defines the pos
  */
typedef struct
{
    /*!
      X pos
      */
    mt_u32 x;
    /*!
      Y pos
      */
    mt_u32 y;
} pos_t;

/*!
  This structure defines the rect size
  */
typedef struct
{
    /*!
      width
      */
    mt_u32 w;
    /*!
      height
      */
    mt_u32 h;
} rect_size_t;

/*!
  This structure defines the rect for driver usage
  */
typedef struct
{
    /*!
      X pos
      */
    mt_u32 x;
    /*!
      Y pos
      */
    mt_u32 y;
    /*!
      width
      */
    mt_u32 w;
    /*!
      height
      */
    mt_u32 h;
} rect_vsb_t;

/*!
  This structure defines the rect for driver usage
  */
typedef struct
{
    /*!
      X pos
      */
    mt_s32 x;
    /*!
      Y pos
      */
    mt_s32 y;
    /*!
      width
      */
    mt_s32 w;
    /*!
      height
      */
    mt_s32 h;
} rect_sign_t;
/*!
  memory storing formats
  */
typedef enum
{
    /*!
      y and cbcr stored in seperate.
      */
    Y_CBCR = 0,
    /*!
      y,cb and cr stored in seperate.
      */
    Y_CB_CR,
    /*!
      cbcr and y stored in seperate.
      */
    CBCR_Y,
    /*!
      cb,cr and y stored in seperate.
      */
    CB_CR_Y,
    /*!
      y, cbcr stored in pic9 mode still layer.
      */
    Y_CBCR_PIC9,
    /*!
      y, cb, cr stored in pic9 mode video layer.
      */
    Y_CB_CR_PIC9,
    /*!
      max.
      */
    YCBCR_MAX
}mem_fmt_t;

/*!
  memory buffer source
  */
typedef enum
{
    /*!
      buffer from configed area.
      */
    BUFFER_SRC_CONFIG = 0,
    /*!
      buffer from malloced by upper user.
      */
    BUFFER_SRC_USER,
    /*!
      buffer from malloced by inner.
      */
    BUFFER_SRC_INNER,
    /*!
      max.
      */
    BUFFER_SRC_MAX
}buf_src_t;
/*!
  This structure defines the pixfmt
  */
typedef enum
{
    /*!
      1-bit RGB Palette index  argb palette  = 0
      */
    PIX_FMT_RGBPALETTE1,      
    /*!
      2-bit RGB Palette index  argb palette  = 1
      */
    PIX_FMT_RGBPALETTE2,       
    /*!
      4-bit RGB Palette index  argb palette  = 2
      */
    PIX_FMT_RGBPALETTE4,      
    /*!
      8-bit RGB Palette index  argb palette  = 3
      */
    PIX_FMT_RGBPALETTE8,       
    /*!
      1-bit YUV Palette index  ayuv palette  = 4
      */
    PIX_FMT_YUVPALETTE1,      
    /*!
      2-bit YUV Palette index  ayuv palette  = 5
      */
    PIX_FMT_YUVPALETTE2,     
    /*!
      4-bit YUV Palette index  ayuv palette  = 6
      */
    PIX_FMT_YUVPALETTE4,     
    /*!
      8-bit YUV Palette index  ayuv palette  = 7
      */
    PIX_FMT_YUVPALETTE8,       
    /*!
      A1 and 1-bit RGB Palette index  argb palette  = 8
      */
    PIX_FMT_ARGBPALETTE11,     
    /*!
      A2 and 2-bit RGB Palette index  argb palette  = 9
      */
    PIX_FMT_ARGBPALETTE22,    
    /*!
      A4 and 4-bit RGB Palette index  argb palette  = 10
      */
    PIX_FMT_ARGBPALETTE44,     
    /*!
      A8 and 8-bit RGB Palette index  argb palette  = 11
      */
    PIX_FMT_ARGBPALETTE88,     
    /*!
      A1 and 1-bit YUV Palette index  ayuv palette  = 12
      */
    PIX_FMT_AYUVPALETTE11,    
    /*!
      A2 and 2-bit YUV Palette index  ayuv palette  = 13
      */
    PIX_FMT_AYUVPALETTE22,     
    /*!
      A4 and 4-bit YUV Palette index  ayuv palette  = 14
      */
    PIX_FMT_AYUVPALETTE44,     
    /*!
      A8 and 8-bit YUV Palette index  ayuv palette  = 15
      */
    PIX_FMT_AYUVPALETTE88,    
    /*!
      8-bit, no per-pixel alpha        = 16
      */
    PIX_FMT_RGB233,         
    /*!
      16-bit, no per-pixel alpha       = 17
      */
    PIX_FMT_RGB565,         
    /*!
      16-bit                           = 18
      */
    PIX_FMT_ARGB1555,     
    /*!
      16-bit                           = 19
      */
    PIX_FMT_RGBA5551,       
    /*!
      16-bit                           = 20
      */
    PIX_FMT_ARGB4444,       
    /*!
      16-bit                           = 21
      */
    PIX_FMT_RGBA4444,       
    /*!
      32-bit                           = 22
      */
    PIX_FMT_ARGB8888,       
    /*!
      32-bit                           = 23
      */
    PIX_FMT_RGBA8888,      
    /*!
      32-bit for 2 pixels, YCbCr422, 8-bit values    = 24    
      */
    PIX_FMT_Y0CBY1CR8888,  
    /*!
      32-bit for 2 pixels, YCbCr422, 8-bit values    = 25
      */
    PIX_FMT_Y0CRY1CB8888,  
    /*!
      32-bit for 2 pixels, YCbCr422, 8-bit values    = 26
      */
    PIX_FMT_Y1CBY0CR8888,  
    /*!
      32-bit for 2 pixels, YCbCr422, 8-bit values    = 27
      */
    PIX_FMT_Y1CRY0CB8888,   
    /*!
      32-bit for 2 pixels, YCbCr422, 8-bit values    = 28
      */
    PIX_FMT_CBY0CRY18888,  
    /*!
      32-bit for 2 pixels, YCbCr422, 8-bit values    = 29
      */
    PIX_FMT_CBY1CRY08888,   
    /*!
      32-bit for 2 pixels, YCbCr422, 8-bit values    = 30
      */
    PIX_FMT_CRY1CBY08888,  
    /*!
      32-bit for 2 pixels, YCbCr422, 8-bit values    = 31
      */
    PIX_FMT_CRY0CBY18888,  
    /*!
      32-bit for 1 pixel,  YCbCr444, 10-bit values   = 32
      */
    PIX_FMT_X2C10Y10CB10,  
    /*!
      YCbCr444, 8-bit values   ayuv8888              = 33
      */
    PIX_FMT_AYCBCR8888,    
    /*!
      YCbCr444, 8-bit values   vuya8888              = 34
      */
    PIX_FMT_CRCBYA8888,        
    /*!
      YCbCr444, 8-bit values   yuva8888              = 35
      */
    PIX_FMT_YCBCRA8888,    
    /*!
      YCbCr444, 8-bit values                         = 36
      */
    PIX_FMT_ACRCBY8888,    
    /*!
      YCbCr444, 24-bit values                        = 37
      */
    PIX_FMT_YCBCR444,       
    /*!
      YCbCr422, 16-bit values                        = 38
      */
    PIX_FMT_YCBCR422,      
    /*!
      YCbCr420, 12-bit values                        = 39
      */
    PIX_FMT_YCBCR420,  
    /*!
      2-bit RGB Palette index  bgra palette  = 40
      */
    PIX_FMT_RGBPALETTE2_PALETTE_BGRA,       
    /*!
      4-bit RGB Palette index  bgra palette  = 41
      */
    PIX_FMT_RGBPALETTE4_PALETTE_BGRA,      
    /*!
      8-bit RGB Palette index  bgra palette  = 42
      */
    PIX_FMT_RGBPALETTE8_PALETTE_BGRA,       
    /*!
      2-bit YUV Palette index  vuya palette  = 43
      */
    PIX_FMT_YUVPALETTE2_PALETTE_VUYA,     
    /*!
      4-bit YUV Palette index  vuya palette  = 44
      */
    PIX_FMT_YUVPALETTE4_PALETTE_VUYA,     
    /*!
      8-bit YUV Palette index  vuya palette  = 45
      */
    PIX_FMT_YUVPALETTE8_PALETTE_VUYA,       
    /*!
      A4 and 4-bit RGB Palette index   bgra palette   = 46
      */
    PIX_FMT_ARGBPALETTE44_PALETTE_BGRA,     
    /*!
      A8 and 8-bit RGB Palette index  bgra palette  = 47
      */
    PIX_FMT_ARGBPALETTE88_PALETTE_BGRA,     
    /*!
      8-bit RGB and A8 Paletteindex   argb palette   = 48
      */
    PIX_FMT_RGBAPALETTE88,     
    /*!
      8-bit RGB and A8 Palette index  bgra palette  = 49
      */
    PIX_FMT_RGBAPALETTE88_PALETTE_BGRA,     
    /*!
      A4 and 4-bit YUV Palette index  vuya palette  = 50
      */
    PIX_FMT_AYUVPALETTE44_PALETTE_VUYA,     
    /*!
      A8 and 8-bit YUV Palette index  vuya palette  = 51
      */
    PIX_FMT_AYUVPALETTE88_PALETTE_VUYA,     
    /*!
      8-bit YUV and A8 Palette index  ayuv palette  = 52
      */
    PIX_FMT_YUVAPALETTE88,     
    /*!
      8-bit YUV and A8 Palette index  vuya palette  = 53
      */
    PIX_FMT_YUVAPALETTE88_PALETTE_VUYA,     
    /*!
      16-bit, no per-pixel alpha       = 54
      */
    PIX_FMT_RGB565_SMALL_ENDIAN,         
    /*!
      16-bit                           = 55
      */
    PIX_FMT_ARGB1555_SMALL_ENDIAN,     
    /*!
      16-bit                           = 56
      */
    PIX_FMT_ARGB4444_SMALL_ENDIAN,       
    /*!
      32-bit                           = 57
      */
    PIX_FMT_ARGB8888_SMALL_ENDIAN,       
    /*!
      32-bit                           = 58
      */
    PIX_FMT_RGBA8888_SMALL_ENDIAN,      
    /*!
      16-bit                           = 59
      */
    PIX_FMT_RGBA5551_SMALL_ENDIAN,     
    /*!
      16-bit                           = 60
      */
    PIX_FMT_RGBA4444_SMALL_ENDIAN,       
    /*!
      2-bit RGB Palette index  rgba palette  = 61
      */
    PIX_FMT_RGBPALETTE2_PALETTE_RGBA,       
    /*!
      2-bit RGB Palette index  abgr palette  = 62
      */
    PIX_FMT_RGBPALETTE2_PALETTE_ABGR,       
    /*!
      4-bit RGB Palette index  rgba palette  = 63
      */
    PIX_FMT_RGBPALETTE4_PALETTE_RGBA,       
    /*!
      4-bit RGB Palette index  abgr palette  = 64
      */
    PIX_FMT_RGBPALETTE4_PALETTE_ABGR,       
    /*!
      8-bit RGB Palette index  rgba palette  = 65
      */
    PIX_FMT_RGBPALETTE8_PALETTE_RGBA,       
    /*!
      8-bit RGB Palette index  abgr palette  = 66
      */
    PIX_FMT_RGBPALETTE8_PALETTE_ABGR,       
    /*!
      2-bit YUV Palette index  yuva palette  = 67
      */
    PIX_FMT_YUVPALETTE2_PALETTE_YUVA,     
    /*!
      2-bit YUV Palette index  avuy palette  = 68
      */
    PIX_FMT_YUVPALETTE2_PALETTE_AVUY,     
    /*!
      4-bit YUV Palette index  yuva palette  = 69
      */
    PIX_FMT_YUVPALETTE4_PALETTE_YUVA,     
    /*!
      4-bit YUV Palette index  avuy palette  = 70
      */
    PIX_FMT_YUVPALETTE4_PALETTE_AVUY,     
    /*!
      8-bit YUV Palette index  yuva palette  = 71
      */
    PIX_FMT_YUVPALETTE8_PALETTE_YUVA,     
    /*!
      8-bit YUV Palette index  avuy palette  = 72
      */
    PIX_FMT_YUVPALETTE8_PALETTE_AVUY,     
    /*!
      A4 and 4-bit RGB Palette index   rgba palette   = 73
      */
    PIX_FMT_ARGBPALETTE44_PALETTE_RGBA,     
    /*!
      A4 and 4-bit RGB Palette index   abgr palette   = 74
      */
    PIX_FMT_ARGBPALETTE44_PALETTE_ABGR,     
    /*!
      A8 and 8-bit RGB Palette index   rgba palette   = 75
      */
    PIX_FMT_ARGBPALETTE88_PALETTE_RGBA,     
    /*!
      A8 and 8-bit RGB Palette index   abgr palette   = 76
      */
    PIX_FMT_ARGBPALETTE88_PALETTE_ABGR,     
    /*!
      8-bit RGB and A8 Paletteindex   rgba palette   = 77
      */
    PIX_FMT_RGBAPALETTE88_PALETTE_RGBA,     
    /*!
      8-bit RGB and A8 Palette index  abgr palette  = 78
      */
    PIX_FMT_RGBAPALETTE88_PALETTE_ABGR,     
    /*!
      A4 and 4-bit YUV Palette index  yuva palette  = 79
      */
    PIX_FMT_AYUVPALETTE44_PALETTE_YUVA,     
    /*!
      A4 and 4-bit YUV Palette index  avuy palette  = 80
      */
    PIX_FMT_AYUVPALETTE44_PALETTE_AVUY,     
    /*!
      A8 and 8-bit YUV Palette index  yuva palette  = 81
      */
    PIX_FMT_AYUVPALETTE88_PALETTE_YUVA,     
    /*!
      A8 and 8-bit YUV Palette index  avuy palette  = 82
      */
    PIX_FMT_AYUVPALETTE88_PALETTE_AVUY,     
    /*!
      8-bit YUV and A8 Palette index  yuva palette  = 83
      */
    PIX_FMT_YUVAPALETTE88_PALETTE_YUVA,     
    /*!
      8-bit YUV and A8 Palette index  avuy palette  = 84
      */
    PIX_FMT_YUVAPALETTE88_PALETTE_AVUY, 
    /*!
      = 85
      */ 
    PIX_FMT_GRAY_8,   
    /*!
      used by concerto = 86
      */
    PIX_FMT_RGBPALETTE1_PALETTE_BGRA, 
    /*!
      used by concerto = 87
      */
    PIX_FMT_XY,
    /*!
      used by concerto = 88
      */
    PIX_FMT_XYL,
    /*!
      used by concerto = 89
      */
    PIX_FMT_XYC,
    /*!
      used by concerto = 90
      */
    PIX_FMT_XYLC,
    /*!
      used by concerto = 91
      */
    PIX_FMT_XY_SMALL,
    /*!
      used by concerto = 92
      */
    PIX_FMT_XYL_SMALL,
    /*!
      used by concerto = 93
      */
    PIX_FMT_XYC_SMALL,
    /*!
      used by concerto = 94
      */
    PIX_FMT_XYLC_SMALL,
    /*!
      used by concerto = 95
      */
    PIX_FMT_TILE,
    /*!
      rgb888 24bits = 96 
      */
    PIX_FMT_RGB888,
    /*! 
      bgr8888 24bits = 97
      */
    PIX_FMT_BGR888,
    /*! 
      gray16 16bits = 98
      */
    PIX_FMT_GRAY_16,
    /*! 
      semi-planner yuv = 99
      */
    PIX_FMT_SP_YUV444,
    /*! 
      semi-planner yuv = 100
      */
    PIX_FMT_SP_YUV422,
    /*! 
      semi-planner yuv = 101
      */
    PIX_FMT_SP_YUV420,
    /*! 
      semi-planner yuv = 102
      */
    PIX_FMT_SP_YUV422_1x2,
    /*! 
      semi-planner yuv = 103
      */
    PIX_FMT_SP_YUV422_2x1,
    /*! 
      semi-planner yuv = 104
      */
    PIX_FMT_SP_YUV444_UVSWAP,
    /*! 
      semi-planner yuv = 105
      */
    PIX_FMT_SP_YUV422_UVSWAP,
    /*! 
      semi-planner yuv = 106
      */
    PIX_FMT_SP_YUV420_UVSWAP,  
    /*! 
      semi-planner yuv = 107
      */
    PIX_FMT_SP_YUV422_1x2_UVSWAP,
    /*! 
      semi-planner yuv = 108
      */
    PIX_FMT_SP_YUV422_2x1_UVSWAP,
    /*!
      cmyk = 109
      */
    PIX_FMT_CMYK,
    /*!
      kymc = 110
      */  
    PIX_FMT_KYMC,
    /*!
      semi-planner cmyk = 111
      */  
    PIX_FMT_SP_CMYK,    
    /*!
      semi-planner cmyk = 112
      */  
    PIX_FMT_SP_CMYK_SWAP,     
    /*!
      gray 1 bit per pixel = 113
      */
    PIX_FMT_GRAY_1,
    /*!
      gray 2 bits per pixel = 114
      */
    PIX_FMT_GRAY_2,
    /*!
      gray 4 bits per pixel = 115
      */
    PIX_FMT_GRAY_4,
    /*!
      gray with alpha, 16bits per pixel,  = 116
      */
    PIX_FMT_AGRAY_8_8,  
    /*!
      gray with alpha, 32bits per pixel,  = 117
      */
    PIX_FMT_AGRAY_16_16,  
    /*!
      bgr16_16_16 48bits per pixel = 118
      */
    PIX_FMT_BGR48,  
    /*!
      abgr16_16_16_16 64bits per pixel= 119
      */
    PIX_FMT_ABGR64,    
    /*!
      max
      */
    PIX_FMT_MAX
} pix_fmt_t;
/*!
  This structure defines region structure.
  */
typedef struct
{
    /*!
size:x,y is based on the plane
*/
    rect_vsb_t          rect;
    /*!
      pitch in bytes
      */
    mt_u32                 pitch;
    /*!
      pixel format
      */
    pix_fmt_t           pix_fmt;
    /*!
      pixel format
      */
    colorspace_t        color_sp;
    /*!
      pixel format
      */
    mt_u8                  bpp;
    /*!
      palette exist flag
      */
    MT_BOOL                b_palette;
    /*!
      palette pointer
      */
    mt_u32                 *p_palette;
    /*!
      the attached plane 
      */
    mt_u8                  layerId;
    /*!
      alpha value 
      */
    mt_u8                  alpha;
    /*!
      alpha enable 
      */
    MT_BOOL                b_alpha;
    /*!
      display flag 
      */
    MT_BOOL                b_show;
    /*!
      odd buffer pointer
      */
    void                *p_buf_odd;
    /*!
      even buffer pointer,, NULL for frame buffer use  
      */
    void                *p_buf_even;
    /*!
      graphic vertical scaler buffer for gaphic engine
      */
    void                *p_buf_vs;
    /*!
      The semaphore for the region operation
      */
    // os_sem_t            sem;
    /*!
      Update flag
      */
    volatile mt_u8                  flag;
    /*!
      memory storing format
      */
    mem_fmt_t           mem_fmt;
    /*!
      palette entry changed
      */
    mt_u32                 entry_changed;
    /*!
      buffer src
      */
    buf_src_t           buf_src;
    /*!
      context(only used for zoran project.)
      */
    void *p_context;    
    /*!
      for linux use
      */
    void *p_rgn_buf;
    /*!
      premultiply alpha 
      */
    mt_u8                  premultiply;

    /*!
      little endian
      */
    MT_BOOL          little_endian;
    mt_u32 buf_size; //order
} region_t;
/*!
  This structure defines region node.
  */
typedef struct rgn_nod
{
    /*! 
      The region handle.
      */
    region_t *p_rgn;
    /*! 
      The region header.
      */
    void *p_header;
    /*! 
      The next.
      */
    struct rgn_nod *p_next;
} rgn_nod_t;

/*!
  This structure defines region node memory pool.
  */
typedef struct rgn_nod_pool
{
    /*! 
      The region node.
      */
    rgn_nod_t nod;
    /*! 
      The used flag.
      */
    MT_BOOL b_used;
} rgn_nod_pool_t;

typedef enum
{
    /*!
      Sharpening mode
      */
    PP_MODE_SHARPENING,
    /*!
      Adaptive mode
      */
    PP_MODE_ADAPTIVE,
    /*!
      Soft mode.
      */
    PP_MODE_SOFT,
    /*!
      NEW standard pass AE freq response
      */
    PP_MODE_STANDARD,
    /*!
      NEW default--montage prefered,look like mstar
      */
    PP_MODE_DEFAULT,
    /*!
      NEW vivid--more like ali
      */
    PP_MODE_VIVID,  
}disp_pp_mode_t;
#define MAX_REGION_NO      (50)
/*!
  Max osd header size.
  */
#define MAX_HEADER_SIZE      (100)

/*!
  This structure defines header memory pool.
  */
typedef struct rgn_header_pool
{
    /*! 
      The header.
      */
    mt_u8 header[MAX_HEADER_SIZE];
    /*! 
      The used flag.
      */
    MT_BOOL b_used;
} rgn_header_pool_t;

/*!
  Initialization parameters for an layer. 
  */
typedef struct 
{
    /*! 
      The start address of idle layer buffer for odd field data.
      */
    mt_u32 odd_mem_start;
    /*! 
      The start address of idle layer buffer for even field data.
      */
    mt_u32 even_mem_start;
    /*! 
      The end address of idle layer buffer for odd field data.
      */
    mt_u32 odd_mem_end;
    /*! 
      The end address of idle layer buffer for even field data.
      */
    mt_u32 even_mem_end;
    /*! 
      The osd filter_param, anti-flicking parameters, from 1~7.
      */
    mt_u32 antiflicker_level;
}layer_cfg_t;

/*!
  Initialization misc using buffer. 
  */
typedef struct 
{
    /*! 
      The di address.
      */
    mt_u32 di_addr;
    /*! 
      The di size.
      */
    mt_u32 di_size;
    /*! 
      The write back address.
      */
    mt_u32 sd_wb_addr;
    /*! 
      The write back size.
      */
    mt_u32 sd_wb_size;
    /*! 
      The write back field num.
      */
    mt_u32 sd_wb_field_no;
}misc_buf_cfg_t;

/*!
  Initialization video pp cfg. 
  */
typedef struct 
{
    /*! 
      The bright,satua,etc.
      */
    mt_u32 pp_value;
    /*! 
      The hd shoot.
      */
    mt_u32 shoot_value;
    /*! 
      pp mode.
      */
    disp_pp_mode_t pp_mode;
}pp_cfg_t;


/*!
  Initialization parameters for the still picture display layer
  */
typedef struct
{
    /*!
      The buffer config for still layer.
      */    
    layer_cfg_t *p_cfg;
    /*! 
      The display mode for still picture: TRUE for frame and FALSE for field
      */
    MT_BOOL b_frame;
    /*!
      The byte order for data storage of sill picture
      */
    MT_BOOL b_endian;
    /*!
      The horizontal intervals of still picture hw value between 0~15
      */
    mt_u16 int_h;
    /*!
      The vertical intervals of still picture  hw value between 0~15
      */
    mt_u16 int_v;  
}still_cfg_t;

/*!
  This structure defines a display plane for 9-picture.
  */
typedef struct
{
    /*! 
      The left x coordinate of 9-picture plane. 
      */
    mt_u16 x;
    /*! 
      The top y coordinate of 9-picture plane
      */
    mt_u16 y;
    /*! 
      The width of 9-picture each unit. It should not exceed 180.
      */
    mt_u8  width;
    /*! 
      The height of 9-picture each unit. It should not exceed 144 
      */
    mt_u8  height;
    /*! 
      The width of horizontal intervals between 9 pic units
      */
    mt_u16 interval_h;
    /*! 
      The width of vertical intervals between 9 pic units
      */
    mt_u16 interval_v;
}disp_pic_9_t;

/*!
  This structure defines a frame of video scaled in 9 picture mode.
  */
typedef struct
{
    /*! 
      The width after scaling.
      */
    mt_u16 width;
    /*! 
      The height after scaling.
      */
    mt_u16 height;
    /*! 
      The offset to the width of a 9 picture unit. 
      */
    mt_u16 x_delta;
    /*! 
      The offset to the height of a 9 picture unit. 
      */  
    mt_u16 y_delta;
}multipic_unit_scale_t;

/*!
  Initialization parameters for the display layers
  */
typedef struct
{
    /*! 
      pointer to the still sd cfg
      */
    still_cfg_t *p_still_sd_cfg;
    /*! 
      pointer to the still sd cfg
      */
    still_cfg_t *p_still_hd_cfg;
    /*! 
      pointer to the osd0 cfg
      */
    layer_cfg_t *p_osd0_cfg;
    /*! 
      pointer to the osd1 cfg
      */
    layer_cfg_t *p_osd1_cfg;
    /*! 
      pointer to the sub cfg
      */
    layer_cfg_t *p_sub_cfg;
    /*! 
      pointer to the dummy osd cfg
      */
    layer_cfg_t *p_dummy_osd_cfg;
    /*! 
      pointer to the osd0 vscaler cfg
      */
    layer_cfg_t *p_osd0_vscale_cfg;
    /*! 
      pointer to the osd1 vscaler cfg
      */
    layer_cfg_t *p_osd1_vscale_cfg;
    /*! 
      pointer to the sub vscaler cfg
      */
    layer_cfg_t *p_sub_vscale_cfg;  
    /*! 
      pointer to misc buffer cfg
      */
    misc_buf_cfg_t misc_buf_cfg;  
    /*! 
      use vscaler or not
      */
    MT_BOOL b_vscale;
    /*! 
      use osd hscale or not
      */
    MT_BOOL b_osd_hscle;
    /*! 
      use osd vscale or not
      */
    MT_BOOL b_osd_vscle;
    /*! 
      use di enable
      */
    MT_BOOL b_di_en;
    /*! 
      ui default graphic size
      */
    rect_size_t *p_gra_default_size;
    /*!
      if use the public sharing driver service, set the handle here 
      */
    void *p_drvsvc;  
    /*!
      if not use the public sharing driver service, set this, hdmi notify task priority
      */
    mt_u32 task_prio;
    /*!
      if not use the public sharing driver service, set this, hdmi notify task size
      */  
    mt_u32 stack_size;
    /*!
      The video pp cfg
      */  
    pp_cfg_t pp_cfg;

    /*!
      The resource lock type
      */
    mt_u32 lock_type;
    /*! 
      yliu add
      The start address of coef tab.
      */
    mt_u32 p_coeff_start;
    /*!
      The background color
      */
    mt_u32 background_color;
    /*!
      The hdcp onoff
      */
    MT_BOOL b_hdcp_on;
    /*!
      The frequence_respond cfg
      */
    MT_BOOL b_freq_resp;
    /*!
      sd write back mode, should set as 444 to pass frequence respond;
      can set as 422 when memory is limited
      */
    MT_BOOL b_wrback_422;  
    /*!
      user close prescale, horizontal scale ratio can't be less than 1/7, 
      and vertical scale ratio can't be less than 1/4
      */
    MT_BOOL b_unuse_prescale;  
    /*!
      shared memory used by av and ap cpu
      */
    mt_u32 av_ap_shared_mem;
    /*!
      shared memory size
      */
    mt_u32 shared_mem_size;
    /*!
      Uninitialized in uboot
      */
    MT_BOOL b_uboot_uninit;  
}disp_cfg_t;

/*!
  This structure defines display common info.
  */
typedef struct
{
    /*! 
      The video channel number
      */
    mt_u32 ch_num;
    /*! 
      The vertical scaler buffer usage
      */
    MT_BOOL use_vscale_buf;  
    /*! 
      The eid info
      */
    mt_u32 *p_hdmi_eid;  
    /*! 
      The hdmi connect status
      */
    MT_BOOL is_hdmi_connected;  
}disp_common_info_t;

/*!
  This structure defines scaler info.
  */
typedef struct
{
    /*! 
      The input rect.
      */
    rect_vsb_t in_rect;
    /*! 
      The output rect.
      */
    rect_vsb_t out_rect;
    /*! 
      zoom in state.
      */
    MT_BOOL b_zoom_in;
    /*! 
      zoom out state.
      */
    MT_BOOL b_zoom_out;
    /*
       cut zoom state
       */
    MT_BOOL b_cut_zoom;
} scale_info_t;

/*!
  This structure defines rate convert  info.
  */
typedef struct
{
    /*! 
      The input rate.
      */
    mt_u32 in_rate;
    /*! 
      The out rate.
      */
    mt_u32 out_rate;
} rc_info_t;

/*!
  This structure defines common layer info.
  */
typedef struct 
{
    /*! 
      The layer alpha value.
      */
    mt_u8 alpha;
    /*! 
      The plane alpha enable flag.
      */
    MT_BOOL b_alpha;
    /*! 
      The layer show state.
      */
    MT_BOOL b_show;
    /*! 
      The colorkey enable flag.
      */
    MT_BOOL b_ck;
    /*! 
      The using gaphic engine vertical scaler.
      */
    MT_BOOL b_use_vs;
    /*! 
      The colorkey value.
      */
    mt_u32 ck;
    /*! 
      The rectangle clip.
      */
    rect_vsb_t rect_clip;
    /*! 
      The scaler info.
      */
    scale_info_t scale_info;
    /*! 
      The region node list in still, osd, sub, video layer.
      */
    rgn_nod_t *p_rgn_list;  
    /*! 
      The region number.
      */
    mt_u8 rgn_no;  
    /*! 
      The flag indicate layer update state, set in api clear in isr.
      */
   // mt_u16 update_flag;  
    /*! 
      layer buffer cfg.
      */
    layer_cfg_t layer_cfg;  
}layer_info_t;
/*!
  This structure defines coeff tables.
  */
typedef struct
{
    /*! 
      The coeff table address.
      */
    mt_u32 coeff_addr;
    /*! 
      The coeff table size.
      */
    mt_u32 coeff_size;
} coeff_t;

/*!
  This structure defines scale info.
  */
typedef struct
{
    /*! 
      The gra hscale coeff.
      */
    coeff_t gra_hscale_coeff;
    /*! 
      The gra vscale coeff.
      */
    coeff_t gra_vscale_coeff;
    /*! 
      The hd video hscale coeff.
      */
    coeff_t hdvid_hscale_coeff;
    /*! 
      The hd video vscale coeff.
      */
    coeff_t hdvid_vscale_coeff;
    /*! 
      The sd video hscale coeff.
      */
    coeff_t sdvid_hscale_coeff;
    /*! 
      The sd video vscale coeff.
      */
    coeff_t sdvid_vscale_coeff;
    /*! 
      The osd scale coeff.
      */
    coeff_t osd_scale_coeff;
} coeff_info_t;

/*!
  The aspect ratio
  */
typedef enum
{
    /*!
4:3
*/
    AR_43,
    /*!
      16:9
      */
    AR_169,
    /*!
      Square or 1:1
      */
    AR_SQUARE
}aspect_ratio_t;

/*!
  The aspect ratio
  */
typedef enum
{
    /*!
      aspect ratio as device Resolution
      */
    MT_DRV_AR_AUTO,
    /*!
      4:3
      */
    MT_DRV_AR_4TO3,
    /*!
      16:9
      */
    MT_DRV_AR_16TO9,
    /*!
      Square or 1:1
      */
    MT_DRV_AR_SQUARE,
    /*!
      Square or 1:1
    */
    MT_DRV_AR_24TO10,
    /*!
      Square or 1:1
    */
    MT_DRV_AR_9TO16
}MT_DRV_ASPECT_RATIO_E;

/*!
  The di mode
  */
typedef enum
{
    /*!
      weave
      */
    DI_MODE_WEAVE,
    /*!
      bob
      */
    DI_MODE_BOB,
    /*!
      auto
      */
    DI_MODE_AUTO
}di_mode_t;

/*!
  The scale mode
  */
typedef enum
{
    /*!
      poly-phase
      */
    SCALE_MODE_POLYPHASE,
    /*!
      matrix
      */
    SCALE_MODE_MATRIX,
    /*!
      mix poly and matrix
      */
    SCALE_MODE_MIX
}scale_mode_t;


/*!
  This structure defines coeff tables.
  */
typedef struct
{
    /*!
      The coeff table physical address.
      */
    mt_u32 coeff_phy_addr;
    /*!
      The coeff table kernel vitual address.
      */
    ulong coeff_kvir_addr;
    /*!
      The coeff table size.
      */
    mt_u32 coeff_size;
} disp_coeff_s;

#if 1

//FILTER table
#define DISP_SINGLE_SCALE_COEFF_TABLE_SIZE 0x100
/*!
  The scale coef table index
  */
typedef enum
{ 
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)  || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
    /*!
      table 7_0 horizontal
      */
    DISP_SCALE_COEFF_TABLE_7_0_H,  
    /*!
      table 7_0 vertical
      */
    DISP_SCALE_COEFF_TABLE_7_0_V,  
#endif  
    /*!
      table 7 vertical sharp
      */
    DISP_SCALE_COEFF_TABLE_7_V_SHARP,
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
    /*!
      table 9 vertical
      */
    DISP_SCALE_COEFF_TABLE_9_V,  
#endif  
#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)  || defined(CONFIG_MT_CHIP_SYMPHONY6)  //sym6

    /*!
      new csc HDR_HDR10 table part 1
      */
    NEW_CSC_COEFF_HDR_HDR10_PART_1,
    /*!
      new csc HDR_HDR10 table part 2
      */
    NEW_CSC_COEFF_HDR_HDR10_PART_2,
    /*!
      new csc HDR_HLG table part 1
      */
    NEW_CSC_COEFF_HDR_HLG_PART_1,
    /*!
      new csc HDR_HLG table part 2
      */
    NEW_CSC_COEFF_HDR_HLG_PART_2,
    /*!
      new csc SDR table part 1
      */
    NEW_CSC_COEFF_SDR_PART_1,
    /*!
      new csc SDR table part 2
      */
    NEW_CSC_COEFF_SDR_PART_2,
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)	//sym6
    ADV_CSC_LUT_TABLE_1,
    ADV_CSC_LUT_TABLE_2,
    ADV_CSC_LUT_TABLE_3,
    ADV_CSC_LUT_TABLE_4,
    ADV_CSC_LUT_TABLE_5,
    ADV_CSC_LUT_TABLE_6,
    ADV_CSC_LUT_TABLE_7,
    ADV_CSC_LUT_TABLE_8,
    ADV_CSC_LUT_TABLE_9,
    ADV_CSC_LUT_TABLE_10,
    ADV_CSC_LUT_TABLE_11,
    ADV_CSC_LUT_TABLE_12,
    ADV_CSC_LUT_TABLE_13,
    ADV_CSC_LUT_TABLE_14,
    ADV_CSC_LUT_TABLE_15,
    ADV_CSC_LUT_TABLE_16,
    ADV_CSC_LUT_TABLE_17,
    ADV_CSC_LUT_TABLE_18,
    ADV_CSC_LUT_TABLE_19,
    ADV_CSC_LUT_TABLE_20,
    ADV_CSC_LUT_TABLE_21,
    ADV_CSC_LUT_TABLE_22,
    ADV_CSC_LUT_TABLE_23,
    ADV_CSC_LUT_TABLE_24,
    ADV_CSC_LUT_TABLE_25,
    ADV_CSC_LUT_TABLE_26,
    ADV_CSC_LUT_TABLE_27,
    ADV_CSC_LUT_TABLE_28,
    ADV_CSC_LUT_TABLE_29,
    ADV_CSC_LUT_TABLE_30,
    ADV_CSC_LUT_TABLE_31,
    ADV_CSC_LUT_TABLE_32,
    ADV_CSC_LUT_TABLE_33,
    ADV_CSC_LUT_TABLE_34,
    ADV_CSC_LUT_TABLE_35,
    ADV_CSC_LUT_TABLE_36,
    ADV_CSC_LUT_TABLE_37,
    ADV_CSC_LUT_TABLE_38,
    ADV_CSC_LUT_TABLE_39,
    ADV_CSC_LUT_TABLE_40,
    ADV_CSC_LUT_TABLE_41,
    ADV_CSC_LUT_TABLE_42,
    ADV_CSC_LUT_TABLE_43,
    ADV_CSC_LUT_TABLE_44,
    ADV_CSC_LUT_TABLE_45,
    ADV_CSC_LUT_TABLE_46,
    ADV_CSC_LUT_TABLE_47,
    ADV_CSC_LUT_TABLE_48,
    ADV_CSC_LUT_TABLE_49,
    ADV_CSC_LUT_TABLE_50,
    ADV_CSC_LUT_TABLE_51,
    ADV_CSC_LUT_TABLE_52,
    ADV_CSC_LUT_TABLE_53,
    ADV_CSC_LUT_TABLE_54,
    ADV_CSC_LUT_TABLE_55,
    ADV_CSC_LUT_TABLE_56,
    ADV_CSC_LUT_TABLE_57,
    ADV_CSC_LUT_TABLE_58,
    ADV_CSC_LUT_TABLE_59,
    ADV_CSC_LUT_TABLE_60,
    ADV_CSC_LUT_TABLE_61,
    ADV_CSC_LUT_TABLE_62,
    ADV_CSC_LUT_TABLE_63,
    ADV_CSC_LUT_TABLE_64,
    ADV_CSC_LUT_TABLE_65,
    ADV_CSC_LUT_TABLE_66,
    ADV_CSC_LUT_TABLE_67,
    ADV_CSC_LUT_TABLE_68,
    ADV_CSC_LUT_TABLE_69,
    ADV_CSC_LUT_TABLE_70,
    ADV_CSC_LUT_TABLE_71,
    ADV_CSC_LUT_TABLE_72,
    ADV_CSC_LUT_TABLE_73,
    ADV_CSC_LUT_TABLE_74,
    ADV_CSC_LUT_TABLE_75,
    ADV_CSC_LUT_TABLE_76,
    ADV_CSC_LUT_TABLE_77,

    SDR_LITE_SD_TABLE_1,
    SDR_LITE_SD_TABLE_2,
    SDR_LITE_SD_TABLE_3,
    SDR_LITE_SD_TABLE_4,
    SDR_LITE_SD_TABLE_5,
    SDR_LITE_SD_TABLE_6,
    SDR_LITE_SD_TABLE_7,
    SDR_LITE_SD_TABLE_8,

    HDR_LITE_SD_TABLE_1,
    HDR_LITE_SD_TABLE_2,
    HDR_LITE_SD_TABLE_3,
    HDR_LITE_SD_TABLE_4,
    HDR_LITE_SD_TABLE_5,
    HDR_LITE_SD_TABLE_6,
    HDR_LITE_SD_TABLE_7,
    HDR_LITE_SD_TABLE_8,

    HDR2SDR_LITE_GAIN_SD_TABLE,

    SDR_LITE_OSD_TABLE_1,
    SDR_LITE_OSD_TABLE_2,
    SDR_LITE_OSD_TABLE_3,
    SDR_LITE_OSD_TABLE_4,
    SDR_LITE_OSD_TABLE_5,
    SDR_LITE_OSD_TABLE_6,
    SDR_LITE_OSD_TABLE_7,
    SDR_LITE_OSD_TABLE_8,

    HDR_LITE_OSD_TABLE_1,
    HDR_LITE_OSD_TABLE_2,
    HDR_LITE_OSD_TABLE_3,
    HDR_LITE_OSD_TABLE_4,
    HDR_LITE_OSD_TABLE_5,
    HDR_LITE_OSD_TABLE_6,
    HDR_LITE_OSD_TABLE_7,
    HDR_LITE_OSD_TABLE_8,

    HDR_LITE_GAIN_OSD_TABLE,  

#endif
    /*!
      table max
      */
    DISP_SCALE_COEFF_TABLE_MAX
}DISP_COEFF_TABLE_E;//coeff_table_t;
#endif
/*!
  venc oversample
  */
typedef enum
{
    /*!
      disable oversample
      */
    OS_DIS,
    /*!
      27->162 or 74.25->148.5
      */
    OS_H,
    /*!
      27->108 or 74.25->148.5
      */
    OS_L,
}oversample_t;
/*!
  This structure defines disp input&output info.
  */
typedef struct
{
    /*! 
      The aspect in video layer.
      */
    aspect_ratio_t ar;
    /*! 
      The aspect mode in video layer.
      */
    disp_vid_aspect_mode_t ar_mode;
    /*! 
      The video format  in video layer.
      */
    disp_sys_t vid_fmt;
    /*! 
      The graphic  default size based on UI.
      */
    rect_size_t gra_default_size;
    /*! 
      The video cur rect, sd hd.
      */
    rect_vsb_t cur_rect[DISP_CHANNEL_MAX];
    /*! 
      The video old rect, sd hd.
      */
    rect_vsb_t old_rect[DISP_CHANNEL_MAX];
    /*! 
      The frame or field.
      */
    MT_BOOL b_frame;
    /*! 
      The progressive or interleaved.
      */
    MT_BOOL b_progressive;
    MT_BOOL bIs3D;
    MT_BOOL update_flag;  
} disp_info_t;


/*!
  This structure defines rate convert  info.
  */
typedef struct
{
    /*! 
      The user cfg di enable.
      */
    volatile MT_BOOL di_user_en;
    /*! 
      The firmware force di disable cur.
      */
    MT_BOOL di_firmware_force_disable_cur;
    /*! 
      The firmware force di disable old.
      */
    MT_BOOL di_firmware_force_disable_old;
    /*! 
      The di enable cur status.
      */
    MT_BOOL di_en_cur;
    /*! 
      The di enable old status.
      */
    MT_BOOL di_en_old;
    /*! 
      The di user enable old status.
      */
    MT_BOOL di_user_en_old;    
    /*! 
      The di mode.
      */
    di_mode_t di_mode;
    /*! 
      The di addr.
      */
    mt_u32 di_addr;
    /*! 
      The di size.
      */
    mt_u32 di_size;
} di_info_t;

/*!
  This structure defines color space info.
  */
typedef struct
{
    /*! 
      color space info: current output resolution.
      */
    mt_u8 cur_resolution;
    /*! 
      color space info: old output resolution.
      */
    mt_u8 old_resolution;
    /*! 
      csc on/off.
      */
    mt_u8 csc_enable;
    mt_u32 cur_transfer_characteristics;
    mt_u32 old_transfer_characteristics;
    mt_u32 cur_colour_primaries;
    mt_u32 old_colour_primaries;
    mt_u32 cur_tv_capability;
    mt_u32 old_tv_capability;
    mt_u32 cur_tv_sys;
    mt_u32 old_tv_sys;
    mt_u8 cur_use_3scaler;
    mt_u8 old_use_3scaler;
    mt_u32 sub_colour_primaries_2020;
    mt_u32 old_sub_colour_primaries_2020;
    mt_u8 hdr10p_metadata_flag;
    mt_u8 old_hdr10p_metadata_flag;
    mt_u8 sl_hdr_metadata_flag;
    mt_u8 old_sl_hdr_metadata_flag;
    mt_u8 vivid_hdr_metadata_flag;
    mt_u8 old_vivid_hdr_metadata_flag;    
    mt_u8 video_full_range;
    mt_u8 old_video_full_range;
    mt_u32 cur_deep_color;
    mt_u32 old_deep_color;
}csc_info_t;

/*!
  This structure defines color space info.
  */
typedef struct
{
    /*! 
      color space info: current output resolution.
      */
    mt_u8 cur_resolution;
    /*! 
      color space info: old output resolution.
      */
    mt_u8 old_resolution;
    /*! 
      denoise on/off.
      */
    mt_u8 denoise_enable;
}denoise_info_t;

/*!
  This structure defines HDMI info.
  */
typedef struct
{
#if 0
    /*!
      whether support HDR10P_EMP.
      */
    MT_BOOL supported_hdr10p_emp;

    /*!
      whether support HDR10P.
      */
    MT_BOOL supported_hdr10p_vsif;

    /*!
      whether support HDR10.
      */
    MT_BOOL supported_hdr10;
    /*!
      whether support HLG.
      */
    MT_BOOL supported_hlg;
    /*!
      whether support BT.2020.
      */
    MT_BOOL supported_bt2020;

    
	/*!
	the flag to indicate is the device support deep color rgb 30bit
	*/
	MT_BOOL rgb30bit;
	/*!
	the flag to indicate is the device support deep color rgb 36bit
	*/
	MT_BOOL rgb36bit;
	/*!
	the flag to indicate is the device support deep color rgb 48bit
	*/
	MT_BOOL rgb48bit;
	/*!
	the flag to indicate is the device support deep color y444
	*/
	MT_BOOL dc_y444;
	/*!
	the flag to indicate is the device support deep color yuv420 30bit
	*/
	MT_BOOL y420_30bit;
	/*!
	the flag to indicate is the device support deep color yuv420 36bit
	*/
	MT_BOOL y420_36bit;
	/*!
	the flag to indicate is the device support deep color yuv420 48bit
	*/
	MT_BOOL y420_48bit;

    /*!
      the flag to indicate is the device support 3d frame packing
      */
    MT_BOOL supported_3d_frmpack;
    /*!
      the flag to indicate is the device support 3d top and bottom
      */
    MT_BOOL supported_3d_tb;
    /*!
      the flag to indicate is the device support 3d side by side
      */
    MT_BOOL supported_3d_sbys; 
    /*!
      TV mode currently used.
      */
    tv_mode_t used_tv_mode;

    MT_U32 brightness_max;
    MT_U8 vsif_timing_mode;

	/*!
	the flag to indicate is the device support 3840x2160p 24Hz
	*/
	mt_u32 supported_3840x2160p_24Hz;
	/*!
	the flag to indicate is the device support 3840x2160p 25Hz
	*/
	mt_u32 supported_3840x2160p_25Hz;
	/*!
	the flag to indicate is the device support 3840x2160p 30Hz
	*/
	mt_u32 supported_3840x2160p_30Hz;
	/*!
	the flag to indicate is the device support 3840x2160p 50Hz
	*/
	mt_u32 supported_3840x2160p_50Hz;
	/*!
	the flag to indicate is the device support 3840x2160p 60Hz
	*/
	mt_u32 supported_3840x2160p_60Hz;
	/*!
	the flag to indicate is the device support 4096x2160p 24Hz
	*/
	mt_u32 supported_4096x2160p_24Hz;
	/*!
	the flag to indicate is the device support 4096x2160p 25Hz
	*/
	mt_u32 supported_4096x2160p_25Hz;
	/*!
	the flag to indicate is the device support 4096x2160p 30Hz
	*/
	mt_u32 supported_4096x2160p_30Hz;
	/*!
	the flag to indicate is the device support 4096x2160p 50Hz
	*/
	mt_u32 supported_4096x2160p_50Hz;
	/*!
	the flag to indicate is the device support 4096x2160p 60Hz
	*/
	mt_u32 supported_4096x2160p_60Hz;
#else
    mt_unf_hdmi_edid_t edid;
/*!
  whether support BT.2020.
  */
    MT_BOOL supported_bt2020;
/*!
  TV mode currently used.
  */
    tv_mode_t used_tv_mode;

    MT_U32 brightness_max;
    MT_U8 vsif_timing_mode;

#endif 
}hdmi_info_t;

/*!
  This structure defines video encoder info.
  */
typedef struct
{
    /*! 
      The brightness in percerntage.
      */
    mt_u8 bright;
    /*! 
      The sature in percerntage.
      */
    mt_u8 sature;
    /*! 
      The contrast in percerntage.
      */
    mt_u8 contrast;
    /*! 
      The gamma pole.
      */
    disp_gamma_pole_t pole;
    /*! 
      The gamma coef.
      */
    mt_u8 coef;
    /*! 
      The gamma enable flag.
      */
    MT_BOOL b_gamma;
    /*! 
      The cvbs DAC.
      */
    cvbs_dac_t cvbs_dac[CVBS_GRP_MAX];
    /*! 
      The component DAC.
      */
    comp_dac_t comp_dac[COMPONENT_GRP_MAX];
    /*! 
      The svideo DAC.
      */
    svideo_dac_t svideo_dac[SVIDEO_GRP_MAX];
} venc_info_t;

#define MAX_SCROLL_NUM 3


typedef struct
{
    mt_u32 sd_wb_addr;
    mt_u32 sd_wb_size;
    mt_u32 sd_wb_field_no;
    mt_u32 av_ap_shared_mem;
    mt_u32 shared_mem_size;
    mt_u32 di_addr;
    mt_u32 di_size;
    MT_BOOL b_wrback_422;
    MT_BOOL b_unuse_prescale;
}DISP_BUF;

//arg struct
struct display_arg {
    mt_u32 screen_width;
    mt_u32 screen_height;
    mt_u32 fmt;
    mt_u32 aspect;
    mt_u8 percent;
    mt_u32 pole;
    mt_u32 color;
    mt_u32 alpha;
    mt_u32 v_aspect;
    mt_u32 p_align;
    mt_u32 grp_id;
    mt_u32 layer;
    mt_u32 b_on;
    mt_u32 ch;
    mt_u8 coef;
    mt_u32 p_size;
    mt_u32 type;
    rect_vsb_t rect;
    rect_vsb_t rect2;
    void *p_region;
    void *p_buf;
    pos_t p_pos;
    DISP_BUF disp_buf;
};

/*!
  for font scroll, jqw
  */
typedef struct scroll_t
{
    mt_u32 speed; //num of pixels need to move each time
    mt_u32 rounds;
    mt_u32 direction; //0:right to left, 1:left to right
    mt_u32 *rgb_text_rgn_arr;
    mt_u32 bmp_id;
    mt_u32 bmp_num;
    void *dst_rgn;
    void *bg_rgn;
    rect_vsb_t dst_rect;
    mt_u32 scroll_id;
    mt_u32 round_id;
    int text_beg;
    MT_BOOL used;
}scroll_info_t;





typedef struct hld_disp
{
    /*!
      The pointer to the private variables and structures.
      */
    void *p_priv;  
    MT_BOOL is_open;
    mt_u32 sem;
}hld_disp_t;


typedef struct mtDISP_LAYERSHOW_S
{
    MT_DRV_DISPLAY_E      enDisp;
    MT_DRV_DISP_LAYER_ID_E    elayer;
    MT_BOOL b_on;
}DISP_LAYERSHOW_S;

typedef struct mtDISP_VDACOUTPUT_S
{
    dac_index_t    dac_id;
    MT_BOOL b_enable;
}DISP_VDACOUTPUT_S;



/*!
  This structure defines the supported trick modes.
  */
typedef enum
{
    /*!
      Fast forward X2 mode
      */
    TM_FFWD_X2,
    /*!
      Slow forward 1/2 mode
      */
    TM_SFWD_X2,
    /*!
      Fast reverse mode
      */
    TM_FREV,
    /*!
      Normal play mode
      */
    TM_NORMAL,
    /*!
      fx4....
      */
    TM_FFWD_X2_MORE,
    /*!
      sfx4....
      */
    TM_SFWD_X2_MORE,
}DISP_TRICK_MODE_E;

/*!
  This structure defines the supported unblank modes.
  */
typedef enum
{
    DISP_UNBLANK_MODE_USER,
    DISP_UNBLANK_MODE_SYNC,
    DISP_UNBLANK_MODE_STABLE,
    DISP_UNBLANK_MODE_FAST,
    DISP_UNBLANK_MODE_BUTT,
}MT_DRV_DISP_UNBLANK_MODE_E;

typedef struct mtDISP_LOWDELAY_ENABLE_S
{
    mt_handle hCast;
    MT_BOOL bEnable;
}DISP_LOWDELAY_ENABLE_S;


#define MT_DISP_VDAC_MAX_NUMBER 4
#define MT_DISP_VDAC_INVALID_ID 0xff
typedef struct mtDRV_DISP_INTF_S
{
    MT_DRV_DISP_INTF_ID_E eID;
    mt_u8 u8VDAC_Y_G;
    mt_u8 u8VDAC_Pb_B;
    mt_u8 u8VDAC_Pr_R;
    MT_BOOL bDacSync;
    vdac_type_t dacMode;
}MT_DRV_DISP_INTF_S;

typedef enum mtDRV_DISP_INTF_DATA_FMT
{
    MT_DRV_DISP_INTF_DATA_FMT_YUV422 = 0,
    MT_DRV_DISP_INTF_DATA_FMT_RGB565,
    MT_DRV_DISP_INTF_DATA_FMT_RGB444,
    MT_DRV_DISP_INTF_DATA_FMT_RGB666,
    MT_DRV_DISP_INTF_DATA_FMT_RGB888,
    MT_DRV_DISP_INTF_DATA_FMT_BUTT
}MT_DRV_DISP_INTF_DATA_FMT_E;

typedef enum mtDRV_DISP_INTF_DATA_WIDTH_E
{
    MT_DRV_DISP_INTF_DATA_WIDTH8 = 0,       /**<8 bits*//**<CNcomment:8位*/
    MT_DRV_DISP_INTF_DATA_WIDTH16,          /**<16 bits*//**<CNcomment:16位*/
    MT_DRV_DISP_INTF_DATA_WIDTH24,          /**<24 bits*//**<CNcomment:24位*/
    MT_DRV_DISP_INTF_DATA_WIDTH_BUTT
}MT_DRV_DISP_INTF_DATA_WIDTH_E;


typedef struct mtDRV_DISP_TIMING_S
{
    mt_u32  u32VFB;
    mt_u32  u32VBB;
    mt_u32  u32VACT;
    mt_u32  u32HFB;
    mt_u32  u32HBB;
    mt_u32  u32HACT;
    mt_u32  u32VPW;
    mt_u32  u32HPW;
    MT_BOOL bIDV;
    MT_BOOL bIHS;
    MT_BOOL bIVS;
    MT_BOOL bClkReversal;
    MT_DRV_DISP_INTF_DATA_WIDTH_E  u32DataWidth;
    MT_DRV_DISP_INTF_DATA_FMT_E eDataFmt;

    MT_BOOL bDitherEnable;
    mt_u32  u32ClkPara0;
    mt_u32  u32ClkPara1;

    MT_BOOL bInterlace;
    mt_u32  u32PixFreq;
    mt_u32  u32VertFreq;
    mt_u32  u32AspectRatioW;
    mt_u32  u32AspectRatioH;

    MT_BOOL u32bUseGamma;

    mt_u32  u32Reserve0;
    mt_u32  u32Reserve1;
}MT_DRV_DISP_TIMING_S;

typedef struct mtDRV_DISP_COLOR_S
{
    mt_u8 u8Red;
    mt_u8 u8Green;
    mt_u8 u8Blue;
}MT_DRV_DISP_COLOR_S;

typedef enum mtDISP_STEREO_E
{
    DISP_STEREO_NONE = 0,
    DISP_STEREO_FPK,
    DISP_STEREO_SBS_HALF,
    DISP_STEREO_TAB,
    DISP_STEREO_FIELD_ALTE,
    DISP_STEREO_LINE_ALTE,
    DISP_STEREO_SBS_FULL,
    DISP_STEREO_L_DEPT,
    DISP_STEREO_L_DEPT_G_DEPT,
    DISP_STEREO_BUTT
}MT_DRV_DISP_STEREO_E;

typedef struct mtDISP_CROP_S
{
    mt_u32 u32LeftOffset;
    mt_u32 u32TopOffset;
    mt_u32 u32RightOffset;
    mt_u32 u32BottomOffset;
}MT_DRV_DISP_CROP_S;

typedef struct mtDRV_DISP_OFFSET_S
{
    mt_u32 u32Left;    /*left offset */
    mt_u32 u32Top;     /*top offset */
    mt_u32 u32Right;   /*right offset */
    mt_u32 u32Bottom;  /*bottom offset */
}MT_DRV_DISP_OFFSET_S;

typedef struct mtDISP_DISPLAY_INFO_S
{
    MT_BOOL bIsMaster;
    MT_BOOL bIsSlave;
    MT_DRV_DISPLAY_E enAttachedDisp;

    MT_DRV_DISP_STEREO_E eDispMode;
    MT_BOOL bRightEyeFirst;
    MT_BOOL bInterlace;
    MT_BOOL bIsBottomField;
    mt_u32 u32Vline;

    /*just a back of display setting, for virt screen and  offset set.*/
    mt_rect_s stVirtaulScreen;
    MT_DRV_DISP_OFFSET_S stOffsetInfo;
    mt_rect_s stFmtResolution;
    mt_rect_s stPixelFmtResolution;

    MT_DRV_ASPECT_RATIO_S stAR;
    mt_u32 u32RefreshRate;
    MT_DRV_COLOR_SPACE_E eColorSpace;

    mt_u32 u32Bright;
    mt_u32 u32Hue;
    mt_u32 u32Satur;
    mt_u32 u32Contrst;

    mt_u32 u32Kr;
    mt_u32 u32Kg;
    mt_u32 u32Kb;
}MT_DISP_DISPLAY_INFO_S;

/*************** IP protect ***************/
//Macrovision
typedef enum mtDRV_DISP_MACROVISION_E
{
    MT_DRV_DISP_MACROVISION_TYPE0,
    MT_DRV_DISP_MACROVISION_TYPE1,
    MT_DRV_DISP_MACROVISION_TYPE2,
    MT_DRV_DISP_MACROVISION_TYPE3,
    MT_DRV_DISP_MACROVISION_CUSTOMER_01,
    MT_DRV_DISP_MACROVISION_CUSTOMER_02,
    MT_DRV_DISP_MACROVISION_BUTT
}MT_DRV_DISP_MACROVISION_E;


//CGMS-A
typedef enum mtDRV_DISP_CGMSA_TYPE_E
{
    MT_DRV_DISP_CGMSA_A = 0x00,
    MT_DRV_DISP_CGMSA_B,
    MT_DRV_DISP_CGMSA_TYPE_BUTT
}MT_DRV_DISP_CGMSA_TYPE_E;

typedef enum mtDRV_DISP_CGMSA_MODE_E
{
    MT_DRV_DISP_CGMSA_COPY_FREELY  = 0,
    MT_DRV_DISP_CGMSA_COPY_NO_MORE = 0x01,
    MT_DRV_DISP_CGMSA_COPY_ONCE    = 0x02,
    MT_DRV_DISP_CGMSA_COPY_NEVER   = 0x03,

    MT_DRV_DISP_CGMSA_MODE_BUTT
}MT_DRV_DISP_CGMSA_MODE_E;

typedef struct mtDRV_DISP_CGMSA_CFG_S
{
    MT_BOOL           bEnable;
    MT_DRV_DISP_CGMSA_TYPE_E  enType;
    MT_DRV_DISP_CGMSA_MODE_E  enMode;
}MT_DRV_DISP_CGMSA_CFG_S;


/*************** about VBI ***************/
typedef enum mtDRV_DISP_VBI_TYPE_E
{
    MT_DRV_DISP_VBI_TTX=0,
    MT_DRV_DISP_VBI_CC,
    MT_DRV_DISP_VBI_VCHIP,
    MT_DRV_DISP_VBI_WSS,
    MT_DRV_DISP_VBI_VPS,                         
    MT_DRV_DISP_VBI_CGMS_A, 
    MT_DRV_DISP_VBI_CC_PES,
    MT_DRV_DISP_VBI_TTX_ES,
    MT_DRV_DISP_VBI_TYPE_BUTT,
}MT_DRV_DISP_VBI_TYPE_E;

typedef struct mtDRV_DISP_VBI_CFG_S
{
    MT_DRV_DISP_VBI_TYPE_E eType;
    mt_u32  u32InBufferSize;
    mt_u32  u32WorkBufferSize;
}MT_DRV_DISP_VBI_CFG_S;

typedef struct mtDRV_DISP_TTX_DATA_S
{
    mt_u8   *pu8DataAddr;
    mt_u32   u32DataLen;
} MT_DRV_DISP_TTX_DATA_S;

typedef struct mtDRV_DISP_VBI_DATA_S
{
    MT_DRV_DISP_VBI_TYPE_E eType;
    mt_u8  *pu8DataAddr;
    mt_u32  u32DataLen;
}MT_DRV_DISP_VBI_DATA_S;

typedef struct mtDRV_DISP_WSS_DATA_S
{
    MT_BOOL bEnable;
    mt_u16  u16Data;
}MT_DRV_DISP_WSS_DATA_S;

typedef struct mtDRV_DISP_HDMI_S
{
    MT_BOOL bHsyncNegative;
    MT_BOOL bVsyncNegative;
    MT_BOOL bDvNegative;
    mt_s32 s32SyncType;

    mt_s32 s32CompNumber;  //0,10bit; 1, 20bit; 2, 30bit
    mt_s32 s32DataFmt;     //0:YCbCr;1: RGB
}MT_DRV_DISP_HDMI_S;

typedef struct mtDRV_DISP_COLOR_SETTING_S
{
    MT_DRV_COLOR_SPACE_E enInCS;
    MT_DRV_COLOR_SPACE_E enOutCS;

    mt_u32 u32Bright;      //bright adjust value,range[0,100],default setting 50;
    mt_u32 u32Hue;         //hue adjust value,range[0,100],default setting 50;
    mt_u32 u32Satur;       //saturation adjust value,range[0,100],default setting 50;
    mt_u32 u32Contrst;     //contrast adjust value,range[0,100],default setting 50;

    mt_u32 u32Kr;
    mt_u32 u32Kg;
    mt_u32 u32Kb;
    MT_BOOL bGammaEnable;
    MT_BOOL bUseCustGammaTable;

    mt_void *pReserve;    /* must be 0 */
    mt_u32   u32Reserve;  /* must be 0 */
}MT_DRV_DISP_COLOR_SETTING_S;

typedef struct mtDRV_DISP_INIT_PARAM_S
{
    mt_u32                u32Version;
    //MT_BOOL               bSelfStart;
    MT_BOOL               bIsMaster;
    MT_BOOL               bIsSlave;
    MT_DRV_DISPLAY_E      enAttachedDisp;
    MT_DRV_DISP_FMT_E     enFormat;
    mt_u32                u32Brightness;
    mt_u32                u32Contrast;
    mt_u32                u32Saturation;
    mt_u32                u32HuePlus;
    MT_BOOL               bGammaEnable;
    mt_u32                u32VirtScreenWidth;
    mt_u32                u32VirtScreenHeight;
    MT_DRV_DISP_OFFSET_S  stOffsetInfo;
    MT_DRV_DISP_COLOR_S   stBgColor;
    MT_BOOL               bCustomRatio;
    mt_u32                u32CustomRatioWidth;
    mt_u32                u32CustomRatioHeight;
    MT_DRV_DISP_INTF_S    stIntf[MT_DRV_DISP_INTF_ID_MAX];
    MT_DRV_DISP_TIMING_S  stDispTiming;

    mt_u32  u32Reseve;
    mt_void *pRevData;
}MT_DRV_DISP_INIT_PARAM_S;

typedef struct mtDRV_DISP_SETTING_S
{
    mt_u32  u32BootVersion;
    MT_BOOL bGetPDMParam;
    //MT_BOOL bSelfStart;
    MT_BOOL bIsMaster;
    MT_BOOL bIsSlave;
    MT_DRV_DISPLAY_E enAttachedDisp;

    /* output format */
    MT_DRV_DISP_STEREO_E eDispMode;
    MT_DRV_DISP_FMT_E enFormat;
    MT_DRV_DISP_TIMING_S stCustomTimg;

    /* about color */
    MT_DRV_DISP_COLOR_SETTING_S stColor;

    /* background color */
    MT_DRV_DISP_COLOR_S stBgColor;

    /*just for screen ajust.*/
    mt_rect_s stVirtaulScreen;
    MT_DRV_DISP_OFFSET_S stOffsetInfo;

    /* interface setting */
    mt_u32 u32IntfNumber;
    MT_DRV_DISP_INTF_S stIntf[MT_DRV_DISP_INTF_ID_MAX];

    mt_u32 u32LayerNumber;
    MT_DRV_DISP_LAYER_E enLayer[MT_DRV_DISP_LAYER_BUTT]; /* Z-order is from bottom to top */

    MT_BOOL bCustomRatio;
    mt_u32 u32CustomRatioWidth;
    mt_u32 u32CustomRatioHeight;

    mt_u32  u32Reseve;
    mt_void *pRevData;
}MT_DRV_DISP_SETTING_S;

typedef struct tagMT_DRV_VDAC_STATE_S
{
    MT_BOOL bDACPlugIn;
    MT_BOOL bDACDetectEn;
}MT_DRV_VDAC_STATE_S;

typedef struct tagMT_DRV_VDAC_ATTR_S
{
    MT_BOOL bDACDetEn;
    MT_DRV_VDAC_STATE_S stDACState[4];
}MT_DRV_VDAC_ATTR_S;

#define DEF_MT_DRV_DISP_MIRROR_BUFFER_MAX_NUMBER 16

typedef struct mtDRV_DISP_CAST_CFG_S
{
    /* frame config */
    MT_DRV_PIX_FORMAT_E eFormat; /* Support ... */
    mt_u32 u32Width;
    mt_u32 u32Height;

    /* buffer config */
    mt_u32  u32BufNumber; /* not more than MT_DISP_MIRROR_BUFFER_MAX_NUMBER */

    MT_BOOL bUserAlloc;  /* TRUE: user alloc buffers; FALSE: enDisp alloc buffers */
    MT_BOOL bLowDelay;

    mt_u32 u32BufSize;    /* every buffer size in Byte */
    mt_u32 u32BufStride;  /* only for 'bUserAlloc = TRUE' */
    mt_u32 u32BufPhyAddr[DEF_MT_DRV_DISP_MIRROR_BUFFER_MAX_NUMBER]; /* only for 'bUserAlloc = TRUE' */
} MT_DRV_DISP_CAST_CFG_S;


/*************** about alg ***************/
typedef enum mtDRV_DISP_ALG_TYPE_E
{
    MT_DRV_DISP_ALG_DEI = 0,
    MT_DRV_DISP_ALG_ACC,
    MT_DRV_DISP_ALG_ACM,
    MT_DRV_DISP_ALG_SHARPNESS,
    MT_DRV_DISP_ALG_DNR,
    MT_DRV_DISP_ALG_TYPE_BUTT
}MT_DRV_DISP_ALG_TYPE_E;

typedef struct mtDRV_DISP_ALG_CFG_S
{
    MT_DRV_DISP_ALG_TYPE_E eType;
    MT_BOOL bEnable;

    mt_u32   u32Reserved;
    mt_void *pPrivate;
}MT_DRV_DISP_ALG_CFG_S;


#define DEF_MT_DRV_DISP_MAX_LAYER_NUMBER 8

typedef struct mtDRV_DISP_VERSION_S
{
    mt_u32 u32VersionPartL;
    mt_u32 u32VersionPartH;
}MT_DRV_DISP_VERSION_S;

#define DEF_DISP_CALLBACK_MAX_EVENT  8

typedef enum mtDRV_DISP_CALLBACK_TYPE_E
{
    /* 中断发起位置，以行有效区为参考，0为第一行，100为最后一行结束 */
    MT_DRV_DISP_C_TYPE_NONE = 0,
    //MT_DRV_DISP_C_SHOW_MODE,
    MT_DRV_DISP_C_INTPOS_0_PERCENT,
    MT_DRV_DISP_C_INTPOS_90_PERCENT,
    MT_DRV_DISP_C_INTPOS_100_PERCENT,
    MT_DRV_DISP_C_DHD0_WBC,
    MT_DRV_DISP_C_GFX_WBC,
    MT_DRV_DISP_C_REG_UP,
    MT_DRV_DISP_C_TYPE_BUTT
}MT_DRV_DISP_CALLBACK_TYPE_E;

typedef enum mtDRV_DISP_CALLBACK_EVENT_E
{
    MT_DRV_DISP_C_EVET_NONE = 0,
    MT_DRV_DISP_C_PREPARE_CLOSE,
    //MT_DRV_DISP_C_CLOSE            = 0x2,
    //MT_DRV_DISP_CALLBACK_PREPARE_TO_OPEN,
    MT_DRV_DISP_C_OPEN,

    MT_DRV_DISP_C_PREPARE_TO_PEND  = 0x10,
    //MT_DRV_DISP_C_PEND             = 0x20,
    //MT_DRV_DISP_C_PREPARE_TO_RESUME,
    MT_DRV_DISP_C_RESUME,

    MT_DRV_DISP_C_DISPLAY_SETTING_CHANGE = 0x20,

    MT_DRV_DISP_C_VT_INT = 0x100,
    MT_DRV_DISP_C_EVENT_BUTT
}MT_DRV_DISP_CALLBACK_EVENT_E;

typedef enum mtDRV_DISP_FIELD_FLAG_E
{
    MT_DRV_DISP_FIELD_PROGRESSIVE = 0,
    MT_DRV_DISP_FIELD_TOP,
    MT_DRV_DISP_FIELD_BOTTOM,
    MT_DRV_DISP_FIELD_FLAG_BUTT
}MT_DRV_DISP_FIELD_FLAG_E;

typedef struct mtDRV_DISP_CALLBACK_INFO_S
{
    MT_DRV_DISP_CALLBACK_EVENT_E eEventType;
    MT_DRV_DISP_FIELD_FLAG_E eField;

    MT_DISP_DISPLAY_INFO_S stDispInfo;
}MT_DRV_DISP_CALLBACK_INFO_S;

typedef struct mtDRV_DISP_Cast_Attr_S
{
    mt_s32  s32Width;
    mt_s32  s32Height;
}MT_DRV_DISP_Cast_Attr_S;
typedef struct mtDRV_DISP_CALLBACK_S
{
    mt_void (* pfDISP_Callback)(mt_handle hDst, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo);
    mt_handle hDst;
}MT_DRV_DISP_CALLBACK_S;

typedef enum mtDRV_DISP_PPMODE_E
{
    MT_DRV_DISP_PPMODE_NONE = 0,
    MT_DRV_DISP_PPMODE_TEST,
    MT_DRV_DISP_PPMODE_STANDARD,         /**<standard pass AE freq response*/
    MT_DRV_DISP_PPMODE_DEFAULT,                 /**<montage prefered,look like mstar*/
    MT_DRV_DISP_PPMODE_VIVID,                       /**<more like ali*/
    MT_DRV_DISP_PPMODE_BUTT
}MT_DRV_DISP_PPMODE_E;

typedef struct mtDISP_PP_S
{
    MT_DRV_DISPLAY_E      enDisp;
    MT_DRV_DISP_PPMODE_E  enPPMode;
}DISP_PP_S;

typedef enum mtDRV_DISP_HDMI_MODE_E
{
    MT_DRV_DISP_HDMI_MODE_SDR = 0,              /**<hdmi mode: SDR*//**<CNcomment: hdmi 模式: SDR*/
    MT_DRV_DISP_HDMI_MODE_HDR10,               /**<hdmi mode: HDR10*//**<CNcomment: hdmi 模式: HDR10*/
    MT_DRV_DISP_HDMI_MODE_HLG,                   /**<hdmi mode: HLG*//**<CNcomment:hdmi 模式: HLG*/
    MT_DRV_DISP_HDMI_MODE_AUTO,                 /**<hdmi mode: AUTO*//**<CNcomment:hdmi 模式: AUTO*/
    MT_DRV_DISP_HDMI_MODE_BUTT
}MT_DRV_DISP_HDMI_MODE_E;

typedef enum mtDRV_DISP_SD_ENC_CFG_E
{
    DISP_SD_ENC_MODE = 0,
    DISP_SD_ENC_CURVE,
    DISP_SD_ENC_DELAY,
    DISP_SD_ENC_BLANK_P,
    DISP_SD_ENC_BLANK_N,
    DISP_SD_ENC_CBLANK,
    DISP_SD_ENC_VDAC_PCARRY,
    DISP_SD_ENC_VDAC_NCARRY,
    DISP_SD_ENC_CFIG1,
    DISP_SD_ENC_CFIG2,
    DISP_SD_ENC_CFIG3,
    DISP_SD_ENC_CFIG4,
    DISP_SD_ENC_CFIG5,
    DISP_SD_ENC_CFIG6,
    DISP_SD_ENC_CFIG7,
    DISP_SD_ENC_SET,
    DISP_SD_ENC_DAC0_PARA,
    DISP_SD_ENC_DRCOEF,
    DISP_SD_ENC_DBCOEF,
    DISP_SD_ENC_CFIG8,
    DISP_SD_ENC_SCARRYDR,
    DISP_SD_ENC_SCARRYDB,
    DISP_SD_ENC_PINCREMENT,
    DISP_SD_ENC_NINCREMENT,
    DISP_SD_ENC_GCONTROL,
    DISP_SD_ENC_DACNUM,
    DISP_SD_ENC_INSTM,
    DISP_SD_ENC_COMPRESS,
    DISP_SD_ENC_SET1,
    DISP_SD_ENC_SET2,
    DISP_SD_ENC_SET3,
    DISP_SD_ENC_LUM_DLY_108M,
    DISP_SD_ENC_COEF11,
    DISP_SD_ENC_COEF10,
    DISP_SD_ENC_COEF9,
    DISP_SD_ENC_COEF8,
    DISP_SD_ENC_COEF7,
    DISP_SD_ENC_COEF6,
    DISP_SD_ENC_COEF5,
    DISP_SD_ENC_COEF4,
    DISP_SD_ENC_COEF3,
    DISP_SD_ENC_COEF2,
    DISP_SD_ENC_COEF1,
    DISP_SD_ENC_COEF0,
    DISP_SD_ENC_COEF_C_LUT3_U,
    DISP_SD_ENC_COEF_C_LUT2_U,
    DISP_SD_ENC_COEF_C_LUT1_U,
    DISP_SD_ENC_COEF_C_LUT0_U,
    DISP_SD_ENC_COEF_C_LUT3_V,
    DISP_SD_ENC_COEF_C_LUT2_V,
    DISP_SD_ENC_COEF_C_LUT1_V,
    DISP_SD_ENC_COEF_C_LUT0_V,
    DISP_SD_ENC_COEF_LUT3_Y,
    DISP_SD_ENC_COEF_LUT2_Y,
    DISP_SD_ENC_COEF_LUT1_Y,
    DISP_SD_ENC_COEF_LUT0_Y,
    DISP_SD_ENC_SIGN_CTL,
    DISP_SD_ENC_DAC_OFFSET,
    DISP_SD_ENC_DAC123_PARA,
    DISP_SD_ENC_DAC0_ANTI_COEF1_0,
    DISP_SD_ENC_DAC0_ANTI_COEF3_2,
    DISP_SD_ENC_DAC123_ANTI_COEF1_0,
    DISP_SD_ENC_DAC123_ANTI_COEF3_2,
    DISP_SD_ENC_BUTT
}MT_DRV_DISP_SD_ENC_CFG_E;

typedef struct mtDISP_SD_ENC_PQ_PARA_S
{
    mt_u32		item;
    mt_u32		val;
}DISP_SD_ENC_PQ_PARA_S;


typedef struct
{
    /*! 
      The left x coordinate. 
      */
    mt_u16 m_left;
    /*! 
      The right x coordinate. 
      */
    mt_u16 m_right;
    /*! 
      The top y coordinate. 
      */
    mt_u16 m_top;
    /*! 
      The bottom y coordinate. 
      */
    mt_u16 m_bottom;
    /*! 
      Region alpha 0. 
      */
    mt_u8 m_alpha0;
    /*! 
      Region alpha 1. ONLY aRGB1555 can select alpha 1. 
      */
    mt_u8 m_alpha1;
    /*! 
      The width of the region. 
      */
    mt_u16 m_pitch;
    /*! 
      TRUE to display the region. otherwise to hide it. 
      */
    MT_BOOL m_enable;
    /*! 
      TRUE means this region contains a palette
      */
    MT_BOOL m_palette;
    /*! 
      The alpha mode of this region: region or plane. 
      */
    mt_u8 m_alphamode;
    /*!
      If current region is followed by another region.
      */
    MT_BOOL m_follow;
    /*!
      The start address of the data
      */
    mt_u32 m_start_addr;
    /*!
      The start address of the data of vertical scaler in graphic engine
      */
    mt_u32 m_start_addr_vs;
    /*!
      The start address of the uv data
      */
    mt_u32 m_uv_start_addr;
    /*! 
      The mode of RGB true color. 
      */
    mt_u8 m_truemode;
    /*! 
      The mode for indexed color. 
      */
    mt_u8 m_clutmode;
    /*! 
      The color space. 
      */
    mt_u8 m_colormode;
    /*! 
      The osd endian mode . 
      */
    mt_u8 m_endianmode;
    /*! 
      The palette endian mode . 
      */
    mt_u8 m_palette_endianmode;
    /*! 
      The alpha position. 
      */
    mt_u8 m_alpha_pos;
    /*! 
      The alpha blending mode. 
      */
    mt_u8 m_alpha_blendmode;
    /*! 
      odd duplication trigger
      */
    MT_BOOL m_odd_only;
    /*! 
      The start address of next region if it exist.
      */
    mt_u32 m_rgnaddr_next;
    /*! 
      The line stride in memory, in pixel
      */
    mt_u16 m_stride;
    /*! 
      TRUE to enable the semi-planar. otherwise to disable it.  
      */
    MT_BOOL m_semi_enable;
    /*! 
      UV exchange. 
      */
    MT_BOOL m_uv_change;
    /*! 
      The semi-planar format . 
      */
    mt_u8 m_semi_format;

}osd_header_info_s;

typedef enum mtDRV_DISP_DUMP_SCALER_SOURCE_E
{
    MT_DRV_DISP_DSCALER_IN_HD_SCREEN = 0,
    MT_DRV_DISP_DSCALER_IN_HD_VIDEO,
    MT_DRV_DISP_DSCALER_IN_SD_SCREEN,
    MT_DRV_DISP_DSCALER_IN__BUTT
}MT_DRV_DISP_DUMP_SCALER_SOURCE_E;

typedef struct mtDISP_DUMP_SCALER_PARA_S
{
    MT_BOOL                             b_enable;
    MT_DRV_DISP_DUMP_SCALER_SOURCE_E    source;
    MT_DRV_DISP_LAYER_ID_E              dst_layer;
    mt_u32		                        out_width;
    mt_u32		                        out_height;
}MT_DRV_DISP_DUMP_SCALER_PARA_S;


mt_s32 MT_DRV_DISP_Init(mt_void);
mt_s32 MT_DRV_DISP_DeInit(mt_void);
mt_s32 MT_DRV_DISP_Attach(MT_DRV_DISPLAY_E enDstDisp, MT_DRV_DISPLAY_E enSlave);
mt_s32 MT_DRV_DISP_Detach(MT_DRV_DISPLAY_E enDstDisp, MT_DRV_DISPLAY_E enSlave);
mt_s32 MT_DRV_DISP_SetFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_FMT_E enEncodingFormat);
mt_s32 MT_DRV_DISP_GetFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_FMT_E *penFormat);
mt_s32 MT_DRV_DISP_SetCustomTiming(MT_DRV_DISPLAY_E enDisp,  MT_DRV_DISP_TIMING_S *pstTiming);
mt_s32 MT_DRV_DISP_GetCustomTiming(MT_DRV_DISPLAY_E enDisp,  MT_DRV_DISP_TIMING_S *pstTiming);
mt_s32 MT_DRV_DISP_AddIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf);
mt_s32 MT_DRV_DISP_DelIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf);

mt_s32 MT_DRV_DISP_Open(MT_DRV_DISPLAY_E enDisp);

mt_s32 MT_DRV_DISP_Close(MT_DRV_DISPLAY_E enDisp);
mt_s32 MT_DRV_DISP_SetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable);
mt_s32 MT_DRV_DISP_GetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL *pbEnable);
mt_s32 MT_DRV_DISP_SetRightEyeFirst(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable);

mt_s32 MT_DRV_DISP_SetBgColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_S *pstBgColor);
mt_s32 MT_DRV_DISP_GetBgColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_S *pstBgColor);
mt_s32 MT_DRV_DISP_SetColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_SETTING_S *pstCS);
mt_s32 MT_DRV_DISP_GetColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_SETTING_S *pstCS);

mt_s32 MT_DRV_DISP_SetAspectRatio(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Ratio_h, mt_u32 u32Ratio_v);
mt_s32 MT_DRV_DISP_GetAspectRatio(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Ratio_h, mt_u32 *pu32Ratio_v);

mt_s32 MT_DRV_DISP_SetLayerZorder(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_ZORDER_ABS_E enZFlag);
mt_s32 MT_DRV_DISP_GetLayerZorder(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Zorder);

mt_s32 MT_DRV_DISP_CreateCast (MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CAST_CFG_S * pstCfg, mt_handle *phCast);
mt_s32 MT_DRV_DISP_DestroyCast(mt_handle hCast);

mt_s32 MT_DRV_DISP_SetCastEnable(mt_handle hCast, MT_BOOL bEnable);
mt_s32 MT_DRV_DISP_GetCastEnable(mt_handle hCast, MT_BOOL *pbEnable);

mt_s32 MT_DRV_DISP_AcquireCastFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame);
mt_s32 MT_DRV_DISP_ReleaseCastFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame);
mt_s32 MT_DRV_DISP_ExternlAttach(mt_handle hCast, mt_handle hSink);
mt_s32 MT_DRV_DISP_ExternlDetach(mt_handle hCast, mt_handle hSink);

mt_s32 MT_DRV_DISP_GetInitFlag(MT_BOOL *pbInited);
mt_s32 MT_DRV_DISP_GetVersion(MT_DRV_DISP_VERSION_S *pstVersion);
MT_BOOL MT_DRV_DISP_IsOpened(MT_DRV_DISPLAY_E enDisp);
mt_s32 MT_DRV_DISP_GetSlave(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISPLAY_E *penSlave);
mt_s32 MT_DRV_DISP_GetMaster(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISPLAY_E *penMaster);
mt_s32 MT_DRV_DISP_GetDisplayInfo(MT_DRV_DISPLAY_E enDisp, MT_DISP_DISPLAY_INFO_S *pstInfo);
mt_s32 MT_DRV_DISP_Process(mt_u32 cmd, mt_void *arg);
mt_s32 MT_DRV_DISP_RegCallback(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
        MT_DRV_DISP_CALLBACK_S *pstCallback);
mt_s32 MT_DRV_DISP_UnRegCallback(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
        MT_DRV_DISP_CALLBACK_S *pstCallback);
mt_s32 MT_DRV_DISP_UpdatePqData(mt_u32 u32UpdateType,PQ_PARAM_S * pstPqParam);

mt_s32 DRV_DISP_AvsyncGetVptsInfo(void *pstDispBP, void *pInfo);
mt_s32 DRV_DISP_AvsyncGetInputRate(void *pstDispBP, MT_U32 *pInputRate);
mt_s32 DRV_DISP_AvsyncSetSkipFrame(MT_U32 skip_num);
mt_s32 DRV_DISP_AvsyncSetRepeatFrame(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
