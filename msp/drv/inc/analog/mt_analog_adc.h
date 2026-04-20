/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_ADC_H__
#define __INC_MT_ANALOG_ADC_H__

/*
 * Usage:
 *     mt_analog_get_attr(MT_ANA_INDEX_CADC, struct mt_analog_cadc_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_CADC, struct mt_analog_cadc_attr *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_SADC_R0, struct mt_analog_sadc_r0_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_SADC_R0, struct mt_analog_sadc_r0_attr *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_SADC_R1, struct mt_analog_sadc_r1_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_SADC_R1, struct mt_analog_sadc_r1_attr *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_ADCLOG, struct mt_analog_adclog_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_ADCLOG, struct mt_analog_adclog_attr *);
 */

/* cadc_reg0: 0xBF5D00D0 */
typedef struct mt_analog_cadc_attr
{
	/*
	 * saradc seedout phase invert or not
	 * 0: for not(default)
	 */
	u32 seedinv_ctr: 1;				/* bit 0 */
	/*
	 * gate saradc input clock or not
	 * 0: disable clock
	 * 1: enable clock
	 */
	u32 clkin_en: 1;				/* bit 1 */
	/*
	 * saradc calibration mode input opamp low current control
	 * 0: high current
	 * 1: low current
	 */
	u32 cal_low_cur: 1;				/* bit 2 */
	/*
	 * saradc calibration mode input clock divider selection
	 * 0: bypass
	 * 1: div2
	 */
	u32 cal_div_sel: 1;				/* bit 3 */
	/*
	 * delay margin control when async loop auto adapt to conversion period
	 * 3: min delay
	 * 0: max delay
	 */
	u32 ofdet_dlyctr: 2;			/* bit 4-5 */
	/*
	 * delay selection for internal sar clk
	 * 3: min
	 * 0: max
	 */
	u32 mainclk_dly_ctr: 2;			/* bit 6-7 */

	/*
	 * control for full scale single-end vpp
	 * 111: 1.05V
	 * 110: 1V
	 * 101: 0.95V
	 * 100: 0.9V
	 * 011: 0.85V
	 * 010: 0.8V
	 * 001: 0.75V
	 * 000: 0.7V
	 */
	u32 vref: 3;					/* bit 8-10 */
	/*
	 * control input common mode voltage when saradc in sample phase
	 * 0.7*(
	 *  111: 0.9V
	 *  110: 0.85V
	 *  101: 0.8V
	 *  100: 0.75V
	 *  011: 0.7V
	 *  010: 0.65V
	 *  001: 0.6V
	 *  000: 0.55V)
	 */
	u32 vcm: 3;						/* bit 11-13 */
	/*
	 * reference generation bias current control
	 * 11: 70uA
	 * 10: 60uA
	 * 01: 50uA
	 * 00: 40uA
	 */
	u32 cur: 2;						/* bit 14-15 */

	/*
	 * control for saradc input common mode bias enable or not
	 * 1: input AC couple
	 * 0: input DC couple
	 */
	u32 vcm_en: 1;					/* bit 16 */
	/*
	 * saradc clock generation non overlap time selection
	 * 0: min
	 * 1: max
	 */
	u32 novctr: 1;					/* bit 17 */
	/*
	 * control for latch input common mode voltage
	 * 0: high voltage about 860mV
	 * 1: low voltage about 590mV
	 */
	u32 cmlat_ctr: 1;				/* bit 18 */
	/*
	 * control for saradc clkout invert phase or not
	 * 3/2: invert
	 * 0/1: not invert
	 */
	u32 buf_dly_ctr: 2;				/* bit 19-20 */
	/*
	 * control for delay cells of saradc async loop
	 * 0: auto adaption to conversion period
	 * 1: register setting
	 */
	u32 dlyloop_sel: 1;				/* bit 21 */
	/*
	 * start control for calibration delay cells of saradc async loop adapt to conversion period
	 * 0: reset
	 * 1: calibration start
	 */
	u32 ofdet: 1;					/* bit 22 */
	/*
	 * control for preamp gain change between conversion steps
	 * 1: dynamic enable
	 * 0: disable
	 */
	u32 dly_dynamic_en: 1;			/* bit 23 */

	/*
	 * control for delay cells of saradc async loop when saradc_dlyloop_sel=1
	 * 0: max delay
	 * 7: min delay
	 */
	u32 async_dly_ext: 3;			/* bit 24-26 */
	/* latch preclock delay control
	 * 0: for max delay
	 * 3: for min delay
	 */
	u32 preclk_dly_ext: 2;			/* bit 27-28 */
	/* conversion steps selection
	 * 1: 12 steps
	 * 0: 14 steps
	 */
	u32 conversion_steps_ctr: 1;	/* bit 29 */
	/* boost clock low voltage bias selection
	 * 1: 0V
	 * 0: 0.5V
	 */
	u32 boost_lv_sel: 1;			/* bit 30 */
	/* preamp gain changed step selection when saradc_dly_dynamic_en=1
	 * 1: changed at 6 step
	 * 0: changed at 8 step
	 */
	u32 dyna_count: 1;				/* bit 31 */

} mt_analog_cadc_attr_t;

