/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_UNF_AI_H__
#define __MT_UNF_AI_H__

#include "mt_unf_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/********************************Macro Definition********************************/
/** \addtogroup      AI */
/** @{ */ /** <!-- AI */

/** @} */ /** <!-- ==== Macro Definition end ==== */

/*************************** Structure Definition ****************************/
/** \addtogroup      AI */
/** @{ */ /** <!--  【AI】 */

/**Audio inputport defination */
/**CNcomment:音频输入端口定义*/
typedef enum mtUNF_AI_E {
    MT_UNF_AI_I2S0 = 0,
    MT_UNF_AI_I2S1,
    MT_UNF_AI_ADC0 = 0x10,
    MT_UNF_AI_ADC1,
    MT_UNF_AI_ADC2,
    MT_UNF_AI_ADC3,
    MT_UNF_AI_ADC4,
    MT_UNF_AI_SIF0 = 0x20,
    MT_UNF_AI_HDMI0 = 0x30,
    MT_UNF_AI_HDMI1,
    MT_UNF_AI_HDMI2,
    MT_UNF_AI_HDMI3,
    MT_UNF_AI_BUTT = 0xff,
} MT_UNF_AI_E;

/**Defines internal Audio ADC inputport attribute */
/**CNcomment:定义内置音频ADC输入端口属性*/
typedef struct mtUNF_AI_ADC_ATTR_S
{
    MT_BOOL bByPass;
} MT_UNF_AI_ADC_ATTR_S;

/**Defines  Audio I2S inputport attribute */
/**CNcomment:定义音频I2S输入端口属性*/
typedef struct mtUNF_AI_I2S_ATTR_S
{
    MT_UNF_I2S_ATTR_S stAttr; /**<I2S Attribute*/ /**<CNcomment:I2S属性*/
} MT_UNF_AI_I2S_ATTR_S;

/**Defines the HDMI RX Audio data format .*/
typedef enum mtMT_UNF_AI_HDMI_FORMAT_E {
    MT_UNF_AI_HDMI_FORMAT_LPCM = 0, /* LPCM 2/8 channels,  Audio Sample Packet layout0 or layout1 */
    MT_UNF_AI_HDMI_FORMAT_LBR = 1,  /* IEC-61937 DD/DDP/DTS, Audio Sample Packet layout0 */
    MT_UNF_AI_HDMI_FORMAT_HBR = 8,  /* IEC-61937 DTSHD/TrueHD, High-Bitrate (HBR) Audio Stream Packet*/
    MT_UNF_AI_HDMI_FORMAT_BUTT
} MT_UNF_AI_HDMI_FORMAT_E;

/**Defines  HDMI Audio inputport attribute */
/**CNcomment:定义HDMI音频输入端口属性*/
typedef struct mtUNF_AI_HDMI_ATTR_S
{
    MT_UNF_I2S_CHNUM_E enChannel; /**<Channel number*/                          /**<CNcomment:通道数*/
    MT_UNF_I2S_BITDEPTH_E enBitDepth; /**<Bit Depth*/                           /**<CNcomment:位宽*/
    MT_UNF_SAMPLE_RATE_E enSampleRate; /**<Sample Rate*/                        /**<CNcomment:采样率*/
    MT_UNF_AI_HDMI_FORMAT_E enHdmiAudioDataFormat; /**<HDMI audio data format*/ /**<CNcomment:HDMI音频数据格式*/
} MT_UNF_AI_HDMI_ATTR_S;

/**Defines internal SIF(Audio Demodulator) inputport attribute */
/**CNcomment:定义SIF输入端口属性*/
typedef struct mtUNF_AI_SIF_ATTR_S
{
    mt_void *pPara;
} MT_UNF_AI_SIF_ATTR_S;

/**Defines  Audio inputport attribute */
/**CNcomment:定义音频输入端口属性*/
typedef struct mtMT_UNF_AI_ATTR_S
{
    MT_UNF_SAMPLE_RATE_E enSampleRate; /**<samplerate, default 48000Hz*/                /**<CNcomment:设备采样频率，默认 48000Hz*/
    mt_u32 u32PcmFrameMaxNum; /**<Max frame of the PCM data at cast buffer, default 6*/ /**<CNcomment: 最大可缓存帧数，默认 6*/
    mt_u32 u32PcmSamplesPerFrame; /**<Number of sample of the PCM data, default 960*/   /**<CNcomment: PCM数据采样点数量，默认 1024*/
    MT_BOOL bInterlaceMod;
    union
    {
	MT_UNF_AI_ADC_ATTR_S stAdcAttr; /**<ADC Attribute*/    /**<CNcomment:ADC属性*/
	MT_UNF_AI_I2S_ATTR_S stI2sAttr; /**<I2S Attribute*/    /**<CNcomment:I2S属性*/
	MT_UNF_AI_HDMI_ATTR_S stHDMIAttr; /**<HDMI Attribute*/ /**<CNcomment:HDMI属性*/
    } unAttr;
} MT_UNF_AI_ATTR_S;

