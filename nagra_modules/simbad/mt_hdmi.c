/*
 * Copyright (C) 2021 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mt_common.h"
#include "mt_hdmi.h"

#include "mt_unf_disp.h"
#include "mt_adp_boardcfg.h"

#define EMSG(fmt, ...)   //printf("[ERR]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define DMSG(fmt, ...)   //printf("[DBG]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#if 0
#define OS_PRINTF printf
#else
#define OS_PRINTF
#endif

static struct HDMI_ARGS_S gstHdmiArgs;
mt_u32 g_default_mode = MT_UNF_HDMI_DEFAULT_ACTION_HDMI;

static mt_s32 mt_hdmi_setHdcpKey(MT_UNF_HDMI_ID_E enHDMIId);
static mt_s32 mt_hdmi_enable(MT_UNF_HDMI_ID_E enHDMIId, mt_u8 hdcp_enable);

static mt_void HDMI_Event_Proc(MT_UNF_HDMI_EVENT_TYPE_E event, mt_void *pPrivateData);
static void HDMI_HotPlug_Proc(mt_void *pPrivateData);
static mt_void HDMI_UnPlug_Proc(mt_void *pPrivateData);

static MT_UNF_HDMI_CALLBACK_FUNC_S gstCallbackFunc;

/*Enable by default, for eraly test we an set to false*/
mt_u32 gHDCPFlag = MT_TRUE;
//mt_u32 gHDCPFlag = MT_FALSE;
mt_u32 hdmiConnectFlag = 0;
pfnHdmiUserCallback_t gpfnHdmiUserCallback = NULL;

MT_UNF_HDMI_ATTR_S gHdmiAttr = {0};

extern MT_UNF_ENC_FMT_E testFormat;

#define DISPLAY_MAX_MATCH_FMT_NUM 32
#define DISPLAY_DEFAULT_MATCH_FMT   MT_UNF_ENC_FMT_1080i_50
static u32 disp_fmt_priority[] = {

MT_UNF_ENC_FMT_3840X2160_30,
MT_UNF_ENC_FMT_3840X2160_25,

MT_UNF_ENC_FMT_1080i_50,

MT_UNF_ENC_FMT_720P_50
};

static u32 match_fmt[DISPLAY_MAX_MATCH_FMT_NUM] = {0};
static u8 match_fmt_count = 0;


static MT_UNF_DISP_HDMI_MODE_E disp_hdrMode = MT_UNF_DISP_HDMI_MODE_SDR;
static MT_UNF_DISP_PP_E disp_ppMode = MT_UNF_DISP_PP_STANDARD;
#if 0
ErrorCode_t drv_unf_display_init(MT_UNF_ENC_FMT_E sd_fmt,
                                 MT_UNF_ENC_FMT_E hd_fm,
                                 MT_UNF_DISP_PP_E ppMode,
                                 u8 hdr_mode)
{
    MT_S32 ret;
    MT_UNF_DISP_BG_COLOR_S BgColor;

    ret = MT_UNF_DISP_Init();
    if (ret != MT_SUCCESS) {
	return MT_FAILURE;
    }
     #if 1
    {
      MT_UNF_DISP_INTF_S              stIntf[2];
      /* set display1 interface */
     stIntf[0].enIntfType                            = MT_UNF_DISP_INTF_TYPE_YPBPR;
     stIntf[0].unIntf.stYPbPr.u8DacY         = DAC_YPBPR_Y;
     stIntf[0].unIntf.stYPbPr.u8DacPb        = DAC_YPBPR_PB;
     stIntf[0].unIntf.stYPbPr.u8DacPr        = DAC_YPBPR_PR;
     stIntf[1].enIntfType                            = MT_UNF_DISP_INTF_TYPE_HDMI;
     stIntf[1].unIntf.enHdmi                         = MT_UNF_HDMI_ID_0;
     ret = MT_UNF_DISP_AttachIntf(MT_UNF_DISPLAY1, &stIntf[0], 2);
     if (ret != MT_SUCCESS)
     {
             printf("call MT_UNF_DISP_AttachIntf failed, Ret=%#x.\n", ret);
             MT_UNF_DISP_DeInit();
             return ret;
     }

     /* set display0 interface */
     stIntf[0].enIntfType                    = MT_UNF_DISP_INTF_TYPE_CVBS;
     stIntf[0].unIntf.stCVBS.u8Dac   = DAC_CVBS;
     ret = MT_UNF_DISP_AttachIntf(MT_UNF_DISPLAY0, &stIntf[0], 1);
     if (ret != MT_SUCCESS)
     {
             printf("call MT_UNF_DISP_AttachIntf failed, Ret=%#x.\n", ret);
             MT_UNF_DISP_DeInit();
             return ret;
     }


     }

    #endif
    MT_UNF_DISP_Attach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
    if(MT_UNF_ENC_FMT_BUTT != hd_fm)
    {
      MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY1, hd_fm);
      /****sd format by hd format auto set*****/
      //MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY0, sd_fmt);
    }
    BgColor.u8Red = 0;
    BgColor.u8Green = 0;
    BgColor.u8Blue = 0;
    MT_UNF_DISP_SetBgColor(MT_UNF_DISPLAY1, &BgColor);

   /**disp ppmode vivid mode action must be set when MT_UNF_VO_CreateWindow create window after*****/
   #if 0 /**performance 500ms***/
   /**disp set hdr mode*****/
   mt_unf_set_display_pp_hdr_mode(mt_unf_trans_hdr_mode(hdr_mode),ppMode);
   #else
   /**it will be set when DRV_AVPlayer_Open,only config it in here**/
   mt_unf_config_display_pp_hdr_mode(mt_unf_trans_hdr_mode(hdr_mode),ppMode);
   #endif

    ret = MT_UNF_DISP_Open(MT_UNF_DISPLAY1);
    if (ret != MT_SUCCESS) {
	return MT_FAILURE;
    }

    ret = MT_UNF_DISP_Open(MT_UNF_DISPLAY0);
    if (ret != MT_SUCCESS) {
	return MT_FAILURE;
    }

    OS_PRINTF("###cgf debug[%s].%d: init %d\n",__FUNCTION__,__LINE__, ret);
    return ret;
}
#endif
MT_UNF_DISP_PP_E  mt_unf_get_postProcessMode(void)
{
  return disp_ppMode;
}

