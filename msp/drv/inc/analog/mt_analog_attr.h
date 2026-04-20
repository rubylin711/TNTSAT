/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Montage LZ Co., Ltd.
 */
#ifndef __INC_MT_ANALOG_ATTR_H__
#define __INC_MT_ANALOG_ATTR_H__

/* Attribute structures of Analog modules */

/* ana_ao_reg0: 0xBF157000 */
typedef struct mt_analog_ao_r0_attr
{
	u32 bias_ao_en: 1;				/* bit 0 */
	u32 pd_usb0: 1; 				/* bit 1 */
	u32 pd_usb1: 1; 				/* bit 2 */
	u32 pd_icx: 1;					/* bit 3 */

	u32 reserved1: 2;				/* bit 4-5 */
	u32 pd_vdac: 1; 				/* bit 6 */
	u32 reserved2: 1;				/* bit 7: hdmi tx reset */

	u32 pd_hdmi_tx_ch0: 1;			/* bit 8 */
	u32 pd_hdmi_tx_ch1: 1;			/* bit 9 */
	u32 pd_hdmi_tx_ch2: 1;			/* bit 10 */
	u32 pd_hdmi_tx_clk_ch: 1;		/* bit 11 */

	/*
	 * ADAC
	 *  2Vrms:
	 *    bit 12: pd_buf_rch
	 *    bit 15: pd_vddb1p2
	 *    bit 16: pd_buf_lch
	 *  0dBu:
	 *    bit 13: pd_adac_rch
	 *    bit 17: pd_adac_lch
	 */
	//u32 adac_buf_power_ctr: 8;		/* bit 12-19 */
	u32 pd_buf_rch: 1;				/* bit 12 */
	u32 pd_adac_rch: 1;				/* bit 13 */
	//A0
	//u32 reserved3: 1;				/* bit 14 */
	//A1
	u32 pd_1bit_int: 1;
	u32 pd_vddb1p2: 1;				/* bit 15 */
	u32 pd_buf_lch: 1;				/* bit 16 */
	u32 pd_adac_lch: 1;				/* bit 17 */
	u32 reserved4: 1;				/* bit 18 */
	//A0
	//u32 reserved5: 1;				/* bit 19 */
	//A1
	u32 pd_internal_vdd2p7: 1;

	u32 pd_cadc: 1; 				/* bit 20 */
	u32 pd_sadc: 1; 				/* bit 21 */
	u32 pd_proc_mon: 1; 			/* bit 22, process monitor */
	u32 reserved6: 1;				/* bit 23, process monitor reset */

	u32 pd_rng1: 1; 				/* bit 24 */
	u32 pd_usb_imp_cal: 1;			/* bit 25 */
	u32 dreg_dc_tst_en: 1; 			/* bit 26 */
	u32 pd_tsensor: 1;				/* bit 27, temp sensor */

	u32 adac_output_pull_down: 1;	/* bit 28 */
	u32 pd_rng2: 1; 				/* bit 29 */
	/*
	 * [0]: pd_pll_ephy
	 * [1]: pd_intp_ephy
	 */
	//u32 pd_ephy_pll: 2; 			/* bit 30-31 */
	u32 pd_pll_ephy: 1; 			/* bit 30 */
	u32 pd_intp_ephy: 1; 			/* bit 31 */

} mt_analog_ao_r0_attr_t;

