/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/******************************************************************************
  File Name     : mt_drv_pq.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/04
  Description   :
  History       :
  1.Date        : 2015/12/04
    Author      :
    Modification: Created file

******************************************************************************/

#ifndef __MT_DRV_PQ_V3_H__
#define __MT_DRV_PQ_V3_H__

#include <linux/ioctl.h>
#include "mt_module.h"

#include "mt_type.h"
#include "mt_debug.h"
#include "drv_pq_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


//#define MT_FATAL_PQ(fmt...) MT_TRACE(MT_LOG_LEVEL_FATAL, MT_ID_PQ, fmt)
//#define MT_ERR_PQ(fmt...)   MT_TRACE(MT_LOG_LEVEL_ERROR, MT_ID_PQ, fmt)
//#define MT_WARN_PQ(fmt...)  MT_TRACE(MT_LOG_LEVEL_WARNING, MT_ID_PQ, fmt)
//#define MT_INFO_PQ(fmt...)  MT_TRACE(MT_LOG_LEVEL_INFO, MT_ID_PQ, fmt)
//#define MT_DEBUG_PQ(fmt...) MT_TRACE(MT_LOG_LEVEL_DBG, MT_ID_PQ, fmt)

/* 卖场模式 */
typedef enum mtPQ_DEMO_E
{
    MT_PQ_DEMO_DBDR = 0  ,
    MT_PQ_DEMO_NR        ,
    MT_PQ_DEMO_SHARPNESS ,
    MT_PQ_DEMO_DCI       ,
    MT_PQ_DEMO_WCG       ,
    //MT_PQ_DEMO_FRC     ,
    MT_PQ_DEMO_COLOR     ,
    MT_PQ_DEMO_SR        ,
    MT_PQ_DEMO_ALL       ,

    MT_PQ_DEMO_BUTT
}  MT_PQ_DEMO_E;

/* PQ模块 */
typedef enum hiHIPQ_MODULE_E
{
    MT_PQ_MODULE_FMD = 0      ,
    MT_PQ_MODULE_TNR          ,
    MT_PQ_MODULE_SNR          ,
    MT_PQ_MODULE_DB           ,
    MT_PQ_MODULE_DR           ,
    MT_PQ_MODULE_HSHARPNESS   ,
    MT_PQ_MODULE_SHARPNESS    ,
    MT_PQ_MODULE_CCCL         ,
    MT_PQ_MODULE_COLOR_CORING ,
    MT_PQ_MODULE_BLUE_STRETCH ,
    MT_PQ_MODULE_GAMMA        ,
    MT_PQ_MODULE_DBC          ,
    MT_PQ_MODULE_DCI          ,
    MT_PQ_MODULE_COLOR        ,
    MT_PQ_MODULE_ES           ,
    MT_PQ_MODULE_SR           ,
    MT_PQ_MODULE_FRC          ,
    MT_PQ_MODULE_WCG          ,

    MT_PQ_MODULE_BUTT
}  MT_PQ_MODULE_E;

/* 色温设定 */
typedef struct mtPQ_COLOR_TEMP_S
{
    MT_S16 s16RedGain;
    MT_S16 s16GreenGain;
    MT_S16 s16BlueGain;
    MT_S16 s16RedOffset;
    MT_S16 s16GreenOffset;
    MT_S16 s16BlueOffset;
}  MT_PQ_COLOR_TEMP_S;

/* PQ文件路径 */
typedef struct mtPQ_PATH_S
{
    MT_CHAR cPqPath[128];
} MT_PQ_PATE_S;


/*
 * 模块开关属性
 */
typedef struct mtPQ_MODULE_S
{
    MT_PQ_MODULE_E enModule;
    MT_U32 u32OnOff; /*开关*/
} MT_PQ_MODULE_S;

/*
 * 卖场模式属性
 */
typedef struct mtPQ_DEMO_S
{
    MT_PQ_DEMO_E enModule;
    MT_BOOL bOnOff; /*开关*/
} MT_PQ_DEMO_S;

