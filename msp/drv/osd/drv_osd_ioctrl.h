/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_OSD_IOCTL_H__
#define __DRV_OSD_IOCTL_H__

#include "mt_type.h"
//#include "mtgo_surface.h"
//#include "mt_go_gdev.h"



#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif


typedef enum mtIOC_OSD_E
{
/*
    IOC_OSD_INIT_DISPLAY = 0,
    IOC_OSD_DEINIT_DISPLAY,
    IOC_OSD_CAPABILITY_INQUIRE,
    IOC_OSD_GET_DEFAULT_PARAM,
    IOC_OSD_CREATE_LAYER,
    IOC_OSD_DESTROY_LAYER,
    IOC_OSD_GET_CANVAS_SURFACE,
    IOC_OSD_SET_POS,
    IOC_OSD_GET_POS,
    IOC_OSD_SET_DISPLAY_SIZE,
    IOC_OSD_GET_DISPLAY_SIZE,
    IOC_OSD_SET_SCREEN_SIZE,
    IOC_OSD_GET_SCREEN_SIZE,
    IOC_OSD_SET_LAYER_ALPHA,
    IOC_OSD_GET_LAYER_ALPHA,
    IOC_OSD_SET_LAYER_COLORKEY,
    IOC_OSD_GET_LAYER_COLORKEY,
    IOC_OSD_SHOW_LAYER,
    IOC_OSD_SET_LAYER_SURFACE,
 */

IOC_GFX_INIT,
IOC_GFX_OPEN_LAYER,
IOC_GFX_CLOSE_LAYER,
IOC_GFX_OPEN_SLV_LAYER,
IOC_GFX_SET_LAYER_ALPHA,
IOC_GFX_SET_LAYER_ADDR,
IOC_GFX_SET_LAYER_STRIDE,
IOC_GFX_SET_LAYER_DATA_FMT,
IOC_GFX_SET_LAYER_RECT,
IOC_GFX_SET_LAYKEY_MASK,
IOC_WBC2ISR,
IOC_GFX_SET_ENABLE,
IOC_GFX_SET_GP_RECT,
IOC_GFX_UP_LAYER_REG,
IOC_GET_GFX_WORK_MODE,
IOC_GP_INIT_FROM_DISP,
IOC_GFX_SET_DISP_FMT_SIZE,
IOC_GFX_SET_TC_FLAG,
IOC_PRINT_OSD_INFO,
IOC_PRINT_OSD_CW_INFO,
IOC_GFX_CMP_DECMP_PROCESS,
IOC_GFX_PROC_SURFACE_INFO,
  IOC_GFX_WAIT_SYNC,


    IOC_OSD_BUTT
}IOC_OSD_E;

/*
typedef struct
{
  mt_u32  LayerID;
  union
  {
    MTGO_LAYER_CAP_S** pstruCap;
  };
 union
  {
  MTGO_LAYER_INFO_S *pLayerInfo;
  mt_handle *pSurface;
  };
 union
  {
  mt_u32 u32XStart;
  mt_u32 u32YStart;
  };
 union
  {
  mt_u32* pXStart;
  mt_u32* pYStart;
  };
 union
  {
  mt_u32 u32Width;
  mt_u32 u32Height;
  };
 union
  {
  mt_u32 *pWidth;
  mt_u32 *pHeight;
  };
 union
  {
  MTGO_LAYER_ALPHA_S *pAlphaInfo;
  };
 union
  {
  MT_BOOL bEnable;
  mt_u32 Key;
  };
 union
  {
  MT_BOOL *pbEnable;
  mt_u32 *pKey;
  };
 union
  {
   MT_BOOL bVisbile;
  };
 union
  {
  mt_handle Layer;
  MTGO_SURFACE_S *pSurface;
  MT_RECT *pRect;
  };
}OSD_IOC_PARAM_S;
*/

typedef struct
{
  MTFB_LAYER_ID_E  LayerID;
  MT_BOOL bEnable;
  MT_BOOL bMask;

  mt_u32 u32Addr;
  mt_u32 u32Stride;

  mt_u32 u32RdAddr;
  mt_u32 pic_width;
  mt_u32 pic_height;
  
  MTFB_COLOR_FMT_E enDataFmt;
  MTFB_ALPHA_S *pstAlpha;
  MTFB_RECT *pstRect;
  MTFB_COLORKEYEX_S *pstColorKey;
  
  mt_u32 u32OffSet;
  mt_u32 u32Color;
  mt_s32 UpFlag;
  mt_u32 EnableOsdc;  
}OSD_IOC_PARAM_S;