/**Defines AI Delay attribute */
/**CNcomment:定义AI延迟属性*/
typedef struct mtMT_UNF_AI_DELAY_S
{
    mt_u32 u32DelayMs;                                                                   /**<buffer delay compensation(ms), Min Value is 20, Max Value depends on u32PcmFrameMaxNum, u32PcmSamplesPerFrame and enSampleRate in MT_UNF_AI_ATTR_S.
                                                    MaxValue = u32PcmSamplesPerFrame * u32PcmFrameMaxNum * 1000 / enSampleRate. 
                                                    MaxValue is equal to 320ms according to default MT_UNF_AI_ATTR_S*/
                                                                                         /**<CNcomment:缓存中数据量延迟补偿控制(ms), 最小值为20ms，最大值依赖MT_UNF_AI_ATTR_S中的u32PcmFrameMaxNum，u32PcmSamplesPerFrame以及enSampleRate
                                                    计算公式:MaxValue = u32PcmSamplesPerFrame * u32PcmFrameMaxNum * 1000 / enSampleRate，
                                                    若根据默认MT_UNF_AI_ATTR_S，则最大值为320ms */
    MT_BOOL bDelayMsAutoHold; /**<if hold buffer delay compensation time automatically*/ /**<CNcomment:是否由AI_AO通路自动保持缓存数据量稳定在u32CompensationMs*/
} MT_UNF_AI_DELAY_S;

/** @} */ /** <!-- ==== Structure Definition end ==== */

/******************************* API declaration *****************************/
/** \addtogroup      AI */
/** @{ */ /** <!--  【AI】 */

/**
\brief Initializes an AI device. CNcomment:初始化音频输入设备 CNend
\attention \n
Before calling the AI module, you must call this application programming interface (API). CNcomment:调用AI模块要求首先调用该接口 CNend
\param N/A
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AI_Init(mt_void);

/**
\brief Deinitializes an AI device. CNcomment:去初始化音频输入设备 CNend
\attention \n
\param N/A
\retval ::MT_SUCCESS Success CNcomment:成功  CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AI_DeInit(mt_void);

/**
 \brief Obtains the default attributes of a AI port. CNcomment: 获取AI端口默认属性 CNend
 \attention \n
 \param[in] pstAttr Pointer to AI attributes CNcomment: AI属性指针 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \retval ::MT_ERR_AI_NULL_PTR   The pointer is null. CNcomment: 空指针 CNend
 \see \n
N/A CNcomment: 无 CNend
 */
mt_s32 MT_UNF_AI_GetDefaultAttr(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr);

/**
 \brief Enables a AI port to create a AI handle. CNcomment: 打开AI Port，创建AI句柄 CNend
 \attention \n
Before calling the AI, you must call this API.\n
CNcomment: 调用AI 模块需要首先调用该接口 CNend
 \param[in] pstAttr CNcomment: Pointer to AI attributes AI属性指针 CNend
 \param[in] phandle CNcomment: hAi Pointer to the AI handle AI句柄指针 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \retval ::MT_ERR_AI_NULL_PTR   The pointer is null. CNcomment: 空指针 CNend
 \retval ::MT_ERR_AI_INVALID_PARA   The attribute parameters are incorrect. CNcomment: 属性参数错误 CNend
 \see \n
N/A CNcomment: 无 CNend
 */
mt_s32 MT_UNF_AI_Create(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr, mt_handle *phAI);

/**
 \brief Disable a AI port to destroy the handle. CNcomment: 关闭AI Port，销毁句柄 CNend
 \attention \n
 \param[in] hAi AI handle CNcomment: AI句柄 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \retval ::MT_ERR_AI_INVALID_PARA  The handle is incorrect. CNcomment: 句柄错误 CNend
 \see \n
N/A CNcomment: 无 CNend
 */
mt_s32 MT_UNF_AI_Destroy(mt_handle hAI);

/**
 \brief Set the attributes of a AI port. CNcomment: 设置AI Port属性 CNend
 \attention \n
 \param[in] hAi AI handle CNcomment: AI句柄 CNend
 \param[in] pstAttr  Pointer to AI attributes CNcomment: AI属性指针 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \retval ::MT_ERR_AI_NULL_PTR   The pointer is null. CNcomment: 空指针 CNend
 \retval ::MT_ERR_AI_INVALID_ID   The handle is invalid. CNcomment: 非法句柄 CNend
 \retval ::MT_ERR_AI_INVALID_PARA   The attribute parameters are incorrect. CNcomment: 属性参数错误 CNend
 \see \n
N/A CNcomment: 无 CNend
 */
mt_s32 MT_UNF_AI_SetAttr(mt_handle hAI, MT_UNF_AI_ATTR_S *pstAttr);

/**
 \brief Obtains the attributes of a AI port. CNcomment: 获取AI Port属性 CNend
 \attention \n
 \param[in] hAi AI handle CNcomment: AI句柄 CNend
 \param[out] pstAttr Pointer to AI attributes CNcomment: AI属性指针 CNend
 \retval ::MT_SUCCESS Success CNcomment: 成功 CNend
 \retval ::MT_ERR_AI_NULL_PTR   The pointer is null. CNcomment: 空指针 CNend
 \retval ::MT_ERR_AI_INVALID_ID   The handle is invalid. CNcomment: 非法句柄 CNend
 \retval ::MT_ERR_AI_INVALID_PARA   The handle is incorrect. CNcomment: 句柄错误 CNend
 \see \n
N/A CNcomment: 无 CNend
 */
