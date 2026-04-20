/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_AO_IOCTL_H__
 #define __DRV_AO_IOCTL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "mt_type.h"
#include "mt_unf_sound.h"
#include "mt_drv_ao.h"


typedef struct hiAO_SND_OpenDefault_Param_S
{
    MT_UNF_SND_E       enSound;
    MT_UNF_SND_ATTR_S  stAttr;
} AO_SND_OpenDefault_Param_S, *AO_SND_OpenDefault_Param_S_PTR;

typedef struct hiAO_SND_Open_Param_S
{
    MT_UNF_SND_E       enSound;
    MT_UNF_SND_ATTR_S  stAttr;
    mt_void                  *pAlsaPara;//MT_ALSA_I2S_ONLY_SUPPORT //0730
} AO_SND_Open_Param_S, *AO_SND_Open_Param_S_PTR;

typedef struct hiAO_SND_Mute_Param_S
{
    MT_UNF_SND_E            enSound;
    MT_UNF_SND_OUTPUTPORT_E enOutPort;
    MT_BOOL                 bMute;
} AO_SND_Mute_Param_S, *AO_SND_Mute_Param_S_PTR;

typedef struct hiAO_SND_HdmiMode_Param_S
{
    MT_UNF_SND_E                        enSound;
    MT_UNF_SND_OUTPUTPORT_E             enOutPort;
    MT_UNF_SND_HDMI_MODE_E              enMode;
} AO_SND_HdmiMode_Param_S, *AO_SND_HdmiMode_Param_S_PTR;

typedef struct hiAO_SND_SpdifMode_Param_S
{
    MT_UNF_SND_E                        enSound;
    MT_UNF_SND_OUTPUTPORT_E             enOutPort;
    MT_UNF_SND_SPDIF_MODE_E             enMode;
} AO_SND_SpdifMode_Param_S, *AO_SND_SpdifMode_Param_S_PTR;

typedef struct hiAO_SND_SpdifSCMSMode_Param_S
{
    MT_UNF_SND_E                        enSound;
    MT_UNF_SND_OUTPUTPORT_E             enOutPort;
    MT_UNF_SND_SPDIF_SCMSMODE_E         enSCMSMode;
} AO_SND_SpdifSCMSMode_Param_S, *AO_SND_SpdifSCMSMode_Param_S_PTR;

typedef struct hiAO_SND_SpdifCategoryCode_Param_S
{
    MT_UNF_SND_E                        enSound;
    MT_UNF_SND_OUTPUTPORT_E             enOutPort;
    MT_UNF_SND_SPDIF_CATEGORYCODE_E     enCategoryCode;
} AO_SND_SpdifCategoryCode_Param_S, *AO_SND_SpdifCategoryCode_Param_S_PTR;
typedef struct hiAO_SND_Volume_Param_S
{
    MT_UNF_SND_E                        enSound;
    MT_UNF_SND_OUTPUTPORT_E             enOutPort;
    MT_UNF_SND_GAIN_ATTR_S stGain;
} AO_SND_Volume_Param_S, *AO_SND_Volume_Param_S_PTR;

typedef struct hiAO_SND_SampleRate_Param_S
{
    MT_UNF_SND_E            enSound;
    MT_UNF_SND_OUTPUTPORT_E enOutPort;
    MT_UNF_SAMPLE_RATE_E    enSampleRate;
} AO_SND_SampleRate_Param_S, *AO_SND_SampleRate_Param_S_PTR;

typedef struct hiAO_SND_TrackMode_Param_S
{
    MT_UNF_SND_E            enSound;
    MT_UNF_SND_OUTPUTPORT_E enOutPort;
    MT_UNF_TRACK_MODE_E     enMode;
} AO_SND_TrackMode_Param_S, *AO_SND_TrackMode_Param_S_PTR;

typedef struct hiAO_SND_AllTrackMute_Param_S
{
    MT_UNF_SND_E            enSound;
    MT_BOOL                 bMute;
} AO_SND_AllTrackMute_Param_S, *AO_SND_AllTrackMute_Param_S_PTR;

typedef struct hiAO_SND_SmartVolume_Param_S
{
    MT_UNF_SND_E            enSound;
    MT_UNF_SND_OUTPUTPORT_E enOutPort;
    MT_BOOL                 bSmartVolume;
} AO_SND_SmartVolume_Param_S, *AO_SND_SmartVolume_Param_S_PTR;


