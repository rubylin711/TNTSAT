/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


/*************************** macrovison  start *****************************/

#include "mt_sdvenc_macv.h"

static inline void hal_put_u32(volatile unsigned long *p_addr, unsigned long data)
{
  /*!
      Write 32 bits register
    */
 // *p_addr = data;
    *((volatile mt_u32 *)(p_addr)) = data;
}

/*!
  register SYMPHONY_SD_ENCODER_MACV_CFG (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_cfg(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_cfg(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG);
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_macv_en(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.macv_en = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_macv_en(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.macv_en;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_cs_en_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.cs_en_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_cs_en_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.cs_en_n;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_cs_en_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.cs_en_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_cs_en_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.cs_en_p;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_burst_mode(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.burst_mode = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_burst_mode(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.burst_mode;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_n16_advanced_start_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.n16_advanced_start_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_n16_advanced_start_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.n16_advanced_start_n;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_n16_advanced_start_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.n16_advanced_start_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_n16_advanced_start_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.n16_advanced_start_p;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_ps_en_rgb(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.ps_en_rgb = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_ps_en_rgb(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.ps_en_rgb;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_ps_en_else(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.ps_en_else = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_ps_en_else(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.ps_en_else;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_ps_en_rgb_on_g(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.ps_en_rgb_on_g = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_ps_en_rgb_on_g(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.ps_en_rgb_on_g;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_agc_en_rgb(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.agc_en_rgb = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_agc_en_rgb(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.agc_en_rgb;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_agc_en_else(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.agc_en_else = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_agc_en_else(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.agc_en_else;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_agc_amp_chs(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.agc_amp_chs = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_agc_amp_chs(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.agc_amp_chs;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_agc_amp_test(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.agc_amp_test = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_agc_amp_test(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.agc_amp_test;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_bp_en_rgb(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.bp_en_rgb = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_bp_en_rgb(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.bp_en_rgb;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_bp_en_else(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.bp_en_else = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_bp_en_else(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.bp_en_else;
}

mt_void reg_symphony_sd_encoder_set_macv_cfg_sync_reduce(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_cfg_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG;
    d.bitc.sync_reduce = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_cfg_sync_reduce(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_cfg_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG).bitc.sync_reduce;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N1_CS_FST_LN_TOP (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n1_cs_fst_ln_top(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LN_TOP, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n1_cs_fst_ln_top(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LN_TOP);
}

mt_void reg_symphony_sd_encoder_set_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n1_cs_fst_ln_top_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LN_TOP;
    d.bitc.n1_cs_fst_ln_top_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LN_TOP, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n1_cs_fst_ln_top_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LN_TOP).bitc.n1_cs_fst_ln_top_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n1_cs_fst_ln_top_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LN_TOP;
    d.bitc.n1_cs_fst_ln_top_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LN_TOP, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n1_cs_fst_ln_top_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LN_TOP).bitc.n1_cs_fst_ln_top_p;
}

mt_void reg_symphony_sd_encoder_set_macv_n1_cs_fst_ln_top_h_sync_rise_delay(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n1_cs_fst_ln_top_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LN_TOP;
    d.bitc.h_sync_rise_delay = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LN_TOP, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n1_cs_fst_ln_top_h_sync_rise_delay(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n1_cs_fst_ln_top_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LN_TOP).bitc.h_sync_rise_delay;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N2_CS_SPC_1_2_TOP (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n2_cs_spc_1_2_top(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n2_cs_spc_1_2_top(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP);
}

mt_void reg_symphony_sd_encoder_set_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n2_cs_spc_1_2_top_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP;
    d.bitc.n2_cs_spc_1_2_top_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n2_cs_spc_1_2_top_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP).bitc.n2_cs_spc_1_2_top_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n2_cs_spc_1_2_top_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP;
    d.bitc.n2_cs_spc_1_2_top_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n2_cs_spc_1_2_top_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP).bitc.n2_cs_spc_1_2_top_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N3_CS_FST_LN_BOT (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n3_cs_fst_ln_bot(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT);
}

mt_void reg_symphony_sd_encoder_set_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n3_cs_fst_ln_bot_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT;
    d.bitc.n3_cs_fst_ln_bot_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n3_cs_fst_ln_bot_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT).bitc.n3_cs_fst_ln_bot_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n3_cs_fst_ln_bot_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT;
    d.bitc.n3_cs_fst_ln_bot_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n3_cs_fst_ln_bot_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT).bitc.n3_cs_fst_ln_bot_p;
}

mt_void reg_symphony_sd_encoder_set_macv_n3_cs_fst_ln_bot_agc_dura_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n3_cs_fst_ln_bot_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT;
    d.bitc.agc_dura_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot_agc_dura_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n3_cs_fst_ln_bot_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT).bitc.agc_dura_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n3_cs_fst_ln_bot_agc_dura_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n3_cs_fst_ln_bot_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT;
    d.bitc.agc_dura_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot_agc_dura_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n3_cs_fst_ln_bot_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT).bitc.agc_dura_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N4_CS_SPC_1_2_BOT (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n4_cs_spc_1_2_bot(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n4_cs_spc_1_2_bot(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT);
}

mt_void reg_symphony_sd_encoder_set_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n4_cs_spc_1_2_bot_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT;
    d.bitc.n3_cs_spc_1_2_bot_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n4_cs_spc_1_2_bot_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT).bitc.n3_cs_spc_1_2_bot_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n4_cs_spc_1_2_bot_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT;
    d.bitc.n3_cs_spc_1_2_bot_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n4_cs_spc_1_2_bot_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT).bitc.n3_cs_spc_1_2_bot_p;
}

mt_void reg_symphony_sd_encoder_set_macv_n4_cs_spc_1_2_bot_non_cs_start_num(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n4_cs_spc_1_2_bot_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT;
    d.bitc.non_cs_start_num = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n4_cs_spc_1_2_bot_non_cs_start_num(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n4_cs_spc_1_2_bot_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT).bitc.non_cs_start_num;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N5_CS_SPC_ELSE (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n5_cs_spc_else(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n5_cs_spc_else(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE);
}

mt_void reg_symphony_sd_encoder_set_macv_n5_cs_spc_else_n5_cs_spc_else_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n5_cs_spc_else_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE;
    d.bitc.n5_cs_spc_else_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n5_cs_spc_else_n5_cs_spc_else_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n5_cs_spc_else_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE).bitc.n5_cs_spc_else_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n5_cs_spc_else_n5_cs_spc_else_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n5_cs_spc_else_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE;
    d.bitc.n5_cs_spc_else_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n5_cs_spc_else_n5_cs_spc_else_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n5_cs_spc_else_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE).bitc.n5_cs_spc_else_p;
}

mt_void reg_symphony_sd_encoder_set_macv_n5_cs_spc_else_agc_step_rgb_n(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n5_cs_spc_else_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE;
    d.bitc.agc_step_rgb_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n5_cs_spc_else_agc_step_rgb_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n5_cs_spc_else_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE).bitc.agc_step_rgb_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n5_cs_spc_else_agc_step_else_n(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n5_cs_spc_else_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE;
    d.bitc.agc_step_else_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n5_cs_spc_else_agc_step_else_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n5_cs_spc_else_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE).bitc.agc_step_else_n;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N6_CS_NUM_IN_FIELD (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n6_cs_num_in_field(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD);
}

mt_void reg_symphony_sd_encoder_set_macv_n6_cs_num_in_field_n6_cs_num_in_field_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n6_cs_num_in_field_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD;
    d.bitc.n6_cs_num_in_field_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field_n6_cs_num_in_field_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n6_cs_num_in_field_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD).bitc.n6_cs_num_in_field_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n6_cs_num_in_field_n6_cs_num_in_field_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n6_cs_num_in_field_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD;
    d.bitc.n6_cs_num_in_field_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field_n6_cs_num_in_field_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n6_cs_num_in_field_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD).bitc.n6_cs_num_in_field_p;
}

mt_void reg_symphony_sd_encoder_set_macv_n6_cs_num_in_field_agc_step_rgb_p(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n6_cs_num_in_field_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD;
    d.bitc.agc_step_rgb_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field_agc_step_rgb_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n6_cs_num_in_field_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD).bitc.agc_step_rgb_p;
}

mt_void reg_symphony_sd_encoder_set_macv_n6_cs_num_in_field_agc_step_else_p(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n6_cs_num_in_field_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD;
    d.bitc.agc_step_else_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field_agc_step_else_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n6_cs_num_in_field_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD).bitc.agc_step_else_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N7_LN_NUM_IN_CS (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n7_ln_num_in_cs(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS);
}

mt_void reg_symphony_sd_encoder_set_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n7_ln_num_in_cs_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS;
    d.bitc.n7_ln_num_in_cs_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n7_ln_num_in_cs_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS).bitc.n7_ln_num_in_cs_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n7_ln_num_in_cs_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS;
    d.bitc.n7_ln_num_in_cs_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n7_ln_num_in_cs_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS).bitc.n7_ln_num_in_cs_p;
}

mt_void reg_symphony_sd_encoder_set_macv_n7_ln_num_in_cs_agc_puls_hi_p(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n7_ln_num_in_cs_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS;
    d.bitc.agc_puls_hi_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs_agc_puls_hi_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n7_ln_num_in_cs_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS).bitc.agc_puls_hi_p;
}

mt_void reg_symphony_sd_encoder_set_macv_n7_ln_num_in_cs_agc_puls_lo_p(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n7_ln_num_in_cs_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS;
    d.bitc.agc_puls_lo_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs_agc_puls_lo_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n7_ln_num_in_cs_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS).bitc.agc_puls_lo_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N8_PS_DURA (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n8_ps_dura(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n8_ps_dura(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA);
}

mt_void reg_symphony_sd_encoder_set_macv_n8_ps_dura_n8_ps_dura_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n8_ps_dura_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA;
    d.bitc.n8_ps_dura_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n8_ps_dura_n8_ps_dura_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n8_ps_dura_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA).bitc.n8_ps_dura_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n8_ps_dura_n8_ps_dura_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n8_ps_dura_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA;
    d.bitc.n8_ps_dura_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n8_ps_dura_n8_ps_dura_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n8_ps_dura_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA).bitc.n8_ps_dura_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N9_FST_PS_START (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n9_fst_ps_start(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n9_fst_ps_start(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START);
}

mt_void reg_symphony_sd_encoder_set_macv_n9_fst_ps_start_n9_fst_ps_start_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n9_fst_ps_start_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START;
    d.bitc.n9_fst_ps_start_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n9_fst_ps_start_n9_fst_ps_start_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n9_fst_ps_start_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START).bitc.n9_fst_ps_start_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n9_fst_ps_start_n9_fst_ps_start_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n9_fst_ps_start_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START;
    d.bitc.n9_fst_ps_start_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n9_fst_ps_start_n9_fst_ps_start_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n9_fst_ps_start_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START).bitc.n9_fst_ps_start_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N10_PS_SPC (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n10_ps_spc(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n10_ps_spc(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC);
}

mt_void reg_symphony_sd_encoder_set_macv_n10_ps_spc_n10_ps_spc_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n10_ps_spc_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC;
    d.bitc.n10_ps_spc_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n10_ps_spc_n10_ps_spc_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n10_ps_spc_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC).bitc.n10_ps_spc_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n10_ps_spc_n10_ps_spc_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n10_ps_spc_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC;
    d.bitc.n10_ps_spc_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n10_ps_spc_n10_ps_spc_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n10_ps_spc_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC).bitc.n10_ps_spc_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N11_PS_AGC_LN_CHS (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n11_ps_agc_ln_chs(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n11_ps_agc_ln_chs(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS);
}

mt_void reg_symphony_sd_encoder_set_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_n(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n11_ps_agc_ln_chs_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS;
    d.bitc.n11_ps_agc_ln_chs_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n11_ps_agc_ln_chs_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS).bitc.n11_ps_agc_ln_chs_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_p(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n11_ps_agc_ln_chs_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS;
    d.bitc.n11_ps_agc_ln_chs_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n11_ps_agc_ln_chs_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS).bitc.n11_ps_agc_ln_chs_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N12_PS_AGC_FMT_CHS (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n12_ps_agc_fmt_chs(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n12_ps_agc_fmt_chs(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS);
}

mt_void reg_symphony_sd_encoder_set_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_n(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n12_ps_agc_fmt_chs_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS;
    d.bitc.n12_ps_agc_fmt_chs_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n12_ps_agc_fmt_chs_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS).bitc.n12_ps_agc_fmt_chs_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_p(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n12_ps_agc_fmt_chs_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS;
    d.bitc.n12_ps_agc_fmt_chs_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n12_ps_agc_fmt_chs_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS).bitc.n12_ps_agc_fmt_chs_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N13_PS_AGC_IVK_A (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n13_ps_agc_ivk_a(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n13_ps_agc_ivk_a(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A);
}

mt_void reg_symphony_sd_encoder_set_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n13_ps_agc_ivk_a_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A;
    d.bitc.n13_ps_agc_ivk_a_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n13_ps_agc_ivk_a_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A).bitc.n13_ps_agc_ivk_a_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n13_ps_agc_ivk_a_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A;
    d.bitc.n13_ps_agc_ivk_a_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n13_ps_agc_ivk_a_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A).bitc.n13_ps_agc_ivk_a_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N14_PS_AGC_IVK_B (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n14_ps_agc_ivk_b(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n14_ps_agc_ivk_b(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B);
}

mt_void reg_symphony_sd_encoder_set_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n14_ps_agc_ivk_b_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B;
    d.bitc.n14_ps_agc_ivk_b_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n14_ps_agc_ivk_b_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B).bitc.n14_ps_agc_ivk_b_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n14_ps_agc_ivk_b_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B;
    d.bitc.n14_ps_agc_ivk_b_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n14_ps_agc_ivk_b_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B).bitc.n14_ps_agc_ivk_b_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N15_BP_LN_CHS (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n15_bp_ln_chs(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n15_bp_ln_chs(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS);
}

mt_void reg_symphony_sd_encoder_set_macv_n15_bp_ln_chs_n15_bp_ln_chs_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n15_bp_ln_chs_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS;
    d.bitc.n15_bp_ln_chs_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n15_bp_ln_chs_n15_bp_ln_chs_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n15_bp_ln_chs_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS).bitc.n15_bp_ln_chs_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n15_bp_ln_chs_n15_bp_ln_chs_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n15_bp_ln_chs_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS;
    d.bitc.n15_bp_ln_chs_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n15_bp_ln_chs_n15_bp_ln_chs_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n15_bp_ln_chs_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS).bitc.n15_bp_ln_chs_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N17_CS_ZONE1 (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n17_cs_zone1(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n17_cs_zone1(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1);
}

mt_void reg_symphony_sd_encoder_set_macv_n17_cs_zone1_n17_cs_zone1_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n17_cs_zone1_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1;
    d.bitc.n17_cs_zone1_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n17_cs_zone1_n17_cs_zone1_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n17_cs_zone1_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1).bitc.n17_cs_zone1_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n17_cs_zone1_n17_cs_zone1_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n17_cs_zone1_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1;
    d.bitc.n17_cs_zone1_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n17_cs_zone1_n17_cs_zone1_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n17_cs_zone1_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1).bitc.n17_cs_zone1_p;
}

mt_void reg_symphony_sd_encoder_set_macv_n17_cs_zone1_bp_amp_n(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n17_cs_zone1_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1;
    d.bitc.bp_amp_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n17_cs_zone1_bp_amp_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n17_cs_zone1_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1).bitc.bp_amp_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n17_cs_zone1_bp_amp_p(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n17_cs_zone1_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1;
    d.bitc.bp_amp_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n17_cs_zone1_bp_amp_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n17_cs_zone1_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1).bitc.bp_amp_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N18_CS_ZONE2 (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n18_cs_zone2(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n18_cs_zone2(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2);
}

mt_void reg_symphony_sd_encoder_set_macv_n18_cs_zone2_n18_cs_zone2_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n18_cs_zone2_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2;
    d.bitc.n18_cs_zone2_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n18_cs_zone2_n18_cs_zone2_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n18_cs_zone2_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2).bitc.n18_cs_zone2_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n18_cs_zone2_n18_cs_zone2_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n18_cs_zone2_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2;
    d.bitc.n18_cs_zone2_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n18_cs_zone2_n18_cs_zone2_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n18_cs_zone2_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2).bitc.n18_cs_zone2_p;
}

mt_void reg_symphony_sd_encoder_set_macv_n18_cs_zone2_bp_dura_num(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n18_cs_zone2_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2;
    d.bitc.bp_dura_num = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n18_cs_zone2_bp_dura_num(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n18_cs_zone2_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2).bitc.bp_dura_num;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N19_CS_ZONE3 (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n19_cs_zone3(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n19_cs_zone3(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3);
}

mt_void reg_symphony_sd_encoder_set_macv_n19_cs_zone3_n19_cs_zone3_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n19_cs_zone3_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3;
    d.bitc.n19_cs_zone3_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n19_cs_zone3_n19_cs_zone3_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n19_cs_zone3_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3).bitc.n19_cs_zone3_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n19_cs_zone3_n19_cs_zone3_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n19_cs_zone3_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3;
    d.bitc.n19_cs_zone3_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n19_cs_zone3_n19_cs_zone3_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n19_cs_zone3_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3).bitc.n19_cs_zone3_p;
}

mt_void reg_symphony_sd_encoder_set_macv_n19_cs_zone3_sync_reduce_amp_n(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n19_cs_zone3_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3;
    d.bitc.sync_reduce_amp_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n19_cs_zone3_sync_reduce_amp_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n19_cs_zone3_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3).bitc.sync_reduce_amp_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n19_cs_zone3_sync_reduce_amp_p(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n19_cs_zone3_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3;
    d.bitc.sync_reduce_amp_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n19_cs_zone3_sync_reduce_amp_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n19_cs_zone3_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3).bitc.sync_reduce_amp_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N20_CS_PHS_MDF (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n20_cs_phs_mdf(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n20_cs_phs_mdf(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF);
}

mt_void reg_symphony_sd_encoder_set_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n20_cs_phs_mdf_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF;
    d.bitc.n20_cs_phs_mdf_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n20_cs_phs_mdf_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF).bitc.n20_cs_phs_mdf_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n20_cs_phs_mdf_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF;
    d.bitc.n20_cs_phs_mdf_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n20_cs_phs_mdf_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF).bitc.n20_cs_phs_mdf_p;
}

mt_void reg_symphony_sd_encoder_set_macv_n20_cs_phs_mdf_non_cs_end_num_p(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n20_cs_phs_mdf_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF;
    d.bitc.non_cs_end_num_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n20_cs_phs_mdf_non_cs_end_num_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n20_cs_phs_mdf_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF).bitc.non_cs_end_num_p;
}


/*!
  register SYMPHONY_SD_ENCODER_MACV_N21_CS_PH_MDF_LN (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n21_cs_ph_mdf_ln(mt_u32 data)
{
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN, data);
}

mt_u32  reg_symphony_sd_encoder_get_macv_n21_cs_ph_mdf_ln(mt_void)
{
    return (*(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN);
}

mt_void reg_symphony_sd_encoder_set_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_n(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n21_cs_ph_mdf_ln_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN;
    d.bitc.n21_cs_ph_mdf_ln_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n21_cs_ph_mdf_ln_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN).bitc.n21_cs_ph_mdf_ln_n;
}

mt_void reg_symphony_sd_encoder_set_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_p(mt_u16 data)
{
    reg_symphony_sd_encoder_macv_n21_cs_ph_mdf_ln_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN;
    d.bitc.n21_cs_ph_mdf_ln_p = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN, d.all);
}

mt_u16  reg_symphony_sd_encoder_get_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_p(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n21_cs_ph_mdf_ln_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN).bitc.n21_cs_ph_mdf_ln_p;
}

mt_void reg_symphony_sd_encoder_set_macv_n21_cs_ph_mdf_ln_non_cs_end_num_n(mt_u8 data)
{
    reg_symphony_sd_encoder_macv_n21_cs_ph_mdf_ln_t d;
    d.all = *(volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN;
    d.bitc.non_cs_end_num_n = data;
    hal_put_u32((volatile unsigned long *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN, d.all);
}

mt_u8   reg_symphony_sd_encoder_get_macv_n21_cs_ph_mdf_ln_non_cs_end_num_n(mt_void)
{
    return (*(volatile reg_symphony_sd_encoder_macv_n21_cs_ph_mdf_ln_t *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN).bitc.non_cs_end_num_n;
}

/**************************  macrovison  end ******************************/



