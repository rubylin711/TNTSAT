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
//#include "mtos_msg.h"
//#include "mdl.h"
//#include "drv_misc.h"

#include "ttx_lang_vsb.h"
#include "ttx_bcd_vsb.h"
#include "ttx_hamm_vsb.h"
#include "ttx_dec_vsb.h"
#include "ttx_format_vsb.h"
#include "ttx_db_vsb.h"
#include "vbi_api.h"

extern const u8 _vbi_bit_reverse_vsb[256];
extern const u8 _vbi_hamm8_fwd_vsb[16];
extern const s8 _vbi_hamm8_inv_vsb[256];
extern const u8 _vbi_hamm24_fwd_0_vsb[256];
extern const u8 _vbi_hamm24_fwd_1_vsb[256];
extern const u8 _vbi_hamm24_fwd_2_vsb[4];
extern const s8 _vbi_hamm24_inv_par_vsb[3][256];
extern const u8 _vbi_hamm24_inv_d1_d4_vsb[64];
extern const s32 _vbi_hamm24_inv_err_vsb[64];

u32 g_inital_page = 0;

/*!
   TODO: fix me!

   \param[in] c
  */
static inline u8 vbi_rev8_vsb(u8 c)
{
    return _vbi_bit_reverse_vsb[c];
}

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

/*!
   TODO: fix me!

   \param[in] c
  */
static inline s8 vbi_unham8_vsb(u8 c)
{
    return _vbi_hamm8_inv_vsb[c];
}

/*!
   TODO: fix me!

   \param[in] p_p
  */
static inline s16 vbi_unham16p_vsb(const u8 *p_p)
{
    return (s16)_vbi_hamm8_inv_vsb[p_p[0]] | (((s16)_vbi_hamm8_inv_vsb[p_p[1]]) << 4);
}

/*
 *  Teletext font descriptors
 *
 *  ETS 300 706 Table 32, 33, 34
 */
static const ttx_font_descr_t ttx_font_descriptors[88] =
{
    /* 0 - Western and Central Europe */
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_ENGLISH},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_GERMAN},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_SWE_FIN_HUN},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_ITALIAN},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_FRENCH,},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_PORTUG_SPANISH},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_CZECH_SLOVAK},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_NONE},

    /* 8 - Eastern Europe */
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_POLISH},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_GERMAN},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_SWE_FIN_HUN},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_ITALIAN},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_FRENCH},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_NONE},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_CZECH_SLOVAK},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_NONE},

    /* 16 - Western Europe and Turkey */
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_ENGLISH},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_GERMAN},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_SWE_FIN_HUN},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_ITALIAN},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_FRENCH},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_PORTUG_SPANISH},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_TURKISH},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_NONE},

    /* 24 - Central and Southeast Europe */
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_NONE},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_NONE},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_NONE},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_NONE},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_NONE},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_SERB_CRO_SLO},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_NONE},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_RUMANIAN},

    /* 32 - Cyrillic */
    { CHAR_SET_CYRILLIC_1_G0, CHAR_SET_CYRILLIC_G2, CHAR_SUBSET_NONE,},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_GERMAN},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_ESTONIAN},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_LETT_LITH},
    { CHAR_SET_CYRILLIC_2_G0, CHAR_SET_CYRILLIC_G2, CHAR_SUBSET_NONE},
    { CHAR_SET_CYRILLIC_3_G0, CHAR_SET_CYRILLIC_G2, CHAR_SUBSET_NONE},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_CZECH_SLOVAK},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},

    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},

    /* 48 */
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_LATIN_G0, CHAR_SET_LATIN_G2, CHAR_SUBSET_TURKISH},
    { CHAR_SET_GREEK_G0, CHAR_SET_GREEK_G2, CHAR_SUBSET_NONE},

    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},

    /* 64 - Arabic */
    { CHAR_SET_LATIN_G0, CHAR_SET_ARABIC_G2, CHAR_SUBSET_ENGLISH},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_LATIN_G0, CHAR_SET_ARABIC_G2, CHAR_SUBSET_FRENCH},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_ARABIC_G0, CHAR_SET_ARABIC_G2, CHAR_SUBSET_NONE},

    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},

    /* 80 */
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_HEBREW_G0, CHAR_SET_ARABIC_G2, CHAR_SUBSET_NONE},
    { CHAR_SET_NONE, CHAR_SET_NONE, CHAR_SUBSET_NONE},
    { CHAR_SET_ARABIC_G0, CHAR_SET_ARABIC_G2, CHAR_SUBSET_NONE},
};

/*
 *  ETS 300 706 Table 30: Colour Map
 */
static const u32 default_color_map[40] =
{
    TTX_RGBA(0x00, 0x00, 0x00, 0xFF), TTX_RGBA(0xFF, 0x00, 0x00, 0xFF),
    TTX_RGBA(0x00, 0xFF, 0x00, 0xFF), TTX_RGBA(0xFF, 0xFF, 0x00, 0xFF),
    TTX_RGBA(0x00, 0x00, 0xFF, 0xFF), TTX_RGBA(0xFF, 0x00, 0xFF, 0xFF),
    TTX_RGBA(0x00, 0xFF, 0xFF, 0xFF), TTX_RGBA(0xFF, 0xFF, 0xFF, 0xFF),
    TTX_RGBA(0x00, 0x00, 0x00, 0x00), TTX_RGBA(0x77, 0x00, 0x00, 0xFF),
    TTX_RGBA(0x00, 0x77, 0x00, 0xFF), TTX_RGBA(0x77, 0x77, 0x00, 0xFF),
    TTX_RGBA(0x00, 0x00, 0x77, 0xFF), TTX_RGBA(0x77, 0x00, 0x77, 0xFF),
    TTX_RGBA(0x00, 0x77, 0x77, 0xFF), TTX_RGBA(0x77, 0x77, 0x77, 0xFF),
    TTX_RGBA(0xFF, 0x00, 0x55, 0xFF), TTX_RGBA(0xFF, 0x77, 0x00, 0xFF),
    TTX_RGBA(0x00, 0xFF, 0x77, 0xFF), TTX_RGBA(0xFF, 0xFF, 0xBB, 0xFF),
    TTX_RGBA(0x00, 0xCC, 0xAA, 0xFF), TTX_RGBA(0x55, 0x00, 0x00, 0xFF),
    TTX_RGBA(0x66, 0x55, 0x22, 0xFF), TTX_RGBA(0xCC, 0x77, 0x77, 0xFF),
    TTX_RGBA(0x33, 0x33, 0x33, 0xFF), TTX_RGBA(0xFF, 0x77, 0x77, 0xFF),
    TTX_RGBA(0x77, 0xFF, 0x77, 0xFF), TTX_RGBA(0xFF, 0xFF, 0x77, 0xFF),
    TTX_RGBA(0x77, 0x77, 0xFF, 0xFF), TTX_RGBA(0xFF, 0x77, 0xFF, 0xFF),
    TTX_RGBA(0x77, 0xFF, 0xFF, 0xFF), TTX_RGBA(0xDD, 0xDD, 0xDD, 0xFF),

    /* Private colors */

    TTX_RGBA(0x00, 0x00, 0x00, 0xFF), TTX_RGBA(0xFF, 0xAA, 0x99, 0xFF),
    TTX_RGBA(0x44, 0xEE, 0x00, 0xFF), TTX_RGBA(0xFF, 0xDD, 0x00, 0xFF),
    TTX_RGBA(0xFF, 0xAA, 0x99, 0xFF), TTX_RGBA(0xFF, 0x00, 0xFF, 0xFF),
    TTX_RGBA(0x00, 0xFF, 0xFF, 0xFF), TTX_RGBA(0xEE, 0xEE, 0xEE, 0xFF)
};

#if TTX_SUPPORT_DRCS
static u32 expand[64];
static void init_expand(void)
{
    s32 i = 0, j = 0, n = 0;

    if(expand[1] != 0)
        return;

    for(i = 0; i < 64; i ++)
    {
        for(n = j = 0; j < 6; j ++)
            if((i != 0) && (0x20 >> j))
                n |= 1 << (j * 4);

        expand[i] = n;
    }
}
#endif

typedef struct
{
    s32 *p_start;
    s32 *p_d;
    s32 *p_end;
    s32 left;
} bs_ttx_t;

static inline void bs_ttx_init(bs_ttx_t *p_s, void *p_data, u32 size)
{
    p_s->p_start = p_data;
    p_s->p_d     = p_data;
    p_s->p_end   = p_s->p_d + size;
    p_s->left    = 18;
}

static inline MT_BOOL bs_ttx_eof(bs_ttx_t *p_s)
{
    return (p_s->p_d >= p_s->p_end ? TRUE: FALSE);
}

static u32 bs_ttx_read(bs_ttx_t *p_s, s32 count)
{
    static const u32 mask[33] =
    {
        0x00,
        0x01,      0x03,      0x07,      0x0f,
        0x1f,      0x3f,      0x7f,      0xff,
        0x1ff,     0x3ff,     0x7ff,     0xfff,
        0x1fff,    0x3fff,    0x7fff,    0xffff,
        0x1ffff,   0x3ffff,   0x7ffff,   0xfffff,
        0x1fffff,  0x3fffff,  0x7fffff,  0xffffff,
        0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
        0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff
    };
    s32 shr    = 0;
    u32 result = 0;

    while(count > 0)
    {
        if(p_s->p_d >= p_s->p_end)
        {
            break;
        }

        if((shr = p_s->left - count) >= 0)
        {
            /* more in the buffer than requested */
            result  |= (*p_s->p_d >> shr) & mask[count];
            p_s->left -= count;
            if(p_s->left == 0)
            {
                p_s->p_d ++;
                p_s->left = 18;
            }
            return (result);
        }
        else
        {
            /* less in the buffer than requested */
            result |= (*p_s->p_d & mask[p_s->left]) << (- shr);
            count  -= p_s->left;
            p_s->p_d ++;
            p_s->left   = 18;
        }
    }

    return (result);
}

static void bs_ttx_skip(bs_ttx_t *p_s, int i_count)
{
    p_s->left -= i_count;

    while(p_s->left <= 0)
    {
        p_s->p_d ++;
        p_s->left += 18;
    }
}

