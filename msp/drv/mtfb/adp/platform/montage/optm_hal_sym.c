/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/types.h>    /* size_t */
#include <linux/compiler.h>

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#include "optm_hal_sym.h"

static inline void hal_put_u32(volatile unsigned long *p_addr, unsigned long data)
{
    //*(p_addr) = data;
    HAL_PUT_U32((volatile u32*)p_addr, (u32)data);
}
static inline MT_U32 hal_get_u32(volatile MT_U32 *p_addr)
{
    //return *(volatile MT_U32 *)(p_addr);
    return (MT_U32)HAL_GET_U32((volatile u32*)p_addr);
}

/*!
  register SYMPHONY_DISP_DISPLAY_CTRL (read/write)
  */
void reg_symphony_optm_disp_set_display_ctrl(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL, data);
}

MT_U32  reg_symphony_optm_disp_get_display_ctrl(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL);
}

void reg_symphony_optm_disp_set_display_ctrl_hd_hf_sel(MT_U8 data)
{
    reg_symphony_optm_disp_display_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL;
    d.bitc.hd_hf_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_display_ctrl_hd_hf_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_display_ctrl_t *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL).bitc.hd_hf_sel;
}

void reg_symphony_optm_disp_set_display_ctrl_sd_hf_sel(MT_U8 data)
{
    reg_symphony_optm_disp_display_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL;
    d.bitc.sd_hf_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_display_ctrl_sd_hf_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_display_ctrl_t *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL).bitc.sd_hf_sel;
}

void reg_symphony_optm_disp_set_display_ctrl_hd_cut_en(MT_U8 data)
{
    reg_symphony_optm_disp_display_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL;
    d.bitc.hd_cut_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_display_ctrl_hd_cut_en(void)
{
    return (*(volatile reg_symphony_optm_disp_display_ctrl_t *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL).bitc.hd_cut_en;
}

void reg_symphony_optm_disp_set_display_ctrl_sd_cut_en(MT_U8 data)
{
    reg_symphony_optm_disp_display_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL;
    d.bitc.sd_cut_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_display_ctrl_sd_cut_en(void)
{
    return (*(volatile reg_symphony_optm_disp_display_ctrl_t *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL).bitc.sd_cut_en;
}

void reg_symphony_optm_disp_set_display_ctrl_hd_vf_sel(MT_U8 data)
{
    reg_symphony_optm_disp_display_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL;
    d.bitc.hd_vf_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_display_ctrl_hd_vf_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_display_ctrl_t *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL).bitc.hd_vf_sel;
}

void reg_symphony_optm_disp_set_display_ctrl_sd_vf_sel(MT_U8 data)
{
    reg_symphony_optm_disp_display_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL;
    d.bitc.sd_vf_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_display_ctrl_sd_vf_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_display_ctrl_t *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL).bitc.sd_vf_sel;
}

void reg_symphony_optm_disp_set_display_ctrl_disp_fw_mode(MT_U8 data)
{
    reg_symphony_optm_disp_display_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL;
    d.bitc.disp_fw_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_display_ctrl_disp_fw_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_display_ctrl_t *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL).bitc.disp_fw_mode;
}

void reg_symphony_optm_disp_set_display_ctrl_vf_in_frame(MT_U8 data)
{
    reg_symphony_optm_disp_display_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL;
    d.bitc.vf_in_frame = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_display_ctrl_vf_in_frame(void)
{
    return (*(volatile reg_symphony_optm_disp_display_ctrl_t *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL).bitc.vf_in_frame;
}

void reg_symphony_optm_disp_set_display_ctrl_vid_sel(MT_U8 data)
{
    reg_symphony_optm_disp_display_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL;
    d.bitc.vid_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_display_ctrl_vid_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_display_ctrl_t *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL).bitc.vid_sel;
}

void reg_symphony_optm_disp_set_display_ctrl_reg_latch_en(MT_U8 data)
{
    reg_symphony_optm_disp_display_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL;
    d.bitc.reg_latch_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_display_ctrl_reg_latch_en(void)
{
    return (*(volatile reg_symphony_optm_disp_display_ctrl_t *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL).bitc.reg_latch_en;
}

void reg_symphony_optm_disp_set_display_ctrl_reg_latch_timing(MT_U8 data)
{
    reg_symphony_optm_disp_display_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL;
    d.bitc.reg_latch_timing = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_display_ctrl_reg_latch_timing(void)
{
    return (*(volatile reg_symphony_optm_disp_display_ctrl_t *)REG_SYMPHONY_OPTM_DISP_DISPLAY_CTRL).bitc.reg_latch_timing;
}


/*!
  register SYMPHONY_DISP_VSCALER_RATIO_HD (read/write)
  */
void reg_symphony_optm_disp_set_vscaler_ratio_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_vscaler_ratio_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD);
}

void reg_symphony_optm_disp_set_vscaler_ratio_hd_hratio_int_hd(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_ratio_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD;
    d.bitc.hratio_int_hd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_ratio_hd_hratio_int_hd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_hd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD).bitc.hratio_int_hd;
}

void reg_symphony_optm_disp_set_vscaler_ratio_hd_hratio_fra_hd(MT_U16 data)
{
    reg_symphony_optm_disp_vscaler_ratio_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD;
    d.bitc.hratio_fra_hd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vscaler_ratio_hd_hratio_fra_hd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_hd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD).bitc.hratio_fra_hd;
}

void reg_symphony_optm_disp_set_vscaler_ratio_hd_vratio_int_hd(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_ratio_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD;
    d.bitc.vratio_int_hd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_ratio_hd_vratio_int_hd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_hd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD).bitc.vratio_int_hd;
}

void reg_symphony_optm_disp_set_vscaler_ratio_hd_vratio_fra_hd(MT_U16 data)
{
    reg_symphony_optm_disp_vscaler_ratio_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD;
    d.bitc.vratio_fra_hd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vscaler_ratio_hd_vratio_fra_hd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_hd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_HD).bitc.vratio_fra_hd;
}


/*!
  register SYMPHONY_DISP_VSCALER_RATIO_INIT_HD (read/write)
  */
void reg_symphony_optm_disp_set_vscaler_ratio_init_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_vscaler_ratio_init_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD);
}

void reg_symphony_optm_disp_set_vscaler_ratio_init_hd_top_int_hd(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_ratio_init_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD;
    d.bitc.top_int_hd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_ratio_init_hd_top_int_hd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_init_hd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD).bitc.top_int_hd;
}

void reg_symphony_optm_disp_set_vscaler_ratio_init_hd_top_fra_hd(MT_U16 data)
{
    reg_symphony_optm_disp_vscaler_ratio_init_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD;
    d.bitc.top_fra_hd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vscaler_ratio_init_hd_top_fra_hd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_init_hd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD).bitc.top_fra_hd;
}

void reg_symphony_optm_disp_set_vscaler_ratio_init_hd_bot_int_hd(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_ratio_init_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD;
    d.bitc.bot_int_hd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_ratio_init_hd_bot_int_hd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_init_hd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD).bitc.bot_int_hd;
}

void reg_symphony_optm_disp_set_vscaler_ratio_init_hd_bot_fra_hd(MT_U16 data)
{
    reg_symphony_optm_disp_vscaler_ratio_init_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD;
    d.bitc.bot_fra_hd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vscaler_ratio_init_hd_bot_fra_hd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_init_hd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_HD).bitc.bot_fra_hd;
}


/*!
  register SYMPHONY_DISP_VID_DISP_FIELD (read/write)
  */
void reg_symphony_optm_disp_set_vid_disp_field(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_disp_field(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD);
}

void reg_symphony_optm_disp_set_vid_disp_field_sd_hf_phase(MT_U16 data)
{
    reg_symphony_optm_disp_vid_disp_field_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD;
    d.bitc.sd_hf_phase = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_disp_field_sd_hf_phase(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_disp_field_t *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD).bitc.sd_hf_phase;
}

void reg_symphony_optm_disp_set_vid_disp_field_hd_hf_tapnum(MT_U8 data)
{
    reg_symphony_optm_disp_vid_disp_field_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD;
    d.bitc.hd_hf_tapnum = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_disp_field_hd_hf_tapnum(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_disp_field_t *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD).bitc.hd_hf_tapnum;
}

void reg_symphony_optm_disp_set_vid_disp_field_hd_hf_phase(MT_U16 data)
{
    reg_symphony_optm_disp_vid_disp_field_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD;
    d.bitc.hd_hf_phase = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_disp_field_hd_hf_phase(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_disp_field_t *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD).bitc.hd_hf_phase;
}

void reg_symphony_optm_disp_set_vid_disp_field_vid_field(MT_U8 data)
{
    reg_symphony_optm_disp_vid_disp_field_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD;
    d.bitc.vid_field = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_disp_field_vid_field(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_disp_field_t *)REG_SYMPHONY_OPTM_DISP_VID_DISP_FIELD).bitc.vid_field;
}


/*!
  register SYMPHONY_DISP_VID_WINDOW_CUT_SD (read/write)
  */
void reg_symphony_optm_disp_set_vid_window_cut_sd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_window_cut_sd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD);
}

void reg_symphony_optm_disp_set_vid_window_cut_sd_left_cut(MT_U8 data)
{
    reg_symphony_optm_disp_vid_window_cut_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD;
    d.bitc.left_cut = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_window_cut_sd_left_cut(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_cut_sd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD).bitc.left_cut;
}

void reg_symphony_optm_disp_set_vid_window_cut_sd_right_cut(MT_U8 data)
{
    reg_symphony_optm_disp_vid_window_cut_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD;
    d.bitc.right_cut = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_window_cut_sd_right_cut(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_cut_sd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD).bitc.right_cut;
}

void reg_symphony_optm_disp_set_vid_window_cut_sd_top_cut(MT_U8 data)
{
    reg_symphony_optm_disp_vid_window_cut_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD;
    d.bitc.top_cut = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_window_cut_sd_top_cut(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_cut_sd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD).bitc.top_cut;
}

void reg_symphony_optm_disp_set_vid_window_cut_sd_bot_cut(MT_U8 data)
{
    reg_symphony_optm_disp_vid_window_cut_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD;
    d.bitc.bot_cut = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_window_cut_sd_bot_cut(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_cut_sd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_SD).bitc.bot_cut;
}


/*!
  register SYMPHONY_DISP_VID_WINDOW_X_HD (read/write)
  */
void reg_symphony_optm_disp_set_vid_window_x_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_window_x_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_HD);
}

void reg_symphony_optm_disp_set_vid_window_x_hd_vid_x_left_hd(MT_U16 data)
{
    reg_symphony_optm_disp_vid_window_x_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_HD;
    d.bitc.vid_x_left_hd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_HD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_window_x_hd_vid_x_left_hd(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_x_hd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_HD).bitc.vid_x_left_hd;
}

void reg_symphony_optm_disp_set_vid_window_x_hd_vid_x_right_hd(MT_U16 data)
{
    reg_symphony_optm_disp_vid_window_x_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_HD;
    d.bitc.vid_x_right_hd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_HD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_window_x_hd_vid_x_right_hd(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_x_hd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_HD).bitc.vid_x_right_hd;
}


/*!
  register SYMPHONY_DISP_VID_WINDOW_Y_HD (read/write)
  */
void reg_symphony_optm_disp_set_vid_window_y_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_window_y_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_HD);
}

void reg_symphony_optm_disp_set_vid_window_y_hd_vid_y_start_hd(MT_U16 data)
{
    reg_symphony_optm_disp_vid_window_y_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_HD;
    d.bitc.vid_y_start_hd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_HD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_window_y_hd_vid_y_start_hd(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_y_hd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_HD).bitc.vid_y_start_hd;
}

void reg_symphony_optm_disp_set_vid_window_y_hd_vid_y_end_hd(MT_U16 data)
{
    reg_symphony_optm_disp_vid_window_y_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_HD;
    d.bitc.vid_y_end_hd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_HD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_window_y_hd_vid_y_end_hd(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_y_hd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_HD).bitc.vid_y_end_hd;
}


/*!
  register SYMPHONY_DISP_VID_WINDOW_CUT_HD (read/write)
  */
void reg_symphony_optm_disp_set_vid_window_cut_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_window_cut_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD);
}

void reg_symphony_optm_disp_set_vid_window_cut_hd_left_cut(MT_U8 data)
{
    reg_symphony_optm_disp_vid_window_cut_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD;
    d.bitc.left_cut = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_window_cut_hd_left_cut(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_cut_hd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD).bitc.left_cut;
}

void reg_symphony_optm_disp_set_vid_window_cut_hd_right_cut(MT_U8 data)
{
    reg_symphony_optm_disp_vid_window_cut_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD;
    d.bitc.right_cut = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_window_cut_hd_right_cut(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_cut_hd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD).bitc.right_cut;
}

void reg_symphony_optm_disp_set_vid_window_cut_hd_top_cut(MT_U8 data)
{
    reg_symphony_optm_disp_vid_window_cut_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD;
    d.bitc.top_cut = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_window_cut_hd_top_cut(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_cut_hd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD).bitc.top_cut;
}

void reg_symphony_optm_disp_set_vid_window_cut_hd_bot_cut(MT_U8 data)
{
    reg_symphony_optm_disp_vid_window_cut_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD;
    d.bitc.bot_cut = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_window_cut_hd_bot_cut(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_cut_hd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_CUT_HD).bitc.bot_cut;
}


/*!
  register SYMPHONY_DISP_VSCALER_RATIO_SD (read/write)
  */
void reg_symphony_optm_disp_set_vscaler_ratio_sd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD, data);
}

MT_U32  reg_symphony_optm_disp_get_vscaler_ratio_sd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD);
}

void reg_symphony_optm_disp_set_vscaler_ratio_sd_hratio_int_sd(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_ratio_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD;
    d.bitc.hratio_int_sd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_ratio_sd_hratio_int_sd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_sd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD).bitc.hratio_int_sd;
}

void reg_symphony_optm_disp_set_vscaler_ratio_sd_hratio_fra_sd(MT_U16 data)
{
    reg_symphony_optm_disp_vscaler_ratio_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD;
    d.bitc.hratio_fra_sd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vscaler_ratio_sd_hratio_fra_sd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_sd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD).bitc.hratio_fra_sd;
}

void reg_symphony_optm_disp_set_vscaler_ratio_sd_vratio_int_sd(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_ratio_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD;
    d.bitc.vratio_int_sd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_ratio_sd_vratio_int_sd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_sd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD).bitc.vratio_int_sd;
}

void reg_symphony_optm_disp_set_vscaler_ratio_sd_vratio_fra_sd(MT_U16 data)
{
    reg_symphony_optm_disp_vscaler_ratio_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD;
    d.bitc.vratio_fra_sd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vscaler_ratio_sd_vratio_fra_sd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_sd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_SD).bitc.vratio_fra_sd;
}


/*!
  register SYMPHONY_DISP_VSCALER_RATIO_INIT_SD (read/write)
  */
void reg_symphony_optm_disp_set_vscaler_ratio_init_sd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD, data);
}

MT_U32  reg_symphony_optm_disp_get_vscaler_ratio_init_sd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD);
}

void reg_symphony_optm_disp_set_vscaler_ratio_init_sd_top_int_sd(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_ratio_init_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD;
    d.bitc.top_int_sd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_ratio_init_sd_top_int_sd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_init_sd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD).bitc.top_int_sd;
}

void reg_symphony_optm_disp_set_vscaler_ratio_init_sd_top_fra_sd(MT_U16 data)
{
    reg_symphony_optm_disp_vscaler_ratio_init_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD;
    d.bitc.top_fra_sd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vscaler_ratio_init_sd_top_fra_sd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_init_sd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD).bitc.top_fra_sd;
}

void reg_symphony_optm_disp_set_vscaler_ratio_init_sd_bot_int_sd(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_ratio_init_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD;
    d.bitc.bot_int_sd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_ratio_init_sd_bot_int_sd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_init_sd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD).bitc.bot_int_sd;
}

void reg_symphony_optm_disp_set_vscaler_ratio_init_sd_bot_fra_sd(MT_U16 data)
{
    reg_symphony_optm_disp_vscaler_ratio_init_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD;
    d.bitc.bot_fra_sd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vscaler_ratio_init_sd_bot_fra_sd(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_ratio_init_sd_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_RATIO_INIT_SD).bitc.bot_fra_sd;
}


/*!
  register SYMPHONY_DISP_VID_WINDOW_X_SD (read/write)
  */
void reg_symphony_optm_disp_set_vid_window_x_sd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_SD, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_window_x_sd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_SD);
}

void reg_symphony_optm_disp_set_vid_window_x_sd_vid_x_left_sd(MT_U16 data)
{
    reg_symphony_optm_disp_vid_window_x_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_SD;
    d.bitc.vid_x_left_sd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_SD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_window_x_sd_vid_x_left_sd(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_x_sd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_SD).bitc.vid_x_left_sd;
}

void reg_symphony_optm_disp_set_vid_window_x_sd_vid_x_right_sd(MT_U16 data)
{
    reg_symphony_optm_disp_vid_window_x_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_SD;
    d.bitc.vid_x_right_sd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_SD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_window_x_sd_vid_x_right_sd(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_x_sd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_X_SD).bitc.vid_x_right_sd;
}


/*!
  register SYMPHONY_DISP_VID_WINDOW_Y_SD (read/write)
  */
void reg_symphony_optm_disp_set_vid_window_y_sd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_SD, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_window_y_sd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_SD);
}

void reg_symphony_optm_disp_set_vid_window_y_sd_vid_y_start_sd(MT_U16 data)
{
    reg_symphony_optm_disp_vid_window_y_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_SD;
    d.bitc.vid_y_start_sd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_SD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_window_y_sd_vid_y_start_sd(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_y_sd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_SD).bitc.vid_y_start_sd;
}

void reg_symphony_optm_disp_set_vid_window_y_sd_vid_y_end_sd(MT_U16 data)
{
    reg_symphony_optm_disp_vid_window_y_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_SD;
    d.bitc.vid_y_end_sd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_SD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_window_y_sd_vid_y_end_sd(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_window_y_sd_t *)REG_SYMPHONY_OPTM_DISP_VID_WINDOW_Y_SD).bitc.vid_y_end_sd;
}


/*!
  register SYMPHONY_DISP_GRAPHIC_CTRL (read/write)
  */
void reg_symphony_optm_disp_set_graphic_ctrl(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL, data);
}

MT_U32  reg_symphony_optm_disp_get_graphic_ctrl(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL);
}

void reg_symphony_optm_disp_set_graphic_ctrl_back_sel(MT_U8 data)
{
    reg_symphony_optm_disp_graphic_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL;
    d.bitc.back_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_graphic_ctrl_back_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_graphic_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL).bitc.back_sel;
}

void reg_symphony_optm_disp_set_graphic_ctrl_still_sel_sd(MT_U8 data)
{
    reg_symphony_optm_disp_graphic_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL;
    d.bitc.still_sel_sd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_graphic_ctrl_still_sel_sd(void)
{
    return (*(volatile reg_symphony_optm_disp_graphic_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL).bitc.still_sel_sd;
}

void reg_symphony_optm_disp_set_graphic_ctrl_still_sd_format_444(MT_U8 data)
{
    reg_symphony_optm_disp_graphic_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL;
    d.bitc.still_sd_format_444 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_graphic_ctrl_still_sd_format_444(void)
{
    return (*(volatile reg_symphony_optm_disp_graphic_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL).bitc.still_sd_format_444;
}

void reg_symphony_optm_disp_set_graphic_ctrl_still_sel_hd(MT_U8 data)
{
    reg_symphony_optm_disp_graphic_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL;
    d.bitc.still_sel_hd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_graphic_ctrl_still_sel_hd(void)
{
    return (*(volatile reg_symphony_optm_disp_graphic_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL).bitc.still_sel_hd;
}

void reg_symphony_optm_disp_set_graphic_ctrl_still_hd_format(MT_U8 data)
{
    reg_symphony_optm_disp_graphic_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL;
    d.bitc.still_hd_format = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_graphic_ctrl_still_hd_format(void)
{
    return (*(volatile reg_symphony_optm_disp_graphic_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL).bitc.still_hd_format;
}

void reg_symphony_optm_disp_set_graphic_ctrl_mix_layer_mode(MT_U8 data)
{
    reg_symphony_optm_disp_graphic_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL;
    d.bitc.mix_layer_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_graphic_ctrl_mix_layer_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_graphic_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRAPHIC_CTRL).bitc.mix_layer_mode;
}


/*!
  register SYMPHONY_DISP_BG_COLOR (read/write)
  */
void reg_symphony_optm_disp_set_bg_color(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_BG_COLOR, data);
}

MT_U32  reg_symphony_optm_disp_get_bg_color(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_BG_COLOR);
}

void reg_symphony_optm_disp_set_bg_color_bg_cr(MT_U8 data)
{
    reg_symphony_optm_disp_bg_color_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_BG_COLOR;
    d.bitc.bg_cr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_BG_COLOR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_bg_color_bg_cr(void)
{
    return (*(volatile reg_symphony_optm_disp_bg_color_t *)REG_SYMPHONY_OPTM_DISP_BG_COLOR).bitc.bg_cr;
}

void reg_symphony_optm_disp_set_bg_color_bg_cb(MT_U8 data)
{
    reg_symphony_optm_disp_bg_color_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_BG_COLOR;
    d.bitc.bg_cb = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_BG_COLOR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_bg_color_bg_cb(void)
{
    return (*(volatile reg_symphony_optm_disp_bg_color_t *)REG_SYMPHONY_OPTM_DISP_BG_COLOR).bitc.bg_cb;
}

void reg_symphony_optm_disp_set_bg_color_bg_y(MT_U8 data)
{
    reg_symphony_optm_disp_bg_color_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_BG_COLOR;
    d.bitc.bg_y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_BG_COLOR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_bg_color_bg_y(void)
{
    return (*(volatile reg_symphony_optm_disp_bg_color_t *)REG_SYMPHONY_OPTM_DISP_BG_COLOR).bitc.bg_y;
}


/*!
  register SYMPHONY_DISP_STILL_X_HD (read/write)
  */
void reg_symphony_optm_disp_set_still_x_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_X_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_still_x_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_X_HD);
}

void reg_symphony_optm_disp_set_still_x_hd_hd_still_startx(MT_U16 data)
{
    reg_symphony_optm_disp_still_x_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_X_HD;
    d.bitc.hd_still_startx = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_X_HD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_still_x_hd_hd_still_startx(void)
{
    return (*(volatile reg_symphony_optm_disp_still_x_hd_t *)REG_SYMPHONY_OPTM_DISP_STILL_X_HD).bitc.hd_still_startx;
}

void reg_symphony_optm_disp_set_still_x_hd_hd_still_endx(MT_U16 data)
{
    reg_symphony_optm_disp_still_x_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_X_HD;
    d.bitc.hd_still_endx = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_X_HD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_still_x_hd_hd_still_endx(void)
{
    return (*(volatile reg_symphony_optm_disp_still_x_hd_t *)REG_SYMPHONY_OPTM_DISP_STILL_X_HD).bitc.hd_still_endx;
}


/*!
  register SYMPHONY_DISP_STILL_Y_HD (read/write)
  */
void reg_symphony_optm_disp_set_still_y_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_Y_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_still_y_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_Y_HD);
}

void reg_symphony_optm_disp_set_still_y_hd_hd_still_starty(MT_U16 data)
{
    reg_symphony_optm_disp_still_y_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_Y_HD;
    d.bitc.hd_still_starty = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_Y_HD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_still_y_hd_hd_still_starty(void)
{
    return (*(volatile reg_symphony_optm_disp_still_y_hd_t *)REG_SYMPHONY_OPTM_DISP_STILL_Y_HD).bitc.hd_still_starty;
}

void reg_symphony_optm_disp_set_still_y_hd_hd_still_endy(MT_U16 data)
{
    reg_symphony_optm_disp_still_y_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_Y_HD;
    d.bitc.hd_still_endy = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_Y_HD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_still_y_hd_hd_still_endy(void)
{
    return (*(volatile reg_symphony_optm_disp_still_y_hd_t *)REG_SYMPHONY_OPTM_DISP_STILL_Y_HD).bitc.hd_still_endy;
}


/*!
  register SYMPHONY_DISP_OSD0_CMD_HD (read/write)
  */
void reg_symphony_optm_disp_set_osd0_cmd_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_osd0_cmd_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD);
}

void reg_symphony_optm_disp_set_osd0_cmd_hd_osd0_sel(MT_U8 data)
{
    reg_symphony_optm_disp_osd0_cmd_hd_t d;
//MT_U32 addr = REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD;
    d.bitc.osd0_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd0_cmd_hd_osd0_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_osd0_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD).bitc.osd0_sel;
}

void reg_symphony_optm_disp_set_osd0_cmd_hd_force_progressive(MT_U8 data)
{
    reg_symphony_optm_disp_osd0_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD;
    d.bitc.force_progressive = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd0_cmd_hd_force_progressive(void)
{
    return (*(volatile reg_symphony_optm_disp_osd0_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD).bitc.force_progressive;
}

void reg_symphony_optm_disp_set_osd0_cmd_hd_plane_alpha_en(MT_U8 data)
{
    reg_symphony_optm_disp_osd0_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD;
    d.bitc.plane_alpha_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd0_cmd_hd_plane_alpha_en(void)
{
    return (*(volatile reg_symphony_optm_disp_osd0_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD).bitc.plane_alpha_en;
}

void reg_symphony_optm_disp_set_osd0_cmd_hd_plane_alpha(MT_U8 data)
{
    reg_symphony_optm_disp_osd0_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD;
    d.bitc.plane_alpha = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd0_cmd_hd_plane_alpha(void)
{
    return (*(volatile reg_symphony_optm_disp_osd0_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD).bitc.plane_alpha;
}

void reg_symphony_optm_disp_set_osd0_cmd_hd_disable_hori_process(MT_U8 data)
{
    reg_symphony_optm_disp_osd0_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD;
    d.bitc.disable_hori_process = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd0_cmd_hd_disable_hori_process(void)
{
    return (*(volatile reg_symphony_optm_disp_osd0_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD).bitc.disable_hori_process;
}

void reg_symphony_optm_disp_set_osd0_cmd_hd_osd0_big_endian(MT_U8 data)
{
    reg_symphony_optm_disp_osd0_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD;
    d.bitc.osd0_big_endian = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd0_cmd_hd_osd0_big_endian(void)
{
    return (*(volatile reg_symphony_optm_disp_osd0_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD0_CMD_HD).bitc.osd0_big_endian;
}


/*!
  register SYMPHONY_DISP_SUB_CMD_HD (read/write)
  */
void reg_symphony_optm_disp_set_sub_cmd_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_sub_cmd_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD);
}

void reg_symphony_optm_disp_set_sub_cmd_hd_sub_sel(MT_U8 data)
{
    reg_symphony_optm_disp_sub_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD;
    d.bitc.sub_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sub_cmd_hd_sub_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_sub_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD).bitc.sub_sel;
}

void reg_symphony_optm_disp_set_sub_cmd_hd_force_progressive(MT_U8 data)
{
    reg_symphony_optm_disp_sub_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD;
    d.bitc.force_progressive = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sub_cmd_hd_force_progressive(void)
{
    return (*(volatile reg_symphony_optm_disp_sub_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD).bitc.force_progressive;
}

void reg_symphony_optm_disp_set_sub_cmd_hd_plane_alpha_en(MT_U8 data)
{
    reg_symphony_optm_disp_sub_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD;
    d.bitc.plane_alpha_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sub_cmd_hd_plane_alpha_en(void)
{
    return (*(volatile reg_symphony_optm_disp_sub_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD).bitc.plane_alpha_en;
}

void reg_symphony_optm_disp_set_sub_cmd_hd_plane_alpha(MT_U8 data)
{
    reg_symphony_optm_disp_sub_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD;
    d.bitc.plane_alpha = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sub_cmd_hd_plane_alpha(void)
{
    return (*(volatile reg_symphony_optm_disp_sub_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD).bitc.plane_alpha;
}

void reg_symphony_optm_disp_set_sub_cmd_hd_disable_hori_process(MT_U8 data)
{
    reg_symphony_optm_disp_sub_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD;
    d.bitc.disable_hori_process = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sub_cmd_hd_disable_hori_process(void)
{
    return (*(volatile reg_symphony_optm_disp_sub_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD).bitc.disable_hori_process;
}

void reg_symphony_optm_disp_set_sub_cmd_hd_sub_big_endian(MT_U8 data)
{
    reg_symphony_optm_disp_sub_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD;
    d.bitc.sub_big_endian = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sub_cmd_hd_sub_big_endian(void)
{
    return (*(volatile reg_symphony_optm_disp_sub_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_SUB_CMD_HD).bitc.sub_big_endian;
}


/*!
  register SYMPHONY_DISP_LAYER_ALPHA (read/write)
  */
void reg_symphony_optm_disp_set_layer_alpha(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LAYER_ALPHA, data);
}

MT_U32  reg_symphony_optm_disp_get_layer_alpha(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LAYER_ALPHA);
}

void reg_symphony_optm_disp_set_layer_alpha_vid_plane_alpha(MT_U8 data)
{
    reg_symphony_optm_disp_layer_alpha_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LAYER_ALPHA;
    d.bitc.vid_plane_alpha = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LAYER_ALPHA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_layer_alpha_vid_plane_alpha(void)
{
    return (*(volatile reg_symphony_optm_disp_layer_alpha_t *)REG_SYMPHONY_OPTM_DISP_LAYER_ALPHA).bitc.vid_plane_alpha;
}

void reg_symphony_optm_disp_set_layer_alpha_still_plane_alpha(MT_U8 data)
{
    reg_symphony_optm_disp_layer_alpha_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LAYER_ALPHA;
    d.bitc.still_plane_alpha = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LAYER_ALPHA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_layer_alpha_still_plane_alpha(void)
{
    return (*(volatile reg_symphony_optm_disp_layer_alpha_t *)REG_SYMPHONY_OPTM_DISP_LAYER_ALPHA).bitc.still_plane_alpha;
}


/*!
  register SYMPHONY_DISP_VID_INPUT_SIZE (read/write)
  */
void reg_symphony_optm_disp_set_vid_input_size(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_INPUT_SIZE, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_input_size(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_INPUT_SIZE);
}

void reg_symphony_optm_disp_set_vid_input_size_vid_frame_h(MT_U16 data)
{
    reg_symphony_optm_disp_vid_input_size_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_INPUT_SIZE;
    d.bitc.vid_frame_h = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_INPUT_SIZE, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_input_size_vid_frame_h(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_input_size_t *)REG_SYMPHONY_OPTM_DISP_VID_INPUT_SIZE).bitc.vid_frame_h;
}

void reg_symphony_optm_disp_set_vid_input_size_vid_frame_w(MT_U16 data)
{
    reg_symphony_optm_disp_vid_input_size_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_INPUT_SIZE;
    d.bitc.vid_frame_w = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_INPUT_SIZE, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_input_size_vid_frame_w(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_input_size_t *)REG_SYMPHONY_OPTM_DISP_VID_INPUT_SIZE).bitc.vid_frame_w;
}


/*!
  register SYMPHONY_DISP_VID_SD_DROP_LINE (read/write)
  */
void reg_symphony_optm_disp_set_vid_sd_drop_line(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_SD_DROP_LINE, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_sd_drop_line(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_SD_DROP_LINE);
}

void reg_symphony_optm_disp_set_vid_sd_drop_line_sd_top_field_drop(MT_U8 data)
{
    reg_symphony_optm_disp_vid_sd_drop_line_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_SD_DROP_LINE;
    d.bitc.sd_top_field_drop = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_SD_DROP_LINE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_sd_drop_line_sd_top_field_drop(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_sd_drop_line_t *)REG_SYMPHONY_OPTM_DISP_VID_SD_DROP_LINE).bitc.sd_top_field_drop;
}

void reg_symphony_optm_disp_set_vid_sd_drop_line_sd_bot_field_drop(MT_U8 data)
{
    reg_symphony_optm_disp_vid_sd_drop_line_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_SD_DROP_LINE;
    d.bitc.sd_bot_field_drop = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_SD_DROP_LINE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_sd_drop_line_sd_bot_field_drop(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_sd_drop_line_t *)REG_SYMPHONY_OPTM_DISP_VID_SD_DROP_LINE).bitc.sd_bot_field_drop;
}


/*!
  register SYMPHONY_DISP_VID_CROP_MODE_EN (read/write)
  */
void reg_symphony_optm_disp_set_vid_crop_mode_en(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_CROP_MODE_EN, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_crop_mode_en(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_CROP_MODE_EN);
}

void reg_symphony_optm_disp_set_vid_crop_mode_en_vid_crop_en(MT_U8 data)
{
    reg_symphony_optm_disp_vid_crop_mode_en_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_CROP_MODE_EN;
    d.bitc.vid_crop_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_CROP_MODE_EN, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_crop_mode_en_vid_crop_en(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_crop_mode_en_t *)REG_SYMPHONY_OPTM_DISP_VID_CROP_MODE_EN).bitc.vid_crop_en;
}


/*!
  register SYMPHONY_DISP_VID_CROP_HORI (read/write)
  */
void reg_symphony_optm_disp_set_vid_crop_hori(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_CROP_HORI, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_crop_hori(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_CROP_HORI);
}

void reg_symphony_optm_disp_set_vid_crop_hori_vid_crop_endx(MT_U16 data)
{
    reg_symphony_optm_disp_vid_crop_hori_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_CROP_HORI;
    d.bitc.vid_crop_endx = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_CROP_HORI, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_crop_hori_vid_crop_endx(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_crop_hori_t *)REG_SYMPHONY_OPTM_DISP_VID_CROP_HORI).bitc.vid_crop_endx;
}

void reg_symphony_optm_disp_set_vid_crop_hori_vid_crop_startx(MT_U16 data)
{
    reg_symphony_optm_disp_vid_crop_hori_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_CROP_HORI;
    d.bitc.vid_crop_startx = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_CROP_HORI, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_crop_hori_vid_crop_startx(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_crop_hori_t *)REG_SYMPHONY_OPTM_DISP_VID_CROP_HORI).bitc.vid_crop_startx;
}


/*!
  register SYMPHONY_DISP_VID_CROP_VERT (read/write)
  */
void reg_symphony_optm_disp_set_vid_crop_vert(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_CROP_VERT, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_crop_vert(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_CROP_VERT);
}

void reg_symphony_optm_disp_set_vid_crop_vert_vid_crop_endy(MT_U16 data)
{
    reg_symphony_optm_disp_vid_crop_vert_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_CROP_VERT;
    d.bitc.vid_crop_endy = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_CROP_VERT, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_crop_vert_vid_crop_endy(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_crop_vert_t *)REG_SYMPHONY_OPTM_DISP_VID_CROP_VERT).bitc.vid_crop_endy;
}

void reg_symphony_optm_disp_set_vid_crop_vert_vid_crop_starty(MT_U16 data)
{
    reg_symphony_optm_disp_vid_crop_vert_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_CROP_VERT;
    d.bitc.vid_crop_starty = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_CROP_VERT, d.all);
}

MT_U16  reg_symphony_optm_disp_get_vid_crop_vert_vid_crop_starty(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_crop_vert_t *)REG_SYMPHONY_OPTM_DISP_VID_CROP_VERT).bitc.vid_crop_starty;
}


/*!
  register SYMPHONY_DISP_VID_PROCESS_MODE (read/write)
  */
void reg_symphony_optm_disp_set_vid_process_mode(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_PROCESS_MODE, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_process_mode(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_PROCESS_MODE);
}

void reg_symphony_optm_disp_set_vid_process_mode_di_process(MT_U8 data)
{
    reg_symphony_optm_disp_vid_process_mode_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_PROCESS_MODE;
    d.bitc.di_process = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_PROCESS_MODE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_process_mode_di_process(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_process_mode_t *)REG_SYMPHONY_OPTM_DISP_VID_PROCESS_MODE).bitc.di_process;
}


/*!
  register SYMPHONY_DISP_OSD1_CMD_HD (read/write)
  */
void reg_symphony_optm_disp_set_osd1_cmd_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_osd1_cmd_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD);
}

void reg_symphony_optm_disp_set_osd1_cmd_hd_osd1_sel(MT_U8 data)
{
    reg_symphony_optm_disp_osd1_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD;
    d.bitc.osd1_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd1_cmd_hd_osd1_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_osd1_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD).bitc.osd1_sel;
}

void reg_symphony_optm_disp_set_osd1_cmd_hd_force_progressive(MT_U8 data)
{
    reg_symphony_optm_disp_osd1_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD;
    d.bitc.force_progressive = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd1_cmd_hd_force_progressive(void)
{
    return (*(volatile reg_symphony_optm_disp_osd1_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD).bitc.force_progressive;
}

void reg_symphony_optm_disp_set_osd1_cmd_hd_plane_alpha_en(MT_U8 data)
{
    reg_symphony_optm_disp_osd1_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD;
    d.bitc.plane_alpha_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd1_cmd_hd_plane_alpha_en(void)
{
    return (*(volatile reg_symphony_optm_disp_osd1_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD).bitc.plane_alpha_en;
}

void reg_symphony_optm_disp_set_osd1_cmd_hd_plane_alpha(MT_U8 data)
{
    reg_symphony_optm_disp_osd1_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD;
    d.bitc.plane_alpha = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd1_cmd_hd_plane_alpha(void)
{
    return (*(volatile reg_symphony_optm_disp_osd1_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD).bitc.plane_alpha;
}

void reg_symphony_optm_disp_set_osd1_cmd_hd_disable_hori_process(MT_U8 data)
{
    reg_symphony_optm_disp_osd1_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD;
    d.bitc.disable_hori_process = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd1_cmd_hd_disable_hori_process(void)
{
    return (*(volatile reg_symphony_optm_disp_osd1_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD).bitc.disable_hori_process;
}

void reg_symphony_optm_disp_set_osd1_cmd_hd_osd1_big_endian(MT_U8 data)
{
    reg_symphony_optm_disp_osd1_cmd_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD;
    d.bitc.osd1_big_endian = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd1_cmd_hd_osd1_big_endian(void)
{
    return (*(volatile reg_symphony_optm_disp_osd1_cmd_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD1_CMD_HD).bitc.osd1_big_endian;
}


/*!
  register SYMPHONY_DISP_OSD1_CK_HD (read/write)
  */
void reg_symphony_optm_disp_set_osd1_ck_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD1_CK_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_osd1_ck_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD1_CK_HD);
}

void reg_symphony_optm_disp_set_osd1_ck_hd_ck_yuv(MT_U32 data)
{
    reg_symphony_optm_disp_osd1_ck_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD1_CK_HD;
    d.bitc.ck_yuv = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD1_CK_HD, d.all);
}

MT_U32  reg_symphony_optm_disp_get_osd1_ck_hd_ck_yuv(void)
{
    return (*(volatile reg_symphony_optm_disp_osd1_ck_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD1_CK_HD).bitc.ck_yuv;
}

void reg_symphony_optm_disp_set_osd1_ck_hd_ck_en(MT_U8 data)
{
    reg_symphony_optm_disp_osd1_ck_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD1_CK_HD;
    d.bitc.ck_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD1_CK_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd1_ck_hd_ck_en(void)
{
    return (*(volatile reg_symphony_optm_disp_osd1_ck_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD1_CK_HD).bitc.ck_en;
}


/*!
  register SYMPHONY_DISP_STILL_X_SD (read/write)
  */
void reg_symphony_optm_disp_set_still_x_sd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_X_SD, data);
}

MT_U32  reg_symphony_optm_disp_get_still_x_sd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_X_SD);
}

void reg_symphony_optm_disp_set_still_x_sd_sd_still_startx(MT_U16 data)
{
    reg_symphony_optm_disp_still_x_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_X_SD;
    d.bitc.sd_still_startx = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_X_SD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_still_x_sd_sd_still_startx(void)
{
    return (*(volatile reg_symphony_optm_disp_still_x_sd_t *)REG_SYMPHONY_OPTM_DISP_STILL_X_SD).bitc.sd_still_startx;
}

void reg_symphony_optm_disp_set_still_x_sd_sd_still_endx(MT_U16 data)
{
    reg_symphony_optm_disp_still_x_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_X_SD;
    d.bitc.sd_still_endx = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_X_SD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_still_x_sd_sd_still_endx(void)
{
    return (*(volatile reg_symphony_optm_disp_still_x_sd_t *)REG_SYMPHONY_OPTM_DISP_STILL_X_SD).bitc.sd_still_endx;
}


/*!
  register SYMPHONY_DISP_STILL_Y_SD (read/write)
  */
void reg_symphony_optm_disp_set_still_y_sd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_Y_SD, data);
}

MT_U32  reg_symphony_optm_disp_get_still_y_sd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_Y_SD);
}

void reg_symphony_optm_disp_set_still_y_sd_sd_still_starty(MT_U16 data)
{
    reg_symphony_optm_disp_still_y_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_Y_SD;
    d.bitc.sd_still_starty = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_Y_SD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_still_y_sd_sd_still_starty(void)
{
    return (*(volatile reg_symphony_optm_disp_still_y_sd_t *)REG_SYMPHONY_OPTM_DISP_STILL_Y_SD).bitc.sd_still_starty;
}

void reg_symphony_optm_disp_set_still_y_sd_sd_still_endy(MT_U16 data)
{
    reg_symphony_optm_disp_still_y_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_Y_SD;
    d.bitc.sd_still_endy = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_Y_SD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_still_y_sd_sd_still_endy(void)
{
    return (*(volatile reg_symphony_optm_disp_still_y_sd_t *)REG_SYMPHONY_OPTM_DISP_STILL_Y_SD).bitc.sd_still_endy;
}


/*!
  register SYMPHONY_DISP_STILL_STRIDE_SD (read/write)
  */
void reg_symphony_optm_disp_set_still_stride_sd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD, data);
}

MT_U32  reg_symphony_optm_disp_get_still_stride_sd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD);
}

void reg_symphony_optm_disp_set_still_stride_sd_sd_still_stride(MT_U16 data)
{
    reg_symphony_optm_disp_still_stride_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD;
    d.bitc.sd_still_stride = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_still_stride_sd_sd_still_stride(void)
{
    return (*(volatile reg_symphony_optm_disp_still_stride_sd_t *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD).bitc.sd_still_stride;
}

void reg_symphony_optm_disp_set_still_stride_sd_cr_byte_sel(MT_U8 data)
{
    reg_symphony_optm_disp_still_stride_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD;
    d.bitc.cr_byte_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_still_stride_sd_cr_byte_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_still_stride_sd_t *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD).bitc.cr_byte_sel;
}

void reg_symphony_optm_disp_set_still_stride_sd_cb_byte_sel(MT_U8 data)
{
    reg_symphony_optm_disp_still_stride_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD;
    d.bitc.cb_byte_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_still_stride_sd_cb_byte_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_still_stride_sd_t *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD).bitc.cb_byte_sel;
}

void reg_symphony_optm_disp_set_still_stride_sd_y_byte_sel(MT_U8 data)
{
    reg_symphony_optm_disp_still_stride_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD;
    d.bitc.y_byte_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_still_stride_sd_y_byte_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_still_stride_sd_t *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_SD).bitc.y_byte_sel;
}


/*!
  register SYMPHONY_DISP_RGB2Y_COEF (read/write)
  */
void reg_symphony_optm_disp_set_rgb2y_coef(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2Y_COEF, data);
}

MT_U32  reg_symphony_optm_disp_get_rgb2y_coef(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2Y_COEF);
}

void reg_symphony_optm_disp_set_rgb2y_coef_r2y_coef(MT_U16 data)
{
    reg_symphony_optm_disp_rgb2y_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2Y_COEF;
    d.bitc.r2y_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2Y_COEF, d.all);
}

MT_U16  reg_symphony_optm_disp_get_rgb2y_coef_r2y_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_rgb2y_coef_t *)REG_SYMPHONY_OPTM_DISP_RGB2Y_COEF).bitc.r2y_coef;
}

void reg_symphony_optm_disp_set_rgb2y_coef_g2y_coef(MT_U16 data)
{
    reg_symphony_optm_disp_rgb2y_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2Y_COEF;
    d.bitc.g2y_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2Y_COEF, d.all);
}

MT_U16  reg_symphony_optm_disp_get_rgb2y_coef_g2y_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_rgb2y_coef_t *)REG_SYMPHONY_OPTM_DISP_RGB2Y_COEF).bitc.g2y_coef;
}

