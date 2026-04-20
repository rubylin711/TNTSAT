/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "string.h"
#include "mt_type.h"
#include "sys_define.h"
//#include "hal_dma.h"
//#include "mtos_printk.h"

#include "ttx_lang_vsb.h"
#include "ttx_bcd_vsb.h"
#include "ttx_hamm_vsb.h"
#include "ttx_dec_vsb.h"
#include "ttx_format_vsb.h"
#include "ttx_db_vsb.h"

#define ELEMENTS(array) (sizeof((array)) / sizeof((array)[0]))

/*!
  enable debug
  */
#ifdef DRV_TTX_DEBUG
/*!
  ABC
  */
#define TTX_PRINTF    OS_PRINTF
#else
#ifdef WIN32
/*!
  ABC
  */
#define TTX_PRINTF
#else
/*!
  ABC
  */
#define TTX_PRINTF(x)    do{} while(0)
#endif
#endif

extern const u8 _vbi_bit_reverse_vsb[256];
extern const u8 _vbi_hamm8_fwd_vsb[16];
extern const s8 _vbi_hamm8_inv_vsb[256];
extern const u8 _vbi_hamm24_fwd_0_vsb[256];
extern const u8 _vbi_hamm24_fwd_1_vsb[256];
extern const u8 _vbi_hamm24_fwd_2_vsb[4];
extern const s8 _vbi_hamm24_inv_par_vsb[3][256];
extern const u8 _vbi_hamm24_inv_d1_d4_vsb[64];
extern const s32 _vbi_hamm24_inv_err_vsb[64];

/*!
   TODO: this function is odd parity decode

   \param[in] c
  */
static inline s32 vbi_unpar8_vsb(u32 c)
{
    if((_vbi_hamm24_inv_par_vsb[0][(u8)c] & 32) != 0)
    {
        return c & 127;
    }
    else
    {
        /*!
           The idea is to OR results together to find a parity
           error in a sequence, rather than a test and branch on
           each byte.
          */
        return -1;
    }
}

static const ttx_color_t flof_link_col_vsb[4] =
{ TTX_RED, TTX_GREEN, TTX_YELLOW, TTX_CYAN};

static void flush_column_vsb(ttx_raw_t *p_raw_page
                      , ttx_char_t *p_ch, ttx_object_type_t type
                      , ttx_char_t *p_attribute, ttx_char_mask_t *p_mask
                      , s32 row, s32 col_no)
{
    s32 i = 0;

    if(row >= TTX_ROWS)
        return;

    if(type == TTX_OBJ_PASSIVE && p_mask->unicode == 0)
        return;

    for(i = 0; i < col_no;)
    {
        ttx_char_t c;

        if(i > 39)
            break;

        c = p_ch[i];

        if(p_mask->underline != 0)
        {
            s32 u = p_attribute->underline;

            if(p_mask->unicode == 0)
                p_attribute->unicode = c.unicode;

            if(vbi_is_gfx_vsb(p_attribute->unicode) == TRUE)
            {
                if(u != 0)
                    p_attribute->unicode &= ~0x20;  /* separated */
                else
                    p_attribute->unicode |= 0x20;   /* contiguous */
                p_mask->unicode = ~0;
                u = 0;
            }

            c.underline = u;
        }

        if(p_mask->foreground != 0)
            c.foreground = (p_attribute->foreground != TTX_TRANSPARENT_BLACK)
                           ? p_attribute->foreground : (p_mask->row_transparent)
                           ? TTX_TRANSPARENT_BLACK : p_mask->row_color;

        if(p_mask->background != 0)
            c.background = (p_attribute->background != TTX_TRANSPARENT_BLACK)
                           ? p_attribute->background : (p_mask->row_transparent)
                           ? TTX_TRANSPARENT_BLACK : p_mask->row_color;

        if(p_mask->invert != 0)
        {
            s32 t = c.foreground;

            c.foreground = c.background;
            c.background = t;
        }

        if(p_mask->opacity != 0)
            c.opacity = p_attribute->opacity;

        if(p_mask->flash != 0)
            c.flash = p_attribute->flash;

        if(p_mask->conceal != 0)
            c.conceal = p_attribute->conceal;

        if(p_mask->unicode != 0)
        {
            c.unicode = p_attribute->unicode;
            p_mask->unicode = 0;

            if(p_mask->size != 0)
                c.size = p_attribute->size;

            switch(c.size)
            {
            case TTX_DOUBLE_HEIGHT:
                if(row < TTX_ROWS - 1)
                    p_ch[i + TTX_COLUMNS].unicode = c.unicode;

                break;

            case TTX_DOUBLE_SIZE:
                if(row < TTX_ROWS - 1 && i < TTX_COLUMNS - 1)
                {
                    p_ch[i + 1].unicode               = c.unicode;
                    p_ch[i + TTX_COLUMNS].unicode     = c.unicode;
                    p_ch[i + TTX_COLUMNS + 1].unicode = c.unicode;
                }

                break;

            case TTX_DOUBLE_WIDTH:
                if(i < TTX_COLUMNS - 1)
                {
                    p_ch[i + 1].unicode = c.unicode;
                }
                break;

            default:
                break;
            }
        }

        p_ch[i] = c;

        if(type == TTX_OBJ_PASSIVE)
            break;

        i ++;

        if(type != TTX_OBJ_PASSIVE && type != TTX_OBJ_ADAPTIVE)
        {
            s32 raw = 0;

            raw = (row == 0 && i < 9)
                  ? 0x20 : vbi_unpar8_vsb(p_raw_page->data.lop.raw[row][i - 1]);

            /* set-after spacing attributes cancelling non-spacing */
            if((raw >= 0 && raw <= 0x07)         /* alpha + foreground color  */
                    || (raw >= 0x10 && raw <= 0x17)) /* mosaic + foreground color */
            {
                p_mask->foreground = 0;
                p_mask->conceal    = 0;
            }
            else if(raw == 0x08)
            {
                p_mask->flash = 0;
            }
            else if(raw == 0x0A || raw == 0x0B)    /* end box || start box    */
            {
                if(i < TTX_COLUMNS
                        && vbi_unpar8_vsb(p_raw_page->data.lop.raw[row][i]) == raw)
                    p_mask->opacity = 0;
            }
            else if(raw == 0x0D         /* double height    */
                    || raw == 0x0E          /* double width     */
                    || raw == 0x0F)         /* double size      */
            {
                p_mask->size = 0;
            }

            if(i > 39)
                break;

            raw = (row == 0 && i < 8)
                  ? 0x20 : vbi_unpar8_vsb(p_raw_page->data.lop.raw[row][i]);

            /* set-at spacing attributes cancelling non-spacing */
            if(raw == 0x09)            /* steady       */
                p_mask->flash = 0;
            else if(raw == 0x0C)       /* normal size  */
                p_mask->size = 0;
            else if(raw == 0x18)       /* conceal      */
                p_mask->conceal = 0;
            /*
             *    Non-spacing underlined/separated display attribute
             *    cannot be cancelled by a subsequent spacing attribute.
             */
            else if(raw == 0x1C || raw == 0x1D)
            {
                /* black background || new background    */
                p_mask->background = 0;
            }
        }
    }
}