static inline void bs_ttx_align(bs_ttx_t *p_s)
{
    if(p_s->left != 18)
    {
        p_s->left = 18;
        p_s->p_d ++;
    }
}

static vbi_rc_t ttx_unham_page_link_vsb(ttx_page_link_t *p_link
                                        , const u8 *p_raw, u8 mag_no)
{
    s32 b1 = 0, b2 = 0, b3 = 0, err = 0, m = 0;

    err  = b1 = vbi_unham16p_vsb(p_raw + 0);
    err |= b2 = vbi_unham16p_vsb(p_raw + 2);
    err |= b3 = vbi_unham16p_vsb(p_raw + 4);

    if(err < 0)
        return VBI_RC_INVALID_DATA;

    m = ((b3 >> 5) & 6) + ((b2 >> 7) & 1);

    m = ((mag_no == 8) ? 0 : mag_no) ^ m;
    if(m == 0)
        m = 8;

    p_link->page_no = (s16)(m * 256 + b1);
    p_link->sub_no  = (s16)((b3 * 256 + b2) & 0x3f7f);

    return VBI_RC_SUCCESS;
}

static vbi_rc_t ttx_parse_pop_vsb(ttx_decoder_t *p_dec
                                  , ttx_raw_t *p_page, u8 *p_raw, u8 packet_no)
{
    s32             dc = 0, triplet[13];
    ttx_triplet_t   *p_trip = NULL;
    s32             i       = 0;


    if(packet_no > 26)
        return VBI_RC_INVALID_DATA;

    if((dc = vbi_unham8_vsb(p_raw[0])) < 0)
        return VBI_RC_INVALID_DATA;

    if(dc > 15)
        return VBI_RC_INVALID_DATA;

    for(p_raw ++, i = 0; i < 13; p_raw += 3, i ++)
        triplet[i] = vbi_unham24p_vsb(p_raw);

    if(packet_no == 26)
        packet_no += (u8)dc;

    if(packet_no == 0)
    {
        return VBI_RC_SUCCESS;
    }
    if(packet_no <= 2)
    {
        if((dc & 1) == 0)
            return VBI_RC_INVALID_DATA;
    }
    else if(packet_no <= 42)
    {
        if(packet_no <= 4)
        {
            if((dc & 1) != 0)
            {
                s32 index = (packet_no - 1) * 26;

                for(index += 2, i = 1; i < 13; index += 2, i ++)
                {
                    if(triplet[i] >= 0)
                    {
                        p_page->data.pop.pointer[index + 0]
                            = (u16)(triplet[i] & 0x1FF);
                        p_page->data.pop.pointer[index + 1]
                            = (u16)(triplet[i] >> 9);
                    }
                }
                return VBI_RC_SUCCESS;
            }
        }

        p_trip = p_page->data.pop.triplet + (packet_no - 3) * 13;

        for(i = 0; i < 13; p_trip ++, i ++)
        {
            if(triplet[i] >= 0)
            {
                p_trip->address = (u8)((triplet[i] >> 0) & 0x3F);
                p_trip->mode    = (u8)((triplet[i] >> 6) & 0x1F);
                p_trip->data    = (u8)((triplet[i] >> 11));
            }
        }

        return VBI_RC_SUCCESS;
    }

    return VBI_RC_INVALID_DATA;
}

static vbi_rc_t ttx_parse_mot_vsb(ttx_decoder_t *p_dec
                                  , ttx_magazine_t *p_mag, u8 *p_raw, u8 packet_no)
{
    s32 err = 0, i = 0, j = 0;

    if(packet_no == 0 || packet_no >= 0x800)
        return VBI_RC_SUCCESS;

    if(packet_no <= 8)
    {
        u32 index = (packet_no - 1) << 5;
        s8  n0 = 0, n1 = 0;

        for(i = 0; i < 20; index ++, i ++)
        {
            if(i == 10)
                index += 6;

            n0 = vbi_unham8_vsb (*p_raw ++);
            n1 = vbi_unham8_vsb (*p_raw ++);

            if((n0 | n1) < 0)
                continue;

            p_mag->pop_lut[index]  = n0 & 7;
            p_mag->drcs_lut[index] = n1 & 7;
        }

        return VBI_RC_SUCCESS;
    }
    else if(packet_no <= 14)
    {
        s32 index = (packet_no - 9) * 0x30 + 10;

        for(i = 0; i < 20; index ++, i ++)
        {
            s32 n0 = 0, n1 = 0;

            if(i == 6 || i == 12)
            {
                if(index == 0x100)
                    break;
                else
                    index += 10;
            }

            n0 = vbi_unham8_vsb (*p_raw ++);
            n1 = vbi_unham8_vsb (*p_raw ++);

            if((n0 | n1) < 0)
                continue;

            p_mag->pop_lut[index]  = (s8)(n0 & 7);
            p_mag->drcs_lut[index] = (s8)(n1 & 7);
        }

        return VBI_RC_SUCCESS;
    }
    else if(packet_no <= 18)
    {
        return VBI_RC_SUCCESS;
    }
    else if(packet_no == 19 || packet_no == 20
            || packet_no == 22 || packet_no == 23)
    {
        ttx_pop_link_t *p_pop = NULL;

        if(packet_no == 22 || packet_no == 23)
            packet_no --;

        p_pop = p_mag->pop_link[0] + (packet_no - 19) * 4;

        for(i = 0; i < 4; p_raw += 10, p_pop ++, i ++)
        {
            s32 n[10];

            for(err = j = 0; j < 10; j ++)
                err |= n[j] = vbi_unham8_vsb (p_raw[j]);

            if(err < 0) /*    unused bytes poss. not hammed (^ N3) */
                continue;

            p_pop->page_no = (s16)((((n[0] & 7) ? (n[0] & 7) : 8) << 8)
                                   + (n[1] << 4) + n[2]);

            /* n[3] number of subpages ignored */

            if((n[4] & 1) != 0)
            {
                memset(&p_pop->fallback, 0, sizeof(p_pop->fallback));
            }
            else
            {
                s32 x = (n[4] >> 1) & 3;

                p_pop->fallback.black_bg_substitution = n[4] >> 3;

                /* x: 0/0, 16/0, 0/16, 8/8 */
                p_pop->fallback.left_panel_columns  = "\00\20\00\10"[x];
                p_pop->fallback.right_panel_columns = "\00\00\20\10"[x];
            }

            p_pop->default_obj[0].type    = (ttx_object_type_t)(n[5] & 3);
            p_pop->default_obj[0].address = (u8)(((n[7] << 4) + n[6]) & 0xFF);
            p_pop->default_obj[1].type    = (ttx_object_type_t)(n[5] >> 2);
            p_pop->default_obj[1].address = (u8)(((n[9] << 4) + n[8]) & 0xFF);
        }

        return VBI_RC_SUCCESS;
    }
    else if(packet_no == 21 || packet_no == 24)
    {
#if TTX_SUPPORT_DRCS
        s32 index = (packet_no == 21) ? 0 : 8;
        s32 n[4];

        for(i = 0; i < 8; p_raw += 4, index ++, i ++)
        {
            for(err = j = 0; j < 4; j ++)
                err |= n[j] = vbi_unham8_vsb (p_raw[j]);

            if(err < 0)
                continue;

            p_mag->drcs_link[0][index] = (((n[0] & 7) ? (n[0] & 7) : 8) << 8)
                                         + (n[1] << 4) + n[2];

            /* n[3] number of subpages ignored */
        }
#endif

        return VBI_RC_SUCCESS;
    }

    return VBI_RC_INVALID_DATA;
}

static vbi_rc_t ttx_parse_pkt1_25_vsb(ttx_decoder_t *p_dec
                                      , ttx_magazine_t *p_mag, ttx_raw_t *p_page, u8 *p_raw, u8 packet_no)
{
    vbi_rc_t    rc;

    switch(p_page->function)
    {
    case TTX_PAGE_FUNC_DISCARD:
    case TTX_PAGE_FUNC_BTT:
    case TTX_PAGE_FUNC_AIT:
    case TTX_PAGE_FUNC_MPT:
    case TTX_PAGE_FUNC_MPT_EX:
    case TTX_PAGE_FUNC_EPG:
        return VBI_RC_SUCCESS;

    case TTX_PAGE_FUNC_MOT:
        rc = ttx_parse_mot_vsb(p_dec, p_mag, p_raw, packet_no);
        if(rc != VBI_RC_SUCCESS)
            return rc;

        break;

    case TTX_PAGE_FUNC_GPOP:
    case TTX_PAGE_FUNC_POP:
        rc = ttx_parse_pop_vsb(p_dec, p_page, p_raw, packet_no);
        if(rc != VBI_RC_SUCCESS)
            return rc;

        break;

    case TTX_PAGE_FUNC_GDRCS:
    case TTX_PAGE_FUNC_DRCS:
        memcpy(p_page->data.unknown.raw[packet_no], p_raw, 40);
        break;

    case TTX_PAGE_FUNC_SUBTITLE:
    case TTX_PAGE_FUNC_LOP:
    {
        u32 i = 0;

        if(p_dec->display_page.page_no == p_page->page_no
                && (p_dec->display_page.sub_no == p_page->sub_no
                    || p_dec->display_page.page_no == TTX_FIRST_SUBPAGE
                    || p_dec->display_page.page_no == TTX_ANY_SUBPAGE
                    || p_dec->display_page.page_no == TTX_NULL_SUBPAGE))
        {
            for(i = 0; i < 40; i ++)
            {
                if(vbi_unpar8_vsb(p_raw[i]) >= 0)
                {
                    if(p_page->data.unknown.raw[packet_no][i] != p_raw[i])
                    {
                        p_page->control_bits |= C8_UPDATE;
                        p_page->data.unknown.raw[packet_no][i] = p_raw[i];
                    }
                }
                else//error handle
                {
                    if(TTX_PAGE_FUNC_SUBTITLE != p_page->function) //fix bug9340(redmine)
                    {
                        p_page->data.unknown.raw[packet_no][i] = 0x20;
                    }
                }
            }
        }
        else
        {
            for(i = 0; i < 40; i ++)
            {
                if(vbi_unpar8_vsb(p_raw[i]) >= 0)
                    p_page->data.unknown.raw[packet_no][i] = p_raw[i];
                else//error handle
                {
                    if(TTX_PAGE_FUNC_SUBTITLE != p_page->function) //fix bug9340(redmine)
                    {
                        p_page->data.unknown.raw[packet_no][i] = 0x20;
                    }
                }
            }
        }

        break;
    }
    case TTX_PAGE_FUNC_EACEMT_TRIGGER:
    case TTX_PAGE_FUNC_MIP:
    default:
        memcpy(p_page->data.unknown.raw[packet_no], p_raw, 40);
        break;
    }

    p_page->lop_packets |= 1 << packet_no;

    return VBI_RC_SUCCESS;
}

