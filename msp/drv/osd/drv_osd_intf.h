/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_OSD_INTF_H__
#define __DRV_OSD_INTF_H__

#ifdef CFG_MTGO_PROC_SUPPORT
typedef struct mtMTGO_PROC_INFO_S
{

  phys_addr_t layerAddr[MTFB_LAYER_ID_BUTT];
  phys_addr_t headerAddr[MTFB_LAYER_ID_BUTT];
  mt_u32 layerPitch[MTFB_LAYER_ID_BUTT];
  MT_BOOL layerUsed[MTFB_LAYER_ID_BUTT];
  MT_BOOL layerVisible[MTFB_LAYER_ID_BUTT];
  mt_u32 mmzMemUsed;
  mt_u32 sysMemUsed;
  mt_u32 format[MTFB_LAYER_ID_BUTT];
  MTFB_RECT rect[MTFB_LAYER_ID_BUTT];
  mt_u32 alpha[MTFB_LAYER_ID_BUTT];
  mt_u32 colorkey[MTFB_LAYER_ID_BUTT];
  mt_u32 surfaceUsed[MTFB_LAYER_ID_BUTT];
  mt_u32 surWidth[MTFB_LAYER_ID_BUTT];
  mt_u32 surHeight[MTFB_LAYER_ID_BUTT];
  mt_u32 surPitch[MTFB_LAYER_ID_BUTT];
  mt_u32 surPhyAddr[MTFB_LAYER_ID_BUTT];
  mt_u32 surFormat[MTFB_LAYER_ID_BUTT];
  mt_u32 surAlpha[MTFB_LAYER_ID_BUTT];
}MT_MTGO_PROC_INFO_S;

#endif

#if 0


/* add include here */
#include "linux/fb.h"

//#include "mtgo_surface.h"
//#include "mtgo_gdev.h"


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Macro Definition ******************************/
/** 图层最多个数, 此数据是可以改变的*/
//#ifndef MTGO_CODE_CUT
#define MTGO_LAYER_MAXCOUNT 6
//#else
//#define MTGO_LAYER_MAXCOUNT 4
//#endif

/** 图层能力集结构 */
typedef struct _MTGO_LAYER_CAP_S
{
    mt_u8              u8FmtCount;       /**< 支持的像素格式的个数 */
    MTGO_PF_E*    enPixelFmt;       /**< 图层支持的像素格式 */
    MT_BOOL         bPremultiply;     /**< 支持预乘格式 */
    MT_BOOL         bNonPremultiply;  /**< 支持非预乘格式 */
    mt_s32            MaxWidth;     
    mt_s32            MaxHeight;
    mt_s32            MinWidth;     
    mt_s32            MinHeight;
    mt_s32            MaxDisplayWidth;     
    mt_s32            MaxDisplayHeight;
    MT_BOOL         bVoScale;
} MTGO_LAYER_CAP_S;

#if 0
struct fb_bitfield {
	mt_u32 offset;			/* beginning of bitfield	*/
	mt_u32 length;			/* length of bitfield		*/
	mt_u32 msb_right;		/* != 0 : Most significant bit is */ 
					/* right */ 
};
#endif

/** 位域信息结构 */
typedef struct _MTGO_BITFIELD_S
{
    struct fb_bitfield red;   /**< 红色位域信息 */
    struct fb_bitfield green; /**< 绿色位域信息 */
    struct fb_bitfield blue;  /**< 蓝色位域信息 */
    struct fb_bitfield transp;/**< 透明色位域信息 */
} MTGO_BITFIELD_S;


#ifndef MTGO_CODE_CUT
extern const MTGO_BITFIELD_S s_BitField[9] ;
#endif

mt_void ADP_Layer_ConvertFieldInfo(MTGO_PF_E enType, MTGO_BITFIELD_S *pstFieldInfo);
mt_s32 MTGO_ADP_InitDisplay(mt_void);
mt_void MTGO_ADP_CapabilityInquire(mt_u32 LayerID,  MTGO_LAYER_CAP_S** pstruCap);
mt_s32 MTGO_ADP_GetDefaultParam(mt_u32  LayerID, MTGO_LAYER_INFO_S *pLayerInfo);



/*************************** Structure Definition ****************************/

