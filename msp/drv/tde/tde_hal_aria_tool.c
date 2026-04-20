#ifndef TDE_BOOT
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <asm/barrier.h>
#else
#include "list.h"
#endif
#include "tde_hal.h"
#include "tde_define.h"
#include "wmalloc.h"

#include "mt_common.h"
//#include "tde_filterPara.h"
#include "tde_adp.h"

#include "tde_hal_aria_reg_addr.h"
#include "tde_hal_aria_reg_def.h"

#include "tde_hal_aria.h"
#include "tde_hal_aria_vsb.h"
#include "mt_module_debug.h"

#include <asm/div64.h>

//#define TDE_DEBUG_DISABLE_1
#if defined(TDE_DEBUG_DISABLE) || defined(TDE_DEBUG_DISABLE_1)
#define DUMP_LOG \
    do {         \
    } while (0)
#define TDE_FUN_IN DUMP_LOG
#define TDE_FUN_OUT DUMP_LOG
#define TDE_LOG(...) DUMP_LOG
#define TDE_LINE DUMP_LOG
#else
#define TDE_FUN_IN MT_INFO_TDE("--------------in------------\n")
#define TDE_FUN_OUT MT_INFO_TDE("------------out------------\n")
#define TDE_LOG MT_INFO_TDE
#define TDE_LINE MT_INFO_TDE("------------------\n")
#endif

#if 0
static void spn_gpe_calc_pattern(spn_gpe_context_t *p_ctx)
{
    u32 beg_x = p_ctx->paint.pat_beg.x;
    u32 end_x = p_ctx->dst_img.rect.w - p_ctx->paint.pat_beg.x;
    u32 beg_y = p_ctx->paint.pat_beg.y;
    u32 end_y = p_ctx->dst_img.rect.h - p_ctx->paint.pat_beg.y;
    u32 src_w = p_ctx->src_img.rect.w;
    u32 src_h = p_ctx->src_img.rect.h;

    p_ctx->paint.pat_remd1 = beg_x % src_w;
    p_ctx->paint.pat_quot1 = beg_x / src_w;
    p_ctx->paint.pat_remd2 = end_x % src_w;
    p_ctx->paint.pat_quot2 = end_x / src_w;
    p_ctx->paint.pat_remd3 = beg_y % src_h;
    p_ctx->paint.pat_quot3 = beg_y / src_h;
    p_ctx->paint.pat_remd4 = end_y % src_h;
    p_ctx->paint.pat_quot4 = end_y / src_h;
}


static void spn_gpe_generate_radial_gradient(spn_gpe_context_t *p_ctx)
{
    u32 width = p_ctx->dst_img.rect.w;
    u32 height = p_ctx->dst_img.rect.h;
    u32 cx = p_ctx->paint.center.x;
    u32 cy = p_ctx->paint.center.y;
    u32 fx = p_ctx->paint.focus.x;
    u32 fy = p_ctx->paint.focus.y;
    u32 r = p_ctx->paint.radius;
    int fc_x = fx - cx;
    int fc_y = fy - cy;
    u32 fc_len = 0;
    u32 x = 0;
    u32 y = 0;
    u32 d1 = 0;
    u32 d2 = 0;
    double gradt = 0;
    int dx = 0;
    int dy = 0;
    u32 tmp = 0;
    int tmp1 = 0;
    int tmp2 = 0;
    u32 dy_dy = 0;
    int dy_fcy = 0;
    u32 r_r = 0;
    u16 *p_ptr = NULL;
    u64 val = 0;
    u32 pitch = 0;
//  pitch = (width * 2 + 127) / 128 * 128;
//  p_ctx->p_gradt_buf = (u16 *)mtos_align_malloc(pitch * height, 128);
    pitch = width * 2;
#ifdef SYMPHONY_VERIFICATION
    if(g_debug.tmp_mem == TRUE)
    {
      p_ctx->p_gradt_buf = (u16 *)0xb6000028;
    }
    else
    {
      p_ctx->p_gradt_buf = (u16 *)mtos_malloc(pitch * height);
    }
#else
    p_ctx->p_gradt_buf = (u16 *)mtos_malloc(pitch * height);
#endif

    if(p_ctx->p_gradt_buf == NULL)
    {
      GPE_PRINT("\n\rgradt_buf malloc failed!");
      GPE_SPN_ASSERT(0);
    }
    GPE_PRINT("\r\n f:[%d,%d]", fx, fy);
    p_ptr = p_ctx->p_gradt_buf;
    val = fc_x * fc_x + fc_y * fc_y;
    fc_len = (u32)(sqrt(val));
    if(fc_len >= r) //the focus point is outsize the circle, clamp it
    {
        if(fc_x > 0)
        {
          fx = (fc_x * r * 1023 / 1024) / fc_len + cx;
        }
        else
        {
          fx = cx - ((cx - fx) * r * 1023 / 1024 / fc_len);
        }
         if(fc_y > 0)
        {
          fy = (fc_y * r * 1023 / 1024) / fc_len + cy;
        }
        else
        {
          fy = cy - ((cy - fy) * r * 1023 / 1024 / fc_len);
        }
        fc_x = fx - cx;
        fc_y = fy - cy;
    }

    GPE_PRINT("\r\n c[%d,%d], f:[%d,%d]", cx, cy, fx, fy);
    r_r = r * r;

    for(y = 0; y < height; y ++)
    {
        dy = y - fy;
        dy_dy = dy * dy;
        dy_fcy = dy * fc_y;
        for(x = 0; x < width; x ++)
        {
            dx = x - fx;
            d1 = dx * dx + dy_dy;
            tmp1 = dx * fc_y - dy * fc_x;
            tmp2 = dx * fc_x + dy_fcy;
            val = (u64)r_r * (u64)d1 - (u64)tmp1 * (u64)tmp1;
            d2 = (u32)sqrt(val) - tmp2;

            if(d2 == 0)
                gradt = 0.0;
            else
                gradt = (double)d1 / d2;

            switch(p_ctx->paint.spread_mod)
            {
                case GPE_SPN_SPREAD_PAD:
                    if(gradt > 1.0)
                        gradt = 1.0;
                    else if(gradt < 0.0)
                        gradt = 0.0;
                    break;
                case GPE_SPN_SPREAD_REPEAT:
                    if((gradt > 1.0)  ||  (gradt < 0.0))
                        gradt = gradt - my_floor(gradt);
                    break;
                case GPE_SPN_SPREAD_REFLECT:
                    if((gradt > 1.0)  ||  (gradt < 0.0))
                    {
                        if((my_floor(gradt) & 0x1) == 0)
                            gradt = gradt - my_floor(gradt);
                        else
                            gradt = 1.0 - (gradt - my_floor(gradt));
                    }
                    break;
                default:
                    break;
            }

            if(gradt == 1.0)
                p_ptr[y * pitch / 2 + x] = (1  <<  12) - 1;
            else
                p_ptr[y * pitch / 2 + x] = (u16)(gradt * (1  <<  12));
        }
    }

    hal_dcache_flush((void *)(p_ctx->p_gradt_buf), pitch * height);

    p_ctx->src_img_en = TRUE;
    p_ctx->src_img.buf = (u32)(p_ctx->p_gradt_buf);
    p_ctx->src_img.width = p_ctx->dst_img.rect.w;
    p_ctx->src_img.height = p_ctx->dst_img.rect.h;
    p_ctx->src_img.pitch = pitch;
    p_ctx->src_img.rect.w = p_ctx->src_img.width;
    p_ctx->src_img.rect.h = p_ctx->src_img.width;
    p_ctx->src_img.rect.x = 0;
    p_ctx->src_img.rect.y = 0;
    p_ctx->src_img.ck_en = FALSE;
    p_ctx->src_img.plane_alpha_en = FALSE;
    p_ctx->src_img.color_info.alpha_ch_en = FALSE;
    p_ctx->src_img.color_info.alpha_pre_mult_en = FALSE;

    p_ctx->src_img.color_info.bpp = GPE_SPN_BPP_16BIT;
    p_ctx->src_img.color_info.color_fmt = GRAY_16;
    p_ctx->src_img.color_info.color_space = GRAY_COLOR_SPACE;
    p_ctx->src_img.color_info.is_pix_alpha = FALSE;
    p_ctx->src_img.color_info.little_endian = FALSE;
    p_ctx->src_img.with_palette = FALSE;
    p_ctx->src_is_xylc = FALSE;
    p_ctx->src_is_tile = FALSE;

}



