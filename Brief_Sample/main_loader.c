/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include "mt_unf_common.h"
#include "common/mt_cmdline.h"
#include "mt_unf_gpio.h"
#include "mt_unf_disp.h"
#include "mt_unf_demux.h"
#include "mt_common.h"
#include "mt_type.h"
#include "mt_unf_misc.h"
#include "mt_cmdline.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_mpi.h"
#include "mt_adp_config.h"
#include "HA.AUDIO.AAC.decode.h"



#define MT_PRINT   printf
#define MT_SAMPLE_VERSION   "v0001"


#define SAMPLE_INFO_PRINT(fmt...)           MT_PRINT("[INFO]"  fmt)

//MT_S32 MT_DownloadMain(MT_S32 argc, MT_CHAR *argv[]);
#ifdef MT_SYM4_DSS
MT_S32 MT_DssPlayMain(MT_S32 argc, MT_CHAR *argv[]);
#endif
MT_S32 MT_VersionMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_DvbplayMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_AfdMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_DownmixMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_KadcMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_ADMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_FlashAvlMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_Str_standbyMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_TemperatureMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_IpstreamMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_PlayreadyMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_SatipMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_SuPlayMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_FrontpanelMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_StandbyFrontpanelMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_SubtMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_J83bMain(MT_S32 argc, MT_CHAR *argv[]);
mt_s32 MT_MtgoScrolltextMain(mt_s32 argc, mt_char *argv[]);
MT_S32 MT_CGMSMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_ShowLogoMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_PowerFrontpanelMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_NetMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_WiFiMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_UdPlayMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_TPlayMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_StandbyTimerMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_SmcMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_PvrPlayMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_OtpMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_FlashMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_R2RMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_RatioMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_DolbyMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_CCMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_SdtMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_SectionMain(MT_S32 argc, MT_CHAR *argv[]);
mt_s32 MT_MtgoDrawMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_MtgoOptlayerMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_MtgoOpacityMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_MtgoBitDecpicMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_MtgoLoopPicMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_RotateMirrorMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_MtgoDecPicMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_DvbsMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_I2cMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_IrMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_Standby_IrMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_PmtMain(mt_s32 argc, mt_char *argv[]);

mt_s32 MT_NitMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_CatMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_TsPlayMain(mt_s32 argc, mt_char *argv[]);

mt_s32 MT_WdgMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_DispZoomMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_BlindscanMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_HashMain(mt_s32 argc, mt_char *argv[]);

mt_s32 MT_HmacMain(mt_s32 argc, mt_char *argv[]);
MT_S32 MT_RSAMain(MT_S32 argc, MT_CHAR *argv[]);

mt_s32 MT_DlnaMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_DisEqcMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_HdmiCecMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_HdcpMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_HdmiHdrMain(mt_s32 argc, mt_char *argv[]);
#ifdef MT_HDMI_HDR_VIVID
MT_S32 MT_HdrVividMain(MT_S32 argc, MT_CHAR *argv[]);
#endif
mt_s32 MT_ConfigMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_PowerTimerMain(mt_s32 argc, mt_char *argv[]);
//mt_s32 MT_PVRRecMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_PVRTimeshiftMain(mt_s32 argc, mt_char *argv[]);

mt_s32 MT_SplitSearchMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_SwitchTrackMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_TeletextMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_UsbdeviceMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_NetDumpMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_VolumeMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_PatMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_EitMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_Power_IrMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_MtgoTextMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_DvbtMain(MT_S32 argc, MT_CHAR *argv[]);
mt_s32 MT_DvbcMain(MT_S32 argc, MT_CHAR *argv[]);
mt_s32 MT_DispFmtMain(int argc, char *argv[]);
mt_s32 MT_AudioModeMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_GpioMain(MT_S32 argc, MT_CHAR *argv[]);

