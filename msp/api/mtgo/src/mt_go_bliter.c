/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "string.h"
#include "mt_type.h"
#include "mt_go_bliter.h"
#include "mt_go_comm.h"
#include "mtgo_common.h"
#include "mtgo_blit.h"

#ifdef TEST_IN_ROOTBOX
#include "mtgo_rect.h"
#endif

#include "adp_gfx.h"

/***************************** Macro Definition ******************************/

#define CHECK_BLITINIT()                        \
    do {                                        \
	if (UN_INIT_STATE == s_InitBlitCount) { \
	    MTGO_ERROR(MTGO_ERR_NOTINIT);       \
	    return MTGO_ERR_NOTINIT;            \
	}                                       \
    } while (0)

#define CHECK_MIRRORTYPE(mirror)            \
    if (mirror >= MTGO_MIRROR_BUTT) {       \
	MTGO_ERROR(MTGO_ERR_INVMIRRORTYPE); \
	return MTGO_ERR_INVMIRRORTYPE;      \
    }

#define CHECK_ROTATETYPE(rotate)            \
    if (rotate >= MTGO_ROTATE_BUTT) {       \
	MTGO_ERROR(MTGO_ERR_INVROTATETYPE); \
	return MTGO_ERR_INVROTATETYPE;      \
    }

#define CHECK_PALPHATYPE(palpha)          \
    if (palpha >= MTGO_COMPOPT_BUTT) {    \
	MTGO_ERROR(MTGO_ERR_INVCOMPTYPE); \
	return MTGO_ERR_INVCOMPTYPE;      \
    }

#define CHECK_COLORKEYTYPE(ckey)          \
    if (ckey >= MTGO_CKEY_BUTT) {         \
	MTGO_ERROR(MTGO_ERR_INVCKEYTYPE); \
	return MTGO_ERR_INVCKEYTYPE;      \
    }
#define CHECK_ROPTYPE(rop)               \
    if (rop >= MTGO_ROP_BUTT) {          \
	MTGO_ERROR(MTGO_ERR_INVROPTYPE); \
	return MTGO_ERR_INVROPTYPE;      \
    }

#define CHECK_KEY(pSrcSurface, pDstSurface, BlitOpt)                              \
    do {                                                                          \
	MT_COLOR CKey;                                                            \
	mt_s32 s32Ret;                                                            \
	if (BlitOpt.ColorKeyFrom == MTGO_CKEY_SRC) {                              \
	    s32Ret = Surface_GetSurfaceColorKey((MTGO_HANDLE)pSrcSurface, &CKey); \
	    if (s32Ret != MT_SUCCESS) {                                           \
		MTGO_ERROR(MTGO_ERR_NOCOLORKEY);                                  \
		return MTGO_ERR_NOCOLORKEY;                                       \
	    }                                                                     \
	} else if (BlitOpt.ColorKeyFrom == MTGO_CKEY_DST) {                       \
	    s32Ret = Surface_GetSurfaceColorKey((MTGO_HANDLE)pDstSurface, &CKey); \
	    if (s32Ret != MT_SUCCESS) {                                           \
		MTGO_ERROR(MTGO_ERR_NOCOLORKEY);                                  \
		return MTGO_ERR_NOCOLORKEY;                                       \
	    }                                                                     \
	}                                                                         \
    } while (0)

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/
static mt_s32 s_InitBlitCount = 0;
/******************************* API declaration *****************************/
mt_s32 MTGO_InitBliter()
{
    mt_s32 s32Ret;

    /** re initial  just remember the times*/
    if (UN_INIT_STATE != s_InitBlitCount) {
	/** */
	s_InitBlitCount++;
	return MT_SUCCESS;
    }

    s32Ret = MTGO_ADP_InitBlitter();
    if (s32Ret != MT_SUCCESS) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }
    s_InitBlitCount++;
    return MT_SUCCESS;
}

