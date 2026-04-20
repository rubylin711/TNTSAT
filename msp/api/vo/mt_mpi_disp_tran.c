/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_disp_tran.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      :
    Modification:

*********************************************************************************************/

#include "memory.h"
#include "mpi_disp_tran.h"
#include "mt_error_mpi.h"



#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

MT_DRV_DISP_FMT_E Transfer_TVHDFmtU2V(MT_UNF_ENC_FMT_E U);
MT_UNF_ENC_FMT_E Transfer_TVHDFmtV2U(MT_DRV_DISP_FMT_E V);
MT_DRV_DISP_FMT_E Transfer_TVSDFmtU2V(MT_UNF_ENC_FMT_E U);
MT_UNF_ENC_FMT_E Transfer_TVSDFmtV2U(MT_DRV_DISP_FMT_E V);
MT_DRV_DISP_FMT_E Transfer_3DFmtU2V(MT_UNF_ENC_FMT_E U);
MT_UNF_ENC_FMT_E Transfer_3DFmtV2U(MT_DRV_DISP_FMT_E V);
MT_UNF_ENC_FMT_E Transfer_4KFmtV2U(MT_DRV_DISP_FMT_E V);
MT_DRV_DISP_FMT_E Transfer_4KFmtU2V(MT_UNF_ENC_FMT_E U);
MT_DRV_DISP_FMT_E Transfer_DVIFmtU2V(MT_UNF_ENC_FMT_E U);
MT_UNF_ENC_FMT_E Transfer_DVIFmtV2U(MT_DRV_DISP_FMT_E V);
mt_s32 Transfer_FrameRate(mt_u32 pM, MT_UNF_VCODEC_FRMRATE_S *pU, MT_BOOL bu2m);
mt_u8 Transfer_GetVdacIdFromPinIDForMPW(mt_u8 PinId);

mt_s32 Transfer_DispID(MT_UNF_DISP_E *pU, MT_DRV_DISPLAY_E *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        *pM = (MT_DRV_DISPLAY_E)(*pU);
        return MT_SUCCESS;
    }
    else
    {
        *pU = (MT_UNF_DISP_E)(*pM);
        return MT_SUCCESS;
    }
}

mt_s32 Transfer_VbiCfg(MT_UNF_DISP_VBI_CFG_S *pU, MT_DRV_DISP_VBI_CFG_S *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        pM->eType = (MT_DRV_DISP_VBI_TYPE_E)pU->enType;
        pM->u32InBufferSize = pU->u32InBufferSize;
        pM->u32WorkBufferSize = pU->u32WorkBufferSize;

        return MT_SUCCESS;
    }
    else
    {
        pU->enType = (MT_UNF_DISP_VBI_TYPE_E)pM->eType;
        pU->u32InBufferSize = pM->u32InBufferSize;
        pU->u32WorkBufferSize = pM->u32WorkBufferSize;
        return MT_SUCCESS;
    }
}

mt_s32 Transfer_VbiData(MT_UNF_DISP_VBI_DATA_S *pU, MT_DRV_DISP_VBI_DATA_S *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        pM->eType = (MT_DRV_DISP_VBI_TYPE_E)pU->enType;
        pM->pu8DataAddr = pU->pu8DataAddr;
        pM->u32DataLen = pU->u32DataLen;

        return MT_SUCCESS;
    }
    else
    {
        pU->enType = (MT_UNF_DISP_VBI_TYPE_E)pM->eType;
        pU->pu8DataAddr = pM->pu8DataAddr;
        pU->u32DataLen = pM->u32DataLen;
        return MT_SUCCESS;
    }
}



mt_s32 Transfer_DispOffset(MT_UNF_DISP_OFFSET_S *pU, MT_DRV_DISP_OFFSET_S *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        *pM = *((MT_DRV_DISP_OFFSET_S* )pU);
        return MT_SUCCESS;
    }
    else
    {
        *pU = *((MT_UNF_DISP_OFFSET_S *)pM);
        return MT_SUCCESS;
    }
}



mt_s32 Transfer_Disp3DMode(MT_UNF_DISP_3D_E *pU, MT_DRV_DISP_STEREO_MODE_E *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        *pM = (MT_DRV_DISP_STEREO_MODE_E)(*pU);
        return MT_SUCCESS;
    }
    else
    {
        *pU = (MT_UNF_DISP_3D_E)(*pM);
        return MT_SUCCESS;
    }
}

mt_s32 Transfer_LayerID(MT_UNF_DISP_LAYER_E *pU, MT_DRV_DISP_LAYER_E *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        if (*pU < MT_UNF_DISP_LAYER_BUTT)
        {
            if  (*pU == MT_UNF_DISP_LAYER_VIDEO)
                *pM  = MT_DRV_DISP_LAYER_VIDEO;
            else
                *pM  = MT_DRV_DISP_LAYER_GFX;

            return MT_SUCCESS;
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else
    {
        if(*pM < MT_DRV_DISP_LAYER_BUTT)
        {
             if  (*pM == MT_DRV_DISP_LAYER_VIDEO)
                *pU  = MT_UNF_DISP_LAYER_VIDEO;
            else
                *pU  = MT_UNF_DISP_LAYER_GFX;
            return MT_SUCCESS;
        }
        else
        {
            return MT_FAILURE;
        }
    }

}

MT_DRV_DISP_FMT_E Transfer_TVHDFmtU2V(MT_UNF_ENC_FMT_E U)
{
    if (U <= MT_UNF_ENC_FMT_480P_60)
    {
        mt_u32 t;

        t = (mt_u32)U;
        t = t - (mt_u32)MT_UNF_ENC_FMT_1080P_60;
        t = t + (mt_u32)MT_DRV_DISP_FMT_1080P_60;
        return (MT_DRV_DISP_FMT_E)t;
    }
    else
    {
        return MT_DRV_DISP_FMT_1080i_50;
    }
}

MT_UNF_ENC_FMT_E Transfer_TVHDFmtV2U(MT_DRV_DISP_FMT_E V)
{
    if (V <= MT_DRV_DISP_FMT_480P_60)
    {
        mt_u32 t;

        t = (mt_u32)V;
        t = t - (mt_u32)MT_DRV_DISP_FMT_1080P_60;
        t = t + (mt_u32)MT_UNF_ENC_FMT_1080P_60;
        return (MT_UNF_ENC_FMT_E)t;
    }
    else
    {
        return MT_UNF_ENC_FMT_1080i_50;
    }
}

MT_DRV_DISP_FMT_E Transfer_TVSDFmtU2V(MT_UNF_ENC_FMT_E U)
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
        case MT_UNF_ENC_FMT_NTSC_443:
            return MT_DRV_DISP_FMT_NTSC_443;
        case MT_UNF_ENC_FMT_SECAM_SIN:
            return MT_DRV_DISP_FMT_SECAM_SIN;
        case MT_UNF_ENC_FMT_SECAM_COS:
            return MT_DRV_DISP_FMT_SECAM_COS;
        default:
            return MT_DRV_DISP_FMT_PAL;
    }
}

