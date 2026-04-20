/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_HAL_AOE_H__
#define __MT_HAL_AOE_H__

#include "mt_type.h"
//#include "mt_audsp_common.h"
#include "hal_aoe_common.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/
#define AOE_AIP_BUFF_LATENCYMS_MIN 10
#define AOE_AIP_FIFO_LATENCYMS_MIN 10
#define AOE_AOP_BUFF_LATENCYMS_MIN 10

#define AOE_AOP_BUFF_LATENCYMS_DF  (AOE_AOP_BUFF_LATENCYMS_MIN*4)
#define AOE_AOP_BUFF_LATENCYMS_MAX (AOE_AOP_BUFF_LATENCYMS_MIN*10)
#define AOE_CAST_BUFF_LATENCYMS_MAX (512)

#define AO_DAC_MMZSIZE_MAX    ((48000*2*sizeof(mt_u32)/1000)*AOE_AOP_BUFF_LATENCYMS_MAX)
#define AO_I2S_MMZSIZE_MAX    ((48000*2*sizeof(mt_u32)/1000)*AOE_AOP_BUFF_LATENCYMS_MAX)
#define AO_SPDIF_MMZSIZE_MAX  ((192000*2*sizeof(mt_u16)/1000)*AOE_AOP_BUFF_LATENCYMS_MAX)
#define AO_HDMI_MMZSIZE_MAX   ((192000*8*sizeof(mt_u32)/1000)*AOE_AOP_BUFF_LATENCYMS_MAX)
#define AO_CAST_MMZSIZE_MAX   ((48000*2*sizeof(mt_u16)/1000)*AOE_CAST_BUFF_LATENCYMS_MAX)

#ifdef CONFIG_MT_CHIP_ARIA

#define AUD_SAMPLE_EF  28922  //28922 = 2^33 / 297000

#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)

#define AUD_SAMPLE_EF  32809  //32809 = 2^33 * 1000 / 261818181

#elif defined(CONFIG_MT_CHIP_SYMPHONY4)  || defined(CONFIG_MT_CHIP_SYMPHONY6)

#if defined(CONFIG_MT_FPGA)
#define AUD_SAMPLE_EF  159073  //159073 = 2^33 * 1000 / 54000000
#else
#define AUD_SAMPLE_EF  32809  //32809 = 2^33 * 1000 / 261818181
#endif

#endif

static inline mt_u32 CALC_LATENCY_MS(mt_u32 Rate, mt_u32 FrameSize, mt_u32 Byte)
{
    if(Rate&&FrameSize)
    {
        return (1000 * Byte) / (Rate * FrameSize);
    }
    else
    {
        return 0;
    }
}

/* global function */
mt_s32                  HAL_AOE_Init(MT_BOOL bSwAoeFlag);
mt_void					HAL_AOE_DeInit(mt_void);

/* AIP function */
mt_s32					HAL_AOE_AIP_Create(AOE_AIP_ID_E *penAIP, AOE_AIP_CHN_ATTR_S *pstAttr);
mt_s32					HAL_AOE_AIP_CreateNew(AOE_AIP_ID_E *penAIP, AOE_AIP_CHN_ATTR_NEW_S *pstAttr);
mt_void					HAL_AOE_AIP_Destroy(AOE_AIP_ID_E enAIP);
mt_s32					HAL_AOE_AIP_SetAttr(AOE_AIP_ID_E enAIP, AOE_AIP_CHN_ATTR_S *pstAttr);
mt_s32					HAL_AOE_AIP_GetAttr(AOE_AIP_ID_E enAIP, AOE_AIP_CHN_ATTR_S *pstAttr);
mt_s32					HAL_AOE_AIP_Start(AOE_AIP_ID_E enAIP);
mt_s32					HAL_AOE_AIP_Stop(AOE_AIP_ID_E enAIP);
mt_s32                  HAL_AOE_AIP_Group_Stop(mt_u32 u32StopMask);                     //only used for master track type
mt_s32					HAL_AOE_AIP_Pause(AOE_AIP_ID_E enAIP);
mt_s32					HAL_AOE_AIP_Flush(mt_u32 Type);
mt_s32                  HAL_AOE_AIP_SetSpeed(AOE_AIP_ID_E enAIP, mt_s32 s32AdjSpeed);
mt_u32					HAL_AOE_AIP_WriteBufData(AOE_AIP_ID_E enAIP, mt_u8 * pu32Src, mt_u32 u32SrcBytes, mt_u32 ChanExist );
mt_u32					HAL_AOE_AIP_QueryBufData(AOE_AIP_ID_E enAIP);
mt_u32					HAL_AOE_AIP_QueryBufFree(AOE_AIP_ID_E enAIP);
void 					HAL_AOE_AIP_SetPcmSampleRate(mt_u32 samplerate, MT_BOOL b_x4);
mt_void					HAL_AOE_AIP_GetBufDelayMs(AOE_AIP_ID_E enAIP, mt_u32 *pDelayms);  // for aip buf delay
mt_void					HAL_AOE_AIP_GetFiFoDelayMs(AOE_AIP_ID_E enAIP, mt_u32 *pDelayms); // for aip fifo delay
mt_void					HAL_AOE_AIP_GetStatus(AOE_AIP_ID_E enAIP, AOE_AIP_STATUS_E *peStatus);
mt_s32  				HAL_AOE_AIP_SetLRVolume(AOE_AIP_ID_E enAIP, mt_u32 u32VolumeLdB, mt_u32 u32RVolumeRdB);
mt_s32                  HAL_AOE_AIP_SetMute(AOE_AIP_ID_E enAIP, MT_BOOL bMute);
mt_s32  				HAL_AOE_AIP_SetChannelMode(AOE_AIP_ID_E enAIP, mt_u32 u32ChannelMode);

/* AOP function */
mt_s32                  HAL_AOE_AOP_SetMute(AOE_AOP_ID_E enAOP, MT_BOOL bMute);
mt_s32                  HAL_AOE_AOP_SetLRVolume(AOE_AOP_ID_E enAOP, mt_u32 u32VolumeLdB, mt_u32 u32VolumeRdB);
mt_s32					HAL_AOE_AOP_Create(AOE_AOP_ID_E *penAOP, AOE_AOP_CHN_ATTR_S *pstAttr);
mt_void					HAL_AOE_AOP_Destroy(AOE_AOP_ID_E enAOP);
mt_s32					HAL_AOE_AOP_SetAttr(AOE_AOP_ID_E enAOP, AOE_AOP_CHN_ATTR_S *pstAttr);
mt_s32					HAL_AOE_AOP_GetAttr(AOE_AOP_ID_E enAOP, AOE_AOP_CHN_ATTR_S *pstAttr);
mt_s32					HAL_AOE_AOP_Start(AOE_AOP_ID_E enAOP);
mt_s32					HAL_AOE_AOP_Stop(AOE_AOP_ID_E enAOP);
mt_s32					HAL_AOE_AOP_GetStatus(AOE_AOP_ID_E enAOP, mt_void *pstStatus);

/* ENGINE function */
mt_s32					HAL_AOE_ENGINE_Create(AOE_ENGINE_ID_E *penENGINE, AOE_ENGINE_CHN_ATTR_S *pstAttr);
mt_void					HAL_AOE_ENGINE_Destroy(AOE_ENGINE_ID_E enENGINE);
mt_s32					HAL_AOE_ENGINE_SetAttr(AOE_ENGINE_ID_E enENGINE, AOE_ENGINE_CHN_ATTR_S *pstAttr);
mt_s32					HAL_AOE_ENGINE_GetAttr(AOE_ENGINE_ID_E enENGINE, AOE_ENGINE_CHN_ATTR_S *pstAttr);
mt_s32					HAL_AOE_ENGINE_Start(AOE_ENGINE_ID_E enENGINE);
mt_s32					HAL_AOE_ENGINE_Stop(AOE_ENGINE_ID_E enENGINE);
mt_s32					HAL_AOE_ENGINE_GetAttr(AOE_ENGINE_ID_E enENGINE, AOE_ENGINE_CHN_ATTR_S *pstAttr);
mt_s32					HAL_AOE_ENGINE_AttachAip(AOE_ENGINE_ID_E enENGINE, AOE_AIP_ID_E enAIP);
mt_s32					HAL_AOE_ENGINE_DetachAip(AOE_ENGINE_ID_E enENGINE, AOE_AIP_ID_E enAIP);
mt_s32					HAL_AOE_ENGINE_AttachAop(AOE_ENGINE_ID_E enENGINE, AOE_AOP_ID_E enAOP);
mt_s32					HAL_AOE_ENGINE_DetachAop(AOE_ENGINE_ID_E enENGINE, AOE_AOP_ID_E enAOP);
mt_s32                  HAL_AOE_ENGINE_AttachAef(AOE_ENGINE_ID_E enENGINE, mt_u32 u32AefId);
mt_s32                  HAL_AOE_ENGINE_DetachAef(AOE_ENGINE_ID_E enENGINE, mt_u32 u32AefId);
mt_s32                  HAL_AOE_AOP_SetAefBypass(AOE_AOP_ID_E enAOP, MT_BOOL bBypass);
mt_s32					HAL_AOE_ENGINE_GetStatus(AOE_ENGINE_ID_E enENGINE, mt_void *pstStatus);


//for ALSA
mt_void HAL_AOE_AIP_SetPeriodSize(AOE_AIP_ID_E enAIP, mt_u32 u32PeriodSize);     //NO USED
//mt_void HAL_AOE_AIP_EnableAlsa(AOE_AIP_ID_E enAIP, MT_BOOL bEnableAlsa);     //NO USED
mt_u32 HAL_AOE_AIP_UpdateWritePos(AOE_AIP_ID_E enAIP, mt_u32 *pu32WptrLen);
mt_u32 HAL_AOE_AIP_UpdateReadPos(AOE_AIP_ID_E enAIP, mt_u32 *pu32RptrLen);
mt_u32 HAL_AOE_AIP_FlushBuf(AOE_AIP_ID_E enAIP);
mt_u32 HAL_AOE_AIP_GetReadPos(AOE_AIP_ID_E enAIP, mt_u32 *pu32ReadPos);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif  // __MT_HAL_AOE_H__