MT_UNF_DISP_HDMI_MODE_E mt_unf_get_displayHdrMode(void)
{
  return disp_hdrMode;
}

void mt_unf_config_display_pp_hdr_mode(MT_UNF_DISP_HDMI_MODE_E hdrMode,MT_UNF_DISP_PP_E ppMode)
{
   disp_ppMode = ppMode;
   disp_hdrMode = hdrMode;
   OS_PRINTF("cgf debug[%s].%d:disp_ppMode:%d;disp_hdrMode:%d\n",
               __FUNCTION__,__LINE__,disp_ppMode,disp_hdrMode);
}

void mt_unf_set_display_pp_hdr_mode(MT_UNF_DISP_HDMI_MODE_E hdrMode,MT_UNF_DISP_PP_E ppMode)
{
  mt_s32 ret = MT_SUCCESS;
  MT_UNF_DISP_PP_E curr_pp = ppMode;
  disp_ppMode = ppMode;

  if(hdrMode == MT_UNF_DISP_HDMI_MODE_SDR)
  {
   ret = MT_UNF_DISP_SetPPMode(MT_UNF_DISPLAY1,curr_pp);
   if(curr_pp != MT_UNF_DISP_PP_DEFAULT)
    {
     ret = MT_UNF_DISP_SetCscEnable(MT_FALSE);
    }
  }
  else /****hdr must be set default(SetCscEnable)*****/
  {
   curr_pp = MT_UNF_DISP_PP_DEFAULT;
   ret = MT_UNF_DISP_SetPPMode(MT_UNF_DISPLAY1,curr_pp);
   ret |= MT_UNF_DISP_SetCscEnable(MT_TRUE);
  }
  ret |=MT_UNF_DISP_SetTvCapability(hdrMode);
  disp_hdrMode = hdrMode;

  OS_PRINTF("cgf debug[%s].%d:disp_ppMode:%d;curr_pp:%d;SetTvCapability:%d;ret:%d\n",
               __FUNCTION__,__LINE__,disp_ppMode,curr_pp,disp_hdrMode,ret);
}
/*  //don not need test HDR now
MT_UNF_DISP_HDMI_MODE_E mt_unf_trans_hdr_mode(u8 mode)
{
  mt_s32 ret = MT_SUCCESS;
  MT_UNF_DISP_HDMI_MODE_E drMode = MT_UNF_DISP_HDMI_MODE_SDR;
  switch(mode)
  {
   case DISP_HDMI_MODE_HDR10:
    drMode = MT_UNF_DISP_HDMI_MODE_HDR10;
   break;
   case DISP_HDMI_MODE_HLG:
    drMode = MT_UNF_DISP_HDMI_MODE_HLG;
   break;
   case DISP_HDMI_MODE_AUTO:
    drMode = MT_UNF_DISP_HDMI_MODE_AUTO;
   break;
   case DISP_HDMI_MODE_SDR:
   default:
   drMode = MT_UNF_DISP_HDMI_MODE_SDR;
   break;
  }
  return drMode;
}
*/
void mt_unf_set_display_hdr_mode(u8 mode)
{
    //don't need test HDR now
    //mt_unf_set_display_pp_hdr_mode(mt_unf_trans_hdr_mode(mode),disp_ppMode);
}