/* ana_ao_reg1: 0xBF157004 */
typedef struct mt_analog_ao_r1_attr
{
	//u32 pd_otp_reg: 1;				/* bit 0 */
	u32 reserved0: 1;

	u32 pd_vdac_det: 1; 			/* bit 1 */
	u32 adac_mute: 1;				/* bit 2 */
	//u32 adac_buf_en_clk: 1; 		/* bit 3 */
	u32 reserved3: 1;

	u32 pd_master_regulator: 1;		/* bit 4 */
	u32 pd_usb3_phy: 1; 			/* bit 5 */
	u32 pd_usb3_pll: 2; 			/* bit 6-7 */

	u32 reserved8: 8;				/* bit 8-15 */

	/*
	 * [18:16] control regulator voltage (50mv/bit)
	 *   111: 0.9
	 *   100: 1.05
	 *   010: 1.15
	 *   000: 1.25
	 */
	//u32 dig_ao_regulator_ctr: 4;	/* bit 16-19, digital always on regulator(dreg) ctr */
	u32 dreg_volt: 3;				/* bit 16-18 */
	u32 dreg_ext_en: 1;				/* bit 19, enable the extral current load */

	u32 xtal_ext_en: 1; 			/* bit 20 */

	u32 pd_ref_pll: 1;				/* bit 21 */

	u32 xtal_gain_sel: 2;			/* bit 22-23 */

	u32 pd_vsd_pll: 2;				/* bit 24-25, 24: VSD PLL 1.001 Pi, 25: VSD PLL */
	u32 pd_vhd_pll: 2;				/* bit 26-27, 26: VHD PLL 1.001 Pi, 27: VHD PLL */
	u32 pd_usb_pll: 3;				/* bit 28-30 */
	u32 pd_adc_pll: 1;				/* bit 31 */

} mt_analog_ao_r1_attr_t;

/* ana_top_reg0: 0xBF5D0090 */
typedef struct mt_analog_top_r0_attr
{
	u32 bias_dc_tst_en: 1;			/* bit 0 */
	u32 usb0_dc_tst_en: 1;			/* bit 1 */
	u32 usb1_dc_tst_en: 1;			/* bit 2 */
	u32 adac_dc_tst_en: 1;			/* bit 3 */
	u32 vdac_dc_tst_en: 1;			/* bit 4 */

	u32 reserved1: 2;				/* bit 5-6 */

	u32 armpll_dc_tst_en: 1;		/* bit 7 */
	u32 sadc_dc_tst_en: 1;			/* bit 8 */

	u32 bias_dc_tst_sel: 3;			/* bit 9-11 */

	u32 hdmi_dc_tst_en: 1;			/* bit 12 */
	u32 cadc_dc_tst_en: 1;			/* bit 13 */
	u32 vddmon_dc_tst_en: 1;		/* bit 14 */
	u32 ddrphy_dc_tst_en: 1;		/* bit 15 */

	u32 adc_domain_dc_tst_sel: 4; 	/* bit 16-19 */
	u32 hdmi_domain_dc_tst_sel: 3; 	/* bit 20-22 */
	u32 usb_domain_dc_tst_sel: 3; 	/* bit 23-25 */
	u32 adac_domain_dc_tst_sel: 3; 	/* bit 26-28 */
	u32 vdac_domain_dc_tst_sel: 3; 	/* bit 29-31 */

} mt_analog_top_r0_attr_t;

/* ana_top_reg1: 0xBF5D0094 */
typedef struct mt_analog_top_r1_attr
{
	/* 0: ADC input, 1: digital input */
	u32 sadc_input_mode: 1;			/* bit 0 */
	u32 reserved1: 1; 				/* bit 1 */
	u32 reserved2: 1;				/* bit 2, adac_reset */
	/* 0: ADC input, 1: digital input */
	u32 cadc_input_mode: 1; 		/* bit 3 */

	u32 clockgen_domain_dc_sel: 12;	/* bit 4-15 */

	u32 ana_tst_io_switch: 1;		/* bit 16 */
	u32 vddcore_sw_en_2_ana_tst: 1;	/* bit 17 */

	//u32 sadc_reg_rsv: 6;			/* bit 18-23 */
	u32 sadc_cal_lpf_enb: 1;		/* bit 18 */
	u32 sadc_cal_clk_reg: 1;		/* bit 19 */
	u32 sadc_cal_clk_sel: 1;		/* bit 20 */
	u32 sadc_cal_mux_sel: 1;		/* bit 21 */
	u32 sadc_clk_cal_ctrl: 1;		/* bit 22 */
	u32 sadc_clk_cal_sel: 1;		/* bit 23 */

	u32 tsensor_tst: 4;				/* bit 24-27, temp sensor tst */

	u32 sec_osc_cal_freq_sel: 1;	/* bit 28 */
	u32 smc_ip_dc_sw_en: 1;			/* bit 29 */
	u32 vdd_pll_dc_sw_en: 1;		/* bit 30 */
	u32 int_ID_read_en: 1;			/* bit 31 */

} mt_analog_top_r1_attr_t;

