
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_bufcore.c
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#include <linux/sizes.h>	/*SZ_1K*/

#include "drv_win_policy.h"
#include "mt_module_debug.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#define _SR_OPEN_



static int caculate_sr_size(mt_rect_s *pstFrameRect,
               mt_rect_s *pstOutRect,
               mt_rect_s *pstSrRect,
               mt_u32    u32WinNum,
               MT_BOOL *pbHorSrEnable,
               MT_BOOL *pbVerSrEnable,
               mt_rect_s *pstFmtResolution,
               MT_BOOL bExistScaleDown,
               MT_DRV_DISP_STEREO_E enStereo)
{

#if 0
    MT_CHIP_TYPE_E   enChipType;
    MT_CHIP_VERSION_E enChipVersion;
#ifndef _SR_OPEN_
    return MT_FALSE;
#endif

    /*no matter 98m or 98c, when winnum >2, sr should not opened.*/
    if ((u32WinNum >= 2) || (bExistScaleDown) || (enStereo != DISP_STEREO_NONE))
    {
        return MT_FALSE;
    }

    /*currently,only 4k support sr.*/
    if (!((pstFmtResolution->s32Width == 3840)
           &&(pstFmtResolution->s32Height == 2160))
       )
    {
        return MT_FALSE;
    }

    if ((pstFrameRect->s32Width*2) <= pstOutRect->s32Width)
    {
        *pbHorSrEnable = MT_TRUE;
        pstSrRect->s32Width =  pstOutRect->s32Width/2;
    }

    if ((pstFrameRect->s32Height*2) <= pstOutRect->s32Height)
    {
        *pbVerSrEnable = MT_TRUE;
        pstSrRect->s32Height =  pstOutRect->s32Height/2;
    }

    /*the limit of sr is: max input size is 2k*2k,
     * if larger than this, and make a zme even in vertical direction, error happens.
     * so we should turn sr off, the recover the value.
     */
    if ((*pbVerSrEnable) || (*pbHorSrEnable))
    {
        if ((pstSrRect->s32Width > 1920)
            || (pstSrRect->s32Height > 2160)
            || !(*pbVerSrEnable && *pbHorSrEnable)
           )
        {
            *pbHorSrEnable = MT_FALSE;
            *pbVerSrEnable = MT_FALSE;
            pstSrRect->s32Width =  pstOutRect->s32Width;
            pstSrRect->s32Height =  pstOutRect->s32Height;
        }
    }

    MT_DRV_SYS_GetChipVersion(&enChipType, &enChipVersion);

    /*when 3798m added, this branch should be opened.*/
    if (0)
    {
        if ((pstFrameRect->s32Width  > 1920) && (pstOutRect->s32Width > 1920))
        {
            *pbHorSrEnable = MT_TRUE;
            pstSrRect->s32Width =  pstOutRect->s32Width/2;
        }
    }
#endif
    return MT_SUCCESS;
}

static  mt_void generate_FrameRect(WINDOW_S *pstWin,
                                   mt_rect_s *pstSourceFrameRect,
                                   MT_BOOL *pbExistScaleDown)
{
    MT_DRV_VIDEO_FRAME_S *pstDispFrame = MT_NULL;
    MT_DRV_VIDEO_PRIVATE_S *pstPrivInfo = MT_NULL;

    *pbExistScaleDown = MT_FALSE;

    if (!pstWin->stBuffer.stWinBP.pstDisplay)
        goto __error;

    pstDispFrame = (MT_DRV_VIDEO_FRAME_S *)pstWin->stBuffer.stWinBP.pstDisplay->u32Data;
    if (!pstDispFrame)
        goto __error;

    pstPrivInfo = (MT_DRV_VIDEO_PRIVATE_S *)(pstDispFrame->u32Priv);
    if (!pstPrivInfo)
        goto __error;

    pstSourceFrameRect->s32X = 0;
    pstSourceFrameRect->s32Y= 0;
    if (pstDispFrame)
    {
        pstSourceFrameRect->s32Width  = pstPrivInfo->stVideoOriginalInfo.u32Width;
        pstSourceFrameRect->s32Height = pstPrivInfo->stVideoOriginalInfo.u32Height;
    }

    if ((pstDispFrame->stLbxInfo.s32Width  < pstSourceFrameRect->s32Width)
        || (pstDispFrame->stLbxInfo.s32Height  < pstSourceFrameRect->s32Height))
    {
        *pbExistScaleDown = MT_TRUE;
    }

    pstWin->stWinInfoForDeveloper.stOringinFrameSize = *pstSourceFrameRect;
    pstWin->stWinInfoForDeveloper.bExistScaleDown_WhenRatioRevise = *pbExistScaleDown;

    return;
__error:

    pstSourceFrameRect->s32Width  = 4096;
    pstSourceFrameRect->s32Height = 2160;
    return;
}


