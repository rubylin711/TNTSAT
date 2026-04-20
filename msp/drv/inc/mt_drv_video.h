/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************
File Name     : mt_drv_video.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2015/11/25
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __MT_DRV_VIDEO_H__
#define __MT_DRV_VIDEO_H__

#include "mt_type.h"
#include "mt_common.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

typedef enum mtDRV_COLOR_SYS_E
{
    MT_DRV_COLOR_SYS_AUTO = 0,
    MT_DRV_COLOR_SYS_PAL,
    MT_DRV_COLOR_SYS_NTSC,
    MT_DRV_COLOR_SYS_SECAM,
    MT_DRV_COLOR_SYS_PAL_M,
    MT_DRV_COLOR_SYS_PAL_N,
    MT_DRV_COLOR_SYS_PAL_60,
    MT_DRV_COLOR_SYS_NTSC443,
    MT_DRV_COLOR_SYS_NTSC_50,

    MT_DRV_COLOR_SYS_BUTT
} MT_DRV_COLOR_SYS_E;

/**Defines the oversample mode of the current input source.*/
typedef enum mtDRV_OVERSAMPLE_MODE_E
{
    MT_DRV_OVERSAMPLE_1X = 0,
    MT_DRV_OVERSAMPLE_2X,
    MT_DRV_OVERSAMPLE_4X,
    MT_DRV_OVERSAMPLE_BUTT,
} MT_DRV_OVERSAMPLE_MODE_E;