/**vivid pp mode action must be set after MT_UNF_VO_CreateWindow create window *****/
void mt_unf_set_postProcessMode(MT_UNF_DISP_PP_E ppMode)
{
    /***gchen:ppmode only for SDR display****/
    if(disp_hdrMode != MT_UNF_DISP_HDMI_MODE_SDR)
    {
     return;
    }
    if(disp_ppMode == ppMode)
    {
      return;
    }
    disp_ppMode = ppMode;
    /**vivid pp mode action must be set after MT_UNF_VO_CreateWindow create window *****/
    OS_PRINTF("###cgf debug[%s].%d: set pp mode:%d\n",__FUNCTION__,__LINE__, disp_ppMode);
    mt_unf_set_display_pp_hdr_mode(disp_hdrMode,ppMode);
}

void drv_disp_update_hdmi_fmt_ability(void)
{
  mt_s32 ret = MT_FAILURE;
  u8 i = 0;
  mt_unf_hdmi_edid_t hdmi_edid_info = {0};
  mt_u32 length = sizeof(mt_unf_hdmi_edid_t);

  memset(match_fmt,0,sizeof(match_fmt));
  match_fmt_count = 0;

  if(hdmiConnectFlag == 0)
  {
   OS_PRINTF("###cgf debug[%s].%d: HDMI is disconnect!!!@\n",__FUNCTION__,__LINE__);
   return;
  }

  ret = MT_UNF_HDMI_ReadEDID((mt_u8 *)&hdmi_edid_info, &length);
  if(MT_SUCCESS != ret)
  {
   OS_PRINTF("###cgf debug[%s].%d: Read HDMI edid fail\n",__FUNCTION__,__LINE__);
   return;
  }

  i = 0;

  if(hdmi_edid_info.supported_4096x2160p_60Hz & 0x01)
  {
    match_fmt[i++] = MT_UNF_ENC_FMT_4096X2160_60;
    OS_PRINTF("HDMI edid supported:4096X2160_60\n");
  }
  if(hdmi_edid_info.supported_4096x2160p_50Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_4096X2160_50;
   OS_PRINTF("HDMI edid supported:4096X2160_50\n");
  }
  if(hdmi_edid_info.supported_4096x2160p_30Hz & 0x01)
  {
    match_fmt[i++] = MT_UNF_ENC_FMT_4096X2160_30;
    OS_PRINTF("HDMI edid supported:4096X2160_30\n");
  }

  if(hdmi_edid_info.supported_4096x2160p_25Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_4096X2160_25;
   OS_PRINTF("HDMI edid supported:4096X2160_25\n");
  }


  if(hdmi_edid_info.supported_3840x2160p_60Hz & 0x01)
  {
    match_fmt[i++] = MT_UNF_ENC_FMT_3840X2160_60;
    OS_PRINTF("HDMI edid supported:3840X2160_60\n");
  }

  if(hdmi_edid_info.supported_3840x2160p_50Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_3840X2160_50;
   OS_PRINTF("HDMI edid supported:3840X2160_50\n");
  }
  if(hdmi_edid_info.supported_3840x2160p_30Hz & 0x01)
  {
    match_fmt[i++] = MT_UNF_ENC_FMT_3840X2160_30;
    OS_PRINTF("HDMI edid supported:3840X2160_30\n");
  }

  if(hdmi_edid_info.supported_3840x2160p_25Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_3840X2160_25;
   OS_PRINTF("HDMI edid supported:3840X2160_25\n");
  }


  if(hdmi_edid_info.supported_1080p_60Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_1080P_60;
   OS_PRINTF("HDMI edid supported:1080P_60\n");
  }
  if(hdmi_edid_info.supported_1080p_50Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_1080P_50;
   OS_PRINTF("HDMI edid supported:1080P_50\n");
  }

  if(hdmi_edid_info.supported_1080i_60Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_1080i_60;
   OS_PRINTF("HDMI edid supported:1080i_60\n");
  }
  if(hdmi_edid_info.supported_1080i_50Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_1080i_50;
   OS_PRINTF("HDMI edid supported:1080i_50\n");
  }

  if(hdmi_edid_info.supported_720p_60Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_720P_60;
   OS_PRINTF("HDMI edid supported:720P_60\n");
  }
  if(hdmi_edid_info.supported_720p_50Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_720P_50;
   OS_PRINTF("HDMI edid supported:720P_50\n");
  }

  if(hdmi_edid_info.supported_576p_50Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_576P_50;
   OS_PRINTF("HDMI edid supported:576P_50\n");
  }
  if(hdmi_edid_info.supported_576i_50Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_PAL;
   OS_PRINTF("HDMI edid supported:576i_50\n");
  }

  if(hdmi_edid_info.supported_720x480p_60Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_480P_60;
   OS_PRINTF("HDMI edid supported:480P_60\n");
  }
  if(hdmi_edid_info.supported_720x480i_60Hz & 0x01)
  {
   match_fmt[i++] = MT_UNF_ENC_FMT_NTSC;
   OS_PRINTF("HDMI edid supported:480i_60\n");
  }

  match_fmt_count = i;
}

