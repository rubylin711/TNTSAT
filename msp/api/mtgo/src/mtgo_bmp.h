/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#ifndef __MTGO_BMP_H__
#define __MTGO_BMP_H__

#include "mt_type.h"
#include "mtgo_common.h"
#include "mtgo_surface.h"
#include "mtgo_io.h"
#include "mt_go_config.h"

/* add include here */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef MTGO_BMP_SUPPORT

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

mt_s32 BMP_CreateDecoder(DEC_HANDLE *pBmpDec, const MTGO_DEC_ATTR_S *pSrcDesc);

mt_s32 BMP_DestroyDecoder(DEC_HANDLE BmpDec);

mt_s32 BMP_ResetDecoder(DEC_HANDLE BmpDec);

mt_s32 BMP_DecCommInfo(DEC_HANDLE BmpDec, MTGO_DEC_PRIMARYINFO_S *pPrimaryInfo);

mt_s32 BMP_DecImgInfo(DEC_HANDLE BmpDec, mt_u32 Index, MTGO_DEC_IMGINFO_S *pImgInfo);

mt_s32 BMP_DecImgData(DEC_HANDLE BmpDec, mt_u32 Index, MTGO_SURFACE_S *pSurface);

mt_s32 BMP_ReleaseDecImgData(DEC_HANDLE BmpDec, MTGO_DEC_IMGDATA_S *pImgData);

#ifdef TEST_IN_ROOTBOX
mt_s32 BMP_DecExtendData(DEC_HANDLE BmpDec, MTGO_DEC_EXTENDTYPE_E DecExtendType, mt_void **pData, mt_u32 *pLength);

mt_s32 BMP_ReleaseDecExtendData(DEC_HANDLE BmpDec, MTGO_DEC_EXTENDTYPE_E DecExtendType, mt_void *pData);
#endif
mt_s32 BMP_GetActualSize(DEC_HANDLE BmpDec, mt_s32 Index, const MT_RECT *pSrcRect, MTGO_SURINFO_S *pSurInfo);

#endif

#ifdef __cplusplus
}
#endif
#endif /* __MTGO_BMP_H__ */