MT_S32 MT_Dvbs_t2miMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_DisEqc2Main(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_SuperBlindscanMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_MotorMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_MacrovisionMain(MT_S32 argc, MT_CHAR *argv[]);
#ifdef MT_GSTPLAYER_ENABLE
MT_S32 MT_GstpMain(MT_S32 argc, MT_CHAR *argv[]);
#endif
mt_s32 MT_MtgoLayerpmMain(mt_s32 argc, mt_char *argv[]);
mt_s32 MT_UnicableMain(mt_s32 argc, mt_char *argv[]);
MT_S32 MT_SoundTrackMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_DvbRangeMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_VideoUnblankMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_SplitTunerMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_GseiperfMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_System_InfoMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_HeaacMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_NimPlayMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_PipPlayMain(MT_S32 argc, MT_CHAR *argv[]);
#ifdef MT_MIRACAST_SUPPORT
MT_S32 MT_MiracastMain(MT_S32 argc, MT_CHAR *argv[]);
#endif
MT_S32 MT_UsbCamMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_StrTimer_standbyMain(MT_S32 argc, MT_CHAR *argv[]);
MT_S32 MT_AC4Main(MT_S32 argc, MT_CHAR *argv[]);

extern MT_S32 MTSampleCommandInit(void);
extern void *MTSampleCommadTask(void *pParam);
extern MT_S32 MTSampleCommandRegister(const char *name, mt_s32 ( * func)(mt_s32 argc,  char *argv[]), const char * help, u32 useres);


int main(mt_s32 argc, char *argv[])
{

    mt_s32 s32Ret = MT_SUCCESS;
    config_t config = {0};
    MT_UNF_WINDOW_ATTR_S getWindowAttr = { 0 };
    mt_handle hwin = 0;
    MT_UNF_DISP_ASPECT_RATIO_S setDispAspectRatio = { 0 };
//---
//    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
//    mt_handle                hAvplay = 0;
//    MT_UNF_ACODEC_ATTR_S             AdecAttr = { 0 };

    s32Ret = mt_sys_init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_INFO_PRINT("mt_sys_init error. ret=0x%x \n", s32Ret);
        return s32Ret;
    }

    s32Ret = MTADP_Read_All_Config(&config);
    if((MT_SUCCESS != s32Ret) || (MT_TRUE != config.is_use))
    {
        SAMPLE_INFO_PRINT("MTADP_Flash_Read[ret = %x] or sample_db flash config[is_use = %d]\n", s32Ret, config.is_use);
        MTADP_Config_Set_Default(&config);
    }

    (mt_void)MTADP_HDMI_Set_HdcpEnable(config.hdcp);

    s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, config.disp_fmt.format);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_INFO_PRINT("MTADP_HDMI_Init failed, ret = %x\n", s32Ret);
        goto ERR0;
    }

//---
#if 0
    if(config.heaac == MT_TRUE)
    {
        (MT_VOID)MT_UNF_DMX_Init();

        (MT_VOID)MT_UNF_AVPLAY_Init();

        (MT_VOID)MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);

        (MT_VOID)MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);

        (MT_VOID)MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);


        (MT_VOID)MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);

        AdecAttr.enType = HA_AUDIO_ID_AAC;

        HA_AAC_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));

        (MT_VOID)MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);

        (MT_VOID)MT_UNF_AVPLAY_Enable_AudioHEAAC(hAvplay);
        SAMPLE_INFO_PRINT("MT_UNF_AVPLAY_Enable_AudioHEAAC \n");

        (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

        (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);

        (MT_VOID)MT_UNF_AVPLAY_DeInit();

        (MT_VOID)MT_UNF_DMX_DeInit();

        /* reset heeac global variable to MT_FALSE after stb reboot, so need to reset it to MT_TRUE.*/
        MTADP_Set_Heaac_Enable(MT_TRUE);
    }
