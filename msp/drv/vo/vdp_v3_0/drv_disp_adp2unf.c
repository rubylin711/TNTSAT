/******************************************************************************

  Copyright (C), 2017, Montage Tech. Co., Ltd.

 ******************************************************************************
  File Name     : drv_disp_adp2unf.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2017/01/20
  Description   :
  History       :
  1.Date        :
  Author        :
  Modification  : Created file

*******************************************************************************/
#include <linux/sizes.h>	/*SZ_1K*/

#include "mt_unf_disp.h"
#include "mt_drv_video.h"
#include "mt_drv_disp.h"
//#include "mpi_disp_tran.h"

//#include "mt_drv_pdm.h"//MT_DISP_PARAM_S
//#include "drv_pdm_ext.h"
#include "mt_drv_module.h"

#include "drv_display.h"
#include "drv_disp_com.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

mt_s32 DISP_PrintParam(MT_UNF_DISP_E enDisp, /*MT_DISP_PARAM_S */ mt_void *   pP)
{
#if 0
    mt_s32 i;

    for (i = 0; i < MT_UNF_DISP_INTF_TYPE_BUTT; i++)
    {
        //DISP_PRINT("INTF %d, type=%d : ", i, pP->stIntf[i].enIntfType);
        if (pP->stIntf[i].enIntfType == MT_UNF_DISP_INTF_TYPE_HDMI)
        {
            DISP_PRINT("HDMI ID=%d\n", pP->stIntf[i].enIntfType);
        }
        else if (pP->stIntf[i].enIntfType == MT_UNF_DISP_INTF_TYPE_YPBPR)
        {
            DISP_PRINT("Y=%d, Pb=%d, Pr=%d\n",
                       pP->stIntf[i].unIntf.stYPbPr.u8DacY,
                       pP->stIntf[i].unIntf.stYPbPr.u8DacPb,
                       pP->stIntf[i].unIntf.stYPbPr.u8DacPr);
        }
        else if (pP->stIntf[i].enIntfType == MT_UNF_DISP_INTF_TYPE_CVBS)
        {
            DISP_PRINT("CVBS=%d\n",
                       pP->stIntf[i].unIntf.stCVBS.u8Dac);
        }
        else
        {
            //DISP_PRINT("NULL\n");
        }

    }
#endif
    return MT_SUCCESS;
}



mt_s32 DISP_CheckParam(MT_UNF_DISP_E enDisp, /*MT_DISP_PARAM_S */ mt_void *  pP)
{
#if 0
    mt_s32 i;

    //DISP_PrintParam(enDisp, pP);

    if (enDisp == MT_UNF_DISPLAY1)
    {
        if (pP->enFormat > MT_UNF_ENC_FMT_BUTT)
        {
            DISP_ERROR("invalid enformt!\n");
            return MT_FAILURE;
        }

        for (i = 0; i < MT_UNF_DISP_INTF_TYPE_BUTT; i++)
        {
            if (pP->stIntf[i].enIntfType != MT_UNF_DISP_INTF_TYPE_BUTT)
            {
                if (pP->stIntf[i].enIntfType == MT_UNF_DISP_INTF_TYPE_YPBPR)
                {
                    if (   ( pP->stIntf[i].unIntf.stYPbPr.u8DacY > 3)
                           || ( pP->stIntf[i].unIntf.stYPbPr.u8DacPb > 3)
                           || ( pP->stIntf[i].unIntf.stYPbPr.u8DacPr > 3)
                           || ( pP->stIntf[i].unIntf.stYPbPr.u8DacY == pP->stIntf[i].unIntf.stYPbPr.u8DacPb)
                           || ( pP->stIntf[i].unIntf.stYPbPr.u8DacY == pP->stIntf[i].unIntf.stYPbPr.u8DacPr)
                           || ( pP->stIntf[i].unIntf.stYPbPr.u8DacPb == pP->stIntf[i].unIntf.stYPbPr.u8DacPr)
                       )
                    {
                        DISP_ERROR("invalid vadc id!\n");
                        return MT_FAILURE;
                    }
                }
            }
        }

    }
    else
    {
        //MT_UNF_DISPLAY1
         if (pP->enFormat > MT_UNF_ENC_FMT_BUTT)
        {
            DISP_ERROR("invalid enformt!\n");
            return MT_FAILURE;
        }

        for (i = 0; i < MT_UNF_DISP_INTF_TYPE_BUTT; i++)
        {
            if (pP->stIntf[i].enIntfType != MT_UNF_DISP_INTF_TYPE_BUTT)
            {
                if (pP->stIntf[i].enIntfType == MT_UNF_DISP_INTF_TYPE_CVBS)
                {
                    if ( pP->stIntf[i].unIntf.stCVBS.u8Dac > 3)
                    {
                        DISP_ERROR("invalid vadc id!\n");
                        return MT_FAILURE;
                    }
                }
            }
        }

    }

    if (  (pP->u32Brightness > 100 )
          || (pP->u32Contrast > 100)
          || (pP->u32Saturation > 100)
          || (pP->u32HuePlus > 100) )
    {
        DISP_ERROR("invalid color param!\n");
        return MT_FAILURE;
    }

    /*
        pP->bGammaEnable;
        pP->u32ScreenXpos;
        pP->u32ScreenYpos;
        pP->u32ScreenWidth;
        pP->u32ScreenHeight;
        pP->stBgColor;
    */
    if (pP->stAspectRatio.enDispAspectRatio > MT_UNF_DISP_ASPECT_RATIO_USER)
    {
        DISP_ERROR("invalid aspect ratio param!\n");
        return MT_FAILURE;
    }

    if (pP->stAspectRatio.enDispAspectRatio == MT_UNF_DISP_ASPECT_RATIO_USER)
    {
        if (  (pP->stAspectRatio.u32UserAspectWidth > (pP->stAspectRatio.u32UserAspectHeight * 16))
              || (pP->stAspectRatio.u32UserAspectHeight > (pP->stAspectRatio.u32UserAspectWidth * 16)))
        {
            DISP_ERROR("invalid aspect ratio param!\n");
            return MT_FAILURE;
        }
    }

    //pP->stDispTiming;
#endif
    return MT_SUCCESS;
}


