/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2020, Montage-LZ Technology Co., Ltd.
 *
 * File Name      : pinmux_sym6.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2020/9/22
 * Description    : Symphony6 PINMUX Register table/group definition.
 * History        :
 * 1.Date         : 2020/9/22
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#if defined(__UBOOT__)
#include <common.h>
#include <asm/arch-symphony6/mt_common.h>
#elif defined(__KERNEL__)
#include <linux/types.h>
#include "mt_drv_pinctrl.h"

/* FIXME */
#include "../crm/mt_io.h"

#else
#include "mt_common.h"
#include "mt_drv_pinctrl.h"
#include "pinctrl_user.h"
#endif

#include "pinctrl.h"

/* function not available */
#define FN_NA		"na"

static struct pinmux_in g_sw_pin_tab[] =
{
	/* INDEX               NAME             OFF   IO_CTRL  SEL     FUNCTION[0-7] */
	/* UART0 */
	{INDEX_SW_PIN_CTRL000, "PAD_UART0_TXD", 0x000, {8, 6}, {0, 3},
		{"uart0_txd", "gpio0", NULL}, },
	{INDEX_SW_PIN_CTRL001, "PAD_UART0_RXD", 0x004, {8, 6}, {0, 3},
		{"uart0_rxd", "gpio1", NULL}, },

	/* SPI_Flash */
#if 0
/* A0 */
	{INDEX_SW_PIN_CTRL002, "PAD_SPI0_CLK", 0x008, {8, 6}, {0, 3},
		{"spi0_clk", "gpio2", "bcfg_bsel[0]", "pll_tst[0]", FN_NA, "ephy_adctst_out[0]", "adc_tst_out0", "adc_tst_in0"}, },
	{INDEX_SW_PIN_CTRL003, "PAD_SPI0_IO0", 0x00C, {8, 6}, {0, 3},
		{"spi0_io[0]", "gpio3", "bcfg_bsel[1]", "pll_tst[1]", FN_NA, "ephy_adctst_out[1]", "adc_tst_out1", "adc_tst_in1"}, },
	{INDEX_SW_PIN_CTRL004, "PAD_SPI0_IO1", 0x010, {8, 6}, {0, 3},
		{"spi0_io[1]", "gpio4", "bcfg_bsel[2]", "pll_tst[2]", FN_NA, "ephy_adctst_out[2]", "adc_tst_out2", "adc_tst_in2"}, },
	{INDEX_SW_PIN_CTRL005, "PAD_SPI0_IO2", 0x014, {8, 6}, {0, 3},
		{"spi0_io[2]", "gpio5", "bcfg_bsel[3]", "pll_tst[3]", FN_NA, "ephy_adctst_out[3]", "adc_tst_out3", "adc_tst_in3"}, },
	{INDEX_SW_PIN_CTRL006, "PAD_SPI0_IO3", 0x018, {8, 6}, {0, 3},
		{"spi0_io[3]", "gpio6", "bcfg_bsel[4]", "pll_tst[4]", FN_NA, "ephy_adctst_out[4]", "adc_tst_out4", "adc_tst_in4"}, },
	{INDEX_SW_PIN_CTRL007, "PAD_SPI0_CS0", 0x01C, {8, 6}, {0, 3},
		{"spi0_cs0", "gpio7", FN_NA, "pll_tst[5]", FN_NA, "ephy_adctst_out[5]", "adc_tst_out5", "adc_tst_in5"}, },
#else
/* A1 */
	{INDEX_SW_PIN_CTRL002, "PAD_SPI0_CLK", 0x008, {8, 6}, {0, 3},
		{"spi0_clk", "gpio2", "bcfg_bsel[0]", "pll_tst[0]", FN_NA, "ephy_adctst_out[0]", "adc_tst_out0", NULL}, },
	{INDEX_SW_PIN_CTRL003, "PAD_SPI0_IO0", 0x00C, {8, 6}, {0, 3},
		{"spi0_io[0]", "gpio3", "bcfg_bsel[1]", "pll_tst[1]", FN_NA, "ephy_adctst_out[1]", "adc_tst_out1", NULL}, },
	{INDEX_SW_PIN_CTRL004, "PAD_SPI0_IO1", 0x010, {8, 6}, {0, 3},
		{"spi0_io[1]", "gpio4", "bcfg_bsel[2]", "pll_tst[2]", FN_NA, "ephy_adctst_out[2]", "adc_tst_out2", NULL}, },
	{INDEX_SW_PIN_CTRL005, "PAD_SPI0_IO2", 0x014, {8, 6}, {0, 3},
		{"spi0_io[2]", "gpio5", "bcfg_bsel[3]", "pll_tst[3]", FN_NA, "ephy_adctst_out[3]", "adc_tst_out3", NULL}, },
	{INDEX_SW_PIN_CTRL006, "PAD_SPI0_IO3", 0x018, {8, 6}, {0, 3},
		{"spi0_io[3]", "gpio6", "bcfg_bsel[4]", "pll_tst[4]", FN_NA, "ephy_adctst_out[4]", "adc_tst_out4", NULL}, },
	{INDEX_SW_PIN_CTRL007, "PAD_SPI0_CS0", 0x01C, {8, 6}, {0, 3},
		{"spi0_cs0", "gpio7", FN_NA, "pll_tst[5]", FN_NA, "ephy_adctst_out[5]", "adc_tst_out5", NULL}, },
#endif