static vbi_rc_t resolve_obj_address_vsb(ttx_decoder_t *p_dec, ttx_object_type_t type
                                        , s16 page_no, s32 address, ttx_page_function_t function
                                        , ttx_raw_t **pp_page, s32 *p_remaining, ttx_triplet_t **pp_next_trip)
{
    vbi_rc_t        rc;
    s32             packet = 0, pointer = 0;
    s16             s1 = 0;
    ttx_raw_t       *p_page = NULL;
    ttx_triplet_t   *p_trip = NULL;
    s32             i = 0;

    *pp_page      = NULL;
    *p_remaining  = 0;
    *pp_next_trip = NULL;

    s1     = (s16)address & 0x0f;
    packet = ((address >> 7) & 3);
    i      = ((address >> 5) & 3) * 3 + type;

    TTX_PRINTF(("TTX:  obj invocation, source page %03x/%04x, "
                "pointer packet %d triplet %d\n", page_no, s1, packet + 1, i));

    rc = ttx_get_raw_page_vsb(p_dec, page_no, s1, 0xffff, &p_page);

    if(rc != VBI_RC_SUCCESS || !p_page)
    {
        TTX_PRINTF(("TTX:  page not cached\n"));
        return rc;
    }

    if(p_page->function == TTX_PAGE_FUNC_UNKOWN)
    {
        rc = ttx_dec_convert_page_vsb(p_dec, p_page, function);
        if(rc != VBI_RC_SUCCESS)
        {
            TTX_PRINTF(("TTX!:    no g/pop page or hamming error\n"));
            return rc;
        }
    }
    else if(p_page->function == TTX_PAGE_FUNC_POP)
    {
        p_page->function = function;
    }
    else if(p_page->function != function)
    {
        TTX_PRINTF(("TTX:  source page wrong function %d, expected %d\n"
                    , p_page->function, function));
        return VBI_RC_FAILED;
    }

    pointer
        = p_page->data.pop.pointer[packet * 24 + i * 2 + ((address >> 4) & 1)];

    if(pointer > 506)
    {
        TTX_PRINTF(("TTX!:    triplet pointer out of bounds (%d)\n", pointer));
        return VBI_RC_FAILED;
    }

#if 0
    {
        packet = (pointer / 13) + 3;

        if(packet <= 25)
            TTX_PRINTF(("TTX:  object start in packet %d,"
                        ," triplet %d (pointer %d)\n", packet, pointer % 13, pointer));
        else
            TTX_PRINTF(("TTX:  object start in packet 26/%d,"
                        , " triplet %d (pointer %d)\n"
                        , packet - 26, pointer % 13, pointer));
    }
#endif

    p_trip       = p_page->data.pop.triplet + pointer;
    *p_remaining = ELEMENTS(p_page->data.pop.triplet) - (pointer + 1);

    TTX_PRINTF(("TTX:  object define: ad 0x%02x mo 0x%04x dat %d=0x%x\n",
                p_trip->address, p_trip->mode, p_trip->data));

    address ^= p_trip->address << 7;
    address ^= p_trip->data;

    if(p_trip->mode != (type + 0x14) || (address & 0x1FF))
    {
        TTX_PRINTF(("TTX:  no object definition\n"));
        return VBI_RC_FAILED;
    }

    *pp_page      = p_page;
    *pp_next_trip = p_trip + 1;

    return VBI_RC_SUCCESS;
}

static void ttx_set_screen_color_vsb(ttx_decoder_t *p_dec
                                     , ttx_osd_page_t *p_osd_page, s32 control_bits, s32 color)
{
    p_osd_page->screen_color = color;

    if(color == TTX_TRANSPARENT_BLACK
            || (control_bits & (C5_NEWSFLASH | C6_SUBTITLE)))
        p_osd_page->screen_opacity = TTX_TRANSPARENT_SPACE;
    else
        p_osd_page->screen_opacity = TTX_OPAQUE;
}

static void ttx_get_character_set_vsb(ttx_decoder_t *p_dec
                                      , ttx_extension_t *p_ext, ttx_raw_t *p_page
                                      , const ttx_font_descr_t **pp_font)
{
    s32 i = 0;


    pp_font[0] = p_dec->p_font_descr;
    pp_font[1] = p_dec->p_font_descr;

    for(i = 0; i < 2; i ++)
    {
        u8 charset_code = p_ext->charset_code[i];

        if(VALID_CHARACTER_SET(p_dec, charset_code) == TRUE)
        {
            pp_font[i] = p_dec->p_font_descr + charset_code;
        }
        charset_code = (charset_code & ~7) + p_page->national;
        if(VALID_CHARACTER_SET(p_dec, charset_code) == TRUE)
        {
            pp_font[i] = p_dec->p_font_descr + charset_code;
        }
    }
}

static void ttx_post_enhance_vsb(ttx_osd_page_t *p_page)
{
    s32 last_row = TTX_ROWS - 2;
    ttx_char_t ac, *p_ac = NULL;
    s32 column = 0, row = 0;

    p_ac = p_page->text;

    for(row = 0; row <= last_row; row ++)
    {
        for(column = 0; column < TTX_COLUMNS; p_ac ++, column ++)
        {
            if(p_ac->opacity == TTX_TRANSPARENT_SPACE
                    || (p_ac->foreground == TTX_TRANSPARENT_BLACK
                        && p_ac->background == TTX_TRANSPARENT_BLACK))
            {
                p_ac->opacity = TTX_TRANSPARENT_SPACE;
                p_ac->unicode = 0x0020;
            }
            else if(p_ac->background == TTX_TRANSPARENT_BLACK)
            {
                p_ac->opacity = TTX_SEMI_TRANSPARENT;
            }
            /* transparent foreground not implemented */

            switch(p_ac->size)
            {
            case TTX_NORMAL_SIZE:
                if((p_ac[1].size == TTX_OVER_TOP
                        || p_ac[1].size == TTX_OVER_BOTTOM)
                        && column < 39)
                {
                    p_ac[1].unicode = 0x0020;
                    p_ac[1].size    = TTX_NORMAL_SIZE;
                }
                if((p_ac[TTX_COLUMNS].size == TTX_DOUBLE_HEIGHT2)
                        && column < 39)
                {
                    p_ac[TTX_COLUMNS].unicode = 0x0020;
                    p_ac[TTX_COLUMNS].size    = TTX_NORMAL_SIZE;
                }
                break;

            case TTX_DOUBLE_HEIGHT:
                if(row < last_row)
                {
                    ac                = p_ac[0];
                    ac.size           = TTX_DOUBLE_HEIGHT2;
                    p_ac[TTX_COLUMNS] = ac;
                }
                break;

            case TTX_DOUBLE_SIZE:
                if(row < last_row && column < 39)
                {
                    ac                    = p_ac[0];
                    ac.size               = TTX_DOUBLE_SIZE2;
                    p_ac[TTX_COLUMNS]     = ac;
                    ac.size               = TTX_OVER_BOTTOM;
                    p_ac[TTX_COLUMNS + 1] = ac;
                    ac.size               = TTX_OVER_TOP;
                    p_ac[1]               = ac;
                }
                break;

            case TTX_DOUBLE_WIDTH:
                if(column < 39)
                {
                    ac      = p_ac[0];
                    ac.size = TTX_OVER_TOP;
                    p_ac[1] = ac;
                }
                break;

            default:
                break;
            }
        }
    }
}