typedef enum mtDRV_PIXEL_BITWIDTH_E
{
    MT_DRV_PIXEL_BITWIDTH_8BIT = 0,
    MT_DRV_PIXEL_BITWIDTH_10BIT,
    MT_DRV_PIXEL_BITWIDTH_12BIT,
    MT_DRV_PIXEL_BITWIDTH_BUTT,
} MT_DRV_PIXEL_BITWIDTH_E;
typedef enum mtDRV_PIX_FORMAT_E
{
    /* RGB formats */
    MT_DRV_PIX_FMT_RGB332 = 0, /*  8  RGB-3-3-2     */
    MT_DRV_PIX_FMT_RGB444  ,   /* 16  xxxxrrrr ggggbbbb */
    MT_DRV_PIX_FMT_RGB555  ,   /* 16  RGB-5-5-5     */
    MT_DRV_PIX_FMT_RGB565  ,   /* 16  RGB-5-6-5     */
    MT_DRV_PIX_FMT_BGR565  ,   /* 16  RGB-5-6-5     */
    MT_DRV_PIX_FMT_RGB555X ,   /* 16  RGB-5-5-5 BE  */
    MT_DRV_PIX_FMT_RGB565X ,   /* 16  RGB-5-6-5 BE  */
    MT_DRV_PIX_FMT_BGR666  ,   /* 18  BGR-6-6-6	  */
    MT_DRV_PIX_FMT_BGR24   ,   /* 24  BGR-8-8-8     */
    MT_DRV_PIX_FMT_RGB24   ,   /* 24  RGB-8-8-8     */
    MT_DRV_PIX_FMT_BGR32   ,   /* 32  BGR-8-8-8-8   */
    MT_DRV_PIX_FMT_RGB32   ,   /* 32  RGB-8-8-8-8   */

    /****** MONTAGE CLUT formats ******/
    MT_DRV_PIX_FMT_CLUT_1BPP,
    MT_DRV_PIX_FMT_CLUT_2BPP,
    MT_DRV_PIX_FMT_CLUT_4BPP,
    MT_DRV_PIX_FMT_CLUT_8BPP,
    MT_DRV_PIX_FMT_ACLUT_44,
    MT_DRV_PIX_FMT_ACLUT_88,

    /****** MONTAGE 16bit RGB formats ******/
    MT_DRV_PIX_FMT_ARGB4444,
    MT_DRV_PIX_FMT_ABGR4444,
    MT_DRV_PIX_FMT_RGBA4444,

    MT_DRV_PIX_FMT_ARGB1555,
    MT_DRV_PIX_FMT_ABGR1555,
    MT_DRV_PIX_FMT_RGBA5551,

    /****** MONTAGE 24bit RGB formats ******/
    MT_DRV_PIX_FMT_ARGB8565,
    MT_DRV_PIX_FMT_ABGR8565,
    MT_DRV_PIX_FMT_RGBA5658,

    MT_DRV_PIX_FMT_ARGB6666,
    MT_DRV_PIX_FMT_RGBA6666,

    /****** MONTAGE 32bit RGB formats ******/
    MT_DRV_PIX_FMT_ARGB8888,
    MT_DRV_PIX_FMT_ABGR8888,
    MT_DRV_PIX_FMT_RGBA8888,

    MT_DRV_PIX_FMT_AYUV8888,
    MT_DRV_PIX_FMT_YUVA8888,

    /* Grey formats */
    MT_DRV_PIX_FMT_GREY    ,    /*  8  Greyscale     */
    MT_DRV_PIX_FMT_Y4      ,    /*  4  Greyscale     */
    MT_DRV_PIX_FMT_Y6      ,    /*  6  Greyscale     */
    MT_DRV_PIX_FMT_Y10     ,    /* 10  Greyscale     */
    MT_DRV_PIX_FMT_Y12     ,    /* 12  Greyscale     */
    MT_DRV_PIX_FMT_Y16     ,    /* 16  Greyscale     */

    /* Grey bit-packed formats */
    MT_DRV_PIX_FMT_Y10BPACK    ,   /* 10  Greyscale bit-packed */

    /* Palette formats */
    MT_DRV_PIX_FMT_PAL8    ,    /*  8  8-bit palette */

    /* Luminance+Chrominance formats */
    MT_DRV_PIX_FMT_YVU410  ,    /*  9  YVU 4:1:0     */
    MT_DRV_PIX_FMT_YVU420  ,    /* 12  YVU 4:2:0     */
    MT_DRV_PIX_FMT_YUYV    ,    /* 16  YUV 4:2:2     */
    MT_DRV_PIX_FMT_YYUV    ,    /* 16  YUV 4:2:2     */
    MT_DRV_PIX_FMT_YVYU    ,    /* 16  YVU 4:2:2 */
    MT_DRV_PIX_FMT_UYVY    ,    /* 16  YUV 4:2:2     */
    MT_DRV_PIX_FMT_VYUY    ,    /* 16  YUV 4:2:2     */
    MT_DRV_PIX_FMT_YUV422P ,    /* 16  YVU422 planar */
    MT_DRV_PIX_FMT_YUV411P ,    /* 16  YVU411 planar */
    MT_DRV_PIX_FMT_Y41P    ,    /* 12  YUV 4:1:1     */
    MT_DRV_PIX_FMT_YUV444  ,    /* 16  xxxxyyyy uuuuvvvv */
    MT_DRV_PIX_FMT_YUV555  ,    /* 16  YUV-5-5-5     */
    MT_DRV_PIX_FMT_YUV565  ,    /* 16  YUV-5-6-5     */
    MT_DRV_PIX_FMT_YUV32   ,    /* 32  YUV-8-8-8-8   */
    MT_DRV_PIX_FMT_YUV410  ,    /*  9  YUV 4:1:0     */
    MT_DRV_PIX_FMT_YUV420  ,    /* 12  YUV 4:2:0     */
    MT_DRV_PIX_FMT_HI240   ,    /*  8  8-bit color   */
    MT_DRV_PIX_FMT_HM12    ,    /*  8  YUV 4:2:0 16x16 macroblocks */
    MT_DRV_PIX_FMT_M420    ,    /* 12  YUV 4:2:0 2 lines y,
                                       1 line uv interleaved */

    /* two planes -- one Y, one Cr + Cb interleaved  */
    MT_DRV_PIX_FMT_NV08    ,     /*08  Y/CbCr 4:0:0 @*/
    MT_DRV_PIX_FMT_NV80    ,     /*08  Y/CrCb 4:0:0 @*/
    MT_DRV_PIX_FMT_NV12    ,    /* 12  Y/CbCr 4:2:0  */
    MT_DRV_PIX_FMT_NV21    ,    /* 12  Y/CrCb 4:2:0  */
    MT_DRV_PIX_FMT_NV12_411,    /* 12  Y/CbCr 4:1:1  @*/
    MT_DRV_PIX_FMT_NV16    ,    /* 16  Y/CbCr 4:2:2  */
    MT_DRV_PIX_FMT_NV61    ,    /* 16  Y/CrCb 4:2:2  */
    MT_DRV_PIX_FMT_NV16_2X1,    /* 16  Y/CbCr 4:2:2 2X1 @*/
    MT_DRV_PIX_FMT_NV61_2X1,    /* 16  Y/CrCb 4:2:2  2X1 @*/
    MT_DRV_PIX_FMT_NV24    ,    /* 24  Y/CbCr 4:4:4  */
    MT_DRV_PIX_FMT_NV42    ,    /* 24  Y/CrCb 4:4:4  */

    /* two non contiguous planes - one Y, one Cr + Cb interleaved  */
    MT_DRV__PIX_FMT_NV12M  ,    /* 12  Y/CbCr 4:2:0  */
    MT_DRV__PIX_FMT_NV12MT ,    /* 12  Y/CbCr 4:2:0 64x32 macroblocks */

    /* three non contiguous planes - Y, Cb, Cr */
    MT_DRV_PIX_FMT_YUV420M ,    /* 12  YUV420 planar */

    /* Bayer formats - see http://www.siliconimaging.com/RGB%20Bayer.htm */
    MT_DRV_PIX_FMT_SBGGR8  ,    /*  8  BGBG.. GRGR.. */
    MT_DRV_PIX_FMT_SGBRG8  ,    /*  8  GBGB.. RGRG.. */
    MT_DRV_PIX_FMT_SGRBG8  ,    /*  8  GRGR.. BGBG.. */
    MT_DRV_PIX_FMT_SRGGB8  ,    /*  8  RGRG.. GBGB.. */
    MT_DRV_PIX_FMT_SBGGR10 ,    /* 10  BGBG.. GRGR.. */
    MT_DRV_PIX_FMT_SGBRG10 ,    /* 10  GBGB.. RGRG.. */
    MT_DRV_PIX_FMT_SGRBG10 ,    /* 10  GRGR.. BGBG.. */
    MT_DRV_PIX_FMT_SRGGB10 ,    /* 10  RGRG.. GBGB.. */
    MT_DRV_PIX_FMT_SBGGR12 ,    /* 12  BGBG.. GRGR.. */
    MT_DRV_PIX_FMT_SGBRG12 ,    /* 12  GBGB.. RGRG.. */
    MT_DRV_PIX_FMT_SGRBG12 ,    /* 12  GRGR.. BGBG.. */
    MT_DRV_PIX_FMT_SRGGB12 ,    /* 12  RGRG.. GBGB.. */

    /****** MONTAGE Luminance+Chrominance formats ******/

    /****** MONTAGE  contiguoustwo planes -- one Y, one Cr + Cb interleaved ******/
    MT_DRV_PIX_FMT_NV08_CMP,     /*08   Y/CbCr 4:0:0 compressed @*/
    MT_DRV_PIX_FMT_NV80_CMP,     /*08   Y/CrCb 4:0:0 compressed @*/
    MT_DRV_PIX_FMT_NV12_CMP ,    /* 12  Y/CbCr 4:2:0 compressed */
    MT_DRV_PIX_FMT_NV21_CMP ,    /* 12  Y/CrCb 4:2:0 compressed */
    MT_DRV_PIX_FMT_NV16_CMP ,    /* 16  Y/CbCr 4:2:2 compressed */
    MT_DRV_PIX_FMT_NV61_CMP ,    /* 16  Y/CrCb 4:2:2 compressed */
    MT_DRV_PIX_FMT_NV16_2X1_CMP,    /* 16  Y/CbCr 4:2:2   2X1@*/
    MT_DRV_PIX_FMT_NV61_2X1_CMP,    /* 16  Y/CrCb 4:2:2  2X1@*/
    MT_DRV_PIX_FMT_NV24_CMP ,    /* 24  Y/CbCr 4:4:4 compressed */
    MT_DRV_PIX_FMT_NV42_CMP ,    /* 24  Y/CrCb 4:4:4 compressed */

    MT_DRV_PIX_FMT_NV12_TILE,    /* 12 tile  */
    MT_DRV_PIX_FMT_NV21_TILE,    /* 21 tile  */
    MT_DRV_PIX_FMT_YUV400_TILE,    /* 21 tile  */

    MT_DRV_PIX_FMT_NV12_TILE_CMP,   /* 12 tile compressed */
    MT_DRV_PIX_FMT_NV21_TILE_CMP,   /* 21 tile compressed */

    /****** MONTAGE three non contiguous planes - Y, Cb, Cr ******/
        MT_DRV_PIX_FMT_YUV400  ,   /*08  YUV400 planar @*/
        MT_DRV_PIX_FMT_YUV410p  ,   /*10  YUV410 planar @*/
        MT_DRV_PIX_FMT_YUV420p ,   /*12  YUV420 planar @*/
        MT_DRV_PIX_FMT_YUV411  ,   /*12  YUV411  planar @*/
        MT_DRV_PIX_FMT_YUV422_1X2, /*16  YUV422  planar 1X2 @*/
        MT_DRV_PIX_FMT_YUV422_2X1, /*16  YUV422  planar 2X1@*/
        MT_DRV_PIX_FMT_YUV_444 ,   /*24  YUV444  planar @*/

    /****** MONTAGE three non contiguous planes - Y, Cb, Cr ******/

    MT_DRV_PIX_BUTT


}MT_DRV_PIX_FORMAT_E;

/* video frame filed in buffer */
typedef enum mtDRV_FIELD_MODE_E
{
    MT_DRV_FIELD_TOP = 0,
    MT_DRV_FIELD_BOTTOM,
    MT_DRV_FIELD_ALL,
    MT_DRV_FIELD_BUTT
}MT_DRV_FIELD_MODE_E;

/* video frame type */
typedef enum mtDRV_FRAME_TYPE_E
{
    MT_DRV_FT_NOT_STEREO = 0,
    MT_DRV_FT_SBS,
    MT_DRV_FT_TAB,
    MT_DRV_FT_FPK,
    MT_DRV_FT_TILE,
    MT_DRV_FT_BUTT
}MT_DRV_FRAME_TYPE_E;

