/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __HI3720_GFXDRIVER_H__
#define __HI3720_GFXDRIVER_H__

/* add include here */
#include "mt_tde_api.h"
#include "mt_tde_errcode.h"
#include "mt_tde_type.h"

#include <core/layers.h>
#include <core/screens.h>

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/

typedef struct {
    /* validation flags */
    int v_flags;

    /* cached/computed values */
    TDE2_SURFACE_S src_surface;
    TDE2_SURFACE_S dst_surface;
    TDE2_SURFACE_S mask_surface;
    TDE2_SURFACE_S bg_surface;

    unsigned long color_pixel;

    /** Add shared data here... **/
    TDE2_RECT_S clip_rect;
    unsigned long src_colorkey;
    unsigned long dst_colorkey;
    DFBSurfaceBlendFunction src_blend;
    DFBSurfaceBlendFunction dst_blend;
    DFBSurfaceBlittingFlags blittingflags;
    DFBSurfaceDrawingFlags drawingflags;
    unsigned long lut_offset;

    TDE_HANDLE cmd_fifo_handle;
    int enable_cmd_fifo_sync_mode;
} TDEDeviceData;

typedef struct {

    /** Add local data here... **/
} TDEDriverData;

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

extern ScreenFuncs mtPrimaryScreenFuncs;
extern DisplayLayerFuncs mtPrimaryLayerFuncs;
extern DisplayLayerFuncs mtOldPrimaryFuncs;
extern void *mtOldPrimaryDriverData;

#ifdef __cplusplus
}
#endif
#endif /* __HI3720_GFXDRIVER_H__ */
