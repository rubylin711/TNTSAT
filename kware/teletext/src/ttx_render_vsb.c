/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include "string.h"
#include "mt_type.h"
#include "sys_define.h"
//#include "drv_dev.h"
//#include "hal_dma.h"
//#include "hal_misc.h"

//#include "mtos_printk.h"
//#include "mtos_sem.h"
//#include "mtos_mem.h"
//#include "mtos_msg.h"

//#include "lib_rect.h"

//#include "common.h"
//#include "region.h"
//#include "display.h"
//#include "gpe_vsb.h"

//#include "drv_misc.h"
//#include "mdl.h"
#include "vbi_api.h"
#include "ttx_lang_vsb.h"
#include "ttx_dec_vsb.h"
#include "ttx_format_vsb.h"
#include "mt_unf_ttx.h"

#include "ttx_render_vsb.h"
#include <sys/time.h>

extern u32 g_inital_page;

static void ttx_render_draw_page_no_vsb_page_mode(ttx_render_vsb_t *p_render, u16 page_no, u16 sub_no, MT_BOOL b_dec);
static void ttx_render_draw_page_vsb_page_mode(ttx_render_vsb_t *p_render, MT_BOOL subtile
                              , ttx_osd_page_t *p_osd_page
                              , u8 first_row, u8 last_row
                              , u8 first_column, u8 last_column
                              , MT_BOOL conceal, MT_BOOL flash_on);
static void draw_blank_vsb_page_mode(ttx_render_vsb_t *p_render
                           , u32 top, u32 left, u8 color, s32 cw, s32 ch,u8 *cache_buf,s32 pitch);
static void draw_char_vsb_page_mode(ttx_render_vsb_t *p_render, u8 *p_pen, u32 top, u32 left
                          , const u8 *p_font, s32 cpl, s32 cw, s32 ch, s32 glyph
                          , s32 bold, u32 underline, ttx_size_t size,u8 *cache_buf,s32 pitch);

/*!
 cp the whole charator to a buffer, then do gpe ops, not every pixel do gpe ops.
  */
#define TTX_RENDER_OPTIMIZE

/*!
  position calc, the unicode mapped to xbm
  example, unicode 0x20 mapped to 0x20-0x20 = 0 position in xbm font
  example, unicode 0x21 mapped to 0x21-0x20 = 1 position in xbm font
  */
static u32 unicode_wstfont2_vsb(u32 c, s32 italic)
{
    static const unsigned short specials[] =
    {
        0x01B5, 0x2016, 0x01CD, 0x01CE, 0x0229, 0x0251, 0x02DD, 0x02C6,
        0x02C7, 0x02C9, 0x02CA, 0x02CB, 0x02CD, 0x02CF, 0x02D8, 0x02D9,
        0x02DA, 0x02DB, 0x02DC, 0x2014, 0x2018, 0x2019, 0x201C, 0x201D,
        0x20A0, 0x2030, 0x20AA, 0x2122, 0x2126, 0x215B, 0x215C, 0x215D,
        0x215E, 0x2190, 0x2191, 0x2192, 0x2193, 0x25A0, 0x266A, 0xE800,
        0xE75F
    };
    const u32   invalid = 357;
    u32         i = 0;

    if(c < 0x0180)
    {
        if(c < 0x0080)
        {
            if(c < 0x0020)
                return invalid;
            /* A workround way to fix bug 10433, because the font in font library 0x5f is
              *  not correct, this method can point to right charactor.
            */
            else if(c == 0x7f)
                c = 357;
            else /* %3 Basic Latin (ASCII) 0x0020 ... 0x007F */
                c = c - 0x0020 + 0 * 32;
        }
        else if(c < 0x00A0)
            return invalid;
        else /* %3 Latin-1 Supplement, Latin Extended-A 0x00A0 ... 0x017F */
            c = c - 0x00A0 + 3 * 32;
    }
    else if(c < 0xEE00)
    {
        if(c < 0x0460)
        {
            if(c < 0x03D0)
            {
                if(c < 0x0370)
                {
                    for(i = 0; i < sizeof(specials) / sizeof(specials[0]); i ++)
                    {
                        if(specials[i] == c)
                        {
                            if(italic != 0)
                                return i + 41 * 32;
                            else
                                return i + 10 * 32;
                        }
                    }
                }
                else /* %5 Greek 0x0370 ... 0x03CF */
                {
                    c = c - 0x0370 + 12 * 32;
                }
            }
            else if(c < 0x0400)
                return invalid;
            else /* %5 Cyrillic 0x0400 ... 0x045F */
                c = c - 0x0400 + 15 * 32;
        }
        else if(c < 0x0620)
        {
            if(c < 0x05F0)
            {
                if(c < 0x05D0)
                    return invalid;
                else /* %6 Hebrew 0x05D0 ... 0x05EF */
                    return c - 0x05D0 + 18 * 32;
            }
            else if(c < 0x0600)
                return invalid;
            else /* %6 Arabic 0x0600 ... 0x061F */
                return c - 0x0600 + 19 * 32;
        }
        else if(c >= 0xE600 && c < 0xE740)
        {
            return c - 0xE600 + 19 * 32; /* %6 Arabic (TTX) */
        }
        else
        {
            for(i = 0; i < sizeof(specials) / sizeof(specials[0]); i ++)
            {
                if(specials[i] == c)
                {
                    if(italic != 0)
                        return i + 41 * 32;
                    else
                        return i + 10 * 32;
                }
            }
        }
    }
    else if(c < 0xEF00)
    {
        /* %3 G1 Graphics */
        return (c ^ 0x20) - 0xEE00 + 23 * 32;
    }
    else if(c < 0xF000)
    {
        /* %4 G3 Graphics */
        return c - 0xEF20 + 27 * 32;
    }
    else /* 0xF000 ... 0xF7FF reserved for DRCS */
        return invalid;

    if(italic != 0)
        return c + 31 * 32;
    else
        return c;

    return invalid;
}

#if TTX_SUPPORT_DRCS
static void draw_drcs_vsb(ttx_render_vsb_t *p_render
                          , u8 *p_pen, u8 color_offset, s32 top, s32 left
                          , u8 *p_font, s32 glyph, ttx_size_t size)
{
    u8  *p_src = NULL;
    u32 col = 0;
    s32 x = 0, y = 0;
    u8  *p_pix_buf = NULL;
    u32 stride = 0;
    s32 char_h = 0;

    if(p_render->font_size == TTX_FONT_HD)
        char_h = TTX_CHAR_H_HD;
    else if(p_render->video_std == VID_STD_PAL)
        char_h = TTX_CHAR_H_PAL;
    else
        char_h = TTX_CHAR_H_NTSC;

    p_src = p_font + glyph * 60;
    p_pen = p_pen + color_offset;

    stride     = p_render->region_w;
    p_pix_buf  = (u8 *)((top & 1)
                        ? p_render->p_even_addr : p_render->p_odd_addr);
    p_pix_buf += left + (top / 2) * p_render->region_w;

    switch(size)
    {
    case TTX_NORMAL_SIZE:
        for(y = 0; y < char_h; top ++, y ++)
        {
            p_pix_buf  = (u8 *)((top & 1)
                                ? p_render->p_even_addr : p_render->p_odd_addr);
            p_pix_buf += left + (top / 2) * stride;

            for(x = 0; x < 12; p_src ++, x += 2)
            {
                p_pix_buf[x + 0] = p_pen[*p_src & 0xF];
                p_pix_buf[x + 1] = p_pen[*p_src >> 4];
            }
        }
        break;

    case TTX_DOUBLE_HEIGHT2:
    case TTX_DOUBLE_HEIGHT:
        if(size == TTX_DOUBLE_HEIGHT2)
          p_src += 30;
        for(y = 0; y < char_h / 2; top += 2, y ++)
        {
            p_pix_buf  = (u8 *)((top & 1)
                                ? p_render->p_even_addr : p_render->p_odd_addr);
            p_pix_buf += left + (top / 2) * stride;

            for(x = 0; x < 12; p_src ++, x += 2)
            {
                col = p_pen[*p_src & 15];
                p_pix_buf[x + 0]          = col;
                p_pix_buf[x + stride + 0] = col;

                col = p_pen[*p_src >> 4];
                p_pix_buf[x + 1]          = col;
                p_pix_buf[x + stride + 1] = col;

            }
        }
        break;

    case TTX_DOUBLE_WIDTH:
        for(y = 0; y < char_h; top ++, y ++)
        {
            p_pix_buf  = (u8 *)((top & 1)
                                ? p_render->p_even_addr : p_render->p_odd_addr);
            p_pix_buf += left + (top / 2) * stride;

            for(x = 0; x < 12 * 2; p_src ++, x += 4)
            {
                col = p_pen[*p_src & 15];
                p_pix_buf[x + 0] = col;
                p_pix_buf[x + 1] = col;

                col = p_pen[*p_src >> 4];
                p_pix_buf[x + 2] = col;
                p_pix_buf[x + 3] = col;
            }
        }
        break;

    case TTX_DOUBLE_SIZE2:
    case TTX_DOUBLE_SIZE:
        if(size == TTX_DOUBLE_SIZE2)
          p_src += 30;
        for(y = 0; y < char_h / 2; top += 2, y ++)
        {
            p_pix_buf  = (u8 *)((top & 1)
                                ? p_render->p_even_addr : p_render->p_odd_addr);
            p_pix_buf += left + (top / 2) * stride;

            for(x = 0; x < 12 * 2; p_src ++, x += 4)
            {
                col = p_pen[*p_src & 15];
                p_pix_buf[x + 0]          = col;
                p_pix_buf[x + 1]          = col;
                p_pix_buf[x + stride + 0] = col;
                p_pix_buf[x + stride + 1] = col;


                col = p_pen[*p_src >> 4];
                p_pix_buf[x + 2]          = col;
                p_pix_buf[x + 3]          = col;
                p_pix_buf[x + stride + 2] = col;
                p_pix_buf[x + stride + 3] = col;
            }
        }
        break;

    default:
        break;
    }
}
#endif
#if 0
static unsigned long long mt_get_time_ms( void )
{
    struct timeval tv;
    unsigned long long time_ms = 0ULL;

    gettimeofday(&tv, NULL);

    time_ms  = (unsigned long long)tv.tv_sec * 1000;
    time_ms += (unsigned long long)tv.tv_usec / 1000;
    return (time_ms);
}

static u64 mt_get_time_interval_ms(u64 after, u64 before)
{
   u64 diff = 0;
	 if (after >= before)
   {
     diff = ((after - before));
	 }
	 else
   {
     diff = ((UINT64_MAX - (before - after) + 1));
   }	
   return diff;
}
#endif