MT_UNF_ENC_FMT_E Transfer_TVSDFmtV2U(MT_DRV_DISP_FMT_E V)
{
    switch (V)
    {
        case MT_DRV_DISP_FMT_PAL:
        case MT_DRV_DISP_FMT_PAL_B:
        case MT_DRV_DISP_FMT_PAL_B1:
        case MT_DRV_DISP_FMT_PAL_D:
        case MT_DRV_DISP_FMT_PAL_D1:
        case MT_DRV_DISP_FMT_PAL_G:
        case MT_DRV_DISP_FMT_PAL_H:
        case MT_DRV_DISP_FMT_PAL_K:
        case MT_DRV_DISP_FMT_PAL_I:
        case MT_DRV_DISP_FMT_1440x576i_50:
            return MT_UNF_ENC_FMT_PAL;
        case MT_DRV_DISP_FMT_PAL_N:
            return MT_UNF_ENC_FMT_PAL_N;
        case MT_DRV_DISP_FMT_PAL_Nc:
            return MT_UNF_ENC_FMT_PAL_Nc;
         case MT_DRV_DISP_FMT_NTSC_443:
            return MT_UNF_ENC_FMT_NTSC_443;
        case MT_DRV_DISP_FMT_NTSC:
        case MT_DRV_DISP_FMT_PAL_60:
       
        case MT_DRV_DISP_FMT_1440x480i_60:
            return MT_UNF_ENC_FMT_NTSC;
        case MT_DRV_DISP_FMT_NTSC_J:
            return MT_UNF_ENC_FMT_NTSC_J;
        case MT_DRV_DISP_FMT_PAL_M:
            return MT_UNF_ENC_FMT_NTSC_PAL_M;

        case MT_DRV_DISP_FMT_SECAM_SIN:
        case MT_DRV_DISP_FMT_SECAM_L:
        case MT_DRV_DISP_FMT_SECAM_B:
        case MT_DRV_DISP_FMT_SECAM_G:
        case MT_DRV_DISP_FMT_SECAM_D:
        case MT_DRV_DISP_FMT_SECAM_K:
        case MT_DRV_DISP_FMT_SECAM_H:
            return MT_UNF_ENC_FMT_SECAM_SIN;
        case MT_DRV_DISP_FMT_SECAM_COS:
            return MT_UNF_ENC_FMT_SECAM_COS;
        default:
            return MT_UNF_ENC_FMT_PAL;
    }
}

MT_DRV_DISP_FMT_E Transfer_3DFmtU2V(MT_UNF_ENC_FMT_E U)
{
    if (U <= MT_UNF_ENC_FMT_720P_50_FRAME_PACKING)
    {
        mt_u32 t;

        t = (mt_u32)U;
        t = t - (mt_u32)MT_UNF_ENC_FMT_1080P_24_FRAME_PACKING;
        t = t + MT_DRV_DISP_FMT_1080P_24_FP;
        return (MT_DRV_DISP_FMT_E)t;
    }
    else
    {
        return MT_DRV_DISP_FMT_1080P_24_FP;
    }
}

MT_UNF_ENC_FMT_E Transfer_3DFmtV2U(MT_DRV_DISP_FMT_E V)
{
    if (V <= MT_DRV_DISP_FMT_720P_50_FP)
    {
        mt_u32 t;

        t = (mt_u32)V;
        t = t - (mt_u32)MT_DRV_DISP_FMT_1080P_24_FP;
        t = t + (mt_u32)MT_UNF_ENC_FMT_1080P_24_FRAME_PACKING;
        return (MT_UNF_ENC_FMT_E)t;
    }
    else
    {
        return MT_UNF_ENC_FMT_1080P_24_FRAME_PACKING;
    }
}

MT_UNF_ENC_FMT_E Transfer_4KFmtV2U(MT_DRV_DISP_FMT_E V)
{
    if (V <= MT_DRV_DISP_FMT_4096X2160_60)
    {
        mt_u32 t;

        t = (mt_u32)V;
        t = t - (mt_u32)MT_DRV_DISP_FMT_3840X2160_24;
        t = t + (mt_u32)MT_UNF_ENC_FMT_3840X2160_24;
        return (MT_UNF_ENC_FMT_E)t;
    }
    else
    {
        return MT_UNF_ENC_FMT_3840X2160_24;
    }
}

MT_DRV_DISP_FMT_E Transfer_4KFmtU2V(MT_UNF_ENC_FMT_E U)
{
    if (U <= MT_UNF_ENC_FMT_4096X2160_60)
    {
        mt_u32 t;

        t = (mt_u32)U;
        t = t - (mt_u32)MT_UNF_ENC_FMT_3840X2160_24;
        t = t + (mt_u32)MT_DRV_DISP_FMT_3840X2160_24;
        return (MT_DRV_DISP_FMT_E)t;
    }
    else
    {
        return MT_DRV_DISP_FMT_3840X2160_24;
    }
}


MT_DRV_DISP_FMT_E Transfer_DVIFmtU2V(MT_UNF_ENC_FMT_E U)
{
    if (U <= MT_UNF_ENC_FMT_VESA_2560X1600_60_RB)
    {
        mt_u32 t;

        t = (mt_u32)U;
        t = t - (mt_u32)MT_UNF_ENC_FMT_861D_640X480_60;
        t = t + (mt_u32)MT_DRV_DISP_FMT_861D_640X480_60;
        return (MT_DRV_DISP_FMT_E)t;
    }
    else
    {
        return MT_DRV_DISP_FMT_861D_640X480_60;
    }
}

MT_UNF_ENC_FMT_E Transfer_DVIFmtV2U(MT_DRV_DISP_FMT_E V)
{
    if (V <= MT_DRV_DISP_FMT_VESA_2560X1600_60_RB)
    {
        mt_u32 t;

        t = (mt_u32)V;
        t = t - (mt_u32)MT_DRV_DISP_FMT_861D_640X480_60;
        t = t + (mt_u32)MT_UNF_ENC_FMT_861D_640X480_60;

        return (MT_UNF_ENC_FMT_E)t;
    }
    else
    {
        return MT_UNF_ENC_FMT_861D_640X480_60;
    }
}

mt_s32 Transfer_SdEncPqPara(MT_UNF_SD_ENC_PQ_PARA_S **pU, DISP_SD_ENC_PQ_PARA_S **pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        *pM = *(DISP_SD_ENC_PQ_PARA_S**)(pU);
        return MT_SUCCESS;
    }
    else
    {
        *pU = *(MT_UNF_SD_ENC_PQ_PARA_S**)(pM);
        return MT_SUCCESS;
    }
}