mt_s32 MT_PDM_GetDispParamTEST(MT_UNF_DISP_E enDisp, /*MT_DISP_PARAM_S */ mt_void *  pP)
{
#if 0
    mt_s32 i;

    if (enDisp == MT_UNF_DISPLAY1)
    {
        pP->enFormat = MT_UNF_ENC_FMT_1080i_50;
        pP->u32Brightness = 50;
        pP->u32Contrast   = 50;
        pP->u32Saturation = 50;
        pP->u32HuePlus    = 50;
        pP->bGammaEnable  = MT_FALSE;
        pP->u32VirtScreenWidth  = 0;
        pP->u32VirtScreenHeight = 0;
        pP->stBgColor.u8Red   = 0;
        pP->stBgColor.u8Green = 0;
        pP->stBgColor.u8Blue  = 0;
        pP->stAspectRatio.enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_16TO9;

        pP->stIntf[0].enIntfType = MT_UNF_DISP_INTF_TYPE_YPBPR;
        pP->stIntf[0].unIntf.stYPbPr.u8DacY  = 0;
        pP->stIntf[0].unIntf.stYPbPr.u8DacPb = 1;
        pP->stIntf[0].unIntf.stYPbPr.u8DacPr = 3;

        pP->stIntf[1].enIntfType = MT_UNF_DISP_INTF_TYPE_HDMI;
        pP->stIntf[1].unIntf.enHdmi = MT_UNF_HDMI_ID_0;

        for (i = 1; i < MT_UNF_DISP_INTF_TYPE_BUTT; i++)
        {
            pP->stIntf[i].enIntfType = MT_UNF_DISP_INTF_TYPE_BUTT;
        }
        //pP->stDispTiming;
    }
    else
    {
        //MT_UNF_DISPLAY1
        pP->enFormat = MT_UNF_ENC_FMT_PAL;
        pP->u32Brightness = 50;
        pP->u32Contrast   = 50;
        pP->u32Saturation = 50;
        pP->u32HuePlus    = 50;
        pP->bGammaEnable  = MT_FALSE;
        pP->u32VirtScreenWidth  = 0;
        pP->u32VirtScreenHeight = 0;
        pP->stBgColor.u8Red   = 0;
        pP->stBgColor.u8Green = 0;
        pP->stBgColor.u8Blue  = 0;
        pP->stAspectRatio.enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_4TO3;

        pP->stIntf[0].enIntfType = MT_UNF_DISP_INTF_TYPE_CVBS;
        pP->stIntf[0].unIntf.stCVBS.u8Dac = 2;

        for (i = 1; i < MT_UNF_DISP_INTF_TYPE_BUTT; i++)
        {
            pP->stIntf[i].enIntfType = MT_UNF_DISP_INTF_TYPE_BUTT;
        }
    }
#endif
    return MT_SUCCESS;
}

