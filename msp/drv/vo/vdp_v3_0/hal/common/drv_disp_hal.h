
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_hal.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_DISP_HAL_H__
#define __DRV_DISP_HAL_H__

#include "mt_drv_disp.h"
#include "drv_disp_version.h"
//#include "vdp_driver.h"
//#include "hal_specific_config.h"
//#include "drv_disp_rwzb.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#define DISP_VDAC_MAX_NUMBER 6
#define DISP_VENC_SIGNAL_MAX_NUMBER 6
#define VDAC_STATE_INT 0xf000

/*talk with jijiagang, all the chip behind cv200,will use the same reg.*/
#define DISP_USED_COMMON_REGISTER      0xf8000094
#define DISP_FASTBOOTUP_FLAG       0xdeadbeaf

//vdac
#define DAC_C_CTRL  0x0
#define DAC_R_CTRL  0x1
#define DAC_G_CTRL  0x2
#define DAC_B_CTRL  0x3
typedef enum tagDISP_INTF_TYPE_E
{
    DISP_INTF_TYPE_VDAC  = 0,

    DISP_INTF_TYPE_BT656,
    DISP_INTF_TYPE_BT1120,
    DISP_INTF_TYPE_LCD,
    DISP_INTF_TYPE_HDMI,
    DISP_INTF_TYPE_HVENC,
    DISP_INTF_TYPE_SVENC,
    DISP_INTF_TYPE_MIRACAST,
    DISP_INTF_TYPE_BUTT
}DISP_INTF_TYPE_E;

typedef enum tagDISP_WBC_E
{
    DISP_WBC_00 = 0,
    DISP_WBC_BUTT
}DISP_WBC_E;

typedef enum tagDISP_PLL_SOURCE_E
{
    DISP_CLOCK_SOURCE_SD0 = 0,
    DISP_CLOCK_SOURCE_HD0  ,
    DISP_CLOCK_SOURCE_HD1,
    DISP_CLOCK_SOURCE_BUTT
}DISP_PLL_SOURCE_E;
typedef enum tagDISP_CHANNEL_E
{
    DISP_CHANNEL_DHD0 = 0,
    DISP_CHANNEL_DHD1  ,
    DISP_CHANNEL_BUTT
}DISP_CHANNEL_E;

typedef enum tagDISP_HDMI_DATA_E
{
    DISP_HDMI_DATA_YUV = 0,
    DISP_HDMI_DATA_RGB  ,
    DISP_HDMI_DATA_BUTT
}DISP_HDMI_DATA_E;
#define HDMI_MODE_YUV  0
#define HDMI_MODE_RGB  1
typedef struct tagDISP_INTF_S
{
    MT_BOOL bOpen;

    MT_BOOL bLinkVenc;
    int eVencId;

    MT_DRV_DISP_INTF_S stIf;

    // signal index is VDACID
    //MT_DRV_DISP_VDAC_SIGNAL_E eSignal[DISP_VENC_SIGNAL_MAX_NUMBER];
    DISP_CHANNEL_E enDispChan;
    DISP_PLL_SOURCE_E enDispPll;
    DISP_HDMI_DATA_E enHDMIDataType;
}DISP_INTF_S;

typedef struct tagDISP_HAL_ENCFMT_PARAM_S
{
    // display into
    MT_DRV_DISP_FMT_E eFmt;
    MT_DRV_DISP_STEREO_E eDispMode;
    MT_BOOL bInterlace;

    mt_rect_s stOrgRect;
    mt_rect_s stRefRect;
    MT_DRV_ASPECT_RATIO_S stAR;

    mt_u32    u32RefreshRate;  /* in 1/100 Hz */

    MT_DRV_COLOR_SPACE_E enColorSpace;

}DISP_HAL_ENCFMT_PARAM_S;

typedef struct tagDISP_FMT_CFG_S
{
 //   VDP_DISP_SYNCINFO_S stTiming;
    DISP_PLL_SOURCE_E enPllIndex;
    mt_u32 u32Pll[2];
    DISP_HAL_ENCFMT_PARAM_S stInfo;
}DISP_FMT_CFG_S;


typedef struct tagDISP_VTTHD_CFG_S
{
    mt_u32 uVt1thdFieldMode;  // 0, frame; 1, field;
    mt_u32 uVt1thd;
    mt_u32 uVt2thdFieldMode;  // 0, frame; 1, field;
    mt_u32 uVt2thd;
}DISP_VTTHD_CFG_S;


typedef struct tagDISP_CAPA_S
{
    MT_BOOL bSupport;
    MT_BOOL bWbc;
    MT_BOOL bHD;
}DISP_CAPA_S;


