/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h> /* mmap */
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/types.h>
#include "mt_drv_tde.h"
#include "mt_tde_api.h"
#include "mt_debug.h"

#include "gpe.h"
#include <pthread.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define TDE_CHECK_FD()                      \
    do {                                    \
		if (0 == g_s32TDeRef) {             \
		    return MT_ERR_TDE_DEV_NOT_OPEN; \
		}                                   \
    } while (0)

static mt_s32 g_s32TDeRef = 0;

static pthread_mutex_t gpe_mutex;

ulong gpe_reg_virt_addr = 0;

#if 0
#define MT_TDE2_FUN_IN printf("%s, LINE IN: %d\n", __FUNCTION__, __LINE__)
#define MT_TDE2_FUN_OUT printf("%s, LINE OUT: %d\n", __FUNCTION__, __LINE__)
#define MT_TDE2_LOG printf
#define MT_TDE2_LINE printf("%s, LINE: %d\n", __FUNCTION__, __LINE__)
#else
#define DUMP_LOG \
    do {         \
    } while (0)
#define MT_TDE2_FUN_IN 		DUMP_LOG
#define MT_TDE2_FUN_OUT 	DUMP_LOG
#define MT_TDE2_LOG(...) 	DUMP_LOG
#define MT_TDE2_LINE 		DUMP_LOG
#endif

#define MT_TDE2_RETURN()   	\
	{                   	\
		return MT_SUCCESS; 	\
	}

#if defined(CONFIG_MT_CHIP_ARIA)
static phys_addr_t s_tde_reg_phy_addr = 0xffd60000;
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
static phys_addr_t s_tde_reg_phy_addr = 0x1f430000;
#endif

mt_s32 MT_TDE2_Open(mt_void)
{
    MT_TDE2_FUN_IN;
	//MT_TDE2_RETURN();

	if (g_s32TDeRef++ != 0)
	{
		return MT_SUCCESS;
	}

	pthread_mutex_init(&gpe_mutex, NULL);

	//+++
    gpe_reg_virt_addr = (ulong)mt_mmap(s_tde_reg_phy_addr, 0x400);
    if ((void *)gpe_reg_virt_addr == MAP_FAILED) {
		MT_TDE2_LOG("[VDI] fail to map vpu registers \n");
		return MT_ERR_TDE_DEV_OPEN_FAILED;
    }

    MT_TDE2_LOG("MT_TDE2_Open: reg base = 0x%x \n", gpe_reg_virt_addr);

    GpeOsiScaleCoeff_New();

    return MT_SUCCESS;
}

mt_void MT_TDE2_Close(mt_void)
{
    MT_TDE2_FUN_IN;
	//MT_TDE2_RETURN();

	if (g_s32TDeRef == 0)
	{
		return;
	}

	g_s32TDeRef--;

	if (g_s32TDeRef != 0)
	{
		return;
	}

    if (gpe_reg_virt_addr != 0) {
	    //+++
		mt_munmap(gpe_reg_virt_addr);
		gpe_reg_virt_addr = 0;
		MT_TDE2_LOG("MT_TDE2_Close: munmap ok\n");
    }

    GpeOsiScaleCoeff_Del();
	pthread_mutex_destroy(&gpe_mutex);

    return;
}

TDE_HANDLE MT_TDE2_BeginJob(mt_void)
{
    TDE_HANDLE s32Handle;

    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();

	pthread_mutex_lock(&gpe_mutex);

    if (GpeOsiBeginJob(&s32Handle) < 0) {
		//for bug 104373
		pthread_mutex_destroy(&gpe_mutex);
		return MT_ERR_TDE_INVALID_HANDLE;
    }

    return s32Handle;
}

mt_s32 MT_TDE_Create_Mid_Mem(mt_u32 mem_size)
{
	return gpe_create_mid_memory(mem_size);
}

mt_s32 MT_TDE_Destroy_Mid_Mem(mt_void)
{
	return gpe_destroy_mid_memory();
}

