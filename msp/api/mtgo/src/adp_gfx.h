/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#ifndef __ADP_GFX_H__
#define __ADP_GFX_H__

#include "mtgo_surface.h"
#include "mtgo_blit.h"
#include "mt_tde_type.h"
/* add include here */

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

/**  a collection about 2D module query function*/
mt_s32 MTGO_ADP_InitBlitter(mt_void);

mt_void MTGO_ADP_DeinitBlitter(mt_void);
#if 0

mt_s32 ADP_GFXBlitBility(const MTGO_SURFACE_S *pSrcSurface, const MTGO_SURFACE_S *pDstSurface, const MTGO_BLTOPT2_S *pBlitOpt);

mt_s32 ADP_GFXStretchBlitBility(const MTGO_SURFACE_S *pSrcSurface, const MTGO_SURFACE_S *pDstSurface, const MTGO_BLTOPT2_S *pBlitOpt);

mt_s32 ADP_GFXOperateBility(const MTGO_SURFACE_S *pDstSurface, const GFX_OPT_TYPE_E GfxOpt, const MTGO_BLTOPT2_S *pBlitOpt);
#endif
/**  a collection of operations about 2D module */
mt_s32 MTGO_ADP_GFXBlit(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, const MTGO_BLTOPT2_S *pBlitOpt);

mt_s32 MTGO_ADP_GFXStretchBlit(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, const MTGO_BLTOPT2_S *pBlitOpt);

mt_s32 MTGO_ADP_GFXOperate(const MTGO_SURFACE_S *pDstSurface, const GFX_OPT_TYPE_E GfxOpt, const void *pInfo, const MTGO_BLTOPT2_S *pBlitOpt);

mt_s32 ADP_ConvertFormat(MTGO_PF_E ColorFmt, TDE2_COLOR_FMT_E *pX5ColorFmt);

mt_s32 MTGO_ADP_GFXMaskBlit(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pMaskSurface, const MT_RECT *pMaskRect, const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, const MTGO_BLTOPT2_S *pBlitOpt);
mt_s32 ADP_MEMSurfaceToTDESurface(const MTGO_SURFACE_S *pSurface, TDE2_SURFACE_S *pTDESurface);
mt_s32 ADP_MEMSurfaceToTDEMBSurface(const MTGO_SURFACE_S *pSurface, TDE2_MB_S *pTDEMBSurface);
mt_s32 ADP_GenerateTDEOpt(const MTGO_SURFACE_S *pSrcSurface,
                                 const MTGO_SURFACE_S *pDstSurface,
                                 const MTGO_BLTOPT2_S *pBlitOpt,
                                 TDE2_OPT_S *pTDEOpt, MT_BOOL bScale);
mt_s32 MTGO_ADP_GFXBlitEx(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pBckSurface, const MT_RECT *pBckRect,
	const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, const MTGO_BLTOPT2_S *pBlitOpt);
mt_s32 MTGO_ADP_GFXStretchBlitEx(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pBckSurface, const MT_RECT *pBckRect,
	const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, const MTGO_BLTOPT2_S *pBlitOpt);

mt_s32 MTGO_ADP_RotateMirror(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect,
                                                    const MTGO_ROTATE_E rotate_mod, const MTGO_MIRROR_E mirror_mod);


#ifdef __cplusplus
}
#endif

#endif /* __ADP_GFX_H__ */