static int pre_ScalerDistribute(mt_rect_s *pstVpssZmeSize,
                                 mt_rect_s *pstFinalZmeSize,
                                 mt_u32  u32WinNum,
                                 MT_BOOL *pbHorSrEnable,
                                 MT_BOOL *pbVerSrEnable,
                                 MT_BOOL  bZmeCapability,
                                 MT_BOOL  bSrCapability,
                                 mt_u32   u32LayerMAXWidth,
                                 mt_u32   u32LayerMAXHeight,
                                 mt_rect_s *pstSourceFrameRect,
                                 mt_rect_s *pstFmtResolution,
                                 MT_BOOL bExistScaleDown,
                                 MT_DRV_DISP_STEREO_E enStereo)
{
    mt_rect_s stVpssZmeSize;
    MT_BOOL bHorSrEnable = MT_FALSE;
    MT_BOOL bVerSrEnable = MT_FALSE;


    stVpssZmeSize = *pstFinalZmeSize;

    /*if no zme exist, the size must be limited to the video capability.*/
    if (!bZmeCapability)
    {
        if (pstFinalZmeSize->s32Width > u32LayerMAXWidth)
        {
            pstVpssZmeSize->s32Width  = u32LayerMAXWidth;
        }

        if (pstFinalZmeSize->s32Height > u32LayerMAXHeight)
        {
            pstVpssZmeSize->s32Height = u32LayerMAXHeight;
        }

    }
    else
    {
        if (!bSrCapability)
        {
            /*if no sr, such as cv200,  should consider the video capability,
             * and  the aspect ratio should be kept.
             */
            if((pstFinalZmeSize->s32Width > u32LayerMAXWidth)||(pstFinalZmeSize->s32Height > u32LayerMAXHeight))
            {
                if(u32LayerMAXWidth * pstFinalZmeSize->s32Height > u32LayerMAXHeight * pstFinalZmeSize->s32Width)
                {
                    pstVpssZmeSize->s32Width = (pstFinalZmeSize->s32Width*u32LayerMAXHeight/pstFinalZmeSize->s32Height)&MT_WIN_OUT_RECT_WIDTH_ALIGN;
                    pstVpssZmeSize->s32Height = u32LayerMAXHeight;
                }
                else
                {
                    pstVpssZmeSize->s32Height = (pstFinalZmeSize->s32Height*u32LayerMAXWidth/pstFinalZmeSize->s32Width)&MT_WIN_OUT_RECT_HEIGHT_ALIGN;
                    pstVpssZmeSize->s32Width = u32LayerMAXWidth;
                }
            }
        }
        else
        {
            /* if SR supported, to judge to  open SR or not.
             * if SR enabled, the size should be divided  into  2 parts bettween VPSS and vdp.
             */
            caculate_sr_size(pstSourceFrameRect,
                            pstFinalZmeSize,
                            &stVpssZmeSize,
                            u32WinNum,
                            &bHorSrEnable,
                            &bVerSrEnable,
                            pstFmtResolution,
                            bExistScaleDown,
                            enStereo);

            /*when sr enable ,but the 1/2 size is larger than video layer cap, do a scale down.*/
            if((stVpssZmeSize.s32Width > u32LayerMAXWidth)||(stVpssZmeSize.s32Height > u32LayerMAXHeight))
            {
                /*when sr enable,but 1/2 size  larger than max, the vpss zme size should be calculated again.*/
                if(u32LayerMAXWidth * pstFinalZmeSize->s32Height > u32LayerMAXHeight * pstFinalZmeSize->s32Width)
                {
                    pstVpssZmeSize->s32Width = (pstFinalZmeSize->s32Width*u32LayerMAXHeight/pstFinalZmeSize->s32Height)&MT_WIN_OUT_RECT_WIDTH_ALIGN;
                    pstVpssZmeSize->s32Height = u32LayerMAXHeight;
                }
                else
                {
                    pstVpssZmeSize->s32Height = (pstFinalZmeSize->s32Height*u32LayerMAXWidth/pstFinalZmeSize->s32Width)&MT_WIN_OUT_RECT_HEIGHT_ALIGN;
                    pstVpssZmeSize->s32Width = u32LayerMAXWidth;
                }

                /*no matter how, sr should be disabled.*/
                bHorSrEnable = MT_FALSE;
                bVerSrEnable = MT_FALSE;
            }
            else
            {
                pstVpssZmeSize->s32Width = stVpssZmeSize.s32Width;
                pstVpssZmeSize->s32Height = stVpssZmeSize.s32Height;
            }
        }
    }

    if ((!pstVpssZmeSize->s32Height) || (!pstVpssZmeSize->s32Width))
    {
        pstVpssZmeSize->s32Width =   VIDEO_LAYER_SUPPORT_MIN_WIDTH;
        pstVpssZmeSize->s32Height =  VIDEO_LAYER_SUPPORT_MIN_HEIGHT;
    }

    pstVpssZmeSize->s32Width  &=  MT_WIN_OUT_RECT_WIDTH_ALIGN;
    pstVpssZmeSize->s32Height &=  MT_WIN_OUT_RECT_HEIGHT_ALIGN;

    *pbHorSrEnable  = bHorSrEnable;
    *pbVerSrEnable  = bVerSrEnable;

    return MT_SUCCESS;
}

/*principly, we only use vpss to zme out or in.  but video layer
 * only support a limit input size.  so in large scale zme, we should
 * do a 2-class zme: vpss and vdp both do scaler.
 */
int Win_Pre_ScalerDistribute(WINDOW_S *pstWin,
                             mt_rect_s *pstVpssZmeSize,
                             mt_rect_s *pstFinalZmeSize,
                             mt_u32 u32WinNum,
                             MT_BOOL *pbHorSrEnable,
                             MT_BOOL *pbVerSrEnable,
                             mt_rect_s *pstFmtResolution,
                             MT_DRV_DISP_STEREO_E enStereo)
{
    unsigned int u32LayerSupportWidthMax = 0, u32LayerSupportHeightMax = 0;
    VIDEO_LAYER_CAPABILITY_S  stVideoLayerCap;
    mt_rect_s stVpssZmeSize;
    mt_rect_s stSourceFrameRect;

    MT_BOOL bZmeCapability = MT_FALSE;
    MT_BOOL bSrCapability  = MT_FALSE;
    mt_s32 ret = 0;
    MT_BOOL bExistScaleDown = MT_FALSE;

    if ((ret = pstWin->stVLayerFunc.PF_GetCapability(pstWin->u32VideoLayer,&stVideoLayerCap)))
    {
        return ret;
    }

    memset((void*)&stSourceFrameRect, 0, sizeof(mt_rect_s));
    generate_FrameRect(pstWin, &stSourceFrameRect, &bExistScaleDown);

    bZmeCapability = stVideoLayerCap.bZme;
    bSrCapability = stVideoLayerCap.bSR;

    u32LayerSupportWidthMax = stVideoLayerCap.u32LayerWidthMax;
    u32LayerSupportHeightMax = stVideoLayerCap.u32LayerHeightMax;
    stVpssZmeSize = *pstFinalZmeSize;

    pre_ScalerDistribute(pstVpssZmeSize,
                        pstFinalZmeSize,
                        u32WinNum,
                        pbHorSrEnable,
                        pbVerSrEnable,
                        bZmeCapability,
                        bSrCapability,
                        u32LayerSupportWidthMax,
                        u32LayerSupportHeightMax,
                        &stSourceFrameRect,
                        pstFmtResolution,
                        bExistScaleDown,
                        enStereo);

    return MT_SUCCESS;
}


/*if  outrect/inrect > 2, the primary condition of sr's openning is satified.
 *but vpss does not give a size vdp want, so vdp should give a second scaler.
 */
