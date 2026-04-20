/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************
  File Name     : mt_video_codec.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/11/25
  Description   : Common definitions of MT_CODEC(video).
                  The codec wants to register to MT_CODEC need to adapt to MT_CODEC_S.
  History       :
  1.Date        : 2015/11/25
    Author      : 
    Modification: Created file

*******************************************************************************/

#ifndef __MT_VIDEO_CODEC_H__
#define __MT_VIDEO_CODEC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "mt_type.h"

/*************************** Structure Definition ****************************/
/** \addtogroup  VCODEC  */
/** @{ */  /** <!--[VCODEC]  */

/*Codec error defination */
/**Operate fail due to insufficient resources(eg. no enough memory) */
/**CNcomment: 资源不足，(编)解码失败*/ 
#define MT_ERR_CODEC_NOENOUGHRES    (mt_s32)(0x80510001)
/**Parameter invalid */
/**CNcomment: 参数无效*/ 
#define MT_ERR_CODEC_INVALIDPARAM   (mt_s32)(0x80510002)
/**The (en)decoding fails due to incorrect input(streams or frames) */
/**CNcomment: 输入数据有误，(编)解码结束*/ 
#define MT_ERR_CODEC_INPUTCORRUPT   (mt_s32)(0x80510003)
/**The (en)decoding ends due to insufficient input data(streams or frames) */
/**CNcomment: 输入数据不足，(编)解码结束*/ 
#define MT_ERR_CODEC_NOENOUGHDATA   (mt_s32)(0x80510004)
/**The (en)decoding mode is not supported */
/**CNcomment: 不支持的(编)解码类型*/ 
#define MT_ERR_CODEC_INVALIDMODE    (mt_s32)(0x80510005)
/**Operate fail */
/**CNcomment: 操作失败*/ 
#define MT_ERR_CODEC_OPERATEFAIL    (mt_s32)(0x80510006)
/**Version unmatch*/
/**CNcomment: 版本不匹配*/ 
#define MT_ERR_CODEC_VERSIONUNMATCH (mt_s32)(0x80510007)
/**Unsupport operation */
/**CNcomment: 不支持的操作*/
#define MT_ERR_CODEC_UNSUPPORT      (mt_s32)(0x80510008)
/**Decorder busy */
/**CNcomment: 解码器忙*/ 
#define MT_ERR_CODEC_BUSY           (mt_s32)(0x80510009)
/**Unknown error */
/**CNcomment: 未知错误*/ 
#define MT_ERR_CODEC_UNKNOWN        (mt_s32)(0x80510010)

