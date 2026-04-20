/******************************************************************************/
/* Copyright (c) 2012 Montage Tech - All Rights Reserved                      */
/******************************************************************************/
#ifndef _AUD_IN_ARIA_REG_H
#define _AUD_IN_ARIA_REG_H


/*!
  the enum of AUDIN_ARIA registers
  */
enum
{
    REG_AUDIN_ARIA_CPTR_EN              = 0x0,
    REG_AUDIN_ARIA_I2S_FMT              = 0x4,
    REG_AUDIN_ARIA_BCLK_OE              = 0x8,
    REG_AUDIN_ARIA_CLK_DIV_CFG          = 0xc,
    REG_AUDIN_ARIA_BUF0_ADDR            = 0x10,
    REG_AUDIN_ARIA_BUF1_ADDR            = 0x14,
    REG_AUDIN_ARIA_BUF_LENGTH           = 0x18,
    REG_AUDIN_ARIA_BUF_MODE             = 0x1c,
    REG_AUDIN_ARIA_CPTR_PAUSE           = 0x20,
    REG_AUDIN_ARIA_AUDON_INTR_SET       = 0x24,
    REG_AUDIN_ARIA_FRM_SIZE             = 0x28,
    REG_AUDIN_ARIA_BUF_THD              = 0x2c,
    REG_AUDIN_ARIA_AXI_WR_CNT           = 0x80,
    REG_AUDIN_ARIA_FIFO_CNT             = 0x84,
    REG_AUDIN_ARIA_FRM_CNT              = 0x88,
    REG_AUDIN_ARIA_I2S_SIZE             = 0x8c,
    REG_AUDIN_ARIA_BUF0_WADDR           = 0x90,
    REG_AUDIN_ARIA_BUF1_WADDR           = 0x94,
    REG_AUDIN_ARIA_RES_REG0             = 0x100,
    REG_AUDIN_ARIA_RES_REG1             = 0x104 
};

/*!
  the union of register reg_audin_aria_cptr_en
  */
typedef union reg_audin_aria_cptr_en
{
    mt_u32 all;
    struct
    {
        mt_u32 audio_cptr_en               : 1;
        mt_u32                             : 7;
        mt_u32 axi_bus_end_flag            : 1;
        mt_u32                             : 7;
        mt_u32 audin_cptr_end_flag         : 1;
        mt_u32                             : 15;
    } bitc;
} reg_audin_aria_cptr_en_t;

/*!
  the union of register reg_audin_aria_i2s_fmt
  */
typedef union reg_audin_aria_i2s_fmt
{
    mt_u32 all;
    struct
    {
        mt_u32 just                        : 1;
        mt_u32 ws_pol                      : 1;
        mt_u32 shift                       : 1;
        mt_u32                             : 1;
        mt_u32 little_endian               : 1;
        mt_u32                             : 27;
    } bitc;
} reg_audin_aria_i2s_fmt_t;

/*!
  the union of register reg_audin_aria_bclk_oe
  */
typedef union reg_audin_aria_bclk_oe
{
    mt_u32 all;
    struct
    {
        mt_u32 bclk_oe                     : 1;
        mt_u32 ws_oe                       : 1;
        mt_u32 mclk_oe                     : 1;
        mt_u32                             : 29;
    } bitc;
} reg_audin_aria_bclk_oe_t;

/*!
  the union of register reg_audin_aria_clk_div_cfg
  */
typedef union reg_audin_aria_clk_div_cfg
{
    mt_u32 all;
    struct
    {
        mt_u32 clk_divider_factor          : 25;
        mt_u32                             : 7;
    } bitc;
} reg_audin_aria_clk_div_cfg_t;

/*!
  the union of register reg_audin_aria_buf0_addr
  */
typedef union reg_audin_aria_buf0_addr
{
    mt_u32 all;
    struct
    {
        mt_u32 buf0_addr                   : 28;
        mt_u32                             : 4;
    } bitc;
} reg_audin_aria_buf0_addr_t;