#endif

    /** Display initialization */
    s32Ret = MTADP_Disp_Init(config.disp_fmt.format);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_INFO_PRINT("MTADP_Disp_Init failed, ret = %x\n", s32Ret);
        goto ERR1;
    }

    /* Temporary solution, circumvent the CVBS boot green screen situation and MT_UNF_VO_GetWindowAttr bug*/
    MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    s32Ret = MTADP_VO_CreatWin(MT_NULL, &hwin);
    if(MT_SUCCESS != s32Ret)
    {
        (MT_VOID)MTADP_VO_DeInit();
        SAMPLE_INFO_PRINT("MTADP_VO_CreatWin failed, ret = %x\n", s32Ret);
        goto ERR2;
    }
    (MT_VOID)MT_UNF_VO_SetWindowEnable(hwin, MT_TRUE);

    if(MT_TRUE == config.ratio.enable)
    {

        setDispAspectRatio.enDispAspectRatio = config.ratio.enDispAspectRatio;
        MT_UNF_DISP_SetAspectRatio(MT_UNF_DISPLAY1, &setDispAspectRatio);
        MT_UNF_VO_GetWindowAttr(hwin, &getWindowAttr);
        getWindowAttr.stWinAspectAttr.enAspectCvrs = config.ratio.enAspectCvrs;
        MT_UNF_VO_SetWindowAttr(hwin, &getWindowAttr);
    }

    (MT_VOID)MT_UNF_VO_SetWindowEnable(hwin, MT_FALSE);
    (MT_VOID)MT_UNF_VO_DestroyWindow(hwin);
    (MT_VOID)MTADP_VO_DeInit();

    SAMPLE_INFO_PRINT("[%s]: [%d]\n", __FUNCTION__, __LINE__);

    MTSampleCommandInit();

