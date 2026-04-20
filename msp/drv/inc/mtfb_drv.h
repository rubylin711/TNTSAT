/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MTFB_DRV_H__
#define __MTFB_DRV_H__


/*********************************add include here******************************/
#include <linux/fb.h>

#include "mt_tde_type.h"
#include "mtfb.h"
#include "mtfb_drv_common.h"

/*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
   extern "C"
{
#endif
#endif /* __cplusplus */



/***************************** Macro Definition ******************************/



/*************************** Structure Definition ****************************/

typedef struct
{
    MT_BOOL   bInRegionClip;
    MT_BOOL   bClip;
    MTFB_RECT stClipRect;
}MTFB_CLIP_S;

typedef struct
{
	MT_BOOL bScale;
    MT_BOOL bBlock;
    MT_BOOL bCallBack;
	MT_BOOL bRegionDeflicker;
	mt_void *pParam;
	phys_addr_t u32CmapAddr;
	MTFB_LAYER_ANTIFLICKER_LEVEL_E enAntiflickerLevel;
	IntCallBack pfnCallBack; 
    MTFB_ALPHA_S stAlpha;
    MTFB_COLORKEYEX_S stCKey;
    MTFB_CLIP_S stClip;    	   
}MTFB_BLIT_OPT_S;


/*mask bit*/
typedef enum
{
    /*Color format*/
    /*CNcomment:颜色格式*/
    MTFB_LAYER_PARAMODIFY_FMT = 0x1,

    /*Line length*/
    /*CNcomment:行间距*/
    MTFB_LAYER_PARAMODIFY_STRIDE = 0x2,

    /*Alpha value*/
    /*CNcomment:alpha值*/
    MTFB_LAYER_PARAMODIFY_ALPHA = 0x4,

    /*Colorkey value*/
    /*CNcomment:colorkey值*/
    MTFB_LAYER_PARAMODIFY_COLORKEY = 0x8,

    /*Input rectangle*/
    /*CNcomment:输入矩形*/
    MTFB_LAYER_PARAMODIFY_INRECT = 0x10,

    /*Output rectangle*/
    /*CNcomment:输出矩形*/
    MTFB_LAYER_PARAMODIFY_OUTRECT = 0x20,

    /*Display buf address*/
    /*CNcomment:显示buffer地址*/
    MTFB_LAYER_PARAMODIFY_DISPLAYADDR = 0x40,

    /*State of show or mtde*/
    /*CNcomment:显示隐藏状态*/
    MTFB_LAYER_PARAMODIFY_SHOW = 0x80,

    /*Whether premultiply data or not*/
    /*CNcomment:是否为预乘数据*/
    MTFB_LAYER_PARAMODIFY_BMUL = 0x100,

    /*Anti deflicker level*/
    /*CNcomment:抗闪烁级别*/
    MTFB_LAYER_PARAMODIFY_ANTIFLICKERLEVEL = 0x200,

	/*refresh usr data*/
    /*CNcomment:是否刷新*/
    MTFB_LAYER_PARAMODIFY_REFRESH = 0x400,  /**color format ,stride,display address only take effect when 
    												usr data was refreshed*/

	/*param modify all*/
	/*CNcomment:所有修改项*/
	MTFB_LAYER_PARAMODIFY_ALL = 0x1 | 0x2 | 0x4 | 0x8 | 0x10 | 0x20 | 0x40 | 0x80 | 0x100 | 0x200 | 0x400,
	
    MTFB_LAYER_PARAMODIFY_BUTT
}MTFB_LAYER_PARAMODIFY_MASKBIT_E;