void reg_symphony_optm_disp_set_rgb2y_coef_b2y_coef(MT_U8 data)
{
    reg_symphony_optm_disp_rgb2y_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2Y_COEF;
    d.bitc.b2y_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2Y_COEF, d.all);
}

MT_U8   reg_symphony_optm_disp_get_rgb2y_coef_b2y_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_rgb2y_coef_t *)REG_SYMPHONY_OPTM_DISP_RGB2Y_COEF).bitc.b2y_coef;
}


/*!
  register SYMPHONY_DISP_RGB2CB_COEF (read/write)
  */
void reg_symphony_optm_disp_set_rgb2cb_coef(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2CB_COEF, data);
}

MT_U32  reg_symphony_optm_disp_get_rgb2cb_coef(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2CB_COEF);
}

void reg_symphony_optm_disp_set_rgb2cb_coef_b2cb_coef(MT_U16 data)
{
    reg_symphony_optm_disp_rgb2cb_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2CB_COEF;
    d.bitc.b2cb_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2CB_COEF, d.all);
}

MT_U16  reg_symphony_optm_disp_get_rgb2cb_coef_b2cb_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_rgb2cb_coef_t *)REG_SYMPHONY_OPTM_DISP_RGB2CB_COEF).bitc.b2cb_coef;
}

void reg_symphony_optm_disp_set_rgb2cb_coef_g2cb_coef(MT_U16 data)
{
    reg_symphony_optm_disp_rgb2cb_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2CB_COEF;
    d.bitc.g2cb_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2CB_COEF, d.all);
}

MT_U16  reg_symphony_optm_disp_get_rgb2cb_coef_g2cb_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_rgb2cb_coef_t *)REG_SYMPHONY_OPTM_DISP_RGB2CB_COEF).bitc.g2cb_coef;
}

void reg_symphony_optm_disp_set_rgb2cb_coef_r2cb_coef(MT_U8 data)
{
    reg_symphony_optm_disp_rgb2cb_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2CB_COEF;
    d.bitc.r2cb_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2CB_COEF, d.all);
}

MT_U8   reg_symphony_optm_disp_get_rgb2cb_coef_r2cb_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_rgb2cb_coef_t *)REG_SYMPHONY_OPTM_DISP_RGB2CB_COEF).bitc.r2cb_coef;
}


/*!
  register SYMPHONY_DISP_RGB2CR_COEF (read/write)
  */
void reg_symphony_optm_disp_set_rgb2cr_coef(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2CR_COEF, data);
}

MT_U32  reg_symphony_optm_disp_get_rgb2cr_coef(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2CR_COEF);
}

void reg_symphony_optm_disp_set_rgb2cr_coef_r2cr_coef(MT_U16 data)
{
    reg_symphony_optm_disp_rgb2cr_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2CR_COEF;
    d.bitc.r2cr_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2CR_COEF, d.all);
}

MT_U16  reg_symphony_optm_disp_get_rgb2cr_coef_r2cr_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_rgb2cr_coef_t *)REG_SYMPHONY_OPTM_DISP_RGB2CR_COEF).bitc.r2cr_coef;
}

void reg_symphony_optm_disp_set_rgb2cr_coef_g2cr_coef(MT_U16 data)
{
    reg_symphony_optm_disp_rgb2cr_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2CR_COEF;
    d.bitc.g2cr_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2CR_COEF, d.all);
}

MT_U16  reg_symphony_optm_disp_get_rgb2cr_coef_g2cr_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_rgb2cr_coef_t *)REG_SYMPHONY_OPTM_DISP_RGB2CR_COEF).bitc.g2cr_coef;
}

void reg_symphony_optm_disp_set_rgb2cr_coef_b2cr_coef(MT_U8 data)
{
    reg_symphony_optm_disp_rgb2cr_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2CR_COEF;
    d.bitc.b2cr_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2CR_COEF, d.all);
}

MT_U8   reg_symphony_optm_disp_get_rgb2cr_coef_b2cr_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_rgb2cr_coef_t *)REG_SYMPHONY_OPTM_DISP_RGB2CR_COEF).bitc.b2cr_coef;
}


/*!
  register SYMPHONY_DISP_VID_DECOMP_CFG (read/write)
  */
void reg_symphony_optm_disp_set_vid_decomp_cfg(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_DECOMP_CFG, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_decomp_cfg(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_DECOMP_CFG);
}

void reg_symphony_optm_disp_set_vid_decomp_cfg_config(MT_U32 data)
{
    reg_symphony_optm_disp_vid_decomp_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_DECOMP_CFG;
    d.bitc.config = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_DECOMP_CFG, d.all);
}

MT_U32  reg_symphony_optm_disp_get_vid_decomp_cfg_config(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_decomp_cfg_t *)REG_SYMPHONY_OPTM_DISP_VID_DECOMP_CFG).bitc.config;
}


/*!
  register SYMPHONY_DISP_OSD0_CK_HD (read/write)
  */
void reg_symphony_optm_disp_set_osd0_ck_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD0_CK_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_osd0_ck_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD0_CK_HD);
}

void reg_symphony_optm_disp_set_osd0_ck_hd_ck_yuv(MT_U32 data)
{
    reg_symphony_optm_disp_osd0_ck_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD0_CK_HD;
    d.bitc.ck_yuv = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD0_CK_HD, d.all);
}

MT_U32  reg_symphony_optm_disp_get_osd0_ck_hd_ck_yuv(void)
{
    return (*(volatile reg_symphony_optm_disp_osd0_ck_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD0_CK_HD).bitc.ck_yuv;
}

void reg_symphony_optm_disp_set_osd0_ck_hd_ck_en(MT_U8 data)
{
    reg_symphony_optm_disp_osd0_ck_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD0_CK_HD;
    d.bitc.ck_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD0_CK_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd0_ck_hd_ck_en(void)
{
    return (*(volatile reg_symphony_optm_disp_osd0_ck_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD0_CK_HD).bitc.ck_en;
}


/*!
  register SYMPHONY_DISP_RGB2YUV_YOFFSET (read/write)
  */
void reg_symphony_optm_disp_set_rgb2yuv_yoffset(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2YUV_YOFFSET, data);
}

MT_U32  reg_symphony_optm_disp_get_rgb2yuv_yoffset(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2YUV_YOFFSET);
}

void reg_symphony_optm_disp_set_rgb2yuv_yoffset_yoffset(MT_U32 data)
{
    reg_symphony_optm_disp_rgb2yuv_yoffset_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2YUV_YOFFSET;
    d.bitc.yoffset = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2YUV_YOFFSET, d.all);
}

MT_U32  reg_symphony_optm_disp_get_rgb2yuv_yoffset_yoffset(void)
{
    return (*(volatile reg_symphony_optm_disp_rgb2yuv_yoffset_t *)REG_SYMPHONY_OPTM_DISP_RGB2YUV_YOFFSET).bitc.yoffset;
}


/*!
  register SYMPHONY_DISP_RGB2YUV_UVOFFSET (read/write)
  */
void reg_symphony_optm_disp_set_rgb2yuv_uvoffset(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2YUV_UVOFFSET, data);
}

MT_U32  reg_symphony_optm_disp_get_rgb2yuv_uvoffset(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2YUV_UVOFFSET);
}

void reg_symphony_optm_disp_set_rgb2yuv_uvoffset_uvoffset(MT_U32 data)
{
    reg_symphony_optm_disp_rgb2yuv_uvoffset_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_RGB2YUV_UVOFFSET;
    d.bitc.uvoffset = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_RGB2YUV_UVOFFSET, d.all);
}

MT_U32  reg_symphony_optm_disp_get_rgb2yuv_uvoffset_uvoffset(void)
{
    return (*(volatile reg_symphony_optm_disp_rgb2yuv_uvoffset_t *)REG_SYMPHONY_OPTM_DISP_RGB2YUV_UVOFFSET).bitc.uvoffset;
}


/*!
  register SYMPHONY_DISP_VID_VERF_CFG (read/write)
  */
void reg_symphony_optm_disp_set_vid_verf_cfg(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_VERF_CFG, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_verf_cfg(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_VERF_CFG);
}

void reg_symphony_optm_disp_set_vid_verf_cfg_hd_dce_en(MT_U8 data)
{
    reg_symphony_optm_disp_vid_verf_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_VERF_CFG;
    d.bitc.hd_dce_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_VERF_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_verf_cfg_hd_dce_en(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_verf_cfg_t *)REG_SYMPHONY_OPTM_DISP_VID_VERF_CFG).bitc.hd_dce_en;
}

void reg_symphony_optm_disp_set_vid_verf_cfg_sd_dce_en(MT_U8 data)
{
    reg_symphony_optm_disp_vid_verf_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_VERF_CFG;
    d.bitc.sd_dce_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_VERF_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_verf_cfg_sd_dce_en(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_verf_cfg_t *)REG_SYMPHONY_OPTM_DISP_VID_VERF_CFG).bitc.sd_dce_en;
}


/*!
  register SYMPHONY_DISP_VID_HORF_CFG (read/write)
  */
void reg_symphony_optm_disp_set_vid_horf_cfg(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_HORF_CFG, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_horf_cfg(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_HORF_CFG);
}

void reg_symphony_optm_disp_set_vid_horf_cfg_sd_infl_thr(MT_U8 data)
{
    reg_symphony_optm_disp_vid_horf_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_HORF_CFG;
    d.bitc.sd_infl_thr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_HORF_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_horf_cfg_sd_infl_thr(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_horf_cfg_t *)REG_SYMPHONY_OPTM_DISP_VID_HORF_CFG).bitc.sd_infl_thr;
}

void reg_symphony_optm_disp_set_vid_horf_cfg_hd_infl_thr(MT_U8 data)
{
    reg_symphony_optm_disp_vid_horf_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_HORF_CFG;
    d.bitc.hd_infl_thr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_HORF_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vid_horf_cfg_hd_infl_thr(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_horf_cfg_t *)REG_SYMPHONY_OPTM_DISP_VID_HORF_CFG).bitc.hd_infl_thr;
}


/*!
  register SYMPHONY_DISP_LAYER_MIX_CFG (read/write)
  */
void reg_symphony_optm_disp_set_layer_mix_cfg(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LAYER_MIX_CFG, data);
}

MT_U32  reg_symphony_optm_disp_get_layer_mix_cfg(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LAYER_MIX_CFG);
}

void reg_symphony_optm_disp_set_layer_mix_cfg_bypass_sd_vid_scaler(MT_U8 data)
{
    reg_symphony_optm_disp_layer_mix_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LAYER_MIX_CFG;
    d.bitc.bypass_sd_vid_scaler = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LAYER_MIX_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_layer_mix_cfg_bypass_sd_vid_scaler(void)
{
    return (*(volatile reg_symphony_optm_disp_layer_mix_cfg_t *)REG_SYMPHONY_OPTM_DISP_LAYER_MIX_CFG).bitc.bypass_sd_vid_scaler;
}

void reg_symphony_optm_disp_set_layer_mix_cfg_sd_yuv444(MT_U8 data)
{
    reg_symphony_optm_disp_layer_mix_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LAYER_MIX_CFG;
    d.bitc.sd_yuv444 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LAYER_MIX_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_layer_mix_cfg_sd_yuv444(void)
{
    return (*(volatile reg_symphony_optm_disp_layer_mix_cfg_t *)REG_SYMPHONY_OPTM_DISP_LAYER_MIX_CFG).bitc.sd_yuv444;
}

void reg_symphony_optm_disp_set_layer_mix_cfg_sd_wr_back_forbidden(MT_U8 data)
{
    reg_symphony_optm_disp_layer_mix_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LAYER_MIX_CFG;
    d.bitc.sd_wr_back_forbidden = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LAYER_MIX_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_layer_mix_cfg_sd_wr_back_forbidden(void)
{
    return (*(volatile reg_symphony_optm_disp_layer_mix_cfg_t *)REG_SYMPHONY_OPTM_DISP_LAYER_MIX_CFG).bitc.sd_wr_back_forbidden;
}


/*!
  register SYMPHONY_DISP_HD_SIZE_OUT (read/write)
  */
void reg_symphony_optm_disp_set_hd_size_out(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_SIZE_OUT, data);
}

MT_U32  reg_symphony_optm_disp_get_hd_size_out(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_SIZE_OUT);
}

void reg_symphony_optm_disp_set_hd_size_out_hd_width_out(MT_U16 data)
{
    reg_symphony_optm_disp_hd_size_out_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_SIZE_OUT;
    d.bitc.hd_width_out = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_SIZE_OUT, d.all);
}

MT_U16  reg_symphony_optm_disp_get_hd_size_out_hd_width_out(void)
{
    return (*(volatile reg_symphony_optm_disp_hd_size_out_t *)REG_SYMPHONY_OPTM_DISP_HD_SIZE_OUT).bitc.hd_width_out;
}

void reg_symphony_optm_disp_set_hd_size_out_hd_height_out(MT_U16 data)
{
    reg_symphony_optm_disp_hd_size_out_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_SIZE_OUT;
    d.bitc.hd_height_out = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_SIZE_OUT, d.all);
}

MT_U16  reg_symphony_optm_disp_get_hd_size_out_hd_height_out(void)
{
    return (*(volatile reg_symphony_optm_disp_hd_size_out_t *)REG_SYMPHONY_OPTM_DISP_HD_SIZE_OUT).bitc.hd_height_out;
}


/*!
  register SYMPHONY_DISP_SD_SIZE_OUT (read/write)
  */
void reg_symphony_optm_disp_set_sd_size_out(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_SIZE_OUT, data);
}

MT_U32  reg_symphony_optm_disp_get_sd_size_out(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_SIZE_OUT);
}

void reg_symphony_optm_disp_set_sd_size_out_sd_width_out(MT_U16 data)
{
    reg_symphony_optm_disp_sd_size_out_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_SIZE_OUT;
    d.bitc.sd_width_out = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_SIZE_OUT, d.all);
}

MT_U16  reg_symphony_optm_disp_get_sd_size_out_sd_width_out(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_size_out_t *)REG_SYMPHONY_OPTM_DISP_SD_SIZE_OUT).bitc.sd_width_out;
}

void reg_symphony_optm_disp_set_sd_size_out_sd_height_out(MT_U16 data)
{
    reg_symphony_optm_disp_sd_size_out_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_SIZE_OUT;
    d.bitc.sd_height_out = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_SIZE_OUT, d.all);
}

MT_U16  reg_symphony_optm_disp_get_sd_size_out_sd_height_out(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_size_out_t *)REG_SYMPHONY_OPTM_DISP_SD_SIZE_OUT).bitc.sd_height_out;
}


/*!
  register SYMPHONY_DISP_STILL_STRIDE_HD (read/write)
  */
void reg_symphony_optm_disp_set_still_stride_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_still_stride_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD);
}

void reg_symphony_optm_disp_set_still_stride_hd_hd_still_stride(MT_U16 data)
{
    reg_symphony_optm_disp_still_stride_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD;
    d.bitc.hd_still_stride = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD, d.all);
}

MT_U16  reg_symphony_optm_disp_get_still_stride_hd_hd_still_stride(void)
{
    return (*(volatile reg_symphony_optm_disp_still_stride_hd_t *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD).bitc.hd_still_stride;
}

void reg_symphony_optm_disp_set_still_stride_hd_cr_byte_sel(MT_U8 data)
{
    reg_symphony_optm_disp_still_stride_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD;
    d.bitc.cr_byte_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_still_stride_hd_cr_byte_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_still_stride_hd_t *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD).bitc.cr_byte_sel;
}

void reg_symphony_optm_disp_set_still_stride_hd_cb_byte_sel(MT_U8 data)
{
    reg_symphony_optm_disp_still_stride_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD;
    d.bitc.cb_byte_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_still_stride_hd_cb_byte_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_still_stride_hd_t *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD).bitc.cb_byte_sel;
}

void reg_symphony_optm_disp_set_still_stride_hd_y_byte_sel(MT_U8 data)
{
    reg_symphony_optm_disp_still_stride_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD;
    d.bitc.y_byte_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_still_stride_hd_y_byte_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_still_stride_hd_t *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD).bitc.y_byte_sel;
}

void reg_symphony_optm_disp_set_still_stride_hd_cr_first(MT_U8 data)
{
    reg_symphony_optm_disp_still_stride_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD;
    d.bitc.cr_first = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_still_stride_hd_cr_first(void)
{
    return (*(volatile reg_symphony_optm_disp_still_stride_hd_t *)REG_SYMPHONY_OPTM_DISP_STILL_STRIDE_HD).bitc.cr_first;
}


/*!
  register SYMPHONY_DISP_HDTV_CFG (read/write)
  */
void reg_symphony_optm_disp_set_hdtv_cfg(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HDTV_CFG, data);
}

MT_U32  reg_symphony_optm_disp_get_hdtv_cfg(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HDTV_CFG);
}

void reg_symphony_optm_disp_set_hdtv_cfg_hd_vid_fmt(MT_U8 data)
{
    reg_symphony_optm_disp_hdtv_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HDTV_CFG;
    d.bitc.hd_vid_fmt = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HDTV_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_hdtv_cfg_hd_vid_fmt(void)
{
    return (*(volatile reg_symphony_optm_disp_hdtv_cfg_t *)REG_SYMPHONY_OPTM_DISP_HDTV_CFG).bitc.hd_vid_fmt;
}


/*!
  register SYMPHONY_DISP_HD_POST_CFG (read/write)
  */
void reg_symphony_optm_disp_set_hd_post_cfg(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG, data);
}

MT_U32  reg_symphony_optm_disp_get_hd_post_cfg(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG);
}

