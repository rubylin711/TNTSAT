/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <stdlib.h>

#include "mt_unf_demux.h"
#include "mt_unf_avplay.h"
#include "mt_unf_common.h"
#include "mt_unf_vo.h"
#include "mt_unf_sound.h"
#include "mt_unf_ai.h"
#include "mt_common.h"
#include "mt_unf_disp.h"
#include "mt_unf_hdmi.h"
#include "mt_audio_codec.h"
#include "mt_error_mpi.h"
//#include "mt_unf_mce.h"
#include "mt_unf_pdm.h"
#include "mt_unf_gpio.h"

#include "mt_adp_mpi.h"
#include "mt_adp_debug.h"
#include "mt_adp_data.h"
//#include "mt_adp_audio.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"

#include "HA.AUDIO.G711.codec.h"
#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "HA.AUDIO.AMRNB.codec.h"
#include "HA.AUDIO.AMRWB.codec.h"
#include "HA.AUDIO.TRUEHDPASSTHROUGH.decode.h"
#include "HA.AUDIO.DOLBYTRUEHD.decode.h"
#include "HA.AUDIO.DTSHD.decode.h"
#if defined (DOLBYPLUS_HACODEC_SUPPORT)
#include "HA.AUDIO.DOLBYPLUS.decode.h"
#endif
#include "HA.AUDIO.AC3PASSTHROUGH.decode.h"
#include "HA.AUDIO.DTSM6.decode.h"

#include "HA.AUDIO.DTSPASSTHROUGH.decode.h"
#include "HA.AUDIO.FFMPEG_DECODE.decode.h"
#include "HA.AUDIO.AAC.encode.h"

#ifdef ANDROID
#include <utils/Log.h>
#include "mt_adp_osd.h"
#endif


#ifdef MTADP_MPI_DEBUG

#define MTADP_MPI_PRINT   MTADP_PRINT

#else

#define MTADP_MPI_PRINT

#endif


#define MTADP_MPI_FUNCTION_ENTER()  MTADP_MPI_PRINT("[MTADP_MPI][%s]: Enter ==>> \n", __FUNCTION__)
#define MTADP_MPI_FUNCTION_EXIT()   MTADP_MPI_PRINT("[MTADP_MPI][%s]: Exit ==<< \n", __FUNCTION__)

#define MTADP_MPI_FATAL_PRINT(fmt...)       MTADP_MPI_PRINT(" [MTADP_MPI][FATAL] " fmt)
#define MTADP_MPI_ERR_PRINT(fmt...)         MTADP_MPI_PRINT(" [MTADP_MPI][ERROR] " fmt)
#define MTADP_MPI_WARN_PRINT(fmt...)        MTADP_MPI_PRINT(" [MTADP_MPI][WARN] "  fmt)
#define MTADP_MPI_INFO_PRINT(fmt...)        MTADP_MPI_PRINT(" [MTADP_MPI][INFO] "  fmt)
#define MTADP_MPI_MT_DBG_PRINT(fmt...)      MTADP_MPI_PRINT(" [MTADP_MPI][DEBUG] " fmt)




#define MPI_DEMUX_NUM 5
#define MPI_DEMUX_PLAY 0
#define MPI_DEMUX_REC_0 1
#define MPI_DEMUX_REC_1 2
#define MPI_DEMUX_TIMETHIFT 3
#define MPI_DEMUX_PLAYBACK 4

#define FIRST_TUNER   0
#define SECOND_TUNER  1
#define THIRD_TUNER   2
#define FOURTH_TUNER  3
/*
big-endian pcm output format, if extword is 1, choose normal pcm decoder,
                                            if extword is 2, choose wifidsp_lpcm decoder(Frame Header:0xA0,0x06)
                                            if others, fail to decode.
*/
#define NORMAL_PCM_EXTWORD    1
#define WIFIDSP_LPCM_EXTWORD  2

/************************************Global varable Definition*******************************/
static mt_u8 u8DecOpenBuf[1024];

#if defined (DOLBYPLUS_HACODEC_SUPPORT)

static DOLBYPLUS_STREAM_INFO_S g_stDDpStreamInfo;

/*dolby Dual Mono type control*/
static mt_u32  g_u32DolbyAcmod = 0;
static MT_BOOL g_bDrawChnBar = MT_TRUE;

#endif

MT_BOOL g_heaac;
static pcm_info_t g_pcm;

static mt_nim_config_info g_save_nim_info = {0};
static MT_UNF_VCODEC_UNBLANK_E g_unblank = MT_UNF_VCODEC_UNBLANK_STABLE;
static MT_UNF_SND_HDMI_MODE_E g_snd_hdmi_mode = MT_UNF_SND_HDMI_MODE_LPCM;
static MT_UNF_SND_SPDIF_MODE_E g_snd_spidf_mode = MT_UNF_SND_SPDIF_MODE_LPCM;
static play_ac4_attr_info g_play_ac4_info = {0};

MT_AVPLAY_INFO avplayHandle;
mt_play_resource_t play_resource;


/************************************DISPLAY Common Interface*******************************/

mt_s32 MTADP_AUD_SetAc4PlayAttrInfo(play_ac4_attr_info play_ac4_info)
{
    g_play_ac4_info = play_ac4_info;
    return MT_SUCCESS;
}

mt_s32 MTADP_AUD_GetAc4PlayAttrInfo(play_ac4_attr_info *play_ac4_info)
{
    *play_ac4_info = g_play_ac4_info;
    return MT_SUCCESS;
}

mt_s32 MTADP_AUD_ResetAc4PlayAttrInfo(void)
{
    memset(&g_play_ac4_info, 0x0, sizeof(play_ac4_attr_info));
    return MT_SUCCESS;
}

mt_s32 MTADP_AUD_RestoreAc4PlayAttrInfo(mt_handle avplay)
{
    mt_s32 ret = MT_SUCCESS;
    MT_S32  dolby_mode = 0;
    play_ac4_attr_info play_ac4_info;

    MTADP_AUD_GetAc4PlayAttrInfo(&play_ac4_info);
    if (!play_ac4_info.ac4_attr_enable)
    {
        MTADP_MPI_INFO_PRINT("no restore ac4 play attr info \n");
        return ret;
    }

    if ((play_ac4_info.ac4_attr_enable & 1) == MT_TRUE)
    {
        ret = MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_AC4_DOWNMIX_MODE, &play_ac4_info.downmix_type);
        if (MT_SUCCESS != ret)
        {
            MTADP_MPI_ERR_PRINT("Set AC4 downmix mode:%d failed ret=0x%x \n", play_ac4_info.downmix_type, ret);
        }
    }

    if (((play_ac4_info.ac4_attr_enable >> 1) & 1) == MT_TRUE)
    {
        ret = MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_AC4_DIALOGUE_ENHANCEMENT, &play_ac4_info.dialogue_enhancement_value);
        if(MT_SUCCESS != ret){
            MTADP_MPI_ERR_PRINT("Set AC4 Dialogue Enhancement:%d failed ret=0x%x \n",play_ac4_info.dialogue_enhancement_value, ret);
        }
    }

    if (((play_ac4_info.ac4_attr_enable >> 2) & 1) == MT_TRUE)
    {
        MT_S32  mat_en = 0;

        if (play_ac4_info.encoder_output_type == 4)
        {
            MT_S32  i = 0;
            MT_S32  cap = 0;
            MT_UNF_EDID_BASE_INFO_S sink_cap;

            ret = MT_UNF_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_0, &sink_cap);
            if (ret != MT_SUCCESS)
            {
                MTADP_MPI_ERR_PRINT("call MT_UNF_HDMI_GetSinkCapability failed ret=0x%x \n",ret);
            }

            for (i = sink_cap.u32AudioInfoNum - 1; i >= 0; i--)
            {
                if (sink_cap.stAudioInfo[i].enAudFmtCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_MAT)
                {
                    cap |= 1 << 3; //bit 3
                    MTADP_MPI_INFO_PRINT("HDMI support MAT\n");
                }
                else if (sink_cap.stAudioInfo[i].enAudFmtCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_DDP)
                {
                    cap |= 1 << 2; //bit 2
                    MTADP_MPI_INFO_PRINT("HDMI support DD+\n");
                }
                else if (sink_cap.stAudioInfo[i].enAudFmtCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_AC3)
                {
                    cap |= 1 << 1; //bit 1
                    MTADP_MPI_INFO_PRINT("HDMI support DD\n");
                }
                else if (sink_cap.stAudioInfo[i].enAudFmtCode == MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM)
                {
                    cap |= 1; //bit 0
                    MTADP_MPI_INFO_PRINT("HDMI support PCM\n");
                }
            }

            if ( sink_cap.bSupportDdMat48k ) {
                cap |= 1<<3;//bit 3
                MTADP_MPI_INFO_PRINT("HDMI support MAT 48K only\n");
            }

            if (cap & (1 << 3))
            {
                mat_en = 1;
                MTADP_MPI_INFO_PRINT("HDMI auto mat\n");
            }
            else if (cap & (1 << 2))
            {
                dolby_mode = 2;
                MTADP_MPI_INFO_PRINT("HDMI auto DDP\n");
            }
            else if (cap & (1 << 1))
            {
                dolby_mode = 1;
                MTADP_MPI_INFO_PRINT("HDMI auto DD\n");
            }
            else if (cap & 1)
            {
                dolby_mode = 0;
                MTADP_MPI_INFO_PRINT("HDMI auto PCM\n");
            }

            if (mat_en == 1)
            {
                ret = MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_AC4_ENCODE_MAT, &mat_en);
                if (ret != MT_SUCCESS)
                {
                    MTADP_MPI_ERR_PRINT("Set AC4 MAT encoder enable failed ret=0x%x \n",ret);
                }
            }
            else
            {
                ret = MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_AC4_ENCODE_DD_DDP, &dolby_mode);
                if (ret != MT_SUCCESS)
                {
                    MTADP_MPI_ERR_PRINT("Set AC4 DD/DDP encoder output type:%d failed ret=0x%x \n", dolby_mode, ret);
                }
            }
        }
        else if (play_ac4_info.encoder_output_type == 3)
        {
            mat_en = 1;
            ret = MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_AC4_ENCODE_MAT, &mat_en);
            if (ret != MT_SUCCESS)
            {
                MTADP_MPI_ERR_PRINT("Set AC4 MAT encoder enable failed ret=0x%x \n", ret);
            }
            MTADP_MPI_INFO_PRINT("HDMI support MAT\n");
        }
        else
        {
            dolby_mode = play_ac4_info.encoder_output_type;
            ret = MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_AC4_ENCODE_DD_DDP, &dolby_mode);
            if (ret != MT_SUCCESS)
            {
                MTADP_MPI_ERR_PRINT("Set AC4 DD/DDP encoder output type:%d failed ret=0x%x \n", dolby_mode, ret);
            }
        }
    }

    if (((play_ac4_info.ac4_attr_enable >> 4) & 1) == MT_TRUE)
    {
        MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_AC4_AD_ONOFF, &play_ac4_info.ad_value);
        if(ret != MT_SUCCESS){
            MTADP_MPI_ERR_PRINT("Set AC4 AD on/off:%d failed ret=0x%x \n",play_ac4_info.ad_value, ret);
        }
    }

    if (((play_ac4_info.ac4_attr_enable >> 5) & 1) == MT_TRUE)
    {
        ret = MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_AC4_AD_VOL_WEIGHT, &play_ac4_info.ad_volume_value);
        if (ret != MT_SUCCESS)
        {
            MTADP_MPI_ERR_PRINT("Set AC4 AD volume weight:%d failed ret=0x%x \n",play_ac4_info.ad_volume_value, ret);
        }
    }

    if (((play_ac4_info.ac4_attr_enable >> 6) & 1) == MT_TRUE)
    {
        ret = MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_AC4_AD_TYPE_OVER_LANG,
            &play_ac4_info.ad_content_type_over_lang_value);
        if (ret != MT_SUCCESS)
        {
            MTADP_MPI_ERR_PRINT("Sel AC4 AD type over lang:%d failed ret=0x%x \n",
                play_ac4_info.ad_content_type_over_lang_value, ret);
        }
    }

    if (((play_ac4_info.ac4_attr_enable >> 7) & 1) == MT_TRUE)
    {
        ret = MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_AC4_AD_TYPE, &play_ac4_info.ad_content_type);
        if (ret != MT_SUCCESS){
            MTADP_MPI_ERR_PRINT("Sel AC4 AD content type:%d failed ret=0x%x \n", play_ac4_info.ad_content_type, ret);
        }
    }

    if (((play_ac4_info.ac4_attr_enable >> 8) & 1) == MT_TRUE)
    {
        ret = MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_AC4_LANG, &play_ac4_info.ac4_lang);
        if (ret != MT_SUCCESS)
        {
            MTADP_MPI_ERR_PRINT("Set AC4 language failed ret=0x%x \n", ret);
        }
    }

    if (((play_ac4_info.ac4_attr_enable >> 9) & 1) == MT_TRUE)
    {
        ret = MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_AC4_SET_PRES_ID, &play_ac4_info.presentation_id);
        if (ret != MT_SUCCESS)
        {
            MTADP_MPI_ERR_PRINT("Set AC4 presentation id:%d failed ret=0x%x \n", play_ac4_info.presentation_id, ret);
        }
    }

    if (((play_ac4_info.ac4_attr_enable >> 10) & 1) == MT_TRUE)
    {
        ret = MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_AC4_SET_ENCODE_DAP, &play_ac4_info.speaker_value);
        if (ret != MT_SUCCESS)
        {
            MTADP_MPI_ERR_PRINT("Set AC4 DAP encoder output type:%d failed ret=0x%x \n", play_ac4_info.speaker_value, ret);
        }
    }

    if (((play_ac4_info.ac4_attr_enable >> 11) & 1) == MT_TRUE)
    {
        ret = MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_DOLBY_FORCE_MS12_DEC, &play_ac4_info.ms12_decode_value);
        if (ret != MT_SUCCESS)
        {
            MTADP_MPI_ERR_PRINT("force ms12 decode dolby:%d failed ret=0x%x \n", play_ac4_info.ms12_decode_value, ret);
        }
    }

    return ret;
}

