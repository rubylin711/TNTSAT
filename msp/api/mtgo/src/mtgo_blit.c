/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

//lint -wlib(0)
#include "string.h"

#include "mtgo_common.h"
#include "mtgo_blit.h"
#include "adp_gfx.h"
#include "mtgo_adp_sys.h"

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

mt_s32 Bliter_Blit(MTGO_HANDLE SrcSurface, const MT_RECT *pSrcRect,
                   MTGO_HANDLE DstSurface, const MT_RECT *pDstRect,
                   const MTGO_BLTOPT2_S *pBlitOpt)
{
    mt_s32 s32Ret;

    s32Ret = MTGO_ADP_GFXBlit((MTGO_SURFACE_S *)SrcSurface, pSrcRect, (MTGO_SURFACE_S *)DstSurface, pDstRect, pBlitOpt);
    if (s32Ret == MT_SUCCESS) {
	return MT_SUCCESS;
    }

    if (MTGO_ERR_UNSUPPORTED != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

    return s32Ret;
}

mt_s32 Bliter_StretchBlit(MTGO_HANDLE SrcSurface, const MT_RECT *pSrcRect,
                          MTGO_HANDLE DstSurface, const MT_RECT *pDstRect,
                          const MTGO_BLTOPT2_S *pBlitOpt)
{
    mt_s32 s32Ret;
    //MTGO_SURFACE_S * pSrc = (MTGO_SURFACE_S * )SrcSurface;

    //MTGO_SURFACE_S * pDst = (MTGO_SURFACE_S * )DstSurface;
    //printf("Blit1 SRC pf %d w %d h %d pitch %d\n",pSrc ->PixelFormat,pSrc ->Width,pSrc ->Height,pSrc ->Data[0].Pitch);
    //printf("Blit2 Dst pf %d w %d h %d pitch %d\n",pDst ->PixelFormat,pDst ->Width,pDst ->Height,pDst ->Data[0].Pitch);
    //printf("pDstRect.w %d,pDstRect.h %d\n",pDstRect->w,pDstRect->h);

    s32Ret = MTGO_ADP_GFXStretchBlit((MTGO_SURFACE_S *)SrcSurface, pSrcRect, (MTGO_SURFACE_S *)DstSurface, pDstRect, pBlitOpt);
    if (s32Ret == MT_SUCCESS) {
	return MT_SUCCESS;
    }

    if (MTGO_ERR_UNSUPPORTED != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }
    return s32Ret;
}

mt_s32 Bliter_BlitEx(MTGO_HANDLE SrcSurface, const MT_RECT *pSrcRect,
					MTGO_HANDLE BckSurface, const MT_RECT *pBckRect,
                   MTGO_HANDLE DstSurface, const MT_RECT *pDstRect,
                   const MTGO_BLTOPT2_S *pBlitOpt)
{
    mt_s32 s32Ret;

    s32Ret = MTGO_ADP_GFXBlitEx((MTGO_SURFACE_S *)SrcSurface, pSrcRect, (MTGO_SURFACE_S *)BckSurface, pBckRect,
						(MTGO_SURFACE_S *)DstSurface, pDstRect, pBlitOpt);
    if (s32Ret == MT_SUCCESS) {
	return MT_SUCCESS;
    }

    if (MTGO_ERR_UNSUPPORTED != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

    return s32Ret;
}

mt_s32 Bliter_StretchBlitEx(MTGO_HANDLE SrcSurface, const MT_RECT *pSrcRect,
							MTGO_HANDLE BckSurface, const MT_RECT *pBckRect,
                          MTGO_HANDLE DstSurface, const MT_RECT *pDstRect,
                          const MTGO_BLTOPT2_S *pBlitOpt)
{
    mt_s32 s32Ret;
    //MTGO_SURFACE_S * pSrc = (MTGO_SURFACE_S * )SrcSurface;

    //MTGO_SURFACE_S * pDst = (MTGO_SURFACE_S * )DstSurface;
    //printf("Blit1 SRC pf %d w %d h %d pitch %d\n",pSrc ->PixelFormat,pSrc ->Width,pSrc ->Height,pSrc ->Data[0].Pitch);
    //printf("Blit2 Dst pf %d w %d h %d pitch %d\n",pDst ->PixelFormat,pDst ->Width,pDst ->Height,pDst ->Data[0].Pitch);
    //printf("pDstRect.w %d,pDstRect.h %d\n",pDstRect->w,pDstRect->h);

    s32Ret = MTGO_ADP_GFXStretchBlitEx((MTGO_SURFACE_S *)SrcSurface, pSrcRect, (MTGO_SURFACE_S *)BckSurface, pBckRect,
    							(MTGO_SURFACE_S *)DstSurface, pDstRect, pBlitOpt);
    if (s32Ret == MT_SUCCESS) {
	return MT_SUCCESS;
    }

    if (MTGO_ERR_UNSUPPORTED != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }
    return s32Ret;
}

mt_s32 Bliter_FillRect(MTGO_HANDLE pSurface, const MT_RECT *pRect, MT_COLOR Color, MTGO_COMPOPT_E CompositeOpt)
{
    mt_s32 s32Ret;
    MTGO_BLTOPT2_S BlitOpt;
    MTGO_OPRECT_S FillRect;

    MTGO_MemSet(&BlitOpt, 0, sizeof(MTGO_BLTOPT2_S));
    BlitOpt.PixelAlphaComp = CompositeOpt;
    MTGO_MemCopy(&FillRect.Rect, pRect, sizeof(MT_RECT));
    FillRect.Color = Color;
    s32Ret = MTGO_ADP_GFXOperate((MTGO_SURFACE_S *)pSurface, GFX_OPT_FILLRECT, (const mt_void *)&FillRect, &BlitOpt);
    if (s32Ret == MT_SUCCESS) {
	return MT_SUCCESS;
    }

    if (MTGO_ERR_UNSUPPORTED != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }
    return s32Ret;
}

mt_s32 Bliter_MaskBlit(MTGO_HANDLE SrcSurface, const MT_RECT *pSrcRect,
                       MTGO_HANDLE MaskSurface, const MT_RECT *pMaskRect,
                       MTGO_HANDLE DstSurface, const MT_RECT *pDstRect,
                       const MTGO_BLTOPT2_S *pBlitOpt)
{
    mt_s32 s32Ret;

    s32Ret = MTGO_ADP_GFXMaskBlit((MTGO_SURFACE_S *)SrcSurface, pSrcRect, (MTGO_SURFACE_S *)MaskSurface, pMaskRect, (MTGO_SURFACE_S *)DstSurface, pDstRect, pBlitOpt);
    if (s32Ret == MT_SUCCESS) {
	return MT_SUCCESS;
    }

    if (MTGO_ERR_UNSUPPORTED != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

    return s32Ret;
}

MT_S32 Bliter_RotateMirror (MTGO_HANDLE SrcSurface, const MT_RECT* pSrcRect, 
                                                    MTGO_HANDLE DstSurface, const MT_RECT *pDstRect,
                                                    const MTGO_ROTATE_E rotate_mod, const MTGO_MIRROR_E mirror_mod)
{
    MT_S32 ret;
	ret = MTGO_ADP_RotateMirror((const MTGO_SURFACE_S *)SrcSurface, pSrcRect, (const MTGO_SURFACE_S *)DstSurface, 
    pDstRect, rotate_mod, mirror_mod);
    return ret;                                                    
}


mt_s32 Bliter_MEMSurfaceToTDESurface(MTGO_HANDLE Surface, mt_void *pTDESurface)
{
  return ADP_MEMSurfaceToTDESurface((MTGO_SURFACE_S *)Surface, pTDESurface);
}
mt_s32 Bliter_MEMSurfaceToTDEMBSurface(MTGO_HANDLE Surface, mt_void *pTDEMBSurface)
{
	return ADP_MEMSurfaceToTDEMBSurface((MTGO_SURFACE_S *)Surface, pTDEMBSurface);
}

mt_s32 Bliter_GenerateTDEOpt(MTGO_HANDLE SrcSurface,
                                 MTGO_HANDLE DstSurface,
                                 MTGO_BLTOPT2_S *pBlitOpt,
                                 mt_void *pTDEOpt, MT_BOOL bScale)
{
	return ADP_GenerateTDEOpt((MTGO_SURFACE_S *)SrcSurface, (MTGO_SURFACE_S *)DstSurface, pBlitOpt, pTDEOpt, bScale);
}

mt_s32 Bliter_ConvertFormat(mt_u32 ColorFmt, mt_u32 *pX5ColorFmt)
{
	return ADP_ConvertFormat(ColorFmt, (TDE2_COLOR_FMT_E *)pX5ColorFmt);
}

