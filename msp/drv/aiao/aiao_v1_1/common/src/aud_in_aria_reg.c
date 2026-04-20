/******************************************************************************/
/* Copyright (c) 2012 Montage Tech - All Rights Reserved                      */
/******************************************************************************/
#include <linux/delay.h>

#include "mt_type.h"
#include "aud_in_aria_reg.h"
#ifdef CONFIG_MT_CHIP_ARIA
#include <mach/aria_io.h>
#else
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif

#include "mt_module_debug.h"
extern ulong mt_get_audioin_base(void);

static inline mt_void hal_put_u32(volatile unsigned long *p_addr, unsigned long data)
{
    /*!
      Write 32 bits register
    */
    *((volatile mt_u32 *)(p_addr)) = data;
}
static inline mt_u32 hal_get_u32(volatile mt_u32 *p_addr)
{
    return *(volatile mt_u32 *)(p_addr);
}

/*!
  register AUDIN_ARIA_CPTR_EN (read/write)
  */
mt_void reg_audin_aria_set_cptr_en(mt_u32 data)
{
    ulong reg = (ulong)REG_AUDIN_ARIA_CPTR_EN + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_cptr_en(mt_void)
{
    ulong reg = (ulong)REG_AUDIN_ARIA_CPTR_EN + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_cptr_en_audio_cptr_en(mt_u8 data)
{
    reg_audin_aria_cptr_en_t d;
    ulong reg = (ulong)REG_AUDIN_ARIA_CPTR_EN + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audio_cptr_en = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_cptr_en_audio_cptr_en(mt_void)
{
    ulong reg = (ulong)REG_AUDIN_ARIA_CPTR_EN + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_cptr_en_t *)reg).bitc.audio_cptr_en;
}

mt_void reg_audin_aria_set_cptr_en_axi_bus_end_flag(mt_u8 data)
{
    reg_audin_aria_cptr_en_t d;
    ulong reg = (ulong)REG_AUDIN_ARIA_CPTR_EN + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.axi_bus_end_flag = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_cptr_en_axi_bus_end_flag(mt_void)
{
    ulong reg = (ulong)REG_AUDIN_ARIA_CPTR_EN + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_cptr_en_t *)reg).bitc.axi_bus_end_flag;
}

mt_void reg_audin_aria_set_cptr_en_audin_cptr_end_flag(mt_u8 data)
{
    reg_audin_aria_cptr_en_t d;
    ulong reg = (ulong)REG_AUDIN_ARIA_CPTR_EN + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audin_cptr_end_flag = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_cptr_en_audin_cptr_end_flag(mt_void)
{
    ulong reg = (ulong)REG_AUDIN_ARIA_CPTR_EN + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_cptr_en_t *)reg).bitc.audin_cptr_end_flag;
}


/*!
  register AUDIN_ARIA_I2S_FMT (read/write)
  */
mt_void reg_audin_aria_set_i2s_fmt(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_I2S_FMT + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_i2s_fmt(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_I2S_FMT + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_i2s_fmt_just(mt_u8 data)
{
    reg_audin_aria_i2s_fmt_t d;
    ulong reg = REG_AUDIN_ARIA_I2S_FMT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.just = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_i2s_fmt_just(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_I2S_FMT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_i2s_fmt_t *)reg).bitc.just;
}

mt_void reg_audin_aria_set_i2s_fmt_ws_pol(mt_u8 data)
{
    reg_audin_aria_i2s_fmt_t d;
    ulong reg = REG_AUDIN_ARIA_I2S_FMT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.ws_pol = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_i2s_fmt_ws_pol(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_I2S_FMT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_i2s_fmt_t *)reg).bitc.ws_pol;
}

mt_void reg_audin_aria_set_i2s_fmt_shift(mt_u8 data)
{
    reg_audin_aria_i2s_fmt_t d;
    ulong reg = REG_AUDIN_ARIA_I2S_FMT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.shift = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_i2s_fmt_shift(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_I2S_FMT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_i2s_fmt_t *)reg).bitc.shift;
}