/* sadc_reg0: 0xBF5D00C8 */
typedef struct mt_analog_sadc_r0_attr
{
	u32 clk_en: 1;			/* bit 0, enable input clock */
	u32 vcm_en: 1;			/* bit 1, enable internal common level of analog input */
	u32 vcm_ctr: 3;			/* bit 2-4, control common level of analog input */
	u32 vref_ctr: 3;		/* bit 5-7, control fullscale of adc */

	u32 reserved1: 1;		/* bit 8, soft reset */

	u32 reserved2: 1;		/* bit 9 */

	u32 clk_edge_ctr: 1;	/* bit 10, control edge of output clock */
	u32 clk_delay_ctr: 2;	/* bit 11-12, control delay of output clock */
	u32 lsb_en: 1;			/* bit 13, enable lsb data */
	u32 delay_ctr: 2;		/* bit 14-15, control delay of sarlogic loop */

	u32 refbuf_ctr: 3;		/* bit 16-18, control ref buf current */
	u32 comp_ctr: 2;		/* bit 19-20, control comparator current */
	u32 comp_mode: 1;		/* bit 21, enable fixed delay of comparator */
	u32 div_en: 1;			/* bit 22, divide input clock by 2 */
	u32 shbuf_ctr: 2;		/* bit 23-24, control sh buf current */
	u32 cal_ctr: 6;			/* bit 25-30, set range of offset calibration */
	u32 delay_ctr2: 1;		/* bit 31, control delay of sarlogic loop */

} mt_analog_sadc_r0_attr_t;

/* sadc_reg1: 0xBF5D00CC */
typedef struct mt_analog_sadc_r1_attr
{
	u32 cal_ext_en: 1;			/* bit 0, enable external setting of offset calibration */

	u32 cal_ext: 18;			/* bit 1-18, external setting of offset calibration */

	u32 reserved: 13;			/* bit 19-31 */

} mt_analog_sadc_r1_attr_t;

/* adc_log: 0xBF5D00D4 */
typedef struct mt_analog_adclog_attr
{
	u32 cal_done: 1;			/* bit 0, offset calibration done */

	u32 cal_data: 18;			/* bit 1-18, offset calibration data */

	u32 saradc_overflow: 1;		/* bit 19, overflow log for calibration delay cells of saradc async loop */
	u32 saradc_dlylog: 3;		/* bit 20-22, delay cells calibration result of saradc async loop */

	u32 reserved: 9;			/* bit 23-31 */

} mt_analog_adclog_attr_t;

#ifndef NO_DUMP_ADC_STATE
static void mt_adc_dump_state(void *s)
{
	struct mt_analog_cadc_attr cadc_attr;
	struct mt_analog_sadc_r0_attr sadc_r0_attr;
	struct mt_analog_sadc_r1_attr sadc_r1_attr;
	struct mt_analog_adclog_attr adclog_attr;

	mt_analog_get_attr(MT_ANA_INDEX_CADC, (void*)&cadc_attr);
	mt_analog_get_attr(MT_ANA_INDEX_SADC_R0, (void*)&sadc_r0_attr);
	mt_analog_get_attr(MT_ANA_INDEX_SADC_R1, (void*)&sadc_r1_attr);
	mt_analog_get_attr(MT_ANA_INDEX_ADCLOG, (void*)&adclog_attr);

	DP_LOG("-----------------------[CADC]--------------------\n");
	DP_LOG("        cadc_reg0[%8lX]: %08X\n", CADC_REG0, MT_IO_READ32(CADC_REG0));
	DP_LOG("         saradc_seedinv_ctr: %u\n", cadc_attr.seedinv_ctr);
	DP_LOG("            saradc_clkin_en: %u\n", cadc_attr.clkin_en);
	DP_LOG("         saradc_cal_low_cur: %u\n", cadc_attr.cal_low_cur);
	DP_LOG("         saradc_cal_div_sel: %u\n", cadc_attr.cal_div_sel);
	DP_LOG("        saradc_ofdet_dlyctr: %u\n", cadc_attr.ofdet_dlyctr);
	DP_LOG("     saradc_mainclk_dly_ctr: %u\n", cadc_attr.mainclk_dly_ctr);
	DP_LOG("                saradc_vref: %u\n", cadc_attr.vref);
	DP_LOG("                 saradc_vcm: %u\n", cadc_attr.vcm);
	DP_LOG("                 saradc_cur: %u\n", cadc_attr.cur);
	DP_LOG("              saradc_vcm_en: %u\n", cadc_attr.vcm_en);
	DP_LOG("              saradc_novctr: %u\n", cadc_attr.novctr);
	DP_LOG("           saradc_cmlat_ctr: %u\n", cadc_attr.cmlat_ctr);
	DP_LOG("         saradc_buf_dly_ctr: %u\n", cadc_attr.buf_dly_ctr);
	DP_LOG("         saradc_dlyloop_sel: %u\n", cadc_attr.dlyloop_sel);
	DP_LOG("               saradc_ofdet: %u\n", cadc_attr.ofdet);
	DP_LOG("      saradc_dly_dynamic_en: %u\n", cadc_attr.dly_dynamic_en);
	DP_LOG("       saradc_async_dly_ext: %u\n", cadc_attr.async_dly_ext);
	DP_LOG("      saradc_preclk_dly_ext: %u\n", cadc_attr.preclk_dly_ext);
	DP_LOG("saradc_conversion_steps_ctr: %u\n", cadc_attr.conversion_steps_ctr);
	DP_LOG("        saradc_boost_lv_sel: %u\n", cadc_attr.boost_lv_sel);
	DP_LOG("          saradc_dyna_count: %u\n", cadc_attr.dyna_count);

	DP_LOG("-----------------------[SADC]--------------------\n");
	DP_LOG("sadc_reg0[%8lX]: %08X\n", SADC_REG0, MT_IO_READ32(SADC_REG0));
	DP_LOG("         clk_en: %u\n", sadc_r0_attr.clk_en);
	DP_LOG("         vcm_en: %u\n", sadc_r0_attr.vcm_en);
	DP_LOG("        vcm_ctr: %u\n", sadc_r0_attr.vcm_ctr);
	DP_LOG("       vref_ctr: %u\n", sadc_r0_attr.vref_ctr);
	DP_LOG("   clk_edge_ctr: %u\n", sadc_r0_attr.clk_edge_ctr);
	DP_LOG("  clk_delay_ctr: %u\n", sadc_r0_attr.clk_delay_ctr);
	DP_LOG("         lsb_en: %u\n", sadc_r0_attr.lsb_en);
	DP_LOG("      delay_ctr: %u\n", sadc_r0_attr.delay_ctr);
	DP_LOG("     refbuf_ctr: %u\n", sadc_r0_attr.refbuf_ctr);
	DP_LOG("       comp_ctr: %u\n", sadc_r0_attr.comp_ctr);
	DP_LOG("      comp_mode: %u\n", sadc_r0_attr.comp_mode);
	DP_LOG("         div_en: %u\n", sadc_r0_attr.div_en);
	DP_LOG("      shbuf_ctr: %u\n", sadc_r0_attr.shbuf_ctr);
	DP_LOG("        cal_ctr: 0x%X\n", sadc_r0_attr.cal_ctr);
	DP_LOG("     delay_ctr2: %u\n", sadc_r0_attr.delay_ctr2);

	DP_LOG("\nsadc_reg1[%8lX]: %08X\n", SADC_REG1, MT_IO_READ32(SADC_REG1));
	DP_LOG("  cal_ext_en: %u\n", sadc_r1_attr.cal_ext_en);
	DP_LOG("     cal_ext: 0x%X\n", sadc_r1_attr.cal_ext);

	DP_LOG("\nadc_log[%8lX]: %08X\n", REG_ADC_LOG, MT_IO_READ32(REG_ADC_LOG));
	DP_LOG("          cal_done: %u\n", adclog_attr.cal_done);
	DP_LOG("          cal_data: 0x%X\n", adclog_attr.cal_data);
	DP_LOG("   saradc_overflow: %u\n", adclog_attr.saradc_overflow);
	DP_LOG("     saradc_dlylog: %u\n", adclog_attr.saradc_dlylog);

}
#endif

#endif

