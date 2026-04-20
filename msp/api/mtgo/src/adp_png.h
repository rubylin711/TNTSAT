/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#ifndef __ADP_PNG_H__
#define __ADP_PNG_H__

/* add include here */

#include "mt_go_config.h"

#ifdef MTGO_PNG_SUPPORT
#include "mtgo_common.h"
#include "mtgo_io.h"
#include "mtgo_surface.h"
#include "mt_go_decoder.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MTGO_PNG_SUPPORT

/***************************** Macro Definition ******************************/



/*************************** Structure Definition ****************************/



/********************** Global Variable declaration **************************/



/******************************* API declaration *****************************/
mt_s32 MTGO_ADP_PngCreateDecoder(DEC_HANDLE *pPngDec, const  MTGO_DEC_ATTR_S *pSrcDesc);

mt_s32 MTGO_ADP_PngDestroyDecoder(DEC_HANDLE PngDec);

mt_s32 MTGO_ADP_PngResetDecoder(DEC_HANDLE PngDec);

mt_s32 MTGO_ADP_PngDecCommInfo(DEC_HANDLE PngDec, MTGO_DEC_PRIMARYINFO_S *pPrimaryInfo);

mt_s32 MTGO_ADP_PngDecImgInfo(DEC_HANDLE PngDec, mt_u32 Index, MTGO_DEC_IMGINFO_S *pImgInfo);

mt_s32 MTGO_ADP_PngDecImgData(DEC_HANDLE PngDec, mt_u32 Index, MTGO_SURFACE_S *pSurface);
#if 0
mt_s32 MTGO_ADP_PngDecExtendData(DEC_HANDLE PngDec, MTGO_DEC_EXTENDTYPE_E DecExtendType, mt_void **pData,
                                 mt_u32 *pLength);

mt_s32 MTGO_ADP_PngReleaseDecExtendData(DEC_HANDLE PngDec, MTGO_DEC_EXTENDTYPE_E DecExtendType, mt_void *pData);
#endif
mt_s32 MTGO_ADP_PngGetActualSize(DEC_HANDLE PngDec, mt_s32 Index, const MT_RECT *pSrcRect, MTGO_SURINFO_S *pSurInfo);
mt_s32 MTGO_ADP_PngSetRawData(DEC_HANDLE PngDec, mt_u32 enable);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ADP_PNG_H__ */