/*
#define CMD_OSD_INIT_DISPLAY                    _IO(MT_ID_OSD, IOC_OSD_INIT_DISPLAY)
#define CMD_OSD_DEINIT_DISPLAY                _IO(MT_ID_OSD, IOC_OSD_DEINIT_DISPLAY)
#define CMD_OSD_CAPABILITY_INQUIRE        _IOR(MT_ID_OSD, IOC_OSD_CAPABILITY_INQUIRE, OSD_IOC_PARAM_S)
#define CMD_OSD_GET_DEFAULT_PARAM       _IOR(MT_ID_OSD, IOC_OSD_GET_DEFAULT_PARAM, OSD_IOC_PARAM_S)
#define CMD_OSD_CREATE_LAYER                   _IOWR(MT_ID_OSD, IOC_OSD_CREATE_LAYER, OSD_IOC_PARAM_S)
#define CMD_OSD_DESTROY_LAYER                 _IO(MT_ID_OSD, IOC_OSD_DESTROY_LAYER)
#define CMD_OSD_GET_CANVAS_SURFACE      _IOR(MT_ID_OSD, IOC_OSD_GET_CANVAS_SURFACE, OSD_IOC_PARAM_S)
#define CMD_OSD_SET_POS                             _IOW(MT_ID_OSD, IOC_OSD_SET_POS, OSD_IOC_PARAM_S)
#define CMD_OSD_GET_POS                             _IOR(MT_ID_OSD, IOC_OSD_GET_POS, OSD_IOC_PARAM_S)
#define CMD_OSD_SET_DISPLAY_SIZE            _IOW(MT_ID_OSD, IOC_OSD_SET_DISPLAY_SIZE, OSD_IOC_PARAM_S)
#define CMD_OSD_GET_DISPLAY_SIZE            _IOR(MT_ID_OSD, IOC_OSD_GET_DISPLAY_SIZE, OSD_IOC_PARAM_S)
#define CMD_OSD_SET_SCREEN_SIZE             _IOW(MT_ID_OSD, IOC_OSD_SET_SCREEN_SIZE, OSD_IOC_PARAM_S)
#define CMD_OSD_GET_SCREEN_SIZE              _IOR(MT_ID_OSD, IOC_OSD_GET_SCREEN_SIZE, OSD_IOC_PARAM_S)
#define CMD_OSD_SET_LAYER_ALPHA             _IOW(MT_ID_OSD, IOC_OSD_SET_LAYER_ALPHA, OSD_IOC_PARAM_S)
#define CMD_OSD_GET_LAYER_ALPHA             _IOR(MT_ID_OSD, IOC_OSD_GET_LAYER_ALPHA, OSD_IOC_PARAM_S)
#define CMD_OSD_SET_LAYER_COLORKEY      _IOW(MT_ID_OSD, IOC_OSD_SET_LAYER_COLORKEY, OSD_IOC_PARAM_S)
#define CMD_OSD_GET_LAYER_COLORKEY      _IOR(MT_ID_OSD, IOC_OSD_GET_LAYER_COLORKEY, OSD_IOC_PARAM_S)
#define CMD_OSD_SHOW_LAYER                      _IOW(MT_ID_OSD, IOC_OSD_SHOW_LAYER, OSD_IOC_PARAM_S)
#define CMD_OSD_SET_LAYER_SURFACE         _IOW(MT_ID_OSD, IOC_OSD_SET_LAYER_SURFACE, OSD_IOC_PARAM_S)
*/


#define CMD_IOC_GFX_INIT                                    _IO(MT_ID_OSD, IOC_GFX_INIT)
#define CMD_IOC_GFX_OPEN_LAYER                       _IOWR(MT_ID_OSD, IOC_GFX_OPEN_LAYER, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_CLOSE_LAYER                     _IOWR(MT_ID_OSD, IOC_GFX_CLOSE_LAYER, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_OPEN_SLV_LAYER               _IOWR(MT_ID_OSD, IOC_GFX_OPEN_SLV_LAYER, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_SET_LAYER_ALPHA            _IOWR(MT_ID_OSD, IOC_GFX_SET_LAYER_ALPHA, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_SET_LAYER_ADDR              _IOWR(MT_ID_OSD, IOC_GFX_SET_LAYER_ADDR, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_SET_LAYER_STRIDE            _IOWR(MT_ID_OSD, IOC_GFX_SET_LAYER_STRIDE, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_SET_LAYER_DATA_FMT      _IOWR(MT_ID_OSD, IOC_GFX_SET_LAYER_DATA_FMT, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_SET_LAYER_RECT               _IOWR(MT_ID_OSD, IOC_GFX_SET_LAYER_RECT, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_SET_LAYKEY_MASK             _IOWR(MT_ID_OSD, IOC_GFX_SET_LAYKEY_MASK, OSD_IOC_PARAM_S)
#define CMD_IOC_WBC2ISR                                      _IOWR(MT_ID_OSD, IOC_WBC2ISR, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_SET_ENABLE                        _IOWR(MT_ID_OSD, IOC_GFX_SET_ENABLE, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_SET_GP_RECT                        _IOWR(MT_ID_OSD, IOC_GFX_SET_GP_RECT, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_UP_LAYER_REG                    _IOWR(MT_ID_OSD, IOC_GFX_UP_LAYER_REG, OSD_IOC_PARAM_S)
#define CMD_IOC_GET_GFX_WORK_MODE                 _IOWR(MT_ID_OSD, IOC_GET_GFX_WORK_MODE, OSD_IOC_PARAM_S)
#define CMD_IOC_GP_INIT_FROM_DISP                      _IOWR(MT_ID_OSD, IOC_GP_INIT_FROM_DISP, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_SET_DISP_FMT_SIZE            _IOWR(MT_ID_OSD, IOC_GFX_SET_DISP_FMT_SIZE, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_SET_TC_FLAG                       _IOWR(MT_ID_OSD, IOC_GFX_SET_TC_FLAG, OSD_IOC_PARAM_S)
#define CMD_IOC_PRINT_OSD_INFO                         _IOWR(MT_ID_OSD, IOC_PRINT_OSD_INFO, OSD_IOC_PARAM_S)
#define CMD_IOC_PRINT_OSD_CW_INFO                 _IOWR(MT_ID_OSD, IOC_PRINT_OSD_CW_INFO, OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_CMP_DECMP_PROCESS         _IOWR(MT_ID_OSD, IOC_GFX_CMP_DECMP_PROCESS , OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_PROC_SURFACE_INFO         _IOWR(MT_ID_OSD, IOC_GFX_PROC_SURFACE_INFO , OSD_IOC_PARAM_S)
#define CMD_IOC_GFX_WAIT_SYNC                 _IO(MT_ID_OSD, IOC_GFX_WAIT_SYNC)



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif



