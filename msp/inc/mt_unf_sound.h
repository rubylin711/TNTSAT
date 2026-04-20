/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_unf_sound.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/11/25
  Last Modified :
  Description   : header file for audio and video output control
  Function List :
  History       :
  1.Date        :
  Author        : 
  Modification  : Created file
******************************************************************************/
/**
 * \file
 * \brief Describes the information about the SOUND (SND) module. CNcomment:提供SOUND的相关信息 CNend
 */

#ifndef  __MT_UNF_SND_H__
#define  __MT_UNF_SND_H__

#include "mt_unf_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/********************************Macro Definition********************************/
/** \addtogroup      SOUND */
/** @{ */  /** <!-- 【SOUND】 */

/**Maximum sound outputport*/
/**CNcomment:最大sound输出端口*/
#define MT_UNF_SND_OUTPUTPORT_MAX 16

/** @} */  /** <!-- ==== Macro Definition end ==== */


/*************************** Structure Definition ****************************/
/** \addtogroup      SOUND */
/** @{ */  /** <!--  【SOUND】 */

/**Defines the ID of the audio output (AO) device.*/
/**CNcomment:定义音频输出设备号*/
typedef enum mtUNF_SND_E
{
    MT_UNF_SND_0,           /**<AO device 0*/ /**<CNcomment:音频输出设备0 */
    MT_UNF_SND_1,           /**<AO device 1*/ /**<CNcomment:音频输出设备1 */
    MT_UNF_SND_2,           /**<AO device 2*/ /**<CNcomment:音频输出设备2 */
    MT_UNF_SND_BUTT
} MT_UNF_SND_E;

/**Audio volume attribute*/
/**CNcomment:音频音量属性*/
typedef struct mtMT_UNF_SND_GAIN_ATTR_S
{
    MT_BOOL bLinearMode; /**< gain type of volume*/ /**<CNcomment:音量模式 */
    mt_s32  s32Gain; /**<Linear gain(bLinearMode is MT_TRUE) , ranging from 0 to 100*/ /**<CNcomment:线性音量: 0~100 */
                     /**<Decibel gain(bLinearMode is MT_FALSE) , ranging from -70dB to 0dB */ /**<CNcomment: dB音量:-70~0*/
} MT_UNF_SND_GAIN_ATTR_S;

/**Audio L/R channel volume attribute*/
/**CNcomment:音频左右声道音量属性*/
typedef struct mtMT_UNF_SND_ABSGAIN_ATTR_S
{
    MT_BOOL bLinearMode; /**< gain type*/ /**<CNcomment:音量模式 */
    mt_s32  s32GainL; /**<Linear left ch gain(bLinearMode is MT_TRUE) , ranging from 0 to 100*/ /**<CNcomment:左声道线性音量: 0~100 */
                     /**<Decibel left ch gain(bLinearMode is MT_FALSE) , ranging from -81dB to 18dB */ /**<CNcomment: 左声道dB音量:-81~+18*/
    mt_s32  s32GainR; /**<Linear right ch gain(bLinearMode is MT_TRUE) , ranging from 0 to 100*/ /**<CNcomment:右声道线性音量: 0~100 */
                  /**<Decibel right ch gain(bLinearMode is MT_FALSE) , ranging from -81dB to 18dB */ /**<CNcomment: 右声道dB音量:-81~+18*/
} MT_UNF_SND_ABSGAIN_ATTR_S;

/**High Precision Gain, ranging from -81dB to 18dB, step 0.125dB*/
/**CNcomment:高精度增益，范围从-81dB到18dB，步长为0.125dB*/
typedef struct mtMT_UNF_SND_PRECIGAIN_ATTR_S
{
    mt_s32 s32IntegerGain; /**<Interger part of high preicision gain*/ /**<CNcomment: 高精度增益的整数部分*/
    mt_s32 s32DecimalGain; /**<decimal part of high preicision gain, if -0.125dB, value is -125*/ /**<CNcomment: 高精度增益的小数部分 如0.125，则值为125*/
}MT_UNF_SND_PRECIGAIN_ATTR_S;

/**Audio Track Type: Master, Slave, Virtual channel*/
/**CNcomment:音频Track类型:主通道 辅通道 虚拟通道*/
typedef enum mtMT_UNF_SND_TRACK_TYPE_E
{
    MT_UNF_SND_TRACK_TYPE_MASTER = 0,
    MT_UNF_SND_TRACK_TYPE_SLAVE,
    MT_UNF_SND_TRACK_TYPE_VIRTUAL,
    MT_UNF_SND_TRACK_TYPE_BUTT
} MT_UNF_SND_TRACK_TYPE_E;

/**Audio output attribute */
/**CNcomment:音频输出属性*/
typedef struct mtMT_UNF_AUDIOTRACK_ATTR_S
{
    MT_UNF_SND_TRACK_TYPE_E     enTrackType;        /**<Track Type*/ /**<CNcomment:Track类型*/
    mt_u32                      u32FadeinMs;        /**<Fade in time(unit:ms)*/ /**<CNcomment:淡入时间(单位: ms)*/
    mt_u32                      u32FadeoutMs;       /**<Fade out time(unit:ms)*/ /**<CNcomment:淡出时间(单位: ms)*/
    mt_u32                      u32OutputBufSize;   /**<Track output buffer size*/ /**<CNcomment:Track输出缓存大小*/
    mt_u32                      u32BufLevelMs;      /**<Output buffer data size control(ms)*/ /**<CNcomment:输出缓存中数据量控制(ms)*/
    MT_BOOL                   	b_spdif_mod;

	mt_u32  u32DebugCrcSize;
	ulong	u32DebugCrc_vir;
	phys_addr_t	u32DebugCrc_phy;
	mt_u32  tts_buffer_size;
	mt_u8*	tts_buffer_vir;
	mt_u8*	tts_buffer_phy;
	mt_u32  tts_buffer_rd;
	mt_u32  tts_buffer_wt;
	MT_BOOL tts_enable;
	/*dolby*/
	char dolby_dd_ddp;		//1-dd, 2-dd+
	char dolby_dualmono;	//0-non dualmono; 1-dualmono
	mt_u32 frame_duration_ms;
} MT_UNF_AUDIOTRACK_ATTR_S;

/**Audio outputport: DAC0,I2S0,SPDIF0,HDMI0,ARC0*/
/**CNcomment:音频输出端口:DAC0,I2S0,SPDIF0,HDMI0,ARC0*/
typedef enum mtUNF_SND_OUTPUTPORT_E
{
    MT_UNF_SND_OUTPUTPORT_DAC0 = 0,

    MT_UNF_SND_OUTPUTPORT_I2S0,

    MT_UNF_SND_OUTPUTPORT_I2S1,

    MT_UNF_SND_OUTPUTPORT_SPDIF0,

    MT_UNF_SND_OUTPUTPORT_HDMI0,

    MT_UNF_SND_OUTPUTPORT_ARC0,
	
    MT_UNF_SND_OUTPUTPORT_EXT_DAC1 = 0x50,

    MT_UNF_SND_OUTPUTPORT_EXT_DAC2,

    MT_UNF_SND_OUTPUTPORT_EXT_DAC3,

    #ifdef CONFIG_MT_AUDIO_AD
    MT_UNF_SND_OUTPUTPORT_AD = 0x60,
    #endif
    
    MT_UNF_SND_OUTPUTPORT_ALL = 0x7fff,

    MT_UNF_SND_OUTPUTPORT_BUTT,
} MT_UNF_SND_OUTPUTPORT_E;

