/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_USB_H__
#define __INC_MT_ANALOG_USB_H__

//TODO

#ifndef NO_DUMP_USB_STATE
static void mt_usb_dump_state(void *s)
{
	//TODO
	DP_LOG("----------------------[USB]----------------------\n");
	DP_LOG("   usb0_sw_reg0[%8lX]: %08X\n", USB0_REG0, MT_IO_READ32(USB0_REG0));
	DP_LOG("   usb0_sw_reg1[%8lX]: %08X\n", USB0_REG1, MT_IO_READ32(USB0_REG1));
	DP_LOG("   usb1_sw_reg0[%8lX]: %08X\n", USB1_REG0, MT_IO_READ32(USB1_REG0));
	DP_LOG("   usb1_sw_reg1[%8lX]: %08X\n", USB1_REG1, MT_IO_READ32(USB1_REG1));

	DP_LOG("usb0phy_sw_reg0[%8lX]: %08X\n", USB0PHY_SW_REG0, MT_IO_READ32(USB0PHY_SW_REG0));
	DP_LOG("usb0phy_sw_reg1[%8lX]: %08X\n", USB0PHY_SW_REG1, MT_IO_READ32(USB0PHY_SW_REG1));
	DP_LOG("usb1phy_sw_reg0[%8lX]: %08X\n", USB1PHY_SW_REG0, MT_IO_READ32(USB1PHY_SW_REG0));
	DP_LOG("usb1phy_sw_reg1[%8lX]: %08X\n", USB1PHY_SW_REG1, MT_IO_READ32(USB1PHY_SW_REG1));

	DP_LOG("    usb_imp_reg[%8lX]: %08X\n", REG_USB_IMP, MT_IO_READ32(REG_USB_IMP));
	DP_LOG("      usb_logic[%8lX]: %08X\n", REG_USB_LOGIC, MT_IO_READ32(REG_USB_LOGIC));
}
#endif

#endif