static vbi_rc_t ttx_enhance_vsb(ttx_decoder_t   *p_dec,
                                ttx_magazine_t      *p_mag,
                                ttx_extension_t     *p_ext,
                                ttx_osd_page_t      *p_osd_page,
                                ttx_raw_t           *p_raw_page,
                                ttx_object_type_t   type,
                                ttx_triplet_t       *p_triplet,
                                s32                 max_triplets,
                                s32                 inv_row,
                                s32                 inv_column,
                                MT_BOOL                header_only)
{
    ttx_char_t ac, *p_ac = NULL;
    ttx_char_mask_t mac;
    s32 active_column = 0, active_row = 0;
    s32 offset_column = 0, offset_row = 0;
    s32 next_row_color = 0;
    const ttx_font_descr_t *p_font = NULL;

#if TTX_SUPPORT_DRCS
    s32 drcs_s1[2];
#endif

    active_column = 0;
    active_row    = 0;

    p_ac = &p_osd_page->text[(inv_row + 0) * TTX_COLUMNS];

    offset_column = 0;
    offset_row    = 0;

    next_row_color      =
        mac.row_color       = p_ext->def_row_color;
    mac.row_transparent = 0;

#if TTX_SUPPORT_DRCS
    drcs_s1[0] = 0;    /* global */
    drcs_s1[1] = 0;    /* normal */
#endif

    memset(&ac, 0, sizeof(ac));
    memset(&mac, 0, sizeof(mac));

    if(type == TTX_OBJ_PASSIVE)
    {
        ac.foreground = TTX_WHITE;
        ac.background = TTX_BLACK;
        ac.opacity    = p_osd_page->page_opacity[1];

        mac.foreground = ~0;
        mac.background = ~0;
        mac.opacity    = ~0;
        mac.size       = ~0;
        mac.underline  = ~0;
        mac.conceal    = ~0;
        mac.flash      = ~0;
        mac.invert     = ~0;
    }

    p_font = p_osd_page->p_font[0];

    for(; max_triplets > 0; p_triplet ++, max_triplets --)
    {
        if(p_triplet->address >= TTX_COLUMNS)
        {
            /*
             *  Row address triplets
             */
            s32 s      = p_triplet->data >> 5;
            s32 row    = (p_triplet->address - TTX_COLUMNS)
                         ? (p_triplet->address - TTX_COLUMNS) : (TTX_ROWS - 1);
            s32 column = 0;

            switch(p_triplet->mode)
            {
            case 0x00:        /* full screen color */
                if(p_dec->max_level >= TTX_DEC_LEVEL_2P5 && s == 0
                        && type <= TTX_OBJ_ACTIVE)
                    ttx_set_screen_color_vsb(p_dec, p_osd_page
                                             , p_raw_page->control_bits, p_triplet->data & 0x1F);
                break;

            case 0x01:        /* full row color */
            case 0x04:        /* full row color */
            case 0x07:        /* address display row 0 */
                if(p_triplet->mode == 0x07)
                {
                    if(p_triplet->address != 0x3F)
                        break; /* reserved, no position */

                    row = 0;
                }

                if(p_triplet->mode == 0x07 || p_triplet->mode == 0x01)
                {
                    mac.row_color = next_row_color;

                    if(s == 0)
                    {
                        mac.row_color  = p_triplet->data & 0x1F;
                        next_row_color = p_ext->def_row_color;
                    }
                    else if(s == 3)
                    {
                        mac.row_color  =
                            next_row_color = p_triplet->data & 0x1F;
                    }
                }

                if(p_triplet->mode == 0x04)
                {
                    if(p_dec->max_level >= TTX_DEC_LEVEL_2P5)
                    {
                        if(p_triplet->data >= TTX_COLUMNS)
                            break; /* reserved */

                        column = p_triplet->data;
                    }

                    if(row > active_row)
                        mac.row_color = next_row_color;
                }

                /*
                **  set Active Position
                */
                if(header_only && row > 0)
                {
                    for(; max_triplets > 1; p_triplet ++, max_triplets --)
                    {
                        if(p_triplet[1].address >= TTX_COLUMNS)
                        {
                            if(p_triplet[1].mode == 0x07)
                                break;
                            else if((u32) p_triplet[1].mode >= 0x1F)
                            {
                                if(type == TTX_OBJ_PASSIVE
                                        || type == TTX_OBJ_ADAPTIVE)
                                {
                                    flush_column_vsb(p_raw_page
                                                     , p_ac + inv_column + active_column
                                                     , type
                                                     , &ac
                                                     , &mac
                                                     , inv_row + active_row
                                                     , 1);

                                    active_column ++;
                                }
                                else
                                {
                                    flush_column_vsb(p_raw_page
                                                     , p_ac + inv_column + active_column
                                                     , type
                                                     , &ac
                                                     , &mac
                                                     , inv_row + active_row
                                                     , TTX_COLUMNS - active_column);

                                    active_column = TTX_COLUMNS;
                                }

                                if(type != TTX_OBJ_PASSIVE)
                                    memset(&mac, 0, sizeof(mac));

                                TTX_PRINTF(("TTX:  enh terminated %02x\n"
                                            , p_triplet->mode));
                                return VBI_RC_SUCCESS;
                            }
                        }
                    }
                    break;
                }

                TTX_PRINTF(("TTX:  enh set_active row %d col %d\n"
                            , row, column));

                if(row > active_row)
                {
                    if(type == TTX_OBJ_PASSIVE
                            || type == TTX_OBJ_ADAPTIVE)
                    {
                        flush_column_vsb(p_raw_page
                                         , p_ac + inv_column + active_column
                                         , type
                                         , &ac
                                         , &mac
                                         , inv_row + active_row
                                         , 1);

                        active_column ++;
                    }
                    else
                    {
                        flush_column_vsb(p_raw_page
                                         , p_ac + inv_column + active_column
                                         , type
                                         , &ac
                                         , &mac
                                         , inv_row + active_row
                                         , TTX_COLUMNS - active_column);

                        active_column = TTX_COLUMNS;
                    }

                    if(type != TTX_OBJ_PASSIVE)
                        memset(&mac, 0, sizeof(mac));
                }

                active_row    = row;
                active_column = column;

                p_ac =
                    &p_osd_page->text[(inv_row + active_row) * TTX_COLUMNS];
                break;

            case 0x10:        /* origin modifier */
                if(p_dec->max_level < TTX_DEC_LEVEL_2P5)
                    break;

                if(p_triplet->data >= 72)
                    break; /* invalid */

                offset_column = p_triplet->data;
                offset_row    = p_triplet->address - TTX_COLUMNS;

                TTX_PRINTF(("enh origin modifier col %+d row %+d\n",
                            offset_column, offset_row));
                break;

            case 0x11:
            case 0x12:
            case 0x13:    /* object invocation */
            {
                s32                 source = (p_triplet->address >> 3) & 3;
                ttx_object_type_t   new_type
                    = (ttx_object_type_t)(p_triplet->mode & 3);
                ttx_raw_t           *p_trip_page = NULL;
                ttx_triplet_t       *p_trip      = NULL;
                s32                 remaining_triplets = 0;

                if(p_dec->max_level < TTX_DEC_LEVEL_2P5)
                    break;

                TTX_PRINTF(("TTX:  object invocation, source %d type %d\n"
                            , source, new_type));

                if(new_type <= type)    /* 13.2++ */
                {
                    TTX_PRINTF(("TTX:  objetc type priority violation\n"));
                    break;
                }

                if(source == 0) /* illegal */
                    break;

                if(source == 1)    /* local */
                {
                    s32 dc      = (p_triplet->data >> 4)
                                  + ((p_triplet->address & 1) << 4);
                    s32 trip_no = p_triplet->data & 15;

                    if(type != TTX_OBJ_LOCAL_ENH_DATA || trip_no > 12)
                        break; /* invalid */

                    TTX_PRINTF(("TTX:  local object %d/%d\n", dc, trip_no));

                    if((p_raw_page->x26_designations & 1) == 0)
                    {
                        TTX_PRINTF(("TX:  no packet %d\n", dc));
                        return VBI_RC_FAILED;
                    }

                    p_trip             = p_raw_page->data.enh_lop.trip
                                         + dc * 13 + trip_no;
                    remaining_triplets
                        = ELEMENTS(p_raw_page->data.enh_lop.trip)
                          - (dc * 13 + trip_no);
                }
                else /* global / public */
                {
                    ttx_page_function_t function;
                    s16                 page_no = 0, i = 0;

                    if(source == 3)    /*    global    */
                    {
                        function = TTX_PAGE_FUNC_GPOP;
                        page_no  = p_raw_page->data.lop.link[24].page_no;

                        if(TTX_NO_PAGE(page_no) == TRUE)
                        {
                            page_no = p_mag->pop_link[1][0].page_no;
                            if(p_dec->max_level < TTX_DEC_LEVEL_3P5
                                    || TTX_NO_PAGE(page_no) == TRUE)
                                page_no = p_mag->pop_link[0][0].page_no;
                        }
                        else
                            TTX_PRINTF(("TTX:  X/27/4 GPOP overrides MOT\n"));
                    }
                    else    /*    public */
                    {
                        function = TTX_PAGE_FUNC_POP;
                        page_no  = p_raw_page->data.lop.link[25].page_no;

                        if(TTX_NO_PAGE(page_no) == TRUE)
                        {
                            i = p_mag->pop_lut[p_raw_page->page_no & 0xFF];
                            if(i == 0)
                            {
                                TTX_PRINTF(("TTX:  MOT pop_lut empty\n"));
                                return VBI_RC_INVALID_DATA;
                            }

                            page_no = p_mag->pop_link[0][i + 8].page_no;
                            if(p_dec->max_level < TTX_DEC_LEVEL_3P5
                                    || TTX_NO_PAGE(page_no) == TRUE)
                                page_no = p_mag->pop_link[0][i + 0].page_no;
                        }
                        else
                            TTX_PRINTF(("TTX:  X/27/4 POP overrides MOT\n"));
                    }

                    if(TTX_NO_PAGE(page_no) == TRUE)
                    {
                        TTX_PRINTF(("TTX:  dead MOT link %d\n", i));
                        return VBI_RC_INVALID_DATA; /* has no link (yet) */
                    }

                    TTX_PRINTF(("TTX:  %s obj\n", (source == 3)
                                ? "global" : "public"));

                    resolve_obj_address_vsb(p_dec, new_type, page_no
                                            , (p_triplet->address << 7) + p_triplet->data
                                            , function, &p_trip_page
                                            , &remaining_triplets, &p_trip);

                    if(p_trip == NULL)
                        return VBI_RC_FAILED;
                }

                row    = inv_row + active_row;
                column = inv_column + active_column;

                if(ttx_enhance_vsb(p_dec, p_mag, p_ext, p_osd_page, p_raw_page
                                   , new_type, p_trip, remaining_triplets
                                   , row + offset_row, column + offset_column
                                   , header_only) != VBI_RC_SUCCESS)
                {
                    p_trip_page = NULL;
                    return VBI_RC_FAILED;
                }

                TTX_PRINTF(("TTX:  object done\n"));

                p_trip_page   = NULL;
                offset_row    = 0;
                offset_column = 0;

                break;
            }
            case 0x14:        /* reserved */
                break;

            case 0x15:
            case 0x16:
            case 0x17:    /* object definition */
                if(type == TTX_OBJ_PASSIVE
                        || type == TTX_OBJ_ADAPTIVE)
                {
                    flush_column_vsb(p_raw_page
                                     , p_ac + inv_column + active_column
                                     , type
                                     , &ac
                                     , &mac
                                     , inv_row + active_row
                                     , 1);

                    active_column ++;
                }
                else
                {
                    flush_column_vsb(p_raw_page
                                     , p_ac + inv_column + active_column
                                     , type
                                     , &ac
                                     , &mac
                                     , inv_row + active_row
                                     , TTX_COLUMNS - active_column);

                    active_column = TTX_COLUMNS;
                }

                if(type != TTX_OBJ_PASSIVE)
                    memset(&mac, 0, sizeof(mac));

                TTX_PRINTF(("TTX:  enh object definition 0x%02x 0x%02x\n"
                            , p_triplet->mode, p_triplet->data));
                return VBI_RC_SUCCESS;

            case 0x18:        /* drcs mode */
#if TTX_SUPPORT_DRCS
                TTX_PRINTF(("TTX:  enh DRCS mode 0x%02x\n"
                            , p_triplet->data));

                drcs_s1[p_triplet->data >> 6] = p_triplet->data & 0x0f;
#endif
                break;

            case 0x1F:        /* termination marker */
                if(type == TTX_OBJ_PASSIVE
                        || type == TTX_OBJ_ADAPTIVE)
                {
                    flush_column_vsb(p_raw_page
                                     , p_ac + inv_column + active_column
                                     , type
                                     , &ac
                                     , &mac
                                     , inv_row + active_row
                                     , 1);

                    active_column ++;
                }
                else
                {
                    flush_column_vsb(p_raw_page
                                     , p_ac + inv_column + active_column
                                     , type
                                     , &ac
                                     , &mac
                                     , inv_row + active_row
                                     , TTX_COLUMNS - active_column);

                    active_column = TTX_COLUMNS;
                }

                if(type != TTX_OBJ_PASSIVE)
                    memset(&mac, 0, sizeof(mac));

                TTX_PRINTF(("TTX:  enh terminated %02x\n"
                            , p_triplet->mode));
                return VBI_RC_SUCCESS;

            case 0x02:/* reserved */
            case 0x03:/* reserved */
            case 0x05:/* reserved */
            case 0x06:/* reserved */
            case 0x08:/* PDC-Country of Origin and Programme Source */
            case 0x09:/* PDC-Month and Day */
            case 0x0A:/* PDC-Cursor Row and Announced Starting Time Hours */
            case 0x0B:/* PDC-Cursor Row and Announced Finishing Time Hours*/
            case 0x0C:/* PDC-Cursor Row and Local Time Offset */
            case 0x0D:/* PDC-Series Identifier and Series Code */
            case 0x0E:/* reserved */
            case 0x0F:/* reserved */
            default:    /* 0x19 ... 0x1E     reserved */
                break;
            }
        }
        else
        {
            /*
             *  Column address triplets
             */
            s32 s       = p_triplet->data >> 5;
            s32 column  = p_triplet->address;
            s32 unicode = 0;

            switch(p_triplet->mode)
            {
            case 0x00:        /* foreground color */
                if(p_dec->max_level >= TTX_DEC_LEVEL_2P5 && s == 0)
                {
                    if(column > active_column)
                    {
                        flush_column_vsb(p_raw_page
                                         , p_ac + inv_column + active_column
                                         , type
                                         , &ac
                                         , &mac
                                         , inv_row + active_row
                                         , column - active_column);

                        active_column = column;
                    }

                    ac.foreground  = p_triplet->data & 0x1F;
                    mac.foreground = ~0;

                    TTX_PRINTF(("TTX:  enh col %d foreground %d\n"
                                , active_column, ac.foreground));
                }
                break;

            case 0x01:        /* G1 block mosaic character */
                if(p_dec->max_level >= TTX_DEC_LEVEL_2P5)
                {
                    if(column > active_column)
                    {
                        flush_column_vsb(p_raw_page
                                         , p_ac + inv_column + active_column
                                         , type
                                         , &ac
                                         , &mac
                                         , inv_row + active_row
                                         , column - active_column);

                        active_column = column;
                    }

                    if((p_triplet->data & 0x20) != 0)
                    {
                        /* G1 contiguous */
                        unicode     = 0xEE00 + p_triplet->data;
                        ac.unicode  = unicode;
                        mac.unicode = ~0;
                    }
                    else if(p_triplet->data >= 0x40)
                    {
                        unicode     = vbi_teletext_unicode_vsb(p_font->G0
                                                               , CHAR_SUBSET_NONE, p_triplet->data);
                        ac.unicode  = unicode;
                        mac.unicode = ~0;
                    }
                }
                break;

            case 0x0B:  /* G3 smooth mosaic or line drawing character */
                if(p_dec->max_level < TTX_DEC_LEVEL_2P5)
                    break;

                /* fall through */

            case 0x02:  /* G3 smooth mosaic or line drawing character */
                if(p_triplet->data >= 0x20)
                {
                    if(column > active_column)
                    {
                        flush_column_vsb(p_raw_page
                                         , p_ac + inv_column + active_column
                                         , type
                                         , &ac
                                         , &mac
                                         , inv_row + active_row
                                         , column - active_column);

                        active_column = column;
                    }

                    unicode     = 0xEF00 + p_triplet->data;
                    ac.unicode  = unicode;
                    mac.unicode = ~0;
                }
                break;

            case 0x03:  /* background color */
                if(p_dec->max_level >= TTX_DEC_LEVEL_2P5 && s == 0)
                {
                    if(column > active_column)
                    {
                        flush_column_vsb(p_raw_page
                                         , p_ac + inv_column + active_column
                                         , type
                                         , &ac
                                         , &mac
                                         , inv_row + active_row
                                         , column - active_column);

                        active_column = column;
                    }

                    ac.background  = p_triplet->data & 0x1F;
                    mac.background = ~0;

                    TTX_PRINTF(("TTX:  enh col %d background %d\n"
                                , active_column, ac.background));
                }
                break;

            case 0x04:  /* reserved */
            case 0x05:  /* reserved */
            case 0x06:  /* PDC - Cursor Column and Announced Starting */
                /* and Finishing Time Minutes */
                break;

            case 0x07:  /* additional flash functions */
                if(p_dec->max_level >= TTX_DEC_LEVEL_2P5)
                {
                    if(column > active_column)
                    {
                        flush_column_vsb(p_raw_page
                                         , p_ac + inv_column + active_column
                                         , type
                                         , &ac
                                         , &mac
                                         , inv_row + active_row
                                         , column - active_column);

                        active_column = column;
                    }

                    /*
                     *  Only one flash function (if any) implemented:
                     *  Mode 1 - Normal flash to background color
                     *  Rate 0 - Slow rate (1 Hz)
                     */
                    ac.flash  = !!(p_triplet->data & 3);
                    mac.flash = ~0;

                    TTX_PRINTF(("TTX:  enh col %d flash 0x%02x\n"
                                , active_column, p_triplet->data));
                }
                break;

            case 0x08:  /* modified G0 and G2 character set designation */
                if(p_dec->max_level >= TTX_DEC_LEVEL_2P5)
                {
                    if(column > active_column)
                    {
                        flush_column_vsb(p_raw_page
                                         , p_ac + inv_column + active_column
                                         , type
                                         , &ac
                                         , &mac
                                         , inv_row + active_row
                                         , column - active_column);

                        active_column = column;
                    }

                    if(VALID_CHARACTER_SET(p_dec, p_triplet->data) == TRUE)
                        p_font = p_dec->p_font_descr + p_triplet->data;
                    else
                        p_font = p_osd_page->p_font[0];

                    TTX_PRINTF(("TTX:  enh col %d modify character set %d\n"
                                , active_column, p_triplet->data));
                }
                break;

            case 0x09:        /* G0 character */
                if(p_dec->max_level >= TTX_DEC_LEVEL_2P5
                        && p_triplet->data >= 0x20)
                {
                    if(column > active_column)
                    {
                        flush_column_vsb(p_raw_page
                                         , p_ac + inv_column + active_column
                                         , type
                                         , &ac
                                         , &mac
                                         , inv_row + active_row
                                         , column - active_column);

                        active_column = column;
                    }

                    unicode     = vbi_teletext_unicode_vsb(p_font->G0
                                                           , CHAR_SUBSET_NONE, p_triplet->data);
                    ac.unicode  = unicode;
                    mac.unicode = ~0;
                }

                break;

            case 0x0A:        /* reserved */
                break;

            case 0x0C:        /* display attributes */
                if(p_dec->max_level < TTX_DEC_LEVEL_2P5)
                    break;

                if(column > active_column)
                {
                    flush_column_vsb(p_raw_page
                                     , p_ac + inv_column + active_column
                                     , type
                                     , &ac
                                     , &mac
                                     , inv_row + active_row
                                     , column - active_column);

                    active_column = column;
                }

                ac.size  = ((p_triplet->data & 0x40) ? TTX_DOUBLE_WIDTH : 0)
                           + ((p_triplet->data & 1) ? TTX_DOUBLE_HEIGHT : 0);
                mac.size = ~0;

                if((p_raw_page->control_bits & (C5_NEWSFLASH | C6_SUBTITLE))
                        != 0)
                {
                    if((p_triplet->data & 2) != 0)
                    {
                        ac.opacity = TTX_SEMI_TRANSPARENT;
                    }
                    else
                    {
                        ac.opacity = p_osd_page->page_opacity[1];
                    }
                    mac.opacity = ~0;
                }
                else
                {
                    mac.row_transparent = (p_triplet->data & 2) ? ~0 : 0;
                }

                ac.conceal    = !!(p_triplet->data & 4);
                mac.conceal   = ~0;
                mac.invert    = p_triplet->data & 0x10;
                ac.underline  = !!(p_triplet->data & 0x20);
                mac.underline = ~0;

                TTX_PRINTF(("TTX:  enh col %d display attr 0x%02x\n"
                            , active_column, p_triplet->data));

                break;

            case 0x0D:        /* drcs character invocation */
#if TTX_SUPPORT_DRCS
            {
                s32     normal = (p_triplet->data & 0x20) ? 1 : 0;
                s32     offset = p_triplet->data & 0x3F;
                ttx_page_function_t function;
                s32                 page_no, page, i = 0;
                ttx_raw_t           *p_page;


                if(p_dec->max_level < TTX_DEC_LEVEL_2P5)
                    break;

                if(offset >= 48)
                    break; /* invalid */

                if(column > active_column)
                {
                    flush_column_vsb(p_raw_page
                                     , p_ac + inv_column + active_column
                                     , type
                                     , &ac
                                     , &mac
                                     , inv_row + active_row
                                     , column - active_column);

                    active_column = column;
                }

                page = normal * 16 + drcs_s1[normal];

                TTX_PRINTF(("TTX:  enh col %d DRCS %d/0x%02x\n"
                            , active_column, page, p_triplet->data));

                if(normal == 0)
                {
                    function = TTX_PAGE_FUNC_GDRCS;
                    page_no  = p_raw_page->data.lop.link[26].page_no;

                    if(TTX_NO_PAGE(page_no) == TRUE)
                    {
                        page_no = p_mag->drcs_link[1][0];
                        if(p_dec->max_level < TTX_DEC_LEVEL_3P5
                                || TTX_NO_PAGE(page_no))
                            page_no = p_mag->drcs_link[0][0];
                    }
                    else
                    {
                        TTX_PRINTF(("TTX:  X/27/4 GDRCS overrides MOT\n"));
                    }
                }
                else
                {
                    function = TTX_PAGE_FUNC_DRCS;
                    page_no  = p_raw_page->data.lop.link[25].page_no;

                    if(TTX_NO_PAGE(page_no) == TRUE)
                    {
                        i = p_mag->drcs_lut[p_raw_page->page_no & 0xFF];
                        if(i == 0)
                        {
                            TTX_PRINTF(("TTX:  MOT drcs_lut empty\n"));
                            return VBI_RC_FAILED;/* has no link (yet) */
                        }

                        page_no = p_mag->drcs_link[0][i + 8];
                        if(p_dec->max_level < TTX_DEC_LEVEL_3P5
                                || TTX_NO_PAGE(page_no) == TRUE)
                            page_no = p_mag->drcs_link[0][i + 0];
                    }
                    else
                        TTX_PRINTF(("TTX:  X/27/4 DRCS overrides MOT\n"));
                }

                if(TTX_NO_PAGE(page_no) == TRUE)
                {
                    TTX_PRINTF(("TTX:  dead MOT link %d\n", i));
                    return VBI_RC_FAILED; /* has no link (yet) */
                }

                TTX_PRINTF(("TTX:  %s drcs from page %03x/%04x\n"
                            , normal ? "normal" : "global"
                            , page_no, drcs_s1[normal]));

                ttx_get_raw_page_vsb(p_dec, page_no, drcs_s1[normal]
                                     , 0xffff, &p_page);

                if(p_page != NULL)
                {
                    TTX_PRINTF(("... page not cached\n"));
                    return VBI_RC_FAILED;
                }

                if(p_page->function == TTX_PAGE_FUNC_UNKOWN)
                {
                    vbi_rc_t rc;
                    rc = ttx_dec_convert_page_vsb(p_dec, p_page, function);
                    if(rc != VBI_RC_SUCCESS)
                    {
                        TTX_PRINTF(("TTX!:    no g/drcs page"
                                    , " or hamming error\n"));
                        return VBI_RC_FAILED;
                    }
                }
                else if(p_page->function == TTX_PAGE_FUNC_DRCS)
                {
                    p_page->function = function;
                }
                else if(p_page->function != function)
                {
                    TTX_PRINTF(("TTX:  source page wrong function"
                                , " %d, expected %d\n"
                                , p_page->function, function));
                    return VBI_RC_FAILED;
                }

                if((p_page->data.drcs.invalid & (1ULL << offset)) != 0)
                {
                    TTX_PRINTF(("... invalid drcs, prob. tx error\n"));
                    return VBI_RC_FAILED;
                }

                p_osd_page->p_drcs[page] = p_page->data.drcs.chars[0];

                unicode      = 0xF000 + (page << 6) + offset;
                ac.unicode   = unicode;
                mac.unicode = ~0;
            }
#else
            break;
#endif

            case 0x0E:        /* font style */
            {
                s32 italic = 0, bold = 0, proportional = 0;
                s32 col = 0, row = 0, count = 0;
                ttx_char_t *p_ac_stype = NULL;

                if(p_dec->max_level < TTX_DEC_LEVEL_3P5)
                    break;

                row   = inv_row + active_row;
                count = (p_triplet->data >> 4) + 1;
                p_ac_stype  = &p_osd_page->text[row * TTX_COLUMNS];

                proportional = (p_triplet->data >> 0) & 1;
                bold         = (p_triplet->data >> 1) & 1;
                italic       = (p_triplet->data >> 2) & 1;

                while(row < TTX_ROWS && count > 0)
                {
                    for(col = inv_column + column
                              ; col < TTX_COLUMNS
                            ; col ++)
                    {
                        p_ac_stype[col].italic       = italic;
                        p_ac_stype[col].bold         = bold;
                        p_ac_stype[col].proportional = proportional;
                    }

                    p_ac_stype += TTX_COLUMNS;
                    row ++;
                    count --;
                }

                TTX_PRINTF(("enh col %d font style 0x%02x\n"
                            , active_column, p_triplet->data));

                break;
            }

            case 0x0F:        /* G2 character */
                if(p_triplet->data >= 0x20)
                {
                    if(column > active_column)
                    {
                        flush_column_vsb(p_raw_page
                                         , p_ac + inv_column + active_column
                                         , type
                                         , &ac
                                         , &mac
                                         , inv_row + active_row
                                         , column - active_column);

                        active_column = column;
                    }

                    unicode     = vbi_teletext_unicode_vsb(p_font->G2
                                                           , CHAR_SUBSET_NONE, p_triplet->data);
                    ac.unicode  = unicode;
                    mac.unicode = ~0;
                }

                break;

            default:/*0x10 ... 0x1F characters including diacritical marks*/
                if(p_triplet->data >= 0x20)
                {
                    if(column > active_column)
                    {
                        flush_column_vsb(p_raw_page
                                         , p_ac + inv_column + active_column
                                         , type
                                         , &ac
                                         , &mac
                                         , inv_row + active_row
                                         , column - active_column);

                        active_column = column;
                    }

                    unicode = vbi_teletext_composed_unicode_vsb
                              (p_triplet->mode - 0x10, p_triplet->data);

                    ac.unicode  = unicode;
                    mac.unicode = ~0;
                }

                break;
            }
        }
    }

    return VBI_RC_SUCCESS;
}

