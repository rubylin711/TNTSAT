/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#include "mt_module.h"
#include "mt_drv_module.h"
#include "drv_tde_ext.h"
#include "mtfb_drv.h"
#include "mtfb_comm.h"




static IntCallBack s_pTdeCallBack;


/**
 ** tde support wmtch color format
 **/
static MT_BOOL s_bTdeColFmt[MTFB_FMT_BUTT] = 
				{
				    MT_TRUE,    /* MTFB_FMT_RGB565 */ 
				    MT_TRUE,    /* MTFB_FMT_RGB888 */
				    MT_TRUE,    /* MTFB_FMT_KRGB444 */
				    MT_TRUE,    /* MTFB_FMT_KRGB555 */
				    
				    MT_FALSE,   /* MTFB_FMT_KRGB888 */
				    MT_TRUE,    /* MTFB_FMT_ARGB4444 */
				    MT_TRUE,    /* MTFB_FMT_ARGB1555 */
				    MT_TRUE,    /* MTFB_FMT_ARGB8888 */
				    
				    MT_TRUE,    /* MTFB_FMT_ARGB8565 */
				    MT_TRUE,   /* MTFB_FMT_RGBA4444 */
				    MT_TRUE,   /* MTFB_FMT_RGBA5551 */
				    MT_TRUE,   /* MTFB_FMT_RGBA5658 */
				    
				    MT_TRUE,   /* MTFB_FMT_RGBA8888 */
				    MT_TRUE,   /**< BGR565 */
				    MT_TRUE,   /**< BGR888 */
				    MT_TRUE,   /**< ABGR4444 */
				    
				    MT_TRUE,   /**< ABGR1555 */
				    MT_TRUE,   /**< ABGR8888 */
				    MT_TRUE,   /**< ABGR8565 */
				    MT_TRUE,   /**< BGR444 16bpp */
				    
				    MT_TRUE,   /**< BGR555 16bpp */
				    MT_TRUE,   /**< BGR888 32bpp */
				    MT_TRUE,   /* MTFB_FMT_1BPP */
				    MT_TRUE,   /* MTFB_FMT_2BPP */
				    
				    MT_TRUE,   /* MTFB_FMT_4BPP */
				    MT_TRUE,   /* MTFB_FMT_8BPP */
				    MT_TRUE,   /* MTFB_FMT_ACLUT44 */
				    MT_TRUE,   /* MTFB_FMT_ACLUT88 */
				    
				};

static TDE_EXPORT_FUNC_S *ps_TdeExportFuncs = MT_NULL;