mt_s32 MTGO_DeinitBliter()
{
    /** avoid no initial  */
    if (UN_INIT_STATE == s_InitBlitCount) {
	MTGO_ERROR(MTGO_ERR_NOTINIT);
	return MTGO_ERR_NOTINIT;
    }

    /** if has done initial,  exit */
    if (s_InitBlitCount != CLEAR_INIT_STATE) {
	s_InitBlitCount--;
	return MT_SUCCESS;
    }

    MTGO_ADP_DeinitBlitter();
    s_InitBlitCount--;
    return MT_SUCCESS;
}

mt_s32 MT_GO_FillRect(mt_handle Surface, const MT_RECT *pRect, MT_COLOR Color, MTGO_COMPOPT_E CompositeOpt)
{
    MTGO_HANDLE pSurface;
    MT_RECT OptRect;
    mt_s32 ret;

    /** check is the module has been initialed  */
    CHECK_BLITINIT();

    /** check input parameters */
    CHECK_PALPHATYPE(CompositeOpt);

    pSurface = Surface;
    /** handle rectangle,  */
    MTGO_GetSurfaceRealRect(pSurface, pRect, &OptRect);

    ret = Bliter_FillRect(pSurface, &OptRect, Color, CompositeOpt);
    if (ret != MT_SUCCESS) {
	MTGO_ERROR(ret);
	return ret;
    }
    return MT_SUCCESS;
}

static MT_BOOL IsVaildCombOpt(const MTGO_BLTOPT_S *pCompOpt)
{
#ifndef MTGO_CODE_CUT
    mt_u32 Opt = 0, Mirror = 0, Rotate = 0;

    if (pCompOpt->EnableGlobalAlpha || (pCompOpt->PixelAlphaComp != MTGO_COMPOPT_NONE)) {
	Opt++;
    }
    if (pCompOpt->EnableRop) {
	Opt++;
    }
    if (pCompOpt->ColorKeyFrom != MTGO_CKEY_NONE) {
	Opt++;
    }
    if (pCompOpt->EnableScale) {
	Opt++;
    }

    if (pCompOpt->MirrorType != MTGO_MIRROR_NONE) {
	Mirror++;
    }

    if (pCompOpt->RotateType != MTGO_ROTATE_NONE) {
	Rotate++;
    }

    if ((Opt >= 1) && (Mirror >= 1)) {
	return MT_TRUE;
    }
    if ((Opt >= 1) && (Rotate >= 1)) {
	return MT_TRUE;
    }
//    if ((Rotate >= 1) && (Mirror >= 1)) {
//	return MT_TRUE;
 //   }

#endif
    return MT_FALSE;
}

