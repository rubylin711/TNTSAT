#ifndef  _COMMON_MPI_H
#define  _COMMON_MPI_H

#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_vo.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_search.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_MT_CHIP_SYMPHONY4
#define AVPLAYER_VIDEO_BUFFER_SIZE 2*1024*1024
#define AVPLAYER_AUDIO_BUFFER_SIZE 192*1024
#elif defined CONFIG_MT_CHIP_SYMPHONY6
#ifdef CONFIG_MT_VDEC_4K	
#define AVPLAYER_VIDEO_BUFFER_SIZE 16*1024*1024
#else
#define AVPLAYER_VIDEO_BUFFER_SIZE 6*1024*1024
#endif
#define AVPLAYER_AUDIO_BUFFER_SIZE 384*1024
#endif


extern char *mt_optarg;

/************************************Struct Definition*******************************/

/*define nim play use nim type */
typedef enum
{
    MT_NIM_TYPE_DVBC = 1,
    MT_NIM_TYPE_J83B = 2,
    MT_NIM_TYPE_DVBT = 4,
    MT_NIM_TYPE_DVBS_IN = 8,
    MT_NIM_TYPE_DVBS_OUT = 16
} mt_nim_type;

/*define   dvbc nim   info */
typedef struct
{
    mt_u32 tuner_id; /**<tuner id*/
    mt_u32 freq;     /**<Frequency, in kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/
    mt_u32 mod_type; /**<QAM mode*/
} mt_nim_dvbc_info;

/*define dvbt nim info */
typedef struct
{
    mt_u32 tuner_id;  /**<tuner id*/
    mt_u32 freq;      /**<Frequency, in kHz*/
    mt_u32 bandwidth; /**<Symbol rate, in bit/s*/
} mt_nim_dvbt_info;

/*define   dvbs nim info*/
typedef struct
{
    mt_s32 tuner_id;  /**<tuner id*/
    mt_u32 freq;      /**<Frequency, in kHz*/
    mt_u32 sym_rate;  /**<Symbol rate, in bit/s*/
    mt_u32 onoff_22k; /**<22k*/
    mt_u32 polar;     /**<Polarization mode>*/
    mt_u32 dvbs_type; /**<dvbs type>*/
} mt_nim_dvbs_info;

/*define nim play use jb38 input info */
typedef struct
{
    mt_u32 tuner_id;   /**<tuner id*/
    mt_u32 freq; /**<Frequency, in kHz*/              /**<CNcomment:频率，单位：kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/      /**<CNcomment:符号率，单位bps */
    mt_u32 mod_type; /**<QAM mode*/                   /**<CNcomment:QAM调制方式*/
} mt_nim_j83b_info;

typedef struct
{
    mt_nim_dvbc_info dvbc;
    mt_nim_dvbt_info dvbt;
    mt_nim_dvbs_info dvbs_in;
    mt_nim_dvbs_info dvbs_out;
    mt_nim_j83b_info j83b;
    mt_u32 cur_nim_use;
} mt_nim_config_info;

typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hWin;
    MT_HANDLE          hSoundTrack;
} MT_AVPLAY_INFO;


/********************** Global Variable declaration **************************/
#define MT_INVALID_ID  0xFF
typedef struct
{
    mt_s32 sig_type;
    mt_u32 s32ProgNum;
    MT_BOOL demux_use;
	mt_u8	demux_id;
    MT_BOOL rec_status;
}mt_play_resource_t;

extern mt_play_resource_t play_resource;

typedef struct {
    /*
    0bit: dowmmix_enable 1bit:dialogue_enhancement_enable 2bit:encoder_output_enable
    3bit: mat_encoder_output_enable 4bit:ad_enable 5bit:ad_volume_enable
    6bit: ad_content_type_over_lang_enable 7bit:ad_content_type_enable 8bit:ac4_lang_enable
    9bit: presentation_id_enable 10bit:speaker_enable 11bit:ms12_decode_enable
    */
    mt_s32 ac4_attr_enable; 
    
    mt_s32  downmix_type;
    mt_s32  dialogue_enhancement_value;
    mt_s32  encoder_output_type;

    mt_s32  mat_encoder_output_value;
    mt_bool ad_value;
    mt_s32  ad_volume_value;

    mt_s32  ad_content_type_over_lang_value;
    mt_s32  ad_content_type;
    MT_UNF_AVPLAY_AC4_LANG_S ac4_lang;

    mt_s32  presentation_id;
    mt_s32  speaker_value;
    mt_s32  ms12_decode_value;
} play_ac4_attr_info;


#if 0
mt_s32  MPI_VoInit();
mt_s32  MPI_VoDeInit();
mt_s32  MPI_SndInit(mt_void);
mt_s32  MPI_SndDeInit(mt_void);
mt_s32  MPI_DispInit(mt_void);
mt_s32  MPI_DispDeInit(mt_void);
mt_s32  MPI_AVPlayInit(mt_void);
mt_s32  MPI_AVPlayDeInit(mt_void);
mt_s32  MPI_AVPlayStart(mt_handle hAvplay, DB_PROGRAM_S *psProg);
mt_s32  MPI_VoDestroyWin(mt_handle hWin);
mt_s32  MPI_VoCreateWin(mt_handle *phWin, mt_u8 u8WinNum);
mt_s32   MPI_SetVideoAttr(mt_handle hAvplay, mt_u32 pid, MT_UNF_VCODEC_TYPE_E videotype);
mt_s32   MPI_SetAudioAttr(mt_handle hAvplay, mt_u32 pid, mt_u32 audiotype);
#else
/* **********************************Demux  Common Interface********************************/
mt_s32 MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_E unblank);
mt_s32 MTADP_Get_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_E *unblank);

