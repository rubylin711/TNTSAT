/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __TTX_LANG_VSB_H__
#define __TTX_LANG_VSB_H__

static const int pi_default_triplet[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,   // slo slk cze ces
    8,              // pol
    24,24,24,24,24, //scc scr srp hrv slv
    24,24,          //rum ron
    32,32,32,32,32, //est lit rus bul ukr
    48,48,          //gre ell
    64,64,             //ara
    88,             //heb
    16, 15,
};           //default
static const char *const ppsz_default_triplet[] =
{
    "slo", "slk", "cze", "ces", "eng","hun","cat","spa",
    "pol",
    "scc", "scr", "srp", "hrv", "slv",
    "rum", "ron",
    "est", "lit", "rus", "bul", "ukr",
    "gre", "ell",
    "ara", "per",
    "heb","tur",
    NULL
};

/*!
  character set
  */
typedef enum
{
    CHAR_SET_NONE = 0,
    CHAR_SET_LATIN_G0,
    CHAR_SET_LATIN_G2,
    CHAR_SET_CYRILLIC_1_G0,
    CHAR_SET_CYRILLIC_2_G0,
    CHAR_SET_CYRILLIC_3_G0,
    CHAR_SET_CYRILLIC_G2,
    CHAR_SET_GREEK_G0,
    CHAR_SET_GREEK_G2,
    CHAR_SET_ARABIC_G0,
    CHAR_SET_ARABIC_G2,
    CHAR_SET_HEBREW_G0,
    CHAR_SET_BLOCK_MOSAIC_G1,
    CHAR_SET_SMOOTH_MOSAIC_G3
} ttx_character_set_t;

/*!
  national subset
  */
typedef enum
{
    CHAR_SUBSET_NONE,
    CHAR_SUBSET_CZECH_SLOVAK,
    CHAR_SUBSET_ENGLISH,
    CHAR_SUBSET_ESTONIAN,
    CHAR_SUBSET_FRENCH,
    CHAR_SUBSET_GERMAN,
    CHAR_SUBSET_ITALIAN,
    CHAR_SUBSET_LETT_LITH,
    CHAR_SUBSET_POLISH,
    CHAR_SUBSET_PORTUG_SPANISH,
    CHAR_SUBSET_RUMANIAN,
    CHAR_SUBSET_SERB_CRO_SLO,
    CHAR_SUBSET_SWE_FIN_HUN,
    CHAR_SUBSET_TURKISH
} ttx_national_subset_t;

/*!
  ABC
  */
typedef struct ttx_font_descr
{
    ttx_character_set_t     G0;
    ttx_character_set_t     G2;
    /*!
      applies only to CHAR_SET_LATIN_G0
      */
    ttx_national_subset_t   subset;
} ttx_font_descr_t;

/*!
  ABC
  */
#define VALID_CHARACTER_SET(p_dec, n)               \
    ((n) < 88 && p_dec->p_font_descr[(n)].G0 != 0)

/*!
    return TRUE if unicode represents a Teletext or Closed Caption
    printable character. This excludes Teletext Arabic characters (which
    are represented by private codes U+E600 ... U+E7FF until the conversion
    table is ready), the Teletext Turkish currency sign U+E800 which is not
    representable in Unicode, the Teletext G1 Block Mosaic and G3 Smooth
    Mosaics and Line Drawing Set, with codes U+EE00 ... U+EFFF, and
    Teletext DRCS coded U+F000 ... U+F7FF.

    \param[in] unicode
  */
static inline MT_BOOL vbi_is_print_vsb(u32 unicode)
{
    return (0xE600 > unicode);
}

/*!
    return TRUE if unicode represents a Teletext G1 Block Mosaic or G3 Smooth
    Mosaics and Line Drawing Set, that is a code in range U+EE00 ... U+EFFF.

    \param[in] unicode
  */
static inline MT_BOOL vbi_is_gfx_vsb(u32 unicode)
{
    return (0xEE00 <= unicode) && (0xEFFF >= unicode);
}

/*!
    return TRUE if unicode represents a Teletext DRCS (Dynamically
    Redefinable Character), that is a code in range U+F000 ... U+F7FF.

    \param[in] unicode
  */
static inline MT_BOOL vbi_is_drcs_vsb(u32 unicode)
{
    return (0xF000 <= unicode);
}

/*!
  ABC

  \param[in] c
  \param[in] to_upper
  */
u32 vbi_caption_unicode_vsb(u32 c, MT_BOOL to_upper);

/*!
  ABC

  \param[in] c
  */
u32 vbi_teletext_unicode_vsb(ttx_character_set_t s, ttx_national_subset_t n
                             , u32 c);

/*!
  ABC

  \param[in] c
  */
u32 vbi_teletext_composed_unicode_vsb(u32 a, u32 c);

#endif