mt_s32 Transfer_EncFmt(MT_UNF_ENC_FMT_E *pU, MT_DRV_DISP_FMT_E *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        if (*pU <= MT_UNF_ENC_FMT_480P_60)
        {
            *pM = Transfer_TVHDFmtU2V(*pU);
            return MT_SUCCESS;
        }
        else if (*pU <= MT_UNF_ENC_FMT_SECAM_COS)
        {
            *pM = Transfer_TVSDFmtU2V(*pU);
            return MT_SUCCESS;
        }
        else if (*pU <= MT_UNF_ENC_FMT_720P_50_FRAME_PACKING)
        {
            *pM = Transfer_3DFmtU2V(*pU);
            return MT_SUCCESS;
        }
        else if (*pU <= MT_UNF_ENC_FMT_VESA_2560X1600_60_RB)
        {
            *pM = Transfer_DVIFmtU2V(*pU);
            return MT_SUCCESS;
        }
        else if (*pU <= MT_UNF_ENC_FMT_4096X2160_60)
        {
            *pM = Transfer_4KFmtU2V(*pU);
            return MT_SUCCESS;
        }

        else if (*pU == MT_UNF_ENC_FMT_BUTT)
        {
            *pM = MT_DRV_DISP_FMT_CUSTOM;
            return MT_SUCCESS;
        }
        else
        {
            return MT_FAILURE;
        }

    }
    else
    {
        if (*pM <= MT_DRV_DISP_FMT_480P_60)
        {
            *pU = Transfer_TVHDFmtV2U(*pM);
            return MT_SUCCESS;
        }
        else if (*pM <= MT_DRV_DISP_FMT_1440x480i_60)
        {
            *pU = Transfer_TVSDFmtV2U(*pM);
            return MT_SUCCESS;
        }
        else if (*pM <= MT_DRV_DISP_FMT_720P_50_FP)
        {
            *pU = Transfer_3DFmtV2U(*pM);
            return MT_SUCCESS;
        }
        else if (*pM <= MT_DRV_DISP_FMT_VESA_2560X1600_60_RB)
        {
            *pU = Transfer_DVIFmtV2U(*pM);
            return MT_SUCCESS;
        }
        else if (*pM <= MT_DRV_DISP_FMT_4096X2160_60)
        {
            *pU = Transfer_4KFmtV2U(*pM);
            return MT_SUCCESS;
        }
        else if (*pM == MT_DRV_DISP_FMT_CUSTOM)
        {
            *pU = MT_UNF_ENC_FMT_BUTT;
            return MT_SUCCESS;
        }
        else
        {
            return MT_FAILURE;
        }
    }

//    return MT_SUCCESS;
}

mt_s32 Transfer_AspectRatio(MT_UNF_DISP_ASPECT_RATIO_S *pU, mt_u32 *pH, mt_u32 *pV, MT_BOOL bu2m)
{
    if (bu2m)
    {
        switch(pU->enDispAspectRatio)
        {
            case MT_UNF_DISP_ASPECT_RATIO_AUTO:
                *pH = 0;
                *pV = 0;
            break;
            case MT_UNF_DISP_ASPECT_RATIO_4TO3:
                *pH = 4;
                *pV = 3;
            break;
            case MT_UNF_DISP_ASPECT_RATIO_16TO9:
                *pH = 16;
                *pV = 9;
            break;
            case MT_UNF_DISP_ASPECT_RATIO_1TO1:
                *pH = 1;
                *pV = 1;
            break;
            case MT_UNF_DISP_ASPECT_RATIO_USER:
                *pH = pU->u32UserAspectWidth;
                *pV = pU->u32UserAspectHeight;
            break;
            default:
                *pH = 0;
                *pV = 0;
            break;
        }
    }
    else
    {
        if ( !(*pH) || !(*pV) )
        {
            pU->enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_AUTO;
        }
        else if ( (*pH == 4) && (*pV == 3) )
        {
            pU->enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_4TO3;
        }
        else if ( (*pH == 16) && (*pV == 9) )
        {
            pU->enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_16TO9;
        }
        else if ( (*pH == 1) && (*pV == 1) )
        {
            pU->enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_1TO1;
        }
        else
        {
            pU->enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_USER;
            pU->u32UserAspectWidth  = *pH;
            pU->u32UserAspectHeight = *pV;
        }
    }

    return MT_SUCCESS;
}

mt_s32 Transfer_Timing(MT_UNF_DISP_TIMING_S *pU, MT_DRV_DISP_TIMING_S *pM, MT_BOOL bu2m)
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

        if (pU->ItfFormat == MT_UNF_DISP_INTF_DATA_FMT_YUV422)
            pM->eDataFmt = MT_DRV_DISP_INTF_DATA_FMT_YUV422;
        else
            pM->eDataFmt = MT_DRV_DISP_INTF_DATA_FMT_RGB444;

        pM->bDitherEnable = pU->DitherEnable;
        pM->bInterlace= pU->bInterlace;
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
        pU->VFB= pM->u32VFB;
        pU->VBB = pM->u32VBB;
        pU->VACT= pM->u32VACT;

        pU->HFB = pM->u32HFB  ;
        pU->HBB = pM->u32HBB  ;
        pU->HACT = pM->u32HACT ;

        pU->VPW = pM->u32VPW;
        pU->HPW = pM->u32HPW ;
        pU->IDV = pM->bIDV ;
        pU->IHS = pM->bIHS ;
        pU->IVS = pM->bIVS ;

        pU->ClockReversal = pM->bClkReversal ;
        pU->DataWidth = (MT_UNF_DISP_INTF_DATA_WIDTH_E)pM->u32DataWidth;

        if (MT_DRV_DISP_INTF_DATA_FMT_YUV422 == pM->eDataFmt  )
            pU->ItfFormat = MT_UNF_DISP_INTF_DATA_FMT_YUV422 ;
        else
            pU->ItfFormat = MT_UNF_DISP_INTF_DATA_FMT_RGB444 ;

        pU->DitherEnable = pM->bDitherEnable ;
        pU->bInterlace = pM->bInterlace ;
        pU->PixFreq = pM->u32PixFreq  ;
        pU->VertFreq = pM->u32VertFreq ;
        pU->AspectRatioW = pM->u32AspectRatioW;
        pU->AspectRatioH = pM->u32AspectRatioH  ;

        pU->bUseGamma = pM->u32bUseGamma ;
         pU->Reserve0 = pM->u32Reserve0 ;
        pU->Reserve1 = pM->u32Reserve1  ;
    }

    return MT_SUCCESS;
}

mt_s32 Transfer_BGColor(MT_UNF_DISP_BG_COLOR_S *pU, MT_DRV_DISP_COLOR_S *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        pM->u8Blue  = pU->u8Blue;
        pM->u8Green = pU->u8Green;
        pM->u8Red   = pU->u8Red;
    }
    else
    {
        pU->u8Blue  = pM->u8Blue;
        pU->u8Green = pM->u8Green;
        pU->u8Red   = pM->u8Red;
    }

    return MT_SUCCESS;
}


mt_s32 Transfer_FrameRate(mt_u32 pM, MT_UNF_VCODEC_FRMRATE_S *pU, MT_BOOL bu2m)
{
    if (bu2m)
    {
        pM = pU->u32fpsInteger * 1000 + pU->u32fpsDecimal;
    }
    else
    {
        pU->u32fpsInteger = pM /1000;
        pU->u32fpsDecimal = pM - ((pM / 1000) * 1000);
    }

    return MT_SUCCESS;
}

