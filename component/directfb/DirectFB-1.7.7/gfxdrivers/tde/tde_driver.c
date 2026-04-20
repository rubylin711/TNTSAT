/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <config.h>

#include <directfb.h>

#include <direct/debug.h>
#include <direct/memcpy.h>
#include <direct/messages.h>

#include <core/state.h>
#include <core/surface.h>
#include <core/system.h>

#include <gfx/convert.h>

#include "mt_tde_api.h"
#include "mt_tde_errcode.h"
#include "mt_tde_type.h"

#include "tde_2d.h"
#include "tde_driver.h"
#include "tde_gfxdriver.h"

/***************************** Macro Definition ******************************/
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

D_DEBUG_DOMAIN(TDEDRV_ERROR, "TDE/ERROR", "TDE DRV Error info");
D_DEBUG_DOMAIN(TDEDRV_DEBUG, "TDE/DRV/DEBUG", "TDE DRV Debug info");

/*Get A/R/G/B component*/
/*CNcomment:根据MT_COLOR获取ARGB */
#define GetARGB(c, a, r, g, b)    \
    do {                          \
        (a) = ((c) >> 24) & 0xff; \
        (r) = ((c) >> 16) & 0xff; \
        (g) = ((c) >> 8) & 0xff;  \
        (b) = (c)&0xff;           \
    } while (0)

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/*Bulid colorfmt map table*/
/*CNcomment:建立像素格式影射表*/
struct
{
    DFBSurfacePixelFormat DfbPF;
    TDE2_COLOR_FMT_E TdePF;
} g_PF_MapTable[] = {
    {DSPF_RGB444, TDE2_COLOR_FMT_RGB444},
    {DSPF_ARGB4444, TDE2_COLOR_FMT_ARGB4444},
    {DSPF_RGB555, TDE2_COLOR_FMT_RGB555},
    {DSPF_BGR555, TDE2_COLOR_FMT_BGR555},
    {DSPF_RGB16, TDE2_COLOR_FMT_RGB565},
    {DSPF_ARGB1555, TDE2_COLOR_FMT_ARGB1555},
    {DSPF_RGB24, TDE2_COLOR_FMT_RGB24},
    {DSPF_ARGB, TDE2_COLOR_FMT_ARGB8888},
    {DSPF_RGB32, TDE2_COLOR_FMT_RGB888},
    {DSPF_RGBA4444, TDE2_COLOR_FMT_RGBA4444},
    {DSPF_AYUV, TDE2_COLOR_FMT_AYCbCr8888},
    {DSPF_ABGR, TDE2_COLOR_FMT_ABGR8888},
    {DSPF_RGBA5551, TDE2_COLOR_FMT_RGBA1555},
    {DSPF_LUT4, TDE2_COLOR_FMT_CLUT4},
    {DSPF_ALUT8, TDE2_COLOR_FMT_ACLUT88},
    {DSPF_LUT8, TDE2_COLOR_FMT_CLUT8},
    {DSPF_LUT2, TDE2_COLOR_FMT_CLUT2},
    {DSPF_ALUT44, TDE2_COLOR_FMT_ACLUT44},
    {DSPF_A8, TDE2_COLOR_FMT_A8},
    {DSPF_YUY2, TDE2_COLOR_FMT_CbY0CrY1},
};

/******************************* API declaration *****************************/

static MT_S32 ADP_DFBConvertFormat(DFBSurfacePixelFormat ColorFmt, TDE2_COLOR_FMT_E *pTDEColorFmt)
{
    MT_S32 i;

    for (i = 0; i < (MT_S32)ARRAY_SIZE(g_PF_MapTable); i++) {
        if (g_PF_MapTable[i].DfbPF == ColorFmt) {
            *pTDEColorFmt = g_PF_MapTable[i].TdePF;
            return MT_SUCCESS;
        }
    }
    D_DEBUG_AT(TDEDRV_ERROR, "%s unsupported colorformat: 0x%x\n", __FUNCTION__, ColorFmt);
    return MT_FAILURE;
}