#if 0
static void draw_wstfont(ttx_render_vsb_t *p_render)
{
    u8 *p_src, p_src;
    s32 x, y, z;
    u8  *p_pix_buf;
    u32 stride = p_render->w;
    u32 top = 0;;

    for(y = 0; y < p_render->h; y ++)
    {
        if((y & 1) != 0)
        {
            p_pix_buf = ((u8 *)p_render->p_odd_addr) + (y / 2) * stride;
        }
        else
        {
            p_pix_buf = ((u8 *)p_render->p_even_addr) + (y / 2) * stride;
        }

        p_src = wstfont2_bits + y * wstfont2_width / 8;

        for(x = 0; x < wstfont2_width / 8; x ++)
        {
            p_src = *(p_src + x);
            for(z = 0; z < 8; p_src >>= 1, z ++)
                p_pix_buf[x * 8 + z] = (p_src & 1) ? TTX_BLACK : TTX_WHITE;
        }
    }
}
#endif

/*
 * Draw blank character.
 */
static void draw_blank_vsb(ttx_render_vsb_t *p_render
                           , u32 top, u32 left, u8 color, s32 cw, s32 ch)
{
    //RET_CODE ret = 0;
//    rect_t rect;
	MT_UNF_TTX_FILLRECT_S fillrect;
	MT_UNF_TTX_PAGEAREA_S rect = {0};
	MT_UNF_TTX_REFRESHLAYER_S refresh_rect;

//	printf("\r\n ~~~~~~~~~~%s, %d", __FUNCTION__, __LINE__);

    if(TRUE == p_render->is_sub)
    {
        MT_ASSERT(NULL != p_render->p_sub_hdl);
    }
    else
    {
        MT_ASSERT(NULL != p_render->p_osd_hdl);
    }

	fillrect.u32Color = color;
	rect.u32Row = top;
	rect.u32Column = left;
	rect.u32RowCount = ch;
	rect.u32ColumnCount = cw;
	fillrect.pstPageArea = &rect;		
	p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_FILLRECT, &fillrect);
	refresh_rect.pstPageArea = &rect;
//	if(g_inital_page)
		p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_REFRESH, &refresh_rect);
#if 0	
    rect.left = left;
    rect.top = top;
    rect.right = left + cw;
    rect.bottom = top + ch;
	
    ret = gpe_draw_rectangle_vsb(p_render->p_gpe,
                                 (TRUE != p_render->is_sub) ? p_render->p_osd_hdl : p_render->p_sub_hdl,
                                 &rect,
                                 color);
    MT_ASSERT(SUCCESS == ret);
#endif	
}

/*
 * Draw a character.
  \param[in] cpl characters per line
  \param[in] cw character width
  \param[in] ch character height
  \param[in] glyph character index in font xbm
 */
static void draw_char_vsb(ttx_render_vsb_t *p_render, u8 *p_pen, u32 top, u32 left
                          , const u8 *p_font, s32 cpl, s32 cw, s32 ch, s32 glyph
                          , s32 bold, u32 underline, ttx_size_t size)
{
    //RET_CODE ret = 0;
    const u8  *p_src = NULL;
    s32 shift = 0, x = 0, y = 0, z = 0;
    u32 bits = 0;
    u8  col = 0;
    MT_UNF_TTX_PAGEAREA_S rect;
    //u32 cp_size = 0;
    u32 ttx_char_max_w = 0;
    u8 *p_char_buf = p_render->p_char_buf;

	MT_UNF_TTX_REFRESHLAYER_S refresh_rect;

 //   MT_ASSERT(NULL != p_char_buf);
    if(NULL == p_char_buf)
    {
        OS_PRINTF("draw_char_vsb NULL == p_char_buf\n");
        return;
    }
    bold = !!bold;

    y = (glyph / cpl) * ch;
    x = (glyph % cpl) * cw;
    if(p_render->font_size == TTX_FONT_HD)
    {
        z = WSTFONT2_WIDTH_HD * y + x;
//        ttx_char_max_w = TTX_CHAR_MAX_HD_W;
    }
    else
    {
#if 0    
        if(p_render->video_std == VID_STD_PAL)
            z = WSTFONT2_WIDTH_PAL * y + x;
        else
            z = WSTFONT2_WIDTH_NTSC * y + x;
#else
        z = WSTFONT2_WIDTH_PAL * y + x;
#endif
//        ttx_char_max_w = TTX_CHAR_MAX_W;
    }
	ttx_char_max_w = p_render->char_buf_pitch;
	
    shift = z & 7;
    p_src = p_font + z / 8;

    rect.u32Column = left;
    rect.u32Row = top;
    rect.u32ColumnCount = cw;
    rect.u32RowCount = ch;
	
    switch(size)
    {
    case TTX_NORMAL_SIZE:
    {
        for(y = 0; y < ch; underline >>= 1,  y ++, p_src += cpl * cw / 8)
        {
            bits = 0xffffffff;
            if((underline & 1) == 0)
            {
                bits  = ((p_src[3]<< 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                        >> shift;
                bits |= bits << bold;
            }

            for(x = 0; x < cw; bits >>= 1, x ++)
            {
                p_char_buf[x + (y * ttx_char_max_w)] = p_pen[bits & 1];
            }

            top ++;
        }

        //cp_size = (u32)(cw * ch);

        break;
    }

    case TTX_DOUBLE_WIDTH:
    {
        for(y = 0; y < ch; underline >>= 1, y ++, p_src += cpl * cw / 8)
        {
            bits = 0xffffffff;
            if((underline & 1) == 0)
            {
                bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                        >> shift;
                bits |= bits << bold;
            }

            for(x = 0; x < cw * 2; bits >>= 1, x += 2)
            {
                col = p_pen[bits & 1];

                p_char_buf[x + (y * ttx_char_max_w)] = col;
                p_char_buf[x + 1 + (y * ttx_char_max_w)] = col;
            }

            top ++;
        }

        //cp_size = (u32)(cw * ch * 2);
        rect.u32ColumnCount = (cw * 2);

        break;
    }
    case TTX_DOUBLE_HEIGHT:
    {
        for(y = 0; y < ch / 2; y ++, p_src += cpl * cw / 8)
        {
            bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                    >> shift;
            bits |= bits << bold;

            for(x = 0; x < cw; bits >>= 1, x ++)
            {
                col = p_pen[bits & 1];

                p_char_buf[x + (y * 2 * ttx_char_max_w)] = col;
                p_char_buf[x + ((y * 2 + 1) * ttx_char_max_w)] = col;
            }

            top += 2;
        }

        if((ch % 2) != 0)
        {
            bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                    >> shift;
            bits |= bits << bold;

            for(x = 0; x < cw; bits >>= 1, x ++)
            {
                p_char_buf[x + (y * 2 * ttx_char_max_w)] = p_pen[bits & 1];
            }
        }

        //cp_size = (u32)(cw * ch * 2);
        //     rect.bottom = rect.top + (ch * 2);      //linda zhu remove it, to fix bug 85889

        break;
    }
    case TTX_DOUBLE_SIZE:
    {
        for(y = 0; y < ch / 2; y ++, p_src += cpl * cw / 8)
        {
            bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                    >> shift;
            bits |= bits << bold;

            for(x = 0; x < cw * 2; bits >>= 1, x += 2)
            {
                col = p_pen[bits & 1];
                p_char_buf[x + (y * 2 * ttx_char_max_w)] = col;
                p_char_buf[x + 1 + (y * 2 * ttx_char_max_w)] = col;
                p_char_buf[x + ((y * 2 + 1) * ttx_char_max_w)] = col;
                p_char_buf[x + 1 + ((y * 2 + 1) * ttx_char_max_w)] = col;
            }

            top += 2;
        }

        if((ch % 2) != 0)
        {
            bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                    >> shift;
            bits |= bits << bold;

            for(x = 0; x < cw * 2; bits >>= 1, x += 2)
            {
                col = p_pen[bits & 1];
                p_char_buf[x + (y * 2 * ttx_char_max_w)] = col;
                p_char_buf[x + 1 + (y * 2 * ttx_char_max_w)] = col;
            }
        }

        //cp_size = (u32)(cw * ch * 4);
        rect.u32ColumnCount = (cw * 2);
        rect.u32RowCount = (ch * 2);

        break;
    }
    case TTX_DOUBLE_HEIGHT2:
    {
        p_src += cpl * cw / 8 * (ch / 2);
        underline >>= ch / 2;

        if((ch % 2) != 0)
        {
            bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                    >> shift;
            bits |= bits << bold;

            for(x = 0; x < cw; bits >>= 1, x ++)
            {
                y = 0;
                p_char_buf[x + (y * 2 * ttx_char_max_w)] = p_pen[bits & 1];
            }

            top ++;
            p_src += cpl * cw / 8;
        }

        for(y = 0; y < ch / 2; underline >>= 1, y ++, p_src += cpl * cw / 8)
        {
            bits = 0xffffffff;

            if((underline & 1) == 0)
            {
                bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                        >> shift;
                bits |= bits << bold;
            }

            for(x = 0; x < cw; bits >>= 1, x ++)
            {
                col = p_pen[bits & 1];

                p_char_buf[x + ((y * 2 + 1) * ttx_char_max_w)] = col;
                p_char_buf[x + ((y * 2 + 2) * ttx_char_max_w)] = col;
            }

            top += 2;
        }

        //cp_size = (u32)(cw * ch * 2);
        //     rect.bottom = rect.top + (ch * 2);      //linda zhu remove it, to fix bug 85889

        break;
    }
    case TTX_DOUBLE_SIZE2:
    {
        p_src += cpl * cw / 8 * (ch / 2);
        underline >>= ch / 2;
        x=y=0;
        
        if((ch % 2) != 0)
        {
            bits = 0xffffffff;

            bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                    >> shift;
            bits |= bits << bold;

			if (y > TTX_CHAR_MAX_H) { /*bug128627:char width beyond the char surface width when insert error code in long test */
				return;
			}

            for(x = 0; x < cw * 2; bits >>= 1, x += 2)
            {
                col = p_pen[bits & 1];

                p_char_buf[x + (y * 2 * ttx_char_max_w)] = col;
                p_char_buf[x + 1 + (y * 2 * ttx_char_max_w)] = col;
            }

            top ++;
            p_src += cpl * cw / 8;
        }

        for(y = 0; y < ch / 2; underline >>= 1, y ++, p_src += cpl * cw / 8)
        {
            bits = 0xffffffff;

            if((underline & 1) == 0)
            {
                bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                        >> shift;
                bits |= bits << bold;
            }

            for(x = 0; x < cw * 2; bits >>= 1, x += 2)
            {
                col = p_pen[bits & 1];
                if((ch % 2) != 0)
                {
                    p_char_buf[x + ((y * 2 + 1) * ttx_char_max_w)] = col;
                    p_char_buf[x + 1 + ((y * 2 + 1) * ttx_char_max_w)] = col;
                    p_char_buf[x + (((y * 2 + 1) + 1) * ttx_char_max_w)] = col;
                    p_char_buf[x + 1 + (((y * 2 + 1) + 1) * ttx_char_max_w)] = col;
                }
                else
                {
                    p_char_buf[x + (y * 2 * ttx_char_max_w)] = col;
                    p_char_buf[x + 1 + (y * 2 * ttx_char_max_w)] = col;
                    p_char_buf[x + ((y * 2 + 1) * ttx_char_max_w)] = col;
                    p_char_buf[x + 1 + ((y * 2 + 1) * ttx_char_max_w)] = col;                    
                }
            }

            top += 2;
        }

        //cp_size = (u32)(cw * ch * 4);
        rect.u32ColumnCount = (cw * 2);
        rect.u32RowCount = (ch * 2);

        break;
    }
    default:
        break;
    }
    {		
		p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_DRAWOSD, &rect);
		refresh_rect.pstPageArea = &rect;
//		if(g_inital_page)
			p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_REFRESH, &refresh_rect);		
    }
}


