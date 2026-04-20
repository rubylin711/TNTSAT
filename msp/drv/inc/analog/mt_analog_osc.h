/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_SEC_OSC_H__
#define __INC_MT_ANALOG_SEC_OSC_H__

/*
 * Usage:
 *     mt_analog_get_attr(MT_ANA_INDEX_OSC, struct mt_analog_osc_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_OSC, struct mt_analog_osc_attr *);
 */

/* sec_osc_reg: 0xBF5D00F4 */
typedef struct mt_analog_osc_attr
{
	u32 wait_count: 4;			/* bit 0-3, wait for number cycle(clko/1024) for sel_clk stable */

	u32 cal_start: 1;			/* bit 4, calibration begin signal, from external register */
	u32 rst_n: 1;				/* bit 5, async rst_n, and not sync to any internal clk */
	u32 sel_cal: 1;				/* bit 6, select calibration data, 1: from external register, 0: from internal calibration data. */
	u32 sel_xtal: 1;			/* bit 7, 1: the xtal is 27MHz, 0: xtal is 24M */

	u32 ctr_ext: 6;				/* bit 8-13 */

} mt_analog_osc_attr_t;

//---------------------------------------------------------------------------//

static void mt_osc_dump_state(void *s)
{
	struct mt_analog_osc_attr osc_attr;

	mt_analog_get_attr(MT_ANA_INDEX_OSC, (void*)&osc_attr);

	DP_LOG("---------------------[SEC OSC]-------------------\n");
	DP_LOG("sec_osc_reg[%8lX]: %08X\n", REG_SEC_OSC, MT_IO_READ32(REG_SEC_OSC));

	DP_LOG("  wait_count: 0x%X\n", osc_attr.wait_count);
	DP_LOG("   cal_start: %u\n", osc_attr.cal_start);
	DP_LOG("       rst_n: %u\n", osc_attr.rst_n);
	DP_LOG("     sel_cal: %u\n", osc_attr.sel_cal);
	DP_LOG("    sel_xtal: %u\n", osc_attr.sel_xtal);
	DP_LOG("     ctr_ext: 0x%X\n", osc_attr.ctr_ext);
}

#endif