static void spn_gpe_calc_gradt_stop_fact(paint_cfg_t *p_paint)
{
    u32 data = 0;
    u32 offset0 = p_paint->stop0.offset;
    u32 offset1 = p_paint->stop1.offset;
    u32 offset2 = p_paint->stop2.offset;
    u32 offset3 = p_paint->stop3.offset;


    if(offset1 == offset0)
    {
        data = 0;
    }
    else
    {
        data = ((1   <<   24) / (offset1 - offset0)) & 0xffffff;
    }
    p_paint->stop_fact0 = data;

    if(offset2 == offset1)
    {
        data = 0;
    }
    else
    {
        data = ((1   <<   24) / (offset2 - offset1)) & 0xffffff;
    }
    p_paint->stop_fact1 = data;

    if(offset3 == offset2)
    {
        data = 0;
    }
    else
    {
        data = ((1   <<   24) / (offset3 - offset2)) & 0xffffff;
    }
    p_paint->stop_fact2 = data;
}



static void spn_gpe_calc_liner_gradient(paint_cfg_t *p_paint)
{
    u32 x0 = p_paint->begin.x;
    u32 x1 = p_paint->end.x;
    u32 y0 = p_paint->begin.y;
    u32 y1 = p_paint->end.y;
    int xd = 0;
    int yd = 0;
    int numerator = 0;
    int denominator = 0;

    xd = x0 - x1;
    yd = y0 - y1;
    if((xd == 0)  &&  (yd == 0))
    {
        p_paint->gradt_start = (1   <<   20);
        p_paint->step_x = 0;
        p_paint->step_y = 0;
    }
    else
    {
        denominator = xd * xd + yd * yd;
        numerator = x0 * xd + y0 * yd;
        p_paint->gradt_start = (int)(((double)numerator / (double)denominator) * (1   <<   20));
        p_paint->step_x = (int)(((double)(- xd) / (double)denominator) * (1   <<   21));
        p_paint->step_y = (int)(((double)(- yd) / (double)denominator) * (1   <<   21));
    }
}


