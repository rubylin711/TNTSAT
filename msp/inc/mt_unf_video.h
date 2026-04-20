/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_UNF_VIDEO_H__
#define __MT_UNF_VIDEO_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "mt_common.h"

/*************************** Structure Definition ****************************/
/** \addtogroup      VIDEO */
/** @{ */  /** <!-- [VIDEO] */

/**Defines the oversample mode.*/
/**CNcomment: 定义过采样模式*/
typedef enum mtUNF_OVERSAMPLE_MODE_E
{
    MT_UNF_OVERSAMPLE_1X = 0,
    MT_UNF_OVERSAMPLE_2X,
    MT_UNF_OVERSAMPLE_4X,
    MT_UNF_OVERSAMPLE_BUTT,
} MT_UNF_OVERSAMPLE_MODE_E;

/**Defines the pixel width.*/
/**CNcomment: 定义像素位宽*/
typedef enum mtUNF_PIXEL_BITWIDTH_E
{
    MT_UNF_PIXEL_BITWIDTH_8BIT = 0,
    MT_UNF_PIXEL_BITWIDTH_10BIT,
    MT_UNF_PIXEL_BITWIDTH_12BIT,
    MT_UNF_PIXEL_BITWIDTH_BUTT,
} MT_UNF_PIXEL_BITWIDTH_E;

/**Defines color space enum*/
/**CNcomment: 定义颜色空间枚举*/
typedef enum mtUNF_COLOR_SPACE_E
{
    MT_UNF_COLOR_SPACE_UNKNOWN = 0,

    MT_UNF_COLOR_SPACE_BT601_YUV_LIMITED = 0x10,/* ::::Current Used:::: BT.601  */
    MT_UNF_COLOR_SPACE_BT601_YUV_FULL,
    MT_UNF_COLOR_SPACE_BT601_RGB_LIMITED,
    MT_UNF_COLOR_SPACE_BT601_RGB_FULL,

    MT_UNF_COLOR_SPACE_NTSC1953 = 0x20,

    /* These should be useful.  Assume 601 extents. */
    MT_UNF_COLOR_SPACE_BT470_SYSTEM_M = 0x30,
    MT_UNF_COLOR_SPACE_BT470_SYSTEM_BG,

    MT_UNF_COLOR_SPACE_BT709_YUV_LIMITED = 0x40,/* ::::Current Used:::: BT.709 */
    MT_UNF_COLOR_SPACE_BT709_YUV_FULL,
    MT_UNF_COLOR_SPACE_BT709_RGB_LIMITED,
    MT_UNF_COLOR_SPACE_BT709_RGB_FULL,   /* ::::Current Used:::: */

    MT_UNF_COLOR_SPACE_REC709 = 0x50,      /* HD and modern captures. */

    MT_UNF_COLOR_SPACE_SMPT170M= 0x60, /* ITU-R 601 -- broadcast NTSC/PAL */
    MT_UNF_COLOR_SPACE_SMPT240M, /* 1125-Line (US) HDTV */

    MT_UNF_COLOR_SPACE_BT878 = 0x70,    /* broken BT878 extents
                           (601, luma range 16-253 instead of 16-235) */

    MT_UNF_COLOR_SPACE_XVYCC = 0x80,

    /* I know there will be cameras that send this.  So, this is
     * unspecified chromaticities and full 0-255 on each of the
     * Y'CbCr components
     */
    MT_UNF_COLOR_SPACE_JPEG = 0x90,
    MT_UNF_COLOR_SPACE_RGB = 0xa0,

    MT_UNF_COLOR_SPACE_BUTT
} MT_UNF_COLOR_SPACE_E;

/**Defines the RGB range.*/
/**CNcomment: 定义RGB 范围*/
typedef enum mtUNF_RGB_RANGE_E
{
	MT_UNF_RGB_DEFAULT,
	MT_UNF_RGB_LIMIT_RANGE,
	MT_UNF_RGB_FULL_RANGE,
	MT_UNF_RGB_RANGE_BUTT
} MT_UNF_RGB_RANGE_E;
/**Defines the video norm.*/
/**CNcomment: 定义视频制式枚举*/
typedef enum mtUNF_ENC_FMT_E
{
    MT_UNF_ENC_FMT_1080P_60 = 0,     /**<1080p 60 Hz*/
    MT_UNF_ENC_FMT_1080P_59_94,      /**<1080p 59.94 Hz*/
    MT_UNF_ENC_FMT_1080P_50,         /**<1080p 50 Hz*/
    MT_UNF_ENC_FMT_1080P_30,         /**<1080p 30 Hz*/
    MT_UNF_ENC_FMT_1080P_29_97,      /**<1080p 29.97 Hz*/
    MT_UNF_ENC_FMT_1080P_25,         /**<1080p 25 Hz*/
    MT_UNF_ENC_FMT_1080P_24,         /**<1080p 24 Hz*/

    MT_UNF_ENC_FMT_1080i_60,         /**<1080i 60 Hz*/
    MT_UNF_ENC_FMT_1080i_59_94,      /**<1080i 59.94 Hz*/
    MT_UNF_ENC_FMT_1080i_50,         /**<1080i 50 Hz*/

    MT_UNF_ENC_FMT_720P_60,          /**<720p 60 Hz*/
    MT_UNF_ENC_FMT_720P_59_94,       /**<720p 59.94 Hz*/
    MT_UNF_ENC_FMT_720P_50,          /**<720p 50 Hz */

    MT_UNF_ENC_FMT_576P_50,          /**<576p 50 Hz*/
    MT_UNF_ENC_FMT_480P_60,          /**<480p 60 Hz*/

    MT_UNF_ENC_FMT_PAL,              /* B D G H I PAL */
    MT_UNF_ENC_FMT_PAL_N,            /* (N)PAL        */
    MT_UNF_ENC_FMT_PAL_Nc,           /* (Nc)PAL       */

    MT_UNF_ENC_FMT_NTSC,             /* (M)NTSC       */
    MT_UNF_ENC_FMT_NTSC_J,           /* NTSC-J        */
    MT_UNF_ENC_FMT_NTSC_PAL_M,       /* (M)PAL        */
    MT_UNF_ENC_FMT_NTSC_443,

    MT_UNF_ENC_FMT_SECAM_SIN,        /**< SECAM_SIN*/
    MT_UNF_ENC_FMT_SECAM_COS,        /**< SECAM_COS*/

    MT_UNF_ENC_FMT_1080P_24_FRAME_PACKING,
    MT_UNF_ENC_FMT_720P_60_FRAME_PACKING,
    MT_UNF_ENC_FMT_720P_50_FRAME_PACKING,

    MT_UNF_ENC_FMT_861D_640X480_60,
    MT_UNF_ENC_FMT_VESA_800X600_60,
    MT_UNF_ENC_FMT_VESA_1024X768_60,
    MT_UNF_ENC_FMT_VESA_1280X720_60,
    MT_UNF_ENC_FMT_VESA_1280X800_60,
    MT_UNF_ENC_FMT_VESA_1280X1024_60,
    MT_UNF_ENC_FMT_VESA_1360X768_60,
    MT_UNF_ENC_FMT_VESA_1366X768_60,
    MT_UNF_ENC_FMT_VESA_1400X1050_60,
    MT_UNF_ENC_FMT_VESA_1440X900_60,
    MT_UNF_ENC_FMT_VESA_1440X900_60_RB,
    MT_UNF_ENC_FMT_VESA_1600X900_60_RB,
    MT_UNF_ENC_FMT_VESA_1600X1200_60,
    MT_UNF_ENC_FMT_VESA_1680X1050_60,
    MT_UNF_ENC_FMT_VESA_1680X1050_60_RB,
    MT_UNF_ENC_FMT_VESA_1920X1080_60,
    MT_UNF_ENC_FMT_VESA_1920X1200_60,
    MT_UNF_ENC_FMT_VESA_1920X1440_60,
    MT_UNF_ENC_FMT_VESA_2048X1152_60,
    MT_UNF_ENC_FMT_VESA_2560X1440_60_RB,
    MT_UNF_ENC_FMT_VESA_2560X1600_60_RB,

    MT_UNF_ENC_FMT_3840X2160_24 = 0x30,
    MT_UNF_ENC_FMT_3840X2160_25,
    MT_UNF_ENC_FMT_3840X2160_29_97,
    MT_UNF_ENC_FMT_3840X2160_30,
    MT_UNF_ENC_FMT_3840X2160_50,
    MT_UNF_ENC_FMT_3840X2160_59_94,
    MT_UNF_ENC_FMT_3840X2160_60,
    MT_UNF_ENC_FMT_4096X2160_24,
    MT_UNF_ENC_FMT_4096X2160_25,
    MT_UNF_ENC_FMT_4096X2160_29_97,
    MT_UNF_ENC_FMT_4096X2160_30,
    MT_UNF_ENC_FMT_4096X2160_50,
    MT_UNF_ENC_FMT_4096X2160_59_94,
    MT_UNF_ENC_FMT_4096X2160_60,

    MT_UNF_ENC_FMT_BUTT
}MT_UNF_ENC_FMT_E;

