/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */


/* add include here */

#include "mt_go_config.h"

#ifdef MTGO_JPEG_SUPPORT
#include "mtgo_common.h"
#include "mtgo_surface.h"
#include "mt_go_decoder.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef MTGO_JPEG_SUPPORT

/***************************** Macro Definition ******************************/



/*************************** Structure Definition ****************************/



/********************** Global Variable declaration **************************/



/******************************* API declaration *****************************/
mt_s32 MTGO_ADP_JPGCreateDecoder(DEC_HANDLE *pJpegDec, const MTGO_DEC_ATTR_S *pSrcDesc);
mt_s32 MTGO_ADP_JPGDestroyDecoder(DEC_HANDLE JpegDec);
mt_s32 MTGO_ADP_JPGResetDecoder(DEC_HANDLE JpegDec);
mt_s32 MTGO_ADP_JPGDecCommInfo(DEC_HANDLE JpegDec, MTGO_DEC_PRIMARYINFO_S *pPrimaryInfo);
mt_s32 MTGO_ADP_JPGDecImgInfo(DEC_HANDLE JpegDec, mt_u32 Index, MTGO_DEC_IMGINFO_S *pImgInfo);
mt_s32 MTGO_ADP_JPGDecImgData(DEC_HANDLE JpegDec, mt_u32 Index, MTGO_SURFACE_S *pSurface);
mt_s32 MTGO_ADP_JPGGetActualSize(DEC_HANDLE JpegDec, MT_S32 Index, const MT_RECT *pSrcRect, MTGO_SURINFO_S *pSurInfo);
mt_s32 MTGO_ADP_JPGSetRotate(DEC_HANDLE JpegDec, MTGO_DEC_ROTATE_E rotate);
mt_s32 MTGO_ADP_JPGGetCropRect(DEC_HANDLE JpegDec, MT_RECT *pCropRect);


#ifdef __cplusplus
}
#endif

#endif /* __ADP_PNG_H__ */


