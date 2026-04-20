/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_avplay.h
  Version       : Initial Draft
  Author        : montage multimedia software group
  Created       : 2015/11/25
  Description   :
  History       :
  1.Date        : 2015/11/25
    Author      :
    Modification: Created file
********************************************************************************************/
#ifndef __MT_MPI_AVPLAY_H__
#define __MT_MPI_AVPLAY_H__

#include "mt_drv_avplay.h"
#include "drv_avplay_ioctl.h"

#ifdef __cplusplus
#if __cplusplus
	extern "C"
	{
#endif
#endif

mt_s32 MT_MPI_AVPLAY_Init(mt_void);
mt_s32 MT_MPI_AVPLAY_DeInit(mt_void);
mt_s32 MT_MPI_AVPLAY_GetDefaultConfig(MT_UNF_AVPLAY_ATTR_S *pstAvAttr, MT_UNF_AVPLAY_STREAM_TYPE_E enCfg);
mt_s32 MT_MPI_AVPLAY_RegisterAcodecLib(const mt_char *pFileName);
mt_s32 MT_MPI_AVPLAY_FoundSupportDeoder(const HA_FORMAT_E enFormat,mt_u32 * penDstCodecID);
mt_s32 MT_MPI_AVPLAY_ConfigAcodec( const mt_u32 enDstCodecID, mt_void *pstConfigStructure);
mt_s32 MT_MPI_AVPLAY_Create(const MT_UNF_AVPLAY_ATTR_S *pstAvAttr, mt_handle *phAvplay);
mt_s32 MT_MPI_AVPLAY_Destroy(mt_handle hAvplay);
mt_s32 MT_MPI_AVPLAY_ChnOpen(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const mt_void *pPara);
mt_s32 MT_MPI_AVPLAY_ChnClose(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn);
mt_s32 MT_MPI_AVPLAY_Reset_Buffer(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, MT_UNF_AVPLAY_MEDIA_BUF_E enBuf,const mt_void *pPara);
mt_s32 MT_MPI_AVPLAY_SetAttr(mt_handle hAvplay, MT_UNF_AVPLAY_ATTR_ID_E enAttrID, mt_void *pPara);
mt_s32 MT_MPI_AVPLAY_GetAttr(mt_handle hAvplay, MT_UNF_AVPLAY_ATTR_ID_E enAttrID, mt_void *pPara);
mt_s32 MT_MPI_AVPLAY_DecodeIFrame(mt_handle hAvplay, const MT_UNF_AVPLAY_I_FRAME_S *pstIframe, MT_UNF_VIDEO_FRAME_INFO_S *pstCapPicture);
mt_s32 MT_MPI_AVPLAY_ReleaseIFrame(mt_handle hAvplay, MT_UNF_VIDEO_FRAME_INFO_S *pstCapPicture);
mt_s32 MT_MPI_AVPLAY_SetDecodeMode(mt_handle hAvplay, MT_UNF_VCODEC_MODE_E enDecodeMode);
mt_s32 MT_MPI_AVPLAY_RegisterEvent(mt_handle      hAvplay,
                                   MT_UNF_AVPLAY_EVENT_E     enEvent,
                                   MT_UNF_AVPLAY_EVENT_CB_FN pfnEventCB);
mt_s32 MT_MPI_AVPLAY_UnRegisterEvent(mt_handle hAvplay, MT_UNF_AVPLAY_EVENT_E enEvent);
mt_s32 MT_MPI_AVPLAY_PreStart(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn);
mt_s32 MT_MPI_AVPLAY_Start(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn);
mt_s32 MT_MPI_AVPLAY_PreStop(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn,const MT_UNF_AVPLAY_PRESTOP_OPT_S *pPreStopOpt);
mt_s32 MT_MPI_AVPLAY_Stop(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const MT_UNF_AVPLAY_STOP_OPT_S *pStop);
mt_s32 MT_MPI_AVPLAY_Pause(mt_handle hAvplay, const MT_UNF_AVPLAY_PAUSE_OPT_S *pstPauseOpt);
mt_s32 MT_MPI_AVPLAY_Freeze(mt_handle hAvplay);
mt_s32 MT_MPI_AVPLAY_Tplay(mt_handle hAvplay, const MT_UNF_AVPLAY_TPLAY_OPT_S *pstTplayOpt);
mt_s32 MT_MPI_AVPLAY_SetTrickCfg(mt_handle hAvplay, MT_UNF_DEC_TRICK_PARAM_S *pstTrickParam);
mt_s32 MT_MPI_AVPLAY_DecFrmType(mt_handle hAvplay, MT_UNF_DEC_FRM_TYPE_E eDecFrmType);

mt_s32 MT_MPI_AVPLAY_Resume(mt_handle hAvplay);
mt_s32 MT_MPI_AVPLAY_Reset(mt_handle hAvplay, const MT_UNF_AVPLAY_RESET_OPT_S *pstResetOpt);
mt_s32 MT_MPI_AVPLAY_HRSeek(mt_handle hAvplay, const MT_UNF_AVPLAY_HRSEEK_OPT_S *pstSeekOpt);
mt_s32 MT_MPI_AVPLAY_Flush(mt_handle hAvplay);
mt_s32 MT_MPI_AVPLAY_Flush_Audio(mt_handle hAvplay);
mt_s32 MT_MPI_AVPLAY_GetFrame(mt_handle  hAvplay,
                            MT_UNF_AO_FRAMEINFO_S * p_ao_frame);
mt_s32 MT_MPI_AVPLAY_ReleaseFrame(mt_handle  hAvplay,
                            MT_UNF_AO_FRAMEINFO_S * p_ao_frame);
mt_s32 MT_MPI_AVPLAY_GetBuf(mt_handle  hAvplay,
                            MT_UNF_AVPLAY_BUFID_E enBufId,
                            mt_u32                u32ReqLen,
                            MT_UNF_STREAM_BUF_S  *pstData,
                            mt_u32                u32TimeOutMs);

mt_s32 MT_MPI_AVPLAY_PutBuf(mt_handle hAvplay, MT_UNF_AVPLAY_BUFID_E enBufId, mt_u32 u32ValidDataLen, mt_u64 u64Pts,
                            MT_UNF_AVPLAY_PUTBUFEX_OPT_S *pstExOpt, mt_u32 PtsValide, mt_u32 FrameFinsh, mt_u32 EosFlag);

mt_s32 MT_MPI_AVPLAY_GetSyncVdecHandle(mt_handle hAvplay, mt_handle *phVdec, mt_handle *phSync);
mt_s32 MT_MPI_AVPLAY_GetSndHandle(mt_handle hAvplay, mt_handle *phSnd);
mt_s32 MT_MPI_AVPLAY_GetWindowHandle(mt_handle hAvplay, mt_handle *phWindow);
mt_s32 MT_MPI_AVPLAY_AttachWindow(mt_handle hAvplay, mt_handle hWindow);
mt_s32 MT_MPI_AVPLAY_DetachWindow(mt_handle hAvplay, mt_handle hWindow);
mt_s32 MT_MPI_AVPLAY_SetWindowRepeat(mt_handle hAvplay, mt_u32 u32Repeat);
mt_s32 MT_MPI_AVPLAY_AttachSnd(mt_handle hAvplay, mt_handle hSnd);
mt_s32 MT_MPI_AVPLAY_DetachSnd(mt_handle hAvplay, mt_handle hSnd);
mt_s32 MT_MPI_AVPLAY_GetDmxAudChnHandle(mt_handle hAvplay, mt_handle *phDmxAudChn);
mt_s32 MT_MPI_AVPLAY_GetDmxVidChnHandle(mt_handle hAvplay, mt_handle *phDmxVidChn);
mt_s32 MT_MPI_AVPLAY_GetStatusInfo(mt_handle hAvplay, MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo);
mt_s32 MT_MPI_AVPLAY_GetCiTestInfo(mt_handle hAvplay, MT_UNF_AVPLAY_CI_TEST_INFO_S *pstInfo);
mt_s32 MT_MPI_AVPLAY_GetStreamInfo(mt_handle hAvplay, MT_UNF_AVPLAY_STREAM_INFO_S *pstStreamInfo);
mt_s32 MT_MPI_AVPLAY_GetAudioSpectrum(mt_handle hAvplay, mt_u16 *pSpectrum, mt_u32 u32BandNum);
mt_s32 MT_MPI_AVPLAY_IsBuffEmpty(mt_handle hAvplay, MT_BOOL *pbIsEmpty);
mt_s32 MT_MPI_AVPLAY_SetDDPTestMode(mt_handle hAvplay, MT_BOOL bEnable);
mt_s32 MT_MPI_AVPLAY_SwitchDmxAudChn(mt_handle hAvplay, mt_handle hNewDmxAud, mt_handle *phOldDmxAud);
mt_s32 MT_MPI_AVPLAY_PutAudPts(mt_handle hAvplay, mt_u32 u32AudPts);
mt_s32 MT_MPI_AVPLAY_FlushStream(mt_handle hAvplay, MT_UNF_AVPLAY_FLUSH_STREAM_OPT_S *pstFlushOpt);
mt_s32 MT_MPI_AVPLAY_Step(mt_handle hAvplay, const MT_UNF_AVPLAY_STEP_OPT_S *pstStepOpt);
mt_s32 MT_MPI_AVPLAY_Invoke(mt_handle hAvplay, MT_UNF_AVPLAY_INVOKE_E enInvokeType, mt_void *pPara);
mt_s32 MT_MPI_AVPLAY_AcqUserData(mt_handle hAvplay, MT_UNF_VIDEO_USERDATA_S *pstUserData, MT_UNF_VIDEO_USERDATA_TYPE_E *penType);
mt_s32 MT_MPI_AVPLAY_RlsUserData(mt_handle hAvplay, MT_UNF_VIDEO_USERDATA_S* pstUserData);
mt_s32 MT_MPI_AVPLAY_RstUserDataBuffer(mt_handle hAvplay);

mt_s32 MT_MPI_AVPLAY_GetVidChnOpenParam(mt_handle hAvplay, MT_UNF_AVPLAY_OPEN_OPT_S *pstOpenPara);

mt_s32 MT_MPI_AVPLAY_UseExternalBuffer(mt_handle hAvplay, mt_handle* phBuffers, mt_u32 u32Cnt, mt_u32 u32Size);
mt_s32 MT_MPI_AVPLAY_DeleteExternalBuffer(mt_handle hAvplay, mt_handle* phBuffers, mt_u32 u32Cnt);
mt_s32 MT_MPI_AVPLAY_CalculateFRC(mt_handle hAvplay, MT_UNF_VIDEO_FRAME_INFO_S* pstFrame,
        mt_u32 u32RefreshRate, mt_s32* ps32RepeatCnt);

mt_s32 MT_MPI_AVPLAY_SetVOBufferClearnComplete(AVPLAY_S *pAvplay, MT_BOOL bVOClearnFlag);

mt_s32 MT_MPI_AVPLAY_GetAudioStatusInfo(mt_handle hAvplay, MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo);
mt_s32 MT_MPI_AVPLAY_GetVideoStatusInfo(mt_handle hAvplay, MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo);
mt_s32 MT_MPI_AVPLAY_GetSyncStatusInfo(mt_handle hAvplay, MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo);

mt_s32 MT_MPI_AVPLAY_GetDebugInfo(mt_handle hAvplay, MT_UNF_AVPLAY_DEBUG_INFO_S *pstDebugInfo);
mt_s32 MT_MPI_AVPLAY_AudioTrick(mt_handle hAvplay, mt_void *ptrick);
mt_s32 MT_MPI_AVPLAY_IsNormaPlay(mt_handle hAvplay);

mt_s32 MT_MPI_AVPLAY_EnableAVOutInfo(mt_handle hAvplay, MT_BOOL bEnable);
mt_s32 MT_MPI_AVPLAY_GetAVOutInfo(mt_handle hAvplay, mt_void *pAVOutInfo);
mt_s32 MT_MPI_AVPLAY_StopAudDec(mt_handle hAvplay);
mt_s32 MT_MPI_AVPLAY_StartAudDec(mt_handle hAvplay);
mt_s32 MT_MPI_AVPLAY_AudioTrack(mt_handle hAvplay, mt_void *ptrack);
mt_s32 MT_MPI_AVPLAY_SetAVsyncMode(mt_handle hAvplay, MT_UNF_SYNC_REF_E enAVsyncMode);
mt_s32 MT_MPI_AVPLAY_Enable_AudHEAAC(mt_handle hAvplay);
mt_s32 MT_MPI_AVPLAY_set_downmix_enable(mt_handle hAvplay, u32 enable);
mt_s32 MT_MPI_AVPLAY_TrickSeekIn(mt_handle hAvplay);
mt_s32 MT_MPI_AVPLAY_TrickSeekOut(mt_handle hAvplay);
mt_s32 MT_MPI_AVPLAY_SW_ADEC_ENABLE(mt_handle hAvplay);
mt_s32 MT_MPI_AVPLAY_GetVideoESPhyAddrr(mt_handle hAvplay, phys_addr_t *esBuffPhyAddr, mt_u32 *esBuffSize);
mt_s32 MT_MPI_AVPLAY_ListAllPlayer(MT_UNF_AVPLAY_PLAYERINFO_S * info);
mt_s32 MT_MPI_AVPLAY_GetMetaInfo(mt_handle hAvplay, MT_UNF_AVPLAY_METARINFO_S *pMetaInfo);
mt_s32 MT_MPI_AVPLAY_SetPos(mt_handle hAvplay, float x, float y, float z);
mt_s32 MT_MPI_AVPLAY_SelObj(mt_handle hAvplay, mt_u16 id, mt_u16 on);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
