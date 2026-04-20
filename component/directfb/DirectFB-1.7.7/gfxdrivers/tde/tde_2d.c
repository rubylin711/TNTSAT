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

#include <core/palette.h>
#include <core/state.h>
#include <core/surface.h>
#include <core/system.h>

#include <gfx/convert.h>

#include "mt_tde_api.h"
#include "mt_tde_errcode.h"
#include "mt_tde_type.h"

#include "SoftOperate.h"
#include "tde_2d.h"
#include "tde_driver.h"
#include "tde_gfxdriver.h"

/***************************** Macro Definition ******************************/

D_DEBUG_DOMAIN(TDE_ERROR, "TDE/ERROR", "TDE 2D Error info");
D_DEBUG_DOMAIN(TDE_DEBUG, "TDE/DEBUG", "TDE 2D Debug info");
D_DEBUG_DOMAIN(TDE_CMD_FIFO, "TDE/CMD_FIFO", "TDE 2D command fifo");
D_DEBUG_DOMAIN(TDE_ACC, "TDE/ACC", "TDE 2D Acceleration");
D_DEBUG_DOMAIN(TDE_DUMP, "TDE/DUMP", "TDE dump data");

D_DEBUG_DOMAIN(TDE_UNSUPPORTED, "TDE/UNSUPPORTED", "TDE 2D Not supported");
D_DEBUG_DOMAIN(TDE_VALIDATE, "TDE/VALIDATE", "TDE validate");
D_DEBUG_DOMAIN(TDE_FUNC, "TDE/FUNC", "TDE Function tracing");
#define FUNCTION_IN() D_DEBUG_AT(TDE_FUNC, "%s %d In\n", __FUNCTION__, __LINE__)
#define FUNCTION_OUT() D_DEBUG_AT(TDE_FUNC, "%s %d Out\n", __FUNCTION__, __LINE__)

/*
 * State validation flags.
 *
 * There's no prefix because of the macros below.
 */

enum {
    BLITINGFLAGS = 0x1 << 0,       /* use: blit flags */
    DRAWINGFLAGS = 0x1 << 1,       /* use: draw flags */
    CLIP = 0x1 << 2,               /* use: blit and draw clip rect*/
    COLOR_BLIT = 0x1 << 3,         /* use: blit color and modulate color*/
    COLOR_DRAW = 0x1 << 4,         /* use: draw color and modulate color*/
    SRC_BLEND = 0x1 << 5,          /* use: src blend function , only for port-duff srcover*/
    DST_BLEND = 0x1 << 6,          /* use: dst blend function , only for port-duff srcover*/
    SRC_COLORKEY = 0x1 << 7,       /* use: for src colorkey value */
    DST_COLORKEY = 0x1 << 8,       /* use: for dst colorkey value */
    DESTINATION = 0x1 << 9,        /* use: for dst surface*/
    SOURCE = 0x1 << 10,            /* use: for src surface*/
    SOURCE_MASK = 0x1 << 11,       /* use: for src surface mask*/
    SOURCE_MASKVALUE = 0x1 << 12,  /* no use: for src mask value */
    INDEX_TRANSLATION = 0x1 << 13, /* no use: clut format key value */
    COLORLEY = 0x1 << 14,          /* no use: protect key*/
    RENDER_OPTS = 0x1 << 15,       /* no use: render operation*/
    MATRIX = 0x1 << 16,            /* no use: matix, only for render operate*/
    SOURCE2 = 0x1 << 17,           /*use, add by  jqw*/
    ALL = 0xffffffff               /* all invalid*/
};
/*
 * State handling macros.
 */

#define TDE_VALIDATE(flags)       \
    do {                          \
        tdev->v_flags |= (flags); \
    } while (0)
#define TDE_INVALIDATE(flags)      \
    do {                           \
        tdev->v_flags &= ~(flags); \
    } while (0)

#define TDE_CHECK_VALIDATE(flag)              \
    do {                                      \
        if ((tdev->v_flags & flag) != flag)   \
            tde_validate_##flag(tdev, state); \
    } while (0)

/*Use 2d acceleration when resolution is larger than MAX_HD_PIXEL*/
/*CNComment:只有分辨率大于一定的尺寸才能使用硬件操作.*/
//#define MAX_HD_PIXEL 14400
/*#define IS_NEED_HDOPT(width, height) \
        ((width * height) > MAX_HD_PIXEL)    */
#define IS_NEED_HDOPT(width, height) MT_TRUE
#define TIMEOUT (10)

#define TDE_FLAG (MT_FALSE)

// refer to #define MAX_NODE_NUM 20 in gpe.h
#define MAX_CMD_FIFO_COUNT (256)

#define TDE_IS_CLUT_FORMAT(p) ((p) >= TDE2_COLOR_FMT_CLUT1 && (p) <= TDE2_COLOR_FMT_ACLUT88)

/** now clut format is not supported*/
#define CHECK_TDE_DST_FORMAT(format)                                               \
    do {                                                                           \
        switch (format) {                                                          \
        case DSPF_ARGB1555:                                                        \
        case DSPF_RGB16:                                                           \
        case DSPF_RGB32:                                                           \
        case DSPF_ARGB4444:                                                        \
        case DSPF_RGBA4444:                                                        \
        case DSPF_AYUV:                                                            \
        case DSPF_ARGB:                                                            \
        case DSPF_RGB555:                                                          \
        case DSPF_RGB444:                                                          \
        case DSPF_ABGR:                                                            \
        case DSPF_RGBA5551:                                                        \
        case DSPF_LUT8:                                                            \
        case DSPF_ALUT8:                                                           \
        case DSPF_ALUT44:                                                          \
            break;                                                                 \
        default:                                                                   \
            D_DEBUG_AT(TDE_UNSUPPORTED, "%s %d -> unsupported source format %s\n", \
                       __FUNCTION__, __LINE__, dfb_pixelformat_name(format));      \
            return;                                                                \
        }                                                                          \
    } while (0);

/** now clut format is not supported*/
#define CHECK_TDE_SRC_FORMAT(format)                                               \
    do {                                                                           \
        switch (format) {                                                          \
        case DSPF_ARGB1555:                                                        \
        case DSPF_RGB16:                                                           \
        case DSPF_RGB32:                                                           \
        case DSPF_RGB24:                                                           \
        case DSPF_A8:                                                              \
        case DSPF_YUY2:                                                            \
        case DSPF_ALUT44:                                                          \
        case DSPF_LUT8:                                                            \
        case DSPF_LUT2:                                                            \
        case DSPF_ARGB4444:                                                        \
        case DSPF_RGBA4444:                                                        \
        case DSPF_AYUV:                                                            \
        case DSPF_ARGB:                                                            \
        case DSPF_RGB555:                                                          \
        case DSPF_RGB444:                                                          \
        case DSPF_ABGR:                                                            \
        case DSPF_RGBA5551:                                                        \
        case DSPF_LUT4:                                                            \
        case DSPF_ALUT8:                                                           \
            break;                                                                 \
        default:                                                                   \
            D_DEBUG_AT(TDE_UNSUPPORTED, "%s %d -> unsupported source format %s\n", \
                       __FUNCTION__, __LINE__, dfb_pixelformat_name(format));      \
            return;                                                                \
        }                                                                          \
    } while (0);

#define IS_HAVE_CLIPRECT(pDev, pRect)                      \
    (pDev->clip_rect.s32Xpos != (pRect)->s32Xpos) ||       \
        (pDev->clip_rect.s32Ypos != (pRect)->s32Ypos) ||   \
        (pDev->clip_rect.u32Width != (pRect)->u32Width) || \
        (pDev->clip_rect.u32Height != (pRect)->u32Height)

static inline void trect2dregion(DFBRegion *pRegion, TDE2_RECT_S *pRect)
{
    pRegion->x1 = pRect->s32Xpos;
    pRegion->y1 = pRect->s32Ypos;
    pRegion->x2 = pRect->s32Xpos + pRect->u32Width - 1;
    pRegion->y2 = pRect->s32Ypos + pRect->u32Height - 1;
}

static inline void dregion2trect(TDE2_RECT_S *pRect, DFBRegion *pRegion)
{
    pRect->s32Xpos = pRegion->x1;
    pRect->s32Ypos = pRegion->y1;
    pRect->u32Width = pRegion->x2 - pRegion->x1 + 1;
    pRect->u32Height = pRegion->y2 - pRegion->y1 + 1;
}

static inline void drect2trect(TDE2_RECT_S *pTDERect, DFBRectangle *pDFBRect)
{
    pTDERect->s32Xpos = pDFBRect->x;
    pTDERect->s32Ypos = pDFBRect->y;
    pTDERect->u32Width = pDFBRect->w;
    pTDERect->u32Height = pDFBRect->h;
}

static inline void trect2drect(DFBRectangle *pDFBRect, TDE2_RECT_S *pTDERect)
{
    pDFBRect->x = pTDERect->s32Xpos;
    pDFBRect->y = pTDERect->s32Ypos;
    pDFBRect->w = pTDERect->u32Width;
    pDFBRect->h = pTDERect->u32Height;
}

