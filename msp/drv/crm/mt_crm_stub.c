/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage LZ Technology Group Limited and its affiliated companies      */
/********************************************************************************************/
#ifdef __UBOOT__
#include <common.h>
#include <asm/arch-symphony6/mt_common.h>
#elif defined(__KERNEL__)
#include <linux/types.h>
#include "mt_io.h"
#include "mt_drv_clock.h"
#endif

#include "mt_clock.h"
#include "mt_mod_reset.h"

/* stub tables */

struct mt_clk_in mt_clk_table[] = {};
u32 mt_clk_table_size = sizeof(mt_clk_table)/sizeof(mt_clk_table[0]);

struct mt_clk_in mt_analog_clk_table[] = {};
u32 mt_analog_clk_table_size = sizeof(mt_analog_clk_table)/sizeof(mt_analog_clk_table[0]);

struct mt_clk_in mt_top_clk_table[] = {};
u32 mt_top_clk_table_size = sizeof(mt_top_clk_table)/sizeof(mt_top_clk_table[0]);

struct mt_mod_rst_in mt_rst_modules[] = {};
u32 mt_rst_modules_count = sizeof(mt_rst_modules) / sizeof(mt_rst_modules[0]);

void mt_top_clk_enable_all(void) {}

void mt_top_clk_hw_auto_gate_enable(void) {}

