/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_USB_ACLK_H__
#define __INC_MT_ANALOG_USB_ACLK_H__

/**
 *
 * <CNComment> USB/Audio ClockͬԴ
 *
 * USB/Audio Clock same origin
 *
 */

/*
 * Usage:
 *     mt_analog_get_attr(MT_ANA_INDEX_USBACLK_R0, struct mt_analog_usbaclk_r0_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_USBACLK_R0, struct mt_analog_usbaclk_r0_attr *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_USBACLK_R2, struct mt_analog_usbaclk_r2_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_USBACLK_R2, struct mt_analog_usbaclk_r2_attr *);
 */

/* clkgen_usbaclk_reg0: 0xBF5D0120 */
typedef struct mt_analog_usbaclk_r0_attr
{
	u32 pd_adac_clk_cp: 1;				/* bit 0 */
	u32 dcc_xpd: 1;						/* bit 1 */
	u32 dcc_sel: 2;						/* bit 2-3 */

	//A0
	//u32 reserved1: 4;					/* bit 4-7 */
	//A1
	/*
	 * 00: video ref from 180M
	 * 01: video ref from ref pll
	 * 1x: video ref from 27M
	 */
	u32 vid_ref_clk_sel: 2;				/* bit 4-5 */
	u32 reserved1: 2;					/* bit 6-7 */

	/*
	 * 0: follow sym4
	 * 1: 297, 594, 108
	 */
	u32 hdmi_clk_tmds_sel: 1;			/* bit 8 */
	/*
	 * 0: follow sym4
	 * 1: 270x1 x1.25 x1.5 => 540x1 x1.25 x1.5
	 */
	u32 sel540M: 1;						/* bit 9 */
	/*
	 * 0: follow sym4
	 * 1: 594M
	 */
	u32 sel2970M: 1;					/* bit 10 */

	u32 reserved2: 9;					/* bit 11-19 */

	u32 pd_clk_vhdpll_d5: 1;			/* bit 20 */
	u32 pd_clk_vhdpll_d6: 1;			/* bit 21 */
	u32 pd_clk_usbpll_d2p25: 1;			/* bit 22 */

	u32 reserved23: 1;					/* bit 23 */

	u32 pll_v_ref_clk_sets: 4;			/* bit 24-27 */
	u32 pll_u_ref_clk_sets: 4;			/* bit 28-31 */

} mt_analog_usbaclk_r0_attr_t;

/* clkgen_usbaclk_reg2: 0xBF5D0128 */
typedef struct mt_analog_usbaclk_r2_attr
{
	/*
	 * audiopll = 480*2^12/FRAC_NUM/5
	 *
	 * e.g.
	 *   FRAC_NUM = 0xFA0
	 *   audiopll = 480 * 4096 / 4000 / 5 = 98.304
	 */
	u32 frac_num: 12;					/* bit 0-11 */

	u32 data_valid: 1;					/* bit 12 */

	u32 reserved2: 1;					/* bit 13, reset */

	u32 pd_clk: 1;						/* bit 14 */
	u32 div_sel: 1;						/* bit 15, 0: div5, 1: div6 */

	u32 reserved3: 16;					/* bit 16-31 */

} mt_analog_usbaclk_r2_attr_t;

#ifndef NO_DUMP_USBACLK_STATE
static void mt_usbaclk_dump_state(void *s)
{
	struct mt_analog_usbaclk_r0_attr usbaclk0_attr;
	struct mt_analog_usbaclk_r2_attr usbaclk2_attr;

	mt_analog_get_attr(MT_ANA_INDEX_USBACLK_R0, (void*)&usbaclk0_attr);
	mt_analog_get_attr(MT_ANA_INDEX_USBACLK_R2, (void*)&usbaclk2_attr);

	DP_LOG("--------------------[USBACLK]--------------------\n");
	DP_LOG("clkgen_usbaclk_reg0[%8lX]: %08X\n", CLKGEN_USBACLK_REG0, MT_IO_READ32(CLKGEN_USBACLK_REG0));
	DP_LOG("clkgen_usbaclk_reg1[%8lX]: %08X\n", CLKGEN_USBACLK_REG1, MT_IO_READ32(CLKGEN_USBACLK_REG1));
	DP_LOG("clkgen_usbaclk_reg2[%8lX]: %08X\n", CLKGEN_USBACLK_REG2, MT_IO_READ32(CLKGEN_USBACLK_REG2));

	DP_LOG("\nclkgen_usbaclk_reg0[%8lX]: %08X\n", CLKGEN_USBACLK_REG0, MT_IO_READ32(CLKGEN_USBACLK_REG0));
	DP_LOG("         pd_adac_clk_cp: %u\n", usbaclk0_attr.pd_adac_clk_cp);
	DP_LOG("                dcc_xpd: %u\n", usbaclk0_attr.dcc_xpd);
	DP_LOG("                dcc_sel: %u\n", usbaclk0_attr.dcc_sel);
	DP_LOG("      video_ref_clk_sel: %u\n", usbaclk0_attr.vid_ref_clk_sel);				/* A1 */
	DP_LOG("      hdmi_clk_tmds_sel: %u\n", usbaclk0_attr.hdmi_clk_tmds_sel);
	DP_LOG("                sel540M: %u\n", usbaclk0_attr.sel540M);
	DP_LOG("               sel2970M: %u\n", usbaclk0_attr.sel2970M);
	DP_LOG("       pd_clk_vhdpll_d5: %u\n", usbaclk0_attr.pd_clk_vhdpll_d5);
	DP_LOG("       pd_clk_vhdpll_d6: %u\n", usbaclk0_attr.pd_clk_vhdpll_d6);
	DP_LOG("    pd_clk_usbpll_d2p25: %u\n", usbaclk0_attr.pd_clk_usbpll_d2p25);
	DP_LOG("     pll_v_ref_clk_sets: 0x%X\n", usbaclk0_attr.pll_v_ref_clk_sets);
	DP_LOG("     pll_u_ref_clk_sets: 0x%X\n", usbaclk0_attr.pll_u_ref_clk_sets);

	DP_LOG("\nclkgen_usbaclk_reg2[%8lX]: %08X\n", CLKGEN_USBACLK_REG2, MT_IO_READ32(CLKGEN_USBACLK_REG2));
	DP_LOG("    FRAC NUM: 0x%X\n", usbaclk2_attr.frac_num);
	DP_LOG("  data_valid: %u\n", usbaclk2_attr.data_valid);
	DP_LOG("      pd_clk: %u\n", usbaclk2_attr.pd_clk);
	DP_LOG("     div_sel: %u\n", usbaclk2_attr.div_sel);
}
#endif

#endif

