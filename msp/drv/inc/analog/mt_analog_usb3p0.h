/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_USB3P0_H__
#define __INC_MT_ANALOG_USB3P0_H__

//TODO

#ifndef NO_DUMP_USB3P0_STATE
static void mt_usb3p0_dump_state(void *s)
{
	//TODO
	DP_LOG("--------------------[USB3.0]---------------------\n");
	DP_LOG("usb3p0_ssc_reg1[%8lX]: %08X\n", USB3P0_SSC_REG1, MT_IO_READ32(USB3P0_SSC_REG1));
	DP_LOG("usb3p0_ssc_reg2[%8lX]: %08X\n", USB3P0_SSC_REG2, MT_IO_READ32(USB3P0_SSC_REG2));

	DP_LOG("usb3p0_reg0[%8lX]: %08X\n", USB3P0_REG0, MT_IO_READ32(USB3P0_REG0));
	DP_LOG("usb3p0_reg1[%8lX]: %08X\n", USB3P0_REG1, MT_IO_READ32(USB3P0_REG1));
	DP_LOG("usb3p0_reg2[%8lX]: %08X\n", USB3P0_REG2, MT_IO_READ32(USB3P0_REG2));
	DP_LOG("usb3p0_reg3[%8lX]: %08X\n", USB3P0_REG3, MT_IO_READ32(USB3P0_REG3));
	DP_LOG("usb3p0_reg4[%8lX]: %08X\n", USB3P0_REG4, MT_IO_READ32(USB3P0_REG4));

	DP_LOG("usb3p0_rxcal_reg_l[%8lX]: %08X\n", REG_USB3P0_RXCAL_L, MT_IO_READ32(REG_USB3P0_RXCAL_L));
	DP_LOG("usb3p0_rxcal_reg_h[%8lX]: %08X\n", REG_USB3P0_RXCAL_H, MT_IO_READ32(REG_USB3P0_RXCAL_H));
	DP_LOG("  usb3p0_power_reg[%8lX]: %08X\n", REG_USB3P0_POWER, MT_IO_READ32(REG_USB3P0_POWER));
}
#endif

#endif