#ifdef __DISP_PLATFORM_BOOT__
mt_s32 MT_UNF_DISP_Init(mt_void)
{
    mt_s32 nRet;
    // disp init
    nRet = DISP_Init();

    return nRet;
}

mt_s32 MT_UNF_DISP_DeInit(mt_void)
{
    //mt_s32 nRet;

    // disp deinit
    DISP_DeInit();
    return MT_SUCCESS;
}

mt_s32 MT_UNF_DISP_Open (MT_UNF_DISP_E enDisp)
{
    mt_s32 nRet;

    nRet = DISP_Open((MT_DRV_DISPLAY_E)(enDisp));

    return nRet;
}

mt_s32 MT_UNF_DISP_Close(MT_UNF_DISP_E enDisp)
{
    mt_s32 nRet;

    nRet =  DISP_Close((MT_DRV_DISPLAY_E)(enDisp));
    return nRet;
}
#endif

MT_DRV_DISP_FMT_E DISP_Disp0FmtRevise_Attach(MT_DRV_DISP_FMT_E U)
{
    if (U >= MT_DRV_DISP_FMT_PAL &&  U  <= MT_DRV_DISP_FMT_SECAM_H)
        return U;

    switch (U)
    {
        case MT_DRV_DISP_FMT_1080P_50:
        case MT_DRV_DISP_FMT_1080P_25:
        case MT_DRV_DISP_FMT_1080i_50:
        case MT_DRV_DISP_FMT_720P_50:
        case MT_DRV_DISP_FMT_576P_50:
        case MT_DRV_DISP_FMT_720P_50_FP:
        case MT_DRV_DISP_FMT_3840X2160_25:
        case MT_DRV_DISP_FMT_1440x576i_50:
            return MT_DRV_DISP_FMT_PAL;
        case MT_DRV_DISP_FMT_4096X2160_24:
        case MT_DRV_DISP_FMT_3840X2160_24:
        case MT_DRV_DISP_FMT_1080P_24:
        case MT_DRV_DISP_FMT_1080P_24_FP:
        case MT_DRV_DISP_FMT_3840X2160_30:
        case MT_DRV_DISP_FMT_1080P_60:
        case MT_DRV_DISP_FMT_1080P_30:
        case MT_DRV_DISP_FMT_1080i_60:
        case MT_DRV_DISP_FMT_720P_60:
        case MT_DRV_DISP_FMT_480P_60:
        case MT_DRV_DISP_FMT_1440x480i_60:
        case MT_DRV_DISP_FMT_720P_60_FP:
        default:
            return MT_DRV_DISP_FMT_NTSC;
    }
}

MT_DRV_DISP_FMT_E DISP_TVHDFmtU2V(MT_UNF_ENC_FMT_E U)
{
    if (U <= MT_UNF_ENC_FMT_480P_60)
    {
        return (MT_DRV_DISP_FMT_E)(MT_DRV_DISP_FMT_1080P_60 + (U - MT_UNF_ENC_FMT_1080P_60));
    }
    else
    {
        return MT_DRV_DISP_FMT_1080i_50;
    }
}



MT_DRV_DISP_FMT_E DISP_TVSDFmtU2V(MT_UNF_ENC_FMT_E U)
{
    switch (U)
    {
        case MT_UNF_ENC_FMT_PAL:
            return MT_DRV_DISP_FMT_PAL;
        case MT_UNF_ENC_FMT_PAL_N:
            return MT_DRV_DISP_FMT_PAL_N;
        case MT_UNF_ENC_FMT_PAL_Nc:
            return MT_DRV_DISP_FMT_PAL_Nc;
        case MT_UNF_ENC_FMT_NTSC:
            return MT_DRV_DISP_FMT_NTSC;
        case MT_UNF_ENC_FMT_NTSC_J:
            return MT_DRV_DISP_FMT_NTSC_J;
        case MT_UNF_ENC_FMT_NTSC_PAL_M:
            return MT_DRV_DISP_FMT_PAL_M;
        case MT_UNF_ENC_FMT_SECAM_SIN:
            return MT_DRV_DISP_FMT_SECAM_SIN;
        case MT_UNF_ENC_FMT_SECAM_COS:
            return MT_DRV_DISP_FMT_SECAM_COS;
        default:
            return MT_DRV_DISP_FMT_PAL;
    }
}

