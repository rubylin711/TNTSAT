/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#ifndef MT_BUILD_IN_BOOT
#include <linux/string.h>
#include <linux/fb.h>
#else
#include "mtfb_debug.h"
#endif
#include "optm_mtfb.h"
#include "mtfb_drv.h"

static OPTM_GFX_OPS_S g_stGfxOps;



/***************************************************************************************
* func          : OPTM_GetGfxGpId
* description   : CNcomment: 根据图层ID获取GP ID CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
OPTM_GFX_GP_E OPTM_GetGfxGpId(MTFB_LAYER_ID_E enLayerId)
{
   return OPTM_GFX_GP_0;
#if 0    
	if(enLayerId <= MTFB_LAYER_HD_3)
	{
		return OPTM_GFX_GP_0;
	}
	else if(enLayerId >= MTFB_LAYER_SD_0 && enLayerId <= MTFB_LAYER_SD_1)
	{
		return OPTM_GFX_GP_1;
	}

	return OPTM_GFX_GP_BUTT;
#endif  
}

/***************************************************************************************
* func          : MTFB_DRV_SetLayerKeyMask
* description   : CNcomment: 设置图层color key CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 MTFB_DRV_SetLayerKeyMask(MTFB_LAYER_ID_E enLayerId, const MTFB_COLORKEYEX_S *pstColorkey)
{
   return g_stGfxOps.OPTM_GfxSetLayKeyMask(enLayerId, pstColorkey);
}


/***************************************************************************************
* func          : MTFB_DRV_EnableLayer
* description   : CNcomment: 图层使能 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 MTFB_DRV_EnableLayer(MTFB_LAYER_ID_E enLayerId, MT_BOOL bEnable)
{
    return g_stGfxOps.OPTM_GfxSetEnable(enLayerId, bEnable);;
}


/***************************************************************************************
* func			: MTFB_DRV_SetLayerAddr
* description	: CNcomment: 设置显示地址 CNend\n
* param[in] 	: mt_void
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_s32 MTFB_DRV_SetLayerAddr(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Addr)
{	
    return g_stGfxOps.OPTM_GfxSetLayerAddr(enLayerId, u32Addr);
}


/***************************************************************************************
* func			: MTFB_DRV_SetLayerStride
* description	: CNcomment: 设置图层行间距 CNend\n
* param[in] 	: mt_void
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_s32 MTFB_DRV_SetLayerStride(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Stride)
{
    return g_stGfxOps.OPTM_GfxSetLayerStride(enLayerId, u32Stride);
}


/***************************************************************************************
* func			: MTFB_DRV_SetLayerDataFmt
* description	: CNcomment: 设置图层像素格式 CNend\n
* param[in] 	: mt_void
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_s32 MTFB_DRV_SetLayerDataFmt(MTFB_LAYER_ID_E enLayerId, MTFB_COLOR_FMT_E enDataFmt)
{
    if(enDataFmt >= MTFB_FMT_BUTT){
     	  return MT_FAILURE;
    }

    return g_stGfxOps.OPTM_GfxSetLayerDataFmt(enLayerId, enDataFmt);
}


mt_s32 MTFB_DRV_SetColorReg(MTFB_LAYER_ID_E enLayerId, mt_u32 u32OffSet, mt_u32 u32Color, mt_u32 UpFlag)
{
    if (u32OffSet > 255)
    {
        MTFB_ERROR("GFX color clut offset > 255.\n");
        return MT_FAILURE;
    }
    return g_stGfxOps.OPTM_GfxSetColorReg(enLayerId, u32OffSet, u32Color, UpFlag);
}


mt_s32 MTFB_DRV_WaitVBlank(MTFB_LAYER_ID_E enLayerId)
{
    return g_stGfxOps.OPTM_GfxWaitVBlank(enLayerId);
}


/***************************************************************************************
* func          : MTFB_DRV_SetLayerDeFlicker
* description   : CNcomment: 设置图层抗闪 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 MTFB_DRV_SetLayerDeFlicker(MTFB_LAYER_ID_E enLayerId, MTFB_DEFLICKER_S *pstDeFlicker)
{
    return g_stGfxOps.OPTM_GfxSetLayerDeFlicker(enLayerId, pstDeFlicker);
}


/***************************************************************************************
* func          : MTFB_DRV_SetLayerAlpha
* description   : CNcomment: 设置图层alpha的值 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 MTFB_DRV_SetLayerAlpha(MTFB_LAYER_ID_E enLayerId, MTFB_ALPHA_S *pstAlpha)
{
    return g_stGfxOps.OPTM_GfxSetLayerAlpha(enLayerId, pstAlpha);
}


mt_s32 MTFB_DRV_GetLayerInRect(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstInputRect)
{
	 mt_s32 s32Ret;

	 s32Ret = MT_SUCCESS;
	 
	 if (MT_NULL != pstInputRect)
	 {
		 s32Ret |=	g_stGfxOps.OPTM_GfxGetLayerRect(enLayerId, pstInputRect);
	 } 

	 return s32Ret;
}


 /***************************************************************************************
 * func 		 : MTFB_DRV_SetLayerInRect
 * description	 : CNcomment: 设置图层输入矩形 CNend\n
 * param[in]	 : mt_void
 * retval		 : NA
 * others:		 : NA
 ***************************************************************************************/
 mt_s32 MTFB_DRV_SetLayerInRect(MTFB_LAYER_ID_E enLayerId, const MTFB_RECT *pstInputRect)
 {
	 mt_s32 s32Ret;
 
	 s32Ret = MT_SUCCESS;
	 
	 if (MT_NULL != pstInputRect)
	 {
		 s32Ret |=	g_stGfxOps.OPTM_GfxSetLayerRect(enLayerId, pstInputRect);
	 } 
 
	 return s32Ret;
 }


 mt_s32 MTFB_DRV_SetLayerOutRect(MTFB_LAYER_ID_E enLayerId, const MTFB_RECT *pstOutputRect)
{
	mt_s32 s32Ret;

	s32Ret = MT_SUCCESS;

	if (MT_NULL != pstOutputRect)
	{
		s32Ret |= g_stGfxOps.OPTM_GfxSetGpRect(OPTM_GetGfxGpId(enLayerId), pstOutputRect);	
	}	

	return s32Ret;
}