mt_s32 post_ScalerProcess(mt_rect_s *pstFinalDisPosition,
                        mt_rect_s *pstV0DisPosition,
                        MT_BOOL   *pbHorSrEnable,
                        MT_BOOL   *pbVerSrEnable,
                        mt_u32    u32WinNum,
                        mt_rect_s *pstSourceFrameRect,
                        MT_CHIP_TYPE_E   enChipType,
                        MT_BOOL bSrCapability,
                        const mt_rect_s *pstFmtResolution,
                        MT_DRV_DISP_STEREO_E enStereo)
{

    *pbHorSrEnable = MT_FALSE;
    *pbVerSrEnable = MT_FALSE;

    if ((u32WinNum  < 2) && (enStereo == DISP_STEREO_NONE))
    {
        if ((pstSourceFrameRect->s32Width > 1920) || (pstSourceFrameRect->s32Height > 2160))
        {
            *pbHorSrEnable = MT_FALSE;
            *pbVerSrEnable = MT_FALSE;
        }
        else if (!((pstFmtResolution->s32Width == 3840)
           &&(pstFmtResolution->s32Height == 2160))
                )
        {
            *pbHorSrEnable = MT_FALSE;
            *pbVerSrEnable = MT_FALSE;
        }
        else
        {
            if (((pstSourceFrameRect->s32Width*2) <= pstFinalDisPosition->s32Width)
                && (bSrCapability))
            {
                *pbHorSrEnable = MT_TRUE;
            }

            if (((pstSourceFrameRect->s32Height*2) <= pstFinalDisPosition->s32Height)
                 && (bSrCapability))
            {
                *pbVerSrEnable = MT_TRUE;
            }
        }

        /*both h and vertical meets sr, then open sr, else close it.*/
        if (!(*pbHorSrEnable && *pbVerSrEnable))
        {
            *pbHorSrEnable = MT_FALSE;
            *pbVerSrEnable = MT_FALSE;
        }
    }

#ifndef _SR_OPEN_
    *pbHorSrEnable = MT_FALSE;
    *pbVerSrEnable = MT_FALSE;
#endif

    {
        /*because SR in v0 or no SR, so  v0 positon == final position.*/
        /*98m sr is in v0, so v0 output position remain the same with cv200.*/
        *pstV0DisPosition = *pstFinalDisPosition;
    }

    pstV0DisPosition->s32X &=  MT_WIN_OUT_RECT_X_ALIGN;
    pstV0DisPosition->s32Y &=  MT_WIN_OUT_RECT_Y_ALIGN;
    pstV0DisPosition->s32Width &=  MT_WIN_OUT_RECT_WIDTH_ALIGN;
    pstV0DisPosition->s32Height &=  MT_WIN_OUT_RECT_HEIGHT_ALIGN;

    return MT_SUCCESS;
}

int Win_Post_ScalerProcess( WINDOW_S  *pstWin,
                            mt_rect_s *pstFinalDisPosition,
                            mt_rect_s *pstV0DisPosition,
                            MT_BOOL   *pbHorSrEnable,
                            MT_BOOL   *pbVerSrEnable,
                            mt_u32 u32WinNum,
                            mt_rect_s *pstSourceFrameRect,
                            const mt_rect_s *pstFmtResolution,
                            MT_DRV_DISP_STEREO_E enStereo)
{
#if 0
    MT_CHIP_TYPE_E   enChipType;
    MT_CHIP_VERSION_E enChipVersion;
    VIDEO_LAYER_CAPABILITY_S  stVideoLayerCap;
    MT_BOOL bSrCapability  = MT_FALSE;
    mt_s32 ret = 0;

    MT_DRV_SYS_GetChipVersion(&enChipType, &enChipVersion);

    if ((ret = pstWin->stVLayerFunc.PF_GetCapability(pstWin->u32VideoLayer,&stVideoLayerCap)))
    {
        return ret;
    }

    bSrCapability = stVideoLayerCap.bSR;
    post_ScalerProcess(pstFinalDisPosition,
                       pstV0DisPosition,
                       pbHorSrEnable,
                       pbVerSrEnable,
                       u32WinNum,
                       pstSourceFrameRect,
                       enChipType,
                       bSrCapability,
                       pstFmtResolution,
                       enStereo);
#endif
    return MT_SUCCESS;
}

#if 0
static mt_void updateDci(WINDOW_S *pstWin,
                      MT_BOOL bDciEnable,
                      MT_PQ_DCI_WIN_S *pstActualDciArea)
{

    if (pstWin->enDisp == MT_DRV_DISPLAY_1)
    {
        /*this flag is not a control of dci,
         *just if dci is open ,then make a choice.
         */
        DRV_PQ_UpdateDCIWin(pstActualDciArea, bDciEnable);
    }

    /*save the value for next judgement.*/
    pstWin->stMiscInfor.stWinDciLastConfig = *pstActualDciArea;
    pstWin->stMiscInfor.bWinDciEnableLastConfig = bDciEnable;

    return;
}
#endif