mt_s32 MT_TDE2_Bitblit(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect,
                       TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect, TDE2_SURFACE_S *pstDst,
                       TDE2_RECT_S *pstDstRect,
                       TDE2_OPT_S *pstOpt)
{
    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();

    return GpeOsiBlit(s32Handle, pstBackGround, pstBackGroundRect, pstForeGround, pstForeGroundRect, pstDst, pstDstRect, pstOpt);
}

mt_s32 MT_TDE2_Bitblit_3src(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect,
                       TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect,
                       TDE2_SURFACE_S *pstExGround, TDE2_RECT_S *pstExGroundRect,
                       TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                       TDE2_OPT_S *pstOpt)
{
    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();

    return GpeOsiBlit_3src(s32Handle, pstBackGround, pstBackGroundRect,
                			pstForeGround, pstForeGroundRect, pstExGround,
                			pstExGroundRect, pstDst, pstDstRect, pstOpt);
}

mt_s32 MT_TDE2_QuickCopy(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstSrc, TDE2_RECT_S *pstSrcRect,
                         TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect)
{
    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();

    if ((NULL == pstSrc) || (NULL == pstSrcRect) || (NULL == pstDst) || (NULL == pstDstRect)) {
		return MT_ERR_TDE_NULL_PTR;
    }

    return GpeOsiQuickCopy(s32Handle, pstSrc, pstSrcRect, pstDst, pstDstRect);
}

/*****************************************************************************
* Function:      MT_TDE2_QuickFill
* Description:   Fill quickly fixed value to target bitmap
* Input:         s32Handle: Task handle
*                pDst: Target bitmap info struct
*                u32FillData: Fill information, pixel format accord with target bitmap
* Output:        none
* Return:        >0: current task id; <0: Failure
* Others:        none
*****************************************************************************/
mt_s32 MT_TDE2_QuickFill(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                         mt_u32 u32FillData)
{
    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();

    if ((NULL == pstDst) || (NULL == pstDstRect)) {
		return MT_ERR_TDE_NULL_PTR;
    }

    return GpeOsiQuickFill(s32Handle, pstDst, pstDstRect, u32FillData);
}

/*****************************************************************************
* Function:      MT_TDE2_QuickResize
* Description:   Zoom source bitmap to the size fixed by target bitmap,source and target bitmap can be the same
* Input:         s32Handle: Task handle
*                pSrc: Source bitmap info struct
*                pDst: Target bitmap info struct
* Output:        None
* Return:        >0: current task id; <0: Faiure
* Others:        None
*****************************************************************************/
mt_s32 MT_TDE2_QuickResize(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstSrc, TDE2_RECT_S *pstSrcRect,
                           TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect)
{
    MT_TDE2_FUN_IN;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();

    if ((NULL == pstSrc) || (NULL == pstSrcRect) || (NULL == pstDst) || (NULL == pstDstRect)) {
		return MT_ERR_TDE_NULL_PTR;
    }

    return GpeOsiQuickResize(s32Handle, pstSrc, pstSrcRect, pstDst, pstDstRect);
}

/*****************************************************************************
* Function:      MT_TDE2_QuickFlicker
* Description:   Deflicker source bitmap and export to target bitmap, source and target bitmap can be the same
* Input:         s32Handle: Task handle
*                pSrc: Source bitmap info struct
*                pDst: Target bitmap info struct
* Output:        None
* Return:        >0: Current Task id; <0: Failure
* Others:        None
*****************************************************************************/
//+++ not used
mt_s32 MT_TDE2_QuickDeflicker(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstSrc, TDE2_RECT_S *pstSrcRect,
                              TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect)

{
	//TODO...
	MT_TDE2_RETURN();
}

/*****************************************************************************
* Function:      MT_TDE2_SolidDraw
* Description:   Operate src1 with src2 and export the result to pDst,set operation in pOpt
*                If src bitmap is mb format, it just support single source, which say is only set either pSrc1 or pSrc2
* Input:         s32Handle: Task handle
*                pSrc: source1 bitmap info struct
*                pstDst: target bitmap information struct
*                pstFillColor:  target bitmap info struct
*                pstOpt:  operate argument setting struct
* Output:        none
* Return:        >0: return task id of current operate; <0: fail
* Others:        none
*****************************************************************************/
//+++ not used
mt_s32 MT_TDE2_SolidDraw(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect,
                         TDE2_SURFACE_S *pstDst,
                         TDE2_RECT_S *pstDstRect, TDE2_FILLCOLOR_S *pstFillColor,
                         TDE2_OPT_S *pstOpt)
{
	//TODO...
	MT_TDE2_RETURN();
}