void reg_symphony_optm_disp_set_hd_post_cfg_hd_leverage(MT_U8 data)
{
    reg_symphony_optm_disp_hd_post_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG;
    d.bitc.hd_leverage = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_hd_post_cfg_hd_leverage(void)
{
    return (*(volatile reg_symphony_optm_disp_hd_post_cfg_t *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG).bitc.hd_leverage;
}

void reg_symphony_optm_disp_set_hd_post_cfg_hd_hp_enha(MT_U8 data)
{
    reg_symphony_optm_disp_hd_post_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG;
    d.bitc.hd_hp_enha = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_hd_post_cfg_hd_hp_enha(void)
{
    return (*(volatile reg_symphony_optm_disp_hd_post_cfg_t *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG).bitc.hd_hp_enha;
}

void reg_symphony_optm_disp_set_hd_post_cfg_hd_hori_enha(MT_U8 data)
{
    reg_symphony_optm_disp_hd_post_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG;
    d.bitc.hd_hori_enha = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_hd_post_cfg_hd_hori_enha(void)
{
    return (*(volatile reg_symphony_optm_disp_hd_post_cfg_t *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG).bitc.hd_hori_enha;
}

void reg_symphony_optm_disp_set_hd_post_cfg_hd_shoot_chg(MT_U8 data)
{
    reg_symphony_optm_disp_hd_post_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG;
    d.bitc.hd_shoot_chg = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_hd_post_cfg_hd_shoot_chg(void)
{
    return (*(volatile reg_symphony_optm_disp_hd_post_cfg_t *)REG_SYMPHONY_OPTM_DISP_HD_POST_CFG).bitc.hd_shoot_chg;
}


/*!
  register SYMPHONY_DISP_SD_POST_CFG (read/write)
  */
void reg_symphony_optm_disp_set_sd_post_cfg(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG, data);
}

MT_U32  reg_symphony_optm_disp_get_sd_post_cfg(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG);
}

void reg_symphony_optm_disp_set_sd_post_cfg_sd_leverage(MT_U8 data)
{
    reg_symphony_optm_disp_sd_post_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG;
    d.bitc.sd_leverage = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_post_cfg_sd_leverage(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_post_cfg_t *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG).bitc.sd_leverage;
}

void reg_symphony_optm_disp_set_sd_post_cfg_sd_hp_enha(MT_U8 data)
{
    reg_symphony_optm_disp_sd_post_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG;
    d.bitc.sd_hp_enha = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_post_cfg_sd_hp_enha(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_post_cfg_t *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG).bitc.sd_hp_enha;
}

void reg_symphony_optm_disp_set_sd_post_cfg_sd_hori_enha(MT_U8 data)
{
    reg_symphony_optm_disp_sd_post_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG;
    d.bitc.sd_hori_enha = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_post_cfg_sd_hori_enha(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_post_cfg_t *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG).bitc.sd_hori_enha;
}

void reg_symphony_optm_disp_set_sd_post_cfg_sd_shoot_chg(MT_U8 data)
{
    reg_symphony_optm_disp_sd_post_cfg_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG;
    d.bitc.sd_shoot_chg = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_post_cfg_sd_shoot_chg(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_post_cfg_t *)REG_SYMPHONY_OPTM_DISP_SD_POST_CFG).bitc.sd_shoot_chg;
}


/*!
  register SYMPHONY_DISP_HD_EFFECT_COEF (read/write)
  */
void reg_symphony_optm_disp_set_hd_effect_coef(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_EFFECT_COEF, data);
}

MT_U32  reg_symphony_optm_disp_get_hd_effect_coef(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_EFFECT_COEF);
}

void reg_symphony_optm_disp_set_hd_effect_coef_bright_coef(MT_U8 data)
{
    reg_symphony_optm_disp_hd_effect_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_EFFECT_COEF;
    d.bitc.bright_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_EFFECT_COEF, d.all);
}

MT_U8   reg_symphony_optm_disp_get_hd_effect_coef_bright_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_hd_effect_coef_t *)REG_SYMPHONY_OPTM_DISP_HD_EFFECT_COEF).bitc.bright_coef;
}

void reg_symphony_optm_disp_set_hd_effect_coef_contrast_coef(MT_U8 data)
{
    reg_symphony_optm_disp_hd_effect_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_EFFECT_COEF;
    d.bitc.contrast_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_EFFECT_COEF, d.all);
}

MT_U8   reg_symphony_optm_disp_get_hd_effect_coef_contrast_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_hd_effect_coef_t *)REG_SYMPHONY_OPTM_DISP_HD_EFFECT_COEF).bitc.contrast_coef;
}

void reg_symphony_optm_disp_set_hd_effect_coef_saturation_coef(MT_U8 data)
{
    reg_symphony_optm_disp_hd_effect_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_EFFECT_COEF;
    d.bitc.saturation_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_EFFECT_COEF, d.all);
}

MT_U8   reg_symphony_optm_disp_get_hd_effect_coef_saturation_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_hd_effect_coef_t *)REG_SYMPHONY_OPTM_DISP_HD_EFFECT_COEF).bitc.saturation_coef;
}


/*!
  register SYMPHONY_DISP_SD_EFFECT_COEF (read/write)
  */
void reg_symphony_optm_disp_set_sd_effect_coef(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_EFFECT_COEF, data);
}

MT_U32  reg_symphony_optm_disp_get_sd_effect_coef(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_EFFECT_COEF);
}

void reg_symphony_optm_disp_set_sd_effect_coef_bright_coef(MT_U8 data)
{
    reg_symphony_optm_disp_sd_effect_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_EFFECT_COEF;
    d.bitc.bright_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_EFFECT_COEF, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_effect_coef_bright_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_effect_coef_t *)REG_SYMPHONY_OPTM_DISP_SD_EFFECT_COEF).bitc.bright_coef;
}

void reg_symphony_optm_disp_set_sd_effect_coef_contrast_coef(MT_U8 data)
{
    reg_symphony_optm_disp_sd_effect_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_EFFECT_COEF;
    d.bitc.contrast_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_EFFECT_COEF, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_effect_coef_contrast_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_effect_coef_t *)REG_SYMPHONY_OPTM_DISP_SD_EFFECT_COEF).bitc.contrast_coef;
}

void reg_symphony_optm_disp_set_sd_effect_coef_saturation_coef(MT_U8 data)
{
    reg_symphony_optm_disp_sd_effect_coef_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_EFFECT_COEF;
    d.bitc.saturation_coef = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_EFFECT_COEF, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_effect_coef_saturation_coef(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_effect_coef_t *)REG_SYMPHONY_OPTM_DISP_SD_EFFECT_COEF).bitc.saturation_coef;
}


/*!
  register SYMPHONY_DISP_GRA_FIFO_THRESHOLD (read/write)
  */
void reg_symphony_optm_disp_set_gra_fifo_threshold(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD, data);
}

MT_U32  reg_symphony_optm_disp_get_gra_fifo_threshold(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD);
}

void reg_symphony_optm_disp_set_gra_fifo_threshold_fifo64_lo(MT_U8 data)
{
    reg_symphony_optm_disp_gra_fifo_threshold_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD;
    d.bitc.fifo64_lo = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_fifo_threshold_fifo64_lo(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_fifo_threshold_t *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD).bitc.fifo64_lo;
}

void reg_symphony_optm_disp_set_gra_fifo_threshold_fifo64_hi(MT_U8 data)
{
    reg_symphony_optm_disp_gra_fifo_threshold_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD;
    d.bitc.fifo64_hi = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_fifo_threshold_fifo64_hi(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_fifo_threshold_t *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD).bitc.fifo64_hi;
}

void reg_symphony_optm_disp_set_gra_fifo_threshold_fifo128_lo(MT_U8 data)
{
    reg_symphony_optm_disp_gra_fifo_threshold_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD;
    d.bitc.fifo128_lo = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_fifo_threshold_fifo128_lo(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_fifo_threshold_t *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD).bitc.fifo128_lo;
}

void reg_symphony_optm_disp_set_gra_fifo_threshold_fifo128_hi(MT_U8 data)
{
    reg_symphony_optm_disp_gra_fifo_threshold_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD;
    d.bitc.fifo128_hi = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_fifo_threshold_fifo128_hi(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_fifo_threshold_t *)REG_SYMPHONY_OPTM_DISP_GRA_FIFO_THRESHOLD).bitc.fifo128_hi;
}


/*!
  register SYMPHONY_DISP_GRA_SCALER_CTRL (read/write)
  */
void reg_symphony_optm_disp_set_gra_scaler_ctrl(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL, data);
}

MT_U32  reg_symphony_optm_disp_get_gra_scaler_ctrl(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL);
}

void reg_symphony_optm_disp_set_gra_scaler_ctrl_hori_filter_en(MT_U8 data)
{
    reg_symphony_optm_disp_gra_scaler_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL;
    d.bitc.hori_filter_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_scaler_ctrl_hori_filter_en(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL).bitc.hori_filter_en;
}

void reg_symphony_optm_disp_set_gra_scaler_ctrl_vert_filter_en(MT_U8 data)
{
    reg_symphony_optm_disp_gra_scaler_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL;
    d.bitc.vert_filter_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_scaler_ctrl_vert_filter_en(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL).bitc.vert_filter_en;
}

void reg_symphony_optm_disp_set_gra_scaler_ctrl_hori_phase_type(MT_U8 data)
{
    reg_symphony_optm_disp_gra_scaler_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL;
    d.bitc.hori_phase_type = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_scaler_ctrl_hori_phase_type(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL).bitc.hori_phase_type;
}

void reg_symphony_optm_disp_set_gra_scaler_ctrl_vert_phase_type(MT_U8 data)
{
    reg_symphony_optm_disp_gra_scaler_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL;
    d.bitc.vert_phase_type = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_scaler_ctrl_vert_phase_type(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL).bitc.vert_phase_type;
}

void reg_symphony_optm_disp_set_gra_scaler_ctrl_vert_table_num(MT_U8 data)
{
    reg_symphony_optm_disp_gra_scaler_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL;
    d.bitc.vert_table_num = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_scaler_ctrl_vert_table_num(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL).bitc.vert_table_num;
}

void reg_symphony_optm_disp_set_gra_scaler_ctrl_hori_table_num(MT_U8 data)
{
    reg_symphony_optm_disp_gra_scaler_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL;
    d.bitc.hori_table_num = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_scaler_ctrl_hori_table_num(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL).bitc.hori_table_num;
}

void reg_symphony_optm_disp_set_gra_scaler_ctrl_vert_start_line_odd(MT_U8 data)
{
    reg_symphony_optm_disp_gra_scaler_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL;
    d.bitc.vert_start_line_odd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_scaler_ctrl_vert_start_line_odd(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL).bitc.vert_start_line_odd;
}

void reg_symphony_optm_disp_set_gra_scaler_ctrl_vert_start_line_even(MT_U8 data)
{
    reg_symphony_optm_disp_gra_scaler_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL;
    d.bitc.vert_start_line_even = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_scaler_ctrl_vert_start_line_even(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_ctrl_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_CTRL).bitc.vert_start_line_even;
}


/*!
  register SYMPHONY_DISP_GRA_SCALER_HRATIO (read/write)
  */
void reg_symphony_optm_disp_set_gra_scaler_hratio(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_HRATIO, data);
}

MT_U32  reg_symphony_optm_disp_get_gra_scaler_hratio(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_HRATIO);
}

void reg_symphony_optm_disp_set_gra_scaler_hratio_hratio_int(MT_U8 data)
{
    reg_symphony_optm_disp_gra_scaler_hratio_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_HRATIO;
    d.bitc.hratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_HRATIO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_scaler_hratio_hratio_int(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_hratio_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_HRATIO).bitc.hratio_int;
}

void reg_symphony_optm_disp_set_gra_scaler_hratio_hratio_fra(MT_U16 data)
{
    reg_symphony_optm_disp_gra_scaler_hratio_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_HRATIO;
    d.bitc.hratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_HRATIO, d.all);
}

MT_U16  reg_symphony_optm_disp_get_gra_scaler_hratio_hratio_fra(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_hratio_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_HRATIO).bitc.hratio_fra;
}


/*!
  register SYMPHONY_DISP_GRA_SCALER_VRATIO (read/write)
  */
void reg_symphony_optm_disp_set_gra_scaler_vratio(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_VRATIO, data);
}

MT_U32  reg_symphony_optm_disp_get_gra_scaler_vratio(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_VRATIO);
}

void reg_symphony_optm_disp_set_gra_scaler_vratio_vratio_int(MT_U8 data)
{
    reg_symphony_optm_disp_gra_scaler_vratio_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_VRATIO;
    d.bitc.vratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_VRATIO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_scaler_vratio_vratio_int(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_vratio_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_VRATIO).bitc.vratio_int;
}

void reg_symphony_optm_disp_set_gra_scaler_vratio_vratio_fra(MT_U16 data)
{
    reg_symphony_optm_disp_gra_scaler_vratio_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_VRATIO;
    d.bitc.vratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_VRATIO, d.all);
}

MT_U16  reg_symphony_optm_disp_get_gra_scaler_vratio_vratio_fra(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_vratio_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_VRATIO).bitc.vratio_fra;
}


/*!
  register SYMPHONY_DISP_GRA_SCALER_H_START_FRA (read/write)
  */
void reg_symphony_optm_disp_set_gra_scaler_h_start_fra(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_H_START_FRA, data);
}

MT_U32  reg_symphony_optm_disp_get_gra_scaler_h_start_fra(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_H_START_FRA);
}

void reg_symphony_optm_disp_set_gra_scaler_h_start_fra_h_start_fra(MT_U16 data)
{
    reg_symphony_optm_disp_gra_scaler_h_start_fra_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_H_START_FRA;
    d.bitc.h_start_fra = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_H_START_FRA, d.all);
}

MT_U16  reg_symphony_optm_disp_get_gra_scaler_h_start_fra_h_start_fra(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_h_start_fra_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_H_START_FRA).bitc.h_start_fra;
}


/*!
  register SYMPHONY_DISP_GRA_SCALER_V_START_FRA (read/write)
  */
void reg_symphony_optm_disp_set_gra_scaler_v_start_fra(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_V_START_FRA, data);
}

MT_U32  reg_symphony_optm_disp_get_gra_scaler_v_start_fra(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_V_START_FRA);
}

void reg_symphony_optm_disp_set_gra_scaler_v_start_fra_v_start_fra_odd(MT_U16 data)
{
    reg_symphony_optm_disp_gra_scaler_v_start_fra_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_V_START_FRA;
    d.bitc.v_start_fra_odd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_V_START_FRA, d.all);
}

MT_U16  reg_symphony_optm_disp_get_gra_scaler_v_start_fra_v_start_fra_odd(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_v_start_fra_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_V_START_FRA).bitc.v_start_fra_odd;
}

void reg_symphony_optm_disp_set_gra_scaler_v_start_fra_v_start_fra_even(MT_U16 data)
{
    reg_symphony_optm_disp_gra_scaler_v_start_fra_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_V_START_FRA;
    d.bitc.v_start_fra_even = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_V_START_FRA, d.all);
}

MT_U16  reg_symphony_optm_disp_get_gra_scaler_v_start_fra_v_start_fra_even(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_scaler_v_start_fra_t *)REG_SYMPHONY_OPTM_DISP_GRA_SCALER_V_START_FRA).bitc.v_start_fra_even;
}


/*!
  register SYMPHONY_DISP_GRA_CTL (read/write)
  */
void reg_symphony_optm_disp_set_gra_ctl(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_CTL, data);
}

MT_U32  reg_symphony_optm_disp_get_gra_ctl(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_CTL);
}

void reg_symphony_optm_disp_set_gra_ctl_hd_prog_mode(MT_U8 data)
{
    reg_symphony_optm_disp_gra_ctl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_CTL;
    d.bitc.hd_prog_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_CTL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_ctl_hd_prog_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_ctl_t *)REG_SYMPHONY_OPTM_DISP_GRA_CTL).bitc.hd_prog_mode;
}

void reg_symphony_optm_disp_set_gra_ctl_reduce_fr_osd0(MT_U8 data)
{
    reg_symphony_optm_disp_gra_ctl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_CTL;
    d.bitc.reduce_fr_osd0 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_CTL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_ctl_reduce_fr_osd0(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_ctl_t *)REG_SYMPHONY_OPTM_DISP_GRA_CTL).bitc.reduce_fr_osd0;
}

void reg_symphony_optm_disp_set_gra_ctl_reduce_fr_osd1(MT_U8 data)
{
    reg_symphony_optm_disp_gra_ctl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_CTL;
    d.bitc.reduce_fr_osd1 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_CTL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_ctl_reduce_fr_osd1(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_ctl_t *)REG_SYMPHONY_OPTM_DISP_GRA_CTL).bitc.reduce_fr_osd1;
}

void reg_symphony_optm_disp_set_gra_ctl_reduce_fr_sub(MT_U8 data)
{
    reg_symphony_optm_disp_gra_ctl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_CTL;
    d.bitc.reduce_fr_sub = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_CTL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_ctl_reduce_fr_sub(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_ctl_t *)REG_SYMPHONY_OPTM_DISP_GRA_CTL).bitc.reduce_fr_sub;
}

void reg_symphony_optm_disp_set_gra_ctl_reduce_fr_hd_still(MT_U8 data)
{
    reg_symphony_optm_disp_gra_ctl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_CTL;
    d.bitc.reduce_fr_hd_still = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_CTL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_gra_ctl_reduce_fr_hd_still(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_ctl_t *)REG_SYMPHONY_OPTM_DISP_GRA_CTL).bitc.reduce_fr_hd_still;
}


/*!
  register SYMPHONY_DISP_OSD_SCALE_HSIZE (read/write)
  */
void reg_symphony_optm_disp_set_osd_scale_hsize(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_HSIZE, data);
}

MT_U32  reg_symphony_optm_disp_get_osd_scale_hsize(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_HSIZE);
}

void reg_symphony_optm_disp_set_osd_scale_hsize_osd_dst_hsize(MT_U16 data)
{
    reg_symphony_optm_disp_osd_scale_hsize_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_HSIZE;
    d.bitc.osd_dst_hsize = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_HSIZE, d.all);
}

MT_U16  reg_symphony_optm_disp_get_osd_scale_hsize_osd_dst_hsize(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_scale_hsize_t *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_HSIZE).bitc.osd_dst_hsize;
}

void reg_symphony_optm_disp_set_osd_scale_hsize_osd_src_hsize(MT_U16 data)
{
    reg_symphony_optm_disp_osd_scale_hsize_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_HSIZE;
    d.bitc.osd_src_hsize = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_HSIZE, d.all);
}

MT_U16  reg_symphony_optm_disp_get_osd_scale_hsize_osd_src_hsize(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_scale_hsize_t *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_HSIZE).bitc.osd_src_hsize;
}


/*!
  register SYMPHONY_DISP_OSD_SCALE_RATIO (read/write)
  */
void reg_symphony_optm_disp_set_osd_scale_ratio(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_RATIO, data);
}

MT_U32  reg_symphony_optm_disp_get_osd_scale_ratio(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_RATIO);
}

void reg_symphony_optm_disp_set_osd_scale_ratio_osd_ratio_fra(MT_U16 data)
{
    reg_symphony_optm_disp_osd_scale_ratio_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_RATIO;
    d.bitc.osd_ratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_RATIO, d.all);
}

MT_U16  reg_symphony_optm_disp_get_osd_scale_ratio_osd_ratio_fra(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_scale_ratio_t *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_RATIO).bitc.osd_ratio_fra;
}

void reg_symphony_optm_disp_set_osd_scale_ratio_osd_ratio_int(MT_U8 data)
{
    reg_symphony_optm_disp_osd_scale_ratio_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_RATIO;
    d.bitc.osd_ratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_RATIO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd_scale_ratio_osd_ratio_int(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_scale_ratio_t *)REG_SYMPHONY_OPTM_DISP_OSD_SCALE_RATIO).bitc.osd_ratio_int;
}


/*!
  register SYMPHONY_DISP_OSD_ALPHA (read/write)
  */
void reg_symphony_optm_disp_set_osd_alpha(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA, data);
}

MT_U32  reg_symphony_optm_disp_get_osd_alpha(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA);
}

void reg_symphony_optm_disp_set_osd_alpha_osd_alpha_filter(MT_U8 data)
{
    reg_symphony_optm_disp_osd_alpha_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA;
    d.bitc.osd_alpha_filter = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd_alpha_osd_alpha_filter(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_alpha_t *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA).bitc.osd_alpha_filter;
}

void reg_symphony_optm_disp_set_osd_alpha_border_cfg(MT_U8 data)
{
    reg_symphony_optm_disp_osd_alpha_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA;
    d.bitc.border_cfg = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd_alpha_border_cfg(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_alpha_t *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA).bitc.border_cfg;
}

void reg_symphony_optm_disp_set_osd_alpha_osd_start_fra(MT_U16 data)
{
    reg_symphony_optm_disp_osd_alpha_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA;
    d.bitc.osd_start_fra = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA, d.all);
}

MT_U16  reg_symphony_optm_disp_get_osd_alpha_osd_start_fra(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_alpha_t *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA).bitc.osd_start_fra;
}

void reg_symphony_optm_disp_set_osd_alpha_osd_no_filter(MT_U8 data)
{
    reg_symphony_optm_disp_osd_alpha_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA;
    d.bitc.osd_no_filter = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd_alpha_osd_no_filter(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_alpha_t *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA).bitc.osd_no_filter;
}

void reg_symphony_optm_disp_set_osd_alpha_osd_sub_mix_first(MT_U8 data)
{
    reg_symphony_optm_disp_osd_alpha_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA;
    d.bitc.osd_sub_mix_first = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd_alpha_osd_sub_mix_first(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_alpha_t *)REG_SYMPHONY_OPTM_DISP_OSD_ALPHA).bitc.osd_sub_mix_first;
}


/*!
  register SYMPHONY_DISP_OSD_VERT_START_LINE (read/write)
  */
void reg_symphony_optm_disp_set_osd_vert_start_line(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VERT_START_LINE, data);
}

MT_U32  reg_symphony_optm_disp_get_osd_vert_start_line(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VERT_START_LINE);
}

void reg_symphony_optm_disp_set_osd_vert_start_line_osd_odd_start_line(MT_U8 data)
{
    reg_symphony_optm_disp_osd_vert_start_line_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VERT_START_LINE;
    d.bitc.osd_odd_start_line = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VERT_START_LINE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd_vert_start_line_osd_odd_start_line(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_vert_start_line_t *)REG_SYMPHONY_OPTM_DISP_OSD_VERT_START_LINE).bitc.osd_odd_start_line;
}

void reg_symphony_optm_disp_set_osd_vert_start_line_osd_even_start_line(MT_U8 data)
{
    reg_symphony_optm_disp_osd_vert_start_line_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VERT_START_LINE;
    d.bitc.osd_even_start_line = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VERT_START_LINE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd_vert_start_line_osd_even_start_line(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_vert_start_line_t *)REG_SYMPHONY_OPTM_DISP_OSD_VERT_START_LINE).bitc.osd_even_start_line;
}


/*!
  register SYMPHONY_DISP_OSD_VERTICAL_CTRL (read/write)
  */
void reg_symphony_optm_disp_set_osd_vertical_ctrl(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_CTRL, data);
}

MT_U32  reg_symphony_optm_disp_get_osd_vertical_ctrl(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_CTRL);
}

void reg_symphony_optm_disp_set_osd_vertical_ctrl_osd_vert_phase_type(MT_U8 data)
{
    reg_symphony_optm_disp_osd_vertical_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_CTRL;
    d.bitc.osd_vert_phase_type = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd_vertical_ctrl_osd_vert_phase_type(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_vertical_ctrl_t *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_CTRL).bitc.osd_vert_phase_type;
}

void reg_symphony_optm_disp_set_osd_vertical_ctrl_osd_vert_no_filter(MT_U8 data)
{
    reg_symphony_optm_disp_osd_vertical_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_CTRL;
    d.bitc.osd_vert_no_filter = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd_vertical_ctrl_osd_vert_no_filter(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_vertical_ctrl_t *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_CTRL).bitc.osd_vert_no_filter;
}

void reg_symphony_optm_disp_set_osd_vertical_ctrl_osd_vert_bypass(MT_U8 data)
{
    reg_symphony_optm_disp_osd_vertical_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_CTRL;
    d.bitc.osd_vert_bypass = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd_vertical_ctrl_osd_vert_bypass(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_vertical_ctrl_t *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_CTRL).bitc.osd_vert_bypass;
}


/*!
  register SYMPHONY_DISP_OSD_VERTICAL_SIZE (read/write)
  */
void reg_symphony_optm_disp_set_osd_vertical_size(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_SIZE, data);
}

MT_U32  reg_symphony_optm_disp_get_osd_vertical_size(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_SIZE);
}

void reg_symphony_optm_disp_set_osd_vertical_size_osd_dst_vsize(MT_U16 data)
{
    reg_symphony_optm_disp_osd_vertical_size_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_SIZE;
    d.bitc.osd_dst_vsize = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_SIZE, d.all);
}

MT_U16  reg_symphony_optm_disp_get_osd_vertical_size_osd_dst_vsize(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_vertical_size_t *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_SIZE).bitc.osd_dst_vsize;
}

void reg_symphony_optm_disp_set_osd_vertical_size_osd_ori_vsize(MT_U16 data)
{
    reg_symphony_optm_disp_osd_vertical_size_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_SIZE;
    d.bitc.osd_ori_vsize = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_SIZE, d.all);
}

MT_U16  reg_symphony_optm_disp_get_osd_vertical_size_osd_ori_vsize(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_vertical_size_t *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_SIZE).bitc.osd_ori_vsize;
}


/*!
  register SYMPHONY_DISP_OSD_VERTICAL_RATIO (read/write)
  */
void reg_symphony_optm_disp_set_osd_vertical_ratio(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_RATIO, data);
}

MT_U32  reg_symphony_optm_disp_get_osd_vertical_ratio(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_RATIO);
}

void reg_symphony_optm_disp_set_osd_vertical_ratio_osd_vratio_fra(MT_U16 data)
{
    reg_symphony_optm_disp_osd_vertical_ratio_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_RATIO;
    d.bitc.osd_vratio_fra = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_RATIO, d.all);
}

MT_U16  reg_symphony_optm_disp_get_osd_vertical_ratio_osd_vratio_fra(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_vertical_ratio_t *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_RATIO).bitc.osd_vratio_fra;
}

void reg_symphony_optm_disp_set_osd_vertical_ratio_osd_vratio_int(MT_U8 data)
{
    reg_symphony_optm_disp_osd_vertical_ratio_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_RATIO;
    d.bitc.osd_vratio_int = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_RATIO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd_vertical_ratio_osd_vratio_int(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_vertical_ratio_t *)REG_SYMPHONY_OPTM_DISP_OSD_VERTICAL_RATIO).bitc.osd_vratio_int;
}


/*!
  register SYMPHONY_DISP_OSD_V_START_FRA (read/write)
  */
void reg_symphony_optm_disp_set_osd_v_start_fra(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_V_START_FRA, data);
}

MT_U32  reg_symphony_optm_disp_get_osd_v_start_fra(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_V_START_FRA);
}

void reg_symphony_optm_disp_set_osd_v_start_fra_osd_v_start_fra_odd(MT_U16 data)
{
    reg_symphony_optm_disp_osd_v_start_fra_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_V_START_FRA;
    d.bitc.osd_v_start_fra_odd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_V_START_FRA, d.all);
}

MT_U16  reg_symphony_optm_disp_get_osd_v_start_fra_osd_v_start_fra_odd(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_v_start_fra_t *)REG_SYMPHONY_OPTM_DISP_OSD_V_START_FRA).bitc.osd_v_start_fra_odd;
}

void reg_symphony_optm_disp_set_osd_v_start_fra_osd_v_start_fra_even(MT_U16 data)
{
    reg_symphony_optm_disp_osd_v_start_fra_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_V_START_FRA;
    d.bitc.osd_v_start_fra_even = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_V_START_FRA, d.all);
}

MT_U16  reg_symphony_optm_disp_get_osd_v_start_fra_osd_v_start_fra_even(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_v_start_fra_t *)REG_SYMPHONY_OPTM_DISP_OSD_V_START_FRA).bitc.osd_v_start_fra_even;
}


/*!
  register SYMPHONY_DISP_OSD_V_TAP_NUM (read/write)
  */
void reg_symphony_optm_disp_set_osd_v_tap_num(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_V_TAP_NUM, data);
}

MT_U32  reg_symphony_optm_disp_get_osd_v_tap_num(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_V_TAP_NUM);
}

void reg_symphony_optm_disp_set_osd_v_tap_num_tap_num(MT_U8 data)
{
    reg_symphony_optm_disp_osd_v_tap_num_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_V_TAP_NUM;
    d.bitc.tap_num = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_V_TAP_NUM, d.all);
}

MT_U8   reg_symphony_optm_disp_get_osd_v_tap_num_tap_num(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_v_tap_num_t *)REG_SYMPHONY_OPTM_DISP_OSD_V_TAP_NUM).bitc.tap_num;
}


/*!
  register SYMPHONY_DISP_CHROMA_COEF0 (read/write)
  */
void reg_symphony_optm_disp_set_chroma_coef0(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF0, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_coef0(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF0);
}

void reg_symphony_optm_disp_set_chroma_coef0_chroma_hori_coef1(MT_U16 data)
{
    reg_symphony_optm_disp_chroma_coef0_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF0;
    d.bitc.chroma_hori_coef1 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF0, d.all);
}

MT_U16  reg_symphony_optm_disp_get_chroma_coef0_chroma_hori_coef1(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_coef0_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF0).bitc.chroma_hori_coef1;
}

void reg_symphony_optm_disp_set_chroma_coef0_chroma_hori_coef0(MT_U16 data)
{
    reg_symphony_optm_disp_chroma_coef0_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF0;
    d.bitc.chroma_hori_coef0 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF0, d.all);
}

MT_U16  reg_symphony_optm_disp_get_chroma_coef0_chroma_hori_coef0(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_coef0_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF0).bitc.chroma_hori_coef0;
}


/*!
  register SYMPHONY_DISP_CHROMA_COEF1 (read/write)
  */
void reg_symphony_optm_disp_set_chroma_coef1(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF1, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_coef1(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF1);
}

void reg_symphony_optm_disp_set_chroma_coef1_chroma_hori_coef3(MT_U16 data)
{
    reg_symphony_optm_disp_chroma_coef1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF1;
    d.bitc.chroma_hori_coef3 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF1, d.all);
}

MT_U16  reg_symphony_optm_disp_get_chroma_coef1_chroma_hori_coef3(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_coef1_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF1).bitc.chroma_hori_coef3;
}

void reg_symphony_optm_disp_set_chroma_coef1_chroma_hori_coef2(MT_U16 data)
{
    reg_symphony_optm_disp_chroma_coef1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF1;
    d.bitc.chroma_hori_coef2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF1, d.all);
}

MT_U16  reg_symphony_optm_disp_get_chroma_coef1_chroma_hori_coef2(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_coef1_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF1).bitc.chroma_hori_coef2;
}


/*!
  register SYMPHONY_DISP_CHROMA_COEF2 (read/write)
  */
void reg_symphony_optm_disp_set_chroma_coef2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF2, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_coef2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF2);
}

void reg_symphony_optm_disp_set_chroma_coef2_chroma_hori_coef5(MT_U16 data)
{
    reg_symphony_optm_disp_chroma_coef2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF2;
    d.bitc.chroma_hori_coef5 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF2, d.all);
}

MT_U16  reg_symphony_optm_disp_get_chroma_coef2_chroma_hori_coef5(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_coef2_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF2).bitc.chroma_hori_coef5;
}

void reg_symphony_optm_disp_set_chroma_coef2_chroma_hori_coef4(MT_U16 data)
{
    reg_symphony_optm_disp_chroma_coef2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF2;
    d.bitc.chroma_hori_coef4 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF2, d.all);
}

MT_U16  reg_symphony_optm_disp_get_chroma_coef2_chroma_hori_coef4(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_coef2_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF2).bitc.chroma_hori_coef4;
}


/*!
  register SYMPHONY_DISP_CHROMA_COEF3 (read/write)
  */
void reg_symphony_optm_disp_set_chroma_coef3(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF3, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_coef3(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF3);
}

void reg_symphony_optm_disp_set_chroma_coef3_chroma_hori_coef6(MT_U16 data)
{
    reg_symphony_optm_disp_chroma_coef3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF3;
    d.bitc.chroma_hori_coef6 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF3, d.all);
}

MT_U16  reg_symphony_optm_disp_get_chroma_coef3_chroma_hori_coef6(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_coef3_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_COEF3).bitc.chroma_hori_coef6;
}


/*!
  register SYMPHONY_DISP_SCALE_INIT_PHASE_OFFSET (read/write)
  */
void reg_symphony_optm_disp_set_scale_init_phase_offset(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SCALE_INIT_PHASE_OFFSET, data);
}

MT_U32  reg_symphony_optm_disp_get_scale_init_phase_offset(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SCALE_INIT_PHASE_OFFSET);
}

void reg_symphony_optm_disp_set_scale_init_phase_offset_offset(MT_U8 data)
{
    reg_symphony_optm_disp_scale_init_phase_offset_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SCALE_INIT_PHASE_OFFSET;
    d.bitc.offset = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SCALE_INIT_PHASE_OFFSET, d.all);
}