/**Codec ID */
/**CNcomment: 协议类型 */
typedef enum mtCODEC_ID_E
{
    MT_CODEC_ID_NONE = 0, 
        
    MT_CODEC_ID_VIDEO_MPEG1,
    MT_CODEC_ID_VIDEO_MPEG2,
    MT_CODEC_ID_VIDEO_MPEG2_XVMC,
    MT_CODEC_ID_VIDEO_MPEG4,      /**<MPEG4 DIVX4 DIVX5*/
    MT_CODEC_ID_VIDEO_MSMPEG4V1,
    MT_CODEC_ID_VIDEO_MSMPEG4V2,
    MT_CODEC_ID_VIDEO_MSMPEG4V3,

    MT_CODEC_ID_VIDEO_DIVX3,
    MT_CODEC_ID_VIDEO_XVID,

    MT_CODEC_ID_VIDEO_H261,
    MT_CODEC_ID_VIDEO_H263,
    MT_CODEC_ID_VIDEO_H263P,
    MT_CODEC_ID_VIDEO_H263I,
    MT_CODEC_ID_VIDEO_H264,
    MT_CODEC_ID_VIDEO_FFH264,

    MT_CODEC_ID_VIDEO_AVS,

    MT_CODEC_ID_VIDEO_REAL8,      /**<REAL*/
    MT_CODEC_ID_VIDEO_REAL9,      /**<REAL*/

    MT_CODEC_ID_VIDEO_VC1,
    MT_CODEC_ID_VIDEO_WMV1,
    MT_CODEC_ID_VIDEO_WMV2,
    MT_CODEC_ID_VIDEO_WMV3,

    MT_CODEC_ID_VIDEO_MSVIDEO1,

    MT_CODEC_ID_VIDEO_VP3,
    MT_CODEC_ID_VIDEO_VP5,
    MT_CODEC_ID_VIDEO_VP6,
    MT_CODEC_ID_VIDEO_VP6F,
    MT_CODEC_ID_VIDEO_VP6A,
    MT_CODEC_ID_VIDEO_VP8,

    MT_CODEC_ID_VIDEO_RAW,
    MT_CODEC_ID_VIDEO_SORENSON,   /**<SORENSON SPARK*/

	MT_CODEC_ID_VIDEO_HEVC,
    MT_CODEC_ID_VIDEO_RV10,
    MT_CODEC_ID_VIDEO_RV20,
    MT_CODEC_ID_VIDEO_RV30,
    MT_CODEC_ID_VIDEO_RV40,
    
    MT_CODEC_ID_VIDEO_SVQ1,
    MT_CODEC_ID_VIDEO_SVQ3,

    MT_CODEC_ID_VIDEO_CINEPAK,
    MT_CODEC_ID_VIDEO_INDEO2,
    MT_CODEC_ID_VIDEO_INDEO3,
    MT_CODEC_ID_VIDEO_INDEO4,
    MT_CODEC_ID_VIDEO_INDEO5,

    MT_CODEC_ID_VIDEO_JPEG,
    MT_CODEC_ID_VIDEO_JPEGLS,
    MT_CODEC_ID_VIDEO_JPEG2000,
    MT_CODEC_ID_VIDEO_MJPEG,
    MT_CODEC_ID_VIDEO_MJPEGB,
    MT_CODEC_ID_VIDEO_LJPEG,

    MT_CODEC_ID_VIDEO_TIFF,
    MT_CODEC_ID_VIDEO_GIF,
    MT_CODEC_ID_VIDEO_PNG,

    MT_CODEC_ID_VIDEO_SP5X,
    
    MT_CODEC_ID_VIDEO_FLV1,
    MT_CODEC_ID_VIDEO_HUFFYUV,
    MT_CODEC_ID_VIDEO_CYUV,
    MT_CODEC_ID_VIDEO_THEORA,
    MT_CODEC_ID_VIDEO_ASV1,
    MT_CODEC_ID_VIDEO_ASV2,
    MT_CODEC_ID_VIDEO_FFV1,
    MT_CODEC_ID_VIDEO_4XM,
    MT_CODEC_ID_VIDEO_VCR1,
    MT_CODEC_ID_VIDEO_CLJR,
    MT_CODEC_ID_VIDEO_MDEC,
    MT_CODEC_ID_VIDEO_ROQ,
    MT_CODEC_ID_VIDEO_INTERPLAY_VIDEO,
    MT_CODEC_ID_VIDEO_XAN_WC3,
    MT_CODEC_ID_VIDEO_XAN_WC4,
    MT_CODEC_ID_VIDEO_RPZA,
    MT_CODEC_ID_VIDEO_WS_VQA,
    MT_CODEC_ID_VIDEO_MSRLE,
    MT_CODEC_ID_VIDEO_IDCIN,
    MT_CODEC_ID_VIDEO_8BPS,
    MT_CODEC_ID_VIDEO_SMC,
    MT_CODEC_ID_VIDEO_FLIC,
    MT_CODEC_ID_VIDEO_TRUEMOTION1,
    MT_CODEC_ID_VIDEO_VMDVIDEO,
    MT_CODEC_ID_VIDEO_MSZH,
    MT_CODEC_ID_VIDEO_ZLIB,
    MT_CODEC_ID_VIDEO_QTRLE,
    MT_CODEC_ID_VIDEO_SNOW,
    MT_CODEC_ID_VIDEO_TSCC,
    MT_CODEC_ID_VIDEO_ULTI,
    MT_CODEC_ID_VIDEO_QDRAW,
    MT_CODEC_ID_VIDEO_VIXL,
    MT_CODEC_ID_VIDEO_QPEG,
    MT_CODEC_ID_VIDEO_PPM,
    MT_CODEC_ID_VIDEO_PBM,
    MT_CODEC_ID_VIDEO_PGM,
    MT_CODEC_ID_VIDEO_PGMYUV,
    MT_CODEC_ID_VIDEO_PAM,
    MT_CODEC_ID_VIDEO_FFVHUFF,
    
    MT_CODEC_ID_VIDEO_LOCO,
    MT_CODEC_ID_VIDEO_WNV1,
    MT_CODEC_ID_VIDEO_AASC,
    MT_CODEC_ID_VIDEO_FRAPS,
    MT_CODEC_ID_VIDEO_TRUEMOTION2,
    MT_CODEC_ID_VIDEO_BMP,
    MT_CODEC_ID_VIDEO_CSCD,
    MT_CODEC_ID_VIDEO_MMVIDEO,
    MT_CODEC_ID_VIDEO_ZMBV,
    MT_CODEC_ID_VIDEO_SMACKVIDEO,
    MT_CODEC_ID_VIDEO_NUV,
    MT_CODEC_ID_VIDEO_KMVC,
    MT_CODEC_ID_VIDEO_FLASHSV,
    MT_CODEC_ID_VIDEO_CAVS,
    
    MT_CODEC_ID_VIDEO_VMNC,
    
    MT_CODEC_ID_VIDEO_TARGA,
    MT_CODEC_ID_VIDEO_DSICINVIDEO,
    MT_CODEC_ID_VIDEO_TIERTEXSEQVIDEO,
    
    MT_CODEC_ID_VIDEO_DXA,
    MT_CODEC_ID_VIDEO_DNXHD,
    MT_CODEC_ID_VIDEO_THP,
    MT_CODEC_ID_VIDEO_SGI,
    MT_CODEC_ID_VIDEO_C93,
    MT_CODEC_ID_VIDEO_BETHSOFTVID,
    MT_CODEC_ID_VIDEO_PTX,
    MT_CODEC_ID_VIDEO_TXD,
    MT_CODEC_ID_VIDEO_AMV,
    MT_CODEC_ID_VIDEO_VB,
    MT_CODEC_ID_VIDEO_PCX,
    MT_CODEC_ID_VIDEO_SUNRAST,
    MT_CODEC_ID_VIDEO_MIMIC,
    MT_CODEC_ID_VIDEO_RL2,
    MT_CODEC_ID_VIDEO_8SVX_EXP,
    MT_CODEC_ID_VIDEO_8SVX_FIB,
    MT_CODEC_ID_VIDEO_ESCAPE124,
    MT_CODEC_ID_VIDEO_DIRAC,
    MT_CODEC_ID_VIDEO_BFI,
    MT_CODEC_ID_VIDEO_CMV,
    MT_CODEC_ID_VIDEO_MOTIONPIXELS,
    MT_CODEC_ID_VIDEO_TGV,
    MT_CODEC_ID_VIDEO_TGQ,
    MT_CODEC_ID_VIDEO_TQI,
    MT_CODEC_ID_VIDEO_AURA,
    MT_CODEC_ID_VIDEO_AURA2,
    MT_CODEC_ID_VIDEO_V210X,
    MT_CODEC_ID_VIDEO_TMV,
    MT_CODEC_ID_VIDEO_V210,
    MT_CODEC_ID_VIDEO_DPX,
    MT_CODEC_ID_VIDEO_MAD,
    MT_CODEC_ID_VIDEO_FRWU,
    MT_CODEC_ID_VIDEO_FLASHSV2,
    MT_CODEC_ID_VIDEO_CDGRAPHICS,
    MT_CODEC_ID_VIDEO_R210,
    MT_CODEC_ID_VIDEO_ANM,
    MT_CODEC_ID_VIDEO_BINKVIDEO,
    MT_CODEC_ID_VIDEO_IFF_ILBM,
    MT_CODEC_ID_VIDEO_IFF_BYTERUN1,
    MT_CODEC_ID_VIDEO_KGV1,
    MT_CODEC_ID_VIDEO_YOP,
    MT_CODEC_ID_VIDEO_DV,
	MT_CODEC_ID_VIDEO_VP9,
	MT_CODEC_ID_VIDEO_AVS2,
    MT_CODEC_ID_BUTT
}MT_CODEC_ID_E;