/*
 * Draw blank character for page mode.
 */
static void draw_blank_vsb_page_mode(ttx_render_vsb_t *p_render
                           , u32 top, u32 left, u8 color, s32 cw, s32 ch,u8 *cache_buf,s32 pitch)
{
    //RET_CODE ret = 0;
    //rect_t rect;
	//MT_UNF_TTX_FILLRECT_S fillrect;
	//MT_UNF_TTX_PAGEAREA_S rect = {0};
	//MT_UNF_TTX_REFRESHLAYER_S refresh_rect;
    s32 x = 0, y = 0;
//	printf("\r\n ~~~~~~~~~~%s, %d", __FUNCTION__, __LINE__);

    if(cache_buf == NULL)
    {
    	OS_PRINTF("draw_char_vsb NULL == p_char_buf \n");
        return;        
    }
    /*
    if(TRUE == p_render->is_sub)
    {
        MT_ASSERT(NULL != p_render->p_sub_hdl);
    }
    else
    {
        MT_ASSERT(NULL != p_render->p_osd_hdl);
    }
    */
    for(y = 0; y < ch; y++)
    {
        for(x = 0; x < cw ; x++)
        {
            cache_buf[(top * pitch + left) + x + (y * pitch)] = (u8)color;
        }
    }
    
#if 0 
	fillrect.u32Color = color;
	rect.u32Row = top;
	rect.u32Column = left;
	rect.u32RowCount = ch;
	rect.u32ColumnCount = cw;
   
	fillrect.pstPageArea = &rect;		
	p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_FILLRECT, &fillrect);
	refresh_rect.pstPageArea = &rect;
//	if(g_inital_page)
		p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_REFRESH, &refresh_rect);
#endif

#if 0	
    rect.left = left;
    rect.top = top;
    rect.right = left + cw;
    rect.bottom = top + ch;
	
    ret = gpe_draw_rectangle_vsb(p_render->p_gpe,
                                 (TRUE != p_render->is_sub) ? p_render->p_osd_hdl : p_render->p_sub_hdl,
                                 &rect,
                                 color);
    MT_ASSERT(SUCCESS == ret);
#endif	
}

/*
 * Draw a character for page mode.
  \param[in] cpl characters per line
  \param[in] cw character width
  \param[in] ch character height
  \param[in] glyph character index in font xbm
 */
static void draw_char_vsb_page_mode(ttx_render_vsb_t *p_render, u8 *p_pen, u32 top, u32 left
                          , const u8 *p_font, s32 cpl, s32 cw, s32 ch, s32 glyph
                          , s32 bold, u32 underline, ttx_size_t size,u8 *cache_buf,s32 pitch)
{
    //RET_CODE ret = 0;
    const u8  *p_src = NULL;
    s32 shift = 0, x = 0, y = 0, z = 0;
    u32 bits = 0;
    u8  col = 0;
    MT_UNF_TTX_PAGEAREA_S rect;
    //u32 cp_size = 0;
    u32 ttx_char_max_w = 0;
    u8 *p_char_buf = cache_buf;
    //int i = 0;
	//MT_UNF_TTX_REFRESHLAYER_S refresh_rect;

    if(NULL == p_char_buf)
    {
    	OS_PRINTF("draw_char_vsb NULL == p_char_buf\n");
      return;
    }
    bold = !!bold;

    y = (glyph / cpl) * ch;
    x = (glyph % cpl) * cw;
    if(p_render->font_size == TTX_FONT_HD)
    {
        z = WSTFONT2_WIDTH_HD * y + x;
//        ttx_char_max_w = TTX_CHAR_MAX_HD_W;
    }
    else
    {
#if 0    
        if(p_render->video_std == VID_STD_PAL)
            z = WSTFONT2_WIDTH_PAL * y + x;
        else
            z = WSTFONT2_WIDTH_NTSC * y + x;
#else
        z = WSTFONT2_WIDTH_PAL * y + x;
#endif
//        ttx_char_max_w = TTX_CHAR_MAX_W;
    }
	ttx_char_max_w = pitch;
	
    shift = z & 7;
    p_src = p_font + z / 8;

    rect.u32Column = left;
    rect.u32Row = top;
    rect.u32ColumnCount = cw;
    rect.u32RowCount = ch;
	
    switch(size)
    {
    case TTX_NORMAL_SIZE:
    {
        for(y = 0; y < ch; underline >>= 1,  y ++, p_src += cpl * cw / 8)
        {
            bits = 0xffffffff;
            if((underline & 1) == 0)
            {
                bits  = ((p_src[3]<< 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                        >> shift;
                bits |= bits << bold;
            }

            for(x = 0; x < cw; bits >>= 1, x ++)
            {
                p_char_buf[(rect.u32Row * pitch + left) + x + (y * ttx_char_max_w)] = p_pen[bits & 1];
            }

            top ++;
        }

        //cp_size = (u32)(cw * ch);

        break;
    }

    case TTX_DOUBLE_WIDTH:
    {
        for(y = 0; y < ch; underline >>= 1, y ++, p_src += cpl * cw / 8)
        {
            bits = 0xffffffff;
            if((underline & 1) == 0)
            {
                bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                        >> shift;
                bits |= bits << bold;
            }

            for(x = 0; x < cw * 2; bits >>= 1, x += 2)
            {
                col = p_pen[bits & 1];

                p_char_buf[(rect.u32Row * pitch + left) +  x + (y * ttx_char_max_w)] = col;
                p_char_buf[(rect.u32Row * pitch + left) +  x + 1 + (y * ttx_char_max_w)] = col;
            }

            top ++;
        }

        //cp_size = (u32)(cw * ch * 2);
        rect.u32ColumnCount = (cw * 2);

        break;
    }
    case TTX_DOUBLE_HEIGHT:
    {
        for(y = 0; y < ch / 2; y ++, p_src += cpl * cw / 8)
        {
            bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                    >> shift;
            bits |= bits << bold;

            for(x = 0; x < cw; bits >>= 1, x ++)
            {
                col = p_pen[bits & 1];

                p_char_buf[(rect.u32Row * pitch + left) + x + (y * 2 * ttx_char_max_w)] = col;
                p_char_buf[(rect.u32Row * pitch + left) + x + ((y * 2 + 1) * ttx_char_max_w)] = col;
            }

            top += 2;
        }

        if((ch % 2) != 0)
        {
            bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                    >> shift;
            bits |= bits << bold;

            for(x = 0; x < cw; bits >>= 1, x ++)
            {
                p_char_buf[(rect.u32Row * pitch + left) + x + (y * 2 * ttx_char_max_w)] = p_pen[bits & 1];
            }
        }

        //cp_size = (u32)(cw * ch * 2);
        //     rect.bottom = rect.top + (ch * 2);      //linda zhu remove it, to fix bug 85889

        break;
    }
    case TTX_DOUBLE_SIZE:
    {
        for(y = 0; y < ch / 2; y ++, p_src += cpl * cw / 8)
        {
            bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                    >> shift;
            bits |= bits << bold;

            for(x = 0; x < cw * 2; bits >>= 1, x += 2)
            {
                col = p_pen[bits & 1];
                p_char_buf[(rect.u32Row * pitch + left) + x + (y * 2 * ttx_char_max_w)] = col;
                p_char_buf[(rect.u32Row * pitch + left) + x + 1 + (y * 2 * ttx_char_max_w)] = col;
                p_char_buf[(rect.u32Row * pitch + left) + x + ((y * 2 + 1) * ttx_char_max_w)] = col;
                p_char_buf[(rect.u32Row * pitch + left) + x + 1 + ((y * 2 + 1) * ttx_char_max_w)] = col;
            }

            top += 2;
        }

        if((ch % 2) != 0)
        {
            bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                    >> shift;
            bits |= bits << bold;

            for(x = 0; x < cw * 2; bits >>= 1, x += 2)
            {
                col = p_pen[bits & 1];
                p_char_buf[(rect.u32Row * pitch + left) + x + (y * 2 * ttx_char_max_w)] = col;
                p_char_buf[(rect.u32Row * pitch + left) + x + 1 + (y * 2 * ttx_char_max_w)] = col;
            }
        }

        //cp_size = (u32)(cw * ch * 4);
        rect.u32ColumnCount = (cw * 2);
        rect.u32RowCount = (ch * 2);

        break;
    }
    case TTX_DOUBLE_HEIGHT2:
    {
        p_src += cpl * cw / 8 * (ch / 2);
        underline >>= ch / 2;

        if((ch % 2) != 0)
        {
            bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                    >> shift;
            bits |= bits << bold;

            for(x = 0; x < cw; bits >>= 1, x ++)
            {
                y = 0;
                p_char_buf[(rect.u32Row * pitch + left) + x + (y * 2 * ttx_char_max_w)] = p_pen[bits & 1];
            }

            top ++;
            p_src += cpl * cw / 8;
        }

        for(y = 0; y < ch / 2; underline >>= 1, y ++, p_src += cpl * cw / 8)
        {
            bits = 0xffffffff;

            if((underline & 1) == 0)
            {
                bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                        >> shift;
                bits |= bits << bold;
            }

            for(x = 0; x < cw; bits >>= 1, x ++)
            {
                col = p_pen[bits & 1];

                p_char_buf[(rect.u32Row * pitch + left) + x + ((y * 2 + 1) * ttx_char_max_w)] = col;
                p_char_buf[(rect.u32Row * pitch + left) + x + ((y * 2 + 2) * ttx_char_max_w)] = col;
            }

            top += 2;
        }

        //cp_size = (u32)(cw * ch * 2);
        //     rect.bottom = rect.top + (ch * 2);      //linda zhu remove it, to fix bug 85889

        break;
    }
    case TTX_DOUBLE_SIZE2:
    {
        p_src += cpl * cw / 8 * (ch / 2);
        underline >>= ch / 2;
        x = y = 0;
        
        if((ch % 2) != 0)
        {
            bits = 0xffffffff;

            bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                    >> shift;
            bits |= bits << bold;

			if (y > TTX_CHAR_MAX_H) { /*bug128627:char width beyond the char surface width when insert error code in long test */
				return;
			}

            for(x = 0; x < cw * 2; bits >>= 1, x += 2)
            {
                col = p_pen[bits & 1];

                p_char_buf[(rect.u32Row * pitch + left) + x + (y * 2 * ttx_char_max_w)] = col;
                p_char_buf[(rect.u32Row * pitch + left) + x + 1 + (y * 2 * ttx_char_max_w)] = col;
            }

            top ++;
            p_src += cpl * cw / 8;
        }

        for(y = 0; y < ch / 2; underline >>= 1, y ++, p_src += cpl * cw / 8)
        {
            bits = 0xffffffff;

            if((underline & 1) == 0)
            {
                bits  = ((p_src[3] << 24) + (p_src[2] << 16) + (p_src[1] << 8) + p_src[0])
                        >> shift;
                bits |= bits << bold;
            }

            for(x = 0; x < cw * 2; bits >>= 1, x += 2)
            {
                col = p_pen[bits & 1];
                
                if((ch % 2) != 0)
                {                    
                    p_char_buf[(rect.u32Row * pitch + left) + x + ((y * 2 + 1) * ttx_char_max_w)] = col;
                    p_char_buf[(rect.u32Row * pitch + left) + x + 1 + ((y * 2 + 1) * ttx_char_max_w)] = col;
                    p_char_buf[(rect.u32Row * pitch + left) + x + (((y * 2 + 1) + 1) * ttx_char_max_w)] = col;
                    p_char_buf[(rect.u32Row * pitch + left) + x + 1 + (((y * 2 + 1) + 1) * ttx_char_max_w)] = col;                    
                }
                else
                {
                    p_char_buf[(rect.u32Row * pitch + left) + x + (y * 2 * ttx_char_max_w)] = col;
                    p_char_buf[(rect.u32Row * pitch + left) + x + 1 + (y * 2 * ttx_char_max_w)] = col;
                    p_char_buf[(rect.u32Row * pitch + left) + x + ((y * 2 + 1) * ttx_char_max_w)] = col;
                    p_char_buf[(rect.u32Row * pitch + left) + x + 1 + ((y * 2 + 1) * ttx_char_max_w)] = col;
                }
            }

            top += 2;
        }

        //cp_size = (u32)(cw * ch * 4);
        rect.u32ColumnCount = (cw * 2);
        rect.u32RowCount = (ch * 2);

        break;
    }
    default:
        break;
    }
#if 0    
    {		
		p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_DRAWOSD, &rect);
		refresh_rect.pstPageArea = &rect;
//		if(g_inital_page)
			p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_REFRESH, &refresh_rect);		
    }
#endif

}