mt_s32 MT_GO_Blit(mt_handle SrcSurface, const MT_RECT *pSrcRect,
                  mt_handle DstSurface, const MT_RECT *pDstRect,
                  const MTGO_BLTOPT_S *pBlitOpt)
{
    MTGO_HANDLE pSrcSurface, pDstSurface;
    MTGO_BLTOPT2_S BlitOpt2;
    MT_RECT OptSrcRect, OptDstRect;
    mt_s32 ret;
    MTGO_BLTOPT_S BlitOpt;

    /** check is the module has been initialed  */
    CHECK_BLITINIT();

    if (NULL != pBlitOpt) {
	MTGO_MemCopy(&BlitOpt, pBlitOpt, sizeof(MTGO_BLTOPT_S));
    } else {
	MTGO_MemSet(&BlitOpt, 0, sizeof(MTGO_BLTOPT_S));
    }

    /** check the mix is right */
    CHECK_PALPHATYPE(BlitOpt.PixelAlphaComp);
    CHECK_COLORKEYTYPE(BlitOpt.ColorKeyFrom);
    if (BlitOpt.EnableRop) {
	CHECK_ROPTYPE(BlitOpt.Rop);
    }

    pSrcSurface = SrcSurface;
    pDstSurface = DstSurface;

    /** check the mix is right */
    if (IsVaildCombOpt(&BlitOpt)) {
    	MTGO_ERROR(MTGO_ERR_UNSUPPORTED);
    	ret = MTGO_ERR_UNSUPPORTED;
    	goto RET;
    }

    CHECK_KEY(pSrcSurface, pDstSurface, BlitOpt);

    /** handle source rectangle  */
    MTGO_GetSurfaceRealRect(pSrcSurface, pSrcRect, &OptSrcRect);

    /** handle target rectangle  */
    MTGO_GetSurfaceRealRect(pDstSurface, pDstRect, &OptDstRect);

    if((BlitOpt.MirrorType != MTGO_MIRROR_NONE) || (BlitOpt.RotateType != MTGO_ROTATE_NONE))
    {
	    //make sure the pSrcRect & pDstRect has value
        if(!pSrcRect)
            pSrcRect = &OptSrcRect;
        if(!pDstRect)
            pDstRect = &OptDstRect;
        ret = Bliter_RotateMirror (SrcSurface, pSrcRect, DstSurface, pDstRect, BlitOpt.RotateType, BlitOpt.MirrorType);
        return ret;
    }

    BlitOpt2.EnableGlobalAlpha = BlitOpt.EnableGlobalAlpha;
    BlitOpt2.PixelAlphaComp = BlitOpt.PixelAlphaComp;
    BlitOpt2.ColorKeyFrom = BlitOpt.ColorKeyFrom;
    BlitOpt2.EnableRop = BlitOpt.EnableRop;
    BlitOpt2.Rop = BlitOpt.Rop;
	BlitOpt2.RopAlpha= BlitOpt.RopAlpha;
	BlitOpt2.EnablePixelAlpha = BlitOpt.EnablePixelAlpha;
		
    if (BlitOpt.EnableScale) {
	ret = Bliter_StretchBlit(pSrcSurface, &OptSrcRect, pDstSurface, &OptDstRect, &BlitOpt2);
    } else {
	ret = Bliter_Blit(pSrcSurface, &OptSrcRect, pDstSurface, &OptDstRect, &BlitOpt2);
    }

RET:
    return ret;
    //return MT_SUCCESS;
}

mt_s32 MT_GO_DrawLine(mt_handle Surface, mt_s32 x0, mt_s32 y0, mt_s32 x1, mt_s32 y1, MT_COLOR color)
{
	mt_s32 ret = 0;
	MT_RECT rect = {0};
	if(x0 == x1)
	{
		rect.x = x0;
		rect.y = y0;
		rect.w = 1;
		if(y1 >= y0)
			rect.h = y1 - y0 + 1;
		else
			rect.h = y0 - y1 + 1;
	}
	else if(y0 == y1)
	{
		rect.x = x0;
		rect.y = y0;
		rect.h = 1;
		if(x1 >= x0)
			rect.w = x1 - x0 + 1;
		else
			rect.w = x0 - x1 + 1;
	}		
	else
		return MTGO_ERR_UNSUPPORTED;
	
	ret = MT_GO_FillRect(Surface, &rect, color, MTGO_COMPOPT_NONE);
	return ret;
}

mt_s32 MT_GO_DrawEllipse(mt_handle Surface, mt_s32 sx, mt_s32 sy, mt_s32 rx, mt_s32 ry, MT_COLOR color)
{
	return MTGO_ERR_UNSUPPORTED;
}

mt_s32 MT_GO_DrawCircle(mt_handle Surface, mt_s32 x, mt_s32 y, mt_s32 r, MT_COLOR color)
{
	return MTGO_ERR_UNSUPPORTED;
}


mt_s32 MT_GO_DrawRect(mt_handle Surface, const MT_RECT *pRect, MT_COLOR color)
{
    mt_s32 ret = 0;
    ret = MT_GO_FillRect(Surface, pRect, color, MTGO_COMPOPT_NONE);
    return ret;
}