mt_s32 dci_enable_policy(mt_rect_s *pstOriginDciArea,/*vpss's effective area.*/
                         mt_rect_s *pstV0DisPosition,/*secondary coordinate*/
                         mt_rect_s *pstVpssGive,/*the original rect give by vpss.*/
                         mt_u32    u32WinNum,
                         MT_BOOL  *pbDciEnable,
                         MT_PQ_DCI_WIN_S *pstActualDciArea)
{
    mt_u32 u32OriginDCI_Width = 0;
    mt_u32 u32OriginDCI_Height = 0;

    mt_u32 u32SecondCoord_DCI_X_Start = 0;
    mt_u32 u32SecondCoord_DCI_X_End = 0;

    mt_u32 u32SecondCoord_DCI_Y_Start = 0;
    mt_u32 u32SecondCoord_DCI_Y_End = 0;

    *pbDciEnable = MT_FALSE;

    /*when win >=2, we only cover the window on V0, 2014-0910,
     *discussed by zk and zlling.*/
#if 0
    if (u32WinNum >= 2)
    {
        memset((void*)pstActualDciArea, 0, sizeof(MT_PQ_DCI_WIN_S));
        return MT_SUCCESS;
    }
#endif

    u32OriginDCI_Width = pstOriginDciArea->s32Width;
    u32OriginDCI_Height = pstOriginDciArea->s32Height;


    /*although 98m different with 98c, but we both use pstV0DisPosition, not final.
     * the final and v0's calculation is done in Win_Post_ScalerProcess*/
    /*secondly, caculate the new dci coordinate in the secondary coordinate.*/
    u32SecondCoord_DCI_X_Start = (pstOriginDciArea->s32X * pstV0DisPosition->s32Width) / pstVpssGive->s32Width;
    u32SecondCoord_DCI_Y_Start = (pstOriginDciArea->s32Y * pstV0DisPosition->s32Height) / pstVpssGive->s32Height;

    u32SecondCoord_DCI_X_End = u32SecondCoord_DCI_X_Start  +
                (u32OriginDCI_Width *  pstV0DisPosition->s32Width) / pstVpssGive->s32Width;

    u32SecondCoord_DCI_Y_End = u32SecondCoord_DCI_Y_Start  +
         (u32OriginDCI_Height *  pstV0DisPosition->s32Height) / pstVpssGive->s32Height;

    /*3rdly, calculate the final dci in middle coordinate system.*/
    u32SecondCoord_DCI_X_Start += pstV0DisPosition->s32X;
    u32SecondCoord_DCI_X_End   += pstV0DisPosition->s32X;

    u32SecondCoord_DCI_Y_Start  += pstV0DisPosition->s32Y;
    u32SecondCoord_DCI_Y_End  += pstV0DisPosition->s32Y;

    pstActualDciArea->u16HStar = u32SecondCoord_DCI_X_Start;
    pstActualDciArea->u16HEnd = u32SecondCoord_DCI_X_End;
    pstActualDciArea->u16VStar = u32SecondCoord_DCI_Y_Start;
    pstActualDciArea->u16VEnd = u32SecondCoord_DCI_Y_End;

    *pbDciEnable = MT_TRUE;
    return MT_SUCCESS;
}

mt_void Win_DciEnable_Policy(WINDOW_S *pstWin,
                         mt_rect_s *pstOriginDciArea,/*vpss's effective area.*/
                         mt_rect_s *pstV0DisPosition,/*secondary coordinate*/
                         mt_rect_s *pstVpssGive,/*the original rect give by vpss.*/
                         mt_u32    u32WinNum)
{
#if 0
    MT_BOOL   bDciEnable = MT_FALSE;
    MT_BOOL   bDciCapability = MT_FALSE;
    MT_PQ_DCI_WIN_S stActualDciArea;
    mt_rect_s stActualDciAreaRect;
    VIDEO_LAYER_CAPABILITY_S  stVideoLayerCap;
    mt_s32 ret = 0;

    if ((ret = pstWin->stVLayerFunc.PF_GetCapability(pstWin->u32VideoLayer,&stVideoLayerCap)))
    {
        return ;
    }

    bDciCapability = stVideoLayerCap.bDci;
    if (bDciCapability)
    {
        dci_enable_policy(pstOriginDciArea,/*vpss's effective area.*/
                            pstV0DisPosition,/*secondary coordinate*/
                            pstVpssGive,/*the original rect give by vpss.*/
                            u32WinNum,
                            &bDciEnable,
                            &stActualDciArea);

        updateDci(pstWin, bDciEnable, &stActualDciArea);

        stActualDciAreaRect.s32X = stActualDciArea.u16HStar;
        stActualDciAreaRect.s32Y = stActualDciArea.u16VStar;

        stActualDciAreaRect.s32Width = stActualDciArea.u16HEnd - stActualDciArea.u16HStar;
        stActualDciAreaRect.s32Height =stActualDciArea.u16VEnd - stActualDciArea.u16VStar;

        pstWin->stWinInfoForDeveloper.stOriginDCIPositionInFrame = *pstOriginDciArea;
        pstWin->stWinInfoForDeveloper.stDciFrameSize             = *pstVpssGive;
        pstWin->stWinInfoForDeveloper.bDciOpen                   = bDciEnable;
        pstWin->stWinInfoForDeveloper.stWinFinalPosition         = stActualDciAreaRect;
    }
#endif
    return ;
}


/*since we support out of window, so when out of window, the content and size of
 * the window  should be  cropped.
 */