mt_s32 Transfer_VideoFormat(MT_UNF_VIDEO_FORMAT_E  *pU, MT_DRV_PIX_FORMAT_E *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
       switch (*pU)
        {
            case MT_UNF_FORMAT_YUV_SEMIPLANAR_422:
            *pM = MT_DRV_PIX_FMT_NV61_2X1;
            break;
            case MT_UNF_FORMAT_YUV_SEMIPLANAR_420:
            *pM = MT_DRV_PIX_FMT_NV21;
            break;
            case MT_UNF_FORMAT_YUV_SEMIPLANAR_400:
            *pM = MT_DRV_PIX_FMT_NV80;
            break;

            case MT_UNF_FORMAT_YUV_SEMIPLANAR_411:
            *pM = MT_DRV_PIX_FMT_NV12_411;
            break;
            case MT_UNF_FORMAT_YUV_SEMIPLANAR_422_1X2:
            *pM = MT_DRV_PIX_FMT_NV61;
            break;
            case MT_UNF_FORMAT_YUV_SEMIPLANAR_444:
            *pM = MT_DRV_PIX_FMT_NV42;
            break;
            case MT_UNF_FORMAT_YUV_PACKAGE_UYVY:
            *pM = MT_DRV_PIX_FMT_UYVY;
            break;
            case MT_UNF_FORMAT_YUV_PACKAGE_YUYV:
            *pM = MT_DRV_PIX_FMT_YUYV;
            break;

            case MT_UNF_FORMAT_YUV_PACKAGE_YVYU:
            *pM = MT_DRV_PIX_FMT_YVYU;
            break;
            case MT_UNF_FORMAT_YUV_PLANAR_400:
            *pM = MT_DRV_PIX_FMT_YUV400;
            break;
            case MT_UNF_FORMAT_YUV_PLANAR_411:
            *pM = MT_DRV_PIX_FMT_YUV411;
            break;
            case MT_UNF_FORMAT_YUV_PLANAR_420:
            *pM = MT_DRV_PIX_FMT_YUV420p;
            break;

            case MT_UNF_FORMAT_YUV_PLANAR_422_1X2:
            *pM = MT_DRV_PIX_FMT_YUV422_1X2;
            break;
            case MT_UNF_FORMAT_YUV_PLANAR_422_2X1:
            *pM = MT_DRV_PIX_FMT_YUV422_2X1;
            break;
            case MT_UNF_FORMAT_YUV_PLANAR_444:
            *pM = MT_DRV_PIX_FMT_YUV_444;
            break;
            case MT_UNF_FORMAT_YUV_PLANAR_410:
            *pM = MT_DRV_PIX_FMT_YUV410p;
            break;
            default:
            return MT_FAILURE;
        }
    }
    else
    {
        switch (*pM)
        {
            case MT_DRV_PIX_FMT_NV61_2X1:
            *pU = MT_UNF_FORMAT_YUV_SEMIPLANAR_422;
            break;
            case MT_DRV_PIX_FMT_NV21:
            *pU = MT_UNF_FORMAT_YUV_SEMIPLANAR_420;
            break;
            case MT_DRV_PIX_FMT_NV80:
            *pU = MT_UNF_FORMAT_YUV_SEMIPLANAR_400;
            break;
            case MT_DRV_PIX_FMT_NV12_411:
            *pU = MT_UNF_FORMAT_YUV_SEMIPLANAR_411;
            break;
            case MT_DRV_PIX_FMT_NV61:
            *pU = MT_UNF_FORMAT_YUV_SEMIPLANAR_422_1X2;
            break;
            case MT_DRV_PIX_FMT_NV42:
            *pU = MT_UNF_FORMAT_YUV_SEMIPLANAR_444;
            break;
            case MT_DRV_PIX_FMT_UYVY:
            *pU = MT_UNF_FORMAT_YUV_PACKAGE_UYVY;
            break;
            case MT_DRV_PIX_FMT_YUYV:
            *pU = MT_UNF_FORMAT_YUV_PACKAGE_YUYV;
            break;
            case MT_DRV_PIX_FMT_YVYU:
            *pU = MT_UNF_FORMAT_YUV_PACKAGE_YVYU;
            break;

            case MT_DRV_PIX_FMT_YUV400:
            *pU = MT_UNF_FORMAT_YUV_PLANAR_400;
            break;
            case MT_DRV_PIX_FMT_YUV411:
            *pU = MT_UNF_FORMAT_YUV_PLANAR_411;
            break;
            case MT_DRV_PIX_FMT_YUV420p:
            *pU = MT_UNF_FORMAT_YUV_PLANAR_420;
            break;
            case MT_DRV_PIX_FMT_YUV422_1X2:
            *pU = MT_UNF_FORMAT_YUV_PLANAR_422_1X2;
            break;
            case MT_DRV_PIX_FMT_YUV422_2X1:
            *pU = MT_UNF_FORMAT_YUV_PLANAR_422_2X1;
            break;
            case MT_DRV_PIX_FMT_YUV_444:
            *pU = MT_UNF_FORMAT_YUV_PLANAR_444;
            break;
            case MT_DRV_PIX_FMT_YUV410p:
            *pU = MT_UNF_FORMAT_YUV_PLANAR_410;
            break;

            case MT_DRV_PIX_FMT_NV12:
            default:
            return MT_FAILURE;

        }
    }

    return MT_SUCCESS;
}

static MT_DRV_ASP_RAT_MODE_E s_DrvACModeTab[MT_UNF_VO_ASPECT_CVRS_BUTT]=
{
    MT_DRV_ASP_RAT_MODE_FULL,
    MT_DRV_ASP_RAT_MODE_LETTERBOX,
    MT_DRV_ASP_RAT_MODE_PANANDSCAN,
    MT_DRV_ASP_RAT_MODE_COMBINED,
    MT_DRV_ASP_RAT_MODE_FULL_H,
    MT_DRV_ASP_RAT_MODE_FULL_V,
};
static MT_UNF_VO_ASPECT_CVRS_E s_UnfACModeTab[MT_DRV_ASP_RAT_MODE_BUTT]=
{
    MT_UNF_VO_ASPECT_CVRS_IGNORE,
    MT_UNF_VO_ASPECT_CVRS_LETTERBOX,
    MT_UNF_VO_ASPECT_CVRS_PAN_SCAN,
    MT_UNF_VO_ASPECT_CVRS_COMBINED,
    MT_UNF_VO_ASPECT_CVRS_HORIZONTAL_FULL,
    MT_UNF_VO_ASPECT_CVRS_VERTICAL_FULL,
    MT_UNF_VO_ASPECT_CVRS_BUTT,
};

mt_s32 Transfe_ARConvert(MT_UNF_VO_ASPECT_CVRS_E  *pU, MT_DRV_ASP_RAT_MODE_E *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        if (*pU < MT_UNF_VO_ASPECT_CVRS_BUTT)
        {
            *pM = s_DrvACModeTab[*pU];
            return MT_SUCCESS;
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else
    {
        if (*pM < MT_DRV_ASP_RAT_MODE_BUTT)
        {
            *pU = s_UnfACModeTab[*pM];
            return MT_SUCCESS;
        }
        else
        {
            return MT_FAILURE;
        }
    }

    //return MT_FAILURE;
}

mt_s32 Transfe_ZOrder(MT_LAYER_ZORDER_ABS_E *pU, MT_DRV_DISP_ZORDER_ABS_E *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        *pM = (MT_DRV_DISP_ZORDER_ABS_E) *pU;
    }
    else
    {
        *pU = (MT_LAYER_ZORDER_ABS_E)*pM;
    }

    return MT_SUCCESS;
}
mt_s32 Transfe_SwitchMode(MT_UNF_WINDOW_FREEZE_MODE_E *pU, MT_DRV_WIN_SWITCH_E *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        *pM = (MT_DRV_WIN_SWITCH_E)(*pU);
    }
    else
    {
        *pU = (MT_UNF_WINDOW_FREEZE_MODE_E)(*pM);
    }

    return MT_SUCCESS;
}