/*!
  the union of register reg_audin_aria_buf1_addr
  */
typedef union reg_audin_aria_buf1_addr
{
    mt_u32 all;
    struct
    {
        mt_u32 buf1_addr                   : 28;
        mt_u32                             : 4;
    } bitc;
} reg_audin_aria_buf1_addr_t;

/*!
  the union of register reg_audin_aria_buf_length
  */
typedef union reg_audin_aria_buf_length
{
    mt_u32 all;
    struct
    {
        mt_u32 buf_len                     : 20;
        mt_u32                             : 12;
    } bitc;
} reg_audin_aria_buf_length_t;

/*!
  the union of register reg_audin_aria_buf_mode
  */
typedef union reg_audin_aria_buf_mode
{
    mt_u32 all;
    struct
    {
        mt_u32 interlace_mode              : 1;
        mt_u32                             : 3;
        mt_u32 addr_incr_mode              : 1;
        mt_u32                             : 3;
        mt_u32 audio_play_mode             : 1;
        mt_u32                             : 23;
    } bitc;
} reg_audin_aria_buf_mode_t;

/*!
  the union of register reg_audin_aria_cptr_pause
  */
typedef union reg_audin_aria_cptr_pause
{
    mt_u32 all;
    struct
    {
        mt_u32 audio_in_cptr_pause         : 1;
        mt_u32                             : 31;
    } bitc;
} reg_audin_aria_cptr_pause_t;

/*!
  the union of register reg_audin_aria_audon_intr_set
  */
typedef union reg_audin_aria_audon_intr_set
{
    mt_u32 all;
    struct
    {
        mt_u32 i_frm_interrupt             : 1;
        mt_u32                             : 3;
        mt_u32 frm_intr_en                 : 1;
        mt_u32                             : 3;
        mt_u32 frm_intr_mask               : 1;
        mt_u32                             : 23;
    } bitc;
} reg_audin_aria_audon_intr_set_t;

/*!
  the union of register reg_audin_aria_frm_size
  */
typedef union reg_audin_aria_frm_size
{
    mt_u32 all;
    struct
    {
        mt_u32 frm_size                    : 14;
        mt_u32                             : 18;
    } bitc;
} reg_audin_aria_frm_size_t;

/*!
  the union of register reg_audin_aria_buf_thd
  */
typedef union reg_audin_aria_buf_thd
{
    mt_u32 all;
    struct
    {
        mt_u32 buf_urgent_thd              : 5;
        mt_u32                             : 27;
    } bitc;
} reg_audin_aria_buf_thd_t;

/*!
  the union of register reg_audin_aria_axi_wr_cnt
  */
typedef union reg_audin_aria_axi_wr_cnt
{
    mt_u32 all;
    struct
    {
        mt_u32 mem_wr_cnt_0                : 6;
        mt_u32                             : 2;
        mt_u32 mem_wr_cnt_1                : 6;
        mt_u32                             : 2;
        mt_u32 aw_valid_buf0               : 1;
        mt_u32 w_valid_buf0                : 1;
        mt_u32 aw_valid_buf1               : 1;
        mt_u32 w_valid_buf1                : 1;
        mt_u32 b_valid_flag_0              : 1;
        mt_u32 b_valid_flag_1              : 1;
        mt_u32                             : 2;
        mt_u32 mem_wr_b_cnt_0              : 4;
        mt_u32 mem_wr_b_cnt_1              : 4;
    } bitc;
} reg_audin_aria_axi_wr_cnt_t;

/*!
  the union of register reg_audin_aria_fifo_cnt
  */
typedef union reg_audin_aria_fifo_cnt
{
    mt_u32 all;
    struct
    {
        mt_u32 left_fifo_cnt               : 6;
        mt_u32                             : 2;
        mt_u32 right_fifo_cnt              : 6;
        mt_u32                             : 18;
    } bitc;
} reg_audin_aria_fifo_cnt_t;