static inline void init_trect_val(TDE2_RECT_S *pTDERect, int x, int y, int w, int h)
{
    pTDERect->s32Xpos = x;
    pTDERect->s32Ypos = y;
    pTDERect->u32Width = w;
    pTDERect->u32Height = h;
}

static inline void trect_resize(TDE2_RECT_S *rect, mt_u32 width, mt_u32 height)
{
    rect->u32Width = width;
    rect->u32Height = height;
}

static inline bool trect_intersect(TDE2_RECT_S *rectangle,
                                   const TDE2_RECT_S *clip_rect)
{
    DFBRegion region;
    DFBRegion clipRegion;
    bool ret;
    trect2dregion(&region, rectangle);
    trect2dregion(&clipRegion, clip_rect);

    ret = dfb_region_region_intersect(&region, &clipRegion);

    dregion2trect(rectangle, &region);
    return ret;
}

static inline MT_BOOL calc_visible_rect(TDE2_RECT_S *src_rect, TDE2_RECT_S *dst_rect, TDE2_RECT_S *clip_rect)
{
    DFBRegion dstRegion;
    DFBRegion clipRegion;
    int adjustx;
    int adjusty;

    if (src_rect->u32Width == 0 || src_rect->u32Height == 0 || dst_rect->u32Width == 0 || dst_rect->u32Height == 0) {
        return MT_FALSE;
    }

    trect2dregion(&dstRegion, dst_rect);
    trect2dregion(&clipRegion, clip_rect);

    if (dstRegion.x1 < clipRegion.x1) {
        adjustx = clipRegion.x1 - dstRegion.x1;
        dst_rect->s32Xpos += adjustx;
        dst_rect->u32Width -= adjustx;
        src_rect->s32Xpos += adjustx;
        src_rect->u32Width -= adjustx;
    }
    if (dstRegion.x2 > clipRegion.x2) {
        adjustx = dstRegion.x2 - clipRegion.x2;
        dst_rect->u32Width -= adjustx;
        src_rect->u32Width -= adjustx;
    }
    if (dstRegion.y1 < clipRegion.y1) {
        adjusty = clipRegion.y1 - dstRegion.y1;
        dst_rect->s32Ypos += adjusty;
        dst_rect->u32Height -= adjusty;
        src_rect->s32Ypos += adjusty;
        src_rect->u32Height -= adjusty;
    }
    if (dstRegion.y2 > clipRegion.y2) {
        adjusty = dstRegion.y2 - clipRegion.y2;
        dst_rect->u32Height -= adjusty;
        src_rect->u32Height -= adjusty;
    }
    if ((mt_s32)dst_rect->s32Ypos < 0 || (mt_s32)src_rect->s32Ypos < 0 ||
        (mt_s32)dst_rect->s32Xpos < 0 || (mt_s32)src_rect->s32Xpos < 0 ||
        (mt_s32)dst_rect->u32Width <= 0 || (mt_s32)src_rect->u32Width <= 0 ||
        (mt_s32)dst_rect->u32Height <= 0 || (mt_s32)src_rect->u32Height <= 0) {
        return MT_FALSE;
    }

    return MT_TRUE;
}

static inline MT_BOOL calc_visible_rect_stretch(TDE2_RECT_S *src_rect, TDE2_RECT_S *dst_rect, TDE2_RECT_S *clip_rect)
{
    DFBRegion dstRegion;
    DFBRegion clipRegion;
    int adjustx;
    int adjusty;

    if (src_rect->u32Width == 0 || src_rect->u32Height == 0 || dst_rect->u32Width == 0 || dst_rect->u32Height == 0) {
        return MT_FALSE;
    }

    double ratew = (double)src_rect->u32Width / dst_rect->u32Width;
    double rateh = (double)src_rect->u32Height / dst_rect->u32Height;

    trect2dregion(&dstRegion, dst_rect);
    trect2dregion(&clipRegion, clip_rect);

    if (dstRegion.x1 < clipRegion.x1) {
        adjustx = clipRegion.x1 - dstRegion.x1;
        dst_rect->s32Xpos += adjustx;
        dst_rect->u32Width -= adjustx;
        src_rect->s32Xpos += adjustx * ratew;
        src_rect->u32Width -= adjustx * ratew;
    }
    if (dstRegion.x2 > clipRegion.x2) {
        adjustx = dstRegion.x2 - clipRegion.x2;
        dst_rect->u32Width -= adjustx;
        src_rect->u32Width -= adjustx * ratew;
    }
    if (dstRegion.y1 < clipRegion.y1) {
        adjusty = clipRegion.y1 - dstRegion.y1;
        dst_rect->s32Ypos += adjusty;
        dst_rect->u32Height -= adjusty;
        src_rect->s32Ypos += adjusty * rateh;
        src_rect->u32Height -= adjusty * rateh;
    }
    if (dstRegion.y2 > clipRegion.y2) {
        adjusty = dstRegion.y2 - clipRegion.y2;
        dst_rect->u32Height -= adjusty;
        src_rect->u32Height -= adjusty * rateh;
    }
    if ((mt_s32)dst_rect->s32Ypos < 0 || (mt_s32)src_rect->s32Ypos < 0 ||
        (mt_s32)dst_rect->s32Xpos < 0 || (mt_s32)src_rect->s32Xpos < 0 ||
        (mt_s32)dst_rect->u32Width <= 0 || (mt_s32)src_rect->u32Width <= 0 ||
        (mt_s32)dst_rect->u32Height <= 0 || (mt_s32)src_rect->u32Height <= 0) {
        return MT_FALSE;
    }

    return MT_TRUE;
}

static inline void dump_dfb_rect(DFBRectangle *rect, char *name)
{
    D_DEBUG_AT(TDE_DUMP, "DFBRectangle:%s, [%d,%d,%d,%d]\n", name ? name : "unnamed", rect->x, rect->y, rect->w, rect->h);
}

static inline void dump_tde_rect(TDE2_RECT_S *rect, char *name)
{
    D_DEBUG_AT(TDE_DUMP, "TDE2_RECT_S:%s, [%d,%d,%d,%d]\n", name ? name : "unnamed", rect->s32Xpos, rect->s32Ypos, rect->u32Width, rect->u32Height);
}

static inline int tde_check_rect(TDE2_RECT_S *pRect, TDE2_SURFACE_S *pSurface)
{
    DFBRegion dstRegion, srcRegion;
    TDE2_RECT_S srcRect = {0, 0, pSurface->u32Width, pSurface->u32Height};
    trect2dregion(&dstRegion, pRect);
    trect2dregion(&srcRegion, &srcRect);
    if (!dfb_region_region_intersect(&dstRegion, &srcRegion)) {
        return -1;
    }
    dregion2trect(pRect, &dstRegion);
    return 0;
}

static inline void tde_ajust_size_with_small_one(TDE2_RECT_S *pRect1, TDE2_RECT_S *pRect2)
{
    if (pRect1->u32Width < pRect2->u32Width) {
        pRect2->u32Width = pRect1->u32Width;
    } else if (pRect1->u32Width > pRect2->u32Width) {
        pRect1->u32Width = pRect2->u32Width;
    }

    if (pRect1->u32Height < pRect2->u32Height) {
        pRect2->u32Height = pRect1->u32Height;
    } else if (pRect1->u32Height > pRect2->u32Height) {
        pRect1->u32Height = pRect2->u32Height;
    }
}

/*
 * Called by SetState() to ensure that the destination registers are properly set
 * for execution of rendering functions.
 */

static inline void
tde_validate_SRC_BLEND(TDEDeviceData *tdev,
                       CardState *state)
{
    /* Remember destination parameters for usage in rendering functions. */
    tdev->src_blend = state->src_blend;
    D_DEBUG_AT(TDE_VALIDATE, "%s src_blend:0x%x\n", __FUNCTION__, state->src_blend);
    /* Set the flag. */
    TDE_VALIDATE(SRC_BLEND);
}

/*
 * Called by SetState() to ensure that the destination registers are properly set
 * for execution of rendering functions.
 */
static inline void
tde_validate_DST_BLEND(TDEDeviceData *tdev,
                       CardState *state)
{
    /* Remember destination parameters for usage in rendering functions. */
    tdev->dst_blend = state->dst_blend;
    D_DEBUG_AT(TDE_VALIDATE, "%s dst_blend:0x%x\n", __FUNCTION__, state->dst_blend);

    /* Set the flag. */
    TDE_VALIDATE(DST_BLEND);
}

#define GPE_DEFAULT_ALPHA 0xFF

static u32 rgb565_expend(u16 pk)
{
    mt_u8 r5 = (pk & 0xF800) >> 8; // xxxx x000 0000 0000
    mt_u8 g6 = (pk & 0x07E0) >> 3; // 0000 0xxx xxx0 0000
    mt_u8 b5 = (pk & 0x001F) << 3; // 0000 0000 000x xxxx

    mt_u32 rgb888 = 0;

    rgb888 = (GPE_DEFAULT_ALPHA << 24) | (r5 << 16) | (g6 << 8) | b5;

    return rgb888;
}

