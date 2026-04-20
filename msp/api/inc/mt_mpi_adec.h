/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
 File Name     : mt_mpi_adec.h
 Version       : Initial Draft
 Author        : Montage multimedia software group
 Created       :
 Last Modified :
 Description   :
 Function List :
 History       :
******************************************************************************/
#ifndef __MT_MPI_ADEC_H__
#define __MT_MPI_ADEC_H__

#include "mt_type.h"
#include "mt_unf_sound.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

typedef enum mtMPI_ADEC_INFO_E
{
	//STATUSINFO
	MT_MPI_ADEC_STATUSINFO = 0,
	//DebugInfo
	MT_MPI_ADEC_DEBUGINFO,
	//GetStreamInfo
	MT_MPI_ADEC_STREAMINFO,
	//BufferStatus
	MT_MPI_ADEC_BUFFERSTATUS,
	//HaSzNameInfo
	MT_MPI_ADEC_HaSzNameInfo,
	MT_MPI_ADEC_METAINFO
}MT_MPI_ADEC_INFO_E;

typedef struct hiADEC_ATTR_S
{
    MT_BOOL              bEnable;                        /* Enable ADEC module 			*/
    MT_BOOL              bEosState;                        /* Eos Flag*/
    mt_u32               u32CodecID;                 /* Codec  type                         */
    mt_u32               u32InBufSize;               /* Input buffer  size             */
    mt_u32               u32OutBufNum;             /* Output buffer number, buffer size depend on  u32CodecID       */
    mt_u32          AudErrorCheckContinuationNum;
    MT_HADECODE_OPENPARAM_S sOpenPram;   /* Decoder open param */
} ADEC_ATTR_S;

typedef struct hiADEC_BUFSTATUS_S
{
    mt_u32 u32BufferSize;            /* Total buffer size, in the unit of byte.*/
    mt_u32 u32BufferAvailable;   /* Available buffer, in the unit of byte.*/
    mt_u32 u32BufferUsed;          /* Used buffer, in the unit of byte.*/
    mt_s32 s32BufReadPos;               /*buffer s32BufReadPos ptr*/
    mt_u32 u32BufWritePos;              /*buffer u32BufWritePos ptr*/
    mt_u32 u32TotDecodeFrame;          /* Total Deocded Frame number.*/
    MT_BOOL bEndOfFrame;          /*EOS flag*/
    MT_BOOL bForceBuffReset;      /*Adec es buffer force reset flag*/
} ADEC_BUFSTATUS_S, *PTR_ADEC_BUFSTATUS_S;

/* Audio output information structure                                                   */
typedef struct hiADEC_STATUSINFO_S
{
    MT_BOOL bWorking;

    /* OutBuf Status */
    mt_u32  u32OutBufNum;
    mt_u32  u32UsedBufNum;
    mt_u32  u32OutBufDurationMs;

    /* InBuf Status */
    mt_u32 u32BufferSize;            /* Total buffer size, in the unit of byte.*/
    mt_u32 u32BufferAvailable;   /* Available buffer, in the unit of byte.*/
    mt_u32 u32BufferUsed;          /* Used buffer, in the unit of byte.*/


    /* statistical Status */
    mt_u32  u32TotDecodeFrame;          /* Total Deocded Frame number.*/
    mt_u32  u32FrameDurationMs;
    mt_u32  u32CodecID;
    MT_BOOL enFmt;        
    mt_u32  u32Channels;
    MT_UNF_SAMPLE_RATE_E enSampleRate;
    MT_UNF_BIT_DEPTH_E     enBitDepth;
    	
} ADEC_STATUSINFO_S;

/* Audio output debug information structure                    */
typedef struct hiADEC_DEBUGINFO_S
{
    mt_u32 u32DecFrameNum;
    mt_u32 u32ErrDecFrameNum;
} ADEC_DEBUGINFO_S;

typedef struct hiADEC_SzNameINFO_S
{
    mt_char szHaCodecName[32];
} ADEC_SzNameINFO_S;


/* Outputting audio stream structure                                         */
typedef struct hiADEC_STREAMINFO_S
{
    mt_u32               u32CodecID;
    MT_UNF_SAMPLE_RATE_E enSampleRate;
} ADEC_STREAMINFO_S;


