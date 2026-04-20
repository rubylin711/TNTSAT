/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_BIAS_H__
#define __INC_MT_ANALOG_BIAS_H__

/*
 * Usage:
 *     mt_analog_get_attr(MT_ANA_INDEX_BIAS, struct mt_analog_bias_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_BIAS, struct mt_analog_bias_attr *);
 */

/* bias_reg: 0xBF5D0138 */
typedef struct mt_analog_bias_attr
{
	u32 vdd1p8: 3;					/* bit 0-2, vdd1p8 voltage control */
	u32 vdd1p5: 3;					/* bit 3-5, vdd1p5 voltage control */
	u32 vdd0p9: 3;					/* bit 6-8, vdd0p9 voltage control */

	u32 vdd1p35: 3;					/* bit 9-11, vdd1p35 voltage control */
	u32 vdd1p0: 3;					/* bit 12-14, vdd1p0 voltage control */

	u32 cp_sel: 1;					/* bit 15, regulator charge-pump frequence select */

	u32 en_chopping_bg: 1;			/* bit 16, bandgap chopping enable */
	u32 reserved1: 1;				/* bit 17 */
	u32 bg_psel_pm: 2;				/* bit 18-19, iph current adjustment code. 0: 1X 01 or 10: add 5% 11: add 10% */
	u32 bg_rsel: 1;					/* bit 20, short res for test leakage with code 1 */
	u32 bg_pm_sel: 1;				/* bit 21, process monitor test reference select. 0: 1V and 1: 0.75V */

	u32 reserved2: 10;				/* bit 22-31 */

} mt_analog_bias_attr_t;

//---------------------------------------------------------------------------//

#ifndef NO_DUMP_BIAS_STATE
static void mt_bias_dump_state(void *s)
{
	struct mt_analog_bias_attr bias_attr;

	mt_analog_get_attr(MT_ANA_INDEX_BIAS, (void*)&bias_attr);

	DP_LOG("-----------------------[BIAS]--------------------\n");
	DP_LOG("bias_reg[%8lX]: %08X\n", REG_BIAS, MT_IO_READ32(REG_BIAS));
	DP_LOG("            vdd1p8: %u\n", bias_attr.vdd1p8);
	DP_LOG("            vdd1p5: %u\n", bias_attr.vdd1p5);
	DP_LOG("            vdd0p9: %u\n", bias_attr.vdd0p9);
	DP_LOG("           vdd1p35: %u\n", bias_attr.vdd1p35);
	DP_LOG("            vdd1p0: %u\n", bias_attr.vdd1p0);
	DP_LOG("            cp sel: %u\n", bias_attr.cp_sel);
	DP_LOG("    en chopping bg: %u\n", bias_attr.en_chopping_bg);
	DP_LOG("        bg psel pm: %u\n", bias_attr.bg_psel_pm);
	DP_LOG("           bg rsel: %u\n", bias_attr.bg_rsel);
	DP_LOG("         bg pm sel: %u\n", bias_attr.bg_pm_sel);

	//DP_LOG("-------------------------------------------------\n");
}
#endif

#endif

