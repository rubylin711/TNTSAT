/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "mt_type.h"
#include "mt_unf_disp.h"
#include "demo.h"


static HDMI_ARGS_S g_stHdmiArgs;
static MT_UNF_HDMI_CALLBACK_FUNC_S g_stCallbackFunc;
static mt_s32 hdmi_status = 0;
mt_u32 g_enDefaultMode = MT_UNF_HDMI_DEFAULT_ACTION_HDMI;

static mt_void demo_hdmi_print(MT_UNF_HDMI_ATTR_S *pstHDMIAttr)
{
#if 0
    printf("=====MT_UNF_HDMI_SetAttr=====\n"
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
#endif
    return;
}

void demo_hdmi_hotplug_proc(mt_void *pPrivateData)
{
	mt_s32 ret = MT_SUCCESS;
	HDMI_ARGS_S *pArgs = (HDMI_ARGS_S *)pPrivateData;
	MT_UNF_HDMI_ID_E hHdmi = pArgs->enHdmi;
	MT_UNF_HDMI_ATTR_S stHdmiAttr;
	MT_UNF_EDID_BASE_INFO_S stSinkCap;
	MT_UNF_HDMI_STATUS_S stHdmiStatus;

	printf("\n --- Get HDMI event: HOTPLUG. --- \n");

	MT_UNF_HDMI_GetStatus(hHdmi, &stHdmiStatus);
	if (MT_FALSE == stHdmiStatus.bConnected)
	{
		printf("%s(line: %d), No Connect\n", __FUNCTION__, __LINE__);
		return;
	}

	MT_UNF_HDMI_GetAttr(hHdmi, &stHdmiAttr);
	ret = MT_UNF_HDMI_GetSinkCapability(hHdmi, &stSinkCap);

	if (ret == MT_SUCCESS)
	{
		//stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_YCBCR444;
		if (MT_TRUE == stSinkCap.bSupportHdmi)
		{
			stHdmiAttr.bEnableHdmi = MT_TRUE;
			if (MT_TRUE != stSinkCap.stColorSpace.bYCbCr444)
			{
				stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
			}
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
		if (g_enDefaultMode != MT_UNF_HDMI_DEFAULT_ACTION_DVI)
			stHdmiAttr.bEnableHdmi = MT_TRUE;
		else
			stHdmiAttr.bEnableHdmi = MT_FALSE;
	}

	if (MT_TRUE == stHdmiAttr.bEnableHdmi)
	{
		stHdmiAttr.bEnableAudio = MT_TRUE;
		stHdmiAttr.bEnableVideo = MT_TRUE;
		stHdmiAttr.bEnableAudInfoFrame = MT_TRUE;
		stHdmiAttr.bEnableAviInfoFrame = MT_TRUE;
	}
	else
	{
		stHdmiAttr.bEnableAudio = MT_FALSE;
		stHdmiAttr.bEnableVideo = MT_TRUE;
		stHdmiAttr.bEnableAudInfoFrame = MT_FALSE;
		stHdmiAttr.bEnableAviInfoFrame = MT_FALSE;
		stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
	}

	ret = MT_UNF_HDMI_SetAttr(hHdmi, &stHdmiAttr);

	/* MT_UNF_HDMI_SetAttr must before MT_UNF_HDMI_Start! */
	ret = MT_UNF_HDMI_Start(hHdmi);

	demo_hdmi_print(&stHdmiAttr);

	return;
}

mt_void demo_hdmi_unplug_proc(mt_void *pPrivateData)
{
    HDMI_ARGS_S *pArgs = (HDMI_ARGS_S *)pPrivateData;
    MT_UNF_HDMI_ID_E hHdmi = pArgs->enHdmi;

    printf("\n --- Get HDMI event: UnPlug. --- \n");
    MT_UNF_HDMI_Stop(hHdmi);
	hdmi_status = 0;
    return;
}

mt_void demo_get_hdmi_status(mt_s32 *status)
{
	*status = hdmi_status;
}

mt_void demo_hdmi_event_proc(MT_UNF_HDMI_EVENT_TYPE_E event, mt_void *pPrivateData)
{

	printf("%s(line: %d), event %d\n", __FUNCTION__, __LINE__, event);
	switch (event)
	{
		case MT_UNF_HDMI_EVENT_HOTPLUG:
			demo_hdmi_hotplug_proc(pPrivateData);
			break;
		case MT_UNF_HDMI_EVENT_NO_PLUG:
			demo_hdmi_unplug_proc(pPrivateData);
			break;
		default:
			break;
	}

	return;
}

mt_s32 demo_hdmi_enable(MT_UNF_HDMI_ID_E enHDMIId)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_HDMI_ATTR_S stHdmiAttr;
    MT_UNF_EDID_BASE_INFO_S stSinkCap;

 	//printf("MT_Av_Dvb_HDMI_enable enter.\n");

    ret = MT_UNF_HDMI_GetAttr(enHDMIId, &stHdmiAttr);
    if (MT_SUCCESS != ret)
    {
        printf("%s(line: %d), MT_UNF_HDMI_GetAttr failed!\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

	printf("%s(line: %d), bHDCPEnable=%d!\n", __FUNCTION__, __LINE__, stHdmiAttr.bHDCPEnable);

    ret = MT_UNF_HDMI_GetSinkCapability(enHDMIId, &stSinkCap);
    if (MT_SUCCESS != ret)
    {
        printf("%s(line: %d), MT_UNF_HDMI_GetSinkCapability failed, Err_Code=0x%x!\n", __FUNCTION__, __LINE__, ret);
        return MT_FAILURE;
    }

    if (ret == MT_SUCCESS)
    {
    	printf("%s(line: %d), sinkcap support hdmi %d, color space YCbCr444 %d\n", __FUNCTION__, __LINE__, stSinkCap.bSupportHdmi, stSinkCap.stColorSpace.bYCbCr444);
        if (MT_TRUE == stSinkCap.bSupportHdmi)
        {
	    	stHdmiAttr.bEnableHdmi = MT_TRUE;
            if (MT_TRUE != stSinkCap.stColorSpace.bYCbCr444)
            {
                stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
            }
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
        printf("%s(line: %d), get sink cap failed.\n", __FUNCTION__, __LINE__);
        stHdmiAttr.bEnableHdmi = MT_TRUE;
    }

    if (MT_TRUE == stHdmiAttr.bEnableHdmi)
    {
        stHdmiAttr.bEnableAudio = MT_TRUE;
        stHdmiAttr.bEnableVideo = MT_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = MT_TRUE;
        stHdmiAttr.bEnableAviInfoFrame = MT_TRUE;
    }
    else
    {
        stHdmiAttr.bEnableAudio = MT_FALSE;
        stHdmiAttr.bEnableVideo = MT_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = MT_FALSE;
        stHdmiAttr.bEnableAviInfoFrame = MT_FALSE;
        stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
    }

    ret = MT_UNF_HDMI_SetAttr(enHDMIId, &stHdmiAttr);
    if (MT_SUCCESS != ret)
    {
        printf("%s(line: %d), MT_UNF_HDMI_SetAttr failed!\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


mt_s32 demo_hdmi_init(MT_UNF_HDMI_ID_E enHDMIId, MT_UNF_ENC_FMT_E enWantFmt)
{
    mt_s32 Ret = MT_FAILURE;
    MT_UNF_HDMI_OPEN_PARA_S stOpenParam;
    MT_UNF_HDMI_DELAY_S stDelay;

	printf("%s(line: %d), hdmi %d init.\n", __FUNCTION__, __LINE__, enHDMIId);
	
    g_stHdmiArgs.enHdmi = enHDMIId;

    Ret = MT_UNF_HDMI_Init();
    if (MT_SUCCESS != Ret)
	{
		printf("%s(line: %d), MT_UNF_HDMI_Init failed:%#x\n", __FUNCTION__, __LINE__, Ret);
		return MT_FAILURE;
    }

    Ret = MT_UNF_HDMI_GetDelay(0, &stDelay);
	if(Ret != MT_SUCCESS)
	{
		printf("%s(line: %d), MT_UNF_HDMI_GetDelay failed.\n", __FUNCTION__, __LINE__);
	}
    stDelay.bForceFmtDelay = MT_TRUE;
    stDelay.bForceMuteDelay = MT_TRUE;
    stDelay.u32FmtDelay = 500;
    stDelay.u32MuteDelay = 120;
    Ret = MT_UNF_HDMI_SetDelay(0, &stDelay);
	if(Ret != MT_SUCCESS)
	{
		printf("%s(line: %d), MT_UNF_HDMI_SetDelay failed.\n", __FUNCTION__, __LINE__);
	}
	
    g_stCallbackFunc.pfnHdmiEventCallback = demo_hdmi_event_proc;
    g_stCallbackFunc.pPrivateData = &g_stHdmiArgs;

    Ret = MT_UNF_HDMI_RegCallbackFunc(enHDMIId, &g_stCallbackFunc);
    if (Ret != MT_SUCCESS)
	{
		printf("%s(line: %d), hdmi reg failed:%#x\n", __FUNCTION__, __LINE__, Ret);
		MT_UNF_HDMI_DeInit();
		return MT_FAILURE;
    }

    stOpenParam.enDefaultMode = g_enDefaultMode; //MT_UNF_HDMI_FORCE_NULL;
    Ret = MT_UNF_HDMI_Open(enHDMIId, &stOpenParam);
    if (Ret != MT_SUCCESS)
	{
		printf("%s(line: %d), MT_UNF_HDMI_Open failed:%#x\n", __FUNCTION__, __LINE__, Ret);
		MT_UNF_HDMI_DeInit();
		return MT_FAILURE;
    }

	Ret = demo_hdmi_enable(enHDMIId);
	if(Ret != MT_SUCCESS)
	{
		printf("%s(line: %d), demo_hdmi_enable failed.\n", __FUNCTION__, __LINE__);
	}
	
    return MT_SUCCESS;
}

mt_s32 demo_hdmi_deinit(MT_UNF_HDMI_ID_E enHDMIId)
{

    MT_UNF_HDMI_Stop(enHDMIId);

    MT_UNF_HDMI_Close(enHDMIId);

    MT_UNF_HDMI_UnRegCallbackFunc(enHDMIId, &g_stCallbackFunc);

    MT_UNF_HDMI_DeInit();

    return MT_SUCCESS;
}


mt_s32 demo_disp_init(MT_UNF_ENC_FMT_E enFormat)
{
    mt_s32                      Ret;
    MT_UNF_DISP_BG_COLOR_S      BgColor;
    MT_UNF_DISP_INTF_S          stIntf[2];
    MT_UNF_DISP_OFFSET_S        offset;

	printf("%s(line: %d), demo display init.\n", __FUNCTION__, __LINE__);
    Ret = MT_UNF_DISP_Init();
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_Init failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        return Ret;
    }

    /* set display1 interface */
    stIntf[0].enIntfType                = MT_UNF_DISP_INTF_TYPE_YPBPR;
    stIntf[0].unIntf.stYPbPr.u8DacY     = MT_DAC_YPBPR_Y;
    stIntf[0].unIntf.stYPbPr.u8DacPb    = MT_DAC_YPBPR_PB;
    stIntf[0].unIntf.stYPbPr.u8DacPr    = MT_DAC_YPBPR_PR;
    stIntf[1].enIntfType                = MT_UNF_DISP_INTF_TYPE_HDMI;
    stIntf[1].unIntf.enHdmi             = MT_UNF_HDMI_ID_0;
    Ret = MT_UNF_DISP_AttachIntf(MT_UNF_DISPLAY1, &stIntf[0], 2);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_AttachIntf failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    /* set display0 interface */
    stIntf[0].enIntfType            = MT_UNF_DISP_INTF_TYPE_CVBS;
    stIntf[0].unIntf.stCVBS.u8Dac   = MT_DAC_CVBS;
    Ret = MT_UNF_DISP_AttachIntf(MT_UNF_DISPLAY0, &stIntf[0], 1);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_AttachIntf failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    Ret = MT_UNF_DISP_Attach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_Attach failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        MT_UNF_DISP_DeInit();
        return Ret;
    }
    /* set display1 format*/
    Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY1, enFormat);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_SetFormat failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
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
            printf("%s(line: %d), call MT_UNF_DISP_SetFormat failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
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
            printf("%s(line: %d), call MT_UNF_DISP_SetFormat failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
            MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
            MT_UNF_DISP_DeInit();
            return Ret;
        }
    }
#if 0
    Ret = MT_UNF_DISP_SetVirtualScreen(MT_UNF_DISPLAY1, 1280, 720);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_SetVirtualScreen failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
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
        printf("%s(line: %d), call MT_UNF_DISP_SetBgColor failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    /*set display0 screen offset*/
    Ret = MT_UNF_DISP_SetScreenOffset(MT_UNF_DISPLAY0, &offset);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_SetBgColor failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
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
        printf("%s(line: %d), call MT_UNF_DISP_SetBgColor failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    Ret = MT_UNF_DISP_Open(MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_Open DISPLAY1 failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    Ret = MT_UNF_DISP_Open(MT_UNF_DISPLAY0);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_Open DISPLAY0 failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        MT_UNF_DISP_Close(MT_UNF_DISPLAY1);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 demo_disp_deinit(mt_void)
{
    mt_s32 Ret;

    Ret = MT_UNF_DISP_Close(MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_Close failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        return Ret;
    }

    Ret = MT_UNF_DISP_Close(MT_UNF_DISPLAY0);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_Close failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        return Ret;
    }

    Ret = MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_Detach failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        return Ret;
    }

    Ret = MT_UNF_DISP_DeInit();
    if (Ret != MT_SUCCESS)
    {
    	printf("%s(line: %d), call MT_UNF_DISP_DeInit failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 demo_disp_init_test(void)
{
	mt_s32 	Ret = MT_SUCCESS;
	MT_UNF_DISP_INTF_S          stIntf[2];
	MT_UNF_DISP_BG_COLOR_S      BgColor;

	printf("%s(line: %d), demo display init.\n", __FUNCTION__, __LINE__);
    Ret = MT_UNF_DISP_Init();
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_Init failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        return Ret;
    }
#if 0
	/* set display1 interface */
    stIntf[0].enIntfType                = MT_UNF_DISP_INTF_TYPE_YPBPR;
    stIntf[0].unIntf.stYPbPr.u8DacY     = MT_DAC_YPBPR_Y;
    stIntf[0].unIntf.stYPbPr.u8DacPb    = MT_DAC_YPBPR_PB;
    stIntf[0].unIntf.stYPbPr.u8DacPr    = MT_DAC_YPBPR_PR;
    stIntf[1].enIntfType                = MT_UNF_DISP_INTF_TYPE_HDMI;
    stIntf[1].unIntf.enHdmi             = MT_UNF_HDMI_ID_0;
    Ret = MT_UNF_DISP_AttachIntf(MT_UNF_DISPLAY1, &stIntf[0], 1);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_AttachIntf failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

	Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY1, MT_UNF_ENC_FMT_720P_60);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_SetFormat failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

	BgColor.u8Red   = 0;
    BgColor.u8Green = 0;
    BgColor.u8Blue  = 0;
    Ret = MT_UNF_DISP_SetBgColor(MT_UNF_DISPLAY1, &BgColor);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_SetBgColor failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        MT_UNF_DISP_DeInit();
        return Ret;
    }

    Ret = MT_UNF_DISP_Open(MT_UNF_DISPLAY1);
    if (Ret != MT_SUCCESS)
    {
        printf("%s(line: %d), call MT_UNF_DISP_Open DISPLAY1 failed, Ret=%#x.\n", __FUNCTION__, __LINE__, Ret);
        MT_UNF_DISP_DeInit();
        return Ret;
    }
#endif
	return Ret;
}