/**Defines internal Audio DAC outport attribute */
/**CNcomment:定义内置音频DAC输出端口属性*/
typedef struct mtUNF_SND_DAC_ATTR_S
{
    mt_void* pPara;
} MT_UNF_SND_DAC_ATTR_S;


/**Defines  Audio I2S outport attribute */
/**CNcomment:定义音频I2S输出端口属性*/
typedef struct mtUNF_SND_I2S_ATTR_S
{
    MT_UNF_I2S_ATTR_S  stAttr;
}  MT_UNF_SND_I2S_ATTR_S;

/**Defines  S/PDIF outport attribute */
/**CNcomment:定义S/PDIF输出端口属性*/
typedef struct mtUNF_SND_SPDIF_ATTR_S
{
    mt_void* pPara;
}  MT_UNF_SND_SPDIF_ATTR_S;

/**Defines  HDMI Audio outport attribute */
/**CNcomment:定义HDMI音频输出端口属性*/
typedef struct mtUNF_SND_HDMI_ATTR_S
{
    mt_void* pPara;
} MT_UNF_SND_HDMI_ATTR_S;

/**Defines  HDMI ARC outport attribute */
/**CNcomment:定义HDMI音频回传通道端口属性*/
typedef struct mtUNF_SND_ARC_ATTR_S
{
    mt_void* pPara;
} MT_UNF_SND_ARC_ATTR_S;


/**Defines  Audio outport attribute */
/**CNcomment:定义音频输出端口属性*/
typedef struct mtUNF_SND_OUTPORT_S
{
    MT_UNF_SND_OUTPUTPORT_E enOutPort;
    union
    {
        MT_UNF_SND_DAC_ATTR_S stDacAttr;
        MT_UNF_SND_I2S_ATTR_S stI2sAttr;
        MT_UNF_SND_SPDIF_ATTR_S stSpdifAttr;
        MT_UNF_SND_HDMI_ATTR_S stHDMIAttr;
        MT_UNF_SND_ARC_ATTR_S stArcAttr;
    } unAttr;
} MT_UNF_SND_OUTPORT_S;

/**Defines  Audio Sound device attribute */
/**CNcomment:定义Sound设备属性*/
typedef struct mtMT_UNF_SND_ATTR_S
{
    mt_u32                    u32PortNum;  /**<Outport number attached sound*/ /**<CNcomment:绑定到Sound设备的输出端口数*/
    MT_UNF_SND_OUTPORT_S stOutport[MT_UNF_SND_OUTPUTPORT_MAX];  /**<Outports attached sound*/ /**<CNcomment:绑定到Sound设备的输出端口*/
    MT_UNF_SAMPLE_RATE_E    enSampleRate;       /**<Sound samplerate*/ /**<CNcomment:Sound设备输出采样率*/
    mt_u32              u32MasterOutputBufSize; /**<Sound master channel buffer size*/ /**<CNcomment:Sound设备主输出通道缓存大小*/
    mt_u32              u32SlaveOutputBufSize;  /**<Sound slave channel buffer size*/ /**<CNcomment:Sound设备从输出通道缓存大小*/
} MT_UNF_SND_ATTR_S;

/**define SND CAST config  struct */
/**CNcomment:定义声音共享配置结构体*/
typedef struct mtUNF_SND_CAST_ATTR_S
{
    mt_u32  u32PcmFrameMaxNum;     /**<Max frame of the PCM data at cast buffer*/ /**<CNcomment: 最大可缓存帧数*/
    mt_u32  u32PcmSamplesPerFrame; /**<Number of sample of the PCM data*/ /**<CNcomment: PCM数据采样点数量*/
} MT_UNF_SND_CAST_ATTR_S;

/**HDMI MODE:AUTO,LPCM,RAW,HBR2LBR*/
/**CNcomment:HDMI 模式:AUTO,LPCM,RAW,HBR2LBR*/
typedef enum mtMT_UNF_SND_HDMI_MODE_E
{
    MT_UNF_SND_HDMI_MODE_LPCM = 0,       /**<HDMI LCPM2.0*/ /**<CNcomment: 立体声pcm*/
    MT_UNF_SND_HDMI_MODE_RAW,            /**<HDMI Pass-through.*/ /**<CNcomment: HDMI透传*/
    MT_UNF_SND_HDMI_MODE_HBR2LBR,        /**<HDMI Pass-through force high-bitrate to low-bitrate.*/ /**<CNcomment: 蓝光次世代音频降规格输出*/
    MT_UNF_SND_HDMI_MODE_AUTO,           /**<automatically match according to the EDID of HDMI */ /**<CNcomment: 根据HDMI EDID能力自动匹配*/
    MT_UNF_SND_HDMI_MODE_FORCE_DD,		/**<HDMI output dolby digit format when dolby digit plus input*/
	MT_UNF_SND_HDMI_MODE_MAT,		    /**<HDMI output MAT format when dolby input*/
    MT_UNF_SND_HDMI_MODE_BUTT
} MT_UNF_SND_HDMI_MODE_E;

/**SPDIF MODE:LPCM,RAW*/
/**CNcomment:SPDIF 模式:LPCM,RAW*/
typedef enum mtMT_UNF_SND_SPDIF_MODE_E
{
    MT_UNF_SND_SPDIF_MODE_LPCM,           /**<SPDIF LCPM2.0*/ /**<CNcomment: 立体声pcm*/
    MT_UNF_SND_SPDIF_MODE_RAW,            /**<SPDIF Pass-through.*/ /**<CNcomment: SPDIF透传*/
    MT_UNF_SND_SPDIF_MODE_BUTT
} MT_UNF_SND_SPDIF_MODE_E;
/** @} */  /** <!-- ==== Structure Definition end ==== */