	/* SMC */
	{INDEX_SW_PIN_CTRL008, "PAD_SMC_DETECT", 0x020, {8, 6}, {0, 3},
		{"smc_detect", "gpio8", "dai_rx_sclk", FN_NA, FN_NA, FN_NA, FN_NA, "adc_tst_clkin"}, },
	{INDEX_SW_PIN_CTRL009, "PAD_SMC_PWREN", 0x024, {8, 6}, {0, 3},
		{"smc_pwren", "gpio9", FN_NA, FN_NA, "pwm0", NULL}, },
	{INDEX_SW_PIN_CTRL010, "PAD_SMC_CLK", 0x028, {8, 6}, {0, 3},
		{"smc_clk", "gpio10", "dai_rx_lrck", FN_NA, "pwm1", NULL}, },
	{INDEX_SW_PIN_CTRL011, "PAD_SMC_RST", 0x02C, {8, 6}, {0, 3},
		{"smc_rst", "gpio11", "dai_rx_mclk", FN_NA, "pwm2", NULL}, },
	{INDEX_SW_PIN_CTRL012, "PAD_SMC_IO", 0x030, {8, 6}, {0, 3},
		{"smc_io", "gpio12", "dai_rx_data", FN_NA, "pwm3", NULL}, },
	{INDEX_SW_PIN_CTRL013, "PAD_SMC5V_CLK", 0x034, {8, 8}, {0, 3},
		{"smc_clk", "gpio13", "dai_rx_lrck", FN_NA, "pwm1", NULL}, },
	{INDEX_SW_PIN_CTRL014, "PAD_SMC5V_RST", 0x038, {8, 8}, {0, 3},
		{"smc_rst", "gpio14", "dai_rx_mclk", FN_NA, "pwm2", NULL}, },
	{INDEX_SW_PIN_CTRL015, "PAD_SMC5V_IO", 0x03C, {8, 8}, {0, 3},
		{"smc_io", "gpio15", "dai_rx_data", FN_NA, "pwm3", NULL}, },

	/* TS */
	{INDEX_SW_PIN_CTRL016, "PAD_TSI_D7", 0x040, {8, 6}, {0, 3},
		{"tsi_d[7]", "gpio16", "dai_rx_lrck", NULL}, },
	{INDEX_SW_PIN_CTRL017, "PAD_TSI_D6", 0x044, {8, 6}, {0, 3},
		{"tsi_d[6]", "gpio17", "dai_rx_data", NULL}, },
	{INDEX_SW_PIN_CTRL018, "PAD_TSI_D5", 0x048, {8, 6}, {0, 3},
		{"tsi_d[5]", "gpio18", "dai_rx_sclk", NULL}, },
	{INDEX_SW_PIN_CTRL019, "PAD_TSI_D4", 0x04C, {8, 6}, {0, 3},
		{"tsi_d[4]", "gpio19", "dai_rx_mclk", "tsi3_d0", NULL}, },
	{INDEX_SW_PIN_CTRL020, "PAD_TSI_D3", 0x050, {8, 6}, {0, 3},
		{"tsi_d[3]", "gpio20", "bcfg_bsel[6]", "tsi3_clk", NULL}, },
	{INDEX_SW_PIN_CTRL021, "PAD_TSI_D2", 0x054, {8, 6}, {0, 3},
		{"tsi_d[2]", "gpio21", FN_NA, "tsi3_valid", NULL}, },
	{INDEX_SW_PIN_CTRL022, "PAD_TSI_D1", 0x058, {8, 6}, {0, 3},
		{"tsi_d[1]", "gpio22", FN_NA, "tsi3_sync", NULL}, },
	{INDEX_SW_PIN_CTRL023, "PAD_TSI_D0", 0x05C, {8, 6}, {0, 3},
		{"tsi_d[0]", "gpio23", NULL}, },
	{INDEX_SW_PIN_CTRL024, "PAD_TSI_VALD", 0x060, {8, 6}, {0, 3},
		{"tsi_valid", "gpio24", NULL}, },
	{INDEX_SW_PIN_CTRL025, "PAD_TSI_SYNC", 0x064, {8, 6}, {0, 3},
		{"tsi_sync", "gpio25", NULL}, },
	{INDEX_SW_PIN_CTRL026, "PAD_TSI_CLK", 0x068, {8, 6}, {0, 3},
		{"tsi_clk", "gpio26", NULL}, },

	/* Demod */
	{INDEX_SW_PIN_CTRL027, "PAD_DISQC", 0x06C, {8, 6}, {0, 3},
		{"diseqc", "gpio27", "arm_ejtg_tck_ch0", "mips_ejtg_tck_ch0", "tsi_clk", "mbist_ejtg_tck", "uart1_cts", "dai_rx_data"}, },
	{INDEX_SW_PIN_CTRL028, "PAD_LNB_VSEL", 0x070, {8, 6}, {0, 3},
		{"lnb_vsel", "gpio28", "arm_ejtg_tms_ch0", "mips_ejtg_tms_ch0", "tsi_sync", "mbist_ejtg_tms", "tsi3_clk", "pwm0"}, },
	{INDEX_SW_PIN_CTRL029, "PAD_DISQC_IN", 0x074, {8, 6}, {0, 3},
		{"diseqc_in", "gpio29", "arm_ejtg_tdi_ch0", "mips_ejtg_tdi_ch0", "tsi_d[0]", "mbist_ejtg_tdi", "pwm8", "diseqc"}, },
	{INDEX_SW_PIN_CTRL030, "PAD_LNB_EN", 0x078, {8, 6}, {0, 3},
		{"lnb_en", "gpio30", "arm_ejtg_tdo_ch0", "mips_ejtg_tdo_ch0", "tsi_valid", "mbist_ejtg_tdo", "tsi3_d0", "pwm9"}, },

