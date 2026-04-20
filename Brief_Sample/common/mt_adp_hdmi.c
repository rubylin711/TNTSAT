#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "mt_unf_hdmi.h"
#include "mt_unf_edid.h"
#include "mt_unf_disp.h"
#include "mt_adp_debug.h"
#include "mt_adp_hdmi.h"
#include "mt_unf_pm.h"
#include "mt_mpi_mem.h"
#include "mt_adp_str.h"
#include "mt_unf_flash.h"


//#include "mt_adp_audio.h"


#ifdef MTADP_HDMI_DEBUG

#define MTADP_HDMI_PRINT   MTADP_PRINT

#else

#define MTADP_HDMI_PRINT

#endif


#define MTADP_HDMI_FUNCTION_ENTER()  MTADP_HDMI_PRINT("[MTADP_HDMI][%s]: Enter ==>> \n", __FUNCTION__)
#define MTADP_HDMI_FUNCTION_EXIT()   MTADP_HDMI_PRINT("[MTADP_HDMI][%s]: Exit ==<< \n", __FUNCTION__)



#define MTADP_HDMI_FATAL_PRINT(fmt...)       MTADP_HDMI_PRINT(" [MTADP_HDMI][FATAL] " fmt)
#define MTADP_HDMI_ERR_PRINT(fmt...)         MTADP_HDMI_PRINT(" [MTADP_HDMI][ERROR] " fmt)
#define MTADP_HDMI_WARN_PRINT(fmt...)        MTADP_HDMI_PRINT(" [MTADP_HDMI][WARN] "  fmt)
#define MTADP_HDMI_INFO_PRINT(fmt...)        MTADP_HDMI_PRINT(" [MTADP_HDMI][INFO] "  fmt)
#define MTADP_HDMI_MT_DBG_PRINT(fmt...)      MTADP_HDMI_PRINT(" [MTADP_HDMI][DEBUG] " fmt)



typedef struct mtHDMI_ARGS_S
{
    MT_UNF_HDMI_ID_E enHdmi;
} HDMI_ARGS_S;

static HDMI_ARGS_S g_stHdmiArgs;
mt_u32 g_HDCPFlag = MT_FALSE;
mt_u32 g_HDMI_Bebug = MT_FALSE;
static mt_u32 g_hdmi_standby_mode = MT_HDMI_CEC_STANDBY_DEFAULT;
mt_u32 g_HDMIUserCallbackFlag = MT_FALSE;
mt_u32 g_enDefaultMode = MT_UNF_HDMI_DEFAULT_ACTION_HDMI; //MT_UNF_HDMI_DEFAULT_ACTION_NULL;
static MT_UNF_HDMI_CALLBACK_FUNC_S g_stCallbackFunc;
cec_config_t g_cec_cfg = {0};
MT_BOOL g_str_enable = MT_FALSE;
static mt_s32 g_HDMI_Standby = MT_FALSE;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

User_HDMI_CallBack pfnHdmiUserCallback = NULL;
//#define MT_HDCP_SUPPORT
#ifdef MT_HDCP_SUPPORT
const mt_char *pstencryptedHdcpKey = "EncryptedKey_332bytes.bin";
#endif

static mt_char *g_pDispFmtString[MT_UNF_ENC_FMT_BUTT + 1] = {
    "1080P_60",
    "1080P_50",
    "1080P_30",
    "1080P_25",
    "1080P_24",
    "1080i_60",
    "1080i_50",
    "720P_60",
    "720P_50",
    "576P_50",
    "480P_60",
    "PAL",
    "PAL_N",
    "PAL_Nc",
    "NTSC",
    "NTSC_J",
    "NTSC_PAL_M",
    "SECAM_SIN",
    "SECAM_COS",
    /*
    "1080p_24FP",
    "720P_60FP",
    "720P_50FP",
    "640_480",
*/
    "BUTT"
};