mt_void reg_audin_aria_set_i2s_fmt_little_endian(mt_u8 data)
{
    reg_audin_aria_i2s_fmt_t d;
    ulong reg = REG_AUDIN_ARIA_I2S_FMT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.little_endian = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_i2s_fmt_little_endian(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_I2S_FMT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_i2s_fmt_t *)reg).bitc.little_endian;
}


/*!
  register AUDIN_ARIA_BCLK_OE (read/write)
  */
mt_void reg_audin_aria_set_bclk_oe(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_BCLK_OE + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_bclk_oe(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BCLK_OE + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_bclk_oe_bclk_oe(mt_u8 data)
{
    reg_audin_aria_bclk_oe_t d;
    ulong reg = REG_AUDIN_ARIA_BCLK_OE + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.bclk_oe = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_bclk_oe_bclk_oe(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BCLK_OE + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_bclk_oe_t *)reg).bitc.bclk_oe;
}

mt_void reg_audin_aria_set_bclk_oe_ws_oe(mt_u8 data)
{
    reg_audin_aria_bclk_oe_t d;
    ulong reg = REG_AUDIN_ARIA_BCLK_OE + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.ws_oe = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_bclk_oe_ws_oe(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BCLK_OE + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_bclk_oe_t *)reg).bitc.ws_oe;
}

mt_void reg_audin_aria_set_bclk_oe_mclk_oe(mt_u8 data)
{
    reg_audin_aria_bclk_oe_t d;
    ulong reg = REG_AUDIN_ARIA_BCLK_OE + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.mclk_oe = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_bclk_oe_mclk_oe(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BCLK_OE + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_bclk_oe_t *)reg).bitc.mclk_oe;
}


/*!
  register AUDIN_ARIA_CLK_DIV_CFG (read/write)
  */
