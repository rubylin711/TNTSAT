/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_VXDSSC_H__
#define __INC_MT_ANALOG_VXDSSC_H__

/*
 * Usage:
 *     mt_analog_get_attr(MT_ANA_INDEX_VHDINTP, struct mt_analog_vhdintp_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_VHDINTP, struct mt_analog_vhdintp_attr *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_VSDINTP, struct mt_analog_vsdintp_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_VSDINTP, struct mt_analog_vsdintp_attr *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_VSDSSC, struct mt_analog_vsdssc_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_VSDSSC, struct mt_analog_vsdssc_attr *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_VHDSSC, struct mt_analog_vhdssc_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_VHDSSC, struct mt_analog_vhdssc_attr *);
 */

/* clkgen_vhdintp_reg: 0xBF5D005C */
typedef struct mt_analog_vhdintp_attr
{
	//A0
	//u32 reserved0: 1;			/* bit 0 */
	//A1
	/*
	 * 0: vsd os108 from vsdpll
	 * 1: vsd os108 from vhdpll
	 */
	u32 vsd_os108M_sel: 1;

	/*
	 * 000: 74.25
	 * 001: 148.5
	 * 010: 297
	 * 011: 594
	 * 10x: 108
	 * 11x: 27
	 */
	u32 clk_os_sel: 3; 			/* bit 1-3 */

	/*
	 * 00: 270M from vhd
	 * 01: 742.5M
	 * 10: 1485M
	 * 11: 270M from vsd
	 */
	u32 clk_hdmitx_sel: 2;		/* bit 4-5 */

	u32 clk_rng_fix_en: 1;		/* bit 6, 1 clk rng fix enable, 0 clk rng fix disable */

	//A0
	//u32 reserved7: 1;			/* bit 7 */
	//A1
	/*
	 * 0: tmds 27M from vhdpll
	 * 1: tmds 54M/108M from vhdpll
	 */
	u32 tmds_sel: 1;

	u32 div_sel: 2;				/* bit 8-9, update freq of intp. update freq = 2^(4-sel) */

	u32 sw_edge: 1;				/* bit 10, switch output clock edge */

	u32 clksel: 1; 				/* bit 11, dpher clock edge sel of intp. 0: negedge, 1: posedge */

	u32 i_sel: 3; 				/* bit 12-14, current sel of video intp. */

	/*
	 * 1 is valid for this signal dco ctrl valid.
	 * This bit must be set to 1 for about 1us, after dco_ctrl_data is stable.
	 * And it is controlled by software.
	 */
	u32 dco_ctrl_valid: 1;		/* bit 15 */

	u32 dco_ctrl_data: 16;		/* bit 16-31 */

} mt_analog_vhdintp_attr_t;

/* clkgen_vsdintp_reg: 0xBF5D0064 */
typedef struct mt_analog_vsdintp_attr
{
	/*
	 * 000: 74.25
	 * 001: 148.5
	 * 010: 297
	 * 011: 74.25
	 * 100: 108
	 * 101: 27
	 * 110: 108
	 * 111: 27
	 */
	u32 clk_hd_ana_sel: 3; 		/* bit 0-2 */

	/*
	 * 0: 108M
	 * 1: 27M
	 */
	u32 clk_vsd_sel: 1;			/* bit 3 */

	u32 pd_clk_vsdpll_d2: 1;	/* bit 4 */
	u32 pd_clk_vsdpll_d4: 1;	/* bit 5 */

	u32 reserved6: 2;			/* bit 6-7 */

	u32 div_sel: 2;				/* bit 8-9, update freq of intp. update freq = 2^(4-sel) */

	u32 sw_edge: 1;				/* bit 10, switch output clock edge */

	u32 clksel: 1; 				/* bit 11, dpher clock edge sel of intp. 0: negedge, 1: posedge */

	u32 i_sel: 3; 				/* bit 12-14, current sel of video intp. */

	/*
	 * 1 is valid for this signal dco ctrl valid.
	 * This bit must be set to 1 for about 1us, after dco_ctrl_data is stable.
	 * And it is controlled by software.
	 */
	u32 dco_ctrl_valid: 1;		/* bit 15 */

	u32 dco_ctrl_data: 16;		/* bit 16-31 */

} mt_analog_vsdintp_attr_t;

/* clkgen_vsdssc_reg: 0xBF5D0144 */
typedef struct mt_analog_vsdssc_attr
{
	/*
	 * 000: no ssc
	 * 001: ssc 1485 or 74.25
	 * 010: ssc 1485 or 74.24 x1 x1.25 x1.5 x2
	 * 011: ssc 1485 or 74.24 x1 x1.25 x1.5 x2
	 * 100: no ssc
	 * 101: ssc 270
	 * 110: ssc 270 x1.25 x1.5 x2
	 * 111: ssc 270 x1.25 x1.5 x2
	 */
	u32 clk_sel: 3;				/* bit 0-2 */

	u32 en_ssc: 1;				/* bit 3 */
	u32 pd_ssc: 1;				/* bit 4 */

	u32 dif_fb_div2_opt: 1;		/* bit 5, 1: dif fb div2 0: dif fb div1 */

	u32 reserved: 26;			/* bit 6-31 */

} mt_analog_vsdssc_attr_t;

/* clkgen_vhdssc_reg: 0xBF5D0140 */
typedef struct mt_analog_vhdssc_attr
{
	u32 udate: 4;				/* bit 0-3, div=2+hex2dec(update) */

	u32 intp_sel: 1;			/* bit 4 */
	u32 intp_clksel: 3;			/* bit 5-7 */

	u32 n: 7;					/* bit 8-14, div=8+hex2dec(n) */
	u32 sign_en: 1;				/* bit 15, 1: signed, 0: unsigned */

	u32 minus_mmax: 8;			/* bit 16-23 */
	u32 mmax: 8;				/* bit 24-31 */

} mt_analog_vhdssc_attr_t;