mt_s32 MTADP_Set_AudPcmInfo(pcm_info_t pcm);

mt_s32 MTADP_Get_AudPcmInfo(pcm_info_t *pcm);

mt_s32 MTADP_Set_Heaac_Enable(MT_BOOL heaac);

mt_s32 MTADP_Get_Heaac_Enable(MT_BOOL *heaac);

mt_s32 MTADP_SND_SetHdmiMode(MT_UNF_SND_HDMI_MODE_E hdmi_mode);

mt_s32 MTADP_SND_GetHdmiMode(MT_UNF_SND_HDMI_MODE_E *hdmi_mode);

mt_s32 MTADP_SND_SetSpidfMode(MT_UNF_SND_SPDIF_MODE_E spidf_mode);

mt_s32 MTADP_SND_GetSpidfMode(MT_UNF_SND_SPDIF_MODE_E *spidf_mode);

mt_s32 MTADP_Str_SetNimInfo(mt_nim_config_info nim_info);

mt_s32 MTADP_Str_GetNimInfo(mt_nim_config_info* nim_info);

mt_s32 MTADP_Demux_Init(mt_u32 DmxPortID,mt_u32 TsPortID);

mt_s32 MTADP_Demux_DeInit(mt_u32 DmxPortID);

/************************************DISPLAY  Common Interface*******************************/
mt_s32 MTADP_Disp_StrToFmt(mt_char *pszFmt);

mt_s32 MTADP_Disp_SetFormat(MT_UNF_ENC_FMT_E enFormat);

mt_s32 MTADP_Disp_Init(MT_UNF_ENC_FMT_E enFormat);

mt_s32 MTADP_Disp_DeInit(mt_void);

/****************************VO  Common Interface********************************************/
mt_s32 MTADP_VO_Init(MT_UNF_VO_DEV_MODE_E enDevMode);

mt_s32 MTADP_VO_CreatWin(mt_rect_s * pstWinRect, mt_handle * phWin);

mt_s32 MTADP_VO_CreatPipWin(mt_rect_s *pstWinRect,mt_handle *phWin);

mt_s32 MTADP_VO_CreatWinExt(mt_rect_s * pstWinRect, mt_handle * phWin, MT_BOOL bVirtScreen);

mt_s32 MTADP_VO_DeInit(mt_void);

mt_s32 MTADP_VO_GetWin(mt_handle *phWin);


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
mt_s32 MTADP_AVPlay_GetAdecAttr(mt_handle hAvplay,HA_CODEC_ID_E mAdecType,MT_UNF_ACODEC_ATTR_S *p_AdecAttr);
mt_s32 MTADP_AVPlay_SetAdecAttr(mt_handle hAvplay,mt_u32 enADecType,MT_HA_DECODEMODE_E enMode, mt_s32 isCoreOnly);

mt_s32 MTADP_AVPlay_PlayProg(mt_handle hAvplay,PMT_COMPACT_TBL *pProgTbl,mt_u32 ProgNum,MT_BOOL bAudPlay);

mt_s32 MTADP_AVPlay_PlayAud(mt_handle hAvplay,PMT_COMPACT_TBL *pProgTbl,mt_u32 ProgNum);

mt_s32 MTADP_AVPlay_SwitchAud(mt_handle hAvplay,mt_u32 AudPid, mt_u32 u32AudType);

mt_s32 MTADP_AENC_GetAttr(MT_UNF_AENC_ATTR_S *pAencAttr, mt_void *pstConfig);

mt_s32 MTADP_MCE_Exit(mt_void);

mt_s32 MTADP_DMX_AttachTSPort(mt_u32 Dmxid, mt_u32 TunerID);

mt_s32 MTADP_Show_LOGO( MT_UNF_VCODEC_TYPE_E VdecType, mt_void * p_data,mt_u32 datalen);
mt_s32 MTADP_Clear_LOGO(mt_void);

mt_s32 MTADP_Switch_Standby_Mode(mt_void);

mt_s32 MTADP_Getopt(int argc, char *argv[], char *opts);

size_t MTADP_Strncpy(char *dest, const char *src, size_t size);

mt_s32 MTADP_ReadPlayStat(mt_s64 *first_vid_frm_show_time, mt_s64 *avsync_done_time);

mt_s32 MTADP_AUD_SetAc4PlayAttrInfo(play_ac4_attr_info play_ac4_info);

mt_s32 MTADP_AUD_GetAc4PlayAttrInfo(play_ac4_attr_info *play_ac4_info);

mt_s32 MTADP_AUD_ResetAc4PlayAttrInfo(void);

mt_s32 MTADP_AUD_RestoreAc4PlayAttrInfo(mt_handle avplay);

#endif

#ifdef __cplusplus
}
#endif

#endif