/* source color space */
typedef enum mtDRV_COLOR_SPACE_E
{
    MT_DRV_CS_UNKNOWN = 0,
    MT_DRV_CS_DEFAULT,

    MT_DRV_CS_BT601_YUV_LIMITED,/* BT.601 */
    MT_DRV_CS_BT601_YUV_FULL,
    MT_DRV_CS_BT601_RGB_LIMITED,
    MT_DRV_CS_BT601_RGB_FULL,

    MT_DRV_CS_NTSC1953,

    /* These should be useful.  Assume 601 extents. */
    MT_DRV_CS_BT470_SYSTEM_M,
    MT_DRV_CS_BT470_SYSTEM_BG,

    MT_DRV_CS_BT709_YUV_LIMITED,/* BT.709 */
    MT_DRV_CS_BT709_YUV_FULL,
    MT_DRV_CS_BT709_RGB_LIMITED,
    MT_DRV_CS_BT709_RGB_FULL,

    MT_DRV_CS_REC709,      /* HD and modern captures. */

    MT_DRV_CS_SMPT170M, /* ITU-R 601 -- broadcast NTSC/PAL */
    MT_DRV_CS_SMPT240M, /* 1125-Line (US) HDTV */

    MT_DRV_CS_BT878,    /* broken BT878 extents
                           (601, luma range 16-253 instead of 16-235) */

    MT_DRV_CS_XVYCC,

    /* I know there will be cameras that send this.  So, this is
     * unspecified chromaticities and full 0-255 on each of the
     * Y'CbCr components
     */
    MT_DRV_CS_JPEG,
    MT_DRV_CS_BUTT
} MT_DRV_COLOR_SPACE_E;

/* frame rotation angle */
typedef enum mtDRV_ROT_ANGLE_E
{
    MT_DRV_ROT_ANGLE_0 = 0,
    MT_DRV_ROT_ANGLE_90,
    MT_DRV_ROT_ANGLE_180,
    MT_DRV_ROT_ANGLE_270,
    MT_DRV_ROT_ANGLE_BUTT
}MT_DRV_ROT_ANGLE_E;

/* video frame aspect ratio mode */
typedef enum mtDRV_ASP_RAT_MODE_E
{
    MT_DRV_ASP_RAT_MODE_FULL = 0x0,
    MT_DRV_ASP_RAT_MODE_LETTERBOX,
    MT_DRV_ASP_RAT_MODE_PANANDSCAN,
    MT_DRV_ASP_RAT_MODE_COMBINED,
    MT_DRV_ASP_RAT_MODE_FULL_H,
    MT_DRV_ASP_RAT_MODE_FULL_V,
    MT_DRV_ASP_RAT_MODE_CUSTOMER,
    MT_DRV_ASP_RAT_MODE_TV,
    MT_DRV_ASP_RAT_MODE_BUTT
}MT_DRV_ASP_RAT_MODE_E;

typedef struct mtDRV_CROP_RECT_S
{
    mt_u32 u32LeftOffset;
    mt_u32 u32TopOffset;
    mt_u32 u32RightOffset;
    mt_u32 u32BottomOffset;
}MT_DRV_CROP_RECT_S;


/* aspect ratio, for monitor or pixel.
  0<= arw <= 256, 0<=arh<=256, and '1/16  <= arw/arh <= 16'.
  e.g., if aspect ratio is 16:9, you can set arw as 16 and arh as 9,
  OR arw as 160, arh as 90.
  Exceptive :
  0:1 means unknown;
  0:2 means display pixel 1:1
  */
typedef struct mtDRV_ASPECT_RATIO_S
{
    mt_u32 u32ARw;
    mt_u32 u32ARh;
}MT_DRV_ASPECT_RATIO_S;

/* video frame buffer physical address */
typedef struct mtDRV_VID_FRAME_ADDR_S
{
    /* Y address*/
    ulong  u32PhyAddr_YHead; /* only for compress format */
    ulong  u32PhyAddr_Y;
    mt_u32  u32Stride_Y;
    ulong  u32DsPhyAddr_Y;

    /* C OR Cb address*/
    ulong  u32PhyAddr_CHead;
    ulong  u32PhyAddr_C;
    mt_u32  u32Stride_C;
    ulong  u32DsPhyAddr_C;

    /* Cr address*/
    ulong  u32PhyAddr_CrHead;
    ulong  u32PhyAddr_Cr;
    mt_u32  u32Stride_Cr;
    ulong  u32DsPhyAddr_Cr;
}MT_DRV_VID_FRAME_ADDR_S;

typedef enum mtDRV_BUF_ADDR_E
{
    MT_DRV_BUF_ADDR_LEFT  = 0,
    MT_DRV_BUF_ADDR_RIGHT = 1,  /* only for right eye frame of 3D video */
    MT_DRV_BUF_ADDR_MAX
}MT_DRV_BUF_ADDR_E;

#define DEF_MT_DRV_FRAME_INFO_SIZE 64
#if 0
typedef struct mtDRV_VIDEO_PRIV_INFO_S
{
    mt_u32 u32FrameIndex;  //óDμY??òa?ó
    mt_u32 u32BufferID;

    //mt_u32 u32Is1D;
    //mt_u32 u32IsCompress;

    mt_u32 u32SpecStrmFlag;
    mt_u32 u32Playtime;

    MT_DRV_COLOR_SPACE_E eColorSpace; /* 'MT_DRV_CS_UNKNOWN' means unknown */
    MT_DRV_FIELD_MODE_E  eOriginField;
    MT_RECT_S stOriginImageRect;  /* orgin image rectagle without letterbox */

    mt_u32 u32VC1RangeInfo[16];
    mt_u32 u32PrivInfo[16];
}DRV_VIDEO_PRIV_INFO_S;

/* video frame info */
typedef struct mtDRV_VIDEO_FRAME_S
{
    //MT_DRV_VID_INFO_S  stFrmInfo;
    MT_DRV_FRAME_TYPE_E  eFrmType;
    MT_DRV_PIX_FORMAT_E  ePixFormat;

    MT_BOOL bInterlaced;
    MT_BOOL bTopFirst;

    mt_u32  u32Width;
    mt_u32  u32Height;

    //display region in rectangle (x,y,w,h)
    MT_RECT_S stDispRect;

    //maybe use
/*
    MT_DRV_ROT_ANGLE_E eRotAngle;
    MT_BOOL bToFlip_H;
    MT_BOOL bToFlip_V;
*/
    MT_DRV_ASPECT_RATIO_S stDispAR;
    //MT_DRV_ASPECT_RATIO_S stSampAR;  /* reserved, must be 0 */

    mt_u32  u32FrameRate;     /* in 1/100 Hz, 0 means unknown */
    MT_DRV_COLOR_SPACE_E eColorSpace; /* 'MT_DRV_CS_UNKNOWN' means unknown */

    /* these member may be changed per frame */
    mt_u32 u32FrmCnt;
    mt_u32 u32SrcPts;  /* 0xffffffff means unknown */
    mt_u32 u32Pts;     /* 0xffffffff means unknown */

    mt_u32  u32PlayTime;  /* 0 means ignore, do not process */
    MT_BOOL bToRelease; /* frame should be released when displayed */

    /* stBufAddr[1] is right eye for stereo video */
    MT_DRV_VID_FRAME_ADDR_S stBufAddr[MT_DRV_BUF_ADDR_MAX];

    MT_DRV_FIELD_MODE_E  eField;

    MT_DRV_FIELD_MODE_E  eOriginField;
    MT_RECT_S stOriginImageRect;  /* orgin image rectagle without letterbox */

    mt_u32 u32ErrorLevel;

    mt_u32 u32Priv[DEF_MT_DRV_FRAME_INFO_SIZE];  /* must be 0 */
}MT_DRV_VIDEO_FRAME_S;
#endif