/**SPDIF Category Code Setting*/
/**CNcomment:设置SPDIF Category Code类型*/
typedef enum mtMT_UNF_SND_SPDIF_CATEGORYCODE_E
{
    MT_UNF_SND_SPDIF_CATEGORY_GENERAL = 0x00,       /**<General*/                  /**<CNcomment:通用*/
    /*broadcast reception of digitally encoded audio 
    with/without video signals*/
    MT_UNF_SND_SPDIF_CATEGORY_BROADCAST_JP = 0x10,  /**<Japan*/                    /**<CNcomment:日本*/
    MT_UNF_SND_SPDIF_CATEGORY_BROADCAST_USA,        /**<United States*/            /**<CNcomment:美国*/
    MT_UNF_SND_SPDIF_CATEGORY_BROADCAST_EU,         /**<Europe*/                   /**<CNcomment:欧洲*/
    /*digital converter & signal-processing products*/
    MT_UNF_SND_SPDIF_CATEGORY_PCM_CODEC = 0x20,     /**<PCM Encoder/Decoder*/      /**<CNcomment:PCM编解码*/
    MT_UNF_SND_SPDIF_CATEGORY_DIGITAL_SNDSAMPLER,   /**<Digital Sound Sampler*/    /**<CNcomment:数字音频采样器*/
    MT_UNF_SND_SPDIF_CATEGORY_DIGITAL_MIXER,        /**<Digital Signal Mixer*/     /**<CNcomment:数字信号混音器*/
    MT_UNF_SND_SPDIF_CATEGORY_DIGITAL_SNDPROCESSOR, /**<Digital Sound Processor*/  /**<CNcomment:数字音频处理器*/
    MT_UNF_SND_SPDIF_CATEGORY_SRC,                  /**<Sample Rate Converter*/    /**<CNcomment:采样率转换器*/
    /*laser optical products*/
    MT_UNF_SND_SPDIF_CATEGORY_MD = 0x30,            /**<MiniDisc*/                 /**<CNcomment:迷你磁光盘*/
    MT_UNF_SND_SPDIF_CATEGORY_DVD,                  /**<Digital Versatile Disc*/   /**<CNcomment:数字多功能光盘*/
    /*musical instruments, microphones and other sources 
    that create original sound*/
    MT_UNF_SND_SPDIF_CATEGORY_SYNTHESISER = 0x40,   /**<Synthesiser*/              /**<CNcomment:合成器*/
    MT_UNF_SND_SPDIF_CATEGORY_MIC,                  /**<Microphone*/               /**<CNcomment:麦克风*/
    /*magnetic tape or magnetic disc based products*/
    MT_UNF_SND_SPDIF_CATEGORY_DAT = 0x50,           /**<Digital Audio Tape*/       /**<CNcomment:数字录音带*/
    MT_UNF_SND_SPDIF_CATEGORY_DCC,                  /**<Digital Compact Cassette*/ /**<CNcomment:数字盒式磁带录音机*/
    MT_UNF_SND_SPDIF_CATEGORY_VCR,                  /**<Video Cassette Recorder*/  /**<CNcomment:盒式磁带录像机*/
    
    MT_UNF_SND_SPDIF_CATEGORY_BUTT
} MT_UNF_SND_SPDIF_CATEGORYCODE_E;



/**SPDIF SCMS Mode Setting*/
/**CNcomment:设置SPDIF SCMS模式*/
typedef enum mtMT_UNF_SND_SPDIF_SCMSMODE_E
{
    MT_UNF_SND_SPDIF_SCMSMODE_COPYALLOW,                /**<Copy Allow*/      /**<CNcomment:允许复制*/
    MT_UNF_SND_SPDIF_SCMSMODE_COPYONCE,                 /**<Copy Once*/       /**<CNcomment:可复制一次*/
    MT_UNF_SND_SPDIF_SCMSMODE_COPYNOMORE,               /**<Copy NoMore*/  	  /**<CNcomment:不可复制*/
    MT_UNF_SND_SPDIF_SCMSMODE_COPYPROHIBITED,           /**<Copy prohibited*/ /**<CNcomment:禁止复制*/
    MT_UNF_SND_SPDIF_SCMSMODE_BUTT
} MT_UNF_SND_SPDIF_SCMSMODE_E;

/**audio effect type, MT_UNF_AUDIO_EFFECT_TYPE_E keep consistent with effect type of ARM/DSP*/
/**CNcomment:音效类型，MT_UNF_AUDIO_EFFECT_TYPE_E 必须 ARM/DSP 保持唯一值 */
typedef enum
{
    MT_UNF_SND_AEF_TYPE_DOLBYDV258 = 0x000,  /**<Dolby audio effect*/ /**<CNcomment: Dolby音效*/

    MT_UNF_SND_AEF_TYPE_SRS3D = 0x010,      /**<SRS audio effect*/ /**<CNcomment: SRS音效*/

    MT_UNF_SND_AEF_TYPE_BASE = 0x080,       /**<Base audio effect*/ /**<CNcomment: 自研音效*/
} MT_UNF_SND_AEF_TYPE_E;

typedef struct mtMT_UNF_SND_FADER_ATTR_S
{
	// 1:enable, 0:disable
	u8 fader_en;
	// 1:fade in, 0:fade out
	u8 fade_in;
	//basic step for change the gain
	u8 fade_step;
	//target gain, end gain for in fade-in, start gain in fade-out
	u16 target_gain;
	//time step for change the gain
	u16 time_step;
} MT_UNF_SND_FADER_ATTR_S;

typedef struct mtMT_UNF_SND_BUF_INFO_S
{
	u32 pcm_size;
	u32 pcm_valid;
	u32 spd_size;
	u32 spd_valid;
} MT_UNF_SND_BUF_INFO_S;

/******************************* API declaration *****************************/
/** \addtogroup      SOUND */
/** @{ */  /** <!--  【SOUND】 */

/**
\brief Initializes an AO device. CNcomment:初始化音频输出设备 CNend
\attention \n
Before calling the SND module, you must call this application programming interface (API). CNcomment:调用SND模块要求首先调用该接口 CNend
\param N/A
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_Init(mt_void);

/**
\brief Deinitializes an AO device. CNcomment:去初始化音频输出设备 CNend
\attention \n
N/A
\param N/A
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_DeInit(mt_void);

/**
\brief Obtains the default configured parameters of an AO device. CNcomment:获取音频输出设备默认设置参数 CNend
\attention \n
\param[in] enSound     ID of an AO device CNcomment:音频输出设备号 CNend
\param[out] pstAttr     Audio attributes CNcomment:音频属性 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_GetDefaultOpenAttr(MT_UNF_SND_E enSound, MT_UNF_SND_ATTR_S *pstAttr);

/**
\brief Starts an AO device. CNcomment:打开音频输出设备 CNend
\attention \n
One port only can attach to one sound.
CNcomment:每个端口只能绑定一个sound CNend
\param[in] enSound     ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] pstAttr     Attribute of an AO device CNcomment:音频输出设备参数 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\retval ::MT_ERR_AO_NULL_PTR               The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_Open(MT_UNF_SND_E enSound, const MT_UNF_SND_ATTR_S *pstAttr);

/**
\brief Destroys a AO SND instance. CNcomment:销毁音频输出Sound实例 CNend
\attention \n
An instance cannot be destroyed repeatedly. CNcomment:不支持重复销毁 CNend
\param[in] enSound     ID of an AO device CNcomment:音频输出设备号 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_Close(MT_UNF_SND_E enSound);


/**
\brief turn on/off adac. CNcomment:打开或关闭ADAC CNend
\attention \n
N/A
\param[in] enSound
\param[in] bOnOff
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_SetAdacOnOff(MT_UNF_SND_E enSound, MT_BOOL bOnOff);


/**
\brief Sets the mute status of  AO ports. CNcomment:音频输出静音开关设置 CNend
\attention \n
N/A
\param[in] enSound
\param[in] enOutPort CNcomment:sound输出端口 CNend
\param[in] bMute
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_SetMute(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bMute);

/**
\brief Obtains the mute status of AO ports. CNcomment:获取音频输出的静音开关状态 CNend
\attention \n
N/A
\param[in] enSound CNcomment:
\param[in] enOutPort CNcomment:sound输出端口 CNend
\param[out] pbMute CNcomment:
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_GetMute(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbMute);

/**
\brief Sets the output mode of the HDMI(Auto/PCM/RAW/HBR2LBR).
\attention \n
\param[in] enSound CNcomment:音频输出设备号  CNend
\param[in] enOutPort  Audio OutputPort   CNcomment:音频输出端口 CNend
\param[in] enHdmiMode HDMI mode CNcomment:HDMI模式CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_SetHdmiMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_HDMI_MODE_E enHdmiMode);

/**
\brief Gets the output mode of the HDMI.
\attention \n
\param[in] enSound CNcomment:音频输出设备号  CNend
\param[in] enOutPort  Audio OutputPort   CNcomment:音频输出端口 CNend
\param[out] Pointer to the obtained HDMI mode CNcomment:获取到的HDMI模式的指针CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_GetHdmiMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_HDMI_MODE_E *penHdmiMode);

/**
\brief Sets the output mode of the SPDIF(PCM/RAW).
\attention \n
\param[in] enSound CNcomment:音频输出设备号  CNend
\param[in] enOutPort  Audio OutputPort   CNcomment:音频输出端口 CNend
\param[in] enHdmiMode SPDIF mode CNcomment:SPDIF模式CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_ID    The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_SetSpdifMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_MODE_E enSpdifMode);

/**
\brief Gets the output mode of the SPDIF.
\attention \n
\param[in] enSound CNcomment:音频输出设备号  CNend
\param[in] enOutPort  Audio OutputPort   CNcomment:音频输出端口 CNend
\param[out] penSpdifMode Pointer to the obtained SPDIF mode CNcomment:获取到的SPDIF模式的指针CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_ID    The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\retval ::MT_ERR_AO_NULL_PTR               The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_GetSpdifMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_MODE_E *penSpdifMode);


/**
 \brief Sets the output volume value. CNcomment:设置输出音量 CNend
 \attention \n
If s32Gain is set to a value greater than 100 or 0dB, then return failure. CNcomment:如果s32Gain设置大于100或0dB，返回失败 CNend
 \param[in] enSound        ID of an AO device CNcomment:音频输出设备号 CNend
 \param[in] enOutPort  Audio OutputPort     CNcomment:音频输出端口 CNend
 \param[in] pstGain     Volume value CNcomment:设置的音量值 CNend
 \retval ::MT_SUCCESS Success CNcomment:成功 CNend
 \retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
 \retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
 \retval ::MT_ERR_AO_INVALID_PARA       The parameter is invalid. CNcomment:无效的参数 CNend
 \retval ::MT_ERR_SND_INVALID_ID        The parameter enSound is invalid. CNcomment:无效Sound ID CNend
 \retval ::MT_ERR_AO_NULL_PTR               The pointer is null. CNcomment:指针参数为空 CNend
 \see \n
N/A
 */
