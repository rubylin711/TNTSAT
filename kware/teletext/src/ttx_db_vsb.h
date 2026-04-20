/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __TTX_DB_VSB_H__
#define __TTX_DB_VSB_H__

/*!
  define page move steps
  */
#define TTX_PAGE_STEPS (100)

/*!
  set section start page

  \param[in] page_section
  \param[in] mag_no
  \param[in] p_page_no
  */
vbi_rc_t ttx_set_section_start_page_no_vsb(ttx_decoder_t *p_dec
        , u16 page_section, u16 mag_no, u16 *p_page_no);

/*!
  set section end page

  \param[in] page_section
  \param[in] mag_no
  \param[in] p_page_no
  */
vbi_rc_t ttx_set_section_end_page_no_vsb(ttx_decoder_t *p_dec
        , u16 page_section, u16 mag_no, u16 *p_page_no);

/*!
  get page section number

  \param[in] cur_page_no
  \param[out] p_page_section
  */
vbi_rc_t ttx_get_page_section_vsb(ttx_decoder_t *p_dec, u16 cur_page_no, u16 *p_page_section);

/*!
  get next page no + 100

  \param[in] cur_page_no
  \param[out] p_next_page_no
  */
vbi_rc_t ttx_get_next_page_up_vsb(ttx_decoder_t *p_dec, u16 cur_page_no, u16 *p_next_page_no);

/*!
  get previous page no -100

  \param[in] cur_page_no
  \param[out] p_pre_page_no
  */
vbi_rc_t ttx_get_prev_page_down_vsb(ttx_decoder_t *p_dec, u16 cur_page_no, u16 *p_prev_page_no);

/*!
  get next page no

  \param[in] cur_page_no
  \param[out] p_next_page_no
  */
vbi_rc_t ttx_get_next_page_no_vsb(ttx_decoder_t *p_dec, u16 cur_page_no, u16 *p_next_page_no);

/*!
  get previous page no

  \param[in] cur_page_no
  \param[out] p_pre_page_no
  */
vbi_rc_t ttx_get_prev_page_no_vsb(ttx_decoder_t *p_dec, u16 cur_page_no, u16 *p_prev_page_no);

vbi_rc_t ttx_get_page_no_vsb(u16 cur_page_no, s16 add_page_no, u16 *p_new_page_no);
/*!
  detect whether the buffer need flush or not

  \param[in] display_page_no
  \param[in] cur_page_no
  */
MT_BOOL ttx_buf_flush_vsb(ttx_decoder_t *p_dec, u16 display_page_no, u16 cur_page_no);

/*!
  get a RAW buffer

  \param[in] pp_buf
  */
vbi_rc_t ttx_get_raw_buf_vsb(ttx_decoder_t *p_dec, u16 page_no, u16 sub_no, ttx_raw_t **pp_buf);

/*!
  free a RAW buffer

  \param[in] p_buf
  */
vbi_rc_t ttx_free_raw_buf_vsb(ttx_decoder_t *p_dec, ttx_raw_t *p_buf);

/*!
  add a RAW page

  \param[in] p_raw
  \param[in] sub_no_mask
  */
vbi_rc_t ttx_add_raw_page_vsb(ttx_decoder_t *p_dec
                              , ttx_raw_t *p_raw, u16 sub_no_mask);

/*!
  remove a RAW page

  \param[in] p_raw
  */
vbi_rc_t ttx_remove_raw_page_vsb(ttx_decoder_t *p_dec, ttx_raw_t *p_raw);

/*!
  get the page no of next RAW page

  \param[in] page_no
  \param[in] p_next_page_no
  */
vbi_rc_t ttx_get_next_raw_page_no_vsb(ttx_decoder_t *p_dec
                                      , s16 page_no, u16 *p_next_page_no);

/*!
  get the page no of prev RAW page

  \param[in] page_no
  \param[in] p_next_page_no
  */
vbi_rc_t ttx_get_prev_raw_page_no_vsb(ttx_decoder_t *p_dec
                                      , s16 page_no, u16 *p_prev_page_no);

/*!
  get an RAW page

  \param[in] page_no
  \param[in] sub_no
  \param[in] sub_no_mask
  \param[in] pp_raw
  */
vbi_rc_t ttx_get_raw_page_vsb(ttx_decoder_t *p_dec
                              , s16 page_no, u16 sub_no, u16 sub_no_mask, ttx_raw_t **pp_raw);

/*!
  get the next RAW page

  \param[in] page_no
  \param[in] sub_no
  \param[in] sub_no_mask
  \param[in] pp_raw
  */
vbi_rc_t ttx_get_next_raw_data_vsb(ttx_decoder_t *p_dec
                                   , s16 page_no, u16 sub_no, u16 sub_no_mask, ttx_raw_t **pp_raw);

/*!
  get the prev RAW page

  \param[in] page_no
  \param[in] sub_no
  \param[in] sub_no_mask
  \param[in] pp_raw
  */
vbi_rc_t ttx_get_prev_raw_data_vsb(ttx_decoder_t *p_dec
                                   , s16 page_no, u16 sub_no, u16 sub_no_mask, ttx_raw_t **pp_raw);

/*!
  get the next RAW sub-page

  \param[in] page_no
  \param[in] sub_no
  \param[in] sub_no_mask
  \param[in] pp_raw
  */
vbi_rc_t ttx_get_next_raw_subpage_vsb(ttx_decoder_t *p_dec
                                      , s16 page_no, u16 sub_no, u16 sub_no_mask, ttx_raw_t **pp_raw);

/*!
  get the prev RAW sub-page

  \param[in] page_no
  \param[in] sub_no
  \param[in] sub_no_mask
  \param[in] pp_raw
  */
vbi_rc_t ttx_get_prev_raw_subpage_vsb(ttx_decoder_t *p_dec
                                      , s16 page_no, u16 sub_no, u16 sub_no_mask, ttx_raw_t **pp_raw);

/*!
  get the entry of RAW page

  \param[in] page_no
  \param[in] pp_inventory
  */
vbi_rc_t ttx_get_raw_page_entry_vsb(ttx_decoder_t *p_dec
                                    , s16 page_no, ttx_raw_inventory_t **pp_inventory);

/*!
  delete a RAW page with low priority

  \param[in] pp_buf
  */
#if 0
vbi_rc_t ttx_recycle_vsb(ttx_decoder_t *p_dec, ttx_page_priority_t sprio, ttx_raw_t **pp_buf);
#endif

vbi_rc_t ttx_remove_all_raw_page_vsb(ttx_decoder_t *p_dec, u16 mag_no);
vbi_rc_t  ttx_remove_raw_page_vsb_from_mag(ttx_decoder_t *p_dec, u16 mag_no, u16 input_page_no, ttx_raw_t **pp_raw);

#endif