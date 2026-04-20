/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_GO_CONFIG_H__
#define __MT_GO_CONFIG_H__

/* add include here */
#ifdef __cplusplus
extern "C" {
#endif

/***************************** Macro Definition ******************************/
#define MTGO_BMP_SUPPORT
#define MTGO_GIF_SUPPORT

#ifdef CONFIG_MT_MTGO_JPEG_SUPPORT
#define MTGO_JPEG_SUPPORT
#endif

#ifndef MT_BUILD_LOADER
#define MTGO_PNG_SUPPORT    
#define MTGO_TEXT_SUPPORT
#endif

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

#ifdef __cplusplus
}
#endif
#endif /* __MT_GO_CONFIG_H__ */