mt_s32   MT_UNF_SND_SetVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, const MT_UNF_SND_GAIN_ATTR_S *pstGain);

/**
\brief Obtains the output volume value. CNcomment:获取输出音量 CNend
\attention \n
The default linear volume value is 100 and abslute volume is 0dB. CNcomment:查询的默认音量值为100(线性音量) and 0dB(绝对音量) CNend
\param[in] enSound         ID of an AO device CNcomment:音频输出设备号 CNend
 \param[in] enOutPort  Audio OutputPort     CNcomment:音频输出端口 CNend
 \param[out] pstGain    Pointer to the obtained volume value CNcomment:指针类型，获取到的音量值 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\retval ::MT_ERR_AO_NULL_PTR               The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_GetVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_GAIN_ATTR_S *pstGain);


/**
\brief Sets the category code of the SPDIF.
\attention \n
\param[in] enSound CNcomment:音频输出设备号  CNend
\param[in] enOutPort  Audio OutputPort   CNcomment:音频输出端口 CNend
\param[in] enSpdifCategoryCode  SPDIF category code CNcomment:SPDIF 输出设备分类码CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_ID    The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_SetSpdifCategoryCode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_CATEGORYCODE_E enSpdifCategoryCode);

/**
\brief Gets the category code of the SPDIF.
\attention \n
\param[in] enSound CNcomment:音频输出设备号  CNend
\param[in] enOutPort  Audio OutputPort   CNcomment:音频输出端口 CNend
\param[out] enSpdifCategoryCode Pointer to the obtained category code CNcomment:获取到的分类码的指针CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN      Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_ID          The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\retval ::MT_ERR_AO_NULL_PTR            The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_GetSpdifCategoryCode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_CATEGORYCODE_E *penSpdifCategoryCode);


/**
\brief Sets the SCMS mode of the SPDIF(COPYALLOW/COPYONCE/COPYPROHIBITED).
\attention \n
\param[in] enSound CNcomment:音频输出设备号  CNend
\param[in] enOutPort  Audio OutputPort   CNcomment:音频输出端口 CNend
\param[in] enSpdifSCMSMode  SPDIF SCMS mode CNcomment:SPDIF SCMS 模式CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_ID    The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_SetSpdifSCMSMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_SCMSMODE_E enSpdifSCMSMode);

/**
\brief Gets the SCMS mode of the SPDIF.
\attention \n
\param[in] enSound CNcomment:音频输出设备号  CNend
\param[in] enOutPort  Audio OutputPort   CNcomment:音频输出端口 CNend
\param[out] enSpdifSCMSMode Pointer to the obtained SPDIF SCMS mode CNcomment:获取到的SPDIF SCMS模式的指针CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN      Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_ID          The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\retval ::MT_ERR_AO_NULL_PTR            The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_GetSpdifSCMSMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_SCMSMODE_E *enSpdifSCMSMode);


/**
\brief Sets the sampling rate during audio output. CNcomment:设置音频输出时的采样率 CNend
\attention \n
At present, the sampling rate cannot be set, and it is fixed at 48 kHz. The streams that are not sampled at 48 kHz are resampled at 48 kHz.
CNcomment:目前输出采样率默认为48k(实际输出采样率为48K~192K)，支持从8K到192K码流输入，因最大支持6倍重采样，\n
因此当设定输出采样率为192K时(实际输出采样率为192K)，播小于32K的码流会出错(此时不影响其它采样率的码流切换)\n
然而当设定输出采样率为8K时(实际输出采样率为8K~192K)，此时不能通过HDMI输出小于32K的码流(HDMI不支持) CNend
\param[in] enSound          ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] enSampleRate    Audio sampling rate. For details, see the description of ::MT_UNF_SAMPLE_RATE_E. CNcomment:音频采样率。请参见::MT_UNF_SAMPLE_RATE_E CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_SetSampleRate(MT_UNF_SND_E enSound, MT_UNF_SAMPLE_RATE_E enSampleRate);

/**
\brief Obtains the sampling rate during audio output. CNcomment:获取音频输出时的采样率 CNend
\attention \n
The 48 kHz sampling rate is returned by default. CNcomment:此接口默认返回48kHz采样率 CNend
\param[in] enSound           ID of an AO device CNcomment:音频输出设备号 CNend
\param[out] penSampleRate   Pointer to the type of the audio sampling rate CNcomment:指针类型，音频采样率的类型 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_GetSampleRate(MT_UNF_SND_E enSound, MT_UNF_SAMPLE_RATE_E *penSampleRate);

/**
\brief Sets the smart volume for audio output. CNcomment:音频输出进行智能音量处理处理开关设置 CNend
\attention \n
1. The smart volume is disabled by default.\n
2. The smart volume is valid only for the master audio.\n
3. The smart volume is enabled only when the program is switched.
CNcomment:1. 默认关闭该智能音量\n
2. 智能音量仅对主音有效\n
3. 智能音量仅在切换节目时触发 CNend
\param[in] enSound     ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] enOutPort  CNcomment:sound输出端口 CNend
\param[in] bSmartVolume     Smart volume enable, MT_TRUE: enabled; MT_FALSE: disabled CNcomment:是否打开智能音量。MT_TRUE：打开；MT_FALSE：关闭 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_SetSmartVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bSmartVolume);

/**
\brief Obtains the status of the smart volume for audio output. CNcomment:获取音频输出智能音量开关状态 CNend
\attention \n
\param[in] enSound     ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] enOutPort  CNcomment:sound输出端口 CNend
\param[out] pbSmartVolume     Pointer to the enable status of the smart volume CNcomment:指针类型，是否打开智能音量 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_NULL_PTR               The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_GetSmartVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E eOutPort, MT_BOOL *pbSmartVolume);

/**
\brief Sets the smart volume for audio output. CNcomment:音频输出进行智能音量处理处理开关设置 CNend
\attention \n
1. The smart volume is disabled by default.\n
2. The smart volume is valid only for the track.\n
3. The smart volume is enabled only when the program is switched.
CNcomment:1. 默认关闭该智能音量\n
2. 智能音量仅对track有效\n
3. 智能音量仅在切换节目时触发 CNend
\param[in] hTrack           Track handle CNcomment:Track 句柄 CNend
\param[in] bSmartVolume     Smart volume enable, MT_TRUE: enabled; MT_FALSE: disabled CNcomment:是否打开智能音量。MT_TRUE：打开；MT_FALSE：关闭 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE Failure CNcomment:失败 CNend
\retval ::MT_ERR_AO_NOTSUPPORT Unsupport CNcomment:不支持 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_SetTrackSmartVolume(mt_handle hTrack, MT_BOOL bEnable);

/**
\brief Obtains the status of the smart volume for audio output. CNcomment:获取音频输出智能音量开关状态 CNend
\attention \n
\param[in] hTrack           Track handle CNcomment:Track 句柄 CNend
\param[out] pbSmartVolume     Pointer to the enable status of the smart volume CNcomment:指针类型，是否打开智能音量 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE Failure CNcomment:失败 CNend
\retval ::MT_ERR_AO_NOTSUPPORT Unsupport CNcomment:不支持 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_NULL_PTR               The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_GetTrackSmartVolume(mt_handle hTrack, MT_BOOL *pbEnable);

/**
\brief Set the AO track mode. CNcomment:设置音频输出声道模式 CNend
\attention \n
N/A
\param[in] enSound     ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] enOutPort   CNcomment:sound输出端口 CNend
\param[in] enMode     Audio track mode. For details, see the description of ::MT_UNF_TRACK_MODE_E. CNcomment:音频声道模式，请参见::MT_UNF_TRACK_MODE_E CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_SetTrackMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_TRACK_MODE_E enMode);

/**
\brief Obtains the AO track mode. CNcomment:获取音频输出声道模式 CNend
\attention \n
N/A
\param[in] enSound     ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] enOutPort   CNcomment:sound输出端口 CNend
\param[out] penMode   Pointer to the AO track mode. For details, see the description of ::MT_UNF_TRACK_MODE_E.
CNcomment:指针类型，音频声道模式。请参见::MT_UNF_TRACK_MODE_E CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA    The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_GetTrackMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_TRACK_MODE_E *penMode);

/**
\brief Attaches the SND module to an audio/video player (AVPLAY). CNcomment:绑定音频输出Sound和AV（Audio Video）播放器 CNend
\attention \n
Before calling this API, you must create a player and ensure that the player has no output. CNcomment:调用此接口前必须先创建播放器，对应这路播放器没有输出 CNend
\param[in] hTrack             Instance handle of an AVPLAY CNcomment:Track 实例句柄 CNend
\param[in] hSource           Instance handle of an AVPLAY CNcomment:AV播放器播放实例句柄 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA            The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/

mt_s32   MT_UNF_SND_Attach(mt_handle hTrack, mt_handle hSource);

/**
\brief Detaches the SND module from an AVPLAY. CNcomment:解除Track和AV播放器绑定 CNend
\attention \n
N/A
\param[in] hTrack             Instance handle of an AVPLAY CNcomment:Track 实例句柄 CNend
\param[in] hSource    Instance handle of an AVPLAY CNcomment:AV播放器播放实例句柄 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/

mt_s32 MT_UNF_SND_Detach(mt_handle hTrack, mt_handle hSource);

/**
\brief Sets the mixing weight of an audio player. CNcomment:设置音频Track 混音权重 CNend
\attention \n
The output volumes of two players are calculated as follows: (volume x weight 1 + volume x weight 2)/100. The formula of calculating the output volumes of multiple players is similar.
CNcomment:两个Track 输出音量的计算方法为：（设置的音量%权重1+设置的音量%权重2）/100，多个播放器的计算方法与此类似 CNend
\param[in] hTrack              ID of an AO device CNcomment:音频输出Track CNend
\param[in] pstMixWeightGain   the MixWeight Gain, ranging from 0 to 100. 0: minimum value; 100: maximum value CNcomment:权重，范围为0～100。0：最小值；100：最大值 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA            The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/

mt_s32  MT_UNF_SND_SetTrackWeight(mt_handle hTrack, const MT_UNF_SND_GAIN_ATTR_S *pstMixWeightGain);

/**
\brief Obtains the mixing weight of an audio player. CNcomment:获取音频播放器混音权重 CNend
\attention \n

\param[in] hTrack              ID of an AO device CNcomment:音频输出Track CNend
\param[in] pstMixWeightGain     Pointer to the MixWeight Gain CNcomment:指针类型，权重属性 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR               The pointer is null. CNcomment:指针参数为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA            The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_GetTrackWeight(mt_handle hTrack, MT_UNF_SND_GAIN_ATTR_S *pstMixWeightGain);


/**
\brief Sets the L/R channel weight of an audio player. CNcomment:设置音频Track左右声道权重 CNend
\attention \n
\param[in] hTrack              ID of an AO device CNcomment:音频输出Track CNend
\param[in] pstAbsWeightGain   the channel Weight Gain CNcomment:权重 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA            The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32  MT_UNF_SND_SetTrackAbsWeight(mt_handle hTrack, const MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain);

/**
\brief Obtains the L/R channel weight of an audio player. CNcomment:获取音频播放器左右声道权重 CNend
\attention \n
\param[in] hTrack              ID of an AO device CNcomment:音频输出Track CNend
\param[in] pstAbsWeightGain     Pointer to the ChannelWeight Gain CNcomment:指针类型，权重属性 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR               The pointer is null. CNcomment:指针参数为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA            The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/
mt_s32  MT_UNF_SND_GetTrackAbsWeight(mt_handle hTrack, MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain);

/**
\brief Sets mute or unmute of an audio player. CNcomment:设置单个音频Track 静音功能 CNend
\attention \n
\param[in] hTrack              ID of an AO device CNcomment:音频输出Track CNend
\param[in] bMute               mute or not CNcomment:静音与否 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/
mt_s32  MT_UNF_SND_SetTrackMute(mt_handle hTrack, MT_BOOL bMute);

/**
\brief Gets mute status of an audio player. CNcomment:获取单个音频Track的静音状态 CNend
\attention \n
\param[in] hTrack              ID of an AO device CNcomment:音频输出Track CNend
\param[out] pbMute               mute status CNcomment:静音状态 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA       The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32  MT_UNF_SND_GetTrackMute(mt_handle hTrack, MT_BOOL *pbMute);

/**
\brief Sets mute or unmute of all players. CNcomment:设置所有音频Track 静音功能 CNend
\attention \n
\param[in] enSound              all track of the sound CNcomment:该sound所有track CNend
\param[in] bMute               mute or not CNcomment:静音与否 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/
mt_s32  MT_UNF_SND_SetAllTrackMute(MT_UNF_SND_E enSound, MT_BOOL bMute);

/**
\brief Gets mute status of all players. CNcomment:获取所有音频Track的静音状态 CNend
\attention \n
\param[in] enSound              all track of the sound CNcomment:该sound track CNend
\param[in] pbMute               mute status CNcomment:静音状态 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA       The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32  MT_UNF_SND_GetAllTrackMute(MT_UNF_SND_E enSound, MT_BOOL *pbMute);

/**
\brief Set the track channel mode. CNcomment:设置音频Track  声道模式 CNend
\attention \n
N/A
\param[in] hTrack              ID of an AO device CNcomment:音频输出Track CNend
\param[in] enMode             The audio track mode. For details, see the description of ::MT_UNF_TRACK_MODE_E. CNcomment:音频声道模式，请参见::MT_UNF_TRACK_MODE_E CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_SetTrackChannelMode(mt_handle hTrack, MT_UNF_TRACK_MODE_E enMode);

/**
\brief Obtains the track channel mode. CNcomment:获取音频Track 声道模式 CNend
\attention \n
N/A
\param[in] hTrack              ID of an AO device CNcomment:音频输出Track CNend
\param[out] penMode   Pointer to the audio track mode. For details, see the description of ::MT_UNF_TRACK_MODE_E.
CNcomment:指针类型，音频声道模式。请参见::MT_UNF_TRACK_MODE_E CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA    The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_GetTrackChannelMode(mt_handle hTrack, MT_UNF_TRACK_MODE_E *penMode);

/**
 \brief Obtains the default configured parameters of an AO Track. CNcomment:获取音频输出Track默认设置参数 CNend
 \attention \n
 \param[in] enTrackType              Track Type CNcomment:Track类型 CNend
 \param[out] pstAttr     Audio attributes CNcomment:音频属性 CNend
 \retval ::MT_SUCCESS Success CNcomment:成功 CNend
 \retval ::MT_FAILURE  Failure  CNcomment:失败 CNend
 \retval ::MT_ERR_AO_NULL_PTR               The pointer is null. CNcomment:指针参数为空 CNend
 \see \n
N/A
 */
