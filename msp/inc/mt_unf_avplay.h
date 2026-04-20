/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_unf_avplay.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      :
    Modification:

*******************************************************************************/
/**
 * \file
 * \brief Describes the information about the audio/video player (AVPLAY) module.
          CNcomment:提供AVPLAY的相关信息 CNend
 */
#ifndef __MT_UNF_AVPLAY_H__
#define __MT_UNF_AVPLAY_H__

#include "mt_unf_common.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif

/*********************************error  macro******************************************/
/*************************** Structure Definition ****************************/
/** \addtogroup      AVPLAY */
/** @{ */  /** <!-- [AVPLAY] */

/**Highest priority of the video decoder*/
/**CNcomment:视频解码器的最大优先级 */
#define MT_UNF_VCODEC_MAX_PRIORITY  16


/**Defines the buffer ID required in an AVPLAY.*/
/**CNcomment:定义AV播放器中需要的BufferID枚举类型 */
typedef enum mtUNF_AVPLAY_BUFID_E
{
    MT_UNF_AVPLAY_BUF_ID_ES_VID  = 0,    /**<ID of the buffer for storing the video elementary streams (ESs) played by an AVPLAY*//**<CNcomment: AV播放器ES视频Buffer ID. */
    MT_UNF_AVPLAY_BUF_ID_ES_AUD  = 1,    /**<ID of the buffer for storing the audio ESs played by an AVPLAY*//**<CNcommentAV播放器ES音频Buffer ID.*/
    MT_UNF_AVPLAY_BUF_ID_BUTT
} MT_UNF_AVPLAY_BUFID_E;

/**Defines the type of a media channel.*/
/**CNcomment:定义媒体通道类型. */
typedef enum mtUNF_AVPLAY_MEDIA_CHAN
{
    MT_UNF_AVPLAY_MEDIA_CHAN_AUD  = 0x01,  /**<Audio channel*//**<CNcomment: 音频通道*/
    MT_UNF_AVPLAY_MEDIA_CHAN_VID  = 0x02,  /**<Video channel*//**<CNcomment: 视频通道*/
    MT_UNF_AVPLAY_MEDIA_CHAN_ADMX = 0x04,  /**<Audio demux: used for operate demux and decoder seprartely*//**<CNcomment: 音频解复用通道*/
    MT_UNF_AVPLAY_MEDIA_CHAN_VDMX = 0x08,  /**<Video demux: used for operate demux and decoder seprartely*//**<CNcomment: 视频解复用通道*/
    MT_UNF_AVPLAY_MEDIA_CHAN_ADEC = 0x10,  /**<Audio decoder: used for operate demux and decoder seprartely*//**<CNcomment: 音频解码器通道*/
    MT_UNF_AVPLAY_MEDIA_CHAN_VDEC = 0x20,  /**<Video decoder: used for operate demux and decoder seprartely*//**<CNcomment: 视频解码器通道*/
    MT_UNF_AVPLAY_MEDIA_CHAN_BUTT
} MT_UNF_AVPLAY_MEDIA_CHAN_E;

/**Defines the type of a media channel.*/
/**CNcomment:定义媒体通道类型. */
typedef enum mtUNF_AVPLAY_MEDIA_BUF
{
    MT_UNF_AVPLAY_MEDIA_BUF_IN  = 0x01,  /*<IN buffer*/
    MT_UNF_AVPLAY_MEDIA_BUF_OUT  = 0x02,  /*<OUT buffer*/

    MT_UNF_AVPLAY_MEDIA_BUF_BUTT = 0x8
} MT_UNF_AVPLAY_MEDIA_BUF_E;


/**Defines the type of an input stream interface.*/
/**CNcomment:定义数据输入流接口类型 */
typedef enum mtUNF_AVPLAY_STREAM_TYPE_E
{
    MT_UNF_AVPLAY_STREAM_TYPE_TS = 0,   /**<Transport stream (TS)*//**<CNcomment:TS码流 */
    MT_UNF_AVPLAY_STREAM_TYPE_ES,       /**<ES stream*//**<CNcomment:ES码流 */

    MT_UNF_AVPLAY_STREAM_TYPE_BUTT
} MT_UNF_AVPLAY_STREAM_TYPE_E;

/**Defines the CRC buffer.*/
/**CNcomment:定义解码器CRC缓冲区的结构体 */
typedef struct mtUNF_AVPLAY_CRC_BUF_S
{
	mt_u32						u32Size;		/**<CRC buffer size*//**<CNcomment:CRC缓冲区大小 */
	phys_addr_t					u32PhyAddr;		/**<CRC buffer physical address*//**<CNcomment:CRC缓冲区物理地址 */
	ulong						u32UsrVirAddr;  /**<CRC buffer userspace virtual address*//**<CNcomment:CRC缓冲区物理地址 */
} MT_UNF_AVPLAY_CRC_BUF_S;

/**Defines the debug information of an AVPLAY.*/
/**CNcomment:定义AV播放调试信息 */
typedef struct mtUNF_AVPLAY_DEBUG_INFO_S
{
	MT_UNF_AVPLAY_CRC_BUF_S		stCrcBuf[MT_UNF_AVPLAY_BUF_ID_BUTT];	/**<Status of the media CRC debug buffer*//**<CNcomment:媒体CRC调试缓冲状态 */

} MT_UNF_AVPLAY_DEBUG_INFO_S;

/**Defines the stream attributes.*/
/**CNcomment:定义码流属性的结构体 */
typedef struct mtUNF_AVPLAY_STREAM_ATTR_S
{
    MT_UNF_AVPLAY_STREAM_TYPE_E enStreamType;   /**<Stream type*//**<CNcomment:码流类型 */

    mt_u32                      u32VidBufSize;  /**<Video buffer size*//**<CNcomment: 视频缓冲大小 */
    mt_u32                      u32AudBufSize;  /**<Audio buffer size*//**<CNcomment: 音频缓冲大小 */

    mt_u32						AudErrorCheckContinuationNum;

	mt_u32						u32DebugAudCrcBufSize;	/**<Debug for Audio CRC buffer size*//**<CNcomment: 调试用音频CRC缓冲大小 */
    mt_u32                      u32MultiAudBufSize;  /**<Audio buffer size*//**<CNcomment: 多音轨音频缓冲大小 */
	mt_u32       vdec_pip_chan;

} MT_UNF_AVPLAY_STREAM_ATTR_S;

/**Supported synchronization control mode*/
/**CNcomment:支持的同步控制模式 */
typedef enum mtUNF_SYNC_REF_E
{
    MT_UNF_SYNC_REF_NONE = 0, /**<Free playing without synchronization*//**<CNcomment: 自由播放 */
    MT_UNF_SYNC_REF_AUDIO,    /**<Audio-based synchronization*//**<CNcomment: 以音频为准 */
    MT_UNF_SYNC_REF_VIDEO,    /**<Video-based synchronization*//**<CNcomment: 以视频为准 */
    MT_UNF_SYNC_REF_PCR,      /**<Program Clock Reference (PCR)-based synchronization*//**<CNcomment: 以PCR（Program Clock Reference）为准 */
    MT_UNF_SYNC_REF_SCR,      /**<System Clock Reference (SCR)-based synchronization*//**<CNcomment: 以SCR (System Clock Reference) 为准 */

    MT_UNF_AVPLAY_SYNC_REF_BUTT
} MT_UNF_SYNC_REF_E;

/**Defines the status of a buffer.*/
/**CNcomment:定义使用的缓冲区状态枚举类型 */
typedef enum mtUNF_AVPLAY_BUF_STATE_E
{
    MT_UNF_AVPLAY_BUF_STATE_EMPTY = 0,   /**<The buffer is idle.*//**<CNcomment: 缓冲区空闲 */
    MT_UNF_AVPLAY_BUF_STATE_LOW,         /**<The buffer usage is too low.*//**<CNcomment: 缓冲区占用率过低 */
    MT_UNF_AVPLAY_BUF_STATE_NORMAL,      /**<The buffer works normally.*//**<CNcomment: 缓冲区使用正常 */
    MT_UNF_AVPLAY_BUF_STATE_HIGH,        /**<The buffer usage is too high.*//**<CNcomment: 缓冲区占用率过高 */
    MT_UNF_AVPLAY_BUF_STATE_FULL,        /**<The buffer is full.*//**<CNcomment: 缓冲区已满 */

    MT_UNF_AVPLAY_BUF_STATE_BUTT
}MT_UNF_AVPLAY_BUF_STATE_E;

/**Defines the type of pts channel.*/
/** CNcomment:定义PTS通道类型 */
typedef enum mtUNF_SYNC_PTS_CHAN_E
{
    MT_UNF_SYNC_PTS_CHAN_VID,   /**<Video pts channel.*//**<CNcomment:视频PTS通道 */
    MT_UNF_SYNC_PTS_CHAN_AUD,   /**<Audio pts channel.*//**<CNcomment:音频PTS通道 */
    MT_UNF_SYNC_PTS_CHAN_PCR,   /**<Pcr channel.*//**<CNcomment:PCR通道 */

    MT_UNF_SYNC_PTS_CHAN_BUTT
}MT_UNF_SYNC_PTS_CHAN_E;

/**Defines the parameters of pts jump.*/
/** CNcomment:定义PTS跳变参数的结构体 */
typedef struct mtUNF_SYNC_PTSJUMP_PARAM_S
{
    MT_UNF_SYNC_PTS_CHAN_E  enPtsChan;  /**<Pts channel.*//**<CNcomment:PTS通道 */
    MT_BOOL                 bLoopback;  /**<Loopback or not.*//**<CNcomment:是否环回 */
    mt_u32                  u32FirstPts;/**<The first pts.*//**<CNcomment:第一个PTS */
    mt_u32                  u32FirstValidPts;/**<The first valid pts.*//**<CNcomment:第一个有效PTS */
    mt_u32                  u32CurSrcPts;   /**<The current src pts.*//**<CNcomment:当前原始PTS */
    mt_u32                  u32CurPts;      /**<The current pts.*//**<CNcomment:当前PTS */
    mt_u32                  u32LastSrcPts;  /**<The last src pts.*//**<CNcomment:上一个原始PTS */
    mt_u32                  u32LastPts;     /**<The last pts.*//**<CNcomment:上一个PTS */
}MT_UNF_SYNC_PTSJUMP_PARAM_S;

/**Defines the parameters of synchronization status change*/
/** CNcomment:定义同步状态变更参数的结构体 */
typedef struct mtUNF_SYNC_STAT_PARAM_S
{
     mt_s32          s32VidAudDiff;     /**<The diffrence between video and audio frames*//**<CNcomment: 音视频差值 */
     mt_s32          s32VidPcrDiff;     /**<The diffrence between video frame and pcr*//**<CNcomment: 视频PCR差值 */
     mt_s32          s32AudPcrDiff;     /**<The diffrence between audio frame and pcr*//**<CNcomment: 音频PCR差值 */
     mt_u32          u32VidLocalTime;   /**<Local video synchronization reference time*//**<CNcomment: 视频本地时间 */
     mt_u32          u32AudLocalTime;   /**<Local audio synchronization reference time*//**<CNcomment: 音频本地时间 */
     mt_u32          u32PcrLocalTime;   /**<Local pcr synchronization reference time*//**<CNcomment: PCR本地时间 */
}MT_UNF_SYNC_STAT_PARAM_S;