	{INDEX_SW_PIN_CTRL031_A, "PAD_I2C0_SCL", 0x07C, {8, 6}, {0, 3},
		{"i2c0_scl", "gpio31", "i2c_scl_rpt", NULL}, },
	{INDEX_SW_PIN_CTRL032_A, "PAD_I2C0_SDA", 0x080, {8, 6}, {0, 3},
		{"i2c0_sda", "gpio32", "i2c_sda_rpt", NULL}, },

	{INDEX_SW_PIN_CTRL033_A, "PAD_C_IF_AGC", 0x084, {8, 6}, {0, 3},
		{"c_if_agc", "gpio33", "tsi2_sync", "tsi2_valid", "diseqc_in", "pwm4", NULL}, },
	{INDEX_SW_PIN_CTRL034_A, "PAD_S_RF_AGC", 0x088, {8, 6}, {0, 3},
		{"s_rf_agc", "gpio34", FN_NA, FN_NA, "pwm4", NULL}, },

	/* ETH */
	/* when fmux = 1, gpio35~38, da_r/da_l: see: INDEX_DA_PIN_SEL */
	{INDEX_SW_PIN_CTRL035, "PAD_EPHY_LED0", 0x08C, {8, 6}, {0, 3},
		{"ephy_led0", "gpio35(da_r-)", "ephy_led1", FN_NA, "pwm5", NULL}, },
	{INDEX_SW_PIN_CTRL036, "PAD_EPHY_LED1", 0x090, {8, 6}, {0, 3},
		{"ephy_led1", "gpio36(da_l-)", "ephy_led0", FN_NA, "pwm6", NULL}, },

#if 0
/* A0 */
	{INDEX_SW_PIN_CTRL037, "PAD_SPDIF_OUT", 0x094, {8, 6}, {0, 3},
		{"spdif_out", "gpio37(da_r+)", "bcg_bsel[5]", "pll_tst[6]", "pwm7", "ephy_adctst_out[6]", "adc_tst_out9", "adc_tst_in9"}, },
	{INDEX_SW_PIN_CTRL038, "PAD_MUTE", 0x098, {8, 6}, {0, 3},
		{FN_NA, "gpio38(da_l+)", FN_NA, "pll_tst[7]", FN_NA, "ephy_adctst_out[7]", "adc_tst_out10", "adc_tst_in10"}, },
#else
/* A1 */
	/* SPDIF */
	{INDEX_SW_PIN_CTRL037, "PAD_SPDIF_OUT", 0x094, {8, 6}, {0, 3},
		{"spdif_out", "gpio37(da_r+)", "bcg_bsel[5]", "pll_tst[6]", "pwm7", "ephy_adctst_out[6]", "adc_tst_out9", NULL}, },
	/* MUTE */
	{INDEX_SW_PIN_CTRL038, "PAD_MUTE", 0x098, {8, 6}, {0, 3},
		{FN_NA, "gpio38(da_l+)", FN_NA, "pll_tst[7]", FN_NA, "ephy_adctst_out[7]", "adc_tst_out10", NULL}, },
#endif