/**Defines the mode of the video decoder.*/
/**CNcomment: 定义视频解码器模式枚举*/
typedef enum mtUNF_VCODEC_MODE_E
{
    MT_UNF_VCODEC_MODE_NORMAL = 0,   /**<Decode all frames.*/ /**<CNcomment: 解所有帧*/
    MT_UNF_VCODEC_MODE_IP,           /**<Decode only I frames and P frames.*/ /**<CNcomment: 只解IP帧*/
    MT_UNF_VCODEC_MODE_I,            /**<Decode only I frames.*/ /**<CNcomment: 只解I帧*/
    MT_UNF_VCODEC_MODE_DROP_INVALID_B,  /**<Decode all frames except the first B frame sequence behind I frames */ /**<CNcomment: 解所有帧，除了紧跟着I帧后面的B帧*/
    MT_UNF_VCODEC_MODE_BUTT
}MT_UNF_VCODEC_MODE_E;

/**Defines special control operation of decoder*/
/**CNcomment: 定义解码器解码的特殊控制选项 */
typedef enum mtMT_UNF_VCODEC_CTRL_OPTION_E
{
    MT_UNF_VCODEC_CTRL_OPTION_SIMPLE_DPB = 0x1,

} MT_UNF_VCODEC_CTRL_OPTION_E;

/**Defines the decoding capability (resolution) of the decoder.*/
/**CNcomment: 定义解码器解码能力(分辨率) */
typedef enum mtUNF_VCODEC_CAP_LEVEL_E
{
    MT_UNF_VCODEC_CAP_LEVEL_NULL = 0, /**<Do not decode.*/ /**<CNcomment: 不需要解码 */
    MT_UNF_VCODEC_CAP_LEVEL_QCIF = 0, /**<The resolution of the picture to be decoded is less than or equal to 176x144.*/ /**<CNcomment: 解码的图像大小不超过176*144 */
    MT_UNF_VCODEC_CAP_LEVEL_CIF,      /**<The resolution of the picture to be decoded less than or equal to 352x288.*/ /**<CNcomment: 解码的图像大小不超过352*288 */
    MT_UNF_VCODEC_CAP_LEVEL_D1,       /**<The resolution of the picture to be decoded less than or equal to 720x576.*/ /**<CNcomment: 解码的图像大小不超过720*576 */
    MT_UNF_VCODEC_CAP_LEVEL_720P,     /**<The resolution of the picture to be decoded is less than or equal to 1280x720.*/ /**<CNcomment: 解码的图像大小不超过1280*720 */
    MT_UNF_VCODEC_CAP_LEVEL_FULLHD,   /**<The resolution of the picture to be decoded is less than or equal to 1920x1080.*/ /**<CNcomment: 解码的图像大小不超过1920*1080 */

    MT_UNF_VCODEC_CAP_LEVEL_1280x800, /**<The resolution of the picture to be decoded is less than or equal to 1280x800.*/ /**<CNcomment: 解码的图像大小不超过1280x800*/
    MT_UNF_VCODEC_CAP_LEVEL_800x1280, /**<The resolution of the picture to be decoded is less than or equal to 800x1280.*/ /**<CNcomment: 解码的图像大小不超过800x1280*/
    MT_UNF_VCODEC_CAP_LEVEL_1488x1280, /**<The resolution of the picture to be decoded is less than or equal to 1488x1280.*/ /**<CNcomment: 解码的图像大小不超过1488x1280 */
    MT_UNF_VCODEC_CAP_LEVEL_1280x1488, /**<The resolution of the picture to be decoded is less than or equal to 1280x1488.*/ /**<CNcomment: 解码的图像大小不超过1280x1488 */
    MT_UNF_VCODEC_CAP_LEVEL_2160x1280, /**<The resolution of the picture to be decoded is less than or equal to 2160x1280.*/ /**<CNcomment: 解码的图像大小不超过2160x1280 */
    MT_UNF_VCODEC_CAP_LEVEL_1280x2160, /**<The resolution of the picture to be decoded is less than or equal to 1280x2160.*/ /**<CNcomment: 解码的图像大小不超过1280x2160 */
    MT_UNF_VCODEC_CAP_LEVEL_2160x2160, /**<The resolution of the picture to be decoded is less than or equal to 2160x2160.*/ /**<CNcomment: 解码的图像大小不超过2160x2160 */
    MT_UNF_VCODEC_CAP_LEVEL_4096x2160, /**<The resolution of the picture to be decoded is less than or equal to 4096x2160.*/ /**<CNcomment: 解码的图像大小不超过4096x2160 */
    MT_UNF_VCODEC_CAP_LEVEL_2160x4096, /**<The resolution of the picture to be decoded is less than or equal to 2160x4096.*/ /**<CNcomment: 解码的图像大小不超过2160x4096 */
    MT_UNF_VCODEC_CAP_LEVEL_4096x4096, /**<The resolution of the picture to be decoded is less than or equal to 4096x4096.*/ /**<CNcomment: 解码的图像大小不超过4096x4096 */
    MT_UNF_VCODEC_CAP_LEVEL_8192x4096, /**<The resolution of the picture to be decoded is less than or equal to 8192x4096.*/ /**<CNcomment: 解码的图像大小不超过8192x4096 */
    MT_UNF_VCODEC_CAP_LEVEL_4096x8192, /**<The resolution of the picture to be decoded is less than or equal to 4096x8192.*/ /**<CNcomment: 解码的图像大小不超过4096x8192 */
    MT_UNF_VCODEC_CAP_LEVEL_8192x8192, /**<The resolution of the picture to be decoded is less than or equal to 8192x8192.*/ /**<CNcomment: 解码的图像大小不超过8192x8192 */

    MT_UNF_VCODEC_CAP_LEVEL_BUTT
} MT_UNF_VCODEC_CAP_LEVEL_E;

