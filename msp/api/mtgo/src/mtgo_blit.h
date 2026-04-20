/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTGO_BLIT_H__
#define __MTGO_BLIT_H__

/* add include here */
#include "mt_go_bliter.h"
#include "mtgo_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/
typedef struct
{
    MT_RECT Rect;
    MT_COLOR Color;
} MTGO_OPRECT_S;

typedef enum {
    GFX_OPT_DRAWLINE = 0, //draw line
    GFX_OPT_FILLRECT,
    GFX_OPT_DRAWRECT,
    GFX_OPT_DRAWCIRCLE,
    GFX_OPT_DRAWELLIPSE,
    GFX_OPT_BUTT
} GFX_OPT_TYPE_E;
/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

mt_s32 MTGO_InitBliter(mt_void);

mt_s32 MTGO_DeinitBliter(mt_void);

mt_s32 Bliter_Blit(MTGO_HANDLE SrcSurface, const MT_RECT *pSrcRect,
                   MTGO_HANDLE DstSurface, const MT_RECT *pDstRect,
                   const MTGO_BLTOPT2_S *pBlitOpt);

mt_s32 Bliter_StretchBlit(MTGO_HANDLE SrcSurface, const MT_RECT *pSrcRect,
                          MTGO_HANDLE DstSurface, const MT_RECT *pDstRect,
                          const MTGO_BLTOPT2_S *pBlitOpt);
mt_s32 Bliter_FillRect(MTGO_HANDLE pSurface, const MT_RECT *pRect, MT_COLOR Color, MTGO_COMPOPT_E CompositeOpt);

mt_s32 Bliter_MaskBlit(MTGO_HANDLE SrcSurface, const MT_RECT *pSrcRect,
                       MTGO_HANDLE MaskSurface, const MT_RECT *pMaskRect,
                       MTGO_HANDLE DstSurface, const MT_RECT *pDstRect,
                       const MTGO_BLTOPT2_S *pBlitOpt);

mt_s32 Bliter_MEMSurfaceToTDESurface(MTGO_HANDLE Surface, mt_void *pTDESurface);

mt_s32 Bliter_MEMSurfaceToTDEMBSurface(MTGO_HANDLE Surface, mt_void *pTDEMBSurface);

mt_s32 Bliter_GenerateTDEOpt(MTGO_HANDLE SrcSurface,
                                 MTGO_HANDLE DstSurface,
                                 MTGO_BLTOPT2_S *pBlitOpt,
                                 mt_void *pTDEOpt, MT_BOOL bScale);

mt_s32 Bliter_ConvertFormat(mt_u32 ColorFmt, mt_u32 *pX5ColorFmt);

#ifdef TEST_IN_ROOTBOX
mt_s32 Bliter_GetPalIndex(MT_COLOR Color, mt_u8 *pIndex, const MT_PALETTE Palette);
#endif
MT_S32 Bliter_RotateMirror (MTGO_HANDLE SrcSurface, const MT_RECT* pSrcRect, 
                                                    MTGO_HANDLE DstSurface, const MT_RECT *pDstRect,
                                                    const MTGO_ROTATE_E rotate_mod, const MTGO_MIRROR_E mirror_mod);
mt_s32 Bliter_StretchBlitEx(MTGO_HANDLE SrcSurface, const MT_RECT *pSrcRect,
							MTGO_HANDLE BckSurface, const MT_RECT *pBckRect,
                          MTGO_HANDLE DstSurface, const MT_RECT *pDstRect,
                          const MTGO_BLTOPT2_S *pBlitOpt);
mt_s32 Bliter_BlitEx(MTGO_HANDLE SrcSurface, const MT_RECT *pSrcRect,
					MTGO_HANDLE BckSurface, const MT_RECT *pBckRect,
                   MTGO_HANDLE DstSurface, const MT_RECT *pDstRect,
                   const MTGO_BLTOPT2_S *pBlitOpt);



#ifdef __cplusplus
}
#endif
#endif /* __MTGO_BLIT_H__ */