/* Outputting audio frame extend infomation                                         */
typedef struct hiADEC_EXTFRAMEINFO_S
{
    mt_u32               u32FrameLeftNum;             /* frame umber left at adef buffer */ 
    mt_u32               u32FrameDurationMs;         /* frame duration  */ 
    mt_u64               u64OrgPtsMs;
} ADEC_EXTFRAMEINFO_S;


typedef struct hiADEC_EVENT_S
{
    MT_BOOL  bFrameInfoChange;
    MT_BOOL  bUnSupportFormat;
    MT_BOOL  bStreamCorrupt;
    MT_BOOL  bnewAudioFrame;

    MT_UNF_ACODEC_STREAMINFO_S stStreamInfo;
	MT_UNF_AO_FRAMEINFO_S      AvplayAudFrm; 
} ADEC_EVENT_S;


mt_s32 MT_MPI_ADEC_RegisterDeoderLib(const mt_char *pszCodecDllName, mt_u32 u32Length);
mt_s32 MT_MPI_ADEC_FoundSupportDeoder(HA_FORMAT_E enFormat, mt_u32 *penDstCodecID);

mt_s32 MT_MPI_ADEC_SetConfigDeoder( const mt_u32 enDstCodecID, mt_void *pstConfigStructure);
mt_s32 MT_MPI_ADEC_Init(void);
mt_s32 MT_MPI_ADEC_deInit(void);
mt_s32 MT_MPI_ADEC_Open(mt_handle *phAdec);
mt_s32 MT_MPI_ADEC_Close (mt_handle hAdec);

mt_s32 MT_MPI_ADEC_Reset(mt_handle hAdec);
mt_s32 MT_MPI_ADEC_ResetBuf(mt_handle hAdec, mt_u32 u32BufType);
mt_s32 MT_MPI_ADEC_SetCodecId(mt_handle hAdec, mt_u32 * pstCodecId);
mt_s32 MT_MPI_ADEC_Start(mt_handle hAdec, mt_u32 aud_type);
mt_s32 MT_MPI_ADEC_Stop(mt_handle hAdec, mt_u32 reset);
mt_s32 MT_MPI_ADEC_SetInBufSize(mt_handle hAdec, mt_u32 * pstInBufSize);
mt_s32 MT_MPI_ADEC_SetOutBufNum(mt_handle hAdec, mt_u32 * pstOutBufNum);
mt_s32 MT_MPI_ADEC_SetDecOpenParam(mt_handle hAdec, mt_u32 * pstDecOpenParam);
mt_s32 MT_MPI_ADEC_SetAllAttr(mt_handle hAdec, ADEC_ATTR_S * pstAllAttr);
mt_s32 MT_MPI_ADEC_GetCodecId(mt_handle hAdec, mt_u32 * pstCodecId);
mt_s32 MT_MPI_ADEC_GetWorkState(mt_handle hAdec, MT_BOOL * pstWorkState);
mt_s32 MT_MPI_ADEC_GetInBufSize(mt_handle hAdec, mt_u32 * pstInBufSize);
mt_s32 MT_MPI_ADEC_GetOutBufNum(mt_handle hAdec, mt_u32 * pstOutBufNum);
mt_s32 MT_MPI_ADEC_GetDelayMs(mt_handle hAdec, mt_u64 *pDelay);
mt_s32 MT_MPI_ADEC_GetDecOpenParam(mt_handle hAdec, mt_u32 * pstDecOpenParam);
mt_s32 MT_MPI_ADEC_GetAllAttr(mt_handle hAdec, ADEC_ATTR_S * pstAllAttr);
mt_s32 MT_MPI_ADEC_SendStream (mt_handle hAdec, const MT_UNF_STREAM_BUF_S *pstStream, mt_u64 u64PtsMs);
mt_s32 MT_MPI_ADEC_GetBuffer(mt_handle hAdec, mt_u32 u32RequestSize, MT_UNF_STREAM_BUF_S *pstStream);
mt_s32 MT_MPI_ADEC_PutBuffer (mt_handle hAdec, const MT_UNF_STREAM_BUF_S *pstStream, mt_u64 u64PtsMs, mt_u32 PtsValide, MT_BOOL b_eos);
mt_s32 MT_MPI_ADEC_ReleaseFrame(mt_handle hAdec, const MT_UNF_AO_FRAMEINFO_S *pstAOFrame);
mt_s32 MT_MPI_ADEC_GetInfo(mt_handle hAdec, MT_MPI_ADEC_INFO_E enAdecInfo, void *pstadecinfo);
mt_s32 MT_MPI_ADEC_GetAudSpectrum(mt_handle hAdec, mt_u16 *pSpectrum ,mt_u32 u32BandNum);
mt_s32 MT_MPI_ADEC_Pull(mt_handle hAdec);
mt_s32 MT_MPI_ADEC_SetDDPTestMode(mt_handle hAdec, MT_BOOL bEnable);