/**Defines the stream type supported by the decoder.*/
/**CNcomment: 定义解码器支持的码流类型 */
typedef enum mtUNF_VCODEC_TYPE_E
{
    MT_UNF_VCODEC_TYPE_MPEG2,       /**<MPEG2*/
    MT_UNF_VCODEC_TYPE_MPEG4,       /**<MPEG4 DIVX4 DIVX5*/
    MT_UNF_VCODEC_TYPE_AVS,         /**<AVS*/
    MT_UNF_VCODEC_TYPE_H263,        /**<H263*/
    MT_UNF_VCODEC_TYPE_H264,        /**<H264*/
    MT_UNF_VCODEC_TYPE_REAL8,       /**<REAL*/
    MT_UNF_VCODEC_TYPE_REAL9,       /**<REAL*/
    MT_UNF_VCODEC_TYPE_VC1,         /**<VC-1*/
    MT_UNF_VCODEC_TYPE_VP6,         /**<VP6*/
    MT_UNF_VCODEC_TYPE_VP6F,        /**<VP6F*/
    MT_UNF_VCODEC_TYPE_VP6A,        /**<VP6A*/
    MT_UNF_VCODEC_TYPE_MJPEG,       /**<MJPEG*/
    MT_UNF_VCODEC_TYPE_SORENSON,    /**<SORENSON SPARK*/
    MT_UNF_VCODEC_TYPE_DIVX3,       /**<DIVX3*/
    MT_UNF_VCODEC_TYPE_RAW,         /**RAW*/
    MT_UNF_VCODEC_TYPE_JPEG,        /**JPEG,added for VENC*/
    MT_UNF_VCODEC_TYPE_VP8,         /**<VP8*/
    MT_UNF_VCODEC_TYPE_MSMPEG4V1,   /**< MS private MPEG4 */
    MT_UNF_VCODEC_TYPE_MSMPEG4V2,
    MT_UNF_VCODEC_TYPE_MSVIDEO1,    /**< MS video */
    MT_UNF_VCODEC_TYPE_WMV1,
    MT_UNF_VCODEC_TYPE_WMV2,
    MT_UNF_VCODEC_TYPE_RV10,
    MT_UNF_VCODEC_TYPE_RV20,
    MT_UNF_VCODEC_TYPE_SVQ1,        /**< Apple video */
    MT_UNF_VCODEC_TYPE_SVQ3,        /**< Apple video */
    MT_UNF_VCODEC_TYPE_H261,
    MT_UNF_VCODEC_TYPE_VP3,
    MT_UNF_VCODEC_TYPE_VP5,
    MT_UNF_VCODEC_TYPE_CINEPAK,
    MT_UNF_VCODEC_TYPE_INDEO2,
    MT_UNF_VCODEC_TYPE_INDEO3,
    MT_UNF_VCODEC_TYPE_INDEO4,
    MT_UNF_VCODEC_TYPE_INDEO5,
    MT_UNF_VCODEC_TYPE_MJPEGB,
    MT_UNF_VCODEC_TYPE_MVC,
    MT_UNF_VCODEC_TYPE_HEVC,     	/**<HEVC*/
    MT_UNF_VCODEC_TYPE_DV,
    MT_UNF_VCODEC_TYPE_VP9,
    MT_UNF_VCODEC_TYPE_AVS2,
    MT_UNF_VCODEC_TYPE_BUTT
}MT_UNF_VCODEC_TYPE_E;

/**Defines the H264 profile of the video encoder.*/
/**CNcomment: 定义视频解码器 H264协议档次*/
typedef enum mtUNF_H264_PROFILE_E
{
    MT_UNF_H264_PROFILE_BASELINE = 0,   /**<Encode H264 stream by baseline profile.*/ /**<CNcomment: 采用基线档次编码H264码流*/
    MT_UNF_H264_PROFILE_MAIN,           /**<Encode H264 stream by main profile.*/     /**<CNcomment:采用主要档次编码H264码流*/
    MT_UNF_H264_PROFILE_EXTENDED,       /**<Encode H264 stream by extended profile.*/ /**<CNcomment:采用扩展档次编码H264码流*/
    MT_UNF_H264_PROFILE_HIGH,           /**<Encode H264 stream by high profile.*/     /**<CNcomment: 采用高级档次编码H264码流*/
    MT_UNF_H264_PROFILE_BUTT
}MT_UNF_H264_PROFILE_E;


typedef struct mtUNF_VCODEC_VC1_ATTR_S
{
    MT_BOOL  bAdvancedProfile;   /**<Whether the profile is an advanced profile*/ /**<CNcomment: 是否Advanced Profile*/
    mt_u32   u32CodecVersion;    /**<Version number*/ /**<CNcomment: 版本号*/
}MT_UNF_VCODEC_VC1_ATTR_S;

typedef struct mtUNF_VCODEC_VP6_ATTR_S
{
    MT_BOOL  bReversed;    /**<To reverse a picture, set this parameter to 1. In this cases, set it to 0.*/ /**<CNcomment: 图像需要倒转时置1，否则置0*/
}MT_UNF_VCODEC_VP6_ATTR_S;

typedef union mtUNF_VCODEC_EXTATTR_U
{
    MT_UNF_VCODEC_VC1_ATTR_S stVC1Attr;
    MT_UNF_VCODEC_VP6_ATTR_S stVP6Attr;
}MT_UNF_VCODEC_EXTATTR_U;

/*!
  This structure defines the behavior of video display when video decoding is ready.
  */
typedef enum mtUNF_VCODEC_UNBLANK_E
{
  /*!
    The video layer will be displayed when video get first I image.
  */
  MT_UNF_VCODEC_UNBLANK_FAST,
  /*!
	The video layer will be displayed when video decoding is ready for display.
	*/
  MT_UNF_VCODEC_UNBLANK_STABLE,
  /*!
    The video layer will be displayed When AV is sync
    */
  MT_UNF_VCODEC_UNBLANK_SYNC,
  /*!
    The state of video layer is handled by user
    */
  MT_UNF_VCODEC_UNBLANK_USER,

  MT_UNF_VCODEC_UNBLANK_BUTT
}MT_UNF_VCODEC_UNBLANK_E;


