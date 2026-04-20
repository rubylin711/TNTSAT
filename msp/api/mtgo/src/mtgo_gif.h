/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTGO_GIF_H__
#define __MTGO_GIF_H__

/* add include here */
#include "mtgo_common.h"
#include "mt_go_decoder.h"
#include "mtgo_io.h"
#include "mt_go_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MTGO_GIF_SUPPORT

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

mt_s32 GIF_CreateDecoder(DEC_HANDLE *pGifDec, const MTGO_DEC_ATTR_S *pSrcDesc);

mt_s32 GIF_DestroyDecoder(DEC_HANDLE GifDec);

mt_s32 GIF_ResetDecoder(DEC_HANDLE GifDec);

mt_s32 GIF_DecCommInfo(DEC_HANDLE GifDec, MTGO_DEC_PRIMARYINFO_S *pPrimaryInfo);

mt_s32 GIF_DecImgInfo(DEC_HANDLE GifDec, mt_u32 Index, MTGO_DEC_IMGINFO_S *pImgInfo);

mt_s32 GIF_SetDecImgAttr(DEC_HANDLE GifDec, mt_u32 Index, const MTGO_DEC_IMGATTR_S *pImgAttr);

mt_s32 GIF_DecImgData(DEC_HANDLE GifDec, mt_u32 Index, MTGO_SURFACE_S *pSurface);

#ifdef TEST_IN_ROOTBOX
mt_s32 GIF_DecExtendData(DEC_HANDLE GifDec, MTGO_DEC_EXTENDTYPE_E DecExtendType, mt_void **pData, mt_u32 *pLength);

mt_s32 GIF_ReleaseDecExtendData(DEC_HANDLE GifDec, MTGO_DEC_EXTENDTYPE_E DecExtendType, mt_void *pData);
#endif
mt_s32 GIF_GetActualSize(DEC_HANDLE GifDec, mt_s32 Index, const MT_RECT *pSrcRect, MTGO_SURINFO_S *pSurInfo);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MTGO_GIF_H__ */


