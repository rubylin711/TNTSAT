/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_ao.h
  Version       : Initial Draft
  Author        : montage multimedia software group
  Created       : 2015/11/25
  Description   :
  History       :
  1.Date        : 2015/11/25
    Author      :
    Modification: Created file
********************************************************************************************/

#ifndef  __MPI_AO_H__
#define  __MPI_AO_H__

#include "mt_type.h"
#include "mt_unf_sound.h"
#include "mt_unf_avplay.h"
#include "snd/snd_api.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

 /* the type of Adjust Audio */
typedef enum mtMT_MPI_SND_SPEEDADJUST_TYPE_E
{
 MT_MPI_AO_SND_SPEEDADJUST_SRC,     /**<samplerate convert */
 MT_MPI_AO_SND_SPEEDADJUST_PITCH,   /**<Sola speedadjust, reversed */
 MT_MPI_AO_SND_SPEEDADJUST_MUTE,    /**<mute */
 MT_MPI_AO_SND_SPEEDADJUST_BUTT
} MT_MPI_SND_SPEEDADJUST_TYPE_E;

/******************************* MPI for UNF Sound Init *****************************/
mt_s32   MT_MPI_AO_Init(mt_void);
mt_s32   MT_MPI_AO_DeInit(mt_void);

/******************************* MPI for UNF_SND *****************************/
mt_s32   MT_MPI_AO_SND_GetDefaultOpenAttr(MT_UNF_SND_E enSound, MT_UNF_SND_ATTR_S *pstAttr);
mt_s32   MT_MPI_AO_SND_Open(MT_UNF_SND_E enSound, const MT_UNF_SND_ATTR_S *pstAttr);
mt_s32   MT_MPI_AO_SND_Close(MT_UNF_SND_E enSound);
mt_s32   MT_MPI_AO_SND_SetMute(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bMute);
mt_s32   MT_MPI_AO_SND_GetMute(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbMute);
mt_s32   MT_MPI_AO_SND_SetHdmiMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                              MT_UNF_SND_HDMI_MODE_E enHdmiMode);
mt_s32   MT_MPI_AO_SND_GetHdmiMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                              MT_UNF_SND_HDMI_MODE_E *penHdmiMode);
mt_s32   MT_MPI_AO_SND_SetSpdifMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                              MT_UNF_SND_SPDIF_MODE_E enSpdifMode);
mt_s32   MT_MPI_AO_SND_GetSpdifMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                              MT_UNF_SND_SPDIF_MODE_E *penSpdifMode);
mt_s32 MT_MPI_AO_SND_SetVolume(ulong volume);
mt_s32 MT_MPI_AO_SND_GetVolume(ulong *volume);
mt_s32   MT_MPI_AO_SND_SetSpdifCategoryCode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                              MT_UNF_SND_SPDIF_CATEGORYCODE_E enSpdifCategoryCode);
mt_s32   MT_MPI_AO_SND_GetSpdifCategoryCode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                              MT_UNF_SND_SPDIF_CATEGORYCODE_E *penSpdifCategoryCode);
mt_s32   MT_MPI_AO_SND_SetSpdifSCMSMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                              MT_UNF_SND_SPDIF_SCMSMODE_E enSpdifSCMSMode);
mt_s32   MT_MPI_AO_SND_GetSpdifSCMSMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                              MT_UNF_SND_SPDIF_SCMSMODE_E *enSpdifSCMSMode);
mt_s32   MT_MPI_AO_SND_SetSampleRate(MT_UNF_SND_E enSound, MT_UNF_SAMPLE_RATE_E enSampleRate);
mt_s32   MT_MPI_AO_SND_GetSampleRate(MT_UNF_SND_E enSound, MT_UNF_SAMPLE_RATE_E *penSampleRate);
mt_s32   MT_MPI_AO_SND_SetTrackMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_TRACK_MODE_E enMode);
mt_s32   MT_MPI_AO_SND_GetTrackMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                    MT_UNF_TRACK_MODE_E *penMode);
mt_s32   MT_MPI_AO_AllTrack_SetMute(MT_UNF_SND_E enSound, MT_BOOL bMute);
mt_s32   MT_MPI_AO_AllTrack_GetMute(MT_UNF_SND_E enSound, MT_BOOL *pbMute);

