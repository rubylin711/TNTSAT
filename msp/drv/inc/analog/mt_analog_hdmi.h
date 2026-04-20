/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Montage LZ Co., Ltd.
 */

#ifndef __INC_MT_ANALOG_HDMI_H__
#define __INC_MT_ANALOG_HDMI_H__

/*
 * Usage:
 *     mt_analog_get_attr(MT_ANA_INDEX_HDMITX_R0, struct mt_analog_hdmitx_ch_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_HDMITX_R0, struct mt_analog_hdmitx_ch_attr *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_HDMITX_R1, struct mt_analog_hdmitx_ch_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_HDMITX_R1, struct mt_analog_hdmitx_ch_attr *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_HDMITX_R2, struct mt_analog_hdmitx_r2_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_HDMITX_R2, struct mt_analog_hdmitx_r2_attr *);
 *
 *     mt_analog_get_attr(MT_ANA_INDEX_HDMI_TEST_R0, struct mt_analog_hdmi_test_r0_attr *);
 *     mt_analog_set_attr(MT_ANA_INDEX_HDMI_TEST_R0, struct mt_analog_hdmi_test_r0_attr *);
 */

/* hdmi_tx_reg2: 0xBF5D01C4 */
typedef struct mt_analog_hdmitx_r2_attr
{
	u32 main: 4;			/* bit 0-3 */
	u32 ffe_en: 4;			/* bit 4-7 */

	u32 ffe_str: 8;			/* bit 8-15, 20% of main current */

	u32 reg_data_tst: 4;	/* bit 16-19, reg data for test */

	u32 hdmi_r_en: 1;		/* bit 20, 1: termination r enable */
	u32 cur_term_en: 1;		/* bit 21, 1: termination enable large current; 0: termination disable */
	/*
	 * 0x: phy output is data
	 * 10: phy output is 0
	 * 11: phy output is 1
	 */
	u32 d_reg: 2;			/* bit 22-23 */

	u32 current_opt: 1;		/* bit 24, work with bit 21 'cur_term_en' */
	u32 r_opt: 1;			/* bit 25 */
	u32 reserved26: 2;		/* bit 26-27 */

	u32 mute0: 1;			/* bit 28 */
	u32 mute1: 1;			/* bit 29 */
	u32 mute2: 1;			/* bit 30 */
	u32 mute_clk: 1;		/* bit 31 */

} mt_analog_hdmitx_r2_attr_t;

/* hdmi_tx_reg0/hdmi_tx_reg1/hdmi_tx_reg2: 0xBF5D01BC/0xBF5D01C0/0xBF5D01C4 */
typedef struct mt_analog_hdmitx_ch_attr
{
	u32 main: 4;			/* bit 0-3 */
	u32 ffe_en: 4;			/* bit 4-7 */

	u32 ffe_str: 8;			/* bit 8-15, 20% of main current */

	u32 reg_data_tst: 4;	/* bit 16-19, reg data for test */

	u32 hdmi_r_en: 1;		/* bit 20, 1: termination r enable */
	u32 cur_term_en: 1;		/* bit 21, 1: termination enable large current; 0: termination disable */
	/*
	 * 0x: phy output is data
	 * 10: phy output is 0
	 * 11: phy output is 1
	 */
	u32 d_reg: 2;			/* bit 22-23 */

	u32 current_opt: 1;		/* bit 24, work with bit 21 'cur_term_en' */
	u32 r_opt: 1;			/* bit 25 */
	u32 reserved26: 2;		/* bit 26-27 */

	u32 reserved28: 1;		/* bit 28 */
	u32 reserved29: 1;		/* bit 29 */
	u32 reserved30: 1;		/* bit 30 */
	u32 reserved31: 1;		/* bit 31 */

} mt_analog_hdmitx_ch_attr_t;