mt_s32 MT_GO_MaskBlit(mt_handle SrcSurface, const MT_RECT *pSrcRect,
                      mt_handle DstSurface, const MT_RECT *pDstRect,
                      mt_handle MaskSurface, const MT_RECT *pMaskRect,
                      const MTGO_MASKOPT_S *pOpt)
{
    MTGO_HANDLE pSrcSurface, pMaskSurface, pDstSurface;
    MTGO_BLTOPT2_S BlitOpt2;
    MT_RECT OptSrcRect, OptMaskRect, OptDstRect;
    mt_s32 ret;
    MTGO_MASKOPT_S BlitOpt;

    /** check is the module has been initialed  */
    CHECK_BLITINIT();

    if (NULL != pOpt) {
	MTGO_MemCopy(&BlitOpt, pOpt, sizeof(MTGO_MASKOPT_S));
    } else {
	MTGO_MemSet(&BlitOpt, 0, sizeof(MTGO_MASKOPT_S));
    }

    /** check the mix is right */
    CHECK_PALPHATYPE(BlitOpt.PixelAlphaComp);
    //CHECK_COLORKEYTYPE(BlitOpt.ColorKeyFrom);

    if (BlitOpt.EnableRop) {
	CHECK_ROPTYPE(BlitOpt.RopColor);
	CHECK_ROPTYPE(BlitOpt.RopAlpha);
    }

    pSrcSurface = SrcSurface;
    pMaskSurface = MaskSurface;
    pDstSurface = DstSurface;

#if 0
    /** check the mix is right */
    if (IsVaildCombOpt(&BlitOpt))
    {
        MTGO_ERROR(MTGO_ERR_UNSUPPORTED);
        ret = MTGO_ERR_UNSUPPORTED;
        goto RET;
    }

    CHECK_KEY(pSrcSurface, pDstSurface, BlitOpt);
#endif

    /** handle source rectangle  */
    MTGO_GetSurfaceRealRect(pSrcSurface, pSrcRect, &OptSrcRect);

    /** handle mask rectangle  */
    MTGO_GetSurfaceRealRect(pMaskSurface, pMaskRect, &OptMaskRect);

    /** handle target rectangle  */
    MTGO_GetSurfaceRealRect(pDstSurface, pDstRect, &OptDstRect);

    BlitOpt2.EnableGlobalAlpha = BlitOpt.EnableGlobalAlpha;
    BlitOpt2.PixelAlphaComp = BlitOpt.PixelAlphaComp;
    BlitOpt2.ColorKeyFrom = MTGO_CKEY_NONE;
    BlitOpt2.EnableRop = BlitOpt.EnableRop;
    BlitOpt2.Rop = BlitOpt.RopColor;
    BlitOpt2.RopAlpha = BlitOpt.RopAlpha;

#if 0
    {
	MTGO_SURFACE_S *pSurf = NULL;

	printf("MT_GO_MaskBlit:\n");
	pSurf = (MTGO_SURFACE_S *)pSrcSurface;
	printf("src sur: [%d][%d]\n", pSurf->Width, pSurf->Height);

	pSurf = (MTGO_SURFACE_S *)pMaskSurface;
	printf("msk sur: [%d][%d]\n", pSurf->Width, pSurf->Height);

	pSurf = (MTGO_SURFACE_S *)pDstSurface;
	printf("dst sur: [%d][%d]\n", pSurf->Width, pSurf->Height);
    }
    printf("src rect:[%d][%d][%d][%d] [%d][%d][%d][%d]\n", pSrcRect->x, pSrcRect->y, pSrcRect->w, pSrcRect->h, OptSrcRect.x, OptSrcRect.y, OptSrcRect.w, OptSrcRect.h);
    printf("msk rect:[%d][%d][%d][%d] [%d][%d][%d][%d]\n", pMaskRect->x, pMaskRect->y, pMaskRect->w, pMaskRect->h, OptMaskRect.x, OptMaskRect.y, OptMaskRect.w, OptMaskRect.h);
    printf("dst rect:[%d][%d][%d][%d] [%d][%d][%d][%d]\n", pDstRect->x, pDstRect->y, pDstRect->w, pDstRect->h, OptDstRect.x, OptDstRect.y, OptDstRect.w, OptDstRect.h);
#endif

    ret = Bliter_MaskBlit(pSrcSurface, &OptSrcRect, pMaskSurface, &OptMaskRect, pDstSurface, &OptDstRect, &BlitOpt2);

    //RET:

    return ret;
    //return MT_SUCCESS;
}

