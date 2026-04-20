/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_AO_TRACK_H__
#define __MT_AO_TRACK_H__

#include "mt_unf_sound.h"
#include "mt_drv_ao.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/
#define AO_TRACK_LATENCYMS_DEFAULT 256
#define AO_TRACK_PCM_BUFSIZE_MS_MAX 512
#define AO_TRACK_PCM_BUFSIZE_MS_MIN 256
#define AO_TRACK_PCM_BUFSIZE_MS_DEF 256
#define AO_TRACK_LBR_BUFSIZE_MS_MAX 512
#define AO_TRACK_HBR_BUFSIZE_MS_MAX 512
#define AO_TRACK_BUF_EMPTY_THRESHOLD_MS 10 //aip 5ms + engine 5ms

#define AO_TRACK_PCM_BUFSIZE_BYTE_MAX (((192000 * 2 * sizeof(mt_u32)) / 1000) * AO_TRACK_PCM_BUFSIZE_MS_MAX)
#define AO_TRACK_PCM_BUFSIZE_BYTE_MIN (((48000 * 2 * sizeof(mt_u32)) / 1000) * AO_TRACK_PCM_BUFSIZE_MS_MIN)
//TODO...
//How about the default AO PCM buffer size???
#define AO_TRACK_PCM_BUFSIZE_BYTE_DEF (256 * 1024)//(((128000 * 2 * sizeof(mt_u32)) / 1000) * AO_TRACK_PCM_BUFSIZE_MS_DEF)
#define AO_TRACK_LBR_BUFSIZE_BYTE_MAX (((192000 * 2 * sizeof(mt_u32)) / 1000) * AO_TRACK_PCM_BUFSIZE_MS_MAX)
#define AO_TRACK_HBR_BUFSIZE_BYTE_MAX (((192000 * 8 * sizeof(mt_u32)) / 1000) * AO_TRACK_PCM_BUFSIZE_MS_MAX)

#define AO_TRACK_NORMAL_CHANNELNUM 8
#define AO_TRACK_MUTILPCM_CHANNELNUM 8
#define AO_TRACK_BITDEPTH_LOW 16
#define AO_TRACK_BITDEPTH_MTGH 24
#define AO_TRACK_DEFATTR_BUFSIZE 1024
#define AO_TRACK_MASTER_DEFATTR_BUFLEVELMS 400
#define AO_TRACK_MASTER_MIN_BUFLEVELMS 300    //8k,aac MaxPcmOutSampleSize:2048,  2048/8000 = 0.258s = 258ms
#define AO_TRACK_MASTER_MAX_BUFLEVELMS 800
#define AO_TRACK_MASTER_DEFATTR_FADEINMS 32
#define AO_TRACK_MASTER_DEFATTR_FADEOUTMS 8
#define AO_TRACK_MASTER_DEFATTR_BUFSIZE (4 * AO_TRACK_DEFATTR_BUFSIZE)
#define AO_TRACK_SLAVE_DEFATTR_BUFLEVELMS 400
#define AO_TRACK_SLAVE_DEFATTR_FADEINMS 1
#define AO_TRACK_SLAVE_DEFATTR_FADEOUTMS 1
#define AO_TRACK_SLAVE_DEFATTR_BUFSIZE (4 * AO_TRACK_DEFATTR_BUFSIZE)
#define AO_TRACK_VIRTUAL_DEFATTR_BUFSIZE (256 * AO_TRACK_DEFATTR_BUFSIZE)

#define AO_TRACK_PATH_NAME_MAXLEN 256
#define AO_TRACK_FILE_NAME_MAXLEN 256

typedef enum
{
    TRACK_STREAMMODE_CHANGE_NONE = 0,
    TRACK_STREAMMODE_CHANGE_PCM2PCM,
    TRACK_STREAMMODE_CHANGE_PCM2LBR,
    TRACK_STREAMMODE_CHANGE_PCM2HBR,
    TRACK_STREAMMODE_CHANGE_LBR2PCM,
    TRACK_STREAMMODE_CHANGE_LBR2LBR,
    TRACK_STREAMMODE_CHANGE_LBR2HBR,
    TRACK_STREAMMODE_CHANGE_HBR2PCM,
    TRACK_STREAMMODE_CHANGE_HBR2LBR,
    TRACK_STREAMMODE_CHANGE_HBR2HBR,
} TRACK_STREAMMODE_CHANGE_E;


/******************************Snd Track process FUNC*************************************/