/*Type of the event callback function*/
/**CNcomment: 事件回调函数类型 */
typedef enum mtUNF_AVPLAY_EVENT_E
{
    MT_UNF_AVPLAY_EVENT_EOS,                   /**<The end of stream (EOS) operation is performed, NULL*//**<CNcomment: EOS执行结束, NULL.*/
    MT_UNF_AVPLAY_EVENT_STOP,                  /**<The stop operation is performed, NULL*//**<CNcomment: STOP执行结束, NULL.*/
    MT_UNF_AVPLAY_EVENT_RNG_BUF_STATE,         /**<Status change of the media buffer queue, MT_UNF_AVPLAY_BUF_STATE_E*//**<CNcomment: 媒体缓存队列状态变化, MT_UNF_AVPLAY_BUF_STATE_E.*/
    MT_UNF_AVPLAY_EVENT_NORM_SWITCH,           /**<Standard switch, MT_UNF_NORMCHANGE_PARAM_S*//**<CNcomment: 制式切换, MT_UNF_NORMCHANGE_PARAM_S .*/
    MT_UNF_AVPLAY_EVENT_FRAMEPACKING_CHANGE,   /**<*3D Frame packing change,MT_UNF_VIDEO_FRAME_PACKING_TYPE_E*//**<CNcomment: 3D帧类型变化, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E .*/
    MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME,         /**<New video frame, MT_UNF_VO_FRAMEINFO_S*//**<CNcomment: 新视频帧, MT_UNF_VO_FRAMEINFO_S .*/
    MT_UNF_AVPLAY_EVENT_NEW_AUD_FRAME,         /**<New audio frame, MT_UNF_AO_FRAMEINFO_S*//**<CNcomment: 新音频帧, MT_UNF_AO_FRAMEINFO_S .*/
    MT_UNF_AVPLAY_EVENT_NEW_USER_DATA,         /**<New video user data, MT_UNF_VIDEO_USERDATA_S*//**<CNcomment: 新视频用户数据, MT_UNF_VIDEO_USERDATA_S .*/
    MT_UNF_AVPLAY_EVENT_GET_AUD_ES,            /**<New audio ES data, MT_UNF_ES_BUF_S*//**<CNcomment: 新音频ES数据, MT_UNF_ES_BUF_S .*/
    MT_UNF_AVPLAY_EVENT_IFRAME_ERR,            /**<I frame decode error*//**<CNcomment: 解码I帧错误 .*/
    MT_UNF_AVPLAY_EVENT_SYNC_PTS_JUMP,         /**<Pts Jump, MT_UNF_SYNC_PTSJUMP_PARAM_S * *//**<CNcomment: PTS跳变, MT_UNF_SYNC_PTSJUMP_PARAM_S * .*/
    MT_UNF_AVPLAY_EVENT_SYNC_STAT_CHANGE,      /**<Synchronization status change, MT_UNF_SYNC_STAT_PARAM_S * *//**<CNcomment: 同步状态变更, MT_UNF_SYNC_STAT_PARAM_S * .*/
    MT_UNF_AVPLAY_EVENT_VID_BUF_STATE,         /**<Status change of the media buffer queue, MT_UNF_AVPLAY_EVENT_VID_BUF_STATE*//**<CNcomment: 视频缓存队列状态变化, MT_UNF_AVPLAY_EVENT_VID_BUF_STATE */
    MT_UNF_AVPLAY_EVENT_AUD_BUF_STATE,         /**<Status change of the media buffer queue, MT_UNF_AVPLAY_EVENT_AUD_BUF_STATE*//**<CNcomment: 音频缓存队列状态变化, MT_UNF_AVPLAY_EVENT_AUD_BUF_STATE */
    MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT,         /**<The video stream is unsupport*//**<CNcomment: 视频码流不支持*/
    MT_UNF_AVPLAY_EVENT_VID_ERR_RATIO,         /**< frame error ratio *//**<CNcomment: 图像帧出错比例*/
	MT_UNF_AVPLAY_EVENT_AUD_INFO_CHANGE,	   /**< audio info change, MT_UNF_ACODEC_STREAMINFO_S *//**<CNcomment: 音频信息变化，MT_UNF_ACODEC_STREAMINFO_S*/
	MT_UNF_AVPLAY_EVENT_AUD_UNSUPPORT,		   /**< unsupported audio *//**<CNcomment: 不支持的音频*/
	MT_UNF_AVPLAY_EVENT_AUD_FRAME_ERR,		   /**< audio frame error *//**<CNcomment: 音频帧出错*/
    MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME,      /**<first video frame, MT_UNF_VO_FRAMEINFO_S*//**<CNcomment:First 视频帧, MT_UNF_VO_FRAMEINFO_S .*/
    MT_UNF_AVPLAY_EVENT_SEEK_COMPLETED,
    MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED,
	MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_SHOWED,
	MT_UNF_AVPLAY_EVENT_SL_HDR_EN_CHANGE,
	MT_UNF_AVPLAY_EVENT_BUTT
} MT_UNF_AVPLAY_EVENT_E;

/**Mode of processing the buffer overflow*/
/**CNcomment: 缓冲溢出处理类型  */
typedef enum mtUNF_AVPLAY_OVERFLOW_E
{
    MT_UNF_AVPLAY_OVERFLOW_RESET,              /**<Reset during overflow*//**<CNcomment: 溢出时进行复位  */
    MT_UNF_AVPLAY_OVERFLOW_DISCARD,            /**<Discard during overflow*//**<CNcomment: 溢出时进行丢弃  */
    MT_UNF_AVPLAY_OVERFLOW_BUTT
} MT_UNF_AVPLAY_OVERFLOW_E;

/**Defines the type of the event callback function.*/
/**CNcomment: 定义事件回调函数枚举类型 */
typedef mt_s32 (*MT_UNF_AVPLAY_EVENT_CB_FN)(mt_handle hAvplay, MT_UNF_AVPLAY_EVENT_E enEvent, ulong u32Para);

/**Defines the attribute ID of an AVPLAY.*/
/**CNcomment: 定义AV播放器属性ID枚举类型 */
typedef enum mtUNF_AVPLAY_ATTR_ID_E
{
    MT_UNF_AVPLAY_ATTR_ID_STREAM_MODE = 0,  /**<Playing mode, MT_UNF_AVPLAY_ATTR_S*//**<CNcomment: 播放模式 , MT_UNF_AVPLAY_ATTR_S .*/

    MT_UNF_AVPLAY_ATTR_ID_ADEC,             /**<Audio attribute, MT_UNF_ACODEC_ATTR_S*//**<CNcomment: 音频属性 , MT_UNF_ACODEC_ATTR_S .*/
    MT_UNF_AVPLAY_ATTR_ID_VDEC,             /**<Video attribute, MT_UNF_VCODEC_ATTR_S*//**<CNcomment: 视频属性 , MT_UNF_VCODEC_ATTR_S  .*/

    MT_UNF_AVPLAY_ATTR_ID_AUD_PID,          /**<Audio packet identifier (PID), mt_u32*//**<CNcomment: 音频PID , mt_u32 .*/
    MT_UNF_AVPLAY_ATTR_ID_VID_PID,          /**<Video PID , mt_u32*//**<CNcomment: 视频PID , mt_u32 .*/
    MT_UNF_AVPLAY_ATTR_ID_PCR_PID,          /**<PCR PID, mt_u32*//**<CNcomment: PCR PID , mt_u32 .*/

    MT_UNF_AVPLAY_ATTR_ID_SYNC,             /**<Synchronization attribute, MT_UNF_SYNC_ATTR_S*//**<CNcomment: 同步属性 , MT_UNF_SYNC_ATTR_S .*/
    MT_UNF_AVPLAY_ATTR_ID_AFD,              /**<Whether to enable the active format descriptor (AFD), MT_BOOL* *//**<CNcomment: AFD 是否开启， MT_BOOL * .*/
    MT_UNF_AVPLAY_ATTR_ID_OVERFLOW,         /**<Overflow processing type, MT_UNF_AVPLAY_OVERFLOW_E* *//**<CNcomment: 溢出处理类型 , MT_UNF_AVPLAY_OVERFLOW_E * .*/

    MT_UNF_AVPLAY_ATTR_ID_MULTIAUD,         /**<Multiple audio attribute,  MT_UNF_AVPLAY_MULTIAUD_ATTR_S **//**<CNcomment: 多音轨属性, MT_UNF_AVPLAY_MULTIAUD_ATTR_S * .*/
    MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM,    /**<Frame Rate Parameter, MT_UNF_AVPLAY_FRMRATE_PARAM_S * *//**<CNcomment:帧率参数,MT_UNF_AVPLAY_FRMRATE_PARAM_S * .*/
    MT_UNF_AVPLAY_ATTR_ID_FRMPACK_TYPE,     /**<3D Frame Packing Type, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E * *//**<CNcomment:3D帧的打包类型,MT_UNF_VIDEO_FRAME_PACKING_TYPE_E * .*/
    MT_UNF_AVPLAY_ATTR_ID_LOW_DELAY,        /**<Low Delay Attr, MT_UNF_AVPLAY_LOW_DELAY_ATTR_S * *//**<CNcomment: 低延时属性 , MT_UNF_AVPLAY_LOW_DELAY_ATTR_S * .*/
    MT_UNF_AVPLAY_ATTR_ID_TVP,              /**<Trusted Video Path Attr, MT_UNF_AVPLAY_TVP_ATTR_S * *//**<CNcomment: 安全视频通路属性 , MT_UNF_AVPLAY_TRUST_VIDEO_PATH_ATTR_S * .*/
    MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC,           /**<AV sync Attr, MT_UNF_DMX_AVPLAY_AVSYNC_S * *//**<CNcomment:设置音视频同步属性 , MT_UNF_AVPLAY_AVSYNC_S * .*/
    MT_UNF_AVPLAY_ATTR_ID_AD,               /*set ad,设置ad pid*/
    MT_UNF_AVPLAY_ATTR_ID_AD_VOL_WEIGHT,    /*set ad volume weights, defined in MT_UNF_AVPLAY_AD_VOL_WEIGHT_E*/
    MT_UNF_AVPLAY_ATTR_ID_AVC_CFG,     /*set avc parameters, ,设置avc 参数*/
    MT_UNF_AVPLAY_ATTR_ID_APTS_ADJUST,     /*set apts adjust parameters, ,设置apts 调整 参数*/
    MT_UNF_AVPLAY_ATTR_ID_DOLBY_DOWNMIX_MODE,               /*set dolby downmix mode,设置dolby downmix模式*/
    MT_UNF_AVPLAY_ATTR_ID_DMX_MULTIAUDSYNC,
    MT_UNF_AVPLAY_ATTR_ID_DMX_BUF_FULL_CARE,
    MT_UNF_AVPLAY_ATTR_ID_WATERMARK_FILTER,
    MT_UNF_AVPLAY_ATTR_ID_DECLEAR_FRMRATE_PARAM,  /**<Frame Rate Parameter, MT_UNF_AVPLAY_FRMRATE_PARAM_S * *//**<CNcomment:帧率参数,MT_UNF_AVPLAY_FRMRATE_PARAM_S * .*/
	MT_UNF_AVPLAY_ATTR_ID_AC4_AD_ONOFF,					//0:off 1:on
	MT_UNF_AVPLAY_ATTR_ID_AC4_AD_VOL_WEIGHT,			//Value range is between -32 and +32dB, -32dB indicates main only (mute associated),+32dB indicates associated only (mute main)
	MT_UNF_AVPLAY_ATTR_ID_AC4_AD_TYPE, 					//select ad content type
	MT_UNF_AVPLAY_ATTR_ID_AC4_AD_TYPE_OVER_LANG, 		//select ad content type over LANG
	MT_UNF_AVPLAY_ATTR_ID_AC4_LANG,						//select ad language
	MT_UNF_AVPLAY_ATTR_ID_AC4_DOWNMIX_MODE,				//0: LtRt(default), 1: LoRo, 2:PCM_5_1, 3:PCM_RAW
	MT_UNF_AVPLAY_ATTR_ID_AC4_DIALOGUE_ENHANCEMENT,		//Range: 0 to 12 dB (in 1 dB steps, default is 0 dB)
	MT_UNF_AVPLAY_ATTR_ID_AC4_ENCODE_DD_DDP,			//0:none 1:DD 2:DDP
	MT_UNF_AVPLAY_ATTR_ID_AC4_ENCODE_MAT,				//0:disable 1:enable
	MT_UNF_AVPLAY_ATTR_ID_AC4_SET_PRES_ID,				//-1 ~ n, -1: auto, n: presentation id
	MT_UNF_AVPLAY_ATTR_ID_AC4_SET_ENCODE_DAP,			//0:disable 1: DAP Speaker 2: DAP Headphone
	MT_UNF_AVPLAY_ATTR_ID_DOLBY_FORCE_MS12_DEC,			
    MT_UNF_AVPLAY_ATTR_ID_BUTT
} MT_UNF_AVPLAY_ATTR_ID_E;

/**Defines the attribute of low delay.*/
/**CNcomment: 定义低延时属性结构体*/
typedef struct mtUNF_AVPLAY_LOW_DELAY_ATTR_S
{
    MT_BOOL               bEnable;  /**<Is low delay enable or not*//**<CNcomment: 低延时是否使能*/
}MT_UNF_AVPLAY_LOW_DELAY_ATTR_S;

/**Defines the attribute of trust video path.*/
/**CNcomment: 定义安全视频通路属性结构体*/
typedef struct mtUNF_AVPLAY_TVP_ATTR_S
{
    MT_BOOL               bEnable;  /**<Is trusted video path enable or not*//**<CNcomment: 安全视频通路是否使能*/
}MT_UNF_AVPLAY_TVP_ATTR_S;

/**Defines the audio/video synchronization .*/
/**CNcomment: 定义音视频同步调整区间 */
typedef struct mtUNF_SYNC_REGION_S
{
    mt_s32                s32VidPlusTime;        /**<Plus time range during video synchronization*//**<CNcomment: 视频同步超前的时间范围 */
    mt_s32                s32VidNegativeTime;    /**<Negative time range during video synchronization*//**<CNcomment: 视频同步落后的时间范围 */
    MT_BOOL               bSmoothPlay;           /**<Slow playing enable*//**<CNcomment: 慢放使能 */
} MT_UNF_SYNC_REGION_S;