/* ana_top_reg2: 0xBF5D00F8 */
typedef struct mt_analog_top_r2_attr
{
	//u32 sadc_reg_rsv: 7;			/* bit 0-6 */
	u32 sadc_ctr_buffer: 2;			/* bit 0-1 */
	u32 reserved1: 5;				/* bit 2-6 */

	u32 reserved2: 1;				/* bit 7 */

	//u32 rng1_reg: 11; 				/* bit 8-18 */
	u32 rng1_clkedge_swqp: 1;		/* bit 0, 0:output clk_rng=~clkl, 1:output clk_rng=clkl */
	u32 rng1_div_sel: 2; 			/* bit 1-2, div_sel, 00: internal clk freq div 2, 01: div 4, 10: clk_fix */
	u32 rng1_clk_gate: 1;			/* bit 3, 0: gate output clkt_rng, 1: output clkt_rng */
	u32 rng1_hfast: 1;				/* bit 4, 0:internal clk fast loop disable, 1:internal clk fast loop enable */
	u32 rng1_hslow: 1;				/* bit 5, 0:internal clk slow loop disable, 1:internal clk slow loop enable */
	u32 rng1_clk_sel: 1; 			/* bit 6, 0:digital process xor path length 8, 1:digital process xor path length 4 */
	u32 rng1_reserved: 1;			/* bit 7, reset */
	u32 rng1_rng_out_selbb: 1;		/* bit 8, 0:digital process bypass, 1:digital process used */
	u32 rng1_cp_sr_sel: 1;			/* bit 9, 0:charge pump current X1, 1:charge pump current X2 */
	u32 rng1_pd_doubler: 1;			/* bit 10, 0:doubler enable, 1:doubler disable */

	//u32 rng2_reg: 11; 				/* bit 19-29 */
	u32 rng2_clkedge_swqp: 1;		/* bit 19 */
	u32 rng2_div_sel: 2; 			/* bit 20-21, div_sel, 00: div 2, 01: div 4, 10: clk_fix */
	u32 rng2_clk_gate: 1;			/* bit 3, 0: gate output clkt_rng, 1: output clkt_rng */
	u32 rng2_hfast: 1;				/* bit 23 */
	u32 rng2_hslow: 1;				/* bit 24 */
	u32 rng2_clk_sel: 1; 			/* bit 25 */
	u32 rng2_reserved: 1;			/* bit 26, reset */
	u32 rng2_rng_out_selbb: 1;		/* bit 27 */
	u32 rng2_cp_sr_sel: 1;			/* bit 29 */
	u32 rng2_pd_doubler: 1;			/* bit 29, 0: enable, 1: disable */

	u32 ephy_ref_clk_div_sel: 2;	/* bit 30-31 */

} mt_analog_top_r2_attr_t;