/**Defines the attributes of a video decoder.*/
/**CNcomment: 定义视频解码器属性结构*/
typedef struct mtUNF_VCODEC_ATTR_S
{
    MT_UNF_VCODEC_TYPE_E        enType;             /**<Video encoding type*/ /**<CNcomment: 视频编码类型*/
    /* not used */
    MT_UNF_VCODEC_EXTATTR_U     unExtAttr;          /**<Extra attributes related to the video encoding type*/ /**<CNcomment: 视频编码类型相关的额外属性*/
    MT_UNF_VCODEC_MODE_E        enMode;             /**<Mode of a video decoder*/ /**<CNcomment: 视频解码器模式模式*/
    mt_u32                      u32ErrCover;        /**<Error concealment threshold of the output frames of a video decoder. The value 0 indicates that no frames are output if an error occurs; the value 100 indicates that all frames are output no matter whether errors occur.*/
                                                    /**<CNcomment: 视频解码器的输出帧错误隐藏门限，0:出现错误即不输出；100:不管错误比例全部输出*/
    /* not used */
    mt_u32                      u32Priority;        /**<Priority of a video decoder. The value range is [1, MT_UNF_VCODEC_MAX_PRIORITY]. The value 0 is a reserved value. If you set the value to 0, no error message is displayed, but the value 1 is used automatically. The smaller the value, the lower the priority.*/
                                                    /**<CNcomment: 视频解码器优先级, 取值范围: 大于等于1，小于等于MT_UNF_VCODEC_MAX_PRIORITY, 0是保留值，配置为0不会报错，但是会自动取值为1,数值越小优先级越低 */
    MT_BOOL                     bOrderOutput;       /**<Whether the videos are output by the decoding sequence. You are advised to set this parameter to MT_TRUE in VP mode, and MT_FALSE in other modes.*/
                                                    /**<CNcomment: 是否按解码序输出，VP模式下推荐配置为MT_TRUE, 一般模式下配置为MT_FALSE */
    /* not used */
    mt_s32                      s32CtrlOptions;     /**<The value is MT_UNF_VCODEC_CTRL_OPTION_E, or the 'or' value of several enum type*/
                                                    /**<CNcomment: 取值为MT_UNF_VCODEC_CTRL_OPTION_E，或者几个枚举的'或'  */
    mt_u32                      u32UseDescInfoFlag; /**<VDEC use PTS descriptor flag*/ /**<CNcomment: 解码器是否使用PTS描述子标志*/
    mt_u32                      u32FrameRateInt;    /**<The value is frame rate(Integer part) */
    mt_u32                      u32FrameRateDec;    /**<The value is frame rate(Decimal part) */
    mt_u32                      u32ForceFrameRateFlag; /**<The value is user force set frame rate flag*/
    /* not used */
    mt_void*                    pCodecContext;      /**<Private codec context */
    MT_UNF_VCODEC_UNBLANK_E     enUnBlank;			/**<Unblank screen mode*/ /**<CNcomment: 视频开屏模式*/

	MT_BOOL 					bDynamicResSupport;	/**@deprecated*/
	MT_BOOL 					bForceDisableTimeout; /**<User force disable FW timeout when stream discontinue*/ /**<CNcomment: 用户强制FW在断流的情况下不做超时重置(适用于AirPlay)*/

	mt_u32						u32DebugCrcBufSize; /**<Debug for VDEC decoded frames' CRC buffer*/
	mt_u32						pip_en; /**<Debug for VDEC pip_en */
}MT_UNF_VCODEC_ATTR_S;


/**Defines the sub stream protocol.*/
/**CNcomment: 定义码流的协议分支枚举*/
typedef enum mtUNF_VIDEO_SUB_STANDARD_E
{
    MT_UNF_VIDEO_SUB_STANDARD_UNKNOWN,       /**<Unknown*/ /**<CNcomment: 未知协议分支*/
    MT_UNF_VIDEO_SUB_STANDARD_MPEG2_MPEG1,   /**<The MPEG2 protocol family is compatible with MPEG1.*/ /**<CNcomment: MPEG2协议族可以兼容MPEG1 */
    MT_UNF_VIDEO_SUB_STANDARD_MPEG4_SHV,     /**<The MPEG4 protocol family is compatible with SHV.*/ /**<CNcomment: MPEG4协议族可以兼容SHV*/
    MT_UNF_VIDEO_SUB_STANDARD_MPEG4_XVID,    /**<The MPEG4 protocol family includes the sub protocol XVID.*/ /**<CNcomment: MPEG4协议包含XVID分支*/
    MT_UNF_VIDEO_SUB_STANDARD_MPEG4_DIVX,    /**<The MPEG4 protocol family includes the sub protocol DIVX.*/ /**<CNcomment: MPEG4协议包含DIVX分支*/

    MT_UNF_VIDEO_SUB_STANDARD_BUTT
}MT_UNF_VIDEO_SUB_STANDARD_E;

/**Defines the frame rate of the video stream.*/
/**CNcomment: 定义视频码流帧率结构*/
typedef struct mtUNF_VCODEC_FRMRATE_S
{
    mt_u32 u32fpsInteger;     /**<Integral part of the frame rate (in frame/s)*/ /**<CNcomment: 码流的帧率的整数部分, fps */
    mt_u32 u32fpsDecimal;     /**<Fractional part (calculated to three decimal places) of the frame rate (in frame/s)*/
                              /**<CNcomment: 码流的帧率的小数部分（保留3位）, fps */
}MT_UNF_VCODEC_FRMRATE_S;

/**Defines the information about video streams.*/
/**CNcomment: 定义视频码流信息结构*/
typedef struct mtUNF_VCODEC_STREAMINFO_S
{
    MT_UNF_VCODEC_TYPE_E        enVCodecType;   /**<Stream type*/ /**<CNcomment: 码流类型 */
    /* not supported */
    MT_UNF_VIDEO_SUB_STANDARD_E enSubStandard;  /**<Sub stream protocol*/ /**<CNcomment: 码流的协议分支 */
    /* not supported */
    mt_u32                      u32SubVersion;  /**<Version of the sub stream protocol*/ /**<CNcomment: 码流子协议版本号 */
    /* not supported */
    mt_u32                      u32Profile;     /**<Stream profile*/ /**<CNcomment: 码流的profile */
    /* not supported */
    mt_u32                      u32Level;       /**<Stream level*/ /**<CNcomment: 码流的level */
    /* not supported */
    MT_UNF_ENC_FMT_E            enDisplayNorm;  /**<Display norm (PAL or NTSC)*/ /**<CNcomment: 显示标准(P/N) */
    MT_BOOL                     bProgressive;   /**<Sampling type (progressive or interlaced)*/ /**<CNcomment: 采样方式(逐行/隔行) */
    mt_u32                      u32AspectWidth; /**<Output aspect ratio: width*/ /**<CNcomment: 输出宽高比之宽值 */
    mt_u32                      u32AspectHeight;/**<Output aspect ratio: height*/ /**<CNcomment: 输出宽高比之高值 */

    /* not supported */
    mt_u32 u32bps;            /**<Bit rate, in kbit/s*/ /**<CNcomment: 码流的码率, Kbps */
    mt_u32 u32fpsInteger;     /**<Integral part of the frame rate (in frame/s)*/ /**<CNcomment: 码流的帧率的整数部分, fps */
    mt_u32 u32fpsDecimal;     /**<Fractional part (calculated to three decimal places) of the frame rate (in frame/s)*/ /**<CNcomment: 码流的帧率的小数部分（保留3位）, fps */
    mt_u32 u32Width;          /**<Width of the decoded picture*/ /**<CNcomment: 解码图像宽 */
    mt_u32 u32Height;         /**<Height of the decoded picture*/ /**<CNcomment: 解码图像高 */
    mt_u32 u32DisplayWidth;   /**<Width of the displayed picture*/ /**<CNcomment: 显示图像宽 */
    mt_u32 u32DisplayHeight;  /**<Height of the displayed picture*/ /**<CNcomment: 显示图像高 */
    mt_u32 u32DisplayCenterX; /**<Horizontal coordinate of the center of the displayed picture (the upper left point of the source picture serves as the coordinate origin)*/
                              /**<CNcomment: 显示图像中心横坐标，以原始图像的左上角为坐标原点 */
    mt_u32 u32DisplayCenterY; /**<Vertical coordinate of the center of the displayed picture (the upper left point of the source picture serves as the coordinate origin)*/
                              /**<CNcomment: 显示图像中心纵坐标，以原始图像的左上角为坐标原点 */
	mt_u32 u32IsHDR;		  /**< HDR stream, 16(HDR)  18(HLG) 0(not HDR) */				  
}MT_UNF_VCODEC_STREAMINFO_S;