	/* NAND */
	{INDEX_SW_PIN_CTRL039, "PAD_NAND_IO7", 0x09C, {8, 8}, {0, 3},
		{"nand_io[7]", "gpio39", "emmc_data[3]", NULL}, },
	{INDEX_SW_PIN_CTRL040, "PAD_NAND_IO6", 0x0A0, {8, 8}, {0, 3},
		{"nand_io[6]", "gpio40", "emmc_data[0]", NULL}, },
	{INDEX_SW_PIN_CTRL041, "PAD_NAND_IO5", 0x0A4, {8, 8}, {0, 3},
		{"nand_io[5]", "gpio41", "emmc_data[4]", NULL}, },
	{INDEX_SW_PIN_CTRL042, "PAD_NAND_IO4", 0x0A8, {8, 8}, {0, 3},
		{"nand_io[4]", "gpio42", "emmc_data[1]", NULL}, },
	{INDEX_SW_PIN_CTRL043, "PAD_NAND_IO3", 0x0AC, {8, 8}, {0, 3},
		{"nand_io[3]", "gpio43", "emmc_data[5]", NULL}, },
	{INDEX_SW_PIN_CTRL044, "PAD_NAND_IO2", 0x0B0, {8, 8}, {0, 3},
		{"nand_io[2]", "gpio44", "emmc_data[2]", NULL}, },
	{INDEX_SW_PIN_CTRL045, "PAD_NAND_IO1", 0x0B4, {8, 8}, {0, 3},
		{"nand_io[1]", "gpio45", "emmc_data[6]", NULL}, },
	{INDEX_SW_PIN_CTRL046, "PAD_NAND_IO0", 0x0B8, {8, 8}, {0, 3},
		{"nand_io[0]", "gpio46", "emmc_data[7]", NULL}, },
	{INDEX_SW_PIN_CTRL047, "PAD_NAND_DQS", 0x0BC, {8, 8}, {0, 3},
		{"nand_dqs", "gpio47", NULL}, },
	{INDEX_SW_PIN_CTRL048, "PAD_NAND_WE_N", 0x0C0, {8, 8}, {0, 3},
		{"nand_we_n", "gpio48", NULL}, },
	{INDEX_SW_PIN_CTRL049, "PAD_NAND_ALE", 0x0C4, {8, 8}, {0, 3},
		{"nand_ale", "gpio49", "emmc_pwren", NULL}, },
	{INDEX_SW_PIN_CTRL050, "PAD_NAND_CLE", 0x0C8, {8, 8}, {0, 3},
		{"nand_cle", "gpio50", "emmc_cmd", NULL}, },
	{INDEX_SW_PIN_CTRL051, "PAD_NAND_CE_N", 0x0CC, {8, 8}, {0, 3},
		{"nand_ce_n", "gpio51", "emmc_cd", NULL}, },
	{INDEX_SW_PIN_CTRL052, "PAD_NAND_RE_N", 0x0D0, {8, 8}, {0, 3},
		{"nand_re_n", "gpio52", "emmc_clk", NULL}, },
	{INDEX_SW_PIN_CTRL053, "PAD_NAND_RB_N", 0x0D4, {8, 8}, {0, 3},
		{"nand_rb_n", "gpio53", "emmc_rstn", NULL}, },

#if 0
/* A0 */
	{INDEX_SW_PIN_CTRL054, "PAD_SDIO_WP", 0x0D8, {8, 6}, {0, 3},
		{"sdio_wp", "gpio54", FN_NA, FN_NA, FN_NA, FN_NA, "adc_tst_out11", "adc_tst_in11"}, },
	{INDEX_SW_PIN_CTRL055, "PAD_SDIO_DATA2", 0x0DC, {8, 8}, {0, 3},
		{"sdio_data[2]", "gpio55", FN_NA, FN_NA, FN_NA, FN_NA, "adc_tst_out12", "adc_tst_in12"}, },
	{INDEX_SW_PIN_CTRL056, "PAD_SDIO_DATA3", 0x0E0, {8, 8}, {0, 3},
		{"sdio_data[3]", "gpio56", FN_NA, FN_NA, FN_NA, FN_NA, "adc_tst_out13", "adc_tst_in13"}, },
	{INDEX_SW_PIN_CTRL057, "PAD_SDIO_CMD", 0x0E4, {8, 8}, {0, 3},
		{"sdio_cmd", "gpio57", "arm_ejtg_trstn_ch1", "mips_ejtg_trstn_ch1", FN_NA, FN_NA, "adc_tst_out14", "adc_tst_in14"}, },
	{INDEX_SW_PIN_CTRL058, "PAD_SDIO_CLK", 0x0E8, {8, 8}, {0, 3},
		{"sdio_clk", "gpio58", "arm_ejtg_tdi_ch1", "mips_ejtg_tdi_ch1", FN_NA, FN_NA, "adc_tst_out15", "adc_tst_in15"}, },
	{INDEX_SW_PIN_CTRL059, "PAD_SDIO_DATA0", 0x0EC, {8, 8}, {0, 3},
		{"sdio_data[0]", "gpio59", "arm_ejtg_tdo_ch1", "mips_ejtg_tdo_ch1", FN_NA, FN_NA, "adc_tst_out6", "adc_tst_in6"}, },
	{INDEX_SW_PIN_CTRL060, "PAD_SDIO_DATA1", 0x0F0, {8, 8}, {0, 3},
		{"sdio_data[1]", "gpio60", "arm_ejtg_tms_ch1", "mips_ejtg_tms_ch1", FN_NA, FN_NA, "adc_tst_out7", "adc_tst_in7"}, },
	{INDEX_SW_PIN_CTRL061, "PAD_SDIO_CD", 0x0F4, {8, 6}, {0, 3},
		{"sdio_cd", "gpio61", "arm_ejtg_tck_ch1", "mips_ejtg_tck_ch1", FN_NA, FN_NA, "adc_tst_out8", "adc_tst_in8"}, },
#else
/* A1 */
	/* SDIO */
	{INDEX_SW_PIN_CTRL054, "PAD_SDIO_WP", 0x0D8, {8, 6}, {0, 3},
		{"sdio_wp", "gpio54", FN_NA, FN_NA, FN_NA, FN_NA, "adc_tst_out11", NULL}, },
	{INDEX_SW_PIN_CTRL055, "PAD_SDIO_DATA2", 0x0DC, {8, 8}, {0, 3},
		{"sdio_data[2]", "gpio55", FN_NA, FN_NA, FN_NA, FN_NA, "adc_tst_out12", NULL}, },
	{INDEX_SW_PIN_CTRL056, "PAD_SDIO_DATA3", 0x0E0, {8, 8}, {0, 3},
		{"sdio_data[3]", "gpio56", FN_NA, FN_NA, FN_NA, FN_NA, "adc_tst_out13", NULL}, },
	{INDEX_SW_PIN_CTRL057, "PAD_SDIO_CMD", 0x0E4, {8, 8}, {0, 3},
		{"sdio_cmd", "gpio57", "arm_ejtg_trstn_ch1", "mips_ejtg_trstn_ch1", FN_NA, FN_NA, "adc_tst_out14", NULL}, },
	{INDEX_SW_PIN_CTRL058, "PAD_SDIO_CLK", 0x0E8, {8, 8}, {0, 3},
		{"sdio_clk", "gpio58", "arm_ejtg_tdi_ch1", "mips_ejtg_tdi_ch1", FN_NA, FN_NA, "adc_tst_out15", NULL}, },
	{INDEX_SW_PIN_CTRL059, "PAD_SDIO_DATA0", 0x0EC, {8, 8}, {0, 3},
		{"sdio_data[0]", "gpio59", "arm_ejtg_tdo_ch1", "mips_ejtg_tdo_ch1", FN_NA, FN_NA, "adc_tst_out6", NULL}, },
	{INDEX_SW_PIN_CTRL060, "PAD_SDIO_DATA1", 0x0F0, {8, 8}, {0, 3},
		{"sdio_data[1]", "gpio60", "arm_ejtg_tms_ch1", "mips_ejtg_tms_ch1", FN_NA, FN_NA, "adc_tst_out7", NULL}, },
	{INDEX_SW_PIN_CTRL061, "PAD_SDIO_CD", 0x0F4, {8, 6}, {0, 3},
		{"sdio_cd", "gpio61", "arm_ejtg_tck_ch1", "mips_ejtg_tck_ch1", FN_NA, FN_NA, "adc_tst_out8", NULL}, },
#endif

