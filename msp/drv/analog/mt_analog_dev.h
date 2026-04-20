/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */
#ifndef __INC__MT_ANALOG_DEV_H__
#define __INC__MT_ANALOG_DEV_H__

/*
 * a: register address
 * s: shift
 * w: width
 * aslk: slock register address
 * alk: lock register address
 * slk: slock/lock register shift
 */
#define MT_INIT_ANA(a, s, w, aslk, alk, slk) \
					MT_INIT_OBJLK(a, s, w, aslk, slk, 1, alk, slk, 1, NULL)

/* Analog Module(Dev) Definition */
struct mt_ana_dev
{
	const char *name;

	unsigned int power_down: 1;	/* power down flag, 1: power down, 0: enable */

	mt_io_objlk_t stat;
};

extern struct mt_ana_dev mt_ana_table[];
extern u32 mt_ana_table_size;

extern mt_reg_value_t suspend_analog_regs[];
extern u32 suspend_analog_regs_size;

/* internal functions */
int _analog_get_status(struct mt_ana_dev *pAna, u32 *status);
int _analog_enable(struct mt_ana_dev *pAna);
int _analog_disable(struct mt_ana_dev *pAna);

#endif

