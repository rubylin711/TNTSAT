/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_hdmi20_cfg.h"
#include <stdio.h>
#include <stdlib.h>
// #include <unistd.h>
#include <errno.h>
#include <string.h>
//#include <sys/types.h>
#include <string.h>
//#include <pthread.h>
//#include <assert.h>
#include <linux/fb.h>
//#include <sys/mman.h>
//#include <sys/ioctl.h>
//#include <fcntl.h>
#if (!__HDMI_UBOOT__)
	#include "mt_type.h"
#endif
#include <time.h>

//#include <dlfcn.h>

// #include <sys/select.h>
//#include <signal.h>
// Copyright ?2007, Deco, Inc.  All rights reserved.
//
// No part of this work may be reproduced, modified, distributed, transmitted,
// transcribed, or translated into any language or computer format, in any form
// or by any means without written permission of: Deco, Inc.,
// 1060 East Arques Avenue, Sunnyvale, California 94085
//------------------------------------------------------------------------------
//#define SII_DEBUG 1

/***** #include statements ***************************************************/
#include "conio.h"
#include "si_datatypes.h"
#include "si_lib_malloc_api.h"
#include "si_lib_seq_api.h"
#include "si_lib_obj_api.h"
#include "si_lib_log_api.h"
#include "si_drv_cra_api.h"
#include "si_app_tx_api.h"

//#include "hdmi20.h"
#include "mt_hdmi20_testcase.h"
#include "si_drv_tx_api.h"
#include "misc_reg_op.h"
#include "si_drv_tx_regs.h"
#include "si_vidpath_regs.h"
#include "si_lib_edid_api.h"

#define SII_INFOFRAME_MAX_LEN 31

#define SII_INFOFRAME_AVI_MAX_LEN 17
#define SII_INFOFRAME_AUDIO_MAX_LEN 14
#define SII_INFOFRAME_VS_MAX_LEN 31
void hdmi_block_reset(void);

#if 1
void hdmi_block_reset(void)
{
	mt_u32 ret = 0;
	mt_u32 reg_addr = REG_SYS_BLOCK_RESET;
	mt_u32 reg_val = 0;

	// etude hdmi reset
	ret = misc_reg_read32(REG_SYS_BLOCK_RESET, &reg_val, 1);
	reg_val  &= ~HDMI_CORE_RESET_BIT;
	reg_val  &= ~HDMI_AHB_RESET_BIT;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	ret = misc_reg_read32(REG_SYS_BLOCK_RESET, &reg_val, 1);
	SSHH_PRINTF("HDMI REG_SYS_BLOCK_RESET(0x%x)=0x%x start\n", (u32)REG_SYS_BLOCK_RESET, reg_val);

	SiiLibTimeMilliDelay(1);

	ret = misc_reg_read32(REG_SYS_BLOCK_RESET, &reg_val, 1);
	reg_val  |= HDMI_CORE_RESET_BIT;
	reg_val  |= HDMI_AHB_RESET_BIT;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	SiiLibTimeMilliDelay(1);

	ret = misc_reg_read32(REG_SYS_BLOCK_RESET, &reg_val, 1);
	SSHH_PRINTF("HDMI REG_SYS_BLOCK_RESET(0x%x)=0x%x end\n", (u32)REG_SYS_BLOCK_RESET, reg_val);
	if (ret ) {
		return;
	}
}
#endif

#if (defined(CONFIG_MT_CHIP_ETUDE2))
static mt_void hdmi_set_analog_clock_sw_reset(mt_void)
{
	mt_u32 reg_val = 0;
	mt_u32 reg_addr = 0;
	mt_u32 ret = BB_SUCCESS;

	reg_addr = 0x1f5d0058;
	reg_val = misc_reg_read32_single(reg_addr);
	reg_val |= 1 << 14;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	SiiLibTimeMilliDelay(1);
	//SSHH_PRINTF("\n[%s_%d] reg[0x%x]=exp:0x%x,actual:0x%x\n",__func__,__LINE__,reg_addr,reg_val,misc_reg_read32_single(reg_addr));
	reg_val = misc_reg_read32_single(reg_addr);
	reg_val &= ~(1 << 14);
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	SiiLibTimeMilliDelay(1);
	//SSHH_PRINTF("\n[%s_%d] reg[0x%x]=exp:0x%x,actual:0x%x\n",__func__,__LINE__,reg_addr,reg_val,misc_reg_read32_single(reg_addr));
	if (ret ) {
		return;
	}
}

static mt_void hdmi_set_alanlog_clock(mt_u32 clk_tmds, mt_u32 clk_os_dig)
{
	mt_u32 reg_val = 0;
	mt_u32 reg_addr = 0;
	mt_u32 ret = BB_SUCCESS;
	mt_u32 clk_case = 0xff;

	if (594000 == clk_tmds && 594000 == clk_os_dig) {
		clk_case = 0;
	} else if (297000 == clk_tmds && 594000 == clk_os_dig) {
		clk_case = 1;
	} else if (297000 == clk_tmds && 297000 == clk_os_dig) {
		clk_case = 2;
	} else if (148500 == clk_tmds && 148500 == clk_os_dig) {
		clk_case = 3;
	} else if (74250 == clk_tmds && 74250 == clk_os_dig) {
		clk_case = 4;
	} else if (27000 == clk_tmds && 108000 == clk_os_dig) {
		clk_case = 5;
	}

	reg_addr = 0x1f5d0058;
	reg_val = 0x1CA983FA;
	if (5 == clk_case) {
		reg_val = 0x9C5883FA;
	}
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	hdmi_set_analog_clock_sw_reset();

	reg_addr = 0x1f5d0060;
	switch (clk_case) {
		case 0:
			reg_val = 0x0;
			break;
		case 1:
		case 2:
			reg_val = 0x1;
			break;
		case 3:
			reg_val = 0x2;
			break;
		case 4:
			reg_val = 0x3;
			break;
		case 5:
			reg_val = 0x7;
			break;
		default:
			reg_val = 0x2;
			break;
	}
	ret = misc_reg_put32(reg_addr, 0x7 << 8, reg_val << 8);
	switch (clk_case) {
		case 0:
		case 1:
			reg_val = 0x3;
			break;
		case 2:
			reg_val = 0x2;
			break;
		case 3:
		case 5:
			reg_val = 0x1;
			break;
		case 4:
			reg_val = 0x0;
			break;
		default:
			reg_val = 0x1;
			break;
	}
	ret = misc_reg_put32(reg_addr, 0x3, reg_val);

	reg_addr = 0x1f5d009c;
	ret = misc_reg_put32(reg_addr, 0x7 << 19, 0x5 << 19);

	//reg_addr = 0x1f157000;
	//ret = misc_reg_put32(reg_addr, 0x1<<7, 0x1<<7);

	SiiLibTimeMicroDelay(1);

	//reg_addr = 0x1f157000;
	//ret = misc_reg_put32(reg_addr, 0x1<<7, 0x0<<7);
	#if defined(CONFIG_MT_CHIP_ETUDE2)
	reg_addr = 0x1f5d006c;
	#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
	reg_addr = 0x1f5d01c4;
	#else
	reg_addr = 0x1f5d01c4;
	#endif
	switch (clk_case) {
		case 0:
			reg_val = 0x0030bbcb;
			break;
		case 1:
		case 2:
			reg_val = 0x0030bb0b;
			break;
		case 3:
		case 4:
		case 5:
			reg_val = 0x0030bb09;
			break;
		default:
			reg_val = 0x0030bb09;
			break;
	}
	#if defined(CONFIG_MT_CHIP_ETUDE2)
	reg_val |= misc_reg_read32_single(reg_addr) & (7 << 22);
	#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
	reg_val |= misc_reg_read32_single(reg_addr) & (0x3f << 22);
	#else
	reg_val |= misc_reg_read32_single(reg_addr) & (0x3f << 22);
	#endif
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	#if defined(CONFIG_MT_CHIP_ETUDE2)
	reg_addr = 0x1f5d0244;
	reg_val |= misc_reg_read32_single(reg_addr) & (7 << 22);
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	reg_addr = 0x1f5d0248;
	reg_val |= misc_reg_read32_single(reg_addr) & (7 << 22);
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	#endif

	#if defined(CONFIG_MT_CHIP_ETUDE2)
	reg_addr = 0x1f5d024c;
	#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
	reg_addr = 0x1f5d01c8;
	#else
	reg_addr = 0x1f5d01c8;
	#endif

	switch (clk_case) {
		case 0:
			reg_val = 0x08000078;
			break;
		case 1:
		case 2:
			reg_val = 0x08000038;
			break;
		case 3:
		case 4:
		case 5:
			reg_val = 0x08000008;
			break;
		default:
			reg_val = 0x08000008;
			break;
	}
	#if defined(CONFIG_MT_CHIP_ETUDE2)
	reg_val |= misc_reg_read32_single(reg_addr) & (1 << 8);
	#endif
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	if (ret ) {
		return;
	}
}

static mt_void hdmi_alanlog_resistance_calibration(mt_u32 clk_tmds, mt_u32 clk_os_dig)
{
	mt_u32 reg_addr = 0x1f5d024c;
	mt_u32 ret = BB_SUCCESS;

	ret = misc_reg_put32(reg_addr, 0x1 << 28, 0x0 << 28);
	ret = misc_reg_put32(reg_addr, 0x1 << 29, 0x0 << 29);
	SiiLibTimeMicroDelay(1);
	ret = misc_reg_put32(reg_addr, 0x1 << 29, 0x1 << 29);
	if (ret ) {
		return;
	}
}

static mt_void hdmi_set_digital_clock(mt_u32 clk_tmds, mt_u32 clk_os_dig)
{
	mt_u32 reg_val = 0;
	mt_u32 reg_addr = 0;
	mt_u32 ret = BB_SUCCESS;
	mt_u32 clk_case = 0xff;

	if (594000 == clk_tmds && 594000 == clk_os_dig) {
		clk_case = 0;
	} else if (297000 == clk_tmds && 594000 == clk_os_dig) {
		clk_case = 1;
	} else if (297000 == clk_tmds && 297000 == clk_os_dig) {
		clk_case = 2;
	} else if (148500 == clk_tmds && 148500 == clk_os_dig) {
		clk_case = 3;
	} else if (74250 == clk_tmds && 74250 == clk_os_dig) {
		clk_case = 4;
	}
	/*else if(27000 == clk_tmds && 108000 == clk_os_dig)
		clk_case = 5;*/
	else {
		clk_case = 5;
	}

	reg_addr = 0x1f50a604;
	if (5 == clk_case) {
		reg_val = 0x5;
	} else {
		reg_val = 0;
	}
	ret = misc_reg_put32(reg_addr, 0x7 << 0, reg_val << 0);
	if (5 == clk_case) {
		reg_val = 0x5;
	} else if (1 == clk_case) {
		reg_val = 0x4;
	} else {
		reg_val = 0;
	}
	ret = misc_reg_put32(reg_addr, 0x7 << 3, reg_val << 3);
	ret = misc_reg_put32(reg_addr, 0x7 << 7, 0x0 << 7);
	SiiLibTimeMilliDelay(1);
	if (ret ) {
		return;
	}
}
#endif

static mt_void hdmi_set_kram(mt_u32 addr, mt_u32 val)
{
	#if defined(CONFIG_MT_CHIP_ETUDE2)
	//mt_u8 reg_val = 0;
	mt_u32 reg_addr = 0;
	mt_u32 ret = BB_SUCCESS;
	mt_u32 reg_val = 0;

	//step 4, set kram 0x1fe  to 0x00
	reg_addr = REG_ADDR__KRAM_W_CTRL;
	reg_val = 0x0;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	SiiLibTimeMicroDelay(1);
	reg_addr = REG_ADDR__KRAM_CTRL;
	//reg_val = 0x01fe1000;
	reg_val = 0x00001000;
	reg_val += ((addr & 0x1ff) << 16);//[24:16]
	reg_val += val & 0xff; //[7:0]
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	SiiLibTimeMicroDelay(1);
	reg_addr = REG_ADDR__KRAM_W_CTRL;
	reg_val = 0x0;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	SiiLibTimeMicroDelay(1);
	reg_addr = REG_ADDR__KRAM_W_CTRL;
	reg_val = 0x1;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	SiiLibTimeMicroDelay(1);
	reg_addr = REG_ADDR__KRAM_W_CTRL;
	reg_val = 0x0;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	SiiLibTimeMicroDelay(1);
	reg_addr = REG_ADDR__KRAM_CTRL;
	reg_val = 0x0;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	SiiLibTimeMicroDelay(1);

	ret = misc_reg_put8(REG_ADDR__EPCM, 0xff, 0x20);
	if (ret ) {
		return;
	}
	#endif
}

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
static mt_void hdmi_init_keep_out_win(mt_u32 win_max, mt_u32 win_min, mt_u8 pix4x, mt_u8 en)
{
	mt_u32 ret = BB_SUCCESS;
	/*+++++ CONFIG KEEP OUT WINDOW ++++++*/
	ret = misc_reg_put8(REG_ADDR__KEEP_OUT_WIN_SIZE_0_MT, BIT_MSK__KEEP_OUT_WIN_MAX_B12_B8, (mt_u8)((win_max >> 5)&BIT_MSK__KEEP_OUT_WIN_MAX_B12_B8));
	ret = misc_reg_put8(REG_ADDR__KEEP_OUT_WIN_SIZE_0_MT, BIT_MSK__KEEP_OUT_WIN_MIN_B8, (mt_u8)((win_min >> 6)&BIT_MSK__KEEP_OUT_WIN_MIN_B8));
	ret = misc_reg_put8(REG_ADDR__KEEP_OUT_WIN_CTRL_MT, BIT_MSK__PIX_X4_MODE, (mt_u8)((pix4x << 7)&BIT_MSK__PIX_X4_MODE));
	ret = misc_reg_put8(REG_ADDR__KEEP_OUT_WIN_CTRL_MT, BIT_MSK__KEEP_OUT_WIN_EN_REQ_SEL, (mt_u8)((en << 6)&BIT_MSK__KEEP_OUT_WIN_EN_REQ_SEL));
	ret = misc_reg_put8(REG_ADDR__KEEP_OUT_WIN_MAX_LOW_MT, BIT_MSK__KEEP_OUT_WIN_MAX_B7_B0, (mt_u8)(win_max & BIT_MSK__KEEP_OUT_WIN_MAX_B7_B0));
	ret = misc_reg_put8(REG_ADDR__KEEP_OUT_WIN_MIN_LOW_MT, BIT_MSK__KEEP_OUT_WIN_MIN_B7_B0, (mt_u8)(win_min & BIT_MSK__KEEP_OUT_WIN_MIN_B7_B0));
	/*------- CONFIG KEEP OUT WINDOW --------*/
	if (ret ) {
		return;
	}
}
#endif