MT_U8   reg_symphony_optm_disp_get_scale_init_phase_offset_offset(void)
{
    return (*(volatile reg_symphony_optm_disp_scale_init_phase_offset_t *)REG_SYMPHONY_OPTM_DISP_SCALE_INIT_PHASE_OFFSET).bitc.offset;
}

MT_U8   reg_symphony_optm_disp_get_vid_fmt(void)
{
    return (*(volatile reg_symphony_optm_disp_scale_init_phase_offset_t *)
    REG_SYMPHONY_OPTM_DISP_SCALE_INIT_PHASE_OFFSET).bitc.vid_fmt;
}

MT_U8   reg_symphony_optm_disp_get_input_ar(void)
{
    return (*(volatile reg_symphony_optm_disp_scale_init_phase_offset_t *)
    REG_SYMPHONY_OPTM_DISP_SCALE_INIT_PHASE_OFFSET).bitc.input_ar;
}

MT_U8   reg_symphony_optm_disp_get_input_fr(void)
{
    return (*(volatile reg_symphony_optm_disp_scale_init_phase_offset_t *)
    REG_SYMPHONY_OPTM_DISP_SCALE_INIT_PHASE_OFFSET).bitc.input_fr;
}

/*!
  register SYMPHONY_DISP_SMALL_PIC_UPSCALE_CTRL (read/write)
  */
void reg_symphony_optm_disp_set_small_pic_upscale_ctrl(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SMALL_PIC_UPSCALE_CTRL, data);
}

MT_U32  reg_symphony_optm_disp_get_small_pic_upscale_ctrl(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SMALL_PIC_UPSCALE_CTRL);
}

void reg_symphony_optm_disp_set_small_pic_upscale_ctrl_upscale_en(MT_U8 data)
{
    reg_symphony_optm_disp_small_pic_upscale_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SMALL_PIC_UPSCALE_CTRL;
    d.bitc.upscale_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SMALL_PIC_UPSCALE_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_small_pic_upscale_ctrl_upscale_en(void)
{
    return (*(volatile reg_symphony_optm_disp_small_pic_upscale_ctrl_t *)REG_SYMPHONY_OPTM_DISP_SMALL_PIC_UPSCALE_CTRL).bitc.upscale_en;
}

void reg_symphony_optm_disp_set_small_pic_upscale_ctrl_chroma_hori_ip_mode(MT_U8 data)
{
    reg_symphony_optm_disp_small_pic_upscale_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SMALL_PIC_UPSCALE_CTRL;
    d.bitc.chroma_hori_ip_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SMALL_PIC_UPSCALE_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_small_pic_upscale_ctrl_chroma_hori_ip_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_small_pic_upscale_ctrl_t *)REG_SYMPHONY_OPTM_DISP_SMALL_PIC_UPSCALE_CTRL).bitc.chroma_hori_ip_mode;
}


/*!
  register SYMPHONY_DISP_ALISING_PROB_REG1 (read/write)
  */
void reg_symphony_optm_disp_set_alising_prob_reg1(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1, data);
}

MT_U32  reg_symphony_optm_disp_get_alising_prob_reg1(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1);
}

void reg_symphony_optm_disp_set_alising_prob_reg1_alpha_2nd_method_sel(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1;
    d.bitc.alpha_2nd_method_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg1_alpha_2nd_method_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg1_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1).bitc.alpha_2nd_method_sel;
}

void reg_symphony_optm_disp_set_alising_prob_reg1_alpha_2nd_diff_ratio_sel(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1;
    d.bitc.alpha_2nd_diff_ratio_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg1_alpha_2nd_diff_ratio_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg1_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1).bitc.alpha_2nd_diff_ratio_sel;
}

void reg_symphony_optm_disp_set_alising_prob_reg1_alpha_2nd_diff_shift_sel(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1;
    d.bitc.alpha_2nd_diff_shift_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg1_alpha_2nd_diff_shift_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg1_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1).bitc.alpha_2nd_diff_shift_sel;
}

void reg_symphony_optm_disp_set_alising_prob_reg1_alpha_2nd_diff(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1;
    d.bitc.alpha_2nd_diff = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg1_alpha_2nd_diff(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg1_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG1).bitc.alpha_2nd_diff;
}


/*!
  register SYMPHONY_DISP_ALISING_PROB_REG2 (read/write)
  */
void reg_symphony_optm_disp_set_alising_prob_reg2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2, data);
}

MT_U32  reg_symphony_optm_disp_get_alising_prob_reg2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2);
}

void reg_symphony_optm_disp_set_alising_prob_reg2_alpha_enlarge(MT_U16 data)
{
    reg_symphony_optm_disp_alising_prob_reg2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2;
    d.bitc.alpha_enlarge = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2, d.all);
}

MT_U16  reg_symphony_optm_disp_get_alising_prob_reg2_alpha_enlarge(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg2_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2).bitc.alpha_enlarge;
}

void reg_symphony_optm_disp_set_alising_prob_reg2_alpha_vdv_sel(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2;
    d.bitc.alpha_vdv_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg2_alpha_vdv_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg2_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2).bitc.alpha_vdv_sel;
}

void reg_symphony_optm_disp_set_alising_prob_reg2_alpha_vdv(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2;
    d.bitc.alpha_vdv = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg2_alpha_vdv(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg2_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2).bitc.alpha_vdv;
}

void reg_symphony_optm_disp_set_alising_prob_reg2_alpha_angle(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2;
    d.bitc.alpha_angle = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg2_alpha_angle(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg2_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG2).bitc.alpha_angle;
}


/*!
  register SYMPHONY_DISP_ALISING_PROB_REG3 (read/write)
  */
void reg_symphony_optm_disp_set_alising_prob_reg3(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3, data);
}

MT_U32  reg_symphony_optm_disp_get_alising_prob_reg3(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3);
}

void reg_symphony_optm_disp_set_alising_prob_reg3_base_blending_factor(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3;
    d.bitc.base_blending_factor = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg3_base_blending_factor(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg3_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3).bitc.base_blending_factor;
}

void reg_symphony_optm_disp_set_alising_prob_reg3_default_prob(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3;
    d.bitc.default_prob = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg3_default_prob(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg3_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3).bitc.default_prob;
}

void reg_symphony_optm_disp_set_alising_prob_reg3_op(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3;
    d.bitc.op = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg3_op(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg3_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3).bitc.op;
}

void reg_symphony_optm_disp_set_alising_prob_reg3_slope_diff1_sel(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3;
    d.bitc.slope_diff1_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg3_slope_diff1_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg3_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG3).bitc.slope_diff1_sel;
}


/*!
  register SYMPHONY_DISP_ALISING_PROB_REG4 (read/write)
  */
void reg_symphony_optm_disp_set_alising_prob_reg4(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4, data);
}

MT_U32  reg_symphony_optm_disp_get_alising_prob_reg4(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4);
}

void reg_symphony_optm_disp_set_alising_prob_reg4_pict_enhance_pix_sel(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4;
    d.bitc.pict_enhance_pix_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg4_pict_enhance_pix_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg4_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4).bitc.pict_enhance_pix_sel;
}

void reg_symphony_optm_disp_set_alising_prob_reg4_adaptive_alpha_sel(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4;
    d.bitc.adaptive_alpha_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg4_adaptive_alpha_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg4_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4).bitc.adaptive_alpha_sel;
}

void reg_symphony_optm_disp_set_alising_prob_reg4_diff_2nd_sel(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4;
    d.bitc.diff_2nd_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg4_diff_2nd_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg4_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4).bitc.diff_2nd_sel;
}

void reg_symphony_optm_disp_set_alising_prob_reg4_interp_factor(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4;
    d.bitc.interp_factor = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg4_interp_factor(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg4_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4).bitc.interp_factor;
}

void reg_symphony_optm_disp_set_alising_prob_reg4_prob_coef3(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4;
    d.bitc.prob_coef3 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg4_prob_coef3(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg4_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4).bitc.prob_coef3;
}

void reg_symphony_optm_disp_set_alising_prob_reg4_prob_coef2(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4;
    d.bitc.prob_coef2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg4_prob_coef2(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg4_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4).bitc.prob_coef2;
}

void reg_symphony_optm_disp_set_alising_prob_reg4_prob_coef1(MT_U8 data)
{
    reg_symphony_optm_disp_alising_prob_reg4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4;
    d.bitc.prob_coef1 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_alising_prob_reg4_prob_coef1(void)
{
    return (*(volatile reg_symphony_optm_disp_alising_prob_reg4_t *)REG_SYMPHONY_OPTM_DISP_ALISING_PROB_REG4).bitc.prob_coef1;
}


/*!
  register SYMPHONY_DISP_CSC_CTRL (read/write)
  */
void reg_symphony_optm_disp_set_csc_ctrl(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_ctrl(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL);
}

void reg_symphony_optm_disp_set_csc_ctrl_sd_bund_out_en(MT_U8 data)
{
    reg_symphony_optm_disp_csc_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL;
    d.bitc.sd_bund_out_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_csc_ctrl_sd_bund_out_en(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_ctrl_t *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL).bitc.sd_bund_out_en;
}

void reg_symphony_optm_disp_set_csc_ctrl_hd_bund_out_en(MT_U8 data)
{
    reg_symphony_optm_disp_csc_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL;
    d.bitc.hd_bund_out_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_csc_ctrl_hd_bund_out_en(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_ctrl_t *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL).bitc.hd_bund_out_en;
}

void reg_symphony_optm_disp_set_csc_ctrl_sd_scs_en(MT_U8 data)
{
    reg_symphony_optm_disp_csc_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL;
    d.bitc.sd_scs_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_csc_ctrl_sd_scs_en(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_ctrl_t *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL).bitc.sd_scs_en;
}

void reg_symphony_optm_disp_set_csc_ctrl_sd_bund_in_en(MT_U8 data)
{
    reg_symphony_optm_disp_csc_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL;
    d.bitc.sd_bund_in_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_csc_ctrl_sd_bund_in_en(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_ctrl_t *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL).bitc.sd_bund_in_en;
}

void reg_symphony_optm_disp_set_csc_ctrl_hd_bund_in_en(MT_U8 data)
{
    reg_symphony_optm_disp_csc_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL;
    d.bitc.hd_bund_in_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_csc_ctrl_hd_bund_in_en(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_ctrl_t *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL).bitc.hd_bund_in_en;
}

void reg_symphony_optm_disp_set_csc_ctrl_hd_csc_en(MT_U8 data)
{
    reg_symphony_optm_disp_csc_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL;
    d.bitc.hd_csc_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_csc_ctrl_hd_csc_en(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_ctrl_t *)REG_SYMPHONY_OPTM_DISP_CSC_CTRL).bitc.hd_csc_en;
}


/*!
  register SYMPHONY_DISP_CSC_HD_COEF1 (read/write)
  */
void reg_symphony_optm_disp_set_csc_hd_coef1(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF1, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_hd_coef1(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF1);
}

void reg_symphony_optm_disp_set_csc_hd_coef1_hd_a_01(MT_U16 data)
{
    reg_symphony_optm_disp_csc_hd_coef1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF1;
    d.bitc.hd_a_01 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF1, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_hd_coef1_hd_a_01(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_hd_coef1_t *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF1).bitc.hd_a_01;
}

void reg_symphony_optm_disp_set_csc_hd_coef1_hd_a_00(MT_U16 data)
{
    reg_symphony_optm_disp_csc_hd_coef1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF1;
    d.bitc.hd_a_00 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF1, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_hd_coef1_hd_a_00(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_hd_coef1_t *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF1).bitc.hd_a_00;
}


/*!
  register SYMPHONY_DISP_CSC_HD_COEF2 (read/write)
  */
void reg_symphony_optm_disp_set_csc_hd_coef2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF2, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_hd_coef2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF2);
}

void reg_symphony_optm_disp_set_csc_hd_coef2_hd_a_10(MT_U16 data)
{
    reg_symphony_optm_disp_csc_hd_coef2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF2;
    d.bitc.hd_a_10 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF2, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_hd_coef2_hd_a_10(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_hd_coef2_t *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF2).bitc.hd_a_10;
}

void reg_symphony_optm_disp_set_csc_hd_coef2_hd_a_02(MT_U16 data)
{
    reg_symphony_optm_disp_csc_hd_coef2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF2;
    d.bitc.hd_a_02 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF2, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_hd_coef2_hd_a_02(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_hd_coef2_t *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF2).bitc.hd_a_02;
}


/*!
  register SYMPHONY_DISP_CSC_HD_COEF3 (read/write)
  */
void reg_symphony_optm_disp_set_csc_hd_coef3(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF3, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_hd_coef3(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF3);
}

void reg_symphony_optm_disp_set_csc_hd_coef3_hd_a_12(MT_U16 data)
{
    reg_symphony_optm_disp_csc_hd_coef3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF3;
    d.bitc.hd_a_12 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF3, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_hd_coef3_hd_a_12(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_hd_coef3_t *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF3).bitc.hd_a_12;
}

void reg_symphony_optm_disp_set_csc_hd_coef3_hd_a_11(MT_U16 data)
{
    reg_symphony_optm_disp_csc_hd_coef3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF3;
    d.bitc.hd_a_11 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF3, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_hd_coef3_hd_a_11(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_hd_coef3_t *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF3).bitc.hd_a_11;
}


/*!
  register SYMPHONY_DISP_CSC_HD_COEF4 (read/write)
  */
void reg_symphony_optm_disp_set_csc_hd_coef4(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF4, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_hd_coef4(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF4);
}

void reg_symphony_optm_disp_set_csc_hd_coef4_hd_a_21(MT_U16 data)
{
    reg_symphony_optm_disp_csc_hd_coef4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF4;
    d.bitc.hd_a_21 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF4, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_hd_coef4_hd_a_21(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_hd_coef4_t *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF4).bitc.hd_a_21;
}

void reg_symphony_optm_disp_set_csc_hd_coef4_hd_a_20(MT_U16 data)
{
    reg_symphony_optm_disp_csc_hd_coef4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF4;
    d.bitc.hd_a_20 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF4, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_hd_coef4_hd_a_20(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_hd_coef4_t *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF4).bitc.hd_a_20;
}


/*!
  register SYMPHONY_DISP_CSC_HD_COEF5 (read/write)
  */
void reg_symphony_optm_disp_set_csc_hd_coef5(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF5, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_hd_coef5(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF5);
}

void reg_symphony_optm_disp_set_csc_hd_coef5_hd_a_22(MT_U16 data)
{
    reg_symphony_optm_disp_csc_hd_coef5_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF5;
    d.bitc.hd_a_22 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF5, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_hd_coef5_hd_a_22(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_hd_coef5_t *)REG_SYMPHONY_OPTM_DISP_CSC_HD_COEF5).bitc.hd_a_22;
}


/*!
  register SYMPHONY_DISP_CSC_SD_COEF1 (read/write)
  */
void reg_symphony_optm_disp_set_csc_sd_coef1(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF1, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_sd_coef1(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF1);
}

void reg_symphony_optm_disp_set_csc_sd_coef1_sd_a_01(MT_U16 data)
{
    reg_symphony_optm_disp_csc_sd_coef1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF1;
    d.bitc.sd_a_01 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF1, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_sd_coef1_sd_a_01(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_sd_coef1_t *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF1).bitc.sd_a_01;
}

void reg_symphony_optm_disp_set_csc_sd_coef1_sd_a_00(MT_U16 data)
{
    reg_symphony_optm_disp_csc_sd_coef1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF1;
    d.bitc.sd_a_00 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF1, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_sd_coef1_sd_a_00(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_sd_coef1_t *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF1).bitc.sd_a_00;
}


/*!
  register SYMPHONY_DISP_CSC_SD_COEF2 (read/write)
  */
void reg_symphony_optm_disp_set_csc_sd_coef2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF2, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_sd_coef2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF2);
}

void reg_symphony_optm_disp_set_csc_sd_coef2_sd_a_10(MT_U16 data)
{
    reg_symphony_optm_disp_csc_sd_coef2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF2;
    d.bitc.sd_a_10 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF2, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_sd_coef2_sd_a_10(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_sd_coef2_t *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF2).bitc.sd_a_10;
}

void reg_symphony_optm_disp_set_csc_sd_coef2_sd_a_02(MT_U16 data)
{
    reg_symphony_optm_disp_csc_sd_coef2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF2;
    d.bitc.sd_a_02 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF2, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_sd_coef2_sd_a_02(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_sd_coef2_t *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF2).bitc.sd_a_02;
}


/*!
  register SYMPHONY_DISP_CSC_SD_COEF3 (read/write)
  */
void reg_symphony_optm_disp_set_csc_sd_coef3(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF3, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_sd_coef3(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF3);
}

void reg_symphony_optm_disp_set_csc_sd_coef3_sd_a_12(MT_U16 data)
{
    reg_symphony_optm_disp_csc_sd_coef3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF3;
    d.bitc.sd_a_12 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF3, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_sd_coef3_sd_a_12(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_sd_coef3_t *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF3).bitc.sd_a_12;
}

void reg_symphony_optm_disp_set_csc_sd_coef3_sd_a_11(MT_U16 data)
{
    reg_symphony_optm_disp_csc_sd_coef3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF3;
    d.bitc.sd_a_11 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF3, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_sd_coef3_sd_a_11(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_sd_coef3_t *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF3).bitc.sd_a_11;
}


/*!
  register SYMPHONY_DISP_CSC_SD_COEF4 (read/write)
  */
void reg_symphony_optm_disp_set_csc_sd_coef4(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF4, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_sd_coef4(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF4);
}

void reg_symphony_optm_disp_set_csc_sd_coef4_sd_a_21(MT_U16 data)
{
    reg_symphony_optm_disp_csc_sd_coef4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF4;
    d.bitc.sd_a_21 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF4, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_sd_coef4_sd_a_21(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_sd_coef4_t *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF4).bitc.sd_a_21;
}

void reg_symphony_optm_disp_set_csc_sd_coef4_sd_a_20(MT_U16 data)
{
    reg_symphony_optm_disp_csc_sd_coef4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF4;
    d.bitc.sd_a_20 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF4, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_sd_coef4_sd_a_20(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_sd_coef4_t *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF4).bitc.sd_a_20;
}


/*!
  register SYMPHONY_DISP_CSC_SD_COEF5 (read/write)
  */
void reg_symphony_optm_disp_set_csc_sd_coef5(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF5, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_sd_coef5(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF5);
}

void reg_symphony_optm_disp_set_csc_sd_coef5_sd_a_22(MT_U16 data)
{
    reg_symphony_optm_disp_csc_sd_coef5_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF5;
    d.bitc.sd_a_22 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF5, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_sd_coef5_sd_a_22(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_sd_coef5_t *)REG_SYMPHONY_OPTM_DISP_CSC_SD_COEF5).bitc.sd_a_22;
}


/*!
  register SYMPHONY_DISP_CSC_STILL_CTRL (read/write)
  */
void reg_symphony_optm_disp_set_csc_still_ctrl(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_CTRL, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_still_ctrl(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_CTRL);
}

void reg_symphony_optm_disp_set_csc_still_ctrl_hd_still_bund_out_en(MT_U8 data)
{
    reg_symphony_optm_disp_csc_still_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_CTRL;
    d.bitc.hd_still_bund_out_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_csc_still_ctrl_hd_still_bund_out_en(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_still_ctrl_t *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_CTRL).bitc.hd_still_bund_out_en;
}

void reg_symphony_optm_disp_set_csc_still_ctrl_hd_still_bund_in_en(MT_U8 data)
{
    reg_symphony_optm_disp_csc_still_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_CTRL;
    d.bitc.hd_still_bund_in_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_csc_still_ctrl_hd_still_bund_in_en(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_still_ctrl_t *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_CTRL).bitc.hd_still_bund_in_en;
}

void reg_symphony_optm_disp_set_csc_still_ctrl_hd_still_csc__en(MT_U8 data)
{
    reg_symphony_optm_disp_csc_still_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_CTRL;
    d.bitc.hd_still_csc__en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_csc_still_ctrl_hd_still_csc__en(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_still_ctrl_t *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_CTRL).bitc.hd_still_csc__en;
}


/*!
  register SYMPHONY_DISP_CSC_STILL_COEF1 (read/write)
  */
void reg_symphony_optm_disp_set_csc_still_coef1(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF1, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_still_coef1(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF1);
}

void reg_symphony_optm_disp_set_csc_still_coef1_still_a_01(MT_U16 data)
{
    reg_symphony_optm_disp_csc_still_coef1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF1;
    d.bitc.still_a_01 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF1, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_still_coef1_still_a_01(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_still_coef1_t *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF1).bitc.still_a_01;
}

void reg_symphony_optm_disp_set_csc_still_coef1_still_a_00(MT_U16 data)
{
    reg_symphony_optm_disp_csc_still_coef1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF1;
    d.bitc.still_a_00 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF1, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_still_coef1_still_a_00(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_still_coef1_t *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF1).bitc.still_a_00;
}


/*!
  register SYMPHONY_DISP_CSC_STILL_COEF2 (read/write)
  */
void reg_symphony_optm_disp_set_csc_still_coef2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF2, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_still_coef2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF2);
}

void reg_symphony_optm_disp_set_csc_still_coef2_still_a_10(MT_U16 data)
{
    reg_symphony_optm_disp_csc_still_coef2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF2;
    d.bitc.still_a_10 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF2, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_still_coef2_still_a_10(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_still_coef2_t *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF2).bitc.still_a_10;
}

void reg_symphony_optm_disp_set_csc_still_coef2_still_a_02(MT_U16 data)
{
    reg_symphony_optm_disp_csc_still_coef2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF2;
    d.bitc.still_a_02 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF2, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_still_coef2_still_a_02(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_still_coef2_t *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF2).bitc.still_a_02;
}


/*!
  register SYMPHONY_DISP_CSC_STILL_COEF3 (read/write)
  */
void reg_symphony_optm_disp_set_csc_still_coef3(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF3, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_still_coef3(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF3);
}

void reg_symphony_optm_disp_set_csc_still_coef3_still_a_12(MT_U16 data)
{
    reg_symphony_optm_disp_csc_still_coef3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF3;
    d.bitc.still_a_12 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF3, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_still_coef3_still_a_12(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_still_coef3_t *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF3).bitc.still_a_12;
}

void reg_symphony_optm_disp_set_csc_still_coef3_still_a_11(MT_U16 data)
{
    reg_symphony_optm_disp_csc_still_coef3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF3;
    d.bitc.still_a_11 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF3, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_still_coef3_still_a_11(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_still_coef3_t *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF3).bitc.still_a_11;
}


/*!
  register SYMPHONY_DISP_CSC_STILL_COEF4 (read/write)
  */
void reg_symphony_optm_disp_set_csc_still_coef4(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF4, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_still_coef4(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF4);
}

void reg_symphony_optm_disp_set_csc_still_coef4_still_a_21(MT_U16 data)
{
    reg_symphony_optm_disp_csc_still_coef4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF4;
    d.bitc.still_a_21 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF4, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_still_coef4_still_a_21(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_still_coef4_t *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF4).bitc.still_a_21;
}

void reg_symphony_optm_disp_set_csc_still_coef4_still_a_20(MT_U16 data)
{
    reg_symphony_optm_disp_csc_still_coef4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF4;
    d.bitc.still_a_20 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF4, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_still_coef4_still_a_20(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_still_coef4_t *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF4).bitc.still_a_20;
}


/*!
  register SYMPHONY_DISP_CSC_STILL_COEF5 (read/write)
  */
void reg_symphony_optm_disp_set_csc_still_coef5(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF5, data);
}

MT_U32  reg_symphony_optm_disp_get_csc_still_coef5(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF5);
}

void reg_symphony_optm_disp_set_csc_still_coef5_still_a_22(MT_U16 data)
{
    reg_symphony_optm_disp_csc_still_coef5_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF5;
    d.bitc.still_a_22 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF5, d.all);
}

MT_U16  reg_symphony_optm_disp_get_csc_still_coef5_still_a_22(void)
{
    return (*(volatile reg_symphony_optm_disp_csc_still_coef5_t *)REG_SYMPHONY_OPTM_DISP_CSC_STILL_COEF5).bitc.still_a_22;
}


/*!
  register SYMPHONY_DISP_ROW_JUMP_00 (read/write)
  */
void reg_symphony_optm_disp_set_row_jump_00(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_00, data);
}

MT_U32  reg_symphony_optm_disp_get_row_jump_00(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_00);
}

void reg_symphony_optm_disp_set_row_jump_00_rowjump_00(MT_U32 data)
{
    reg_symphony_optm_disp_row_jump_00_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_00;
    d.bitc.rowjump_00 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_00, d.all);
}

MT_U32  reg_symphony_optm_disp_get_row_jump_00_rowjump_00(void)
{
    return (*(volatile reg_symphony_optm_disp_row_jump_00_t *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_00).bitc.rowjump_00;
}


/*!
  register SYMPHONY_DISP_ROW_JUMP_01 (read/write)
  */
void reg_symphony_optm_disp_set_row_jump_01(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_01, data);
}

MT_U32  reg_symphony_optm_disp_get_row_jump_01(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_01);
}

void reg_symphony_optm_disp_set_row_jump_01_rowjump_01(MT_U32 data)
{
    reg_symphony_optm_disp_row_jump_01_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_01;
    d.bitc.rowjump_01 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_01, d.all);
}

MT_U32  reg_symphony_optm_disp_get_row_jump_01_rowjump_01(void)
{
    return (*(volatile reg_symphony_optm_disp_row_jump_01_t *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_01).bitc.rowjump_01;
}


/*!
  register SYMPHONY_DISP_ROW_JUMP_10 (read/write)
  */
void reg_symphony_optm_disp_set_row_jump_10(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_10, data);
}

MT_U32  reg_symphony_optm_disp_get_row_jump_10(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_10);
}

void reg_symphony_optm_disp_set_row_jump_10_rowjump_10(MT_U32 data)
{
    reg_symphony_optm_disp_row_jump_10_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_10;
    d.bitc.rowjump_10 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_10, d.all);
}

MT_U32  reg_symphony_optm_disp_get_row_jump_10_rowjump_10(void)
{
    return (*(volatile reg_symphony_optm_disp_row_jump_10_t *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_10).bitc.rowjump_10;
}


/*!
  register SYMPHONY_DISP_ROW_JUMP_11 (read/write)
  */
void reg_symphony_optm_disp_set_row_jump_11(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_11, data);
}

MT_U32  reg_symphony_optm_disp_get_row_jump_11(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_11);
}

void reg_symphony_optm_disp_set_row_jump_11_rowjump_11(MT_U32 data)
{
    reg_symphony_optm_disp_row_jump_11_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_11;
    d.bitc.rowjump_11 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_11, d.all);
}

MT_U32  reg_symphony_optm_disp_get_row_jump_11_rowjump_11(void)
{
    return (*(volatile reg_symphony_optm_disp_row_jump_11_t *)REG_SYMPHONY_OPTM_DISP_ROW_JUMP_11).bitc.rowjump_11;
}


/*!
  register SYMPHONY_DISP_DENOISE_DOMAIN_1 (read/write)
  */
void reg_symphony_optm_disp_set_denoise_domain_1(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_1, data);
}

MT_U32  reg_symphony_optm_disp_get_denoise_domain_1(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_1);
}

void reg_symphony_optm_disp_set_denoise_domain_1_domain_c2(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_domain_1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_1;
    d.bitc.domain_c2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_domain_1_domain_c2(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_domain_1_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_1).bitc.domain_c2;
}

void reg_symphony_optm_disp_set_denoise_domain_1_domain_c1(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_domain_1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_1;
    d.bitc.domain_c1 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_domain_1_domain_c1(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_domain_1_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_1).bitc.domain_c1;
}


/*!
  register SYMPHONY_DISP_DENOISE_DOMAIN_2 (read/write)
  */
void reg_symphony_optm_disp_set_denoise_domain_2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_2, data);
}

MT_U32  reg_symphony_optm_disp_get_denoise_domain_2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_2);
}

void reg_symphony_optm_disp_set_denoise_domain_2_domain_c4(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_domain_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_2;
    d.bitc.domain_c4 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_domain_2_domain_c4(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_domain_2_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_2).bitc.domain_c4;
}

void reg_symphony_optm_disp_set_denoise_domain_2_domain_c3(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_domain_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_2;
    d.bitc.domain_c3 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_domain_2_domain_c3(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_domain_2_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_2).bitc.domain_c3;
}


/*!
  register SYMPHONY_DISP_DENOISE_DOMAIN_3 (read/write)
  */
void reg_symphony_optm_disp_set_denoise_domain_3(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_3, data);
}

MT_U32  reg_symphony_optm_disp_get_denoise_domain_3(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_3);
}

void reg_symphony_optm_disp_set_denoise_domain_3_domain_c5(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_domain_3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_3;
    d.bitc.domain_c5 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_domain_3_domain_c5(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_domain_3_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_DOMAIN_3).bitc.domain_c5;
}


/*!
  register SYMPHONY_DISP_DENOISE_RANGE_1 (read/write)
  */
void reg_symphony_optm_disp_set_denoise_range_1(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1, data);
}

MT_U32  reg_symphony_optm_disp_get_denoise_range_1(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1);
}

void reg_symphony_optm_disp_set_denoise_range_1_range_3(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1;
    d.bitc.range_3 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_1_range_3(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_1_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1).bitc.range_3;
}

void reg_symphony_optm_disp_set_denoise_range_1_range_2(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1;
    d.bitc.range_2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_1_range_2(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_1_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1).bitc.range_2;
}

void reg_symphony_optm_disp_set_denoise_range_1_range_1(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1;
    d.bitc.range_1 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_1_range_1(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_1_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1).bitc.range_1;
}

void reg_symphony_optm_disp_set_denoise_range_1_range_0(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1;
    d.bitc.range_0 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_1_range_0(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_1_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_1).bitc.range_0;
}


/*!
  register SYMPHONY_DISP_DENOISE_RANGE_2 (read/write)
  */
void reg_symphony_optm_disp_set_denoise_range_2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2, data);
}

MT_U32  reg_symphony_optm_disp_get_denoise_range_2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2);
}

void reg_symphony_optm_disp_set_denoise_range_2_range_7(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2;
    d.bitc.range_7 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_2_range_7(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_2_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2).bitc.range_7;
}

void reg_symphony_optm_disp_set_denoise_range_2_range_6(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2;
    d.bitc.range_6 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_2_range_6(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_2_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2).bitc.range_6;
}

void reg_symphony_optm_disp_set_denoise_range_2_range_5(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2;
    d.bitc.range_5 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_2_range_5(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_2_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2).bitc.range_5;
}

void reg_symphony_optm_disp_set_denoise_range_2_range_4(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2;
    d.bitc.range_4 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_2_range_4(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_2_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_2).bitc.range_4;
}


/*!
  register SYMPHONY_DISP_DENOISE_RANGE_3 (read/write)
  */
void reg_symphony_optm_disp_set_denoise_range_3(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3, data);
}

MT_U32  reg_symphony_optm_disp_get_denoise_range_3(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3);
}

void reg_symphony_optm_disp_set_denoise_range_3_range_11(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3;
    d.bitc.range_11 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_3_range_11(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_3_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3).bitc.range_11;
}

void reg_symphony_optm_disp_set_denoise_range_3_range_10(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3;
    d.bitc.range_10 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_3_range_10(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_3_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3).bitc.range_10;
}

void reg_symphony_optm_disp_set_denoise_range_3_range_9(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3;
    d.bitc.range_9 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_3_range_9(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_3_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3).bitc.range_9;
}

void reg_symphony_optm_disp_set_denoise_range_3_range_8(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3;
    d.bitc.range_8 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_3_range_8(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_3_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_3).bitc.range_8;
}


/*!
  register SYMPHONY_DISP_DENOISE_RANGE_4 (read/write)
  */
void reg_symphony_optm_disp_set_denoise_range_4(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4, data);
}

MT_U32  reg_symphony_optm_disp_get_denoise_range_4(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4);
}