/**Defines the audio/video synchronization attributes.*/
/**CNcomment: 定义音视频同步属性 */
typedef struct mtUNF_SYNC_ATTR_S
{
    MT_UNF_SYNC_REF_E     enSyncRef;             /**<Audio-based synchronization, PCR-based synchronization, and free playing without synchronization*//**<CNcomment: 音频为准，PCR为准，自由播放等 */
    MT_UNF_SYNC_REGION_S  stSyncStartRegion;     /**<Synchronization start region*//**<CNcomment: 同步起调区间 */
    MT_UNF_SYNC_REGION_S  stSyncNovelRegion;     /**<Synchronization exception region*//**<CNcomment: 同步异常区间 */
    mt_s32                s32VidPtsAdjust;       /**<Video presentation time stamp (PTS) adjustment*//**<CNcomment: 视频PTS调整 */
    mt_s32                s32AudPtsAdjust;       /**<Audio PTS adjustment*//**<CNcomment: 音频PTS调整 */

    mt_u32                u32PreSyncTimeoutMs;   /**<Pre-synchronization timeout, in ms*//**<CNcomment: 预同步的超时时间，单位为毫秒 */
    MT_BOOL               bQuickOutput;          /**<Fast output enable*//**<CNcomment: 快速输出使能 */
    MT_UNF_AVPLAY_STREAM_TYPE_E enStreamType;	 /**<Stream type*//**<CNcomment:码流类型 */
} MT_UNF_SYNC_ATTR_S;

/**Defines the playing attributes of an AVPLAY.*/
/**CNcomment: 定义AV播放属性 */
typedef struct mtUNF_AVPLAY_ATTR_S
{
    mt_u32                       u32DemuxId;   /**<ID of the DEMUX used by an AVPLAY*//**<CNcomment: AVPLAY所使用的DEMUX ID 仅当码流类型为TS时有效 */
    MT_UNF_AVPLAY_STREAM_ATTR_S  stStreamAttr; /**<Stream attributes*//**<CNcomment: 码流属性 */
} MT_UNF_AVPLAY_ATTR_S;

/**Defines the synchronization status when an AVPLAY is running.*/
/**CNcomment: 定义播放器运行状态信息中同步状态信息类型 */
typedef struct mtUNF_SYNC_STATUS_S
{
    mt_u64 u64FirstAudPts;    /**<Timestamp of the first audio frame in us*//**<CNcomment: 第一个音频帧时间戳，单位为us.*/
    mt_u64 u64LastAudPts;     /**<Timestamp of the last audio frame in us*//**<CNcomment: 最近播放的一个音频帧时间戳，单位为us.*/
    mt_u64 u64FirstVidPts;    /**<Timestamp of the first video frame in us*//**<CNcomment: 第一个视频帧时间戳，单位为us.*/
    mt_u64 u64LastVidPts;     /**<Timestamp of the last video frame in us*//**<CNcomment: 最近播放的一个视频帧时间戳，单位为us.*/
    mt_s64 s64DiffAvPlayTime; /**<Playing time difference between audio and video frames*//**<CNcomment: 音视频播放时差 .*/
    mt_u64 u64PlayTime;       /**<Playing time*//**<CNcomment: 当前已播放时间 .*/
    mt_u64 u64LocalTime;      /**<Local synchronization reference time*//**<CNcomment: 本地同步参考时间 .*/
} MT_UNF_SYNC_STATUS_S;

/**Defines the status of a media buffer.*/
/**CNcomment:定义媒体缓冲区的状态信息 */
typedef struct mtUNF_AVPLAY_BUF_STATUS_S
{
    mt_u32 u32BufId;         /**<Media buffer ID*//**<CNcomment: 媒体缓冲 标识 */
    mt_u32 u32BufSize;       /**<Media buffer size*//**<CNcomment: 媒体缓冲大小 */
    mt_u32 u32BufRptr;       /*Read pointer of the media buffer. This pointer is valid when TSs are being played.*//**<CNcomment: 媒体缓冲读指针,Ts播放时有效 */
    mt_u32 u32BufWptr;       /*Write pointer of the media buffer. This pointer is valid when TSs are being played.*//**<CNcomment: 媒体缓冲写指针,Ts播放时有效 */
    mt_u32 u32UsedSize;      /**<Used size of the media buffer*//**<CNcomment: 媒体缓冲已使用大小 */
    mt_u32 u32FrameBufTime;  /**<Frame buffer time*//**<CNcomment: 帧缓冲时间 */
    mt_u32 u32FrameBufNum;   /**<The number of frames in frame buffer*//**<CNcomment: 帧缓冲数目 仅VIDEO有效 */
    MT_BOOL bEndOfStream;    /**<Flag to indicate end of stream*//**<CNcomment: 缓冲中码流解码完毕标识 仅VIDEO有效 */
    mt_u32 u32VideoESFrameNumber;    /**<Flag to indicate end of stream*//**<CNcomment: 缓冲中未解码 码流帧数 仅VIDEO有效 */
} MT_UNF_AVPLAY_BUF_STATUS_S;

/**Defines the playing status of an AVPLAY.*/
/**CNcomment:定义AV的播放状态 */
typedef enum mtUNF_AVPLAY_STATUS_E
{
    MT_UNF_AVPLAY_STATUS_STOP = 0,  /**<Stop*/      /**<CNcomment: 停止 */
    MT_UNF_AVPLAY_STATUS_PREPLAY,   /**<Buffer*/    /**<CNcomment: 缓冲 */
    MT_UNF_AVPLAY_STATUS_PLAY,      /**<Play*/      /**<CNcomment: 播放 */
    MT_UNF_AVPLAY_STATUS_TPLAY,     /**<Trick play, such as fast forward and rewind*/   /**<CNcomment: TPlay, 快进快退 */
    MT_UNF_AVPLAY_STATUS_PAUSE,     /**<Pause*/     /**<CNcomment: 暂停 */
    MT_UNF_AVPLAY_STATUS_EOS,       /**<EOS*/       /**<CNcomment: 码流播放结束 */
    MT_UNF_AVPLAY_STATUS_SEEK ,     /**<Seek play*/ /**<CNcomment: 定位播放 */
    MT_UNF_AVPLAY_STATUS_FREEZE,    /**<Freeze*/    /**<CNcomment: 冻结画面 */

    MT_UNF_AVPLAY_STATUS_BUTT
} MT_UNF_AVPLAY_STATUS_E;

/**Defines the attribute of low delay.*/
/**CNcomment: 定义低延时属性结构体*/
typedef struct mtUNF_AVPLAY_DMX_AVSYNC_ATTR_S
{
    MT_UNF_VCODEC_TYPE_E  VdecType;/**<Video type*//**<CNcomment: 视频类型*/
    mt_u32                AdecType;/**<Audio type*//**<CNcomment: 音频类型*/
    mt_u32                AvsyncFlage;/**<Is av sync enable or not*//**<CNcomment: av sync是否使能*/
}MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S;

/**Defines the output value after the playing status of an AVPLAY is queried.*/
/**CNcomment:定义AV播放状态查询输出值类型 */
typedef struct mtUNF_AVPLAY_STATUS_INFO_S
{
    MT_UNF_SYNC_STATUS_S       stSyncStatus;      /**<Synchronization status*//**<CNcomment: 同步状态 */
    MT_UNF_AVPLAY_STATUS_E     enRunStatus;       /**<Running status*//**< CNcomment:运行状态 */
    mt_u32                     u32VidFrameCount;  /**<Count of played video frames*//**<CNcomment: 视频已播放帧数 */
    mt_u32                     u32AuddFrameCount; /**<Count of played audio frames*//**<CNcomment: 音频已播放帧数 */
    MT_UNF_AVPLAY_BUF_STATUS_S stBufStatus[MT_UNF_AVPLAY_BUF_ID_BUTT]; /**<Status of the media buffer*//**<CNcomment:媒体缓冲状态 */
    mt_u32                     u32VidErrorFrameCount; /**<Number of error frames during video decoding*/ /**<CNcomment: 视频解码错误帧数 */
} MT_UNF_AVPLAY_STATUS_INFO_S;

/**Defines the info of video/audio/avsync for ci test.*/
/**CNcomment:定义AV播放状态查询输出值类型 */
typedef struct mtUNF_AVPLAY_CI_TEST_INFO_S
{
	mt_u32		VFrmRate;
	mt_u32		VDecErrFrmCnt;
	mt_u32 		VFirstFrmShowed;	//1-first video frame showed, 0-no
	mt_u32 		VWrite2DisplayIdx;
	mt_u32 		VDropFrmCnt;
	
	mt_u32		AMute;	//bit0:adac mute, bit1:spdif mute, bit2:hdmi mute; 1-mute, 0-unmute;
	mt_u32		AVolume;
	mt_u32		ADecFrmCnt;
	mt_u32		AErrFrmCnt;
	mt_u32		ADropFrmCnt;
	mt_u32		ADescriptionFrmCnt;
	mt_u32		AUnderrunCnt;
	mt_u32		ASndPlayStatus;//0:PLAY, 1:STOP
	mt_u32		ASndPcmBufCnt;
	
	mt_u32		SyncMode;
	mt_u32		SyncFlag;
	mt_u32		SyncVPts;
	mt_u32		SyncAPts;
	mt_u32		SyncAVptsDiff;	//abs(vpts - apts)/45
	mt_u32		SyncTotalVFrmPlayCnt;
	mt_u32		SyncTotalVFrmHoldCnt;
	mt_u32		SyncTotalVFrmDropCnt;
} MT_UNF_AVPLAY_CI_TEST_INFO_S;

/**Defines the information about the playing program streams to be queried.*/
/**CNcomment: 定义播放节目流查询信息类型 */
typedef struct mtUNF_AVPLAY_STREAM_INFO_S
{
    MT_UNF_VCODEC_STREAMINFO_S stVidStreamInfo; /**<Video stream information*//**<CNcomment:视频流信息 */
    MT_UNF_ACODEC_STREAMINFO_S stAudStreamInfo; /**<Audio stream information*//**<CNcomment:音频流信息 */
} MT_UNF_AVPLAY_STREAM_INFO_S;

/**Defines the information about an I frame.*//**CNcomment:定义I帧数据信息类型 */
typedef struct mtUNF_AVPLAY_I_FRAME_S
{
    mt_u8                *pu8Addr;    /**<User-state virtual address of a frame*//**<CNcomment:帧数据用户态虚拟地址 */
    mt_u32               u32BufSize; /**<Frame size, in byte*//**<CNcomment:帧数据大小，单位字节 */
    MT_UNF_VCODEC_TYPE_E enType;     /*Protocol type of a data segment*//**<CNcomment:该片数据的协议类型 */
} MT_UNF_AVPLAY_I_FRAME_S;

/**Defines the decoder type. The occupied memory varies according to decoders.*/
/**CNcomment:定义解码器类型 不同类型的解码器占用内存不同 */
typedef enum mtMT_UNF_VCODEC_DEC_TYPE_E
{
    MT_UNF_VCODEC_DEC_TYPE_NORMAL,            /**<Normal type.*//**<CNcomment:普通类型 */

    /**<I frame decoding type. If an AVPLAY is used to decode I frames only (MT_UNF_AVPLAY_DecodeIFrame), you can select this type to reduce the memory usage.*/
    /**<CNcomment:I帧解码类型 如果avplay仅用于I帧解码(MT_UNF_AVPLAY_DecodeIFrame) 设置为此类型可以节省内存 */
    MT_UNF_VCODEC_DEC_TYPE_ISINGLE,

    MT_UNF_VCODEC_DEC_TYPE_BUTT
}MT_UNF_VCODEC_DEC_TYPE_E;

/**Defines the level of the protocol supported by the decoder. This value affects the number of frame buffers allocated by the normal decoder.
The greater the value, the more the required frame buffers.*/
/**CNcomment:定义解码器支持的协议级别 影响NORMAL类型解码器分配的帧存个数 值越大需要的帧存数目越多 */
typedef enum mtMT_UNF_VCODEC_PRTCL_LEVEL_E
{
    MT_UNF_VCODEC_PRTCL_LEVEL_MPEG = 0,     /**<Protocols excluding the H.264 protocol*//**<CNcomment:除h264外的其他协议 */
    MT_UNF_VCODEC_PRTCL_LEVEL_H264 = 1,     /**<H.264 protocol and other protocols*//**<CNcomment:h264协议 */
    MT_UNF_VCODEC_PRTCL_LEVEL_MVC,
    MT_UNF_VCODEC_PRTCL_LEVEL_BUTT
}MT_UNF_VCODEC_PRTCL_LEVEL_E;

/**Defines the attributes when an AVPLAY enables the video decoder. The settings affect the memory occupied by the video decoder and decoding performance.*/
/**CNcomment:定义AV播放器打开视频解码器时属性设置结构体 影响视频解码器占用内存大小及解码能力 */
typedef struct mtMT_UNF_AVPLAY_OPEN_OPT_S
{
    MT_UNF_VCODEC_DEC_TYPE_E    enDecType;       /**<Decoder type.*//**<CNcomment:解码器类型*/
    MT_UNF_VCODEC_CAP_LEVEL_E   enCapLevel;      /**<Maximum resolution supported by the decoder. This value affects the size of each frame buffer.*//**<CNcomment:解码器支持的最大分辨率 影响每个帧存的大小 */
    MT_UNF_VCODEC_PRTCL_LEVEL_E enProtocolLevel; /**<Supported protocol level. This value affects the number of frame buffers.*//**<CNcomment:支持的协议级别 影响帧存数目 */
}MT_UNF_AVPLAY_OPEN_OPT_S;