mt_s32 MTFB_DRV_SetLayerScreenSize(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Width, mt_u32 u32Height)
{

	return g_stGfxOps.OPTM_GfxSetGpInPutSize(OPTM_GetGfxGpId(enLayerId), u32Width, u32Height);	
}


mt_s32 MTFB_DRV_GetLayerOutRect(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstOutputRect)
{
	mt_s32 s32Ret;

	s32Ret = MT_SUCCESS;

	if (MT_NULL != pstOutputRect)
	{
		s32Ret |= g_stGfxOps.OPTM_GfxGetOutRect(enLayerId, pstOutputRect);	
	}	

	return s32Ret;
}


mt_s32 MTFB_DRV_GetDispSize(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstOutputRect)
{
	mt_s32 s32Ret;

	s32Ret = MT_SUCCESS;

	if (MT_NULL != pstOutputRect)
	{
		s32Ret |= g_stGfxOps.OPTM_GfxGetDispFMTSize(OPTM_GetGfxGpId(enLayerId), pstOutputRect);	
	}	

	return s32Ret;
}


/***************************************************************************************
* func          : MTFB_DRV_SetTriDimMode
* description   : CNcomment: 设置3D模式 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT  
mt_s32 MTFB_DRV_SetTriDimMode(mt_u32 u32LayerId, MTFB_STEREO_MODE_E enStereoMode)
{
     if( MTFB_STEREO_MONO == enStereoMode)
     {
     	g_stGfxOps.OPTM_GfxSetTriDimEnable(u32LayerId, MT_FALSE);
		return g_stGfxOps.OPTM_GfxSetTriDimMode(u32LayerId, MTFB_STEREO_MONO);
     }
     else
     {
        g_stGfxOps.OPTM_GfxSetTriDimEnable(u32LayerId, MT_TRUE);
		return g_stGfxOps.OPTM_GfxSetTriDimMode(u32LayerId, enStereoMode);
     }
}


/***************************************************************************************
* func			: MTFB_DRV_SetTriDimAddr
* description	: CNcomment: 设置3D显示地址 CNend\n
* param[in] 	: mt_void
* retval		: NA
* others:		: NA
***************************************************************************************/
mt_s32 MTFB_DRV_SetTriDimAddr(mt_u32 u32LayerId, mt_u32 u32TriDimAddr)
{
	g_stGfxOps.OPTM_GfxSetTriDimAddr(u32LayerId, u32TriDimAddr);
	return MT_SUCCESS;
}
#endif


mt_s32 MTFB_DRV_ColorConvert(const struct fb_var_screeninfo *pstVar, MTFB_COLORKEYEX_S *pCkey)
{
    mt_u8 rOff, gOff, bOff;

    rOff = pstVar->red.length;
    gOff = pstVar->green.length;
    bOff = pstVar->blue.length;

    pCkey->u8RedMask = (0xff >> rOff);
    pCkey->u8GreenMask = (0xff >> gOff);
    pCkey->u8BlueMask  = (0xff >> bOff);

    return MT_SUCCESS;
}