mt_void reg_audin_aria_set_clk_div_cfg(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_CLK_DIV_CFG + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_clk_div_cfg(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_CLK_DIV_CFG + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_clk_div_cfg_clk_divider_factor(mt_u32 data)
{
    reg_audin_aria_clk_div_cfg_t d;
    ulong reg = REG_AUDIN_ARIA_CLK_DIV_CFG + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.clk_divider_factor = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u32  reg_audin_aria_get_clk_div_cfg_clk_divider_factor(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_CLK_DIV_CFG + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_clk_div_cfg_t *)reg).bitc.clk_divider_factor;
}


/*!
  register AUDIN_ARIA_BUF0_ADDR (read/write)
  */
mt_void reg_audin_aria_set_buf0_addr(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_BUF0_ADDR + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_buf0_addr(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF0_ADDR + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_buf0_addr_buf0_addr(mt_u32 data)
{
    reg_audin_aria_buf0_addr_t d;
    ulong reg = REG_AUDIN_ARIA_BUF0_ADDR + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.buf0_addr = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u32  reg_audin_aria_get_buf0_addr_buf0_addr(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF0_ADDR + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_buf0_addr_t *)reg).bitc.buf0_addr;
}


/*!
  register AUDIN_ARIA_BUF1_ADDR (read/write)
  */
mt_void reg_audin_aria_set_buf1_addr(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_BUF1_ADDR + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_buf1_addr(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF1_ADDR + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_buf1_addr_buf1_addr(mt_u32 data)
{
    reg_audin_aria_buf1_addr_t d;
    ulong reg = REG_AUDIN_ARIA_BUF1_ADDR + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.buf1_addr = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u32  reg_audin_aria_get_buf1_addr_buf1_addr(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF1_ADDR + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_buf1_addr_t *)reg).bitc.buf1_addr;
}


/*!
  register AUDIN_ARIA_BUF_LENGTH (read/write)
  */
mt_void reg_audin_aria_set_buf_length(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_BUF_LENGTH + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_buf_length(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF_LENGTH + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_buf_length_buf_len(mt_u32 data)
{
    reg_audin_aria_buf_length_t d;
    ulong reg = REG_AUDIN_ARIA_BUF_LENGTH + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.buf_len = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u32  reg_audin_aria_get_buf_length_buf_len(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF_LENGTH + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_buf_length_t *)reg).bitc.buf_len;
}


/*!
  register AUDIN_ARIA_BUF_MODE (read/write)
  */
mt_void reg_audin_aria_set_buf_mode(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_BUF_MODE + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_buf_mode(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF_MODE + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_buf_mode_interlace_mode(mt_u8 data)
{
    reg_audin_aria_buf_mode_t d;
    ulong reg = REG_AUDIN_ARIA_BUF_MODE + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.interlace_mode = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_buf_mode_interlace_mode(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF_MODE + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_buf_mode_t *)reg).bitc.interlace_mode;
}

mt_void reg_audin_aria_set_buf_mode_addr_incr_mode(mt_u8 data)
{
    reg_audin_aria_buf_mode_t d;
    ulong reg = REG_AUDIN_ARIA_BUF_MODE + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.addr_incr_mode = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_buf_mode_addr_incr_mode(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF_MODE + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_buf_mode_t *)reg).bitc.addr_incr_mode;
}

mt_void reg_audin_aria_set_buf_mode_audio_play_mode(mt_u8 data)
{
    reg_audin_aria_buf_mode_t d;
    ulong reg = REG_AUDIN_ARIA_BUF_MODE + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audio_play_mode = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_buf_mode_audio_play_mode(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF_MODE + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_buf_mode_t *)reg).bitc.audio_play_mode;
}


/*!
  register AUDIN_ARIA_CPTR_PAUSE (read/write)
  */
mt_void reg_audin_aria_set_cptr_pause(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_CPTR_PAUSE + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_cptr_pause(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_CPTR_PAUSE + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_cptr_pause_audio_in_cptr_pause(mt_u8 data)
{
    reg_audin_aria_cptr_pause_t d;
    ulong reg = REG_AUDIN_ARIA_CPTR_PAUSE + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audio_in_cptr_pause = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_cptr_pause_audio_in_cptr_pause(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_CPTR_PAUSE + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_cptr_pause_t *)reg).bitc.audio_in_cptr_pause;
}


/*!
  register AUDIN_ARIA_AUDON_INTR_SET (read/write)
  */
mt_void reg_audin_aria_set_audon_intr_set(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_AUDON_INTR_SET + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_audon_intr_set(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AUDON_INTR_SET + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_audon_intr_set_i_frm_interrupt(mt_u8 data)
{
    reg_audin_aria_audon_intr_set_t d;
    ulong reg = REG_AUDIN_ARIA_AUDON_INTR_SET + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.i_frm_interrupt = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_audon_intr_set_i_frm_interrupt(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AUDON_INTR_SET + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_audon_intr_set_t *)reg).bitc.i_frm_interrupt;
}

mt_void reg_audin_aria_set_audon_intr_set_frm_intr_en(mt_u8 data)
{
    reg_audin_aria_audon_intr_set_t d;
    ulong reg = REG_AUDIN_ARIA_AUDON_INTR_SET + mt_get_audioin_base();
    MT_INFO_AIAO("-----\n");
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.frm_intr_en = data;
    MT_INFO_AIAO("-----\n");
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_audon_intr_set_frm_intr_en(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AUDON_INTR_SET + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_audon_intr_set_t *)reg).bitc.frm_intr_en;
}

mt_void reg_audin_aria_set_audon_intr_set_frm_intr_mask(mt_u8 data)
{
    reg_audin_aria_audon_intr_set_t d;
    ulong reg = REG_AUDIN_ARIA_AUDON_INTR_SET + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.frm_intr_mask = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_audon_intr_set_frm_intr_mask(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AUDON_INTR_SET + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_audon_intr_set_t *)reg).bitc.frm_intr_mask;
}


/*!
  register AUDIN_ARIA_FRM_SIZE (read/write)
  */
mt_void reg_audin_aria_set_frm_size(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_FRM_SIZE + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_frm_size(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_FRM_SIZE + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_frm_size_frm_size(mt_u16 data)
{
    reg_audin_aria_frm_size_t d;
    ulong reg = REG_AUDIN_ARIA_FRM_SIZE + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.frm_size = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u16  reg_audin_aria_get_frm_size_frm_size(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_FRM_SIZE + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_frm_size_t *)reg).bitc.frm_size;
}


/*!
  register AUDIN_ARIA_BUF_THD (read/write)
  */
mt_void reg_audin_aria_set_buf_thd(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_BUF_THD + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_buf_thd(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF_THD + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_buf_thd_buf_urgent_thd(mt_u8 data)
{
    reg_audin_aria_buf_thd_t d;
    ulong reg = REG_AUDIN_ARIA_BUF_THD + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.buf_urgent_thd = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_buf_thd_buf_urgent_thd(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF_THD + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_buf_thd_t *)reg).bitc.buf_urgent_thd;
}


/*!
  register AUDIN_ARIA_AXI_WR_CNT (read/write)
  */
mt_void reg_audin_aria_set_axi_wr_cnt(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_axi_wr_cnt(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_axi_wr_cnt_mem_wr_cnt_0(mt_u8 data)
{
    reg_audin_aria_axi_wr_cnt_t d;
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.mem_wr_cnt_0 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_axi_wr_cnt_mem_wr_cnt_0(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_axi_wr_cnt_t *)reg).bitc.mem_wr_cnt_0;
}

mt_void reg_audin_aria_set_axi_wr_cnt_mem_wr_cnt_1(mt_u8 data)
{
    reg_audin_aria_axi_wr_cnt_t d;
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.mem_wr_cnt_1 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_axi_wr_cnt_mem_wr_cnt_1(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_axi_wr_cnt_t *)reg).bitc.mem_wr_cnt_1;
}

mt_void reg_audin_aria_set_axi_wr_cnt_aw_valid_buf0(mt_u8 data)
{
    reg_audin_aria_axi_wr_cnt_t d;
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.aw_valid_buf0 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_axi_wr_cnt_aw_valid_buf0(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_axi_wr_cnt_t *)reg).bitc.aw_valid_buf0;
}

mt_void reg_audin_aria_set_axi_wr_cnt_w_valid_buf0(mt_u8 data)
{
    reg_audin_aria_axi_wr_cnt_t d;
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.w_valid_buf0 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_axi_wr_cnt_w_valid_buf0(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_axi_wr_cnt_t *)reg).bitc.w_valid_buf0;
}

mt_void reg_audin_aria_set_axi_wr_cnt_aw_valid_buf1(mt_u8 data)
{
    reg_audin_aria_axi_wr_cnt_t d;
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.aw_valid_buf1 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_axi_wr_cnt_aw_valid_buf1(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_axi_wr_cnt_t *)reg).bitc.aw_valid_buf1;
}

mt_void reg_audin_aria_set_axi_wr_cnt_w_valid_buf1(mt_u8 data)
{
    reg_audin_aria_axi_wr_cnt_t d;
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.w_valid_buf1 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_axi_wr_cnt_w_valid_buf1(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_axi_wr_cnt_t *)reg).bitc.w_valid_buf1;
}

mt_void reg_audin_aria_set_axi_wr_cnt_b_valid_flag_0(mt_u8 data)
{
    reg_audin_aria_axi_wr_cnt_t d;
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.b_valid_flag_0 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_axi_wr_cnt_b_valid_flag_0(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_axi_wr_cnt_t *)reg).bitc.b_valid_flag_0;
}