/**Defines the video display mode after an AVPLAY is stopped.*/
/**CNcomment:定义AV播放器停止时视频显示模式 */
typedef enum mtUNF_AVPLAY_STOP_MODE_E
{
    MT_UNF_AVPLAY_STOP_MODE_STILL = 0,  /**<The last frame is still after an AVPLAY is stopped.*//**<CNcomment:stop后保留最后一帧 */
    MT_UNF_AVPLAY_STOP_MODE_BLACK = 1,  /**<The blank screen is displayed after an AVPLAY is stopped.*//**<CNcomment:stop后黑屏 */
    MT_UNF_AVPLAY_STOP_MODE_BUTT
} MT_UNF_AVPLAY_STOP_MODE_E;


/**Defines the attributes when an AVPLAY is prestarted.*/
/**CNcomment:定义AV播放器预启动时属性设置结构体 */
typedef struct mtAVPLAY_PRESTART_OPT_S
{
    mt_u32       u32Reserved;
} MT_UNF_AVPLAY_PRESTART_OPT_S;

/**Defines the attributes when an AVPLAY is started.*/
/**CNcomment:定义AV播放器启动时属性设置结构体 */
typedef struct mtAVPLAY_START_OPT_S
{
    mt_u32       u32Reserved;
} MT_UNF_AVPLAY_START_OPT_S;

/**Defines the attributes when an AVPLAY is prestoped.*/
/**CNcomment:定义AV播放器预停止时属性设置结构体 */
typedef struct mtAVPLAY_PRESTOP_OPT_S
{
    mt_u32       u32Reserved;
} MT_UNF_AVPLAY_PRESTOP_OPT_S;


/**Defines the attributes when an AVPLAY is stopped.*/
/**CNcomment:定义AV播放器停止时属性设置结构体 */
typedef struct mtAVPLAY_STOP_OPT_S
{
    /*
         s32Timeout: end of stream timeout
         s32Timeout = 0   Wait until streams are played in non-block mode, that is, the interface is returned immediately. CNcomment:非阻塞等待码流播放结束，立即返回 CNend
         s32Timeout > 0   Block timeout, in ms, CNcomment:阻塞超时时间，单位为毫秒 CNend
         s32Timeout = -1  Infinite wait,CNcomment:无限等待 CNend
     */
    mt_u32                    u32TimeoutMs;    /**<Timeout*//**<CNcomment:超时值 */
    MT_UNF_AVPLAY_STOP_MODE_E enMode;          /**<Video display mode*//**<CNcomment:视频显示模式 */
} MT_UNF_AVPLAY_STOP_OPT_S;

/*Defines the attributes when an AVPLAY is paused.*/
/**CNcomment:定义AV播放器暂停时属性设置结构体 */
typedef struct mtAVPLAY_PAUSE_OPT_S
{
    mt_u32       avPushPause;    
    mt_u32       avDecPause;
    mt_u32       avSyncPause;
    mt_u32       avRenderPause;
} MT_UNF_AVPLAY_PAUSE_OPT_S;

/**Defines the direction of tplay*/
/**CNcomment:定义AV播放器TPLAY的方向 */
typedef enum mtUNF_AVPLAY_TPLAY_DIRECT_E
{
    MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD,     /**<Tplay forward*//**<CNcomment: 向前TPLAY. */
    MT_UNF_AVPLAY_TPLAY_DIRECT_BACKWARD,    /**<Tplay backward*//**<CNcomment: 向后TPLAY. */
    MT_UNF_AVPLAY_TPLAY_DIRECT_BUTT

} MT_UNF_AVPLAY_TPLAY_DIRECT_E;

/**Defines the attributes when the playing mode of an AVPLAY is tplay.*/
/**CNcomment:定义AV播放器TPALY时属性设置结构体 */
typedef struct mtAVPLAY_TPLAY_OPT_S
{
    MT_UNF_AVPLAY_TPLAY_DIRECT_E    enTplayDirect;      /**<Tplay direction*//**<CNcomment: TPLAY方向 */
    mt_u32                          u32SpeedInteger;    /**<Integral part of tplay speed*//**<CNcomment: TPLAY倍数的整数部分 */
    mt_u32                          u32SpeedDecimal;    /**<Fractional part (calculated to three decimal places) of tplay speed*//**<CNcomment: TPLAY倍数的小数部分，保留3位小数 */
} MT_UNF_AVPLAY_TPLAY_OPT_S;

/**Defines the attributes when an AVPLAY is resumed.*/
/**CNcomment:定义AV播放器恢复时属性设置结构体 */
typedef struct mtAVPLAY_RESUME_OPT_S
{
    mt_u32       u32Reserved;
} MT_UNF_AVPLAY_RESUME_OPT_S;

/**Defines the attributes when an AVPLAY is resumed.*/
/**CNcomment:定义AV播放器冻结时属性设置结构体 */
typedef struct mtAVPLAY_FREEZE_OPT_S
{
    mt_u32       u32Reserved;
} MT_UNF_AVPLAY_FREEZE_OPT_S;

/**Defines the attributes when an AVPLAY is reset.*/
/**CNcomment:定义AV播放器复位时属性设置结构体 */
typedef struct mtAVPLAY_RESET_OPT_S
{
    mt_u64       u64SeekPtsMs;   /**<clear these datas which pts is smaller than u32SeekPtsMs in buffer *//**<CNcomment:清空buffer内u32SeekPtsMs之前的数据 */
} MT_UNF_AVPLAY_RESET_OPT_S;

/**Defines the attributes when an AVPLAY is high resolution seek.*/
/**CNcomment:定义AV播放器高精度SEEK时属性设置结构体 */
typedef struct mtAVPLAY_HRSEEK_OPT_S
{
    mt_u32      u32SeekTimeMs;   /**<discard these av frame which pts is smaller than u32SeekTimeMs *//**<CNcomment:在执行seek时，u32SeekTimeMs之前的帧将会被丢掉 */
} MT_UNF_AVPLAY_HRSEEK_OPT_S;

/**Defines the attributes when an AVPLAY is step.*/
/**CNcomment:定义AV播放器步进时属性设置结构体 */
typedef struct mtUNF_AVPLAY_STEP_OPT_S
{
    mt_u32       u32Reserved;
}MT_UNF_AVPLAY_STEP_OPT_S;

/**Defines the attributes when an AVPLAY is in Flushing Stream Status.*/
/**CNcomment:定义AV播放器Flush Stream时属性设置结构体 */
typedef struct mtUNF_AVPLAY_FLUSH_STREAM_OPT_S
{
    mt_u32          u32Reserved;
}MT_UNF_AVPLAY_FLUSH_STREAM_OPT_S;

#ifdef CONFIG_MT_AUDIO_AD
/**Defines the attributes of AD audio.*/
/**CNcomment:定义AD音轨属性结构体 */
typedef struct mt_UNF_AVPLAY_AD_ATTR_S
{
    mt_u32 bAdEnble;        /**<0:close ad;1:open ad *//**< CNcomment:ad 开关 */
    mt_u32 u32AdPid;        /**<pid of ad *//**< CNcomment:ad pid */
    HA_CODEC_ID_E adectype; /**<type of ad *//**< CNcomment:ad type */
}MT_UNF_AVPLAY_AD_ATTR_S;
#endif

/**Defines the attributes of multiple audio.*/
/**CNcomment:定义多音轨属性结构体 */
typedef struct mtUNF_AVPLAY_MULTIAUD_ATTR_S
{
    mt_u32                  u32PidNum;      /**<the number of Audio PID *//**< CNcomment:音频PID个数 */
    mt_u32                  *pu32AudPid;    /**<Pointer to the array of audio PID*//**< CNcomment:指向PID数组的指针 */
    MT_UNF_ACODEC_ATTR_S    *pstAcodecAttr; /**<Pointer to the array of audio attribute*//**< CNcomment:指向音频属性数组的指针 */
    mt_u32                  u32AudStartIdx;    /**<the audio start index *//**< CNcomment:指定哪个音轨开始播放*/
}MT_UNF_AVPLAY_MULTIAUD_ATTR_S;

/**Defines the source of frame rate.*/
/**CNcomment: 定义帧率来源类型的枚举 */
typedef enum mtUNF_AVPLAY_FRMRATE_TYPE_E
{
    MT_UNF_AVPLAY_FRMRATE_TYPE_PTS,         /**<Use the frame rate calculates from PTS*//**<CNcomment: 采用PTS计算帧率 */
    MT_UNF_AVPLAY_FRMRATE_TYPE_STREAM,      /**<Use the frame rate comes from stream*//**<CNcomment: 采用码流信息中的帧率 */
    MT_UNF_AVPLAY_FRMRATE_TYPE_USER,        /**<Use the frame rate set by user*//**<CNcomment: 采用用户设置的帧率 */
    MT_UNF_AVPLAY_FRMRATE_TYPE_USER_PTS,    /**<Use the frame rate set by user until the 2nd I frame comes, then use the frame rate calculates from PTS*//**<CNcomment: 第二个I帧来之前采用用户设置的帧率，之后根据PTS计算帧率 */
    MT_UNF_AVPLAY_FRMRATE_TYPE_BUTT
}MT_UNF_AVPLAY_FRMRATE_TYPE_E;

/**Defines the parameter of frame rate.*/
/**CNcomment: 定义帧率属性参数的结构体 */
typedef struct mtUNF_AVPLAY_FRMRATE_PARAM_S
{
    MT_UNF_AVPLAY_FRMRATE_TYPE_E    enFrmRateType;  /**<The source of frame rate*//**<CNcomment: 帧率来源类型 */
    MT_UNF_VCODEC_FRMRATE_S         stSetFrmRate;   /**<Setting frame rate*//**<CNcomment: 设置的帧率 */
}MT_UNF_AVPLAY_FRMRATE_PARAM_S;

/**Defines commond to get vdec information, the parameter is MT_UNF_AVPLAY_VDEC_INFO_S.*/
/**CNcomment: 获取解码器信息命令，参数对应类型为MT_UNF_AVPLAY_VDEC_INFO_S */
#define MT_UNF_AVPLAY_GET_VDEC_INFO_CMD         0x20
/**Defines commond to set TPLAY parameter, the parameter is MT_UNF_AVPLAY_TPLAY_OPT_S.*/
/**CNcomment: 设置TPLAY参数命令，参数对应类型为MT_UNF_AVPLAY_TPLAY_OPT_S*/
#define MT_UNF_AVPLAY_SET_TPLAY_PARA_CMD        0x21
/**Defines commond to set special control information of stream, the parameter is MT_UNF_AVPLAY_CONTROL_INFO_S*/
/**CNcomment: 用来设置一些码流的特殊控制信息，参数对应类型为MT_UNF_AVPLAY_CONTROL_INFO_S*/
#define MT_UNF_AVPLAY_SET_CTRL_INFO_CMD         0x22

/**Defines commond to set video sample type, MT_BOOL *, MT_TRUE: force Progressive, MT_FALSE: auto recognise Progressive or Interlance */
/**CNcomment: 设置视频逐行信息, MT_TRUE: 强制逐行, MT_FALSE: 自动识别逐隔行*/
#define MT_UNF_AVPLAY_SET_PROGRESSIVE_CMD       0x23

/**Defines commond to set video color space, the parameter is MT_UNF_COLOR_SPACE_E*/
/**CNcomment: 设置视频色彩空间, 参数对应类型为MT_UNF_COLOR_SPACE_E*/
#define MT_UNF_AVPLAY_SET_COLORSPACE_CMD        0x24

/**Defines commond to set dpb full control, MT_BOOL* ,MT_TRUE:force delete min poc frame when dpb is full,MT_FALSE:return error when dpb is full*/
/**CNcomment:设置dpb满的时候的处理策略, 参数对应类型为MT_BOOL*/
#define MT_UNF_AVPLAY_SET_DPBFULL_CTRL_CMD      0x25

/**Defines the type of AVPLAY invoke.*/
/**CNcomment: 定义AVPLAY Invoke调用类型的枚举 */
typedef enum mtUNF_AVPLAY_INVOKE_E
{
    MT_UNF_AVPLAY_INVOKE_ACODEC  = 0,   /**<Invoke commond to control audio codec*//**<CNcomment: 控制音频解码器的Invoke调用 */
    MT_UNF_AVPLAY_INVOKE_VCODEC,        /**<Invoke commond to control video codec, MT_CODEC_VIDEO_CMD_S*//**<CNcomment: 控制视频解码器的Invoke调用 */
    MT_UNF_AVPLAY_INVOKE_GET_PRIV_PLAYINFO, /**<Invoke commond to get private play infomation,the parameter is MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S*//**<CNcomment: 获取私有播放信息的Invoke调用， 参数为MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S * */
    MT_UNF_AVPLAY_INVOKE_SET_DISP_OPTIMIZE_FLAG, /**Defines commond to set Display Optimize Flag, The Parameter is mt_u32, 1: Enable, 0: Disable */
    MT_UNF_AVPLAY_INVOKE_GET_GLOBAL_PLAYINFO,	/**<Get global play information of avplay*//**<CNcomment: 获取AVPLAY全局播放信息 */
    MT_UNF_AVPLAY_INVOKE_SET_SYNC_MODE,  /**<Invoke commond to set sync mode, mt_u32, 0 normal sync, 1 use sync replace frc*//**<CNcomment: 设置同步模式，mt_u32，0: 正常模式, 1: 使用同步替代帧率转换*/
    MT_UNF_AVPLAY_INVOKE_BUTT
} MT_UNF_AVPLAY_INVOKE_E;