static mt_void hdmi_init(mt_u32 cs, mt_u32 first)
{
	mt_u32 ret = BB_SUCCESS;
	//mt_u8 val[2];

	if (first) {
		ret = misc_reg_put8(REG_ADDR__PKT_FILTER_0, 0xff, 0x02);  //SDK not set. default 0
		ret = misc_reg_put8(REG_ADDR__VP__INPUT_SYNC_ADJUST_CONFIG, 0xff, 0x01);  //same with SDK, disable auto adjust sync polarity
		ret = misc_reg_put8(REG_ADDR__VP__CMS__CSC0__MULTI_CSC_CONFIG, 0xff, 0x00);  //video path,  input yuv+8bit+full range
		ret = misc_reg_put8(REG_ADDR__VP__CMS__CSC0__MULTI_CSC_CONFIG + 1, 0xff, 0x08);  //video path,  input yuv+8bit+full range
		ret = misc_reg_put8(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR1C1, 0xff, 0x00);  //video path
		ret = misc_reg_put8(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_MULTCOEFFR1C1 + 1, 0xff, 0x10);  //video path
		#if defined(CONFIG_MT_CHIP_ETUDE2)
		ret = misc_reg_put8(REG_ADDR__BYTE_RW_MODE, 0xff, 0x01);
		#endif
	}
	#if 0
	switch (cs) { //8bit
		case 0:
			//RGB
			val[0] = misc_reg_read8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG);
			val[1] = misc_reg_read8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG + 1);
			ret = misc_reg_write8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG, 0x21);
			ret = misc_reg_write8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG + 1, val[1] & 0xfc);
			ret = misc_reg_write8_single(REG_ADDR__VP__OUTPUT_FORMAT, 0x00);
			ret = misc_reg_write8_single(REG_ADDR__VP__OUTPUT_FORMAT + 1, 0x00);
			ret = misc_reg_write8_single(REG_ADDR__VP__CMS__CSC1__C444_C422_CONFIG, 0x00);
			ret = misc_reg_write8_single(REG_ADDR__VP__CMS__CSC1__C422_C420_CONFIG, 0x02);
			break;
		case 1:
			//YUV422
			val[0] = misc_reg_read8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG);
			val[1] = misc_reg_read8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG + 1);
			ret = misc_reg_write8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG, 0x00);
			ret = misc_reg_write8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG + 1, val[1] & 0xfc);
			val[0] = misc_reg_read8_single(REG_ADDR__VP__OUTPUT_FORMAT);
			val[1] = misc_reg_read8_single(REG_ADDR__VP__OUTPUT_FORMAT + 1);
			ret = misc_reg_write8_single(REG_ADDR__VP__OUTPUT_FORMAT, val[0]);
			ret = misc_reg_put8(REG_ADDR__VP__OUTPUT_FORMAT + 1, 1 << 2, 0 << 2);
			ret = misc_reg_put8(REG_ADDR__VP__CMS__CSC1__C444_C422_CONFIG, 1, 1);
			ret = misc_reg_put8(REG_ADDR__VP__CMS__CSC1__C422_C420_CONFIG, 1, 0);
			break;
		case 2:
			//YUV444
			val[0] = misc_reg_read8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG);
			val[1] = misc_reg_read8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG + 1);
			ret = misc_reg_write8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG, 0x00);
			ret = misc_reg_write8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG + 1, val[1] & 0xfc);
			val[0] = misc_reg_read8_single(REG_ADDR__VP__OUTPUT_FORMAT);
			val[1] = misc_reg_read8_single(REG_ADDR__VP__OUTPUT_FORMAT + 1);
			ret = misc_reg_write8_single(REG_ADDR__VP__OUTPUT_FORMAT, val[0]);
			ret = misc_reg_put8(REG_ADDR__VP__OUTPUT_FORMAT + 1, 1 << 2, 0 << 2);
			ret = misc_reg_put8(REG_ADDR__VP__CMS__CSC1__C444_C422_CONFIG, 1, 0);
			ret = misc_reg_put8(REG_ADDR__VP__CMS__CSC1__C422_C420_CONFIG, 1, 0);
			break;
		case 3:
			//YUV420
			val[0] = misc_reg_read8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG);
			val[1] = misc_reg_read8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG + 1);
			ret = misc_reg_write8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG, 0x00);
			ret = misc_reg_write8_single(REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG + 1, val[1] & 0xfc);
			val[0] = misc_reg_read8_single(REG_ADDR__VP__OUTPUT_FORMAT);
			val[1] = misc_reg_read8_single(REG_ADDR__VP__OUTPUT_FORMAT + 1);
			ret = misc_reg_write8_single(REG_ADDR__VP__OUTPUT_FORMAT, val[0]);
			ret = misc_reg_put8(REG_ADDR__VP__OUTPUT_FORMAT + 1, 1 << 2, 1 << 2);
			ret = misc_reg_put8(REG_ADDR__VP__CMS__CSC1__C444_C422_CONFIG, 1, 1);
			ret = misc_reg_put8(REG_ADDR__VP__CMS__CSC1__C422_C420_CONFIG, 0x3, 1);
			break;
		default:
			break;
	}
	#endif
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	hdmi_init_keep_out_win(636, 240, 0, 1);
	#endif
	if (ret ) {
		return;
	}
}

static mt_void hdmi_send_info(mt_u32 info_type, info_struct_t info)
{
	mt_u32 ret = BB_SUCCESS;
	mt_u32 i;

	ret = misc_reg_put8(REG_ADDR__TPI_INFO_FSEL, 0xff, (mt_u8)info_type);
	ret = misc_reg_put8(REG_ADDR__TPI_INFO_B0, 0xff, info.type);
	ret = misc_reg_put8(REG_ADDR__TPI_INFO_B1, 0xff, info.version);
	ret = misc_reg_put8(REG_ADDR__TPI_INFO_B2, 0xff, info.length);
	for (i = 0; i < info.length + 1; i++) {
		ret = misc_reg_put8(REG_ADDR__TPI_INFO_B3 + i, 0xff, info.pb_byte[i]);
	}
	ret = misc_reg_put8(REG_ADDR__TPI_INFO_EN, 0xff, 0xc0);
	if (ret ) {
		return;
	}
}

#define misc_reg_put32_maskdelay(a,b,c) ({\
		mt_u32 ra;\
		mt_u32 ia;\
		ra = misc_reg_read32_single(a);\
		ia = c & (~(7<<8)) & b;\
		ia |= (ra & (7<<8));\
		ret = misc_reg_write32_single(a,ia);\
	})
static mt_void hdmi_set_hdvenc(mt_u32 tvsys, mt_u32 mode3d)
{
	#define D3_CFG_VAL		(3)
	mt_u32 dtmp = 0;
	/*
	    "0: (99)4096x2160@25_256:135\n"\
	    "1: (101)4096x2160@50_256:135\n"\
	    "2: (100)4096x2160@30_256:135\n"\
	    "3: (102)4096x2160@60_256:135\n"\
	    "4: (98)4096x2160@24_256:135\n"\
	    "5: (94)3840x2160@25_16:9\n"\
	    "6: (96)3840x2160@50_16:9\n"\
	    "7: (95)3840x2160@30_16:9\n"\
	    "8: (97)3840x2160@60_16:9\n"\
	    "9: (93)3840x2160@24_16:9\n"\
	    "10: (31)1080p@50_16:9\n"\
	    "11: (16)1080p@60_16:9\n"\
	    "12: (19)720p@50_16:9\n"\
	    "13: (4)720p@60_16:9\n"\
	    "14: (20)1080i@50_16:9\n"\
	    "15: (5)1080i@60_16:9\n"\
	    "16: (32)1080p@24_16:9\n"\
	    "17: (34)1080p@30_16:9\n"\
	    "18: (33)1080p@25_16:9\n"\
	    "19: (17)576p@50_4:3\n"\
	    "20: (2)480p@60_4:3\n"\
	    "21: (21)576i@50_4:3\n"\
	    "22: (6)480i@60_4:3\n"\
	    "23: (18)576p@50_16:9"\
	    "24: (3)480p@60_16:9"\
	    "25: (22)576i@50_16:9"\
	    "26: (7)480i@60_16:9"\
	    "27: (104)3840x2160@25_64:27\n"\
	    "28: (106)3840x2160@50_64:27\n"\
	    "29: (105)3840x2160@30_64:27\n"\
	    "30: (107)3840x2160@60_64:27\n"\
	    "31: (103)3840x2160@24_64:27\n"\
	    "32: (68)720p@50_64:27\n"\
	    "33: (69)720p@60_64:27\n"\
	    "34: (73)1080p@25_64:27\n"\
	    "35: (74)1080p@30_64:27\n"\
	    "36: (75)1080p@50_64:27\n"\
	    "37: (76)1080p@60_64:27\n"

	*/
	mt_u32 ret = BB_SUCCESS;

	ret = misc_reg_put32(0x1f470014, 0x00000700, 0x00000000);
	switch (tvsys) {
		case 0:
		case 1:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2100002c);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x00587000);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x008003c8);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x00051000);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x000a0008);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x08700048);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x10000001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x08700001);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x000003c8);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x00000058);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000080);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00001000);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000008);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x0000000a);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000048);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x00000870);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x80000200);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xe100002c);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 2:
		case 3:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2100002c);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x00587000);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x00800058);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x00051000);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x000a0008);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x08700048);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x10000001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x08700001);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x00000058);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x00000058);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000080);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00001000);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000008);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x0000000a);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000048);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x00000870);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x80000200);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xe100002c);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 4:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2100002c);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x00587000);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x012803fc);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x00051000);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x000a0008);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x08700048);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x10000001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x08700001);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x000003fc);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x00000058);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000128);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00001000);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000008);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x0000000a);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000048);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x00000870);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x80000200);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xe100002c);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 5:
		case 6:
		case 27:
		case 28:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2100002c);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x00580420);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x01280420);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x00050f00);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x000a0008);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x08700048);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x0f000001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x08700001);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x00000420);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x00000058);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000128);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00000f00);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000008);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x0000000a);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000048);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x00000870);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800001e0);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xe100002c);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 7:
		case 8:
		case 29:
		case 30:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2100002c);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x005800b0);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x012800b0);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x00050f00);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x000a0008);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x08700048);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x0f000001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x08700001);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x000000b0);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x00000058);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000128);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00000f00);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000008);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x0000000a);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000048);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x00000870);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800001e0);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xe100002c);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 9:
		case 31:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2100002c);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x005804fc);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x012804fc);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x00050f00);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x000a0008);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x08700048);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x0f000001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x08700001);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x000004fc);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x00000058);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000128);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00000f00);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000008);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x0000000a);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000048);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x00000870);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800001e0);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xe100002c);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 10:
		case 36:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2011002c);
			ret = misc_reg_put32(0x1f470000, 0xffffffff, 0x00100600);
			ret = misc_reg_put32(0x1f470004, 0xffffffff, 0x00fc02a6);
			ret = misc_reg_put32(0x1f4700f4, 0xffffffff, 0x01080f80);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x005801e4);
			ret = misc_reg_put32(0x1f47001c, 0xffffffff, 0x07bc0058);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x009401e4);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x00010780);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x00050004);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x04380024);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x00000210);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x0000002c);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000094);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00000780);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000004);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x00000005);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000024);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x00000438);
			ret = misc_reg_put32(0x1f470070, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470030, 0xffffffff, 0x07840001);
			ret = misc_reg_put32(0x1f470034, 0xffffffff, 0x04380001);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x07800001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x04380002);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800000f0);
			ret = misc_reg_put32(0x1f4700d8, 0xffffffff, 0x80000000);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xa011002c);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 11:
		case 37:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2011002e);
			ret = misc_reg_put32(0x1f470000, 0xffffffff, 0x00100600);
			ret = misc_reg_put32(0x1f470004, 0xffffffff, 0x110d02d5);
			ret = misc_reg_put32(0x1f4700f4, 0xffffffff, 0xf0000438);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x1058102c);
			ret = misc_reg_put32(0x1f47001c, 0xffffffff, 0x07bc0058);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x0094002c);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x00010780);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x00050004);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x04380024);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x00000058);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x0000002c);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000094);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00000780);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000004);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x00000005);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000024);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x00000438);
			ret = misc_reg_put32(0x1f470070, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470030, 0xffffffff, 0x07850001);
			ret = misc_reg_put32(0x1f470034, 0xffffffff, 0x04380001);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x07800001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x04380001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800000f0);
			ret = misc_reg_put32(0x1f4700d8, 0xffffffff, 0x80000000);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xa011002e);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 12:
		case 32:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2010022c);
			ret = misc_reg_put32(0x1f470000, 0xffffffff, 0x00100601);
			ret = misc_reg_put32(0x1f470004, 0xffffffff, 0x110d02d7);
			ret = misc_reg_put32(0x1f4700f4, 0xffffffff, 0xf00002d0);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x20502190);
			ret = misc_reg_put32(0x1f47001c, 0xffffffff, 0x050000dc);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x00dc0190);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x00010500);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x00050005);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x02d00014);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x000001b8);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x00000028);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x000000dc);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00000500);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000005);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x00000005);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000014);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x000002d0);
			ret = misc_reg_put32(0x1f470070, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470030, 0xffffffff, 0x05040001);
			ret = misc_reg_put32(0x1f470034, 0xffffffff, 0x02d00001);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x05000001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x02d00001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800000a0);
			ret = misc_reg_put32(0x1f4700d8, 0xffffffff, 0xa0010000);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xa010022c);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 13:
		case 33:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2010022e);
			ret = misc_reg_put32(0x1f470000, 0xffffffff, 0x00100601);
			ret = misc_reg_put32(0x1f470004, 0xffffffff, 0x110b02d6);
			ret = misc_reg_put32(0x1f4700f4, 0xffffffff, 0xf00002d0);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x20502046);
			ret = misc_reg_put32(0x1f47001c, 0xffffffff, 0x050000dc);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x00dc0046);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x00010500);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x00050005);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x02d00014);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x0000006e);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x00000028);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x000000dc);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00000500);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000005);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x00000005);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000014);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x000002d0);
			ret = misc_reg_put32(0x1f470070, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470030, 0xffffffff, 0x05050001);
			ret = misc_reg_put32(0x1f470034, 0xffffffff, 0x02d00001);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x05000001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x02d00001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800000a0);
			ret = misc_reg_put32(0x1f4700d8, 0xffffffff, 0xa0010000);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xa010022e);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 14:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2010002d);
			ret = misc_reg_put32(0x1f470000, 0xffffffff, 0x00100610);
			ret = misc_reg_put32(0x1f470004, 0xffffffff, 0x110d02da);
			ret = misc_reg_put32(0x1f4700f4, 0xffffffff, 0xf000021c);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x105811e4);
			ret = misc_reg_put32(0x1f47001c, 0xffffffff, 0x07bc0058);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x009401e4);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x04680780);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x000a0004);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x0438001e);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x00000210);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x0000002c);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000094);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00000780);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000002);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x00000005);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x0000000f);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x0000021c);
			ret = misc_reg_put32(0x1f470070, 0xffffffff, 0x00000468);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470030, 0xffffffff, 0x07840001);
			ret = misc_reg_put32(0x1f470034, 0xffffffff, 0x04380001);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x07800001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x04380001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800000f0);
			ret = misc_reg_put32(0x1f4700d8, 0xffffffff, 0xa0010000);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xa010002d);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 15:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2010002f);
			ret = misc_reg_put32(0x1f470000, 0xffffffff, 0x00100610);
			ret = misc_reg_put32(0x1f470004, 0xffffffff, 0x110f02d6);
			ret = misc_reg_put32(0x1f4700f4, 0xffffffff, 0xf000021c);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x2058202c);
			ret = misc_reg_put32(0x1f47001c, 0xffffffff, 0x07bc0058);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x0094002c);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x038c0780);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x000a0004);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x0438001e);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x00000058);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x0000002c);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000094);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00000780);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000002);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x00000005);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x0000000f);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x0000021c);
			ret = misc_reg_put32(0x1f470070, 0xffffffff, 0x0000038c);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470030, 0xffffffff, 0x07850001);
			ret = misc_reg_put32(0x1f470034, 0xffffffff, 0x04380001);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x07800001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x04380001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800000f0);
			ret = misc_reg_put32(0x1f4700d8, 0xffffffff, 0xa0010000);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xa010002f);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 16:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2001002c);
			ret = misc_reg_put32(0x1f470000, 0xffffffff, 0x00100600);
			ret = misc_reg_put32(0x1f470004, 0xffffffff, 0x010d02da);
			ret = misc_reg_put32(0x1f4700f4, 0xffffffff, 0x73000001);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x00580252);
			ret = misc_reg_put32(0x1f47001c, 0xffffffff, 0x07bc0058);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x00940252);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x00010780);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x00050004);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x04380024);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x0000027e);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x0000002c);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000094);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00000780);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000004);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x00000005);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000024);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x00000438);
			ret = misc_reg_put32(0x1f470070, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470030, 0xffffffff, 0x07810001);
			ret = misc_reg_put32(0x1f470034, 0xffffffff, 0x04380001);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x07800001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x04380001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800000f0);
			ret = misc_reg_put32(0x1f4700d8, 0xffffffff, 0xa0010000);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xa001002c);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 17:
		case 35:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2001002e);
			ret = misc_reg_put32(0x1f470000, 0xffffffff, 0x00100600);
			ret = misc_reg_put32(0x1f470004, 0xffffffff, 0x010d02da);
			ret = misc_reg_put32(0x1f4700f4, 0xffffffff, 0x73000001);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x0058002c);
			ret = misc_reg_put32(0x1f47001c, 0xffffffff, 0x07bc0058);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x0094002c);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x00010780);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x00050004);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x04380024);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x00000058);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x0000002c);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000094);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00000780);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000004);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x00000005);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000024);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x00000438);
			ret = misc_reg_put32(0x1f470070, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470030, 0xffffffff, 0x07830001);
			ret = misc_reg_put32(0x1f470034, 0xffffffff, 0x04380001);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x07800001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x04380001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800000f0);
			ret = misc_reg_put32(0x1f4700d8, 0xffffffff, 0xa0010000);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xa001002e);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 18:
		case 34:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2001002c);
			ret = misc_reg_put32(0x1f470000, 0xffffffff, 0x00100600);
			ret = misc_reg_put32(0x1f470004, 0xffffffff, 0x010d02da);
			ret = misc_reg_put32(0x1f4700f4, 0xffffffff, 0x73000001);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x005801e4);
			ret = misc_reg_put32(0x1f47001c, 0xffffffff, 0x07bc0058);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x009401e4);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x00010780);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x00050004);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x04380024);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x00000210);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x0000002c);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000094);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x00000780);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000004);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x00000005);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000024);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x00000438);
			ret = misc_reg_put32(0x1f470070, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000001);
			ret = misc_reg_put32(0x1f470030, 0xffffffff, 0x07820001);
			ret = misc_reg_put32(0x1f470034, 0xffffffff, 0x04380001);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x07800001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x04380001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800000f0);
			ret = misc_reg_put32(0x1f4700d8, 0xffffffff, 0xa0010000);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xa001002c);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 19:
		case 23:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x20100208);
			ret = misc_reg_put32(0x1f470000, 0xffffffff, 0x00100602);
			ret = misc_reg_put32(0x1f470004, 0xffffffff, 0x111302db);
			ret = misc_reg_put32(0x1f4700f4, 0xffffffff, 0xf0000240);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x403c400c);
			ret = misc_reg_put32(0x1f47001c, 0xffffffff, 0x02d00044);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x00440010);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x000102d0);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x00050005);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x02400027);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x0000000c);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x00000040);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000044);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x000002d0);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000005);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x00000005);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000027);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x00000240);
			ret = misc_reg_put32(0x1f470070, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470030, 0xffffffff, 0x02d40001);
			ret = misc_reg_put32(0x1f470034, 0xffffffff, 0x02400001);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x02d00001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x02400001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x8000005a);
			ret = misc_reg_put32(0x1f4700d8, 0xffffffff, 0xa0000300);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xa0100208);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 20:
		case 24:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2010021a);
			ret = misc_reg_put32(0x1f470000, 0xffffffff, 0x00100603);
			ret = misc_reg_put32(0x1f470004, 0xffffffff, 0x110a02e5);
			ret = misc_reg_put32(0x1f4700f4, 0xffffffff, 0xf00001e0);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x403e4010);
			ret = misc_reg_put32(0x1f47001c, 0xffffffff, 0x02d0003c);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x003c0010);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x000102d0);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x00060009);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x01e0001e);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x00000010);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x0000003e);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x0000003c);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x000002d0);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000009);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x00000006);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x0000001e);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x000001e0);
			ret = misc_reg_put32(0x1f470070, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470030, 0xffffffff, 0x02d50001);
			ret = misc_reg_put32(0x1f470034, 0xffffffff, 0x01e00001);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x02d00001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x01e00001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x8000005a);
			ret = misc_reg_put32(0x1f4700d8, 0xffffffff, 0xa0000300);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xa010021a);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 21:
		case 25:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x20000009);
			ret = misc_reg_put32(0x1f470000, 0xffffffff, 0x00100614);
			ret = misc_reg_put32(0x1f470004, 0xffffffff, 0x110d02da);
			ret = misc_reg_put32(0x1f4700f4, 0xffffffff, 0x73000001);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x007e0018);
			ret = misc_reg_put32(0x1f47001c, 0xffffffff, 0x05a0008a);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x008a0018);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x025805a0);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x00060004);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x02400026);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x00000018);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x0000007e);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x0000008a);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x000005a0);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000002);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x00000003);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x00000013);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x00000120);
			ret = misc_reg_put32(0x1f470070, 0xffffffff, 0x00000258);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470030, 0xffffffff, 0x05a40001);
			ret = misc_reg_put32(0x1f470034, 0xffffffff, 0x02400002);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x05a00001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x02400002);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800000b4);
			ret = misc_reg_put32(0x1f4700d8, 0xffffffff, 0x80000000);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xa0000009);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		case 22:
		case 26:
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0x2000001b);
			ret = misc_reg_put32(0x1f470000, 0xffffffff, 0x00100616);
			ret = misc_reg_put32(0x1f470004, 0xffffffff, 0x110d02da);
			ret = misc_reg_put32(0x1f4700f4, 0xffffffff, 0x73000001);
			ret = misc_reg_put32(0x1f470018, 0xffffffff, 0x007c0026);
			ret = misc_reg_put32(0x1f47001c, 0xffffffff, 0x05a00072);
			ret = misc_reg_put32(0x1f470020, 0xffffffff, 0x00720026);
			ret = misc_reg_put32(0x1f470024, 0xffffffff, 0x026c05a0);
			ret = misc_reg_put32(0x1f470028, 0xffffffff, 0x00060008);
			ret = misc_reg_put32(0x1f47002c, 0xffffffff, 0x01e0001e);
			ret = misc_reg_put32(0x1f470050, 0xffffffff, 0x00000026);
			ret = misc_reg_put32(0x1f470054, 0xffffffff, 0x0000007c);
			ret = misc_reg_put32(0x1f470058, 0xffffffff, 0x00000072);
			ret = misc_reg_put32(0x1f47005c, 0xffffffff, 0x000005a0);
			ret = misc_reg_put32(0x1f470060, 0xffffffff, 0x00000004);
			ret = misc_reg_put32(0x1f470064, 0xffffffff, 0x00000003);
			ret = misc_reg_put32(0x1f470068, 0xffffffff, 0x0000000f);
			ret = misc_reg_put32(0x1f47006c, 0xffffffff, 0x000000f0);
			ret = misc_reg_put32(0x1f470070, 0xffffffff, 0x0000026c);
			ret = misc_reg_put32(0x1f470074, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470078, 0xffffffff, 0x00000000);
			ret = misc_reg_put32(0x1f470030, 0xffffffff, 0x05a50001);
			ret = misc_reg_put32(0x1f470034, 0xffffffff, 0x01e00001);
			ret = misc_reg_put32(0x1f470038, 0xffffffff, 0x05a00001);
			ret = misc_reg_put32(0x1f47003c, 0xffffffff, 0x01e00001);
			ret = misc_reg_put32(0x1f470040, 0xffffffff, 0x800000b4);
			ret = misc_reg_put32(0x1f4700d8, 0xffffffff, 0x80000000);
			ret = misc_reg_read32(0x1f470004, &dtmp, 1);
			if (mode3d == 1) { //frame packing
				dtmp &= ~(3 << 12);dtmp |= D3_CFG_VAL << 12;
			} else {
				dtmp &= ~(3 << 12);
			}
			ret = misc_reg_put32(0x1f470004, 0xffffffff, dtmp);
			ret = misc_reg_put32_maskdelay(0x1f470014, 0xffffffff, 0xa000001b);
			ret = misc_reg_put32(0x1f440000, 0xffffffff, 0x00000100);
			break;
		default:
			break;
	}
	if (ret ) {
		return;
	}
}