/* ana_top_reg3: 0xBF5D013C */
typedef struct mt_analog_top_r3_attr
{
	u32 adac_rl_data_swap: 1;		/* bit 0 */
	u32 adac_data_buf_edge_sel: 1;	/* bit 1 */
	u32 reserved1: 3; 				/* bit 2-4 */

	u32 ephy_pll_dc_tst_sel: 2;		/* bit 5-6 */
	u32 ephy_pll_dc_tst_en: 1;		/* bit 7 */

	u32 clockgen_dc_tst_en: 6;		/* bit 8-13 */
	//A0
	//u32 reserved2: 6; 				/* bit 14-19 */
	//A1
	u32 adac_pu_1bit_pcm_to_extern: 1;	/* bit 14, pu 1bit pcm to external, 1: normal woark, 0: power down */
	u32 adac_pu_1bit_pcm_to_inter: 1;	/* bit 15, pu 1bit pcm to internal buffer, 1: normal woark, 0: power down */
	u32 adac_dig_pad_oe: 1;				/* bit 16, A1 add digital pad on when ana_top_reg3<14>=0 */
	u32 adac_dig_pad_driving: 2;		/* bit 17-18, driving ability */
	u32 reserved2: 1; 					/* bit 19 */

	u32 usb3_dc_tst_en: 3;			/* bit 20-22 */
	u32 usb3_dc_tst_sel: 4;			/* bit 23-26 */
	//A0
	//u32 reserved3: 5; 				/* bit 27-31 */
	//A1
	u32 cadc_internal_reg_ctrl: 3;	/* bit 27-29, A1 add for cadc, adc 0.9V regulator control
	                                 * 100: for 0.9V
	                                 * 011: for 0.85V
	                                 * 010: for 0.80V
	                                 * 001: for 0.75V
	                                 * 000: for 0.7V
	                                 * 101: for 0.95V
	                                 * 110: for 1.0V
	                                 * 111: for 1.05V
	                                 */
	u32 reserved3: 2;				 /* bit 30-31 */

} mt_analog_top_r3_attr_t;

/* clkgen_pdsys_reg: 0xBF5D009C */
typedef struct mt_analog_pdsys_attr
{
	//u32 pd_clk_usb: 8;				/* bit 0-7 */
	u32 pd_clk_usbpll_d2: 1;		/* bit 0 */
	u32 pd_clk_usbpll_d3: 1;		/* bit 1 */
	u32 pd_clk_usbpll_d4: 1;		/* bit 2 */
	u32 pd_clk_usbpll_d5: 1;		/* bit 3 */
	u32 pd_clk_usbpll_d7: 1;		/* bit 4 */
	u32 pd_clk_usbpll_d11: 1;		/* bit 5 */
	u32 mux: 1;						/* bit 6, 0: div1, 1: div6 */
	u32 div7_duty: 1;				/* bit 7 */

	//u32 pd_clk_vhd: 8;				/* bit 8-15 */
	/*
	 * clkgen_vhd_div clk sel
	 *  0: when hdmi_clk*1.0
	 *  1: when hdmi_clk*1.25/hdmi_clk*1.5
	 */
	u32 vhd_div_sel: 1;				/* bit 8 */
	/*
	 * 0: hdmi_clk 1485M/742.5M
	 * 1: hdmi_clk 1485M*1.25  /1485M*1.5
	 *             742.5M*1.25/742.5M*1.5
	 */
	u32 vhd_clk_sel_1: 1;			/* bit 9 */
	u32 reserved1: 1;				/* bit 10 */
	/*
	 * vhd clk pd
	 * 0: normal mode 1: pd
	 */
	u32 pd_vhd_clk: 1;				/* bit 11 */
	/*
	 * 0x: hdmi_clk 1485M/742.5M*1.0
	 * 10: hdmi_clk 1485M/742.5M*1.25
	 * 11: hdmi_clk 1485M/742.5M*1.5
	 */
	u32 vhd_clk_sel_2: 2;			/* bit 12-13 */
	u32 reserved2: 2;				/* bit 14-15 */

	//u32 pd_clk_vsd: 8;				/* bit 16-23 */
	/*
	 * clkgen_vsd_div clk sel
	 *  0: when hdmi_clk*1.0
	 *  1: when hdmi_clk*1.25/hdmi_clk*1.5
	 */
	u32 vsd_div_sel: 1;				/* bit 16 */
	/*
	 * 0: hdmi_clk 270M 1: hdmi_clk 270M*1.5/270M*1.25
	 */
	u32 vsd_clk_sel_1: 1;			/* bit 17 */
	/*
	 * clk drv0 sel
	 * 0: when hdmi_clk*1.0
	 * 1: when hdmi_clk*1.25/hdmi_clk*1.5
	 */
	u32 vsd_drv0_sel: 1;			/* bit 18 */
	/*
	 * clk drv0, hdmi_clk, vsd clk pd
	 * 0: normal mode, 1: pd
	 */
	u32 pd_vsd_clk: 1;				/* bit 19 */
	/*
	 * 0x: hdmi_clk 270M*1.0
	 * 10: hdmi_clk 270M*1.25
	 * 11: hdmi_clk 270M*1.5
	 */
	u32 vsd_clk_sel_2: 2;			/* bit 20-21 */
	u32 reserved3: 2;				/* bit 22-23 */

	//u32 pd_clkgen_usb: 6;			/* bit 24-29 */
	u32 pd_drv0_clk: 1;				/* bit 24 */
	u32 pd_drv1_clk: 1;				/* bit 25 */
	/* 0: 57.6M, 1: 28.8M */
	u32 drv0_clk_sel: 1;			/* bit 26 */
	/* 0: 192M, 1: 96M */
	u32 drv1_clk_sel: 1;			/* bit 27 */
	u32 pd_usb_div: 1;				/* bit 28 */
	u32 pd_clk_mon_int: 1;			/* bit 29, pd clk mon int in clkgen buf */

	u32 reserved30: 1;				/* bit 30 */
	u32 pd_clk_cal: 1;				/* bit 31, pd clk cal in xtal top */

} mt_analog_pdsys_attr_t;