/**Defines the types of the user data in the video information.*/
/**CNcomment: 定义视频信息中的用户数据类型 */
typedef enum mtMT_UNF_VIDEO_USERDATA_TYPE_E
{
    MT_UNF_VIDEO_USERDATA_UNKNOWN = 0,      /**<Unknown type*/ /**<CNcomment: 未知类型*/
    MT_UNF_VIDEO_USERDATA_DVB1_CC = 0x1,    /**<Closed Caption Data*/ /**<CNcomment: 字幕数据*/
    MT_UNF_VIDEO_USERDATA_DVB1_BAR = 0x2,   /**<Bar Data*/ /**<CNcomment: Bar数据*/
    MT_UNF_VIDEO_USERDATA_AFD = 0x10000,    /**<Active Format Description*/ /**<CNcomment: 模式描述数据*/
    MT_UNF_VIDEO_USERDATA_BUTT
}MT_UNF_VIDEO_USERDATA_TYPE_E;

/**Defines the profile of video broadcasting.*/
/**CNcomment: 定义视频广播的profile枚举*/
typedef enum mtUNF_VIDEO_BROADCAST_PROFILE_E
{
    MT_UNF_VIDEO_BROADCAST_DVB,        /**<Digital video broadcasting (DVB)*/ /**<CNcomment: 数字视频广播DVB*/
    MT_UNF_VIDEO_BROADCAST_DIRECTV,    /**<American live broadcast operator DirecTV*/ /**<CNcomment: 美国直播运营商DirecTV*/
    MT_UNF_VIDEO_BROADCAST_ATSC,       /**<Advanced Television Systems Committee (ATSC)*/ /**<CNcomment: 先进电视制式委员会ATSC（Advanced Television Systems Committee）*/
    MT_UNF_VIDEO_BROADCAST_DVD,        /**<Digital video disc (DVD)*/ /**<CNcomment: 数字视频光盘*/
    MT_UNF_VIDEO_BROADCAST_ARIB,       /**<Association of Radio Industries and Businesses (ARIB)*/ /**<CNcomment: 无线电工业及商业协会规格*/
    MT_UNF_VIDEO_BROADCAST_BUTT
}MT_UNF_VIDEO_BROADCAST_PROFILE_E;

/**Defines the position of the user data in the video information.*/
/**CNcomment: 定义视频信息中用户数据的位置枚举*/
typedef enum mtUNF_VIDEO_USER_DATA_POSITION_E
{
    MT_UNF_VIDEO_USER_DATA_POSITION_UNKNOWN,       /**<Unknown*/ /**<CNcomment: 未知位置*/
    MT_UNF_VIDEO_USER_DATA_POSITION_MPEG2_SEQ,     /**<The data is parsed from sequences under the MPEG2 protocol.*/ /**<CNcomment: MPEG2协议下，从序列中解出*/
    MT_UNF_VIDEO_USER_DATA_POSITION_MPEG2_GOP,     /**<The data is parsed from the group of pictures (GOP) under the MPEG2 protocol.*/ /**<CNcomment: MPEG2协议下，从GOP（Group Of Pictures）中解出*/
    MT_UNF_VIDEO_USER_DATA_POSITION_MPEG2_FRAME,   /**<The data is parsed from picture frames under the MPEG2 protocol.*/ /**<CNcomment: MPEG2协议下，从图像帧中解出*/
    MT_UNF_VIDEO_USER_DATA_POSITION_MPEG4_VSOS,    /**<The data is parsed from the sequences of visible objects under the MPEG4 protocol.*/ /**<CNcomment: MPEG4协议下，从可视对像序列中解出*/
    MT_UNF_VIDEO_USER_DATA_POSITION_MPEG4_VSO,     /**<The data is parsed from visible objects under the MPEG4 protocol.*/ /**<CNcomment: MPEG4协议下，从可视对像中解出*/
    MT_UNF_VIDEO_USER_DATA_POSITION_MPEG4_VOL,     /**<The data is parsed from the video object layer under the MPEG4 protocol.*/ /**<CNcomment: MPEG4协议下，从视频对像层中解出*/
    MT_UNF_VIDEO_USER_DATA_POSITION_MPEG4_GOP,     /**<The data is parsed from the GOP under the MPEG4 protocol.*/ /**<CNcomment: MPEG4协议下，从GOP中解出*/
    MT_UNF_VIDEO_USER_DATA_POSITION_H264_REG,      /**<The data is parsed from the user_data_regestered_itu_t_t35() syntax under the H.264 protocol.*/ /**<CNcomment: 从H.264协议的user_data_regestered_itu_t_t35()语法中解出*/
    MT_UNF_VIDEO_USER_DATA_POSITION_H264_UNREG,    /**<The data is parsed from the user_data_unregestered() syntax under the H.264 protocol.*/ /**<CNcomment: 从H.264协议的user_data_unregestered()语法中解出*/
    MT_UNF_VIDEO_USER_DATA_POSITION_BUTT
}MT_UNF_VIDEO_USER_DATA_POSITION_E;

