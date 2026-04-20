/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2020, Montage-LZ Technology Co., Ltd.
 *
 * File Name      : pinctrl.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2020/9/22
 * Description    : PinCtrl functions definition.
 * History        :
 * 1.Date         : 2020/9/22
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_PINCTRL_H__
#define __INC_PINCTRL_H__

#if defined(__KERNEL__) && !defined(__UBOOT__)
#include "mach/symphony_reg_base_addr.h"

#define REG_SW_PIN_CTRL_BASE		SYMPHONY_IO_VA(0xBF13C000)
#define REG_AO_PIN_CTRL_BASE		SYMPHONY_IO_VA(0xBF15B000)
#endif

/* 3bit at most */
#define CFG_MAX_FMUX_COUNT			8

/* PINMUX structure definition */
struct pinmux_in
{
	PINMUX_INDEX_E index;				/* PIN Index */
	const char *name;					/* PIN Name */
	unsigned int offset;				/* Reg Offset */

/* Symphony6 do NOT support switch OD/CMOS! */
#if 0
	/* PIN type, 1: OD, 0: CMOS */
	struct
	{
		unsigned char shift;
		unsigned char width;
	} type;
#endif

	/* sw_io_ctrl */
	struct
	{
		unsigned char shift;
		unsigned char width;
	} io_ctrl;

	/* sw_pin_sel: function sel */
	struct
	{
		unsigned char shift;
		unsigned char width;
	} sel;

	/* function mux */
	const char *fmux_name[CFG_MAX_FMUX_COUNT];
};

/* pinmux group with same reg base */
struct pinmux_group
{
	union
	{
		unsigned long reg_base;
		unsigned long reg_mapped;
	};
	unsigned int reg_size;

	unsigned int pin_count;
	struct pinmux_in *pin_tab;
};

/* ARCH related global variable and functions */
extern unsigned int g_pin_group_count;
extern struct pinmux_group g_pin_group[];

/* API */
#if defined(__KERNEL__) && !defined(__UBOOT__)
void pinctrl_dump_state(struct seq_file *s);
#else
void pinctrl_dump_state(void);
#endif

#endif