/* clkgen_test_reg: 0xBF5D0098 */
typedef struct mt_analog_test_attr
{
	u32 clk_test_div: 4;			/* bit 0-3 */

	/*
	 * usb
	 * 0000: clk eth 50M
	 * 0001: clk adc 96M
	 * 0010: clk usb 480M
	 * 0011: clk sys
	 * vsd
	 * 0100: clk sys
	 * 0101: clk sys
	 * 0110: clk vsd 108M
	 * 0111: -
	 * vhd
	 * 1000: clk rng
	 * 1001: -
	 * 1010: clk os, 74.25M, vhdintp_reg<3:0>
	 * 1011: clk hd ana, 74.25M, vsdintp_reg<3:0>
	 *
	 * 1100: clk cpu
	 * 1101: clk cpu
	 * 1110: clk cpu
	 * 1111: clk cpu
	 */
	u32 clk_test_sel: 4;			/* bit 4-7 */

	/*
	 * 000: div3
	 * 001: div4
	 * 010: div5
	 * 011: div7
	 * 100: div9
	 * 101: div11
	 * 110: xx
	 * 111: xx
	 */
	u32 clk_divxx_sel: 3;			/* bit 8-10 */

	u32 clk_test_en: 1;				/* bit 11 */

	//u32 cpu_clk_sel: 4;			/* bit 12-15, apcpll? */
	u32 pd_81M: 1;					/* bit 12 */
	u32 pd_cadc_270M: 1;			/* bit 13 */
	u32 pd_1350M: 1;				/* bit 14 */
	u32 pd_270M: 1;					/* bit 15 */

	//u32 pd_clkgen_vhd: 6;			/* bit 16-21 */
	u32 pd_vhd_inth: 1;				/* bit 16 */
	u32 pd_vhd_pll: 1;				/* bit 17 */
	u32 reserved18: 1;				/* bit 18 */
	u32 pd_vhd_clk: 3;				/* bit 19-21 */

	//u32 pd_clkgen_vsd: 6;			/* bit 22-27 */
	u32 pd_vsd_inth: 1;				/* bit 22 */
	u32 pd_vsd_pll: 1;				/* bit 23 */
	u32 pd_vsd_clk: 4;				/* bit 24-27 */

	/*
	 * 00: div1
	 * 01: div2
	 * 10: div4
	 * 11: div6
	 */
	u32 clkcal_sel_xtal_div: 2;		/* bit 28-29 */

	/*
	 * 00: div1
	 * 01: div2
	 * 10: div4
	 * 11: div6
	 */
	u32 clkcal_sel_usb_div: 2;		/* bit 30-31 */

} mt_analog_test_attr_t;