mt_s32 MT_UNF_AI_GetAttr(mt_handle hAI, MT_UNF_AI_ATTR_S *pstAttr);

/**
\brief enable AI port. CNcomment: 使能AI通道 CNend
\attention \n
\param[in] AI handle CNcomment: AI句柄 CNend
\param[in] bEnable      enable  .CNcomment:使能控制 CNend
\retval ::MT_SUCCESS CNcomment: success.成功 CNend
\retval ::MT_ERR_AI_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AI_INVALID_ID     The handle is invalid. CNcomment: 非法句柄 CNend
\retval ::MT_ERR_AI_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
N/A CNcomment: 无 CNend
*/
mt_s32 MT_UNF_AI_SetEnable(mt_handle hAI, MT_BOOL bEnable);

/**
\brief Obtains enable/disable status of AI port. CNcomment: 获取AI通道时能状态 CNend
\attention \n
\param[in] AI handle CNcomment: AI句柄 CNend
\param[out] pbEnable enable/disable status AI port. CNcomment:AI通道时能状态 CNend
\retval ::MT_SUCCESS CNcomment: success.成功 CNend
\retval ::MT_ERR_AI_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AI_INVALID_ID     The handle is invalid. CNcomment: 非法句柄 CNend
\retval ::MT_ERR_AI_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
N/A CNcomment: 无 CNend
*/
mt_s32 MT_UNF_AI_GetEnable(mt_handle hAI, MT_BOOL *pbEnable);

/**
\brief set AI delay compensation. CNcomment: 设置AI缓存延迟补偿 CNend
\attention \n
\param[in] AI handle CNcomment: AI句柄 CNend
\param[in] pstDelay  delay compensation .CNcomment:延迟补偿参数 CNend
\retval ::MT_SUCCESS CNcomment: success.成功 CNend
\retval ::MT_ERR_AI_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AI_INVALID_ID     The handle is invalid. CNcomment: 非法句柄 CNend
\retval ::MT_ERR_AI_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
N/A CNcomment: 无 CNend
*/
mt_s32 MT_UNF_AI_SetDelay(mt_handle hAI, const MT_UNF_AI_DELAY_S *pstDelay);

/**
\brief Obtains AI delay compensation. CNcomment: 获取AI缓存延迟补偿 CNend
\attention \n
\param[in] AI handle CNcomment: AI句柄 CNend
\param[out] pstDelay delay compensation. CNcomment:延迟补偿参数 CNend
\retval ::MT_SUCCESS CNcomment: success.成功 CNend
\retval ::MT_ERR_AI_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AI_INVALID_ID     The handle is invalid. CNcomment: 非法句柄 CNend
\retval ::MT_ERR_AI_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
N/A CNcomment: 无 CNend
*/
mt_s32 MT_UNF_AI_GetDelay(mt_handle hAI, MT_UNF_AI_DELAY_S *pstDelay);

/**
\brief get frame buffer from AI. CNcomment: 获取声音帧存 CNend
\attention \n
Cast pcm data format  s32BitPerSample(16), u32Channels(2),bInterleaved(MT_TRUE), u32SampleRate(same as AI).
\param[in] AI handle CNcomment: AI句柄 CNend
\param[in] u32TimeoutMs     acquire timeout.CNcomment:获取超时 CNend
\param[out] pstFrame        frame info.CNcomment:帧信息 CNend
\retval ::MT_SUCCESS CNcomment: success.成功 CNend
\retval ::MT_ERR_AI_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AI_INVALID_ID     The handle is invalid. CNcomment: 非法句柄 CNend
\retval ::MT_ERR_AI_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
N/A CNcomment: 无 CNend
*/
mt_s32 MT_UNF_AI_AcquireFrame(mt_handle hAI, MT_UNF_AO_FRAMEINFO_S *pstFrame, mt_u32 u32TimeoutMs);

/**
\brief Releases the frame buffer for AI . CNcomment: 释放声音帧存 CNend
\attention \n
\param[in] AI handle CNcomment: AI句柄 CNend
\param[in] u32TimeoutMs     acquire timeout.CNcomment:释放超时 CNend
\param[out] pstFrame        frame info.CNcomment:帧信息 CNend
\retval ::MT_SUCCESS CNcomment: success.成功 CNend
\retval ::MT_ERR_AI_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AI_INVALID_ID     The handle is invalid. CNcomment: 非法句柄 CNend
\retval ::MT_ERR_AI_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
N/A CNcomment: 无 CNend
*/
mt_s32 MT_UNF_AI_ReleaseFrame(mt_handle hAI, MT_UNF_AO_FRAMEINFO_S *pstFrame);

/** @} */ /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__MT_UNF_AI_H__*/