mt_s32 Win_Revise_OutOfScreenWin_OutRect(mt_rect_s *pstInRect,
                        mt_rect_s *pstOutRect,
                        mt_rect_s stScreen,
                        MT_DRV_DISP_OFFSET_S stOffsetRect,
                        WIN_HAL_PARA_S *pstLayerPara)
{
    mt_rect_s stInRect = *pstInRect;
    mt_rect_s stOutRect = *pstOutRect;
    mt_s32 s32VaildWidth,s32VaildHeight;
    mt_s32 ix = 0, iy = 0, iw = 0, ih = 0;
    mt_s32 ox =0, oy = 0, ow = 0, oh = 0;
    MT_BOOL bDispFlag = MT_FALSE;

    stScreen.s32X = 0;
    stScreen.s32Y = 0;

    stScreen.s32Width -= stOffsetRect.u32Left + stOffsetRect.u32Right;
    stScreen.s32Height -= stOffsetRect.u32Top + stOffsetRect.u32Bottom;
    /*stOutRect considered the offset already, so no need to - left.*/
    stOutRect.s32X  -= stOffsetRect.u32Left;
    stOutRect.s32Y  -= stOffsetRect.u32Top;

    if(stOutRect.s32X < 0)
    {
        s32VaildWidth = stOutRect.s32Width  + stOutRect.s32X;

        if(s32VaildWidth < WIN_INRECT_MIN_WIDTH)
        {
            ow = WIN_INRECT_MIN_WIDTH;
            ox = 0;
            iw = stInRect.s32Width * ow/stOutRect.s32Width;
            ix = stInRect.s32Width - iw;

            bDispFlag = MT_TRUE;
        }
        else
        {   // >screen size
            ow = stScreen.s32Width;
            if(s32VaildWidth < ow) // no full screan
            {
                 ow = s32VaildWidth;
            }

            ox = 0;
            iw = stInRect.s32Width * ow/stOutRect.s32Width;
            ix = stInRect.s32Width - stInRect.s32Width * s32VaildWidth/stOutRect.s32Width;
        }
    }
    else if (stOutRect.s32X < (stScreen.s32Width - WIN_INRECT_MIN_WIDTH))
    {
        s32VaildWidth = stOutRect.s32Width;

        if(s32VaildWidth < WIN_INRECT_MIN_WIDTH)
        {
            ow = WIN_INRECT_MIN_WIDTH;
            ox = stOutRect.s32X;
            iw = stInRect.s32Width;
            ix = 0;
            bDispFlag = MT_TRUE;
        }
        else
        {
            ox = stOutRect.s32X;
            ix = 0;

            if((s32VaildWidth + ox) > stScreen.s32Width)
            {
                ow = stScreen.s32Width - stOutRect.s32X;
                iw = stInRect.s32Width*ow/stOutRect.s32Width;
            }
            else
            {
               ow = stOutRect.s32Width;
               iw = stInRect.s32Width;
            }
        }
    }
    else
    {
        ow = WIN_INRECT_MIN_WIDTH;
        ox = stScreen.s32Width  - ow;
        iw = stInRect.s32Width*ow/stOutRect.s32Width;
        ix = 0;
        bDispFlag = MT_TRUE;
    }


    if(stOutRect.s32Y < 0)
    {
        s32VaildHeight = stOutRect.s32Height + stOutRect.s32Y;
        if(s32VaildHeight < WIN_INRECT_MIN_HEIGHT)
        {
            oy = 0;
            oh = WIN_INRECT_MIN_HEIGHT;
            ih = stInRect.s32Height*oh/stOutRect.s32Height;
            iy = stInRect.s32Height - ih;
            bDispFlag = MT_TRUE;
        }
        else
        {
            oy = 0;
            oh = stScreen.s32Height;
            if(s32VaildHeight < oh)
            {
                oh = s32VaildHeight;
            }

            ih = stInRect.s32Height*oh/stOutRect.s32Height;
            iy = stInRect.s32Height - stInRect.s32Height*s32VaildHeight/stOutRect.s32Height;
        }
    }
    else if (stOutRect.s32Y < (stScreen.s32Height - WIN_INRECT_MIN_HEIGHT))
    {
        s32VaildHeight = stOutRect.s32Height;

        if(s32VaildHeight < WIN_INRECT_MIN_HEIGHT)
        {
            oy = stOutRect.s32Y;
            oh = WIN_INRECT_MIN_HEIGHT;
            ih = stInRect.s32Height;
            iy = 0;
            bDispFlag = MT_TRUE;
        }
        else
        {
            oy = stOutRect.s32Y;
            iy = 0;
            if((s32VaildHeight + oy) > stScreen.s32Height)
            {
                oh = stScreen.s32Height - stOutRect.s32Y;
                ih = stInRect.s32Height * oh/stOutRect.s32Height;
            }
            else
            {
                oh = stOutRect.s32Height;
                ih = stInRect.s32Height;
            }
         }
    }
    else
    {
        oh = WIN_INRECT_MIN_HEIGHT;
        oy = stScreen.s32Height - oh;
        ih = stInRect.s32Height * oh/stOutRect.s32Height;
        iy = 0;
        bDispFlag = MT_TRUE;
    }

    ox  += stOffsetRect.u32Left;
    oy  += stOffsetRect.u32Top;

    /*DTS2013100801678, pstInRect in fact is the output of vpss(zme,dei,crop dnr)
     * it does not confirm to  the align limit of inrect.
     * If we give a wrong use, and make a wrong align,
     * there will be no consistency bettween GFX and VIDEO, and the backgroud color will appears.
     */
    pstInRect->s32X = ix & MT_WIN_OUT_RECT_X_ALIGN;
    pstInRect->s32Y = iy & MT_WIN_OUT_RECT_Y_ALIGN;
    pstInRect->s32Width = iw & MT_WIN_OUT_RECT_WIDTH_ALIGN;
    pstInRect->s32Height = ih & MT_WIN_OUT_RECT_HEIGHT_ALIGN;

    pstOutRect->s32X = ox & MT_WIN_OUT_RECT_X_ALIGN;
    pstOutRect->s32Y = oy & MT_WIN_OUT_RECT_Y_ALIGN;
    pstOutRect->s32Width = ow & MT_WIN_OUT_RECT_WIDTH_ALIGN;
    pstOutRect->s32Height = oh & MT_WIN_OUT_RECT_HEIGHT_ALIGN;

    return bDispFlag;
}


#define DEBUG_TEST 1

#if DEBUG_TEST
typedef struct
{
    mt_u32  u32WinNum;
    MT_BOOL bHorSrEnable;
    MT_BOOL bVerSrEnable;

    MT_BOOL  bZmeCapability;
    MT_BOOL  bSrCapability;

    mt_u32   u32LayerMAXWidth;
    mt_u32   u32LayerMAXHeight;
    mt_rect_s stVpssZmeSize;
    mt_rect_s stFinalZmeSize;
    mt_rect_s stSourceFrameRect;
} sr_struct_s;

//printk("expect:%d,%d,%d,%d, actual:%d,%d,%d,%d!\n", a,b,w,h,sr_test.bHorSrEnable,sr_test.bVerSrEnable,sr_test.stVpssZmeSize.s32Width,sr_test.stVpssZmeSize.s32Height);
#define check_return_value(sr_test, a,b,w,h) ({ \
        MT_BOOL bValue;\
        bValue = ((sr_test.bHorSrEnable != a) \
                  || (sr_test.bVerSrEnable != b) \
                  || (sr_test.stVpssZmeSize.s32Width != w) \
                  || (sr_test.stVpssZmeSize.s32Height != h)); \
        bValue; \
    })

#define check_return_value_post(post_test, a,b,x,y,w,h) ({ \
        MT_BOOL bValue;\
        bValue = ((post_test.bHorSrEnable != a) \
                  || (post_test.bVerSrEnable != b) \
                  || (post_test.stV0DisPosition.s32X != x) \
                  || (post_test.stV0DisPosition.s32Y != y) \
                  || (post_test.stV0DisPosition.s32Width != w) \
                  || (post_test.stV0DisPosition.s32Height != h)); \
        bValue; \
    })