static vbi_rc_t ttx_parse_pkt26_vsb(ttx_decoder_t *p_dec
                                    , ttx_raw_t *p_page, u8 *p_raw, u8 packet_no)
{
    ttx_triplet_t   triplet;
    s32             trip_no = 0;
    s32             dc      = 0;
    s32             i       = 0;

    switch(p_page->function)
    {
    case TTX_PAGE_FUNC_DISCARD:
        return VBI_RC_SUCCESS;

    case TTX_PAGE_FUNC_GPOP:
    case TTX_PAGE_FUNC_POP:
        return (ttx_parse_pop_vsb(p_dec, p_page, p_raw, packet_no));

    case TTX_PAGE_FUNC_GDRCS:
    case TTX_PAGE_FUNC_DRCS:
    case TTX_PAGE_FUNC_BTT:
    case TTX_PAGE_FUNC_AIT:
    case TTX_PAGE_FUNC_MPT:
    case TTX_PAGE_FUNC_MPT_EX:
        OS_PRINTF("TTX!:    packet X/26 is invaild in this page[func=%d]\n"
                  , p_page->function);
        return VBI_RC_SUCCESS;

    case TTX_PAGE_FUNC_EACEMT_TRIGGER:
    default:
        break;
    }

    if((dc = vbi_unham8_vsb (*p_raw)) < 0)
        return VBI_RC_INVALID_DATA;

#if 1
    trip_no = 0;

    for(p_raw ++, i = 0; i < 13; p_raw += 3, i ++, trip_no  ++)
    {
        s32 t = vbi_unham24p_vsb (p_raw);

        if(t < 0)
            continue;

        triplet.address = (u8)(t & 0x3F);
        triplet.mode    = (u8)((t >> 6) & 0x1F);
        triplet.data    = (u8)(t >> 11);

        p_page->data.enh_lop.trip[dc * 13 + i] = triplet;
    }

    if(i >= 13 && !(p_page->x26_designations & (1 << dc)))
    {
        p_page->data.enh_lop.trip_no += trip_no;
        p_page->x26_designations     |= 1 << dc;
    }
#else
    trip_no = p_page->data.enh_lop.trip_no;


    if(trip_no >= 16 * 13 || trip_no != dc * 13)
    {
        //p_page->data.enh_lop.trip_no = 0;
        OS_PRINTF(("TTX!:    triplet number error, trip_no=%d, dc=%d\n"
                   , trip_no, dc));
        return (FALSE);
    }

    for(p_raw ++, i = 0; i < 13; p_raw += 3, i ++)
    {
        s32 t = vbi_unham24p_vsb (p_raw);

        if(t < 0)
            break;

        triplet.address = t & 0x3F;
        triplet.mode    = (t >> 6) & 0x1F;
        triplet.data    = t >> 11;

        p_page->data.enh_lop.trip[trip_no ++] = triplet;
    }

    p_page->data.enh_lop.trip_no = trip_no;
    p_page->x26_designations |= 1 << dc;
#endif
    return VBI_RC_SUCCESS;
}

static vbi_rc_t ttx_parse_pkt27_vsb(ttx_decoder_t *p_dec
                                    , u8 *p_raw, ttx_raw_t *p_page, u8 mag_no)
{
    s32 dc = 0, control = 0;
    s32 i = 0;

    if(p_page->function == TTX_PAGE_FUNC_DISCARD)
        return VBI_RC_SUCCESS;

    if((dc = vbi_unham8_vsb(*p_raw)) < 0)
        return VBI_RC_INVALID_DATA;

    switch(dc)
    {
    case 0:
        if((control = vbi_unham8_vsb(p_raw[37])) < 0)
            return VBI_RC_INVALID_DATA;

        p_page->data.unknown.have_flof = control >> 3; /* display row 24 */

        /* fall through */
    case 1:
    case 2:
    case 3:
        for(p_raw ++, i = 0; i <= 5; p_raw += 6, i  ++)
            ttx_unham_page_link_vsb(p_page->data.unknown.link + dc * 6 + i
                                    , p_raw, mag_no);

        break;

    case 4:
    case 5:
        for(p_raw ++, i = 0; i <= 5; p_raw += 6, i ++)
        {
            u8  mag0 = 0;
            s32 t1 = 0, t2 = 0;

            t1 = vbi_unham24p_vsb (p_raw + 0);
            t2 = vbi_unham24p_vsb (p_raw + 3);

            if((t1 | t2) < 0)
                return VBI_RC_INVALID_DATA;

            if(((t1 >> 10) & 1) != 0 || (t2 & 1) != 0)
                return VBI_RC_SUCCESS;

            p_page->data.unknown.link[dc * 6 + i].function
                = (ttx_page_function_t)(t1 & 3);
            p_page->data.unknown.link[dc * 6 + i].sub_no
                = (s16)((t2 >> 3) & 0xFFFF);

            mag0   = (mag_no == 8) ? 0 : mag_no;
            mag0   = (u8)(((t1 >> 12) & 0x7) ^ mag0);
            mag_no = (mag0 == 0) ? 8 : mag0;
            p_page->data.unknown.link[dc * 6 + i].page_no
                = (s16)(mag_no * 256
                        + ((t1 >> 11) & 0x0F0) + ((t1 >> 7) & 0x00F));
        }

        break;
    }

    return VBI_RC_SUCCESS;
}

/*
 *  Teletext packets 28 and 29, Level 2.5/3.5 enhancement
 */
static vbi_rc_t ttx_parse_pkt28_29_vsb(ttx_decoder_t *p_dec
                                       , u8 *p_d, ttx_raw_t *p_page, u8 mag_no, u8 packet_no)
{
    s32             dc = 0, function = 0;
    s32             triplets[13], *p_triplet = triplets;
    ttx_extension_t *p_ext = NULL;
    s32             i = 0, j = 0, err = 0;
    bs_ttx_t            bs;

    if((dc = vbi_unham8_vsb (*p_d)) < 0)
        return VBI_RC_INVALID_DATA;

    for(p_d ++, i = 0; i < 13; p_d += 3, i ++)
    {
        j = p_triplet[i] = vbi_unham24p_vsb (p_d);
        err |= j;
    }

    bs_ttx_init(&bs, p_triplet, 13);

    switch(dc)
    {
    case 0:    /* X/28/0, M/29/0 Level 2.5 */
    case 4:    /* X/28/4, M/29/4 Level 3.5 */
        if(err < 0)
            return VBI_RC_INVALID_DATA;

        function = bs_ttx_read(&bs, 4);
        bs_ttx_skip(&bs, 3); /* page coding ignored */

        /*
            X/28/0 Format 2, distinguish how?
        */
        ///TODO:
        if(function != TTX_PAGE_FUNC_LOP && packet_no == 28)
        {
            if(p_page->function != TTX_PAGE_FUNC_UNKOWN
                    && p_page->function != function)
            {
                //OS_PRINTF(("TTX!:    page 0x%x, func NO match,"))
                //OS_PRINTF((" old func[%d], func in X/28[%d]\n"
                //    , p_page->page_no, p_page->function, function));

                //MT_ASSERT(0);
                //return VBI_RC_INVALID_DATA;

                //if(function >= TTX_PAGE_FUNC_BTT
                //  || function == TTX_PAGE_FUNC_DATA)
                //    p_page->function = TTX_PAGE_FUNC_DISCARD;
            }
        }

        if(function != TTX_PAGE_FUNC_LOP)
        {
            //OS_PRINTF("TTX!:   function error,cur func=%d,dc:%d\n",function,dc);
            //return VBI_RC_SUCCESS; //for fix bug 12072(redmine) 
        }

        if(p_page->function != TTX_PAGE_FUNC_UNKOWN
                && p_page->function != TTX_PAGE_FUNC_LOP)
        {
            //OS_PRINTF("TTX!:    page function error,page func=%d,cur func=%d,dc:%d\n"
            //          , p_page->function,function,dc);
            //return VBI_RC_FAILED;  //for fix bug 12072(redmine) 
        }

        p_ext = &p_dec->mag[mag_no - 1].extension;

        if(packet_no == 28)
        {
            if(p_page->data.ext_lop.ext.designations == 0)
            {
                p_page->data.ext_lop.ext = *p_ext;
            }

            p_page->x28_designations |= 1 << dc;

            p_ext = &p_page->data.ext_lop.ext;
        }

        if(dc == 4 && (p_ext->designations & (1 << 0)))
        {
            bs_ttx_skip(&bs, 14 + 2 + 1 + 4);
        }
        else
        {
            MT_BOOL    left_panel   = FALSE;
            MT_BOOL    right_panel  = FALSE;
            u32     left_columns = 0;

            //determine to use which character set
            p_ext->charset_code[0] = (u8)(bs_ttx_read(&bs, 7));
            p_ext->charset_code[1] = (u8)(bs_ttx_read(&bs, 7));
            p_page->charset_code_set = TRUE;
            left_panel  = bs_ttx_read(&bs, 1);
            right_panel = bs_ttx_read(&bs, 1);

            /* 0 - panels required at Level 3.5 only,
               1 - at 2.5 and 3.5
               ignored. */
            bs_ttx_skip(&bs, 1);

            left_columns = bs_ttx_read(&bs, 4);

            if(left_panel && 0 == left_columns)
                left_columns = 16;

            p_ext->fallback.left_panel_columns
                = (u8)(left_columns & (- left_panel));
            p_ext->fallback.right_panel_columns
                = (u8)((16 - left_columns) & (- right_panel));
        }

        j = (dc == 4) ? 16 : 32;

        for(i = j - 16; i < j; i ++)
        {
            u32 col = bs_ttx_read(&bs, 12);

            if(i == 8)    /* transparent */
                continue;

            col = TTX_RGBA((col >> 0) & 0xFF, (col >> 4) & 0xFF
                           , (col >> 8) & 0xFF, 0xFF);

            p_ext->color_map[i] = col | (col << 4);
        }

        if(dc == 4 && (p_ext->designations & (1 << 0)))
        {
            bs_ttx_skip(&bs, 10 + 1 + 3);
        }
        else
        {
            p_ext->def_screen_color = bs_ttx_read(&bs, 5);
            p_ext->def_row_color    = bs_ttx_read(&bs, 5);

            p_ext->fallback.black_bg_substitution = bs_ttx_read(&bs, 1);

            i = bs_ttx_read(&bs, 3); /* color table remapping */

            p_ext->foreground_clut = "\00\00\00\10\10\20\20\20"[i];
            p_ext->background_clut = "\00\10\20\10\20\10\20\30"[i];
        }

        p_ext->designations |= 1 << dc;

        if(packet_no == 29)
        {
            if(0 && dc == 4)
                p_ext->designations &= ~(1 << 0);
        }

        return VBI_RC_SUCCESS;

    case 1:    /* X/28/1, M/29/1 Level 3.5 DRCS CLUT */
        p_ext = &p_dec->mag[mag_no - 1].extension;

        if(packet_no == 28)
        {
            if(p_page->data.ext_lop.ext.designations == 0)
            {
                p_page->data.ext_lop.ext = *p_ext;
            }

            p_page->x28_designations |= 1 << dc;

            p_ext = &p_page->data.ext_lop.ext;
            /*
                TODO:
                    lop?
            */
        }

        p_triplet ++;

        for(i = 0; i < 8; i ++)
            p_ext->drcs_clut[i + 2] = vbi_rev8_vsb((u8)bs_ttx_read(&bs, 5)) >> 3;

        for(i = 0; i < 32; i ++)
            p_ext->drcs_clut[i + 10] = vbi_rev8_vsb((u8)bs_ttx_read(&bs, 5)) >> 3;

        p_ext->designations |= 1 << 1;

        return VBI_RC_SUCCESS;

    case 3:    /* X/28/3 Level 2.5, 3.5 DRCS download page */
#if TTX_SUPPORT_DRCS
        if(packet_no == 29)
            break; /* M/29/3 undefined */

        if(err < 0)
            return VBI_RC_INVALID_DATA;

        function = bs_ttx_read(&bs, 4);
        bs_ttx_skip(&bs, 3); /* page coding ignored */

        if(function != TTX_PAGE_FUNC_GDRCS
                || function != TTX_PAGE_FUNC_DRCS)
            return VBI_RC_FAILED;

        if(p_page->function == TTX_PAGE_FUNC_UNKOWN)
        {
            p_page->function = function;
        }
        else if(p_page->function != function)
        {
            p_page->function = TTX_PAGE_FUNC_DISCARD;
            return VBI_RC_SUCCESS;
        }

        bs_ttx_skip(&bs, 11);

        for(i = 0; i < 48; i ++)
            p_page->data.drcs.mode[i] = bs_ttx_read(&bs, 4);

#endif

        break;
    default:
        break;
    }

    return VBI_RC_SUCCESS;
}