mt_void reg_audin_aria_set_axi_wr_cnt_b_valid_flag_1(mt_u8 data)
{
    reg_audin_aria_axi_wr_cnt_t d;
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.b_valid_flag_1 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_axi_wr_cnt_b_valid_flag_1(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_axi_wr_cnt_t *)reg).bitc.b_valid_flag_1;
}

mt_void reg_audin_aria_set_axi_wr_cnt_mem_wr_b_cnt_0(mt_u8 data)
{
    reg_audin_aria_axi_wr_cnt_t d;
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.mem_wr_b_cnt_0 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_axi_wr_cnt_mem_wr_b_cnt_0(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_axi_wr_cnt_t *)reg).bitc.mem_wr_b_cnt_0;
}

mt_void reg_audin_aria_set_axi_wr_cnt_mem_wr_b_cnt_1(mt_u8 data)
{
    reg_audin_aria_axi_wr_cnt_t d;
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.mem_wr_b_cnt_1 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_axi_wr_cnt_mem_wr_b_cnt_1(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_AXI_WR_CNT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_axi_wr_cnt_t *)reg).bitc.mem_wr_b_cnt_1;
}


/*!
  register AUDIN_ARIA_FIFO_CNT (read)
  */
mt_u32  reg_audin_aria_get_fifo_cnt(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_FIFO_CNT + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}
mt_u8   reg_audin_aria_get_fifo_cnt_left_fifo_cnt(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_FIFO_CNT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_fifo_cnt_t *)reg).bitc.left_fifo_cnt;
}
mt_u8   reg_audin_aria_get_fifo_cnt_right_fifo_cnt(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_FIFO_CNT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_fifo_cnt_t *)reg).bitc.right_fifo_cnt;
}