//printk("expect:%d,%d,%d,%d,%d, actual:%d,%d,%d,%d,%d!\n", a,x,y,w,h,dci_test.bDciEnable,dci_test.stActualDciArea.u16HStar,dci_test.stActualDciArea.u16VStar,dci_test.stActualDciArea.u16HEnd - dci_test.stActualDciArea.u16HStar, dci_test.stActualDciArea.u16VEnd - dci_test.stActualDciArea.u16VStar);
#define check_return_value_dci(dci_test, a,x,y,w,h) ({ \
        MT_BOOL bValue;\
        bValue = ((dci_test.bDciEnable != a) \
                  || (dci_test.stActualDciArea.u16HStar != x) \
                  || (dci_test.stActualDciArea.u16VStar != y) \
                  || ((dci_test.stActualDciArea.u16HEnd - dci_test.stActualDciArea.u16HStar)  != w) \
                  || ((dci_test.stActualDciArea.u16VEnd - dci_test.stActualDciArea.u16VStar) != h)); \
        bValue; \
    })

mt_void  memset_SR(mt_rect_s* pstRect, mt_u32 u32X, mt_u32 u32Y, mt_u32 u32Width, mt_u32 u32Height)
{
    pstRect->s32Width = u32Width;
    pstRect->s32Height = u32Height;

    pstRect->s32X = u32X;
    pstRect->s32Y = u32Y;

}


mt_s32 call_sr_func(sr_struct_s* sr_test)
{
#if 0
    return pre_ScalerDistribute(&sr_test->stVpssZmeSize,
                                &sr_test->stFinalZmeSize,
                                sr_test->u32WinNum,
                                &sr_test->bHorSrEnable,
                                &sr_test->bVerSrEnable,
                                sr_test->bZmeCapability,
                                sr_test->bSrCapability,
                                sr_test->u32LayerMAXWidth,
                                sr_test->u32LayerMAXHeight,
                                &sr_test->stSourceFrameRect,
                                MT_NULL,
                                MT_FALSE,
                                DISP_STEREO_NONE);
#endif

    if (sr_test)
    {
        return MT_SUCCESS;
    }
    else
    {
        return MT_FAILURE;
    }
}


