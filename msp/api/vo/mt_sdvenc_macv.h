/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_SDVENC_MACV_H_
#define __MT_SDVENC_MACV_H_

#include "mt_common.h"

extern ulong p_sdenc_macv_addr;
#define REG_SYMPHONY_SD_ENCODER_MACV_BASE (p_sdenc_macv_addr) 

#define MT_SDVENC_MACV_PRINT printf

#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG			     (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0xa8)  // 0xbf4500a8,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LN_TOP	 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x58) // 0xbf450058,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP	 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x5c) // 0xbf45005c,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT	 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x60) // 0xbf450060,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT	 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x64) // 0xbf450064,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE		 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x68) // 0xbf450068,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD	 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x6c) // 0xbf45006c,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS	 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x70) // 0xbf450070,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA			 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x74) // 0xbf450074,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START	 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x78) // 0xbf450078,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC			 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x7c) // 0xbf45007c,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS	 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x80) // 0xbf450080,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS	 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x84) // 0xbf450084,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A	 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x88) // 0xbf450088,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B	 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x8c) // 0xbf45008c,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS		 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x90) // 0xbf450090,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1		 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x94) // 0xbf450094,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2		 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x98) // 0xbf450098,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3		 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0x9c) // 0xbf45009C,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF		 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0xa0) // 0xbf4500a0,
#define		REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN	 (REG_SYMPHONY_SD_ENCODER_MACV_BASE + 0xa4) // 0xbf4500a4 


/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG
  */