void drv_disp_reset_hdmi_fmt_ability(void)
{
  memset(match_fmt,0,sizeof(match_fmt));
  match_fmt_count = 0;
}


u32 drv_disp_get_auto_hdmi_fmt(void)
{
  u8 i = 0,j = 0;
  u8 proi_fmt_cnt = sizeof(disp_fmt_priority)/sizeof(u32);
  u32 priorityFmt = MT_UNF_ENC_FMT_1080i_50;

  if(match_fmt_count == 0) /**retry hdmi edid format ability****/
  {
   drv_disp_update_hdmi_fmt_ability();
  }

  if(match_fmt_count > 0)
  {
    for(i = 0;i < proi_fmt_cnt;i ++)
    {
      priorityFmt = disp_fmt_priority[i];
      for(j = 0;j < match_fmt_count;j ++ )
      {
       if(priorityFmt == match_fmt[j])
        {
          OS_PRINTF("###cgf debug[%s].%d: auto high priory format:%d\n",__FUNCTION__,__LINE__,priorityFmt);
          return priorityFmt;
        }
      }
    }
  }
  /***return default fmt******/
  OS_PRINTF("###cgf debug[%s].%d: auto default format:%d\n",__FUNCTION__,__LINE__,DISPLAY_DEFAULT_MATCH_FMT);
  return DISPLAY_DEFAULT_MATCH_FMT;
}


void drv_disp_set_sl_hdr(u32 trans_mode,u32 brightness,u32 tun_level)
{
   OS_PRINTF("###cgf debug[%s].%d: @MT_UNF_DISP_Set_Sl_Hdr  transparent_mode:%d;brightness:%d;level:%d\n",__FUNCTION__,__LINE__,trans_mode,brightness,tun_level);
   MT_UNF_DISP_Set_Sl_Hdr(MT_UNF_DISPLAY1,trans_mode,brightness, brightness,tun_level, 0);
}

static MT_UNF_HDMI_ID_E mt_hdmi_getHdmi(void)
{
	return gstHdmiArgs.enHdmi;
}

static void mt_hdmi_getDefaultHdmiAttr(MT_UNF_HDMI_ATTR_S *pstAttr)
{
	memcpy(pstAttr, &(gHdmiAttr), sizeof(MT_UNF_HDMI_ATTR_S));
}

static void mt_hdmi_updateDefaultHdmiAttr(MT_UNF_HDMI_ATTR_S *pstAttr)
{
	memcpy(&(gHdmiAttr), pstAttr, sizeof(MT_UNF_HDMI_ATTR_S));
}

static void saveHdmiAttr(MT_UNF_HDMI_EVENT_TYPE_E event, mt_void *pPrivateData)
{
	struct HDMI_ARGS_S *pArgs = (struct HDMI_ARGS_S *)pPrivateData;

	memcpy(&gHdmiAttr, &(pArgs->hdmiAttr), sizeof(MT_UNF_HDMI_ATTR_S));

	//dump attr
	DMSG("Current Hdmi Attr:");
	DMSG("bEnableHdmi:%d\nbEnableAudio:%d\nbEnableVideo:%d\nbEnableAudInfoFrame:%d\nbEnableAviInfoFrame:%d\nenVidOutMode:%d\nbHDCPEnable:%d",
		 gHdmiAttr.bEnableHdmi,
		 gHdmiAttr.bEnableAudio,
		 gHdmiAttr.bEnableVideo,
		 gHdmiAttr.bEnableAudInfoFrame,
		 gHdmiAttr.bEnableAviInfoFrame,
		 gHdmiAttr.enVidOutMode,
		 gHdmiAttr.bHDCPEnable);
}