typedef struct hiAO_Track_Create_Param_S
{
    MT_UNF_SND_E             enSound;
    MT_UNF_AUDIOTRACK_ATTR_S stAttr;
    MT_BOOL                  bAlsaTrack;      //if bAlsaTrack = MT_TRUE ALSA, or UNF
    AO_BUF_ATTR_S            stBuf;          //Only for compatible alsa
    mt_handle                hTrack;
} AO_Track_Create_Param_S, *AO_Track_Create_Param_S_PTR;

typedef struct hiAO_SND_SetPeriodSize_Param_S
{
    MT_UNF_SND_E            enSound;
    MT_UNF_SND_OUTPUTPORT_E enOutPort;
    mt_u32                  u32PeriodSize;
    mt_void                 (*Func)(mt_void);
} AO_SND_SetPeriodSize_Param_S;

typedef struct hiAO_Track_Ci_Test_Info_Param_S
{
    mt_handle                    hTrack;
    MT_UNF_AVPLAY_CI_TEST_INFO_S     stInfo;
} AO_Track_Ci_Test_Info_Param_S;

typedef struct hiAO_Track_Attr_Param_S
{
    mt_handle                    hTrack;
    MT_UNF_AUDIOTRACK_ATTR_S     stAttr;
} AO_Track_Attr_Param_S, *AO_Track_Attr_Param_S_PTR;

typedef struct hiAO_Track_Weight_Param_S
{
    mt_handle               hTrack;
    MT_UNF_SND_GAIN_ATTR_S  stTrackGain;
} AO_Track_Weight_Param_S, *AO_Track_Weight_Param_S_PTR;


typedef struct hiAO_Track_AbsGain_Param_S
{
    mt_handle               hTrack;
    MT_UNF_SND_ABSGAIN_ATTR_S  stTrackAbsGain;
} AO_Track_AbsGain_Param_S, *AO_Track_AbsGain_Param_S_PTR;

typedef struct hiAO_Track_Mute_Param_S
{
    mt_handle               hTrack;
    MT_BOOL                 bMute;
} AO_Track_Mute_Param_S, *AO_Track_Mute_Param_S_PTR;

typedef struct hiAO_Track_ChannelMode_Param_S
{
    mt_handle               hTrack;
    MT_UNF_TRACK_MODE_E     enMode;
} AO_Track_ChannelMode_Param_S, *AO_Track_ChannelMode_Param_S_PTR;

typedef struct hiAO_Track_SendData_Param_S
{
    mt_handle              hTrack;
    MT_UNF_AO_FRAMEINFO_S  stAOFrame;
} AO_Track_SendData_Param_S, *AO_Track_SendData_Param_S_PTR;

typedef struct hiAO_Track_SpeedAdjust_Param_S
{
    mt_handle                 hTrack;
    AO_SND_SPEEDADJUST_TYPE_E enType;
    mt_s32                    s32Speed;
} AO_Track_SpeedAdjust_Param_S, *AO_Track_SpeedAdjust_Param_S_PTR;

typedef struct hiAO_Track_DelayMs_Param_S
{
    mt_handle hTrack;
    mt_u32    u32DelayMs;
} AO_Track_DelayMs_Param_S, *AO_Track_DelayMs_Param_S_PTR;

typedef struct hiAO_Track_BufEmpty_Param_S
{
    mt_handle hTrack;
    MT_BOOL   bEmpty;
} AO_Track_BufEmpty_Param_S, *AO_Track_BufEmpty_Param_S_PTR;

typedef struct hiAO_Track_EosFlag_Param_S
{
    mt_handle hTrack;
    MT_BOOL   bEosFlag;
} AO_Track_EosFlag_Param_S, *AO_Track_EosFlag_Param_S_PTR;

typedef struct hiAO_Track_Status_Param_S
{
    mt_handle hTrack;
    mt_void * pstStatus;
} AO_Track_Status_Param_S, *AO_Track_Status_Param_S_PTR;

typedef struct hiAO_Track_AttAi_Param_S
{
    mt_handle hTrack;
    mt_handle hAi;
} AO_Track_AttAi_Param_S, *AO_Track_AttAi_Param_S_PTR;

typedef struct hiAO_Cast_Mute_Param_S
{
    mt_handle               hCast;
    MT_BOOL                 bMute;
} AO_Cast_Mute_Param_S, *AO_Cast_Mute_Param_S_PTR;

typedef struct hiAO_Cast_AbsGain_Param_S
{
    mt_handle               hCast;
    MT_UNF_SND_ABSGAIN_ATTR_S  stCastAbsGain;
} AO_Cast_AbsGain_Param_S, *AO_Cast_AbsGain_Param_S_PTR;

