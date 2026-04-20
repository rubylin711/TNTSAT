/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mt_unf_hdmi.h"
#include "mt_unf_edid.h"
#include "mt_unf_disp.h"
#include "mt_adp.h"
#include "mt_adp_hdmi.h"

//#include "mt_adp_audio.h"

typedef struct mtHDMI_ARGS_S
{
    MT_UNF_HDMI_ID_E  enHdmi;
}HDMI_ARGS_S;

static HDMI_ARGS_S g_stHdmiArgs;
//static mt_u32 g_HDCPFlag         = MT_FALSE;
static mt_u32 g_HDMI_Bebug       = MT_FALSE;
static mt_u32 g_HDMIUserCallbackFlag = MT_FALSE;
static mt_u32 g_enDefaultMode    = MT_UNF_HDMI_DEFAULT_ACTION_HDMI;//MT_UNF_HDMI_DEFAULT_ACTION_NULL;
static MT_UNF_HDMI_CALLBACK_FUNC_S g_stCallbackFunc;

static User_HDMI_CallBack pfnHdmiUserCallback = NULL;
//#define MT_HDCP_SUPPORT
#ifdef MT_HDCP_SUPPORT
const mt_char * pstencryptedHdcpKey = "EncryptedKey_332bytes.bin";
#endif

static mt_char *g_pDispFmtString[MT_UNF_ENC_FMT_BUTT+1] = {
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

    if (NULL == pszFmt)
    {
        return MT_UNF_ENC_FMT_BUTT;
    }

    for (i = 0; i < MT_UNF_ENC_FMT_BUTT; i++)
    {
        if (strcasestr(pszFmt, g_pDispFmtString[i]))
    	{
    		fmtReturn = i;
    		break;
    	}
    }

    if (i >= MT_UNF_ENC_FMT_BUTT)
    {
        i = MT_UNF_ENC_FMT_720P_50;
        fmtReturn = i;
        sample_common_printf("\n!!! Can NOT match format, set format to is '%s'/%d.\n\n", g_pDispFmtString[i], i);
    }
    else
    {
	    sample_common_printf("\n!!! The format is '%s'/%d.\n\n", g_pDispFmtString[i], i);
    }
	return fmtReturn;
}


static mt_void HDMI_PrintAttr(MT_UNF_HDMI_ATTR_S *pstHDMIAttr)
{
    if (MT_TRUE != g_HDMI_Bebug)
    {
        return;
    }

    sample_common_printf("=====MT_UNF_HDMI_SetAttr=====\n"
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
           pstHDMIAttr->enVidOutMode,pstHDMIAttr->enDeepColorMode,pstHDMIAttr->bxvYCCMode,
           pstHDMIAttr->bEnableAudio,
           pstHDMIAttr->bEnableAudInfoFrame,pstHDMIAttr->bEnableAudInfoFrame,
           pstHDMIAttr->bEnableSpdInfoFrame,pstHDMIAttr->bEnableMpegInfoFrame);
    return;
}