/* define of how to adjust the TB match */
typedef enum mtDRV_VIDEO_TB_ADJUST_E
{
    MT_DRV_VIDEO_TB_PLAY = 0,
    MT_DRV_VIDEO_TB_REPEAT,
    MT_DRV_VIDEO_TB_DISCARD,
    MT_DRV_VIDEO_TB_BUTT
}MT_DRV_VIDEO_TB_ADJUST_E;


typedef enum mtDRV_SOURCE_E
{
    MT_DRV_SOURCE_DTV = 0,
    MT_DRV_SOURCE_USB,

    MT_DRV_SOURCE_ATV,
    MT_DRV_SOURCE_SCART,
    MT_DRV_SOURCE_SVIDEO,
    MT_DRV_SOURCE_CVBS,
    MT_DRV_SOURCE_VGA,
    MT_DRV_SOURCE_YPBPR,
    MT_DRV_SOURCE_HDMI,

    MT_DRV_SOURCE_BUTT
} MT_DRV_SOURCE_E;

/* Source Timing Info */
typedef struct mtDRV_VIDEO_ORIGINAL_INFO_S
{
    MT_DRV_SOURCE_E             enSource;       //DTV Default MT_DRV_SOURCE_DTV
    mt_u32                      u32Width;       //source width
    mt_u32                      u32Height;      //source height
    mt_u32                      u32FrmRate;     //source framerate
    MT_DRV_FRAME_TYPE_E         en3dType;       //source 3D Type
    MT_DRV_COLOR_SPACE_E        enSrcColorSpace;//Only use three Type:BT601_YUV_LIMITED,BT709_YUV_LIMITED,BT709_RGB_FULL
    MT_DRV_COLOR_SYS_E          enColorSys;     //DTV Default MT_DRV_COLOR_SYS_AUTO
    MT_BOOL                     bGraphicMode;   //DTV default MT_FALSE
    MT_BOOL                     bInterlace;     //source is Interlace or Progress

}MT_DRV_VIDEO_ORIGINAL_INFO_S;

typedef enum
{
  /*!
    display frame once
  */
  MT_DIS_FRAME_ONCE,

  /*!
    display frame twice
  */
  MT_DIS_FRAME_TWICE,

  /*!
    display frame triple
  */
  MT_DIS_FRAME_TRIPLE,

  /*!
    display frame triple
  */
  MT_DIS_FRAME_N_TIME,

  /*!
    display top field, then bottom field
  */
  MT_DIS_TOP_BOT,

  /*!
    display bottom field, then top field
  */
  MT_DIS_BOT_TOP,

  /*!
    display top field, then bottom field, then top filed
  */
  MT_DIS_TOP_BOT_TOP,

  /*!
    display bottom field, then top field, then bottom field
  */
  MT_DIS_BOT_TOP_BOT
}MT_DIS_ORDER_MODE_E;

/*!
  TV mode
  */
typedef enum
{
  /*!
    poly-phase
    */
  TV_MODE_SDR,
  /*!
    matrix
    */
  TV_MODE_HDR10,
  /*!
    mix poly and matrix
    */
  TV_MODE_HLG,

  TV_MODE_HDR10P,

  TV_MODE_HDR10_BT709,
  TV_MODE_HLG_BT709,
  TV_MODE_HDR10P_BT709
}tv_mode_t;

typedef struct
{
  mt_u32 slot_idx;
  mt_u32 pts;
  mt_u32 addrLuma;  // byte allian
  mt_u32 addrChroma;
  mt_u32 ds_addrLuma;  // byte allian
  mt_u32 ds_addrChroma;
  /* mfbc */
  mt_u32 addrLuma_lut;
  mt_u32 addrChroma_lut;
  mt_u32 stride_data_luma;
  mt_u32 stride_data_chroma;
  mt_u32 bit_depth_luma;
  mt_u32 bit_depth_chroma;
   //for mfbc on
  mt_u32  bgsNumMinus1_luma;
  mt_u32  bgsNumMinus1_chroma;
  mt_u32  bgs2kSize_luma;
  mt_u32  bgsRowSize_luma;
  mt_u32  bgs2kSize_chroma;
  mt_u32  bgsRowSize_chroma;
}MT_DIS_FIELD_INFO_T;

typedef enum
{
    DISP_FRAME_PACKING_TYPE_NONE,             /* normal frame, not a 3D frame */
    DISP_FRAME_PACKING_TYPE_SIDE_BY_SIDE,     /* side by side */
    DISP_FRAME_PACKING_TYPE_TOP_BOTTOM,       /* top bottom */
    DISP_FRAME_PACKING_TYPE_TIME_INTERLACED,  /* time interlaced: one frame for left eye, the next frame for right eye */
    DISP_FRAME_PACKING_TYPE_BUTT
}DISP_FRAME_PACKING_TYPE_E;

/* extream value */
#define MAX_USRDAT_SIZE         1024

/* the num of dce index */
#define DCE_NUM                 130
#define XYZ_HISTDAT_SIZE        128
/* user data source */
typedef enum
{
    DISP_USD_INVALID = 0,
    DISP_USD_MP2SEQ,
    DISP_USD_MP2GOP,
    DISP_USD_MP2PIC,
    DISP_USD_MP4VSOS,
    DISP_USD_MP4VSO,
    DISP_USD_MP4VOL,
    DISP_USD_MP4GOP,
    DISP_USD_H264,
    DISP_USD_AVSSEQ,
    DISP_USD_AVSPIC
} DISP_VDEC_USD_TYPE_E;

typedef enum
{
  /*!
    H.264/AVC
  */
  FW_VIDEO_TYPE_H264,

  /*!
    AVS
  */
  FW_VIDEO_TYPE_AVS,

  /*!
    MPEG1/2
  */
  FW_VIDEO_TYPE_MPEG2,

  /*!
    MPEG4
  */
  FW_VIDEO_TYPE_MPEG4,

  /*!
    VC1
  */
  FW_VIDEO_TYPE_VC1,

  /*!
    VP8
  */
  FW_VIDEO_TYPE_VP8,

  /*!
    RV34
  */
  FW_VIDEO_TYPE_RV34,

  /*!
    HEVC
    */
  FW_VIDEO_TYPE_HEVC,

  /*!
    AVS2
    */
  FW_VIDEO_TYPE_AVS2,

  /*!
    VP9
    */
  FW_VIDEO_TYPE_VP9,

  /*!
    H266/VVC
    */
  FW_VIDEO_TYPE_H266,

  /*!
    AV1
    */
  FW_VIDEO_TYPE_AV1,

  /*!
    add new codec here
    */

  /*!
    MVC
  */
  FW_VIDEO_TYPE_MVC = 14,// should be consistent with i2c for b2b tests

  /*!
    BLUERAY
  */
  FW_VIDEO_TYPE_BLUERAY
}fw_video_coding_type_t;