//---
#if 0
    MTSampleCommandRegister("j83b", MT_J83bMain, "j83b play sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("dvbc", MT_DvbcMain, "DVBC play sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("dvbt", MT_DvbtMain, "DVBT play sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("dvbs", MT_DvbsMain, "DVBS play sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("nim_play", MT_NimPlayMain, "NiM play sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("pid_play", MT_DvbplayMain, "DVB play by pid sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);

    MTSampleCommandRegister("front", MT_FrontpanelMain, "frontpanel sample command.", MT_RESOURCE_FRONTPANEL);
    MTSampleCommandRegister("i2c", MT_I2cMain, "i2c sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("ir", MT_IrMain, "ir sample command.", MT_RESOURCE_IR);
    MTSampleCommandRegister("kadc", MT_KadcMain, "kadc sample command.", MT_RESOURCE_FRONTPANEL);

    MTSampleCommandRegister("power_front", MT_PowerFrontpanelMain, "power frontpanel sample command.", MT_RESOURCE_FRONTPANEL);
    MTSampleCommandRegister("power_timer", MT_PowerTimerMain, "power timer sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("power_ir", MT_Power_IrMain, "power_ir sample command.", MT_RESOURCE_IR);
    MTSampleCommandRegister("str", MT_Str_standbyMain, " str standby sample command.", MT_RESOURCE_IR);
    MTSampleCommandRegister("standby_front", MT_StandbyFrontpanelMain, "standby frontpanel sample command.", MT_RESOURCE_FRONTPANEL);
    MTSampleCommandRegister("standby_timer", MT_StandbyTimerMain, "standby timer sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("standby_ir", MT_Standby_IrMain, "standby_ir sample command.", MT_RESOURCE_IR);
//    MTSampleCommandRegister("download", MT_DownloadMain, "download sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("wdg", MT_WdgMain, "wdg sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("otp", MT_OtpMain, "otp sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("flash", MT_FlashMain, "flash sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("flashavl", MT_FlashAvlMain, "flash avl sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("ades", MT_ADMain, "audio description sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("gpio", MT_GpioMain, "gpio sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("smc", MT_SmcMain, "smc sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("cgms", MT_CGMSMain, "cgms sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);

    MTSampleCommandRegister("gseiperf", MT_GseiperfMain, "gseiperf sample command.", MT_RESOURCE_DMX);

#ifdef MT_SYM4_DSS
    MTSampleCommandRegister("dssplay", MT_DssPlayMain, "dssplay sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
#endif
    MTSampleCommandRegister("tsplay", MT_TsPlayMain, "tsplay sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("audio", MT_AudioModeMain, "audio  sample command.", MT_RESOURCE_SND);
    MTSampleCommandRegister("suplay", MT_SuPlayMain, "suplay play sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("showlogo", MT_ShowLogoMain, "showlogo sample command.", MT_RESOURCE_DISP);
    MTSampleCommandRegister("playready", MT_PlayreadyMain, "playready sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND);
    MTSampleCommandRegister("ipstream", MT_IpstreamMain, "ipstream sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND);
#ifdef MT_GSTPLAYER_ENABLE
    MTSampleCommandRegister("gstplay", MT_GstpMain, "gstplay sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND);
#endif


    MTSampleCommandRegister("tplay", MT_TPlayMain, "tplay sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("volume", MT_VolumeMain, "volume sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("disp_fmt", MT_DispFmtMain, " display formart sample command.", MT_RESOURCE_NO );
    MTSampleCommandRegister("ratio", MT_RatioMain, "ratio sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("afd", MT_AfdMain, "afd sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("disp_zoom", MT_DispZoomMain, "disp_zoom sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("downmix", MT_DownmixMain, "downmix sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("sw_track", MT_SwitchTrackMain, "switch_track sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("sound_track", MT_SoundTrackMain, "souind_track sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("video_unblank", MT_VideoUnblankMain, "video_unblank sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);

    MTSampleCommandRegister("hdmi_cec", MT_HdmiCecMain, "CEC sample command.", MT_RESOURCE_IR);
    MTSampleCommandRegister("hdcp", MT_HdcpMain, "hdcp sample command.",MT_RESOURCE_NO);
    MTSampleCommandRegister("hdmi_hdr", MT_HdmiHdrMain, "HDR sample command.", MT_RESOURCE_NO);
#ifdef MT_HDMI_HDR_VIVID
	MTSampleCommandRegister("hdr_vivid", MT_HdrVividMain, "HDR Vivid sample command.", MT_RESOURCE_NO);
#endif
    MTSampleCommandRegister("config", MT_ConfigMain, "config sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("dolby", MT_DolbyMain, "dolby sample command.", MT_RESOURCE_NO);


    MTSampleCommandRegister("net", MT_NetMain, "net sample command.", MT_RESOURCE_ETH);
    MTSampleCommandRegister("wifi", MT_WiFiMain, "wifi sample command.", MT_RESOURCE_WIFI);
    MTSampleCommandRegister("netdump", MT_NetDumpMain, "Net_dump sample command.", MT_RESOURCE_ETH);
    MTSampleCommandRegister("udplay", MT_UdPlayMain, "udplay sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("dlna", MT_DlnaMain, "Dlna sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("satip", MT_SatipMain, "satip sample command.", MT_RESOURCE_DMX);

    MTSampleCommandRegister("cc", MT_CCMain, "cc sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("subtitle", MT_SubtMain, "subtitle play sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("teletext", MT_TeletextMain, "Teletext sample command.", MT_RESOURCE_NO);

    MTSampleCommandRegister("mtgo_text", MT_MtgoTextMain, "MTGO text sample command.", MT_RESOURCE_OSD);
    MTSampleCommandRegister("mtgo_scrtext", MT_MtgoScrolltextMain, "mtgo scroll sample command.", MT_RESOURCE_OSD);
    MTSampleCommandRegister("mtgo_draw", MT_MtgoDrawMain, "mtgo draw sample command.", MT_RESOURCE_OSD);
    MTSampleCommandRegister("mtgo_opt", MT_MtgoOptlayerMain, "mtgo opt sample command.", MT_RESOURCE_OSD);
    MTSampleCommandRegister("mtgo_pic", MT_MtgoDecPicMain, "MTGO decpic sample command.", MT_RESOURCE_OSD);
    MTSampleCommandRegister("mtgo_romi", MT_RotateMirrorMain, "MTGO romote_mirror sample command.", MT_RESOURCE_OSD);
    MTSampleCommandRegister("mtgo_bit", MT_MtgoBitDecpicMain, "MTGO bit sample command.", MT_RESOURCE_OSD);
    MTSampleCommandRegister("mtgo_opa", MT_MtgoOpacityMain, "MTGO opacity sample command.", MT_RESOURCE_OSD);
    MTSampleCommandRegister("layerpm", MT_MtgoLayerpmMain, "l play sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX|MT_RESOURCE_OSD);
    MTSampleCommandRegister("mtgo_loop_pic", MT_MtgoLoopPicMain, "MTGO loop pic sample command.", MT_RESOURCE_OSD);

    MTSampleCommandRegister("dmx_pat", MT_PatMain, "DMX pat sample command.", MT_RESOURCE_DMX);
    MTSampleCommandRegister("dmx_eit", MT_EitMain, "DMX eit sample command.", MT_RESOURCE_DMX);
    MTSampleCommandRegister("dmx_pmt", MT_PmtMain, "DMX pmt sample command.", MT_RESOURCE_DMX);
    MTSampleCommandRegister("dmx_nit", MT_NitMain, "DMX nit sample command.", MT_RESOURCE_DMX);
    MTSampleCommandRegister("dmx_cat", MT_CatMain, "DMX cat sample command.", MT_RESOURCE_DMX);
    MTSampleCommandRegister("dmx_sec", MT_SectionMain, "DMX section sample command.", MT_RESOURCE_DMX);
    MTSampleCommandRegister("dmx_sdt", MT_SdtMain, "sdt sample command.", MT_RESOURCE_DMX);

    MTSampleCommandRegister("blindscan", MT_BlindscanMain, "Blindscan sample command.", MT_RESOURCE_DMX);
    MTSampleCommandRegister("dvbs_diseqc", MT_DisEqcMain, "diseqc sample command.", MT_RESOURCE_DMX);
    MTSampleCommandRegister("sp_search", MT_SplitSearchMain, "split_search sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("sp_tuner", MT_SplitTunerMain, "sp_tuner sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("unicable", MT_UnicableMain, "unicable sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("dvb_range", MT_DvbRangeMain, "dvb_range sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);

    MTSampleCommandRegister("pvr", MT_PVRTimeshiftMain, "timeshift sample command.", MT_RESOURCE_NO);
    //MTSampleCommandRegister("pvr_rec", MT_PVRRecMain, "PVR_REC sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("pvr_play", MT_PvrPlayMain, "pvr play sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);



    MTSampleCommandRegister("r2r", MT_R2RMain, "r2r sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("rsa", MT_RSAMain, "rsa sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("hash", MT_HashMain, "hash sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("hmac", MT_HmacMain, "hmac sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("version", MT_VersionMain, "version sample command.", MT_RESOURCE_NO);

    MTSampleCommandRegister("usbdevice", MT_UsbdeviceMain, "usb_device sample command.", MT_RESOURCE_NO);

    MTSampleCommandRegister("dvbs_t2mi", MT_Dvbs_t2miMain, "dvbs_t2mi sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("diseqc2", MT_DisEqc2Main, "diseqc2 sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("superblindscan", MT_SuperBlindscanMain, "superblindscan sample command.", MT_RESOURCE_DMX);
    MTSampleCommandRegister("motor", MT_MotorMain, "motor sample command.", MT_RESOURCE_DMX);
    MTSampleCommandRegister("macrovision", MT_MacrovisionMain, "macrovision sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("temperature", MT_TemperatureMain, "temperature sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("system_info", MT_System_InfoMain, "system_info sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("heaac", MT_HeaacMain, "heaac sample command.", MT_RESOURCE_NO);
    MTSampleCommandRegister("piplay", MT_PipPlayMain, "pip play sample command.", MT_RESOURCE_NO);

#ifdef MT_MIRACAST_SUPPORT
    MTSampleCommandRegister("miracast", MT_MiracastMain, "mira cast sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
#endif
    MTSampleCommandRegister("usbcam", MT_UsbCamMain, "usbcam sample command.", MT_RESOURCE_DISP|MT_RESOURCE_SND|MT_RESOURCE_DMX);
    MTSampleCommandRegister("str_timer", MT_StrTimer_standbyMain, "str_timer sample command.", MT_RESOURCE_NO);

	MTSampleCommandRegister("ac4", MT_AC4Main, "AC4 sample command.", MT_RESOURCE_NO);
#endif

//+++
//CONFIG_MT_PRODUCT_LOADER
#if 1
	MTSampleCommandRegister("diseqc2", MT_DisEqc2Main, "diseqc2 sample command.", MT_RESOURCE_NO);

	MTSampleCommandRegister("dmx_cat", MT_CatMain, "DMX cat sample command.", MT_RESOURCE_DMX);
	MTSampleCommandRegister("dmx_eit", MT_EitMain, "DMX eit sample command.", MT_RESOURCE_DMX);
	MTSampleCommandRegister("dmx_nit", MT_NitMain, "DMX nit sample command.", MT_RESOURCE_DMX);
	MTSampleCommandRegister("dmx_pat", MT_PatMain, "DMX pat sample command.", MT_RESOURCE_DMX);
	MTSampleCommandRegister("dmx_pmt", MT_PmtMain, "DMX pmt sample command.", MT_RESOURCE_DMX);
	MTSampleCommandRegister("dmx_sdt", MT_SdtMain, "sdt sample command.", MT_RESOURCE_DMX);
	MTSampleCommandRegister("dmx_sec", MT_SectionMain, "DMX section sample command.", MT_RESOURCE_DMX);

	MTSampleCommandRegister("flash", MT_FlashMain, "flash sample command.", MT_RESOURCE_NO);
	MTSampleCommandRegister("front", MT_FrontpanelMain, "frontpanel sample command.", MT_RESOURCE_FRONTPANEL);

	MTSampleCommandRegister("gpio", MT_GpioMain, "gpio sample command.", MT_RESOURCE_NO);

	MTSampleCommandRegister("hash", MT_HashMain, "hash sample command.", MT_RESOURCE_NO);
	MTSampleCommandRegister("hmac", MT_HmacMain, "hmac sample command.", MT_RESOURCE_NO);

	MTSampleCommandRegister("i2c", MT_I2cMain, "i2c sample command.", MT_RESOURCE_NO);
	MTSampleCommandRegister("ir", MT_IrMain, "ir sample command.", MT_RESOURCE_IR);

	MTSampleCommandRegister("kadc", MT_KadcMain, "kadc sample command.", MT_RESOURCE_FRONTPANEL);

    MTSampleCommandRegister("motor", MT_MotorMain, "motor sample command.", MT_RESOURCE_DMX);

	MTSampleCommandRegister("mtgo_bit", MT_MtgoBitDecpicMain, "MTGO bit sample command.", MT_RESOURCE_OSD);
	MTSampleCommandRegister("mtgo_draw", MT_MtgoDrawMain, "mtgo draw sample command.", MT_RESOURCE_OSD);
	MTSampleCommandRegister("mtgo_loop_pic", MT_MtgoLoopPicMain, "MTGO loop pic sample command.", MT_RESOURCE_OSD);
	MTSampleCommandRegister("mtgo_opa", MT_MtgoOpacityMain, "MTGO opacity sample command.", MT_RESOURCE_OSD);
	MTSampleCommandRegister("mtgo_opt", MT_MtgoOptlayerMain, "mtgo opt sample command.", MT_RESOURCE_OSD);
	MTSampleCommandRegister("mtgo_pic", MT_MtgoDecPicMain, "MTGO decpic sample command.", MT_RESOURCE_OSD);
	MTSampleCommandRegister("mtgo_romi", MT_RotateMirrorMain, "MTGO romote_mirror sample command.", MT_RESOURCE_OSD);
	MTSampleCommandRegister("mtgo_scrtext", MT_MtgoScrolltextMain, "mtgo scroll sample command.", MT_RESOURCE_OSD);
	MTSampleCommandRegister("mtgo_text", MT_MtgoTextMain, "MTGO text sample command.", MT_RESOURCE_OSD);

    MTSampleCommandRegister("net", MT_NetMain, "net sample command.", MT_RESOURCE_ETH);
    MTSampleCommandRegister("netdump", MT_NetDumpMain, "Net_dump sample command.", MT_RESOURCE_ETH);

	MTSampleCommandRegister("otp", MT_OtpMain, "otp sample command.", MT_RESOURCE_NO);

	MTSampleCommandRegister("r2r", MT_R2RMain, "r2r sample command.", MT_RESOURCE_NO);
	MTSampleCommandRegister("rsa", MT_RSAMain, "rsa sample command.", MT_RESOURCE_NO);

	MTSampleCommandRegister("smc", MT_SmcMain, "smc sample command.", MT_RESOURCE_NO);
	MTSampleCommandRegister("system_info", MT_System_InfoMain, "system_info sample command.", MT_RESOURCE_NO);

    MTSampleCommandRegister("temperature", MT_TemperatureMain, "temperature sample command.", MT_RESOURCE_NO);

	MTSampleCommandRegister("version", MT_VersionMain, "version sample command.", MT_RESOURCE_NO);

	MTSampleCommandRegister("wdg", MT_WdgMain, "wdg sample command.", MT_RESOURCE_NO);
#endif

    MTSampleCommadTask(NULL);

ERR2:
    (MT_VOID)MTADP_Disp_DeInit();

ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

ERR0:
    (MT_VOID)mt_sys_deinit();

    return 0;
}

