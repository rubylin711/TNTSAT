/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "mt_type.h"

#ifndef __ADP_MTFM_H__
#define __ADP_MTFM_H__


typedef struct mt_FB_PROC_SURFACE_S
{
  mt_u32 addr;
  mt_u32 width;
  mt_u32 height;
  mt_u32 stride;
  mt_u32 fmt;
}MTFB_PROC_SURFACE_S;
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            initial operation
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


/*gfx init*/
	/*CNcomment:图形初始化*/
	mt_s32 MTFB_GfxInit(mt_void);

	/*gfx deinit*/
	/*CNcomment:图形去初始化*/
	mt_s32 MTFB_GfxDeInit(mt_void);

	/*open layer*/
	/*CNcomment:图层打开*/
	mt_s32 MTFB_GfxOpenLayer(MTFB_LAYER_ID_E enLayerId, mt_u32 bEnableOsdc);

	/*close layer*/
	/*CNcomment:图层关闭*/
	mt_s32 MTFB_GfxCloseLayer(MTFB_LAYER_ID_E enLayerId);

	/*enable/disable layer*/
	/*CNcomment:图层使能或者非使能*/
	mt_s32 MTFB_GfxSetEnable(MTFB_LAYER_ID_E enLayerId, MT_BOOL bEnable);

	/*mask layer*/
	/*CNcomment:是否屏蔽图层*/
	mt_s32 MTFB_GfxMaskLayer(MTFB_LAYER_ID_E enLayerId, MT_BOOL bMask);

	/*set layer address*/
	/*CNcomment:设置图层显示地址*/
	mt_s32 MTFB_GfxSetLayerAddr(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Addr);

	/*set layer stride*/
	/*CNcomment:设置图层行距*/
	mt_s32 MTFB_GfxSetLayerStride(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Stride);

	/*set layer data format*/
	/*CNcomment:设置图层像素格式*/
	mt_s32 MTFB_GfxSetLayerDataFmt(MTFB_LAYER_ID_E enLayerId, MTFB_COLOR_FMT_E enDataFmt);

	mt_s32 MTFB_GfxSetColorReg(MTFB_LAYER_ID_E u32LayerId, mt_u32 u32OffSet, mt_u32 u32Color, mt_s32 UpFlag);

	/*wait for vertical blank*/
	/*CNcomment:等待垂直消隐*/
	mt_s32 MTFB_GfxWaitVBlank(MTFB_LAYER_ID_E u32LayerId);

	/*set layer deflick level*/
	/*CNcomment:设置抗闪级别*/
	mt_s32 MTFB_GfxSetLayerDeFlicker(MTFB_LAYER_ID_E enLayerId, MTFB_DEFLICKER_S *pstDeFlicker);

	/*set layer alpha*/
	/*CNcomment:设置图层alpha 值*/
	mt_s32 MTFB_GfxSetLayerAlpha(MTFB_LAYER_ID_E enLayerId, MTFB_ALPHA_S *pstAlpha);

	/*set layer rect*/
	/*CNcomment:设置图层输入矩形*/
	mt_s32 MTFB_GfxSetLayerRect(MTFB_LAYER_ID_E enLayerId, const MTFB_RECT *pstRect);

	/*set graphics process device Inrect and Outrect*/
	/*CNcomment:设置图层管理器输入输出矩形*/
	mt_s32 MTFB_GfxSetGpRect(OPTM_GFX_GP_E enGpId, const MTFB_RECT *pstInputRect);

	mt_s32 MTFB_GfxSetGpInPutSize(OPTM_GFX_GP_E enGpId, mt_u32 u32Width, mt_u32 u32Height);

	/*set layer key mask*/
	/*CNcomment:设置图层colorkey mask*/
	mt_s32 MTFB_GfxSetLayKeyMask(MTFB_LAYER_ID_E enLayerId, const MTFB_COLORKEYEX_S *pstColorkey);
  
#if 0
	/* set bit-extension mode */
	mt_s32 MTFB_GfxSetLayerBitExtMode(MTFB_LAYER_ID_E enLayerId, OPTM_GFX_BITEXTEND_E enBtMode);
#endif

	/*set layer pre mult*/
	/*CNcomment:设置图层预乘*/
	mt_s32 MTFB_GfxSetLayerPreMult(MTFB_LAYER_ID_E enLayerId, MT_BOOL bEnable);

	/*set clut address*/
	/*CNcomment:设置CLUT 地址*/
	mt_s32 MTFB_GfxSetClutAddr(MTFB_LAYER_ID_E enLayerId, mt_u32 u32PhyAddr);

	mt_s32 MTFB_GfxGetOSDData(MTFB_LAYER_ID_E enLayerId, MTFB_OSD_DATA_S *pstLayerData);

	/*set call back*/
	/*CNcomment:注册回调函数*/
#ifndef HI_BUILD_IN_BOOT	
	mt_s32 MTFB_GfxSetCallback(MTFB_LAYER_ID_E enLayerId, IntCallBack pCallBack, MTFB_CALLBACK_TPYE_E eIntType);
#endif

	/*update layer register*/
	/*CNcomment:更新寄存器*/
	mt_s32 MTFB_GfxUpLayerReg(MTFB_LAYER_ID_E enLayerId);

#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
	/*enable/disable stereo*/
	/*CNcomment:设置3D 使能或者非使能*/
	mt_s32 MTFB_GfxSetTriDimEnable(MTFB_LAYER_ID_E enLayerId, mt_u32 bEnable);

	/*set stereo mode*/
	/*CNcomment:设置3D 模式SBS/TB/MVC*/
	mt_s32 MTFB_GfxSetTriDimMode(MTFB_LAYER_ID_E enLayerId, MTFB_STEREO_MODE_E enMode);

	/*set stereo address*/
	/*CNcomment:设置3D 模式下右眼地址*/
	mt_s32 MTFB_GfxSetTriDimAddr(MTFB_LAYER_ID_E enLayerId, mt_u32 u32TriDimAddr);
#endif	

	/*set the priority of layer in gp*/
	/*CNcomment:设置图层在GP 中的优先级*/
	mt_s32 MTFB_GfxSetLayerPriority(MTFB_LAYER_ID_E u32LayerId, MTFB_ZORDER_E enZOrder);

	/*get the priority of layer in gp*/
	/*CNcomment:获取图层在GP 中的优先级*/
	mt_s32 MTFB_GfxGetLayerPriority(MTFB_LAYER_ID_E u32LayerId, mt_u32 *pU32Priority);
	
	/*获取图层的能力集*/
	mt_s32 MTFB_GFX_GetDevCap(const MTFB_CAPABILITY_S **pstCap);
  
	/*获取图层的输出区域大小*/
	mt_s32 MTFB_GfxGetOutRect(MTFB_LAYER_ID_E enLayerId, MTFB_RECT * pstOutputRect);
  
	/*获取图层的输入区域大小*/
	mt_s32 MTFB_GfxGetLayerRect(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstRect);
  
	/*设置用户设置GP输入分辨率标志*/
	mt_s32 MTFB_GFX_SetGpInUsrFlag(OPTM_GFX_GP_E enGpId, MT_BOOL bFlag);
  
	/*获取用户设置GP输入分辨率标志*/
	mt_s32 MTFB_GFX_GetGpInUsrFlag(OPTM_GFX_GP_E enGpId);
  
	/*设置用户设置GP输入分辨率标志*/
	mt_s32 MTFB_GFX_SetGpInInitFlag(OPTM_GFX_GP_E enGpId, MT_BOOL bFlag);
  
	/*获取用户设置GP输入分辨率标志*/
	mt_s32 MTFB_GFX_GetGpInInitFlag(OPTM_GFX_GP_E enGpId);
  
	/*set gfx mask flag*/
	mt_s32 MTFB_GFX_SetGfxMask(OPTM_GFX_GP_E enGpId, MT_BOOL bFlag);
  
	/*get gfx mask flag*/
	mt_s32 MTFB_GFX_GetGfxMask(OPTM_GFX_GP_E enGpId);
  
	mt_s32 MTFB_GfxGetDispFMTSize(OPTM_GFX_GP_E enGpId, MTFB_RECT *pstOutRect);
  
#ifndef HI_BUILD_IN_BOOT	
	mt_s32 MTFB_GFX_ClearLogoOsd(MTFB_LAYER_ID_E enLayerId);

	mt_s32 MTFB_GFX_SetStereoDepth(MTFB_LAYER_ID_E enLayerId, mt_s32 s32Depth);
  
	mt_s32 MTFB_GFX_CMP_Open(MTFB_LAYER_ID_E enLayerId);
  
	mt_s32 MTFB_GFX_CMP_Close(MTFB_LAYER_ID_E enLayerId);
  
	mt_s32 MTFB_GFX_CMP_GetSwitch(MTFB_LAYER_ID_E enLayerId);
  
	mt_s32 MTFB_GFX_SetCmpRect(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstRect);
  
	/*Set compression Mode*/
	mt_s32 MTFB_GFX_SetCmpMode(MTFB_LAYER_ID_E enLayerId, MTFB_CMP_MODE_E enCMPMode);
  
	/*Get compression Mode*/
	MTFB_CMP_MODE_E MTFB_GFX_GetCmpMode(MTFB_LAYER_ID_E enLayerId);
  
	/*Get compression Mode*/
	mt_s32 MTFB_GFX_SetCmpDDROpen(MTFB_LAYER_ID_E enLayerId, MT_BOOL bOpen);
  
	/*Get slvavery layer info*/
	mt_s32 MTFB_GFX_GetSlvLayerInfo(MTFB_SLVLAYER_DATA_S *pstLayerInfo);
#endif	

	mt_s32 MTFB_GFX_SetTCFlag(MT_BOOL bFlag);

    mt_s32 MTFB_GfxSetGpDeflicker(OPTM_GFX_GP_E enGpId, MT_BOOL bDeflicker);
#ifdef CFG_MTGO_PROC_SUPPORT    
    mt_s32 MTFB_GfxProcSurfaceInfo(MTFB_LAYER_ID_E enLayerId, MTFB_PROC_SURFACE_S *p_info);
#endif    
mt_s32 MTFB_GfxWaitSync(mt_void);

mt_s32 MTFB_GfxCmpDecmpProcess(MTFB_LAYER_ID_E enLayerId, mt_u32 u32RdAddr, 
                                                         mt_u32 pic_width, mt_u32 pic_height, mt_u32 u32Stride, mt_u32 u32WrAddr);

#endif /* __ADP_MTFM_H__*/








