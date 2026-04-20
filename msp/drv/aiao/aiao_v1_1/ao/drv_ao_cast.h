/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_AO_CAST_H__
#define __MT_AO_CAST_H__

#include "mt_unf_sound.h"
#include "mt_drv_ao.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/
#define AO_CAST_DEFATTR_FRAMEMAXNUM  8
#define AO_CAST_DEFATTR_SAMPLESPERFRAME  1024
#define AO_CAST_DEFATTR_CHANNEL  2
#define AO_CAST_DEFATTR_BITSPERSAMPLE   16
#define AO_CAST_DEFATTR_SAMPLERATE  48000


/******************************Snd Cast process FUNC*************************************/

typedef enum
{
    SND_CAST_STATUS_STOP = 0,
    SND_CAST_STATUS_START,
    SND_CAST_STATUS_PAUSE,
    SND_CAST_STATUS_BUTT,
} SND_CAST_STATUS_E;
 
typedef struct
{
    MT_UNF_SND_ABSGAIN_ATTR_S  stCastAbsGain;  // L/R Gain
    MT_BOOL bMute;
    MT_UNF_SND_CAST_ATTR_S stUserCastAttr;
    ulong  u32UserVirtAddr;
    ulong  u32KernelVirtAddr;	//for kernel use cast
    phys_addr_t  u32PhyAddr;
    MT_BOOL bUserEnableSetting;

    mt_u32                  u32Channels;
    mt_u32                  u32SampleRate;
    mt_s32                  s32BitPerSample;
    /* internal state */
    mt_u32                  hCast;
    mt_u32                  CastId;
    mt_u32                  u32FrameBytes;
    mt_u32                  u32FrameSamples;
    mt_u32                  u32SampleBytes;
    
    MT_BOOL                 bAcquireCastFrameFlag;
    SND_CAST_STATUS_E      enCurnStatus;

} SND_CAST_STATE_S;


mt_s32 CAST_GetDefAttr(MT_UNF_SND_CAST_ATTR_S * pstDefAttr);
mt_s32 CAST_CreateNew(SND_CARD_STATE_S *pCard, MT_UNF_SND_CAST_ATTR_S *pstCastAttr, mmz_buffer_s *pstMMz, mt_u32 hCast);
mt_s32 CAST_DestroyCast(SND_CARD_STATE_S *pCard, mt_u32 hCast);
mt_s32 CAST_SetInfo(SND_CARD_STATE_S *pCard, mt_u32 u32CastID, mt_u32 u32UserVirtAddr);
mt_s32 CAST_GetInfo(SND_CARD_STATE_S *pCard, mt_u32 hCast, AO_Cast_Info_Param_S *pstInfo);
mt_s32 CAST_SetEnable(SND_CARD_STATE_S *pCard, mt_u32 u32CastID, MT_BOOL bEnable);
mt_s32 CAST_GetEnable(SND_CARD_STATE_S *pCard, mt_u32 hCast, MT_BOOL *pbEnable);
mt_s32 CAST_ReadData(SND_CARD_STATE_S *pCard, mt_u32 u32CastId, AO_Cast_Data_Param_S *pstCastData);
mt_s32 CAST_ReleaseData(SND_CARD_STATE_S *pCard, mt_u32 u32CastId,  AO_Cast_Data_Param_S *pstCastData);
mt_void CAST_GetSettings(SND_CARD_STATE_S *pCard, mt_handle hCast, SND_CAST_SETTINGS_S* pstCastSettings);
mt_void CAST_RestoreSettings(SND_CARD_STATE_S *pCard, mt_handle hCast, SND_CAST_SETTINGS_S* pstCastSettings);

mt_s32 CAST_GetAbsGain(SND_CARD_STATE_S *pCard, mt_u32 u32CastID, MT_UNF_SND_ABSGAIN_ATTR_S *pstCastAbsGain);
mt_s32 CAST_SetAbsGain(SND_CARD_STATE_S *pCard, mt_u32 u32CastID, MT_UNF_SND_ABSGAIN_ATTR_S *pstTrackAbsGain);
mt_s32 CAST_SetMute(SND_CARD_STATE_S *pCard, mt_u32 u32CastID, MT_BOOL bMute);
mt_s32 CAST_GetMute(SND_CARD_STATE_S *pCard, mt_u32 u32CastID, MT_BOOL *pbMute);
mt_void CAST_ReadProc(struct seq_file* p, SND_CARD_STATE_S *pCard);

   
#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif  // __MT_AO_TRACK_H__