typedef struct hiAO_Cast_Create_Param_S
{
    MT_UNF_SND_E            enSound;
    mt_handle               hCast;
    mt_u32                     u32ReqSize;
    MT_UNF_SND_CAST_ATTR_S stCastAttr;
} AO_Cast_Create_Param_S, *AO_Cast_Create_Param_S_PTR;
typedef struct hiAO_Cast_Info_Param_S
{
    mt_handle          hCast;
    ulong                u32UserVirtAddr;
    ulong                u32KernelVirtAddr;
    phys_addr_t                u32PhyAddr;

    mt_u32                u32FrameBytes;
    mt_u32                u32FrameSamples;
    mt_u32                u32Channels;
    mt_s32                s32BitPerSample;
    
} AO_Cast_Info_Param_S, *AO_Cast_Info_Param_S_PTR;

typedef struct hiAO_Cast_Enable_Param_S
{
    mt_handle          hCast;
    MT_BOOL              bCastEnable;
} AO_Cast_Enable_Param_S, *AO_Cast_Enable_Param_S_PTR;

typedef struct hiAO_Cast_Data_Param_S
{
    mt_handle          hCast;
    mt_u32                u32FrameBytes;
    mt_u32                u32SampleBytes;
    
    mt_u32                u32DataOffset;
    MT_UNF_AO_FRAMEINFO_S   stAOFrame;
} AO_Cast_Data_Param_S, *AO_Cast_Data_Param_S_PTR;

typedef struct hiAO_SND_AttAef_Param_S
{
    MT_UNF_SND_E            enSound;
    mt_u32                  u32AefId;
    mt_u32                  u32AefProcAddr;
} AO_SND_AttAef_Param_S, *AO_SND_AttAef_Param_S_PTR;

typedef struct hiAO_SND_AefBypass_Param_S
{
    MT_UNF_SND_E            enSound;
    MT_UNF_SND_OUTPUTPORT_E enOutPort;
    MT_BOOL                 bBypass;
} AO_SND_AefBypass_Param_S, *AO_SND_AefBypass_Param_S_PTR;

typedef struct hiAO_SND_GET_XRUN_S
{
    MT_UNF_SND_E            enSound;
    mt_u32                  u32Count;
} AO_SND_Get_Xrun_Param_S, *AO_SND_Get_Xrun_Param_S_PTR;

/********************************************************
  AO command code definition
 *********************************************************/
