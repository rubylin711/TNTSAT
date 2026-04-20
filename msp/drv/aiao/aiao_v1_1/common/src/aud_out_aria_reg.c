/******************************************************************************/
/* Copyright (c) 2017 Montage Tech - All Rights Reserved                      */
/******************************************************************************/
#include <linux/delay.h>

#include "mt_type.h"
#include "aud_out_aria_reg.h"
#ifdef CONFIG_MT_CHIP_ARIA
#include <mach/aria_io.h>
#else
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif

static inline mt_void hal_put_u32(volatile unsigned long *p_addr, unsigned long data)
{
    /*!
      Write 32 bits register
    */
	HAL_PUT_U32((volatile mt_u32 *)(p_addr), data),
}
static inline mt_u32 hal_get_u32(volatile mt_u32 *p_addr)
{
	return HAL_GET_U32(p_addr);
}

/*!
  register REG_ARIA_AUD_CLK_ADJ (read/write)
  */
mt_u32   reg_aria_aud_get_aud_clk_adj(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_CLK_ADJ + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_clk_adj_t *)reg).bitc.samp_rate_offset;
}

/*!
  register REG_ARIA_AUD_SAMP_FRM (read/write)
  */
mt_u32   reg_aria_aud_get_aud_samp_frm(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_SAMP_FRM + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_samp_frm_t *)reg).bitc.samp_num_perfrm;
}

/*!
  register REG_ARIA_AUD_INTR_SET (read/write)
  */
mt_u32   reg_aria_aud_get_aud_intr_set(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_aria_aud_clr_audfrm_intr(mt_u32 data)
{
    reg_aria_aud_intr_set_t d;
    mt_u32 reg = REG_ARIA_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.all = d.all & 0xffffffc0;
    d.bitc.audfrm_intr = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_aria_aud_get_audfrm_intr(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_intr_set_t *)reg).bitc.audfrm_intr;
}

mt_void reg_aria_aud_clr_audfrm_intr_2(mt_u32 data)
{
    reg_aria_aud_intr_set_t d;
    mt_u32 reg = REG_ARIA_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.all = d.all & 0xffffffc0;
    d.bitc.audfrm_intr_2 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_aria_aud_get_audfrm_intr_2(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_intr_set_t *)reg).bitc.audfrm_intr_2;
}

mt_void reg_aria_aud_set_audfrm_intr_mask(mt_u32 data)
{
    reg_aria_aud_intr_set_t d;
    mt_u32 reg = REG_ARIA_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_intr_mask = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_aria_aud_get_audfrm_intr_mask(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_intr_set_t *)reg).bitc.audfrm_intr_mask;
}

mt_void reg_aria_aud_set_audfrm_intr_2_mask(mt_u32 data)
{
    reg_aria_aud_intr_set_t d;
    mt_u32 reg = REG_ARIA_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_intr_mask_2 = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_aria_aud_get_audfrm_intr_2_mask(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_intr_set_t *)reg).bitc.audfrm_intr_mask_2;
}

mt_void reg_aria_aud_set_audfrm_intr_en(mt_u32 data)
{
    reg_aria_aud_intr_set_t d;
    mt_u32 reg = REG_ARIA_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_en = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_aria_aud_get_audfrm_intr_en(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_intr_set_t *)reg).bitc.audfrm_en;
}

mt_void reg_aria_aud_set_audfrm_intr_2_en(mt_u32 data)
{
    reg_aria_aud_intr_set_t d;
    mt_u32 reg = REG_ARIA_AUD_INTR_SET + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_intr2_en = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_aria_aud_get_audfrm_intr_2_en(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_INTR_SET + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_intr_set_t *)reg).bitc.audfrm_intr2_en;
}

/*!
  register REG_ARIA_AUD_FRM_INTR_CFG (read/write)
  */
mt_u32   reg_aria_aud_get_audfrm_intr_cfg_all(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    return (*(volatile mt_u32 *)reg);
}

mt_void reg_aria_aud_set_audfrm_intr_cfg(mt_u32 data)
{
    reg_aria_aud_frm_intr_cfg_t d;
    mt_u32 reg = REG_ARIA_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_intr_cfg = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_aria_aud_get_audfrm_intr_cfg(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_frm_intr_cfg_t *)reg).bitc.audfrm_intr_cfg;
}

mt_void reg_aria_aud_set_audfrm_intr_2_cfg(mt_u32 data)
{
    reg_aria_aud_frm_intr_cfg_t d;
    mt_u32 reg = REG_ARIA_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.audfrm_intr_2_cfg = data;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_u8   reg_aria_aud_get_audfrm_intr_2_cfg(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_frm_intr_cfg_t *)reg).bitc.audfrm_intr_2_cfg;
}

mt_void reg_aria_aud_frm_cnt_clr(mt_u32 data)
{
    reg_aria_aud_frm_intr_cfg_t d;
    mt_u32 reg = REG_ARIA_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.frm_cnt_clr = data;
    d.bitc.frm_cnt_2_clr = 0;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

mt_void reg_aria_aud_frm_cnt_2_clr(mt_u32 data)
{
    reg_aria_aud_frm_intr_cfg_t d;
    mt_u32 reg = REG_ARIA_AUD_FRM_INTR_CFG + mt_get_audioout_base();
    d.all = *(volatile mt_u32 *)reg;
    d.bitc.frm_cnt_2_clr = data;
    d.bitc.frm_cnt_clr = 0;
    hal_put_u32((volatile unsigned long *)reg, d.all);
}

/*!
  register REG_ARIA_AUD_FRMINTR_CNT (read)
  */
mt_u32   reg_aria_aud_get_audfrm_intr_cnt(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_FRMINTR_CNT + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_frmintr_cnt_t *)reg).bitc.audfrm_intr_cnt;
}

mt_u32   reg_aria_aud_get_audfrm_intr_2_cnt(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_FRMINTR_CNT + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_frmintr_cnt_t *)reg).bitc.audfrm_intr_2_cnt;
}

/*!
  register REG_ARIA_AUD_FRM_CNT (read)
  */
mt_u32   reg_aria_aud_get_aud_frm_cnt(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_FRM_CNT + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_frm_cnt_t *)reg).bitc.frame_cnt;
}

/*!
  register REG_ARIA_AUD_FRM_CNT_2 (read)
  */
mt_u32   reg_aria_aud_get_aud_frm_cnt_2(mt_void)
{
    mt_u32 reg = REG_ARIA_AUD_FRM_CNT_2 + mt_get_audioout_base();
    return (*(volatile reg_aria_aud_frm_cnt_2_t *)reg).bitc.frame_cnt_2;
}

/*!
  init function
  */
mt_void reg_aria_aud_init(mt_void)
{
    /* read read-clear registers in order to set mirror variables */
}

/*!
  end of file
  */