/**Defines the decoding information of video codec.*/
/**CNcomment: 定义VDEC解码信息的结构体 */
typedef struct mtUNF_AVPLAY_VDEC_INFO_S
{
    mt_u32                  u32DispFrmBufNum;   /**<the number of display frame*//**<CNcomment: 显示帧存个数 */
    mt_u32                  u32FieldFlag;       /**<The encoding mode of image, 0 frame mode, 1 filed mode*//**<CNcomment: 图像编码方式, 0 帧模式，1 场模式 */
    MT_UNF_VCODEC_FRMRATE_S stDecFrmRate;      /**<decoding frame rate*//**<CNcomment: 解码帧率 */
    mt_u32                  u32UndecFrmNum;     /**<the number of undecoded frame*//**<CNcomment: 未解码帧个数 */
}MT_UNF_AVPLAY_VDEC_INFO_S;

/**Defines the private status information.*/
/**CNcomment: 定义AVPLAY私有状态信息 */
typedef struct mtUNF_AVPLAY_PRIVATE_STATUS_INFO_S
{
    mt_u64 u64LastPts;   /**<PTS of the last audio or video frame*/ /**<CNcomment: 最近播放的一个音频帧 PTS或视频PTS*/
    mt_u32 u32LastPlayTime; /**< PlayTime of the last audio or video frame */ /**<CNcomment: 最近播放的一个音频帧 PlayTime或视PlayTime  */
    mt_u32 u32DispOptimizeFlag; /**<Display Optimize Flag,1: Enable, 0: Disable*//**<CNcomment: 显示优化标志*/
} MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S;

/**Defines the special control information of stream.*/
/**CNcomment: 定义特殊控制信息 */
typedef struct mtUNF_AVPLAY_CONTROL_INFO_S
{
    mt_u32 u32IDRFlag;               /**<IDR frame Flag, 1 means IDR(instantaneous decoding refresh) frame.*/ /**<CNcomment: 是否是IDR(此帧前后无参考关系)帧，1表示是*/
    mt_u32 u32BFrmRefFlag;           /**<Whether B frame is refer frame, 1 means B frame is refer frame.*/ /**<CNcomment: B帧是否是参考帧，1表示是*/
    mt_u32 u32ContinuousFlag;        /**<Whether send frame is continusous. 1 means continusous*/ /**<CNcomment: 帧是否连续，1表示连续*/
    mt_u32 u32BackwardOptimizeFlag;  /**<The Backward Optimize Flag*//**<CNcomment: 快退优化使能标志.*/
    mt_u32 u32DispOptimizeFlag;      /**<Display Optimize Flag,1: Enable, 0: Disable*//**<CNcomment: 显示优化标志*/
}MT_UNF_AVPLAY_CONTROL_INFO_S;

/**Defines the parameter when the stream is send by MT_UNF_AVPLAY_PutBuf64.*/
/**CNcomment: 定义按PutBuf64模式送码流的参数结构体 */
typedef struct mtUNF_AVPLAY_PUTBUFEX_OPT_S
{
    MT_BOOL bEndOfFrm;      /**<whether this package of stream is the end of one frame(Video Only)*//**<CNcomment: 该包码流是否为一帧的最后一包(仅Video) */
    MT_BOOL bContinue;      /**<whether this package of stream is continued with the last package(Video Only)*//**<CNcomment: 该包码流是否与之前连续(仅Video) */
    mt_u32  u32PtsValide;   /**<whether this timestamp validity*//**<CNcomment: 该时间戳是否有效 */
    mt_u32  u32FrameFinsh;  /**<whether this frame data finsh(Video Only)*//**<CNcomment: 该帧数据是否传送结束(仅Video) */
    mt_u32  u32EosFlag;     /**<whether this end of stream flag*//**<CNcomment: 码流是否传送结束 */
}MT_UNF_AVPLAY_PUTBUFEX_OPT_S;

typedef struct mtUNF_AVPLAY_GLOBAL_PLAY_INFO_S
{
    mt_u32 u32ContentCount;
} MT_UNF_AVPLAY_GLOBAL_PLAY_INFO_S;

typedef struct mtUNF_AVPLAY_VIDEO_FRAME_INFO_S
{
    mt_u32                              u32Width;           /**<Width of the source picture*/ /**<CNcomment: 原始图像宽*/
    mt_u32                              u32Height;          /**<Height of the source picture*/ /**<CNcomment: 原始图像高*/
    mt_u32                              u32AspectWidth;     /**<aspect ratio: width*/ /**<CNcomment:宽高比之宽值 */
    mt_u32                              u32AspectHeight;    /**<aspect ratio: height*/ /**<CNcomment:宽高比之高值 */
    mt_u32                              u32fpsInteger;     /**<Integral part of the frame rate (in frame/s)*/ /**<CNcomment: 码流的帧率的整数部分, fps */
    mt_u32                              u32fpsDecimal;     /**<Fractional part (calculated to three decimal places) of the frame rate (in frame/s)*/
    MT_BOOL                             bProgressive;       /**<Sampling type (progressive or interlaced)*/ /**<CNcomment: 采样方式(逐行/隔行) */
    MT_UNF_VIDEO_FRAME_PACKING_TYPE_E   enFramePackingType; /**<3D frame packing type*/
} MT_UNF_AVPLAY_VIDEO_FRAME_INFO_S;

/**Define avc feature parameter**/
typedef struct mtUNF_AVPLAY_AVC_PARAM_S 
{
    MT_BOOL  bAVCOn;  /**<enable/disable avc feature, range: 0-disable(default), 1-enable*/ /**<CNcomment: 使能/关闭avc 功能*/
    mt_s16 s16AVCLevelCfg; /**<set avc config level,range: 0 ~ 20; */ /**<CNcomment: 设置avc 等级，范围:  0  ~ 20*/
} MT_UNF_AVPLAY_AVC_PARAM_S;


typedef struct mt_UNF_AVPLAY_DMX_BUF_FULL_CARE_S
{
    mt_u32    pid;
    mt_u32    es_buf_full;
    mt_u32    dsc_buf_full;
    mt_u32    sec_buf_full;
} MT_UNF_AVPLAY_DMX_BUF_FULL_CARE_S;

typedef struct mtUNF_SYNC_AV_INFO_S
{
    mt_u32  cur_apts;
    mt_u32  cur_vpts;

} MT_UNF_SYNC_AV_INFO_S;

typedef struct mtUNF_AVPLAY_PLAY_INFO_S
{
    mt_u32 a_pid;
    mt_u32 v_pid;
    mt_u32 a_type;
    mt_u32 v_type;
	mt_u32 tvformat;
	mt_u32 is_hdmi_connected;
}MT_UNF_AVPLAY_PLAY_INFO_S;

typedef struct mtUNF_AVPLAY_WATERMARK_FILTER_ATTR_S
{    
    mt_u32  bAudioEnable;/**<Enable watermark filtering for audio channel*//**<CNcomment: 是否打开音频通道的水印过滤功能*/
    mt_u32  bVideoEnable;/**<Enable watermark filtering for video channel*//**<CNcomment: 是否打开视频通道的水印过滤功能*/
}MT_UNF_AVPLAY_WATERMARK_FILTER_ATTR_S;

typedef struct mtUNF_AVPLAY_AV_OUT_DEBUG_INFO_S
{
    unsigned int idx;       // frame index
    unsigned int apts;      // audio pts
    unsigned int vpts;      // video pts
    unsigned int out_crc;   // for video: the crc value of the video displaying frame
                            // for audio: the crc value of the audio rendering frame
} MT_UNF_AVPLAY_AVOUT_DEBUG_INFO_S;

typedef enum mtUNF_AVPLAY_AD_VOL_WEIGHTS_E
{
    MT_UNF_AVPLAY_AD_VOL_WEIGHT_DEFAULT = 0,  //ad volue default, level 0
    MT_UNF_AVPLAY_AD_VOL_WEIGHT_LEVEL_NEG_3,  //ad volue -3
    MT_UNF_AVPLAY_AD_VOL_WEIGHT_LEVEL_NEG_2,  //ad volue -2
    MT_UNF_AVPLAY_AD_VOL_WEIGHT_LEVEL_NEG_1,  //ad volue -1
    MT_UNF_AVPLAY_AD_VOL_WEIGHT_LEVEL_0,      //ad volue default, level 0
    MT_UNF_AVPLAY_AD_VOL_WEIGHT_LEVEL_1,      //ad volue +1
    MT_UNF_AVPLAY_AD_VOL_WEIGHT_LEVEL_2,      //ad volue +2
    MT_UNF_AVPLAY_AD_VOL_WEIGHT_LEVEL_3,      //ad volue +3
} MT_UNF_AVPLAY_AD_VOL_WEIGHT_E;

typedef struct mtUNF_AVPLAY_PLAYERINFO_S
{
	mt_u32							index;
	mt_u32							u32DemuxId;
    MT_UNF_AVPLAY_STREAM_TYPE_E 	enStreamType;
	MT_UNF_AVPLAY_STATUS_E          CurStatus;
    MT_BOOL                         VidEnable;
    MT_BOOL                         AudEnable;
    mt_u32         					AvplayCount;
} MT_UNF_AVPLAY_PLAYERINFO_S;

/** @} */  /** <!-- ==== Structure Definition end ==== */

/******************************* API declaration *****************************/
/** \addtogroup      AVPLAY */
/** @{ */  /** <!-- [AVPLAY] */

