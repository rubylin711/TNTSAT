/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "optm_hal.h"

#ifndef HI_BUILD_IN_BOOT
#include "mt_debug.h"
#else
#include "mtfb_debug.h"
#include "mt_common.h"
#endif


/*!
  Write 32 bits register
  
  \param[in] addr register address
  \param[in] p_addr data to write
  */
static inline void hal_put_u32(volatile unsigned long *p_addr, 
                                                 unsigned long data)
{
  /*!
      Write 32 bits register
    */
  *p_addr = data;
}

/*!
  register ARIA_DISP_video_ctrl_1 (read/write)
  */
void reg_aria_disp_set_video_ctrl_1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_1, data);
}

mt_u32  reg_aria_disp_get_video_ctrl_1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_1);
}

void reg_aria_disp_set_video_ctrl_1_video_sel(mt_u8 data)
{
    reg_aria_disp_video_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_1;
    d.bitc.video_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_1_video_sel(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_1_t *)REG_ARIA_DISP_VIDEO_CTRL_1).bitc.video_sel;
}

void reg_aria_disp_set_video_ctrl_1_sd_hf_sel(mt_u8 data)
{
    reg_aria_disp_video_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_1;
    d.bitc.sd_hf_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_1_sd_hf_sel(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_1_t *)REG_ARIA_DISP_VIDEO_CTRL_1).bitc.sd_hf_sel;
}

void reg_aria_disp_set_video_ctrl_1_sd_vf_sel(mt_u8 data)
{
    reg_aria_disp_video_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_1;
    d.bitc.sd_vf_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_1_sd_vf_sel(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_1_t *)REG_ARIA_DISP_VIDEO_CTRL_1).bitc.sd_vf_sel;
}

void reg_aria_disp_set_video_ctrl_1_hd_hf_sel(mt_u8 data)
{
    reg_aria_disp_video_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_1;
    d.bitc.hd_hf_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_1_hd_hf_sel(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_1_t *)REG_ARIA_DISP_VIDEO_CTRL_1).bitc.hd_hf_sel;
}

void reg_aria_disp_set_video_ctrl_1_hd_vf_sel(mt_u8 data)
{
    reg_aria_disp_video_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_1;
    d.bitc.hd_vf_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_1_hd_vf_sel(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_1_t *)REG_ARIA_DISP_VIDEO_CTRL_1).bitc.hd_vf_sel;
}

void reg_aria_disp_set_video_ctrl_1_sd_video_cut_en(mt_u8 data)
{
    reg_aria_disp_video_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_1;
    d.bitc.sd_video_cut_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_1_sd_video_cut_en(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_1_t *)REG_ARIA_DISP_VIDEO_CTRL_1).bitc.sd_video_cut_en;
}

void reg_aria_disp_set_video_ctrl_1_hd_video_cut_en(mt_u8 data)
{
    reg_aria_disp_video_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_1;
    d.bitc.hd_video_cut_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_1_hd_video_cut_en(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_1_t *)REG_ARIA_DISP_VIDEO_CTRL_1).bitc.hd_video_cut_en;
}

void reg_aria_disp_set_video_ctrl_1_reg_latch_top_or_bot(mt_u8 data)
{
    reg_aria_disp_video_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_1;
    d.bitc.reg_latch_top_or_bot = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_1_reg_latch_top_or_bot(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_1_t *)REG_ARIA_DISP_VIDEO_CTRL_1).bitc.reg_latch_top_or_bot;
}

void reg_aria_disp_set_video_ctrl_1_reg_latch_or_not(mt_u8 data)
{
    reg_aria_disp_video_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_1;
    d.bitc.reg_latch_or_not = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_1_reg_latch_or_not(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_1_t *)REG_ARIA_DISP_VIDEO_CTRL_1).bitc.reg_latch_or_not;
}

void reg_aria_disp_set_video_ctrl_1_only_use_first_reg_set(mt_u8 data)
{
    reg_aria_disp_video_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_1;
    d.bitc.only_use_first_reg_set = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_1_only_use_first_reg_set(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_1_t *)REG_ARIA_DISP_VIDEO_CTRL_1).bitc.only_use_first_reg_set;
}


/*!
  register ARIA_DISP_video_ctrl_2 (read/write)
  */
void reg_aria_disp_set_video_ctrl_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_2, data);
}

mt_u32  reg_aria_disp_get_video_ctrl_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_2);
}

void reg_aria_disp_set_video_ctrl_2_small_picture_upscaling_en(mt_u8 data)
{
    reg_aria_disp_video_ctrl_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_2;
    d.bitc.small_picture_upscaling_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_2, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_2_small_picture_upscaling_en(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_2_t *)REG_ARIA_DISP_VIDEO_CTRL_2).bitc.small_picture_upscaling_en;
}

void reg_aria_disp_set_video_ctrl_2_luma_post_en(mt_u8 data)
{
    reg_aria_disp_video_ctrl_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_2;
    d.bitc.luma_post_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_2, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_2_luma_post_en(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_2_t *)REG_ARIA_DISP_VIDEO_CTRL_2).bitc.luma_post_en;
}

void reg_aria_disp_set_video_ctrl_2_cbcr_swap(mt_u8 data)
{
    reg_aria_disp_video_ctrl_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_2;
    d.bitc.cbcr_swap = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_2, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_2_cbcr_swap(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_2_t *)REG_ARIA_DISP_VIDEO_CTRL_2).bitc.cbcr_swap;
}

void reg_aria_disp_set_video_ctrl_2_vid_rd_4k_process_en(mt_u8 data)
{
    reg_aria_disp_video_ctrl_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_2;
    d.bitc.vid_rd_4k_process_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_2, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_2_vid_rd_4k_process_en(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_2_t *)REG_ARIA_DISP_VIDEO_CTRL_2).bitc.vid_rd_4k_process_en;
}

void reg_aria_disp_set_video_ctrl_2_video_plane_alpha(mt_u8 data)
{
    reg_aria_disp_video_ctrl_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_2;
    d.bitc.video_plane_alpha = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_2, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_2_video_plane_alpha(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_2_t *)REG_ARIA_DISP_VIDEO_CTRL_2).bitc.video_plane_alpha;
}

void reg_aria_disp_set_video_ctrl_2_video_display_field(mt_u8 data)
{
    reg_aria_disp_video_ctrl_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CTRL_2;
    d.bitc.video_display_field = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CTRL_2, d.all);
}

mt_u8   reg_aria_disp_get_video_ctrl_2_video_display_field(void)
{
    return (*(volatile reg_aria_disp_video_ctrl_2_t *)REG_ARIA_DISP_VIDEO_CTRL_2).bitc.video_display_field;
}


/*!
  register ARIA_DISP_video_input_frame_size (read/write)
  */
void reg_aria_disp_set_video_input_frame_size(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_INPUT_FRAME_SIZE, data);
}

mt_u32  reg_aria_disp_get_video_input_frame_size(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_INPUT_FRAME_SIZE);
}

void reg_aria_disp_set_video_input_frame_size_video_input_frame_height(mt_u16 data)
{
    reg_aria_disp_video_input_frame_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_INPUT_FRAME_SIZE;
    d.bitc.video_input_frame_height = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_INPUT_FRAME_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_video_input_frame_size_video_input_frame_height(void)
{
    return (*(volatile reg_aria_disp_video_input_frame_size_t *)REG_ARIA_DISP_VIDEO_INPUT_FRAME_SIZE).bitc.video_input_frame_height;
}

void reg_aria_disp_set_video_input_frame_size_video_input_frame_width(mt_u16 data)
{
    reg_aria_disp_video_input_frame_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_INPUT_FRAME_SIZE;
    d.bitc.video_input_frame_width = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_INPUT_FRAME_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_video_input_frame_size_video_input_frame_width(void)
{
    return (*(volatile reg_aria_disp_video_input_frame_size_t *)REG_ARIA_DISP_VIDEO_INPUT_FRAME_SIZE).bitc.video_input_frame_width;
}


/*!
  register ARIA_DISP_video_crop_en (read/write)
  */
void reg_aria_disp_set_video_crop_en(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CROP_EN, data);
}

mt_u32  reg_aria_disp_get_video_crop_en(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CROP_EN);
}

void reg_aria_disp_set_video_crop_en_video_crop_en(mt_u8 data)
{
    reg_aria_disp_video_crop_en_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CROP_EN;
    d.bitc.video_crop_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CROP_EN, d.all);
}

mt_u8   reg_aria_disp_get_video_crop_en_video_crop_en(void)
{
    return (*(volatile reg_aria_disp_video_crop_en_t *)REG_ARIA_DISP_VIDEO_CROP_EN).bitc.video_crop_en;
}


/*!
  register ARIA_DISP_video_crop_pixel (read/write)
  */
void reg_aria_disp_set_video_crop_pixel(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CROP_PIXEL, data);
}

mt_u32  reg_aria_disp_get_video_crop_pixel(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CROP_PIXEL);
}

void reg_aria_disp_set_video_crop_pixel_crop_end_pixel(mt_u16 data)
{
    reg_aria_disp_video_crop_pixel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CROP_PIXEL;
    d.bitc.crop_end_pixel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CROP_PIXEL, d.all);
}

mt_u16  reg_aria_disp_get_video_crop_pixel_crop_end_pixel(void)
{
    return (*(volatile reg_aria_disp_video_crop_pixel_t *)REG_ARIA_DISP_VIDEO_CROP_PIXEL).bitc.crop_end_pixel;
}

void reg_aria_disp_set_video_crop_pixel_crop_start_pixel(mt_u16 data)
{
    reg_aria_disp_video_crop_pixel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CROP_PIXEL;
    d.bitc.crop_start_pixel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CROP_PIXEL, d.all);
}

mt_u16  reg_aria_disp_get_video_crop_pixel_crop_start_pixel(void)
{
    return (*(volatile reg_aria_disp_video_crop_pixel_t *)REG_ARIA_DISP_VIDEO_CROP_PIXEL).bitc.crop_start_pixel;
}


/*!
  register ARIA_DISP_video_crop_line (read/write)
  */
void reg_aria_disp_set_video_crop_line(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CROP_LINE, data);
}

mt_u32  reg_aria_disp_get_video_crop_line(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CROP_LINE);
}

void reg_aria_disp_set_video_crop_line_crop_end_line(mt_u16 data)
{
    reg_aria_disp_video_crop_line_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CROP_LINE;
    d.bitc.crop_end_line = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CROP_LINE, d.all);
}

mt_u16  reg_aria_disp_get_video_crop_line_crop_end_line(void)
{
    return (*(volatile reg_aria_disp_video_crop_line_t *)REG_ARIA_DISP_VIDEO_CROP_LINE).bitc.crop_end_line;
}

void reg_aria_disp_set_video_crop_line_crop_start_line(mt_u16 data)
{
    reg_aria_disp_video_crop_line_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_CROP_LINE;
    d.bitc.crop_start_line = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_CROP_LINE, d.all);
}

mt_u16  reg_aria_disp_get_video_crop_line_crop_start_line(void)
{
    return (*(volatile reg_aria_disp_video_crop_line_t *)REG_ARIA_DISP_VIDEO_CROP_LINE).bitc.crop_start_line;
}


/*!
  register ARIA_DISP_video_pix_align_ctrl (read/write)
  */
void reg_aria_disp_set_video_pix_align_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_PIX_ALIGN_CTRL, data);
}

mt_u32  reg_aria_disp_get_video_pix_align_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_PIX_ALIGN_CTRL);
}

void reg_aria_disp_set_video_pix_align_ctrl_line_end_cut(mt_u8 data)
{
    reg_aria_disp_video_pix_align_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_PIX_ALIGN_CTRL;
    d.bitc.line_end_cut = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_PIX_ALIGN_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_video_pix_align_ctrl_line_end_cut(void)
{
    return (*(volatile reg_aria_disp_video_pix_align_ctrl_t *)REG_ARIA_DISP_VIDEO_PIX_ALIGN_CTRL).bitc.line_end_cut;
}

void reg_aria_disp_set_video_pix_align_ctrl_line_pre_cut(mt_u8 data)
{
    reg_aria_disp_video_pix_align_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_PIX_ALIGN_CTRL;
    d.bitc.line_pre_cut = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_PIX_ALIGN_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_video_pix_align_ctrl_line_pre_cut(void)
{
    return (*(volatile reg_aria_disp_video_pix_align_ctrl_t *)REG_ARIA_DISP_VIDEO_PIX_ALIGN_CTRL).bitc.line_pre_cut;
}


/*!
  register ARIA_DISP_video_data_endian_ctrl (read/write)
  */
void reg_aria_disp_set_video_data_endian_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_DATA_ENDIAN_CTRL, data);
}

mt_u32  reg_aria_disp_get_video_data_endian_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_DATA_ENDIAN_CTRL);
}

void reg_aria_disp_set_video_data_endian_ctrl_video_data_endian_ctrl_1(mt_u8 data)
{
    reg_aria_disp_video_data_endian_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_DATA_ENDIAN_CTRL;
    d.bitc.video_data_endian_ctrl_1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_DATA_ENDIAN_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_video_data_endian_ctrl_video_data_endian_ctrl_1(void)
{
    return (*(volatile reg_aria_disp_video_data_endian_ctrl_t *)REG_ARIA_DISP_VIDEO_DATA_ENDIAN_CTRL).bitc.video_data_endian_ctrl_1;
}

void reg_aria_disp_set_video_data_endian_ctrl_video_data_endian_ctrl_2(mt_u8 data)
{
    reg_aria_disp_video_data_endian_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_DATA_ENDIAN_CTRL;
    d.bitc.video_data_endian_ctrl_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_DATA_ENDIAN_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_video_data_endian_ctrl_video_data_endian_ctrl_2(void)
{
    return (*(volatile reg_aria_disp_video_data_endian_ctrl_t *)REG_ARIA_DISP_VIDEO_DATA_ENDIAN_CTRL).bitc.video_data_endian_ctrl_2;
}


/*!
  register ARIA_DISP_video_burst_info_1 (read/write)
  */
void reg_aria_disp_set_video_burst_info_1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_BURST_INFO_1, data);
}

mt_u32  reg_aria_disp_get_video_burst_info_1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_BURST_INFO_1);
}

void reg_aria_disp_set_video_burst_info_1_output_bytes_sel(mt_u8 data)
{
    reg_aria_disp_video_burst_info_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_BURST_INFO_1;
    d.bitc.output_bytes_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_BURST_INFO_1, d.all);
}

mt_u8   reg_aria_disp_get_video_burst_info_1_output_bytes_sel(void)
{
    return (*(volatile reg_aria_disp_video_burst_info_1_t *)REG_ARIA_DISP_VIDEO_BURST_INFO_1).bitc.output_bytes_sel;
}

void reg_aria_disp_set_video_burst_info_1_burst_length_sel(mt_u8 data)
{
    reg_aria_disp_video_burst_info_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_BURST_INFO_1;
    d.bitc.burst_length_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_BURST_INFO_1, d.all);
}

mt_u8   reg_aria_disp_get_video_burst_info_1_burst_length_sel(void)
{
    return (*(volatile reg_aria_disp_video_burst_info_1_t *)REG_ARIA_DISP_VIDEO_BURST_INFO_1).bitc.burst_length_sel;
}

void reg_aria_disp_set_video_burst_info_1_video_linear_addr_en(mt_u8 data)
{
    reg_aria_disp_video_burst_info_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_BURST_INFO_1;
    d.bitc.video_linear_addr_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_BURST_INFO_1, d.all);
}

mt_u8   reg_aria_disp_get_video_burst_info_1_video_linear_addr_en(void)
{
    return (*(volatile reg_aria_disp_video_burst_info_1_t *)REG_ARIA_DISP_VIDEO_BURST_INFO_1).bitc.video_linear_addr_en;
}

void reg_aria_disp_set_video_burst_info_1_vid_rd_stride(mt_u16 data)
{
    reg_aria_disp_video_burst_info_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_BURST_INFO_1;
    d.bitc.vid_rd_stride = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_BURST_INFO_1, d.all);
}

mt_u16  reg_aria_disp_get_video_burst_info_1_vid_rd_stride(void)
{
    return (*(volatile reg_aria_disp_video_burst_info_1_t *)REG_ARIA_DISP_VIDEO_BURST_INFO_1).bitc.vid_rd_stride;
}


/*!
  register ARIA_DISP_video_burst_info_2 (read/write)
  */
void reg_aria_disp_set_video_burst_info_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_BURST_INFO_2, data);
}

mt_u32  reg_aria_disp_get_video_burst_info_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_BURST_INFO_2);
}

void reg_aria_disp_set_video_burst_info_2_line_rd_cnt_max_axi(mt_u8 data)
{
    reg_aria_disp_video_burst_info_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_BURST_INFO_2;
    d.bitc.line_rd_cnt_max_axi = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_BURST_INFO_2, d.all);
}

mt_u8   reg_aria_disp_get_video_burst_info_2_line_rd_cnt_max_axi(void)
{
    return (*(volatile reg_aria_disp_video_burst_info_2_t *)REG_ARIA_DISP_VIDEO_BURST_INFO_2).bitc.line_rd_cnt_max_axi;
}

void reg_aria_disp_set_video_burst_info_2_last_burst_length(mt_u8 data)
{
    reg_aria_disp_video_burst_info_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_BURST_INFO_2;
    d.bitc.last_burst_length = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_BURST_INFO_2, d.all);
}

mt_u8   reg_aria_disp_get_video_burst_info_2_last_burst_length(void)
{
    return (*(volatile reg_aria_disp_video_burst_info_2_t *)REG_ARIA_DISP_VIDEO_BURST_INFO_2).bitc.last_burst_length;
}

void reg_aria_disp_set_video_burst_info_2_first_burst_length(mt_u8 data)
{
    reg_aria_disp_video_burst_info_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_BURST_INFO_2;
    d.bitc.first_burst_length = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_BURST_INFO_2, d.all);
}

mt_u8   reg_aria_disp_get_video_burst_info_2_first_burst_length(void)
{
    return (*(volatile reg_aria_disp_video_burst_info_2_t *)REG_ARIA_DISP_VIDEO_BURST_INFO_2).bitc.first_burst_length;
}

void reg_aria_disp_set_video_burst_info_2_burst_info_en(mt_u8 data)
{
    reg_aria_disp_video_burst_info_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_BURST_INFO_2;
    d.bitc.burst_info_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_BURST_INFO_2, d.all);
}

mt_u8   reg_aria_disp_get_video_burst_info_2_burst_info_en(void)
{
    return (*(volatile reg_aria_disp_video_burst_info_2_t *)REG_ARIA_DISP_VIDEO_BURST_INFO_2).bitc.burst_info_en;
}


/*!
  register ARIA_DISP_video_line_rd_cnt_max (read/write)
  */
void reg_aria_disp_set_video_line_rd_cnt_max(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_LINE_RD_CNT_MAX, data);
}

mt_u32  reg_aria_disp_get_video_line_rd_cnt_max(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_LINE_RD_CNT_MAX);
}

void reg_aria_disp_set_video_line_rd_cnt_max_line_rd_cnt_max(mt_u16 data)
{
    reg_aria_disp_video_line_rd_cnt_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_LINE_RD_CNT_MAX;
    d.bitc.line_rd_cnt_max = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_LINE_RD_CNT_MAX, d.all);
}

mt_u16  reg_aria_disp_get_video_line_rd_cnt_max_line_rd_cnt_max(void)
{
    return (*(volatile reg_aria_disp_video_line_rd_cnt_max_t *)REG_ARIA_DISP_VIDEO_LINE_RD_CNT_MAX).bitc.line_rd_cnt_max;
}


/*!
  register ARIA_DISP_video_half_scale_ctrl (read/write)
  */
void reg_aria_disp_set_video_half_scale_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HALF_SCALE_CTRL, data);
}

mt_u32  reg_aria_disp_get_video_half_scale_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HALF_SCALE_CTRL);
}

void reg_aria_disp_set_video_half_scale_ctrl_horizontal_half_scale_en(mt_u8 data)
{
    reg_aria_disp_video_half_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HALF_SCALE_CTRL;
    d.bitc.horizontal_half_scale_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HALF_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_video_half_scale_ctrl_horizontal_half_scale_en(void)
{
    return (*(volatile reg_aria_disp_video_half_scale_ctrl_t *)REG_ARIA_DISP_VIDEO_HALF_SCALE_CTRL).bitc.horizontal_half_scale_en;
}

void reg_aria_disp_set_video_half_scale_ctrl_vertical_half_scale_en(mt_u8 data)
{
    reg_aria_disp_video_half_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HALF_SCALE_CTRL;
    d.bitc.vertical_half_scale_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HALF_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_video_half_scale_ctrl_vertical_half_scale_en(void)
{
    return (*(volatile reg_aria_disp_video_half_scale_ctrl_t *)REG_ARIA_DISP_VIDEO_HALF_SCALE_CTRL).bitc.vertical_half_scale_en;
}


/*!
  register ARIA_DISP_video_hf_phase (read/write)
  */
void reg_aria_disp_set_video_hf_phase(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HF_PHASE, data);
}

mt_u32  reg_aria_disp_get_video_hf_phase(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HF_PHASE);
}

void reg_aria_disp_set_video_hf_phase_sd_hf_init_phase(mt_u16 data)
{
    reg_aria_disp_video_hf_phase_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HF_PHASE;
    d.bitc.sd_hf_init_phase = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HF_PHASE, d.all);
}

mt_u16  reg_aria_disp_get_video_hf_phase_sd_hf_init_phase(void)
{
    return (*(volatile reg_aria_disp_video_hf_phase_t *)REG_ARIA_DISP_VIDEO_HF_PHASE).bitc.sd_hf_init_phase;
}

void reg_aria_disp_set_video_hf_phase_hd_hf_tapnum(mt_u8 data)
{
    reg_aria_disp_video_hf_phase_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HF_PHASE;
    d.bitc.hd_hf_tapnum = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HF_PHASE, d.all);
}

mt_u8   reg_aria_disp_get_video_hf_phase_hd_hf_tapnum(void)
{
    return (*(volatile reg_aria_disp_video_hf_phase_t *)REG_ARIA_DISP_VIDEO_HF_PHASE).bitc.hd_hf_tapnum;
}

void reg_aria_disp_set_video_hf_phase_hd_hf_init_phase(mt_u16 data)
{
    reg_aria_disp_video_hf_phase_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HF_PHASE;
    d.bitc.hd_hf_init_phase = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HF_PHASE, d.all);
}

mt_u16  reg_aria_disp_get_video_hf_phase_hd_hf_init_phase(void)
{
    return (*(volatile reg_aria_disp_video_hf_phase_t *)REG_ARIA_DISP_VIDEO_HF_PHASE).bitc.hd_hf_init_phase;
}


/*!
  register ARIA_DISP_video_scale_init_phase_offset (read/write)
  */
void reg_aria_disp_set_video_scale_init_phase_offset(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_INIT_PHASE_OFFSET, data);
}

mt_u32  reg_aria_disp_get_video_scale_init_phase_offset(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_INIT_PHASE_OFFSET);
}

void reg_aria_disp_set_video_scale_init_phase_offset_init_phase_offset(mt_u16 data)
{
    reg_aria_disp_video_scale_init_phase_offset_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_INIT_PHASE_OFFSET;
    d.bitc.init_phase_offset = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_INIT_PHASE_OFFSET, d.all);
}

mt_u16  reg_aria_disp_get_video_scale_init_phase_offset_init_phase_offset(void)
{
    return (*(volatile reg_aria_disp_video_scale_init_phase_offset_t *)REG_ARIA_DISP_VIDEO_SCALE_INIT_PHASE_OFFSET).bitc.init_phase_offset;
}


/*!
  register ARIA_DISP_video_scale_hd_ratio (read/write)
  */
void reg_aria_disp_set_video_scale_hd_ratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO, data);
}

mt_u32  reg_aria_disp_get_video_scale_hd_ratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO);
}

void reg_aria_disp_set_video_scale_hd_ratio_vratio_fra_hd(mt_u16 data)
{
    reg_aria_disp_video_scale_hd_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO;
    d.bitc.vratio_fra_hd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_video_scale_hd_ratio_vratio_fra_hd(void)
{
    return (*(volatile reg_aria_disp_video_scale_hd_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO).bitc.vratio_fra_hd;
}

void reg_aria_disp_set_video_scale_hd_ratio_vratio_int_hd(mt_u8 data)
{
    reg_aria_disp_video_scale_hd_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO;
    d.bitc.vratio_int_hd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_video_scale_hd_ratio_vratio_int_hd(void)
{
    return (*(volatile reg_aria_disp_video_scale_hd_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO).bitc.vratio_int_hd;
}

void reg_aria_disp_set_video_scale_hd_ratio_hratio_fra_hd(mt_u16 data)
{
    reg_aria_disp_video_scale_hd_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO;
    d.bitc.hratio_fra_hd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_video_scale_hd_ratio_hratio_fra_hd(void)
{
    return (*(volatile reg_aria_disp_video_scale_hd_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO).bitc.hratio_fra_hd;
}

void reg_aria_disp_set_video_scale_hd_ratio_hratio_int_hd(mt_u8 data)
{
    reg_aria_disp_video_scale_hd_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO;
    d.bitc.hratio_int_hd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_video_scale_hd_ratio_hratio_int_hd(void)
{
    return (*(volatile reg_aria_disp_video_scale_hd_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_HD_RATIO).bitc.hratio_int_hd;
}


/*!
  register ARIA_DISP_video_scale_hd_init_ratio (read/write)
  */
void reg_aria_disp_set_video_scale_hd_init_ratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO, data);
}

mt_u32  reg_aria_disp_get_video_scale_hd_init_ratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO);
}

void reg_aria_disp_set_video_scale_hd_init_ratio_bot_fra_init_hd(mt_u16 data)
{
    reg_aria_disp_video_scale_hd_init_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO;
    d.bitc.bot_fra_init_hd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_video_scale_hd_init_ratio_bot_fra_init_hd(void)
{
    return (*(volatile reg_aria_disp_video_scale_hd_init_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO).bitc.bot_fra_init_hd;
}

void reg_aria_disp_set_video_scale_hd_init_ratio_bot_int_init_hd(mt_u8 data)
{
    reg_aria_disp_video_scale_hd_init_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO;
    d.bitc.bot_int_init_hd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_video_scale_hd_init_ratio_bot_int_init_hd(void)
{
    return (*(volatile reg_aria_disp_video_scale_hd_init_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO).bitc.bot_int_init_hd;
}

void reg_aria_disp_set_video_scale_hd_init_ratio_top_fra_init_hd(mt_u16 data)
{
    reg_aria_disp_video_scale_hd_init_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO;
    d.bitc.top_fra_init_hd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_video_scale_hd_init_ratio_top_fra_init_hd(void)
{
    return (*(volatile reg_aria_disp_video_scale_hd_init_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO).bitc.top_fra_init_hd;
}

void reg_aria_disp_set_video_scale_hd_init_ratio_top_int_init_hd(mt_u8 data)
{
    reg_aria_disp_video_scale_hd_init_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO;
    d.bitc.top_int_init_hd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_video_scale_hd_init_ratio_top_int_init_hd(void)
{
    return (*(volatile reg_aria_disp_video_scale_hd_init_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_HD_INIT_RATIO).bitc.top_int_init_hd;
}


/*!
  register ARIA_DISP_video_hd_window_x (read/write)
  */
void reg_aria_disp_set_video_hd_window_x(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_WINDOW_X, data);
}

mt_u32  reg_aria_disp_get_video_hd_window_x(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_WINDOW_X);
}

void reg_aria_disp_set_video_hd_window_x_hd_video_end_x(mt_u16 data)
{
    reg_aria_disp_video_hd_window_x_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_WINDOW_X;
    d.bitc.hd_video_end_x = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_WINDOW_X, d.all);
}

mt_u16  reg_aria_disp_get_video_hd_window_x_hd_video_end_x(void)
{
    return (*(volatile reg_aria_disp_video_hd_window_x_t *)REG_ARIA_DISP_VIDEO_HD_WINDOW_X).bitc.hd_video_end_x;
}

void reg_aria_disp_set_video_hd_window_x_hd_video_start_x(mt_u16 data)
{
    reg_aria_disp_video_hd_window_x_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_WINDOW_X;
    d.bitc.hd_video_start_x = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_WINDOW_X, d.all);
}

mt_u16  reg_aria_disp_get_video_hd_window_x_hd_video_start_x(void)
{
    return (*(volatile reg_aria_disp_video_hd_window_x_t *)REG_ARIA_DISP_VIDEO_HD_WINDOW_X).bitc.hd_video_start_x;
}


/*!
  register ARIA_DISP_video_hd_window_y (read/write)
  */
void reg_aria_disp_set_video_hd_window_y(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_WINDOW_Y, data);
}

mt_u32  reg_aria_disp_get_video_hd_window_y(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_WINDOW_Y);
}

void reg_aria_disp_set_video_hd_window_y_hd_video_end_y(mt_u16 data)
{
    reg_aria_disp_video_hd_window_y_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_WINDOW_Y;
    d.bitc.hd_video_end_y = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_WINDOW_Y, d.all);
}

mt_u16  reg_aria_disp_get_video_hd_window_y_hd_video_end_y(void)
{
    return (*(volatile reg_aria_disp_video_hd_window_y_t *)REG_ARIA_DISP_VIDEO_HD_WINDOW_Y).bitc.hd_video_end_y;
}

void reg_aria_disp_set_video_hd_window_y_hd_video_start_y(mt_u16 data)
{
    reg_aria_disp_video_hd_window_y_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_WINDOW_Y;
    d.bitc.hd_video_start_y = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_WINDOW_Y, d.all);
}

mt_u16  reg_aria_disp_get_video_hd_window_y_hd_video_start_y(void)
{
    return (*(volatile reg_aria_disp_video_hd_window_y_t *)REG_ARIA_DISP_VIDEO_HD_WINDOW_Y).bitc.hd_video_start_y;
}


/*!
  register ARIA_DISP_video_hd_window_cut (read/write)
  */
void reg_aria_disp_set_video_hd_window_cut(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT, data);
}

mt_u32  reg_aria_disp_get_video_hd_window_cut(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT);
}

void reg_aria_disp_set_video_hd_window_cut_hd_video_cut_right(mt_u8 data)
{
    reg_aria_disp_video_hd_window_cut_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT;
    d.bitc.hd_video_cut_right = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT, d.all);
}

mt_u8   reg_aria_disp_get_video_hd_window_cut_hd_video_cut_right(void)
{
    return (*(volatile reg_aria_disp_video_hd_window_cut_t *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT).bitc.hd_video_cut_right;
}

void reg_aria_disp_set_video_hd_window_cut_hd_video_cut_left(mt_u8 data)
{
    reg_aria_disp_video_hd_window_cut_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT;
    d.bitc.hd_video_cut_left = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT, d.all);
}

mt_u8   reg_aria_disp_get_video_hd_window_cut_hd_video_cut_left(void)
{
    return (*(volatile reg_aria_disp_video_hd_window_cut_t *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT).bitc.hd_video_cut_left;
}

void reg_aria_disp_set_video_hd_window_cut_hd_video_cut_bottom(mt_u8 data)
{
    reg_aria_disp_video_hd_window_cut_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT;
    d.bitc.hd_video_cut_bottom = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT, d.all);
}

mt_u8   reg_aria_disp_get_video_hd_window_cut_hd_video_cut_bottom(void)
{
    return (*(volatile reg_aria_disp_video_hd_window_cut_t *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT).bitc.hd_video_cut_bottom;
}

void reg_aria_disp_set_video_hd_window_cut_hd_video_cut_top(mt_u8 data)
{
    reg_aria_disp_video_hd_window_cut_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT;
    d.bitc.hd_video_cut_top = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT, d.all);
}

mt_u8   reg_aria_disp_get_video_hd_window_cut_hd_video_cut_top(void)
{
    return (*(volatile reg_aria_disp_video_hd_window_cut_t *)REG_ARIA_DISP_VIDEO_HD_WINDOW_CUT).bitc.hd_video_cut_top;
}


/*!
  register ARIA_DISP_video_scale_sd_ratio (read/write)
  */
void reg_aria_disp_set_video_scale_sd_ratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO, data);
}

mt_u32  reg_aria_disp_get_video_scale_sd_ratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO);
}

void reg_aria_disp_set_video_scale_sd_ratio_vratio_fra_sd(mt_u16 data)
{
    reg_aria_disp_video_scale_sd_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO;
    d.bitc.vratio_fra_sd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_video_scale_sd_ratio_vratio_fra_sd(void)
{
    return (*(volatile reg_aria_disp_video_scale_sd_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO).bitc.vratio_fra_sd;
}

void reg_aria_disp_set_video_scale_sd_ratio_vratio_int_sd(mt_u8 data)
{
    reg_aria_disp_video_scale_sd_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO;
    d.bitc.vratio_int_sd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_video_scale_sd_ratio_vratio_int_sd(void)
{
    return (*(volatile reg_aria_disp_video_scale_sd_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO).bitc.vratio_int_sd;
}

void reg_aria_disp_set_video_scale_sd_ratio_hratio_fra_sd(mt_u16 data)
{
    reg_aria_disp_video_scale_sd_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO;
    d.bitc.hratio_fra_sd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_video_scale_sd_ratio_hratio_fra_sd(void)
{
    return (*(volatile reg_aria_disp_video_scale_sd_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO).bitc.hratio_fra_sd;
}

void reg_aria_disp_set_video_scale_sd_ratio_hratio_int_sd(mt_u8 data)
{
    reg_aria_disp_video_scale_sd_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO;
    d.bitc.hratio_int_sd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_video_scale_sd_ratio_hratio_int_sd(void)
{
    return (*(volatile reg_aria_disp_video_scale_sd_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_SD_RATIO).bitc.hratio_int_sd;
}


/*!
  register ARIA_DISP_video_scale_sd_init_ratio (read/write)
  */
void reg_aria_disp_set_video_scale_sd_init_ratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO, data);
}

mt_u32  reg_aria_disp_get_video_scale_sd_init_ratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO);
}

void reg_aria_disp_set_video_scale_sd_init_ratio_bot_fra_init_sd(mt_u16 data)
{
    reg_aria_disp_video_scale_sd_init_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO;
    d.bitc.bot_fra_init_sd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_video_scale_sd_init_ratio_bot_fra_init_sd(void)
{
    return (*(volatile reg_aria_disp_video_scale_sd_init_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO).bitc.bot_fra_init_sd;
}

void reg_aria_disp_set_video_scale_sd_init_ratio_bot_int_init_sd(mt_u8 data)
{
    reg_aria_disp_video_scale_sd_init_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO;
    d.bitc.bot_int_init_sd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_video_scale_sd_init_ratio_bot_int_init_sd(void)
{
    return (*(volatile reg_aria_disp_video_scale_sd_init_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO).bitc.bot_int_init_sd;
}

void reg_aria_disp_set_video_scale_sd_init_ratio_top_fra_init_sd(mt_u16 data)
{
    reg_aria_disp_video_scale_sd_init_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO;
    d.bitc.top_fra_init_sd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_video_scale_sd_init_ratio_top_fra_init_sd(void)
{
    return (*(volatile reg_aria_disp_video_scale_sd_init_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO).bitc.top_fra_init_sd;
}

void reg_aria_disp_set_video_scale_sd_init_ratio_top_int_init_sd(mt_u8 data)
{
    reg_aria_disp_video_scale_sd_init_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO;
    d.bitc.top_int_init_sd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_video_scale_sd_init_ratio_top_int_init_sd(void)
{
    return (*(volatile reg_aria_disp_video_scale_sd_init_ratio_t *)REG_ARIA_DISP_VIDEO_SCALE_SD_INIT_RATIO).bitc.top_int_init_sd;
}


/*!
  register ARIA_DISP_video_sd_window_x (read/write)
  */
void reg_aria_disp_set_video_sd_window_x(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_WINDOW_X, data);
}

mt_u32  reg_aria_disp_get_video_sd_window_x(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_WINDOW_X);
}

void reg_aria_disp_set_video_sd_window_x_sd_video_end_x(mt_u16 data)
{
    reg_aria_disp_video_sd_window_x_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_WINDOW_X;
    d.bitc.sd_video_end_x = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_WINDOW_X, d.all);
}

mt_u16  reg_aria_disp_get_video_sd_window_x_sd_video_end_x(void)
{
    return (*(volatile reg_aria_disp_video_sd_window_x_t *)REG_ARIA_DISP_VIDEO_SD_WINDOW_X).bitc.sd_video_end_x;
}

void reg_aria_disp_set_video_sd_window_x_sd_video_start_x(mt_u16 data)
{
    reg_aria_disp_video_sd_window_x_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_WINDOW_X;
    d.bitc.sd_video_start_x = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_WINDOW_X, d.all);
}

mt_u16  reg_aria_disp_get_video_sd_window_x_sd_video_start_x(void)
{
    return (*(volatile reg_aria_disp_video_sd_window_x_t *)REG_ARIA_DISP_VIDEO_SD_WINDOW_X).bitc.sd_video_start_x;
}


/*!
  register ARIA_DISP_video_sd_window_y (read/write)
  */
void reg_aria_disp_set_video_sd_window_y(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_WINDOW_Y, data);
}

mt_u32  reg_aria_disp_get_video_sd_window_y(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_WINDOW_Y);
}

void reg_aria_disp_set_video_sd_window_y_sd_video_end_y(mt_u16 data)
{
    reg_aria_disp_video_sd_window_y_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_WINDOW_Y;
    d.bitc.sd_video_end_y = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_WINDOW_Y, d.all);
}

mt_u16  reg_aria_disp_get_video_sd_window_y_sd_video_end_y(void)
{
    return (*(volatile reg_aria_disp_video_sd_window_y_t *)REG_ARIA_DISP_VIDEO_SD_WINDOW_Y).bitc.sd_video_end_y;
}

void reg_aria_disp_set_video_sd_window_y_sd_video_start_y(mt_u16 data)
{
    reg_aria_disp_video_sd_window_y_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_WINDOW_Y;
    d.bitc.sd_video_start_y = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_WINDOW_Y, d.all);
}

mt_u16  reg_aria_disp_get_video_sd_window_y_sd_video_start_y(void)
{
    return (*(volatile reg_aria_disp_video_sd_window_y_t *)REG_ARIA_DISP_VIDEO_SD_WINDOW_Y).bitc.sd_video_start_y;
}


/*!
  register ARIA_DISP_video_sd_window_cut (read/write)
  */
void reg_aria_disp_set_video_sd_window_cut(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT, data);
}

mt_u32  reg_aria_disp_get_video_sd_window_cut(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT);
}

void reg_aria_disp_set_video_sd_window_cut_sd_video_cut_right(mt_u8 data)
{
    reg_aria_disp_video_sd_window_cut_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT;
    d.bitc.sd_video_cut_right = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT, d.all);
}

mt_u8   reg_aria_disp_get_video_sd_window_cut_sd_video_cut_right(void)
{
    return (*(volatile reg_aria_disp_video_sd_window_cut_t *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT).bitc.sd_video_cut_right;
}

void reg_aria_disp_set_video_sd_window_cut_sd_video_cut_left(mt_u8 data)
{
    reg_aria_disp_video_sd_window_cut_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT;
    d.bitc.sd_video_cut_left = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT, d.all);
}

mt_u8   reg_aria_disp_get_video_sd_window_cut_sd_video_cut_left(void)
{
    return (*(volatile reg_aria_disp_video_sd_window_cut_t *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT).bitc.sd_video_cut_left;
}

void reg_aria_disp_set_video_sd_window_cut_sd_video_cut_bottom(mt_u8 data)
{
    reg_aria_disp_video_sd_window_cut_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT;
    d.bitc.sd_video_cut_bottom = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT, d.all);
}

mt_u8   reg_aria_disp_get_video_sd_window_cut_sd_video_cut_bottom(void)
{
    return (*(volatile reg_aria_disp_video_sd_window_cut_t *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT).bitc.sd_video_cut_bottom;
}

void reg_aria_disp_set_video_sd_window_cut_sd_video_cut_top(mt_u8 data)
{
    reg_aria_disp_video_sd_window_cut_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT;
    d.bitc.sd_video_cut_top = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT, d.all);
}

mt_u8   reg_aria_disp_get_video_sd_window_cut_sd_video_cut_top(void)
{
    return (*(volatile reg_aria_disp_video_sd_window_cut_t *)REG_ARIA_DISP_VIDEO_SD_WINDOW_CUT).bitc.sd_video_cut_top;
}


/*!
  register ARIA_DISP_sd_video_path_ctrl (read/write)
  */
void reg_aria_disp_set_sd_video_path_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_PATH_CTRL, data);
}

mt_u32  reg_aria_disp_get_sd_video_path_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_PATH_CTRL);
}

void reg_aria_disp_set_sd_video_path_ctrl_sd_video_path_en(mt_u8 data)
{
    reg_aria_disp_sd_video_path_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_PATH_CTRL;
    d.bitc.sd_video_path_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_PATH_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_video_path_ctrl_sd_video_path_en(void)
{
    return (*(volatile reg_aria_disp_sd_video_path_ctrl_t *)REG_ARIA_DISP_SD_VIDEO_PATH_CTRL).bitc.sd_video_path_en;
}

void reg_aria_disp_set_sd_video_path_ctrl_video_down_scale_en(mt_u8 data)
{
    reg_aria_disp_sd_video_path_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_PATH_CTRL;
    d.bitc.video_down_scale_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_PATH_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_video_path_ctrl_video_down_scale_en(void)
{
    return (*(volatile reg_aria_disp_sd_video_path_ctrl_t *)REG_ARIA_DISP_SD_VIDEO_PATH_CTRL).bitc.video_down_scale_en;
}

void reg_aria_disp_set_sd_video_path_ctrl_sd_use_hd_output_data(mt_u8 data)
{
    reg_aria_disp_sd_video_path_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_PATH_CTRL;
    d.bitc.sd_use_hd_output_data = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_PATH_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_video_path_ctrl_sd_use_hd_output_data(void)
{
    return (*(volatile reg_aria_disp_sd_video_path_ctrl_t *)REG_ARIA_DISP_SD_VIDEO_PATH_CTRL).bitc.sd_use_hd_output_data;
}


/*!
  register ARIA_DISP_hd_luma_vf_coeff_addr (read/write)
  */
void reg_aria_disp_set_hd_luma_vf_coeff_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_LUMA_VF_COEFF_ADDR, data);
}

mt_u32  reg_aria_disp_get_hd_luma_vf_coeff_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HD_LUMA_VF_COEFF_ADDR);
}

void reg_aria_disp_set_hd_luma_vf_coeff_addr_hd_luma_vf_coeff_addr(mt_u32 data)
{
    reg_aria_disp_hd_luma_vf_coeff_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_LUMA_VF_COEFF_ADDR;
    d.bitc.hd_luma_vf_coeff_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_LUMA_VF_COEFF_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_hd_luma_vf_coeff_addr_hd_luma_vf_coeff_addr(void)
{
    return (*(volatile reg_aria_disp_hd_luma_vf_coeff_addr_t *)REG_ARIA_DISP_HD_LUMA_VF_COEFF_ADDR).bitc.hd_luma_vf_coeff_addr;
}


/*!
  register ARIA_DISP_hd_luma_hf_coeff_addr (read/write)
  */
void reg_aria_disp_set_hd_luma_hf_coeff_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_LUMA_HF_COEFF_ADDR, data);
}

mt_u32  reg_aria_disp_get_hd_luma_hf_coeff_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HD_LUMA_HF_COEFF_ADDR);
}

void reg_aria_disp_set_hd_luma_hf_coeff_addr_hd_luma_hf_coeff_addr(mt_u32 data)
{
    reg_aria_disp_hd_luma_hf_coeff_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_LUMA_HF_COEFF_ADDR;
    d.bitc.hd_luma_hf_coeff_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_LUMA_HF_COEFF_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_hd_luma_hf_coeff_addr_hd_luma_hf_coeff_addr(void)
{
    return (*(volatile reg_aria_disp_hd_luma_hf_coeff_addr_t *)REG_ARIA_DISP_HD_LUMA_HF_COEFF_ADDR).bitc.hd_luma_hf_coeff_addr;
}


/*!
  register ARIA_DISP_hd_chroma_hf_coeff_addr (read/write)
  */
void reg_aria_disp_set_hd_chroma_hf_coeff_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CHROMA_HF_COEFF_ADDR, data);
}

mt_u32  reg_aria_disp_get_hd_chroma_hf_coeff_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HD_CHROMA_HF_COEFF_ADDR);
}

void reg_aria_disp_set_hd_chroma_hf_coeff_addr_hd_chroma_hf_coeff_addr(mt_u32 data)
{
    reg_aria_disp_hd_chroma_hf_coeff_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_CHROMA_HF_COEFF_ADDR;
    d.bitc.hd_chroma_hf_coeff_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CHROMA_HF_COEFF_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_hd_chroma_hf_coeff_addr_hd_chroma_hf_coeff_addr(void)
{
    return (*(volatile reg_aria_disp_hd_chroma_hf_coeff_addr_t *)REG_ARIA_DISP_HD_CHROMA_HF_COEFF_ADDR).bitc.hd_chroma_hf_coeff_addr;
}


/*!
  register ARIA_DISP_sd_luma_vf_coeff_addr (read/write)
  */
void reg_aria_disp_set_sd_luma_vf_coeff_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_LUMA_VF_COEFF_ADDR, data);
}

mt_u32  reg_aria_disp_get_sd_luma_vf_coeff_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_LUMA_VF_COEFF_ADDR);
}

void reg_aria_disp_set_sd_luma_vf_coeff_addr_sd_luma_vf_coeff_addr(mt_u32 data)
{
    reg_aria_disp_sd_luma_vf_coeff_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_LUMA_VF_COEFF_ADDR;
    d.bitc.sd_luma_vf_coeff_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_LUMA_VF_COEFF_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_sd_luma_vf_coeff_addr_sd_luma_vf_coeff_addr(void)
{
    return (*(volatile reg_aria_disp_sd_luma_vf_coeff_addr_t *)REG_ARIA_DISP_SD_LUMA_VF_COEFF_ADDR).bitc.sd_luma_vf_coeff_addr;
}


/*!
  register ARIA_DISP_sd_luma_hf_coeff_addr (read/write)
  */
void reg_aria_disp_set_sd_luma_hf_coeff_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_LUMA_HF_COEFF_ADDR, data);
}

mt_u32  reg_aria_disp_get_sd_luma_hf_coeff_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_LUMA_HF_COEFF_ADDR);
}

void reg_aria_disp_set_sd_luma_hf_coeff_addr_sd_luma_hf_coeff_addr(mt_u32 data)
{
    reg_aria_disp_sd_luma_hf_coeff_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_LUMA_HF_COEFF_ADDR;
    d.bitc.sd_luma_hf_coeff_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_LUMA_HF_COEFF_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_sd_luma_hf_coeff_addr_sd_luma_hf_coeff_addr(void)
{
    return (*(volatile reg_aria_disp_sd_luma_hf_coeff_addr_t *)REG_ARIA_DISP_SD_LUMA_HF_COEFF_ADDR).bitc.sd_luma_hf_coeff_addr;
}


/*!
  register ARIA_DISP_sd_chroma_hf_coeff_addr (read/write)
  */
void reg_aria_disp_set_sd_chroma_hf_coeff_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CHROMA_HF_COEFF_ADDR, data);
}

mt_u32  reg_aria_disp_get_sd_chroma_hf_coeff_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_CHROMA_HF_COEFF_ADDR);
}

void reg_aria_disp_set_sd_chroma_hf_coeff_addr_sd_chroma_hf_coeff_addr(mt_u32 data)
{
    reg_aria_disp_sd_chroma_hf_coeff_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_CHROMA_HF_COEFF_ADDR;
    d.bitc.sd_chroma_hf_coeff_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CHROMA_HF_COEFF_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_sd_chroma_hf_coeff_addr_sd_chroma_hf_coeff_addr(void)
{
    return (*(volatile reg_aria_disp_sd_chroma_hf_coeff_addr_t *)REG_ARIA_DISP_SD_CHROMA_HF_COEFF_ADDR).bitc.sd_chroma_hf_coeff_addr;
}


/*!
  register ARIA_DISP_video_dce_map_addr (read/write)
  */
void reg_aria_disp_set_video_dce_map_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_DCE_MAP_ADDR, data);
}

mt_u32  reg_aria_disp_get_video_dce_map_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_DCE_MAP_ADDR);
}

void reg_aria_disp_set_video_dce_map_addr_dce_map_addr(mt_u32 data)
{
    reg_aria_disp_video_dce_map_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_DCE_MAP_ADDR;
    d.bitc.dce_map_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_DCE_MAP_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_video_dce_map_addr_dce_map_addr(void)
{
    return (*(volatile reg_aria_disp_video_dce_map_addr_t *)REG_ARIA_DISP_VIDEO_DCE_MAP_ADDR).bitc.dce_map_addr;
}


/*!
  register ARIA_DISP_histo_info_addr_0 (read/write)
  */
void reg_aria_disp_set_histo_info_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HISTO_INFO_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_histo_info_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HISTO_INFO_ADDR_0);
}

void reg_aria_disp_set_histo_info_addr_0_histo_info_addr_0(mt_u32 data)
{
    reg_aria_disp_histo_info_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HISTO_INFO_ADDR_0;
    d.bitc.histo_info_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HISTO_INFO_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_histo_info_addr_0_histo_info_addr_0(void)
{
    return (*(volatile reg_aria_disp_histo_info_addr_0_t *)REG_ARIA_DISP_HISTO_INFO_ADDR_0).bitc.histo_info_addr_0;
}


/*!
  register ARIA_DISP_histo_info_addr_2 (read/write)
  */
void reg_aria_disp_set_histo_info_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HISTO_INFO_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_histo_info_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HISTO_INFO_ADDR_2);
}

void reg_aria_disp_set_histo_info_addr_2_histo_info_addr_2(mt_u32 data)
{
    reg_aria_disp_histo_info_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HISTO_INFO_ADDR_2;
    d.bitc.histo_info_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HISTO_INFO_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_histo_info_addr_2_histo_info_addr_2(void)
{
    return (*(volatile reg_aria_disp_histo_info_addr_2_t *)REG_ARIA_DISP_HISTO_INFO_ADDR_2).bitc.histo_info_addr_2;
}


/*!
  register ARIA_DISP_chroma_upscale_ctrl (read/write)
  */
void reg_aria_disp_set_chroma_upscale_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_UPSCALE_CTRL, data);
}

mt_u32  reg_aria_disp_get_chroma_upscale_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_UPSCALE_CTRL);
}

void reg_aria_disp_set_chroma_upscale_ctrl_chroma_hori_ip_mode(mt_u8 data)
{
    reg_aria_disp_chroma_upscale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_UPSCALE_CTRL;
    d.bitc.chroma_hori_ip_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_UPSCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_chroma_upscale_ctrl_chroma_hori_ip_mode(void)
{
    return (*(volatile reg_aria_disp_chroma_upscale_ctrl_t *)REG_ARIA_DISP_CHROMA_UPSCALE_CTRL).bitc.chroma_hori_ip_mode;
}

void reg_aria_disp_set_chroma_upscale_ctrl_chroma_alpha_old(mt_u16 data)
{
    reg_aria_disp_chroma_upscale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_UPSCALE_CTRL;
    d.bitc.chroma_alpha_old = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_UPSCALE_CTRL, d.all);
}

mt_u16  reg_aria_disp_get_chroma_upscale_ctrl_chroma_alpha_old(void)
{
    return (*(volatile reg_aria_disp_chroma_upscale_ctrl_t *)REG_ARIA_DISP_CHROMA_UPSCALE_CTRL).bitc.chroma_alpha_old;
}

void reg_aria_disp_set_chroma_upscale_ctrl_mono_display_en(mt_u8 data)
{
    reg_aria_disp_chroma_upscale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_UPSCALE_CTRL;
    d.bitc.mono_display_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_UPSCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_chroma_upscale_ctrl_mono_display_en(void)
{
    return (*(volatile reg_aria_disp_chroma_upscale_ctrl_t *)REG_ARIA_DISP_CHROMA_UPSCALE_CTRL).bitc.mono_display_en;
}


/*!
  register ARIA_DISP_chroma_coef_0 (read/write)
  */
void reg_aria_disp_set_chroma_coef_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_COEF_0, data);
}

mt_u32  reg_aria_disp_get_chroma_coef_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_COEF_0);
}

void reg_aria_disp_set_chroma_coef_0_chroma_coef1(mt_u16 data)
{
    reg_aria_disp_chroma_coef_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_COEF_0;
    d.bitc.chroma_coef1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_COEF_0, d.all);
}

mt_u16  reg_aria_disp_get_chroma_coef_0_chroma_coef1(void)
{
    return (*(volatile reg_aria_disp_chroma_coef_0_t *)REG_ARIA_DISP_CHROMA_COEF_0).bitc.chroma_coef1;
}

void reg_aria_disp_set_chroma_coef_0_chroma_coef0(mt_u16 data)
{
    reg_aria_disp_chroma_coef_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_COEF_0;
    d.bitc.chroma_coef0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_COEF_0, d.all);
}

mt_u16  reg_aria_disp_get_chroma_coef_0_chroma_coef0(void)
{
    return (*(volatile reg_aria_disp_chroma_coef_0_t *)REG_ARIA_DISP_CHROMA_COEF_0).bitc.chroma_coef0;
}


/*!
  register ARIA_DISP_chroma_coef_1 (read/write)
  */
void reg_aria_disp_set_chroma_coef_1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_COEF_1, data);
}

mt_u32  reg_aria_disp_get_chroma_coef_1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_COEF_1);
}

void reg_aria_disp_set_chroma_coef_1_chroma_coef3(mt_u16 data)
{
    reg_aria_disp_chroma_coef_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_COEF_1;
    d.bitc.chroma_coef3 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_COEF_1, d.all);
}

mt_u16  reg_aria_disp_get_chroma_coef_1_chroma_coef3(void)
{
    return (*(volatile reg_aria_disp_chroma_coef_1_t *)REG_ARIA_DISP_CHROMA_COEF_1).bitc.chroma_coef3;
}

void reg_aria_disp_set_chroma_coef_1_chroma_coef2(mt_u16 data)
{
    reg_aria_disp_chroma_coef_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_COEF_1;
    d.bitc.chroma_coef2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_COEF_1, d.all);
}

mt_u16  reg_aria_disp_get_chroma_coef_1_chroma_coef2(void)
{
    return (*(volatile reg_aria_disp_chroma_coef_1_t *)REG_ARIA_DISP_CHROMA_COEF_1).bitc.chroma_coef2;
}


/*!
  register ARIA_DISP_chroma_coef_2 (read/write)
  */
void reg_aria_disp_set_chroma_coef_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_COEF_2, data);
}

mt_u32  reg_aria_disp_get_chroma_coef_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_COEF_2);
}

void reg_aria_disp_set_chroma_coef_2_chroma_coef5(mt_u16 data)
{
    reg_aria_disp_chroma_coef_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_COEF_2;
    d.bitc.chroma_coef5 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_COEF_2, d.all);
}

mt_u16  reg_aria_disp_get_chroma_coef_2_chroma_coef5(void)
{
    return (*(volatile reg_aria_disp_chroma_coef_2_t *)REG_ARIA_DISP_CHROMA_COEF_2).bitc.chroma_coef5;
}

void reg_aria_disp_set_chroma_coef_2_chroma_coef4(mt_u16 data)
{
    reg_aria_disp_chroma_coef_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_COEF_2;
    d.bitc.chroma_coef4 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_COEF_2, d.all);
}

mt_u16  reg_aria_disp_get_chroma_coef_2_chroma_coef4(void)
{
    return (*(volatile reg_aria_disp_chroma_coef_2_t *)REG_ARIA_DISP_CHROMA_COEF_2).bitc.chroma_coef4;
}


/*!
  register ARIA_DISP_chroma_coef_3 (read/write)
  */
void reg_aria_disp_set_chroma_coef_3(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_COEF_3, data);
}

mt_u32  reg_aria_disp_get_chroma_coef_3(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_COEF_3);
}

void reg_aria_disp_set_chroma_coef_3_chroma_coef6(mt_u16 data)
{
    reg_aria_disp_chroma_coef_3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_COEF_3;
    d.bitc.chroma_coef6 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_COEF_3, d.all);
}

mt_u16  reg_aria_disp_get_chroma_coef_3_chroma_coef6(void)
{
    return (*(volatile reg_aria_disp_chroma_coef_3_t *)REG_ARIA_DISP_CHROMA_COEF_3).bitc.chroma_coef6;
}


/*!
  register ARIA_DISP_color_enhance_ctrl (read/write)
  */
void reg_aria_disp_set_color_enhance_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL, data);
}

mt_u32  reg_aria_disp_get_color_enhance_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL);
}

void reg_aria_disp_set_color_enhance_ctrl_color_enhance_en(mt_u8 data)
{
    reg_aria_disp_color_enhance_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL;
    d.bitc.color_enhance_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_color_enhance_ctrl_color_enhance_en(void)
{
    return (*(volatile reg_aria_disp_color_enhance_ctrl_t *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL).bitc.color_enhance_en;
}

void reg_aria_disp_set_color_enhance_ctrl_color_enhance_red_dec(mt_u8 data)
{
    reg_aria_disp_color_enhance_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL;
    d.bitc.color_enhance_red_dec = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_color_enhance_ctrl_color_enhance_red_dec(void)
{
    return (*(volatile reg_aria_disp_color_enhance_ctrl_t *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL).bitc.color_enhance_red_dec;
}

void reg_aria_disp_set_color_enhance_ctrl_color_enhance_length(mt_u8 data)
{
    reg_aria_disp_color_enhance_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL;
    d.bitc.color_enhance_length = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_color_enhance_ctrl_color_enhance_length(void)
{
    return (*(volatile reg_aria_disp_color_enhance_ctrl_t *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL).bitc.color_enhance_length;
}

void reg_aria_disp_set_color_enhance_ctrl_color_enhance_thr(mt_u8 data)
{
    reg_aria_disp_color_enhance_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL;
    d.bitc.color_enhance_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_color_enhance_ctrl_color_enhance_thr(void)
{
    return (*(volatile reg_aria_disp_color_enhance_ctrl_t *)REG_ARIA_DISP_COLOR_ENHANCE_CTRL).bitc.color_enhance_thr;
}


/*!
  register ARIA_DISP_video_dce_config (read/write)
  */
void reg_aria_disp_set_video_dce_config(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_DCE_CONFIG, data);
}

mt_u32  reg_aria_disp_get_video_dce_config(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_DCE_CONFIG);
}

void reg_aria_disp_set_video_dce_config_sd_video_dce_en(mt_u8 data)
{
    reg_aria_disp_video_dce_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_DCE_CONFIG;
    d.bitc.sd_video_dce_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_DCE_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_video_dce_config_sd_video_dce_en(void)
{
    return (*(volatile reg_aria_disp_video_dce_config_t *)REG_ARIA_DISP_VIDEO_DCE_CONFIG).bitc.sd_video_dce_en;
}

void reg_aria_disp_set_video_dce_config_hd_video_dce_en(mt_u8 data)
{
    reg_aria_disp_video_dce_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_DCE_CONFIG;
    d.bitc.hd_video_dce_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_DCE_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_video_dce_config_hd_video_dce_en(void)
{
    return (*(volatile reg_aria_disp_video_dce_config_t *)REG_ARIA_DISP_VIDEO_DCE_CONFIG).bitc.hd_video_dce_en;
}


/*!
  register ARIA_DISP_video_horf_config (read/write)
  */
void reg_aria_disp_set_video_horf_config(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HORF_CONFIG, data);
}

mt_u32  reg_aria_disp_get_video_horf_config(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HORF_CONFIG);
}

void reg_aria_disp_set_video_horf_config_sd_infl_thr(mt_u8 data)
{
    reg_aria_disp_video_horf_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HORF_CONFIG;
    d.bitc.sd_infl_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HORF_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_video_horf_config_sd_infl_thr(void)
{
    return (*(volatile reg_aria_disp_video_horf_config_t *)REG_ARIA_DISP_VIDEO_HORF_CONFIG).bitc.sd_infl_thr;
}

void reg_aria_disp_set_video_horf_config_hd_infl_thr(mt_u8 data)
{
    reg_aria_disp_video_horf_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HORF_CONFIG;
    d.bitc.hd_infl_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HORF_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_video_horf_config_hd_infl_thr(void)
{
    return (*(volatile reg_aria_disp_video_horf_config_t *)REG_ARIA_DISP_VIDEO_HORF_CONFIG).bitc.hd_infl_thr;
}

void reg_aria_disp_set_video_horf_config_start_addr(mt_u8 data)
{
    reg_aria_disp_video_horf_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HORF_CONFIG;
    d.bitc.start_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HORF_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_video_horf_config_start_addr(void)
{
    return (*(volatile reg_aria_disp_video_horf_config_t *)REG_ARIA_DISP_VIDEO_HORF_CONFIG).bitc.start_addr;
}

void reg_aria_disp_set_video_horf_config_hd_phase_type(mt_u8 data)
{
    reg_aria_disp_video_horf_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HORF_CONFIG;
    d.bitc.hd_phase_type = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HORF_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_video_horf_config_hd_phase_type(void)
{
    return (*(volatile reg_aria_disp_video_horf_config_t *)REG_ARIA_DISP_VIDEO_HORF_CONFIG).bitc.hd_phase_type;
}


/*!
  register ARIA_DISP_hd_video_post_config (read/write)
  */
void reg_aria_disp_set_hd_video_post_config(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG, data);
}

mt_u32  reg_aria_disp_get_hd_video_post_config(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG);
}

void reg_aria_disp_set_hd_video_post_config_hd_leverage(mt_u8 data)
{
    reg_aria_disp_hd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG;
    d.bitc.hd_leverage = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_hd_video_post_config_hd_leverage(void)
{
    return (*(volatile reg_aria_disp_hd_video_post_config_t *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG).bitc.hd_leverage;
}

void reg_aria_disp_set_hd_video_post_config_hd_hp_enha(mt_u8 data)
{
    reg_aria_disp_hd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG;
    d.bitc.hd_hp_enha = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_hd_video_post_config_hd_hp_enha(void)
{
    return (*(volatile reg_aria_disp_hd_video_post_config_t *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG).bitc.hd_hp_enha;
}

void reg_aria_disp_set_hd_video_post_config_hd_hori_enha(mt_u8 data)
{
    reg_aria_disp_hd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG;
    d.bitc.hd_hori_enha = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_hd_video_post_config_hd_hori_enha(void)
{
    return (*(volatile reg_aria_disp_hd_video_post_config_t *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG).bitc.hd_hori_enha;
}

void reg_aria_disp_set_hd_video_post_config_hd_shoot_cfg(mt_u8 data)
{
    reg_aria_disp_hd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG;
    d.bitc.hd_shoot_cfg = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_hd_video_post_config_hd_shoot_cfg(void)
{
    return (*(volatile reg_aria_disp_hd_video_post_config_t *)REG_ARIA_DISP_HD_VIDEO_POST_CONFIG).bitc.hd_shoot_cfg;
}


/*!
  register ARIA_DISP_sd_video_post_config (read/write)
  */
void reg_aria_disp_set_sd_video_post_config(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG, data);
}

mt_u32  reg_aria_disp_get_sd_video_post_config(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG);
}

void reg_aria_disp_set_sd_video_post_config_sd_leverage(mt_u8 data)
{
    reg_aria_disp_sd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG;
    d.bitc.sd_leverage = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_sd_video_post_config_sd_leverage(void)
{
    return (*(volatile reg_aria_disp_sd_video_post_config_t *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG).bitc.sd_leverage;
}

void reg_aria_disp_set_sd_video_post_config_sd_hp_enha(mt_u8 data)
{
    reg_aria_disp_sd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG;
    d.bitc.sd_hp_enha = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_sd_video_post_config_sd_hp_enha(void)
{
    return (*(volatile reg_aria_disp_sd_video_post_config_t *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG).bitc.sd_hp_enha;
}

void reg_aria_disp_set_sd_video_post_config_sd_hori_enha(mt_u8 data)
{
    reg_aria_disp_sd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG;
    d.bitc.sd_hori_enha = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_sd_video_post_config_sd_hori_enha(void)
{
    return (*(volatile reg_aria_disp_sd_video_post_config_t *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG).bitc.sd_hori_enha;
}

void reg_aria_disp_set_sd_video_post_config_sd_shoot_cfg(mt_u8 data)
{
    reg_aria_disp_sd_video_post_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG;
    d.bitc.sd_shoot_cfg = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG, d.all);
}

mt_u8   reg_aria_disp_get_sd_video_post_config_sd_shoot_cfg(void)
{
    return (*(volatile reg_aria_disp_sd_video_post_config_t *)REG_ARIA_DISP_SD_VIDEO_POST_CONFIG).bitc.sd_shoot_cfg;
}


/*!
  register ARIA_DISP_hd_video_effect_coef (read/write)
  */
void reg_aria_disp_set_hd_video_effect_coef(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_VIDEO_EFFECT_COEF, data);
}

mt_u32  reg_aria_disp_get_hd_video_effect_coef(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HD_VIDEO_EFFECT_COEF);
}

void reg_aria_disp_set_hd_video_effect_coef_bright_coeff(mt_u8 data)
{
    reg_aria_disp_hd_video_effect_coef_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_VIDEO_EFFECT_COEF;
    d.bitc.bright_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_VIDEO_EFFECT_COEF, d.all);
}

mt_u8   reg_aria_disp_get_hd_video_effect_coef_bright_coeff(void)
{
    return (*(volatile reg_aria_disp_hd_video_effect_coef_t *)REG_ARIA_DISP_HD_VIDEO_EFFECT_COEF).bitc.bright_coeff;
}

void reg_aria_disp_set_hd_video_effect_coef_contrast_coeff(mt_u8 data)
{
    reg_aria_disp_hd_video_effect_coef_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_VIDEO_EFFECT_COEF;
    d.bitc.contrast_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_VIDEO_EFFECT_COEF, d.all);
}

mt_u8   reg_aria_disp_get_hd_video_effect_coef_contrast_coeff(void)
{
    return (*(volatile reg_aria_disp_hd_video_effect_coef_t *)REG_ARIA_DISP_HD_VIDEO_EFFECT_COEF).bitc.contrast_coeff;
}

void reg_aria_disp_set_hd_video_effect_coef_saturation_coeff(mt_u8 data)
{
    reg_aria_disp_hd_video_effect_coef_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_VIDEO_EFFECT_COEF;
    d.bitc.saturation_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_VIDEO_EFFECT_COEF, d.all);
}

mt_u8   reg_aria_disp_get_hd_video_effect_coef_saturation_coeff(void)
{
    return (*(volatile reg_aria_disp_hd_video_effect_coef_t *)REG_ARIA_DISP_HD_VIDEO_EFFECT_COEF).bitc.saturation_coeff;
}


/*!
  register ARIA_DISP_hd_video_hue_adjust (read/write)
  */
void reg_aria_disp_set_hd_video_hue_adjust(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_VIDEO_HUE_ADJUST, data);
}

mt_u32  reg_aria_disp_get_hd_video_hue_adjust(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HD_VIDEO_HUE_ADJUST);
}

void reg_aria_disp_set_hd_video_hue_adjust_hd_sina(mt_u16 data)
{
    reg_aria_disp_hd_video_hue_adjust_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_VIDEO_HUE_ADJUST;
    d.bitc.hd_sina = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_VIDEO_HUE_ADJUST, d.all);
}

mt_u16  reg_aria_disp_get_hd_video_hue_adjust_hd_sina(void)
{
    return (*(volatile reg_aria_disp_hd_video_hue_adjust_t *)REG_ARIA_DISP_HD_VIDEO_HUE_ADJUST).bitc.hd_sina;
}

void reg_aria_disp_set_hd_video_hue_adjust_hd_cosa(mt_u16 data)
{
    reg_aria_disp_hd_video_hue_adjust_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_VIDEO_HUE_ADJUST;
    d.bitc.hd_cosa = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_VIDEO_HUE_ADJUST, d.all);
}

mt_u16  reg_aria_disp_get_hd_video_hue_adjust_hd_cosa(void)
{
    return (*(volatile reg_aria_disp_hd_video_hue_adjust_t *)REG_ARIA_DISP_HD_VIDEO_HUE_ADJUST).bitc.hd_cosa;
}

void reg_aria_disp_set_hd_video_hue_adjust_hd_hue_adjust_en(mt_u8 data)
{
    reg_aria_disp_hd_video_hue_adjust_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_VIDEO_HUE_ADJUST;
    d.bitc.hd_hue_adjust_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_VIDEO_HUE_ADJUST, d.all);
}

mt_u8   reg_aria_disp_get_hd_video_hue_adjust_hd_hue_adjust_en(void)
{
    return (*(volatile reg_aria_disp_hd_video_hue_adjust_t *)REG_ARIA_DISP_HD_VIDEO_HUE_ADJUST).bitc.hd_hue_adjust_en;
}


/*!
  register ARIA_DISP_sd_video_drop_line (read/write)
  */
void reg_aria_disp_set_sd_video_drop_line(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_DROP_LINE, data);
}

mt_u32  reg_aria_disp_get_sd_video_drop_line(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_DROP_LINE);
}

void reg_aria_disp_set_sd_video_drop_line_sd_bot_field_drop_line(mt_u8 data)
{
    reg_aria_disp_sd_video_drop_line_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_DROP_LINE;
    d.bitc.sd_bot_field_drop_line = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_DROP_LINE, d.all);
}

mt_u8   reg_aria_disp_get_sd_video_drop_line_sd_bot_field_drop_line(void)
{
    return (*(volatile reg_aria_disp_sd_video_drop_line_t *)REG_ARIA_DISP_SD_VIDEO_DROP_LINE).bitc.sd_bot_field_drop_line;
}

void reg_aria_disp_set_sd_video_drop_line_sd_top_field_drop_line(mt_u8 data)
{
    reg_aria_disp_sd_video_drop_line_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_DROP_LINE;
    d.bitc.sd_top_field_drop_line = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_DROP_LINE, d.all);
}

mt_u8   reg_aria_disp_get_sd_video_drop_line_sd_top_field_drop_line(void)
{
    return (*(volatile reg_aria_disp_sd_video_drop_line_t *)REG_ARIA_DISP_SD_VIDEO_DROP_LINE).bitc.sd_top_field_drop_line;
}


/*!
  register ARIA_DISP_video_scalar_buf_full_thr (read/write)
  */
void reg_aria_disp_set_video_scalar_buf_full_thr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALAR_BUF_FULL_THR, data);
}

mt_u32  reg_aria_disp_get_video_scalar_buf_full_thr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALAR_BUF_FULL_THR);
}

void reg_aria_disp_set_video_scalar_buf_full_thr_scaler_data_sfifo_thr(mt_u8 data)
{
    reg_aria_disp_video_scalar_buf_full_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALAR_BUF_FULL_THR;
    d.bitc.scaler_data_sfifo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALAR_BUF_FULL_THR, d.all);
}

mt_u8   reg_aria_disp_get_video_scalar_buf_full_thr_scaler_data_sfifo_thr(void)
{
    return (*(volatile reg_aria_disp_video_scalar_buf_full_thr_t *)REG_ARIA_DISP_VIDEO_SCALAR_BUF_FULL_THR).bitc.scaler_data_sfifo_thr;
}

void reg_aria_disp_set_video_scalar_buf_full_thr_di_data_sfifo_thr(mt_u8 data)
{
    reg_aria_disp_video_scalar_buf_full_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALAR_BUF_FULL_THR;
    d.bitc.di_data_sfifo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALAR_BUF_FULL_THR, d.all);
}

mt_u8   reg_aria_disp_get_video_scalar_buf_full_thr_di_data_sfifo_thr(void)
{
    return (*(volatile reg_aria_disp_video_scalar_buf_full_thr_t *)REG_ARIA_DISP_VIDEO_SCALAR_BUF_FULL_THR).bitc.di_data_sfifo_thr;
}


/*!
  register ARIA_DISP_video_scalar_outbuf_full_thr (read/write)
  */
void reg_aria_disp_set_video_scalar_outbuf_full_thr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALAR_OUTBUF_FULL_THR, data);
}

mt_u32  reg_aria_disp_get_video_scalar_outbuf_full_thr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALAR_OUTBUF_FULL_THR);
}

void reg_aria_disp_set_video_scalar_outbuf_full_thr_sd_scalar_buf_full_thr(mt_u16 data)
{
    reg_aria_disp_video_scalar_outbuf_full_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALAR_OUTBUF_FULL_THR;
    d.bitc.sd_scalar_buf_full_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALAR_OUTBUF_FULL_THR, d.all);
}

mt_u16  reg_aria_disp_get_video_scalar_outbuf_full_thr_sd_scalar_buf_full_thr(void)
{
    return (*(volatile reg_aria_disp_video_scalar_outbuf_full_thr_t *)REG_ARIA_DISP_VIDEO_SCALAR_OUTBUF_FULL_THR).bitc.sd_scalar_buf_full_thr;
}

void reg_aria_disp_set_video_scalar_outbuf_full_thr_hd_scalar_buf_full_thr(mt_u16 data)
{
    reg_aria_disp_video_scalar_outbuf_full_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SCALAR_OUTBUF_FULL_THR;
    d.bitc.hd_scalar_buf_full_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SCALAR_OUTBUF_FULL_THR, d.all);
}

mt_u16  reg_aria_disp_get_video_scalar_outbuf_full_thr_hd_scalar_buf_full_thr(void)
{
    return (*(volatile reg_aria_disp_video_scalar_outbuf_full_thr_t *)REG_ARIA_DISP_VIDEO_SCALAR_OUTBUF_FULL_THR).bitc.hd_scalar_buf_full_thr;
}


/*!
  register ARIA_DISP_axi_cmd_req_fifo_thr (read/write)
  */
void reg_aria_disp_set_axi_cmd_req_fifo_thr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_AXI_CMD_REQ_FIFO_THR, data);
}

mt_u32  reg_aria_disp_get_axi_cmd_req_fifo_thr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_AXI_CMD_REQ_FIFO_THR);
}

void reg_aria_disp_set_axi_cmd_req_fifo_thr_axi_req_sfifo_thr(mt_u8 data)
{
    reg_aria_disp_axi_cmd_req_fifo_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_AXI_CMD_REQ_FIFO_THR;
    d.bitc.axi_req_sfifo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_AXI_CMD_REQ_FIFO_THR, d.all);
}

mt_u8   reg_aria_disp_get_axi_cmd_req_fifo_thr_axi_req_sfifo_thr(void)
{
    return (*(volatile reg_aria_disp_axi_cmd_req_fifo_thr_t *)REG_ARIA_DISP_AXI_CMD_REQ_FIFO_THR).bitc.axi_req_sfifo_thr;
}

void reg_aria_disp_set_axi_cmd_req_fifo_thr_axi_cmd_sfifo_thr(mt_u8 data)
{
    reg_aria_disp_axi_cmd_req_fifo_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_AXI_CMD_REQ_FIFO_THR;
    d.bitc.axi_cmd_sfifo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_AXI_CMD_REQ_FIFO_THR, d.all);
}

mt_u8   reg_aria_disp_get_axi_cmd_req_fifo_thr_axi_cmd_sfifo_thr(void)
{
    return (*(volatile reg_aria_disp_axi_cmd_req_fifo_thr_t *)REG_ARIA_DISP_AXI_CMD_REQ_FIFO_THR).bitc.axi_cmd_sfifo_thr;
}

void reg_aria_disp_set_axi_cmd_req_fifo_thr_lout_afifo_thr(mt_u8 data)
{
    reg_aria_disp_axi_cmd_req_fifo_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_AXI_CMD_REQ_FIFO_THR;
    d.bitc.lout_afifo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_AXI_CMD_REQ_FIFO_THR, d.all);
}

mt_u8   reg_aria_disp_get_axi_cmd_req_fifo_thr_lout_afifo_thr(void)
{
    return (*(volatile reg_aria_disp_axi_cmd_req_fifo_thr_t *)REG_ARIA_DISP_AXI_CMD_REQ_FIFO_THR).bitc.lout_afifo_thr;
}


/*!
  register ARIA_DISP_access_fifo_lo_hi_thr (read/write)
  */
void reg_aria_disp_set_access_fifo_lo_hi_thr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ACCESS_FIFO_LO_HI_THR, data);
}

mt_u32  reg_aria_disp_get_access_fifo_lo_hi_thr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_ACCESS_FIFO_LO_HI_THR);
}

void reg_aria_disp_set_access_fifo_lo_hi_thr_acc_fifo_lo_thr(mt_u8 data)
{
    reg_aria_disp_access_fifo_lo_hi_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ACCESS_FIFO_LO_HI_THR;
    d.bitc.acc_fifo_lo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ACCESS_FIFO_LO_HI_THR, d.all);
}

mt_u8   reg_aria_disp_get_access_fifo_lo_hi_thr_acc_fifo_lo_thr(void)
{
    return (*(volatile reg_aria_disp_access_fifo_lo_hi_thr_t *)REG_ARIA_DISP_ACCESS_FIFO_LO_HI_THR).bitc.acc_fifo_lo_thr;
}

void reg_aria_disp_set_access_fifo_lo_hi_thr_acc_fifo_hi_thr(mt_u8 data)
{
    reg_aria_disp_access_fifo_lo_hi_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ACCESS_FIFO_LO_HI_THR;
    d.bitc.acc_fifo_hi_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ACCESS_FIFO_LO_HI_THR, d.all);
}

mt_u8   reg_aria_disp_get_access_fifo_lo_hi_thr_acc_fifo_hi_thr(void)
{
    return (*(volatile reg_aria_disp_access_fifo_lo_hi_thr_t *)REG_ARIA_DISP_ACCESS_FIFO_LO_HI_THR).bitc.acc_fifo_hi_thr;
}


/*!
  register ARIA_DISP_access_fifo_req_thr (read/write)
  */
void reg_aria_disp_set_access_fifo_req_thr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR, data);
}

mt_u32  reg_aria_disp_get_access_fifo_req_thr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR);
}

void reg_aria_disp_set_access_fifo_req_thr_fifo_thr_2(mt_u8 data)
{
    reg_aria_disp_access_fifo_req_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR;
    d.bitc.fifo_thr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR, d.all);
}

mt_u8   reg_aria_disp_get_access_fifo_req_thr_fifo_thr_2(void)
{
    return (*(volatile reg_aria_disp_access_fifo_req_thr_t *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR).bitc.fifo_thr_2;
}

void reg_aria_disp_set_access_fifo_req_thr_fifo_thr_1(mt_u8 data)
{
    reg_aria_disp_access_fifo_req_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR;
    d.bitc.fifo_thr_1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR, d.all);
}

mt_u8   reg_aria_disp_get_access_fifo_req_thr_fifo_thr_1(void)
{
    return (*(volatile reg_aria_disp_access_fifo_req_thr_t *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR).bitc.fifo_thr_1;
}

void reg_aria_disp_set_access_fifo_req_thr_fifo_thr_0(mt_u8 data)
{
    reg_aria_disp_access_fifo_req_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR;
    d.bitc.fifo_thr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR, d.all);
}

mt_u8   reg_aria_disp_get_access_fifo_req_thr_fifo_thr_0(void)
{
    return (*(volatile reg_aria_disp_access_fifo_req_thr_t *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR).bitc.fifo_thr_0;
}

void reg_aria_disp_set_access_fifo_req_thr_asyncfifo_low_thr(mt_u8 data)
{
    reg_aria_disp_access_fifo_req_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR;
    d.bitc.asyncfifo_low_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR, d.all);
}

mt_u8   reg_aria_disp_get_access_fifo_req_thr_asyncfifo_low_thr(void)
{
    return (*(volatile reg_aria_disp_access_fifo_req_thr_t *)REG_ARIA_DISP_ACCESS_FIFO_REQ_THR).bitc.asyncfifo_low_thr;
}


/*!
  register ARIA_DISP_asym_fifo_thr (read/write)
  */
void reg_aria_disp_set_asym_fifo_thr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ASYM_FIFO_THR, data);
}

mt_u32  reg_aria_disp_get_asym_fifo_thr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_ASYM_FIFO_THR);
}

void reg_aria_disp_set_asym_fifo_thr_disc2hdtv_async_fifo_full_thr(mt_u8 data)
{
    reg_aria_disp_asym_fifo_thr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ASYM_FIFO_THR;
    d.bitc.disc2hdtv_async_fifo_full_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ASYM_FIFO_THR, d.all);
}

mt_u8   reg_aria_disp_get_asym_fifo_thr_disc2hdtv_async_fifo_full_thr(void)
{
    return (*(volatile reg_aria_disp_asym_fifo_thr_t *)REG_ARIA_DISP_ASYM_FIFO_THR).bitc.disc2hdtv_async_fifo_full_thr;
}


/*!
  register ARIA_DISP_video_hd_line_cnt (read/write)
  */
void reg_aria_disp_set_video_hd_line_cnt(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_LINE_CNT, data);
}

mt_u32  reg_aria_disp_get_video_hd_line_cnt(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_LINE_CNT);
}

void reg_aria_disp_set_video_hd_line_cnt_hd_line_cnt_bot(mt_u16 data)
{
    reg_aria_disp_video_hd_line_cnt_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_LINE_CNT;
    d.bitc.hd_line_cnt_bot = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_LINE_CNT, d.all);
}

mt_u16  reg_aria_disp_get_video_hd_line_cnt_hd_line_cnt_bot(void)
{
    return (*(volatile reg_aria_disp_video_hd_line_cnt_t *)REG_ARIA_DISP_VIDEO_HD_LINE_CNT).bitc.hd_line_cnt_bot;
}

void reg_aria_disp_set_video_hd_line_cnt_hd_line_cnt_top(mt_u16 data)
{
    reg_aria_disp_video_hd_line_cnt_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_HD_LINE_CNT;
    d.bitc.hd_line_cnt_top = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_HD_LINE_CNT, d.all);
}

mt_u16  reg_aria_disp_get_video_hd_line_cnt_hd_line_cnt_top(void)
{
    return (*(volatile reg_aria_disp_video_hd_line_cnt_t *)REG_ARIA_DISP_VIDEO_HD_LINE_CNT).bitc.hd_line_cnt_top;
}


/*!
  register ARIA_DISP_video_sd_line_cnt (read/write)
  */
void reg_aria_disp_set_video_sd_line_cnt(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_LINE_CNT, data);
}

mt_u32  reg_aria_disp_get_video_sd_line_cnt(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_LINE_CNT);
}

void reg_aria_disp_set_video_sd_line_cnt_sd_line_cnt_bot(mt_u16 data)
{
    reg_aria_disp_video_sd_line_cnt_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_LINE_CNT;
    d.bitc.sd_line_cnt_bot = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_LINE_CNT, d.all);
}

mt_u16  reg_aria_disp_get_video_sd_line_cnt_sd_line_cnt_bot(void)
{
    return (*(volatile reg_aria_disp_video_sd_line_cnt_t *)REG_ARIA_DISP_VIDEO_SD_LINE_CNT).bitc.sd_line_cnt_bot;
}

void reg_aria_disp_set_video_sd_line_cnt_sd_line_cnt_top(mt_u16 data)
{
    reg_aria_disp_video_sd_line_cnt_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_SD_LINE_CNT;
    d.bitc.sd_line_cnt_top = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_SD_LINE_CNT, d.all);
}

mt_u16  reg_aria_disp_get_video_sd_line_cnt_sd_line_cnt_top(void)
{
    return (*(volatile reg_aria_disp_video_sd_line_cnt_t *)REG_ARIA_DISP_VIDEO_SD_LINE_CNT).bitc.sd_line_cnt_top;
}


/*!
  register ARIA_DISP_video_line_proc_status (read/write)
  */
void reg_aria_disp_set_video_line_proc_status(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_LINE_PROC_STATUS, data);
}

mt_u32  reg_aria_disp_get_video_line_proc_status(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_LINE_PROC_STATUS);
}

void reg_aria_disp_set_video_line_proc_status_proc_line_num(mt_u16 data)
{
    reg_aria_disp_video_line_proc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_LINE_PROC_STATUS;
    d.bitc.proc_line_num = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_LINE_PROC_STATUS, d.all);
}

mt_u16  reg_aria_disp_get_video_line_proc_status_proc_line_num(void)
{
    return (*(volatile reg_aria_disp_video_line_proc_status_t *)REG_ARIA_DISP_VIDEO_LINE_PROC_STATUS).bitc.proc_line_num;
}

void reg_aria_disp_set_video_line_proc_status_not_proc_over(mt_u8 data)
{
    reg_aria_disp_video_line_proc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_LINE_PROC_STATUS;
    d.bitc.not_proc_over = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_LINE_PROC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_video_line_proc_status_not_proc_over(void)
{
    return (*(volatile reg_aria_disp_video_line_proc_status_t *)REG_ARIA_DISP_VIDEO_LINE_PROC_STATUS).bitc.not_proc_over;
}


/*!
  register ARIA_DISP_alising_prob_reg1 (read/write)
  */
void reg_aria_disp_set_alising_prob_reg1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG1, data);
}

mt_u32  reg_aria_disp_get_alising_prob_reg1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG1);
}

void reg_aria_disp_set_alising_prob_reg1_alpha_2nd_method_sel(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG1;
    d.bitc.alpha_2nd_method_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG1, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg1_alpha_2nd_method_sel(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg1_t *)REG_ARIA_DISP_ALISING_PROB_REG1).bitc.alpha_2nd_method_sel;
}

void reg_aria_disp_set_alising_prob_reg1_alpha_2nd_diff_ratio_sel(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG1;
    d.bitc.alpha_2nd_diff_ratio_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG1, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg1_alpha_2nd_diff_ratio_sel(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg1_t *)REG_ARIA_DISP_ALISING_PROB_REG1).bitc.alpha_2nd_diff_ratio_sel;
}

void reg_aria_disp_set_alising_prob_reg1_alpha_2nd_diff_shift_sel(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG1;
    d.bitc.alpha_2nd_diff_shift_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG1, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg1_alpha_2nd_diff_shift_sel(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg1_t *)REG_ARIA_DISP_ALISING_PROB_REG1).bitc.alpha_2nd_diff_shift_sel;
}

void reg_aria_disp_set_alising_prob_reg1_alpha_2nd_diff(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG1;
    d.bitc.alpha_2nd_diff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG1, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg1_alpha_2nd_diff(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg1_t *)REG_ARIA_DISP_ALISING_PROB_REG1).bitc.alpha_2nd_diff;
}


/*!
  register ARIA_DISP_alising_prob_reg2 (read/write)
  */
void reg_aria_disp_set_alising_prob_reg2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG2, data);
}

mt_u32  reg_aria_disp_get_alising_prob_reg2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG2);
}

void reg_aria_disp_set_alising_prob_reg2_alpha_enlarge(mt_u16 data)
{
    reg_aria_disp_alising_prob_reg2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG2;
    d.bitc.alpha_enlarge = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG2, d.all);
}

mt_u16  reg_aria_disp_get_alising_prob_reg2_alpha_enlarge(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg2_t *)REG_ARIA_DISP_ALISING_PROB_REG2).bitc.alpha_enlarge;
}

void reg_aria_disp_set_alising_prob_reg2_alpha_vdv_sel(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG2;
    d.bitc.alpha_vdv_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG2, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg2_alpha_vdv_sel(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg2_t *)REG_ARIA_DISP_ALISING_PROB_REG2).bitc.alpha_vdv_sel;
}

void reg_aria_disp_set_alising_prob_reg2_alpha_vdv(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG2;
    d.bitc.alpha_vdv = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG2, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg2_alpha_vdv(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg2_t *)REG_ARIA_DISP_ALISING_PROB_REG2).bitc.alpha_vdv;
}

void reg_aria_disp_set_alising_prob_reg2_alpha_angle(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG2;
    d.bitc.alpha_angle = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG2, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg2_alpha_angle(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg2_t *)REG_ARIA_DISP_ALISING_PROB_REG2).bitc.alpha_angle;
}


/*!
  register ARIA_DISP_alising_prob_reg3 (read/write)
  */
void reg_aria_disp_set_alising_prob_reg3(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG3, data);
}

mt_u32  reg_aria_disp_get_alising_prob_reg3(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG3);
}

void reg_aria_disp_set_alising_prob_reg3_base_blending_factor(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG3;
    d.bitc.base_blending_factor = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG3, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg3_base_blending_factor(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg3_t *)REG_ARIA_DISP_ALISING_PROB_REG3).bitc.base_blending_factor;
}

void reg_aria_disp_set_alising_prob_reg3_default_prob(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG3;
    d.bitc.default_prob = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG3, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg3_default_prob(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg3_t *)REG_ARIA_DISP_ALISING_PROB_REG3).bitc.default_prob;
}

void reg_aria_disp_set_alising_prob_reg3_op(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG3;
    d.bitc.op = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG3, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg3_op(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg3_t *)REG_ARIA_DISP_ALISING_PROB_REG3).bitc.op;
}

void reg_aria_disp_set_alising_prob_reg3_slope_diff1_sel(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG3;
    d.bitc.slope_diff1_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG3, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg3_slope_diff1_sel(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg3_t *)REG_ARIA_DISP_ALISING_PROB_REG3).bitc.slope_diff1_sel;
}


/*!
  register ARIA_DISP_alising_prob_reg4 (read/write)
  */
void reg_aria_disp_set_alising_prob_reg4(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG4, data);
}

mt_u32  reg_aria_disp_get_alising_prob_reg4(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG4);
}

void reg_aria_disp_set_alising_prob_reg4_pict_enhance_pix_sel(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG4;
    d.bitc.pict_enhance_pix_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG4, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg4_pict_enhance_pix_sel(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg4_t *)REG_ARIA_DISP_ALISING_PROB_REG4).bitc.pict_enhance_pix_sel;
}

void reg_aria_disp_set_alising_prob_reg4_adaptive_alpha_sel(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG4;
    d.bitc.adaptive_alpha_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG4, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg4_adaptive_alpha_sel(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg4_t *)REG_ARIA_DISP_ALISING_PROB_REG4).bitc.adaptive_alpha_sel;
}

void reg_aria_disp_set_alising_prob_reg4_diff_2nd_sel(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG4;
    d.bitc.diff_2nd_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG4, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg4_diff_2nd_sel(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg4_t *)REG_ARIA_DISP_ALISING_PROB_REG4).bitc.diff_2nd_sel;
}

void reg_aria_disp_set_alising_prob_reg4_interp_factor(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG4;
    d.bitc.interp_factor = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG4, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg4_interp_factor(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg4_t *)REG_ARIA_DISP_ALISING_PROB_REG4).bitc.interp_factor;
}

void reg_aria_disp_set_alising_prob_reg4_prob_coef3(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG4;
    d.bitc.prob_coef3 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG4, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg4_prob_coef3(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg4_t *)REG_ARIA_DISP_ALISING_PROB_REG4).bitc.prob_coef3;
}

void reg_aria_disp_set_alising_prob_reg4_prob_coef2(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG4;
    d.bitc.prob_coef2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG4, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg4_prob_coef2(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg4_t *)REG_ARIA_DISP_ALISING_PROB_REG4).bitc.prob_coef2;
}

void reg_aria_disp_set_alising_prob_reg4_prob_coef1(mt_u8 data)
{
    reg_aria_disp_alising_prob_reg4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_ALISING_PROB_REG4;
    d.bitc.prob_coef1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_ALISING_PROB_REG4, d.all);
}

mt_u8   reg_aria_disp_get_alising_prob_reg4_prob_coef1(void)
{
    return (*(volatile reg_aria_disp_alising_prob_reg4_t *)REG_ARIA_DISP_ALISING_PROB_REG4).bitc.prob_coef1;
}


/*!
  register ARIA_DISP_di_ctrl (read/write)
  */
void reg_aria_disp_set_di_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_CTRL, data);
}

mt_u32  reg_aria_disp_get_di_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_CTRL);
}

void reg_aria_disp_set_di_ctrl_di_en(mt_u8 data)
{
    reg_aria_disp_di_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_CTRL;
    d.bitc.di_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_di_ctrl_di_en(void)
{
    return (*(volatile reg_aria_disp_di_ctrl_t *)REG_ARIA_DISP_DI_CTRL).bitc.di_en;
}

void reg_aria_disp_set_di_ctrl_video_source_mode(mt_u8 data)
{
    reg_aria_disp_di_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_CTRL;
    d.bitc.video_source_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_di_ctrl_video_source_mode(void)
{
    return (*(volatile reg_aria_disp_di_ctrl_t *)REG_ARIA_DISP_DI_CTRL).bitc.video_source_mode;
}

void reg_aria_disp_set_di_ctrl_pdd_en(mt_u8 data)
{
    reg_aria_disp_di_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_CTRL;
    d.bitc.pdd_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_di_ctrl_pdd_en(void)
{
    return (*(volatile reg_aria_disp_di_ctrl_t *)REG_ARIA_DISP_DI_CTRL).bitc.pdd_en;
}

void reg_aria_disp_set_di_ctrl_is_movie_type(mt_u8 data)
{
    reg_aria_disp_di_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_CTRL;
    d.bitc.is_movie_type = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_di_ctrl_is_movie_type(void)
{
    return (*(volatile reg_aria_disp_di_ctrl_t *)REG_ARIA_DISP_DI_CTRL).bitc.is_movie_type;
}

void reg_aria_disp_set_di_ctrl_thr_reg(mt_u8 data)
{
    reg_aria_disp_di_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_CTRL;
    d.bitc.thr_reg = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_di_ctrl_thr_reg(void)
{
    return (*(volatile reg_aria_disp_di_ctrl_t *)REG_ARIA_DISP_DI_CTRL).bitc.thr_reg;
}

void reg_aria_disp_set_di_ctrl_thr_sel(mt_u8 data)
{
    reg_aria_disp_di_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_CTRL;
    d.bitc.thr_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_di_ctrl_thr_sel(void)
{
    return (*(volatile reg_aria_disp_di_ctrl_t *)REG_ARIA_DISP_DI_CTRL).bitc.thr_sel;
}


/*!
  register ARIA_DISP_di_pause_en (read/write)
  */
void reg_aria_disp_set_di_pause_en(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_PAUSE_EN, data);
}

mt_u32  reg_aria_disp_get_di_pause_en(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_PAUSE_EN);
}

void reg_aria_disp_set_di_pause_en_di_pause_en(mt_u8 data)
{
    reg_aria_disp_di_pause_en_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_PAUSE_EN;
    d.bitc.di_pause_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_PAUSE_EN, d.all);
}

mt_u8   reg_aria_disp_get_di_pause_en_di_pause_en(void)
{
    return (*(volatile reg_aria_disp_di_pause_en_t *)REG_ARIA_DISP_DI_PAUSE_EN).bitc.di_pause_en;
}

void reg_aria_disp_set_di_pause_en_di_pause_bot_field_flag(mt_u8 data)
{
    reg_aria_disp_di_pause_en_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_PAUSE_EN;
    d.bitc.di_pause_bot_field_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_PAUSE_EN, d.all);
}

mt_u8   reg_aria_disp_get_di_pause_en_di_pause_bot_field_flag(void)
{
    return (*(volatile reg_aria_disp_di_pause_en_t *)REG_ARIA_DISP_DI_PAUSE_EN).bitc.di_pause_bot_field_flag;
}

void reg_aria_disp_set_di_pause_en_di_pause_top_field_flag(mt_u8 data)
{
    reg_aria_disp_di_pause_en_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_PAUSE_EN;
    d.bitc.di_pause_top_field_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_PAUSE_EN, d.all);
}

mt_u8   reg_aria_disp_get_di_pause_en_di_pause_top_field_flag(void)
{
    return (*(volatile reg_aria_disp_di_pause_en_t *)REG_ARIA_DISP_DI_PAUSE_EN).bitc.di_pause_top_field_flag;
}


/*!
  register ARIA_DISP_di_oper_mode (read/write)
  */
void reg_aria_disp_set_di_oper_mode(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_OPER_MODE, data);
}

mt_u32  reg_aria_disp_get_di_oper_mode(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_OPER_MODE);
}

void reg_aria_disp_set_di_oper_mode_mix_output_mode(mt_u8 data)
{
    reg_aria_disp_di_oper_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_OPER_MODE;
    d.bitc.mix_output_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_OPER_MODE, d.all);
}

mt_u8   reg_aria_disp_get_di_oper_mode_mix_output_mode(void)
{
    return (*(volatile reg_aria_disp_di_oper_mode_t *)REG_ARIA_DISP_DI_OPER_MODE).bitc.mix_output_mode;
}

void reg_aria_disp_set_di_oper_mode_motion_output_mode(mt_u8 data)
{
    reg_aria_disp_di_oper_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_OPER_MODE;
    d.bitc.motion_output_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_OPER_MODE, d.all);
}

mt_u8   reg_aria_disp_get_di_oper_mode_motion_output_mode(void)
{
    return (*(volatile reg_aria_disp_di_oper_mode_t *)REG_ARIA_DISP_DI_OPER_MODE).bitc.motion_output_mode;
}

void reg_aria_disp_set_di_oper_mode_para_after_filter_en(mt_u8 data)
{
    reg_aria_disp_di_oper_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_OPER_MODE;
    d.bitc.para_after_filter_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_OPER_MODE, d.all);
}

mt_u8   reg_aria_disp_get_di_oper_mode_para_after_filter_en(void)
{
    return (*(volatile reg_aria_disp_di_oper_mode_t *)REG_ARIA_DISP_DI_OPER_MODE).bitc.para_after_filter_en;
}

void reg_aria_disp_set_di_oper_mode_spatial_ip_mode(mt_u8 data)
{
    reg_aria_disp_di_oper_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_OPER_MODE;
    d.bitc.spatial_ip_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_OPER_MODE, d.all);
}

mt_u8   reg_aria_disp_get_di_oper_mode_spatial_ip_mode(void)
{
    return (*(volatile reg_aria_disp_di_oper_mode_t *)REG_ARIA_DISP_DI_OPER_MODE).bitc.spatial_ip_mode;
}

void reg_aria_disp_set_di_oper_mode_hori_ip_en(mt_u8 data)
{
    reg_aria_disp_di_oper_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_OPER_MODE;
    d.bitc.hori_ip_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_OPER_MODE, d.all);
}

mt_u8   reg_aria_disp_get_di_oper_mode_hori_ip_en(void)
{
    return (*(volatile reg_aria_disp_di_oper_mode_t *)REG_ARIA_DISP_DI_OPER_MODE).bitc.hori_ip_en;
}

void reg_aria_disp_set_di_oper_mode_temporal_ip_mode_bot(mt_u8 data)
{
    reg_aria_disp_di_oper_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_OPER_MODE;
    d.bitc.temporal_ip_mode_bot = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_OPER_MODE, d.all);
}

mt_u8   reg_aria_disp_get_di_oper_mode_temporal_ip_mode_bot(void)
{
    return (*(volatile reg_aria_disp_di_oper_mode_t *)REG_ARIA_DISP_DI_OPER_MODE).bitc.temporal_ip_mode_bot;
}

void reg_aria_disp_set_di_oper_mode_temporal_ip_mode_top(mt_u8 data)
{
    reg_aria_disp_di_oper_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_OPER_MODE;
    d.bitc.temporal_ip_mode_top = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_OPER_MODE, d.all);
}

mt_u8   reg_aria_disp_get_di_oper_mode_temporal_ip_mode_top(void)
{
    return (*(volatile reg_aria_disp_di_oper_mode_t *)REG_ARIA_DISP_DI_OPER_MODE).bitc.temporal_ip_mode_top;
}

void reg_aria_disp_set_di_oper_mode_lbam_en(mt_u8 data)
{
    reg_aria_disp_di_oper_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_OPER_MODE;
    d.bitc.lbam_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_OPER_MODE, d.all);
}

mt_u8   reg_aria_disp_get_di_oper_mode_lbam_en(void)
{
    return (*(volatile reg_aria_disp_di_oper_mode_t *)REG_ARIA_DISP_DI_OPER_MODE).bitc.lbam_en;
}

void reg_aria_disp_set_di_oper_mode_motion_est_mode(mt_u8 data)
{
    reg_aria_disp_di_oper_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_OPER_MODE;
    d.bitc.motion_est_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_OPER_MODE, d.all);
}

mt_u8   reg_aria_disp_get_di_oper_mode_motion_est_mode(void)
{
    return (*(volatile reg_aria_disp_di_oper_mode_t *)REG_ARIA_DISP_DI_OPER_MODE).bitc.motion_est_mode;
}

void reg_aria_disp_set_di_oper_mode_motion_rd_en(mt_u8 data)
{
    reg_aria_disp_di_oper_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_OPER_MODE;
    d.bitc.motion_rd_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_OPER_MODE, d.all);
}

mt_u8   reg_aria_disp_get_di_oper_mode_motion_rd_en(void)
{
    return (*(volatile reg_aria_disp_di_oper_mode_t *)REG_ARIA_DISP_DI_OPER_MODE).bitc.motion_rd_en;
}

void reg_aria_disp_set_di_oper_mode_motion_wr_en(mt_u8 data)
{
    reg_aria_disp_di_oper_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_OPER_MODE;
    d.bitc.motion_wr_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_OPER_MODE, d.all);
}

mt_u8   reg_aria_disp_get_di_oper_mode_motion_wr_en(void)
{
    return (*(volatile reg_aria_disp_di_oper_mode_t *)REG_ARIA_DISP_DI_OPER_MODE).bitc.motion_wr_en;
}


/*!
  register ARIA_DISP_di_p_or_n_pair (read/write)
  */
void reg_aria_disp_set_di_p_or_n_pair(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_P_OR_N_PAIR, data);
}

mt_u32  reg_aria_disp_get_di_p_or_n_pair(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_P_OR_N_PAIR);
}

void reg_aria_disp_set_di_p_or_n_pair_p_or_n_pair_bot(mt_u8 data)
{
    reg_aria_disp_di_p_or_n_pair_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_P_OR_N_PAIR;
    d.bitc.p_or_n_pair_bot = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_P_OR_N_PAIR, d.all);
}

mt_u8   reg_aria_disp_get_di_p_or_n_pair_p_or_n_pair_bot(void)
{
    return (*(volatile reg_aria_disp_di_p_or_n_pair_t *)REG_ARIA_DISP_DI_P_OR_N_PAIR).bitc.p_or_n_pair_bot;
}

void reg_aria_disp_set_di_p_or_n_pair_p_or_n_pair_top(mt_u8 data)
{
    reg_aria_disp_di_p_or_n_pair_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_P_OR_N_PAIR;
    d.bitc.p_or_n_pair_top = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_P_OR_N_PAIR, d.all);
}

mt_u8   reg_aria_disp_get_di_p_or_n_pair_p_or_n_pair_top(void)
{
    return (*(volatile reg_aria_disp_di_p_or_n_pair_t *)REG_ARIA_DISP_DI_P_OR_N_PAIR).bitc.p_or_n_pair_top;
}


/*!
  register ARIA_DISP_di_para (read/write)
  */
void reg_aria_disp_set_di_para(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_PARA, data);
}

mt_u32  reg_aria_disp_get_di_para(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_PARA);
}

void reg_aria_disp_set_di_para_g_alpha_k(mt_u8 data)
{
    reg_aria_disp_di_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_PARA;
    d.bitc.g_alpha_k = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_PARA, d.all);
}

mt_u8   reg_aria_disp_get_di_para_g_alpha_k(void)
{
    return (*(volatile reg_aria_disp_di_para_t *)REG_ARIA_DISP_DI_PARA).bitc.g_alpha_k;
}

void reg_aria_disp_set_di_para_g_alpha_0(mt_u8 data)
{
    reg_aria_disp_di_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_PARA;
    d.bitc.g_alpha_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_PARA, d.all);
}

mt_u8   reg_aria_disp_get_di_para_g_alpha_0(void)
{
    return (*(volatile reg_aria_disp_di_para_t *)REG_ARIA_DISP_DI_PARA).bitc.g_alpha_0;
}

void reg_aria_disp_set_di_para_p_tl(mt_u8 data)
{
    reg_aria_disp_di_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_PARA;
    d.bitc.p_tl = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_PARA, d.all);
}

mt_u8   reg_aria_disp_get_di_para_p_tl(void)
{
    return (*(volatile reg_aria_disp_di_para_t *)REG_ARIA_DISP_DI_PARA).bitc.p_tl;
}

void reg_aria_disp_set_di_para_pdd_noise_thr(mt_u8 data)
{
    reg_aria_disp_di_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_PARA;
    d.bitc.pdd_noise_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_PARA, d.all);
}

mt_u8   reg_aria_disp_get_di_para_pdd_noise_thr(void)
{
    return (*(volatile reg_aria_disp_di_para_t *)REG_ARIA_DISP_DI_PARA).bitc.pdd_noise_thr;
}


/*!
  register ARIA_DISP_di_chroma_para (read/write)
  */
void reg_aria_disp_set_di_chroma_para(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_CHROMA_PARA, data);
}

mt_u32  reg_aria_disp_get_di_chroma_para(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_CHROMA_PARA);
}

void reg_aria_disp_set_di_chroma_para_chroma_g_alpha_k(mt_u8 data)
{
    reg_aria_disp_di_chroma_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_CHROMA_PARA;
    d.bitc.chroma_g_alpha_k = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_CHROMA_PARA, d.all);
}

mt_u8   reg_aria_disp_get_di_chroma_para_chroma_g_alpha_k(void)
{
    return (*(volatile reg_aria_disp_di_chroma_para_t *)REG_ARIA_DISP_DI_CHROMA_PARA).bitc.chroma_g_alpha_k;
}

void reg_aria_disp_set_di_chroma_para_chroma_g_alpha_0(mt_u8 data)
{
    reg_aria_disp_di_chroma_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_CHROMA_PARA;
    d.bitc.chroma_g_alpha_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_CHROMA_PARA, d.all);
}

mt_u8   reg_aria_disp_get_di_chroma_para_chroma_g_alpha_0(void)
{
    return (*(volatile reg_aria_disp_di_chroma_para_t *)REG_ARIA_DISP_DI_CHROMA_PARA).bitc.chroma_g_alpha_0;
}


/*!
  register ARIA_DISP_di_alpha_para (read/write)
  */
void reg_aria_disp_set_di_alpha_para(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_ALPHA_PARA, data);
}

mt_u32  reg_aria_disp_get_di_alpha_para(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_ALPHA_PARA);
}

void reg_aria_disp_set_di_alpha_para_g_alpha_0_min(mt_u8 data)
{
    reg_aria_disp_di_alpha_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_ALPHA_PARA;
    d.bitc.g_alpha_0_min = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_ALPHA_PARA, d.all);
}

mt_u8   reg_aria_disp_get_di_alpha_para_g_alpha_0_min(void)
{
    return (*(volatile reg_aria_disp_di_alpha_para_t *)REG_ARIA_DISP_DI_ALPHA_PARA).bitc.g_alpha_0_min;
}

void reg_aria_disp_set_di_alpha_para_g_alpha_0_max(mt_u8 data)
{
    reg_aria_disp_di_alpha_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_ALPHA_PARA;
    d.bitc.g_alpha_0_max = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_ALPHA_PARA, d.all);
}

mt_u8   reg_aria_disp_get_di_alpha_para_g_alpha_0_max(void)
{
    return (*(volatile reg_aria_disp_di_alpha_para_t *)REG_ARIA_DISP_DI_ALPHA_PARA).bitc.g_alpha_0_max;
}

void reg_aria_disp_set_di_alpha_para_luma_diff_k(mt_u8 data)
{
    reg_aria_disp_di_alpha_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_ALPHA_PARA;
    d.bitc.luma_diff_k = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_ALPHA_PARA, d.all);
}

mt_u8   reg_aria_disp_get_di_alpha_para_luma_diff_k(void)
{
    return (*(volatile reg_aria_disp_di_alpha_para_t *)REG_ARIA_DISP_DI_ALPHA_PARA).bitc.luma_diff_k;
}

void reg_aria_disp_set_di_alpha_para_diff_sel(mt_u8 data)
{
    reg_aria_disp_di_alpha_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_ALPHA_PARA;
    d.bitc.diff_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_ALPHA_PARA, d.all);
}

mt_u8   reg_aria_disp_get_di_alpha_para_diff_sel(void)
{
    return (*(volatile reg_aria_disp_di_alpha_para_t *)REG_ARIA_DISP_DI_ALPHA_PARA).bitc.diff_sel;
}

void reg_aria_disp_set_di_alpha_para_new_alpha_en(mt_u8 data)
{
    reg_aria_disp_di_alpha_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_ALPHA_PARA;
    d.bitc.new_alpha_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_ALPHA_PARA, d.all);
}

mt_u8   reg_aria_disp_get_di_alpha_para_new_alpha_en(void)
{
    return (*(volatile reg_aria_disp_di_alpha_para_t *)REG_ARIA_DISP_DI_ALPHA_PARA).bitc.new_alpha_en;
}


/*!
  register ARIA_DISP_di_motion_ctrl_1 (read/write)
  */
void reg_aria_disp_set_di_motion_ctrl_1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_1, data);
}

mt_u32  reg_aria_disp_get_di_motion_ctrl_1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_1);
}

void reg_aria_disp_set_di_motion_ctrl_1_motion_propa_type(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_1;
    d.bitc.motion_propa_type = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_1_motion_propa_type(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_1_t *)REG_ARIA_DISP_DI_MOTION_CTRL_1).bitc.motion_propa_type;
}

void reg_aria_disp_set_di_motion_ctrl_1_motion_damping2(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_1;
    d.bitc.motion_damping2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_1_motion_damping2(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_1_t *)REG_ARIA_DISP_DI_MOTION_CTRL_1).bitc.motion_damping2;
}

void reg_aria_disp_set_di_motion_ctrl_1_motion_damping1(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_1;
    d.bitc.motion_damping1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_1_motion_damping1(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_1_t *)REG_ARIA_DISP_DI_MOTION_CTRL_1).bitc.motion_damping1;
}

void reg_aria_disp_set_di_motion_ctrl_1_medrsp_thr(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_1;
    d.bitc.medrsp_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_1_medrsp_thr(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_1_t *)REG_ARIA_DISP_DI_MOTION_CTRL_1).bitc.medrsp_thr;
}

void reg_aria_disp_set_di_motion_ctrl_1_difdamping(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_1;
    d.bitc.difdamping = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_1, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_1_difdamping(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_1_t *)REG_ARIA_DISP_DI_MOTION_CTRL_1).bitc.difdamping;
}


/*!
  register ARIA_DISP_di_motion_ctrl_2 (read/write)
  */
void reg_aria_disp_set_di_motion_ctrl_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_2, data);
}

mt_u32  reg_aria_disp_get_di_motion_ctrl_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_2);
}

void reg_aria_disp_set_di_motion_ctrl_2_half_motion_en(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_2;
    d.bitc.half_motion_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_2, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_2_half_motion_en(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_2_t *)REG_ARIA_DISP_DI_MOTION_CTRL_2).bitc.half_motion_en;
}

void reg_aria_disp_set_di_motion_ctrl_2_motion_data_mode(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_2;
    d.bitc.motion_data_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_2, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_2_motion_data_mode(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_2_t *)REG_ARIA_DISP_DI_MOTION_CTRL_2).bitc.motion_data_mode;
}

void reg_aria_disp_set_di_motion_ctrl_2_motion_estmethod(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_2;
    d.bitc.motion_estmethod = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_2, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_2_motion_estmethod(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_2_t *)REG_ARIA_DISP_DI_MOTION_CTRL_2).bitc.motion_estmethod;
}

void reg_aria_disp_set_di_motion_ctrl_2_l0l2_motion_mode(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_2;
    d.bitc.l0l2_motion_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_2, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_2_l0l2_motion_mode(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_2_t *)REG_ARIA_DISP_DI_MOTION_CTRL_2).bitc.l0l2_motion_mode;
}

void reg_aria_disp_set_di_motion_ctrl_2_ip_smallmotion(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_2;
    d.bitc.ip_smallmotion = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_2, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_2_ip_smallmotion(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_2_t *)REG_ARIA_DISP_DI_MOTION_CTRL_2).bitc.ip_smallmotion;
}

void reg_aria_disp_set_di_motion_ctrl_2_ip_average(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_2;
    d.bitc.ip_average = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_2, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_2_ip_average(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_2_t *)REG_ARIA_DISP_DI_MOTION_CTRL_2).bitc.ip_average;
}

void reg_aria_disp_set_di_motion_ctrl_2_ip_l0orl2(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_2;
    d.bitc.ip_l0orl2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_2, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_2_ip_l0orl2(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_2_t *)REG_ARIA_DISP_DI_MOTION_CTRL_2).bitc.ip_l0orl2;
}


/*!
  register ARIA_DISP_di_motion_ctrl_3 (read/write)
  */
void reg_aria_disp_set_di_motion_ctrl_3(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_3, data);
}

mt_u32  reg_aria_disp_get_di_motion_ctrl_3(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_3);
}

void reg_aria_disp_set_di_motion_ctrl_3_new_algo_en(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_3;
    d.bitc.new_algo_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_3, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_3_new_algo_en(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_3_t *)REG_ARIA_DISP_DI_MOTION_CTRL_3).bitc.new_algo_en;
}

void reg_aria_disp_set_di_motion_ctrl_3_small_motion_magnify(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_3;
    d.bitc.small_motion_magnify = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_3, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_3_small_motion_magnify(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_3_t *)REG_ARIA_DISP_DI_MOTION_CTRL_3).bitc.small_motion_magnify;
}

void reg_aria_disp_set_di_motion_ctrl_3_small_motion_thr2(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_3;
    d.bitc.small_motion_thr2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_3, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_3_small_motion_thr2(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_3_t *)REG_ARIA_DISP_DI_MOTION_CTRL_3).bitc.small_motion_thr2;
}

void reg_aria_disp_set_di_motion_ctrl_3_small_motion_thr1(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_3;
    d.bitc.small_motion_thr1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_3, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_3_small_motion_thr1(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_3_t *)REG_ARIA_DISP_DI_MOTION_CTRL_3).bitc.small_motion_thr1;
}

void reg_aria_disp_set_di_motion_ctrl_3_medrsp7dir_thr(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_3;
    d.bitc.medrsp7dir_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_3, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_3_medrsp7dir_thr(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_3_t *)REG_ARIA_DISP_DI_MOTION_CTRL_3).bitc.medrsp7dir_thr;
}


/*!
  register ARIA_DISP_di_motion_ctrl_4 (read/write)
  */
void reg_aria_disp_set_di_motion_ctrl_4(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_4, data);
}

mt_u32  reg_aria_disp_get_di_motion_ctrl_4(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_4);
}

void reg_aria_disp_set_di_motion_ctrl_4_uv_motion_gain(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_4;
    d.bitc.uv_motion_gain = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_4, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_4_uv_motion_gain(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_4_t *)REG_ARIA_DISP_DI_MOTION_CTRL_4).bitc.uv_motion_gain;
}

void reg_aria_disp_set_di_motion_ctrl_4_ip_l0andl2(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_4;
    d.bitc.ip_l0andl2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_4, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_4_ip_l0andl2(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_4_t *)REG_ARIA_DISP_DI_MOTION_CTRL_4).bitc.ip_l0andl2;
}

void reg_aria_disp_set_di_motion_ctrl_4_ip_l1_difthr(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_4;
    d.bitc.ip_l1_difthr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_4, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_4_ip_l1_difthr(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_4_t *)REG_ARIA_DISP_DI_MOTION_CTRL_4).bitc.ip_l1_difthr;
}

void reg_aria_disp_set_di_motion_ctrl_4_preserve_all_motion(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_4;
    d.bitc.preserve_all_motion = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_4, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_4_preserve_all_motion(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_4_t *)REG_ARIA_DISP_DI_MOTION_CTRL_4).bitc.preserve_all_motion;
}

void reg_aria_disp_set_di_motion_ctrl_4_preserve_small_motion(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_4;
    d.bitc.preserve_small_motion = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_4, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_4_preserve_small_motion(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_4_t *)REG_ARIA_DISP_DI_MOTION_CTRL_4).bitc.preserve_small_motion;
}

void reg_aria_disp_set_di_motion_ctrl_4_motion_magnify(mt_u8 data)
{
    reg_aria_disp_di_motion_ctrl_4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_MOTION_CTRL_4;
    d.bitc.motion_magnify = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_MOTION_CTRL_4, d.all);
}

mt_u8   reg_aria_disp_get_di_motion_ctrl_4_motion_magnify(void)
{
    return (*(volatile reg_aria_disp_di_motion_ctrl_4_t *)REG_ARIA_DISP_DI_MOTION_CTRL_4).bitc.motion_magnify;
}


/*!
  register ARIA_DISP_di_acc_odd_result (read/write)
  */
void reg_aria_disp_set_di_acc_odd_result(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_ACC_ODD_RESULT, data);
}

mt_u32  reg_aria_disp_get_di_acc_odd_result(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_ACC_ODD_RESULT);
}

void reg_aria_disp_set_di_acc_odd_result_di_acc_odd_result(mt_u32 data)
{
    reg_aria_disp_di_acc_odd_result_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_ACC_ODD_RESULT;
    d.bitc.di_acc_odd_result = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_ACC_ODD_RESULT, d.all);
}

mt_u32  reg_aria_disp_get_di_acc_odd_result_di_acc_odd_result(void)
{
    return (*(volatile reg_aria_disp_di_acc_odd_result_t *)REG_ARIA_DISP_DI_ACC_ODD_RESULT).bitc.di_acc_odd_result;
}


/*!
  register ARIA_DISP_di_acc_even_result (read/write)
  */
void reg_aria_disp_set_di_acc_even_result(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_ACC_EVEN_RESULT, data);
}

mt_u32  reg_aria_disp_get_di_acc_even_result(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_ACC_EVEN_RESULT);
}

void reg_aria_disp_set_di_acc_even_result_di_acc_even_result(mt_u32 data)
{
    reg_aria_disp_di_acc_even_result_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_ACC_EVEN_RESULT;
    d.bitc.di_acc_even_result = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_ACC_EVEN_RESULT, d.all);
}

mt_u32  reg_aria_disp_get_di_acc_even_result_di_acc_even_result(void)
{
    return (*(volatile reg_aria_disp_di_acc_even_result_t *)REG_ARIA_DISP_DI_ACC_EVEN_RESULT).bitc.di_acc_even_result;
}


/*!
  register ARIA_DISP_di_fields_flag (read/write)
  */
void reg_aria_disp_set_di_fields_flag(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_FIELDS_FLAG, data);
}

mt_u32  reg_aria_disp_get_di_fields_flag(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_FIELDS_FLAG);
}

void reg_aria_disp_set_di_fields_flag_nxt_field_flag_2(mt_u8 data)
{
    reg_aria_disp_di_fields_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_FIELDS_FLAG;
    d.bitc.nxt_field_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_FIELDS_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_fields_flag_nxt_field_flag_2(void)
{
    return (*(volatile reg_aria_disp_di_fields_flag_t *)REG_ARIA_DISP_DI_FIELDS_FLAG).bitc.nxt_field_flag_2;
}

void reg_aria_disp_set_di_fields_flag_cur_field_flag_2(mt_u8 data)
{
    reg_aria_disp_di_fields_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_FIELDS_FLAG;
    d.bitc.cur_field_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_FIELDS_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_fields_flag_cur_field_flag_2(void)
{
    return (*(volatile reg_aria_disp_di_fields_flag_t *)REG_ARIA_DISP_DI_FIELDS_FLAG).bitc.cur_field_flag_2;
}

void reg_aria_disp_set_di_fields_flag_pre_field_flag_2(mt_u8 data)
{
    reg_aria_disp_di_fields_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_FIELDS_FLAG;
    d.bitc.pre_field_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_FIELDS_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_fields_flag_pre_field_flag_2(void)
{
    return (*(volatile reg_aria_disp_di_fields_flag_t *)REG_ARIA_DISP_DI_FIELDS_FLAG).bitc.pre_field_flag_2;
}

void reg_aria_disp_set_di_fields_flag_ppre_field_flag_2(mt_u8 data)
{
    reg_aria_disp_di_fields_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_FIELDS_FLAG;
    d.bitc.ppre_field_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_FIELDS_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_fields_flag_ppre_field_flag_2(void)
{
    return (*(volatile reg_aria_disp_di_fields_flag_t *)REG_ARIA_DISP_DI_FIELDS_FLAG).bitc.ppre_field_flag_2;
}

void reg_aria_disp_set_di_fields_flag_nxt_field_flag(mt_u8 data)
{
    reg_aria_disp_di_fields_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_FIELDS_FLAG;
    d.bitc.nxt_field_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_FIELDS_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_fields_flag_nxt_field_flag(void)
{
    return (*(volatile reg_aria_disp_di_fields_flag_t *)REG_ARIA_DISP_DI_FIELDS_FLAG).bitc.nxt_field_flag;
}

void reg_aria_disp_set_di_fields_flag_cur_field_flag(mt_u8 data)
{
    reg_aria_disp_di_fields_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_FIELDS_FLAG;
    d.bitc.cur_field_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_FIELDS_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_fields_flag_cur_field_flag(void)
{
    return (*(volatile reg_aria_disp_di_fields_flag_t *)REG_ARIA_DISP_DI_FIELDS_FLAG).bitc.cur_field_flag;
}

void reg_aria_disp_set_di_fields_flag_pre_field_flag(mt_u8 data)
{
    reg_aria_disp_di_fields_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_FIELDS_FLAG;
    d.bitc.pre_field_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_FIELDS_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_fields_flag_pre_field_flag(void)
{
    return (*(volatile reg_aria_disp_di_fields_flag_t *)REG_ARIA_DISP_DI_FIELDS_FLAG).bitc.pre_field_flag;
}

void reg_aria_disp_set_di_fields_flag_ppre_field_flag(mt_u8 data)
{
    reg_aria_disp_di_fields_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_FIELDS_FLAG;
    d.bitc.ppre_field_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_FIELDS_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_fields_flag_ppre_field_flag(void)
{
    return (*(volatile reg_aria_disp_di_fields_flag_t *)REG_ARIA_DISP_DI_FIELDS_FLAG).bitc.ppre_field_flag;
}


/*!
  register ARIA_DISP_di_hevc_flag (read/write)
  */
void reg_aria_disp_set_di_hevc_flag(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_HEVC_FLAG, data);
}

mt_u32  reg_aria_disp_get_di_hevc_flag(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DI_HEVC_FLAG);
}

void reg_aria_disp_set_di_hevc_flag_nxt_hevc_flag_2(mt_u8 data)
{
    reg_aria_disp_di_hevc_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_HEVC_FLAG;
    d.bitc.nxt_hevc_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_HEVC_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_hevc_flag_nxt_hevc_flag_2(void)
{
    return (*(volatile reg_aria_disp_di_hevc_flag_t *)REG_ARIA_DISP_DI_HEVC_FLAG).bitc.nxt_hevc_flag_2;
}

void reg_aria_disp_set_di_hevc_flag_cur_hevc_flag_2(mt_u8 data)
{
    reg_aria_disp_di_hevc_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_HEVC_FLAG;
    d.bitc.cur_hevc_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_HEVC_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_hevc_flag_cur_hevc_flag_2(void)
{
    return (*(volatile reg_aria_disp_di_hevc_flag_t *)REG_ARIA_DISP_DI_HEVC_FLAG).bitc.cur_hevc_flag_2;
}

void reg_aria_disp_set_di_hevc_flag_pre_hevc_flag_2(mt_u8 data)
{
    reg_aria_disp_di_hevc_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_HEVC_FLAG;
    d.bitc.pre_hevc_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_HEVC_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_hevc_flag_pre_hevc_flag_2(void)
{
    return (*(volatile reg_aria_disp_di_hevc_flag_t *)REG_ARIA_DISP_DI_HEVC_FLAG).bitc.pre_hevc_flag_2;
}

void reg_aria_disp_set_di_hevc_flag_ppre_hevc_flag_2(mt_u8 data)
{
    reg_aria_disp_di_hevc_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_HEVC_FLAG;
    d.bitc.ppre_hevc_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_HEVC_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_hevc_flag_ppre_hevc_flag_2(void)
{
    return (*(volatile reg_aria_disp_di_hevc_flag_t *)REG_ARIA_DISP_DI_HEVC_FLAG).bitc.ppre_hevc_flag_2;
}

void reg_aria_disp_set_di_hevc_flag_nxt_hevc_flag(mt_u8 data)
{
    reg_aria_disp_di_hevc_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_HEVC_FLAG;
    d.bitc.nxt_hevc_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_HEVC_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_hevc_flag_nxt_hevc_flag(void)
{
    return (*(volatile reg_aria_disp_di_hevc_flag_t *)REG_ARIA_DISP_DI_HEVC_FLAG).bitc.nxt_hevc_flag;
}

void reg_aria_disp_set_di_hevc_flag_cur_hevc_flag(mt_u8 data)
{
    reg_aria_disp_di_hevc_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_HEVC_FLAG;
    d.bitc.cur_hevc_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_HEVC_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_hevc_flag_cur_hevc_flag(void)
{
    return (*(volatile reg_aria_disp_di_hevc_flag_t *)REG_ARIA_DISP_DI_HEVC_FLAG).bitc.cur_hevc_flag;
}

void reg_aria_disp_set_di_hevc_flag_pre_hevc_flag(mt_u8 data)
{
    reg_aria_disp_di_hevc_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_HEVC_FLAG;
    d.bitc.pre_hevc_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_HEVC_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_hevc_flag_pre_hevc_flag(void)
{
    return (*(volatile reg_aria_disp_di_hevc_flag_t *)REG_ARIA_DISP_DI_HEVC_FLAG).bitc.pre_hevc_flag;
}

void reg_aria_disp_set_di_hevc_flag_ppre_hevc_flag(mt_u8 data)
{
    reg_aria_disp_di_hevc_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DI_HEVC_FLAG;
    d.bitc.ppre_hevc_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DI_HEVC_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_di_hevc_flag_ppre_hevc_flag(void)
{
    return (*(volatile reg_aria_disp_di_hevc_flag_t *)REG_ARIA_DISP_DI_HEVC_FLAG).bitc.ppre_hevc_flag;
}


/*!
  register ARIA_DISP_none_di_progressive_flag (read/write)
  */
void reg_aria_disp_set_none_di_progressive_flag(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG, data);
}

mt_u32  reg_aria_disp_get_none_di_progressive_flag(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG);
}

void reg_aria_disp_set_none_di_progressive_flag_progressive_frame_2(mt_u8 data)
{
    reg_aria_disp_none_di_progressive_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG;
    d.bitc.progressive_frame_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_none_di_progressive_flag_progressive_frame_2(void)
{
    return (*(volatile reg_aria_disp_none_di_progressive_flag_t *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG).bitc.progressive_frame_2;
}

void reg_aria_disp_set_none_di_progressive_flag_progressive_frame_0(mt_u8 data)
{
    reg_aria_disp_none_di_progressive_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG;
    d.bitc.progressive_frame_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_none_di_progressive_flag_progressive_frame_0(void)
{
    return (*(volatile reg_aria_disp_none_di_progressive_flag_t *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG).bitc.progressive_frame_0;
}


/*!
  register ARIA_DISP_none_di_fields_flag (read/write)
  */
void reg_aria_disp_set_none_di_fields_flag(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG, data);
}

mt_u32  reg_aria_disp_get_none_di_fields_flag(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG);
}

void reg_aria_disp_set_none_di_fields_flag_cur_bot_field_flag_2(mt_u8 data)
{
    reg_aria_disp_none_di_fields_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG;
    d.bitc.cur_bot_field_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_none_di_fields_flag_cur_bot_field_flag_2(void)
{
    return (*(volatile reg_aria_disp_none_di_fields_flag_t *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG).bitc.cur_bot_field_flag_2;
}

void reg_aria_disp_set_none_di_fields_flag_cur_top_field_flag_2(mt_u8 data)
{
    reg_aria_disp_none_di_fields_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG;
    d.bitc.cur_top_field_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_none_di_fields_flag_cur_top_field_flag_2(void)
{
    return (*(volatile reg_aria_disp_none_di_fields_flag_t *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG).bitc.cur_top_field_flag_2;
}

void reg_aria_disp_set_none_di_fields_flag_cur_bot_field_flag_0(mt_u8 data)
{
    reg_aria_disp_none_di_fields_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG;
    d.bitc.cur_bot_field_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_none_di_fields_flag_cur_bot_field_flag_0(void)
{
    return (*(volatile reg_aria_disp_none_di_fields_flag_t *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG).bitc.cur_bot_field_flag_0;
}

void reg_aria_disp_set_none_di_fields_flag_cur_top_field_flag_0(mt_u8 data)
{
    reg_aria_disp_none_di_fields_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG;
    d.bitc.cur_top_field_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_none_di_fields_flag_cur_top_field_flag_0(void)
{
    return (*(volatile reg_aria_disp_none_di_fields_flag_t *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG).bitc.cur_top_field_flag_0;
}


/*!
  register ARIA_DISP_none_di_hevc_flag (read/write)
  */
void reg_aria_disp_set_none_di_hevc_flag(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG, data);
}

mt_u32  reg_aria_disp_get_none_di_hevc_flag(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG);
}

void reg_aria_disp_set_none_di_hevc_flag_cur_bot_hevc_flag_2(mt_u8 data)
{
    reg_aria_disp_none_di_hevc_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG;
    d.bitc.cur_bot_hevc_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_none_di_hevc_flag_cur_bot_hevc_flag_2(void)
{
    return (*(volatile reg_aria_disp_none_di_hevc_flag_t *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG).bitc.cur_bot_hevc_flag_2;
}

void reg_aria_disp_set_none_di_hevc_flag_cur_top_hevc_flag_2(mt_u8 data)
{
    reg_aria_disp_none_di_hevc_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG;
    d.bitc.cur_top_hevc_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_none_di_hevc_flag_cur_top_hevc_flag_2(void)
{
    return (*(volatile reg_aria_disp_none_di_hevc_flag_t *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG).bitc.cur_top_hevc_flag_2;
}

void reg_aria_disp_set_none_di_hevc_flag_cur_bot_hevc_flag_0(mt_u8 data)
{
    reg_aria_disp_none_di_hevc_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG;
    d.bitc.cur_bot_hevc_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_none_di_hevc_flag_cur_bot_hevc_flag_0(void)
{
    return (*(volatile reg_aria_disp_none_di_hevc_flag_t *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG).bitc.cur_bot_hevc_flag_0;
}

void reg_aria_disp_set_none_di_hevc_flag_cur_top_hevc_flag_0(mt_u8 data)
{
    reg_aria_disp_none_di_hevc_flag_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG;
    d.bitc.cur_top_hevc_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG, d.all);
}

mt_u8   reg_aria_disp_get_none_di_hevc_flag_cur_top_hevc_flag_0(void)
{
    return (*(volatile reg_aria_disp_none_di_hevc_flag_t *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG).bitc.cur_top_hevc_flag_0;
}


/*!
  register ARIA_DISP_none_di_progressive_flag_2nd (read/write)
  */
void reg_aria_disp_set_none_di_progressive_flag_2nd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG_2ND, data);
}

mt_u32  reg_aria_disp_get_none_di_progressive_flag_2nd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG_2ND);
}

void reg_aria_disp_set_none_di_progressive_flag_2nd_progressive_frame_2(mt_u8 data)
{
    reg_aria_disp_none_di_progressive_flag_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG_2ND;
    d.bitc.progressive_frame_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG_2ND, d.all);
}

mt_u8   reg_aria_disp_get_none_di_progressive_flag_2nd_progressive_frame_2(void)
{
    return (*(volatile reg_aria_disp_none_di_progressive_flag_2nd_t *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG_2ND).bitc.progressive_frame_2;
}

void reg_aria_disp_set_none_di_progressive_flag_2nd_progressive_frame_0(mt_u8 data)
{
    reg_aria_disp_none_di_progressive_flag_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG_2ND;
    d.bitc.progressive_frame_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG_2ND, d.all);
}

mt_u8   reg_aria_disp_get_none_di_progressive_flag_2nd_progressive_frame_0(void)
{
    return (*(volatile reg_aria_disp_none_di_progressive_flag_2nd_t *)REG_ARIA_DISP_NONE_DI_PROGRESSIVE_FLAG_2ND).bitc.progressive_frame_0;
}


/*!
  register ARIA_DISP_none_di_fields_flag_2nd (read/write)
  */
void reg_aria_disp_set_none_di_fields_flag_2nd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND, data);
}

mt_u32  reg_aria_disp_get_none_di_fields_flag_2nd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND);
}

void reg_aria_disp_set_none_di_fields_flag_2nd_cur_bot_field_flag_2(mt_u8 data)
{
    reg_aria_disp_none_di_fields_flag_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND;
    d.bitc.cur_bot_field_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND, d.all);
}

mt_u8   reg_aria_disp_get_none_di_fields_flag_2nd_cur_bot_field_flag_2(void)
{
    return (*(volatile reg_aria_disp_none_di_fields_flag_2nd_t *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND).bitc.cur_bot_field_flag_2;
}

void reg_aria_disp_set_none_di_fields_flag_2nd_cur_top_field_flag_2(mt_u8 data)
{
    reg_aria_disp_none_di_fields_flag_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND;
    d.bitc.cur_top_field_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND, d.all);
}

mt_u8   reg_aria_disp_get_none_di_fields_flag_2nd_cur_top_field_flag_2(void)
{
    return (*(volatile reg_aria_disp_none_di_fields_flag_2nd_t *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND).bitc.cur_top_field_flag_2;
}

void reg_aria_disp_set_none_di_fields_flag_2nd_cur_bot_field_flag_0(mt_u8 data)
{
    reg_aria_disp_none_di_fields_flag_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND;
    d.bitc.cur_bot_field_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND, d.all);
}

mt_u8   reg_aria_disp_get_none_di_fields_flag_2nd_cur_bot_field_flag_0(void)
{
    return (*(volatile reg_aria_disp_none_di_fields_flag_2nd_t *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND).bitc.cur_bot_field_flag_0;
}

void reg_aria_disp_set_none_di_fields_flag_2nd_cur_top_field_flag_0(mt_u8 data)
{
    reg_aria_disp_none_di_fields_flag_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND;
    d.bitc.cur_top_field_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND, d.all);
}

mt_u8   reg_aria_disp_get_none_di_fields_flag_2nd_cur_top_field_flag_0(void)
{
    return (*(volatile reg_aria_disp_none_di_fields_flag_2nd_t *)REG_ARIA_DISP_NONE_DI_FIELDS_FLAG_2ND).bitc.cur_top_field_flag_0;
}


/*!
  register ARIA_DISP_none_di_hevc_flag_2nd (read/write)
  */
void reg_aria_disp_set_none_di_hevc_flag_2nd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND, data);
}

mt_u32  reg_aria_disp_get_none_di_hevc_flag_2nd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND);
}

void reg_aria_disp_set_none_di_hevc_flag_2nd_cur_bot_hevc_flag_2(mt_u8 data)
{
    reg_aria_disp_none_di_hevc_flag_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND;
    d.bitc.cur_bot_hevc_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND, d.all);
}

mt_u8   reg_aria_disp_get_none_di_hevc_flag_2nd_cur_bot_hevc_flag_2(void)
{
    return (*(volatile reg_aria_disp_none_di_hevc_flag_2nd_t *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND).bitc.cur_bot_hevc_flag_2;
}

void reg_aria_disp_set_none_di_hevc_flag_2nd_cur_top_hevc_flag_2(mt_u8 data)
{
    reg_aria_disp_none_di_hevc_flag_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND;
    d.bitc.cur_top_hevc_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND, d.all);
}

mt_u8   reg_aria_disp_get_none_di_hevc_flag_2nd_cur_top_hevc_flag_2(void)
{
    return (*(volatile reg_aria_disp_none_di_hevc_flag_2nd_t *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND).bitc.cur_top_hevc_flag_2;
}

void reg_aria_disp_set_none_di_hevc_flag_2nd_cur_bot_hevc_flag_0(mt_u8 data)
{
    reg_aria_disp_none_di_hevc_flag_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND;
    d.bitc.cur_bot_hevc_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND, d.all);
}

mt_u8   reg_aria_disp_get_none_di_hevc_flag_2nd_cur_bot_hevc_flag_0(void)
{
    return (*(volatile reg_aria_disp_none_di_hevc_flag_2nd_t *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND).bitc.cur_bot_hevc_flag_0;
}

void reg_aria_disp_set_none_di_hevc_flag_2nd_cur_top_hevc_flag_0(mt_u8 data)
{
    reg_aria_disp_none_di_hevc_flag_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND;
    d.bitc.cur_top_hevc_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND, d.all);
}

mt_u8   reg_aria_disp_get_none_di_hevc_flag_2nd_cur_top_hevc_flag_0(void)
{
    return (*(volatile reg_aria_disp_none_di_hevc_flag_2nd_t *)REG_ARIA_DISP_NONE_DI_HEVC_FLAG_2ND).bitc.cur_top_hevc_flag_0;
}


/*!
  register ARIA_DISP_motion_pre_addr_0 (read/write)
  */
void reg_aria_disp_set_motion_pre_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_PRE_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_motion_pre_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_MOTION_PRE_ADDR_0);
}

void reg_aria_disp_set_motion_pre_addr_0_motion_pre_addr_0(mt_u32 data)
{
    reg_aria_disp_motion_pre_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_MOTION_PRE_ADDR_0;
    d.bitc.motion_pre_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_PRE_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_motion_pre_addr_0_motion_pre_addr_0(void)
{
    return (*(volatile reg_aria_disp_motion_pre_addr_0_t *)REG_ARIA_DISP_MOTION_PRE_ADDR_0).bitc.motion_pre_addr_0;
}


/*!
  register ARIA_DISP_motion_cur_addr_0 (read/write)
  */
void reg_aria_disp_set_motion_cur_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_CUR_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_motion_cur_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_MOTION_CUR_ADDR_0);
}

void reg_aria_disp_set_motion_cur_addr_0_motion_cur_addr_0(mt_u32 data)
{
    reg_aria_disp_motion_cur_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_MOTION_CUR_ADDR_0;
    d.bitc.motion_cur_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_CUR_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_motion_cur_addr_0_motion_cur_addr_0(void)
{
    return (*(volatile reg_aria_disp_motion_cur_addr_0_t *)REG_ARIA_DISP_MOTION_CUR_ADDR_0).bitc.motion_cur_addr_0;
}


/*!
  register ARIA_DISP_motion_pre_addr_2 (read/write)
  */
void reg_aria_disp_set_motion_pre_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_PRE_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_motion_pre_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_MOTION_PRE_ADDR_2);
}

void reg_aria_disp_set_motion_pre_addr_2_motion_pre_addr_2(mt_u32 data)
{
    reg_aria_disp_motion_pre_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_MOTION_PRE_ADDR_2;
    d.bitc.motion_pre_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_PRE_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_motion_pre_addr_2_motion_pre_addr_2(void)
{
    return (*(volatile reg_aria_disp_motion_pre_addr_2_t *)REG_ARIA_DISP_MOTION_PRE_ADDR_2).bitc.motion_pre_addr_2;
}


/*!
  register ARIA_DISP_motion_cur_addr_2 (read/write)
  */
void reg_aria_disp_set_motion_cur_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_CUR_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_motion_cur_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_MOTION_CUR_ADDR_2);
}

void reg_aria_disp_set_motion_cur_addr_2_motion_cur_addr_2(mt_u32 data)
{
    reg_aria_disp_motion_cur_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_MOTION_CUR_ADDR_2;
    d.bitc.motion_cur_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_CUR_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_motion_cur_addr_2_motion_cur_addr_2(void)
{
    return (*(volatile reg_aria_disp_motion_cur_addr_2_t *)REG_ARIA_DISP_MOTION_CUR_ADDR_2).bitc.motion_cur_addr_2;
}


/*!
  register ARIA_DISP_luma_pre_addr_0 (read/write)
  */
void reg_aria_disp_set_luma_pre_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_PRE_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_luma_pre_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_LUMA_PRE_ADDR_0);
}

void reg_aria_disp_set_luma_pre_addr_0_luma_pre_addr_0(mt_u32 data)
{
    reg_aria_disp_luma_pre_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_LUMA_PRE_ADDR_0;
    d.bitc.luma_pre_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_PRE_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_luma_pre_addr_0_luma_pre_addr_0(void)
{
    return (*(volatile reg_aria_disp_luma_pre_addr_0_t *)REG_ARIA_DISP_LUMA_PRE_ADDR_0).bitc.luma_pre_addr_0;
}


/*!
  register ARIA_DISP_luma_top_cur_addr_0 (read/write)
  */
void reg_aria_disp_set_luma_top_cur_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_luma_top_cur_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_0);
}

void reg_aria_disp_set_luma_top_cur_addr_0_luma_top_cur_addr_0(mt_u32 data)
{
    reg_aria_disp_luma_top_cur_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_0;
    d.bitc.luma_top_cur_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_luma_top_cur_addr_0_luma_top_cur_addr_0(void)
{
    return (*(volatile reg_aria_disp_luma_top_cur_addr_0_t *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_0).bitc.luma_top_cur_addr_0;
}


/*!
  register ARIA_DISP_luma_bot_cur_addr_0 (read/write)
  */
void reg_aria_disp_set_luma_bot_cur_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_luma_bot_cur_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_0);
}

void reg_aria_disp_set_luma_bot_cur_addr_0_luma_bot_cur_addr_0(mt_u32 data)
{
    reg_aria_disp_luma_bot_cur_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_0;
    d.bitc.luma_bot_cur_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_luma_bot_cur_addr_0_luma_bot_cur_addr_0(void)
{
    return (*(volatile reg_aria_disp_luma_bot_cur_addr_0_t *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_0).bitc.luma_bot_cur_addr_0;
}


/*!
  register ARIA_DISP_luma_nxt_addr_0 (read/write)
  */
void reg_aria_disp_set_luma_nxt_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_NXT_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_luma_nxt_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_LUMA_NXT_ADDR_0);
}

void reg_aria_disp_set_luma_nxt_addr_0_luma_nxt_addr_0(mt_u32 data)
{
    reg_aria_disp_luma_nxt_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_LUMA_NXT_ADDR_0;
    d.bitc.luma_nxt_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_NXT_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_luma_nxt_addr_0_luma_nxt_addr_0(void)
{
    return (*(volatile reg_aria_disp_luma_nxt_addr_0_t *)REG_ARIA_DISP_LUMA_NXT_ADDR_0).bitc.luma_nxt_addr_0;
}


/*!
  register ARIA_DISP_luma_top_cur_addr_2nd_0 (read/write)
  */
void reg_aria_disp_set_luma_top_cur_addr_2nd_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2ND_0, data);
}

mt_u32  reg_aria_disp_get_luma_top_cur_addr_2nd_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2ND_0);
}

void reg_aria_disp_set_luma_top_cur_addr_2nd_0_luma_top_cur_addr_2nd_0(mt_u32 data)
{
    reg_aria_disp_luma_top_cur_addr_2nd_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2ND_0;
    d.bitc.luma_top_cur_addr_2nd_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2ND_0, d.all);
}

mt_u32  reg_aria_disp_get_luma_top_cur_addr_2nd_0_luma_top_cur_addr_2nd_0(void)
{
    return (*(volatile reg_aria_disp_luma_top_cur_addr_2nd_0_t *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2ND_0).bitc.luma_top_cur_addr_2nd_0;
}


/*!
  register ARIA_DISP_luma_bot_cur_addr_2nd_0 (read/write)
  */
void reg_aria_disp_set_luma_bot_cur_addr_2nd_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2ND_0, data);
}

mt_u32  reg_aria_disp_get_luma_bot_cur_addr_2nd_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2ND_0);
}

void reg_aria_disp_set_luma_bot_cur_addr_2nd_0_luma_bot_cur_addr_2nd_0(mt_u32 data)
{
    reg_aria_disp_luma_bot_cur_addr_2nd_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2ND_0;
    d.bitc.luma_bot_cur_addr_2nd_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2ND_0, d.all);
}

mt_u32  reg_aria_disp_get_luma_bot_cur_addr_2nd_0_luma_bot_cur_addr_2nd_0(void)
{
    return (*(volatile reg_aria_disp_luma_bot_cur_addr_2nd_0_t *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2ND_0).bitc.luma_bot_cur_addr_2nd_0;
}


/*!
  register ARIA_DISP_luma_pre_addr_2 (read/write)
  */
void reg_aria_disp_set_luma_pre_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_PRE_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_luma_pre_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_LUMA_PRE_ADDR_2);
}

void reg_aria_disp_set_luma_pre_addr_2_luma_pre_addr_2(mt_u32 data)
{
    reg_aria_disp_luma_pre_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_LUMA_PRE_ADDR_2;
    d.bitc.luma_pre_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_PRE_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_luma_pre_addr_2_luma_pre_addr_2(void)
{
    return (*(volatile reg_aria_disp_luma_pre_addr_2_t *)REG_ARIA_DISP_LUMA_PRE_ADDR_2).bitc.luma_pre_addr_2;
}


/*!
  register ARIA_DISP_luma_top_cur_addr_2 (read/write)
  */
void reg_aria_disp_set_luma_top_cur_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_luma_top_cur_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2);
}

void reg_aria_disp_set_luma_top_cur_addr_2_luma_top_cur_addr_2(mt_u32 data)
{
    reg_aria_disp_luma_top_cur_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2;
    d.bitc.luma_top_cur_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_luma_top_cur_addr_2_luma_top_cur_addr_2(void)
{
    return (*(volatile reg_aria_disp_luma_top_cur_addr_2_t *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2).bitc.luma_top_cur_addr_2;
}


/*!
  register ARIA_DISP_luma_bot_cur_addr_2 (read/write)
  */
void reg_aria_disp_set_luma_bot_cur_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_luma_bot_cur_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2);
}

void reg_aria_disp_set_luma_bot_cur_addr_2_luma_bot_cur_addr_2(mt_u32 data)
{
    reg_aria_disp_luma_bot_cur_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2;
    d.bitc.luma_bot_cur_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_luma_bot_cur_addr_2_luma_bot_cur_addr_2(void)
{
    return (*(volatile reg_aria_disp_luma_bot_cur_addr_2_t *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2).bitc.luma_bot_cur_addr_2;
}


/*!
  register ARIA_DISP_luma_nxt_addr_2 (read/write)
  */
void reg_aria_disp_set_luma_nxt_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_NXT_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_luma_nxt_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_LUMA_NXT_ADDR_2);
}

void reg_aria_disp_set_luma_nxt_addr_2_luma_nxt_addr_2(mt_u32 data)
{
    reg_aria_disp_luma_nxt_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_LUMA_NXT_ADDR_2;
    d.bitc.luma_nxt_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_NXT_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_luma_nxt_addr_2_luma_nxt_addr_2(void)
{
    return (*(volatile reg_aria_disp_luma_nxt_addr_2_t *)REG_ARIA_DISP_LUMA_NXT_ADDR_2).bitc.luma_nxt_addr_2;
}


/*!
  register ARIA_DISP_luma_top_cur_addr_2nd_2 (read/write)
  */
void reg_aria_disp_set_luma_top_cur_addr_2nd_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2ND_2, data);
}

mt_u32  reg_aria_disp_get_luma_top_cur_addr_2nd_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2ND_2);
}

void reg_aria_disp_set_luma_top_cur_addr_2nd_2_luma_top_cur_addr_2nd_2(mt_u32 data)
{
    reg_aria_disp_luma_top_cur_addr_2nd_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2ND_2;
    d.bitc.luma_top_cur_addr_2nd_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2ND_2, d.all);
}

mt_u32  reg_aria_disp_get_luma_top_cur_addr_2nd_2_luma_top_cur_addr_2nd_2(void)
{
    return (*(volatile reg_aria_disp_luma_top_cur_addr_2nd_2_t *)REG_ARIA_DISP_LUMA_TOP_CUR_ADDR_2ND_2).bitc.luma_top_cur_addr_2nd_2;
}


/*!
  register ARIA_DISP_luma_bot_cur_addr_2nd_2 (read/write)
  */
void reg_aria_disp_set_luma_bot_cur_addr_2nd_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2ND_2, data);
}

mt_u32  reg_aria_disp_get_luma_bot_cur_addr_2nd_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2ND_2);
}

void reg_aria_disp_set_luma_bot_cur_addr_2nd_2_luma_bot_cur_addr_2nd_2(mt_u32 data)
{
    reg_aria_disp_luma_bot_cur_addr_2nd_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2ND_2;
    d.bitc.luma_bot_cur_addr_2nd_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2ND_2, d.all);
}

mt_u32  reg_aria_disp_get_luma_bot_cur_addr_2nd_2_luma_bot_cur_addr_2nd_2(void)
{
    return (*(volatile reg_aria_disp_luma_bot_cur_addr_2nd_2_t *)REG_ARIA_DISP_LUMA_BOT_CUR_ADDR_2ND_2).bitc.luma_bot_cur_addr_2nd_2;
}


/*!
  register ARIA_DISP_chroma_ppre_addr_0 (read/write)
  */
void reg_aria_disp_set_chroma_ppre_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_PPRE_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_chroma_ppre_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_PPRE_ADDR_0);
}

void reg_aria_disp_set_chroma_ppre_addr_0_chroma_ppre_addr_0(mt_u32 data)
{
    reg_aria_disp_chroma_ppre_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_PPRE_ADDR_0;
    d.bitc.chroma_ppre_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_PPRE_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_chroma_ppre_addr_0_chroma_ppre_addr_0(void)
{
    return (*(volatile reg_aria_disp_chroma_ppre_addr_0_t *)REG_ARIA_DISP_CHROMA_PPRE_ADDR_0).bitc.chroma_ppre_addr_0;
}


/*!
  register ARIA_DISP_chroma_pre_addr_0 (read/write)
  */
void reg_aria_disp_set_chroma_pre_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_PRE_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_chroma_pre_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_PRE_ADDR_0);
}

void reg_aria_disp_set_chroma_pre_addr_0_chroma_pre_addr_0(mt_u32 data)
{
    reg_aria_disp_chroma_pre_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_PRE_ADDR_0;
    d.bitc.chroma_pre_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_PRE_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_chroma_pre_addr_0_chroma_pre_addr_0(void)
{
    return (*(volatile reg_aria_disp_chroma_pre_addr_0_t *)REG_ARIA_DISP_CHROMA_PRE_ADDR_0).bitc.chroma_pre_addr_0;
}


/*!
  register ARIA_DISP_chroma_top_cur_addr_0 (read/write)
  */
void reg_aria_disp_set_chroma_top_cur_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_chroma_top_cur_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_0);
}

void reg_aria_disp_set_chroma_top_cur_addr_0_chroma_top_cur_addr_0(mt_u32 data)
{
    reg_aria_disp_chroma_top_cur_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_0;
    d.bitc.chroma_top_cur_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_chroma_top_cur_addr_0_chroma_top_cur_addr_0(void)
{
    return (*(volatile reg_aria_disp_chroma_top_cur_addr_0_t *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_0).bitc.chroma_top_cur_addr_0;
}


/*!
  register ARIA_DISP_chroma_bot_cur_addr_0 (read/write)
  */
void reg_aria_disp_set_chroma_bot_cur_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_chroma_bot_cur_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_0);
}

void reg_aria_disp_set_chroma_bot_cur_addr_0_chroma_bot_cur_addr_0(mt_u32 data)
{
    reg_aria_disp_chroma_bot_cur_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_0;
    d.bitc.chroma_bot_cur_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_chroma_bot_cur_addr_0_chroma_bot_cur_addr_0(void)
{
    return (*(volatile reg_aria_disp_chroma_bot_cur_addr_0_t *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_0).bitc.chroma_bot_cur_addr_0;
}


/*!
  register ARIA_DISP_chroma_nxt_addr_0 (read/write)
  */
void reg_aria_disp_set_chroma_nxt_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_NXT_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_chroma_nxt_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_NXT_ADDR_0);
}

void reg_aria_disp_set_chroma_nxt_addr_0_chroma_nxt_addr_0(mt_u32 data)
{
    reg_aria_disp_chroma_nxt_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_NXT_ADDR_0;
    d.bitc.chroma_nxt_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_NXT_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_chroma_nxt_addr_0_chroma_nxt_addr_0(void)
{
    return (*(volatile reg_aria_disp_chroma_nxt_addr_0_t *)REG_ARIA_DISP_CHROMA_NXT_ADDR_0).bitc.chroma_nxt_addr_0;
}


/*!
  register ARIA_DISP_chroma_top_cur_addr_2nd_0 (read/write)
  */
void reg_aria_disp_set_chroma_top_cur_addr_2nd_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2ND_0, data);
}

mt_u32  reg_aria_disp_get_chroma_top_cur_addr_2nd_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2ND_0);
}

void reg_aria_disp_set_chroma_top_cur_addr_2nd_0_chroma_top_cur_addr_2nd_0(mt_u32 data)
{
    reg_aria_disp_chroma_top_cur_addr_2nd_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2ND_0;
    d.bitc.chroma_top_cur_addr_2nd_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2ND_0, d.all);
}

mt_u32  reg_aria_disp_get_chroma_top_cur_addr_2nd_0_chroma_top_cur_addr_2nd_0(void)
{
    return (*(volatile reg_aria_disp_chroma_top_cur_addr_2nd_0_t *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2ND_0).bitc.chroma_top_cur_addr_2nd_0;
}


/*!
  register ARIA_DISP_chroma_bot_cur_addr_2nd_0 (read/write)
  */
void reg_aria_disp_set_chroma_bot_cur_addr_2nd_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2ND_0, data);
}

mt_u32  reg_aria_disp_get_chroma_bot_cur_addr_2nd_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2ND_0);
}

void reg_aria_disp_set_chroma_bot_cur_addr_2nd_0_chroma_bot_cur_addr_2nd_0(mt_u32 data)
{
    reg_aria_disp_chroma_bot_cur_addr_2nd_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2ND_0;
    d.bitc.chroma_bot_cur_addr_2nd_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2ND_0, d.all);
}

mt_u32  reg_aria_disp_get_chroma_bot_cur_addr_2nd_0_chroma_bot_cur_addr_2nd_0(void)
{
    return (*(volatile reg_aria_disp_chroma_bot_cur_addr_2nd_0_t *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2ND_0).bitc.chroma_bot_cur_addr_2nd_0;
}


/*!
  register ARIA_DISP_chroma_ppre_addr_2 (read/write)
  */
void reg_aria_disp_set_chroma_ppre_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_PPRE_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_chroma_ppre_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_PPRE_ADDR_2);
}

void reg_aria_disp_set_chroma_ppre_addr_2_chroma_ppre_addr_2(mt_u32 data)
{
    reg_aria_disp_chroma_ppre_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_PPRE_ADDR_2;
    d.bitc.chroma_ppre_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_PPRE_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_chroma_ppre_addr_2_chroma_ppre_addr_2(void)
{
    return (*(volatile reg_aria_disp_chroma_ppre_addr_2_t *)REG_ARIA_DISP_CHROMA_PPRE_ADDR_2).bitc.chroma_ppre_addr_2;
}


/*!
  register ARIA_DISP_chroma_pre_addr_2 (read/write)
  */
void reg_aria_disp_set_chroma_pre_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_PRE_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_chroma_pre_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_PRE_ADDR_2);
}

void reg_aria_disp_set_chroma_pre_addr_2_chroma_pre_addr_2(mt_u32 data)
{
    reg_aria_disp_chroma_pre_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_PRE_ADDR_2;
    d.bitc.chroma_pre_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_PRE_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_chroma_pre_addr_2_chroma_pre_addr_2(void)
{
    return (*(volatile reg_aria_disp_chroma_pre_addr_2_t *)REG_ARIA_DISP_CHROMA_PRE_ADDR_2).bitc.chroma_pre_addr_2;
}


/*!
  register ARIA_DISP_chroma_top_cur_addr_2 (read/write)
  */
void reg_aria_disp_set_chroma_top_cur_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_chroma_top_cur_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2);
}

void reg_aria_disp_set_chroma_top_cur_addr_2_chroma_top_cur_addr_2(mt_u32 data)
{
    reg_aria_disp_chroma_top_cur_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2;
    d.bitc.chroma_top_cur_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_chroma_top_cur_addr_2_chroma_top_cur_addr_2(void)
{
    return (*(volatile reg_aria_disp_chroma_top_cur_addr_2_t *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2).bitc.chroma_top_cur_addr_2;
}


/*!
  register ARIA_DISP_chroma_bot_cur_addr_2 (read/write)
  */
void reg_aria_disp_set_chroma_bot_cur_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_chroma_bot_cur_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2);
}

void reg_aria_disp_set_chroma_bot_cur_addr_2_chroma_bot_cur_addr_2(mt_u32 data)
{
    reg_aria_disp_chroma_bot_cur_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2;
    d.bitc.chroma_bot_cur_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_chroma_bot_cur_addr_2_chroma_bot_cur_addr_2(void)
{
    return (*(volatile reg_aria_disp_chroma_bot_cur_addr_2_t *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2).bitc.chroma_bot_cur_addr_2;
}


/*!
  register ARIA_DISP_chroma_nxt_addr_2 (read/write)
  */
void reg_aria_disp_set_chroma_nxt_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_NXT_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_chroma_nxt_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_NXT_ADDR_2);
}

void reg_aria_disp_set_chroma_nxt_addr_2_chroma_nxt_addr_2(mt_u32 data)
{
    reg_aria_disp_chroma_nxt_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_NXT_ADDR_2;
    d.bitc.chroma_nxt_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_NXT_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_chroma_nxt_addr_2_chroma_nxt_addr_2(void)
{
    return (*(volatile reg_aria_disp_chroma_nxt_addr_2_t *)REG_ARIA_DISP_CHROMA_NXT_ADDR_2).bitc.chroma_nxt_addr_2;
}


/*!
  register ARIA_DISP_chroma_top_cur_addr_2nd_2 (read/write)
  */
void reg_aria_disp_set_chroma_top_cur_addr_2nd_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2ND_2, data);
}

mt_u32  reg_aria_disp_get_chroma_top_cur_addr_2nd_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2ND_2);
}

void reg_aria_disp_set_chroma_top_cur_addr_2nd_2_chroma_top_cur_addr_2nd_2(mt_u32 data)
{
    reg_aria_disp_chroma_top_cur_addr_2nd_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2ND_2;
    d.bitc.chroma_top_cur_addr_2nd_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2ND_2, d.all);
}

mt_u32  reg_aria_disp_get_chroma_top_cur_addr_2nd_2_chroma_top_cur_addr_2nd_2(void)
{
    return (*(volatile reg_aria_disp_chroma_top_cur_addr_2nd_2_t *)REG_ARIA_DISP_CHROMA_TOP_CUR_ADDR_2ND_2).bitc.chroma_top_cur_addr_2nd_2;
}


/*!
  register ARIA_DISP_chroma_bot_cur_addr_2nd_2 (read/write)
  */
void reg_aria_disp_set_chroma_bot_cur_addr_2nd_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2ND_2, data);
}

mt_u32  reg_aria_disp_get_chroma_bot_cur_addr_2nd_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2ND_2);
}

void reg_aria_disp_set_chroma_bot_cur_addr_2nd_2_chroma_bot_cur_addr_2nd_2(mt_u32 data)
{
    reg_aria_disp_chroma_bot_cur_addr_2nd_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2ND_2;
    d.bitc.chroma_bot_cur_addr_2nd_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2ND_2, d.all);
}

mt_u32  reg_aria_disp_get_chroma_bot_cur_addr_2nd_2_chroma_bot_cur_addr_2nd_2(void)
{
    return (*(volatile reg_aria_disp_chroma_bot_cur_addr_2nd_2_t *)REG_ARIA_DISP_CHROMA_BOT_CUR_ADDR_2ND_2).bitc.chroma_bot_cur_addr_2nd_2;
}


/*!
  register ARIA_DISP_down_scale_ctrl (read/write)
  */
void reg_aria_disp_set_down_scale_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL, data);
}

mt_u32  reg_aria_disp_get_down_scale_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL);
}

void reg_aria_disp_set_down_scale_ctrl_down_scale_wr_en_2(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL;
    d.bitc.down_scale_wr_en_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_down_scale_wr_en_2(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL).bitc.down_scale_wr_en_2;
}

void reg_aria_disp_set_down_scale_ctrl_down_scale_wr_en_0(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL;
    d.bitc.down_scale_wr_en_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_down_scale_wr_en_0(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL).bitc.down_scale_wr_en_0;
}

void reg_aria_disp_set_down_scale_ctrl_wr_top_field_flag_2(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL;
    d.bitc.wr_top_field_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_wr_top_field_flag_2(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL).bitc.wr_top_field_flag_2;
}

void reg_aria_disp_set_down_scale_ctrl_wr_top_field_flag_0(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL;
    d.bitc.wr_top_field_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_wr_top_field_flag_0(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL).bitc.wr_top_field_flag_0;
}

void reg_aria_disp_set_down_scale_ctrl_wr_progressive_flag_2(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL;
    d.bitc.wr_progressive_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_wr_progressive_flag_2(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL).bitc.wr_progressive_flag_2;
}

void reg_aria_disp_set_down_scale_ctrl_wr_progressive_flag_0(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL;
    d.bitc.wr_progressive_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_wr_progressive_flag_0(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL).bitc.wr_progressive_flag_0;
}

void reg_aria_disp_set_down_scale_ctrl_cbcr_fifo_full_thr(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL;
    d.bitc.cbcr_fifo_full_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_cbcr_fifo_full_thr(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL).bitc.cbcr_fifo_full_thr;
}

void reg_aria_disp_set_down_scale_ctrl_luma_fifo_full_thr(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL;
    d.bitc.luma_fifo_full_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_luma_fifo_full_thr(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL).bitc.luma_fifo_full_thr;
}


/*!
  register ARIA_DISP_down_scale_out_size (read/write)
  */
void reg_aria_disp_set_down_scale_out_size(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE, data);
}

mt_u32  reg_aria_disp_get_down_scale_out_size(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE);
}

void reg_aria_disp_set_down_scale_out_size_down_scale_output_height(mt_u16 data)
{
    reg_aria_disp_down_scale_out_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE;
    d.bitc.down_scale_output_height = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_down_scale_out_size_down_scale_output_height(void)
{
    return (*(volatile reg_aria_disp_down_scale_out_size_t *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE).bitc.down_scale_output_height;
}

void reg_aria_disp_set_down_scale_out_size_down_scale_output_width(mt_u16 data)
{
    reg_aria_disp_down_scale_out_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE;
    d.bitc.down_scale_output_width = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_down_scale_out_size_down_scale_output_width(void)
{
    return (*(volatile reg_aria_disp_down_scale_out_size_t *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE).bitc.down_scale_output_width;
}


/*!
  register ARIA_DISP_down_scale_wr_stride (read/write)
  */
void reg_aria_disp_set_down_scale_wr_stride(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_WR_STRIDE, data);
}

mt_u32  reg_aria_disp_get_down_scale_wr_stride(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_WR_STRIDE);
}

void reg_aria_disp_set_down_scale_wr_stride_down_scale_wr_stride(mt_u16 data)
{
    reg_aria_disp_down_scale_wr_stride_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_WR_STRIDE;
    d.bitc.down_scale_wr_stride = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_WR_STRIDE, d.all);
}

mt_u16  reg_aria_disp_get_down_scale_wr_stride_down_scale_wr_stride(void)
{
    return (*(volatile reg_aria_disp_down_scale_wr_stride_t *)REG_ARIA_DISP_DOWN_SCALE_WR_STRIDE).bitc.down_scale_wr_stride;
}


/*!
  register ARIA_DISP_down_scale_luma_wr_addr_0 (read/write)
  */
void reg_aria_disp_set_down_scale_luma_wr_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_down_scale_luma_wr_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_0);
}

void reg_aria_disp_set_down_scale_luma_wr_addr_0_down_scale_luma_wr_addr_0(mt_u32 data)
{
    reg_aria_disp_down_scale_luma_wr_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_0;
    d.bitc.down_scale_luma_wr_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_down_scale_luma_wr_addr_0_down_scale_luma_wr_addr_0(void)
{
    return (*(volatile reg_aria_disp_down_scale_luma_wr_addr_0_t *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_0).bitc.down_scale_luma_wr_addr_0;
}


/*!
  register ARIA_DISP_down_scale_cbcr_wr_addr_0 (read/write)
  */
void reg_aria_disp_set_down_scale_cbcr_wr_addr_0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_0, data);
}

mt_u32  reg_aria_disp_get_down_scale_cbcr_wr_addr_0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_0);
}

void reg_aria_disp_set_down_scale_cbcr_wr_addr_0_down_scale_cbcr_wr_addr_0(mt_u32 data)
{
    reg_aria_disp_down_scale_cbcr_wr_addr_0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_0;
    d.bitc.down_scale_cbcr_wr_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_0, d.all);
}

mt_u32  reg_aria_disp_get_down_scale_cbcr_wr_addr_0_down_scale_cbcr_wr_addr_0(void)
{
    return (*(volatile reg_aria_disp_down_scale_cbcr_wr_addr_0_t *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_0).bitc.down_scale_cbcr_wr_addr_0;
}


/*!
  register ARIA_DISP_down_scale_luma_wr_addr_2 (read/write)
  */
void reg_aria_disp_set_down_scale_luma_wr_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_down_scale_luma_wr_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_2);
}

void reg_aria_disp_set_down_scale_luma_wr_addr_2_down_scale_luma_wr_addr_2(mt_u32 data)
{
    reg_aria_disp_down_scale_luma_wr_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_2;
    d.bitc.down_scale_luma_wr_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_down_scale_luma_wr_addr_2_down_scale_luma_wr_addr_2(void)
{
    return (*(volatile reg_aria_disp_down_scale_luma_wr_addr_2_t *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_2).bitc.down_scale_luma_wr_addr_2;
}


/*!
  register ARIA_DISP_down_scale_cbcr_wr_addr_2 (read/write)
  */
void reg_aria_disp_set_down_scale_cbcr_wr_addr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_2, data);
}

mt_u32  reg_aria_disp_get_down_scale_cbcr_wr_addr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_2);
}

void reg_aria_disp_set_down_scale_cbcr_wr_addr_2_down_scale_cbcr_wr_addr_2(mt_u32 data)
{
    reg_aria_disp_down_scale_cbcr_wr_addr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_2;
    d.bitc.down_scale_cbcr_wr_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_down_scale_cbcr_wr_addr_2_down_scale_cbcr_wr_addr_2(void)
{
    return (*(volatile reg_aria_disp_down_scale_cbcr_wr_addr_2_t *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_2).bitc.down_scale_cbcr_wr_addr_2;
}


/*!
  register ARIA_DISP_down_scale_ctrl_2nd (read/write)
  */
void reg_aria_disp_set_down_scale_ctrl_2nd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND, data);
}

mt_u32  reg_aria_disp_get_down_scale_ctrl_2nd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND);
}

void reg_aria_disp_set_down_scale_ctrl_2nd_down_scale_wr_en_2(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND;
    d.bitc.down_scale_wr_en_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_2nd_down_scale_wr_en_2(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND).bitc.down_scale_wr_en_2;
}

void reg_aria_disp_set_down_scale_ctrl_2nd_down_scale_wr_en_0(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND;
    d.bitc.down_scale_wr_en_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_2nd_down_scale_wr_en_0(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND).bitc.down_scale_wr_en_0;
}

void reg_aria_disp_set_down_scale_ctrl_2nd_wr_top_field_flag_2(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND;
    d.bitc.wr_top_field_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_2nd_wr_top_field_flag_2(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND).bitc.wr_top_field_flag_2;
}

void reg_aria_disp_set_down_scale_ctrl_2nd_wr_top_field_flag_0(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND;
    d.bitc.wr_top_field_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_2nd_wr_top_field_flag_0(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND).bitc.wr_top_field_flag_0;
}

void reg_aria_disp_set_down_scale_ctrl_2nd_wr_progressive_flag_2(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND;
    d.bitc.wr_progressive_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_2nd_wr_progressive_flag_2(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND).bitc.wr_progressive_flag_2;
}

void reg_aria_disp_set_down_scale_ctrl_2nd_wr_progressive_flag_0(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND;
    d.bitc.wr_progressive_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_2nd_wr_progressive_flag_0(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND).bitc.wr_progressive_flag_0;
}

void reg_aria_disp_set_down_scale_ctrl_2nd_cbcr_fifo_full_thr(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND;
    d.bitc.cbcr_fifo_full_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_2nd_cbcr_fifo_full_thr(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND).bitc.cbcr_fifo_full_thr;
}

void reg_aria_disp_set_down_scale_ctrl_2nd_luma_fifo_full_thr(mt_u8 data)
{
    reg_aria_disp_down_scale_ctrl_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND;
    d.bitc.luma_fifo_full_thr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_ctrl_2nd_luma_fifo_full_thr(void)
{
    return (*(volatile reg_aria_disp_down_scale_ctrl_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_CTRL_2ND).bitc.luma_fifo_full_thr;
}


/*!
  register ARIA_DISP_down_scale_out_size_2nd (read/write)
  */
void reg_aria_disp_set_down_scale_out_size_2nd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE_2ND, data);
}

mt_u32  reg_aria_disp_get_down_scale_out_size_2nd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE_2ND);
}

void reg_aria_disp_set_down_scale_out_size_2nd_down_scale_output_height(mt_u16 data)
{
    reg_aria_disp_down_scale_out_size_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE_2ND;
    d.bitc.down_scale_output_height = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE_2ND, d.all);
}

mt_u16  reg_aria_disp_get_down_scale_out_size_2nd_down_scale_output_height(void)
{
    return (*(volatile reg_aria_disp_down_scale_out_size_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE_2ND).bitc.down_scale_output_height;
}

void reg_aria_disp_set_down_scale_out_size_2nd_down_scale_output_width(mt_u16 data)
{
    reg_aria_disp_down_scale_out_size_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE_2ND;
    d.bitc.down_scale_output_width = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE_2ND, d.all);
}

mt_u16  reg_aria_disp_get_down_scale_out_size_2nd_down_scale_output_width(void)
{
    return (*(volatile reg_aria_disp_down_scale_out_size_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_OUT_SIZE_2ND).bitc.down_scale_output_width;
}


/*!
  register ARIA_DISP_down_scale_wr_stride_2nd (read/write)
  */
void reg_aria_disp_set_down_scale_wr_stride_2nd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_WR_STRIDE_2ND, data);
}

mt_u32  reg_aria_disp_get_down_scale_wr_stride_2nd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_WR_STRIDE_2ND);
}

void reg_aria_disp_set_down_scale_wr_stride_2nd_down_scale_wr_stride(mt_u16 data)
{
    reg_aria_disp_down_scale_wr_stride_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_WR_STRIDE_2ND;
    d.bitc.down_scale_wr_stride = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_WR_STRIDE_2ND, d.all);
}

mt_u16  reg_aria_disp_get_down_scale_wr_stride_2nd_down_scale_wr_stride(void)
{
    return (*(volatile reg_aria_disp_down_scale_wr_stride_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_WR_STRIDE_2ND).bitc.down_scale_wr_stride;
}


/*!
  register ARIA_DISP_down_scale_luma_wr_addr_0_2nd (read/write)
  */
void reg_aria_disp_set_down_scale_luma_wr_addr_0_2nd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_0_2ND, data);
}

mt_u32  reg_aria_disp_get_down_scale_luma_wr_addr_0_2nd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_0_2ND);
}

void reg_aria_disp_set_down_scale_luma_wr_addr_0_2nd_down_scale_luma_wr_addr_0(mt_u32 data)
{
    reg_aria_disp_down_scale_luma_wr_addr_0_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_0_2ND;
    d.bitc.down_scale_luma_wr_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_0_2ND, d.all);
}

mt_u32  reg_aria_disp_get_down_scale_luma_wr_addr_0_2nd_down_scale_luma_wr_addr_0(void)
{
    return (*(volatile reg_aria_disp_down_scale_luma_wr_addr_0_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_0_2ND).bitc.down_scale_luma_wr_addr_0;
}


/*!
  register ARIA_DISP_down_scale_cbcr_wr_addr_0_2nd (read/write)
  */
void reg_aria_disp_set_down_scale_cbcr_wr_addr_0_2nd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_0_2ND, data);
}

mt_u32  reg_aria_disp_get_down_scale_cbcr_wr_addr_0_2nd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_0_2ND);
}

void reg_aria_disp_set_down_scale_cbcr_wr_addr_0_2nd_down_scale_cbcr_wr_addr_0(mt_u32 data)
{
    reg_aria_disp_down_scale_cbcr_wr_addr_0_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_0_2ND;
    d.bitc.down_scale_cbcr_wr_addr_0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_0_2ND, d.all);
}

mt_u32  reg_aria_disp_get_down_scale_cbcr_wr_addr_0_2nd_down_scale_cbcr_wr_addr_0(void)
{
    return (*(volatile reg_aria_disp_down_scale_cbcr_wr_addr_0_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_0_2ND).bitc.down_scale_cbcr_wr_addr_0;
}


/*!
  register ARIA_DISP_down_scale_luma_wr_addr_2_2nd (read/write)
  */
void reg_aria_disp_set_down_scale_luma_wr_addr_2_2nd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_2_2ND, data);
}

mt_u32  reg_aria_disp_get_down_scale_luma_wr_addr_2_2nd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_2_2ND);
}

void reg_aria_disp_set_down_scale_luma_wr_addr_2_2nd_down_scale_luma_wr_addr_2(mt_u32 data)
{
    reg_aria_disp_down_scale_luma_wr_addr_2_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_2_2ND;
    d.bitc.down_scale_luma_wr_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_2_2ND, d.all);
}

mt_u32  reg_aria_disp_get_down_scale_luma_wr_addr_2_2nd_down_scale_luma_wr_addr_2(void)
{
    return (*(volatile reg_aria_disp_down_scale_luma_wr_addr_2_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_LUMA_WR_ADDR_2_2ND).bitc.down_scale_luma_wr_addr_2;
}


/*!
  register ARIA_DISP_down_scale_cbcr_wr_addr_2_2nd (read/write)
  */
void reg_aria_disp_set_down_scale_cbcr_wr_addr_2_2nd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_2_2ND, data);
}

mt_u32  reg_aria_disp_get_down_scale_cbcr_wr_addr_2_2nd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_2_2ND);
}

void reg_aria_disp_set_down_scale_cbcr_wr_addr_2_2nd_down_scale_cbcr_wr_addr_2(mt_u32 data)
{
    reg_aria_disp_down_scale_cbcr_wr_addr_2_2nd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_2_2ND;
    d.bitc.down_scale_cbcr_wr_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_2_2ND, d.all);
}

mt_u32  reg_aria_disp_get_down_scale_cbcr_wr_addr_2_2nd_down_scale_cbcr_wr_addr_2(void)
{
    return (*(volatile reg_aria_disp_down_scale_cbcr_wr_addr_2_2nd_t *)REG_ARIA_DISP_DOWN_SCALE_CBCR_WR_ADDR_2_2ND).bitc.down_scale_cbcr_wr_addr_2;
}


/*!
  register ARIA_DISP_down_scale_data_endian_ctrl (read/write)
  */
void reg_aria_disp_set_down_scale_data_endian_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL, data);
}

mt_u32  reg_aria_disp_get_down_scale_data_endian_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL);
}

void reg_aria_disp_set_down_scale_data_endian_ctrl_wr_data_endian_ctrl_2nd(mt_u8 data)
{
    reg_aria_disp_down_scale_data_endian_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL;
    d.bitc.wr_data_endian_ctrl_2nd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_data_endian_ctrl_wr_data_endian_ctrl_2nd(void)
{
    return (*(volatile reg_aria_disp_down_scale_data_endian_ctrl_t *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL).bitc.wr_data_endian_ctrl_2nd;
}

void reg_aria_disp_set_down_scale_data_endian_ctrl_cbcr_swap_2nd(mt_u8 data)
{
    reg_aria_disp_down_scale_data_endian_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL;
    d.bitc.cbcr_swap_2nd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_data_endian_ctrl_cbcr_swap_2nd(void)
{
    return (*(volatile reg_aria_disp_down_scale_data_endian_ctrl_t *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL).bitc.cbcr_swap_2nd;
}

void reg_aria_disp_set_down_scale_data_endian_ctrl_wr_data_endian_ctrl(mt_u8 data)
{
    reg_aria_disp_down_scale_data_endian_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL;
    d.bitc.wr_data_endian_ctrl = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_data_endian_ctrl_wr_data_endian_ctrl(void)
{
    return (*(volatile reg_aria_disp_down_scale_data_endian_ctrl_t *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL).bitc.wr_data_endian_ctrl;
}

void reg_aria_disp_set_down_scale_data_endian_ctrl_cbcr_swap(mt_u8 data)
{
    reg_aria_disp_down_scale_data_endian_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL;
    d.bitc.cbcr_swap = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_down_scale_data_endian_ctrl_cbcr_swap(void)
{
    return (*(volatile reg_aria_disp_down_scale_data_endian_ctrl_t *)REG_ARIA_DISP_DOWN_SCALE_DATA_ENDIAN_CTRL).bitc.cbcr_swap;
}


/*!
  register ARIA_DISP_hd_csc_ctrl (read/write)
  */
void reg_aria_disp_set_hd_csc_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_CTRL, data);
}

mt_u32  reg_aria_disp_get_hd_csc_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_CTRL);
}

void reg_aria_disp_set_hd_csc_ctrl_hd_csc_en(mt_u8 data)
{
    reg_aria_disp_hd_csc_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_CTRL;
    d.bitc.hd_csc_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_hd_csc_ctrl_hd_csc_en(void)
{
    return (*(volatile reg_aria_disp_hd_csc_ctrl_t *)REG_ARIA_DISP_HD_CSC_CTRL).bitc.hd_csc_en;
}

void reg_aria_disp_set_hd_csc_ctrl_hd_bound_output_en(mt_u8 data)
{
    reg_aria_disp_hd_csc_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_CTRL;
    d.bitc.hd_bound_output_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_hd_csc_ctrl_hd_bound_output_en(void)
{
    return (*(volatile reg_aria_disp_hd_csc_ctrl_t *)REG_ARIA_DISP_HD_CSC_CTRL).bitc.hd_bound_output_en;
}

void reg_aria_disp_set_hd_csc_ctrl_hd_bound_input_en(mt_u8 data)
{
    reg_aria_disp_hd_csc_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_CTRL;
    d.bitc.hd_bound_input_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_hd_csc_ctrl_hd_bound_input_en(void)
{
    return (*(volatile reg_aria_disp_hd_csc_ctrl_t *)REG_ARIA_DISP_HD_CSC_CTRL).bitc.hd_bound_input_en;
}


/*!
  register ARIA_DISP_hd_csc_coef_1 (read/write)
  */
void reg_aria_disp_set_hd_csc_coef_1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_1, data);
}

mt_u32  reg_aria_disp_get_hd_csc_coef_1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_1);
}

void reg_aria_disp_set_hd_csc_coef_1_csc_a01(mt_u16 data)
{
    reg_aria_disp_hd_csc_coef_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_1;
    d.bitc.csc_a01 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_1, d.all);
}

mt_u16  reg_aria_disp_get_hd_csc_coef_1_csc_a01(void)
{
    return (*(volatile reg_aria_disp_hd_csc_coef_1_t *)REG_ARIA_DISP_HD_CSC_COEF_1).bitc.csc_a01;
}

void reg_aria_disp_set_hd_csc_coef_1_csc_a00(mt_u16 data)
{
    reg_aria_disp_hd_csc_coef_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_1;
    d.bitc.csc_a00 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_1, d.all);
}

mt_u16  reg_aria_disp_get_hd_csc_coef_1_csc_a00(void)
{
    return (*(volatile reg_aria_disp_hd_csc_coef_1_t *)REG_ARIA_DISP_HD_CSC_COEF_1).bitc.csc_a00;
}


/*!
  register ARIA_DISP_hd_csc_coef_2 (read/write)
  */
void reg_aria_disp_set_hd_csc_coef_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_2, data);
}

mt_u32  reg_aria_disp_get_hd_csc_coef_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_2);
}

void reg_aria_disp_set_hd_csc_coef_2_csc_a10(mt_u16 data)
{
    reg_aria_disp_hd_csc_coef_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_2;
    d.bitc.csc_a10 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_2, d.all);
}

mt_u16  reg_aria_disp_get_hd_csc_coef_2_csc_a10(void)
{
    return (*(volatile reg_aria_disp_hd_csc_coef_2_t *)REG_ARIA_DISP_HD_CSC_COEF_2).bitc.csc_a10;
}

void reg_aria_disp_set_hd_csc_coef_2_csc_a02(mt_u16 data)
{
    reg_aria_disp_hd_csc_coef_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_2;
    d.bitc.csc_a02 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_2, d.all);
}

mt_u16  reg_aria_disp_get_hd_csc_coef_2_csc_a02(void)
{
    return (*(volatile reg_aria_disp_hd_csc_coef_2_t *)REG_ARIA_DISP_HD_CSC_COEF_2).bitc.csc_a02;
}


/*!
  register ARIA_DISP_hd_csc_coef_3 (read/write)
  */
void reg_aria_disp_set_hd_csc_coef_3(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_3, data);
}

mt_u32  reg_aria_disp_get_hd_csc_coef_3(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_3);
}

void reg_aria_disp_set_hd_csc_coef_3_csc_a12(mt_u16 data)
{
    reg_aria_disp_hd_csc_coef_3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_3;
    d.bitc.csc_a12 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_3, d.all);
}

mt_u16  reg_aria_disp_get_hd_csc_coef_3_csc_a12(void)
{
    return (*(volatile reg_aria_disp_hd_csc_coef_3_t *)REG_ARIA_DISP_HD_CSC_COEF_3).bitc.csc_a12;
}

void reg_aria_disp_set_hd_csc_coef_3_csc_a11(mt_u16 data)
{
    reg_aria_disp_hd_csc_coef_3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_3;
    d.bitc.csc_a11 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_3, d.all);
}

mt_u16  reg_aria_disp_get_hd_csc_coef_3_csc_a11(void)
{
    return (*(volatile reg_aria_disp_hd_csc_coef_3_t *)REG_ARIA_DISP_HD_CSC_COEF_3).bitc.csc_a11;
}


/*!
  register ARIA_DISP_hd_csc_coef_4 (read/write)
  */
void reg_aria_disp_set_hd_csc_coef_4(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_4, data);
}

mt_u32  reg_aria_disp_get_hd_csc_coef_4(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_4);
}

void reg_aria_disp_set_hd_csc_coef_4_csc_a21(mt_u16 data)
{
    reg_aria_disp_hd_csc_coef_4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_4;
    d.bitc.csc_a21 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_4, d.all);
}

mt_u16  reg_aria_disp_get_hd_csc_coef_4_csc_a21(void)
{
    return (*(volatile reg_aria_disp_hd_csc_coef_4_t *)REG_ARIA_DISP_HD_CSC_COEF_4).bitc.csc_a21;
}

void reg_aria_disp_set_hd_csc_coef_4_csc_a20(mt_u16 data)
{
    reg_aria_disp_hd_csc_coef_4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_4;
    d.bitc.csc_a20 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_4, d.all);
}

mt_u16  reg_aria_disp_get_hd_csc_coef_4_csc_a20(void)
{
    return (*(volatile reg_aria_disp_hd_csc_coef_4_t *)REG_ARIA_DISP_HD_CSC_COEF_4).bitc.csc_a20;
}


/*!
  register ARIA_DISP_hd_csc_coef_5 (read/write)
  */
void reg_aria_disp_set_hd_csc_coef_5(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_5, data);
}

mt_u32  reg_aria_disp_get_hd_csc_coef_5(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_5);
}

void reg_aria_disp_set_hd_csc_coef_5_csc_a22(mt_u16 data)
{
    reg_aria_disp_hd_csc_coef_5_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_CSC_COEF_5;
    d.bitc.csc_a22 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_CSC_COEF_5, d.all);
}

mt_u16  reg_aria_disp_get_hd_csc_coef_5_csc_a22(void)
{
    return (*(volatile reg_aria_disp_hd_csc_coef_5_t *)REG_ARIA_DISP_HD_CSC_COEF_5).bitc.csc_a22;
}


/*!
  register ARIA_DISP_tile_para (read/write)
  */
void reg_aria_disp_set_tile_para(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_TILE_PARA, data);
}

mt_u32  reg_aria_disp_get_tile_para(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_TILE_PARA);
}

void reg_aria_disp_set_tile_para_col_size_mode(mt_u8 data)
{
    reg_aria_disp_tile_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_TILE_PARA;
    d.bitc.col_size_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_TILE_PARA, d.all);
}

mt_u8   reg_aria_disp_get_tile_para_col_size_mode(void)
{
    return (*(volatile reg_aria_disp_tile_para_t *)REG_ARIA_DISP_TILE_PARA).bitc.col_size_mode;
}

void reg_aria_disp_set_tile_para_field_picture(mt_u8 data)
{
    reg_aria_disp_tile_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_TILE_PARA;
    d.bitc.field_picture = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_TILE_PARA, d.all);
}

mt_u8   reg_aria_disp_get_tile_para_field_picture(void)
{
    return (*(volatile reg_aria_disp_tile_para_t *)REG_ARIA_DISP_TILE_PARA).bitc.field_picture;
}

void reg_aria_disp_set_tile_para_hd_map_mode(mt_u8 data)
{
    reg_aria_disp_tile_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_TILE_PARA;
    d.bitc.hd_map_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_TILE_PARA, d.all);
}

mt_u8   reg_aria_disp_get_tile_para_hd_map_mode(void)
{
    return (*(volatile reg_aria_disp_tile_para_t *)REG_ARIA_DISP_TILE_PARA).bitc.hd_map_mode;
}

void reg_aria_disp_set_tile_para_tile_config(mt_u8 data)
{
    reg_aria_disp_tile_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_TILE_PARA;
    d.bitc.tile_config = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_TILE_PARA, d.all);
}

mt_u8   reg_aria_disp_get_tile_para_tile_config(void)
{
    return (*(volatile reg_aria_disp_tile_para_t *)REG_ARIA_DISP_TILE_PARA).bitc.tile_config;
}


/*!
  register ARIA_DISP_tile_rowjump_00 (read/write)
  */
void reg_aria_disp_set_tile_rowjump_00(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_TILE_ROWJUMP_00, data);
}

mt_u32  reg_aria_disp_get_tile_rowjump_00(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_TILE_ROWJUMP_00);
}

void reg_aria_disp_set_tile_rowjump_00_tile_rowjump_00(mt_u32 data)
{
    reg_aria_disp_tile_rowjump_00_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_TILE_ROWJUMP_00;
    d.bitc.tile_rowjump_00 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_TILE_ROWJUMP_00, d.all);
}

mt_u32  reg_aria_disp_get_tile_rowjump_00_tile_rowjump_00(void)
{
    return (*(volatile reg_aria_disp_tile_rowjump_00_t *)REG_ARIA_DISP_TILE_ROWJUMP_00).bitc.tile_rowjump_00;
}


/*!
  register ARIA_DISP_tile_rowjump_01 (read/write)
  */
void reg_aria_disp_set_tile_rowjump_01(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_TILE_ROWJUMP_01, data);
}

mt_u32  reg_aria_disp_get_tile_rowjump_01(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_TILE_ROWJUMP_01);
}

void reg_aria_disp_set_tile_rowjump_01_tile_rowjump_01(mt_u32 data)
{
    reg_aria_disp_tile_rowjump_01_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_TILE_ROWJUMP_01;
    d.bitc.tile_rowjump_01 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_TILE_ROWJUMP_01, d.all);
}

mt_u32  reg_aria_disp_get_tile_rowjump_01_tile_rowjump_01(void)
{
    return (*(volatile reg_aria_disp_tile_rowjump_01_t *)REG_ARIA_DISP_TILE_ROWJUMP_01).bitc.tile_rowjump_01;
}


/*!
  register ARIA_DISP_tile_rowjump_10 (read/write)
  */
void reg_aria_disp_set_tile_rowjump_10(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_TILE_ROWJUMP_10, data);
}

mt_u32  reg_aria_disp_get_tile_rowjump_10(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_TILE_ROWJUMP_10);
}

void reg_aria_disp_set_tile_rowjump_10_tile_rowjump_10(mt_u32 data)
{
    reg_aria_disp_tile_rowjump_10_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_TILE_ROWJUMP_10;
    d.bitc.tile_rowjump_10 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_TILE_ROWJUMP_10, d.all);
}

mt_u32  reg_aria_disp_get_tile_rowjump_10_tile_rowjump_10(void)
{
    return (*(volatile reg_aria_disp_tile_rowjump_10_t *)REG_ARIA_DISP_TILE_ROWJUMP_10).bitc.tile_rowjump_10;
}


/*!
  register ARIA_DISP_tile_rowjump_11 (read/write)
  */
void reg_aria_disp_set_tile_rowjump_11(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_TILE_ROWJUMP_11, data);
}

mt_u32  reg_aria_disp_get_tile_rowjump_11(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_TILE_ROWJUMP_11);
}

void reg_aria_disp_set_tile_rowjump_11_tile_rowjump_11(mt_u32 data)
{
    reg_aria_disp_tile_rowjump_11_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_TILE_ROWJUMP_11;
    d.bitc.tile_rowjump_11 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_TILE_ROWJUMP_11, d.all);
}

mt_u32  reg_aria_disp_get_tile_rowjump_11_tile_rowjump_11(void)
{
    return (*(volatile reg_aria_disp_tile_rowjump_11_t *)REG_ARIA_DISP_TILE_ROWJUMP_11).bitc.tile_rowjump_11;
}


/*!
  register ARIA_DISP_denoise_ctrl (read/write)
  */
void reg_aria_disp_set_denoise_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DENOISE_CTRL, data);
}

mt_u32  reg_aria_disp_get_denoise_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DENOISE_CTRL);
}

void reg_aria_disp_set_denoise_ctrl_denoise_en(mt_u8 data)
{
    reg_aria_disp_denoise_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DENOISE_CTRL;
    d.bitc.denoise_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DENOISE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_denoise_ctrl_denoise_en(void)
{
    return (*(volatile reg_aria_disp_denoise_ctrl_t *)REG_ARIA_DISP_DENOISE_CTRL).bitc.denoise_en;
}

void reg_aria_disp_set_denoise_ctrl_denoise_thrd(mt_u8 data)
{
    reg_aria_disp_denoise_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DENOISE_CTRL;
    d.bitc.denoise_thrd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DENOISE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_denoise_ctrl_denoise_thrd(void)
{
    return (*(volatile reg_aria_disp_denoise_ctrl_t *)REG_ARIA_DISP_DENOISE_CTRL).bitc.denoise_thrd;
}

void reg_aria_disp_set_denoise_ctrl_denoise_thrg(mt_u8 data)
{
    reg_aria_disp_denoise_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DENOISE_CTRL;
    d.bitc.denoise_thrg = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DENOISE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_denoise_ctrl_denoise_thrg(void)
{
    return (*(volatile reg_aria_disp_denoise_ctrl_t *)REG_ARIA_DISP_DENOISE_CTRL).bitc.denoise_thrg;
}


/*!
  register ARIA_DISP_denoise_para_1 (read/write)
  */
void reg_aria_disp_set_denoise_para_1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DENOISE_PARA_1, data);
}

mt_u32  reg_aria_disp_get_denoise_para_1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DENOISE_PARA_1);
}

void reg_aria_disp_set_denoise_para_1_denoise_a2(mt_u16 data)
{
    reg_aria_disp_denoise_para_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DENOISE_PARA_1;
    d.bitc.denoise_a2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DENOISE_PARA_1, d.all);
}

mt_u16  reg_aria_disp_get_denoise_para_1_denoise_a2(void)
{
    return (*(volatile reg_aria_disp_denoise_para_1_t *)REG_ARIA_DISP_DENOISE_PARA_1).bitc.denoise_a2;
}

void reg_aria_disp_set_denoise_para_1_denoise_a1(mt_u16 data)
{
    reg_aria_disp_denoise_para_1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DENOISE_PARA_1;
    d.bitc.denoise_a1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DENOISE_PARA_1, d.all);
}

mt_u16  reg_aria_disp_get_denoise_para_1_denoise_a1(void)
{
    return (*(volatile reg_aria_disp_denoise_para_1_t *)REG_ARIA_DISP_DENOISE_PARA_1).bitc.denoise_a1;
}


/*!
  register ARIA_DISP_denoise_para_2 (read/write)
  */
void reg_aria_disp_set_denoise_para_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DENOISE_PARA_2, data);
}

mt_u32  reg_aria_disp_get_denoise_para_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DENOISE_PARA_2);
}

void reg_aria_disp_set_denoise_para_2_denoise_b1(mt_u16 data)
{
    reg_aria_disp_denoise_para_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DENOISE_PARA_2;
    d.bitc.denoise_b1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DENOISE_PARA_2, d.all);
}

mt_u16  reg_aria_disp_get_denoise_para_2_denoise_b1(void)
{
    return (*(volatile reg_aria_disp_denoise_para_2_t *)REG_ARIA_DISP_DENOISE_PARA_2).bitc.denoise_b1;
}

void reg_aria_disp_set_denoise_para_2_denoise_a3(mt_u16 data)
{
    reg_aria_disp_denoise_para_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DENOISE_PARA_2;
    d.bitc.denoise_a3 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DENOISE_PARA_2, d.all);
}

mt_u16  reg_aria_disp_get_denoise_para_2_denoise_a3(void)
{
    return (*(volatile reg_aria_disp_denoise_para_2_t *)REG_ARIA_DISP_DENOISE_PARA_2).bitc.denoise_a3;
}


/*!
  register ARIA_DISP_denoise_para_3 (read/write)
  */
void reg_aria_disp_set_denoise_para_3(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DENOISE_PARA_3, data);
}

mt_u32  reg_aria_disp_get_denoise_para_3(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DENOISE_PARA_3);
}

void reg_aria_disp_set_denoise_para_3_denoise_b3(mt_u16 data)
{
    reg_aria_disp_denoise_para_3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DENOISE_PARA_3;
    d.bitc.denoise_b3 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DENOISE_PARA_3, d.all);
}

mt_u16  reg_aria_disp_get_denoise_para_3_denoise_b3(void)
{
    return (*(volatile reg_aria_disp_denoise_para_3_t *)REG_ARIA_DISP_DENOISE_PARA_3).bitc.denoise_b3;
}

void reg_aria_disp_set_denoise_para_3_denoise_b2(mt_u16 data)
{
    reg_aria_disp_denoise_para_3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DENOISE_PARA_3;
    d.bitc.denoise_b2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DENOISE_PARA_3, d.all);
}

mt_u16  reg_aria_disp_get_denoise_para_3_denoise_b2(void)
{
    return (*(volatile reg_aria_disp_denoise_para_3_t *)REG_ARIA_DISP_DENOISE_PARA_3).bitc.denoise_b2;
}


/*!
  register ARIA_DISP_background_color (read/write)
  */
void reg_aria_disp_set_background_color(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_BACKGROUND_COLOR, data);
}

mt_u32  reg_aria_disp_get_background_color(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_BACKGROUND_COLOR);
}

void reg_aria_disp_set_background_color_background_cr(mt_u8 data)
{
    reg_aria_disp_background_color_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_BACKGROUND_COLOR;
    d.bitc.background_cr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_BACKGROUND_COLOR, d.all);
}

mt_u8   reg_aria_disp_get_background_color_background_cr(void)
{
    return (*(volatile reg_aria_disp_background_color_t *)REG_ARIA_DISP_BACKGROUND_COLOR).bitc.background_cr;
}

void reg_aria_disp_set_background_color_background_cb(mt_u8 data)
{
    reg_aria_disp_background_color_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_BACKGROUND_COLOR;
    d.bitc.background_cb = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_BACKGROUND_COLOR, d.all);
}

mt_u8   reg_aria_disp_get_background_color_background_cb(void)
{
    return (*(volatile reg_aria_disp_background_color_t *)REG_ARIA_DISP_BACKGROUND_COLOR).bitc.background_cb;
}

void reg_aria_disp_set_background_color_background_luma(mt_u8 data)
{
    reg_aria_disp_background_color_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_BACKGROUND_COLOR;
    d.bitc.background_luma = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_BACKGROUND_COLOR, d.all);
}

mt_u8   reg_aria_disp_get_background_color_background_luma(void)
{
    return (*(volatile reg_aria_disp_background_color_t *)REG_ARIA_DISP_BACKGROUND_COLOR).bitc.background_luma;
}

void reg_aria_disp_set_background_color_background_sel(mt_u8 data)
{
    reg_aria_disp_background_color_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_BACKGROUND_COLOR;
    d.bitc.background_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_BACKGROUND_COLOR, d.all);
}

mt_u8   reg_aria_disp_get_background_color_background_sel(void)
{
    return (*(volatile reg_aria_disp_background_color_t *)REG_ARIA_DISP_BACKGROUND_COLOR).bitc.background_sel;
}


/*!
  register ARIA_DISP_hdenc_test_cmd (read/write)
  */
void reg_aria_disp_set_hdenc_test_cmd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HDENC_TEST_CMD, data);
}

mt_u32  reg_aria_disp_get_hdenc_test_cmd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HDENC_TEST_CMD);
}

void reg_aria_disp_set_hdenc_test_cmd_test_en(mt_u8 data)
{
    reg_aria_disp_hdenc_test_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HDENC_TEST_CMD;
    d.bitc.test_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HDENC_TEST_CMD, d.all);
}

mt_u8   reg_aria_disp_get_hdenc_test_cmd_test_en(void)
{
    return (*(volatile reg_aria_disp_hdenc_test_cmd_t *)REG_ARIA_DISP_HDENC_TEST_CMD).bitc.test_en;
}

void reg_aria_disp_set_hdenc_test_cmd_wr_data_en(mt_u8 data)
{
    reg_aria_disp_hdenc_test_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HDENC_TEST_CMD;
    d.bitc.wr_data_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HDENC_TEST_CMD, d.all);
}

mt_u8   reg_aria_disp_get_hdenc_test_cmd_wr_data_en(void)
{
    return (*(volatile reg_aria_disp_hdenc_test_cmd_t *)REG_ARIA_DISP_HDENC_TEST_CMD).bitc.wr_data_en;
}

void reg_aria_disp_set_hdenc_test_cmd_test_length_minus_1(mt_u16 data)
{
    reg_aria_disp_hdenc_test_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HDENC_TEST_CMD;
    d.bitc.test_length_minus_1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HDENC_TEST_CMD, d.all);
}

mt_u16  reg_aria_disp_get_hdenc_test_cmd_test_length_minus_1(void)
{
    return (*(volatile reg_aria_disp_hdenc_test_cmd_t *)REG_ARIA_DISP_HDENC_TEST_CMD).bitc.test_length_minus_1;
}


/*!
  register ARIA_DISP_hdenc_test_data (read/write)
  */
void reg_aria_disp_set_hdenc_test_data(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HDENC_TEST_DATA, data);
}

mt_u32  reg_aria_disp_get_hdenc_test_data(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HDENC_TEST_DATA);
}

void reg_aria_disp_set_hdenc_test_data_test_data(mt_u32 data)
{
    reg_aria_disp_hdenc_test_data_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HDENC_TEST_DATA;
    d.bitc.test_data = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HDENC_TEST_DATA, d.all);
}

mt_u32  reg_aria_disp_get_hdenc_test_data_test_data(void)
{
    return (*(volatile reg_aria_disp_hdenc_test_data_t *)REG_ARIA_DISP_HDENC_TEST_DATA).bitc.test_data;
}


/*!
  register ARIA_DISP_video_axi_monitor_clr (read/write)
  */
void reg_aria_disp_set_video_axi_monitor_clr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR, data);
}

mt_u32  reg_aria_disp_get_video_axi_monitor_clr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR);
}

void reg_aria_disp_set_video_axi_monitor_clr_video_read_cmd_lantency_cnt_clr(mt_u8 data)
{
    reg_aria_disp_video_axi_monitor_clr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR;
    d.bitc.video_read_cmd_lantency_cnt_clr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR, d.all);
}

mt_u8   reg_aria_disp_get_video_axi_monitor_clr_video_read_cmd_lantency_cnt_clr(void)
{
    return (*(volatile reg_aria_disp_video_axi_monitor_clr_t *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR).bitc.video_read_cmd_lantency_cnt_clr;
}

void reg_aria_disp_set_video_axi_monitor_clr_video_read_data_lantency_cnt_clr(mt_u8 data)
{
    reg_aria_disp_video_axi_monitor_clr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR;
    d.bitc.video_read_data_lantency_cnt_clr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR, d.all);
}

mt_u8   reg_aria_disp_get_video_axi_monitor_clr_video_read_data_lantency_cnt_clr(void)
{
    return (*(volatile reg_aria_disp_video_axi_monitor_clr_t *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR).bitc.video_read_data_lantency_cnt_clr;
}

void reg_aria_disp_set_video_axi_monitor_clr_motion_write_cmd_lantency_cnt_clr(mt_u8 data)
{
    reg_aria_disp_video_axi_monitor_clr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR;
    d.bitc.motion_write_cmd_lantency_cnt_clr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR, d.all);
}

mt_u8   reg_aria_disp_get_video_axi_monitor_clr_motion_write_cmd_lantency_cnt_clr(void)
{
    return (*(volatile reg_aria_disp_video_axi_monitor_clr_t *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR).bitc.motion_write_cmd_lantency_cnt_clr;
}

void reg_aria_disp_set_video_axi_monitor_clr_motion_write_data_lantency_cnt_clr(mt_u8 data)
{
    reg_aria_disp_video_axi_monitor_clr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR;
    d.bitc.motion_write_data_lantency_cnt_clr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR, d.all);
}

mt_u8   reg_aria_disp_get_video_axi_monitor_clr_motion_write_data_lantency_cnt_clr(void)
{
    return (*(volatile reg_aria_disp_video_axi_monitor_clr_t *)REG_ARIA_DISP_VIDEO_AXI_MONITOR_CLR).bitc.motion_write_data_lantency_cnt_clr;
}


/*!
  register ARIA_DISP_video_read_cmd_latency_cnt_max (read/write)
  */
void reg_aria_disp_set_video_read_cmd_latency_cnt_max(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_READ_CMD_LATENCY_CNT_MAX, data);
}

mt_u32  reg_aria_disp_get_video_read_cmd_latency_cnt_max(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_READ_CMD_LATENCY_CNT_MAX);
}

void reg_aria_disp_set_video_read_cmd_latency_cnt_max_video_read_cmd_latency_cnt_max(mt_u32 data)
{
    reg_aria_disp_video_read_cmd_latency_cnt_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_READ_CMD_LATENCY_CNT_MAX;
    d.bitc.video_read_cmd_latency_cnt_max = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_READ_CMD_LATENCY_CNT_MAX, d.all);
}

mt_u32  reg_aria_disp_get_video_read_cmd_latency_cnt_max_video_read_cmd_latency_cnt_max(void)
{
    return (*(volatile reg_aria_disp_video_read_cmd_latency_cnt_max_t *)REG_ARIA_DISP_VIDEO_READ_CMD_LATENCY_CNT_MAX).bitc.video_read_cmd_latency_cnt_max;
}


/*!
  register ARIA_DISP_video_read_cmd_latency_cnt_sum (read/write)
  */
void reg_aria_disp_set_video_read_cmd_latency_cnt_sum(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_READ_CMD_LATENCY_CNT_SUM, data);
}

mt_u32  reg_aria_disp_get_video_read_cmd_latency_cnt_sum(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_READ_CMD_LATENCY_CNT_SUM);
}

void reg_aria_disp_set_video_read_cmd_latency_cnt_sum_video_read_cmd_latency_cnt_sum(mt_u32 data)
{
    reg_aria_disp_video_read_cmd_latency_cnt_sum_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_READ_CMD_LATENCY_CNT_SUM;
    d.bitc.video_read_cmd_latency_cnt_sum = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_READ_CMD_LATENCY_CNT_SUM, d.all);
}

mt_u32  reg_aria_disp_get_video_read_cmd_latency_cnt_sum_video_read_cmd_latency_cnt_sum(void)
{
    return (*(volatile reg_aria_disp_video_read_cmd_latency_cnt_sum_t *)REG_ARIA_DISP_VIDEO_READ_CMD_LATENCY_CNT_SUM).bitc.video_read_cmd_latency_cnt_sum;
}


/*!
  register ARIA_DISP_video_read_cmd_req_cnt_sum (read/write)
  */
void reg_aria_disp_set_video_read_cmd_req_cnt_sum(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_READ_CMD_REQ_CNT_SUM, data);
}

mt_u32  reg_aria_disp_get_video_read_cmd_req_cnt_sum(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_READ_CMD_REQ_CNT_SUM);
}

void reg_aria_disp_set_video_read_cmd_req_cnt_sum_video_read_cmd_req_cnt_sum(mt_u32 data)
{
    reg_aria_disp_video_read_cmd_req_cnt_sum_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_READ_CMD_REQ_CNT_SUM;
    d.bitc.video_read_cmd_req_cnt_sum = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_READ_CMD_REQ_CNT_SUM, d.all);
}

mt_u32  reg_aria_disp_get_video_read_cmd_req_cnt_sum_video_read_cmd_req_cnt_sum(void)
{
    return (*(volatile reg_aria_disp_video_read_cmd_req_cnt_sum_t *)REG_ARIA_DISP_VIDEO_READ_CMD_REQ_CNT_SUM).bitc.video_read_cmd_req_cnt_sum;
}


/*!
  register ARIA_DISP_video_read_data_latency_cnt_max (read/write)
  */
void reg_aria_disp_set_video_read_data_latency_cnt_max(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_READ_DATA_LATENCY_CNT_MAX, data);
}

mt_u32  reg_aria_disp_get_video_read_data_latency_cnt_max(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_READ_DATA_LATENCY_CNT_MAX);
}

void reg_aria_disp_set_video_read_data_latency_cnt_max_video_read_data_latency_cnt_max(mt_u32 data)
{
    reg_aria_disp_video_read_data_latency_cnt_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_READ_DATA_LATENCY_CNT_MAX;
    d.bitc.video_read_data_latency_cnt_max = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_READ_DATA_LATENCY_CNT_MAX, d.all);
}

mt_u32  reg_aria_disp_get_video_read_data_latency_cnt_max_video_read_data_latency_cnt_max(void)
{
    return (*(volatile reg_aria_disp_video_read_data_latency_cnt_max_t *)REG_ARIA_DISP_VIDEO_READ_DATA_LATENCY_CNT_MAX).bitc.video_read_data_latency_cnt_max;
}


/*!
  register ARIA_DISP_video_read_data_latency_cnt_sum (read/write)
  */
void reg_aria_disp_set_video_read_data_latency_cnt_sum(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_READ_DATA_LATENCY_CNT_SUM, data);
}

mt_u32  reg_aria_disp_get_video_read_data_latency_cnt_sum(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_READ_DATA_LATENCY_CNT_SUM);
}

void reg_aria_disp_set_video_read_data_latency_cnt_sum_video_read_data_latency_cnt_sum(mt_u32 data)
{
    reg_aria_disp_video_read_data_latency_cnt_sum_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_READ_DATA_LATENCY_CNT_SUM;
    d.bitc.video_read_data_latency_cnt_sum = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_READ_DATA_LATENCY_CNT_SUM, d.all);
}

mt_u32  reg_aria_disp_get_video_read_data_latency_cnt_sum_video_read_data_latency_cnt_sum(void)
{
    return (*(volatile reg_aria_disp_video_read_data_latency_cnt_sum_t *)REG_ARIA_DISP_VIDEO_READ_DATA_LATENCY_CNT_SUM).bitc.video_read_data_latency_cnt_sum;
}


/*!
  register ARIA_DISP_video_read_data_req_cnt_sum (read/write)
  */
void reg_aria_disp_set_video_read_data_req_cnt_sum(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_READ_DATA_REQ_CNT_SUM, data);
}

mt_u32  reg_aria_disp_get_video_read_data_req_cnt_sum(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_READ_DATA_REQ_CNT_SUM);
}

void reg_aria_disp_set_video_read_data_req_cnt_sum_video_read_data_req_cnt_sum(mt_u32 data)
{
    reg_aria_disp_video_read_data_req_cnt_sum_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_VIDEO_READ_DATA_REQ_CNT_SUM;
    d.bitc.video_read_data_req_cnt_sum = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_VIDEO_READ_DATA_REQ_CNT_SUM, d.all);
}

mt_u32  reg_aria_disp_get_video_read_data_req_cnt_sum_video_read_data_req_cnt_sum(void)
{
    return (*(volatile reg_aria_disp_video_read_data_req_cnt_sum_t *)REG_ARIA_DISP_VIDEO_READ_DATA_REQ_CNT_SUM).bitc.video_read_data_req_cnt_sum;
}


/*!
  register ARIA_DISP_motion_write_cmd_latency_cnt_max (read/write)
  */
void reg_aria_disp_set_motion_write_cmd_latency_cnt_max(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_WRITE_CMD_LATENCY_CNT_MAX, data);
}

mt_u32  reg_aria_disp_get_motion_write_cmd_latency_cnt_max(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_MOTION_WRITE_CMD_LATENCY_CNT_MAX);
}

void reg_aria_disp_set_motion_write_cmd_latency_cnt_max_motion_write_cmd_latency_cnt_max(mt_u32 data)
{
    reg_aria_disp_motion_write_cmd_latency_cnt_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_MOTION_WRITE_CMD_LATENCY_CNT_MAX;
    d.bitc.motion_write_cmd_latency_cnt_max = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_WRITE_CMD_LATENCY_CNT_MAX, d.all);
}

mt_u32  reg_aria_disp_get_motion_write_cmd_latency_cnt_max_motion_write_cmd_latency_cnt_max(void)
{
    return (*(volatile reg_aria_disp_motion_write_cmd_latency_cnt_max_t *)REG_ARIA_DISP_MOTION_WRITE_CMD_LATENCY_CNT_MAX).bitc.motion_write_cmd_latency_cnt_max;
}


/*!
  register ARIA_DISP_motion_write_cmd_latency_cnt_sum (read/write)
  */
void reg_aria_disp_set_motion_write_cmd_latency_cnt_sum(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_WRITE_CMD_LATENCY_CNT_SUM, data);
}

mt_u32  reg_aria_disp_get_motion_write_cmd_latency_cnt_sum(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_MOTION_WRITE_CMD_LATENCY_CNT_SUM);
}

void reg_aria_disp_set_motion_write_cmd_latency_cnt_sum_motion_write_cmd_latency_cnt_sum(mt_u32 data)
{
    reg_aria_disp_motion_write_cmd_latency_cnt_sum_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_MOTION_WRITE_CMD_LATENCY_CNT_SUM;
    d.bitc.motion_write_cmd_latency_cnt_sum = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_WRITE_CMD_LATENCY_CNT_SUM, d.all);
}

mt_u32  reg_aria_disp_get_motion_write_cmd_latency_cnt_sum_motion_write_cmd_latency_cnt_sum(void)
{
    return (*(volatile reg_aria_disp_motion_write_cmd_latency_cnt_sum_t *)REG_ARIA_DISP_MOTION_WRITE_CMD_LATENCY_CNT_SUM).bitc.motion_write_cmd_latency_cnt_sum;
}


/*!
  register ARIA_DISP_motion_write_cmd_req_cnt_sum (read/write)
  */
void reg_aria_disp_set_motion_write_cmd_req_cnt_sum(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_WRITE_CMD_REQ_CNT_SUM, data);
}

mt_u32  reg_aria_disp_get_motion_write_cmd_req_cnt_sum(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_MOTION_WRITE_CMD_REQ_CNT_SUM);
}

void reg_aria_disp_set_motion_write_cmd_req_cnt_sum_motion_write_cmd_req_cnt_sum(mt_u32 data)
{
    reg_aria_disp_motion_write_cmd_req_cnt_sum_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_MOTION_WRITE_CMD_REQ_CNT_SUM;
    d.bitc.motion_write_cmd_req_cnt_sum = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_WRITE_CMD_REQ_CNT_SUM, d.all);
}

mt_u32  reg_aria_disp_get_motion_write_cmd_req_cnt_sum_motion_write_cmd_req_cnt_sum(void)
{
    return (*(volatile reg_aria_disp_motion_write_cmd_req_cnt_sum_t *)REG_ARIA_DISP_MOTION_WRITE_CMD_REQ_CNT_SUM).bitc.motion_write_cmd_req_cnt_sum;
}


/*!
  register ARIA_DISP_motion_write_data_latency_cnt_max (read/write)
  */
void reg_aria_disp_set_motion_write_data_latency_cnt_max(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_WRITE_DATA_LATENCY_CNT_MAX, data);
}

mt_u32  reg_aria_disp_get_motion_write_data_latency_cnt_max(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_MOTION_WRITE_DATA_LATENCY_CNT_MAX);
}

void reg_aria_disp_set_motion_write_data_latency_cnt_max_motion_write_data_latency_cnt_max(mt_u32 data)
{
    reg_aria_disp_motion_write_data_latency_cnt_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_MOTION_WRITE_DATA_LATENCY_CNT_MAX;
    d.bitc.motion_write_data_latency_cnt_max = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_WRITE_DATA_LATENCY_CNT_MAX, d.all);
}

mt_u32  reg_aria_disp_get_motion_write_data_latency_cnt_max_motion_write_data_latency_cnt_max(void)
{
    return (*(volatile reg_aria_disp_motion_write_data_latency_cnt_max_t *)REG_ARIA_DISP_MOTION_WRITE_DATA_LATENCY_CNT_MAX).bitc.motion_write_data_latency_cnt_max;
}


/*!
  register ARIA_DISP_motion_write_data_latency_cnt_sum (read/write)
  */
void reg_aria_disp_set_motion_write_data_latency_cnt_sum(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_WRITE_DATA_LATENCY_CNT_SUM, data);
}

mt_u32  reg_aria_disp_get_motion_write_data_latency_cnt_sum(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_MOTION_WRITE_DATA_LATENCY_CNT_SUM);
}

void reg_aria_disp_set_motion_write_data_latency_cnt_sum_motion_write_data_latency_cnt_sum(mt_u32 data)
{
    reg_aria_disp_motion_write_data_latency_cnt_sum_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_MOTION_WRITE_DATA_LATENCY_CNT_SUM;
    d.bitc.motion_write_data_latency_cnt_sum = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_WRITE_DATA_LATENCY_CNT_SUM, d.all);
}

mt_u32  reg_aria_disp_get_motion_write_data_latency_cnt_sum_motion_write_data_latency_cnt_sum(void)
{
    return (*(volatile reg_aria_disp_motion_write_data_latency_cnt_sum_t *)REG_ARIA_DISP_MOTION_WRITE_DATA_LATENCY_CNT_SUM).bitc.motion_write_data_latency_cnt_sum;
}


/*!
  register ARIA_DISP_motion_write_data_req_cnt_sum (read/write)
  */
void reg_aria_disp_set_motion_write_data_req_cnt_sum(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_WRITE_DATA_REQ_CNT_SUM, data);
}

mt_u32  reg_aria_disp_get_motion_write_data_req_cnt_sum(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_MOTION_WRITE_DATA_REQ_CNT_SUM);
}

void reg_aria_disp_set_motion_write_data_req_cnt_sum_motion_write_data_req_cnt_sum(mt_u32 data)
{
    reg_aria_disp_motion_write_data_req_cnt_sum_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_MOTION_WRITE_DATA_REQ_CNT_SUM;
    d.bitc.motion_write_data_req_cnt_sum = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_MOTION_WRITE_DATA_REQ_CNT_SUM, d.all);
}

mt_u32  reg_aria_disp_get_motion_write_data_req_cnt_sum_motion_write_data_req_cnt_sum(void)
{
    return (*(volatile reg_aria_disp_motion_write_data_req_cnt_sum_t *)REG_ARIA_DISP_MOTION_WRITE_DATA_REQ_CNT_SUM).bitc.motion_write_data_req_cnt_sum;
}


/*!
  register ARIA_DISP_osdl_osd0_cmd (read/write)
  */
void reg_aria_disp_set_osdl_osd0_cmd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD0_CMD, data);
}

mt_u32  reg_aria_disp_get_osdl_osd0_cmd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD0_CMD);
}

void reg_aria_disp_set_osdl_osd0_cmd_osd_layer_en(mt_u8 data)
{
    reg_aria_disp_osdl_osd0_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD0_CMD;
    d.bitc.osd_layer_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD0_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd0_cmd_osd_layer_en(void)
{
    return (*(volatile reg_aria_disp_osdl_osd0_cmd_t *)REG_ARIA_DISP_OSDL_OSD0_CMD).bitc.osd_layer_en;
}

void reg_aria_disp_set_osdl_osd0_cmd_force_progressive_mode(mt_u8 data)
{
    reg_aria_disp_osdl_osd0_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD0_CMD;
    d.bitc.force_progressive_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD0_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd0_cmd_force_progressive_mode(void)
{
    return (*(volatile reg_aria_disp_osdl_osd0_cmd_t *)REG_ARIA_DISP_OSDL_OSD0_CMD).bitc.force_progressive_mode;
}

void reg_aria_disp_set_osdl_osd0_cmd_osd_plane_alpha_en(mt_u8 data)
{
    reg_aria_disp_osdl_osd0_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD0_CMD;
    d.bitc.osd_plane_alpha_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD0_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd0_cmd_osd_plane_alpha_en(void)
{
    return (*(volatile reg_aria_disp_osdl_osd0_cmd_t *)REG_ARIA_DISP_OSDL_OSD0_CMD).bitc.osd_plane_alpha_en;
}

void reg_aria_disp_set_osdl_osd0_cmd_osd0_reduce_framerate(mt_u8 data)
{
    reg_aria_disp_osdl_osd0_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD0_CMD;
    d.bitc.osd0_reduce_framerate = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD0_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd0_cmd_osd0_reduce_framerate(void)
{
    return (*(volatile reg_aria_disp_osdl_osd0_cmd_t *)REG_ARIA_DISP_OSDL_OSD0_CMD).bitc.osd0_reduce_framerate;
}

void reg_aria_disp_set_osdl_osd0_cmd_plane_alpha(mt_u8 data)
{
    reg_aria_disp_osdl_osd0_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD0_CMD;
    d.bitc.plane_alpha = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD0_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd0_cmd_plane_alpha(void)
{
    return (*(volatile reg_aria_disp_osdl_osd0_cmd_t *)REG_ARIA_DISP_OSDL_OSD0_CMD).bitc.plane_alpha;
}

void reg_aria_disp_set_osdl_osd0_cmd_weak_edge_enable(mt_u8 data)
{
    reg_aria_disp_osdl_osd0_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD0_CMD;
    d.bitc.weak_edge_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD0_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd0_cmd_weak_edge_enable(void)
{
    return (*(volatile reg_aria_disp_osdl_osd0_cmd_t *)REG_ARIA_DISP_OSDL_OSD0_CMD).bitc.weak_edge_enable;
}


/*!
  register ARIA_DISP_osdl_osd1_cmd (read/write)
  */
void reg_aria_disp_set_osdl_osd1_cmd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD1_CMD, data);
}

mt_u32  reg_aria_disp_get_osdl_osd1_cmd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD1_CMD);
}

void reg_aria_disp_set_osdl_osd1_cmd_osd_layer_en(mt_u8 data)
{
    reg_aria_disp_osdl_osd1_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD1_CMD;
    d.bitc.osd_layer_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD1_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd1_cmd_osd_layer_en(void)
{
    return (*(volatile reg_aria_disp_osdl_osd1_cmd_t *)REG_ARIA_DISP_OSDL_OSD1_CMD).bitc.osd_layer_en;
}

void reg_aria_disp_set_osdl_osd1_cmd_force_progressive_mode(mt_u8 data)
{
    reg_aria_disp_osdl_osd1_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD1_CMD;
    d.bitc.force_progressive_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD1_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd1_cmd_force_progressive_mode(void)
{
    return (*(volatile reg_aria_disp_osdl_osd1_cmd_t *)REG_ARIA_DISP_OSDL_OSD1_CMD).bitc.force_progressive_mode;
}

void reg_aria_disp_set_osdl_osd1_cmd_osd_plane_alpha_en(mt_u8 data)
{
    reg_aria_disp_osdl_osd1_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD1_CMD;
    d.bitc.osd_plane_alpha_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD1_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd1_cmd_osd_plane_alpha_en(void)
{
    return (*(volatile reg_aria_disp_osdl_osd1_cmd_t *)REG_ARIA_DISP_OSDL_OSD1_CMD).bitc.osd_plane_alpha_en;
}

void reg_aria_disp_set_osdl_osd1_cmd_osd1_reduce_framerate(mt_u8 data)
{
    reg_aria_disp_osdl_osd1_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD1_CMD;
    d.bitc.osd1_reduce_framerate = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD1_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd1_cmd_osd1_reduce_framerate(void)
{
    return (*(volatile reg_aria_disp_osdl_osd1_cmd_t *)REG_ARIA_DISP_OSDL_OSD1_CMD).bitc.osd1_reduce_framerate;
}

void reg_aria_disp_set_osdl_osd1_cmd_plane_alpha(mt_u8 data)
{
    reg_aria_disp_osdl_osd1_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD1_CMD;
    d.bitc.plane_alpha = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD1_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd1_cmd_plane_alpha(void)
{
    return (*(volatile reg_aria_disp_osdl_osd1_cmd_t *)REG_ARIA_DISP_OSDL_OSD1_CMD).bitc.plane_alpha;
}

void reg_aria_disp_set_osdl_osd1_cmd_weak_edge_enable(mt_u8 data)
{
    reg_aria_disp_osdl_osd1_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD1_CMD;
    d.bitc.weak_edge_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD1_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd1_cmd_weak_edge_enable(void)
{
    return (*(volatile reg_aria_disp_osdl_osd1_cmd_t *)REG_ARIA_DISP_OSDL_OSD1_CMD).bitc.weak_edge_enable;
}


/*!
  register ARIA_DISP_osdl_sub_cmd (read/write)
  */
void reg_aria_disp_set_osdl_sub_cmd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_SUB_CMD, data);
}

mt_u32  reg_aria_disp_get_osdl_sub_cmd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_SUB_CMD);
}

void reg_aria_disp_set_osdl_sub_cmd_osd_layer(mt_u8 data)
{
    reg_aria_disp_osdl_sub_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_SUB_CMD;
    d.bitc.osd_layer = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_SUB_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_sub_cmd_osd_layer(void)
{
    return (*(volatile reg_aria_disp_osdl_sub_cmd_t *)REG_ARIA_DISP_OSDL_SUB_CMD).bitc.osd_layer;
}

void reg_aria_disp_set_osdl_sub_cmd_force_progressive_mode(mt_u8 data)
{
    reg_aria_disp_osdl_sub_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_SUB_CMD;
    d.bitc.force_progressive_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_SUB_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_sub_cmd_force_progressive_mode(void)
{
    return (*(volatile reg_aria_disp_osdl_sub_cmd_t *)REG_ARIA_DISP_OSDL_SUB_CMD).bitc.force_progressive_mode;
}

void reg_aria_disp_set_osdl_sub_cmd_osd_plane_alpha_en(mt_u8 data)
{
    reg_aria_disp_osdl_sub_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_SUB_CMD;
    d.bitc.osd_plane_alpha_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_SUB_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_sub_cmd_osd_plane_alpha_en(void)
{
    return (*(volatile reg_aria_disp_osdl_sub_cmd_t *)REG_ARIA_DISP_OSDL_SUB_CMD).bitc.osd_plane_alpha_en;
}

void reg_aria_disp_set_osdl_sub_cmd_sub_reduce_framerate(mt_u8 data)
{
    reg_aria_disp_osdl_sub_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_SUB_CMD;
    d.bitc.sub_reduce_framerate = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_SUB_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_sub_cmd_sub_reduce_framerate(void)
{
    return (*(volatile reg_aria_disp_osdl_sub_cmd_t *)REG_ARIA_DISP_OSDL_SUB_CMD).bitc.sub_reduce_framerate;
}

void reg_aria_disp_set_osdl_sub_cmd_plane_alpha(mt_u8 data)
{
    reg_aria_disp_osdl_sub_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_SUB_CMD;
    d.bitc.plane_alpha = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_SUB_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_sub_cmd_plane_alpha(void)
{
    return (*(volatile reg_aria_disp_osdl_sub_cmd_t *)REG_ARIA_DISP_OSDL_SUB_CMD).bitc.plane_alpha;
}

void reg_aria_disp_set_osdl_sub_cmd_weak_edge_enable(mt_u8 data)
{
    reg_aria_disp_osdl_sub_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_SUB_CMD;
    d.bitc.weak_edge_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_SUB_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_sub_cmd_weak_edge_enable(void)
{
    return (*(volatile reg_aria_disp_osdl_sub_cmd_t *)REG_ARIA_DISP_OSDL_SUB_CMD).bitc.weak_edge_enable;
}


/*!
  register ARIA_DISP_osdl_cmd (read/write)
  */
void reg_aria_disp_set_osdl_cmd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD, data);
}

mt_u32  reg_aria_disp_get_osdl_cmd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD);
}

void reg_aria_disp_set_osdl_cmd_osd0_decomp_sync(mt_u8 data)
{
    reg_aria_disp_osdl_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD;
    d.bitc.osd0_decomp_sync = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_cmd_osd0_decomp_sync(void)
{
    return (*(volatile reg_aria_disp_osdl_cmd_t *)REG_ARIA_DISP_OSDL_CMD).bitc.osd0_decomp_sync;
}

void reg_aria_disp_set_osdl_cmd_osd1_decomp_sync(mt_u8 data)
{
    reg_aria_disp_osdl_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD;
    d.bitc.osd1_decomp_sync = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_cmd_osd1_decomp_sync(void)
{
    return (*(volatile reg_aria_disp_osdl_cmd_t *)REG_ARIA_DISP_OSDL_CMD).bitc.osd1_decomp_sync;
}

void reg_aria_disp_set_osdl_cmd_sub_decomp_sync(mt_u8 data)
{
    reg_aria_disp_osdl_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD;
    d.bitc.sub_decomp_sync = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_cmd_sub_decomp_sync(void)
{
    return (*(volatile reg_aria_disp_osdl_cmd_t *)REG_ARIA_DISP_OSDL_CMD).bitc.sub_decomp_sync;
}

void reg_aria_disp_set_osdl_cmd_osdl_monitor_reload(mt_u8 data)
{
    reg_aria_disp_osdl_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD;
    d.bitc.osdl_monitor_reload = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_cmd_osdl_monitor_reload(void)
{
    return (*(volatile reg_aria_disp_osdl_cmd_t *)REG_ARIA_DISP_OSDL_CMD).bitc.osdl_monitor_reload;
}

void reg_aria_disp_set_osdl_cmd_osd_latch_top(mt_u8 data)
{
    reg_aria_disp_osdl_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD;
    d.bitc.osd_latch_top = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_cmd_osd_latch_top(void)
{
    return (*(volatile reg_aria_disp_osdl_cmd_t *)REG_ARIA_DISP_OSDL_CMD).bitc.osd_latch_top;
}

void reg_aria_disp_set_osdl_cmd_osd_latch_bot(mt_u8 data)
{
    reg_aria_disp_osdl_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD;
    d.bitc.osd_latch_bot = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_cmd_osd_latch_bot(void)
{
    return (*(volatile reg_aria_disp_osdl_cmd_t *)REG_ARIA_DISP_OSDL_CMD).bitc.osd_latch_bot;
}

void reg_aria_disp_set_osdl_cmd_osd_latch_3d_1st(mt_u8 data)
{
    reg_aria_disp_osdl_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD;
    d.bitc.osd_latch_3d_1st = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_cmd_osd_latch_3d_1st(void)
{
    return (*(volatile reg_aria_disp_osdl_cmd_t *)REG_ARIA_DISP_OSDL_CMD).bitc.osd_latch_3d_1st;
}

void reg_aria_disp_set_osdl_cmd_osd_latch_3d_2nd(mt_u8 data)
{
    reg_aria_disp_osdl_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD;
    d.bitc.osd_latch_3d_2nd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_cmd_osd_latch_3d_2nd(void)
{
    return (*(volatile reg_aria_disp_osdl_cmd_t *)REG_ARIA_DISP_OSDL_CMD).bitc.osd_latch_3d_2nd;
}

void reg_aria_disp_set_osdl_cmd_osd_latch_or_not(mt_u8 data)
{
    reg_aria_disp_osdl_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD;
    d.bitc.osd_latch_or_not = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_cmd_osd_latch_or_not(void)
{
    return (*(volatile reg_aria_disp_osdl_cmd_t *)REG_ARIA_DISP_OSDL_CMD).bitc.osd_latch_or_not;
}


/*!
  register ARIA_DISP_osdl_osd0_ini_addr (read/write)
  */
void reg_aria_disp_set_osdl_osd0_ini_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD0_INI_ADDR, data);
}

mt_u32  reg_aria_disp_get_osdl_osd0_ini_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD0_INI_ADDR);
}

void reg_aria_disp_set_osdl_osd0_ini_addr_osd0_ini_addr(mt_u32 data)
{
    reg_aria_disp_osdl_osd0_ini_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD0_INI_ADDR;
    d.bitc.osd0_ini_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD0_INI_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_osdl_osd0_ini_addr_osd0_ini_addr(void)
{
    return (*(volatile reg_aria_disp_osdl_osd0_ini_addr_t *)REG_ARIA_DISP_OSDL_OSD0_INI_ADDR).bitc.osd0_ini_addr;
}


/*!
  register ARIA_DISP_osdl_osd1_ini_addr (read/write)
  */
void reg_aria_disp_set_osdl_osd1_ini_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD1_INI_ADDR, data);
}

mt_u32  reg_aria_disp_get_osdl_osd1_ini_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD1_INI_ADDR);
}

void reg_aria_disp_set_osdl_osd1_ini_addr_osd1_ini_addr(mt_u32 data)
{
    reg_aria_disp_osdl_osd1_ini_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD1_INI_ADDR;
    d.bitc.osd1_ini_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD1_INI_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_osdl_osd1_ini_addr_osd1_ini_addr(void)
{
    return (*(volatile reg_aria_disp_osdl_osd1_ini_addr_t *)REG_ARIA_DISP_OSDL_OSD1_INI_ADDR).bitc.osd1_ini_addr;
}


/*!
  register ARIA_DISP_osdl_sub_ini_addr (read/write)
  */
void reg_aria_disp_set_osdl_sub_ini_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_SUB_INI_ADDR, data);
}

mt_u32  reg_aria_disp_get_osdl_sub_ini_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_SUB_INI_ADDR);
}

void reg_aria_disp_set_osdl_sub_ini_addr_sub_ini_addr(mt_u32 data)
{
    reg_aria_disp_osdl_sub_ini_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_SUB_INI_ADDR;
    d.bitc.sub_ini_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_SUB_INI_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_osdl_sub_ini_addr_sub_ini_addr(void)
{
    return (*(volatile reg_aria_disp_osdl_sub_ini_addr_t *)REG_ARIA_DISP_OSDL_SUB_INI_ADDR).bitc.sub_ini_addr;
}


/*!
  register ARIA_DISP_osdl_ff_threshold (read/write)
  */
void reg_aria_disp_set_osdl_ff_threshold(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_FF_THRESHOLD, data);
}

mt_u32  reg_aria_disp_get_osdl_ff_threshold(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_FF_THRESHOLD);
}

void reg_aria_disp_set_osdl_ff_threshold_osdl_ff_urgent_num(mt_u8 data)
{
    reg_aria_disp_osdl_ff_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_FF_THRESHOLD;
    d.bitc.osdl_ff_urgent_num = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_FF_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_osdl_ff_threshold_osdl_ff_urgent_num(void)
{
    return (*(volatile reg_aria_disp_osdl_ff_threshold_t *)REG_ARIA_DISP_OSDL_FF_THRESHOLD).bitc.osdl_ff_urgent_num;
}


/*!
  register ARIA_DISP_osdl_rgb2y_coeff (read/write)
  */
void reg_aria_disp_set_osdl_rgb2y_coeff(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_RGB2Y_COEFF, data);
}

mt_u32  reg_aria_disp_get_osdl_rgb2y_coeff(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_RGB2Y_COEFF);
}

void reg_aria_disp_set_osdl_rgb2y_coeff_r2y_coeff(mt_u16 data)
{
    reg_aria_disp_osdl_rgb2y_coeff_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_RGB2Y_COEFF;
    d.bitc.r2y_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_RGB2Y_COEFF, d.all);
}

mt_u16  reg_aria_disp_get_osdl_rgb2y_coeff_r2y_coeff(void)
{
    return (*(volatile reg_aria_disp_osdl_rgb2y_coeff_t *)REG_ARIA_DISP_OSDL_RGB2Y_COEFF).bitc.r2y_coeff;
}

void reg_aria_disp_set_osdl_rgb2y_coeff_g2y_coeff(mt_u16 data)
{
    reg_aria_disp_osdl_rgb2y_coeff_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_RGB2Y_COEFF;
    d.bitc.g2y_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_RGB2Y_COEFF, d.all);
}

mt_u16  reg_aria_disp_get_osdl_rgb2y_coeff_g2y_coeff(void)
{
    return (*(volatile reg_aria_disp_osdl_rgb2y_coeff_t *)REG_ARIA_DISP_OSDL_RGB2Y_COEFF).bitc.g2y_coeff;
}

void reg_aria_disp_set_osdl_rgb2y_coeff_b2y_coeff(mt_u16 data)
{
    reg_aria_disp_osdl_rgb2y_coeff_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_RGB2Y_COEFF;
    d.bitc.b2y_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_RGB2Y_COEFF, d.all);
}

mt_u16  reg_aria_disp_get_osdl_rgb2y_coeff_b2y_coeff(void)
{
    return (*(volatile reg_aria_disp_osdl_rgb2y_coeff_t *)REG_ARIA_DISP_OSDL_RGB2Y_COEFF).bitc.b2y_coeff;
}


/*!
  register ARIA_DISP_osdl_rgb2cb_coeff (read/write)
  */
void reg_aria_disp_set_osdl_rgb2cb_coeff(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_RGB2CB_COEFF, data);
}

mt_u32  reg_aria_disp_get_osdl_rgb2cb_coeff(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_RGB2CB_COEFF);
}

void reg_aria_disp_set_osdl_rgb2cb_coeff_r2cb_coeff(mt_u16 data)
{
    reg_aria_disp_osdl_rgb2cb_coeff_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_RGB2CB_COEFF;
    d.bitc.r2cb_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_RGB2CB_COEFF, d.all);
}

mt_u16  reg_aria_disp_get_osdl_rgb2cb_coeff_r2cb_coeff(void)
{
    return (*(volatile reg_aria_disp_osdl_rgb2cb_coeff_t *)REG_ARIA_DISP_OSDL_RGB2CB_COEFF).bitc.r2cb_coeff;
}

void reg_aria_disp_set_osdl_rgb2cb_coeff_g2cb_coeff(mt_u16 data)
{
    reg_aria_disp_osdl_rgb2cb_coeff_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_RGB2CB_COEFF;
    d.bitc.g2cb_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_RGB2CB_COEFF, d.all);
}

mt_u16  reg_aria_disp_get_osdl_rgb2cb_coeff_g2cb_coeff(void)
{
    return (*(volatile reg_aria_disp_osdl_rgb2cb_coeff_t *)REG_ARIA_DISP_OSDL_RGB2CB_COEFF).bitc.g2cb_coeff;
}

void reg_aria_disp_set_osdl_rgb2cb_coeff_b2cb_coeff(mt_u16 data)
{
    reg_aria_disp_osdl_rgb2cb_coeff_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_RGB2CB_COEFF;
    d.bitc.b2cb_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_RGB2CB_COEFF, d.all);
}

mt_u16  reg_aria_disp_get_osdl_rgb2cb_coeff_b2cb_coeff(void)
{
    return (*(volatile reg_aria_disp_osdl_rgb2cb_coeff_t *)REG_ARIA_DISP_OSDL_RGB2CB_COEFF).bitc.b2cb_coeff;
}


/*!
  register ARIA_DISP_osdl_rgb2cr_coeff (read/write)
  */
void reg_aria_disp_set_osdl_rgb2cr_coeff(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_RGB2CR_COEFF, data);
}

mt_u32  reg_aria_disp_get_osdl_rgb2cr_coeff(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_RGB2CR_COEFF);
}

void reg_aria_disp_set_osdl_rgb2cr_coeff_r2cr_coeff(mt_u16 data)
{
    reg_aria_disp_osdl_rgb2cr_coeff_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_RGB2CR_COEFF;
    d.bitc.r2cr_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_RGB2CR_COEFF, d.all);
}

mt_u16  reg_aria_disp_get_osdl_rgb2cr_coeff_r2cr_coeff(void)
{
    return (*(volatile reg_aria_disp_osdl_rgb2cr_coeff_t *)REG_ARIA_DISP_OSDL_RGB2CR_COEFF).bitc.r2cr_coeff;
}

void reg_aria_disp_set_osdl_rgb2cr_coeff_g2cr_coeff(mt_u16 data)
{
    reg_aria_disp_osdl_rgb2cr_coeff_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_RGB2CR_COEFF;
    d.bitc.g2cr_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_RGB2CR_COEFF, d.all);
}

mt_u16  reg_aria_disp_get_osdl_rgb2cr_coeff_g2cr_coeff(void)
{
    return (*(volatile reg_aria_disp_osdl_rgb2cr_coeff_t *)REG_ARIA_DISP_OSDL_RGB2CR_COEFF).bitc.g2cr_coeff;
}

void reg_aria_disp_set_osdl_rgb2cr_coeff_b2cr_coeff(mt_u16 data)
{
    reg_aria_disp_osdl_rgb2cr_coeff_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_RGB2CR_COEFF;
    d.bitc.b2cr_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_RGB2CR_COEFF, d.all);
}

mt_u16  reg_aria_disp_get_osdl_rgb2cr_coeff_b2cr_coeff(void)
{
    return (*(volatile reg_aria_disp_osdl_rgb2cr_coeff_t *)REG_ARIA_DISP_OSDL_RGB2CR_COEFF).bitc.b2cr_coeff;
}


/*!
  register ARIA_DISP_osdl_y_offset (read/write)
  */
void reg_aria_disp_set_osdl_y_offset(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_Y_OFFSET, data);
}

mt_u32  reg_aria_disp_get_osdl_y_offset(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_Y_OFFSET);
}

void reg_aria_disp_set_osdl_y_offset_y_offset(mt_u32 data)
{
    reg_aria_disp_osdl_y_offset_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_Y_OFFSET;
    d.bitc.y_offset = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_Y_OFFSET, d.all);
}

mt_u32  reg_aria_disp_get_osdl_y_offset_y_offset(void)
{
    return (*(volatile reg_aria_disp_osdl_y_offset_t *)REG_ARIA_DISP_OSDL_Y_OFFSET).bitc.y_offset;
}


/*!
  register ARIA_DISP_osdl_cbcr_offset (read/write)
  */
void reg_aria_disp_set_osdl_cbcr_offset(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CBCR_OFFSET, data);
}

mt_u32  reg_aria_disp_get_osdl_cbcr_offset(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CBCR_OFFSET);
}

void reg_aria_disp_set_osdl_cbcr_offset_cbcr_offset(mt_u32 data)
{
    reg_aria_disp_osdl_cbcr_offset_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CBCR_OFFSET;
    d.bitc.cbcr_offset = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CBCR_OFFSET, d.all);
}

mt_u32  reg_aria_disp_get_osdl_cbcr_offset_cbcr_offset(void)
{
    return (*(volatile reg_aria_disp_osdl_cbcr_offset_t *)REG_ARIA_DISP_OSDL_CBCR_OFFSET).bitc.cbcr_offset;
}


/*!
  register ARIA_DISP_osdl_osd0_debug (read/write)
  */
void reg_aria_disp_set_osdl_osd0_debug(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD0_DEBUG, data);
}

mt_u32  reg_aria_disp_get_osdl_osd0_debug(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD0_DEBUG);
}

void reg_aria_disp_set_osdl_osd0_debug_osd0_decff_full(mt_u8 data)
{
    reg_aria_disp_osdl_osd0_debug_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD0_DEBUG;
    d.bitc.osd0_decff_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD0_DEBUG, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd0_debug_osd0_decff_full(void)
{
    return (*(volatile reg_aria_disp_osdl_osd0_debug_t *)REG_ARIA_DISP_OSDL_OSD0_DEBUG).bitc.osd0_decff_full;
}

void reg_aria_disp_set_osdl_osd0_debug_osd0_decff_empty(mt_u8 data)
{
    reg_aria_disp_osdl_osd0_debug_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD0_DEBUG;
    d.bitc.osd0_decff_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD0_DEBUG, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd0_debug_osd0_decff_empty(void)
{
    return (*(volatile reg_aria_disp_osdl_osd0_debug_t *)REG_ARIA_DISP_OSDL_OSD0_DEBUG).bitc.osd0_decff_empty;
}


/*!
  register ARIA_DISP_osdl_osd1_debug (read/write)
  */
void reg_aria_disp_set_osdl_osd1_debug(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD1_DEBUG, data);
}

mt_u32  reg_aria_disp_get_osdl_osd1_debug(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD1_DEBUG);
}

void reg_aria_disp_set_osdl_osd1_debug_osd1_decff_full(mt_u8 data)
{
    reg_aria_disp_osdl_osd1_debug_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD1_DEBUG;
    d.bitc.osd1_decff_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD1_DEBUG, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd1_debug_osd1_decff_full(void)
{
    return (*(volatile reg_aria_disp_osdl_osd1_debug_t *)REG_ARIA_DISP_OSDL_OSD1_DEBUG).bitc.osd1_decff_full;
}

void reg_aria_disp_set_osdl_osd1_debug_osd1_decff_empty(mt_u8 data)
{
    reg_aria_disp_osdl_osd1_debug_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_OSD1_DEBUG;
    d.bitc.osd1_decff_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_OSD1_DEBUG, d.all);
}

mt_u8   reg_aria_disp_get_osdl_osd1_debug_osd1_decff_empty(void)
{
    return (*(volatile reg_aria_disp_osdl_osd1_debug_t *)REG_ARIA_DISP_OSDL_OSD1_DEBUG).bitc.osd1_decff_empty;
}


/*!
  register ARIA_DISP_osdl_sub_debug (read/write)
  */
void reg_aria_disp_set_osdl_sub_debug(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_SUB_DEBUG, data);
}

mt_u32  reg_aria_disp_get_osdl_sub_debug(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_SUB_DEBUG);
}

void reg_aria_disp_set_osdl_sub_debug_sub_decff_full(mt_u8 data)
{
    reg_aria_disp_osdl_sub_debug_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_SUB_DEBUG;
    d.bitc.sub_decff_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_SUB_DEBUG, d.all);
}

mt_u8   reg_aria_disp_get_osdl_sub_debug_sub_decff_full(void)
{
    return (*(volatile reg_aria_disp_osdl_sub_debug_t *)REG_ARIA_DISP_OSDL_SUB_DEBUG).bitc.sub_decff_full;
}

void reg_aria_disp_set_osdl_sub_debug_sub_decff_empty(mt_u8 data)
{
    reg_aria_disp_osdl_sub_debug_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_SUB_DEBUG;
    d.bitc.sub_decff_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_SUB_DEBUG, d.all);
}

mt_u8   reg_aria_disp_get_osdl_sub_debug_sub_decff_empty(void)
{
    return (*(volatile reg_aria_disp_osdl_sub_debug_t *)REG_ARIA_DISP_OSDL_SUB_DEBUG).bitc.sub_decff_empty;
}


/*!
  register ARIA_DISP_osdl_cmd_ack_latency (read/write)
  */
void reg_aria_disp_set_osdl_cmd_ack_latency(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD_ACK_LATENCY, data);
}

mt_u32  reg_aria_disp_get_osdl_cmd_ack_latency(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD_ACK_LATENCY);
}

void reg_aria_disp_set_osdl_cmd_ack_latency_cmd_ack_latency_max(mt_u16 data)
{
    reg_aria_disp_osdl_cmd_ack_latency_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD_ACK_LATENCY;
    d.bitc.cmd_ack_latency_max = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD_ACK_LATENCY, d.all);
}

mt_u16  reg_aria_disp_get_osdl_cmd_ack_latency_cmd_ack_latency_max(void)
{
    return (*(volatile reg_aria_disp_osdl_cmd_ack_latency_t *)REG_ARIA_DISP_OSDL_CMD_ACK_LATENCY).bitc.cmd_ack_latency_max;
}

void reg_aria_disp_set_osdl_cmd_ack_latency_cmd_ack_latency_avg(mt_u16 data)
{
    reg_aria_disp_osdl_cmd_ack_latency_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD_ACK_LATENCY;
    d.bitc.cmd_ack_latency_avg = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD_ACK_LATENCY, d.all);
}

mt_u16  reg_aria_disp_get_osdl_cmd_ack_latency_cmd_ack_latency_avg(void)
{
    return (*(volatile reg_aria_disp_osdl_cmd_ack_latency_t *)REG_ARIA_DISP_OSDL_CMD_ACK_LATENCY).bitc.cmd_ack_latency_avg;
}


/*!
  register ARIA_DISP_osdl_cmd_dat_latency (read/write)
  */
void reg_aria_disp_set_osdl_cmd_dat_latency(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD_DAT_LATENCY, data);
}

mt_u32  reg_aria_disp_get_osdl_cmd_dat_latency(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD_DAT_LATENCY);
}

void reg_aria_disp_set_osdl_cmd_dat_latency_cmd_dat_latency_max(mt_u16 data)
{
    reg_aria_disp_osdl_cmd_dat_latency_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD_DAT_LATENCY;
    d.bitc.cmd_dat_latency_max = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD_DAT_LATENCY, d.all);
}

mt_u16  reg_aria_disp_get_osdl_cmd_dat_latency_cmd_dat_latency_max(void)
{
    return (*(volatile reg_aria_disp_osdl_cmd_dat_latency_t *)REG_ARIA_DISP_OSDL_CMD_DAT_LATENCY).bitc.cmd_dat_latency_max;
}

void reg_aria_disp_set_osdl_cmd_dat_latency_cmd_dat_latency_avg(mt_u16 data)
{
    reg_aria_disp_osdl_cmd_dat_latency_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_CMD_DAT_LATENCY;
    d.bitc.cmd_dat_latency_avg = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_CMD_DAT_LATENCY, d.all);
}

mt_u16  reg_aria_disp_get_osdl_cmd_dat_latency_cmd_dat_latency_avg(void)
{
    return (*(volatile reg_aria_disp_osdl_cmd_dat_latency_t *)REG_ARIA_DISP_OSDL_CMD_DAT_LATENCY).bitc.cmd_dat_latency_avg;
}


/*!
  register ARIA_DISP_osdl_datlast_latency (read/write)
  */
void reg_aria_disp_set_osdl_datlast_latency(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_DATLAST_LATENCY, data);
}

mt_u32  reg_aria_disp_get_osdl_datlast_latency(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDL_DATLAST_LATENCY);
}

void reg_aria_disp_set_osdl_datlast_latency_datlast_latency_max(mt_u16 data)
{
    reg_aria_disp_osdl_datlast_latency_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_DATLAST_LATENCY;
    d.bitc.datlast_latency_max = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_DATLAST_LATENCY, d.all);
}

mt_u16  reg_aria_disp_get_osdl_datlast_latency_datlast_latency_max(void)
{
    return (*(volatile reg_aria_disp_osdl_datlast_latency_t *)REG_ARIA_DISP_OSDL_DATLAST_LATENCY).bitc.datlast_latency_max;
}

void reg_aria_disp_set_osdl_datlast_latency_datlast_latency_avg(mt_u16 data)
{
    reg_aria_disp_osdl_datlast_latency_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDL_DATLAST_LATENCY;
    d.bitc.datlast_latency_avg = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDL_DATLAST_LATENCY, d.all);
}

mt_u16  reg_aria_disp_get_osdl_datlast_latency_datlast_latency_avg(void)
{
    return (*(volatile reg_aria_disp_osdl_datlast_latency_t *)REG_ARIA_DISP_OSDL_DATLAST_LATENCY).bitc.datlast_latency_avg;
}


/*!
  register ARIA_DISP_osdm_cmd (read/write)
  */
void reg_aria_disp_set_osdm_cmd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDM_CMD, data);
}

mt_u32  reg_aria_disp_get_osdm_cmd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDM_CMD);
}

void reg_aria_disp_set_osdm_cmd_osd_sub_mux_sel(mt_u8 data)
{
    reg_aria_disp_osdm_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDM_CMD;
    d.bitc.osd_sub_mux_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDM_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdm_cmd_osd_sub_mux_sel(void)
{
    return (*(volatile reg_aria_disp_osdm_cmd_t *)REG_ARIA_DISP_OSDM_CMD).bitc.osd_sub_mux_sel;
}

void reg_aria_disp_set_osdm_cmd_osd_sub_mix_first(mt_u8 data)
{
    reg_aria_disp_osdm_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDM_CMD;
    d.bitc.osd_sub_mix_first = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDM_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdm_cmd_osd_sub_mix_first(void)
{
    return (*(volatile reg_aria_disp_osdm_cmd_t *)REG_ARIA_DISP_OSDM_CMD).bitc.osd_sub_mix_first;
}


/*!
  register ARIA_DISP_osdm_threshold (read/write)
  */
void reg_aria_disp_set_osdm_threshold(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDM_THRESHOLD, data);
}

mt_u32  reg_aria_disp_get_osdm_threshold(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDM_THRESHOLD);
}

void reg_aria_disp_set_osdm_threshold_osdm_full_threshold(mt_u8 data)
{
    reg_aria_disp_osdm_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDM_THRESHOLD;
    d.bitc.osdm_full_threshold = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDM_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_osdm_threshold_osdm_full_threshold(void)
{
    return (*(volatile reg_aria_disp_osdm_threshold_t *)REG_ARIA_DISP_OSDM_THRESHOLD).bitc.osdm_full_threshold;
}

void reg_aria_disp_set_osdm_threshold_osdm_empty_threshold(mt_u8 data)
{
    reg_aria_disp_osdm_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDM_THRESHOLD;
    d.bitc.osdm_empty_threshold = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDM_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_osdm_threshold_osdm_empty_threshold(void)
{
    return (*(volatile reg_aria_disp_osdm_threshold_t *)REG_ARIA_DISP_OSDM_THRESHOLD).bitc.osdm_empty_threshold;
}


/*!
  register ARIA_DISP_osdm_osd0_ckey (read/write)
  */
void reg_aria_disp_set_osdm_osd0_ckey(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDM_OSD0_CKEY, data);
}

mt_u32  reg_aria_disp_get_osdm_osd0_ckey(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDM_OSD0_CKEY);
}

void reg_aria_disp_set_osdm_osd0_ckey_colorkey_value(mt_u32 data)
{
    reg_aria_disp_osdm_osd0_ckey_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDM_OSD0_CKEY;
    d.bitc.colorkey_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDM_OSD0_CKEY, d.all);
}

mt_u32  reg_aria_disp_get_osdm_osd0_ckey_colorkey_value(void)
{
    return (*(volatile reg_aria_disp_osdm_osd0_ckey_t *)REG_ARIA_DISP_OSDM_OSD0_CKEY).bitc.colorkey_value;
}

void reg_aria_disp_set_osdm_osd0_ckey_osd0_colorkey_en(mt_u8 data)
{
    reg_aria_disp_osdm_osd0_ckey_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDM_OSD0_CKEY;
    d.bitc.osd0_colorkey_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDM_OSD0_CKEY, d.all);
}

mt_u8   reg_aria_disp_get_osdm_osd0_ckey_osd0_colorkey_en(void)
{
    return (*(volatile reg_aria_disp_osdm_osd0_ckey_t *)REG_ARIA_DISP_OSDM_OSD0_CKEY).bitc.osd0_colorkey_en;
}


/*!
  register ARIA_DISP_osdm_osd1_ckey (read/write)
  */
void reg_aria_disp_set_osdm_osd1_ckey(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDM_OSD1_CKEY, data);
}

mt_u32  reg_aria_disp_get_osdm_osd1_ckey(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDM_OSD1_CKEY);
}

void reg_aria_disp_set_osdm_osd1_ckey_colorkey_value(mt_u32 data)
{
    reg_aria_disp_osdm_osd1_ckey_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDM_OSD1_CKEY;
    d.bitc.colorkey_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDM_OSD1_CKEY, d.all);
}

mt_u32  reg_aria_disp_get_osdm_osd1_ckey_colorkey_value(void)
{
    return (*(volatile reg_aria_disp_osdm_osd1_ckey_t *)REG_ARIA_DISP_OSDM_OSD1_CKEY).bitc.colorkey_value;
}

void reg_aria_disp_set_osdm_osd1_ckey_osd1_colorkey_en(mt_u8 data)
{
    reg_aria_disp_osdm_osd1_ckey_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDM_OSD1_CKEY;
    d.bitc.osd1_colorkey_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDM_OSD1_CKEY, d.all);
}

mt_u8   reg_aria_disp_get_osdm_osd1_ckey_osd1_colorkey_en(void)
{
    return (*(volatile reg_aria_disp_osdm_osd1_ckey_t *)REG_ARIA_DISP_OSDM_OSD1_CKEY).bitc.osd1_colorkey_en;
}


/*!
  register ARIA_DISP_osds_cmd (read/write)
  */
void reg_aria_disp_set_osds_cmd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_CMD, data);
}

mt_u32  reg_aria_disp_get_osds_cmd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDS_CMD);
}

void reg_aria_disp_set_osds_cmd_osd_vphase_type(mt_u8 data)
{
    reg_aria_disp_osds_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_CMD;
    d.bitc.osd_vphase_type = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osds_cmd_osd_vphase_type(void)
{
    return (*(volatile reg_aria_disp_osds_cmd_t *)REG_ARIA_DISP_OSDS_CMD).bitc.osd_vphase_type;
}

void reg_aria_disp_set_osds_cmd_osd_vert_no_filter(mt_u8 data)
{
    reg_aria_disp_osds_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_CMD;
    d.bitc.osd_vert_no_filter = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osds_cmd_osd_vert_no_filter(void)
{
    return (*(volatile reg_aria_disp_osds_cmd_t *)REG_ARIA_DISP_OSDS_CMD).bitc.osd_vert_no_filter;
}

void reg_aria_disp_set_osds_cmd_osd_vert_no_filter_alpha(mt_u8 data)
{
    reg_aria_disp_osds_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_CMD;
    d.bitc.osd_vert_no_filter_alpha = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osds_cmd_osd_vert_no_filter_alpha(void)
{
    return (*(volatile reg_aria_disp_osds_cmd_t *)REG_ARIA_DISP_OSDS_CMD).bitc.osd_vert_no_filter_alpha;
}

void reg_aria_disp_set_osds_cmd_osd_vert_no_boundary(mt_u8 data)
{
    reg_aria_disp_osds_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_CMD;
    d.bitc.osd_vert_no_boundary = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osds_cmd_osd_vert_no_boundary(void)
{
    return (*(volatile reg_aria_disp_osds_cmd_t *)REG_ARIA_DISP_OSDS_CMD).bitc.osd_vert_no_boundary;
}

void reg_aria_disp_set_osds_cmd_osd_vert_bypass_en(mt_u8 data)
{
    reg_aria_disp_osds_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_CMD;
    d.bitc.osd_vert_bypass_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osds_cmd_osd_vert_bypass_en(void)
{
    return (*(volatile reg_aria_disp_osds_cmd_t *)REG_ARIA_DISP_OSDS_CMD).bitc.osd_vert_bypass_en;
}

void reg_aria_disp_set_osds_cmd_osd_hori_no_filter_alpha(mt_u8 data)
{
    reg_aria_disp_osds_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_CMD;
    d.bitc.osd_hori_no_filter_alpha = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osds_cmd_osd_hori_no_filter_alpha(void)
{
    return (*(volatile reg_aria_disp_osds_cmd_t *)REG_ARIA_DISP_OSDS_CMD).bitc.osd_hori_no_filter_alpha;
}

void reg_aria_disp_set_osds_cmd_osd_hori_do_boundary(mt_u8 data)
{
    reg_aria_disp_osds_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_CMD;
    d.bitc.osd_hori_do_boundary = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osds_cmd_osd_hori_do_boundary(void)
{
    return (*(volatile reg_aria_disp_osds_cmd_t *)REG_ARIA_DISP_OSDS_CMD).bitc.osd_hori_do_boundary;
}

void reg_aria_disp_set_osds_cmd_osd_hori_start_fra(mt_u16 data)
{
    reg_aria_disp_osds_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_CMD;
    d.bitc.osd_hori_start_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_CMD, d.all);
}

mt_u16  reg_aria_disp_get_osds_cmd_osd_hori_start_fra(void)
{
    return (*(volatile reg_aria_disp_osds_cmd_t *)REG_ARIA_DISP_OSDS_CMD).bitc.osd_hori_start_fra;
}

void reg_aria_disp_set_osds_cmd_osd_hori_no_filter(mt_u8 data)
{
    reg_aria_disp_osds_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_CMD;
    d.bitc.osd_hori_no_filter = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osds_cmd_osd_hori_no_filter(void)
{
    return (*(volatile reg_aria_disp_osds_cmd_t *)REG_ARIA_DISP_OSDS_CMD).bitc.osd_hori_no_filter;
}


/*!
  register ARIA_DISP_osds_hsize (read/write)
  */
void reg_aria_disp_set_osds_hsize(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_HSIZE, data);
}

mt_u32  reg_aria_disp_get_osds_hsize(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDS_HSIZE);
}

void reg_aria_disp_set_osds_hsize_osd_dst_hsize(mt_u16 data)
{
    reg_aria_disp_osds_hsize_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_HSIZE;
    d.bitc.osd_dst_hsize = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_HSIZE, d.all);
}

mt_u16  reg_aria_disp_get_osds_hsize_osd_dst_hsize(void)
{
    return (*(volatile reg_aria_disp_osds_hsize_t *)REG_ARIA_DISP_OSDS_HSIZE).bitc.osd_dst_hsize;
}

void reg_aria_disp_set_osds_hsize_osd_ori_hsize(mt_u16 data)
{
    reg_aria_disp_osds_hsize_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_HSIZE;
    d.bitc.osd_ori_hsize = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_HSIZE, d.all);
}

mt_u16  reg_aria_disp_get_osds_hsize_osd_ori_hsize(void)
{
    return (*(volatile reg_aria_disp_osds_hsize_t *)REG_ARIA_DISP_OSDS_HSIZE).bitc.osd_ori_hsize;
}


/*!
  register ARIA_DISP_osds_hratio (read/write)
  */
void reg_aria_disp_set_osds_hratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_HRATIO, data);
}

mt_u32  reg_aria_disp_get_osds_hratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDS_HRATIO);
}

void reg_aria_disp_set_osds_hratio_osd_hori_ratio_fra(mt_u16 data)
{
    reg_aria_disp_osds_hratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_HRATIO;
    d.bitc.osd_hori_ratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_HRATIO, d.all);
}

mt_u16  reg_aria_disp_get_osds_hratio_osd_hori_ratio_fra(void)
{
    return (*(volatile reg_aria_disp_osds_hratio_t *)REG_ARIA_DISP_OSDS_HRATIO).bitc.osd_hori_ratio_fra;
}

void reg_aria_disp_set_osds_hratio_osd_hori_ratio_int(mt_u8 data)
{
    reg_aria_disp_osds_hratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_HRATIO;
    d.bitc.osd_hori_ratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_HRATIO, d.all);
}

mt_u8   reg_aria_disp_get_osds_hratio_osd_hori_ratio_int(void)
{
    return (*(volatile reg_aria_disp_osds_hratio_t *)REG_ARIA_DISP_OSDS_HRATIO).bitc.osd_hori_ratio_int;
}


/*!
  register ARIA_DISP_osds_hf_coeff_addr (read/write)
  */
void reg_aria_disp_set_osds_hf_coeff_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_HF_COEFF_ADDR, data);
}

mt_u32  reg_aria_disp_get_osds_hf_coeff_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDS_HF_COEFF_ADDR);
}

void reg_aria_disp_set_osds_hf_coeff_addr_osd_hf_coeff_addr(mt_u32 data)
{
    reg_aria_disp_osds_hf_coeff_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_HF_COEFF_ADDR;
    d.bitc.osd_hf_coeff_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_HF_COEFF_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_osds_hf_coeff_addr_osd_hf_coeff_addr(void)
{
    return (*(volatile reg_aria_disp_osds_hf_coeff_addr_t *)REG_ARIA_DISP_OSDS_HF_COEFF_ADDR).bitc.osd_hf_coeff_addr;
}


/*!
  register ARIA_DISP_osds_vsize (read/write)
  */
void reg_aria_disp_set_osds_vsize(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_VSIZE, data);
}

mt_u32  reg_aria_disp_get_osds_vsize(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDS_VSIZE);
}

void reg_aria_disp_set_osds_vsize_osd_dst_vsize(mt_u16 data)
{
    reg_aria_disp_osds_vsize_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_VSIZE;
    d.bitc.osd_dst_vsize = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_VSIZE, d.all);
}

mt_u16  reg_aria_disp_get_osds_vsize_osd_dst_vsize(void)
{
    return (*(volatile reg_aria_disp_osds_vsize_t *)REG_ARIA_DISP_OSDS_VSIZE).bitc.osd_dst_vsize;
}

void reg_aria_disp_set_osds_vsize_osd_ori_vsize(mt_u16 data)
{
    reg_aria_disp_osds_vsize_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_VSIZE;
    d.bitc.osd_ori_vsize = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_VSIZE, d.all);
}

mt_u16  reg_aria_disp_get_osds_vsize_osd_ori_vsize(void)
{
    return (*(volatile reg_aria_disp_osds_vsize_t *)REG_ARIA_DISP_OSDS_VSIZE).bitc.osd_ori_vsize;
}


/*!
  register ARIA_DISP_osds_vratio (read/write)
  */
void reg_aria_disp_set_osds_vratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_VRATIO, data);
}

mt_u32  reg_aria_disp_get_osds_vratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDS_VRATIO);
}

void reg_aria_disp_set_osds_vratio_osd_vert_ratio_fra(mt_u16 data)
{
    reg_aria_disp_osds_vratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_VRATIO;
    d.bitc.osd_vert_ratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_VRATIO, d.all);
}

mt_u16  reg_aria_disp_get_osds_vratio_osd_vert_ratio_fra(void)
{
    return (*(volatile reg_aria_disp_osds_vratio_t *)REG_ARIA_DISP_OSDS_VRATIO).bitc.osd_vert_ratio_fra;
}

void reg_aria_disp_set_osds_vratio_osd_vert_ratio_int(mt_u8 data)
{
    reg_aria_disp_osds_vratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_VRATIO;
    d.bitc.osd_vert_ratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_VRATIO, d.all);
}

mt_u8   reg_aria_disp_get_osds_vratio_osd_vert_ratio_int(void)
{
    return (*(volatile reg_aria_disp_osds_vratio_t *)REG_ARIA_DISP_OSDS_VRATIO).bitc.osd_vert_ratio_int;
}


/*!
  register ARIA_DISP_osds_vf_coeff_addr (read/write)
  */
void reg_aria_disp_set_osds_vf_coeff_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_VF_COEFF_ADDR, data);
}

mt_u32  reg_aria_disp_get_osds_vf_coeff_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDS_VF_COEFF_ADDR);
}

void reg_aria_disp_set_osds_vf_coeff_addr_osd_vf_coeff_addr(mt_u32 data)
{
    reg_aria_disp_osds_vf_coeff_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_VF_COEFF_ADDR;
    d.bitc.osd_vf_coeff_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_VF_COEFF_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_osds_vf_coeff_addr_osd_vf_coeff_addr(void)
{
    return (*(volatile reg_aria_disp_osds_vf_coeff_addr_t *)REG_ARIA_DISP_OSDS_VF_COEFF_ADDR).bitc.osd_vf_coeff_addr;
}


/*!
  register ARIA_DISP_osds_v_start_line (read/write)
  */
void reg_aria_disp_set_osds_v_start_line(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_V_START_LINE, data);
}

mt_u32  reg_aria_disp_get_osds_v_start_line(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDS_V_START_LINE);
}

void reg_aria_disp_set_osds_v_start_line_osd_odd_start_line(mt_u8 data)
{
    reg_aria_disp_osds_v_start_line_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_V_START_LINE;
    d.bitc.osd_odd_start_line = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_V_START_LINE, d.all);
}

mt_u8   reg_aria_disp_get_osds_v_start_line_osd_odd_start_line(void)
{
    return (*(volatile reg_aria_disp_osds_v_start_line_t *)REG_ARIA_DISP_OSDS_V_START_LINE).bitc.osd_odd_start_line;
}

void reg_aria_disp_set_osds_v_start_line_osd_even_start_line(mt_u8 data)
{
    reg_aria_disp_osds_v_start_line_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_V_START_LINE;
    d.bitc.osd_even_start_line = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_V_START_LINE, d.all);
}

mt_u8   reg_aria_disp_get_osds_v_start_line_osd_even_start_line(void)
{
    return (*(volatile reg_aria_disp_osds_v_start_line_t *)REG_ARIA_DISP_OSDS_V_START_LINE).bitc.osd_even_start_line;
}


/*!
  register ARIA_DISP_osds_v_start_fra (read/write)
  */
void reg_aria_disp_set_osds_v_start_fra(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_V_START_FRA, data);
}

mt_u32  reg_aria_disp_get_osds_v_start_fra(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDS_V_START_FRA);
}

void reg_aria_disp_set_osds_v_start_fra_osd_vert_start_fra_odd(mt_u16 data)
{
    reg_aria_disp_osds_v_start_fra_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_V_START_FRA;
    d.bitc.osd_vert_start_fra_odd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_V_START_FRA, d.all);
}

mt_u16  reg_aria_disp_get_osds_v_start_fra_osd_vert_start_fra_odd(void)
{
    return (*(volatile reg_aria_disp_osds_v_start_fra_t *)REG_ARIA_DISP_OSDS_V_START_FRA).bitc.osd_vert_start_fra_odd;
}

void reg_aria_disp_set_osds_v_start_fra_osd_vert_start_fra_even(mt_u16 data)
{
    reg_aria_disp_osds_v_start_fra_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_V_START_FRA;
    d.bitc.osd_vert_start_fra_even = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_V_START_FRA, d.all);
}

mt_u16  reg_aria_disp_get_osds_v_start_fra_osd_vert_start_fra_even(void)
{
    return (*(volatile reg_aria_disp_osds_v_start_fra_t *)REG_ARIA_DISP_OSDS_V_START_FRA).bitc.osd_vert_start_fra_even;
}


/*!
  register ARIA_DISP_osds_v_tap (read/write)
  */
void reg_aria_disp_set_osds_v_tap(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_V_TAP, data);
}

mt_u32  reg_aria_disp_get_osds_v_tap(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDS_V_TAP);
}

void reg_aria_disp_set_osds_v_tap_osd_vert_tap_num(mt_u8 data)
{
    reg_aria_disp_osds_v_tap_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDS_V_TAP;
    d.bitc.osd_vert_tap_num = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDS_V_TAP, d.all);
}

mt_u8   reg_aria_disp_get_osds_v_tap_osd_vert_tap_num(void)
{
    return (*(volatile reg_aria_disp_osds_v_tap_t *)REG_ARIA_DISP_OSDS_V_TAP).bitc.osd_vert_tap_num;
}


/*!
  register ARIA_DISP_osdd_osd0_cmd (read/write)
  */
void reg_aria_disp_set_osdd_osd0_cmd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CMD, data);
}

mt_u32  reg_aria_disp_get_osdd_osd0_cmd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CMD);
}

void reg_aria_disp_set_osdd_osd0_cmd_compress_en_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CMD;
    d.bitc.compress_en_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_cmd_compress_en_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_cmd_t *)REG_ARIA_DISP_OSDD_OSD0_CMD).bitc.compress_en_osd0;
}


/*!
  register ARIA_DISP_osdd_osd0_length_a (read/write)
  */
void reg_aria_disp_set_osdd_osd0_length_a(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_A, data);
}

mt_u32  reg_aria_disp_get_osdd_osd0_length_a(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_A);
}

void reg_aria_disp_set_osdd_osd0_length_a_a_compress_bits_osd0(mt_u32 data)
{
    reg_aria_disp_osdd_osd0_length_a_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_A;
    d.bitc.a_compress_bits_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_A, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd0_length_a_a_compress_bits_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_length_a_t *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_A).bitc.a_compress_bits_osd0;
}


/*!
  register ARIA_DISP_osdd_osd0_length_r (read/write)
  */
void reg_aria_disp_set_osdd_osd0_length_r(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_R, data);
}

mt_u32  reg_aria_disp_get_osdd_osd0_length_r(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_R);
}

void reg_aria_disp_set_osdd_osd0_length_r_r_compress_bits_osd0(mt_u32 data)
{
    reg_aria_disp_osdd_osd0_length_r_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_R;
    d.bitc.r_compress_bits_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_R, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd0_length_r_r_compress_bits_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_length_r_t *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_R).bitc.r_compress_bits_osd0;
}


/*!
  register ARIA_DISP_osdd_osd0_length_g (read/write)
  */
void reg_aria_disp_set_osdd_osd0_length_g(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_G, data);
}

mt_u32  reg_aria_disp_get_osdd_osd0_length_g(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_G);
}

void reg_aria_disp_set_osdd_osd0_length_g_g_compress_bits_osd0(mt_u32 data)
{
    reg_aria_disp_osdd_osd0_length_g_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_G;
    d.bitc.g_compress_bits_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_G, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd0_length_g_g_compress_bits_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_length_g_t *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_G).bitc.g_compress_bits_osd0;
}


/*!
  register ARIA_DISP_osdd_osd0_length_b (read/write)
  */
void reg_aria_disp_set_osdd_osd0_length_b(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_B, data);
}

mt_u32  reg_aria_disp_get_osdd_osd0_length_b(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_B);
}

void reg_aria_disp_set_osdd_osd0_length_b_b_compress_bits_osd0(mt_u32 data)
{
    reg_aria_disp_osdd_osd0_length_b_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_B;
    d.bitc.b_compress_bits_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_B, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd0_length_b_b_compress_bits_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_length_b_t *)REG_ARIA_DISP_OSDD_OSD0_LENGTH_B).bitc.b_compress_bits_osd0;
}


/*!
  register ARIA_DISP_osdd_osd0_addr_a (read/write)
  */
void reg_aria_disp_set_osdd_osd0_addr_a(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_ADDR_A, data);
}

mt_u32  reg_aria_disp_get_osdd_osd0_addr_a(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_ADDR_A);
}

void reg_aria_disp_set_osdd_osd0_addr_a_ff0_ddr_addr_base_osd0(mt_u32 data)
{
    reg_aria_disp_osdd_osd0_addr_a_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_ADDR_A;
    d.bitc.ff0_ddr_addr_base_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_ADDR_A, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd0_addr_a_ff0_ddr_addr_base_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_addr_a_t *)REG_ARIA_DISP_OSDD_OSD0_ADDR_A).bitc.ff0_ddr_addr_base_osd0;
}


/*!
  register ARIA_DISP_osdd_osd0_addr_r (read/write)
  */
void reg_aria_disp_set_osdd_osd0_addr_r(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_ADDR_R, data);
}

mt_u32  reg_aria_disp_get_osdd_osd0_addr_r(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_ADDR_R);
}

void reg_aria_disp_set_osdd_osd0_addr_r_ff1_ddr_addr_base_osd0(mt_u32 data)
{
    reg_aria_disp_osdd_osd0_addr_r_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_ADDR_R;
    d.bitc.ff1_ddr_addr_base_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_ADDR_R, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd0_addr_r_ff1_ddr_addr_base_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_addr_r_t *)REG_ARIA_DISP_OSDD_OSD0_ADDR_R).bitc.ff1_ddr_addr_base_osd0;
}


/*!
  register ARIA_DISP_osdd_osd0_addr_g (read/write)
  */
void reg_aria_disp_set_osdd_osd0_addr_g(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_ADDR_G, data);
}

mt_u32  reg_aria_disp_get_osdd_osd0_addr_g(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_ADDR_G);
}

void reg_aria_disp_set_osdd_osd0_addr_g_ff2_ddr_addr_base_osd0(mt_u32 data)
{
    reg_aria_disp_osdd_osd0_addr_g_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_ADDR_G;
    d.bitc.ff2_ddr_addr_base_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_ADDR_G, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd0_addr_g_ff2_ddr_addr_base_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_addr_g_t *)REG_ARIA_DISP_OSDD_OSD0_ADDR_G).bitc.ff2_ddr_addr_base_osd0;
}


/*!
  register ARIA_DISP_osdd_osd0_addr_b (read/write)
  */
void reg_aria_disp_set_osdd_osd0_addr_b(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_ADDR_B, data);
}

mt_u32  reg_aria_disp_get_osdd_osd0_addr_b(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_ADDR_B);
}

void reg_aria_disp_set_osdd_osd0_addr_b_ff3_ddr_addr_base_osd0(mt_u32 data)
{
    reg_aria_disp_osdd_osd0_addr_b_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_ADDR_B;
    d.bitc.ff3_ddr_addr_base_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_ADDR_B, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd0_addr_b_ff3_ddr_addr_base_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_addr_b_t *)REG_ARIA_DISP_OSDD_OSD0_ADDR_B).bitc.ff3_ddr_addr_base_osd0;
}


/*!
  register ARIA_DISP_osdd_osd0_ctl (read/write)
  */
void reg_aria_disp_set_osdd_osd0_ctl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL, data);
}

mt_u32  reg_aria_disp_get_osdd_osd0_ctl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL);
}

void reg_aria_disp_set_osdd_osd0_ctl_runl_diff_num_a_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL;
    d.bitc.runl_diff_num_a_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl_runl_diff_num_a_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl_t *)REG_ARIA_DISP_OSDD_OSD0_CTL).bitc.runl_diff_num_a_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl_runl_diff_num_r_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL;
    d.bitc.runl_diff_num_r_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl_runl_diff_num_r_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl_t *)REG_ARIA_DISP_OSDD_OSD0_CTL).bitc.runl_diff_num_r_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl_runl_diff_num_g_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL;
    d.bitc.runl_diff_num_g_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl_runl_diff_num_g_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl_t *)REG_ARIA_DISP_OSDD_OSD0_CTL).bitc.runl_diff_num_g_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl_runl_diff_num_b_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL;
    d.bitc.runl_diff_num_b_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl_runl_diff_num_b_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl_t *)REG_ARIA_DISP_OSDD_OSD0_CTL).bitc.runl_diff_num_b_osd0;
}


/*!
  register ARIA_DISP_osdd_osd0_ctl2 (read/write)
  */
void reg_aria_disp_set_osdd_osd0_ctl2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, data);
}

mt_u32  reg_aria_disp_get_osdd_osd0_ctl2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2);
}

void reg_aria_disp_set_osdd_osd0_ctl2_a_osdd_arith_adapt_en_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.a_osdd_arith_adapt_en_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_a_osdd_arith_adapt_en_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.a_osdd_arith_adapt_en_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_r_osdd_arith_adapt_en_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.r_osdd_arith_adapt_en_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_r_osdd_arith_adapt_en_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.r_osdd_arith_adapt_en_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_g_osdd_arith_adapt_en_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.g_osdd_arith_adapt_en_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_g_osdd_arith_adapt_en_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.g_osdd_arith_adapt_en_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_b_osdd_arith_adapt_en_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.b_osdd_arith_adapt_en_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_b_osdd_arith_adapt_en_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.b_osdd_arith_adapt_en_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_lossy_a_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.lossy_a_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_lossy_a_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.lossy_a_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_lossy_r_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.lossy_r_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_lossy_r_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.lossy_r_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_lossy_g_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.lossy_g_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_lossy_g_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.lossy_g_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_lossy_b_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.lossy_b_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_lossy_b_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.lossy_b_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_a_osdd_init_arith_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.a_osdd_init_arith_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_a_osdd_init_arith_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.a_osdd_init_arith_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_r_osdd_init_arith_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.r_osdd_init_arith_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_r_osdd_init_arith_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.r_osdd_init_arith_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_g_osdd_init_arith_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.g_osdd_init_arith_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_g_osdd_init_arith_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.g_osdd_init_arith_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_b_osdd_init_arith_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.b_osdd_init_arith_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_b_osdd_init_arith_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.b_osdd_init_arith_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_a_dpcm_quanmode_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.a_dpcm_quanmode_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_a_dpcm_quanmode_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.a_dpcm_quanmode_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_r_dpcm_quanmode_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.r_dpcm_quanmode_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_r_dpcm_quanmode_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.r_dpcm_quanmode_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_g_dpcm_quanmode_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.g_dpcm_quanmode_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_g_dpcm_quanmode_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.g_dpcm_quanmode_osd0;
}

void reg_aria_disp_set_osdd_osd0_ctl2_b_dpcm_quanmode_osd0(mt_u8 data)
{
    reg_aria_disp_osdd_osd0_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD0_CTL2;
    d.bitc.b_dpcm_quanmode_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD0_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd0_ctl2_b_dpcm_quanmode_osd0(void)
{
    return (*(volatile reg_aria_disp_osdd_osd0_ctl2_t *)REG_ARIA_DISP_OSDD_OSD0_CTL2).bitc.b_dpcm_quanmode_osd0;
}


/*!
  register ARIA_DISP_osdd_osd1_cmd (read/write)
  */
void reg_aria_disp_set_osdd_osd1_cmd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CMD, data);
}

mt_u32  reg_aria_disp_get_osdd_osd1_cmd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CMD);
}

void reg_aria_disp_set_osdd_osd1_cmd_compress_en_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CMD;
    d.bitc.compress_en_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_cmd_compress_en_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_cmd_t *)REG_ARIA_DISP_OSDD_OSD1_CMD).bitc.compress_en_osd1;
}


/*!
  register ARIA_DISP_osdd_osd1_length_a (read/write)
  */
void reg_aria_disp_set_osdd_osd1_length_a(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_A, data);
}

mt_u32  reg_aria_disp_get_osdd_osd1_length_a(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_A);
}

void reg_aria_disp_set_osdd_osd1_length_a_a_compress_bits_osd1(mt_u32 data)
{
    reg_aria_disp_osdd_osd1_length_a_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_A;
    d.bitc.a_compress_bits_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_A, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd1_length_a_a_compress_bits_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_length_a_t *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_A).bitc.a_compress_bits_osd1;
}


/*!
  register ARIA_DISP_osdd_osd1_length_r (read/write)
  */
void reg_aria_disp_set_osdd_osd1_length_r(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_R, data);
}

mt_u32  reg_aria_disp_get_osdd_osd1_length_r(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_R);
}

void reg_aria_disp_set_osdd_osd1_length_r_r_compress_bits_osd1(mt_u32 data)
{
    reg_aria_disp_osdd_osd1_length_r_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_R;
    d.bitc.r_compress_bits_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_R, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd1_length_r_r_compress_bits_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_length_r_t *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_R).bitc.r_compress_bits_osd1;
}


/*!
  register ARIA_DISP_osdd_osd1_length_g (read/write)
  */
void reg_aria_disp_set_osdd_osd1_length_g(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_G, data);
}

mt_u32  reg_aria_disp_get_osdd_osd1_length_g(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_G);
}

void reg_aria_disp_set_osdd_osd1_length_g_g_compress_bits_osd1(mt_u32 data)
{
    reg_aria_disp_osdd_osd1_length_g_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_G;
    d.bitc.g_compress_bits_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_G, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd1_length_g_g_compress_bits_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_length_g_t *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_G).bitc.g_compress_bits_osd1;
}


/*!
  register ARIA_DISP_osdd_osd1_length_b (read/write)
  */
void reg_aria_disp_set_osdd_osd1_length_b(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_B, data);
}

mt_u32  reg_aria_disp_get_osdd_osd1_length_b(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_B);
}

void reg_aria_disp_set_osdd_osd1_length_b_b_compress_bits_osd1(mt_u32 data)
{
    reg_aria_disp_osdd_osd1_length_b_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_B;
    d.bitc.b_compress_bits_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_B, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd1_length_b_b_compress_bits_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_length_b_t *)REG_ARIA_DISP_OSDD_OSD1_LENGTH_B).bitc.b_compress_bits_osd1;
}


/*!
  register ARIA_DISP_osdd_osd1_addr_a (read/write)
  */
void reg_aria_disp_set_osdd_osd1_addr_a(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_ADDR_A, data);
}

mt_u32  reg_aria_disp_get_osdd_osd1_addr_a(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_ADDR_A);
}

void reg_aria_disp_set_osdd_osd1_addr_a_ff0_ddr_addr_base_osd1(mt_u32 data)
{
    reg_aria_disp_osdd_osd1_addr_a_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_ADDR_A;
    d.bitc.ff0_ddr_addr_base_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_ADDR_A, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd1_addr_a_ff0_ddr_addr_base_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_addr_a_t *)REG_ARIA_DISP_OSDD_OSD1_ADDR_A).bitc.ff0_ddr_addr_base_osd1;
}


/*!
  register ARIA_DISP_osdd_osd1_addr_r (read/write)
  */
void reg_aria_disp_set_osdd_osd1_addr_r(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_ADDR_R, data);
}

mt_u32  reg_aria_disp_get_osdd_osd1_addr_r(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_ADDR_R);
}

void reg_aria_disp_set_osdd_osd1_addr_r_ff1_ddr_addr_base_osd1(mt_u32 data)
{
    reg_aria_disp_osdd_osd1_addr_r_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_ADDR_R;
    d.bitc.ff1_ddr_addr_base_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_ADDR_R, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd1_addr_r_ff1_ddr_addr_base_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_addr_r_t *)REG_ARIA_DISP_OSDD_OSD1_ADDR_R).bitc.ff1_ddr_addr_base_osd1;
}


/*!
  register ARIA_DISP_osdd_osd1_addr_g (read/write)
  */
void reg_aria_disp_set_osdd_osd1_addr_g(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_ADDR_G, data);
}

mt_u32  reg_aria_disp_get_osdd_osd1_addr_g(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_ADDR_G);
}

void reg_aria_disp_set_osdd_osd1_addr_g_ff2_ddr_addr_base_osd1(mt_u32 data)
{
    reg_aria_disp_osdd_osd1_addr_g_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_ADDR_G;
    d.bitc.ff2_ddr_addr_base_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_ADDR_G, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd1_addr_g_ff2_ddr_addr_base_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_addr_g_t *)REG_ARIA_DISP_OSDD_OSD1_ADDR_G).bitc.ff2_ddr_addr_base_osd1;
}


/*!
  register ARIA_DISP_osdd_osd1_addr_b (read/write)
  */
void reg_aria_disp_set_osdd_osd1_addr_b(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_ADDR_B, data);
}

mt_u32  reg_aria_disp_get_osdd_osd1_addr_b(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_ADDR_B);
}

void reg_aria_disp_set_osdd_osd1_addr_b_ff3_ddr_addr_base_osd1(mt_u32 data)
{
    reg_aria_disp_osdd_osd1_addr_b_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_ADDR_B;
    d.bitc.ff3_ddr_addr_base_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_ADDR_B, d.all);
}

mt_u32  reg_aria_disp_get_osdd_osd1_addr_b_ff3_ddr_addr_base_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_addr_b_t *)REG_ARIA_DISP_OSDD_OSD1_ADDR_B).bitc.ff3_ddr_addr_base_osd1;
}


/*!
  register ARIA_DISP_osdd_osd1_ctl (read/write)
  */
void reg_aria_disp_set_osdd_osd1_ctl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL, data);
}

mt_u32  reg_aria_disp_get_osdd_osd1_ctl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL);
}

void reg_aria_disp_set_osdd_osd1_ctl_runl_diff_num_a_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL;
    d.bitc.runl_diff_num_a_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl_runl_diff_num_a_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl_t *)REG_ARIA_DISP_OSDD_OSD1_CTL).bitc.runl_diff_num_a_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl_runl_diff_num_r_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL;
    d.bitc.runl_diff_num_r_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl_runl_diff_num_r_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl_t *)REG_ARIA_DISP_OSDD_OSD1_CTL).bitc.runl_diff_num_r_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl_runl_diff_num_g_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL;
    d.bitc.runl_diff_num_g_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl_runl_diff_num_g_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl_t *)REG_ARIA_DISP_OSDD_OSD1_CTL).bitc.runl_diff_num_g_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl_runl_diff_num_b_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL;
    d.bitc.runl_diff_num_b_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl_runl_diff_num_b_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl_t *)REG_ARIA_DISP_OSDD_OSD1_CTL).bitc.runl_diff_num_b_osd1;
}


/*!
  register ARIA_DISP_osdd_osd1_ctl2 (read/write)
  */
void reg_aria_disp_set_osdd_osd1_ctl2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, data);
}

mt_u32  reg_aria_disp_get_osdd_osd1_ctl2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2);
}

void reg_aria_disp_set_osdd_osd1_ctl2_a_osdd_arith_adapt_en_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.a_osdd_arith_adapt_en_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_a_osdd_arith_adapt_en_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.a_osdd_arith_adapt_en_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_r_osdd_arith_adapt_en_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.r_osdd_arith_adapt_en_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_r_osdd_arith_adapt_en_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.r_osdd_arith_adapt_en_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_g_osdd_arith_adapt_en_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.g_osdd_arith_adapt_en_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_g_osdd_arith_adapt_en_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.g_osdd_arith_adapt_en_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_b_osdd_arith_adapt_en_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.b_osdd_arith_adapt_en_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_b_osdd_arith_adapt_en_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.b_osdd_arith_adapt_en_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_lossy_a_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.lossy_a_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_lossy_a_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.lossy_a_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_lossy_r_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.lossy_r_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_lossy_r_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.lossy_r_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_lossy_g_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.lossy_g_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_lossy_g_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.lossy_g_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_lossy_b_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.lossy_b_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_lossy_b_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.lossy_b_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_a_osdd_init_arith_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.a_osdd_init_arith_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_a_osdd_init_arith_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.a_osdd_init_arith_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_r_osdd_init_arith_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.r_osdd_init_arith_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_r_osdd_init_arith_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.r_osdd_init_arith_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_g_osdd_init_arith_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.g_osdd_init_arith_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_g_osdd_init_arith_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.g_osdd_init_arith_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_b_osdd_init_arith_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.b_osdd_init_arith_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_b_osdd_init_arith_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.b_osdd_init_arith_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_a_dpcm_quanmode_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.a_dpcm_quanmode_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_a_dpcm_quanmode_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.a_dpcm_quanmode_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_r_dpcm_quanmode_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.r_dpcm_quanmode_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_r_dpcm_quanmode_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.r_dpcm_quanmode_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_g_dpcm_quanmode_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.g_dpcm_quanmode_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_g_dpcm_quanmode_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.g_dpcm_quanmode_osd1;
}

void reg_aria_disp_set_osdd_osd1_ctl2_b_dpcm_quanmode_osd1(mt_u8 data)
{
    reg_aria_disp_osdd_osd1_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_OSD1_CTL2;
    d.bitc.b_dpcm_quanmode_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_OSD1_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_osd1_ctl2_b_dpcm_quanmode_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_osd1_ctl2_t *)REG_ARIA_DISP_OSDD_OSD1_CTL2).bitc.b_dpcm_quanmode_osd1;
}


/*!
  register ARIA_DISP_osdd_sub_cmd (read/write)
  */
void reg_aria_disp_set_osdd_sub_cmd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CMD, data);
}

mt_u32  reg_aria_disp_get_osdd_sub_cmd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CMD);
}

void reg_aria_disp_set_osdd_sub_cmd_compress_en_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CMD;
    d.bitc.compress_en_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_cmd_compress_en_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_cmd_t *)REG_ARIA_DISP_OSDD_SUB_CMD).bitc.compress_en_sub;
}


/*!
  register ARIA_DISP_osdd_sub_length_a (read/write)
  */
void reg_aria_disp_set_osdd_sub_length_a(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_LENGTH_A, data);
}

mt_u32  reg_aria_disp_get_osdd_sub_length_a(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_LENGTH_A);
}

void reg_aria_disp_set_osdd_sub_length_a_a_compress_bits_osd1(mt_u32 data)
{
    reg_aria_disp_osdd_sub_length_a_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_LENGTH_A;
    d.bitc.a_compress_bits_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_LENGTH_A, d.all);
}

mt_u32  reg_aria_disp_get_osdd_sub_length_a_a_compress_bits_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_length_a_t *)REG_ARIA_DISP_OSDD_SUB_LENGTH_A).bitc.a_compress_bits_osd1;
}


/*!
  register ARIA_DISP_osdd_sub_length_r (read/write)
  */
void reg_aria_disp_set_osdd_sub_length_r(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_LENGTH_R, data);
}

mt_u32  reg_aria_disp_get_osdd_sub_length_r(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_LENGTH_R);
}

void reg_aria_disp_set_osdd_sub_length_r_r_compress_bits_osd1(mt_u32 data)
{
    reg_aria_disp_osdd_sub_length_r_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_LENGTH_R;
    d.bitc.r_compress_bits_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_LENGTH_R, d.all);
}

mt_u32  reg_aria_disp_get_osdd_sub_length_r_r_compress_bits_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_length_r_t *)REG_ARIA_DISP_OSDD_SUB_LENGTH_R).bitc.r_compress_bits_osd1;
}


/*!
  register ARIA_DISP_osdd_sub_lenght_g (read/write)
  */
void reg_aria_disp_set_osdd_sub_lenght_g(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_LENGHT_G, data);
}

mt_u32  reg_aria_disp_get_osdd_sub_lenght_g(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_LENGHT_G);
}

void reg_aria_disp_set_osdd_sub_lenght_g_g_compress_bits_osd1(mt_u32 data)
{
    reg_aria_disp_osdd_sub_lenght_g_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_LENGHT_G;
    d.bitc.g_compress_bits_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_LENGHT_G, d.all);
}

mt_u32  reg_aria_disp_get_osdd_sub_lenght_g_g_compress_bits_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_lenght_g_t *)REG_ARIA_DISP_OSDD_SUB_LENGHT_G).bitc.g_compress_bits_osd1;
}


/*!
  register ARIA_DISP_osdd_sub_length_b (read/write)
  */
void reg_aria_disp_set_osdd_sub_length_b(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_LENGTH_B, data);
}

mt_u32  reg_aria_disp_get_osdd_sub_length_b(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_LENGTH_B);
}

void reg_aria_disp_set_osdd_sub_length_b_b_compress_bits_osd1(mt_u32 data)
{
    reg_aria_disp_osdd_sub_length_b_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_LENGTH_B;
    d.bitc.b_compress_bits_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_LENGTH_B, d.all);
}

mt_u32  reg_aria_disp_get_osdd_sub_length_b_b_compress_bits_osd1(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_length_b_t *)REG_ARIA_DISP_OSDD_SUB_LENGTH_B).bitc.b_compress_bits_osd1;
}


/*!
  register ARIA_DISP_osdd_sub_addr_a (read/write)
  */
void reg_aria_disp_set_osdd_sub_addr_a(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_ADDR_A, data);
}

mt_u32  reg_aria_disp_get_osdd_sub_addr_a(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_ADDR_A);
}

void reg_aria_disp_set_osdd_sub_addr_a_ff0_ddr_addr_base_sub(mt_u32 data)
{
    reg_aria_disp_osdd_sub_addr_a_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_ADDR_A;
    d.bitc.ff0_ddr_addr_base_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_ADDR_A, d.all);
}

mt_u32  reg_aria_disp_get_osdd_sub_addr_a_ff0_ddr_addr_base_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_addr_a_t *)REG_ARIA_DISP_OSDD_SUB_ADDR_A).bitc.ff0_ddr_addr_base_sub;
}


/*!
  register ARIA_DISP_osdd_sub_addr_r (read/write)
  */
void reg_aria_disp_set_osdd_sub_addr_r(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_ADDR_R, data);
}

mt_u32  reg_aria_disp_get_osdd_sub_addr_r(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_ADDR_R);
}

void reg_aria_disp_set_osdd_sub_addr_r_ff1_ddr_addr_base_sub(mt_u32 data)
{
    reg_aria_disp_osdd_sub_addr_r_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_ADDR_R;
    d.bitc.ff1_ddr_addr_base_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_ADDR_R, d.all);
}

mt_u32  reg_aria_disp_get_osdd_sub_addr_r_ff1_ddr_addr_base_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_addr_r_t *)REG_ARIA_DISP_OSDD_SUB_ADDR_R).bitc.ff1_ddr_addr_base_sub;
}


/*!
  register ARIA_DISP_osdd_sub_addr_g (read/write)
  */
void reg_aria_disp_set_osdd_sub_addr_g(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_ADDR_G, data);
}

mt_u32  reg_aria_disp_get_osdd_sub_addr_g(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_ADDR_G);
}

void reg_aria_disp_set_osdd_sub_addr_g_ff2_ddr_addr_base_sub(mt_u32 data)
{
    reg_aria_disp_osdd_sub_addr_g_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_ADDR_G;
    d.bitc.ff2_ddr_addr_base_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_ADDR_G, d.all);
}

mt_u32  reg_aria_disp_get_osdd_sub_addr_g_ff2_ddr_addr_base_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_addr_g_t *)REG_ARIA_DISP_OSDD_SUB_ADDR_G).bitc.ff2_ddr_addr_base_sub;
}


/*!
  register ARIA_DISP_osdd_sub_addr_b (read/write)
  */
void reg_aria_disp_set_osdd_sub_addr_b(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_ADDR_B, data);
}

mt_u32  reg_aria_disp_get_osdd_sub_addr_b(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_ADDR_B);
}

void reg_aria_disp_set_osdd_sub_addr_b_ff3_ddr_addr_base_sub(mt_u32 data)
{
    reg_aria_disp_osdd_sub_addr_b_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_ADDR_B;
    d.bitc.ff3_ddr_addr_base_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_ADDR_B, d.all);
}

mt_u32  reg_aria_disp_get_osdd_sub_addr_b_ff3_ddr_addr_base_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_addr_b_t *)REG_ARIA_DISP_OSDD_SUB_ADDR_B).bitc.ff3_ddr_addr_base_sub;
}


/*!
  register ARIA_DISP_osdd_sub_ctl (read/write)
  */
void reg_aria_disp_set_osdd_sub_ctl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL, data);
}

mt_u32  reg_aria_disp_get_osdd_sub_ctl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL);
}

void reg_aria_disp_set_osdd_sub_ctl_runl_diff_num_a_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL;
    d.bitc.runl_diff_num_a_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl_runl_diff_num_a_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl_t *)REG_ARIA_DISP_OSDD_SUB_CTL).bitc.runl_diff_num_a_sub;
}

void reg_aria_disp_set_osdd_sub_ctl_runl_diff_num_r_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL;
    d.bitc.runl_diff_num_r_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl_runl_diff_num_r_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl_t *)REG_ARIA_DISP_OSDD_SUB_CTL).bitc.runl_diff_num_r_sub;
}

void reg_aria_disp_set_osdd_sub_ctl_runl_diff_num_g_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL;
    d.bitc.runl_diff_num_g_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl_runl_diff_num_g_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl_t *)REG_ARIA_DISP_OSDD_SUB_CTL).bitc.runl_diff_num_g_sub;
}

void reg_aria_disp_set_osdd_sub_ctl_runl_diff_num_b_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL;
    d.bitc.runl_diff_num_b_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl_runl_diff_num_b_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl_t *)REG_ARIA_DISP_OSDD_SUB_CTL).bitc.runl_diff_num_b_sub;
}


/*!
  register ARIA_DISP_osdd_sub_ctl2 (read/write)
  */
void reg_aria_disp_set_osdd_sub_ctl2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, data);
}

mt_u32  reg_aria_disp_get_osdd_sub_ctl2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2);
}

void reg_aria_disp_set_osdd_sub_ctl2_a_osdd_arith_adapt_en_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.a_osdd_arith_adapt_en_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_a_osdd_arith_adapt_en_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.a_osdd_arith_adapt_en_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_r_osdd_arith_adapt_en_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.r_osdd_arith_adapt_en_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_r_osdd_arith_adapt_en_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.r_osdd_arith_adapt_en_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_g_osdd_arith_adapt_en_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.g_osdd_arith_adapt_en_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_g_osdd_arith_adapt_en_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.g_osdd_arith_adapt_en_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_b_osdd_arith_adapt_en_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.b_osdd_arith_adapt_en_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_b_osdd_arith_adapt_en_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.b_osdd_arith_adapt_en_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_lossy_a_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.lossy_a_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_lossy_a_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.lossy_a_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_lossy_r_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.lossy_r_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_lossy_r_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.lossy_r_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_lossy_g_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.lossy_g_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_lossy_g_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.lossy_g_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_lossy_b_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.lossy_b_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_lossy_b_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.lossy_b_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_a_osdd_init_arith_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.a_osdd_init_arith_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_a_osdd_init_arith_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.a_osdd_init_arith_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_r_osdd_init_arith_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.r_osdd_init_arith_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_r_osdd_init_arith_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.r_osdd_init_arith_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_g_osdd_init_arith_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.g_osdd_init_arith_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_g_osdd_init_arith_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.g_osdd_init_arith_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_b_osdd_init_arith_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.b_osdd_init_arith_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_b_osdd_init_arith_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.b_osdd_init_arith_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_a_dpcm_quanmode_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.a_dpcm_quanmode_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_a_dpcm_quanmode_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.a_dpcm_quanmode_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_r_dpcm_quanmode_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.r_dpcm_quanmode_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_r_dpcm_quanmode_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.r_dpcm_quanmode_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_g_dpcm_quanmode_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.g_dpcm_quanmode_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_g_dpcm_quanmode_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.g_dpcm_quanmode_sub;
}

void reg_aria_disp_set_osdd_sub_ctl2_b_dpcm_quanmode_sub(mt_u8 data)
{
    reg_aria_disp_osdd_sub_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDD_SUB_CTL2;
    d.bitc.b_dpcm_quanmode_sub = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDD_SUB_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdd_sub_ctl2_b_dpcm_quanmode_sub(void)
{
    return (*(volatile reg_aria_disp_osdd_sub_ctl2_t *)REG_ARIA_DISP_OSDD_SUB_CTL2).bitc.b_dpcm_quanmode_sub;
}


/*!
  register ARIA_DISP_osdc_cmd (read/write)
  */
void reg_aria_disp_set_osdc_cmd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD, data);
}

mt_u32  reg_aria_disp_get_osdc_cmd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD);
}

void reg_aria_disp_set_osdc_cmd_osdc_osd_width(mt_u16 data)
{
    reg_aria_disp_osdc_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD;
    d.bitc.osdc_osd_width = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD, d.all);
}

mt_u16  reg_aria_disp_get_osdc_cmd_osdc_osd_width(void)
{
    return (*(volatile reg_aria_disp_osdc_cmd_t *)REG_ARIA_DISP_OSDC_CMD).bitc.osdc_osd_width;
}

void reg_aria_disp_set_osdc_cmd_osdc_endian(mt_u8 data)
{
    reg_aria_disp_osdc_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD;
    d.bitc.osdc_endian = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdc_cmd_osdc_endian(void)
{
    return (*(volatile reg_aria_disp_osdc_cmd_t *)REG_ARIA_DISP_OSDC_CMD).bitc.osdc_endian;
}

void reg_aria_disp_set_osdc_cmd_osdc_osd_height(mt_u16 data)
{
    reg_aria_disp_osdc_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD;
    d.bitc.osdc_osd_height = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD, d.all);
}

mt_u16  reg_aria_disp_get_osdc_cmd_osdc_osd_height(void)
{
    return (*(volatile reg_aria_disp_osdc_cmd_t *)REG_ARIA_DISP_OSDC_CMD).bitc.osdc_osd_height;
}

void reg_aria_disp_set_osdc_cmd_osdc_pre_judge(mt_u8 data)
{
    reg_aria_disp_osdc_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD;
    d.bitc.osdc_pre_judge = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdc_cmd_osdc_pre_judge(void)
{
    return (*(volatile reg_aria_disp_osdc_cmd_t *)REG_ARIA_DISP_OSDC_CMD).bitc.osdc_pre_judge;
}

void reg_aria_disp_set_osdc_cmd_osdc_osdcomp_start(mt_u8 data)
{
    reg_aria_disp_osdc_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD;
    d.bitc.osdc_osdcomp_start = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD, d.all);
}

mt_u8   reg_aria_disp_get_osdc_cmd_osdc_osdcomp_start(void)
{
    return (*(volatile reg_aria_disp_osdc_cmd_t *)REG_ARIA_DISP_OSDC_CMD).bitc.osdc_osdcomp_start;
}


/*!
  register ARIA_DISP_osdc_rst (read/write)
  */
void reg_aria_disp_set_osdc_rst(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_RST, data);
}

mt_u32  reg_aria_disp_get_osdc_rst(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_RST);
}

void reg_aria_disp_set_osdc_rst_osdc_rst_h(mt_u8 data)
{
    reg_aria_disp_osdc_rst_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_RST;
    d.bitc.osdc_rst_h = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_RST, d.all);
}

mt_u8   reg_aria_disp_get_osdc_rst_osdc_rst_h(void)
{
    return (*(volatile reg_aria_disp_osdc_rst_t *)REG_ARIA_DISP_OSDC_RST).bitc.osdc_rst_h;
}

void reg_aria_disp_set_osdc_rst_osdc_terminate(mt_u8 data)
{
    reg_aria_disp_osdc_rst_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_RST;
    d.bitc.osdc_terminate = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_RST, d.all);
}

mt_u8   reg_aria_disp_get_osdc_rst_osdc_terminate(void)
{
    return (*(volatile reg_aria_disp_osdc_rst_t *)REG_ARIA_DISP_OSDC_RST).bitc.osdc_terminate;
}

void reg_aria_disp_set_osdc_rst_osdc_monitor_reload(mt_u8 data)
{
    reg_aria_disp_osdc_rst_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_RST;
    d.bitc.osdc_monitor_reload = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_RST, d.all);
}

mt_u8   reg_aria_disp_get_osdc_rst_osdc_monitor_reload(void)
{
    return (*(volatile reg_aria_disp_osdc_rst_t *)REG_ARIA_DISP_OSDC_RST).bitc.osdc_monitor_reload;
}

void reg_aria_disp_set_osdc_rst_osdc_rdndt_latch_sdb(mt_u8 data)
{
    reg_aria_disp_osdc_rst_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_RST;
    d.bitc.osdc_rdndt_latch_sdb = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_RST, d.all);
}

mt_u8   reg_aria_disp_get_osdc_rst_osdc_rdndt_latch_sdb(void)
{
    return (*(volatile reg_aria_disp_osdc_rst_t *)REG_ARIA_DISP_OSDC_RST).bitc.osdc_rdndt_latch_sdb;
}

void reg_aria_disp_set_osdc_rst_osdc_rdndt_latch_osd(mt_u8 data)
{
    reg_aria_disp_osdc_rst_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_RST;
    d.bitc.osdc_rdndt_latch_osd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_RST, d.all);
}

mt_u8   reg_aria_disp_get_osdc_rst_osdc_rdndt_latch_osd(void)
{
    return (*(volatile reg_aria_disp_osdc_rst_t *)REG_ARIA_DISP_OSDC_RST).bitc.osdc_rdndt_latch_osd;
}

void reg_aria_disp_set_osdc_rst_osdc_rdndt_latch_pre(mt_u8 data)
{
    reg_aria_disp_osdc_rst_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_RST;
    d.bitc.osdc_rdndt_latch_pre = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_RST, d.all);
}

mt_u8   reg_aria_disp_get_osdc_rst_osdc_rdndt_latch_pre(void)
{
    return (*(volatile reg_aria_disp_osdc_rst_t *)REG_ARIA_DISP_OSDC_RST).bitc.osdc_rdndt_latch_pre;
}

void reg_aria_disp_set_osdc_rst_osdc_rdndt_latch_sti(mt_u8 data)
{
    reg_aria_disp_osdc_rst_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_RST;
    d.bitc.osdc_rdndt_latch_sti = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_RST, d.all);
}

mt_u8   reg_aria_disp_get_osdc_rst_osdc_rdndt_latch_sti(void)
{
    return (*(volatile reg_aria_disp_osdc_rst_t *)REG_ARIA_DISP_OSDC_RST).bitc.osdc_rdndt_latch_sti;
}

void reg_aria_disp_set_osdc_rst_osdc_axi_w_limit(mt_u8 data)
{
    reg_aria_disp_osdc_rst_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_RST;
    d.bitc.osdc_axi_w_limit = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_RST, d.all);
}

mt_u8   reg_aria_disp_get_osdc_rst_osdc_axi_w_limit(void)
{
    return (*(volatile reg_aria_disp_osdc_rst_t *)REG_ARIA_DISP_OSDC_RST).bitc.osdc_axi_w_limit;
}

void reg_aria_disp_set_osdc_rst_osdc_axi_r_limit(mt_u8 data)
{
    reg_aria_disp_osdc_rst_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_RST;
    d.bitc.osdc_axi_r_limit = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_RST, d.all);
}

mt_u8   reg_aria_disp_get_osdc_rst_osdc_axi_r_limit(void)
{
    return (*(volatile reg_aria_disp_osdc_rst_t *)REG_ARIA_DISP_OSDC_RST).bitc.osdc_axi_r_limit;
}


/*!
  register ARIA_DISP_osdc_ctl (read/write)
  */
void reg_aria_disp_set_osdc_ctl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL, data);
}

mt_u32  reg_aria_disp_get_osdc_ctl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL);
}

void reg_aria_disp_set_osdc_ctl_osdc_a_value_diff_num(mt_u8 data)
{
    reg_aria_disp_osdc_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL;
    d.bitc.osdc_a_value_diff_num = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl_osdc_a_value_diff_num(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl_t *)REG_ARIA_DISP_OSDC_CTL).bitc.osdc_a_value_diff_num;
}

void reg_aria_disp_set_osdc_ctl_osdc_r_value_diff_num(mt_u8 data)
{
    reg_aria_disp_osdc_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL;
    d.bitc.osdc_r_value_diff_num = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl_osdc_r_value_diff_num(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl_t *)REG_ARIA_DISP_OSDC_CTL).bitc.osdc_r_value_diff_num;
}

void reg_aria_disp_set_osdc_ctl_osdc_g_value_diff_num(mt_u8 data)
{
    reg_aria_disp_osdc_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL;
    d.bitc.osdc_g_value_diff_num = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl_osdc_g_value_diff_num(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl_t *)REG_ARIA_DISP_OSDC_CTL).bitc.osdc_g_value_diff_num;
}

void reg_aria_disp_set_osdc_ctl_osdc_b_value_diff_num(mt_u8 data)
{
    reg_aria_disp_osdc_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL;
    d.bitc.osdc_b_value_diff_num = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl_osdc_b_value_diff_num(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl_t *)REG_ARIA_DISP_OSDC_CTL).bitc.osdc_b_value_diff_num;
}

void reg_aria_disp_set_osdc_ctl_osdc_a_dn_same_num(mt_u8 data)
{
    reg_aria_disp_osdc_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL;
    d.bitc.osdc_a_dn_same_num = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl_osdc_a_dn_same_num(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl_t *)REG_ARIA_DISP_OSDC_CTL).bitc.osdc_a_dn_same_num;
}

void reg_aria_disp_set_osdc_ctl_osdc_r_dn_same_num(mt_u8 data)
{
    reg_aria_disp_osdc_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL;
    d.bitc.osdc_r_dn_same_num = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl_osdc_r_dn_same_num(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl_t *)REG_ARIA_DISP_OSDC_CTL).bitc.osdc_r_dn_same_num;
}

void reg_aria_disp_set_osdc_ctl_osdc_g_dn_same_num(mt_u8 data)
{
    reg_aria_disp_osdc_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL;
    d.bitc.osdc_g_dn_same_num = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl_osdc_g_dn_same_num(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl_t *)REG_ARIA_DISP_OSDC_CTL).bitc.osdc_g_dn_same_num;
}

void reg_aria_disp_set_osdc_ctl_osdc_b_dn_same_num(mt_u8 data)
{
    reg_aria_disp_osdc_ctl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL;
    d.bitc.osdc_b_dn_same_num = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl_osdc_b_dn_same_num(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl_t *)REG_ARIA_DISP_OSDC_CTL).bitc.osdc_b_dn_same_num;
}


/*!
  register ARIA_DISP_osdc_ctl2 (read/write)
  */
void reg_aria_disp_set_osdc_ctl2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, data);
}

mt_u32  reg_aria_disp_get_osdc_ctl2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2);
}

void reg_aria_disp_set_osdc_ctl2_osdc_a_arith_adapt_en(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_a_arith_adapt_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_a_arith_adapt_en(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_a_arith_adapt_en;
}

void reg_aria_disp_set_osdc_ctl2_osdc_r_arith_adapt_en(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_r_arith_adapt_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_r_arith_adapt_en(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_r_arith_adapt_en;
}

void reg_aria_disp_set_osdc_ctl2_osdc_g_arith_adapt_en(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_g_arith_adapt_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_g_arith_adapt_en(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_g_arith_adapt_en;
}

void reg_aria_disp_set_osdc_ctl2_osdc_b_arith_adapt_en(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_b_arith_adapt_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_b_arith_adapt_en(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_b_arith_adapt_en;
}

void reg_aria_disp_set_osdc_ctl2_osdc_a_lossy(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_a_lossy = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_a_lossy(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_a_lossy;
}

void reg_aria_disp_set_osdc_ctl2_osdc_r_lossy(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_r_lossy = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_r_lossy(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_r_lossy;
}

void reg_aria_disp_set_osdc_ctl2_osdc_g_lossy(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_g_lossy = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_g_lossy(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_g_lossy;
}

void reg_aria_disp_set_osdc_ctl2_osdc_b_lossy(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_b_lossy = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_b_lossy(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_b_lossy;
}

void reg_aria_disp_set_osdc_ctl2_osdc_a_init_arith(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_a_init_arith = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_a_init_arith(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_a_init_arith;
}

void reg_aria_disp_set_osdc_ctl2_osdc_r_init_arith(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_r_init_arith = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_r_init_arith(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_r_init_arith;
}

void reg_aria_disp_set_osdc_ctl2_osdc_g_init_arith(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_g_init_arith = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_g_init_arith(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_g_init_arith;
}

void reg_aria_disp_set_osdc_ctl2_osdc_b_init_arith(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_b_init_arith = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_b_init_arith(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_b_init_arith;
}

void reg_aria_disp_set_osdc_ctl2_osdc_a_dpcm_quanmode(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_a_dpcm_quanmode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_a_dpcm_quanmode(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_a_dpcm_quanmode;
}

void reg_aria_disp_set_osdc_ctl2_osdc_r_dpcm_quanmode(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_r_dpcm_quanmode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_r_dpcm_quanmode(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_r_dpcm_quanmode;
}

void reg_aria_disp_set_osdc_ctl2_osdc_g_dpcm_quanmode(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_g_dpcm_quanmode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_g_dpcm_quanmode(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_g_dpcm_quanmode;
}

void reg_aria_disp_set_osdc_ctl2_osdc_b_dpcm_quanmode(mt_u8 data)
{
    reg_aria_disp_osdc_ctl2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CTL2;
    d.bitc.osdc_b_dpcm_quanmode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CTL2, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ctl2_osdc_b_dpcm_quanmode(void)
{
    return (*(volatile reg_aria_disp_osdc_ctl2_t *)REG_ARIA_DISP_OSDC_CTL2).bitc.osdc_b_dpcm_quanmode;
}


/*!
  register ARIA_DISP_osdc_ffrd_threshold (read/write)
  */
void reg_aria_disp_set_osdc_ffrd_threshold(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_FFRD_THRESHOLD, data);
}

mt_u32  reg_aria_disp_get_osdc_ffrd_threshold(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_FFRD_THRESHOLD);
}

void reg_aria_disp_set_osdc_ffrd_threshold_osdc_urgent_num_ddr_rd(mt_u8 data)
{
    reg_aria_disp_osdc_ffrd_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_FFRD_THRESHOLD;
    d.bitc.osdc_urgent_num_ddr_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_FFRD_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ffrd_threshold_osdc_urgent_num_ddr_rd(void)
{
    return (*(volatile reg_aria_disp_osdc_ffrd_threshold_t *)REG_ARIA_DISP_OSDC_FFRD_THRESHOLD).bitc.osdc_urgent_num_ddr_rd;
}

void reg_aria_disp_set_osdc_ffrd_threshold_osdc_weight_num_ddr_rd(mt_u8 data)
{
    reg_aria_disp_osdc_ffrd_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_FFRD_THRESHOLD;
    d.bitc.osdc_weight_num_ddr_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_FFRD_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ffrd_threshold_osdc_weight_num_ddr_rd(void)
{
    return (*(volatile reg_aria_disp_osdc_ffrd_threshold_t *)REG_ARIA_DISP_OSDC_FFRD_THRESHOLD).bitc.osdc_weight_num_ddr_rd;
}


/*!
  register ARIA_DISP_osdc_ffwr_threshold (read/write)
  */
void reg_aria_disp_set_osdc_ffwr_threshold(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD, data);
}

mt_u32  reg_aria_disp_get_osdc_ffwr_threshold(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD);
}

void reg_aria_disp_set_osdc_ffwr_threshold_osdc_urgent_num_ddr_wr0(mt_u8 data)
{
    reg_aria_disp_osdc_ffwr_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD;
    d.bitc.osdc_urgent_num_ddr_wr0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ffwr_threshold_osdc_urgent_num_ddr_wr0(void)
{
    return (*(volatile reg_aria_disp_osdc_ffwr_threshold_t *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD).bitc.osdc_urgent_num_ddr_wr0;
}

void reg_aria_disp_set_osdc_ffwr_threshold_osdc_weight_num_ddr_wr0(mt_u8 data)
{
    reg_aria_disp_osdc_ffwr_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD;
    d.bitc.osdc_weight_num_ddr_wr0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ffwr_threshold_osdc_weight_num_ddr_wr0(void)
{
    return (*(volatile reg_aria_disp_osdc_ffwr_threshold_t *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD).bitc.osdc_weight_num_ddr_wr0;
}

void reg_aria_disp_set_osdc_ffwr_threshold_osdc_urgent_num_ddr_wr1(mt_u8 data)
{
    reg_aria_disp_osdc_ffwr_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD;
    d.bitc.osdc_urgent_num_ddr_wr1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ffwr_threshold_osdc_urgent_num_ddr_wr1(void)
{
    return (*(volatile reg_aria_disp_osdc_ffwr_threshold_t *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD).bitc.osdc_urgent_num_ddr_wr1;
}

void reg_aria_disp_set_osdc_ffwr_threshold_osdc_weight_num_ddr_wr1(mt_u8 data)
{
    reg_aria_disp_osdc_ffwr_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD;
    d.bitc.osdc_weight_num_ddr_wr1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ffwr_threshold_osdc_weight_num_ddr_wr1(void)
{
    return (*(volatile reg_aria_disp_osdc_ffwr_threshold_t *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD).bitc.osdc_weight_num_ddr_wr1;
}

void reg_aria_disp_set_osdc_ffwr_threshold_osdc_urgent_num_ddr_wr2(mt_u8 data)
{
    reg_aria_disp_osdc_ffwr_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD;
    d.bitc.osdc_urgent_num_ddr_wr2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ffwr_threshold_osdc_urgent_num_ddr_wr2(void)
{
    return (*(volatile reg_aria_disp_osdc_ffwr_threshold_t *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD).bitc.osdc_urgent_num_ddr_wr2;
}

void reg_aria_disp_set_osdc_ffwr_threshold_osdc_weight_num_ddr_wr2(mt_u8 data)
{
    reg_aria_disp_osdc_ffwr_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD;
    d.bitc.osdc_weight_num_ddr_wr2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ffwr_threshold_osdc_weight_num_ddr_wr2(void)
{
    return (*(volatile reg_aria_disp_osdc_ffwr_threshold_t *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD).bitc.osdc_weight_num_ddr_wr2;
}

void reg_aria_disp_set_osdc_ffwr_threshold_osdc_urgent_num_ddr_wr3(mt_u8 data)
{
    reg_aria_disp_osdc_ffwr_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD;
    d.bitc.osdc_urgent_num_ddr_wr3 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ffwr_threshold_osdc_urgent_num_ddr_wr3(void)
{
    return (*(volatile reg_aria_disp_osdc_ffwr_threshold_t *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD).bitc.osdc_urgent_num_ddr_wr3;
}

void reg_aria_disp_set_osdc_ffwr_threshold_osdc_weight_num_ddr_wr3(mt_u8 data)
{
    reg_aria_disp_osdc_ffwr_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD;
    d.bitc.osdc_weight_num_ddr_wr3 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_osdc_ffwr_threshold_osdc_weight_num_ddr_wr3(void)
{
    return (*(volatile reg_aria_disp_osdc_ffwr_threshold_t *)REG_ARIA_DISP_OSDC_FFWR_THRESHOLD).bitc.osdc_weight_num_ddr_wr3;
}


/*!
  register ARIA_DISP_osdc_ddr_rd_addr (read/write)
  */
void reg_aria_disp_set_osdc_ddr_rd_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DDR_RD_ADDR, data);
}

mt_u32  reg_aria_disp_get_osdc_ddr_rd_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DDR_RD_ADDR);
}

void reg_aria_disp_set_osdc_ddr_rd_addr_osdc_ddr_rd_base_addr(mt_u32 data)
{
    reg_aria_disp_osdc_ddr_rd_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DDR_RD_ADDR;
    d.bitc.osdc_ddr_rd_base_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DDR_RD_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_osdc_ddr_rd_addr_osdc_ddr_rd_base_addr(void)
{
    return (*(volatile reg_aria_disp_osdc_ddr_rd_addr_t *)REG_ARIA_DISP_OSDC_DDR_RD_ADDR).bitc.osdc_ddr_rd_base_addr;
}


/*!
  register ARIA_DISP_osdc_width_stride (read/write)
  */
void reg_aria_disp_set_osdc_width_stride(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_WIDTH_STRIDE, data);
}

mt_u32  reg_aria_disp_get_osdc_width_stride(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_WIDTH_STRIDE);
}

void reg_aria_disp_set_osdc_width_stride_osdc_osd_width_stride(mt_u16 data)
{
    reg_aria_disp_osdc_width_stride_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_WIDTH_STRIDE;
    d.bitc.osdc_osd_width_stride = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_WIDTH_STRIDE, d.all);
}

mt_u16  reg_aria_disp_get_osdc_width_stride_osdc_osd_width_stride(void)
{
    return (*(volatile reg_aria_disp_osdc_width_stride_t *)REG_ARIA_DISP_OSDC_WIDTH_STRIDE).bitc.osdc_osd_width_stride;
}


/*!
  register ARIA_DISP_osdc_ddr_wr_addr_a (read/write)
  */
void reg_aria_disp_set_osdc_ddr_wr_addr_a(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_A, data);
}

mt_u32  reg_aria_disp_get_osdc_ddr_wr_addr_a(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_A);
}

void reg_aria_disp_set_osdc_ddr_wr_addr_a_osdc_ddr_wr_base_addr0(mt_u32 data)
{
    reg_aria_disp_osdc_ddr_wr_addr_a_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_A;
    d.bitc.osdc_ddr_wr_base_addr0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_A, d.all);
}

mt_u32  reg_aria_disp_get_osdc_ddr_wr_addr_a_osdc_ddr_wr_base_addr0(void)
{
    return (*(volatile reg_aria_disp_osdc_ddr_wr_addr_a_t *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_A).bitc.osdc_ddr_wr_base_addr0;
}


/*!
  register ARIA_DISP_osdc_ddr_wr_addr_r (read/write)
  */
void reg_aria_disp_set_osdc_ddr_wr_addr_r(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_R, data);
}

mt_u32  reg_aria_disp_get_osdc_ddr_wr_addr_r(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_R);
}

void reg_aria_disp_set_osdc_ddr_wr_addr_r_osdc_ddr_wr_base_addr1(mt_u32 data)
{
    reg_aria_disp_osdc_ddr_wr_addr_r_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_R;
    d.bitc.osdc_ddr_wr_base_addr1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_R, d.all);
}

mt_u32  reg_aria_disp_get_osdc_ddr_wr_addr_r_osdc_ddr_wr_base_addr1(void)
{
    return (*(volatile reg_aria_disp_osdc_ddr_wr_addr_r_t *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_R).bitc.osdc_ddr_wr_base_addr1;
}


/*!
  register ARIA_DISP_osdc_ddr_wr_addr_g (read/write)
  */
void reg_aria_disp_set_osdc_ddr_wr_addr_g(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_G, data);
}

mt_u32  reg_aria_disp_get_osdc_ddr_wr_addr_g(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_G);
}

void reg_aria_disp_set_osdc_ddr_wr_addr_g_osdc_ddr_wr_base_addr2(mt_u32 data)
{
    reg_aria_disp_osdc_ddr_wr_addr_g_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_G;
    d.bitc.osdc_ddr_wr_base_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_G, d.all);
}

mt_u32  reg_aria_disp_get_osdc_ddr_wr_addr_g_osdc_ddr_wr_base_addr2(void)
{
    return (*(volatile reg_aria_disp_osdc_ddr_wr_addr_g_t *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_G).bitc.osdc_ddr_wr_base_addr2;
}


/*!
  register ARIA_DISP_osdc_ddr_wr_addr_b (read/write)
  */
void reg_aria_disp_set_osdc_ddr_wr_addr_b(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_B, data);
}

mt_u32  reg_aria_disp_get_osdc_ddr_wr_addr_b(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_B);
}

void reg_aria_disp_set_osdc_ddr_wr_addr_b_osdc_ddr_wr_base_addr3(mt_u32 data)
{
    reg_aria_disp_osdc_ddr_wr_addr_b_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_B;
    d.bitc.osdc_ddr_wr_base_addr3 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_B, d.all);
}

mt_u32  reg_aria_disp_get_osdc_ddr_wr_addr_b_osdc_ddr_wr_base_addr3(void)
{
    return (*(volatile reg_aria_disp_osdc_ddr_wr_addr_b_t *)REG_ARIA_DISP_OSDC_DDR_WR_ADDR_B).bitc.osdc_ddr_wr_base_addr3;
}


/*!
  register ARIA_DISP_osdc_status (read/write)
  */
void reg_aria_disp_set_osdc_status(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, data);
}

mt_u32  reg_aria_disp_get_osdc_status(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS);
}

void reg_aria_disp_set_osdc_status_osdc_soft_rst_done(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_soft_rst_done = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_soft_rst_done(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_soft_rst_done;
}

void reg_aria_disp_set_osdc_status_osdc_terminate_done(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_terminate_done = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_terminate_done(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_terminate_done;
}

void reg_aria_disp_set_osdc_status_osdc_axi_w_done(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_axi_w_done = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_axi_w_done(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_axi_w_done;
}

void reg_aria_disp_set_osdc_status_osdc_axi_r_done(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_axi_r_done = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_axi_r_done(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_axi_r_done;
}

void reg_aria_disp_set_osdc_status_osdc_rdfifo_full(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_rdfifo_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_rdfifo_full(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_rdfifo_full;
}

void reg_aria_disp_set_osdc_status_osdc_rdfifo_empty(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_rdfifo_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_rdfifo_empty(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_rdfifo_empty;
}

void reg_aria_disp_set_osdc_status_osdc_wrfifo0_full(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_wrfifo0_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_wrfifo0_full(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_wrfifo0_full;
}

void reg_aria_disp_set_osdc_status_osdc_wrfifo0_empty(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_wrfifo0_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_wrfifo0_empty(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_wrfifo0_empty;
}

void reg_aria_disp_set_osdc_status_osdc_wrfifo1_full(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_wrfifo1_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_wrfifo1_full(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_wrfifo1_full;
}

void reg_aria_disp_set_osdc_status_osdc_wrfifo1_empty(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_wrfifo1_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_wrfifo1_empty(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_wrfifo1_empty;
}

void reg_aria_disp_set_osdc_status_osdc_wrfifo2_full(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_wrfifo2_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_wrfifo2_full(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_wrfifo2_full;
}

void reg_aria_disp_set_osdc_status_osdc_wrfifo2_empty(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_wrfifo2_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_wrfifo2_empty(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_wrfifo2_empty;
}

void reg_aria_disp_set_osdc_status_osdc_wrfifo3_full(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_wrfifo3_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_wrfifo3_full(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_wrfifo3_full;
}

void reg_aria_disp_set_osdc_status_osdc_wrfifo3_empty(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_wrfifo3_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_wrfifo3_empty(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_wrfifo3_empty;
}

void reg_aria_disp_set_osdc_status_osdc_wrddr_error0(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_wrddr_error0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_wrddr_error0(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_wrddr_error0;
}

void reg_aria_disp_set_osdc_status_osdc_wrddr_error1(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_wrddr_error1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_wrddr_error1(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_wrddr_error1;
}

void reg_aria_disp_set_osdc_status_osdc_wrddr_error2(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_wrddr_error2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_wrddr_error2(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_wrddr_error2;
}

void reg_aria_disp_set_osdc_status_osdc_wrddr_error3(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_wrddr_error3 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_wrddr_error3(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_wrddr_error3;
}

void reg_aria_disp_set_osdc_status_osdc_latency_overflow_rd(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_latency_overflow_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_latency_overflow_rd(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_latency_overflow_rd;
}

void reg_aria_disp_set_osdc_status_osdc_latency_overflow_wr(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_latency_overflow_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_latency_overflow_wr(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_latency_overflow_wr;
}

void reg_aria_disp_set_osdc_status_osdc_free(mt_u8 data)
{
    reg_aria_disp_osdc_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_STATUS;
    d.bitc.osdc_free = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_osdc_status_osdc_free(void)
{
    return (*(volatile reg_aria_disp_osdc_status_t *)REG_ARIA_DISP_OSDC_STATUS).bitc.osdc_free;
}


/*!
  register ARIA_DISP_osdc_irq_en (read/write)
  */
void reg_aria_disp_set_osdc_irq_en(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_IRQ_EN, data);
}

mt_u32  reg_aria_disp_get_osdc_irq_en(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_IRQ_EN);
}

void reg_aria_disp_set_osdc_irq_en_osdc_end_irq_en(mt_u8 data)
{
    reg_aria_disp_osdc_irq_en_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_IRQ_EN;
    d.bitc.osdc_end_irq_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_IRQ_EN, d.all);
}

mt_u8   reg_aria_disp_get_osdc_irq_en_osdc_end_irq_en(void)
{
    return (*(volatile reg_aria_disp_osdc_irq_en_t *)REG_ARIA_DISP_OSDC_IRQ_EN).bitc.osdc_end_irq_en;
}


/*!
  register ARIA_DISP_osdc_irq (read/write)
  */
void reg_aria_disp_set_osdc_irq(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_IRQ, data);
}

mt_u32  reg_aria_disp_get_osdc_irq(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_IRQ);
}

void reg_aria_disp_set_osdc_irq_osdc_end_irq(mt_u8 data)
{
    reg_aria_disp_osdc_irq_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_IRQ;
    d.bitc.osdc_end_irq = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_IRQ, d.all);
}

mt_u8   reg_aria_disp_get_osdc_irq_osdc_end_irq(void)
{
    return (*(volatile reg_aria_disp_osdc_irq_t *)REG_ARIA_DISP_OSDC_IRQ).bitc.osdc_end_irq;
}

void reg_aria_disp_set_osdc_irq_osdc_bits_exceed_irq(mt_u8 data)
{
    reg_aria_disp_osdc_irq_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_IRQ;
    d.bitc.osdc_bits_exceed_irq = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_IRQ, d.all);
}

mt_u8   reg_aria_disp_get_osdc_irq_osdc_bits_exceed_irq(void)
{
    return (*(volatile reg_aria_disp_osdc_irq_t *)REG_ARIA_DISP_OSDC_IRQ).bitc.osdc_bits_exceed_irq;
}

void reg_aria_disp_set_osdc_irq_osdc_terminate_irq(mt_u8 data)
{
    reg_aria_disp_osdc_irq_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_IRQ;
    d.bitc.osdc_terminate_irq = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_IRQ, d.all);
}

mt_u8   reg_aria_disp_get_osdc_irq_osdc_terminate_irq(void)
{
    return (*(volatile reg_aria_disp_osdc_irq_t *)REG_ARIA_DISP_OSDC_IRQ).bitc.osdc_terminate_irq;
}

void reg_aria_disp_set_osdc_irq_osdc_end0_irq(mt_u8 data)
{
    reg_aria_disp_osdc_irq_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_IRQ;
    d.bitc.osdc_end0_irq = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_IRQ, d.all);
}

mt_u8   reg_aria_disp_get_osdc_irq_osdc_end0_irq(void)
{
    return (*(volatile reg_aria_disp_osdc_irq_t *)REG_ARIA_DISP_OSDC_IRQ).bitc.osdc_end0_irq;
}

void reg_aria_disp_set_osdc_irq_osdc_end1_irq(mt_u8 data)
{
    reg_aria_disp_osdc_irq_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_IRQ;
    d.bitc.osdc_end1_irq = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_IRQ, d.all);
}

mt_u8   reg_aria_disp_get_osdc_irq_osdc_end1_irq(void)
{
    return (*(volatile reg_aria_disp_osdc_irq_t *)REG_ARIA_DISP_OSDC_IRQ).bitc.osdc_end1_irq;
}

void reg_aria_disp_set_osdc_irq_osdc_end2_irq(mt_u8 data)
{
    reg_aria_disp_osdc_irq_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_IRQ;
    d.bitc.osdc_end2_irq = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_IRQ, d.all);
}

mt_u8   reg_aria_disp_get_osdc_irq_osdc_end2_irq(void)
{
    return (*(volatile reg_aria_disp_osdc_irq_t *)REG_ARIA_DISP_OSDC_IRQ).bitc.osdc_end2_irq;
}

void reg_aria_disp_set_osdc_irq_osdc_end3_irq(mt_u8 data)
{
    reg_aria_disp_osdc_irq_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_IRQ;
    d.bitc.osdc_end3_irq = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_IRQ, d.all);
}

mt_u8   reg_aria_disp_get_osdc_irq_osdc_end3_irq(void)
{
    return (*(volatile reg_aria_disp_osdc_irq_t *)REG_ARIA_DISP_OSDC_IRQ).bitc.osdc_end3_irq;
}


/*!
  register ARIA_DISP_osdc_compress_bit_a (read/write)
  */
void reg_aria_disp_set_osdc_compress_bit_a(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_A, data);
}

mt_u32  reg_aria_disp_get_osdc_compress_bit_a(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_A);
}

void reg_aria_disp_set_osdc_compress_bit_a_osdc_compress_bit_a(mt_u32 data)
{
    reg_aria_disp_osdc_compress_bit_a_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_A;
    d.bitc.osdc_compress_bit_a = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_A, d.all);
}

mt_u32  reg_aria_disp_get_osdc_compress_bit_a_osdc_compress_bit_a(void)
{
    return (*(volatile reg_aria_disp_osdc_compress_bit_a_t *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_A).bitc.osdc_compress_bit_a;
}


/*!
  register ARIA_DISP_osdc_compress_bit_r (read/write)
  */
void reg_aria_disp_set_osdc_compress_bit_r(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_R, data);
}

mt_u32  reg_aria_disp_get_osdc_compress_bit_r(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_R);
}

void reg_aria_disp_set_osdc_compress_bit_r_osdc_compress_bit_r(mt_u32 data)
{
    reg_aria_disp_osdc_compress_bit_r_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_R;
    d.bitc.osdc_compress_bit_r = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_R, d.all);
}

mt_u32  reg_aria_disp_get_osdc_compress_bit_r_osdc_compress_bit_r(void)
{
    return (*(volatile reg_aria_disp_osdc_compress_bit_r_t *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_R).bitc.osdc_compress_bit_r;
}


/*!
  register ARIA_DISP_osdc_compress_bit_g (read/write)
  */
void reg_aria_disp_set_osdc_compress_bit_g(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_G, data);
}

mt_u32  reg_aria_disp_get_osdc_compress_bit_g(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_G);
}

void reg_aria_disp_set_osdc_compress_bit_g_osdc_compress_bit_g(mt_u32 data)
{
    reg_aria_disp_osdc_compress_bit_g_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_G;
    d.bitc.osdc_compress_bit_g = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_G, d.all);
}

mt_u32  reg_aria_disp_get_osdc_compress_bit_g_osdc_compress_bit_g(void)
{
    return (*(volatile reg_aria_disp_osdc_compress_bit_g_t *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_G).bitc.osdc_compress_bit_g;
}


/*!
  register ARIA_DISP_osdc_compress_bit_b (read/write)
  */
void reg_aria_disp_set_osdc_compress_bit_b(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_B, data);
}

mt_u32  reg_aria_disp_get_osdc_compress_bit_b(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_B);
}

void reg_aria_disp_set_osdc_compress_bit_b_osdc_compress_bit_b(mt_u32 data)
{
    reg_aria_disp_osdc_compress_bit_b_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_B;
    d.bitc.osdc_compress_bit_b = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_B, d.all);
}

mt_u32  reg_aria_disp_get_osdc_compress_bit_b_osdc_compress_bit_b(void)
{
    return (*(volatile reg_aria_disp_osdc_compress_bit_b_t *)REG_ARIA_DISP_OSDC_COMPRESS_BIT_B).bitc.osdc_compress_bit_b;
}


/*!
  register ARIA_DISP_osdc_bits_max_a (read/write)
  */
void reg_aria_disp_set_osdc_bits_max_a(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_BITS_MAX_A, data);
}

mt_u32  reg_aria_disp_get_osdc_bits_max_a(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_BITS_MAX_A);
}

void reg_aria_disp_set_osdc_bits_max_a_osdc_bits_max_a(mt_u32 data)
{
    reg_aria_disp_osdc_bits_max_a_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_BITS_MAX_A;
    d.bitc.osdc_bits_max_a = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_BITS_MAX_A, d.all);
}

mt_u32  reg_aria_disp_get_osdc_bits_max_a_osdc_bits_max_a(void)
{
    return (*(volatile reg_aria_disp_osdc_bits_max_a_t *)REG_ARIA_DISP_OSDC_BITS_MAX_A).bitc.osdc_bits_max_a;
}


/*!
  register ARIA_DISP_osdc_bits_max_r (read/write)
  */
void reg_aria_disp_set_osdc_bits_max_r(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_BITS_MAX_R, data);
}

mt_u32  reg_aria_disp_get_osdc_bits_max_r(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_BITS_MAX_R);
}

void reg_aria_disp_set_osdc_bits_max_r_osdc_bits_max_r(mt_u32 data)
{
    reg_aria_disp_osdc_bits_max_r_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_BITS_MAX_R;
    d.bitc.osdc_bits_max_r = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_BITS_MAX_R, d.all);
}

mt_u32  reg_aria_disp_get_osdc_bits_max_r_osdc_bits_max_r(void)
{
    return (*(volatile reg_aria_disp_osdc_bits_max_r_t *)REG_ARIA_DISP_OSDC_BITS_MAX_R).bitc.osdc_bits_max_r;
}


/*!
  register ARIA_DISP_osdc_bits_max_g (read/write)
  */
void reg_aria_disp_set_osdc_bits_max_g(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_BITS_MAX_G, data);
}

mt_u32  reg_aria_disp_get_osdc_bits_max_g(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_BITS_MAX_G);
}

void reg_aria_disp_set_osdc_bits_max_g_osdc_bits_max_g(mt_u32 data)
{
    reg_aria_disp_osdc_bits_max_g_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_BITS_MAX_G;
    d.bitc.osdc_bits_max_g = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_BITS_MAX_G, d.all);
}

mt_u32  reg_aria_disp_get_osdc_bits_max_g_osdc_bits_max_g(void)
{
    return (*(volatile reg_aria_disp_osdc_bits_max_g_t *)REG_ARIA_DISP_OSDC_BITS_MAX_G).bitc.osdc_bits_max_g;
}


/*!
  register ARIA_DISP_osdc_bits_max_b (read/write)
  */
void reg_aria_disp_set_osdc_bits_max_b(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_BITS_MAX_B, data);
}

mt_u32  reg_aria_disp_get_osdc_bits_max_b(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_BITS_MAX_B);
}

void reg_aria_disp_set_osdc_bits_max_b_osdc_bits_max_b(mt_u32 data)
{
    reg_aria_disp_osdc_bits_max_b_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_BITS_MAX_B;
    d.bitc.osdc_bits_max_b = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_BITS_MAX_B, d.all);
}

mt_u32  reg_aria_disp_get_osdc_bits_max_b_osdc_bits_max_b(void)
{
    return (*(volatile reg_aria_disp_osdc_bits_max_b_t *)REG_ARIA_DISP_OSDC_BITS_MAX_B).bitc.osdc_bits_max_b;
}


/*!
  register ARIA_DISP_osdc_cmd_ack_latency_avg (read/write)
  */
void reg_aria_disp_set_osdc_cmd_ack_latency_avg(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_AVG, data);
}

mt_u32  reg_aria_disp_get_osdc_cmd_ack_latency_avg(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_AVG);
}

void reg_aria_disp_set_osdc_cmd_ack_latency_avg_osdc_cmd_ack_latency_avg_rd(mt_u16 data)
{
    reg_aria_disp_osdc_cmd_ack_latency_avg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_AVG;
    d.bitc.osdc_cmd_ack_latency_avg_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_AVG, d.all);
}

mt_u16  reg_aria_disp_get_osdc_cmd_ack_latency_avg_osdc_cmd_ack_latency_avg_rd(void)
{
    return (*(volatile reg_aria_disp_osdc_cmd_ack_latency_avg_t *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_AVG).bitc.osdc_cmd_ack_latency_avg_rd;
}

void reg_aria_disp_set_osdc_cmd_ack_latency_avg_osdc_cmd_ack_latency_avg_wr(mt_u16 data)
{
    reg_aria_disp_osdc_cmd_ack_latency_avg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_AVG;
    d.bitc.osdc_cmd_ack_latency_avg_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_AVG, d.all);
}

mt_u16  reg_aria_disp_get_osdc_cmd_ack_latency_avg_osdc_cmd_ack_latency_avg_wr(void)
{
    return (*(volatile reg_aria_disp_osdc_cmd_ack_latency_avg_t *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_AVG).bitc.osdc_cmd_ack_latency_avg_wr;
}


/*!
  register ARIA_DISP_osdc_cmd_dat_latency_avg (read/write)
  */
void reg_aria_disp_set_osdc_cmd_dat_latency_avg(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_AVG, data);
}

mt_u32  reg_aria_disp_get_osdc_cmd_dat_latency_avg(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_AVG);
}

void reg_aria_disp_set_osdc_cmd_dat_latency_avg_osdc_cmd_dat_latency_avg_rd(mt_u16 data)
{
    reg_aria_disp_osdc_cmd_dat_latency_avg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_AVG;
    d.bitc.osdc_cmd_dat_latency_avg_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_AVG, d.all);
}

mt_u16  reg_aria_disp_get_osdc_cmd_dat_latency_avg_osdc_cmd_dat_latency_avg_rd(void)
{
    return (*(volatile reg_aria_disp_osdc_cmd_dat_latency_avg_t *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_AVG).bitc.osdc_cmd_dat_latency_avg_rd;
}

void reg_aria_disp_set_osdc_cmd_dat_latency_avg_osdc_cmd_dat_latency_avg_wr(mt_u16 data)
{
    reg_aria_disp_osdc_cmd_dat_latency_avg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_AVG;
    d.bitc.osdc_cmd_dat_latency_avg_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_AVG, d.all);
}

mt_u16  reg_aria_disp_get_osdc_cmd_dat_latency_avg_osdc_cmd_dat_latency_avg_wr(void)
{
    return (*(volatile reg_aria_disp_osdc_cmd_dat_latency_avg_t *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_AVG).bitc.osdc_cmd_dat_latency_avg_wr;
}


/*!
  register ARIA_DISP_osdc_datlast_latency_avg (read/write)
  */
void reg_aria_disp_set_osdc_datlast_latency_avg(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_AVG, data);
}

mt_u32  reg_aria_disp_get_osdc_datlast_latency_avg(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_AVG);
}

void reg_aria_disp_set_osdc_datlast_latency_avg_osdc_cmd_dat_latency_avg_rd(mt_u16 data)
{
    reg_aria_disp_osdc_datlast_latency_avg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_AVG;
    d.bitc.osdc_cmd_dat_latency_avg_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_AVG, d.all);
}

mt_u16  reg_aria_disp_get_osdc_datlast_latency_avg_osdc_cmd_dat_latency_avg_rd(void)
{
    return (*(volatile reg_aria_disp_osdc_datlast_latency_avg_t *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_AVG).bitc.osdc_cmd_dat_latency_avg_rd;
}

void reg_aria_disp_set_osdc_datlast_latency_avg_osdc_cmd_dat_latency_avg_wr(mt_u16 data)
{
    reg_aria_disp_osdc_datlast_latency_avg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_AVG;
    d.bitc.osdc_cmd_dat_latency_avg_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_AVG, d.all);
}

mt_u16  reg_aria_disp_get_osdc_datlast_latency_avg_osdc_cmd_dat_latency_avg_wr(void)
{
    return (*(volatile reg_aria_disp_osdc_datlast_latency_avg_t *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_AVG).bitc.osdc_cmd_dat_latency_avg_wr;
}


/*!
  register ARIA_DISP_osdc_cmd_ack_latency_max (read/write)
  */
void reg_aria_disp_set_osdc_cmd_ack_latency_max(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_MAX, data);
}

mt_u32  reg_aria_disp_get_osdc_cmd_ack_latency_max(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_MAX);
}

void reg_aria_disp_set_osdc_cmd_ack_latency_max_osdc_cmd_ack_latency_max_rd(mt_u16 data)
{
    reg_aria_disp_osdc_cmd_ack_latency_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_MAX;
    d.bitc.osdc_cmd_ack_latency_max_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_MAX, d.all);
}

mt_u16  reg_aria_disp_get_osdc_cmd_ack_latency_max_osdc_cmd_ack_latency_max_rd(void)
{
    return (*(volatile reg_aria_disp_osdc_cmd_ack_latency_max_t *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_MAX).bitc.osdc_cmd_ack_latency_max_rd;
}

void reg_aria_disp_set_osdc_cmd_ack_latency_max_osdc_cmd_ack_latency_max_wr(mt_u16 data)
{
    reg_aria_disp_osdc_cmd_ack_latency_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_MAX;
    d.bitc.osdc_cmd_ack_latency_max_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_MAX, d.all);
}

mt_u16  reg_aria_disp_get_osdc_cmd_ack_latency_max_osdc_cmd_ack_latency_max_wr(void)
{
    return (*(volatile reg_aria_disp_osdc_cmd_ack_latency_max_t *)REG_ARIA_DISP_OSDC_CMD_ACK_LATENCY_MAX).bitc.osdc_cmd_ack_latency_max_wr;
}


/*!
  register ARIA_DISP_osdc_cmd_dat_latency_max (read/write)
  */
void reg_aria_disp_set_osdc_cmd_dat_latency_max(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_MAX, data);
}

mt_u32  reg_aria_disp_get_osdc_cmd_dat_latency_max(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_MAX);
}

void reg_aria_disp_set_osdc_cmd_dat_latency_max_osdc_cmd_dat_latency_max_rd(mt_u16 data)
{
    reg_aria_disp_osdc_cmd_dat_latency_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_MAX;
    d.bitc.osdc_cmd_dat_latency_max_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_MAX, d.all);
}

mt_u16  reg_aria_disp_get_osdc_cmd_dat_latency_max_osdc_cmd_dat_latency_max_rd(void)
{
    return (*(volatile reg_aria_disp_osdc_cmd_dat_latency_max_t *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_MAX).bitc.osdc_cmd_dat_latency_max_rd;
}

void reg_aria_disp_set_osdc_cmd_dat_latency_max_osdc_cmd_dat_latency_max_wr(mt_u16 data)
{
    reg_aria_disp_osdc_cmd_dat_latency_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_MAX;
    d.bitc.osdc_cmd_dat_latency_max_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_MAX, d.all);
}

mt_u16  reg_aria_disp_get_osdc_cmd_dat_latency_max_osdc_cmd_dat_latency_max_wr(void)
{
    return (*(volatile reg_aria_disp_osdc_cmd_dat_latency_max_t *)REG_ARIA_DISP_OSDC_CMD_DAT_LATENCY_MAX).bitc.osdc_cmd_dat_latency_max_wr;
}


/*!
  register ARIA_DISP_osdc_datlast_latency_max (read/write)
  */
void reg_aria_disp_set_osdc_datlast_latency_max(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_MAX, data);
}

mt_u32  reg_aria_disp_get_osdc_datlast_latency_max(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_MAX);
}

void reg_aria_disp_set_osdc_datlast_latency_max_osdc_datlast_latency_max_rd(mt_u16 data)
{
    reg_aria_disp_osdc_datlast_latency_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_MAX;
    d.bitc.osdc_datlast_latency_max_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_MAX, d.all);
}

mt_u16  reg_aria_disp_get_osdc_datlast_latency_max_osdc_datlast_latency_max_rd(void)
{
    return (*(volatile reg_aria_disp_osdc_datlast_latency_max_t *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_MAX).bitc.osdc_datlast_latency_max_rd;
}

void reg_aria_disp_set_osdc_datlast_latency_max_osdc_datlast_latency_max_wr(mt_u16 data)
{
    reg_aria_disp_osdc_datlast_latency_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_MAX;
    d.bitc.osdc_datlast_latency_max_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_MAX, d.all);
}

mt_u16  reg_aria_disp_get_osdc_datlast_latency_max_osdc_datlast_latency_max_wr(void)
{
    return (*(volatile reg_aria_disp_osdc_datlast_latency_max_t *)REG_ARIA_DISP_OSDC_DATLAST_LATENCY_MAX).bitc.osdc_datlast_latency_max_wr;
}


/*!
  register ARIA_DISP_osdc_redundant0 (read/write)
  */
void reg_aria_disp_set_osdc_redundant0(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_REDUNDANT0, data);
}

mt_u32  reg_aria_disp_get_osdc_redundant0(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_REDUNDANT0);
}

void reg_aria_disp_set_osdc_redundant0_osdc_redundant0(mt_u32 data)
{
    reg_aria_disp_osdc_redundant0_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_REDUNDANT0;
    d.bitc.osdc_redundant0 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_REDUNDANT0, d.all);
}

mt_u32  reg_aria_disp_get_osdc_redundant0_osdc_redundant0(void)
{
    return (*(volatile reg_aria_disp_osdc_redundant0_t *)REG_ARIA_DISP_OSDC_REDUNDANT0).bitc.osdc_redundant0;
}


/*!
  register ARIA_DISP_osdc_redundant1 (read/write)
  */
void reg_aria_disp_set_osdc_redundant1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_REDUNDANT1, data);
}

mt_u32  reg_aria_disp_get_osdc_redundant1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_REDUNDANT1);
}

void reg_aria_disp_set_osdc_redundant1_osdc_redundant1(mt_u32 data)
{
    reg_aria_disp_osdc_redundant1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_REDUNDANT1;
    d.bitc.osdc_redundant1 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_REDUNDANT1, d.all);
}

mt_u32  reg_aria_disp_get_osdc_redundant1_osdc_redundant1(void)
{
    return (*(volatile reg_aria_disp_osdc_redundant1_t *)REG_ARIA_DISP_OSDC_REDUNDANT1).bitc.osdc_redundant1;
}


/*!
  register ARIA_DISP_osdc_redundant2 (read/write)
  */
void reg_aria_disp_set_osdc_redundant2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_REDUNDANT2, data);
}

mt_u32  reg_aria_disp_get_osdc_redundant2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_REDUNDANT2);
}

void reg_aria_disp_set_osdc_redundant2_osdc_redundant2(mt_u32 data)
{
    reg_aria_disp_osdc_redundant2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_REDUNDANT2;
    d.bitc.osdc_redundant2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_REDUNDANT2, d.all);
}

mt_u32  reg_aria_disp_get_osdc_redundant2_osdc_redundant2(void)
{
    return (*(volatile reg_aria_disp_osdc_redundant2_t *)REG_ARIA_DISP_OSDC_REDUNDANT2).bitc.osdc_redundant2;
}


/*!
  register ARIA_DISP_osdc_redundant3 (read/write)
  */
void reg_aria_disp_set_osdc_redundant3(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_REDUNDANT3, data);
}

mt_u32  reg_aria_disp_get_osdc_redundant3(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_REDUNDANT3);
}

void reg_aria_disp_set_osdc_redundant3_osdc_redundant3(mt_u32 data)
{
    reg_aria_disp_osdc_redundant3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_REDUNDANT3;
    d.bitc.osdc_redundant3 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_REDUNDANT3, d.all);
}

mt_u32  reg_aria_disp_get_osdc_redundant3_osdc_redundant3(void)
{
    return (*(volatile reg_aria_disp_osdc_redundant3_t *)REG_ARIA_DISP_OSDC_REDUNDANT3).bitc.osdc_redundant3;
}


/*!
  register ARIA_DISP_osdc_redundant4 (read/write)
  */
void reg_aria_disp_set_osdc_redundant4(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_REDUNDANT4, data);
}

mt_u32  reg_aria_disp_get_osdc_redundant4(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_OSDC_REDUNDANT4);
}

void reg_aria_disp_set_osdc_redundant4_osdc_redundant4(mt_u32 data)
{
    reg_aria_disp_osdc_redundant4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_OSDC_REDUNDANT4;
    d.bitc.osdc_redundant4 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_OSDC_REDUNDANT4, d.all);
}

mt_u32  reg_aria_disp_get_osdc_redundant4_osdc_redundant4(void)
{
    return (*(volatile reg_aria_disp_osdc_redundant4_t *)REG_ARIA_DISP_OSDC_REDUNDANT4).bitc.osdc_redundant4;
}


/*!
  register ARIA_DISP_still_control (read/write)
  */
void reg_aria_disp_set_still_control(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CONTROL, data);
}

mt_u32  reg_aria_disp_get_still_control(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_CONTROL);
}

void reg_aria_disp_set_still_control_progressive_mode(mt_u8 data)
{
    reg_aria_disp_still_control_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CONTROL;
    d.bitc.progressive_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CONTROL, d.all);
}

mt_u8   reg_aria_disp_get_still_control_progressive_mode(void)
{
    return (*(volatile reg_aria_disp_still_control_t *)REG_ARIA_DISP_STILL_CONTROL).bitc.progressive_mode;
}

void reg_aria_disp_set_still_control_still_select(mt_u8 data)
{
    reg_aria_disp_still_control_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CONTROL;
    d.bitc.still_select = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CONTROL, d.all);
}

mt_u8   reg_aria_disp_get_still_control_still_select(void)
{
    return (*(volatile reg_aria_disp_still_control_t *)REG_ARIA_DISP_STILL_CONTROL).bitc.still_select;
}

void reg_aria_disp_set_still_control_still_format(mt_u8 data)
{
    reg_aria_disp_still_control_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CONTROL;
    d.bitc.still_format = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CONTROL, d.all);
}

mt_u8   reg_aria_disp_get_still_control_still_format(void)
{
    return (*(volatile reg_aria_disp_still_control_t *)REG_ARIA_DISP_STILL_CONTROL).bitc.still_format;
}

void reg_aria_disp_set_still_control_tile_mode(mt_u8 data)
{
    reg_aria_disp_still_control_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CONTROL;
    d.bitc.tile_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CONTROL, d.all);
}

mt_u8   reg_aria_disp_get_still_control_tile_mode(void)
{
    return (*(volatile reg_aria_disp_still_control_t *)REG_ARIA_DISP_STILL_CONTROL).bitc.tile_mode;
}

void reg_aria_disp_set_still_control_tile_burst_length_select(mt_u8 data)
{
    reg_aria_disp_still_control_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CONTROL;
    d.bitc.tile_burst_length_select = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CONTROL, d.all);
}

mt_u8   reg_aria_disp_get_still_control_tile_burst_length_select(void)
{
    return (*(volatile reg_aria_disp_still_control_t *)REG_ARIA_DISP_STILL_CONTROL).bitc.tile_burst_length_select;
}

void reg_aria_disp_set_still_control_still_cr_select(mt_u8 data)
{
    reg_aria_disp_still_control_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CONTROL;
    d.bitc.still_cr_select = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CONTROL, d.all);
}

mt_u8   reg_aria_disp_get_still_control_still_cr_select(void)
{
    return (*(volatile reg_aria_disp_still_control_t *)REG_ARIA_DISP_STILL_CONTROL).bitc.still_cr_select;
}

void reg_aria_disp_set_still_control_still_cb_select(mt_u8 data)
{
    reg_aria_disp_still_control_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CONTROL;
    d.bitc.still_cb_select = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CONTROL, d.all);
}

mt_u8   reg_aria_disp_get_still_control_still_cb_select(void)
{
    return (*(volatile reg_aria_disp_still_control_t *)REG_ARIA_DISP_STILL_CONTROL).bitc.still_cb_select;
}

void reg_aria_disp_set_still_control_still_y_select(mt_u8 data)
{
    reg_aria_disp_still_control_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CONTROL;
    d.bitc.still_y_select = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CONTROL, d.all);
}

mt_u8   reg_aria_disp_get_still_control_still_y_select(void)
{
    return (*(volatile reg_aria_disp_still_control_t *)REG_ARIA_DISP_STILL_CONTROL).bitc.still_y_select;
}

void reg_aria_disp_set_still_control_still_endian_change(mt_u8 data)
{
    reg_aria_disp_still_control_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CONTROL;
    d.bitc.still_endian_change = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CONTROL, d.all);
}

mt_u8   reg_aria_disp_get_still_control_still_endian_change(void)
{
    return (*(volatile reg_aria_disp_still_control_t *)REG_ARIA_DISP_STILL_CONTROL).bitc.still_endian_change;
}

void reg_aria_disp_set_still_control_still_cr_first(mt_u8 data)
{
    reg_aria_disp_still_control_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CONTROL;
    d.bitc.still_cr_first = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CONTROL, d.all);
}

mt_u8   reg_aria_disp_get_still_control_still_cr_first(void)
{
    return (*(volatile reg_aria_disp_still_control_t *)REG_ARIA_DISP_STILL_CONTROL).bitc.still_cr_first;
}


/*!
  register ARIA_DISP_still_latch_command (read/write)
  */
void reg_aria_disp_set_still_latch_command(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_LATCH_COMMAND, data);
}

mt_u32  reg_aria_disp_get_still_latch_command(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_LATCH_COMMAND);
}

void reg_aria_disp_set_still_latch_command_still_latch_top(mt_u8 data)
{
    reg_aria_disp_still_latch_command_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_LATCH_COMMAND;
    d.bitc.still_latch_top = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_LATCH_COMMAND, d.all);
}

mt_u8   reg_aria_disp_get_still_latch_command_still_latch_top(void)
{
    return (*(volatile reg_aria_disp_still_latch_command_t *)REG_ARIA_DISP_STILL_LATCH_COMMAND).bitc.still_latch_top;
}

void reg_aria_disp_set_still_latch_command_still_latch_bot(mt_u8 data)
{
    reg_aria_disp_still_latch_command_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_LATCH_COMMAND;
    d.bitc.still_latch_bot = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_LATCH_COMMAND, d.all);
}

mt_u8   reg_aria_disp_get_still_latch_command_still_latch_bot(void)
{
    return (*(volatile reg_aria_disp_still_latch_command_t *)REG_ARIA_DISP_STILL_LATCH_COMMAND).bitc.still_latch_bot;
}

void reg_aria_disp_set_still_latch_command_still_latch_3d_1st(mt_u8 data)
{
    reg_aria_disp_still_latch_command_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_LATCH_COMMAND;
    d.bitc.still_latch_3d_1st = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_LATCH_COMMAND, d.all);
}

mt_u8   reg_aria_disp_get_still_latch_command_still_latch_3d_1st(void)
{
    return (*(volatile reg_aria_disp_still_latch_command_t *)REG_ARIA_DISP_STILL_LATCH_COMMAND).bitc.still_latch_3d_1st;
}

void reg_aria_disp_set_still_latch_command_still_latch_3d_2nd(mt_u8 data)
{
    reg_aria_disp_still_latch_command_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_LATCH_COMMAND;
    d.bitc.still_latch_3d_2nd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_LATCH_COMMAND, d.all);
}

mt_u8   reg_aria_disp_get_still_latch_command_still_latch_3d_2nd(void)
{
    return (*(volatile reg_aria_disp_still_latch_command_t *)REG_ARIA_DISP_STILL_LATCH_COMMAND).bitc.still_latch_3d_2nd;
}

void reg_aria_disp_set_still_latch_command_still_latch_or_not(mt_u8 data)
{
    reg_aria_disp_still_latch_command_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_LATCH_COMMAND;
    d.bitc.still_latch_or_not = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_LATCH_COMMAND, d.all);
}

mt_u8   reg_aria_disp_get_still_latch_command_still_latch_or_not(void)
{
    return (*(volatile reg_aria_disp_still_latch_command_t *)REG_ARIA_DISP_STILL_LATCH_COMMAND).bitc.still_latch_or_not;
}


/*!
  register ARIA_DISP_still_read_x_cfg (read/write)
  */
void reg_aria_disp_set_still_read_x_cfg(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_READ_X_CFG, data);
}

mt_u32  reg_aria_disp_get_still_read_x_cfg(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_READ_X_CFG);
}

void reg_aria_disp_set_still_read_x_cfg_still_read_x_start(mt_u16 data)
{
    reg_aria_disp_still_read_x_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_READ_X_CFG;
    d.bitc.still_read_x_start = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_READ_X_CFG, d.all);
}

mt_u16  reg_aria_disp_get_still_read_x_cfg_still_read_x_start(void)
{
    return (*(volatile reg_aria_disp_still_read_x_cfg_t *)REG_ARIA_DISP_STILL_READ_X_CFG).bitc.still_read_x_start;
}

void reg_aria_disp_set_still_read_x_cfg_still_read_x_end(mt_u16 data)
{
    reg_aria_disp_still_read_x_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_READ_X_CFG;
    d.bitc.still_read_x_end = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_READ_X_CFG, d.all);
}

mt_u16  reg_aria_disp_get_still_read_x_cfg_still_read_x_end(void)
{
    return (*(volatile reg_aria_disp_still_read_x_cfg_t *)REG_ARIA_DISP_STILL_READ_X_CFG).bitc.still_read_x_end;
}


/*!
  register ARIA_DISP_still_read_y_cfg (read/write)
  */
void reg_aria_disp_set_still_read_y_cfg(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_READ_Y_CFG, data);
}

mt_u32  reg_aria_disp_get_still_read_y_cfg(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_READ_Y_CFG);
}

void reg_aria_disp_set_still_read_y_cfg_still_read_y_start(mt_u16 data)
{
    reg_aria_disp_still_read_y_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_READ_Y_CFG;
    d.bitc.still_read_y_start = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_READ_Y_CFG, d.all);
}

mt_u16  reg_aria_disp_get_still_read_y_cfg_still_read_y_start(void)
{
    return (*(volatile reg_aria_disp_still_read_y_cfg_t *)REG_ARIA_DISP_STILL_READ_Y_CFG).bitc.still_read_y_start;
}

void reg_aria_disp_set_still_read_y_cfg_still_read_y_end(mt_u16 data)
{
    reg_aria_disp_still_read_y_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_READ_Y_CFG;
    d.bitc.still_read_y_end = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_READ_Y_CFG, d.all);
}

mt_u16  reg_aria_disp_get_still_read_y_cfg_still_read_y_end(void)
{
    return (*(volatile reg_aria_disp_still_read_y_cfg_t *)REG_ARIA_DISP_STILL_READ_Y_CFG).bitc.still_read_y_end;
}


/*!
  register ARIA_DISP_still_stride (read/write)
  */
void reg_aria_disp_set_still_stride(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_STRIDE, data);
}

mt_u32  reg_aria_disp_get_still_stride(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_STRIDE);
}

void reg_aria_disp_set_still_stride_still_stride(mt_u16 data)
{
    reg_aria_disp_still_stride_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_STRIDE;
    d.bitc.still_stride = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_STRIDE, d.all);
}

mt_u16  reg_aria_disp_get_still_stride_still_stride(void)
{
    return (*(volatile reg_aria_disp_still_stride_t *)REG_ARIA_DISP_STILL_STRIDE).bitc.still_stride;
}


/*!
  register ARIA_DISP_still_luma_baseaddr (read/write)
  */
void reg_aria_disp_set_still_luma_baseaddr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_LUMA_BASEADDR, data);
}

mt_u32  reg_aria_disp_get_still_luma_baseaddr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_LUMA_BASEADDR);
}

void reg_aria_disp_set_still_luma_baseaddr_still_luma_baseaddr(mt_u32 data)
{
    reg_aria_disp_still_luma_baseaddr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_LUMA_BASEADDR;
    d.bitc.still_luma_baseaddr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_LUMA_BASEADDR, d.all);
}

mt_u32  reg_aria_disp_get_still_luma_baseaddr_still_luma_baseaddr(void)
{
    return (*(volatile reg_aria_disp_still_luma_baseaddr_t *)REG_ARIA_DISP_STILL_LUMA_BASEADDR).bitc.still_luma_baseaddr;
}


/*!
  register ARIA_DISP_still_cbcr_baseaddr (read/write)
  */
void reg_aria_disp_set_still_cbcr_baseaddr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CBCR_BASEADDR, data);
}

mt_u32  reg_aria_disp_get_still_cbcr_baseaddr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_CBCR_BASEADDR);
}

void reg_aria_disp_set_still_cbcr_baseaddr_still_cbcr_baseaddr(mt_u32 data)
{
    reg_aria_disp_still_cbcr_baseaddr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CBCR_BASEADDR;
    d.bitc.still_cbcr_baseaddr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CBCR_BASEADDR, d.all);
}

mt_u32  reg_aria_disp_get_still_cbcr_baseaddr_still_cbcr_baseaddr(void)
{
    return (*(volatile reg_aria_disp_still_cbcr_baseaddr_t *)REG_ARIA_DISP_STILL_CBCR_BASEADDR).bitc.still_cbcr_baseaddr;
}


/*!
  register ARIA_DISP_still_fifo_threshold (read/write)
  */
void reg_aria_disp_set_still_fifo_threshold(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_FIFO_THRESHOLD, data);
}

mt_u32  reg_aria_disp_get_still_fifo_threshold(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_FIFO_THRESHOLD);
}

void reg_aria_disp_set_still_fifo_threshold_still_fifo_low_threshold(mt_u8 data)
{
    reg_aria_disp_still_fifo_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_FIFO_THRESHOLD;
    d.bitc.still_fifo_low_threshold = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_FIFO_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_still_fifo_threshold_still_fifo_low_threshold(void)
{
    return (*(volatile reg_aria_disp_still_fifo_threshold_t *)REG_ARIA_DISP_STILL_FIFO_THRESHOLD).bitc.still_fifo_low_threshold;
}

void reg_aria_disp_set_still_fifo_threshold_still_fifo_high_threshold(mt_u8 data)
{
    reg_aria_disp_still_fifo_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_FIFO_THRESHOLD;
    d.bitc.still_fifo_high_threshold = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_FIFO_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_still_fifo_threshold_still_fifo_high_threshold(void)
{
    return (*(volatile reg_aria_disp_still_fifo_threshold_t *)REG_ARIA_DISP_STILL_FIFO_THRESHOLD).bitc.still_fifo_high_threshold;
}


/*!
  register ARIA_DISP_still_tile_parameter (read/write)
  */
void reg_aria_disp_set_still_tile_parameter(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_TILE_PARAMETER, data);
}

mt_u32  reg_aria_disp_get_still_tile_parameter(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_TILE_PARAMETER);
}

void reg_aria_disp_set_still_tile_parameter_tile_col_size_mode(mt_u8 data)
{
    reg_aria_disp_still_tile_parameter_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_TILE_PARAMETER;
    d.bitc.tile_col_size_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_TILE_PARAMETER, d.all);
}

mt_u8   reg_aria_disp_get_still_tile_parameter_tile_col_size_mode(void)
{
    return (*(volatile reg_aria_disp_still_tile_parameter_t *)REG_ARIA_DISP_STILL_TILE_PARAMETER).bitc.tile_col_size_mode;
}

void reg_aria_disp_set_still_tile_parameter_still_field_picture(mt_u8 data)
{
    reg_aria_disp_still_tile_parameter_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_TILE_PARAMETER;
    d.bitc.still_field_picture = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_TILE_PARAMETER, d.all);
}

mt_u8   reg_aria_disp_get_still_tile_parameter_still_field_picture(void)
{
    return (*(volatile reg_aria_disp_still_tile_parameter_t *)REG_ARIA_DISP_STILL_TILE_PARAMETER).bitc.still_field_picture;
}

void reg_aria_disp_set_still_tile_parameter_still_hd_map_mode(mt_u8 data)
{
    reg_aria_disp_still_tile_parameter_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_TILE_PARAMETER;
    d.bitc.still_hd_map_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_TILE_PARAMETER, d.all);
}

mt_u8   reg_aria_disp_get_still_tile_parameter_still_hd_map_mode(void)
{
    return (*(volatile reg_aria_disp_still_tile_parameter_t *)REG_ARIA_DISP_STILL_TILE_PARAMETER).bitc.still_hd_map_mode;
}

void reg_aria_disp_set_still_tile_parameter_still_tile_config(mt_u8 data)
{
    reg_aria_disp_still_tile_parameter_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_TILE_PARAMETER;
    d.bitc.still_tile_config = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_TILE_PARAMETER, d.all);
}

mt_u8   reg_aria_disp_get_still_tile_parameter_still_tile_config(void)
{
    return (*(volatile reg_aria_disp_still_tile_parameter_t *)REG_ARIA_DISP_STILL_TILE_PARAMETER).bitc.still_tile_config;
}


/*!
  register ARIA_DISP_still_tile_rowjump_00 (read/write)
  */
void reg_aria_disp_set_still_tile_rowjump_00(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_00, data);
}

mt_u32  reg_aria_disp_get_still_tile_rowjump_00(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_00);
}

void reg_aria_disp_set_still_tile_rowjump_00_still_tile_rowjump_00(mt_u32 data)
{
    reg_aria_disp_still_tile_rowjump_00_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_00;
    d.bitc.still_tile_rowjump_00 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_00, d.all);
}

mt_u32  reg_aria_disp_get_still_tile_rowjump_00_still_tile_rowjump_00(void)
{
    return (*(volatile reg_aria_disp_still_tile_rowjump_00_t *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_00).bitc.still_tile_rowjump_00;
}


/*!
  register ARIA_DISP_still_tile_rowjump_01 (read/write)
  */
void reg_aria_disp_set_still_tile_rowjump_01(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_01, data);
}

mt_u32  reg_aria_disp_get_still_tile_rowjump_01(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_01);
}

void reg_aria_disp_set_still_tile_rowjump_01_still_tile_rowjump_01(mt_u32 data)
{
    reg_aria_disp_still_tile_rowjump_01_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_01;
    d.bitc.still_tile_rowjump_01 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_01, d.all);
}

mt_u32  reg_aria_disp_get_still_tile_rowjump_01_still_tile_rowjump_01(void)
{
    return (*(volatile reg_aria_disp_still_tile_rowjump_01_t *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_01).bitc.still_tile_rowjump_01;
}


/*!
  register ARIA_DISP_still_tile_rowjump_10 (read/write)
  */
void reg_aria_disp_set_still_tile_rowjump_10(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_10, data);
}

mt_u32  reg_aria_disp_get_still_tile_rowjump_10(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_10);
}

void reg_aria_disp_set_still_tile_rowjump_10_still_tile_rowjump_10(mt_u32 data)
{
    reg_aria_disp_still_tile_rowjump_10_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_10;
    d.bitc.still_tile_rowjump_10 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_10, d.all);
}

mt_u32  reg_aria_disp_get_still_tile_rowjump_10_still_tile_rowjump_10(void)
{
    return (*(volatile reg_aria_disp_still_tile_rowjump_10_t *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_10).bitc.still_tile_rowjump_10;
}


/*!
  register ARIA_DISP_still_tile_rowjump_11 (read/write)
  */
void reg_aria_disp_set_still_tile_rowjump_11(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_11, data);
}

mt_u32  reg_aria_disp_get_still_tile_rowjump_11(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_11);
}

void reg_aria_disp_set_still_tile_rowjump_11_still_tile_rowjump_11(mt_u32 data)
{
    reg_aria_disp_still_tile_rowjump_11_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_11;
    d.bitc.still_tile_rowjump_11 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_11, d.all);
}

mt_u32  reg_aria_disp_get_still_tile_rowjump_11_still_tile_rowjump_11(void)
{
    return (*(volatile reg_aria_disp_still_tile_rowjump_11_t *)REG_ARIA_DISP_STILL_TILE_ROWJUMP_11).bitc.still_tile_rowjump_11;
}


/*!
  register ARIA_DISP_still_status (read/write)
  */
void reg_aria_disp_set_still_status(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_STATUS, data);
}

mt_u32  reg_aria_disp_get_still_status(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_STATUS);
}

void reg_aria_disp_set_still_status_still_axi_rready_error(mt_u8 data)
{
    reg_aria_disp_still_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_STATUS;
    d.bitc.still_axi_rready_error = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_still_status_still_axi_rready_error(void)
{
    return (*(volatile reg_aria_disp_still_status_t *)REG_ARIA_DISP_STILL_STATUS).bitc.still_axi_rready_error;
}

void reg_aria_disp_set_still_status_still_fifo_uv_full(mt_u8 data)
{
    reg_aria_disp_still_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_STATUS;
    d.bitc.still_fifo_uv_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_still_status_still_fifo_uv_full(void)
{
    return (*(volatile reg_aria_disp_still_status_t *)REG_ARIA_DISP_STILL_STATUS).bitc.still_fifo_uv_full;
}

void reg_aria_disp_set_still_status_still_fifo_y_full(mt_u8 data)
{
    reg_aria_disp_still_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_STATUS;
    d.bitc.still_fifo_y_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_still_status_still_fifo_y_full(void)
{
    return (*(volatile reg_aria_disp_still_status_t *)REG_ARIA_DISP_STILL_STATUS).bitc.still_fifo_y_full;
}

void reg_aria_disp_set_still_status_still_fifo_uv_empty(mt_u8 data)
{
    reg_aria_disp_still_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_STATUS;
    d.bitc.still_fifo_uv_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_still_status_still_fifo_uv_empty(void)
{
    return (*(volatile reg_aria_disp_still_status_t *)REG_ARIA_DISP_STILL_STATUS).bitc.still_fifo_uv_empty;
}

void reg_aria_disp_set_still_status_still_fifo_y_empty(mt_u8 data)
{
    reg_aria_disp_still_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_STATUS;
    d.bitc.still_fifo_y_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_still_status_still_fifo_y_empty(void)
{
    return (*(volatile reg_aria_disp_still_status_t *)REG_ARIA_DISP_STILL_STATUS).bitc.still_fifo_y_empty;
}


/*!
  register ARIA_DISP_still_axi_monitor_ctrl (read/write)
  */
void reg_aria_disp_set_still_axi_monitor_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_AXI_MONITOR_CTRL, data);
}

mt_u32  reg_aria_disp_get_still_axi_monitor_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_AXI_MONITOR_CTRL);
}

void reg_aria_disp_set_still_axi_monitor_ctrl_still_axi_monitor_reload(mt_u8 data)
{
    reg_aria_disp_still_axi_monitor_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_AXI_MONITOR_CTRL;
    d.bitc.still_axi_monitor_reload = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_AXI_MONITOR_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_still_axi_monitor_ctrl_still_axi_monitor_reload(void)
{
    return (*(volatile reg_aria_disp_still_axi_monitor_ctrl_t *)REG_ARIA_DISP_STILL_AXI_MONITOR_CTRL).bitc.still_axi_monitor_reload;
}


/*!
  register ARIA_DISP_still_cmd_ack_latency_monitor (read/write)
  */
void reg_aria_disp_set_still_cmd_ack_latency_monitor(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CMD_ACK_LATENCY_MONITOR, data);
}

mt_u32  reg_aria_disp_get_still_cmd_ack_latency_monitor(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_CMD_ACK_LATENCY_MONITOR);
}

void reg_aria_disp_set_still_cmd_ack_latency_monitor_cmd_ack_latency_max_value(mt_u16 data)
{
    reg_aria_disp_still_cmd_ack_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CMD_ACK_LATENCY_MONITOR;
    d.bitc.cmd_ack_latency_max_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CMD_ACK_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_still_cmd_ack_latency_monitor_cmd_ack_latency_max_value(void)
{
    return (*(volatile reg_aria_disp_still_cmd_ack_latency_monitor_t *)REG_ARIA_DISP_STILL_CMD_ACK_LATENCY_MONITOR).bitc.cmd_ack_latency_max_value;
}

void reg_aria_disp_set_still_cmd_ack_latency_monitor_cmd_ack_latency_average_value(mt_u16 data)
{
    reg_aria_disp_still_cmd_ack_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CMD_ACK_LATENCY_MONITOR;
    d.bitc.cmd_ack_latency_average_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CMD_ACK_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_still_cmd_ack_latency_monitor_cmd_ack_latency_average_value(void)
{
    return (*(volatile reg_aria_disp_still_cmd_ack_latency_monitor_t *)REG_ARIA_DISP_STILL_CMD_ACK_LATENCY_MONITOR).bitc.cmd_ack_latency_average_value;
}


/*!
  register ARIA_DISP_still_data_ack_latency_monitor (read/write)
  */
void reg_aria_disp_set_still_data_ack_latency_monitor(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_DATA_ACK_LATENCY_MONITOR, data);
}

mt_u32  reg_aria_disp_get_still_data_ack_latency_monitor(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_DATA_ACK_LATENCY_MONITOR);
}

void reg_aria_disp_set_still_data_ack_latency_monitor_cmd_data_latency_max_value(mt_u16 data)
{
    reg_aria_disp_still_data_ack_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_DATA_ACK_LATENCY_MONITOR;
    d.bitc.cmd_data_latency_max_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_DATA_ACK_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_still_data_ack_latency_monitor_cmd_data_latency_max_value(void)
{
    return (*(volatile reg_aria_disp_still_data_ack_latency_monitor_t *)REG_ARIA_DISP_STILL_DATA_ACK_LATENCY_MONITOR).bitc.cmd_data_latency_max_value;
}

void reg_aria_disp_set_still_data_ack_latency_monitor_cmd_data_latency_average_value(mt_u16 data)
{
    reg_aria_disp_still_data_ack_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_DATA_ACK_LATENCY_MONITOR;
    d.bitc.cmd_data_latency_average_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_DATA_ACK_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_still_data_ack_latency_monitor_cmd_data_latency_average_value(void)
{
    return (*(volatile reg_aria_disp_still_data_ack_latency_monitor_t *)REG_ARIA_DISP_STILL_DATA_ACK_LATENCY_MONITOR).bitc.cmd_data_latency_average_value;
}


/*!
  register ARIA_DISP_still_data_last_latency_monitor (read/write)
  */
void reg_aria_disp_set_still_data_last_latency_monitor(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_DATA_LAST_LATENCY_MONITOR, data);
}

mt_u32  reg_aria_disp_get_still_data_last_latency_monitor(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_DATA_LAST_LATENCY_MONITOR);
}

void reg_aria_disp_set_still_data_last_latency_monitor_last_data_latency_max_value(mt_u16 data)
{
    reg_aria_disp_still_data_last_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_DATA_LAST_LATENCY_MONITOR;
    d.bitc.last_data_latency_max_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_DATA_LAST_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_still_data_last_latency_monitor_last_data_latency_max_value(void)
{
    return (*(volatile reg_aria_disp_still_data_last_latency_monitor_t *)REG_ARIA_DISP_STILL_DATA_LAST_LATENCY_MONITOR).bitc.last_data_latency_max_value;
}

void reg_aria_disp_set_still_data_last_latency_monitor_last_data_latency_average_value(mt_u16 data)
{
    reg_aria_disp_still_data_last_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_DATA_LAST_LATENCY_MONITOR;
    d.bitc.last_data_latency_average_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_DATA_LAST_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_still_data_last_latency_monitor_last_data_latency_average_value(void)
{
    return (*(volatile reg_aria_disp_still_data_last_latency_monitor_t *)REG_ARIA_DISP_STILL_DATA_LAST_LATENCY_MONITOR).bitc.last_data_latency_average_value;
}


/*!
  register ARIA_DISP_still_scale_ctrl (read/write)
  */
void reg_aria_disp_set_still_scale_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_CTRL, data);
}

mt_u32  reg_aria_disp_get_still_scale_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_CTRL);
}

void reg_aria_disp_set_still_scale_ctrl_still_h_filter_en(mt_u8 data)
{
    reg_aria_disp_still_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_CTRL;
    d.bitc.still_h_filter_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_still_scale_ctrl_still_h_filter_en(void)
{
    return (*(volatile reg_aria_disp_still_scale_ctrl_t *)REG_ARIA_DISP_STILL_SCALE_CTRL).bitc.still_h_filter_en;
}

void reg_aria_disp_set_still_scale_ctrl_still_v_filter_en(mt_u8 data)
{
    reg_aria_disp_still_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_CTRL;
    d.bitc.still_v_filter_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_still_scale_ctrl_still_v_filter_en(void)
{
    return (*(volatile reg_aria_disp_still_scale_ctrl_t *)REG_ARIA_DISP_STILL_SCALE_CTRL).bitc.still_v_filter_en;
}

void reg_aria_disp_set_still_scale_ctrl_downsample_en(mt_u8 data)
{
    reg_aria_disp_still_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_CTRL;
    d.bitc.downsample_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_still_scale_ctrl_downsample_en(void)
{
    return (*(volatile reg_aria_disp_still_scale_ctrl_t *)REG_ARIA_DISP_STILL_SCALE_CTRL).bitc.downsample_en;
}

void reg_aria_disp_set_still_scale_ctrl_v_phase_type(mt_u8 data)
{
    reg_aria_disp_still_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_CTRL;
    d.bitc.v_phase_type = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_still_scale_ctrl_v_phase_type(void)
{
    return (*(volatile reg_aria_disp_still_scale_ctrl_t *)REG_ARIA_DISP_STILL_SCALE_CTRL).bitc.v_phase_type;
}

void reg_aria_disp_set_still_scale_ctrl_odd_start_line_number(mt_u8 data)
{
    reg_aria_disp_still_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_CTRL;
    d.bitc.odd_start_line_number = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_still_scale_ctrl_odd_start_line_number(void)
{
    return (*(volatile reg_aria_disp_still_scale_ctrl_t *)REG_ARIA_DISP_STILL_SCALE_CTRL).bitc.odd_start_line_number;
}

void reg_aria_disp_set_still_scale_ctrl_even_start_line_number(mt_u8 data)
{
    reg_aria_disp_still_scale_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_CTRL;
    d.bitc.even_start_line_number = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_still_scale_ctrl_even_start_line_number(void)
{
    return (*(volatile reg_aria_disp_still_scale_ctrl_t *)REG_ARIA_DISP_STILL_SCALE_CTRL).bitc.even_start_line_number;
}


/*!
  register ARIA_DISP_still_scale_h_ratio (read/write)
  */
void reg_aria_disp_set_still_scale_h_ratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_H_RATIO, data);
}

mt_u32  reg_aria_disp_get_still_scale_h_ratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_H_RATIO);
}

void reg_aria_disp_set_still_scale_h_ratio_h_ratio_int(mt_u8 data)
{
    reg_aria_disp_still_scale_h_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_H_RATIO;
    d.bitc.h_ratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_H_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_still_scale_h_ratio_h_ratio_int(void)
{
    return (*(volatile reg_aria_disp_still_scale_h_ratio_t *)REG_ARIA_DISP_STILL_SCALE_H_RATIO).bitc.h_ratio_int;
}

void reg_aria_disp_set_still_scale_h_ratio_h_ratio_fra(mt_u16 data)
{
    reg_aria_disp_still_scale_h_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_H_RATIO;
    d.bitc.h_ratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_H_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_still_scale_h_ratio_h_ratio_fra(void)
{
    return (*(volatile reg_aria_disp_still_scale_h_ratio_t *)REG_ARIA_DISP_STILL_SCALE_H_RATIO).bitc.h_ratio_fra;
}


/*!
  register ARIA_DISP_still_scale_v_ratio (read/write)
  */
void reg_aria_disp_set_still_scale_v_ratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_V_RATIO, data);
}

mt_u32  reg_aria_disp_get_still_scale_v_ratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_V_RATIO);
}

void reg_aria_disp_set_still_scale_v_ratio_v_ratio_int(mt_u8 data)
{
    reg_aria_disp_still_scale_v_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_V_RATIO;
    d.bitc.v_ratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_V_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_still_scale_v_ratio_v_ratio_int(void)
{
    return (*(volatile reg_aria_disp_still_scale_v_ratio_t *)REG_ARIA_DISP_STILL_SCALE_V_RATIO).bitc.v_ratio_int;
}

void reg_aria_disp_set_still_scale_v_ratio_v_ratio_fra(mt_u16 data)
{
    reg_aria_disp_still_scale_v_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_V_RATIO;
    d.bitc.v_ratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_V_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_still_scale_v_ratio_v_ratio_fra(void)
{
    return (*(volatile reg_aria_disp_still_scale_v_ratio_t *)REG_ARIA_DISP_STILL_SCALE_V_RATIO).bitc.v_ratio_fra;
}


/*!
  register ARIA_DISP_still_scale_h_start_fra (read/write)
  */
void reg_aria_disp_set_still_scale_h_start_fra(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_H_START_FRA, data);
}

mt_u32  reg_aria_disp_get_still_scale_h_start_fra(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_H_START_FRA);
}

void reg_aria_disp_set_still_scale_h_start_fra_still_scale_h_start_fra(mt_u16 data)
{
    reg_aria_disp_still_scale_h_start_fra_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_H_START_FRA;
    d.bitc.still_scale_h_start_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_H_START_FRA, d.all);
}

mt_u16  reg_aria_disp_get_still_scale_h_start_fra_still_scale_h_start_fra(void)
{
    return (*(volatile reg_aria_disp_still_scale_h_start_fra_t *)REG_ARIA_DISP_STILL_SCALE_H_START_FRA).bitc.still_scale_h_start_fra;
}


/*!
  register ARIA_DISP_still_scale_v_start_fra (read/write)
  */
void reg_aria_disp_set_still_scale_v_start_fra(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_V_START_FRA, data);
}

mt_u32  reg_aria_disp_get_still_scale_v_start_fra(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_V_START_FRA);
}

void reg_aria_disp_set_still_scale_v_start_fra_still_scale_v_start_fra_odd(mt_u16 data)
{
    reg_aria_disp_still_scale_v_start_fra_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_V_START_FRA;
    d.bitc.still_scale_v_start_fra_odd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_V_START_FRA, d.all);
}

mt_u16  reg_aria_disp_get_still_scale_v_start_fra_still_scale_v_start_fra_odd(void)
{
    return (*(volatile reg_aria_disp_still_scale_v_start_fra_t *)REG_ARIA_DISP_STILL_SCALE_V_START_FRA).bitc.still_scale_v_start_fra_odd;
}

void reg_aria_disp_set_still_scale_v_start_fra_still_scale_v_start_fra_even(mt_u16 data)
{
    reg_aria_disp_still_scale_v_start_fra_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_V_START_FRA;
    d.bitc.still_scale_v_start_fra_even = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_V_START_FRA, d.all);
}

mt_u16  reg_aria_disp_get_still_scale_v_start_fra_still_scale_v_start_fra_even(void)
{
    return (*(volatile reg_aria_disp_still_scale_v_start_fra_t *)REG_ARIA_DISP_STILL_SCALE_V_START_FRA).bitc.still_scale_v_start_fra_even;
}


/*!
  register ARIA_DISP_still_scale_hsize (read/write)
  */
void reg_aria_disp_set_still_scale_hsize(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_HSIZE, data);
}

mt_u32  reg_aria_disp_get_still_scale_hsize(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_HSIZE);
}

void reg_aria_disp_set_still_scale_hsize_dst_hsize(mt_u16 data)
{
    reg_aria_disp_still_scale_hsize_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_HSIZE;
    d.bitc.dst_hsize = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_HSIZE, d.all);
}

mt_u16  reg_aria_disp_get_still_scale_hsize_dst_hsize(void)
{
    return (*(volatile reg_aria_disp_still_scale_hsize_t *)REG_ARIA_DISP_STILL_SCALE_HSIZE).bitc.dst_hsize;
}


/*!
  register ARIA_DISP_still_scale_vsize (read/write)
  */
void reg_aria_disp_set_still_scale_vsize(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_VSIZE, data);
}

mt_u32  reg_aria_disp_get_still_scale_vsize(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_VSIZE);
}

void reg_aria_disp_set_still_scale_vsize_dst_vsize(mt_u16 data)
{
    reg_aria_disp_still_scale_vsize_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_VSIZE;
    d.bitc.dst_vsize = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_VSIZE, d.all);
}

mt_u16  reg_aria_disp_get_still_scale_vsize_dst_vsize(void)
{
    return (*(volatile reg_aria_disp_still_scale_vsize_t *)REG_ARIA_DISP_STILL_SCALE_VSIZE).bitc.dst_vsize;
}


/*!
  register ARIA_DISP_still_x_config (read/write)
  */
void reg_aria_disp_set_still_x_config(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_X_CONFIG, data);
}

mt_u32  reg_aria_disp_get_still_x_config(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_X_CONFIG);
}

void reg_aria_disp_set_still_x_config_x_start(mt_u16 data)
{
    reg_aria_disp_still_x_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_X_CONFIG;
    d.bitc.x_start = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_X_CONFIG, d.all);
}

mt_u16  reg_aria_disp_get_still_x_config_x_start(void)
{
    return (*(volatile reg_aria_disp_still_x_config_t *)REG_ARIA_DISP_STILL_X_CONFIG).bitc.x_start;
}


/*!
  register ARIA_DISP_still_y_config (read/write)
  */
void reg_aria_disp_set_still_y_config(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_Y_CONFIG, data);
}

mt_u32  reg_aria_disp_get_still_y_config(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_Y_CONFIG);
}

void reg_aria_disp_set_still_y_config_y_start(mt_u16 data)
{
    reg_aria_disp_still_y_config_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_Y_CONFIG;
    d.bitc.y_start = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_Y_CONFIG, d.all);
}

mt_u16  reg_aria_disp_get_still_y_config_y_start(void)
{
    return (*(volatile reg_aria_disp_still_y_config_t *)REG_ARIA_DISP_STILL_Y_CONFIG).bitc.y_start;
}


/*!
  register ARIA_DISP_still_scale_y_coeff_address (read/write)
  */
void reg_aria_disp_set_still_scale_y_coeff_address(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_Y_COEFF_ADDRESS, data);
}

mt_u32  reg_aria_disp_get_still_scale_y_coeff_address(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_Y_COEFF_ADDRESS);
}

void reg_aria_disp_set_still_scale_y_coeff_address_still_scale_y_coeff_address(mt_u32 data)
{
    reg_aria_disp_still_scale_y_coeff_address_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_Y_COEFF_ADDRESS;
    d.bitc.still_scale_y_coeff_address = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_Y_COEFF_ADDRESS, d.all);
}

mt_u32  reg_aria_disp_get_still_scale_y_coeff_address_still_scale_y_coeff_address(void)
{
    return (*(volatile reg_aria_disp_still_scale_y_coeff_address_t *)REG_ARIA_DISP_STILL_SCALE_Y_COEFF_ADDRESS).bitc.still_scale_y_coeff_address;
}


/*!
  register ARIA_DISP_still_scale_uv_coeff_address (read/write)
  */
void reg_aria_disp_set_still_scale_uv_coeff_address(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_UV_COEFF_ADDRESS, data);
}

mt_u32  reg_aria_disp_get_still_scale_uv_coeff_address(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_UV_COEFF_ADDRESS);
}

void reg_aria_disp_set_still_scale_uv_coeff_address_still_scale_uv_coeff_address(mt_u32 data)
{
    reg_aria_disp_still_scale_uv_coeff_address_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_UV_COEFF_ADDRESS;
    d.bitc.still_scale_uv_coeff_address = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_UV_COEFF_ADDRESS, d.all);
}

mt_u32  reg_aria_disp_get_still_scale_uv_coeff_address_still_scale_uv_coeff_address(void)
{
    return (*(volatile reg_aria_disp_still_scale_uv_coeff_address_t *)REG_ARIA_DISP_STILL_SCALE_UV_COEFF_ADDRESS).bitc.still_scale_uv_coeff_address;
}


/*!
  register ARIA_DISP_still_scale_fifo1_threshold (read/write)
  */
void reg_aria_disp_set_still_scale_fifo1_threshold(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_FIFO1_THRESHOLD, data);
}

mt_u32  reg_aria_disp_get_still_scale_fifo1_threshold(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_FIFO1_THRESHOLD);
}

void reg_aria_disp_set_still_scale_fifo1_threshold_still_scale_fifo1_threshold(mt_u32 data)
{
    reg_aria_disp_still_scale_fifo1_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_FIFO1_THRESHOLD;
    d.bitc.still_scale_fifo1_threshold = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_FIFO1_THRESHOLD, d.all);
}

mt_u32  reg_aria_disp_get_still_scale_fifo1_threshold_still_scale_fifo1_threshold(void)
{
    return (*(volatile reg_aria_disp_still_scale_fifo1_threshold_t *)REG_ARIA_DISP_STILL_SCALE_FIFO1_THRESHOLD).bitc.still_scale_fifo1_threshold;
}


/*!
  register ARIA_DISP_still_scale_fifo2_threshold (read/write)
  */
void reg_aria_disp_set_still_scale_fifo2_threshold(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_FIFO2_THRESHOLD, data);
}

mt_u32  reg_aria_disp_get_still_scale_fifo2_threshold(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_FIFO2_THRESHOLD);
}

void reg_aria_disp_set_still_scale_fifo2_threshold_still_scale_fifo2_threshold(mt_u32 data)
{
    reg_aria_disp_still_scale_fifo2_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALE_FIFO2_THRESHOLD;
    d.bitc.still_scale_fifo2_threshold = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALE_FIFO2_THRESHOLD, d.all);
}

mt_u32  reg_aria_disp_get_still_scale_fifo2_threshold_still_scale_fifo2_threshold(void)
{
    return (*(volatile reg_aria_disp_still_scale_fifo2_threshold_t *)REG_ARIA_DISP_STILL_SCALE_FIFO2_THRESHOLD).bitc.still_scale_fifo2_threshold;
}


/*!
  register ARIA_DISP_still_scaler_status (read/write)
  */
void reg_aria_disp_set_still_scaler_status(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALER_STATUS, data);
}

mt_u32  reg_aria_disp_get_still_scaler_status(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALER_STATUS);
}

void reg_aria_disp_set_still_scaler_status_still_saler_fifo_full(mt_u8 data)
{
    reg_aria_disp_still_scaler_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALER_STATUS;
    d.bitc.still_saler_fifo_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALER_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_still_scaler_status_still_saler_fifo_full(void)
{
    return (*(volatile reg_aria_disp_still_scaler_status_t *)REG_ARIA_DISP_STILL_SCALER_STATUS).bitc.still_saler_fifo_full;
}

void reg_aria_disp_set_still_scaler_status_still_scaler_fifo_empty(mt_u8 data)
{
    reg_aria_disp_still_scaler_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_SCALER_STATUS;
    d.bitc.still_scaler_fifo_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_SCALER_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_still_scaler_status_still_scaler_fifo_empty(void)
{
    return (*(volatile reg_aria_disp_still_scaler_status_t *)REG_ARIA_DISP_STILL_SCALER_STATUS).bitc.still_scaler_fifo_empty;
}


/*!
  register ARIA_DISP_still_csc_ctrl (read/write)
  */
void reg_aria_disp_set_still_csc_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_CTRL, data);
}

mt_u32  reg_aria_disp_get_still_csc_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_CTRL);
}

void reg_aria_disp_set_still_csc_ctrl_still_bound_output_en(mt_u8 data)
{
    reg_aria_disp_still_csc_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_CTRL;
    d.bitc.still_bound_output_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_still_csc_ctrl_still_bound_output_en(void)
{
    return (*(volatile reg_aria_disp_still_csc_ctrl_t *)REG_ARIA_DISP_STILL_CSC_CTRL).bitc.still_bound_output_en;
}

void reg_aria_disp_set_still_csc_ctrl_still_bound_input_en(mt_u8 data)
{
    reg_aria_disp_still_csc_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_CTRL;
    d.bitc.still_bound_input_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_still_csc_ctrl_still_bound_input_en(void)
{
    return (*(volatile reg_aria_disp_still_csc_ctrl_t *)REG_ARIA_DISP_STILL_CSC_CTRL).bitc.still_bound_input_en;
}

void reg_aria_disp_set_still_csc_ctrl_still_csc_en(mt_u8 data)
{
    reg_aria_disp_still_csc_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_CTRL;
    d.bitc.still_csc_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_still_csc_ctrl_still_csc_en(void)
{
    return (*(volatile reg_aria_disp_still_csc_ctrl_t *)REG_ARIA_DISP_STILL_CSC_CTRL).bitc.still_csc_en;
}


/*!
  register ARIA_DISP_still_csc_coeff1 (read/write)
  */
void reg_aria_disp_set_still_csc_coeff1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF1, data);
}

mt_u32  reg_aria_disp_get_still_csc_coeff1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF1);
}

void reg_aria_disp_set_still_csc_coeff1_still_csc_a01(mt_u16 data)
{
    reg_aria_disp_still_csc_coeff1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF1;
    d.bitc.still_csc_a01 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF1, d.all);
}

mt_u16  reg_aria_disp_get_still_csc_coeff1_still_csc_a01(void)
{
    return (*(volatile reg_aria_disp_still_csc_coeff1_t *)REG_ARIA_DISP_STILL_CSC_COEFF1).bitc.still_csc_a01;
}

void reg_aria_disp_set_still_csc_coeff1_still_csc_a00(mt_u16 data)
{
    reg_aria_disp_still_csc_coeff1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF1;
    d.bitc.still_csc_a00 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF1, d.all);
}

mt_u16  reg_aria_disp_get_still_csc_coeff1_still_csc_a00(void)
{
    return (*(volatile reg_aria_disp_still_csc_coeff1_t *)REG_ARIA_DISP_STILL_CSC_COEFF1).bitc.still_csc_a00;
}


/*!
  register ARIA_DISP_still_csc_coeff2 (read/write)
  */
void reg_aria_disp_set_still_csc_coeff2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF2, data);
}

mt_u32  reg_aria_disp_get_still_csc_coeff2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF2);
}

void reg_aria_disp_set_still_csc_coeff2_still_csc_a10(mt_u16 data)
{
    reg_aria_disp_still_csc_coeff2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF2;
    d.bitc.still_csc_a10 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF2, d.all);
}

mt_u16  reg_aria_disp_get_still_csc_coeff2_still_csc_a10(void)
{
    return (*(volatile reg_aria_disp_still_csc_coeff2_t *)REG_ARIA_DISP_STILL_CSC_COEFF2).bitc.still_csc_a10;
}

void reg_aria_disp_set_still_csc_coeff2_still_csc_a02(mt_u16 data)
{
    reg_aria_disp_still_csc_coeff2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF2;
    d.bitc.still_csc_a02 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF2, d.all);
}

mt_u16  reg_aria_disp_get_still_csc_coeff2_still_csc_a02(void)
{
    return (*(volatile reg_aria_disp_still_csc_coeff2_t *)REG_ARIA_DISP_STILL_CSC_COEFF2).bitc.still_csc_a02;
}


/*!
  register ARIA_DISP_still_csc_coeff3 (read/write)
  */
void reg_aria_disp_set_still_csc_coeff3(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF3, data);
}

mt_u32  reg_aria_disp_get_still_csc_coeff3(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF3);
}

void reg_aria_disp_set_still_csc_coeff3_still_csc_a12(mt_u16 data)
{
    reg_aria_disp_still_csc_coeff3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF3;
    d.bitc.still_csc_a12 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF3, d.all);
}

mt_u16  reg_aria_disp_get_still_csc_coeff3_still_csc_a12(void)
{
    return (*(volatile reg_aria_disp_still_csc_coeff3_t *)REG_ARIA_DISP_STILL_CSC_COEFF3).bitc.still_csc_a12;
}

void reg_aria_disp_set_still_csc_coeff3_still_csc_a11(mt_u16 data)
{
    reg_aria_disp_still_csc_coeff3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF3;
    d.bitc.still_csc_a11 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF3, d.all);
}

mt_u16  reg_aria_disp_get_still_csc_coeff3_still_csc_a11(void)
{
    return (*(volatile reg_aria_disp_still_csc_coeff3_t *)REG_ARIA_DISP_STILL_CSC_COEFF3).bitc.still_csc_a11;
}


/*!
  register ARIA_DISP_still_csc_coeff4 (read/write)
  */
void reg_aria_disp_set_still_csc_coeff4(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF4, data);
}

mt_u32  reg_aria_disp_get_still_csc_coeff4(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF4);
}

void reg_aria_disp_set_still_csc_coeff4_still_csc_a21(mt_u16 data)
{
    reg_aria_disp_still_csc_coeff4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF4;
    d.bitc.still_csc_a21 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF4, d.all);
}

mt_u16  reg_aria_disp_get_still_csc_coeff4_still_csc_a21(void)
{
    return (*(volatile reg_aria_disp_still_csc_coeff4_t *)REG_ARIA_DISP_STILL_CSC_COEFF4).bitc.still_csc_a21;
}

void reg_aria_disp_set_still_csc_coeff4_still_csc_a20(mt_u16 data)
{
    reg_aria_disp_still_csc_coeff4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF4;
    d.bitc.still_csc_a20 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF4, d.all);
}

mt_u16  reg_aria_disp_get_still_csc_coeff4_still_csc_a20(void)
{
    return (*(volatile reg_aria_disp_still_csc_coeff4_t *)REG_ARIA_DISP_STILL_CSC_COEFF4).bitc.still_csc_a20;
}


/*!
  register ARIA_DISP_still_csc_coeff5 (read/write)
  */
void reg_aria_disp_set_still_csc_coeff5(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF5, data);
}

mt_u32  reg_aria_disp_get_still_csc_coeff5(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF5);
}

void reg_aria_disp_set_still_csc_coeff5_still_csc_a22(mt_u16 data)
{
    reg_aria_disp_still_csc_coeff5_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_STILL_CSC_COEFF5;
    d.bitc.still_csc_a22 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_STILL_CSC_COEFF5, d.all);
}

mt_u16  reg_aria_disp_get_still_csc_coeff5_still_csc_a22(void)
{
    return (*(volatile reg_aria_disp_still_csc_coeff5_t *)REG_ARIA_DISP_STILL_CSC_COEFF5).bitc.still_csc_a22;
}


/*!
  register ARIA_DISP_pres_cmd (read/write)
  */
void reg_aria_disp_set_pres_cmd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, data);
}

mt_u32  reg_aria_disp_get_pres_cmd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD);
}

void reg_aria_disp_set_pres_cmd_pres_hcoeff_load_en(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_hcoeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_hcoeff_load_en(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_hcoeff_load_en;
}

void reg_aria_disp_set_pres_cmd_pres_vcoeff_load_en(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_vcoeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_vcoeff_load_en(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_vcoeff_load_en;
}

void reg_aria_disp_set_pres_cmd_pres_input_interlace(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_input_interlace = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_input_interlace(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_input_interlace;
}

void reg_aria_disp_set_pres_cmd_pres_output_interlace(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_output_interlace = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_output_interlace(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_output_interlace;
}

void reg_aria_disp_set_pres_cmd_pres_2_picture(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_2_picture = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_2_picture(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_2_picture;
}

void reg_aria_disp_set_pres_cmd_pres_top_bot_inverse(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_top_bot_inverse = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_top_bot_inverse(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_top_bot_inverse;
}

void reg_aria_disp_set_pres_cmd_pres_endian_input(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_endian_input = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_endian_input(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_endian_input;
}

void reg_aria_disp_set_pres_cmd_pres_endian_output(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_endian_output = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_endian_output(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_endian_output;
}

void reg_aria_disp_set_pres_cmd_pres_wr_stride_sel(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_wr_stride_sel = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_wr_stride_sel(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_wr_stride_sel;
}

void reg_aria_disp_set_pres_cmd_pres_uv_change(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_uv_change = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_uv_change(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_uv_change;
}

void reg_aria_disp_set_pres_cmd_field0007(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.field0007 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_field0007(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.field0007;
}

void reg_aria_disp_set_pres_cmd_pres_hf_flag(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_hf_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_hf_flag(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_hf_flag;
}

void reg_aria_disp_set_pres_cmd_pres_vf_flag(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_vf_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_vf_flag(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_vf_flag;
}

void reg_aria_disp_set_pres_cmd_pres_ddr_rd_qos(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_ddr_rd_qos = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_ddr_rd_qos(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_ddr_rd_qos;
}

void reg_aria_disp_set_pres_cmd_pres_enable(mt_u8 data)
{
    reg_aria_disp_pres_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD;
    d.bitc.pres_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd_pres_enable(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_t *)REG_ARIA_DISP_PRES_CMD).bitc.pres_enable;
}


/*!
  register ARIA_DISP_pres_id (read/write)
  */
void reg_aria_disp_set_pres_id(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_ID, data);
}

mt_u32  reg_aria_disp_get_pres_id(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_ID);
}

void reg_aria_disp_set_pres_id_pres_id(mt_u32 data)
{
    reg_aria_disp_pres_id_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_ID;
    d.bitc.pres_id = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_ID, d.all);
}

mt_u32  reg_aria_disp_get_pres_id_pres_id(void)
{
    return (*(volatile reg_aria_disp_pres_id_t *)REG_ARIA_DISP_PRES_ID).bitc.pres_id;
}


/*!
  register ARIA_DISP_pres_cmd2 (read/write)
  */
void reg_aria_disp_set_pres_cmd2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, data);
}

mt_u32  reg_aria_disp_get_pres_cmd2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2);
}

void reg_aria_disp_set_pres_cmd2_pres_off_line(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_off_line = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_off_line(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_off_line;
}

void reg_aria_disp_set_pres_cmd2_pres_latch_or_not(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_latch_or_not = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_latch_or_not(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_latch_or_not;
}

void reg_aria_disp_set_pres_cmd2_pres_swrst_h(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_swrst_h = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_swrst_h(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_swrst_h;
}

void reg_aria_disp_set_pres_cmd2_pres_terminate(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_terminate = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_terminate(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_terminate;
}

void reg_aria_disp_set_pres_cmd2_pres_work_top(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_work_top = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_work_top(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_work_top;
}

void reg_aria_disp_set_pres_cmd2_pres_work_bot(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_work_bot = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_work_bot(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_work_bot;
}

void reg_aria_disp_set_pres_cmd2_pres_work_3d_1st(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_work_3d_1st = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_work_3d_1st(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_work_3d_1st;
}

void reg_aria_disp_set_pres_cmd2_pres_work_3d_2nd(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_work_3d_2nd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_work_3d_2nd(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_work_3d_2nd;
}

void reg_aria_disp_set_pres_cmd2_pres_latch_top(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_latch_top = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_latch_top(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_latch_top;
}

void reg_aria_disp_set_pres_cmd2_pres_latch_bot(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_latch_bot = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_latch_bot(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_latch_bot;
}

void reg_aria_disp_set_pres_cmd2_pres_latch_3d_1st(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_latch_3d_1st = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_latch_3d_1st(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_latch_3d_1st;
}

void reg_aria_disp_set_pres_cmd2_pres_latch_3d_2nd(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_latch_3d_2nd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_latch_3d_2nd(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_latch_3d_2nd;
}

void reg_aria_disp_set_pres_cmd2_pres_axi_w_limit(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_axi_w_limit = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_axi_w_limit(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_axi_w_limit;
}

void reg_aria_disp_set_pres_cmd2_pres_axi_r_limit(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_axi_r_limit = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_axi_r_limit(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_axi_r_limit;
}

void reg_aria_disp_set_pres_cmd2_pres_monitor_reload(mt_u8 data)
{
    reg_aria_disp_pres_cmd2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD2;
    d.bitc.pres_monitor_reload = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD2, d.all);
}

mt_u8   reg_aria_disp_get_pres_cmd2_pres_monitor_reload(void)
{
    return (*(volatile reg_aria_disp_pres_cmd2_t *)REG_ARIA_DISP_PRES_CMD2).bitc.pres_monitor_reload;
}


/*!
  register ARIA_DISP_pres_lum_raddr (read/write)
  */
void reg_aria_disp_set_pres_lum_raddr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_LUM_RADDR, data);
}

mt_u32  reg_aria_disp_get_pres_lum_raddr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_LUM_RADDR);
}

void reg_aria_disp_set_pres_lum_raddr_pres_luma_rd_addr(mt_u32 data)
{
    reg_aria_disp_pres_lum_raddr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_LUM_RADDR;
    d.bitc.pres_luma_rd_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_LUM_RADDR, d.all);
}

mt_u32  reg_aria_disp_get_pres_lum_raddr_pres_luma_rd_addr(void)
{
    return (*(volatile reg_aria_disp_pres_lum_raddr_t *)REG_ARIA_DISP_PRES_LUM_RADDR).bitc.pres_luma_rd_addr;
}


/*!
  register ARIA_DISP_pres_lum_raddr_2 (read/write)
  */
void reg_aria_disp_set_pres_lum_raddr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_LUM_RADDR_2, data);
}

mt_u32  reg_aria_disp_get_pres_lum_raddr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_LUM_RADDR_2);
}

void reg_aria_disp_set_pres_lum_raddr_2_pres_luma_rd_addr_2(mt_u32 data)
{
    reg_aria_disp_pres_lum_raddr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_LUM_RADDR_2;
    d.bitc.pres_luma_rd_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_LUM_RADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_pres_lum_raddr_2_pres_luma_rd_addr_2(void)
{
    return (*(volatile reg_aria_disp_pres_lum_raddr_2_t *)REG_ARIA_DISP_PRES_LUM_RADDR_2).bitc.pres_luma_rd_addr_2;
}


/*!
  register ARIA_DISP_pres_lum_waddr (read/write)
  */
void reg_aria_disp_set_pres_lum_waddr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_LUM_WADDR, data);
}

mt_u32  reg_aria_disp_get_pres_lum_waddr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_LUM_WADDR);
}

void reg_aria_disp_set_pres_lum_waddr_pres_luma_wr_addr(mt_u32 data)
{
    reg_aria_disp_pres_lum_waddr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_LUM_WADDR;
    d.bitc.pres_luma_wr_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_LUM_WADDR, d.all);
}

mt_u32  reg_aria_disp_get_pres_lum_waddr_pres_luma_wr_addr(void)
{
    return (*(volatile reg_aria_disp_pres_lum_waddr_t *)REG_ARIA_DISP_PRES_LUM_WADDR).bitc.pres_luma_wr_addr;
}


/*!
  register ARIA_DISP_pres_lum_waddr_2 (read/write)
  */
void reg_aria_disp_set_pres_lum_waddr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_LUM_WADDR_2, data);
}

mt_u32  reg_aria_disp_get_pres_lum_waddr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_LUM_WADDR_2);
}

void reg_aria_disp_set_pres_lum_waddr_2_pres_luma_wr_addr_2(mt_u32 data)
{
    reg_aria_disp_pres_lum_waddr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_LUM_WADDR_2;
    d.bitc.pres_luma_wr_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_LUM_WADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_pres_lum_waddr_2_pres_luma_wr_addr_2(void)
{
    return (*(volatile reg_aria_disp_pres_lum_waddr_2_t *)REG_ARIA_DISP_PRES_LUM_WADDR_2).bitc.pres_luma_wr_addr_2;
}


/*!
  register ARIA_DISP_pres_chm_raddr (read/write)
  */
void reg_aria_disp_set_pres_chm_raddr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CHM_RADDR, data);
}

mt_u32  reg_aria_disp_get_pres_chm_raddr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_CHM_RADDR);
}

void reg_aria_disp_set_pres_chm_raddr_pres_chroma_rd_addr(mt_u32 data)
{
    reg_aria_disp_pres_chm_raddr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CHM_RADDR;
    d.bitc.pres_chroma_rd_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CHM_RADDR, d.all);
}

mt_u32  reg_aria_disp_get_pres_chm_raddr_pres_chroma_rd_addr(void)
{
    return (*(volatile reg_aria_disp_pres_chm_raddr_t *)REG_ARIA_DISP_PRES_CHM_RADDR).bitc.pres_chroma_rd_addr;
}


/*!
  register ARIA_DISP_pres_chm_raddr_2 (read/write)
  */
void reg_aria_disp_set_pres_chm_raddr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CHM_RADDR_2, data);
}

mt_u32  reg_aria_disp_get_pres_chm_raddr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_CHM_RADDR_2);
}

void reg_aria_disp_set_pres_chm_raddr_2_pres_chroma_rd_addr_2(mt_u32 data)
{
    reg_aria_disp_pres_chm_raddr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CHM_RADDR_2;
    d.bitc.pres_chroma_rd_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CHM_RADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_pres_chm_raddr_2_pres_chroma_rd_addr_2(void)
{
    return (*(volatile reg_aria_disp_pres_chm_raddr_2_t *)REG_ARIA_DISP_PRES_CHM_RADDR_2).bitc.pres_chroma_rd_addr_2;
}


/*!
  register ARIA_DISP_pres_chm_waddr (read/write)
  */
void reg_aria_disp_set_pres_chm_waddr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CHM_WADDR, data);
}

mt_u32  reg_aria_disp_get_pres_chm_waddr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_CHM_WADDR);
}

void reg_aria_disp_set_pres_chm_waddr_pres_chroma_wr_addr(mt_u32 data)
{
    reg_aria_disp_pres_chm_waddr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CHM_WADDR;
    d.bitc.pres_chroma_wr_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CHM_WADDR, d.all);
}

mt_u32  reg_aria_disp_get_pres_chm_waddr_pres_chroma_wr_addr(void)
{
    return (*(volatile reg_aria_disp_pres_chm_waddr_t *)REG_ARIA_DISP_PRES_CHM_WADDR).bitc.pres_chroma_wr_addr;
}


/*!
  register ARIA_DISP_pres_chm_waddr_2 (read/write)
  */
void reg_aria_disp_set_pres_chm_waddr_2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CHM_WADDR_2, data);
}

mt_u32  reg_aria_disp_get_pres_chm_waddr_2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_CHM_WADDR_2);
}

void reg_aria_disp_set_pres_chm_waddr_2_pres_chroma_wr_addr_2(mt_u32 data)
{
    reg_aria_disp_pres_chm_waddr_2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CHM_WADDR_2;
    d.bitc.pres_chroma_wr_addr_2 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CHM_WADDR_2, d.all);
}

mt_u32  reg_aria_disp_get_pres_chm_waddr_2_pres_chroma_wr_addr_2(void)
{
    return (*(volatile reg_aria_disp_pres_chm_waddr_2_t *)REG_ARIA_DISP_PRES_CHM_WADDR_2).bitc.pres_chroma_wr_addr_2;
}


/*!
  register ARIA_DISP_pres_hcoeff_lum_addr (read/write)
  */
void reg_aria_disp_set_pres_hcoeff_lum_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_HCOEFF_LUM_ADDR, data);
}

mt_u32  reg_aria_disp_get_pres_hcoeff_lum_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_HCOEFF_LUM_ADDR);
}

void reg_aria_disp_set_pres_hcoeff_lum_addr_pres_hcoeff_lum_addr(mt_u32 data)
{
    reg_aria_disp_pres_hcoeff_lum_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_HCOEFF_LUM_ADDR;
    d.bitc.pres_hcoeff_lum_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_HCOEFF_LUM_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_pres_hcoeff_lum_addr_pres_hcoeff_lum_addr(void)
{
    return (*(volatile reg_aria_disp_pres_hcoeff_lum_addr_t *)REG_ARIA_DISP_PRES_HCOEFF_LUM_ADDR).bitc.pres_hcoeff_lum_addr;
}


/*!
  register ARIA_DISP_pres_vcoeff_lum_addr (read/write)
  */
void reg_aria_disp_set_pres_vcoeff_lum_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_VCOEFF_LUM_ADDR, data);
}

mt_u32  reg_aria_disp_get_pres_vcoeff_lum_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_VCOEFF_LUM_ADDR);
}

void reg_aria_disp_set_pres_vcoeff_lum_addr_pres_vcoeff_lum_addr(mt_u32 data)
{
    reg_aria_disp_pres_vcoeff_lum_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_VCOEFF_LUM_ADDR;
    d.bitc.pres_vcoeff_lum_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_VCOEFF_LUM_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_pres_vcoeff_lum_addr_pres_vcoeff_lum_addr(void)
{
    return (*(volatile reg_aria_disp_pres_vcoeff_lum_addr_t *)REG_ARIA_DISP_PRES_VCOEFF_LUM_ADDR).bitc.pres_vcoeff_lum_addr;
}


/*!
  register ARIA_DISP_pres_hcoeff_chm_addr (read/write)
  */
void reg_aria_disp_set_pres_hcoeff_chm_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_HCOEFF_CHM_ADDR, data);
}

mt_u32  reg_aria_disp_get_pres_hcoeff_chm_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_HCOEFF_CHM_ADDR);
}

void reg_aria_disp_set_pres_hcoeff_chm_addr_pres_hcoeff_chm_addr(mt_u32 data)
{
    reg_aria_disp_pres_hcoeff_chm_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_HCOEFF_CHM_ADDR;
    d.bitc.pres_hcoeff_chm_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_HCOEFF_CHM_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_pres_hcoeff_chm_addr_pres_hcoeff_chm_addr(void)
{
    return (*(volatile reg_aria_disp_pres_hcoeff_chm_addr_t *)REG_ARIA_DISP_PRES_HCOEFF_CHM_ADDR).bitc.pres_hcoeff_chm_addr;
}


/*!
  register ARIA_DISP_pres_vcoeff_chm_addr (read/write)
  */
void reg_aria_disp_set_pres_vcoeff_chm_addr(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_VCOEFF_CHM_ADDR, data);
}

mt_u32  reg_aria_disp_get_pres_vcoeff_chm_addr(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_VCOEFF_CHM_ADDR);
}

void reg_aria_disp_set_pres_vcoeff_chm_addr_pres_vcoeff_chm_addr(mt_u32 data)
{
    reg_aria_disp_pres_vcoeff_chm_addr_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_VCOEFF_CHM_ADDR;
    d.bitc.pres_vcoeff_chm_addr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_VCOEFF_CHM_ADDR, d.all);
}

mt_u32  reg_aria_disp_get_pres_vcoeff_chm_addr_pres_vcoeff_chm_addr(void)
{
    return (*(volatile reg_aria_disp_pres_vcoeff_chm_addr_t *)REG_ARIA_DISP_PRES_VCOEFF_CHM_ADDR).bitc.pres_vcoeff_chm_addr;
}


/*!
  register ARIA_DISP_pres_ffr_threshold (read/write)
  */
void reg_aria_disp_set_pres_ffr_threshold(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_FFR_THRESHOLD, data);
}

mt_u32  reg_aria_disp_get_pres_ffr_threshold(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_FFR_THRESHOLD);
}

void reg_aria_disp_set_pres_ffr_threshold_pres_urgent_num_ddr_wr(mt_u8 data)
{
    reg_aria_disp_pres_ffr_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_FFR_THRESHOLD;
    d.bitc.pres_urgent_num_ddr_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_FFR_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_pres_ffr_threshold_pres_urgent_num_ddr_wr(void)
{
    return (*(volatile reg_aria_disp_pres_ffr_threshold_t *)REG_ARIA_DISP_PRES_FFR_THRESHOLD).bitc.pres_urgent_num_ddr_wr;
}

void reg_aria_disp_set_pres_ffr_threshold_pres_weight_num_ddr_wr(mt_u8 data)
{
    reg_aria_disp_pres_ffr_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_FFR_THRESHOLD;
    d.bitc.pres_weight_num_ddr_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_FFR_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_pres_ffr_threshold_pres_weight_num_ddr_wr(void)
{
    return (*(volatile reg_aria_disp_pres_ffr_threshold_t *)REG_ARIA_DISP_PRES_FFR_THRESHOLD).bitc.pres_weight_num_ddr_wr;
}

void reg_aria_disp_set_pres_ffr_threshold_pres_urgent_num_ddr_rd(mt_u8 data)
{
    reg_aria_disp_pres_ffr_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_FFR_THRESHOLD;
    d.bitc.pres_urgent_num_ddr_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_FFR_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_pres_ffr_threshold_pres_urgent_num_ddr_rd(void)
{
    return (*(volatile reg_aria_disp_pres_ffr_threshold_t *)REG_ARIA_DISP_PRES_FFR_THRESHOLD).bitc.pres_urgent_num_ddr_rd;
}

void reg_aria_disp_set_pres_ffr_threshold_pres_weight_num_ddr_rd(mt_u8 data)
{
    reg_aria_disp_pres_ffr_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_FFR_THRESHOLD;
    d.bitc.pres_weight_num_ddr_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_FFR_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_pres_ffr_threshold_pres_weight_num_ddr_rd(void)
{
    return (*(volatile reg_aria_disp_pres_ffr_threshold_t *)REG_ARIA_DISP_PRES_FFR_THRESHOLD).bitc.pres_weight_num_ddr_rd;
}


/*!
  register ARIA_DISP_pres_ffw_threshold (read/write)
  */
void reg_aria_disp_set_pres_ffw_threshold(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_FFW_THRESHOLD, data);
}

mt_u32  reg_aria_disp_get_pres_ffw_threshold(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_FFW_THRESHOLD);
}

void reg_aria_disp_set_pres_ffw_threshold_field0000(mt_u8 data)
{
    reg_aria_disp_pres_ffw_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_FFW_THRESHOLD;
    d.bitc.field0000 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_FFW_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_pres_ffw_threshold_field0000(void)
{
    return (*(volatile reg_aria_disp_pres_ffw_threshold_t *)REG_ARIA_DISP_PRES_FFW_THRESHOLD).bitc.field0000;
}


/*!
  register ARIA_DISP_pres_ddr_wr_stride (read/write)
  */
void reg_aria_disp_set_pres_ddr_wr_stride(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_DDR_WR_STRIDE, data);
}

mt_u32  reg_aria_disp_get_pres_ddr_wr_stride(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_DDR_WR_STRIDE);
}

void reg_aria_disp_set_pres_ddr_wr_stride_pres_ddr_wr_line_stride(mt_u16 data)
{
    reg_aria_disp_pres_ddr_wr_stride_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_DDR_WR_STRIDE;
    d.bitc.pres_ddr_wr_line_stride = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_DDR_WR_STRIDE, d.all);
}

mt_u16  reg_aria_disp_get_pres_ddr_wr_stride_pres_ddr_wr_line_stride(void)
{
    return (*(volatile reg_aria_disp_pres_ddr_wr_stride_t *)REG_ARIA_DISP_PRES_DDR_WR_STRIDE).bitc.pres_ddr_wr_line_stride;
}


/*!
  register ARIA_DISP_pres_irq_en (read/write)
  */
void reg_aria_disp_set_pres_irq_en(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_IRQ_EN, data);
}

mt_u32  reg_aria_disp_get_pres_irq_en(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_IRQ_EN);
}

void reg_aria_disp_set_pres_irq_en_pres_end_irq_en(mt_u8 data)
{
    reg_aria_disp_pres_irq_en_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_IRQ_EN;
    d.bitc.pres_end_irq_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_IRQ_EN, d.all);
}

mt_u8   reg_aria_disp_get_pres_irq_en_pres_end_irq_en(void)
{
    return (*(volatile reg_aria_disp_pres_irq_en_t *)REG_ARIA_DISP_PRES_IRQ_EN).bitc.pres_end_irq_en;
}


/*!
  register ARIA_DISP_pres_irq (read/write)
  */
void reg_aria_disp_set_pres_irq(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_IRQ, data);
}

mt_u32  reg_aria_disp_get_pres_irq(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_IRQ);
}

void reg_aria_disp_set_pres_irq_pres_end_irq(mt_u8 data)
{
    reg_aria_disp_pres_irq_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_IRQ;
    d.bitc.pres_end_irq = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_IRQ, d.all);
}

mt_u8   reg_aria_disp_get_pres_irq_pres_end_irq(void)
{
    return (*(volatile reg_aria_disp_pres_irq_t *)REG_ARIA_DISP_PRES_IRQ).bitc.pres_end_irq;
}


/*!
  register ARIA_DISP_pres_src_size (read/write)
  */
void reg_aria_disp_set_pres_src_size(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_SRC_SIZE, data);
}

mt_u32  reg_aria_disp_get_pres_src_size(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_SRC_SIZE);
}

void reg_aria_disp_set_pres_src_size_pres_src_vsize(mt_u16 data)
{
    reg_aria_disp_pres_src_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_SRC_SIZE;
    d.bitc.pres_src_vsize = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_SRC_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_pres_src_size_pres_src_vsize(void)
{
    return (*(volatile reg_aria_disp_pres_src_size_t *)REG_ARIA_DISP_PRES_SRC_SIZE).bitc.pres_src_vsize;
}

void reg_aria_disp_set_pres_src_size_pres_src_hsize(mt_u16 data)
{
    reg_aria_disp_pres_src_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_SRC_SIZE;
    d.bitc.pres_src_hsize = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_SRC_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_pres_src_size_pres_src_hsize(void)
{
    return (*(volatile reg_aria_disp_pres_src_size_t *)REG_ARIA_DISP_PRES_SRC_SIZE).bitc.pres_src_hsize;
}


/*!
  register ARIA_DISP_pres_dst_size (read/write)
  */
void reg_aria_disp_set_pres_dst_size(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_DST_SIZE, data);
}

mt_u32  reg_aria_disp_get_pres_dst_size(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_DST_SIZE);
}

void reg_aria_disp_set_pres_dst_size_pres_dst_vsize(mt_u16 data)
{
    reg_aria_disp_pres_dst_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_DST_SIZE;
    d.bitc.pres_dst_vsize = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_DST_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_pres_dst_size_pres_dst_vsize(void)
{
    return (*(volatile reg_aria_disp_pres_dst_size_t *)REG_ARIA_DISP_PRES_DST_SIZE).bitc.pres_dst_vsize;
}

void reg_aria_disp_set_pres_dst_size_pres_dst_hsize(mt_u16 data)
{
    reg_aria_disp_pres_dst_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_DST_SIZE;
    d.bitc.pres_dst_hsize = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_DST_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_pres_dst_size_pres_dst_hsize(void)
{
    return (*(volatile reg_aria_disp_pres_dst_size_t *)REG_ARIA_DISP_PRES_DST_SIZE).bitc.pres_dst_hsize;
}


/*!
  register ARIA_DISP_pres_hratio (read/write)
  */
void reg_aria_disp_set_pres_hratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_HRATIO, data);
}

mt_u32  reg_aria_disp_get_pres_hratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_HRATIO);
}

void reg_aria_disp_set_pres_hratio_pres_h_ratio_fra(mt_u16 data)
{
    reg_aria_disp_pres_hratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_HRATIO;
    d.bitc.pres_h_ratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_HRATIO, d.all);
}

mt_u16  reg_aria_disp_get_pres_hratio_pres_h_ratio_fra(void)
{
    return (*(volatile reg_aria_disp_pres_hratio_t *)REG_ARIA_DISP_PRES_HRATIO).bitc.pres_h_ratio_fra;
}

void reg_aria_disp_set_pres_hratio_pres_h_ratio_int(mt_u8 data)
{
    reg_aria_disp_pres_hratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_HRATIO;
    d.bitc.pres_h_ratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_HRATIO, d.all);
}

mt_u8   reg_aria_disp_get_pres_hratio_pres_h_ratio_int(void)
{
    return (*(volatile reg_aria_disp_pres_hratio_t *)REG_ARIA_DISP_PRES_HRATIO).bitc.pres_h_ratio_int;
}


/*!
  register ARIA_DISP_pres_vratio (read/write)
  */
void reg_aria_disp_set_pres_vratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_VRATIO, data);
}

mt_u32  reg_aria_disp_get_pres_vratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_VRATIO);
}

void reg_aria_disp_set_pres_vratio_pres_v_ratio_fra(mt_u16 data)
{
    reg_aria_disp_pres_vratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_VRATIO;
    d.bitc.pres_v_ratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_VRATIO, d.all);
}

mt_u16  reg_aria_disp_get_pres_vratio_pres_v_ratio_fra(void)
{
    return (*(volatile reg_aria_disp_pres_vratio_t *)REG_ARIA_DISP_PRES_VRATIO).bitc.pres_v_ratio_fra;
}

void reg_aria_disp_set_pres_vratio_pres_v_ratio_int(mt_u16 data)
{
    reg_aria_disp_pres_vratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_VRATIO;
    d.bitc.pres_v_ratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_VRATIO, d.all);
}

mt_u16  reg_aria_disp_get_pres_vratio_pres_v_ratio_int(void)
{
    return (*(volatile reg_aria_disp_pres_vratio_t *)REG_ARIA_DISP_PRES_VRATIO).bitc.pres_v_ratio_int;
}


/*!
  register ARIA_DISP_pres_hinit (read/write)
  */
void reg_aria_disp_set_pres_hinit(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_HINIT, data);
}

mt_u32  reg_aria_disp_get_pres_hinit(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_HINIT);
}

void reg_aria_disp_set_pres_hinit_pres_h_start_fra(mt_u16 data)
{
    reg_aria_disp_pres_hinit_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_HINIT;
    d.bitc.pres_h_start_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_HINIT, d.all);
}

mt_u16  reg_aria_disp_get_pres_hinit_pres_h_start_fra(void)
{
    return (*(volatile reg_aria_disp_pres_hinit_t *)REG_ARIA_DISP_PRES_HINIT).bitc.pres_h_start_fra;
}


/*!
  register ARIA_DISP_pres_vinit (read/write)
  */
void reg_aria_disp_set_pres_vinit(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_VINIT, data);
}

mt_u32  reg_aria_disp_get_pres_vinit(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_VINIT);
}

void reg_aria_disp_set_pres_vinit_pres_v_start_fra(mt_u16 data)
{
    reg_aria_disp_pres_vinit_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_VINIT;
    d.bitc.pres_v_start_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_VINIT, d.all);
}

mt_u16  reg_aria_disp_get_pres_vinit_pres_v_start_fra(void)
{
    return (*(volatile reg_aria_disp_pres_vinit_t *)REG_ARIA_DISP_PRES_VINIT).bitc.pres_v_start_fra;
}

void reg_aria_disp_set_pres_vinit_pres_v_start_int(mt_u8 data)
{
    reg_aria_disp_pres_vinit_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_VINIT;
    d.bitc.pres_v_start_int = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_VINIT, d.all);
}

mt_u8   reg_aria_disp_get_pres_vinit_pres_v_start_int(void)
{
    return (*(volatile reg_aria_disp_pres_vinit_t *)REG_ARIA_DISP_PRES_VINIT).bitc.pres_v_start_int;
}

void reg_aria_disp_set_pres_vinit_pres_v_start_fra_bot(mt_u16 data)
{
    reg_aria_disp_pres_vinit_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_VINIT;
    d.bitc.pres_v_start_fra_bot = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_VINIT, d.all);
}

mt_u16  reg_aria_disp_get_pres_vinit_pres_v_start_fra_bot(void)
{
    return (*(volatile reg_aria_disp_pres_vinit_t *)REG_ARIA_DISP_PRES_VINIT).bitc.pres_v_start_fra_bot;
}

void reg_aria_disp_set_pres_vinit_pres_v_start_int_bot(mt_u8 data)
{
    reg_aria_disp_pres_vinit_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_VINIT;
    d.bitc.pres_v_start_int_bot = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_VINIT, d.all);
}

mt_u8   reg_aria_disp_get_pres_vinit_pres_v_start_int_bot(void)
{
    return (*(volatile reg_aria_disp_pres_vinit_t *)REG_ARIA_DISP_PRES_VINIT).bitc.pres_v_start_int_bot;
}


/*!
  register ARIA_DISP_pres_status (read/write)
  */
void reg_aria_disp_set_pres_status(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS, data);
}

mt_u32  reg_aria_disp_get_pres_status(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS);
}

void reg_aria_disp_set_pres_status_pres_end_irq(mt_u8 data)
{
    reg_aria_disp_pres_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS;
    d.bitc.pres_end_irq = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_pres_status_pres_end_irq(void)
{
    return (*(volatile reg_aria_disp_pres_status_t *)REG_ARIA_DISP_PRES_STATUS).bitc.pres_end_irq;
}

void reg_aria_disp_set_pres_status_pres_wrddr_error(mt_u8 data)
{
    reg_aria_disp_pres_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS;
    d.bitc.pres_wrddr_error = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_pres_status_pres_wrddr_error(void)
{
    return (*(volatile reg_aria_disp_pres_status_t *)REG_ARIA_DISP_PRES_STATUS).bitc.pres_wrddr_error;
}

void reg_aria_disp_set_pres_status_pres_wrfifo_full(mt_u8 data)
{
    reg_aria_disp_pres_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS;
    d.bitc.pres_wrfifo_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_pres_status_pres_wrfifo_full(void)
{
    return (*(volatile reg_aria_disp_pres_status_t *)REG_ARIA_DISP_PRES_STATUS).bitc.pres_wrfifo_full;
}

void reg_aria_disp_set_pres_status_pres_wrfifo_empty(mt_u8 data)
{
    reg_aria_disp_pres_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS;
    d.bitc.pres_wrfifo_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_pres_status_pres_wrfifo_empty(void)
{
    return (*(volatile reg_aria_disp_pres_status_t *)REG_ARIA_DISP_PRES_STATUS).bitc.pres_wrfifo_empty;
}

void reg_aria_disp_set_pres_status_pres_soft_rst_done(mt_u8 data)
{
    reg_aria_disp_pres_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS;
    d.bitc.pres_soft_rst_done = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_pres_status_pres_soft_rst_done(void)
{
    return (*(volatile reg_aria_disp_pres_status_t *)REG_ARIA_DISP_PRES_STATUS).bitc.pres_soft_rst_done;
}

void reg_aria_disp_set_pres_status_pres_axi_w_done(mt_u8 data)
{
    reg_aria_disp_pres_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS;
    d.bitc.pres_axi_w_done = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_pres_status_pres_axi_w_done(void)
{
    return (*(volatile reg_aria_disp_pres_status_t *)REG_ARIA_DISP_PRES_STATUS).bitc.pres_axi_w_done;
}

void reg_aria_disp_set_pres_status_pres_axi_r_done(mt_u8 data)
{
    reg_aria_disp_pres_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS;
    d.bitc.pres_axi_r_done = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_pres_status_pres_axi_r_done(void)
{
    return (*(volatile reg_aria_disp_pres_status_t *)REG_ARIA_DISP_PRES_STATUS).bitc.pres_axi_r_done;
}

void reg_aria_disp_set_pres_status_pres_latency_overflow_rd(mt_u8 data)
{
    reg_aria_disp_pres_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS;
    d.bitc.pres_latency_overflow_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_pres_status_pres_latency_overflow_rd(void)
{
    return (*(volatile reg_aria_disp_pres_status_t *)REG_ARIA_DISP_PRES_STATUS).bitc.pres_latency_overflow_rd;
}

void reg_aria_disp_set_pres_status_pres_latency_overflow_wr(mt_u8 data)
{
    reg_aria_disp_pres_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS;
    d.bitc.pres_latency_overflow_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_pres_status_pres_latency_overflow_wr(void)
{
    return (*(volatile reg_aria_disp_pres_status_t *)REG_ARIA_DISP_PRES_STATUS).bitc.pres_latency_overflow_wr;
}

void reg_aria_disp_set_pres_status_pres_free(mt_u8 data)
{
    reg_aria_disp_pres_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS;
    d.bitc.pres_free = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_pres_status_pres_free(void)
{
    return (*(volatile reg_aria_disp_pres_status_t *)REG_ARIA_DISP_PRES_STATUS).bitc.pres_free;
}


/*!
  register ARIA_DISP_pres_tile_rowjump_00 (read/write)
  */
void reg_aria_disp_set_pres_tile_rowjump_00(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_00, data);
}

mt_u32  reg_aria_disp_get_pres_tile_rowjump_00(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_00);
}

void reg_aria_disp_set_pres_tile_rowjump_00_pres_tile_rowjump_00(mt_u32 data)
{
    reg_aria_disp_pres_tile_rowjump_00_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_00;
    d.bitc.pres_tile_rowjump_00 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_00, d.all);
}

mt_u32  reg_aria_disp_get_pres_tile_rowjump_00_pres_tile_rowjump_00(void)
{
    return (*(volatile reg_aria_disp_pres_tile_rowjump_00_t *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_00).bitc.pres_tile_rowjump_00;
}


/*!
  register ARIA_DISP_pres_tile_rowjump_01 (read/write)
  */
void reg_aria_disp_set_pres_tile_rowjump_01(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_01, data);
}

mt_u32  reg_aria_disp_get_pres_tile_rowjump_01(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_01);
}

void reg_aria_disp_set_pres_tile_rowjump_01_pres_tile_rowjump_01(mt_u32 data)
{
    reg_aria_disp_pres_tile_rowjump_01_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_01;
    d.bitc.pres_tile_rowjump_01 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_01, d.all);
}

mt_u32  reg_aria_disp_get_pres_tile_rowjump_01_pres_tile_rowjump_01(void)
{
    return (*(volatile reg_aria_disp_pres_tile_rowjump_01_t *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_01).bitc.pres_tile_rowjump_01;
}


/*!
  register ARIA_DISP_pres_tile_rowjump_10 (read/write)
  */
void reg_aria_disp_set_pres_tile_rowjump_10(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_10, data);
}

mt_u32  reg_aria_disp_get_pres_tile_rowjump_10(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_10);
}

void reg_aria_disp_set_pres_tile_rowjump_10_pres_tile_rowjump_10(mt_u32 data)
{
    reg_aria_disp_pres_tile_rowjump_10_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_10;
    d.bitc.pres_tile_rowjump_10 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_10, d.all);
}

mt_u32  reg_aria_disp_get_pres_tile_rowjump_10_pres_tile_rowjump_10(void)
{
    return (*(volatile reg_aria_disp_pres_tile_rowjump_10_t *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_10).bitc.pres_tile_rowjump_10;
}


/*!
  register ARIA_DISP_pres_tile_rowjump_11 (read/write)
  */
void reg_aria_disp_set_pres_tile_rowjump_11(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_11, data);
}

mt_u32  reg_aria_disp_get_pres_tile_rowjump_11(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_11);
}

void reg_aria_disp_set_pres_tile_rowjump_11_pres_tile_rowjump_11(mt_u32 data)
{
    reg_aria_disp_pres_tile_rowjump_11_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_11;
    d.bitc.pres_tile_rowjump_11 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_11, d.all);
}

mt_u32  reg_aria_disp_get_pres_tile_rowjump_11_pres_tile_rowjump_11(void)
{
    return (*(volatile reg_aria_disp_pres_tile_rowjump_11_t *)REG_ARIA_DISP_PRES_TILE_ROWJUMP_11).bitc.pres_tile_rowjump_11;
}


/*!
  register ARIA_DISP_pres_tile_para (read/write)
  */
void reg_aria_disp_set_pres_tile_para(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_TILE_PARA, data);
}

mt_u32  reg_aria_disp_get_pres_tile_para(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_TILE_PARA);
}

void reg_aria_disp_set_pres_tile_para_pres_col_size_mode(mt_u8 data)
{
    reg_aria_disp_pres_tile_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_TILE_PARA;
    d.bitc.pres_col_size_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_TILE_PARA, d.all);
}

mt_u8   reg_aria_disp_get_pres_tile_para_pres_col_size_mode(void)
{
    return (*(volatile reg_aria_disp_pres_tile_para_t *)REG_ARIA_DISP_PRES_TILE_PARA).bitc.pres_col_size_mode;
}

void reg_aria_disp_set_pres_tile_para_pres_field_picture(mt_u8 data)
{
    reg_aria_disp_pres_tile_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_TILE_PARA;
    d.bitc.pres_field_picture = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_TILE_PARA, d.all);
}

mt_u8   reg_aria_disp_get_pres_tile_para_pres_field_picture(void)
{
    return (*(volatile reg_aria_disp_pres_tile_para_t *)REG_ARIA_DISP_PRES_TILE_PARA).bitc.pres_field_picture;
}

void reg_aria_disp_set_pres_tile_para_pres_hd_map_mode(mt_u8 data)
{
    reg_aria_disp_pres_tile_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_TILE_PARA;
    d.bitc.pres_hd_map_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_TILE_PARA, d.all);
}

mt_u8   reg_aria_disp_get_pres_tile_para_pres_hd_map_mode(void)
{
    return (*(volatile reg_aria_disp_pres_tile_para_t *)REG_ARIA_DISP_PRES_TILE_PARA).bitc.pres_hd_map_mode;
}

void reg_aria_disp_set_pres_tile_para_pres_tile_config(mt_u8 data)
{
    reg_aria_disp_pres_tile_para_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_TILE_PARA;
    d.bitc.pres_tile_config = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_TILE_PARA, d.all);
}

mt_u8   reg_aria_disp_get_pres_tile_para_pres_tile_config(void)
{
    return (*(volatile reg_aria_disp_pres_tile_para_t *)REG_ARIA_DISP_PRES_TILE_PARA).bitc.pres_tile_config;
}


/*!
  register ARIA_DISP_pres_cmd_ack_latency_avg (read/write)
  */
void reg_aria_disp_set_pres_cmd_ack_latency_avg(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_AVG, data);
}

mt_u32  reg_aria_disp_get_pres_cmd_ack_latency_avg(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_AVG);
}

void reg_aria_disp_set_pres_cmd_ack_latency_avg_pres_cmd_ack_latency_avg_rd(mt_u16 data)
{
    reg_aria_disp_pres_cmd_ack_latency_avg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_AVG;
    d.bitc.pres_cmd_ack_latency_avg_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_AVG, d.all);
}

mt_u16  reg_aria_disp_get_pres_cmd_ack_latency_avg_pres_cmd_ack_latency_avg_rd(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_ack_latency_avg_t *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_AVG).bitc.pres_cmd_ack_latency_avg_rd;
}

void reg_aria_disp_set_pres_cmd_ack_latency_avg_pres_cmd_ack_latency_avg_wr(mt_u16 data)
{
    reg_aria_disp_pres_cmd_ack_latency_avg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_AVG;
    d.bitc.pres_cmd_ack_latency_avg_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_AVG, d.all);
}

mt_u16  reg_aria_disp_get_pres_cmd_ack_latency_avg_pres_cmd_ack_latency_avg_wr(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_ack_latency_avg_t *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_AVG).bitc.pres_cmd_ack_latency_avg_wr;
}


/*!
  register ARIA_DISP_pres_cmd_dat_latency_avg (read/write)
  */
void reg_aria_disp_set_pres_cmd_dat_latency_avg(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_AVG, data);
}

mt_u32  reg_aria_disp_get_pres_cmd_dat_latency_avg(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_AVG);
}

void reg_aria_disp_set_pres_cmd_dat_latency_avg_pres_cmd_dat_latency_avg_rd(mt_u16 data)
{
    reg_aria_disp_pres_cmd_dat_latency_avg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_AVG;
    d.bitc.pres_cmd_dat_latency_avg_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_AVG, d.all);
}

mt_u16  reg_aria_disp_get_pres_cmd_dat_latency_avg_pres_cmd_dat_latency_avg_rd(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_dat_latency_avg_t *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_AVG).bitc.pres_cmd_dat_latency_avg_rd;
}

void reg_aria_disp_set_pres_cmd_dat_latency_avg_pres_cmd_dat_latency_avg_wr(mt_u16 data)
{
    reg_aria_disp_pres_cmd_dat_latency_avg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_AVG;
    d.bitc.pres_cmd_dat_latency_avg_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_AVG, d.all);
}

mt_u16  reg_aria_disp_get_pres_cmd_dat_latency_avg_pres_cmd_dat_latency_avg_wr(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_dat_latency_avg_t *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_AVG).bitc.pres_cmd_dat_latency_avg_wr;
}


/*!
  register ARIA_DISP_pres_datlast_latency_avg (read/write)
  */
void reg_aria_disp_set_pres_datlast_latency_avg(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_AVG, data);
}

mt_u32  reg_aria_disp_get_pres_datlast_latency_avg(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_AVG);
}

void reg_aria_disp_set_pres_datlast_latency_avg_pres_datlast_latency_avg_rd(mt_u16 data)
{
    reg_aria_disp_pres_datlast_latency_avg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_AVG;
    d.bitc.pres_datlast_latency_avg_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_AVG, d.all);
}

mt_u16  reg_aria_disp_get_pres_datlast_latency_avg_pres_datlast_latency_avg_rd(void)
{
    return (*(volatile reg_aria_disp_pres_datlast_latency_avg_t *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_AVG).bitc.pres_datlast_latency_avg_rd;
}

void reg_aria_disp_set_pres_datlast_latency_avg_pres_datlast_latency_avg_wr(mt_u16 data)
{
    reg_aria_disp_pres_datlast_latency_avg_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_AVG;
    d.bitc.pres_datlast_latency_avg_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_AVG, d.all);
}

mt_u16  reg_aria_disp_get_pres_datlast_latency_avg_pres_datlast_latency_avg_wr(void)
{
    return (*(volatile reg_aria_disp_pres_datlast_latency_avg_t *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_AVG).bitc.pres_datlast_latency_avg_wr;
}


/*!
  register ARIA_DISP_pres_cmd_ack_latency_max (read/write)
  */
void reg_aria_disp_set_pres_cmd_ack_latency_max(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_MAX, data);
}

mt_u32  reg_aria_disp_get_pres_cmd_ack_latency_max(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_MAX);
}

void reg_aria_disp_set_pres_cmd_ack_latency_max_pres_cmd_ack_latency_max_rd(mt_u16 data)
{
    reg_aria_disp_pres_cmd_ack_latency_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_MAX;
    d.bitc.pres_cmd_ack_latency_max_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_MAX, d.all);
}

mt_u16  reg_aria_disp_get_pres_cmd_ack_latency_max_pres_cmd_ack_latency_max_rd(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_ack_latency_max_t *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_MAX).bitc.pres_cmd_ack_latency_max_rd;
}

void reg_aria_disp_set_pres_cmd_ack_latency_max_pres_cmd_ack_latency_max_wr(mt_u16 data)
{
    reg_aria_disp_pres_cmd_ack_latency_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_MAX;
    d.bitc.pres_cmd_ack_latency_max_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_MAX, d.all);
}

mt_u16  reg_aria_disp_get_pres_cmd_ack_latency_max_pres_cmd_ack_latency_max_wr(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_ack_latency_max_t *)REG_ARIA_DISP_PRES_CMD_ACK_LATENCY_MAX).bitc.pres_cmd_ack_latency_max_wr;
}


/*!
  register ARIA_DISP_pres_cmd_dat_latency_max (read/write)
  */
void reg_aria_disp_set_pres_cmd_dat_latency_max(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_MAX, data);
}

mt_u32  reg_aria_disp_get_pres_cmd_dat_latency_max(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_MAX);
}

void reg_aria_disp_set_pres_cmd_dat_latency_max_pres_cmd_dat_latency_max_rd(mt_u16 data)
{
    reg_aria_disp_pres_cmd_dat_latency_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_MAX;
    d.bitc.pres_cmd_dat_latency_max_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_MAX, d.all);
}

mt_u16  reg_aria_disp_get_pres_cmd_dat_latency_max_pres_cmd_dat_latency_max_rd(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_dat_latency_max_t *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_MAX).bitc.pres_cmd_dat_latency_max_rd;
}

void reg_aria_disp_set_pres_cmd_dat_latency_max_pres_cmd_dat_latency_max_wr(mt_u16 data)
{
    reg_aria_disp_pres_cmd_dat_latency_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_MAX;
    d.bitc.pres_cmd_dat_latency_max_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_MAX, d.all);
}

mt_u16  reg_aria_disp_get_pres_cmd_dat_latency_max_pres_cmd_dat_latency_max_wr(void)
{
    return (*(volatile reg_aria_disp_pres_cmd_dat_latency_max_t *)REG_ARIA_DISP_PRES_CMD_DAT_LATENCY_MAX).bitc.pres_cmd_dat_latency_max_wr;
}


/*!
  register ARIA_DISP_pres_datlast_latency_max (read/write)
  */
void reg_aria_disp_set_pres_datlast_latency_max(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_MAX, data);
}

mt_u32  reg_aria_disp_get_pres_datlast_latency_max(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_MAX);
}

void reg_aria_disp_set_pres_datlast_latency_max_pres_datlast_latency_max_rd(mt_u16 data)
{
    reg_aria_disp_pres_datlast_latency_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_MAX;
    d.bitc.pres_datlast_latency_max_rd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_MAX, d.all);
}

mt_u16  reg_aria_disp_get_pres_datlast_latency_max_pres_datlast_latency_max_rd(void)
{
    return (*(volatile reg_aria_disp_pres_datlast_latency_max_t *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_MAX).bitc.pres_datlast_latency_max_rd;
}

void reg_aria_disp_set_pres_datlast_latency_max_pres_datlast_latency_max_wr(mt_u16 data)
{
    reg_aria_disp_pres_datlast_latency_max_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_MAX;
    d.bitc.pres_datlast_latency_max_wr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_MAX, d.all);
}

mt_u16  reg_aria_disp_get_pres_datlast_latency_max_pres_datlast_latency_max_wr(void)
{
    return (*(volatile reg_aria_disp_pres_datlast_latency_max_t *)REG_ARIA_DISP_PRES_DATLAST_LATENCY_MAX).bitc.pres_datlast_latency_max_wr;
}


/*!
  register ARIA_DISP_pres_status1 (read/write)
  */
void reg_aria_disp_set_pres_status1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS1, data);
}

mt_u32  reg_aria_disp_get_pres_status1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS1);
}

void reg_aria_disp_set_pres_status1_pres_vf_line_cnt(mt_u16 data)
{
    reg_aria_disp_pres_status1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS1;
    d.bitc.pres_vf_line_cnt = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS1, d.all);
}

mt_u16  reg_aria_disp_get_pres_status1_pres_vf_line_cnt(void)
{
    return (*(volatile reg_aria_disp_pres_status1_t *)REG_ARIA_DISP_PRES_STATUS1).bitc.pres_vf_line_cnt;
}

void reg_aria_disp_set_pres_status1_pres_hf_line_cnt(mt_u16 data)
{
    reg_aria_disp_pres_status1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS1;
    d.bitc.pres_hf_line_cnt = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS1, d.all);
}

mt_u16  reg_aria_disp_get_pres_status1_pres_hf_line_cnt(void)
{
    return (*(volatile reg_aria_disp_pres_status1_t *)REG_ARIA_DISP_PRES_STATUS1).bitc.pres_hf_line_cnt;
}


/*!
  register ARIA_DISP_pres_status2 (read/write)
  */
void reg_aria_disp_set_pres_status2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS2, data);
}

mt_u32  reg_aria_disp_get_pres_status2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS2);
}

void reg_aria_disp_set_pres_status2_pres_ffwr_lne_cnt(mt_u16 data)
{
    reg_aria_disp_pres_status2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS2;
    d.bitc.pres_ffwr_lne_cnt = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS2, d.all);
}

mt_u16  reg_aria_disp_get_pres_status2_pres_ffwr_lne_cnt(void)
{
    return (*(volatile reg_aria_disp_pres_status2_t *)REG_ARIA_DISP_PRES_STATUS2).bitc.pres_ffwr_lne_cnt;
}

void reg_aria_disp_set_pres_status2_pres_ffwr_pxl_cnt(mt_u8 data)
{
    reg_aria_disp_pres_status2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS2;
    d.bitc.pres_ffwr_pxl_cnt = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS2, d.all);
}

mt_u8   reg_aria_disp_get_pres_status2_pres_ffwr_pxl_cnt(void)
{
    return (*(volatile reg_aria_disp_pres_status2_t *)REG_ARIA_DISP_PRES_STATUS2).bitc.pres_ffwr_pxl_cnt;
}


/*!
  register ARIA_DISP_pres_status3 (read/write)
  */
void reg_aria_disp_set_pres_status3(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS3, data);
}

mt_u32  reg_aria_disp_get_pres_status3(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS3);
}

void reg_aria_disp_set_pres_status3_pres_ffrd_data_cnt(mt_u8 data)
{
    reg_aria_disp_pres_status3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS3;
    d.bitc.pres_ffrd_data_cnt = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS3, d.all);
}

mt_u8   reg_aria_disp_get_pres_status3_pres_ffrd_data_cnt(void)
{
    return (*(volatile reg_aria_disp_pres_status3_t *)REG_ARIA_DISP_PRES_STATUS3).bitc.pres_ffrd_data_cnt;
}

void reg_aria_disp_set_pres_status3_pres_rdreq_stt(mt_u16 data)
{
    reg_aria_disp_pres_status3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS3;
    d.bitc.pres_rdreq_stt = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS3, d.all);
}

mt_u16  reg_aria_disp_get_pres_status3_pres_rdreq_stt(void)
{
    return (*(volatile reg_aria_disp_pres_status3_t *)REG_ARIA_DISP_PRES_STATUS3).bitc.pres_rdreq_stt;
}

void reg_aria_disp_set_pres_status3_pres_flt_stt(mt_u8 data)
{
    reg_aria_disp_pres_status3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_PRES_STATUS3;
    d.bitc.pres_flt_stt = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_PRES_STATUS3, d.all);
}

mt_u8   reg_aria_disp_get_pres_status3_pres_flt_stt(void)
{
    return (*(volatile reg_aria_disp_pres_status3_t *)REG_ARIA_DISP_PRES_STATUS3).bitc.pres_flt_stt;
}


/*!
  register ARIA_DISP_gra_scale0_ctrl (read/write)
  */
void reg_aria_disp_set_gra_scale0_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_CTRL, data);
}

mt_u32  reg_aria_disp_get_gra_scale0_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_CTRL);
}

void reg_aria_disp_set_gra_scale0_ctrl_h_filter_enable(mt_u8 data)
{
    reg_aria_disp_gra_scale0_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_CTRL;
    d.bitc.h_filter_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_ctrl_h_filter_enable(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_ctrl_t *)REG_ARIA_DISP_GRA_SCALE0_CTRL).bitc.h_filter_enable;
}

void reg_aria_disp_set_gra_scale0_ctrl_v_filter_enable(mt_u8 data)
{
    reg_aria_disp_gra_scale0_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_CTRL;
    d.bitc.v_filter_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_ctrl_v_filter_enable(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_ctrl_t *)REG_ARIA_DISP_GRA_SCALE0_CTRL).bitc.v_filter_enable;
}

void reg_aria_disp_set_gra_scale0_ctrl_h_phase(mt_u8 data)
{
    reg_aria_disp_gra_scale0_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_CTRL;
    d.bitc.h_phase = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_ctrl_h_phase(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_ctrl_t *)REG_ARIA_DISP_GRA_SCALE0_CTRL).bitc.h_phase;
}

void reg_aria_disp_set_gra_scale0_ctrl_v_phase(mt_u8 data)
{
    reg_aria_disp_gra_scale0_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_CTRL;
    d.bitc.v_phase = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_ctrl_v_phase(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_ctrl_t *)REG_ARIA_DISP_GRA_SCALE0_CTRL).bitc.v_phase;
}

void reg_aria_disp_set_gra_scale0_ctrl_hscaler_alpha_bypass(mt_u8 data)
{
    reg_aria_disp_gra_scale0_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_CTRL;
    d.bitc.hscaler_alpha_bypass = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_ctrl_hscaler_alpha_bypass(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_ctrl_t *)REG_ARIA_DISP_GRA_SCALE0_CTRL).bitc.hscaler_alpha_bypass;
}

void reg_aria_disp_set_gra_scale0_ctrl_hscaler_tapnum(mt_u8 data)
{
    reg_aria_disp_gra_scale0_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_CTRL;
    d.bitc.hscaler_tapnum = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_ctrl_hscaler_tapnum(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_ctrl_t *)REG_ARIA_DISP_GRA_SCALE0_CTRL).bitc.hscaler_tapnum;
}

void reg_aria_disp_set_gra_scale0_ctrl_odd_startline(mt_u8 data)
{
    reg_aria_disp_gra_scale0_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_CTRL;
    d.bitc.odd_startline = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_ctrl_odd_startline(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_ctrl_t *)REG_ARIA_DISP_GRA_SCALE0_CTRL).bitc.odd_startline;
}

void reg_aria_disp_set_gra_scale0_ctrl_even_startline(mt_u8 data)
{
    reg_aria_disp_gra_scale0_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_CTRL;
    d.bitc.even_startline = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_ctrl_even_startline(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_ctrl_t *)REG_ARIA_DISP_GRA_SCALE0_CTRL).bitc.even_startline;
}

void reg_aria_disp_set_gra_scale0_ctrl_v_scale_border_disable(mt_u8 data)
{
    reg_aria_disp_gra_scale0_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_CTRL;
    d.bitc.v_scale_border_disable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_ctrl_v_scale_border_disable(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_ctrl_t *)REG_ARIA_DISP_GRA_SCALE0_CTRL).bitc.v_scale_border_disable;
}

void reg_aria_disp_set_gra_scale0_ctrl_h_scale_border_disable(mt_u8 data)
{
    reg_aria_disp_gra_scale0_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_CTRL;
    d.bitc.h_scale_border_disable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_ctrl_h_scale_border_disable(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_ctrl_t *)REG_ARIA_DISP_GRA_SCALE0_CTRL).bitc.h_scale_border_disable;
}

void reg_aria_disp_set_gra_scale0_ctrl_h_scaler_startnum(mt_u8 data)
{
    reg_aria_disp_gra_scale0_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_CTRL;
    d.bitc.h_scaler_startnum = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_ctrl_h_scaler_startnum(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_ctrl_t *)REG_ARIA_DISP_GRA_SCALE0_CTRL).bitc.h_scaler_startnum;
}


/*!
  register ARIA_DISP_gra_scale0_h_ratio (read/write)
  */
void reg_aria_disp_set_gra_scale0_h_ratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_H_RATIO, data);
}

mt_u32  reg_aria_disp_get_gra_scale0_h_ratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_H_RATIO);
}

void reg_aria_disp_set_gra_scale0_h_ratio_h_ratio_int(mt_u8 data)
{
    reg_aria_disp_gra_scale0_h_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_H_RATIO;
    d.bitc.h_ratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_H_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_h_ratio_h_ratio_int(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_h_ratio_t *)REG_ARIA_DISP_GRA_SCALE0_H_RATIO).bitc.h_ratio_int;
}

void reg_aria_disp_set_gra_scale0_h_ratio_h_ratio_fra(mt_u16 data)
{
    reg_aria_disp_gra_scale0_h_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_H_RATIO;
    d.bitc.h_ratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_H_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale0_h_ratio_h_ratio_fra(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_h_ratio_t *)REG_ARIA_DISP_GRA_SCALE0_H_RATIO).bitc.h_ratio_fra;
}


/*!
  register ARIA_DISP_gra_scale0_v_ratio (read/write)
  */
void reg_aria_disp_set_gra_scale0_v_ratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_V_RATIO, data);
}

mt_u32  reg_aria_disp_get_gra_scale0_v_ratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_V_RATIO);
}

void reg_aria_disp_set_gra_scale0_v_ratio_v_ratio_int(mt_u8 data)
{
    reg_aria_disp_gra_scale0_v_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_V_RATIO;
    d.bitc.v_ratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_V_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_v_ratio_v_ratio_int(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_v_ratio_t *)REG_ARIA_DISP_GRA_SCALE0_V_RATIO).bitc.v_ratio_int;
}

void reg_aria_disp_set_gra_scale0_v_ratio_v_ratio_fra(mt_u16 data)
{
    reg_aria_disp_gra_scale0_v_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_V_RATIO;
    d.bitc.v_ratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_V_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale0_v_ratio_v_ratio_fra(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_v_ratio_t *)REG_ARIA_DISP_GRA_SCALE0_V_RATIO).bitc.v_ratio_fra;
}


/*!
  register ARIA_DISP_gra_scale0_h_start_fra (read/write)
  */
void reg_aria_disp_set_gra_scale0_h_start_fra(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_H_START_FRA, data);
}

mt_u32  reg_aria_disp_get_gra_scale0_h_start_fra(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_H_START_FRA);
}

void reg_aria_disp_set_gra_scale0_h_start_fra_gra_scale0_h_start_fra(mt_u16 data)
{
    reg_aria_disp_gra_scale0_h_start_fra_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_H_START_FRA;
    d.bitc.gra_scale0_h_start_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_H_START_FRA, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale0_h_start_fra_gra_scale0_h_start_fra(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_h_start_fra_t *)REG_ARIA_DISP_GRA_SCALE0_H_START_FRA).bitc.gra_scale0_h_start_fra;
}


/*!
  register ARIA_DISP_gra_scale0_v_start_fra (read/write)
  */
void reg_aria_disp_set_gra_scale0_v_start_fra(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_V_START_FRA, data);
}

mt_u32  reg_aria_disp_get_gra_scale0_v_start_fra(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_V_START_FRA);
}

void reg_aria_disp_set_gra_scale0_v_start_fra_gra_scale0_v_start_fra_odd(mt_u16 data)
{
    reg_aria_disp_gra_scale0_v_start_fra_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_V_START_FRA;
    d.bitc.gra_scale0_v_start_fra_odd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_V_START_FRA, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale0_v_start_fra_gra_scale0_v_start_fra_odd(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_v_start_fra_t *)REG_ARIA_DISP_GRA_SCALE0_V_START_FRA).bitc.gra_scale0_v_start_fra_odd;
}

void reg_aria_disp_set_gra_scale0_v_start_fra_gra_scale0_v_start_fra_even(mt_u16 data)
{
    reg_aria_disp_gra_scale0_v_start_fra_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_V_START_FRA;
    d.bitc.gra_scale0_v_start_fra_even = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_V_START_FRA, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale0_v_start_fra_gra_scale0_v_start_fra_even(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_v_start_fra_t *)REG_ARIA_DISP_GRA_SCALE0_V_START_FRA).bitc.gra_scale0_v_start_fra_even;
}


/*!
  register ARIA_DISP_gra_scale0_postprocess (read/write)
  */
void reg_aria_disp_set_gra_scale0_postprocess(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_POSTPROCESS, data);
}

mt_u32  reg_aria_disp_get_gra_scale0_postprocess(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_POSTPROCESS);
}

void reg_aria_disp_set_gra_scale0_postprocess_shoot_chg(mt_u8 data)
{
    reg_aria_disp_gra_scale0_postprocess_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_POSTPROCESS;
    d.bitc.shoot_chg = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_POSTPROCESS, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_postprocess_shoot_chg(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_postprocess_t *)REG_ARIA_DISP_GRA_SCALE0_POSTPROCESS).bitc.shoot_chg;
}

void reg_aria_disp_set_gra_scale0_postprocess_hp_enha(mt_u8 data)
{
    reg_aria_disp_gra_scale0_postprocess_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_POSTPROCESS;
    d.bitc.hp_enha = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_POSTPROCESS, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale0_postprocess_hp_enha(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_postprocess_t *)REG_ARIA_DISP_GRA_SCALE0_POSTPROCESS).bitc.hp_enha;
}


/*!
  register ARIA_DISP_gra_scale0_hscaler_alpha_coeff_address (read/write)
  */
void reg_aria_disp_set_gra_scale0_hscaler_alpha_coeff_address(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_ALPHA_COEFF_ADDRESS, data);
}

mt_u32  reg_aria_disp_get_gra_scale0_hscaler_alpha_coeff_address(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_ALPHA_COEFF_ADDRESS);
}

void reg_aria_disp_set_gra_scale0_hscaler_alpha_coeff_address_gra_scale0_hscaler_alpha_coeff_address(mt_u32 data)
{
    reg_aria_disp_gra_scale0_hscaler_alpha_coeff_address_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_ALPHA_COEFF_ADDRESS;
    d.bitc.gra_scale0_hscaler_alpha_coeff_address = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_ALPHA_COEFF_ADDRESS, d.all);
}

mt_u32  reg_aria_disp_get_gra_scale0_hscaler_alpha_coeff_address_gra_scale0_hscaler_alpha_coeff_address(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_hscaler_alpha_coeff_address_t *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_ALPHA_COEFF_ADDRESS).bitc.gra_scale0_hscaler_alpha_coeff_address;
}


/*!
  register ARIA_DISP_gra_scale0_hscaler_luma_coeff_address (read/write)
  */
void reg_aria_disp_set_gra_scale0_hscaler_luma_coeff_address(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_LUMA_COEFF_ADDRESS, data);
}

mt_u32  reg_aria_disp_get_gra_scale0_hscaler_luma_coeff_address(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_LUMA_COEFF_ADDRESS);
}

void reg_aria_disp_set_gra_scale0_hscaler_luma_coeff_address_gra_scale0_hscaler_luma_coeff_address(mt_u32 data)
{
    reg_aria_disp_gra_scale0_hscaler_luma_coeff_address_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_LUMA_COEFF_ADDRESS;
    d.bitc.gra_scale0_hscaler_luma_coeff_address = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_LUMA_COEFF_ADDRESS, d.all);
}

mt_u32  reg_aria_disp_get_gra_scale0_hscaler_luma_coeff_address_gra_scale0_hscaler_luma_coeff_address(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_hscaler_luma_coeff_address_t *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_LUMA_COEFF_ADDRESS).bitc.gra_scale0_hscaler_luma_coeff_address;
}


/*!
  register ARIA_DISP_gra_scale0_hscaler_cbcr_coeff_address (read/write)
  */
void reg_aria_disp_set_gra_scale0_hscaler_cbcr_coeff_address(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_CBCR_COEFF_ADDRESS, data);
}

mt_u32  reg_aria_disp_get_gra_scale0_hscaler_cbcr_coeff_address(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_CBCR_COEFF_ADDRESS);
}

void reg_aria_disp_set_gra_scale0_hscaler_cbcr_coeff_address_gra_scale0_hscaler_cbcr_coeff_address(mt_u32 data)
{
    reg_aria_disp_gra_scale0_hscaler_cbcr_coeff_address_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_CBCR_COEFF_ADDRESS;
    d.bitc.gra_scale0_hscaler_cbcr_coeff_address = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_CBCR_COEFF_ADDRESS, d.all);
}

mt_u32  reg_aria_disp_get_gra_scale0_hscaler_cbcr_coeff_address_gra_scale0_hscaler_cbcr_coeff_address(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_hscaler_cbcr_coeff_address_t *)REG_ARIA_DISP_GRA_SCALE0_HSCALER_CBCR_COEFF_ADDRESS).bitc.gra_scale0_hscaler_cbcr_coeff_address;
}


/*!
  register ARIA_DISP_gra_scale0_vscaler_luma_coeff_address (read/write)
  */
void reg_aria_disp_set_gra_scale0_vscaler_luma_coeff_address(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_VSCALER_LUMA_COEFF_ADDRESS, data);
}

mt_u32  reg_aria_disp_get_gra_scale0_vscaler_luma_coeff_address(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_VSCALER_LUMA_COEFF_ADDRESS);
}

void reg_aria_disp_set_gra_scale0_vscaler_luma_coeff_address_gra_scale0_vscaler_luma_coeff_address(mt_u32 data)
{
    reg_aria_disp_gra_scale0_vscaler_luma_coeff_address_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_VSCALER_LUMA_COEFF_ADDRESS;
    d.bitc.gra_scale0_vscaler_luma_coeff_address = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_VSCALER_LUMA_COEFF_ADDRESS, d.all);
}

mt_u32  reg_aria_disp_get_gra_scale0_vscaler_luma_coeff_address_gra_scale0_vscaler_luma_coeff_address(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_vscaler_luma_coeff_address_t *)REG_ARIA_DISP_GRA_SCALE0_VSCALER_LUMA_COEFF_ADDRESS).bitc.gra_scale0_vscaler_luma_coeff_address;
}


/*!
  register ARIA_DISP_gra_scale0_output_size (read/write)
  */
void reg_aria_disp_set_gra_scale0_output_size(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_OUTPUT_SIZE, data);
}

mt_u32  reg_aria_disp_get_gra_scale0_output_size(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_OUTPUT_SIZE);
}

void reg_aria_disp_set_gra_scale0_output_size_gra_scale0_height_out(mt_u16 data)
{
    reg_aria_disp_gra_scale0_output_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_OUTPUT_SIZE;
    d.bitc.gra_scale0_height_out = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_OUTPUT_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale0_output_size_gra_scale0_height_out(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_output_size_t *)REG_ARIA_DISP_GRA_SCALE0_OUTPUT_SIZE).bitc.gra_scale0_height_out;
}

void reg_aria_disp_set_gra_scale0_output_size_gra_scale0_width_out(mt_u16 data)
{
    reg_aria_disp_gra_scale0_output_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE0_OUTPUT_SIZE;
    d.bitc.gra_scale0_width_out = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE0_OUTPUT_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale0_output_size_gra_scale0_width_out(void)
{
    return (*(volatile reg_aria_disp_gra_scale0_output_size_t *)REG_ARIA_DISP_GRA_SCALE0_OUTPUT_SIZE).bitc.gra_scale0_width_out;
}


/*!
  register ARIA_DISP_gra_scale1_ctrl (read/write)
  */
void reg_aria_disp_set_gra_scale1_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_CTRL, data);
}

mt_u32  reg_aria_disp_get_gra_scale1_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_CTRL);
}

void reg_aria_disp_set_gra_scale1_ctrl_h_filter_enable(mt_u8 data)
{
    reg_aria_disp_gra_scale1_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_CTRL;
    d.bitc.h_filter_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_ctrl_h_filter_enable(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_ctrl_t *)REG_ARIA_DISP_GRA_SCALE1_CTRL).bitc.h_filter_enable;
}

void reg_aria_disp_set_gra_scale1_ctrl_v_filter_enable(mt_u8 data)
{
    reg_aria_disp_gra_scale1_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_CTRL;
    d.bitc.v_filter_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_ctrl_v_filter_enable(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_ctrl_t *)REG_ARIA_DISP_GRA_SCALE1_CTRL).bitc.v_filter_enable;
}

void reg_aria_disp_set_gra_scale1_ctrl_h_phase(mt_u8 data)
{
    reg_aria_disp_gra_scale1_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_CTRL;
    d.bitc.h_phase = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_ctrl_h_phase(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_ctrl_t *)REG_ARIA_DISP_GRA_SCALE1_CTRL).bitc.h_phase;
}

void reg_aria_disp_set_gra_scale1_ctrl_v_phase(mt_u8 data)
{
    reg_aria_disp_gra_scale1_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_CTRL;
    d.bitc.v_phase = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_ctrl_v_phase(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_ctrl_t *)REG_ARIA_DISP_GRA_SCALE1_CTRL).bitc.v_phase;
}

void reg_aria_disp_set_gra_scale1_ctrl_hscaler_alpha_bypass(mt_u8 data)
{
    reg_aria_disp_gra_scale1_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_CTRL;
    d.bitc.hscaler_alpha_bypass = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_ctrl_hscaler_alpha_bypass(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_ctrl_t *)REG_ARIA_DISP_GRA_SCALE1_CTRL).bitc.hscaler_alpha_bypass;
}

void reg_aria_disp_set_gra_scale1_ctrl_hscaler_tapnum(mt_u8 data)
{
    reg_aria_disp_gra_scale1_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_CTRL;
    d.bitc.hscaler_tapnum = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_ctrl_hscaler_tapnum(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_ctrl_t *)REG_ARIA_DISP_GRA_SCALE1_CTRL).bitc.hscaler_tapnum;
}

void reg_aria_disp_set_gra_scale1_ctrl_odd_startline(mt_u8 data)
{
    reg_aria_disp_gra_scale1_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_CTRL;
    d.bitc.odd_startline = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_ctrl_odd_startline(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_ctrl_t *)REG_ARIA_DISP_GRA_SCALE1_CTRL).bitc.odd_startline;
}

void reg_aria_disp_set_gra_scale1_ctrl_even_startline(mt_u8 data)
{
    reg_aria_disp_gra_scale1_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_CTRL;
    d.bitc.even_startline = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_ctrl_even_startline(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_ctrl_t *)REG_ARIA_DISP_GRA_SCALE1_CTRL).bitc.even_startline;
}

void reg_aria_disp_set_gra_scale1_ctrl_v_scale_border_disable(mt_u8 data)
{
    reg_aria_disp_gra_scale1_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_CTRL;
    d.bitc.v_scale_border_disable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_ctrl_v_scale_border_disable(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_ctrl_t *)REG_ARIA_DISP_GRA_SCALE1_CTRL).bitc.v_scale_border_disable;
}

void reg_aria_disp_set_gra_scale1_ctrl_h_scale_border_disable(mt_u8 data)
{
    reg_aria_disp_gra_scale1_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_CTRL;
    d.bitc.h_scale_border_disable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_ctrl_h_scale_border_disable(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_ctrl_t *)REG_ARIA_DISP_GRA_SCALE1_CTRL).bitc.h_scale_border_disable;
}

void reg_aria_disp_set_gra_scale1_ctrl_h_scaler_startnum(mt_u8 data)
{
    reg_aria_disp_gra_scale1_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_CTRL;
    d.bitc.h_scaler_startnum = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_ctrl_h_scaler_startnum(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_ctrl_t *)REG_ARIA_DISP_GRA_SCALE1_CTRL).bitc.h_scaler_startnum;
}

void reg_aria_disp_set_gra_scale1_ctrl_downsample_enable(mt_u8 data)
{
    reg_aria_disp_gra_scale1_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_CTRL;
    d.bitc.downsample_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_ctrl_downsample_enable(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_ctrl_t *)REG_ARIA_DISP_GRA_SCALE1_CTRL).bitc.downsample_enable;
}


/*!
  register ARIA_DISP_gra_scale1_h_ratio (read/write)
  */
void reg_aria_disp_set_gra_scale1_h_ratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_H_RATIO, data);
}

mt_u32  reg_aria_disp_get_gra_scale1_h_ratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_H_RATIO);
}

void reg_aria_disp_set_gra_scale1_h_ratio_h_ratio_int(mt_u8 data)
{
    reg_aria_disp_gra_scale1_h_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_H_RATIO;
    d.bitc.h_ratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_H_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_h_ratio_h_ratio_int(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_h_ratio_t *)REG_ARIA_DISP_GRA_SCALE1_H_RATIO).bitc.h_ratio_int;
}

void reg_aria_disp_set_gra_scale1_h_ratio_h_ratio_fra(mt_u16 data)
{
    reg_aria_disp_gra_scale1_h_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_H_RATIO;
    d.bitc.h_ratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_H_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale1_h_ratio_h_ratio_fra(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_h_ratio_t *)REG_ARIA_DISP_GRA_SCALE1_H_RATIO).bitc.h_ratio_fra;
}


/*!
  register ARIA_DISP_gra_scale1_v_ratio (read/write)
  */
void reg_aria_disp_set_gra_scale1_v_ratio(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_V_RATIO, data);
}

mt_u32  reg_aria_disp_get_gra_scale1_v_ratio(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_V_RATIO);
}

void reg_aria_disp_set_gra_scale1_v_ratio_v_ratio_int(mt_u8 data)
{
    reg_aria_disp_gra_scale1_v_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_V_RATIO;
    d.bitc.v_ratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_V_RATIO, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_v_ratio_v_ratio_int(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_v_ratio_t *)REG_ARIA_DISP_GRA_SCALE1_V_RATIO).bitc.v_ratio_int;
}

void reg_aria_disp_set_gra_scale1_v_ratio_v_ratio_fra(mt_u16 data)
{
    reg_aria_disp_gra_scale1_v_ratio_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_V_RATIO;
    d.bitc.v_ratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_V_RATIO, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale1_v_ratio_v_ratio_fra(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_v_ratio_t *)REG_ARIA_DISP_GRA_SCALE1_V_RATIO).bitc.v_ratio_fra;
}


/*!
  register ARIA_DISP_gra_scale1_h_start_fra (read/write)
  */
void reg_aria_disp_set_gra_scale1_h_start_fra(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_H_START_FRA, data);
}

mt_u32  reg_aria_disp_get_gra_scale1_h_start_fra(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_H_START_FRA);
}

void reg_aria_disp_set_gra_scale1_h_start_fra_gra_scale1_h_start_fra(mt_u16 data)
{
    reg_aria_disp_gra_scale1_h_start_fra_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_H_START_FRA;
    d.bitc.gra_scale1_h_start_fra = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_H_START_FRA, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale1_h_start_fra_gra_scale1_h_start_fra(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_h_start_fra_t *)REG_ARIA_DISP_GRA_SCALE1_H_START_FRA).bitc.gra_scale1_h_start_fra;
}


/*!
  register ARIA_DISP_gra_scale1_v_start_fra (read/write)
  */
void reg_aria_disp_set_gra_scale1_v_start_fra(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_V_START_FRA, data);
}

mt_u32  reg_aria_disp_get_gra_scale1_v_start_fra(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_V_START_FRA);
}

void reg_aria_disp_set_gra_scale1_v_start_fra_gra_scale1_v_start_fra_odd(mt_u16 data)
{
    reg_aria_disp_gra_scale1_v_start_fra_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_V_START_FRA;
    d.bitc.gra_scale1_v_start_fra_odd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_V_START_FRA, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale1_v_start_fra_gra_scale1_v_start_fra_odd(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_v_start_fra_t *)REG_ARIA_DISP_GRA_SCALE1_V_START_FRA).bitc.gra_scale1_v_start_fra_odd;
}

void reg_aria_disp_set_gra_scale1_v_start_fra_gra_scale1_v_start_fra_even(mt_u16 data)
{
    reg_aria_disp_gra_scale1_v_start_fra_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_V_START_FRA;
    d.bitc.gra_scale1_v_start_fra_even = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_V_START_FRA, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale1_v_start_fra_gra_scale1_v_start_fra_even(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_v_start_fra_t *)REG_ARIA_DISP_GRA_SCALE1_V_START_FRA).bitc.gra_scale1_v_start_fra_even;
}


/*!
  register ARIA_DISP_gra_scale1_postprocess (read/write)
  */
void reg_aria_disp_set_gra_scale1_postprocess(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_POSTPROCESS, data);
}

mt_u32  reg_aria_disp_get_gra_scale1_postprocess(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_POSTPROCESS);
}

void reg_aria_disp_set_gra_scale1_postprocess_shoot_chg(mt_u8 data)
{
    reg_aria_disp_gra_scale1_postprocess_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_POSTPROCESS;
    d.bitc.shoot_chg = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_POSTPROCESS, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_postprocess_shoot_chg(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_postprocess_t *)REG_ARIA_DISP_GRA_SCALE1_POSTPROCESS).bitc.shoot_chg;
}

void reg_aria_disp_set_gra_scale1_postprocess_hp_enha(mt_u8 data)
{
    reg_aria_disp_gra_scale1_postprocess_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_POSTPROCESS;
    d.bitc.hp_enha = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_POSTPROCESS, d.all);
}

mt_u8   reg_aria_disp_get_gra_scale1_postprocess_hp_enha(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_postprocess_t *)REG_ARIA_DISP_GRA_SCALE1_POSTPROCESS).bitc.hp_enha;
}


/*!
  register ARIA_DISP_gra_scale1_hscaler_alpha_coeff_address (read/write)
  */
void reg_aria_disp_set_gra_scale1_hscaler_alpha_coeff_address(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_ALPHA_COEFF_ADDRESS, data);
}

mt_u32  reg_aria_disp_get_gra_scale1_hscaler_alpha_coeff_address(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_ALPHA_COEFF_ADDRESS);
}

void reg_aria_disp_set_gra_scale1_hscaler_alpha_coeff_address_gra_scale1_hscaler_alpha_coeff_address(mt_u32 data)
{
    reg_aria_disp_gra_scale1_hscaler_alpha_coeff_address_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_ALPHA_COEFF_ADDRESS;
    d.bitc.gra_scale1_hscaler_alpha_coeff_address = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_ALPHA_COEFF_ADDRESS, d.all);
}

mt_u32  reg_aria_disp_get_gra_scale1_hscaler_alpha_coeff_address_gra_scale1_hscaler_alpha_coeff_address(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_hscaler_alpha_coeff_address_t *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_ALPHA_COEFF_ADDRESS).bitc.gra_scale1_hscaler_alpha_coeff_address;
}


/*!
  register ARIA_DISP_gra_scale1_hscaler_luma_coeff_address (read/write)
  */
void reg_aria_disp_set_gra_scale1_hscaler_luma_coeff_address(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_LUMA_COEFF_ADDRESS, data);
}

mt_u32  reg_aria_disp_get_gra_scale1_hscaler_luma_coeff_address(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_LUMA_COEFF_ADDRESS);
}

void reg_aria_disp_set_gra_scale1_hscaler_luma_coeff_address_gra_scale1_hscaler_luma_coeff_address(mt_u32 data)
{
    reg_aria_disp_gra_scale1_hscaler_luma_coeff_address_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_LUMA_COEFF_ADDRESS;
    d.bitc.gra_scale1_hscaler_luma_coeff_address = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_LUMA_COEFF_ADDRESS, d.all);
}

mt_u32  reg_aria_disp_get_gra_scale1_hscaler_luma_coeff_address_gra_scale1_hscaler_luma_coeff_address(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_hscaler_luma_coeff_address_t *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_LUMA_COEFF_ADDRESS).bitc.gra_scale1_hscaler_luma_coeff_address;
}


/*!
  register ARIA_DISP_gra_scale1_hscaler_cbcr_coeff_address (read/write)
  */
void reg_aria_disp_set_gra_scale1_hscaler_cbcr_coeff_address(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_CBCR_COEFF_ADDRESS, data);
}

mt_u32  reg_aria_disp_get_gra_scale1_hscaler_cbcr_coeff_address(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_CBCR_COEFF_ADDRESS);
}

void reg_aria_disp_set_gra_scale1_hscaler_cbcr_coeff_address_gra_scale1_hscaler_cbcr_coeff_address(mt_u32 data)
{
    reg_aria_disp_gra_scale1_hscaler_cbcr_coeff_address_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_CBCR_COEFF_ADDRESS;
    d.bitc.gra_scale1_hscaler_cbcr_coeff_address = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_CBCR_COEFF_ADDRESS, d.all);
}

mt_u32  reg_aria_disp_get_gra_scale1_hscaler_cbcr_coeff_address_gra_scale1_hscaler_cbcr_coeff_address(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_hscaler_cbcr_coeff_address_t *)REG_ARIA_DISP_GRA_SCALE1_HSCALER_CBCR_COEFF_ADDRESS).bitc.gra_scale1_hscaler_cbcr_coeff_address;
}


/*!
  register ARIA_DISP_gra_scale1_vscaler_luma_coeff_address (read/write)
  */
void reg_aria_disp_set_gra_scale1_vscaler_luma_coeff_address(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_VSCALER_LUMA_COEFF_ADDRESS, data);
}

mt_u32  reg_aria_disp_get_gra_scale1_vscaler_luma_coeff_address(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_VSCALER_LUMA_COEFF_ADDRESS);
}

void reg_aria_disp_set_gra_scale1_vscaler_luma_coeff_address_gra_scale1_vscaler_luma_coeff_address(mt_u32 data)
{
    reg_aria_disp_gra_scale1_vscaler_luma_coeff_address_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE1_VSCALER_LUMA_COEFF_ADDRESS;
    d.bitc.gra_scale1_vscaler_luma_coeff_address = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE1_VSCALER_LUMA_COEFF_ADDRESS, d.all);
}

mt_u32  reg_aria_disp_get_gra_scale1_vscaler_luma_coeff_address_gra_scale1_vscaler_luma_coeff_address(void)
{
    return (*(volatile reg_aria_disp_gra_scale1_vscaler_luma_coeff_address_t *)REG_ARIA_DISP_GRA_SCALE1_VSCALER_LUMA_COEFF_ADDRESS).bitc.gra_scale1_vscaler_luma_coeff_address;
}


/*!
  register ARIA_DISP_gra_scale_fifo_threshold (read/write)
  */
void reg_aria_disp_set_gra_scale_fifo_threshold(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE_FIFO_THRESHOLD, data);
}

mt_u32  reg_aria_disp_get_gra_scale_fifo_threshold(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE_FIFO_THRESHOLD);
}

void reg_aria_disp_set_gra_scale_fifo_threshold_gra_scale0_fifo_threshold(mt_u16 data)
{
    reg_aria_disp_gra_scale_fifo_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE_FIFO_THRESHOLD;
    d.bitc.gra_scale0_fifo_threshold = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE_FIFO_THRESHOLD, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale_fifo_threshold_gra_scale0_fifo_threshold(void)
{
    return (*(volatile reg_aria_disp_gra_scale_fifo_threshold_t *)REG_ARIA_DISP_GRA_SCALE_FIFO_THRESHOLD).bitc.gra_scale0_fifo_threshold;
}

void reg_aria_disp_set_gra_scale_fifo_threshold_gra_scale1_fifo_threshold(mt_u16 data)
{
    reg_aria_disp_gra_scale_fifo_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALE_FIFO_THRESHOLD;
    d.bitc.gra_scale1_fifo_threshold = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALE_FIFO_THRESHOLD, d.all);
}

mt_u16  reg_aria_disp_get_gra_scale_fifo_threshold_gra_scale1_fifo_threshold(void)
{
    return (*(volatile reg_aria_disp_gra_scale_fifo_threshold_t *)REG_ARIA_DISP_GRA_SCALE_FIFO_THRESHOLD).bitc.gra_scale1_fifo_threshold;
}


/*!
  register ARIA_DISP_gra_scaler_status (read/write)
  */
void reg_aria_disp_set_gra_scaler_status(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALER_STATUS, data);
}

mt_u32  reg_aria_disp_get_gra_scaler_status(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALER_STATUS);
}

void reg_aria_disp_set_gra_scaler_status_gra_scaler1_fifo_full(mt_u8 data)
{
    reg_aria_disp_gra_scaler_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALER_STATUS;
    d.bitc.gra_scaler1_fifo_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALER_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_gra_scaler_status_gra_scaler1_fifo_full(void)
{
    return (*(volatile reg_aria_disp_gra_scaler_status_t *)REG_ARIA_DISP_GRA_SCALER_STATUS).bitc.gra_scaler1_fifo_full;
}

void reg_aria_disp_set_gra_scaler_status_gra_scaler1_fifo_empty(mt_u8 data)
{
    reg_aria_disp_gra_scaler_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALER_STATUS;
    d.bitc.gra_scaler1_fifo_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALER_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_gra_scaler_status_gra_scaler1_fifo_empty(void)
{
    return (*(volatile reg_aria_disp_gra_scaler_status_t *)REG_ARIA_DISP_GRA_SCALER_STATUS).bitc.gra_scaler1_fifo_empty;
}

void reg_aria_disp_set_gra_scaler_status_gra_scaler0_fifo_full(mt_u8 data)
{
    reg_aria_disp_gra_scaler_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALER_STATUS;
    d.bitc.gra_scaler0_fifo_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALER_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_gra_scaler_status_gra_scaler0_fifo_full(void)
{
    return (*(volatile reg_aria_disp_gra_scaler_status_t *)REG_ARIA_DISP_GRA_SCALER_STATUS).bitc.gra_scaler0_fifo_full;
}

void reg_aria_disp_set_gra_scaler_status_gra_scaler0_fifo_empty(mt_u8 data)
{
    reg_aria_disp_gra_scaler_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SCALER_STATUS;
    d.bitc.gra_scaler0_fifo_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SCALER_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_gra_scaler_status_gra_scaler0_fifo_empty(void)
{
    return (*(volatile reg_aria_disp_gra_scaler_status_t *)REG_ARIA_DISP_GRA_SCALER_STATUS).bitc.gra_scaler0_fifo_empty;
}


/*!
  register ARIA_DISP_gra_saler_latch_cmd (read/write)
  */
void reg_aria_disp_set_gra_saler_latch_cmd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD, data);
}

mt_u32  reg_aria_disp_get_gra_saler_latch_cmd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD);
}

void reg_aria_disp_set_gra_saler_latch_cmd_gra_latch_top(mt_u8 data)
{
    reg_aria_disp_gra_saler_latch_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD;
    d.bitc.gra_latch_top = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD, d.all);
}

mt_u8   reg_aria_disp_get_gra_saler_latch_cmd_gra_latch_top(void)
{
    return (*(volatile reg_aria_disp_gra_saler_latch_cmd_t *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD).bitc.gra_latch_top;
}

void reg_aria_disp_set_gra_saler_latch_cmd_gra_latch_bot(mt_u8 data)
{
    reg_aria_disp_gra_saler_latch_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD;
    d.bitc.gra_latch_bot = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD, d.all);
}

mt_u8   reg_aria_disp_get_gra_saler_latch_cmd_gra_latch_bot(void)
{
    return (*(volatile reg_aria_disp_gra_saler_latch_cmd_t *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD).bitc.gra_latch_bot;
}

void reg_aria_disp_set_gra_saler_latch_cmd_gra_latch_3d_1st(mt_u8 data)
{
    reg_aria_disp_gra_saler_latch_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD;
    d.bitc.gra_latch_3d_1st = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD, d.all);
}

mt_u8   reg_aria_disp_get_gra_saler_latch_cmd_gra_latch_3d_1st(void)
{
    return (*(volatile reg_aria_disp_gra_saler_latch_cmd_t *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD).bitc.gra_latch_3d_1st;
}

void reg_aria_disp_set_gra_saler_latch_cmd_gra_latch_3d_2nd(mt_u8 data)
{
    reg_aria_disp_gra_saler_latch_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD;
    d.bitc.gra_latch_3d_2nd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD, d.all);
}

mt_u8   reg_aria_disp_get_gra_saler_latch_cmd_gra_latch_3d_2nd(void)
{
    return (*(volatile reg_aria_disp_gra_saler_latch_cmd_t *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD).bitc.gra_latch_3d_2nd;
}

void reg_aria_disp_set_gra_saler_latch_cmd_gra_latch_or_not(mt_u8 data)
{
    reg_aria_disp_gra_saler_latch_cmd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD;
    d.bitc.gra_latch_or_not = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD, d.all);
}

mt_u8   reg_aria_disp_get_gra_saler_latch_cmd_gra_latch_or_not(void)
{
    return (*(volatile reg_aria_disp_gra_saler_latch_cmd_t *)REG_ARIA_DISP_GRA_SALER_LATCH_CMD).bitc.gra_latch_or_not;
}


/*!
  register ARIA_DISP_sd_wr_ctrl (read/write)
  */
void reg_aria_disp_set_sd_wr_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WR_CTRL, data);
}

mt_u32  reg_aria_disp_get_sd_wr_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_WR_CTRL);
}

void reg_aria_disp_set_sd_wr_ctrl_cfg_sd_startlines(mt_u16 data)
{
    reg_aria_disp_sd_wr_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_WR_CTRL;
    d.bitc.cfg_sd_startlines = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WR_CTRL, d.all);
}

mt_u16  reg_aria_disp_get_sd_wr_ctrl_cfg_sd_startlines(void)
{
    return (*(volatile reg_aria_disp_sd_wr_ctrl_t *)REG_ARIA_DISP_SD_WR_CTRL).bitc.cfg_sd_startlines;
}

void reg_aria_disp_set_sd_wr_ctrl_sd_pal_format(mt_u8 data)
{
    reg_aria_disp_sd_wr_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_WR_CTRL;
    d.bitc.sd_pal_format = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WR_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_wr_ctrl_sd_pal_format(void)
{
    return (*(volatile reg_aria_disp_sd_wr_ctrl_t *)REG_ARIA_DISP_SD_WR_CTRL).bitc.sd_pal_format;
}

void reg_aria_disp_set_sd_wr_ctrl_cfg_sd_onefield_mode(mt_u8 data)
{
    reg_aria_disp_sd_wr_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_WR_CTRL;
    d.bitc.cfg_sd_onefield_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WR_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_wr_ctrl_cfg_sd_onefield_mode(void)
{
    return (*(volatile reg_aria_disp_sd_wr_ctrl_t *)REG_ARIA_DISP_SD_WR_CTRL).bitc.cfg_sd_onefield_mode;
}

void reg_aria_disp_set_sd_wr_ctrl_cfg_rate_conversion_enable(mt_u8 data)
{
    reg_aria_disp_sd_wr_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_WR_CTRL;
    d.bitc.cfg_rate_conversion_enable = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WR_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_wr_ctrl_cfg_rate_conversion_enable(void)
{
    return (*(volatile reg_aria_disp_sd_wr_ctrl_t *)REG_ARIA_DISP_SD_WR_CTRL).bitc.cfg_rate_conversion_enable;
}

void reg_aria_disp_set_sd_wr_ctrl_sd_buffer_number(mt_u8 data)
{
    reg_aria_disp_sd_wr_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_WR_CTRL;
    d.bitc.sd_buffer_number = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WR_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_wr_ctrl_sd_buffer_number(void)
{
    return (*(volatile reg_aria_disp_sd_wr_ctrl_t *)REG_ARIA_DISP_SD_WR_CTRL).bitc.sd_buffer_number;
}

void reg_aria_disp_set_sd_wr_ctrl_sd_wr_back_forbidden(mt_u8 data)
{
    reg_aria_disp_sd_wr_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_WR_CTRL;
    d.bitc.sd_wr_back_forbidden = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WR_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_wr_ctrl_sd_wr_back_forbidden(void)
{
    return (*(volatile reg_aria_disp_sd_wr_ctrl_t *)REG_ARIA_DISP_SD_WR_CTRL).bitc.sd_wr_back_forbidden;
}

void reg_aria_disp_set_sd_wr_ctrl_sd_wrback_yuv444(mt_u8 data)
{
    reg_aria_disp_sd_wr_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_WR_CTRL;
    d.bitc.sd_wrback_yuv444 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WR_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_wr_ctrl_sd_wrback_yuv444(void)
{
    return (*(volatile reg_aria_disp_sd_wr_ctrl_t *)REG_ARIA_DISP_SD_WR_CTRL).bitc.sd_wrback_yuv444;
}

void reg_aria_disp_set_sd_wr_ctrl_sdbuf_softctrl_en(mt_u8 data)
{
    reg_aria_disp_sd_wr_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_WR_CTRL;
    d.bitc.sdbuf_softctrl_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WR_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_wr_ctrl_sdbuf_softctrl_en(void)
{
    return (*(volatile reg_aria_disp_sd_wr_ctrl_t *)REG_ARIA_DISP_SD_WR_CTRL).bitc.sdbuf_softctrl_en;
}


/*!
  register ARIA_DISP_sd_latch_command (read/write)
  */
void reg_aria_disp_set_sd_latch_command(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_LATCH_COMMAND, data);
}

mt_u32  reg_aria_disp_get_sd_latch_command(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_LATCH_COMMAND);
}

void reg_aria_disp_set_sd_latch_command_sd_latch_top(mt_u8 data)
{
    reg_aria_disp_sd_latch_command_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_LATCH_COMMAND;
    d.bitc.sd_latch_top = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_LATCH_COMMAND, d.all);
}

mt_u8   reg_aria_disp_get_sd_latch_command_sd_latch_top(void)
{
    return (*(volatile reg_aria_disp_sd_latch_command_t *)REG_ARIA_DISP_SD_LATCH_COMMAND).bitc.sd_latch_top;
}

void reg_aria_disp_set_sd_latch_command_sd_latch_bot(mt_u8 data)
{
    reg_aria_disp_sd_latch_command_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_LATCH_COMMAND;
    d.bitc.sd_latch_bot = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_LATCH_COMMAND, d.all);
}

mt_u8   reg_aria_disp_get_sd_latch_command_sd_latch_bot(void)
{
    return (*(volatile reg_aria_disp_sd_latch_command_t *)REG_ARIA_DISP_SD_LATCH_COMMAND).bitc.sd_latch_bot;
}

void reg_aria_disp_set_sd_latch_command_sd_latch_3d_1st(mt_u8 data)
{
    reg_aria_disp_sd_latch_command_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_LATCH_COMMAND;
    d.bitc.sd_latch_3d_1st = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_LATCH_COMMAND, d.all);
}

mt_u8   reg_aria_disp_get_sd_latch_command_sd_latch_3d_1st(void)
{
    return (*(volatile reg_aria_disp_sd_latch_command_t *)REG_ARIA_DISP_SD_LATCH_COMMAND).bitc.sd_latch_3d_1st;
}

void reg_aria_disp_set_sd_latch_command_sd_latch_3d_2nd(mt_u8 data)
{
    reg_aria_disp_sd_latch_command_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_LATCH_COMMAND;
    d.bitc.sd_latch_3d_2nd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_LATCH_COMMAND, d.all);
}

mt_u8   reg_aria_disp_get_sd_latch_command_sd_latch_3d_2nd(void)
{
    return (*(volatile reg_aria_disp_sd_latch_command_t *)REG_ARIA_DISP_SD_LATCH_COMMAND).bitc.sd_latch_3d_2nd;
}

void reg_aria_disp_set_sd_latch_command_sd_latch_or_not(mt_u8 data)
{
    reg_aria_disp_sd_latch_command_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_LATCH_COMMAND;
    d.bitc.sd_latch_or_not = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_LATCH_COMMAND, d.all);
}

mt_u8   reg_aria_disp_get_sd_latch_command_sd_latch_or_not(void)
{
    return (*(volatile reg_aria_disp_sd_latch_command_t *)REG_ARIA_DISP_SD_LATCH_COMMAND).bitc.sd_latch_or_not;
}


/*!
  register ARIA_DISP_sd_wrback_addr_odd (read/write)
  */
void reg_aria_disp_set_sd_wrback_addr_odd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WRBACK_ADDR_ODD, data);
}

mt_u32  reg_aria_disp_get_sd_wrback_addr_odd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_WRBACK_ADDR_ODD);
}

void reg_aria_disp_set_sd_wrback_addr_odd_sd_wrback_addr_odd(mt_u32 data)
{
    reg_aria_disp_sd_wrback_addr_odd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_WRBACK_ADDR_ODD;
    d.bitc.sd_wrback_addr_odd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WRBACK_ADDR_ODD, d.all);
}

mt_u32  reg_aria_disp_get_sd_wrback_addr_odd_sd_wrback_addr_odd(void)
{
    return (*(volatile reg_aria_disp_sd_wrback_addr_odd_t *)REG_ARIA_DISP_SD_WRBACK_ADDR_ODD).bitc.sd_wrback_addr_odd;
}


/*!
  register ARIA_DISP_sd_wrback_addr_even (read/write)
  */
void reg_aria_disp_set_sd_wrback_addr_even(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WRBACK_ADDR_EVEN, data);
}

mt_u32  reg_aria_disp_get_sd_wrback_addr_even(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_WRBACK_ADDR_EVEN);
}

void reg_aria_disp_set_sd_wrback_addr_even_sd_wrback_addr_odd(mt_u32 data)
{
    reg_aria_disp_sd_wrback_addr_even_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_WRBACK_ADDR_EVEN;
    d.bitc.sd_wrback_addr_odd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WRBACK_ADDR_EVEN, d.all);
}

mt_u32  reg_aria_disp_get_sd_wrback_addr_even_sd_wrback_addr_odd(void)
{
    return (*(volatile reg_aria_disp_sd_wrback_addr_even_t *)REG_ARIA_DISP_SD_WRBACK_ADDR_EVEN).bitc.sd_wrback_addr_odd;
}


/*!
  register ARIA_DISP_sd_rdback_addr_odd (read/write)
  */
void reg_aria_disp_set_sd_rdback_addr_odd(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_RDBACK_ADDR_ODD, data);
}

mt_u32  reg_aria_disp_get_sd_rdback_addr_odd(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_RDBACK_ADDR_ODD);
}

void reg_aria_disp_set_sd_rdback_addr_odd_sd_rdback_addr_odd(mt_u32 data)
{
    reg_aria_disp_sd_rdback_addr_odd_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_RDBACK_ADDR_ODD;
    d.bitc.sd_rdback_addr_odd = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_RDBACK_ADDR_ODD, d.all);
}

mt_u32  reg_aria_disp_get_sd_rdback_addr_odd_sd_rdback_addr_odd(void)
{
    return (*(volatile reg_aria_disp_sd_rdback_addr_odd_t *)REG_ARIA_DISP_SD_RDBACK_ADDR_ODD).bitc.sd_rdback_addr_odd;
}


/*!
  register ARIA_DISP_sd_rdback_addr_even (read/write)
  */
void reg_aria_disp_set_sd_rdback_addr_even(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_RDBACK_ADDR_EVEN, data);
}

mt_u32  reg_aria_disp_get_sd_rdback_addr_even(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_RDBACK_ADDR_EVEN);
}

void reg_aria_disp_set_sd_rdback_addr_even_sd_rdback_addr_even(mt_u32 data)
{
    reg_aria_disp_sd_rdback_addr_even_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_RDBACK_ADDR_EVEN;
    d.bitc.sd_rdback_addr_even = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_RDBACK_ADDR_EVEN, d.all);
}

mt_u32  reg_aria_disp_get_sd_rdback_addr_even_sd_rdback_addr_even(void)
{
    return (*(volatile reg_aria_disp_sd_rdback_addr_even_t *)REG_ARIA_DISP_SD_RDBACK_ADDR_EVEN).bitc.sd_rdback_addr_even;
}


/*!
  register ARIA_DISP_sd_wrback_fifo_threshold (read/write)
  */
void reg_aria_disp_set_sd_wrback_fifo_threshold(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WRBACK_FIFO_THRESHOLD, data);
}

mt_u32  reg_aria_disp_get_sd_wrback_fifo_threshold(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_WRBACK_FIFO_THRESHOLD);
}

void reg_aria_disp_set_sd_wrback_fifo_threshold_sd_wrback_fifo_threshold(mt_u8 data)
{
    reg_aria_disp_sd_wrback_fifo_threshold_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_WRBACK_FIFO_THRESHOLD;
    d.bitc.sd_wrback_fifo_threshold = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_WRBACK_FIFO_THRESHOLD, d.all);
}

mt_u8   reg_aria_disp_get_sd_wrback_fifo_threshold_sd_wrback_fifo_threshold(void)
{
    return (*(volatile reg_aria_disp_sd_wrback_fifo_threshold_t *)REG_ARIA_DISP_SD_WRBACK_FIFO_THRESHOLD).bitc.sd_wrback_fifo_threshold;
}


/*!
  register ARIA_DISP_sd_blankscreen_mode (read/write)
  */
void reg_aria_disp_set_sd_blankscreen_mode(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE, data);
}

mt_u32  reg_aria_disp_get_sd_blankscreen_mode(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE);
}

void reg_aria_disp_set_sd_blankscreen_mode_sd_blankscreen_cr_color(mt_u8 data)
{
    reg_aria_disp_sd_blankscreen_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE;
    d.bitc.sd_blankscreen_cr_color = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE, d.all);
}

mt_u8   reg_aria_disp_get_sd_blankscreen_mode_sd_blankscreen_cr_color(void)
{
    return (*(volatile reg_aria_disp_sd_blankscreen_mode_t *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE).bitc.sd_blankscreen_cr_color;
}

void reg_aria_disp_set_sd_blankscreen_mode_sd_blankscreen_cb_color(mt_u8 data)
{
    reg_aria_disp_sd_blankscreen_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE;
    d.bitc.sd_blankscreen_cb_color = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE, d.all);
}

mt_u8   reg_aria_disp_get_sd_blankscreen_mode_sd_blankscreen_cb_color(void)
{
    return (*(volatile reg_aria_disp_sd_blankscreen_mode_t *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE).bitc.sd_blankscreen_cb_color;
}

void reg_aria_disp_set_sd_blankscreen_mode_sd_blankscreen_luma(mt_u8 data)
{
    reg_aria_disp_sd_blankscreen_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE;
    d.bitc.sd_blankscreen_luma = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE, d.all);
}

mt_u8   reg_aria_disp_get_sd_blankscreen_mode_sd_blankscreen_luma(void)
{
    return (*(volatile reg_aria_disp_sd_blankscreen_mode_t *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE).bitc.sd_blankscreen_luma;
}

void reg_aria_disp_set_sd_blankscreen_mode_cfg_sd_blankscreen_mode(mt_u8 data)
{
    reg_aria_disp_sd_blankscreen_mode_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE;
    d.bitc.cfg_sd_blankscreen_mode = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE, d.all);
}

mt_u8   reg_aria_disp_get_sd_blankscreen_mode_cfg_sd_blankscreen_mode(void)
{
    return (*(volatile reg_aria_disp_sd_blankscreen_mode_t *)REG_ARIA_DISP_SD_BLANKSCREEN_MODE).bitc.cfg_sd_blankscreen_mode;
}


/*!
  register ARIA_DISP_sd_status (read/write)
  */
void reg_aria_disp_set_sd_status(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_STATUS, data);
}

mt_u32  reg_aria_disp_get_sd_status(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_STATUS);
}

void reg_aria_disp_set_sd_status_sdrd_fifo_empty(mt_u8 data)
{
    reg_aria_disp_sd_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_STATUS;
    d.bitc.sdrd_fifo_empty = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_sd_status_sdrd_fifo_empty(void)
{
    return (*(volatile reg_aria_disp_sd_status_t *)REG_ARIA_DISP_SD_STATUS).bitc.sdrd_fifo_empty;
}

void reg_aria_disp_set_sd_status_sdwr_fifo_full(mt_u8 data)
{
    reg_aria_disp_sd_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_STATUS;
    d.bitc.sdwr_fifo_full = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_sd_status_sdwr_fifo_full(void)
{
    return (*(volatile reg_aria_disp_sd_status_t *)REG_ARIA_DISP_SD_STATUS).bitc.sdwr_fifo_full;
}

void reg_aria_disp_set_sd_status_sdbuf_empty_flag(mt_u8 data)
{
    reg_aria_disp_sd_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_STATUS;
    d.bitc.sdbuf_empty_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_sd_status_sdbuf_empty_flag(void)
{
    return (*(volatile reg_aria_disp_sd_status_t *)REG_ARIA_DISP_SD_STATUS).bitc.sdbuf_empty_flag;
}

void reg_aria_disp_set_sd_status_sdbuf_full_flag(mt_u8 data)
{
    reg_aria_disp_sd_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_STATUS;
    d.bitc.sdbuf_full_flag = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_sd_status_sdbuf_full_flag(void)
{
    return (*(volatile reg_aria_disp_sd_status_t *)REG_ARIA_DISP_SD_STATUS).bitc.sdbuf_full_flag;
}

void reg_aria_disp_set_sd_status_sdbuf_rdptr(mt_u8 data)
{
    reg_aria_disp_sd_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_STATUS;
    d.bitc.sdbuf_rdptr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_sd_status_sdbuf_rdptr(void)
{
    return (*(volatile reg_aria_disp_sd_status_t *)REG_ARIA_DISP_SD_STATUS).bitc.sdbuf_rdptr;
}

void reg_aria_disp_set_sd_status_sdbuf_wrptr(mt_u8 data)
{
    reg_aria_disp_sd_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_STATUS;
    d.bitc.sdbuf_wrptr = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_sd_status_sdbuf_wrptr(void)
{
    return (*(volatile reg_aria_disp_sd_status_t *)REG_ARIA_DISP_SD_STATUS).bitc.sdbuf_wrptr;
}

void reg_aria_disp_set_sd_status_disable_sdenv(mt_u8 data)
{
    reg_aria_disp_sd_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_STATUS;
    d.bitc.disable_sdenv = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_sd_status_disable_sdenv(void)
{
    return (*(volatile reg_aria_disp_sd_status_t *)REG_ARIA_DISP_SD_STATUS).bitc.disable_sdenv;
}

void reg_aria_disp_set_sd_status_sdwr_axi_bresp_error(mt_u8 data)
{
    reg_aria_disp_sd_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_STATUS;
    d.bitc.sdwr_axi_bresp_error = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_sd_status_sdwr_axi_bresp_error(void)
{
    return (*(volatile reg_aria_disp_sd_status_t *)REG_ARIA_DISP_SD_STATUS).bitc.sdwr_axi_bresp_error;
}

void reg_aria_disp_set_sd_status_sdrd_axi_rready_error(mt_u8 data)
{
    reg_aria_disp_sd_status_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_STATUS;
    d.bitc.sdrd_axi_rready_error = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_STATUS, d.all);
}

mt_u8   reg_aria_disp_get_sd_status_sdrd_axi_rready_error(void)
{
    return (*(volatile reg_aria_disp_sd_status_t *)REG_ARIA_DISP_SD_STATUS).bitc.sdrd_axi_rready_error;
}


/*!
  register ARIA_DISP_sd_axi_monitor_ctrl (read/write)
  */
void reg_aria_disp_set_sd_axi_monitor_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_AXI_MONITOR_CTRL, data);
}

mt_u32  reg_aria_disp_get_sd_axi_monitor_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_AXI_MONITOR_CTRL);
}

void reg_aria_disp_set_sd_axi_monitor_ctrl_sd_rd_axi_monitor_restart(mt_u8 data)
{
    reg_aria_disp_sd_axi_monitor_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_AXI_MONITOR_CTRL;
    d.bitc.sd_rd_axi_monitor_restart = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_AXI_MONITOR_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_axi_monitor_ctrl_sd_rd_axi_monitor_restart(void)
{
    return (*(volatile reg_aria_disp_sd_axi_monitor_ctrl_t *)REG_ARIA_DISP_SD_AXI_MONITOR_CTRL).bitc.sd_rd_axi_monitor_restart;
}

void reg_aria_disp_set_sd_axi_monitor_ctrl_sd_wr_axi_monitor_restart(mt_u8 data)
{
    reg_aria_disp_sd_axi_monitor_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_AXI_MONITOR_CTRL;
    d.bitc.sd_wr_axi_monitor_restart = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_AXI_MONITOR_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_axi_monitor_ctrl_sd_wr_axi_monitor_restart(void)
{
    return (*(volatile reg_aria_disp_sd_axi_monitor_ctrl_t *)REG_ARIA_DISP_SD_AXI_MONITOR_CTRL).bitc.sd_wr_axi_monitor_restart;
}


/*!
  register ARIA_DISP_sdrd_cmd_ack_latency_monitor (read/write)
  */
void reg_aria_disp_set_sdrd_cmd_ack_latency_monitor(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDRD_CMD_ACK_LATENCY_MONITOR, data);
}

mt_u32  reg_aria_disp_get_sdrd_cmd_ack_latency_monitor(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SDRD_CMD_ACK_LATENCY_MONITOR);
}

void reg_aria_disp_set_sdrd_cmd_ack_latency_monitor_cmd_ack_latency_max_value(mt_u16 data)
{
    reg_aria_disp_sdrd_cmd_ack_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SDRD_CMD_ACK_LATENCY_MONITOR;
    d.bitc.cmd_ack_latency_max_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDRD_CMD_ACK_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_sdrd_cmd_ack_latency_monitor_cmd_ack_latency_max_value(void)
{
    return (*(volatile reg_aria_disp_sdrd_cmd_ack_latency_monitor_t *)REG_ARIA_DISP_SDRD_CMD_ACK_LATENCY_MONITOR).bitc.cmd_ack_latency_max_value;
}

void reg_aria_disp_set_sdrd_cmd_ack_latency_monitor_cmd_ack_latency_average_value(mt_u16 data)
{
    reg_aria_disp_sdrd_cmd_ack_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SDRD_CMD_ACK_LATENCY_MONITOR;
    d.bitc.cmd_ack_latency_average_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDRD_CMD_ACK_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_sdrd_cmd_ack_latency_monitor_cmd_ack_latency_average_value(void)
{
    return (*(volatile reg_aria_disp_sdrd_cmd_ack_latency_monitor_t *)REG_ARIA_DISP_SDRD_CMD_ACK_LATENCY_MONITOR).bitc.cmd_ack_latency_average_value;
}


/*!
  register ARIA_DISP_sdrd_data_ack_latency_monitor (read/write)
  */
void reg_aria_disp_set_sdrd_data_ack_latency_monitor(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDRD_DATA_ACK_LATENCY_MONITOR, data);
}

mt_u32  reg_aria_disp_get_sdrd_data_ack_latency_monitor(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SDRD_DATA_ACK_LATENCY_MONITOR);
}

void reg_aria_disp_set_sdrd_data_ack_latency_monitor_cmd_data_latency_max_value(mt_u16 data)
{
    reg_aria_disp_sdrd_data_ack_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SDRD_DATA_ACK_LATENCY_MONITOR;
    d.bitc.cmd_data_latency_max_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDRD_DATA_ACK_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_sdrd_data_ack_latency_monitor_cmd_data_latency_max_value(void)
{
    return (*(volatile reg_aria_disp_sdrd_data_ack_latency_monitor_t *)REG_ARIA_DISP_SDRD_DATA_ACK_LATENCY_MONITOR).bitc.cmd_data_latency_max_value;
}

void reg_aria_disp_set_sdrd_data_ack_latency_monitor_cmd_data_latency_average_value(mt_u16 data)
{
    reg_aria_disp_sdrd_data_ack_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SDRD_DATA_ACK_LATENCY_MONITOR;
    d.bitc.cmd_data_latency_average_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDRD_DATA_ACK_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_sdrd_data_ack_latency_monitor_cmd_data_latency_average_value(void)
{
    return (*(volatile reg_aria_disp_sdrd_data_ack_latency_monitor_t *)REG_ARIA_DISP_SDRD_DATA_ACK_LATENCY_MONITOR).bitc.cmd_data_latency_average_value;
}


/*!
  register ARIA_DISP_sdrd_data_last_latency_monitor (read/write)
  */
void reg_aria_disp_set_sdrd_data_last_latency_monitor(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDRD_DATA_LAST_LATENCY_MONITOR, data);
}

mt_u32  reg_aria_disp_get_sdrd_data_last_latency_monitor(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SDRD_DATA_LAST_LATENCY_MONITOR);
}

void reg_aria_disp_set_sdrd_data_last_latency_monitor_last_data_latency_max_value(mt_u16 data)
{
    reg_aria_disp_sdrd_data_last_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SDRD_DATA_LAST_LATENCY_MONITOR;
    d.bitc.last_data_latency_max_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDRD_DATA_LAST_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_sdrd_data_last_latency_monitor_last_data_latency_max_value(void)
{
    return (*(volatile reg_aria_disp_sdrd_data_last_latency_monitor_t *)REG_ARIA_DISP_SDRD_DATA_LAST_LATENCY_MONITOR).bitc.last_data_latency_max_value;
}

void reg_aria_disp_set_sdrd_data_last_latency_monitor_last_data_latency_average_value(mt_u16 data)
{
    reg_aria_disp_sdrd_data_last_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SDRD_DATA_LAST_LATENCY_MONITOR;
    d.bitc.last_data_latency_average_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDRD_DATA_LAST_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_sdrd_data_last_latency_monitor_last_data_latency_average_value(void)
{
    return (*(volatile reg_aria_disp_sdrd_data_last_latency_monitor_t *)REG_ARIA_DISP_SDRD_DATA_LAST_LATENCY_MONITOR).bitc.last_data_latency_average_value;
}


/*!
  register ARIA_DISP_sdwr_cmd_ack_latency_monitor (read/write)
  */
void reg_aria_disp_set_sdwr_cmd_ack_latency_monitor(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDWR_CMD_ACK_LATENCY_MONITOR, data);
}

mt_u32  reg_aria_disp_get_sdwr_cmd_ack_latency_monitor(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SDWR_CMD_ACK_LATENCY_MONITOR);
}

void reg_aria_disp_set_sdwr_cmd_ack_latency_monitor_cmd_ack_latency_max_value(mt_u16 data)
{
    reg_aria_disp_sdwr_cmd_ack_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SDWR_CMD_ACK_LATENCY_MONITOR;
    d.bitc.cmd_ack_latency_max_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDWR_CMD_ACK_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_sdwr_cmd_ack_latency_monitor_cmd_ack_latency_max_value(void)
{
    return (*(volatile reg_aria_disp_sdwr_cmd_ack_latency_monitor_t *)REG_ARIA_DISP_SDWR_CMD_ACK_LATENCY_MONITOR).bitc.cmd_ack_latency_max_value;
}

void reg_aria_disp_set_sdwr_cmd_ack_latency_monitor_cmd_ack_latency_average_value(mt_u16 data)
{
    reg_aria_disp_sdwr_cmd_ack_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SDWR_CMD_ACK_LATENCY_MONITOR;
    d.bitc.cmd_ack_latency_average_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDWR_CMD_ACK_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_sdwr_cmd_ack_latency_monitor_cmd_ack_latency_average_value(void)
{
    return (*(volatile reg_aria_disp_sdwr_cmd_ack_latency_monitor_t *)REG_ARIA_DISP_SDWR_CMD_ACK_LATENCY_MONITOR).bitc.cmd_ack_latency_average_value;
}


/*!
  register ARIA_DISP_sdwr_data_ack_latency_monitor (read/write)
  */
void reg_aria_disp_set_sdwr_data_ack_latency_monitor(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDWR_DATA_ACK_LATENCY_MONITOR, data);
}

mt_u32  reg_aria_disp_get_sdwr_data_ack_latency_monitor(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SDWR_DATA_ACK_LATENCY_MONITOR);
}

void reg_aria_disp_set_sdwr_data_ack_latency_monitor_cmd_data_latency_max_value(mt_u16 data)
{
    reg_aria_disp_sdwr_data_ack_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SDWR_DATA_ACK_LATENCY_MONITOR;
    d.bitc.cmd_data_latency_max_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDWR_DATA_ACK_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_sdwr_data_ack_latency_monitor_cmd_data_latency_max_value(void)
{
    return (*(volatile reg_aria_disp_sdwr_data_ack_latency_monitor_t *)REG_ARIA_DISP_SDWR_DATA_ACK_LATENCY_MONITOR).bitc.cmd_data_latency_max_value;
}

void reg_aria_disp_set_sdwr_data_ack_latency_monitor_cmd_data_latency_average_value(mt_u16 data)
{
    reg_aria_disp_sdwr_data_ack_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SDWR_DATA_ACK_LATENCY_MONITOR;
    d.bitc.cmd_data_latency_average_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDWR_DATA_ACK_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_sdwr_data_ack_latency_monitor_cmd_data_latency_average_value(void)
{
    return (*(volatile reg_aria_disp_sdwr_data_ack_latency_monitor_t *)REG_ARIA_DISP_SDWR_DATA_ACK_LATENCY_MONITOR).bitc.cmd_data_latency_average_value;
}


/*!
  register ARIA_DISP_sdwr_data_bready_latency_monitor (read/write)
  */
void reg_aria_disp_set_sdwr_data_bready_latency_monitor(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDWR_DATA_BREADY_LATENCY_MONITOR, data);
}

mt_u32  reg_aria_disp_get_sdwr_data_bready_latency_monitor(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SDWR_DATA_BREADY_LATENCY_MONITOR);
}

void reg_aria_disp_set_sdwr_data_bready_latency_monitor_data_bready_latency_max_value(mt_u16 data)
{
    reg_aria_disp_sdwr_data_bready_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SDWR_DATA_BREADY_LATENCY_MONITOR;
    d.bitc.data_bready_latency_max_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDWR_DATA_BREADY_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_sdwr_data_bready_latency_monitor_data_bready_latency_max_value(void)
{
    return (*(volatile reg_aria_disp_sdwr_data_bready_latency_monitor_t *)REG_ARIA_DISP_SDWR_DATA_BREADY_LATENCY_MONITOR).bitc.data_bready_latency_max_value;
}

void reg_aria_disp_set_sdwr_data_bready_latency_monitor_data_bready_latency_average_value(mt_u16 data)
{
    reg_aria_disp_sdwr_data_bready_latency_monitor_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SDWR_DATA_BREADY_LATENCY_MONITOR;
    d.bitc.data_bready_latency_average_value = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SDWR_DATA_BREADY_LATENCY_MONITOR, d.all);
}

mt_u16  reg_aria_disp_get_sdwr_data_bready_latency_monitor_data_bready_latency_average_value(void)
{
    return (*(volatile reg_aria_disp_sdwr_data_bready_latency_monitor_t *)REG_ARIA_DISP_SDWR_DATA_BREADY_LATENCY_MONITOR).bitc.data_bready_latency_average_value;
}


/*!
  register ARIA_DISP_sd_video_hue_adjust (read/write)
  */
void reg_aria_disp_set_sd_video_hue_adjust(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_HUE_ADJUST, data);
}

mt_u32  reg_aria_disp_get_sd_video_hue_adjust(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_HUE_ADJUST);
}

void reg_aria_disp_set_sd_video_hue_adjust_sd_sina(mt_u16 data)
{
    reg_aria_disp_sd_video_hue_adjust_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_HUE_ADJUST;
    d.bitc.sd_sina = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_HUE_ADJUST, d.all);
}

mt_u16  reg_aria_disp_get_sd_video_hue_adjust_sd_sina(void)
{
    return (*(volatile reg_aria_disp_sd_video_hue_adjust_t *)REG_ARIA_DISP_SD_VIDEO_HUE_ADJUST).bitc.sd_sina;
}

void reg_aria_disp_set_sd_video_hue_adjust_sd_cosa(mt_u16 data)
{
    reg_aria_disp_sd_video_hue_adjust_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_HUE_ADJUST;
    d.bitc.sd_cosa = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_HUE_ADJUST, d.all);
}

mt_u16  reg_aria_disp_get_sd_video_hue_adjust_sd_cosa(void)
{
    return (*(volatile reg_aria_disp_sd_video_hue_adjust_t *)REG_ARIA_DISP_SD_VIDEO_HUE_ADJUST).bitc.sd_cosa;
}

void reg_aria_disp_set_sd_video_hue_adjust_sd_hue_adjust_en(mt_u8 data)
{
    reg_aria_disp_sd_video_hue_adjust_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_HUE_ADJUST;
    d.bitc.sd_hue_adjust_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_HUE_ADJUST, d.all);
}

mt_u8   reg_aria_disp_get_sd_video_hue_adjust_sd_hue_adjust_en(void)
{
    return (*(volatile reg_aria_disp_sd_video_hue_adjust_t *)REG_ARIA_DISP_SD_VIDEO_HUE_ADJUST).bitc.sd_hue_adjust_en;
}


/*!
  register ARIA_DISP_sd_video_effect_coef (read/write)
  */
void reg_aria_disp_set_sd_video_effect_coef(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_EFFECT_COEF, data);
}

mt_u32  reg_aria_disp_get_sd_video_effect_coef(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_EFFECT_COEF);
}

void reg_aria_disp_set_sd_video_effect_coef_sd_bright_coeff(mt_u8 data)
{
    reg_aria_disp_sd_video_effect_coef_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_EFFECT_COEF;
    d.bitc.sd_bright_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_EFFECT_COEF, d.all);
}

mt_u8   reg_aria_disp_get_sd_video_effect_coef_sd_bright_coeff(void)
{
    return (*(volatile reg_aria_disp_sd_video_effect_coef_t *)REG_ARIA_DISP_SD_VIDEO_EFFECT_COEF).bitc.sd_bright_coeff;
}

void reg_aria_disp_set_sd_video_effect_coef_sd_contrast_coeff(mt_u8 data)
{
    reg_aria_disp_sd_video_effect_coef_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_EFFECT_COEF;
    d.bitc.sd_contrast_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_EFFECT_COEF, d.all);
}

mt_u8   reg_aria_disp_get_sd_video_effect_coef_sd_contrast_coeff(void)
{
    return (*(volatile reg_aria_disp_sd_video_effect_coef_t *)REG_ARIA_DISP_SD_VIDEO_EFFECT_COEF).bitc.sd_contrast_coeff;
}

void reg_aria_disp_set_sd_video_effect_coef_sd_saturation_coeff(mt_u8 data)
{
    reg_aria_disp_sd_video_effect_coef_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_VIDEO_EFFECT_COEF;
    d.bitc.sd_saturation_coeff = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_VIDEO_EFFECT_COEF, d.all);
}

mt_u8   reg_aria_disp_get_sd_video_effect_coef_sd_saturation_coeff(void)
{
    return (*(volatile reg_aria_disp_sd_video_effect_coef_t *)REG_ARIA_DISP_SD_VIDEO_EFFECT_COEF).bitc.sd_saturation_coeff;
}


/*!
  register ARIA_DISP_sd_csc_ctrl (read/write)
  */
void reg_aria_disp_set_sd_csc_ctrl(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_CTRL, data);
}

mt_u32  reg_aria_disp_get_sd_csc_ctrl(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_CTRL);
}

void reg_aria_disp_set_sd_csc_ctrl_sd_bound_output_en(mt_u8 data)
{
    reg_aria_disp_sd_csc_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_CTRL;
    d.bitc.sd_bound_output_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_csc_ctrl_sd_bound_output_en(void)
{
    return (*(volatile reg_aria_disp_sd_csc_ctrl_t *)REG_ARIA_DISP_SD_CSC_CTRL).bitc.sd_bound_output_en;
}

void reg_aria_disp_set_sd_csc_ctrl_sd_bound_input_en(mt_u8 data)
{
    reg_aria_disp_sd_csc_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_CTRL;
    d.bitc.sd_bound_input_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_csc_ctrl_sd_bound_input_en(void)
{
    return (*(volatile reg_aria_disp_sd_csc_ctrl_t *)REG_ARIA_DISP_SD_CSC_CTRL).bitc.sd_bound_input_en;
}

void reg_aria_disp_set_sd_csc_ctrl_sd_csc_en(mt_u8 data)
{
    reg_aria_disp_sd_csc_ctrl_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_CTRL;
    d.bitc.sd_csc_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_CTRL, d.all);
}

mt_u8   reg_aria_disp_get_sd_csc_ctrl_sd_csc_en(void)
{
    return (*(volatile reg_aria_disp_sd_csc_ctrl_t *)REG_ARIA_DISP_SD_CSC_CTRL).bitc.sd_csc_en;
}


/*!
  register ARIA_DISP_sd_csc_coeff1 (read/write)
  */
void reg_aria_disp_set_sd_csc_coeff1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF1, data);
}

mt_u32  reg_aria_disp_get_sd_csc_coeff1(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF1);
}

void reg_aria_disp_set_sd_csc_coeff1_sd_csc_a01(mt_u16 data)
{
    reg_aria_disp_sd_csc_coeff1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF1;
    d.bitc.sd_csc_a01 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF1, d.all);
}

mt_u16  reg_aria_disp_get_sd_csc_coeff1_sd_csc_a01(void)
{
    return (*(volatile reg_aria_disp_sd_csc_coeff1_t *)REG_ARIA_DISP_SD_CSC_COEFF1).bitc.sd_csc_a01;
}

void reg_aria_disp_set_sd_csc_coeff1_sd_csc_a00(mt_u16 data)
{
    reg_aria_disp_sd_csc_coeff1_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF1;
    d.bitc.sd_csc_a00 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF1, d.all);
}

mt_u16  reg_aria_disp_get_sd_csc_coeff1_sd_csc_a00(void)
{
    return (*(volatile reg_aria_disp_sd_csc_coeff1_t *)REG_ARIA_DISP_SD_CSC_COEFF1).bitc.sd_csc_a00;
}


/*!
  register ARIA_DISP_sd_csc_coeff2 (read/write)
  */
void reg_aria_disp_set_sd_csc_coeff2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF2, data);
}

mt_u32  reg_aria_disp_get_sd_csc_coeff2(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF2);
}

void reg_aria_disp_set_sd_csc_coeff2_sd_csc_a10(mt_u16 data)
{
    reg_aria_disp_sd_csc_coeff2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF2;
    d.bitc.sd_csc_a10 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF2, d.all);
}

mt_u16  reg_aria_disp_get_sd_csc_coeff2_sd_csc_a10(void)
{
    return (*(volatile reg_aria_disp_sd_csc_coeff2_t *)REG_ARIA_DISP_SD_CSC_COEFF2).bitc.sd_csc_a10;
}

void reg_aria_disp_set_sd_csc_coeff2_sd_csc_a02(mt_u16 data)
{
    reg_aria_disp_sd_csc_coeff2_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF2;
    d.bitc.sd_csc_a02 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF2, d.all);
}

mt_u16  reg_aria_disp_get_sd_csc_coeff2_sd_csc_a02(void)
{
    return (*(volatile reg_aria_disp_sd_csc_coeff2_t *)REG_ARIA_DISP_SD_CSC_COEFF2).bitc.sd_csc_a02;
}


/*!
  register ARIA_DISP_sd_csc_coeff3 (read/write)
  */
void reg_aria_disp_set_sd_csc_coeff3(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF3, data);
}

mt_u32  reg_aria_disp_get_sd_csc_coeff3(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF3);
}

void reg_aria_disp_set_sd_csc_coeff3_sd_csc_a12(mt_u16 data)
{
    reg_aria_disp_sd_csc_coeff3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF3;
    d.bitc.sd_csc_a12 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF3, d.all);
}

mt_u16  reg_aria_disp_get_sd_csc_coeff3_sd_csc_a12(void)
{
    return (*(volatile reg_aria_disp_sd_csc_coeff3_t *)REG_ARIA_DISP_SD_CSC_COEFF3).bitc.sd_csc_a12;
}

void reg_aria_disp_set_sd_csc_coeff3_sd_csc_a11(mt_u16 data)
{
    reg_aria_disp_sd_csc_coeff3_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF3;
    d.bitc.sd_csc_a11 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF3, d.all);
}

mt_u16  reg_aria_disp_get_sd_csc_coeff3_sd_csc_a11(void)
{
    return (*(volatile reg_aria_disp_sd_csc_coeff3_t *)REG_ARIA_DISP_SD_CSC_COEFF3).bitc.sd_csc_a11;
}


/*!
  register ARIA_DISP_sd_csc_coeff4 (read/write)
  */
void reg_aria_disp_set_sd_csc_coeff4(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF4, data);
}

mt_u32  reg_aria_disp_get_sd_csc_coeff4(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF4);
}

void reg_aria_disp_set_sd_csc_coeff4_sd_csc_a21(mt_u16 data)
{
    reg_aria_disp_sd_csc_coeff4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF4;
    d.bitc.sd_csc_a21 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF4, d.all);
}

mt_u16  reg_aria_disp_get_sd_csc_coeff4_sd_csc_a21(void)
{
    return (*(volatile reg_aria_disp_sd_csc_coeff4_t *)REG_ARIA_DISP_SD_CSC_COEFF4).bitc.sd_csc_a21;
}

void reg_aria_disp_set_sd_csc_coeff4_sd_csc_a20(mt_u16 data)
{
    reg_aria_disp_sd_csc_coeff4_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF4;
    d.bitc.sd_csc_a20 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF4, d.all);
}

mt_u16  reg_aria_disp_get_sd_csc_coeff4_sd_csc_a20(void)
{
    return (*(volatile reg_aria_disp_sd_csc_coeff4_t *)REG_ARIA_DISP_SD_CSC_COEFF4).bitc.sd_csc_a20;
}


/*!
  register ARIA_DISP_sd_csc_coeff5 (read/write)
  */
void reg_aria_disp_set_sd_csc_coeff5(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF5, data);
}

mt_u32  reg_aria_disp_get_sd_csc_coeff5(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF5);
}

void reg_aria_disp_set_sd_csc_coeff5_sd_csc_a22(mt_u16 data)
{
    reg_aria_disp_sd_csc_coeff5_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_CSC_COEFF5;
    d.bitc.sd_csc_a22 = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_CSC_COEFF5, d.all);
}

mt_u16  reg_aria_disp_get_sd_csc_coeff5_sd_csc_a22(void)
{
    return (*(volatile reg_aria_disp_sd_csc_coeff5_t *)REG_ARIA_DISP_SD_CSC_COEFF5).bitc.sd_csc_a22;
}


/*!
  register ARIA_DISP_hd_screen_out_size (read/write)
  */
void reg_aria_disp_set_hd_screen_out_size(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_SCREEN_OUT_SIZE, data);
}

mt_u32  reg_aria_disp_get_hd_screen_out_size(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_HD_SCREEN_OUT_SIZE);
}

void reg_aria_disp_set_hd_screen_out_size_hd_screen_height(mt_u16 data)
{
    reg_aria_disp_hd_screen_out_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_SCREEN_OUT_SIZE;
    d.bitc.hd_screen_height = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_SCREEN_OUT_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_hd_screen_out_size_hd_screen_height(void)
{
    return (*(volatile reg_aria_disp_hd_screen_out_size_t *)REG_ARIA_DISP_HD_SCREEN_OUT_SIZE).bitc.hd_screen_height;
}

void reg_aria_disp_set_hd_screen_out_size_hd_screen_width(mt_u16 data)
{
    reg_aria_disp_hd_screen_out_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_HD_SCREEN_OUT_SIZE;
    d.bitc.hd_screen_width = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_HD_SCREEN_OUT_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_hd_screen_out_size_hd_screen_width(void)
{
    return (*(volatile reg_aria_disp_hd_screen_out_size_t *)REG_ARIA_DISP_HD_SCREEN_OUT_SIZE).bitc.hd_screen_width;
}


/*!
  register ARIA_DISP_sd_screen_out_size (read/write)
  */
void reg_aria_disp_set_sd_screen_out_size(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_SCREEN_OUT_SIZE, data);
}

mt_u32  reg_aria_disp_get_sd_screen_out_size(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_SD_SCREEN_OUT_SIZE);
}

void reg_aria_disp_set_sd_screen_out_size_sd_screen_height(mt_u16 data)
{
    reg_aria_disp_sd_screen_out_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_SCREEN_OUT_SIZE;
    d.bitc.sd_screen_height = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_SCREEN_OUT_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_sd_screen_out_size_sd_screen_height(void)
{
    return (*(volatile reg_aria_disp_sd_screen_out_size_t *)REG_ARIA_DISP_SD_SCREEN_OUT_SIZE).bitc.sd_screen_height;
}

void reg_aria_disp_set_sd_screen_out_size_sd_screen_width(mt_u16 data)
{
    reg_aria_disp_sd_screen_out_size_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_SD_SCREEN_OUT_SIZE;
    d.bitc.sd_screen_width = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_SD_SCREEN_OUT_SIZE, d.all);
}

mt_u16  reg_aria_disp_get_sd_screen_out_size_sd_screen_width(void)
{
    return (*(volatile reg_aria_disp_sd_screen_out_size_t *)REG_ARIA_DISP_SD_SCREEN_OUT_SIZE).bitc.sd_screen_width;
}


/*!
  register ARIA_DISP_coeff_table_sel (read/write)
  */
void reg_aria_disp_set_coeff_table_sel(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, data);
}

mt_u32  reg_aria_disp_get_coeff_table_sel(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL);
}

void reg_aria_disp_set_coeff_table_sel_video_dce_map_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.video_dce_map_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_video_dce_map_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.video_dce_map_load_en;
}

void reg_aria_disp_set_coeff_table_sel_gra1_alpha_hf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.gra1_alpha_hf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_gra1_alpha_hf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.gra1_alpha_hf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_gra1_chroma_hf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.gra1_chroma_hf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_gra1_chroma_hf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.gra1_chroma_hf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_gra1_luma_hf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.gra1_luma_hf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_gra1_luma_hf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.gra1_luma_hf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_gra1_luma_vf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.gra1_luma_vf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_gra1_luma_vf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.gra1_luma_vf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_gra0_alpha_hf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.gra0_alpha_hf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_gra0_alpha_hf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.gra0_alpha_hf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_gra0_chroma_hf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.gra0_chroma_hf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_gra0_chroma_hf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.gra0_chroma_hf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_gra0_luma_hf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.gra0_luma_hf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_gra0_luma_hf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.gra0_luma_hf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_gra0_luma_vf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.gra0_luma_vf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_gra0_luma_vf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.gra0_luma_vf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_stillscalar_chroma_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.stillscalar_chroma_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_stillscalar_chroma_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.stillscalar_chroma_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_stillscalar_luma_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.stillscalar_luma_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_stillscalar_luma_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.stillscalar_luma_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_osd_hf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.osd_hf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_osd_hf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.osd_hf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_osd_vf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.osd_vf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_osd_vf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.osd_vf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_sd_chroma_hf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.sd_chroma_hf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_sd_chroma_hf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.sd_chroma_hf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_sd_luma_hf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.sd_luma_hf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_sd_luma_hf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.sd_luma_hf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_sd_luma_vf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.sd_luma_vf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_sd_luma_vf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.sd_luma_vf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_hd_chroma_hf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.hd_chroma_hf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_hd_chroma_hf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.hd_chroma_hf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_hd_luma_hf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.hd_luma_hf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_hd_luma_hf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.hd_luma_hf_coeff_load_en;
}

void reg_aria_disp_set_coeff_table_sel_hd_luma_vf_coeff_load_en(mt_u8 data)
{
    reg_aria_disp_coeff_table_sel_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_COEFF_TABLE_SEL;
    d.bitc.hd_luma_vf_coeff_load_en = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_COEFF_TABLE_SEL, d.all);
}

mt_u8   reg_aria_disp_get_coeff_table_sel_hd_luma_vf_coeff_load_en(void)
{
    return (*(volatile reg_aria_disp_coeff_table_sel_t *)REG_ARIA_DISP_COEFF_TABLE_SEL).bitc.hd_luma_vf_coeff_load_en;
}


/*!
  register ARIA_DISP_display_ctrl_limit (read/write)
  */
void reg_aria_disp_set_display_ctrl_limit(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DISPLAY_CTRL_LIMIT, data);
}

mt_u32  reg_aria_disp_get_display_ctrl_limit(void)
{
    return (*(volatile mt_u32 *)REG_ARIA_DISP_DISPLAY_CTRL_LIMIT);
}

void reg_aria_disp_set_display_ctrl_limit_wseccpu(mt_u16 data)
{
    reg_aria_disp_display_ctrl_limit_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DISPLAY_CTRL_LIMIT;
    d.bitc.wseccpu = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DISPLAY_CTRL_LIMIT, d.all);
}

mt_u16  reg_aria_disp_get_display_ctrl_limit_wseccpu(void)
{
    return (*(volatile reg_aria_disp_display_ctrl_limit_t *)REG_ARIA_DISP_DISPLAY_CTRL_LIMIT).bitc.wseccpu;
}

void reg_aria_disp_set_display_ctrl_limit_rseccpu(mt_u16 data)
{
    reg_aria_disp_display_ctrl_limit_t d;
    d.all = *(volatile mt_u32 *)REG_ARIA_DISP_DISPLAY_CTRL_LIMIT;
    d.bitc.rseccpu = data;
    hal_put_u32((volatile unsigned long *)REG_ARIA_DISP_DISPLAY_CTRL_LIMIT, d.all);
}

mt_u16  reg_aria_disp_get_display_ctrl_limit_rseccpu(void)
{
    return (*(volatile reg_aria_disp_display_ctrl_limit_t *)REG_ARIA_DISP_DISPLAY_CTRL_LIMIT).bitc.rseccpu;
}


/*!
  init function
  */
void reg_aria_disp_init(void)
{
    reg_aria_disp_set_video_ctrl_1((mt_u32)0x00100000);
    reg_aria_disp_set_video_ctrl_2((mt_u32)0x00ff1010);
    reg_aria_disp_set_video_input_frame_size((mt_u32)0x07800438);
    reg_aria_disp_set_video_crop_en((mt_u32)0x00000000);
    reg_aria_disp_set_video_crop_pixel((mt_u32)0x00000000);
    reg_aria_disp_set_video_crop_line((mt_u32)0x00000000);
    reg_aria_disp_set_video_pix_align_ctrl((mt_u32)0x00000000);
    reg_aria_disp_set_video_data_endian_ctrl((mt_u32)0x00000000);
    reg_aria_disp_set_video_burst_info_1((mt_u32)0x00000001);
    reg_aria_disp_set_video_burst_info_2((mt_u32)0x00000000);
    reg_aria_disp_set_video_line_rd_cnt_max((mt_u32)0x000000f0);
    reg_aria_disp_set_video_half_scale_ctrl((mt_u32)0x00000000);
    reg_aria_disp_set_video_hf_phase((mt_u32)0x00007000);
    reg_aria_disp_set_video_scale_init_phase_offset((mt_u32)0x00000000);
    reg_aria_disp_set_video_scale_hd_ratio((mt_u32)0x10001000);
    reg_aria_disp_set_video_scale_hd_init_ratio((mt_u32)0x00000000);
    reg_aria_disp_set_video_hd_window_x((mt_u32)0x00010781);
    reg_aria_disp_set_video_hd_window_y((mt_u32)0x00010439);
    reg_aria_disp_set_video_hd_window_cut((mt_u32)0x00000000);
    reg_aria_disp_set_video_scale_sd_ratio((mt_u32)0x2aaa1e00);
    reg_aria_disp_set_video_scale_sd_init_ratio((mt_u32)0x00000000);
    reg_aria_disp_set_video_sd_window_x((mt_u32)0x000102d1);
    reg_aria_disp_set_video_sd_window_y((mt_u32)0x00010121);
    reg_aria_disp_set_video_sd_window_cut((mt_u32)0x00000000);
    reg_aria_disp_set_sd_video_path_ctrl((mt_u32)0x00000000);
    reg_aria_disp_set_hd_luma_vf_coeff_addr((mt_u32)0x00000000);
    reg_aria_disp_set_hd_luma_hf_coeff_addr((mt_u32)0x00000000);
    reg_aria_disp_set_hd_chroma_hf_coeff_addr((mt_u32)0x00000000);
    reg_aria_disp_set_sd_luma_vf_coeff_addr((mt_u32)0x00000000);
    reg_aria_disp_set_sd_luma_hf_coeff_addr((mt_u32)0x00000000);
    reg_aria_disp_set_sd_chroma_hf_coeff_addr((mt_u32)0x00000000);
    reg_aria_disp_set_video_dce_map_addr((mt_u32)0x00000000);
    reg_aria_disp_set_histo_info_addr_0((mt_u32)0x00000000);
    reg_aria_disp_set_histo_info_addr_2((mt_u32)0x00000000);
    reg_aria_disp_set_chroma_upscale_ctrl((mt_u32)0x00000000);
    reg_aria_disp_set_chroma_coef_0((mt_u32)0x000a0796);
    reg_aria_disp_set_chroma_coef_1((mt_u32)0x00f602d4);
    reg_aria_disp_set_chroma_coef_2((mt_u32)0x00f60796);
    reg_aria_disp_set_chroma_coef_3((mt_u32)0x000a0000);
    reg_aria_disp_set_color_enhance_ctrl((mt_u32)0x47f04000);
    reg_aria_disp_set_video_dce_config((mt_u32)0x00000000);
    reg_aria_disp_set_video_horf_config((mt_u32)0x00000808);
    reg_aria_disp_set_hd_video_post_config((mt_u32)0x00000000);
    reg_aria_disp_set_sd_video_post_config((mt_u32)0x00000000);
    reg_aria_disp_set_hd_video_effect_coef((mt_u32)0x00808080);
    reg_aria_disp_set_hd_video_hue_adjust((mt_u32)0x03ff0000);
    reg_aria_disp_set_sd_video_drop_line((mt_u32)0x00000000);
    reg_aria_disp_set_video_scalar_buf_full_thr((mt_u32)0x002a000a);
    reg_aria_disp_set_video_scalar_outbuf_full_thr((mt_u32)0x00400040);
    reg_aria_disp_set_axi_cmd_req_fifo_thr((mt_u32)0x00080a0a);
    reg_aria_disp_set_access_fifo_lo_hi_thr((mt_u32)0x00080038);
    reg_aria_disp_set_access_fifo_req_thr((mt_u32)0x043d3b37);
    reg_aria_disp_set_asym_fifo_thr((mt_u32)0x00000030);
    reg_aria_disp_set_video_hd_line_cnt((mt_u32)0x00000000);
    reg_aria_disp_set_video_sd_line_cnt((mt_u32)0x00000000);
    reg_aria_disp_set_video_line_proc_status((mt_u32)0x00000000);
    reg_aria_disp_set_alising_prob_reg1((mt_u32)0x01000000);
    reg_aria_disp_set_alising_prob_reg2((mt_u32)0x8e901000);
    reg_aria_disp_set_alising_prob_reg3((mt_u32)0x0101ff40);
    reg_aria_disp_set_alising_prob_reg4((mt_u32)0x40181889);
    reg_aria_disp_set_di_ctrl((mt_u32)0x00000000);
    reg_aria_disp_set_di_pause_en((mt_u32)0x00000010);
    reg_aria_disp_set_di_oper_mode((mt_u32)0x310a1000);
    reg_aria_disp_set_di_p_or_n_pair((mt_u32)0x00000000);
    reg_aria_disp_set_di_para((mt_u32)0x000c000c);
    reg_aria_disp_set_di_chroma_para((mt_u32)0x00000001);
    reg_aria_disp_set_di_alpha_para((mt_u32)0x00011400);
    reg_aria_disp_set_di_motion_ctrl_1((mt_u32)0x4040d803);
    reg_aria_disp_set_di_motion_ctrl_2((mt_u32)0x2a020001);
    reg_aria_disp_set_di_motion_ctrl_3((mt_u32)0x02020801);
    reg_aria_disp_set_di_motion_ctrl_4((mt_u32)0x0300020c);
    reg_aria_disp_set_di_acc_odd_result((mt_u32)0x00000000);
    reg_aria_disp_set_di_acc_even_result((mt_u32)0x00000000);
    reg_aria_disp_set_di_fields_flag((mt_u32)0x01011010);
    reg_aria_disp_set_di_hevc_flag((mt_u32)0x00000000);
    reg_aria_disp_set_none_di_progressive_flag((mt_u32)0x00000000);
    reg_aria_disp_set_none_di_fields_flag((mt_u32)0x00000111);
    reg_aria_disp_set_none_di_hevc_flag((mt_u32)0x00000000);
    reg_aria_disp_set_none_di_progressive_flag_2nd((mt_u32)0x00000000);
    reg_aria_disp_set_none_di_fields_flag_2nd((mt_u32)0x00000111);
    reg_aria_disp_set_none_di_hevc_flag_2nd((mt_u32)0x00000000);
    reg_aria_disp_set_motion_pre_addr_0((mt_u32)0x00020000);
    reg_aria_disp_set_motion_cur_addr_0((mt_u32)0x00020000);
    reg_aria_disp_set_motion_pre_addr_2((mt_u32)0x00020000);
    reg_aria_disp_set_motion_cur_addr_2((mt_u32)0x00020000);
    reg_aria_disp_set_luma_pre_addr_0((mt_u32)0x00200000);
    reg_aria_disp_set_luma_top_cur_addr_0((mt_u32)0x00200000);
    reg_aria_disp_set_luma_bot_cur_addr_0((mt_u32)0x00200000);
    reg_aria_disp_set_luma_nxt_addr_0((mt_u32)0x00200000);
    reg_aria_disp_set_luma_top_cur_addr_2nd_0((mt_u32)0x00200000);
    reg_aria_disp_set_luma_bot_cur_addr_2nd_0((mt_u32)0x00200000);
    reg_aria_disp_set_luma_pre_addr_2((mt_u32)0x00200000);
    reg_aria_disp_set_luma_top_cur_addr_2((mt_u32)0x00200000);
    reg_aria_disp_set_luma_bot_cur_addr_2((mt_u32)0x00200000);
    reg_aria_disp_set_luma_nxt_addr_2((mt_u32)0x00200000);
    reg_aria_disp_set_luma_top_cur_addr_2nd_2((mt_u32)0x00200000);
    reg_aria_disp_set_luma_bot_cur_addr_2nd_2((mt_u32)0x00200000);
    reg_aria_disp_set_chroma_ppre_addr_0((mt_u32)0x00500000);
    reg_aria_disp_set_chroma_pre_addr_0((mt_u32)0x00500000);
    reg_aria_disp_set_chroma_top_cur_addr_0((mt_u32)0x00500000);
    reg_aria_disp_set_chroma_bot_cur_addr_0((mt_u32)0x00500000);
    reg_aria_disp_set_chroma_nxt_addr_0((mt_u32)0x00500000);
    reg_aria_disp_set_chroma_top_cur_addr_2nd_0((mt_u32)0x00500000);
    reg_aria_disp_set_chroma_bot_cur_addr_2nd_0((mt_u32)0x00500000);
    reg_aria_disp_set_chroma_ppre_addr_2((mt_u32)0x00500000);
    reg_aria_disp_set_chroma_pre_addr_2((mt_u32)0x00500000);
    reg_aria_disp_set_chroma_top_cur_addr_2((mt_u32)0x00500000);
    reg_aria_disp_set_chroma_bot_cur_addr_2((mt_u32)0x00500000);
    reg_aria_disp_set_chroma_nxt_addr_2((mt_u32)0x00500000);
    reg_aria_disp_set_chroma_top_cur_addr_2nd_2((mt_u32)0x00500000);
    reg_aria_disp_set_chroma_bot_cur_addr_2nd_2((mt_u32)0x00500000);
    reg_aria_disp_set_down_scale_ctrl((mt_u32)0x80800200);
    reg_aria_disp_set_down_scale_out_size((mt_u32)0x0780021c);
    reg_aria_disp_set_down_scale_wr_stride((mt_u32)0x00000780);
    reg_aria_disp_set_down_scale_luma_wr_addr_0((mt_u32)0x00200000);
    reg_aria_disp_set_down_scale_cbcr_wr_addr_0((mt_u32)0x00500000);
    reg_aria_disp_set_down_scale_luma_wr_addr_2((mt_u32)0x00200000);
    reg_aria_disp_set_down_scale_cbcr_wr_addr_2((mt_u32)0x00500000);
    reg_aria_disp_set_down_scale_ctrl_2nd((mt_u32)0x80800200);
    reg_aria_disp_set_down_scale_out_size_2nd((mt_u32)0x0780021c);
    reg_aria_disp_set_down_scale_wr_stride_2nd((mt_u32)0x00000780);
    reg_aria_disp_set_down_scale_luma_wr_addr_0_2nd((mt_u32)0x00200000);
    reg_aria_disp_set_down_scale_cbcr_wr_addr_0_2nd((mt_u32)0x00500000);
    reg_aria_disp_set_down_scale_luma_wr_addr_2_2nd((mt_u32)0x00200000);
    reg_aria_disp_set_down_scale_cbcr_wr_addr_2_2nd((mt_u32)0x00500000);
    reg_aria_disp_set_down_scale_data_endian_ctrl((mt_u32)0x00000000);
    reg_aria_disp_set_hd_csc_ctrl((mt_u32)0x00000110);
    reg_aria_disp_set_hd_csc_coef_1((mt_u32)0x04000000);
    reg_aria_disp_set_hd_csc_coef_2((mt_u32)0x00000000);
    reg_aria_disp_set_hd_csc_coef_3((mt_u32)0x04000000);
    reg_aria_disp_set_hd_csc_coef_4((mt_u32)0x00000000);
    reg_aria_disp_set_hd_csc_coef_5((mt_u32)0x04000000);
    reg_aria_disp_set_tile_para((mt_u32)0x00001102);
    reg_aria_disp_set_tile_rowjump_00((mt_u32)0x08040810);
    reg_aria_disp_set_tile_rowjump_01((mt_u32)0x10081020);
    reg_aria_disp_set_tile_rowjump_10((mt_u32)0x08040810);
    reg_aria_disp_set_tile_rowjump_11((mt_u32)0x10081020);
    reg_aria_disp_set_denoise_ctrl((mt_u32)0x000f0000);
    reg_aria_disp_set_denoise_para_1((mt_u32)0x00110007);
    reg_aria_disp_set_denoise_para_2((mt_u32)0x000206fe);
    reg_aria_disp_set_denoise_para_3((mt_u32)0x05000225);
    reg_aria_disp_set_background_color((mt_u32)0x00108080);
    reg_aria_disp_set_hdenc_test_cmd((mt_u32)0x00000000);
    reg_aria_disp_set_hdenc_test_data((mt_u32)0x00000000);
    reg_aria_disp_set_video_axi_monitor_clr((mt_u32)0x00000000);
    reg_aria_disp_set_video_read_cmd_latency_cnt_max((mt_u32)0x00000000);
    reg_aria_disp_set_video_read_cmd_latency_cnt_sum((mt_u32)0x00000000);
    reg_aria_disp_set_video_read_cmd_req_cnt_sum((mt_u32)0x00000000);
    reg_aria_disp_set_video_read_data_latency_cnt_max((mt_u32)0x00000000);
    reg_aria_disp_set_video_read_data_latency_cnt_sum((mt_u32)0x00000000);
    reg_aria_disp_set_video_read_data_req_cnt_sum((mt_u32)0x00000000);
    reg_aria_disp_set_motion_write_cmd_latency_cnt_max((mt_u32)0x00000000);
    reg_aria_disp_set_motion_write_cmd_latency_cnt_sum((mt_u32)0x00000000);
    reg_aria_disp_set_motion_write_cmd_req_cnt_sum((mt_u32)0x00000000);
    reg_aria_disp_set_motion_write_data_latency_cnt_max((mt_u32)0x00000000);
    reg_aria_disp_set_motion_write_data_latency_cnt_sum((mt_u32)0x00000000);
    reg_aria_disp_set_motion_write_data_req_cnt_sum((mt_u32)0x00000000);
    reg_aria_disp_set_osdl_osd0_cmd((mt_u32)0x00000010);
    reg_aria_disp_set_osdl_osd1_cmd((mt_u32)0x00000000);
    reg_aria_disp_set_osdl_sub_cmd((mt_u32)0x00000000);
    reg_aria_disp_set_osdl_cmd((mt_u32)0x001f0000);
    reg_aria_disp_set_osdl_osd0_ini_addr((mt_u32)0x01000000);
    reg_aria_disp_set_osdl_osd1_ini_addr((mt_u32)0x01800000);
    reg_aria_disp_set_osdl_sub_ini_addr((mt_u32)0x02800000);
    reg_aria_disp_set_osdl_ff_threshold((mt_u32)0x0000001f);
    reg_aria_disp_set_osdl_rgb2y_coeff((mt_u32)0x06481107);
    reg_aria_disp_set_osdl_rgb2cb_coeff((mt_u32)0x1c24a497);
    reg_aria_disp_set_osdl_rgb2cr_coeff((mt_u32)0x0485e1c2);
    reg_aria_disp_set_osdl_y_offset((mt_u32)0x00004000);
    reg_aria_disp_set_osdl_cbcr_offset((mt_u32)0x00020000);
    reg_aria_disp_set_osdl_osd0_debug((mt_u32)0x00000002);
    reg_aria_disp_set_osdl_osd1_debug((mt_u32)0x00000000);
    reg_aria_disp_set_osdl_sub_debug((mt_u32)0x00000000);
    reg_aria_disp_set_osdl_cmd_ack_latency((mt_u32)0x00000000);
    reg_aria_disp_set_osdl_cmd_dat_latency((mt_u32)0x00000000);
    reg_aria_disp_set_osdl_datlast_latency((mt_u32)0x00000000);
    reg_aria_disp_set_osdm_cmd((mt_u32)0x00000000);
    reg_aria_disp_set_osdm_threshold((mt_u32)0x0000041c);
    reg_aria_disp_set_osdm_osd0_ckey((mt_u32)0x00000000);
    reg_aria_disp_set_osdm_osd1_ckey((mt_u32)0x00000000);
    reg_aria_disp_set_osds_cmd((mt_u32)0x10000421);
    reg_aria_disp_set_osds_hsize((mt_u32)0x07800000);
    reg_aria_disp_set_osds_hratio((mt_u32)0x00001000);
    reg_aria_disp_set_osds_hf_coeff_addr((mt_u32)0x02000000);
    reg_aria_disp_set_osds_vsize((mt_u32)0x021c021c);
    reg_aria_disp_set_osds_vratio((mt_u32)0x00000000);
    reg_aria_disp_set_osds_vf_coeff_addr((mt_u32)0x02000800);
    reg_aria_disp_set_osds_v_start_line((mt_u32)0x00000011);
    reg_aria_disp_set_osds_v_start_fra((mt_u32)0x00000000);
    reg_aria_disp_set_osds_v_tap((mt_u32)0x00000003);
    reg_aria_disp_set_osdd_osd0_cmd((mt_u32)0x00000000);
    reg_aria_disp_set_osdd_osd0_length_a((mt_u32)0x00015180);
    reg_aria_disp_set_osdd_osd0_length_r((mt_u32)0x00015180);
    reg_aria_disp_set_osdd_osd0_length_g((mt_u32)0x00015180);
    reg_aria_disp_set_osdd_osd0_length_b((mt_u32)0x00015180);
    reg_aria_disp_set_osdd_osd0_addr_a((mt_u32)0x01000800);
    reg_aria_disp_set_osdd_osd0_addr_r((mt_u32)0x01080000);
    reg_aria_disp_set_osdd_osd0_addr_g((mt_u32)0x01100000);
    reg_aria_disp_set_osdd_osd0_addr_b((mt_u32)0x01180000);
    reg_aria_disp_set_osdd_osd0_ctl((mt_u32)0x00005555);
    reg_aria_disp_set_osdd_osd0_ctl2((mt_u32)0x0000f00f);
    reg_aria_disp_set_osdd_osd1_cmd((mt_u32)0x00000000);
    reg_aria_disp_set_osdd_osd1_length_a((mt_u32)0x00015180);
    reg_aria_disp_set_osdd_osd1_length_r((mt_u32)0x00015180);
    reg_aria_disp_set_osdd_osd1_length_g((mt_u32)0x00015180);
    reg_aria_disp_set_osdd_osd1_length_b((mt_u32)0x00015180);
    reg_aria_disp_set_osdd_osd1_addr_a((mt_u32)0x01000800);
    reg_aria_disp_set_osdd_osd1_addr_r((mt_u32)0x01080000);
    reg_aria_disp_set_osdd_osd1_addr_g((mt_u32)0x01100000);
    reg_aria_disp_set_osdd_osd1_addr_b((mt_u32)0x01180000);
    reg_aria_disp_set_osdd_osd1_ctl((mt_u32)0x00005555);
    reg_aria_disp_set_osdd_osd1_ctl2((mt_u32)0x0000f00f);
    reg_aria_disp_set_osdd_sub_cmd((mt_u32)0x00000000);
    reg_aria_disp_set_osdd_sub_length_a((mt_u32)0x00015180);
    reg_aria_disp_set_osdd_sub_length_r((mt_u32)0x00015180);
    reg_aria_disp_set_osdd_sub_lenght_g((mt_u32)0x00015180);
    reg_aria_disp_set_osdd_sub_length_b((mt_u32)0x00015180);
    reg_aria_disp_set_osdd_sub_addr_a((mt_u32)0x01000800);
    reg_aria_disp_set_osdd_sub_addr_r((mt_u32)0x01080000);
    reg_aria_disp_set_osdd_sub_addr_g((mt_u32)0x01100000);
    reg_aria_disp_set_osdd_sub_addr_b((mt_u32)0x01180000);
    reg_aria_disp_set_osdd_sub_ctl((mt_u32)0x00005555);
    reg_aria_disp_set_osdd_sub_ctl2((mt_u32)0x0000f00f);
    reg_aria_disp_set_osdc_cmd((mt_u32)0x04380780);
    reg_aria_disp_set_osdc_rst((mt_u32)0x0000ff00);
    reg_aria_disp_set_osdc_ctl((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_ctl2((mt_u32)0x0000f00f);
    reg_aria_disp_set_osdc_ffrd_threshold((mt_u32)0x00003d04);
    reg_aria_disp_set_osdc_ffwr_threshold((mt_u32)0x4d4d4d4d);
    reg_aria_disp_set_osdc_ddr_rd_addr((mt_u32)0x04000000);
    reg_aria_disp_set_osdc_width_stride((mt_u32)0x000001ff);
    reg_aria_disp_set_osdc_ddr_wr_addr_a((mt_u32)0x05000000);
    reg_aria_disp_set_osdc_ddr_wr_addr_r((mt_u32)0x05400000);
    reg_aria_disp_set_osdc_ddr_wr_addr_g((mt_u32)0x05800000);
    reg_aria_disp_set_osdc_ddr_wr_addr_b((mt_u32)0x05c00000);
    reg_aria_disp_set_osdc_status((mt_u32)0x8000000c);
    reg_aria_disp_set_osdc_irq_en((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_irq((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_compress_bit_a((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_compress_bit_r((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_compress_bit_g((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_compress_bit_b((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_bits_max_a((mt_u32)0x007e9000);
    reg_aria_disp_set_osdc_bits_max_r((mt_u32)0x007e9000);
    reg_aria_disp_set_osdc_bits_max_g((mt_u32)0x007e9000);
    reg_aria_disp_set_osdc_bits_max_b((mt_u32)0x007e9000);
    reg_aria_disp_set_osdc_cmd_ack_latency_avg((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_cmd_dat_latency_avg((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_datlast_latency_avg((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_cmd_ack_latency_max((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_cmd_dat_latency_max((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_datlast_latency_max((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_redundant0((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_redundant1((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_redundant2((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_redundant3((mt_u32)0x00000000);
    reg_aria_disp_set_osdc_redundant4((mt_u32)0x00000000);
    reg_aria_disp_set_still_control((mt_u32)0x82100000);
    reg_aria_disp_set_still_latch_command((mt_u32)0x00010000);
    reg_aria_disp_set_still_read_x_cfg((mt_u32)0x077f0000);
    reg_aria_disp_set_still_read_y_cfg((mt_u32)0x04370000);
    reg_aria_disp_set_still_stride((mt_u32)0x00000780);
    reg_aria_disp_set_still_luma_baseaddr((mt_u32)0x00010000);
    reg_aria_disp_set_still_cbcr_baseaddr((mt_u32)0x00016000);
    reg_aria_disp_set_still_fifo_threshold((mt_u32)0x0014001f);
    reg_aria_disp_set_still_tile_parameter((mt_u32)0x00001102);
    reg_aria_disp_set_still_tile_rowjump_00((mt_u32)0x08040810);
    reg_aria_disp_set_still_tile_rowjump_01((mt_u32)0x10081020);
    reg_aria_disp_set_still_tile_rowjump_10((mt_u32)0x08040810);
    reg_aria_disp_set_still_tile_rowjump_11((mt_u32)0x10081020);
    reg_aria_disp_set_still_status((mt_u32)0x00000000);
    reg_aria_disp_set_still_axi_monitor_ctrl((mt_u32)0x00000000);
    reg_aria_disp_set_still_cmd_ack_latency_monitor((mt_u32)0x00000000);
    reg_aria_disp_set_still_data_ack_latency_monitor((mt_u32)0x00000000);
    reg_aria_disp_set_still_data_last_latency_monitor((mt_u32)0x00000000);
    reg_aria_disp_set_still_scale_ctrl((mt_u32)0x00022000);
    reg_aria_disp_set_still_scale_h_ratio((mt_u32)0x00000001);
    reg_aria_disp_set_still_scale_v_ratio((mt_u32)0x00000001);
    reg_aria_disp_set_still_scale_h_start_fra((mt_u32)0x00000800);
    reg_aria_disp_set_still_scale_v_start_fra((mt_u32)0x00000000);
    reg_aria_disp_set_still_scale_hsize((mt_u32)0x00000780);
    reg_aria_disp_set_still_scale_vsize((mt_u32)0x0000021c);
    reg_aria_disp_set_still_x_config((mt_u32)0x00000001);
    reg_aria_disp_set_still_y_config((mt_u32)0x00000001);
    reg_aria_disp_set_still_scale_y_coeff_address((mt_u32)0x00000000);
    reg_aria_disp_set_still_scale_uv_coeff_address((mt_u32)0x00000000);
    reg_aria_disp_set_still_scale_fifo1_threshold((mt_u32)0x00000014);
    reg_aria_disp_set_still_scale_fifo2_threshold((mt_u32)0x000003f0);
    reg_aria_disp_set_still_scaler_status((mt_u32)0x00000000);
    reg_aria_disp_set_still_csc_ctrl((mt_u32)0x01000100);
    reg_aria_disp_set_still_csc_coeff1((mt_u32)0x04000000);
    reg_aria_disp_set_still_csc_coeff2((mt_u32)0x00000000);
    reg_aria_disp_set_still_csc_coeff3((mt_u32)0x04000000);
    reg_aria_disp_set_still_csc_coeff4((mt_u32)0x00000000);
    reg_aria_disp_set_still_csc_coeff5((mt_u32)0x04000000);
    reg_aria_disp_set_pres_cmd((mt_u32)0x40000000);
    reg_aria_disp_set_pres_id((mt_u32)0x00000000);
    reg_aria_disp_set_pres_cmd2((mt_u32)0x000ffff3);
    reg_aria_disp_set_pres_lum_raddr((mt_u32)0x04000000);
    reg_aria_disp_set_pres_lum_raddr_2((mt_u32)0x04000000);
    reg_aria_disp_set_pres_lum_waddr((mt_u32)0x05000000);
    reg_aria_disp_set_pres_lum_waddr_2((mt_u32)0x05000000);
    reg_aria_disp_set_pres_chm_raddr((mt_u32)0x04000000);
    reg_aria_disp_set_pres_chm_raddr_2((mt_u32)0x04000000);
    reg_aria_disp_set_pres_chm_waddr((mt_u32)0x05000000);
    reg_aria_disp_set_pres_chm_waddr_2((mt_u32)0x05000000);
    reg_aria_disp_set_pres_hcoeff_lum_addr((mt_u32)0x02000000);
    reg_aria_disp_set_pres_vcoeff_lum_addr((mt_u32)0x02000800);
    reg_aria_disp_set_pres_hcoeff_chm_addr((mt_u32)0x02000000);
    reg_aria_disp_set_pres_vcoeff_chm_addr((mt_u32)0x02000800);
    reg_aria_disp_set_pres_ffr_threshold((mt_u32)0x1c04041c);
    reg_aria_disp_set_pres_ffw_threshold((mt_u32)0x00000000);
    reg_aria_disp_set_pres_ddr_wr_stride((mt_u32)0x00000780);
    reg_aria_disp_set_pres_irq_en((mt_u32)0x00000000);
    reg_aria_disp_set_pres_irq((mt_u32)0x00000000);
    reg_aria_disp_set_pres_src_size((mt_u32)0x07800438);
    reg_aria_disp_set_pres_dst_size((mt_u32)0x07800438);
    reg_aria_disp_set_pres_hratio((mt_u32)0x00001000);
    reg_aria_disp_set_pres_vratio((mt_u32)0x00001000);
    reg_aria_disp_set_pres_hinit((mt_u32)0x00000000);
    reg_aria_disp_set_pres_vinit((mt_u32)0x00000000);
    reg_aria_disp_set_pres_status((mt_u32)0x800000c0);
    reg_aria_disp_set_pres_tile_rowjump_00((mt_u32)0x00000000);
    reg_aria_disp_set_pres_tile_rowjump_01((mt_u32)0x00000000);
    reg_aria_disp_set_pres_tile_rowjump_10((mt_u32)0x00000000);
    reg_aria_disp_set_pres_tile_rowjump_11((mt_u32)0x00000000);
    reg_aria_disp_set_pres_tile_para((mt_u32)0x00000000);
    reg_aria_disp_set_pres_cmd_ack_latency_avg((mt_u32)0x00000000);
    reg_aria_disp_set_pres_cmd_dat_latency_avg((mt_u32)0x00000000);
    reg_aria_disp_set_pres_datlast_latency_avg((mt_u32)0x00000000);
    reg_aria_disp_set_pres_cmd_ack_latency_max((mt_u32)0x00000000);
    reg_aria_disp_set_pres_cmd_dat_latency_max((mt_u32)0x00000000);
    reg_aria_disp_set_pres_datlast_latency_max((mt_u32)0x00000000);
    reg_aria_disp_set_pres_status1((mt_u32)0x00000000);
    reg_aria_disp_set_pres_status2((mt_u32)0x00000000);
    reg_aria_disp_set_pres_status3((mt_u32)0x00000000);
    reg_aria_disp_set_gra_scale0_ctrl((mt_u32)0x04033700);
    reg_aria_disp_set_gra_scale0_h_ratio((mt_u32)0x00022202);
    reg_aria_disp_set_gra_scale0_v_ratio((mt_u32)0x00000002);
    reg_aria_disp_set_gra_scale0_h_start_fra((mt_u32)0x00000000);
    reg_aria_disp_set_gra_scale0_v_start_fra((mt_u32)0x00000000);
    reg_aria_disp_set_gra_scale0_postprocess((mt_u32)0x00000010);
    reg_aria_disp_set_gra_scale0_hscaler_alpha_coeff_address((mt_u32)0x00000000);
    reg_aria_disp_set_gra_scale0_hscaler_luma_coeff_address((mt_u32)0x00000000);
    reg_aria_disp_set_gra_scale0_hscaler_cbcr_coeff_address((mt_u32)0x00000000);
    reg_aria_disp_set_gra_scale0_vscaler_luma_coeff_address((mt_u32)0x00000000);
    reg_aria_disp_set_gra_scale0_output_size((mt_u32)0x0780021c);
    reg_aria_disp_set_gra_scale1_ctrl((mt_u32)0x04033703);
    reg_aria_disp_set_gra_scale1_h_ratio((mt_u32)0x000aaa02);
    reg_aria_disp_set_gra_scale1_v_ratio((mt_u32)0x000e0001);
    reg_aria_disp_set_gra_scale1_h_start_fra((mt_u32)0x00000000);
    reg_aria_disp_set_gra_scale1_v_start_fra((mt_u32)0x00000000);
    reg_aria_disp_set_gra_scale1_postprocess((mt_u32)0x00000010);
    reg_aria_disp_set_gra_scale1_hscaler_alpha_coeff_address((mt_u32)0x00000000);
    reg_aria_disp_set_gra_scale1_hscaler_luma_coeff_address((mt_u32)0x00000000);
    reg_aria_disp_set_gra_scale1_hscaler_cbcr_coeff_address((mt_u32)0x00000000);
    reg_aria_disp_set_gra_scale1_vscaler_luma_coeff_address((mt_u32)0x00000000);
    reg_aria_disp_set_gra_scale_fifo_threshold((mt_u32)0x02f003f8);
    reg_aria_disp_set_gra_scaler_status((mt_u32)0x00000000);
    reg_aria_disp_set_gra_saler_latch_cmd((mt_u32)0x00010000);
    reg_aria_disp_set_sd_wr_ctrl((mt_u32)0x11001032);
    reg_aria_disp_set_sd_latch_command((mt_u32)0x00010000);
    reg_aria_disp_set_sd_wrback_addr_odd((mt_u32)0x00040000);
    reg_aria_disp_set_sd_wrback_addr_even((mt_u32)0x0000ca80);
    reg_aria_disp_set_sd_rdback_addr_odd((mt_u32)0x00040000);
    reg_aria_disp_set_sd_rdback_addr_even((mt_u32)0x00019500);
    reg_aria_disp_set_sd_wrback_fifo_threshold((mt_u32)0x00000080);
    reg_aria_disp_set_sd_blankscreen_mode((mt_u32)0x00108080);
    reg_aria_disp_set_sd_status((mt_u32)0x00010011);
    reg_aria_disp_set_sd_axi_monitor_ctrl((mt_u32)0x00000000);
    reg_aria_disp_set_sdrd_cmd_ack_latency_monitor((mt_u32)0x00000000);
    reg_aria_disp_set_sdrd_data_ack_latency_monitor((mt_u32)0x00000000);
    reg_aria_disp_set_sdrd_data_last_latency_monitor((mt_u32)0x00000000);
    reg_aria_disp_set_sdwr_cmd_ack_latency_monitor((mt_u32)0x00000000);
    reg_aria_disp_set_sdwr_data_ack_latency_monitor((mt_u32)0x00000000);
    reg_aria_disp_set_sdwr_data_bready_latency_monitor((mt_u32)0x00000000);
    reg_aria_disp_set_sd_video_hue_adjust((mt_u32)0x00000000);
    reg_aria_disp_set_sd_video_effect_coef((mt_u32)0x00808080);
    reg_aria_disp_set_sd_csc_ctrl((mt_u32)0x00000000);
    reg_aria_disp_set_sd_csc_coeff1((mt_u32)0x00000000);
    reg_aria_disp_set_sd_csc_coeff2((mt_u32)0x00000000);
    reg_aria_disp_set_sd_csc_coeff3((mt_u32)0x00000000);
    reg_aria_disp_set_sd_csc_coeff4((mt_u32)0x00000000);
    reg_aria_disp_set_sd_csc_coeff5((mt_u32)0x00000000);
    reg_aria_disp_set_hd_screen_out_size((mt_u32)0x0780021c);
    reg_aria_disp_set_sd_screen_out_size((mt_u32)0x02d00120);
    reg_aria_disp_set_coeff_table_sel((mt_u32)0x00000000);
    reg_aria_disp_set_display_ctrl_limit((mt_u32)0x00000000);
    /* read read-clear registers in order to set mirror variables */
}

/*!
  end of file
  */