static vbi_rc_t ttx_parse_m8pkt30_vsb(ttx_decoder_t *p_dec, u8 *p_d, u8 packet_no)
{
    vbi_rc_t        rc;
    s32             dc = 0;
    ttx_page_link_t link;

    if((dc = vbi_unham8_vsb (*p_d)) < 0)
        return VBI_RC_INVALID_DATA;

    if(dc > 4)
        return VBI_RC_SUCCESS;


    rc = ttx_unham_page_link_vsb(&link, p_d + 1, 8);
    if(rc != VBI_RC_SUCCESS)
        return rc;

    p_dec->initial_page = link;

    return VBI_RC_SUCCESS;
}

static u8 ttx_page_language_vsb(ttx_decoder_t *p_dec
                                , ttx_raw_t *p_page, s16 page_no, u8 national)
{
    const ttx_magazine_t    *p_mag = NULL;
    const ttx_extension_t   *p_ext = NULL;
    u8                      charset_code = 0;
    u8                      lang = 0xFF;

    if(p_page != NULL)
    {
        if(p_page->function != TTX_PAGE_FUNC_LOP)
            return (lang);

        page_no  = p_page->page_no;
        national = p_page->national;
    }

    if(p_dec->max_level <= TTX_DEC_LEVEL_1P5)
        p_mag = &p_dec->def_mag;
    else
        p_mag = &p_dec->mag[((page_no >> 8) & 0xf) - 1];

    p_ext = (p_page != NULL &&p_page->x28_designations != 0)
            ? &p_page->data.ext_lop.ext : &p_mag->extension;

    charset_code = p_ext->charset_code[0];

    if(VALID_CHARACTER_SET(p_dec, charset_code) == TRUE)
        lang = charset_code;

    charset_code = (u8)((charset_code & ~7) + national);

    if(VALID_CHARACTER_SET(p_dec, charset_code) == TRUE)
        lang = charset_code;

    return (lang);
}

static vbi_rc_t ttx_store_lop_vsb(ttx_decoder_t *p_dec, ttx_raw_t *p_page)
{
    vbi_rc_t            rc;
    ttx_raw_inventory_t *p_inventory = NULL;
    u16                 sub_no_mask  = 0xffff;
    //  ttx_notify_msg_t    msg = TTX_NOTIFY_RECEIVED_PAGE;

    /*
     *  Collect information about those pages
     *  not listed in MIP etc.
     */
    rc = ttx_get_raw_page_entry_vsb(p_dec, p_page->page_no, &p_inventory);

    if(p_inventory->page_type == TTX_PAGE_TYPE_SUBTITLE)
    {
        if(p_inventory->charset_code == 0xFF)
            p_inventory->charset_code = ttx_page_language_vsb(p_dec, p_page, 0, 0);
    }
    else if(p_inventory->page_type == TTX_PAGE_TYPE_INVALID
            || p_inventory->page_type == TTX_PAGE_TYPE_UNKNOWN)
    {
        p_inventory->page_type = TTX_PAGE_TYPE_NORMAL;
    }
    else if(p_inventory->page_type == TTX_PAGE_TYPE_NO_STD_SUBPAGE)
    {
        p_page->sub_no = TTX_NULL_SUBPAGE;
        sub_no_mask    = 0;
    }

    p_inventory->cached = 1;

    //  if(p_dec->notify_func != NULL)
    //      p_dec->notify_func(msg, (u32)p_page, 0, p_dec->p_context);

    //p_page->control_bits &= ~(C8_UPDATE | C4_ERASE_PAGE);
    if(p_page->control_bits & C10_INHIBIT_DISPLAY)
        return VBI_RC_SUCCESS;

    rc = ttx_add_raw_page_vsb(p_dec, p_page, sub_no_mask);
    if(rc != VBI_RC_SUCCESS)
        return VBI_RC_FAILED;

    return VBI_RC_SUCCESS;
}

#if TTX_SUPPORT_DRCS
static vbi_rc_t ttx_convert_drcs_vsb(ttx_decoder_t *p_dec, ttx_raw_t *p_page)
{
    u8  *p_d = NULL, *d = NULL;
    s32 i = 0, j = 0, q = 0;
    u8  raw[26][40];

    memcpy(raw, p_page->data.unknown.raw, 26 * 40);

    memset(p_page->data.drcs.chars, 0xFF, TTX_DRCS_PTUS_PER_PAGE * 12 * 10 / 2);

    p_d = raw[1];
    p_page->data.drcs.invalid = 0;

    for(i = 0; i < 24; p_d += 40, i ++)
    {
        if((p_page->lop_packets & (2 << i)) != 0)
        {
            for(j = 0; j < 20; j ++)
            {
                if(vbi_unpar8_vsb (p_d[j]) < 0x40)
                {
                    p_page->data.drcs.invalid |= 1ULL << (i * 2);
                    break;
                }
            }
            for(j = 20; j < 40; j ++)
            {
                if(vbi_unpar8_vsb (p_d[j]) < 0x40)
                {
                    p_page->data.drcs.invalid |= 1ULL << (i * 2 + 1);
                    break;
                }
            }
        }
        else
        {
            p_page->data.drcs.invalid |= 3ULL << (i * 2);
        }
    }

    p_d = raw[1];
    d = p_page->data.drcs.chars[0];

    for(i = 0; i < 48; i ++)
    {
        switch(p_page->data.drcs.mode[i])
        {
        case DRCS_MODE_12B10D1:
            for(j = 0; j < 20; d += 3, j ++)
            {
                d[0] = q = expand[p_d[j] & 0x3F];
                d[1] = q >> 8;
                d[2] = q >> 16;
            }
            p_d += 20;
            break;

        case DRCS_MODE_12B10D2:
            if((p_page->data.drcs.invalid & (3ULL << i)) != 0)
            {
                p_page->data.drcs.invalid |= (3ULL << i);
                d += 60;
            }
            else
            {
                for(j = 0; j < 20; d += 3, j ++)
                {
                    q = expand[p_d[j +  0] & 0x3F]
                        + expand[p_d[j + 20] & 0x3F] * 2;
                    d[0] = q;
                    d[1] = q >> 8;
                    d[2] = q >> 16;
                }
            }
            p_d += 40;
            d += 60;
            i += 1;
            break;

        case DRCS_MODE_12B10D4:
            if((p_page->data.drcs.invalid & (15ULL << i)) != 0)
            {
                p_page->data.drcs.invalid |= (15ULL << i);
                d += 60;
            }
            else
            {
                for(j = 0; j < 20; d += 3, j ++)
                {
                    q = expand[p_d[j +  0] & 0x3F]
                        + expand[p_d[j + 20] & 0x3F] * 2
                        + expand[p_d[j + 40] & 0x3F] * 4
                        + expand[p_d[j + 60] & 0x3F] * 8;
                    d[0] = q;
                    d[1] = q >> 8;
                    d[2] = q >> 16;
                }
            }
            p_d += 80;
            d += 180;
            i += 3;
            break;

        case DRCS_MODE_6B5D4:
            for(j = 0; j < 20; p_d += 4, d += 6, j ++)
            {
                q = expand[p_d[0] & 0x3F]
                    + expand[p_d[1] & 0x3F] * 2
                    + expand[p_d[2] & 0x3F] * 4
                    + expand[p_d[3] & 0x3F] * 8;
                d[0] = (q & 15) * 0x11;
                d[1] = ((q >> 4) & 15) * 0x11;
                d[2] = ((q >> 8) & 15) * 0x11;
                d[3] = ((q >> 12) & 15) * 0x11;
                d[4] = ((q >> 16) & 15) * 0x11;
                d[5] = (q >> 20) * 0x11;
            }
            break;

        default:
            p_page->data.drcs.invalid |= (1ULL << i);
            p_d += 20;
            d += 60;
            break;
        }
    }

    memcpy(p_page->data.drcs.chars, p_page->data.drcs.chars, TTX_DRCS_PTUS_PER_PAGE * 12 * 10 / 2);

    return VBI_RC_SUCCESS;
}
#endif