/* clkgen_ethintp_reg: 0xBF5D004C */
typedef struct mt_analog_ethintp_attr
{
	u32 clk_sel: 1;					/* bit 0, 0: 25M, 1: 50M */

	/* 0: 28.8/57.6, 1: 81M */
	u32 drv0_clk_sel: 1;			/* bit 1 */

	/* pllcpu_clkref_en, 0: disable clk30M, 1: enable clk30M */
	u32 clkref_30M_en: 1;			/* bit 2 */
	/* pllcpu_clkref_sel, 0: xtal, 1: 30M */
	u32 clkref_sel: 1;				/* bit 3 */
	/* 0: 960M, 1: 1100M */
	u32 sadc_clk_sel: 1;			/* bit 4 */

	/* 0: from cpu pll, 1: from vsd pll */
	u32 drv0_clk_81M_sel: 1;		/* bit 5 */
	u32 reserved1: 1;				/* bit 6 */
	/*
	 * 1: for at speed scan mode 270M is from 2700M/10.0,
	 * 0: normal mode 270M=3240/12
	 */
	u32 cadc_clk_270M_sel: 1;		/* bit 7 */

	u32 div_sel: 2;					/* bit 8-9 */
	u32 sw_edge: 1;					/* bit 10, switch output clock edge */
	/*
	 * dpher clock edge sel of intp
	 * 0: negedge
	 * 1: posedge
	 */
	u32 clk_edge_sel: 1;			/* bit 11 */
	u32 i_sel: 3;					/* bit 12-14, current sel of video intp */
	u32 dco_ctrl_valid: 1;			/* bit 15 */

	u32 dco_ctrl_data: 16;			/* bit 16-31 */

} mt_analog_ethintp_attr_t;

/* Enum of Analog modules for get/set attribute */
enum MT_ANALOG_INDEX_E
{
	MT_ANA_INDEX_AO_R0      = (0x157000),			/* Always On, struct mt_analog_ao_r0_attr */
	MT_ANA_INDEX_AO_R1      = (0x157000 + 4),		/* Always On, struct mt_analog_ao_r1_attr */

	MT_ANA_INDEX_TOP_R0     = (0x5D0000 + 0x90),	/* struct mt_analog_top_r0_attr */
	MT_ANA_INDEX_TOP_R1     = (0x5D0000 + 0x94), 	/* struct mt_analog_top_r1_attr */
	MT_ANA_INDEX_TOP_R2     = (0x5D0000 + 0xF8),	/* struct mt_analog_top_r2_attr */
	MT_ANA_INDEX_TOP_R3     = (0x5D0000 + 0x13C),	/* struct mt_analog_top_r3_attr */

	MT_ANA_INDEX_PDSYS      = (0x5D0000 + 0x9C),	/* struct mt_analog_pdsys_attr */
	MT_ANA_INDEX_TEST       = (0x5D0000 + 0x98),	/* struct mt_analog_test_attr */
	MT_ANA_INDEX_ETHINTP    = (0x5D0000 + 0x4C),	/* struct mt_analog_ethintp_attr */

	MT_ANA_INDEX_VHDINTP    = (0x5D0000 + 0x5C),	/* struct mt_analog_vhdintp_attr */
	MT_ANA_INDEX_VSDINTP    = (0x5D0000 + 0x64),	/* struct mt_analog_vsdintp_attr */

	MT_ANA_INDEX_VHDSSC     = (0x5D0000 + 0x140),	/* struct mt_analog_vhdssc_attr */
	MT_ANA_INDEX_VSDSSC     = (0x5D0000 + 0x144),	/* struct mt_analog_vsdssc_attr */
	MT_ANA_INDEX_EPHY       = (0x5D0000 + 0x11C),	/* struct mt_analog_ephy_attr */