mt_s32 MT_GO_BitBlit (mt_handle SrcSurface, const MT_RECT* pSrcRect,
                       mt_handle DstSurface, const MT_RECT* pDstRect,
                       const MTGO_BLTOPT2_S* pBlitOpt)
{
	mt_s32 ret;
	MTGO_BLTOPT_S opt = {0};
	opt.EnableGlobalAlpha = pBlitOpt->EnableGlobalAlpha;
	opt.EnablePixelAlpha = pBlitOpt->EnablePixelAlpha;
	opt.PixelAlphaComp = pBlitOpt->PixelAlphaComp;
	opt.ColorKeyFrom = pBlitOpt->ColorKeyFrom;
	opt.EnableRop = pBlitOpt->EnableRop;
	opt.Rop = pBlitOpt->Rop;
	opt.RopAlpha = pBlitOpt->RopAlpha;
	opt.EnableScale = MT_FALSE;
	opt.RotateType = MTGO_ROTATE_NONE;
	opt.MirrorType = MTGO_MIRROR_NONE;
	ret = MT_GO_Blit(SrcSurface, pSrcRect, DstSurface, pDstRect, &opt);
	return ret;
}

mt_s32 MT_GO_StretchBlit (mt_handle SrcSurface, const MT_RECT* pSrcRect,
                       mt_handle DstSurface, const MT_RECT* pDstRect,
                       const MTGO_BLTOPT2_S* pBlitOpt)
{
	mt_s32 ret;
	MTGO_BLTOPT_S opt = {0};
	opt.EnableGlobalAlpha = pBlitOpt->EnableGlobalAlpha;
	opt.EnablePixelAlpha = pBlitOpt->EnablePixelAlpha;
	opt.PixelAlphaComp = pBlitOpt->PixelAlphaComp;
	opt.ColorKeyFrom = pBlitOpt->ColorKeyFrom;
	opt.EnableRop = pBlitOpt->EnableRop;
	opt.Rop = pBlitOpt->Rop;
	opt.RopAlpha = pBlitOpt->RopAlpha;
	opt.EnableScale = MT_TRUE;
	opt.RotateType = MTGO_ROTATE_NONE;
	opt.MirrorType = MTGO_MIRROR_NONE;
	ret = MT_GO_Blit(SrcSurface, pSrcRect, DstSurface, pDstRect, &opt);
	return ret;

}

mt_s32 MT_GO_PatternBlit(mt_handle SrcSurface, const MT_RECT* pSrcRect, 
                              mt_handle DstSurface, const MT_RECT * pDstRect, 
                              const MTGO_BLTOPT2_S* pParOpt)
{
	return MTGO_ERR_UNSUPPORTED;
}

mt_s32 MT_GO_DeflickerBlit(mt_handle SrcSurface, const MT_RECT* pSrcRect, 
                                 mt_handle DstSurface, const MT_RECT * pDstRect, 
                                 const MTGO_DEFLICKEROPT_S* pDefOpt)
{
	return MTGO_ERR_UNSUPPORTED;
}

