
#ifndef __TDE_HAL_REG_ARIA_DEF_H__
#define __TDE_HAL_REG_ARIA_DEF_H__


/*!
  the union of register reg_ip_graphic_gra_eng_start
  */
typedef union reg_ip_graphic_gra_eng_start
{
    u32 all;
    struct
    {
        u32 gra_start                   : 1;
        u32                             : 3;
        u32 sync_start                  : 1;
        u32 sync_restart                : 1;
        u32 async_start                 : 1;
        u32                             : 1;
        u32 reg_clr_start               : 1;
        u32                             : 23;
    } bitc;
} reg_ip_graphic_gra_eng_start_t;

/*!
  the union of register reg_ip_graphic_gra_eng_en
  */
typedef union reg_ip_graphic_gra_eng_en
{
    u32 all;
    struct
    {
        u32 comp_en                     : 1;
        u32                             : 3;
        u32 scaler_en                   : 1;
        u32                             : 3;
        u32 rot_en                      : 1;
        u32                             : 3;
        u32 pat_en                      : 1;
        u32                             : 3;
        u32 xylc_en                     : 1;
        u32                             : 3;
        u32 gradt_en                    : 1;
        u32                             : 3;
        u32 clip_en                     : 1;
        u32                             : 7;
    } bitc;
} reg_ip_graphic_gra_eng_en_t;

/*!
  the union of register reg_ip_graphic_gra_eng_cfg
  */
typedef union reg_ip_graphic_gra_eng_cfg
{
    u32 all;
    struct
    {
        u32 reg_clr_mod                 : 2;
        u32                             : 2;
        u32 state_clr_mod               : 5;
        u32                             : 23;
    } bitc;
} reg_ip_graphic_gra_eng_cfg_t;

/*!
  the union of register reg_ip_graphic_gra_core_done
  */
typedef union reg_ip_graphic_gra_core_done
{
    u32 all;
    struct
    {
        u32 dst_out_done                : 1;
        u32                             : 3;
        u32 dst0_out_done               : 1;
        u32                             : 3;
        u32 dst1_out_done               : 1;
        u32                             : 3;
        u32 comp_out_done               : 1;
        u32                             : 19;
    } bitc;
} reg_ip_graphic_gra_core_done_t;

/*!
  the union of register reg_ip_graphic_src0_fmt_cfg0
  */
typedef union reg_ip_graphic_src0_fmt_cfg0
{
    u32 all;
    struct
    {
        u32 src0_en                     : 1;
        u32                             : 15;
        u32 src0_bpp_mod                : 3;
        u32                             : 5;
        u32 src0_swap_mod               : 8;
    } bitc;
} reg_ip_graphic_src0_fmt_cfg0_t;

/*!
  the union of register reg_ip_graphic_src0_fmt_cfg1
  */
typedef union reg_ip_graphic_src0_fmt_cfg1
{
    u32 all;
    struct
    {
        u32 src0_pic_fmt                : 7;
        u32                             : 25;
    } bitc;
} reg_ip_graphic_src0_fmt_cfg1_t;

/*!
  the union of register reg_ip_graphic_src0_pic_addr
  */
typedef union reg_ip_graphic_src0_pic_addr
{
    u32 all;
    struct
    {
        u32 src0_pic_addr               : 31;
        u32                             : 1;
    } bitc;
} reg_ip_graphic_src0_pic_addr_t;

/*!
  the union of register reg_ip_graphic_src0_pic_stride
  */
typedef union reg_ip_graphic_src0_pic_stride
{
    u32 all;
    struct
    {
        u32 src0_pic_stride             : 17;
        u32                             : 15;
    } bitc;
} reg_ip_graphic_src0_pic_stride_t;

/*!
  the union of register reg_ip_graphic_src1_fmt_cfg0
  */
typedef union reg_ip_graphic_src1_fmt_cfg0
{
    u32 all;
    struct
    {
        u32 src1_en                     : 1;
        u32                             : 3;
        u32 src1_clut_en                : 1;
        u32                             : 3;
        u32 src1_alp_en                 : 1;
        u32                             : 7;
        u32 src1_bpp_mod                : 3;
        u32                             : 1;
        u32 src1_csc_mod                : 2;
        u32                             : 2;
        u32 src1_swap_mod               : 8;
    } bitc;
} reg_ip_graphic_src1_fmt_cfg0_t;

/*!
  the union of register reg_ip_graphic_src1_fmt_cfg1
  */
typedef union reg_ip_graphic_src1_fmt_cfg1
{
    u32 all;
    struct
    {
        u32 src1_pic_fmt                : 7;
        u32                             : 1;
        u32 src1_key_inv                : 1;
        u32                             : 3;
        u32 src1_exp_mod                : 4;
        u32 src1_key_mod                : 16;
    } bitc;
} reg_ip_graphic_src1_fmt_cfg1_t;

/*!
  the union of register reg_ip_graphic_src1_key_min
  */
typedef union reg_ip_graphic_src1_key_min
{
    u32 all;
    struct
    {
        u32 src1_key_min                : 32;
    } bitc;
} reg_ip_graphic_src1_key_min_t;

/*!
  the union of register reg_ip_graphic_src1_key_max
  */
typedef union reg_ip_graphic_src1_key_max
{
    u32 all;
    struct
    {
        u32 src1_key_max                : 32;
    } bitc;
} reg_ip_graphic_src1_key_max_t;

/*!
  the union of register reg_ip_graphic_src1_pic_addr
  */
typedef union reg_ip_graphic_src1_pic_addr
{
    u32 all;
    struct
    {
        u32 src1_pic_addr               : 31;
        u32                             : 1;
    } bitc;
} reg_ip_graphic_src1_pic_addr_t;

/*!
  the union of register reg_ip_graphic_src1_pic_stride
  */
typedef union reg_ip_graphic_src1_pic_stride
{
    u32 all;
    struct
    {
        u32 src1_pic_stride             : 17;
        u32                             : 15;
    } bitc;
} reg_ip_graphic_src1_pic_stride_t;

/*!
  the union of register reg_ip_graphic_src1_pic_size
  */
