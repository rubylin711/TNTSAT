/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_EDID_H__
#define __MT_UNF_EDID_H__

#include "mt_unf_common.h"
#include "mt_unf_audio.h"
#include "mt_unf_video.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*************************** Structure Definition ****************************/
/** \addtogroup      EDID */
/** @{*/ /** <!-- [EDID] */

/**EDID Audio type enum*/ /**CNcomment:HDMI 音频类型 枚举 */
typedef enum mtUNF_EDID_AUDIO_FORMAT_CODE_E {
    MT_UNF_EDID_AUDIO_FORMAT_CODE_RESERVED = 0x00,
    /**<Audio coding type, refer stream,default type*/ /**<CNcomment:音频编码类型，参考数据流，缺省格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM,
    /**<Audio coding PCM type*/ /**<CNcomment:音频编码PCM格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_AC3,
    /**<Audio coding AC3 type*/ /**<CNcomment:音频编码AC3格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_MPEG1,
    /**<Audio coding MPEG1 type*/ /**<CNcomment:音频编码MPEG1格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_MP3,
    /**<Audio coding MP3 type*/ /**<CNcomment:音频编码MP3格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_MPEG2,
    /**<Audio coding MPEG2 type*/ /**<CNcomment:音频编码MPEG2格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_AAC,
    /**<Audio coding AAC type*/ /**<CNcomment:音频编码AAC格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS,
    /**<Audio coding DTS type*/ /**<CNcomment:音频编码DTS格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_ATRAC,
    /**<Audio coding ATRAC type*/ /**<CNcomment:音频编码ATRAC格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_ONE_BIT,
    /**<Audio coding ONE BIT AUDIO type*/ /**<CNcomment:音频编码ONE_BIT格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_DDP,
    /**<Audio coding DDPLUS type*/ /**<CNcomment:音频编码DDPLUS格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS_HD,
    /**<Audio coding DTS HD type*/ /**<CNcomment:音频编码DTS HD格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_MAT,
    /**<Audio coding MAT type*/ /**<CNcomment:音频编码MAT格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_DST,
    /**<Audio coding DST type*/ /**<CNcomment:音频编码DST格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_WMA_PRO,
    /**<Audio coding WMA PRO type*/ /**<CNcomment:音频编码WMA PRO格式 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_BUTT,
} MT_UNF_EDID_AUDIO_FORMAT_CODE_E;

/**the max audio sample rate count*/ /**CNcomment:最大音频采样率个数 */
#define MAX_SAMPE_RATE_NUM 8

/**the max audio bit depth count*/ /**CNcomment:最大音频比特深度个数 */
#define MAX_BIT_DEPTH_NUM 6

/**EDID Audio Info struct*/ /**CNcomment:EDID 音频信息结构体 */
typedef struct mtUNF_EDID_AUDIO_INFO_S
{
    MT_UNF_EDID_AUDIO_FORMAT_CODE_E enAudFmtCode; /**<Audio coding type*/                 /**<CNcomment:音频编码类型 */
    MT_UNF_SAMPLE_RATE_E enSupportSampleRate[MAX_SAMPE_RATE_NUM]; /**<Audio sample rate*/ /**<CNcomment:音频采样率 */
    mt_u32 u32SupportSampleRateNum; /**<Audio sample rate num*/                           /**<CNcomment:音频采样率类型 */
    mt_u8 u8AudChannel; /**<Aud Channel of the coding type*/                              /**<CNcomment:编码类型对应的音频通道能力 */

    MT_UNF_BIT_DEPTH_E bSupportBitDepth[MAX_BIT_DEPTH_NUM]; /*sample bit depth,when audio format code is MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM*/         /**<CNcomment:PCM类型下扩展信息为比特深度 */
    mt_u32 u32SupportBitDepthNum; /*sample bit depth Num,when audio format code is MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM*/                               /**<CNcomment:PCM类型下扩展信息为比特深度个数 */
    mt_u32 u32MaxBitRate; /**enter max bit rate,when audio format code is MT_UNF_EDID_AUDIO_FORMAT_CODE_AC3 - MT_UNF_EDID_AUDIO_FORMAT_CODE_ATRAC**/ /**<CNcomment:AC3-ATRAC时 扩展信息类型为最大比特速率*/
} MT_UNF_EDID_AUDIO_INFO_S;

/**EDID Audio speaker enum*/ /**CNcomment:EDID 音频扬声器配置枚举 */
typedef enum mtUNF_EDID_AUDIO_SPEAKER_E {
    MT_UNF_EDID_AUDIO_SPEAKER_FL_FR,
    MT_UNF_EDID_AUDIO_SPEAKER_LFE,
    MT_UNF_EDID_AUDIO_SPEAKER_FC,
    MT_UNF_EDID_AUDIO_SPEAKER_RL_RR,
    MT_UNF_EDID_AUDIO_SPEAKER_RC,
    MT_UNF_EDID_AUDIO_SPEAKER_FLC_FRC,
    MT_UNF_EDID_AUDIO_SPEAKER_RLC_RRC,
    MT_UNF_EDID_AUDIO_SPEAKER_FLW_FRW,
    MT_UNF_EDID_AUDIO_SPEAKER_FLH_FRH,
    MT_UNF_EDID_AUDIO_SPEAKER_TC,
    MT_UNF_EDID_AUDIO_SPEAKER_FCH,
    MT_UNF_EDID_AUDIO_SPEAKER_BUTT,
} MT_UNF_EDID_AUDIO_SPEAKER_E;

/**EDID MANUFACTURE Info struct*/ /**CNcomment:EDID制造商信息 */
typedef struct mtUNF_EDID_MANUFACTURE_INFO_E
{
    mt_u8 u8MfrsName[4]; /**<Manufacture name*/                 /**<CNcomment:设备厂商标识 */
    mt_u32 u32ProductCode; /**<Product code*/                   /**<CNcomment:设备ID */
    mt_u32 u32SerialNumber; /**<Serial numeber of Manufacture*/ /**<CNcomment:设备序列号 */
    mt_u32 u32Week; /**<the week of manufacture*/               /**<CNcomment:设备生产日期(周) */
    mt_u32 u32Year; /**<the year of manufacture*/               /**<CNcomment:设备生产日期(年) */
} MT_UNF_EDID_MANUFACTURE_INFO_S;

/**EDID Colorimety Info struct*/ /**CNcomment:EDID 色调信息结构体 */
typedef struct mtUNF_EDID_COLORIMETRY_S
{
    MT_BOOL bxvYCC601; /**<The sink is support xcYCC601 or not flag*/         /**<CNcomment:是否支持xvYCC601颜色格式 */
    MT_BOOL bxvYCC709; /**<The sink is support xvYCC709 or not flag*/         /**<CNcomment:是否支持xvYCC709颜色格式 */
    MT_BOOL bsYCC601; /**<The sink is support sYCC601 or not flag*/           /**<CNcomment:是否支持sYCC601颜色格式 */
    MT_BOOL bAdobleYCC601; /**<The sink is support AdobleYCC601 or not flag*/ /**<CNcomment:是否支持AdobleYCC601颜色格式 */
    MT_BOOL bAdobleRGB; /**<The sink is support AdobleRGB or not flag*/       /**<CNcomment:是否支持AdobleRGB颜色格式 */
} MT_UNF_EDID_COLORIMETRY_S;

/**EDID color space Info struct*/ /**CNcomment:EDID 色彩空间信息结构体 */
typedef struct mtUNF_EDID_COLOR_SPACE_S
{
    MT_BOOL bRGB444; /**<The sink is support RGB444 or not flag*/     /**<CNcomment:< 是否支持RGB444显示 */
    MT_BOOL bYCbCr422; /**<The sink is support YCbCr422 or not flag*/ /**<CNcomment:< 是否支持YCbCr422显示 */
    MT_BOOL bYCbCr444; /**<The sink is support YCbCr444 or not flag*/ /**<CNcomment:< 是否支持YCbCr444显示 */
} MT_UNF_EDID_COLOR_SPACE_S;

/**EDID cec address Info struct*/ /**CNcomment:EDID cec地址信息结构体 */
typedef struct mtUNF_EDID_CEC_ADDRESS_S
{
    MT_BOOL bPhyAddrValid; /**<the flag of phyiscs address is valid or not*/ /**<CNcomment:CEC物理地址是否有效标志 */
    mt_u8 u8PhyAddrA; /**<phyiscs address A of CEC*/                         /**<CNcomment:CEC物理地址A */
    mt_u8 u8PhyAddrB; /**<phyiscs address B of CEC*/                         /**<CNcomment:CEC物理地址B */
    mt_u8 u8PhyAddrC; /**<phyiscs address C of CEC*/                         /**<CNcomment:CEC物理地址C */
    mt_u8 u8PhyAddrD; /**<phyiscs address D of CEC*/                         /**<CNcomment:CEC物理地址D */
} MT_UNF_EDID_CEC_ADDRESS_S;

/**EDID deep color Info struct*/ /**CNcomment:EDID 深色信息结构体 */
typedef struct mtUNF_EDID_DEEP_COLOR_S
{
    MT_BOOL bDeepColorY444; /**<the Deep Color support YCBCR444 or not*/  /**<CNcomment:是否支持 YCBCR 4:4:4  Deep Color 模式 */
    MT_BOOL bDeepColor30Bit; /**<the Deep Color support 30 bit  or not */ /**<CNcomment:是否支持Deep Color 30bit 模式 */
    MT_BOOL bDeepColor36Bit; /**<the Deep Color support 36 bit  or not */ /**<CNcomment:是否支持Deep Color 36bit 模式 */
    MT_BOOL bDeepColor48Bit; /**<the Deep Color support 48 bit  or not */ /**<CNcomment:是否支持Deep Color 48bit 模式 */
    MT_BOOL bDeepColor30Bit_Y420; /**<the Deep Color support YCBCR420 30 bit  or not */ /**<CNcomment:是否支持Deep Color 30bit 模式 */
    MT_BOOL bDeepColor36Bit_Y420; /**<the Deep Color support YCBCR420 36 bit  or not */ /**<CNcomment:是否支持Deep Color 36bit 模式 */
    MT_BOOL bDeepColor48Bit_Y420; /**<the Deep Color support YCBCR420 48 bit  or not */ /**<CNcomment:是否支持Deep Color 48bit 模式 */
} MT_UNF_EDID_DEEP_COLOR_S;

/**HDMI 3d enum type*/
/**CNcomment: HDMI 3d 枚举类型 */
typedef enum mtUNF_EDID_3D_TYPE_E {
    MT_UNF_EDID_3D_FRAME_PACKETING = 0x00,
    /**<3d type:Frame Packing*/ /**<CNcomment:3d 模式:帧封装*/
    MT_UNF_EDID_3D_FIELD_ALTERNATIVE = 0x01,
    /**<3d type:Field alternative*/ /**<CNcomment:3d 模式:场交错*/
    MT_UNF_EDID_3D_LINE_ALTERNATIVE = 0x02,
    /**<3d type:Line alternative*/ /**<CNcomment:3d 模式:行交错*/
    MT_UNF_EDID_3D_SIDE_BY_SIDE_FULL = 0x03,
    /**<3d type:Side by side full*/ /**<CNcomment:3d 模式:并排式 左右全场*/
    MT_UNF_EDID_3D_L_DEPTH = 0x04,
    /**<3d type:L+depth*/ /**<CNcomment:3d 模式:L+DEPTH*/
    MT_UNF_EDID_3D_L_DEPTH_GRAPHICS_GRAPHICS_DEPTH = 0x05,
    /**<3d type:L+depth+Graphics+Graphics-depth*/ /**<CNcomment:3d 模式:L+depth+Graphics+Graphics-depth*/
    MT_UNF_EDID_3D_TOP_AND_BOTTOM = 0x06,
    /**<3d type:Top and Bottom*/ /**<CNcomment:3d 模式:上下模式*/
    MT_UNF_EDID_3D_SIDE_BY_SIDE_HALF = 0x08,
    /**<3d type:Side by side half*/ /**<CNcomment:3d 模式:并排式 左右半边*/
    MT_UNF_EDID_3D_BUTT,
} MT_UNF_EDID_3D_TYPE_E;

/**3d Info struct*/
/**CNcomment: 3d 信息结构体 */
typedef struct mtUNF_EDID_3D_INFO_S
{
    MT_BOOL bSupport3D; /**<flag of 3d*/                                 /**<CNcomment:是否支持3d*/
    MT_BOOL bSupport3DType[MT_UNF_EDID_3D_BUTT]; /**<supported 3d type*/ /**<CNcomment:支持的3d类型*/
} MT_UNF_EDID_3D_INFO_S;

/**EDID detailed Timing Info struct*/
/**CNcomment: EDID 详细时序*/
typedef struct mtUNF_EDID_TIMING_S
{
    mt_u32 u32VFB; /**<vertical front blank*/                       /**<CNcomment:垂直前消隐*/
    mt_u32 u32VBB; /**<vertical back blank*/                        /**<CNcomment:垂直后消隐*/
    mt_u32 u32VACT; /**<vertical active area*/                      /**<CNcomment:垂直有效区*/
    mt_u32 u32HFB; /**<horizonal front blank*/                      /**<CNcomment:水平前消隐*/
    mt_u32 u32HBB; /**<horizonal back blank*/                       /**<CNcomment:水平后消隐*/
    mt_u32 u32HACT; /**<horizonal active area*/                     /**<CNcomment:水平有效区*/
    mt_u32 u32VPW; /**<vertical sync pluse width*/                  /**<CNcomment:垂直脉冲宽度*/
    mt_u32 u32HPW; /**<horizonal sync pluse width*/                 /**<CNcomment:水平脉冲宽度*/
    MT_BOOL bIDV; /**< flag of data valid signal is needed flip*/   /**<CNcomment:有效数据信号是否翻转*/
    MT_BOOL bIHS; /**<flag of horizonal sync pluse is needed flip*/ /**<CNcomment:水平同步脉冲信号是否翻转*/
    MT_BOOL bIVS; /**<flag of vertical sync pluse is needed flip*/  /**<CNcomment:垂直同步脉冲信号是否翻转*/
    mt_u32 u32ImageWidth; /**<image width */                        /**<CNcomment:图像宽*/
    mt_u32 u32ImageHeight; /**<image height */                      /**<CNcomment:图像高 */
    mt_u32 u32AspectRatioW; /**<aspect ratio width */               /**<CNcomment:宽高比 */
    mt_u32 u32AspectRatioH; /**<aspect ratio height */              /**<CNcomment:宽高比 */
    MT_BOOL bInterlace; /**<flag of interlace */                    /**<CNcomment:逐隔行检测标记 */
    mt_s32 u32PixelClk; /**<pixelc clk for this timing */           /**<CNcomment:当前制式对应的时钟 */
} MT_UNF_EDID_TIMING_S;

/**the max audio capability count*/ /**CNcomment:最大音频能力集个数 */
#define MT_UNF_EDID_MAX_AUDIO_CAP_COUNT 16

/**HDMI sink capability of interface*/
/**CNcomment: HDMI sink 接口能力集 */
typedef struct mtUNF_EDID_BASE_INFO_S
{
    MT_BOOL bSupportHdmi; /**<The Device suppot HDMI or not,the device is DVI when nonsupport HDMI*/                                                          /**<CNcomment:设备是否支持HDMI，如果不支持，则为DVI设备.*/
    MT_UNF_ENC_FMT_E enNativeFormat; /**<The sink native video format*/                                                                                       /**<CNcomment:显示设备物理分辨率 */
    MT_BOOL bSupportFormat[MT_UNF_ENC_FMT_BUTT + 1]; /**<video capability,MT_TRUE:support the video display format;MT_FALSE:nonsupport the video display foramt*/ /**<CNcomment:视频能力集,MT_TRUE表示支持这种显示格式，MT_FALSE表示不支持 */
    MT_UNF_EDID_3D_INFO_S st3DInfo; /**<3d Info*/                                                                                                             /**<CNcomment:3d 能力集 */
    MT_UNF_EDID_DEEP_COLOR_S stDeepColor; /**<deep color Info*/                                                                                               /**<CNcomment:3d 能力集 */
    MT_UNF_EDID_COLORIMETRY_S stColorMetry; /**<colorimetry Info*/                                                                                            /**<CNcomment:3d 能力集 */
    MT_UNF_EDID_COLOR_SPACE_S stColorSpace; /**<color space Info*/                                                                                            /**<CNcomment:3d 能力集 */

    MT_UNF_EDID_AUDIO_INFO_S stAudioInfo[MT_UNF_EDID_MAX_AUDIO_CAP_COUNT]; /**<audio Info*/ /**<CNcomment:3d 能力集 */
    mt_u32 u32AudioInfoNum; /**<num of audio Info*/                                         /**<CNcomment:3d 能力集 */
    MT_BOOL bSupportAudioSpeaker[MT_UNF_EDID_AUDIO_SPEAKER_BUTT]; /**<speaker Info*/        /**<CNcomment:3d 能力集 */

    mt_u8 u8ExtBlockNum; /**<edid extend block num Info*/ /**<CNcomment:edid扩展块个数 */

    mt_u8 u8Version; /**<the version of manufacture*/   /**<CNcomment:设备版本号 */
    mt_u8 u8Revision; /**<the revision of manufacture*/ /**<CNcomment:设备子版本号 */
    MT_UNF_EDID_MANUFACTURE_INFO_S stMfrsInfo;

    MT_UNF_EDID_CEC_ADDRESS_S stCECAddr; /**<cec address Info*/           /**<CNcomment:cec地址信息 */
    MT_BOOL bSupportDVIDual; /**<the DVI support dual-link or not */      /**<CNcomment:是否支持 DVI dual-link 操作 */
    MT_BOOL bSupportsAI; /**<support AI or not */                         /**<CNcomment:是否支持 Supports_AI 模式 */
    MT_UNF_EDID_TIMING_S stPerferTiming; /**<first detailed timing Info*/ /**<CNcomment:VESA最佳详细制式信息 */
	mt_u16		u16SupportEMP; /**<unsupport*/ /**<CNcomment:不支持 */
	MT_BOOL		bSupportHDR10pVSIF; /**<unsupport*/ /**<CNcomment:不支持 */
	MT_BOOL		bSupportHDR10pVivid; /**<unsupport*/ /**<CNcomment:不支持 */
	MT_BOOL		bSupportScdc; /**<unsupport*/ /**<CNcomment:不支持 */
	MT_BOOL		bSupportLTE; /**<unsupport*/ /**<CNcomment:不支持 */
	MT_BOOL 	bSupportDdMat48k; /**<unsupport*/ /**<CNcomment:不支持 */
} MT_UNF_EDID_BASE_INFO_S;

/**enum define about Video port*/
/**CNcomment:定义视频接口*/
typedef enum mtUNF_EDID_VIDEO_PORT_E {
    MT_UNF_EDID_VIDEO_PORT_VGA = 0x00,
    /**< VGA port */ /**<CNcomment: VGA接口 */
    MT_UNF_EDID_VIDEO_PORT_DVI,
    /**< DVI port */ /**<CNcomment: DVI接口 */
    MT_UNF_EDID_VIDEO_PORT_HDMI,
    /**< HDMI port */ /**<CNcomment: HDMI 接口 */
    MT_UNF_EDID_VIDEO_PORT_BUTT
} MT_UNF_EDID_VIDEO_PORT_E;

/**enum define about timing protocol type*/
/**CNcomment:定义时序的协议类型*/
typedef enum mtUNF_EDID_TIMING_TYPE_E {
    MT_UNF_EDID_TIMING_TYPE_DMT = 0x00,
    /**< DMT  protocol Timing */ /**<CNcomment: DMT协议时序 */
    MT_UNF_EDID_TIMING_TYPE_861,
    /**< 861D  protocol Timing */ /**<CNcomment: 861D协议时序 */
    MT_UNF_EDID_TIMING_TYPE_CVT,
    /**< CVT  protocol Timing */ /**<CNcomment: CVT协议时序 */
    MT_UNF_EDID_TIMING_TYPE_CVT_RB,
    /**< CVT_RB  protocol Timing */ /**<CNcomment: CVT_RB协议时序 */
    MT_UNF_EDID_TIMING_TYPE_GTF,
    /**< GTF  protocol Timing */ /**<CNcomment: GTF协议时序 */
    MT_UNF_EDID_TIMING_TYPE_BUTT
} MT_UNF_EDID_TIMING_TYPE_E;

/**enum define about timing type*/
/**CNcomment:定义时序的类型*/
typedef enum mtUNF_EDID_TIMING_ATTR_E {
    MT_UNF_EDID_TIMING_ATTR_NONE = 0x00,
    /**< None */ /**<CNcomment: 无 */
    MT_UNF_EDID_TIMING_ATTR_PREFERRED_TIMING,
    /**< None */ /**<CNcomment: 最佳时序*/
    MT_UNF_EDID_TIMING_ATTR_PREFERRED_VERTICAL_FREQ,
    /**< None */ /**<CNcomment: 最佳刷新率*/
    MT_UNF_EDID_TIMING_ATTR_BUTT
} MT_UNF_EDID_TIMING_ATTR_E;

/**Detailed Timing Definitions Sync Signal Definitions*/
/**CNcomment:详细时序中定义同步类型*/
typedef enum mtUNF_EDID_SYNC_TYPE_E {
    /*Analog Sync Signal Definitions*/
    MT_UNF_EDID_SYNC_ACS_WS_GREEN = 0x00,
    /**<Analog Composite Sync - Without Serrations - Sync On Green Signal only*/ /**<CNcomment: 模拟复合同步去垂直同步时期的水平锯齿，绿色通道叠加同步*/
    MT_UNF_EDID_SYNC_ACS_WS_ALL,
    /**<Analog Composite Sync - Without Serrations - Sync On all three (RGB) video signals*/ /**<CNcomment: 模拟复合同步去垂直同步时期的水平锯齿，三个通道叠加同步*/
    MT_UNF_EDID_SYNC_ACS_DS_GREEN,
    /**<Analog Composite Sync - With Serrations (H-sync during V-sync); - Sync On Green Signal only*/ /**<CNcomment: 模拟复合同步保持垂直同步时期的水平锯齿，绿色通道叠加同步*/
    MT_UNF_EDID_SYNC_ACS_DS_ALL,
    /**<Analog Composite Sync - With Serrations (H-sync during V-sync); - Sync On all three (RGB) video signals*/ /**<CNcomment: 模拟复合同步保持垂直同步时期的水平锯齿，三个通道叠加同步*/
    MT_UNF_EDID_SYNC_BACS_WS_GREEN,
    /**<Bipolar Analog Composite Sync - Without Serrations; - Sync On Green Signal only*/ /**<CNcomment: 模拟复合同步去垂直同步时期的水平锯齿，三个通道叠加同步*/
    MT_UNF_EDID_SYNC_BACS_WS_ALL,
    /**<Bipolar Analog Composite Sync - Without Serrations; - Sync On all three (RGB) video signals*/ /**<CNcomment: 模拟复合同步去垂直同步时期的水平锯齿，三个通道叠加同步*/
    MT_UNF_EDID_SYNC_BACS_DS_GREEN,
    /**<Bipolar Analog Composite Sync - With Serrations (H-sync during V-sync); - Sync On Green Signal only*/ /**<CNcomment:  极性模拟复合同步保持垂直同步时期的水平锯齿，绿色通道叠加同步*/
    MT_UNF_EDID_SYNC_BACS_DS_ALL,
    /**<Bipolar Analog Composite Sync - With Serrations (H-sync during V-sync); - Sync On all three (RGB) video signals*/ /**<CNcomment:  极性模拟复合同步保持垂直同步时期的水平锯齿，绿色通道叠加同步*/
    /*Digital Sync Signal Definitions*/
    MT_UNF_EDID_SYNC_DCS_WS,
    /**<Digital Composite Sync - Without Serrations*/ /**<CNcomment: 数字复合同步，去垂直同步期间行同步锯齿信号*/
    MT_UNF_EDID_SYNC_DCS_DS,
    /**<Digital Composite Sync - With Serrations (H-sync during V-sync)*/ /**<CNcomment: 数字复合同步，保持垂直同步期间行同步锯齿信号*/
    MT_UNF_EDID_SYNC_DSS_VN_HN,
    /**<Digital Separate Sync Vsync(-) Hsync(-)*/ /**<CNcomment: 数字复合同步，水平和垂直同步极性(-)(-)*/
    MT_UNF_EDID_SYNC_DSS_VN_HP,
    /**<Digital Separate Sync Vsync(-) Hsync(+)*/ /**<CNcomment: 数字复合同步，水平和垂直同步极性(-)(+)*/
    MT_UNF_EDID_SYNC_DSS_VP_HN,
    /**<Digital Separate Sync Vsync(+) Hsync(-)*/ /**<CNcomment: 数字复合同步，水平和垂直同步极性(+)(-)*/
    MT_UNF_EDID_SYNC_DSS_VP_HP,
    /**<Digital Separate Sync Vsync(+) Hsync(+)*/ /**<CNcomment: 数字复合同步，水平和垂直同步极性(+)(+)*/
    MT_UNF_EDID_SYNC_BUTT
} MT_UNF_EDID_SYNC_TYPE_E;

/**Simple Timing Definitions*/
/**CNcomment:定义简要时序*/
typedef struct mtUNF_EDID_SIMPLE_TIMING_S
{
    mt_u32 u32Vact; /**<  Active Line */                                 /**<CNcomment: 有效行*/
    mt_u32 u32Hact; /**< Active Pixels */                                /**<CNcomment: 有效像素*/
    mt_u32 u32VerFreq; /**<Ver Frequency */                              /**<CNcomment: 刷新率 */
    MT_UNF_EDID_TIMING_TYPE_E enTimingType; /**< Timing protocol type */ /**<CNcomment: 定义时序的协议类型*/
    MT_UNF_EDID_TIMING_ATTR_E enTimingAttr; /**<timing type */           /**<CNcomment: :定义时序的类型*/
} MT_UNF_EDID_SIMPLE_TIMING_S;

/**Detailed Timing Definitions*/
/**CNcomment:定义详细时序*/
typedef struct mtUNF_EDID_DETAIL_TIMING_S
{
    MT_UNF_EDID_TIMING_S stTiming; /**<  Detailed Timing */                                      /**<CNcomment: 详细时序*/
    MT_UNF_EDID_SYNC_TYPE_E enSyncType; /**Detailed Timing Definitions Sync Signal Definitions*/ /**CNcomment:详细时序中定义同步类型*/
    MT_UNF_EDID_TIMING_ATTR_E enTimingAttr; /**<timing type */                                   /**<CNcomment: :定义时序的类型*/
} MT_UNF_EDID_DETAIL_TIMING_S;

/**EDID information*/
/**CNcomment:EDID解析信息*/
typedef struct mtUNF_EDID_INFO_S
{
    MT_UNF_EDID_BASE_INFO_S stEDIDBaseInfo; /**< EDID base information */ /**<CNcomment: EDID基本信息 */
    MT_UNF_EDID_VIDEO_PORT_E enVideoPort; /**<video port  */              /**<CNcomment: 视频接口类型 */
    MT_UNF_EDID_SIMPLE_TIMING_S *pstSimpleTiming; /**<Simple timing   */  /**<CNcomment: 简要时序*/
    mt_u32 u32SimpleTimingNum; /**<Simple timing number */                /**<CNcomment: 简要时序个数*/
    MT_UNF_EDID_DETAIL_TIMING_S *pstDetailTiming; /**<Simple timing */    /**<CNcomment: 详细时序个数*/
    mt_u32 u32DetailTimingNum; /**<Simple timing number */                /**<CNcomment: 详细时序个数*/
} MT_UNF_EDID_INFO_S;

/** @}*/ /** <!-- ==== Structure Definition End ====*/

/******************************* API Declaration *****************************/
/** \addtogroup      EDID*/
/** @{*/ /** <!-- [EDID] */

/**
\brief parse the edid information. CNcomment:解析EDID数据 CNend
\attention \n
\param[in]  u32EdidLength EDID data length . CNcomment:EDID数据 的结构 长度 CNend
\param[in]  *pu8Edid  EDID data point . CNcomment:EDID数据 的指针CNend
\param[in]  *pstEdidInfo  EDID parse information . CNcomment:EDID解析信息指针CNend

\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_EDID_EdidParse(mt_u8 *pu8Edid, mt_u32 u32EdidLength, MT_UNF_EDID_INFO_S *pstEdidInfo);

/**
\brief release the edid information. CNcomment:释放EDID信息 CNend
\attention \n
\param[in]  *pstEdidInfo  EDID parse information . CNcomment:EDID解析信息指针CNend
\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_void MT_UNF_EDID_EdidRelease(MT_UNF_EDID_INFO_S *pstEdidInfo);

/** @} */ /** <!-- ==== API declaration end ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_UNF_EDID_H__ */