/*
 * clear first character line is also the header line.
 */
void ttx_render_clear_header_vsb(ttx_render_vsb_t *p_render, u8 color)
{
    //RET_CODE ret = 0;
    s16 char_h = 0;
	MT_UNF_TTX_FILLRECT_S fillopt = {0};
	MT_UNF_TTX_PAGEAREA_S rect = {0};
	MT_UNF_TTX_REFRESHLAYER_S refresh_rect;
    if(p_render->font_size == TTX_FONT_HD)
    {
        char_h = TTX_CHAR_H_HD;
    }
    else if(p_render->video_std == VID_STD_PAL)
    {
        char_h = TTX_CHAR_H_PAL;
    }
    else
    {
        char_h = TTX_CHAR_H_NTSC;
    }

    if(TRUE == p_render->is_sub)
    {
        MT_ASSERT(NULL != p_render->p_sub_hdl);
    }
    else
    {
        MT_ASSERT(NULL != p_render->p_osd_hdl);
    }


	fillopt.u32Color = color;
	rect.u32Row = p_render->page_y;
	rect.u32Column = p_render->page_x;
	rect.u32RowCount = char_h;
	rect.u32ColumnCount = p_render->page_w;
	fillopt.pstPageArea = &rect;	

	p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_FILLRECT, &fillopt);
	refresh_rect.pstPageArea = &rect;
//	if(g_inital_page)
		p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_REFRESH, &refresh_rect);	
		


    return;
}

/*
 * clear whole page.
 */
void ttx_render_clear_page_vsb(ttx_render_vsb_t *p_render, u8 color)
{
    //RET_CODE ret = 0;
	MT_UNF_TTX_FILLRECT_S fillopt = {0};
	MT_UNF_TTX_PAGEAREA_S rect = {0};
	MT_UNF_TTX_REFRESHLAYER_S refresh_rect;
//	printf("\r\n ~~~~~~~~~~~~%s", __FUNCTION__);

    if(TRUE == p_render->is_sub)
    {
        MT_ASSERT(NULL != p_render->p_sub_hdl);
    }
    else
    {
        MT_ASSERT(NULL != p_render->p_osd_hdl);
    }

	fillopt.u32Color = color;
	rect.u32Row = p_render->page_y;
	rect.u32Column = p_render->page_x;
	rect.u32RowCount = p_render->page_h;
	rect.u32ColumnCount = p_render->page_w;
	fillopt.pstPageArea = &rect;

	p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_FILLRECT, &fillopt);
	refresh_rect.pstPageArea = &rect;
//	if(g_inital_page)
		p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_REFRESH, &refresh_rect);
		


    return;
}

/*
 * Draw page number.
 */
void ttx_render_draw_page_no_vsb(ttx_render_vsb_t *p_render, u16 page_no, u16 sub_no, MT_BOOL b_dec)
{
    s32 i = 0;
    u16 unicode = 0;
    u8  pen[2];
    u32 cnt = 0;
    u32 char_w = 0, char_h = 0;
    const u8 *p_font = NULL;

    RET_CODE    ret __attribute__((unused)) = 0;

//printf("\r\n ~~~~~~~~~~~~%s,%d", __FUNCTION__, __LINE__);
    if(vbi_get_ttx_page_draw_mode() == 1)
    {
        ttx_render_draw_page_no_vsb_page_mode(p_render,page_no,sub_no,b_dec);
    }
    else
    {
        if(p_render->font_size == TTX_FONT_NORMAL)
        {
            if(p_render->video_std == VID_STD_PAL)
            {
                char_h = TTX_CHAR_H_PAL;
                char_w = TTX_CHAR_W_PAL;
                p_font = vbi_get_font(TTX_FONT_SRC_PAL);
            }
            else
            {
                char_h = TTX_CHAR_H_NTSC;
                char_w = TTX_CHAR_W_NTSC;
                p_font = vbi_get_font(TTX_FONT_SRC_NTSL);
            }
        }
        else if(p_render->font_size == TTX_FONT_SMALL)
        {
            char_h = TTX_CHAR_H_SMALL;
            char_w = TTX_CHAR_W_SMALL;
            p_font = vbi_get_font(TTX_FONT_SRC_SMALL);
        }
        else
        {
            char_h = TTX_CHAR_H_HD;
            char_w = TTX_CHAR_W_HD;
            p_font = vbi_get_font(TTX_FONT_SRC_HD);
        }
        MT_ASSERT(p_font != NULL);


        if(TRUE != b_dec)
        {
        	ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_SETPALETTE, p_render->clut);
    //        ret = region_set_palette(p_render->p_osd_hdl, (u32 *)p_render->clut, 256);
            MT_ASSERT(SUCCESS == ret);
        }

        pen[0] = 40 + TTX_BLACK;

        cnt = (page_no &0xf000) >> 12;
        cnt = (cnt <= 3) ? 3 - cnt : 0;
    //	printf("\r\n b_dec:%d, page_no:0x%08x,sub_no:%d", b_dec, page_no, sub_no);

        if(TRUE == b_dec)//when is true, update the render area of the incoming page_no
        {
            if((page_no >= 0x100) && (page_no <= 0x8ff))
            {
                for(i = 1; i < 4; i ++)
                {
                    if(cnt-- > 0)
                        unicode = ((page_no >> ((3 - i) * 4)) & 0xf) + 0x30;
                    pen[1] = TTX_RED;
    				printf("\r\n %s, %d", __FUNCTION__, __LINE__);
                    draw_char_vsb(p_render
                                  , pen
                                  , p_render->page_y
                                  , p_render->page_x + char_w * i + (16 * char_w)//incoming page is at 16 colume
                                  , p_font
                                  , TTX_CHAR_NUM_PER_LINE, char_w, char_h
                                  , unicode_wstfont2_vsb (unicode, 0)
                                  , 0
                                  , 0
                                  , TTX_NORMAL_SIZE);
                }
            }
        }
        else
        {
            for(i = 0; i < 8; i ++)
            {
                if(sub_no == 0
                        || sub_no == TTX_NULL_SUBPAGE
                        || sub_no == TTX_ANY_SUBPAGE
                        || sub_no == TTX_FIRST_SUBPAGE)
                {
                    if(i == 0)
                    {
                        unicode = ' ';
                    }
                    else if(i < 4)
                    {
                        if(cnt-- > 0)
                            unicode = ((page_no >> ((3 - i) * 4)) & 0xf) + 0x30;
                        else
                            unicode = '_';
                    }
                    else
                    {
                        unicode = ' ';
                    }
                }
                else
                {
                    if(i == 0)
                    {
                        unicode = ' ';
                    }
                    else if(i < 4)
                    {
                        if(cnt-- > 0)
                            unicode = ((page_no >> ((3 - i) * 4)) & 0xf) + 0x30;
                        else
                            unicode = '_';
                    }
                    else if(i == 4)
                    {
                        if(sub_no != 0)
                            unicode = '/';
                    }
                    else
                    {
                        unicode = ((sub_no >> ((6 - i) * 4)) & 0xf) + 0x30;
                    }
                }

                pen[1] = TTX_WHITE;
    	//		printf("\r\n %s, %d", __FUNCTION__, __LINE__);
                draw_char_vsb(p_render
                              , pen
                              , p_render->page_y
                              , p_render->page_x + char_w * i
                              , p_font
                              , TTX_CHAR_NUM_PER_LINE, char_w, char_h
                              , unicode_wstfont2_vsb (unicode, 0)
                              , 0
                              , 0
                              , TTX_NORMAL_SIZE);
            }
        }
    //	printf("\r\n ~~~~~~~~~~~~%s,%d", __FUNCTION__, __LINE__);
    //	printf("\r\n ~~~~~~~~~~~~%s,%d", __FUNCTION__, __LINE__);
#if 0
        p_disp_dev = dev_find_identifier(NULL, DEV_IDT_TYPE,
                                         SYS_DEV_TYPE_DISPLAY);
#ifndef WIN32
        if(FALSE == b_dec)
            disp_layer_update_region(p_disp_dev, p_render->p_osd_hdl, NULL);
#endif
#ifndef WIN32
#ifdef CACHE_ON
        extern void flush_dcache_all();
        flush_dcache_all();
#endif
#endif
#endif
    }
}