mt_s32   MT_MPI_AO_SND_GetCastDefaultOpenAttr(MT_UNF_SND_CAST_ATTR_S *pstAttr);
mt_s32   MT_MPI_AO_SND_CreateCast(MT_UNF_SND_E enSound, MT_UNF_SND_CAST_ATTR_S *pstCastAttr, mt_handle *phCast);
mt_s32   MT_MPI_AO_SND_DestroyCast(mt_handle hCast);
mt_s32   MT_MPI_AO_SND_SetCastEnable(mt_handle hCast, MT_BOOL bEnable);
mt_s32   MT_MPI_AO_SND_GetCastEnable(mt_handle hCast, MT_BOOL *pbEnable);
mt_s32   MT_MPI_AO_SND_AcquireCastFrame(mt_handle hCast, MT_UNF_AO_FRAMEINFO_S *pstCastFrame);
mt_s32   MT_MPI_AO_SND_ReleaseCastFrame(mt_handle hCast, MT_UNF_AO_FRAMEINFO_S *pstCastFrame);
mt_s32   MT_MPI_AO_Cast_SetMute(mt_handle hCast, MT_BOOL bMute);
mt_s32   MT_MPI_AO_Cast_GetMute(mt_handle hCast, MT_BOOL *pbMute);
mt_s32   MT_MPI_AO_SND_SetCastAbsWeight (mt_handle hCast, const MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain);
mt_s32   MT_MPI_AO_SND_GetCastAbsWeight(mt_handle hCast, MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain);

mt_s32   MT_MPI_AO_SND_AttachAef(MT_UNF_SND_E enSound, mt_u32 u32AefId, mt_u32 *pu32AefProcAddr);
mt_s32   MT_MPI_AO_SND_DetachAef(MT_UNF_SND_E enSound, mt_u32 u32AefId);
mt_s32   MT_MPI_AO_SND_SetAefBypass(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bBypass);
mt_s32   MT_MPI_AO_SND_GetAefBypass(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbBypass);
mt_s32   MT_MPI_AO_SND_GetXrunCount(MT_UNF_SND_E enSound,mt_u32 *pu32Count);
//mt_s32   MT_MPI_AO_SND_AttachTrack(MT_UNF_SND_E enSound, mt_handle hTrack);   //MT_MPI_AO_TRACK_AttachSnd
//mt_s32   MT_MPI_AO_SND_DetachTrack(MT_UNF_SND_E enSound, mt_handle hTrack);