/*
 * 寄存器属性
 */
typedef struct mtPQ_REGISTER_S
{
    MT_U32 u32RegAddr;     //register addr
    MT_U8  u8Lsb;          //register lsb
    MT_U8  u8Msb;          //register msb
    MT_U8  u8SourceMode;   //video source
    MT_U8  u8OutputMode;   //output mode
    MT_U32 u32Module;      //module
    MT_U32 u32Value;       //register value
} MT_PQ_REGISTER_S;

/*ACM 寄存器控制参数结构*/
typedef struct mtPQ_COLOR_CTRL_S
{
    MT_U32 u32En;
    MT_U32 u32DbgEn;
    MT_U32 u32Stretch;
    MT_U32 u32Cliprange;
    MT_U32 u32Cliporwrap;
    MT_U32 u32Cbcrthr;
} MT_PQ_COLOR_CTRL_S;

/*ACM table结构*/
typedef struct mtPQ_DRV_ACM_LUT_S
{
    MT_U32 u32LutType;           /*ACM 查找表曲线类型*/
    MT_S16 as16Lut[ACM_Y_NUM][ACM_S_NUM][ACM_H_NUM];   /*ACM 查找表*/
} MT_PQ_ACM_LUT_S;

/*DCI曲线配置表*/
typedef struct mtPQ_DCI_WGT_S
{
    MT_S16 s16WgtCoef0[33];
    MT_S16 s16WgtCoef1[33];
    MT_S16 s16WgtCoef2[33];
    MT_U16 u16Gain0;
    MT_U16 u16Gain1;
    MT_U16 u16Gain2;
} MT_PQ_DCI_WGT_S;

/*DCI直方图结构*/
typedef struct mtPQ_DCI_HISTGRAM_S
{
    MT_S32 s32HistGram[32];
} MT_PQ_DCI_HISTGRAM_S;

/*TNR通用结构*/
typedef struct mtPQ_TNR_S
{
    MT_S32 s32MappingMax;
    MT_S32 s32MappingMin;
    MT_S32 s32MappingR[5];
    MT_S32 s32MappingT[6];
    MT_S32 s32MappingK[4];
} MT_PQ_TNR_S;

/*SNR的pixmean-ratio结构*/
typedef struct mtPQ_SNR_PIXMEAN_2_RATIO_S
{
    MT_U8 u8EdgeMaxRatio;
    MT_U8 u8EdgeMinRatio;
    MT_U8 u8EdgeOriRatio;
    MT_U8 u8Reserve;
    MT_U16 u16EdgeMeanTh[8];
    MT_U8 u8EdgeMeanK[8];
    MT_U16 u16PixMeanRatio[8];
} MT_PQ_SNR_PIXMEAN_2_RATIO_S;

/*SNR 的pixdiff-edgestr结构*/
typedef struct mtPQ_SNR_PIXDIFF_2_EDGESTR_S
{
    MT_U8 u8EdgeOriStrength;
    MT_U8 u8EdgeMaxStrength;
    MT_U8 u8EdgeMinStrength;
    MT_U8 u8Reserve1;
    MT_U16 u16EdgeStrTh[3];
    MT_U16 u16Reserve2;
    MT_U8 u8EdgeStrK[3];
    MT_U8 u8Reserve3;
    MT_U8 u8EdgeStr[3];
    MT_U8 u8Reserve4;
} MT_PQ_SNR_PIXDIFF_2_EDGESTR_S;

/*SR演示模式*/
typedef enum mtPQ_SR_DEMO_E
{
    MT_PQ_SR_DISABLE  = 0,//关掉SR,只ZME
    MT_PQ_SR_ENABLE_R,  //  右边SR
    MT_PQ_SR_ENABLE_L,  //左边SR
    MT_PQ_SR_ENABLE_A,  //全屏

    MT_PQ_SR_DEMO_BUTT
} MT_PQ_SR_DEMO_E;