/**
\brief Initializes the AVPLAY module.CNcomment:初始化AVPLAY模块 CNend
\attention \n
Before calling ::MT_UNF_AVPLAY_Create to create an AVPLAY, you must call this application programming interface (API).
CNcomment 在调用AVPLAY模块其他接口前，要求首先调用本接口 CNend
\param  N/A
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NOT_EXIST There is no AVPLAY. CNcomment:AVPLAY设备不存在 CNend
\retval ::MT_ERR_AVPLAY_NOT_DEV_FILE  The file is not an AVPLAY file. CNcomment:AVPLAY非设备 CNend
\retval ::MT_ERR_AVPLAY_DEV_OPEN_ERR  An AVPLAY fails to be started. CNcomment:AVPLAY打开失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Init(mt_void);

/**
\brief Deinitializes the AVPLAY module. CNcomment:去初始化AVPLAY模块 CNend
\attention \n
Please call this API function, before call anyother API of AVPLAY module.
CNcomment: 在调用::MT_UNF_AVPLAY_Destroy接口销毁所有的播放器后，调用本接口 CNend
\param N/A
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT  The operation is invalid.  CNcomment:操作非法 CNend
\retval ::MT_ERR_AVPLAY_DEV_CLOSE_ERR  An AVPLAY fails to be stopped. CNcomment:AVPLAY关闭失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_DeInit(mt_void);

/**
\brief Get AVPLAY handle which attach to the window. CNcomment:获取邦定到指定窗口的AVPLAY句柄 CNend
\attention \n
\param[out] phAvplay  Pointer to AVPLAY handle. CNcomment: 指针类型，指向AVPLAY句柄的指针. CNend
\param[in] hWindow    window handle . CNcomment:窗口句柄 . CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR  The pointer is null.  CNcomment:指针为空 CNend
\retval ::MT_FAILURE  Failure, no AVPLAY attached to the window. CNcomment:失败,没有AVPLAY邦定到窗口. CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetHandleByWindow(mt_handle hWindow, mt_handle *phAvplay);

/**
\brief Get video frame info. CNcomment:获取视频帧信息 CNend
\attention \n
\param[out] pstVideoFrameInfo  Pointer to video frame info. For details, see the description of ::MT_UNF_AVPLAY_VIDEO_FRAME_INFO_S. CNcomment:指针类型，视频帧信息，请参见::MT_UNF_AVPLAY_VIDEO_FRAME_INFO_S. CNend
\param[in] hAvplay    AVPLAY handle . CNcomment:AVPLAY句柄 . CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR  The pointer is null.  CNcomment:指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_FAILURE  Failure. CNcomment:失败. CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetVideoFrameInfo(mt_handle hAvplay, MT_UNF_AVPLAY_VIDEO_FRAME_INFO_S *pstVideoFrameInfo);

/**
\brief Obtains the default configuration of an AVPLAY. CNcomment:获取缺省的AV播放配置 CNend
\attention \n
When calling this API to set the enCfg parameter, you must enter correct mode of the player to be created.\n
It is recommended that you call this API to obtain the default AVPLAY attributes before creating an AVPLAY. This avoids creation failures due to incomplete or incorrect parameters.
CNcomment:调用本接口输入enCfg参数时，请正确输入想要创建播放器模式\n
创建AV播放器前建议调用本接口，获取到AV播放器默认属性，避免创建AV播放器时由于参数不全或参数错误导致播放器创建不成功现象 CNend
\param[out] pstAvAttr  Pointer to AVPLAY attributes. For details, see the description of ::MT_UNF_AVPLAY_ATTR_S. CNcomment: 指针类型，AV播放属性，请参见::MT_UNF_AVPLAY_ATTR_S. CNend
\param[in] enCfg       AVPLAY type. For details, see the description of ::MT_UNF_AVPLAY_STREAM_TYPE_E. CNcomment: AV播放的类型，请参见::MT_UNF_AVPLAY_STREAM_TYPE_E. CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetDefaultConfig(MT_UNF_AVPLAY_ATTR_S *pstAvAttr, MT_UNF_AVPLAY_STREAM_TYPE_E enCfg);

/**
\brief Registers a dynamic audio decoding library. CNcomment:注册音频动态解码库 CNend
\attention \n
\param[in] pFileName Name of the file in the audio decoding library CNcomment:音频解码库文件名 CNend
\retval ::MT_SUCCESS  Success CNcomment:成功 CNend
\retval ::MT_FAILURE Failure CNcomment:失败 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR  The input pointer is null. CNcomment:输入指针为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_RegisterAcodecLib(const mt_char *pFileName);

/**
\brief Registers a dynamic video decoding library. CNcomment:注册视频动态解码库 CNend
\attention \n
\param[in] pFileName Name of the file in the video decoding library CNcomment:视频解码库文件名 CNend
\retval ::MT_SUCCESS  Success CNcomment:成功 CNend
\retval ::MT_FAILURE Failure CNcomment:失败 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR  The input pointer is null. CNcomment:输入指针为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_RegisterVcodecLib(const mt_char *pFileName);

/**
\brief Searches for registered dynamic audio decoding libraries based on the audio format.
CNcomment:根据音频格式, 查找注册音频动态解码库 CNend
\attention \n
\param[in] enFormat Audio format CNcomment:音频格式 CNend
\param[out] penDstCodecID If an audio decoding library is found, its codec ID is returned.
CNcomment:成功则返回音频解码库CodecID CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE Failure CNcomment:失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_FoundSupportDeoder(const HA_FORMAT_E enFormat,mt_u32 * penDstCodecID);

/**
\brief Sets private commands for a dynamic audio decoding library. These commands are used to call ha_codec.
CNcomment:设置私有命令给音频动态解码库, 调用ha_codec 方法  CNend
MT_HA_ERRORTYPE_E (*DecSetConfig)(mt_void * hDecoder, mt_void * pstConfigStructure);
\attention \n
\param[in] enDstCodecID  The audio Codec ID  CNcomment:音频解码库ID CNend
\param[in] pPara  Attribute structure CNcomment:属性结构 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_FAILURE Failure CNcomment:失败 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR  The input pointer is null. CNcomment:输入指针为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_ConfigAcodec(const mt_u32 enDstCodecID, mt_void *pPara);

/**
\brief Creates an AVPLAY. CNcomment:创建AV播放器 CNend
\attention \n
Note the following point when setting the input parameter pstAttr: The stream source can be MT_UNF_AVPLAY_STREAM_TYPE_ES (ESs input from the memory) or MT_UNF_AVPLAY_STREAM_TYPE_TS (ESs input from the memory and TSs input from the Tuner).
CNcomment:输入属性参数pstAttr中有几点需要注意：码流源支持MT_UNF_AVPLAY_STREAM_TYPE_ES（内存输入ES流）、MT_UNF_AVPLAY_STREAM_TYPE_TS（内存输入TS流或TUNER输入TS 流）CNend
\param[in]  pstAvAttr   Pointer to AVPLAY attributes. For details, see the description of ::MT_UNF_AVPLAY_ATTR_S. CNcomment:指针类型，AV播放属性，请参见::MT_UNF_AVPLAY_ATTR_S. CNend
\param[out] phAvplay    Pointer to the handle of a created AVPLAY.CNcomment:指针类型，创建的AV播放句柄 CNend
\retval ::MT_SUCCESS  Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_CREATE_ERR       The AVPLAY fails to be created. CNcomment:AVPLAY创建失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Create(const MT_UNF_AVPLAY_ATTR_S *pstAvAttr, mt_handle *phAvplay);

/**
\brief Destroys an AVPLAY. CNcomment:销毁AV播放器 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Destroy(mt_handle hAvplay);

mt_s32 MT_UNF_AVPLAY_Reset_Buffer(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, MT_UNF_AVPLAY_MEDIA_BUF_E enBuf,const mt_void *pPara);

/**
\brief Sets the attributes of an AVPLAY. CNcomment:设置AV播放属性 CNend
\attention \n
You can set the audio/video PID, audio/video decoding type, and synchronization mode by calling this API.\n
Different attribute IDs correspond to different data types. For details, see the Note part of MT_UNF_AVPLAY_ATTR_ID_E. The attribute types of the pPara and enAttrID parameters must be the same.\n
Before setting MT_UNF_AVPLAY_ATTR_ID_ADEC (audio decoding attribute) and MT_UNF_AVPLAY_ATTR_ID_VDEC (video decoding attribute),\n
you must disable the audio channel or video channel. The new attributes take effect when you enable the audio channel or video channel again.
CNcomment:调用本接口可实现设置音视频PID、设置音视频解码类型、设置同步方式等功能\n
不同的属性ID对应的结构体请参见结构体MT_UNF_AVPLAY_ATTR_ID_E的[注意], pPara参数要与enAttrID对应的属性结构体类型保持一致\n
当需要设置MT_UNF_AVPLAY_ATTR_ID_ADEC(音频解码属性),MT_UNF_AVPLAY_ATTR_ID_VDEC(视频解码属性)时\n
需要先关闭音频或视频通道，再设置新属性，然后再重新打开音频或视频通道新属性才可以生效。 CNend
\param[in] hAvplay         AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] enAttrID        Attribute ID CNcomment:属性ID CNend
\param[in] pPara  Data type corresponding to an attribute ID CNcomment:属性ID对应结构 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_SetAttr(mt_handle hAvplay, MT_UNF_AVPLAY_ATTR_ID_E enAttrID, mt_void *pPara);

/**
\brief Obtains the attributes of an AVPLAY. CNcomment:获取AV播放属性 CNend
\attention \n
N/A
\param[in] hAvplay          AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] enAttrID         Attribute ID CNcomment:属性ID CNend
\param[in] pPara   Data type corresponding to an attribute ID, CNcomment:属性ID对应结构 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetAttr(mt_handle hAvplay, MT_UNF_AVPLAY_ATTR_ID_E enAttrID, mt_void *pPara);

/**
\brief Sets the mode of a video decoder. CNcomment:设置视频解码器的模式 CNend
\attention \n
This API is used in trick play mode. Before switching the mode to the trick play mode, you must enable a decoder to decode only I frames by calling this API.\n
Before switching the mode to the normal mode, you also need to set the mode of a decoder to normal by calling this API.
CNcomment:本接口主要应用在快进播放的场景，当切换到快进播放前，可以先调用本接口将解码器设置为只解I帧，\n
当切换回正常播放前，先调用本接口将解码器设置为NORMAL。 CNend
\param[in] hAvplay            AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] enDecodeMode       Decoding mode CNcomment:解码模式 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_SetDecodeMode(mt_handle hAvplay, MT_UNF_VCODEC_MODE_E enDecodeMode);

/**
\brief Registers an event. CNcomment:注册事件 CNend
\attention \n
N/A
\param[in] hAvplay     AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] enEvent     Event type enumeration CNcomment:枚举类型，表示事件类型 CNend
\param[in] pfnEventCB  Pointer to the callback function corresponding to the registered event. CNcomment:回调函数指针，指向与注册事件对应的回调函数 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT     The AVPLAY is not initialized.  CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_RegisterEvent(mt_handle      hAvplay,
                                   MT_UNF_AVPLAY_EVENT_E     enEvent,
                                   MT_UNF_AVPLAY_EVENT_CB_FN pfnEventCB);

/**
\brief Deregisters an event. CNcomment:取消注册事件 CNend
\attention \n
N/A
\param[in] hAvplay   AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] enEvent   Event type enumeration CNcomment:枚举类型，表示事件类型 CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_UnRegisterEvent(mt_handle hAvplay, MT_UNF_AVPLAY_EVENT_E enEvent);

/**
\brief set dolby certification test mode. CNcomment:设置杜比认证模式 CNend
\attention \n
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] bEnable    Enable/Disable dolby certification test mode. 0-diable, 1-enable; CNcomment:使能或禁止杜比认证模式 CNend
\retval ::MT_SUCCESS Success             CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_SetDDPTestMode(mt_handle hAvplay, MT_BOOL bEnable);

/**
\brief Enables an AVPLAY channel. CNcomment:打开AV播放器通道 CNend
\attention \n
You can enable an audio channel and a video channel for each AVPLAY. If you only need to play videos or audios, you can enable the corresponding channel to save resources.
CNcomment:每个AV播放器仅支持打开音视频通道各1个。如果只播放音频或视频，只需要打开相应通道，以节省资源。 CNend
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] enChn    Separate audio channel or video channel. For details, see the description of ::MT_UNF_AVPLAY_MEDIA_CHAN_E. CNcomment:单独的音视频通道，请参见::MT_UNF_AVPLAY_MEDIA_CHAN_E. CNend
\param[in] pPara    Pointer type. For details, see the description of ::MT_UNF_AVPLAY_OPEN_OPT_S. CNcomment:指针类型，请参见::MT_UNF_AVPLAY_OPEN_OPT_S. CNend
    If enChn is set to MT_UNF_AVPLAY_MEDIA_CHAN_VID, this API is used to specify the maximum decoding performance of the video decoder.
    If enChn is set to NULL, the maximum performance H264+MT_UNF_VCODEC_CAP_LEVEL_FULLHD is used by default.
    The higher the configured decoding performance, the larger the required MMZ. It is recommended that you configure the performance as required.
    CNcomment:enChn为MT_UNF_AVPLAY_MEDIA_CHAN_VID时用来指定视频解码器的最大解码能力。
    如果设为NULL，将默认为最大能力: H264+MT_UNF_VCODEC_CAP_LEVEL_FULLHD。
    配置支持的能力越大，需要的MMZ物理内存也就越大，建议按需配置即可。 CNend
\retval ::MT_SUCCESS Success             CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_ChnOpen(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const mt_void *pPara);

/**
\brief Disables an AVPLAY channel. CNcomment:关闭AV播放器通道 CNend
\attention \n
N/A
\param[in] hAvplay   AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] enChn     Separate audio channel or video channel. For details, see the description of ::MT_UNF_AVPLAY_MEDIA_CHAN_E. CNcomment:单独的音视频通道，请参见::MT_UNF_AVPLAY_MEDIA_CHAN_E. CNend
\retval ::MT_SUCCESS Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_ChnClose(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn);

/**
\brief Start an AVPLAY to prepare play mode, which just demux ts stream. CNcomment:启动播放器进入仅仅做解复用TS的PREPLAY状态 CNend
\attention \n
After enabling channels and setting their attributes, you can call this API to start an AVPLAY to enable it to work in prepare play mode. The audios and videos can be prepared play separately or simultaneously.
CNcomment:当完成通道打开和属性设置后，调用本接口启动预播放，进入PREPLAY状态。支持分别和同时启动音视频预播放。 CNend
\param[in] hAvplay         AVPLAY handle  CNcomment:AV播放句柄 CNend
\param[in] enChn           Separate audio channel or video channel. For details, see the description of ::MT_UNF_AVPLAY_MEDIA_CHAN_E. CNcomment:单独的音视频通道，请参见::MT_UNF_AVPLAY_MEDIA_CHAN_E. CNend
\param[in] pstPreStartOpt     Pointer used for expansion. You can set it to NULL. CNcomment:指针类型，待扩展使用，设为NULL即可. CNend
\retval ::MT_SUCCESS  Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_PreStart(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const MT_UNF_AVPLAY_PRESTART_OPT_S *pstPreStartOpt);

/**
\brief Starts an AVPLAY. The AVPLAY is in play mode. CNcomment:启动播放器，进入PLAY状态 CNend
\attention \n
After enabling channels and setting their attributes, you can call this API to start an AVPLAY to enable it to work in play mode. The audios and videos can be played separately or simultaneously.
CNcomment:当完成通道打开和属性设置后，调用本接口启动播放，进入PLAY状态。支持分别和同时启动音视频播放。 CNend
\param[in] hAvplay         AVPLAY handle  CNcomment:AV播放句柄 CNend
\param[in] enChn           Separate audio channel or video channel. For details, see the description of ::MT_UNF_AVPLAY_MEDIA_CHAN_E. CNcomment:单独的音视频通道，请参见::MT_UNF_AVPLAY_MEDIA_CHAN_E. CNend
\param[in] pstStartOpt     Pointer used for expansion. You can set it to NULL. CNcomment:指针类型，待扩展使用，设为NULL即可. CNend
\retval ::MT_SUCCESS  Success CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Start(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const MT_UNF_AVPLAY_START_OPT_S *pstStartOpt);

/**
\brief Stop an AVPLAY to prepare stop mode,the interface is reserved for future use. CNcomment:停止AV播放使其进入PRESTOP状态,该接口保留备用 CNend
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] enChn    Separate audio channel or video channel. For details, see the description of ::MT_UNF_AVPLAY_MEDIA_CHAN_E. CNcomment:单独的音视频通道，请参见::MT_UNF_AVPLAY_MEDIA_CHAN_E. CNend
\param[in] pstPreStopOpt   Pointer to stop mode. For details, see the description of ::MT_UNF_AVPLAY_PRESTOP_OPT_S. CNcomment:指针类型，清屏模式，请参见::MT_UNF_AVPLAY_STOP_OPT_S. CNend
\retval ::MT_ERR_AVPLAY_NOT_SUPPORT not support for the moment  CNcomment:暂不支持 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_PreStop(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const MT_UNF_AVPLAY_PRESTOP_OPT_S *pstPreStopOpt);

/**
\brief Stops an AVPLAY. Then the AVPLAY is in stop mode. CNcomment:停止AV播放，进入STOP状态 CNend
\attention \n
If you call this API, all selected channels stop playing. The playing audios and videos can be stopped separately or simultaneously.\n
The parameter pstStopOpt->enMode takes effect only when a video channel is selected.\n
If pstStopOpt->enMode is set to NULL or MT_UNF_AVPLAY_STOP_MODE_STILL, the last frame is kept still.\n
If pstStopOpt->enMode is set to MT_UNF_AVPLAY_STOP_MODE_BLACK, the last frame is cleared, and the blank screen appears.\n
When pstStopOpt->u32TimeoutMs is 0, the AVPLAY stops and this API is returned.\n
When pstStopOpt->u32TimeoutMs is greater than 0, this API is blocked until the data in the buffer is used up.\n
When pstStopOpt->u32TimeoutMs is greater than -1, this API is blocked until the data in the buffer is used up.\n
To stop the audio or video separately when both the audio and video are enabled, you must set pstStopOpt->u32TimeoutMs to 0.
CNcomment:调用本接口将停止所选通道的播放，支持分别和同时停止音视频播放。\n
当所选通道中包含视频通道时，参数pstStopOpt->enMode才有意义。\n
当pstStopOpt->enMode为空或者为MT_UNF_AVPLAY_STOP_MODE_STILL时，保留最后一帧视频图像。\n
当pstStopOpt->enMode为MT_UNF_AVPLAY_STOP_MODE_BLACK时，清除视频最后一帧，视频输出为黑屏。\n
当pstStopOpt->u32TimeoutMs为0时将离开停止播放并返回。\n
当pstStopOpt->u32TimeoutMs>0时将阻塞相应时间，直到缓冲中的数据播完。\n
当pstStopOpt->u32TimeoutMs=-1时将一直阻塞到缓冲中的数据播完。\n
当音视频都处于开启状态时,要单独停止音频和视频，必须设置pstStopOpt->u32TimeoutMs为0. CNend
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] enChn    Separate audio channel or video channel. For details, see the description of ::MT_UNF_AVPLAY_MEDIA_CHAN_E. CNcomment:单独的音视频通道，请参见::MT_UNF_AVPLAY_MEDIA_CHAN_E. CNend
\param[in] pstStopOpt   Pointer to the clear screen mode. For details, see the description of ::MT_UNF_AVPLAY_STOP_OPT_S. CNcomment:指针类型，清屏模式，请参见::MT_UNF_AVPLAY_STOP_OPT_S. CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Stop(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const MT_UNF_AVPLAY_STOP_OPT_S *pstStopOpt);

/**
\brief Pauses an AVPLAY. Then the AVPLAY is in pause mode. CNcomment:暂停AV播放，进入PAUSE状态 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle CNcomment: AV播放句柄 CNend
\param[in] pstPauseOpt  Pointer used for expansion. You can set it to NULL. CNcomment:指针类型，待扩展使用，设为为NULL即可 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Pause(mt_handle hAvplay, const MT_UNF_AVPLAY_PAUSE_OPT_S *pstPauseOpt);

/**
\brief Plays videos or audios in trick play mode. Then the AVPLAY is in TPLAY mode. CNcomment:倍速播放，进入TPLAY状态 CNend
\attention \n
pstTplayOpt->u32SpeedInteger is the integer part of speed, the range is 0-64.
pstTplayOpt->u32SpeedDecimal is the decimal part of speed, the range is 0-999.
CNcomment: pstTplayOpt->u32SpeedInteger为速度的整数部分，取值范围为0-64. CNend
CNcomment: pstTplayOpt->u32SpeedDecimal为速度的小数部分，保留3位小数，取值范围为0-999. CNend
\param[in] hAvplay  AVPLAY handle CNcomment: AV播放句柄 CNend
\param[in] pstTplayOpt   The pointer of Tplay parameter,For details, see the description of ::MT_UNF_AVPLAY_TPLAY_OPT_S . CNcomment:指针类型，TPLAY参数指针,请参见MT_UNF_AVPLAY_TPLAY_OPT_S结构体定义 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Tplay(mt_handle hAvplay, const MT_UNF_AVPLAY_TPLAY_OPT_S *pstTplayOpt);

/**
is_incomplete_stream:前端有丢数据时置TRUE，否则置FALSE
*/
mt_s32 MT_UNF_AVPLAY_SetTrickCfg(mt_handle hAvplay, MT_UNF_DEC_TRICK_PARAM_S *pstTrickParam);