static u32 argb1555_expend(u16 pk)
{
    mt_u8 a1 = (pk & 0x8000) >> 15;
    mt_u8 r5 = ((pk & 0x7C00) << 1) >> 8; // 0xxx xx00 0000 0000
    mt_u8 g5 = (pk & 0x03E0) >> 2;        // 0000 00xx xxx 0000
    mt_u8 b5 = (pk & 0x001F) << 3;        // 0000 0000 000x xxxx
    mt_u8 alpha = 0;

    mt_u32 rgb888 = 0;

    if (a1) {
        alpha = GPE_DEFAULT_ALPHA;
    } else {
        alpha = 0x0;
    }

    rgb888 = (alpha << 24) | (r5 << 16) | (g5 << 8) | b5;

    return rgb888;
}

static u32 rgba5551_expend(u16 pk)
{
    mt_u8 r5 = (pk & 0xF800) >> 8;        // 0xxx xx00 0000 0000
    mt_u8 g5 = ((pk & 0x07C0) << 1) >> 4; // 0000 00xx xxx 0000
    mt_u8 b5 = (pk & 0x003E) << 2;        // 0000 0000 000x xxxx

    mt_u8 a1 = (pk & 0x1);
    mt_u8 alpha = 0;

    mt_u32 rgb888 = 0;

    if (a1) {
        alpha = GPE_DEFAULT_ALPHA;
    } else {
        alpha = 0x00;
    }

    rgb888 = (alpha << 24) | (r5 << 16) | (g5 << 8) | b5;

    return rgb888;
}

static u32 argb4444_expend(u16 ck)
{
    mt_u8 a4 = (ck & 0xF000) >> 8;
    mt_u8 r4 = (ck & 0x0F00) >> 4;
    mt_u8 g4 = (ck & 0x00F0);
    mt_u8 b4 = (ck & 0x000F) << 4;

    mt_u32 rgb888 = 0;

    a4 = (a4 << 4) | a4;
    r4 = (a4 << 4) | r4;
    g4 = (a4 << 4) | g4;
    b4 = (a4 << 4) | b4;

    rgb888 = (a4 << 24) | (r4 << 16) | (g4 << 8) | b4;

    return rgb888;
}

static u32 rgba4444_expend(u16 ck)
{
    mt_u8 r4 = (ck & 0xF000) >> 8;
    mt_u8 g4 = (ck & 0x0F00) >> 4;
    mt_u8 b4 = (ck & 0x00F0);
    mt_u8 a4 = (ck & 0x000F) << 4;

    mt_u32 rgb888 = 0;

    a4 = (a4 << 4) | a4;
    r4 = (a4 << 4) | r4;
    g4 = (a4 << 4) | g4;
    b4 = (a4 << 4) | b4;

    rgb888 = (a4 << 24) | (r4 << 16) | (g4 << 8) | b4;

    return rgb888;
}

static mt_u32 uyvy2ayuv(mt_u32 src)
{
    mt_u32 dst = 0;
    mt_u8 y0 = 0, y1 = 0, u = 0, v = 0;

    u = (src >> 24) & 0xff;
    y0 = (src >> 16) & 0xff;
    v = (src >> 8) & 0xff;
    y1 = src & 0xff;
    dst = (0xff << 24) | (((y0 + y1 + 1) >> 1) << 16) | (u << 8) | v;
    return dst;
}

//color->ARGB8888
static unsigned long convert2ARGBcolor(DFBSurfacePixelFormat fmt, unsigned long src_color)
{
    mt_u32 tmppk = 0;
    mt_u8 a = 0;
    mt_u8 idx = 0;

    //填色全部转化成argb，colorkey对rgb需转成argb，对yuv需转成ayuv
    switch (fmt) {
    case DSPF_RGB16:
        tmppk = rgb565_expend(src_color);
        break;
    case DSPF_ARGB1555:
        tmppk = argb1555_expend(src_color);
        break;
    case DSPF_ARGB4444:
        tmppk = argb4444_expend(src_color);
        break;
    case DSPF_RGBA5551:
        tmppk = rgba5551_expend(src_color);
        break;
    case DSPF_RGBA4444:
        tmppk = rgba4444_expend(src_color);
        break;
    case DSPF_YUY2:
        tmppk = uyvy2ayuv(src_color);
        break;
    case DSPF_ARGB:
        tmppk = src_color;
        break;
    case DSPF_AYUV:
        tmppk = src_color;
        break;
    case DSPF_RGB32:
    case DSPF_RGB24:
        tmppk = src_color | 0xff000000;
        break;
    case DSPF_RGB555:
        src_color = 0x8000 | src_color;
        tmppk = argb1555_expend(src_color);
        break;
    case DSPF_RGB444:
        src_color = 0xf000 | src_color;
        tmppk = argb4444_expend(src_color);
        break;
    case DSPF_ALUT44:
        a = (src_color >> 4) & 0xf;
        a = a << 4;
        idx = src_color & 0xf;
        tmppk = (a << 24) | idx;
        break;
    case DSPF_LUT8:
        tmppk = src_color;
        break;
    case DSPF_ALUT8:
        a = (src_color >> 8) & 0xff;
        idx = src_color & 0xff;
        tmppk = (a << 24) | idx;
        break;
    default:
        tmppk = src_color;
        break;
    }

    return tmppk;
}

/*
 * Called by SetState() to ensure that the destination registers are properly set
 * for execution of rendering functions.
 */
static inline void
tde_validate_DST_COLORKEY(TDEDeviceData *tdev,
                          CardState *state)
{
    /* Remember destination parameters for usage in rendering functions. */
    tdev->dst_colorkey = convert2ARGBcolor(state->destination->config.format, state->dst_colorkey);

    /* Set the flag. */
    TDE_VALIDATE(DST_COLORKEY);
}

/*
 * Called by SetState() to ensure that the destination registers are properly set
 * for execution of rendering functions.
 */
static inline void
tde_validate_SRC_COLORKEY(TDEDeviceData *tdev,
                          CardState *state)
{
    /* Remember destination parameters for usage in rendering functions. */
    tdev->src_colorkey = convert2ARGBcolor(state->source->config.format, state->src_colorkey);

    /* Set the flag. */
    TDE_VALIDATE(SRC_COLORKEY);
}
/*
 * Called by SetState() to ensure that the destination registers are properly set
 * for execution of rendering functions.
 */
static inline void
tde_validate_DRAWINGFLAGS(TDEDeviceData *tdev,
                          CardState *state)
{
    /* Remember destination parameters for usage in rendering functions. */
    tdev->drawingflags = state->drawingflags;
    /* Set the flag. */
    TDE_VALIDATE(DRAWINGFLAGS);
}

/*
 * Called by SetState() to ensure that the destination registers are properly set
 * for execution of rendering functions.
 */
static inline void
tde_validate_BLITINGFLAGS(TDEDeviceData *tdev,
                          CardState *state)
{
    /* Remember destination parameters for usage in rendering functions. */
    tdev->blittingflags = state->blittingflags;
    /* Set the flag. */
    TDE_VALIDATE(BLITINGFLAGS);
}
/*
 * Called by SetState() to ensure that the destination registers are properly set
 * for execution of rendering functions.
 */
static inline void
tde_validate_CLIP(TDEDeviceData *tdev,
                  CardState *state)
{
    /* Remember destination parameters for usage in rendering functions. */
    tdev->clip_rect.s32Xpos = state->clip.x1;
    tdev->clip_rect.s32Ypos = state->clip.y1;
    tdev->clip_rect.u32Width = state->clip.x2 - state->clip.x1 + 1;
    tdev->clip_rect.u32Height = state->clip.y2 - state->clip.y1 + 1;
    /* Set the flag. */
    TDE_VALIDATE(CLIP);
}

/*
 * Called by SetState() to ensure that the destination registers are properly set
 * for execution of rendering functions.
 */
static inline void
tde_validate_DESTINATION(TDEDeviceData *tdev,
                         CardState *state)
{
    /* Remember destination parameters for usage in rendering functions. */
    ADP_DFBSurfaceToTDESurface(&state->dst, &state->destination->config, &tdev->dst_surface);
    /* Set the flag. */
    TDE_VALIDATE(DESTINATION);
}

static inline void
tde_validate_COLOR_DRAW(TDEDeviceData *tdev,
                        CardState *state)
{
    if (state->drawingflags & DSDRAW_SRC_PREMULTIPLY) {
        state->color.r = state->color.r * state->color.a >> 8;
        state->color.g = state->color.g * state->color.a >> 8;
        state->color.b = state->color.b * state->color.a >> 8;
    }
    tdev->color_pixel = PIXEL_ARGB(state->color.a,
                                   state->color.r,
                                   state->color.g,
                                   state->color.b);
    D_DEBUG_AT(TDE_VALIDATE, "%s %d blittingflags:%x, drawingflags:%x color_pixel:%x state->color.argb:%02x,%02x,%02x,%02x \n", __FUNCTION__, __LINE__,
               state->blittingflags,
               state->drawingflags,
               tdev->color_pixel,
               state->color.a,
               state->color.r,
               state->color.g,
               state->color.b);
    /* Set the flag. */
    TDE_VALIDATE(COLOR_DRAW);
}

