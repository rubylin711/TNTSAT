
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_com.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_DISP_COM_H__
#define __DRV_DISP_COM_H__

#include "mt_type.h"
#include "mt_drv_video.h"
#include "mt_drv_disp.h"
#include "drv_disp_version.h"
#include "drv_disp_osal.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/*CVBS  Video Buff Type */
#define  VIDEO_BUFF_CVBS_NONE            0
#define  VIDEO_BUFF_CVBS_FMS6141            1
#define  VIDEO_BUFF_CVBS_FMS6143a           2
#define  VIDEO_BUFF_CVBS_TT_FILTER           3

/*YPbPr  ideo Buff Type */
#define  VIDEO_BUFF_YPBPR_NONE          0
#define  VIDEO_BUFF_YPBPR_FMS6363    1
#define  VIDEO_BUFF_YPBPR_DIO2176       2
#define  VIDEO_BUFF_YPBPR_TT_FILTER       3

/*DAC type */
#define   DAC_TYPE_MONTAGE_TORNADO
/* Video Buff Type!*/
#define  VIDEO_BUFF_CVBS     VIDEO_BUFF_CVBS_FMS6141
#define  VIDEO_BUFF_YPBPR  VIDEO_BUFF_YPBPR_FMS6363


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /*  __DRV_DISP_COMMON_H__  */


