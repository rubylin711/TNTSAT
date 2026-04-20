/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************
  File Name     : optm_hifb.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      : 
    Modification: 

*********************************************************************************************/
#ifndef MT_BUILD_IN_BOOT
#include "mt_type.h"
#include "mt_common.h"
#include "mtfb_drv_common.h"
#include "mt_drv_disp.h"

#else
#include "mtfb_debug.h"
#endif


#ifndef __OPTM_MTFB_H_H__
#define __OPTM_MTFB_H_H__

#define CMP_INTNUM  (8+32)
#define CFG_MTGO_PROC_SUPPORT

/*!
   Align bytes macro
  */
#define ALIGN_SHIFT    (3)

typedef struct tagOPTM_GFX_CSC_PARA_S
{
	mt_u32               u32Bright;
    mt_u32               u32Contrast;
    mt_u32               u32Saturation;
    mt_u32               u32Hue;
    mt_u32               u32Kr;
    mt_u32               u32Kg;
    mt_u32               u32Kb;
}OPTM_GFX_CSC_PARA_S;

typedef enum tagOPTM_GFX_GP_E
{
	OPTM_GFX_GP_0 = 0x0,/** process gfx0,gfx1,gfx2,gfx3*/
	OPTM_GFX_GP_1,		/** process gfx4,gfx5                */
	OPTM_GFX_GP_BUTT
}OPTM_GFX_GP_E;

typedef struct tagOPTM_GFX_OFFSET_S
{
    mt_u32 u32Left;    /*left offset */
    mt_u32 u32Top;     /*top offset */
    mt_u32 u32Right;   /*right offset */
    mt_u32 u32Bottom;  /*bottom offset */
}OPTM_GFX_OFFSET_S;



/** csc state*/
typedef enum tagOPTM_CSC_STATE_E
{
    OPTM_CSC_SET_PARA_ENABLE = 0x0, 
    OPTM_CSC_SET_PARA_RGB,       
    OPTM_CSC_SET_PARA_BGR,
    OPTM_CSC_SET_PARA_CLUT,
    OPTM_CSC_SET_PARA_CbYCrY,
    OPTM_CSC_SET_PARA_YCbYCr,
    OPTM_CSC_SET_PARA_BUTT 
} OPTM_CSC_STATE_E;

typedef enum optm_COLOR_SPACE_E
{
    OPTM_CS_UNKNOWN = 0,
    
    OPTM_CS_BT601_YUV_LIMITED,/* BT.601 */
    OPTM_CS_BT601_YUV_FULL,
    OPTM_CS_BT601_RGB_LIMITED,
    OPTM_CS_BT601_RGB_FULL,    

    OPTM_CS_BT709_YUV_LIMITED,/* BT.709 */
    OPTM_CS_BT709_YUV_FULL,
    OPTM_CS_BT709_RGB_LIMITED,
    OPTM_CS_BT709_RGB_FULL,
    
    OPTM_CS_BUTT
} OPTM_COLOR_SPACE_E;



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            initial operation
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
typedef struct
{
	/*gfx init*/
	/*CNcomment:图形初始化*/
	mt_s32 (*OPTM_GfxInit)(mt_void);

	/*gfx deinit*/
	/*CNcomment:图形去初始化*/
	mt_s32 (*OPTM_GfxDeInit)(mt_void);

	/*open layer*/
	/*CNcomment:图层打开*/
	mt_s32 (*OPTM_GfxOpenLayer)(MTFB_LAYER_ID_E enLayerId, mt_u32 bEnableOsdc);

	/*close layer*/
	/*CNcomment:图层关闭*/
	mt_s32 (*OPTM_GfxCloseLayer)(MTFB_LAYER_ID_E enLayerId);

	/*enable/disable layer*/
	/*CNcomment:图层使能或者非使能*/
	mt_s32 (*OPTM_GfxSetEnable)(MTFB_LAYER_ID_E enLayerId, MT_BOOL bEnable);

	/*mask layer*/
	/*CNcomment:是否屏蔽图层*/
	mt_s32 (*OPTM_GfxMaskLayer)(MTFB_LAYER_ID_E enLayerId, MT_BOOL bMask);

	/*set layer address*/
	/*CNcomment:设置图层显示地址*/
	mt_s32 (*OPTM_GfxSetLayerAddr)(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Addr);

	/*set layer stride*/
	/*CNcomment:设置图层行距*/
	mt_s32 (*OPTM_GfxSetLayerStride)(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Stride);

	/*set layer data format*/
	/*CNcomment:设置图层像素格式*/
	mt_s32 (*OPTM_GfxSetLayerDataFmt)(MTFB_LAYER_ID_E enLayerId, MTFB_COLOR_FMT_E enDataFmt);

	mt_s32 (*OPTM_GfxSetColorReg)(MTFB_LAYER_ID_E u32LayerId, mt_u32 u32OffSet, mt_u32 u32Color, mt_u32 UpFlag);

	/*wait for vertical blank*/
	/*CNcomment:等待垂直消隐*/
	mt_s32 (*OPTM_GfxWaitVBlank)(MTFB_LAYER_ID_E u32LayerId);

	/*set layer deflick level*/
	/*CNcomment:设置抗闪级别*/
	mt_s32 (*OPTM_GfxSetLayerDeFlicker)(MTFB_LAYER_ID_E enLayerId, MTFB_DEFLICKER_S *pstDeFlicker);

	/*set layer alpha*/
	/*CNcomment:设置图层alpha 值*/
	mt_s32 (*OPTM_GfxSetLayerAlpha)(MTFB_LAYER_ID_E enLayerId, MTFB_ALPHA_S *pstAlpha);

	/*set layer rect*/
	/*CNcomment:设置图层输入矩形*/
	mt_s32 (*OPTM_GfxSetLayerRect)(MTFB_LAYER_ID_E enLayerId, const MTFB_RECT *pstRect);

	/*set graphics process device Inrect and Outrect*/
	/*CNcomment:设置图层管理器输入输出矩形*/
	mt_s32 (*OPTM_GfxSetGpRect)(OPTM_GFX_GP_E enGpId, const MTFB_RECT *pstInputRect);

	mt_s32 (*OPTM_GfxSetGpInPutSize)(OPTM_GFX_GP_E enGpId, mt_u32 u32Width, mt_u32 u32Height);

	/*set layer key mask*/
	/*CNcomment:设置图层colorkey mask*/
	mt_s32 (*OPTM_GfxSetLayKeyMask)(MTFB_LAYER_ID_E enLayerId, const MTFB_COLORKEYEX_S *pstColorkey);
#if 0
	/* set bit-extension mode */
	mt_s32 OPTM_GfxSetLayerBitExtMode(MTFB_LAYER_ID_E enLayerId, OPTM_GFX_BITEXTEND_E enBtMode);
#endif

	/*set layer pre mult*/
	/*CNcomment:设置图层预乘*/
	mt_s32 (*OPTM_GfxSetLayerPreMult)(MTFB_LAYER_ID_E enLayerId, MT_BOOL bEnable);

	/*set clut address*/
	/*CNcomment:设置CLUT 地址*/
	mt_s32 (*OPTM_GfxSetClutAddr)(MTFB_LAYER_ID_E enLayerId, mt_u32 u32PhyAddr);

	mt_s32 (*OPTM_GfxGetOSDData)(MTFB_LAYER_ID_E enLayerId, MTFB_OSD_DATA_S *pstLayerData);

	/*set call back*/
	/*CNcomment:注册回调函数*/
#ifndef MT_BUILD_IN_BOOT	
	mt_s32 (*OPTM_GfxSetCallback)(MTFB_LAYER_ID_E enLayerId, IntCallBack pCallBack, MTFB_CALLBACK_TPYE_E eIntType);
#endif
	/*update layer register*/
	/*CNcomment:更新寄存器*/
	mt_s32 (*OPTM_GfxUpLayerReg)(MTFB_LAYER_ID_E enLayerId);
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
	/*enable/disable stereo*/
	/*CNcomment:设置3D 使能或者非使能*/
	mt_s32 (*OPTM_GfxSetTriDimEnable)(MTFB_LAYER_ID_E enLayerId, mt_u32 bEnable);

	/*set stereo mode*/
	/*CNcomment:设置3D 模式SBS/TB/MVC*/
	mt_s32 (*OPTM_GfxSetTriDimMode)(MTFB_LAYER_ID_E enLayerId, MTFB_STEREO_MODE_E enMode);

	/*set stereo address*/
	/*CNcomment:设置3D 模式下右眼地址*/
	mt_s32 (*OPTM_GfxSetTriDimAddr)(MTFB_LAYER_ID_E enLayerId, mt_u32 u32TriDimAddr);
#endif	
	/*set the priority of layer in gp*/
	/*CNcomment:设置图层在GP 中的优先级*/
	mt_s32 (*OPTM_GfxSetLayerPriority)(MTFB_LAYER_ID_E u32LayerId, MTFB_ZORDER_E enZOrder);

	/*get the priority of layer in gp*/
	/*CNcomment:获取图层在GP 中的优先级*/
	mt_s32 (*OPTM_GfxGetLayerPriority)(MTFB_LAYER_ID_E u32LayerId, mt_u32 *pU32Priority);
	
	/*获取图层的能力集*/
	mt_s32 (*OPTM_GFX_GetDevCap)(const MTFB_CAPABILITY_S **pstCap);
	/*获取图层的输出区域大小*/
	mt_s32 (*OPTM_GfxGetOutRect)(MTFB_LAYER_ID_E enLayerId, MTFB_RECT * pstOutputRect);
	/*获取图层的输入区域大小*/
	mt_s32 (*OPTM_GfxGetLayerRect)(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstRect);
	/*设置用户设置GP输入分辨率标志*/
	mt_s32 (*OPTM_GFX_SetGpInUsrFlag)(OPTM_GFX_GP_E enGpId, MT_BOOL bFlag);
	/*获取用户设置GP输入分辨率标志*/
	mt_s32 (*OPTM_GFX_GetGpInUsrFlag)(OPTM_GFX_GP_E enGpId);
	/*设置用户设置GP输入分辨率标志*/
	mt_s32 (*OPTM_GFX_SetGpInInitFlag)(OPTM_GFX_GP_E enGpId, MT_BOOL bFlag);
	/*获取用户设置GP输入分辨率标志*/
	mt_s32 (*OPTM_GFX_GetGpInInitFlag)(OPTM_GFX_GP_E enGpId);
	/*set gfx mask flag*/
	mt_s32 (*OPTM_GFX_SetGfxMask)(OPTM_GFX_GP_E enGpId, MT_BOOL bFlag);
	/*get gfx mask flag*/
	mt_s32 (*OPTM_GFX_GetGfxMask)(OPTM_GFX_GP_E enGpId);
	mt_s32 (*OPTM_GfxGetDispFMTSize)(OPTM_GFX_GP_E enGpId, MTFB_RECT *pstOutRect);
#ifndef MT_BUILD_IN_BOOT	
	mt_s32 (*OPTM_GFX_ClearLogoOsd)(MTFB_LAYER_ID_E enLayerId);
	mt_s32 (*OPTM_GFX_SetStereoDepth)(MTFB_LAYER_ID_E enLayerId, mt_s32 s32Depth);
	mt_s32 (*OPTM_GFX_CMP_Open)(MTFB_LAYER_ID_E enLayerId);
	mt_s32 (*OPTM_GFX_CMP_Close)(MTFB_LAYER_ID_E enLayerId);
	mt_s32 (*OPTM_GFX_CMP_GetSwitch)(MTFB_LAYER_ID_E enLayerId);
	mt_s32 (*OPTM_GFX_SetCmpRect)(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstRect);
	/*Set compression Mode*/
	mt_s32 (*OPTM_GFX_SetCmpMode)(MTFB_LAYER_ID_E enLayerId, MTFB_CMP_MODE_E enCMPMode);
	/*Get compression Mode*/
	MTFB_CMP_MODE_E (*OPTM_GFX_GetCmpMode)(MTFB_LAYER_ID_E enLayerId);
	/*Get slvavery layer info*/
	mt_s32 (*OPTM_GFX_GetSlvLayerInfo)(MTFB_SLVLAYER_DATA_S *pstLayerInfo);
#endif	
	mt_s32 (*OPTM_GFX_SetTCFlag)(MT_BOOL bFlag);
    mt_s32 (*OPTM_GfxSetGpDeflicker)(OPTM_GFX_GP_E enGpId, MT_BOOL bDeflicker);
    mt_s32 (*OPTM_GFX_CMP_DECMP_Process)(MTFB_LAYER_ID_E enLayerId, mt_u32 u32RdAddr, 
                                                         mt_u32 pic_width, mt_u32 pic_height, mt_u32 u32Stride, mt_u32 u32WrAddr);
    mt_s32 (*OPTM_GfxWaitSync)(mt_void);    
    mt_s32 (*OPTM_GfxGetOsdHeader)(MTFB_LAYER_ID_E enLayerId, osd_header_info_s *info);
}OPTM_GFX_OPS_S;

