/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_clock.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : MT clock internal header file.
 * History        :
 * 1.Date         : 2022/06/29
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_MT_CLOCK_H__
#define __INC_MT_CLOCK_H__

/* mt_clock.h for CRM */

#define MT_INIT_GATE(a, s, aslk, alk, slk) \
					MT_INIT_OBJLK(a, s, 1, aslk, slk, 1, alk, slk, 1, NULL)

#define MT_INIT_MUX(a, s, w, e, aslk, alk, slk) \
					MT_INIT_OBJLK(a, s, w, aslk, slk, 1, alk, slk, 1, e)

#define MT_INIT_RATE			MT_INIT_MUX
#define MT_INIT_DIV				MT_INIT_MUX

#define MT_INIT_FIXED_DIV(d, p)	.fixed_div.value = d, .fixed_div.parent = p

#define MT_INIT_ATTR(a, s, w, aslk, alk, slk) \
					MT_INIT_OBJLK(a, s, w, aslk, slk, 1, alk, slk, 1, NULL)

/* MUX/RATE/DIV/ATTR flags */
#define FLAG_CLK_NONE			0
/*#define FLAG_CLK_GATE			0x01*/
#define FLAG_CLK_MUX			0x02
#define FLAG_CLK_RATE			0x04
#define FLAG_CLK_DIV			0x08
#define FLAG_CLK_ATTR			0x10
#define FLAG_CLK_FIXED_RATE		0x20
#define FLAG_CLK_FIXED_DIV		0x40

/* active low, or power down flag */
#define FLAG_CLK_GATE_ACTIVE_LOW	0x100

/* mux/rate/divider map table */
struct mt_clk_map_table
{
	u32 key;				/* register value */
	unsigned long value;	/* mapped value, such as: rate, divider */

	const char *parent;
};

/* Clock Definition */
struct mt_clk_in
{
	const char *name;
	unsigned int flags;			/* MUX/RATE/DIV/ATTR flags */

	mt_io_objlk_t gate;

	union
	{
		unsigned long fixed_rate;
		struct mt_clk_map_table fixed_div;

		/* MUX/RATE/DIV */
		mt_io_objlk_t rate;
		mt_io_objlk_t mux;
		mt_io_objlk_t div;
		mt_io_objlk_t comb;		/* combination */
	};

	/* extra attribute */
	mt_io_objlk_t attr;
};

extern struct mt_clk_in mt_clk_table[];
extern u32 mt_clk_table_size;

extern mt_io_param_t mt_autogate_params[];
extern u32 mt_autogate_params_size;

extern mt_io_param_t mt_autogate_params_a0[];
extern u32 mt_autogate_params_size_a0;

extern mt_io_param_t mt_autogate_params_rd[];

extern struct mt_clk_in mt_top_clk_table[];
extern u32 mt_top_clk_table_size;

extern struct mt_clk_in mt_analog_clk_table[];
extern u32 mt_analog_clk_table_size;

extern mt_reg_value_t suspend_clock_regs[];
extern u32 suspend_clock_regs_size;

void patch_clk_enable(mt_clk_t *clk);
void patch_clk_disable(mt_clk_t *clk);

#endif