mt_s32   MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_E enTrackType, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr);

/**
\brief Create a Track. CNcomment:创建一路Track CNend
\attention \n
Create 8 output tracks(master/slave track) and 6 virtual tracks at the most. only create 1 master track on every sound.
CNcomment:最多可创建8路输出track(master/slave), 6路虚拟track，每个sound只能创建一路master track CNend
\param[in] enSound     ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] pTrackAttr  Track attributes CNcomment:指针类型，Track  属性 CNend
\param[out] phTrack   Pointer to the handle of the created Track CNcomment:指针类型，创建的Track 句柄 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE  Failure  CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_CreateTrack(MT_UNF_SND_E enSound,const MT_UNF_AUDIOTRACK_ATTR_S *pTrackAttr,mt_handle *phTrack);

/**
\brief Destroy a Track. CNcomment:销毁一路Track CNend
\attention \n
\param[in] hTrack   the handle of the Track CNcomment:Track 句柄 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_DestroyTrack(mt_handle hTrack);

/**
\brief set the attribute of  a track, reversed. CNcomment:设置某一路track的属性， 预留 CNend
\attention \n
\param[in] hTrack   the handle of the Track CNcomment:Track 句柄 CNend
\param[in] stTrackAttr   the attribute of the Track CNcomment:Track 属性 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_ID The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_PARA The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_SetTrackAttr(mt_handle hTrack, const MT_UNF_AUDIOTRACK_ATTR_S *pstTrackAttr);

/**
\brief get the attribute of  a track, reversed. CNcomment:获取某一路track的属性， 预留 CNend
\attention \n
1.virtual track Attr not support set.\n
2.pstTrackAttr struct: just enTrackType&u32BufLevelMs can be set ,other members not support.
CNcomment:1.虚拟track不支持设置属性\n
2.pstTrackAttr结构体中仅enTrackType&u32BufLevelMs可以设置，其他成员不支持 CNend
\param[in] hTrack   the handle of the Track CNcomment:Track 句柄 CNend
\param[out] pstTrackAttr   the attribute of the Track CNcomment:Track 属性 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_ID The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_PARA The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_GetTrackAttr(mt_handle hTrack, MT_UNF_AUDIOTRACK_ATTR_S *pstTrackAttr);

/**
\brief Acquire the audio frame from the track. CNcomment:获取某一路track的音频帧 CNend
\attention \n
\param[in] hTrack   the handle of the Track CNcomment:Track 句柄 CNend
\param[in] u32TimeoutMs         acquire timeout.CNcomment:获取超时 CNend
\param[out] pstAOFrame the audio frame  CNcomment:Track 音频帧 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_INVALID_ID The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_PARA   The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_AcquireTrackFrame(mt_handle hTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame, mt_u32 u32TimeoutMs);

/**
\brief Release the audio frame . CNcomment:释放track 音频帧 CNend
\attention \n
\param[in] hTrack   the handle of the Track CNcomment:Track 句柄 CNend
\param[in] pstAOFrame the audio frame  CNcomment:Track 音频帧 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_INVALID_ID  The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_PARA    The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32  MT_UNF_SND_ReleaseTrackFrame(mt_handle hTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame);

/**
\brief  Transmits data to a slave track directly. CNcomment:直接将数据送到slave track CNend
\attention \n
1. If MT_ERR_AO_OUT_BUF_FULL is returned, you need to transmit the data that fails to be transmitted last time to ensure the audio continuity.
2. For the PCM data, the restrictions are as follows:
    s32BitPerSample must be set to 16.
    bInterleaved must be set to MT_TRUE. Only interlaced mode is supported.
    u32Channels can be set to 1 or 2.
    u32PtsMs can be ignored.
    ps32PcmBuffer indicates the PCM data pointer.
    ps32BitsBuffer can be ignored.
    u32PcmSamplesPerFrame indicates the number of audio sampling. The data length (in byte) is calculated as follows: u32PcmSamplesPerFrame x u32Channels x sizeof(MT_u16)
    u32BitsBytesPerFrame can be ignored.
    u32FrameIndex can be ignored.
CNcomment:1 如果返回MT_ERR_AO_OUT_BUF_FULL，需要调度者继续送上次失败数据，才能保证声音的连续
2 PCM 数据格式在混音器的限制如下
    s32BitPerSample: 必须为16
    bInterleaved: 必须为MT_TRUE, 仅支持交织模式
    u32Channels: 1 或2
    u32PtsMs: 忽略该参数
    ps32PcmBuffer: PCM 数据指针
    ps32BitsBuffer: 忽略该参数
    u32PcmSamplesPerFrame: 音频样点数, 数据长度(unit:Bytes): u32PcmSamplesPerFrame*u32Channels*sizeof(MT_u16)
    u32BitsBytesPerFrame: 忽略该参数
    u32FrameIndex: 忽略该参数 CNend
\param[in] hTrack   Track handle CNcomment:Track 句柄 CNend
\param[in] pstAOFrame   Information about the audio data CNcomment:音频数据信息 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_NULL_PTR    The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_PARA    The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_OUT_BUF_FULL  Data fails to be transmitted because the mixer buffer is full. CNcomment:混音缓冲区数据满，送数据失败 CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_SendTrackData(mt_handle hTrack, const MT_UNF_AO_FRAMEINFO_S *pstAOFrame);

/**
\brief Obtains the delay ms of track. CNcomment:获取音频Track延时时间 CNend
\attention \n
\param[in] hTrack     ID of ao track CNcomment:音频Track ID CNend
\param[out] pDelayMs     DelayMs attributes CNcomment:Track延时时间 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_GetTrackDelayMs(const mt_handle hTrack, mt_u32 *pDelayMs);

/**
\brief Sets the output high precision volume value. CNcomment:设置输出的高精度音量 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] enSound        ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] enOutPort  Audio OutputPort     CNcomment:音频输出端口 CNend
\param[in] pstPreciGain     high precision volume value CNcomment:设置的高精度音量值 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA     The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_SND_INVALID_ID     The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\retval ::MT_ERR_AO_NULL_PTR               The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_SetPrecisionVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, const MT_UNF_SND_PRECIGAIN_ATTR_S *pstPreciGain);

/** 
\brief Obtains the output high precision volume value. CNcomment:获取输出的高精度音量 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] enSound         ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] enOutPort  Audio OutputPort     CNcomment:音频输出端口 CNend
\param[out] pstPreciGain   Pointer to the obtained high precision volume value CNcomment:指针类型，获取到的高精度音量值 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA     The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\retval ::MT_ERR_AO_NULL_PTR               The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_GetPrecisionVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_PRECIGAIN_ATTR_S *pstPreciGain);

/**
\brief Sets the output balance. CNcomment:设置输出的平衡 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] enSound        ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] enOutPort  Audio OutputPort     CNcomment:音频输出端口 CNend
\param[in] s32Balance     balance value CNcomment:设置的平衡值 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA     The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_SND_INVALID_ID     The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_SetBalance(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, mt_s32 s32Balance);

/** 
\brief Obtains balance value. CNcomment:获取输出的平衡值 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] enSound         ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] enOutPort  Audio OutputPort     CNcomment:音频输出端口 CNend
\param[out] ps32Balance   Pointer to the obtained balance value CNcomment:指针类型，获取到的平衡值 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA     The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\retval ::MT_ERR_AO_NULL_PTR               The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_GetBalance(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, mt_s32 *ps32Balance);

/**
\brief create screen share channel. CNcomment: 获取共享通道设置默认属性 CNend
\attention \n
none. CNcomment:无
\param[in] enSound      display channel.CNcomment:播放通路 CNend
\param[out] pstAttr      handle of default attr  .CNcomment:设置默认属性句柄 CNend
\retval ::MT_SUCCESS CNcomment: success.成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/
mt_s32   MT_UNF_SND_GetDefaultCastAttr(MT_UNF_SND_E enSound, MT_UNF_SND_CAST_ATTR_S *pstAttr);

/**
\brief create screen share channel. CNcomment: 创建声音共享通道 CNend
\attention \n
none. CNcomment:无
\param[in] enSound      display channel.CNcomment:播放通路 CNend
\param[in] pstAttr      pointer of parameter .CNcomment:指针,属性参数 CNend
\param[out] phCast      handle of screen share .CNcomment:声音共享句柄 CNend
\retval ::MT_SUCCESS CNcomment: success.成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/
mt_s32 MT_UNF_SND_CreateCast(MT_UNF_SND_E enSound, MT_UNF_SND_CAST_ATTR_S *pstAttr, mt_handle *phCast);

/**
\brief destroy screen share channel. CNcomment: 销毁声音共享通道 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] phCast      handle of screen share .CNcomment:声音共享句柄 CNend
\retval ::MT_SUCCESS CNcomment: success.成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/
mt_s32 MT_UNF_SND_DestroyCast(mt_handle hCast);

/**
\brief enable screen share. CNcomment: 使能声音共享功能 CNend
\attention \n
none. CNcomment:无
\param[in] phCast      handle of screen share .CNcomment:声音共享句柄
\param[in] bEnable      enable screen share .CNcomment:使能声音共享 CNend
\retval ::MT_SUCCESS CNcomment: success.成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/
mt_s32 MT_UNF_SND_SetCastEnable(mt_handle hCast, MT_BOOL bEnable);

/**
\brief get enable flag of screen share. CNcomment: 获取声音共享是否使能 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] phCast      handle of screen share .CNcomment:声音共享句柄 CNend
\param[out] bEnable     flag .CNcomment:标志 CNend
\retval ::MT_SUCCESS CNcomment: success.成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/
mt_s32 MT_UNF_SND_GetCastEnable(mt_handle hCast, MT_BOOL *pbEnable);

/**
\brief get frame info of snd cast. CNcomment: 获取声音共享帧信息 CNend
\attention \n
Cast pcm data format  s32BitPerSample(16), u32Channels(2),bInterleaved(MT_TRUE), u32SampleRate(same as SND).
\param[in] hCast      handle of screen share .CNcomment:声音共享句柄 CNend
\param[out] pstCastFrame        frame info.CNcomment:帧信息 CNend
\param[in] u32TimeoutMs         acquire timeout.CNcomment:获取超时 CNend
\retval ::MT_SUCCESS CNcomment: success.成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_V_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AO_CAST_TIMEOUT   no enough data.CNcomment:数据不够 CNend
\see \n
none.CNcomment:无 CNend
*/
mt_s32 MT_UNF_SND_AcquireCastFrame(mt_handle hCast, MT_UNF_AO_FRAMEINFO_S *pstCastFrame, mt_u32 u32TimeoutMs);