/*
 * Called by SetState() to ensure that the color register is properly set
 * for execution of rendering functions.
 */
static inline void
tde_validate_COLOR_BLIT(TDEDeviceData *tdev,
                        CardState *state)
{
    int r, g, b, a;
    int s = 0xFF;

    if (state->blittingflags & DSBLIT_COLORIZE) {
        r = state->color.r;
        g = state->color.g;
        b = state->color.b;
    } else {
        r = g = b = 0xFF;
    }

    if (state->blittingflags & DSBLIT_BLEND_COLORALPHA) {
        a = state->color.a;
    } else {
        a = 0xFF;
    }

    if (state->blittingflags & DSBLIT_SRC_PREMULTCOLOR) {
        r = r * a / s;
        g = g * a / s;
        b = b * a / s;
    }

    tdev->color_pixel = PIXEL_ARGB((u8)a,
                                   (u8)r,
                                   (u8)g,
                                   (u8)b);
    D_DEBUG_AT(TDE_VALIDATE, "%s %d blittingflags:%x, drawingflags:%x a:%x, color_pixel:%x state->color.argb:%02x,%02x,%02x,%02x \n", __FUNCTION__, __LINE__,
               state->blittingflags,
               state->drawingflags,
               (u8)a,
               tdev->color_pixel,
               state->color.a,
               state->color.r,
               state->color.g,
               state->color.b);
    /* Set the flag. */
    TDE_VALIDATE(COLOR_BLIT);
}

/*
 * Called by SetState() to ensure that the source registers are properly set
 * for execution of blitting functions.
 */
static inline void
tde_validate_SOURCE(TDEDeviceData *tdev,
                    CardState *state)
{
    /* Remember source parameters for usage in rendering functions. */
    ADP_DFBSurfaceToTDESurface(&state->src, &state->source->config, &tdev->src_surface);

    if (TDE_IS_CLUT_FORMAT(tdev->src_surface.enColorFmt)) {
        CoreSurface *surface = state->source;
        CorePalette *palette = surface->palette;
        tdev->src_surface.pu8ClutPhyAddr = (mt_u8 *)dfb_gfxcard_memory_physical(NULL, tdev->lut_offset);
        tdev->src_surface.pu8ClutVirAddr = (mt_u8 *)dfb_gfxcard_memory_virtual(NULL, tdev->lut_offset);
        u32 *dst = dfb_gfxcard_memory_virtual(NULL, tdev->lut_offset);
        for (int i = 0; i < palette->num_entries; i++) {
            *dst++ = dfb_pixel_from_color(DSPF_ARGB, &palette->entries[i]);
        }
    }

    /* Set the flag. */
    TDE_VALIDATE(SOURCE);
}

/*
 * Called by vmwareSetState() to ensure that the source registers are properly set
 * for execution of blitting functions.
 */
static inline void
tde_validate_SOURCE_MASK(TDEDeviceData *tdev,
                         CardState *state)
{
    /* Remember source parameters for usage in rendering functions. */
    ADP_DFBSurfaceToTDESurface(&state->src_mask, &state->source_mask->config, &tdev->mask_surface);
    /* Set the flag. */
    TDE_VALIDATE(SOURCE_MASK);
}

static inline void
tde_validate_SOURCE2(TDEDeviceData *tdev,
                     CardState *state)
{
    /* Remember source parameters for usage in rendering functions. */
    ADP_DFBSurfaceToTDESurface(&state->src2, &state->source2->config, &tdev->bg_surface);
    /* Set the flag. */
    TDE_VALIDATE(SOURCE2);
}

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

static TDE_HANDLE prepare_tde_fifo(TDEDeviceData *tdev)
{
    TDE_HANDLE h = NULL;
#ifdef CONFIG_MT_TDE_CMD_FIFO_SUPPORT
    h = MT_TDE2_BeginJob();
    if (h == MT_ERR_TDE_INVALID_HANDLE) {
        D_DEBUG_AT(TDE_ERROR, "Error MT_TDE2_BeginJob");
        return h;
    }
    if (!tdev->cmd_fifo_handle) {
  	   if(tdev->enable_cmd_fifo_sync_mode) 
      	  tdev->cmd_fifo_handle = MT_TDE2_CmdFifo_CreateBeginEx(MT_TRUE);
	   else
      	  tdev->cmd_fifo_handle = MT_TDE2_CmdFifo_CreateBegin( );
	 
        D_ASSERT(tdev->cmd_fifo_handle);
    }
#endif
    return h;
}

static int submit_tde_fifo(TDEDeviceData *tdev, TDE_HANDLE tde_handle)
{
    int ret = 0;
#ifdef CONFIG_MT_TDE_CMD_FIFO_SUPPORT
    if (!tdev->cmd_fifo_handle) {
        return MT_FAILURE;
    }
    tdev->enable_cmd_fifo_sync_mode = 0;
    ret = MT_TDE2_CmdFifo_CreateEnd(tdev->cmd_fifo_handle);
    if (ret != MT_SUCCESS) {
        D_DEBUG_AT(TDE_ERROR, "Error MT_TDE2_CmdFifo_CreateEnd, ret:0x%x\n", ret);
    }
    ret = MT_TDE2_CmdFifo_Run(tdev->cmd_fifo_handle);
    if (ret != MT_SUCCESS) {
        D_DEBUG_AT(TDE_ERROR, "Error MT_TDE2_CmdFifo_Run, ret:0x%x\n", ret);
    }
    ret = MT_TDE2_WaitAllDone();
    if (ret != MT_SUCCESS) {
        D_DEBUG_AT(TDE_ERROR, "Error MT_TDE2_WaitAllDone, ret:0x%x\n", ret);
    }
    ret = MT_TDE2_CmdFifo_Destroy(tdev->cmd_fifo_handle);
    if (ret != MT_SUCCESS) {
        D_DEBUG_AT(TDE_ERROR, "Error MT_TDE2_CmdFifo_Destroy, ret:0x%x\n", ret);
    }
    ret = MT_TDE2_EndJob(tde_handle, MT_FALSE, TDE_FLAG, TIMEOUT);
    if (ret != MT_SUCCESS) {
        D_DEBUG_AT(TDE_ERROR, "Error MT_TDE2_EndJob");
    }
	
    tdev->cmd_fifo_handle = 0;
#endif
    return ret;
}

// //////////////////////////////////

static TDE_HANDLE prepare_tde_single()
{
    TDE_HANDLE h = MT_TDE2_BeginJob();
    if (h == MT_ERR_TDE_INVALID_HANDLE) {
        D_DEBUG_AT(TDE_ERROR, "Error MT_TDE2_BeginJob");
    }

    return h;
}

static int submit_tde_single(TDE_HANDLE tde_handle)
{
    int ret = 0;

    ret = MT_TDE2_EndJob(tde_handle, MT_FALSE, TDE_FLAG, TIMEOUT);
    if (ret != MT_SUCCESS) {
        D_DEBUG_AT(TDE_ERROR, "Error MT_TDE2_EndJob");
    }

    return ret;
}

/*
 * Wait for the blitter to be idle.
 *
 * This function is called before memory that has been written to by the hardware is about to be
 * accessed by the CPU (software driver) or another hardware entity like video encoder (by Flip()).
 * It can also be called by applications explicitly, e.g. at the end of a benchmark loop to include
 * execution time of queued commands in the measurement.
 */
DFBResult TDEEngineSync(void *drv, void *dev)
{
    int ret;

    FUNCTION_IN();
    /* wait all the task finish */
    ret = MT_TDE2_WaitAllDone();
    if (ret != MT_SUCCESS) {
        D_DEBUG_AT(TDE_ERROR, "TDE sync Engine failed\n");
        return DFB_FAILURE;
    }
    FUNCTION_OUT();

    return DFB_OK;
}

/*
 * Reset the graphics engine.
 */
void TDEEngineReset(void *drv, void *dev)
{
    /** no need to realize*/
    FUNCTION_IN();

    int ret = MT_TDE2_Reset();
    if (ret != MT_SUCCESS) {
        D_DEBUG_AT(TDE_ACC, "%s() Error ret:0x%x\n", __FUNCTION__, ret);
    }
    FUNCTION_OUT();
}

/*
 * Start processing of queued commands if required.
 *
 * This function is called before returning from the graphics core to the application.
 * Usually that's after each rendering function. The only functions causing multiple commands
 * to be queued with a single emition at the end are DrawString(), TileBlit(), BatchBlit(),
 * DrawLines() and possibly FillTriangle() which is emulated using multiple FillRectangle() calls.
 */
void TDEEmitCommands(void *drv, void *dev)
{
    /** no need to realize: all the tde commond is submit immediately*/
    FUNCTION_IN();

    if (MT_TDE2_WaitAllDone() != MT_SUCCESS) {
        D_DEBUG_AT(TDE_ERROR, "Error MT_TDE2_WaitAllDone\n");
    }

    FUNCTION_OUT();
}

