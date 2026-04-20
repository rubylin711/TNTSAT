/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_PMON_H__
#define __INC_MT_ANALOG_PMON_H__

/*
 * Process Monitor
 *
 * Usage:
 *     mt_analog_get_attr(MT_ANA_INDEX_PMON, struct mt_analog_pmon_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_PMON, struct mt_analog_pmon_attr *);
 */

/* pm_sw_reg0: 0xBF5D00D8 */
typedef struct mt_analog_pmon_attr
{
	u32 cali_mode: 2;		/* bit 0-1, calibration mode, 10:0.5vref 11:vbg 0X:normal */
	u32 r_mode: 1;			/* bit 2, test range, 1:0.36V-2.16V, 0:0.3V-1.8V */
	u32 f_mode: 1;			/* bit 3, test function, 1:analog dc test, 0:vdd */

	u32 item_mode: 1;		/* bit 4, corner test enable, 1:corner test 0:normal mode */
	u32 read_en: 1;			/* bit 5, read vddmonitor result, 1:read, 0:normal */

	/*
	 * item corner select
	 * 000: res corner
	 * 001: NMOS Vth variation with L=0.1u
	 * 010: NMOS Vth variation with L=0.035u
	 * 011: NMOS LVTth variation with L=0.03u
	 * 100: PMOS Vth variation with L=0.035u
	 * 101: PMOS LVth variation with L=0.03u
	 */
	u32 item_sel: 4; 		/* bit 6-9 */

	u32 rmode_sel: 1;		/* bit 10, change range mode select, 0: auto switch range, 1: set test range */
	u32 clk_sel: 1;			/* bit 11, clock frequency select, 0:5MHz, 1:2.5MHz */

	u32 reserved: 20; 		/* bit 12-31 */

} mt_analog_pmon_attr_t;

//---------------------------------------------------------------------------//

static void mt_pmon_dump_state(void *s)
{
	struct mt_analog_pmon_attr pmon_attr;

	mt_analog_get_attr(MT_ANA_INDEX_PMON, (void*)&pmon_attr);

	DP_LOG("---------------------[PROCMON]-------------------\n");
	DP_LOG("pm_sw_reg0[%8lX]: %08X\n", PM_SW_REG0, MT_IO_READ32(PM_SW_REG0));
	DP_LOG("           cali_mode: %u\n", pmon_attr.cali_mode);
	DP_LOG("              r_mode: %u\n", pmon_attr.r_mode);
	DP_LOG("              f_mode: %u\n", pmon_attr.f_mode);
	DP_LOG("           item_mode: %u\n", pmon_attr.item_mode);
	DP_LOG("             read_en: %u\n", pmon_attr.read_en);
	DP_LOG("            item_sel: %X\n", pmon_attr.item_sel);
	DP_LOG("           rmode_sel: %u\n", pmon_attr.rmode_sel);
	DP_LOG("             clk_sel: %u\n", pmon_attr.clk_sel);

	//DP_LOG("-------------------------------------------------\n");
}

#endif