/*Audio output attribute */
typedef struct
{
    mt_u32 u32ChannelExist;  
    mt_u32 u32PcmSampleRate;    
    mt_u32 u32LbrSampleRate;
    mt_u32 u32HbrSampleRate;
    MT_BOOL bEac4TimeSampleRate;

    mt_u32 u32PcmChannels;
    mt_u32 u32LbrChannels; /* 2 */
    mt_u32 u32HbrChannels; /* 2(DDP) or 8 */
    mt_u32 u32OrgMultiPcmChannels; /* 6 or 8 */

    mt_u32 u32PcmBitDepth; /* 16 or 24 */
    mt_u32 u32LbrBitDepth; /* 16 */
    mt_u32 u32HbrBitDepth; /* 16(Iec61937) or 24(blue-ray LPCM) */

    mt_u32 u32PcmSamplesPerFrame;
    mt_u32 u32PcmBytesPerFrame;
    mt_u32 u32LbrBytesPerFrame;
    mt_u32 u32HbrBytesPerFrame;

    mt_u32 u32LbrFormat;  /* DD/DTS */
    mt_u32 u32HbrFormat;  /* (DDP/DTSHD/TRUEHD or 8ch-LPCM */

    mt_void *pPcmDataBuf;       /**<I/O, pointer to the decoded PCM data.
                                  Note: 1) pDataBuf must be 32-word aligned. */
    mt_void *pLbrDataBuf;       /**<I/O, pointer to the decoded LBR data.
                                Note: 1) pDataBuf must be 32-word aligned. */
    mt_void *pHbrDataBuf;       /**<I/O, pointer to the decoded HBR data.
                                  Note: 1) pDataBuf must be 32-word aligned. */
    mt_u32 u32chan;                                
} SND_TRACK_STREAM_ATTR_S;

typedef struct
{
    TRACK_STREAMMODE_CHANGE_E enPcmChange;
    TRACK_STREAMMODE_CHANGE_E enSpdifChange;
    TRACK_STREAMMODE_CHANGE_E enHdmiChnage;
} STREAMMODE_CHANGE_ATTR_S;

typedef struct
{
    MT_UNF_AUDIOTRACK_ATTR_S stUserTrackAttr;
    MT_UNF_SND_ABSGAIN_ATTR_S  stTrackAbsGain;  // L/R Gain
    MT_BOOL bMute;                
    MT_UNF_TRACK_MODE_E enChannelMode;
    AO_SND_SPEEDADJUST_TYPE_E enUserSpeedType;
    mt_s32 s32UserSpeedRate;
    mt_u32 u32AddMuteFrameNum;
    MT_BOOL bEosFlag;

    /* internal state */
    mt_u32                  TrackId;
    MT_BOOL                 bAlsaTrack;
    SND_TRACK_STATUS_E      enCurnStatus;
    SND_TRACK_STREAM_ATTR_S stStreamAttr;
    MT_BOOL                 bAttAi;
    mt_handle               hAi;

    /*save pcm*/
    SND_DEBUG_CMD_CTRL_E    enSaveState;
    mt_u32                  u32SaveCnt;
    struct file *           fileHandle;            

    /*track send statistics*/
    mt_u32                  u32SendTryCnt;
    mt_u32                  u32SendCnt;

    AOE_AIP_ID_E       enAIP[SND_ENGINE_TYPE_BUTT];
    mmz_buffer_s       stAipRbfMmz[SND_ENGINE_TYPE_BUTT];
    MT_BOOL            bAipRbfExtDmaMem[SND_ENGINE_TYPE_BUTT];
    mt_u32            u32PcmSampleRate;
    MT_BOOL         b_spdif_mod;
    MT_BOOL         bEac4TimeSampleRate;


    mt_u32            u32PcmSampleRate2;
    
    MT_BOOL         bEac4TimeSampleRate2;
} SND_TRACK_STATE_S;

typedef enum
{
    AO_HDMI_CAPABILITY_Reserved = 0,
    AO_HDMI_CAPABILITY_PCM,
    AO_HDMI_CAPABILITY_AC3,
    AO_HDMI_CAPABILITY_MPEG1,
    AO_HDMI_CAPABILITY_MP3,
    AO_HDMI_CAPABILITY_MPEG2,
    AO_HDMI_CAPABILITY_AAC,
    AO_HDMI_CAPABILITY_DTS,
    AO_HDMI_CAPABILITY_ATRAC,
    AO_HDMI_CAPABILITY_ONEBITAUDIO,
    AO_HDMI_CAPABILITY_DDP,
    AO_HDMI_CAPABILITY_DTSHD,
    AO_HDMI_CAPABILITY_MAT,
    AO_HDMI_CAPABILITY_DST,
    AO_HDMI_CAPABILITY_WMAPRO,
    AO_HDMI_CAPABILITY_ReservedFmt15,
    AO_HDMI_CAPABILITY_BUTT
} AO_HDMI_CAPABILITY_E;