mt_s32 MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_E unblank)
{
    g_unblank = unblank;
    return MT_SUCCESS;
}

mt_s32 MTADP_Get_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_E *unblank)
{
    *unblank = g_unblank;
    return MT_SUCCESS;
}

mt_s32 MTADP_Set_Heaac_Enable(MT_BOOL heaac)
{
    g_heaac = heaac;
    MTADP_MPI_INFO_PRINT("set heaac %d \n", g_heaac);
    return MT_SUCCESS;
}

mt_s32 MTADP_Get_Heaac_Enable(MT_BOOL *heaac)
{
    *heaac = g_heaac;
    MTADP_MPI_INFO_PRINT("get heaac %d \n", g_heaac);
    return MT_SUCCESS;
}

mt_s32 MTADP_SND_SetHdmiMode(MT_UNF_SND_HDMI_MODE_E hdmi_mode)
{
    g_snd_hdmi_mode = hdmi_mode;

    return MT_SUCCESS;
}

mt_s32 MTADP_SND_GetHdmiMode(MT_UNF_SND_HDMI_MODE_E *hdmi_mode)
{
    *hdmi_mode = g_snd_hdmi_mode;

    return MT_SUCCESS;
}

mt_s32 MTADP_SND_SetSpidfMode(MT_UNF_SND_SPDIF_MODE_E spidf_mode)
{
    g_snd_spidf_mode = spidf_mode;

    return MT_SUCCESS;
}

mt_s32 MTADP_SND_GetSpidfMode(MT_UNF_SND_SPDIF_MODE_E *spidf_mode)
{
    *spidf_mode = g_snd_spidf_mode;

    return MT_SUCCESS;
}

mt_s32 MTADP_Set_AudPcmInfo(pcm_info_t pcm)
{
    memset(&g_pcm, 0, sizeof(pcm_info_t));
    g_pcm = pcm;

    return MT_SUCCESS;
}

mt_s32 MTADP_Get_AudPcmInfo(pcm_info_t *pcm)
{
    *pcm = g_pcm;

    return MT_SUCCESS;
}

mt_s32 MTADP_Str_SetNimInfo(mt_nim_config_info nim_info)
{
    g_save_nim_info = nim_info;

    return MT_SUCCESS;
}

mt_s32 MTADP_Str_GetNimInfo(mt_nim_config_info* nim_info)
{
    *nim_info = g_save_nim_info;

    return MT_SUCCESS;
}

mt_s32 MTADP_Disp_StrToFmt(mt_char *pszFmt)
{
    MT_UNF_ENC_FMT_E fmtReturn = MT_UNF_ENC_FMT_BUTT;

    if (NULL == pszFmt)
    {
        return MT_UNF_ENC_FMT_BUTT;
    }

    if (0 == strcasecmp(pszFmt, "1080P_60"))
    {
        fmtReturn = MT_UNF_ENC_FMT_1080P_60;
    }
    else if (0 == strcasecmp(pszFmt, "1080P_50"))
    {
        fmtReturn = MT_UNF_ENC_FMT_1080P_50;
    }
    else if (0 == strcasecmp(pszFmt, "1080P_30"))
    {
        fmtReturn = MT_UNF_ENC_FMT_1080P_30;
    }
    else if (0 == strcasecmp(pszFmt, "1080P_25"))
    {
        fmtReturn = MT_UNF_ENC_FMT_1080P_25;
    }
    else if (0 == strcasecmp(pszFmt, "1080P_24"))
    {
        fmtReturn = MT_UNF_ENC_FMT_1080P_24;
    }
    else if (0 == strcasecmp(pszFmt, "1080i_60"))
    {
        fmtReturn = MT_UNF_ENC_FMT_1080i_60;
    }
    else if (0 == strcasecmp(pszFmt, "1080i_50"))
    {
        fmtReturn = MT_UNF_ENC_FMT_1080i_50;
    }
    else if (0 == strcasecmp(pszFmt, "720P_60"))
    {
        fmtReturn = MT_UNF_ENC_FMT_720P_60;
    }
    else if (0 == strcasecmp(pszFmt, "720P_50"))
    {
        fmtReturn = MT_UNF_ENC_FMT_720P_50;
    }
    else if (0 == strcasecmp(pszFmt, "576P_50"))
    {
        fmtReturn = MT_UNF_ENC_FMT_576P_50;
    }
    else if (0 == strcasecmp(pszFmt, "480P_60"))
    {
        fmtReturn = MT_UNF_ENC_FMT_480P_60;
    }
    else if (0 == strcasecmp(pszFmt, "PAL"))
    {
        fmtReturn = MT_UNF_ENC_FMT_PAL;
    }
    else if (0 == strcasecmp(pszFmt, "NTSC"))
    {
        fmtReturn = MT_UNF_ENC_FMT_NTSC;
    }
    else if (0 == strcasecmp(pszFmt, "1080P_24_FP"))
    {
        fmtReturn = MT_UNF_ENC_FMT_1080P_24_FRAME_PACKING;
    }
    else if (0 == strcasecmp(pszFmt, "720P_60_FP"))
    {
        fmtReturn = MT_UNF_ENC_FMT_720P_60_FRAME_PACKING;
    }
    else if (0 == strcasecmp(pszFmt, "720P_50_FP"))
    {
        fmtReturn = MT_UNF_ENC_FMT_720P_50_FRAME_PACKING;
    }
    else if (0 == strcasecmp(pszFmt, "2160P_24"))
    {
        fmtReturn = MT_UNF_ENC_FMT_3840X2160_24;
    }
    else if (0 == strcasecmp(pszFmt, "2160P_30"))
    {
        fmtReturn = MT_UNF_ENC_FMT_3840X2160_30;
    }
    else
    {
        fmtReturn = MT_UNF_ENC_FMT_720P_50;
        MTADP_MPI_INFO_PRINT("\n!!! Can NOT match format, set format to is '720P_50'/%d.\n\n", MT_UNF_ENC_FMT_720P_50);
    }

    return fmtReturn;
}