/*!
  the union of register reg_audin_aria_frm_cnt
  */
typedef union reg_audin_aria_frm_cnt
{
    mt_u32 all;
    struct
    {
        mt_u32 frm_cnt                     : 21;
        mt_u32                             : 11;
    } bitc;
} reg_audin_aria_frm_cnt_t;

/*!
  the union of register reg_audin_aria_i2s_size
  */
typedef union reg_audin_aria_i2s_size
{
    mt_u32 all;
    struct
    {
        mt_u32 i2s_bit_size                : 6;
        mt_u32                             : 2;
        mt_u32 pcm_tec_cnt                 : 14;
        mt_u32                             : 2;
        mt_u32 i2s_bit_size_err_cnt        : 8;
    } bitc;
} reg_audin_aria_i2s_size_t;

/*!
  the union of register reg_audin_aria_buf0_waddr
  */
typedef union reg_audin_aria_buf0_waddr
{
    mt_u32 all;
    struct
    {
        mt_u32 buf0_waddr                  : 28;
        mt_u32                             : 4;
    } bitc;
} reg_audin_aria_buf0_waddr_t;

/*!
  the union of register reg_audin_aria_buf1_waddr
  */
typedef union reg_audin_aria_buf1_waddr
{
    mt_u32 all;
    struct
    {
        mt_u32 buf1_waddr                  : 28;
        mt_u32                             : 4;
    } bitc;
} reg_audin_aria_buf1_waddr_t;

/*!
  the union of register reg_audin_aria_res_reg0
  */
typedef union reg_audin_aria_res_reg0
{
    mt_u32 all;
    struct
    {
        mt_u32 reserved_reg0               : 32;
    } bitc;
} reg_audin_aria_res_reg0_t;

/*!
  the union of register reg_audin_aria_res_reg1
  */
typedef union reg_audin_aria_res_reg1
{
    mt_u32 all;
    struct
    {
        mt_u32 reserved_reg1               : 32;
    } bitc;
} reg_audin_aria_res_reg1_t;