static vbi_rc_t ttx_parse_mip_page_vsb(ttx_decoder_t *p_dec
                                       , ttx_raw_t *p_page, u16 page_no, u8 code, s32 *p_subp_index)
{
    u8                  *p_raw = NULL;
    s32                 subc = 0, old_code = 0;
    ttx_raw_inventory_t *p_inventory = NULL;

//    if(code < 0)  //disable by andy.chen code is unsigned
//        return VBI_RC_FAILED;

    ttx_get_raw_page_entry_vsb(p_dec, page_no, &p_inventory);

    if((code >= 0x51 && code <= 0x6F)
            || (code >= 0xD2 && code <= 0xDF)
            || (code >= 0xFA && code <= 0xFC)
            || code == 0xFF)
    {
        return VBI_RC_SUCCESS;
    }
    else if((code >= 0x02 && code <= 0x4F)
            || (code >= 0x82 && code <= 0xCF))
    {
        subc = code & 0x7F;
        code = (code >= 0x80)
               ? TTX_PAGE_TYPE_PROG_SCHEDULE : TTX_PAGE_TYPE_NORMAL;
    }
    else if(code >= 0x70 && code <= 0x77)
    {
        ttx_raw_t *p_d = NULL;

        /* p_d may be NULL. */
        ttx_get_raw_page_vsb(p_dec, page_no, 0xffff, TTX_ANY_SUBPAGE, &p_d);

        p_inventory->charset_code
            = ttx_page_language_vsb(p_dec, p_d, page_no, code & 7);

        code = TTX_PAGE_TYPE_SUBTITLE;
        subc = 0;
    }
    else if(code == 0x50 || code == 0x51
            || code == 0xD0 || code == 0xD1
            || code == 0xE0 || code == 0xE1
            || code == 0x7B || code == 0xF8)
    {
        s32 i = 0;
        if(*p_subp_index > 10 * 13)
            return VBI_RC_FAILED;

        i = *p_subp_index;
        p_raw = &p_page->data.unknown.raw[i / 13 + 15][(i % 13) * 3 + 1];
        (*p_subp_index) ++;

        if((subc = vbi_unham16p_vsb(p_raw) | (vbi_unham8_vsb(p_raw[2]) << 8)) < 0)
            return VBI_RC_INVALID_DATA;

        if((code & 15) == 1)
            subc += 1 << 12;
        else if(subc < 2)
            return VBI_RC_FAILED;

        code =  (code == 0xF8) ? 0xF8 :
                (code == 0x7B) ? 0x7B :
                (code >= 0xE0) ? TTX_PAGE_TYPE_CA_DATA_BRODCAST :
                (code >= 0xD0) ? TTX_PAGE_TYPE_PROG_SCHEDULE :
                TTX_PAGE_TYPE_NORMAL;
    }
    else
    {
//        code = code;
        subc = 0;
    }

    old_code = p_inventory->page_type;

    /*
     *  When we got incorrect numbers and proved otherwise by
     *  actually receiving the page...
     */
    if(old_code == TTX_PAGE_TYPE_UNKNOWN || old_code == TTX_PAGE_TYPE_SUBTITLE
      || code != TTX_PAGE_TYPE_INVALID)
//            || code != TTX_PAGE_TYPE_INVALID || code == TTX_PAGE_TYPE_SUBTITLE)
        p_inventory->page_type = code;

    return VBI_RC_SUCCESS;
}

static vbi_rc_t ttx_parse_mip_vsb(ttx_decoder_t *p_dec, ttx_raw_t *p_page)
{
    s16 packet_no = 0, page_no = 0, i = 0;
    s32 spi = 0;

    for(packet_no = 1, page_no = p_page->page_no & 0xF00
                                 ; packet_no <= 8
            ; packet_no ++, page_no += 0x20)
    {
        if((p_page->lop_packets & (1 << packet_no)) != 0)
        {
            u8 *p_raw = p_page->data.unknown.raw[packet_no];

            for(i = 0x00; i <= 0x09; p_raw += 2, i ++)
                if(ttx_parse_mip_page_vsb(p_dec, p_page, page_no + i
                                          , (u8)vbi_unham16p_vsb(p_raw), &spi) != VBI_RC_SUCCESS)
                    return VBI_RC_FAILED;


            for(i = 0x10; i <= 0x19; p_raw += 2, i ++)
                if(ttx_parse_mip_page_vsb(p_dec, p_page, page_no + i
                                          , (u8)vbi_unham16p_vsb(p_raw), &spi) != VBI_RC_SUCCESS)
                    return VBI_RC_FAILED;
        }
    }

    for(packet_no = 9, page_no = p_page->page_no & 0xF00
                                 ; packet_no <= 14
            ; packet_no ++, page_no += 0x30)
    {
        if((p_page->lop_packets & (1 << packet_no)) != 0)
        {
            u8 *p_raw = p_page->data.unknown.raw[packet_no];

            for(i = 0x0A; i <= 0x0F; p_raw += 2, i ++)
                if(ttx_parse_mip_page_vsb(p_dec, p_page, page_no + i
                                          , (u8)vbi_unham16p_vsb(p_raw), &spi) != VBI_RC_SUCCESS)
                    return VBI_RC_FAILED;

            if(packet_no == 14) /* 0xFA ... 0xFF */
                break;

            for(i = 0x1A; i <= 0x1F; p_raw += 2, i ++)
                if(ttx_parse_mip_page_vsb(p_dec, p_page, page_no + i
                                          , (u8)vbi_unham16p_vsb(p_raw), &spi) != VBI_RC_SUCCESS)
                    return VBI_RC_FAILED;

            for(i = 0x2A; i <= 0x2F; p_raw += 2, i ++)
                if(ttx_parse_mip_page_vsb(p_dec, p_page, page_no + i
                                          , (u8)vbi_unham16p_vsb(p_raw), &spi) != VBI_RC_SUCCESS)
                    return VBI_RC_FAILED;
        }
    }

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_dec_convert_page_vsb(ttx_decoder_t *p_dec
                                  , ttx_raw_t *p_page, ttx_page_function_t new_function)
{
    vbi_rc_t    rc;
    u8         i = 0;

    if(p_page->function != TTX_PAGE_FUNC_UNKOWN)
        return VBI_RC_INVALID_DATA;

    switch(new_function)
    {
    case TTX_PAGE_FUNC_LOP:
        p_page->function = new_function;
        return VBI_RC_SUCCESS;

    case TTX_PAGE_FUNC_GPOP:
    case TTX_PAGE_FUNC_POP:
    {
        u8 raw[26][40];
        ttx_triplet_t trip[16 * 13];

        memcpy(raw, p_page->data.unknown.raw, 26 * 40);
        if(p_page->x26_designations != 0)
        {
            memcpy(trip, p_page->data.enh_lop.trip, 16 * 13 * sizeof(ttx_triplet_t));
        }

        memset(p_page->data.pop.pointer, 0xFF
               , sizeof(p_page->data.pop.pointer));
        memset(p_page->data.pop.triplet, 0xFF
               , sizeof(p_page->data.pop.triplet));


        for(i = 1; i <= 25; i ++)
        {
            if((p_page->lop_packets & (1 << i)) != 0)
            {
                rc = ttx_parse_pop_vsb(p_dec, p_page, raw[i], i);
                if(rc != VBI_RC_FAILED)
                    return rc;
            }
        }

        if(p_page->x26_designations != 0)
        {
            memcpy(&p_page->data.pop.triplet[23 * 13]
                   , trip
                   , 16 * 13 * sizeof (ttx_triplet_t));
        }

        break;
    }
    case TTX_PAGE_FUNC_GDRCS:
    case TTX_PAGE_FUNC_DRCS:
#if TTX_SUPPORT_DRCS
        rc = ttx_convert_drcs_vsb(p_dec, p_page);
        if(rc != VBI_RC_SUCCESS)
            return rc;
#endif

        break;

    default:
        return VBI_RC_FAILED;
    }

    return VBI_RC_SUCCESS;
}

static ttx_page_priority_t ttx_get_raw_priority_vsb(
    ttx_page_function_t function, s16 page_no, u16 sub_no)
{
    switch(function)
    {
    case TTX_PAGE_FUNC_GPOP:
    case TTX_PAGE_FUNC_POP:
    case TTX_PAGE_FUNC_DRCS:
    case TTX_PAGE_FUNC_GDRCS:
        return TTX_PAGE_PRI_RESOURCE;
        break;

    case TTX_PAGE_FUNC_SUBTITLE:
    case TTX_PAGE_FUNC_LOP:
    default:
        if(sub_no == 0 || sub_no == 1)
        {
            if((page_no & 0xff) == 0)
            {
                return TTX_PAGE_PRI_1;
            }
            else if((((page_no & 0xf00) >> 8) == ((page_no & 0xf0) >> 4))
                    && (((page_no & 0xf00) >> 8) == ((page_no & 0xf) >> 0)))
            {
                return TTX_PAGE_PRI_2;
            }
            else
            {
                return TTX_PAGE_PRI_3;
            }
        }
        else
        {
            return TTX_PAGE_PRI_LOWEST;
        }
        break;
    }
}
/*!
 * EN 300706 9.3.1.3 Control bits:C11
 * Contents:When set to '1' the service is designated to be in Serial mode and the transmission of
 * a page is terminated by the next page header with a different page number.
 * When set to '0' the service is designated to be in Parallel mode and the transmission
 * of a page is terminated by the next page header with a different page number but the
 * same magazine number.
 * The same setting shall be used for all page headers in the service
 */
static MT_BOOL check_all_lines_of_page_received(u16 page_no_1, u16 page_no_2, u16 serial)
{
    u16 mag_no_1 = 0;
    u16 mag_no_2 = 0;
    u16 page_1   = 0;
    u16 page_2   = 0;

    page_1 = page_no_1 & 0x00FF;
    page_2 = page_no_2 & 0x00FF;

    mag_no_1 = page_no_1 & 0x0F00;
    mag_no_2 = page_no_2 & 0x0F00;

    if(0 != serial)
    {
        return (page_1 != page_2);
    }

    return ((page_1 != page_2) && (mag_no_1 == mag_no_2));
}

static void check_notify_new_page_received(ttx_decoder_t *p_dec, u16 page_no)
{
    MT_BOOL       all_received = FALSE;
    u8         mag_no       = 0;
    ttx_raw_t *p_cache      = NULL;

    if((NULL == p_dec) || (NULL == p_dec->notify_func))
    {
        return;
    }

    mag_no = (p_dec->waiting_page.page_no >> 8) & 0x0F;

    p_cache = p_dec->mag[mag_no - 1].p_cache;

    if(NULL == p_cache)
    {
        return;
    }

    if(FALSE == p_dec->is_waiting_page_coming)
    {
        if(page_no == p_dec->waiting_page.page_no)
        {
            p_dec->is_waiting_page_coming = TRUE;
        }

        return;
    }

    if(p_dec->waiting_page.page_no != p_cache->page_no)
    {
        p_dec->is_waiting_page_coming = FALSE;
        return;
    }

    /*!
     * p_dec->incoming_page_no is received last time, page_no is received this time
     *
     */
    all_received = check_all_lines_of_page_received(p_dec->waiting_page.page_no,
                   page_no,
                   (p_cache->control_bits & C11_MAGAZINE_SERIAL));
    if(FALSE == all_received)
    {
        return;
    }

    p_dec->last_page = p_cache->page_no;
    p_dec->last_sub = p_cache->sub_no;
    p_dec->notify_func(TTX_NOTIFY_RECEIVED_PAGE, (ulong)p_cache, 0, p_dec->p_context);

    p_dec->is_waiting_page_coming = FALSE;
}
static void ttx_dec_data_cache_ctrl(ttx_decoder_t *pdec,u8  packet_no_in,u8 mag_no_in,
    u16 page_no_in,u16 sub_no_in)
{
    //ttx_magazine_t  *p_mag   = NULL;
    ttx_raw_t       *p_cache = NULL;
    u8              mag_no  = 0;
    u16             control_bits = 0;

    if(!pdec)
    {
        return;
    }
    
    if((((page_no_in & 0xff) != 0xff)) && (0 == packet_no_in)) //subtile package should be display //for bug8591(redmine)
    {
        mag_no = (pdec->waiting_page.page_no  & 0xF00) >> 8;
        if((mag_no > 0) && (mag_no <= 8))
        {
            p_cache = pdec->mag[mag_no -1].p_cache;
            //p_mag = &pdec->mag[mag_no -1];
        }
        
        if((NULL!=p_cache) && (p_cache->page_no == pdec->waiting_page.page_no )
            && (p_cache->sub_no == pdec->waiting_page.sub_no)
            && (p_cache->control_bits & C6_SUBTITLE)
            && (p_cache->lop_packets > 1)
            && (!(p_cache->control_bits & C4_ERASE_PAGE))
            && ((page_no_in != p_cache->page_no)
            || (sub_no_in != p_cache->sub_no)))
        {
            control_bits = p_cache->control_bits;
            if(!(control_bits & C8_UPDATE))
            {
                p_cache->control_bits |=  C8_UPDATE;//force to set C8_UPDATE for display.//for bug8591(redmine)
            }

            //pdec->last_page = p_cache->page_no;
            //pdec->last_sub = p_cache->sub_no;
            
            pdec->notify_func(TTX_NOTIFY_RECEIVED_PAGE, (ulong)p_cache, 0, pdec->p_context);
            pdec->is_waiting_page_coming = FALSE;
            p_cache->control_bits = control_bits;
            p_cache->lop_packets = 1;
        }
    }
    return;
}



vbi_rc_t ttx_dec_data_process_vsb(ttx_decoder_t *p_dec, u8 *p_data, u8 ttx_type)
{
    u8  mag_no       = 0;
    u8  packet_no    = 0;
    u16 control_bits = 0;
    s16 tmp1 = 0;
    s16 tmp2 = 0;
    s16 tmp3 = 0;
    vbi_rc_t        rc = VBI_RC_FAILED;
    u16             page_no  = 0;
    u16             sub_no   = 0;
    ttx_magazine_t  *p_mag   = NULL;
    ttx_raw_t       *p_cache = NULL;
    ttx_page_link_t link;

    if(p_dec->is_running != TRUE)
        return VBI_RC_FAILED;

    if((tmp1 = vbi_unham16p_vsb (p_data)) < 0)
        return VBI_RC_INVALID_DATA;

    mag_no    = (u8)((tmp1 & 7) ? (tmp1 & 7) : 8);
    packet_no = (u8)(tmp1 >> 3);

    p_data += 2;

    if(packet_no == 0)
    {
        if((tmp1 = vbi_unham16p_vsb (p_data)) < 0)
            return VBI_RC_INVALID_DATA;

        page_no = (u16)((mag_no * 256) + (tmp1 & 0xff));
        p_dec->incoming_page_no = page_no;

        tmp1 = vbi_unham16p_vsb (p_data + 2);
        tmp2 = vbi_unham16p_vsb (p_data + 4);
        tmp3 = vbi_unham16p_vsb (p_data + 6);
        if(tmp1 < 0
                || tmp2 < 0
                || tmp3 < 0)
            return VBI_RC_INVALID_DATA;

        sub_no       = (u16)(tmp1 + tmp2 * 256) & 0x3f7f;
        control_bits = (u16)((tmp3 << 8) | (tmp2 & 0xc0)
                             | (((tmp1 & 0x80)) >> 4));
        
        link.control_bit = control_bits;////add for fixing bug 18731         
        control_bits &= ~C8_UPDATE;

        p_mag = &p_dec->mag[mag_no - 1];
        p_cache = p_dec->p_cache;

        link.page_no = page_no;
        link.sub_no  = sub_no;

        if(p_dec->notify_func != NULL)
            p_dec->notify_func(TTX_NOTIFY_TIME_UPDATE
                               , (ulong)&link
                               , (ulong)p_data
                               , p_dec->p_context);

				//fix bug 108790
		//for fix bug 12072(redmine)
		if((page_no == p_dec->initial_page.page_no) || (page_no == p_dec->waiting_page.page_no)) 
		{
			g_inital_page = 1;
		}

        ttx_dec_data_cache_ctrl(p_dec,packet_no,mag_no,page_no,sub_no);//for bug8591(redmine)


        check_notify_new_page_received(p_dec, page_no);

        if((page_no & 0xff) == 0xff)   /*  filling page    */
        {
            /*        if(p_cache != NULL)
                    {
                        ttx_page_link_t link;

                        link.page_no = page_no;
                        link.sub_no  = sub_no;
                        if(p_dec->notify_func != NULL)
                            p_dec->notify_func(TTX_NOTIFY_TIME_UPDATE
                                , (u32)&link
                                , (u32)p_data
                                , p_dec->p_context);
                    }*/

            return VBI_RC_SUCCESS;
        }
        else
        {
            if(p_cache != NULL
                    && ((p_cache->control_bits & C11_MAGAZINE_SERIAL) ||
                        (p_cache->control_bits & C6_SUBTITLE)))
            {
                if(p_cache->page_no == page_no && p_cache->sub_no == sub_no)
                {
                    if(p_cache->control_bits & C6_SUBTITLE)
                    {
                        //p_cache do nothing
                        ;
                    }
                    else
                    {
                        p_cache = NULL;
                    }
                    p_dec->last_page = 0;
                    p_dec->last_sub = 0;
                }
            }
            else
            {
                //fix bug 10164
                // p_cache = p_mag->p_cache;
                //fix bug 6874.
                //if(p_cache == NULL
                //  || (((p_cache->page_no & 0xFF) == (page_no & 0xFF))
                //  && (p_cache->sub_no == sub_no)))
                //p_cache = NULL;
            }
        }

        //while(p_cache != NULL)
        if(p_cache != NULL)
        {
#if 1 //def WARRIORS
            if((p_cache->page_no != p_dec->last_page)
                    || (p_cache->sub_no != p_dec->last_sub)
                    || ((p_cache->control_bits & (C6_SUBTITLE | C8_UPDATE)) != 0))
            {
                //     OS_PRINTF("notify pg %x  sub %x, last %x %x  func %d\n",
                //       p_cache->page_no, p_cache->sub_no, p_dec->last_page, p_dec->last_sub,
                //       p_cache->function);
                p_dec->last_page = p_cache->page_no;
                p_dec->last_sub = p_cache->sub_no;
                if((p_cache->page_no == p_dec->waiting_page.page_no) &&(p_dec->notify_func != NULL))
                {                    
                    p_dec->notify_func(TTX_NOTIFY_RECEIVED_PAGE, (ulong)p_cache, 0, p_dec->p_context);
                }
            }
#endif
            switch(p_cache->function)
            {
            case TTX_PAGE_FUNC_SUBTITLE:
            case TTX_PAGE_FUNC_LOP:
                rc = ttx_store_lop_vsb(p_dec, p_cache);
                break;

            case TTX_PAGE_FUNC_GPOP:
            case TTX_PAGE_FUNC_POP:
                rc = ttx_add_raw_page_vsb(p_dec, p_cache, 0xffff);
                break;

            case TTX_PAGE_FUNC_DRCS:
            case TTX_PAGE_FUNC_GDRCS:
#if TTX_SUPPORT_DRCS
                p_cache->priority = TTX_PAGE_PRI_RESOURCE;
                rc = ttx_convert_drcs_vsb(p_dec, p_cache);
                if(rc == VBI_RC_SUCCESS)
                {
                    rc = ttx_add_raw_page_vsb(p_dec, p_cache, 0xf);
                }
#endif
                p_cache->function = TTX_PAGE_FUNC_DISCARD;
                break;

            case TTX_PAGE_FUNC_MIP:
                rc = ttx_parse_mip_vsb(p_dec, p_cache);
                p_cache->function = TTX_PAGE_FUNC_DISCARD;
                break;

            default:
                p_cache->function = TTX_PAGE_FUNC_DISCARD;
                break;
            }

            if(p_cache->function != TTX_PAGE_FUNC_DISCARD)
                p_cache->priority
                    = ttx_get_raw_priority_vsb(p_cache->function
                                               , p_cache->page_no, p_cache->sub_no);


            if(p_cache->control_bits & C11_MAGAZINE_SERIAL
                    && p_cache->function != TTX_PAGE_FUNC_DISCARD)
            {
                p_mag->p_cache = NULL;
            }

            //break;
        }

        if(mag_no != (p_dec->waiting_page.page_no & 0xf00) >> 8)
        {
//          OS_PRINTF("not the same mag [%x][%x]return\n",mag_no, p_dec->waiting_page.page_no);
            MT_BOOL is_big_mem = vbi_is_big_mem();

            if(is_big_mem == FALSE)
            {
                if(p_cache != NULL)
                {
                    p_cache->control_bits &= ~(C8_UPDATE | C4_ERASE_PAGE);
                }
                return VBI_RC_FAILED;
            }
        }



        rc = ttx_get_raw_page_vsb(p_dec, page_no, sub_no, 0xffff, &p_cache);
        if(rc == VBI_RC_SUCCESS)
        {
            if(p_mag->p_cache != NULL
                    && p_mag->p_cache->function == TTX_PAGE_FUNC_DISCARD)
            {
                ttx_free_raw_buf_vsb(p_dec, p_mag->p_cache);
            }
            p_cache->continued = TRUE;
            //fix bug 6242, no need to remove raw page.
            //ttx_remove_raw_page(p_dec, p_cache);
            p_mag->p_cache = p_cache;
        }
        else
        {
            if(p_mag->p_cache != NULL)
            {
                if(p_mag->p_cache->function == TTX_PAGE_FUNC_DISCARD)
                {
                    memset(p_mag->p_cache, 0, sizeof(ttx_raw_t));
                    p_mag->p_cache->function = TTX_PAGE_FUNC_DISCARD;
                }
                else if(p_mag->p_cache->page_no != page_no
                        || p_mag->p_cache->sub_no != sub_no)
                {
                    p_mag->p_cache = NULL;
                }
            }

            if(p_mag->p_cache == NULL)
            {

                rc = ttx_get_raw_buf_vsb(p_dec, page_no, sub_no, &p_mag->p_cache);
                if(NULL == p_mag->p_cache)
                {
                    p_dec->p_cache = NULL;
                    return VBI_RC_FAILED;
                }
#if 0
                if(rc != VBI_RC_SUCCESS)
                {
                    ttx_page_priority_t prio =
                        ttx_get_raw_priority_vsb(TTX_PAGE_FUNC_LOP
                                                 , p_cache->page_no, p_cache->sub_no);

                    if(p_cache->page_no == p_dec->waiting_page.page_no)
                    {
                        prio = TTX_PAGE_PRI_WAITING_PAGE;
                    }
                    else
                    {
                        ttx_raw_t *p_t = NULL;
                        rc = ttx_get_raw_page_vsb(p_dec
                                                  , p_dec->waiting_page.page_no
                                                  , p_dec->waiting_page.sub_no
                                                  , 0xff
                                                  , &p_t);

                        if((rc == VBI_RC_SUCCESS)
                                && (page_no == p_t->data.lop.link[0].page_no
                                    || page_no == p_t->data.lop.link[1].page_no
                                    || page_no == p_t->data.lop.link[2].page_no
                                    || page_no == p_t->data.lop.link[3].page_no))
                        {
                            prio = TTX_PAGE_PRI_LINK_PAGE;
                        }
                        else
                        {
                            u32 tmp1 = 0, tmp2 = 0, tmp3 = 0;

                            tmp1 = ((page_no & 0xf00) >> 8) * 100
                                   + ((page_no & 0xf00) >> 8) * 10
                                   + ((page_no & 0xf00) >> 8);

                            tmp2 = ((p_t->page_no & 0xf00) >> 8) * 100
                                   + ((p_t->page_no & 0xf00) >> 8) * 10
                                   + ((p_t->page_no & 0xf00) >> 8);

                            tmp3 = p_dec->max_page_num / 5;

                            if((sub_no == 0)
                                    && (tmp1 < (tmp2 + tmp3) && tmp1 > (tmp2 - tmp3)))
                            {
                                prio = TTX_PAGE_PRI_LINK_PAGE;
                            }
                        }
                    }

                    rc = ttx_recycle_vsb(p_dec, prio, &p_mag->p_cache);
                    if(rc != VBI_RC_SUCCESS)
                    {
                        //            extern void ttx_print_db_vsb(ttx_decoder_t *p_dec);
                        MT_ASSERT(0);
                        p_mag->p_cache = NULL;
                        return VBI_RC_FAILED;
                    }
                }
#endif
            }
        }

        p_dec->p_cache        = p_mag->p_cache;

        p_cache               = p_mag->p_cache;
        p_cache->page_no      = page_no;
        p_cache->sub_no       = sub_no;
        p_cache->national     = vbi_rev8_vsb((u8)tmp3) & 7;
        p_cache->national_set = TRUE;
        p_cache->control_bits = control_bits;

        if(!(control_bits & C4_ERASE_PAGE)
                && page_no != 0x1E7
                && page_no != 0x1DF
                && p_cache->function != TTX_PAGE_FUNC_DISCARD)
        {
            if(p_cache->function == TTX_PAGE_FUNC_LOP
                    || p_cache->function == TTX_PAGE_FUNC_UNKOWN)
            {
                u32 i = 0;
                u8  *p_raw_data = p_cache->data.unknown.raw[0];

                for(i = 0; i < 8; i ++)
                {
                    *p_raw_data ++ = *p_data ++;
                }
                for(; i < 32; i ++)  //here error. data max size 42,should be 32
                {
                    if(vbi_unpar8_vsb (*p_data) >= 0)
                    {
                        *p_raw_data ++ = *p_data ++;
                    }
                }
            }
        }
        else
        {
            ttx_raw_inventory_t  *p_inventory = NULL;
            rc = ttx_get_raw_page_entry_vsb(p_dec, page_no, &p_inventory);

            if((control_bits & C6_SUBTITLE) != 0)
            {
                p_cache->function      = TTX_PAGE_FUNC_SUBTITLE;
                p_inventory->page_type = TTX_PAGE_TYPE_SUBTITLE;

                memcpy(p_cache->data.unknown.raw[0] + 0, p_data, 40);
                memset(p_cache->data.unknown.raw[0] + 40, 0x20
                       , sizeof(p_cache->data.unknown.raw) - 40);
                memset(p_cache->data.unknown.link, 0xFF
                       , sizeof(p_cache->data.unknown.link));
                memset(p_cache->data.enh_lop.trip, 0xFF
                       , sizeof(p_cache->data.enh_lop.trip));
                p_cache->data.unknown.have_flof = FALSE;
            }
            else if(page_no == 0x1BE    /*    ACI                           */
                    || page_no == 0x1F0     /*    BTT                           */
                    || page_no == 0x1E7     /*    EACEM TRIGGER                 */
                    || page_no == 0x1F1     /*    AIT                           */
                    || page_no == 0x1F2     /*    MPT & MTP-EX                  */
                    || page_no == 0x1F3     /*    assigned throgh the Basic TOP */
                    || page_no == 0x1F4)    /*    BTT                           */
            {
                p_cache->function = TTX_PAGE_FUNC_DISCARD;
            }
            else if((page_no & 0xFF) == 0xFD)
            {
                p_cache->function      = TTX_PAGE_FUNC_MIP;
                p_inventory->page_type = TTX_PAGE_TYPE_SYS_PAGE;
            }
            else if((page_no & 0xFF) == 0xFE)
            {
                p_cache->function      = TTX_PAGE_FUNC_MOT;
                p_inventory->page_type = TTX_PAGE_TYPE_SYS_PAGE;
            }
            else
            {
                p_cache->function = TTX_PAGE_FUNC_UNKOWN;

                memcpy(p_cache->data.unknown.raw[0] + 0, p_data, 40);
                if(!p_cache->continued)
                {
                    memset(p_cache->data.unknown.raw[0] + 40, 0x20
                           , sizeof(p_cache->data.unknown.raw) - 40);
                    memset(p_cache->data.unknown.link, 0xFF
                           , sizeof(p_cache->data.unknown.link));
                    memset(p_cache->data.enh_lop.trip, 0xFF
                           , sizeof(p_cache->data.enh_lop.trip));

                    p_cache->data.unknown.have_flof = FALSE;
                }
                else
                {
                    /*
                      FIXME: There is no other place to set continued field to FLASE,
                                  so set here
                    */
                    p_cache->continued = FALSE;
                }
            }

            p_cache->lop_packets      = 1;
            p_cache->x26_designations = 0;
            p_cache->x27_designations = 0;
            p_cache->x28_designations = 0;
        }

        if(p_cache->function == TTX_PAGE_FUNC_UNKOWN)
        {
            ttx_page_function_t function;
            ttx_raw_inventory_t *p_inventory = NULL;
            u8                  type         = 0;

            function = TTX_PAGE_FUNC_UNKOWN;

            rc = ttx_get_raw_page_entry_vsb(p_dec, p_cache->page_no, &p_inventory);
            if(rc == VBI_RC_SUCCESS && p_inventory != NULL)
                type = p_inventory->page_type;
            else
                type = TTX_PAGE_TYPE_UNKNOWN;



            if((type >= 0x01 && type <= 0x51)
                    || (type >= 0x70 && type <= 0x7F)
                    || (type >= 0x81 && type <= 0xD1)
                    || (type >= 0xF4 && type <= 0xF7))
            {
                function = TTX_PAGE_FUNC_LOP;
            }
            else if(type == TTX_PAGE_TYPE_SYS_PAGE)
            {
                /* no MOT or MIP?? */
            }
            else if(type == 0xE5
                    || (type >= 0xE8 && type <= 0xEB))
            {
                function = TTX_PAGE_FUNC_DRCS;
            }
            else if(type == 0xE6
                    || (type >= 0xEC && type <= 0xEF))
            {
                function = TTX_PAGE_FUNC_POP;
            }
            else if(type == TTX_PAGE_TYPE_TRIGGER_DATA
                    || type == TTX_PAGE_TYPE_TOP_BLOCK
                    || type == TTX_PAGE_TYPE_TOP_GROUP
                    || type == TTX_PAGE_TYPE_TOP_PAGE
                    || type == TTX_PAGE_TYPE_EPG_DATA
                    || type == TTX_PAGE_TYPE_ACI
                    || type == TTX_PAGE_TYPE_NOT_PUBLIC
                    || type == 0xFF                     /* reserved               */
                    || (type >= 0x52 && type <= 0x6F)   /* reserved               */
                    || (type >= 0xD2 && type <= 0xDF)   /* reserved               */
                    || (type >= 0xE0 && type <= 0xE4)   /* data broadcasting      */
                    || (type >= 0xF0 && type <= 0xF3))  /* broadcaster system page*/
            {
                function = TTX_PAGE_FUNC_DISCARD;
            }
            else
            {
                if((page_no & 0xff) <= 0x99 && (page_no & 0x0f) <= 9)
                    function = TTX_PAGE_FUNC_LOP;
            }

            if(function != TTX_PAGE_FUNC_UNKOWN
                    && function != TTX_PAGE_FUNC_DISCARD)
            {
                ttx_dec_convert_page_vsb(p_dec, p_cache, function);
            }
        }

        p_cache->data.ext_lop.ext.designations = 0;
        p_cache->data.enh_lop.trip_no          = 0;

        return VBI_RC_SUCCESS;
    }
    else if(p_dec->mag[mag_no - 1].p_cache != NULL)
    {
        p_dec->mag[mag_no - 1].p_cache->charset_code_set = FALSE;

        if(packet_no <= 25)
        {
            ttx_parse_pkt1_25_vsb(p_dec, &p_dec->mag[mag_no - 1]
                                  , p_dec->mag[mag_no - 1].p_cache, p_data, packet_no);
        }
        else if(packet_no <= 26)
        {
            return ttx_parse_pkt26_vsb(p_dec
                                       , p_dec->mag[mag_no - 1].p_cache, p_data, packet_no);
        }
        else if(packet_no <= 27)
        {
            return ttx_parse_pkt27_vsb(p_dec
                                       , p_data, p_dec->mag[mag_no - 1].p_cache, mag_no);
        }
        else if(packet_no == 28 || packet_no == 29)
        {
            if(packet_no == 28
                    && p_dec->mag[mag_no - 1].p_cache->function
                    == TTX_PAGE_FUNC_DISCARD)
                return VBI_RC_SUCCESS;

            return ttx_parse_pkt28_29_vsb(p_dec
                                          , p_data, p_dec->mag[mag_no - 1].p_cache, mag_no, packet_no);
        }
        else if(packet_no == 30)
        {
            /*    IDL packet (ETS 300 708)    */
            //vbi_rc_t         rc;
            switch(mag_no & 0xF)
            {
            case 8:    /* Packet 8/30 (ETS 300 706) */
                rc = ttx_parse_m8pkt30_vsb(p_dec, p_data, packet_no);
                if(rc != VBI_RC_SUCCESS)
                    return rc;
                break;

            default:
                break;
            }

            return VBI_RC_SUCCESS;
        }
    }

    return VBI_RC_SUCCESS;
}

static inline void ttx_extension_init_vsb(ttx_extension_t *p_ext)
{
    u8 i = 0;
    u8 tmp_charset_code[2];

    if(p_ext == NULL)
    {
        return;
    }  
    tmp_charset_code[0]=p_ext->charset_code[0];
    tmp_charset_code[1]=p_ext->charset_code[1];		
    memset(p_ext, 0, sizeof(*p_ext));
    p_ext->charset_code[0] = tmp_charset_code[0];
    p_ext->charset_code[1] = tmp_charset_code[1];		
    p_ext->def_screen_color = TTX_BLACK;    /* A.5 */
    p_ext->def_row_color    = TTX_BLACK;    /* A.5 */

    for(i = 0; i < 8; ++ i)
        p_ext->drcs_clut[2 + i] = i & 3;

    for(i = 0; i < 32; ++ i)
        p_ext->drcs_clut[2 + 8 + i] = i & 15;

    memcpy(p_ext->color_map, default_color_map, sizeof(p_ext->color_map));
}

//static inline void ttx_raw_page_init(ttx_raw_t *raw)
//{
//    hal_dma_memset(raw, 0, sizeof(*raw), DMA_HSIZE_TYPE_BYTE);
//    raw->function = TTX_PAGE_FUNC_DISCARD;
//}

static inline void ttx_magazine_init_vsb(ttx_magazine_t *p_mag)
{
    ttx_extension_init_vsb(&p_mag->extension);
    //ttx_raw_page_init(&p_mag->cache);

    /* Valid range 0 ... 7, -1 == broken link. */
    memset (p_mag->pop_lut, 0xFF, sizeof (p_mag->pop_lut));
    memset (p_mag->drcs_lut, 0xFF, sizeof (p_mag->pop_lut));

    /* NO_PAGE (pgno): (pgno & 0xFF) == 0xFF. */
    memset (p_mag->pop_link, 0xFF, sizeof (p_mag->pop_link));
#if TTX_SUPPORT_DRCS
    memset (p_mag->drcs_link, 0xFF, sizeof (p_mag->drcs_link));
#endif

    p_mag->p_raw   = NULL;
    p_mag->p_cache = NULL;
}

//static void ttx_raw_page_entry_init(ttx_raw_inventory_t *p_inventory)
//{
//    p_inventory->cached       = 0;
//    p_inventory->priority     = 0;
//    p_inventory->page_type    = TTX_PAGE_TYPE_UNKNOWN;
//    p_inventory->charset_code = 0xFF;
//}

vbi_rc_t ttx_dec_reset_vsb(ttx_decoder_t *p_dec)
{
    s32 i = 0;
    int reset_count = 0;	
    ttx_raw_inventory_t *p_inventory = NULL;
    ttx_raw_t           *p_raw       = NULL;
    ttx_raw_t     *p_next = NULL;
    p_dec->raw_cnt  = 0;
    p_dec->p_cache  = NULL;

    p_dec->is_buf_update = FALSE;

    p_dec->initial_page.function = TTX_PAGE_FUNC_LOP;
    p_dec->initial_page.page_no  = 0x100;
    p_dec->initial_page.sub_no   = TTX_FIRST_SUBPAGE;

    p_dec->display_page.function = TTX_PAGE_FUNC_LOP;
    p_dec->display_page.page_no  = TTX_NULL_PAGE_NO;
    p_dec->display_page.sub_no   = TTX_NULL_SUBPAGE;

    p_dec->waiting_page.function = TTX_PAGE_FUNC_LOP;
    p_dec->waiting_page.page_no  = 0x100;
    p_dec->waiting_page.sub_no   = TTX_FIRST_SUBPAGE;

    p_dec->input_page_no         = 0xffff;
    p_dec->incoming_page_no      = 0xffff;
    p_dec->p_font_descr          = ttx_font_descriptors;

    for(i = 0, p_inventory = p_dec->inventory; i < 800; i ++, p_inventory ++)
    {
        p_inventory->cached       = 0;
        p_inventory->priority     = 0;

        p_inventory->page_type    = TTX_PAGE_TYPE_UNKNOWN;
        p_inventory->charset_code = 0xFF;
    }

    for(i = 0, p_raw = p_dec->p_raw_page_buf; i < p_dec->max_page_num; i ++)
    {
        if(p_raw)
        {
            p_next = p_raw->p_next_buf;
            memset(p_raw, 0, sizeof(ttx_raw_t));
            p_raw->priority = TTX_PAGE_PRI_LOWEST;
            p_raw->function = TTX_PAGE_FUNC_DISCARD;
            p_raw->p_next_buf = p_next;
            p_raw = p_next;
            reset_count ++;
        }
    }
    p_dec->p_free_raw_buf = p_dec->p_raw_page_buf;

#if 0
    for(i = 0, p_raw = p_dec->p_raw_sub; i < p_dec->max_sub_page_num; i ++, p_raw ++)
    {
        memset(p_raw, 0, sizeof(ttx_raw_t));
        p_raw->priority = TTX_PAGE_PRI_LOWEST;
        p_raw->function = TTX_PAGE_FUNC_DISCARD;
        p_raw->p_next   = p_raw + 1;
    }
    (p_dec->p_raw_sub + (p_dec->max_sub_page_num - 1))->p_next = NULL;
    p_dec->p_free_raw_sub_buf = p_dec->p_raw_sub;
#endif
    p_dec->recycle = 0;

    ttx_magazine_init_vsb(&p_dec->def_mag);

    for(i = 0; i < 8; i ++)
    {
        ttx_magazine_init_vsb(&p_dec->mag[i]);
    }
		g_inital_page = 0;

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_dec_start_vsb(ttx_decoder_t *p_dec
                           , ttx_notify_t p_func, ulong cb_addr, void *p_context)
{
#if TTX_SUPPORT_DRCS
    init_expand();
#endif

    ttx_dec_reset_vsb(p_dec);

    p_dec->max_level   = TTX_DEC_LEVEL_1P5;
    p_dec->notify_func = p_func;
    p_dec->p_context   = p_context;
    p_dec->set_national_char_cb = (ttx_national_subset_cb_fun_t)cb_addr;
    p_dec->is_running  = TRUE;
    p_dec->is_display  = FALSE;
    p_dec->is_display_subtitle = FALSE;
    p_dec->is_subtitle_mode    = FALSE;
		g_inital_page = 0;

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_dec_stop_vsb(ttx_decoder_t *p_dec)
{
    p_dec->is_running = FALSE;
		g_inital_page = 0;

    return VBI_RC_SUCCESS;
}