#if (defined(CONFIG_MT_CHIP_ETUDE2))
static mt_void hdmi_set_no_glitch_clock(mt_u32 no_glitch_on)
{
	ulong reg_addr = 0;
	mt_u32 ret = BB_SUCCESS;

	reg_addr = 0x1f50a604UL;
	ret = misc_reg_put32(reg_addr, 0x1 << 6, no_glitch_on << 6);
	if (ret ) {
		return;
	}
}

static mt_void hdmi_analog_clk_en(mt_u32 en)
{
#define NO_GLITCH_OPTION (4)    // 1:Testlink hdvenc no glitch; 2:808 from Xingdi; 3/4:Os & Tmds from Lin Jiangfeng
#define ANA_PRINT_CLK_EN (0)
	mt_u32 ret = BB_SUCCESS;
	#if ANA_PRINT_CLK_EN
	mt_u32 regv0, regv2;
	#endif
	mt_u32 regv1;

	#if (NO_GLITCH_OPTION == 1)
	ret |= mt_sys_read_register(SYMPHONY_IO_PA(0xbf50f818), &regv1);
	regv1 &= ~(1 << 6);
	regv1 |= (en << 6);
	ret |= mt_sys_write_register(SYMPHONY_IO_PA(0xbf50f818), regv1);
	#elif (NO_GLITCH_OPTION == 2)
	ret |= mt_sys_read_register(SYMPHONY_IO_PA(0xbf50f808), &regv1);
	#if ANA_PRINT_CLK_EN
	regv0 = regv1;
	#endif
	regv1 &= ~(1 << 26);
	regv1 |= ((1 - en) << 26);
	ret |= mt_sys_write_register(SYMPHONY_IO_PA(0xbf50f808), regv1);
	#if ANA_PRINT_CLK_EN
	ret |= mt_sys_read_register(SYMPHONY_IO_PA(0xbf50f808), &regv2);
	printf("\n[%s_%d] expect reg[0x%x]=before write:0x%x, expect:%x, after write:%x,%x\n", __func__, __LINE__, 0xbf50f808, regv0, regv1, regv2, 1 - en);
	#endif
	SiiLibTimeMilliDelay(1);
	#elif (NO_GLITCH_OPTION == 3)
	if (en == 0) {
		SiiLibTimeMilliDelay(5);
	}
	ret |= mt_sys_read_register(SYMPHONY_IO_PA(0xbf5d0060), &regv1);
	#if ANA_PRINT_CLK_EN
	regv0 = regv1;
	#endif
	regv1 &= ~(1 << 7);
	regv1 |= (en << 7);
	ret |= mt_sys_write_register(SYMPHONY_IO_PA(0xbf5d0060), regv1);
	#if ANA_PRINT_CLK_EN
	ret |= mt_sys_read_register(SYMPHONY_IO_PA(0xbf5d0060), &regv2);
	printf("\n[%s_%d] expect reg[0x%x]=before write:0x%x, expect:%x, after write:%x,%x\n", __func__, __LINE__, 0xbf5d0060, regv0, regv1, regv2, en);
	#endif
	#elif (NO_GLITCH_OPTION == 4)
	if (en == 0) {
		SiiLibTimeMilliDelay(1);
	}
	ret |= mt_sys_read_register(SYMPHONY_IO_PA(0xbf5d0060), &regv1);
	#if ANA_PRINT_CLK_EN
	regv0 = regv1;
	#endif
	regv1 &= ~(1 << 7);
	regv1 |= (en << 7);
	ret |= mt_sys_write_register(SYMPHONY_IO_PA(0xbf5d0060), regv1);
	#if ANA_PRINT_CLK_EN
	ret |= mt_sys_read_register(SYMPHONY_IO_PA(0xbf5d0060), &regv2);
	printf("\n[%s_%d] expect reg[0x%x]=before write:0x%x, expect:%x, after write:%x,%x\n", __func__, __LINE__, 0xbf5d0060, regv0, regv1, regv2, en);
	#endif
	ret |= mt_sys_read_register(SYMPHONY_IO_PA(0xbf157000), &regv1);
	#if ANA_PRINT_CLK_EN
	regv0 = regv1;
	#endif
	regv1 &= ~(1 << 7);
	regv1 |= (en << 7);
	ret |= mt_sys_write_register(SYMPHONY_IO_PA(0xbf157000), regv1);
	#if ANA_PRINT_CLK_EN
	ret |= mt_sys_read_register(SYMPHONY_IO_PA(0xbf157000), &regv2);
	printf("\n[%s_%d] expect reg[0x%x]=before write:0x%x, expect:%x, after write:%x,%x\n", __func__, __LINE__, 0xbf157000, regv0, regv1, regv2, en);
	#endif
	#else
	#endif
	if (ret ) {
		return;
	}
}
#endif

#if 0
static mt_void hdmi_set_sink_scr(mt_u8 on)
{
	mt_u32 ret = BB_SUCCESS;
	ret = misc_reg_put8(REG_ADDR__SCDC_CTL, 0xff, 0x01);
	ret = misc_reg_put8(REG_ADDR__DDC_CMD, 0xff, 0x09);	//clear fifo
	ret = misc_reg_put8(REG_ADDR__DDC_ADDR, 0xff, 0xa8);	//reg_ddc_addr
	ret = misc_reg_put8(REG_ADDR__DDC_SEGM, 0xff, 0x00);	//reg_ddc_segment
	ret = misc_reg_put8(REG_ADDR__DDC_OFFSET, 0xff, 0x20);	//reg_ddc_offset
	ret = misc_reg_put8(REG_ADDR__DDC_DIN_CNT2, 0xff, 0x00);	//ddc_din_cnt[9:8]
	ret = misc_reg_put8(REG_ADDR__DDC_DIN_CNT1, 0xff, 0x01);	//ddc_din_cnt[7:0]
	if (on) {
		ret = misc_reg_put8(REG_ADDR__DDC_DATA, 0xff, 0x03);    //ffc fifo register
	} else {
		ret = misc_reg_put8(REG_ADDR__DDC_DATA, 0xff, 0x00);    //ffc fifo register
	}
	ret = misc_reg_put8(REG_ADDR__DDC_CMD, 0xff, 0x06);	//sequential write ignoring ACK on last byte
}

static mt_void hdmi_set_source_scr(mt_u8 on, mt_u8 hdmi_mode)
{
	mt_u32 ret = BB_SUCCESS;
	mt_u8 val = 0;
	mt_u32 reg_addr = REG_ADDR__SCRCTL;

	//ret = misc_reg_read8(reg_addr, &val, 1);
	val &= ~((1 << 5) | (1 << 0));
	if (on) {
		val |= 0x1;
	}
	if (1 == hdmi_mode) {
		//hdmi2 stream
		val |= (1 << 5);
	}
	ret = misc_reg_write8(reg_addr, &val, 1);
}
#endif

static mt_void hdmi_set_avmute(mt_u8 on)
{
	mt_u32 ret = BB_SUCCESS;
	ret = misc_reg_put8(REG_ADDR__TPI_SC, 1 << 3, on << 3);
	if (ret ) {
		return;
	}
}

static mt_u32 hdmi_get_avmute(mt_void)
{
	mt_u8 ret = 0;
	ret = misc_reg_read8_single(REG_ADDR__TPI_SC);
	return (ret & (0x1 << 3));
}

mt_u32 g_ae_test_clock_en = 0;
mt_void set_ae_test_en(mt_void)
{
	char key;
	printf("\nSet whether to do test clock step by step, '0':NO, '1':YES\n");
	key = (char)_getch();
	g_ae_test_clock_en =  (mt_u32)(key - '0');
	printf("\nYou selected %s to do test clock step by step\n", g_ae_test_clock_en ? "YES" : "NO");
}

#define AE_TEST_CLK(en) {\
		if (en) { \
			do { \
				printf("\nPress 'g' to continue:\n"); \
			} while ( 'g' != (char)_getch()); \
		}\
	}
#define AE_TEST_STEP_PRINT(en) {\
		if (en) { \
			printf("test_step:%d\n", test_step++); \
		}\
	}

#if (defined(CONFIG_MT_CHIP_SYMPHONY6))
//+++++++++++++++++++++++for sym6 chip
MT_BOOL is_pd_pi(void);
MT_BOOL is_ssc(void);
static void sym6_hdmi_analog_cfg(U32 clk, MT_BOOL is_ssc);
void sym6_hdmi_analog_cfg_default(U32 clk, MT_BOOL is_ssc);

#define ChipRegRd misc_reg_read32_single
#define R_VOUT_CLKSEL_REG  0xBF50A604
#define R_HDMI_TX_REG0 0xbf5d01bc
#define R_HDMI_TX_REG1 0xbf5d01c0
#define R_HDMI_TX_REG2 0xbf5d01c4
#define R_HDMI_TEST_REG 0xbf5d01c8
#define REG_CLK_ANALOG  0xbf157000

#define R_CLKGEN_VHD_REG   0xBF5D0058
#define R_CLKGEN_VHDINTP    0xBF5D005C
#define R_CLKGEN_VSDPLL    0xBF5D0060
#define R_CLKGEN_VSDINTP    0xBF5D0064
#define R_CLKGEN_PDSYS_REG    0xBF5D009C
#define R_CLKGEN_USBACLK_REG0 0xBF5D0120
#define R_CLKGEN_VHDSSC_REG 0xbf5d0140
#define R_CLKGEN_VSDSSC_REG 0xbf5d0144