/**
\brief release frame info of screen share. CNcomment: 释放声音共享帧信息 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] hCast      handle of screen share .CNcomment:声音共享句柄 CNend
\param[in] pstCastFrame     frame info.CNcomment:帧信息 CNend
\param[in] u32TimeoutMs    release timeout.CNcomment:释放超时 CNend
\retval ::MT_SUCCESS CNcomment: success.成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/
mt_s32 MT_UNF_SND_ReleaseCastFrame(mt_handle hCast, MT_UNF_AO_FRAMEINFO_S *pstCastFrame);

/**
\brief Sets the L/R channel weight of snd cast. CNcomment:设置音频Cast左右声道权重 CNend
\attention \n
\param[in] hCast              ID of snd cast CNcomment:音频Cast ID CNend
\param[in] pstAbsWeightGain   the channel Weight Gain, ranging from 0 to 100. 0: minimum value; 100: maximum value CNcomment:权重，范围为0～100。0：最小值；100：最大值 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA            The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_SND_SetCastAbsWeight(mt_handle hCast, const MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain);

/**
\brief Obtains the L/R channel weight of audio cast. CNcomment:获取音频Cast左右声道权重 CNend
\attention \n

\param[in] hCast              ID of snd cast CNcomment:音频Cast ID CNend
\param[out] pstAbsWeightGain     Pointer to the ChannelWeight Gain CNcomment:指针类型，权重属性 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR               The pointer is null. CNcomment:指针参数为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA            The parameter is invalid. CNcomment:无效的参数 CNend
\see \n
N/A
*/
mt_s32  MT_UNF_SND_GetCastAbsWeight(mt_handle hCast,  MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain);