MT_BOOL tde_hal_set_paint_info(paint_cfg_t *p_paint, paint_info_t *paint_info)
{
    MT_BOOL ret = TRUE;
    u32 mod = 0;
    u32 mask_mod = 0;

    switch(paint_info->paint_type)
    {
        case VG_PAINT_TYPE_COLOR:
        p_paint->is_pattern_paint = FALSE;
        p_paint->true_liner_gradt = FALSE;
        //hardware theat flat color paint as liner gradient paint
        p_paint->gradt_type = GPE_SPN_LINER_GRADT;
        p_paint->paint_color = paint_info->paint_color;
        p_paint->spread_mod = GPE_SPN_FLAT_COLOR_FILL;
        break;
    case VG_PAINT_TYPE_LINEAR_GRADIENT:
        p_paint->is_pattern_paint = FALSE;
        p_paint->true_liner_gradt = TRUE;
        p_paint->gradt_type = GPE_SPN_LINER_GRADT;
        p_paint->begin.x = paint_info->paint_liner_gradt.begin.x;
        p_paint->begin.y = paint_info->paint_liner_gradt.begin.y;
        p_paint->end.x = paint_info->paint_liner_gradt.end.x;
        p_paint->end.y = paint_info->paint_liner_gradt.end.y;
        p_paint->stop0.argb = paint_info->paint_liner_gradt.stop0.argb;
        p_paint->stop1.argb = paint_info->paint_liner_gradt.stop1.argb;
        p_paint->stop2.argb = paint_info->paint_liner_gradt.stop2.argb;
        p_paint->stop3.argb = paint_info->paint_liner_gradt.stop3.argb;
        p_paint->stop0.offset = paint_info->paint_liner_gradt.stop0.offset;
        p_paint->stop1.offset = paint_info->paint_liner_gradt.stop1.offset;
        p_paint->stop2.offset = paint_info->paint_liner_gradt.stop2.offset;
        p_paint->stop3.offset = paint_info->paint_liner_gradt.stop3.offset;
        spn_gpe_calc_liner_gradient(p_paint);
        spn_gpe_calc_gradt_stop_fact(p_paint);
        break;
    case VG_PAINT_TYPE_RADIAL_GRADIENT:
        p_paint->is_pattern_paint = FALSE;
        p_paint->true_liner_gradt = FALSE;
        p_paint->gradt_type = GPE_SPN_RADIAL_GRADT;
        p_paint->center.x = paint_info->paint_radial_gradt.center.x;
        p_paint->center.y = paint_info->paint_radial_gradt.center.y;
        p_paint->focus.x = paint_info->paint_radial_gradt.focus.x;
        p_paint->focus.y = paint_info->paint_radial_gradt.focus.y;
        p_paint->radius = paint_info->paint_radial_gradt.radius;
        p_paint->stop0.argb = paint_info->paint_radial_gradt.stop0.argb;
        p_paint->stop1.argb = paint_info->paint_radial_gradt.stop1.argb;
        p_paint->stop2.argb = paint_info->paint_radial_gradt.stop2.argb;
        p_paint->stop3.argb = paint_info->paint_radial_gradt.stop3.argb;
        p_paint->stop0.offset = paint_info->paint_radial_gradt.stop0.offset;
        p_paint->stop1.offset = paint_info->paint_radial_gradt.stop1.offset;
        p_paint->stop2.offset = paint_info->paint_radial_gradt.stop2.offset;
        p_paint->stop3.offset = paint_info->paint_radial_gradt.stop3.offset;
        spn_gpe_calc_gradt_stop_fact(p_paint);
        break;
    case VG_PAINT_TYPE_ELLIPSE_GRADIENT:
        p_paint->is_pattern_paint = FALSE;
        p_paint->true_liner_gradt = FALSE;
        p_paint->gradt_type = GPE_SPN_ELLIPSE_GRADT;
        p_paint->center.x = paint_info->paint_ellipse_gradt.center.x;
        p_paint->center.y = paint_info->paint_ellipse_gradt.center.y;
        p_paint->focus.x = paint_info->paint_ellipse_gradt.focus.x;
        p_paint->focus.y = paint_info->paint_ellipse_gradt.focus.y;
        p_paint->long_a = paint_info->paint_ellipse_gradt.long_a;
        p_paint->short_b = paint_info->paint_ellipse_gradt.short_b;
        p_paint->stop0.argb = paint_info->paint_ellipse_gradt.stop0.argb;
        p_paint->stop1.argb = paint_info->paint_ellipse_gradt.stop1.argb;
        p_paint->stop2.argb = paint_info->paint_ellipse_gradt.stop2.argb;
        p_paint->stop3.argb = paint_info->paint_ellipse_gradt.stop3.argb;
        p_paint->stop0.offset = paint_info->paint_ellipse_gradt.stop0.offset;
        p_paint->stop1.offset = paint_info->paint_ellipse_gradt.stop1.offset;
        p_paint->stop2.offset = paint_info->paint_ellipse_gradt.stop2.offset;
        p_paint->stop3.offset = paint_info->paint_ellipse_gradt.stop3.offset;
        p_paint->stop_num = paint_info->paint_ellipse_gradt.stop_num;
        spn_gpe_calc_gradt_stop_fact(p_paint);
        break;
    case VG_PAINT_TYPE_PATTERN:
        p_paint->is_pattern_paint = TRUE;
        p_paint->true_liner_gradt = FALSE;
        p_paint->pat_beg.x = paint_info->paint_pattern.pat_beg.x;
        p_paint->pat_beg.y = paint_info->paint_pattern.pat_beg.y;
        if(paint_info->paint_pattern.tiling_mod == VG_TILE_FILL)
        {
           p_paint->paint_color = paint_info->paint_pattern.fill_color;
        }
      break;
    default:
        ret = FALSE;
        break;
    }

    if((paint_info->paint_type == VG_PAINT_TYPE_LINEAR_GRADIENT)  ||
      (paint_info->paint_type == VG_PAINT_TYPE_RADIAL_GRADIENT) ||
      (paint_info->paint_type == VG_PAINT_TYPE_ELLIPSE_GRADIENT))
    {
        if(paint_info->paint_type == VG_PAINT_TYPE_LINEAR_GRADIENT)
            mod = paint_info->paint_liner_gradt.spread_mod;
        else if(paint_info->paint_type == VG_PAINT_TYPE_RADIAL_GRADIENT)
            mod = paint_info->paint_radial_gradt.spread_mod;
        else
            mod = paint_info->paint_ellipse_gradt.spread_mod;
        switch(mod)
        {
            case VG_COLOR_RAMP_SPREAD_PAD:
                p_paint->spread_mod = GPE_SPN_SPREAD_PAD;
                break;
            case VG_COLOR_RAMP_SPREAD_REPEAT:
                p_paint->spread_mod = GPE_SPN_SPREAD_REPEAT;
                break;
            case VG_COLOR_RAMP_SPREAD_REFLECT:
                p_paint->spread_mod = GPE_SPN_SPREAD_REFLECT;
                break;
            default:
                ret = FALSE;
                break;
        }
    }
    if((paint_info->paint_type == VG_PAINT_TYPE_LINEAR_GRADIENT)  ||
      (paint_info->paint_type == VG_PAINT_TYPE_RADIAL_GRADIENT) ||
      (paint_info->paint_type == VG_PAINT_TYPE_ELLIPSE_GRADIENT))
    {
        if(paint_info->paint_type == VG_PAINT_TYPE_LINEAR_GRADIENT)
            mask_mod = paint_info->paint_liner_gradt.mask_mod;
        else if(paint_info->paint_type == VG_PAINT_TYPE_RADIAL_GRADIENT)
            mask_mod = paint_info->paint_radial_gradt.mask_mod;
        else
            mask_mod = paint_info->paint_ellipse_gradt.mask_mod;
        switch(mask_mod)
        {
            case VG_GRADIENT_MASK_0:
                p_paint->mask_mod = GPE_SPN_MASK_0;
                break;
            case VG_GRADIENT_MASK_1:
                p_paint->mask_mod = GPE_SPN_MASK_1;
                break;
            case VG_GRADIENT_MASK_2:
                p_paint->mask_mod = GPE_SPN_MASK_2;
                break;
            default:
                ret = FALSE;
                break;
        }
    }
    if(paint_info->paint_type == VG_PAINT_TYPE_PATTERN)
    {
        switch(paint_info->paint_pattern.tiling_mod)
        {
            case VG_TILE_FILL:
                p_paint->tiling_mod = GPE_SPN_TILE_FILL;
                break;
            case VG_TILE_PAD:
                p_paint->tiling_mod = GPE_SPN_TILE_PAD;
                break;
            case VG_TILE_REPEAT:
                p_paint->tiling_mod = GPE_SPN_TILE_REPEAT;
                break;
            case VG_TILE_REFLECT:
                p_paint->tiling_mod = GPE_SPN_TILE_REFLECT;
                break;
            default:
                ret = FALSE;
                break;
        }
    }
  return ret;
}
#endif

static mt_s64 gpe_div(mt_s64 dividend, mt_s64 divisor)
{
    mt_u32 sign_0 = 0;
    mt_u32 sign_1 = 0;
    mt_u64 dividend_tmp;

    if (dividend < 0)
    {
        sign_0 = 1;
        dividend = 0 - dividend;
    }

    if (divisor < 0)
    {
        sign_1 = 1;
        divisor = 0 - divisor;
    }

    dividend_tmp = dividend;
    do_div(dividend_tmp, divisor);
    dividend = dividend_tmp;

    if (sign_0 != sign_1)
        return (0 - dividend);
    else
        return dividend;
}

mt_void tde_hal_get_scale_coeff(rect_vsb_t *p_src,
                                pos_t *p_dst00,
                                pos_t *p_dst10,
                                pos_t *p_dst01,
                                pos_t *p_dst11,
                                mt_s32 *p_coeff,
                                scale_type_t *scale_type)
{
    mt_s64 x1, x2, x3, x4, y1, y2, y3, y4, u1, u2, u3, u4, v1, v2, v3, v4;
    mt_s32 tmp;
    mt_u32 para_id = 0;
    mt_s64 para11[3];
    mt_s64 para21[3];
    mt_s64 para22[3];
    mt_s64 para23[3];
    mt_s64 para31[3];

    mt_s64 m1 = 0;
    //mt_s64 m2 = 0;
    mt_s64 d1 = 0;
    //mt_s64 d2 = 0;

    if ((p_dst00->y == p_dst10->y) && (p_dst01->y == p_dst11->y))
    {
        tmp = (p_dst00->x > p_dst01->x) ? p_dst01->x : p_dst00->x;
        u1 = 0;
        v1 = 0;
        u2 = p_src->w;
        v2 = 0;
        u3 = 0;
        v3 = p_src->h;
        u4 = p_src->w;
        v4 = p_src->h;

        x1 = p_dst00->x - tmp;
        y1 = 0;
        x2 = p_dst10->x - tmp;
        y2 = 0;
        x3 = p_dst01->x - tmp;
        y3 = p_dst01->y - p_dst00->y;
        x4 = p_dst11->x - tmp;
        y4 = p_dst11->y - p_dst00->y;

        if ((p_dst00->x == p_dst01->x) && (p_dst10->x == p_dst11->x))
        {
            if (v3 - v1 == y3 - y1) {
                *scale_type = SCALE_ONLY_HORI_RECT;
            } else if (u2 - u1 == x2 - x1) {
                *scale_type = SCALE_ONLY_VERT_RECT;
            } else {
                *scale_type = SCALE_RECT;
            }
                para_id = 0;
                p_coeff[5] = 0;
        }
        else
        {
            if (v3 - v1 == y3 - y1) {
                *scale_type = SCALE_ONLY_HORI_TRAPZ;
            } else {
                *scale_type = SCALE_TRAPZ;
            }
                para_id = 1;
                p_coeff[5] = 1;
        }
    }
    else if ((p_dst00->x == p_dst01->x) && (p_dst10->x == p_dst11->x))
    {
        tmp = (p_dst01->y > p_dst11->y) ? p_dst01->y : p_dst11->y;
        u1 = 0;
        v1 = 0;
        u2 = p_src->h;
        v2 = 0;
        u3 = 0;
        v3 = p_src->w;
        u4 = p_src->h;
        v4 = p_src->w;

        x3 = tmp - p_dst11->y;
        y1 = 0;
        x4 = tmp - p_dst10->y;
        y2 = 0;
        x1 = tmp - p_dst01->y;
        y3 = p_dst10->x - p_dst00->x;
        x2 = tmp - p_dst00->y;
        y4 = p_dst10->x - p_dst00->x;
        *scale_type = SCALE_TRANS_TRAPZ;
        para_id = 2;
        p_coeff[5] = 1;
    }
    else
    {
        TDE_LOG("\r\n scale type not unkwon!");
        return;
    }

    //para11[0] = ((u2<<PARA_FRA)+(x2-x1) / 2) / (x2-x1);
    {
        m1 = ((u2 << PARA_FRA) + (x2 - x1) / 2);
        d1 = (x2 - x1);
        para11[0] = gpe_div(m1, d1);
    }

    para11[1] = para11[0];
    para11[2] = para11[0];

    if ((x3 - x4 + x2 - x1) == 0)
    {
        //para21[0] = (((u2*(x3-x1))<<SOR_FRA)+(y3*(x1-x2))/2)/(y3*(x1-x2));
        m1 = (((u2 * (x3 - x1)) << SOR_FRA) + (y3 * (x1 - x2)) / 2);
        d1 = (y3 * (x1 - x2));
        para21[0] = gpe_div(m1, d1);
    }
    else
    {
        //para21[0] = (((u2*(x3-x1))<<PARA_FRA_22)+(x3-x4+x2-x1)/2)/(x3-x4+x2-x1);
        m1 = (((u2 * (x3 - x1)) << PARA_FRA_22) + (x3 - x4 + x2 - x1) / 2);
        d1 = (x3 - x4 + x2 - x1);
        para21[0] = gpe_div(m1, d1);
    }

    para21[1] = para21[0];
    para21[2] = para21[0];
    if ((x3 - x4 + x2 - x1) == 0)
    {
        //para22[0] = ((v3<<SOR_FRA))/y3;
        m1 = ((v3 << SOR_FRA));
        d1 = y3;
        para22[0] = gpe_div(m1, d1);

        para22[1] = para22[0];
        para22[2] = para22[0];
    }
    else
    {
        //para22[0] = (((v3*(x3-x4))<<PARA_FRA_22)+(x3-x4+x2-x1)/2)/(x3-x4+x2-x1);
        m1 = (((v3 * (x3 - x4)) << PARA_FRA_22) + (x3 - x4 + x2 - x1) / 2);
        d1 = (x3 - x4 + x2 - x1);
        para22[0] = gpe_div(m1, d1);

        para22[1] = para22[0];
        para22[2] = para22[0];
    }

    //para31[0] = (((u2*x1)<<PARA_FRA)+(x1-x2)/2)/(x1-x2);
    m1 = (((u2 * x1) << PARA_FRA) + (x1 - x2) / 2);
    d1 = (x1 - x2);
    para31[0] = gpe_div(m1, d1);

    para31[1] = para31[0];
    para31[2] = para31[0];
    if ((x3 - x4 + x2 - x1) == 0)
    {
        para23[0] = 0;
    }
    else
    {
        //para23[0] = (((x3-x4+x2-x1)<<PARA_FRA_23_33)+(y3*(x1-x2))/2)/(y3*(x1-x2));
        m1 = (((x3 - x4 + x2 - x1) << PARA_FRA_23_33) + (y3 * (x1 - x2)) / 2);
        d1 = (y3 * (x1 - x2));
        para23[0] = gpe_div(m1, d1);
    }

    para23[1] = para23[0];
    para23[2] = para23[0];

    p_coeff[0] = (s32)para11[para_id];
    p_coeff[1] = (s32)para21[para_id];
    p_coeff[2] = (s32)para31[para_id];
    p_coeff[3] = (s32)para22[para_id];
    p_coeff[4] = (s32)para23[para_id];

    return;
}