/*AO SND command code*/
 #define CMD_AO_GETSNDDEFOPENATTR _IOWR  (MT_ID_AO, 0x00, AO_SND_OpenDefault_Param_S)
 #define CMD_AO_SND_OPEN _IOWR (MT_ID_AO, 0x01, AO_SND_Open_Param_S)
 #define CMD_AO_SND_CLOSE _IOW  (MT_ID_AO, 0x02, MT_UNF_SND_E)
 #define CMD_AO_SND_SETMUTE _IOW (MT_ID_AO, 0x03, AO_SND_Mute_Param_S)
 #define CMD_AO_SND_GETMUTE _IOWR (MT_ID_AO, 0x04, AO_SND_Mute_Param_S)
 #define CMD_AO_SND_SETHDMIMODE _IOW (MT_ID_AO, 0x05, AO_SND_HdmiMode_Param_S)
 #define CMD_AO_SND_SETSPDIFMODE _IOW (MT_ID_AO, 0x06, AO_SND_SpdifMode_Param_S)
 #define CMD_AO_SND_SETVOLUME _IOW (MT_ID_AO, 0x07, AO_SND_Volume_Param_S)
 #define CMD_AO_SND_GETVOLUME _IOWR (MT_ID_AO, 0x08, AO_SND_Volume_Param_S)
 #define CMD_AO_SND_SETSAMPLERATE _IOW (MT_ID_AO, 0x09, AO_SND_SampleRate_Param_S)
 #define CMD_AO_SND_GETSAMPLERATE _IOWR (MT_ID_AO, 0x0a, AO_SND_SampleRate_Param_S)
 #define CMD_AO_SND_SETTRACKMODE _IOW (MT_ID_AO, 0x0b, AO_SND_TrackMode_Param_S)
 #define CMD_AO_SND_GETTRACKMODE _IOWR (MT_ID_AO, 0x0c, AO_SND_TrackMode_Param_S)
 #define CMD_AO_SND_SETSMARTVOLUME _IOW (MT_ID_AO, 0x0d, AO_SND_SmartVolume_Param_S)
 #define CMD_AO_SND_GETSMARTVOLUME _IOWR (MT_ID_AO, 0x0e, AO_SND_SmartVolume_Param_S)
 #define CMD_AO_SND_SETCASTENABLE _IOW (MT_ID_AO, 0x0f, AO_SND_CastEnable_Param_S)  
 #define CMD_AO_SND_READCASTDATA _IOW (MT_ID_AO, 0x10, AO_SND_CastData_Param_S)  //_IOW verify
 #define CMD_AO_SND_GETHDMIMODE _IOWR (MT_ID_AO, 0x11, AO_SND_HdmiMode_Param_S)
 #define CMD_AO_SND_GETSPDIFMODE _IOWR (MT_ID_AO, 0x12, AO_SND_SpdifMode_Param_S)
 #define CMD_AO_SND_SETSPDIFSCMSMODE _IOW (MT_ID_AO, 0x13, AO_SND_SpdifSCMSMode_Param_S)
 #define CMD_AO_SND_GETSPDIFSCMSMODE _IOWR (MT_ID_AO, 0x14, AO_SND_SpdifSCMSMode_Param_S)
 #define CMD_AO_SND_SETSPDIFCATEGORYCODE _IOW (MT_ID_AO, 0x15, AO_SND_SpdifCategoryCode_Param_S)
 #define CMD_AO_SND_GETSPDIFCATEGORYCODE _IOWR (MT_ID_AO, 0x16, AO_SND_SpdifCategoryCode_Param_S)
 #define CMD_AO_SND_SETADAC _IOW  (MT_ID_AO, 0x17, MT_BOOL *)
 #define CMD_AO_SND_SETRENDERINGRATE _IOW (MT_ID_AO, 0x18, mt_u32 *)

 #define CMD_AO_SND_ATTACHTRACK _IOW (MT_ID_AO, 0x020, mt_handle)
 #define CMD_AO_SND_DETACHTRACK _IOW  (MT_ID_AO, 0x21, mt_handle)

 #define CMD_AO_SND_ATTACHAEF _IOWR (MT_ID_AO, 0x022, AO_SND_AttAef_Param_S)
 #define CMD_AO_SND_DETACHAEF _IOW  (MT_ID_AO, 0x23, AO_SND_AttAef_Param_S)
 #define CMD_AO_SND_SETAEFBYPASS _IOW (MT_ID_AO, 0x024, AO_SND_AefBypass_Param_S)
 #define CMD_AO_SND_GETAEFBYPASS _IOWR (MT_ID_AO, 0x25, AO_SND_AefBypass_Param_S)
 
 #define CMD_AO_SND_SETALLTRACKMUTE _IOW  (MT_ID_AO, 0x26, AO_SND_AllTrackMute_Param_S)
 #define CMD_AO_SND_GETALLTRACKMUTE _IOWR	(MT_ID_AO, 0x27, AO_SND_AllTrackMute_Param_S)
 #define CMD_AO_SND_GETXRUNCOUNT _IOWR	(MT_ID_AO, 0x28, AO_SND_Get_Xrun_Param_S)