MT_DRV_DISP_FMT_E DISP_3DFmtU2V(MT_UNF_ENC_FMT_E U)
{
    if (U <= MT_UNF_ENC_FMT_720P_50_FRAME_PACKING)
    {
        return (MT_DRV_DISP_FMT_E)(MT_DRV_DISP_FMT_1080P_24_FP + (U - MT_UNF_ENC_FMT_1080P_24_FRAME_PACKING));
    }
    else
    {
        return MT_DRV_DISP_FMT_1080P_24_FP;
    }
}

MT_DRV_DISP_FMT_E DISP_4KFmtU2V(MT_UNF_ENC_FMT_E U)
{
    if (U <= MT_UNF_ENC_FMT_4096X2160_24)
    {
        return (MT_DRV_DISP_FMT_E)(MT_DRV_DISP_FMT_3840X2160_24 + (U - MT_UNF_ENC_FMT_3840X2160_24));
    }
    else
    {
        return MT_DRV_DISP_FMT_3840X2160_24;
    }
}


MT_DRV_DISP_FMT_E DISP_DVIFmtU2V(MT_UNF_ENC_FMT_E U)
{
    if (U <= MT_UNF_ENC_FMT_VESA_2048X1152_60)
    {
        return (MT_DRV_DISP_FMT_E)(MT_DRV_DISP_FMT_861D_640X480_60 + (U - MT_UNF_ENC_FMT_861D_640X480_60));
    }
    else
    {
        return MT_DRV_DISP_FMT_861D_640X480_60;
    }
}

MT_DRV_DISP_FMT_E DISP_GetEncFmt(MT_UNF_ENC_FMT_E enUnFmt)
{
    if (enUnFmt <= MT_UNF_ENC_FMT_480P_60)
    {
        return DISP_TVHDFmtU2V(enUnFmt);
    }
    else if (enUnFmt <= MT_UNF_ENC_FMT_SECAM_COS)
    {
        return DISP_TVSDFmtU2V(enUnFmt);
    }
    else if (enUnFmt <= MT_UNF_ENC_FMT_720P_50_FRAME_PACKING)
    {
        return DISP_3DFmtU2V(enUnFmt);
    }
    else if (enUnFmt <= MT_UNF_ENC_FMT_VESA_2560X1600_60_RB)
    {
        return DISP_DVIFmtU2V(enUnFmt);
    }
    else if (enUnFmt <= MT_UNF_ENC_FMT_4096X2160_24)
    {
        return DISP_4KFmtU2V(enUnFmt);
    }
    else if (enUnFmt == MT_UNF_ENC_FMT_BUTT)
    {
        return MT_DRV_DISP_FMT_CUSTOM;
        return MT_SUCCESS;
    }
    else
    {
        return MT_DRV_DISP_FMT_PAL;
    }
}

mt_u8 DISP_GetVdacIdFromPinIDForMPW(mt_u8 PinId)
{
    switch (PinId)
    {
        case 0:
            return (mt_u8)0;
        case 1:
            return (mt_u8)1;
        case 2:
            return (mt_u8)2;
        case 3:
            return (mt_u8)3;
        default:
            return (mt_u8)0xff;
    }
}