	MT_ANA_INDEX_HDMITX_R0  = (0x5D0000 + 0x1BC),	/* struct mt_analog_hdmitx_ch_attr */
	MT_ANA_INDEX_HDMITX_R1	= (0x5D0000 + 0x1C0),	/* struct mt_analog_hdmitx_ch_attr */
	MT_ANA_INDEX_HDMITX_R2  = (0x5D0000 + 0x1C4),	/* struct mt_analog_hdmitx_r2_attr */
	MT_ANA_INDEX_HDMI_TEST_R0 = (0x5D0000 + 0x1C8),	/* struct mt_analog_hdmi_test_r0_attr */

	MT_ANA_INDEX_USBACLK_R0 = (0x5D0000 + 0x120),	/* struct mt_analog_usbaclk_r0_attr */
	MT_ANA_INDEX_USBACLK_R2 = (0x5D0000 + 0x128),	/* struct mt_analog_usbaclk_r2_attr */

	MT_ANA_INDEX_ADAC       = (0x5D0000),			/* struct mt_analog_adac_attr */
	MT_ANA_INDEX_VDAC       = (0x5D0000 + 0x10),	/* struct mt_analog_vdac_attr */

	MT_ANA_INDEX_SADC_R0    = (0x5D0000 + 0xC8),	/* struct mt_analog_sadc_r0_attr */
	MT_ANA_INDEX_SADC_R1    = (0x5D0000 + 0xCC),	/* struct mt_analog_sadc_r1_attr */
	MT_ANA_INDEX_CADC       = (0x5D0000 + 0xD0),	/* struct mt_analog_cadc_attr */
	MT_ANA_INDEX_ADCLOG     = (0x5D0000 + 0xD4),	/* struct mt_analog_adclog_attr */

	MT_ANA_INDEX_BIAS       = (0x5D0000 + 0x138),	/* struct mt_analog_bias_attr */
	MT_ANA_INDEX_OTPREG25   = (0x5D0000 + 0x130),	/* struct mt_analog_otpreg_attr */
	MT_ANA_INDEX_OSC        = (0x5D0000 + 0xF4),	/* struct mt_analog_osc_attr */
	MT_ANA_INDEX_TSENSOR    = (0x5D0000 + 0xEC),	/* struct mt_analog_tsensor_attr */
	MT_ANA_INDEX_PMON       = (0x5D0000 + 0xD8),	/* struct mt_analog_pmon_attr */

	/* PLL */
	MT_ANA_INDEX_ADCPLL     = (0x5D0000 + 0x48), 	/* struct mt_analog_adcpll_attr */
	MT_ANA_INDEX_USBPLL 	= (0x5D0000 + 0x54),	/* mt_analog_usbpll_attr_t */
	MT_ANA_INDEX_VHDPLL 	= (0x5D0000 + 0x58),	/* mt_analog_vhdpll_attr_t */
	MT_ANA_INDEX_VSDPLL 	= (0x5D0000 + 0x60),	/* struct mt_analog_vsdpll_attr */
	MT_ANA_INDEX_EPHYPLL	= (0x5D0000 + 0x118),	/* mt_analog_ephypll_attr_t */
	MT_ANA_INDEX_USB3P0PLL	= (0x5D0000 + 0x1A4),	/* mt_analog_usb3p0pll_attr_t */

	/* CLKGEN_USBACLK_REG1 */
	MT_ANA_INDEX_REFPLL 	= (0x5D0000 + 0x124),	/* mt_analog_refpll_attr_t */

	/* CLKGEN_DDRPLL_REG */
	MT_ANA_INDEX_DDRPLL     = (0x090000 + 0x70),	/* mt_analog_ddrpll_attr_t */

	/* REG_APCPU_ARMPLL */
	MT_ANA_INDEX_ARMPLL 	= (0x500000 + 0x8108),	/* struct mt_analog_armpll_attr */

};

#endif	//__INC_MT_ANALOG_ATTR_H__

