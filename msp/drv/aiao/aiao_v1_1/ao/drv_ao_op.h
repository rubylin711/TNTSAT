/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_AO_OP_FUNC_H__
#define __MT_AO_OP_FUNC_H__

#include "mt_unf_sound.h"
#include "mt_drv_ao.h"
#include "hal_aoe_func.h"
#include "hal_aiao_func.h"
#include "hal_aiao_common.h"
#include "hal_aoe.h"
#include "hal_cast.h"
#include "hal_aiao.h"
//#include "hal_tianlai_adac_v500.h"
//#include "hal_tianlai_adac_v510.h"
#include "drv_ao_private.h"

#ifdef MT_SND_AMP_SUPPORT
#include "drv_amp_ext.h"
#endif

#ifdef MT_SND_CAST_SUPPORT
#include "drv_ao_ioctl.h"
#endif
#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/
#define AO_SNDOP_PERIODBUFSIZE  4096
#define AO_SNDOP_LATENCY_THDMS  64

#define AO_SNDOP_GLOBAL_MUTE_BIT 1
#define AO_SNDOP_LOCAL_MUTE_BIT  0

#define AO_SNDOP_MAX_AOP_NUM 2  //max aop number per op 

/******************************Snd OP process FUNC*************************************/

//zgjiere; 检查部分直接绕过OP操作Hal(aop/aiao)的函数 ? 模块解耦

//zgjiere; sndop分层，该文件代码行太大，不利于维护

typedef struct
{
    mt_u32 u32BitPerSample;
    mt_u32 u32Channels;
    mt_u32 u32SampleRate;
    mt_u32 u32DataFormat;
    mt_u32 u32LatencyThdMs;
    mt_u32 u32PeriodBufSize;
    mt_u32 u32PeriodNumber;
    union
    {
        MT_UNF_SND_DAC_ATTR_S stDacAttr;
        MT_UNF_SND_I2S_ATTR_S stI2sAttr;
        MT_UNF_SND_SPDIF_ATTR_S stSpdifAttr;
        MT_UNF_SND_HDMI_ATTR_S stHDMIAttr;
        MT_UNF_SND_ARC_ATTR_S stArcAttr;
#if defined(SND_CAST_SUPPORT) 
        MT_UNF_SND_CAPTURE_ATTR_S stCaptureAttr;
#endif
    } unAttr;
} SND_OP_ATTR_S;

typedef enum
{
    SND_AOP_TYPE_I2S   = 0,     /* hbr or 2.0 pcm or 7.1 lpcm */
    SND_AOP_TYPE_SPDIF,         /* lbr or hbr(ddp) */
    SND_AOP_TYPE_CAST,          /* 2.0 16bit pcm only */

    SND_AOP_TYPE_BUTT
} SND_AOP_TYPE_E;

typedef enum
{
    SND_OUTPUT_TYPE_DAC,

    SND_OUTPUT_TYPE_I2S,

    SND_OUTPUT_TYPE_SPDIF,

    SND_OUTPUT_TYPE_HDMI,

    SND_OUTPUT_TYPE_CAST,

    SND_OUTPUT_TYPE_BUTT,
} SND_OUTPUT_TYPE_E;
typedef struct
{
    SND_OP_ATTR_S stSndPortAttr;

    MT_UNF_SND_GAIN_ATTR_S stUserGain;
    MT_UNF_TRACK_MODE_E    enUserTrackMode;
    mt_u32                 u32UserMute;   //bit[1]:global mute; bit[0]:local mute. if u32UserMute=0, real mute;else real unmute
    MT_BOOL                bBypass;
    MT_UNF_SAMPLE_RATE_E   enSampleRate;

    /* internal state */
    SND_OP_STATUS_E         enCurnStatus;
    SND_OUTPUT_TYPE_E enOutType;
    MT_UNF_SND_OUTPUTPORT_E enOutPort;
    mt_s32                  ActiveId; /* 0 or 1*/
    mt_u32                  u32OpMask; /* bit0(0/1) and bit1(0/1)*/
    AIAO_PORT_ID_E          enPortID[AO_SNDOP_MAX_AOP_NUM];
    AOE_AOP_ID_E            enAOP[AO_SNDOP_MAX_AOP_NUM];
#ifdef MT_SND_AMP_SUPPORT
    AMP_EXPORT_FUNC_S       *pstAmpFunc;
#endif
    SND_ENGINE_TYPE_E       enEngineType[AO_SNDOP_MAX_AOP_NUM];
    mmz_buffer_s            stRbfMmz[AO_SNDOP_MAX_AOP_NUM];
    AIAO_PORT_USER_CFG_S    stPortUserAttr[AO_SNDOP_MAX_AOP_NUM];
    AIAO_CAST_ID_E          CastId;
    AIAO_CAST_ATTR_S        stCastAttr;
    MT_UNF_SND_SPDIF_SCMSMODE_E     enUserSPDIFSCMSMode;
    MT_UNF_SND_SPDIF_CATEGORYCODE_E  enUserSPDIFCategoryCode;
} SND_OP_STATE_S;


    /* internal state */


