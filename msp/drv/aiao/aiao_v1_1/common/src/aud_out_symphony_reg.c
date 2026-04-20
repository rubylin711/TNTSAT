/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/delay.h>

#include "mt_type.h"
#include "aud_out_symphony_reg.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

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
  register REG_SYMPHONY_AUD_CLK_ADJ (read/write)
  */
mt_u32   reg_symphony_aud_get_aud_clk_adj(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_CLK_ADJ + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_clk_adj_t *)reg).bitc.samp_rate_offset;
}

/*!
  register REG_SYMPHONY_AUD_SAMP_FRM (read/write)
  */
mt_u32   reg_symphony_aud_get_aud_samp_frm(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_SAMP_FRM + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_samp_frm_t *)reg).bitc.samp_num_perfrm;
}

/*!
  register REG_SYMPHONY_AUD_INTR_SET (read/write)
  */
mt_u32   reg_symphony_aud_get_aud_intr_set(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_symphony_aud_clr_audfrm_intr(mt_u32 data)
{
    reg_symphony_aud_intr_set_t d;
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.all = d.all & 0xffffffc0;
    d.bitc.audfrm_intr = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_symphony_aud_get_audfrm_intr(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_intr_set_t *)reg).bitc.audfrm_intr;
}

mt_void reg_symphony_aud_clr_audfrm_intr_2(mt_u32 data)
{
    reg_symphony_aud_intr_set_t d;
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.all = d.all & 0xffffffc0;
    d.bitc.audfrm_intr_2 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_symphony_aud_get_audfrm_intr_2(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_intr_set_t *)reg).bitc.audfrm_intr_2;
}

mt_void reg_symphony_aud_set_audfrm_intr_mask(mt_u32 data)
{
    reg_symphony_aud_intr_set_t d;
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_intr_mask = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_void reg_symphony_aud_set_intr_mask(mt_u32 data)
{
    reg_symphony_aud_intr_set_t d;
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_intr_mask = (data & 0x1) != 0;
    d.bitc.ppbufw_intr_mask = (data & 0x2) != 0;
    d.bitc.bufrw_intr_mask = (data & 0x4) != 0;
    d.bitc.pcmfifo_emp_intr_mask = (data & 0x8) != 0;
    d.bitc.pcmfifo_diff_intr_mask = (data & 0x10) != 0;
    d.bitc.audfrm_intr_mask_2 = (data & 0x20) != 0;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_symphony_aud_get_audfrm_intr_mask(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_intr_set_t *)reg).bitc.audfrm_intr_mask;
}

mt_void reg_symphony_aud_set_audfrm_intr_2_mask(mt_u32 data)
{
    reg_symphony_aud_intr_set_t d;
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_intr_mask_2 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_symphony_aud_get_audfrm_intr_2_mask(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_intr_set_t *)reg).bitc.audfrm_intr_mask_2;
}

mt_void reg_symphony_aud_set_intr_en(mt_u32 data)
{
    reg_symphony_aud_intr_set_t d;
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_en = data & 0x01;
    d.bitc.ppbuf_w_en = (data & 0x02) != 0;
    d.bitc.buf_rw_en = (data & 0x04) != 0;
    d.bitc.pcmfifo_emp_en = (data & 0x08) != 0;
    d.bitc.pcmfifo_diff_en = (data & 0x10) != 0;
    d.bitc.audfrm_intr2_en = (data & 0x20) != 0;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_void reg_symphony_aud_set_audfrm_intr_en(mt_u32 data)
{
    reg_symphony_aud_intr_set_t d;
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_en = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_symphony_aud_get_audfrm_intr_en(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_intr_set_t *)reg).bitc.audfrm_en;
}

mt_void reg_symphony_aud_set_audfrm_intr_2_en(mt_u32 data)
{
    reg_symphony_aud_intr_set_t d;
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_intr2_en = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_symphony_aud_get_audfrm_intr_2_en(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_intr_set_t *)reg).bitc.audfrm_intr2_en;
}

/*!
  register REG_SYMPHONY_AUD_FRM_INTR_CFG (read/write)
  */
mt_u32   reg_symphony_aud_get_audfrm_intr_cfg_all(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_symphony_aud_set_audfrm_intr_cfg(mt_u32 data)
{
    reg_symphony_aud_frm_intr_cfg_t d;
    ulong reg = REG_SYMPHONY_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_intr_cfg = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_symphony_aud_get_audfrm_intr_cfg(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_frm_intr_cfg_t *)reg).bitc.audfrm_intr_cfg;
}

mt_void reg_symphony_aud_set_audfrm_intr_2_cfg(mt_u32 data)
{
    reg_symphony_aud_frm_intr_cfg_t d;
    ulong reg = REG_SYMPHONY_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_intr_2_cfg = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_symphony_aud_get_audfrm_intr_2_cfg(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_frm_intr_cfg_t *)reg).bitc.audfrm_intr_2_cfg;
}

mt_void reg_symphony_aud_frm_cnt_clr(mt_u32 data)
{
    reg_symphony_aud_frm_intr_cfg_t d;
    ulong reg = REG_SYMPHONY_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.frm_cnt_clr = data;
    d.bitc.frm_cnt_2_clr = 0;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_void reg_symphony_aud_frm_cnt_2_clr(mt_u32 data)
{
    reg_symphony_aud_frm_intr_cfg_t d;
    ulong reg = REG_SYMPHONY_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.frm_cnt_2_clr = data;
    d.bitc.frm_cnt_clr = 0;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

/*!
  register REG_SYMPHONY_AUD_FRMINTR_CNT (read)
  */
mt_u32   reg_symphony_aud_get_audfrm_intr_cnt(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_FRMINTR_CNT + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_frmintr_cnt_t *)reg).bitc.audfrm_intr_cnt;
}

mt_u32   reg_symphony_aud_get_audfrm_intr_2_cnt(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_FRMINTR_CNT + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_frmintr_cnt_t *)reg).bitc.audfrm_intr_2_cnt;
}

/*!
  register REG_SYMPHONY_AUD_FRM_CNT (read)
  */
mt_u32   reg_symphony_aud_get_aud_frm_cnt(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_FRM_CNT + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_frm_cnt_t *)reg).bitc.frame_cnt;
}

/*!
  register REG_SYMPHONY_AUD_FRM_CNT_2 (read)
  */
mt_u32   reg_symphony_aud_get_aud_frm_cnt_2(mt_void)
{
    ulong reg = REG_SYMPHONY_AUD_FRM_CNT_2 + mt_get_audioout_base();
    return (*(volatile reg_symphony_aud_frm_cnt_2_t *)reg).bitc.frame_cnt_2;
}

/*!
  init function
  */
mt_void reg_symphony_aud_init(mt_void)
{
    /* read read-clear registers in order to set mirror variables */
}

/*!
  end of file
  */