mt_s32 MTADP_Disp_Init(MT_UNF_ENC_FMT_E enFormat)
{
    mt_s32                      Ret = 0;
    MT_UNF_DISP_BG_COLOR_S      BgColor = { 0 };
    MT_UNF_DISP_INTF_S          stIntf[2] = { 0 };
    MT_UNF_DISP_OFFSET_S        offset = { 0 };

    MTADP_MPI_FUNCTION_ENTER();

    Ret = MT_UNF_DISP_Init();
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("MT_UNF_DISP_Init failed, Ret=%#x.\n", Ret);
        return Ret;
    }

    /* set display1 interface */
    stIntf[0].enIntfType                = MT_UNF_DISP_INTF_TYPE_YPBPR;
    stIntf[0].unIntf.stYPbPr.u8DacY     = DAC_YPBPR_Y;
    stIntf[0].unIntf.stYPbPr.u8DacPb    = DAC_YPBPR_PB;
    stIntf[0].unIntf.stYPbPr.u8DacPr    = DAC_YPBPR_PR;
    stIntf[1].enIntfType                = MT_UNF_DISP_INTF_TYPE_HDMI;
    stIntf[1].unIntf.enHdmi             = MT_UNF_HDMI_ID_0;
    Ret = MT_UNF_DISP_AttachIntf(MT_UNF_DISPLAY1, &stIntf[0], 2);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_AttachIntf failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    /* set display0 interface */
    stIntf[0].enIntfType            = MT_UNF_DISP_INTF_TYPE_CVBS;
    stIntf[0].unIntf.stCVBS.u8Dac   = DAC_CVBS;
    Ret = MT_UNF_DISP_AttachIntf(MT_UNF_DISPLAY0, &stIntf[0], 1);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_AttachIntf failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    Ret = MT_UNF_DISP_Attach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_Attach failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_DeInit();
        return Ret;
    }
    /* set display1 format*/
    Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY1, enFormat);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_Attach failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }
    if ((MT_UNF_ENC_FMT_1080P_60 == enFormat)
        ||(MT_UNF_ENC_FMT_1080i_60 == enFormat)
        ||(MT_UNF_ENC_FMT_1080P_30 == enFormat)
        ||(MT_UNF_ENC_FMT_1080P_24 == enFormat)
        ||(MT_UNF_ENC_FMT_720P_60 == enFormat)
        ||(MT_UNF_ENC_FMT_480P_60 == enFormat)
        ||(MT_UNF_ENC_FMT_NTSC == enFormat)
        ||(MT_UNF_ENC_FMT_4096X2160_24 == enFormat)
        ||(MT_UNF_ENC_FMT_3840X2160_30 == enFormat)
        ||(MT_UNF_ENC_FMT_3840X2160_24 == enFormat))
    {
        Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY0, MT_UNF_ENC_FMT_NTSC);
        if (MT_SUCCESS != Ret)
        {
            MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_SetFormat failed, Ret=%#x.\n", Ret);
            MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
            MT_UNF_DISP_DeInit();
            return Ret;
        }
    }

    if ((MT_UNF_ENC_FMT_1080P_50 == enFormat)
        ||(MT_UNF_ENC_FMT_1080i_50 == enFormat)
        ||(MT_UNF_ENC_FMT_1080P_25 == enFormat)
        ||(MT_UNF_ENC_FMT_720P_50 == enFormat)
        ||(MT_UNF_ENC_FMT_576P_50 == enFormat)
        ||(MT_UNF_ENC_FMT_PAL == enFormat)
        ||(MT_UNF_ENC_FMT_3840X2160_25 == enFormat))
    {
        Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY0, MT_UNF_ENC_FMT_PAL);
        if (MT_SUCCESS != Ret)
        {
            MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_SetFormat failed, Ret=%#x.\n", Ret);
            MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
            MT_UNF_DISP_DeInit();
            return Ret;
        }
    }

#ifndef ANDROID
    Ret = MT_UNF_DISP_SetVirtualScreen(MT_UNF_DISPLAY1, 1280, 720);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_SetVirtualScreen failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    offset.u32Left      = 0;
    offset.u32Top       = 0;
    offset.u32Right     = 0;
    offset.u32Bottom    = 0;
    /*set display1 screen offset*/
    Ret = MT_UNF_DISP_SetScreenOffset(MT_UNF_DISPLAY1, &offset);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_SetBgColor failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    /*set display0 screen offset*/
    Ret = MT_UNF_DISP_SetScreenOffset(MT_UNF_DISPLAY0, &offset);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_SetBgColor failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }
#endif

    BgColor.u8Red   = 0;
    BgColor.u8Green = 0;
    BgColor.u8Blue  = 0;
    Ret = MT_UNF_DISP_SetBgColor(MT_UNF_DISPLAY1, &BgColor);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_SetBgColor failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    Ret = MT_UNF_DISP_Open(MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_Open DISPLAY1 failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    Ret = MT_UNF_DISP_Open(MT_UNF_DISPLAY0);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_Open DISPLAY0 failed, Ret=%#x.\n", Ret);
        MT_UNF_DISP_Close(MT_UNF_DISPLAY1);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }


#ifdef ANDROID
    MTADP_SURFACE_ATTR_S    stSurAttr;
    MT_UNF_PDM_DISP_PARAM_S stDispParam;

    MTADP_OSD_Init();

    Ret = MT_UNF_PDM_GetBaseParam(MT_UNF_PDM_BASEPARAM_DISP0, &stDispParam);
    if (MT_SUCCESS != Ret)
    {
        stSurAttr.u32Width = 1280;
        stSurAttr.u32Height = 720;
    }
    else
    {
        stSurAttr.u32Width = stDispParam.u32VirtScreenWidth;
        stSurAttr.u32Height = stDispParam.u32VirtScreenHeight;
    }

    stSurAttr.enPixelFormat = MTADP_PF_8888;
    Ret = MTADP_OSD_CreateSurface(&stSurAttr, &g_hSurface);
    if (MT_SUCCESS != Ret)
    {
        MT_UNF_DISP_Close(MT_UNF_DISPLAY0);
        MT_UNF_DISP_Close(MT_UNF_DISPLAY1);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        MTADP_OSD_DeInit();
        return Ret;
    }
#endif
    MTADP_MPI_FUNCTION_EXIT();

    return MT_SUCCESS;
}


mt_s32 MTADP_Disp_DeInit(mt_void)
{
    mt_s32                      Ret = 0;

#ifdef ANDROID
    Ret = MTADP_OSD_DestroySurface(g_hSurface);
    if (MT_SUCCESS != Ret)
    {
        return Ret;
    }

    MTADP_OSD_DeInit();
#endif

    Ret = MT_UNF_DISP_Close(MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_Close failed, Ret=%#x.\n", Ret);
        return Ret;
    }

    Ret = MT_UNF_DISP_Close(MT_UNF_DISPLAY0);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_Close failed, Ret=%#x.\n", Ret);
        return Ret;
    }

    Ret = MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_Detach failed, Ret=%#x.\n", Ret);
        return Ret;
    }

    Ret = MT_UNF_DISP_DeInit();
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_DISP_DeInit failed, Ret=%#x.\n", Ret);
        return Ret;
    }

    //MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);//Rock_huÉ¾³ý´úÂë

    return MT_SUCCESS;
}

/****************************VO Common Interface********************************************/
mt_s32 MTADP_VO_Init(MT_UNF_VO_DEV_MODE_E enDevMode)
{
    mt_s32             Ret = 0;


    Ret = MT_UNF_VO_Init(enDevMode);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_VO_Init failed.\n");
        return Ret;
    }

#if 0
    Ret = MT_UNF_VO_Open(MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_VO_Open failed.\n");
        MT_UNF_VO_DeInit();
        return Ret;
    }
#endif
    return MT_SUCCESS;
}

mt_s32 MTADP_VO_CreatWin(mt_rect_s *pstWinRect,mt_handle *phWin)
{
    mt_s32 Ret = 0;
    MT_UNF_WINDOW_ATTR_S   WinAttr = { 0 };
    memset(&WinAttr, 0, sizeof(MT_UNF_WINDOW_ATTR_S));
    WinAttr.enDisp = MT_UNF_DISPLAY1;
    WinAttr.bVirtual = MT_FALSE;
    WinAttr.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_IGNORE;
    WinAttr.stWinAspectAttr.bUserDefAspectRatio = MT_FALSE;
    WinAttr.stWinAspectAttr.u32UserAspectWidth  = 0;
    WinAttr.stWinAspectAttr.u32UserAspectHeight = 0;
    WinAttr.bUseCropRect = MT_FALSE;
    WinAttr.stInputRect.s32X = 0;
    WinAttr.stInputRect.s32Y = 0;
    WinAttr.stInputRect.s32Width = 0;
    WinAttr.stInputRect.s32Height = 0;

    if (MT_NULL == pstWinRect)
    {
        memset(&WinAttr.stOutputRect, 0x0, sizeof(mt_rect_s));
    }
    else
    {
        memcpy(&WinAttr.stOutputRect,pstWinRect,sizeof(mt_rect_s));
    }

    Ret = MT_UNF_VO_CreateWindow(&WinAttr, phWin);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_VO_CreateWindow failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}


