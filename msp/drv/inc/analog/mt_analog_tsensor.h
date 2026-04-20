/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_TSENSOR_H__
#define __INC_MT_ANALOG_TSENSOR_H__

/*
 * Usage:
 *     mt_analog_get_attr(MT_ANA_INDEX_TSENSOR, struct mt_analog_tsensor_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_TSENSOR, struct mt_analog_tsensor_attr *);
 */

/* tsensor_reg: 0xBF5D00EC */
typedef struct mt_analog_tsensor_attr
{
	u32 reserved1: 1;				/* bit 0 */

	u32 en_div: 1;					/* bit 1, chopper clock divider power, 0:disable div 8192, 1:enable div 8192 */
	u32 clk_sel: 1;					/* bit 2, chopper clock sel, 0:ph1/ph2, 1:(ph1/ph2)/8192 */
	u32 en_gen: 1;					/* bit 3, Dvbe generator DEM power down, 0:disable, 1:enable */

	u32 en_adc: 1;					/* bit 4, adc DEM power down, 0:disable, 1:enable */
	u32 reserved2: 1;				/* bit 5, reset */
	u32 pd_startup: 1;				/* bit 6, startup power down, 0:enable the startup circuit, 1:disable */
	u32 en_boostn: 1;				/* bit 7, adc opamp boostn power down, 0:disable, 1:enable */

	u32 en_boostp: 1;				/* bit 8, adc opamp boostp power down, 0:disable, 1:enable */
	u32 input_div: 5;				/* bit 9-13, input clock divider, 0: div2, 11011: div11 */
	u32 en_output: 1;				/* bit 14, output data power down, 0:disable, 1:enable */
	u32 dclk_edge_sel: 1;			/* bit 15, tsensor dclk edge sel, 0:falling edge, 1:rising edge */

	/*
	 * 000:70deg
	 * 001:80deg
	 * 010:90deg
	 * 011:95deg
	 * 100:100deg
	 * 101:105deg
	 * 110:110deg
	 * 111:120deg
	 */
	u32 ref_sel: 3;					/* bit 16-18, temperature reference select */
	u32 intr_reset: 1;				/* bit 19, interrupt compare result reset, high active */

	u32 reserved3: 12;				/* bit 20-31 */

} mt_analog_tsensor_attr_t;

//---------------------------------------------------------------------------//

static void mt_tsensor_dump_state(void *s)
{
	struct mt_analog_tsensor_attr tsensor_attr;

	mt_analog_get_attr(MT_ANA_INDEX_TSENSOR, (void*)&tsensor_attr);

	DP_LOG("---------------------[TSENSOR]-------------------\n");
	DP_LOG("    TSENSOR_TH_REG[%8lX]: %08X\n", REG_TSENSOR_TH, MT_IO_READ32(REG_TSENSOR_TH));
	DP_LOG("       TSENSOR_INT[%8lX]: %08X\n", REG_TSENSOR_INT, MT_IO_READ32(REG_TSENSOR_INT));
	//DP_LOG("       TSENSOR_REG[%8lX]: %08X\n", REG_TSENSOR, MT_IO_READ32(REG_TSENSOR));
	DP_LOG("      TSENSOR_DATA[%8lX]: %08X\n", REG_TSENSOR_DOUT, MT_IO_READ32(REG_TSENSOR_DOUT));
	DP_LOG("TSENSOR_RST_REQ_TH[%8lX]: %08X\n", REG_TSENSOR_RST_REQ_TH, MT_IO_READ32(REG_TSENSOR_RST_REQ_TH));

	DP_LOG("\ntsensor_reg[%8lX]: %08X\n", REG_TSENSOR, MT_IO_READ32(REG_TSENSOR));
	DP_LOG("                     en div 8192: %u\n", tsensor_attr.en_div);
	DP_LOG("                         clk sel: %u\n", tsensor_attr.clk_sel);
	DP_LOG("                en generator DEM: %u\n", tsensor_attr.en_gen);
	DP_LOG("                      en adc DEM: %u\n", tsensor_attr.en_adc);
	//DP_LOG("                           reset: %u\n", tsensor_attr.reset);
	DP_LOG("              pd startup circuit: %u\n", tsensor_attr.pd_startup);
	DP_LOG("             en adc opamp boostn: %u\n", tsensor_attr.en_boostn);
	DP_LOG("             en adc opamp boostp: %u\n", tsensor_attr.en_boostp);
	DP_LOG("                   input clk div: %X\n", tsensor_attr.input_div);
	DP_LOG("                  en output data: %u\n", tsensor_attr.en_output);
	DP_LOG("                   dclk edge sel: %u\n", tsensor_attr.dclk_edge_sel);
	DP_LOG("              temp reference sel: %u\n", tsensor_attr.ref_sel);
	DP_LOG("  interrupt compare result reset: %u\n", tsensor_attr.intr_reset);

	//DP_LOG("-------------------------------------------------\n");
}

#endif