mt_s32 MT_TDE2_Draw_3d_Trapez(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect,
                         TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                         TDE2_TRAPEZ_OPT_S *pstOpt)
{
    MT_TDE2_FUN_IN;

    TDE_CHECK_FD();

    return GpeOsiDraw_3d_trapez(s32Handle, pstForeGround, pstForeGroundRect, pstDst, pstDstRect, pstOpt);
}

mt_s32 ADP_TDEMBSurfaceToTDESurface(TDE2_MB_S *pTDEMBSurface, TDE2_SURFACE_S *pTDESurface)
{
    mt_s32 s32Ret;

	switch(pTDEMBSurface->enMbFmt)
	{
	case TDE2_MB_COLOR_FMT_JPG_YCbCr400MBP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_JPG_YCbCr400MBP;
		break;
	case TDE2_MB_COLOR_FMT_JPG_YCbCr422MBHP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_JPG_YCbCr422MBHP;
		break;
	case TDE2_MB_COLOR_FMT_JPG_YCbCr422MBVP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_JPG_YCbCr422MBVP;
		break;
	case TDE2_MB_COLOR_FMT_MP1_YCbCr420MBP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_MP1_YCbCr420MBP;
		break;
	case TDE2_MB_COLOR_FMT_MP2_YCbCr420MBP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_MP2_YCbCr420MBP;
		break;
	case TDE2_MB_COLOR_FMT_MP2_YCbCr420MBI:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_MP2_YCbCr420MBI;
		break;
	case TDE2_MB_COLOR_FMT_JPG_YCbCr420MBP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_JPG_YCbCr420MBP;
		break;
	case TDE2_MB_COLOR_FMT_JPG_YCbCr444MBP:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_JPG_YCbCr444MBP;
		break;
	case TDE2_MB_COLOR_FMT_JPG_SP_CMYK:
		pTDESurface->enColorFmt = TDE2_COLOR_FMT_JPG_SP_CMYK;
		break;
	default:
		break;
	}

	pTDESurface->u32PhyAddr = pTDEMBSurface->u32YPhyAddr;
	pTDESurface->u32CbCrPhyAddr = pTDEMBSurface->u32CbCrPhyAddr;
	pTDESurface->u32Stride = pTDEMBSurface->u32YStride;
	pTDESurface->u32CbCrStride = pTDEMBSurface->u32CbCrStride;
    pTDESurface->u32Height = pTDEMBSurface->u32YHeight;
    pTDESurface->u32Width = pTDEMBSurface->u32YWidth;
    pTDESurface->u32Stride = pTDEMBSurface->u32YStride;

	pTDESurface->pu8ClutPhyAddr = NULL;
	pTDESurface->bYCbCrClut = MT_FALSE;

    /** below to be modified*/
    pTDESurface->bAlphaMax255 = MT_TRUE;
    pTDESurface->bAlphaExt1555 = MT_TRUE;
    pTDESurface->u8Alpha0 = 0;
    pTDESurface->u8Alpha1 = 255;
    return MT_SUCCESS;
}

mt_s32 ADP_TDEMBOptToTDEOpt(TDE2_MBOPT_S *pStMbOpt, TDE2_OPT_S *pStOpt)
{
	pStOpt->bResize = pStMbOpt->enResize;
	pStOpt->enClipMode = pStMbOpt->enClipMode;
	pStOpt->enDeflickerMode = pStMbOpt->bDeflicker;
	if(pStMbOpt->bSetOutAlpha)
	{
		pStOpt->enOutAlphaFrom = TDE2_OUTALPHA_FROM_GLOBALALPHA;
		pStOpt->u8GlobalAlpha = pStMbOpt->u8OutAlpha;
	}
	else
	{
		pStOpt->enOutAlphaFrom = TDE2_OUTALPHA_FROM_GLOBALALPHA;
		pStOpt->u8GlobalAlpha = 0xff;
	}
	pStOpt->enClipMode = pStMbOpt->enClipMode;
	pStOpt->stClipRect.s32Xpos = pStMbOpt->stClipRect.s32Xpos;
	pStOpt->stClipRect.s32Ypos = pStMbOpt->stClipRect.s32Ypos;
	pStOpt->stClipRect.u32Width = pStMbOpt->stClipRect.u32Width;
	pStOpt->stClipRect.u32Height = pStMbOpt->stClipRect.u32Height;
	return MT_SUCCESS;
}