/*BIT0 of MT_CODEC_SUPPORT_S.u32Type: Driven type */
/**Need be driven by codec manager. Like FFMPEG. */ /**CNcomment:编解码器需要管理器驱动 */
#define MT_CODEC_CAP_DRIVENOUTSIDE      (0x00000000)
/**Self-driven, only need call Start()/Stop(). Like VFMW. */ /**CNcomment:编解码器有自驱动能力 */
#define MT_CODEC_CAP_DRIVENSELF         (0x00000001)

/*BIT1 of MT_CODEC_SUPPORT_S.u32Type: Output type */
/**Codec outputs frame or stream to specified address directly.  */ /**CNcomment:编解码器支持将帧数据或流数据输出到指定地址 */
#define MT_CODEC_CAP_OUTPUT2SELFADDR    (0x00000000)
/**Codec outputs frame or stream to the address self allocated . */ /**CNcomment:编解码器将帧数据或流数据输出到自身地址，需要外部拷贝 */
#define MT_CODEC_CAP_OUTPUT2SPECADDR    (0x00000002)

/**Defines codec type: encoder or decoder  */
/**CNcomment:Codec类型:编码或解码*/ 
typedef enum mtCODEC_TYPE_E
{
    MT_CODEC_TYPE_DEC = 0x01,       /**<decoder, value is b00000001*/ /**< CNcomment:解码器 */
    MT_CODEC_TYPE_ENC = 0x02,       /**<encoder, value is b00000010*/ /**< CNcomment:编码器 */
    
    MT_CODEC_TYPE_BUTT
}MT_CODEC_TYPE_E;

/**Defines codec types and formats */
/**CNcomment:Codec类型与支持的协议*/ 
typedef struct mtCODEC_SUPPORT_S
{
    mt_u32          u32Type;            /**< Codec type, OR value of MT_CODEC_TYPE_E */ /**< CNcomment:Codec类型，MT_CODEC_TYPE_E的或值 */
    MT_CODEC_ID_E   enID;               /**< Codec ID*/ /**< CNcomment:Codec支持的协议类型 */
    struct mtCODEC_SUPPORT_S* pstNext;  /**< Pointer to next node*/ /**< CNcomment:指向下一个能力结点的指针 */
}MT_CODEC_SUPPORT_S;

/**Defines codec capability */
/**CNcomment:Codec能力*/ 
typedef struct mtCODEC_CAP_S
{
    mt_u32              u32CapNumber;   /**< Codec capability index, OR value of several MT_CODEC_CAP_XXX */ /**< CNcomment:Codec能力指示值，MT_CODEC_CAP_XXX的或值 */
    MT_CODEC_SUPPORT_S* pstSupport;     /**< Pointer to the support type and formats*/ /**< CNcomment:Codec能力结构体指针 */
}MT_CODEC_CAP_S;

/**Defines the codec version*/
/**CNcomment:版本定义*/ 
typedef union mtCODEC_VERSION_U
{
    struct
    {
        mt_u8 u8VersionMajor;           /**< Major version */ /**< CNcomment:主版本号 */
        mt_u8 u8VersionMinor;           /**< Minor version */ /**< CNcomment:次版本号 */
        mt_u8 u8Revision;               /**< Revision version */ /**< CNcomment:修订版本号 */
        mt_u8 u8Step;                   /**< Step version */ /**< CNcomment:步进版本号 */
    } stVersion;
    mt_u32 u32Version;
} MT_CODEC_VERSION_U;

/**Defines video decoder open parameter*/
/**CNcomment:视频解码器打开参数*/ 
typedef struct mtCODEC_VDEC_OPENPARAM_S
{
    mt_u32 u32Reserve;
    mt_void* pPlatformPriv;         /**< Special parameter for platform hardware codec */ /**< CNcomment:硬解码器私有参数 */
}MT_CODEC_VDEC_OPENPARAM_S;

/**Defines video encoder open parameter*/
/**CNcomment:视频编码器打开参数*/ 
typedef struct mtCODEC_VENC_OPENPARAM_S
{
    mt_u32 u32Reserve;
}MT_CODEC_VENC_OPENPARAM_S;

/**Defines the codec open parameters*/
/**CNcomment:实例创建参数*/
typedef struct mtCODEC_OPENPARAM_S
{
    MT_CODEC_TYPE_E enType;       /** Encode or Decode */ /**< CNcomment:创建编码器还是解码器 */
    MT_CODEC_ID_E enID;           /** Format */ /**< CNcomment:需要编码或解码的协议类型 */
    union {
        MT_CODEC_VDEC_OPENPARAM_S stVdec; /** Open parameters of video decoder */ /**< CNcomment:视频解码器创建参数 */
        MT_CODEC_VENC_OPENPARAM_S stVenc; /** Open parameters of video encoder */ /**< CNcomment:视频编码器创建参数 */
    }unParam;
} MT_CODEC_OPENPARAM_S;

/**Defines the codec max priority */
/**CNcomment:编解码器最大优先级定义*/
#define MT_CODEC_MAX_PRIORITY (16)

/**Defines the codec reset parameters*/
/**CNcomment:视频解码器复位参数*/
typedef struct mtCODEC_RESETPARAM_S
{
    MT_BOOL resetDQ;
} MT_CODEC_RESETPARAM_S;

/**Defines attribute of video decoder */
/**CNcomment:视频解码器实例属性*/
typedef struct mtCODEC_VDEC_ATTR_S
{
    mt_void*                pCodecContext;  /**< Codec context */ /**< CNcomment:编解码器上下文，可以传递解码器特定参数 */
    mt_void*                pPlatformPriv;  /**< Only used by platform */ /**< CNcomment:硬解码器私有参数 */
} MT_CODEC_VDEC_ATTR_S;