typedef struct tagDISP_INTF_SETTING_S
{
    MT_DRV_DISP_FMT_E eFmt;

    // signal index is VDACID
    MT_DRV_DISP_VDAC_SIGNAL_E eSignal[DISP_VENC_SIGNAL_MAX_NUMBER];

}DISP_INTF_SETTING_S;


#if 0
typedef struct tagDISP_VENC_SETTING_S
{
    MT_DRV_DISP_FMT_E eFmt;

    // signal index is VDACID
    MT_DRV_DISP_VDAC_SIGNAL_E eSignal[DISP_VENC_SIGNAL_MAX_NUMBER];

}DISP_VENC_SETTING_S;

typedef struct tagDISP_HDMI_SETTING_S
{
    MT_DRV_COLOR_SPACE_E eColorSpace;
    mt_u32 u32DataWidth;  /* 8/10/12 bits */

}DISP_HDMI_SETTING_S;


typedef struct tagDISP_VDAC_SETTING_S
{
    MT_DRV_DISP_VDAC_SIGNAL_E eSignal;

}DISP_VDAC_SETTING_S;
#endif


typedef struct tagDISP_LOCAL_INTF_S
{
    MT_BOOL bSupport;
    MT_BOOL bIdle;
    MT_DRV_DISPLAY_E enChan;
}DISP_LOCAL_INTF_S;

typedef struct tagDISP_LOCAL_VENC_S
{
    MT_BOOL bSupport;
    MT_BOOL bIdle;
}DISP_LOCAL_VENC_S;

typedef struct tagDISP_LOCAL_VDAC_S
{
    MT_BOOL bSupport;
    MT_BOOL bIdle;
}DISP_LOCAL_VDAC_S;

typedef struct tagDISP_LOCAL_WBC_S
{
    MT_BOOL bSupport;
    MT_BOOL bIdle;
    mt_u32 u32RefCnt;
}DISP_LOCAL_WBC_S;

typedef struct tagDISP_HAL_COLOR_S
{
    MT_DRV_COLOR_SPACE_E enInputCS;
    MT_DRV_COLOR_SPACE_E enOutputCS;

    mt_u32 u32Bright;      //bright adjust value,range[0,100],default setting 50;
    mt_u32 u32Contrst;     //contrast adjust value,range[0,100],default setting 50;
    mt_u32 u32Hue;         //hue adjust value,range[0,100],default setting 50;
    mt_u32 u32Satur;       //saturation adjust value,range[0,100],default setting 50;
    mt_u32 u32Kr;          //red component gain adjust value for color temperature adjust,range[0,100],default setting 50;
    mt_u32 u32Kg;          //green component gain adjust value for color temperature adjust,range[0,100],default setting 50;
    mt_u32 u32Kb;          //blue component gain adjust value for color temperature adjust,range[0,100],default setting 50;

    MT_BOOL bGammaEnable;
    MT_BOOL bUseCustGammaTable;
}DISP_HAL_COLOR_S;

typedef struct tagDISP_CH_CFG_S
{
    MT_BOOL bLinkVenc;
//    DISP_VENC_E enVenc;
    mt_u32 RESERVE;
}DISP_CH_CFG_S;

/**************************************PLL PARAMETRE BEGIN*********************************************/
#define FBDIV_I_MAX       2400
#define FBDIV_I_MIN       16

/*    pll_refdiv [17:12]  max=64*/
#define REFDIV_I_MAX       (1<<6)
#define REFDIV_I_MIN       1

/*KHz*/
/*800M~24000M*/
#define FOUTVCO_MAX       2400000
#define FOUTVCO_MIN       800000

#define PIX_DIFF_MAX       (3)

typedef struct tag_PLL_PARA_S
{
    mt_u32 u32FREF;
    mt_u32 u32REFDIV;
    mt_u32 u32FBDIV;
    mt_u32 u32FRAC;
    mt_u32 u32PSTDIV1;
    mt_u32 u32PSTDIV2;
    mt_u32 u32FOUTVCO;
    mt_u32 u32FOUTPOSTDIV;
    mt_u32 u32FOUT1PH0;
    mt_u32 u32REG1;
    mt_u32 u32REG2;

    /*set pix clk*/
    mt_u32 New_FOUT1PH0;
} PLL_PARA_S;

typedef struct tag_TEMP_PARA_S
{
    mt_u32 u32TmpFOUTVCO;
    mt_u32 u32TmpN;
    mt_s32 u32TmpM;
    mt_s32 u32TmpFBDIV;
} TEMP_PARA_S;

typedef struct tag_REG_PARA_S
{
    mt_s32 u32CalcPixFreq;
    mt_u32 u32ClkPara0;
    mt_u32 u32ClkPara1;
} REG_PARA_S;
/**************************************PLL PARAMETRE END**********************************************/