/**
\brief Sets the mute status of audio cast. CNcomment:设置音频Cast静音状态 CNend
\attention \n
N/A
\param[in] enSound
\param[in] enOutPort CNcomment:sound输出端口 CNend
\param[in] bMute
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_SetCastMute(mt_handle hCast, MT_BOOL bMute);

/**
\brief Obtains the mute status of audio cast. CNcomment:获取音频Cast静音状态 CNend
\attention \n
N/A
\param[in] enSound CNcomment:
\param[in] enOutPort CNcomment:sound输出端口 CNend
\param[out] pbMute CNcomment:
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE FAILURE CNcomment:失败 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR           The pointer is null. CNcomment:指针参数为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA        The parameter is invalid. CNcomment:无效的参数 CNend
\retval ::MT_ERR_AO_INVALID_ID      The parameter enSound is invalid. CNcomment:无效Sound ID CNend
\see \n
N/A
*/
mt_s32   MT_UNF_SND_GetCastMute(mt_handle hCast, MT_BOOL *pbMute);

/**
\brief enable/disable audio effect for output port. CNcomment: 使能输出端口的音频音效 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] enSound    ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] enOutPort  output port .CNcomment:输出端口号 CNend
\param[in] bBypass    enable/disable .CNcomment:使能标记 CNend
\retval ::MT_SUCCESS  success. CNcomment: 成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA      invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/
mt_s32 MT_UNF_SND_SetAefBypass(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bBypass);

/**
\brief get info of audio effect for output port. CNcomment: 获取输出端口的音频音效使能信息 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] enSound    ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] enOutPort  output port .CNcomment:输出端口号 CNend
\param[in] pbBypass   enable/disable .CNcomment:使能标记指针 CNend
\retval ::MT_SUCCESS  success. CNcomment: 成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA      invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/
mt_s32 MT_UNF_SND_GetAefBypass(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbBypass);
 
/**
\brief Registers Audio effect authorize library . CNcomment: 注册音频音效授权库 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] pAefLibFileName  file name of authorize library .CNcomment:授权库文件名 CNend
\retval ::MT_SUCCESS    success. CNcomment:成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA      invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/   
mt_s32 MT_UNF_SND_RegisterAefAuthLib(const mt_char *pAefLibFileName);

/**
\brief Create audio effect . CNcomment: 根据类型创建一路音效处理 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] enSound    ID of an AO device CNcomment:音频输出设备号 CNend
\param[in] enAefType  audio effect type .CNcomment:音效类型 CNend
\param[in] pstAdvAttr audio effect attribute .CNcomment:音效创建参数 CNend
\param[out] phAef     the point of audio effect handle  .CNcomment:音效句柄指针 CNend
\retval ::MT_SUCCESS   success. CNcomment: 成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA      invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/   
mt_s32 MT_UNF_SND_CreateAef(MT_UNF_SND_E enSound, MT_UNF_SND_AEF_TYPE_E enAefType, mt_void *pstAdvAttr, mt_handle *phAef);

/**
\brief Destroy audio effect . CNcomment: 销毁一路音效 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] phAef  audio effect handle .CNcomment:音效句柄 CNend
\retval ::MT_SUCCESS success. CNcomment: 成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA      invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/   
mt_s32 MT_UNF_SND_DestroyAef(mt_handle hAef);

/**
\brief start/stop audio effect postprocess. CNcomment: 启动/停止音效处理 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] phAef   audio effect handle .CNcomment:音效句柄 CNend
\param[in] bEnable start/stop .CNcomment:音效启动/停止 CNend
\retval ::MT_SUCCESS  success. CNcomment: 成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_INVALID_PARA      invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/ 
mt_s32 MT_UNF_SND_SetAefEnable(mt_handle hAef, MT_BOOL bEnable);

/**
\brief Set audio effect params. CNcomment: 设置音效静态参数 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] phAef  audio effect handle .CNcomment:音效句柄 CNend
\param[in] u32ParamType parameter type .CNcomment:参数类型 CNend
\param[in] pstParms     point of parameter .CNcomment:参数指针 CNend
\retval ::MT_SUCCESSsuccess. CNcomment: 成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA      invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/
mt_s32 MT_UNF_SND_SetAefParams(mt_handle hAef, mt_u32 u32ParamType, const mt_void *pstParms);

/**
\brief Get audio effect params. CNcomment: 获取音效静态参数 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] phAef    audio effect handle .CNcomment:音效句柄 CNend
\param[in] u32ParamType  parameter type .CNcomment:参数类型 CNend
\param[out] pstParms      point of parameter .CNcomment:参数指针 CNend
\retval ::MT_SUCCESS     success. CNcomment: 成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA      invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/
mt_s32 MT_UNF_SND_GetAefParams(mt_handle hAef, mt_u32 u32ParamType, mt_void *pstParms);

/**
\brief Set audio effect config. CNcomment: 设置音效动态配置 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] phAef  audio effect handle .CNcomment:音效句柄 CNend
\param[in] u32CfgType   config type .CNcomment:配置类型 CNend
\param[in] pstConfig    point of config .CNcomment:配置指针 CNend
\retval ::MT_SUCCESS    success. CNcomment: 成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA      invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/
mt_s32 MT_UNF_SND_SetAefConfig(mt_handle hAef, mt_u32 u32CfgType, const mt_void *pstConfig);

/**
\brief Get audio effect config. CNcomment: 获取音效动态配置 CNend
\attention \n
none. CNcomment:无 CNend
\param[in] phAef   audio effect handle .CNcomment:音效句柄 CNend
\param[in] u32CfgType  config type .CNcomment:配置类型 CNend
\param[out] pstConfig   point of config .CNcomment:配置指针 CNend
\retval ::MT_SUCCESS   success. CNcomment: 成功 CNend
\retval ::MT_ERR_AO_SOUND_NOT_OPEN    Sound device is not opened. CNcomment:Sound设备未打开 CNend
\retval ::MT_ERR_AO_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AO_INVALID_PARA      invalid input parameter.CNcomment:输入参数非法 CNend
\see \n
none.CNcomment:无 CNend
*/
mt_s32 MT_UNF_SND_GetAefConfig(mt_handle hAef, mt_u32 u32CfgType, mt_void *pstConfig);

mt_s32 MT_UNF_SND_SetFaderAttr(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_FADER_ATTR_S *pstFader);

mt_s32 MT_UNF_SND_GetBufInfo(MT_UNF_SND_E enSound, MT_UNF_SND_BUF_INFO_S *pstBufInfo);


mt_s32  MT_UNF_SND_StartTTS(MT_UNF_SND_E enSound);
mt_s32  MT_UNF_SND_SendTTS(MT_UNF_SND_E enSound, mt_u8 *data, mt_u32 size);
mt_s32  MT_UNF_SND_StopTTS(MT_UNF_SND_E enSound);

/** @} */  /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif  /*__MT_UNF_SND_H__*/