struct _MTGO_LAYER_ADP_S
{
    mt_s32  (*InitDisplay)(mt_void);              /**< 系统初始化 */
    mt_void (*DeinitDisplay)(mt_void);          /**< 系统去初始化*/
    mt_s32  (*GetDefautParam)(mt_u32 LayerID, MTGO_LAYER_INFO_S *pLayerInfo);
    mt_s32  (*CreateLayer)(mt_u32  LayerID,  MTGO_LAYER_INFO_S *pLayerInfo);/**< 创建图层*/
    mt_void  (*DestroyLayer)(mt_u32 LayerID);/**< 销毁图层 */

	mt_s32  (*SetLayerSurface)(mt_u32 LayerID, const MTGO_SURFACE_S *pSurface, MT_RECT *pRect);  /**刷新图层CANVAS BUFFER*/
	mt_s32  (*GetCanvasSurface)(mt_u32 LayerID, const MTGO_LAYER_INFO_S *pLayerInfo, mt_handle *pSurface); /**获取图层CANVAS BUFFER*/
  //mt_s32  (*SetLayerZorder)(mt_u32 LayerID, MTGO_LAYER_ZORDER_E enZFlag);/**< 设置图层Z序*/
  //mt_s32  (*GetLayerZorder)(mt_u32 LayerID, mt_u32* u32ZFlag);
    mt_s32  (*ShowLayer)(mt_u32 LayerID, MT_BOOL bVisbile);/**< 显示图层*/
//    mt_s32  (*GetLayerShowState)(mt_u32 LayerID, MT_BOOL *pbVisbile);/**< 显示图层*/ 
    mt_s32  (*SetLayerAlpha)(mt_u32 LayerID, const MTGO_LAYER_ALPHA_S *pAlphaInfo); /**< 设置ALPHA函数 */
   // mt_s32  (*SetLayerColorKey)(mt_u32 LayerID, MT_BOOL bEnable, mt_u32 Key);/**< 设置COLORKEY */
    mt_s32  (*GetLayerAlpha)(mt_u32 LayerID, MTGO_LAYER_ALPHA_S *pAlphaInfo); /**< 获取ALPHA函数 */
#ifndef MTGO_CODE_CUT
    mt_s32  (*SetLayerColorKey)(mt_u32 LayerID, MT_BOOL bEnable, mt_u32 Key);/**< 设置COLORKEY */
    mt_s32  (*GetLayerColorKey)(mt_u32 LayerID,MT_BOOL *pbEnable, mt_u32 *pKey);/**< 设置COLORKEY */
   // mt_s32  (*GetPalette)(mt_u32 LayerID, HI_PALETTE pPalette);/**< 设置调色板 */
    mt_s32  (*SetPos)( mt_u32 LayerID, mt_u32 u32XStart, mt_u32 u32YStart);   /**< 修改图层在屏幕上的显示位置 */
    mt_s32  (*GetPos)(mt_u32 LayerID, mt_u32* pXStart, mt_u32* pYStart);   /**< 获取图层在屏幕上的显示位置 */
	mt_s32  (*SetScreenSize)(mt_u32 LayerID, mt_u32 u32Width, mt_u32 u32Height);       /**< 获取屏幕的高宽 */
	mt_s32  (*GetScreenSize)(mt_u32 LayerID, mt_u32 *pWidth, mt_u32 *pHeight);       /**< 获取屏幕的高宽 */	
	mt_s32  (*SetDisplaySize)(mt_u32 LayerID, mt_u32 u32Width, mt_u32 u32Height);       /**< 设置中间显示buffer的高宽 */
	mt_s32  (*GetDisplaySize)(mt_u32 LayerID, mt_u32 *pWidth, mt_u32 *pHeight);       /**< 获取中间显示buffer的高宽 */	

    mt_s32  (*RefreshLayer)(mt_u32 LayerID, const MT_RECT *pRect);    
    mt_void (*CapabilityInquire)(mt_u32 LayerID,  MTGO_LAYER_CAP_S** pstruCap);
    mt_u32  LayerCount;
#endif    
};


typedef struct _MTGO_LAYER_ADP_S MTGO_LAYER_ADP_S;

/******************************* API declaration *****************************/

mt_s32 MTGO_ADP_CreateVideoDevice(MTGO_LAYER_ADP_S *thiz);

#endif

#ifdef __cplusplus
}
#endif
#endif   /* __DRV_OSD_INTF_H__ */