MT_BOOL tde_hal_get_MB_info(TDE_DRV_SURFACE_S *p_MBSurface, MB_INFO_TYPE_E mbType, ARIA_MB_INFO_S *p_info)
{
    MT_BOOL ret = MT_FALSE;
    mt_u32 bpp = 0;
    mt_u32 fmt = 0;

    TDE_DRV_COLOR_FMT_E sur_fmt = 0;

    MT_ASSERT(p_MBSurface != NULL);
    sur_fmt = p_MBSurface->enColorFmt;

    switch (sur_fmt)
    {
        case TDE_DRV_COLOR_FMT_YCbCr400MBP:
            MT_ASSERT(0);
            break;
        case TDE_DRV_COLOR_FMT_YCbCr422MBH:
            fmt = SP_YUV422_Y;
            if (mbType == MT_TYPE_CbCr)
            {
                fmt = SP_YUV422_C;
            }
            bpp = 0x3;
            ret = MT_TRUE;
            break;
        case TDE_DRV_COLOR_FMT_YCbCr422MBV:
            fmt = SP_YUV422_Y;
            if (mbType == MT_TYPE_CbCr)
            {
                fmt = SP_YUV422_C;
            }
            bpp = 0x3;
            ret = MT_TRUE;
            break;
        case TDE_DRV_COLOR_FMT_YCbCr420MB:
            fmt = SP_YUV420_Y;
            if (mbType == MT_TYPE_CbCr) {
                fmt = SP_YUV420_C;
            }
            bpp = 0x3;
            ret = MT_TRUE;
            break;
        case TDE_DRV_COLOR_FMT_YCbCr444MB:
            fmt = SP_YUV444_Y;
            if (mbType == MT_TYPE_CbCr) {
                fmt = SP_YUV444_C;
            }
            bpp = 0x4;
            ret = MT_TRUE;
            break;
        default:
            MT_ASSERT(0);
    }

    if (ret)
    {
        p_info->bpp = bpp;
        p_info->fmt = fmt;
        p_info->exp_mod = 0x0;

        if (mbType == MB_TYPE_Y)
        {
            p_info->pic_addr = p_MBSurface->u32PhyAddr;
            p_info->pic_stride = p_MBSurface->u32Pitch;
            p_info->mb_type = MB_TYPE_Y;
        }
        else if (mbType == MT_TYPE_CbCr)
        {
            //p_info->pic_addr = p_MBSurface->u32CbCrPhyAddr;
            //p_info->pic_stride = p_MBSurface->u32CbCrPitch;
            p_info->pic_addr = p_MBSurface->u32PhyAddr;
            p_info->pic_stride = p_MBSurface->u32Pitch;
            p_info->mb_type = MT_TYPE_CbCr;
        }
        else
        {
            MT_ASSERT(0);
        }
    }

    return ret;
}

rop_mod_t tde_hal_convert_tde_rop_id_to_aria_rop_mod(TDE2_ROP_CODE_E rop)
{
    rop_mod_t rop_mod = 0;
    switch (rop)
    {
        case TDE2_ROP_BLACK:
            rop_mod = ROP_BLACK;
            break;
        case TDE2_ROP_NOTMERGEPEN:
            rop_mod = ROP_NOTMERGEPEN;
            break;
        case TDE2_ROP_MASKNOTPEN:
            rop_mod = ROP_MASKNOTPEN;
            break;
        case TDE2_ROP_NOTCOPYPEN:
            rop_mod = ROP_NOTCOPYPEN;
            break;
        case TDE2_ROP_MASKPENNOT:
            rop_mod = ROP_MASKPENNOT;
            break;
        case TDE2_ROP_NOT:
            rop_mod = ROP_NOT;
            break;
        case TDE2_ROP_XORPEN:
            rop_mod = ROP_XORPEN;
            break;
        case TDE2_ROP_NOTMASKPEN:
            rop_mod = ROP_NOTMASKPEN;
            break;
        case TDE2_ROP_MASKPEN:
            rop_mod = ROP_MASKPEN;
            break;
        case TDE2_ROP_NOTXORPEN:
            rop_mod = ROP_NOTXORPEN;
            break;
        case TDE2_ROP_NOP:
            rop_mod = ROP_NOP;
            break;
        case TDE2_ROP_MERGENOTPEN:
            rop_mod = ROP_MERGENOTPEN;
            break;
        case TDE2_ROP_COPYPEN:
            rop_mod = ROP_COPYPEN;
            break;
        case TDE2_ROP_MERGEPENNOT:
            rop_mod = ROP_MERGEPENNOT;
            break;
        case TDE2_ROP_MERGEPEN:
            rop_mod = ROP_MERGEPEN;
            break;
        case TDE2_ROP_WHITE:
            rop_mod = ROP_WHITE;
            break;
        case TDE2_ROP_PATINVERT:
            rop_mod = ROP_PATINVERT;
            break;
        case TDE2_ROP_MERGEPAINT:
            rop_mod = ROP_MERGEPAINT;
            break;
        case TDE2_ROP_PATCOPY:
            rop_mod = ROP_PATCOPY;
            break;
        case TDE2_ROP_PATPAINT:
            rop_mod = ROP_PATPAINT;
            break;
        default:
            MT_ASSERT(0);
            break;
    }

    return rop_mod;
}

bld_fact_t tde_hal_convert_bld_mod_to_fact(TDE2_BLEND_MODE_E bld_mod)
{
    bld_fact_t fact = GL_ZERO;
    switch (bld_mod)
    {
        case TDE2_BLEND_ZERO:
            fact = GL_ZERO;
            break;
        case TDE2_BLEND_ONE:
            fact = GL_ONE;
            break;
        case TDE2_BLEND_SRC2COLOR:
            fact = GL_SRC_COLOR;
            break;
        case TDE2_BLEND_INVSRC2COLOR:
            fact = GL_ONE_MINUS_SRC_COLOR;
            break;
        case TDE2_BLEND_SRC2ALPHA:
            fact = GL_SRC_ALPHA;
            break;
        case TDE2_BLEND_INVSRC2ALPHA:
            fact = GL_ONE_MINUS_SRC_ALPHA;
            break;
        case TDE2_BLEND_SRC1COLOR:
            fact = GL_DST_COLOR;
            break;
        case TDE2_BLEND_INVSRC1COLOR:
            fact = GL_ONE_MINUS_DST_COLOR;
            break;
        case TDE2_BLEND_SRC1ALPHA:
            fact = GL_DST_ALPHA;
            break;
        case TDE2_BLEND_INVSRC1ALPHA:
            fact = GL_ONE_MINUS_DST_ALPHA;
            break;
        case TDE2_BLEND_SRC2ALPHASAT:
            fact = GL_SRC_ALPHA_SATURATE;
            break;
        default:
            MT_ASSERT(0);
            break;
    }

    return fact;
}