/***************************************************************************************
* func          : MTFB_DRV_TdeOpen
* description   : CNcomment: TDE打开设备 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 MTFB_DRV_TdeOpen(mt_void)
{
    if(MT_NULL != ps_TdeExportFuncs)
    {
        return MT_SUCCESS;
    }
	/**
	 **获取TDE函数
	 **/
    if (MT_SUCCESS != mt_drv_module_getfunction(MT_ID_TDE, (mt_void**)&ps_TdeExportFuncs))
    {
    	return MT_FAILURE;
    }
    if(MT_NULL == ps_TdeExportFuncs)
    {
        MTFB_ERROR("Tde is not available!\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}


mt_s32 MTFB_DRV_TdeClose(mt_void)
{
	ps_TdeExportFuncs = MT_NULL;
    return MT_SUCCESS;
}


/***************************************************************************************
* func          : MTFB_DRV_TdeSupportFmt
* description   : CNcomment: 判断TDE是否支持该像素格式操作 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 MTFB_DRV_TdeSupportFmt(MTFB_COLOR_FMT_E fmt)
{
	if(MTFB_FMT_BUTT <= fmt)
	{
		return MT_FALSE;
	}
	return s_bTdeColFmt[fmt];
}

/***************************************************************************************
* func          : MTFB_DRV_ConvFmt
* description   : CNcomment:FB像素格式转成TDE像素格式 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
TDE2_COLOR_FMT_E MTFB_DRV_ConvFmt(MTFB_COLOR_FMT_E Fmt)
{
    switch (Fmt)
    {
        case MTFB_FMT_RGB233:
            return TDE2_COLOR_FMT_RGB233;
        case MTFB_FMT_RGB565:
            return TDE2_COLOR_FMT_RGB565;
        case MTFB_FMT_RGB888:
            return TDE2_COLOR_FMT_RGB24;
        case MTFB_FMT_KRGB444:
            return TDE2_COLOR_FMT_RGB444;
        case MTFB_FMT_KRGB555:
            return TDE2_COLOR_FMT_RGB555;
        case MTFB_FMT_KRGB888:
            return TDE2_COLOR_FMT_RGB888;
        case MTFB_FMT_ARGB4444:
            return TDE2_COLOR_FMT_ARGB4444;
        case MTFB_FMT_ARGB1555:
            return TDE2_COLOR_FMT_ARGB1555;
        case MTFB_FMT_ARGB8888:
            return TDE2_COLOR_FMT_ARGB8888;
        case MTFB_FMT_ARGB8565:
            return TDE2_COLOR_FMT_ARGB8565;
        case MTFB_FMT_BGR565: 
            return TDE2_COLOR_FMT_BGR565;           
        case MTFB_FMT_ABGR1555:
            return TDE2_COLOR_FMT_ABGR1555;  
        case MTFB_FMT_ABGR4444:
            return TDE2_COLOR_FMT_ABGR4444;  
        case MTFB_FMT_KBGR444:
            return TDE2_COLOR_FMT_BGR444;  
        case MTFB_FMT_KBGR555:
            return TDE2_COLOR_FMT_BGR555;  
        case MTFB_FMT_BGR888: 
            return TDE2_COLOR_FMT_BGR24;    
        case MTFB_FMT_ABGR8888:
            return TDE2_COLOR_FMT_ABGR8888;  
        case MTFB_FMT_ABGR8565:
            return TDE2_COLOR_FMT_ABGR8565;  
        case MTFB_FMT_KBGR888:  
            return TDE2_COLOR_FMT_BGR888;  
        case MTFB_FMT_1BPP:
            return TDE2_COLOR_FMT_CLUT1;
        case MTFB_FMT_2BPP:
            return TDE2_COLOR_FMT_CLUT2;
        case MTFB_FMT_4BPP:
            return TDE2_COLOR_FMT_CLUT4;
        case MTFB_FMT_8BPP:
            return TDE2_COLOR_FMT_CLUT8;
        case MTFB_FMT_ACLUT44:
            return TDE2_COLOR_FMT_ACLUT44;
        case MTFB_FMT_ACLUT88:
            return TDE2_COLOR_FMT_ACLUT88;
        default:
            return TDE2_COLOR_FMT_BUTT;
    }
}

MT_BOOL MTFB_IsTdeColorFmtClut(TDE2_COLOR_FMT_E enColorFmt)
{
	if (enColorFmt >= TDE2_COLOR_FMT_CLUT1
		&& enColorFmt <= TDE2_COLOR_FMT_ACLUT88)
	{
		return MT_TRUE;
	}
	
	return MT_FALSE;
}


/***************************************************************************************
* func			: MTFB_DRV_Blit
* description	: CNcomment: tde blit CNend\n
* param[in] 	: mt_void
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_s32 MTFB_DRV_Blit(MTFB_BUFFER_S *pSrcImg, MTFB_BUFFER_S *pDstImg,  MTFB_BLIT_OPT_S *pstOpt, MT_BOOL bRefreshScreen)
{
    mt_s32 s32Ret;
    TDE2_SURFACE_S stSrcSur = {0};
    TDE2_SURFACE_S stDstSur = {0};
    TDE2_RECT_S stSrcRect;
    TDE2_RECT_S stDstRect;
    TDE_HANDLE handle;
    TDE2_OPT_S stOpt = {0};    
    TDE_DEFLICKER_LEVEL_E enTdeDflkLevel;

    if(NULL == ps_TdeExportFuncs)
    {
        MTFB_ERROR("Tde is not available!\n");
        return MT_FAILURE;
    }

    /** confing src*/
    stSrcSur.u32PhyAddr = pSrcImg->stCanvas.u32PhyAddr;
    stSrcSur.u32Width = pSrcImg->stCanvas.u32Width;
    stSrcSur.u32Height = pSrcImg->stCanvas.u32Height;
    stSrcSur.u32Stride = pSrcImg->stCanvas.u32Pitch;
    stSrcSur.bAlphaMax255 = MT_TRUE;
    stSrcSur.bYCbCrClut = MT_FALSE;
    stSrcSur.enColorFmt = MTFB_DRV_ConvFmt(pSrcImg->stCanvas.enFmt);
    stSrcSur.u8Alpha0 = pstOpt->stAlpha.u8Alpha0;
    stSrcSur.u8Alpha1 = pstOpt->stAlpha.u8Alpha1;
    if (!((stSrcSur.u8Alpha0 == 0) && (stSrcSur.u8Alpha1 == 0)))
    {
        stSrcSur.bAlphaExt1555 = MT_TRUE;
    }

    stSrcRect.s32Xpos = pSrcImg->UpdateRect.x;
    stSrcRect.s32Ypos = pSrcImg->UpdateRect.y;
    stSrcRect.u32Width = pSrcImg->UpdateRect.w;
    stSrcRect.u32Height = pSrcImg->UpdateRect.h;

    /** confing dst*/
    stDstSur.u32PhyAddr = pDstImg->stCanvas.u32PhyAddr;
    stDstSur.u32Width = pDstImg->stCanvas.u32Width;
    stDstSur.u32Height = pDstImg->stCanvas.u32Height;
    stDstSur.u32Stride = pDstImg->stCanvas.u32Pitch;
    stDstSur.bAlphaMax255 = MT_TRUE;
    stDstSur.bYCbCrClut = MT_FALSE;
    stDstSur.enColorFmt = MTFB_DRV_ConvFmt(pDstImg->stCanvas.enFmt);
    stDstSur.u8Alpha0 = pstOpt->stAlpha.u8Alpha0;
    stDstSur.u8Alpha1 = pstOpt->stAlpha.u8Alpha1;

    stDstRect.s32Xpos = pDstImg->UpdateRect.x;
    stDstRect.s32Ypos = pDstImg->UpdateRect.y;
    stDstRect.u32Width = pDstImg->UpdateRect.w;
    stDstRect.u32Height = pDstImg->UpdateRect.h;


    stOpt.bResize = pstOpt->bScale;
    //if (((mt_u32)stSrcSur.enColorFmt >= (mt_u32)MTFB_FMT_1BPP) && ((mt_u32)stSrcSur.enColorFmt <= (mt_u32)MTFB_FMT_ACLUT88))
    if (MTFB_IsTdeColorFmtClut(stSrcSur.enColorFmt))
    {
        stOpt.bClutReload = MT_TRUE;
        stSrcSur.pu8ClutPhyAddr = pstOpt->u32CmapAddr;
        stDstSur.pu8ClutPhyAddr = pstOpt->u32CmapAddr;
    }

    switch(pstOpt->enAntiflickerLevel)
    {
        case MTFB_LAYER_ANTIFLICKER_NONE:
        {
            stOpt.enDeflickerMode = TDE2_DEFLICKER_MODE_NONE;
            enTdeDflkLevel = TDE_DEFLICKER_BUTT;
            break;
        }
        case MTFB_LAYER_ANTIFLICKER_LOW:
        {
            stOpt.enDeflickerMode = TDE2_DEFLICKER_MODE_BOTH;
            enTdeDflkLevel = TDE_DEFLICKER_LOW;
            break;
        }
        case MTFB_LAYER_ANTIFLICKER_MIDDLE:
        {
            stOpt.enDeflickerMode = TDE2_DEFLICKER_MODE_BOTH;
            enTdeDflkLevel = TDE_DEFLICKER_MIDDLE;
            break;
        }
        case MTFB_LAYER_ANTIFLICKER_HIGH:
        {
            stOpt.enDeflickerMode = TDE2_DEFLICKER_MODE_BOTH;
            enTdeDflkLevel = TDE_DEFLICKER_HIGH;
            break;
        }
        case MTFB_LAYER_ANTIFLICKER_AUTO:
        {
            stOpt.enDeflickerMode = TDE2_DEFLICKER_MODE_BOTH;
            enTdeDflkLevel = TDE_DEFLICKER_AUTO;
            break;
        }
        default:
		{
			stOpt.enDeflickerMode = TDE2_DEFLICKER_MODE_NONE;
			enTdeDflkLevel = TDE_DEFLICKER_BUTT;
            break;
		}

    }

    if(TDE_DEFLICKER_BUTT != enTdeDflkLevel)
    {
        ps_TdeExportFuncs->pfnTdeSetDeflickerLevel(enTdeDflkLevel);
    }

    if (pstOpt->stCKey.bKeyEnable)
    {
        //if (((MTFB_COLOR_FMT_E)(stSrcSur.enColorFmt) >= MTFB_FMT_1BPP) && ((MTFB_COLOR_FMT_E)(stSrcSur.enColorFmt) <= MTFB_FMT_ACLUT88))
		if (MTFB_IsTdeColorFmtClut(stSrcSur.enColorFmt))
        {
            stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_FOREGROUND;
            stOpt.unColorKeyValue.struCkClut.stAlpha.bCompIgnore = MT_TRUE;
            stOpt.unColorKeyValue.struCkClut.stClut.bCompOut = pstOpt->stCKey.u32KeyMode;
            stOpt.unColorKeyValue.struCkClut.stClut.u8CompMax = pstOpt->stCKey.u8BlueMax;
            stOpt.unColorKeyValue.struCkClut.stClut.u8CompMin = pstOpt->stCKey.u8BlueMin;
            stOpt.unColorKeyValue.struCkClut.stClut.u8CompMask = 0xff;
        }
        else
        {
            stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_FOREGROUND;
            stOpt.unColorKeyValue.struCkARGB.stAlpha.bCompIgnore = MT_TRUE;
            stOpt.unColorKeyValue.struCkARGB.stRed.u8CompMax = pstOpt->stCKey.u8RedMax;
            stOpt.unColorKeyValue.struCkARGB.stRed.u8CompMin = pstOpt->stCKey.u8RedMin;
            stOpt.unColorKeyValue.struCkARGB.stRed.bCompOut = pstOpt->stCKey.u32KeyMode;
            stOpt.unColorKeyValue.struCkARGB.stRed.u8CompMask = 0xff;
            
            stOpt.unColorKeyValue.struCkARGB.stGreen.u8CompMax = pstOpt->stCKey.u8GreenMax;
            stOpt.unColorKeyValue.struCkARGB.stGreen.u8CompMin = pstOpt->stCKey.u8GreenMin;
            stOpt.unColorKeyValue.struCkARGB.stGreen.bCompOut = pstOpt->stCKey.u32KeyMode;
            stOpt.unColorKeyValue.struCkARGB.stGreen.u8CompMask = 0xff;
            
            stOpt.unColorKeyValue.struCkARGB.stBlue.u8CompMax = pstOpt->stCKey.u8BlueMax;
            stOpt.unColorKeyValue.struCkARGB.stBlue.u8CompMin = pstOpt->stCKey.u8BlueMin;
            stOpt.unColorKeyValue.struCkARGB.stBlue.bCompOut = pstOpt->stCKey.u32KeyMode;
            stOpt.unColorKeyValue.struCkARGB.stBlue.u8CompMask = 0xff;
        }
    }
    stOpt.u8GlobalAlpha = 255;
    if (pstOpt->stAlpha.bAlphaEnable)
    {
        stOpt.enAluCmd = TDE2_ALUCMD_BLEND;
        stOpt.u8GlobalAlpha = pstOpt->stAlpha.u8GlobalAlpha;
        stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_NORM;
        stOpt.stBlendOpt.bGlobalAlphaEnable = MT_TRUE;
        stOpt.stBlendOpt.bPixelAlphaEnable = MT_TRUE;
        stOpt.stBlendOpt.bSrc1AlphaPremulti = MT_TRUE;
        stOpt.stBlendOpt.bSrc2AlphaPremulti = MT_TRUE;
        //stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_SRCOVER;
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_SRC;

    }
    else
    {
        stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_FOREGROUND;
    }

    s32Ret = ps_TdeExportFuncs->pfnTdeEnableRegionDeflicker(pstOpt->bRegionDeflicker);
    if (s32Ret != MT_SUCCESS)
    {
        MTFB_ERROR("enable region deflicker failed!\n");
        return s32Ret;
    }

    if (pstOpt->stClip.bClip)
    {
        stOpt.enClipMode = pstOpt->stClip.bInRegionClip ? TDE2_CLIPMODE_INSIDE : TDE2_CLIPMODE_OUTSIDE;
        stOpt.stClipRect.s32Xpos = pstOpt->stClip.stClipRect.x;
        stOpt.stClipRect.s32Ypos = pstOpt->stClip.stClipRect.y;
        stOpt.stClipRect.u32Width = pstOpt->stClip.stClipRect.w;
        stOpt.stClipRect.u32Height = pstOpt->stClip.stClipRect.h;
    }

    s32Ret = ps_TdeExportFuncs->pfnTdeBeginJob(&handle);
    if(s32Ret != MT_SUCCESS)
    {
        MTFB_ERROR("begin job failed\n");
        return s32Ret;
    }

    s32Ret = ps_TdeExportFuncs->pfnTdeBlit(handle, &stDstSur, &stDstRect, &stSrcSur, &stSrcRect, &stDstSur, \
                     &stDstRect, &stOpt);
    if(s32Ret != MT_SUCCESS)
    {
        MTFB_ERROR("tde blit failed\n");    
        ps_TdeExportFuncs->pfnTdeCancelJob(handle);
        return s32Ret;
    }

    if (pstOpt->bCallBack)
    {/**
      **要是TDE任务完成，tde内部会调用pstOpt->pfnCallBack这个函数，这个在fb注册过了
      **/
        s32Ret = ps_TdeExportFuncs->pfnTdeEndJob(handle,                         \
			                                       pstOpt->bBlock,                \
			                                       100,                            \
			                                       MT_FALSE,                       \
                                                  (TDE_FUNC_CB)pstOpt->pfnCallBack,\
                                                  pstOpt->pParam);
    }
    else
    {
        s32Ret = ps_TdeExportFuncs->pfnTdeEndJob(handle,                    \
			                                       pstOpt->bBlock,           \
			                                       100,                      \
			                                       MT_FALSE,                 \
                                                   MT_NULL,                   \
                                                   MT_NULL);
    }
    if(s32Ret != MT_SUCCESS)
    {
        MTFB_ERROR("end job failed\n"); 
        ps_TdeExportFuncs->pfnTdeCancelJob(handle);
        return s32Ret;
    }
    
    if (pstOpt->bRegionDeflicker)
    {
        s32Ret = ps_TdeExportFuncs->pfnTdeEnableRegionDeflicker(MT_FALSE);
        if (s32Ret != MT_SUCCESS)
        {
            MTFB_ERROR("disable region deflicker failed!\n");
            return s32Ret;
        }
    } 
    return handle;
}


/***************************************************************************************
* func          : MTFB_DRV_SetTdeCallBack
* description   : CNcomment: 设置TDE回调函数 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 MTFB_DRV_SetTdeCallBack(IntCallBack pTdeCallBack)
{/** 这个全局变量没有人用 **/
   s_pTdeCallBack = pTdeCallBack;
   return MT_SUCCESS;    
}