typedef enum
{
  /*!
    The state of video layer is handled by user
    */
  FW_MODE_UNBLANK_USER,
  /*!
    The video layer will be displayed When AV is sync
    */
  FW_MODE_UNBLANK_SYNC,
  /*!
    The video layer will be displayed when video decoding is ready for display.
    */
  FW_MODE_UNBLANK_STABLE,
  /*!
    The video layer will be displayed when video get first I image.
  */
  FW_MODE_UNBLANK_FAST
}fw_video_open_screen_mode_t;

/*!
  Flush Display types
  */
typedef enum
{
  /*!
  FIFO Queue
    */
  MT_DRV_WIN_FLUSH_FIFO_QUEUE     = 0x00000001,

  /*!
  Display Queue
    */
  MT_DRV_WIN_FLUSH_DISPLAY_QUEUE  = 0x00000002,

  /*!
  Both FIFO and Display Queues
    */
  MT_DRV_WIN_FLUSH_BOTH           = MT_DRV_WIN_FLUSH_FIFO_QUEUE|MT_DRV_WIN_FLUSH_DISPLAY_QUEUE,

} MT_DRV_WIN_FLUSH_TYPE_E;

typedef enum
{
  VIDEO_FORMAT_COMPONET,
  VIDEO_FORMAT_PAL,
  VIDEO_FORMAT_NTSC,
  VIDEO_FORMAT_SECAM,
  VIDEO_FORMAT_MAC,
  VIDEO_FORMAT_UNSPECIFIED
}fw_video_format_t;

/* userdata desc. */
typedef struct
{
    mt_u8   data[MAX_USRDAT_SIZE]; /* USRDAT data entity */
    mt_u8   pic_coding_type;
    mt_u8   top_field_first;
    mt_u32  pic_num_count;
    mt_u32  data_size;             /* USRDAT size, in byte */
    mt_u64  PTS;                   /* pts of the frame containning the userdata */
#if 0
    mt_u32  dnr_used_flag;         /* internal used only, ignore */
    DISP_VDEC_USD_TYPE_E  from;         /* USRDAT source */
    mt_u32  seq_cnt;               /** to be removed later */
    mt_u32  seq_img_cnt;
    /* for CC, valid when IsRegistered=1 */              //y00226912
    mt_s8    IsRegistered;
    mt_u8    itu_t_t35_country_code;
    mt_u8    itu_t_t35_country_code_extension_byte;
    mt_u16  itu_t_t35_provider_code;
#endif
} MT_VDEC_USRDAT_S;

/* lcevcdata desc. */
typedef struct
{
    mt_u8    bValid;
    mt_u8*   data;
    mt_u32   data_size;             /* USRDAT size, in byte */
    mt_u64   PTS;                   /* pts of the frame containning the userdata */
    mt_u32    numReorderPics;
} MT_VDEC_LCEVC_DATA_S;

typedef struct
{
  mt_u32 bEnable;
  mt_u32 left;
  mt_u32 top;
  mt_u32 width;
  mt_u32 height;
}MT_VIDEO_CROP_INFO_T;

// ITU-T H.265 D.2.28
typedef struct
{
  mt_u8 colour_volume_enable;
  mt_u32 display_primaries_x[3];
  mt_u32 display_primaries_y[3];
  mt_u32 white_point_x;
  mt_u32 white_point_y;
  mt_u32 max_luminance;
  mt_u32 min_luminance;
}fw_mastering_disp_colour_volume_t;

#define MAX_NUM_WINDOWS 3
#define MAX_RAW_DYNAMIC_METADATA_PAYLOAD_SIZE 500
#define MAX_SL_HDR_METADAT_SIZE 800
#define MAX_SEI_HDR10P_METADAT_BUF_SIZE MAX_SL_HDR_METADAT_SIZE // MAX(MAX_SL_HDR_METADAT_SIZE,MAX_RAW_DYNAMIC_METADATA_PAYLOAD_SIZE)

typedef struct
{
  mt_u8  vsif_data_valid;
  mt_u32 targeted_system_display_maximum_luminance;
  mt_u32 average_maxrgb;
  mt_u32 distribution_values[9];
  mt_u32 num_bezier_curve_anchors;
  mt_u32 knee_point_x;
  mt_u32 knee_point_y;
  mt_u32 bezier_curve_anchors[9];
}dynamic_metadata_vsif_t;

typedef struct
{
  MT_U32 payload_size;
  MT_U8  sei_payload[MAX_SEI_HDR10P_METADAT_BUF_SIZE];
  MT_U8  hdr_dynamic_metadata_type;
  dynamic_metadata_vsif_t dynamic_metadata_vsif;
}dynamic_metadata_common_t;

typedef struct
{
  MT_U8  num_windows;
  MT_U16 window_upper_left_corner_x[MAX_NUM_WINDOWS];
  MT_U16 window_upper_left_corner_y[MAX_NUM_WINDOWS];
  MT_U16 window_lower_right_corner_x[MAX_NUM_WINDOWS];
  MT_U16 window_lower_right_corner_y[MAX_NUM_WINDOWS];
  MT_U16 center_of_ellipse_x[MAX_NUM_WINDOWS];
  MT_U16 center_of_ellipse_y[MAX_NUM_WINDOWS];
  MT_U8  rotation_angle[MAX_NUM_WINDOWS];
  MT_U16 semimajor_axis_internal_ellipse[MAX_NUM_WINDOWS];
  MT_U16 semimajor_axis_external_ellipse[MAX_NUM_WINDOWS];
  MT_U16 semiminor_axis_external_ellipse[MAX_NUM_WINDOWS];
  MT_U8  overlap_process_option[MAX_NUM_WINDOWS];

  MT_U32 targeted_system_display_maximum_luminance;
  MT_U8  targeted_system_display_actual_peak_luminance_flag;
  MT_U8  num_rows_targeted_system_display_actual_peak_luminance;
  MT_U8  num_cols_targeted_system_display_actual_peak_luminance;
  MT_U8  targeted_system_display_actual_peak_luminance[25][25];

  MT_U32 maxscl[MAX_NUM_WINDOWS][3];
  MT_U32 average_maxrgb[MAX_NUM_WINDOWS];
  MT_U8  num_distributions[MAX_NUM_WINDOWS];
  MT_U8  distribution_index[MAX_NUM_WINDOWS][16];
  MT_U32 distribution_values[MAX_NUM_WINDOWS][16];
  MT_U16 fraction_bright_pixels[MAX_NUM_WINDOWS];

  MT_U8  mastering_display_actual_peak_luminance_flag;
  MT_U8  num_rows_mastering_display_actual_peak_luminance;
  MT_U8  num_cols_mastering_display_actual_peak_luminance;
  MT_U8  mastering_display_actual_peak_luminance[25][25];

  MT_U8  tone_mapping_flag[MAX_NUM_WINDOWS];
  MT_U16 knee_point_x[MAX_NUM_WINDOWS];
  MT_U16 knee_point_y[MAX_NUM_WINDOWS];
  MT_U8  num_bezier_curve_anchors[MAX_NUM_WINDOWS];
  MT_U16 bezier_curve_anchors[MAX_NUM_WINDOWS][16];
  MT_U8  color_saturation_mapping_flag[MAX_NUM_WINDOWS];
  MT_U8  color_saturation_weight[MAX_NUM_WINDOWS];
}sei_dynamic_hdr_metadata_t;