typedef enum CLK_NUM_S {
	CLK_27M = 27000000,
	CLK_27M_1_25 = 27000000 * 5 / 4,
	CLK_27M_1_5 = 27000000 * 3 / 2,
	CLK_54M = 54000000,
	CLK_54M_1_25 = 54000000 * 5 / 4,
	CLK_54M_1_5 = 54000000 * 3 / 2,
	CLK_108M = 108000000,
	CLK_74_25M = 74250000,
	CLK_74_25M_1_25 = 74250000 * 5 / 4,
	CLK_74_25M_1_5 = 74250000 * 3 / 2,
	CLK_148M = 148500000,
	CLK_148M_1_25 = 148500000 * 5 / 4,
	CLK_148M_1_5 = 148500000 * 3 / 2,
	CLK_297M = 297000000,
	CLK_297M_1_25 = 297000000 * 5 / 4,
	CLK_297M_1_5 = 297000000 * 3 / 2,
	CLK_297M_O5 = 300000000,
	CLK_297M_1_25_O5 = 300000000 * 5 / 4,
	CLK_297M_1_5_O5 = 300000000 * 3 / 2,
	CLK_594M = 594000000
} CLK_NUM_T;

//according to symphony6 HDMI 8bit_10bit_12bit clock config(20230718).xlxs,  jqw@20230808
#define SIZE_NOSSC_VSD 14
static U32 clk_table_nossc_vsd[][8] = {
	//clk, pd_pi, clkgen_pdsys, clkgen_vhdintp, clkgen_vsd, clkgen_vsd, clkgen_unbaclk, clkgen_pdsys
	//108M
	{CLK_108M,		0, 0x08 << 16, 0x1c << 1, 0x1c0dc2fa, 0x1c0d82fa, 0x3 << 8, (0x00 << 16) | (0x0 << 11)},
	//54M*1.25
	{CLK_54M_1_25,	0, 0x2f << 16, 0x1c << 1, 0x5c3af0fa, 0x5c3ab0fa, 0x1 << 8, (0x27 << 16) | (0x0 << 11)},
	//54M*1.5
	{CLK_54M_1_5,	0, 0x3f << 16, 0x1c << 1, 0x5c16d0fa, 0x5c1690fa, 0x1 << 8, (0x37 << 16) | (0x0 << 11)},
	//54M
	{CLK_54M,		0, 0x08 << 16, 0x1c << 1, 0x1c0dc2fa, 0x1c0d82fa, 0x1 << 8, (0x00 << 16) | (0x0 << 11)},
	//27M
	{CLK_27M,		0, 0x08 << 16, 0x1e << 1, 0x1c0dc2fa, 0x1c0d82fa, 0x0 << 8, (0x00 << 16) | (0x0 << 11)},
	//27M*1.25
	{CLK_27M_1_25,	0, 0x2f << 16, 0x1e << 1, 0x5c3af0fa, 0x5c3ab0fa, 0x0 << 8, (0x27 << 16) | (0x0 << 11)},
	//27M*1.5
	{CLK_27M_1_5,	0, 0x3f << 16, 0x1e << 1, 0x5c16d0fa, 0x5c1690fa, 0x0 << 8, (0x37 << 16) | (0x0 << 11)},
	//108M
	{CLK_108M, 		1, 0x08 << 16, 0x1c << 1, 0x1c0dc2fa, 0x1c0d82fa, 0x3 << 8, (0x00 << 16) | (0x0 << 11)},
	//54M*1.25
	{CLK_54M_1_25,	1, 0x2f << 16, 0x1c << 1, 0x5c94f08a, 0x5c94b08a, 0x1 << 8, (0x27 << 16) | (0x0 << 11)},
	//54M*1.5
	{CLK_54M_1_5,	1, 0x3f << 16, 0x1c << 1, 0x5cb8d08a, 0x5cb8908a, 0x1 << 8, (0x37 << 16) | (0x0 << 11)},
	//54M
	{CLK_54M,		1, 0x08 << 16, 0x1c << 1, 0x1c0dc2ca, 0x1c0d82ca, 0x1 << 8, (0x00 << 16) | (0x0 << 11)},
	//27M
	{CLK_27M,		1, 0x08 << 16, 0x1e << 1, 0x1c0dc2ca, 0x1c0d82ca, 0x0 << 8, (0x00 << 16) | (0x0 << 11)},
	//27M*1.25
	{CLK_27M_1_25,	1, 0x2f << 16, 0x1e << 1, 0x5c94f08a, 0x5c94b08a, 0x0 << 8, (0x27 << 16) | (0x0 << 11)},
	//27M*1.5
	{CLK_27M_1_5,	1, 0x3f << 16, 0x1e << 1, 0x5cb8d08a, 0x5cb8908a, 0x0 << 8, (0x37 << 16) | (0x0 << 11)},

};

#define SIZE_NOSSC_VHD 16
static U32 clk_table_nossc_vhd[][8] = {
	//clk, pd_pi, clkgen_pdsys, clkgen_vhdintp, clkgen_vsd, clkgen_vsd, clkgen_unbaclk, clkgen_pdsys
	//594M
	{CLK_594M,			0, 0x08 << 8, 0x13 << 1, 0x1c01c0fa, 0x1c0180fa, 0x5 << 8, (0x0 << 19) | (0x00 << 8)},
	//297M*1.25
	{CLK_297M_1_25,		0, 0x2c << 8, 0x12 << 1, 0x0c85b0f2, 0x0c85b0f2, 0x4 << 8, (0x0 << 19) | (0x24 << 8)},
	//297M*1.5
	{CLK_297M_1_5,		0, 0x3f << 8, 0x12 << 1, 0xcc43f0f9, 0xcc43b0f9, 0x5 << 8, (0x0 << 19) | (0x37 << 8)},
	//297M
	{CLK_297M,			0, 0x08 << 8, 0x12 << 1, 0x1c01c0fa, 0x1c0180fa, 0x4 << 8, (0x0 << 19) | (0x00 << 8)},
	//297M*1.25
	{CLK_297M_1_25_O5, 	0, 0x2c << 8, 0x13 << 1, 0x0c85b0f2, 0x0c85b0f2, 0x4 << 8, (0x0 << 19) | (0x24 << 8)},
	//297M*1.5
	{CLK_297M_1_5_O5,	0, 0x1c << 8, 0x13 << 1, 0xcc43f0f9, 0xcc43b0f9, 0x5 << 8, (0x0 << 19) | (0x14 << 8)},
	//297M
	{CLK_297M_O5,		0, 0x08 << 8, 0x13 << 1, 0x1c01c0fa, 0x1c0180fa, 0x4 << 8, (0x0 << 19) | (0x00 << 8)},
	//148.5M
	{CLK_148M,			0, 0x08 << 8, 0x11 << 1, 0x1c01c0fa, 0x1c0180fa, 0x0 << 8, (0x0 << 19) | (0x00 << 8)},
	#if 0
	//297M
	{CLK_297M,			0, 0x08 << 8, 0x11 << 1, 0x1c01c0fa, 0x1c0180fa, 0x4 << 8, (0x0 << 19) | (0x00 << 8)},
	//148.5M
	{CLK_148M,			0, 0x08 << 8, 0x12 << 1, 0x1c01c0fa, 0x1c0180fa, 0x0 << 8, (0x0 << 19) | (0x00 << 8)},
	#endif
	//148.5M*1.25
	{CLK_148M_1_25,		0, 0x2c << 8, 0x11 << 1, 0x0c85b0f2, 0x0c85b0f2, 0x0 << 8, (0x0 << 19) | (0x24 << 8)},
	//148.5M*1.5
	{CLK_148M_1_5,		0, 0x3f << 8, 0x11 << 1, 0xcc43f0f9, 0xcc43b0f9, 0x0 << 8, (0x0 << 19) | (0x37 << 8)},
	//148.5M
	{CLK_148M, 			1, 0x08 << 8, 0x11 << 1, 0x1c01c0ca, 0x1c0180ca, 0x0 << 8, (0x0 << 19) | (0x00 << 8)},
	//148.5M*1.25
	{CLK_148M_1_25, 		1, 0x2f << 8, 0x11 << 1, 0xcc85f08a, 0xcc85b08a, 0x0 << 8, (0x0 << 19) | (0x27 << 8)},
	//148.5M*1.5
	{CLK_148M_1_5, 		1, 0x3f << 8, 0x11 << 1, 0xcca6d089, 0xcca69089, 0x0 << 8, (0x0 << 19) | (0x37 << 8)},
	//74.25M
	{CLK_74_25M,		0, 0x08 << 8, 0x08 << 1, 0x1c01c0fa, 0x1c0180fa, 0x0 << 8, (0x0 << 19) | (0x00 << 8)},
	//74.25M*1.25
	{CLK_74_25M_1_25,	0, 0x2c << 8, 0x08 << 1, 0x0c85b0f2, 0x0c85b0f2, 0x0 << 8, (0x0 << 19) | (0x24 << 8)},
	//74.25M*1.5
	{CLK_74_25M_1_5,	0, 0x3f << 8, 0x08 << 1, 0xcc43f0f9, 0xcc43b0f9, 0x0 << 8, (0x0 << 19) | (0x37 << 8)},
	//74.25M
	{CLK_74_25M, 		1, 0x08 << 8, 0x08 << 1, 0x1c01c0ca, 0x1c0180ca, 0x0 << 8, (0x0 << 19) | (0x00 << 8)},
	//74.25M*1.25
	{CLK_74_25M_1_25, 	1, 0x2f << 8, 0x08 << 1, 0xcc85f08a, 0xcc85b08a, 0x0 << 8, (0x0 << 19) | (0x27 << 8)},
	//74.25M*1.5
	{CLK_74_25M_1_5, 	1, 0x3f << 8, 0x08 << 1, 0xcca6d089, 0xcca69089, 0x0 << 8, (0x0 << 19) | (0x37 << 8)},
};


#define SIZE_SSC_VSD 14
static U32 clk_table_ssc_vsd[][12] = {
	//clk, pd_pi, clkgen_pdsys, clkgen_vsdssc, clkgen_vsdssc, clkgen_vhdintp, clkgen_usbaclk, clkgen_vsdpll, clkgen_vsdpll, clkgen_vsd, clkgen_vhdssc, clkgen_pdsys
	//108M
	{CLK_108M,		0, 0x08 << 16, 0x4d, 0x0d, 0x1c << 1, 0x3 << 8, 0x1c28e04a, 0x1c28a04a, 0x1c28a05a, 0x3ccebc72, (0x00 << 16) | (0x0 << 11)},
	//54M*1.25
	{CLK_54M_1_25,	0, 0x2f << 16, 0x4e, 0x0e, 0x1c << 1, 0x1 << 8, 0x5c3af04a, 0x5c3ab04a, 0x5c3ab05a, 0x1fe1d571, (0x27 << 16) | (0x0 << 11)},
	//54M*1.5
	{CLK_54M_1_5,	0, 0x3f << 16, 0x4e, 0x0e, 0x1c << 1, 0x1 << 8, 0x5c4cf04a, 0x5c4cb04a, 0x5c4cb05a, 0x1fe1d271, (0x37 << 16) | (0x0 << 11)},
	//54M
	{CLK_54M,		0, 0x08 << 16, 0x4d, 0x0d, 0x1c << 1, 0x1 << 8, 0x1c28e04a, 0x1c28a04a, 0x1c28a05a, 0x32cebc72, (0x00 << 16) | (0x0 << 11)},
	//27M
	{CLK_27M,		0, 0x08 << 16, 0x4d, 0x0d, 0x1e << 1, 0x0 << 8, 0x1c28e04a, 0x1c28a04a, 0x1c28a05a, 0x32cebc72, (0x00 << 16) | (0x0 << 11)},
	//27M*1.25
	{CLK_27M_1_25,	0, 0x2f << 16, 0x4e, 0x0e, 0x1e << 1, 0x0 << 8, 0x5c3af04a, 0x5c3ab04a, 0x5c3ab05a, 0x1fe1d571, (0x27 << 16) | (0x0 << 11)},
	//27M*1.5
	{CLK_27M_1_5,	0, 0x3f << 16, 0x4e, 0x0e, 0x1e << 1, 0x0 << 8, 0x5c4cf04a, 0x5c4cb04a, 0x5c4cb05a, 0x1fe1d271, (0x37 << 16) | (0x0 << 11)},
	//108M
	{CLK_108M, 		1, 0x08 << 16, 0x4d, 0x0d, 0x1c << 1, 0x3 << 8, 0x1c28e04a, 0x1c28a04a, 0x1c28a04a, 0x32cebc72, (0x00 << 16) | (0x0 << 11)},
	//54M*1.25
	{CLK_54M_1_25,	1, 0x2f << 16, 0x4e, 0x0e, 0x1c << 1, 0x1 << 8, 0x5c3af04a, 0x5c3ab04a, 0x5c3ab04a, 0x1fe1d571, (0x27 << 16) | (0x0 << 11)},
	//54M*1.5
	{CLK_54M_1_5,	1, 0x3f << 16, 0x4e, 0x0e, 0x1c << 1, 0x1 << 8, 0x5c4cf04a, 0x5c4cb04a, 0x5c4cb04a, 0x1fe1d271, (0x37 << 16) | (0x0 << 11)},
	//54M
	{CLK_54M,		1, 0x08 << 16, 0x4d, 0x0d, 0x1c << 1, 0x1 << 8, 0x1c28e04a, 0x1c28a04a, 0x1c28a04a, 0x32cebc72, (0x00 << 16) | (0x0 << 11)},
	//27M
	{CLK_27M,		1, 0x08 << 16, 0x4d, 0x0d, 0x1e << 1, 0x0 << 8, 0x1c28e04a, 0x1c28a04a, 0x1c28a04a, 0x32cebc72, (0x00 << 16) | (0x0 << 11)},
	//27M*1.25
	{CLK_27M_1_25,	1, 0x2f << 16, 0x4e, 0x0e, 0x1e << 1, 0x0 << 8, 0x5c3af04a, 0x5c4cb04a, 0x5c4cb04a, 0x1fe1d571, (0x27 << 16) | (0x0 << 11)},
	//27M*1.5
	{CLK_27M_1_5,	1, 0x3f << 16, 0x4e, 0x0e, 0x1e << 1, 0x0 << 8, 0x5c4cf04a, 0x5c4cb04a, 0x5c4cb04a, 0x1fe1d271, (0x37 << 16) | (0x0 << 11)},

};

