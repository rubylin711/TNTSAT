/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_DRV_HDMI_H__
#define __MT_DRV_HDMI_H__

//#include "mt_common_id.h"
#include "mt_module.h"
//#include "mt_common_log.h"
#include "mt_debug.h"

#include "mt_unf_hdmi.h"
#include "mt_error_mpi.h"
#include "mt_drv_disp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
**HDMI Debug
*/
#ifndef MT_ADVCA_FUNCTION_RELEASE
#define MT_FATAL_HDMI(fmt...) MT_FATAL_PRINT(MT_ID_HDMI, fmt)
#define MT_ERR_HDMI(fmt...) MT_ERR_PRINT(MT_ID_HDMI, fmt)
#define MT_WARN_HDMI(fmt...) MT_WARN_PRINT(MT_ID_HDMI, fmt)
#define MT_INFO_HDMI(fmt...) MT_INFO_PRINT(MT_ID_HDMI, fmt)

#define debug_printk(fmt, args...) // printk(fmt,##args)
#else

#define MT_FATAL_HDMI(fmt...)
#define MT_ERR_HDMI(fmt...)
#define MT_WARN_HDMI(fmt...)
#define MT_INFO_HDMI(fmt...)
#define debug_printk(fmt, args...)

#endif

/*hdmi audio interface */
typedef enum mtHDMI_AUDIOINTERFACE_E {
    HDMI_AUDIO_INTERFACE_I2S,
    HDMI_AUDIO_INTERFACE_SPDIF,
    HDMI_AUDIO_INTERFACE_HBR,
    HDMI_AUDIO_INTERFACE_BUTT
} HDMI_AUDIOINTERFACE_E;

typedef struct mtHDMI_AUDIO_ATTR_S
{
    //  MT_BOOL                 bEnableAudio;        /**<Enable flag of Audio*//**CNcomment:是否Enable音频 */
    HDMI_AUDIOINTERFACE_E enSoundIntf; /**<the origin of Sound,suggestion set MT_UNF_SND_INTERFACE_I2S,the parameter need consistent with Ao input */ /**<CNcomment:HDMI音频来源, 建议MT_UNF_SND_INTERFACE_I2S,此参数需要与AO输入保持一致 */
    MT_BOOL bIsMultiChannel; /**<set mutiChannel or stereo ;0:stereo,1:mutichannel fixup 8 channel */                                                 /**<CNcomment:多声道还是立体声，0:立体声，1:多声道固定为8声道 */
    mt_u32 u32Channels;                                                                                                                               //先channel和multy channel都保留，后续在内核态干掉multy channel
    MT_UNF_SAMPLE_RATE_E enSampleRate; /**<the samplerate of audio,this parameter consistent with AO config */                                        /**<CNcomment:PCM音频采样率,此参数需要与AO的配置保持一致 */
    mt_u8 u8DownSampleParm; /**<PCM parameter of dowmsample,default 0*/                                                                               /**CNcomment:PCM音频向下downsample采样率的参数，默认为0 */

    MT_UNF_BIT_DEPTH_E enBitDepth;                                    //目前默认配16bit    /**<the audio bit depth,defualt 16,this parameter consistent with AO config*//**<CNcomment:音频位宽，默认为16,此参数需要与AO的配置保持一致 */
    mt_u8 u8I2SCtlVbit; /**<reserve:config 0,I2S control(0x7A:0x1D)*/ /**CNcomment:保留，请配置为0, I2S control (0x7A:0x1D) */

    MT_UNF_EDID_AUDIO_FORMAT_CODE_E enAudioCode;
    //  MT_BOOL                 bEnableAudInfoFrame; /**<Enable flag of Audio InfoFrame,suggestion:enable*//**<CNcomment:是否使能 AUDIO InfoFrame，建议使能 */
} HDMI_AUDIO_ATTR_S;