/////////////////////////////////////////////////

typedef struct
{
    MT_U32 system_start_code;
    MT_U32 minimum_maxrgb;
    MT_U32 average_maxrgb;
    MT_U32 variance_maxrgb;
    MT_U32 maximum_maxrgb;
    MT_U32 tone_mapping_mode;
    MT_U32 tone_mapping_param_num;
    MT_U32 targeted_system_display_maximum_luminance[2];
    MT_U32 Base_flag[4];
    MT_U32 Base_param_m_p[2];
    MT_U32 Base_param_m_m[2];
    MT_U32 Base_param_m_a[2];
    MT_U32 Base_param_m_b[2];
    MT_U32 Base_param_m_n[2];
    MT_U32 Base_param_K1[2];
    MT_U32 Base_param_K2[2];
    MT_U32 Base_param_K3[2];
    MT_U32 base_param_Delta_mode[2];
    MT_U32 base_param_Delta[2];
    MT_U32 P3Spline_flag[2];
    MT_U32 P3Spline_num[2];
    MT_U32 P3Spline_TH_mode[2][4];
    MT_U32 P3Spline_TH_MB[2][4];
    MT_U32 P3Spline_TH[2][4][3];
    MT_U32 P3Spline_Strength[2][4];
    MT_U32 color_saturation_mapping_flag;
    MT_U32 color_saturation_num;
    MT_U32 color_saturation_gain[16];
}CuvaMetadata;

//techni hdr
typedef struct
{
  MT_S32 tmInputSignalBlackLevelOffset;
  MT_S32 tmInputSignalWhiteLevelOffset;
  MT_S32 shadowGain;
  MT_S32 highlightGain;
  MT_S32 midToneWidthAdjFactor;
  MT_S32 tmOutputFineTuningNumVal;
  MT_S32 tmOutputFineTuningX[10];
  MT_S32 tmOutputFineTuningY[10];
  MT_S32 saturationGainNumVal;
  MT_S32 saturationGainX[6];
  MT_S32 saturationGainY[6];
}sl_hdr_metadata_variables_t;

typedef struct
{
  MT_S32 luminanceMappingNumVal;
  MT_S32 luminanceMappingX[65];
  MT_S32 luminanceMappingY[65];
  MT_S32 colourCorrectionNumVal;
  MT_S32 colourCorrectionX[65];
  MT_S32 colourCorrectionY[65];
}sl_hdr_metadata_tables_t;

typedef struct
{
  MT_S32 partID;
  MT_S32 majorSpecVersionID;
  MT_S32 minorSpecVersionID;
  MT_S32 payloadMode;
  MT_S32 hdrPicColourSpace;
  MT_S32 hdrDisplayColourSpace;
  MT_S32 hdrDisplayMaxLuminance;
  MT_S32 hdrDisplayMinLuminance;
  MT_S32 sdrPicColourSpace;
  MT_S32 sdrDisplayColourSpace;
  MT_S32 sdrDisplayMaxLuminance;
  MT_S32 sdrDisplayMinLuminance;
  MT_S32 matrixCoefficient[4];
  MT_S32 chromaToLumaInjection[2];
  MT_S32 kCoefficient[3];
  union
  {
    sl_hdr_metadata_variables_t variables;
    sl_hdr_metadata_tables_t tables;
  } u;
}sl_hdr_metadata_t;

typedef struct
{
  dynamic_metadata_common_t hdr_dynmaic_metadata_common;
  union
  {
    sl_hdr_metadata_t sl_hdr_metadata_type2;
    sei_dynamic_hdr_metadata_t dynamic_hdr_metadata_type4;
    CuvaMetadata dynamic_hdr_vivid_metadata;
  }u;
}hdr_metadata_t;

// ITU-T H.265 D.2.35
typedef struct
{
  mt_u8 light_level_info_enable;
  mt_u32 max_light_level;
  mt_u32 max_pic_ave_light_level;
}fw_content_light_level_info_t;

/* MUST same as DIS_FRAME_SLOT_INFO_T! */
typedef struct
{
  mt_u32 frm_cnt;
  mt_u32 gop_id;             // mpeg2:temporal_reference, avs:picture_distance,mpeg4:time_increment
  mt_u64 pts;                // in unit of us
  mt_u32 pic_width;
  mt_u32 pic_height;
  mt_u32 row_jump_value;
  mt_u32 row_jump_offset;
  mt_u16 ds_pic_width;
  mt_u16 ds_pic_height;
  mt_u16 ds_stride;
  mt_u8  ds_enable;
  mt_u16 ds_ratio;
  MT_DIS_FIELD_INFO_T filedInfoTop;
  MT_DIS_FIELD_INFO_T filedInfoBot;
  MT_DIS_FIELD_INFO_T filedInfoTopRight;
  MT_DIS_FIELD_INFO_T filedInfoBotRight;
  mt_u32 display_order_mode_valid;
  MT_DIS_ORDER_MODE_E display_order_mode;
  mt_u32 repeat_frm_num;     // case FW_DIS_FRAME_N_time, repeat_frm_num show repeat times

  mt_u8 frame_pic_flag;      // 1:frame 0:field
  mt_u8 progressive_frame;
  mt_u8 dirty_flag;          // 1: frame dirty, decode with error  0: frame clean
  mt_u8 progressive_sequence;// When a new sequence is coming, main_progressive_sequence will
                             // be changed in decode interrupt, but the progreesive_sequence
                             // of this frame may still be used in display interrupt.
                             // So we should store it for this frame.
  mt_u8 top_field_first;     // 1: top field first  0: bottom field first
  mt_u8 picture_coding_type; // 0:I, 1:P, 2:B
  mt_u8 aspect_ratio;        // 1-16:9, 2-4:3
  mt_u8 fw_disable_di;
  mt_u8 filed_storage_mode;  // 0: two filed merged storage, 1: two filed separate storage
  mt_u8 isHalf;              // 0 ---null item, 1---top half item, 2---bottom half item, 3---full item
  mt_u8 is_3D_flag;          // 0: no 3D, 1: 3D
  DISP_FRAME_PACKING_TYPE_E packing_type;

  mt_u8 tile_cfg;
  mt_u8 col_size;
  mt_u32 frame_size;
  mt_u32 input_rate;
  //video_coding_type_t video_type;
//FIXME:
//  mt_u32 (*p_dcedat)[DCE_NUM];
//  mt_u32 *p_dcedat;
  mt_u32 dcedat[DCE_NUM];
  MT_VDEC_USRDAT_S* p_usrdat[4];

  mt_u32 sar_width;									// SAR: Sample Aspect Ratio(ITU-T H.265 Table E.1)
  mt_u32 sar_height;
  mt_u8 active_format_flag;							// ANSI/SCTE 128-1 2018 Table 19
  mt_u8 active_format;								// A 4 bit field describing the area of interest
  													// in terms of its aspect ratio within the coded
  													// frame as defined in AVC
  mt_u8 end_of_stream_flag;

  //first frame of stream output
  mt_u8 first_of_stream;

  mt_u32 vdec_mfbc_enable;

  fw_video_coding_type_t dec_type;

  mt_u8 uCurBank;

  MT_VIDEO_CROP_INFO_T crop_info;

  // for HDR
  mt_u32 transfer_characteristics;            //(ITU-T H.265 Table E.4)
                                                                  //HDR PQ10
                                                                  //HDR HLG
                                                                  //SDR
  mt_u32 colour_primaries;                      //(ITU-T H.265 Table E.3)
                                                                //BT709
                                                                //BT601
                                                                //BT2020
  mt_u32 sub_colour_primaries_601; //PAL OR NTSC
  mt_u32 sub_colour_primaries_2020; //0: NCL, 1: CL
  fw_mastering_disp_colour_volume_t colour_volume;    // ITU-T H.265 D.2.28
  fw_content_light_level_info_t light_level;                      // ITU-T H.265 D.2.35

  //hdr10p
  mt_u8 hdr10p_enable;
  mt_u8 hdr10p_dynamic_metadata_valid_flag;
  ulong hdr10p_common_info_addr;  //storage hdr_metadata_t
    // hdr techni hdr
  mt_u32 sl_hdr_metadata_recovery_flag;
  mt_s32 sl_hdr_yuv_range;

  fw_video_format_t video_format;
  mt_u8 video_range;
}MT_DIS_FRAME_SLOT_INFO_T;

