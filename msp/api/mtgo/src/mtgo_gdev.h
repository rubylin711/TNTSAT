/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTGO_GDEV_H__
#define __MTGO_GDEV_H__

/* add include here */
#include "mtgo_common.h"
#include "mt_go_surface.h"
#include "mt_go_gdev.h"

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Macro Definition ******************************/



#define MTGO_SD_MAXWIDTH  720
#define MTGO_SD_MAXHEIGHT 576
#define MTGO_SD_MINWIDTH  8
#define MTGO_SD_MINHEIGHT 8

#define MTGO_HD_MAXWIDTH  3840
#define MTGO_HD_MAXHEIGHT 2160
#define MTGO_HD_MINWIDTH  8
#define MTGO_HD_MINHEIGHT 8
#define MTGO_MAX_FLIPBUF_NUM 2

/*************************** Structure Definition ****************************/

typedef struct _MTGO_LAYER_REC_S
{
    mt_u32                          LayerID;       /**< 硬件图层ID */
    mt_handle                      hLayer;        /**< 图层句柄 */
    MT_BOOL                       bCreated;      /**< 图层是否已经被创建 */
    MTGO_LAYER_INFO_S     LayerInfo;     /**< 图层信息*/
    mt_handle                      hLayerSurface;  /**< 图层surface句柄*/
} MTGO_LAYER_REC_S;

typedef struct
{
    mt_s32 s32XPos;         /**<  horizontal position *//**<CNcommnet:水平位置 */
    mt_s32 s32YPos;         /**<  vertical position *//**<CNcomment:垂直位置 */
}MTGO_POINT_S;

typedef struct
{
    MT_BOOL   bKeyEnable;      /*colorkey enable flag*//*CNcomment:colorkey 是否使能*/
    MT_BOOL   bMaskEnable;    /*key mask enable flag*//*CNcomment:key mask 是否使能*/
    mt_u32      u32Key;              /*key value*/
    mt_u8        u8RedMask;          /*red mask*/
    mt_u8        u8GreenMask;       /*green mask*/
    mt_u8        u8BlueMask;          /*blue mask*/
    mt_u8        u8Reserved;           
    mt_u32      u32KeyMode;	 /*0:In region; 1:Out region*/

    /*Max colorkey value of red component*/
    /*CNcomment:colorkey红色分量最大值*/
    mt_u8 u8RedMax;

    /*Max colorkey value of Green component*/
    /*CNcomment:colorkey绿色分量最大值*/
    mt_u8 u8GreenMax; 

    /*Max colorkey value of blue component*/
    /*CNcomment:colorkey蓝色分量最大值*/
    mt_u8 u8BlueMax;           
    mt_u8 u8Reserved1;

    /*Min colorkey value of red component*/
    /*CNcomment:colorkey红色分量最小值*/
    mt_u8 u8RedMin;            

    /*Min colorkey value of Green component*/
    /*CNcomment:colorkey绿色分量最小值*/
    mt_u8 u8GreenMin;         

    /*Min colorkey value of blue component*/
    /*CNcomment:colorkey蓝色分量最小值*/
    mt_u8 u8BlueMin;            
    mt_u8 u8Reserved2;
}MTGO_COLORKEYEX_S;

/********************** Global Variable declaration **************************/

mt_s32 MTGO_InitDisplay(mt_void);
mt_s32 MTGO_DinitDisplay(mt_void);

#ifdef __cplusplus
}
#endif
#endif /* __MTGO_GDEV_H__ */