//---------------------------------------------------------------------------//

#ifndef NO_DUMP_VXDSSC_STATE
static void mt_vxdssc_dump_state(void *s)
{
	struct mt_analog_vhdintp_attr vhdinpt_attr;
	struct mt_analog_vsdintp_attr vsdinpt_attr;

	struct mt_analog_vsdssc_attr vsdssc_attr;
	struct mt_analog_vhdssc_attr vhdssc_attr;

	mt_analog_get_attr(MT_ANA_INDEX_VHDINTP, (void*)&vhdinpt_attr);
	mt_analog_get_attr(MT_ANA_INDEX_VSDINTP, (void*)&vsdinpt_attr);

	mt_analog_get_attr(MT_ANA_INDEX_VSDSSC, (void*)&vsdssc_attr);
	mt_analog_get_attr(MT_ANA_INDEX_VHDSSC, (void*)&vhdssc_attr);

	DP_LOG("----------------------[VHDINTP]-------------------\n");
	DP_LOG("clkgen_vhdintp_reg[%8lX]: %08X\n", REG_CLKGEN_VHDINTP, MT_IO_READ32(REG_CLKGEN_VHDINTP));

	DP_LOG("       vsd_os108M_sel: %u\n", vhdinpt_attr.vsd_os108M_sel);	/* A1+ */
	DP_LOG("         clk_os_sel: %u\n", vhdinpt_attr.clk_os_sel);
	DP_LOG("     clk_hdmitx_sel: %u\n", vhdinpt_attr.clk_hdmitx_sel);
	DP_LOG("     clk_rng_fix_en: %u\n", vhdinpt_attr.clk_rng_fix_en);
	DP_LOG("tmds 27M 54M/108M sel: %u\n", vhdinpt_attr.tmds_sel);		/* A1+ */
	DP_LOG("            div_sel: %u\n", vhdinpt_attr.div_sel);
	DP_LOG("            sw_edge: %u\n", vhdinpt_attr.sw_edge);
	DP_LOG("             clksel: %u\n", vhdinpt_attr.clksel);
	DP_LOG("              i_sel: %u\n", vhdinpt_attr.i_sel);
	DP_LOG("     dco_ctrl_valid: %u\n", vhdinpt_attr.dco_ctrl_valid);
	DP_LOG("      dco_ctrl_data: 0x%04X\n", vhdinpt_attr.dco_ctrl_data);

	DP_LOG("----------------------[VSDINTP]-------------------\n");
	DP_LOG("clkgen_vsdintp_reg[%8lX]: %08X\n", REG_CLKGEN_VSDINTP, MT_IO_READ32(REG_CLKGEN_VSDINTP));

	DP_LOG("     clk_hd_ana_sel: %u\n", vsdinpt_attr.clk_hd_ana_sel);
	DP_LOG("        clk_vsd_sel: %u\n", vsdinpt_attr.clk_vsd_sel);
	DP_LOG("   pd_clk_vsdpll_d2: %u\n", vsdinpt_attr.pd_clk_vsdpll_d2);
	DP_LOG("   pd_clk_vsdpll_d4: %u\n", vsdinpt_attr.pd_clk_vsdpll_d4);
	DP_LOG("            div_sel: %u\n", vsdinpt_attr.div_sel);
	DP_LOG("            sw_edge: %u\n", vsdinpt_attr.sw_edge);
	DP_LOG("             clksel: %u\n", vsdinpt_attr.clksel);
	DP_LOG("              i_sel: %u\n", vsdinpt_attr.i_sel);
	DP_LOG("     dco_ctrl_valid: %u\n", vsdinpt_attr.dco_ctrl_valid);
	DP_LOG("      dco_ctrl_data: 0x%04X\n", vsdinpt_attr.dco_ctrl_data);

	DP_LOG("----------------------[VSDSSC]-------------------\n");
	DP_LOG("clkgen_vsdssc_reg[%8lX]: %08X\n", REG_CLKGEN_VSDSSC, MT_IO_READ32(REG_CLKGEN_VSDSSC));

	DP_LOG("            clk_sel: %u\n", vsdssc_attr.clk_sel);
	DP_LOG("         enable ssc: %u\n", vsdssc_attr.en_ssc);
	DP_LOG("             pd ssc: %u\n", vsdssc_attr.pd_ssc);
	DP_LOG(" dif fb div2 option: %u\n", vsdssc_attr.dif_fb_div2_opt);

	DP_LOG("----------------------[VHDSSC]-------------------\n");
	DP_LOG("clkgen_vhdssc_reg[%8lX]: %08X\n", REG_CLKGEN_VHDSSC, MT_IO_READ32(REG_CLKGEN_VHDSSC));

	DP_LOG("        udate: 0x%X\n", vhdssc_attr.udate);
	DP_LOG("     intp_sel: %u\n", vhdssc_attr.intp_sel);
	DP_LOG("  intp_clksel: %u\n", vhdssc_attr.intp_clksel);
	DP_LOG("            n: 0x%X\n", vhdssc_attr.n);
	DP_LOG("      sign_en: %u\n", vhdssc_attr.sign_en);
	DP_LOG("   minus_mmax: 0x%X\n", vhdssc_attr.minus_mmax);
	DP_LOG("         mmax: 0x%X\n", vhdssc_attr.mmax);
}
#endif

#endif