/**Defines attribute of codec instance */
/**CNcomment:实例属性*/
typedef struct mtCODEC_ATTR_S
{
    MT_CODEC_ID_E           enID;   /**< Codec ID*/ /**< CNcomment:编码或解码的协议类型*/
    union {
        MT_CODEC_VDEC_ATTR_S stVdec;/**< Instance attribute*/ /**< CNcomment:视频解码器属性参数*/
    } unAttr;
} MT_CODEC_ATTR_S;

/**Defines video sub standard */
/**CNcomment: 视频协议分支标准 */
typedef enum mtCODEC_VIDEO_SUB_STANDARD_E
{
    MT_CODEC_VIDEO_SUB_STANDARD_UNKNOWN,       /**<Unknown*/ /**<CNcomment: 未知协议分支*/
    MT_CODEC_VIDEO_SUB_STANDARD_MPEG2_MPEG1,   /**<The MPEG2 protocol family is compatible with MPEG1.*/ /**<CNcomment: MPEG2协议族可以兼容MPEG1 */
    MT_CODEC_VIDEO_SUB_STANDARD_MPEG4_SHV,     /**<The MPEG4 protocol family is compatible with SHV.*/ /**<CNcomment: MPEG4协议族可以兼容SHV*/
    MT_CODEC_VIDEO_SUB_STANDARD_MPEG4_XVID,    /**<The MPEG4 protocol family includes the sub protocol XVID.*/ /**<CNcomment: MPEG4协议包含XVID分支*/
    MT_CODEC_VIDEO_SUB_STANDARD_MPEG4_DIVX,    /**<The MPEG4 protocol family includes the sub protocol DIVX.*/ /**<CNcomment: MPEG4协议包含DIVX分支*/

    MT_CODEC_VIDEO_SUB_STANDARD_BUTT
}MT_CODEC_VIDEO_SUB_STANDARD_E;

/**Defines the video norm*/
/**CNcomment: 定义视频制式枚举*/
typedef enum mtCODEC_ENC_FMT_E
{
    MT_CODEC_ENC_FMT_1080P_60 = 0,     /**<1080p 60 Hz*/ /**< CNcomment:1080p 60赫兹*/
    MT_CODEC_ENC_FMT_1080P_50,         /**<1080p 50 Hz*/ /**< CNcomment:1080p 50赫兹*/
    MT_CODEC_ENC_FMT_1080P_30,         /**<1080p 30 Hz*/ /**< CNcomment:1080p 30赫兹*/
    MT_CODEC_ENC_FMT_1080P_25,         /**<1080p 25 Hz*/ /**< CNcomment:1080p 25赫兹*/
    MT_CODEC_ENC_FMT_1080P_24,         /**<1080p 24 Hz*/ /**< CNcomment:1080p 24赫兹*/

    MT_CODEC_ENC_FMT_1080i_60,         /**<1080i 60 Hz*/ /**< CNcomment:1080i 60赫兹*/
    MT_CODEC_ENC_FMT_1080i_50,         /**<1080i 50 Hz*/ /**< CNcomment:1080i 50赫兹*/

    MT_CODEC_ENC_FMT_720P_60,          /**<720p 60 Hz*/  /**< CNcomment:720p 60赫兹*/
    MT_CODEC_ENC_FMT_720P_50,          /**<720p 50 Hz */ /**< CNcomment:720p 50赫兹*/

    MT_CODEC_ENC_FMT_576P_50,          /**<576p 50 Hz*/  /**< CNcomment:576p 50赫兹*/
    MT_CODEC_ENC_FMT_480P_60,          /**<480p 60 Hz*/  /**< CNcomment:480p 60赫兹*/

    MT_CODEC_ENC_FMT_PAL,              /**<B D G H I PAL */ /**< CNcomment:B D G H I PAL制式*/
    MT_CODEC_ENC_FMT_PAL_N,            /**<(N)PAL        */ /**< CNcomment:(N)PAL制式*/
    MT_CODEC_ENC_FMT_PAL_Nc,           /**<(Nc)PAL       */ /**< CNcomment:(Nc)PAL制式*/

    MT_CODEC_ENC_FMT_NTSC,             /**<(M)NTSC       */ /**< CNcomment:(M)NTSC制式*/
    MT_CODEC_ENC_FMT_NTSC_J,           /**<NTSC-J        */ /**< CNcomment:NTSC-J制式*/
    MT_CODEC_ENC_FMT_NTSC_PAL_M,       /**<(M)PAL        */ /**< CNcomment:(M)PAL制式*/

    MT_CODEC_ENC_FMT_SECAM_SIN,        /**< SECAM_SIN*/ /**< CNcomment:SECAM_SIN制式*/
    MT_CODEC_ENC_FMT_SECAM_COS,        /**< SECAM_COS*/ /**< CNcomment:SECAM_COS制式*/

    MT_CODEC_ENC_FMT_1080P_24_FRAME_PACKING,
    MT_CODEC_ENC_FMT_720P_60_FRAME_PACKING,
    MT_CODEC_ENC_FMT_720P_50_FRAME_PACKING,
    
    MT_CODEC_ENC_FMT_861D_640X480_60,
    MT_CODEC_ENC_FMT_VESA_800X600_60,
    MT_CODEC_ENC_FMT_VESA_1024X768_60,
    MT_CODEC_ENC_FMT_VESA_1280X720_60,
    MT_CODEC_ENC_FMT_VESA_1280X800_60,
    MT_CODEC_ENC_FMT_VESA_1280X1024_60,
    MT_CODEC_ENC_FMT_VESA_1360X768_60,
    MT_CODEC_ENC_FMT_VESA_1366X768_60,
    MT_CODEC_ENC_FMT_VESA_1400X1050_60,
    MT_CODEC_ENC_FMT_VESA_1440X900_60,
    MT_CODEC_ENC_FMT_VESA_1440X900_60_RB,
    MT_CODEC_ENC_FMT_VESA_1600X900_60_RB,
    MT_CODEC_ENC_FMT_VESA_1600X1200_60,
    MT_CODEC_ENC_FMT_VESA_1680X1050_60,
    MT_CODEC_ENC_FMT_VESA_1920X1080_60,
    MT_CODEC_ENC_FMT_VESA_1920X1200_60,
    MT_CODEC_ENC_FMT_VESA_2048X1152_60,

    MT_CODEC_ENC_FMT_BUTT
}MT_CODEC_ENC_FMT_E;