MT_UNF_ENC_FMT_E stringToUnfFmt(mt_char *pszFmt)
{
    mt_s32 i;
    MT_UNF_ENC_FMT_E fmtReturn = MT_UNF_ENC_FMT_BUTT;

    if (NULL == pszFmt) {
    return MT_UNF_ENC_FMT_BUTT;
    }

    for (i = 0; i < MT_UNF_ENC_FMT_BUTT; i++) {
    if (strcasestr(pszFmt, g_pDispFmtString[i])) {
        fmtReturn = i;
        break;
    }
    }

    if (i >= MT_UNF_ENC_FMT_BUTT) {
    i = MT_UNF_ENC_FMT_720P_50;
    fmtReturn = i;
    MTADP_HDMI_INFO_PRINT("\n!!! Can NOT match format, set format to is '%s'/%d.\n\n", g_pDispFmtString[i], i);
    } else {
    MTADP_HDMI_INFO_PRINT("\n!!! The format is '%s'/%d.\n\n", g_pDispFmtString[i], i);
    }
    return fmtReturn;
}

static mt_void HDMI_PrintAttr(MT_UNF_HDMI_ATTR_S *pstHDMIAttr)
{
    if (MT_TRUE != g_HDMI_Bebug) {
    return;
    }

    MTADP_HDMI_INFO_PRINT("=====MT_UNF_HDMI_SetAttr=====\n"
                         "bEnableHdmi:%d\n"
                         "bEnableVideo:%d\n"
                         "enVidOutMode:%d\n"
                         "enDeepColorMode:%d\n"
                         "bxvYCCMode:%d\n\n"
                         "bEnableAudio:%d\n"
                         "bEnableAviInfoFrame:%d\n"
                         "bEnableAudInfoFrame:%d\n"
                         "bEnableSpdInfoFrame:%d\n"
                         "bEnableMpegInfoFrame:%d\n\n"
                         "==============================\n",
                         pstHDMIAttr->bEnableHdmi,
                         pstHDMIAttr->bEnableVideo,
                         pstHDMIAttr->enVidOutMode, pstHDMIAttr->enDeepColorMode, pstHDMIAttr->bxvYCCMode,
                         pstHDMIAttr->bEnableAudio,
                         pstHDMIAttr->bEnableAudInfoFrame, pstHDMIAttr->bEnableAudInfoFrame,
                         pstHDMIAttr->bEnableSpdInfoFrame, pstHDMIAttr->bEnableMpegInfoFrame);
    return;
}

static mt_void HDMI_HotPlug_Proc(mt_void *pPrivateData)
{
    mt_s32 ret = MT_SUCCESS;
    HDMI_ARGS_S *pArgs = (HDMI_ARGS_S *)pPrivateData;
    MT_UNF_HDMI_ID_E hHdmi = pArgs->enHdmi;
    MT_UNF_HDMI_ATTR_S stHdmiAttr;
    //MT_UNF_HDMI_INFOFRAME_S        stInfoFrame;
    MT_UNF_EDID_BASE_INFO_S stSinkCap;
    MT_UNF_HDMI_STATUS_S stHdmiStatus;

    static mt_u8 u8FirstTimeSetting = MT_TRUE;

    MTADP_HDMI_INFO_PRINT("\n --- Get HDMI event: HOTPLUG. --- \n");

    MT_UNF_HDMI_GetStatus(hHdmi, &stHdmiStatus);
    if (MT_FALSE == stHdmiStatus.bConnected) {
    MTADP_HDMI_ERR_PRINT("No Connect\n");
    return;
    }

    MT_UNF_HDMI_GetAttr(hHdmi, &stHdmiAttr);
    ret = MT_UNF_HDMI_GetSinkCapability(hHdmi, &stSinkCap);

    if (ret == MT_SUCCESS) {
        //stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_YCBCR444;
        if (MT_TRUE == stSinkCap.bSupportHdmi) {
            stHdmiAttr.bEnableHdmi = MT_TRUE;
        } else {
            stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
            //读取到了edid，并且不支持hdmi则进入dvi模式
            //read real edid ok && sink not support hdmi,then we run in dvi mode
            stHdmiAttr.bEnableHdmi = MT_FALSE;
        }
    } else {
    //when get capability fail,use default mode
    if (g_enDefaultMode != MT_UNF_HDMI_DEFAULT_ACTION_DVI)
        stHdmiAttr.bEnableHdmi = MT_TRUE;
    else
        stHdmiAttr.bEnableHdmi = MT_FALSE;
      }

    if (MT_TRUE == stHdmiAttr.bEnableHdmi) {
        stHdmiAttr.bEnableAudio = MT_TRUE;
        stHdmiAttr.bEnableVideo = MT_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = MT_TRUE;
        stHdmiAttr.bEnableAviInfoFrame = MT_TRUE;
        stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_BUTT;// MT_UNF_HDMI_VIDEO_MODE_BUTT will auto output
        stHdmiAttr.enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_BUTT;// MT_UNF_HDMI_DEEP_COLOR_BUTT will auto output
    } else {
        stHdmiAttr.bEnableAudio = MT_FALSE;
        stHdmiAttr.bEnableVideo = MT_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = MT_FALSE;
        stHdmiAttr.bEnableAviInfoFrame = MT_FALSE;
        stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
        stHdmiAttr.enDeepColorMode = MT_UNF_HDMI_DEEP_COLOR_24BIT;// MT_UNF_HDMI_DEEP_COLOR_BUTT will auto output
    }

    if (u8FirstTimeSetting == MT_TRUE) {
    u8FirstTimeSetting = MT_FALSE;
    if (g_HDCPFlag == MT_TRUE) {
        stHdmiAttr.bHDCPEnable = MT_TRUE; //Enable HDCP
    } else {
        stHdmiAttr.bHDCPEnable = MT_FALSE;
    }
    } else {
    //HDCP Enable use default setting!!
    }

    ret = MT_UNF_HDMI_SetAttr(hHdmi, &stHdmiAttr);

    /* MT_UNF_HDMI_SetAttr must before MT_UNF_HDMI_Start! */
    ret = MT_UNF_HDMI_Start(hHdmi);

    HDMI_PrintAttr(&stHdmiAttr);

    return;
}