/* hdmi_test_reg0: 0xBF5D01C8 */
typedef struct mt_analog_hdmi_test_r0_attr
{
	u32 main: 4;				/* bit 0-3 */

	u32 hdmi_r_en: 1;			/* bit 4 */
	u32 cur_term_en: 1;			/* bit 5 */
	u32 sel: 1;					/* bit 6, 0: div10, 1: div40 */
	u32 current_opt: 1;			/* bit 7 */

	u32 mute: 1;				/* bit 8, 1: mute channel(?) */
	u32 reserved2: 3;			/* bit 9-11 */

	u32 d0_bist_reg: 4;			/* bit 12-15 */
	u32 d1_bist_reg: 4;			/* bit 16-19 */

	u32 d2_hdmi_bist_en: 1;		/* bit 20 */
	u32 d2_bist_rst: 1;			/* bit 21 */
	u32 d2_err_flag_en: 1;		/* bit 22 */
	u32 d2_bist_reg_en: 1;		/* bit 23 */

	u32 imp_ext: 4;				/* bit 24-27, imp setting from reg */

	u32 imp_ext_en: 1;			/* bit 28, 1: enable imp setting from reg */
	u32 resetb: 1;				/* bit 29, 1: release, 0: reset */
	u32 pd_cal: 1;				/* bit 30, pd calibration resistor */
	u32 pd_reg: 1;				/* bit 31, pd calibration reg */

} mt_analog_hdmi_test_r0_attr_t;

//---------------------------------------------------------------------------//

#ifndef NO_DUMP_HDMI_STATE