	/* GMAC */
	{INDEX_SW_PIN_CTRL062, "PAD_RGMII_RX_EN", 0x0F8, {8, 8}, {0, 3},
		{"rgmii_rx_en", "gpio62", "rgmii_tx[3]", NULL}, },
	{INDEX_SW_PIN_CTRL063, "PAD_RGMII_RX0", 0x0FC, {8, 8}, {0, 3},
		{"rgmii_rx[0]", "gpio63", "rgmii_tx[2]", NULL}, },
	{INDEX_SW_PIN_CTRL064, "PAD_RGMII_RX1", 0x100, {8, 8}, {0, 3},
		{"rgmii_rx[1]", "gpio64", "rgmii_tx[1]", NULL}, },
	{INDEX_SW_PIN_CTRL065, "PAD_RGMII_RX2", 0x104, {8, 8}, {0, 3},
		{"rgmii_rx[2]", "gpio65", "rgmii_tx[0]", "ci_we_n", NULL}, },
	{INDEX_SW_PIN_CTRL066, "PAD_RGMII_RX3", 0x108, {8, 8}, {0, 3},
		{"rgmii_rx[3]", "gpio66", "rgmii_tx_en", "ci_a[14]", NULL}, },
	{INDEX_SW_PIN_CTRL067, "PAD_RGMII_RXC", 0x10C, {8, 8}, {0, 3},
		{"rgmii_rxc", "gpio67", "rgmii_txc", "ci_a[13]", NULL}, },
	{INDEX_SW_PIN_CTRL068, "PAD_RGMII_TXC", 0x110, {8, 8}, {0, 3},
		{"rgmii_txc", "gpio68", "rgmii_rx[3]", "ci_a[8]", NULL}, },
	{INDEX_SW_PIN_CTRL069, "PAD_RGMII_TX0", 0x114, {8, 8}, {0, 3},
		{"rgmii_tx[0]", "gpio69", "rgmii_rx[2]", "ci_a[9]", NULL}, },
	{INDEX_SW_PIN_CTRL070, "PAD_RGMII_TX1", 0x118, {8, 8}, {0, 3},
		{"rgmii_tx[1]", "gpio70", "rgmii_rx[1]", "ci_iowr_n", NULL}, },
	{INDEX_SW_PIN_CTRL071, "PAD_RGMII_TX2", 0x11C, {8, 8}, {0, 3},
		{"rgmii_tx[2]", "gpio71", "rgmii_rx[0]", "ci_a[11]", NULL}, },
	{INDEX_SW_PIN_CTRL072, "PAD_RGMII_TX3", 0x120, {8, 8}, {0, 3},
		{"rgmii_tx[3]", "gpio72", "rgmii_rx_en", "ci_oe_n", NULL}, },
	{INDEX_SW_PIN_CTRL073, "PAD_RGMII_TX_EN", 0x124, {8, 8}, {0, 3},
		{"rgmii_tx_en", "gpio73", "rgmii_rxc", "ci_iord_n", NULL}, },
	{INDEX_SW_PIN_CTRL074, "PAD_RGMII_MDC", 0x128, {8, 6}, {0, 3},
		{"rgmii_mdc", "gpio74", "rgmii_mdio", "ci_a[10]", NULL}, },
	{INDEX_SW_PIN_CTRL075, "PAD_RGMII_MDIO", 0x12C, {8, 6}, {0, 3},
		{"rgmii_mdio", "gpio75", "rgmii_mdc", "ci_ce1_n", NULL}, },
	{INDEX_SW_PIN_CTRL076, "PAD_RGMII_REFCLK", 0x130, {8, 6}, {0, 3},
		{"rgmii_refclk", "gpio76", NULL}, },