mt_u32	TRACK_RS_GetFreeId(mt_void);

mt_void TRACK_RS_Init(mt_void);

mt_void TRACK_RS_DeInit(mt_void);

mt_void TRACK_RS_RegisterId(mt_u32 TrackId);

mt_void TRACK_RS_DeRegisterId(mt_u32 TrackId);

mt_s32	TRACK_Create(SND_CARD_STATE_S *pCard, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr,
                     MT_BOOL bAlsaTrack, AO_BUF_ATTR_S *pstBuf, mt_handle *phTrack);

mt_s32 TRACK_CreateNew(SND_CARD_STATE_S *pCard, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr,
                    MT_BOOL bAlsaTrack, AO_BUF_ATTR_S *pstBuf, mt_u32 TrackId);

mt_s32	TRACK_Destroy(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID);
mt_s32	TRACK_Start(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID);
mt_s32	TRACK_Stop(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID);
mt_s32	TRACK_Pause(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID);
mt_s32	TRACK_Flush(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID);
mt_void TRACK_DestroyEngine(SND_CARD_STATE_S *pCard);
mt_s32  TRACK_CheckAttr(MT_UNF_AUDIOTRACK_ATTR_S * pstTrackAttr);
mt_s32	TRACK_SetAttr(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_AUDIOTRACK_ATTR_S * pstTrackAttr);
mt_s32	TRACK_GetAttr(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_AUDIOTRACK_ATTR_S * pstTrackAttr);
mt_s32	TRACK_SendData(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_AO_FRAMEINFO_S * pstAOFrame);
mt_s32	TRACK_SetWeight(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_SND_GAIN_ATTR_S *pstTrackGain);
mt_s32	TRACK_GetWeight(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_SND_GAIN_ATTR_S *pstTrackGain);
mt_s32	TRACK_SetSpeedAdjust(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, AO_SND_SPEEDADJUST_TYPE_E enType, mt_s32 s32Speed);
mt_s32	TRACK_GetDelayMs(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_u32 *pu32DelayMs);
mt_s32  TRACK_IsBufEmpty(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_BOOL *pbBufEmpty);
mt_s32  TRACK_SetEosFlag(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_BOOL bEosFlag);
mt_s32	TRACK_GetStatus(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_void *pstParam);
mt_s32	TRACK_GetDefAttr(MT_UNF_AUDIOTRACK_ATTR_S * pstDefAttr);
mt_s32  Track_ReadProc(struct seq_file* p, SND_CARD_STATE_S *pCard);
mt_s32  TRACK_WriteProc(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, SND_DEBUG_CMD_CTRL_E enCmd);
mt_s32  TRACK_DetectAttr(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_AUDIOTRACK_ATTR_S * pstTrackAttr, SND_TRACK_ATTR_SETTING_E *penAttrSetting);
mt_u32  TRACK_GetMasterId(SND_CARD_STATE_S *pCard);
mt_s32  TRACK_SetPcmAttr(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_handle hAi);
mt_s32  TRACK_AttachAi(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_handle hAi);
mt_s32  TRACK_DetachAi(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID);
mt_s32 TRACK_GetSetting(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, SND_TRACK_SETTINGS_S* pstSndSettings);
mt_s32 TRACK_RestoreSetting(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, SND_TRACK_SETTINGS_S* pstSndSettings);
mt_s32 TRACK_SetAbsGain(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_SND_ABSGAIN_ATTR_S *pstTrackAbsGain);
mt_s32 TRACK_GetAbsGain(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_SND_ABSGAIN_ATTR_S *pstTrackAbsGain);
mt_s32 TRACK_SetMute(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_BOOL bMute);
mt_s32 TRACK_GetMute(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_BOOL *pbMute);
mt_s32 TRACK_SetAllMute(SND_CARD_STATE_S *pCard, MT_BOOL bMute);
mt_s32 TRACK_SetChannelMode(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_TRACK_MODE_E *penMode);
mt_s32 TRACK_GetChannelMode(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_TRACK_MODE_E *penMode);
//for ALSA
mt_s32 TRACK_SetBufAttr(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_u32 u32PeriodSize);    //  NO  USED
mt_s32 TRACK_UpdateWptrPos(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_u32 *pu32WptrLen);  //for alsa
mt_s32 TRACK_UpdateRptrPos(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_u32 *pu32RptrLen);    //for alsa
mt_s32 TRACK_FlushBuf(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID);    //for alsa
mt_s32 TRACK_GetReadPos(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_u32 *pu32ReadPos);    //for alsa

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif  // __MT_AO_TRACK_H__