mt_s32 DISP_GetDrvIntf(MT_UNF_DISP_INTF_S* pU, MT_DRV_DISP_INTF_S* pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        /* set invalid value */
        pM->u8VDAC_Y_G  = MT_DISP_VDAC_INVALID_ID;
        pM->u8VDAC_Pb_B = MT_DISP_VDAC_INVALID_ID;
        pM->u8VDAC_Pr_R = MT_DISP_VDAC_INVALID_ID;

        switch (pU->enIntfType)
        {
            case MT_UNF_DISP_INTF_TYPE_HDMI:
                pM->eID = MT_DRV_DISP_INTF_HDMI0 + (pU->unIntf.enHdmi - MT_UNF_HDMI_ID_0);
                if (pM->eID > MT_DRV_DISP_INTF_HDMI2)
                {
                    return MT_FAILURE;
                }
                break;
            case MT_UNF_DISP_INTF_TYPE_YPBPR:
                pM->eID = MT_DRV_DISP_INTF_YPBPR0;
                pM->u8VDAC_Y_G  = DISP_GetVdacIdFromPinIDForMPW(pU->unIntf.stYPbPr.u8DacY);
                pM->u8VDAC_Pb_B = DISP_GetVdacIdFromPinIDForMPW(pU->unIntf.stYPbPr.u8DacPb);
                pM->u8VDAC_Pr_R = DISP_GetVdacIdFromPinIDForMPW(pU->unIntf.stYPbPr.u8DacPr);
                break;
            case MT_UNF_DISP_INTF_TYPE_SVIDEO:
                pM->eID = MT_DRV_DISP_INTF_SVIDEO0;
                pM->u8VDAC_Y_G  = DISP_GetVdacIdFromPinIDForMPW(pU->unIntf.stSVideo.u8DacY);
                pM->u8VDAC_Pb_B = DISP_GetVdacIdFromPinIDForMPW(pU->unIntf.stSVideo.u8DacC);
                break;
            case MT_UNF_DISP_INTF_TYPE_CVBS:
                pM->eID = MT_DRV_DISP_INTF_CVBS0;
                pM->u8VDAC_Y_G  = DISP_GetVdacIdFromPinIDForMPW(pU->unIntf.stCVBS.u8Dac);
                break;
            case MT_UNF_DISP_INTF_TYPE_VGA:
                pM->eID = MT_DRV_DISP_INTF_VGA0;
                pM->u8VDAC_Y_G  = DISP_GetVdacIdFromPinIDForMPW(pU->unIntf.stVGA.u8DacG);
                pM->u8VDAC_Pb_B = DISP_GetVdacIdFromPinIDForMPW(pU->unIntf.stVGA.u8DacB);
                pM->u8VDAC_Pr_R = DISP_GetVdacIdFromPinIDForMPW(pU->unIntf.stVGA.u8DacR);
                break;
            case MT_UNF_DISP_INTF_TYPE_RGB:
                pM->eID = MT_DRV_DISP_INTF_RGB0;
                pM->u8VDAC_Y_G  = DISP_GetVdacIdFromPinIDForMPW(pU->unIntf.stRGB.u8DacG);
                pM->u8VDAC_Pb_B = DISP_GetVdacIdFromPinIDForMPW(pU->unIntf.stRGB.u8DacB);
                pM->u8VDAC_Pr_R = DISP_GetVdacIdFromPinIDForMPW(pU->unIntf.stRGB.u8DacR);
                pM->bDacSync= DISP_GetVdacIdFromPinIDForMPW(pU->unIntf.stRGB.bDacSync);
                break;
            case MT_UNF_DISP_INTF_TYPE_LCD:
                pM->eID = MT_DRV_DISP_INTF_LCD0 + (pU->unIntf.enLcd - MT_UNF_DISP_LCD_0);
                if (pM->eID > MT_DRV_DISP_INTF_LCD2)
                {
                    return MT_FAILURE;
                }
                break;
            case MT_UNF_DISP_INTF_TYPE_BT1120:
                pM->eID = MT_DRV_DISP_INTF_BT1120_0 + (pU->unIntf.enBT1120 - MT_UNF_DISP_BT1120_0);
                if (pM->eID > MT_DRV_DISP_INTF_BT1120_2)
                {
                    return MT_FAILURE;
                }
                break;
            /*
            case MT_UNF_DISP_INTF_TYPE_LCD:
                pM->eID =  + (pU->unIntf.enLCD - );
                if (pM->eID > )
                {
                    return MT_FAILURE;
                }
                break;
            case MT_UNF_DISP_INTF_TYPE_BT1120:
                pM->eID =  + (pU->unIntf.enHDMI - );
                if (pM->eID > )
                {
                    return MT_FAILURE;
                }
                break;
            case MT_UNF_DISP_INTF_TYPE_BT656:
                pM->eID =  + (pU->unIntf.enHDMI - );
                if (pM->eID > )
                {
                    return MT_FAILURE;
                }
                break;
            case MT_UNF_DISP_INTF_TYPE_RGB:
                pM->eID =  + (pU->unIntf.enHDMI - );
                if (pM->eID > )
                {
                    return MT_FAILURE;
                }
                break;
            */
            default:
                return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

mt_s32 ParaTransfer_Timing(MT_UNF_DISP_TIMING_S *pU, MT_DRV_DISP_TIMING_S *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        pM->u32VFB = pU->VFB;
        pM->u32VBB = pU->VBB;
        pM->u32VACT = pU->VACT;

        pM->u32HFB = pU->HFB;
        pM->u32HBB = pU->HBB;
        pM->u32HACT = pU->HACT;

        pM->u32VPW = pU->VPW;
        pM->u32HPW = pU->HPW;
        pM->bIDV = pU->IDV;
        pM->bIHS = pU->IHS;
        pM->bIVS = pU->IVS;

        pM->bClkReversal = pU->ClockReversal;
        pM->u32DataWidth = (MT_DRV_DISP_INTF_DATA_WIDTH_E)pU->DataWidth;
        pM->u32ClkPara0 = pU->ClkPara0;
        pM->u32ClkPara1 = pU->ClkPara1;

        pM->bDitherEnable = pU->DitherEnable;
        pM->bInterlace = pU->bInterlace;

        pM->u32PixFreq = pU->PixFreq;
        pM->u32VertFreq = pU->VertFreq;
        pM->u32AspectRatioW = pU->AspectRatioW;
        pM->u32AspectRatioH = pU->AspectRatioH;

        pM->u32bUseGamma = pU->bUseGamma;
        pM->u32Reserve0 = pU->Reserve0;
        pM->u32Reserve1 = pU->Reserve1;
    }
    else
    {
        pU->VFB = pM->u32VFB;
        pU->VBB = pM->u32VBB;
        pU->VACT = pM->u32VACT;

        pU->HFB = pM->u32HFB ;
        pU->HBB = pM->u32HBB;
        pU->HACT = pM->u32HACT;

        pU->VPW = pM->u32VPW;
        pU->HPW = pM->u32HPW;
        pU->IDV = pM->bIDV;
        pU->IHS = pM->bIHS;
        pU->IVS = pM->bIVS;

        pU->ClockReversal = pM->bClkReversal;
        pU->DataWidth = (MT_UNF_DISP_INTF_DATA_WIDTH_E)pM->u32DataWidth;
        pU->ClkPara0 = pM->u32ClkPara0;
        pU->ClkPara1 = pM->u32ClkPara1;


        pU->DitherEnable = pM->bDitherEnable;
        pU->bInterlace = pM->bInterlace;
        pU->PixFreq = pM->u32PixFreq ;
        pU->VertFreq = pM->u32VertFreq;
        pU->AspectRatioW = pM->u32AspectRatioW;
        pU->AspectRatioH = pM->u32AspectRatioH;

        pU->bUseGamma = pM->u32bUseGamma;
        pU->Reserve0 = pM->u32Reserve0;
        pU->Reserve1 = pM->u32Reserve1;
    }

    return MT_SUCCESS;
}


#define DISP_VERSION_HI3716CV200_MPW 0x20130417ul

mt_s32 DispGetInitParam(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INIT_PARAM_S* pstSetting)
{

    memset(pstSetting, 0, sizeof(MT_DRV_DISP_INIT_PARAM_S));
    if (enDisp > MT_DRV_DISPLAY_1)
    {
        return MT_FAILURE;
    }
    if (MT_DRV_DISPLAY_0 == enDisp)
    {
        pstSetting->bIsMaster = MT_FALSE;
        pstSetting->bIsSlave  = MT_FALSE;
        pstSetting->u32VirtScreenWidth  = 720;
        pstSetting->u32VirtScreenHeight = 576;
    }
    else if (MT_DRV_DISPLAY_1 == enDisp)
    {

        pstSetting->bIsMaster = MT_TRUE;
        pstSetting->bIsSlave  = MT_FALSE;
        pstSetting->u32VirtScreenWidth  = 1920;
        pstSetting->u32VirtScreenHeight = 1080;
        pstSetting->u32Brightness = 50;
        pstSetting->u32Contrast   = 50;
        pstSetting->u32Saturation = 50;
    }


#if 0
    MT_DISP_PARAM_S stDispParam0, stDispParam1, *pstDispParam;
    MT_DRV_DISP_FMT_E  enFormat = MT_DRV_DISP_FMT_BUTT;

    #ifndef __DISP_PLATFORM_BOOT__
    PDM_EXPORT_FUNC_S* pst_PDMFunc;
    #endif

    MT_UNF_DISP_E enUnfDisp;
    mt_s32 i, j, nRet;
    memset((void*)&stDispParam0, 0, sizeof(MT_DISP_PARAM_S));
    memset((void*)&stDispParam1, 0, sizeof(MT_DISP_PARAM_S));

    if (enDisp > MT_DRV_DISPLAY_1)
    {
        return MT_FAILURE;
    }
    else
    {
         #ifndef __DISP_PLATFORM_BOOT__
        // get PDM data
        nRet = MT_DRV_MODULE_GetFunction(MT_ID_PDM, (mt_void**)&pst_PDMFunc);
        if (nRet || !pst_PDMFunc->pfnPDM_GetDispParam)
        {
            DISP_ERROR("DISP_get PDM funt failed!");
            return MT_FAILURE;;
        }

        enUnfDisp = (MT_DRV_DISPLAY_0 == enDisp) ? MT_UNF_DISPLAY0 : MT_UNF_DISPLAY1;

        nRet  = pst_PDMFunc->pfnPDM_GetDispParam(MT_UNF_DISPLAY0, &stDispParam0);
        nRet |= pst_PDMFunc->pfnPDM_GetDispParam(MT_UNF_DISPLAY1, &stDispParam1);
        if (nRet)
        {
            DISP_WARN("Got no disp params, may not burned!\n");
            return MT_FAILURE;
        }

        nRet  = DISP_CheckParam(MT_UNF_DISPLAY0, &stDispParam0);
        nRet |= DISP_CheckParam(MT_UNF_DISPLAY1, &stDispParam1);
        if (nRet)
        {
            DISP_ERROR("DISP0 Param invalid!\n");
            return MT_FAILURE;
        }

        #else

        enUnfDisp = (MT_DRV_DISPLAY_0 == enDisp) ? MT_UNF_DISPLAY0 : MT_UNF_DISPLAY1;

        nRet = MT_DRV_PDM_GetDispParam(MT_UNF_DISPLAY0, &stDispParam0);
        nRet |= MT_DRV_PDM_GetDispParam(MT_UNF_DISPLAY1, &stDispParam1);
        if (nRet)
        {
            DISP_ERROR("DISP get param failed!\n");
            return MT_FAILURE;
        }

        nRet  = DISP_CheckParam(MT_UNF_DISPLAY0, &stDispParam0);
        nRet  |= DISP_CheckParam(MT_UNF_DISPLAY1, &stDispParam1);
        if (nRet)
        {
            DISP_INFO("DISP0 Param invalid!\n");
            return MT_FAILURE;
        }

        #endif
    }

    if (MT_DRV_DISPLAY_0 == enDisp)
    {
        pstDispParam = &stDispParam0;
    }
    else
    {
        pstDispParam = &stDispParam1;
    }

    pstSetting->u32Version = DISP_VERSION_HI3716CV200_MPW;

    pstSetting->enFormat   = DISP_GetEncFmt(pstDispParam->enFormat);


    if (MT_UNF_ENC_FMT_BUTT == pstDispParam->enFormat)
        ParaTransfer_Timing(&pstDispParam->stDispTiming,&pstSetting->stDispTiming,MT_TRUE);

    //printk("custom (%d,%d)   " ,pstSetting->stDispTiming.u32HACT,pstSetting->stDispTiming.u32VACT);

    if (MT_DRV_DISPLAY_1 == stDispParam0.enSrcDisp)
    {
        /*attach  mode*/

        //printk("pdm attach  mode!\n ");
        if (MT_DRV_DISPLAY_0 == enDisp)
        {
            enFormat = DISP_GetEncFmt(stDispParam1.enFormat);

#ifndef MT_DISP_DOUBLE_HD_SUPPORT
            pstSetting->enFormat = DISP_Disp0FmtRevise_Attach(enFormat);
#endif
            pstSetting->bIsMaster = MT_FALSE;
            pstSetting->bIsSlave  = MT_TRUE;
            pstSetting->enAttachedDisp = MT_DRV_DISPLAY_1;

            pstSetting->u32VirtScreenWidth  = stDispParam0.u32VirtScreenWidth;
            pstSetting->u32VirtScreenHeight = stDispParam0.u32VirtScreenHeight;
            pstSetting->stOffsetInfo        = *((MT_DRV_DISP_OFFSET_S*)&stDispParam0.stOffsetInfo);
        }

        if (MT_DRV_DISPLAY_1 == enDisp)
        {
            pstSetting->bIsMaster = MT_TRUE;
            pstSetting->bIsSlave  = MT_FALSE;
            pstSetting->enAttachedDisp = MT_DRV_DISPLAY_0;
            pstSetting->u32VirtScreenWidth  = stDispParam1.u32VirtScreenWidth;
            pstSetting->u32VirtScreenHeight = stDispParam1.u32VirtScreenHeight;
            pstSetting->stOffsetInfo        = *((MT_DRV_DISP_OFFSET_S*)&stDispParam1.stOffsetInfo);

        }

        pstSetting->u32Brightness = stDispParam1.u32Brightness;
        pstSetting->u32Contrast   = stDispParam1.u32Contrast;
        pstSetting->u32Saturation = stDispParam1.u32Saturation;
        pstSetting->u32HuePlus    = stDispParam1.u32HuePlus;
        pstSetting->bGammaEnable  = stDispParam1.bGammaEnable;
        pstSetting->stBgColor.u8Red     = stDispParam1.stBgColor.u8Red;
        pstSetting->stBgColor.u8Green   = stDispParam1.stBgColor.u8Green;
        pstSetting->stBgColor.u8Blue    = stDispParam1.stBgColor.u8Blue;
    }
    else
    {
        //printk("pdm no attach  mode!\n ");
        pstSetting->bIsMaster = MT_FALSE;
        pstSetting->bIsSlave  = MT_FALSE;
        pstSetting->enAttachedDisp = MT_DRV_DISPLAY_BUTT;

        pstSetting->u32Brightness = pstDispParam->u32Brightness;
        pstSetting->u32Contrast   = pstDispParam->u32Contrast;
        pstSetting->u32Saturation = pstDispParam->u32Saturation;
        pstSetting->u32HuePlus    = pstDispParam->u32HuePlus;
        pstSetting->bGammaEnable  = pstDispParam->bGammaEnable;
        pstSetting->u32VirtScreenWidth  = pstDispParam->u32VirtScreenWidth;
        pstSetting->u32VirtScreenHeight = pstDispParam->u32VirtScreenHeight;
        pstSetting->stOffsetInfo        = *((MT_DRV_DISP_OFFSET_S*)&pstDispParam->stOffsetInfo);
        pstSetting->stBgColor.u8Red     = pstDispParam->stBgColor.u8Red;
        pstSetting->stBgColor.u8Green   = pstDispParam->stBgColor.u8Green;
        pstSetting->stBgColor.u8Blue    = pstDispParam->stBgColor.u8Blue;
    }

    switch (pstDispParam->stAspectRatio.enDispAspectRatio)
    {
        case MT_UNF_DISP_ASPECT_RATIO_4TO3:
            pstSetting->bCustomRatio = MT_TRUE;
            pstSetting->u32CustomRatioWidth  = 4;
            pstSetting->u32CustomRatioHeight = 3;
            break;
        case MT_UNF_DISP_ASPECT_RATIO_16TO9:
            pstSetting->bCustomRatio = MT_TRUE;
            pstSetting->u32CustomRatioWidth  = 16;
            pstSetting->u32CustomRatioHeight = 9;
            break;
        case MT_UNF_DISP_ASPECT_RATIO_221TO1:
            pstSetting->bCustomRatio = MT_TRUE;
            pstSetting->u32CustomRatioWidth  = 221;
            pstSetting->u32CustomRatioHeight = 100;
            break;
        case MT_UNF_DISP_ASPECT_RATIO_USER:
            pstSetting->bCustomRatio = MT_TRUE;
            pstSetting->u32CustomRatioWidth  = pstDispParam->stAspectRatio.u32UserAspectWidth;
            pstSetting->u32CustomRatioHeight = pstDispParam->stAspectRatio.u32UserAspectHeight;
            break;
        case MT_UNF_DISP_ASPECT_RATIO_AUTO:
        default:
            pstSetting->bCustomRatio = MT_FALSE;
            pstSetting->u32CustomRatioWidth  = 0;
            pstSetting->u32CustomRatioHeight = 0;
            break;
    }

    for (i = 0, j = 0; i < MT_UNF_DISP_INTF_TYPE_BUTT; i++)
    {
        //printk(" i=%d,  intf type=%d\n", i, pstDispParam->stIntf[i].enIntfType);
        if (pstDispParam->stIntf[i].enIntfType < MT_UNF_DISP_INTF_TYPE_BUTT)
        {
            DISP_GetDrvIntf(&(pstDispParam->stIntf[i]), &pstSetting->stIntf[j], MT_TRUE);
            j++;
            //printk("================= i=%d,j=%d\n", i, j);
        }
    }

    for (; j < MT_DRV_DISP_INTF_ID_MAX; j++)
    {
        pstSetting->stIntf[j].eID = MT_DRV_DISP_INTF_ID_MAX;
    }


    //pstSetting->stDispTiming;
#endif
    return MT_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