mt_s32 MT_MPI_ADEC_ReceiveFrame (mt_handle hAdec, MT_UNF_AO_FRAMEINFO_S *pstAOFrame, ADEC_EXTFRAMEINFO_S *pstExtInfo);
mt_s32 MT_MPI_ADEC_SeekAudPts(mt_handle hAdec, mt_u32 u32AudPts);

mt_s32 MT_MPI_ADEC_SetEosFlag(mt_handle hAdec);

mt_s32 MT_MPI_ADEC_DropStream(mt_handle hAdec, mt_u64 u64SeekPts);

mt_s32 MT_MPI_ADEC_SetCodecCmd(mt_handle hAdec, mt_void *pstCodecCmd);
mt_s32 MT_MPI_ADEC_CheckNewEvent(mt_handle hAdec, ADEC_EVENT_S* pstNewEvent);

mt_s32 MT_MPI_ADEC_RegisterDeoder(const mt_char *pszCodecDllName);
mt_s32 MT_MPI_ADEC_Flush_Buf(mt_handle hAdec);
mt_s32 MT_MPI_ADEC_Reset_In_Buf(mt_handle hAdec);
mt_s32 MT_MPI_ADEC_Reset_Out_Buf(mt_handle hAdec);
mt_s32 MT_MPI_ADEC_SetDolbyDownmixMode(mt_handle hAdec, const mt_u32 *pDolbyDownmixMode);
mt_s32 MT_MPI_ADEC_AvcConfig(mt_handle hAdec,  MT_HADECODE_AVC_PARAM_S * pstAvcParam);
void MT_MPI_ADEC_Set_Pause (mt_handle hAdec, int st);
void ADEC_AD_PUT_PCM(mt_u8 *pcmdata,mt_u32 pcmlen);
mt_s32 ADEC_AD_SET_VOL_WEIGHT(mt_u32 *vol_weight);
mt_s32 ADEC_AD_GET_VOL_WEIGHT(mt_u32 *vol_weight);
void ADEC_AD_SET_DATATYPE(mt_handle hAdec, mt_u32 flag_es);
mt_u32 ADEC_AD_PCM_GETFREE(void);
mt_s32 MT_MPI_ADEC_Enable_HEAAC(mt_handle hAdec);
mt_s32 MT_MPI_ADEC_set_downmix_enable(mt_handle hAdec, mt_u32 enable);
mt_s32 MT_MPI_ADEC_Set_AC4_Downmix_Mode(mt_u32 *type);
mt_s32 MT_MPI_ADEC_Set_AC4_Dialogue_Enhancement(mt_u32 *val);
mt_s32 MT_MPI_ADEC_Set_AC4_Encode_DD_DDP(mt_u32 *encode_type);
mt_s32 MT_MPI_ADEC_Set_AC4_Encode_MAT(mt_u32 *enable);
mt_s32 MT_MPI_ADEC_Set_AC4_AD_OnOff(mt_u32 *enable);
mt_s32 MT_MPI_ADEC_Set_AC4_AD_Weight(mt_s32 *val);
mt_s32 MT_MPI_ADEC_Sel_AC4_AD_Type(mt_u32 *val);
mt_s32 MT_MPI_ADEC_Sel_AC4_AD_Type_Over_Lang(void);
mt_s32 MT_MPI_ADEC_Sel_AC4_Lang(MT_UNF_AVPLAY_AC4_LANG_S *ac4_lang);
mt_s32 MT_MPI_ADEC_Set_AC4_Pres_ID(mt_s32 *val);
mt_s32 MT_MPI_ADEC_Set_AC4_Encode_DAP(mt_u32 *encode_type);
mt_s32 MT_MPI_ADEC_Set_Dolby_Force_MS12_Dec(mt_u32 *enable);
mt_s32 MT_MPI_ADEC_Set_VIVID_Pos(float x, float y, float z);
mt_s32 MT_MPI_ADEC_Sel_VIVID_Obj(mt_u16 id, mt_u16 on);
mt_s32 MT_MPI_ADEC_set_trickmode(mt_handle hAdec, u32 mode);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MT_ADEC_H__ */