typedef struct mtPQ_SIX_BASE_COLOR_S
{
    MT_U32  u32Red;
    MT_U32  u32Green;
    MT_U32  u32Blue;
    MT_U32  u32Cyan;
    MT_U32  u32Magenta;
    MT_U32  u32Yellow;
} MT_PQ_SIX_BASE_COLOR_S;

/*ACM GAIN 消息结构*/
typedef struct mtPQ_COLOR_GAIN_S
{
    MT_U32 u32GainMode;   /*0:SD;1:HD;2:UHD*/
    MT_U32 u32Gainluma;   /*表示对Hue的增益，范围0-1023*/
    MT_U32 u32Gainhue;    /*表示对Hue的增益，范围0-1023*/
    MT_U32 u32Gainsat;    /*表示对Luma的增益*/
} MT_PQ_COLOR_GAIN_S;

/*颜色增强类型*/
typedef enum mtPQ_COLOR_ENHANCE_E
{
    MT_PQ_COLOR_ENHANCE_FLESHTONE = 0,    //肤色增强
    MT_PQ_COLOR_ENHANCE_SIX_BASE,         //六基色增强,自定义颜色的增强
    MT_PQ_COLOR_ENHANCE_SPEC_COLOR_MODE,  //固定模式的颜色增强模式
    MT_PQ_COLOR_ENHANCE_BUTT
} MT_PQ_COLOR_ENHANCE_E;

/*六基色增强参数*/
typedef struct  mtPQ_SIX_BASE_S
{
    MT_U32  u32Red;       //范围:0~100
    MT_U32  u32Green;     //范围:0~100
    MT_U32  u32Blue;      //范围:0~100

    MT_U32  u32Cyan;      //范围:0~100
    MT_U32  u32Magenta;   //范围:0~100
    MT_U32  u32Yellow;    //范围:0~100
} MT_PQ_SIX_BASE_S;


/*肤色增益参数*/
typedef enum mtPQ_FLESHTONE_E
{
    MT_PQ_FLESHTONE_GAIN_OFF = 0,
    MT_PQ_FLESHTONE_GAIN_LOW,
    MT_PQ_FLESHTONE_GAIN_MID,
    MT_PQ_FLESHTONE_GAIN_HIGH,

    MT_PQ_FLESHTONE_GAIN_BUTT
}  MT_PQ_FLESHTONE_E;


/*颜色增强类型*/
typedef enum mtPQ_COLOR_SPEC_MODE_E
{
    MT_PQ_COLOR_MODE_RECOMMEND = 0, //推荐的颜色增强模式
    MT_PQ_COLOR_MODE_BLUE,          //固定的蓝色增强模式
    MT_PQ_COLOR_MODE_GREEN,         //固定的绿色增强模式
    MT_PQ_COLOR_MODE_BG,            //固定的蓝绿色增强模式
    MT_PQ_COLOR_MODE_BUTT
} MT_PQ_COLOR_SPEC_MODE_E;


/*颜色增强参数*/
typedef struct  mtPQ_COLOR_ENHANCE_S
{
    MT_PQ_COLOR_ENHANCE_E    enColorEnhanceType;   //色彩增强类型
    union
    {
        MT_PQ_FLESHTONE_E    enFleshtone;        //肤色增强参数
        MT_PQ_SIX_BASE_S     stSixBase;          //六基色增强参数
        MT_PQ_COLOR_SPEC_MODE_E   enColorMode;   //固定的颜色增强模式

    } unColorGain;
} MT_PQ_COLOR_ENHANCE_S;

/*用户接口*/
#define MTIOC_PQ_S_COLORTEMP            _IOW(MT_ID_PQ, 1, MT_PQ_COLOR_TEMP_S)         /* 设置色温参数*/
#define MTIOC_PQ_G_COLORTEMP            _IOR(MT_ID_PQ, 2, MT_PQ_COLOR_TEMP_S)         /* 获取色温参数*/