typedef struct mtDRV_LINEAR_FRAME_ADDR_S
{
    mt_u32             u32YAddr;    /**<Address of the Y component in the current frame*/
    mt_u32             u32CAddr;    /**<Address of the C component in the current frame*/
    mt_u32             u32CrAddr;   /**<Address of the Cr component in the current frame*/

    mt_u32             u32YStride;  /**<Stride of the Y component*/
    mt_u32             u32CStride;  /**<Stride of the C component*/
    mt_u32             u32CrStride; /**<Stride of the Cr component*/
    mt_u32             u32BufSize;  /**<Stride of the Cr component*/

}MT_DRV_LINEAR_FRAME_ADDR_S;

typedef struct mtDRV_VIDEO_FRAME_S
{
    mt_u32 u32FrameNo;
    mt_u32 u32FrameIndex; //frame display index
    MT_DRV_LINEAR_FRAME_ADDR_S stLBufAddr[MT_DRV_BUF_ADDR_MAX];
    /* stBufAddr[1] is right eye for stereo video */
    MT_DRV_VID_FRAME_ADDR_S stBufAddr[MT_DRV_BUF_ADDR_MAX];
    mt_u32 u32TunnelPhyAddr;
    mt_handle hTunnelSrc;
    mt_u32 u32Width;
    mt_u32 u32Height;

    /** @deprecated Use {@link #u64Pts} instead. */
    mt_u32 u32SrcPts;  /* 0xffffffff means unknown */
    /** @deprecated Use {@link #u64Pts} instead. */
    mt_u32 u32Pts;     /* 0xffffffff means unknown */
    /** @deprecated Use {@link #u64Pts} instead. */
    mt_s64 s64OmxPts;  /* for OMX */
    mt_u64 u64Pts;	   /* PTS in unit of us */
    mt_u32 u32AspectWidth;
    mt_u32 u32AspectHeight;

    mt_u32 u32FrameRate;     /* in 1/100 Hz, 0 means unknown */
    mt_u8 end_of_stream_flag;

    MT_DRV_PIX_FORMAT_E ePixFormat;
    MT_BOOL bProgressive;
    MT_DRV_FIELD_MODE_E enFieldMode;
    MT_BOOL bTopFieldFirst;

    MT_BOOL                 bCompressd;
    MT_DRV_PIXEL_BITWIDTH_E enBitWidth;

    //display region in rectangle (x,y,w,h)
    mt_rect_s stDispRect;

    MT_DRV_FRAME_TYPE_E eFrmType;          // 2D or 3D

    mt_u32 u32Circumrotate;

    MT_BOOL bToFlip_H;
    MT_BOOL bToFlip_V;
    /*
    MT_DRV_ROT_ANGLE_E eRotAngle;
    */
    mt_u32 u32ErrorLevel;
    mt_u32 u32Priv[DEF_MT_DRV_FRAME_INFO_SIZE];  /* must be 0 */
    /***********above as unf***************/

    MT_BOOL bIsFirstIFrame;
    MT_BOOL bStillFrame;
    MT_BOOL bVOBufClearFlag;

    MT_DRV_VIDEO_TB_ADJUST_E enTBAdjust;
	mt_rect_s                stLbxInfo;

    MT_DIS_FRAME_SLOT_INFO_T slotInfo;

    mt_handle hVdecHandle;
    mt_u8 bUseNewDI;
    mt_u8 bIsLcevc;
    MT_DRV_VID_FRAME_ADDR_S stLcevcBufAddr;
    mt_u32 u32LcevcPicWidth;
    mt_u32 u32LcevcPicHeight;
    mt_u8  u8LcevcFormat; //0:422,1:444,2:420,3:422_1x2, 4:422_2x1,5:400
    mt_s16 k[2][8];
    mt_u32 k_len;

    /***********below should be deleted************/
    //maybe use

    //MT_DRV_ASPECT_RATIO_S stDispAR;
    //MT_BOOL bInterlaced;

    //MT_DRV_COLOR_SPACE_E eColorSpace; /* 'MT_DRV_CS_UNKNOWN' means unknown */

    /* these member may be changed per frame */
    //mt_u32 u32FrmCnt;

    //mt_u32  u32PlayTime;  /* 0 means ignore, do not process */
    //MT_BOOL bToRelease; /* frame should be released when displayed */


    //MT_DRV_FIELD_MODE_E  eOriginField;
    //MT_RECT_S stOriginImageRect;  /* orgin image rectagle without letterbox */

} MT_DRV_VIDEO_FRAME_S;

/* video frame or field */
typedef enum mtDRV_PICTURE_TYPE_E
{
    MT_DRV_PICTURE_FIELD = 0,
    MT_DRV_PICTURE_FRAME,
    MT_DRV_PICTURE_BUTT
}MT_DRV_PICTURE_TYPE_E;

typedef enum mtDRV_SAMPLE_TYPE_E
{
    MT_DRV_SAMPLE_TYPE_UNKNOWN,                  /**<Unknown*/ /**<CNcomment: 未知采样方式*/
    MT_DRV_SAMPLE_TYPE_PROGRESSIVE,              /**<Progressive*/ /**<CNcomment: 采样方式为逐行*/
    MT_DRV_SAMPLE_TYPE_INTERLACE,                /**<Interlaced*/ /**<CNcomment: 采样方式为隔行*/
    MT_DRV_SAMPLE_TYPE_BUTT
}MT_DRV_SAMPLE_TYPE_E;