static mt_void HDMI_UnPlug_Proc(mt_void *pPrivateData)
{
    HDMI_ARGS_S *pArgs = (HDMI_ARGS_S *)pPrivateData;
    MT_UNF_HDMI_ID_E hHdmi = pArgs->enHdmi;

    MTADP_HDMI_INFO_PRINT("\n --- Get HDMI event: UnPlug. --- \n");
    MT_UNF_HDMI_Stop(hHdmi);

    return;
}
static mt_u32 HDCPFailCount = 0;
static mt_void HDMI_HdcpFail_Proc(mt_void *pPrivateData)
{
    //MT_UNF_HDMI_ATTR_S             stHdmiAttr;
    MTADP_HDMI_INFO_PRINT("\n --- Get HDMI event: HDCP_FAIL. --- \n");

    HDCPFailCount++;
    if (HDCPFailCount >= 50) {
    HDCPFailCount = 0;
    MTADP_HDMI_INFO_PRINT("\nWarrning:Customer need to deal with HDCP Fail!!!!!!\n");
    }
#if 0
    MT_UNF_HDMI_GetAttr(0, &stHdmiAttr);

    stHdmiAttr.bHDCPEnable = MT_FALSE;

    MT_UNF_HDMI_SetAttr(0, &stHdmiAttr);
#endif
    return;
}

static mt_void HDMI_HdcpSuccess_Proc(mt_void *pPrivateData)
{
    MTADP_HDMI_INFO_PRINT("\n --- Get HDMI event: HDCP_SUCCESS. --- \n");
    return;
}

static mt_s32 MT_HDMI_Standby_Str(void)
{
    mt_s32 ret = MT_SUCCESS;

    MTADP_HDMI_FUNCTION_ENTER();
    MTADP_STR_StartSigStrStandby();
    MTADP_HDMI_FUNCTION_EXIT();

    return ret;
}