#define SIZE_SSC_VHD 14
static U32 clk_table_ssc_vhd[][12] = {
	//clk, pd_pi, clkgen_pdsys, clkgen_vsdssc, clkgen_vsdssc, clkgen_vhdintp, clkgen_usbaclk, clkgen_vsdpll, clkgen_vsdpll, clkgen_vsd, clkgen_vhdssc, clkgen_pdsys
	//594M
	{CLK_594M,			0, 0x08 << 8, 0x49, 0x09, 0x13 << 1, 0x5 << 8, 0x1c22e04a, 0x1c22a04a, 0x1c22a05a, 0x15ebee73, (0x0 << 19) | (0x00 << 8)},
	//297M
	{CLK_297M,			0, 0x08 << 8, 0x49, 0x09, 0x12 << 1, 0x4 << 8, 0x1c22e04a, 0x1c22a04a, 0x1c22a05a, 0x15ebee73, (0x0 << 19) | (0x00 << 8)},
	//148.5M
	{CLK_148M,			0, 0x08 << 8, 0x49, 0x09, 0x11 << 1, 0x0 << 8, 0x1c22e04a, 0x1c22a04a, 0x1c22a05a, 0x15ebee73, (0x0 << 19) | (0x00 << 8)},
	//148.5M*1.25
	{CLK_148M_1_25,		0, 0x2c << 8, 0x49, 0x09, 0x11 << 1, 0x0 << 8, 0x0c85b042, 0x0c85b042, 0x0c85b052, 0x11efb276, (0x0 << 19) | (0x24 << 8)},
	//148.5M*1.5
	{CLK_148M_1_5,		0, 0x3f << 8, 0x4a, 0x0a, 0x11 << 1, 0x0 << 8, 0xcc43f049, 0xcc43b049, 0xcc43b059, 0x11efbe76, (0x0 << 19) | (0x37 << 8)},
	//148.5M
	{CLK_148M, 			1, 0x08 << 8, 0x49, 0x09, 0x11 << 1, 0x0 << 8, 0x1c22e04a, 0x1c22a04a, 0x1c22a04a, 0x15ebee73, (0x0 << 19) | (0x00 << 8)},
	//148.5M*1.25
	{CLK_148M_1_25, 		1, 0x2f << 8, 0x4a, 0x0a, 0x11 << 1, 0x0 << 8, 0xcc85f08a, 0xcc85b08a, 0xcc85b08a, 0x11efb276, (0x0 << 19) | (0x27 << 8)},
	//148.5M*1.5
	{CLK_148M_1_5, 		1, 0x3f << 8, 0x4a, 0x0a, 0x11 << 1, 0x0 << 8, 0xcc43f049, 0xcc43b049, 0xcc43b049, 0x11efbe76, (0x0 << 19) | (0x37 << 8)},
	//74.25M
	{CLK_74_25M,		0, 0x08 << 8, 0x49, 0x09, 0x08 << 1, 0x0 << 8, 0x1c22e04a, 0x1c22a04a, 0x1c22a05a, 0x14ecf673, (0x0 << 19) | (0x00 << 8)},
	//74.25M*1.25
	{CLK_74_25M_1_25,	0, 0x2c << 8, 0x49, 0x09, 0x08 << 1, 0x0 << 8, 0x0c85b042, 0x0c85b042, 0x0c85b052, 0x22deb276, (0x0 << 19) | (0x24 << 8)},
	//74.25M*1.5
	{CLK_74_25M_1_5,	0, 0x3f << 8, 0x4a, 0x0a, 0x08 << 1, 0x0 << 8, 0xcc43f049, 0xcc43b049, 0xcc43b059, 0x22debe76, (0x0 << 19) | (0x37 << 8)},
	//74.25M
	{CLK_74_25M, 		1, 0x08 << 8, 0x49, 0x09, 0x08 << 1, 0x0 << 8, 0x1c22e04a, 0x1c22a04a, 0x1c22a04a, 0x14ecf673, (0x0 << 19) | (0x00 << 8)},
	//74.25M*1.25
	{CLK_74_25M_1_25, 	1, 0x2f << 8, 0x4a, 0x0a, 0x08 << 1, 0x0 << 8, 0xcc85f08a, 0xcc85b08a, 0xcc85b08a, 0x22deb276, (0x0 << 19) | (0x27 << 8)},
	//74.25M*1.5
	{CLK_74_25M_1_5, 	1, 0x3f << 8, 0x4a, 0x0a, 0x08 << 1, 0x0 << 8, 0xcc43f049, 0xcc43b049, 0xcc43b049, 0x22debe76, (0x0 << 19) | (0x37 << 8)},
};

static MT_BOOL is_clk_vsd(U32 clk)
{
	if (clk == CLK_108M ||  clk == CLK_54M_1_25 || clk == CLK_54M_1_5 || clk == CLK_54M
			|| clk == CLK_27M || clk == CLK_27M_1_25 || clk == CLK_27M_1_5) {
		return TRUE;
	} else {
		return FALSE;
	}
}
static U32 get_config_nossc(U32 clk, MT_BOOL is_pd_pi, U32 id)
{
	int i;
	if (is_clk_vsd(clk) == TRUE) {
		for (i = 0; i < SIZE_NOSSC_VSD; i++)
			if ((clk == clk_table_nossc_vsd[i][0]) && (is_pd_pi == clk_table_nossc_vsd[i][1])) {
				return clk_table_nossc_vsd[i][id];
			}
	} else {
		for (i = 0; i < SIZE_NOSSC_VHD; i++)
			if ((clk == clk_table_nossc_vhd[i][0]) && (is_pd_pi == clk_table_nossc_vhd[i][1])) {
				return clk_table_nossc_vhd[i][id];
			}
	}
	printf("\n no config for clk :%d", clk);
	return 0xffffffff;
}

static U32 get_config_ssc(U32 clk, MT_BOOL is_pd_pi, U32 id)
{
	int i;
	if (is_clk_vsd(clk) == TRUE) {
		for (i = 0; i < SIZE_SSC_VSD; i++)
			if ((clk == clk_table_ssc_vsd[i][0]) && (is_pd_pi == clk_table_ssc_vsd[i][1])) {
				return clk_table_ssc_vsd[i][id];
			}
	} else {
		for (i = 0; i < SIZE_SSC_VHD; i++)
			if ((clk == clk_table_ssc_vhd[i][0]) && (is_pd_pi == clk_table_ssc_vhd[i][1])) {
				return clk_table_ssc_vhd[i][id];
			}
	}
	printf("\n no config for clk :%d", clk);
	return 0xffffffff;
}

static void hdmi_clk_nossc_gateclk(U32 clk, MT_BOOL is_pd_pi)
{
	//U32 reg;
	//U32 val;
	if (clk != 27000000 && clk != 27000000 * 5 / 4 && clk != 27000000 * 3 / 2
			&& clk != 148500000 && clk != 148500000 * 5 / 4 && clk != 148500000 * 3 / 2
			&& clk != 74250000 && clk != 74250000 * 5 / 4 && clk != 74250000 * 3 / 2
			&& clk != 297000000 && clk != 297000000 * 5 / 4 && clk != 297000000 * 3 / 2
			&& clk != 594000000
			&& clk != 54000000 && clk != 54000000 * 5 / 4 && clk != 54000000 * 3 / 2
			&& clk != 108000000 && clk != CLK_297M_O5 && clk != CLK_297M_1_25_O5 && clk != CLK_297M_1_5_O5) {
		printf("\n invalid clk :%d", clk);
		return;
	}
	if (clk <= 340000000) {
		misc_reg_put32 (R_HDMI_TEST_REG, (1 << 6), (0 << 6));
	} else {
		misc_reg_put32 (R_HDMI_TEST_REG, (1 << 6), (1 << 6));
	}
	printf("\n 0xbf5d01c8:0x%08x", ChipRegRd(0xbf5d01c8));

	//step1:gate video clk
	misc_reg_put32 (R_CLKGEN_PDSYS_REG, (1 << 19) | (1 << 11), (1 << 19) | (1 << 11));
	//step2:set video clk
	if (is_clk_vsd(clk) == TRUE) {
		misc_reg_put32 (R_CLKGEN_PDSYS_REG, 0xff << 16, get_config_nossc(clk, is_pd_pi, 2));
	} else {
		misc_reg_put32 (R_CLKGEN_PDSYS_REG, 0xff << 8, get_config_nossc(clk, is_pd_pi, 2));
	}
	SiiLibTimeMilliDelay(1);
	//step3:sel hdmi clk
	misc_reg_put32 (R_CLKGEN_VHDINTP, 0x1f << 1, get_config_nossc(clk, is_pd_pi, 3));
	//step4:reset pll
	if (is_clk_vsd(clk) == TRUE) {
		misc_reg_put32 (R_CLKGEN_VSDPLL, 0xffffffff, get_config_nossc(clk, is_pd_pi, 4));
	} else {
		misc_reg_put32 (R_CLKGEN_VHD_REG, 0xffffffff, get_config_nossc(clk, is_pd_pi, 4));
	}
	//step5:release pll
	if (is_clk_vsd(clk) == TRUE) {
		misc_reg_put32 (R_CLKGEN_VSDPLL, 0xffffffff, get_config_nossc(clk, is_pd_pi, 5));
	} else {
		misc_reg_put32 (R_CLKGEN_VHD_REG, 0xffffffff, get_config_nossc(clk, is_pd_pi, 5));
	}
	SiiLibTimeMilliDelay(4);
	//step5.1:selx2
	misc_reg_put32 (R_CLKGEN_USBACLK_REG0, 0x7 << 8, get_config_nossc(clk, is_pd_pi, 6));
	//step6:turn on video clk
	if (is_clk_vsd(clk) == TRUE) {
		misc_reg_put32 (R_CLKGEN_PDSYS_REG, (0xff << 16) | (1 << 11), get_config_nossc(clk, is_pd_pi, 7));
	} else {
		misc_reg_put32 (R_CLKGEN_PDSYS_REG, (1 << 19) | (0xff << 8), get_config_nossc(clk, is_pd_pi, 7));
	}

}

static void hdmi_clk_ssc_gateclk(U32 clk, MT_BOOL is_pd_pi)
{
	//U32 reg;
	//U32 val;
	if (clk != 27000000 && clk != 27000000 * 5 / 4 && clk != 27000000 * 3 / 2
			&& clk != 148500000 && clk != 148500000 * 5 / 4 && clk != 148500000 * 3 / 2
			&& clk != 74250000 && clk != 74250000 * 5 / 4 && clk != 74250000 * 3 / 2
			&& clk != 297000000 && clk != 297000000 * 5 / 4 && clk != 297000000 * 3 / 2
			&& clk != 594000000
			&& clk != 54000000 && clk != 54000000 * 5 / 4 && clk != 54000000 * 3 / 2
			&& clk != 108000000 && clk != CLK_297M_O5 && clk != CLK_297M_1_25_O5 && clk != CLK_297M_1_5_O5) {
		printf("\n invalid clk :%d", clk);
		return;
	}

	if (clk <= 340000000) {
		misc_reg_put32 (R_HDMI_TEST_REG, (1 << 6), (0 << 6));
	} else {
		misc_reg_put32 (R_HDMI_TEST_REG, (1 << 6), (1 << 6));
	}
	printf("\n 0xbf5d01c8:0x%08x", ChipRegRd(0xbf5d01c8));
	//step1:gate video clk
	misc_reg_put32 (R_CLKGEN_PDSYS_REG, (1 << 19) | (1 << 11), (1 << 19) | (1 << 11));
	//step2:set video clk
	if (is_clk_vsd(clk) == TRUE) {
		misc_reg_put32 (R_CLKGEN_PDSYS_REG, 0xff << 16, get_config_ssc(clk, is_pd_pi, 2));
	} else {
		misc_reg_put32 (R_CLKGEN_PDSYS_REG, 0xff << 8, get_config_ssc(clk, is_pd_pi, 2));
	}
	//step3:reset ssc
	misc_reg_put32 (R_CLKGEN_VSDSSC_REG, 0xff, get_config_ssc(clk, is_pd_pi, 3));
	//step4:release ssc
	misc_reg_put32 (R_CLKGEN_VSDSSC_REG, 0xff, get_config_ssc(clk, is_pd_pi, 4));
	//step5:sel hdmi clk
	misc_reg_put32 (R_CLKGEN_VHDINTP, 0x1f << 1, get_config_ssc(clk, is_pd_pi, 5));
	//step5.1:selx2
	misc_reg_put32 (R_CLKGEN_USBACLK_REG0, 0x7 << 8, get_config_ssc(clk, is_pd_pi, 6));
	//step6:reset pll
	if (is_clk_vsd(clk) == TRUE) {
		misc_reg_put32 (R_CLKGEN_VSDPLL, 0xffffffff, get_config_ssc(clk, is_pd_pi, 7));
	} else {
		misc_reg_put32 (R_CLKGEN_VHD_REG, 0xffffffff, get_config_ssc(clk, is_pd_pi, 7));
	}
	//step7:release pll
	if (is_clk_vsd(clk) == TRUE) {
		misc_reg_put32 (R_CLKGEN_VSDPLL, 0xffffffff, get_config_ssc(clk, is_pd_pi, 8));
	} else {
		misc_reg_put32 (R_CLKGEN_VHD_REG, 0xffffffff, get_config_ssc(clk, is_pd_pi, 8));
	}
	//step8:include pi in pll
	if (is_clk_vsd(clk) == TRUE) {
		misc_reg_put32 (R_CLKGEN_VSDPLL, 0xffffffff, get_config_ssc(clk, is_pd_pi, 9));
	} else {
		misc_reg_put32 (R_CLKGEN_VHD_REG, 0xffffffff, get_config_ssc(clk, is_pd_pi, 9));
	}
	//step9:config ssc
	misc_reg_put32 (R_CLKGEN_VHDSSC_REG, 0xffffffff, get_config_ssc(clk, is_pd_pi, 10));
	//step10:turn on video clk
	misc_reg_put32 (R_CLKGEN_PDSYS_REG, 0xffffffff, get_config_ssc(clk, is_pd_pi, 11));

}

MT_BOOL is_pd_pi(void)
{
	if (((ChipRegRd(R_CLKGEN_VSDINTP) >> 12) & 0x3) == 0x3) {
		return FALSE;
	} else {
		return TRUE;
	}
}

