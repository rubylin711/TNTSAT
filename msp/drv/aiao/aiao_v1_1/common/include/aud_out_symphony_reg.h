/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _AUD_OUT_SYMPHONY_REG_H
#define _AUD_OUT_SYMPHONY_REG_H

/*!
  the enum of AUDOUT_SYMPHONY registers
  */
enum
{
    REG_SYMPHONY_AUD_CLK_ADJ              = 0x14,
    REG_SYMPHONY_AUD_SAMP_FRM              = 0x90,
    REG_SYMPHONY_AUD_INTR_SET              = 0xA0,
    REG_SYMPHONY_AUD_FRM_INTR_CFG       = 0xD8,
    REG_SYMPHONY_AUD_FRMINTR_CNT		= 0x130,
    REG_SYMPHONY_AUD_FRM_CNT			= 0x140,
    REG_SYMPHONY_AUD_FRM_CNT_2		= 0x144,
};

/*!
  the union of register reg_symphony_aud_clk_adj
  */
typedef union reg_symphony_aud_clk_adj
{
	mt_u32 all;
	struct
	{
		mt_u32 samp_rate_offset               : 25;
		mt_u32              : 7;
	} bitc;
} reg_symphony_aud_clk_adj_t;

/*!
  the union of register reg_symphony_aud_samp_frm
  */
typedef union reg_symphony_aud_samp_frm
{
	mt_u32 all;
	struct
	{
		mt_u32 samp_num_perfrm               : 14;
		mt_u32              : 18;
	} bitc;
} reg_symphony_aud_samp_frm_t;

/*!
  the union of register reg_symphony_aud_intr_set
  */
typedef union reg_symphony_aud_intr_set
{
	mt_u32 all;
	struct
	{
		mt_u32 audfrm_intr               : 1;
		mt_u32 ppbuf_w_intr             : 1;
		mt_u32 buf_rw_intr               : 1;
		mt_u32 pcmfifo_emp_intr       : 1;
		mt_u32 pcmfifo_diff_intr         : 1;
		mt_u32 audfrm_intr_2             : 1;
		mt_u32             				: 2;
		mt_u32 audfrm_intr_mask         : 1;
		mt_u32 ppbufw_intr_mask         : 1;
		mt_u32 bufrw_intr_mask               : 1;
		mt_u32 pcmfifo_emp_intr_mask       : 1;
		mt_u32 pcmfifo_diff_intr_mask         : 1;
		mt_u32 audfrm_intr_mask_2             : 1;
		mt_u32             				: 2;
		mt_u32 audfrm_en         : 1;
		mt_u32 ppbuf_w_en         : 1;
		mt_u32 buf_rw_en               : 1;
		mt_u32 pcmfifo_emp_en       : 1;
		mt_u32 pcmfifo_diff_en         : 1;
		mt_u32 audfrm_intr2_en             : 1;
		mt_u32             				: 10;
	} bitc;
} reg_symphony_aud_intr_set_t;

/*!
  the union of register reg_symphony_aud_frm_intr_cfg
  */
typedef union reg_symphony_aud_frm_intr_cfg
{
    mt_u32 all;
    struct
    {
        mt_u32 audfrm_intr_cfg                        : 1;
        mt_u32                             : 2;
        mt_u32 audfrm_intr_debug                        : 1;
        mt_u32 reserved2                        : 1;
        mt_u32                             : 3;
        mt_u32 audfrm_intr_2_cfg                        : 1;
        mt_u32                             : 3;
        mt_u32 audfrm_intr_cfg_play                        : 1;
        mt_u32                             : 3;
        mt_u32 frm_cnt_clr                        : 1;
        mt_u32                             : 2;
        mt_u32 frm_cnt_2_clr                        : 1;
        mt_u32 reserved                            : 12;
    } bitc;
} reg_symphony_aud_frm_intr_cfg_t;

/*!
  the union of register reg_symphony_aud_frmintr_cnt
  */
typedef union reg_symphony_aud_frmintr_cnt
{
    mt_u32 all;
    struct
    {
        mt_u32 audfrm_intr_cnt                        : 14;
        mt_u32                             : 2;
        mt_u32 audfrm_intr_2_cnt                        : 14;
        mt_u32                             : 2;
    } bitc;
} reg_symphony_aud_frmintr_cnt_t;

