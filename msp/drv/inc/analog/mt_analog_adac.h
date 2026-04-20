/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_ADAC_H__
#define __INC_MT_ANALOG_ADAC_H__

/*
 * Usage:
 *     mt_analog_get_attr(MT_ANA_INDEX_ADAC, struct mt_analog_adac_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_ADAC, struct mt_analog_adac_attr *);
 */

/* adac_sw_reg: 0xBF5D0000 */
typedef struct mt_analog_adac_attr
{
	/*
	 * ensure SC filter to start up
	 * 0x: normal operation
	 * 10: easy to re-startup when unstable state
	 * 11: startup mode
	 */
	u32 scsw: 2;					/* bit 0-1 */

	/*
	 * Phase Inversion for Right Channel
	 * 0: Do not swap
	 * 1: Swap phase1 and phase2
	 */
	u32 phinv_rch: 1;				/* bit 2 */

	/*
	 * Phase Inversion for Left Channel
	 * 0: Do not swap
	 * 1: Swap phase1 and phase2
	 */
	u32 phinv_lch: 1;				/* bit 3 */

#if 0
//A0
	/*
	 * power supply level
	 *     Vref  Vdda
	 * 00: 1.25V 2.5V
	 * 01: 1.3V  2.6V
	 * 10: 1.35V 2.7V
	 * 11: 1.4V  2.8V
	 */
	u32 vsel: 2;					/* bit 4-5 */
#else
//A1
	/*
	 * A1: bit 29 + bit 5-4
	 * power supply level
	 *	    Vref  Vdda
	 * 100  1.2V  2.4V
	 * 000: 1.25V 2.5V
	 * 001: 1.3V  2.6V
	 * 010: 1.35V 2.7V
	 * 011: 1.4V  2.8V
	 * 111: 1.45V 2.9V
	 */
	u32 vsel: 2;					/* bit 4-5 */
#endif

	/*
	 * oscillation frequency of the charge-pump
	 * 0: 200 MHz
	 * 1: 340 MHz
	 */
	u32 hifreq: 1;					/* bit 6 */

	u32 selfreg: 1;					/* bit 7 */

	/*
	 * FIR Filter
	 * 0: Enable the analog FIR filter
	 * 1: Disable the analog FIR filter
	 */
	u32 fir_dis: 1;					/* bit 8 */

	/*
	 * adac mode sel
	 * 0: 0dB gain
	 * 1: -5dB gain
	 */
	u32 mode_sel: 1;				/* bit 9 */

	/*
	 * regulator gate resistor option(adjust MOS gate voltage)
	 * 0: 82kohm
	 * 1: 28kohm (increase 0dBu Rcfilter current)
	 */
	u32 gate_sel: 1;				/* bit 10 */

	/*
	 * adac reference option
	 * 0: reference from slave regulator
	 * 1: reference from resistor divider
	 */
	u32 ref_sel: 1;					/* bit 11 */

	/*
	 * power detect option
	 * 00: 3.2-3.4V
	 * 01: <3.2V
	 * 10: >3.4V
	 */
	u32 vdda_dtct: 2;				/* bit 12-13 */

	/*
	 * vdda detection data enable option
	 * 0: data disable(default 3.3V)
	 * 1: data enable
	 */
	u32 en_vdda_dtct: 1;			/* bit 14 */

	//A0
	//u32 reserved15: 1;				/* bit 15 */
	//A1
	/*
	 * remove pop noise from 1bit pcm
	 * 0: startup mode
	 * 1: normal
	 */
	u32 buf_pcm_start: 1;			/* bit 15 */

	/*
	 * 2Vrms buffer current loading option
	 * 00: off (for remove startup pop noise)
	 * 01: 2X (default 10kohm resistor)
	 * 10: 4X
	 * 11: 6X (dc couple 700ohm resistance loading)
	 */
	u32 buf_sel_out: 2;				/* bit 16-17 */

	/*
	 * oscillation frequency of the charge-pump in negative 1p2 voltage generation cp
	 * 0: 320 MHz
	 * 1: 420 MHz
	 */
	u32 buf_sel_cp: 1;				/* bit 18 */

	/*
	 * clock select
	 * 0: internal charge pump
	 * 1: external 480MHz
	 */
	u32 sel_500MHz: 1;				/* bit 19 */

	/*
	 * buf clock enable signal
	 * <2> 0: X1
	 * <2> 1: X2
	 * <1:0> 00: disable
	 * <1:0> 01: flying cap discharge
	 * <1:0> 10: startup status for low startup current
	 * <1:0> 11: normal operation
	 */
	//u32 buf_en_clk: 3;				/* bit 20-22 */
	u32 buf_en_clk: 2;				/* bit 20-21 */
	u32 buf_en_clk_X: 1;			/* bit 22 */

	/*
	 * remove pop noise when vdd2p8 startup works
	 * <2> 0: remove P part pop noise
	 * <2> 1: normal operation
	 * <1> 0: remove N part pop noise
	 * <1> 1: normal operation
	 * <0> 0: normal operation
	 * <0> 1: assisted vddb2p8 works
	 */
	//u32 buf_start: 3;				/* bit 23-25 */
	u32 buf_start_vddb2p8: 1;		/* bit 23 */
	u32 buf_start_N: 1;				/* bit 24 */
	u32 buf_start_P: 1;				/* bit 25 */

#if 0
//A0
	/*
	 * when set high, 2Vrms buffer input current on
	 * <3> Lch 2.5uA
	 * <2> Rch 2.5uA
	 * <1> Lch 50uA
	 * <0> Rch 50uA
	 */
	u32 enb_svpw: 4;				/* bit 26-29 */
#else
//A1
	/*
	 * when set high, 2Vrms buffer input current on
	 * <2> L&Rch 2.5uA
	 * <1> Lch 50uA
	 * <0> Rch 50uA
	 */
	u32 enb_svpw: 3;				/* bit 26-28 */

	/* vsel bit2, see: bit 4-5 */
	u32 vsel2: 1;					/* bit 29 */
#endif

	/*
	 * vgate mos cap bottom plate set different voltage
	 * 1: 0.9V
	 * 0: 2.9V
	 */
	u32 rst_vcbp: 1;				/* bit 30 */

	//A0
	//u32 reserved31: 1;				/* bit 31 */
	//A1
	/*
	 * remove pop noise from sc filter output
	 * 0: startup mode
	 * 1: normal
	 */
	u32 scfilter_start: 1;			/* bit 31 */

} mt_analog_adac_attr_t;

