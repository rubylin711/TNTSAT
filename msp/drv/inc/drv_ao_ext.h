/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_DRV_AO_EXT_H__
#define __MT_DRV_AO_EXT_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include "mt_drv_dev.h"
#include "mt_drv_ao.h"
#include "drv_ao_ioctl.h"

typedef mt_s32 (*FN_AO_DrvResume)(basedev_s *);
typedef mt_s32 (*FN_AO_DrvSuspend)(basedev_s * , pm_message_t);
//for voip
typedef mt_s32  (*FN_AO_TrackGetDefAttr)(MT_UNF_AUDIOTRACK_ATTR_S * );
typedef mt_s32  (*FN_AO_TrackAllocHandle)(mt_handle * , struct file * , AO_Track_Create_Param_S_PTR);
typedef mt_void (*FN_AO_TrackFreeHandle)(mt_handle);
typedef mt_s32	(*FN_AO_TrackCreate)(MT_UNF_SND_E , MT_UNF_AUDIOTRACK_ATTR_S * , MT_BOOL , AO_BUF_ATTR_S * , mt_handle );
typedef mt_s32	(*FN_AO_TrackStart)(mt_u32);
typedef mt_s32	(*FN_AO_TrackStop)(mt_u32);
typedef mt_s32	(*FN_AO_TrackDestory)(mt_u32);
typedef mt_s32	(*FN_AO_TrackSendData)(mt_u32 , MT_UNF_AO_FRAMEINFO_S *);


typedef struct
{
	FN_AO_DrvResume  pfnAO_DrvResume;
	FN_AO_DrvSuspend pfnAO_DrvSuspend;

	FN_AO_TrackGetDefAttr  pfnAO_TrackGetDefAttr;
	FN_AO_TrackAllocHandle pfnAO_TrackAllocHandle;
	FN_AO_TrackFreeHandle  pfnAO_TrackFreeHandle;
	FN_AO_TrackCreate      pfnAO_TrackCreate;
	FN_AO_TrackDestory     pfnAO_TrackDestory;
	FN_AO_TrackStart       pfnAO_TrackStart;
	FN_AO_TrackStop        pfnAO_TrackStop;
	FN_AO_TrackSendData    pfnAO_TrackSendData;


} AIAO_EXPORT_FUNC_S;

mt_s32 AIAO_DRV_ModInit(mt_void);
mt_void AIAO_DRV_ModExit(mt_void);

#if defined(MT_MCE_SUPPORT)
mt_s32  MT_DRV_AO_Init(mt_void);
mt_void MT_DRV_AO_DeInit(mt_void);
mt_s32  MT_DRV_AO_SND_Init(struct file  *pfile);
mt_s32  MT_DRV_AO_SND_DeInit(struct file  *pfile);
mt_s32  MT_DRV_AO_SND_GetDefaultOpenAttr(MT_UNF_SND_E enSound, MT_UNF_SND_ATTR_S *pstAttr);
mt_s32  MT_DRV_AO_SND_Open(MT_UNF_SND_E enSound, MT_UNF_SND_ATTR_S *pstAttr, struct file *pfile);
mt_s32  MT_DRV_AO_SND_Close(MT_UNF_SND_E enSound, struct file  *pfile);
mt_s32  MT_DRV_AO_SND_SetVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_GAIN_ATTR_S stGain);
mt_s32  MT_DRV_AO_Track_GetDefaultOpenAttr(MT_UNF_SND_TRACK_TYPE_E enTrackType, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr);
mt_s32  MT_DRV_AO_Track_Create(MT_UNF_SND_E enSound, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr, MT_BOOL bAlsaTrack, struct file *pfile, mt_handle *phTrack);
mt_s32  MT_DRV_AO_Track_Destroy(mt_handle hSndTrack);
mt_s32  MT_DRV_AO_Track_Flush(mt_handle hSndTrack);
mt_s32  MT_DRV_AO_Track_Start(mt_handle hSndTrack);
mt_s32  MT_DRV_AO_Track_Stop(mt_handle hSndTrack);
mt_s32  MT_DRV_AO_Track_GetDelayMs(mt_handle hSndTrack, mt_u32 *pDelayMs);
mt_s32  MT_DRV_AO_Track_SendData(mt_handle hSndTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame);
mt_s32  MT_DRV_AO_Track_AttachAi(mt_handle hSndTrack, mt_handle hAi);
mt_s32  MT_DRV_AO_Track_DetachAi(mt_handle hSndTrack, mt_handle hAi);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif//__MT_DRV_AO_EXT_H__