typedef struct 
{
	 /* support how many layers*/
	/*CNcomment:返回支持的图层数 */
	//mt_s32 MTFB_DRV_GetSupportLayerCount();

	/*set layer the default bit extenal mode*/
	/*CNcomment:设置默认的扩展bit 模式*/
	//mt_s32 (*MTFB_DRV_SetLayerBitExtMode)(MTFB_LAYER_ID_E enLayerID);

	/*enable/disable the layer*/
	/*CNcomment:使能图层*/
	mt_s32 (*MTFB_DRV_EnableLayer)(MTFB_LAYER_ID_E enLayerId,MT_BOOL bEnable);

	/*set the address of layer*/
	/*CNcomment:设置图层的显示地址*/
	mt_s32 (*MTFB_DRV_SetLayerAddr)(MTFB_LAYER_ID_E enLayerId,mt_u32 u32Addr);

	/*set layer stride*/
	/*CNcomment:设置图层行间距*/
	mt_s32 (*MTFB_DRV_SetLayerStride)(MTFB_LAYER_ID_E enLayerId,mt_u32 u32Stride);

	/*set layer pixel format*/
	/*CNcomment:设置图层像素格式*/
	mt_s32 (*MTFB_DRV_SetLayerDataFmt)(MTFB_LAYER_ID_E enLayerId,MTFB_COLOR_FMT_E enDataFmt);

	/*set color register*/
	/*CNcomment:设置color 值*/
	mt_s32 (*MTFB_DRV_SetColorReg)(MTFB_LAYER_ID_E enLayerId, mt_u32 u32OffSet, mt_u32 u32Color, mt_u32 UpFlag);

	/*wait until vblank, it's a block interface*/
	/*CNcomment:等待垂直时序中断，阻塞型接口*/
	mt_s32 (*MTFB_DRV_WaitVBlank)(MTFB_LAYER_ID_E enLayerId);

	/* set layer deflicker */
	/*CNcomment:设置图层抗闪参数*/
	mt_s32 (*MTFB_DRV_SetLayerDeFlicker)(MTFB_LAYER_ID_E enLayerId, MTFB_DEFLICKER_S *pstDeFlicker);

	/*set layer alpha*/
	/*CNcomment:设置图层alpha*/
	mt_s32 (*MTFB_DRV_SetLayerAlpha)(MTFB_LAYER_ID_E enLayerId, MTFB_ALPHA_S *pstAlpha);

	/*set layer start position and size*/
	/*CNcomment:设置图层起始位置和大小*/
	mt_s32 (*MTFB_DRV_SetLayerRect)(MTFB_LAYER_ID_E enLayerId, const MTFB_RECT *pstInputRect, const MTFB_RECT *pstOutputRect);
	
	/*set layer start position and size*/
	/*CNcomment:设置图层起始位置和大小*/
	mt_s32 (*MTFB_DRV_SetLayerInRect)(MTFB_LAYER_ID_E enLayerId, const MTFB_RECT *pstInputRect);

	/*set layer output size*/
	/*CNcomment:设置图层的输出区域*/
	mt_s32 (*MTFB_DRV_SetLayerOutRect)(MTFB_LAYER_ID_E enLayerId, const MTFB_RECT *pstOutputRect);


	/* set layer colorkey */
	/* CNcomment:设置图层colorkey*/
	mt_s32 (*MTFB_DRV_SetLayerKeyMask)(MTFB_LAYER_ID_E enLayerId, const MTFB_COLORKEYEX_S* pstColorkey);

	/* update layer register */
	/*CNcomment:寄存器更新*/
	mt_s32 (*MTFB_DRV_UpdataLayerReg)(MTFB_LAYER_ID_E enLayerId);

	/*wait for the config register completed*/
	/*CNcomment:等待寄存器配置完成*/
	mt_s32 (*MTFB_DRV_WaitRegUpdateFinished)(MTFB_LAYER_ID_E enLayerId);

	/*set premul data*/
	/*CNcomment:设置预乘*/
	mt_s32 (*MTFB_DRV_SetLayerPreMult)(MTFB_LAYER_ID_E enLayerId, MT_BOOL bPreMul);

	/*set clut address*/
	/*CNcomment:设置CLUT 地址*/
	mt_s32 (*MTFB_DRV_SetClutAddr)(MTFB_LAYER_ID_E enLayerId, mt_u32 u32PhyAddr);

	/* get osd data */
	/*CNcomment:获取硬件数据*/
	mt_s32 (*MTFB_DRV_GetOSDData)(MTFB_LAYER_ID_E enLayerId, MTFB_OSD_DATA_S *pstLayerData);
  
	/* register call back function*/
	/*CNcomment:注册中断回调函数*/
	mt_s32 (*MTFB_DRV_SetIntCallback)(MTFB_CALLBACK_TPYE_E enCType, IntCallBack pCallback, MTFB_LAYER_ID_E enLayerId);

	/*open layer*/
	/*CNcomment:打开图层*/
	mt_s32 (*MTFB_DRV_OpenLayer)(MTFB_LAYER_ID_E enLayerId, mt_u32 bEnableOsdc);

	/*close layer*/
	/*CNcomment:关闭图层*/
	mt_s32 (*MTFB_DRV_CloseLayer)(MTFB_LAYER_ID_E enLayerId);

	/*get status of disp*/
	/*CNcomment:获取DISP 打开状态*/
	mt_s32 (*MTFB_DRV_GetHaltDispStatus)(MTFB_LAYER_ID_E enLayerId,MT_BOOL *pbDispInit);

	/*set 3D mode*/
	/*CNcomment:设置3D 模式*/
	mt_s32 (*MTFB_DRV_SetTriDimMode)(MTFB_LAYER_ID_E enLayerId, MTFB_STEREO_MODE_E enStereoMode);  

	
	/*set 3D address*/
	/*CNcomment:设置3D 数据地址*/
	mt_s32 (*MTFB_DRV_SetTriDimAddr)(MTFB_LAYER_ID_E enLayerId, mt_s32 u32StereoAddr);  

	/*get capability of gfx*/
	/*CNcomment:获取图层能力集*/
	mt_s32 (*MTFB_DRV_GetGFXCap)(const MTFB_CAPABILITY_S **pstCap);

	/*pause compression*/
	/*CNcomment:暂停压缩*/
	mt_s32 (*MTFB_DRV_PauseCompression)(MTFB_LAYER_ID_E enLayerId);

	/*resume compression*/
	/*CNcomment:恢复压缩*/
	mt_s32 (*MTFB_DRV_ResumeCompression)(MTFB_LAYER_ID_E enLayerId);

	/*set the priority of layer in gp*/
	/*CNcomment:设置图层在GP 中的优先级*/
	mt_s32 (*MTFB_DRV_SetLayerPriority)(MTFB_LAYER_ID_E enLayerId, MTFB_ZORDER_E enZOrder);

	/*get the priority of layer in gp*/
	/*CNcomment:获取图层在GP 中的优先级*/
	mt_s32 (*MTFB_DRV_GetLayerPriority)(MTFB_LAYER_ID_E enLayerId, mt_u32 *pU32Priority);

	/*mask layer,  prevent user to operating the layer in  the period of display format changing*/
	/*CNcomment:屏蔽图层，阻止用户对图层的硬件设置操作*/
	mt_s32 (*MTFB_DRV_MaskLayer)(MTFB_LAYER_ID_E enLayerId, MT_BOOL bFlag);	
  
	mt_s32 (*MTFB_DRV_ColorConvert)(const struct fb_var_screeninfo *pstVar, MTFB_COLORKEYEX_S *pCkey);	

	mt_s32 (*MTFB_DRV_GfxInit)(mt_void);
	mt_s32 (*MTFB_DRV_GfxDeInit)(mt_void);

	mt_s32 (*MTFB_DRV_GetLayerOutRect)(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstOutputRect);
	mt_s32 (*MTFB_DRV_GetLayerInRect)(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstOutputRect);  
	/*set layer screen size*/
	/*CNcomment:设置图层的输出区域*/
	mt_s32 (*MTFB_DRV_SetLayerScreenSize)(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Width, mt_u32 u32Height);
	/*set flag of screensize modified by usr*/
	/*CNcomment:设置用户修改SCREEN 的标志*/
	mt_s32 (*MTFB_DRV_SetScreenFlag)(MTFB_LAYER_ID_E enLayerId, MT_BOOL bFlag);
	/*Get flag of screensize modified by usr*/
	/*CNcomment:获取用户修改SCREEN 的标志*/
	mt_s32 (*MTFB_DRV_GetScreenFlag)(MTFB_LAYER_ID_E enLayerId);

	/*set flag of screensize modified by usr*/
	/*CNcomment:设置用户修改SCREEN 的标志*/
	mt_s32 (*MTFB_DRV_SetInitScreenFlag)(MTFB_LAYER_ID_E enLayerId, MT_BOOL bFlag);
	/*Get flag of screensize modified by usr*/
	/*CNcomment:获取用户修改SCREEN 的标志*/
	mt_s32 (*MTFB_DRV_GetInitScreenFlag)(MTFB_LAYER_ID_E enLayerId);
	/*set gfx mask flag*/
	/*CNcomment:设置图形屏蔽标志*/
	mt_s32 (*MTFB_DRV_SetLayerMaskFlag)(MTFB_LAYER_ID_E enLayerId, MT_BOOL bFlag);
	/*Get gfx mask flag*/
	/*CNcomment:获取图形屏蔽标志*/
	mt_s32 (*MTFB_DRV_GetLayerMaskFlag)(MTFB_LAYER_ID_E enLayerId);

	mt_s32 (*MTFB_DRV_GetDispSize)(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstOutputRect);
	mt_s32 (*MTFB_DRV_ClearLogo)(MTFB_LAYER_ID_E enLayerId);
	mt_s32 (*MTFB_DRV_SetStereoDepth)(MTFB_LAYER_ID_E enLayerId, mt_s32 s32Depth);
	mt_s32 (*MTFB_DRV_SetTCFlag)(MT_BOOL bFlag);
	/*Set compression status*/
	/*CNcomment:设置压缩开关*/
	mt_s32 (*MTFB_DRV_SetCmpSwitch)(MTFB_LAYER_ID_E enLayerId, MT_BOOL bOpen);
	/*Get compression status*/
	/*CNcomment:获取压缩开关*/
	mt_s32 (*MTFB_DRV_GetCmpSwitch)(MTFB_LAYER_ID_E enLayerId);
	/*Set compression update rect*/
	/*CNcomment:设置压缩更新区域*/
	mt_s32 (*MTFB_DRV_SetCmpRect)(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstRect);
	/*Set compression Mode*/
	/*CNcomment:设置压缩开关*/
	mt_s32 (*MTFB_DRV_SetCmpMode)(MTFB_LAYER_ID_E enLayerId, MTFB_CMP_MODE_E enCMPMode);
	/*Get compression Mode*/
	/*CNcomment:获取压缩开关*/
	MTFB_CMP_MODE_E (*MTFB_DRV_GetCmpMode)(MTFB_LAYER_ID_E enLayerId);
    mt_s32 (*MTFB_DRV_SetGpDeflicker)(mt_u32 u32DispChn, MT_BOOL bDeflicker);
	/*Get slvavery layer info*/
	/*CNcomment:?????????????*/
	mt_s32 (*MTFB_DRV_GetSlvLayerInfo)(MTFB_SLVLAYER_DATA_S *pstLayerInfo);
}MTFB_DRV_OPS_S;