	/* CI */
	{INDEX_SW_PIN_CTRL077, "PAD_CI_D3", 0x134, {8, 6}, {0, 3},
		{"ci_d[3]", "gpio77", "dai_tx_mclk", NULL}, },
	{INDEX_SW_PIN_CTRL078, "PAD_CI_D4", 0x138, {8, 6}, {0, 3},
		{"ci_d[4]", "gpio78", "i2s_lrck", NULL}, },
	{INDEX_SW_PIN_CTRL079, "PAD_CI_D5", 0x13C, {8, 6}, {0, 3},
		{"ci_d[5]", "gpio79", "i2s_data", NULL}, },
	{INDEX_SW_PIN_CTRL080, "PAD_CI_D6", 0x140, {8, 6}, {0, 3},
		{"ci_d[6]", "gpio80", "i2s_sclk", NULL}, },
	{INDEX_SW_PIN_CTRL081, "PAD_CI_D7", 0x144, {8, 6}, {0, 3},
		{"ci_d[7]", "gpio81", "i2s_mclk", NULL}, },
	{INDEX_SW_PIN_CTRL082, "PAD_CI_MDI2", 0x148, {8, 6}, {0, 3},
		{"ci_mdi[2]", "gpio82", NULL}, },
	{INDEX_SW_PIN_CTRL083, "PAD_CI_MDI1", 0x14C, {8, 6}, {0, 3},
		{"ci_mdi[1]", "gpio83", NULL}, },
	{INDEX_SW_PIN_CTRL084, "PAD_CI_MDI0", 0x150, {8, 6}, {0, 3},
		{"ci_mdi[0]", "gpio84", NULL}, },
	{INDEX_SW_PIN_CTRL085, "PAD_CI_MISTRT", 0x154, {8, 6}, {0, 3},
		{"ci_mistrt", "gpio85", NULL}, },
	{INDEX_SW_PIN_CTRL086, "PAD_CI_MIVAL", 0x158, {8, 6}, {0, 3},
		{"ci_mival", "gpio86", "pwm9", NULL}, },
	{INDEX_SW_PIN_CTRL087, "PAD_CI_MCLKI", 0x15C, {8, 6}, {0, 3},
		{"ci_mclki", "gpio87", "pwm8", NULL}, },
	{INDEX_SW_PIN_CTRL088, "PAD_CI_MDI7", 0x160, {8, 6}, {0, 3},
		{"ci_mdi[7]", "gpio88", NULL}, },
	{INDEX_SW_PIN_CTRL089, "PAD_CI_MDI6", 0x164, {8, 6}, {0, 3},
		{"ci_mdi[6]", "gpio89", NULL}, },
	{INDEX_SW_PIN_CTRL090, "PAD_CI_MDI5", 0x168, {8, 6}, {0, 3},
		{"ci_mdi[5]", "gpio90", NULL}, },
	{INDEX_SW_PIN_CTRL091, "PAD_CI_IREQ_N", 0x16C, {8, 6}, {0, 3},
		{"ci_ireq_n", "gpio91", NULL}, },
	{INDEX_SW_PIN_CTRL092, "PAD_CI_A12", 0x170, {8, 6}, {0, 3},
		{"ci_a12", "gpio92", NULL}, },
	{INDEX_SW_PIN_CTRL093, "PAD_CI_A7", 0x174, {8, 6}, {0, 3},
		{"ci_a[7]", "gpio93", "pwm7", NULL}, },
	{INDEX_SW_PIN_CTRL094, "PAD_CI_A6", 0x178, {8, 6}, {0, 3},
		{"ci_a[6]", "gpio94", "pwm6", NULL}, },
	{INDEX_SW_PIN_CTRL095, "PAD_CI_A5", 0x17C, {8, 6}, {0, 3},
		{"ci_a[5]", "gpio95", "pwm5", NULL}, },
	{INDEX_SW_PIN_CTRL096, "PAD_CI_A4", 0x180, {8, 6}, {0, 3},
		{"ci_a[4]", "gpio96", "pwm4", NULL}, },
	{INDEX_SW_PIN_CTRL097, "PAD_CI_A3", 0x184, {8, 6}, {0, 3},
		{"ci_a[3]", "gpio97", "pwm3", NULL}, },
	{INDEX_SW_PIN_CTRL098, "PAD_CI_A2", 0x188, {8, 6}, {0, 3},
		{"ci_a[2]", "gpio98", "pwm2", NULL}, },
	{INDEX_SW_PIN_CTRL099, "PAD_CI_A1", 0x18C, {8, 6}, {0, 3},
		{"ci_a[1]", "gpio99", "pwm1", NULL}, },
	{INDEX_SW_PIN_CTRL100, "PAD_CI_A0", 0x190, {8, 6}, {0, 3},
		{"ci_a[0]", "gpio100", "pwm0", NULL}, },
	{INDEX_SW_PIN_CTRL101, "PAD_CI_D0", 0x194, {8, 6}, {0, 3},
		{"ci_d[0]", "gpio101", "dai_tx_lrck", NULL}, },
	{INDEX_SW_PIN_CTRL102, "PAD_CI_D1", 0x198, {8, 6}, {0, 3},
		{"ci_d[1]", "gpio102", "dai_tx_data", NULL}, },
	{INDEX_SW_PIN_CTRL103, "PAD_CI_D2", 0x19C, {8, 6}, {0, 3},
		{"ci_d[2]", "gpio103", "dai_tx_sclk", NULL}, },
	{INDEX_SW_PIN_CTRL104, "PAD_CI_CD1_N", 0x1A0, {8, 6}, {0, 3},
		{"ci_cd1_n", "gpio104", NULL}, },
	{INDEX_SW_PIN_CTRL105, "PAD_CI_MDO3", 0x1A4, {8, 6}, {0, 3},
		{"ci_mdo[3]", "gpio105", NULL}, },
	{INDEX_SW_PIN_CTRL106, "PAD_CI_MDO4", 0x1A8, {8, 6}, {0, 3},
		{"ci_mdo[4]", "gpio106", NULL}, },
	{INDEX_SW_PIN_CTRL107, "PAD_CI_MDO5", 0x1AC, {8, 6}, {0, 3},
		{"ci_mdo[5]", "gpio107", NULL}, },
	{INDEX_SW_PIN_CTRL108, "PAD_CI_MDO6", 0x1B0, {8, 6}, {0, 3},
		{"ci_mdo[6]", "gpio108", NULL}, },
	{INDEX_SW_PIN_CTRL109, "PAD_CI_MDO7", 0x1B4, {8, 6}, {0, 3},
		{"ci_mdo[7]", "gpio109", NULL}, },
	{INDEX_SW_PIN_CTRL110, "PAD_CI_MDI4", 0x1B8, {8, 6}, {0, 3},
		{"ci_mdi[4]", "gpio110", NULL}, },
	{INDEX_SW_PIN_CTRL111, "PAD_CI_MDI3", 0x1BC, {8, 6}, {0, 3},
		{"ci_mdi[3]", "gpio111", NULL}, },
	{INDEX_SW_PIN_CTRL112, "PAD_CI_MCLKO", 0x1C0, {8, 6}, {0, 3},
		{"ci_mclko", "gpio112", NULL}, },
	{INDEX_SW_PIN_CTRL113, "PAD_CI_RESET", 0x1C4, {8, 6}, {0, 3},
		{"ci_reset", "gpio113", NULL}, },
	{INDEX_SW_PIN_CTRL114, "PAD_CI_WAIT_N", 0x1C8, {8, 6}, {0, 3},
		{"ci_wait_n", "gpio114", NULL}, },
	{INDEX_SW_PIN_CTRL115, "PAD_CI_REG_N", 0x1CC, {8, 6}, {0, 3},
		{"ci_reg_n", "gpio115", NULL}, },
	{INDEX_SW_PIN_CTRL116, "PAD_CI_MOVAL", 0x1D0, {8, 6}, {0, 3},
		{"ci_moval", "gpio116", NULL}, },
	{INDEX_SW_PIN_CTRL117, "PAD_CI_MOSTRT", 0x1D4, {8, 6}, {0, 3},
		{"ci_mostrt", "gpio117", NULL}, },
	{INDEX_SW_PIN_CTRL118, "PAD_CI_MDO0", 0x1D8, {8, 6}, {0, 3},
		{"ci_mdo[0]", "gpio118", NULL}, },
	{INDEX_SW_PIN_CTRL119, "PAD_CI_MDO1", 0x1DC, {8, 6}, {0, 3},
		{"ci_mdo[1]", "gpio119", NULL}, },
	{INDEX_SW_PIN_CTRL120, "PAD_CI_MDO2", 0x1E0, {8, 6}, {0, 3},
		{"ci_mdo[2]", "gpio120", NULL}, },
	{INDEX_SW_PIN_CTRL121, "PAD_CI_CD2_N", 0x1E4, {8, 6}, {0, 3},
		{"ci_cd2_n", "gpio121", NULL}, },
	{INDEX_SW_PIN_CTRL122, "PAD_CI_PWEN", 0x1E8, {8, 6}, {0, 3},
		{FN_NA, "gpio122", NULL}, },