/**Defines the structure of the user data in the video information.*/
/**CNcomment: 定义视频信息中的用户数据结构*/
typedef struct mtUNF_VIDEO_USERDATA_S
{
    MT_UNF_VIDEO_BROADCAST_PROFILE_E   enBroadcastProfile;   /**<Broadcasting profile of the user data*/ /**<CNcomment: 用户数据的广播profile*/
    MT_UNF_VIDEO_USER_DATA_POSITION_E  enPositionInStream;   /**<Position of the user data in video streams*/ /**<CNcomment: 用户数据在视频流中的位置*/
    mt_u32                             u32Pts;               /**<PTS corresponding to the user data*/ /**<CNcomment: 用户数据对应的时间戳*/
    mt_u32                             u32SeqCnt;            /**<Sequence ID of the user data*/ /**<CNcomment: 用户数据的前一序列数*/
    mt_u32                             u32SeqFrameCnt;       /**<Frame number of the user data*/ /**<CNcomment: 用户数据的前一帧数*/
    mt_u8                              *pu8Buffer;           /**<Initial address of the user data memory, output parameter*/ /**<CNcomment: 用户数据内存区的初始地址,输出参数*/
    mt_u32                             u32Length;            /**<User data size (a multiple of 1 byte)*/ /**<CNcomment: 用户数据的大小，以1byte为单位*/
    MT_BOOL                            bBufferOverflow;      /**<Indicate that whether the user data size exceeds the maximum size defined by MAX_USER_DATA_LEN.*/ /**<CNcomment: 标志用户数据的长度是否超过了MAX_USER_DATA_LEN定义的最大值*/
    MT_BOOL                            bTopFieldFirst;       /**<Top field first flag*/ /**<CNcomment: 顶场优先标志*/
}MT_UNF_VIDEO_USERDATA_S;

/**Defines the parameters of format changing */
/**CNcomment: 定义制式切换时的相关参数*/
typedef struct mtUNF_NORMCHANGE_PARAM_S
{
    MT_UNF_ENC_FMT_E            enNewFormat;            /**<New format*/ /**<CNcomment: 新的制式*/
    mt_u32                      u32ImageWidth;          /**<Width of image*/ /**<CNcomment: 图像宽度*/
    mt_u32                      u32ImageHeight;         /**<Height of image*/ /**<CNcomment: 图像高度*/
    MT_BOOL                     bProgressive;           /**<Sampling type (progressive or interlaced)*/ /**<CNcomment: 采样方式(逐行/隔行) */
    mt_u32                      u32FrameRate;           /**<Frame rate*//**<CNcomment:帧率*/
}MT_UNF_NORMCHANGE_PARAM_S;

/**Defines the video format.*/
/**CNcomment: 定义视频格式枚举*/
typedef enum mtUNF_VIDEO_FORMAT_E
{
    /* Semi-Planner */
    MT_UNF_FORMAT_YUV_SEMIPLANAR_422,       /**<The YUV spatial sampling format is 4:2:2.*/ /**<CNcomment: YUV空间采样格式为4:2:2*/
    MT_UNF_FORMAT_YUV_SEMIPLANAR_420,       /**<The YUV spatial sampling format is 4:2:0, V first.*/ /**<CNcomment: YUV空间采样格式为4:2:0，V在低位*/
    MT_UNF_FORMAT_YUV_SEMIPLANAR_400,
    MT_UNF_FORMAT_YUV_SEMIPLANAR_411,
    MT_UNF_FORMAT_YUV_SEMIPLANAR_422_1X2,
    MT_UNF_FORMAT_YUV_SEMIPLANAR_444,
    MT_UNF_FORMAT_YUV_SEMIPLANAR_420_UV,   /**<The YUV spatial sampling format is 4:2:0,U first.*/ /**<CNcomment: YUV空间采样格式为4:2:0, U在低位*/


    /* Package */
    MT_UNF_FORMAT_YUV_PACKAGE_UYVY,         /**<The YUV spatial sampling format is package, and the pixel arrangement sequence in the memory is UYVY.*/ /**<CNcomment: YUV空间采样格式为package,内存排列为UYVY*/
    MT_UNF_FORMAT_YUV_PACKAGE_YUYV,         /**<The YUV spatial sampling format is package, and the pixel arrangement sequence in the memory is YUYV.*/ /**<CNcomment: YUV空间采样格式为package,内存排列为YUYV*/
    MT_UNF_FORMAT_YUV_PACKAGE_YVYU,         /**<The YUV spatial sampling format is package, and the pixel arrangement sequence in the memory is YVYU.*/ /**<CNcomment: YUV空间采样格式为package,内存排列为YVYU*/

    /* Planner */
    MT_UNF_FORMAT_YUV_PLANAR_400,
    MT_UNF_FORMAT_YUV_PLANAR_411,
    MT_UNF_FORMAT_YUV_PLANAR_420,
    MT_UNF_FORMAT_YUV_PLANAR_422_1X2,
    MT_UNF_FORMAT_YUV_PLANAR_422_2X1,
    MT_UNF_FORMAT_YUV_PLANAR_444,
    MT_UNF_FORMAT_YUV_PLANAR_410,
    MT_UNF_FORMAT_YUV_BUTT,

    MT_UNF_FORMAT_RGB_SEMIPLANAR_444,
    MT_UNF_FORMAT_RGB_BUTT
}MT_UNF_VIDEO_FORMAT_E;

/**Defines the type of the video frame.*/
/**CNcomment: 定义视频帧的类型枚举*/
typedef enum mtUNF_VIDEO_FRAME_TYPE_E
{
    MT_UNF_FRAME_TYPE_UNKNOWN,   /**<Unknown*/ /**<CNcomment: 未知的帧类型*/
    MT_UNF_FRAME_TYPE_I,         /**<I frame*/ /**<CNcomment: I帧*/
    MT_UNF_FRAME_TYPE_P,         /**<P frame*/ /**<CNcomment: P帧*/
    MT_UNF_FRAME_TYPE_B,         /**<B frame*/ /**<CNcomment: B帧*/
    MT_UNF_FRAME_TYPE_BUTT
}MT_UNF_VIDEO_FRAME_TYPE_E;

/**Defines the video frame/field mode.*/
/**CNcomment: 定义视频帧场模式枚举*/
typedef enum mtUNF_VIDEO_FIELD_MODE_E
{
    MT_UNF_VIDEO_FIELD_ALL,        /**<Frame mode*/ /**<CNcomment: 帧模式*/
    MT_UNF_VIDEO_FIELD_TOP,        /**<Top field mode*/ /**<CNcomment: 顶场模式*/
    MT_UNF_VIDEO_FIELD_BOTTOM,     /**<Bottom field mode*/ /**<CNcomment: 底场模式*/
    MT_UNF_VIDEO_FIELD_BUTT
}MT_UNF_VIDEO_FIELD_MODE_E;


typedef struct mtUNF_CAPTURE_MEM_MODE_S
{
    mt_u32              u32StartPhyAddr;        /**<start phy addr*/ /**<CNcomment: 起始物理地址*/
    mt_u32              u32StartUserAddr;       /**<start user addr*/ /**<CNcomment: 起始用户地址*/
    mt_u32              u32DataLen;             /**<len of databuf*/ /**<CNcomment: 数据区长度*/
}MT_UNF_CAPTURE_MEM_MODE_S;