#define MTIOC_PQ_S_SD_BRIGHTNESS        _IOW(MT_ID_PQ, 3, MT_U32)                     /* 设置标清BRIGHTNESS level*/
#define MTIOC_PQ_G_SD_BRIGHTNESS        _IOR(MT_ID_PQ, 4, MT_U32)                     /* 获取标清BRIGHTNESS level*/

#define MTIOC_PQ_S_SD_CONTRAST          _IOW(MT_ID_PQ, 5, MT_U32)                     /* 设置标清CONTRAST level*/
#define MTIOC_PQ_G_SD_CONTRAST          _IOR(MT_ID_PQ, 6, MT_U32)                     /* 获取标清CONTRAST level*/

#define MTIOC_PQ_S_SD_SATURATION        _IOW(MT_ID_PQ, 7, MT_U32)                     /* 设置标清SATURATION level*/
#define MTIOC_PQ_G_SD_SATURATION        _IOR(MT_ID_PQ, 8, MT_U32)                     /* 获取标清SATURATION level*/

#define MTIOC_PQ_S_SD_HUE               _IOW(MT_ID_PQ, 9, MT_U32)                     /* 设置标清HUE level*/
#define MTIOC_PQ_G_SD_HUE               _IOR(MT_ID_PQ, 10, MT_U32)                    /* 获取标清HUE level*/

#define MTIOC_PQ_S_NR                   _IOW(MT_ID_PQ, 11, MT_U32)                    /* 设置NR level*/
#define MTIOC_PQ_G_NR                   _IOR(MT_ID_PQ, 12, MT_U32)                    /* 获取NR level*/

#define MTIOC_PQ_S_SHARPNESS            _IOW(MT_ID_PQ, 13, MT_U32)                    /* 设置SHARPNESS level*/
#define MTIOC_PQ_G_SHARPNESS            _IOR(MT_ID_PQ, 14, MT_U32)                    /* 获取SHARPNESS level*/

#define MTIOC_PQ_S_DB                   _IOW(MT_ID_PQ, 15, MT_U32)                    /* 设置De-blocking level*/
#define MTIOC_PQ_G_DB                   _IOR(MT_ID_PQ, 16, MT_U32)                    /* 获取De-blocking level*/

#define MTIOC_PQ_S_DR                   _IOW(MT_ID_PQ, 17, MT_U32)                    /* 设置De-Ring level*/
#define MTIOC_PQ_G_DR                   _IOR(MT_ID_PQ, 18, MT_U32)                    /* 获取De-Ring level*/

#define MTIOC_PQ_S_COLORGAIN            _IOW(MT_ID_PQ, 19, MT_U32)                    /* 设置Color gain level*/
#define MTIOC_PQ_G_COLORGAIN            _IOR(MT_ID_PQ, 20, MT_U32)                    /* 获取Color gain level*/

#define MTIOC_PQ_S_FLESHTONE            _IOW(MT_ID_PQ, 21, MT_PQ_FLESHTONE_E)         /* 设置FleshTone level*/
#define MTIOC_PQ_G_FLESHTONE            _IOR(MT_ID_PQ, 22, MT_PQ_FLESHTONE_E)         /* 获取FleshTone level*/

#define MTIOC_PQ_S_BACKLIGHT            _IOW(MT_ID_PQ, 23, MT_U32)                    /* 设置BackLight level*/
#define MTIOC_PQ_G_BACKLIGHT            _IOR(MT_ID_PQ, 24, MT_U32)                    /* 获取BackLight level*/

#define MTIOC_PQ_S_GAMMAIDX             _IOW(MT_ID_PQ, 25, MT_S32)                    /* 设置GAMMA index*/
#define MTIOC_PQ_G_GAMMANUM             _IOR(MT_ID_PQ, 26, MT_S32)                    /* 获取GAMMA num*/

#define MTIOC_PQ_S_MODULE               _IOW(MT_ID_PQ, 27, MT_PQ_MODULE_S)            /* 设置各算法模块开关*/
#define MTIOC_PQ_G_MODULE               _IOWR(MT_ID_PQ, 28, MT_PQ_MODULE_S)           /* 获取各算法模块开关*/