mt_s32 MTADP_VO_CreatPipWin(mt_rect_s *pstWinRect,mt_handle *phWin)
{
    mt_s32 Ret = 0;
    MT_UNF_WINDOW_ATTR_S   WinAttr = { 0 };
    memset(&WinAttr, 0, sizeof(MT_UNF_WINDOW_ATTR_S));
    WinAttr.enDisp = MT_UNF_DISPLAY1;
    WinAttr.bVirtual = MT_FALSE;
    WinAttr.bSetVideoBot = MT_TRUE;
    WinAttr.bUseSubLayer = MT_TRUE;
    WinAttr.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_IGNORE;
    WinAttr.stWinAspectAttr.bUserDefAspectRatio = MT_FALSE;
    WinAttr.stWinAspectAttr.u32UserAspectWidth  = 0;
    WinAttr.stWinAspectAttr.u32UserAspectHeight = 0;
    WinAttr.bUseCropRect = MT_FALSE;
    WinAttr.stInputRect.s32X = 0;
    WinAttr.stInputRect.s32Y = 0;
    WinAttr.stInputRect.s32Width = 0;
    WinAttr.stInputRect.s32Height = 0;

    if (MT_NULL == pstWinRect)
    {
        memset(&WinAttr.stOutputRect, 0x0, sizeof(mt_rect_s));
    }
    else
    {
        memcpy(&WinAttr.stOutputRect,pstWinRect,sizeof(mt_rect_s));
    }

    Ret = MT_UNF_VO_CreateWindow(&WinAttr, phWin);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_VO_CreateWindow failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 MTADP_VO_CreatWinExt(mt_rect_s *pstWinRect,mt_handle *phWin,MT_BOOL bVirtScreen)
{
    mt_s32 Ret = 0;
    MT_UNF_WINDOW_ATTR_S WinAttr = { 0 };
    memset(&WinAttr, 0, sizeof(MT_UNF_WINDOW_ATTR_S));
    WinAttr.enDisp = MT_UNF_DISPLAY1;
    WinAttr.bVirtual = MT_FALSE;
    WinAttr.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_IGNORE;
    WinAttr.stWinAspectAttr.bUserDefAspectRatio = MT_FALSE;
    WinAttr.stWinAspectAttr.u32UserAspectWidth  = 0;
    WinAttr.stWinAspectAttr.u32UserAspectHeight = 0;
    WinAttr.bUseCropRect = MT_FALSE;
    WinAttr.stInputRect.s32X = 0;
    WinAttr.stInputRect.s32Y = 0;
    WinAttr.stInputRect.s32Width = 0;
    WinAttr.stInputRect.s32Height = 0;

    if (MT_NULL == pstWinRect)
    {
        memset(&WinAttr.stOutputRect, 0x0, sizeof(mt_rect_s));
    }
    else
    {
        memcpy(&WinAttr.stOutputRect,pstWinRect,sizeof(mt_rect_s));
    }

    Ret =  MT_UNF_VO_CreateWindowExt(&WinAttr, phWin, bVirtScreen);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_VO_CreateWindowExt failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 MTADP_VO_DeInit()
{
    mt_s32         Ret = 0;

#if 0
    Ret = MT_UNF_VO_Close(MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_VO_Close failed.\n");
        return Ret;
    }
#endif
    Ret = MT_UNF_VO_DeInit();
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_VO_DeInit failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}

static mt_void MT_Snd_gpio_set(void)
{
    MT_UNF_GPIO_Init();
    MT_UNF_GPIO_SetDirBit(MT_UNF_GPIO_7, MT_FALSE);
    MT_UNF_GPIO_WriteBit(MT_UNF_GPIO_7, MT_TRUE);
    MT_UNF_GPIO_Deinit();
}


/*
 @brief snd init
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
mt_s32 MTADP_Snd_Init(mt_void)
{
    mt_s32                  ret = 0;
    MT_UNF_SND_ATTR_S       stAttr = { 0 };

    MTADP_MPI_FUNCTION_ENTER();

    (MT_VOID)MT_Snd_gpio_set();

    ret = MT_UNF_SND_Init();
    if(MT_SUCCESS != ret)
    {
        MTADP_MPI_ERR_PRINT(" MT_UNF_SND_Init failed.\n");
        return ret;
    }

    ret = MT_UNF_SND_GetDefaultOpenAttr(MT_UNF_SND_0, &stAttr);
    if(MT_SUCCESS != ret)
    {
        MTADP_MPI_ERR_PRINT(" MT_UNF_SND_GetDefaultOpenAttr failed.\n");
        MT_UNF_SND_DeInit();
        return ret;
    }

    ret = MT_UNF_SND_Open(MT_UNF_SND_0, &stAttr);
    if(MT_SUCCESS != ret)
    {
        MTADP_MPI_ERR_PRINT(" MT_UNF_SND_Open failed.\n");
        MT_UNF_SND_DeInit();
        return ret;
    }

    return MT_SUCCESS;
}

/*
 @brief Snd Deinit
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
mt_s32 MTADP_Snd_DeInit(mt_void)
{
     mt_s32 ret = 0;

    ret = MT_UNF_SND_Close(MT_UNF_SND_0);
    if(MT_SUCCESS != ret)
    {
        MTADP_MPI_ERR_PRINT(" MT_UNF_SND_Close failed.\n");
        return ret;
    }

    ret = MT_UNF_SND_DeInit();
    if(MT_SUCCESS != ret)
    {
        MTADP_MPI_ERR_PRINT(" MT_UNF_SND_DeInit failed.\n");
        return ret;
    }

    return MT_SUCCESS;
}

#if defined(CONFIG_MT_CHIP_ARIA)
/*****************************************AI Common Interface************************************/
mt_s32 MTADP_AI_Init(MT_UNF_AI_E enAISrc, mt_handle *pAIHandle, mt_handle *pTrackSlave, mt_handle *pATrackVir)
{
    mt_s32                  Ret;
    MT_UNF_AI_ATTR_S        stAitAttr = {0};
    MT_UNF_AUDIOTRACK_ATTR_S  stTrackAttr;

    Ret = MT_UNF_AI_Init();
    if (MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_AI_Init failed.\n");
    }

    Ret = MT_UNF_AI_GetDefaultAttr(enAISrc,&stAitAttr);
    stAitAttr.u32PcmFrameMaxNum = 8;
    if(MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_AI_GetDefaultAttr Failed \n");
        return Ret;
    }

    Ret = MT_UNF_AI_Create(enAISrc, &stAitAttr, pAIHandle);
    if(MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_AI_Create Failed \n");
        return Ret;
    }


    Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_SLAVE, &stTrackAttr);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        return Ret;
    }
    Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, pTrackSlave);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_SND_CreateTrack failed.\n");
        return Ret;
    }

    Ret = MT_UNF_SND_Attach(*pTrackSlave, *pAIHandle);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_SND_Attach failed.\n");
        return Ret;
    }

    Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_VIRTUAL, &stTrackAttr);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        return Ret;
    }

    Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr,pATrackVir);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_SND_CreateTrack failed.\n");
        return Ret;
    }

    Ret = MT_UNF_SND_Attach(*pATrackVir, *pAIHandle);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_SND_Attach failed.\n");
        return Ret;
    }

    Ret = MT_UNF_AI_SetEnable(*pAIHandle, MT_TRUE);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_AI_SetEnable failed.\n");
        return Ret;
    }


    return MT_SUCCESS;
}

mt_s32 MTADP_AI_DeInit(mt_handle hAI, mt_handle hAISlave, mt_handle hAIVir)
{
    mt_s32                  Ret;

    Ret = MT_UNF_AI_SetEnable(hAI, MT_FALSE);
    if (Ret != MT_SUCCESS )
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_AI_SetEnable failed.\n");
        return Ret;
    }

    MT_UNF_SND_Detach(hAIVir, hAI);
    MT_UNF_SND_DestroyTrack(hAIVir);
    MT_UNF_SND_Detach(hAISlave, hAI);
    MT_UNF_SND_DestroyTrack(hAISlave);

    MT_UNF_AI_Destroy(hAI);
    if (Ret != MT_SUCCESS )
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_AI_Destroy failed.\n");
        return Ret;
    }

    MT_UNF_AI_DeInit();
    if (Ret != MT_SUCCESS )
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_AI_DeInit failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}
#endif
#if 0  //v1r3
mt_s32 MTADP_Snd_RegAefAuthLib()
{
    mt_s32 Ret = MT_SUCCESS;

    Ret = MT_UNF_SND_RegisterAefAuthLib("libHA.AUDIO.SRS.effect.auth.so");

    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("\n\n!!! some audio effect authorize lib NOT found. you may NOT able to realize some audio effect process.\n\n");
    }

    return MT_SUCCESS;
}
#endif

mt_s32 MTADP_AVPlay_RegADecLib()
{
    mt_s32 Ret = MT_SUCCESS;

    Ret = MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AMRWB.codec.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.MP3.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.MP2.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AAC.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DOLBYTRUEHD.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DRA.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.TRUEHDPASSTHROUGH.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AMRNB.codec.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.WMA.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.COOK.decode.so");
#ifdef DOLBYPLUS_HACODEC_SUPPORT
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DOLBYPLUS.decode.so");
#endif
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSHD.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSM6.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSPASSTHROUGH.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AC3PASSTHROUGH.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.PCM.decode.so");
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("\n\n!!! some audio codec NOT found. you may NOT able to decode some audio type.\n\n");
    }

    return MT_SUCCESS;
}

mt_s32 MTADP_AVPlay_Init()
{
    mt_s32 Ret;
    Ret = MTADP_AVPlay_RegADecLib();
    Ret |= MT_UNF_AVPLAY_Init();
    return Ret;
}

mt_s32 MTADP_AVPlay_Create(mt_handle *avplay,
                                 mt_u32 u32DemuxId,
                                 MT_UNF_AVPLAY_STREAM_TYPE_E streamtype,
                                 MT_UNF_VCODEC_CAP_LEVEL_E vdeccap,
                                 mt_u32 channelflag)
{
    MT_UNF_AVPLAY_ATTR_S attr;
    mt_handle avhandle;
    MT_UNF_AVPLAY_OPEN_OPT_S maxCapbility;

    if(avplay == MT_NULL)
        return MT_FAILURE;

    if ((u32DemuxId != MPI_DEMUX_PLAY) && (u32DemuxId != MPI_DEMUX_PLAYBACK))
    {
        MTADP_MPI_ERR_PRINT("%d is not a play demux , please select play demux \n", u32DemuxId);
        return MT_FAILURE;
    }

    if(streamtype >= MT_UNF_AVPLAY_STREAM_TYPE_BUTT)
        return MT_FAILURE;

    if(vdeccap >= MT_UNF_VCODEC_CAP_LEVEL_BUTT)
        return MT_FAILURE;

    MT_UNF_AVPLAY_GetDefaultConfig(&attr, streamtype);

    attr.u32DemuxId = u32DemuxId;
    attr.stStreamAttr.u32VidBufSize = 0x300000;
    MT_UNF_AVPLAY_Create(&attr, &avhandle);
    maxCapbility.enDecType = MT_UNF_VCODEC_DEC_TYPE_NORMAL;
    maxCapbility.enCapLevel = vdeccap;
    maxCapbility.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_H264;

    if(channelflag&MT_UNF_AVPLAY_MEDIA_CHAN_AUD)
        MT_UNF_AVPLAY_ChnOpen(avhandle, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);

    if(channelflag&MT_UNF_AVPLAY_MEDIA_CHAN_VID)
       MT_UNF_AVPLAY_ChnOpen(avhandle, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &maxCapbility);

    *avplay = avhandle;

    MTADP_MPI_INFO_PRINT("demux %u create avplay 0x%x  \n", u32DemuxId, (mt_u32)avhandle);

    return MT_SUCCESS;
}

mt_s32 MTADP_AVPlay_SetVdecAttr(mt_handle hAvplay,MT_UNF_VCODEC_TYPE_E enType,MT_UNF_VCODEC_MODE_E enMode)
{
    mt_s32 Ret;
    MT_UNF_VCODEC_ATTR_S        VdecAttr;

    Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
    if (MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed:%#x\n",Ret);
        return Ret;
    }

    VdecAttr.enType = enType;
    VdecAttr.enMode = enMode;
    VdecAttr.u32ErrCover = 100;
    VdecAttr.u32Priority = 3;

    Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr failed.\n");
        return Ret;
    }

    return Ret;
}

#if defined (DOLBYPLUS_HACODEC_SUPPORT)

