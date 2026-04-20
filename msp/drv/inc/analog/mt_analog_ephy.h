/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_EPHY_H__
#define __INC_MT_ANALOG_EPHY_H__

/*
 * Usage:
 *     mt_analog_get_attr(MT_ANA_INDEX_EPHY, struct mt_analog_ephy_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_EPHY, struct mt_analog_ephy_attr *);
 */

/* clkgen_ephy: 0xBF5D011C */
typedef struct mt_analog_ephy_attr
{
	/* clkgen_ephyrx_reg<15:0> */
	u32 data_ext: 6;				/* bit 0-5 */
	u32 data_sel: 1;				/* bit 6 */
	u32 reserved1: 1;				/* bit 7, reset */

	u32 div_sel: 2;					/* bit 8-9, update freq of intp */
	u32 reserved2: 1;				/* bit 10 */
	u32 clk_edge_sel: 1;			/* bit 11, dpher clock edge sel of intp. 0: negedge, 1: posedge */
	u32 i_sel: 3;					/* bit 12-14, current set of video intp */
	u32 dco_ctrl_valid: 1;			/* bit 15, This bit must be set to 1 for about 1us, after dco_ctrl_data is stable. */

	/* clkgen_pdephy_reg<15:0> */
	u32 pd_ethpll_d2: 1;			/* bit 16 */
	u32 pd_ethpll_d3: 1;			/* bit 17 */
	u32 pd_ethpll_d3p25: 1;			/* bit 18 */
	u32 pd_ethpll_d5: 1;			/* bit 19 */
	u32 reserved3: 2;				/* bit 20-21 */
	u32 mux: 1;						/* bit 22, 0: div1, 1: div6 */
	u32 div7_duty: 1;				/* bit 23 */

	u32 pd_clk_div_4p5: 1;			/* bit 24 */
	u32 ephy_ana_en: 1;				/* bit 25, ephy ana enable */
	u32 clk_tst_sel_ephy: 2;		/* bit 26-27 */
	u32 clk_tst_sel_dig: 3;			/* bit 28-30 */
	u32 clk_tst_en_ephy: 1;			/* bit 31 */

} mt_analog_ephy_attr_t;

//---------------------------------------------------------------------------//

static void mt_ephy_dump_state(void *s)
{
	struct mt_analog_ephy_attr ephy_attr;

	mt_analog_get_attr(MT_ANA_INDEX_EPHY, (void*)&ephy_attr);

	DP_LOG("-----------------------[EPHY]--------------------\n");
	DP_LOG("clkgen_ephy_reg[%8lX]: %08X\n", REG_CLKGEN_EPHY, MT_IO_READ32(REG_CLKGEN_EPHY));

	DP_LOG("                 data ext: 0x%02X\n", ephy_attr.data_ext);
	DP_LOG("                 data sel: %u\n", ephy_attr.data_sel);
	DP_LOG("                  div sel: %u\n", ephy_attr.div_sel);
	DP_LOG("             clk edge sel: %u\n", ephy_attr.clk_edge_sel);
	DP_LOG("                    i sel: %u\n", ephy_attr.i_sel);
	DP_LOG("           dco ctrl valid: %u\n", ephy_attr.dco_ctrl_valid);
	DP_LOG("         pd clk ethpll d2: %u\n", ephy_attr.pd_ethpll_d2);
	DP_LOG("         pd clk ethpll d3: %u\n", ephy_attr.pd_ethpll_d3);
	DP_LOG("      pd clk ethpll d3p25: %u\n", ephy_attr.pd_ethpll_d3p25);
	DP_LOG("         pd clk ethpll d5: %u\n", ephy_attr.pd_ethpll_d5);
	DP_LOG("                      mux: %u\n", ephy_attr.mux);
	DP_LOG("                div7 duty: %u\n", ephy_attr.div7_duty);
	DP_LOG("           pd clk div 4p5: %u\n", ephy_attr.pd_clk_div_4p5);
	DP_LOG("          ephy ana enable: %u\n", ephy_attr.ephy_ana_en);
	DP_LOG("        clk test sel ephy: %u\n", ephy_attr.clk_tst_sel_ephy);
	DP_LOG("         clk test sel dig: %u\n", ephy_attr.clk_tst_sel_dig);
	DP_LOG("         clk test en ephy: %u\n", ephy_attr.clk_tst_en_ephy);
}

#endif

