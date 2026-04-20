/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_OSD_COMM_H__
#define __DRV_OSD_COMM_H__

/* add include here */
#include "linux/fb.h"

//#include "mtgo_surface.h"
//#include "mtgo_gdev.h"
#include "drv_display.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    mt_s32 (*pfnOsdGetHeaderInfo)(MT_DRV_DISP_LAYER_ID_E disp_layer, osd_header_info_s *info);
} OSD_EXPORT_FUNC_S;



#ifdef __cplusplus
extern "C" }
#endif


#endif // #ifnde __DRV_OSD_COMM_H__