mt_void OPTM_GFX_GetOps(OPTM_GFX_OPS_S *ops);
#ifdef MT_BUILD_IN_BOOT
mt_s32 OPTM_GfxInit(mt_void);
mt_s32 OPTM_GfxOpenLayer(MTFB_LAYER_ID_E enLayerId, mt_u32 bEnableOsdc);
mt_s32 OPTM_GfxCloseLayer(MTFB_LAYER_ID_E enLayerId);
mt_s32 OPTM_GfxOpenSlvLayer(MTFB_LAYER_ID_E enLayerId);
mt_s32 OPTM_GfxSetLayerAlpha(MTFB_LAYER_ID_E enLayerId, MTFB_ALPHA_S *pstAlpha);
mt_s32 OPTM_GfxSetLayerAddr(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Addr);
mt_s32 OPTM_GfxSetLayerStride(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Stride);
mt_s32 OPTM_GfxSetLayerDataFmt(MTFB_LAYER_ID_E enLayerId, MTFB_COLOR_FMT_E enDataFmt);
mt_s32 OPTM_GfxSetLayerRect(MTFB_LAYER_ID_E enLayerId, const MTFB_RECT *pstRect);
mt_s32 OPTM_GfxSetLayKeyMask(MTFB_LAYER_ID_E enLayerId, const MTFB_COLORKEYEX_S *pstColorkey);
mt_void OPTM_Wbc2Isr(mt_void* pParam0, mt_void *pParam1);
mt_s32 OPTM_GfxSetEnable(MTFB_LAYER_ID_E enLayerId, MT_BOOL bEnable);
mt_s32 OPTM_GfxSetGpRect(OPTM_GFX_GP_E enGpId, const MTFB_RECT * pstInputRect);
mt_s32 OPTM_GfxUpLayerReg(MTFB_LAYER_ID_E enLayerId);
MTFB_GFX_MODE_EN OPTM_Get_GfxWorkMode(mt_void);
mt_s32 OPTM_GpInitFromDisp(OPTM_GFX_GP_E enGPId);
mt_s32 OPTM_GfxSetDispFMTSize(OPTM_GFX_GP_E enGpId, const MT_RECT_S *pstOutRect);
mt_s32 OPTM_GFX_SetTCFlag(MT_BOOL bFlag);
#endif
#endif /* __OPTM_MTFB_H_H__*/