/**Defines 3D frame packing type*/
typedef enum mtUNF_VIDEO_FRAME_PACKING_TYPE_E
{
    MT_UNF_FRAME_PACKING_TYPE_NONE,             /**< Normal frame, not a 3D frame */
    MT_UNF_FRAME_PACKING_TYPE_SIDE_BY_SIDE,     /**< Side by side */
    MT_UNF_FRAME_PACKING_TYPE_TOP_AND_BOTTOM,   /**< Top and bottom */
    MT_UNF_FRAME_PACKING_TYPE_TIME_INTERLACED,  /**< Time interlaced: one frame for left eye, the next frame for right eye */
    MT_UNF_FRAME_PACKING_TYPE_FRAME_PACKING,    /**< frame packing */
    MT_UNF_FRAME_PACKING_TYPE_3D_TILE,          /**< Tile 3D */
    MT_UNF_FRAME_PACKING_TYPE_BUTT
}MT_UNF_VIDEO_FRAME_PACKING_TYPE_E;

typedef struct mtUNF_VIDEO_FRAME_ADDR_S
{
    mt_u32             u32YAddr;    /**<Address of the Y component in the current frame*/ /**<CNcomment: 当前帧Y分量数据的地址*/
    mt_u32             u32CAddr;    /**<Address of the C component in the current frame*/ /**<CNcomment: 当前帧C分量数据的地址*/
    mt_u32             u32CrAddr;   /**<Address of the Cr component in the current frame*/ /**<CNcomment: 当前帧Cr分量数据的地址*/

    mt_u32             u32YStride;  /**<Stride of the Y component*/ /**<CNcomment: Y分量数据的跨幅*/
    mt_u32             u32CStride;  /**<Stride of the C component*/ /**<CNcomment: C分量数据的跨幅*/
    mt_u32             u32CrStride; /**<Stride of the Cr component*/ /**<CNcomment: Cr分量数据的跨幅*/
}MT_UNF_VIDEO_FRAME_ADDR_S;

typedef struct mtUNF_LINEAR_FRAME_ADDR_S
{
    mt_u32             u32YAddr;    /**<Address of the Y component in the current frame*/ /**<CNcomment: 当前帧Y分量数据的地址*/
    mt_u32             u32CAddr;    /**<Address of the C component in the current frame*/ /**<CNcomment: 当前帧C分量数据的地址*/
    mt_u32             u32CrAddr;   /**<Address of the Cr component in the current frame*/ /**<CNcomment: 当前帧Cr分量数据的地址*/

    mt_u32             u32YStride;  /**<Stride of the Y component*/ /**<CNcomment: Y分量数据的跨幅*/
    mt_u32             u32CStride;  /**<Stride of the C component*/ /**<CNcomment: C分量数据的跨幅*/
    mt_u32             u32CrStride; /**<Stride of the Cr component*/ /**<CNcomment: Cr分量数据的跨幅*/
    mt_u32             u32BufSize; /**<Stride of the Cr component*/ /**<CNcomment: Cr分量数据的跨幅*/

}MT_UNF_LINEAR_FRAME_ADDR_S;

typedef enum
{
  /*!
    display frame once
  */
  MT_UNF_DIS_FRAME_ONCE,

  /*!
    display frame twice
  */
  MT_UNF_DIS_FRAME_TWICE,

  /*!
    display frame triple
  */
  MT_UNF_DIS_FRAME_TRIPLE,

  /*!
    display frame triple
  */
  MT_UNF_DIS_FRAME_N_TIME,

  /*!
    display top field, then bottom field
  */
  MT_UNF_DIS_TOP_BOT,

  /*!
    display bottom field, then top field
  */
  MT_UNF_DIS_BOT_TOP,

  /*!
    display top field, then bottom field, then top filed
  */
  MT_UNF_DIS_TOP_BOT_TOP,

  /*!
    display bottom field, then top field, then bottom field
  */
  MT_UNF_DIS_BOT_TOP_BOT
}MT_UNF_DIS_ORDER_MODE_E;

typedef struct
{
  mt_u32 slot_idx;
  mt_u32 pts;
  mt_u32 addrLuma;  // byte allian
  mt_u32 addrChroma;
}MT_UNF_DIS_FIELD_INFO_T;

typedef enum
{
    MT_UNF_DISP_FRAME_PACKING_TYPE_NONE,             /* normal frame, not a 3D frame */
    MT_UNF_DISP_FRAME_PACKING_TYPE_SIDE_BY_SIDE,     /* side by side */
    MT_UNF_DISP_FRAME_PACKING_TYPE_TOP_BOTTOM,       /* top bottom */
    MT_UNF_DISP_FRAME_PACKING_TYPE_TIME_INTERLACED,  /* time interlaced: one frame for left eye, the next frame for right eye */
    MT_UNF_DISP_FRAME_PACKING_TYPE_BUTT
}MT_UNF_DISP_FRAME_PACKING_TYPE_E;

typedef struct mtUNF_VIDEO_FRAME_INFO_S
{
    mt_u32                              u32FrameIndex;      /**<Frame index ID of a video sequence*/ /**<CNcomment: 视频序列中的帧索引号*/
    MT_UNF_LINEAR_FRAME_ADDR_S          stLinearFrameAddr[2];
    MT_UNF_VIDEO_FRAME_ADDR_S           stVideoFrameAddr[2];
    mt_u32                              u32Width;           /**<Width of the source picture*/ /**<CNcomment: 原始图像宽*/
    mt_u32                              u32Height;          /**<Height of the source picture*/ /**<CNcomment: 原始图像高*/
    mt_u32                              u32SrcPts;          /**<Original PTS of a video frame*/ /**<CNcomment: 视频帧的原始时间戳*/
    mt_u32                              u32Pts;             /**<PTS of a video frame*/ /**<CNcomment: 视频帧的时间戳*/
    mt_u64                              u64Pts;
    mt_u32                              u32AspectWidth;
    mt_u32                              u32AspectHeight;
    MT_UNF_VCODEC_FRMRATE_S             stFrameRate;
    /* not used */
    mt_u8                               end_of_stream_flag; /**<End of stream flag*/

    MT_UNF_VIDEO_FORMAT_E               enVideoFormat;      /**<Video YUV format*/ /**<CNcomment: 视频YUV格式*/
    MT_BOOL                             bProgressive;       /**<Sampling type (progressive or interlaced)*/ /**<CNcomment: 采样方式(逐行/隔行) */
    MT_UNF_VIDEO_FIELD_MODE_E           enFieldMode;        /**<Frame or field encoding mode*/ /**<CNcomment: 帧或场编码模式*/
    MT_BOOL                             bTopFieldFirst;     /**<Top field first flag*/ /**<CNcomment: 顶场优先标志*/
    MT_UNF_VIDEO_FRAME_PACKING_TYPE_E   enFramePackingType; /**<3D frame packing type*/
    mt_u32                              u32Circumrotate;    /**<Need circumrotate, 1 need */
    MT_BOOL                             bVerticalMirror;
    MT_BOOL                             bHorizontalMirror;
    mt_u32                              u32DisplayWidth;    /**<Width of the displayed picture*/ /**<CNcomment: 显示图像宽*/
    mt_u32                              u32DisplayHeight;   /**<Height of the displayed picture*/ /**<CNcomment: 显示图像高*/
    mt_u32                              u32DisplayCenterX;  /**<Horizontal coordinate of the center of the displayed picture (the upper left point of the source picture serves as the coordinate origin)*/ /**<CNcomment: 显示中心x坐标，原始图像左上角为坐标原点*/
    mt_u32                              u32DisplayCenterY;  /**<Vertical coordinate of the center of the displayed picture (the upper left point of the source picture serves as the coordinate origin)*/ /**<CNcomment: 显示中心y坐标，原始图像左上角为坐标原点*/
    mt_u32                              u32ErrorLevel;      /**<Error percentage of a decoded picture, ranging from 0% to 100%*/ /**<CNcomment: 一幅解码图像中的错误比例，取值为0～100*/
    mt_u32                              u32Private[64];
    /* not used */
    MT_UNF_DIS_FIELD_INFO_T             filedInfoTop;
    /* not used */
    MT_UNF_DIS_FIELD_INFO_T             filedInfoBot;
    /* not used */
    MT_UNF_DIS_FIELD_INFO_T             filedInfoTopRight;
    /* not used */
    MT_UNF_DIS_FIELD_INFO_T             filedInfoBotRight;
    /* not used */
    MT_UNF_DIS_ORDER_MODE_E             display_order_mode;
    /* not used */
    MT_UNF_DISP_FRAME_PACKING_TYPE_E    packing_type;
    /* not used */
    mt_u32                              display_order_mode_valid;
    /* not used */
    mt_u8                               filed_storage_mode;  /**<Field storage mod, 0: two field merged storage, 1: two field separate storage*/
    /* not used */
    mt_u8                               is_3D_flag;          /**<3D flag, 0: no 3D, 1: 3D*/

    mt_u8                               picture_coding_type; /**<picture coding type, 0:I, 1:P, 2:B*/ /**<CNcomment: 帧编码类型, 0:I, 1:P, 2:B*/
    mt_u8                               active_format;       /**<H264 Active format description*/ /**<CNcomment: 活动图像格式描述符*/
	mt_u8 							    vid_format;
}MT_UNF_VIDEO_FRAME_INFO_S;