static mt_void DDPlusCallBack(DOLBYPLUS_EVENT_E Event, mt_void *pUserData)
{
    DOLBYPLUS_STREAM_INFO_S *pstInfo = (DOLBYPLUS_STREAM_INFO_S *)pUserData;

#if 0
    MTADP_MPI_ERR_PRINT( "DDPlusCallBack show info:\n \
                s16StreamType          = %d\n \
                s16Acmod               = %d\n \
                s32BitRate             = %d\n \
                s32SampleRateRate      = %d\n \
                Event                  = %d\n",
                pstInfo->s16StreamType, pstInfo->s16Acmod, pstInfo->s32BitRate, pstInfo->s32SampleRateRate,Event);
#endif
    g_u32DolbyAcmod = pstInfo->s16Acmod;

    if (HA_DOLBYPLUS_EVENT_SOURCE_CHANGE == Event)
    {
        g_bDrawChnBar = MT_TRUE;
        //printf("DDPlusCallBack enent !\n");
    }
    return;
}

#endif//DOLBYPLUS_HACODEC_SUPPORT

mt_s32 MTADP_AVPlay_GetAdecAttr(mt_handle hAvplay,HA_CODEC_ID_E mAdecType,MT_UNF_ACODEC_ATTR_S *p_AdecAttr)
{
    mt_s32 ret = 0;
    WAV_FORMAT_S stWavFormat = { 0 };

    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, p_AdecAttr);
    if (ret != MT_SUCCESS) {
        MTADP_MPI_ERR_PRINT("get adec attr failed!ret=0x%x\n",ret);
        return MT_FAILURE;
    }
    p_AdecAttr->enType = mAdecType;

    if (HA_AUDIO_ID_PCM == p_AdecAttr->enType)
    {
        /* set pcm wav format here base on pcm file 48k.raw */
        stWavFormat.nChannels = 2;
        stWavFormat.nSamplesPerSec = 48000;
        stWavFormat.wBitsPerSample = 16;
        stWavFormat.cbExtWord[0] = 0;
        stWavFormat.cbExtWord[1] = 1;
        HA_PCM_DecGetDefalutOpenParam(&(p_AdecAttr->stDecodeParam),&stWavFormat);
        MTADP_MPI_ERR_PRINT("please make sure the attributes of PCM stream is tme same as defined in function of \"MTADP_AVPlay_SetAdecAttr\"? \n");
        MTADP_MPI_ERR_PRINT("(nChannels = 2, wBitsPerSample = 16, nSamplesPerSec = 48000, isBigEndian = MT_FALSE) \n");
    }