mt_s32 Transfe_Rotate(MT_UNF_VO_ROTATION_E *pU, MT_DRV_ROT_ANGLE_E *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        if (*pU < MT_UNF_VO_ROTATION_BUTT)
        {
            *pM = (MT_DRV_ROT_ANGLE_E)*pU;
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else
    {
        if (*pM < MT_DRV_ROT_ANGLE_BUTT)
        {
            *pU = (MT_UNF_VO_ROTATION_E)*pM;
        }
        else
        {
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}


static MT_DRV_FRAME_TYPE_E s_DrvFrameTypeTab[MT_UNF_FRAME_PACKING_TYPE_BUTT] =
{MT_DRV_FT_NOT_STEREO, MT_DRV_FT_SBS, MT_DRV_FT_TAB, MT_DRV_FT_FPK};

static MT_UNF_VIDEO_FRAME_PACKING_TYPE_E s_UnfFrameTypeTab[MT_DRV_FT_BUTT] =
{
MT_UNF_FRAME_PACKING_TYPE_NONE,
MT_UNF_FRAME_PACKING_TYPE_SIDE_BY_SIDE,
MT_UNF_FRAME_PACKING_TYPE_TOP_AND_BOTTOM,
MT_UNF_FRAME_PACKING_TYPE_TIME_INTERLACED,
};

static MT_DRV_FIELD_MODE_E s_DrvFMTab[MT_UNF_VIDEO_FIELD_BUTT] =
{MT_DRV_FIELD_ALL, MT_DRV_FIELD_TOP, MT_DRV_FIELD_BOTTOM};

static MT_UNF_VIDEO_FIELD_MODE_E s_UnfFMTab[MT_DRV_FIELD_BUTT] =
{MT_UNF_VIDEO_FIELD_TOP,MT_UNF_VIDEO_FIELD_BOTTOM, MT_UNF_VIDEO_FIELD_ALL};

#define TRANSFER_A2B(a, b)   a = b
#define TRANSFER_A2B_R(a, b) b = a



mt_s32 Transfer_Frame(MT_UNF_VIDEO_FRAME_INFO_S  *pU, MT_DRV_VIDEO_FRAME_S *pM, MT_BOOL bu2m)
{
    MT_UNF_VIDEO_FIELD_MODE_E  enFieldtmp = MT_UNF_VIDEO_FIELD_BUTT;
    MT_DRV_FIELD_MODE_E        enDrvField = MT_DRV_FIELD_BUTT;
    MT_DRV_VIDEO_TB_ADJUST_E   enDrvTbAdjust = MT_DRV_VIDEO_TB_BUTT;

    if (bu2m)
    {
        if ((pU->enVideoFormat > MT_UNF_FORMAT_YUV_BUTT)
            || (pU->enFieldMode > enFieldtmp)
            || (pU->enFramePackingType > MT_UNF_FRAME_PACKING_TYPE_BUTT))
        {
            return MT_ERR_VO_INVALID_PARA;
        }

        memset(pM, 0, sizeof(MT_DRV_VIDEO_FRAME_S));
        if (pU->enFramePackingType != MT_UNF_FRAME_PACKING_TYPE_BUTT)
        {
            pM->eFrmType = s_DrvFrameTypeTab[pU->enFramePackingType];
        }
        else
        {
            pM->eFrmType = MT_DRV_FT_BUTT;
        }

        Transfer_VideoFormat(&pU->enVideoFormat, &pM->ePixFormat, MT_TRUE);

        TRANSFER_A2B(pM->bProgressive, pU->bProgressive);
        TRANSFER_A2B(pM->bTopFieldFirst, pU->bTopFieldFirst);
        TRANSFER_A2B(pM->u32Width    , pU->u32Width);
        TRANSFER_A2B(pM->u32Height   , pU->u32Height);
        TRANSFER_A2B(pM->u64Pts, pU->u64Pts);
        TRANSFER_A2B(pM->end_of_stream_flag, pU->end_of_stream_flag);
        //TRANSFER_A2B(pM->stDispRect.s32X      , pU->u32DisplayCenterX - (pU->u32DisplayWidth>>1) );
        pM->stDispRect.s32X = (mt_s32)(pU->u32DisplayCenterX - (pU->u32DisplayWidth>>1));
        //TRANSFER_A2B(pM->stDispRect.s32Y      , pU->u32DisplayCenterY - (pU->u32DisplayHeight>>1));
        pM->stDispRect.s32Y = (mt_s32)(pU->u32DisplayCenterY - (pU->u32DisplayHeight>>1));
        TRANSFER_A2B(pM->stDispRect.s32Width  , (mt_s32)pU->u32DisplayWidth);
        TRANSFER_A2B(pM->stDispRect.s32Height , (mt_s32)pU->u32DisplayHeight);
        TRANSFER_A2B(pM->u32AspectWidth , (mt_u8)pU->u32AspectWidth);
        TRANSFER_A2B(pM->u32AspectHeight, (mt_u8)pU->u32AspectHeight);

        // TODO:
        Transfer_FrameRate(pM->u32FrameRate , &pU->stFrameRate, MT_TRUE);
        TRANSFER_A2B(pM->u32FrameIndex, pU->u32FrameIndex);


        TRANSFER_A2B(pM->u32SrcPts , pU->u32SrcPts);
        TRANSFER_A2B(pM->u32Pts    , pU->u32Pts);

        if (pU->enFieldMode != MT_UNF_VIDEO_FIELD_BUTT)
        {
            pM->enFieldMode = s_DrvFMTab[pU->enFieldMode];
        }
        else
        {
            pM->enFieldMode = MT_DRV_FIELD_BUTT;
        }
        TRANSFER_A2B(pM->u32ErrorLevel, pU->u32ErrorLevel);


        TRANSFER_A2B(pM->stBufAddr[0].u32PhyAddr_Y  , pU->stVideoFrameAddr[0].u32YAddr    );
        TRANSFER_A2B(pM->stBufAddr[0].u32Stride_Y   , pU->stVideoFrameAddr[0].u32YStride  );
        TRANSFER_A2B(pM->stBufAddr[0].u32PhyAddr_C  , pU->stVideoFrameAddr[0].u32CAddr    );
        TRANSFER_A2B(pM->stBufAddr[0].u32Stride_C   , pU->stVideoFrameAddr[0].u32CStride  );
        TRANSFER_A2B(pM->stBufAddr[0].u32PhyAddr_Cr , pU->stVideoFrameAddr[0].u32CrAddr   );
        TRANSFER_A2B(pM->stBufAddr[0].u32Stride_Cr  , pU->stVideoFrameAddr[0].u32CrStride );
        TRANSFER_A2B(pM->stBufAddr[1].u32PhyAddr_Y  , pU->stVideoFrameAddr[1].u32YAddr    );
        TRANSFER_A2B(pM->stBufAddr[1].u32Stride_Y   , pU->stVideoFrameAddr[1].u32YStride  );
        TRANSFER_A2B(pM->stBufAddr[1].u32PhyAddr_C  , pU->stVideoFrameAddr[1].u32CAddr    );
        TRANSFER_A2B(pM->stBufAddr[1].u32Stride_C   , pU->stVideoFrameAddr[1].u32CStride  );
        TRANSFER_A2B(pM->stBufAddr[1].u32PhyAddr_Cr , pU->stVideoFrameAddr[1].u32CrAddr   );
        TRANSFER_A2B(pM->stBufAddr[1].u32Stride_Cr  , pU->stVideoFrameAddr[1].u32CrStride );

        TRANSFER_A2B(pM->u32Circumrotate, pU->u32Circumrotate);
        TRANSFER_A2B(pM->bToFlip_V, pU->bVerticalMirror);
        TRANSFER_A2B(pM->bToFlip_H, pU->bHorizontalMirror);

        memcpy(pM->u32Priv, pU->u32Private, sizeof(mt_u32) * 64);
        TRANSFER_A2B(pM->slotInfo.filedInfoTop.slot_idx, pU->filedInfoTop.slot_idx);
        TRANSFER_A2B(pM->slotInfo.filedInfoTop.pts, pU->filedInfoTop.pts);
        TRANSFER_A2B(pM->slotInfo.filedInfoTop.addrLuma, pU->filedInfoTop.addrLuma);
        TRANSFER_A2B(pM->slotInfo.filedInfoTop.addrChroma, pU->filedInfoTop.addrChroma);

        TRANSFER_A2B(pM->slotInfo.filedInfoBot.slot_idx, pU->filedInfoBot.slot_idx);
        TRANSFER_A2B(pM->slotInfo.filedInfoBot.pts, pU->filedInfoBot.pts);
        TRANSFER_A2B(pM->slotInfo.filedInfoBot.addrLuma, pU->filedInfoBot.addrLuma);
        TRANSFER_A2B(pM->slotInfo.filedInfoBot.addrChroma, pU->filedInfoBot.addrChroma);

        TRANSFER_A2B(pM->slotInfo.filedInfoTopRight.slot_idx, pU->filedInfoTopRight.slot_idx);
        TRANSFER_A2B(pM->slotInfo.filedInfoTopRight.pts, pU->filedInfoTopRight.pts);
        TRANSFER_A2B(pM->slotInfo.filedInfoTopRight.addrLuma, pU->filedInfoTopRight.addrLuma);
        TRANSFER_A2B(pM->slotInfo.filedInfoTopRight.addrChroma, pU->filedInfoTopRight.addrChroma);

        TRANSFER_A2B(pM->slotInfo.filedInfoBotRight.slot_idx, pU->filedInfoBotRight.slot_idx);
        TRANSFER_A2B(pM->slotInfo.filedInfoBotRight.pts, pU->filedInfoBotRight.pts);
        TRANSFER_A2B(pM->slotInfo.filedInfoBotRight.addrLuma, pU->filedInfoBotRight.addrLuma);
        TRANSFER_A2B(pM->slotInfo.filedInfoBotRight.addrChroma, pU->filedInfoBotRight.addrChroma);

        TRANSFER_A2B(pM->slotInfo.display_order_mode, pU->display_order_mode);
        TRANSFER_A2B(pM->slotInfo.packing_type, pU->packing_type);
        TRANSFER_A2B(pM->slotInfo.display_order_mode_valid, pU->display_order_mode_valid);
        TRANSFER_A2B(pM->slotInfo.filed_storage_mode, pU->filed_storage_mode);
        TRANSFER_A2B(pM->slotInfo.is_3D_flag, pU->is_3D_flag);


        return MT_SUCCESS;
    }
    else
    {
        if ((pM->ePixFormat > MT_DRV_PIX_BUTT)
            || (pM->enFieldMode > enDrvField)
            || (pM->eFrmType > MT_DRV_FT_BUTT)
            || (pM->enTBAdjust > enDrvTbAdjust))
        {
            return MT_ERR_VO_INVALID_PARA;
        }

        memset(pU, 0, sizeof(MT_UNF_VIDEO_FRAME_INFO_S));

        if (pM->eFrmType != MT_DRV_FT_BUTT)
        {
            pU->enFramePackingType = s_UnfFrameTypeTab[pM->eFrmType];
        }
        else
        {
            pU->enFramePackingType = MT_UNF_FRAME_PACKING_TYPE_BUTT;
        }

        Transfer_VideoFormat(&pU->enVideoFormat, &pM->ePixFormat, MT_FALSE);

        TRANSFER_A2B_R(pM->bProgressive, pU->bProgressive);
        TRANSFER_A2B_R(pM->bTopFieldFirst, pU->bTopFieldFirst);
        TRANSFER_A2B_R(pM->u32Width    , pU->u32Width);
        TRANSFER_A2B_R(pM->u32Height   , pU->u32Height);
        TRANSFER_A2B_R(pM->u64Pts, pU->u64Pts);
        TRANSFER_A2B_R(pM->end_of_stream_flag, pU->end_of_stream_flag);
        //TRANSFER_A2B_R(pM->stDispRect.s32X + (pM->stDispRect.s32Width>>1)  , pU->u32DisplayCenterX);
        pU->u32DisplayCenterX = (mt_u32)(pM->stDispRect.s32X + (pM->stDispRect.s32Width>>1));
        //TRANSFER_A2B_R(pM->stDispRect.s32Y + (pM->stDispRect.s32Height>>1) , pU->u32DisplayCenterY);
        pU->u32DisplayCenterY = (mt_u32)(pM->stDispRect.s32Y + (pM->stDispRect.s32Height>>1) );
        TRANSFER_A2B_R((mt_u32)pM->stDispRect.s32Width  , pU->u32DisplayWidth);

        TRANSFER_A2B_R((mt_u32)pM->stDispRect.s32Height , pU->u32DisplayHeight);
        TRANSFER_A2B_R((mt_u32)pM->u32AspectWidth, pU->u32AspectWidth);
        TRANSFER_A2B_R((mt_u32)pM->u32AspectHeight, pU->u32AspectHeight);

        Transfer_FrameRate(pM->u32FrameRate , &pU->stFrameRate, MT_FALSE);

        TRANSFER_A2B_R(pM->u32FrameIndex, pU->u32FrameIndex);
        TRANSFER_A2B_R(pM->u32SrcPts , pU->u32SrcPts);
        TRANSFER_A2B_R(pM->u32Pts    , pU->u32Pts);

        if (pM->enFieldMode != MT_DRV_FIELD_BUTT)
        {
            pU->enFieldMode = s_UnfFMTab[pM->enFieldMode];
        }
        else
        {
            pU->enFieldMode = MT_UNF_VIDEO_FIELD_BUTT;
        }
        TRANSFER_A2B_R(pM->u32ErrorLevel, pU->u32ErrorLevel);



        TRANSFER_A2B_R(pM->stBufAddr[0].u32PhyAddr_Y  , pU->stVideoFrameAddr[0].u32YAddr    );
        TRANSFER_A2B_R(pM->stBufAddr[0].u32Stride_Y   , pU->stVideoFrameAddr[0].u32YStride  );
        TRANSFER_A2B_R(pM->stBufAddr[0].u32PhyAddr_C  , pU->stVideoFrameAddr[0].u32CAddr    );
        TRANSFER_A2B_R(pM->stBufAddr[0].u32Stride_C   , pU->stVideoFrameAddr[0].u32CStride  );

        TRANSFER_A2B_R(pM->stBufAddr[0].u32PhyAddr_Cr , pU->stVideoFrameAddr[0].u32CrAddr   );
        TRANSFER_A2B_R(pM->stBufAddr[0].u32Stride_Cr  , pU->stVideoFrameAddr[0].u32CrStride );
        TRANSFER_A2B_R(pM->stBufAddr[1].u32PhyAddr_Y  , pU->stVideoFrameAddr[1].u32YAddr    );
        TRANSFER_A2B_R(pM->stBufAddr[1].u32Stride_Y   , pU->stVideoFrameAddr[1].u32YStride  );

        TRANSFER_A2B_R(pM->stBufAddr[1].u32PhyAddr_C  , pU->stVideoFrameAddr[1].u32CAddr    );
        TRANSFER_A2B_R(pM->stBufAddr[1].u32Stride_C   , pU->stVideoFrameAddr[1].u32CStride  );
        TRANSFER_A2B_R(pM->stBufAddr[1].u32PhyAddr_Cr , pU->stVideoFrameAddr[1].u32CrAddr   );
        TRANSFER_A2B_R(pM->stBufAddr[1].u32Stride_Cr  , pU->stVideoFrameAddr[1].u32CrStride );


        TRANSFER_A2B_R(pM->u32Circumrotate, pU->u32Circumrotate);
        TRANSFER_A2B_R(pM->bToFlip_V, pU->bVerticalMirror);
        TRANSFER_A2B_R(pM->bToFlip_H, pU->bHorizontalMirror);

        TRANSFER_A2B_R(pM->stLBufAddr[0].u32YAddr, pU->stLinearFrameAddr[0].u32YAddr);
        TRANSFER_A2B_R(pM->stLBufAddr[0].u32CAddr, pU->stLinearFrameAddr[0].u32CAddr);
        TRANSFER_A2B_R(pM->stLBufAddr[0].u32CrAddr, pU->stLinearFrameAddr[0].u32CrAddr);
        TRANSFER_A2B_R(pM->stLBufAddr[0].u32YStride, pU->stLinearFrameAddr[0].u32YStride);
        TRANSFER_A2B_R(pM->stLBufAddr[0].u32CStride, pU->stLinearFrameAddr[0].u32CStride);
        TRANSFER_A2B_R(pM->stLBufAddr[0].u32CrStride, pU->stLinearFrameAddr[0].u32CrStride);
        TRANSFER_A2B_R(pM->stLBufAddr[0].u32BufSize, pU->stLinearFrameAddr[0].u32BufSize);
        TRANSFER_A2B_R(pM->stLBufAddr[1].u32YAddr, pU->stLinearFrameAddr[1].u32YAddr);
        TRANSFER_A2B_R(pM->stLBufAddr[1].u32CAddr, pU->stLinearFrameAddr[1].u32CAddr);
        TRANSFER_A2B_R(pM->stLBufAddr[1].u32CrAddr, pU->stLinearFrameAddr[1].u32CrAddr);
        TRANSFER_A2B_R(pM->stLBufAddr[1].u32YStride, pU->stLinearFrameAddr[1].u32YStride);
        TRANSFER_A2B_R(pM->stLBufAddr[1].u32CStride, pU->stLinearFrameAddr[1].u32CStride);
        TRANSFER_A2B_R(pM->stLBufAddr[1].u32CrStride, pU->stLinearFrameAddr[1].u32CrStride);
        TRANSFER_A2B_R(pM->stLBufAddr[1].u32BufSize, pU->stLinearFrameAddr[1].u32BufSize);

        memcpy(pU->u32Private, pM->u32Priv, sizeof(mt_u32) * 64);
        TRANSFER_A2B_R(pM->slotInfo.filedInfoTop.slot_idx, pU->filedInfoTop.slot_idx);
        TRANSFER_A2B_R(pM->slotInfo.filedInfoTop.pts, pU->filedInfoTop.pts);
        TRANSFER_A2B_R(pM->slotInfo.filedInfoTop.addrLuma, pU->filedInfoTop.addrLuma);
        TRANSFER_A2B_R(pM->slotInfo.filedInfoTop.addrChroma, pU->filedInfoTop.addrChroma);

        TRANSFER_A2B_R(pM->slotInfo.filedInfoBot.slot_idx, pU->filedInfoBot.slot_idx);
        TRANSFER_A2B_R(pM->slotInfo.filedInfoBot.pts, pU->filedInfoBot.pts);
        TRANSFER_A2B_R(pM->slotInfo.filedInfoBot.addrLuma, pU->filedInfoBot.addrLuma);
        TRANSFER_A2B_R(pM->slotInfo.filedInfoBot.addrChroma, pU->filedInfoBot.addrChroma);

        TRANSFER_A2B_R(pM->slotInfo.filedInfoTopRight.slot_idx, pU->filedInfoTopRight.slot_idx);
        TRANSFER_A2B_R(pM->slotInfo.filedInfoTopRight.pts, pU->filedInfoTopRight.pts);
        TRANSFER_A2B_R(pM->slotInfo.filedInfoTopRight.addrLuma, pU->filedInfoTopRight.addrLuma);
        TRANSFER_A2B_R(pM->slotInfo.filedInfoTopRight.addrChroma, pU->filedInfoTopRight.addrChroma);

        TRANSFER_A2B_R(pM->slotInfo.filedInfoBotRight.slot_idx, pU->filedInfoBotRight.slot_idx);
        TRANSFER_A2B_R(pM->slotInfo.filedInfoBotRight.pts, pU->filedInfoBotRight.pts);
        TRANSFER_A2B_R(pM->slotInfo.filedInfoBotRight.addrLuma, pU->filedInfoBotRight.addrLuma);
        TRANSFER_A2B_R(pM->slotInfo.filedInfoBotRight.addrChroma, pU->filedInfoBotRight.addrChroma);

        TRANSFER_A2B_R(pM->slotInfo.display_order_mode, pU->display_order_mode);
        TRANSFER_A2B_R(pM->slotInfo.packing_type, pU->packing_type);
        TRANSFER_A2B_R(pM->slotInfo.display_order_mode_valid, pU->display_order_mode_valid);
        TRANSFER_A2B_R(pM->slotInfo.filed_storage_mode, pU->filed_storage_mode);
        TRANSFER_A2B_R(pM->slotInfo.is_3D_flag, pU->is_3D_flag);


        return MT_SUCCESS;
    }
}

mt_s32 Transfer_BufferPool(MT_UNF_BUFFER_ATTR_S *pU, MT_DRV_VIDEO_BUFFER_POOL_S*pM, MT_BOOL bu2m)
{
#if 0
    if (bu2m)
    {

    }
    else
    {

    }
#endif

    return MT_SUCCESS;
}

mt_s32 Transfer_CastCfg(MT_UNF_DISP_CAST_ATTR_S  *pU, MT_DRV_DISP_CAST_CFG_S *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        mt_s32 i;
        memset(pM, 0, sizeof(MT_DRV_DISP_CAST_CFG_S));

        Transfer_VideoFormat(&pU->enFormat, &pM->eFormat, MT_TRUE);

        pM->u32Width   = pU->u32Width;
        pM->u32Height  = pU->u32Height;

        pM->u32BufNumber = pU->u32BufNum;
        pM->bUserAlloc   = pU->bUserAlloc;
        pM->bLowDelay    = pU->bLowDelay;
        pM->u32BufSize   = pU->u32BufSize;
        pM->u32BufStride = pU->u32BufStride;
        for(i=0; i<MT_DISP_CAST_BUFFER_MAX_NUMBER; i++)
        {
            pM->u32BufPhyAddr[i] = pU->u32BufPhyAddr[i];
        }
        return MT_SUCCESS;
    }
    else
    {
        return MT_FAILURE;
    }
}

mt_s32 Transfer_CgmsCfg(const MT_UNF_DISP_CGMS_CFG_S  *pU, MT_DRV_DISP_CGMSA_CFG_S *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
		pM->bEnable =  pU->bEnable;
		pM->enType  =  (MT_DRV_DISP_CGMSA_TYPE_E)pU->enType;
		pM->enMode  =  (MT_DRV_DISP_CGMSA_MODE_E)pU->enMode;
		return MT_SUCCESS;
    }
    else
    {
        return MT_FAILURE;
    }
}

mt_u8 Transfer_GetVdacIdFromPinIDForMPW(mt_u8 PinId)
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

mt_s32 Transfer_Intf(MT_UNF_DISP_INTF_S *pU, MT_DRV_DISP_INTF_S *pM, MT_BOOL bu2m)
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
                pM->eID = (MT_DRV_DISP_INTF_ID_E)((mt_u32)MT_DRV_DISP_INTF_HDMI0 + ((mt_u32)pU->unIntf.enHdmi - (mt_u32)MT_UNF_HDMI_ID_0));
                if ((pM->eID > MT_DRV_DISP_INTF_HDMI2) || (pM->eID < MT_DRV_DISP_INTF_HDMI0))
                {
                    return MT_FAILURE;
                }
                break;
            case MT_UNF_DISP_INTF_TYPE_YPBPR:
                pM->eID = MT_DRV_DISP_INTF_YPBPR0;
                pM->u8VDAC_Y_G  = Transfer_GetVdacIdFromPinIDForMPW(pU->unIntf.stYPbPr.u8DacY);
                pM->u8VDAC_Pb_B = Transfer_GetVdacIdFromPinIDForMPW(pU->unIntf.stYPbPr.u8DacPb);
                pM->u8VDAC_Pr_R = Transfer_GetVdacIdFromPinIDForMPW(pU->unIntf.stYPbPr.u8DacPr);
                break;
            case MT_UNF_DISP_INTF_TYPE_SVIDEO:
                pM->eID = MT_DRV_DISP_INTF_SVIDEO0;
                pM->u8VDAC_Y_G  = Transfer_GetVdacIdFromPinIDForMPW(pU->unIntf.stSVideo.u8DacY);
                pM->u8VDAC_Pb_B = Transfer_GetVdacIdFromPinIDForMPW(pU->unIntf.stSVideo.u8DacC);
                break;
            case MT_UNF_DISP_INTF_TYPE_CVBS:
                pM->eID = MT_DRV_DISP_INTF_CVBS0;
                pM->u8VDAC_Y_G  = Transfer_GetVdacIdFromPinIDForMPW(pU->unIntf.stCVBS.u8Dac);
                break;
            case MT_UNF_DISP_INTF_TYPE_RGB:
                pM->eID = MT_DRV_DISP_INTF_RGB0;
                pM->u8VDAC_Y_G  = Transfer_GetVdacIdFromPinIDForMPW(pU->unIntf.stRGB.u8DacG);
                pM->u8VDAC_Pb_B = Transfer_GetVdacIdFromPinIDForMPW(pU->unIntf.stRGB.u8DacB);
                pM->u8VDAC_Pr_R = Transfer_GetVdacIdFromPinIDForMPW(pU->unIntf.stRGB.u8DacR);
                pM->bDacSync = pU->unIntf.stRGB.bDacSync;
                break;
            case MT_UNF_DISP_INTF_TYPE_VGA:
                pM->eID = MT_DRV_DISP_INTF_VGA0;
                pM->u8VDAC_Y_G  = Transfer_GetVdacIdFromPinIDForMPW(pU->unIntf.stRGB.u8DacG);
                pM->u8VDAC_Pb_B = Transfer_GetVdacIdFromPinIDForMPW(pU->unIntf.stRGB.u8DacB);
                pM->u8VDAC_Pr_R = Transfer_GetVdacIdFromPinIDForMPW(pU->unIntf.stRGB.u8DacR);
                pM->bDacSync = pU->unIntf.stRGB.bDacSync;
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

mt_s32 Transfer_DispPPMode(MT_UNF_DISP_PP_E *pU, MT_DRV_DISP_PPMODE_E *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        *pM = (MT_DRV_DISP_PPMODE_E)(*pU);
        return MT_SUCCESS;
    }
    else
    {
        *pU = (MT_UNF_DISP_PP_E)(*pM);
        return MT_SUCCESS;
    }
}