MT_BOOL tde_hal_converto_surface_to_pix_fmt(TDE_DRV_SURFACE_S *p_surface, pix_fmt_t *p_fmt)
{
    pix_fmt_t fmt = PIX_FMT_MAX;

    MT_ASSERT(p_surface != NULL);

    switch (p_surface->enColorFmt)
    {
        case TDE_DRV_COLOR_FMT_RGB233:
            fmt = PIX_FMT_RGB233;
            break;
        case TDE_DRV_COLOR_FMT_RGB444:
            //fmt = ;
            break;
        case TDE_DRV_COLOR_FMT_RGB555:
            //fmt = ;
            break;
        case TDE_DRV_COLOR_FMT_RGB565:
            fmt = PIX_FMT_RGB565;
            break;
        case TDE_DRV_COLOR_FMT_RGB888:
            if (p_surface->enRgbOrder == TDE_DRV_ORDER_ARGB)
            {
                fmt = PIX_FMT_RGB888;
            }
            else if (p_surface->enRgbOrder == TDE_DRV_ORDER_ABGR)
            {
            fmt = PIX_FMT_BGR888;
            }
            break;
        case TDE_DRV_COLOR_FMT_ARGB4444:
            if (p_surface->enRgbOrder == TDE_DRV_ORDER_ARGB)
            {
                fmt = PIX_FMT_ARGB4444;
            }
            else if (p_surface->enRgbOrder == TDE_DRV_ORDER_RGBA)
            {
                fmt = PIX_FMT_RGBA4444;
            }
            break;
        case TDE_DRV_COLOR_FMT_ARGB1555:
            if (p_surface->enRgbOrder == TDE_DRV_ORDER_ARGB)
            {
                fmt = PIX_FMT_ARGB1555;
            }
            else if (p_surface->enRgbOrder == TDE_DRV_ORDER_RGBA)
            {
                fmt = PIX_FMT_RGBA5551;
            }
            break;
        case TDE_DRV_COLOR_FMT_ARGB8565:
            //fmt = ;
            break;
        case TDE_DRV_COLOR_FMT_ARGB8888:
            if (p_surface->enRgbOrder == TDE_DRV_ORDER_ARGB)
            {
                fmt = PIX_FMT_ARGB8888;
            }
            else if (p_surface->enRgbOrder == TDE_DRV_ORDER_RGBA)
            {
                fmt = PIX_FMT_RGBA8888;
            }
            break;
        case TDE_DRV_COLOR_FMT_CLUT1:
            fmt = PIX_FMT_RGBPALETTE1;
            break;
        case TDE_DRV_COLOR_FMT_CLUT2:
            fmt = PIX_FMT_RGBPALETTE2;
            break;
        case TDE_DRV_COLOR_FMT_CLUT4:
            fmt = PIX_FMT_RGBPALETTE4;
            break;
        case TDE_DRV_COLOR_FMT_CLUT8:
            fmt = PIX_FMT_RGBPALETTE8;
            break;
        case TDE_DRV_COLOR_FMT_ACLUT44:
            fmt = PIX_FMT_ARGBPALETTE44;
            break;
        case TDE_DRV_COLOR_FMT_ACLUT88:
            fmt = PIX_FMT_ARGBPALETTE88;
            break;
        case TDE_DRV_COLOR_FMT_A1:
            //fmt = ;
            break;
        case TDE_DRV_COLOR_FMT_A8:
            fmt = PIX_FMT_GRAY_8;
            break;
        case TDE_DRV_COLOR_FMT_YCbCr888:
            // fmt = ;
            break;
        case TDE_DRV_COLOR_FMT_AYCbCr8888:
            fmt = PIX_FMT_AYCBCR8888;
            break;
        case TDE_DRV_COLOR_FMT_YCbCr422:
            fmt = PIX_FMT_Y1CRY0CB8888;
            break;
        case TDE_DRV_COLOR_FMT_byte:
            //fmt = ;
            break;
        case TDE_DRV_COLOR_FMT_halfword:
            //fmt = ;
            break;
        case TDE_DRV_COLOR_FMT_YCbCr400MBP:
            //fmt = ;
            break;
        case TDE_DRV_COLOR_FMT_YCbCr422MBH:
            fmt = PIX_FMT_SP_YUV422_1x2;
            break;
        case TDE_DRV_COLOR_FMT_YCbCr422MBV:
            fmt = PIX_FMT_SP_YUV422_2x1;
            break;
        case TDE_DRV_COLOR_FMT_YCbCr420MB:
            fmt = PIX_FMT_SP_YUV420;
            break;
        case TDE_DRV_COLOR_FMT_YCbCr444MB:
            fmt = PIX_FMT_SP_YUV444;
            break;
        case TDE_DRV_COLOR_FMT_RABG8888:
            //fmt = ;
            break;
        default:
            break;
    }

    if (fmt == PIX_FMT_MAX)
    {
        return MT_FALSE;
    }
    else
    {
        *p_fmt = fmt;
        return MT_TRUE;
    }
}

mt_u8 tde_hal_get_img_swap(gpe_img_t *p_img)
{
    mt_u8 swap_mod = 0;
    if (p_img->color_info.little_endian)
    {
        if (p_img->color_info.color_fmt == Y1VY0U)
            swap_mod = 1;
        else if (p_img->color_info.color_fmt == SP_YUV444_Y)
            swap_mod = 0;
        else
        {
            if (p_img->color_info.bpp == GPE_SPN_BPP_32BIT)
                swap_mod = 1;
            else if (p_img->color_info.bpp == GPE_SPN_BPP_16BIT)
                swap_mod = 2;
            else
                swap_mod = 0;
        }
    }
    else
    {
        swap_mod = 0;
    }

    if(p_img->color_info.color_fmt == CLUT_1)
        swap_mod |= (3 << 4);
    else if(p_img->color_info.color_fmt == CLUT_2)
        swap_mod |= (2 << 4);
    else if(p_img->color_info.color_fmt == CLUT_4)
        swap_mod |= (1 << 4);

    return swap_mod;
}