static mt_s32 MT_HDMI_Standby_Pmu(void)
{
    mt_s32 ret = MT_SUCCESS;
    sty_wakeup_conf_t wconfig = {0};

    ret = MT_UNF_PMOC_Init();
    if(ret < 0)
    {
        MTADP_HDMI_ERR_PRINT("MT_UNF_PMOC_Init failed\n");
    }

    ret = MT_UNF_PMOC_SetCecConfig(g_cec_cfg);
    if(ret < 0)
    {
        MTADP_HDMI_ERR_PRINT("MT_UNF_PMOC_SetCecConfig failed\n");
        MT_UNF_PMOC_DeInit();
        return ret;
    }

    ret = MT_UNF_PMOC_SetDevType(3);
    if(ret < 0)
    {
        MTADP_HDMI_ERR_PRINT("MT_UNF_PMOC_SetDevType failed\n");
        MT_UNF_PMOC_DeInit();
        return ret;
    }

    /** Pull the GPEN pin voltage low */
    ret = MT_UNF_PMOC_SetGpenPin(GPEN_LOW);
    if(MT_SUCCESS != ret)
    {
        MTADP_HDMI_ERR_PRINT("MT_UNF_PMOC_SetGpenPin failed, ret = %d\n", ret);
        MT_UNF_PMOC_DeInit();
    }

//      wconfig.w_key.irda_wkey0 = 0x45;
//      wconfig.w_key.irda_wkey1 = 0x23;
    wconfig.w_key.fp_wkey = 0xe;
    ret = MT_UNF_PMOC_SetWakeUpAttr(wconfig);
    if(ret < 0)
    {
        MTADP_HDMI_ERR_PRINT("MT_UNF_PMOC_SetWakeUpAttr  failed\n");
        MT_UNF_PMOC_DeInit();
        return ret;
    }

    ret = MTADP_Switch_Standby_Mode();
    if(MT_SUCCESS != ret)
    {
        MTADP_HDMI_ERR_PRINT("MTADP_Switch_Standby_Mode  failed\n");
    }

    ret = MT_UNF_PMOC_SwitchSystemMode();
    if(ret < 0)
    {
        MTADP_HDMI_ERR_PRINT("MT_UNF_PMOC_SwitchSystemMode  failed\n");
        MT_UNF_PMOC_DeInit();
        return ret;
    }

    return ret;
}

static mt_s32 MT_HDMI_Standby_Fake(void)
{
  mt_s32 Ret = MT_SUCCESS;
  //printf("fake sleep vdc off\n");
  
  MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_0, MT_FALSE);
  MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_1, MT_FALSE);
  MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_2, MT_FALSE);
  MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_3, MT_FALSE);

  MT_UNF_DISP_SetHdVideoEnable(MT_FALSE);
  MT_UNF_DISP_SetSdVideoEnable(MT_FALSE);
  MT_UNF_HDMI_Output_Set(MT_UNF_HDMI_ID_0, MT_FALSE);
  MT_UNF_HDMI_Stop(MT_UNF_HDMI_ID_0);
  MT_UNF_SND_SetAdacOnOff(MT_UNF_SND_0, 0);
  //MT_UNF_HDMI_Close(MT_UNF_HDMI_ID_0);

  return Ret;
}