typedef union reg_symphony_sd_encoder_vbi_macv_cfg
{
    mt_u32 all;
    struct
    {
        mt_u32 macv_en                     : 1;
        mt_u32                             : 3;
        mt_u32 cs_en_n                     : 1;
        mt_u32 cs_en_p                     : 1;
        mt_u32 burst_mode                  : 1;
        mt_u32                             : 1;
        mt_u32 n16_advanced_start_n        : 1;
        mt_u32 n16_advanced_start_p        : 1;
        mt_u32                             : 2;
        mt_u32 ps_en_rgb                   : 1;
        mt_u32 ps_en_else                  : 1;
        mt_u32 ps_en_rgb_on_g              : 1;
        mt_u32                             : 1;
        mt_u32 agc_en_rgb                  : 1;
        mt_u32 agc_en_else                 : 1;
        mt_u32                             : 2;
        mt_u32 agc_amp_chs                 : 1;
        mt_u32 agc_amp_test                : 2;
        mt_u32                             : 1;
        mt_u32 bp_en_rgb                   : 1;
        mt_u32 bp_en_else                  : 1;
        mt_u32                             : 2;
        mt_u32 sync_reduce                 : 2;
        mt_u32                             : 2;
    } bitc;
} reg_symphony_sd_encoder_macv_cfg_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LIN_TOP
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n1_cs_fst_lin_top
{
    mt_u32 all;
    struct
    {
        mt_u32 n1_cs_fst_ln_top_n          : 6;
        mt_u32                             : 2;
        mt_u32 n1_cs_fst_ln_top_p          : 6;
        mt_u32                             : 2;
        mt_u32 h_sync_rise_delay           : 2;
        mt_u32                             : 14;
    } bitc;
} reg_symphony_sd_encoder_macv_n1_cs_fst_ln_top_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n2_cs_spc_1_2_top
{
    mt_u32 all;
    struct
    {
        mt_u32 n2_cs_spc_1_2_top_n         : 6;
        mt_u32                             : 2;
        mt_u32 n2_cs_spc_1_2_top_p         : 6;
        mt_u32                             : 18;
    } bitc;
} reg_symphony_sd_encoder_macv_n2_cs_spc_1_2_top_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n3_cs_fst_ln_bot
{
    mt_u32 all;
    struct
    {
        mt_u32 n3_cs_fst_ln_bot_n          : 6;
        mt_u32                             : 2;
        mt_u32 n3_cs_fst_ln_bot_p          : 6;
        mt_u32                             : 2;
        mt_u32 agc_dura_n                  : 7;
        mt_u32                             : 1;
        mt_u32 agc_dura_p                  : 7;
        mt_u32                             : 1;
    } bitc;
} reg_symphony_sd_encoder_macv_n3_cs_fst_ln_bot_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n4_cs_spc_1_2_bot
{
    mt_u32 all;
    struct
    {
        mt_u32 n3_cs_spc_1_2_bot_n         : 6;
        mt_u32                             : 2;
        mt_u32 n3_cs_spc_1_2_bot_p         : 6;
        mt_u32                             : 2;
        mt_u32 non_cs_start_num            : 5;
        mt_u32                             : 11;
    } bitc;
} reg_symphony_sd_encoder_macv_n4_cs_spc_1_2_bot_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n5_cs_spc_else
{
    mt_u32 all;
    struct
    {
        mt_u32 n5_cs_spc_else_n            : 3;
        mt_u32                             : 1;
        mt_u32 n5_cs_spc_else_p            : 3;
        mt_u32                             : 1;
        mt_u32 agc_step_rgb_n              : 9;
        mt_u32                             : 3;
        mt_u32 agc_step_else_n             : 9;
        mt_u32                             : 3;
    } bitc;
} reg_symphony_sd_encoder_macv_n5_cs_spc_else_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n6_cs_num_in_field
{
    mt_u32 all;
    struct
    {
        mt_u32 n6_cs_num_in_field_n        : 3;
        mt_u32                             : 1;
        mt_u32 n6_cs_num_in_field_p        : 3;
        mt_u32                             : 1;
        mt_u32 agc_step_rgb_p              : 9;
        mt_u32                             : 3;
        mt_u32 agc_step_else_p             : 9;
        mt_u32                             : 3;
    } bitc;
} reg_symphony_sd_encoder_macv_n6_cs_num_in_field_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n7_ln_num_in_cs
{
    mt_u32 all;
    struct
    {
        mt_u32 n7_ln_num_in_cs_n           : 3;
        mt_u32                             : 1;
        mt_u32 n7_ln_num_in_cs_p           : 3;
        mt_u32                             : 1;
        mt_u32 agc_puls_hi_p               : 10;
        mt_u32                             : 2;
        mt_u32 agc_puls_lo_p               : 10;
        mt_u32                             : 2;
    } bitc;
} reg_symphony_sd_encoder_macv_n7_ln_num_in_cs_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n8_ps_dura
{
    mt_u32 all;
    struct
    {
        mt_u32 n8_ps_dura_n                : 6;
        mt_u32                             : 2;
        mt_u32 n8_ps_dura_p                : 6;
        mt_u32                             : 18;
    } bitc;
} reg_symphony_sd_encoder_macv_n8_ps_dura_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n9_fst_ps_start
{
    mt_u32 all;
    struct
    {
        mt_u32 n9_fst_ps_start_n           : 6;
        mt_u32                             : 2;
        mt_u32 n9_fst_ps_start_p           : 6;
        mt_u32                             : 18;
    } bitc;
} reg_symphony_sd_encoder_macv_n9_fst_ps_start_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n10_ps_spc
{
    mt_u32 all;
    struct
    {
        mt_u32 n10_ps_spc_n                : 6;
        mt_u32                             : 2;
        mt_u32 n10_ps_spc_p                : 6;
        mt_u32                             : 18;
    } bitc;
} reg_symphony_sd_encoder_macv_n10_ps_spc_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n11_ps_agc_ln_chs
{
    mt_u32 all;
    struct
    {
        mt_u32 n11_ps_agc_ln_chs_n         : 15;
        mt_u32                             : 1;
        mt_u32 n11_ps_agc_ln_chs_p         : 15;
        mt_u32                             : 1;
    } bitc;
} reg_symphony_sd_encoder_macv_n11_ps_agc_ln_chs_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n12_ps_agc_fmt_chs
{
    mt_u32 all;
    struct
    {
        mt_u32 n12_ps_agc_fmt_chs_n        : 15;
        mt_u32                             : 1;
        mt_u32 n12_ps_agc_fmt_chs_p        : 15;
        mt_u32                             : 1;
    } bitc;
} reg_symphony_sd_encoder_macv_n12_ps_agc_fmt_chs_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n13_ps_agc_ivk_a
{
    mt_u32 all;
    struct
    {
        mt_u32 n13_ps_agc_ivk_a_n          : 8;
        mt_u32 n13_ps_agc_ivk_a_p          : 8;
        mt_u32                             : 16;
    } bitc;
} reg_symphony_sd_encoder_macv_n13_ps_agc_ivk_a_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n14_ps_agc_ivk_b
{
    mt_u32 all;
    struct
    {
        mt_u32 n14_ps_agc_ivk_b_n          : 8;
        mt_u32 n14_ps_agc_ivk_b_p          : 8;
        mt_u32                             : 16;
    } bitc;
} reg_symphony_sd_encoder_macv_n14_ps_agc_ivk_b_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n15_bp_ln_chs
{
    mt_u32 all;
    struct
    {
        mt_u32 n15_bp_ln_chs_n             : 8;
        mt_u32 n15_bp_ln_chs_p             : 8;
        mt_u32                             : 16;
    } bitc;
} reg_symphony_sd_encoder_macv_n15_bp_ln_chs_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n17_cs_zone1
{
    mt_u32 all;
    struct
    {
        mt_u32 n17_cs_zone1_n              : 4;
        mt_u32 n17_cs_zone1_p              : 4;
        mt_u32 bp_amp_n                    : 10;
        mt_u32                             : 2;
        mt_u32 bp_amp_p                    : 10;
        mt_u32                             : 2;
    } bitc;
} reg_symphony_sd_encoder_macv_n17_cs_zone1_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n18_cs_zone2
{
    mt_u32 all;
    struct
    {
        mt_u32 n18_cs_zone2_n              : 4;
        mt_u32 n18_cs_zone2_p              : 4;
        mt_u32 bp_dura_num                 : 6;
        mt_u32                             : 18;
    } bitc;
} reg_symphony_sd_encoder_macv_n18_cs_zone2_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n19_cs_zone3
{
    mt_u32 all;
    struct
    {
        mt_u32 n19_cs_zone3_n              : 4;
        mt_u32 n19_cs_zone3_p              : 4;
        mt_u32 sync_reduce_amp_n           : 10;
        mt_u32                             : 2;
        mt_u32 sync_reduce_amp_p           : 10;
        mt_u32                             : 2;
    } bitc;
} reg_symphony_sd_encoder_macv_n19_cs_zone3_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n20_cs_phs_mdf
{
    mt_u32 all;
    struct
    {
        mt_u32 n20_cs_phs_mdf_n            : 3;
        mt_u32                             : 1;
        mt_u32 n20_cs_phs_mdf_p            : 3;
        mt_u32                             : 1;
        mt_u32 non_cs_end_num_p            : 8;
        mt_u32                             : 16;
    } bitc;
} reg_symphony_sd_encoder_macv_n20_cs_phs_mdf_t;