typedef struct tagDISP_INTF_OPERATION_S
{
    /* usual */
    /* reset vdp */
    mt_s32 (*PF_ResetHardware)(mt_void);
    mt_s32 (*PF_CloseClkResetModule)(mt_void);

    /* Display config */
    MT_BOOL (* PF_TestChnSupport)(MT_DRV_DISPLAY_E eChn);
    MT_BOOL (* PF_TestChnSupportHD)(MT_DRV_DISPLAY_E eChn);
    MT_BOOL (* PF_TestIntfSupport)(MT_DRV_DISPLAY_E eChn, MT_DRV_DISP_INTF_ID_E eIntf);
    MT_BOOL (* PF_TestChnSupportCast)(MT_DRV_DISPLAY_E eChn);
    MT_BOOL (* PF_TestChnEncFmt)(MT_DRV_DISPLAY_E eChn, MT_DRV_DISP_FMT_E eFmt);
    MT_BOOL (* PF_TestChnAttach)(MT_DRV_DISPLAY_E enM, MT_DRV_DISPLAY_E enS);

    mt_s32 (* PF_SetChnFmt)(MT_DRV_DISPLAY_E eChn, MT_DRV_DISP_FMT_E eFmt, MT_DRV_DISP_STEREO_E enStereo);
    mt_s32 (* PF_SetChnTiming)(MT_DRV_DISPLAY_E eChn, MT_DRV_DISP_TIMING_S *pstTiming);

    mt_s32 (* PF_SetChnPixFmt)(MT_DRV_DISPLAY_E eChn, MT_DRV_PIX_FORMAT_E ePix);
    mt_s32 (* PF_SetChnBgColor)(MT_DRV_DISPLAY_E eChn, MT_DRV_COLOR_SPACE_E enCS, MT_DRV_DISP_COLOR_S *pstBGC);
    mt_s32 (* PF_SetChnColor)(MT_DRV_DISPLAY_E eChn, DISP_HAL_COLOR_S *pstColor);

    mt_s32 (* PF_SetChnEnable)(MT_DRV_DISPLAY_E eChn, MT_BOOL bEnalbe);
    mt_s32 (* PF_GetChnEnable)(MT_DRV_DISPLAY_E eChn, MT_BOOL *pbEnalbe);
    mt_s32 (* PF_InitDacDetect)(MT_DRV_DISP_INTF_S *pstIf);
    mt_s32 (* PF_SetMSChnEnable)(MT_DRV_DISPLAY_E eChnM, MT_DRV_DISPLAY_E eChnS, mt_u32 u32DelayMs, MT_BOOL bEnalbe);
#if ((!defined(__DISP_PLATFORM_BOOT__)) && defined(DAC_TYPE_SYNOPSYS))
    mt_s32 (* PF_UpdateGamma)(GAMMA_CS_E  enGammaCsMode, PQ_GAMMA_RGB_MODE_S* pstPqGammaModeData);
    mt_s32 (* PF_SetGammaCtrl)(MT_DRV_DISPLAY_E eDisp,GAMMA_CS_E enGammaCsMode,GAMMA_MODE_E enGammaMode,MT_BOOL bEnable);
    mt_s32 (* PF_SetGammaRWZBCtrl)(MT_DRV_DISPLAY_E eDisp,mt_u32 ParaIndex);
#endif
    /* interrup */
    mt_s32 (* PF_SetIntEnable)(mt_u32 u32Int, MT_BOOL bEnable);
    mt_s32 (* PF_SetIntDisable)(mt_u32 u32Int);

    mt_s32 (* PF_GetIntSetting)(mt_u32 *pu32IntSetting);

    mt_s32 (* PF_GetMaskedIntState)(mt_u32 *pu32State);
    mt_s32 (* PF_GetUnmaskedIntState)(mt_u32 *pu32State);
    mt_s32 (* PF_CleanIntState)(mt_u32 u32State);
    mt_u32 (* FP_GetChnIntState)(MT_DRV_DISPLAY_E enDisp, mt_u32 u32IntState);
    mt_u32 (* FP_GetChnBottomFlag)(MT_DRV_DISPLAY_E enDisp,  MT_BOOL *pbBtm, mt_u32 *pu32Vcnt);

    /* intf manage */
    mt_s32 (* PF_AcquireIntf2)(MT_DRV_DISPLAY_E enDisp, DISP_INTF_S *pstIf);
    mt_s32 (* PF_ReleaseIntf2)(MT_DRV_DISPLAY_E enDisp, DISP_INTF_S *pstIf);
    mt_s32 (* PF_ResetIntfFmt2)(MT_DRV_DISPLAY_E enDisp, DISP_INTF_S *pstIntf, MT_DRV_DISP_FMT_E eFmt,MT_DRV_DISP_TIMING_S *pstCustomTimg);
    mt_s32 (* PF_SetIntfEnable2)(MT_DRV_DISPLAY_E enDisp, DISP_INTF_S *pstIntf, MT_BOOL bEnable);

    /* WBC manager */
    mt_s32 (* PF_AcquireWbcByChn)(MT_DRV_DISPLAY_E eChn, DISP_WBC_E *peWbc);
    mt_s32 (* PF_AcquireWbc)(DISP_WBC_E eWbc);
    mt_s32 (* PF_ReleaseWbc)(DISP_WBC_E eWbc);

    /* WBC */
    mt_s32 (*PF_SetWbcIORect)(DISP_WBC_E eWbc, MT_DISP_DISPLAY_INFO_S *pstDispInfo, mt_rect_s *in, mt_rect_s *out);
    mt_s32 (*PF_SetWbc3DInfo)(DISP_WBC_E eWbc, MT_DISP_DISPLAY_INFO_S *pstDispInfo, mt_rect_s *in);
    mt_s32 (*PF_SetWbcColorSpace)(DISP_WBC_E eWbc, MT_DRV_COLOR_SPACE_E eSrcCS, MT_DRV_COLOR_SPACE_E eDstCS);
    mt_s32 (*PF_SetWbcPixFmt)(DISP_WBC_E eWbc, MT_DRV_PIX_FORMAT_E eFmt);
    mt_s32 (*PF_SetWbcAddr)(DISP_WBC_E eWbc, MT_DRV_VID_FRAME_ADDR_S *pstAddr);
    mt_s32 (*PF_SetWbcEnable)(DISP_WBC_E eWbc, MT_BOOL bEnable);
    mt_s32 (*PF_SetWbcLowDlyEnable)(DISP_WBC_E eWbc, MT_BOOL bEnable);
    mt_s32 (*PF_SetWbcPartfnsLineNum)(DISP_WBC_E eWbc, mt_u32 u32LineNum);
    mt_s32 (*PF_SetWbcLineNumInterval)(DISP_WBC_E eWbc, mt_u32 u32Interval);
    mt_s32 (*PF_SetWbcLineAddr)(DISP_WBC_E eWbc, mt_u32 u32Addr);
    mt_s32 (*PF_UpdateWbc)(DISP_WBC_E eWbc);
    mt_s32 (*PF_DACIsr)(mt_u8 u8DAC);
    mt_s32 (*PF_SetDACDetEn)(MT_BOOL bDACDetEn);
    mt_s32 (*PF_GetDACAttr)(MT_DRV_VDAC_ATTR_S *pDACAttr);
    /*Zorder*/
    mt_s32 (*PF_CBM_MovTOP)(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_LAYER_E enLayer);
    mt_s32 (*PF_VDP_RegSave)(mt_void);
    mt_s32 (*PF_VDP_RegReStore)(mt_void);

    /* date */
    mt_s32 (*PF_DATE_SetCoef)(MT_DRV_DISPLAY_E enDisp, mt_s32 u32Index);
    mt_s32 (*PF_DATE_SetIRE)(MT_DRV_DISPLAY_E enDisp, mt_u32 u32IRE);

    /* O5 enable/disable DAC*/
    mt_s32 (*PF_SetAllDACEn)(MT_BOOL bDACEn);
}DISP_INTF_OPERATION_S;


//mt_s32 DISP_HAL_Init(mt_u32 u32Base);
mt_s32 DISP_HAL_DeInit(mt_void);

mt_s32 DISP_HAL_GetVersion(MT_DRV_DISP_VERSION_S *pstVersion);
mt_s32 DISP_HAL_GetOperation(DISP_INTF_OPERATION_S *pstFunction);
DISP_INTF_OPERATION_S *DISP_HAL_GetOperationPtr(mt_void);



mt_s32 DISP_HAL_GetEncFmtPara(MT_DRV_DISP_FMT_E eFmt, DISP_HAL_ENCFMT_PARAM_S *pstFmtPara);
mt_s32 DISP_HAL_GetFmtAspectRatio(MT_DRV_DISP_FMT_E eFmt, mt_u32 *pH, mt_u32 *pV);
mt_s32 DISP_HAL_GetFmtColorSpace(MT_DRV_DISP_FMT_E eFmt, MT_DRV_COLOR_SPACE_E  *penColorSpace);

mt_s32 DISP_DEBUG_PrintPtr(mt_void);

extern mt_u32 Disp_SetFastbootupFlag(mt_u32 u32Value);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /*  __DRV_DISP_CMP_H__  */