MT_BOOL is_ssc(void)
{
	if (((ChipRegRd(R_CLKGEN_VSDSSC_REG) >> 2) & 0x1) == 0x1) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void sym6_hdmi_analog_init_load_para(U32 clk_tmds)
{
	U32 rega;
	U32 regb;
	U32 maska = 0x033fffff;
	U32 maskb = 0xffffffbf;

	#if 0
	misc_reg_put32 (0xbf5d01bc, 0x0330ffff, 0x130442c);
	misc_reg_put32 (0xbf5d01c0, 0x0330ffff, 0x130442c);
	misc_reg_put32 (R_HDMI_TX_REG2, 0x0330ffff, 0x130442c);
	misc_reg_put32 (R_HDMI_TEST_REG, 0x000001bf, 0x080000f0);
	#else
	switch ( clk_tmds) {
		case CLK_594M:
		case CLK_297M_1_5:
		case CLK_297M_1_25:
			rega = 0x013044ca;
			regb = 0x080000f0;
			break;
		case CLK_297M:
		case CLK_148M_1_25:
		case CLK_148M_1_5:
			rega = 0x031044ca;
			regb = 0x1500019a;
			break;
		case CLK_148M:
		case CLK_74_25M:
		case CLK_74_25M_1_25:
		case CLK_74_25M_1_5:
		case CLK_27M:
		case CLK_27M_1_25:
		case CLK_27M_1_5:
			rega = 0x0200bb08;
			regb = 0x08000108;
			break;
		default:
			rega = 0x0200bb08;
			regb = 0x08000108;
			break;
	}
	misc_reg_put32 (R_HDMI_TX_REG0, maska, rega);
	misc_reg_put32 (R_HDMI_TX_REG1, maska, rega);
	misc_reg_put32 (R_HDMI_TX_REG2, maska, rega);
	misc_reg_put32 (R_HDMI_TEST_REG, maskb, regb);
	#endif
}

void sym6_hdmi_analog_cfg_default(U32 clk, MT_BOOL is_ssc)
{
	//use xtal
	misc_reg_put32 (R_VOUT_CLKSEL_REG, 0x3 << 20, 0x3 << 20);
	//phy mute
	misc_reg_put32 (R_HDMI_TX_REG2, 0xf << 28, 0xf << 28);
	//reset phy tmds clk
	misc_reg_put32 (REG_CLK_ANALOG, 1 << 7, 1 << 7);
	if (is_ssc) {
		hdmi_clk_ssc_gateclk(clk, is_pd_pi());
	} else {
		hdmi_clk_nossc_gateclk(clk, is_pd_pi());
	}
	//release phy tmds clk
	misc_reg_put32 (REG_CLK_ANALOG, 1 << 7, 0);
	//phy unmute
	misc_reg_put32 (R_HDMI_TX_REG2, 0xf << 28, 0); //tmds unmute outside
	//hdvenc_clksel, hdmi_pixnx_clksel, hdmi_tmds_clksel
	if (clk == CLK_297M_O5 || clk == CLK_297M_1_25_O5 || clk == CLK_297M_1_5_O5 ) {
		misc_reg_put32 (R_VOUT_CLKSEL_REG, 0x7 | (0x7 << 12) | (0x7 << 16), 0x4 << 16);
	} else {
		misc_reg_put32 (R_VOUT_CLKSEL_REG, 0x7 | (0x7 << 12) | (0x7 << 16), 0);
	}
	//not use xtal
	misc_reg_put32 (R_VOUT_CLKSEL_REG, 0x3 << 20, 0);
}

static void sym6_hdmi_analog_cfg(U32 clk, MT_BOOL is_ssc)
{
	//use xtal
	misc_reg_put32 (R_VOUT_CLKSEL_REG, 0x3 << 20, 0x3 << 20);
	//phy mute
	//misc_reg_put32 (R_HDMI_TX_REG2, 0xf << 28, 0xf << 28);
	//reset phy tmds clk
	misc_reg_put32 (REG_CLK_ANALOG, 1 << 7, 1 << 7);
	if (is_ssc) {
		hdmi_clk_ssc_gateclk(clk, is_pd_pi());
	} else {
		hdmi_clk_nossc_gateclk(clk, is_pd_pi());
	}
	//release phy tmds clk
	misc_reg_put32 (REG_CLK_ANALOG, 1 << 7, 0);
	//phy unmute
	//misc_reg_put32 (R_HDMI_TX_REG2, 0xf << 28, 0); //tmds unmute outside
	//hdvenc_clksel, hdmi_pixnx_clksel, hdmi_tmds_clksel
	if (clk == CLK_297M_O5 || clk == CLK_297M_1_25_O5 || clk == CLK_297M_1_5_O5 ) {
		misc_reg_put32 (R_VOUT_CLKSEL_REG, 0x7 | (0x7 << 12) | (0x7 << 16), 0x4 << 16);
	} else {
		misc_reg_put32 (R_VOUT_CLKSEL_REG, 0x7 | (0x7 << 12) | (0x7 << 16), 0);
	}
	//not use xtal
	misc_reg_put32 (R_VOUT_CLKSEL_REG, 0x3 << 20, 0);
	sym6_hdmi_analog_init_load_para(clk);
}
//+++++++++++++++++++++++for sym6 chip
#endif

static void hdmi_soft_rst(mt_u32 clk_tmds, mt_u32 rst)
{
	mt_u32 ret = 0;
	ulong reg_addr = HDMI20_REG_BASE_ADDR + 0x10;
	mt_u8 reg_val = 0x0;
	ret = misc_reg_read8(reg_addr, &reg_val, 1);
	if ( rst ) {
		reg_val |= BIT_MSK__PWD_SRST__REG_SW_RST;
		ret = misc_reg_put8(reg_addr, 0xff, reg_val);
	} else {
		SiiLibTimeMilliDelay(1);
		reg_val &= ~BIT_MSK__PWD_SRST__REG_SW_RST;
		ret = misc_reg_put8(reg_addr, 0xff, reg_val);
		SiiLibTimeMilliDelay(2);
	}
	//printf("\n soft reset done!!!\n");
	if (ret) {
		return;
	}
}

static void hdmi_ana_phy_rst(mt_u32 rst)
{
	mt_u32 ret = 0;
	ulong reg_addr = REG_CLK_ANALOG;
	mt_u32 reg_val = 0x0;
	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	printf("\n hdmi_ana_phy_rst 0x%x!!!\n",reg_val);
	if ( rst ) {
		reg_val |= 1<<7;
		printf("\n hdmi_ana_phy_rst0 0x%x!!!\n",reg_val);
		ret = misc_reg_put32(reg_addr, 1<<7, reg_val);
	} else {
		SiiLibTimeMilliDelay(1);
		reg_val &= ~(1<<7);
		printf("\n hdmi_ana_phy_rst1 0x%x!!!\n",reg_val);
		ret = misc_reg_put32(reg_addr, 1<<7, reg_val);
		SiiLibTimeMilliDelay(2);
	}
	//printf("\n soft reset done!!!\n");
	if (ret) {
		return;
	}
}


static void hdmi_aip_rst(mt_u32 rst)
{
	mt_u32 ret = 0;
	ulong reg_addr = HDMI20_REG_BASE_ADDR + 0xA2C;
	mt_u8 reg_val = 0x0;

	if ( rst ) {
		ret = misc_reg_read8(reg_addr, &reg_val, 1);
		reg_val |= BIT_MSK__AIP_RST__REG_RST4AUDIO_FIFO | BIT_MSK__AIP_RST__REG_RST4AUDIO;
	} else {
		SiiLibTimeMilliDelay(1);
		ret = misc_reg_read8(reg_addr, &reg_val, 1);
		reg_val &= ~(BIT_MSK__AIP_RST__REG_RST4AUDIO_FIFO | BIT_MSK__AIP_RST__REG_RST4AUDIO);
	}
	ret = misc_reg_put8(reg_addr, 0xff, reg_val);
	if (ret) {
		return;
	}
}

static mt_void hdmi_rcfg_info(SiiInst_t inst)
{
	#define HDMI20_INFOFRAME_CH_MAX (9)
	mt_u32 ret = BB_SUCCESS;
	mt_u32 j;
	uint8_t infoOn[HDMI20_INFOFRAME_CH_MAX];
	SiiDrvTxInfoframeOnOffGet(inst,infoOn);
	for (j=0;j<HDMI20_INFOFRAME_CH_MAX;j++) {
		ret = misc_reg_put8(REG_ADDR__TPI_INFO_FSEL, 0x0f, (mt_u8)j);
		if ( infoOn[j]) {
			ret = misc_reg_put8(REG_ADDR__TPI_INFO_EN, 0xff, 0xc0);
		} else {
			ret = misc_reg_put8(REG_ADDR__TPI_INFO_EN, BIT_MSK__TPI_INFO_EN__REG_TPI_INFO_RPT,0);
			ret = misc_reg_put8(REG_ADDR__TPI_INFO_EN, BIT_MSK__TPI_INFO_EN__REG_TPI_INFO_EN,0);
		}
	}
	if (ret ) {
		return;
	}
}

static mt_u32 do_case_set_tvsys(mt_u32 clk_tmds, mt_u32 clk_os_dig, mt_u32 pw_on,
								mt_u32 cs, mt_u32 dcc_on, mt_u32 tvsys, mt_u8 scr_on, mt_u32 hdmi_mode, info_struct_t info, mt_u32 mode3d, mt_u32 bitDepth, mt_u32 cm)
{
	mt_u32 test_step = 0;
	mt_u32 av_mute = 0;
	mt_u32 ae_step_test = g_ae_test_clock_en;

	printf("\nYou selected %s to do test clock step by step\n", ae_step_test ? "YES" : "NO");
	AE_TEST_STEP_PRINT(ae_step_test);
	AE_TEST_CLK(ae_step_test);

	if (clk_tmds != 27000 && clk_tmds != 27000 * 5 / 4 && clk_tmds != 27000 * 3 / 2
			&& clk_tmds != 148500 && clk_tmds != 148500 * 5 / 4 && clk_tmds != 148500 * 3 / 2
			&& clk_tmds != 74250 && clk_tmds != 74250 * 5 / 4 && clk_tmds != 74250 * 3 / 2
			&& clk_tmds != 297000 && clk_tmds != 297000 * 5 / 4 && clk_tmds != 297000 * 3 / 2
			&& clk_tmds != 594000
			&& clk_tmds != 54000 && clk_tmds != 54000 * 5 / 4 && clk_tmds != 54000 * 3 / 2
			&& clk_tmds != 108000 && 300000 != clk_tmds) {
		SSHH_PRINTF("Input error clk_tmds:%d\n", clk_tmds);
		return BB_FAILURE;
	}
	if (594000 != clk_os_dig && 297000 != clk_os_dig && 148500 != clk_os_dig &&
			74250 != clk_os_dig && 108000 != clk_os_dig) {
		SSHH_PRINTF("Input error clk_os_dig:%d\n", clk_os_dig);
		return BB_FAILURE;
	}

	av_mute = hdmi_get_avmute();
	//swith tvsys step
	if (0 == pw_on) {
		hdmi_set_avmute(1);
	}

	AE_TEST_CLK(ae_step_test);
	//step 1, config no glitch clock on
	AE_TEST_STEP_PRINT(ae_step_test);
	#if (defined(CONFIG_MT_CHIP_ETUDE2))
	hdmi_set_no_glitch_clock(1);
	hdmi_analog_clk_en(1);
	AE_TEST_CLK(ae_step_test);

	//step 2, set analog clock
	AE_TEST_STEP_PRINT(ae_step_test);
	hdmi_set_alanlog_clock(clk_tmds, clk_os_dig);
	AE_TEST_CLK(ae_step_test);

	//step 3, analog internal resistance calibration
	if (1 == pw_on) {
		hdmi_alanlog_resistance_calibration(clk_tmds, clk_os_dig);
		AE_TEST_CLK(ae_step_test);
	}
	hdmi_analog_clk_en(0);

	//step 4, set digital clock
	AE_TEST_STEP_PRINT(ae_step_test);
	hdmi_set_digital_clock(clk_tmds, clk_os_dig);
	AE_TEST_CLK(ae_step_test);
	//hdmi_set_hdvenc(tvsys);

	//step 5, config no glitch clock off
	AE_TEST_STEP_PRINT(ae_step_test);
	hdmi_set_no_glitch_clock(0);
	AE_TEST_CLK(ae_step_test);
	#elif (defined(CONFIG_MT_CHIP_SYMPHONY6))
	clk_tmds *= 1000;
	if ( bitDepth == 1 ) {
		clk_tmds = clk_tmds * 5 / 4;
	} else if ( bitDepth == 2 ) {
		clk_tmds = clk_tmds * 3 / 2;
	}
	//SSHH_PRINTF("clk_tmds:%d %d\n", clk_tmds,bitDepth);
	sym6_hdmi_analog_cfg(clk_tmds, is_ssc());
	SiiLibTimeMicroDelay(50);
	#endif

	hdmi_soft_rst(clk_tmds, 1);
	hdmi_aip_rst(1);
	//step 6, set kram 0x1fe  to 0x00
	//if(1 == pw_on)
	hdmi_set_kram(0x1fe, 0x00);
	AE_TEST_CLK(ae_step_test);

	//step 7, init hdmi
	AE_TEST_STEP_PRINT(ae_step_test);
	hdmi_init(cs, pw_on);
	AE_TEST_CLK(ae_step_test);

	//4kp@50/60 yuv420 step
	AE_TEST_STEP_PRINT(ae_step_test);
	//if( 1 == pw_on )
	hdmi_send_info(0, info);
	AE_TEST_CLK(ae_step_test);

	//step 8, set hdvenc
	AE_TEST_STEP_PRINT(ae_step_test);
	hdmi_set_hdvenc(tvsys, mode3d);
	AE_TEST_CLK(ae_step_test);
	{
		SiiDrvTxColorInfoCfg_t FmtVidIn = {0};
		switch (cm) {
			case 1:
				FmtVidIn.inputClrConvStd = SII_DRV_CONV_STD__BT_601;
				break;
			case 3:
				FmtVidIn.inputClrConvStd = SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS;
				break;
			case 4:
				FmtVidIn.inputClrConvStd = SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS;
				break;
			default:
				FmtVidIn.inputClrConvStd = SII_DRV_CONV_STD__BT_709;
				break;
		}
		FmtVidIn.inputClrSpc = SII_DRV_CLRSPC__YC444_709;
		FmtVidIn.inputVidDcDepth = SII_DRV_BIT_DEPTH__12_BIT;
		SiiDrvTxColorInfoConfig(getTxInstance(), &FmtVidIn);
	}
	{
		SiiDrvClrSpc_t clrSpc;
		SiiDrvBitDepth_t dc;
		switch ( cs ) {
			case 0:
				clrSpc = SII_DRV_CLRSPC__RGB_FULL;
				break;
			case 1:
				if (cm == 3 || cm == 4) {
					clrSpc = SII_DRV_CLRSPC__YC422_2020;
				} else {
					clrSpc = SII_DRV_CLRSPC__YC422_709;
				}
				break;
			case 2:
				if (cm == 3 || cm == 4) {
					clrSpc = SII_DRV_CLRSPC__YC444_2020;
				} else {
					clrSpc = SII_DRV_CLRSPC__YC444_709;
				}
				break;
			case 3:
				if (cm == 3 || cm == 4) {
					clrSpc = SII_DRV_CLRSPC__YC420_2020;
				} else {
					clrSpc = SII_DRV_CLRSPC__YC420_709;
				}
				break;
			default:
				clrSpc = SII_DRV_CLRSPC__RGB_FULL;
				break;
		}
		SiiDrvTxOutputColorSpaceSet(getTxInstance(), &clrSpc);
		switch ( bitDepth ) {
			case 0:
				dc = SII_DRV_BIT_DEPTH__8_BIT;
				break;
			case 1:
				dc = SII_DRV_BIT_DEPTH__10_BIT;
				break;
			case 2:
				dc = SII_DRV_BIT_DEPTH__12_BIT;
				break;
			default:
				dc = SII_DRV_BIT_DEPTH__8_BIT;
				break;
		}
		SiiDrvTxOutputBitDepthSet(getTxInstance(), dc);
	}
	hdmi_soft_rst(clk_tmds, 0);
	hdmi_rcfg_info(getTxInstance());
	hdmi_aip_rst(0);
	hdmi_ana_phy_rst(1);
	hdmi_ana_phy_rst(0);
	{
		AE_TEST_STEP_PRINT(ae_step_test);
		if (getScdcEnable()) {
			#if 1
			if (scr_on) {
				SiiDrvTxScdcScrambleenable(getTxInstance(), NULL);
				SiiLibTimeMicroDelay(1);
				//SiiLibTimeMilliDelay(100);

			} else {
				SiiDrvTxScdcScrambleDisable(getTxInstance(), NULL);
			}
			#else
			SiiDrvTxScdcScrambleDisable(getTxInstance(), NULL);
			#endif
		}
	}

	//swith tvsys step
	if (0 == pw_on && 0 == av_mute) {
		hdmi_set_avmute(0);
	}

	return BB_SUCCESS;
}

static mt_u32 hdmi_tvsys2vic(mt_u32 tvsys)
{
	mt_u32 vic = 0x0;
	//	mt_u32 ar = 0;//0:16x9, 1:4x3
	//	mt_u32 ret = BB_SUCCESS;
	//	AVINFOSTORAGE_T param;

	//	ret = get_hdmi_storage_params(&param);
	//	ar = param.ar;

	//need to be done...
	switch (tvsys) {
		case 19:
			vic = 17;
			break;
		case 20:
			vic = 2;
			break;
		case 21:
			vic = 21;
			break;
		case 22:
			vic = 6;
			break;
		case 23:
			vic = 18;
			break;
		case 24:
			vic = 3;
			break;
		case 25:
			vic = 22;
			break;
		case 26:
			vic = 7;
			break;
		case 27:
			vic = 104;
			break;
		case 28:
			vic = 106;
			break;
		case 29:
			vic = 105;
			break;
		case 30:
			vic = 107;
			break;
		case 31:
			vic = 103;
			break;
		case 32:
			vic = 68;
			break;
		case 33:
			vic = 69;
			break;
		case 34:
			vic = 73;
			break;
		case 35:
			vic = 74;
			break;
		case 36:
			vic = 75;
			break;
		case 37:
			vic = 76;
			break;
		case 10:
			vic = 31;
			break;
		case 11:
			vic = 16;
			break;
		case 12:
			vic = 19;
			break;
		case 13:
			vic = 4;
			break;
		case 14:
			vic = 20;
			break;
		case 15:
			vic = 5;
			break;
		case 16:
			vic = 32;
			break;
		case 17:
			vic = 34;
			break;
		case 18:
			vic = 33;
			break;
		case 0:
			vic = 99;
			break;
		case 1:
			vic = 101;
			break;
		case 2:
			vic = 100;
			break;
		case 3:
			vic = 102;
			break;
		case 4:
			vic = 98;
			break;
		case 5:
			vic = 94;
			break;
		case 6:
			vic = 96;
			break;
		case 7:
			vic = 95;
			break;
		case 8:
			vic = 97;
			break;
		case 9:
			vic = 93;
			break;
		default:
			vic = 0;
			break;
	}

	return vic;
}

static mt_u32 hdmi_vic2ar(mt_u32 vic)
{
	mt_u32 ar = 0;
	if ((vic >= 65 && vic <= 92) || (vic >= 98 && vic <= 107)) {
		return 0;
	}
	if (vic >= 93 && vic <= 97) {
		return 2;
	}

	switch (vic) {
		case 1:
		case 2:
		case 6:
		case 8:
		case 10:
		case 12:
		case 14:
		case 17:
		case 21:
		case 23:
		case 25:
		case 27:
		case 29:
		case 35:
		case 37:
		case 42:
		case 44:
		case 48:
		case 50:
		case 52:
		case 54:
		case 56:
		case 58:
			ar = 1;
			break;
		case 3:
		case 4:
		case 5:
		case 7:
		case 9:
		case 11:
		case 13:
		case 15:
		case 16:
		case 18:
		case 19:
		case 20:
		case 22:
		case 24:
		case 26:
		case 28:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 36:
		case 38:
		case 39:
		case 40:
		case 41:
		case 43:
		case 45:
		case 46:
		case 47:
		case 49:
		case 51:
		case 53:
		case 55:
		case 57:
		case 59:
		case 60:
		case 61:
		case 62:
		case 63:
		case 64:
			ar = 2;
			break;
		default:
			ar = 0;
			break;
	}

	return ar;
}

static mt_u32 hdmi_tvsys2rep(mt_u32 tvsys)
{
	mt_u32 rep = 0x0;

	//need to be done...
	switch (tvsys) {
		case 21:
		case 22:
		case 25:
		case 26:
			rep = 1;
			break;
		default:
			rep = 0;
			break;
	}

	return rep;
}

static mt_void hdmi_set_avi_infoframe(MtAviInfo_t AvInfo, info_struct_t *info)
{
	mt_u8 cksum = 0;
	mt_u8 i;

	memset(info, 0, sizeof(info_struct_t));
	info->type = 0x82;
	info->version = 0x02;
	info->length = 0x0D;

	info->pb_byte[1] = ((AvInfo.clrSpc & 0x7) << 5) | ((AvInfo.activeFormatInfo & 1) << 4) | ((AvInfo.barInfo & 0x3) << 2) | (AvInfo.scanInfo & 0x3);
	info->pb_byte[2] = ((AvInfo.colorimetry & 0x3) << 6) | ((AvInfo.pictureAR & 0x3) << 4) | (AvInfo.activeAR & 0xf);
	info->pb_byte[3] = ((AvInfo.itContent & 0x1) << 7) | ((AvInfo.extColorimetry & 0x7) << 4) | ((AvInfo.rgbQR & 0x3) << 2) | (AvInfo.scalingInfo & 0x3);
	info->pb_byte[4] =  AvInfo.vic;
	info->pb_byte[5] = ((AvInfo.yccQR & 0x3) << 6) | ((AvInfo.itContentType & 0x3) << 4) | (AvInfo.repetition & 0xf);

	cksum += info->type;
	cksum += info->version;
	cksum += info->length;
	for (i = 1; i < info->length; i++) {
		cksum += info->pb_byte[i];
	}
	cksum = 256 - cksum;
	info->pb_byte[0] = cksum;
}

void test_set_tvsys_cmd(TVSYSSWITCH_T *ptvsyscmd)
{
	char key;
	mt_u32 tvsys = 0;
	mt_u32 pw_on;
	mt_u32 cs;
	mt_u32 mode_3d;
	mt_u32 mode_3d_ext;
	AVINFOSTORAGE_T param;
	mt_u32 bdepth;
	mt_u32 cm;

	(mt_void)get_hdmi_storage_params(&param);
	fflush(stdin);
	fflush(stdout);

SET_TV_SYS:
	printf("\n%s_%d: Input tv format:"\
		   "00: (99)4096x2160@25_256:135\n"\
		   "01: (101)4096x2160@50_256:135\n"\
		   "02: (100)4096x2160@30_256:135\n"\
		   "03: (102)4096x2160@60_256:135\n"\
		   "04: (98)4096x2160@24_256:135\n"\
		   "05: (94)3840x2160@25_16:9\n"\
		   "06: (96)3840x2160@50_16:9\n"\
		   "07: (95)3840x2160@30_16:9\n"\
		   "08: (97)3840x2160@60_16:9\n"\
		   "09: (93)3840x2160@24_16:9\n"\
		   "10: (31)1080p@50_16:9\n"\
		   "11: (16)1080p@60_16:9\n"\
		   "12: (19)720p@50_16:9\n"\
		   "13: (4)720p@60_16:9\n"\
		   "14: (20)1080i@50_16:9\n"\
		   "15: (5)1080i@60_16:9\n"\
		   "16: (32)1080p@24_16:9\n"\
		   "17: (34)1080p@30_16:9\n"\
		   "18: (33)1080p@25_16:9\n"\
		   "19: (17)576p@50_4:3\n"\
		   "20: (2)480p@60_4:3\n"\
		   "21: (21)576i@50_4:3\n"\
		   "22: (6)480i@60_4:3\n"\
		   "23: (18)576p@50_16:9\n"\
		   "24: (3)480p@60_16:9\n"\
		   "25: (22)576i@50_16:9\n"\
		   "26: (7)480i@60_16:9\n"\
		   "27: (104)3840x2160@25_64:27\n"\
		   "28: (106)3840x2160@50_64:27\n"\
		   "29: (105)3840x2160@30_64:27\n"\
		   "30: (107)3840x2160@60_64:27\n"\
		   "31: (103)3840x2160@24_64:27\n"\
		   "32: (68)720p@50_64:27\n"\
		   "33: (69)720p@60_64:27\n"\
		   "34: (73)1080p@25_64:27\n"\
		   "35: (74)1080p@30_64:27\n"\
		   "36: (75)1080p@50_64:27\n"\
		   "37: (76)1080p@60_64:27\n", __func__, __LINE__);
	fflush(stdin);
	key = (char)_getch();
	tvsys += ((mt_u32)key - '0') * 10;
	key = (char)_getch();
	tvsys += ((mt_u32)key - '0') * 1;
	printf("tvsys:%d\n", tvsys);
	if (tvsys > 37) {
		tvsys = 0;
		printf("\nError input params!!! input again...\n");
		fflush(stdin);
		goto SET_TV_SYS;
	}

SET_COLOS_SPACE:
	#if HDMI20_RELEASE_2_QA
	pw_on = 0;
	printf("input color space: 0: RGB, 1: YUV422, 2:YUV444, 3:YUV420\n");
	key = (char)_getch();
	cs = (mt_u32)key - '0';
	cm = cs >> 2;
	if (cm) {
		SET_COLORIMETRY:
		printf("input colorimetry: 1: 601, 0/2: 709, 3:2020 cYCbCr, 4:2020 YCbCr/RGB\n");
		key = (char)_getch();
		cm = (mt_u32)key - '0';
		if (cm > 4 ){
			goto SET_COLORIMETRY;
		}
		cs = cs & 0x3;
	}
	printf("csc:%d\n", cs);
	#else
	printf("input if power on param: 0: tvsys has been set, 1: first set\n");
	key = (char)_getch();
	pw_on = (mt_u32)key - '0';
	printf("power on:%d\n", pw_on);

	printf("input color space: 0: RGB, 1: YUV422, 2:YUV444, 3:YUV420\n");
	key = (char)_getch();
	cs = (mt_u32)key - '0';
	cm = cs >> 2;
	if (cm) {
		SET_COLORIMETRY:
		printf("input colorimetry: 1: 601, 0/2: 709, 3:2020 cYCbCr, 4:2020 YCbCr/RGB\n");
		key = (char)_getch();
		cm = (mt_u32)key - '0';
		if (cm > 4 ){
			goto SET_COLORIMETRY;
		}
		cs = cs & 0x3;
	}
	printf("csc:%d\n", cs);
	#endif
	if ((cs > 3) || (((tvsys != 1) && (tvsys != 3) && (tvsys != 6) && (tvsys != 8) && (tvsys != 28) && (tvsys != 30)) && (cs == 3))) {
		printf("\nError input params!!! input again...\n");
		goto SET_COLOS_SPACE;
	}

	#if HDMI20_DISP_3D
//SET_3D_STRUCTURE:
	printf("3D Structure: 0: 2D, 1: 3D frame packing, 2:3D side by side(half), 3:3D top and bottom\n");
	key = (char)_getch();
	mode_3d = (mt_u32)key - '0';
	if (mode_3d > 3) {
		mode_3d = 0;
	}
	printf("mode_3d:%d\n", mode_3d);

//SET_3D_EXTERNSION:
	//printf("3D Externsion: 0: no, 1: 3d osd disparity, 2:3D dual view, 3:3D independent view\n");
	//key = (char)_getch();
	//mode_3d_ext = (mt_u32)key - '0';
	//printf("mode_3d_ext:%d\n", mode_3d_ext);
	mode_3d_ext = 0;
	#else
	mode_3d = 0;
	mode_3d_ext = 0;
	#endif

	#if HDMI20_DEEP_COLOR
SET_DEEP_COLOR:
	printf("Select Deep Color mode: 0: 8bit, 1: 10bit, 2:12bit\n");
	printf("Warning:BT2020 shall set to 10/12/16bit!!!\n");
	key = (char)_getch();
	bdepth = (mt_u32)key - '0';
	if ( (bdepth > 2) || (cm > 2 && bdepth == 0)) {
		goto SET_DEEP_COLOR;
	}
	printf("bdepth:%s\n", (bdepth == 0) ? "8bit" : ((bdepth == 1) ? "10bit" : "12bit"));
	#else
	bdepth = 0;
	#endif

	ptvsyscmd->cs = cs;
	ptvsyscmd->tvsys = tvsys;
	ptvsyscmd->first_set = pw_on;
	ptvsyscmd->mode_3d = mode_3d;
	ptvsyscmd->mode_3d_ext = mode_3d_ext;
	ptvsyscmd->bdepth = bdepth;
	ptvsyscmd->colorimetry = cm;
	param.tvsys = tvsys;
	set_hdmi_storage_params(param);
}

static void hdmiVideoMuteEnable(uint8_t en)
{
	mt_u32 ret = 0;
	ulong reg_addr = HDMI20_REG_BASE_ADDR + 0xcc;//0x1f4800ccUL;
	mt_u8 reg_val = 0x0;

	ret = misc_reg_read8(reg_addr, &reg_val, 1);
	reg_val &= ~(1 << 5);
	reg_val |= (en << 5) ;
	ret = misc_reg_put8(reg_addr, 0xff, reg_val);  //mute/unmute audio fifo data
	if (ret ) {
		return;
	}
}

static void hdmi_set_h14bvsif(SiiInst_t inst, mt_u32 vic, mt_u32 mode_3d)
{
	uint8_t i = 0;
	uint8_t cksum = 0;
	SiiInfoFrame_t infoframe_t = {0};
	uint8_t *infoframe;

	printf("\r\n %s, vic:%d, mode_3d:%d", __FUNCTION__, vic, mode_3d);
	infoframe = infoframe_t.b;

	//Followed from h1.4b table8-10 ~ 8-15
	infoframe[0] = 0x81; //packet type
	infoframe[1] = 0x01; //version
	infoframe[2] = 0x05; //length
	infoframe[3] = 0; //checksum

	infoframe[4] = 0x03;
	infoframe[5] = 0x0c;
	infoframe[6] = 0x00;

	if (vic == 93) {
		infoframe[7] = 0x20;
		infoframe[8] = 3;
	} else if (vic == 94) {
		infoframe[7] = 0x20;
		infoframe[8] = 2;
	} else if (vic == 95) {
		infoframe[7] = 0x20;
		infoframe[8] = 1;
	} else if (vic == 98) {
		infoframe[7] = 0x20;
		infoframe[8] = 4;
	}

	//3d_structure
	if (mode_3d == 1) { //frame packing
		infoframe[7] = 0x40;
		infoframe[8] = 0x00;//[7:4]
	} else if (mode_3d == 2) { //side by side (half)
		infoframe[2] = 0x06; //length
		infoframe[7] = 0x40;
		infoframe[8] = 0x80;
	} else if (mode_3d == 3) { //top and bottom
		infoframe[7] = 0x40;
		infoframe[8] = 0x60;
	}

	for (i = 0; i < SII_INFOFRAME_MAX_LEN; i++) {
		cksum += infoframe[i];
	}
	cksum = 256 - cksum;
	infoframe[3] = cksum;

	infoframe_t.ifId = SII_INFO_FRAME_ID__VS;
	SiiDrvTxInfoframeSet(inst, &infoframe_t);
}
typedef struct {
	mt_u32 mode; //0:3d osd disparity, 1:3d dual-view, 2:3d independent view
	mt_u32 disparity_ver;
	mt_u32 disparity_len;
	mt_u32 dual_view;
	mt_u32 view_dependency;
	mt_u32 preferred2dview;
} mode3d_ext_t;
static void hdmi_set_hfvsif(SiiInst_t inst, mt_u32 vic, mt_u32 mode_3d, mode3d_ext_t *p_mode)
{
	uint8_t i = 0;
	uint8_t cksum = 0;
	SiiInfoFrame_t infoframe_t = {0};
	uint8_t *infoframe;

	printf("\r\n %s, vic:%d, mode_3d:%d", __FUNCTION__, vic, mode_3d);
	infoframe = infoframe_t.b;

	//Followed from h1.4b table8-10 ~ 8-15
	infoframe[0] = 0x81; //packet type
	infoframe[1] = 0x01; //version
	infoframe[2] = 0x1b; //length
	infoframe[3] = 0; //checksum

	infoframe[4] = 0xd8;
	infoframe[5] = 0x5d;
	infoframe[6] = 0xc4;
	infoframe[7] = 0x01;

	if (mode_3d) {
		infoframe[8] = 1; //3d_valid

		//3d_f_structure
		if (mode_3d == 1) { //frame packing
			infoframe[9] = 0x00;
		} else if (mode_3d == 2) { //side by side (half)
			infoframe[9] = 0x80;
		} else if (mode_3d == 3) { //top and bottom
			infoframe[9] = 0x60;
		}

		if (p_mode->mode == 0) {
			infoframe[9] |= (1 << 2); //3d_disparity_data_present
			infoframe[12] = ((p_mode->disparity_ver & 0x7) << 5) | (p_mode->disparity_len & 0x1f);
		} else if (p_mode->mode == 1) { //3
			infoframe[9] |= (1 << 3); //3d_additional_info_present
			infoframe[11] = 1 << 4;
		} else if (p_mode->mode == 2) {
			infoframe[9] |= (1 << 3); //3d_additional_info_present
			infoframe[11] = ((p_mode->view_dependency & 0x3) << 2) | (p_mode->preferred2dview & 0x3);
		}
	}
	for (i = 0; i < SII_INFOFRAME_MAX_LEN; i++) {
		cksum += infoframe[i];
	}
	cksum = 256 - cksum;
	infoframe[3] = cksum;

	infoframe_t.ifId = SII_INFO_FRAME_ID__VS;
	SiiDrvTxInfoframeSet(inst, &infoframe_t);
}

static void hdmi_check_dc(TVSYSSWITCH_T *tvsys, mt_u32 tmds_clk)
{
	SiiLibEdidPar_t parseEdid = {0};
	mt_u32 dc_valid = 0;

	SiiDrvTxEdidParseGet(tvsys->instTx, &parseEdid);
	switch (tvsys->bdepth) {
		case 1:
			switch (tvsys->cs) {
				case 0:
					if (parseEdid.b444DC10) {
						dc_valid = 1;
					}
					break;
				case 1:
					dc_valid = 0;//YUV422 is always 12bit
					break;
				case 2:
					if (parseEdid.b444DC10 && parseEdid.bY444) {
						dc_valid = 1;
					}
					break;
				case 3:
					if (parseEdid.scdc.bDc30bit420) {
						dc_valid = 1;
					}
					break;
				default:
					break;
			}
			break;
		case 2:
			switch (tvsys->cs) {
				case 0:
					if (parseEdid.b444DC12) {
						dc_valid = 1;
					}
					break;
				case 1:
					dc_valid = 0;//YUV422 is always 12bit
					break;
				case 2:
					if (parseEdid.b444DC12 && parseEdid.bY444) {
						dc_valid = 1;
					}
					break;
				case 3:
					if (parseEdid.scdc.bDc36bit420) {
						dc_valid = 1;
					}
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	if (tmds_clk > 300000) {
		dc_valid = 0;
	}
	if ( dc_valid == 0 ) {
		if (tvsys->bdepth) {
			SSHH_PRINTF("Sink Not Suppor Deep Color for this cs:%d,bp:%d\n", tvsys->cs, tvsys->bdepth);
		}
		tvsys->bdepth = 0;
	}
	SSHH_PRINTF("%s_%d:%d,%d,%d,%d\n",__func__,__LINE__,tvsys->bdepth,tvsys->cs,dc_valid,parseEdid.scdc.bDc30bit420);
}

mt_void set_tvsys_cmd(TVSYSSWITCH_T tvsys)
{
	mt_u32 ret = BB_SUCCESS;
	mt_u32 clk_tmds = 0;
	mt_u32 clk_os_dig = 0;
	mt_u32 pw_on;
	mt_u32 cs;
	mt_u8 scr_on = 0;
	mt_u32 hdmi_mode = 0;
	info_struct_t info;
	MtAviInfo_t AvInfo;
	SiiInfoFrame_t info_st = {0};
	AVINFOSTORAGE_T param;
	static mt_u32 last_mode3d = 0;
	static mt_u32 last_mode3d_ext = 0;
	mt_u32 isclkabove340m = 0;
	hdmiVideoMuteEnable(1); //mute video fifo data and unmute it in Tx_hdmi
	ret = get_hdmi_storage_params(&param);
	//clk_tmds *= 1000;
	//clk_os_dig *= 1000;
	switch (tvsys.tvsys) {
		case 0:
		case 2:
		case 4:
		case 5:
		case 7:
		case 9:
		case 27:
		case 29:
		case 31:
			clk_tmds = 297000;
			clk_os_dig = 297000;
			break;
		case 1:
		case 3:
		case 6:
		case 8:
		case 28:
		case 30:
			clk_tmds = 594000;//RGB/YUV444/YUV422
			clk_os_dig = 594000;
			break;
		case 10:
		case 11:
		case 36:
		case 37:
			clk_tmds = 148500;
			clk_os_dig = 148500;
			break;
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 32:
		case 33:
		case 34:
		case 35:
			clk_tmds = 74250;
			clk_os_dig = 74250;
			break;
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
			clk_tmds = 27000;
			clk_os_dig = 108000;
			break;
		default:
			break;
	}

	pw_on = tvsys.first_set;
	cs = tvsys.cs;
	memset(&AvInfo, 0, sizeof(MtAviInfo_t));
	AvInfo.vic = (mt_u8)hdmi_tvsys2vic(tvsys.tvsys);
	#if 0
	if (tvsys.tvsys >= 19 && tvsys.tvsys <= 22 && param.ar) {
		AvInfo.pictureAR = 1;
	} else {
		AvInfo.pictureAR = 2;
	}
	#else

	AvInfo.pictureAR = hdmi_vic2ar(AvInfo.vic);
	#endif

	{
		//check if this vic support yuv420
		SiiLibEdidPar_t parseEdid = {0};
		int i;
		mt_u32 yuv420cmdb = 0;
		mt_u32 yuv420support = 0;
		mt_u32 need_vsif = 1;

		SiiDrvTxEdidParseGet(tvsys.instTx, &parseEdid);

		if ((cs == 1 && parseEdid.Yuv422 == 0 ) || (cs == 2 && parseEdid.Yuv444 == 0)  ) {
			SSHH_PRINTF("Not support YUV422/444, force output RGB\n");
			cs = 0;
		}

		//check yuv420 CMDB
		for (i = 0; i < 32; i++) {
			if ( (parseEdid.vdb.svd[i].p[0] & 0x7F) == AvInfo.vic) {
				yuv420cmdb = parseEdid.yuv420CMDB[0] | (parseEdid.yuv420CMDB[1] << 8) | \
							 (parseEdid.yuv420CMDB[2] << 16) | (parseEdid.yuv420CMDB[3] << 24);
				SSHH_PRINTF("yuv420cmdb:0x%x\n", yuv420cmdb);
				if (yuv420cmdb & ( 1 << i)) {
					yuv420support = 1;
				}
				break;
			}
		}

		//check yuv420 VDB
		for (i = 0; i < parseEdid.yuv420Vdb.size; i++) {
			if ( (parseEdid.yuv420Vdb.svd[i].p[0] & 0x7F) == AvInfo.vic) {
				yuv420support = 1;
				break;
			}
		}
		if ( parseEdid.maxTmds < 297000000 && cs == 3) {
			SSHH_PRINTF("MaxTmds %d is not support yuv420, Force to set to RGB output\n", parseEdid.maxTmds);
			yuv420support = 0;
		}
		if ( 0 == yuv420support && cs == 3) {
			SSHH_PRINTF("vic %d DO NOT support yuv420, Force to set to RGB output\n", (mt_u32)AvInfo.vic);
			cs = 0;
		}
		SSHH_PRINTF("support 3d:%x\n", parseEdid.db3d.b[0] & 0x80);

		//check 3d_present
		if (((parseEdid.db3d.b[0] & 0x80) == 0) && tvsys.mode_3d) {
			SSHH_PRINTF("sink doesn't support 3d\n");
			tvsys.mode_3d = 0;
		}
		if (tvsys.mode_3d) {
			if (parseEdid.scdc.b3DOsdDisparity == 0 && tvsys.mode_3d_ext == 1) {
				SSHH_PRINTF("sink doesn't support Osd Disparity\n");
				tvsys.mode_3d_ext = 0;
			}
			if (parseEdid.scdc.bDualView == 0 && tvsys.mode_3d_ext == 2) {
				SSHH_PRINTF("sink doesn't support Dual View\n");
				tvsys.mode_3d_ext = 0;
			}
			if (parseEdid.scdc.bIndependentView == 0 && tvsys.mode_3d_ext == 3) {
				SSHH_PRINTF("sink doesn't support Independent View\n");
				tvsys.mode_3d_ext = 0;
			}
		} else {
			tvsys.mode_3d_ext = 0;
		}

		SSHH_PRINTF("mode_3d:%d, last_mode3d:%d, mode_3d_ext:%d, last_mode3d_ext:%d\n",
					tvsys.mode_3d, last_mode3d, tvsys.mode_3d_ext, last_mode3d_ext);

		if (AvInfo.vic == 93 || AvInfo.vic == 94 || AvInfo.vic == 95 || AvInfo.vic == 98) {
			if (parseEdid.scdc.bUHD_VIC == 1) {
				for (i = 0; i < 32; i++) {
					if ((parseEdid.vdb.svd[i].p[0] & 0x7F) == AvInfo.vic) {
						need_vsif = 0;
					}
				}
			}
		} else if (tvsys.mode_3d == 0) {
			need_vsif = 0;
		}

		if (need_vsif == 1) {
			if (tvsys.mode_3d && tvsys.mode_3d_ext) {
				mode3d_ext_t ext = {0};
				if (tvsys.mode_3d_ext == 1) {
					ext.mode = 0;
					ext.disparity_ver = 0;
					ext.disparity_len = 0;
				} else if (tvsys.mode_3d_ext == 2) {
					ext.mode = 1;
				} else if (tvsys.mode_3d_ext == 3) {
					ext.mode = 2;
					ext.preferred2dview = 3;
					ext.view_dependency = 0;
				}
				hdmi_set_hfvsif(tvsys.instTx, AvInfo.vic, tvsys.mode_3d, &ext);
				SiiDrvTxInfoframeOnOffSet(tvsys.instTx, SII_INFO_FRAME_ID__VS	, true);
			} else {
				hdmi_set_h14bvsif(tvsys.instTx, AvInfo.vic, tvsys.mode_3d);
				SiiDrvTxInfoframeOnOffSet(tvsys.instTx, SII_INFO_FRAME_ID__VS	, true);
			}
			if (tvsys.mode_3d == 0) { //hdmi_vic
				SSHH_PRINTF("send h14b-vsif, set vic to 0 in avi\n");
				AvInfo.vic = 0;
				AvInfo.pictureAR = 2; //16:9
			}
		} else if (last_mode3d_ext != 0) { //switch 3d to 2d
			mode3d_ext_t ext = {0};
			SSHH_PRINTF("switch from 3d to 2d, send hf-vsif with 3d_valid=000\n");
			hdmi_set_hfvsif(tvsys.instTx, 0, 0, &ext);
			SiiDrvTxInfoframeOnOffSet(tvsys.instTx, SII_INFO_FRAME_ID__VS	, true);
		} else if (last_mode3d != 0) { //switch 3d to 2d
			SSHH_PRINTF("switch from 3d to 2d, send h14b-vsif with hdmi_video_format(3d structure)=000\n");
			hdmi_set_h14bvsif(tvsys.instTx, 0, 0);
			SiiDrvTxInfoframeOnOffSet(tvsys.instTx, SII_INFO_FRAME_ID__VS	, true);
		}
		#if 1
		else {
			SSHH_PRINTF("disable vsif\n");
			SiiDrvTxInfoframeOnOffSet(tvsys.instTx, SII_INFO_FRAME_ID__VS	, false);
		}
		#endif
	}
	last_mode3d = tvsys.mode_3d;
	last_mode3d_ext = tvsys.mode_3d_ext;

	if (3 == cs) {
		#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		clk_tmds = 300000;//YUV420
		#else
		clk_tmds = 297000;//YUV420
		#endif
	}

	if (tvsys.mode_3d == 1 && clk_tmds < 300000 ) {
		clk_tmds <<= 1;
	} else if ( tvsys.mode_3d == 1 ) {
		SSHH_PRINTF("Err param in, Not support 3D for tvsys:%d,cs:%d,bp:%d,3d:%d\n",tvsys.tvsys,tvsys.cs,tvsys.bdepth,tvsys.mode_3d);
	}
	hdmi_check_dc(&tvsys, clk_tmds);

	{
		//check tv format
		SiiLibEdidPar_t parseEdid = {0};
		int i;
		mt_u32 tvSysSupport = 0;
		SiiDrvTxEdidParseGet(tvsys.instTx, &parseEdid);
		//check VDB
		for (i = 0; i < 32; i++) {
			if ( (parseEdid.vdb.svd[i].p[0] & 0x7F) == AvInfo.vic) {
				tvSysSupport = 1;
				break;
			}
		}

		//check yuv420 VDB
		for (i = 0; i < parseEdid.yuv420Vdb.size; i++) {
			if ( (parseEdid.yuv420Vdb.svd[i].p[0] & 0x7F) == AvInfo.vic) {
				tvSysSupport = 1;
				break;
			}
		}
		if ( 0 == tvSysSupport ) {
			SSHH_PRINTF("WARNING: vic %d DO NOT support\n", (mt_u32)AvInfo.vic);
		}

		//init av infoframe
		SSHH_PRINTF("tmds:%d, os_dig:%d\n", clk_tmds, clk_os_dig);
		AvInfo.clrSpc = (mt_u8)cs;
		//AvInfo.vic = (mt_u8)hdmi_tvsys2vic(tvsys.tvsys);
		AvInfo.repetition = (mt_u8)hdmi_tvsys2rep(tvsys.tvsys);
		AvInfo.scanInfo = 0x0;//underscan..
		if (tvsys.colorimetry == 0) {
			AvInfo.colorimetry = 0x2;    //BT709
			AvInfo.extColorimetry = 0;
		} else {
			switch (tvsys.colorimetry) {
				case 1:
					AvInfo.colorimetry = 1;
					AvInfo.extColorimetry = 0; //Bt.601
					break;
				case 2:
					AvInfo.colorimetry = 2;
					AvInfo.extColorimetry = 0; //Bt.709
					break;
				case 3:
					if (parseEdid.colorimetry.BT2020cYCC) {
						AvInfo.colorimetry = 3;
						AvInfo.extColorimetry = 5; //Bt.2020 cYCbCr
					} else {
						AvInfo.colorimetry = 2;
						AvInfo.extColorimetry = 0; //Bt.709
					}
					break;
				case 4:
					if (parseEdid.colorimetry.BT2020YCC || parseEdid.colorimetry.BT2020RGB) {
						AvInfo.colorimetry = 3;
						AvInfo.extColorimetry = 6; //Bt.2020 YCbCr or RGB
					} else {
						AvInfo.colorimetry = 2;
						AvInfo.extColorimetry = 0; //Bt.709
					}
					break;
				default:
					AvInfo.colorimetry = 2;
					AvInfo.extColorimetry = 0;
					break;
			}
		}
	}
	AvInfo.activeAR = 8;

	SSHH_PRINTF("csc:%d, vic:%d, rep:%d,ar:%d\n", (mt_u32)AvInfo.clrSpc, (mt_u32)AvInfo.vic, (mt_u32)AvInfo.repetition, (mt_u32)AvInfo.pictureAR);

	hdmi_set_avi_infoframe(AvInfo, &info);
	info_st.ifId = SII_INFO_FRAME_ID__AVI;
	SII_MEMCPY((uint8_t*)&info_st.b, &info, SII_INFOFRAME_AVI_MAX_LEN);
	SiiDrvTxInfoframeSet(tvsys.instTx, &info_st);
	if (tvsys.bdepth == 0) {
		isclkabove340m = (clk_tmds > 340000);
	} else if (tvsys.bdepth == 1) {
		isclkabove340m = ((clk_tmds * 5 / 4) > 340000);
	} else if (tvsys.bdepth == 2) {
		isclkabove340m = ((clk_tmds * 3 / 2) > 340000);
	}
	#if 1
	if (isclkabove340m) {
		//4kP50/60 use hdmi2 stream
		SiiDrvTxScdcSrcCrSet(tvsys.instTx, 1);
	} else {
		SiiDrvTxScdcSrcCrSet(tvsys.instTx, 0);
	}
	hdmi_mode =  1;
	scr_on =  1;
	if (isclkabove340m) {
		SiiDrvTxScdcSrcStreamTypeSet(tvsys.instTx, 1);
	} else {
		SiiDrvTxScdcSrcStreamTypeSet(tvsys.instTx, 0);
	}
	#endif
	ret = do_case_set_tvsys(clk_tmds, clk_os_dig, pw_on, cs, 0, tvsys.tvsys, scr_on, hdmi_mode, info, tvsys.mode_3d, tvsys.bdepth, tvsys.colorimetry);
	param.tvsys = tvsys.tvsys;
	if (ret != BB_FAILURE) {
		set_hdmi_storage_params(param);
	}

	if (ret ) {
		return;
	}
}

mt_void hdmi_save_avinfoframe(SiiInst_t inst)
{
	SiiInfoFrame_t info_st = {0};
	MtAviInfo_t AvInfo;
	info_struct_t info;
	AVINFOSTORAGE_T param;
	mt_u32 ret = BB_SUCCESS;

	ret = get_hdmi_storage_params(&param);

	memset(&AvInfo, 0, sizeof(MtAviInfo_t));
	AvInfo.clrSpc = (mt_u8)param.vinfo.clrSpc;
	AvInfo.vic = (mt_u8)hdmi_tvsys2vic(param.tvsys);
	AvInfo.repetition = (mt_u8)hdmi_tvsys2rep(param.tvsys);
	AvInfo.scanInfo = 0x2;//underscan
	if (AvInfo.clrSpc) {
		AvInfo.colorimetry = 0x2;    //BT709
	}
	AvInfo.activeAR = 8;
	#if 0
	AvInfo.pictureAR = param.ar ? 1 : 2;
	#else
	AvInfo.pictureAR = hdmi_vic2ar(AvInfo.vic);
	#endif

	hdmi_set_avi_infoframe(AvInfo, &info);
	info_st.ifId = SII_INFO_FRAME_ID__AVI;
	SII_MEMCPY((uint8_t*)&info_st.b, &info, SII_INFOFRAME_AVI_MAX_LEN);
	SiiDrvTxInfoframeSet(inst, &info_st);
	param.vinfo = AvInfo;
	set_hdmi_storage_params(param);
	if (ret ) {
		return;
	}
}

mt_u32 hdmi_get_test_tvsys(mt_void)
{
	AVINFOSTORAGE_T param = {0};
	mt_u32 ret = BB_SUCCESS;

	ret = get_hdmi_storage_params(&param);
	if (ret == BB_SUCCESS ) {
		return (param.tvsys);
	} else {
		return (0xffff);
	}
}

mt_void test_set_ar_cmd(SiiInst_t        instTx)
{
	char key;
	mt_u32 ar = 0;
	AVINFOSTORAGE_T param;
	MtAviInfo_t AvInfo;
	SiiInfoFrame_t info_st = {0};
	info_struct_t info;

	(mt_void)get_hdmi_storage_params(&param);
	AvInfo = param.vinfo;
	memset(&info_st, 0, sizeof(SiiInfoFrame_t));

SET_DISP_AR:

	printf("\nInput ar, '0': 16:9, '1': 4:3, instTx:0x%x\n", (mt_u32)instTx);
	key = (char)_getch();
	ar = (mt_u32)(key - '0');
	printf("Disp AR:%d\n", ar);
	if (ar < 0 || ar > 1) {
		printf("\nError input params!!! input again...\n");
		goto SET_DISP_AR;
	}

	if (ar == param.ar) {
		printf("\nAR is the same...\n");
		return;
	}
	if (param.tvsys >= 19 && param.tvsys <= 22) {
		printf("csc:%d, vic:%d, rep:%d,ar:%d\n", (mt_u32)AvInfo.clrSpc, (mt_u32)AvInfo.vic, (mt_u32)AvInfo.repetition, (mt_u32)AvInfo.pictureAR);
		hdmi_set_avi_infoframe(AvInfo, &info);
		info_st.ifId = SII_INFO_FRAME_ID__AVI;
		SII_MEMCPY((uint8_t*)&info_st.b, &info, SII_INFOFRAME_AVI_MAX_LEN);
		SiiDrvTxInfoframeSet(instTx, &info_st);
	}

	param.ar = ar;
	set_hdmi_storage_params(param);
	//return 0;
}

/***** local functions *******************************************************/

/***** end of file ***********************************************************/