/***************************************************************************************
* func          : MTFB_DRV_ClearRect
* description   : CNcomment:清surface CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 MTFB_DRV_ClearRect(MTFB_SURFACE_S* pDstImg, MTFB_BLIT_OPT_S* pstOpt)
{
    mt_s32 s32Ret;
    TDE2_SURFACE_S TDESurface = {0};
    TDE2_RECT_S Rect;
    TDE_HANDLE s32Handle;

    if(NULL == ps_TdeExportFuncs)
    {
        MTFB_ERROR("Tde is not available!\n");
        return MT_FAILURE;
    }
    
    /** confing dst*/
    TDESurface.u32PhyAddr    = pDstImg->u32PhyAddr;
    TDESurface.u32Width      = pDstImg->u32Width;
    TDESurface.u32Height     = pDstImg->u32Height;
    TDESurface.u32Stride     = pDstImg->u32Pitch;
    TDESurface.bAlphaMax255  = MT_TRUE;
    TDESurface.bYCbCrClut    = MT_FALSE;
    TDESurface.enColorFmt    = MTFB_DRV_ConvFmt(pDstImg->enFmt);
    TDESurface.u8Alpha0      = pstOpt->stAlpha.u8Alpha0;
    TDESurface.u8Alpha1      = pstOpt->stAlpha.u8Alpha1;

    Rect.s32Xpos  = 0;
    Rect.s32Ypos  = 0;
    Rect.u32Width = pDstImg->u32Width;
    Rect.u32Height = pDstImg->u32Height;

    s32Ret = ps_TdeExportFuncs->pfnTdeBeginJob(&s32Handle);
    if(s32Ret != MT_SUCCESS)
    {
        MTFB_ERROR("begin job failed\n");
        return s32Ret;
    }
    s32Ret = ps_TdeExportFuncs->pfnTdeQuickFill(s32Handle, &TDESurface, &Rect, 0x0);
    if(s32Ret != MT_SUCCESS)
    {
        MTFB_ERROR("tde quick fill failed s32Ret:0x%x\n", s32Ret);    
        ps_TdeExportFuncs->pfnTdeCancelJob(s32Handle);
        return s32Ret;
    }
    s32Ret = ps_TdeExportFuncs->pfnTdeEndJob(s32Handle, pstOpt->bBlock, 100, MT_FALSE, MT_NULL, MT_NULL);
    if(s32Ret != MT_SUCCESS)
    {
        MTFB_ERROR("end job failed ret:%d\n", s32Ret); 
        ps_TdeExportFuncs->pfnTdeCancelJob(s32Handle);
        return s32Ret;
    }
    return s32Handle;
	
}