typedef union reg_ip_graphic_src1_pic_size
{
    u32 all;
    struct
    {
        u32 src1_pic_w                  : 13;
        u32                             : 3;
        u32 src1_pic_h                  : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_src1_pic_size_t;

/*!
  the union of register reg_ip_graphic_src1_op_size
  */
typedef union reg_ip_graphic_src1_op_size
{
    u32 all;
    struct
    {
        u32 src1_op_w                   : 13;
        u32                             : 3;
        u32 src1_op_h                   : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_src1_op_size_t;

/*!
  the union of register reg_ip_graphic_src1_op_pos
  */
typedef union reg_ip_graphic_src1_op_pos
{
    u32 all;
    struct
    {
        u32 src1_op_x                   : 13;
        u32                             : 3;
        u32 src1_op_y                   : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_src1_op_pos_t;

/*!
  the union of register reg_ip_graphic_src1_cmyk_cfg
  */
typedef union reg_ip_graphic_src1_cmyk_cfg
{
    u32 all;
    struct
    {
        u32 cmyk_en                     : 1;
        u32                             : 7;
        u32 max                         : 8;
        u32 coef                        : 11;
        u32                             : 5;
    } bitc;
} reg_ip_graphic_src1_cmyk_cfg_t;

/*!
  the union of register reg_ip_graphic_src1_status
  */
typedef union reg_ip_graphic_src1_status
{
    u32 all;
    struct
    {
        u32 src1_status                 : 32;
    } bitc;
} reg_ip_graphic_src1_status_t;

/*!
  the union of register reg_ip_graphic_src2_fmt_cfg0
  */
typedef union reg_ip_graphic_src2_fmt_cfg0
{
    u32 all;
    struct
    {
        u32 src2_en                     : 1;
        u32                             : 7;
        u32 src2_alp_en                 : 1;
        u32                             : 7;
        u32 src2_bpp_mod                : 3;
        u32                             : 1;
        u32 src2_csc_mod                : 2;
        u32                             : 2;
        u32 src2_swap_mod               : 8;
    } bitc;
} reg_ip_graphic_src2_fmt_cfg0_t;

/*!
  the union of register reg_ip_graphic_src2_fmt_cfg1
  */
typedef union reg_ip_graphic_src2_fmt_cfg1
{
    u32 all;
    struct
    {
        u32 src2_pic_fmt                : 7;
        u32                             : 1;
        u32 src2_key_inv                : 1;
        u32                             : 3;
        u32 src2_exp_mod                : 4;
        u32 src2_key_mod                : 16;
    } bitc;
} reg_ip_graphic_src2_fmt_cfg1_t;

/*!
  the union of register reg_ip_graphic_src2_key_min
  */
typedef union reg_ip_graphic_src2_key_min
{
    u32 all;
    struct
    {
        u32 src2_key_min                : 32;
    } bitc;
} reg_ip_graphic_src2_key_min_t;

/*!
  the union of register reg_ip_graphic_src2_key_max
  */
typedef union reg_ip_graphic_src2_key_max
{
    u32 all;
    struct
    {
        u32 src2_key_max                : 32;
    } bitc;
} reg_ip_graphic_src2_key_max_t;

/*!
  the union of register reg_ip_graphic_src2_pic_addr
  */
typedef union reg_ip_graphic_src2_pic_addr
{
    u32 all;
    struct
    {
        u32 src2_pic_addr               : 31;
        u32                             : 1;
    } bitc;
} reg_ip_graphic_src2_pic_addr_t;

/*!
  the union of register reg_ip_graphic_src2_pic_stride
  */
typedef union reg_ip_graphic_src2_pic_stride
{
    u32 all;
    struct
    {
        u32 src2_pic_stride             : 17;
        u32                             : 15;
    } bitc;
} reg_ip_graphic_src2_pic_stride_t;

/*!
  the union of register reg_ip_graphic_src2_op_pos
  */
typedef union reg_ip_graphic_src2_op_pos
{
    u32 all;
    struct
    {
        u32 src2_op_x                   : 13;
        u32                             : 3;
        u32 src2_op_y                   : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_src2_op_pos_t;

/*!
  the union of register reg_ip_graphic_src2_status
  */
typedef union reg_ip_graphic_src2_status
{
    u32 all;
    struct
    {
        u32 src2_status                 : 32;
    } bitc;
} reg_ip_graphic_src2_status_t;

/*!
  the union of register reg_ip_graphic_src3_fmt_cfg0
  */
typedef union reg_ip_graphic_src3_fmt_cfg0
{
    u32 all;
    struct
    {
        u32 src3_en                     : 1;
        u32                             : 3;
        u32 src3_clut_en                : 1;
        u32                             : 3;
        u32 src3_alp_en                 : 1;
        u32                             : 7;
        u32 src3_bpp_mod                : 3;
        u32                             : 1;
        u32 src3_csc_mod                : 2;
        u32                             : 2;
        u32 src3_swap_mod               : 8;
    } bitc;
} reg_ip_graphic_src3_fmt_cfg0_t;

/*!
  the union of register reg_ip_graphic_src3_fmt_cfg1
  */
typedef union reg_ip_graphic_src3_fmt_cfg1
{
    u32 all;
    struct
    {
        u32 src3_pic_fmt                : 7;
        u32                             : 1;
        u32 src3_key_inv                : 1;
        u32                             : 3;
        u32 src3_exp_mod                : 4;
        u32 src3_key_mod                : 16;
    } bitc;
} reg_ip_graphic_src3_fmt_cfg1_t;

/*!
  the union of register reg_ip_graphic_src3_key_min
  */
typedef union reg_ip_graphic_src3_key_min
{
    u32 all;
    struct
    {
        u32 src3_key_min                : 32;
    } bitc;
} reg_ip_graphic_src3_key_min_t;

/*!
  the union of register reg_ip_graphic_src3_key_max
  */
typedef union reg_ip_graphic_src3_key_max
{
    u32 all;
    struct
    {
        u32 src3_key_max                : 32;
    } bitc;
} reg_ip_graphic_src3_key_max_t;

/*!
  the union of register reg_ip_graphic_src3_pic_addr
  */
typedef union reg_ip_graphic_src3_pic_addr
{
    u32 all;
    struct
    {
        u32 src3_pic_addr               : 31;
        u32                             : 1;
    } bitc;
} reg_ip_graphic_src3_pic_addr_t;

/*!
  the union of register reg_ip_graphic_src3_pic_stride
  */
typedef union reg_ip_graphic_src3_pic_stride
{
    u32 all;
    struct
    {
        u32 src3_pic_stride             : 17;
        u32                             : 15;
    } bitc;
} reg_ip_graphic_src3_pic_stride_t;

/*!
  the union of register reg_ip_graphic_src3_op_pos
  */
typedef union reg_ip_graphic_src3_op_pos
{
    u32 all;
    struct
    {
        u32 src3_op_x                   : 13;
        u32                             : 3;
        u32 src3_op_y                   : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_src3_op_pos_t;

/*!
  the union of register reg_ip_graphic_src3_status
  */
typedef union reg_ip_graphic_src3_status
{
    u32 all;
    struct
    {
        u32 src3_status                 : 32;
    } bitc;
} reg_ip_graphic_src3_status_t;

/*!
  the union of register reg_ip_graphic_dst0_fmt_cfg0
  */
typedef union reg_ip_graphic_dst0_fmt_cfg0
{
    u32 all;
    struct
    {
        u32 dst0_en                     : 1;
        u32                             : 3;
        u32 dst0_adj_mtx_en             : 1;
        u32                             : 3;
        u32 dst0_ds_mod                 : 3;
        u32                             : 1;
        u32 dst0_msk_mod                : 2;
        u32                             : 2;
        u32 dst0_bpp_mod                : 3;
        u32                             : 1;
        u32 dst0_csc_mod                : 2;
        u32                             : 2;
        u32 dst0_swap_mod               : 8;
    } bitc;
} reg_ip_graphic_dst0_fmt_cfg0_t;

/*!
  the union of register reg_ip_graphic_dst0_fmt_cfg1
  */
typedef union reg_ip_graphic_dst0_fmt_cfg1
{
    u32 all;
    struct
    {
        u32 dst0_pic_fmt                : 7;
        u32                             : 1;
        u32 dst0_premult_en             : 1;
        u32                             : 23;
    } bitc;
} reg_ip_graphic_dst0_fmt_cfg1_t;

/*!
  the union of register reg_ip_graphic_dst0_pic_addr
  */
typedef union reg_ip_graphic_dst0_pic_addr
{
    u32 all;
    struct
    {
        u32 dst0_pic_addr               : 31;
        u32                             : 1;
    } bitc;
} reg_ip_graphic_dst0_pic_addr_t;

/*!
  the union of register reg_ip_graphic_dst0_pic_stride
  */
typedef union reg_ip_graphic_dst0_pic_stride
{
    u32 all;
    struct
    {
        u32 dst0_pic_stride             : 17;
        u32                             : 15;
    } bitc;
} reg_ip_graphic_dst0_pic_stride_t;

/*!
  the union of register reg_ip_graphic_dst0_op_pos
  */
typedef union reg_ip_graphic_dst0_op_pos
{
    u32 all;
    struct
    {
        u32 dst0_op_x                   : 13;
        u32                             : 3;
        u32 dst0_op_y                   : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_dst0_op_pos_t;

/*!
  the union of register reg_ip_graphic_dst1_fmt_cfg0
  */
typedef union reg_ip_graphic_dst1_fmt_cfg0
{
    u32 all;
    struct
    {
        u32 dst1_en                     : 1;
        u32                             : 11;
        u32 dst1_msk_mod                : 2;
        u32                             : 10;
        u32 dst1_swap_mod               : 8;
    } bitc;
} reg_ip_graphic_dst1_fmt_cfg0_t;

/*!
  the union of register reg_ip_graphic_dst1_fmt_cfg1
  */
typedef union reg_ip_graphic_dst1_fmt_cfg1
{
    u32 all;
    struct
    {
        u32 dst1_pic_fmt                : 7;
        u32                             : 25;
    } bitc;
} reg_ip_graphic_dst1_fmt_cfg1_t;

/*!
  the union of register reg_ip_graphic_dst1_pic_addr
  */
typedef union reg_ip_graphic_dst1_pic_addr
{
    u32 all;
    struct
    {
        u32 dst1_pic_addr               : 31;
        u32                             : 1;
    } bitc;
} reg_ip_graphic_dst1_pic_addr_t;

/*!
  the union of register reg_ip_graphic_dst1_pic_stride
  */
typedef union reg_ip_graphic_dst1_pic_stride
{
    u32 all;
    struct
    {
        u32 dst1_pic_stride             : 17;
        u32                             : 15;
    } bitc;
} reg_ip_graphic_dst1_pic_stride_t;

/*!
  the union of register reg_ip_graphic_dst1_op_pos
  */
typedef union reg_ip_graphic_dst1_op_pos
{
    u32 all;
    struct
    {
        u32 dst1_op_x                   : 13;
        u32                             : 3;
        u32 dst1_op_y                   : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_dst1_op_pos_t;

/*!
  the union of register reg_ip_graphic_dst_pic_size
  */
typedef union reg_ip_graphic_dst_pic_size
{
    u32 all;
    struct
    {
        u32 dst_pic_w                   : 13;
        u32                             : 3;
        u32 dst_pic_h                   : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_dst_pic_size_t;

/*!
  the union of register reg_ip_graphic_dst_op_size
  */
typedef union reg_ip_graphic_dst_op_size
{
    u32 all;
    struct
    {
        u32 dst_op_w                    : 13;
        u32                             : 3;
        u32 dst_op_h                    : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_dst_op_size_t;

/*!
  the union of register reg_ip_graphic_dst_status
  */
typedef union reg_ip_graphic_dst_status
{
    u32 all;
    struct
    {
        u32 dst_status                  : 32;
    } bitc;
} reg_ip_graphic_dst_status_t;

/*!
  the union of register reg_ip_graphic_cmd_fifo_ctrl
  */
typedef union reg_ip_graphic_cmd_fifo_ctrl
{
    u32 all;
    struct
    {
        u32 sync_susp0                  : 1;
        u32 sync_susp1                  : 1;
        u32                             : 2;
        u32 sync_reg_exit               : 1;
        u32 sync_halt_mod               : 1;
        u32                             : 2;
        u32 async_susp0                 : 1;
        u32 async_susp1                 : 1;
        u32                             : 2;
        u32 async_reg_exit              : 1;
        u32 async_conf_mod              : 1;
        u32 conf_recovery_mod           : 1;
        u32                             : 16;
        u32 cmd_fifo_hang               : 1;
    } bitc;
} reg_ip_graphic_cmd_fifo_ctrl_t;

/*!
  the union of register reg_ip_graphic_cmd_fifo_sync
  */
typedef union reg_ip_graphic_cmd_fifo_sync
{
    u32 all;
    struct
    {
        u32 sync_trig_dly               : 23;
        u32                             : 1;
        u32 sync_trig_num               : 4;
        u32 sync_trig_mod               : 3;
        u32                             : 1;
    } bitc;
} reg_ip_graphic_cmd_fifo_sync_t;

/*!
  the union of register reg_ip_graphic_cmd_fifo_addr_sync
  */
typedef union reg_ip_graphic_cmd_fifo_addr_sync
{
    u32 all;
    struct
    {
        u32 cmd_fifo_addr_sync          : 29;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_cmd_fifo_addr_sync_t;

/*!
  the union of register reg_ip_graphic_cmd_fifo_addr_async
  */
typedef union reg_ip_graphic_cmd_fifo_addr_async
{
    u32 all;
    struct
    {
        u32 cmd_fifo_addr_async         : 29;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_cmd_fifo_addr_async_t;

/*!
  the union of register reg_ip_graphic_cmd_fifo_id0
  */
typedef union reg_ip_graphic_cmd_fifo_id0
{
    u32 all;
    struct
    {
        u32 cmd_fifo_id0                : 32;
    } bitc;
} reg_ip_graphic_cmd_fifo_id0_t;

/*!
  the union of register reg_ip_graphic_cmd_fifo_id1
  */
typedef union reg_ip_graphic_cmd_fifo_id1
{
    u32 all;
    struct
    {
        u32 cmd_fifo_id1                : 32;
    } bitc;
} reg_ip_graphic_cmd_fifo_id1_t;

/*!
  the union of register reg_ip_graphic_cmd_fifo_status
  */
typedef union reg_ip_graphic_cmd_fifo_status
{
    u32 all;
    struct
    {
        u32 cmd_fifo_status             : 32;
    } bitc;
} reg_ip_graphic_cmd_fifo_status_t;

/*!
  the union of register reg_ip_graphic_gra_axi_ctrl
  */
typedef union reg_ip_graphic_gra_axi_ctrl
{
    u32 all;
    struct
    {
        u32 gra_stop_ctrl               : 2;
        u32                             : 2;
        u32 gra_pause_ctrl              : 2;
        u32                             : 26;
    } bitc;
} reg_ip_graphic_gra_axi_ctrl_t;

/*!
  the union of register reg_ip_graphic_gra_axi_status
  */
typedef union reg_ip_graphic_gra_axi_status
{
    u32 all;
    struct
    {
        u32 axi_bus_idle                : 1;
        u32                             : 3;
        u32 axi_bus_empty               : 1;
        u32                             : 3;
        u32 rch_buf_empty               : 1;
        u32                             : 3;
        u32 wch0_buf_empty              : 1;
        u32                             : 3;
        u32 wch1_buf_empty              : 1;
        u32                             : 3;
        u32 cmd_buf_empty               : 1;
        u32                             : 11;
    } bitc;
} reg_ip_graphic_gra_axi_status_t;

/*!
  the union of register reg_ip_graphic_xylc_cfg
  */
typedef union reg_ip_graphic_xylc_cfg
{
    u32 all;
    struct
    {
        u32 xylc_cmd_num                : 16;
        u32                             : 16;
    } bitc;
} reg_ip_graphic_xylc_cfg_t;

/*!
  the union of register reg_ip_graphic_xylc_err
  */
typedef union reg_ip_graphic_xylc_err
{
    u32 all;
    struct
    {
        u32 xylc_err_pos                : 16;
        u32 xylc_err_code               : 4;
        u32                             : 12;
    } bitc;
} reg_ip_graphic_xylc_err_t;

/*!
  the union of register reg_ip_graphic_xylc_status
  */
typedef union reg_ip_graphic_xylc_status
{
    u32 all;
    struct
    {
        u32 xylc_cmd_pos                : 16;
        u32 xylc_cmd_end                : 1;
        u32                             : 15;
    } bitc;
} reg_ip_graphic_xylc_status_t;

/*!
  the union of register reg_ip_graphic_scaler_cfg
  */
typedef union reg_ip_graphic_scaler_cfg
{
    u32 all;
    struct
    {
        u32 scaler_mod                  : 2;
        u32                             : 2;
        u32 scaler_flt_mod              : 2;
        u32                             : 2;
        u32 scaler_edge_option          : 2;
        u32                             : 2;
        u32 scaler_af_mod               : 2;
        u32                             : 2;
        u32 blur_en                     : 1;
        u32                             : 3;
        u32 blur_num                    : 6;
        u32                             : 6;
    } bitc;
} reg_ip_graphic_scaler_cfg_t;

/*!
  the union of register reg_ip_graphic_scaler_coef_11
  */
typedef union reg_ip_graphic_scaler_coef_11
{
    u32 all;
    struct
    {
        u32 scaler_coef_11              : 32;
    } bitc;
} reg_ip_graphic_scaler_coef_11_t;

/*!
  the union of register reg_ip_graphic_scaler_coef_21
  */
typedef union reg_ip_graphic_scaler_coef_21
{
    u32 all;
    struct
    {
        u32 scaler_coef_21               : 32;
    } bitc;
} reg_ip_graphic_scaler_coef_21_t;

/*!
  the union of register reg_ip_graphic_scaler_coef_31
  */
typedef union reg_ip_graphic_scaler_coef_31
{
    u32 all;
    struct
    {
        u32 scaler_coef_31              : 32;
    } bitc;
} reg_ip_graphic_scaler_coef_31_t;

/*!
  the union of register reg_ip_graphic_scaler_coef_22
  */
typedef union reg_ip_graphic_scaler_coef_22
{
    u32 all;
    struct
    {
        u32 scaler_coef_22              : 32;
    } bitc;
} reg_ip_graphic_scaler_coef_22_t;

/*!
  the union of register reg_ip_graphic_scaler_coef_23
  */
typedef union reg_ip_graphic_scaler_coef_23
{
    u32 all;
    struct
    {
        u32 scaler_coef_23              : 32;
    } bitc;
} reg_ip_graphic_scaler_coef_23_t;

/*!
  the union of register reg_ip_graphic_scaler_init_phase
  */
typedef union reg_ip_graphic_scaler_init_phase
{
    u32 all;
    struct
    {
        u32 scaler_init_phase           : 16;
        u32 scaler_msk_mod              : 4;
        u32                             : 12;
    } bitc;
} reg_ip_graphic_scaler_init_phase_t;

/*!
  the union of register reg_ip_graphic_scaler_msk_color
  */
typedef union reg_ip_graphic_scaler_msk_color
{
    u32 all;
    struct
    {
        u32 scaler_msk_color            : 32;
    } bitc;
} reg_ip_graphic_scaler_msk_color_t;

/*!
  the union of register reg_ip_graphic_scaler_status
  */
typedef union reg_ip_graphic_scaler_status
{
    u32 all;
    struct
    {
        u32 scaler_status               : 32;
    } bitc;
} reg_ip_graphic_scaler_status_t;

/*!
  the union of register reg_ip_graphic_rot_pat_cfg
  */
typedef union reg_ip_graphic_rot_pat_cfg
{
    u32 all;
    struct
    {
        u32 rot_type                    : 3;
        u32                             : 1;
        u32 pat_mod                     : 2;
        u32                             : 26;
    } bitc;
} reg_ip_graphic_rot_pat_cfg_t;

/*!
  the union of register reg_ip_graphic_pat_color
  */
typedef union reg_ip_graphic_pat_color
{
    u32 all;
    struct
    {
        u32 pat_color                   : 32;
    } bitc;
} reg_ip_graphic_pat_color_t;

/*!
  the union of register reg_ip_graphic_pat_offset_pos
  */
typedef union reg_ip_graphic_pat_offset_pos
{
    u32 all;
    struct
    {
        u32 pat_offset_x                : 13;
        u32                             : 3;
        u32 pat_offset_y                : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_pat_offset_pos_t;

/*!
  the union of register reg_ip_graphic_pat_ratio_x_0
  */
typedef union reg_ip_graphic_pat_ratio_x_0
{
    u32 all;
    struct
    {
        u32 pat_ratio_x_0_r             : 13;
        u32                             : 3;
        u32 pat_ratio_x_0_q             : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_pat_ratio_x_0_t;

/*!
  the union of register reg_ip_graphic_pat_ratio_x_1
  */
typedef union reg_ip_graphic_pat_ratio_x_1
{
    u32 all;
    struct
    {
        u32 pat_ratio_x_1_r             : 13;
        u32                             : 3;
        u32 pat_ratio_x_1_q             : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_pat_ratio_x_1_t;

/*!
  the union of register reg_ip_graphic_pat_ratio_y_0
  */
typedef union reg_ip_graphic_pat_ratio_y_0
{
    u32 all;
    struct
    {
        u32 pat_ratio_y_0_r             : 13;
        u32                             : 3;
        u32 pat_ratio_y_0_q             : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_pat_ratio_y_0_t;

/*!
  the union of register reg_ip_graphic_pat_ratio_y_1
  */
typedef union reg_ip_graphic_pat_ratio_y_1
{
    u32 all;
    struct
    {
        u32 pat_ratio_y_1_r             : 13;
        u32                             : 3;
        u32 pat_ratio_y_1_q             : 13;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_pat_ratio_y_1_t;

/*!
  the union of register reg_ip_graphic_gradt_cfg
  */
typedef union reg_ip_graphic_gradt_cfg
{
    u32 all;
    struct
    {
        u32 gradt_mod                   : 4;
        u32 gradt_msk_mod               : 2;
        u32                             : 26;
    } bitc;
} reg_ip_graphic_gradt_cfg_t;

/*!
  the union of register reg_ip_graphic_gradt_x_step
  */
typedef union reg_ip_graphic_gradt_x_step
{
    u32 all;
    struct
    {
        u32 gradt_x_step                : 23;
        u32                             : 9;
    } bitc;
} reg_ip_graphic_gradt_x_step_t;

/*!
  the union of register reg_ip_graphic_gradt_y_step
  */
typedef union reg_ip_graphic_gradt_y_step
{
    u32 all;
    struct
    {
        u32 gradt_y_step                : 23;
        u32                             : 9;
    } bitc;
} reg_ip_graphic_gradt_y_step_t;

/*!
  the union of register reg_ip_graphic_gradt_start_v
  */
typedef union reg_ip_graphic_gradt_start_v
{
    u32 all;
    struct
    {
        u32 gradt_start_v               : 32;
    } bitc;
} reg_ip_graphic_gradt_start_v_t;

/*!
  the union of register reg_ip_graphic_stop0_argb
  */
typedef union reg_ip_graphic_stop0_argb
{
    u32 all;
    struct
    {
        u32 stop0_argb                  : 32;
    } bitc;
} reg_ip_graphic_stop0_argb_t;

/*!
  the union of register reg_ip_graphic_stop1_argb
  */
typedef union reg_ip_graphic_stop1_argb
{
    u32 all;
    struct
    {
        u32 stop1_argb                  : 32;
    } bitc;
} reg_ip_graphic_stop1_argb_t;

/*!
  the union of register reg_ip_graphic_stop2_argb
  */
typedef union reg_ip_graphic_stop2_argb
{
    u32 all;
    struct
    {
        u32 stop2_argb                  : 32;
    } bitc;
} reg_ip_graphic_stop2_argb_t;

/*!
  the union of register reg_ip_graphic_stop3_argb
  */
typedef union reg_ip_graphic_stop3_argb
{
    u32 all;
    struct
    {
        u32 stop3_argb                  : 32;
    } bitc;
} reg_ip_graphic_stop3_argb_t;

/*!
  the union of register reg_ip_graphic_stop_oft
  */
typedef union reg_ip_graphic_stop_offset
{
    u32 all;
    struct
    {
        u32 stop1_offset                : 12;
        u32                             : 4;
        u32 stop2_offset                : 12;
        u32                             : 4;
    } bitc;
} reg_ip_graphic_stop_offset_t;

/*!
  the union of register reg_ip_graphic_stop0_fact
  */
typedef union reg_ip_graphic_stop0_fact
{
    u32 all;
    struct
    {
        u32 stop0_fact                  : 24;
        u32                             : 8;
    } bitc;
} reg_ip_graphic_stop0_fact_t;

/*!
  the union of register reg_ip_graphic_stop1_fact
  */
typedef union reg_ip_graphic_stop1_fact
{
    u32 all;
    struct
    {
        u32 stop1_fact                  : 24;
        u32                             : 8;
    } bitc;
} reg_ip_graphic_stop1_fact_t;

/*!
  the union of register reg_ip_graphic_stop2_fact
  */
typedef union reg_ip_graphic_stop2_fact
{
    u32 all;
    struct
    {
        u32 stop2_fact                  : 24;
        u32                             : 8;
    } bitc;
} reg_ip_graphic_stop2_fact_t;

/*!
  the union of register reg_ip_graphic_comp_cfg
  */
typedef union reg_ip_graphic_comp_cfg
{
    u32 all;
    struct
    {
        u32 comp_mod                    : 1;
        u32                             : 3;
        u32 comp_key_set                : 1;
        u32                             : 3;
        u32 clip_logic_en               : 1;
        u32                             : 3;
        u32 comp_key_msk                : 1;
        u32                             : 3;
        u32 comp_src1_en                : 1;
        u32                             : 7;
        u32 comp_bp                     : 4;
        u32 comp_test                   : 4;
    } bitc;
} reg_ip_graphic_comp_cfg_t;

/*!
  the union of register reg_ip_graphic_comp_mult_mod
  */
typedef union reg_ip_graphic_comp_mult_mod
{
    u32 all;
    struct
    {
        u32 src1_glb_alp                : 8;
        u32 src1_glb_alp_en             : 1;
        u32 src1_premult_en             : 1;
        u32                             : 2;
        u32 src1_mult_mod               : 2;
        u32                             : 2;
        u32 src3_glb_alp                : 8;
        u32 src3_glb_alp_en             : 1;
        u32 src3_premult_en             : 1;
        u32                             : 2;
        u32 src3_mult_mod               : 2;
        u32                             : 2;
    } bitc;
} reg_ip_graphic_comp_mult_mod_t;

/*!
  the union of register reg_ip_graphic_comp_bld_mod
  */
typedef union reg_ip_graphic_comp_bld_mod
{
    u32 all;
    struct
    {
        u32 src1_color_bld_mod          : 4;
        u32 src2_color_bld_mod          : 4;
        u32 src1_alp_bld_mod            : 4;
        u32 src2_alp_bld_mod            : 4;
        u32                             : 16;
    } bitc;
} reg_ip_graphic_comp_bld_mod_t;

/*!
  the union of register reg_ip_graphic_rop_id
  */
typedef union reg_ip_graphic_rop_id
{
    u32 all;
    struct
    {
        u32 rop_id                      : 16;
        u32                             : 16;
    } bitc;
} reg_ip_graphic_rop_id_t;

/*!
  the union of register reg_ip_graphic_rop_pat
  */
typedef union reg_ip_graphic_rop_pat
{
    u32 all;
    struct
    {
        u32 rop_pat                     : 32;
    } bitc;
} reg_ip_graphic_rop_pat_t;

/*!
  the union of register reg_ip_graphic_comp_status
  */
typedef union reg_ip_graphic_comp_status
{
    u32 all;
    struct
    {
        u32 x                           : 12;
        u32 comp_out_rdy                : 1;
        u32 src1_out_rdy                : 1;
        u32 src2_out_rdy                : 1;
        u32 src3_out_rdy                : 1;
        u32 y                           : 12;
        u32 diff_flg                    : 1;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_comp_status_t;

/*!
  the union of register reg_ip_graphic_load_en
  */
typedef union reg_ip_graphic_load_en
{
    u32 all;
    struct
    {
        u32 pal1_load_en                : 1;
        u32                             : 3;
        u32 pal3_load_en                : 1;
        u32                             : 3;
        u32 coef_load_en                : 1;
        u32                             : 7;
        u32 pal1_swap_mod               : 2;
        u32                             : 2;
        u32 pal3_swap_mod               : 2;
        u32                             : 10;
    } bitc;
} reg_ip_graphic_load_en_t;

/*!
  the union of register reg_ip_graphic_pal_size
  */
typedef union reg_ip_graphic_pal_size
{
    u32 all;
    struct
    {
        u32 pal1_size                   : 9;
        u32                             : 7;
        u32 pal3_size                   : 9;
        u32                             : 7;
    } bitc;
} reg_ip_graphic_pal_size_t;

/*!
  the union of register reg_ip_graphic_pal1_addr
  */
typedef union reg_ip_graphic_pal1_addr
{
    u32 all;
    struct
    {
        u32 pal1_addr                   : 29;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_pal1_addr_t;

/*!
  the union of register reg_ip_graphic_pal3_addr
  */
typedef union reg_ip_graphic_pal3_addr
{
    u32 all;
    struct
    {
        u32 pal3_addr                   : 29;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_pal3_addr_t;

/*!
  the union of register reg_ip_graphic_coef_addr
  */
typedef union reg_ip_graphic_coef_addr
{
    u32 all;
    struct
    {
        u32 coef_addr                   : 29;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_coef_addr_t;

/*!
  the union of register reg_ip_graphic_ctrl_status
  */
typedef union reg_ip_graphic_ctrl_status
{
    u32 all;
    struct
    {
        u32 ctrl_status                 : 32;
    } bitc;
} reg_ip_graphic_ctrl_status_t;

/*!
  the union of register reg_ip_graphic_src1_tile_cfg
  */
typedef union reg_ip_graphic_src1_tile_cfg
{
    u32 all;
    struct
    {
        u32 src1_tile_cfg               : 2;
        u32                             : 2;
        u32 src1_tile_size              : 2;
        u32                             : 2;
        u32 src1_tile_hd                : 1;
        u32                             : 3;
        u32 src1_tile_fld               : 1;
        u32                             : 19;
    } bitc;
} reg_ip_graphic_src1_tile_cfg_t;

/*!
  the union of register reg_ip_graphic_src1_tile_jmp00
  */
typedef union reg_ip_graphic_src1_tile_jmp00
{
    u32 all;
    struct
    {
        u32 src1_tile_jmp00             : 32;
    } bitc;
} reg_ip_graphic_src1_tile_jmp00_t;

/*!
  the union of register reg_ip_graphic_src1_tile_jmp01
  */
typedef union reg_ip_graphic_src1_tile_jmp01
{
    u32 all;
    struct
    {
        u32 src1_tile_jmp01             : 32;
    } bitc;
} reg_ip_graphic_src1_tile_jmp01_t;

/*!
  the union of register reg_ip_graphic_src1_tile_jmp10
  */
typedef union reg_ip_graphic_src1_tile_jmp10
{
    u32 all;
    struct
    {
        u32 src1_tile_jmp10             : 32;
    } bitc;
} reg_ip_graphic_src1_tile_jmp10_t;

/*!
  the union of register reg_ip_graphic_src1_tile_jmp11
  */
typedef union reg_ip_graphic_src1_tile_jmp11
{
    u32 all;
    struct
    {
        u32 src1_tile_jmp11             : 32;
    } bitc;
} reg_ip_graphic_src1_tile_jmp11_t;

/*!
  the union of register reg_ip_graphic_src0_req_status0
  */
typedef union reg_ip_graphic_src0_req_status0
{
    u32 all;
    struct
    {
        u32 src0_req_status0            : 32;
    } bitc;
} reg_ip_graphic_src0_req_status0_t;

/*!
  the union of register reg_ip_graphic_src0_req_status1
  */
typedef union reg_ip_graphic_src0_req_status1
{
    u32 all;
    struct
    {
        u32 src0_req_status1            : 32;
    } bitc;
} reg_ip_graphic_src0_req_status1_t;

/*!
  the union of register reg_ip_graphic_src0_req_status2
  */
typedef union reg_ip_graphic_src0_req_status2
{
    u32 all;
    struct
    {
        u32 src0_req_status2            : 32;
    } bitc;
} reg_ip_graphic_src0_req_status2_t;

/*!
  the union of register reg_ip_graphic_src0_req_status3
  */
typedef union reg_ip_graphic_src0_req_status3
{
    u32 all;
    struct
    {
        u32 src0_req_status3            : 32;
    } bitc;
} reg_ip_graphic_src0_req_status3_t;

/*!
  the union of register reg_ip_graphic_src1_req_status0
  */
typedef union reg_ip_graphic_src1_req_status0
{
    u32 all;
    struct
    {
        u32 src1_req_status0            : 32;
    } bitc;
} reg_ip_graphic_src1_req_status0_t;

/*!
  the union of register reg_ip_graphic_src1_req_status1
  */
typedef union reg_ip_graphic_src1_req_status1
{
    u32 all;
    struct
    {
        u32 src1_req_status1            : 32;
    } bitc;
} reg_ip_graphic_src1_req_status1_t;

/*!
  the union of register reg_ip_graphic_src1_req_status2
  */
typedef union reg_ip_graphic_src1_req_status2
{
    u32 all;
    struct
    {
        u32 src1_req_status2            : 32;
    } bitc;
} reg_ip_graphic_src1_req_status2_t;

/*!
  the union of register reg_ip_graphic_src1_req_status3
  */
typedef union reg_ip_graphic_src1_req_status3
{
    u32 all;
    struct
    {
        u32 src1_req_status3            : 32;
    } bitc;
} reg_ip_graphic_src1_req_status3_t;

/*!
  the union of register reg_ip_graphic_src2_req_status0
  */
typedef union reg_ip_graphic_src2_req_status0
{
    u32 all;
    struct
    {
        u32 src2_req_status0            : 32;
    } bitc;
} reg_ip_graphic_src2_req_status0_t;

/*!
  the union of register reg_ip_graphic_src2_req_status1
  */
typedef union reg_ip_graphic_src2_req_status1
{
    u32 all;
    struct
    {
        u32 src2_req_status1            : 32;
    } bitc;
} reg_ip_graphic_src2_req_status1_t;

/*!
  the union of register reg_ip_graphic_src2_req_status2
  */
typedef union reg_ip_graphic_src2_req_status2
{
    u32 all;
    struct
    {
        u32 src2_req_status2            : 32;
    } bitc;
} reg_ip_graphic_src2_req_status2_t;

/*!
  the union of register reg_ip_graphic_src2_req_status3
  */
typedef union reg_ip_graphic_src2_req_status3
{
    u32 all;
    struct
    {
        u32 src2_req_status3            : 32;
    } bitc;
} reg_ip_graphic_src2_req_status3_t;

/*!
  the union of register reg_ip_graphic_src3_req_status0
  */
typedef union reg_ip_graphic_src3_req_status0
{
    u32 all;
    struct
    {
        u32 src3_req_status0            : 32;
    } bitc;
} reg_ip_graphic_src3_req_status0_t;

/*!
  the union of register reg_ip_graphic_src3_req_status1
  */
typedef union reg_ip_graphic_src3_req_status1
{
    u32 all;
    struct
    {
        u32 src3_req_status1            : 32;
    } bitc;
} reg_ip_graphic_src3_req_status1_t;

/*!
  the union of register reg_ip_graphic_src3_req_status2
  */
typedef union reg_ip_graphic_src3_req_status2
{
    u32 all;
    struct
    {
        u32 src3_req_status2            : 32;
    } bitc;
} reg_ip_graphic_src3_req_status2_t;

/*!
  the union of register reg_ip_graphic_src3_req_status3
  */
typedef union reg_ip_graphic_src3_req_status3
{
    u32 all;
    struct
    {
        u32 src3_req_status3            : 32;
    } bitc;
} reg_ip_graphic_src3_req_status3_t;

/*!
  the union of register reg_ip_graphic__status0
  */
typedef union reg_ip_graphic__status0
{
    u32 all;
    struct
    {
        u32 pat_status0                 : 32;
    } bitc;
} reg_ip_graphic__status0_t;

/*!
  the union of register reg_ip_graphic_pat_status1
  */
typedef union reg_ip_graphic_pat_status1
{
    u32 all;
    struct
    {
        u32 pat_status1                 : 32;
    } bitc;
} reg_ip_graphic_pat_status1_t;

/*!
  the union of register reg_ip_graphic_pat_status2
  */
typedef union reg_ip_graphic_pat_status2
{
    u32 all;
    struct
    {
        u32 pat_status2                 : 32;
    } bitc;
} reg_ip_graphic_pat_status2_t;

/*!
  the union of register reg_ip_graphic_pat_status3
  */
typedef union reg_ip_graphic_pat_status3
{
    u32 all;
    struct
    {
        u32 pat_status3                 : 32;
    } bitc;
} reg_ip_graphic_pat_status3_t;

/*!
  the union of register reg_ip_graphic_gm_status0
  */
typedef union reg_ip_graphic_gm_status0
{
    u32 all;
    struct
    {
        u32 gm_status0                  : 32;
    } bitc;
} reg_ip_graphic_gm_status0_t;

/*!
  the union of register reg_ip_graphic_gm_status1
  */
typedef union reg_ip_graphic_gm_status1
{
    u32 all;
    struct
    {
        u32 gm_status1                  : 32;
    } bitc;
} reg_ip_graphic_gm_status1_t;

/*!
  the union of register reg_ip_graphic_gm_status2
  */
typedef union reg_ip_graphic_gm_status2
{
    u32 all;
    struct
    {
        u32 gm_status2                  : 32;
    } bitc;
} reg_ip_graphic_gm_status2_t;

/*!
  the union of register reg_ip_graphic_gm_status3
  */
typedef union reg_ip_graphic_gm_status3
{
    u32 all;
    struct
    {
        u32 gm_status3                  : 32;
    } bitc;
} reg_ip_graphic_gm_status3_t;

/*!
  the union of register reg_ip_graphic_gra_int_en
  */
typedef union reg_ip_graphic_gra_int_en
{
    u32 all;
    struct
    {
        u32 gra_int_en                  : 32;
    } bitc;
} reg_ip_graphic_gra_int_en_t;

/*!
  the union of register reg_ip_graphic_gra_int_state
  */
typedef union reg_ip_graphic_gra_int_state
{
    u32 all;
    struct
    {
        u32 gra_int_state               : 32;
    } bitc;
} reg_ip_graphic_gra_int_state_t;

/*!
  the union of register reg_ip_graphic_gra_state
  */
typedef union reg_ip_graphic_gra_state
{
    u32 all;
    struct
    {
        u32 gra_state                   : 32;
    } bitc;
} reg_ip_graphic_gra_state_t;

/*!
  the union of register reg_ip_graphic_gra_int_mod
  */
typedef union reg_ip_graphic_gra_int_mod
{
    u32 all;
    struct
    {
        u32 gra_int_mod                 : 2;
        u32                             : 30;
    } bitc;
} reg_ip_graphic_gra_int_mod_t;

/*!
  the union of register reg_ip_graphic_grp0_cscp_0
  */
typedef union reg_ip_graphic_grp0_cscp_0
{
    u32 all;
    struct
    {
        u32 grp0_cscp_00                : 11;
        u32                             : 5;
        u32 grp0_cscp_01                : 11;
        u32                             : 5;
    } bitc;
} reg_ip_graphic_grp0_cscp_0_t;

/*!
  the union of register reg_ip_graphic_grp0_cscp_1
  */
typedef union reg_ip_graphic_grp0_cscp_1
{
    u32 all;
    struct
    {
        u32 grp0_cscp_02                : 11;
        u32                             : 5;
        u32 grp0_cscp_10                : 11;
        u32                             : 5;
    } bitc;
} reg_ip_graphic_grp0_cscp_1_t;

/*!
  the union of register reg_ip_graphic_grp0_cscp_2
  */
typedef union reg_ip_graphic_grp0_cscp_2
{
    u32 all;
    struct
    {
        u32 grp0_cscp_11                : 11;
        u32                             : 5;
        u32 grp0_cscp_12                : 11;
        u32                             : 5;
    } bitc;
} reg_ip_graphic_grp0_cscp_2_t;

/*!
  the union of register reg_ip_graphic_grp0_cscp_3
  */
typedef union reg_ip_graphic_grp0_cscp_3
{
    u32 all;
    struct
    {
        u32 grp0_cscp_20                : 11;
        u32                             : 5;
        u32 grp0_cscp_21                : 11;
        u32                             : 5;
    } bitc;
} reg_ip_graphic_grp0_cscp_3_t;

/*!
  the union of register reg_ip_graphic_grp0_cscp_4
  */
typedef union reg_ip_graphic_grp0_cscp_4
{
    u32 all;
    struct
    {
        u32 grp0_cscp_22                : 11;
        u32                             : 21;
    } bitc;
} reg_ip_graphic_grp0_cscp_4_t;

/*!
  the union of register reg_ip_graphic_grp0_cscdc_0
  */
typedef union reg_ip_graphic_grp0_cscdc_0
{
    u32 all;
    struct
    {
        u32 grp0_cscdc_0                : 19;
        u32                             : 13;
    } bitc;
} reg_ip_graphic_grp0_cscdc_0_t;

/*!
  the union of register reg_ip_graphic_grp0_cscdc_1
  */
typedef union reg_ip_graphic_grp0_cscdc_1
{
    u32 all;
    struct
    {
        u32 grp0_cscdc_1                : 19;
        u32                             : 13;
    } bitc;
} reg_ip_graphic_grp0_cscdc_1_t;

/*!
  the union of register reg_ip_graphic_grp0_cscdc_2
  */
typedef union reg_ip_graphic_grp0_cscdc_2
{
    u32 all;
    struct
    {
        u32 grp0_cscdc_2                : 19;
        u32                             : 13;
    } bitc;
} reg_ip_graphic_grp0_cscdc_2_t;

/*!
  the union of register reg_ip_graphic_grp1_cscp_0
  */
typedef union reg_ip_graphic_grp1_cscp_0
{
    u32 all;
    struct
    {
        u32 grp1_cscp_00                : 11;
        u32                             : 5;
        u32 grp1_cscp_01                : 11;
        u32                             : 5;
    } bitc;
} reg_ip_graphic_grp1_cscp_0_t;

/*!
  the union of register reg_ip_graphic_grp1_cscp_1
  */
typedef union reg_ip_graphic_grp1_cscp_1
{
    u32 all;
    struct
    {
        u32 grp1_cscp_02                : 11;
        u32                             : 5;
        u32 grp1_cscp_10                : 11;
        u32                             : 5;
    } bitc;
} reg_ip_graphic_grp1_cscp_1_t;

/*!
  the union of register reg_ip_graphic_grp1_cscp_2
  */
typedef union reg_ip_graphic_grp1_cscp_2
{
    u32 all;
    struct
    {
        u32 grp1_cscp_11                : 11;
        u32                             : 5;
        u32 grp1_cscp_12                : 11;
        u32                             : 5;
    } bitc;
} reg_ip_graphic_grp1_cscp_2_t;

/*!
  the union of register reg_ip_graphic_grp1_cscp_3
  */
typedef union reg_ip_graphic_grp1_cscp_3
{
    u32 all;
    struct
    {
        u32 grp1_cscp_20                : 11;
        u32                             : 5;
        u32 grp1_cscp_21                : 11;
        u32                             : 5;
    } bitc;
} reg_ip_graphic_grp1_cscp_3_t;

/*!
  the union of register reg_ip_graphic_grp1_cscp_4
  */
typedef union reg_ip_graphic_grp1_cscp_4
{
    u32 all;
    struct
    {
        u32 grp1_cscp_22                : 11;
        u32                             : 21;
    } bitc;
} reg_ip_graphic_grp1_cscp_4_t;

/*!
  the union of register reg_ip_graphic_grp1_cscdc_0
  */
typedef union reg_ip_graphic_grp1_cscdc_0
{
    u32 all;
    struct
    {
        u32 grp1_cscdc_0                : 19;
        u32                             : 13;
    } bitc;
} reg_ip_graphic_grp1_cscdc_0_t;

/*!
  the union of register reg_ip_graphic_grp1_cscdc_1
  */
typedef union reg_ip_graphic_grp1_cscdc_1
{
    u32 all;
    struct
    {
        u32 grp1_cscdc_1                : 19;
        u32                             : 13;
    } bitc;
} reg_ip_graphic_grp1_cscdc_1_t;

/*!
  the union of register reg_ip_graphic_grp1_cscdc_2
  */
typedef union reg_ip_graphic_grp1_cscdc_2
{
    u32 all;
    struct
    {
        u32 grp1_cscdc_2                : 19;
        u32                             : 13;
    } bitc;
} reg_ip_graphic_grp1_cscdc_2_t;

/*!
  the union of register reg_ip_graphic_gradt_radius_a_0
  */
typedef union reg_ip_graphic_gradt_radius_a_0
{
    u32 all;
    struct
    {
        u32 gradt_radius_a_0            : 32;
    } bitc;
} reg_ip_graphic_gradt_radius_a_0_t;

/*!
  the union of register reg_ip_graphic_gradt_radius_a_1
  */
typedef union reg_ip_graphic_gradt_radius_a_1
{
    u32 all;
    struct
    {
        u32 gradt_radius_a_1            : 12;
        u32                             : 20;
    } bitc;
} reg_ip_graphic_gradt_radius_a_1_t;

/*!
  the union of register reg_ip_graphic_gradt_radius_b_0
  */
typedef union reg_ip_graphic_gradt_radius_b_0
{
    u32 all;
    struct
    {
        u32 gradt_radius_b_0            : 32;
    } bitc;
} reg_ip_graphic_gradt_radius_b_0_t;

/*!
  the union of register reg_ip_graphic_gradt_radius_b_1
  */
typedef union reg_ip_graphic_gradt_radius_b_1
{
    u32 all;
    struct
    {
        u32 gradt_radius_b_1            : 12;
        u32                             : 20;
    } bitc;
} reg_ip_graphic_gradt_radius_b_1_t;

/*!
  the union of register reg_ip_graphic_gradt_radius_c_0
  */
typedef union reg_ip_graphic_gradt_radius_c_0
{
    u32 all;
    struct
    {
        u32 gradt_radius_c_0            : 32;
    } bitc;
} reg_ip_graphic_gradt_radius_c_0_t;

/*!
  the union of register reg_ip_graphic_gradt_radius_c_1
  */
typedef union reg_ip_graphic_gradt_radius_c_1
{
    u32 all;
    struct
    {
        u32 gradt_radius_c_1            : 12;
        u32                             : 20;
    } bitc;
} reg_ip_graphic_gradt_radius_c_1_t;

/*!
  the union of register reg_ip_graphic_gradt_radius_d_0
  */
typedef union reg_ip_graphic_gradt_radius_d_0
{
    u32 all;
    struct
    {
        u32 gradt_radius_d_0            : 32;
    } bitc;
} reg_ip_graphic_gradt_radius_d_0_t;

/*!
  the union of register reg_ip_graphic_gradt_radius_d_1
  */
typedef union reg_ip_graphic_gradt_radius_d_1
{
    u32 all;
    struct
    {
        u32 gradt_radius_d_1            : 21;
        u32                             : 11;
    } bitc;
} reg_ip_graphic_gradt_radius_d_1_t;

/*!
  the union of register reg_ip_graphic_gradt_radius_e_0
  */
typedef union reg_ip_graphic_gradt_radius_e_0
{
    u32 all;
    struct
    {
        u32 gradt_radius_e_0            : 32;
    } bitc;
} reg_ip_graphic_gradt_radius_e_0_t;

/*!
  the union of register reg_ip_graphic_gradt_radius_e_1
  */
typedef union reg_ip_graphic_gradt_radius_e_1
{
    u32 all;
    struct
    {
        u32 gradt_radius_e_1            : 12;
        u32                             : 20;
    } bitc;
} reg_ip_graphic_gradt_radius_e_1_t;

/*!
  the union of register reg_ip_graphic_gra_pin_sel
  */
typedef union reg_ip_graphic_gra_pin_sel
{
    u32 all;
    struct
    {
        u32 gra_pin_sel                 : 32;
    } bitc;
} reg_ip_graphic_gra_pin_sel_t;

/*!
  the union of register reg_ip_graphic_gra_mac_set
  */
typedef union reg_ip_graphic_gra_mac_set
{
    u32 all;
    struct
    {
        u32 gra_mac_set                 : 32;
    } bitc;
} reg_ip_graphic_gra_mac_set_t;

/*!
  the union of register reg_ip_graphic_gra_req_cfg
  */
typedef union reg_ip_graphic_gra_req_cfg
{
    u32 all;
    struct
    {
        u32 src0_req_cfg                : 4;
        u32 src1_req_cfg                : 4;
        u32 src2_req_cfg                : 4;
        u32 src3_req_cfg                : 4;
        u32 dst0_req_cfg                : 4;
        u32 dst1_req_cfg                : 4;
        u32 axi_rd_cmd_dep              : 4;
        u32 axi_wr_last_mod             : 1;
        u32                             : 3;
    } bitc;
} reg_ip_graphic_gra_req_cfg_t;

/*!
  the union of register reg_ip_graphic_src0_pix_status0
  */
typedef union reg_ip_graphic_src0_pix_status0
{
    u32 all;
    struct
    {
        u32 src0_pix_status0            : 32;
    } bitc;
} reg_ip_graphic_src0_pix_status0_t;

/*!
  the union of register reg_ip_graphic_src0_pix_status1
  */
typedef union reg_ip_graphic_src0_pix_status1
{
    u32 all;
    struct
    {
        u32 src0_pix_status1            : 32;
    } bitc;
} reg_ip_graphic_src0_pix_status1_t;

/*!
  the union of register reg_ip_graphic_src1_pix_status0
  */
typedef union reg_ip_graphic_src1_pix_status0
{
    u32 all;
    struct
    {
        u32 src1_pix_status0            : 32;
    } bitc;
} reg_ip_graphic_src1_pix_status0_t;

/*!
  the union of register reg_ip_graphic_src1_pix_status1
  */
typedef union reg_ip_graphic_src1_pix_status1
{
    u32 all;
    struct
    {
        u32 src1_pix_status1            : 32;
    } bitc;
} reg_ip_graphic_src1_pix_status1_t;

/*!
  the union of register reg_ip_graphic_src2_pix_status0
  */
typedef union reg_ip_graphic_src2_pix_status0
{
    u32 all;
    struct
    {
        u32 src2_pix_status0            : 32;
    } bitc;
} reg_ip_graphic_src2_pix_status0_t;

/*!
  the union of register reg_ip_graphic_src2_pix_status1
  */
typedef union reg_ip_graphic_src2_pix_status1
{
    u32 all;
    struct
    {
        u32 src2_pix_status1            : 32;
    } bitc;
} reg_ip_graphic_src2_pix_status1_t;

/*!
  the union of register reg_ip_graphic_src3_pix_status0
  */
typedef union reg_ip_graphic_src3_pix_status0
{
    u32 all;
    struct
    {
        u32 src3_pix_status0            : 32;
    } bitc;
} reg_ip_graphic_src3_pix_status0_t;

/*!
  the union of register reg_ip_graphic_src3_pix_status1
  */
typedef union reg_ip_graphic_src3_pix_status1
{
    u32 all;
    struct
    {
        u32 src3_pix_status1            : 32;
    } bitc;
} reg_ip_graphic_src3_pix_status1_t;

/*!
  the union of register reg_ip_graphic_gm_status4
  */
typedef union reg_ip_graphic_gm_status4
{
    u32 all;
    struct
    {
        u32 gm_status4                  : 32;
    } bitc;
} reg_ip_graphic_gm_status4_t;

/*!
  the union of register reg_ip_graphic_gm_status5
  */
typedef union reg_ip_graphic_gm_status5
{
    u32 all;
    struct
    {
        u32 gm_status5                  : 32;
    } bitc;
} reg_ip_graphic_gm_status5_t;

/*!
  the union of register reg_ip_graphic_gm_status6
  */
typedef union reg_ip_graphic_gm_status6
{
    u32 all;
    struct
    {
        u32 gm_status6                  : 32;
    } bitc;
} reg_ip_graphic_gm_status6_t;

/*!
  the union of register reg_ip_graphic_gm_status7
  */
typedef union reg_ip_graphic_gm_status7
{
    u32 all;
    struct
    {
        u32 gm_status7                  : 32;
    } bitc;
} reg_ip_graphic_gm_status7_t;



typedef reg_ip_graphic_gra_eng_start_t   REG_GRA_ENG_START        ;
typedef reg_ip_graphic_gra_eng_en_t   REG_GRA_EN               ;
typedef reg_ip_graphic_gra_eng_cfg_t   REG_GRA_ENG_CFG          ;
typedef reg_ip_graphic_gra_core_done_t   REG_GRA_CORE_DONE        ;
typedef reg_ip_graphic_src0_fmt_cfg0_t   REG_SRC0_FMT_CFG0        ;
typedef reg_ip_graphic_src0_fmt_cfg1_t   REG_SRC0_FMT_CFG1        ;
typedef reg_ip_graphic_src0_pic_addr_t   REG_SRC0_PIC_ADDR        ;
typedef reg_ip_graphic_src0_pic_stride_t   REG_SRC0_PIC_STRIDE      ;
typedef reg_ip_graphic_src1_fmt_cfg0_t   REG_SRC1_FMT_CFG0        ;
typedef reg_ip_graphic_src1_fmt_cfg1_t   REG_SRC1_FMT_CFG1        ;
typedef reg_ip_graphic_src1_key_min_t   REG_SRC1_KEY_MIN         ;
typedef reg_ip_graphic_src1_key_max_t   REG_SRC1_KEY_MAX         ;
typedef reg_ip_graphic_src1_pic_addr_t   REG_SRC1_PIC_ADDR        ;
typedef reg_ip_graphic_src1_pic_stride_t   REG_SRC1_PIC_STRIDE      ;
typedef reg_ip_graphic_src1_pic_size_t   REG_SRC1_PIC_SIZE        ;
typedef reg_ip_graphic_src1_op_size_t   REG_SRC1_OP_SIZE         ;
typedef reg_ip_graphic_src1_op_pos_t   REG_SRC1_OP_POS          ;
typedef reg_ip_graphic_src1_cmyk_cfg_t   REG_SRC1_CMYK_CFG        ;
typedef reg_ip_graphic_src1_status_t   REG_SRC1_STATUS          ;
typedef reg_ip_graphic_src2_fmt_cfg0_t   REG_SRC2_FMT_CFG0        ;
typedef reg_ip_graphic_src2_fmt_cfg1_t   REG_SRC2_FMT_CFG1        ;
typedef reg_ip_graphic_src2_key_min_t   REG_SRC2_KEY_MIN         ;
typedef reg_ip_graphic_src2_key_max_t   REG_SRC2_KEY_MAX         ;
typedef reg_ip_graphic_src2_pic_addr_t   REG_SRC2_PIC_ADDR        ;
typedef reg_ip_graphic_src2_pic_stride_t   REG_SRC2_PIC_STRIDE      ;
typedef reg_ip_graphic_src2_op_pos_t   REG_SRC2_OP_POS          ;
typedef reg_ip_graphic_src2_status_t   REG_SRC2_STATUS          ;
typedef reg_ip_graphic_src3_fmt_cfg0_t   REG_SRC3_FMT_CFG0        ;
typedef reg_ip_graphic_src3_fmt_cfg1_t   REG_SRC3_FMT_CFG1        ;
typedef reg_ip_graphic_src3_key_max_t   REG_SRC3_KEY_MIN         ;
typedef reg_ip_graphic_src3_key_max_t   REG_SRC3_KEY_MAX         ;
typedef reg_ip_graphic_src3_pic_addr_t   REG_SRC3_PIC_ADDR        ;
typedef reg_ip_graphic_src3_pic_stride_t   REG_SRC3_PIC_STRIDE      ;
typedef reg_ip_graphic_src3_op_pos_t   REG_SRC3_OP_POS          ;
typedef reg_ip_graphic_src3_status_t   REG_SRC3_STATUS          ;
typedef reg_ip_graphic_dst0_fmt_cfg0_t   REG_DST0_FMT_CFG0        ;
typedef reg_ip_graphic_dst0_fmt_cfg1_t   REG_DST0_FMT_CFG1        ;
typedef reg_ip_graphic_dst0_pic_addr_t   REG_DST0_PIC_ADDR        ;
typedef reg_ip_graphic_dst0_pic_stride_t   REG_DST0_PIC_STRIDE      ;
typedef reg_ip_graphic_dst0_op_pos_t   REG_DST0_OP_POS          ;
typedef reg_ip_graphic_dst1_fmt_cfg0_t   REG_DST1_FMT_CFG0        ;
typedef reg_ip_graphic_dst1_fmt_cfg1_t   REG_DST1_FMT_CFG1        ;
typedef reg_ip_graphic_dst1_pic_addr_t   REG_DST1_PIC_ADDR        ;
typedef reg_ip_graphic_dst1_pic_stride_t   REG_DST1_PIC_STRIDE      ;
typedef reg_ip_graphic_dst1_op_pos_t   REG_DST1_OP_POS          ;
typedef reg_ip_graphic_dst_pic_size_t   REG_DST_PIC_SIZE         ;
typedef reg_ip_graphic_dst_op_size_t   REG_DST_OP_SIZE          ;
typedef reg_ip_graphic_dst_status_t   REG_DST_STATUS           ;
typedef reg_ip_graphic_cmd_fifo_ctrl_t   REG_CMD_FIFO_CTRL        ;
typedef reg_ip_graphic_cmd_fifo_sync_t   REG_CMD_FIFO_TRIG_CFG    ;
typedef reg_ip_graphic_cmd_fifo_addr_sync_t   REG_CMD_FIFO_ADDR_SYNC   ;
typedef reg_ip_graphic_cmd_fifo_addr_async_t   REG_CMD_FIFO_ADDR_ASYNC  ;
typedef reg_ip_graphic_cmd_fifo_id0_t   REG_CMD_ID0              ;
typedef reg_ip_graphic_cmd_fifo_id1_t   REG_CMD_ID1              ;
typedef reg_ip_graphic_cmd_fifo_status_t   REG_CMD_FIFO_STATUS      ;
typedef reg_ip_graphic_gra_axi_ctrl_t   REG_GRA_AXI_CTRL         ;
typedef reg_ip_graphic_gra_axi_status_t   REG_GRA_AXI_ATATUS       ;
typedef reg_ip_graphic_xylc_cfg_t   REG_XYLC_CFG             ;
typedef reg_ip_graphic_xylc_err_t   REG_XYLC_ERR             ;
typedef reg_ip_graphic_xylc_status_t   REG_XYLC_STATUS          ;
typedef reg_ip_graphic_scaler_cfg_t   REG_SCALER_CFG           ;
typedef reg_ip_graphic_scaler_coef_11_t   REG_SCALER_COEF_11       ;
typedef reg_ip_graphic_scaler_coef_21_t   REG_SCALER_COEF_21       ;
typedef reg_ip_graphic_scaler_coef_31_t   REG_SCALER_COEF_31       ;
typedef reg_ip_graphic_scaler_coef_22_t   REG_SCALER_COEF_22       ;
typedef reg_ip_graphic_scaler_coef_23_t   REG_SCALER_COEF_23       ;
typedef reg_ip_graphic_scaler_init_phase_t   REG_SCALER_INIT_PHASE    ;
typedef reg_ip_graphic_scaler_msk_color_t    REG_SCALER_MSK_COLOR  ;      
typedef reg_ip_graphic_scaler_status_t   REG_SCALER_STATUS        ;
typedef reg_ip_graphic_rot_pat_cfg_t   REG_ROT_PAT_CFG          ;
typedef reg_ip_graphic_pat_color_t   REG_PAT_COLOR            ;
typedef reg_ip_graphic_pat_offset_pos_t   REG_PAT_OFFSET_POS       ;
typedef reg_ip_graphic_pat_ratio_x_0_t   REG_PAT_RATIO_X_0        ;
typedef reg_ip_graphic_pat_ratio_x_1_t   REG_PAT_RATIO_X_1        ;
typedef reg_ip_graphic_pat_ratio_y_0_t   REG_PAT_RATIO_Y_0        ;
typedef reg_ip_graphic_pat_ratio_y_1_t   REG_PAT_RATIO_Y_1        ;
typedef reg_ip_graphic_gradt_cfg_t   REG_GRADT_CFG            ;
typedef reg_ip_graphic_gradt_x_step_t   REG_GRADT_X_STEP         ;
typedef reg_ip_graphic_gradt_y_step_t   REG_GRADT_Y_STEP         ;
typedef reg_ip_graphic_gradt_start_v_t   REG_GRADT_START_V        ;
typedef reg_ip_graphic_stop0_argb_t   REG_STOP0_ARGB           ;
typedef reg_ip_graphic_stop1_argb_t   REG_STOP1_ARGB           ;
typedef reg_ip_graphic_stop2_argb_t   REG_STOP2_ARGB           ;
typedef reg_ip_graphic_stop3_argb_t   REG_STOP3_ARGB           ;
typedef reg_ip_graphic_stop_offset_t   REG_STOP_OFFSET          ;
typedef reg_ip_graphic_stop0_fact_t   REG_STOP0_FACT           ;
typedef reg_ip_graphic_stop1_fact_t   REG_STOP1_FACT           ;
typedef reg_ip_graphic_stop2_fact_t   REG_STOP2_FACT           ;
typedef reg_ip_graphic_comp_cfg_t   REG_COMP_CFG             ;
typedef reg_ip_graphic_comp_mult_mod_t   REG_COMP_MULT_MOD        ;
typedef reg_ip_graphic_comp_bld_mod_t   REG_COMP_BLD_MOD         ;
typedef reg_ip_graphic_rop_id_t   REG_ROP_ID               ;
typedef reg_ip_graphic_rop_pat_t   REG_ROP_PAT              ;
typedef reg_ip_graphic_comp_status_t   REG_COMP_STATUS          ;
typedef reg_ip_graphic_load_en_t   REG_LOAD_EN              ;
typedef reg_ip_graphic_pal_size_t   REG_PAL_SIZE             ;
typedef reg_ip_graphic_pal1_addr_t   REG_PAL1_ADDR            ;
typedef reg_ip_graphic_pal3_addr_t   REG_PAL3_ADDR            ;
typedef reg_ip_graphic_coef_addr_t   REG_COEF_ADDR            ;
typedef reg_ip_graphic_ctrl_status_t   REG_CTRL_STATUS          ; 
typedef reg_ip_graphic_src1_tile_cfg_t   REG_SRC1_TILE_CFG        ;
typedef reg_ip_graphic_src1_tile_jmp00_t   REG_SRC1_TILE_JMP00      ;
typedef reg_ip_graphic_src1_tile_jmp01_t   REG_SRC1_TILE_JMP01      ;
typedef reg_ip_graphic_src1_tile_jmp10_t   REG_SRC1_TILE_JMP10      ;
typedef reg_ip_graphic_src1_tile_jmp11_t   REG_SRC1_TILE_JMP11      ;
typedef  reg_ip_graphic_src0_req_status0_t  REG_SRC0_REQ_STATUS0    ;
typedef  reg_ip_graphic_src0_req_status1_t  REG_SRC0_REQ_STATUS1    ;
typedef  reg_ip_graphic_src0_req_status2_t  REG_SRC0_REQ_STATUS2    ;
typedef  reg_ip_graphic_src0_req_status3_t  REG_SRC0_REQ_STATUS3    ;
typedef  reg_ip_graphic_src1_req_status0_t  REG_SRC1_REQ_STATUS0    ;
typedef  reg_ip_graphic_src1_req_status1_t  REG_SRC1_REQ_STATUS1    ;
typedef  reg_ip_graphic_src1_req_status2_t  REG_SRC1_REQ_STATUS2    ;
typedef  reg_ip_graphic_src1_req_status3_t  REG_SRC1_REQ_STATUS3    ;
typedef  reg_ip_graphic_src2_req_status0_t  REG_SRC2_REQ_STATUS0    ;
typedef  reg_ip_graphic_src2_req_status1_t  REG_SRC2_REQ_STATUS1    ;
typedef  reg_ip_graphic_src2_req_status2_t  REG_SRC2_REQ_STATUS2    ;
typedef  reg_ip_graphic_src2_req_status3_t  REG_SRC2_REQ_STATUS3    ;
typedef  reg_ip_graphic_src3_req_status0_t  REG_SRC3_REQ_STATUS0    ;
typedef  reg_ip_graphic_src3_req_status1_t  REG_SRC3_REQ_STATUS1    ;
typedef  reg_ip_graphic_src3_req_status2_t  REG_SRC3_REQ_STATUS2    ;
typedef  reg_ip_graphic_src3_req_status3_t  REG_SRC3_REQ_STATUS3    ;
typedef  reg_ip_graphic__status0_t  REG__STATUS0            ;
typedef  reg_ip_graphic_pat_status1_t  REG_PAT_STATUS1         ;
typedef  reg_ip_graphic_pat_status2_t  REG_PAT_STATUS2         ;
typedef  reg_ip_graphic_pat_status3_t  REG_PAT_STATUS3         ;
typedef  reg_ip_graphic_gm_status0_t  REG_GM_STATUS0          ;
typedef  reg_ip_graphic_gm_status1_t  REG_GM_STATUS1          ;
typedef  reg_ip_graphic_gm_status2_t  REG_GM_STATUS2          ;
typedef  reg_ip_graphic_gm_status3_t  REG_GM_STATUS3  ;
typedef reg_ip_graphic_gra_int_en_t   REG_GRA_INT_EN           ;
typedef reg_ip_graphic_gra_int_state_t   REG_GRA_INT_STATE        ;
typedef reg_ip_graphic_gra_state_t   REG_GRA_STATE            ;
typedef reg_ip_graphic_gra_int_mod_t   REG_GRA_INT_MOD          ;
typedef reg_ip_graphic_grp0_cscp_0_t   REG_GRP0_CSCP_0          ;
typedef reg_ip_graphic_grp0_cscp_1_t   REG_GRP0_CSCP_1          ;
typedef reg_ip_graphic_grp0_cscp_2_t   REG_GRP0_CSCP_2          ;
typedef reg_ip_graphic_grp0_cscp_3_t   REG_GRP0_CSCP_3          ;
typedef reg_ip_graphic_grp0_cscp_4_t   REG_GRP0_CSCP_4          ;
typedef reg_ip_graphic_grp0_cscdc_0_t   REG_GRP0_CSCDC_0         ;
typedef reg_ip_graphic_grp0_cscdc_1_t   REG_GRP0_CSCDC_1         ;
typedef reg_ip_graphic_grp0_cscdc_2_t   REG_GRP0_CSCDC_2         ;
typedef reg_ip_graphic_grp1_cscp_0_t   REG_GRP1_CSCP_0          ;
typedef reg_ip_graphic_grp1_cscp_1_t   REG_GRP1_CSCP_1          ;
typedef reg_ip_graphic_grp1_cscp_2_t   REG_GRP1_CSCP_2          ;
typedef reg_ip_graphic_grp1_cscp_3_t   REG_GRP1_CSCP_3          ;
typedef reg_ip_graphic_grp1_cscp_4_t   REG_GRP1_CSCP_4          ;
typedef reg_ip_graphic_grp1_cscdc_0_t   REG_GRP1_CSCDC_0         ;
typedef reg_ip_graphic_grp1_cscdc_1_t   REG_GRP1_CSCDC_1         ;
typedef reg_ip_graphic_grp1_cscdc_2_t   REG_GRP1_CSCDC_2         ;
typedef reg_ip_graphic_gradt_radius_a_0_t   REG_GRADT_RADIUS_A_0     ;
typedef reg_ip_graphic_gradt_radius_a_1_t   REG_GRADT_RADIUS_A_1     ;
typedef reg_ip_graphic_gradt_radius_b_0_t   REG_GRADT_RADIUS_B_0     ;
typedef reg_ip_graphic_gradt_radius_b_1_t   REG_GRADT_RADIUS_B_1     ;
typedef reg_ip_graphic_gradt_radius_c_0_t   REG_GRADT_RADIUS_C_0     ;
typedef reg_ip_graphic_gradt_radius_c_1_t   REG_GRADT_RADIUS_C_1     ;
typedef reg_ip_graphic_gradt_radius_d_0_t   REG_GRADT_RADIUS_D_0     ;
typedef reg_ip_graphic_gradt_radius_d_1_t   REG_GRADT_RADIUS_D_1     ;
typedef reg_ip_graphic_gradt_radius_e_0_t   REG_GRADT_RADIUS_E_0     ;
typedef reg_ip_graphic_gradt_radius_e_1_t   REG_GRADT_RADIUS_E_1     ;
typedef reg_ip_graphic_gra_pin_sel_t   REG_GRA_PIN_SEL          ;
typedef reg_ip_graphic_gra_mac_set_t   REG_GRA_MAC_SET          ;
typedef reg_ip_graphic_gra_req_cfg_t   REG_GRA_REQ_CFG          ;
typedef reg_ip_graphic_src0_pix_status0_t   REG_SRC0_PIX_STATUS0     ;
typedef reg_ip_graphic_src0_pix_status1_t   REG_SRC0_PIX_STATUS1     ;
typedef reg_ip_graphic_src1_pix_status0_t   REG_SRC1_PIX_STATUS0     ;
typedef reg_ip_graphic_src1_pix_status1_t   REG_SRC1_PIX_STATUS1     ;
typedef reg_ip_graphic_src2_pix_status0_t   REG_SRC2_PIX_STATUS0     ;
typedef reg_ip_graphic_src2_pix_status1_t   REG_SRC2_PIX_STATUS1     ;
typedef reg_ip_graphic_src3_pix_status0_t   REG_SRC3_PIX_STATUS0     ;
typedef reg_ip_graphic_src3_pix_status1_t   REG_SRC3_PIX_STATUS1     ;
typedef reg_ip_graphic_gm_status4_t   REG_GM_STATUS4          ;
typedef reg_ip_graphic_gm_status5_t   REG_GM_STATUS5          ;
typedef reg_ip_graphic_gm_status6_t   REG_GM_STATUS6          ;
typedef reg_ip_graphic_gm_status7_t   REG_GM_STATUS7          ;


#endif     // __TDE_HAL_REG_ARIA_DEF_H__