/*****************************************************************************
* Function:      MT_TDE2_MbBlit
* Description:   MB blit interface
* Input:         s32Handle: task handle
*                pY:    brightness info struct
*                pCbCr: chroma information struct
*                pDst:  target bitmap inforamtion struct
*                pMbOpt: operate argument setting struct
* Output:        none
* Return:        >0: return task id of current operate; <0: fail
* Others:        none
*****************************************************************************/
mt_s32 MT_TDE2_MbBlit(TDE_HANDLE s32Handle, TDE2_MB_S *pstMB, TDE2_RECT_S *pstMbRect, TDE2_SURFACE_S *pstDst,
                      TDE2_RECT_S *pstDstRect,
                      TDE2_MBOPT_S *pstMbOpt)
{
	TDE2_OPT_S stOpt = {0};
	TDE2_SURFACE_S TDESurface;

    MT_TDE2_FUN_IN;
	//MT_TDE2_RETURN();

    TDE_CHECK_FD();

    if ((NULL == pstMB) || (NULL == pstDst) || (NULL == pstMbOpt)) {
		return MT_ERR_TDE_NULL_PTR;
    }

	ADP_TDEMBSurfaceToTDESurface(pstMB, &TDESurface);
	ADP_TDEMBOptToTDEOpt(pstMbOpt, &stOpt);

	return MT_TDE2_Bitblit(s32Handle, NULL, NULL,
	                       &TDESurface, pstMbRect, pstDst,
	                       pstDstRect, &stOpt);
}

/*****************************************************************************
* Function:      MT_TDE2_EndJob
* Description:   submit TDE2 task
* Input:         s32Handle: task handle
*                bSync: if synchronization
*                bBlock: if block
*                u32TimeOut: timeout value(unit by 10ms)
* Output:        none
* Return:        success/fail
* Others:        none
*****************************************************************************/
mt_s32 MT_TDE2_EndJob(TDE_HANDLE s32Handle, MT_BOOL bSync, MT_BOOL bBlock, mt_u32 u32TimeOut)
{
    MT_TDE2_FUN_IN;
	mt_s32 ret;
    //MT_TDE2_RETURN();

    TDE_CHECK_FD();
    /* Disable sync function */
    bSync = MT_FALSE;

    ret = GpeOsiEndJob(s32Handle, bBlock, u32TimeOut, bSync, MT_NULL, MT_NULL);
	pthread_mutex_unlock(&gpe_mutex);
	return ret;
}

/*****************************************************************************
* Function:      MT_TDE2_WaitForDone
* Description:   wait for completion of sumit TDE2 operate
* Input:         s32Handle: task handle
* Output:        none
* Return:        true: assigned task completed  / false: assigned task not completed
* Others:        none
*****************************************************************************/
//+++ not used
mt_s32 MT_TDE2_WaitForDone(TDE_HANDLE s32Handle)
{
	//TODO...
	MT_TDE2_RETURN();
}

/*****************************************************************************
* Function:      MT_TDE2_WaitAllDone
* Description:   wait for all TDE operate if all be done
* Input:         none
* Output:        none
* Return:        success / fail
* Others:        none
*****************************************************************************/
//+++ not used
mt_s32 MT_TDE2_WaitAllDone()
{
	//TODO...
	MT_TDE2_RETURN();
}

/*****************************************************************************
* Function:      MT_TDE2_Reset
* Description:   reset TDE all states
* Input:         none
* Output:        none
* Return:        success / fail
* Others:        none
*****************************************************************************/
//+++ not used
mt_s32 MT_TDE2_Reset(mt_void)
{
	//TODO...
	MT_TDE2_RETURN();
}