/*!
  register AUDIN_ARIA_FRM_CNT (read)
  */
mt_u32  reg_audin_aria_get_frm_cnt(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_FRM_CNT + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}
mt_u32  reg_audin_aria_get_frm_cnt_frm_cnt(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_FRM_CNT + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_frm_cnt_t *)reg).bitc.frm_cnt;
}

/*!
  register AUDIN_ARIA_I2S_SIZE (read/write)
  */
mt_void reg_audin_aria_set_i2s_size(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_I2S_SIZE + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_i2s_size(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_I2S_SIZE + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_i2s_size_i2s_bit_size(mt_u8 data)
{
    reg_audin_aria_i2s_size_t d;
    ulong reg = REG_AUDIN_ARIA_I2S_SIZE + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.i2s_bit_size = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_i2s_size_i2s_bit_size(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_I2S_SIZE + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_i2s_size_t *)reg).bitc.i2s_bit_size;
}

mt_void reg_audin_aria_set_i2s_size_pcm_tec_cnt(mt_u16 data)
{
    reg_audin_aria_i2s_size_t d;
    ulong reg = REG_AUDIN_ARIA_I2S_SIZE + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.pcm_tec_cnt = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u16  reg_audin_aria_get_i2s_size_pcm_tec_cnt(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_I2S_SIZE + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_i2s_size_t *)reg).bitc.pcm_tec_cnt;
}

mt_void reg_audin_aria_set_i2s_size_i2s_bit_size_err_cnt(mt_u8 data)
{
    reg_audin_aria_i2s_size_t d;
    ulong reg = REG_AUDIN_ARIA_I2S_SIZE + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.i2s_bit_size_err_cnt = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_audin_aria_get_i2s_size_i2s_bit_size_err_cnt(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_I2S_SIZE + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_i2s_size_t *)reg).bitc.i2s_bit_size_err_cnt;
}


/*!
  register AUDIN_ARIA_BUF0_WADDR (read/write)
  */
