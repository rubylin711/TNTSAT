/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef MT_COMMOM_H
#define MT_COMMOM_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DISP_ON_AP_LINUX
#include "mt_drv_disp.h"
#include "drv_disp_osal.h"
#define DF_PRINT printk
#elif defined FW_INTERNAL_TEST
extern int fw_printf(const char *p_fmt, ...);
#define DF_PRINT fw_printf
#else
#pragma message("Error: DF_PRINT not defined!!")
#define DF_PRINT
#endif

#include "mt_drv_video.h"

extern MT_U8 g_df_log_level;

#define DF_LOG_PDD(fmt, ...) \
  do { \
    if(g_df_log_level==9) \
    { \
      DF_PRINT(fmt, ## __VA_ARGS__); \
    } \
  }while(0)

#define DF_LOG_HDR(fmt, ...) \
  do { \
    if(g_df_log_level==8) \
    { \
      DF_PRINT(fmt, ## __VA_ARGS__); \
    } \
  }while(0)

#define DF_LOG_PTS(fmt, ...) \
  do { \
    if(g_df_log_level==7) \
    { \
      DF_PRINT(fmt, ## __VA_ARGS__); \
    } \
  }while(0)

#define DF_LOG_SCALE(fmt, ...) \
  do { \
    if(g_df_log_level==6) \
    { \
      DF_PRINT(fmt, ## __VA_ARGS__); \
    } \
  }while(0)

#define DF_LOG_QUEUE(fmt, ...) \
  do { \
    if(g_df_log_level==5) \
    { \
      DF_PRINT(fmt, ## __VA_ARGS__); \
    } \
  }while(0)

#define DF_LOG_DEBUG(fmt, ...) \
  do { \
    if(g_df_log_level==4) \
    { \
      DF_PRINT(fmt, ## __VA_ARGS__); \
    } \
  }while(0)

#define DF_LOG_INFO(fmt, ...) \
  do { \
    if(g_df_log_level==3) \
    { \
      DF_PRINT(fmt, ## __VA_ARGS__); \
    } \
  }while(0)

#define DF_LOG_WARN(fmt, ...) \
  do { \
    if(g_df_log_level>=2) \
    { \
      DF_PRINT(fmt, ## __VA_ARGS__); \
    } \
  }while(0)

#define DF_LOG_ERR(fmt, ...) \
  do { \
    if(g_df_log_level>=1) \
    { \
      DF_PRINT(fmt, ## __VA_ARGS__); \
    } \
  }while(0)


#define MT_CHIP_REV_SYMPHONY1 1
#define MT_CHIP_REV_SYMPHONY2 2
#define MT_CHIP_REV_SYMPHONY4 3
#define MT_CHIP_REV_SYMPHONY6 4

#define DF_VIDEO_720_FULLSCR_WIDTH 1280
#define DF_VIDEO_720_FULLSCR_HEIGHT 720
#define DF_VIDEO_1080_FULLSCR_WIDTH 1920
#define DF_VIDEO_1080_FULLSCR_HEIGHT 1080
#define DF_VIDEO_3840X2160_FULLSCR_WIDTH 3840
#define DF_VIDEO_3840X2160_FULLSCR_HEIGHT 2160
#define DF_VIDEO_4096X2160_FULLSCR_WIDTH 4096
#define DF_VIDEO_4096X2160_FULLSCR_HEIGHT 2160
#define DF_VIDEO_SD_FULLSCR_WIDTH 720
#define DF_VIDEO_SD_FULLSCR_HEIGHT_PAL 576
#define DF_VIDEO_SD_FULLSCR_HEIGHT_HALF_PAL 288
#define DF_VIDEO_SD_FULLSCR_HEIGHT_NTSC 480
#define DF_VIDEO_SD_FULLSCR_HEIGHT_HALF_NTSC 240

//ALIGN
#define MT_16BYTE_ALIGN 16
#define MT_32BYTE_ALIGN 32
#define MT_128BYTE_ALIGN 128

//#define MT_UHD_WIDTH 3840
#define MT_UHD_WIDTH_LARGEST 4096
//#define MT_UHD_HEIGHT 2160

//#define MT_HD_WIDTH 1920
//#define MT_HD_HEIGHT 1080
#define MT_HD_HEIGHT_LARGEST 1088

#define MT_SD_WIDTH 1280
#define MT_SD_HEIGHT 720

#define MT_MAX_PIC_WIDTH MT_UHD_WIDTH_LARGEST
#define MT_MAX_PIC_HEIGHT MT_HD_HEIGHT_LARGEST

//#define MT_U32_FF 0xFFFFFFFF

typedef MT_BOOL MT_DF_BOOL;
#define MT_DF_FALSE MT_FALSE
#define MT_DF_TRUE MT_TRUE

#define MT_RET MT_S32
#define MT_DF_SUCCESS 0
#define MT_DF_FAILURE (-1)

#define MT_DF_NULL 0L

/*!
   INT(x/y+ 1) * y
  */
#define ROUNDUP(x, y)           (((x) + (y) - 1) & ~((y) - 1))
/*!
   INT(x/y) * y
  */
#define ROUNDDOWN(x, y)         ((x) & ~((y) - 1))

#define MT_DF_DISP_ASSERT(X)            \
    \
do                         \
    \
{                          \
    if (!(X))                 \
        return MT_DF_FAILURE; \
    \
}                          \
    while (0)

#define DISP_CHECK_NULL_RETURN(p) \
    \
do                         \
    {                             \
    if (!(p)) {               \
        return;               \
    }                         \
    \
}                          \
    while (0)

#define NODE_TO_FRAME(PNODE,PFRAME)\
do\
{\
    if(PNODE==MT_DF_NULL)\
    {\
        PFRAME=MT_DF_NULL;\
    }\
    else\
    {\
        PFRAME=(MT_DF_VIDEO_FRAME_S *)PNODE->u32Data;\
    }\
}while(0)

#define DISP_CHECK_NULL_RETURN_NULL(p)                                                \
    do {                                                                              \
      if (!p) {                                                                     \
        DF_LOG_DEBUG("FUNC %s Line%d input NULL Pointer!\n", __FUNCTION__, __LINE__); \
        return MT_DF_NULL;                                                        \
      }                                                                             \
    } while (0)

#define DISP_CHECK_NULL_RETURN_FAILURE(p)                                             \
    do {                                                                              \
      if (!p) {                                                                     \
        DF_LOG_DEBUG("FUNC %s Line%d input NULL Pointer!\n", __FUNCTION__, __LINE__); \
        return MT_DF_FAILURE;                                                     \
      }                                                                             \
    } while (0)

#define DISP_CHECK_NULL_RETURN_FALSE(p)                                               \
    do {                                                                              \
      if (!p) {                                                                     \
        DF_LOG_DEBUG("FUNC %s Line%d input NULL Pointer!\n", __FUNCTION__, __LINE__); \
        return MT_DF_FALSE;                                                       \
      }                                                                             \
    } while (0)


void DF_Get_VoutSize(disp_sys_t out_fmt, MT_U32 *p_width, MT_U32 *p_height);

typedef enum
{
  HD_ADV_CSC_BT2020_TO_BT709 = 0,  //SDR
  HD_ADV_CSC_HDR10_TO_SDR    = 1,
  HD_ADV_CSC_HLG_TO_SDR      = 2,
  HD_ADV_CSC_HDR_BT2020_TO_BT709 = 3,
  HD_ADV_CSC_HLG_BT2020_TO_BT709 = 4,
  HD_ADV_CSC_HDR_BT709_TO_BT2020 = 5 //SL SDR 709 => SL HDR 709 => SL HDR2020
}hd_adv_csc_mode_t;

typedef enum
{
  OSD_NEW_CSC_SDR_TO_HDR10    = 0,
  OSD_NEW_CSC_SDR_TO_HLG      = 1
}osd_new_csc_mode_t;


typedef enum
{
  SD_NEW_CSC_HDR10_TO_SDR    = 0,
  SD_NEW_CSC_HLG_TO_SDR      = 1
}sd_new_csc_mode_t;

#ifdef __cplusplus
}
#endif
#endif

