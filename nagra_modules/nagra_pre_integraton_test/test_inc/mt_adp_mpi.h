#ifndef  _COMMON_MPI_H
#define  _COMMON_MPI_H

#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_vo.h"
#include "mt_adp.h"
#include "mt_adp_search.h"
#include "mt_adp_boardcfg.h"
#if defined(CONFIG_MT_CHIP_ARIA)
#include "mt_unf_ai.h"
#endif
#define DOLBYPLUS_HACODEC_SUPPORT
#if  defined (CHIP_TYPE_hi3716mv300_fpga)
 #define TLV320_AUDIO_DEVICE_ENABLE
#else
 #define SLIC_AUDIO_DEVICE_ENABLE
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if 0
mt_s32	MPI_VoInit();
mt_s32	MPI_VoDeInit();
mt_s32	MPI_SndInit(mt_void);
mt_s32	MPI_SndDeInit(mt_void);
mt_s32	MPI_DispInit(mt_void);
mt_s32	MPI_DispDeInit(mt_void);
mt_s32	MPI_AVPlayInit(mt_void);
mt_s32	MPI_AVPlayDeInit(mt_void);
mt_s32	MPI_AVPlayStart(mt_handle hAvplay, DB_PROGRAM_S *psProg);
mt_s32	MPI_VoDestroyWin(mt_handle hWin);
mt_s32	MPI_VoCreateWin(mt_handle *phWin, mt_u8 u8WinNum);
mt_s32   MPI_SetVideoAttr(mt_handle hAvplay, mt_u32 pid, MT_UNF_VCODEC_TYPE_E videotype);
mt_s32   MPI_SetAudioAttr(mt_handle hAvplay, mt_u32 pid, mt_u32 audiotype);
#else
/* **********************************Demux  Common Interface********************************/
mt_s32 MTADP_Demux_Init(mt_u32 DmxPortID,mt_u32 TsPortID);

mt_s32 MTADP_Demux_DeInit(mt_u32 DmxPortID);

/************************************DISPLAY  Common Interface*******************************/
mt_s32 MTADP_Disp_StrToFmt(mt_char *pszFmt);

mt_s32 MTADP_Disp_Init(MT_UNF_ENC_FMT_E enFormat);

mt_s32 MTADP_Disp_DeInit(mt_void);

/****************************VO  Common Interface********************************************/
mt_s32 MTADP_VO_Init(MT_UNF_VO_DEV_MODE_E enDevMode);

mt_s32 MTADP_VO_CreatWin(mt_rect_s * pstWinRect, mt_handle * phWin);

mt_s32 MTADP_VO_CreatWinExt(mt_rect_s * pstWinRect, mt_handle * phWin, MT_BOOL bVirtScreen);

mt_s32 MTADP_VO_DeInit(mt_void);

/*****************************************SOUND  Common Interface************************************/
mt_s32 MTADP_Snd_Init(mt_void);


mt_s32 MTADP_Snd_DeInit(mt_void);

/*Only Support Single AI Chn*/
#if defined(CONFIG_MT_CHIP_ARIA)
mt_s32 MTADP_AI_Init(MT_UNF_AI_E enAISrc, mt_handle *pAIHandle, mt_handle *pTrackSlave, mt_handle *pATrackVir);

mt_s32 MTADP_AI_DeInit(mt_handle hAI, mt_handle hAISlave, mt_handle hAIVir);
#endif
/*****************************************AIAO  Common Interface************************************/
mt_s32 MTADP_AIAO_Init(mt_s32 DevId, mt_s32 AI_Ch, mt_s32 AO_Ch, MT_UNF_SAMPLE_RATE_E enSamplerate, mt_u32 u32SamplePerFrame);


mt_s32 MTADP_AIAO_DeInit(mt_void);

mt_s32 MTADP_SLIC_Open(mt_void);
mt_s32 MTADP_SLIC_Close(mt_void);
mt_s32 MTADP_SLIC_GetHookOff(MT_BOOL *pbEnable);
mt_s32 MTADP_SLIC_GetHookOn(MT_BOOL *pbEnable);
mt_s32 MTADP_SLIC_SetRinging(MT_BOOL bEnable);


/**************************************AVPLAY  Common Interface***************************************/
mt_s32 MTADP_AVPlay_RegADecLib(mt_void);

mt_s32 MTADP_AVPlay_Init(mt_void);

mt_s32 MTADP_AVPlay_Create(mt_handle *avplay,mt_u32 u32DemuxId,
                                 MT_UNF_AVPLAY_STREAM_TYPE_E streamtype,
                                 MT_UNF_VCODEC_CAP_LEVEL_E vdeccap,
                                 mt_u32 channelflag);

mt_s32 MTADP_AVPlay_SetVdecAttr(mt_handle hAvplay,MT_UNF_VCODEC_TYPE_E enType,MT_UNF_VCODEC_MODE_E enMode);

mt_s32 MTADP_AVPlay_SetAdecAttr(mt_handle hAvplay,mt_u32 enADecType,MT_HA_DECODEMODE_E enMode, mt_s32 isCoreOnly);

mt_s32 MTADP_AVPlay_PlayProg(mt_handle hAvplay,PMT_COMPACT_TBL *pProgTbl,mt_u32 ProgNum,MT_BOOL bAudPlay);

mt_s32 MTADP_AVPlay_PlayAud(mt_handle hAvplay,PMT_COMPACT_TBL *pProgTbl,mt_u32 ProgNum);

mt_s32 MTADP_AVPlay_SwitchAud(mt_handle hAvplay,mt_u32 AudPid, mt_u32 u32AudType);

mt_s32 MTADP_AENC_GetAttr(MT_UNF_AENC_ATTR_S *pAencAttr, mt_void *pstConfig);

mt_s32 MTADP_MCE_Exit(mt_void);

mt_s32 MTADP_DMX_AttachTSPort(mt_u32 Dmxid, mt_u32 TunerID);

mt_s32 MTADP_Show_LOGO( MT_UNF_VCODEC_TYPE_E VdecType, mt_void * p_data,mt_u32 datalen);
mt_s32 MTADP_Clear_LOGO(mt_void);
#endif

#ifdef __cplusplus
}
#endif

#endif