/******************************* MPI Track for UNF_SND/UNF_Mixer *****************************/
mt_s32   MT_MPI_AO_Track_GetDefaultOpenAttr(MT_UNF_SND_TRACK_TYPE_E enTrackType, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr);
mt_s32   MT_MPI_AO_Track_GetAttr(mt_handle hTrack, MT_UNF_AUDIOTRACK_ATTR_S    *pstAttr);
mt_s32   MT_MPI_AO_Track_SetAttr(mt_handle hTrack, const MT_UNF_AUDIOTRACK_ATTR_S    *pstAttr);
mt_s32   MT_MPI_AO_Track_Create(MT_UNF_SND_E enSound, const MT_UNF_AUDIOTRACK_ATTR_S *pstAttr, mt_handle *phTrack);
mt_s32   MT_MPI_AO_Track_Destroy(mt_handle hTrack);
mt_s32   MT_MPI_AO_Track_Start(mt_handle hTrack);
mt_s32   MT_MPI_AO_Track_Stop(mt_handle hTrack);
mt_s32   MT_MPI_AO_Track_Pause(mt_handle hTrack);
mt_s32   MT_MPI_AO_Track_Resume(mt_handle hTrack);
mt_s32   MT_MPI_AO_Track_Flush(mt_handle hTrack);
mt_s32   MT_MPI_AO_Track_SendData(mt_handle hTrack, const MT_UNF_AO_FRAMEINFO_S *pstAOFrame);
mt_s32   MT_MPI_AO_Track_SetWeight(mt_handle hTrack, const MT_UNF_SND_GAIN_ATTR_S *pstTrackGain);
mt_s32   MT_MPI_AO_Track_GetWeight(mt_handle hTrack, MT_UNF_SND_GAIN_ATTR_S* pstTrackGain);
//#define  MT_MPI_AO_Track_Resume MT_MPI_AO_Track_Start
mt_s32   MT_MPI_AO_Track_AttachAi(const mt_handle hAi, const mt_handle hTrack);
mt_s32   MT_MPI_AO_Track_DetachAi(const mt_handle hAi, const mt_handle hTrack);
mt_s32   MT_MPI_AO_Track_SetAbsWeight(mt_handle hTrack, const MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain);
mt_s32   MT_MPI_AO_Track_GetAbsWeight(mt_handle hTrack, MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain);
mt_s32   MT_MPI_AO_Track_SetMute(mt_handle hTrack, MT_BOOL bMute);
mt_s32   MT_MPI_AO_Track_GetMute(mt_handle hTrack, MT_BOOL *pbMute);
mt_s32   MT_MPI_AO_Track_SetSmartVolume(mt_handle hTrack, MT_BOOL bEnable);
mt_s32   MT_MPI_AO_Track_GetSmartVolume(mt_handle hTrack, MT_BOOL *pbEnable);
mt_s32   MT_MPI_AO_Track_GetCiTestInfo(mt_handle hTrack, MT_UNF_AVPLAY_CI_TEST_INFO_S *pInfo);

mt_s32   MT_MPI_AO_Track_SetChannelMode(mt_handle hTrack, MT_UNF_TRACK_MODE_E enMode);
mt_s32   MT_MPI_AO_Track_GetChannelMode(mt_handle hTrack, MT_UNF_TRACK_MODE_E *penMode);

// MT_UNF_SND_TRACK_TYPE_VIRTUAL only
mt_s32   MT_MPI_AO_Track_AcquireFrame(mt_handle hTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame, mt_u32 u32TimeoutMs);
mt_s32   MT_MPI_AO_Track_ReleaseFrame(mt_handle hTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame);

/******************************* MPI Track for MPI_AVPlay only **********************/
mt_s32   MT_MPI_AO_Track_SetEosFlag(mt_handle hTrack, MT_BOOL bEosFlag);
mt_s32   MT_MPI_AO_Track_SetSpeedAdjust(mt_handle hTrack, mt_s32 s32Speed, MT_MPI_SND_SPEEDADJUST_TYPE_E enType);
mt_s32   MT_MPI_AO_Track_GetDelayMs(const mt_handle hTrack, mt_u32 *pDelayMs);
mt_s32   MT_MPI_AO_Track_IsBufEmpty(const mt_handle hTrack, MT_BOOL *pbEmpty);
mt_s32  MT_MPI_AO_SND_SetRenderingRate(MT_UNF_SND_E enSound, mt_u32 rate);
mt_s32   MT_MPI_AO_SND_SetAdacOnOff(MT_UNF_SND_E enSound, MT_BOOL bOnOff);
mt_s32 MT_MPI_AO_SND_SetFaderAttr(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_FADER_ATTR_S *pstFader);
mt_s32 MT_MPI_AO_SND_GetBufInfo(MT_UNF_SND_E enSound, MT_UNF_SND_BUF_INFO_S *pstBufInfo);
mt_s32 MT_MPI_AO_SND_StartTTS(MT_UNF_SND_E enSound);
mt_s32 MT_MPI_AO_SND_SendTTS(MT_UNF_SND_E enSound, mt_u8 *data, mt_u32 size);
mt_s32 MT_MPI_AO_SND_StopTTS(MT_UNF_SND_E enSound);
#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */

#endif //__MPI_AO_H__