/*!
  the union of register REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN
  */
typedef union reg_symphony_sd_encoder_vbi_macv_n21_cs_ph_mdf_ln
{
    mt_u32 all;
    struct
    {
        mt_u32 n21_cs_ph_mdf_ln_n          : 10;
        mt_u32                             : 2;
        mt_u32 n21_cs_ph_mdf_ln_p          : 10;
        mt_u32                             : 2;
        mt_u32 non_cs_end_num_n            : 8;
    } bitc;
} reg_symphony_sd_encoder_macv_n21_cs_ph_mdf_ln_t;


/*!
  register REGSYMPHONY_SD_ENCODER_MACV_CFG (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_cfg(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_cfg(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_macv_en(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_macv_en(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_cs_en_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_cs_en_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_cs_en_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_cs_en_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_burst_mode(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_burst_mode(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_n16_advanced_start_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_n16_advanced_start_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_n16_advanced_start_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_n16_advanced_start_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_ps_en_rgb(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_ps_en_rgb(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_ps_en_else(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_ps_en_else(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_ps_en_rgb_on_g(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_ps_en_rgb_on_g(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_agc_en_rgb(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_agc_en_rgb(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_agc_en_else(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_agc_en_else(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_agc_amp_chs(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_agc_amp_chs(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_agc_amp_test(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_agc_amp_test(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_bp_en_rgb(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_bp_en_rgb(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_bp_en_else(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_bp_en_else(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_cfg_sync_reduce(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_cfg_sync_reduce(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N1_CS_FST_LN_TOP (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n1_cs_fst_ln_top(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n1_cs_fst_ln_top(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n1_cs_fst_ln_top_n1_cs_fst_ln_top_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n1_cs_fst_ln_top_h_sync_rise_delay(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n1_cs_fst_ln_top_h_sync_rise_delay(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N2_CS_SPC_1_2_TOP (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n2_cs_spc_1_2_top(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n2_cs_spc_1_2_top(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n2_cs_spc_1_2_top_n2_cs_spc_1_2_top_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N3_CS_FST_LN_BOT (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n3_cs_fst_ln_bot(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot_n3_cs_fst_ln_bot_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n3_cs_fst_ln_bot_agc_dura_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot_agc_dura_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n3_cs_fst_ln_bot_agc_dura_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n3_cs_fst_ln_bot_agc_dura_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N4_CS_SPC_1_2_BOT (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n4_cs_spc_1_2_bot(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n4_cs_spc_1_2_bot(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n4_cs_spc_1_2_bot_n3_cs_spc_1_2_bot_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n4_cs_spc_1_2_bot_non_cs_start_num(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n4_cs_spc_1_2_bot_non_cs_start_num(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N5_CS_SPC_ELSE (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n5_cs_spc_else(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n5_cs_spc_else(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n5_cs_spc_else_n5_cs_spc_else_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n5_cs_spc_else_n5_cs_spc_else_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n5_cs_spc_else_n5_cs_spc_else_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n5_cs_spc_else_n5_cs_spc_else_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n5_cs_spc_else_agc_step_rgb_n(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n5_cs_spc_else_agc_step_rgb_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n5_cs_spc_else_agc_step_else_n(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n5_cs_spc_else_agc_step_else_n(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N6_CS_NUM_IN_FIELD (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n6_cs_num_in_field(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n6_cs_num_in_field_n6_cs_num_in_field_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field_n6_cs_num_in_field_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n6_cs_num_in_field_n6_cs_num_in_field_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field_n6_cs_num_in_field_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n6_cs_num_in_field_agc_step_rgb_p(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field_agc_step_rgb_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n6_cs_num_in_field_agc_step_else_p(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n6_cs_num_in_field_agc_step_else_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N7_LN_NUM_IN_CS (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n7_ln_num_in_cs(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs_n7_ln_num_in_cs_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n7_ln_num_in_cs_agc_puls_hi_p(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs_agc_puls_hi_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n7_ln_num_in_cs_agc_puls_lo_p(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n7_ln_num_in_cs_agc_puls_lo_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N8_PS_DURA (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n8_ps_dura(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n8_ps_dura(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n8_ps_dura_n8_ps_dura_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n8_ps_dura_n8_ps_dura_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n8_ps_dura_n8_ps_dura_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n8_ps_dura_n8_ps_dura_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N9_FST_PS_START (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n9_fst_ps_start(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n9_fst_ps_start(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n9_fst_ps_start_n9_fst_ps_start_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n9_fst_ps_start_n9_fst_ps_start_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n9_fst_ps_start_n9_fst_ps_start_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n9_fst_ps_start_n9_fst_ps_start_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N10_PS_SPC (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n10_ps_spc(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n10_ps_spc(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n10_ps_spc_n10_ps_spc_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n10_ps_spc_n10_ps_spc_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n10_ps_spc_n10_ps_spc_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n10_ps_spc_n10_ps_spc_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N11_PS_AGC_LN_CHS (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n11_ps_agc_ln_chs(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n11_ps_agc_ln_chs(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_n(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_p(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n11_ps_agc_ln_chs_n11_ps_agc_ln_chs_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N12_PS_AGC_FMT_CHS (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n12_ps_agc_fmt_chs(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n12_ps_agc_fmt_chs(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_n(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_p(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n12_ps_agc_fmt_chs_n12_ps_agc_fmt_chs_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N13_PS_AGC_IVK_A (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n13_ps_agc_ivk_a(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n13_ps_agc_ivk_a(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n13_ps_agc_ivk_a_n13_ps_agc_ivk_a_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N14_PS_AGC_IVK_B (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n14_ps_agc_ivk_b(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n14_ps_agc_ivk_b(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n14_ps_agc_ivk_b_n14_ps_agc_ivk_b_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N15_BP_LN_CHS (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n15_bp_ln_chs(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n15_bp_ln_chs(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n15_bp_ln_chs_n15_bp_ln_chs_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n15_bp_ln_chs_n15_bp_ln_chs_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n15_bp_ln_chs_n15_bp_ln_chs_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n15_bp_ln_chs_n15_bp_ln_chs_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N17_CS_ZONE1 (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n17_cs_zone1(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n17_cs_zone1(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n17_cs_zone1_n17_cs_zone1_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n17_cs_zone1_n17_cs_zone1_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n17_cs_zone1_n17_cs_zone1_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n17_cs_zone1_n17_cs_zone1_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n17_cs_zone1_bp_amp_n(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n17_cs_zone1_bp_amp_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n17_cs_zone1_bp_amp_p(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n17_cs_zone1_bp_amp_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N18_CS_ZONE2 (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n18_cs_zone2(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n18_cs_zone2(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n18_cs_zone2_n18_cs_zone2_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n18_cs_zone2_n18_cs_zone2_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n18_cs_zone2_n18_cs_zone2_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n18_cs_zone2_n18_cs_zone2_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n18_cs_zone2_bp_dura_num(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n18_cs_zone2_bp_dura_num(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N19_CS_ZONE3 (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n19_cs_zone3(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n19_cs_zone3(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n19_cs_zone3_n19_cs_zone3_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n19_cs_zone3_n19_cs_zone3_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n19_cs_zone3_n19_cs_zone3_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n19_cs_zone3_n19_cs_zone3_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n19_cs_zone3_sync_reduce_amp_n(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n19_cs_zone3_sync_reduce_amp_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n19_cs_zone3_sync_reduce_amp_p(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n19_cs_zone3_sync_reduce_amp_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N20_CS_PHS_MDF (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n20_cs_phs_mdf(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n20_cs_phs_mdf(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n20_cs_phs_mdf_n20_cs_phs_mdf_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n20_cs_phs_mdf_non_cs_end_num_p(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n20_cs_phs_mdf_non_cs_end_num_p(mt_void);

/*!
  register REGSYMPHONY_SD_ENCODER_MACV_N21_CS_PH_MDF_LN (read/write)
  */
mt_void reg_symphony_sd_encoder_set_macv_n21_cs_ph_mdf_ln(mt_u32 data);
mt_u32  reg_symphony_sd_encoder_get_macv_n21_cs_ph_mdf_ln(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_n(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_n(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_p(mt_u16 data);
mt_u16  reg_symphony_sd_encoder_get_macv_n21_cs_ph_mdf_ln_n21_cs_ph_mdf_ln_p(mt_void);
mt_void reg_symphony_sd_encoder_set_macv_n21_cs_ph_mdf_ln_non_cs_end_num_n(mt_u8 data);
mt_u8   reg_symphony_sd_encoder_get_macv_n21_cs_ph_mdf_ln_non_cs_end_num_n(mt_void);



#endif