MT_BOOL tde_hal_convert_surface_to_img(TDE_DRV_SURFACE_S *p_surface, gpe_img_t *p_img, src_ch_t ch)
{
    MT_BOOL is_pix_alpha = MT_FALSE;
    MT_BOOL is_support = MT_FALSE;
    MT_BOOL with_palette = MT_FALSE;
    MT_BOOL is_xylc = MT_FALSE;
    MT_BOOL is_tile = MT_FALSE;
    MT_BOOL is_sp = MT_FALSE;
    u32 pix_little_endian = 0;
    u32 palt_little_endian = 0;
    u32 bpp = 0;
    color_space_t color_space = RGB_COLOR_SPACE;
    palette_format_t palt_format = GPE_SPN_PALT_ARGB8888;
    color_format_t color_fmt = CLUT_1;
    u32 pix_fmt = PIX_FMT_RGBPALETTE1;

    TDE_FUN_IN;
    if (!tde_hal_converto_surface_to_pix_fmt(p_surface, &pix_fmt))
    {
        TDE_FUN_OUT;
        return MT_FALSE;
    }

    TDE_LOG("PARAM: [%x][%x][%x]\n", p_surface->enColorFmt, pix_fmt, ch);

    /*
  if(ch == SPN_SRC1)
    pix_fmt = param->src_img.pix_format;
  else if(ch == SPN_DST)
    pix_fmt = param->dst_img.pix_format;
  else if(ch == SPN_SRC3)
    pix_fmt = param->ex_img.pix_format;
  else  if(ch == SPN_SRC2)
    pix_fmt = param->bg_img.pix_format;
*/

    switch (pix_fmt)
    {
    case PIX_FMT_TILE:
        is_tile = MT_TRUE;
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = TILE_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        if (ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XY:
        is_xylc = MT_TRUE;
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = XY;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if (ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XYL:
        is_xylc = MT_TRUE;
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = XYL;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if (ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XYC:
        is_xylc = MT_TRUE;
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = XYC;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if (ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XYLC:
        is_xylc = MT_TRUE;
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = XYLC;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if (ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XY_SMALL:
        is_xylc = MT_TRUE;
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = XY;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        if (ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XYL_SMALL:
        is_xylc = MT_TRUE;
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = XYL;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        if (ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XYC_SMALL:
        is_xylc = MT_TRUE;
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = XYC;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        if (ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XYLC_SMALL:
        is_xylc = MT_TRUE;
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = XYLC;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        if (ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_RGBPALETTE1_PALETTE_BGRA:
        bpp = GPE_SPN_BPP_1BIT;
        color_fmt = CLUT_1;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_GRAY_8:
        is_xylc = MT_FALSE;
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = GRAY_8;
        color_space = GRAY_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBPALETTE1:
        bpp = GPE_SPN_BPP_1BIT;
        color_fmt = CLUT_1;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE2:
        bpp = GPE_SPN_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE4:
        bpp = GPE_SPN_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE8:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVPALETTE1:
        bpp = GPE_SPN_BPP_1BIT;
        color_fmt = CLUT_1;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_AYUV8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE2:
        bpp = GPE_SPN_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_AYUV8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE4:
        bpp = GPE_SPN_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_AYUV8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE8:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE44:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE88:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE44:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE88:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGB565:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = RGB565;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGB1555:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ARGB1555;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBA5551:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = RGBA5551;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGB4444:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ARGB4444;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBA4444:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = RGBA4444;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGB8888:
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = ARGB8888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBA8888:
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = RGBA8888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_Y1CRY0CB8888:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = Y1VY0U; //UY0VY1;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_CBY0CRY18888:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = Y1VY0U; //UY0VY1;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYCBCR8888:
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = AYUV8888;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_CRCBYA8888:
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = AYUV8888;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YCBCRA8888:
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = YUVA8888;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ACRCBY8888:
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = YUVA8888;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBPALETTE2_PALETTE_BGRA:
        bpp = GPE_SPN_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE4_PALETTE_BGRA:
        bpp = GPE_SPN_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE8_PALETTE_BGRA:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVPALETTE2_PALETTE_VUYA:
        bpp = GPE_SPN_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_AYUV8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE4_PALETTE_VUYA:
        bpp = GPE_SPN_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_AYUV8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE8_PALETTE_VUYA:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE44_PALETTE_BGRA:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE88_PALETTE_BGRA:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBAPALETTE88:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBAPALETTE88_PALETTE_BGRA:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE44_PALETTE_VUYA:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE88_PALETTE_VUYA:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVAPALETTE88:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVAPALETTE88_PALETTE_VUYA:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGB565_SMALL_ENDIAN:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = RGB565;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGB1555_SMALL_ENDIAN:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ARGB1555;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGB4444_SMALL_ENDIAN:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ARGB4444;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGB8888_SMALL_ENDIAN:
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = ARGB8888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBA8888_SMALL_ENDIAN:
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = RGBA8888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBA5551_SMALL_ENDIAN:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = RGBA5551;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBA4444_SMALL_ENDIAN:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = RGBA4444;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBPALETTE2_PALETTE_RGBA:
        bpp = GPE_SPN_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_RGBA8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE2_PALETTE_ABGR:
        bpp = GPE_SPN_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_RGBA8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE4_PALETTE_RGBA:
        bpp = GPE_SPN_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_RGBA8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE4_PALETTE_ABGR:
        bpp = GPE_SPN_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_RGBA8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE8_PALETTE_RGBA:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBPALETTE8_PALETTE_ABGR:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVPALETTE2_PALETTE_YUVA:
        bpp = GPE_SPN_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_YUVA8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE2_PALETTE_AVUY:
        bpp = GPE_SPN_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_YUVA8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE4_PALETTE_YUVA:
        bpp = GPE_SPN_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_YUVA8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE4_PALETTE_AVUY:
        bpp = GPE_SPN_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_YUVA8888;
        is_support = MT_TRUE;
        if (ch == SPN_DST) {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE8_PALETTE_YUVA:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVPALETTE8_PALETTE_AVUY:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE44_PALETTE_RGBA:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE44_PALETTE_ABGR:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE88_PALETTE_RGBA:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE88_PALETTE_ABGR:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
     case PIX_FMT_RGBAPALETTE88_PALETTE_RGBA:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBAPALETTE88_PALETTE_ABGR:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE44_PALETTE_YUVA:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE44_PALETTE_AVUY:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE88_PALETTE_YUVA:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE88_PALETTE_AVUY:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVAPALETTE88_PALETTE_YUVA:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 0;
        palt_format = GPE_SPN_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVAPALETTE88_PALETTE_AVUY:
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 1;
        palt_format = GPE_SPN_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGB233:
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = RGB233;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGB888:
        bpp = GPE_SPN_BPP_24BIT;
        color_fmt = RGB888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if (ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_BGR888:
        bpp = GPE_SPN_BPP_24BIT;
        color_fmt = BGR888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if (ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_GRAY_16:
        is_xylc = MT_FALSE;
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = GRAY_16;
        color_space = GRAY_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if (ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_SP_YUV444:
        is_tile = MT_FALSE;
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = SP_YUV444_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV422:
    case PIX_FMT_SP_YUV422_1x2:
        is_tile = MT_FALSE;
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = SP_YUV422_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV422_2x1:
        is_tile = MT_FALSE;
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = SP_YUV422_Y2;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV420:
        is_tile = MT_FALSE;
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = SP_YUV420_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV444_UVSWAP:
        is_tile = MT_FALSE;
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = SP_YUV444_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV422_UVSWAP:
    case PIX_FMT_SP_YUV422_1x2_UVSWAP:
        is_tile = MT_FALSE;
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = SP_YUV422_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV422_2x1_UVSWAP:
        is_tile = MT_FALSE;
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = SP_YUV422_Y2;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV420_UVSWAP:
        is_tile = MT_FALSE;
        bpp = GPE_SPN_BPP_8BIT;
        color_fmt = SP_YUV420_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_CMYK:
        is_tile = MT_FALSE;
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = CMYK8888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_sp = MT_FALSE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_KYMC:
        is_tile = MT_FALSE;
        bpp = GPE_SPN_BPP_32BIT;
        color_fmt = CMYK8888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_sp = MT_FALSE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_CMYK:
        is_tile = MT_FALSE;
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = SP_CMYK8888_CM;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_CMYK_SWAP:
        is_tile = MT_FALSE;
        bpp = GPE_SPN_BPP_16BIT;
        color_fmt = SP_CMYK8888_CM;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    //RGBA5551,RGBA4444,alut88 small endian
    //gray-8,gray-16, tile, xylc
    case PIX_FMT_ARGBPALETTE11:
    case PIX_FMT_ARGBPALETTE22:
    case PIX_FMT_AYUVPALETTE11:
    case PIX_FMT_AYUVPALETTE22:
    case PIX_FMT_Y0CBY1CR8888:
    case PIX_FMT_Y0CRY1CB8888:
    case PIX_FMT_Y1CBY0CR8888:
    case PIX_FMT_CBY1CRY08888:
    case PIX_FMT_CRY1CBY08888:
    case PIX_FMT_CRY0CBY18888:
    case PIX_FMT_X2C10Y10CB10:
    case PIX_FMT_YCBCR444:
    case PIX_FMT_YCBCR422:
    case PIX_FMT_YCBCR420:
        is_support = MT_FALSE;
        break;
    default:
        is_support = MT_FALSE;
        break;
    }

    if (is_support)
    {
#if 0
        if(ch == SPN_SRC1)
        {
            p_ctx->src_img.color_info.bpp = bpp;
            p_ctx->src_img.color_info.color_fmt = color_fmt;
            p_ctx->src_img.color_info.color_space = color_space;
            p_ctx->src_img.color_info.is_pix_alpha = is_pix_alpha;
            p_ctx->src_img.color_info.little_endian = pix_little_endian;
            p_ctx->src_img.with_palette = with_palette;
            p_ctx->src_img.palt_format = palt_format;
            p_ctx->src_img.palt_little_endian = palt_little_endian;

            p_ctx->src_img.palt_size = param->src_img.palette_size;
            p_ctx->src_img.palt_buf = param->src_img.palette_base;
            if(MT_TRUE == is_xylc)
            {
                p_ctx->src_is_xylc = MT_TRUE;
                p_ctx->xylc_cfg.xylc_num = param->src_img.xylc_num;
                p_ctx->xylc_cfg.xylc_color = param->src_img.xylc_color;
            }
            else
            {
                p_ctx->src_is_xylc = MT_FALSE;
            }
            if(MT_TRUE == is_tile)
            {
                p_ctx->src_is_tile = MT_TRUE;
            }
            if((MT_TRUE == is_tile) || (MT_TRUE == is_sp))
            {
                p_ctx->src0_buf = param->src_img.chroma_addr;
                p_ctx->src0_pitch = param->src_img.chroma_pitch;
            }
        }
        else if(ch == SPN_DST)
        {
            p_ctx->dst_img.color_info.bpp = bpp;
            p_ctx->dst_img.color_info.color_fmt = color_fmt;
            p_ctx->dst_img.color_info.color_space = color_space;
            p_ctx->dst_img.color_info.is_pix_alpha = is_pix_alpha;
            p_ctx->dst_img.color_info.little_endian = pix_little_endian;
            p_ctx->dst_img.with_palette = with_palette;
            p_ctx->dst_img.palt_format = palt_format;
            p_ctx->dst_img.palt_little_endian = palt_little_endian;
        }
        else if(ch == SPN_SRC3)
        {
            p_ctx->ex_img.color_info.bpp = bpp;
            p_ctx->ex_img.color_info.color_fmt = color_fmt;
            p_ctx->ex_img.color_info.color_space = color_space;
            p_ctx->ex_img.color_info.is_pix_alpha = is_pix_alpha;
            p_ctx->ex_img.color_info.little_endian = pix_little_endian;
            p_ctx->ex_img.with_palette = with_palette;
            p_ctx->ex_img.palt_format = palt_format;
            p_ctx->ex_img.palt_little_endian = palt_little_endian;
            p_ctx->ex_img.palt_size = param->ex_img.palette_size;
            p_ctx->ex_img.palt_buf = param->ex_img.palette_base;
        }
        else if(ch == SPN_SRC2)
        {
            p_ctx->bg_img.color_info.bpp = bpp;
            p_ctx->bg_img.color_info.color_fmt = color_fmt;
            p_ctx->bg_img.color_info.color_space = color_space;
            p_ctx->bg_img.color_info.is_pix_alpha = is_pix_alpha;
            p_ctx->bg_img.color_info.little_endian = pix_little_endian;
        }
#endif

        p_img->color_info.bpp = bpp;
        p_img->color_info.color_fmt = color_fmt;
        p_img->color_info.color_space = color_space;
        p_img->color_info.is_pix_alpha = is_pix_alpha;
        p_img->color_info.little_endian = pix_little_endian;
        p_img->with_palette = with_palette;
        p_img->palt_format = palt_format;
        p_img->palt_little_endian = palt_little_endian;
    }

    TDE_FUN_OUT;

    return is_support;
}

extern mt_u32 TdeHalGetRegVriBaseAddr(mt_void);

#define TDE_REG_READ2(reg_addr) (*((volatile unsigned int *)((ulong)reg_addr)))

mt_void TdeHalAriaPrintAllReg(mt_void)
{
    mt_u32 base = 0;
    mt_u32 reg_offset = 0;
    mt_u32 reg_data = 0;
    mt_u32 reg_addr = 0;

    base = TdeHalGetRegVriBaseAddr();

    MT_INFO_TDE("Tde reg info: \n");
    for (reg_offset = 0; reg_offset <= 0x03FC; reg_offset += 4)
    {
        reg_addr = base + reg_offset;
        reg_data = TDE_REG_READ2(reg_addr);

        MT_INFO_TDE("%4x: [%x]\n", reg_offset, reg_data);
    }

    return;
}