/*
 * Draw whole page.
 */
void ttx_render_draw_page_vsb(ttx_render_vsb_t *p_render, MT_BOOL subtile
                              , ttx_osd_page_t *p_osd_page
                              , u8 first_row, u8 last_row
                              , u8 first_column, u8 last_column
                              , MT_BOOL conceal, MT_BOOL flash_on)
{  
    RET_CODE    ret __attribute__((unused)) = 0;
    u32         i = 0;
    u32         *p_c = NULL;
    u8          pen[42];
    u16         unicode = 0;
    ttx_char_t  *p_ac = NULL;
    u8          row = 0, column = 0;
    s32         top = 0, left = 0;
    s32         char_w = 0, char_h = 0;
    const u8    *p_font = NULL;
    

    //unsigned long long        tickms_old     = 0;
    //unsigned long long        tickms_new      = 0;

    if(vbi_get_ttx_page_draw_mode() == 1)
    {
        ttx_render_draw_page_vsb_page_mode(p_render,subtile,p_osd_page, first_row,last_row
        ,first_column, last_column,conceal,flash_on);
    }
    else    
    {
        //tickms_old =  mt_get_time_ms();
        if(p_render->font_size == TTX_FONT_NORMAL)
        {
            if(p_render->video_std == VID_STD_PAL)
            {
                char_h = TTX_CHAR_H_PAL;
                char_w = TTX_CHAR_W_PAL;
                p_font = vbi_get_font(TTX_FONT_SRC_PAL);
            }
            else
            {
                char_h = TTX_CHAR_H_NTSC;
                char_w = TTX_CHAR_W_NTSC;
                p_font = vbi_get_font(TTX_FONT_SRC_NTSL);
            }
        }
        else if(p_render->font_size == TTX_FONT_SMALL)
        {
            char_h = TTX_CHAR_H_SMALL;
            char_w = TTX_CHAR_W_SMALL;
            p_font = vbi_get_font(TTX_FONT_SRC_SMALL);
        }
        else
        {
            char_h = TTX_CHAR_H_HD;
            char_w = TTX_CHAR_W_HD;
            p_font = vbi_get_font(TTX_FONT_SRC_HD);
        }
        MT_ASSERT(p_font != NULL);
        top  = p_render->page_y + char_h * first_row;
        left = p_render->page_x + char_w * first_column;

    //	printf("\r\n ~~~~~~~~~~~~%s,%d, page_x:%d, char_w:%d, first col:%d", __FUNCTION__, __LINE__,
    //		p_render->page_x, char_w, first_column);

        memcpy(p_render->clut
               , p_osd_page->color_map
               , sizeof(p_osd_page->color_map));

        p_c = p_render->clut + sizeof(p_osd_page->color_map) / 4;
        for(i = 0; i < (sizeof(p_osd_page->color_map) / 4); i ++, p_c ++)
            *p_c = (p_render->clut[i] & 0xffffff)
                   | (0xff * p_osd_page->user_screen_opacity / 100) << 24;

#if 0
        if(subtile)
        {
        	ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_SETPALETTE, p_render->clut);
    //        ret = region_set_palette(p_render->p_sub_hdl, (u32 *)p_render->clut, 256);
            MT_ASSERT(SUCCESS == ret);
            g_inital_page = 1;
        }
        else
        {
        	ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_SETPALETTE, p_render->clut);
            //ret = region_set_palette(p_render->p_osd_hdl, (u32 *)p_render->clut, 256);
            MT_ASSERT(SUCCESS == ret);
        }
#else
        	ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_SETPALETTE, p_render->clut);
          MT_ASSERT(SUCCESS == ret);

#endif
        if(p_osd_page->p_drcs_clut != NULL)
            memcpy(pen + 2, p_osd_page->p_drcs_clut, 40);

        
        for(row = first_row; row < last_row; row ++)
        {
            p_ac = &p_osd_page->text[row * p_osd_page->columns + first_column];

            for(column = first_column; column < last_column; column ++, p_ac ++)
            {
                if((p_ac->conceal != 0 && conceal == TRUE)
                        || (p_ac->flash != 0 && flash_on == TRUE))
                    unicode = 0x0020;
                else
                    unicode = p_ac->unicode;

                pen[0] = p_ac->background + sizeof(p_osd_page->color_map) / 4;
                pen[1] = p_ac->foreground;

                if(p_ac->opacity == TTX_TRANSPARENT_SPACE
                        || p_ac->opacity == TTX_TRANSPARENT_FULL)
                {
                    draw_blank_vsb(p_render
                                   , top
                                   , left
                                   , SCREEM_COLOR_INDEX
                                   , char_w, char_h);

                    left += char_w;

                    continue;
                }

                switch(p_ac->size)
                {
                case TTX_OVER_TOP:
                case TTX_OVER_BOTTOM:
                    break;

                default:
                    if(vbi_is_drcs_vsb(unicode) == TRUE)
                    {
#if TTX_SUPPORT_DRCS
                        p_font = p_osd_page->p_drcs[(unicode >> 6) & 0x1F];

                        if(p_font != NULL)
                            draw_drcs_vsb(p_render
                                          , pen
                                          , p_ac->drcs_clut_offs
                                          , top
                                          , left
                                          , p_font
                                          , unicode & 0x3F
                                          , p_ac->size);
                        else /* shouldn't happen */
                            MT_ASSERT(0);
#else
                        draw_blank_vsb(p_render
                                       , top
                                       , left
                                       , pen[0]
                                       , char_w, char_h);
#endif
                    }
                    else
                    {
                    
                    if(g_inital_page)  //fix bug 108790       	
                        draw_char_vsb(p_render
                                      , pen
                                      , top
                                      , left
                                      , p_font
                                      , TTX_CHAR_NUM_PER_LINE, char_w, char_h
                                      , unicode_wstfont2_vsb (unicode, p_ac->italic)
                                      , p_ac->bold
                                      , p_ac->underline << (char_h - 1) /* cell row 9 */
                                      , p_ac->size);
                    }
                }

                left += char_w;
            }

            if(column == TTX_COLUMNS)
            {
                ttx_char_t  *p_tmp = p_ac - 1;

                if(p_tmp->opacity == TTX_TRANSPARENT_SPACE
                        || p_tmp->opacity == TTX_TRANSPARENT_FULL)
                {
                    draw_blank_vsb(p_render
                                   , top
                                   , left
                                   , SCREEM_COLOR_INDEX
                                   , char_w, char_h);
                }
                else
                {
                    draw_blank_vsb(p_render
                                   , top
                                   , left
                                   , 40 + TTX_BLACK
                                   , char_w, char_h);
                }
            }
            else
            {
                draw_blank_vsb(p_render
                               , top
                               , left
                               , SCREEM_COLOR_INDEX
                               , char_w, char_h);
            }

            left = p_render->page_x + char_w * first_column;
            top += char_h;
        }
        //tickms_new = mt_get_time_ms();
        //printf("%s -- tick: %lld \n ",__FUNCTION__,mt_get_time_interval_ms(tickms_new,tickms_old));    
#if 0
        p_disp_dev = dev_find_identifier(NULL, DEV_IDT_TYPE,
                                         SYS_DEV_TYPE_DISPLAY);
#ifndef WIN32
        if(subtile)
        {
            disp_layer_update_region(p_disp_dev, p_render->p_sub_hdl, NULL);
            ret = region_show(p_render->p_sub_hdl, TRUE);
            MT_ASSERT(SUCCESS == ret);
        }
        else
        {
            disp_layer_update_region(p_disp_dev, p_render->p_osd_hdl, NULL);
            ret = region_show(p_render->p_osd_hdl, TRUE);
            MT_ASSERT(SUCCESS == ret);
        }
#endif

#ifndef WIN32
#ifdef CACHE_ON
        extern void flush_dcache_all();
        flush_dcache_all();
#endif
#endif
#endif
    }
    return; 
}


/*
 * Draw page number.
 */
static void ttx_render_draw_page_no_vsb_page_mode(ttx_render_vsb_t *p_render, u16 page_no, u16 sub_no, MT_BOOL b_dec)
{
    s32 i = 0;
    u16 unicode = 0;
    u8  pen[2];
    u32 cnt = 0;
    u32 char_w = 0, char_h = 0;
    const u8 *p_font = NULL;
    //void *p_disp_dev = NULL;
    
    RET_CODE    ret __attribute__((unused)) = 0;
    MT_UNF_TTX_CHAR_BUFFER_PARAM_S      draw_page_param = {0}; 
    MT_UNF_TTX_REFRESHLAYER_S           reflush_param = {NULL};
    MT_UNF_TTX_PAGEAREA_S               reflush_area = {0};
//printf("\r\n ~~~~~~~~~~~~%s,%d", __FUNCTION__, __LINE__);

    draw_page_param.width= p_render->region_w;
    draw_page_param.height= p_render->region_h;
    draw_page_param.pBuf= NULL;
    draw_page_param.pitch= 0;
    
    //printf("draw_page %d w:%d,h:%d !!!!! \n ", __LINE__,draw_page_param.width,draw_page_param.height);    
    ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_DRAWPAGE_START, &draw_page_param);
    //printf("draw_page %d buf:0x%x,pitch:%d !!!!! \n ", __LINE__,draw_page_param.pBuf,draw_page_param.pitch); 
    if(draw_page_param.pBuf == NULL)
    {
        printf("\r\n %s, %d error !!!!!", __FUNCTION__, __LINE__);
        return;
    }
    
    if(p_render->font_size == TTX_FONT_NORMAL)
    {
        if(p_render->video_std == VID_STD_PAL)
        {
            char_h = TTX_CHAR_H_PAL;
            char_w = TTX_CHAR_W_PAL;
            p_font = vbi_get_font(TTX_FONT_SRC_PAL);
        }
        else
        {
            char_h = TTX_CHAR_H_NTSC;
            char_w = TTX_CHAR_W_NTSC;
            p_font = vbi_get_font(TTX_FONT_SRC_NTSL);
        }
    }
    else if(p_render->font_size == TTX_FONT_SMALL)
    {
        char_h = TTX_CHAR_H_SMALL;
        char_w = TTX_CHAR_W_SMALL;
        p_font = vbi_get_font(TTX_FONT_SRC_SMALL);
    }
    else
    {
        char_h = TTX_CHAR_H_HD;
        char_w = TTX_CHAR_W_HD;
        p_font = vbi_get_font(TTX_FONT_SRC_HD);
    }
    MT_ASSERT(p_font != NULL);
    
    reflush_area.u32Row=p_render->page_y;
    reflush_area.u32Column=p_render->page_x;

    if(TRUE != b_dec)
    {
    	ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_SETPALETTE, p_render->clut);