static void mt_hdmi_dump_state(void *s)
{
	struct mt_analog_hdmitx_ch_attr hdmitx_ch0_attr;
	struct mt_analog_hdmitx_ch_attr hdmitx_ch1_attr;
	struct mt_analog_hdmitx_r2_attr hdmitx_ch2_attr;
	struct mt_analog_hdmi_test_r0_attr hdmi_tst_attr;

	mt_analog_get_attr(MT_ANA_INDEX_HDMITX_R0, (void*)&hdmitx_ch0_attr);
	mt_analog_get_attr(MT_ANA_INDEX_HDMITX_R1, (void*)&hdmitx_ch1_attr);
	mt_analog_get_attr(MT_ANA_INDEX_HDMITX_R2, (void*)&hdmitx_ch2_attr);
	mt_analog_get_attr(MT_ANA_INDEX_HDMI_TEST_R0, (void*)&hdmi_tst_attr);

	DP_LOG("-----------------------[HDMI]--------------------\n");
	DP_LOG(" hdmi_tx_reg0[%8lX]: %08X\n", HDMI_TX_REG0, MT_IO_READ32(HDMI_TX_REG0));
	DP_LOG(" hdmi_tx_reg1[%8lX]: %08X\n", HDMI_TX_REG1, MT_IO_READ32(HDMI_TX_REG1));
	DP_LOG(" hdmi_tx_reg2[%8lX]: %08X\n", HDMI_TX_REG2, MT_IO_READ32(HDMI_TX_REG2));
	DP_LOG("hdmi_test_reg[%8lX]: %08X\n", REG_HDMI_TEST, MT_IO_READ32(REG_HDMI_TEST));
	DP_LOG("   hdmi_logic[%8lX]: %08X\n", REG_HDMI_LOGIC, MT_IO_READ32(REG_HDMI_LOGIC));

	DP_LOG("\nhdmi_tx_reg0[%8lX]: %08X\n", HDMI_TX_REG0, MT_IO_READ32(HDMI_TX_REG0));
	DP_LOG("                  main: 0x%X\n", hdmitx_ch0_attr.main);
	DP_LOG("                ffe_en: 0x%X\n", hdmitx_ch0_attr.ffe_en);
	DP_LOG("               ffe_str: 0x%X\n", hdmitx_ch0_attr.ffe_str);
	DP_LOG("          reg_data_tst: 0x%X\n", hdmitx_ch0_attr.reg_data_tst);
	DP_LOG("             hdmi_r_en: %u\n", hdmitx_ch0_attr.hdmi_r_en);
	DP_LOG("           cur_term_en: %u\n", hdmitx_ch0_attr.cur_term_en);
	DP_LOG("                 d_reg: %u\n", hdmitx_ch0_attr.d_reg);
	DP_LOG("           current_opt: %u\n", hdmitx_ch0_attr.current_opt);
	DP_LOG("                 r_opt: %u\n", hdmitx_ch0_attr.r_opt);

	DP_LOG("\nhdmi_tx_reg1[%8lX]: %08X\n", HDMI_TX_REG1, MT_IO_READ32(HDMI_TX_REG1));
	DP_LOG("                  main: 0x%X\n", hdmitx_ch1_attr.main);
	DP_LOG("                ffe_en: 0x%X\n", hdmitx_ch1_attr.ffe_en);
	DP_LOG("               ffe_str: 0x%X\n", hdmitx_ch1_attr.ffe_str);
	DP_LOG("          reg_data_tst: 0x%X\n", hdmitx_ch1_attr.reg_data_tst);
	DP_LOG("             hdmi_r_en: %u\n", hdmitx_ch1_attr.hdmi_r_en);
	DP_LOG("           cur_term_en: %u\n", hdmitx_ch1_attr.cur_term_en);
	DP_LOG("                 d_reg: %u\n", hdmitx_ch1_attr.d_reg);
	DP_LOG("           current_opt: %u\n", hdmitx_ch1_attr.current_opt);
	DP_LOG("                 r_opt: %u\n", hdmitx_ch1_attr.r_opt);

	DP_LOG("\nhdmi_tx_reg2[%8lX]: %08X\n", HDMI_TX_REG2, MT_IO_READ32(HDMI_TX_REG2));
	DP_LOG("                  main: 0x%X\n", hdmitx_ch2_attr.main);
	DP_LOG("                ffe_en: 0x%X\n", hdmitx_ch2_attr.ffe_en);
	DP_LOG("               ffe_str: 0x%X\n", hdmitx_ch2_attr.ffe_str);
	DP_LOG("          reg_data_tst: 0x%X\n", hdmitx_ch2_attr.reg_data_tst);
	DP_LOG("             hdmi_r_en: %u\n", hdmitx_ch2_attr.hdmi_r_en);
	DP_LOG("           cur_term_en: %u\n", hdmitx_ch2_attr.cur_term_en);
	DP_LOG("                 d_reg: %u\n", hdmitx_ch2_attr.d_reg);
	DP_LOG("           current_opt: %u\n", hdmitx_ch2_attr.current_opt);
	DP_LOG("                 r_opt: %u\n", hdmitx_ch2_attr.r_opt);
	DP_LOG("                 mute0: %u\n", hdmitx_ch2_attr.mute0);
	DP_LOG("                 mute1: %u\n", hdmitx_ch2_attr.mute1);
	DP_LOG("                 mute2: %u\n", hdmitx_ch2_attr.mute2);
	DP_LOG("              mute_clk: %u\n", hdmitx_ch2_attr.mute_clk);

	DP_LOG("\nhdmi_test_reg0[%8lX]: %08X\n", REG_HDMI_TEST, MT_IO_READ32(REG_HDMI_TEST));
	DP_LOG("                    main: 0x%X\n", hdmi_tst_attr.main);
	DP_LOG("               hdmi_r_en: %u\n", hdmi_tst_attr.hdmi_r_en);
	DP_LOG("             cur_term_en: %u\n", hdmi_tst_attr.cur_term_en);
	DP_LOG("                     sel: %u\n", hdmi_tst_attr.sel);
	DP_LOG("             current_opt: %u\n", hdmi_tst_attr.current_opt);
	DP_LOG("                    mute: %u\n", hdmi_tst_attr.mute);
	DP_LOG("             d0_bist_reg: 0x%X\n", hdmi_tst_attr.d0_bist_reg);
	DP_LOG("             d1_bist_reg: 0x%X\n", hdmi_tst_attr.d1_bist_reg);
	DP_LOG("         d2_hdmi_bist_en: %u\n", hdmi_tst_attr.d2_hdmi_bist_en);
	DP_LOG("             d2_bist_rst: %u\n", hdmi_tst_attr.d2_bist_rst);
	DP_LOG("          d2_err_flag_en: %u\n", hdmi_tst_attr.d2_err_flag_en);
	DP_LOG("          d2_bist_reg_en: %u\n", hdmi_tst_attr.d2_bist_reg_en);

	DP_LOG("                 imp_ext: 0x%X\n", hdmi_tst_attr.imp_ext);
	DP_LOG("              imp_ext_en: %u\n", hdmi_tst_attr.imp_ext_en);
	DP_LOG("                  resetb: %u\n", hdmi_tst_attr.resetb);
	DP_LOG("                  pd_cal: %u\n", hdmi_tst_attr.pd_cal);
	DP_LOG("                  pd_reg: %u\n", hdmi_tst_attr.pd_reg);

	//DP_LOG("-------------------------------------------------\n");
}

#endif //NO_DUMP_HDMI_STATE

#endif