#ifdef __cplusplus
extern "C" {
#endif

/*!
  register REGAUDIN_ARIA_CPTR_EN (read/write)
  */
mt_void reg_audin_aria_set_cptr_en(mt_u32 data);
mt_u32  reg_audin_aria_get_cptr_en(mt_void);
mt_void reg_audin_aria_set_cptr_en_audio_cptr_en(mt_u8 data);
mt_u8   reg_audin_aria_get_cptr_en_audio_cptr_en(mt_void);
mt_void reg_audin_aria_set_cptr_en_axi_bus_end_flag(mt_u8 data);
mt_u8   reg_audin_aria_get_cptr_en_axi_bus_end_flag(mt_void);
mt_void reg_audin_aria_set_cptr_en_audin_cptr_end_flag(mt_u8 data);
mt_u8   reg_audin_aria_get_cptr_en_audin_cptr_end_flag(mt_void);

/*!
  register REGAUDIN_ARIA_I2S_FMT (read/write)
  */
mt_void reg_audin_aria_set_i2s_fmt(mt_u32 data);
mt_u32  reg_audin_aria_get_i2s_fmt(mt_void);
mt_void reg_audin_aria_set_i2s_fmt_just(mt_u8 data);
mt_u8   reg_audin_aria_get_i2s_fmt_just(mt_void);
mt_void reg_audin_aria_set_i2s_fmt_ws_pol(mt_u8 data);
mt_u8   reg_audin_aria_get_i2s_fmt_ws_pol(mt_void);
mt_void reg_audin_aria_set_i2s_fmt_shift(mt_u8 data);
mt_u8   reg_audin_aria_get_i2s_fmt_shift(mt_void);
mt_void reg_audin_aria_set_i2s_fmt_little_endian(mt_u8 data);
mt_u8   reg_audin_aria_get_i2s_fmt_little_endian(mt_void);

/*!
  register REGAUDIN_ARIA_BCLK_OE (read/write)
  */
mt_void reg_audin_aria_set_bclk_oe(mt_u32 data);
mt_u32  reg_audin_aria_get_bclk_oe(mt_void);
mt_void reg_audin_aria_set_bclk_oe_bclk_oe(mt_u8 data);
mt_u8   reg_audin_aria_get_bclk_oe_bclk_oe(mt_void);
mt_void reg_audin_aria_set_bclk_oe_ws_oe(mt_u8 data);
mt_u8   reg_audin_aria_get_bclk_oe_ws_oe(mt_void);
mt_void reg_audin_aria_set_bclk_oe_mclk_oe(mt_u8 data);
mt_u8   reg_audin_aria_get_bclk_oe_mclk_oe(mt_void);

/*!
  register REGAUDIN_ARIA_CLK_DIV_CFG (read/write)
  */
mt_void reg_audin_aria_set_clk_div_cfg(mt_u32 data);
mt_u32  reg_audin_aria_get_clk_div_cfg(mt_void);
mt_void reg_audin_aria_set_clk_div_cfg_clk_divider_factor(mt_u32 data);
mt_u32  reg_audin_aria_get_clk_div_cfg_clk_divider_factor(mt_void);

/*!
  register REGAUDIN_ARIA_BUF0_ADDR (read/write)
  */
mt_void reg_audin_aria_set_buf0_addr(mt_u32 data);
mt_u32  reg_audin_aria_get_buf0_addr(mt_void);
mt_void reg_audin_aria_set_buf0_addr_buf0_addr(mt_u32 data);
mt_u32  reg_audin_aria_get_buf0_addr_buf0_addr(mt_void);

/*!
  register REGAUDIN_ARIA_BUF1_ADDR (read/write)
  */
mt_void reg_audin_aria_set_buf1_addr(mt_u32 data);
mt_u32  reg_audin_aria_get_buf1_addr(mt_void);
mt_void reg_audin_aria_set_buf1_addr_buf1_addr(mt_u32 data);
mt_u32  reg_audin_aria_get_buf1_addr_buf1_addr(mt_void);

/*!
  register REGAUDIN_ARIA_BUF_LENGTH (read/write)
  */
mt_void reg_audin_aria_set_buf_length(mt_u32 data);
mt_u32  reg_audin_aria_get_buf_length(mt_void);
mt_void reg_audin_aria_set_buf_length_buf_len(mt_u32 data);
mt_u32  reg_audin_aria_get_buf_length_buf_len(mt_void);

/*!
  register REGAUDIN_ARIA_BUF_MODE (read/write)
  */
mt_void reg_audin_aria_set_buf_mode(mt_u32 data);
mt_u32  reg_audin_aria_get_buf_mode(mt_void);
mt_void reg_audin_aria_set_buf_mode_interlace_mode(mt_u8 data);
mt_u8   reg_audin_aria_get_buf_mode_interlace_mode(mt_void);
mt_void reg_audin_aria_set_buf_mode_addr_incr_mode(mt_u8 data);
mt_u8   reg_audin_aria_get_buf_mode_addr_incr_mode(mt_void);
mt_void reg_audin_aria_set_buf_mode_audio_play_mode(mt_u8 data);
mt_u8   reg_audin_aria_get_buf_mode_audio_play_mode(mt_void);

/*!
  register REGAUDIN_ARIA_CPTR_PAUSE (read/write)
  */
mt_void reg_audin_aria_set_cptr_pause(mt_u32 data);
mt_u32  reg_audin_aria_get_cptr_pause(mt_void);
mt_void reg_audin_aria_set_cptr_pause_audio_in_cptr_pause(mt_u8 data);
mt_u8   reg_audin_aria_get_cptr_pause_audio_in_cptr_pause(mt_void);

/*!
  register REGAUDIN_ARIA_AUDON_INTR_SET (read/write)
  */
mt_void reg_audin_aria_set_audon_intr_set(mt_u32 data);
mt_u32  reg_audin_aria_get_audon_intr_set(mt_void);
mt_void reg_audin_aria_set_audon_intr_set_i_frm_interrupt(mt_u8 data);
mt_u8   reg_audin_aria_get_audon_intr_set_i_frm_interrupt(mt_void);
mt_void reg_audin_aria_set_audon_intr_set_frm_intr_en(mt_u8 data);
mt_u8   reg_audin_aria_get_audon_intr_set_frm_intr_en(mt_void);
mt_void reg_audin_aria_set_audon_intr_set_frm_intr_mask(mt_u8 data);
mt_u8   reg_audin_aria_get_audon_intr_set_frm_intr_mask(mt_void);

/*!
  register REGAUDIN_ARIA_FRM_SIZE (read/write)
  */
mt_void reg_audin_aria_set_frm_size(mt_u32 data);
mt_u32  reg_audin_aria_get_frm_size(mt_void);
mt_void reg_audin_aria_set_frm_size_frm_size(mt_u16 data);
mt_u16  reg_audin_aria_get_frm_size_frm_size(mt_void);

/*!
  register REGAUDIN_ARIA_BUF_THD (read/write)
  */
mt_void reg_audin_aria_set_buf_thd(mt_u32 data);
mt_u32  reg_audin_aria_get_buf_thd(mt_void);
mt_void reg_audin_aria_set_buf_thd_buf_urgent_thd(mt_u8 data);
mt_u8   reg_audin_aria_get_buf_thd_buf_urgent_thd(mt_void);

/*!
  register REGAUDIN_ARIA_AXI_WR_CNT (read/write)
  */
mt_void reg_audin_aria_set_axi_wr_cnt(mt_u32 data);
mt_u32  reg_audin_aria_get_axi_wr_cnt(mt_void);
mt_void reg_audin_aria_set_axi_wr_cnt_mem_wr_cnt_0(mt_u8 data);
mt_u8   reg_audin_aria_get_axi_wr_cnt_mem_wr_cnt_0(mt_void);
mt_void reg_audin_aria_set_axi_wr_cnt_mem_wr_cnt_1(mt_u8 data);
mt_u8   reg_audin_aria_get_axi_wr_cnt_mem_wr_cnt_1(mt_void);
mt_void reg_audin_aria_set_axi_wr_cnt_aw_valid_buf0(mt_u8 data);
mt_u8   reg_audin_aria_get_axi_wr_cnt_aw_valid_buf0(mt_void);
mt_void reg_audin_aria_set_axi_wr_cnt_w_valid_buf0(mt_u8 data);
mt_u8   reg_audin_aria_get_axi_wr_cnt_w_valid_buf0(mt_void);
mt_void reg_audin_aria_set_axi_wr_cnt_aw_valid_buf1(mt_u8 data);
mt_u8   reg_audin_aria_get_axi_wr_cnt_aw_valid_buf1(mt_void);
mt_void reg_audin_aria_set_axi_wr_cnt_w_valid_buf1(mt_u8 data);
mt_u8   reg_audin_aria_get_axi_wr_cnt_w_valid_buf1(mt_void);
mt_void reg_audin_aria_set_axi_wr_cnt_b_valid_flag_0(mt_u8 data);
mt_u8   reg_audin_aria_get_axi_wr_cnt_b_valid_flag_0(mt_void);
mt_void reg_audin_aria_set_axi_wr_cnt_b_valid_flag_1(mt_u8 data);
mt_u8   reg_audin_aria_get_axi_wr_cnt_b_valid_flag_1(mt_void);
mt_void reg_audin_aria_set_axi_wr_cnt_mem_wr_b_cnt_0(mt_u8 data);
mt_u8   reg_audin_aria_get_axi_wr_cnt_mem_wr_b_cnt_0(mt_void);
mt_void reg_audin_aria_set_axi_wr_cnt_mem_wr_b_cnt_1(mt_u8 data);
mt_u8   reg_audin_aria_get_axi_wr_cnt_mem_wr_b_cnt_1(mt_void);

/*!
  register REGAUDIN_ARIA_FIFO_CNT (read)
  */
mt_u32  reg_audin_aria_get_fifo_cnt(mt_void);
mt_u8   reg_audin_aria_get_fifo_cnt_left_fifo_cnt(mt_void);
mt_u8   reg_audin_aria_get_fifo_cnt_right_fifo_cnt(mt_void);

/*!
  register REGAUDIN_ARIA_FRM_CNT (read)
  */
mt_u32  reg_audin_aria_get_frm_cnt(mt_void);
mt_u32  reg_audin_aria_get_frm_cnt_frm_cnt(mt_void);

/*!
  register REGAUDIN_ARIA_I2S_SIZE (read/write)
  */
mt_void reg_audin_aria_set_i2s_size(mt_u32 data);
mt_u32  reg_audin_aria_get_i2s_size(mt_void);
mt_void reg_audin_aria_set_i2s_size_i2s_bit_size(mt_u8 data);
mt_u8   reg_audin_aria_get_i2s_size_i2s_bit_size(mt_void);
mt_void reg_audin_aria_set_i2s_size_pcm_tec_cnt(mt_u16 data);
mt_u16  reg_audin_aria_get_i2s_size_pcm_tec_cnt(mt_void);
mt_void reg_audin_aria_set_i2s_size_i2s_bit_size_err_cnt(mt_u8 data);
mt_u8   reg_audin_aria_get_i2s_size_i2s_bit_size_err_cnt(mt_void);

/*!
  register REGAUDIN_ARIA_BUF0_WADDR (read/write)
  */
mt_void reg_audin_aria_set_buf0_waddr(mt_u32 data);
mt_u32  reg_audin_aria_get_buf0_waddr(mt_void);
mt_void reg_audin_aria_set_buf0_waddr_buf0_waddr(mt_u32 data);
mt_u32  reg_audin_aria_get_buf0_waddr_buf0_waddr(mt_void);

/*!
  register REGAUDIN_ARIA_BUF1_WADDR (read/write)
  */
mt_void reg_audin_aria_set_buf1_waddr(mt_u32 data);
mt_u32  reg_audin_aria_get_buf1_waddr(mt_void);
mt_void reg_audin_aria_set_buf1_waddr_buf1_waddr(mt_u32 data);
mt_u32  reg_audin_aria_get_buf1_waddr_buf1_waddr(mt_void);

/*!
  register REGAUDIN_ARIA_RES_REG0 (read/write)
  */
mt_void reg_audin_aria_set_res_reg0(mt_u32 data);
mt_u32  reg_audin_aria_get_res_reg0(mt_void);
mt_void reg_audin_aria_set_res_reg0_reserved_reg0(mt_u32 data);
mt_u32  reg_audin_aria_get_res_reg0_reserved_reg0(mt_void);

/*!
  register REGAUDIN_ARIA_RES_REG1 (read/write)
  */
mt_void reg_audin_aria_set_res_reg1(mt_u32 data);
mt_u32  reg_audin_aria_get_res_reg1(mt_void);
mt_void reg_audin_aria_set_res_reg1_reserved_reg1(mt_u32 data);
mt_u32  reg_audin_aria_get_res_reg1_reserved_reg1(mt_void);

/*!
  AUDIN_ARIA reg init function
  */
mt_void reg_audin_aria_init(mt_void);

#ifdef __cplusplus
}
#endif

#endif /* _AUD_IN_ARIA_REG_H */

