/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_VDAC_H__
#define __INC_MT_ANALOG_VDAC_H__

/*
 * Usage:
 *     mt_analog_get_attr(MT_ANA_INDEX_VDAC, struct mt_analog_vdac_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_VDAC, struct mt_analog_vdac_attr *);
 */

/* vdac_reg0: 0xBF5D0010 */
typedef struct mt_analog_vdac_attr
{
	u32 vref_sel: 7;				/* bit 0-6, vdac0 reference current select */
	u32 bypath_filter: 1;			/* bit 7, vdac0 digital filter select, 0: through, 1: bypath */

	u32 dac_det_sel: 1;				/* bit 8, vdac0 det select, 0: register control, 1: reg<19> control */
	u32 rpu: 2;						/* bit 9-10, vdac0 driver select, 00: 0mA, 01: 0.4mA, 10: 0.6mA, 11: 1.0mA */
	u32 clk_polarity: 1;			/* bit 11, vdac0 clk polarity select, 0: fall edge sample, 1: rise edge sample */

	u32 en_clk_ext: 1;				/* bit 12, vdac0 clk select, 0: select pll clk, 1: select external clk */
	u32 en_clk_pll_sd: 1;			/* bit 13, vdac0 clk select, 0: select hd clk, 1: select sd clk */
	u32 clk_dig_polarity: 1;		/* bit 14, vdac0 clk dig polarity select, 0: fall edge sample, 1: rise edge sample */
	u32 reserved1: 1;				/* bit 15, vdac0 rst, 0: normal mode, 1: reset */

	u32 rpd_imp: 2; 				/* bit 16-17, vdac0 rpd imp, 00: 2k/7, 01: 2k/8, 10: 2k/9, 11: 2k/10 */
	u32 det_en_reg_ctr: 1;			/* bit 18, det en reg control */
	u32 det_sel_reg_ctr: 1;			/* bit 19, det sel reg control */

	u32 buf_cap_sel: 1;				/* bit 20, buf cap select, 0: no cap, 1: with cap */
	u32 buf_out_sel: 1;				/* bit 21, buf output driver select, 0: 8mA, 1: 16mA */
	u32 buf_cur_sel: 1;				/* bit 22, buf current select, 0: 50uA, 1: 62.5uA */
	u32 buf_vcm_sel: 1;				/* bit 23, buf vcm select, 0: vdda/2, 1: 1.5V */

	u32 reserved2: 8;				/* bit 24-31 */

} mt_analog_vdac_attr_t;

//---------------------------------------------------------------------------//

static void mt_vdac_dump_state(void *s)
{
	struct mt_analog_vdac_attr vdac_attr;

	mt_analog_get_attr(MT_ANA_INDEX_VDAC, (void*)&vdac_attr);

	DP_LOG("-----------------------[VDAC]--------------------\n");
	DP_LOG("vdac_reg0[%8lX]: %08X\n", VDAC_REG0, MT_IO_READ32(VDAC_REG0));
	DP_LOG("           vref sel: 0x%02X\n", vdac_attr.vref_sel);
	DP_LOG("      bypath filter: %u\n", vdac_attr.bypath_filter);
	DP_LOG("        dac det sel: %u\n", vdac_attr.dac_det_sel);
	DP_LOG("                rpu: %u\n", vdac_attr.rpu);
	DP_LOG("       clk polarity: %u\n", vdac_attr.clk_polarity);
	DP_LOG("         en clk ext: %u\n", vdac_attr.en_clk_ext);
	DP_LOG("      en clk pll sd: %u\n", vdac_attr.en_clk_pll_sd);
	DP_LOG("   clk dig polarity: %u\n", vdac_attr.clk_dig_polarity);
	//DP_LOG("              reset: %u\n", vdac_attr.rst);
	DP_LOG("            rpd imp: %u\n", vdac_attr.rpd_imp);
	DP_LOG("    det en reg ctrl: %u\n", vdac_attr.det_en_reg_ctr);
	DP_LOG("   det sel reg ctrl: %u\n", vdac_attr.det_sel_reg_ctr);
	DP_LOG("        buf cap sel: %u\n", vdac_attr.buf_cap_sel);
	DP_LOG("        buf out sel: %u\n", vdac_attr.buf_out_sel);
	DP_LOG("        buf cur sel: %u\n", vdac_attr.buf_cur_sel);
	DP_LOG("        buf vcm sel: %u\n", vdac_attr.buf_vcm_sel);

	//DP_LOG("-------------------------------------------------\n");
}

#endif