mt_s32 MT_GO_Blit3Source(mt_handle BckSurface, const MT_RECT* pBckRect,
                   mt_handle ForSurface, const MT_RECT* pForRect,
                   mt_handle DstSurface, const MT_RECT* pDstRect,
                   const MTGO_BLTOPT_S* pBlitOpt)
{
    MTGO_HANDLE pSrcSurface, pDstSurface, pBckSurface;
    MTGO_BLTOPT2_S BlitOpt2;
    MT_RECT OptSrcRect, OptDstRect, OptBckRect;
    mt_s32 ret;
    MTGO_BLTOPT_S BlitOpt;

    /** check is the module has been initialed  */
    CHECK_BLITINIT();

    if (NULL != pBlitOpt) {
	MTGO_MemCopy(&BlitOpt, pBlitOpt, sizeof(MTGO_BLTOPT_S));
    } else {
	MTGO_MemSet(&BlitOpt, 0, sizeof(MTGO_BLTOPT_S));
    }

    /** check the mix is right */
    CHECK_PALPHATYPE(BlitOpt.PixelAlphaComp);
    CHECK_COLORKEYTYPE(BlitOpt.ColorKeyFrom);
    if (BlitOpt.EnableRop) {
	CHECK_ROPTYPE(BlitOpt.Rop);
    }

    pSrcSurface = ForSurface;
    pDstSurface = DstSurface;
	pBckSurface = BckSurface;

    /** check the mix is right */
    if (IsVaildCombOpt(&BlitOpt)) {
	MTGO_ERROR(MTGO_ERR_UNSUPPORTED);
	ret = MTGO_ERR_UNSUPPORTED;
	goto RET;
    }

    CHECK_KEY(pSrcSurface, pDstSurface, BlitOpt);

    /** handle source rectangle  */
    MTGO_GetSurfaceRealRect(pSrcSurface, pForRect, &OptSrcRect);

    /** handle target rectangle  */
    MTGO_GetSurfaceRealRect(pDstSurface, pDstRect, &OptDstRect);

    /** handle back rectangle  */
    MTGO_GetSurfaceRealRect(pBckSurface, pBckRect, &OptBckRect);

    BlitOpt2.EnableGlobalAlpha = BlitOpt.EnableGlobalAlpha;
	BlitOpt2.EnablePixelAlpha = BlitOpt.EnablePixelAlpha;
    BlitOpt2.PixelAlphaComp = BlitOpt.PixelAlphaComp;
    BlitOpt2.ColorKeyFrom = BlitOpt.ColorKeyFrom;
    BlitOpt2.EnableRop = BlitOpt.EnableRop;
    BlitOpt2.Rop = BlitOpt.Rop;
	BlitOpt2.RopAlpha= BlitOpt.RopAlpha;

    if (BlitOpt.EnableScale) {
	ret = Bliter_StretchBlitEx(pSrcSurface, &OptSrcRect, pBckSurface, &OptBckRect, pDstSurface, &OptDstRect, &BlitOpt2);
    } else {
	ret = Bliter_BlitEx(pSrcSurface, &OptSrcRect, pBckSurface, &OptBckRect, pDstSurface, &OptDstRect, &BlitOpt2);
    }

RET:
    return ret;	
}

mt_s32 MT_GO_FillRoundRect(mt_handle Surface, const MT_RECT* pRect, MT_COLOR Color, mt_s32 s32Radius)
{
	return MTGO_ERR_UNSUPPORTED;
}

mt_s32 MT_GO_MEMSurfaceToTDESurface(mt_handle Surface, mt_void *pTDESurface)
{
  return Bliter_MEMSurfaceToTDESurface((MTGO_HANDLE)Surface, pTDESurface);
}
mt_s32 MT_GO_MEMSurfaceToTDEMBSurface(mt_handle Surface, mt_void *pTDEMBSurface)
{
	return Bliter_MEMSurfaceToTDEMBSurface((MTGO_HANDLE)Surface, pTDEMBSurface);
}

mt_s32 MT_GO_GenerateTDEOpt(mt_handle SrcSurface,
                                 mt_handle DstSurface,
                                 MTGO_BLTOPT2_S *pBlitOpt,
                                 mt_void *pTDEOpt, MT_BOOL bScale)
{
    MTGO_BLTOPT2_S BlitOpt2;

    if (NULL != pBlitOpt) {
	MTGO_MemCopy(&BlitOpt2, pBlitOpt, sizeof(MTGO_BLTOPT2_S));
    } else {
	MTGO_MemSet(&BlitOpt2, 0, sizeof(MTGO_BLTOPT2_S));
    }

	return Bliter_GenerateTDEOpt((MTGO_HANDLE)SrcSurface, (MTGO_HANDLE)DstSurface, &BlitOpt2, pTDEOpt, bScale);
}
mt_s32 MT_GO_ConvertTDEFmt(mt_u32 mtgoFmt, mt_u32 *tdeFmt)
{
	return Bliter_ConvertFormat(mtgoFmt, tdeFmt);
}

