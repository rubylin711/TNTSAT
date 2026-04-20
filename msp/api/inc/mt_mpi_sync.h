/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_sync.h
  Version       : Initial Draft
  Author        : montage multimedia software group
  Created       : 2015/11/25
  Description   :
  History       :
  1.Date        : 2015/11/25
    Author      : 
    Modification: Created file
********************************************************************************************/
#ifndef __MT_MPI_SYNC_H__
#define __MT_MPI_SYNC_H__

#include "mt_drv_sync.h"
#include "drv_sync_ioctl.h"

#ifdef __cplusplus
#if __cplusplus
	extern "C"
	{
#endif
#endif

mt_s32 MT_MPI_SYNC_Init(mt_void);
mt_s32 MT_MPI_SYNC_DeInit(mt_void);
mt_void MT_MPI_SYNC_GetDefaultAttr(MT_UNF_SYNC_ATTR_S *pstSyncAttr);
mt_s32 MT_MPI_SYNC_Create(MT_UNF_SYNC_ATTR_S *pstSyncAttr, mt_handle *phSync);
mt_s32 MT_MPI_SYNC_Destroy(mt_handle hSync);
mt_s32 MT_MPI_SYNC_SetAttr(mt_handle hSync, MT_UNF_SYNC_ATTR_S *pstSyncAttr);
mt_s32 MT_MPI_SYNC_GetAttr(mt_handle hSync, MT_UNF_SYNC_ATTR_S *pstSyncAttr);
mt_s32 MT_MPI_SYNC_Start(mt_handle hSync, SYNC_CHAN_E enChn);
mt_s32 MT_MPI_SYNC_Stop(mt_handle hSync, SYNC_CHAN_E enChn);
mt_s32 MT_MPI_SYNC_Play(mt_handle hSync);
mt_s32 MT_MPI_SYNC_Seek(mt_handle hSync, mt_u64 u64SeekPts);
mt_s32 MT_MPI_SYNC_Pause(mt_handle hSync);
mt_s32 MT_MPI_SYNC_Tplay(mt_handle hSync);
mt_s32 MT_MPI_SYNC_Resume(mt_handle hSync);
mt_s32 MT_MPI_SYNC_SetBufState(mt_handle hSync, SYNC_BUF_STATUS_S stBufStatus);
mt_s32 MT_MPI_SYNC_AudJudge(mt_handle hSync, SYNC_AUD_INFO_S *pAudInfo, SYNC_AUD_OPT_S *pAudOpt);
mt_s32 MT_MPI_SYNC_GetStatus(mt_handle hSync, MT_UNF_SYNC_STATUS_S *pSyncStatus);
mt_s32 MT_MPI_SYNC_SetDDPTestMode(mt_handle hSync, MT_BOOL bEnable);
mt_s32 MT_MPI_SYNC_CheckNewEvent(mt_handle hSync, SYNC_EVENT_S *pstEvent);

mt_s32 MT_MPI_SYNC_SetExtInfo(mt_handle hSync, SYNC_EXT_INFO_E enExtInfo, mt_void *pData);

mt_s32 MT_MPI_SYNC_VidJudge(mt_handle hSync, SYNC_VID_INFO_S *pVidInfo, SYNC_VID_OPT_S *pVidOpt);
mt_s32 MT_MPI_SYNC_Push_Apts(mt_handle hSync, SYNC_PUSH_APTS_S *SyncApts);
mt_s32 MT_MPI_SYNC_Push_Adec_Apts(mt_handle hSync, SYNC_PUSH_APTS_S *SyncApts);

mt_s32 MT_MPI_SYNC_Vid_Init(mt_handle hSync, mt_u32 vdec_fmt);
mt_s32 MT_MPI_SYNC_Aud_Init(mt_handle hSync, mt_u32 adec_fmt);

mt_s32 MT_MPI_SYNC_Vid_DeInit(mt_handle hSync);
mt_s32 MT_MPI_SYNC_Aud_DeInit(mt_handle hSync);

mt_s32 MT_MPI_SYNC_Debug(mt_handle hSync, mt_u32 debug, mt_u32 param);
mt_s32 MT_MPI_SYNC_AptsAdjust(mt_handle hSync, mt_u32 adjust);
mt_s32 MT_MPI_SYNC_Aud_Track(mt_handle hSync, mt_u32 *ptrick);
mt_s32 MT_MPI_SYNC_SetAVsyncMode(mt_handle hSync, MT_UNF_SYNC_REF_E enAVsyncMode);
mt_s32 MT_MPI_SYNC_GetAVSyncInfo(mt_handle hSync, MT_UNF_SYNC_AV_INFO_S *pSyncAVInfo);
mt_s32 MT_MPI_SYNC_GetVFrm_count(mt_handle hSync, mt_s32 *pcnt);
mt_s32 MT_MPI_SYNC_VFRM_INFO_CAP_Enable(mt_handle hSync, mt_u32 enable);
mt_s32 MT_MPI_SYNC_VFRM_INFO_CAP_Read(mt_handle hSync, SYNC_VOUT_FRAME_INFO *info);
mt_s32 MT_MPI_SYNC_Data_Source_Cfg(mt_handle hSync, MT_SYNC_DATA_SOURCE_E *pargs);
mt_s32 MT_MPI_SYNC_Play_Info_Cfg(mt_handle hSync, mt_u32 *pargs);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