/*AO Track command code*/
 #define CMD_AO_TRACK_GETDEFATTR _IOWR  (MT_ID_AO, 0x40, MT_UNF_AUDIOTRACK_ATTR_S)
 #define CMD_AO_TRACK_CREATE _IOWR (MT_ID_AO, 0x41, AO_Track_Create_Param_S)
 #define CMD_AO_TRACK_DESTROY _IOW  (MT_ID_AO, 0x42, mt_handle)
 #define CMD_AO_TRACK_START _IOW  (MT_ID_AO, 0x43, mt_handle)
 #define CMD_AO_TRACK_STOP _IOW  (MT_ID_AO, 0x44, mt_handle)
 #define CMD_AO_TRACK_PAUSE _IOW  (MT_ID_AO, 0x45, mt_handle)
 #define CMD_AO_TRACK_FLUSH _IOW  (MT_ID_AO, 0x46, mt_handle)
 #define CMD_AO_TRACK_SENDDATA _IOW  (MT_ID_AO, 0x47, AO_Track_SendData_Param_S)
 #define CMD_AO_TRACK_SETWEITHT _IOW  (MT_ID_AO, 0x48, AO_Track_Weight_Param_S)
 #define CMD_AO_TRACK_GETWEITHT _IOWR  (MT_ID_AO, 0x49, AO_Track_Weight_Param_S)
 #define CMD_AO_TRACK_SETSPEEDADJUST _IOW  (MT_ID_AO, 0x4a, AO_Track_SpeedAdjust_Param_S)
 #define CMD_AO_TRACK_GETDELAYMS _IOWR  (MT_ID_AO, 0x4b, AO_Track_DelayMs_Param_S)
 #define CMD_AO_TRACK_ISBUFEMPTY _IOWR  (MT_ID_AO, 0x4c, AO_Track_BufEmpty_Param_S)
 #define CMD_AO_TRACK_SETEOSFLAG _IOW  (MT_ID_AO, 0x4d, AO_Track_Status_Param_S)
 #define CMD_AO_TRACK_GETATTR _IOWR  (MT_ID_AO, 0x4e, AO_Track_Attr_Param_S)
 #define CMD_AO_TRACK_SETATTR _IOW  (MT_ID_AO, 0x4f, AO_Track_Attr_Param_S)
 //+
 #define CMD_AO_TRACK_GETFREEBUfSIZE _IOWR  (MT_ID_AO, 0x50, AO_Track_Status_Param_S)
 #define CMD_AO_TRACK_ATTACHAI _IOW  (MT_ID_AO, 0x51, AO_Track_AttAi_Param_S)
 #define CMD_AO_TRACK_DETACHAI _IOW  (MT_ID_AO, 0x52, AO_Track_AttAi_Param_S)
 #define CMD_AO_TRACK_SETABSGAIN _IOW  (MT_ID_AO, 0x51, AO_Track_AbsGain_Param_S)
 #define CMD_AO_TRACK_GETABSGAIN _IOWR  (MT_ID_AO, 0x52, AO_Track_AbsGain_Param_S)
 #define CMD_AO_TRACK_SETMUTE _IOW  (MT_ID_AO, 0x53, AO_Track_Mute_Param_S)
 #define CMD_AO_TRACK_GETMUTE _IOWR  (MT_ID_AO, 0x54, AO_Track_Mute_Param_S)

#define CMD_AO_TRACK_SETCHANNELMODE _IOW  (MT_ID_AO, 0x55, AO_Track_ChannelMode_Param_S)
#define CMD_AO_TRACK_GETCHANNELMODE _IOWR  (MT_ID_AO, 0x56, AO_Track_ChannelMode_Param_S)
#define CMD_AO_TRACK_GETCITESTINFO _IOWR  (MT_ID_AO, 0x57, AO_Track_Ci_Test_Info_Param_S)
 
/*AO Cast command code*/
 #define CMD_AO_CAST_GETDEFATTR _IOWR  (MT_ID_AO, 0x60, MT_UNF_SND_CAST_ATTR_S)
 #define CMD_AO_CAST_CREATE _IOWR (MT_ID_AO, 0x61, AO_Cast_Create_Param_S)
 #define CMD_AO_CAST_DESTROY _IOW  (MT_ID_AO, 0x62, mt_handle)
 #define CMD_AO_CAST_SETENABLE _IOW  (MT_ID_AO, 0x63, AO_Cast_Enable_Param_S)
 #define CMD_AO_CAST_GETENABLE _IOWR  (MT_ID_AO, 0x64, AO_Cast_Enable_Param_S)
 #define CMD_AO_CAST_GETINFO  _IOWR  (MT_ID_AO, 0x65, AO_Cast_Info_Param_S)
 #define CMD_AO_CAST_SETINFO  _IOW  (MT_ID_AO, 0x66, AO_Cast_Info_Param_S)
 #define CMD_AO_CAST_ACQUIREFRAME  _IOWR  (MT_ID_AO, 0x67, AO_Cast_Data_Param_S)
 #define CMD_AO_CAST_RELEASEFRAME  _IOW  (MT_ID_AO, 0x68, AO_Cast_Data_Param_S)
 #define CMD_AO_CAST_SETABSGAIN _IOW  (MT_ID_AO, 0x69, AO_Cast_AbsGain_Param_S)
 #define CMD_AO_CAST_GETABSGAIN _IOWR  (MT_ID_AO, 0x6A, AO_Cast_AbsGain_Param_S)
 #define CMD_AO_CAST_SETMUTE _IOW  (MT_ID_AO, 0x6B, AO_Cast_Mute_Param_S)
 #define CMD_AO_CAST_GETMUTE _IOWR  (MT_ID_AO, 0x6C, AO_Cast_Mute_Param_S)
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
 
 #endif