void reg_symphony_optm_disp_set_denoise_range_4_range_15(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4;
    d.bitc.range_15 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_4_range_15(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_4_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4).bitc.range_15;
}

void reg_symphony_optm_disp_set_denoise_range_4_range_14(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4;
    d.bitc.range_14 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_4_range_14(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_4_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4).bitc.range_14;
}

void reg_symphony_optm_disp_set_denoise_range_4_range_13(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4;
    d.bitc.range_13 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_4_range_13(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_4_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4).bitc.range_13;
}

void reg_symphony_optm_disp_set_denoise_range_4_range_12(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4;
    d.bitc.range_12 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_4_range_12(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_4_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_4).bitc.range_12;
}


/*!
  register SYMPHONY_DISP_DENOISE_RANGE_5 (read/write)
  */
void reg_symphony_optm_disp_set_denoise_range_5(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5, data);
}

MT_U32  reg_symphony_optm_disp_get_denoise_range_5(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5);
}

void reg_symphony_optm_disp_set_denoise_range_5_range_19(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_5_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5;
    d.bitc.range_19 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_5_range_19(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_5_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5).bitc.range_19;
}

void reg_symphony_optm_disp_set_denoise_range_5_range_18(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_5_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5;
    d.bitc.range_18 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_5_range_18(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_5_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5).bitc.range_18;
}

void reg_symphony_optm_disp_set_denoise_range_5_range_17(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_5_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5;
    d.bitc.range_17 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_5_range_17(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_5_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5).bitc.range_17;
}

void reg_symphony_optm_disp_set_denoise_range_5_range_16(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_5_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5;
    d.bitc.range_16 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_5_range_16(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_5_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_5).bitc.range_16;
}


/*!
  register SYMPHONY_DISP_DENOISE_RANGE_6 (read/write)
  */
void reg_symphony_optm_disp_set_denoise_range_6(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_6, data);
}

MT_U32  reg_symphony_optm_disp_get_denoise_range_6(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_6);
}

void reg_symphony_optm_disp_set_denoise_range_6_vid_denoise_en(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_6_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_6;
    d.bitc.vid_denoise_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_6, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_6_vid_denoise_en(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_6_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_6).bitc.vid_denoise_en;
}

void reg_symphony_optm_disp_set_denoise_range_6_z_reg(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_6_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_6;
    d.bitc.z_reg = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_6, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_6_z_reg(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_6_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_6).bitc.z_reg;
}

void reg_symphony_optm_disp_set_denoise_range_6_range_20(MT_U8 data)
{
    reg_symphony_optm_disp_denoise_range_6_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_6;
    d.bitc.range_20 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_6, d.all);
}

MT_U8   reg_symphony_optm_disp_get_denoise_range_6_range_20(void)
{
    return (*(volatile reg_symphony_optm_disp_denoise_range_6_t *)REG_SYMPHONY_OPTM_DISP_DENOISE_RANGE_6).bitc.range_20;
}


/*!
  register SYMPHONY_DISP_SD_WRBACK_ADDR_ODD (read/write)
  */
void reg_symphony_optm_disp_set_sd_wrback_addr_odd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_ADDR_ODD, data);
}

MT_U32  reg_symphony_optm_disp_get_sd_wrback_addr_odd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_ADDR_ODD);
}

void reg_symphony_optm_disp_set_sd_wrback_addr_odd_sd_wrback_addr_odd(MT_U32 data)
{
    reg_symphony_optm_disp_sd_wrback_addr_odd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_ADDR_ODD;
    d.bitc.sd_wrback_addr_odd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_ADDR_ODD, d.all);
}

MT_U32  reg_symphony_optm_disp_get_sd_wrback_addr_odd_sd_wrback_addr_odd(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_wrback_addr_odd_t *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_ADDR_ODD).bitc.sd_wrback_addr_odd;
}


/*!
  register SYMPHONY_DISP_SD_BASE_ADDR_EVEN (read/write)
  */
void reg_symphony_optm_disp_set_sd_base_addr_even(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_BASE_ADDR_EVEN, data);
}

MT_U32  reg_symphony_optm_disp_get_sd_base_addr_even(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_BASE_ADDR_EVEN);
}

void reg_symphony_optm_disp_set_sd_base_addr_even_sd_base_addr_even(MT_U32 data)
{
    reg_symphony_optm_disp_sd_base_addr_even_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_BASE_ADDR_EVEN;
    d.bitc.sd_base_addr_even = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_BASE_ADDR_EVEN, d.all);
}

MT_U32  reg_symphony_optm_disp_get_sd_base_addr_even_sd_base_addr_even(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_base_addr_even_t *)REG_SYMPHONY_OPTM_DISP_SD_BASE_ADDR_EVEN).bitc.sd_base_addr_even;
}


/*!
  register SYMPHONY_DISP_SD_WRBACK_CTRL (read/write)
  */
void reg_symphony_optm_disp_set_sd_wrback_ctrl(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL, data);
}

MT_U32  reg_symphony_optm_disp_get_sd_wrback_ctrl(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL);
}

void reg_symphony_optm_disp_set_sd_wrback_ctrl_sd_start_lines(MT_U16 data)
{
    reg_symphony_optm_disp_sd_wrback_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL;
    d.bitc.sd_start_lines = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL, d.all);
}

MT_U16  reg_symphony_optm_disp_get_sd_wrback_ctrl_sd_start_lines(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_wrback_ctrl_t *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL).bitc.sd_start_lines;
}

void reg_symphony_optm_disp_set_sd_wrback_ctrl_one_field_buffer(MT_U8 data)
{
    reg_symphony_optm_disp_sd_wrback_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL;
    d.bitc.one_field_buffer = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_wrback_ctrl_one_field_buffer(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_wrback_ctrl_t *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL).bitc.one_field_buffer;
}

void reg_symphony_optm_disp_set_sd_wrback_ctrl_sd_buffer_num(MT_U8 data)
{
    reg_symphony_optm_disp_sd_wrback_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL;
    d.bitc.sd_buffer_num = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_wrback_ctrl_sd_buffer_num(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_wrback_ctrl_t *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL).bitc.sd_buffer_num;
}

void reg_symphony_optm_disp_set_sd_wrback_ctrl_sd_blankscreen_mode(MT_U8 data)
{
    reg_symphony_optm_disp_sd_wrback_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL;
    d.bitc.sd_blankscreen_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_wrback_ctrl_sd_blankscreen_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_wrback_ctrl_t *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL).bitc.sd_blankscreen_mode;
}

void reg_symphony_optm_disp_set_sd_wrback_ctrl_sd_softctrl_en(MT_U8 data)
{
    reg_symphony_optm_disp_sd_wrback_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL;
    d.bitc.sd_softctrl_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_wrback_ctrl_sd_softctrl_en(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_wrback_ctrl_t *)REG_SYMPHONY_OPTM_DISP_SD_WRBACK_CTRL).bitc.sd_softctrl_en;
}


/*!
  register SYMPHONY_DISP_SD_BACK_COLOR (read/write)
  */
void reg_symphony_optm_disp_set_sd_back_color(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_BACK_COLOR, data);
}

MT_U32  reg_symphony_optm_disp_get_sd_back_color(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_BACK_COLOR);
}

void reg_symphony_optm_disp_set_sd_back_color_v(MT_U8 data)
{
    reg_symphony_optm_disp_sd_back_color_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_BACK_COLOR;
    d.bitc.v = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_BACK_COLOR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_back_color_v(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_back_color_t *)REG_SYMPHONY_OPTM_DISP_SD_BACK_COLOR).bitc.v;
}

void reg_symphony_optm_disp_set_sd_back_color_u(MT_U8 data)
{
    reg_symphony_optm_disp_sd_back_color_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_BACK_COLOR;
    d.bitc.u = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_BACK_COLOR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_back_color_u(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_back_color_t *)REG_SYMPHONY_OPTM_DISP_SD_BACK_COLOR).bitc.u;
}

void reg_symphony_optm_disp_set_sd_back_color_y(MT_U8 data)
{
    reg_symphony_optm_disp_sd_back_color_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_BACK_COLOR;
    d.bitc.y = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_BACK_COLOR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_back_color_y(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_back_color_t *)REG_SYMPHONY_OPTM_DISP_SD_BACK_COLOR).bitc.y;
}


/*!
  register SYMPHONY_DISP_STILL_UV_START_ADDR_HD (read/write)
  */
void reg_symphony_optm_disp_set_still_uv_start_addr_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_UV_START_ADDR_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_still_uv_start_addr_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_UV_START_ADDR_HD);
}

void reg_symphony_optm_disp_set_still_uv_start_addr_hd_hd_still_uv_start_addr(MT_U32 data)
{
    reg_symphony_optm_disp_still_uv_start_addr_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_UV_START_ADDR_HD;
    d.bitc.hd_still_uv_start_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_UV_START_ADDR_HD, d.all);
}

MT_U32  reg_symphony_optm_disp_get_still_uv_start_addr_hd_hd_still_uv_start_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_still_uv_start_addr_hd_t *)REG_SYMPHONY_OPTM_DISP_STILL_UV_START_ADDR_HD).bitc.hd_still_uv_start_addr;
}


/*!
  register SYMPHONY_DISP_STILL_Y_START_ADDR_HD (read/write)
  */
void reg_symphony_optm_disp_set_still_y_start_addr_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_Y_START_ADDR_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_still_y_start_addr_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_Y_START_ADDR_HD);
}

void reg_symphony_optm_disp_set_still_y_start_addr_hd_hd_still_y_start_addr(MT_U32 data)
{
    reg_symphony_optm_disp_still_y_start_addr_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_Y_START_ADDR_HD;
    d.bitc.hd_still_y_start_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_Y_START_ADDR_HD, d.all);
}

MT_U32  reg_symphony_optm_disp_get_still_y_start_addr_hd_hd_still_y_start_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_still_y_start_addr_hd_t *)REG_SYMPHONY_OPTM_DISP_STILL_Y_START_ADDR_HD).bitc.hd_still_y_start_addr;
}


/*!
  register SYMPHONY_DISP_SUB_START_ADDR_HD (read/write)
  */
void reg_symphony_optm_disp_set_sub_start_addr_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SUB_START_ADDR_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_sub_start_addr_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SUB_START_ADDR_HD);
}

void reg_symphony_optm_disp_set_sub_start_addr_hd_sub_header_addr(MT_U32 data)
{
    reg_symphony_optm_disp_sub_start_addr_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SUB_START_ADDR_HD;
    d.bitc.sub_header_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SUB_START_ADDR_HD, d.all);
}

MT_U32  reg_symphony_optm_disp_get_sub_start_addr_hd_sub_header_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_sub_start_addr_hd_t *)REG_SYMPHONY_OPTM_DISP_SUB_START_ADDR_HD).bitc.sub_header_addr;
}


/*!
  register SYMPHONY_DISP_OSD1_START_ADDR_HD (read/write)
  */
void reg_symphony_optm_disp_set_osd1_start_addr_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD1_START_ADDR_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_osd1_start_addr_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD1_START_ADDR_HD);
}

void reg_symphony_optm_disp_set_osd1_start_addr_hd_osd1_header_addr(MT_U32 data)
{
    reg_symphony_optm_disp_osd1_start_addr_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD1_START_ADDR_HD;
    d.bitc.osd1_header_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD1_START_ADDR_HD, d.all);
}

MT_U32  reg_symphony_optm_disp_get_osd1_start_addr_hd_osd1_header_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_osd1_start_addr_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD1_START_ADDR_HD).bitc.osd1_header_addr;
}


/*!
  register SYMPHONY_DISP_STILL_START_ADDR_SD (read/write)
  */
void reg_symphony_optm_disp_set_still_start_addr_sd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_START_ADDR_SD, data);
}

MT_U32  reg_symphony_optm_disp_get_still_start_addr_sd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_START_ADDR_SD);
}

void reg_symphony_optm_disp_set_still_start_addr_sd_sd_still_start_addr(MT_U32 data)
{
    reg_symphony_optm_disp_still_start_addr_sd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_STILL_START_ADDR_SD;
    d.bitc.sd_still_start_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_STILL_START_ADDR_SD, d.all);
}

MT_U32  reg_symphony_optm_disp_get_still_start_addr_sd_sd_still_start_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_still_start_addr_sd_t *)REG_SYMPHONY_OPTM_DISP_STILL_START_ADDR_SD).bitc.sd_still_start_addr;
}


/*!
  register SYMPHONY_DISP_OSD0_START_ADDR_HD (read/write)
  */
void reg_symphony_optm_disp_set_osd0_start_addr_hd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD0_START_ADDR_HD, data);
}

MT_U32  reg_symphony_optm_disp_get_osd0_start_addr_hd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD0_START_ADDR_HD);
}

void reg_symphony_optm_disp_set_osd0_start_addr_hd_osd0_header_addr(MT_U32 data)
{
    reg_symphony_optm_disp_osd0_start_addr_hd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD0_START_ADDR_HD;
    d.bitc.osd0_header_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD0_START_ADDR_HD, d.all);
}

MT_U32  reg_symphony_optm_disp_get_osd0_start_addr_hd_osd0_header_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_osd0_start_addr_hd_t *)REG_SYMPHONY_OPTM_DISP_OSD0_START_ADDR_HD).bitc.osd0_header_addr;
}


/*!
  register SYMPHONY_DISP_VIDEO_DISPLAY_INFO (read/write)
  */
void reg_symphony_optm_disp_set_video_display_info(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO, data);
}

MT_U32  reg_symphony_optm_disp_get_video_display_info(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO);
}

void reg_symphony_optm_disp_set_video_display_info_firmware_force_di_disable(MT_U8 data)
{
    reg_symphony_optm_disp_video_display_info_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO;
    d.bitc.firmware_force_di_disable = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_display_info_firmware_force_di_disable(void)
{
    return (*(volatile reg_symphony_optm_disp_video_display_info_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO).bitc.firmware_force_di_disable;
}

void reg_symphony_optm_disp_set_video_display_info_progressive_flag(MT_U8 data)
{
    reg_symphony_optm_disp_video_display_info_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO;
    d.bitc.progressive_flag = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_display_info_progressive_flag(void)
{
    return (*(volatile reg_symphony_optm_disp_video_display_info_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO).bitc.progressive_flag;
}

void reg_symphony_optm_disp_set_video_display_info_frame_field_flag(MT_U8 data)
{
    reg_symphony_optm_disp_video_display_info_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO;
    d.bitc.frame_field_flag = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_display_info_frame_field_flag(void)
{
    return (*(volatile reg_symphony_optm_disp_video_display_info_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO).bitc.frame_field_flag;
}

void reg_symphony_optm_disp_set_video_display_info_height_low2bits(MT_U8 data)
{
    reg_symphony_optm_disp_video_display_info_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO;
    d.bitc.height_low2bits = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_display_info_height_low2bits(void)
{
    return (*(volatile reg_symphony_optm_disp_video_display_info_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO).bitc.height_low2bits;
}

void reg_symphony_optm_disp_set_video_display_info_height_mid3bits(MT_U8 data)
{
    reg_symphony_optm_disp_video_display_info_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO;
    d.bitc.height_mid3bits = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_display_info_height_mid3bits(void)
{
    return (*(volatile reg_symphony_optm_disp_video_display_info_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO).bitc.height_mid3bits;
}

void reg_symphony_optm_disp_set_video_display_info_height_high3bits(MT_U8 data)
{
    reg_symphony_optm_disp_video_display_info_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO;
    d.bitc.height_high3bits = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_display_info_height_high3bits(void)
{
    return (*(volatile reg_symphony_optm_disp_video_display_info_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO).bitc.height_high3bits;
}

void reg_symphony_optm_disp_set_video_display_info_width_low2bits(MT_U8 data)
{
    reg_symphony_optm_disp_video_display_info_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO;
    d.bitc.width_low2bits = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_display_info_width_low2bits(void)
{
    return (*(volatile reg_symphony_optm_disp_video_display_info_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO).bitc.width_low2bits;
}

void reg_symphony_optm_disp_set_video_display_info_width_mid3bits(MT_U8 data)
{
    reg_symphony_optm_disp_video_display_info_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO;
    d.bitc.width_mid3bits = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_display_info_width_mid3bits(void)
{
    return (*(volatile reg_symphony_optm_disp_video_display_info_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO).bitc.width_mid3bits;
}

void reg_symphony_optm_disp_set_video_display_info_width_high3bits(MT_U8 data)
{
    reg_symphony_optm_disp_video_display_info_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO;
    d.bitc.width_high3bits = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_display_info_width_high3bits(void)
{
    return (*(volatile reg_symphony_optm_disp_video_display_info_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_DISPLAY_INFO).bitc.width_high3bits;
}


/*!
  register SYMPHONY_DISP_MOTION_PRE_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_motion_pre_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_MOTION_PRE_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_motion_pre_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_MOTION_PRE_ADDR);
}

void reg_symphony_optm_disp_set_motion_pre_addr_motion_pre_addr(MT_U32 data)
{
    reg_symphony_optm_disp_motion_pre_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_MOTION_PRE_ADDR;
    d.bitc.motion_pre_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_MOTION_PRE_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_motion_pre_addr_motion_pre_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_motion_pre_addr_t *)REG_SYMPHONY_OPTM_DISP_MOTION_PRE_ADDR).bitc.motion_pre_addr;
}


/*!
  register SYMPHONY_DISP_MOTION_CUR_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_motion_cur_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_MOTION_CUR_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_motion_cur_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_MOTION_CUR_ADDR);
}

void reg_symphony_optm_disp_set_motion_cur_addr_motion_cur_addr(MT_U32 data)
{
    reg_symphony_optm_disp_motion_cur_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_MOTION_CUR_ADDR;
    d.bitc.motion_cur_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_MOTION_CUR_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_motion_cur_addr_motion_cur_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_motion_cur_addr_t *)REG_SYMPHONY_OPTM_DISP_MOTION_CUR_ADDR).bitc.motion_cur_addr;
}


/*!
  register SYMPHONY_DISP_LUMA_PRE_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_luma_pre_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_PRE_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_luma_pre_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LUMA_PRE_ADDR);
}

void reg_symphony_optm_disp_set_luma_pre_addr_luma_pre_addr(MT_U32 data)
{
    reg_symphony_optm_disp_luma_pre_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LUMA_PRE_ADDR;
    d.bitc.luma_pre_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_PRE_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_luma_pre_addr_luma_pre_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_luma_pre_addr_t *)REG_SYMPHONY_OPTM_DISP_LUMA_PRE_ADDR).bitc.luma_pre_addr;
}


/*!
  register SYMPHONY_DISP_LUMA_CUR_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_luma_cur_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_CUR_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_luma_cur_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LUMA_CUR_ADDR);
}

void reg_symphony_optm_disp_set_luma_cur_addr_luma_cur_addr(MT_U32 data)
{
    reg_symphony_optm_disp_luma_cur_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LUMA_CUR_ADDR;
    d.bitc.luma_cur_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_CUR_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_luma_cur_addr_luma_cur_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_luma_cur_addr_t *)REG_SYMPHONY_OPTM_DISP_LUMA_CUR_ADDR).bitc.luma_cur_addr;
}


/*!
  register SYMPHONY_DISP_LUMA_NEXT_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_luma_next_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_NEXT_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_luma_next_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LUMA_NEXT_ADDR);
}

void reg_symphony_optm_disp_set_luma_next_addr_luma_next_addr(MT_U32 data)
{
    reg_symphony_optm_disp_luma_next_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LUMA_NEXT_ADDR;
    d.bitc.luma_next_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_NEXT_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_luma_next_addr_luma_next_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_luma_next_addr_t *)REG_SYMPHONY_OPTM_DISP_LUMA_NEXT_ADDR).bitc.luma_next_addr;
}


/*!
  register SYMPHONY_DISP_CHROMA_PPRE_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_chroma_ppre_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_PPRE_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_ppre_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_PPRE_ADDR);
}

void reg_symphony_optm_disp_set_chroma_ppre_addr_chroma_ppre_addr(MT_U32 data)
{
    reg_symphony_optm_disp_chroma_ppre_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_PPRE_ADDR;
    d.bitc.chroma_ppre_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_PPRE_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_chroma_ppre_addr_chroma_ppre_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_ppre_addr_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_PPRE_ADDR).bitc.chroma_ppre_addr;
}


/*!
  register SYMPHONY_DISP_CHROMA_PRE_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_chroma_pre_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_PRE_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_pre_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_PRE_ADDR);
}

void reg_symphony_optm_disp_set_chroma_pre_addr_chroma_pre_addr(MT_U32 data)
{
    reg_symphony_optm_disp_chroma_pre_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_PRE_ADDR;
    d.bitc.chroma_pre_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_PRE_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_chroma_pre_addr_chroma_pre_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_pre_addr_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_PRE_ADDR).bitc.chroma_pre_addr;
}


/*!
  register SYMPHONY_DISP_CHROMA_CUR_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_chroma_cur_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_CUR_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_cur_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_CUR_ADDR);
}

void reg_symphony_optm_disp_set_chroma_cur_addr_chroma_cur_addr(MT_U32 data)
{
    reg_symphony_optm_disp_chroma_cur_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_CUR_ADDR;
    d.bitc.chroma_cur_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_CUR_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_chroma_cur_addr_chroma_cur_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_cur_addr_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_CUR_ADDR).bitc.chroma_cur_addr;
}


/*!
  register SYMPHONY_DISP_CHROMA_NEXT_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_chroma_next_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_NEXT_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_next_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_NEXT_ADDR);
}

void reg_symphony_optm_disp_set_chroma_next_addr_chroma_next_addr(MT_U32 data)
{
    reg_symphony_optm_disp_chroma_next_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_NEXT_ADDR;
    d.bitc.chroma_next_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_NEXT_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_chroma_next_addr_chroma_next_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_next_addr_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_NEXT_ADDR).bitc.chroma_next_addr;
}


/*!
  register SYMPHONY_DISP_MOTION_PRE_ADDR2 (read/write)
  */
void reg_symphony_optm_disp_set_motion_pre_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_MOTION_PRE_ADDR2, data);
}

MT_U32  reg_symphony_optm_disp_get_motion_pre_addr2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_MOTION_PRE_ADDR2);
}

void reg_symphony_optm_disp_set_motion_pre_addr2_motion_pre_addr2(MT_U32 data)
{
    reg_symphony_optm_disp_motion_pre_addr2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_MOTION_PRE_ADDR2;
    d.bitc.motion_pre_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_MOTION_PRE_ADDR2, d.all);
}

MT_U32  reg_symphony_optm_disp_get_motion_pre_addr2_motion_pre_addr2(void)
{
    return (*(volatile reg_symphony_optm_disp_motion_pre_addr2_t *)REG_SYMPHONY_OPTM_DISP_MOTION_PRE_ADDR2).bitc.motion_pre_addr2;
}


/*!
  register SYMPHONY_DISP_MOTION_CUR_ADDR2 (read/write)
  */
void reg_symphony_optm_disp_set_motion_cur_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_MOTION_CUR_ADDR2, data);
}

MT_U32  reg_symphony_optm_disp_get_motion_cur_addr2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_MOTION_CUR_ADDR2);
}

void reg_symphony_optm_disp_set_motion_cur_addr2_motion_cur_addr2(MT_U32 data)
{
    reg_symphony_optm_disp_motion_cur_addr2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_MOTION_CUR_ADDR2;
    d.bitc.motion_cur_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_MOTION_CUR_ADDR2, d.all);
}

MT_U32  reg_symphony_optm_disp_get_motion_cur_addr2_motion_cur_addr2(void)
{
    return (*(volatile reg_symphony_optm_disp_motion_cur_addr2_t *)REG_SYMPHONY_OPTM_DISP_MOTION_CUR_ADDR2).bitc.motion_cur_addr2;
}


/*!
  register SYMPHONY_DISP_LUMA_PRE_ADDR2 (read/write)
  */
void reg_symphony_optm_disp_set_luma_pre_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_PRE_ADDR2, data);
}

MT_U32  reg_symphony_optm_disp_get_luma_pre_addr2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LUMA_PRE_ADDR2);
}

void reg_symphony_optm_disp_set_luma_pre_addr2_luma_pre_addr2(MT_U32 data)
{
    reg_symphony_optm_disp_luma_pre_addr2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LUMA_PRE_ADDR2;
    d.bitc.luma_pre_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_PRE_ADDR2, d.all);
}

MT_U32  reg_symphony_optm_disp_get_luma_pre_addr2_luma_pre_addr2(void)
{
    return (*(volatile reg_symphony_optm_disp_luma_pre_addr2_t *)REG_SYMPHONY_OPTM_DISP_LUMA_PRE_ADDR2).bitc.luma_pre_addr2;
}


/*!
  register SYMPHONY_DISP_LUMA_CUR_ADDR2 (read/write)
  */
void reg_symphony_optm_disp_set_luma_cur_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_CUR_ADDR2, data);
}

MT_U32  reg_symphony_optm_disp_get_luma_cur_addr2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LUMA_CUR_ADDR2);
}

void reg_symphony_optm_disp_set_luma_cur_addr2_luma_cur_addr2(MT_U32 data)
{
    reg_symphony_optm_disp_luma_cur_addr2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LUMA_CUR_ADDR2;
    d.bitc.luma_cur_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_CUR_ADDR2, d.all);
}

MT_U32  reg_symphony_optm_disp_get_luma_cur_addr2_luma_cur_addr2(void)
{
    return (*(volatile reg_symphony_optm_disp_luma_cur_addr2_t *)REG_SYMPHONY_OPTM_DISP_LUMA_CUR_ADDR2).bitc.luma_cur_addr2;
}


/*!
  register SYMPHONY_DISP_LUMA_NEXT_ADDR2 (read/write)
  */
void reg_symphony_optm_disp_set_luma_next_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_NEXT_ADDR2, data);
}

MT_U32  reg_symphony_optm_disp_get_luma_next_addr2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LUMA_NEXT_ADDR2);
}

void reg_symphony_optm_disp_set_luma_next_addr2_luma_next_addr2(MT_U32 data)
{
    reg_symphony_optm_disp_luma_next_addr2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_LUMA_NEXT_ADDR2;
    d.bitc.luma_next_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_NEXT_ADDR2, d.all);
}

MT_U32  reg_symphony_optm_disp_get_luma_next_addr2_luma_next_addr2(void)
{
    return (*(volatile reg_symphony_optm_disp_luma_next_addr2_t *)REG_SYMPHONY_OPTM_DISP_LUMA_NEXT_ADDR2).bitc.luma_next_addr2;
}


/*!
  register SYMPHONY_DISP_CHROMA_PPRE_ADDR2 (read/write)
  */
void reg_symphony_optm_disp_set_chroma_ppre_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_PPRE_ADDR2, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_ppre_addr2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_PPRE_ADDR2);
}

void reg_symphony_optm_disp_set_chroma_ppre_addr2_chroma_ppre_addr2(MT_U32 data)
{
    reg_symphony_optm_disp_chroma_ppre_addr2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_PPRE_ADDR2;
    d.bitc.chroma_ppre_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_PPRE_ADDR2, d.all);
}

MT_U32  reg_symphony_optm_disp_get_chroma_ppre_addr2_chroma_ppre_addr2(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_ppre_addr2_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_PPRE_ADDR2).bitc.chroma_ppre_addr2;
}


/*!
  register SYMPHONY_DISP_CHROMA_PRE_ADDR2 (read/write)
  */
void reg_symphony_optm_disp_set_chroma_pre_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_PRE_ADDR2, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_pre_addr2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_PRE_ADDR2);
}

void reg_symphony_optm_disp_set_chroma_pre_addr2_chroma_pre_addr2(MT_U32 data)
{
    reg_symphony_optm_disp_chroma_pre_addr2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_PRE_ADDR2;
    d.bitc.chroma_pre_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_PRE_ADDR2, d.all);
}

MT_U32  reg_symphony_optm_disp_get_chroma_pre_addr2_chroma_pre_addr2(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_pre_addr2_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_PRE_ADDR2).bitc.chroma_pre_addr2;
}


/*!
  register SYMPHONY_DISP_CHROMA_CUR_ADDR2 (read/write)
  */
void reg_symphony_optm_disp_set_chroma_cur_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_CUR_ADDR2, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_cur_addr2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_CUR_ADDR2);
}

void reg_symphony_optm_disp_set_chroma_cur_addr2_chroma_cur_addr2(MT_U32 data)
{
    reg_symphony_optm_disp_chroma_cur_addr2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_CUR_ADDR2;
    d.bitc.chroma_cur_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_CUR_ADDR2, d.all);
}

MT_U32  reg_symphony_optm_disp_get_chroma_cur_addr2_chroma_cur_addr2(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_cur_addr2_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_CUR_ADDR2).bitc.chroma_cur_addr2;
}


/*!
  register SYMPHONY_DISP_CHROMA_NEXT_ADDR2 (read/write)
  */
void reg_symphony_optm_disp_set_chroma_next_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_NEXT_ADDR2, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_next_addr2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_NEXT_ADDR2);
}

void reg_symphony_optm_disp_set_chroma_next_addr2_chroma_next_addr2(MT_U32 data)
{
    reg_symphony_optm_disp_chroma_next_addr2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_NEXT_ADDR2;
    d.bitc.chroma_next_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_NEXT_ADDR2, d.all);
}

MT_U32  reg_symphony_optm_disp_get_chroma_next_addr2_chroma_next_addr2(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_next_addr2_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_NEXT_ADDR2).bitc.chroma_next_addr2;
}


/*!
  register SYMPHONY_DISP_FIELD_PIC_FMT (read/write)
  */
void reg_symphony_optm_disp_set_field_pic_fmt(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_FIELD_PIC_FMT, data);
}

MT_U32  reg_symphony_optm_disp_get_field_pic_fmt(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_FIELD_PIC_FMT);
}

void reg_symphony_optm_disp_set_field_pic_fmt_field_pic_fmt(MT_U8 data)
{
    reg_symphony_optm_disp_field_pic_fmt_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_FIELD_PIC_FMT;
    d.bitc.field_pic_fmt = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_FIELD_PIC_FMT, d.all);
}

MT_U8   reg_symphony_optm_disp_get_field_pic_fmt_field_pic_fmt(void)
{
    return (*(volatile reg_symphony_optm_disp_field_pic_fmt_t *)REG_SYMPHONY_OPTM_DISP_FIELD_PIC_FMT).bitc.field_pic_fmt;
}