void HDMI_HotPlug_Proc_gst(mt_void *pPrivateData)
{
    mt_s32          ret = MT_SUCCESS;
    HDMI_ARGS_S     *pArgs  = (HDMI_ARGS_S*)pPrivateData;
    MT_UNF_HDMI_ID_E       hHdmi   =  pArgs->enHdmi;
    MT_UNF_HDMI_ATTR_S             stHdmiAttr;
    //MT_UNF_HDMI_INFOFRAME_S        stInfoFrame;
    MT_UNF_EDID_BASE_INFO_S        stSinkCap;
    MT_UNF_HDMI_STATUS_S           stHdmiStatus;

#ifdef MT_HDCP_SUPPORT
    static mt_u8 u8FirstTimeSetting = MT_TRUE;
#endif

    sample_common_printf("\n --- Get HDMI event: HOTPLUG. --- \n");

    MT_UNF_HDMI_GetStatus(hHdmi,&stHdmiStatus);
    if (MT_FALSE == stHdmiStatus.bConnected)
    {
        sample_common_printf("No Connect\n");
        return;
    }


    MT_UNF_HDMI_GetAttr(hHdmi, &stHdmiAttr);
    ret = MT_UNF_HDMI_GetSinkCapability(hHdmi, &stSinkCap);


    if(ret == MT_SUCCESS)
    {
        //stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_YCBCR444;
        if(MT_TRUE == stSinkCap.bSupportHdmi)
        {
            stHdmiAttr.bEnableHdmi = MT_TRUE;
        }
        else
        {
            stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
            //读取到了edid，并且不支持hdmi则进入dvi模式
            //read real edid ok && sink not support hdmi,then we run in dvi mode
            stHdmiAttr.bEnableHdmi = MT_FALSE;
        }
    }
    else
    {
        //when get capability fail,use default mode
        if(g_enDefaultMode != MT_UNF_HDMI_DEFAULT_ACTION_DVI)
            stHdmiAttr.bEnableHdmi = MT_TRUE;
        else
            stHdmiAttr.bEnableHdmi = MT_FALSE;
    }

    if(MT_TRUE == stHdmiAttr.bEnableHdmi)
    {
        stHdmiAttr.bEnableAudio = MT_TRUE;
        stHdmiAttr.bEnableVideo = MT_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = MT_TRUE;
        stHdmiAttr.bEnableAviInfoFrame = MT_TRUE;
        stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_BUTT;// MT_UNF_HDMI_VIDEO_MODE_BUTT will auto output
    }
    else
    {
        stHdmiAttr.bEnableAudio = MT_FALSE;
        stHdmiAttr.bEnableVideo = MT_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = MT_FALSE;
        stHdmiAttr.bEnableAviInfoFrame = MT_FALSE;
        stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
    }

#ifdef MT_HDCP_SUPPORT
    if (u8FirstTimeSetting == MT_TRUE)
    {
        u8FirstTimeSetting = MT_FALSE;
        if (g_HDCPFlag == MT_TRUE)
        {
            stHdmiAttr.bHDCPEnable = MT_TRUE;//Enable HDCP
        }
        else
        {
            stHdmiAttr.bHDCPEnable= MT_FALSE;
        }
    }
    else
    {
        //HDCP Enable use default setting!!
    }
#endif

    ret = MT_UNF_HDMI_SetAttr(hHdmi, &stHdmiAttr);

    /* MT_UNF_HDMI_SetAttr must before MT_UNF_HDMI_Start! */
    ret = MT_UNF_HDMI_Start(hHdmi);

    HDMI_PrintAttr(&stHdmiAttr);

    return;

}

mt_void HDMI_UnPlug_Proc_gst(mt_void *pPrivateData)
{
    HDMI_ARGS_S     *pArgs  = (HDMI_ARGS_S*)pPrivateData;
    MT_UNF_HDMI_ID_E       hHdmi   =  pArgs->enHdmi;

    sample_common_printf("\n --- Get HDMI event: UnPlug. --- \n");
    MT_UNF_HDMI_Stop(hHdmi);

    return;
}
static mt_u32 HDCPFailCount = 0;
mt_void HDMI_HdcpFail_Proc_gst(mt_void *pPrivateData)
{
    //MT_UNF_HDMI_ATTR_S             stHdmiAttr;
    sample_common_printf("\n --- Get HDMI event: HDCP_FAIL. --- \n");

    HDCPFailCount ++ ;
    if(HDCPFailCount >= 50)
    {
        HDCPFailCount = 0;
        sample_common_printf("\nWarrning:Customer need to deal with HDCP Fail!!!!!!\n");
    }
#if 0
    MT_UNF_HDMI_GetAttr(0, &stHdmiAttr);

    stHdmiAttr.bHDCPEnable = MT_FALSE;

    MT_UNF_HDMI_SetAttr(0, &stHdmiAttr);
#endif
    return;
}

mt_void HDMI_HdcpSuccess_Proc_gst(mt_void *pPrivateData)
{
    sample_common_printf("\n --- Get HDMI event: HDCP_SUCCESS. --- \n");
    return;
}