//        ret = region_set_palette(p_render->p_osd_hdl, (u32 *)p_render->clut, 256);
        MT_ASSERT(SUCCESS == ret);
    }

    pen[0] = 40 + TTX_BLACK;

    cnt = (page_no &0xf000) >> 12;
    cnt = (cnt <= 3) ? 3 - cnt : 0;
//	printf("\r\n b_dec:%d, page_no:0x%08x,sub_no:%d", b_dec, page_no, sub_no);

    if(TRUE == b_dec)//when is true, update the render area of the incoming page_no
    {
        if((page_no >= 0x100) && (page_no <= 0x8ff))
        {
            for(i = 1; i < 4; i ++)
            {
                if(cnt-- > 0)
                    unicode = ((page_no >> ((3 - i) * 4)) & 0xf) + 0x30;
                pen[1] = TTX_RED;
				printf("\r\n %s, %d", __FUNCTION__, __LINE__);
                draw_char_vsb_page_mode(p_render
                              , pen
                              , p_render->page_y
                              , p_render->page_x + char_w * i + (16 * char_w)//incoming page is at 16 colume
                              , p_font
                              , TTX_CHAR_NUM_PER_LINE, char_w, char_h
                              , unicode_wstfont2_vsb (unicode, 0)
                              , 0
                              , 0
                              , TTX_NORMAL_SIZE,draw_page_param.pBuf,draw_page_param.pitch);
            }
            reflush_area.u32RowCount   =char_h;
            reflush_area.u32ColumnCount=p_render->page_x + char_w * i + (16 * char_w)  - reflush_area.u32Column;            
        }
    }
    else
    {
        for(i = 0; i < 8; i ++)
        {
            if(sub_no == 0
                    || sub_no == TTX_NULL_SUBPAGE
                    || sub_no == TTX_ANY_SUBPAGE
                    || sub_no == TTX_FIRST_SUBPAGE)
            {
                if(i == 0)
                {
                    unicode = ' ';
                }
                else if(i < 4)
                {
                    if(cnt-- > 0)
                        unicode = ((page_no >> ((3 - i) * 4)) & 0xf) + 0x30;
                    else
                        unicode = '_';
                }
                else
                {
                    unicode = ' ';
                }
            }
            else
            {
                if(i == 0)
                {
                    unicode = ' ';
                }
                else if(i < 4)
                {
                    if(cnt-- > 0)
                        unicode = ((page_no >> ((3 - i) * 4)) & 0xf) + 0x30;
                    else
                        unicode = '_';
                }
                else if(i == 4)
                {
                    if(sub_no != 0)
                        unicode = '/';
                }
                else
                {
                    unicode = ((sub_no >> ((6 - i) * 4)) & 0xf) + 0x30;
                }
            }

            pen[1] = TTX_WHITE;
	//		printf("\r\n %s, %d", __FUNCTION__, __LINE__);
            draw_char_vsb_page_mode(p_render
                          , pen
                          , p_render->page_y
                          , p_render->page_x + char_w * i
                          , p_font
                          , TTX_CHAR_NUM_PER_LINE, char_w, char_h
                          , unicode_wstfont2_vsb (unicode, 0)
                          , 0
                          , 0
                          , TTX_NORMAL_SIZE,draw_page_param.pBuf,draw_page_param.pitch);
        }
        
        reflush_area.u32RowCount   =char_h;
        reflush_area.u32ColumnCount=p_render->page_x + char_w * i  - reflush_area.u32Column;           
    }
//	printf("\r\n ~~~~~~~~~~~~%s,%d", __FUNCTION__, __LINE__);
//	printf("\r\n ~~~~~~~~~~~~%s,%d", __FUNCTION__, __LINE__);

    ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_DRAWPAGE_END,&reflush_param);

#if 0
    p_disp_dev = dev_find_identifier(NULL, DEV_IDT_TYPE,
                                     SYS_DEV_TYPE_DISPLAY);
#ifndef WIN32
    if(FALSE == b_dec)
        disp_layer_update_region(p_disp_dev, p_render->p_osd_hdl, NULL);
#endif
#ifndef WIN32
#ifdef CACHE_ON
    extern void flush_dcache_all();
    flush_dcache_all();
#endif
#endif
#endif
}


/*
    ttx_render_draw_page_vsb_page_mode
  draw a page use whole page mode

  \param[in] p_osd_page
  \param[in] first_row
  \param[in] first_column
  \param[in] last_row
  \param[in] last_column
  \param[in] conceal
  \param[in] flash_on
 */
static void ttx_render_draw_page_vsb_page_mode(ttx_render_vsb_t *p_render, MT_BOOL subtile
                              , ttx_osd_page_t *p_osd_page
                              , u8 first_row, u8 last_row
                              , u8 first_column, u8 last_column
                              , MT_BOOL conceal, MT_BOOL flash_on)
{
    RET_CODE    ret __attribute__((unused)) = 0;
    u32         i = 0;
    u32         *p_c = NULL;
    u8          pen[42];
    u16         unicode = 0;
    ttx_char_t  *p_ac = NULL;
    u8          row = 0, column = 0;
    s32         top = 0, left = 0;
    s32         char_w = 0, char_h = 0;
    const u8    *p_font = NULL;
    //void *p_disp_dev = NULL;

    //unsigned long long        tickms_old     = 0;
    //unsigned long long        tickms_new      = 0;
    
    MT_UNF_TTX_CHAR_BUFFER_PARAM_S      draw_page_param = {0}; 
    MT_UNF_TTX_REFRESHLAYER_S           reflush_param = {NULL};
    MT_UNF_TTX_PAGEAREA_S               reflush_area = {0};
        
    //tickms_old =  mt_get_time_ms();
    draw_page_param.width= p_render->region_w;
    draw_page_param.height= p_render->region_h;
    draw_page_param.pBuf= NULL;
    draw_page_param.pitch= 0;

    reflush_param.pstPageArea = &reflush_area;
    
    //printf("draw_page %d w:%d,h:%d !!!!! \n ", __LINE__,draw_page_param.width,draw_page_param.height);    
    ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_DRAWPAGE_START, &draw_page_param);
    //printf("draw_page %d buf:0x%x,pitch:%d !!!!! \n ", __LINE__,draw_page_param.pBuf,draw_page_param.pitch); 
    if(draw_page_param.pBuf == NULL)
    {
        printf("\r\n %s, %d error !!!!!", __FUNCTION__, __LINE__);
        return;
    }
    
    if(p_render->font_size == TTX_FONT_NORMAL)
    {
        if(p_render->video_std == VID_STD_PAL)
        {
            char_h = TTX_CHAR_H_PAL;
            char_w = TTX_CHAR_W_PAL;
            p_font = vbi_get_font(TTX_FONT_SRC_PAL);
        }
        else
        {
            char_h = TTX_CHAR_H_NTSC;
            char_w = TTX_CHAR_W_NTSC;
            p_font = vbi_get_font(TTX_FONT_SRC_NTSL);
        }
    }
    else if(p_render->font_size == TTX_FONT_SMALL)
    {
        char_h = TTX_CHAR_H_SMALL;
        char_w = TTX_CHAR_W_SMALL;
        p_font = vbi_get_font(TTX_FONT_SRC_SMALL);
    }
    else
    {
        char_h = TTX_CHAR_H_HD;
        char_w = TTX_CHAR_W_HD;
        p_font = vbi_get_font(TTX_FONT_SRC_HD);
    }
    MT_ASSERT(p_font != NULL);
    top  = p_render->page_y + char_h * first_row;
    left = p_render->page_x + char_w * first_column;
    reflush_area.u32Row=top;
    reflush_area.u32Column=left;
        
    //printf("%s,%d, page_x:%d, char_w:%d, first col:%d \n", __FUNCTION__, __LINE__,
    //p_render->page_x, char_w, first_column);
    //
    //printf("%s,%d, page_y:%d, char_h:%d, first row:%d \n", __FUNCTION__, __LINE__,
    //p_render->page_y, char_h, first_row);
    
    memcpy(p_render->clut
           , p_osd_page->color_map
           , sizeof(p_osd_page->color_map));

    p_c = p_render->clut + sizeof(p_osd_page->color_map) / 4;
    for(i = 0; i < (sizeof(p_osd_page->color_map) / 4); i ++, p_c ++)
        *p_c = (p_render->clut[i] & 0xffffff)
               | (0xff * p_osd_page->user_screen_opacity / 100) << 24;

#if 0
    if(subtile)
    {
    	ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_SETPALETTE, p_render->clut);
//        ret = region_set_palette(p_render->p_sub_hdl, (u32 *)p_render->clut, 256);
        MT_ASSERT(SUCCESS == ret);
        g_inital_page = 1;
    }
    else
    {
    	ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_SETPALETTE, p_render->clut);
        //ret = region_set_palette(p_render->p_osd_hdl, (u32 *)p_render->clut, 256);
        MT_ASSERT(SUCCESS == ret);
    }
#else
    ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_SETPALETTE, p_render->clut);
    MT_ASSERT(SUCCESS == ret);