//---------------------------------------------------------------------------//

#ifndef NO_DUMP_ADAC_STATE
static void mt_adac_dump_state(void *s)
{
	struct mt_analog_adac_attr adac_attr;

	mt_analog_get_attr(MT_ANA_INDEX_ADAC, (void*)&adac_attr);

	DP_LOG("-----------------------[ADAC]--------------------\n");
	DP_LOG("adacphy_sw_reg0[%8lX]: %08X\n", ADACPHY_SW_REG0, MT_IO_READ32(ADACPHY_SW_REG0));
	DP_LOG("adacphy_sw_reg1[%8lX]: %08X\n", ADACPHY_SW_REG1, MT_IO_READ32(ADACPHY_SW_REG1));
	DP_LOG("adacphy_sw_reg2[%8lX]: %08X\n", ADACPHY_SW_REG2, MT_IO_READ32(ADACPHY_SW_REG2));
	DP_LOG("adacphy_sw_reg3[%8lX]: %08X\n", ADACPHY_SW_REG3, MT_IO_READ32(ADACPHY_SW_REG3));

	DP_LOG("\nadac_sw_reg[%8lX]: %08X\n", REG_ADAC_SW, MT_IO_READ32(REG_ADAC_SW));
	DP_LOG("                 scsw: %u\n", adac_attr.scsw);
	DP_LOG("            phinv rch: %u\n", adac_attr.phinv_rch);
	DP_LOG("            phinv lch: %u\n", adac_attr.phinv_lch);
	DP_LOG("                 vsel: %u\n", adac_attr.vsel);
	DP_LOG("               hifreq: %u\n", adac_attr.hifreq);
	DP_LOG("              selfreg: %u\n", adac_attr.selfreg);
	DP_LOG("          FIR_disable: %u\n", adac_attr.fir_dis);
	DP_LOG("             mode sel: %u\n", adac_attr.mode_sel);
	DP_LOG("             gate sel: %u\n", adac_attr.gate_sel);
	DP_LOG("              ref sel: %u\n", adac_attr.ref_sel);
	DP_LOG("            vdda dtct: %u\n", adac_attr.vdda_dtct);
	DP_LOG("         en vdda dtct: %u\n", adac_attr.en_vdda_dtct);

	//A1+
	DP_LOG("        buf pcm start: %u\n", adac_attr.buf_pcm_start);

	DP_LOG("          buf sel out: %u\n", adac_attr.buf_sel_out);
	DP_LOG("           buf sel cp: %u\n", adac_attr.buf_sel_cp);

	DP_LOG("           sel 500MHz: %u\n", adac_attr.sel_500MHz);
	DP_LOG("           buf en clk: %u\n", adac_attr.buf_en_clk);
	DP_LOG("         buf en clk X: %u\n", adac_attr.buf_en_clk_X);
	DP_LOG("    buf start vddb2p8: %u\n", adac_attr.buf_start_vddb2p8);
	DP_LOG("       buf start NMOS: %u\n", adac_attr.buf_start_N);
	DP_LOG("       buf start PMOS: %u\n", adac_attr.buf_start_P);
	DP_LOG("             enb svpw: %u\n", adac_attr.enb_svpw);

	//A1+
	DP_LOG("                vsel2: %u\n", adac_attr.vsel2);

	DP_LOG("             rst vcbp: %u\n", adac_attr.rst_vcbp);

	//A1+
	DP_LOG("       scfilter start: %u\n", adac_attr.scfilter_start);

	//DP_LOG("-------------------------------------------------\n");
}
#endif

#endif