/**
解码模式
*/
mt_s32 MT_UNF_AVPLAY_DecFrmType(mt_handle hAvplay, MT_UNF_DEC_FRM_TYPE_E eDecFrmType);

/**
\brief Resumes an AVPLAY. Then the AVPLAY is in play mode. CNcomment:恢复AV播放，进入PLAY状态 CNend
\attention \n
By calling this API, you can resume an AVPLAY from the trick play mode or pause mode rather than the stop mode.
CNcomment:本接口用来将倍速或暂停状态恢复为播放状态，但无法将停止状态恢复为播放状态。 CNend
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] pstResumeOpt  Pointer used for expansion. You can set it to NULL. CNcomment:指针类型，待扩展使用，暂置为空即可 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Resume(mt_handle hAvplay, const MT_UNF_AVPLAY_RESUME_OPT_S *pstResumeOpt);

/**
\brief Freezes an AVPLAY. Then the AVPLAY is in freeze mode. AV DECs are on going, but no AV output. Keep displaying last frozen frame. CNcomment:暂停AV播放，进入FREEZE状态。解码器正常解码，但是音视频没有输出，保持显示Freeze时的最后一帧。 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle CNcomment: AV播放句柄 CNend
\param[in] pstFreezeOpt  Pointer used for expansion. You can set it to NULL. CNcomment:指针类型，待扩展使用，设为为NULL即可 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Freeze(mt_handle hAvplay, const MT_UNF_AVPLAY_FREEZE_OPT_S *pstFreezeOpt);

/**
\brief Resets an AVPLAY. In this case, the play mode is not changed. CNcomment:复位AV播放，不改变状态 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] pstResetOpt   Pointer used for expansion. You can set it to NULL. CNcomment:指针类型，待扩展使用，设为为NULL即可 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Reset(mt_handle hAvplay, const MT_UNF_AVPLAY_RESET_OPT_S *pstResetOpt);

/**
\brief Do a high resolution seek. In this case, the play mode is not changed. CNcomment:送下来的ES数据解出来的Video帧的时间戳小于参数指定的time，该帧将会被丢弃，不改变状态 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] pstSeekOpt->u32SeekTimeMs is the target seek time. CNcomment: pstSeekOpt->u32SeekTimeMs seek的目标时间，以毫秒为单位
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_FAILURE Failure CNcomment:失败 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_HRSeek(mt_handle hAvplay, const MT_UNF_AVPLAY_HRSEEK_OPT_S *pstSeekOpt);

/**
\brief Flush an AVPLAY's all input and output buffers in playing state. In this case, the play mode is not changed. CNcomment:播放状态下Flush AV播放的缓冲数据，不改变状态 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Flush(mt_handle hAvplay);

/**
\brief Flush audio all input and output buffers in playing state. In this case, the play mode is not changed. CNcomment:播放状态下Flush AV播放的缓冲数据，不改变状态 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Flush_Audio(mt_handle hAvplay);

/**
\brief Step play. CNcomment:步进播放 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] pstStepOpt   Pointer used for expansion. You can set it to NULL. CNcomment:指针类型，待扩展使用，设为为NULL即可 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Step(mt_handle hAvplay, const MT_UNF_AVPLAY_STEP_OPT_S *pstStepOpt);

mt_s32 MT_UNF_AVPLAY_GetFrame(mt_handle  hAvplay, MT_UNF_AO_FRAMEINFO_S * p_ao_frame);

mt_s32 MT_UNF_AVPLAY_ReleaseFrame(mt_handle  hAvplay,  MT_UNF_AO_FRAMEINFO_S * p_ao_frame);

/**
\brief Applies for a buffer for storing the streams played by an AVPLAY. CNcomment:申请AV播放的码流缓冲 CNend
\attention \n
This API is used only when you want to play the ESs obtained from Internet or local ESs.\n
The pstData parameter is used to return the start address and size of the buffer applied for.\n
If u32TimeOutMs is set to 0, it indicates that the waiting time is 0; if u32TimeOutMs is set to 0XFFFFFFFF, it indicates that the API waits for an infinite time; if u32TimeOutMs is set to other values, it indicates that the waiting time is u32TimeOutMs ms.\n
If no buffer can be applied for during the block period, an error code indicating full buffer is returned.\n
If u32TimeOutMs is set to 0, and no buffer can be applied for, it indicates that the audio and video buffers are full. In this case, you need to call the usleep(N*1000) function to release the CPU.
Therefore, other threads can be scheduled.
CNcomment:当播放网络或本地ES流时才需要使用本接口。\n
参数pstData用来返回成功申请到的Buffer的首地址以及大小。\n
u32TimeOutMs设置为0表示不等待，设置为0xffffffff表示一直等待，设置为其他值表示等待u32TimeOutMs毫秒。\n
若超过阻塞时间，还无法申请到Buffer，则返回buffer满错误码\n
u32TimeOutMs配置为0时，如果申请不到Buffer，说明此时音视频的Buffer已满，需要通过usleep(N*1000)释放cpu
以使其它线程能够得到调度。 CNend
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] enBufId     Buffer queue ID CNcomment:缓冲队列ID CNend
\param[in] u32ReqLen   Size of the buffer applied for CNcomment:申请缓存的大小 CNend
\param[out] pstData    Pointer to the returned buffer CNcomment:返回缓存指针 CNend
\param[in] u32TimeOutMs      Wait timeout, in ms CNcomment:等待超时时间，单位ms. CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetBuf(mt_handle  hAvplay,
                            MT_UNF_AVPLAY_BUFID_E enBufId,
                            mt_u32                u32ReqLen,
                            MT_UNF_STREAM_BUF_S  *pstData,
                            mt_u32                u32TimeOutMs);

/* @DEPRECATED：please use MT_UNF_AVPLAY_PutBuf64 instead */
mt_s32 MT_UNF_AVPLAY_PutBuf(mt_handle hAvplay, MT_UNF_AVPLAY_BUFID_E enBufId,
                         mt_u32 u32ValidDataLen, mt_u32 u32TimestampMs);

/** @DEPRECATED：please use MT_UNF_AVPLAY_PutBuf64 instead */
mt_s32 MT_UNF_AVPLAY_PutBuf_V1(mt_handle hAvplay, MT_UNF_AVPLAY_BUFID_E enBufId,
    mt_u32 u32ValidDataLen, mt_u64 u64Pts, mt_u32 PtsValide, mt_u32 FrameFinsh, mt_u32 EosFlag);

/** @DEPRECATED：please use MT_UNF_AVPLAY_PutBuf64 instead */
mt_s32 MT_UNF_AVPLAY_64BitPTS_PutBufEx(mt_handle hAvplay, MT_UNF_AVPLAY_BUFID_E enBufId,
    mt_u32 u32ValidDataLen, mt_u64 u64PtsUs, MT_UNF_AVPLAY_PUTBUFEX_OPT_S *pPutOpt);