mt_s32 Win_TestSR(mt_void)
{
    mt_s32  s32Ret = 0;
    sr_struct_s sr_test;

    memset((void*)&sr_test, 0, sizeof(sr_struct_s));
    sr_test.bZmeCapability = MT_TRUE;
    sr_test.bSrCapability = MT_TRUE;
    sr_test.u32LayerMAXWidth = 5000;
    sr_test.u32LayerMAXHeight = 5000;

    /*1. win== 2,even 1/2,sr not opened.*/
    sr_test.u32WinNum = 2;
    memset_SR(&sr_test.stFinalZmeSize, 0, 0, 360, 288);
    memset_SR(&sr_test.stSourceFrameRect, 0, 0, 180, 144);
    if ((s32Ret = call_sr_func(&sr_test)))
    {
        WIN_ERROR("sr test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value(sr_test, MT_FALSE, MT_FALSE, 360, 288))
        {
            WIN_ERROR("sr test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    /*2. win== 2,even less 1/2,sr not opened.*/
    sr_test.u32WinNum = 2;
    memset_SR(&sr_test.stFinalZmeSize, 0, 0, 360, 288);
    memset_SR(&sr_test.stSourceFrameRect, 0, 0, 176, 140);
    if ((s32Ret = call_sr_func(&sr_test)))
    {
        WIN_ERROR("sr test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value(sr_test, MT_FALSE, MT_FALSE, 360, 288))
        {
            WIN_ERROR("sr test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    /*3. win== 2, large 1/2,sr not opened.*/
    sr_test.u32WinNum = 2;
    memset_SR(&sr_test.stFinalZmeSize, 0, 0, 360, 288);
    memset_SR(&sr_test.stSourceFrameRect, 0, 0, 184, 148);
    if ((s32Ret = call_sr_func(&sr_test)))
    {
        WIN_ERROR("sr test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value(sr_test, MT_FALSE, MT_FALSE, 360, 288))
        {
            WIN_ERROR("sr test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    /*4. win== 1, 1/2,sr opened.*/
    sr_test.u32WinNum = 1;
    memset_SR(&sr_test.stFinalZmeSize, 0, 0, 360, 288);
    memset_SR(&sr_test.stSourceFrameRect, 0, 0, 180, 144);
    if ((s32Ret = call_sr_func(&sr_test)))
    {
        WIN_ERROR("sr test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value(sr_test, MT_TRUE, MT_TRUE, 180, 144))
        {
            WIN_ERROR("sr test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    /*5. win== 1, less 1/2,sr opened.*/
    sr_test.u32WinNum = 1;
    memset_SR(&sr_test.stFinalZmeSize, 0, 0, 360, 288);
    memset_SR(&sr_test.stSourceFrameRect, 0, 0, 176, 140);
    if ((s32Ret = call_sr_func(&sr_test)))
    {
        WIN_ERROR("sr test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value(sr_test, MT_TRUE, MT_TRUE, 180, 144))
        {
            WIN_ERROR("sr test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    /*6. win== 1, height less  1/2,sr opened.*/
    sr_test.u32WinNum = 1;
    memset_SR(&sr_test.stFinalZmeSize, 0, 0, 360, 288);
    memset_SR(&sr_test.stSourceFrameRect, 0, 0, 184, 140);
    if ((s32Ret = call_sr_func(&sr_test)))
    {
        WIN_ERROR("sr test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value(sr_test, MT_FALSE, MT_TRUE, 360, 144))
        {
            WIN_ERROR("sr test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    /*7. win== 1, width less  1/2,sr opened.*/
    sr_test.u32WinNum = 1;
    memset_SR(&sr_test.stFinalZmeSize, 0, 0, 360, 288);
    memset_SR(&sr_test.stSourceFrameRect, 0, 0, 176, 148);
    if ((s32Ret = call_sr_func(&sr_test)))
    {
        WIN_ERROR("sr test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value(sr_test, MT_TRUE, MT_FALSE, 180, 288))
        {
            WIN_ERROR("sr test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    /*8. win== 1, both larger then  1/2,sr opened.*/
    sr_test.u32WinNum = 1;
    memset_SR(&sr_test.stFinalZmeSize, 0, 0, 360, 288);
    memset_SR(&sr_test.stSourceFrameRect, 0, 0, 188, 148);
    if ((s32Ret = call_sr_func(&sr_test)))
    {
        WIN_ERROR("sr test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value(sr_test, MT_FALSE, MT_FALSE, 360, 288))
        {
            WIN_ERROR("sr test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    /*9. win== 1, both equal,sr opened.*/
    sr_test.u32WinNum = 1;
    memset_SR(&sr_test.stFinalZmeSize, 0, 0, 360, 288);
    memset_SR(&sr_test.stSourceFrameRect, 0, 0, 360, 288);
    if ((s32Ret = call_sr_func(&sr_test)))
    {
        WIN_ERROR("sr test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value(sr_test, MT_FALSE, MT_FALSE, 360, 288))
        {
            WIN_ERROR("sr test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    /*a. win== 1, both larger than outrect,sr opened.*/
    sr_test.u32WinNum = 1;
    memset_SR(&sr_test.stFinalZmeSize, 0, 0, 360, 288);
    memset_SR(&sr_test.stSourceFrameRect, 0, 0, 366, 292);
    if ((s32Ret = call_sr_func(&sr_test)))
    {
        WIN_ERROR("sr test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value(sr_test, MT_FALSE, MT_FALSE, 360, 288))
        {
            WIN_ERROR("sr test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }


    /*a. win== 1, both larger than outrect,sr opened.*/
    sr_test.u32WinNum = 1;
    memset_SR(&sr_test.stFinalZmeSize, 0, 0, 3840, 2160);
    memset_SR(&sr_test.stSourceFrameRect, 0, 0, 2000, 1070);
    if ((s32Ret = call_sr_func(&sr_test)))
    {
        WIN_ERROR("sr test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value(sr_test, MT_FALSE, MT_FALSE, 3840, 2160))
        {
            WIN_ERROR("sr test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    MT_INFO_VO("test success!\n");
    return MT_SUCCESS;
}

typedef struct
{
    mt_rect_s stFinalDisPosition;
    mt_rect_s stV0DisPosition;
    MT_BOOL   bHorSrEnable;
    MT_BOOL   bVerSrEnable;
    mt_u32    u32WinNum;
    mt_rect_s stSourceFrameRect;
    MT_CHIP_TYPE_E   enChipType;
    MT_BOOL bSrCapability;
} post_test_s;

typedef struct
{
    mt_rect_s stOriginDciArea;
    mt_rect_s stV0DisPosition;
    mt_rect_s stVpssGive;
    mt_u32    u32WinNum;
    MT_BOOL   bDciEnable;
    MT_PQ_DCI_WIN_S stActualDciArea;
} dci_test_s;

mt_s32 call_post_func(post_test_s* post_test)
{
#if 0
    return post_ScalerProcess(&post_test->stFinalDisPosition,
                              &post_test->stV0DisPosition,
                              &post_test->bHorSrEnable,
                              &post_test->bVerSrEnable,
                              post_test->u32WinNum,
                              &post_test->stSourceFrameRect,
                              post_test->enChipType,
                              post_test->bSrCapability,
                              MT_NULL,
                              DISP_STEREO_NONE);
#endif

    if (post_test)
        return MT_SUCCESS;
    else
        return MT_FAILURE;
}

mt_s32 call_dci_func(dci_test_s* dci_test)
{
    return dci_enable_policy(&dci_test->stOriginDciArea,
                             &dci_test->stV0DisPosition,
                             &dci_test->stVpssGive,
                             dci_test->u32WinNum,
                             &dci_test->bDciEnable,
                             &dci_test->stActualDciArea);
}

mt_s32 Win_TestPost(mt_void)
{

    post_test_s post_test;
    mt_s32 s32Ret = 0;

    memset((void*)&post_test, 0, sizeof(post_test_s));

    post_test.bSrCapability = MT_TRUE;

    /*1. 2 WIN, less than 1/2.*/
    post_test.u32WinNum = 2;
    memset_SR(&post_test.stFinalDisPosition, 24, 36, 360, 288);
    memset_SR(&post_test.stSourceFrameRect, 0, 0, 170, 136);
    if ((s32Ret = call_post_func(&post_test)))
    {
        WIN_ERROR("post test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_post(post_test, MT_FALSE, MT_FALSE, 24, 36, 360, 288))
        {
            WIN_ERROR("post test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    /*2. 2 WIN, == 1/2.*/
    post_test.u32WinNum = 2;
    memset_SR(&post_test.stFinalDisPosition, 24, 36, 360, 288);
    memset_SR(&post_test.stSourceFrameRect, 0, 0, 180, 144);
    if ((s32Ret = call_post_func(&post_test)))
    {
        WIN_ERROR("post test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_post(post_test, MT_FALSE, MT_FALSE, 24, 36, 360, 288))
        {
            WIN_ERROR("post test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }


    /*3. 2 WIN, > 1/2.*/
    post_test.u32WinNum = 2;
    memset_SR(&post_test.stFinalDisPosition, 24, 36, 360, 288);
    memset_SR(&post_test.stSourceFrameRect, 0, 0, 190, 160);
    if ((s32Ret = call_post_func(&post_test)))
    {
        WIN_ERROR("post test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_post(post_test, MT_FALSE, MT_FALSE, 24, 36, 360, 288))
        {
            WIN_ERROR("post test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    /*4. 1 WIN, == 1/2.*/
    post_test.u32WinNum = 1;
    memset_SR(&post_test.stFinalDisPosition, 24, 36, 360, 288);
    memset_SR(&post_test.stSourceFrameRect, 0, 0, 180, 144);
    if ((s32Ret = call_post_func(&post_test)))
    {
        WIN_ERROR("post test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_post(post_test, MT_TRUE, MT_TRUE, 12, 18, 180, 144))
        {
            WIN_ERROR("post test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }


    /*5. 1 WIN, width ==  1/2.*/
    post_test.u32WinNum = 1;
    memset_SR(&post_test.stFinalDisPosition, 24, 36, 360, 288);
    memset_SR(&post_test.stSourceFrameRect, 0, 0, 180, 146);
    if ((s32Ret = call_post_func(&post_test)))
    {
        WIN_ERROR("post test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_post(post_test, MT_TRUE, MT_FALSE, 12, 36, 180, 288))
        {
            WIN_ERROR("post test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    /*6. 1 WIN, height ==  1/2.*/
    post_test.u32WinNum = 1;
    memset_SR(&post_test.stFinalDisPosition, 24, 36, 360, 288);
    memset_SR(&post_test.stSourceFrameRect, 0, 0, 190, 144);
    if ((s32Ret = call_post_func(&post_test)))
    {
        WIN_ERROR("post test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_post(post_test, MT_FALSE, MT_TRUE, 24, 18, 360, 144))
        {
            WIN_ERROR("post test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }


    /*7. 1 WIN, both <=  1/2.*/
    post_test.u32WinNum = 1;
    memset_SR(&post_test.stFinalDisPosition, 24, 36, 360, 288);
    memset_SR(&post_test.stSourceFrameRect, 0, 0, 148, 140);
    if ((s32Ret = call_post_func(&post_test)))
    {
        WIN_ERROR("post test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_post(post_test, MT_TRUE, MT_TRUE, 12, 18, 180, 144))
        {
            WIN_ERROR("post test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }


    /*8. 1 WIN, both >=  1/2.*/
    post_test.u32WinNum = 1;
    memset_SR(&post_test.stFinalDisPosition, 24, 36, 360, 288);
    memset_SR(&post_test.stSourceFrameRect, 0, 0, 190, 148);
    if ((s32Ret = call_post_func(&post_test)))
    {
        WIN_ERROR("post test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_post(post_test, MT_FALSE, MT_FALSE, 24, 36, 360, 288))
        {
            WIN_ERROR("post test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }


    MT_INFO_VO("test success!\n");



    return MT_SUCCESS;
}


mt_s32 Win_TestDci(mt_void)
{
    dci_test_s dci_test;
    mt_s32 s32Ret = 0;

    memset((void*)&dci_test, 0, sizeof(dci_test_s));

    /*1. 2 WIN*/
    dci_test.u32WinNum = 2;
    memset_SR(&dci_test.stOriginDciArea, 24, 36, 360, 288);
    memset_SR(&dci_test.stVpssGive, 0, 0, 500, 400);
    memset_SR(&dci_test.stV0DisPosition, 0, 0, 500, 400);
    if ((s32Ret = call_dci_func(&dci_test)))
    {
        WIN_ERROR("dci test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_dci(dci_test, MT_FALSE, 0, 0, 0, 0))
        {
            WIN_ERROR("dci test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }
    }

    /*2. 2 WIN */
    dci_test.u32WinNum = 3;
    memset_SR(&dci_test.stOriginDciArea, 24, 36, 360, 288);
    memset_SR(&dci_test.stVpssGive, 0, 0, 500, 400);
    memset_SR(&dci_test.stV0DisPosition, 0, 0, 500, 400);
    if ((s32Ret = call_dci_func(&dci_test)))
    {
        WIN_ERROR("dci test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_dci(dci_test, MT_FALSE, 0, 0, 0, 0))
        {
            WIN_ERROR("dci test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }

    }

    /*3. 1 WIN, vpss == vdp0 */
    dci_test.u32WinNum = 1;
    memset_SR(&dci_test.stOriginDciArea, 24, 36, 360, 288);
    memset_SR(&dci_test.stVpssGive, 0, 0, 500, 400);
    memset_SR(&dci_test.stV0DisPosition, 0, 0, 500, 400);
    if ((s32Ret = call_dci_func(&dci_test)))
    {
        WIN_ERROR("dci test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_dci(dci_test, MT_TRUE, 24, 36, 360, 288))
        {
            WIN_ERROR("dci test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }

    }

    /*4. 1 WIN vpss_w == vdp0/2*/
    dci_test.u32WinNum = 1;
    memset_SR(&dci_test.stOriginDciArea, 24, 36, 160, 150);
    memset_SR(&dci_test.stVpssGive, 0, 0, 250, 400);
    memset_SR(&dci_test.stV0DisPosition, 0, 0, 500, 400);
    if ((s32Ret = call_dci_func(&dci_test)))
    {
        WIN_ERROR("dci test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_dci(dci_test, MT_TRUE, 48, 36, 320, 150))
        {
            WIN_ERROR("dci test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }

    }

    /*5. 1 WIN  vpss_h == vdp0/2*/
    dci_test.u32WinNum = 1;
    memset_SR(&dci_test.stOriginDciArea, 24, 36, 160, 150);
    memset_SR(&dci_test.stVpssGive, 0, 0, 500, 200);
    memset_SR(&dci_test.stV0DisPosition, 0, 0, 500, 400);
    if ((s32Ret = call_dci_func(&dci_test)))
    {
        WIN_ERROR("dci test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_dci(dci_test, MT_TRUE, 24, 72, 160, 300))
        {
            WIN_ERROR("dci test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }

    }


    /*6. 1 WIN  vpss_wh == vdp0/2*/
    dci_test.u32WinNum = 1;
    memset_SR(&dci_test.stOriginDciArea, 24, 36, 160, 150);
    memset_SR(&dci_test.stVpssGive, 0, 0, 250, 200);
    memset_SR(&dci_test.stV0DisPosition, 0, 0, 500, 400);
    if ((s32Ret = call_dci_func(&dci_test)))
    {
        WIN_ERROR("dci test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_dci(dci_test, MT_TRUE, 48, 72, 320, 300))
        {
            WIN_ERROR("dci test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }

    }


    /*7. 1 WIN  vpss_wh == vdp0/2*/
    dci_test.u32WinNum = 1;
    memset_SR(&dci_test.stOriginDciArea, 24, 36, 160, 150);
    memset_SR(&dci_test.stVpssGive, 0, 0, 250, 300);
    memset_SR(&dci_test.stV0DisPosition, 32, 40, 500, 600);
    if ((s32Ret = call_dci_func(&dci_test)))
    {
        WIN_ERROR("dci test error:%d!\n", __LINE__);
        return MT_FAILURE;
    }
    else
    {
        if (check_return_value_dci(dci_test, MT_TRUE, 80, 112, 320, 300))
        {
            WIN_ERROR("dci test error:%d!\n", __LINE__);
            return MT_FAILURE;
        }

    }

    MT_INFO_VO("test success!\n");
    return MT_SUCCESS;
}

#endif

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */




