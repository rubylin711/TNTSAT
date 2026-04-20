/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __TTX_FORMAT_VSB_H__
#define __TTX_FORMAT_VSB_H__

/*!
  Code depends on order, don't change.
  */
typedef enum
{
    TTX_BLACK,
    TTX_RED,
    TTX_GREEN,
    TTX_YELLOW,
    TTX_BLUE,
    TTX_MAGENTA,
    TTX_CYAN,
    TTX_WHITE
} ttx_color_t;

/*!
  ABC
  */
#define TTX_RGBA(r, g, b, a)    \
        ((a << 24)              \
         | (((r) & 0xFF) << 16) \
         | (((g) & 0xFF) << 8)  \
         | (((b) & 0xFF) << 0))
/*!
  ABC
  */
#define TTX_R(rgba) (((rgba) >> 16) & 0xFF)
/*!
  ABC
  */
#define TTX_G(rgba) (((rgba) >> 8) & 0xFF)
/*!
  ABC
  */
#define TTX_B(rgba) (((rgba) >> 0) & 0xFF)
/*!
  ABC
  */
#define TTX_A(rgba) (((rgba) >> 24) & 0xFF)

/*!
  option of teletext opacity
  */
typedef enum
{
    TTX_TRANSPARENT_SPACE,
    TTX_TRANSPARENT_FULL,
    TTX_SEMI_TRANSPARENT,
    TTX_OPAQUE
} ttx_opacity_t;

/*!
  Code depends on order, don't change.
  */
typedef enum
{
    TTX_NORMAL_SIZE,    //    0x00
    TTX_DOUBLE_WIDTH,   //    0x01
    TTX_DOUBLE_HEIGHT,  //    0x02
    TTX_DOUBLE_SIZE,    //    0x03
    TTX_OVER_TOP,       //    0x04
    TTX_OVER_BOTTOM,    //    0x05
    TTX_DOUBLE_HEIGHT2, //    0x06
    TTX_DOUBLE_SIZE2    //    0x07
} ttx_size_t;

/*!
  ABC
  */
typedef struct ttx_char_mask
{
    unsigned    underline       : 1;
    unsigned    bold            : 1;
    unsigned    italic          : 1;
    unsigned    flash           : 1;
    unsigned    conceal         : 1;
    unsigned    proportional    : 1;
    unsigned    link            : 1;
    unsigned    reserved        : 1;
    unsigned    size            : 1;
    unsigned    opacity         : 1;
    unsigned    foreground      : 1;
    unsigned    background      : 1;
    unsigned    drcs_clut_offs  : 1;
    unsigned    unicode         : 1;
    unsigned    invert          : 1;
    unsigned    row_transparent : 1;
    unsigned    row_color       : 5;
} ttx_char_mask_t;

/*!
  ABC
  */
typedef struct ttx_char
{
    unsigned    underline       : 1;
    unsigned    bold            : 1;
    unsigned    italic          : 1;
    unsigned    flash           : 1;
    unsigned    conceal         : 1;
    unsigned    proportional    : 1;
    unsigned    link            : 1;
    unsigned    reserved        : 1;
    unsigned    size            : 8;
    unsigned    opacity         : 8;
    unsigned    foreground      : 8;
    unsigned    background      : 8;
    unsigned    drcs_clut_offs  : 8;
    unsigned    unicode         : 16;
} ttx_char_t;

/*!
  ABC
  */
typedef struct
{
    s16             page_no;
    s16             sub_no;
    s8              rows;
    s8              columns;
    u16             control_bits;
    ttx_char_t      text[40*25];

    struct
    {
        int         y0, y1;
        int         roll;
    } dirty;

    ttx_color_t     screen_color;
    ttx_opacity_t   screen_opacity;
    s8              user_screen_opacity;

    u32             color_map[40];
    /*!
        64 entries
      */
    u8              *p_drcs_clut;
    u8              *p_drcs[32];

    struct
    {
        s16         page_no, sub_no;
    } nav_link[6];


    ttx_font_descr_t    *p_font[2];

    ttx_opacity_t       page_opacity[2];
    ttx_opacity_t       boxed_opacity[2];
} ttx_osd_page_t;

/*!
  initialize a OSD page

  \param[in] p_dec
  \param[in] p_osd_page
  */
void ttx_osd_page_init_vsb(ttx_decoder_t *p_dec, ttx_osd_page_t *p_osd_page);

/*!
  Fomate one OSD page

  \param[in] page_no
  \param[in] p_osd_page
  \param[in] p_raw_page
  \param[in] header_only
  \param[in] navigation
  */
vbi_rc_t ttx_osd_page_format_vsb(ttx_decoder_t *p_dec
                                 , ttx_osd_page_t *p_osd_page, ttx_raw_t *p_raw_page
                                 , MT_BOOL header_only, MT_BOOL navigation);

void ttx_add_navigation_bar_vsb(ttx_osd_page_t *p_osd_page, u16 page_no);
void ttx_get_flash_line_pos(ttx_osd_page_t *p_osd_page,u8 *start_line,u8 *end_line);

#endif