/**
\brief Updates the write pointer after data is successfully copied. CNcomment:拷贝数据成功后，更新写指针 CNend
\attention \n
After transmitting streams to the buffer applied for, you can call this API to update the write pointer of the audio and video buffers.\n
If the transmitted streams do not contain timestamp, u64TimestampUs must be set to (0xFFFFFFFFFFFFFFFF).
CNcomment:在向申请到的缓冲区内送码流完毕后，调用本接口更新音视频缓冲区的写指针, 本接口可支持一帧码流分多包送入(仅Video)。\n
如果本次送入的码流没有对应的时间戳，u64TimestampUs必须为(0xFFFFFFFFFFFFFFFF)。 CNend
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] enBufId     Buffer queue ID CNcomment:缓冲队列ID CNend
\param[in] u32ValidDataLen     Number of bytes that are written to the buffer CNcomment:实际写入缓冲区的字节数 CNend
\param[in] u64TimestampUs      Timestamp, in unit of us  CNcomment:时间戳,以微秒为单位 CNend
\param[in] pPutOpt   the extern parameter of PutBuf64, see the description of ::MT_UNF_AVPLAY_PUTBUFEX_OPT_S.CNcomment:PutBuf64的额外参数，请参见::MT_UNF_AVPLAY_PUTBUFEX_OPT_S. CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT      The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_PutBuf64(mt_handle hAvplay, MT_UNF_AVPLAY_BUFID_E enBufId,
                              mt_u32 u32ValidDataLen, mt_u64 u64TimestampUs, MT_UNF_AVPLAY_PUTBUFEX_OPT_S *pPutOpt);

/**
\brief Obtains the handle of the DMX audio channel used by an AVPLAY in TS mode. CNcomment:TS模式时获取AV播放器使用的DMX音频通道的Handle CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] phDmxAudChn    Pointer to the handle of a DMX audio channel CNcomment:DMX音频通道Handle指针 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetDmxAudChnHandle(mt_handle hAvplay, mt_handle *phDmxAudChn);

/**
\brief Obtains the handle of the DMX video channel used by an AVPLAY in TS mode. CNcomment:TS模式时获取AV播放器使用的DMX视频通道的Handle. CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle CNcomment:AV播放句柄 CNend
\param[in] phDmxVidChn    Pointer to the handle of a DMX video channel CNcomment:DMX视频通道Handle指针 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetDmxVidChnHandle(mt_handle hAvplay, mt_handle *phDmxVidChn);

/**
\brief Obtains the status information about an AVPLAY. CNcomment:获取AV播放状态信息 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\param[out] pstStatusInfo  Pointer to the status of an AVPLAY. For details, see the description of MT_UNF_AVPLAY_STATUS_INFO_S. CNcomment:指针类型，AV播放状态信息，请参见MT_UNF_AVPLAY_STATUS_INFO_S. CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetStatusInfo(mt_handle          hAvplay,
                                 MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo);

/**
\brief Obtains the information about an AVPLAY for ci test. CNcomment:获取AV播放状态信息 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\param[out] pstInfo  Pointer to the information about an AVPLAY. For details, see the description of MT_UNF_AVPLAY_CI_INFO_S. CNcomment:指针类型，AV播放状态信息，请参见MT_UNF_AVPLAY_STATUS_INFO_S. CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetCiTestInfo(mt_handle          hAvplay,
                                 MT_UNF_AVPLAY_CI_TEST_INFO_S *pstCiTestInfo);

/**
\brief Obtains the information about audio and video streams. CNcomment:获取音视频码流信息 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\param[in] pstStreamInfo     Pointer to the information about audio and video streams. For details, see the description of MT_UNF_AVPLAY_STREAM_INFO_S. CNcomment:指针类型，音视频码流信息，请参见MT_UNF_AVPLAY_STREAM_INFO_S CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetStreamInfo(mt_handle          hAvplay,
                                   MT_UNF_AVPLAY_STREAM_INFO_S *pstStreamInfo);

/**
\brief Obtains the information audio spectrums. CNcomment:获取音频能量信息 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\param[in] pSpectrum      Pointer to the array of audio spectrums. CNcomment:指针类型，音频能量信息数组指针 CNend
\param[in] u32BandNum     Length of an audio spectrum array CNcomment:音频能量信息数组长度 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetAudioSpectrum(mt_handle hAvplay, mt_u16 *pSpectrum, mt_u32 u32BandNum);

/**
\brief Queries whether the AVPLAY buffer is empty. CNcomment:查询AVPLAY buffer是否已经为空 CNend
\attention \n
N/A
\param[in] hAvplay        AVPLAY handle CNcomment:AV播放句柄 CNend
\param[out] pbIsEmpty      Pointer type. This pointer indicates whether the AVPLAY buffer is empty (the playing ends). CNcomment:指针类型，指示buffer是否已经为空(播放完成) CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_IsBuffEmpty(mt_handle hAvplay, MT_BOOL * pbIsEmpty);

/**
\brief Switch the demux audio channel CNcomment:切换音频DEMUX句柄 CNend
\attention \n
N/A
\param[in] hAvplay       AVPLAY handle CNcomment: AV播放句柄 CNend
\param[in] hNewDmxAud     New demux audio handle CNcomment:新DMX句柄 CNend
\param[out] phOldDmxAud     Old  demux audio handle CNcomment:旧DMX句柄指针 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_SwitchDmxAudChn(mt_handle hAvplay, mt_handle hNewDmxAud, mt_handle *phOldDmxAud);

/**
\brief Notify an AVPLAY the stream is end CNcomment:通知AVPLAY码流已经送完 CNend
\attention \n
Call this interface to notice AVPLAY when the last package of stream has been sent,
then check whether the last frame has been output by eos event or by invoking ::MT_UNF_AVPLAY_IsBuffEmpty,
this interface is only apply to ES mode.
CNcomment: 当用户送完最后一包码流时，调用该接口通知AVPLAY，之后可以通过检测EOS事件或者调用::MT_UNF_AVPLAY_IsBuffEmpty判断最后一帧是否输出
目前该接口仅适用于ES模式 CNend
\param[in] hAvplay       AVPLAY handle CNcomment: AV播放句柄 CNend
\param[in] pstFlushOpt   Pointer used for expansion. You can set it to NULL.CNcomment:指针类型，待扩展使用，设为为NULL即可 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_FlushStream(mt_handle hAvplay, MT_UNF_AVPLAY_FLUSH_STREAM_OPT_S *pstFlushOpt);

/**
\brief AVPLAY private command invoking. CNcomment: AVPLAY私有命令调用 CNend
\attention \n
\param[in] hAvplay       AVPLAY handle CNcomment: AV播放句柄 CNend
\param[in] enInvokeType  Type of private command invoking.CNcomment:私有命令调用类型 CNend
\param[in] pPara         Pointer to the parameter of invoking. CNcomment:指针类型，指向Invoke调用的参数 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Invoke(mt_handle hAvplay, MT_UNF_AVPLAY_INVOKE_E enInvokeType, mt_void *pPara);

/**
\brief Accquire user dada. CNcomment: 获取用户数据 CNend
\attention \n
Only support Closed Caption Data.
CNcomment: 仅支持CC数据 CNend
\param[in] hAvplay       AVPLAY handle CNcomment: AV播放句柄 CNend
\param[out] pstUserData  user data.CNcomment:用户数据 CNend
\param[out] penType      user data type. CNcomment:用户数据类型 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_AcqUserData(mt_handle hAvplay, MT_UNF_VIDEO_USERDATA_S *pstUserData, MT_UNF_VIDEO_USERDATA_TYPE_E *penType);

/**
\brief Accquire user dada. CNcomment: 释放用户数据 CNend
\attention \n
Only support Closed Caption Data.
CNcomment: 仅支持CC数据 CNend
\param[in] hAvplay       AVPLAY handle CNcomment: AV播放句柄 CNend
\param[in] pstUserData  user data.CNcomment:用户数据 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_RlsUserData(mt_handle hAvplay, MT_UNF_VIDEO_USERDATA_S* pstUserData);

/**
\brief Reset user data buffer. CNcomment: 清空所有用户数据 CNend
\attention \n
Only support Closed Caption Data.
CNcomment: 仅支持CC数据 CNend
\param[in] hAvplay       AVPLAY handle CNcomment: AV播放句柄 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_RstUserDataBuffer(mt_handle hAvplay);

/**
\brief Obtains the Audio's status information about an AVPLAY. CNcomment:获取Audio播放状态信息 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\param[out] pstStatusInfo  Pointer to the status of an AVPLAY. For details, see the description of MT_UNF_AVPLAY_STATUS_INFO_S. CNcomment:指针类型，Audio播放状态信息，请参见MT_UNF_AVPLAY_STATUS_INFO_S. CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetAudioStatusInfo(mt_handle          hAvplay,
                                 MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo);

/**
\brief Obtains the Video's status information about an AVPLAY. CNcomment:获取Video播放状态信息 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\param[out] pstStatusInfo  Pointer to the status of an AVPLAY. For details, see the description of MT_UNF_AVPLAY_STATUS_INFO_S. CNcomment:指针类型，Video播放状态信息，请参见MT_UNF_AVPLAY_STATUS_INFO_S. CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetVideoStatusInfo(mt_handle          hAvplay,
                                 MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo);

/**
\brief Obtains the Sync's status information about an AVPLAY. CNcomment:获取同步状态信息 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\param[out] pstStatusInfo  Pointer to the status of an AVPLAY. For details, see the description of MT_UNF_AVPLAY_STATUS_INFO_S. CNcomment:指针类型，同步状态信息，请参见MT_UNF_AVPLAY_STATUS_INFO_S. CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetSyncStatusInfo(mt_handle          hAvplay,
                                 MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo);

/**
\brief config avc info. about an AVPLAY. CNcomment:配置AVC 信息 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\param[in] pstStatusInfo  Pointer to the status of an AVPLAY. For details, see the description of MT_UNF_AVPLAY_STATUS_INFO_S. CNcomment:指针类型，同步状态信息，请参见MT_UNF_AVPLAY_STATUS_INFO_S. CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_AvcConfig(mt_handle          hAvplay,
                                 MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo);
								 
/**
\brief set the AVsync reference mode for AVsync module. CNcomment:设置AVsync同步模式 CNend
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\param[in] enAVsyncMode  AVsync reference mode. CNcomment:AVsync 同步模式. CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_ERR_AVPLAY_DEV_NO_INIT      The AVPLAY is not initialized. CNcomment:AVPLAY未初始化 CNend
\retval ::MT_ERR_AVPLAY_NULL_PTR         The input pointer is null. CNcomment:输入指针为空 CNend
\retval ::MT_ERR_AVPLAY_INVALID_PARA     The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_AVPLAY_INVALID_OPT    The operation is invalid. CNcomment:操作非法 CNend
\retval ::MT_ERR_SYNC_INVALID_PARA       The input parameter is invalid. CNcomment:输入参数非法 CNend
\retval ::MT_ERR_SYNC_NULL_PTR           The input pointer is null. CNcomment:输入指针为空 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_SetAVsyncMode(mt_handle hAvplay, MT_UNF_SYNC_REF_E enAVsyncMode);

/* For CI test only */
mt_s32 MT_UNF_AVPLAY_EnableAVOutInfo(mt_handle hAvplay, mt_u32 enable);


/* For CI test only */ 
mt_s32 MT_UNF_AVPLAY_GetAVOutInfo(mt_handle          hAvplay,
                                 MT_UNF_AVPLAY_AVOUT_DEBUG_INFO_S *pstOutInfo);

/**
\brief stop the ADEC module. CNcomment:关闭ADEC
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_FAILURE Failure  CNcomment:失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_StopAudDec(mt_handle hAvplay);

/**
\brief start the ADEC module. CNcomment:打开ADEC
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_FAILURE Failure  CNcomment:失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_StartAudDec(mt_handle hAvplay);

/**
\brief set do audio track signal to drv. CNcomment:打开ADEC
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_FAILURE Failure  CNcomment:失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_AudioTrack(mt_handle hAvplay);

/**
\brief set the audio HEAAC ON/OFF.
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle
\retval ::MT_SUCCESS Success
\retval ::MT_FAILURE Failure
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_Enable_AudioHEAAC(mt_handle hAvplay);

/** @} */  /** <!-- ==== API declaration end ==== */

/**
\brief set the audio downmix ON/OFF. 
\attention \n
N/A
\param[in] hAvplay	AVPLAY handle
\param[in] enable  downmix enable
\retval ::MT_SUCCESS Success
\retval ::MT_FAILURE Failure
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_set_downmix_enable(mt_handle hAvplay, u32 enable);

/**
\brief set trick or seek start signal to drv. CNcomment:发送启动trick或seek信号到drv
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_FAILURE Failure  CNcomment:失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_TrickSeekIn(mt_handle hAvplay);

/**
\brief set trick or seek end signal to drv. CNcomment:发送结束trick或seek信号到drv
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_FAILURE Failure  CNcomment:失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_TrickSeekOut(mt_handle hAvplay);

/**
\brief Enable sw adec decoder
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_FAILURE Failure  CNcomment:失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_SW_ADEC_ENABLE(mt_handle hAvplay);

/**
\brief GetVideoESPhyAddr
\attention \n
N/A
\param[in] hAvplay  AVPLAY handle  CNcomment:AV播放句柄 CNend
\param[out] esBuffPhyAddr address  CNcomment: Video ES 物理地址
\param[out] esBuffPhyAddr lenth  CNcomment: Video ES 长度
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_FAILURE Failure  CNcomment:失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_GetVideoESPhyAddr(mt_handle hAvplay, phys_addr_t *esBuffPhyAddr, mt_u32 *esBuffSize);

/** @} */  /** <!-- ==== API declaration end ==== */

/**
\brief GetVideoESPhyAddr
\attention \n
N/A
\param[out] esBuffPhyAddr address  CNcomment: MT_UNF_AVPLAY_PLAYERINFO_S 
\retval ::MT_SUCCESS Success  CNcomment:成功 CNend
\retval ::MT_FAILURE Failure  CNcomment:失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_AVPLAY_ListAllPlayer(MT_UNF_AVPLAY_PLAYERINFO_S * info);

mt_s32 MT_UNF_AVPLAY_GetMetaInfo(mt_handle hAvplay, MT_UNF_AVPLAY_METARINFO_S *pMetaInfo);
mt_s32 MT_UNF_AVPLAY_SetPos(mt_handle hAvplay, float x, float y, float z);
mt_s32 MT_UNF_AVPLAY_SelObj(mt_handle hAvplay, mt_u16 id, mt_u16 on);

/** @} */  /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif

#endif