	{INDEX_SW_PIN_CTRL123, "PAD_SDIO_PWREN", 0x1EC, {8, 6}, {0, 3},
		{"sdio_pwren", "gpio123", NULL}, },

	/* undefined io_ctrl */
	{INDEX_SW_PIN_CTRL_IN0, "PAD_S_QP", 0x200, {INV_SFT, INV_WD}, {0, 3},
		{FN_NA, "gpio_in0", "tsi1_d0", NULL}, },
	/* undefined io_ctrl */
	{INDEX_SW_PIN_CTRL_IN1, "PAD_S_QN", 0x204, {INV_SFT, INV_WD}, {0, 3},
		{FN_NA, "gpio_in1", "tsi1_valid", "tsi3_d0", NULL}, },
	/* undefined io_ctrl */
	{INDEX_SW_PIN_CTRL_IN2, "PAD_S_IN", 0x208, {INV_SFT, INV_WD}, {0, 3},
		{FN_NA, "gpio_in2", "tsi1_sync", "tsi3_clk", NULL}, },
	/* undefined io_ctrl */
	{INDEX_SW_PIN_CTRL_IN3, "PAD_S_IP", 0x20C, {INV_SFT, INV_WD}, {0, 3},
		{FN_NA, "gpio_in3", "tsi1_clk", NULL}, },