#define MTIOC_PQ_S_DEMO                 _IOW(MT_ID_PQ, 29, MT_PQ_DEMO_S)              /* 设置各算法模块卖场模式开关*/

#define MTIOC_PQ_S_SIXBASECOLOR         _IOW(MT_ID_PQ, 30, MT_PQ_SIX_BASE_COLOR_S)    /* 设置 六基色类型*/
#define MTIOC_PQ_G_SIXBASECOLOR         _IOR(MT_ID_PQ, 31, MT_PQ_SIX_BASE_COLOR_S)    /* 获取六基色类型*/

#define MTIOC_PQ_S_PQ_PATH              _IOW(MT_ID_PQ, 32, MT_PQ_PATE_S)              /* 设置 PQ文件路径*/

#define MTIOC_PQ_S_NR_AUTO              _IOW(MT_ID_PQ, 33, MT_U32)                    /* 设置NR自动模式*/
#define MTIOC_PQ_G_NR_AUTO              _IOR(MT_ID_PQ, 34, MT_U32)                    /* 获取NR自动模式*/

#define MTIOC_PQ_S_SR_DEMO              _IOW(MT_ID_PQ, 35, MT_PQ_SR_DEMO_E)           /* 设置SR演示模式*/
#define MTIOC_PQ_G_SR_DEMO              _IOR(MT_ID_PQ, 36, MT_PQ_SR_DEMO_E)           /* 获取SR演示模式*/

/*调试接口*/
#define MTIOC_PQ_S_3DSHARP              _IOW(MT_ID_PQ, 40, MT_U32)                    /* 设置3D SHARP mode*/
#define MTIOC_PQ_G_3DSHARP              _IOR(MT_ID_PQ, 41, MT_U32)                    /* 获取3D SHARP mode*/

//#define MTIOC_PQ_S_CSCMODE                    _IOW(MT_ID_PQ, 42, MT_PQ_CSC_S)                          /* 设置CSC mode*/
//#define MTIOC_PQ_G_CSCMODE                    _IOR(MT_ID_PQ, 43, MT_PQ_CSC_S)                           /* 获取CSC mode*/

#define MTIOC_PQ_S_REGISTER             _IOW(MT_ID_PQ, 44, MT_PQ_REGISTER_S)          /* 写寄存器*/
#define MTIOC_PQ_G_REGISTER             _IOWR(MT_ID_PQ, 45, MT_PQ_REGISTER_S)         /*读寄存器*/

//#define MTIOC_PQ_S_GAMMA                      _IOW(MT_ID_PQ, 46, MT_PQ_GAMMA_TABLE_S)            /* 写GAMMA曲线*/
//#define MTIOC_PQ_G_GAMMA                      _IOR(MT_ID_PQ, 47, MT_PQ_GAMMA_TABLE_S)             /*读GAMMA曲线*/

#define MTIOC_PQ_S_DCI                 _IOW(MT_ID_PQ, 48, MT_PQ_DCI_WGT_S)            /*写DCI曲线*/
#define MTIOC_PQ_G_DCI                 _IOR(MT_ID_PQ, 49, MT_PQ_DCI_WGT_S)            /*读DCI曲线*/

#define MTIOC_PQ_S_ACM_GAIN            _IOW(MT_ID_PQ, 50, MT_PQ_COLOR_GAIN_S)         /*写COLOR GAIN*/
#define MTIOC_PQ_G_ACM_GAIN            _IOR(MT_ID_PQ, 51, MT_PQ_COLOR_GAIN_S)         /*读COLOR GAIN*/

#define MTIOC_PQ_S_ACM_LUMA            _IOW(MT_ID_PQ, 52, MT_PQ_ACM_LUT_S)            /*写COLOR LUMA曲线*/
#define MTIOC_PQ_G_ACM_LUMA            _IOR(MT_ID_PQ, 53, MT_PQ_ACM_LUT_S)            /*读COLOR LUMA曲线*/