/**Defines the video YUV format*/
/**CNcomment: YUV格式枚举*/
typedef enum mtCODEC_COLOR_FORMAT_E
{
    MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_400 = 0,   /**<The YUV spatial sampling format is SEMIPLANAR 4:0:0.*/ /**<CNcomment: YUV空间采样格式为SEMIPLANAR 4:0:0*/
    MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_411,       /**<The YUV spatial sampling format is SEMIPLANAR 4:1:1.*/ /**<CNcomment: YUV空间采样格式为SEMIPLANAR 4:1:1*/
    MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_420,       /**<The YUV spatial sampling format is SEMIPLANAR 4:2:0.*/ /**<CNcomment: YUV空间采样格式为SEMIPLANAR 4:2:0*/
    MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_422_1X2,   /**<The YUV spatial sampling format is SEMIPLANAR 4:2:2,two Y correspond to one U and V at vertical direction.*/ /**<CNcomment: YUV空间采样格式为SEMIPLANAR 4:2:2，垂直方向两个亮度采样点共用一对色度采样点*/
    MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_422_2X1,   /**<The YUV spatial sampling format is SEMIPLANAR 4:2:2,two Y correspond to one U and V at horizontal direction.*/ /**<CNcomment: YUV空间采样格式为SEMIPLANAR 4:2:2，水平方向两个亮度采样点共用一对色度采样点*/
    MT_CODEC_COLOR_FORMAT_YUV_SEMIPLANAR_444,       /**<The YUV spatial sampling format is SEMIPLANAR 4:4:4.*/ /**<CNcomment: YUV空间采样格式为SEMIPLANAR 4:4:4*/

    MT_CODEC_COLOR_FORMAT_YUV_PLANAR_400 = 0x10,    /**<The YUV spatial sampling format is PLANAR 4:0:0.*/ /**<CNcomment: YUV空间采样格式为PLANAR 4:0:0*/   
    MT_CODEC_COLOR_FORMAT_YUV_PLANAR_411,           /**<The YUV spatial sampling format is PLANAR 4:1:1.*/ /**<CNcomment: YUV空间采样格式为PLANAR 4:1:1*/   
    MT_CODEC_COLOR_FORMAT_YUV_PLANAR_420,           /**<The YUV spatial sampling format is PLANAR 4:2:0.*/ /**<CNcomment: YUV空间采样格式为PLANAR 4:2:0*/   
    MT_CODEC_COLOR_FORMAT_YUV_PLANAR_422_1X2,       /**<The YUV spatial sampling format is PLANAR 4:2:2,two Y correspond to one U and V at vertical direction.*/ /**<CNcomment: YUV空间采样格式为PLANAR 4:2:2，垂直方向两个亮度采样点共用一对色度采样点*/
    MT_CODEC_COLOR_FORMAT_YUV_PLANAR_422_2X1,       /**<The YUV spatial sampling format is PLANAR 4:2:2,two Y correspond to one U and V at horizontal direction.*/ /**<CNcomment: YUV空间采样格式为PLANAR 4:2:2，水平方向两个亮度采样点共用一对色度采样点*/
    MT_CODEC_COLOR_FORMAT_YUV_PLANAR_444,           /**<The YUV spatial sampling format is PLANAR 4:4:4.*/ /**<CNcomment: YUV空间采样格式为PLANAR 4:4:4*/   
    MT_CODEC_COLOR_FORMAT_YUV_PLANAR_410,           /**<The YUV spatial sampling format is PLANAR 4:1:0.*/ /**<CNcomment: YUV空间采样格式为PLANAR 4:1:0*/   
    
    MT_CODEC_COLOR_FORMAT_YUV_PACKAGE_UYVY422 = 0x20,   /**<The YUV spatial sampling format is package, and the pixel arrangement sequence in the memory is UYVY.*/ /**<CNcomment: YUV空间采样格式为package,内存排列为UYVY*/
    MT_CODEC_COLOR_FORMAT_YUV_PACKAGE_YUYV422,          /**<The YUV spatial sampling format is package, and the pixel arrangement sequence in the memory is YUYV.*/ /**<CNcomment: YUV空间采样格式为package,内存排列为YUYV*/
    MT_CODEC_COLOR_FORMAT_YUV_PACKAGE_YVYU422,          /**<The YUV spatial sampling format is package, and the pixel arrangement sequence in the memory is YVYU.*/ /**<CNcomment: YUV空间采样格式为package,内存排列为YVYU*/
    MT_CODEC_COLOR_FORMAT_YUV_BUTT
}MT_CODEC_COLOR_FORMAT_E;

/**Defines the type of the video frame*/
/**CNcomment: 定义视频帧的类型枚举*/
typedef enum mtCODEC_VIDEO_FRAME_TYPE_E
{
    MT_CODEC_VIDEO_FRAME_TYPE_UNKNOWN,   /**<Unknown*/ /**<CNcomment: 未知的帧类型*/
    MT_CODEC_VIDEO_FRAME_TYPE_I,         /**<I frame*/ /**<CNcomment: I帧*/
    MT_CODEC_VIDEO_FRAME_TYPE_P,         /**<P frame*/ /**<CNcomment: P帧*/
    MT_CODEC_VIDEO_FRAME_TYPE_B,         /**<B frame*/ /**<CNcomment: B帧*/
    MT_CODEC_VIDEO_FRAME_TYPE_BUTT
}MT_CODEC_VIDEO_FRAME_TYPE_E;

/**Defines the video frame/field mode*/
/**CNcomment: 定义视频帧场模式枚举*/
typedef enum mtCODEC_VIDEO_FIELD_MODE_E
{
    MT_CODEC_VIDEO_FIELD_ALL,        /**<Frame mode*/ /**<CNcomment: 帧模式*/
    MT_CODEC_VIDEO_FIELD_TOP,        /**<Top field mode*/ /**<CNcomment: 顶场模式*/
    MT_CODEC_VIDEO_FIELD_BOTTOM,     /**<Bottom field mode*/ /**<CNcomment: 底场模式*/
    MT_CODEC_VIDEO_FIELD_BUTT
}MT_CODEC_VIDEO_FIELD_MODE_E;