mt_s32 mt_hdmi_init(MT_UNF_HDMI_ID_E enHDMIId, MT_UNF_ENC_FMT_E enWantFmt)
{
    mt_s32 Ret = MT_FAILURE;
    MT_UNF_HDMI_OPEN_PARA_S stOpenParam;
    MT_UNF_HDMI_DELAY_S stDelay;

	gstHdmiArgs.enHdmi = enHDMIId;

	Ret = MT_UNF_HDMI_Init();
	if (MT_SUCCESS != Ret) {
		DMSG("MT_UNF_HDMI_Init failed:%#x\n", Ret);
		return MT_FAILURE;
	}
#if 0
	Ret = mt_hdmi_setHdcpKey(enHDMIId);
	if (MT_SUCCESS != Ret) {
		DMSG("Set hdcp erro:%#x\n", Ret);
	}
#endif
	DMSG("=============HDCP Key is already loaed by TEE-OS==============");

    MT_UNF_HDMI_GetDelay(0, &stDelay);
    stDelay.bForceFmtDelay = MT_TRUE;
    stDelay.bForceMuteDelay = MT_TRUE;
    stDelay.u32FmtDelay = 500;
    stDelay.u32MuteDelay = 120;
    MT_UNF_HDMI_SetDelay(0, &stDelay);

	gpfnHdmiUserCallback = saveHdmiAttr;

    gstCallbackFunc.pfnHdmiEventCallback = HDMI_Event_Proc;
    gstCallbackFunc.pPrivateData = &gstHdmiArgs;

	Ret = MT_UNF_HDMI_RegCallbackFunc(enHDMIId, &gstCallbackFunc);
	if (Ret != MT_SUCCESS) {
		EMSG("hdmi reg failed:%#x\n", Ret);
		MT_UNF_HDMI_DeInit();
		return MT_FAILURE;
	}

	stOpenParam.enDefaultMode = g_default_mode; //MT_UNF_HDMI_FORCE_NULL;
	Ret = MT_UNF_HDMI_Open(enHDMIId, &stOpenParam);
	if (Ret != MT_SUCCESS) {
		EMSG("MT_UNF_HDMI_Open failed:%#x\n", Ret);
		MT_UNF_HDMI_DeInit();
		return MT_FAILURE;
	}

	mt_hdmi_enable(enHDMIId, gHDCPFlag);

    return MT_SUCCESS;
}

#if 0
unsigned int hdcp_key_len = 304;
static unsigned char g_hdcp_key_m2m[] = {
    0x00, 0x92, 0x17, 0x73, 0x4a, 0xbc, 0xad, 0x05, 0x7e, 0x29, 0x38, 0xd2,
    0xe4, 0x0b, 0xdd, 0x61, 0xda, 0x75, 0xda, 0x56, 0xca, 0x75, 0x4c, 0x29,
    0x89, 0x75, 0x7c, 0x8f, 0x71, 0x08, 0xca, 0x39, 0x2d, 0xbe, 0xc6, 0xb2,
    0xed, 0x6b, 0x21, 0x70, 0x67, 0xae, 0xb1, 0x3f, 0xd5, 0x11, 0xc9, 0x41,
    0xdd, 0x38, 0x8e, 0x32, 0xa0, 0xb7, 0x7d, 0x19, 0x3b, 0xf4, 0xbf, 0xca,
    0x93, 0x5a, 0x5d, 0xc1, 0x24, 0x54, 0x3b, 0xdd, 0xdb, 0x42, 0x2b, 0x79,
    0x7e, 0xbc, 0xa4, 0xd3, 0xe5, 0xf4, 0x68, 0xad, 0xe3, 0xdf, 0x7d, 0x47,
    0x3d, 0x5d, 0xb9, 0x91, 0xb9, 0x83, 0x96, 0x42, 0x08, 0x65, 0xd9, 0xc9,
    0x38, 0xdb, 0x43, 0xd0, 0x48, 0x3c, 0xfe, 0x99, 0x34, 0xf1, 0xce, 0xc2,
    0x82, 0x68, 0x7d, 0xed, 0x85, 0xee, 0xff, 0x4d, 0xfb, 0x8f, 0xc0, 0xbe,
    0xe4, 0x5e, 0x3d, 0x61, 0x1f, 0xb1, 0x15, 0xbe, 0x34, 0x1a, 0x69, 0xcd,
    0xe8, 0x34, 0xa6, 0x6f, 0x68, 0x06, 0x85, 0xee, 0xc2, 0x9d, 0xcb, 0xf6,
    0xa0, 0x11, 0x82, 0xdc, 0x00, 0x62, 0xb2, 0x70, 0x8b, 0x48, 0x58, 0x3a,
    0x23, 0xf6, 0x96, 0xdb, 0x13, 0xe3, 0x17, 0x63, 0x21, 0xea, 0xc9, 0xca,
    0x79, 0xd3, 0x9d, 0xf8, 0xbe, 0x61, 0x6e, 0xc3, 0x3e, 0x09, 0x59, 0x7e,
    0x03, 0xfa, 0x18, 0x7c, 0x98, 0xe5, 0xce, 0x6e, 0xd1, 0xc7, 0xd6, 0x95,
    0x06, 0x82, 0x9a, 0x64, 0x8c, 0xfb, 0x44, 0x97, 0x59, 0xb9, 0x2d, 0x71,
    0xa2, 0x0a, 0x8f, 0xb6, 0xed, 0x5f, 0xfe, 0x29, 0x8f, 0xab, 0x1a, 0x31,
    0xf1, 0xc7, 0x10, 0x3f, 0x96, 0x03, 0x84, 0x51, 0x2f, 0x06, 0x5f, 0x31,
    0x1a, 0x39, 0x09, 0x7c, 0x1c, 0x99, 0x8a, 0x82, 0x91, 0xf3, 0x59, 0x25,
    0xc0, 0x3e, 0xf7, 0x20, 0x19, 0x6e, 0x4a, 0x3a, 0xb4, 0xaa, 0xdf, 0x9c,
    0x64, 0x90, 0x36, 0x4b, 0xcc, 0xa2, 0x49, 0xb0, 0x34, 0x40, 0x10, 0x03,
    0x03, 0x16, 0xba, 0xb5, 0x2b, 0x20, 0x1b, 0xe0, 0xf9, 0x41, 0xac, 0xe8,
    0xf7, 0x38, 0x94, 0x77, 0x3b, 0x01, 0x07, 0x21, 0xf3, 0x0f, 0xef, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf9, 0xe9, 0x00, 0x34, 0xf9,
    0xdb, 0x27, 0xbd, 0x00
};