static MT_S32 ADP_DFBConverKey(unsigned long ColorKey, TDE2_COLOR_FMT_E PixelF, TDE2_COLORKEY_U *pKeyValue)
{
    MT_U8 a, r, g, b;
    MT_U32 RMin = 0, RMax = 0, GMin = 0, GMax = 0, BMin = 0, BMax = 0;

    GetARGB(ColorKey, a, r, g, b);
    switch (PixelF) {
    case TDE2_COLOR_FMT_RGB444:
    case TDE2_COLOR_FMT_ARGB4444:
    case TDE2_COLOR_FMT_RGBA4444:
        RMin = r & 0xf0;
        RMax = r | 0x0f;
        GMin = g & 0xf0;
        GMax = g | 0x0f;
        BMin = b & 0xf0;
        BMax = b | 0x0f;
        break;
    case TDE2_COLOR_FMT_RGB555:
    case TDE2_COLOR_FMT_ARGB1555:
    case TDE2_COLOR_FMT_RGBA1555:
        RMin = r & 0xf8;
        RMax = r | 0x07;
        GMin = g & 0xf8;
        GMax = g | 0x07;
        BMin = b & 0xf8;
        BMax = b | 0x07;
        break;
    case TDE2_COLOR_FMT_RGB565:
        RMin = r & 0xf8;
        RMax = r | 0x07;
        GMin = g & 0xfc;
        GMax = g | 0x03;
        BMin = b & 0xf8;
        BMax = b | 0x07;
        break;
    default:
        RMin = RMax = r;
        GMin = GMax = g;
        BMin = BMax = b;
        break;
    }
    pKeyValue->struCkARGB.stRed.u8CompMin = (MT_U8)RMin;
    pKeyValue->struCkARGB.stRed.u8CompMax = (MT_U8)RMax;
    pKeyValue->struCkARGB.stGreen.u8CompMin = (MT_U8)GMin;
    pKeyValue->struCkARGB.stGreen.u8CompMax = (MT_U8)GMax;
    pKeyValue->struCkARGB.stBlue.u8CompMin = (MT_U8)BMin;
    pKeyValue->struCkARGB.stBlue.u8CompMax = (MT_U8)BMax;
    pKeyValue->struCkARGB.stRed.u8CompMask = 0xff;
    pKeyValue->struCkARGB.stGreen.u8CompMask = 0xff;
    pKeyValue->struCkARGB.stBlue.u8CompMask = 0xff;
    pKeyValue->struCkARGB.stAlpha.bCompIgnore = MT_TRUE;
    return MT_SUCCESS;
}

static TDE2_BLEND_MODE_E dfb2mt_blend_convert(DFBSurfaceBlendFunction dfb_bld_fact)
{
    TDE2_BLEND_MODE_E mt_bld_fact = TDE2_BLEND_ZERO;

    switch (dfb_bld_fact) {
    case DSBF_ZERO:
        mt_bld_fact = TDE2_BLEND_ZERO;
        break;
    case DSBF_ONE:
        mt_bld_fact = TDE2_BLEND_ONE;
        break;
    case DSBF_SRCCOLOR:
        mt_bld_fact = TDE2_BLEND_SRC2COLOR;
        break;
    case DSBF_INVSRCCOLOR:
        mt_bld_fact = TDE2_BLEND_INVSRC2COLOR;
        break;
    case DSBF_SRCALPHA:
        mt_bld_fact = TDE2_BLEND_SRC2ALPHA;
        break;
    case DSBF_INVSRCALPHA:
        mt_bld_fact = TDE2_BLEND_INVSRC2ALPHA;
        break;
    case DSBF_DESTALPHA:
        mt_bld_fact = TDE2_BLEND_SRC1ALPHA;
        break;
    case DSBF_INVDESTALPHA:
        mt_bld_fact = TDE2_BLEND_INVSRC1ALPHA;
        break;
    case DSBF_DESTCOLOR:
        mt_bld_fact = TDE2_BLEND_SRC1COLOR;
        break;
    case DSBF_INVDESTCOLOR:
        mt_bld_fact = TDE2_BLEND_INVSRC1COLOR;
        break;
    case DSBF_SRCALPHASAT:
        mt_bld_fact = TDE2_BLEND_SRC2ALPHASAT;
        break;
    default:
        D_DEBUG_AT(TDEDRV_ERROR, "%s unknown blend factor %d\n", __FUNCTION__, dfb_bld_fact);
        break;
    }
    return mt_bld_fact;
}