/* Check if the function 'accel' can be accelerated with the 'state'. If that's true, the
function sets the 'accel' bit in 'state->accel'. Otherwise the function just returns, no
need to clear the bit.
attention: state include : source format, dst format, accele and accele flags*/
void TDECheckState(void *drv, void *dev, CardState *state, DFBAccelerationMask accel)
{

    D_DEBUG_AT(TDE_DEBUG, "%s( state %p, accel 0x%08x ) <- dest %p [%lu]\n", __FUNCTION__,
               state, accel, state->destination, state->dst.offset);

    /* step 1: check operation. Return if the desired function is not supported at all. */
    if (accel & ~(TDE_SUPPORTED_DRAWINGFUNCTIONS | TDE_SUPPORTED_BLITTINGFUNCTIONS)) {
        D_DEBUG_AT(TDE_UNSUPPORTED, "%s %d -> unsupported function, accel:0x%x\n", __FUNCTION__, __LINE__, accel);
        // printf( "  -> unsupported function, accel:0x%08x\n",accel);
        return;
    }

    /* Return if the source format is not supported. */

    /* Return if the destination format is not supported. */
    CHECK_TDE_DST_FORMAT(state->destination->config.format);
    /* step 2: Check if drawing or blitting is requested. */
    if (DFB_DRAWING_FUNCTION(accel)) {
        /* Return if unsupported drawing flags are set. */
        if (state->drawingflags & ~TDE_SUPPORTED_DRAWINGFLAGS) {
            D_DEBUG_AT(TDE_UNSUPPORTED, "%s %d -> unsupported drawing flags 0x%08x\n", __FUNCTION__, __LINE__, state->drawingflags);
            return;
        }
        /** when the pixel is too small , don't use TDE*/
        if (!IS_NEED_HDOPT(state->destination->config.size.w, state->destination->config.size.h)) {
            return;
        }

    } else {
        /* Return if the source format is not supported. */
        CHECK_TDE_SRC_FORMAT(state->source->config.format)

        /* Return if unsupported blitting flags are set. */
        if (state->blittingflags & ~TDE_SUPPORTED_BLITTINGFLAGS) {
            D_DEBUG_AT(TDE_UNSUPPORTED, "%s %d -> unsupported blitting flags 0x%08x\n", __FUNCTION__, __LINE__, state->blittingflags);
            return;
        }
        /* Mask checking. */
        if (state->blittingflags & (DSBLIT_SRC_MASK_ALPHA | DSBLIT_SRC_MASK_COLOR)) {
            if (!state->source_mask) {
                D_DEBUG_AT(TDE_UNSUPPORTED, "%s %d -> no source_mask blitting flags 0x%08x\n", __FUNCTION__, __LINE__, state->blittingflags);
                return;
            }
        }
    }

    /* Enable acceleration of the function. */
    state->accel |= accel;

    D_DEBUG_AT(TDE_DEBUG, "  => OK\n");
}

/*
    Program card for execution of the function 'accel' with the 'state'. 'state->modified'
contains information about changed entries. This function has to set at least 'accel' in
'state->set'. The driver should remember 'state->modified' and clear it. The driver may
modify 'funcs' depending on 'state' settings.
attention: don't chang the fields , if accel and field is not changged, else change the fields;
*/
void TDESetState(void *drv, void *dev, GraphicsDeviceFuncs *funcs, CardState *state, DFBAccelerationMask accel)
{
    //TDEDriverData           *tdrv     = drv;
    TDEDeviceData *tdev = dev;
    StateModificationFlags modified = state->mod_hw;

    D_DEBUG_AT(TDE_DEBUG, "%s( state %p, accel 0x%08x ) <- dest %p, modified 0x%08x, blittingflags:0x%x\n", __FUNCTION__,
               state, accel, state->destination, modified, state->blittingflags);

    /*
      * 1) Invalidate hardware states
      *
      * Each modification to the hw independent state invalidates one or more hardware states.
      */
	tdev->enable_cmd_fifo_sync_mode =  state->enable_cmdfifo_sync_mode;
	state->enable_cmdfifo_sync_mode = 0;
    /* Simply invalidate all */
    if (modified == SMF_ALL) {
        TDE_INVALIDATE(ALL);
    } else if (modified) {

        /* no draw flags */
        if (modified & SMF_BLITTING_FLAGS)
            TDE_INVALIDATE(BLITINGFLAGS);

        if (modified & SMF_DRAWING_FLAGS)
            TDE_INVALIDATE(DRAWINGFLAGS);

        if (modified & SMF_CLIP)
            TDE_INVALIDATE(CLIP);

        if (modified & SMF_COLOR)
            TDE_INVALIDATE(COLOR_DRAW | COLOR_BLIT);

        if (modified & SMF_DRAWING_FLAGS)
            TDE_INVALIDATE(COLOR_DRAW);

        if (modified & SMF_BLITTING_FLAGS)
            TDE_INVALIDATE(COLOR_BLIT);

        if (modified & (SMF_SRC_BLEND))
            TDE_INVALIDATE(SRC_BLEND);

        if (modified & (SMF_DST_BLEND))
            TDE_INVALIDATE(DST_BLEND);

        if (modified & SMF_SRC_COLORKEY)
            TDE_INVALIDATE(SRC_COLORKEY);

        if (modified & SMF_DST_COLORKEY)
            TDE_INVALIDATE(DST_COLORKEY);

        if (modified & SMF_DESTINATION)
            TDE_INVALIDATE(DESTINATION);

        if (modified & SMF_SOURCE)
            TDE_INVALIDATE(SOURCE);

        if (modified & SMF_SOURCE_MASK)
            TDE_INVALIDATE(SOURCE_MASK);

        if (modified & SMF_SOURCE2)
            TDE_INVALIDATE(SOURCE2);

        /* others variable no use, so don't need to add: 
                   SMF_MATRIX, SMF_RENDER_OPTIONS,
                   SMF_MATRIX, SMF_SOURCE_MASK, SMF_SOURCE_MASK_VALS, and so on*/
    }

    /*
      * 2) Validate hardware states
      *
      * Each function has its own set of states that need to be validated.
      */

    /* Always requiring valid destination... */
    TDE_CHECK_VALIDATE(DESTINATION);
    TDE_CHECK_VALIDATE(CLIP);
    /* Depending on the function... */
    if (DFB_DRAWING_FUNCTION(accel)) {
        /** check draw color */
        TDE_CHECK_VALIDATE(DRAWINGFLAGS);
        TDE_CHECK_VALIDATE(COLOR_DRAW);
        /** CHECK blend function, only support srcover opt, now*/
        TDE_CHECK_VALIDATE(SRC_BLEND);
        TDE_CHECK_VALIDATE(DST_BLEND);
        /*check color key*/
        TDE_CHECK_VALIDATE(DST_COLORKEY);
    } else if (DFB_BLITTING_FUNCTION(accel)) {
        /** check blit function */
        TDE_CHECK_VALIDATE(BLITINGFLAGS);
        /** check blit function */
        TDE_CHECK_VALIDATE(SOURCE);
        /** CHECK blend function, only support srcover opt, now*/
        TDE_CHECK_VALIDATE(SRC_BLEND);
        TDE_CHECK_VALIDATE(DST_BLEND);
        TDE_CHECK_VALIDATE(COLOR_BLIT);
        /** check color key*/
        TDE_CHECK_VALIDATE(SRC_COLORKEY);
        /** check dst color key*/
        TDE_CHECK_VALIDATE(DST_COLORKEY);
        if (accel == DFXL_BLIT2)
            TDE_CHECK_VALIDATE(SOURCE2);
        if (state->blittingflags & (DSBLIT_SRC_MASK_ALPHA | DSBLIT_SRC_MASK_COLOR))
            TDE_CHECK_VALIDATE(SOURCE_MASK);

    } else {
        D_DEBUG_AT(TDE_UNSUPPORTED, "  -> unsupported operate: 0x%x\n", accel);
    }

    return;
}

