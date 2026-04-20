/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_PLL_H__
#define __INC_MT_ANALOG_PLL_H__

/*
 * PLL
 *
 * Usage:
 *     mt_analog_get_attr(MT_ANA_INDEX_ADCPLL, struct mt_analog_adcpll_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_ADCPLL, struct mt_analog_adcpll_attr *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_USBPLL, mt_analog_usbpll_attr_t *);
 *     mt_analog_set_attr(MT_ANA_INDEX_USBPLL, mt_analog_usbpll_attr_t *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_VHDPLL, mt_analog_vhdpll_attr_t *);
 *     mt_analog_set_attr(MT_ANA_INDEX_VHDPLL, mt_analog_vhdpll_attr_t *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_VSDPLL, struct mt_analog_adcpll_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_VSDPLL, struct mt_analog_adcpll_attr *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_REFPLL, mt_analog_refpll_attr_t *);
 *     mt_analog_set_attr(MT_ANA_INDEX_REFPLL, mt_analog_refpll_attr_t *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_USB3P0PLL, mt_analog_usb3p0pll_attr_t *);
 *     mt_analog_set_attr(MT_ANA_INDEX_USB3P0PLL, mt_analog_usb3p0pll_attr_t *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_EPHYPLL, mt_analog_ephypll_attr_t *);
 *     mt_analog_set_attr(MT_ANA_INDEX_EPHYPLL, mt_analog_ephypll_attr_t *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_DDRPLL, mt_analog_ddrpll_attr_t *);
 *     mt_analog_set_attr(MT_ANA_INDEX_DDRPLL, mt_analog_ddrpll_attr_t *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_ARMPLL, struct mt_analog_armpll_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_ARMPLL, struct mt_analog_armpll_attr *);
 */

/* general pll structure */
typedef struct mt_analog_gen_pll_attr
{
	u32 ctr_i: 3;					/* bit 0-2, pll charge pump, i = 10u + ctr_i*10u */
	/*
	 * pll calibration enable
	 * 1: kvco is calibration results
	 * 0: kvco is vco_ext i.e<31:30> of this reg
	 */
	u32 cal_en: 1;					/* bit 3 */

	u32 div_cal: 4;					/* bit 4-7, pllcal div = 2 + div_cal */

	u32 div_front: 5;				/* bit 8-12 */

	u32 pre_div: 1;					/* bit 13, pre div intp sel, 0: div4, 1: div2 */

	u32 reserved14: 1; 				/* bit 14, soft reset */
	u32 ctr_dly: 1;					/* bit 15 */

	u32 div_fb: 8;					/* bit 16-23 */
	u32 cpvr_sel: 2;				/* bit 24-25 */
	/*
	 * pll vco cal time control
	 * 00: 64*7 clk_cal period
	 * 01: 64*15 clk_cal period
	 * 10: 64*31 clk_cal period
	 * 11: 64*63 clk_cal period
	 */
	u32 vco_cal: 2;					/* bit 26-27 */
	u32 vco_ext: 2;					/* bit 28-29, ext kvco sel */
	/*
	 * vco sel
	 * 00: 1.9G
	 * 01: 1.7G
	 * 10: 1.5G
	 * 11: 1.3G
	 */
	u32 vco_sel: 2;					/* bit 30-31 */

} mt_analog_gen_pll_attr_t;

/* clkgen_usbpll_reg: 0xBF5D0054 */
typedef mt_analog_gen_pll_attr_t mt_analog_usbpll_attr_t;

/* clkgen_vhdpll_reg: 0xBF5D0058 */
typedef mt_analog_gen_pll_attr_t mt_analog_vhdpll_attr_t;

/* clkgen_ephypll_reg: 0xBF5D0118 */
typedef mt_analog_gen_pll_attr_t mt_analog_ephypll_attr_t;

/* usb3p0_pll_reg: 0xBF5D01A4 */
typedef mt_analog_gen_pll_attr_t mt_analog_usb3p0pll_attr_t;

/* clkgen_usbaclk_reg1: 0xBF5D0124 */
typedef mt_analog_gen_pll_attr_t mt_analog_refpll_attr_t;

/* !clkgen_ddrpll_reg: 0xBF090070 */
typedef mt_analog_gen_pll_attr_t mt_analog_ddrpll_attr_t;

/*
 * clkgen_cpupll_reg(clkgen_adcpll_reg): 0xBF5D0048
 *
 * bit 4-7, & bit 13
 *
 * different with struct mt_analog_gen_pll_attr.
 * <CNComment>与struct mt_analog_gen_pll_attr不同.
 */
typedef struct mt_analog_adcpll_attr
{
	u32 ctr_i: 3;					/* bit 0-2, pll charge pump, i = 10u + ctr_i*10u */
	/*
	 * pll calibration enable
	 * 1: kvco is calibration results
	 * 0: kvco is vco_ext i.e<31:30> of this reg
	 */
	u32 cal_en: 1;					/* bit 3 */

	u32 pd_cadc_270M: 1;			/* bit 4, pd 270M for c-adc */
	u32 pd_cdemod_81M: 1;			/* bit 5, pd 81M for c-demod */
	u32 pd_sadc_1100M: 1;			/* bit 6, pd clk 1000M~1350M for s-adc (1350M) */
	u32 pd_sdemod_270M: 1;			/* bit 7, pd drv0_clk (source in cpu pll) for s-demod (270M) */

	u32 div_front: 5;				/* bit 8-12 */

	u32 cadc_clk_sel: 1;			/* bit 13, 0: clk cadc 144M. 1: clk cadc 270M. */

	u32 reserved14: 1; 				/* bit 14, soft reset */
	u32 ctr_dly: 1;					/* bit 15 */

	u32 div_fb: 8;					/* bit 16-23 */
	u32 cpvr_sel: 2;				/* bit 24-25 */

	u32 clk_rng_fix_en: 1;			/* bit 26 */
	u32 reserved27: 1;				/* bit 27 */

	u32 vco_ext: 2;					/* bit 28-29 */
	u32 vco_sel: 2;					/* bit 30-31 */

} mt_analog_adcpll_attr_t;

/*
 * clkgen_vsdpll_reg: 0xBF5D0060
 *
 * bit 4-7, & bit 13
 *
 * different with struct mt_analog_gen_pll_attr.
 * <CNComment>与struct mt_analog_gen_pll_attr不同.
 */
typedef struct mt_analog_vsdpll_attr
{
	u32 ctr_i: 3;					/* bit 0-2, pll charge pump, i = 10u + ctr_i*10u */
	/*
	 * pll calibration enable
	 * 1: kvco is calibration results
	 * 0: kvco is vco_ext i.e<31:30> of this reg
	 */
	u32 cal_en: 1;					/* bit 3 */

	/*
	 * feedback div setting
	 * 1111: hdmi clk*1.0
	 * 1000: hdmi clk*1.25/hdmi clk*1.5
	 */
	u32 fb_div: 4;					/* bit 4-7 */

	u32 div_front: 5;				/* bit 8-12 */

	u32 reserved13: 1;				/* bit 13 */

	u32 reserved14: 1; 				/* bit 14, soft reset */
	u32 ctr_dly: 1;					/* bit 15 */

	u32 div_fb: 8;					/* bit 16-23 */
	u32 reserved2: 2;				/* bit 24-25 */
	/*
	 * pll vco cal time control
	 * 00: 64*7 clk_cal period
	 * 01: 64*15 clk_cal period
	 * 10: 64*31 clk_cal period
	 * 11: 64*63 clk_cal period
	 */
	u32 vco_cal: 2;					/* bit 26-27 */
	u32 vco_ext: 2;					/* bit 28-29, ext kvco sel */
	/*
	 * vco sel
	 * 00: 1.9G
	 * 01: 1.7G
	 * 10: 1.5G
	 * 11: 1.3G
	 */
	u32 vco_sel: 2;					/* bit 30-31 */

} mt_analog_vsdpll_attr_t;

/*
 * !clkgen_armpll_reg: 0xBF508108
 *
 * bit 4-7, bit 13, bit 26-27
 *
 * different with struct mt_analog_gen_pll_attr.
 * <CNComment>与struct mt_analog_gen_pll_attr不同.
 */
typedef struct mt_analog_armpll_attr
{
	u32 ctr_i: 3;					/* bit 0-2, pll charge pump, i = 10u + ctr_i*10u */
	/*
	 * pll calibration enable
	 * 1: kvco is calibration results
	 * 0: kvco is vco_ext i.e<31:30> of this reg
	 */
	u32 cal_en: 1;					/* bit 3 */

	/*
	 * sel cpu clk
	 * 0xxx: clk_fix_dig
	 * 100x: clk ref
	 * 101x: vco/3
	 * 1100: vco/1
	 * 1101: vco/2
	 * 1110: vco/4
	 * 1111: vco/8
	 */
	u32 clk_cpu_sel: 4;				/* bit 4-7 */

	u32 div_front: 5;				/* bit 8-12 */

	u32 dc_tst_en: 1;				/* bit 13, dc test en */

	u32 reserved14: 1; 				/* bit 14, soft reset */
	u32 ctr_dly: 1;					/* bit 15 */

	u32 div_fb: 8;					/* bit 16-23 */
	u32 cpvr_sel: 2;				/* bit 24-25 */
	/*
	 * when dc test en = 1
	 * 00: vref bottom
	 * 01: vref top
	 * 10: vdd vco
	 * 11: vcon
	 */
	u32 dc_tst_sel: 2;				/* bit 26-27 */

	u32 vco_ext: 2;					/* bit 28-29, ext kvco sel */
	/*
	 * vco sel
	 * 00: 3G
	 * 01: 2G
	 * 10: 1.6G
	 * 11: 1.0G
	 */
	u32 vco_sel: 2;					/* bit 30-31 */

} mt_analog_armpll_attr_t;

//---------------------------------------------------------------------------//

#ifndef NO_DUMP_PLL_STATE

static void dump_gen_pll_state(void *s, const char *title, const char *reg_name, unsigned long reg, mt_analog_gen_pll_attr_t *attr)
{
	DP_LOG("--------------------[%s]--------------------\n", title);
	DP_LOG("%s[%8lX]: %08X\n", reg_name, reg, MT_IO_READ32(reg));

	DP_LOG("                      ctr_i: %u\n", attr->ctr_i);
	DP_LOG("                     cal_en: %u\n", attr->cal_en);
	DP_LOG("                    div_cal: 0x%X\n", attr->div_cal);
	DP_LOG("                  div_front: 0x%X\n", attr->div_front);
	DP_LOG("           pre div intp sel: %u\n", attr->pre_div);
	DP_LOG("                    ctr_dly: %u\n", attr->ctr_dly);
	DP_LOG("                     div_fb: 0x%X\n", attr->div_fb);
	DP_LOG("                   cpvr_sel: %u\n", attr->cpvr_sel);

	DP_LOG("                    vco_cal: %u\n", attr->vco_cal);
	DP_LOG("                    vco_ext: %u\n", attr->vco_ext);
	DP_LOG("                    vco_sel: %u\n", attr->vco_sel);
}

static void mt_ana_pll_dump_state(void *s)
{
	struct mt_analog_adcpll_attr adcpll_attr;
	struct mt_analog_vsdpll_attr vsdpll_attr;
	struct mt_analog_armpll_attr armpll_attr;

	mt_analog_usbpll_attr_t usbpll_attr;
	mt_analog_vhdpll_attr_t vhdpll_attr;
	mt_analog_ephypll_attr_t ephypll_attr;
	mt_analog_usb3p0pll_attr_t usb3p0pll_attr;
	mt_analog_refpll_attr_t refpll_attr;
	mt_analog_ddrpll_attr_t ddrpll_attr;

	mt_analog_get_attr(MT_ANA_INDEX_ADCPLL, &adcpll_attr);
	mt_analog_get_attr(MT_ANA_INDEX_VSDPLL, &vsdpll_attr);

	mt_analog_get_attr(MT_ANA_INDEX_USBPLL, &usbpll_attr);
	mt_analog_get_attr(MT_ANA_INDEX_VHDPLL, &vhdpll_attr);
	mt_analog_get_attr(MT_ANA_INDEX_EPHYPLL, &ephypll_attr);
	mt_analog_get_attr(MT_ANA_INDEX_USB3P0PLL, &usb3p0pll_attr);
	mt_analog_get_attr(MT_ANA_INDEX_REFPLL, &refpll_attr);
	mt_analog_get_attr(MT_ANA_INDEX_DDRPLL, &ddrpll_attr);

	mt_analog_get_attr(MT_ANA_INDEX_ARMPLL, &armpll_attr);

	DP_LOG("---------------------[ADCPLL]--------------------\n");
	DP_LOG("clkgen_adcpll_reg[%8lX]: %08X\n", REG_CLKGEN_ADCPLL, MT_IO_READ32(REG_CLKGEN_ADCPLL));
	DP_LOG("                        ctr_i: %u\n", adcpll_attr.ctr_i);
	DP_LOG("                       cal_en: %u\n", adcpll_attr.cal_en);
	DP_LOG("            pd 270M for c-adc: %u\n", adcpll_attr.pd_cadc_270M);
	DP_LOG("           pd 81M for c-demod: %u\n", adcpll_attr.pd_cdemod_81M);
	DP_LOG(" pd clk 1000M~1350M for s-adc: %u\n", adcpll_attr.pd_sadc_1100M);
	DP_LOG("  pd drv0 clk 270M for sdemod: %u\n", adcpll_attr.pd_sdemod_270M);
	DP_LOG("                    div_front: 0x%X\n", adcpll_attr.div_front);
	DP_LOG("                 cadc clk sel: %u\n", adcpll_attr.cadc_clk_sel);
	DP_LOG("                      ctr_dly: %u\n", adcpll_attr.ctr_dly);
	DP_LOG("                       div_fb: 0x%X\n", adcpll_attr.div_fb);
	DP_LOG("                     cpvr_sel: %u\n", adcpll_attr.cpvr_sel);
	DP_LOG("               clk_rng_fix_en: %u\n", adcpll_attr.clk_rng_fix_en);
	DP_LOG("                      vco_ext: %u\n", adcpll_attr.vco_ext);
	DP_LOG("                      vco_sel: %u\n", adcpll_attr.vco_sel);

	dump_gen_pll_state(s, "USBPLL", "clkgen_usbpll_reg", REG_CLKGEN_USBPLL, &usbpll_attr);
	dump_gen_pll_state(s, "VHDPLL", "clkgen_vhdpll_reg", REG_CLKGEN_VHDPLL, &vhdpll_attr);

	DP_LOG("---------------------[VSDPLL]--------------------\n");
	DP_LOG("clkgen_vsdpll_reg[%8lX]: %08X\n", REG_CLKGEN_VSDPLL, MT_IO_READ32(REG_CLKGEN_VSDPLL));
	DP_LOG("                      ctr_i: %u\n", vsdpll_attr.ctr_i);
	DP_LOG("                     cal_en: %u\n", vsdpll_attr.cal_en);
	DP_LOG("       feedback div setting: 0x%X\n", vsdpll_attr.fb_div);
	DP_LOG("                  div_front: 0x%X\n", vsdpll_attr.div_front);
	DP_LOG("                    ctr_dly: %u\n", vsdpll_attr.ctr_dly);
	DP_LOG("                     div_fb: 0x%X\n", vsdpll_attr.div_fb);
	DP_LOG("                    vco_cal: %u\n", vsdpll_attr.vco_cal);
	DP_LOG("                    vco_ext: %u\n", vsdpll_attr.vco_ext);
	DP_LOG("                    vco_sel: %u\n", vsdpll_attr.vco_sel);

	dump_gen_pll_state(s, "REFPLL", "clkgen_usbaclk_reg1", CLKGEN_USBACLK_REG1, &refpll_attr);
	dump_gen_pll_state(s, "USB3P0PLL", "usb3p0_pll_reg", REG_USB3P0_PLL, &usb3p0pll_attr);
	dump_gen_pll_state(s, "EPHYPLL", "clkgen_ephypll_reg", REG_CLKGEN_EPHYPLL, &ephypll_attr);
	dump_gen_pll_state(s, "DDRPLL", "clkgen_ddrpll_reg", REG_DDR_PHY_BASE+0x70, &ddrpll_attr);

	DP_LOG("---------------------[ARMPLL]--------------------\n");
	DP_LOG("clkgen_armpll_reg[%8lX]: %08X\n", REG_APCPU_ARMPLL, MT_IO_READ32(REG_APCPU_ARMPLL));
	DP_LOG("                        ctr_i: %u\n", armpll_attr.ctr_i);
	DP_LOG("                       cal_en: %u\n", armpll_attr.cal_en);
	DP_LOG("                  clk_cpu_sel: 0x%X\n", armpll_attr.clk_cpu_sel);
	DP_LOG("                    div_front: 0x%X\n", armpll_attr.div_front);
	DP_LOG("                    dc_tst_en: %u\n", armpll_attr.dc_tst_en);
	DP_LOG("                      ctr_dly: %u\n", armpll_attr.ctr_dly);
	DP_LOG("                       div_fb: 0x%X\n", armpll_attr.div_fb);
	DP_LOG("                     cpvr_sel: %u\n", armpll_attr.cpvr_sel);
	DP_LOG("                   dc_tst_sel: %u\n", armpll_attr.dc_tst_sel);
	DP_LOG("                      vco_ext: %u\n", armpll_attr.vco_ext);
	DP_LOG("                      vco_sel: %u\n", armpll_attr.vco_sel);

	//DP_LOG("-------------------------------------------------\n");
}
#endif

#endif