#endif
    if(p_osd_page->p_drcs_clut != NULL)
        memcpy(pen + 2, p_osd_page->p_drcs_clut, 40);

    for(row = first_row; row < last_row; row ++)
    {
        p_ac = &p_osd_page->text[row * p_osd_page->columns + first_column];

        for(column = first_column; column < last_column; column ++, p_ac ++)
        {
            if((p_ac->conceal != 0 && conceal == TRUE)
                    || (p_ac->flash != 0 && flash_on == TRUE))
                unicode = 0x0020;
            else
                unicode = p_ac->unicode;

            pen[0] = p_ac->background + sizeof(p_osd_page->color_map) / 4;
            pen[1] = p_ac->foreground;

            if(p_ac->opacity == TTX_TRANSPARENT_SPACE
                    || p_ac->opacity == TTX_TRANSPARENT_FULL)
            {
                draw_blank_vsb_page_mode(p_render
                               , top
                               , left
                               , SCREEM_COLOR_INDEX
                               , char_w, char_h,draw_page_param.pBuf,draw_page_param.pitch);

                left += char_w;

                continue;
            }

            switch(p_ac->size)
            {
            case TTX_OVER_TOP:
            case TTX_OVER_BOTTOM:
                break;

            default:
                if(vbi_is_drcs_vsb(unicode) == TRUE)
                {
#if TTX_SUPPORT_DRCS
                    p_font = p_osd_page->p_drcs[(unicode >> 6) & 0x1F];

                    if(p_font != NULL)
                        draw_drcs_vsb(p_render
                                      , pen
                                      , p_ac->drcs_clut_offs
                                      , top
                                      , left
                                      , p_font
                                      , unicode & 0x3F
                                      , p_ac->size);
                    else /* shouldn't happen */
                        MT_ASSERT(0);
#else
                    draw_blank_vsb_page_mode(p_render
                                   , top
                                   , left
                                   , pen[0]
                                   , char_w, char_h,draw_page_param.pBuf,draw_page_param.pitch);
#endif
                }
                else
                {
                
                if(g_inital_page)  //fix bug 108790       	
                    draw_char_vsb_page_mode(p_render
                                  , pen
                                  , top
                                  , left
                                  , p_font
                                  , TTX_CHAR_NUM_PER_LINE, char_w, char_h
                                  , unicode_wstfont2_vsb (unicode, p_ac->italic)
                                  , p_ac->bold
                                  , p_ac->underline << (char_h - 1) /* cell row 9 */
                                  , p_ac->size,draw_page_param.pBuf,draw_page_param.pitch);
                }
            }

            left += char_w;
        }

        if(column == TTX_COLUMNS)
        {
            ttx_char_t  *p_tmp = p_ac - 1;

            if(p_tmp->opacity == TTX_TRANSPARENT_SPACE
                    || p_tmp->opacity == TTX_TRANSPARENT_FULL)
            {
                draw_blank_vsb_page_mode(p_render
                               , top
                               , left
                               , SCREEM_COLOR_INDEX
                               , char_w, char_h,draw_page_param.pBuf,draw_page_param.pitch);
            }
            else
            {
                draw_blank_vsb_page_mode(p_render
                               , top
                               , left
                               , 40 + TTX_BLACK
                               , char_w, char_h,draw_page_param.pBuf,draw_page_param.pitch);
            }
        }
        else
        {
            draw_blank_vsb_page_mode(p_render
                           , top
                           , left
                           , SCREEM_COLOR_INDEX
                           , char_w, char_h,draw_page_param.pBuf,draw_page_param.pitch);
        }
        //printf("top:%d, left:%d \n",top,left);
        left = p_render->page_x + char_w * first_column;
        top += char_h;
    }

    reflush_area.u32RowCount = top - reflush_area.u32Row;
    reflush_area.u32ColumnCount = p_render->page_x + char_w * last_column- reflush_area.u32Column;  

    ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_DRAWPAGE_END,&reflush_param);
    //tickms_new = mt_get_time_ms();
    //printf("%s -- tick: %lld \n ",__FUNCTION__,mt_get_time_interval_ms(tickms_new,tickms_old));    
#if 0
    p_disp_dev = dev_find_identifier(NULL, DEV_IDT_TYPE,
                                     SYS_DEV_TYPE_DISPLAY);
#ifndef WIN32
    if(subtile)
    {
        disp_layer_update_region(p_disp_dev, p_render->p_sub_hdl, NULL);
        ret = region_show(p_render->p_sub_hdl, TRUE);
        MT_ASSERT(SUCCESS == ret);
    }
    else
    {
        disp_layer_update_region(p_disp_dev, p_render->p_osd_hdl, NULL);
        ret = region_show(p_render->p_osd_hdl, TRUE);
        MT_ASSERT(SUCCESS == ret);
    }
#endif

#ifndef WIN32
#ifdef CACHE_ON
    extern void flush_dcache_all();
    flush_dcache_all();
#endif
#endif
#endif
}


void ttx_render_set_bg_transparent_vsb(ttx_render_vsb_t *p_render
                                       , ttx_osd_page_t *p_osd_page, u8 percent)
{
    RET_CODE ret __attribute__((unused)) = 0;
    u32 i = 0;
    u32 *p_c = p_render->clut + sizeof(p_osd_page->color_map) / 4;


    for(i = 0; i < (sizeof(p_osd_page->color_map) / 4); i ++, p_c ++)
    {
        *p_c = (*p_c & 0xffffff) | (0xff * percent / 100) << 24;
    }

	ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_SETPALETTE, p_render->clut);
//    ret = region_set_palette(p_render->p_osd_hdl, (u32 *)p_render->clut, 256);
    MT_ASSERT(SUCCESS == ret);
}

/*!
  The region in osd0 layer is created by upper user, we only configure and show.

  */

vbi_rc_t ttx_render_create_region_vsb(ttx_render_vsb_t *p_render, video_std_t std)
{
	u32 w = 0;
	u32 h = 0;
	MT_UNF_TTX_FILLRECT_S fillopt = {0};
	MT_UNF_TTX_PAGEAREA_S rect = {0};
	MT_UNF_TTX_REFRESHLAYER_S refresh_rect;
//	printf("\r\n ~~~~~~~~~~%s, %d", __FUNCTION__, __LINE__);

    p_render->video_std = std;

    if(p_render->font_size == TTX_FONT_HD)
    {
        w = 1280;
        h = 720;

    }
    else if(p_render->video_std == VID_STD_PAL)
    {
        w = 720;
        h = 576;
    }
    else
    {
        w = 720;
        h = 480;

    }	

    if(p_render->font_size == TTX_FONT_HD)
    {
        p_render->page_x = (w - TTX_PAGE_W_HD) / 2;
        p_render->page_y = (h - TTX_PAGE_H_HD) / 2;
        p_render->page_w = TTX_PAGE_W_HD;
        p_render->page_h = TTX_PAGE_H_HD;
    }
    else if(p_render->video_std == VID_STD_PAL)
    {
        p_render->page_x = (w - TTX_PAGE_W_PAL) / 2;
        p_render->page_y = (h - TTX_PAGE_H_PAL) / 2;
        p_render->page_w = TTX_PAGE_W_PAL;
        p_render->page_h = TTX_PAGE_H_PAL;
    }
    else
    {
        p_render->page_x = (w - TTX_PAGE_W_NTSC) / 2;
        p_render->page_y = (h - TTX_PAGE_H_NTSC) / 2;
        p_render->page_w = TTX_PAGE_W_NTSC;
        p_render->page_h = TTX_PAGE_H_NTSC;
    }
    p_render->region_w    = w;
    p_render->region_h    = h;
    p_render->clut[SCREEM_COLOR_INDEX] = 0x00000000;
	printf("\r\n page:%d, %d,%d,%d, %d,%d", p_render->page_x, p_render->page_y,p_render->page_w,p_render->page_h,
	p_render->region_w, p_render->region_h);

	fillopt.u32Color = SCREEM_COLOR_INDEX - 1;
	
	rect.u32Row = 0;
	rect.u32Column = 0;
	rect.u32RowCount = h;
	rect.u32ColumnCount = w;
	fillopt.pstPageArea = &rect;

	p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_FILLRECT, &fillopt);
		
	refresh_rect.pstPageArea = &rect;
	p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_REFRESH, &refresh_rect);

    return VBI_RC_SUCCESS;
}

/*!
  The region in osd0 layer is created by upper user, we only hide and configure.

  */
vbi_rc_t ttx_render_delete_region_vsb(ttx_render_vsb_t *p_render)
{

    RET_CODE    ret __attribute__((unused)) = 0;
	MT_UNF_TTX_FILLRECT_S fillopt = {0};
	MT_UNF_TTX_PAGEAREA_S rect = {0};
	MT_UNF_TTX_REFRESHLAYER_S refresh_rect;

    if(vbi_get_region_pos() == TTX_REGION_OUT_DECODER)
    {
        p_render->clut[SCREEM_COLOR_INDEX] = 0x00000000;
		
		fillopt.u32Color = SCREEM_COLOR_INDEX - 1;
		rect.u32Row = 0;
		rect.u32Column = 0;
		rect.u32RowCount = p_render->region_h;
		rect.u32ColumnCount = p_render->region_w;
		fillopt.pstPageArea = &rect;		

		ret = p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_FILLRECT, &fillopt);		
        MT_ASSERT(SUCCESS == ret);
		refresh_rect.pstPageArea = &rect;
		p_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_APP_REFRESH, &refresh_rect);
    }

    return VBI_RC_SUCCESS;
}

/*!
  The region in SUB layer is created, we will malloc buffer or use inner buffer.
  */