mt_void MTFB_DRV_WaitAllTdeDone(MT_BOOL bSync)
{
    if(NULL == ps_TdeExportFuncs)
    {
        MTFB_ERROR("Tde is not available!\n");
        //return -1;
        return;
    }
    ps_TdeExportFuncs->pfnTdeWaitAllDone(MT_FALSE);
}


/***************************************************************************************
* func          : MTFB_DRV_CalScaleRect
* description   : CNcomment: 计算缩放更新区域 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 MTFB_DRV_CalScaleRect(const TDE2_RECT_S* pstSrcRect, const TDE2_RECT_S* pstDstRect,
                                TDE2_RECT_S* pstRectInSrc, TDE2_RECT_S* pstRectInDst)
{
    if(NULL == ps_TdeExportFuncs)
    {
        MTFB_ERROR("Tde is not available!\n");
        return MT_FAILURE;
    }
    return ps_TdeExportFuncs->pfnTdeCalScaleRect(pstSrcRect, pstDstRect, pstRectInSrc, pstRectInDst);
}

mt_s32 MTFB_DRV_WaitForDone(TDE_HANDLE s32Handle, mt_u32 u32TimeOut)
{
    if(NULL == ps_TdeExportFuncs)
    {
        MTFB_ERROR("Tde is not available!\n");
        return MT_FAILURE;
    }
    return ps_TdeExportFuncs->pfnTdeWaitForDone(s32Handle, u32TimeOut);
}


/***************************************************************************************
* func          : MTFB_DRV_GetTdeOps
* description   : CNcomment: 获取TDE上下文 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_void MTFB_DRV_GetTdeOps(MTFB_DRV_TDEOPS_S *Ops)
{
	Ops->MTFB_DRV_Blit           = MTFB_DRV_Blit;
	Ops->MTFB_DRV_CalScaleRect   = MTFB_DRV_CalScaleRect;
	Ops->MTFB_DRV_ClearRect      = MTFB_DRV_ClearRect;
	Ops->MTFB_DRV_SetTdeCallBack = MTFB_DRV_SetTdeCallBack;
	Ops->MTFB_DRV_TdeClose       = MTFB_DRV_TdeClose;
	Ops->MTFB_DRV_TdeOpen        = MTFB_DRV_TdeOpen;
	Ops->MTFB_DRV_TdeSupportFmt  = MTFB_DRV_TdeSupportFmt;
	Ops->MTFB_DRV_WaitAllTdeDone = MTFB_DRV_WaitAllTdeDone;
	Ops->MTFB_DRV_WaitForDone    = MTFB_DRV_WaitForDone;
}