bool TDEFillRectangle(void *drv, void *dev, DFBRectangle *rect)
{
    MT_S32 ret;
    TDE2_RECT_S dstRect = {rect->x, rect->y, rect->w, rect->h};
    TDE_HANDLE hTDEHandle;
    TDEDeviceData *pDev;
    bool result = MT_TRUE;

    FUNCTION_IN();

    /**check tde operation*/
    if (!IS_NEED_HDOPT(rect->w, rect->h)) {
        return MT_FALSE;
    }

    pDev = (TDEDeviceData *)dev;
    if (trect_intersect(&dstRect, &pDev->clip_rect) < 0) {
        D_DERROR_AT(TDE_ERROR, "%s %d dst rect error\n", __FUNCTION__, __LINE__);
        dump_tde_rect(&dstRect, "TDEFillRectangle_dstRect");
        return MT_FALSE;
    }

    hTDEHandle = prepare_tde_single();
    if (hTDEHandle == MT_ERR_TDE_INVALID_HANDLE) {
        D_DERROR_AT(TDE_ERROR, "[%s][%d]Error prepare_tde_single\n", __FUNCTION__, __LINE__);
        return MT_FALSE;
    }
    do {
        if (pDev->drawingflags == DSDRAW_NOFX) {
            /** quick fill */
            ret = MT_TDE2_QuickFill(hTDEHandle, &pDev->dst_surface, &dstRect, pDev->color_pixel);
            if (ret != MT_SUCCESS) {
                D_DERROR_AT(TDE_ERROR, "[%s][%d] Error MT_TDE2_QuickFill\n", __FUNCTION__, __LINE__);
                result = MT_FALSE;
                break;
            }
        } else {
            TDE2_OPT_S TDEOpt;
            memset(&TDEOpt, 0, sizeof(TDE2_OPT_S));
            /** using solidDraw do alpha blending */

            /** prepare alpha,key,rop opt */
            ret = ADP_DFBGenerateTDEFillOpt(pDev, &TDEOpt, MT_FALSE);
            ret = MT_TDE2_Bitblit(hTDEHandle, NULL, NULL,
                                  NULL, NULL, &pDev->dst_surface,
                                  &dstRect,
                                  &TDEOpt);

            if (ret != MT_SUCCESS) {
                D_DEBUG_AT(TDE_ERROR, "[%s][%d] Error MT_TDE2_Bitblit\n", __FUNCTION__, __LINE__);
                result = MT_FALSE;
                break;
            }
        }
    } while (0);

    submit_tde_single(hTDEHandle);

    FUNCTION_OUT();
    return result;
}

bool TDEDrawRectangle(void *drv, void *dev, DFBRectangle *rect)
{
    DFBRectangle Rect0, Rect1, Rect2, Rect3;

    FUNCTION_IN();

    Rect0.x = rect->x;
    Rect0.y = rect->y;
    Rect0.w = rect->w;
    Rect0.h = 1;
    if (MT_TRUE != TDEFillRectangle(drv, dev, &Rect0)) {
        D_DEBUG_AT(TDE_ERROR, "draw rectangle failed\n");
        return MT_FALSE;
    }
    Rect1.x = rect->x;
    Rect1.y = rect->y;
    Rect1.w = 1;
    Rect1.h = rect->h;
    if (MT_TRUE != TDEFillRectangle(drv, dev, &Rect1)) {
        D_DEBUG_AT(TDE_ERROR, "draw rectangle failed\n");
        return MT_FALSE;
    }
    Rect2.x = rect->x + rect->w - 1;
    Rect2.y = rect->y;
    Rect2.w = 1;
    Rect2.h = rect->h;
    if (MT_TRUE != TDEFillRectangle(drv, dev, &Rect2)) {
        D_DEBUG_AT(TDE_ERROR, "draw rectangle failed\n");
        return MT_FALSE;
    }
    Rect3.x = rect->x;
    Rect3.y = rect->y + rect->h - 1;
    Rect3.w = rect->w;
    Rect3.h = 1;
    if (MT_TRUE != TDEFillRectangle(drv, dev, &Rect3)) {
        D_DEBUG_AT(TDE_ERROR, "draw rectangle failed\n");
        return MT_FALSE;
    }

    FUNCTION_OUT();
    return MT_TRUE;
}

bool TDEDrawLine(void *drv, void *dev, DFBRegion *line)
{
    /** no need to relize*/
    FUNCTION_IN();
    FUNCTION_OUT();
    return MT_FALSE;
}

bool TDEFillTriangle(void *drv, void *dev, DFBTriangle *tri)
{
    /** no need to relize*/
    FUNCTION_IN();
    FUNCTION_OUT();
    return MT_FALSE;
}


bool TDEBlit(void *drv, void *dev, DFBRectangle *srect, int dx, int dy)
{
    TDEDeviceData *pDev;
    MT_S32 ret = MT_SUCCESS;
    TDE_HANDLE hTDEHandle;
    TDE2_RECT_S srcRect = {srect->x, srect->y, (MT_U32)srect->w, (MT_U32)srect->h};
    TDE2_RECT_S dstRect = {dx, dy, (MT_U32)srect->w, (MT_U32)srect->h};
    TDE2_RECT_S MaskRect;
    TDE2_OPT_S opt;
    bool result = MT_TRUE;
    pDev = (TDEDeviceData *)dev;

    FUNCTION_IN();

    /** when the pixel is too small , don't use TDE*/
    if (!IS_NEED_HDOPT(srect->w, srect->h)) {
        return MT_FALSE;
    }

    memset(&opt, 0, sizeof(TDE2_OPT_S));
    /** RGB 2 RGB: attention no support clut blit*/
    D_DEBUG_AT(TDE_ACC, "%s %d Enter, blitflag:0x%x, drawflag:0x%x \n", __FUNCTION__, __LINE__, pDev->blittingflags, pDev->drawingflags);

    if (!calc_visible_rect(&srcRect, &dstRect, &pDev->clip_rect)) {
        dump_tde_rect(&srcRect, "TDEBlit_srect");
        dump_tde_rect(&dstRect, "TDEBlit_dstRect");
        dump_tde_rect(&pDev->clip_rect, "TDEBlit_clip_rect");
        D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error calc_visible_rect\n", __FUNCTION__, __LINE__);
        return MT_FALSE;
    }

    MaskRect.s32Xpos = 0;
    MaskRect.s32Ypos = 0;
    MaskRect.u32Width = dstRect.u32Width;
    MaskRect.u32Height = dstRect.u32Height;

    /** prepare alpha,key,rop opt */
    ret = ADP_DFBGenerateTDEOpt(pDev, &opt, MT_FALSE);
    if (MT_SUCCESS != ret) {
        return MT_FALSE;
    }

    /** do tde job*/
    hTDEHandle = prepare_tde_single();
    if (hTDEHandle == MT_ERR_TDE_INVALID_HANDLE) {
        D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error prepare_tde_single\n", __FUNCTION__, __LINE__);
        return MT_FALSE;
    }
    do {
        if (pDev->blittingflags & DSBLIT_COLORIZE) {
            ret = MT_TDE2_Bitblit_3src(hTDEHandle, NULL, NULL, NULL, NULL, &pDev->src_surface, &srcRect, &pDev->dst_surface,
                                       &dstRect, &opt);
            if (ret != MT_SUCCESS) {
                D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error MT_TDE2_Bitblit_3src, ret:0x%x\n", __FUNCTION__, __LINE__, ret);
                result = MT_FALSE;
                break;
            }
        } else if (pDev->blittingflags & (DSBLIT_SRC_MASK_ALPHA | DSBLIT_SRC_MASK_COLOR)) {
            ret = MT_TDE2_Bitblit_3src(hTDEHandle, NULL, NULL,
                                       &pDev->src_surface, &srcRect, &pDev->mask_surface, &MaskRect, &pDev->dst_surface,
                                       &dstRect, &opt);
            if (ret != MT_SUCCESS) {
                D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error MT_TDE2_Bitblit_3src, ret:0x%x\n", __FUNCTION__, __LINE__, ret);
                result = MT_FALSE;
                break;
            }
        } else {
            ret = MT_TDE2_Bitblit(hTDEHandle, NULL, NULL, &pDev->src_surface, &srcRect, &pDev->dst_surface,
                                  &dstRect, &opt);
            if (ret != MT_SUCCESS) {
                D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error MT_TDE2_Bitblit, ret:0x%x\n", __FUNCTION__, __LINE__, ret);
                result = MT_FALSE;
                break;
            }
        }
    } while (0);

    submit_tde_single(hTDEHandle);

    FUNCTION_OUT();

    return result;
}