/*In order to extern ,so we define struct*/
typedef struct mtHDMI_VIDEO_ATTR_S
{
    //	MT_BOOL                 bEnableHdmi;         /**<force to HDMI or DVI,the value must set before MT_UNF_HDMI_Start or behind MT_UNF_HDMI_Stop*//**<CNcomment:是否强制HDMI,否则为DVI.该值必须在 MT_UNF_HDMI_Start之前或者MT_UNF_HDMI_Stop之后设置  */
    //  MT_BOOL                 bEnableVideo;        /**<parameter must set MT_TRUE,or the HDMI diver will force to set MT_TRUE*//**<CNcomment:必须是MT_TRUE, 如果是MT_FALSE:HDMI驱动会强制设置为MT_TRUE */
    MT_DRV_DISP_FMT_E enVideoFmt; /**<video fromat ,the format must consistent with display  config*/ /**<CNcomment:视频制式,此参数需要与Display配置的制式保持一致 */
                                                                                                      //  MT_UNF_HDMI_VIDEO_MODE_E enVidOutMode;       /**<HDMI output vedio mode VIDEO_MODE_YCBCR,VIDEO_MODE_YCBCR444，VIDEO_MODE_YCBCR422，VIDEO_MODE_RGB444 *//**<CNcomment:HDMI输出视频模式，VIDEO_MODE_YCBCR444，VIDEO_MODE_YCBCR422，VIDEO_MODE_RGB444 */
                                                                                                      //  MT_UNF_HDMI_DEEP_COLOR_E enDeepColorMode;    /**<Deep Color output mode,defualt: MT_UNF_HDMI_DEEP_COLOR_24BIT *//**<CNcomment:DeepColor输出模式, 默认为MT_UNF_HDMI_DEEP_COLOR_24BIT */
                                                                                                      //  MT_BOOL                 bxvYCCMode;          /**<the xvYCC output mode,default:MT_FALSE*//**<CNcomment:< xvYCC输出模式，默认为MT_FALSE */

    //  MT_BOOL                 bEnableAviInfoFrame; /**<Enable flag of AVI InfoFrame,suggestion:enable *//**<CNcomment:是否使能 AVI InfoFrame，建议使能 */
    //  MT_BOOL                 bEnableSpdInfoFrame; /**<Enable flag of SPD info frame,suggestion:disable*//**<CNcomment:是否使能 SPD InfoFrame， 建议关闭 */
    //  MT_BOOL                 bEnableMpegInfoFrame;/**<Enable flag of MPEG info frame,suggestion:disable*//**<CNcomment:是否使能 MPEG InfoFrame， 建议关闭 */

    MT_BOOL b3DEnable; /**<0:disable 3d,1,enable 3d mode*/ /**<CNcomment:< 0:3D不激活，1:3D模式打开 */
    mt_u32 u83DParam; /**<3D Parameter,defualt MT_FALSE*/  /**<CNcomment:< 3D Parameter, 默认为MT_FALSE */

    //  mt_u32                  bDebugFlag;          /**<the flag of hdmi dubug,suggestion:disable*//**<CNcomment:< 是否使能 打开hdmi内部debug信息， 建议关闭 */
    //  MT_BOOL                 bHDCPEnable;         /**<0:HDCP disable mode,1:eable HDCP mode*//**<CNcomment:< 0:HDCP不激活，1:HDCP模式打开 */
} HDMI_VIDEO_ATTR_S;

/*In order to extern ,so we define struct*/
typedef struct mtHDMI_APP_ATTR_S
{
    MT_BOOL bEnableHdmi; /**<force to HDMI or DVI,the value must set before MT_UNF_HDMI_Start or behind MT_UNF_HDMI_Stop*/ /**<CNcomment:是否强制HDMI,否则为DVI.该值必须在 MT_UNF_HDMI_Start之前或者MT_UNF_HDMI_Stop之后设置  */
    MT_BOOL bEnableVideo; /**<parameter must set MT_TRUE,or the HDMI diver will force to set MT_TRUE*/                     /**<CNcomment:必须是MT_TRUE, 如果是MT_FALSE:HDMI驱动会强制设置为MT_TRUE */
    MT_BOOL bEnableAudio; /**<Enable flag of Audio*/                                                                       /**CNcomment:是否Enable音频 */

    MT_UNF_HDMI_VIDEO_MODE_E enVidOutMode; /**<HDMI output vedio mode VIDEO_MODE_YCBCR,VIDEO_MODE_YCBCR444，VIDEO_MODE_YCBCR422，VIDEO_MODE_RGB444 */ /**<CNcomment:HDMI输出视频模式，VIDEO_MODE_YCBCR444，VIDEO_MODE_YCBCR422，VIDEO_MODE_RGB444 */
    MT_UNF_HDMI_DEEP_COLOR_E enDeepColorMode; /**<Deep Color output mode,defualt: MT_UNF_HDMI_DEEP_COLOR_24BIT */                                     /**<CNcomment:DeepColor输出模式, 默认为MT_UNF_HDMI_DEEP_COLOR_24BIT */
    MT_BOOL bxvYCCMode; /**<the xvYCC output mode,default:MT_FALSE*/                                                                                  /**<CNcomment:< xvYCC输出模式，默认为MT_FALSE */

    MT_BOOL bEnableAviInfoFrame; /**<Enable flag of AVI InfoFrame,suggestion:enable */    /**<CNcomment:是否使能 AVI InfoFrame，建议使能 */
    MT_BOOL bEnableSpdInfoFrame; /**<Enable flag of SPD info frame,suggestion:disable*/   /**<CNcomment:是否使能 SPD InfoFrame， 建议关闭 */
    MT_BOOL bEnableMpegInfoFrame; /**<Enable flag of MPEG info frame,suggestion:disable*/ /**<CNcomment:是否使能 MPEG InfoFrame， 建议关闭 */
    MT_BOOL bEnableAudInfoFrame; /**<Enable flag of Audio InfoFrame,suggestion:enable*/   /**<CNcomment:是否使能 AUDIO InfoFrame，建议使能 */

    mt_u32 bDebugFlag; /**<the flag of hdmi dubug,suggestion:disable*/ /**<CNcomment:< 是否使能 打开hdmi内部debug信息， 建议关闭 */
    MT_BOOL bHDCPEnable; /**<0:HDCP disable mode,1:eable HDCP mode*/   /**<CNcomment:< 0:HDCP不激活，1:HDCP模式打开 */
} HDMI_APP_ATTR_S;