/*****************************************************************************
* Function:      MT_TDE2_CancelJob
* Description:   delete TDE2 tasks created, effective called before endjob
* Input:         s32Handle: task handle
* Output:        none
* Return:        success / fail
* Others:        none
*****************************************************************************/
//+++ not used
mt_s32 MT_TDE2_CancelJob(TDE_HANDLE s32Handle)
{
	//TODO...
	MT_TDE2_RETURN();
}

/*****************************************************************************
* Function:      MT_TDE2_BitmapMaskRop
* Description:   Ropmask source2 with mask bitmap at first, Ropmask source1 with middle bitmap at then
*                output result into target bitmap.
* Input:         s32Handle: task handle
*                pstBackGround: background bitmap information struct
*                pstBackGroundRect: background bitmap operate rect
*                pstForeGround: foreground bitmap information struct
*                pstForeGroundRect: foreground bitmap operate rect
*                pstMask: bitmap info struct of doing mask operate
*                pstMaskRect: bitmap operate rect of  mask operate
*                pstDst:  target bitmap information struct
*                pstDstRect:  target bitmap operate rect
* Output:        none
* Return:        >0: return task id of current operate; <0: fail
* Others:        none
*****************************************************************************/
//+++ not used ???
mt_s32 MT_TDE2_BitmapMaskRop(TDE_HANDLE s32Handle,
                             TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect,
                             TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect,
                             TDE2_SURFACE_S *pstMask, TDE2_RECT_S *pstMaskRect,
                             TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                             TDE2_ROP_CODE_E enRopCode_Color, TDE2_ROP_CODE_E enRopCode_Alpha)
{
	//TODO...
	MT_TDE2_LOG("[ERROR]: function %s not implement!\n",__FUNCTION__);
	return MT_ERR_TDE_UNSUPPORTED_OPERATION;
}

/*****************************************************************************
* Function:      MT_TDE2_BitmapMaskBlend
* Description:   blendmask source2 with mask bitmap at first, and then blend source1 with middle bitmap
*                put output result into target bitmap
* Input:         s32Handle: task handle
*                pstBackGround: background bitmap information struct
*                pstBackGroundRect: background bitamp operate rect
*                pstForeGround: foreground bitmap information struct
*                pstForeGroundRect: foreground bitamp operate rect
*                pstMask:  bitmap info struct of doing mask operate
*                pstMaskRect: bitmap operate rect of  mask operate
*                pstDst:  target bitmap information struct
*                pstDstRect:  target bitmap operate rect
*                u8Alpha:  alpha value is operated
* Output:        none
* Return:        >0: return task id of current operate; <0: fail
* Others:        none
*****************************************************************************/
//+++ not used ???
mt_s32 MT_TDE2_BitmapMaskBlend(TDE_HANDLE s32Handle,
                               TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect,
                               TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect,
                               TDE2_SURFACE_S *pstMask, TDE2_RECT_S *pstMaskRect,
                               TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                               mt_u8 u8Alpha, TDE2_ALUCMD_E enBlendMode)
{
	//TODO...
	MT_TDE2_LOG("[ERROR]: function %s not implement!\n",__FUNCTION__);
	return MT_ERR_TDE_UNSUPPORTED_OPERATION;
}

/*****************************************************************************
* Function:      MT_TDE2_ImageMultiply
* Description:   image multiply/stencil source1 bitmap at first, and then blend/rop source2 with dst bitmap
*                put output result into target bitmap
* Input:         s32Handle: task handle
*                pstBackGround: background bitmap information struct
*                pstBackGroundRect: background bitamp operate rect
*                pstForeGround: foreground bitmap information struct
*                pstForeGroundRect: foreground bitamp operate rect
*                pstPattern:  bitmap info struct of doing multiply operate
*                pstPatternRect: bitmap operate rect of  multiply operate
*                pstDst:  target bitmap information struct
*                pstDstRect:  target bitmap operate rect
*                pMultiplyOpt:  multiply option
* Output:        none
* Return:        >0: return task id of current operate; <0: fail
* Others:        none
*****************************************************************************/
//+++ not used
mt_s32 MT_TDE2_ImageMultiply(TDE_HANDLE s32Handle,
                             TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect,
                             TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect,
                             TDE2_SURFACE_S *pstPattern, TDE2_RECT_S *pstPatternRect,
                             TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                             TDE2_MULTIPLY_OPT_S *pMultiplyOpt)
{
	//TODO...
	MT_TDE2_RETURN();
}