static mt_s32 mt_hdmi_setHdcpKey(MT_UNF_HDMI_ID_E enHDMIId)
{
    mt_s32 Ret = MT_FAILURE;
    MT_UNF_HDMI_LOAD_KEY_S stLoadKey = { 0 };

    stLoadKey.u32KeyLength = hdcp_key_len;
    stLoadKey.pu8InputEncryptedKey = (mt_u8 *)mt_mem_malloc(MT_ID_HDMI, hdcp_key_len);
    memcpy(stLoadKey.pu8InputEncryptedKey, g_hdcp_key_m2m, hdcp_key_len);

    Ret = MT_UNF_HDMI_LoadHDCPKey(enHDMIId, &stLoadKey);
    mt_mem_free(MT_ID_HDMI, stLoadKey.pu8InputEncryptedKey);

	DMSG("LoadHDCPKey, return:%x", Ret);

    return Ret;
}
#endif

static mt_s32 mt_hdmi_enable(MT_UNF_HDMI_ID_E enHDMIId, mt_u8 hdcp_enable)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_HDMI_ATTR_S stHdmiAttr;
    MT_UNF_EDID_BASE_INFO_S stSinkCap;

	ret = MT_UNF_HDMI_GetAttr(enHDMIId, &stHdmiAttr);
	if (MT_SUCCESS != ret) {
		EMSG(" MT_UNF_HDMI_GetAttr failed!\n");
		return MT_FAILURE;
	}

	ret = MT_UNF_HDMI_GetSinkCapability(enHDMIId, &stSinkCap);
	if (MT_SUCCESS != ret) {
		EMSG(" MT_UNF_HDMI_GetSinkCapability failed, Err_Code=0x%x!\n", ret);
	}

    if (ret == MT_SUCCESS) {
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
        stHdmiAttr.bEnableHdmi = MT_TRUE;
    }

    if (MT_TRUE == stHdmiAttr.bEnableHdmi) {
        stHdmiAttr.bEnableAudio = MT_TRUE;
        stHdmiAttr.bEnableVideo = MT_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = MT_TRUE;
        stHdmiAttr.bEnableAviInfoFrame = MT_TRUE;
        stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_BUTT;// MT_UNF_HDMI_VIDEO_MODE_BUTT will auto output
    } else {
        stHdmiAttr.bEnableAudio = MT_FALSE;
        stHdmiAttr.bEnableVideo = MT_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = MT_FALSE;
        stHdmiAttr.bEnableAviInfoFrame = MT_FALSE;
        stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
    }

	stHdmiAttr.bHDCPEnable = hdcp_enable;

	DMSG("bHDCPEnable=%d\n", stHdmiAttr.bHDCPEnable);
    ret = MT_UNF_HDMI_SetAttr(enHDMIId, &stHdmiAttr);
    if (MT_SUCCESS != ret) {
        EMSG("MT_UNF_HDMI_SetAttr failed!\n");
        return MT_FAILURE;
    }

#ifdef CEC_FLAG
    MT_UNF_HDMI_CEC_Enable(MT_UNF_HDMI_ID_0);
    MT_UNF_HDMI_RegCECCallBackFunc(MT_UNF_HDMI_ID_0, HDMI_CEC_Proc);
#endif

    //must start hdmi
    MT_UNF_HDMI_Start(MT_UNF_HDMI_ID_0);

    mt_hdmi_updateDefaultHdmiAttr(&stHdmiAttr);

    return MT_SUCCESS;
}