/**Defines 3D frame packing type*/
/**CNcomment: 3D帧打包方式 */
typedef enum mtCODEC_VIDEO_FRAME_PACKING_TYPE_E
{
    MT_CODEC_VIDEO_FRAME_PACKING_NONE,              /**< normal frame, not a 3D frame */ /**<CNcomment: 非3D帧*/
    MT_CODEC_VIDEO_FRAME_PACKING_SIDE_BY_SIDE,      /**< side by side */ /**<CNcomment:左右模式3D帧 */
    MT_CODEC_VIDEO_FRAME_PACKING_TOP_AND_BOTTOM,    /**< top and bottom */ /**<CNcomment:上下模式3D帧 */
    MT_CODEC_VIDEO_FRAME_PACKING_TIME_INTERLACED,   /**< time interlaced: one frame for left eye, the next frame for right eye */ /**<CNcomment:时间差模式3D帧 */
    MT_CODEC_VIDEO_FRAME_PACKING_BUTT             
}MT_CODEC_VIDEO_FRAME_PACKING_TYPE_E;

/**Defines address information*/
/**CNcomment: 地址描述结构体*/
typedef struct mtCODEC_ADDRESS_S
{
    mt_u32  u32Phy;             /**<Physical address of (frame or stream) buffer */ /**<CNcomment: 帧或流数据的物理地址 */
    mt_u32  u32Vir;             /**<Virtual address of (frame or stream) buffer */ /**<CNcomment: 帧或流数据的虚拟地址 */
    mt_u32  u32Size;            /**<Size of (frame or stream) buffer*/ /**<CNcomment: 帧或流数据的大小 */
}MT_CODEC_ADDRESS_S;

/**Defines video stream info */
/** CNcomment: 视频流信息结构体*/
typedef struct mtCODEC_VIDEO_STREAMINFO_S
{
    MT_CODEC_ID_E                   enCodecID;      /**<Stream type*/ /**<CNcomment: 码流类型 */
    MT_CODEC_VIDEO_SUB_STANDARD_E   enSubStandard;  /**<Sub stream protocol*/ /**<CNcomment: 码流的协议分支 */
    mt_u32                          u32SubVersion;  /**<Version of the sub stream protocol*/ /**<CNcomment: 码流子协议版本号 */
    mt_u32                          u32Profile;     /**<Stream profile*/ /**<CNcomment: 码流的profile */
    mt_u32                          u32Level;       /**<Stream level*/ /**<CNcomment: 码流的level */
    MT_CODEC_ENC_FMT_E              enDisplayNorm;  /**<Display norm*/ /**<CNcomment: 显示标准 */
    MT_BOOL                         bProgressive;   /**<Sampling type (progressive or interlaced)*/ /**<CNcomment: 采样方式(逐行/隔行) */

    mt_u32                          u32AspectWidth; /**<Aspect width*/ /**<CNcomment: 最佳显示幅型比之宽度比值*/
    mt_u32                          u32AspectHeight;/**<Aspect height*/ /**<CNcomment: 最佳显示幅型比之高度比值*/

    mt_u32 u32bps;            /**<Bit rate, in kbit/s*/ /**<CNcomment: 码流的码率, Kbps */
    mt_u32 u32FrameRateInt;   /**<Integral part of the frame rate (in frame/s)*/ /**<CNcomment: 码流的帧率的整数部分, fps */
    mt_u32 u32FrameRateDec;   /**<Fractional part (calculated to three decimal places) of the frame rate (in frame/s)*/ /**<CNcomment: 码流的帧率的小数部分（保留3位）, fps */
    mt_u32 u32Width;          /**<Width of the decoded picture*/ /**<CNcomment: 解码图像宽 */
    mt_u32 u32Height;         /**<Height of the decoded picture*/ /**<CNcomment: 解码图像高 */
    mt_u32 u32DisplayWidth;   /**<Width of the displayed picture*/ /**<CNcomment: 显示图像宽 */
    mt_u32 u32DisplayHeight;  /**<Height of the displayed picture*/ /**<CNcomment: 显示图像高 */
    mt_u32 u32DisplayCenterX; /**<Horizontal coordinate of the center of the displayed picture (the upper left point of the source picture serves as the coordinate origin)*/
                              /**<CNcomment: 显示图像中心横坐标，以原始图像的左上角为坐标原点 */
    mt_u32 u32DisplayCenterY; /**<Vertical coordinate of the center of the displayed picture (the upper left point of the source picture serves as the coordinate origin)*/
                              /**<CNcomment: 显示图像中心纵坐标，以原始图像的左上角为坐标原点 */
	mt_u32 u32IsHDR;
}MT_CODEC_VIDEO_STREAMINFO_S;

/**Defines stream information */
/**CNcomment: 码流信息结构体*/
typedef union mtCODEC_STREAMINFO_S
{
    MT_CODEC_VIDEO_STREAMINFO_S stVideo;    /**<video stream information*/ /**<CNcomment: 视频流信息*/
}MT_CODEC_STREAMINFO_S;

typedef struct mtCODEC_FRAME_BUF_S
{
	mt_u32 u32PhyAddr;
	mt_u32 u32Size;
}MT_CODEC_FRAME_BUF_S;

/**Defines stream descriptor */
/**CNcomment: 码流数据结构体*/
typedef struct mtCODEC_STREAM_S
{
    mt_u8* pu8Addr;     /**<Stream buffer address*/ /**<CNcomment: 流数据地址*/
	mt_u32 u32PhyAddr;  /**<Stream buffer address*/ /**<CNcomment: 流数据物理地址*/
    mt_u32 u32Size;     /**<Stream buffer size*/ /**<CNcomment: 流数据大小*/
    mt_s64 s64PtsMs;    /**<PTS(ms)*/ /**<CNcomment: 流数据对应PTS，毫秒为单位*/
}MT_CODEC_STREAM_S;