static vbi_rc_t ttx_default_object_invocation_vsb(ttx_decoder_t *p_dec,
        ttx_magazine_t *p_mag,
        ttx_extension_t *p_ext,
        ttx_osd_page_t *p_osd_page,
        ttx_raw_t *p_raw_page,
        MT_BOOL header_only)
{
    vbi_rc_t        rc;
    ttx_pop_link_t  *p_pop = NULL;
    s32             i = 0, order = 0;

    if((i = p_mag->pop_lut[p_raw_page->page_no & 0xFF]) != TRUE)
        return VBI_RC_FAILED; /* has no link (yet) */

    p_pop = &p_mag->pop_link[0][(i + 8) % 8];

    if(p_dec->max_level < TTX_DEC_LEVEL_3P5 || TTX_NO_PAGE(p_pop->page_no))
    {
        p_pop = &p_mag->pop_link[0][i];

        if(TTX_NO_PAGE(p_pop->page_no) == TRUE)
        {
            TTX_PRINTF(("default object has dead MOT pop link %d\n", i));
            return VBI_RC_FAILED;
        }
    }

    order = p_pop->default_obj[0].type > p_pop->default_obj[1].type;

    for(i = 0; i < 2; i ++)
    {
        ttx_object_type_t   type         = p_pop->default_obj[i ^ order].type;
        ttx_raw_t           *p_trip_page = NULL;
        ttx_triplet_t       *p_next_trip = NULL;
        s32                 remaining_triplets = 0;

        if(type == TTX_OBJ_NONE)
            continue;

        TTX_PRINTF(("default object #%d invocation, type %d\n"
                    , i ^ order, type));

        rc = resolve_obj_address_vsb(p_dec
                                     , type, p_pop->page_no, p_pop->default_obj[i ^ order].address
                                     , TTX_PAGE_FUNC_POP
                                     , &p_trip_page, &remaining_triplets, &p_next_trip);

        if(rc != VBI_RC_SUCCESS || !p_next_trip)
            return VBI_RC_FAILED;

        rc = ttx_enhance_vsb(p_dec
                             , p_mag, p_ext, p_osd_page, p_raw_page, type, p_next_trip
                             , remaining_triplets, 0, 0, header_only);
        if(rc != VBI_RC_SUCCESS)
        {
            return rc;
        }
    }

    return VBI_RC_SUCCESS;
}

