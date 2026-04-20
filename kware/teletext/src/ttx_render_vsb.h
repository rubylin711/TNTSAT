/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __TTX_RENDER_VSB_H__
#define __TTX_RENDER_VSB_H__

/*!
  Index of background color in CLUT
  */
#define SCREEM_COLOR_INDEX      0xff

/*!
  ABC
  */
#define TTX_CHAR_W_PAL      15
/*!
  ABC
  */
#define TTX_CHAR_H_PAL      19
/*!
  ABC
  */
#define TTX_CHAR_W_SMALL      15
/*!
  ABC
  */
#define TTX_CHAR_H_SMALL      15
/*!
  ABC
  */
#define TTX_PAGE_W_SMALL      ((TTX_CHAR_W_SMALL) * 41)
/*!
  ABC
  */
#define TTX_PAGE_H_SMALL      ((TTX_CHAR_H_SMALL) * 25)
/*!
  ABC
  */
#define TTX_PAGE_W_PAL      ((TTX_CHAR_W_PAL) * 41)
/*!
  ABC
  */
#define TTX_PAGE_H_PAL      ((TTX_CHAR_H_PAL) * 25)
/*!
  ABC
  */
#define TTX_PAGE_X_PAL      ((720 - (TTX_PAGE_W_PAL)) / 2)
/*!
  ABC
  */
#define TTX_PAGE_Y_PAL      ((576 - (TTX_PAGE_H_PAL)) / 2)
/*!
  ABC
  */
#define WSTFONT2_WIDTH_PAL 480
/*!
  ABC
  */
#define WSTFONT2_HEIGHT_PAL 912
/*!
  ABC
  */
#define TTX_CHAR_W_NTSC     15
/*!
  ABC
  */
#define TTX_CHAR_H_NTSC     17
/*!
  ABC
  */
#define TTX_PAGE_W_NTSC     ((TTX_CHAR_W_NTSC) * 41)
/*!
  ABC
  */
#define TTX_PAGE_H_NTSC     ((TTX_CHAR_H_NTSC) * 25)
/*!
  ABC
  */
#define TTX_PAGE_X_NTSC     ((720 - (TTX_PAGE_W_NTSC)) / 2)
/*!
  ABC
  */
#define TTX_PAGE_Y_NTSC     ((480 - (TTX_PAGE_H_NTSC)) / 2)

/*!
  ABC
  */
#define WSTFONT2_WIDTH_NTSC 480
/*!
  ABC
  */
#define WSTFONT2_HEIGHT_NTSC 816
/*!
  ABC
  */
#define TTX_CHAR_W_HD      23
/*!
  ABC
  */
#define TTX_CHAR_H_HD      23
/*!
  ABC
  */
#define TTX_PAGE_W_HD      ((TTX_CHAR_W_HD) * 41)
/*!
  ABC
  */
#define TTX_PAGE_H_HD      ((TTX_CHAR_H_HD) * 25)
/*!
  ABC
  */
#define TTX_PAGE_X_HD      ((1280 - (TTX_PAGE_W_HD)) / 2)
/*!
  ABC
  */
#define TTX_PAGE_Y_HD      ((720 - (TTX_PAGE_H_HD)) / 2)
/*!
  ABC
  */
#define WSTFONT2_WIDTH_HD     736
/*!
  ABC
  */
#define WSTFONT2_HEIGHT_HD     1104

/*!
  Teletext character cell dimensions - hardcoded (DRCS)
  */
#define TTX_CHAR_NUM_PER_LINE ((WSTFONT2_WIDTH_PAL) / (TTX_CHAR_W_PAL))

/*!
  max is double pal char width
  */
#define TTX_CHAR_MAX_H ((TTX_CHAR_H_PAL) * 2)
/*!
  max is double pal height
  */
#define TTX_CHAR_MAX_W ((TTX_CHAR_W_PAL) * 2)

/*!
  max hd char width
  */
#define TTX_CHAR_MAX_HD_H ((TTX_CHAR_H_HD) * 2)
/*!
  max hd char height
  */
#define TTX_CHAR_MAX_HD_W ((TTX_CHAR_W_HD) * 2)

/*!
  ABC
  */
typedef struct ttx_render
{
//    void           *p_disp;
//    void           *p_gpe;
    video_std_t     video_std;
    ttx_font_size_t font_size;
    u32             page_x; //base on region, startx in region
    u32             page_y;
    u32             page_w;
    u32             page_h;
    void            *p_osd_hdl;
    void            *p_sub_hdl;
    void            *p_buf; //NULL means using inner buffer
    void            *p_buf_actual; //actual buffer
    u8              *p_char_buf; //char buffer
    u32 			char_buf_pitch;
//	u8              *p_convert_buf; //char buffer
    u32             region_w;
    u32             region_h;
    u32             clut[256];
    MT_BOOL            is_sub; //distinguish render in sub plane or osd plane
    u32             *p_odd_addr;
    u32             *p_even_addr;
//    disp_layer_id_t sub_layer_id;
	
	MT_UNF_TTX_CB_FN pfnCB;
} ttx_render_vsb_t;

/*!
  draw a page

  \param[in] p_osd_page
  \param[in] first_row
  \param[in] first_column
  \param[in] last_row
  \param[in] last_column
  \param[in] conceal
  \param[in] flash_on
  */
void ttx_render_draw_page_vsb(ttx_render_vsb_t *p_render, MT_BOOL subtile
                              , ttx_osd_page_t *p_osd_page
                              , u8 first_row, u8 last_row
                              , u8 first_column, u8 last_column
                              , MT_BOOL conceal, MT_BOOL flash_on);


/*!
  draw the page number

  \param[in] page_no
  \param[in] sub_no
  */
void ttx_render_draw_page_no_vsb(ttx_render_vsb_t *p_region
                                 , u16 page_no, u16 sub_no, MT_BOOL b_dec);

/*!
  set transparent of background

  \param[in] p_osd_page
  \param[in] percent
  */
void ttx_render_set_bg_transparent_vsb(ttx_render_vsb_t *p_render
                                       , ttx_osd_page_t *p_osd_page, u8 percent);

/*!
  create a OSD region

  \param[in] p_render
  */
vbi_rc_t ttx_render_create_region_vsb(ttx_render_vsb_t *p_render
                                      , video_std_t std);
/*!
  delete a OSD region

  \param[in] p_render
  */
vbi_rc_t ttx_render_delete_region_vsb(ttx_render_vsb_t *p_render);

/*!
  create a SUB region

  \param[in] p_render
  */
vbi_rc_t ttx_render_create_sub_region_vsb(ttx_render_vsb_t *p_render
        , video_std_t std);
/*!
  delete a SUB region

  \param[in] p_render
  */
vbi_rc_t ttx_render_delete_sub_region_vsb(ttx_render_vsb_t *p_render);

/*!
  clear page on screem

  \param[in] color
  */
void ttx_render_clear_page_vsb(ttx_render_vsb_t *p_region, u8 color);

/*!
  clear header on screem

  \param[in] color
  */
void ttx_render_clear_header_vsb(ttx_render_vsb_t *p_render, u8 color);

#endif