bool TDEStretchBlit(void *drv, void *dev, DFBRectangle *srect, DFBRectangle *drect)
{
    TDEDeviceData *pDev;
    MT_S32 ret = MT_SUCCESS;
    TDE_HANDLE hTDEHandle;
    TDE2_RECT_S srcRect = {srect->x, srect->y, (MT_U32)srect->w, (MT_U32)srect->h};
    TDE2_RECT_S dstRect = {drect->x, drect->y, (MT_U32)drect->w, (MT_U32)drect->h};
    TDE2_RECT_S MaskRect;

    TDE2_OPT_S opt;
    bool result = MT_TRUE;

    FUNCTION_IN();

    memset(&opt, 0, sizeof(TDE2_OPT_S));

    if (!IS_NEED_HDOPT(srect->w, srect->h)) {
        return MT_FALSE;
    }

    if (!IS_NEED_HDOPT(drect->w, drect->h)) {
        return MT_FALSE;
    }

    /** RGB 2 RGB: attention no support clut blit*/
    pDev = (TDEDeviceData *)dev;
    D_DEBUG_AT(TDE_ACC, "%s %d Enter, blitflag:0x%x, drawflag:0x%x \n", __FUNCTION__, __LINE__, pDev->blittingflags, pDev->drawingflags);

    if (!calc_visible_rect_stretch(&srcRect, &dstRect, &pDev->clip_rect)) {
        dump_tde_rect(&srcRect, "TDEStretchBlit_srcRect");
        dump_tde_rect(&dstRect, "TDEStretchBlit_dstRect");
        dump_tde_rect(&pDev->clip_rect, "TDEStretchBlit_clip_rect");
        D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error calc_visible_rect\n", __FUNCTION__, __LINE__);
        return MT_FALSE;
    }

    /** prepare alpha,key,rop opt */
    ret = ADP_DFBGenerateTDEOpt(dev, &opt, MT_TRUE);
    if (MT_SUCCESS != ret) {
        return MT_FALSE;
    }

    MaskRect.s32Xpos = 0;
    MaskRect.s32Ypos = 0;
    MaskRect.u32Width = dstRect.u32Width;
    MaskRect.u32Height = dstRect.u32Height;

    /** do tde job*/
    hTDEHandle = prepare_tde_single();
    if (hTDEHandle == MT_ERR_TDE_INVALID_HANDLE) {
        D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error prepare_tde_single\n", __FUNCTION__, __LINE__);
        return MT_FALSE;
    }
    do {
        if (pDev->blittingflags & (DSBLIT_SRC_MASK_ALPHA | DSBLIT_SRC_MASK_COLOR)) {
            ret = MT_TDE2_Bitblit_3src(hTDEHandle, NULL, NULL,
                                       &pDev->src_surface, &srcRect, &pDev->mask_surface, &MaskRect, &pDev->dst_surface,
                                       &dstRect, &opt);
            if (ret != MT_SUCCESS) {
                D_DEBUG_AT(TDE_ERROR, "Error MT_TDE2_Bitblit_3src, ret:0x%x\n", ret);
                result = MT_FALSE;
                break;
            }
        } else {
            ret = MT_TDE2_Bitblit(hTDEHandle, NULL, NULL, &pDev->src_surface, &srcRect, &pDev->dst_surface, &dstRect, &opt);
            if (ret != MT_SUCCESS) {
                D_DEBUG_AT(TDE_ERROR, "[%s][%d] Error MT_TDE2_Bitblit, ret:0x%x, blittingflags:0x%x\n", __FUNCTION__, __LINE__, ret, pDev->blittingflags);
                result = MT_FALSE;
                break;
            }
        }

    } while (0);
    submit_tde_single(hTDEHandle);

    FUNCTION_OUT();
    return result;
}

bool TDEBlit2(void *drv, void *dev, DFBRectangle *srect, int dx, int dy, int sx2, int sy2)
{
    TDEDeviceData *pDev = (TDEDeviceData *)dev;
    ;
    MT_S32 ret = MT_SUCCESS;
    TDE_HANDLE hTDEHandle;
    TDE2_RECT_S srcRect = {srect->x, srect->y, (MT_U32)srect->w, (MT_U32)srect->h};
    TDE2_RECT_S dstRect = {dx, dy, (MT_U32)srect->w, (MT_U32)srect->h};
    TDE2_RECT_S BgRect = {sx2, sy2, (MT_U32)srect->w, (MT_U32)srect->h};
    TDE2_RECT_S MaskRect;
    TDE2_OPT_S opt;
    bool result = MT_TRUE;

    FUNCTION_IN();

    /** when the pixel is too small , don't use TDE*/
    if (!IS_NEED_HDOPT(srect->w, srect->h)) {
        return MT_FALSE;
    }

    /** RGB 2 RGB: attention no support clut blit*/
    D_DEBUG_AT(TDE_ACC, "%s %d Enter, blitflag:0x%x, drawflag:0x%x \n", __FUNCTION__, __LINE__, pDev->blittingflags, pDev->drawingflags);

    MaskRect.s32Xpos = 0;
    MaskRect.s32Ypos = 0;
    MaskRect.u32Width = dstRect.u32Width;
    MaskRect.u32Height = dstRect.u32Height;

    /** prepare alpha,key,rop opt */
    memset(&opt, 0, sizeof(TDE2_OPT_S));
    ret = ADP_DFBGenerateTDEOpt(pDev, &opt, MT_FALSE);
    if (MT_SUCCESS != ret) {
        return MT_FALSE;
    }

    /** do tde job*/
    hTDEHandle = prepare_tde_single();
    if (hTDEHandle == MT_ERR_TDE_INVALID_HANDLE) {
        D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error prepare_tde_single\n", __FUNCTION__, __LINE__);
        return MT_FALSE;
    }
    do {
        if (pDev->blittingflags & DSBLIT_COLORIZE) {
            ret = MT_TDE2_Bitblit_3src(hTDEHandle, &pDev->bg_surface, &BgRect,
                                       NULL, NULL, &pDev->src_surface, &srcRect, &pDev->dst_surface,
                                       &dstRect, &opt);
            if (ret != MT_SUCCESS) {
                D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error MT_TDE2_Bitblit_3src, ret:0x%x\n", __FUNCTION__, __LINE__, ret);
                result = MT_FALSE;
                break;
            }
        } else if (pDev->blittingflags & (DSBLIT_SRC_MASK_ALPHA | DSBLIT_SRC_MASK_COLOR)) {
            ret = MT_TDE2_Bitblit_3src(hTDEHandle, &pDev->bg_surface, &BgRect,
                                       &pDev->src_surface, &srcRect, &pDev->mask_surface, &MaskRect, &pDev->dst_surface,
                                       &dstRect, &opt);
            if (ret != MT_SUCCESS) {
                D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error MT_TDE2_Bitblit_3src, ret:0x%x\n", __FUNCTION__, __LINE__, ret);
                result = MT_FALSE;
                break;
            }
        } else {
            ret = MT_TDE2_Bitblit(hTDEHandle, &pDev->bg_surface, &BgRect, &pDev->src_surface, &srcRect, &pDev->dst_surface,
                                  &dstRect, &opt);
            if (ret != MT_SUCCESS) {
                D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error MT_TDE2_Bitblit, ret:0x%x\n", __FUNCTION__, __LINE__, ret);
                result = MT_FALSE;
                break;
            }
        }
    } while (0);

    submit_tde_single(hTDEHandle);

    FUNCTION_OUT();
    return result;
}

#ifdef CONFIG_MT_TDE_CMD_FIFO_SUPPORT

bool batchQuickFill(TDEDeviceData *pDev,
                    const DFBRectangle *rects,
                    unsigned int num, unsigned int *ret_num)
{
    mt_s32 ret = MT_SUCCESS;
    TDE_HANDLE hTDEHandle;
    bool result = MT_TRUE;
    unsigned int i;
    unsigned int j;
    unsigned int batches;
    unsigned int suceessed = 0;

    FUNCTION_IN();
    D_ASSERT(pDev != NULL);
    D_ASSERT(rects != NULL);
    D_ASSERT(ret_num != NULL);
    *ret_num = 0;

    j = 0;
    while (num != 0) {
        batches = num > MAX_CMD_FIFO_COUNT ? MAX_CMD_FIFO_COUNT : num;
        TDE2_RECT_S dstRects[batches];
        mt_u32 colors[batches];
        for (i = 0; i < batches; ++i, ++j) {
            drect2trect(&dstRects[i], &rects[j]);
            if (trect_intersect(&dstRects[i], &pDev->clip_rect) < 0) {
                D_DEBUG_AT(TDE_ERROR, "%s %d dst rect error\n", __FUNCTION__, __LINE__);
                dump_tde_rect(&dstRects[i], "batchQuickFill_dstRect");
                ret = MT_FAILURE;
                break;
            }
            colors[i] = pDev->color_pixel;
        }
        if (ret != MT_SUCCESS) {
            // break from while
            break;
        }

        hTDEHandle = prepare_tde_fifo(pDev);
        if (hTDEHandle == MT_ERR_TDE_INVALID_HANDLE) {
            D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error prepare_tde_fifo\n", __FUNCTION__, __LINE__);
            ret = MT_FAILURE;
            break;
        }
        ret = MT_TDE2_CmdFifo_BatchFill(hTDEHandle, &pDev->dst_surface, dstRects, &colors, batches, &suceessed);
        if (ret != MT_SUCCESS || suceessed != batches) {
            submit_tde_fifo(pDev, hTDEHandle);
            D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error MT_TDE2_CmdFifo_BatchFill\n", __FUNCTION__, __LINE__);
            break;
        }
        ret = submit_tde_fifo(pDev, hTDEHandle);
        if (ret != MT_SUCCESS) {
            D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error submit_tde_fifo\n", __FUNCTION__, __LINE__);
            break;
        }
        num -= batches;
        *ret_num += batches;
    }

    FUNCTION_OUT();
    // if the one or more successed, return success.
    return (ret == MT_SUCCESS) | (*ret_num);
}