static void ttx_flof_navigation_bar_vsb(ttx_osd_page_t *p_osd_page
                                        , ttx_raw_t *p_raw_page)
{
    ttx_char_t ac;
    s32 n = 0, i = 0, k = 0, j = 0;

    memset(&ac, 0, sizeof(ac));

    ac.foreground = TTX_WHITE;
    ac.background = TTX_BLACK;
    ac.opacity    = p_osd_page->page_opacity[1];
    ac.unicode    = 0x0020;

    for(i = 0; i < TTX_COLUMNS; i ++)
        p_osd_page->text[TTX_LAST_ROW + i] = ac;

    for(i = 0; i < 4; i ++)
    {
        j = i * 10 + 3;

        p_osd_page->nav_link[i].page_no  = p_raw_page->data.lop.link[i].page_no;
        p_osd_page->nav_link[i].sub_no   = p_raw_page->data.lop.link[i].sub_no;

        if(p_raw_page->data.lop.link[i].page_no == TTX_NULL_PAGE_NO)
            continue;

        for(k = 0; k < 3; k ++)
        {
            n = ((p_raw_page->data.lop.link[i].page_no >> ((2 - k) * 4)) & 15)
                + '0';

            if(n > '9')
                n += 'A' - '9';

            ac.unicode                              = n;
            ac.foreground                           = flof_link_col_vsb[i];
            p_osd_page->text[TTX_LAST_ROW + j + k] = ac;
        }

    }
}