/*!
  register SYMPHONY_DISP_VID_HD_VF_COEF_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_vid_hd_vf_coef_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_HD_VF_COEF_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_hd_vf_coef_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_HD_VF_COEF_ADDR);
}

void reg_symphony_optm_disp_set_vid_hd_vf_coef_addr_vid_hd_vf_coef_addr(MT_U32 data)
{
    reg_symphony_optm_disp_vid_hd_vf_coef_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_HD_VF_COEF_ADDR;
    d.bitc.vid_hd_vf_coef_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_HD_VF_COEF_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_vid_hd_vf_coef_addr_vid_hd_vf_coef_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_hd_vf_coef_addr_t *)REG_SYMPHONY_OPTM_DISP_VID_HD_VF_COEF_ADDR).bitc.vid_hd_vf_coef_addr;
}


/*!
  register SYMPHONY_DISP_VID_HD_HF_COEF_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_vid_hd_hf_coef_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_HD_HF_COEF_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_hd_hf_coef_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_HD_HF_COEF_ADDR);
}

void reg_symphony_optm_disp_set_vid_hd_hf_coef_addr_vid_hd_hf_coef_addr(MT_U32 data)
{
    reg_symphony_optm_disp_vid_hd_hf_coef_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_HD_HF_COEF_ADDR;
    d.bitc.vid_hd_hf_coef_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_HD_HF_COEF_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_vid_hd_hf_coef_addr_vid_hd_hf_coef_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_hd_hf_coef_addr_t *)REG_SYMPHONY_OPTM_DISP_VID_HD_HF_COEF_ADDR).bitc.vid_hd_hf_coef_addr;
}


/*!
  register SYMPHONY_DISP_VID_SD_VF_COEF_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_vid_sd_vf_coef_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_SD_VF_COEF_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_sd_vf_coef_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_SD_VF_COEF_ADDR);
}

void reg_symphony_optm_disp_set_vid_sd_vf_coef_addr_vid_sd_vf_coef_addr(MT_U32 data)
{
    reg_symphony_optm_disp_vid_sd_vf_coef_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_SD_VF_COEF_ADDR;
    d.bitc.vid_sd_vf_coef_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_SD_VF_COEF_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_vid_sd_vf_coef_addr_vid_sd_vf_coef_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_sd_vf_coef_addr_t *)REG_SYMPHONY_OPTM_DISP_VID_SD_VF_COEF_ADDR).bitc.vid_sd_vf_coef_addr;
}


/*!
  register SYMPHONY_DISP_VID_SD_HF_COEF_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_vid_sd_hf_coef_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_SD_HF_COEF_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_sd_hf_coef_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_SD_HF_COEF_ADDR);
}

void reg_symphony_optm_disp_set_vid_sd_hf_coef_addr_vid_sd_hf_coef_addr(MT_U32 data)
{
    reg_symphony_optm_disp_vid_sd_hf_coef_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_SD_HF_COEF_ADDR;
    d.bitc.vid_sd_hf_coef_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_SD_HF_COEF_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_vid_sd_hf_coef_addr_vid_sd_hf_coef_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_sd_hf_coef_addr_t *)REG_SYMPHONY_OPTM_DISP_VID_SD_HF_COEF_ADDR).bitc.vid_sd_hf_coef_addr;
}


/*!
  register SYMPHONY_DISP_GRA_VF_COEF_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_gra_vf_coef_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_VF_COEF_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_gra_vf_coef_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_VF_COEF_ADDR);
}

void reg_symphony_optm_disp_set_gra_vf_coef_addr_vf_coef_addr(MT_U32 data)
{
    reg_symphony_optm_disp_gra_vf_coef_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_VF_COEF_ADDR;
    d.bitc.vf_coef_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_VF_COEF_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_gra_vf_coef_addr_vf_coef_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_vf_coef_addr_t *)REG_SYMPHONY_OPTM_DISP_GRA_VF_COEF_ADDR).bitc.vf_coef_addr;
}


/*!
  register SYMPHONY_DISP_GRA_HF_COEF_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_gra_hf_coef_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_HF_COEF_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_gra_hf_coef_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_HF_COEF_ADDR);
}

void reg_symphony_optm_disp_set_gra_hf_coef_addr_hf_coef_addr(MT_U32 data)
{
    reg_symphony_optm_disp_gra_hf_coef_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_GRA_HF_COEF_ADDR;
    d.bitc.hf_coef_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_GRA_HF_COEF_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_gra_hf_coef_addr_hf_coef_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_gra_hf_coef_addr_t *)REG_SYMPHONY_OPTM_DISP_GRA_HF_COEF_ADDR).bitc.hf_coef_addr;
}


/*!
  register SYMPHONY_DISP_VID_DCE_MAP_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_vid_dce_map_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_DCE_MAP_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_vid_dce_map_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_DCE_MAP_ADDR);
}

void reg_symphony_optm_disp_set_vid_dce_map_addr_dce_map_addr(MT_U32 data)
{
    reg_symphony_optm_disp_vid_dce_map_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VID_DCE_MAP_ADDR;
    d.bitc.dce_map_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VID_DCE_MAP_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_vid_dce_map_addr_dce_map_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_vid_dce_map_addr_t *)REG_SYMPHONY_OPTM_DISP_VID_DCE_MAP_ADDR).bitc.dce_map_addr;
}


/*!
  register SYMPHONY_DISP_VSCALER_TABLE_SEL (read/write)
  */
void reg_symphony_optm_disp_set_vscaler_table_sel(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL, data);
}

MT_U32  reg_symphony_optm_disp_get_vscaler_table_sel(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL);
}

void reg_symphony_optm_disp_set_vscaler_table_sel_dce_table_sel(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_table_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL;
    d.bitc.dce_table_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_table_sel_dce_table_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_table_sel_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL).bitc.dce_table_sel;
}

void reg_symphony_optm_disp_set_vscaler_table_sel_osd_hori_table_sel(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_table_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL;
    d.bitc.osd_hori_table_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_table_sel_osd_hori_table_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_table_sel_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL).bitc.osd_hori_table_sel;
}

void reg_symphony_optm_disp_set_vscaler_table_sel_osd_vert_table_sel(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_table_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL;
    d.bitc.osd_vert_table_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_table_sel_osd_vert_table_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_table_sel_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL).bitc.osd_vert_table_sel;
}

void reg_symphony_optm_disp_set_vscaler_table_sel_gra_hori_table_sel(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_table_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL;
    d.bitc.gra_hori_table_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_table_sel_gra_hori_table_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_table_sel_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL).bitc.gra_hori_table_sel;
}

void reg_symphony_optm_disp_set_vscaler_table_sel_gra_vert_table_sel(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_table_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL;
    d.bitc.gra_vert_table_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_table_sel_gra_vert_table_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_table_sel_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL).bitc.gra_vert_table_sel;
}

void reg_symphony_optm_disp_set_vscaler_table_sel_chroma_sd_hori_table_sel(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_table_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL;
    d.bitc.chroma_sd_hori_table_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_table_sel_chroma_sd_hori_table_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_table_sel_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL).bitc.chroma_sd_hori_table_sel;
}

void reg_symphony_optm_disp_set_vscaler_table_sel_chroma_hd_hori_table_sel(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_table_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL;
    d.bitc.chroma_hd_hori_table_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_table_sel_chroma_hd_hori_table_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_table_sel_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL).bitc.chroma_hd_hori_table_sel;
}

void reg_symphony_optm_disp_set_vscaler_table_sel_sd_hori_table_sel(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_table_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL;
    d.bitc.sd_hori_table_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_table_sel_sd_hori_table_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_table_sel_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL).bitc.sd_hori_table_sel;
}

void reg_symphony_optm_disp_set_vscaler_table_sel_sd_vert_table_sel(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_table_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL;
    d.bitc.sd_vert_table_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_table_sel_sd_vert_table_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_table_sel_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL).bitc.sd_vert_table_sel;
}

void reg_symphony_optm_disp_set_vscaler_table_sel_hd_hori_table_sel(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_table_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL;
    d.bitc.hd_hori_table_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_table_sel_hd_hori_table_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_table_sel_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL).bitc.hd_hori_table_sel;
}

void reg_symphony_optm_disp_set_vscaler_table_sel_hd_vert_table_sel(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_table_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL;
    d.bitc.hd_vert_table_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_table_sel_hd_vert_table_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_table_sel_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_TABLE_SEL).bitc.hd_vert_table_sel;
}


/*!
  register SYMPHONY_DISP_OSD_VF_COEF_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_osd_vf_coef_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VF_COEF_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_osd_vf_coef_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VF_COEF_ADDR);
}

void reg_symphony_optm_disp_set_osd_vf_coef_addr_osd_vf_coef_addr(MT_U32 data)
{
    reg_symphony_optm_disp_osd_vf_coef_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_VF_COEF_ADDR;
    d.bitc.osd_vf_coef_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_VF_COEF_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_osd_vf_coef_addr_osd_vf_coef_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_vf_coef_addr_t *)REG_SYMPHONY_OPTM_DISP_OSD_VF_COEF_ADDR).bitc.osd_vf_coef_addr;
}


/*!
  register SYMPHONY_DISP_OSD_HF_COEF_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_osd_hf_coef_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_HF_COEF_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_osd_hf_coef_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_HF_COEF_ADDR);
}

void reg_symphony_optm_disp_set_osd_hf_coef_addr_osd_hf_coef_addr(MT_U32 data)
{
    reg_symphony_optm_disp_osd_hf_coef_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_OSD_HF_COEF_ADDR;
    d.bitc.osd_hf_coef_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_OSD_HF_COEF_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_osd_hf_coef_addr_osd_hf_coef_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_osd_hf_coef_addr_t *)REG_SYMPHONY_OPTM_DISP_OSD_HF_COEF_ADDR).bitc.osd_hf_coef_addr;
}


/*!
  register SYMPHONY_DISP_CHROMA_HD_HF_COEF_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_chroma_hd_hf_coef_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_HD_HF_COEF_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_hd_hf_coef_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_HD_HF_COEF_ADDR);
}

void reg_symphony_optm_disp_set_chroma_hd_hf_coef_addr_chroma_hd_hf_coef_addr(MT_U32 data)
{
    reg_symphony_optm_disp_chroma_hd_hf_coef_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_HD_HF_COEF_ADDR;
    d.bitc.chroma_hd_hf_coef_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_HD_HF_COEF_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_chroma_hd_hf_coef_addr_chroma_hd_hf_coef_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_hd_hf_coef_addr_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_HD_HF_COEF_ADDR).bitc.chroma_hd_hf_coef_addr;
}


/*!
  register SYMPHONY_DISP_CHROMA_SD_HF_COEF_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_chroma_sd_hf_coef_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_SD_HF_COEF_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_chroma_sd_hf_coef_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_SD_HF_COEF_ADDR);
}

void reg_symphony_optm_disp_set_chroma_sd_hf_coef_addr_chroma_sd_hf_coef_addr(MT_U32 data)
{
    reg_symphony_optm_disp_chroma_sd_hf_coef_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_CHROMA_SD_HF_COEF_ADDR;
    d.bitc.chroma_sd_hf_coef_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_SD_HF_COEF_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_chroma_sd_hf_coef_addr_chroma_sd_hf_coef_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_chroma_sd_hf_coef_addr_t *)REG_SYMPHONY_OPTM_DISP_CHROMA_SD_HF_COEF_ADDR).bitc.chroma_sd_hf_coef_addr;
}

//hmccccccccccccccccccccccccccccccccccccccccc
void reg_symphony_optm_disp_set_luma_bot_cur_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_BOT_CUR_ADDR, data);
}

void reg_symphony_optm_disp_set_chroma_bot_cur_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_BOT_CUR_ADDR, data);
}

void reg_symphony_optm_disp_set_luma_bot_cur_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_LUMA_BOT_CUR_ADDR2, data);
}

void reg_symphony_optm_disp_set_chroma_bot_cur_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_CHROMA_BOT_CUR_ADDR2, data);
}




/*!
  register SYMPHONY_DISP_NLMEANS_DENOISE_CTRL (read/write)
  */
void reg_symphony_optm_disp_set_nlmeans_denoise_ctrl(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL, data);
}

MT_U32  reg_symphony_optm_disp_get_nlmeans_denoise_ctrl(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL);
}

void reg_symphony_optm_disp_set_nlmeans_denoise_ctrl_nlmeans_alpha(MT_U16 data)
{
    reg_symphony_optm_disp_nlmeans_denoise_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL;
    d.bitc.nlmeans_alpha = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL, d.all);
}

MT_U16  reg_symphony_optm_disp_get_nlmeans_denoise_ctrl_nlmeans_alpha(void)
{
    return (*(volatile reg_symphony_optm_disp_nlmeans_denoise_ctrl_t *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL).bitc.nlmeans_alpha;
}

void reg_symphony_optm_disp_set_nlmeans_denoise_ctrl_denoise_thrn(MT_U8 data)
{
    reg_symphony_optm_disp_nlmeans_denoise_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL;
    d.bitc.denoise_thrn = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_nlmeans_denoise_ctrl_denoise_thrn(void)
{
    return (*(volatile reg_symphony_optm_disp_nlmeans_denoise_ctrl_t *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL).bitc.denoise_thrn;
}

void reg_symphony_optm_disp_set_nlmeans_denoise_ctrl_denoise_thrb(MT_U8 data)
{
    reg_symphony_optm_disp_nlmeans_denoise_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL;
    d.bitc.denoise_thrb = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_nlmeans_denoise_ctrl_denoise_thrb(void)
{
    return (*(volatile reg_symphony_optm_disp_nlmeans_denoise_ctrl_t *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL).bitc.denoise_thrb;
}

void reg_symphony_optm_disp_set_nlmeans_denoise_ctrl_denoise_thrg(MT_U8 data)
{
    reg_symphony_optm_disp_nlmeans_denoise_ctrl_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL;
    d.bitc.denoise_thrg = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_nlmeans_denoise_ctrl_denoise_thrg(void)
{
    return (*(volatile reg_symphony_optm_disp_nlmeans_denoise_ctrl_t *)REG_SYMPHONY_OPTM_DISP_NLMEANS_DENOISE_CTRL).bitc.denoise_thrg;
}


/*!
  register SYMPHONY_DISP_NLMEANS_PARAMETER_1 (read/write)
  */
void reg_symphony_optm_disp_set_nlmeans_parameter_1(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_1, data);
}

MT_U32  reg_symphony_optm_disp_get_nlmeans_parameter_1(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_1);
}

void reg_symphony_optm_disp_set_nlmeans_parameter_1_reg_a2(MT_U16 data)
{
    reg_symphony_optm_disp_nlmeans_parameter_1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_1;
    d.bitc.reg_a2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_1, d.all);
}

MT_U16  reg_symphony_optm_disp_get_nlmeans_parameter_1_reg_a2(void)
{
    return (*(volatile reg_symphony_optm_disp_nlmeans_parameter_1_t *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_1).bitc.reg_a2;
}

void reg_symphony_optm_disp_set_nlmeans_parameter_1_reg_a1(MT_U16 data)
{
    reg_symphony_optm_disp_nlmeans_parameter_1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_1;
    d.bitc.reg_a1 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_1, d.all);
}

MT_U16  reg_symphony_optm_disp_get_nlmeans_parameter_1_reg_a1(void)
{
    return (*(volatile reg_symphony_optm_disp_nlmeans_parameter_1_t *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_1).bitc.reg_a1;
}


/*!
  register SYMPHONY_DISP_NLMEANS_PARAMETER_2 (read/write)
  */
void reg_symphony_optm_disp_set_nlmeans_parameter_2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_2, data);
}

MT_U32  reg_symphony_optm_disp_get_nlmeans_parameter_2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_2);
}

void reg_symphony_optm_disp_set_nlmeans_parameter_2_reg_b1(MT_U16 data)
{
    reg_symphony_optm_disp_nlmeans_parameter_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_2;
    d.bitc.reg_b1 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_2, d.all);
}

MT_U16  reg_symphony_optm_disp_get_nlmeans_parameter_2_reg_b1(void)
{
    return (*(volatile reg_symphony_optm_disp_nlmeans_parameter_2_t *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_2).bitc.reg_b1;
}

void reg_symphony_optm_disp_set_nlmeans_parameter_2_reg_a3(MT_U16 data)
{
    reg_symphony_optm_disp_nlmeans_parameter_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_2;
    d.bitc.reg_a3 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_2, d.all);
}

MT_U16  reg_symphony_optm_disp_get_nlmeans_parameter_2_reg_a3(void)
{
    return (*(volatile reg_symphony_optm_disp_nlmeans_parameter_2_t *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_2).bitc.reg_a3;
}


/*!
  register SYMPHONY_DISP_NLMEANS_PARAMETER_3 (read/write)
  */
void reg_symphony_optm_disp_set_nlmeans_parameter_3(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_3, data);
}

MT_U32  reg_symphony_optm_disp_get_nlmeans_parameter_3(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_3);
}

void reg_symphony_optm_disp_set_nlmeans_parameter_3_reg_b3(MT_U16 data)
{
    reg_symphony_optm_disp_nlmeans_parameter_3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_3;
    d.bitc.reg_b3 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_3, d.all);
}

MT_U16  reg_symphony_optm_disp_get_nlmeans_parameter_3_reg_b3(void)
{
    return (*(volatile reg_symphony_optm_disp_nlmeans_parameter_3_t *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_3).bitc.reg_b3;
}

void reg_symphony_optm_disp_set_nlmeans_parameter_3_reg_b2(MT_U16 data)
{
    reg_symphony_optm_disp_nlmeans_parameter_3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_3;
    d.bitc.reg_b2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_3, d.all);
}

MT_U16  reg_symphony_optm_disp_get_nlmeans_parameter_3_reg_b2(void)
{
    return (*(volatile reg_symphony_optm_disp_nlmeans_parameter_3_t *)REG_SYMPHONY_OPTM_DISP_NLMEANS_PARAMETER_3).bitc.reg_b2;
}


/*!
  register SYMPHONY_DISP_DI_ENABLE (read/write)
  */
void reg_symphony_optm_disp_set_di_enable(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_ENABLE, data);
}

MT_U32  reg_symphony_optm_disp_get_di_enable(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_ENABLE);
}

void reg_symphony_optm_disp_set_di_enable_enable(MT_U8 data)
{
    reg_symphony_optm_disp_di_enable_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_ENABLE;
    d.bitc.enable = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_ENABLE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_enable_enable(void)
{
    return (*(volatile reg_symphony_optm_disp_di_enable_t *)REG_SYMPHONY_OPTM_DISP_DI_ENABLE).bitc.enable;
}


/*!
  register SYMPHONY_DISP_VIDEO_PDD_EN (read/write)
  */
void reg_symphony_optm_disp_set_video_pdd_en(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_PDD_EN, data);
}

MT_U32  reg_symphony_optm_disp_get_video_pdd_en(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_PDD_EN);
}

void reg_symphony_optm_disp_set_video_pdd_en_di_pdd_en(MT_U8 data)
{
    reg_symphony_optm_disp_video_pdd_en_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_PDD_EN;
    d.bitc.di_pdd_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_PDD_EN, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_pdd_en_di_pdd_en(void)
{
    return (*(volatile reg_symphony_optm_disp_video_pdd_en_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_PDD_EN).bitc.di_pdd_en;
}


/*!
  register SYMPHONY_DISP_VIDEO_IS_MOVIE_TYPE (read/write)
  */
void reg_symphony_optm_disp_set_video_is_movie_type(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_IS_MOVIE_TYPE, data);
}

MT_U32  reg_symphony_optm_disp_get_video_is_movie_type(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_IS_MOVIE_TYPE);
}

void reg_symphony_optm_disp_set_video_is_movie_type_is_movie_type(MT_U8 data)
{
    reg_symphony_optm_disp_video_is_movie_type_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_IS_MOVIE_TYPE;
    d.bitc.is_movie_type = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_IS_MOVIE_TYPE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_is_movie_type_is_movie_type(void)
{
    return (*(volatile reg_symphony_optm_disp_video_is_movie_type_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_IS_MOVIE_TYPE).bitc.is_movie_type;
}


/*!
  register SYMPHONY_DISP_DI_P_OR_N_PAIRED (read/write)
  */
void reg_symphony_optm_disp_set_di_p_or_n_paired(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_P_OR_N_PAIRED, data);
}

MT_U32  reg_symphony_optm_disp_get_di_p_or_n_paired(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_P_OR_N_PAIRED);
}

void reg_symphony_optm_disp_set_di_p_or_n_paired_p_or_n_paired(MT_U8 data)
{
    reg_symphony_optm_disp_di_p_or_n_paired_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_P_OR_N_PAIRED;
    d.bitc.p_or_n_paired = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_P_OR_N_PAIRED, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_p_or_n_paired_p_or_n_paired(void)
{
    return (*(volatile reg_symphony_optm_disp_di_p_or_n_paired_t *)REG_SYMPHONY_OPTM_DISP_DI_P_OR_N_PAIRED).bitc.p_or_n_paired;
}


/*!
  register SYMPHONY_DISP_DI_OPER_MODE (read/write)
  */
void reg_symphony_optm_disp_set_di_oper_mode(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE, data);
}

MT_U32  reg_symphony_optm_disp_get_di_oper_mode(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE);
}

void reg_symphony_optm_disp_set_di_oper_mode_mix_output_mode(MT_U8 data)
{
    reg_symphony_optm_disp_di_oper_mode_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE;
    d.bitc.mix_output_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_oper_mode_mix_output_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_di_oper_mode_t *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE).bitc.mix_output_mode;
}

void reg_symphony_optm_disp_set_di_oper_mode_spatial_ip_mode(MT_U8 data)
{
    reg_symphony_optm_disp_di_oper_mode_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE;
    d.bitc.spatial_ip_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_oper_mode_spatial_ip_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_di_oper_mode_t *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE).bitc.spatial_ip_mode;
}

void reg_symphony_optm_disp_set_di_oper_mode_hori_ip_en(MT_U8 data)
{
    reg_symphony_optm_disp_di_oper_mode_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE;
    d.bitc.hori_ip_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_oper_mode_hori_ip_en(void)
{
    return (*(volatile reg_symphony_optm_disp_di_oper_mode_t *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE).bitc.hori_ip_en;
}

void reg_symphony_optm_disp_set_di_oper_mode_temporal_ip_mode(MT_U8 data)
{
    reg_symphony_optm_disp_di_oper_mode_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE;
    d.bitc.temporal_ip_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_oper_mode_temporal_ip_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_di_oper_mode_t *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE).bitc.temporal_ip_mode;
}

void reg_symphony_optm_disp_set_di_oper_mode_lbam_en(MT_U8 data)
{
    reg_symphony_optm_disp_di_oper_mode_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE;
    d.bitc.lbam_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_oper_mode_lbam_en(void)
{
    return (*(volatile reg_symphony_optm_disp_di_oper_mode_t *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE).bitc.lbam_en;
}

void reg_symphony_optm_disp_set_di_oper_mode_motion_est_mode(MT_U8 data)
{
    reg_symphony_optm_disp_di_oper_mode_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE;
    d.bitc.motion_est_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_oper_mode_motion_est_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_di_oper_mode_t *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE).bitc.motion_est_mode;
}

void reg_symphony_optm_disp_set_di_oper_mode_motion_rd_en(MT_U8 data)
{
    reg_symphony_optm_disp_di_oper_mode_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE;
    d.bitc.motion_rd_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_oper_mode_motion_rd_en(void)
{
    return (*(volatile reg_symphony_optm_disp_di_oper_mode_t *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE).bitc.motion_rd_en;
}

void reg_symphony_optm_disp_set_di_oper_mode_motion_wr_en(MT_U8 data)
{
    reg_symphony_optm_disp_di_oper_mode_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE;
    d.bitc.motion_wr_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_oper_mode_motion_wr_en(void)
{
    return (*(volatile reg_symphony_optm_disp_di_oper_mode_t *)REG_SYMPHONY_OPTM_DISP_DI_OPER_MODE).bitc.motion_wr_en;
}


/*!
  register SYMPHONY_DISP_DI_PARA (read/write)
  */
void reg_symphony_optm_disp_set_di_para(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_PARA, data);
}

MT_U32  reg_symphony_optm_disp_get_di_para(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_PARA);
}

void reg_symphony_optm_disp_set_di_para_g_alpha_k(MT_U8 data)
{
    reg_symphony_optm_disp_di_para_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_PARA;
    d.bitc.g_alpha_k = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_PARA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_para_g_alpha_k(void)
{
    return (*(volatile reg_symphony_optm_disp_di_para_t *)REG_SYMPHONY_OPTM_DISP_DI_PARA).bitc.g_alpha_k;
}

void reg_symphony_optm_disp_set_di_para_g_alpha_0(MT_U8 data)
{
    reg_symphony_optm_disp_di_para_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_PARA;
    d.bitc.g_alpha_0 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_PARA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_para_g_alpha_0(void)
{
    return (*(volatile reg_symphony_optm_disp_di_para_t *)REG_SYMPHONY_OPTM_DISP_DI_PARA).bitc.g_alpha_0;
}

void reg_symphony_optm_disp_set_di_para_p_tl(MT_U8 data)
{
    reg_symphony_optm_disp_di_para_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_PARA;
    d.bitc.p_tl = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_PARA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_para_p_tl(void)
{
    return (*(volatile reg_symphony_optm_disp_di_para_t *)REG_SYMPHONY_OPTM_DISP_DI_PARA).bitc.p_tl;
}


/*!
  register SYMPHONY_DISP_DI_DATA_SFIFO_THR (read/write)
  */
void reg_symphony_optm_disp_set_di_data_sfifo_thr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_DATA_SFIFO_THR, data);
}

MT_U32  reg_symphony_optm_disp_get_di_data_sfifo_thr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_DATA_SFIFO_THR);
}

void reg_symphony_optm_disp_set_di_data_sfifo_thr_di_data_sfifo_thr(MT_U8 data)
{
    reg_symphony_optm_disp_di_data_sfifo_thr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_DATA_SFIFO_THR;
    d.bitc.di_data_sfifo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_DATA_SFIFO_THR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_data_sfifo_thr_di_data_sfifo_thr(void)
{
    return (*(volatile reg_symphony_optm_disp_di_data_sfifo_thr_t *)REG_SYMPHONY_OPTM_DISP_DI_DATA_SFIFO_THR).bitc.di_data_sfifo_thr;
}


/*!
  register SYMPHONY_DISP_VIDEO_SCALER_DATA_SFIFO_THR (read/write)
  */
void reg_symphony_optm_disp_set_video_scaler_data_sfifo_thr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_SCALER_DATA_SFIFO_THR, data);
}

MT_U32  reg_symphony_optm_disp_get_video_scaler_data_sfifo_thr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_SCALER_DATA_SFIFO_THR);
}

void reg_symphony_optm_disp_set_video_scaler_data_sfifo_thr_video_scaler_data_sfifo_thr(MT_U8 data)
{
    reg_symphony_optm_disp_video_scaler_data_sfifo_thr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_SCALER_DATA_SFIFO_THR;
    d.bitc.video_scaler_data_sfifo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_SCALER_DATA_SFIFO_THR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_scaler_data_sfifo_thr_video_scaler_data_sfifo_thr(void)
{
    return (*(volatile reg_symphony_optm_disp_video_scaler_data_sfifo_thr_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_SCALER_DATA_SFIFO_THR).bitc.video_scaler_data_sfifo_thr;
}


/*!
  register SYMPHONY_DISP_DI_LOUT_AFIFO_THR (read/write)
  */
void reg_symphony_optm_disp_set_di_lout_afifo_thr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_LOUT_AFIFO_THR, data);
}

MT_U32  reg_symphony_optm_disp_get_di_lout_afifo_thr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_LOUT_AFIFO_THR);
}

void reg_symphony_optm_disp_set_di_lout_afifo_thr_di_lout_afifo_thr(MT_U8 data)
{
    reg_symphony_optm_disp_di_lout_afifo_thr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_LOUT_AFIFO_THR;
    d.bitc.di_lout_afifo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_LOUT_AFIFO_THR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_lout_afifo_thr_di_lout_afifo_thr(void)
{
    return (*(volatile reg_symphony_optm_disp_di_lout_afifo_thr_t *)REG_SYMPHONY_OPTM_DISP_DI_LOUT_AFIFO_THR).bitc.di_lout_afifo_thr;
}


/*!
  register SYMPHONY_DISP_VSCALER_AXI_CMD_SFIFO_THR (read/write)
  */
void reg_symphony_optm_disp_set_vscaler_axi_cmd_sfifo_thr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_AXI_CMD_SFIFO_THR, data);
}

MT_U32  reg_symphony_optm_disp_get_vscaler_axi_cmd_sfifo_thr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_AXI_CMD_SFIFO_THR);
}

void reg_symphony_optm_disp_set_vscaler_axi_cmd_sfifo_thr_vscaler_axi_cmd_sfifo_thr(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_axi_cmd_sfifo_thr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_AXI_CMD_SFIFO_THR;
    d.bitc.vscaler_axi_cmd_sfifo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_AXI_CMD_SFIFO_THR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_axi_cmd_sfifo_thr_vscaler_axi_cmd_sfifo_thr(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_axi_cmd_sfifo_thr_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_AXI_CMD_SFIFO_THR).bitc.vscaler_axi_cmd_sfifo_thr;
}


/*!
  register SYMPHONY_DISP_VSCALER_AXI_REQ_SFIFO_THR (read/write)
  */
void reg_symphony_optm_disp_set_vscaler_axi_req_sfifo_thr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_AXI_REQ_SFIFO_THR, data);
}

MT_U32  reg_symphony_optm_disp_get_vscaler_axi_req_sfifo_thr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_AXI_REQ_SFIFO_THR);
}

void reg_symphony_optm_disp_set_vscaler_axi_req_sfifo_thr_vscaler_axi_req_sfifo_thr(MT_U8 data)
{
    reg_symphony_optm_disp_vscaler_axi_req_sfifo_thr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VSCALER_AXI_REQ_SFIFO_THR;
    d.bitc.vscaler_axi_req_sfifo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VSCALER_AXI_REQ_SFIFO_THR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_vscaler_axi_req_sfifo_thr_vscaler_axi_req_sfifo_thr(void)
{
    return (*(volatile reg_symphony_optm_disp_vscaler_axi_req_sfifo_thr_t *)REG_SYMPHONY_OPTM_DISP_VSCALER_AXI_REQ_SFIFO_THR).bitc.vscaler_axi_req_sfifo_thr;
}


/*!
  register SYMPHONY_DISP_DI_PDD_NOISE_THR (read/write)
  */
void reg_symphony_optm_disp_set_di_pdd_noise_thr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_PDD_NOISE_THR, data);
}

MT_U32  reg_symphony_optm_disp_get_di_pdd_noise_thr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_PDD_NOISE_THR);
}

void reg_symphony_optm_disp_set_di_pdd_noise_thr_di_pdd_noise_thr(MT_U8 data)
{
    reg_symphony_optm_disp_di_pdd_noise_thr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_PDD_NOISE_THR;
    d.bitc.di_pdd_noise_thr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_PDD_NOISE_THR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_pdd_noise_thr_di_pdd_noise_thr(void)
{
    return (*(volatile reg_symphony_optm_disp_di_pdd_noise_thr_t *)REG_SYMPHONY_OPTM_DISP_DI_PDD_NOISE_THR).bitc.di_pdd_noise_thr;
}


/*!
  register SYMPHONY_DISP_DI_ACC_RESULT_ODD (read/write)
  */
void reg_symphony_optm_disp_set_di_acc_result_odd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_ACC_RESULT_ODD, data);
}

MT_U32  reg_symphony_optm_disp_get_di_acc_result_odd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_ACC_RESULT_ODD);
}

void reg_symphony_optm_disp_set_di_acc_result_odd_di_acc_result_odd(MT_U32 data)
{
    reg_symphony_optm_disp_di_acc_result_odd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_ACC_RESULT_ODD;
    d.bitc.di_acc_result_odd = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_ACC_RESULT_ODD, d.all);
}

MT_U32  reg_symphony_optm_disp_get_di_acc_result_odd_di_acc_result_odd(void)
{
    return (*(volatile reg_symphony_optm_disp_di_acc_result_odd_t *)REG_SYMPHONY_OPTM_DISP_DI_ACC_RESULT_ODD).bitc.di_acc_result_odd;
}


/*!
  register SYMPHONY_DISP_DI_ACC_RESULT_EVEN (read/write)
  */
void reg_symphony_optm_disp_set_di_acc_result_even(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_ACC_RESULT_EVEN, data);
}

MT_U32  reg_symphony_optm_disp_get_di_acc_result_even(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_ACC_RESULT_EVEN);
}

void reg_symphony_optm_disp_set_di_acc_result_even_di_acc_result_even(MT_U32 data)
{
    reg_symphony_optm_disp_di_acc_result_even_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_ACC_RESULT_EVEN;
    d.bitc.di_acc_result_even = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_ACC_RESULT_EVEN, d.all);
}

MT_U32  reg_symphony_optm_disp_get_di_acc_result_even_di_acc_result_even(void)
{
    return (*(volatile reg_symphony_optm_disp_di_acc_result_even_t *)REG_SYMPHONY_OPTM_DISP_DI_ACC_RESULT_EVEN).bitc.di_acc_result_even;
}


/*!
  register SYMPHONY_DISP_DI_PAUSE_EN (read/write)
  */