//+++ not used
mt_s32 MT_TDE2_SetDeflickerLevel(TDE_DEFLICKER_LEVEL_E enDeflickerLevel)
{
	//TODO...
	MT_TDE2_RETURN();
}

//+++ not used
mt_s32 MT_TDE2_GetDeflickerLevel(TDE_DEFLICKER_LEVEL_E *pDeflickerLevel)
{
	//TODO...
	MT_TDE2_RETURN();
}

mt_s32 MT_TDE2_SetAlphaThresholdValue(mt_u8 u8ThresholdValue)
{
	//do nothing
	return MT_SUCCESS;
}

//+++ not used
mt_s32 MT_TDE2_GetAlphaThresholdValue(mt_u8 *pu8ThresholdValue)
{
	//TODO...
	MT_TDE2_RETURN();
}

mt_s32 MT_TDE2_SetAlphaThresholdState(MT_BOOL bEnAlphaThreshold)
{
	//do nothing
	return MT_SUCCESS;
}

//+++ not used
mt_s32 MT_TDE2_GetAlphaThresholdState(MT_BOOL *p_bEnAlphaThreshold)
{
	//TODO...
	MT_TDE2_RETURN();
}

//+++ not used
mt_s32 MT_TDE2_PatternFill(TDE_HANDLE s32Handle,
                           TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect,
                           TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect,
                           TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
                           TDE2_PATTERN_FILL_OPT_S *pstOpt)
{
	//TODO...
	MT_TDE2_RETURN();
}

//+++ not used
mt_s32 MT_TDE2_EnableRegionDeflicker(MT_BOOL bRegionDeflicker)
{
	//TODO...
	MT_TDE2_RETURN();
}

//+++ not used
mt_s32 MT_TDE2_MultiBlending(TDE_HANDLE s32Handle, TDE_SURFACE_LIST_S *pstSurfaceList)
{
	//TODO...
	MT_TDE2_RETURN();
}

mt_s32 MT_TDE2_VScreenCapture(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstDst,
                       TDE2_RECT_S *pstDstRect,
                       TDE2_OPT_S *pstOpt)
{

    return GpeOsiVideoScreenCapture(s32Handle, pstDst, pstDstRect, pstOpt);
}

mt_s32 MT_TDE2_ClipMask(TDE_HANDLE s32Handle,
										   TDE2_SURFACE_S *pstForeGround,
										   TDE2_SURFACE_S *pstDst,
										   TDE2_RECT_S *pstForeGroundRect,
										   TDE2_RECT_S *pstDstRect,
										   TDE2_MASK_OPT_E enMaskOpt)
{
	return GpeClipMask(s32Handle, pstForeGround, pstDst,
						pstForeGroundRect, pstDstRect, enMaskOpt);
}

mt_s32 MT_TDE2_RotateAribtraryAngle(TDE_HANDLE s32Handle,
                                        TDE2_SURFACE_S *pstSrc,
                                        TDE2_SURFACE_S *pstDst,
                                        TDE2_RECT_S *pstSrcRect,
                                        TDE2_POS_S *pDstPos,
                                        TDE2_ROTATOR_ANGLE_S *pAngle,
                                        TDE2_TRAPEZ_OPT_S *pOpt)
{
 	return GpeOsiRotateAribtraryAngle(s32Handle, pstSrc, pstDst, pstSrcRect,
                                        pDstPos, pAngle, pOpt);
}

mt_s32 MT_TDE2_ExpendColorkey(TDE2_FILLCOLOR_S *pstFillColor, TDE2_COLORKEY_U *pKeyValue)
{
	mt_u32 colorkey;

	colorkey = colorkey_expend(pstFillColor->enColorFmt, pstFillColor->u32FillColor);

	return getColorkeyMinMax(colorkey,pstFillColor->enColorFmt, pKeyValue);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