typedef enum
{
  /*!
    SD NTSC J mode
    */
  VID_SYS_NTSC_J,
  /*!
    SD NTSC M mode
    */
  VID_SYS_NTSC_M,
  /*!
    SD NTSC 443 mode
    */
  VID_SYS_NTSC_443,
  /*!
    SD normal PAL mode
    */
  VID_SYS_PAL,
  /*!
    SD PAL N mode
    */
  VID_SYS_PAL_N,
  /*!
    SD PAL Nc mode
    */
  VID_SYS_PAL_NC,
  /*!
    SD PAL M mode
    */
  VID_SYS_PAL_M,
  /*!
    SD SECAM mode
    */
  VID_SYS_SECAM,
  /*!
    HD 1080i
    */
  VID_SYS_1080I,
  /*!
    HD 1080i 50Hz, 1125 sample per line, SMPTE 274M
    */
  VID_SYS_1080I_50HZ,
  /*!
    HD 1080p 60/59.94Hz, SMPTE 274M-1998
    */
  VID_SYS_1080P,
  /*!
    HD 1080p 24Hz, 2750 sample per line, SMPTE 274M-1998
    */
  VID_SYS_1080P_24HZ,
  /*!
    HD 1080p 25Hz, 2640 sample per line, SMPTE 274M-1998
    */
  VID_SYS_1080P_25HZ,
  /*!
    HD 1080p 30Hz, 2200 sample per line, SMPTE 274M-1998
    */
  VID_SYS_1080P_30HZ,
  /*!
    HD 1080p 50Hz
    */
  VID_SYS_1080P_50HZ,
  /*!
    HD 1250i 50Hz, another 1080i_50hz standard SMPTE 295M
    */
  VID_SYS_1250I_50HZ,
  /*!
    HD 720p
    */
  VID_SYS_720P,
  /*!
    HD 720p 23.976/24Hz, 750 line, SMPTE 296M
    */
  VID_SYS_720P_24HZ,
  /*!
    HD 720p 25Hz, 750 line, SMPTE 296M
    */
  VID_SYS_720P_25HZ,
  /*!
    HD 720p 30Hz, 750 line, SMPTE 296M
    */
  VID_SYS_720P_30HZ,
  /*!
    HD 720p 50Hz (Australia)
    */
  VID_SYS_720P_50HZ,
  /*!
    SD 576p 50Hz (Australia)
    */
  VID_SYS_576P_50HZ,
  /*!
    HD 480p
    */
  VID_SYS_480P,
  /*!
    NTSC 240p
    */
  VID_SYS_240P_60HZ,
  /*!
    PAL 288p
    */
  VID_SYS_288P_50HZ,
  /*!
    HD 4k 3840x2160 29.97/30p
    */
  VID_SYS_3840X2160_30HZ,
  /*!
    HD 4k 3840x2160 25p
    */
  VID_SYS_3840X2160_25HZ,
  /*!
    HD 4k 3840x2160 23.98/24p
    */
  VID_SYS_3840X2160_24HZ,
  /*!
    HD 4k 3840x2160 50p
    */
  VID_SYS_3840X2160_50HZ,
  /*!
    HD 4k 3840x2160 60p
    */
  VID_SYS_3840X2160_60HZ,
  /*!
    HD 4k 4096x2160  24p
    */
  VID_SYS_4096X2160_24HZ,
  /*!
    HD 4k 4096x2160  25p
    */
  VID_SYS_4096X2160_25HZ,
  /*!
    HD 4k 4096x2160  29.97/30p
    */
  VID_SYS_4096X2160_30HZ,
  /*!
    HD 4k 4096x2160  50p
    */
  VID_SYS_4096X2160_50HZ,
  /*!
    HD 4k 4096x2160  59.94/60p
    */
  VID_SYS_4096X2160_60HZ,
  /*!
    AUTO
    */
  VID_SYS_AUTO,
  /*!
    MAX
    */
  VID_SYS_MAX
} disp_sys_t;

/*private struct*/
typedef struct mtDRV_VIDEO_PRIVATE_S
{
    MT_DRV_VIDEO_ORIGINAL_INFO_S  stVideoOriginalInfo;  //Video original info
    MT_BOOL                 bValid;
    mt_u32                  u32LastFlag;
    MT_DRV_COLOR_SPACE_E    eColorSpace;        //Current Frame ColorSpace, when DTV VPSS will set this, when Atv Vicap will set this
    mt_u32                  u32BufferID;        //Buffer ID
    mt_u32                  u32FrmCnt;
    mt_u32                  u32PlayTime;
    mt_u32                  u32Fidelity;
    MT_DRV_FIELD_MODE_E     eOriginField;
    mt_rect_s stOriginImageRect;
    mt_u32                  u32PrivDispTime;    //This displaytime is for pvr smooth tplay
    MT_DRV_PICTURE_TYPE_E      ePictureMode;
    MT_DRV_SAMPLE_TYPE_E       eSampleType;
    mt_u32                  u32Reserve[32];     //Reserve Bytes
}MT_DRV_VIDEO_PRIVATE_S;

//20180820
//#define DEF_MT_DRV_FRAME_PACKAGE_MAX_FRAME_NUMBER 3
#define DEF_MT_DRV_FRAME_PACKAGE_MAX_FRAME_NUMBER 1

//add by l00225186
/*vdec frame include port handle*/
typedef struct mtDRV_VDEC_FRAME_S
{
    mt_handle hport;
    MT_DRV_VIDEO_FRAME_S stFrameVideo;
}MT_DRV_VDEC_FRAME_S;
/* video frame package */
typedef struct mtDRV_VIDEO_FRAME_PACKAGE_S
{
    mt_u32 u32FrmNum;
    MT_DRV_VDEC_FRAME_S stFrame[DEF_MT_DRV_FRAME_PACKAGE_MAX_FRAME_NUMBER];
}MT_DRV_VIDEO_FRAME_PACKAGE_S;

#define DEF_MT_DRV_VIDEO_BUFFER_MAX_NUMBER 16

/* extern frame buffer setting for virtual window,
   user alloc memory and set this setting to virtual window.*/
typedef struct mtDRV_VIDEO_BUFFER_POOL_S
{
    mt_s32 s32BufNum;
    mt_s32 s32BufStride;
    mt_s32 s32BufHeight;
    mt_u32 u32PhyAddr[DEF_MT_DRV_VIDEO_BUFFER_MAX_NUMBER];
}MT_DRV_VIDEO_BUFFER_POOL_S;

/* window private information, it could be calllback to win-sourece */
typedef struct mtDRV_WIN_PRIV_INFO_S
{
    MT_DRV_PIX_FORMAT_E ePixFmt;

    MT_BOOL   bUseCropRect;
    mt_rect_s stInRect;  /* (0,0,0,0) means full imgae, not clip */
    MT_DRV_CROP_RECT_S stCropRect;
    mt_rect_s stOutRect;

    /* may change when window lives */
    MT_DRV_ASPECT_RATIO_S stCustmAR;
    MT_DRV_ASP_RAT_MODE_E enARCvrs;

    /* external buffer config */
    MT_BOOL bUseExtBuf;
    MT_DRV_VIDEO_BUFFER_POOL_S stExtBufPool;

    /* Display Info */
    MT_BOOL   bInterlaced;
    mt_rect_s stScreen;
    MT_DRV_ASPECT_RATIO_S stScreenAR;
    MT_BOOL  bIn3DMode;
    MT_BOOL bTunnelSupport;
	MT_DRV_ROT_ANGLE_E enRotation;
    MT_BOOL bVertFlip;
    MT_BOOL bHoriFlip;

    /*Display MaxRate*/
    mt_u32 u32MaxRate;     /* in 1/100 Hz', if 0, full rate */
    MT_BOOL bCompressFlag;     /* in 1/100 Hz', if 0, full rate */
}MT_DRV_WIN_PRIV_INFO_S;


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MT_DRV_VIDEO_H__ */