mt_void reg_audin_aria_set_buf0_waddr(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_BUF0_WADDR + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_buf0_waddr(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF0_WADDR + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_buf0_waddr_buf0_waddr(mt_u32 data)
{
    reg_audin_aria_buf0_waddr_t d;
    ulong reg = REG_AUDIN_ARIA_BUF0_WADDR + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.buf0_waddr = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u32  reg_audin_aria_get_buf0_waddr_buf0_waddr(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF0_WADDR + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_buf0_waddr_t *)reg).bitc.buf0_waddr;
}


/*!
  register AUDIN_ARIA_BUF1_WADDR (read/write)
  */
mt_void reg_audin_aria_set_buf1_waddr(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_BUF1_WADDR + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_buf1_waddr(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF1_WADDR + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_buf1_waddr_buf1_waddr(mt_u32 data)
{
    reg_audin_aria_buf1_waddr_t d;
    ulong reg = REG_AUDIN_ARIA_BUF1_WADDR + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.buf1_waddr = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u32  reg_audin_aria_get_buf1_waddr_buf1_waddr(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_BUF1_WADDR + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_buf1_waddr_t *)reg).bitc.buf1_waddr;
}


/*!
  register AUDIN_ARIA_RES_REG0 (read/write)
  */
mt_void reg_audin_aria_set_res_reg0(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_RES_REG0 + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_res_reg0(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_RES_REG0 + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_res_reg0_reserved_reg0(mt_u32 data)
{
    reg_audin_aria_res_reg0_t d;
    ulong reg = REG_AUDIN_ARIA_RES_REG0 + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.reserved_reg0 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u32  reg_audin_aria_get_res_reg0_reserved_reg0(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_RES_REG0 + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_res_reg0_t *)reg).bitc.reserved_reg0;
}


/*!
  register AUDIN_ARIA_RES_REG1 (read/write)
  */
mt_void reg_audin_aria_set_res_reg1(mt_u32 data)
{
    ulong reg = REG_AUDIN_ARIA_RES_REG1 + mt_get_audioin_base();
    hal_put_u32((volatile unsigned long *)reg, data);
}

mt_u32  reg_audin_aria_get_res_reg1(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_RES_REG1 + mt_get_audioin_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_audin_aria_set_res_reg1_reserved_reg1(mt_u32 data)
{
    reg_audin_aria_res_reg1_t d;
    ulong reg = REG_AUDIN_ARIA_RES_REG1 + mt_get_audioin_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.reserved_reg1 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u32  reg_audin_aria_get_res_reg1_reserved_reg1(mt_void)
{
    ulong reg = REG_AUDIN_ARIA_RES_REG1 + mt_get_audioin_base();
    return (*(volatile reg_audin_aria_res_reg1_t *)reg).bitc.reserved_reg1;
}


/*!
  init function
  */
mt_void reg_audin_aria_init(mt_void)
{
    reg_audin_aria_set_cptr_en((mt_u32)0x00000000);
    reg_audin_aria_set_i2s_fmt((mt_u32)0x00000000);
    reg_audin_aria_set_bclk_oe((mt_u32)0x00000007);
    reg_audin_aria_set_clk_div_cfg((mt_u32)0x001ccccc);
    reg_audin_aria_set_buf0_addr((mt_u32)0x00010000);
    reg_audin_aria_set_buf1_addr((mt_u32)0x00010000);
    reg_audin_aria_set_buf_length((mt_u32)0x00000010);
    reg_audin_aria_set_buf_mode((mt_u32)0x00000011);
    reg_audin_aria_set_cptr_pause((mt_u32)0x00000001);
    reg_audin_aria_set_audon_intr_set((mt_u32)0x00000110);
    reg_audin_aria_set_frm_size((mt_u32)0x00000480);
    reg_audin_aria_set_buf_thd((mt_u32)0x00000018);
    reg_audin_aria_set_axi_wr_cnt((mt_u32)0x00000000);
    reg_audin_aria_set_i2s_size((mt_u32)0x00000020);
    reg_audin_aria_set_buf0_waddr((mt_u32)0x00000000);
    reg_audin_aria_set_buf1_waddr((mt_u32)0x00000000);
    reg_audin_aria_set_res_reg0((mt_u32)0x00000000);
    reg_audin_aria_set_res_reg1((mt_u32)0x00000000);
    /* read read-clear registers in order to set mirror variables */
}

/*!
  end of file
  */

