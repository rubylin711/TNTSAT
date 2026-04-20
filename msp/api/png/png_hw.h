/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __JCODEC_SYMPHONY_H__
#define __JCODEC_SYMPHONY_H__

#define PNG_BASE_ADDR 		0xbf1c1000

#define PNG_START 			(0x0)
#define PNG_SIZE 			(0x40)
#define PNG_TYPE 			(0x44)
#define PNG_TRNS_COLOR_R 	(0x50)
#define PNG_TRNS_COLOR_G 	(0x54)
#define PNG_TRNS_COLOR_B 	(0x58)
#define PNG_OUT_TYPE 		(0x80)
#define PNG_CONV_CFG 		(0x84)
#define PNG_PIX_ALP_CFG 	(0x88)
#define PNG_STATE 			(0xe0)
#define PNG_INT_STATE 		(0xe4)
#define PNG_INT_EN 			(0xe8)
#define PNG_STATE_CLR 		(0xf0)
#define PNG_INT_STATE_CLR 	(0xf4)
#define PNG_IDAT_CNT 		(0x100)
#define PNG_AXI_CFG 		(0x104)
#define PNG_AXI_BUSY 		(0x108)
#define PNG_AXI_TEST 		(0x10c)
#define PNG_AXI_IDLE 		(0x110)
#define PNG_ZLIB_CFG 			(0x120)
#define PNG_ZLIB_HDR_INFO 		(0x124)
#define PNG_ZLIB_BLK_HDR_INFO 	(0x128)
#define PNG_ZLIB_DICTID 		(0x12c)
#define PNG_ZLIB_ADLER32_REF 	(0x130)
#define PNG_ZLIB_HNUM 			(0x134)
#define PNG_ZLIB_STATUS 		(0x140)
#define PNG_ZLIB_ADLER32_IMP 	(0x144)
#define PNG_ZLIB_DBG_CFG 		(0x150)
#define PNG_ZLIB_HLIT_MAX 		(0x154)
#define PNG_ZLIB_HDIST_MAX 		(0x158)
#define PNG_ZLIB_HCLEN_MAX 		(0x16c)
#define PNG_ZLIB_MAX_EN 		(0x170)
#define PNG_ZLIB_SYM_TAB 		(0x174)
#define PNG_ZLIB_BS_OUT_CNT 	(0x178)
#define PNG_BUF_START_ADDR 	(0x180)
#define PNG_BUF_END_ADDR 	(0x184)
#define PNG_BS_START_ADDR 	(0x188)
#define PNG_BS_END_ADDR 	(0x18c)
#define PNG_BS_CUR_ADDR 	(0x190)
#define PNG_PASS1_START_ADDR (0x1a0)
#define PNG_PASS2_START_ADDR (0x1a4)
#define PNG_PASS3_START_ADDR (0x1a8)
#define PNG_PASS4_START_ADDR (0x1b0)
#define PNG_PASS5_START_ADDR (0x1b4)
#define PNG_PASS6_START_ADDR (0x1b8)
#define PNG_PASS7_START_ADDR (0x1bc)

#define PNG_PASS1_STRIDE (0x1c0)
#define PNG_PASS2_STRIDE (0x1c4)
#define PNG_PASS3_STRIDE (0x1c8)
#define PNG_PASS4_STRIDE (0x1d0)
#define PNG_PASS5_STRIDE (0x1d4)
#define PNG_PASS6_STRIDE (0x1d8)
#define PNG_PASS7_STRIDE (0x1e0)

#define PNG_FLT_START_ADDR (0x1e4)
#define PNG_ZLIB_START_ADDR (0x1e8)
#define PNG_ZLIB_END_ADDR (0x1ec)
#define PNG_ZLIB_RD_ADDR (0x1f0)
#define PNG_ZLIB_CUR_ADDR (0x1f4)
#define PNG_ZLIB_OUT_CNT (0x200)
#define PNG_HUFF_OUT_CNT (0x204)
#define PNG_ZLIB_BS_TEST_CFG (0x300)
#define PNG_ZLIB_BS_TEST_DAT (0x304)

#define PNG_ALIGN(x, n) (((x) + ((n) - 1)) & (~((n) - 1)))
#define PNG_STATE_DONE (1 << 0)
#define PNG_STATE_BS_IN_DONE (1 << 1)
#define MAX_PNG_TIME (5000)
//#define PNG_ALIGN_N (16)
#define PNG_ALIGN_N (64) //cacheline 64byte


typedef struct
{
	pix_fmt_t out_fmt;
	MT_BOOL comp_dis;
	MT_BOOL eof_flag;
	phys_addr_t bs_start;
	phys_addr_t bs_end;
	phys_addr_t bs_buf_start;
	phys_addr_t bs_buf_end;
	phys_addr_t img_buf_start;
	phys_addr_t filter_buf_start;
	phys_addr_t zout_buf_start;
	phys_addr_t zout_buf_end;
}png_cfg_t;

void dump_reg(void);
mt_u32 calc_filter_buf_size(png_structp pngdec_ptr, png_infop info_ptr);
void png_hw_force_exit(void);
mt_s32 png_hw_start(png_structp pngdec_ptr, png_infop info_ptr, png_cfg_t *p_cfg);
mt_s32 png_hw_restart(mt_u32 bs_start, mt_u32 bs_end, MT_BOOL eof_flag);
MT_BOOL is_png_dec_done(void);
MT_BOOL is_png_bs_in_done(void);
MT_BOOL is_png_err_occured(void);
void get_img_buf_stride(MT_BOOL is_interlace, pix_fmt_t out_fmt, mt_u32 width, mt_u32 *p_stride);
mt_u32 png_get_bpp(pix_fmt_t fmt);

#ifdef CONFIG_MT_FPGA_GPE
void png_int_enable(void);
void png_int_disable(void);
#endif

#endif