mt_void 				SND_GetDelayMs(SND_CARD_STATE_S *pCard, mt_u32 *pdelayms);
mt_void					SND_DestroyOp(SND_CARD_STATE_S *pCard, MT_BOOL bSuspend);
mt_s32                  SND_CreateOp(SND_CARD_STATE_S *pCard, MT_UNF_SND_ATTR_S *pstAttr,AO_ALSA_I2S_Param_S* pstAoI2sParam, MT_BOOL bResume);
mt_s32					SND_SetOpMute(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bMute);
mt_s32					SND_GetOpMute(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbMute);
mt_s32                  SND_SetOpHdmiMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_HDMI_MODE_E enMode);
mt_s32                  SND_GetOpHdmiMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_HDMI_MODE_E *penMode);
mt_s32                  SND_SetOpSpdifMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_MODE_E enMode);
mt_s32                  SND_GetOpSpdifMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_MODE_E *penMode);
mt_s32					SND_SetOpVolume(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                        MT_UNF_SND_GAIN_ATTR_S stGain);
mt_s32					SND_GetOpVolume(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                        MT_UNF_SND_GAIN_ATTR_S *pstGain);
mt_s32                  SND_SetOpSpdifCategoryCode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, 
                                                   MT_UNF_SND_SPDIF_CATEGORYCODE_E enCategoryCode);
mt_s32                  SND_GetOpSpdifCategoryCode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, 
                                                   MT_UNF_SND_SPDIF_CATEGORYCODE_E *penCategoryCode);
mt_s32                  SND_SetOpSpdifSCMSMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                               MT_UNF_SND_SPDIF_SCMSMODE_E enSCMSMode);
mt_s32                  SND_GetOpSpdifSCMSMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, 
                                               MT_UNF_SND_SPDIF_SCMSMODE_E *penSCMSMode);
mt_s32					SND_SetOpSampleRate(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                            MT_UNF_SAMPLE_RATE_E enSampleRate);
mt_s32					SND_GetOpSampleRate(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                            MT_UNF_SAMPLE_RATE_E *penSampleRate);
mt_s32					SND_SetOpTrackMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                           MT_UNF_TRACK_MODE_E enMode);
mt_s32					SND_GetOpTrackMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                           MT_UNF_TRACK_MODE_E *penMode);
mt_s32					SND_SetOpAttr(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                      SND_OP_ATTR_S *pstSndPortAttr);
mt_s32					SND_GetOpAttr(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                      SND_OP_ATTR_S *pstSndPortAttr);
mt_s32                  SND_GetOpStatus(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, AIAO_PORT_STAUTS_S *pstPortStatus);

mt_s32					SND_StopOp(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort);
mt_s32					SND_StartOp(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort);
mt_s32                  SND_SetOpAefBypass(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bBypass);
mt_s32                  SND_GetOpAefBypass(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbBypass);

mt_handle				SND_GetOpHandlebyOutType(SND_CARD_STATE_S *pCard, SND_OUTPUT_TYPE_E enOutType);
SND_ENGINE_TYPE_E		SND_OpGetEngineType(mt_handle hSndOp);
AOE_AOP_ID_E			SND_OpGetAopId(mt_handle hSndOp);
SND_ENGINE_TYPE_E		SND_GetOpGetOutType(mt_handle hSndOp);
MT_UNF_SND_OUTPUTPORT_E SND_GetOpOutputport(mt_handle hSndOp);

#if defined (MT_SND_DRV_SUSPEND_SUPPORT)
mt_s32 SND_GetOpSetting(SND_CARD_STATE_S *pCard, SND_CARD_SETTINGS_S* pstSndSettings);
mt_s32 SND_RestoreOpSetting(SND_CARD_STATE_S *pCard, SND_CARD_SETTINGS_S* pstSndSettings);
#endif


#ifdef MT_SND_CAST_SUPPORT
mt_s32 SND_StopCastOp(SND_CARD_STATE_S *pCard, mt_s32 s32CastID);
mt_s32 SND_StartCastOp(SND_CARD_STATE_S *pCard, mt_s32 s32CastID);
mt_s32 SND_CreateCastOp(SND_CARD_STATE_S *pCard,  mt_handle *ps32CastId, MT_UNF_SND_CAST_ATTR_S *pstAttr, mmz_buffer_s *pstMMz);

mt_s32 SND_DestoryCastOp(SND_CARD_STATE_S *pCard,  mt_u32 CastId);
mt_u32 SND_ReadCastData(SND_CARD_STATE_S *pCard, mt_s32 u32CastId, AO_Cast_Data_Param_S *pstCastData);
mt_u32 SND_ReleaseCastData(SND_CARD_STATE_S *pCard, mt_s32 u32CastId, AO_Cast_Data_Param_S *pstCastData);
#endif
mt_s32 SND_ReadOpProc(struct seq_file* p, SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enPort);

mt_void SND_GetXRunCount(SND_CARD_STATE_S *pCard, mt_u32 *pu32Count);
#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif  // __MT_AO_OP_FUNC_H__