/*!
  the union of register reg_symphony_aud_frm_cnt
  */
typedef union reg_symphony_aud_frm_cnt
{
    mt_u32 all;
    struct
    {
        mt_u32 frame_cnt                        : 21;
        mt_u32                             : 11;
    } bitc;
} reg_symphony_aud_frm_cnt_t;

/*!
  the union of register reg_symphony_aud_frm_cnt
  */
typedef union reg_symphony_aud_frm_cnt_2
{
    mt_u32 all;
    struct
    {
        mt_u32 frame_cnt_2                        : 21;
        mt_u32                             : 11;
    } bitc;
} reg_symphony_aud_frm_cnt_2_t;

#ifdef __cplusplus
extern "C" {
#endif

/*!
  register REG_SYMPHONY_AUD_CLK_ADJ (read/write)
  */
mt_u32   reg_symphony_aud_get_aud_clk_adj(mt_void);

/*!
  register REG_SYMPHONY_AUD_SAMP_FRM (read/write)
  */
mt_u32   reg_symphony_aud_get_aud_samp_frm(mt_void);

/*!
  register REG_SYMPHONY_AUD_INTR_SET (read/write)
  */
mt_u32   reg_symphony_aud_get_aud_intr_set(mt_void);
mt_void reg_symphony_aud_clr_audfrm_intr(mt_u32 data);
mt_u8   reg_symphony_aud_get_audfrm_intr(mt_void);
mt_void reg_symphony_aud_clr_audfrm_intr_2(mt_u32 data);
mt_u8   reg_symphony_aud_get_audfrm_intr_2(mt_void);
mt_void reg_symphony_aud_set_intr_mask(mt_u32 data);
mt_void reg_symphony_aud_set_audfrm_intr_mask(mt_u32 data);
mt_u8   reg_symphony_aud_get_audfrm_intr_mask(mt_void);
mt_void reg_symphony_aud_set_audfrm_intr_2_mask(mt_u32 data);
mt_u8   reg_symphony_aud_get_audfrm_intr_2_mask(mt_void);
mt_void reg_symphony_aud_set_intr_en(mt_u32 data);
mt_void reg_symphony_aud_set_audfrm_intr_en(mt_u32 data);
mt_u8   reg_symphony_aud_get_audfrm_intr_en(mt_void);
mt_void reg_symphony_aud_set_audfrm_intr_2_en(mt_u32 data);
mt_u8   reg_symphony_aud_get_audfrm_intr_2_en(mt_void);

/*!
  register REG_SYMPHONY_AUD_FRM_INTR_CFG (read/write)
  */
mt_u32   reg_symphony_aud_get_audfrm_intr_cfg_all(mt_void);
mt_void reg_symphony_aud_set_audfrm_intr_cfg(mt_u32 data);
mt_u8   reg_symphony_aud_get_audfrm_intr_cfg(mt_void);
mt_void reg_symphony_aud_set_audfrm_intr_2_cfg(mt_u32 data);
mt_u8   reg_symphony_aud_get_audfrm_intr_2_cfg(mt_void);
mt_void reg_symphony_aud_frm_cnt_clr(mt_u32 data);
mt_void reg_symphony_aud_frm_cnt_2_clr(mt_u32 data);

/*!
  register REG_SYMPHONY_AUD_FRMINTR_CNT (read)
  */
mt_u32   reg_symphony_aud_get_audfrm_intr_cnt(mt_void);
mt_u32   reg_symphony_aud_get_audfrm_intr_2_cnt(mt_void);

/*!
  register REG_SYMPHONY_AUD_FRM_CNT (read)
  */
mt_u32   reg_symphony_aud_get_aud_frm_cnt(mt_void);

/*!
  register REG_SYMPHONY_AUD_FRM_CNT_2 (read)
  */
mt_u32   reg_symphony_aud_get_aud_frm_cnt_2(mt_void);

/*!
  AUDIN_SYMPHONY reg init function
  */
mt_void reg_symphony_aud_init(mt_void);

#ifdef __cplusplus
}
#endif

#endif /* _AUD_OUT_SYMPHONY_REG_H */