/*In order to extern ,so we define struct*/
typedef struct mtHDMI_APP_ATTRMT_S
{
	HDMI_APP_ATTR_S stAppAttr;
	mt_u32    tmds_clk;
	mt_u32    os_clk;
	MT_DRV_DISP_FMT_E fmt;
} HDMI_APP_ATTRMT_S;


/*hdmi struct */
typedef struct mtHDMI_ATTR_S
{
    HDMI_AUDIO_ATTR_S stAudioAttr;
    HDMI_VIDEO_ATTR_S stVideoAttr;
    HDMI_APP_ATTR_S stAppAttr;
} HDMI_ATTR_S;

typedef struct mtHDMI_AUDIO_CAPABILITY_S
{
    MT_BOOL bAudioFmtSupported[MT_UNF_EDID_AUDIO_FORMAT_CODE_BUTT]; /**<Audio capability,reference EIA-CEA-861-D,table 37,MT_TRUE:support this Audio type;MT_FALSE,nonsupport this Audio type*/ /**<CNcomment:音频能力集, 请参考EIA-CEA-861-D 表37;MT_TRUE表示支持这种显示格式，MT_FALSE表示不支持 */
    mt_u32 u32AudioSampleRateSupported[MAX_SAMPE_RATE_NUM]; /**<PCM smprate capability,0: illegal value,other is support PCM smprate */                                                         /**<CNcomment:PCM音频采样率能力集，0为非法值，其他为支持的音频采样率 */
    mt_u32 u32MaxPcmChannels; /**<Audio max PCM Channels number*/                                                                                                                               /**CNcomment:音频最大的PCM通道数 */
                                                                                                                                                                                                //mt_u8             u8Speaker;                /**<Speaker location,please reference EIA-CEA-D the definition of SpekearDATABlock*//**<CNcomment:扬声器位置，请参考EIA-CEA-861-D中SpeakerDATABlock的定义 */
} MT_DRV_HDMI_AUDIO_CAPABILITY_S;

mt_s32 MT_DRV_HDMI_Init(mt_void);
mt_void MT_DRV_HDMI_Deinit(mt_void);
mt_s32 MT_DRV_HDMI_Open(MT_UNF_HDMI_ID_E enHdmi);
mt_s32 MT_DRV_HDMI_Close(MT_UNF_HDMI_ID_E enHdmi);

mt_s32 MT_DRV_HDMI_PlayStus(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *pu32Stutus);
mt_s32 MT_DRV_AO_HDMI_GetAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr);
mt_s32 MT_DRV_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_EDID_BASE_INFO_S *pstSinkCap);
mt_s32 MT_DRV_HDMI_GetAudioCapability(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_HDMI_AUDIO_CAPABILITY_S *pstAudCap);
mt_s32 MT_DRV_HDMI_SetAudioMute(MT_UNF_HDMI_ID_E enHdmi);
mt_s32 MT_DRV_HDMI_SetAudioUnMute(MT_UNF_HDMI_ID_E enHdmi);

mt_s32 MT_DRV_HDMI_AudioChange(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr);

mt_s32 MT_DRV_HDMI_PreFormat(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_DISP_FMT_E enEncodingFormat);
mt_s32 MT_DRV_HDMI_SetFormat(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_DISP_FMT_E enFmt, MT_DRV_DISP_STEREO_E enStereo);

mt_s32 MT_DRV_HDMI_Detach(MT_UNF_HDMI_ID_E enHdmi);
mt_s32 MT_DRV_HDMI_Attach(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_DISP_FMT_E enFmt, MT_DRV_DISP_STEREO_E enStereo);

mt_void MT_DRV_HDMI_ProcHotPlug(mt_handle hHdmi);
mt_s32 MT_DRV_HDMI_ExtIoctl(unsigned int cmd, void *argp);
mt_s32 MT_DRV_HDMI_GetCECAddr(mt_u32 *pcec_addr);
mt_s32 MT_DRV_HDMI_GetBinInfoFrame(MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, void *infor_ptr);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
/*--------------------------END-------------------------------*/