void ttx_add_navigation_bar_vsb(ttx_osd_page_t *p_osd_page,  u16 page_no)
{
    ttx_char_t ac;
    s32 n, i, k, j;
    memset(&ac, 0, sizeof(ac));

    ac.foreground = TTX_WHITE;
    ac.background = TTX_BLACK;
    ac.opacity    = p_osd_page->page_opacity[1];
    ac.unicode    = 0x0020;
    for(i = 0; i < TTX_COLUMNS; i ++)
    {
        p_osd_page->text[TTX_LAST_ROW + i] = ac;
    }
    
	ttx_get_page_no_vsb(page_no, 1, (u16 *)(&(p_osd_page->nav_link[0].page_no)));
	ttx_get_page_no_vsb(page_no, -1, (u16 *)(&(p_osd_page->nav_link[1].page_no)));
	ttx_get_page_no_vsb(page_no, 10, (u16 *)(&(p_osd_page->nav_link[2].page_no)));
	ttx_get_page_no_vsb(page_no, 100,(u16 *)(&(p_osd_page->nav_link[3].page_no)));
    for(i = 0; i < 4; i ++)
    {
        j = i * 10 + 3;
       p_osd_page->nav_link[i].sub_no   = TTX_FIRST_SUBPAGE;
        for(k = 0; k < 3; k ++)
        {
            n = ((p_osd_page->nav_link[i].page_no >> ((2 - k) * 4)) & 15)
                + '0';
            if(n > '9')
                n += 'A' - '9';
            ac.unicode                              = n;
            ac.foreground                           = flof_link_col_vsb[i];
            p_osd_page->text[TTX_LAST_ROW + j + k] = ac;
        }
    }
}

static void ttx_flof_links_vsb(ttx_osd_page_t *p_osd_page
                               , ttx_raw_t *p_raw_page)
{
    ttx_char_t *p_ac = p_osd_page->text + TTX_LAST_ROW;
    s32 i = 0, k = 0, col = -1;
    int link_count;

    link_count = 0;
    for(i = 0; i < TTX_COLUMNS; i ++)
    {
        if((p_ac[i].foreground & 7) != col)
        {
            col = p_ac[i].foreground & 7;

            for(k = 0; k < 4; k ++)
            {
                if((s32)flof_link_col_vsb[k] == col)
                    break;
            }

            if(k < 4 && !TTX_NO_PAGE(p_raw_page->data.lop.link[k].page_no))
            {
                p_osd_page->nav_link[k].page_no
                    = p_raw_page->data.lop.link[k].page_no;
                p_osd_page->nav_link[k].sub_no
                    = p_raw_page->data.lop.link[k].sub_no;
		  link_count ++;		
            }
        }
    }
    if(link_count == 0)
    {
        ttx_add_navigation_bar_vsb(p_osd_page, p_osd_page->page_no);
     }
}

void ttx_osd_page_init_vsb(ttx_decoder_t *p_dec, ttx_osd_page_t *p_osd_page)
{
    p_osd_page->user_screen_opacity = 100;
    p_osd_page->screen_color        = TTX_BLACK;
}