static mt_void HDMI_Event_Proc(MT_UNF_HDMI_EVENT_TYPE_E event, mt_void *pPrivateData)
{
	switch (event) {
	case MT_UNF_HDMI_EVENT_HOTPLUG:
		HDMI_HotPlug_Proc(pPrivateData);
		break;
	case MT_UNF_HDMI_EVENT_NO_PLUG:
		HDMI_UnPlug_Proc(pPrivateData);
		break;
	case MT_UNF_HDMI_EVENT_EDID_FAIL:
		DMSG("------------------EDID-FAIL-----------------------");
		break;
	case MT_UNF_HDMI_EVENT_HDCP_FAIL:
		DMSG("------------------HDCP-FAIL-----------------------");
		break;
	case MT_UNF_HDMI_EVENT_HDCP_SUCCESS:
		DMSG("HDCP-SUCCESS");
		DMSG("------------------HDCP-SUCCESS-----------------------");
              hdmiConnectFlag = 0xFF;
		break;
	case MT_UNF_HDMI_EVENT_RSEN_CONNECT:
		DMSG("------------------RSEN CONNECT-----------------------");
		break;
	case MT_UNF_HDMI_EVENT_RSEN_DISCONNECT:
		DMSG("------------------RSEN DISCONNECT-----------------------");
		break;
	default:
		break;
	}
	/* Private Usage */
	if (gpfnHdmiUserCallback != NULL) {
		gpfnHdmiUserCallback(event, pPrivateData);
	}
}

extern void mtTestUpdateDisplayResolution(MT_UNF_ENC_FMT_E encFormat);

static void HDMI_Start_Proc(mt_void *pPrivateData)
{
	mt_s32 ret = MT_SUCCESS;
	struct HDMI_ARGS_S *pArgs = (struct HDMI_ARGS_S *)pPrivateData;
	MT_UNF_HDMI_ID_E hHdmi = pArgs->enHdmi;
	MT_UNF_HDMI_ATTR_S stHdmiAttr;
	//MT_UNF_HDMI_INFOFRAME_S        stInfoFrame;
	MT_UNF_EDID_BASE_INFO_S stSinkCap;
	MT_UNF_HDMI_STATUS_S stHdmiStatus;


	static mt_u8 u8FirstTimeSetting = MT_TRUE;


	MT_UNF_HDMI_GetStatus(hHdmi, &stHdmiStatus);
	DMSG("%s : %s : %d  bConnected=%x \n", __FILE__, __FUNCTION__, __LINE__, stHdmiStatus.bConnected);
	if (MT_FALSE == stHdmiStatus.bConnected) {
		DMSG("No Connect\n");
            hdmiConnectFlag = 0;
		return;
	}
    hdmiConnectFlag = 0xFF;

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
		if (g_default_mode != MT_UNF_HDMI_DEFAULT_ACTION_DVI)
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
	} else {
		stHdmiAttr.bEnableAudio = MT_FALSE;
		stHdmiAttr.bEnableVideo = MT_TRUE;
		stHdmiAttr.bEnableAudInfoFrame = MT_FALSE;
		stHdmiAttr.bEnableAviInfoFrame = MT_FALSE;
		stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
	}

#if 0
	if (u8FirstTimeSetting == MT_TRUE) {
		u8FirstTimeSetting = MT_FALSE;
             #if 1 // set to 0 for test manul HDCP case: 7. [M](1039)ISecStreamSession Decryption With Clear Text Key HDCP Dynamic changes Test (Software Mode)
		if (gHDCPFlag == MT_TRUE) {
			stHdmiAttr.bHDCPEnable = MT_TRUE; //Enable HDCP
		} else {
			stHdmiAttr.bHDCPEnable = MT_FALSE;
		}
              #else
                stHdmiAttr.bHDCPEnable = MT_FALSE;

             #endif
	} else {
		//HDCP Enable use default setting!!
	}
#endif
       stHdmiAttr.bHDCPEnable = gHDCPFlag; //Enable HDCP
       //MT_USLEEP(5000000);
    mt_sys_read_register(0xbf314400, &ret);
    DMSG("hdcp 0xbf314400 = 0x%x \n", ret);
    if (ret & 0x40000000) { //set hdcp disable or enable

        ret = (ret >> 8)  & 0xFF; //hdcp disable or enable
        DMSG("hdcp = 0x%x \n", ret);
        if (ret)
            stHdmiAttr.bHDCPEnable = MT_TRUE; //Enable HDCP
        else
            stHdmiAttr.bHDCPEnable = MT_FALSE; //disable HDCP
    }

	ret = MT_UNF_HDMI_SetAttr(hHdmi, &stHdmiAttr);

	/* MT_UNF_HDMI_SetAttr must before MT_UNF_HDMI_Start! */
	ret = MT_UNF_HDMI_Start(hHdmi);

	//HDMI_PrintAttr(&stHdmiAttr);
       //MT_UNF_HDMI_SetFormat(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_4096X2160_24);// why set to be UHD???
	DMSG("save current HdmiAttr to Spr. stHdmiAttr.bHDCPEnable = %d ", stHdmiAttr.bHDCPEnable);
	memcpy(&(pArgs->hdmiAttr), &stHdmiAttr, sizeof(MT_UNF_HDMI_ATTR_S));

    drv_disp_reset_hdmi_fmt_ability();
    testFormat = drv_disp_get_auto_hdmi_fmt();
    mtTestUpdateDisplayResolution(testFormat);
	return;
}



