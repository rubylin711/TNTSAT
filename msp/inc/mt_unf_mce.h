/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/**
 * \file
 * \brief Describes the logo and play contents and related configuration information.
 */
#ifndef __MT_UNF_MCE_H__
#define __MT_UNF_MCE_H__

#include "mt_unf_common.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_ecs.h"
#include "mt_go_surface.h"
#include "mt_unf_avplay.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/********************************Struct Definition********************************/
/** \addtogroup      MCE */
/** @{ */ /** <!-- [MCE] */

/** Defines the logo parameter*/
/** CNcomment:开机Logo参数*/
typedef struct mtUNF_MCE_LOGO_PARAM_S
{
    MT_BOOL bLogoEnable; /**<Whether logo is enable*/ /**<CNcomment: Logo是否使能*/
    mt_u32 u32LogoLen; /**<Length of logo data*/      /**<CNcomment: Logo数据的长度*/
} MT_UNF_MCE_LOGO_PARAM_S;

/*fastplay supports the decoding mode of ADEC*/ /**<CNcomment:fastplay 支持的ADEC解码格式 */
typedef enum mtUNF_MCE_ADEC_TYPE_E {
    MT_UNF_MCE_ADEC_TYPE_MP2 = 0,
    /**<MP2 format*/ /**<CNcomment:MP2 制式 */
    MT_UNF_MCE_ADEC_TYPE_MP3 = 1,
    /**<MP3 format*/                               /**<CNcomment:MP3 制式 */
    MT_UNF_MCE_ADEC_TYPE_BUTT /**<Invalid format*/ /**<CNcomment:无效 制式 */
} MT_UNF_MCE_ADEC_TYPE_E;

/**fastplay supports the decoding mode of VDEC*/ /**<CNcomment:fastplay 支持的VDEC解码格式 */
typedef enum mtUNF_MCE_VDEC_TYPE_E {
    MT_UNF_MCE_VDEC_TYPE_MPEG2 = 0,
    /**<MPEG2 format*/ /**<CNcomment:MPEG2  制式 */
    MT_UNF_MCE_VDEC_TYPE_MPEG4 = 1,
    /**<MPEG4 format*/ /**<CNcomment:MPEG4  制式 */
    MT_UNF_MCE_VDEC_TYPE_H264 = 2,
    /**<H264 format*/ /**<CNcomment:H264  制式 */
    MT_UNF_MCE_VDEC_TYPE_AVS = 3,
    /**<AVS format*/                               /**<CNcomment:AVS   制式 */
    MT_UNF_MCE_VDEC_TYPE_BUTT /**<Invalid format*/ /**<CNcomment:无效 制式 */
} MT_UNF_MCE_VDEC_TYPE_E;

/**Parameters for digital video broadcasting (DVB) play configuration*/ /**<CNcomment:DVB配置信息  */
typedef struct mtUNF_MCE_DVB_PARAM_S
{
    mt_u32 u32PcrPid; /**<Program clock reference (PCR) packet ID (PID)*/                          /**<CNcomment:pcr pid*/
    mt_u32 u32VideoPid; /**<Video PID*/                                                            /**<CNcomment:视频 pid*/
    mt_u32 u32AudioPid; /**<Audio PID*/                                                            /**<CNcomment:音频 pid*/
    MT_UNF_MCE_VDEC_TYPE_E enVideoType; /**<Video type*/                                           /**<CNcomment:视频 类型*/
    MT_UNF_MCE_ADEC_TYPE_E enAudioType; /**<Audio type*/                                           /**<CNcomment:音频 类型*/
    mt_u32 u32Volume; /**<volume of output*/                                                       /**<CNcomment:音量 */
    MT_UNF_TRACK_MODE_E enTrackMode; /**<Track mode. Only MT_UNF_TRACK_MODE_STEREO is supported.*/ /**<CNcomment:仅支持立体声 */
    MT_UNF_VO_DEV_MODE_E enDevMode; /**<Working mode of the VO device*/                            /**<CNcomment:dev 的 vo 模式  */
    mt_unf_fe_connect_para_t stConnectPara; /**<Tuner connect parameter*/                          /**<CNcomment:Tuner锁频参数.*/
#if 1
    mt_unf_fe_lnb_config_t stLnbCfg; /**<LNB config*/                  /**<CNcomment:LNB 配置 */
    mt_unf_fe_lnb_power_t enLNBPower; /**<LNB Power*/                  /**<CNcomment:LNB供电控制 */
    mt_unf_fe_diseqc_switch16port_t st16Port; /**<Switch16 parameter*/ /**<CNcomment:Switch16开关参数 */
    mt_unf_fe_diseqc_switch4port_t st4Port; /**<Switch4 parameter*/    /**<CNcomment:Switch4开关参数 */
    mt_unf_fe_switch_22k_t enSwitch22K; /**<22K switch parameter*/     /**<CNcomment:22K开关参数 */
#endif

} MT_UNF_MCE_DVB_PARAM_S;