void reg_symphony_optm_disp_set_di_pause_en(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_PAUSE_EN, data);
}

MT_U32  reg_symphony_optm_disp_get_di_pause_en(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_PAUSE_EN);
}

void reg_symphony_optm_disp_set_di_pause_en_di_pause_en(MT_U8 data)
{
    reg_symphony_optm_disp_di_pause_en_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_PAUSE_EN;
    d.bitc.di_pause_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_PAUSE_EN, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_pause_en_di_pause_en(void)
{
    return (*(volatile reg_symphony_optm_disp_di_pause_en_t *)REG_SYMPHONY_OPTM_DISP_DI_PAUSE_EN).bitc.di_pause_en;
}

void reg_symphony_optm_disp_set_di_pause_en_di_pause_bot_field_flag(MT_U8 data)
{
    reg_symphony_optm_disp_di_pause_en_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_PAUSE_EN;
    d.bitc.di_pause_bot_field_flag = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_PAUSE_EN, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_pause_en_di_pause_bot_field_flag(void)
{
    return (*(volatile reg_symphony_optm_disp_di_pause_en_t *)REG_SYMPHONY_OPTM_DISP_DI_PAUSE_EN).bitc.di_pause_bot_field_flag;
}

void reg_symphony_optm_disp_set_di_pause_en_di_pause_top_field(MT_U8 data)
{
    reg_symphony_optm_disp_di_pause_en_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_PAUSE_EN;
    d.bitc.di_pause_top_field = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_PAUSE_EN, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_pause_en_di_pause_top_field(void)
{
    return (*(volatile reg_symphony_optm_disp_di_pause_en_t *)REG_SYMPHONY_OPTM_DISP_DI_PAUSE_EN).bitc.di_pause_top_field;
}


/*!
  register SYMPHONY_DISP_DI_ALPHA_PARA (read/write)
  */
void reg_symphony_optm_disp_set_di_alpha_para(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA, data);
}

MT_U32  reg_symphony_optm_disp_get_di_alpha_para(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA);
}

void reg_symphony_optm_disp_set_di_alpha_para_g_alpha_0_min(MT_U8 data)
{
    reg_symphony_optm_disp_di_alpha_para_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA;
    d.bitc.g_alpha_0_min = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_alpha_para_g_alpha_0_min(void)
{
    return (*(volatile reg_symphony_optm_disp_di_alpha_para_t *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA).bitc.g_alpha_0_min;
}

void reg_symphony_optm_disp_set_di_alpha_para_g_alpha_0_max(MT_U8 data)
{
    reg_symphony_optm_disp_di_alpha_para_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA;
    d.bitc.g_alpha_0_max = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_alpha_para_g_alpha_0_max(void)
{
    return (*(volatile reg_symphony_optm_disp_di_alpha_para_t *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA).bitc.g_alpha_0_max;
}

void reg_symphony_optm_disp_set_di_alpha_para_luma_diff_k(MT_U8 data)
{
    reg_symphony_optm_disp_di_alpha_para_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA;
    d.bitc.luma_diff_k = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_alpha_para_luma_diff_k(void)
{
    return (*(volatile reg_symphony_optm_disp_di_alpha_para_t *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA).bitc.luma_diff_k;
}

void reg_symphony_optm_disp_set_di_alpha_para_diff_sel(MT_U8 data)
{
    reg_symphony_optm_disp_di_alpha_para_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA;
    d.bitc.diff_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_alpha_para_diff_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_di_alpha_para_t *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA).bitc.diff_sel;
}

void reg_symphony_optm_disp_set_di_alpha_para_new_alpha_en(MT_U8 data)
{
    reg_symphony_optm_disp_di_alpha_para_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA;
    d.bitc.new_alpha_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_alpha_para_new_alpha_en(void)
{
    return (*(volatile reg_symphony_optm_disp_di_alpha_para_t *)REG_SYMPHONY_OPTM_DISP_DI_ALPHA_PARA).bitc.new_alpha_en;
}


/*!
  register SYMPHONY_DISP_DI_MOTION_CTRL_1 (read/write)
  */
void reg_symphony_optm_disp_set_di_motion_ctrl_1(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1, data);
}

MT_U32  reg_symphony_optm_disp_get_di_motion_ctrl_1(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1);
}

void reg_symphony_optm_disp_set_di_motion_ctrl_1_motion_propa_type(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1;
    d.bitc.motion_propa_type = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_1_motion_propa_type(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_1_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1).bitc.motion_propa_type;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_1_motion_damping2(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1;
    d.bitc.motion_damping2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_1_motion_damping2(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_1_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1).bitc.motion_damping2;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_1_motion_damping1(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1;
    d.bitc.motion_damping1 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_1_motion_damping1(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_1_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1).bitc.motion_damping1;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_1_medrsp_thr(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1;
    d.bitc.medrsp_thr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_1_medrsp_thr(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_1_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1).bitc.medrsp_thr;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_1_difdamping(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_1_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1;
    d.bitc.difdamping = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_1_difdamping(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_1_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_1).bitc.difdamping;
}


/*!
  register SYMPHONY_DISP_DI_MOTION_CTRL_2 (read/write)
  */
void reg_symphony_optm_disp_set_di_motion_ctrl_2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2, data);
}

MT_U32  reg_symphony_optm_disp_get_di_motion_ctrl_2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2);
}

void reg_symphony_optm_disp_set_di_motion_ctrl_2_half_motion_en(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2;
    d.bitc.half_motion_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_2_half_motion_en(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_2_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2).bitc.half_motion_en;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_2_motion_data_mode(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2;
    d.bitc.motion_data_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_2_motion_data_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_2_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2).bitc.motion_data_mode;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_2_motion_estmethod(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2;
    d.bitc.motion_estmethod = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_2_motion_estmethod(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_2_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2).bitc.motion_estmethod;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_2_l0l2_motion_mode(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2;
    d.bitc.l0l2_motion_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_2_l0l2_motion_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_2_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2).bitc.l0l2_motion_mode;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_2_ip_smallmotion(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2;
    d.bitc.ip_smallmotion = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_2_ip_smallmotion(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_2_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2).bitc.ip_smallmotion;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_2_ip_average(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2;
    d.bitc.ip_average = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_2_ip_average(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_2_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2).bitc.ip_average;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_2_ip_l0_or_l2(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2;
    d.bitc.ip_l0_or_l2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_2_ip_l0_or_l2(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_2_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_2).bitc.ip_l0_or_l2;
}


/*!
  register SYMPHONY_DISP_DI_HEVC_FLAG (read/write)
  */
void reg_symphony_optm_disp_set_di_hevc_flag(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG, data);
}

MT_U32  reg_symphony_optm_disp_get_di_hevc_flag(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG);
}

void reg_symphony_optm_disp_set_di_hevc_flag_nxt_hevc_flag2(MT_U8 data)
{
    reg_symphony_optm_disp_di_hevc_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG;
    d.bitc.nxt_hevc_flag2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_hevc_flag_nxt_hevc_flag2(void)
{
    return (*(volatile reg_symphony_optm_disp_di_hevc_flag_t *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG).bitc.nxt_hevc_flag2;
}

void reg_symphony_optm_disp_set_di_hevc_flag_cur_hevc_flag2(MT_U8 data)
{
    reg_symphony_optm_disp_di_hevc_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG;
    d.bitc.cur_hevc_flag2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_hevc_flag_cur_hevc_flag2(void)
{
    return (*(volatile reg_symphony_optm_disp_di_hevc_flag_t *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG).bitc.cur_hevc_flag2;
}

void reg_symphony_optm_disp_set_di_hevc_flag_pre_hevc_flag2(MT_U8 data)
{
    reg_symphony_optm_disp_di_hevc_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG;
    d.bitc.pre_hevc_flag2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_hevc_flag_pre_hevc_flag2(void)
{
    return (*(volatile reg_symphony_optm_disp_di_hevc_flag_t *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG).bitc.pre_hevc_flag2;
}

void reg_symphony_optm_disp_set_di_hevc_flag_ppre_hevc_flag2(MT_U8 data)
{
    reg_symphony_optm_disp_di_hevc_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG;
    d.bitc.ppre_hevc_flag2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_hevc_flag_ppre_hevc_flag2(void)
{
    return (*(volatile reg_symphony_optm_disp_di_hevc_flag_t *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG).bitc.ppre_hevc_flag2;
}

void reg_symphony_optm_disp_set_di_hevc_flag_nxt_hevc_flag(MT_U8 data)
{
    reg_symphony_optm_disp_di_hevc_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG;
    d.bitc.nxt_hevc_flag = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_hevc_flag_nxt_hevc_flag(void)
{
    return (*(volatile reg_symphony_optm_disp_di_hevc_flag_t *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG).bitc.nxt_hevc_flag;
}

void reg_symphony_optm_disp_set_di_hevc_flag_cur_hevc_flag(MT_U8 data)
{
    reg_symphony_optm_disp_di_hevc_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG;
    d.bitc.cur_hevc_flag = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_hevc_flag_cur_hevc_flag(void)
{
    return (*(volatile reg_symphony_optm_disp_di_hevc_flag_t *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG).bitc.cur_hevc_flag;
}

void reg_symphony_optm_disp_set_di_hevc_flag_pre_hevc_flag(MT_U8 data)
{
    reg_symphony_optm_disp_di_hevc_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG;
    d.bitc.pre_hevc_flag = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_hevc_flag_pre_hevc_flag(void)
{
    return (*(volatile reg_symphony_optm_disp_di_hevc_flag_t *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG).bitc.pre_hevc_flag;
}

void reg_symphony_optm_disp_set_di_hevc_flag_ppre_hevc_flag(MT_U8 data)
{
    reg_symphony_optm_disp_di_hevc_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG;
    d.bitc.ppre_hevc_flag = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_hevc_flag_ppre_hevc_flag(void)
{
    return (*(volatile reg_symphony_optm_disp_di_hevc_flag_t *)REG_SYMPHONY_OPTM_DISP_DI_HEVC_FLAG).bitc.ppre_hevc_flag;
}


/*!
  register SYMPHONY_DISP_NONE_DI_FIELDS_FLAG (read/write)
  */
void reg_symphony_optm_disp_set_none_di_fields_flag(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG, data);
}

MT_U32  reg_symphony_optm_disp_get_none_di_fields_flag(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG);
}

void reg_symphony_optm_disp_set_none_di_fields_flag_cur_bot_field_flag_2(MT_U8 data)
{
    reg_symphony_optm_disp_none_di_fields_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG;
    d.bitc.cur_bot_field_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_none_di_fields_flag_cur_bot_field_flag_2(void)
{
    return (*(volatile reg_symphony_optm_disp_none_di_fields_flag_t *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG).bitc.cur_bot_field_flag_2;
}

void reg_symphony_optm_disp_set_none_di_fields_flag_cur_top_field_flag_2(MT_U8 data)
{
    reg_symphony_optm_disp_none_di_fields_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG;
    d.bitc.cur_top_field_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_none_di_fields_flag_cur_top_field_flag_2(void)
{
    return (*(volatile reg_symphony_optm_disp_none_di_fields_flag_t *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG).bitc.cur_top_field_flag_2;
}

void reg_symphony_optm_disp_set_none_di_fields_flag_cur_bot_field_flag_0(MT_U8 data)
{
    reg_symphony_optm_disp_none_di_fields_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG;
    d.bitc.cur_bot_field_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_none_di_fields_flag_cur_bot_field_flag_0(void)
{
    return (*(volatile reg_symphony_optm_disp_none_di_fields_flag_t *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG).bitc.cur_bot_field_flag_0;
}

void reg_symphony_optm_disp_set_none_di_fields_flag_cur_top_field_flag_0(MT_U8 data)
{
    reg_symphony_optm_disp_none_di_fields_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG;
    d.bitc.cur_top_field_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_none_di_fields_flag_cur_top_field_flag_0(void)
{
    return (*(volatile reg_symphony_optm_disp_none_di_fields_flag_t *)REG_SYMPHONY_OPTM_DISP_NONE_DI_FIELDS_FLAG).bitc.cur_top_field_flag_0;
}


/*!
  register SYMPHONY_DISP_NONE_DI_HEVC_FLAG (read/write)
  */
void reg_symphony_optm_disp_set_none_di_hevc_flag(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG, data);
}

MT_U32  reg_symphony_optm_disp_get_none_di_hevc_flag(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG);
}

void reg_symphony_optm_disp_set_none_di_hevc_flag_cur_bot_hevc_flag_2(MT_U8 data)
{
    reg_symphony_optm_disp_none_di_hevc_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG;
    d.bitc.cur_bot_hevc_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_none_di_hevc_flag_cur_bot_hevc_flag_2(void)
{
    return (*(volatile reg_symphony_optm_disp_none_di_hevc_flag_t *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG).bitc.cur_bot_hevc_flag_2;
}

void reg_symphony_optm_disp_set_none_di_hevc_flag_cur_top_hevc_flag_2(MT_U8 data)
{
    reg_symphony_optm_disp_none_di_hevc_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG;
    d.bitc.cur_top_hevc_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_none_di_hevc_flag_cur_top_hevc_flag_2(void)
{
    return (*(volatile reg_symphony_optm_disp_none_di_hevc_flag_t *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG).bitc.cur_top_hevc_flag_2;
}

void reg_symphony_optm_disp_set_none_di_hevc_flag_cur_bot_hevc_flag_0(MT_U8 data)
{
    reg_symphony_optm_disp_none_di_hevc_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG;
    d.bitc.cur_bot_hevc_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_none_di_hevc_flag_cur_bot_hevc_flag_0(void)
{
    return (*(volatile reg_symphony_optm_disp_none_di_hevc_flag_t *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG).bitc.cur_bot_hevc_flag_0;
}

void reg_symphony_optm_disp_set_none_di_hevc_flag_cur_top_hevc_flag_0(MT_U8 data)
{
    reg_symphony_optm_disp_none_di_hevc_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG;
    d.bitc.cur_top_hevc_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_none_di_hevc_flag_cur_top_hevc_flag_0(void)
{
    return (*(volatile reg_symphony_optm_disp_none_di_hevc_flag_t *)REG_SYMPHONY_OPTM_DISP_NONE_DI_HEVC_FLAG).bitc.cur_top_hevc_flag_0;
}


/*!
  register SYMPHONY_DISP_NONE_DI_PROGRESSIVE_FLAG (read/write)
  */
void reg_symphony_optm_disp_set_none_di_progressive_flag(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_PROGRESSIVE_FLAG, data);
}

MT_U32  reg_symphony_optm_disp_get_none_di_progressive_flag(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_PROGRESSIVE_FLAG);
}

void reg_symphony_optm_disp_set_none_di_progressive_flag_progressive_flag_2(MT_U8 data)
{
    reg_symphony_optm_disp_none_di_progressive_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_PROGRESSIVE_FLAG;
    d.bitc.progressive_flag_2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_PROGRESSIVE_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_none_di_progressive_flag_progressive_flag_2(void)
{
    return (*(volatile reg_symphony_optm_disp_none_di_progressive_flag_t *)REG_SYMPHONY_OPTM_DISP_NONE_DI_PROGRESSIVE_FLAG).bitc.progressive_flag_2;
}

void reg_symphony_optm_disp_set_none_di_progressive_flag_progressive_flag_0(MT_U8 data)
{
    reg_symphony_optm_disp_none_di_progressive_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_PROGRESSIVE_FLAG;
    d.bitc.progressive_flag_0 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_PROGRESSIVE_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_none_di_progressive_flag_progressive_flag_0(void)
{
    return (*(volatile reg_symphony_optm_disp_none_di_progressive_flag_t *)REG_SYMPHONY_OPTM_DISP_NONE_DI_PROGRESSIVE_FLAG).bitc.progressive_flag_0;
}

void reg_symphony_optm_disp_set_none_di_progressive_flag_only_use_set_0_en(MT_U8 data)
{
    reg_symphony_optm_disp_none_di_progressive_flag_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_NONE_DI_PROGRESSIVE_FLAG;
    d.bitc.only_use_set_0_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_NONE_DI_PROGRESSIVE_FLAG, d.all);
}

MT_U8   reg_symphony_optm_disp_get_none_di_progressive_flag_only_use_set_0_en(void)
{
    return (*(volatile reg_symphony_optm_disp_none_di_progressive_flag_t *)REG_SYMPHONY_OPTM_DISP_NONE_DI_PROGRESSIVE_FLAG).bitc.only_use_set_0_en;
}


/*!
  register SYMPHONY_DISP_DI_MOTION_CTRL_3 (read/write)
  */
void reg_symphony_optm_disp_set_di_motion_ctrl_3(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3, data);
}

MT_U32  reg_symphony_optm_disp_get_di_motion_ctrl_3(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3);
}

void reg_symphony_optm_disp_set_di_motion_ctrl_3_new_algorithm_en(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3;
    d.bitc.new_algorithm_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_3_new_algorithm_en(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_3_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3).bitc.new_algorithm_en;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_3_small_motion_magnify(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3;
    d.bitc.small_motion_magnify = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_3_small_motion_magnify(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_3_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3).bitc.small_motion_magnify;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_3_small_motion_thr2(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3;
    d.bitc.small_motion_thr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_3_small_motion_thr2(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_3_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3).bitc.small_motion_thr2;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_3_small_motion_thr1(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3;
    d.bitc.small_motion_thr1 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_3_small_motion_thr1(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_3_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3).bitc.small_motion_thr1;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_3_medrsp7dir_thr(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_3_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3;
    d.bitc.medrsp7dir_thr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_3_medrsp7dir_thr(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_3_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_3).bitc.medrsp7dir_thr;
}


/*!
  register SYMPHONY_DISP_DI_MOTION_CTRL_4 (read/write)
  */
void reg_symphony_optm_disp_set_di_motion_ctrl_4(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4, data);
}

MT_U32  reg_symphony_optm_disp_get_di_motion_ctrl_4(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4);
}

void reg_symphony_optm_disp_set_di_motion_ctrl_4_uv_motion_gain(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4;
    d.bitc.uv_motion_gain = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_4_uv_motion_gain(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_4_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4).bitc.uv_motion_gain;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_4_ip_l0andl2(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4;
    d.bitc.ip_l0andl2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_4_ip_l0andl2(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_4_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4).bitc.ip_l0andl2;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_4_ip_l1_difthr(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4;
    d.bitc.ip_l1_difthr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_4_ip_l1_difthr(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_4_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4).bitc.ip_l1_difthr;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_4_preserve_all_motion(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4;
    d.bitc.preserve_all_motion = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_4_preserve_all_motion(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_4_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4).bitc.preserve_all_motion;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_4_preserve_small_motion(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4;
    d.bitc.preserve_small_motion = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_4_preserve_small_motion(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_4_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4).bitc.preserve_small_motion;
}

void reg_symphony_optm_disp_set_di_motion_ctrl_4_motion_magnify(MT_U8 data)
{
    reg_symphony_optm_disp_di_motion_ctrl_4_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4;
    d.bitc.motion_magnify = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_motion_ctrl_4_motion_magnify(void)
{
    return (*(volatile reg_symphony_optm_disp_di_motion_ctrl_4_t *)REG_SYMPHONY_OPTM_DISP_DI_MOTION_CTRL_4).bitc.motion_magnify;
}


/*!
  register SYMPHONY_DISP_DI_CHROMA_PARA (read/write)
  */
void reg_symphony_optm_disp_set_di_chroma_para(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_CHROMA_PARA, data);
}

MT_U32  reg_symphony_optm_disp_get_di_chroma_para(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_CHROMA_PARA);
}

void reg_symphony_optm_disp_set_di_chroma_para_g_alpha_k_chroma(MT_U8 data)
{
    reg_symphony_optm_disp_di_chroma_para_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_CHROMA_PARA;
    d.bitc.g_alpha_k_chroma = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_CHROMA_PARA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_chroma_para_g_alpha_k_chroma(void)
{
    return (*(volatile reg_symphony_optm_disp_di_chroma_para_t *)REG_SYMPHONY_OPTM_DISP_DI_CHROMA_PARA).bitc.g_alpha_k_chroma;
}

void reg_symphony_optm_disp_set_di_chroma_para_g_alpha_0_chroma(MT_U8 data)
{
    reg_symphony_optm_disp_di_chroma_para_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DI_CHROMA_PARA;
    d.bitc.g_alpha_0_chroma = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DI_CHROMA_PARA, d.all);
}

MT_U8   reg_symphony_optm_disp_get_di_chroma_para_g_alpha_0_chroma(void)
{
    return (*(volatile reg_symphony_optm_disp_di_chroma_para_t *)REG_SYMPHONY_OPTM_DISP_DI_CHROMA_PARA).bitc.g_alpha_0_chroma;
}


/*!
  register SYMPHONY_DISP_VIDEO_BURST_LENGTH_SEL (read/write)
  */
void reg_symphony_optm_disp_set_video_burst_length_sel(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL, data);
}

MT_U32  reg_symphony_optm_disp_get_video_burst_length_sel(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL);
}

void reg_symphony_optm_disp_set_video_burst_length_sel_vid_burst_length(MT_U8 data)
{
    reg_symphony_optm_disp_video_burst_length_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL;
    d.bitc.vid_burst_length = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_burst_length_sel_vid_burst_length(void)
{
    return (*(volatile reg_symphony_optm_disp_video_burst_length_sel_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL).bitc.vid_burst_length;
}

void reg_symphony_optm_disp_set_video_burst_length_sel_vid_rd_stride_sel(MT_U8 data)
{
    reg_symphony_optm_disp_video_burst_length_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL;
    d.bitc.vid_rd_stride_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_burst_length_sel_vid_rd_stride_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_video_burst_length_sel_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL).bitc.vid_rd_stride_sel;
}

void reg_symphony_optm_disp_set_video_burst_length_sel_vid_linear_addr_en(MT_U8 data)
{
    reg_symphony_optm_disp_video_burst_length_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL;
    d.bitc.vid_linear_addr_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_burst_length_sel_vid_linear_addr_en(void)
{
    return (*(volatile reg_symphony_optm_disp_video_burst_length_sel_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL).bitc.vid_linear_addr_en;
}

void reg_symphony_optm_disp_set_video_burst_length_sel_v_half_en(MT_U8 data)
{
    reg_symphony_optm_disp_video_burst_length_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL;
    d.bitc.v_half_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_burst_length_sel_v_half_en(void)
{
    return (*(volatile reg_symphony_optm_disp_video_burst_length_sel_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL).bitc.v_half_en;
}

void reg_symphony_optm_disp_set_video_burst_length_sel_h_half_en(MT_U8 data)
{
    reg_symphony_optm_disp_video_burst_length_sel_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL;
    d.bitc.h_half_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_burst_length_sel_h_half_en(void)
{
    return (*(volatile reg_symphony_optm_disp_video_burst_length_sel_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_LENGTH_SEL).bitc.h_half_en;
}


/*!
  register SYMPHONY_DISP_VIDEO_BURST_INFO (read/write)
  */
void reg_symphony_optm_disp_set_video_burst_info(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO, data);
}

MT_U32  reg_symphony_optm_disp_get_video_burst_info(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO);
}

void reg_symphony_optm_disp_set_video_burst_info_last_burst_length(MT_U8 data)
{
    reg_symphony_optm_disp_video_burst_info_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO;
    d.bitc.last_burst_length = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_burst_info_last_burst_length(void)
{
    return (*(volatile reg_symphony_optm_disp_video_burst_info_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO).bitc.last_burst_length;
}

void reg_symphony_optm_disp_set_video_burst_info_first_burst_length(MT_U8 data)
{
    reg_symphony_optm_disp_video_burst_info_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO;
    d.bitc.first_burst_length = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_burst_info_first_burst_length(void)
{
    return (*(volatile reg_symphony_optm_disp_video_burst_info_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO).bitc.first_burst_length;
}

void reg_symphony_optm_disp_set_video_burst_info_line_rd_max(MT_U8 data)
{
    reg_symphony_optm_disp_video_burst_info_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO;
    d.bitc.line_rd_max = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_burst_info_line_rd_max(void)
{
    return (*(volatile reg_symphony_optm_disp_video_burst_info_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO).bitc.line_rd_max;
}

void reg_symphony_optm_disp_set_video_burst_info_burst_info_en(MT_U8 data)
{
    reg_symphony_optm_disp_video_burst_info_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO;
    d.bitc.burst_info_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_burst_info_burst_info_en(void)
{
    return (*(volatile reg_symphony_optm_disp_video_burst_info_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_BURST_INFO).bitc.burst_info_en;
}


/*!
  register SYMPHONY_DISP_VIDEO_LINE_RD_CNT_MAX (read/write)
  */
void reg_symphony_optm_disp_set_video_line_rd_cnt_max(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_LINE_RD_CNT_MAX, data);
}

MT_U32  reg_symphony_optm_disp_get_video_line_rd_cnt_max(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_LINE_RD_CNT_MAX);
}

void reg_symphony_optm_disp_set_video_line_rd_cnt_max_line_cnt_max(MT_U8 data)
{
    reg_symphony_optm_disp_video_line_rd_cnt_max_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_VIDEO_LINE_RD_CNT_MAX;
    d.bitc.line_cnt_max = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_VIDEO_LINE_RD_CNT_MAX, d.all);
}

MT_U8   reg_symphony_optm_disp_get_video_line_rd_cnt_max_line_cnt_max(void)
{
    return (*(volatile reg_symphony_optm_disp_video_line_rd_cnt_max_t *)REG_SYMPHONY_OPTM_DISP_VIDEO_LINE_RD_CNT_MAX).bitc.line_cnt_max;
}


/*!
  register SYMPHONY_DISP_PRESCALE_CMD (read/write)
  */
void reg_symphony_optm_disp_set_prescale_cmd(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD, data);
}

MT_U32  reg_symphony_optm_disp_get_prescale_cmd(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD);
}

void reg_symphony_optm_disp_set_prescale_cmd_prescale_en(MT_U8 data)
{
    reg_symphony_optm_disp_prescale_cmd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD;
    d.bitc.prescale_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_prescale_cmd_prescale_en(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_cmd_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD).bitc.prescale_en;
}

void reg_symphony_optm_disp_set_prescale_cmd_prescale_field_mode(MT_U8 data)
{
    reg_symphony_optm_disp_prescale_cmd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD;
    d.bitc.prescale_field_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_prescale_cmd_prescale_field_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_cmd_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD).bitc.prescale_field_mode;
}

void reg_symphony_optm_disp_set_prescale_cmd_prescale_hscale_mode(MT_U8 data)
{
    reg_symphony_optm_disp_prescale_cmd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD;
    d.bitc.prescale_hscale_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_prescale_cmd_prescale_hscale_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_cmd_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD).bitc.prescale_hscale_mode;
}

void reg_symphony_optm_disp_set_prescale_cmd_prescale_vscale_mode(MT_U8 data)
{
    reg_symphony_optm_disp_prescale_cmd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD;
    d.bitc.prescale_vscale_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_prescale_cmd_prescale_vscale_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_cmd_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD).bitc.prescale_vscale_mode;
}

void reg_symphony_optm_disp_set_prescale_cmd_prescale_wr_stride_sel(MT_U8 data)
{
    reg_symphony_optm_disp_prescale_cmd_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD;
    d.bitc.prescale_wr_stride_sel = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD, d.all);
}

MT_U8   reg_symphony_optm_disp_get_prescale_cmd_prescale_wr_stride_sel(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_cmd_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CMD).bitc.prescale_wr_stride_sel;
}


/*!
  register SYMPHONY_DISP_PRESCALE_FRAME_SIZE (read/write)
  */
void reg_symphony_optm_disp_set_prescale_frame_size(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FRAME_SIZE, data);
}

MT_U32  reg_symphony_optm_disp_get_prescale_frame_size(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FRAME_SIZE);
}

void reg_symphony_optm_disp_set_prescale_frame_size_prescale_frame_height(MT_U16 data)
{
    reg_symphony_optm_disp_prescale_frame_size_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FRAME_SIZE;
    d.bitc.prescale_frame_height = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FRAME_SIZE, d.all);
}

MT_U16  reg_symphony_optm_disp_get_prescale_frame_size_prescale_frame_height(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_frame_size_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FRAME_SIZE).bitc.prescale_frame_height;
}

void reg_symphony_optm_disp_set_prescale_frame_size_prescale_frame_width(MT_U16 data)
{
    reg_symphony_optm_disp_prescale_frame_size_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FRAME_SIZE;
    d.bitc.prescale_frame_width = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FRAME_SIZE, d.all);
}

MT_U16  reg_symphony_optm_disp_get_prescale_frame_size_prescale_frame_width(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_frame_size_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FRAME_SIZE).bitc.prescale_frame_width;
}


/*!
  register SYMPHONY_DISP_PRESCALE_TILING_PARAMS (read/write)
  */
void reg_symphony_optm_disp_set_prescale_tiling_params(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_TILING_PARAMS, data);
}

MT_U32  reg_symphony_optm_disp_get_prescale_tiling_params(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_TILING_PARAMS);
}

void reg_symphony_optm_disp_set_prescale_tiling_params_prescale_field_pic(MT_U8 data)
{
    reg_symphony_optm_disp_prescale_tiling_params_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_TILING_PARAMS;
    d.bitc.prescale_field_pic = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_TILING_PARAMS, d.all);
}

MT_U8   reg_symphony_optm_disp_get_prescale_tiling_params_prescale_field_pic(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_tiling_params_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_TILING_PARAMS).bitc.prescale_field_pic;
}

void reg_symphony_optm_disp_set_prescale_tiling_params_prescale_col_size_mode(MT_U8 data)
{
    reg_symphony_optm_disp_prescale_tiling_params_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_TILING_PARAMS;
    d.bitc.prescale_col_size_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_TILING_PARAMS, d.all);
}

MT_U8   reg_symphony_optm_disp_get_prescale_tiling_params_prescale_col_size_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_tiling_params_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_TILING_PARAMS).bitc.prescale_col_size_mode;
}

void reg_symphony_optm_disp_set_prescale_tiling_params_prescale_hd_map_mode(MT_U8 data)
{
    reg_symphony_optm_disp_prescale_tiling_params_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_TILING_PARAMS;
    d.bitc.prescale_hd_map_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_TILING_PARAMS, d.all);
}

MT_U8   reg_symphony_optm_disp_get_prescale_tiling_params_prescale_hd_map_mode(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_tiling_params_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_TILING_PARAMS).bitc.prescale_hd_map_mode;
}


/*!
  register SYMPHONY_DISP_PRESCALE_LUMA_RD_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_prescale_luma_rd_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_RD_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_prescale_luma_rd_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_RD_ADDR);
}

void reg_symphony_optm_disp_set_prescale_luma_rd_addr_luma_rd_addr(MT_U32 data)
{
    reg_symphony_optm_disp_prescale_luma_rd_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_RD_ADDR;
    d.bitc.luma_rd_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_RD_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_prescale_luma_rd_addr_luma_rd_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_luma_rd_addr_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_RD_ADDR).bitc.luma_rd_addr;
}


/*!
  register SYMPHONY_DISP_PRESCALE_LUMA_RD_ADDR2 (read/write)
  */
void reg_symphony_optm_disp_set_prescale_luma_rd_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_RD_ADDR2, data);
}

MT_U32  reg_symphony_optm_disp_get_prescale_luma_rd_addr2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_RD_ADDR2);
}

void reg_symphony_optm_disp_set_prescale_luma_rd_addr2_luma_rd_addr2(MT_U32 data)
{
    reg_symphony_optm_disp_prescale_luma_rd_addr2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_RD_ADDR2;
    d.bitc.luma_rd_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_RD_ADDR2, d.all);
}

MT_U32  reg_symphony_optm_disp_get_prescale_luma_rd_addr2_luma_rd_addr2(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_luma_rd_addr2_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_RD_ADDR2).bitc.luma_rd_addr2;
}


/*!
  register SYMPHONY_DISP_PRESCALE_LUMA_WR_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_prescale_luma_wr_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_WR_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_prescale_luma_wr_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_WR_ADDR);
}

void reg_symphony_optm_disp_set_prescale_luma_wr_addr_luma_wr_addr(MT_U32 data)
{
    reg_symphony_optm_disp_prescale_luma_wr_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_WR_ADDR;
    d.bitc.luma_wr_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_WR_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_prescale_luma_wr_addr_luma_wr_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_luma_wr_addr_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_WR_ADDR).bitc.luma_wr_addr;
}


/*!
  register SYMPHONY_DISP_PRESCALE_LUMA_WR_ADDR2 (read/write)
  */
void reg_symphony_optm_disp_set_prescale_luma_wr_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_WR_ADDR2, data);
}

MT_U32  reg_symphony_optm_disp_get_prescale_luma_wr_addr2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_WR_ADDR2);
}

void reg_symphony_optm_disp_set_prescale_luma_wr_addr2_luma_wr_addr2(MT_U32 data)
{
    reg_symphony_optm_disp_prescale_luma_wr_addr2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_WR_ADDR2;
    d.bitc.luma_wr_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_WR_ADDR2, d.all);
}

MT_U32  reg_symphony_optm_disp_get_prescale_luma_wr_addr2_luma_wr_addr2(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_luma_wr_addr2_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_LUMA_WR_ADDR2).bitc.luma_wr_addr2;
}


/*!
  register SYMPHONY_DISP_PRESCALE_CHROMA_RD_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_prescale_chroma_rd_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_RD_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_prescale_chroma_rd_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_RD_ADDR);
}

void reg_symphony_optm_disp_set_prescale_chroma_rd_addr_chroma_rd_addr(MT_U32 data)
{
    reg_symphony_optm_disp_prescale_chroma_rd_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_RD_ADDR;
    d.bitc.chroma_rd_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_RD_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_prescale_chroma_rd_addr_chroma_rd_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_chroma_rd_addr_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_RD_ADDR).bitc.chroma_rd_addr;
}


/*!
  register SYMPHONY_DISP_PRESCALE_CHROMA_RD_ADDR2 (read/write)
  */
void reg_symphony_optm_disp_set_prescale_chroma_rd_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_RD_ADDR2, data);
}

MT_U32  reg_symphony_optm_disp_get_prescale_chroma_rd_addr2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_RD_ADDR2);
}

void reg_symphony_optm_disp_set_prescale_chroma_rd_addr2_chroma_rd_addr2(MT_U32 data)
{
    reg_symphony_optm_disp_prescale_chroma_rd_addr2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_RD_ADDR2;
    d.bitc.chroma_rd_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_RD_ADDR2, d.all);
}

MT_U32  reg_symphony_optm_disp_get_prescale_chroma_rd_addr2_chroma_rd_addr2(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_chroma_rd_addr2_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_RD_ADDR2).bitc.chroma_rd_addr2;
}


/*!
  register SYMPHONY_DISP_PRESCALE_CHROMA_WR_ADDR (read/write)
  */
void reg_symphony_optm_disp_set_prescale_chroma_wr_addr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_WR_ADDR, data);
}

