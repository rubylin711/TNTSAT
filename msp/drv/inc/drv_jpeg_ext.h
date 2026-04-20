/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_JPEG_EXT_H__
#define __DRV_JPEG_EXT_H__

/*********************************add include here******************************/

#include "mt_drv_dev.h"

/*****************************************************************************/


/*****************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/


/*************************** Structure Definition ****************************/

/***************************  The enum of Jpeg image format  ******************/

/********************** Global Variable declaration **************************/


/******************************* API declaration *****************************/

typedef MT_S32  (*FN_JPEG_Suspend)(basedev_s *, pm_message_t);
typedef MT_S32  (*FN_JPEG_Resume)(basedev_s *);

typedef struct
{
	FN_JPEG_Suspend			pfnJpegSuspend;
	FN_JPEG_Resume			pfnJpegResume;
}JPEG_EXPORT_FUNC_S;


MT_VOID JPEG_DRV_ModExit(MT_VOID);

MT_S32 JPEG_DRV_ModInit(MT_VOID);
    
#ifdef __cplusplus
#if __cplusplus
}      
#endif
#endif /* __cplusplus */

#endif /*__DRV_JPEG_EXT_H__ */