/**Play parameter configuration of transport stream (TS) files*/ /**<CNcomment:TS文件配置信息 */
typedef struct mtUNF_MCE_TSFILE_PARAM_S
{
    mt_u32 u32ContentLen; /**File length*/                                                         /**<CNcomment:文件长度  */
    mt_u32 u32PcrPid; /**<pcr pid*/                                                                /**<CNcomment:pcr pid  */
    mt_u32 u32VideoPid; /**<Video PID*/                                                            /**<CNcomment:视频 pid. */
    mt_u32 u32AudioPid; /**<Audio PID*/                                                            /**<CNcomment:音频 pid. */
    MT_UNF_MCE_VDEC_TYPE_E enVideoType; /**<Video type*/                                           /**<CNcomment:视频 类型 */
    MT_UNF_MCE_ADEC_TYPE_E enAudioType; /**<Audio type*/                                           /**<CNcomment:音频 类型 */
    mt_u32 u32Volume; /**<volume of output */                                                      /**<CNcomment:音量 */
    MT_UNF_TRACK_MODE_E enTrackMode; /**<Track mode. Only MT_UNF_TRACK_MODE_STEREO is supported.*/ /**<CNcomment:仅支持立体声 */
    MT_UNF_VO_DEV_MODE_E enDevMode; /**<mode of vo device  */                                      /**<CNcomment:vo设备的模式  */
} MT_UNF_MCE_TSFILE_PARAM_S;

#define ANI_MAX_PIC_SUPPORT (30)
typedef struct mtUNF_MCE_ANI_PARAM_S
{
    mt_u32 u32ContentLen; /**File length*/ /**<CNcomment:文件长度  */
    mt_u32 u32PicCount;
    mt_u32 au32PicTime[ANI_MAX_PIC_SUPPORT];
} MT_UNF_MCE_ANI_PARAM_S;

/**Play type*/ /**<CNcomment:播放类型*/
typedef enum mtUNF_MCE_PLAY_TYPE_E {
    MT_UNF_MCE_TYPE_PLAY_DVB = 0,
    /**<DVB type*/ /**<CNcomment:DVB 类型 */
    MT_UNF_MCE_TYPE_PLAY_TSFILE = 1,
    /**<TS file type*/ /**<CNcomment:ts 文件类型 */
    MT_UNF_MCE_TYPE_PLAY_ANI = 2,
    /**<ES file type*/ /**<CNcomment:es 文件类型 */
    MT_UNF_MCE_TYPE_PLAY_BUTT
} MT_UNF_MCE_PLAY_TYPE_E;

/**Play configuration*/ /**<CNcomment:播放配置信息 */
typedef struct mtUNF_MCE_PLAY_PARAM_S
{
    MT_UNF_MCE_PLAY_TYPE_E enPlayType; /**<Play type*/ /**<CNcomment:播放类型*/
    MT_BOOL bPlayEnable; /**<Whether play is enable*/  /**<CNcomment: 瞬播是否使能*/

    union
    {
	MT_UNF_MCE_DVB_PARAM_S stDvbParam; /**<dvb parameter*/  /**<CNcomment:dvb参数*/
	MT_UNF_MCE_TSFILE_PARAM_S stTsParam; /**<ts parameter*/ /**<CNcomment:ts参数*/
	MT_UNF_MCE_ANI_PARAM_S stAniParam; /**<es parameter*/   /**<CNcomment:es参数*/
    } unParam;

    MT_BOOL bContentValid; /**<whether mce content is valid */ /**<CNcomment:mce 数据是否有效*/
} MT_UNF_MCE_PLAY_PARAM_S;

/**Mode of Play Control*/ /**CNcomment:播放控制模式 */
typedef enum mtUNF_MCE_PLAYCTRL_MODE_E {
    MT_UNF_MCE_PLAYCTRL_BY_TIME,
    /**<play control by time*/ /**<CNcomment:通过时间控制*/
    MT_UNF_MCE_PLAYCTRL_BY_COUNT,
    /**<play control by count*/ /**<CNcomment:通过播放次数控制*/
    MT_UNF_MCE_PLAYCTRL_BUTT
} MT_UNF_MCE_PLAYCTRL_MODE_E;

/**Defines the parameter of fastplay stop*/ /**CNcomment:瞬播停止参数 */
typedef struct mtUNF_MCE_STOPPARM_S
{
    MT_UNF_AVPLAY_STOP_MODE_E enStopMode; /**<Mode of Stop,see ::MT_UNF_AVPLAY_STOP_MODE_E*/ /**<CNcomment:停止模式，参见::MT_UNF_AVPLAY_STOP_MODE_E*/
    MT_UNF_MCE_PLAYCTRL_MODE_E enCtrlMode; /**<Mode of Play Control*/                        /**<CNcomment:播放控制模式 */
    mt_u32 u32PlayTimeMs; /**<Play time*/                                                    /**<CNcomment:播放时间 */
    mt_u32 u32PlayCount; /**<Play count*/                                                    /**<CNcomment:播放次数 */
} MT_UNF_MCE_STOPPARM_S;

/**Defines the parameter of fastplay exit*/ /**CNcomment:瞬播退出参数 */
typedef struct mtUNF_MCE_EXITPARAM_S
{
    mt_handle hNewWin; /**<Handle of new window*/ /**<CNcomment:新窗口句柄 */
} MT_UNF_MCE_EXITPARAM_S;

/**Defines the parameter of fastplay init*/ /**CNcomment:瞬播初始化参数 */
typedef struct mtUNF_MCE_INIT_PARAM_S
{
    mt_u32 u32Reserved;
} MT_UNF_MCE_INIT_PARAM_S;

/** @} */ /** <!-- ==== Struct Definition end ==== */

/********************************API declaration********************************/
/** \addtogroup      MCE */
/** @{ */ /** <!-- [MCE]*/

/** 
\brief init mce  CNcomment:MCE配置初始化接口 CNend
\attention \n
Initializes the media control engine (MCE) before call other MCE API.
CNcomment:使用MCE接口前首先调用该接口 CNend
\param[in]  pstInitParam  Pointer to MCE init parameter, reserved. CNcomment:指针类型，指向瞬播初始化参数，保留 CNend
\retval ::MT_SUCCESS       Success. CNcomment:成功 CNend
\retval :: MT_ERR_MCE_DEV_NOT_EXIST      Device is not exist. CNcomment:设备不存在 CNend
\retval :: MT_ERR_MCE_NOT_DEVICE      Not a device. CNcomment:非设备 CNend
\retval :: MT_ERR_MCE_DEV_OPEN_ERR      Parameter is invalid. CNcomment:设备打开失败 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_MCE_Init(MT_UNF_MCE_INIT_PARAM_S *pstInitParam);

/** 
\brief deinit mce  CNcomment:MCE配置去初始化接口 CNend
\attention \n
Destroys all MCE resources.  CNcomment:完成MCE配置工作最终调用该接口 CNend
\see \n
N/A
*/
mt_void MT_UNF_MCE_DeInit(mt_void);

/** 
\brief deinit mce  CNcomment:清空开机画面接口 CNend
\attention \n
Destroys the logo.  CNcomment:清空开机画面 CNend
\retval ::MT_SUCCESS       Success. CNcomment:成功 CNend
\retval :: MT_ERR_MCE_DEV_NOT_INIT      Device is not init. CNcomment:设备未初始化 CNend
\retval :: MT_ERR_MCE_INVALID_OPT      The operation is invalid. CNcomment:无效操作 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_MCE_ClearLogo(mt_void);

/**
\brief  stop fastplay. CNcomment:停止fastplay CNend
\attention \n
If enCtrlMode is MT_UNF_MCE_PLAYCTRL_BY_TIME, u32PlayTime is valid; if enCtrlMode is MT_UNF_MCE_PLAYCTRL_BY_COUNT, u32PlayCount is valid
CNcomment: enCtrlMode选为MT_UNF_MCE_PLAYCTRL_BY_TIME时，u32PlayTime生效, 选为MT_UNF_MCE_PLAYCTRL_BY_COUNT时，u32PlayCount生效 CNend
\param[in]  pstStopParam  Pointer to fastplay  stop parameter. CNcomment:指针类型，指向瞬播停止参数 CNend
\retval ::MT_SUCCESS       Success. CNcomment:成功 CNend
\retval :: MT_ERR_MCE_DEV_NOT_INIT      Device is not init. CNcomment:设备未初始化 CNend
\retval :: MT_ERR_MCE_PTR_NULL      Pointer is null. CNcomment:空指针 CNend
\retval :: MT_ERR_MCE_PARAM_INVALID      Parameter is invalid. CNcomment无效参数 CNend
\retval :: MT_ERR_MCE_INVALID_OPT      The operation is invalid. CNcomment:无效操作 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_MCE_Stop(MT_UNF_MCE_STOPPARM_S *pstStopParam);

/**
\brief  Exit fastplay and destrory fastplay source CNcomment:退出fastplay,销毁fastplay资源 CNend
\attention \n
N/A
\param[in]  pstExitParam  Pointer to fastplay  exit parameter. CNcomment:指针类型，指向瞬播退出参数 CNend
\retval ::MT_SUCCESS       Success. CNcomment:成功 CNend
\retval :: MT_ERR_MCE_DEV_NOT_INIT      Device is not init. CNcomment:设备未初始化 CNend
\retval :: MT_ERR_MCE_PARAM_INVALID      Parameter is invalid. CNcomment无效参数 CNend
\retval :: MT_ERR_MCE_INVALID_OPT      The operation is invalid. CNcomment:无效操作 CNend
\see \n
N/A
*/
mt_s32 MT_UNF_MCE_Exit(MT_UNF_MCE_EXITPARAM_S *pstExitParam);

/** @} */ /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