/**Defines video codec comonds */
/**CNcomment: 视频编解码器控制命令结构体*/
typedef struct mtCODEC_VIDEO_CMD_S
{
    mt_u32      u32CmdID;   /**<Commond ID*/ /**<CNcomment: 命令ID*/
    mt_void     *pPara;     /**<Control parameter*/ /**<CNcomment: 命令携带参数*/
}MT_CODEC_VIDEO_CMD_S;

/**Defines video frame information*/
/**CNcomment: 视频帧信息结构体*/
typedef struct mtCODEC_VIDEO_FRAME_S
{
    MT_CODEC_COLOR_FORMAT_E             enColorFormat;        /**<Video format*/ /**<CNcomment: 视频格式*/
    MT_CODEC_VIDEO_FRAME_TYPE_E         enFrameType;          /**<Frame type*/ /**<CNcomment: 帧类型*/
    MT_BOOL                             bProgressive;         /**<Sampling type*/ /**<CNcomment: 采样类型*/
    
    MT_CODEC_VIDEO_FIELD_MODE_E         enFieldMode;          /**<Frame or field encoding mode*/ /**<CNcomment: 帧或场编码模式*/
    MT_BOOL                             bTopFieldFirst;       /**<Top field first flag*/ /**<CNcomment: 顶场优先标志*/

    MT_CODEC_VIDEO_FRAME_PACKING_TYPE_E enFramePackingType;   /**<3D frame packing type*/ /**<CNcomment: 3D帧打包方式*/
    
    mt_u32                              u32FrameRate;         /**<Frame rate*//**<CNcomment: 帧率*/
    
    mt_u32                              u32Width;             /**<Width of the source picture*/ /**<CNcomment: 原始图像宽*/
    mt_u32                              u32Height;            /**<Height of the source picture*/ /**<CNcomment: 原始图像高*/
    mt_u32                              u32AspectWidth;       /**<Aspect width*/ /**<CNcomment: 最佳显示宽度*/
    mt_u32                              u32AspectHeight;      /**<Aspect height*/ /**<CNcomment: 最佳显示高度*/

    mt_u32                              u32YAddr;             /**<Address of the Y component in the current frame*/ /**<CNcomment: 当前帧Y分量数据的地址*/
    mt_u32                              u32UAddr;             /**<Address of the U component in the current frame*/ /**<CNcomment: 当前帧U分量数据的地址*/
    mt_u32                              u32VAddr;             /**<Address of the V component in the current frame*/ /**<CNcomment: 当前帧V分量数据的地址*/
    mt_u32                              u32YStride;           /**<Stride of the Y component*/ /**<CNcomment: Y分量数据的跨幅*/
    mt_u32                              u32UStride;           /**<Stride of the U component*/ /**<CNcomment: C分量数据的跨幅*/
    mt_u32                              u32VStride;           /**<Stride of the V component*/ /**<CNcomment: C分量数据的跨幅*/

    mt_u8*                              pu8UserData;          /**<User data *//**<CNcomment: 用户数据*/
    mt_u32                              u32UserDataSize;      /**<User data size *//**<CNcomment: 用户数据长度*/
}MT_CODEC_VIDEO_FRAME_S;

/**Defines frame descriptor */
/**CNcomment: 帧描述结构体*/
typedef struct mtCODEC_FRAME_S
{
    MT_CODEC_ADDRESS_S          stOutputAddr;   /**< Always [in], only used by the codecs which support MT_CODEC_OUTPUT_TOSPECADDR, Only used by decoder  */
                                                /**< CNcomment: 帧输出地址，一直是输入参数，仅适用于支持MT_CODEC_OUTPUT_TOSPECADDR的解码器 */
    mt_s64                      s64SrcPtsMs;    /**< Decoder [out], encoder [in], The source pts(ms) */
                                                /**< CNcomment: 帧源PTS，毫秒为单位，对解码器来说是输出参数，对编码器来说是输入参数*/
    mt_s64                      s64PtsMs;       /**< Decoder [out], encoder [in], The pts(ms)*/
                                                /**< CNcomment: 帧PTS，毫秒为单位，对解码器来说是输出参数，对编码器来说是输入参数*/
    union{
        MT_CODEC_VIDEO_FRAME_S  stVideo;        /**< Decoder [out], encoder [in], The video frame information*/ 
                                                /**< CNcomment: 视频帧信息，对解码器来说是输出参数，对编码器来说是输入参数*/
    }unInfo;
}MT_CODEC_FRAME_S;

/**Defines codec*/
/**CNcomment: CODEC结构体定义*/
typedef struct mtCODEC_S
{
    /**<Description information about a codec */
    /**<CNcomment: 编解码器描述名字 */ 
    const mt_char *pszName;

    /**<Codec version */
    /**<CNcomment: 编解码器版本描述 */ 
    const MT_CODEC_VERSION_U unVersion;

    /**<Detailed information about a codec */
    /**<CNcomment: 编解码器详细描述信息 */
    const mt_char *pszDescription;

    /**
    \brief Get codec capability. CNcomment:获取编解码器能力 CNend
    \attention \n
    N/A
    \param [out] pstCodecCap, Pointer of the codec capability. CNcomment:指针类型，指向编解码器能力 CNend
    \retval ::MT_SUCCESS CNcomment:操作成功 CNend
    \retval ::MT_FAILURE CNcomment:操作失败 CNend
    \see \n
    N/A
    */
    mt_s32 (*GetCap)(MT_CODEC_CAP_S *pstCodecCap);
  
    /**
    \brief Create a codec instance. CNcomment:创建编解码器实例 CNend
    \attention \n
    N/A
    \param [in] pstParam, Pointer of the open params. CNcomment:指针类型，指向实例打开参数 CNend
    \param [out] phInst, Pointer to instance handle. CNcomment:指针类型，指向实例句柄 CNend
    \retval ::MT_SUCCESS CNcomment:操作成功 CNend
    \retval ::MT_FAILURE CNcomment:操作失败 CNend
    \see \n
    N/A
    */
    mt_s32 (*Create)(mt_handle* phInst, const MT_CODEC_OPENPARAM_S * pstParam);

    /**
    \brief Destroy a codec instance. CNcomment:销毁编解码器实例 CNend
    \attention \n
    N/A
    \param [in] hInst, Instance handle. CNcomment:实例句柄 CNend
    \retval ::MT_SUCCESS CNcomment:操作成功 CNend
    \retval ::MT_FAILURE CNcomment:操作失败 CNend
    \see \n
    N/A
    */     
    mt_s32 (*Destroy)(mt_handle hInst);

    /**
    \brief Start a codec instance. CNcomment:启动编解码器实例 CNend
    \attention \n
    N/A
    \param [in] hInst, Instance handle. CNcomment:实例句柄 CNend
    \retval ::MT_SUCCESS CNcomment:操作成功 CNend
    \retval ::MT_FAILURE CNcomment:操作失败 CNend
    \see \n
    N/A
    */       
    mt_s32 (*Start)(mt_handle hInst);

    /**
    \brief Stop a codec instance. CNcomment:停止编解码器实例 CNend
    \attention \n
    N/A
    \param [in] hInst, Instance handle. CNcomment:实例句柄 CNend
    \retval ::MT_SUCCESS CNcomment:操作成功 CNend
    \retval ::MT_FAILURE CNcomment:操作失败 CNend
    \see \n
    N/A
    */       
    mt_s32 (*Stop)(mt_handle hInst);

    /**
    \brief Reset a codec instance. CNcomment:复位编解码器实例 CNend
    \attention \n
    N/A
    \param [in] hInst, Instance handle. CNcomment:实例句柄 CNend
    \retval ::MT_SUCCESS CNcomment:操作成功 CNend
    \retval ::MT_FAILURE CNcomment:操作失败 CNend
    \see \n
    N/A
    */        
    mt_s32 (*Reset)(mt_handle hInst, const MT_CODEC_RESETPARAM_S *pstParam);

    /**
    \brief Set attribute to a codec instance. CNcomment:设置编解码器属性 CNend
    \attention \n
    N/A
    \param [in] hInst, Instance handle. CNcomment:实例句柄 CNend
    \param [in] pstAttr, Pointer to the instance attribute. CNcomment:指针类型，指向实例属性 CNend
    \retval ::MT_SUCCESS CNcomment:操作成功 CNend
    \retval ::MT_FAILURE CNcomment:操作失败 CNend
    \see \n
    N/A
    */      
    mt_s32 (*SetAttr)(mt_handle hInst, const MT_CODEC_ATTR_S * pstAttr);

    /**
    \brief Get attribute to a codec instance. CNcomment:获取编解码器属性 CNend
    \attention \n
    N/A
    \param [in] hInst, Instance handle. CNcomment:实例句柄 CNend
    \param [out] pstAttr, Pointer to the instance attribute. CNcomment:指针类型，指向实例属性 CNend
    \retval ::MT_SUCCESS CNcomment:操作成功 CNend
    \retval ::MT_FAILURE CNcomment:操作失败 CNend
    \see \n
    N/A
    */        
    mt_s32 (*GetAttr)(mt_handle hInst, MT_CODEC_ATTR_S * pstAttr);

    /**
    \brief Decode a frame.. CNcomment:解码一帧 CNend
    \attention \n
    N/A
    \param [in] hInst, Instance handle. CNcomment:实例句柄 CNend
    \param [in] pstIn, Pointer to stream data descriptor. CNcomment:指针类型，指向码流数据信息结构体 CNend
    \param [out] pstOut, Pointer to frame data descriptor. CNcomment:指针类型，指向帧数据信息结构体 CNend
    \retval ::MT_SUCCESS CNcomment:操作成功 CNend
    \retval ::MT_FAILURE CNcomment:操作失败 CNend
    \see \n
    N/A
    */     
    mt_s32 (*DecodeFrame)(mt_handle hInst, MT_CODEC_STREAM_S * pstIn, MT_CODEC_FRAME_S * pstOut);

    /**
    \brief Encode a frame. CNcomment:编码一帧 CNend
    \attention \n
    N/A
    \param [in] hInst, Instance handle. CNcomment:实例句柄 CNend
    \param [in] pstIn, Pointer to stream data descriptor. CNcomment:指针类型，指向码流数据信息结构体 CNend
    \param [out] pstOut, Pointer to frame data descriptor. CNcomment:指针类型，指向帧数据信息结构体 CNend
    \retval ::MT_SUCCESS CNcomment:操作成功 CNend
    \retval ::MT_FAILURE CNcomment:操作失败 CNend
    \see \n
    N/A
    */     
    mt_s32 (*EncodeFrame)(mt_handle hInst, MT_CODEC_FRAME_S * pstIn, MT_CODEC_STREAM_S * pstOut);

    /**
    \brief Get stream information. CNcomment:获取码流信息 CNend
    \attention \n
    N/A
    \param [in] hInst, Instance handle. CNcomment:实例句柄 CNend
    \param [out] pstAttr, Pointer to stream information. CNcomment:指针类型，指向码流信息结构体 CNend
    \retval ::MT_SUCCESS CNcomment:操作成功 CNend
    \retval ::MT_FAILURE CNcomment:操作失败 CNend
    \see \n
    N/A
    */     
    mt_s32 (*GetStreamInfo)(mt_handle hInst, MT_CODEC_STREAMINFO_S * pstAttr);

	mt_s32 (*RegFrameBuffer)(mt_handle hInst, MT_CODEC_STREAM_S *pstRawPacket);
    /**
    \brief Other control, can be extended. CNcomment:其他控制选项，可扩展 CNend
    \attention \n
    N/A
    \param [in] hInst, Instance handle. CNcomment:实例句柄 CNend
    \param [out] u32CMD, The commond ID. CNcomment:命令ID CNend
    \param [out] pParam, Pointer to control parameter. CNcomment:指针类型，指向控制参数 CNend
    \retval ::MT_SUCCESS CNcomment:操作成功 CNend
    \retval ::MT_FAILURE CNcomment:操作失败 CNend
    \see \n
    N/A
    */     
    mt_s32 (*Control)(mt_handle hInst, mt_u32 u32CMD, mt_void * pParam);
}MT_CODEC_S;

/** @} */  /** <!-- ==== Structure Definition End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_VIDEO_CODEC_H__ */