MT_S32 ADP_DFBGenerateTDEOpt(void *dev, TDE2_OPT_S *pTDEOpt, MT_BOOL bScale)
{
    TDE2_OPT_S stOpt;
    TDEDeviceData *tdev = dev;

    memset(&stOpt, 0, sizeof(TDE2_OPT_S));

    if ((tdev->blittingflags & DSBLIT_SRC_COLORKEY) || (tdev->blittingflags & DSBLIT_DST_COLORKEY)) {
        //MT_U8 clutIndex = 0;
        //MT_BOOL IsClut = MT_FALSE;

        if (tdev->blittingflags & DSBLIT_SRC_COLORKEY) {
            (MT_VOID) ADP_DFBConverKey(tdev->src_colorkey, tdev->src_surface.enColorFmt, &stOpt.unColorKeyValue);
            stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_FOREGROUND;
            stOpt.enColorKeySelect = TDE2_MASK_KEY_MATCH;
        } else {
            (MT_VOID) ADP_DFBConverKey(tdev->dst_colorkey, tdev->src_surface.enColorFmt, &stOpt.unColorKeyValue);
            stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_BACKGROUND;
            stOpt.enColorKeySelect = TDE2_MASK_KEY_MISMATCH;
        }
    } else {
        stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_NONE;
    }

    /** process alpha */
    if ((tdev->blittingflags & DSBLIT_BLEND_ALPHACHANNEL) || (tdev->blittingflags & DSBLIT_BLEND_COLORALPHA)) {
        stOpt.enAluCmd = TDE2_ALUCMD_BLEND;
        stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_NORM;

        if (tdev->blittingflags & DSBLIT_BLEND_ALPHACHANNEL) {
            stOpt.stBlendOpt.bPixelAlphaEnable = MT_TRUE;
            stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_CONFIG;

            stOpt.stBlendOpt.eSrc1BlendMode = dfb2mt_blend_convert(tdev->dst_blend);
            stOpt.stBlendOpt.eSrc2BlendMode = dfb2mt_blend_convert(tdev->src_blend);
        }

        stOpt.u8GlobalAlpha = 0xff;
        /** process global alpha opt */
        if (tdev->blittingflags & DSBLIT_BLEND_COLORALPHA) {
            stOpt.stBlendOpt.bGlobalAlphaEnable = MT_TRUE;
            stOpt.u8GlobalAlpha = (tdev->color_pixel >> 24);
            D_DEBUG_AT(TDEDRV_DEBUG, "%s u8GlobalAlpha:0x%02x, color_pixel:0x%x\n", __FUNCTION__, stOpt.u8GlobalAlpha, tdev->color_pixel);
        }

        if (tdev->blittingflags & DSBLIT_SRC_PREMULTIPLY) {
            stOpt.stBlendOpt.bSrc2AlphaPremulti = MT_TRUE;
        }

        if (tdev->blittingflags & DSBLIT_SRC_MASK_ALPHA) {
            stOpt.enAMapMixEx = MT_TRUE;
        }
        if (tdev->blittingflags & DSBLIT_SRC_MASK_COLOR) {
            stOpt.enMultiply = MT_TRUE;
            stOpt.stSurfaceCfg.disExAlpha = MT_TRUE;
        }
    }

    if (tdev->blittingflags & DSBLIT_COLORIZE) {
        if (bScale == MT_FALSE) {
            if (tdev->src_surface.enColorFmt == TDE2_COLOR_FMT_A8) {
                stOpt.enAMapMix = MT_TRUE;
            } else {
                stOpt.enMultiply = MT_TRUE;
            }
            stOpt.enPaint = MT_TRUE;
            stOpt.stPaintOpt.paint_color = tdev->color_pixel;
            stOpt.stPaintOpt.paint_type = TDE2_PAINT_TYPE_COLOR;
            D_DEBUG_AT(TDEDRV_DEBUG, "%s paint_color:0x%x\n", __FUNCTION__, stOpt.stPaintOpt.paint_color);
        } else {
            D_DEBUG_AT(TDEDRV_ERROR, "%s colorize with scale, blittingflags:0x%x\n", __FUNCTION__, tdev->blittingflags);
            return MT_FAILURE;
        }
    }

    if (tdev->drawingflags & DSBLIT_XOR) {
        stOpt.enAluCmd |= TDE2_ALUCMD_ROP;
        stOpt.enRopCode_Alpha = stOpt.enRopCode_Color = TDE2_ROP_XORPEN;
    }

    if (tdev->blittingflags & DSBLIT_ROTATE90) {
        stOpt.enRotator = TDE2_ROTATE_90;
    } else if (tdev->blittingflags & DSBLIT_ROTATE180) {
        stOpt.enRotator = TDE2_ROTATE_180;
    } else if (tdev->blittingflags & DSBLIT_ROTATE270) {
        stOpt.enRotator = TDE2_ROTATE_270;
    }

    if ((tdev->blittingflags & DSBLIT_FLIP_HORIZONTAL) && (tdev->blittingflags & DSBLIT_FLIP_VERTICAL)) {
        stOpt.enMirror = TDE2_MIRROR_BOTH;
    } else if (tdev->blittingflags & DSBLIT_FLIP_HORIZONTAL) {
        stOpt.enMirror = TDE2_MIRROR_HORIZONTAL;
    } else if (tdev->blittingflags & DSBLIT_FLIP_VERTICAL) {
        stOpt.enMirror = TDE2_MIRROR_VERTICAL;
    }

    /** deal with clip rect*/
    stOpt.enClipMode = TDE2_CLIPMODE_INSIDE;
    stOpt.stClipRect.s32Xpos = tdev->clip_rect.s32Xpos;
    stOpt.stClipRect.s32Ypos = tdev->clip_rect.s32Ypos;
    stOpt.stClipRect.u32Width = tdev->clip_rect.u32Width;
    stOpt.stClipRect.u32Height = tdev->clip_rect.u32Height;

    /** deal scale*/
    if (bScale) {
        stOpt.bResize = MT_TRUE;
    }
    memcpy(pTDEOpt, &stOpt, sizeof(stOpt));
    return MT_SUCCESS;
}