	/* undefined io_ctrl */
	{INDEX_SW_PIN_CTRL_IN4, "PAD_C_VINN", 0x210, {INV_SFT, INV_WD}, {0, 3},
		{FN_NA, "gpio_in4", "tsi2_d0", NULL}, },
	/* undefined io_ctrl */
	{INDEX_SW_PIN_CTRL_IN5, "PAD_C_VINP", 0x214, {INV_SFT, INV_WD}, {0, 3},
		{FN_NA, "gpio_in5", "tsi2_clk", NULL}, },

	/* undefined io_ctrl */
	/* see: INDEX_SW_PIN_CTRL035 ~ 038 */
	{INDEX_DA_PIN_SEL, "PAD_DA_SEL", 0x218, {INV_SFT, INV_WD}, {0, 4},
		{"GPIO", "DA_R/DA_L", NULL}, },
};

struct pinmux_in g_ao_pin_tab[] =
{
	/* FP */
	{INDEX_AO_PIN_CTRL0, "PAD_FP_DATA", 0x400, {8, 6}, {0, 3},
		{"fp_data", "ao_gpio0", "fp_spi_data", "fp_i2c_sda", FN_NA, "lvdc_rstn", NULL}, },
	{INDEX_AO_PIN_CTRL1, "PAD_FP_CLK", 0x404, {8, 6}, {0, 3},
		{"fp_clk", "ao_gpio1", "fp_spi_clk", "fp_i2c_scl", "cpugp_en", NULL}, },
	{INDEX_AO_PIN_CTRL2, "PAD_FP_STB", 0x408, {8, 6}, {0, 3},
		{"fp_stb", "ao_gpio2", "fp_spi_csn", "mips_ejtg_trstn_ch0", "dai_rx_sclk", "mbist_ejtg_trstn", "uart1_rts", "arm_ejtg_trstn_ch0"}, },
	{INDEX_AO_PIN_CTRL3, "PAD_IRDA_IN", 0x40C, {8, 6}, {0, 3},
		{"irda_in", "ao_gpio3", NULL}, },

	/* System */
	{INDEX_AO_PIN_CTRL4, "PAD_SYS_RSTN", 0x410, {8, 6}, {0, 3},
		{"sys_rstn", "ao_gpio4", NULL}, },
	{INDEX_AO_PIN_CTRL5, "PAD_GP_EN", 0x414, {8, 6}, {0, 3},
		{"gp_en", "ao_gpio5", NULL}, },
	{INDEX_AO_PIN_CTRL6, "PAD_CPU_AVS", 0x418, {8, 6}, {0, 3},
		{"cpu_avs", "ao_gpio6", NULL}, },
	{INDEX_AO_PIN_CTRL7, "PAD_CORE_AVS", 0x41C, {8, 6}, {0, 3},
		{"core_avs", "ao_gpio7", NULL}, },

	/* UART1 */
	{INDEX_AO_PIN_CTRL8, "PAD_UART1_TXD", 0x420, {8, 6}, {0, 3},
		{"uart1_txd", "ao_gpio8", "dbg_scl", "i2c1_scl", "dai_rx_lrck", NULL}, },
	{INDEX_AO_PIN_CTRL9, "PAD_UART1_RXD", 0x424, {8, 6}, {0, 3},
		{"uart1_rxd", "ao_gpio9", "dbg_sda", "i2c1_sda", "dai_rx_mclk", NULL}, },

	/* HDMI */
	{INDEX_AO_PIN_CTRL10_A, "PAD_HDMI_CEC", 0x428, {8, 6}, {0, 3},
		{"hdmi_cec", "ao_gpio10", "ao_cec", NULL}, },
	{INDEX_AO_PIN_CTRL11_A, "PAD_HDMI_HPD", 0x42C, {8, 6}, {0, 3},
		{"hdmi_hpd", "ao_gpio11", NULL}, },
	{INDEX_AO_PIN_CTRL12_A, "PAD_HDMI_SDA", 0x430, {8, 6}, {0, 3},
		{"hdmi_sda", "ao_gpio12", NULL}, },
	{INDEX_AO_PIN_CTRL13_A, "PAD_HDMI_SCL", 0x434, {8, 6}, {0, 3},
		{"hdmi_scl", "ao_gpio13", NULL}, },
};

struct pinmux_group g_pin_group[] =
{
	{ .reg_base = REG_SW_PIN_CTRL_BASE,
	  .reg_size = 0x4000,
	  .pin_count = sizeof(g_sw_pin_tab) / sizeof(g_sw_pin_tab[0]),
	  .pin_tab = g_sw_pin_tab,
	},
	{ .reg_base = REG_AO_PIN_CTRL_BASE,
	  .reg_size = 0x1000,
	  .pin_count = sizeof(g_ao_pin_tab) / sizeof(g_ao_pin_tab[0]),
	  .pin_tab = g_ao_pin_tab,
	},
};

unsigned int g_pin_group_count = sizeof(g_pin_group) / sizeof(g_pin_group[0]);