mt_s32 Transfer_DispTvCap(MT_UNF_DISP_HDMI_MODE_E *pU, MT_DRV_DISP_HDMI_MODE_E *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        *pM = (MT_DRV_DISP_HDMI_MODE_E)(*pU);
        return MT_SUCCESS;
    }
    else
    {
        *pU = (MT_UNF_DISP_HDMI_MODE_E)(*pM);
        return MT_SUCCESS;
    }
}

mt_s32 Transfer_Wss(MT_UNF_DISP_WSS_DATA_S *pU, MT_DRV_DISP_WSS_DATA_S *pM, MT_BOOL bu2m)
{
	if (bu2m)
	{
		pM->bEnable = pU->bEnable;
		pM->u16Data = pU->u16Data;
	}
	else
	{
		pU->bEnable = pM->bEnable;
		pU->u16Data = pM->u16Data;
	}

	return MT_SUCCESS;
}

mt_s32 Transfer_DumpScaler(MT_UNF_DISP_DUMP_SCALER_PARA_S *pU, MT_DRV_DISP_DUMP_SCALER_PARA_S *pM, MT_BOOL bu2m)
{
    if (bu2m)
    {
        pM->b_enable = pU->b_enable;
        pM->source = pU->source;
        pM->dst_layer = pU->dst_layer;
        pM->out_width = pU->out_width;
        pM->out_height = pU->out_height;            
    }
    else
    {
        pU->b_enable = pM->b_enable;
        pU->source = pM->source;
        pU->dst_layer = pM->dst_layer;
        pU->out_width = pM->out_width;
        pU->out_height = pM->out_height;  
    }
    return MT_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