#if 0
    else if (HA_AUDIO_ID_G711 == AdecAttr.enType)
    {
         HA_G711_GetDecDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
#endif
    else if (HA_AUDIO_ID_MP2 == p_AdecAttr->enType)
    {
         HA_MP2_DecGetDefalutOpenParam(&(p_AdecAttr->stDecodeParam));
    }
    else if (HA_AUDIO_ID_AAC == p_AdecAttr->enType)
    {
         HA_AAC_DecGetDefalutOpenParam(&(p_AdecAttr->stDecodeParam));
    }
    else if (HA_AUDIO_ID_MP3 == p_AdecAttr->enType)
    {
         HA_MP3_DecGetDefalutOpenParam(&(p_AdecAttr->stDecodeParam));
    }
#if 0
    else if (HA_AUDIO_ID_AMRNB== AdecAttr.enType)
    {
        AMRNB_DECODE_OPENCONFIG_S *pstConfig = (AMRNB_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_AMRNB_GetDecDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        pstConfig->enFormat = AMRNB_MIME;
    }
    else if (HA_AUDIO_ID_AMRWB== AdecAttr.enType)
    {
        AMRWB_DECODE_OPENCONFIG_S *pstConfig = (AMRWB_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_AMRWB_GetDecDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        pstConfig->enFormat = AMRWB_FORMAT_MIME;
    }
#endif
    else if (HA_AUDIO_ID_AC3PASSTHROUGH== p_AdecAttr->enType)
    {
        HA_AC3PASSTHROUGH_DecGetDefalutOpenParam(&(p_AdecAttr->stDecodeParam));
        p_AdecAttr->stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
    }
    else if(HA_AUDIO_ID_DTSPASSTHROUGH ==  p_AdecAttr->enType)
    {
            HA_DTSPASSTHROUGH_DecGetDefalutOpenParam(&(p_AdecAttr->stDecodeParam));
             p_AdecAttr->stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
    }
    else if (HA_AUDIO_ID_TRUEHD == p_AdecAttr->enType)
    {
        HA_TRUEHD_DecGetDefalutOpenParam(&(p_AdecAttr->stDecodeParam));
        #if 0
        if (HD_DEC_MODE_THRU != enMode)
        {
            MTADP_MPI_ERR_PRINT(" MLP decoder enMode(%d) error (mlp only support hbr Pass-through only).\n", enMode);
            return -1;
        }
        #endif
        p_AdecAttr->stDecodeParam.enDecMode = HD_DEC_MODE_THRU;        /* truehd just support pass-through */
        MTADP_MPI_ERR_PRINT(" TrueHD decoder(HBR Pass-through only).\n");
    }
    else if (HA_AUDIO_ID_DOLBY_TRUEHD == p_AdecAttr->enType)
    {
          TRUEHD_DECODE_OPENCONFIG_S *pstConfig = (TRUEHD_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DOLBY_TRUEHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DOLBY_TRUEHD_DecGetDefalutOpenParam(&(p_AdecAttr->stDecodeParam), pstConfig);
    }
    else if (HA_AUDIO_ID_DOLBY_CONVERT == p_AdecAttr->enType)
    {
        TRUEHD_DECODE_OPENCONFIG_S *pstConfig = (TRUEHD_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DOLBY_CONVERT_DecGetDefalutOpenConfig(pstConfig);
        HA_DOLBY_CONVERT_DecGetDefalutOpenParam(&(p_AdecAttr->stDecodeParam), pstConfig);
    }
    else if (HA_AUDIO_ID_DTSHD == p_AdecAttr->enType)
    {
        DTSHD_DECODE_OPENCONFIG_S *pstConfig = (DTSHD_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DTSHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSHD_DecGetDefalutOpenParam(&(p_AdecAttr->stDecodeParam), pstConfig);
        p_AdecAttr->stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
    }
    else if (HA_AUDIO_ID_DTSM6 == p_AdecAttr->enType)
    {
        DTSM6_DECODE_OPENCONFIG_S *pstConfig = (DTSM6_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DTSM6_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSM6_DecGetDefalutOpenParam(&(p_AdecAttr->stDecodeParam), pstConfig);
    }
#if defined (DOLBYPLUS_HACODEC_SUPPORT)
    else if (HA_AUDIO_ID_DOLBY_PLUS == p_AdecAttr->enType)
    {
        DOLBYPLUS_DECODE_OPENCONFIG_S *pstConfig = (DOLBYPLUS_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DOLBYPLUS_DecGetDefalutOpenConfig(pstConfig);
        pstConfig->pfnEvtCbFunc[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = DDPlusCallBack;
        pstConfig->pAppData[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = &g_stDDpStreamInfo;
        /* Dolby DVB Broadcast default settings */
        pstConfig->enDrcMode = DOLBYPLUS_DRC_RF;
        pstConfig->enDmxMode = DOLBYPLUS_DMX_SRND;
        HA_DOLBYPLUS_DecGetDefalutOpenParam(&(p_AdecAttr->stDecodeParam), pstConfig);
        //AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
    }
#endif
    else if(HA_AUDIO_ID_DRA == p_AdecAttr->enType)
    {
//       HA_DRA_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
         HA_DRA_DecGetOpenParam_MultichPcm(&(p_AdecAttr->stDecodeParam));
    }
    else if(HA_AUDIO_ID_COOK == p_AdecAttr->enType || HA_AUDIO_ID_AMRNB ==p_AdecAttr->enType
        || HA_AUDIO_ID_AMRWB == p_AdecAttr->enType)
    {
        HA_FFMPEG_DECODE_OPENCONFIG_S *pstConfig = (HA_FFMPEG_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_FFMPEG_DecGetDefalutOpenConfig(pstConfig);
        HA_FFMPEGC_DecGetDefalutOpenParam(&(p_AdecAttr->stDecodeParam), pstConfig);
        MTADP_MPI_INFO_PRINT("cook dec set ffmpeg dec param \n");
    }
    return MT_SUCCESS;
}

mt_s32 MTADP_AVPlay_SetAdecAttr(mt_handle hAvplay, mt_u32 enADecType, MT_HA_DECODEMODE_E enMode, mt_s32 isCoreOnly)
{
    MT_UNF_ACODEC_ATTR_S AdecAttr = { 0 };
    WAV_FORMAT_S stWavFormat = { 0 };

    MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);
    AdecAttr.enType = enADecType;

    if (HA_AUDIO_ID_PCM == AdecAttr.enType)
    {
        pcm_info_t pcm;

        memset(&pcm, 0, sizeof(pcm_info_t));
        MTADP_Get_AudPcmInfo(&pcm);
        /* set pcm wav format here base on pcm file 48k.raw */
        stWavFormat.nChannels = pcm.channels;
        stWavFormat.nSamplesPerSec = pcm.sample_rate;
        stWavFormat.wBitsPerSample = pcm.bits_per_coded_sample;

        if (pcm.is_big_endian == 1)
        {
            stWavFormat.cbExtWord[0] = 1;
        }
        HA_PCM_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam),&stWavFormat);
        MTADP_MPI_INFO_PRINT("(Channels = %d, BitsPerSample = %d, SamplesPerSec = %d, isBigEndian = %d) \n",pcm.channels,
            pcm.bits_per_coded_sample, pcm.sample_rate, pcm.is_big_endian);
    }
#if 0
    else if (HA_AUDIO_ID_G711 == AdecAttr.enType)
    {
         HA_G711_GetDecDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
#endif
    else if (HA_AUDIO_ID_MP2 == AdecAttr.enType)
    {
         HA_MP2_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if (HA_AUDIO_ID_AAC == AdecAttr.enType)
    {
         HA_AAC_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if (HA_AUDIO_ID_MP3 == AdecAttr.enType)
    {
         HA_MP3_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
#if 0
    else if (HA_AUDIO_ID_AMRNB== AdecAttr.enType)
    {
        AMRNB_DECODE_OPENCONFIG_S *pstConfig = (AMRNB_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_AMRNB_GetDecDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        pstConfig->enFormat = AMRNB_MIME;
    }
    else if (HA_AUDIO_ID_AMRWB== AdecAttr.enType)
    {
        AMRWB_DECODE_OPENCONFIG_S *pstConfig = (AMRWB_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_AMRWB_GetDecDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        pstConfig->enFormat = AMRWB_FORMAT_MIME;
    }
#endif
    else if (HA_AUDIO_ID_AC3PASSTHROUGH== AdecAttr.enType)
    {
        HA_AC3PASSTHROUGH_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
    }
    else if(HA_AUDIO_ID_DTSPASSTHROUGH ==  AdecAttr.enType)
    {
                HA_DTSPASSTHROUGH_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
             AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
    }
    else if (HA_AUDIO_ID_TRUEHD == AdecAttr.enType)
    {
        HA_TRUEHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        if (HD_DEC_MODE_THRU != enMode)
        {
            MTADP_MPI_ERR_PRINT(" MLP decoder enMode(%d) error (mlp only support hbr Pass-through only).\n", enMode);
            return -1;
        }

        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;        /* truehd just support pass-through */
        MTADP_MPI_ERR_PRINT(" TrueHD decoder(HBR Pass-through only).\n");
    }
    else if (HA_AUDIO_ID_DOLBY_TRUEHD == AdecAttr.enType)
    {
          TRUEHD_DECODE_OPENCONFIG_S *pstConfig = (TRUEHD_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DOLBY_TRUEHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DOLBY_TRUEHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
    else if (HA_AUDIO_ID_DOLBY_CONVERT == AdecAttr.enType)
    {
        TRUEHD_DECODE_OPENCONFIG_S *pstConfig = (TRUEHD_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DOLBY_CONVERT_DecGetDefalutOpenConfig(pstConfig);
        HA_DOLBY_CONVERT_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
    else if (HA_AUDIO_ID_DTSHD == AdecAttr.enType)
    {
        DTSHD_DECODE_OPENCONFIG_S *pstConfig = (DTSHD_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DTSHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
    }
    else if (HA_AUDIO_ID_DTSM6 == AdecAttr.enType)
    {
        DTSM6_DECODE_OPENCONFIG_S *pstConfig = (DTSM6_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DTSM6_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSM6_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
#if defined (DOLBYPLUS_HACODEC_SUPPORT)
    else if (HA_AUDIO_ID_DOLBY_PLUS == AdecAttr.enType)
    {
        DOLBYPLUS_DECODE_OPENCONFIG_S *pstConfig = (DOLBYPLUS_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DOLBYPLUS_DecGetDefalutOpenConfig(pstConfig);
        pstConfig->pfnEvtCbFunc[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = DDPlusCallBack;
        pstConfig->pAppData[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = &g_stDDpStreamInfo;
        /* Dolby DVB Broadcast default settings */
        pstConfig->enDrcMode = DOLBYPLUS_DRC_RF;
        pstConfig->enDmxMode = DOLBYPLUS_DMX_SRND;
        HA_DOLBYPLUS_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        //AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
    }
#endif
    else if(HA_AUDIO_ID_DRA == AdecAttr.enType)
    {
//       HA_DRA_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
         HA_DRA_DecGetOpenParam_MultichPcm(&(AdecAttr.stDecodeParam));
    }
    else if(HA_AUDIO_ID_COOK == AdecAttr.enType || HA_AUDIO_ID_AMRNB ==AdecAttr.enType
        || HA_AUDIO_ID_AMRWB == AdecAttr.enType)
    {
        HA_FFMPEG_DECODE_OPENCONFIG_S *pstConfig = (HA_FFMPEG_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_FFMPEG_DecGetDefalutOpenConfig(pstConfig);
        HA_FFMPEGC_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        MTADP_MPI_INFO_PRINT("cook dec set ffmpeg dec param \n");
    }

    MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);

    return MT_SUCCESS;
}

mt_s32 MTADP_AENC_GetAttr(MT_UNF_AENC_ATTR_S *pAencAttr, mt_void *pstConfig)
{
    if(NULL == pAencAttr || NULL == pstConfig)
    {
        return MT_ERR_AENC_NULL_PTR;
    }

    if (HA_AUDIO_ID_AAC == pAencAttr->enAencType)
    {
        HA_AAC_GetEncDefaultOpenParam(&(pAencAttr->sOpenParam), pstConfig);
        MTADP_MPI_INFO_PRINT("u32DesiredSampleRate =%d\n", pAencAttr->sOpenParam.u32DesiredSampleRate);
    }
    else
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;

}

mt_s32 MTADP_AVPlay_PlayProg(mt_handle hAvplay,PMT_COMPACT_TBL *pProgTbl,mt_u32 ProgNum,MT_BOOL bAudPlay)
{
    MT_UNF_AVPLAY_STOP_OPT_S    Stop;
    mt_u32                  VidPid;
    mt_u32                  AudPid;
    mt_u32                  PcrPid;
    MT_UNF_VCODEC_TYPE_E    enVidType;
    mt_u32                  u32AudType;
    mt_s32                  Ret;
    mt_s32                  i=0;
    Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    Stop.u32TimeoutMs = 0;
    Ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &Stop);
    if (MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_AVPLAY_Stop failed.\n");
        return Ret;
    }

    printf("UUUU 0000 ProgNum = %x, prog_num = %x \n", ProgNum, pProgTbl->prog_num);
    {
        for(i=0;i<pProgTbl->prog_num;i++)
        {
            printf("VElementNum = %x,  AElementNum=%x\n", pProgTbl->proginfo[i].VElementNum, pProgTbl->proginfo[i].AElementNum);
            printf("vpid = %x , videoType = %x \n",
                pProgTbl->proginfo[i].VElementPid, pProgTbl->proginfo[i].VideoType);
            printf("apid = %x , audioType = %x \n",
                pProgTbl->proginfo[i].AElementPid, pProgTbl->proginfo[i].AudioType);
        }
    }
    ProgNum = ProgNum % pProgTbl->prog_num;

    //printf("UUUU 1111 VElementNum = %x \n", pProgTbl->proginfo[ProgNum].VElementNum);
    //printf("UUUU 2222 vpid = %x , videoType = %x \n",
    //  pProgTbl->proginfo[ProgNum].VElementPid, pProgTbl->proginfo[ProgNum].VideoType);
    if (pProgTbl->proginfo[ProgNum].VElementNum > 0 )
    {
        VidPid = pProgTbl->proginfo[ProgNum].VElementPid;
        enVidType = pProgTbl->proginfo[ProgNum].VideoType;
        printf("UUUU 2222 vpid = %x \n", VidPid);
    }
    else
    {
        VidPid = INVALID_TSPID;
        enVidType = MT_UNF_VCODEC_TYPE_BUTT;
    }

    if (pProgTbl->proginfo[ProgNum].AElementNum > 0)
    {
        AudPid  = pProgTbl->proginfo[ProgNum].AElementPid;
        u32AudType = pProgTbl->proginfo[ProgNum].AudioType;
    }
    else
    {
        AudPid = INVALID_TSPID;
        u32AudType = 0xffffffff;
    }

    PcrPid = pProgTbl->proginfo[ProgNum].PcrPid;
    printf("%s =========%d\n",__FILE__,PcrPid);
    if (INVALID_TSPID != PcrPid)
    {
        Ret = MT_UNF_AVPLAY_SetAttr(hAvplay,MT_UNF_AVPLAY_ATTR_ID_PCR_PID,&PcrPid);
        if (MT_SUCCESS != Ret)
        {
            MTADP_MPI_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr failed.\n");

            PcrPid = INVALID_TSPID;
            Ret = MT_UNF_AVPLAY_SetAttr(hAvplay,MT_UNF_AVPLAY_ATTR_ID_PCR_PID,&PcrPid);
            if (MT_SUCCESS != Ret)
            {
                MTADP_MPI_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr failed.\n");
                return Ret;
            }
        }
    }
    printf("%s =========%d  vidpid = %x\n",__FILE__,__LINE__, VidPid);
    if (VidPid != INVALID_TSPID)
    {
        Ret = MTADP_AVPlay_SetVdecAttr(hAvplay,enVidType,MT_UNF_VCODEC_MODE_NORMAL);
        Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        printf("%s =========%d\n",__FILE__,Ret);
        if (Ret != MT_SUCCESS)
        {
            MTADP_MPI_ERR_PRINT("call MTADP_AVPlay_SetVdecAttr failed.\n");
            return Ret;
        }
        printf("%s =========%d\n",__FILE__,__LINE__);

        /*set compress attr*/
        MT_UNF_VCODEC_ATTR_S VcodecAttr;
        Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);

        if (MT_UNF_VCODEC_TYPE_VC1 == enVidType)
        {
            VcodecAttr.unExtAttr.stVC1Attr.bAdvancedProfile = 1;
            VcodecAttr.unExtAttr.stVC1Attr.u32CodecVersion = 8;
        }

        if (MT_UNF_VCODEC_TYPE_VP6 == enVidType)
        {
            VcodecAttr.unExtAttr.stVP6Attr.bReversed = 0;
        }

        VcodecAttr.enType = enVidType;
        VcodecAttr.u32UseDescInfoFlag = 1;
        Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if (Ret != MT_SUCCESS)
        {
            MTADP_MPI_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr failed.\n");
            return Ret;
        }

        Ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
        if (Ret != MT_SUCCESS)
        {
            MTADP_MPI_ERR_PRINT("call MT_UNF_AVPLAY_Start failed.\n");
            return Ret;
        }
    }
    printf("%s =========%d\n",__FILE__,__LINE__);

    if (MT_TRUE == bAudPlay && AudPid != INVALID_TSPID)
    {
        //u32AudType = HA_AUDIO_ID_DTSHD;
        printf("u32AudType = %#x\n",u32AudType);
        Ret  = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);//Rock_huÉ¾³ý´úÂë
        printf("%s =========%d audiopid %d \n",__FILE__,__LINE__,AudPid);
        Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if (MT_SUCCESS != Ret)
        {
            MTADP_MPI_ERR_PRINT("MTADP_AVPlay_SetAdecAttr failed:%#x\n",Ret);
            return Ret;
        }
        printf("%s =========%d\n",__FILE__,__LINE__);
        Ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
        if (Ret != MT_SUCCESS)
        {
            MTADP_MPI_ERR_PRINT("call MT_UNF_AVPLAY_Start to start audio failed.\n");
            //return Ret;
        }
    }

    printf("%s =========%d\n",__FILE__,__LINE__);
    return MT_SUCCESS;
}

mt_s32 MTADP_AVPlay_PlayAud(mt_handle hAvplay,PMT_COMPACT_TBL *pProgTbl,mt_u32 ProgNum)
{
    mt_u32                  AudPid = 0;
    mt_s32                  Ret = 0;

    Ret = MT_UNF_AVPLAY_Stop(hAvplay,MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_AVPLAY_Stop failed.\n");
        return Ret;
    }

    ProgNum = ProgNum % pProgTbl->prog_num;
    if (pProgTbl->proginfo[ProgNum].AElementNum > 0)
    {
        AudPid  = pProgTbl->proginfo[ProgNum].AElementPid;
    }
    else
    {
        AudPid = INVALID_TSPID;
    }

    if (AudPid != INVALID_TSPID)
    {
        //Ret  = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);//Rock_hu É¾³ý´úÂë
        Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if (MT_SUCCESS != Ret)
        {
            MTADP_MPI_ERR_PRINT("MTADP_AVPlay_SetAdecAttr failed:%#x\n",Ret);
            return Ret;
        }

        Ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
        if (Ret != MT_SUCCESS)
        {
            MTADP_MPI_ERR_PRINT("call MT_UNF_AVPLAY_Start failed.\n");
            return Ret;
        }
    }

    return MT_SUCCESS;
}

mt_s32 MTADP_AVPlay_SwitchAud(mt_handle hAvplay,mt_u32 AudPid, mt_u32 u32AudType)
{
    mt_s32 Ret = MT_SUCCESS;

    if (AudPid == INVALID_TSPID)
    {
        MTADP_MPI_ERR_PRINT("%s, audio pid is invalid!\n", __func__);
        return MT_FAILURE;
    }

    Ret = MT_UNF_AVPLAY_Stop(hAvplay,MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_AVPLAY_Stop failed.\n");
        return Ret;
    }


    //Ret  = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1); //Rock_huÉ¾³ý´úÂë
    Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
    if (MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("MTADP_AVPlay_SetAdecAttr failed:%#x\n",Ret);
        return Ret;
    }

    Ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_AVPLAY_Start failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}

//----------------------------------------------------------------------------//
//--------------------------- Show Logo Adapt API ----------------------------//

/** Show Logo's Window handle */
mt_handle g_hLogoWin = MT_INVALID_HANDLE;
/** Show Logo's AVPlay handle */
mt_handle g_hLogoAvplay = MT_INVALID_HANDLE;

/**
 * @brief Clear Video Logo
 *
 * @retval
 *     MT_SUCCESS: success
 *     Other: failed
 */
mt_s32 MTADP_Clear_LOGO(mt_void)
{
    mt_s32 Ret;
    MT_UNF_AVPLAY_STOP_OPT_S Stop;

    if (g_hLogoAvplay == MT_INVALID_HANDLE || g_hLogoWin == MT_INVALID_HANDLE)
    {
        MTADP_MPI_ERR_PRINT("%s: Invalid Logo's AVPlay(%x) or Win(%x) handle\n\n",__FUNCTION__,
                            (mt_u32)g_hLogoAvplay,(mt_u32)g_hLogoWin);
        return MT_FAILURE;
    }

    Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    Stop.u32TimeoutMs = 0;
    Ret = MT_UNF_AVPLAY_Stop(g_hLogoAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &Stop);
    MTADP_MPI_INFO_PRINT("%s: AVPLAY_VSTOP \n\n",__FUNCTION__);

    Ret |= MT_UNF_VO_SetWindowEnable(g_hLogoWin, MT_FALSE);
    Ret |= MT_UNF_VO_DetachWindow(g_hLogoWin, g_hLogoAvplay);
    MTADP_MPI_INFO_PRINT("%s: WIN_DETATCH \n\n",__FUNCTION__);

    Ret |= MT_UNF_AVPLAY_ChnClose(g_hLogoAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    MTADP_MPI_INFO_PRINT("%s: VCHN_CLOSE \n\n",__FUNCTION__);

    Ret |= MT_UNF_AVPLAY_Destroy(g_hLogoAvplay);
    g_hLogoAvplay = MT_INVALID_HANDLE;
    MTADP_MPI_INFO_PRINT("%s: AVPLAY_DESTROY \n\n",__FUNCTION__);

    Ret |= MT_UNF_AVPLAY_DeInit();
    MTADP_MPI_INFO_PRINT("%s: AVPLAY_DEINIT \n\n",__FUNCTION__);

    Ret |= MT_UNF_VO_DestroyWindow(g_hLogoWin);
    g_hLogoWin = MT_INVALID_HANDLE;
    Ret |= MTADP_VO_DeInit();

    Ret |= MTADP_Disp_DeInit();
    MTADP_MPI_INFO_PRINT("%s: DISP_DEINIT \n\n",__FUNCTION__);

    return Ret;
}

/**
 * @brief Decode and show Video Logo(Video Intra Frame)
 *
 * @param[in] VdecType Video Intra Frame type, shall be:
 *                     MT_UNF_VCODEC_TYPE_MPEG2,
 *                     MT_UNF_VCODEC_TYPE_MPEG4,
 *                     MT_UNF_VCODEC_TYPE_AVS,
 *                     MT_UNF_VCODEC_TYPE_H263,
 *                     MT_UNF_VCODEC_TYPE_H264,
 *                     MT_UNF_VCODEC_TYPE_VC1,
 *                     MT_UNF_VCODEC_TYPE_HEVC
 * @param[in] p_data Video(Intra Frame) elementary stream data
 * @param[in] datalen Video(Intra Frame) elementary stream data's length in bytes
 *
 * @retval
 *     MT_SUCCESS: success
 *     Other: failed
 */
mt_s32 MTADP_Show_LOGO(MT_UNF_VCODEC_TYPE_E VdecType, mt_void *p_data, mt_u32 datalen)
{
#define PUSH_ES_TIMES           20
#define WAIT_BUFFER_DELAY       10000   //10ms
#define DEFAULT_ES_BUFFER_SIZE  0x10000 //64k

    mt_s32 Ret;

    MT_UNF_AVPLAY_ATTR_S AvplayAttr;
    MT_UNF_SYNC_ATTR_S AvSyncAttr;
    MT_UNF_ENC_FMT_E g_enDefaultFmt = MT_UNF_ENC_FMT_1080i_50;
    MT_BOOL bAdvancedProfil = 1;
    mt_u32  u32CodecVersion = 8;
    MT_UNF_AVPLAY_OPEN_OPT_S stMaxCapbility;
    MT_UNF_AVPLAY_STOP_OPT_S StopOpt;

    MT_UNF_STREAM_BUF_S StreamBuf;
    mt_u32 push_cnt = 0;
    mt_u32 es_buf_size = MAX(DEFAULT_ES_BUFFER_SIZE, datalen);

    if (p_data == NULL || datalen <= 0)
    {
        MTADP_MPI_ERR_PRINT("%s: Invalid data or length(%p, %u)!\n",__FUNCTION__,p_data,datalen);
        return MT_FAILURE;
    }

    //check VdecType
    if (VdecType != MT_UNF_VCODEC_TYPE_MPEG2
        && VdecType != MT_UNF_VCODEC_TYPE_MPEG4
        && VdecType != MT_UNF_VCODEC_TYPE_AVS
        && VdecType != MT_UNF_VCODEC_TYPE_H263
        && VdecType != MT_UNF_VCODEC_TYPE_H264
        && VdecType != MT_UNF_VCODEC_TYPE_VC1
        && VdecType != MT_UNF_VCODEC_TYPE_HEVC)
    {
        MTADP_MPI_INFO_PRINT("%s: Invalid VdecType(%d)!\n",__FUNCTION__,VdecType);
        return MT_FAILURE;
    }

    Ret = MT_UNF_AVPLAY_Init();

    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("%s: call MT_UNF_AVPLAY_Init failed.\n",__FUNCTION__);
        goto AVPLAY_DEINIT;
    }

    Ret  = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    AvplayAttr.stStreamAttr.u32AudBufSize = (192*1024);
#endif
    Ret |= MT_UNF_AVPLAY_Create(&AvplayAttr, &g_hLogoAvplay);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("%s: call MT_UNF_AVPLAY_Create failed.\n",__FUNCTION__);
        goto AVPLAY_DEINIT;
    }

    Ret = MT_UNF_AVPLAY_GetAttr(g_hLogoAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
    AvSyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
    Ret |= MT_UNF_AVPLAY_SetAttr(g_hLogoAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
    if (MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("%s: call MT_UNF_AVPLAY_SetAttr failed.\n",__FUNCTION__);
        goto AVPLAY_DESTROY;
    }

    Ret = MTADP_Disp_Init(g_enDefaultFmt);

    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("%s: call DispInit failed.\n",__FUNCTION__);
        goto AVPLAY_DESTROY;
    }


    Ret  = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    MTADP_MPI_INFO_PRINT("[%s] line%d\n",__FUNCTION__,__LINE__);
    Ret |= MTADP_VO_CreatWin(MT_NULL, &g_hLogoWin);
    MTADP_MPI_INFO_PRINT("[%s] line%d\n",__FUNCTION__,__LINE__);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("%s: call VoInit failed.\n",__FUNCTION__);
        MTADP_VO_DeInit();
        goto DISP_DEINIT;
    }

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    stMaxCapbility.enCapLevel   = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
    stMaxCapbility.enDecType    = MT_UNF_VCODEC_DEC_TYPE_BUTT;
    if (MT_UNF_VCODEC_TYPE_MVC == VdecType)
    {
        //Symphony not support MVC in fact.
        stMaxCapbility.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_MVC;
    }
    else if (MT_UNF_VCODEC_TYPE_H264 == VdecType)
    {
        stMaxCapbility.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_H264;
    }
    else
    {
        stMaxCapbility.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_MPEG;    //!H264
    }
#else
    if (MT_UNF_VCODEC_TYPE_MVC == VdecType)
    {
        stMaxCapbility.enCapLevel      = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
        stMaxCapbility.enDecType       = MT_UNF_VCODEC_DEC_TYPE_BUTT;
        stMaxCapbility.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_MVC;
    }
    else
    {
        stMaxCapbility.enCapLevel      = MT_UNF_VCODEC_CAP_LEVEL_4096x2160;
        stMaxCapbility.enDecType       = MT_UNF_VCODEC_DEC_TYPE_BUTT;
        stMaxCapbility.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_BUTT;
    }
#endif

    Ret = MT_UNF_AVPLAY_ChnOpen(g_hLogoAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &stMaxCapbility);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("%s: call MT_UNF_AVPLAY_ChnOpen failed.\n",__FUNCTION__);
        goto VO_DEINIT;
    }

    /*set compress attr*/
    MT_UNF_VCODEC_ATTR_S VcodecAttr;
    Ret = MT_UNF_AVPLAY_GetAttr(g_hLogoAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);

    if (MT_UNF_VCODEC_TYPE_VC1 == VdecType)
    {
        VcodecAttr.unExtAttr.stVC1Attr.bAdvancedProfile = bAdvancedProfil;
        VcodecAttr.unExtAttr.stVC1Attr.u32CodecVersion = u32CodecVersion;
    }

    if (MT_UNF_VCODEC_TYPE_VP6 == VdecType)
    {
        VcodecAttr.unExtAttr.stVP6Attr.bReversed = 0;
    }

    VcodecAttr.enType = VdecType;
    VcodecAttr.u32UseDescInfoFlag = 0;
    Ret |= MT_UNF_AVPLAY_SetAttr(g_hLogoAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
    if (MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("%s: call MT_UNF_AVPLAY_SetAttr failed.\n",__FUNCTION__);
        goto VCHN_CLOSE;
    }

    Ret = MT_UNF_VO_AttachWindow(g_hLogoWin, g_hLogoAvplay);

    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("%s: call MT_UNF_VO_AttachWindow failed.\n",__FUNCTION__);
        goto VCHN_CLOSE;
    }

    Ret = MT_UNF_VO_SetWindowEnable(g_hLogoWin, MT_TRUE);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("%s: call MT_UNF_VO_SetWindowEnable failed.\n",__FUNCTION__);
        goto WIN_DETATCH;
    }

    Ret = MTADP_AVPlay_SetVdecAttr(g_hLogoAvplay, VdecType, MT_UNF_VCODEC_MODE_NORMAL);

    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("%s: call MTADP_AVPlay_SetVdecAttr failed.\n",__FUNCTION__);
        goto WIN_DETATCH;
    }

    Ret = MT_UNF_AVPLAY_Start(g_hLogoAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if (Ret != MT_SUCCESS)
    {
        MTADP_MPI_ERR_PRINT("%s: call MT_UNF_AVPLAY_Start failed.\n",__FUNCTION__);
        goto WIN_DETATCH;
    }

    while (push_cnt < PUSH_ES_TIMES)
    {
        Ret = MT_UNF_AVPLAY_GetBuf(g_hLogoAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID,
                                    es_buf_size, &StreamBuf, 0);
        //printf("push_data_to_esbuf: GetBuf %x, %d, ret %d\n",
        //      StreamBuf.pu8Data,StreamBuf.u32Size,Ret);

        if (MT_SUCCESS == Ret
            && StreamBuf.u32Size > 0 && StreamBuf.u32Size <= es_buf_size
            && StreamBuf.pu8Data != NULL)
        {
            memset(StreamBuf.pu8Data, 0, StreamBuf.u32Size);

            memcpy(StreamBuf.pu8Data, p_data, datalen);

            MTADP_MPI_INFO_PRINT("%s: !!!!!!!!!!!!!!!!!! memcpy over\n",__FUNCTION__);

            Ret = MT_UNF_AVPLAY_PutBuf(g_hLogoAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID,
                                         StreamBuf.u32Size, 0);

            if (Ret != MT_SUCCESS)
            {
                MTADP_MPI_ERR_PRINT("%s: call MT_UNF_AVPLAY_PutBuf failed.\n",__FUNCTION__);
            }
        }
        else
        {
            MTADP_MPI_WARN_PRINT("%s: Warning, MT_UNF_AVPLAY_GetBuf(%u) return %d!\n",__FUNCTION__,
                                    es_buf_size,Ret);
        }

        /* wait for buffer */
        MT_USLEEP(WAIT_BUFFER_DELAY);
        push_cnt ++;
    }

    StopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    StopOpt.u32TimeoutMs = 0;

    MTADP_MPI_INFO_PRINT("%s: call MT_UNF_AVPLAY_Stop \n\n",__FUNCTION__);
    Ret = MT_UNF_AVPLAY_Stop(g_hLogoAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &StopOpt);

    return MT_SUCCESS;

WIN_DETATCH:
    MT_UNF_VO_SetWindowEnable(g_hLogoWin, MT_FALSE);
    MT_UNF_VO_DetachWindow(g_hLogoWin, g_hLogoAvplay);

VCHN_CLOSE:
    MT_UNF_AVPLAY_ChnClose(g_hLogoAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

VO_DEINIT:
    MT_UNF_VO_DestroyWindow(g_hLogoWin);
    g_hLogoWin = MT_INVALID_HANDLE;
    MTADP_VO_DeInit();

DISP_DEINIT:
    MTADP_Disp_DeInit();

AVPLAY_DESTROY:
    MT_UNF_AVPLAY_Destroy(g_hLogoAvplay);
    g_hLogoAvplay = MT_INVALID_HANDLE;

AVPLAY_DEINIT:
    MT_UNF_AVPLAY_DeInit();

    return MT_FAILURE;
}

static MT_VOID MTADP_Standby_Mode_Deinit(MT_AVPLAY_INFO *phAvplayInfo)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    MTADP_MPI_FUNCTION_ENTER();

    option.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    option.u32TimeoutMs = 0;

    MTADP_MPI_INFO_PRINT("stop live play ...\n");

    /*stop playing audio and video*/
    MT_UNF_AVPLAY_Stop(phAvplayInfo->hAvPlay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);

    /** Enable/disable windows */
    (MT_VOID)MT_UNF_VO_SetWindowEnable(phAvplayInfo->hWin, MT_FALSE);

    /** Unbind the window and AV player */
    (MT_VOID)MT_UNF_VO_DetachWindow(phAvplayInfo->hWin, phAvplayInfo->hAvPlay);

    /** Destroy window */
    (MT_VOID)MT_UNF_VO_DestroyWindow(phAvplayInfo->hWin);

    /** Contact the binding of track and AV player */
    (MT_VOID)MT_UNF_SND_Detach(phAvplayInfo->hSoundTrack, phAvplayInfo->hAvPlay);

    /** Destroy a Track */
    (MT_VOID)MT_UNF_SND_DestroyTrack(phAvplayInfo->hSoundTrack);

    /** Turn off the video channel */
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(phAvplayInfo->hAvPlay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

    /** Turn off the audio channel */
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(phAvplayInfo->hAvPlay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

    /** Destroy the AV player */
    (MT_VOID)MT_UNF_AVPLAY_Destroy(phAvplayInfo->hAvPlay);

    /** Deinitializes the AV player module */
    (MT_VOID)MT_UNF_AVPLAY_DeInit();

    MTADP_MPI_FUNCTION_EXIT();
}


mt_s32 MTADP_Switch_Standby_Mode(mt_void)
{
    MTADP_Standby_Mode_Deinit(&avplayHandle);

    system("killall -SIGUSR1 audio_ta_service");
    system("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
    system("echo 960000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed");

    return MT_SUCCESS;
}



int mt_optind = 1;
static int mt_optopt;
char *mt_optarg;

mt_s32 MTADP_Getopt(int argc, char *argv[], char *opts)
{
    static int sp = 1;
    int c;
    char *cp;

    MTADP_MPI_FUNCTION_ENTER();

    if (sp == 1) {
        if (mt_optind >= argc)
            return EOF;
        else if (!strcmp(argv[mt_optind], "--")) {
            mt_optind++;
            return EOF;
        }
        else if(argv[mt_optind][0] != '-' || argv[mt_optind][1] == '\0')
        {
            return '?';
        }
    }
    mt_optopt = c = argv[mt_optind][sp];

    if (c == ':' || (cp = strchr(opts, c)) == NULL) {
        //fprintf(stderr, ": illegal option -- %c\n", c);
        if (argv[mt_optind][++sp] == '\0') {
            mt_optind++;
        }
        sp = 1;
        return '?';
    }

    if (*++cp == ':') {
        if (argv[mt_optind][sp+1] != '\0')
            mt_optarg = &argv[mt_optind++][sp+1];
        else if(++mt_optind >= argc) {
            //fprintf(stderr, ": option requires an argument -- %c\n", c);
            sp = 1;
            return '?';
        } else
            mt_optarg = argv[mt_optind++];
        sp = 1;
    } else {
        if (argv[mt_optind][++sp] == '\0') {
            sp = 1;
            mt_optind++;
        }
        mt_optarg = NULL;
    }

    MTADP_MPI_FUNCTION_EXIT();

    return c;
}


mt_s32 MTADP_MCE_Exit(mt_void)
{
#if 0 //Rock_hu ×¢µô´úÂë
    mt_s32                  Ret;
    MT_UNF_MCE_STOPPARM_S   stStop;

    Ret = MT_UNF_MCE_Init(MT_NULL);
    if (MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_MCE_Init failed, Ret=%#x!\n", Ret);
        return Ret;
    }

    Ret = MT_UNF_MCE_ClearLogo();
    if (MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_MCE_ClearLogo failed, Ret=%#x!\n", Ret);
        return Ret;
    }

    stStop.enStopMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    stStop.enCtrlMode = MT_UNF_MCE_PLAYCTRL_BY_TIME;
    stStop.u32PlayTimeMs = 0;
    Ret = MT_UNF_MCE_Stop(&stStop);
    if (MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_MCE_Stop failed, Ret=%#x!\n", Ret);
        return Ret;
    }

    Ret = MT_UNF_MCE_Exit(MT_NULL);
    if (MT_SUCCESS != Ret)
    {
        MTADP_MPI_ERR_PRINT("call MT_UNF_MCE_Exit failed, Ret=%#x!\n", Ret);
        return Ret;
    }

    MT_UNF_MCE_DeInit();
#endif
    return MT_SUCCESS;
}

size_t MTADP_Strncpy(char *dest, const char *src, size_t size)
{
    size_t len = strlen(src);

    if (len >= size)
    {
        len = size - 1;
    }

    memcpy(dest, src, len);
    dest[len]= '\0';

    return len;
}

mt_s32 MTADP_ReadPlayStat(mt_s64 *first_vid_frm_show_time, mt_s64 *avsync_done_time)
{
    FILE *fp = NULL;
    char buf[128];
    mt_s64 val_f[2];
    int i = 0;
    int n = 0;
    char* stat_str[] = {
        "TOTAL_AV       = %lld",
        "SYNC_DONE       = %lld",
    };

    fp = fopen("/proc/msp/stat", "r");
    if (NULL == fp) {
        MTADP_MPI_ERR_PRINT("open failed %s", strerror(errno));
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    while (fgets(buf, 128, fp)) {
        for (i = 0; i < 2; i++) {
            n = sscanf(buf, stat_str[i], &val_f[i]);
            if (n == 1) {
                break;
            }
        }
        memset(buf, 0, sizeof(buf));
    }

    fclose(fp);
    fp = NULL;

    *first_vid_frm_show_time = val_f[0];
    *avsync_done_time = val_f[1];

    return MT_SUCCESS;
}