#define MTIOC_PQ_S_ACM_HUE             _IOW(MT_ID_PQ, 54, MT_PQ_ACM_LUT_S)            /*写COLOR HUE曲线*/
#define MTIOC_PQ_G_ACM_HUE             _IOR(MT_ID_PQ, 55, MT_PQ_ACM_LUT_S)            /*读COLOR HUE曲线*/

#define MTIOC_PQ_S_ACM_SAT             _IOW(MT_ID_PQ, 56, MT_PQ_ACM_LUT_S)            /*写COLOR SAT曲线*/
#define MTIOC_PQ_G_ACM_SAT             _IOR(MT_ID_PQ, 57, MT_PQ_ACM_LUT_S)            /*读COLOR SAT曲线*/

#define MTIOC_PQ_G_DCI_HIST            _IOR(MT_ID_PQ, 58, MT_PQ_DCI_HISTGRAM_S)       /*读DCI直方图*/

//#define MTIOC_PQ_S_DIM                          _IOW(MT_ID_PQ, 60, MT_PQ_DIM_LUT_S)                     /*写DIM映射曲线*/
//#define MTIOC_PQ_G_DIM                          _IOR(MT_ID_PQ, 61, MT_PQ_DIM_LUT_S)                      /*读DIM映射曲线*/

#define MTIOC_PQ_S_TNR_Y_PIXMEAN_2_RATIO       _IOW(MT_ID_PQ, 62, MT_PQ_TNR_S)        /*写TNR的亮度PixMean-to-Ratio*/
#define MTIOC_PQ_G_TNR_Y_PIXMEAN_2_RATIO       _IOR(MT_ID_PQ, 63, MT_PQ_TNR_S)        /*读TNR的亮度PixMean-to-Ratio*/

#define MTIOC_PQ_S_TNR_C_PIXMEAN_2_RATIO       _IOW(MT_ID_PQ, 64, MT_PQ_TNR_S)        /*写TNR 色度PixMean-to-Ratio映射曲线*/
#define MTIOC_PQ_G_TNR_C_PIXMEAN_2_RATIO       _IOR(MT_ID_PQ, 65, MT_PQ_TNR_S)        /*读TNR 色度PixMean-to-Ratio映射曲线*/

#define MTIOC_PQ_S_TNR_Y_MOTION_MAPPING        _IOW(MT_ID_PQ, 66, MT_PQ_TNR_S)        /*写TNR亮度MotionMapping曲线*/
#define MTIOC_PQ_G_TNR_Y_MOTION_MAPPING        _IOR(MT_ID_PQ, 67, MT_PQ_TNR_S)        /*读TNR亮度MotionMapping曲线*/

#define MTIOC_PQ_S_TNR_C_MOTION_MAPPING        _IOW(MT_ID_PQ, 68, MT_PQ_TNR_S)        /*写TNR色度MotionMapping曲线*/
#define MTIOC_PQ_G_TNR_C_MOTION_MAPPING        _IOR(MT_ID_PQ, 69, MT_PQ_TNR_S)        /*读TNR色度MotionMapping曲线*/

#define MTIOC_PQ_S_TNR_Y_FINAL_MOTION_MAPPING  _IOW(MT_ID_PQ, 70, MT_PQ_TNR_S)        /*写TNR亮度FINAL MotionMapping曲线*/
#define MTIOC_PQ_G_TNR_Y_FINAL_MOTION_MAPPING  _IOR(MT_ID_PQ, 71, MT_PQ_TNR_S)        /*读TNR亮度FINAL MotionMapping曲线*/

#define MTIOC_PQ_S_TNR_C_FINAL_MOTION_MAPPING  _IOW(MT_ID_PQ, 72, MT_PQ_TNR_S)        /*写TNR色度FINAL MotionMapping曲线*/
#define MTIOC_PQ_G_TNR_C_FINAL_MOTION_MAPPING  _IOR(MT_ID_PQ, 73, MT_PQ_TNR_S)        /*读TNR色度FINAL MotionMapping曲线*/