MT_U32  reg_symphony_optm_disp_get_prescale_chroma_wr_addr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_WR_ADDR);
}

void reg_symphony_optm_disp_set_prescale_chroma_wr_addr_chroma_wr_addr(MT_U32 data)
{
    reg_symphony_optm_disp_prescale_chroma_wr_addr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_WR_ADDR;
    d.bitc.chroma_wr_addr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_WR_ADDR, d.all);
}

MT_U32  reg_symphony_optm_disp_get_prescale_chroma_wr_addr_chroma_wr_addr(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_chroma_wr_addr_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_WR_ADDR).bitc.chroma_wr_addr;
}


/*!
  register SYMPHONY_DISP_PRESCALE_CHROMA_WR_ADDR2 (read/write)
  */
void reg_symphony_optm_disp_set_prescale_chroma_wr_addr2(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_WR_ADDR2, data);
}

MT_U32  reg_symphony_optm_disp_get_prescale_chroma_wr_addr2(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_WR_ADDR2);
}

void reg_symphony_optm_disp_set_prescale_chroma_wr_addr2_chroma_wr_addr2(MT_U32 data)
{
    reg_symphony_optm_disp_prescale_chroma_wr_addr2_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_WR_ADDR2;
    d.bitc.chroma_wr_addr2 = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_WR_ADDR2, d.all);
}

MT_U32  reg_symphony_optm_disp_get_prescale_chroma_wr_addr2_chroma_wr_addr2(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_chroma_wr_addr2_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_CHROMA_WR_ADDR2).bitc.chroma_wr_addr2;
}


/*!
  register SYMPHONY_DISP_PRESCALE_FIFO_THR (read/write)
  */
void reg_symphony_optm_disp_set_prescale_fifo_thr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR, data);
}

MT_U32  reg_symphony_optm_disp_get_prescale_fifo_thr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR);
}

void reg_symphony_optm_disp_set_prescale_fifo_thr_wr_fifo_thr(MT_U8 data)
{
    reg_symphony_optm_disp_prescale_fifo_thr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR;
    d.bitc.wr_fifo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_prescale_fifo_thr_wr_fifo_thr(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_fifo_thr_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR).bitc.wr_fifo_thr;
}

void reg_symphony_optm_disp_set_prescale_fifo_thr_proc_mode_sfifo_thr(MT_U8 data)
{
    reg_symphony_optm_disp_prescale_fifo_thr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR;
    d.bitc.proc_mode_sfifo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_prescale_fifo_thr_proc_mode_sfifo_thr(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_fifo_thr_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR).bitc.proc_mode_sfifo_thr;
}

void reg_symphony_optm_disp_set_prescale_fifo_thr_req_sfifo_thr(MT_U8 data)
{
    reg_symphony_optm_disp_prescale_fifo_thr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR;
    d.bitc.req_sfifo_thr = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_prescale_fifo_thr_req_sfifo_thr(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_fifo_thr_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR).bitc.req_sfifo_thr;
}

void reg_symphony_optm_disp_set_prescale_fifo_thr_block_rate_cnt_max(MT_U8 data)
{
    reg_symphony_optm_disp_prescale_fifo_thr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR;
    d.bitc.block_rate_cnt_max = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_prescale_fifo_thr_block_rate_cnt_max(void)
{
    return (*(volatile reg_symphony_optm_disp_prescale_fifo_thr_t *)REG_SYMPHONY_OPTM_DISP_PRESCALE_FIFO_THR).bitc.block_rate_cnt_max;
}


/*!
  register SYMPHONY_DISP_HD_HUE_ADJUST (read/write)
  */
void reg_symphony_optm_disp_set_hd_hue_adjust(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_HUE_ADJUST, data);
}

MT_U32  reg_symphony_optm_disp_get_hd_hue_adjust(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_HUE_ADJUST);
}

void reg_symphony_optm_disp_set_hd_hue_adjust_hd_sina(MT_U16 data)
{
    reg_symphony_optm_disp_hd_hue_adjust_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_HUE_ADJUST;
    d.bitc.hd_sina = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_HUE_ADJUST, d.all);
}

MT_U16  reg_symphony_optm_disp_get_hd_hue_adjust_hd_sina(void)
{
    return (*(volatile reg_symphony_optm_disp_hd_hue_adjust_t *)REG_SYMPHONY_OPTM_DISP_HD_HUE_ADJUST).bitc.hd_sina;
}

void reg_symphony_optm_disp_set_hd_hue_adjust_hd_cosa(MT_U16 data)
{
    reg_symphony_optm_disp_hd_hue_adjust_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_HUE_ADJUST;
    d.bitc.hd_cosa = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_HUE_ADJUST, d.all);
}

MT_U16  reg_symphony_optm_disp_get_hd_hue_adjust_hd_cosa(void)
{
    return (*(volatile reg_symphony_optm_disp_hd_hue_adjust_t *)REG_SYMPHONY_OPTM_DISP_HD_HUE_ADJUST).bitc.hd_cosa;
}

void reg_symphony_optm_disp_set_hd_hue_adjust_hd_hue_en(MT_U8 data)
{
    reg_symphony_optm_disp_hd_hue_adjust_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HD_HUE_ADJUST;
    d.bitc.hd_hue_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HD_HUE_ADJUST, d.all);
}

MT_U8   reg_symphony_optm_disp_get_hd_hue_adjust_hd_hue_en(void)
{
    return (*(volatile reg_symphony_optm_disp_hd_hue_adjust_t *)REG_SYMPHONY_OPTM_DISP_HD_HUE_ADJUST).bitc.hd_hue_en;
}


/*!
  register SYMPHONY_DISP_SD_HUE_ADJUST (read/write)
  */
void reg_symphony_optm_disp_set_sd_hue_adjust(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_HUE_ADJUST, data);
}

MT_U32  reg_symphony_optm_disp_get_sd_hue_adjust(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_HUE_ADJUST);
}

void reg_symphony_optm_disp_set_sd_hue_adjust_sd_sina(MT_U16 data)
{
    reg_symphony_optm_disp_sd_hue_adjust_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_HUE_ADJUST;
    d.bitc.sd_sina = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_HUE_ADJUST, d.all);
}

MT_U16  reg_symphony_optm_disp_get_sd_hue_adjust_sd_sina(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_hue_adjust_t *)REG_SYMPHONY_OPTM_DISP_SD_HUE_ADJUST).bitc.sd_sina;
}

void reg_symphony_optm_disp_set_sd_hue_adjust_sd_cosa(MT_U16 data)
{
    reg_symphony_optm_disp_sd_hue_adjust_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_HUE_ADJUST;
    d.bitc.sd_cosa = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_HUE_ADJUST, d.all);
}

MT_U16  reg_symphony_optm_disp_get_sd_hue_adjust_sd_cosa(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_hue_adjust_t *)REG_SYMPHONY_OPTM_DISP_SD_HUE_ADJUST).bitc.sd_cosa;
}

void reg_symphony_optm_disp_set_sd_hue_adjust_sd_hue_en(MT_U8 data)
{
    reg_symphony_optm_disp_sd_hue_adjust_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_SD_HUE_ADJUST;
    d.bitc.sd_hue_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_SD_HUE_ADJUST, d.all);
}

MT_U8   reg_symphony_optm_disp_get_sd_hue_adjust_sd_hue_en(void)
{
    return (*(volatile reg_symphony_optm_disp_sd_hue_adjust_t *)REG_SYMPHONY_OPTM_DISP_SD_HUE_ADJUST).bitc.sd_hue_en;
}


/*!
  register SYMPHONY_DISP_HALF_SCALE_CTR (read/write)
  */
void reg_symphony_optm_disp_set_half_scale_ctr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HALF_SCALE_CTR, data);
}

MT_U32  reg_symphony_optm_disp_get_half_scale_ctr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HALF_SCALE_CTR);
}

void reg_symphony_optm_disp_set_half_scale_ctr_h_enable(MT_U8 data)
{
    reg_symphony_optm_disp_half_scale_ctr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HALF_SCALE_CTR;
    d.bitc.h_enable = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HALF_SCALE_CTR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_half_scale_ctr_h_enable(void)
{
    return (*(volatile reg_symphony_optm_disp_half_scale_ctr_t *)REG_SYMPHONY_OPTM_DISP_HALF_SCALE_CTR).bitc.h_enable;
}

void reg_symphony_optm_disp_set_half_scale_ctr_v_enable(MT_U8 data)
{
    reg_symphony_optm_disp_half_scale_ctr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_HALF_SCALE_CTR;
    d.bitc.v_enable = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_HALF_SCALE_CTR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_half_scale_ctr_v_enable(void)
{
    return (*(volatile reg_symphony_optm_disp_half_scale_ctr_t *)REG_SYMPHONY_OPTM_DISP_HALF_SCALE_CTR).bitc.v_enable;
}


/*!
  register SYMPHONY_DISP_PIX_ALIGN_CTR (read/write)
  */
void reg_symphony_optm_disp_set_pix_align_ctr(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PIX_ALIGN_CTR, data);
}

MT_U32  reg_symphony_optm_disp_get_pix_align_ctr(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PIX_ALIGN_CTR);
}

void reg_symphony_optm_disp_set_pix_align_ctr_last_cut_pix_num(MT_U8 data)
{
    reg_symphony_optm_disp_pix_align_ctr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PIX_ALIGN_CTR;
    d.bitc.last_cut_pix_num = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PIX_ALIGN_CTR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_pix_align_ctr_last_cut_pix_num(void)
{
    return (*(volatile reg_symphony_optm_disp_pix_align_ctr_t *)REG_SYMPHONY_OPTM_DISP_PIX_ALIGN_CTR).bitc.last_cut_pix_num;
}

void reg_symphony_optm_disp_set_pix_align_ctr_pre_cut_pix_num(MT_U8 data)
{
    reg_symphony_optm_disp_pix_align_ctr_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_PIX_ALIGN_CTR;
    d.bitc.pre_cut_pix_num = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_PIX_ALIGN_CTR, d.all);
}

MT_U8   reg_symphony_optm_disp_get_pix_align_ctr_pre_cut_pix_num(void)
{
    return (*(volatile reg_symphony_optm_disp_pix_align_ctr_t *)REG_SYMPHONY_OPTM_DISP_PIX_ALIGN_CTR).bitc.pre_cut_pix_num;
}


/*!
  register SYMPHONY_DISP_DISP_PROT_LIMIT (read/write)
  */
void reg_symphony_optm_disp_set_disp_prot_limit(MT_U32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT, data);
}

MT_U32  reg_symphony_optm_disp_get_disp_prot_limit(void)
{
    return (*(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT);
}

void reg_symphony_optm_disp_set_disp_prot_limit_back_prot_en(MT_U8 data)
{
    reg_symphony_optm_disp_disp_prot_limit_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT;
    d.bitc.back_prot_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT, d.all);
}

MT_U8   reg_symphony_optm_disp_get_disp_prot_limit_back_prot_en(void)
{
    return (*(volatile reg_symphony_optm_disp_disp_prot_limit_t *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT).bitc.back_prot_en;
}

void reg_symphony_optm_disp_set_disp_prot_limit_still_prot_en(MT_U8 data)
{
    reg_symphony_optm_disp_disp_prot_limit_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT;
    d.bitc.still_prot_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT, d.all);
}

MT_U8   reg_symphony_optm_disp_get_disp_prot_limit_still_prot_en(void)
{
    return (*(volatile reg_symphony_optm_disp_disp_prot_limit_t *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT).bitc.still_prot_en;
}

void reg_symphony_optm_disp_set_disp_prot_limit_vid_prot_en(MT_U8 data)
{
    reg_symphony_optm_disp_disp_prot_limit_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT;
    d.bitc.vid_prot_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT, d.all);
}

MT_U8   reg_symphony_optm_disp_get_disp_prot_limit_vid_prot_en(void)
{
    return (*(volatile reg_symphony_optm_disp_disp_prot_limit_t *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT).bitc.vid_prot_en;
}

void reg_symphony_optm_disp_set_disp_prot_limit_osd0_rpot_en(MT_U8 data)
{
    reg_symphony_optm_disp_disp_prot_limit_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT;
    d.bitc.osd0_rpot_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT, d.all);
}

MT_U8   reg_symphony_optm_disp_get_disp_prot_limit_osd0_rpot_en(void)
{
    return (*(volatile reg_symphony_optm_disp_disp_prot_limit_t *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT).bitc.osd0_rpot_en;
}

void reg_symphony_optm_disp_set_disp_prot_limit_osd1_prot_en(MT_U8 data)
{
    reg_symphony_optm_disp_disp_prot_limit_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT;
    d.bitc.osd1_prot_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT, d.all);
}

MT_U8   reg_symphony_optm_disp_get_disp_prot_limit_osd1_prot_en(void)
{
    return (*(volatile reg_symphony_optm_disp_disp_prot_limit_t *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT).bitc.osd1_prot_en;
}

void reg_symphony_optm_disp_set_disp_prot_limit_sub_prot_en(MT_U8 data)
{
    reg_symphony_optm_disp_disp_prot_limit_t d;
    d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT;
    d.bitc.sub_prot_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT, d.all);
}

MT_U8   reg_symphony_optm_disp_get_disp_prot_limit_sub_prot_en(void)
{
    return (*(volatile reg_symphony_optm_disp_disp_prot_limit_t *)REG_SYMPHONY_OPTM_DISP_DISP_PROT_LIMIT).bitc.sub_prot_en;
}


//mfbc
void reg_symphony_optm_disp_set_luma_lut_pre_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_LPRE_LUT_BASE_REG, data);
}

void reg_symphony_optm_disp_set_luma_lut_cur_top_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_LTOP_LUT_BASE_REG, data);
}

void reg_symphony_optm_disp_set_luma_lut_cur_bot_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_LBOT_LUT_BASE_REG, data);
}

void reg_symphony_optm_disp_set_luma_lut_nxt_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_LNXT_LUT_BASE_REG, data);
}

void reg_symphony_optm_disp_set_chroma_lut_ppre_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_CPPR_LUT_BASE_REG, data);
}

void reg_symphony_optm_disp_set_chroma_lut_pre_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_CPRE_LUT_BASE_REG, data);
}

void reg_symphony_optm_disp_set_chroma_lut_cur_top_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_CTOP_LUT_BASE_REG, data);
}

void reg_symphony_optm_disp_set_chroma_lut_cur_bot_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_CBOT_LUT_BASE_REG, data);
}

void reg_symphony_optm_disp_set_chroma_lut_nxt_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_CNXT_LUT_BASE_REG, data);
}

void reg_symphony_optm_disp_set_luma_lut_pre_2_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_LPRE_LUT_BASE_2_REG, data);
}

void reg_symphony_optm_disp_set_luma_lut_cur_top_2_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_LTOP_LUT_BASE_2_REG, data);
}

void reg_symphony_optm_disp_set_luma_lut_cur_bot_2_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_LBOT_LUT_BASE_2_REG, data);
}

void reg_symphony_optm_disp_set_luma_lut_nxt_2_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_LNXT_LUT_BASE_2_REG, data);
}

void reg_symphony_optm_disp_set_chroma_lut_ppre_2_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_CPPR_LUT_BASE_2_REG, data);
}

void reg_symphony_optm_disp_set_chroma_lut_pre_2_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_CPRE_LUT_BASE_2_REG, data);
}

void reg_symphony_optm_disp_set_chroma_lut_cur_top_2_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_CTOP_LUT_BASE_2_REG, data);
}

void reg_symphony_optm_disp_set_chroma_lut_cur_bot_2_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_CBOT_LUT_BASE_2_REG, data);
}

void reg_symphony_optm_disp_set_chroma_lut_nxt_2_addr(MT_U32 data)
{
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_CNXT_LUT_BASE_2_REG, data);
}

void reg_symphony_optm_disp_set_decomp_ctrl_en(MT_U8 data)
{
  reg_symphony_optm_disp_decomp_ctrl_0_t d;
  d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DECOMP_CTRL_0_REG;
  d.bitc.en = data;
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_CTRL_0_REG, d.all);
}

void reg_symphony_optm_disp_set_luma_bit_depth(MT_U8 data)
{
  reg_symphony_optm_disp_decomp_ctrl_0_t d;
  d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DECOMP_CTRL_0_REG;
  d.bitc.bit_depth_luma = data;
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_CTRL_0_REG, d.all);
}

void reg_symphony_optm_disp_set_chroma_bit_depth(MT_U8 data)
{
  reg_symphony_optm_disp_decomp_ctrl_0_t d;
  d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DECOMP_CTRL_0_REG;
  d.bitc.bit_depth_chroma = data;
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_CTRL_0_REG, d.all);
}

void reg_symphony_optm_disp_set_1bgs_luma_stride_data(MT_U16 data)
{
  reg_symphony_optm_disp_decomp_1bgs_str_t d;
  d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DECOMP_1BGS_STR_REG;
  d.bitc.stride_data_luma = data;
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_1BGS_STR_REG, d.all);
}

void reg_symphony_optm_disp_set_1bgs_chroma_stride_data(MT_U16 data)
{
  reg_symphony_optm_disp_decomp_1bgs_str_t d;
  d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DECOMP_1BGS_STR_REG;
  d.bitc.stride_data_chroma = data;
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_1BGS_STR_REG, d.all);
}

void reg_symphony_optm_disp_set_pic_resolution_width(MT_U16 data)
{
  reg_symphony_optm_disp_decomp_pic_resl_t d;
  d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DECOMP_PIC_RESOLUTION_REG;
  d.bitc.pic_width = data;
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_PIC_RESOLUTION_REG, d.all);
}

void reg_symphony_optm_disp_set_pic_resolution_height(MT_U16 data)
{
  reg_symphony_optm_disp_decomp_pic_resl_t d;
  d.all = *(volatile MT_U32 *)REG_SYMPHONY_OPTM_DISP_DECOMP_PIC_RESOLUTION_REG;
  d.bitc.pic_height = data;
  hal_put_u32((volatile unsigned long *)REG_SYMPHONY_OPTM_DISP_DECOMP_PIC_RESOLUTION_REG, d.all);
}

/*!
  init function
  */
void reg_symphony_optm_disp_init(void)
{
    reg_symphony_optm_disp_set_display_ctrl((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vscaler_ratio_hd((MT_U32)0x00000001);
    reg_symphony_optm_disp_set_vscaler_ratio_init_hd((MT_U32)0x00000001);
    reg_symphony_optm_disp_set_vid_disp_field((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vid_window_cut_sd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vid_window_x_hd((MT_U32)0x07800000);
    reg_symphony_optm_disp_set_vid_window_y_hd((MT_U32)0x021b0000);
    reg_symphony_optm_disp_set_vid_window_cut_hd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vscaler_ratio_sd((MT_U32)0x00010002);
    reg_symphony_optm_disp_set_vscaler_ratio_init_sd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vid_window_x_sd((MT_U32)0x02d10001);
    reg_symphony_optm_disp_set_vid_window_y_sd((MT_U32)0x01200000);
    reg_symphony_optm_disp_set_graphic_ctrl((MT_U32)0x00000100);
    reg_symphony_optm_disp_set_bg_color((MT_U32)0x00108080);
    reg_symphony_optm_disp_set_still_x_hd((MT_U32)0x017c0014);
    reg_symphony_optm_disp_set_still_y_hd((MT_U32)0x00900000);
    reg_symphony_optm_disp_set_osd0_cmd_hd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_sub_cmd_hd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_layer_alpha((MT_U32)0x80ff0000);
    reg_symphony_optm_disp_set_vid_input_size((MT_U32)0x07800438);
    reg_symphony_optm_disp_set_vid_sd_drop_line((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vid_crop_mode_en((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vid_crop_hori((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vid_crop_vert((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vid_process_mode((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_osd1_cmd_hd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_osd1_ck_hd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_still_x_sd((MT_U32)0x017c0014);
    reg_symphony_optm_disp_set_still_y_sd((MT_U32)0x00900000);
    reg_symphony_optm_disp_set_still_stride_sd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_rgb2y_coef((MT_U32)0x64204107);
    reg_symphony_optm_disp_set_rgb2cb_coef((MT_U32)0x971291c2);
    reg_symphony_optm_disp_set_rgb2cr_coef((MT_U32)0x481781c2);
    reg_symphony_optm_disp_set_vid_decomp_cfg((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_osd0_ck_hd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_rgb2yuv_yoffset((MT_U32)0x00004000);
    reg_symphony_optm_disp_set_rgb2yuv_uvoffset((MT_U32)0x00020000);
    reg_symphony_optm_disp_set_vid_verf_cfg((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vid_horf_cfg((MT_U32)0x00000008);
    reg_symphony_optm_disp_set_layer_mix_cfg((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_hd_size_out((MT_U32)0x012002d0);
    reg_symphony_optm_disp_set_sd_size_out((MT_U32)0x012002d0);
    reg_symphony_optm_disp_set_still_stride_hd((MT_U32)0x02310000);
    reg_symphony_optm_disp_set_hdtv_cfg((MT_U32)0x00000300);
    reg_symphony_optm_disp_set_hd_post_cfg((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_sd_post_cfg((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_hd_effect_coef((MT_U32)0x00808080);
    reg_symphony_optm_disp_set_sd_effect_coef((MT_U32)0x00808080);
    reg_symphony_optm_disp_set_gra_fifo_threshold((MT_U32)0x28102810);
    reg_symphony_optm_disp_set_gra_scaler_ctrl((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_gra_scaler_hratio((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_gra_scaler_vratio((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_gra_scaler_h_start_fra((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_gra_scaler_v_start_fra((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_gra_ctl((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_osd_scale_hsize((MT_U32)0x07800780);
    reg_symphony_optm_disp_set_osd_scale_ratio((MT_U32)0x00010000);
    reg_symphony_optm_disp_set_osd_alpha((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_osd_vert_start_line((MT_U32)0x00000011);
    reg_symphony_optm_disp_set_osd_vertical_ctrl((MT_U32)0x00000010);
    reg_symphony_optm_disp_set_osd_vertical_size((MT_U32)0x043802d0);
    reg_symphony_optm_disp_set_osd_vertical_ratio((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_osd_v_start_fra((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_osd_v_tap_num((MT_U32)0x00000003);
    reg_symphony_optm_disp_set_chroma_coef0((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_chroma_coef1((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_chroma_coef2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_chroma_coef3((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_scale_init_phase_offset((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_small_pic_upscale_ctrl((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_alising_prob_reg1((MT_U32)0x01000000);
    reg_symphony_optm_disp_set_alising_prob_reg2((MT_U32)0x8e900100);
    reg_symphony_optm_disp_set_alising_prob_reg3((MT_U32)0x0101ff40);
    reg_symphony_optm_disp_set_alising_prob_reg4((MT_U32)0x4018188c);
    reg_symphony_optm_disp_set_csc_ctrl((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_hd_coef1((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_hd_coef2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_hd_coef3((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_hd_coef4((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_hd_coef5((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_sd_coef1((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_sd_coef2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_sd_coef3((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_sd_coef4((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_sd_coef5((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_still_ctrl((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_still_coef1((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_still_coef2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_still_coef3((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_still_coef4((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_csc_still_coef5((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_row_jump_00((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_row_jump_01((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_row_jump_10((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_row_jump_11((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_denoise_domain_1((MT_U32)0x00e1009a);
    reg_symphony_optm_disp_set_denoise_domain_2((MT_U32)0x00520022);
    reg_symphony_optm_disp_set_denoise_domain_3((MT_U32)0x000b0000);
    reg_symphony_optm_disp_set_denoise_range_1((MT_U32)0xfffcf4e8);
    reg_symphony_optm_disp_set_denoise_range_2((MT_U32)0xd8c5b09a);
    reg_symphony_optm_disp_set_denoise_range_3((MT_U32)0x846f5b4a);
    reg_symphony_optm_disp_set_denoise_range_4((MT_U32)0x3a2d0019);
    reg_symphony_optm_disp_set_denoise_range_5((MT_U32)0x120d0906);
    reg_symphony_optm_disp_set_denoise_range_6((MT_U32)0x04006400);
    reg_symphony_optm_disp_set_sd_wrback_addr_odd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_sd_base_addr_even((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_sd_wrback_ctrl((MT_U32)0x02000032);
    reg_symphony_optm_disp_set_sd_back_color((MT_U32)0x00108080);
    reg_symphony_optm_disp_set_still_uv_start_addr_hd((MT_U32)0x0002c000);
    reg_symphony_optm_disp_set_still_y_start_addr_hd((MT_U32)0x00020000);
    reg_symphony_optm_disp_set_sub_start_addr_hd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_osd1_start_addr_hd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_still_start_addr_sd((MT_U32)0x00020000);
    reg_symphony_optm_disp_set_osd0_start_addr_hd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_video_display_info((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_motion_pre_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_motion_cur_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_luma_pre_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_luma_cur_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_luma_next_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_chroma_ppre_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_chroma_pre_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_chroma_cur_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_chroma_next_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_motion_pre_addr2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_motion_cur_addr2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_luma_pre_addr2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_luma_cur_addr2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_luma_next_addr2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_chroma_ppre_addr2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_chroma_pre_addr2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_chroma_cur_addr2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_chroma_next_addr2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_field_pic_fmt((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vid_hd_vf_coef_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vid_hd_hf_coef_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vid_sd_vf_coef_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vid_sd_hf_coef_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_gra_vf_coef_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_gra_hf_coef_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vid_dce_map_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_vscaler_table_sel((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_osd_vf_coef_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_osd_hf_coef_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_chroma_hd_hf_coef_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_chroma_sd_hf_coef_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_nlmeans_denoise_ctrl((MT_U32)0x1e320100);
    reg_symphony_optm_disp_set_nlmeans_parameter_1((MT_U32)0x00130008);
    reg_symphony_optm_disp_set_nlmeans_parameter_2((MT_U32)0x000207de);
    reg_symphony_optm_disp_set_nlmeans_parameter_3((MT_U32)0x005a026a);
    reg_symphony_optm_disp_set_di_enable((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_video_pdd_en((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_video_is_movie_type((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_di_p_or_n_paired((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_di_oper_mode((MT_U32)0x11001000);
    reg_symphony_optm_disp_set_di_para((MT_U32)0x000c000c);
    reg_symphony_optm_disp_set_di_data_sfifo_thr((MT_U32)0x0000002a);
    reg_symphony_optm_disp_set_video_scaler_data_sfifo_thr((MT_U32)0x0000000a);
    reg_symphony_optm_disp_set_di_lout_afifo_thr((MT_U32)0x00000008);
    reg_symphony_optm_disp_set_vscaler_axi_cmd_sfifo_thr((MT_U32)0x0000000a);
    reg_symphony_optm_disp_set_vscaler_axi_req_sfifo_thr((MT_U32)0x0000000a);
    reg_symphony_optm_disp_set_di_pdd_noise_thr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_di_acc_result_odd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_di_acc_result_even((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_di_pause_en((MT_U32)0x00000010);
    reg_symphony_optm_disp_set_di_alpha_para((MT_U32)0x00011400);
    reg_symphony_optm_disp_set_di_motion_ctrl_1((MT_U32)0x4040d801);
    reg_symphony_optm_disp_set_di_motion_ctrl_2((MT_U32)0x2a020001);
    reg_symphony_optm_disp_set_di_hevc_flag((MT_U32)0x01011010);
    reg_symphony_optm_disp_set_none_di_fields_flag((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_none_di_hevc_flag((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_none_di_progressive_flag((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_di_motion_ctrl_3((MT_U32)0x02020801);
    reg_symphony_optm_disp_set_di_motion_ctrl_4((MT_U32)0x0300020c);
    reg_symphony_optm_disp_set_di_chroma_para((MT_U32)0x00000001);
    reg_symphony_optm_disp_set_video_burst_length_sel((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_video_burst_info((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_video_line_rd_cnt_max((MT_U32)0x000000f0);
    reg_symphony_optm_disp_set_prescale_cmd((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_prescale_frame_size((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_prescale_tiling_params((MT_U32)0x00000131);
    reg_symphony_optm_disp_set_prescale_luma_rd_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_prescale_luma_rd_addr2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_prescale_luma_wr_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_prescale_luma_wr_addr2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_prescale_chroma_rd_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_prescale_chroma_rd_addr2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_prescale_chroma_wr_addr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_prescale_chroma_wr_addr2((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_prescale_fifo_thr((MT_U32)0x0008080a);
    reg_symphony_optm_disp_set_hd_hue_adjust((MT_U32)0x03ff0000);
    reg_symphony_optm_disp_set_sd_hue_adjust((MT_U32)0x03ff0000);
    reg_symphony_optm_disp_set_half_scale_ctr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_pix_align_ctr((MT_U32)0x00000000);
    reg_symphony_optm_disp_set_disp_prot_limit((MT_U32)0x00000000);

    /* read read-clear registers in order to set mirror variables */
}

/*!
  end of file
  */