static MT_S32 MT_HDMI_Wakeup(void)
{
    MT_S32                   Ret = MT_FAILURE;
    //MT_UNF_HDMI_OPEN_PARA_S  DefaultMode = { 0 };
	//printf("fake sleep vdc on.\n");

    //Ret = MT_UNF_HDMI_Open(MT_UNF_HDMI_ID_0, &DefaultMode);
    //enable hdmi only
    Ret |= MT_UNF_HDMI_Start(MT_UNF_HDMI_ID_0);
	Ret |= MT_UNF_HDMI_Output_Set(MT_UNF_HDMI_ID_0, MT_TRUE);
    Ret |= MT_UNF_DISP_SetSdVideoEnable(MT_TRUE);
    Ret |= MT_UNF_DISP_SetHdVideoEnable(MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_3, MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_2, MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_1, MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_0, MT_TRUE);
	Ret |= MT_UNF_SND_SetAdacOnOff(MT_UNF_SND_0, 1);
    if (Ret != MT_SUCCESS)
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_void HDMI_Event_Proc(MT_UNF_HDMI_EVENT_TYPE_E event, mt_void *pPrivateData)
{
    MT_S32 ret = 0;
    MT_S32     volume = 0;
    MT_UNF_SND_GAIN_ATTR_S stGainVolume = { 0 };
    mt_hdmi_cec_standby_mode standby_smode;


    switch (event)
    {
        case MT_UNF_HDMI_EVENT_HOTPLUG:
            HDMI_HotPlug_Proc(pPrivateData);
            break;
        case MT_UNF_HDMI_EVENT_NO_PLUG:
            HDMI_UnPlug_Proc(pPrivateData);
            break;
        case MT_UNF_HDMI_EVENT_EDID_FAIL:
            break;
        case MT_UNF_HDMI_EVENT_HDCP_FAIL:
            HDMI_HdcpFail_Proc(pPrivateData);
            break;
        case MT_UNF_HDMI_EVENT_HDCP_SUCCESS:
            HDMI_HdcpSuccess_Proc(pPrivateData);
            break;
        case MT_UNF_HDMI_EVENT_RSEN_CONNECT:
            //printf("MT_UNF_HDMI_EVENT_RSEN_CONNECT**********\n");
            break;
        case MT_UNF_HDMI_EVENT_RSEN_DISCONNECT:
            //printf("MT_UNF_HDMI_EVENT_RSEN_DISCONNECT**********\n");
            break;
        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_STANDBY:
            {
                MTADP_HDMI_INFO_PRINT("MT_UNF_HDMI_EVENT_TV_SEND_CEC_STANDBY**********\n");
                if(g_cec_cfg.cec_enable != 1)
                {
                    break;
                }

                MTADP_HDMI_INFO_PRINT("box will enter standby mode!!!!\n");
                standby_smode = MTADP_HDMI_Get_Standby_mode();
                switch (standby_smode)
                {
                    case MT_HDMI_CEC_STANDBY_DEFAULT:
                    case MT_HDMI_CEC_STANDBY_PMU:
                        MT_HDMI_Standby_Pmu();
                        break;

                    case MT_HDMI_CEC_STANDBY_FAKE:
                        MT_HDMI_Standby_Fake();
                        break;

                    case MT_HDMI_CEC_STANDBY_STR:
                        /*prevent the TV from sending two standby messages, which would cause the resources to be released twice.*/
                        if (pthread_mutex_trylock(&g_mutex) != 0) {
                            break;
                        }

                        MT_HDMI_Standby_Str();
                        pthread_mutex_unlock(&g_mutex);
                        break;
                    default:
                        MT_HDMI_Standby_Pmu();
                        break;
                }
            }
            break;
        case MT_UNF_HDMI_EVENT_CEC_STATUS_POWER_ON:
            MTADP_HDMI_INFO_PRINT("MT_UNF_HDMI_EVENT_CEC_STATUS_POWER_ON**********\n");
            if(g_cec_cfg.cec_enable != 1)
            {
                break;
            }

            MTADP_HDMI_INFO_PRINT("box will enter wakeup mode!!!!\n");
            standby_smode = MTADP_HDMI_Get_Standby_mode();
            switch (standby_smode)
            {
                case MT_HDMI_CEC_STANDBY_DEFAULT:
                case MT_HDMI_CEC_STANDBY_PMU:
                    break;

                case MT_HDMI_CEC_STANDBY_FAKE:
                    MT_HDMI_Wakeup();
                    break;

                case MT_HDMI_CEC_STANDBY_STR:
                    break;
                default:
                    break;
            }
            break;
        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_LEFT:
            MTADP_HDMI_INFO_PRINT("MT_UNF_HDMI_EVENT_TV_SEND_CEC_LEFT**********\n");
            if(g_cec_cfg.cec_enable != 1)
            {
                break;
            }
            ret = MT_UNF_SND_GetVolume(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, &stGainVolume);
            if(MT_SUCCESS != ret)
            {
                MTADP_HDMI_ERR_PRINT("failed to MT_UNF_SND_GetVolume\n");
                break;
            }
            volume = stGainVolume.s32Gain;
            volume -= 1;
            if(volume < 0)
            {
                MTADP_HDMI_ERR_PRINT("The volume has reached its minimum 0.\n");
                volume = 0;
            }

            MTADP_HDMI_INFO_PRINT(" Set volume to %d \n",  volume);
            stGainVolume.s32Gain = volume;
            ret = MT_UNF_SND_SetVolume( MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, &stGainVolume);
            if(MT_SUCCESS != ret)
            {
                MTADP_HDMI_ERR_PRINT("failed to MT_UNF_SND_SetVolume\n");
            }
            break;
        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_RIGHT:
            MTADP_HDMI_INFO_PRINT("MT_UNF_HDMI_EVENT_TV_SEND_CEC_RIGHT**********\n");
            if(g_cec_cfg.cec_enable != 1)
            {
                break;
            }
            ret = MT_UNF_SND_GetVolume(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, &stGainVolume);
            if(MT_SUCCESS != ret)
            {
                MTADP_HDMI_ERR_PRINT("failed to MT_UNF_SND_GetVolume\n");
                break;
            }
            volume = stGainVolume.s32Gain;
            volume += 1;
            if(volume > 100)
            {
                MTADP_HDMI_ERR_PRINT("The volume has reached its maximum 100.\n");
                volume = 100;
            }

            MTADP_HDMI_INFO_PRINT(" Set volume to %d \n",  volume);
            stGainVolume.s32Gain = volume;
            ret = MT_UNF_SND_SetVolume( MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, &stGainVolume);
            if(MT_SUCCESS != ret)
            {
                MTADP_HDMI_ERR_PRINT("failed to MT_UNF_SND_SetVolume\n");
            }
            break;

        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_OK:
            MTADP_HDMI_INFO_PRINT("MT_UNF_HDMI_EVENT_TV_SEND_CEC_OK**********\n");
            break;
        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_UP:
            MTADP_HDMI_INFO_PRINT("MT_UNF_HDMI_EVENT_TV_SEND_CEC_UP**********\n");
            break;
        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_DOWN:
            MTADP_HDMI_INFO_PRINT("MT_UNF_HDMI_EVENT_TV_SEND_CEC_DOWN**********\n");
            break;
        default:
            break;
    }
    /* Private Usage */
    if ((g_HDMIUserCallbackFlag == MT_TRUE) && (pfnHdmiUserCallback != NULL))
    {
        pfnHdmiUserCallback(event, NULL);
    }

    return;
}

#ifdef MT_HDCP_SUPPORT
static mt_s32 MTADP_HDMI_SetHDCPKey(MT_UNF_HDMI_ID_E enHDMIId)
{
    MT_UNF_HDMI_LOAD_KEY_S stLoadKey;
    FILE *pBinFile;
    mt_u32 u32Len;
    mt_u32 u32Ret;

    pBinFile = fopen(pstencryptedHdcpKey, "rb");
    if (MT_NULL == pBinFile) {
    MTADP_HDMI_INFO_PRINT("can't find key file\n");
    return MT_FAILURE;
    }
    fseek(pBinFile, 0, SEEK_END);
    u32Len = ftell(pBinFile);
    fseek(pBinFile, 0, SEEK_SET);

    stLoadKey.u32KeyLength = u32Len; //332
    stLoadKey.pu8InputEncryptedKey = (mt_u8 *)malloc(u32Len);
    if (MT_NULL == stLoadKey.pu8InputEncryptedKey) {
    MTADP_HDMI_INFO_PRINT("malloc erro!\n");
    fclose(pBinFile);
    return MT_FAILURE;
    }
    if (u32Len != fread(stLoadKey.pu8InputEncryptedKey, 1, u32Len, pBinFile)) {
    MTADP_HDMI_INFO_PRINT("read file %d!\n", __LINE__);
    fclose(pBinFile);
    free(stLoadKey.pu8InputEncryptedKey);
    return MT_FAILURE;
    }

    u32Ret = MT_UNF_HDMI_LoadHDCPKey(enHDMIId, &stLoadKey);
    free(stLoadKey.pu8InputEncryptedKey);
    fclose(pBinFile);

    return u32Ret;
}
#endif



#ifdef CONFIG_MT_CHIP_SYMPHONY4
static mt_u8 g_hdcp_key_m2m_hdmi20[304] = {0};
#elif defined CONFIG_MT_CHIP_SYMPHONY6
static unsigned char g_hdcp_key_m2m_hdmi20[292] = {0};
#endif

mt_void MTADP_HDMI_SetStandbyStatus(mt_s32 standby_flag)
{
    g_HDMI_Standby = standby_flag;
}

mt_void MTADP_HDMI_GetStandbyStatus(mt_s32 *standby_flag)
{
    *standby_flag = g_HDMI_Standby;
}

mt_void MTADP_HDMI_Set_HdcpEnable(mt_u32 enable)
{
    g_HDCPFlag = enable;
}

mt_u32 MTADP_HDMI_Get_HdcpEnable(mt_void)
{
    return g_HDCPFlag;
}

mt_void MTADP_HDMI_Set_Standby_mode(mt_hdmi_cec_standby_mode standby_mode)
{
    g_hdmi_standby_mode = standby_mode;
}

mt_u32 MTADP_HDMI_Get_Standby_mode(mt_void)
{
    return g_hdmi_standby_mode;
}

static mt_s32 MTADP_HDMI_Set_HDCPKey(MT_UNF_HDMI_ID_E enHDMIId, mt_u8 *p_key, mt_u32 key_len)
{
    mt_s32 Ret = MT_FAILURE;
    MT_UNF_HDMI_LOAD_KEY_S stLoadKey = { 0 };

    //printf("\n %s line[%d].\n", __FILE__, __LINE__);
    stLoadKey.u32KeyLength = key_len; //332
    stLoadKey.pu8InputEncryptedKey = (mt_u8 *)mt_malloc(MT_ID_HDMI, key_len);

    memcpy(stLoadKey.pu8InputEncryptedKey, p_key, key_len);

    Ret = MT_UNF_HDMI_LoadHDCPKey(enHDMIId, &stLoadKey);
    mt_free(MT_ID_HDMI, stLoadKey.pu8InputEncryptedKey);

    return Ret;

}

mt_s32 MTADP_HDMI_ReadHdcpKey(MT_UNF_HDMI_ID_E enHDMIId, mt_u8 *p_key, mt_u32 key_len)
{
    MT_U32 mtddev = 17;//DEFAULT_MTDDEV;
    char mtddev_name[64] = {0};
    ulong gmtd_handle = 0;
    mt_s32 ret = MT_FAILURE;
    FILE* file = NULL;
    char idname[4] = {0};
	MT_U32 rval = 0;

	ret = mt_sys_read_register(SYMPHONY_IO_PA(0x1f140020UL), &rval );
	if ( MT_FAILURE != ret ) {
		switch ( (rval>> 14)&0x3 ) {
			case 0:
			case 1:
			case 2:
				system("cat /proc/mtd | grep user > /tmp/hdcpk_ttt.txt");
				file = fopen("/tmp/hdcpk_ttt.txt", "rb");
				printf("%s_%d:bootdev:%d,flash hdcp key file %p\n",__func__,__LINE__,(rval>> 14)&0x3,file);
				if (NULL != file)
				{
					fseek(file, 3, SEEK_SET);
					fread(idname, 1, 1, file);
					fread(idname+1, 1, 1, file);
					if(idname[1] == ':') {
						idname[1] = 0;
					}
					mtddev=atoi(idname);
					fclose(file);
					file = NULL;
				}

				ret = mt_unf_flash_init();
				if ( ret == MT_SUCCESS )
				{
					sprintf(mtddev_name,"/dev/mtd%d",mtddev);
					ret = mt_unf_flash_open(mtddev_name, &gmtd_handle);
					if (MT_SUCCESS == ret)
					{
						ret = mt_unf_flash_read(gmtd_handle, 0, p_key, key_len);
					}
				}

				(MT_VOID)mt_unf_flash_close(gmtd_handle);

				(MT_VOID)mt_unf_flash_deinit();

				break;
			case 3:
				mtddev = 18;
				system("dd if=/dev/mmcblk0p18 of=/tmp/hdcpkey_test.bin bs=512 count=1");
				file = fopen("/tmp/hdcpkey_test.bin", "rb");
				printf("%s_%d:emmc hdcp key file %p\n",__func__,__LINE__,file);
			    if (NULL != file)
			    {
			        fread(p_key, 289, 1, file);
					//printf("%s_%d:%d storage:%d,%d,%d,%d,%d,%d\n",__func__,__LINE__,ret,p_key[0],p_key[1],p_key[2],p_key[287],p_key[288]);
			        fclose(file);
			        file = NULL;
					ret = MT_SUCCESS;
			    } else {
					ret = MT_FAILURE;
				}
				system("rm -f /tmp/hdcpkey_test.bin");

				break;
			default :
				break;
		}
		printf("%s_%d:%d\n",__func__,__LINE__,ret);
	}
    return ret;
}

mt_s32 MTADP_HDMI_Init(MT_UNF_HDMI_ID_E enHDMIId, MT_UNF_ENC_FMT_E enWantFmt)
{
    mt_s32 Ret = MT_FAILURE;
    MT_UNF_HDMI_OPEN_PARA_S stOpenParam;
    MT_UNF_HDMI_DELAY_S stDelay;

    g_stHdmiArgs.enHdmi = enHDMIId;

    Ret = MT_UNF_HDMI_Init();
    if (MT_SUCCESS != Ret) {
        MTADP_HDMI_ERR_PRINT("MT_UNF_HDMI_Init failed:%#x\n", Ret);
        return MT_FAILURE;
    }

#if (defined(CONFIG_MT_CHIP_ETUDE2) || defined(CONFIG_MT_CHIP_SYMPHONY6))
    {
    unsigned char hdcp_key_m2m_hdmi20_tmp[304] = {0};
    mt_u32 key_len = 0;

    key_len = sizeof(g_hdcp_key_m2m_hdmi20)/sizeof(g_hdcp_key_m2m_hdmi20[0]);
    memset(g_hdcp_key_m2m_hdmi20, 0, key_len);
    (void)MTADP_HDMI_ReadHdcpKey(MT_UNF_HDMI_ID_0, g_hdcp_key_m2m_hdmi20, key_len);
    memcpy(hdcp_key_m2m_hdmi20_tmp, g_hdcp_key_m2m_hdmi20, key_len);

    Ret = MTADP_HDMI_Set_HDCPKey(enHDMIId, hdcp_key_m2m_hdmi20_tmp, key_len);
    if (Ret != MT_SUCCESS) {
        MTADP_HDMI_ERR_PRINT("Set hdcp erro:%#x\n", Ret);
        MT_UNF_HDMI_DeInit();
        return MT_FAILURE;
    }
    }
#else
    {
    Ret = MTADP_HDMI_Set_HDCPKey(enHDMIId, g_hdcp_key_m2m_hdmi20, hdcp_key_len);
    if (Ret != MT_SUCCESS) {
        MTADP_HDMI_ERR_PRINT("Set hdcp erro:%#x\n", Ret);
        MT_UNF_HDMI_DeInit();
        return MT_FAILURE;
    }
    }
#endif

    MT_UNF_HDMI_GetDelay(0, &stDelay);
    stDelay.bForceFmtDelay = MT_TRUE;
    stDelay.bForceMuteDelay = MT_TRUE;
    stDelay.u32FmtDelay = 500;
    stDelay.u32MuteDelay = 120;
    MT_UNF_HDMI_SetDelay(0, &stDelay);

    g_stCallbackFunc.pfnHdmiEventCallback = HDMI_Event_Proc;
    g_stCallbackFunc.pPrivateData = &g_stHdmiArgs;
    Ret = MT_UNF_HDMI_RegCallbackFunc(enHDMIId, &g_stCallbackFunc);
    if (Ret != MT_SUCCESS) {
        MTADP_HDMI_ERR_PRINT("hdmi reg failed:%#x\n", Ret);
        MT_UNF_HDMI_DeInit();
        return MT_FAILURE;
    }

    memset(&stOpenParam, 0, sizeof(MT_UNF_HDMI_OPEN_PARA_S));
    stOpenParam.enDefaultMode = g_enDefaultMode; //MT_UNF_HDMI_FORCE_NULL;
    stOpenParam.u32InitParam |= HDMI_INIT_OUT_HDCP_KEY;
    Ret = MT_UNF_HDMI_Open(enHDMIId, &stOpenParam);
    if (Ret != MT_SUCCESS) {
        MTADP_HDMI_ERR_PRINT("MT_UNF_HDMI_Open failed:%#x\n", Ret);
        MT_UNF_HDMI_DeInit();
        return MT_FAILURE;
    }

    Ret = MT_UNF_HDMI_CEC_Enable(MT_UNF_HDMI_ID_0);
    if (Ret != MT_SUCCESS) {
        MTADP_HDMI_ERR_PRINT("MT_UNF_HDMI_Open failed:%#x\n", Ret);
        MT_UNF_HDMI_Close(enHDMIId);
        MT_UNF_HDMI_DeInit();
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_E enHDMIId)
{
    MT_UNF_HDMI_Close(enHDMIId);

    MT_UNF_HDMI_UnRegCallbackFunc(enHDMIId, &g_stCallbackFunc);

    MT_UNF_HDMI_DeInit();

    return MT_SUCCESS;
}