#define MTIOC_PQ_S_SNR_PIXMEAN_2_RATIO         _IOW(MT_ID_PQ, 74, MT_PQ_SNR_PIXMEAN_2_RATIO_S)     /*写SNR 的pixmean-ratio映射曲线*/
#define MTIOC_PQ_G_SNR_PIXMEAN_2_RATIO         _IOR(MT_ID_PQ, 75, MT_PQ_SNR_PIXMEAN_2_RATIO_S)     /*读SNR 的pixmean-ratio映射曲线*/

#define MTIOC_PQ_S_SNR_PIXDIFF_2_EDGESTR       _IOW(MT_ID_PQ, 76, MT_PQ_SNR_PIXDIFF_2_EDGESTR_S)   /*写SNR 的pixdiff-edgestr映射曲线*/
#define MTIOC_PQ_G_SNR_PIXDIFF_2_EDGESTR       _IOR(MT_ID_PQ, 77, MT_PQ_SNR_PIXDIFF_2_EDGESTR_S)   /*读SNR 的pixdiff-edgestr映射曲线*/

#define MTIOC_PQ_S_HD_BRIGHTNESS        _IOW(MT_ID_PQ, 84, MT_U32)                    /* 设置高清BRIGHTNESS level*/
#define MTIOC_PQ_G_HD_BRIGHTNESS        _IOR(MT_ID_PQ, 85, MT_U32)                    /* 获取高清BRIGHTNESS level*/

#define MTIOC_PQ_S_HD_CONTRAST          _IOW(MT_ID_PQ, 86, MT_U32)                    /* 设置高清CONTRAST level*/
#define MTIOC_PQ_G_HD_CONTRAST          _IOR(MT_ID_PQ, 87, MT_U32)                    /* 获取高清CONTRAST level*/

#define MTIOC_PQ_S_HD_SATURATION        _IOW(MT_ID_PQ, 88, MT_U32)                    /* 设置高清SATURATION level*/
#define MTIOC_PQ_G_HD_SATURATION        _IOR(MT_ID_PQ,89, MT_U32)                     /* 获取高清SATURATION level*/

#define MTIOC_PQ_S_HD_HUE               _IOW(MT_ID_PQ, 90, MT_U32)                    /* 设置高清HUE level*/
#define MTIOC_PQ_G_HD_HUE               _IOR(MT_ID_PQ, 91, MT_U32)                    /* 获取高清HUE level*/

#define MTIOC_PQ_S_DCI_LEVEL            _IOW(MT_ID_PQ, 92, MT_U32)                    /* 设置DCI level*/
#define MTIOC_PQ_G_DCI_LEVEL            _IOR(MT_ID_PQ, 93, MT_U32)                    /* 获取DCI level*/

#define MTIOC_PQ_G_BIN_ADDR             _IOR(MT_ID_PQ, 94, MT_U32)                    /* 获取PQBin的物理地址*/

#define MTIOC_PQ_S_COLOR_ENHANCE_MODE   _IOW(MT_ID_PQ, 95, MT_PQ_COLOR_SPEC_MODE_E)   /* 设置颜色增强模式*/
#define MTIOC_PQ_G_COLOR_ENHANCE_MODE   _IOR(MT_ID_PQ, 96, MT_PQ_COLOR_SPEC_MODE_E)   /* 获取颜色增强模式*/

#define MTIOC_PQ_S_ACM_CTRL             _IOW(MT_ID_PQ, 97, MT_PQ_COLOR_CTRL_S)        /* 设置ACM 控制寄存器*/
#define MTIOC_PQ_G_ACM_CTRL             _IOR(MT_ID_PQ, 98, MT_PQ_COLOR_CTRL_S)        /* 获取ACM 控制寄存器*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* __MT_DRV_PQ_H__ */