static void HDMI_HotPlug_Proc(mt_void *pPrivateData)
{
	HDMI_Start_Proc(pPrivateData);
       DMSG("\n --- HDMI_HotPlug_Proc. --- \n");

}

static mt_void HDMI_UnPlug_Proc(mt_void *pPrivateData)
{
    struct HDMI_ARGS_S *pArgs = (struct HDMI_ARGS_S *)pPrivateData;
    MT_UNF_HDMI_ID_E hHdmi = pArgs->enHdmi;

    DMSG("\n --- Get HDMI event: UnPlug. --- \n");
    hdmiConnectFlag = 0;
    drv_disp_reset_hdmi_fmt_ability();
    testFormat = MT_UNF_ENC_FMT_1080i_50;
    MT_UNF_HDMI_Stop(hHdmi);

}

MT_BOOL g_hdcp_onoff = MT_TRUE;



extern uint64_t MT_GetOPCStatus(void);

mt_s32 mt_hdmi_setHdcp(mt_u8 hdcp_level)
{
	mt_s32 ret = MT_SUCCESS;
	MT_UNF_HDMI_ATTR_S stAttr = {0};

	MT_UNF_HDMI_ID_E enHDMIId = mt_hdmi_getHdmi();

	//mt_hdmi_getDefaultHdmiAttr(&stAttr);

      MT_UNF_HDMI_GetAttr(enHDMIId, &stAttr);
      mt_hdmi_updateDefaultHdmiAttr(&stAttr);

	DMSG("in= HDCP bEnableHdmi: %d status:%d hdcp_level = %d  MT_GetOPCStatus = 0x%lx, g_hdcp_onoff = %d \n",stAttr.bEnableHdmi,  stAttr.bHDCPEnable, hdcp_level, MT_GetOPCStatus(), g_hdcp_onoff);

	switch(hdcp_level) {
	case 0: //hdcp off
		stAttr.bHDCPEnable = MT_FALSE; //Disable HDCP
		DMSG("Disable HDCP");
		break;
	case 1: //hdcp on, level:1.4
		stAttr.bHDCPEnable = MT_TRUE; //Enable HDCP
		DMSG("Enable HDCP1");
             mt_sys_read_register(0xbf314400, &ret);
             DMSG("hdcp 0xbf314400 = 0x%x \n", ret);
             if (ret & 0x40000000) { //set hdcp disable or enable, for test NOT SUPOORT HDCP TV

                ret = (ret >> 8)  & 0xFF; //hdcp disable or enable
                printf("hdcp = 0x%x \n", ret);
                if (ret)
                    stAttr.bHDCPEnable = MT_TRUE; //Enable HDCP
                else
                    stAttr.bHDCPEnable = MT_FALSE; //disable HDCP
            }
		break;
	case 2: //hdcp on, level:2.2
		stAttr.bHDCPEnable = MT_TRUE; //Enable HDCP, we only have 1.4 on S4
		DMSG("Enable HDCP2");
		break;
	default:
		EMSG("input level:%d is unknown!", hdcp_level);
		ret = MT_FAILURE;
		break;
	}

	if (ret == MT_SUCCESS) {
		if (stAttr.bEnableHdmi && (g_hdcp_onoff == stAttr.bHDCPEnable)) {
			return ret;
		}
             DMSG("\n");
		g_hdcp_onoff = stAttr.bHDCPEnable;
		ret = MT_UNF_HDMI_Stop(enHDMIId);
             //MT_USLEEP(500000);
		ret |= MT_UNF_HDMI_SetAttr(enHDMIId, &stAttr);
		ret |= MT_UNF_HDMI_Start(enHDMIId);
	}
       stAttr.bEnableHdmi = 1;
       //MT_USLEEP(3000000);
#ifdef _MT_WITH_TALTS_
        MT_USLEEP(1000000);
#else
        MT_USLEEP(100000);

#endif
	DMSG("\n return  HDCP status:%d hdcp_level = %d  MT_GetOPCStatus = %lx \n", stAttr.bHDCPEnable, hdcp_level, MT_GetOPCStatus());

	return ret;
}