typedef struct
{
	mt_s32 (*MTFB_DRV_Blit)(MTFB_BUFFER_S *pSrcImg, MTFB_BUFFER_S *pDstImg,  MTFB_BLIT_OPT_S *pstOpt,MT_BOOL bScreenRefresh);
	mt_s32 (*MTFB_DRV_ClearRect)(MTFB_SURFACE_S* pDstImg, MTFB_BLIT_OPT_S* pstOpt);
	mt_s32 (*MTFB_DRV_SetTdeCallBack)(IntCallBack pTdeCallBack);
	mt_void (*MTFB_DRV_WaitAllTdeDone)(MT_BOOL bSync);
	mt_s32 (*MTFB_DRV_TdeSupportFmt)(MTFB_COLOR_FMT_E fmt);
	
	mt_s32 (*MTFB_DRV_CalScaleRect)(const TDE2_RECT_S* pstSrcRect, const TDE2_RECT_S* pstDstRect,
								TDE2_RECT_S* pstRectInSrc, TDE2_RECT_S* pstRectInDst);
	mt_s32 (*MTFB_DRV_WaitForDone)(TDE_HANDLE s32Handle, mt_u32 u32TimeOut);
	
	mt_s32 (*MTFB_DRV_TdeOpen)(mt_void);
	mt_s32 (*MTFB_DRV_TdeClose)(mt_void);

}MTFB_DRV_TDEOPS_S;


/********************** Global Variable declaration **************************/



/******************************* API declaration *****************************/

mt_void MTFB_DRV_GetTdeOps(MTFB_DRV_TDEOPS_S *Ops);
mt_void MTFB_DRV_GetDevOps(MTFB_DRV_OPS_S    *Ops);

#ifdef __cplusplus

#if __cplusplus

}
#endif
#endif /* __cplusplus */

#endif /* __MTFB_DRV_H__ */