/**Defines the decode frame type.*/
/**CNcomment: 定义解码帧类型*/
typedef enum mtUNF_DEC_FRM_TYPE_E
{
	MT_UNF_DEC_FRM_ALL, /*decode all type of frames*/
	MT_UNF_DEC_FRM_IP,  /*only decode I and P frames*/
	MT_UNF_DEC_FRM_I,   /*only decode I frames*/
	MT_UNF_DEC_FRM_BUTT
} MT_UNF_DEC_FRM_TYPE_E;

typedef enum mtUNF_DEC_TRICK_MODE_E
{
  /*!
    Fast forward mode
    */
  MT_UNF_DEC_TM_FFWD,
  /*!
    Fast reverse mode
    */
  MT_UNF_DEC_TM_FREV,
  /*!
    Slow forward mode
    */
  MT_UNF_DEC_TM_SFWD,
  /*!
    Slow reverse mode
    */
  MT_UNF_DEC_TM_SREV,
  /*!
    Normal play mode
    */
  MT_UNF_DEC_TM_NORMAL,
  MT_UNF_DEC_TM_BUTT
}MT_UNF_DEC_TRICK_MODE_E;

typedef struct mtUNF_DEC_TRICK_PARAM_S
{
	MT_BOOL is_incomplete_stream;
	MT_UNF_DEC_TRICK_MODE_E trick_mode;
}MT_UNF_DEC_TRICK_PARAM_S;

/**
 * @brief HDR parameters
 */
typedef struct mtUNF_VIDEO_DISP_HDR_INFO_S
{
    mt_u32 colour_range;                /* Visual content value range. */
	mt_u32 colour_primaries;            /* Match the ones defined by ISO/IEC 23091-2_2019 subclause 8.1 and ITU-T H.273. */
    mt_u32 transfer_characteristics;    /* Match the ones defined by ISO/IEC 23091-2_2019 subclause 8.2. */
    mt_u32 colour_space;                /* Match the ones defined by ISO/IEC 23091-2_2019 subclause 8.3. */
    mt_u32 chroma_location;
	mt_u32 max_light_level;             /* Max content light level, in units of 1 cd/m^2 */
	mt_u32 max_pic_ave_light_level;     /* Max average light level per frame, in units of 1 cd/m^2 */
	mt_u32 has_primaries;               /* Whether the display primaries (and white point) are set */
    mt_u32 has_luminance;               /* Whether the luminance (min_ and max_)  are set */
    mt_u32 primary_r_chromaticity_x;    /* Display primaries x of r,range[0,50000], in units of 0.00002. */
    mt_u32 primary_r_chromaticity_y;    /* Display primaries y of r,range[0,50000], in units of 0.00002. */
    mt_u32 primary_g_chromaticity_x;    /* Display primaries x of g,range[0,50000], in units of 0.00002. */
    mt_u32 primary_g_chromaticity_y;    /* Display primaries y of g,range[0,50000], in units of 0.00002. */
    mt_u32 primary_b_chromaticity_x;    /* Display primaries x of b,range[0,50000], in units of 0.00002. */
    mt_u32 primary_b_chromaticity_y;    /* Display primaries y of b,range[0,50000], in units of 0.00002. */
	mt_u32 white_point_chromaticity_x;  /* White point x,range[0,50000], in units of 0.00002. */
	mt_u32 white_point_chromaticity_y;  /* White point x,range[0,50000], in units of 0.00002. */
	mt_u32 max_luminance;               /* Max luminance of mastering display, in units of 1 cd/m^2 */
	mt_u32 min_luminance;               /* Min luminance of mastering display, in units of 0.0001 cd/m^2 */
} MT_UNF_VIDEO_DISP_HDR_INFO_S;

/**
 *  @Deprecated, will be remove later, 
 */
typedef struct mtUNF_VIDEO_HDR_INFO_S
{
	mt_u32 hdr_info_valid_flag;
	mt_u32 transfer_characteristics;
	mt_u32 colour_primaries;
	mt_u32 max_light_level;
	mt_u32 max_pic_ave_light_level;
	mt_u32 mastering_metadata_valid_flag;
	mt_u32 primary_r_chromaticity_x_fra;
	mt_u32 primary_r_chromaticity_y_fra;
	mt_u32 primary_g_chromaticity_x_fra;
	mt_u32 primary_g_chromaticity_y_fra;
	mt_u32 primary_b_chromaticity_x_fra;
	mt_u32 primary_b_chromaticity_y_fra;
	mt_u32 white_point_chromaticity_x_fra;
	mt_u32 white_point_chromaticity_y_fra;
	mt_u32 luminance_max_int;
	mt_u32 luminance_max_fra;
	mt_u32 luminance_min_int;
	mt_u32 luminance_min_fra;
	mt_u32 accuracy_value;

} MT_UNF_VIDEO_HDR_INFO_S;

/** @} */  /** <!-- ==== Structure Definition End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __MT_UNF_VIDEO_ H*/