vbi_rc_t ttx_render_create_sub_region_vsb(ttx_render_vsb_t *p_render, video_std_t std)
{
#if 0
    RET_CODE            ret = 0;
    rect_size_t         rect_size;
    point_t             pos;
    u32 align = 0;
    u32 rgn_size = 0;
#ifndef WIN32
    chip_rev_t chip_rev = hal_get_chip_rev();
#else
    chip_rev_t chip_rev = IC_MAGIC;
#endif

    p_render->p_disp = (void *)dev_find_identifier(NULL
                       , DEV_IDT_TYPE, SYS_DEV_TYPE_DISPLAY);
    MT_ASSERT(NULL != p_render->p_disp);

    p_render->p_gpe = (void *)dev_find_identifier(NULL
                      , DEV_IDT_TYPE, SYS_DEV_TYPE_GPE_VSB);
    MT_ASSERT(NULL != p_render->p_gpe);

    p_render->video_std = std;
    if(p_render->font_size == TTX_FONT_HD)
    {
        pos.x = (TTX_PAGE_X_HD & (~3));
        pos.y =(TTX_PAGE_Y_HD & (~1));
        rect_size.w =(TTX_PAGE_W_HD & (~3));
        rect_size.h = (TTX_PAGE_H_HD & (~1));	//bug 75349
        //  rect_size.h = TTX_PAGE_H_HD + TTX_PAGE_H_HD % 2;
        p_render->page_x = 0;
        p_render->page_y = 0;
        p_render->page_w = rect_size.w;
        p_render->page_h = rect_size.h;
    }
    else if(p_render->video_std == VID_STD_PAL)
    {
        pos.x = (TTX_PAGE_X_PAL & (~3));
        pos.y = (TTX_PAGE_Y_PAL & (~1));
        rect_size.w = (TTX_PAGE_W_PAL & (~3));
        rect_size.h = (TTX_PAGE_H_PAL & (~1));
        //  rect_size.h = TTX_PAGE_H_PAL + TTX_PAGE_H_PAL % 2;
        //   rect_size.w = (TTX_PAGE_W_PAL /4) *4;
        //    rect_size.h = (TTX_PAGE_H_PAL /4) * 4;
        p_render->page_x = 0;
        p_render->page_y = 0;
        p_render->page_w = rect_size.w;
        p_render->page_h = rect_size.h;
    }
    else
    {
        pos.x = (TTX_PAGE_X_NTSC & (~3));
        pos.y = (TTX_PAGE_Y_NTSC & (~1));
        // pos.y = TTX_PAGE_Y_NTSC;
        rect_size.w = (TTX_PAGE_W_NTSC  & (~3));
        rect_size.h = (TTX_PAGE_H_NTSC & (~1));
        // rect_size.h = TTX_PAGE_H_NTSC + TTX_PAGE_H_NTSC % 2;

        //rect_size.w = (TTX_PAGE_W_NTSC /4) *4;
        //   rect_size.h = (TTX_PAGE_H_NTSC /4) * 4;
        p_render->page_x = 0;
        p_render->page_y = 0;
        p_render->page_w = rect_size.w;
        p_render->page_h = rect_size.h;
    }

    p_render->clut[SCREEM_COLOR_INDEX] = 0x00000000;

    p_render->p_sub_hdl = region_create(&rect_size, PIX_FMT_RGBPALETTE8);
    MT_ASSERT(NULL != p_render->p_sub_hdl);

    p_render->region_w    = rect_size.w;
    p_render->region_h    = rect_size.h;

    if((CHIP_IC_ID_MASK & chip_rev) == IC_WIZARDS)
    {
        ret = disp_calc_region_size(p_render->p_disp, DISP_LAYER_ID_OSD0,
                                    p_render->p_sub_hdl, &align, &rgn_size);
        MT_ASSERT(SUCCESS == ret);
    }
    if((CHIP_IC_ID_MASK & chip_rev) == IC_MAGIC)
    {
        ret = disp_calc_region_size(p_render->p_disp, DISP_LAYER_ID_SUBTITL,
                                    p_render->p_sub_hdl, &align, &rgn_size);
        MT_ASSERT(SUCCESS == ret);
    }
    if((CHIP_IC_ID_MASK & chip_rev) == IC_ENSEMBLE)
    {
        ret = disp_calc_region_size(p_render->p_disp, DISP_LAYER_ID_SUBTITL,
                                    p_render->p_sub_hdl, &align, &rgn_size);
        MT_ASSERT(SUCCESS == ret);
    }
    if(((CHIP_IC_ID_MASK & chip_rev) == IC_WARRIORS) ||
            ((CHIP_IC_ID_MASK & chip_rev) == IC_SONATA) ||
            ((CHIP_IC_ID_MASK & chip_rev) == IC_TANGO))
    {
        ret = disp_calc_region_size(p_render->p_disp, DISP_LAYER_ID_SUBTITL,
                                    p_render->p_sub_hdl, &align, &rgn_size);
        MT_ASSERT(SUCCESS == ret);
    }
    if(((CHIP_IC_ID_MASK & chip_rev) == IC_CONCERTO) || ((CHIP_IC_ID_MASK & chip_rev) == IC_SYMPHONY))
    {
        ret = disp_calc_region_size(p_render->p_disp, DISP_LAYER_ID_SUBTITL,
                                    p_render->p_sub_hdl, &align, &rgn_size);
        MT_ASSERT(SUCCESS == ret);
    }
    p_render->p_buf = NULL;

    //the buffer NULL means to use the configured buffer
    if((CHIP_IC_ID_MASK & chip_rev) == IC_WIZARDS)
    {
        ret = disp_layer_add_region(p_render->p_disp, DISP_LAYER_ID_OSD0,
                                    p_render->p_sub_hdl, &pos, p_render->p_buf);
        MT_ASSERT(SUCCESS == ret);
    }
    if((CHIP_IC_ID_MASK & chip_rev) == IC_MAGIC)
    {
        ret = disp_layer_add_region(p_render->p_disp, DISP_LAYER_ID_SUBTITL,
                                    p_render->p_sub_hdl, &pos, p_render->p_buf);
        MT_ASSERT(SUCCESS == ret);
    }
    if((CHIP_IC_ID_MASK & chip_rev) == IC_ENSEMBLE)
    {
        ret = disp_layer_add_region(p_render->p_disp, DISP_LAYER_ID_SUBTITL,
                                    p_render->p_sub_hdl, &pos, p_render->p_buf);
        MT_ASSERT(SUCCESS == ret);
    }
    if(((CHIP_IC_ID_MASK & chip_rev) == IC_CONCERTO) || ((CHIP_IC_ID_MASK & chip_rev) == IC_SYMPHONY))
    {
        ret = disp_layer_add_region(p_render->p_disp, DISP_LAYER_ID_SUBTITL,
                                    p_render->p_sub_hdl, &pos, p_render->p_buf);
        MT_ASSERT(SUCCESS == ret);
    }
    if(((CHIP_IC_ID_MASK & chip_rev) == IC_WARRIORS) ||
            ((CHIP_IC_ID_MASK & chip_rev) == IC_SONATA) ||
            ((CHIP_IC_ID_MASK & chip_rev) == IC_TANGO))
    {
        ret = disp_layer_add_region(p_render->p_disp, DISP_LAYER_ID_SUBTITL,
                                    p_render->p_sub_hdl, &pos, p_render->p_buf);
        //MT_ASSERT(SUCCESS == ret); bug 27483
        if(SUCCESS != ret)
        {
            region_delete(p_render->p_sub_hdl);
            return ret;
        }
    }
    ret = region_set_palette(p_render->p_sub_hdl, (u32 *)p_render->clut, 256);
    MT_ASSERT(SUCCESS == ret);

    ret = region_show(p_render->p_sub_hdl, FALSE);
    MT_ASSERT(SUCCESS == ret);

    if((CHIP_IC_ID_MASK & chip_rev) == IC_WIZARDS)
    {
        ret = disp_layer_show(p_render->p_disp, DISP_LAYER_ID_OSD0, TRUE);
        MT_ASSERT(SUCCESS == ret);
    }
    if((CHIP_IC_ID_MASK & chip_rev) == IC_MAGIC)
    {
        ret = disp_layer_show(p_render->p_disp, DISP_LAYER_ID_SUBTITL, TRUE);
        MT_ASSERT(SUCCESS == ret);
    }
    if((CHIP_IC_ID_MASK & chip_rev) == IC_ENSEMBLE)
    {
        ret = disp_layer_show(p_render->p_disp, DISP_LAYER_ID_SUBTITL, TRUE);
        MT_ASSERT(SUCCESS == ret);
    }
    if(((CHIP_IC_ID_MASK & chip_rev) == IC_WARRIORS) ||
            ((CHIP_IC_ID_MASK & chip_rev) == IC_SONATA) ||
            ((CHIP_IC_ID_MASK & chip_rev) == IC_TANGO))
    {
        ret = disp_layer_show(p_render->p_disp, DISP_LAYER_ID_SUBTITL, TRUE);
        MT_ASSERT(SUCCESS == ret);
    }
    if(((CHIP_IC_ID_MASK & chip_rev) == IC_CONCERTO) || ((CHIP_IC_ID_MASK & chip_rev) == IC_SYMPHONY))
    {
        ret = disp_layer_show(p_render->p_disp, DISP_LAYER_ID_SUBTITL, TRUE);
        MT_ASSERT(SUCCESS == ret);
    }
    //render in subtitle plane
    p_render->is_sub = TRUE;
#endif
    return VBI_RC_SUCCESS;
}

/*!
  The region in SUB layer is removed and deleted, we will free buffer or move back inner buffer.
  */
vbi_rc_t ttx_render_delete_sub_region_vsb(ttx_render_vsb_t *p_render)
{
#if 0
    RET_CODE            ret = 0;
#ifndef WIN32
    chip_rev_t chip_rev = hal_get_chip_rev();
#else
    chip_rev_t chip_rev = IC_MAGIC;
#endif

    ret = region_show(p_render->p_sub_hdl, FALSE);
    MT_ASSERT(SUCCESS == ret);
    if((CHIP_IC_ID_MASK & chip_rev) == IC_WIZARDS)
    {
        ret = disp_layer_remove_region(p_render->p_disp,
                                       DISP_LAYER_ID_OSD0,
                                       p_render->p_sub_hdl);
        MT_ASSERT(SUCCESS == ret);
    }
    if((CHIP_IC_ID_MASK & chip_rev) == IC_MAGIC)
    {
        ret = disp_layer_remove_region(p_render->p_disp,
                                       DISP_LAYER_ID_SUBTITL,
                                       p_render->p_sub_hdl);
        MT_ASSERT(SUCCESS == ret);
    }
    if((CHIP_IC_ID_MASK & chip_rev) == IC_ENSEMBLE)
    {
        ret = disp_layer_remove_region(p_render->p_disp,
                                       DISP_LAYER_ID_SUBTITL,
                                       p_render->p_sub_hdl);
        MT_ASSERT(SUCCESS == ret);
    }
    if(((CHIP_IC_ID_MASK & chip_rev) == IC_WARRIORS) ||
            ((CHIP_IC_ID_MASK & chip_rev) == IC_SONATA) ||
			((CHIP_IC_ID_MASK & chip_rev) == IC_CONCERTO) ||
            ((CHIP_IC_ID_MASK & chip_rev) == IC_TANGO) ||
            ((CHIP_IC_ID_MASK & chip_rev) == IC_SYMPHONY))
    {
        ret = disp_layer_remove_region(p_render->p_disp,
                                       DISP_LAYER_ID_SUBTITL,
                                       p_render->p_sub_hdl);
        MT_ASSERT(SUCCESS == ret);
    }

    ret = region_delete(p_render->p_sub_hdl);
    MT_ASSERT(SUCCESS == ret);
    p_render->p_sub_hdl = NULL;

    if(NULL != p_render->p_buf)
    {
        mtos_free(p_render->p_buf);
        p_render->p_buf = NULL;
    }
    //clear flag
    p_render->is_sub = FALSE;
#endif
    return VBI_RC_SUCCESS;
}