//#endif
mt_s32 MTFB_DRV_UpdataLayerReg(MTFB_LAYER_ID_E enLayerId)
{
    return g_stGfxOps.OPTM_GfxUpLayerReg(enLayerId);
}


/***************************************************************************************
* func          : MTFB_DRV_SetLayerPreMult
* description   : CNcomment: 设置图层预乘 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 MTFB_DRV_SetLayerPreMult(MTFB_LAYER_ID_E enLayerId, MT_BOOL bEnable)
{
    return g_stGfxOps.OPTM_GfxSetLayerPreMult(enLayerId, bEnable);
}


mt_s32  MTFB_DRV_SetClutAddr(MTFB_LAYER_ID_E enLayerId, mt_u32 u32PhyAddr)
{
    return g_stGfxOps.OPTM_GfxSetClutAddr(enLayerId, u32PhyAddr);
}


mt_s32 MTFB_DRV_SetIntCallback(MTFB_CALLBACK_TPYE_E eCallbackType, IntCallBack pCallBack, MTFB_LAYER_ID_E enLayerId)
{
    return g_stGfxOps.OPTM_GfxSetCallback(enLayerId, pCallBack, eCallbackType);
}


/***************************************************************************************
* func          : MTFB_DRV_OpenLayer
* description   : CNcomment: 打开对应的图层 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 MTFB_DRV_OpenLayer(MTFB_LAYER_ID_E enLayerId, mt_u32 bEnableOsdc)
{
    return g_stGfxOps.OPTM_GfxOpenLayer(enLayerId, bEnableOsdc);
}


mt_s32 MTFB_DRV_GetOSDData(MTFB_LAYER_ID_E enLayerId, MTFB_OSD_DATA_S *pstLayerData)
{
	return g_stGfxOps.OPTM_GfxGetOSDData(enLayerId, pstLayerData);
}


mt_s32 MTFB_DRV_CloseLayer(MTFB_LAYER_ID_E enLayerId)
{
    return g_stGfxOps.OPTM_GfxCloseLayer(enLayerId);
}


/***************************************************************************************
* func          : MTFB_DRV_GetDevOps
* description   : CNcomment: 获取adp设备上下文 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 MTFB_DRV_GfxInit(mt_void)
{
	OPTM_GFX_GetOps(&g_stGfxOps);
    return g_stGfxOps.OPTM_GfxInit();
}


/***************************************************************************************
* func          : MTFB_DRV_GfxDeInit
* description   : CNcomment: 图形设备去初始化 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 MTFB_DRV_GfxDeInit(mt_void)
{
    return g_stGfxOps.OPTM_GfxDeInit();
}

mt_s32 MTFB_DRV_SetLayerPriority(MTFB_LAYER_ID_E enLayerId, MTFB_ZORDER_E enZOrder)
{
	g_stGfxOps.OPTM_GfxSetLayerPriority(enLayerId, enZOrder);
    return MT_SUCCESS;
}

mt_s32 MTFB_DRV_GetLayerPriority(MTFB_LAYER_ID_E enLayerId, mt_u32 *pU32ZOrder)
{
	g_stGfxOps.OPTM_GfxGetLayerPriority(enLayerId, pU32ZOrder);
    return MT_SUCCESS;
}


mt_s32 MTFB_DRV_PauseCompression(MTFB_LAYER_ID_E enLayerId)
{
     return MT_SUCCESS;
}
mt_s32 MTFB_DRV_ResumeCompression(MTFB_LAYER_ID_E enLayerId)
{
     return MT_SUCCESS;
}

mt_s32 MTFB_DRV_GetGFXCap(const MTFB_CAPABILITY_S **pstCap)
{
	g_stGfxOps.OPTM_GFX_GetDevCap(pstCap);

	if (MT_NULL == pstCap)
	{
		MTFB_ERROR("GFX get device capability failed!\n");
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

mt_s32 MTFB_DRV_SetScreenFlag(MTFB_LAYER_ID_E enLayerId, MT_BOOL bFlag)
{
	g_stGfxOps.OPTM_GFX_SetGpInUsrFlag(OPTM_GetGfxGpId(enLayerId), bFlag);
	return MT_SUCCESS;
}

mt_s32 MTFB_DRV_GetScreenFlag(MTFB_LAYER_ID_E enLayerId)
{	
	return g_stGfxOps.OPTM_GFX_GetGpInUsrFlag(OPTM_GetGfxGpId(enLayerId));
}

mt_s32 MTFB_DRV_SetInitScreenFlag(MTFB_LAYER_ID_E enLayerId, MT_BOOL bFlag)
{
	g_stGfxOps.OPTM_GFX_SetGpInInitFlag(OPTM_GetGfxGpId(enLayerId), bFlag);
	return MT_SUCCESS;
}

mt_s32 MTFB_DRV_GetInitScreenFlag(MTFB_LAYER_ID_E enLayerId)
{	
	return g_stGfxOps.OPTM_GFX_GetGpInInitFlag(OPTM_GetGfxGpId(enLayerId));
}

mt_s32 MTFB_DRV_SetLayerMaskFlag(MTFB_LAYER_ID_E enLayerId, MT_BOOL bFlag)
{
	g_stGfxOps.OPTM_GFX_SetGfxMask(OPTM_GetGfxGpId(enLayerId), bFlag);
	return MT_SUCCESS;
}

mt_s32 MTFB_DRV_GetLayerMaskFlag(MTFB_LAYER_ID_E enLayerId)
{	
	return g_stGfxOps.OPTM_GFX_GetGfxMask(OPTM_GetGfxGpId(enLayerId));
}

mt_s32 MTFB_DRV_ClearLogo(MTFB_LAYER_ID_E enLayerId)
{
	return g_stGfxOps.OPTM_GFX_ClearLogoOsd(enLayerId);
}

mt_s32 MTFB_DRV_SetStereoDepth(MTFB_LAYER_ID_E enLayerId, mt_s32 s32Depth)
{
	return g_stGfxOps.OPTM_GFX_SetStereoDepth(enLayerId, s32Depth);
}

mt_s32 MTFB_DRV_SetTCFlag(MT_BOOL bFlag)
{
	return g_stGfxOps.OPTM_GFX_SetTCFlag(bFlag);
}

mt_s32 MTFB_DRV_SetCmpSwitch(MTFB_LAYER_ID_E enLayerId, MT_BOOL bOpen)
{
	if (bOpen)
	{
		return g_stGfxOps.OPTM_GFX_CMP_Open(enLayerId);
	}
	else
	{
		return g_stGfxOps.OPTM_GFX_CMP_Close(enLayerId);
	}	
}

mt_s32 MTFB_DRV_GetCmpSwitch(MTFB_LAYER_ID_E enLayerId)
{
	return g_stGfxOps.OPTM_GFX_CMP_GetSwitch(enLayerId);
}

mt_s32 MTFB_DRV_SetCmpRect(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstRect)
{
	return g_stGfxOps.OPTM_GFX_SetCmpRect(enLayerId, pstRect);
}

mt_s32 MTFB_DRV_SetCmpMode(MTFB_LAYER_ID_E enLayerId, MTFB_CMP_MODE_E enCMPMode)
{
	return g_stGfxOps.OPTM_GFX_SetCmpMode(enLayerId, enCMPMode);
}

MTFB_CMP_MODE_E MTFB_DRV_GetCmpMode(MTFB_LAYER_ID_E enLayerId)
{
	return g_stGfxOps.OPTM_GFX_GetCmpMode(enLayerId);
}

mt_s32 MTFB_DRV_SetGpDeflicker(mt_u32 u32DispChn, MT_BOOL bDeflicker)
{
    OPTM_GFX_GP_E enGpID;
    enGpID = u32DispChn ? OPTM_GFX_GP_0 : OPTM_GFX_GP_1;
    return g_stGfxOps.OPTM_GfxSetGpDeflicker(enGpID, bDeflicker);
}

mt_s32 MTFB_DRV_GetSlvLayerInfo(MTFB_SLVLAYER_DATA_S *pstLayerInfo)
{
    return g_stGfxOps.OPTM_GFX_GetSlvLayerInfo(pstLayerInfo);
}


/***************************************************************************************
* func          : MTFB_DRV_GetDevOps
* description   : CNcomment: 获取设备上下文 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_void MTFB_DRV_GetDevOps(MTFB_DRV_OPS_S    *Ops)
{
	Ops->MTFB_DRV_CloseLayer        = MTFB_DRV_CloseLayer;
	Ops->MTFB_DRV_ColorConvert      = MTFB_DRV_ColorConvert;
	Ops->MTFB_DRV_EnableLayer       = MTFB_DRV_EnableLayer;
	Ops->MTFB_DRV_GetGFXCap         = MTFB_DRV_GetGFXCap;
	Ops->MTFB_DRV_GetOSDData        = MTFB_DRV_GetOSDData;
	Ops->MTFB_DRV_GetLayerPriority  = MTFB_DRV_GetLayerPriority;
	Ops->MTFB_DRV_GfxDeInit         = MTFB_DRV_GfxDeInit;
	Ops->MTFB_DRV_GfxInit           = MTFB_DRV_GfxInit;
	Ops->MTFB_DRV_OpenLayer         = MTFB_DRV_OpenLayer;
	Ops->MTFB_DRV_PauseCompression  = MTFB_DRV_PauseCompression;
	Ops->MTFB_DRV_ResumeCompression = MTFB_DRV_ResumeCompression;
	Ops->MTFB_DRV_SetClutAddr       = MTFB_DRV_SetClutAddr;
	Ops->MTFB_DRV_SetColorReg       = MTFB_DRV_SetColorReg;
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT    
	Ops->MTFB_DRV_SetTriDimMode     = MTFB_DRV_SetTriDimMode;
	Ops->MTFB_DRV_SetTriDimAddr     = MTFB_DRV_SetTriDimAddr;
#endif    
	Ops->MTFB_DRV_SetLayerAddr      = MTFB_DRV_SetLayerAddr;
	Ops->MTFB_DRV_SetLayerAlpha     = MTFB_DRV_SetLayerAlpha;
	Ops->MTFB_DRV_SetLayerDataFmt   = MTFB_DRV_SetLayerDataFmt;
	Ops->MTFB_DRV_SetLayerDeFlicker = MTFB_DRV_SetLayerDeFlicker;
	Ops->MTFB_DRV_SetLayerPriority  = MTFB_DRV_SetLayerPriority;
	Ops->MTFB_DRV_UpdataLayerReg    = MTFB_DRV_UpdataLayerReg;	
	Ops->MTFB_DRV_WaitVBlank        = MTFB_DRV_WaitVBlank;
	Ops->MTFB_DRV_SetLayerDataFmt   = MTFB_DRV_SetLayerDataFmt;
	Ops->MTFB_DRV_SetLayerKeyMask   = MTFB_DRV_SetLayerKeyMask;
	Ops->MTFB_DRV_SetLayerPreMult   = MTFB_DRV_SetLayerPreMult;
	Ops->MTFB_DRV_SetIntCallback    = MTFB_DRV_SetIntCallback;
	Ops->MTFB_DRV_SetLayerStride    = MTFB_DRV_SetLayerStride;
	Ops->MTFB_DRV_SetLayerInRect    = MTFB_DRV_SetLayerInRect;
	Ops->MTFB_DRV_SetLayerOutRect   = MTFB_DRV_SetLayerOutRect;
	Ops->MTFB_DRV_GetLayerOutRect   = MTFB_DRV_GetLayerOutRect;
	Ops->MTFB_DRV_GetLayerInRect    = MTFB_DRV_GetLayerInRect;
	Ops->MTFB_DRV_SetLayerScreenSize = MTFB_DRV_SetLayerScreenSize;
	Ops->MTFB_DRV_SetScreenFlag     = MTFB_DRV_SetScreenFlag;
	Ops->MTFB_DRV_GetScreenFlag     = MTFB_DRV_GetScreenFlag;
	Ops->MTFB_DRV_SetInitScreenFlag = MTFB_DRV_SetInitScreenFlag;
	Ops->MTFB_DRV_GetInitScreenFlag = MTFB_DRV_GetInitScreenFlag;
	Ops->MTFB_DRV_SetLayerMaskFlag  = MTFB_DRV_SetLayerMaskFlag;
	Ops->MTFB_DRV_GetLayerMaskFlag  = MTFB_DRV_GetLayerMaskFlag;
    Ops->MTFB_DRV_GetDispSize       = MTFB_DRV_GetDispSize;
	Ops->MTFB_DRV_ClearLogo         = MTFB_DRV_ClearLogo;
	Ops->MTFB_DRV_SetStereoDepth    = MTFB_DRV_SetStereoDepth;
	Ops->MTFB_DRV_SetTCFlag         = MTFB_DRV_SetTCFlag;
	Ops->MTFB_DRV_SetCmpSwitch      = MTFB_DRV_SetCmpSwitch;
	Ops->MTFB_DRV_GetCmpSwitch      = MTFB_DRV_GetCmpSwitch;
	Ops->MTFB_DRV_SetCmpRect        = MTFB_DRV_SetCmpRect;
	Ops->MTFB_DRV_SetCmpMode        = MTFB_DRV_SetCmpMode;
	Ops->MTFB_DRV_GetCmpMode        = MTFB_DRV_GetCmpMode;
    Ops->MTFB_DRV_SetGpDeflicker    = MTFB_DRV_SetGpDeflicker;
	Ops->MTFB_DRV_GetSlvLayerInfo   = MTFB_DRV_GetSlvLayerInfo;
	
	return;
}