MT_S32 ADP_DFBGenerateTDEFillOpt(void *dev, TDE2_OPT_S *pTDEOpt, MT_BOOL bScale)
{
    TDE2_OPT_S stOpt;
    TDEDeviceData *tdev = dev;

    memset(&stOpt, 0, sizeof(TDE2_OPT_S));

    stOpt.enPaint = MT_TRUE;
    stOpt.stPaintOpt.paint_color = tdev->color_pixel;
    stOpt.stPaintOpt.paint_type = TDE2_PAINT_TYPE_COLOR;

    if (tdev->drawingflags & DSDRAW_DST_COLORKEY) {
        (MT_VOID) ADP_DFBConverKey(tdev->dst_colorkey, tdev->src_surface.enColorFmt, &stOpt.unColorKeyValue);
        stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_BACKGROUND;
        stOpt.enColorKeySelect = TDE2_MASK_KEY_MISMATCH;
    } else {
        stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_NONE;
    }

    /** process alpha */
    if ((tdev->drawingflags & DSDRAW_BLEND)) {
        stOpt.enAluCmd = TDE2_ALUCMD_BLEND;
        stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_NORM;

        stOpt.stBlendOpt.bPixelAlphaEnable = MT_TRUE;
        stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_CONFIG;

        stOpt.stBlendOpt.eSrc1BlendMode = dfb2mt_blend_convert(tdev->dst_blend);
        stOpt.stBlendOpt.eSrc2BlendMode = dfb2mt_blend_convert(tdev->src_blend);

        stOpt.u8GlobalAlpha = 255;
    } else if (tdev->drawingflags & DSDRAW_XOR) {
        stOpt.enAluCmd |= TDE2_ALUCMD_ROP;
        stOpt.enRopCode_Alpha = stOpt.enRopCode_Color = TDE2_ROP_XORPEN;
    } else {
        stOpt.enAluCmd |= TDE2_ALUCMD_ROP;
        stOpt.enRopCode_Alpha = stOpt.enRopCode_Color = TDE2_ROP_COPYPEN;
    }

    /** deal with clip rect*/
    memcpy(pTDEOpt, &stOpt, sizeof(stOpt));
    return MT_SUCCESS;
}

MT_S32 ADP_DFBSurfaceToTDESurface(CoreSurfaceBufferLock *pSurLock, CoreSurfaceConfig *pConfig, TDE2_SURFACE_S *pTDESurface)
{
    pTDESurface->u32PhyAddr = pSurLock->phys;
    pTDESurface->u32VirAddr  = pSurLock->addr;

    if (MT_SUCCESS != ADP_DFBConvertFormat(pSurLock->buffer->format, &(pTDESurface->enColorFmt))) {
        D_DEBUG_AT(TDEDRV_ERROR, "%s unsupported format\n", __FUNCTION__);
        return MT_FAILURE;
    }

    pTDESurface->u32Height = (MT_U32)pConfig->size.h;
    pTDESurface->u32Width = (MT_U32)pConfig->size.w;
    pTDESurface->u32Stride = pSurLock->pitch;
    /** if clut format , reload the clut, attention the pSurface->Palette address must be phyaddress*/
    pTDESurface->pu8ClutPhyAddr = NULL;
    pTDESurface->bYCbCrClut = MT_FALSE;

    /** below to be modified*/
    pTDESurface->bAlphaMax255 = MT_TRUE;
    pTDESurface->bAlphaExt1555 = MT_FALSE;
    pTDESurface->u8Alpha0 = 0;
    pTDESurface->u8Alpha1 = 255;
    return MT_SUCCESS;
}