vbi_rc_t ttx_osd_page_format_vsb(ttx_decoder_t *p_dec
                                 , ttx_osd_page_t *p_osd_page, ttx_raw_t *p_raw_page
                                 , MT_BOOL header_only, MT_BOOL navigation)
{
    ttx_magazine_t  *p_mag = NULL;
    ttx_extension_t *p_ext = NULL;
    s32             column = 0, row = 0, i = 0;

    s32 cnt = 0;
    MT_BOOL  dis_char = FALSE;
    s32   dis_raw = 0;
    
    TTX_PRINTF(("TTX:  Formatting page  %03x/%04x\n"
                , p_raw_page->page_no, p_raw_page->sub_no));

    if(p_raw_page->function != TTX_PAGE_FUNC_LOP
            && p_raw_page->function != TTX_PAGE_FUNC_SUBTITLE
            && p_raw_page->function != TTX_PAGE_FUNC_EACEMT_TRIGGER)
        return VBI_RC_INVALID_DATA;

    p_osd_page->page_no      = p_raw_page->page_no;
    p_osd_page->sub_no       = p_raw_page->sub_no;
    p_osd_page->rows         = header_only ? 1 : TTX_ROWS;
    p_osd_page->columns      = TTX_COLUMNS;
    p_osd_page->control_bits = p_raw_page->control_bits;
    p_osd_page->dirty.y0     = 0;
    p_osd_page->dirty.y1     = TTX_ROWS - 1;
    p_osd_page->dirty.roll   = 0;

    p_mag = (p_dec->max_level <= TTX_DEC_LEVEL_1P5) ? &p_dec->def_mag
            : &p_dec->mag[((p_raw_page->page_no >> 8) & 0xf) - 1];

    if((p_raw_page->x28_designations & 0x11) != 0)
        p_ext = &p_raw_page->data.ext_lop.ext;
    else
        p_ext = &p_mag->extension;

    /* Character set designation */
    ttx_get_character_set_vsb(p_dec, p_ext, p_raw_page,
                              (const ttx_font_descr_t **)p_osd_page->p_font);

    /* Colors */
    ttx_set_screen_color_vsb(p_dec
                             , p_osd_page
                             , p_raw_page->control_bits
                             , p_ext->def_screen_color);
    p_osd_page->p_drcs_clut = p_ext->drcs_clut;

    memcpy(p_osd_page->color_map, p_ext->color_map, sizeof(p_ext->color_map));

    /* Opacity */
    p_osd_page->page_opacity[1] = (p_raw_page->control_bits
                                   & (C5_NEWSFLASH | C6_SUBTITLE | C10_INHIBIT_DISPLAY))
                                  ? TTX_TRANSPARENT_SPACE : TTX_OPAQUE;
    p_osd_page->boxed_opacity[1] = (p_raw_page->control_bits
                                    & C10_INHIBIT_DISPLAY)
                                   ? TTX_TRANSPARENT_SPACE : TTX_SEMI_TRANSPARENT;

    if((p_raw_page->control_bits & C7_SUPPRESS_HEADER) != 0)
    {
        p_osd_page->page_opacity[0]  = TTX_TRANSPARENT_SPACE;
        p_osd_page->boxed_opacity[0] = TTX_TRANSPARENT_SPACE;
    }
    else
    {
        p_osd_page->page_opacity[0]  = TTX_OPAQUE;
        p_osd_page->boxed_opacity[0] = TTX_SEMI_TRANSPARENT;
    }

    /* DRCS */
    memset(p_osd_page->p_drcs, 0, sizeof(p_osd_page->p_drcs));

    /* Level 1 formatting */

    i = 0;

    for(row = 0; row < p_osd_page->rows; row ++)
    {
        ttx_font_descr_t *p_font = NULL;
        s32         mosaic_unicodes = 0;/* 0xEE00 separate, 0xEE20 contiguous */
        s32         held_mosaic_unicode = 0;
        s32         esc = 0;
        MT_BOOL        hold = FALSE, mosaic = FALSE;
        MT_BOOL        double_height = FALSE, wide_char = FALSE;
        ttx_char_t  ac, *p_ac = &p_osd_page->text[row * TTX_COLUMNS];

        held_mosaic_unicode = 0xEE20;   /* G1 block mosaic, blank, contiguous */

        memset(&ac, 0, sizeof(ac));

        ac.unicode      = 0x0020;
        ac.foreground   = p_ext->foreground_clut + TTX_WHITE;
        ac.background   = p_ext->background_clut + TTX_BLACK;
        mosaic_unicodes = 0xEE20;    /* contiguous */
        ac.opacity      = p_osd_page->page_opacity[row > 0];
        p_font          = p_osd_page->p_font[0];
        esc             = 0;
        hold            = FALSE;
        mosaic          = FALSE;
        double_height   = FALSE;
        wide_char       = FALSE;

        for(column = 0; column < TTX_COLUMNS; ++column)
        {
            s32 raw = 0;

            if(row == 0 && column < 8)
            {
                    raw = ' ';

                if((i > 0) &&(i < 4))
                {
                    raw = ((p_osd_page->page_no >> ((3 - i) * 4)) & 0xf) + 0x30;
                }
		  else if(p_osd_page->sub_no != 0)
		  {
		  	if(i == 4)
                {
                        raw = '/';
                	}
                    else
		  	{
				if(p_dec->display_page.manual_flag == 0)
				{
					if(i == 5)
					{
						raw = 'A';
                }
					else if(i == 6)
                {
						raw = 'T';
					}
				}
				else
				{
                    if(p_osd_page->sub_no != 0)
                    			{
                    				if(i==5)
                    				{
							raw = ((p_osd_page->sub_no >> 4) & 0xf) + 0x30;			
						}
						else if(i == 6)
						{
							raw = (p_osd_page->sub_no & 0xf) + 0x30;
                }
					}		
				}
		  	}
		  }
                i ++;
            }
            #if 0            
            else if((raw = vbi_unpar8_vsb(p_raw_page->data.lop.raw[0][i ++])) < 0)
            {
                raw = ' ';
            }
            #else  //fix Bug 133836(bugzilla)
            else
            {
                if((raw = vbi_unpar8_vsb(p_raw_page->data.lop.raw[row][column])) < 0)
                {
                  raw = ' ';
                }
                i++;
            }
            #endif
            /* set-at spacing attributes */
            switch(raw)
            {
            case 0x09:        /* steady */
                ac.flash = FALSE;
                break;

            case 0x0C:        /* normal size */
                ac.size = TTX_NORMAL_SIZE;
                break;

            case 0x18:        /* conceal */
                ac.conceal = TRUE;
                break;

            case 0x19:        /* contiguous mosaics */
                mosaic_unicodes = 0xEE20;
                break;

            case 0x1A:        /* separated mosaics */
                mosaic_unicodes = 0xEE00;
                break;

            case 0x1C:        /* black background */
                ac.background = p_ext->background_clut + TTX_BLACK;
                break;

            case 0x1D:        /* new background */
                ac.background = p_ext->background_clut
                                + (ac.foreground & 7);
                break;

            case 0x1E:        /* hold mosaic */
                hold = TRUE;
                break;
            }

            if(raw <= 0x1F)
            {
                ac.unicode = (hold & mosaic) ? held_mosaic_unicode : 0x0020;
            }
            else
            {
                if(mosaic == TRUE && (raw & 0x20) != 0)
                {
                    held_mosaic_unicode = mosaic_unicodes + raw - 0x20;
                    ac.unicode          = held_mosaic_unicode;
                }
                else
                {
                    ac.unicode = vbi_teletext_unicode_vsb(p_font->G0
                                                          , p_font->subset, raw);
                }
            }

            if(wide_char == TRUE)
            {
                wide_char = FALSE;
            }
            else
            {
                p_ac[column] = ac;

                if(ac.unicode != ' ')
                {
                    wide_char = ac.size & TTX_DOUBLE_WIDTH;
                    if(wide_char == TRUE)
                    {
                        if(column < (TTX_COLUMNS - 1))
                        {
                            p_ac[column + 1] = ac;
                            p_ac[column + 1].size = TTX_OVER_TOP;
                        }
                        else
                        {
                            p_ac[column].size = TTX_NORMAL_SIZE;
                            wide_char = FALSE;
                        }
                    }
                }
                else if((ac.size & TTX_DOUBLE_HEIGHT) != 0)
                {
                    p_ac[column].size = TTX_DOUBLE_HEIGHT;
                }
                else
                {
                    p_ac[column].size = TTX_NORMAL_SIZE;
                }
            }

            /* set-after spacing attributes */
            switch(raw)
            {
            case 0x08:        /* flash */
                ac.flash = TRUE;
                break;

            case 0x0A:        /* end box */
                if(column < (TTX_COLUMNS) 
                        && vbi_unpar8_vsb(p_raw_page->data.lop.raw[row][column]) == 0x0a) // //fix Bug 133836(bugzilla)
                    ac.opacity = p_osd_page->page_opacity[row > 0];
                break;

            case 0x0B:        /* start box */
                if(column < (TTX_COLUMNS)
                        && vbi_unpar8_vsb(p_raw_page->data.lop.raw[row][column]) == 0x0b) // //fix Bug 133836(bugzilla)
                    ac.opacity = p_osd_page->boxed_opacity[row > 0];
                
                ////fix bug,// 10859,15824,18542
                if((p_dec->is_display_subtitle) || ((p_osd_page->control_bits & C6_SUBTITLE) == C6_SUBTITLE))
                {
                    dis_char = FALSE; 
                    dis_raw = 0;
                    //OS_PRINTF("dis_raw:\n",dis_raw);
                    for(cnt = column; cnt < TTX_COLUMNS; cnt++)
                    {
                        dis_raw = vbi_unpar8_vsb(p_raw_page->data.lop.raw[row][cnt]); 
                        //OS_PRINTF("%x ",dis_raw);                        
                        if((dis_raw > 0x20) && (dis_raw < 0x80))
                        {
                            dis_char = TRUE;
                            break;
                        }
                    }
                    if((!dis_char) && (cnt > column))
                    {
                        ac.opacity = TTX_TRANSPARENT_SPACE;
                        p_ac[column] = ac;
                    }
                }
                ////fix bug,// 10859,15824,18542
                break;

            case 0x0D:        /* double height */
                if(row <= 0 || row >= 23)
                    break;
                ac.size       = TTX_DOUBLE_HEIGHT;
                double_height = TRUE;
                break;

            case 0x0E:        /* double width */
                if(column < (TTX_COLUMNS - 1))
                    ac.size = TTX_DOUBLE_WIDTH;
                break;

            case 0x0F:        /* double size */
                if(column >= (TTX_COLUMNS - 1)
                        || row <= 0 || row >= 23)
                    break;
                ac.size = TTX_DOUBLE_SIZE;
                double_height = TRUE;
                break;

            case 0x1F:        /* release mosaic */
                hold = FALSE;
                break;

            case 0x1B:        /* ESC */
                p_font = p_osd_page->p_font[esc ^= 1];
                break;

            default:
                if(raw >= 0 && raw <= 0x07)
                {
                    ac.foreground = p_ext->foreground_clut + (raw & 7);
                    ac.conceal      = FALSE;
                    mosaic          = FALSE;
                }
                else if(raw >= 0x10 && raw <= 0x17)
                {
                    ac.foreground = p_ext->foreground_clut + (raw & 7);
                    ac.conceal    = FALSE;
                    mosaic        = TRUE;
                }
                break;
            }
        }

        if(double_height == TRUE)
        {
            for(column = 0; column < TTX_COLUMNS; column ++)
            {
                ac = p_ac[column];

                switch(ac.size)
                {
                case TTX_DOUBLE_HEIGHT:
                    ac.size = TTX_DOUBLE_HEIGHT2;
                    p_ac[TTX_COLUMNS + column] = ac;
                    break;

                case TTX_DOUBLE_SIZE:
                    ac.size                        = TTX_OVER_TOP;
                    p_ac[column + 1]               = ac;
                    ac.size                        = TTX_DOUBLE_SIZE2;
                    p_ac[TTX_COLUMNS + column]     = ac;
                    ac.size                        = TTX_OVER_BOTTOM;
                    p_ac[TTX_COLUMNS + (++column)] = ac;
                    break;

                default: /* NORMAL, DOUBLE_WIDTH, OVER_TOP */
                    ac.size = TTX_NORMAL_SIZE;
                    ac.unicode = 0x0020;
                    p_ac[TTX_COLUMNS + column] = ac;
                    break;
                }
            }

            i += TTX_COLUMNS;
            row ++;
        }
    }

    /* Local enhancement data and objects */

    if(p_dec->max_level >= TTX_DEC_LEVEL_1P5)
    {
        ttx_osd_page_t  page;
        vbi_rc_t        rc;

        memcpy(&page, p_osd_page, sizeof(page));

        if((p_raw_page->control_bits & (C5_NEWSFLASH | C6_SUBTITLE)) == 0)
        {
            p_osd_page->boxed_opacity[0] = TTX_TRANSPARENT_SPACE;
            p_osd_page->boxed_opacity[1] = TTX_TRANSPARENT_SPACE;
        }

        if((p_raw_page->x26_designations & 1) != 0)
        {
            TTX_PRINTF(("TTX:  enhancement packets %08x\n"
                        , p_raw_page->x26_designations));
            rc = ttx_enhance_vsb(p_dec
                                 , p_mag, p_ext, p_osd_page, p_raw_page
                                 , TTX_OBJ_LOCAL_ENH_DATA
                                 , p_raw_page->data.enh_lop.trip
                                 , ELEMENTS(p_raw_page->data.enh_lop.trip)
                                 , 0, 0, header_only);
        }
        else
        {
            rc = ttx_default_object_invocation_vsb(p_dec
                                                   , p_mag, p_ext, p_osd_page, p_raw_page, header_only);
        }

        if(rc == VBI_RC_SUCCESS)
        {
            if(p_dec->max_level >= TTX_DEC_LEVEL_2P5 && !header_only)
                ttx_post_enhance_vsb(p_osd_page);
        }
        else
        {
            memcpy(p_osd_page, &page, sizeof(*p_osd_page));
        }
    }

    /* Navigation */

    if(navigation == TRUE)
    {
        //u32 i = 0;
        for(i = 0; i < 5; i ++)
        {
            p_osd_page->nav_link[i].page_no = TTX_NULL_PAGE_NO;
        }
        p_osd_page->nav_link[5].page_no = p_dec->initial_page.page_no;
        p_osd_page->nav_link[5].sub_no  = p_dec->initial_page.sub_no;

        if(p_raw_page->data.lop.have_flof == TRUE)
        {
            if(p_raw_page->data.lop.link[5].page_no >= 0x100
                    && p_raw_page->data.lop.link[5].page_no <= 0x899
                    && (p_raw_page->data.lop.link[5].page_no & 0xFF) != 0xFF)
            {
                p_osd_page->nav_link[5].page_no
                    = p_raw_page->data.lop.link[5].page_no;
                p_osd_page->nav_link[5].sub_no
                    = p_raw_page->data.lop.link[5].sub_no;
            }

            if((p_raw_page->lop_packets & (1 << 24)) != 0)
            	{
                ttx_flof_links_vsb(p_osd_page, p_raw_page);
            	}
            else
            	{
                ttx_flof_navigation_bar_vsb(p_osd_page, p_raw_page);
		}
        }
		else
		{
			ttx_add_navigation_bar_vsb(p_osd_page, p_osd_page->page_no);
		}
    }

    return VBI_RC_SUCCESS;
}

void ttx_get_flash_line_pos(ttx_osd_page_t *p_osd_page,u8 *start_line,u8 *end_line)
{
    s32 row = 0;
    s32 column = 0;
    ttx_char_t  *p_ttx_char = NULL;
    u8  start_cnt = 0;
    u8  end_cnt = 0;

    if(p_osd_page && start_line && end_line)
    {
        start_cnt = end_cnt = 0;
        for(row = 1; row < p_osd_page->rows;  row++)
        {
            for(column = 0; column < TTX_COLUMNS; column++)
            {
                p_ttx_char= &(p_osd_page->text[row * TTX_COLUMNS + column]);
                if(p_ttx_char->flash)
                {
                    if(!start_cnt)
                    {
                        start_cnt = row;
                    }
                    end_cnt = row + 1;
                    break;
                }
            }
        }
        *start_line = start_cnt;
        *end_line = end_cnt;
    }
}