bool batchEffectsFill(TDEDeviceData *pDev,
                      const DFBRectangle *rects,
                      unsigned int num, unsigned int *ret_num)
{
    mt_s32 ret = MT_SUCCESS;
    TDE_HANDLE hTDEHandle;
    bool result = MT_TRUE;
    unsigned int i;
    unsigned int j;
    unsigned int batches;
    unsigned int suceessed = 0;
    TDE2_OPT_S opt;

    FUNCTION_IN();

    D_ASSERT(pDev != NULL);
    D_ASSERT(rects != NULL);
    D_ASSERT(ret_num != NULL);
    *ret_num = 0;

    ret = ADP_DFBGenerateTDEFillOpt(pDev, &opt, MT_FALSE);
    if (ret != MT_SUCCESS) {
        D_DEBUG_AT(TDE_ERROR, "%s %d Error ADP_DFBGenerateTDEFillOpt\n", __FUNCTION__, __LINE__);
        return MT_FALSE;
    }

    j = 0;
    while (num != 0) {
        batches = num > MAX_CMD_FIFO_COUNT ? MAX_CMD_FIFO_COUNT : num;
        TDE2_RECT_S dstRects[batches];
        TDE_BLIT_INFO_S blitInfos[batches];
        TDE_BLIT_LIST_S blitList = {0};

        for (i = 0; i < batches; ++i, ++j) {
            drect2trect(&dstRects[i], &rects[j]);
            if (trect_intersect(&dstRects[i], &pDev->clip_rect) < 0) {
                D_DEBUG_AT(TDE_ERROR, "%s %d dst rect error\n", __FUNCTION__, __LINE__);
                dump_tde_rect(&dstRects[i], "batchEffectsFill_dstRect");
                ret = MT_FAILURE;
                break;
            }
            blitInfos[i].pstSrcSur = NULL;
            blitInfos[i].pstInRect = NULL;
            blitInfos[i].pstBgSur = NULL;
            blitInfos[i].pstBgRect = NULL;
            blitInfos[i].pstMaskSur = NULL;
            blitInfos[i].pstMaskRect = NULL;
            blitInfos[i].pstOutRect = &dstRects[i];
            blitInfos[i].pstOpt = &opt;
        }
        if (ret != MT_SUCCESS) {
            // break from while
            break;
        }
        blitList.u32BlitNum = batches;
        blitList.pDstSurface = &pDev->dst_surface;
        blitList.pstComposor = blitInfos;

        hTDEHandle = prepare_tde_fifo(pDev);
        if (hTDEHandle == MT_ERR_TDE_INVALID_HANDLE) {
            D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error prepare_tde_fifo\n", __FUNCTION__, __LINE__);
            ret = MT_FAILURE;
            break;
        }
        ret = MT_TDE2_CmdFifo_BatchBlit(hTDEHandle, &blitList, &suceessed);
        if (ret != MT_SUCCESS || suceessed != batches) {
            submit_tde_fifo(pDev, hTDEHandle);
            D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error MT_TDE2_CmdFifo_BatchBlit, ret:0x%x\n", __FUNCTION__, __LINE__, ret);
            break;
        }
        ret = submit_tde_fifo(pDev, hTDEHandle);
        if (ret != MT_SUCCESS) {
            D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error submit_tde_fifo\n", __FUNCTION__, __LINE__);
            break;
        }
        num -= batches;
        *ret_num += batches;
    }

    FUNCTION_OUT();

    // if the one or more successed, return success.
    return (ret == MT_SUCCESS) | (*ret_num);
}

bool TDEBatchBlit(void *driver_data, void *device_data,
                  const DFBRectangle *rects, const DFBPoint *points,
                  unsigned int num, unsigned int *ret_num)
{
    TDEDeviceData *pDev = device_data;
    unsigned int i;
    unsigned int j;
    TDE2_OPT_S opt;
    mt_s32 ret = MT_SUCCESS;
    TDE_HANDLE hTDEHandle;
    TDE2_RECT_S maskRect;
    unsigned int suceessed = 0;
    unsigned int batches;

    FUNCTION_IN();

    D_ASSERT(pDev != NULL);
    D_ASSERT(rects != NULL);
    D_ASSERT(points != NULL);
    D_ASSERT(ret_num != NULL);
    *ret_num = 0;

    memset(&opt, 0, sizeof(TDE2_OPT_S));
    ret = ADP_DFBGenerateTDEOpt(pDev, &opt, MT_FALSE);
    if (ret != MT_SUCCESS) {
        return MT_FALSE;
    }

    D_DEBUG_AT(TDE_CMD_FIFO, "[%s][%d] blittingflags:0x%x, drawingflags:0x%x, num:%d\n", __FUNCTION__, __LINE__, pDev->blittingflags, pDev->drawingflags, num);

    j = 0;
    while (num != 0) {
        batches = num > MAX_CMD_FIFO_COUNT ? MAX_CMD_FIFO_COUNT : num;
        TDE_BLIT_INFO_S blitInfos[batches];
        TDE2_RECT_S srcRects[batches];
        TDE2_RECT_S dstRects[batches];
        TDE2_RECT_S maskRects[batches];
        TDE_BLIT_LIST_S blitList = {0};

        for (i = 0; i < batches; ++i, ++j) {
            drect2trect(&srcRects[i], &rects[j]);
            init_trect_val(&dstRects[i], points[j].x, points[j].y, rects[j].w, rects[j].h);
            if (!calc_visible_rect(&srcRects[i], &dstRects[i], &pDev->clip_rect)) {
                D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error calc_visible_rect\n", __FUNCTION__, __LINE__);
                dump_tde_rect(&srcRects[i], "TDEBatchBlit_srcRects[i]");
                dump_tde_rect(&dstRects[i], "TDEBatchBlit_dstRects[i]");
                dump_tde_rect(&pDev->clip_rect, "TDEBatchBlit_clip_rect");
                ret = MT_FAILURE;
                break;
            }

            if (pDev->blittingflags & DSBLIT_COLORIZE) {
                blitInfos[i].pstSrcSur = NULL;
                blitInfos[i].pstInRect = NULL;
                blitInfos[i].pstMaskSur = &pDev->src_surface;
                blitInfos[i].pstMaskRect = &srcRects[i];
            } else if (pDev->blittingflags & (DSBLIT_SRC_MASK_ALPHA | DSBLIT_SRC_MASK_COLOR)) {
                init_trect_val(&maskRects[i], 0, 0, dstRects[i].u32Width, dstRects[i].u32Height);
                blitInfos[i].pstSrcSur = &pDev->src_surface;
                blitInfos[i].pstInRect = &srcRects[i];
                blitInfos[i].pstMaskSur = &pDev->mask_surface;
                blitInfos[i].pstMaskRect = &maskRects[i];
            } else {
                blitInfos[i].pstSrcSur = &pDev->src_surface;
                blitInfos[i].pstInRect = &srcRects[i];
                blitInfos[i].pstMaskSur = NULL;
                blitInfos[i].pstMaskRect = NULL;
            }
            blitInfos[i].pstBgSur = NULL;
            blitInfos[i].pstBgRect = NULL;
            blitInfos[i].pstOutRect = &dstRects[i];
            blitInfos[i].pstOpt = &opt;
        }
        if (ret != MT_SUCCESS) {
            // break from while
            break;
        }
        blitList.u32BlitNum = batches;
        blitList.pDstSurface = &pDev->dst_surface;
        blitList.pstComposor = blitInfos;

        hTDEHandle = prepare_tde_fifo(pDev);
        if (hTDEHandle == MT_ERR_TDE_INVALID_HANDLE) {
            D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error prepare_tde_fifo\n", __FUNCTION__, __LINE__);
            ret = MT_FAILURE;
            break;
        }

        ret = MT_TDE2_CmdFifo_BatchBlit(hTDEHandle, &blitList, &suceessed);
        if (ret != MT_SUCCESS || suceessed != batches) {
            submit_tde_fifo(pDev, hTDEHandle);
            D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error MT_TDE2_CmdFifo_BatchBlit, ret:0x%x\n", __FUNCTION__, __LINE__, ret);
            break;
        }

        ret = submit_tde_fifo(pDev, hTDEHandle);
        if (ret != MT_SUCCESS) {
            D_DEBUG_AT(TDE_ERROR, "[%s][%d]Error submit_tde_fifo\n", __FUNCTION__, __LINE__);
            break;
        }
        num -= batches;
        *ret_num += batches;
    }

    FUNCTION_OUT();

    // if the one or more successed, return success.
    return (ret == MT_SUCCESS) | (*ret_num);
}

bool TDEBatchFill(void *driver_data, void *device_data,
                  const DFBRectangle *rects,
                  unsigned int num, unsigned int *ret_num)
{

    TDEDeviceData *pDev = (TDEDeviceData *)device_data;
    bool result;

    FUNCTION_IN();

    D_ASSERT(pDev != NULL);
    D_ASSERT(rects != NULL);
    D_ASSERT(ret_num != NULL);
    *ret_num = 0;

    D_DEBUG_AT(TDE_CMD_FIFO, "[%s][%d] blittingflags:0x%x, drawingflags:0x%x, num:%d\n", __FUNCTION__, __LINE__, pDev->blittingflags, pDev->drawingflags, num);

    if (pDev->drawingflags == DSDRAW_NOFX) {
        result = batchQuickFill(pDev, rects, num, ret_num);
    } else {
        result = batchEffectsFill(pDev, rects, num, ret_num);
    }

    FUNCTION_OUT();
    return result;
}
#endif
