/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_OTPREG25_H__
#define __INC_MT_ANALOG_OTPREG25_H__

/*
 * Usage:
 *     mt_analog_get_attr(MT_ANA_INDEX_OTPREG25, struct mt_analog_otpreg_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_OTPREG25, struct mt_analog_otpreg_attr *);
 */

/* otpreg25_reg: 0xBF5D0130 */
typedef struct mt_analog_otpreg_attr
{
	/* control the output pmos cell: 11:30mA; 10:23mA; 01:23mA; 00:15mA */
	u32 cur_ctrl: 2;			/* bit 0-1 */

	/* control the regulator voltage: 11:2.4; 10:2.5; 01:2.6; 00:2.73 */
	u32 vref_ctrl: 2;			/* bit 2-3 */

	u32 reserved: 28;			/* bit 4-31 */

} mt_analog_otpreg_attr_t;

//---------------------------------------------------------------------------//

static void mt_otpreg_dump_state(void *s)
{
	struct mt_analog_otpreg_attr otpreg_attr;

	mt_analog_get_attr(MT_ANA_INDEX_OTPREG25, (void*)&otpreg_attr);

	DP_LOG("----------------------[OTPREG]-------------------\n");
	DP_LOG("otpreg25_reg[%8lX]: %08X\n", REG_OTPREG25, MT_IO_READ32(REG_OTPREG25));

	DP_LOG("   cur_ctrl: %u\n", otpreg_attr.cur_ctrl);
	DP_LOG("  vref_ctrl: %u\n", otpreg_attr.vref_ctrl);
}

#endif