mt_void HDMI_Event_Proc_gst(MT_UNF_HDMI_EVENT_TYPE_E event, 
								mt_void *pPrivateData)
{

    switch ( event )
    {
        case MT_UNF_HDMI_EVENT_HOTPLUG:
            HDMI_HotPlug_Proc_gst(pPrivateData);
            break;
        case MT_UNF_HDMI_EVENT_NO_PLUG:
            HDMI_UnPlug_Proc_gst(pPrivateData);
            break;
        case MT_UNF_HDMI_EVENT_EDID_FAIL:
            break;
        case MT_UNF_HDMI_EVENT_HDCP_FAIL:
            HDMI_HdcpFail_Proc_gst(pPrivateData);
            break;
        case MT_UNF_HDMI_EVENT_HDCP_SUCCESS:
            HDMI_HdcpSuccess_Proc_gst(pPrivateData);
            break;
        case MT_UNF_HDMI_EVENT_RSEN_CONNECT:
            //printf("MT_UNF_HDMI_EVENT_RSEN_CONNECT**********\n");
            break;
        case MT_UNF_HDMI_EVENT_RSEN_DISCONNECT:
            //printf("MT_UNF_HDMI_EVENT_RSEN_DISCONNECT**********\n");
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
mt_s32 MTADP_HDMI_SetHDCPKey(MT_UNF_HDMI_ID_E enHDMIId)
{
	MT_UNF_HDMI_LOAD_KEY_S stLoadKey;
	FILE *pBinFile;
	mt_u32 u32Len;
	mt_u32 u32Ret;

	pBinFile = fopen(pstencryptedHdcpKey, "rb");
	if(MT_NULL == pBinFile)
	{
		sample_common_printf("can't find key file\n");
		return MT_FAILURE;
	}
	fseek(pBinFile, 0, SEEK_END);
	u32Len = ftell(pBinFile);
	fseek(pBinFile, 0, SEEK_SET);

	stLoadKey.u32KeyLength = u32Len; //332
	stLoadKey.pu8InputEncryptedKey  = (mt_u8*)malloc(u32Len);
	if(MT_NULL == stLoadKey.pu8InputEncryptedKey)
	{
		sample_common_printf("malloc erro!\n");
		fclose(pBinFile);
        free(stLoadKey.pu8InputEncryptedKey);       // for tsscan
		return MT_FAILURE;
	}
	if (u32Len != fread(stLoadKey.pu8InputEncryptedKey, 1, u32Len, pBinFile))
	{
		sample_common_printf("read file %d!\n", __LINE__);
		fclose(pBinFile);
		free(stLoadKey.pu8InputEncryptedKey);
		return MT_FAILURE;
	}

	u32Ret = MT_UNF_HDMI_LoadHDCPKey(enHDMIId,&stLoadKey);
	free(stLoadKey.pu8InputEncryptedKey);
	fclose(pBinFile);

	return u32Ret;
}
#endif

mt_s32 MTADP_HDMI_Init(MT_UNF_HDMI_ID_E enHDMIId, MT_UNF_ENC_FMT_E enWantFmt)
{
    mt_s32 Ret = MT_FAILURE;
    MT_UNF_HDMI_OPEN_PARA_S stOpenParam;
    MT_UNF_HDMI_DELAY_S  stDelay;

    g_stHdmiArgs.enHdmi       = enHDMIId;

    Ret = MT_UNF_HDMI_Init();
    if (MT_SUCCESS != Ret)
    {
        sample_common_printf("MT_UNF_HDMI_Init failed:%#x\n",Ret);
        return MT_FAILURE;
    }
#ifdef MT_HDCP_SUPPORT
	Ret = MTADP_HDMI_SetHDCPKey(enHDMIId);
	if (MT_SUCCESS != Ret)
    {
        sample_common_printf("Set hdcp erro:%#x\n",Ret);
		//return MT_FAILURE;
    }
#endif

    MT_UNF_HDMI_GetDelay(0,&stDelay);
    stDelay.bForceFmtDelay = MT_TRUE;
    stDelay.bForceMuteDelay = MT_TRUE;
    stDelay.u32FmtDelay = 500;
    stDelay.u32MuteDelay = 120;
    MT_UNF_HDMI_SetDelay(0,&stDelay);


	g_stCallbackFunc.pfnHdmiEventCallback = HDMI_Event_Proc_gst;
	g_stCallbackFunc.pPrivateData = &g_stHdmiArgs;

	Ret = MT_UNF_HDMI_RegCallbackFunc(enHDMIId, &g_stCallbackFunc);
	if (Ret != MT_SUCCESS)
    {
        sample_common_printf("hdmi reg failed:%#x\n",Ret);
        MT_UNF_HDMI_DeInit();
        return MT_FAILURE;
    }

	stOpenParam.enDefaultMode = g_enDefaultMode;//MT_UNF_HDMI_FORCE_NULL;
    Ret = MT_UNF_HDMI_Open(enHDMIId, &stOpenParam);
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("MT_UNF_HDMI_Open failed:%#x\n",Ret);
        MT_UNF_HDMI_DeInit();
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


mt_s32 MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_E enHDMIId)
{

    MT_UNF_HDMI_Stop(enHDMIId);

    MT_UNF_HDMI_Close(enHDMIId);

	MT_UNF_HDMI_UnRegCallbackFunc(enHDMIId, &g_stCallbackFunc);

    MT_UNF_HDMI_DeInit();

    return MT_SUCCESS;
}

