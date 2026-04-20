//***************************************************************************
//! @file     si_drv_tx_regs.h
//! @brief    Deco register definition header.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Deco, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2014, Deco, Inc.  All rights reserved.
//***************************************************************************/

#ifndef _SI_DRV_TX_REGS_H_
#define _SI_DRV_TX_REGS_H_

#if __HDMI_OS_LINUX__
	#define BASE_ADDRESS 0x3000
#else
	#define BASE_ADDRESS 0x0000    //base address is defined in blackbox.h
#endif

#if defined(CONFIG_MT_CHIP_ETUDE2)
	#include "si_drv_tx_regs_mt.h"
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
	#include "si_drv_tx_regs_sym6_mt.h"
#else
	#include "si_drv_tx_regs_mt.h"
#endif

typedef enum {
	REGTX_SOC_P0	= (BASE_ADDRESS | 0x000),
	REGTX_AVLINK	= (BASE_ADDRESS | 0x200),
	TMDS_BIST_REG	= (BASE_ADDRESS | 0x300),
	REGTX_TPI		= (BASE_ADDRESS | 0x600),
	REGTX_SERDES	= (BASE_ADDRESS | 0x700),
	REGTX_HDCP2X	= (BASE_ADDRESS | 0x800),
	REGTX_HDMI2		= (BASE_ADDRESS | 0x900),
	REGTX_AIP		= (BASE_ADDRESS | 0xA00),
	REGTX_CEC		= (BASE_ADDRESS | 0xF00),
} Page_t;

//***************************************************************************
// REGTX_SOC_P0. Address: 40
// Vendor ID Low byte Register
#define REG_ADDR__VND_IDL                                                (REGTX_SOC_P0 | 0x0000)
// (ReadOnly, Bits 7:0)
// Vendor ID Low byte (01h)
#define BIT_MSK__VND_IDL__REG_VHDL_IDL                                          0xFF

// Vendor ID High byte Register
#define REG_ADDR__VND_IDH                                                (REGTX_SOC_P0 | 0x0001)
// (ReadOnly, Bits 7:0)
// Vendor ID High byte (00h)
#define BIT_MSK__VND_IDH__REG_VHDL_IDH                                          0xFF

// Device ID Low byte Register
#define REG_ADDR__DEV_IDL                                                (REGTX_SOC_P0 | 0x0002)
// (ReadOnly, Bits 7:0)
// Device ID Low byte
#define BIT_MSK__DEV_IDL__REG_DEV_IDL                                           0xFF

// Device ID High byte Register
#define REG_ADDR__DEV_IDH                                                (REGTX_SOC_P0 | 0x0003)
// (ReadOnly, Bits 7:0)
// Device ID High
#define BIT_MSK__DEV_IDH__REG_DEV_IDH                                           0xFF

// Device Revision Register Low Byte
#define REG_ADDR__DEV_REVL                                               (REGTX_SOC_P0 | 0x0004)
// (ReadOnly, Bits 7:0)
// Device Revision Low byte
#define BIT_MSK__DEV_REVL__DEV_REV_IDL                                           0xFF

// Device Revision Register High Byte
#define REG_ADDR__DEV_REVH                                               (REGTX_SOC_P0 | 0x0005)
// (ReadOnly, Bits 7:0)
// Device Revision High byte
#define BIT_MSK__DEV_REVH__DEV_REV_IDH                                           0xFF

// SOC INTR Register
#define REG_ADDR__SOC_INTR                                               (REGTX_SOC_P0 | 0x0006)
// (ReadWrite, Bits 0)
// PRIF timeout
#define BIT_MSK__SOC_INTR__REG_SOC_INTR0                                         0x01

// SOC INTR Mask Register
#define REG_ADDR__SOC_INTRMASK                                           (REGTX_SOC_P0 | 0x0007)
// (ReadWrite, Bits 0)
// mask bit for SOC_INTR[0]
#define BIT_MSK__SOC_INTRMASK__REG_SOC_INTRMASK0                                     0x01

// Host Control1 Register
#define REG_ADDR__HOST_CTRL1                                             (REGTX_SOC_P0 | 0x0008)
// (ReadWrite, Bits 0)
// Soft reset to IP registers other than SOC. Active high.
#define BIT_MSK__HOST_CTRL1__REG_NONSOC_REG_RST                                    0x01

// Host Control2 Register
#define REG_ADDR__HOST_CTRL2                                             (REGTX_SOC_P0 | 0x0009)
// (ReadWrite, Bits 7)
// Interrupt polarity. 1 (default): inverted or INT active low; 0: original or INT active high.
#define BIT_MSK__HOST_CTRL2__REG_INTR_POL                                          0x80
// (ReadOnly, Bits 1)
// interrupt status for PRIF monitor
#define BIT_MSK__HOST_CTRL2__REG_INTR_STAT1                                        0x02
// (ReadOnly, Bits 0)
// Interrupt status for cypress_top
#define BIT_MSK__HOST_CTRL2__REG_INTR_STAT0                                        0x01

// OTP DBYTE510 Register
#define REG_ADDR__OTP_DBYTE510                                           (REGTX_SOC_P0 | 0x000A)
// (ReadOnly, Bits 7:0)
// OTP data byte at offset 510
#define BIT_MSK__OTP_DBYTE510__REG_OTP_DBYTE510                                      0xFF

// Function Select Register
#define REG_ADDR__FUNC_SEL                                               (REGTX_SOC_P0 | 0x000B)
// (ReadWrite, Bits 1)
// Enable MHL3 for Tx
#define BIT_MSK__FUNC_SEL__REG_MHL3_EN                                           0x02
// (ReadWrite, Bits 0)
// Enable HDMI for Tx
#define BIT_MSK__FUNC_SEL__REG_HDMI_EN                                           0x01

// System Control #1 Register
#define REG_ADDR__SYS_CTRL1                                              (REGTX_SOC_P0 | 0x000C)
// (ReadWrite, Bits 7)
// Enable bit to let SW override OTP HDCP1.4 Video control. Override value is reg_otpvmuteovr_set.
#define BIT_MSK__SYS_CTRL1__REG_OTP14VOVR_EN                                      0x80
// (ReadWrite, Bits 6)
// Enable bit to let SW override OTP HDCP1.4 Audio control. Override value is reg_otpamuteovr_set and reg_otpadropovr_set. These 2 bits can not be both 1.
#define BIT_MSK__SYS_CTRL1__REG_OTP14AOVR_EN                                      0x40
// (ReadWrite, Bits 5)
// Enable bit to let SW override OTP HDCP2.x Video control. Override value is reg_otpvmuteovr_set.
#define BIT_MSK__SYS_CTRL1__REG_OTP2XVOVR_EN                                      0x20
// (ReadWrite, Bits 4)
// Enable bit to let SW override OTP HDCP2.x Audio control. Override value is reg_otpamuteovr_set and reg_otpadropovr_set. These 2 bits can not be both 1.
#define BIT_MSK__SYS_CTRL1__REG_OTP2XAOVR_EN                                      0x10
// (ReadWrite, Bits 2)
// SW OTP HDCP 1.4 and 2.x Video Mute Override set bit
#define BIT_MSK__SYS_CTRL1__REG_OTPVMUTEOVR_SET                                   0x04
// (ReadWrite, Bits 1)
// SW OTP HDCP 1.4 and 2.x  Audio Drop Override set bit
#define BIT_MSK__SYS_CTRL1__REG_OTPADROPOVR_SET                                   0x02
// (ReadWrite, Bits 0)
// SW OTP HDCP 1.4 and 2.x Audio Mute Override set bit
#define BIT_MSK__SYS_CTRL1__REG_OTPAMUTEOVR_SET                                   0x01

// System Clock PWD Register
#define REG_ADDR__CLKPWD                                                 (REGTX_SOC_P0 | 0x000D)
// (ReadWrite, Bits 2)
// Gate off pclk. Active low. Default 0.
#define BIT_MSK__CLKPWD__REG_PDIDCK_N                                          0x04
// (ReadWrite, Bits 1)
// Gate off mhl1/2/3 clock. Active low. Default 0.
#define BIT_MSK__CLKPWD__REG_PD_MHL_CLK_N                                      0x02
// (ReadWrite, Bits 0)
// Gate off DIPT clock. Active low. Default 0.
#define BIT_MSK__CLKPWD__REG_PD_DIPT_CLK_N                                     0x01

// Clock Phase Control1 Register
#define REG_ADDR__CLKPHASE1                                              (REGTX_SOC_P0 | 0x000E)
// (ReadWrite, Bits 7)
// Selects tdm_lclk phase.   0 - Original phase; the same as it comes out of PLL (default) 1 - Invert tdm_lclk: change its phase 180 degrees
#define BIT_MSK__CLKPHASE1__REG_TDM_LCLK_PHASE                                    0x80
// (ReadWrite, Bits 6)
// Selects hsic_clk phase.   0 - Original phase; the same as it comes out of PLL (default) 1 - Invert hsic_clk: change its phase 180 degrees
#define BIT_MSK__CLKPHASE1__REG_HSIC_CLK_PHASE                                    0x40
// (ReadWrite, Bits 5)
// Selects cts_tck phase.   0 - Original phase; the same as it comes out of PLL (default) 1 - Invert cts_tck: change its phase 180 degrees
#define BIT_MSK__CLKPHASE1__REG_CTS_TCK_PHASE                                     0x20
// (ReadWrite, Bits 4)
// register bit to control the phase of iclk 0: no phase change (default) 1: inverted
#define BIT_MSK__CLKPHASE1__REG_IDCK_PHASE                                        0x10
// (ReadWrite, Bits 3)
// register bit to control the phase of keeper_clk 0: no phase change (default) 1: inverted
#define BIT_MSK__CLKPHASE1__REG_KEEPER_CLK_PHASE                                  0x08
// (ReadWrite, Bits 2)
// register bit to control the phase of fc_clk 0: no phase change (default) 1: inverted
#define BIT_MSK__CLKPHASE1__REG_FC_CLK_PHASE                                      0x04
// (ReadWrite, Bits 1)
// Selects hsic_rx_strobe phase.   0 - Original phase; the same as it comes out of HSIC Strobe pin (default) 1 - Invert hsic_rx_strobe: change its phase 180 degrees
#define BIT_MSK__CLKPHASE1__REG_HSIC_RX_STROBE_PHASE                              0x02
// (ReadWrite, Bits 0)
// register bit to control the phase of mif_clk 0: no phase change (default) 1: inverted
#define BIT_MSK__CLKPHASE1__REG_MIF_CLK_PHASE                                     0x01

// Clock Phase Control2 Register
#define REG_ADDR__CLKPHASE2                                              (REGTX_SOC_P0 | 0x000F)
// (ReadWrite, Bits 1)
// register bit to control the phase of PCLKNX 0: no phase change (default) 1: inverted
#define BIT_MSK__CLKPHASE2__REG_PCLKNX_PHASE                                      0x02
// (ReadWrite, Bits 0)
// register bit to control the phase of PCLK 0: no phase change (default) 1: inverted
#define BIT_MSK__CLKPHASE2__REG_PCLK_PHASE                                        0x01

// Software Reset Register
#define REG_ADDR__PWD_SRST                                               (REGTX_SOC_P0 | 0x0010)
// (ReadWrite, Bits 0)
// Software Reset. Reset all internal logic; except register interface; hdcp; and EEPROM interface.  Note: asserting Software Reset will not reset writable register contents: 1 - Reset 0 - Normal operation (default).
#define BIT_MSK__PWD_SRST__REG_SW_RST                                            0x01

// SoC SW Reset2 Register
#define REG_ADDR__SW_RST2                                                (REGTX_SOC_P0 | 0x0011)
// (ReadWrite, Bits 3)
// Select MHL3 CTS operation mode 1 - 3 lanes (default) 0 - 2 lanes
#define BIT_MSK__SW_RST2__REG_MHL3CTS_3LANE                                     0x08

// SW Reset #3 Register
#define REG_ADDR__SW_RST3                                                (REGTX_SOC_P0 | 0x0012)
// (ReadWrite, Bits 7)
// Soft reset for CEC only, active high.
#define BIT_MSK__SW_RST3__REG_CEC_RST                                           0x80
// (ReadWrite, Bits 2)
// Reset MHL3 CTS FIFO when there is overflow or underflow happens 0 - Disable 1 - Enable (default)
#define BIT_MSK__SW_RST3__REG_CTS_FIFO_AUTO_RST_EN                              0x04
// (ReadWrite, Bits 1)
// Reset timing closure FIFO when there is overflow or underflow happens 0 - Disable (default) 1 - Enable
#define BIT_MSK__SW_RST3__REG_TCF_AUTO_RST_EN                                   0x02
// (ReadWrite, Bits 0)
// Reset MHL FIFO when there is overflow or underflow happens 0 - Disable 1 - Enable (default)
#define BIT_MSK__SW_RST3__REG_MHL_FIFO_AUTO_RST_EN                              0x01

// SoC MISC Register
#define REG_ADDR__SYS_MISC                                               (REGTX_SOC_P0 | 0x0013)
// (ReadWrite, Bits 7:2)
// bit[0]: bypass tclknx bit[1]: bypass hsic_clk bit[2]:  bypass mif_clk  bit[3]:  bypass mhl_clk; bit[4]:  bypass fb_clk;  bit[5]:  bypass ref_clk;
#define BIT_MSK__SYS_MISC__REG_BYPASS_PLL_CLK                                    0xFC
// (ReadWrite, Bits 1)
// Edge select for SPI MISO.  1'b0: MISO is launched by the falling edge of spi_sclk 1'b1: MISO is launched by the rising edge of spi_sclk
#define BIT_MSK__SYS_MISC__REG_SPI_MISO_EDGE                                     0x02
// (ReadWrite, Bits 0)
// Edge select for SPI MOSI. 1'b0: MOSI is latched by the rising edge of spi_sclk 1'b1: MOSI is latched by the falling edge of spi_sclk
#define BIT_MSK__SYS_MISC__REG_SPI_MOSI_EDGE                                     0x01

// SoC MISC2 Register
#define REG_ADDR__SYS_MISC2                                              (REGTX_SOC_P0 | 0x0014)
// (ReadWrite, Bits 5)
// Enable I2C communicate with EMSC instead of SPI
#define BIT_MSK__SYS_MISC2__REG_I2C_TO_EMSC_EN                                    0x20
// (ReadWrite, Bits 3)
// Choose osc_clk_dvd_1024 if set 1. Choose osc_clk_dvd_2 if set 0.
#define BIT_MSK__SYS_MISC2__REG_60K_SEL                                           0x08
// (ReadWrite, Bits 2)
// Bypass OCLK and cbus_disc_clk; use GPIO[3] for cbus_disc_clk; use RPWR for oclk
#define BIT_MSK__SYS_MISC2__REG_BYP_OCLKRING_OPTION                               0x04
// (ReadWrite, Bits 1:0)
// OCLK Divide 00: by 2 (default); 01; by 4; 10: by 8; 11: by 16
#define BIT_MSK__SYS_MISC2__REG_OCLKDIV                                           0x03

// SoC MISC3 Register
#define REG_ADDR__SYS_MISC3                                              (REGTX_SOC_P0 | 0x0015)
// (ReadWrite, Bits 4)
// OTP_PD bit
#define BIT_MSK__SYS_MISC3__REG_OTP_PD                                            0x10
// (ReadWrite, Bits 3)
// OTP_PS bit
#define BIT_MSK__SYS_MISC3__REG_OTP_PS                                            0x08
// (ReadOnly, Bits 2:1)
// OTP status for HDCP Audio Mute option.  00 indicates when HDCP encryption off; audio is sent as-is. 01 indicates when HDCP encryption off; audio will be blank.  10 indicates when HDCP encryption off; audio related packets or InfoFrames will be dropped.  11 reserved
#define BIT_MSK__SYS_MISC3__REG_OTP_HDCPADRPMUT                                   0x06
// (ReadOnly, Bits 0)
// OTP status for HDCP Video Mute option. 1 indicates when HDCP encryption off video will be blank. 0 indicates when HDCP encryption off video is sent as-is.
#define BIT_MSK__SYS_MISC3__REG_OTP_HDCPVMUTE                                     0x01

// PRIF Control Register
#define REG_ADDR__PRIFCNTL                                               (REGTX_SOC_P0 | 0x0016)
// (ReadWrite, Bits 7)
// This register enables or disables PRIF timeout. 0: disable (default) 1: enable
#define BIT_MSK__PRIFCNTL__REG_PRIF_TOUT_EN                                      0x80
// (ReadWrite, Bits 6)
// This register can enable PRIF timeout for simulation mode. PRIF timeout is allowed in 160us.  0: disable (default) 1: enable
#define BIT_MSK__PRIFCNTL__REG_PRIF_TOUT_SMODE                                   0x40
// (ReadWrite, Bits 1:0)
// This register defines PRIF timeout criteria. If par_rdy stuck low for too long; an error will be generated and may be reflected by INT. 00: 10ms (default); 01: 20ms; 10: 40ms; 11: 80ms
#define BIT_MSK__PRIFCNTL__REG_PRIF_TOUT_SEL                                     0x03

// I2C Address for Page 1 Register
#define REG_ADDR__PAGE_1_ADDR                                            (REGTX_SOC_P0 | 0x0017)
// (ReadWrite, Bits 7:0)
// I2C address for Page 1
#define BIT_MSK__PAGE_1_ADDR__REG_PAGE_1_ADDR_B7_B0                                 0xFF

// I2C Address for Page 2 Register
#define REG_ADDR__PAGE_2_ADDR                                            (REGTX_SOC_P0 | 0x0018)
// (ReadWrite, Bits 7:0)
// I2C address for Page 2
#define BIT_MSK__PAGE_2_ADDR__REG_PAGE_2_ADDR_B7_B0                                 0xFF

// I2C Address for Page 3 Register
#define REG_ADDR__PAGE_3_ADDR                                            (REGTX_SOC_P0 | 0x0019)
// (ReadWrite, Bits 7:0)
// I2C address for Page 3
#define BIT_MSK__PAGE_3_ADDR__REG_PAGE_3_ADDR_B7_B0                                 0xFF

// I2C Address for Page MHLSpec Register
#define REG_ADDR__PAGE_MHLSPEC_ADDR                                      (REGTX_SOC_P0 | 0x001A)
// (ReadWrite, Bits 7:0)
// I2C address for Page MHLSPEC
#define BIT_MSK__PAGE_MHLSPEC_ADDR__REG_PAGE_MHLSPEC_ADDR_B7_B0                           0xFF

// CBUS Address Register
#define REG_ADDR__PAGE_CBUS_ADDR                                         (REGTX_SOC_P0 | 0x001B)
// (ReadWrite, Bits 7:0)
// I2C address for Page CBUS
#define BIT_MSK__PAGE_CBUS_ADDR__REG_PAGE_CBUS_ADDR_B7_B0                              0xFF

// I2C Address for HW TPI Register
#define REG_ADDR__HW_TPI_ADDR                                            (REGTX_SOC_P0 | 0x001C)
// (ReadWrite, Bits 7:0)
// I2C address for HW TPI
#define BIT_MSK__HW_TPI_ADDR__REG_HW_TPI_ADDR_B7_B0                                 0xFF

// I2C Address for Page 7 Register
#define REG_ADDR__PAGE_7_ADDR                                            (REGTX_SOC_P0 | 0x001D)
// (ReadWrite, Bits 7:0)
// I2C address for Page 7
#define BIT_MSK__PAGE_7_ADDR__REG_PAGE_7_ADDR_B7_B0                                 0xFF

// I2C Address for Page 8 Register
#define REG_ADDR__PAGE_8_ADDR                                            (REGTX_SOC_P0 | 0x001E)
// (ReadWrite, Bits 7:0)
// I2C address for Page 8
#define BIT_MSK__PAGE_8_ADDR__REG_PAGE_8_ADDR_B7_B0                                 0xFF

// I2C Address for Page 9 Register
#define REG_ADDR__PAGE_9_ADDR                                            (REGTX_SOC_P0 | 0x001F)
// (ReadWrite, Bits 7:0)
// I2C address for Page 9
#define BIT_MSK__PAGE_9_ADDR__REG_PAGE_9_ADDR_B7_B0                                 0xFF

// I2C Address for Page 10 Register
#define REG_ADDR__PAGE_10_ADDR                                           (REGTX_SOC_P0 | 0x0020)
// (ReadWrite, Bits 7:0)
// I2C address for Page 10
#define BIT_MSK__PAGE_10_ADDR__REG_PAGE_10_ADDR_B7_B0                                0xFF

// I2C Address for Page 11 Register
#define REG_ADDR__PAGE_11_ADDR                                           (REGTX_SOC_P0 | 0x0021)
// (ReadWrite, Bits 7:0)
// I2C address for Page 11
#define BIT_MSK__PAGE_11_ADDR__REG_PAGE_11_ADDR_B7_B0                                0xFF

// I2C Address for Page 12 Register
#define REG_ADDR__PAGE_12_ADDR                                           (REGTX_SOC_P0 | 0x0022)
// (ReadWrite, Bits 7:0)
// I2C address for Page 12
#define BIT_MSK__PAGE_12_ADDR__REG_PAGE_12_ADDR_B7_B0                                0xFF

// I2C Address for Page 13 Register
#define REG_ADDR__PAGE_13_ADDR                                           (REGTX_SOC_P0 | 0x0023)
// (ReadWrite, Bits 7:0)
// I2C address for Page 13
#define BIT_MSK__PAGE_13_ADDR__REG_PAGE_13_ADDR_B7_B0                                0xFF

// I2C Address for Page 14 Register
#define REG_ADDR__PAGE_14_ADDR                                           (REGTX_SOC_P0 | 0x0024)
// (ReadWrite, Bits 7:0)
// I2C address for Page 14
#define BIT_MSK__PAGE_14_ADDR__REG_PAGE_14_ADDR_B7_B0                                0xFF

// I2C Address for Page 15 Register
#define REG_ADDR__PAGE_15_ADDR                                           (REGTX_SOC_P0 | 0x0025)
// (ReadWrite, Bits 7:0)
// I2C address for Page 15
#define BIT_MSK__PAGE_15_ADDR__REG_PAGE_15_ADDR_B7_B0                                0xFF

// IP RegRST Register 0
#define REG_ADDR__IPREGRST0                                              (REGTX_SOC_P0 | 0x0026)
// (ReadWrite, Bits 7:0)
// IP specific reset for register and PRIF decoding bit 0: AIP reset bit 1: AVLINK reset bit 2: CBUS_DISC reset bit 3: CBUS reset bit 4: reserved bit 5: COC_DOC reset bit 6: DPLL reset bit 7: eMSC reset
#define BIT_MSK__IPREGRST0__REG_IPREGRST0                                         0xFF

// IP RegRST Register 1
#define REG_ADDR__IPREGRST1                                              (REGTX_SOC_P0 | 0x0027)
// (ReadWrite, Bits 7:0)
// IP specific reset for register and PRIF decoding bit 0: Harry reset bit 1: HDCP2 reset bit 2: HDMI2 reset bit 3: HDMI2MHL1 reset bit 4: HDMI2MHL3 reset bit 5: HSIC_BIST reset bit 6: HSIC_PHY reset bit 7: DS_EDID reset
#define BIT_MSK__IPREGRST1__REG_IPREGRST1                                         0xFF

// IP RegRST Register 2
#define REG_ADDR__IPREGRST2                                              (REGTX_SOC_P0 | 0x0028)
// (ReadWrite, Bits 7:0)
// IP specific reset for register and PRIF decoding bit 0: MHL_TX_TOP reset bit 1: P0 reset bit 2: PTPI reset bit 3:  TOP reset bit 4: TDMCORE reset bit 5: USBTCORE reset bit 6: MCU reset
#define BIT_MSK__IPREGRST2__REG_IPREGRST2                                         0xFF

// IP RegRST Register 3
#define REG_ADDR__IPREGRST3                                              (REGTX_SOC_P0 | 0x0029)
// (ReadWrite, Bits 7:0)
// IP specific reset for register and PRIF decoding
#define BIT_MSK__IPREGRST3__REG_IPREGRST3                                         0xFF

// Top Interrupt Register
#define REG_ADDR__TOP_INTR                                               (REGTX_SOC_P0 | 0x002C)
// (ReadOnly, Bits 2)
// Interrupt status for avlink  wrapper IP
#define BIT_MSK__TOP_INTR__REG_AVLINK_INTR                                       0x04
// (ReadOnly, Bits 1)
// Interrupt status for cocdoc wrapper IP
#define BIT_MSK__TOP_INTR__REG_COCDOC_INTR                                       0x02
// (ReadOnly, Bits 0)
// Interrupt status at top level
#define BIT_MSK__TOP_INTR__REG_INTR_TOP                                          0x01

// IP MISC Register
#define REG_ADDR__DEBUG_MODE_EN                                          (REGTX_SOC_P0 | 0x0030)
// (ReadOnly, Bits 3)
// The current status of the VSYNC input pin. Monitor this bit with I2C to recognize the arrival of VSYNC pulses.
#define BIT_MSK__DEBUG_MODE_EN__VSYNCPIN                                              0x08
// (ReadWrite, Bits 2)
// Enabled to block DDC access when HPD is low
#define BIT_MSK__DEBUG_MODE_EN__REG_BLOCK_DDC_BY_HPD                                  0x04
// (ReadWrite, Bits 1)
// Debug Mode 3 Enable 0: Disable 1: vsync; p_stable; HPD (facing upstream); RSEN; hdcp_ri_rdy; sup_an_stop; hdcp_bksv_err; and hdcp_enc_on are available from registers.
#define BIT_MSK__DEBUG_MODE_EN__REG_DBGMODE3_EN                                       0x02
// (ReadWrite, Bits 0)
// Simulation mode. When enabled; time constant used is shortened. 0: disable (default) 1: enable
#define BIT_MSK__DEBUG_MODE_EN__REG_SIMODE                                            0x01

// System Status Register
#define REG_ADDR__SYS_STAT                                               (REGTX_SOC_P0 | 0x0031)
// (ReadWrite, Bits 7:4)
// This register defines a timer. After timer expiration IP prif_ready will be forced high for registers located in tclk domain. 4'h0: reserved    4'h1: 4 cycles;    4'h2: 8 cycles;    4'h3: 16 cyles;  4'h4: 32 cycles;   4'h5: 64 cycles;  4'h6: 128 cycles; 4'h7: 256 cyles; 4'h8: 512 cycles;   4'h9: 1k cycles;  4'hA: 2k cycles; 4'hB: 4k cyles; 4'hC: 8k cycles (default);   4'hD: 16k cycles;  4'hE: 32k cycles; 4'hF: 64k cyles;
#define BIT_MSK__SYS_STAT__REG_FORCERDY_SEL                                      0xF0
// (ReadWrite, Bits 3)
// Set this bit will force IP prif_ready becomes 1 after certain timeout, for register byte located in tclk domain while tclk is not provided by the system.
#define BIT_MSK__SYS_STAT__REG_FORCERDY_EN                                       0x08
// (ReadOnly, Bits 2)
// This bit is HIGH if a powered on receiver is connected to the transmitter outputs; LOW otherwise. This function is only available for use in DC-coupled systems.  Rsen is active even if SYS_CTRL1's PD bit is 0 (in other words; rsen is available during power down).
#define BIT_MSK__SYS_STAT__RSEN                                                  0x04
// (ReadOnly, Bits 1)
// Upstream Hot Plug Detect status. This is the status manipulated by FW based on CBUS HPD and need
#define BIT_MSK__SYS_STAT__HPDPIN                                                0x02
// (ReadOnly, Bits 0)
// IDCK to TMDS clock stable.  The state of this internal signal can be read from this bit.  Upon changes to the IDCK and/or TMDS_CTRL register's tclk_sel bits; this bit can go low.  After a low to high transition; a software reset is recommended (unless auto-reset is enabled).  Note: This bit was previously called MDI in earlier Deco transmitters.
#define BIT_MSK__SYS_STAT__P_STABLE                                              0x01

// System Control #3 Register
#define REG_ADDR__SYS_CTRL3                                              (REGTX_SOC_P0 | 0x0032)
// (ReadWrite, Bits 5)
// 1: system wait counter time of 5000ms(needed for HDCP) and 2000ms(needed for CBUS) will start 0: counter will not start
#define BIT_MSK__SYS_CTRL3__REG_SYS_CNTR                                          0x20
// (ReadWrite, Bits 2:1)
// General Purpose Control. These CTL bit states are transmitted across the TMDS link during blanking times only. DVI 1.0 mode only.A754
#define BIT_MSK__SYS_CTRL3__REG_CTL                                               0x06
// (ReadWrite, Bits 0)
// When set to 1'b1; 0x61A[0] will decide whether output HDMI or DVI
#define BIT_MSK__SYS_CTRL3__REG_TX_CONTROL_HDMI                                   0x01

// System Control DPD Register
#define REG_ADDR__DPD                                                    (REGTX_SOC_P0 | 0x0033)
// (ReadWrite, Bits 7)
// Power on TX PLL
#define BIT_MSK__DPD__REG_PWRON_PLL                                         0x80
// (ReadWrite, Bits 6)
// Power on MHL Tx data path
#define BIT_MSK__DPD__REG_PWRON_DP                                          0x40
// (ReadWrite, Bits 5)
// Power on ETMDS
#define BIT_MSK__DPD__REG_PWRON_ETMDS                                       0x20
// (ReadWrite, Bits 4)
// Power Down internal oscillator; effectively disabling Master I2C; Slave I2C; Charge Pump; and preventing uploading of new vectors from EPROM 0 - Power down.   1 - normal operation (default).
#define BIT_MSK__DPD__REG_OSC_EN                                            0x10
// (ReadWrite, Bits 3)
// Power on HSIC
#define BIT_MSK__DPD__REG_PWRON_HSIC                                        0x08

// System Control #4 Register
#define REG_ADDR__SYS_CTRL4                                              (REGTX_SOC_P0 | 0x0034)
// (ReadWrite, Bits 2)
// This bit can select between Local I2C (including internal DDC master) and DDC port.  0 - Use Local I2C as a source port (default) 1 - Use DDC port as a source port
#define BIT_MSK__SYS_CTRL4__REG_EXT_DDC_SEL                                       0x04
// (ReadWrite, Bits 1)
//
#define BIT_MSK__SYS_CTRL4__REG_HSIC_TX_BIST_START_SEL                            0x02
// (ReadWrite, Bits 0)
// HDMI TransCode Mode enable bit. When set 1; HDMI stream will bypass internal process and  be sent to MHL TX directly. 0 - disable (default) 1 - enable
#define BIT_MSK__SYS_CTRL4__REG_TRANSCODE                                         0x01

// MCU Timeout Register
#define REG_ADDR__MCUTOUT                                                (REGTX_SOC_P0 | 0x0035)
// (ReadWrite, Bits 7:0)
// This register defines in how many OCLK cycles MCU timeout will occur if there is no response for the request. The granularity is 256.
#define BIT_MSK__MCUTOUT__REG_MCU_TIMEOUT                                       0xFF

// HDCP Control Register
#define REG_ADDR__HDCP_CTRL                                              (REGTX_SOC_P0 | 0x003F)
// (ReadOnly, Bits 6)
// This bit is set to 1 whenever the encryption is enabled and encryption is in process.
#define BIT_MSK__HDCP_CTRL__HDCP_ENC_ON                                           0x40
// (ReadOnly, Bits 5)
// BKSV Error.    This bit is cleared (set to 0) when: 1. writing the last byte of the BKSV . 2. Hardware reset through the RESET pin  3. By setting cp_reset bit (reg. 0x00F[2])  It is set when load of the keys is done and BKSV did not had 20 ones.
#define BIT_MSK__HDCP_CTRL__HDCP_BKSV_ERR                                         0x20
// (ReadWrite, Bits 4)
// Set this bit prior to authentication if the receiver is a repeater.
#define BIT_MSK__HDCP_CTRL__REG_RX_RPTR                                           0x10
// (ReadWrite, Bits 3)
// When cleared; the cipher engine is allowed to free-run; and the WR_AN registers will cycle through a sequence of pseudo-random values.  This bit must be set for normal cipher operation.  When it is; the value read from the WR_AN registers may be used to initialize the AN register in the receiver.                                                This bit is automatically cleared under any of the following conditions:           - Software write 0 into it  - Hardware reset   - BKSV_ERR     - RX_RPTR is changed                                                                                     Note that either external hardware or software should detect when a device is not plugged in; and assert (= 1) this bit in such cases.
#define BIT_MSK__HDCP_CTRL__REG_AN_STOP                                           0x08
// (ReadWrite, Bits 2)
// This is the reset bit for the cipher engine.  This bit is asserted by hardware reset or software.  It can be de-asserted (= 1) at any time.
#define BIT_MSK__HDCP_CTRL__REG_CP_RESETN                                         0x04
// (ReadOnly, Bits 1)
// Ri Ready.  This bit indicates that the first Ri value is available.  This bit is cleared by a hardware reset; cp_reset (reg. 0x009[2]) or by writing the last byte of the BKSV. It is set when finish loading the last byte of the KEYS.
#define BIT_MSK__HDCP_CTRL__HDCP_RI_RDY                                           0x02
// (ReadWrite, Bits 0)
// Encryption Enable.                                               0 - Encryption disabled (default) 1 - Encryption enabled     Note: This bit can now be written to a 0.  If a 1 to 0 transition occurs; appropriate security measures are taken.
#define BIT_MSK__HDCP_CTRL__REG_ENC_EN                                            0x01

// Write BKSV1 Register
#define REG_ADDR__WR_BKSV_1                                              (REGTX_SOC_P0 | 0x0040)
// (ReadWrite, Bits 7:0)
// Reciever's Key Select Vector. It is read from the reciever and then writen to here. When software writes byte #1 of Key Selector Vector in this register the 1 clock write strobe is generated and data is loaded into the bksv1 register located inside HDCP block.  During read get Data of BKSV #1 register.
#define BIT_MSK__WR_BKSV_1__REG_BKSV0                                             0xFF

// Write BKSV2 Register
#define REG_ADDR__WR_BKSV_2                                              (REGTX_SOC_P0 | 0x0041)
// (ReadWrite, Bits 7:0)
// Reciever's Key Select Vector. It is read from the reciever and then writen to here. When software writes byte #2 of Key Selector Vector in this register the 1 clock write strobe is generated and data is loaded into the bksv2 register located inside HDCP block.  During read get Data of BKSV #2 register.
#define BIT_MSK__WR_BKSV_2__REG_BKSV1                                             0xFF

// Write BKSV3 Register
#define REG_ADDR__WR_BKSV_3                                              (REGTX_SOC_P0 | 0x0042)
// (ReadWrite, Bits 7:0)
// Reciever's Key Select Vector. It is read from the reciever and then writen to here. When software writes byte #3 of Key Selector Vector in this register the 1 clock write strobe is generated and data is loaded into the bksv3 register located inside HDCP block.  During read get Data of BKSV #3 register.
#define BIT_MSK__WR_BKSV_3__REG_BKSV2                                             0xFF

// Write BKSV4 Register
#define REG_ADDR__WR_BKSV_4                                              (REGTX_SOC_P0 | 0x0043)
// (ReadWrite, Bits 7:0)
// Reciever's Key Select Vector. It is read from the reciever and then writen to here. When software writes byte #4 of Key Selector Vector in this register the 1 clock write strobe is generated and data is loaded into the bksv4 register located inside HDCP block.  During read get Data of BKSV #4 register.
#define BIT_MSK__WR_BKSV_4__REG_BKSV3                                             0xFF

// Write BKSV5 Register
#define REG_ADDR__WR_BKSV_5                                              (REGTX_SOC_P0 | 0x0044)
// (ReadWrite, Bits 7:0)
// Reciever's Key Select Vector. It is read from the reciever and then writen to here. When software writes byte #5 of Key Selector Vector in this register the 1 clock write strobe is generated and data is loaded into the bksv5 register located inside HDCP block. In addition if HDCP is enabled then by writing into this register the authentication is triggered.  During read get Data of BKSV #5 register.
#define BIT_MSK__WR_BKSV_5__REG_BKSV4                                             0xFF

// HDCP AN_1 Register
#define REG_ADDR__AN1                                                    (REGTX_SOC_P0 | 0x0045)
// (ReadWrite, Bits 7:0)
// Byte #1 of a 64-bit pseudo-random value. May be read from this register and used in the authentication process. Also can write into this register
#define BIT_MSK__AN1__HDCP_AN0                                              0xFF

// HDCP AN_2 Register
#define REG_ADDR__AN2                                                    (REGTX_SOC_P0 | 0x0046)
// (ReadWrite, Bits 7:0)
// Byte #2 of a 64-bit pseudo-random value. May be read from this register and used in the authentication process. Also can write into this register
#define BIT_MSK__AN2__HDCP_AN1                                              0xFF

// HDCP AN_3 Register
#define REG_ADDR__AN3                                                    (REGTX_SOC_P0 | 0x0047)
// (ReadWrite, Bits 7:0)
// Byte #3 of a 64-bit pseudo-random value. May be read from this register and used in the authentication process. Also can write into this register
#define BIT_MSK__AN3__HDCP_AN2                                              0xFF

// HDCP AN_4 Register
#define REG_ADDR__AN4                                                    (REGTX_SOC_P0 | 0x0048)
// (ReadWrite, Bits 7:0)
// Byte #4 of a 64-bit pseudo-random value. May be read from this register and used in the authentication process. Also can write into this register
#define BIT_MSK__AN4__HDCP_AN3                                              0xFF

// HDCP AN_5 Register
#define REG_ADDR__AN5                                                    (REGTX_SOC_P0 | 0x0049)
// (ReadWrite, Bits 7:0)
// Byte #5 of a 64-bit pseudo-random value. May be read from this register and used in the authentication process. Also can write into this register
#define BIT_MSK__AN5__HDCP_AN4                                              0xFF

// HDCP AN_6 Register
#define REG_ADDR__AN6                                                    (REGTX_SOC_P0 | 0x004A)
// (ReadWrite, Bits 7:0)
// Byte #6 of a 64-bit pseudo-random value. May be read from this register and used in the authentication process. Also can write into this register
#define BIT_MSK__AN6__HDCP_AN5                                              0xFF

// HDCP AN_7 Register
#define REG_ADDR__AN7                                                    (REGTX_SOC_P0 | 0x004B)
// (ReadWrite, Bits 7:0)
// Byte #7 of a 64-bit pseudo-random value. May be read from this register and used in the authentication process.  Also can write into this register.
#define BIT_MSK__AN7__HDCP_AN6                                              0xFF

// HDCP AN_8 Register
#define REG_ADDR__AN8                                                    (REGTX_SOC_P0 | 0x004C)
// (ReadWrite, Bits 7:0)
// Byte #8 of a 64-bit pseudo-random value. May be read from this register and used in the authentication process. Also can write into this register
#define BIT_MSK__AN8__HDCP_AN7                                              0xFF

// AKSV_1 Register
#define REG_ADDR__AKSV_1                                                 (REGTX_SOC_P0 | 0x004D)
// (ReadOnly, Bits 7:0)
// Byte #1 of transmitters key select vector.  Five bytes: 1 is the LSB; 5 is the MSB.  All five bytes should be read from here and then written to the receiver.  Byte 5 should be written last into the receiver; and will trigger the authentication process in the receiver.
#define BIT_MSK__AKSV_1__REG_AKSV0                                             0xFF

// AKSV_2 Register
#define REG_ADDR__AKSV_2                                                 (REGTX_SOC_P0 | 0x004E)
// (ReadOnly, Bits 7:0)
// Byte #2 of transmitters key select vector.  Five bytes: 1 is the LSB; 5 is the MSB.  All five bytes should be read from here and then written to the receiver.  Byte 5 should be written last into the receiver; and will trigger the authentication process in the receiver.
#define BIT_MSK__AKSV_2__REG_AKSV1                                             0xFF

// AKSV_3 Register
#define REG_ADDR__AKSV_3                                                 (REGTX_SOC_P0 | 0x004F)
// (ReadOnly, Bits 7:0)
// Byte #3 of transmitters key select vector.  Five bytes: 1 is the LSB; 5 is the MSB.  All five bytes should be read from here and then written to the receiver.  Byte 5 should be written last into the receiver; and will trigger the authentication process in the receiver.
#define BIT_MSK__AKSV_3__REG_AKSV2                                             0xFF

// AKSV_4 Register
#define REG_ADDR__AKSV_4                                                 (REGTX_SOC_P0 | 0x0050)
// (ReadOnly, Bits 7:0)
// Byte #4 of transmitters key select vector.  Five bytes: 1 is the LSB; 5 is the MSB.  All five bytes should be read from here and then written to the receiver.  Byte 5 should be written last into the receiver; and will trigger the authentication process in the receiver.
#define BIT_MSK__AKSV_4__REG_AKSV3                                             0xFF

// AKSV_5 Register
#define REG_ADDR__AKSV_5                                                 (REGTX_SOC_P0 | 0x0051)
// (ReadOnly, Bits 7:0)
// Byte #5 of transmitters key select vector.  Five bytes: 1 is the LSB; 5 is the MSB.  All five bytes should be read from here and then written to the receiver.  Byte 5 should be written last into the receiver; and will trigger the authentication process in the receiver.
#define BIT_MSK__AKSV_5__REG_AKSV4                                             0xFF

// Ri_1 Register
#define REG_ADDR__RI_1                                                   (REGTX_SOC_P0 | 0x0052)
// (ReadOnly, Bits 7:0)
// Ri value.  The values of these registers should be read and compared against the Ri value of the receiver to ensure that the encryption process on the transmitter and receiver are synchronized.  with vp_sel = 1 (page 0 0xEC bit 6); the status is from HW TPI; with vp_sel = 0; the status from normal path.
#define BIT_MSK__RI_1__REG_RI_PRIME0                                         0xFF

// Ri_2 Register
#define REG_ADDR__RI_2                                                   (REGTX_SOC_P0 | 0x0053)
// (ReadOnly, Bits 7:0)
// Ri value.  The values of these registers should be read and compared against the Ri value of the receiver to ensure that the encryption process on the transmitter and receiver are synchronized. with vp_sel = 1 (page 0 0xEC bit 6); the status is from HW TPI; with vp_sel = 0; the status from normal path.
#define BIT_MSK__RI_2__REG_RI_PRIME1                                         0xFF

// HDCP Ri 128 Compare Value Register
#define REG_ADDR__RI_128_COMP                                            (REGTX_SOC_P0 | 0x0054)
// (ReadWrite, Bits 6:0)
// HDCP Ri 128 Compare Value
#define BIT_MSK__RI_128_COMP__REG_RI_128_COMP                                       0x7F

// HDCP I counter Register
#define REG_ADDR__HDCP_I_CNT                                             (REGTX_SOC_P0 | 0x0055)
// (ReadOnly, Bits 6:0)
// I counter
#define BIT_MSK__HDCP_I_CNT__HDCP_I_CNT                                            0x7F

// Ri Status Register
#define REG_ADDR__RI_STAT                                                (REGTX_SOC_P0 | 0x0056)

// Ri Command Register
#define REG_ADDR__RI_CMD                                                 (REGTX_SOC_P0 | 0x0057)

// Ri Line Start Register
#define REG_ADDR__RI_START                                               (REGTX_SOC_P0 | 0x0058)
// (ReadWrite, Bits 7:0)
// Indicates at what line within frame 127 or 0 to start Ri Check. This number is power of 2 - 2 lsb are 0; default is 17.
#define BIT_MSK__RI_START__REG_RI_LN_NUM                                         0xFF

// Ri from Rx #1 Register
#define REG_ADDR__RI_RX_1                                                (REGTX_SOC_P0 | 0x0059)

// Ri from Rx #2 Register
#define REG_ADDR__RI_RX_2                                                (REGTX_SOC_P0 | 0x005A)

// HDCP Debug Register
#define REG_ADDR__TXHDCP_DEBUG                                           (REGTX_SOC_P0 | 0x005B)
// (ReadWrite, Bits 7)
// Setting this bit to 1 will force trush of Ri values (it will be set to 0)
#define BIT_MSK__TXHDCP_DEBUG__REG_RI_TRUSH                                          0x80
// (ReadWrite, Bits 6)
// Setting this bit to 1 will force stop update of the Ri values
#define BIT_MSK__TXHDCP_DEBUG__REG_RI_HOLD                                           0x40

// CLK Ratio Control Register
#define REG_ADDR__CLKRATIO                                               (REGTX_SOC_P0 | 0x0065)
// (ReadWrite, Bits 7)
// This register enables SW clock ratios; reg_idck2pclk_ratio and reg_pclknx2pclk_ratio.  1: enable SW clock ratio (default) 0: enable HW decoded clock ratio
#define BIT_MSK__CLKRATIO__REG_CLKRATIO_SW_EN                                    0x80
// (ReadWrite, Bits 3:2)
// This register defines the ratio between PCLKNX and PCLK when reg_clkratio_sw_en is set. This is a special SWHR register. SW can write. When read; it is HW value decoded based on video_path_core configuration. 00: PCLKNX is half rate as PCLK. 01: PCLKNX is the same rate as PCLK (default) 10: PCLKNX is twice as fast as PCLK 11: PCLKNX is four times as fast as PCLK.
#define BIT_MSK__CLKRATIO__REG_PCLKNX2PCLK_RATIO                                 0x0C
// (ReadWrite, Bits 1:0)
// This register defines the ratio between IDCK and PCLK when reg_clkratio_sw_en is set. This is a special SWHR register. SW can write. When read; it is HW value decoded based on video_path_core configuration. 00: IDCK is half rate as PCLK. 01: IDCK is the same rate as PCLK (default) 10: Customized relationship driven by SoC without HW automation 11: reserved
#define BIT_MSK__CLKRATIO__REG_IDCK2PCLK_RATIO                                   0x03

// PCLK2TCLK Control Register
#define REG_ADDR__P2T_CTRL                                               (REGTX_SOC_P0 | 0x0066)
// (ReadWrite, Bits 7)
// Deep Color Packet Enable bit 1: enable(default) 0: disable
#define BIT_MSK__P2T_CTRL__REG_DC_PKT_EN                                         0x80
// (ReadWrite, Bits 6)
// Null Packet Enable bit 1: enable(default) 0: disable
#define BIT_MSK__P2T_CTRL__REG_NULL_PKT_EN                                       0x40
// (ReadWrite, Bits 5)
// Null Packet Enable at VSYNC High 1: enable(default) 0: disable
#define BIT_MSK__P2T_CTRL__REG_NULL_PKT_EN_VS_HI                                 0x20
// (ReadWrite, Bits 1:0)
// PACK Mode 00:  8bpp (default) 01: 10bpp 10: 12bpp 11: 16bpp
#define BIT_MSK__P2T_CTRL__REG_PACK_MODE                                         0x03

// Video Blank Data Low Byte Register
#define REG_ADDR__VID_BLANK0                                             (REGTX_SOC_P0 | 0x0067)
// (ReadWrite, Bits 7:0)
// This byte defines the blank data for channel B/Cb. Default is 0.
#define BIT_MSK__VID_BLANK0__REG_VIDEO_MUTE_DATA_B0                                0xFF

// Video Blank Data Mid Byte Register
#define REG_ADDR__VID_BLANK1                                             (REGTX_SOC_P0 | 0x0068)
// (ReadWrite, Bits 7:0)
// This byte defines the blank data for channel G/Y. Default is 0.
#define BIT_MSK__VID_BLANK1__REG_VIDEO_MUTE_DATA_B1                                0xFF

// Video Blank Data High Byte Register
#define REG_ADDR__VID_BLANK2                                             (REGTX_SOC_P0 | 0x0069)
// (ReadWrite, Bits 7:0)
// This byte defines the blank data for channel R/Cr. Default is 0.
#define BIT_MSK__VID_BLANK2__REG_VIDEO_MUTE_DATA_B2                                0xFF

// Video Override Register
#define REG_ADDR__VID_OVRRD                                              (REGTX_SOC_P0 | 0x006A)
// (ReadWrite, Bits 5)
// Enable the insertion of a mini Vsync when reg_3dconv_en is set
#define BIT_MSK__VID_OVRRD__REG_MINIVSYNC_ON                                      0x20
// (ReadWrite, Bits 4)
// Enable MHL 3D fram packing conversion
#define BIT_MSK__VID_OVRRD__REG_3DCONV_EN                                         0x10

// Video Sync Polarity Detection Register
#define REG_ADDR__POL_DETECT                                             (REGTX_SOC_P0 | 0x006F)
// (ReadOnly, Bits 2)
// A high indicates the detection of interlace video: 1 - interlace video; 0 - noninterlaced.
#define BIT_MSK__POL_DETECT__INTERLACEDOUT                                         0x04
// (ReadOnly, Bits 1)
// Internal circuit detected Vsync polarity: 0 - active high 1 - active low
#define BIT_MSK__POL_DETECT__VSYNCPOLOUT                                           0x02
// (ReadOnly, Bits 0)
// Internal circuit detected Hsync polarity: 0 - active high 1 - active low
#define BIT_MSK__POL_DETECT__HSYNCPOLOUT                                           0x01

// CEA-861 VSI InfoFrame MHL IEEE No #0 Register
#define REG_ADDR__VSI_MHL_IEEE_NO_0                                      (REGTX_SOC_P0 | 0x0072)
// (ReadWrite, Bits 7:0)
// Read CEA-861 for detailed description of this register OUI=0x7CA61D
#define BIT_MSK__VSI_MHL_IEEE_NO_0__REG_MHL_IEEE_NO_B7_B0                                 0xFF

// CEA-861 VSI InfoFrame MHL IEEE No #1 Register
#define REG_ADDR__VSI_MHL_IEEE_NO_1                                      (REGTX_SOC_P0 | 0x0073)
// (ReadWrite, Bits 7:0)
// Read CEA-861 for detailed description of this register OUI=0x7CA61D
#define BIT_MSK__VSI_MHL_IEEE_NO_1__REG_MHL_IEEE_NO_B15_B8                                0xFF

// CEA-861 VSI InfoFrame MHL IEEE No #2 Register
#define REG_ADDR__VSI_MHL_IEEE_NO_2                                      (REGTX_SOC_P0 | 0x0074)
// (ReadWrite, Bits 7:0)
// Read CEA-861 for detailed description of this register OUI=0x7CA61D
#define BIT_MSK__VSI_MHL_IEEE_NO_2__REG_MHL_IEEE_NO_B23_B16                               0xFF

// Packet Filter0 Register
#define REG_ADDR__PKT_FILTER_0                                           (REGTX_SOC_P0 | 0x0075)
// (ReadWrite, Bits 7)
// Block Gamut Metadata packet from pass-through 0 - Pass-through Gamut Metadata packet from active input pipe 1 - Block Gamut Metadata packet from active input pipe
#define BIT_MSK__PKT_FILTER_0__REG_DROP_CEA_GAMUT_PKT                                0x80
// (ReadWrite, Bits 6)
// Block Audio Content Protection packet (0x04) from pass-through 0 - Pass-through Audio Content Protection packet from active input pipe 1 - Block Audio Content Protection packet from active input pipe
#define BIT_MSK__PKT_FILTER_0__REG_DROP_CEA_CP_PKT                                   0x40
// (ReadWrite, Bits 5)
// Block MPEG InfoFrame from pass-through 0 - Pass-through MPEG InfoFrame from active input pipe 1 - Block MPEG InfoFrame from active input pipe
#define BIT_MSK__PKT_FILTER_0__REG_DROP_MPEG_PKT                                     0x20
// (ReadWrite, Bits 4)
// Block SPD InfoFrame from pass-through 0 - Pass-through SPD InfoFrame from active input pipe 1 - Block SPD InfoFrame from active input pipe
#define BIT_MSK__PKT_FILTER_0__REG_DROP_SPIF_PKT                                     0x10
// (ReadWrite, Bits 3)
// Block Audio InfoFrame from pass-through 0 - Pass-through Audio InfoFrame from active input pipe 1 - Block Audio InfoFrame from active input pipe
#define BIT_MSK__PKT_FILTER_0__REG_DROP_AIF_PKT                                      0x08
// (ReadWrite, Bits 2)
// Block AVI InfoFrame from pass-through 0 - Pass-through AVI InfoFrame from active input pipe  1 - Block AVI InfoFrame from active input pipe
#define BIT_MSK__PKT_FILTER_0__REG_DROP_AVI_PKT                                      0x04
// (ReadWrite, Bits 1)
// Block ACR packet from pass-through 0 - Pass-through ACR packet from active input pipe 1 - Block ACR packet from active input pipe
#define BIT_MSK__PKT_FILTER_0__REG_DROP_CTS_PKT                                      0x02
// (ReadWrite, Bits 0)
// GCP Packet pass through disable. 0 - GCP packet received from the active input port is passed through to the output port 1 - disable the pass through
#define BIT_MSK__PKT_FILTER_0__REG_DROP_GCP_PKT                                      0x01

// Packet Filter1 Register
#define REG_ADDR__PKT_FILTER_1                                           (REGTX_SOC_P0 | 0x0076)
// (ReadWrite, Bits 7)
// Disable VSIF override
#define BIT_MSK__PKT_FILTER_1__REG_VSI_OVERRIDE_DIS                                  0x80
// (ReadWrite, Bits 6)
// Disable AVI override
#define BIT_MSK__PKT_FILTER_1__REG_AVI_OVERRIDE_DIS                                  0x40
// (ReadWrite, Bits 4)
// Block Null Packet from pass-through  0 - Pass-through Null Packet from active input pipe  1 - Block Null Packet from active input pipe
#define BIT_MSK__PKT_FILTER_1__REG_DROP_NULL_PKT                                     0x10
// (ReadWrite, Bits 3)
// Audio Packet pass through enable 0 - Disable the pass through for the Audio packets 1 - Audio packets received from the active input pipe are passed through to the output port
#define BIT_MSK__PKT_FILTER_1__REG_DROP_AUDIO_PKT                                    0x08
// (ReadWrite, Bits 2)
// Block Generic (2) InfoFrame from pass-through 0 - Pass-through Generic (2) InfoFrame from active input pipe  1 - Block Generic (2) InfoFrame from active input pipe  The Generic InfoFrame type is decided by the reg_drop_gen2_type.
#define BIT_MSK__PKT_FILTER_1__REG_DROP_GEN2_PKT                                     0x04
// (ReadWrite, Bits 1)
// Block Generic InfoFrame from pass-through 0 - Pass-through Generic InfoFrame from active input pipe 1 - Block Generic InfoFrame from active input pipe The Generic InfoFrame type is decided by the reg_drop_gen_type.
#define BIT_MSK__PKT_FILTER_1__REG_DROP_GEN_PKT                                      0x02
// (ReadWrite, Bits 0)
// Block Vendor-Specific InfoFrame from pass-through 0 - Pass-through Vendor-Specific InfoFrame from active input pipe  1 - Block Vendor-Specific InfoFrame from active input pipe
#define BIT_MSK__PKT_FILTER_1__REG_DROP_VSIF_PKT                                     0x01

// DROP GEN PACKET TYPE 0 Register
#define REG_ADDR__DROP_GEN_TYPE_0                                        (REGTX_SOC_P0 | 0x0077)
// (ReadWrite, Bits 7:0)
// Block packets matched with this packet header type
#define BIT_MSK__DROP_GEN_TYPE_0__REG_DROP_GEN_TYPE                                     0xFF

// DROP GEN PACKET TYPE 1 Register
#define REG_ADDR__DROP_GEN_TYPE_1                                        (REGTX_SOC_P0 | 0x0078)
// (ReadWrite, Bits 7:0)
// Block packets matched with this packet header type
#define BIT_MSK__DROP_GEN_TYPE_1__REG_DROP_GEN2_TYPE                                    0xFF

// DI PASS THROUGH CONTROL Register
#define REG_ADDR__DIPT_CNTL                                              (REGTX_SOC_P0 | 0x0079)
// (ReadWrite, Bits 5)
// 1 - the deep color packing mode information is masked 0 - the ddep color packing mode information is not masked This bit is used in bypassing deep color video without unpacking and packing it
#define BIT_MSK__DIPT_CNTL__REG_DC_PACK_MODE_MASK                                 0x20
// (ReadWrite, Bits 4)
// 1 - the priority of pass through packets is high 0 - the priority of pass through packets is low
#define BIT_MSK__DIPT_CNTL__REG_PB_PRIORITY_CTL                                   0x10
// (ReadWrite, Bits 3)
// 1 - the contens of deep color control information in General Control Packet are overrided 0 -  the contens of deep color control information in General Control Packet are not overrided
#define BIT_MSK__DIPT_CNTL__REG_PB_OVR_DC_PKT_EN                                  0x08
// (ReadWrite, Bits 2)
// 1 - split audio packets and other static packets 0 - do not split audio packets and othre static packets
#define BIT_MSK__DIPT_CNTL__REG_AUD_SPLIT_EN                                      0x04
// (ReadWrite, Bits 1)
// 1 - enable audio packets pass through function 0 - disable audio packets pass through function
#define BIT_MSK__DIPT_CNTL__REG_AUD_BYP_MODE                                      0x02
// (ReadWrite, Bits 0)
// 1 - enable packets pass through function 0 - disable packets pass through function
#define BIT_MSK__DIPT_CNTL__REG_PKT_BYP_MODE                                      0x01

// system counter_0 Register
#define REG_ADDR__SYS_CNTR_0                                             (REGTX_SOC_P0 | 0x007A)
// (ReadWrite, Bits 7:0)
// System counter low with granularity 0x1000 in 2MHz clock domain. This value is to be loaded in the system counter.
#define BIT_MSK__SYS_CNTR_0__REG_CNTR_VALUE_B7_B0                                  0xFF

// system counter_1 Register
#define REG_ADDR__SYS_CNTR_1                                             (REGTX_SOC_P0 | 0x007B)
// (ReadWrite, Bits 7:0)
// System counter high
#define BIT_MSK__SYS_CNTR_1__REG_CNTR_VALUE_B15_B8                                 0xFF

// system counter_0 Status Register
#define REG_ADDR__SYS_CNTR_ST0                                           (REGTX_SOC_P0 | 0x007C)
// (ReadOnly, Bits 7:0)
// System counter low with granularity 0x1000 in 2MHz clock domain. This is the current value of the system counter.
#define BIT_MSK__SYS_CNTR_ST0__SYS_CNTR_B7_B0                                        0xFF

// system counter_1 Status Register
#define REG_ADDR__SYS_CNTR_ST1                                           (REGTX_SOC_P0 | 0x007D)
// (ReadOnly, Bits 7:0)
// System counter high
#define BIT_MSK__SYS_CNTR_ST1__SYS_CNTR_B15_B8                                       0xFF

// 1st Layer Interrupt Status #1 Register
#define REG_ADDR__L1_INTR_STAT_0                                         (REGTX_SOC_P0 | 0x007E)
// (ReadOnly, Bits 7)
// HDCP2X IP interrupt
#define BIT_MSK__L1_INTR_STAT_0__L1_INTR_STAT_B7                                       0x80
// (ReadOnly, Bits 6)
// Aggregated SCDC INTR interrupts with masks
#define BIT_MSK__L1_INTR_STAT_0__L1_INTR_STAT_B6                                       0x40
// (ReadOnly, Bits 5)
// MHL_TX IP interrupt
#define BIT_MSK__L1_INTR_STAT_0__L1_INTR_STAT_B5                                       0x20
// (ReadOnly, Bits 4)
// TPI IP interrupt
#define BIT_MSK__L1_INTR_STAT_0__L1_INTR_STAT_B4                                       0x10
// (ReadOnly, Bits 3)
// Aggregated INTR5 interrupts with masks
#define BIT_MSK__L1_INTR_STAT_0__L1_INTR_STAT_B3                                       0x08
// (ReadOnly, Bits 2)
// Aggregated INTR3 interrupts with masks
#define BIT_MSK__L1_INTR_STAT_0__L1_INTR_STAT_B2                                       0x04
// (ReadOnly, Bits 1)
// Aggregated INTR2 interrupts with masks
#define BIT_MSK__L1_INTR_STAT_0__L1_INTR_STAT_B1                                       0x02
// (ReadOnly, Bits 0)
// Aggregated INTR1 interrupts with masks
#define BIT_MSK__L1_INTR_STAT_0__L1_INTR_STAT_B0                                       0x01

// 1st Layer Interrupt Status #2 Register
#define REG_ADDR__L1_INTR_STAT_1                                         (REGTX_SOC_P0 | 0x007F)
// (ReadOnly, Bits 3)
// Downstream EDID interrupt
#define BIT_MSK__L1_INTR_STAT_1__L1_INTR_STAT_B11                                      0x08
// (ReadOnly, Bits 2)
// Video_Path_Core interrupt
#define BIT_MSK__L1_INTR_STAT_1__L1_INTR_STAT_B10                                      0x04
// (ReadOnly, Bits 1)
// CEC IP interrupt
#define BIT_MSK__L1_INTR_STAT_1__L1_INTR_STAT_B9                                       0x02
// (ReadOnly, Bits 0)
// AIP IP interrupt
#define BIT_MSK__L1_INTR_STAT_1__L1_INTR_STAT_B8                                       0x01

// Hot Plug Connection Debouncing Timer 2 Register
#define REG_ADDR__HTPLG_T2                                               (REGTX_SOC_P0 | 0x008A)
// (ReadWrite, Bits 7:0)
// Hot Plug Detect debouncing timer counter 2nd stage
#define BIT_MSK__HTPLG_T2__REG_HPD_T2                                            0xFF

// Hot Plug Connection Debouncing Timer 1 Register
#define REG_ADDR__HTPLG_T1                                               (REGTX_SOC_P0 | 0x008B)
// (ReadWrite, Bits 7:0)
// Hot Plug Detect debouncing timer counter 1st stage
#define BIT_MSK__HTPLG_T1__REG_HPD_T1                                            0xFF

// Interrupt State Register
#define REG_ADDR__INTR_STATE                                             (REGTX_SOC_P0 | 0x008E)
// (ReadWrite, Bits 7)
// Set software interrupt INTR2[2] when 1'b1 is written (self resetting)
#define BIT_MSK__INTR_STATE__REG_SOFTWARE                                          0x80
// (ReadOnly, Bits 0)
// Interrupt state bit value.  It shows whether the interrupt is active or not.  It is one gate before the polarity is applied to the interrupt.  In other words; whenever the interrupt is asserted; this bit is high.
#define BIT_MSK__INTR_STATE__INTR_STATE                                            0x01

// Interrupt Source #1 Register
#define REG_ADDR__INTR1                                                  (REGTX_SOC_P0 | 0x008F)
// (ReadWrite, Bits 7)
// 5000ms(HDCP); 2000ms(CBUS) wait time counter. Asserted if set to 1. Write '1' to clear this bit
#define BIT_MSK__INTR1__REG_INTR1_STAT7                                       0x80
// (ReadWrite, Bits 6)
// Monitor Detect Interrupt; '1' if detection signal (HPD) has changed logic level. Asserted if set to 1. Write '1' to clear this bit.
#define BIT_MSK__INTR1__REG_INTR1_STAT6                                       0x40
// (ReadWrite, Bits 5)
// Monitor Detect Interrupt. Not applicable for Cypress HDMI Mode. During normal operation: '1' if detection signal (RSEN) has changed logic level.  During power down: '1' if detection signal (RSEN) changes.  Asserted if set to 1. Write 1 to clear.
#define BIT_MSK__INTR1__REG_INTR1_STAT5                                       0x20
// (ReadWrite, Bits 4)
// P0 PRIF ready stuck low due for too long and is forced back to high.
#define BIT_MSK__INTR1__REG_INTR1_STAT4                                       0x10
// (ReadWrite, Bits 3)
// MCU timeout for request. Write '1' to clear this bit.
#define BIT_MSK__INTR1__REG_INTR1_STAT3                                       0x08
// (ReadWrite, Bits 2)
// Counts frames after encryption is turned on and rolls over after 128 frames. Asserted if set to 1. Write '1' to clear this bit.
#define BIT_MSK__INTR1__REG_INTR1_STAT2                                       0x04
// (ReadWrite, Bits 1)
// KSVFIFO pending stuck for too long and PRIF ready is forced back to high.
#define BIT_MSK__INTR1__REG_INTR1_STAT1                                       0x02

// Interrupt Source #2 Register
#define REG_ADDR__INTR2                                                  (REGTX_SOC_P0 | 0x0090)
// (ReadWrite, Bits 6)
// Hash Done interrupt. Write 1 to clear.
#define BIT_MSK__INTR2__REG_INTR2_STAT6                                       0x40
// (ReadWrite, Bits 5)
// ENC_EN changed from 1 to 0. Write 1 to clear
#define BIT_MSK__INTR2__REG_INTR2_STAT5                                       0x20
// (ReadWrite, Bits 2)
// The interrupt will be asserted when 1'b1 is written into INT_CTRL[7]
#define BIT_MSK__INTR2__REG_INTR2_STAT2                                       0x04
// (ReadWrite, Bits 1)
// TCLK_STABLE has changed state Interrupt.  '1' if internal signal indicating TCLK_STABLE has changed logic level; indicating a change in the stability of the clock going to TMDS (from stable to non-stable or vice-versa).  Read the SYS_STAT register bit 1 to see the live state of TCLK_STABLE. Asserted if set to 1. Write 1 to clear.
#define BIT_MSK__INTR2__REG_INTR2_STAT1                                       0x02
// (ReadWrite, Bits 0)
// VSync active edge is recognized.  Asserted if set to 1. Write 1 to clear
#define BIT_MSK__INTR2__REG_INTR2_STAT0                                       0x01

// Interrupt Source #3 Register
#define REG_ADDR__INTR3                                                  (REGTX_SOC_P0 | 0x0091)
// (ReadWrite, Bits 3)
// DDC command is done interrupt. Asserted if set to 1. Write 1 to clear
#define BIT_MSK__INTR3__REG_INTR3_STAT3                                       0x08
// (ReadWrite, Bits 2)
// DDC FIFO is half-full interrupt. Asserted if set to 1. Write 1 to clear
#define BIT_MSK__INTR3__REG_INTR3_STAT2                                       0x04
// (ReadWrite, Bits 1)
// DDC FIFO is full interrupt. Asserted if set to 1. Write 1 to clear
#define BIT_MSK__INTR3__REG_INTR3_STAT1                                       0x02
// (ReadWrite, Bits 0)
// DDC FIFO is empty. Asserted if set to 1. Write 1 to clear
#define BIT_MSK__INTR3__REG_INTR3_STAT0                                       0x01

// Interrupt Source #5 Register
#define REG_ADDR__INTR5                                                  (REGTX_SOC_P0 | 0x0092)
// (ReadWrite, Bits 7)
// Packet Queue Overflow
#define BIT_MSK__INTR5__REG_INTR5_STAT7                                       0x80
// (ReadWrite, Bits 6)
// MHL3 CTS FIFO overflow and underflow. Not applicable for Cypress.
#define BIT_MSK__INTR5__REG_INTR5_STAT6                                       0x40
// (ReadWrite, Bits 3)
// MHL2/3 timing closure FIFO Overflow/Underflow. Asserted if set to 1. Write 1 to clear. Not applicable for Cypress HDMI Mode.
#define BIT_MSK__INTR5__REG_INTR5_STAT3                                       0x08
// (ReadWrite, Bits 2)
// MHL2 3-] FIFO Overflow/Undeflow. Asserted if set to 1. Write 1 to clear. Not applicable for Cypress HDMI Mode.
#define BIT_MSK__INTR5__REG_INTR5_STAT2                                       0x04

// Interrupt #1 Mask Register
#define REG_ADDR__INTR1_MASK                                             (REGTX_SOC_P0 | 0x0095)
// (ReadWrite, Bits 7)
// Enable INT1[7]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR1_MASK__REG_INTR1_MASK7                                       0x80
// (ReadWrite, Bits 6)
// Enable INT1[6]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR1_MASK__REG_INTR1_MASK6                                       0x40
// (ReadWrite, Bits 5)
// Enable INT1[5]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR1_MASK__REG_INTR1_MASK5                                       0x20
// (ReadWrite, Bits 4)
// Enable INT1[4]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR1_MASK__REG_INTR1_MASK4                                       0x10
// (ReadWrite, Bits 3)
// Enable INT1[3]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR1_MASK__REG_INTR1_MASK3                                       0x08
// (ReadWrite, Bits 2)
// Enable INT1[2]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR1_MASK__REG_INTR1_MASK2                                       0x04
// (ReadWrite, Bits 1)
// Enable INT1[1]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR1_MASK__REG_INTR1_MASK1                                       0x02

// Interrupt #2 Mask Register
#define REG_ADDR__INTR2_MASK                                             (REGTX_SOC_P0 | 0x0096)
// (ReadWrite, Bits 6)
// Enable INTR2[6]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR2_MASK__REG_INTR2_MASK6                                       0x40
// (ReadWrite, Bits 5)
// Enable INTR2[5]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR2_MASK__REG_INTR2_MASK5                                       0x20
// (ReadWrite, Bits 2)
// Enable INTR2[2]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR2_MASK__REG_INTR2_MASK2                                       0x04
// (ReadWrite, Bits 1)
// Enable INTR2[1]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR2_MASK__REG_INTR2_MASK1                                       0x02
// (ReadWrite, Bits 0)
// Enable INTR2[0]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR2_MASK__REG_INTR2_MASK0                                       0x01

// Interrupt #3 Mask Register
#define REG_ADDR__INTR3_MASK                                             (REGTX_SOC_P0 | 0x0097)
// (ReadWrite, Bits 3)
// Enable INTR3[3]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR3_MASK__REG_INTR3_MASK3                                       0x08
// (ReadWrite, Bits 2)
// Enable INTR3[2]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR3_MASK__REG_INTR3_MASK2                                       0x04
// (ReadWrite, Bits 1)
// Enable INTR3[1]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR3_MASK__REG_INTR3_MASK1                                       0x02
// (ReadWrite, Bits 0)
// Enable INTR3[0]: 1 - enable; 0 - disable (default)
#define BIT_MSK__INTR3_MASK__REG_INTR3_MASK0                                       0x01

// Interrupt #5 Mask Register
#define REG_ADDR__INTR5_MASK                                             (REGTX_SOC_P0 | 0x0098)
// (ReadWrite, Bits 7)
// Enable INT5[4]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__INTR5_MASK__REG_INTR5_MASK7                                       0x80
// (ReadWrite, Bits 6)
// Enable INT5[6]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__INTR5_MASK__REG_INTR5_MASK6                                       0x40
// (ReadWrite, Bits 5)
// Enable INT5[5]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__INTR5_MASK__REG_INTR5_MASK5                                       0x20
// (ReadWrite, Bits 3)
// Enable INT5[3]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__INTR5_MASK__REG_INTR5_MASK3                                       0x08
// (ReadWrite, Bits 2)
// Enable INT5[2]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__INTR5_MASK__REG_INTR5_MASK2                                       0x04

// Hot Plug Connection Control Register
#define REG_ADDR__HPD_CTRL                                               (REGTX_SOC_P0 | 0x009B)
// (ReadWrite, Bits 7)
// This value is ORed with Downstream HPD internally.
#define BIT_MSK__HPD_CTRL__REG_HPD_DS_SIGNAL                                     0x80
// (ReadWrite, Bits 6)
// Open Drain 0: disable 1: enable (default)
#define BIT_MSK__HPD_CTRL__REG_HPD_OUT_OD_EN                                     0x40
// (ReadWrite, Bits 5)
// 0 : HPD output = LOW (if reg_hpd_out_ovr_en=1); 1: HPD output = HIGH (if reg_hpd_out_ovr_en=1)
#define BIT_MSK__HPD_CTRL__REG_HPD_OUT_OVR_VAL                                   0x20
// (ReadWrite, Bits 4)
// 0 : HPD output = SET_HPD status; 1: HPD output = reg_hpd_out_ovr_val
#define BIT_MSK__HPD_CTRL__REG_HPD_OUT_OVR_EN                                    0x10

// TMDS Clock Status Register
#define REG_ADDR__TMDS_CSTAT                                             (REGTX_SOC_P0 | 0x009F)
// (ReadOnly, Bits 2)
// HDMI RPWR5V status 0 - 5V gone and cable disconnected 1 - Cable connected
#define BIT_MSK__TMDS_CSTAT__RPWR5V                                                0x04

// TMDS Control #4 Register
#define REG_ADDR__TMDS_CTRL4                                             (REGTX_SOC_P0 | 0x00A3)
// (ReadWrite, Bits 0)
// Enable SCDT to control tx_en for MHL TMDS TX 0: Disable ( to default by #23345) 1: Enable (default to non-default by #23345)
#define BIT_MSK__TMDS_CTRL4__REG_TX_EN_BY_SCDT                                     0x01

// SCDT Holdoff MSB Register
#define REG_ADDR__SCDT_HOLDOFF_MSB                                       (REGTX_SOC_P0 | 0x00A4)
// (ReadWrite, Bits 7:0)
// MSB of 24 bit holdoff counter threshold that deglitches SCDT high active status.
#define BIT_MSK__SCDT_HOLDOFF_MSB__REG_SCDT_HOLDOFF_MSB_B7_B0                            0xFF

// TMDS Control #7 Register
#define REG_ADDR__TMDS_CTRL7                                             (REGTX_SOC_P0 | 0x00A6)
// (ReadWrite, Bits 7)
// This bit enables/disables MHL TMDS sequence 0 - disable (default) 1 - enable
#define BIT_MSK__TMDS_CTRL7__REG_TMDS_SWAP_BIT                                     0x80

// HPD In Override Register
#define REG_ADDR__HPD_IN_CTRL                                            (REGTX_SOC_P0 | 0x00A7)
// (ReadWrite, Bits 1)
// HPD input override select 0:  HDMI (default) 1:  CBUS
#define BIT_MSK__HPD_IN_CTRL__REG_HPDIN_OVER_SEL                                    0x02
// (ReadWrite, Bits 0)
// HPD input override enable 0:  Auto detect (default) 1:  Software set
#define BIT_MSK__HPD_IN_CTRL__REG_HPDIN_OVER_EN                                     0x01

// LM DDC Register
#define REG_ADDR__LM_DDC                                                 (REGTX_SOC_P0 | 0x00CC)
// (ReadWrite, Bits 7)
// SW TPI enable
#define BIT_MSK__LM_DDC__REG_SW_TPI_EN                                         0x80
// (ReadWrite, Bits 5)
// Video mute enable
#define BIT_MSK__LM_DDC__REG_VIDEO_MUTE_EN                                     0x20
// (ReadWrite, Bits 2)
// Write 1'b1 to flip the switch from DDC to I2C so that I2C can access the downstream DDC slave through CBUS like DDC. The switch will be flipped back after STOP condition.
#define BIT_MSK__LM_DDC__REG_DDC_TPI_SW                                        0x04
// (ReadOnly, Bits 1)
// DDC Grant 1'b0: TX ownx DDC bus 1'b1: Host is granted DDC bus access
#define BIT_MSK__LM_DDC__REG_DDC_GRANT                                         0x02
// (ReadWrite, Bits 0)
// 0 = Not using DDC 1 = Request to use DDC
#define BIT_MSK__LM_DDC__REG_DDC_GPU_REQUEST                                   0x01

// TX SHA Control Register
#define REG_ADDR__TXSHA_CTRL                                             (REGTX_SOC_P0 | 0x00D1)
// (ReadWrite, Bits 1)
// If 1 means that SHA picked up the SHA go stat command.  Write 1 to clear; before setting new SHA go stat command.
#define BIT_MSK__TXSHA_CTRL__REG_SHACTRL_STAT1                                     0x02
// (ReadWrite, Bits 0)
// Firmware starts the SHA generation by writing 1; which generates 1 clock strobe.  Reading this bit  will not read what was written; but rather a status of SHA producing  a V value; such as:  If 0 is read then SHA is processing and V is not ready. If 1 is read then SHA is done processing and V is ready.  SHA processes very quickly and will often be finished by the time a read can be performed.  Note: before the above sequence this bit will return a 1 when read. In order to make for sure that SHA picked 1 written to this bit; firmware need to check bit #1 of the same register.
#define BIT_MSK__TXSHA_CTRL__REG_SHA_GO_STAT                                       0x01

// TX KSV FIFO Register
#define REG_ADDR__TXKSV_FIFO                                             (REGTX_SOC_P0 | 0x00D2)
// (ReadWrite, Bits 7:0)
// This Address  is a port for access to the KSV FIFO. When the firmware starts a  I2C transaction with the offset address set at 38h the access control will be transferred to the KSV FIFO.  The address located inside the KSV Start Address register acts as the start offset within the KSV FIFO space.   Consecutive I2C transactions to address 38h will be auto-incremented in the KSV FIFO Address space.
#define BIT_MSK__TXKSV_FIFO__REG_KSV_FIFO_OUT                                      0xFF

// HDCP Repeater Down Stream BSTATUS #1 Register
#define REG_ADDR__TXDS_BSTATUS1                                          (REGTX_SOC_P0 | 0x00D3)
// (ReadWrite, Bits 7)
// need description
#define BIT_MSK__TXDS_BSTATUS1__REG_DS_DEV_EXCEED                                     0x80
// (ReadWrite, Bits 6:0)
// need description
#define BIT_MSK__TXDS_BSTATUS1__REG_DS_DEV_CNT                                        0x7F

// HDCP Repeater Down Stream BSTATUS #2 Register
#define REG_ADDR__TXDS_BSTATUS2                                          (REGTX_SOC_P0 | 0x00D4)
// (ReadWrite, Bits 7:5)
// Bstatus bits
#define BIT_MSK__TXDS_BSTATUS2__REG_DS_BSTATUS                                        0xE0
// (ReadWrite, Bits 4)
// HDMI Mode. (controlled through the I2C local side) 1 - receiver is in HDMI Mode; 0 - receiver is in DVI mode.
#define BIT_MSK__TXDS_BSTATUS2__REG_DS_HDMI_MODE                                      0x10
// (ReadWrite, Bits 3)
// Max cascade exceeded - need description
#define BIT_MSK__TXDS_BSTATUS2__REG_DS_CASC_EXCEED                                    0x08
// (ReadWrite, Bits 2:0)
// Depth - need description
#define BIT_MSK__TXDS_BSTATUS2__REG_DS_DEPTH                                          0x07

// HDCP Repeater V.H0 #0 Register
#define REG_ADDR__TXVH0_0                                                (REGTX_SOC_P0 | 0x00D8)
// (ReadWrite, Bits 7:0)
// Vp.H0 bits [7:0] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH0_0__REG_VP_H0_W_B0                                        0xFF

// HDCP Repeater V.H0 #1 Register
#define REG_ADDR__TXVH0_1                                                (REGTX_SOC_P0 | 0x00D9)
// (ReadWrite, Bits 7:0)
// Vp.H0 bits [15:8] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config
#define BIT_MSK__TXVH0_1__REG_VP_H0_W_B1                                        0xFF

// HDCP Repeater V.H0 #2 Register
#define REG_ADDR__TXVH0_2                                                (REGTX_SOC_P0 | 0x00DA)
// (ReadWrite, Bits 7:0)
// Vp.H0 bits [23:16] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH0_2__REG_VP_H0_W_B2                                        0xFF

// HDCP Repeater V.H0 #3 Register
#define REG_ADDR__TXVH0_3                                                (REGTX_SOC_P0 | 0x00DB)
// (ReadWrite, Bits 7:0)
// Vp.H0 bits [31:24] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH0_3__REG_VP_H0_W_B3                                        0xFF

// HDCP Repeater V.H1 #0 Register
#define REG_ADDR__TXVH1_0                                                (REGTX_SOC_P0 | 0x00DC)
// (ReadWrite, Bits 7:0)
// Vp.H1 bits [7:0] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH1_0__REG_VP_H1_W_B0                                        0xFF

// HDCP Repeater V.H1 #1 Register
#define REG_ADDR__TXVH1_1                                                (REGTX_SOC_P0 | 0x00DD)
// (ReadWrite, Bits 7:0)
// Vp.H1 bits [15:8] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH1_1__REG_VP_H1_W_B1                                        0xFF

// HDCP Repeater V.H1 #2 Register
#define REG_ADDR__TXVH1_2                                                (REGTX_SOC_P0 | 0x00DE)
// (ReadWrite, Bits 7:0)
// Vp.H1 bits [23:16] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH1_2__REG_VP_H1_W_B2                                        0xFF

// HDCP Repeater V.H1 #3 Register
#define REG_ADDR__TXVH1_3                                                (REGTX_SOC_P0 | 0x00DF)
// (ReadWrite, Bits 7:0)
// Vp.H1 bits [31:24] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH1_3__REG_VP_H1_W_B3                                        0xFF

// HDCP Repeater V.H2 #0 Register
#define REG_ADDR__TXVH2_0                                                (REGTX_SOC_P0 | 0x00E0)
// (ReadWrite, Bits 7:0)
// Vp.H2 bits [7:0] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH2_0__REG_VP_H2_W_B0                                        0xFF

// HDCP Repeater V.H2 #1 Register
#define REG_ADDR__TXVH2_1                                                (REGTX_SOC_P0 | 0x00E1)
// (ReadWrite, Bits 7:0)
// Vp.H2 bits [15:8] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH2_1__REG_VP_H2_W_B1                                        0xFF

// HDCP Repeater V.H2 #2 Register
#define REG_ADDR__TXVH2_2                                                (REGTX_SOC_P0 | 0x00E2)
// (ReadWrite, Bits 7:0)
// Vp.H2 bits [23:16] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH2_2__REG_VP_H2_W_B2                                        0xFF

// HDCP Repeater V.H2 #3 Register
#define REG_ADDR__TXVH2_3                                                (REGTX_SOC_P0 | 0x00E3)
// (ReadWrite, Bits 7:0)
// Vp.H2 bits [31:24] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH2_3__REG_VP_H2_W_B3                                        0xFF

// HDCP Repeater V.H3 #0 Register
#define REG_ADDR__TXVH3_0                                                (REGTX_SOC_P0 | 0x00E4)
// (ReadWrite, Bits 7:0)
// Vp.H3 bits [7:0] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH3_0__REG_VP_H3_W_B0                                        0xFF

// HDCP Repeater V.H3 #1 Register
#define REG_ADDR__TXVH3_1                                                (REGTX_SOC_P0 | 0x00E5)
// (ReadWrite, Bits 7:0)
// Vp.H3 bits [15:8] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH3_1__REG_VP_H3_W_B1                                        0xFF

// HDCP Repeater V.H3 #2 Register
#define REG_ADDR__TXVH3_2                                                (REGTX_SOC_P0 | 0x00E6)
// (ReadWrite, Bits 7:0)
// Vp.H3 bits [23:16] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH3_2__REG_VP_H3_W_B2                                        0xFF

// HDCP Repeater V.H3 #3 Register
#define REG_ADDR__TXVH3_3                                                (REGTX_SOC_P0 | 0x00E7)
// (ReadWrite, Bits 7:0)
// Vp.H3 bits [31:24] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH3_3__REG_VP_H3_W_B3                                        0xFF

// HDCP Repeater V.H4 #0 Register
#define REG_ADDR__TXVH4_0                                                (REGTX_SOC_P0 | 0x00E8)
// (ReadWrite, Bits 7:0)
// Vp.H4 bits [7:0] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH4_0__REG_VP_H4_W_B0                                        0xFF

// HDCP Repeater V.H4 #1 Register
#define REG_ADDR__TXVH4_1                                                (REGTX_SOC_P0 | 0x00E9)
// (ReadWrite, Bits 7:0)
// Vp.H4 bits [15:8] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH4_1__REG_VP_H4_W_B1                                        0xFF

// HDCP Repeater V.H4 #2 Register
#define REG_ADDR__TXVH4_2                                                (REGTX_SOC_P0 | 0x00EA)
// (ReadWrite, Bits 7:0)
// Vp.H4 bits [23:16] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH4_2__REG_VP_H4_W_B2                                        0xFF

// HDCP Repeater V.H4 #3 Register
#define REG_ADDR__TXVH4_3                                                (REGTX_SOC_P0 | 0x00EB)
// (ReadWrite, Bits 7:0)
// Vp.H4 bits [31:24] with vp_sel = 1 (page 0 0xEC bit 6). When vp_sel = 0 it is config type
#define BIT_MSK__TXVH4_3__REG_VP_H4_W_B3                                        0xFF

// DDC I2C Manual Register
#define REG_ADDR__DDC_MANUAL                                             (REGTX_SOC_P0 | 0x00EC)
// (ReadWrite, Bits 7)
// manual override of scl and sda output
#define BIT_MSK__DDC_MANUAL__REG_MAN_DDC                                           0x80
// (ReadWrite, Bits 6)
// 1'b1: select V prime from TPI 1'b0: select V prime from downstream (read through DDC)
#define BIT_MSK__DDC_MANUAL__VP_SEL                                                0x40
// (ReadWrite, Bits 5)
// DSDA manual output value.  Only enabled onto the DSDA output if reg_man_ddc = 1.
#define BIT_MSK__DDC_MANUAL__REG_DSDA                                              0x20
// (ReadWrite, Bits 4)
// DSCL manual output value.  Only enabled onto the DSCL output if reg_man_ddc = 1.
#define BIT_MSK__DDC_MANUAL__REG_DSCL                                              0x10
// (ReadWrite, Bits 3)
// Set to allow HW to drop the incoming GCP packets and automatically send downstream GCP packets
#define BIT_MSK__DDC_MANUAL__REG_GCP_HW_CTL_EN                                     0x08
// (ReadWrite, Bits 2)
// Write 1 to abort DDC master. Self-clearing
#define BIT_MSK__DDC_MANUAL__REG_DDCM_ABORT                                        0x04
// (ReadOnly, Bits 1)
// DSDA input status.  This bit reflects the live status of the DSDA pin.
#define BIT_MSK__DDC_MANUAL__IO_DSDA                                               0x02
// (ReadOnly, Bits 0)
// DSCL input status.  This bit reflects the live status of the DSCL pin.
#define BIT_MSK__DDC_MANUAL__IO_DSCL                                               0x01

// DDC I2C Target Slave Address Register
#define REG_ADDR__DDC_ADDR                                               (REGTX_SOC_P0 | 0x00ED)
// (ReadWrite, Bits 7:1)
// DDC target Slave address
#define BIT_MSK__DDC_ADDR__REG_DDC_ADDR                                          0xFE

// DDC I2C Target Segment Address Register
#define REG_ADDR__DDC_SEGM                                               (REGTX_SOC_P0 | 0x00EE)
// (ReadWrite, Bits 7:0)
// DDC Target Segment address
#define BIT_MSK__DDC_SEGM__REG_DDC_SEGMENT                                       0xFF

// DDC I2C Target Offset Adress Register
#define REG_ADDR__DDC_OFFSET                                             (REGTX_SOC_P0 | 0x00EF)
// (ReadWrite, Bits 7:0)
// target slave register offset
#define BIT_MSK__DDC_OFFSET__REG_DDC_OFFSET                                        0xFF

// DDC I2C Data In count #1 Register
#define REG_ADDR__DDC_DIN_CNT1                                           (REGTX_SOC_P0 | 0x00F0)
// (ReadWrite, Bits 7:0)
// Bits [7:0] of the number of bytes to transfer in
#define BIT_MSK__DDC_DIN_CNT1__REG_DDC_DIN_CNT_B7_B0                                 0xFF

// DDC I2C Data In count #2 Register
#define REG_ADDR__DDC_DIN_CNT2                                           (REGTX_SOC_P0 | 0x00F1)
// (ReadWrite, Bits 1:0)
// Bits [9:8] of the number of bytes to transfer in
#define BIT_MSK__DDC_DIN_CNT2__REG_DDC_DIN_CNT_B9_B8                                 0x03

// DDC I2C Status Register
#define REG_ADDR__DDC_STATUS                                             (REGTX_SOC_P0 | 0x00F2)
// (ReadWrite, Bits 6)
// When read get the status from hardware: to clear write 0
#define BIT_MSK__DDC_STATUS__REG_DDC_BUS_LOW                                       0x40
// (ReadWrite, Bits 5)
// When read get the status from hardware: to clear write 0
#define BIT_MSK__DDC_STATUS__REG_DDC_NO_ACK                                        0x20
// (ReadOnly, Bits 4)
// DDC in programming mode
#define BIT_MSK__DDC_STATUS__DDC_I2C_IN_PROG                                       0x10
// (ReadOnly, Bits 3)
// DDC fifo full
#define BIT_MSK__DDC_STATUS__DDC_FIFO_FULL                                         0x08
// (ReadOnly, Bits 2)
// DDC FIFO is empty
#define BIT_MSK__DDC_STATUS__DDC_FIFO_EMPTY                                        0x04
// (ReadOnly, Bits 1)
// DDC fifo read in use
#define BIT_MSK__DDC_STATUS__DDC_FIFO_READ_IN_SUE                                  0x02
// (ReadOnly, Bits 0)
// DDC fifo write in use
#define BIT_MSK__DDC_STATUS__DDC_FIFO_WRITE_IN_USE                                 0x01

// DDC I2C Command Register
#define REG_ADDR__DDC_CMD                                                (REGTX_SOC_P0 | 0x00F3)
// (ReadWrite, Bits 6)
// Enable short Ri read for DDC
#define BIT_MSK__DDC_CMD__REG_HDCP_DDC_EN                                       0x40
// (ReadWrite, Bits 5)
// Enable the DDC del: 0 - is enable (default). DDC delay has been inserted into the SDA line to create 300ns delay for the falling edge of the DDC SDA signal in order to avoid erroneous I2C START condition.  The real start condition must have setup time of the 600ns and therefore this delay of 300ns will not remove the real START condition.  Filtering is done using Ring Oscillator.
#define BIT_MSK__DDC_CMD__REG_SDA_DEL_EN                                        0x20
// (ReadWrite, Bits 4)
// Enable 3ns glitch filtering on the DDC clock and data line. 0 - is enabled (default). Filtering is done using Ring Oscillator.
#define BIT_MSK__DDC_CMD__REG_DDC_FLT_EN                                        0x10
// (ReadWrite, Bits 3:0)
// DDC Command[3:0]: 1111 - Abort transaction 1001 - Clear FIFO 1010 - Clock SCL 0000 - Current address read with no ACK on last byte 0001 - Current address read with ACK on last byte 0010 - Sequential read with no ACK on last byte 0011 - Sequential read with ACK on last byte 0100 - Enhanced DDC read with no ACK on last byte 0101 - Enhanced DDC read with ACK on last byte 0110 - Sequential write ignoring ACK on last byte 0111 - Sequential write requiring ACK on last byte  write to this register generates 1 clock strobe; which will force decoding of the ddc command
#define BIT_MSK__DDC_CMD__REG_DDC_CMD                                           0x0F

// DDC I2C FIFO Data In/Out Register
#define REG_ADDR__DDC_DATA                                               (REGTX_SOC_P0 | 0x00F4)
// (ReadOnly, Bits 7:0)
// DDC FIFO register.  During write to FIFO write data into his register During the read from FIFO this register will have the FIFO data
#define BIT_MSK__DDC_DATA__DDC_DATA_OUT                                          0xFF

// DDC I2C Data Out Counter Register
#define REG_ADDR__DDC_DOUT_CNT                                           (REGTX_SOC_P0 | 0x00F5)
// (ReadWrite, Bits 7)
// DDC I2C delay counter register 9-bit of 9
#define BIT_MSK__DDC_DOUT_CNT__REG_DDC_DELAY_CNT_B1                                  0x80
// (ReadOnly, Bits 4:0)
// Data out counter register
#define BIT_MSK__DDC_DOUT_CNT__DDC_DATA_OUT_CNT                                      0x1F

// DDC I2C Delay Count Register
#define REG_ADDR__DDC_DELAY_CNT                                          (REGTX_SOC_P0 | 0x00F6)
// (ReadWrite, Bits 7:0)
// DDC I2C delay counter register 9-bit of 8
#define BIT_MSK__DDC_DELAY_CNT__REG_DDC_DELAY_CNT_B0                                  0xFF

// Test Control Register
#define REG_ADDR__TEST_TXCTRL                                            (REGTX_SOC_P0 | 0x00F7)
// (ReadWrite, Bits 1)
// Enable HDMI/MHL mode for output when  set 1. Output DVI mode when set 0.
#define BIT_MSK__TEST_TXCTRL__REG_HDMI_MODE                                         0x02
// (ReadWrite, Bits 0)
// Test TMDS PLL Clocks. 0 - Test TMDS PLL Clocks disabled (default) 1 - TMDS filter PLL and main PLL Clocks muxed to test output pins.
#define BIT_MSK__TEST_TXCTRL__REG_TST_PLLCK                                         0x01

// I2C Status Register
#define REG_ADDR__EPST                                                   (REGTX_SOC_P0 | 0x00F8)
// (ReadOnly, Bits 7)
// Status of the OTP programming: 1 - OTP has not been locked and can be programmed again.  The OTP is based on the Jet City fuse based memory and once the fuse has been blown it can be changed. That why every consequent write to the OTP memory will be the OR of the previous ones.
#define BIT_MSK__EPST__OTP_UNLOCKED                                          0x80
// (ReadWrite, Bits 6)
// self-authentication #2 error if set to 1; Write 0 to clear
#define BIT_MSK__EPST__REG_BIST2_ERR_CLR                                     0x40
// (ReadWrite, Bits 5)
// self-authentication #1 error if set to 1; Write 0 to clear
#define BIT_MSK__EPST__REG_BIST1_ERR_CLR                                     0x20
// (ReadWrite, Bits 1)
// CRC error if  set to 1; Write 0 to clear
#define BIT_MSK__EPST__REG_BIST_ERR_CLR                                      0x02
// (ReadWrite, Bits 0)
// Last command had been completed; For the status of this command please check error status bits in this register. Write 0 to clear.
#define BIT_MSK__EPST__REG_CMD_DONE_CLR                                      0x01

// I2C Command Register
#define REG_ADDR__EPCM                                                   (REGTX_SOC_P0 | 0x00F9)
// (ReadWrite, Bits 5)
// Enable Load of the KSV from OTP : 1 - enable.  Write 0 before enabling again.
#define BIT_MSK__EPCM__REG_LD_KSV                                            0x20
// (ReadWrite, Bits 4:0)
// I2C Master commands: Five bits are SA2_en : SA1_en : CRC_en : 2 bit Command 00000 - Clear command register; no action will be taken xxx01 - rsvd xxx10 - rsvd xxx11 - will enable all BIST: CRC; self-authentication #1 and self_authentication #2 xx100 - will enable individual CRC  x1x00 - will enable individual self-authentication #1  1xx00 - will enable individual self_authentication #2   New value can be safely written into this register only after getting command done bit (reg. 0x0F9[0]) or waiting at least 0.125s. That means that new value can be safely written into this register only after previous command has been completed. Otherwise; it is possible to get either self-authentication #1 error status bit or to get BIST done bit without actually performing one of the new enabled BIST test.
#define BIT_MSK__EPCM__REG_EPCM                                              0x1F

// I2C Command Register
#define REG_ADDR__OTP_CLK                                                (REGTX_SOC_P0 | 0x00FA)
// (ReadWrite, Bits 5)
// 1: enabled the writing of protect byte (xff) in otp rom.  Default value 0: protect byte will not be written to otp rom.
#define BIT_MSK__OTP_CLK__REG_OTP_PROTECT_EN                                    0x20

//***************************************************************************
// REGTX_AVLINK. Address: 40
// Tx Zone Ctl0 Register
#define REG_ADDR__TX_ZONE_CTL0                                           (REGTX_AVLINK | 0x0020)
// (ReadWrite, Bits 7:0)
// Used to decide the zone range. Fixed to 8.
#define BIT_MSK__TX_ZONE_CTL0__REG_MAX_DIFF_LIMIT                                    0xFF

// Tx Zone Ctl1 Register
#define REG_ADDR__TX_ZONE_CTL1                                           (REGTX_AVLINK | 0x0021)
// (ReadWrite, Bits 7:0)
// Zone control parameters (from i2c; default = 6'b00000)   [7:6]    : df_tap   [5]    : mult_comp   [4]    : sel_cecclk   [3:2]    : zonectl_mode   [1]    : strict   [0]    : longer   After transition state; it should be programmed to 4'b10** thru   i2c for normal PLL operation
#define BIT_MSK__TX_ZONE_CTL1__REG_TX_ZONE_CTRL                                      0xFF

// Tx Zone Ctl2 Register
#define REG_ADDR__TX_ZONE_CTL2                                           (REGTX_AVLINK | 0x0022)
// (ReadOnly, Bits 7:6)
// MHL2 Tx zone status 00: 1x Zone 01: 2x Zone 10: 4x Zone 11: 8x Zone
#define BIT_MSK__TX_ZONE_CTL2__REG_TX_ZONE_SEL                                       0xC0
// (ReadWrite, Bits 1:0)
// MHL2 Tx zone select from I2C 00: 1x Zone 01: 2x Zone 10: 4x Zone 11: 8x Zone
#define BIT_MSK__TX_ZONE_CTL2__REG_TX_ZONE                                           0x03

// Tx Zone Ctl3 Register
#define REG_ADDR__TX_ZONE_CTL3                                           (REGTX_AVLINK | 0x0023)
// (ReadWrite, Bits 7:6)
// [7]    : run reference clock counter   : reg_tx_zone_ctrl3[1] [6]    : clear reference clock counter : reg_tx_zone_ctrl3[0]
#define BIT_MSK__TX_ZONE_CTL3__REG_TX_ZONE_CTRL3                                     0xC0
// (ReadOnly, Bits 5:0)
// [5]    : overflow flag [4:0]    : counter
#define BIT_MSK__TX_ZONE_CTL3__HRV_ZONE_CTRL3                                        0x3F

// TX Zone Ctl 4 Register
#define REG_ADDR__TX_ZONEL_CTL4                                          (REGTX_AVLINK | 0x0024)
// (ReadWrite, Bits 3:2)
// Select frequency ratio betweeen hdmi_clk and pxl_clk 2'b00 0.5x mode. hdmi_clk_freq = 0.5*pxl_clk_freq 2'b01 1x mode. hdmi_clk_freq = 1*pxl_clk_freq (default) 2'b10 2x mode. hdmi_clk_freq = 2*pxl_clk_freq 2'b11 4x mode. hdmi_clk_freq = 4*pxl_clk_freq
#define BIT_MSK__TX_ZONEL_CTL4__REG_HDMI_CLK_RATIO                                    0x0C

// TX Zone Ctl 5 Register
#define REG_ADDR__TX_ZONE_CTL5                                           (REGTX_AVLINK | 0x0025)
// (ReadWrite, Bits 7)
// Enable clock detect
#define BIT_MSK__TX_ZONE_CTL5__REG_CLKDETECT_EN                                      0x80
// (ReadWrite, Bits 3)
// measure enable signal for zone control. when enable;  pll is in open loop
#define BIT_MSK__TX_ZONE_CTL5__REG_MEAS_FVCO                                         0x08

// MHL3 Tx Zone Ctl Register
#define REG_ADDR__MHL3_TX_ZONE_CTL                                       (REGTX_AVLINK | 0x0026)
// (ReadWrite, Bits 7)
// When set to 1'b1; zone controls for Tx PHY and interpolator are separated. The interpolator zone is controlled by reg_mhl3_tx_zone.
#define BIT_MSK__MHL3_TX_ZONE_CTL__REG_MHL2_INTPLT_ZONE_MANU_EN                          0x80
// (ReadWrite, Bits 1:0)
// MHL3 Tx zone control. This controls MHL3 sub-multiple mode 00: 6G mode 01: 3G mode 10: 1.5G mode
#define BIT_MSK__MHL3_TX_ZONE_CTL__REG_MHL3_TX_ZONE                                      0x03

// Zone VCO Control Register
#define REG_ADDR__TX_ZONEVCO_CTL                                         (REGTX_AVLINK | 0x0027)
// (ReadWrite, Bits 0)
// 1 to start auto zone calculation. To start next calculation, it should be cleared first and asserted.
#define BIT_MSK__TX_ZONEVCO_CTL__REG_ZONEVCO_START                                     0x01

// MHL3 CTS Control Register
#define REG_ADDR__MHL3CTS_CTL                                            (REGTX_AVLINK | 0x0028)
// (ReadWrite, Bits 7)
// Enable MHL3 CTS mode
#define BIT_MSK__MHL3CTS_CTL__REG_MHL3CTS_EN                                        0x80
// (ReadWrite, Bits 6)
// Enable MHL3 CTS Snoop mode
#define BIT_MSK__MHL3CTS_CTL__REG_COC_DOC_SNOOP_EN                                  0x40
// (ReadWrite, Bits 4)
// Select MHL3 CTS operation mode 1 - 1 lanes (default) 0 - up to reg_mhl3cts_3lane (0xB4[1])
#define BIT_MSK__MHL3CTS_CTL__REG_MHL3CTS_1LANE                                     0x10
// (ReadWrite, Bits 3:2)
// Select the output clock for MHL3 CTS 00 - 1/2 of link clock when reg_mhl3cts_3lane = 1'b0 or 1/3 of link clock when reg_mhl3cts_3lane = 1'b1 01 - 1/4 of link clock when reg_mhl3cts_3lane = 1'b0 or 1/6 of link clock when reg_mhl3cts_3lane = 1'b1 10 - 1/8 of link clock when reg_mhl3cts_3lane = 1'b0 or 1/12 of link clock when reg_mhl3cts_3lane = 1'b1 (default) 11 - 1/16 of link clock when reg_mhl3cts_3lane = 1'b0 or 1/24 of link clock when reg_mhl3cts_3lane = 1'b1
#define BIT_MSK__MHL3CTS_CTL__REG_MHL3CTS_CLKOUT_SEL                                0x0C
// (ReadWrite, Bits 1)
// Select MHL3 CTS operation mode 1 - 3 lanes (default) 0 - 2 lanes
#define BIT_MSK__MHL3CTS_CTL__REG_MHL3CTS_3LANE                                     0x02

// TX X BIST CNTL Register
#define REG_ADDR__TX_XBIST_CNTL                                          (REGTX_AVLINK | 0x0029)
// (ReadWrite, Bits 6)
// Backward compatibility for previous version.
#define BIT_MSK__TX_XBIST_CNTL__REG_LEGACY                                            0x40
// (ReadWrite, Bits 5)
// Disable create signature if = 1'b1
#define BIT_MSK__TX_XBIST_CNTL__REG_DISABLE_ENC                                       0x20
// (ReadWrite, Bits 3)
// Make AV link data speed 4x slower. 1 = enable
#define BIT_MSK__TX_XBIST_CNTL__REG_QUARTER_CLK_SEL                                   0x08
// (ReadWrite, Bits 2)
// 1'b1 - divide clock by 2
#define BIT_MSK__TX_XBIST_CNTL__REG_HALF_CLK_SEL                                      0x04
// (ReadWrite, Bits 1)
// 1'b0  txbist is disabled.    (default value)1'b1 -  txbist is enabled.
#define BIT_MSK__TX_XBIST_CNTL__REG_BIST_EN                                           0x02
// (ReadWrite, Bits 0)
// select bist data path to serializer
#define BIT_MSK__TX_XBIST_CNTL__REG_BIST_SEL                                          0x01

// TX X BIST INST LOW Register
#define REG_ADDR__TX_XBIST_INST_LOW                                      (REGTX_AVLINK | 0x002A)
// (ReadWrite, Bits 7:0)
// 11-bit Instruction register for Red Green and Blue channels. LSB 8 bits here and 2 bits to follow
#define BIT_MSK__TX_XBIST_INST_LOW__REG_INSTRUCTION_B7_B0                                 0xFF

// TX X BIST INST HIGH Register
#define REG_ADDR__TX_XBIST_INST_HIGH                                     (REGTX_AVLINK | 0x002B)
// (ReadWrite, Bits 2:0)
// 11-bit Instruction register for Red Green and Blue channels. LSB 8 bits here and 2 bits to follow
#define BIT_MSK__TX_XBIST_INST_HIGH__REG_INSTRUCTION_B10_B8                                0x07

// TX X BIST PATTERN LOW Register
#define REG_ADDR__TX_XBIST_PAT_LOW                                       (REGTX_AVLINK | 0x002C)
// (ReadWrite, Bits 7:0)
// pattern control register for Red Green and Blue channels.
#define BIT_MSK__TX_XBIST_PAT_LOW__REG_PATTERN_B7_B0                                     0xFF

// TX X BIST PATTERN HIGH Register
#define REG_ADDR__TX_XBIST_PAT_HIGH                                      (REGTX_AVLINK | 0x002D)
// (ReadWrite, Bits 1:0)
// pattern control register for Red Green and Blue channels.
#define BIT_MSK__TX_XBIST_PAT_HIGH__REG_PATTERN_B9_B8                                     0x03

// TX X BIST CONFIGURE LOW Register
#define REG_ADDR__TX_XBIST_CONF_LOW                                      (REGTX_AVLINK | 0x002E)
// (ReadWrite, Bits 7:0)
// TBA
#define BIT_MSK__TX_XBIST_CONF_LOW__REG_CONFIGURE_B7_B0                                   0xFF

// TX X BIST CONFIGURE HIGH Register
#define REG_ADDR__TX_XBIST_CONF_HIGH                                     (REGTX_AVLINK | 0x002F)
// (ReadWrite, Bits 1:0)
// TBA
#define BIT_MSK__TX_XBIST_CONF_HIGH__REG_CONFIGURE_B9_B8                                   0x03

// TX X BIST STATUS Register
#define REG_ADDR__TX_XBIST_STATUS                                        (REGTX_AVLINK | 0x0030)
// (ReadOnly, Bits 7:0)
// bit 1 and 0 for channel 0; bit 3 and 2 for channel 1; bit 5 and 4 for channel 3, bit 7 and 6 for channel 4. 00 : tx bist is disable. 01 : tx bist is enable. 10 : tx bist is running. 11 : tx bist is done.
#define BIT_MSK__TX_XBIST_STATUS__BIST_STATE                                            0xFF

// TX XBIST Test Period Count Register
#define REG_ADDR__TX_XBIST_PAT_PERIOD_CNT                                (REGTX_AVLINK | 0x0031)
// (ReadWrite, Bits 7:0)
// Number of specified test to be repeated ( 3    reg_period_cnt    255 ) for rand1; rand2; DC and hs10Bus
#define BIT_MSK__TX_XBIST_PAT_PERIOD_CNT__REG_PAT_PERIOD_CNT_B7_B0                              0xFF

// HDMI Control 0 Register
#define REG_ADDR__HDMICTL0                                               (REGTX_AVLINK | 0x0032)
// (ReadWrite, Bits 7:6)
// 00:  select ch0 01:  select ch1 10:  select ch2 11:  select ck
#define BIT_MSK__HDMICTL0__REG_Q3_SEL                                            0xC0
// (ReadWrite, Bits 2)
// 1:  Invert tx bit  for ch0;1;2CK 0:  Normal (default)
#define BIT_MSK__HDMICTL0__REG_TX_BIT_INV                                        0x04
// (ReadWrite, Bits 1)
// 1:  Has 4 full channels (RGB; CK) (default) 0:  Force CK channel to all zero
#define BIT_MSK__HDMICTL0__REG_USE_CH_MUX                                        0x02
// (ReadWrite, Bits 0)
// 1:  Bit swap Q data out from HDMI2 encoder 0:  No swap (default)
#define BIT_MSK__HDMICTL0__REG_Q_9T0                                             0x01

// HDMI Control 1 Register
#define REG_ADDR__HDMICTL1                                               (REGTX_AVLINK | 0x0033)
// (ReadWrite, Bits 7:6)
// 00:  select ch0 01:  select ch1 10:  select ch2 11:  select ck
#define BIT_MSK__HDMICTL1__REG_QC_SEL                                            0xC0
// (ReadWrite, Bits 5:4)
// 00:  select ch0 01:  select ch1 10:  select ch2 11:  select ck
#define BIT_MSK__HDMICTL1__REG_Q2_SEL                                            0x30
// (ReadWrite, Bits 3:2)
// 00:  select ch0 01:  select ch1 10:  select ch2 11:  select ck
#define BIT_MSK__HDMICTL1__REG_Q1_SEL                                            0x0C
// (ReadWrite, Bits 1:0)
// 00:  select ch0 01:  select ch1 10:  select ch2 11:  select ck
#define BIT_MSK__HDMICTL1__REG_Q0_SEL                                            0x03

// AVLINK SW Reset Register
#define REG_ADDR__SW_RST                                                 (REGTX_AVLINK | 0x0034)
// (ReadWrite, Bits 2)
// Software reset for MHL3 CTS logic 1 - Reset 0 - Normal operation (default)
#define BIT_MSK__SW_RST__REG_MHL3CTS_RST                                       0x04

//***************************************************************************
// TMDS_BIST_REG. Address: 40
// Config Register
#define REG_ADDR__RX_TBIST_CTRL_1                                        (TMDS_BIST_REG | 0x0091)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_1__REG_BIST_CTRL_1                                       0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_2                                        (TMDS_BIST_REG | 0x0092)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_2__REG_BIST_CTRL_2                                       0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_3                                        (TMDS_BIST_REG | 0x0093)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_3__REG_BIST_CTRL_3                                       0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_4                                        (TMDS_BIST_REG | 0x0094)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_4__REG_BIST_CTRL_4                                       0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_5                                        (TMDS_BIST_REG | 0x0095)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_5__REG_BIST_CTRL_5                                       0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_6                                        (TMDS_BIST_REG | 0x0096)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_6__REG_BIST_CTRL_6                                       0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_7                                        (TMDS_BIST_REG | 0x0097)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_7__REG_BIST_CTRL_7                                       0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_8                                        (TMDS_BIST_REG | 0x0098)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_8__REG_BIST_CTRL_8                                       0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_9                                        (TMDS_BIST_REG | 0x0099)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_9__REG_BIST_CTRL_9                                       0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_10                                       (TMDS_BIST_REG | 0x009A)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_10__REG_BIST_CTRL_10                                      0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_11                                       (TMDS_BIST_REG | 0x009B)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_11__REG_BIST_CTRL_11                                      0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_12                                       (TMDS_BIST_REG | 0x009C)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_12__REG_BIST_CTRL_12                                      0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_13                                       (TMDS_BIST_REG | 0x009D)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_13__REG_BIST_CTRL_13                                      0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_14                                       (TMDS_BIST_REG | 0x009E)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_14__REG_BIST_CTRL_14                                      0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_15                                       (TMDS_BIST_REG | 0x009F)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_15__REG_BIST_CTRL_15                                      0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_16                                       (TMDS_BIST_REG | 0x00A0)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_16__REG_BIST_CTRL_16                                      0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_17                                       (TMDS_BIST_REG | 0x00A1)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_17__REG_BIST_CTRL_17                                      0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_18                                       (TMDS_BIST_REG | 0x00A2)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_18__REG_BIST_CTRL_18                                      0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_19                                       (TMDS_BIST_REG | 0x00A3)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_19__REG_BIST_CTRL_19                                      0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_20                                       (TMDS_BIST_REG | 0x00A4)
// (ReadWrite, Bits 7:0)
// TMDS BIST config control register
#define BIT_MSK__RX_TBIST_CTRL_20__REG_BIST_CTRL_20                                      0xFF

// Config Register
#define REG_ADDR__RX_TBIST_CTRL_21                                       (TMDS_BIST_REG | 0x00A5)
// (ReadWrite, Bits 7:0)
// bit #0 - reg_rxbist_vgb_en bit #1 - reg_tmds0_bist_vgb_mask bit #2 - enbaled tx tbist bit #3 - enable txbist_vgb
#define BIT_MSK__RX_TBIST_CTRL_21__REG_BIST_CTRL_21                                      0xFF

// HANA BIST BLUE CONTROL0 Register
#define REG_ADDR__CONFIG_BIST_10BIT_B_1ST_0                              (TMDS_BIST_REG | 0x00A7)
// (ReadWrite, Bits 7:0)
// Hana Bist blue
#define BIT_MSK__CONFIG_BIST_10BIT_B_1ST_0__REG_BIST_10BIT_BLUE_1ST_B7_B0                         0xFF

// HANA BIST BLUE CONTROL1 Register
#define REG_ADDR__CONFIG_BIST_10BIT_B_1ST_1                              (TMDS_BIST_REG | 0x00A8)
// (ReadWrite, Bits 7:2)
// reserved
#define BIT_MSK__CONFIG_BIST_10BIT_B_1ST_1__RSVD                                                  0xFC
// (ReadWrite, Bits 1:0)
// Hana Bist blue
#define BIT_MSK__CONFIG_BIST_10BIT_B_1ST_1__REG_BIST_10BIT_BLUE_1ST_B9_B8                         0x03

// HANA BIST BLUE CONTROL 2 Register
#define REG_ADDR__CONFIG_BIST_10BIT_B_2ND_0                              (TMDS_BIST_REG | 0x00A9)
// (ReadWrite, Bits 7:0)
// Hana Bist blue
#define BIT_MSK__CONFIG_BIST_10BIT_B_2ND_0__REG_BIST_10BIT_BLUE_2ND_B7_B0                         0xFF

// HANA BIST BLUE CONTROL3 Register
#define REG_ADDR__CONFIG_BIST_10BIT_B_2ND_1                              (TMDS_BIST_REG | 0x00AA)
// (ReadWrite, Bits 1:0)
// Hana Bist blue
#define BIT_MSK__CONFIG_BIST_10BIT_B_2ND_1__REG_BIST_10BIT_BLUE_2ND_B9_B8                         0x03

// HANA BIST Green CONTROL0 Register
#define REG_ADDR__CONFIG_BIST_10BIT_G_1ST_0                              (TMDS_BIST_REG | 0x00AB)
// (ReadWrite, Bits 7:0)
// Hana Bist Green
#define BIT_MSK__CONFIG_BIST_10BIT_G_1ST_0__REG_BIST_10BIT_GREEN_1ST_B7_B0                        0xFF

// HANA BIST Green CONTROL1 Register
#define REG_ADDR__CONFIG_BIST_10BIT_G_1ST_1                              (TMDS_BIST_REG | 0x00AC)
// (ReadWrite, Bits 1:0)
// Hana Bist Green
#define BIT_MSK__CONFIG_BIST_10BIT_G_1ST_1__REG_BIST_10BIT_GREEN_1ST_B9_B8                        0x03

// HANA BIST Green CONTROL2 Register
#define REG_ADDR__CONFIG_BIST_10BIT_G_2ND_0                              (TMDS_BIST_REG | 0x00AD)
// (ReadWrite, Bits 7:0)
// Hana Bist Green
#define BIT_MSK__CONFIG_BIST_10BIT_G_2ND_0__REG_BIST_10BIT_GREEN_2ND_B7_B0                        0xFF

// HANA BIST Green CONTROL3 Register
#define REG_ADDR__CONFIG_BIST_10BIT_G_2ND_1                              (TMDS_BIST_REG | 0x00AE)
// (ReadWrite, Bits 1:0)
// Hana Bist Green
#define BIT_MSK__CONFIG_BIST_10BIT_G_2ND_1__REG_BIST_10BIT_GREEN_2ND_B9_B8                        0x03

// HANA BIST Red CONTROL0 Register
#define REG_ADDR__CONFIG_BIST_10BIT_R_1ST_0                              (TMDS_BIST_REG | 0x00AF)
// (ReadWrite, Bits 7:0)
// Hana Bist Red
#define BIT_MSK__CONFIG_BIST_10BIT_R_1ST_0__REG_BIST_10BIT_RED_1ST_B7_B0                          0xFF

// HANA BIST Red CONTROL1 Register
#define REG_ADDR__CONFIG_BIST_10BIT_R_1ST_1                              (TMDS_BIST_REG | 0x00B0)
// (ReadWrite, Bits 1:0)
// Hana Bist Red
#define BIT_MSK__CONFIG_BIST_10BIT_R_1ST_1__REG_BIST_10BIT_RED_1ST_B9_B8                          0x03

// HANA BIST Red CONTROL2 Register
#define REG_ADDR__CONFIG_BIST_10BIT_R_2ND_0                              (TMDS_BIST_REG | 0x00B1)
// (ReadWrite, Bits 7:0)
// Hana Bist Red
#define BIT_MSK__CONFIG_BIST_10BIT_R_2ND_0__REG_BIST_10BIT_RED_2ND_B7_B0                          0xFF

// HANA BIST Red CONTROL3 Register
#define REG_ADDR__CONFIG_BIST_10BIT_R_2ND_1                              (TMDS_BIST_REG | 0x00B2)
// (ReadWrite, Bits 1:0)
// Hana Bist Red
#define BIT_MSK__CONFIG_BIST_10BIT_R_2ND_1__REG_BIST_10BIT_RED_2ND_B9_B8                          0x03

// TMDS STPG LUM #0 Register
#define REG_ADDR__STPG_LUM0                                              (TMDS_BIST_REG | 0x00B4)
// (ReadWrite, Bits 7:0)
// Programmable value for channel R  for STPG Simp92 with Zone ID V5_H4 - V5_H13
#define BIT_MSK__STPG_LUM0__V5_H4TO13_R                                           0xFF

// TMDS STPG LUM #1 Register
#define REG_ADDR__STPG_LUM1                                              (TMDS_BIST_REG | 0x00B5)
// (ReadWrite, Bits 7:0)
// Programmable value for channel G  for STPG Simp92 with Zone ID V5_H4 - V5_H13
#define BIT_MSK__STPG_LUM1__V5_H4TO13_G                                           0xFF

// TMDS STPG LUM #2 Register
#define REG_ADDR__STPG_LUM2                                              (TMDS_BIST_REG | 0x00B6)
// (ReadWrite, Bits 7:0)
// Programmable value for channel B  for STPG Simp92 with Zone ID V5_H4 - V5_H13
#define BIT_MSK__STPG_LUM2__V5_H4TO13_B                                           0xFF

// TMDS STPG LUM #3 Register
#define REG_ADDR__STPG_LUM0B                                             (TMDS_BIST_REG | 0x00B7)
// (ReadWrite, Bits 7:0)
// Programmable value for channel R for STPG Simp92 with Zone V7_H4 - V7_H13
#define BIT_MSK__STPG_LUM0B__V7_H4TO13_R                                           0xFF

// TMDS STPG LUM #4 Register
#define REG_ADDR__STPG_LUM1B                                             (TMDS_BIST_REG | 0x00B8)
// (ReadWrite, Bits 7:0)
// Programmable value for channel G for STPG Simp92 with Zone V7_H4 - V7_H13
#define BIT_MSK__STPG_LUM1B__V7_H4TO13_G                                           0xFF

// TMDS STPG LUM #5 Register
#define REG_ADDR__STPG_LUM2B                                             (TMDS_BIST_REG | 0x00B9)
// (ReadWrite, Bits 7:0)
// Programmable value for channel B for STPG Simp92 with Zone V7_H4 - V7_H13
#define BIT_MSK__STPG_LUM2B__V7_H4TO13_B                                           0xFF

// TMDS STPG #0 Register
#define REG_ADDR__REG_V5_H4TO13                                          (TMDS_BIST_REG | 0x00BA)
// (ReadWrite, Bits 7:0)
// NVRAM data for V5H4TO13
#define BIT_MSK__REG_V5_H4TO13__V5_H4TO13                                             0xFF

// TMDS STPG #1 Register
#define REG_ADDR__REG_V6_H2                                              (TMDS_BIST_REG | 0x00BB)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H2
#define BIT_MSK__REG_V6_H2__V6_H2                                                 0xFF

// TMDS STPG #2 Register
#define REG_ADDR__REG_V6_H3                                              (TMDS_BIST_REG | 0x00BC)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H3
#define BIT_MSK__REG_V6_H3__V6_H3                                                 0xFF

// TMDS STPG #3 Register
#define REG_ADDR__REG_V6_H4                                              (TMDS_BIST_REG | 0x00BD)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H4
#define BIT_MSK__REG_V6_H4__V6_H4                                                 0xFF

// TMDS STPG #4 Register
#define REG_ADDR__REG_V6_H5                                              (TMDS_BIST_REG | 0x00BE)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H5
#define BIT_MSK__REG_V6_H5__V6_H5                                                 0xFF

// TMDS STPG #5 Register
#define REG_ADDR__REG_V6_H6                                              (TMDS_BIST_REG | 0x00BF)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H6
#define BIT_MSK__REG_V6_H6__V6_H6                                                 0xFF

// TMDS STPG #6 Register
#define REG_ADDR__REG_V6_H7                                              (TMDS_BIST_REG | 0x00C0)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H7
#define BIT_MSK__REG_V6_H7__V6_H7                                                 0xFF

// TMDS STPG #7 Register
#define REG_ADDR__REG_V6_H8                                              (TMDS_BIST_REG | 0x00C1)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H8
#define BIT_MSK__REG_V6_H8__V6_H8                                                 0xFF

// TMDS STPG #8 Register
#define REG_ADDR__REG_V6_H9                                              (TMDS_BIST_REG | 0x00C2)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H9
#define BIT_MSK__REG_V6_H9__V6_H9                                                 0xFF

// TMDS STPG #9 Register
#define REG_ADDR__REG_V6_H10                                             (TMDS_BIST_REG | 0x00C3)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H10
#define BIT_MSK__REG_V6_H10__V6_H10                                                0xFF

// TMDS STPG #10 Register
#define REG_ADDR__REG_V6_H11                                             (TMDS_BIST_REG | 0x00C4)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H11
#define BIT_MSK__REG_V6_H11__V6_H11                                                0xFF

// TMDS STPG #11 Register
#define REG_ADDR__REG_V6_H12                                             (TMDS_BIST_REG | 0x00C5)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H12
#define BIT_MSK__REG_V6_H12__V6_H12                                                0xFF

// TMDS STPG #12 Register
#define REG_ADDR__REG_V6_H13                                             (TMDS_BIST_REG | 0x00C6)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H13
#define BIT_MSK__REG_V6_H13__V6_H13                                                0xFF

// TMDS STPG #13 Register
#define REG_ADDR__REG_V6_H14                                             (TMDS_BIST_REG | 0x00C7)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H14
#define BIT_MSK__REG_V6_H14__V6_H14                                                0xFF

// TMDS STPG #14 Register
#define REG_ADDR__REG_V6_H15                                             (TMDS_BIST_REG | 0x00C8)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H15
#define BIT_MSK__REG_V6_H15__V6_H15                                                0xFF

// TMDS STPG #15 Register
#define REG_ADDR__REG_V6_H16                                             (TMDS_BIST_REG | 0x00C9)
// (ReadWrite, Bits 7:0)
// NVRAM data for V6H16
#define BIT_MSK__REG_V6_H16__V6_H16                                                0xFF

// TMDS STPG #16 Register
#define REG_ADDR__REG_V7_CONST61                                         (TMDS_BIST_REG | 0x00CA)
// (ReadWrite, Bits 7:0)
// data for V7
#define BIT_MSK__REG_V7_CONST61__V7_CONST61                                            0xFF

// TMDS STPG #17 Register
#define REG_ADDR__REG_V7_CONST50                                         (TMDS_BIST_REG | 0x00CB)
// (ReadWrite, Bits 7:0)
// data for V7
#define BIT_MSK__REG_V7_CONST50__V7_CONST50                                            0xFF

// TMDS STPG #18 Register
#define REG_ADDR__REG_V7_CONST102                                        (TMDS_BIST_REG | 0x00CC)
// (ReadWrite, Bits 7:0)
// data for V7
#define BIT_MSK__REG_V7_CONST102__V7_CONST102                                           0xFF

// TMDS STPG #19 Register
#define REG_ADDR__REG_V9_CONST204                                        (TMDS_BIST_REG | 0x00CD)
// (ReadWrite, Bits 7:0)
// data for V9
#define BIT_MSK__REG_V9_CONST204__V9_CONST204                                           0xFF

// TMDS STPG #20 Register
#define REG_ADDR__REG_V9_CONST205                                        (TMDS_BIST_REG | 0x00CE)
// (ReadWrite, Bits 7:0)
// data for V9
#define BIT_MSK__REG_V9_CONST205__V9_CONST205                                           0xFF

// TMDS STPG #21 Register
#define REG_ADDR__REG_V9_CONST209                                        (TMDS_BIST_REG | 0x00CF)
// (ReadWrite, Bits 7:0)
// data for V9
#define BIT_MSK__REG_V9_CONST209__V9_CONST209                                           0xFF

// TMDS STPG #22 Register
#define REG_ADDR__REG_V9_CONST229                                        (TMDS_BIST_REG | 0x00D0)
// (ReadWrite, Bits 7:0)
// data for V9
#define BIT_MSK__REG_V9_CONST229__V9_CONST229                                           0xFF

// TMDS STPG #23 Register
#define REG_ADDR__REG_V9_CONST231                                        (TMDS_BIST_REG | 0x00D1)
// (ReadWrite, Bits 7:0)
// data for V9
#define BIT_MSK__REG_V9_CONST231__V9_CONST231                                           0xFF

// TMDS STPG #24 Register
#define REG_ADDR__REG_V10_H3TO4                                          (TMDS_BIST_REG | 0x00D2)
// (ReadWrite, Bits 7:0)
// V10_H3TO4
#define BIT_MSK__REG_V10_H3TO4__V10_H3TO4                                             0xFF

// TMDS STPG #25 Register
#define REG_ADDR__REG_V10_H5TO6                                          (TMDS_BIST_REG | 0x00D3)
// (ReadWrite, Bits 7:0)
// V10_H5TO6
#define BIT_MSK__REG_V10_H5TO6__V10_H5TO6                                             0xFF

// TMDS STPG #26 Register
#define REG_ADDR__REG_V10_H7TO8                                          (TMDS_BIST_REG | 0x00D4)
// (ReadWrite, Bits 7:0)
// V10_H7TO8
#define BIT_MSK__REG_V10_H7TO8__V10_H7TO8                                             0xFF

// TMDS STPG #27 Register
#define REG_ADDR__REG_V10_H9TO10                                         (TMDS_BIST_REG | 0x00D5)
// (ReadWrite, Bits 7:0)
// V10_H9TO10
#define BIT_MSK__REG_V10_H9TO10__V10_H9TO10                                            0xFF

// TMDS STPG #28 Register
#define REG_ADDR__REG_V10_H11TO12                                        (TMDS_BIST_REG | 0x00D6)
// (ReadWrite, Bits 7:0)
// V10_H11TO12
#define BIT_MSK__REG_V10_H11TO12__V10_H11TO12                                           0xFF

// TMDS STPG #29 Register
#define REG_ADDR__REG_V10_H13TO14                                        (TMDS_BIST_REG | 0x00D7)
// (ReadWrite, Bits 7:0)
// V10_H13TO14
#define BIT_MSK__REG_V10_H13TO14__V10_H13TO14                                           0xFF

// TMDS 1 BIST CNTL Register
#define REG_ADDR__BIST_CTRL                                              (TMDS_BIST_REG | 0x00D8)
// (ReadWrite, Bits 4)
// Bist start register is a write only  register. To start a Bist sequence 1 to this register. This will generate a pulse. The falling edge of the pulse triggers the bist. Read to this register will provide unknown value.All bist control logic gets reset at the start of a bist including the read only status registers.  Other configuration registers do not get reset.
#define BIT_MSK__BIST_CTRL__REG_BIST_START                                        0x10
// (ReadWrite, Bits 3)
// 1b 0  The bist module runs the pattern check  for programmed number of frames. 1b 1  The bist module runs continuously till the bist is disabled.
#define BIT_MSK__BIST_CTRL__REG_BIST_CONT_PROG_DURAT                              0x08
// (ReadWrite, Bits 2)
// The enable bit for Samsung Test Pattern Generator. When set, it have higher priority than other BIST settings and reg_stpg_sel[3:0] will decide which pattern to generate. When clear, STPG is disabled. 1'b 0 -- clear (default) 1'b 1 -- set
#define BIT_MSK__BIST_CTRL__REG_STPG_EN                                           0x04
// (ReadWrite, Bits 1)
// 0  the bist module is out of  reset state      1  the bist module is  forced to be in the reset state.         The Bist reset bit is provided as a backup. Normal operation of Bist does not require any toggling of this bit. This is used only for lab debug.
#define BIT_MSK__BIST_CTRL__REG_BIST_RESET                                        0x02
// (ReadWrite, Bits 0)
// 0  bist is disabled       1 -  bist is enabled        Whenever bist is enabled normal operation of the chip ceases and the chip goes into bist mode.
#define BIT_MSK__BIST_CTRL__REG_BIST_ENABLE                                       0x01

// TMDS 1 BIST DURATION0 Register
#define REG_ADDR__BIST_DURATION0                                         (TMDS_BIST_REG | 0x00D9)
// (ReadWrite, Bits 7:0)
// BIST Duration register is used to specify the duration of a test in frames. Counting of frames starts with the first vsync falling edge detected after the bist has been enabled. Also a programmed value of n results in n+1 frames. (If zero is programmed; the bist/tx is active for 1 frame)
#define BIT_MSK__BIST_DURATION0__REG_BIST_DURATION_B7_B0                               0xFF

// TMDS 1 BIST DURATION1 Register
#define REG_ADDR__BIST_DURATION1                                         (TMDS_BIST_REG | 0x00DA)
// (ReadWrite, Bits 7:0)
// Bist duration[15:8]; total is 23 bits.
#define BIT_MSK__BIST_DURATION1__REG_BIST_DURATION_B15_B8                              0xFF

// TMDS 1 BIST DURATION2 Register
#define REG_ADDR__BIST_DURATION2                                         (TMDS_BIST_REG | 0x00DB)
// (ReadWrite, Bits 7:0)
// Bist duration[23:16]; total is 24 bits.
#define BIT_MSK__BIST_DURATION2__REG_BIST_DURATION_B23_B16                             0xFF

// TMDS 1 BIST TEST_SEL Register
#define REG_ADDR__BIST_TEST_SEL                                          (TMDS_BIST_REG | 0x00DC)
// (ReadWrite, Bits 6:5)
// 00-CTL lines[3:0] are static (00);  01-CTL lines[3:0] follow a ramp pattern where each count is valid for 2 clocks;  10-CTL line[3:0] follow a random pattern where each pattern is valid for 2 clocks;  11-reserved (do not use)
#define BIT_MSK__BIST_TEST_SEL__REG_BIST_TEST_SELECT                                  0x60
// (ReadWrite, Bits 4:0)
// 5b 0000X  Walking one pattern 5b 0001X   Ramping pattern       5b 0010X  LFSR pattern1         5b 0011X  LFSR pattern 2          5b 0100X  Static pattern 1           5b 0101X  Static pattern 2             5b 0110X  Max switching pattern            5b 0111X  Two column max- switching pattern            5b 1000X  SDVO/TMDS half clock pattern       5b 1001X  SDVO/TMDS 8 bit  static  pattern         5b 1010X  TMDS 10 bit static  pattern.  5'b 1011X - Enable all 4 TMDS patterns at one time      The pattern corresponding to 4h 10  is applicable only to TMDS transmit and are not applicable for SDVO receive test.
#define BIT_MSK__BIST_TEST_SEL__REG_BIST_PATTERN_SELECT                               0x1F

// TMDS 1 BIST VIDEO_MODE Register
#define REG_ADDR__BIST_VIDEO_MODE                                        (TMDS_BIST_REG | 0x00DD)
// (ReadWrite, Bits 7:4)
// Samsung Test Pattern select. This only takes effect when the enable bit (reg_stpg_en) is set. 4b 0000  Full screen red (default)   4b 0001  Full screen green    4b 0010  Full screen blue                   4b 0011  Full screen black    4b 0100  Full screen white   4b 0101  Ramp, from (0, 0, 0) to (255, 255, 255), each color repeats 5 pixels 4b 0110  Chess. The screen is divided into 8x6 blocks, with the size of 160x120. Black and white blocks alternate horizontally and vertically. 4b 0111  Color bar. 8 color strips with the width of 160 pixels. 4b 1000  Simp92. Simplified version of Master Pattern Generator #92 others      reserved
#define BIT_MSK__BIST_VIDEO_MODE__REG_STPG_SEL                                          0xF0
// (ReadWrite, Bits 3)
// Setting this bit 1 forces the DE going to the TMDS core to be forced to be 1.
#define BIT_MSK__BIST_VIDEO_MODE__REG_BIST_VIDEO_MODE                                   0x08
// (ReadWrite, Bits 2:0)
// reserved
#define BIT_MSK__BIST_VIDEO_MODE__RSVD                                                  0x07

// TMDS 1 BIST 8BIT_PATTERN Register
#define REG_ADDR__BIST_8BIT_PATTERN                                      (TMDS_BIST_REG | 0x00DE)
// (ReadWrite, Bits 7:0)
//
#define BIT_MSK__BIST_8BIT_PATTERN__REG_BIST_8BIT_PATTERN                                 0xFF

// TMDS 1 BIST 10BIT_PATTERN_L Register
#define REG_ADDR__BIST_10BIT_PATTERN_L                                   (TMDS_BIST_REG | 0x00DF)
// (ReadWrite, Bits 7:0)
// Lower 8 bits of the static 10-bit pattern to be provided to the TMDS transmitter
#define BIT_MSK__BIST_10BIT_PATTERN_L__REG_BIST_10BIT_PATTERN_B7_B0                          0xFF

// TMDS 1 BIST 10BIT_PATTERN_U Register
#define REG_ADDR__BIST_10BIT_PATTERN_U                                   (TMDS_BIST_REG | 0x00E0)
// (ReadWrite, Bits 1:0)
// Upper 2 bits of the static 10-bit pattern to be provided to the TMDS transmitter
#define BIT_MSK__BIST_10BIT_PATTERN_U__REG_BIST_10BIT_PATTERN_B9_B8                          0x03

// TMDS 1 BIST status Register
#define REG_ADDR__BIST_STATUS                                            (TMDS_BIST_REG | 0x00E1)
// (ReadOnly, Bits 1:0)
// [1:0]  Bist idle/busy/complete bits 00 b  Bist is idle 01 b  Bist is busy (test in           progress) 10 b  Bist is complete and result is ready in the BIST_RESULT  register
#define BIT_MSK__BIST_STATUS__BIST_CONFIG_STATUS                                    0x03

// TMDS 1 BIST RESULT Register
#define REG_ADDR__BIST_RESULT                                            (TMDS_BIST_REG | 0x00E2)
// (ReadWrite, Bits 7:6)
// reserved
#define BIT_MSK__BIST_RESULT__RSVD                                                  0xC0
// (ReadOnly, Bits 5)
// SDVO receive ctl error.
#define BIT_MSK__BIST_RESULT__BIST_CONFIG_CTL_ERROR                                 0x20
// (ReadOnly, Bits 4)
// SDVO receive. error count overflow . This bit gets set if  any of the error counters         overflows the max 16 bit count. If this bit is set it may  not be possible to pinpoint  which counter overflowed.All the error bits are valid only if the bit 0 indicates that bist test failed.
#define BIT_MSK__BIST_RESULT__BIST_CONFIG_OVERFLOW                                  0x10
// (ReadOnly, Bits 3)
// SDVO receive. de error
#define BIT_MSK__BIST_RESULT__BIST_CONFIG_DE_ERROR                                  0x08
// (ReadOnly, Bits 2)
// SDVO receive. vsync error
#define BIT_MSK__BIST_RESULT__BIST_CONFIG_VSYNC_ERROR                               0x04
// (ReadOnly, Bits 1)
// SDVO receive. hsync error
#define BIT_MSK__BIST_RESULT__BIST_CONFIG_HSYNC_ERROR                               0x02
// (ReadOnly, Bits 0)
// Bist pass/fail bit        0  bist test passed        1  bist test failed
#define BIT_MSK__BIST_RESULT__BIST_CONFIG_FAIL                                      0x01

// TMDS 1 BIST P_ERROR_COUNT_0 Register
#define REG_ADDR__BIST_P_ERR_CNT_0                                       (TMDS_BIST_REG | 0x00E3)
// (ReadOnly, Bits 7:0)
// Lower 8 bits of the pixel error count. This register is a read only register. Its contents are valid only if the bist test failed.A received pixel is in error if any of the 3 channels (Red. Green. Blue) have a value mismatch between generated value and received value.
#define BIT_MSK__BIST_P_ERR_CNT_0__BIST_P_ERROR_COUNT_0                                  0xFF

// TMDS 1 BIST P_ERROR_COUNT_1 Register
#define REG_ADDR__BIST_P_ERR_CNT_1                                       (TMDS_BIST_REG | 0x00E4)
// (ReadOnly, Bits 7:0)
// Upper 8 bits of the pixel error count. This register is a read only register. Its contents are valid only if the bist test failed.A received pixel is in error if any of the 3 channels (Red. Green. Blue) have a value mismatch between generated value and received value.If the any of the error count registers exceeds the max 16 bit count then the overflow bit is set in BIST_status register.
#define BIT_MSK__BIST_P_ERR_CNT_1__BIST_P_ERROR_COUNT_1                                  0xFF

// TMDS 1 BIST R_ERROR_COUNT_0 Register
#define REG_ADDR__BIST_R_ERR_CNT_0                                       (TMDS_BIST_REG | 0x00E5)
// (ReadOnly, Bits 7:0)
// Lower 8 bits of the red channel error count. This register is a read only register. Its contents are valid only if the bist test failed.A channel is in error if   there is a mismatch between   generated 8 bit value and received 8 bit value
#define BIT_MSK__BIST_R_ERR_CNT_0__BIST_R_ERROR_COUNT_0                                  0xFF

// TMDS 1 BIST R_ERROR_COUNT_1 Register
#define REG_ADDR__BIST_R_ERR_CNT_1                                       (TMDS_BIST_REG | 0x00E6)
// (ReadOnly, Bits 7:0)
// Upper 8 bits of the red channel error count. This register is a read only register. Its contents are valid only if the bist test failed. A channel is in error if there is a mismatch between generated 8-bit values and received 8-bit value.If the any of the error count registers exceeds the max 16 bit count then the overflow bit is set in BIST_status register.
#define BIT_MSK__BIST_R_ERR_CNT_1__BIST_R_ERROR_COUNT_1                                  0xFF

// TMDS 1 BIST G_ERROR_COUNT_0 Register
#define REG_ADDR__BIST_G_ERR_CNT_0                                       (TMDS_BIST_REG | 0x00E7)
// (ReadOnly, Bits 7:0)
// Lower 8 bits of the green channel error count. This register is a read only register. Its contents are valid only if the bist test failed.A channel is in error if   there is a mismatch between   generated 8-bit values and received 8-bit value.
#define BIT_MSK__BIST_G_ERR_CNT_0__BIST_G_ERROR_COUNT_0                                  0xFF

// TMDS 1 BIST G_ERROR_COUNT_1 Register
#define REG_ADDR__BIST_G_ERR_CNT_1                                       (TMDS_BIST_REG | 0x00E8)
// (ReadOnly, Bits 7:0)
// Upper 8 bits of the green channel error count. This register is a read only register. Its contents are valid only if the bist test failed.A channel is in error if there is a mismatch between generated 8-bit values and received 8-bit value.If the any of the error count registers exceeds the max 16 bit count then the overflow bit is set in BIST_status register.
#define BIT_MSK__BIST_G_ERR_CNT_1__BIST_G_ERROR_COUNT_1                                  0xFF

// TMDS 1 BIST B_ERROR_COUNT_0 Register
#define REG_ADDR__BIST_B_ERR_CNT_0                                       (TMDS_BIST_REG | 0x00E9)
// (ReadOnly, Bits 7:0)
// Lower 8 bits of the blue channel error count. This register is a read only register. Its contents are valid only if the bist test failed.A channel is in error if   there is a mismatch between   generated 8 bit value and receive
#define BIT_MSK__BIST_B_ERR_CNT_0__BIST_B_ERROR_COUNT_0                                  0xFF

// TMDS 1 BIST B_ERROR_COUNT_1 Register
#define REG_ADDR__BIST_B_ERR_CNT_1                                       (TMDS_BIST_REG | 0x00EA)
// (ReadOnly, Bits 7:0)
// Upper 8 bits of the blue channel error count. This register is a read only register. Its contents are valid only if the bist test failed.A channel is in error if there is a mismatch between generated 8-bit values and received 8-bit value.If the any of the error count registers exceeds the max 16 bit count then the overflow bit is set in BIST_status register.
#define BIT_MSK__BIST_B_ERR_CNT_1__BIST_B_ERROR_COUNT_1                                  0xFF

// TMDS 1 BIST CNTL_ERROR_COUNT Register
#define REG_ADDR__BIST_CNTL_ERR_CNT                                      (TMDS_BIST_REG | 0x00EB)
// (ReadOnly, Bits 7:0)
// 8 bit counter for counting the number of control erros.  This register is a read only register. Its contents are valid only if the bist test failed.A channel is in error if there is a mismatch between generated control[3:0] values and received control[3:0] values.
#define BIT_MSK__BIST_CNTL_ERR_CNT__BIST_CNTL_ERROR_COUNT                                 0xFF

// BIST STPG Size1 Register
#define REG_ADDR__STPG_SIZE1                                             (TMDS_BIST_REG | 0x00EC)
// (ReadWrite, Bits 7:0)
// This register byte defines the height of V1, V6 and V10 for Simp92. Default is 8'h3C. STPG_SIZE1x3 + STPG_SIZE2x5 + STPG_SIZE3x2 must be equal to 720.
#define BIT_MSK__STPG_SIZE1__REG_V1610_HEIGHT                                      0xFF

// BIST STPG Size2 Register
#define REG_ADDR__STPG_SIZE2                                             (TMDS_BIST_REG | 0x00ED)
// (ReadWrite, Bits 7:0)
// This register byte defines the height of V2, V3, V4, V8 and V9 for Simp92. Default is 8'h1E. STPG_SIZE1x3 + STPG_SIZE2x5 + STPG_SIZE3x2 must be equal to 720.
#define BIT_MSK__STPG_SIZE2__REG_V23489_HEIGHT                                     0xFF

// BIST STPG Size3 Register
#define REG_ADDR__STPG_SIZE3                                             (TMDS_BIST_REG | 0x00EE)
// (ReadWrite, Bits 7:0)
// This register byte defines the height of V5 and V7 for Simp92. Default is 8'hC3. STPG_SIZE1x3 + STPG_SIZE2x5 + STPG_SIZE3x2 must be equal to 720.
#define BIT_MSK__STPG_SIZE3__REG_V57_HEIGHT                                        0xFF

// Fake Vid Ctrl Register
#define REG_ADDR__BIST_CTRL2                                             (TMDS_BIST_REG | 0x00F5)
// (ReadWrite, Bits 7:3)
// reserved
#define BIT_MSK__BIST_CTRL2__RSVD                                                  0xF8
// (ReadWrite, Bits 2)
// if 0 enable STPG out, else Tx BIST
#define BIT_MSK__BIST_CTRL2__REG_OUT_SEL                                           0x04
// (ReadWrite, Bits 1)
// if 1 invert vsync
#define BIT_MSK__BIST_CTRL2__REG_INV_VSYNC_EN                                      0x02
// (ReadWrite, Bits 0)
// if 1 invert hsync
#define BIT_MSK__BIST_CTRL2__REG_INV_HSYNC_EN                                      0x01

// Fake Vid Ctrl Register
#define REG_ADDR__BIST_TIMING_CTRL                                       (TMDS_BIST_REG | 0x00F6)
// (ReadWrite, Bits 7:4)
// This register controls BIST timing including video timing.           8'b0000:                    //720p60 case           8'b0001:                     //576p case           8'b0010:                     //480p case           8'b0011:                     //1080p@50 8bpp case           8'b0100:                     //4k2k@30           8'b0101:                     //fully programmable           8'b1000:                     //vga            8'b1001:                     //svga           8'b1010:                     //xga           8'b1011:                      //sxga           8'b1100:                     //uxga           8'b1101:                      //4k2k@30 case
#define BIT_MSK__BIST_TIMING_CTRL__REG_TIME_MODE                                         0xF0
// (ReadWrite, Bits 3:2)
// select refresh rate:     for 4k2k:         00 - 30         01 - 25         10 - 24         11 - 24SMPTE    for 1080p:          00 - 50           01 - 60
#define BIT_MSK__BIST_TIMING_CTRL__REG_REFRESH                                           0x0C
// (ReadWrite, Bits 1:0)
// select # of bits: 00 - 8bpp 01 - 10bpp 10 - 12bpp
#define BIT_MSK__BIST_TIMING_CTRL__REG_DEPTH                                             0x03

//***************************************************************************
// REGTX_TPI. Address: 40
// TPI DTD Byte7 Register
#define REG_ADDR__TPI_MISC                                               (REGTX_TPI | 0x0007)
// (ReadWrite, Bits 0)
// Select the event to trigger the interrupt at 0x3D[2] 1: Interrupt will be asserted only after both BKSV and BCAPS are read successfully 0: Interrupt will be asserted after BKSV is read successfully
#define BIT_MSK__TPI_MISC__REG_ONLY_BKSV_DONE_SEL                                0x01

// TPI Pixel Repetition Data Register
#define REG_ADDR__TPI_PRD                                                (REGTX_TPI | 0x0008)
// (ReadWrite, Bits 4)
// Edge Select (same function as EDGE pin)                                                  0  Input data is falling edge latched (falling edge latched first in dual edge mode)                                                                                                       1  Input data is rising edge latched (rising edge latched first in dual edge mode)
#define BIT_MSK__TPI_PRD__REG_EDGE                                              0x10
// (ReadWrite, Bits 1:0)
// Clock mode register bits 00 = Pixel data is not replicated (default).   01 = Pixels are replicated once.   11 = Pixels are replicated four times.   10 = rsvd.   (Note: Whatever is programmed into these 2 bits is also the value to program into the pixel replication field of the AVI v2 data byte 5 if the EnDeMux bit is clear (register 0x4A bit 1). Use the next higher entry if EnDeMux is set. )
#define BIT_MSK__TPI_PRD__REG_ICLK                                              0x03

// Input Format Register
#define REG_ADDR__TPI_INPUT                                              (REGTX_TPI | 0x0009)
// (ReadWrite, Bits 3:2)
// 00 = Auto-selected by [1:0] 01 = Full range (0-255) 10 = Limited range (16-235) 11 = Rsvd
#define BIT_MSK__TPI_INPUT__REG_INPUT_QUAN_RANGE                                  0x0C
// (ReadWrite, Bits 1:0)
// 00 = RGB 01 = YCbCr 4:4:4 10 = YCbCr 4:2:2 11 = Internal RGB solid color (from blank register bytes)
#define BIT_MSK__TPI_INPUT__REG_INPUT_FORMAT                                      0x03

// Output Format Register
#define REG_ADDR__TPI_OUTPUT                                             (REGTX_TPI | 0x000A)
// (ReadWrite, Bits 3:2)
// 00 = Auto-selected by [1:0] 01 = Full range (0-255) 10 = Limited range (16-235) 11 = Rsvd
#define BIT_MSK__TPI_OUTPUT__REG_OUTPUT_QUAN_RANGE                                 0x0C
// (ReadWrite, Bits 1:0)
// 00 = HDMI to RGB 01 = HDMI to YCbCr 4:4:4 10 = HDMI to YCbCr 4:2:2 11 = DVI to RGB
#define BIT_MSK__TPI_OUTPUT__REG_OUTPUT_FORMAT                                     0x03

// TPI AVI Check Sum Register
#define REG_ADDR__TPI_AVI_CHSUM                                          (REGTX_TPI | 0x000C)
// (ReadWrite, Bits 7:0)
// AVI InfoFrame Checksum (also pre loads AVI Header to 0x0282; and AVI Length to 13)
#define BIT_MSK__TPI_AVI_CHSUM__TPI_AVI_CHSUM                                         0xFF

// TPI AVI Data Byte 1 Register
#define REG_ADDR__TPI_AVI_BYTE1                                          (REGTX_TPI | 0x000D)
// (ReadWrite, Bits 7:0)
// Bit[1:0] = S1:0 scan information Bit[3:2] = B1:0 bar info data valid Bit[4] = A0 active info Bit[6:5] = Y1:0 RGB/YCbCr Indicator Bit[7] = reserved
#define BIT_MSK__TPI_AVI_BYTE1__TPI_AVI_BYTE1                                         0xFF

// TPI AVI Data Byte 2 Register
#define REG_ADDR__TPI_AVI_BYTE2                                          (REGTX_TPI | 0x000E)
// (ReadWrite, Bits 7:0)
// Bit[3:0] = R 3:0 active format aspect ratio Bit[5:4] = M 1:0 picture aspect ratio Bit[7:6] = C 1:0 colorimetry info
#define BIT_MSK__TPI_AVI_BYTE2__TPI_AVI_BYTE2                                         0xFF

// TPI AVI Data Byte 3 Register
#define REG_ADDR__TPI_AVI_BYTE3                                          (REGTX_TPI | 0x000F)
// (ReadWrite, Bits 7:0)
// Bit[1:0] = SC 1:0 non-uniform scaling Bit[3:2] = Q 1:0 RGB Quantization Range Bit[6:4] = ES 2:0 Extended Colorimetry Bit[7] = ITC IT Content
#define BIT_MSK__TPI_AVI_BYTE3__TPI_AVI_BYTE3                                         0xFF

// TPI AVI Data Byte 4 Register
#define REG_ADDR__TPI_AVI_BYTE4                                          (REGTX_TPI | 0x0010)
// (ReadWrite, Bits 7:0)
// Bit[6:0] = VIC 6:0 video format identification code Bit[7] = reserved
#define BIT_MSK__TPI_AVI_BYTE4__TPI_AVI_BYTE4                                         0xFF

// TPI AVI Data Byte 5 Register
#define REG_ADDR__TPI_AVI_BYTE5                                          (REGTX_TPI | 0x0011)
// (ReadWrite, Bits 7:0)
// Bit[3:0] = PR 3:0 Pixel Repetition Factor Bit[5:4] = CN1:0 Content Type Bit[7:6] = YQ 1:0 YCC Quantiztion Range
#define BIT_MSK__TPI_AVI_BYTE5__TPI_AVI_BYTE5                                         0xFF

// TPI AVI Data Byte 6 Register
#define REG_ADDR__TPI_AVI_BYTE6                                          (REGTX_TPI | 0x0012)
// (ReadWrite, Bits 7:0)
// EndTopBar Line Number of the End of Top Bar - LSB
#define BIT_MSK__TPI_AVI_BYTE6__TPI_AVI_BYTE6                                         0xFF

// TPI AVI Data Byte 7 Register
#define REG_ADDR__TPI_AVI_BYTE7                                          (REGTX_TPI | 0x0013)
// (ReadWrite, Bits 7:0)
// EndTopBar Line Number of the End of Top Bar - MSB
#define BIT_MSK__TPI_AVI_BYTE7__TPI_AVI_BYTE7                                         0xFF

// TPI AVI Data Byte 8 Register
#define REG_ADDR__TPI_AVI_BYTE8                                          (REGTX_TPI | 0x0014)
// (ReadWrite, Bits 7:0)
// StartBottomBar Line Number of start of Bottom Bar - LSB
#define BIT_MSK__TPI_AVI_BYTE8__TPI_AVI_BYTE8                                         0xFF

// TPI AVI Data Byte 9 Register
#define REG_ADDR__TPI_AVI_BYTE9                                          (REGTX_TPI | 0x0015)
// (ReadWrite, Bits 7:0)
// StartBottomBar Line Number of start of Bottom Bar - MSB
#define BIT_MSK__TPI_AVI_BYTE9__TPI_AVI_BYTE9                                         0xFF

// TPI AVI Data Byte 10 Register
#define REG_ADDR__TPI_AVI_BYTE10                                         (REGTX_TPI | 0x0016)
// (ReadWrite, Bits 7:0)
// EndLeftBar Pixel Number of End of Left Bar - LSB
#define BIT_MSK__TPI_AVI_BYTE10__TPI_AVI_BYTE10                                        0xFF

// TPI AVI Data Byte 11 Register
#define REG_ADDR__TPI_AVI_BYTE11                                         (REGTX_TPI | 0x0017)
// (ReadWrite, Bits 7:0)
// EndLeftBar Pixel Number of End of Left Bar - MSB
#define BIT_MSK__TPI_AVI_BYTE11__TPI_AVI_BYTE11                                        0xFF

// TPI AVI Data Byte 12 Register
#define REG_ADDR__TPI_AVI_BYTE12                                         (REGTX_TPI | 0x0018)
// (ReadWrite, Bits 7:0)
// EndRightBar Pixel Number of End of Right Bar - LSB
#define BIT_MSK__TPI_AVI_BYTE12__TPI_AVI_BYTE12                                        0xFF

// TPI AVI Data Byte 13 Register
#define REG_ADDR__TPI_AVI_BYTE13                                         (REGTX_TPI | 0x0019)
// (ReadWrite, Bits 7:0)
// EndRightBar Pixel Number of End of Right Bar - MSB Write Triggers Info Frame to be sent (AVI_RPT and AVI_EN; CP_RPT and CP_EN)
#define BIT_MSK__TPI_AVI_BYTE13__TPI_AVI_BYTE13                                        0xFF

// TPI System Control Register
#define REG_ADDR__TPI_SC                                                 (REGTX_TPI | 0x001A)
// (ReadWrite, Bits 7)
// 0 = Normal operation (only r/w in 1932; no use) 1 = Jump to Flash Update
#define BIT_MSK__TPI_SC__REG_TPI_UPDATE_FLG                                    0x80
// (ReadWrite, Bits 6)
// Cipher Initialize bit (0x1A[6]) and Link encryption suspended (0x29[5:4]=11) status need to be considered together. Lets say that link is secured and protection level (0x2A[0]) changes from 1 (max security) to 0 (min security) then based on Cipher Initialize bit HW TPI will behave in two different ways. 0 (default) =] Cipher engine is NOT initialized; 0x29[5:4] =11 (Link encryption suspended); 0x29[6] = 0 (Local link not secured); COPP status change interrupt bit (0x3D[5])) is set 1 =] Initialize Cipher engine; 0x29[5:4] = 00 (normal); 0x29[6] = 0 (Local link not secured); COPP status change interrupt bit (0x3D[5])) is set
#define BIT_MSK__TPI_SC__REG_TPI_REAUTH_CTL                                    0x40
// (ReadWrite, Bits 5)
// See description in reg_tpi_output_mode[0] 0x1A bit 0
#define BIT_MSK__TPI_SC__REG_TPI_OUTPUT_MODE_B1                                0x20
// (ReadWrite, Bits 3)
// 0 = Normal audio/video 1 = Mute audio/video at receiver
#define BIT_MSK__TPI_SC__REG_TPI_AV_MUTE                                       0x08
// (ReadWrite, Bits 2)
// 0 = Not using DDC 1 = Request to use DDC
#define BIT_MSK__TPI_SC__REG_DDC_GPU_REQUEST                                   0x04
// (ReadWrite, Bits 1)
// DDC TPI SW. This is a special bit. FW can write as normal. When read back; it actually reflects the status of (GPU request and TPI grant).
#define BIT_MSK__TPI_SC__REG_DDC_TPI_SW                                        0x02
// (ReadWrite, Bits 0)
// Output mode Read: 1'b0: DVI 1'b1: HDMI Write (in Hot Plug Service Loop): 1'b0: set DVI 1'b1 set HDMI
#define BIT_MSK__TPI_SC__REG_TPI_OUTPUT_MODE_B0                                0x01

// TPI Device ID Register
#define REG_ADDR__TPI_DEV_ID                                             (REGTX_TPI | 0x001B)
// (ReadOnly, Bits 7:0)
// TPI Device ID(0xB2)
#define BIT_MSK__TPI_DEV_ID__REG_TPI_DEV_ID                                        0xFF

// TPI COPP Query Data Register
#define REG_ADDR__TPI_COPP_DATA1                                         (REGTX_TPI | 0x0029)
// (ReadOnly, Bits 7)
// 0  No link protection 1  Link secure
#define BIT_MSK__TPI_COPP_DATA1__REG_COPP_GPROT                                        0x80
// (ReadOnly, Bits 6)
// 0  No link protection 1  Link secure
#define BIT_MSK__TPI_COPP_DATA1__REG_COPP_LPROT                                        0x40
// (ReadOnly, Bits 5:4)
// 00  Normal 01  Link Lost 10  Renegotiation required 11 - Rsvd If 0x29[5:4] = 11 and 0x2A[0] goes from 0 (min protection) to 1 (max protection) 0x29[5:4] =] 00 (normal); 0x29[6] = 1 (Local link secured); status change interrupt bit (0x3D[5])) is set.
#define BIT_MSK__TPI_COPP_DATA1__REG_COPP_LINK_STATUS                                  0x30
// (ReadOnly, Bits 3)
// 0  No 1  Yes
#define BIT_MSK__TPI_COPP_DATA1__REG_COPP_HDCP_REP                                     0x08
// (ReadOnly, Bits 2)
// See description in reg_copp_conntype[1]
#define BIT_MSK__TPI_COPP_DATA1__REG_COPP_CONNTYPE_B0                                  0x04
// (ReadOnly, Bits 1)
// 0  None 1  HDCP
#define BIT_MSK__TPI_COPP_DATA1__REG_COPP_PROTYPE                                      0x02
// (ReadOnly, Bits 0)
// 00  DVI 01  HDMI 10  Unrecognized 11  UDI (Reserved)
#define BIT_MSK__TPI_COPP_DATA1__REG_COPP_CONNTYPE_B1                                  0x01

// TPI COPP Control Data Register
#define REG_ADDR__TPI_COPP_DATA2                                         (REGTX_TPI | 0x002A)
// (ReadWrite, Bits 7)
// TPI HDCP Prep Enable set to enable the TPI HDCP State machine to advance to the Prep state to start discovering if HDCP is available through reading the BCAPs and BKSV.
#define BIT_MSK__TPI_COPP_DATA2__REG_TPI_HDCP_PREP_EN                                  0x80
// (ReadWrite, Bits 6)
// Cancel Protect Request Enable when 0x2A bit 0 goes from 1 to 0 the HW TPI state machine will get reset CONFIG state 0  disabled 1  enabled
#define BIT_MSK__TPI_COPP_DATA2__REG_CANCEL_PROT_EN                                    0x40
// (ReadWrite, Bits 5)
// Interrupt encryption enable bit 0: disable (default) 1: enable
#define BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION                                   0x20
// (ReadWrite, Bits 4)
// KSV forward enable bit 0: disable (default) 1: enable
#define BIT_MSK__TPI_COPP_DATA2__REG_KSV_FORWARD                                       0x10
// (ReadWrite, Bits 3)
// Intermediate Ri check enable bit 0: disable (default) 1: enable
#define BIT_MSK__TPI_COPP_DATA2__REG_INTERM_RI_CHECK_EN                                0x08
// (ReadWrite, Bits 2)
// Double Ri check enable bit 0: disable (default) 1: enable
#define BIT_MSK__TPI_COPP_DATA2__REG_DOUBLE_RI_CHECK                                   0x04
// (ReadWrite, Bits 1)
// DDC Short Ri read enable bit 0: disable (default) 1: enable
#define BIT_MSK__TPI_COPP_DATA2__REG_DDC_SHORT_RI_RD                                   0x02
// (ReadWrite, Bits 0)
// 0  Min (no protection needed) 1  Max (HDCP required) Link encryption suspended means that link was secured before protection level (0x2A[0]) changed from 1 to 0 Renegotiation required status happens when authentication fails.
#define BIT_MSK__TPI_COPP_DATA2__REG_COPP_PROTLEVEL                                    0x01

// TPI Write BKSV1 Register
#define REG_ADDR__TPI_WR_BKSV_1                                          (REGTX_TPI | 0x002B)
// (ReadWrite, Bits 7:0)
// Reciever's Key Select Vector. It is read from the reciever and then writen to here. When software writes byte #1 of Key Selector Vector in this register the 1 clock write strobe is generated and data is loaded into the bksv1 register located inside HDCP block.  During read get Data of BKSV #1 register.
#define BIT_MSK__TPI_WR_BKSV_1__TPI_BKSV0                                             0xFF

// TPI Write BKSV2 Register
#define REG_ADDR__TPI_WR_BKSV_2                                          (REGTX_TPI | 0x002C)
// (ReadWrite, Bits 7:0)
// Reciever's Key Select Vector. It is read from the reciever and then writen to here. When software writes byte #2 of Key Selector Vector in this register the 1 clock write strobe is generated and data is loaded into the bksv2 register located inside HDCP block.  During read get Data of BKSV #2 register.
#define BIT_MSK__TPI_WR_BKSV_2__TPI_BKSV1                                             0xFF

// TPI_Write BKSV3 Register
#define REG_ADDR__TPI_WR_BKSV_3                                          (REGTX_TPI | 0x002D)
// (ReadWrite, Bits 7:0)
// Reciever's Key Select Vector. It is read from the reciever and then writen to here. When software writes byte #3 of Key Selector Vector in this register the 1 clock write strobe is generated and data is loaded into the bksv3 register located inside HDCP block.  During read get Data of BKSV #3 register.
#define BIT_MSK__TPI_WR_BKSV_3__TPI_BKSV2                                             0xFF

// TPI Write BKSV4 Register
#define REG_ADDR__TPI_WR_BKSV_4                                          (REGTX_TPI | 0x002E)
// (ReadWrite, Bits 7:0)
// Reciever's Key Select Vector. It is read from the reciever and then writen to here. When software writes byte #4 of Key Selector Vector in this register the 1 clock write strobe is generated and data is loaded into the bksv4 register located inside HDCP block.  During read get Data of BKSV #4 register.
#define BIT_MSK__TPI_WR_BKSV_4__TPI_BKSV3                                             0xFF

// TPI Write BKSV5 Register
#define REG_ADDR__TPI_WR_BKSV_5                                          (REGTX_TPI | 0x002F)
// (ReadWrite, Bits 7:0)
// Reciever's Key Select Vector. It is read from the reciever and then writen to here. When software writes byte #5 of Key Selector Vector in this register the 1 clock write strobe is generated and data is loaded into the bksv5 register located inside HDCP block. In addition if HDCP is enabled then by writing into this register the authentication is triggered.  During read get Data of BKSV #5 register.
#define BIT_MSK__TPI_WR_BKSV_5__TPI_BKSV4                                             0xFF

// TPI HDCP Revision Register
#define REG_ADDR__TPI_HDCP_REV                                           (REGTX_TPI | 0x0030)
// (ReadOnly, Bits 7:0)
// Without HDCP; 8'h00. else with HDCP is 8'h12
#define BIT_MSK__TPI_HDCP_REV__REG_TPI_HDCP_REV                                      0xFF

// TPI KSV and V' Value Data Register
#define REG_ADDR__TPI_KSV_V                                              (REGTX_TPI | 0x0031)
// (ReadOnly, Bits 7:6)
// 00=Auth. required 01=Re-auth. required 10=Authenticated 11=Repeater auth. Required
#define BIT_MSK__TPI_KSV_V__TPI_AUTH_STATE                                        0xC0
// (ReadWrite, Bits 3)
// 0=No 1=Yes
#define BIT_MSK__TPI_KSV_V__REG_COPP_VSEL_RDY                                     0x08
// (ReadWrite, Bits 2:0)
// V Value Select 000=H0 001=H1 010=H2 011=H3 100=H4 101=Rsvd 11x=Rsvd
#define BIT_MSK__TPI_KSV_V__REG_TPI_V_SEL                                         0x07

// TPI V' Value Byte 0 Register
#define REG_ADDR__TPI_VVALUE_B0                                          (REGTX_TPI | 0x0032)
// (ReadOnly, Bits 7:0)
// Based on reg_tpi_v_sel (0x31 bit[2:0]) 3'b000 = tpi_vp_h0_b0 3'b001 = tpi_vp_h1_b0 3'b010 = tpi_vp_h2_b0 3'b011 = tpi_vp_h3_b0 3'b100 = tpi_vp_h4_b0
#define BIT_MSK__TPI_VVALUE_B0__TPI_VP_HX_B0                                          0xFF

// TPI V' Value Byte 1 Register
#define REG_ADDR__TPI_VVALUE_B1                                          (REGTX_TPI | 0x0033)
// (ReadOnly, Bits 7:0)
// Based on reg_tpi_v_sel (0x31 bit[2:0]) 3'b000 = tpi_vp_h0_b1 3'b001 = tpi_vp_h1_b1 3'b010 = tpi_vp_h2_b1 3'b011 = tpi_vp_h3_b1 3'b100 = tpi_vp_h4_b1
#define BIT_MSK__TPI_VVALUE_B1__TPI_VP_HX_B1                                          0xFF

// TPI V' Value Byte 2 Register
#define REG_ADDR__TPI_VVALUE_B2                                          (REGTX_TPI | 0x0034)
// (ReadOnly, Bits 7:0)
// Based on reg_tpi_v_sel (0x31 bit[2:0]) 3'b000 = tpi_vp_h0_b2 3'b001 = tpi_vp_h1_b2 3'b010 = tpi_vp_h2_b2 3'b011 = tpi_vp_h3_b2 3'b100 = tpi_vp_h4_b2
#define BIT_MSK__TPI_VVALUE_B2__TPI_VP_HX_B2                                          0xFF

// TPI V' Value Byte 3 Register
#define REG_ADDR__TPI_VVALUE_B3                                          (REGTX_TPI | 0x0035)
// (ReadOnly, Bits 7:0)
// Based on reg_tpi_v_sel (0x31 bit[2:0]) 3'b000 = tpi_vp_h0_b3 3'b001 = tpi_vp_h1_b3 3'b010 = tpi_vp_h2_b3 3'b011 = tpi_vp_h3_b3 3'b100 = tpi_vp_h4_b3
#define BIT_MSK__TPI_VVALUE_B3__TPI_VP_HX_B3                                          0xFF

// TPI AKSV_1 Register
#define REG_ADDR__TPI_AKSV_1                                             (REGTX_TPI | 0x0036)
// (ReadOnly, Bits 7:0)
// Byte #1 of transmitters key select vector.  Five bytes: 1 is the LSB; 5 is the MSB.  All five bytes should be read from here and then written to the receiver.  Byte 5 should be written last into the receiver; and will trigger the authentication process in the receiver.
#define BIT_MSK__TPI_AKSV_1__REG_TPI_AKSV0                                         0xFF

// TPI AKSV_2 Register
#define REG_ADDR__TPI_AKSV_2                                             (REGTX_TPI | 0x0037)
// (ReadOnly, Bits 7:0)
// Byte #2 of transmitters key select vector.  Five bytes: 1 is the LSB; 5 is the MSB.  All five bytes should be read from here and then written to the receiver.  Byte 5 should be written last into the receiver; and will trigger the authentication process in the receiver.
#define BIT_MSK__TPI_AKSV_2__REG_TPI_AKSV1                                         0xFF

// TPI AKSV_3 Register
#define REG_ADDR__TPI_AKSV_3                                             (REGTX_TPI | 0x0038)
// (ReadOnly, Bits 7:0)
// Byte #3 of transmitters key select vector.  Five bytes: 1 is the LSB; 5 is the MSB.  All five bytes should be read from here and then written to the receiver.  Byte 5 should be written last into the receiver; and will trigger the authentication process in the receiver.
#define BIT_MSK__TPI_AKSV_3__REG_TPI_AKSV2                                         0xFF

// TPI AKSV_4 Register
#define REG_ADDR__TPI_AKSV_4                                             (REGTX_TPI | 0x0039)
// (ReadOnly, Bits 7:0)
// Byte #4 of transmitters key select vector.  Five bytes: 1 is the LSB; 5 is the MSB.  All five bytes should be read from here and then written to the receiver.  Byte 5 should be written last into the receiver; and will trigger the authentication process in the receiver.
#define BIT_MSK__TPI_AKSV_4__REG_TPI_AKSV3                                         0xFF

// TPI AKSV_5 Register
#define REG_ADDR__TPI_AKSV_5                                             (REGTX_TPI | 0x003A)
// (ReadOnly, Bits 7:0)
// Byte #5 of transmitters key select vector.  Five bytes: 1 is the LSB; 5 is the MSB.  All five bytes should be read from here and then written to the receiver.  Byte 5 should be written last into the receiver; and will trigger the authentication process in the receiver.
#define BIT_MSK__TPI_AKSV_5__REG_TPI_AKSV4                                         0xFF

// HPD and RSEN status Register
#define REG_ADDR__TPI_HPD_RSEN                                           (REGTX_TPI | 0x003B)
// (ReadOnly, Bits 6:5)
// RSEN State 0 - Disconnected 1 - Measuring a Connection 2 - Connection
#define BIT_MSK__TPI_HPD_RSEN__RSEN_STATE                                            0x60
// (ReadOnly, Bits 4)
// RSEN
#define BIT_MSK__TPI_HPD_RSEN__RSEN                                                  0x10
// (ReadOnly, Bits 2:1)
// HPD State 0 - Disconnected 1 - Measuring a Connection 2 - Connection
#define BIT_MSK__TPI_HPD_RSEN__HPD_STATE                                             0x06
// (ReadOnly, Bits 0)
// HPD
#define BIT_MSK__TPI_HPD_RSEN__HPD                                                   0x01

// TPI Interrupt Enable Register
#define REG_ADDR__TPI_INTR_EN                                            (REGTX_TPI | 0x003C)
// (ReadWrite, Bits 7)
// HDCP authentication status change 0=Disable 1=Enable
#define BIT_MSK__TPI_INTR_EN__REG_TPI_INTR_MASK_B7                                  0x80
// (ReadWrite, Bits 6)
// HDCP V value ready 0=Disable 1=Enable
#define BIT_MSK__TPI_INTR_EN__REG_TPI_INTR_MASK_B6                                  0x40
// (ReadWrite, Bits 5)
// COPP link status change 0=Disable 1=Enable
#define BIT_MSK__TPI_INTR_EN__REG_TPI_INTR_MASK_B5                                  0x20
// (ReadWrite, Bits 4)
// rsvd
#define BIT_MSK__TPI_INTR_EN__REG_TPI_INTR_MASK_B4                                  0x10
// (ReadWrite, Bits 3)
// KSV FIFO First enable 0=Disable 1=Enable
#define BIT_MSK__TPI_INTR_EN__REG_TPI_INTR_MASK_B3                                  0x08
// (ReadWrite, Bits 2)
// Read BKSV done enable 0=Disable 1=Enable
#define BIT_MSK__TPI_INTR_EN__REG_TPI_INTR_MASK_B2                                  0x04
// (ReadWrite, Bits 1)
// Read BKSV error enable 0=Disable 1=Enable
#define BIT_MSK__TPI_INTR_EN__REG_TPI_INTR_MASK_B1                                  0x02
// (ReadWrite, Bits 0)
// Read Rx repeater bit enable 0=Disable 1=Enable
#define BIT_MSK__TPI_INTR_EN__REG_TPI_INTR_MASK_B0                                  0x01

// TPI Interrupt Status Low Byte Register
#define REG_ADDR__TPI_INTR_ST0                                           (REGTX_TPI | 0x003D)
// (ReadWrite, Bits 7)
// indicates either that the previous authentication request (from a write to the COPP ProtLevel bit) has completed; or that an Ri mismatch has caused authentication to fail.
#define BIT_MSK__TPI_INTR_ST0__TPI_INTR_ST7                                          0x80
// (ReadWrite, Bits 6)
// indicates whether the V value computation has completed.
#define BIT_MSK__TPI_INTR_ST0__TPI_INTR_ST6                                          0x40
// (ReadWrite, Bits 5)
// indicates a status change event in the LinkStatus value so that the host driver can take appropriate action to re-establish the link.
#define BIT_MSK__TPI_INTR_ST0__TPI_INTR_ST5                                          0x20
// (ReadWrite, Bits 3)
// KSV FIFO first byte indicator
#define BIT_MSK__TPI_INTR_ST0__TPI_INTR_ST3                                          0x08
// (ReadWrite, Bits 2)
// Both BKSV and BCAPS read done indicator or only BKSV read done indicator. The behavior can controlled by 0x07[0]
#define BIT_MSK__TPI_INTR_ST0__TPI_INTR_ST2                                          0x04
// (ReadWrite, Bits 1)
// Either BKSV or BCAPS read error indicator
#define BIT_MSK__TPI_INTR_ST0__TPI_INTR_ST1                                          0x02
// (ReadWrite, Bits 0)
// BKSV read error indicator
#define BIT_MSK__TPI_INTR_ST0__TPI_INTR_ST0                                          0x01

// TPI KSV FIFO Fill Level Status Register
#define REG_ADDR__TPI_KSV_FIFO_STAT                                      (REGTX_TPI | 0x0041)
// (ReadOnly, Bits 7)
// This bit indicates KSV FIFO last byte arriving
#define BIT_MSK__TPI_KSV_FIFO_STAT__KSV_FIFO_LAST                                         0x80
// (ReadOnly, Bits 4:0)
// The number of KSV FIFO bytes to forward when KSV Forward feature is enabled
#define BIT_MSK__TPI_KSV_FIFO_STAT__KSV_FIFO_BYTES                                        0x1F

// TPI KSV FIFO Forward Port Register
#define REG_ADDR__TPI_KSV_FIFO_FORW                                      (REGTX_TPI | 0x0042)
// (ReadOnly, Bits 7:0)
// Internal port for KSV FIFO Forwarding
#define BIT_MSK__TPI_KSV_FIFO_FORW__REG_KSV_FIFO_OUT                                      0xFF

// TPI DS BCAPS Status Register
#define REG_ADDR__TPI_DS_BCAPS                                           (REGTX_TPI | 0x0044)
// (ReadOnly, Bits 7:0)
// dowstream BCAPS status
#define BIT_MSK__TPI_DS_BCAPS__REG_DS_BCAPS                                          0xFF

// TPI BStatus1 Register
#define REG_ADDR__TPI_BSTATUS1                                           (REGTX_TPI | 0x0045)
// (ReadOnly, Bits 7)
// Device Count Exceeds
#define BIT_MSK__TPI_BSTATUS1__REG_DS_DEV_EXCEED                                     0x80
// (ReadOnly, Bits 6:0)
// Device Count
#define BIT_MSK__TPI_BSTATUS1__REG_DS_DEV_CNT                                        0x7F

// TPI BStatus2 Register
#define REG_ADDR__TPI_BSTATUS2                                           (REGTX_TPI | 0x0046)
// (ReadOnly, Bits 7:5)
// Downstream BSTATUS
#define BIT_MSK__TPI_BSTATUS2__REG_DS_BSTATUS                                        0xE0
// (ReadOnly, Bits 4)
// downstream HDMI mode 0 = DVI mode 1 = HDMI mode
#define BIT_MSK__TPI_BSTATUS2__REG_DS_HDMI_MODE                                      0x10
// (ReadOnly, Bits 3)
// Down Stream Cascade Exceeds
#define BIT_MSK__TPI_BSTATUS2__REG_DS_CASC_EXCEED                                    0x08
// (ReadOnly, Bits 2:0)
// Device Depth
#define BIT_MSK__TPI_BSTATUS2__REG_DS_DEPTH                                          0x07

// TPI Video Mute Low Byte Register
#define REG_ADDR__TPI_VID_MUTE0                                          (REGTX_TPI | 0x004B)
// (ReadWrite, Bits 7:0)
// Video mute data for channel 0. When video mute is enabled; regular video is replaced by mute data.
#define BIT_MSK__TPI_VID_MUTE0__TPI_VID_MUTE0                                         0xFF

// TPI Video Mute Mid Byte Register
#define REG_ADDR__TPI_VID_MUTE1                                          (REGTX_TPI | 0x004D)
// (ReadWrite, Bits 7:0)
// Video mute data for channel 1. When video mute is enabled; regular video is replaced by mute data.
#define BIT_MSK__TPI_VID_MUTE1__TPI_VID_MUTE1                                         0xFF

// TPI Video Mute High Byte Register
#define REG_ADDR__TPI_VID_MUTE2                                          (REGTX_TPI | 0x004F)
// (ReadWrite, Bits 7:0)
// Video mute data for channel 2. When video mute is enabled; regular video is replaced by mute data.
#define BIT_MSK__TPI_VID_MUTE2__TPI_VID_MUTE2                                         0xFF

// TPI HW Debug #1 Register
#define REG_ADDR__TPI_HW_DBG1                                            (REGTX_TPI | 0x0079)
// (ReadOnly, Bits 7)
// Read KSV list successful
#define BIT_MSK__TPI_HW_DBG1__READ_KSV_LIST_DONE                                    0x80
// (ReadOnly, Bits 6)
// Read Bstatus successful
#define BIT_MSK__TPI_HW_DBG1__READ_BSTATUS_DONE                                     0x40
// (ReadOnly, Bits 5)
// Read KSV FIFO ready successful
#define BIT_MSK__TPI_HW_DBG1__READ_KSV_FIFO_RDY_DONE                                0x20
// (ReadOnly, Bits 4)
// Read R0 Prime successful
#define BIT_MSK__TPI_HW_DBG1__READ_R0_PRIME_DONE                                    0x10
// (ReadOnly, Bits 3)
// Write AKSV successful
#define BIT_MSK__TPI_HW_DBG1__WRITE_AKSV_DONE                                       0x08
// (ReadOnly, Bits 2)
// Write AN successful
#define BIT_MSK__TPI_HW_DBG1__WRITE_AN_DONE                                         0x04
// (ReadOnly, Bits 1)
// Read RX repeater successful
#define BIT_MSK__TPI_HW_DBG1__READ_RX_REPEATER_DONE                                 0x02
// (ReadOnly, Bits 0)
// Read BKSV successful
#define BIT_MSK__TPI_HW_DBG1__READ_BKSV_DONE                                        0x01

// TPI HW Debug #2 Register
#define REG_ADDR__TPI_HW_DBG2                                            (REGTX_TPI | 0x007A)
// (ReadOnly, Bits 2)
// Read 2nd RI prime successful
#define BIT_MSK__TPI_HW_DBG2__READ_RI_2ND_DONE                                      0x04
// (ReadOnly, Bits 1)
// Read RI prime successful
#define BIT_MSK__TPI_HW_DBG2__READ_RI_PRIME_DONE                                    0x02
// (ReadOnly, Bits 0)
// Read V prime successful
#define BIT_MSK__TPI_HW_DBG2__READ_V_PRIME_DONE                                     0x01

// TPI HW Debug #3 Register
#define REG_ADDR__TPI_HW_DBG3                                            (REGTX_TPI | 0x007B)
// (ReadOnly, Bits 7)
// Read KSV list error
#define BIT_MSK__TPI_HW_DBG3__READ_KSV_LIST_ERR                                     0x80
// (ReadOnly, Bits 6)
// Read Bstatus error
#define BIT_MSK__TPI_HW_DBG3__READ_BSTATUS_ERR                                      0x40
// (ReadOnly, Bits 5)
// Read KSV FIFO ready error
#define BIT_MSK__TPI_HW_DBG3__READ_KSV_FIFO_RDY_ERR                                 0x20
// (ReadOnly, Bits 4)
// Read R0 Prime error
#define BIT_MSK__TPI_HW_DBG3__READ_R0_PRIME_ERR                                     0x10
// (ReadOnly, Bits 3)
// Write AKSV error
#define BIT_MSK__TPI_HW_DBG3__WRITE_AKSV_ERR                                        0x08
// (ReadOnly, Bits 2)
// Write AN error
#define BIT_MSK__TPI_HW_DBG3__WRITE_AN_ERR                                          0x04
// (ReadOnly, Bits 1)
// Read RX repeater error
#define BIT_MSK__TPI_HW_DBG3__READ_RX_REPEATER_ERR                                  0x02
// (ReadOnly, Bits 0)
// Read BKSV error
#define BIT_MSK__TPI_HW_DBG3__READ_BKSV_ERR                                         0x01

// TPI HW Debug #4 Register
#define REG_ADDR__TPI_HW_DBG4                                            (REGTX_TPI | 0x007C)
// (ReadOnly, Bits 2)
// Read 2nd RI prime error
#define BIT_MSK__TPI_HW_DBG4__READ_RI_2ND_ERR                                       0x04
// (ReadOnly, Bits 1)
// Read RI prime error
#define BIT_MSK__TPI_HW_DBG4__READ_RI_PRIME_ERR                                     0x02
// (ReadOnly, Bits 0)
// Read V prime error
#define BIT_MSK__TPI_HW_DBG4__READ_V_PRIME_ERR                                      0x01

// TPI HW Debug #5 Register
#define REG_ADDR__TPI_HW_DBG5                                            (REGTX_TPI | 0x007D)
// (ReadOnly, Bits 7:4)
// TPI HW Repeater Authentication SM
#define BIT_MSK__TPI_HW_DBG5__TPI_DS_AUTH_CS                                        0xF0
// (ReadOnly, Bits 3:0)
// TPI HW current state
#define BIT_MSK__TPI_HW_DBG5__TPI_HW_CS                                             0x0F

// TPI HW Debug #6 Register
#define REG_ADDR__TPI_HW_DBG6                                            (REGTX_TPI | 0x007E)
// (ReadOnly, Bits 7:5)
// TPI HW Link Integrity SM
#define BIT_MSK__TPI_HW_DBG6__TPI_LINK_ENC_CS_B2_B0                                 0xE0
// (ReadOnly, Bits 4:0)
// TPI HW RX Authentication SM
#define BIT_MSK__TPI_HW_DBG6__TPI_RX_AUTH_CS_B4_B0                                  0x1F

// TPI HW Debug #7 Register
#define REG_ADDR__TPI_HW_DBG7                                            (REGTX_TPI | 0x007F)
// (ReadOnly, Bits 3:0)
// TPI HW DDC Master Controller SM
#define BIT_MSK__TPI_HW_DBG7__TPI_DDCM_CTL_CS_B3_B0                                 0x0F

// TPI HW Debug #8 Register
#define REG_ADDR__TPI_HW_DBG8                                            (REGTX_TPI | 0x0080)
// (ReadOnly, Bits 7:0)
// # of DDC master access byte (LSb) including read and write
#define BIT_MSK__TPI_HW_DBG8__REG_DDC_HDCP_ACC_NMB_B7_B0                            0xFF

// TPI HW Debug #9 Register
#define REG_ADDR__TPI_HW_DBG9                                            (REGTX_TPI | 0x0081)
// (ReadOnly, Bits 1:0)
// # of DDC master access byte (MSb) including read and write
#define BIT_MSK__TPI_HW_DBG9__REG_DDC_HDCP_ACC_NMB_B9_B8                            0x03

// TPI HW Optimization Control #0 Register
#define REG_ADDR__TPI_HW_OPT0                                            (REGTX_TPI | 0x00B8)
// (ReadWrite, Bits 1)
// HW TPI State Machine Reset (includes DDC Master) 1: Enabled 0: disabled
#define BIT_MSK__TPI_HW_OPT0__REG_HW_TPI_SM_RST                                     0x02
// (ReadWrite, Bits 0)
// TPI R0 Wait Absolute time Enable 1: use 2MHz clock to generate a ] 100ms pulse; 2MHz + 5% = 2.1MHz; 18'h34968 (18'd215400) = ~102.57ms  0: use # (0x6B9[3:0]) of vsyncs
#define BIT_MSK__TPI_HW_OPT0__REG_R0_ABSOLUTE                                       0x01

// TPI HW Optimization Control #1 Register
#define REG_ADDR__TPI_HW_OPT1                                            (REGTX_TPI | 0x00B9)
// (ReadWrite, Bits 7)
// DDC delay counter 9 bits of MSB bit
#define BIT_MSK__TPI_HW_OPT1__REG_DDC_DELAY_CNT_B1                                  0x80
// (ReadWrite, Bits 6:4)
// TPI Autnentication Retry Counter
#define BIT_MSK__TPI_HW_OPT1__REG_TPI_AUTH_RETRY_CNT_B2_B0                          0x70
// (ReadWrite, Bits 3:0)
// TPI R0 Calculation Time
#define BIT_MSK__TPI_HW_OPT1__REG_TPI_R0_CALC_TIME_B3_B0                            0x0F

// TPI HW Optimization Control #2 Register
#define REG_ADDR__TPI_HW_OPT2                                            (REGTX_TPI | 0x00BA)
// (ReadWrite, Bits 7:0)
// DDC delay counter 9 bits of lower 8-bit
#define BIT_MSK__TPI_HW_OPT2__REG_DDC_DELAY_CNT_B0                                  0xFF

// TPI HW Optimization Control #3 Register
#define REG_ADDR__TPI_HW_OPT3                                            (REGTX_TPI | 0x00BB)
// (ReadWrite, Bits 7)
// DDC delay counter 9 bits of MSB bit
#define BIT_MSK__TPI_HW_OPT3__REG_DDC_DEBUG                                         0x80
// (ReadWrite, Bits 6:4)
// Legacy ri check
#define BIT_MSK__TPI_HW_OPT3__REG_LEGACY_TPI_RI_CHECK                               0x70
// (ReadWrite, Bits 3)
// RI Check Skip
#define BIT_MSK__TPI_HW_OPT3__REG_RI_CHECK_SKIP                                     0x08
// (ReadWrite, Bits 2)
// TPI DDC Burst Mode
#define BIT_MSK__TPI_HW_OPT3__REG_TPI_DDC_BURST_MODE                                0x04
// (ReadWrite, Bits 1:0)
// TPI DDC Request Level
#define BIT_MSK__TPI_HW_OPT3__REG_TPI_DDC_REQ_LEVEL                                 0x03

// TPI Info Frame Select Register
#define REG_ADDR__TPI_INFO_FSEL                                          (REGTX_TPI | 0x00BF)
// (ReadWrite, Bits 3:0)
// InfoFrame Packet Buffer Selection. 0  Buffer 0 (19 bytes) i.e. AVI 1  Buffer 1 (31 bytes) i.e. GAMUT 2  Buffer 2 (14 bytes) i.e. Audio 3  Buffer 3 (31 bytes) i.e. SPD 4  Buffer 4 (31 bytes) i.e. MPEG 5  Buffer 5 (31 bytes) i.e. VSIF 6  Buffer 6 (31 bytes) i.e. GEN1 7  Buffer 7 (31 bytes) i.e. GEN2 8  Buffer 8 (31 bytes) i.e. GEN3 9  Buffer 9 (31 bytes) i.e. GEN4 10  Buffer 10 (31 bytes) i.e. GEN5 11  Buffer 11 (31 bytes) i.e. VSIF1 for compression/decompression 12  Buffer 12 (31 bytes) i.e. VSIF2 for compression/decompression 13  Buffer 13 (31 bytes) i.e. VSIF3 for compression/decompression 14  Buffer 14 (31 bytes) i.e. VSIF4 for compression/decompression Buffers are in priority order of how they will be sent 0; then 1; etc.
#define BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL                                      0x0F

// TPI Info Byte #0 Register
#define REG_ADDR__TPI_INFO_B0                                            (REGTX_TPI | 0x00C0)
// (ReadWrite, Bits 7:0)
// Bit[6:0] = I-F_Type InfoFrame type per CEA-861-E spec Bit[7] = always write as 1
#define BIT_MSK__TPI_INFO_B0__TPI_INFO_B0                                           0xFF

// TPI Info Byte #1 Register
#define REG_ADDR__TPI_INFO_B1                                            (REGTX_TPI | 0x00C1)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = I-F_Ver InfoFrame version per CEA-861-E spec
#define BIT_MSK__TPI_INFO_B1__TPI_INFO_B1                                           0xFF

// TPI Info Byte #2 Register
#define REG_ADDR__TPI_INFO_B2                                            (REGTX_TPI | 0x00C2)
// (ReadWrite, Bits 7:0)
// Bit[4:0] = I-F_Length InfoFrame length per CEA-861-E spec Bit[7:5] = rsvd
#define BIT_MSK__TPI_INFO_B2__TPI_INFO_B2                                           0xFF

// TPI Info Byte #3 Register
#define REG_ADDR__TPI_INFO_B3                                            (REGTX_TPI | 0x00C3)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame checksum
#define BIT_MSK__TPI_INFO_B3__TPI_INFO_B3                                           0xFF

// TPI Info Byte #4 Register
#define REG_ADDR__TPI_INFO_B4                                            (REGTX_TPI | 0x00C4)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte1
#define BIT_MSK__TPI_INFO_B4__TPI_INFO_B4                                           0xFF

// TPI Info Byte #5 Register
#define REG_ADDR__TPI_INFO_B5                                            (REGTX_TPI | 0x00C5)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte2
#define BIT_MSK__TPI_INFO_B5__TPI_INFO_B5                                           0xFF

// TPI Info Byte #6 Register
#define REG_ADDR__TPI_INFO_B6                                            (REGTX_TPI | 0x00C6)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte3
#define BIT_MSK__TPI_INFO_B6__TPI_INFO_B6                                           0xFF

// TPI Info Byte #7 Register
#define REG_ADDR__TPI_INFO_B7                                            (REGTX_TPI | 0x00C7)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte4
#define BIT_MSK__TPI_INFO_B7__TPI_INFO_B7                                           0xFF

// TPI Info Byte #8 Register
#define REG_ADDR__TPI_INFO_B8                                            (REGTX_TPI | 0x00C8)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte5
#define BIT_MSK__TPI_INFO_B8__TPI_INFO_B8                                           0xFF

// TPI Info Byte #9 Register
#define REG_ADDR__TPI_INFO_B9                                            (REGTX_TPI | 0x00C9)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte6
#define BIT_MSK__TPI_INFO_B9__TPI_INFO_B9                                           0xFF

// TPI Info Byte #10 Register
#define REG_ADDR__TPI_INFO_B10                                           (REGTX_TPI | 0x00CA)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte7
#define BIT_MSK__TPI_INFO_B10__TPI_INFO_B10                                          0xFF

// TPI Info Byte #11 Register
#define REG_ADDR__TPI_INFO_B11                                           (REGTX_TPI | 0x00CB)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte8
#define BIT_MSK__TPI_INFO_B11__TPI_INFO_B11                                          0xFF

// TPI Info Byte #12 Register
#define REG_ADDR__TPI_INFO_B12                                           (REGTX_TPI | 0x00CC)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte9
#define BIT_MSK__TPI_INFO_B12__TPI_INFO_B12                                          0xFF

// TPI Info Byte #13 Register
#define REG_ADDR__TPI_INFO_B13                                           (REGTX_TPI | 0x00CD)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte10 (the last location for Audio InfoFrames)
#define BIT_MSK__TPI_INFO_B13__TPI_INFO_B13                                          0xFF

// TPI Info Byte #14 Register
#define REG_ADDR__TPI_INFO_B14                                           (REGTX_TPI | 0x00CE)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte11
#define BIT_MSK__TPI_INFO_B14__TPI_INFO_B14                                          0xFF

// TPI Info Byte #15 Register
#define REG_ADDR__TPI_INFO_B15                                           (REGTX_TPI | 0x00CF)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte12
#define BIT_MSK__TPI_INFO_B15__TPI_INFO_B15                                          0xFF

// TPI Info Byte #16 Register
#define REG_ADDR__TPI_INFO_B16                                           (REGTX_TPI | 0x00D0)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte13
#define BIT_MSK__TPI_INFO_B16__TPI_INFO_B16                                          0xFF

// TPI Info Byte #17 Register
#define REG_ADDR__TPI_INFO_B17                                           (REGTX_TPI | 0x00D1)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte14
#define BIT_MSK__TPI_INFO_B17__TPI_INFO_B17                                          0xFF

// TPI Info Byte #18 Register
#define REG_ADDR__TPI_INFO_B18                                           (REGTX_TPI | 0x00D2)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte15
#define BIT_MSK__TPI_INFO_B18__TPI_INFO_B18                                          0xFF

// TPI Info Byte #19 Register
#define REG_ADDR__TPI_INFO_B19                                           (REGTX_TPI | 0x00D3)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte16
#define BIT_MSK__TPI_INFO_B19__TPI_INFO_B19                                          0xFF

// TPI Info Byte #20 Register
#define REG_ADDR__TPI_INFO_B20                                           (REGTX_TPI | 0x00D4)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte17
#define BIT_MSK__TPI_INFO_B20__TPI_INFO_B20                                          0xFF

// TPI Info Byte #21 Register
#define REG_ADDR__TPI_INFO_B21                                           (REGTX_TPI | 0x00D5)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte18
#define BIT_MSK__TPI_INFO_B21__TPI_INFO_B21                                          0xFF

// TPI Info Byte #22 Register
#define REG_ADDR__TPI_INFO_B22                                           (REGTX_TPI | 0x00D6)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte19
#define BIT_MSK__TPI_INFO_B22__TPI_INFO_B22                                          0xFF

// TPI Info Byte #23 Register
#define REG_ADDR__TPI_INFO_B23                                           (REGTX_TPI | 0x00D7)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte20
#define BIT_MSK__TPI_INFO_B23__TPI_INFO_B23                                          0xFF

// TPI Info Byte #24 Register
#define REG_ADDR__TPI_INFO_B24                                           (REGTX_TPI | 0x00D8)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte21
#define BIT_MSK__TPI_INFO_B24__TPI_INFO_B24                                          0xFF

// TPI Info Byte #25 Register
#define REG_ADDR__TPI_INFO_B25                                           (REGTX_TPI | 0x00D9)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte22
#define BIT_MSK__TPI_INFO_B25__TPI_INFO_B25                                          0xFF

// TPI Info Byte #26 Register
#define REG_ADDR__TPI_INFO_B26                                           (REGTX_TPI | 0x00DA)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte23
#define BIT_MSK__TPI_INFO_B26__TPI_INFO_B26                                          0xFF

// TPI Info Byte #27 Register
#define REG_ADDR__TPI_INFO_B27                                           (REGTX_TPI | 0x00DB)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte24
#define BIT_MSK__TPI_INFO_B27__TPI_INFO_B27                                          0xFF

// TPI Info Byte #28 Register
#define REG_ADDR__TPI_INFO_B28                                           (REGTX_TPI | 0x00DC)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte25
#define BIT_MSK__TPI_INFO_B28__TPI_INFO_B28                                          0xFF

// TPI Info Byte #29 Register
#define REG_ADDR__TPI_INFO_B29                                           (REGTX_TPI | 0x00DD)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = InfoFrame data byte26
#define BIT_MSK__TPI_INFO_B29__TPI_INFO_B29                                          0xFF

// TPI Info Byte #30 Register
#define REG_ADDR__TPI_INFO_B30                                           (REGTX_TPI | 0x00DE)
// (ReadWrite, Bits 7:0)
// Bit[7:0] = Last InfoFrame data byte (for amm except Audio InfoFrames)
#define BIT_MSK__TPI_INFO_B30__TPI_INFO_B30                                          0xFF

// TPI Info Enable Register
#define REG_ADDR__TPI_INFO_EN                                            (REGTX_TPI | 0x00DF)
// (ReadWrite, Bits 7)
// Enable request of selected TPI InfoFrame transmission on HDMI. To enable transmission write 1 into this bit. Have to be enabled only after packet data is written to the registers:
#define BIT_MSK__TPI_INFO_EN__REG_TPI_INFO_EN                                       0x80
// (ReadWrite, Bits 6)
// Repeat selected TPI InfoFrame Packet data each frame. Once this bit is set and transmition bit is enabled hardware would try to send InfoFrame Packet once every vblank period. Software has to disable this bit first to force clear of the transmission enable bit
#define BIT_MSK__TPI_INFO_EN__REG_TPI_INFO_RPT                                      0x40
// (ReadOnly, Bits 5)
// Selected TPI InfoFrame transmission on HDMI Enabled and Sending next vsync.
#define BIT_MSK__TPI_INFO_EN__CEA_INFO_EN                                           0x20

// VSIF Compression Control Register
#define REG_ADDR__VSIF_COMP_CNTL                                         (REGTX_TPI | 0x00E0)
// (ReadWrite, Bits 0)
// 1: send only one enabled VSIF compression packet per frame 0: send all enabled VSIF compression packets per frame (up to 4 packets)
#define BIT_MSK__VSIF_COMP_CNTL__REG_VSIF_COMP_ONE_PER_FRAME                           0x01

// TPI DDC Master Enable Register
#define REG_ADDR__TPI_DDC_MASTER_EN                                      (REGTX_TPI | 0x00F8)
// (ReadWrite, Bits 7)
// Write 1 to enable DDC master access in HW TPI mode
#define BIT_MSK__TPI_DDC_MASTER_EN__REG_HW_DDC_MASTER                                     0x80

//***************************************************************************
// REGTX_SERDES. Address: 40
// MHL Top Ctl Register
#define REG_ADDR__MHL_TOP_CTL                                            (REGTX_SERDES | 0x00B0)
// (ReadWrite, Bits 7)
// when mhl3_en is enable : mhl3_doc or mhl3_coc mode  selection 1'b0 mhl3 CoC mode 1'b1 mhl3 DoC mode
#define BIT_MSK__MHL_TOP_CTL__REG_MHL3_DOC_SEL                                      0x80
// (ReadWrite, Bits 6)
// Valid only in MHL2 mode - Normal mode or PackedPixel mode selection 1'b0 Normal mode 1'b1 PackedPixel mode
#define BIT_MSK__MHL_TOP_CTL__REG_MHL_PP_SEL                                        0x40
// (ReadWrite, Bits 5)
// MHL 3 in CE mode enable
#define BIT_MSK__MHL_TOP_CTL__REG_MHL3CE_EN                                         0x20
// (ReadWrite, Bits 4)
// HDMI mode enable for Harry PHY
#define BIT_MSK__MHL_TOP_CTL__REG_HDMI_EN                                           0x10
// (ReadWrite, Bits 3)
// driver output enable signal for channel 2
#define BIT_MSK__MHL_TOP_CTL__REG_DP2_OE                                            0x08
// (ReadWrite, Bits 2)
// driver output enable signal for channel 1
#define BIT_MSK__MHL_TOP_CTL__REG_DP1_OE                                            0x04
// (ReadWrite, Bits 1:0)
// interface data latching timing through I2C 2'b00 normal clock (default) 2'b01 Early clock increase setup margin 2'b10 Late clock, increase hold margin 2'b11 Interted clock
#define BIT_MSK__MHL_TOP_CTL__REG_IF_TIMING_CTL                                     0x03

// MHL DataPath 1st Ctl Register
#define REG_ADDR__MHL_DP_CTL0                                            (REGTX_SERDES | 0x00B1)
// (ReadWrite, Bits 7)
// driver output enable signal
#define BIT_MSK__MHL_DP_CTL0__REG_DP_OE                                             0x80
// (ReadWrite, Bits 6)
// Tx output configuration override. 1: Tx output config will be bit [5:0] 0: Tx output config will be 6'b110011 if DoC is enabled and active. Otherwise if HDCP2x is not selected; it will be controlled by HDCP1.x state machine and if HDCP2x is selected; it will b bit [5:0]
#define BIT_MSK__MHL_DP_CTL0__REG_TX_OE_OVR                                         0x40
// (ReadWrite, Bits 5:0)
// tx output mode control: 6'b000000  tx driver output disable mode 6b'111100   legacy MHL mode and PP mode 6b'110011   MHL3CE or PLL test mode 6b'110000  MHL3_mobile mode
#define BIT_MSK__MHL_DP_CTL0__REG_TX_OE                                             0x3F

// MHL DataPath 2nd Ctl Register
#define REG_ADDR__MHL_DP_CTL1                                            (REGTX_SERDES | 0x00B2)
// (ReadWrite, Bits 7:4)
// clock swing control bit
#define BIT_MSK__MHL_DP_CTL1__REG_CK_SWING_CTL                                      0xF0
// (ReadWrite, Bits 3:0)
// data swing control bit
#define BIT_MSK__MHL_DP_CTL1__REG_DT_SWING_CTL                                      0x0F

// MHL DataPath 3rd Ctl Register
#define REG_ADDR__MHL_DP_CTL2                                            (REGTX_SERDES | 0x00B3)
// (ReadWrite, Bits 7)
// After enabled, the pixel clock from eTMDS will be sent out on Tx
#define BIT_MSK__MHL_DP_CTL2__REG_CLK_BYPASS_EN                                     0x80
// (ReadWrite, Bits 6)
// driver output enable signal for clock channel
#define BIT_MSK__MHL_DP_CTL2__REG_CK_OE                                             0x40
// (ReadWrite, Bits 5:4)
// Legacy MHL mode  common mode damping clock termintaion selection 2b'00 open 2b'01 400 ohm 2b'10 200 ohm (default) 2b'11 133 ohm
#define BIT_MSK__MHL_DP_CTL2__REG_DAMP_TERM_SEL                                     0x30
// (ReadWrite, Bits 3:2)
// driver clock channel terminal selection signal 2b'00 open 2b'01 300 ohm 2b'10 150 ohm 2b'11 100 ohm (default)
#define BIT_MSK__MHL_DP_CTL2__REG_CK_TERM_SEL                                       0x0C
// (ReadWrite, Bits 1:0)
// driver data channel terminal selection signal 2b'00 open 2b'01 300 ohm 2b'10 150 ohm 2b'11 100 ohm (default)
#define BIT_MSK__MHL_DP_CTL2__REG_DT_TERM_SEL                                       0x03

// MHL DataPath 4th Ctl Register
#define REG_ADDR__MHL_DP_CTL3                                            (REGTX_SERDES | 0x00B4)
// (ReadWrite, Bits 7:4)
// bias vdsat control signal for cascade transistor for data driver
#define BIT_MSK__MHL_DP_CTL3__REG_DT_DRV_VNBC_CTL                                   0xF0
// (ReadWrite, Bits 3:0)
// bias vdsat control signal for data driver
#define BIT_MSK__MHL_DP_CTL3__REG_DT_DRV_VNB_CTL                                    0x0F

// MHL DataPath 5th Ctl Register
#define REG_ADDR__MHL_DP_CTL4                                            (REGTX_SERDES | 0x00B5)
// (ReadWrite, Bits 7:4)
// bias vdsat control signal for cascade transistor for data driver
#define BIT_MSK__MHL_DP_CTL4__REG_CK_DRV_VNBC_CTL                                   0xF0
// (ReadWrite, Bits 3:0)
// bias vdsat control signal for data driver
#define BIT_MSK__MHL_DP_CTL4__REG_CK_DRV_VNB_CTL                                    0x0F

// MHL DataPath 6th Ctl Register
#define REG_ADDR__MHL_DP_CTL5                                            (REGTX_SERDES | 0x00B6)
// (ReadWrite, Bits 7)
// When it is disable, allows MHL Tx discovery state machine to control RSEN detection automatically; When it is enabled, bit 6 will control RSEN detection
#define BIT_MSK__MHL_DP_CTL5__REG_RSEN_EN_OVR                                       0x80
// (ReadWrite, Bits 6)
// rx sense enable signal only when bit 7 is enabled.
#define BIT_MSK__MHL_DP_CTL5__REG_RSEN_EN                                           0x40
// (ReadWrite, Bits 5:4)
// fine tunning for damping resistor
#define BIT_MSK__MHL_DP_CTL5__REG_DAMP_TERM_VGS_CTL                                 0x30
// (ReadWrite, Bits 3:2)
// fine tunning for doc path term resistor
#define BIT_MSK__MHL_DP_CTL5__REG_CK_TERM_VGS_CTL                                   0x0C
// (ReadWrite, Bits 1:0)
// fine tunning for data term resistor
#define BIT_MSK__MHL_DP_CTL5__REG_DT_TERM_VGS_CTL                                   0x03

// MHL PLL 1st Ctl Register
#define REG_ADDR__MHL_PLL_CTL0                                           (REGTX_SERDES | 0x00B7)
// (ReadWrite, Bits 7)
// enable audio clock generation
#define BIT_MSK__MHL_PLL_CTL0__REG_AUD_CLK_EN                                        0x80
// (ReadWrite, Bits 6:4)
// Select frequency ratio between aud_clk and hdmi_clk 3'b000  aud_clk_freq = 5/1 hdmi_clk_freq 3'b001  aud_clk_freq = 5/2 hdmi_clk_freq 3'b010  aud_clk_freq = 5/3 hdmi_clk_freq 3'b011  aud_clk_freq = 5/5 hdmi_clk_freq 3'b100  aud_clk_freq = 5/2 hdmi_clk_freq 3'b101  aud_clk_freq = 5/4 hdmi_clk_freq 3'b110  aud_clk_freq = 5/6 hdmi_clk_freq 3'b111  aud_clk_freq = 5/10 hdmi_clk_freq
#define BIT_MSK__MHL_PLL_CTL0__REG_AUD_CLK_RATIO                                     0x70
// (ReadWrite, Bits 3:2)
// Select frequency ratio betweeen hdmi_clk and pxl_clk 2'b00 0.5x mode, hdmi_clk_freq = 0.5*pxl_clk_freq 2'b01 1x mode, hdmi_clk_freq = 1*pxl_clk_freq (default) 2'b10 2x mode, hdmi_clk_freq = 2*pxl_clk_freq 2'b11 4x mode, hdmi_clk_freq = 4*pxl_clk_freq  For MHL2 Tx auto zone, it should be set to 2'b10. For all other cases, it should programmed to 2'b01
#define BIT_MSK__MHL_PLL_CTL0__REG_HDMI_CLK_RATIO                                    0x0C
// (ReadWrite, Bits 1)
// Select the input clock from HSIC PHY (crystal) or from eTMDS 1: from HSIC PHY 0: from eTMDS
#define BIT_MSK__MHL_PLL_CTL0__REG_CRYSTAL_CLK_SEL                                   0x02
// (ReadWrite, Bits 0)
// control signal to choose oe signal 1'b0 oe signal from I2C 1'b1 oeandlocked signal
#define BIT_MSK__MHL_PLL_CTL0__REG_ZONE_MASK_OE                                      0x01

// MHL PLL 2nd Ctl Register
#define REG_ADDR__MHL_PLL_CTL1                                           (REGTX_SERDES | 0x00B8)
// (ReadWrite, Bits 7:4)
// 4 bit open loop fvco control signal
#define BIT_MSK__MHL_PLL_CTL1__REG_FVCO_CTL                                          0xF0
// (ReadWrite, Bits 3:0)
// control PLL BW through I2C
#define BIT_MSK__MHL_PLL_CTL1__REG_PLL_BW_CTL                                        0x0F

// MHL PLL 3rd Ctl Register
#define REG_ADDR__MHL_PLL_CTL2                                           (REGTX_SERDES | 0x00B9)
// (ReadWrite, Bits 7)
// Enable clock detect
#define BIT_MSK__MHL_PLL_CTL2__REG_CLKDETECT_EN                                      0x80
// (ReadWrite, Bits 3)
// measure enable signal for zone control, when enable,  pll is in open loop
#define BIT_MSK__MHL_PLL_CTL2__REG_MEAS_FVCO                                         0x08
// (ReadWrite, Bits 2)
// control bit for PLL fast lock enable
#define BIT_MSK__MHL_PLL_CTL2__REG_PLL_FAST_LOCK                                     0x04
// (ReadWrite, Bits 1:0)
// PLL loop filter R/C select signal
#define BIT_MSK__MHL_PLL_CTL2__REG_PLL_LF_SEL                                        0x03

// MHL BIAS 1st Ctl Register
#define REG_ADDR__MHL_BIAS_CTL0                                          (REGTX_SERDES | 0x00BA)
// (ReadWrite, Bits 7)
// bias select between bgr and resistor divider 1'b0 resistor divider 1'b1 BGR
#define BIT_MSK__MHL_BIAS_CTL0__REG_BIAS_SEL                                          0x80
// (ReadWrite, Bits 6:4)
// bias swing control signal (in harvey it is fixed setting)
#define BIT_MSK__MHL_BIAS_CTL0__REG_BIAS_SW_CTL                                       0x70
// (ReadWrite, Bits 3:0)
// bgr output reference voltage control
#define BIT_MSK__MHL_BIAS_CTL0__REG_BGR_CTL                                           0x0F

// MHL BIAS 2nd Ctl Register
#define REG_ADDR__MHL_BIAS_CTL1                                          (REGTX_SERDES | 0x00BB)
// (ReadWrite, Bits 7:6)
// driver data channel terminal selection signal for channel 2 2b'00 open 2b'01 300 ohm 2b'10 150 ohm 2b'11 100 ohm (default)
#define BIT_MSK__MHL_BIAS_CTL1__REG_DT2_TERM_SEL                                      0xC0
// (ReadWrite, Bits 5:4)
// driver data channel terminal selection signal for channel 1 2b'00 open 2b'01 300 ohm 2b'10 150 ohm 2b'11 100 ohm (default)
#define BIT_MSK__MHL_BIAS_CTL1__REG_DT1_TERM_SEL                                      0x30
// (ReadWrite, Bits 3:0)
// internal resistor control bit
#define BIT_MSK__MHL_BIAS_CTL1__REG_RSWING_CTL                                        0x0F

// MHL BIAS 3rd Ctl Register
#define REG_ADDR__MHL_BIAS_CTL2                                          (REGTX_SERDES | 0x00BC)
// (ReadWrite, Bits 4:3)
// control driver termination selection (reserved,not use?
#define BIT_MSK__MHL_BIAS_CTL2__REG_BIAS_TERM_SEL                                     0x18
// (ReadWrite, Bits 2:0)
// current select between BGR and PTAT (3 10uA bgr, and 3 10uA PTAT, each is 10uA) 3'b000 betaM,3 PTAT 3'b001 2PTAT+1BGR 3'b011 1PTAT+2BGR 3'b111 3 BGR
#define BIT_MSK__MHL_BIAS_CTL2__REG_IBIAS_SEL                                         0x07

// MHL Misc 1st Ctl Register
#define REG_ADDR__MHL_MISC_CTL0                                          (REGTX_SERDES | 0x00BD)
// (ReadWrite, Bits 7:6)
// fine tunning for data term resistor for channel 2
#define BIT_MSK__MHL_MISC_CTL0__REG_DT2_TERM_VGS_CTL                                  0xC0
// (ReadWrite, Bits 5:4)
// fine tunning for data term resistor for channel 1
#define BIT_MSK__MHL_MISC_CTL0__REG_DT1_TERM_VGS_CTL                                  0x30
// (ReadWrite, Bits 2:0)
// test mode control signal 3b'001 clk_bypass enable 3b'110 bgrtest (reserved)
#define BIT_MSK__MHL_MISC_CTL0__REG_TEST_MODE                                         0x07

// MHL Misc 2nd Ctl Register
#define REG_ADDR__MHL_MISC_CTL1                                          (REGTX_SERDES | 0x00BE)
// (ReadWrite, Bits 7:0)
// Spare registers for MHL Tx PHY
#define BIT_MSK__MHL_MISC_CTL1__REG_RSV_B7_B0                                         0xFF

// MHL Misc 3rd Ctl Register
#define REG_ADDR__MHL_MISC_CTL2                                          (REGTX_SERDES | 0x00BF)
// (ReadWrite, Bits 7:0)
// Spare registers for MHL Tx PHY
#define BIT_MSK__MHL_MISC_CTL2__REG_RSV_B15_B8                                        0xFF

// MHL CBUS 1st Ctl Register
#define REG_ADDR__MHL_CBUS_CTL0                                          (REGTX_SERDES | 0x00C0)
// (ReadWrite, Bits 7)
// cbus rground measurement testmode enable
#define BIT_MSK__MHL_CBUS_CTL0__REG_CBUS_RGND_TEST_MODE                               0x80
// (ReadWrite, Bits 5:4)
// Threshold voltage control for vbias        VIH    VIL   vbias 00: 625   426   734 01: 596   447   747 10: 701   478   740 11: 672    506  754
#define BIT_MSK__MHL_CBUS_CTL0__REG_CBUS_RGND_VTH_CTL                                 0x30
// (ReadWrite, Bits 3:2)
// Control Rgnd measurement resistance through I2C
#define BIT_MSK__MHL_CBUS_CTL0__REG_CBUS_RES_TEST_SEL                                 0x0C
// (ReadWrite, Bits 1:0)
// cbus driver strength selection 2'b00 weakest value 2'b01 weak value 2'b10 strong value (default) 2'b11 strongest value
#define BIT_MSK__MHL_CBUS_CTL0__REG_CBUS_DRV_SEL                                      0x03

// MHL CBUS 2nd Ctl Register
#define REG_ADDR__MHL_CBUS_CTL1                                          (REGTX_SERDES | 0x00C1)
// (ReadWrite, Bits 2:0)
// Rgnd resistance calibration (rough value for 1K reference) 000:  888 ohm 100: 1115 ohm 111: 1378 ohm
#define BIT_MSK__MHL_CBUS_CTL1__REG_CBUS_RGND_RES_CTL                                 0x07

// MHL CoC 1st Ctl Register
#define REG_ADDR__MHL_COC_CTL0                                           (REGTX_SERDES | 0x00C2)
// (ReadWrite, Bits 7)
// enbale signal for coc bias
#define BIT_MSK__MHL_COC_CTL0__REG_COC_BIAS_EN                                       0x80
// (ReadWrite, Bits 6:4)
// coc bias control
#define BIT_MSK__MHL_COC_CTL0__REG_COC_BIAS_CTL                                      0x70
// (ReadWrite, Bits 2:0)
// coc termination control
#define BIT_MSK__MHL_COC_CTL0__REG_COC_TERM_CTL                                      0x07

// MHL CoC 2nd Ctl Register
#define REG_ADDR__MHL_COC_CTL1                                           (REGTX_SERDES | 0x00C3)
// (ReadWrite, Bits 7)
// enbale signal for coc block
#define BIT_MSK__MHL_COC_CTL1__REG_COC_EN                                            0x80
// (ReadWrite, Bits 5:0)
// coc driver strength control
#define BIT_MSK__MHL_COC_CTL1__REG_COC_DRV_CTL                                       0x3F

// MHL CoC 3rd Ctl Register
#define REG_ADDR__MHL_COC_CTL2                                           (REGTX_SERDES | 0x00C4)
// (ReadWrite, Bits 7:4)
// CoC gain control
#define BIT_MSK__MHL_COC_CTL2__REG_COC_GAIN_CTL                                      0xF0

// MHL CoC 4th Ctl Register
#define REG_ADDR__MHL_COC_CTL3                                           (REGTX_SERDES | 0x00C5)
// (ReadWrite, Bits 0)
// Enable CoC analog echo canceller
#define BIT_MSK__MHL_COC_CTL3__REG_COC_AECHO_EN                                      0x01

// MHL CoC 5th Ctl Register
#define REG_ADDR__MHL_COC_CTL4                                           (REGTX_SERDES | 0x00C6)
// (ReadWrite, Bits 7:4)
// coc interface timing control signal
#define BIT_MSK__MHL_COC_CTL4__REG_COC_IF_CTL                                        0xF0
// (ReadWrite, Bits 3:0)
// coc driver slew rate control signal
#define BIT_MSK__MHL_COC_CTL4__REG_COC_SLEW_CTL                                      0x0F

// MHL CoC 6th Ctl Register
#define REG_ADDR__MHL_COC_CTL5                                           (REGTX_SERDES | 0x00C7)
// (ReadWrite, Bits 7:0)
// Spare registers for CoC
#define BIT_MSK__MHL_COC_CTL5__REG_COC_RSV_B7_B0                                     0xFF

// MHL CoC 7th Ctl Register
#define REG_ADDR__MHL_COC_CTL6                                           (REGTX_SERDES | 0x00C8)
// (ReadWrite, Bits 7:0)
// Spare registers for CoC
#define BIT_MSK__MHL_COC_CTL6__REG_COC_RSV_B15_B8                                    0xFF

// MHL DoC 1st Ctl Register
#define REG_ADDR__MHL_DOC_CTL0                                           (REGTX_SERDES | 0x00C9)
// (ReadWrite, Bits 7)
// doc data enable signal in source DOC rx part
#define BIT_MSK__MHL_DOC_CTL0__REG_DOC_RXDATA_EN                                     0x80
// (ReadWrite, Bits 5:3)
// Dummy driver source resistance control
#define BIT_MSK__MHL_DOC_CTL0__REG_DOC_DM_TERM                                       0x38
// (ReadWrite, Bits 2:1)
// doc opamp operating or test mode control
#define BIT_MSK__MHL_DOC_CTL0__REG_DOC_OPMODE                                        0x06
// (ReadWrite, Bits 0)
// doc rx bias enable signal
#define BIT_MSK__MHL_DOC_CTL0__REG_DOC_RXBIAS_EN                                     0x01

// MHL DoC 2nd Ctl Register
#define REG_ADDR__MHL_DOC_CTL1                                           (REGTX_SERDES | 0x00CA)
// (ReadWrite, Bits 7:0)
// doc bias level option control
#define BIT_MSK__MHL_DOC_CTL1__REG_DOC_BIAS                                          0xFF

// MHL DoC 3rd Ctl Register
#define REG_ADDR__MHL_DOC_CTL2                                           (REGTX_SERDES | 0x00CB)
// (ReadWrite, Bits 5:4)
// DoC dummy driver termination fine control
#define BIT_MSK__MHL_DOC_CTL2__REG_DOC_DM_TERM_VGS                                   0x30
// (ReadWrite, Bits 3:0)
// Dummy driver fixed current source control by I2C
#define BIT_MSK__MHL_DOC_CTL2__REG_DOC_DM_SWING_I2C                                  0x0F

// MHL DoC 5th Ctl Register
#define REG_ADDR__MHL_DOC_CTL4                                           (REGTX_SERDES | 0x00CD)
// (ReadWrite, Bits 7:0)
// Spare registers for DoC
#define BIT_MSK__MHL_DOC_CTL4__REG_DOC_RSV_B7_B0                                     0xFF

// MHL DoC 6th Ctl Register
#define REG_ADDR__MHL_DOC_CTL5                                           (REGTX_SERDES | 0x00CE)
// (ReadWrite, Bits 7:0)
// Interface control registers for DoC
#define BIT_MSK__MHL_DOC_CTL5__REG_DOC_IF                                            0xFF

// MHL Oscillator 1st Ctl Register
#define REG_ADDR__MHL_OSC_CTL0                                           (REGTX_SERDES | 0x00CF)
// (ReadWrite, Bits 7:5)
// reg18 control
#define BIT_MSK__MHL_OSC_CTL0__REG_REG18_CTL                                         0xE0
// (ReadWrite, Bits 4)
// reg18 enable
#define BIT_MSK__MHL_OSC_CTL0__REG_REG18_EN                                          0x10
// (ReadWrite, Bits 3:0)
// oscillator speed control signal
#define BIT_MSK__MHL_OSC_CTL0__REG_OSC_CTL                                           0x0F

// MHL DataPath 7th Ctl Register
#define REG_ADDR__MHL_DP_CTL6                                            (REGTX_SERDES | 0x00D0)
// (ReadWrite, Bits 5)
// Pre-emphasis tap2 polarity control 1'b0: positive 1'b1: negative
#define BIT_MSK__MHL_DP_CTL6__REG_DP_TAP2_SGN                                       0x20
// (ReadWrite, Bits 4)
// Pre-emphasis tap2 enable signal
#define BIT_MSK__MHL_DP_CTL6__REG_DP_TAP2_EN                                        0x10
// (ReadWrite, Bits 3)
// Pre-emphasis tap1 polarity control 1'b0: positive 1'b1: negative
#define BIT_MSK__MHL_DP_CTL6__REG_DP_TAP1_SGN                                       0x08
// (ReadWrite, Bits 2)
// Pre-emphasis tap1 enable signal
#define BIT_MSK__MHL_DP_CTL6__REG_DP_TAP1_EN                                        0x04
// (ReadWrite, Bits 1)
// Data pre-driver feed-through cap enable signal 1'b0: disable 1'b1: enable
#define BIT_MSK__MHL_DP_CTL6__REG_DT_PREDRV_FEEDCAP_EN                              0x02
// (ReadWrite, Bits 0)
// Pre-emphasis pre/post cursor selection 1'b0: pre-cursor 1'b1: post-cursor
#define BIT_MSK__MHL_DP_CTL6__REG_DP_PRE_POST_SEL                                   0x01

// MHL DataPath 8th Ctl Register
#define REG_ADDR__MHL_DP_CTL7                                            (REGTX_SERDES | 0x00D1)
// (ReadWrite, Bits 7:4)
// Driver cascade bias control. Control for bias stability option
#define BIT_MSK__MHL_DP_CTL7__REG_DT_DRV_VBIAS_CASCTL                               0xF0
// (ReadWrite, Bits 3:0)
// Driver reference current control.
#define BIT_MSK__MHL_DP_CTL7__REG_DT_DRV_IREF_CTL                                   0x0F

// MHL DataPath 9th Ctl Register
#define REG_ADDR__MHL_DP_CTL8                                            (REGTX_SERDES | 0x00D2)
// (ReadWrite, Bits 7:4)
// Driver pre-emphasis tap2 coefficient control
#define BIT_MSK__MHL_DP_CTL8__REG_DT_DRV_TAP2_CTL                                   0xF0
// (ReadWrite, Bits 3:0)
// Driver pre-emphasis tap1 coefficient control
#define BIT_MSK__MHL_DP_CTL8__REG_DT_DRV_TAP1_CTL                                   0x0F

// MHL DataPath 9th Ctl Register
#define REG_ADDR__MHL_DP_CTL9                                            (REGTX_SERDES | 0x00D3)
// (ReadWrite, Bits 7:4)
// Driver reference pre-drive control
#define BIT_MSK__MHL_DP_CTL9__REG_DT_PREDRV_RFT_CTL                                 0xF0
// (ReadWrite, Bits 3:0)
// Low power mode support for data driver swing control Normal and pre-emphasis mode: 4'b1111 low power dongle mode: can be adjust accoding to MHL3 low power spec
#define BIT_MSK__MHL_DP_CTL9__REG_DT_DRV_LOWPWR                                     0x0F

// MHL Channel 3 Ctl Register
#define REG_ADDR__MHL_CH3_CTL                                            (REGTX_SERDES | 0x00D4)
// (ReadWrite, Bits 6)
// driver output enable signal for channel 3
#define BIT_MSK__MHL_CH3_CTL__REG_DP3_OE                                            0x40
// (ReadWrite, Bits 5:4)
// fine tunning for data term resistor for channel 3
#define BIT_MSK__MHL_CH3_CTL__REG_DT3_TERM_VGS_CTL                                  0x30
// (ReadWrite, Bits 3:2)
// driver data channel terminal selection signal for channel 3 2b'00 open 2b'01 300 ohm 2b'10 150 ohm 2b'11 100 ohm
#define BIT_MSK__MHL_CH3_CTL__REG_DT3_TERM_SEL                                      0x0C
// (ReadWrite, Bits 0)
// Channel flip enable 0:  Normal connection 1: flipping connection
#define BIT_MSK__MHL_CH3_CTL__REG_CH_FLIP_EN                                        0x01

// TMDS Control Register
#define REG_ADDR__TX_PHY_TMDS_CTL                                        (REGTX_SERDES | 0x00D5)
// (ReadWrite, Bits 4)
// TMDS output enable control
#define BIT_MSK__TX_PHY_TMDS_CTL__REG_TMDS_OE                                           0x10
// (ReadWrite, Bits 3)
// Enable SCDT to control tx_en for MHL TMDS TX 0: Disable ( to default by #23345) 1: Enable (default to non-default by #23345)
#define BIT_MSK__TX_PHY_TMDS_CTL__REG_TX_EN_BY_SCDT                                     0x08

// PHY Power Down Register
#define REG_ADDR__PHY_PD                                                 (REGTX_SERDES | 0x00D6)
// (ReadWrite, Bits 7)
// Power on TX PLL
#define BIT_MSK__PHY_PD__REG_PWRON_PLL                                         0x80
// (ReadWrite, Bits 6)
// Datapath power switch enable 0  Power Down Datapath.  1  Power On Datapath.
#define BIT_MSK__PHY_PD__REG_PWRON_DP                                          0x40
// (ReadWrite, Bits 4)
// Power Down internal oscillator; effectively disabling Master I2C; Slave I2C; Charge Pump; and preventing uploading of new vectors from EPROM 0 - Power down.   1 - normal operation (default).
#define BIT_MSK__PHY_PD__REG_OSC_EN                                            0x10

//***************************************************************************
// REGTX_HDCP2X. Address: 40
// HDCP General Control 0 Register
#define REG_ADDR__HDCP2X_CTRL_0                                          (REGTX_HDCP2X | 0x0000)
// (ReadWrite, Bits 7:3)
// rsvd
#define BIT_MSK__HDCP2X_CTRL_0__RSVD                                                  0xF8
// (ReadWrite, Bits 2)
// Constant: 1 for HDMI 0 for MHL
#define BIT_MSK__HDCP2X_CTRL_0__RI_HDCP2_HDMIMODE                                     0x04
// (ReadWrite, Bits 1)
// Valid only for Rx. Don't care for Tx. Constant: 1 for Repeater 0 for Receiver
#define BIT_MSK__HDCP2X_CTRL_0__RI_HDCP2RX_REPEATER                                   0x02
// (ReadWrite, Bits 0)
// Constant: 1 for TX 0 for RX
#define BIT_MSK__HDCP2X_CTRL_0__RI_HDCP2_HDCPTX                                       0x01

// HDCP General Control 1 Register
#define REG_ADDR__HDCP2X_CTRL_1                                          (REGTX_HDCP2X | 0x0001)
// (ReadWrite, Bits 7:4)
// {mask_ecc mask_hpd mask_req mask_sw}  1 to mask(off) . 0 to unmask(on)
#define BIT_MSK__HDCP2X_CTRL_1__RI_HDCP2_REAUTH_MSK                                   0xF0
// (ReadWrite, Bits 1)
// Valid only for Tx. Don't care for Rx. 1 to mask auth_done to link (No CTL3) . Default is 0 - unmaked.
#define BIT_MSK__HDCP2X_CTRL_1__RI_HDCP2TX_CTL3MSK                                    0x02
// (ReadWrite, Bits 0)
// Active high long pulse
#define BIT_MSK__HDCP2X_CTRL_1__RI_HDCP2_REAUTH_SW                                    0x01

// HDCP General Control 2 Register
#define REG_ADDR__HDCP2X_CTRL_2                                          (REGTX_HDCP2X | 0x0002)
// (ReadWrite, Bits 7:4)
// 4'hA for HDCP 2.2 for HDMI or MHL. Do not change this regiseter.
#define BIT_MSK__HDCP2X_CTRL_2__RI_HDCP2_CPVER_B3_B0                                  0xF0
// (ReadWrite, Bits 2)
// 1(default): Use i_hw_cupd_start/i_hw_cupd_done. 0: Use ri_hdcp2_cupd_start/ri_hdcp2_cupd_done.
#define BIT_MSK__HDCP2X_CTRL_2__RI_HDCP2_CUPD_HW                                      0x04
// (ReadWrite, Bits 1)
// Active high long pulse Write 1 to this register to gain write access to PRAM for patch.
#define BIT_MSK__HDCP2X_CTRL_2__RI_HDCP2_CUPD_START                                   0x02
// (ReadWrite, Bits 0)
// Active high long pulse Write 1 to this register to start patch code check. Write back to 0 after code check finishes.
#define BIT_MSK__HDCP2X_CTRL_2__RI_HDCP2_CUPD_DONE                                    0x01

// HDCP Interrupt0 Status Register
#define REG_ADDR__HDCP2X_INTR0                                           (REGTX_HDCP2X | 0x0003)
// (ReadWrite, Bits 7)
// ro_hdcp2_auth_stat[7]: polling_interval
#define BIT_MSK__HDCP2X_INTR0__INTR0_STAT7                                           0x80
// (ReadWrite, Bits 6)
// ro_hdcp2_auth_stat[6] : reauth_req
#define BIT_MSK__HDCP2X_INTR0__INTR0_STAT6                                           0x40
// (ReadWrite, Bits 5)
// ro_hdcp2_auth_stat[5] : cchk_fail
#define BIT_MSK__HDCP2X_INTR0__INTR0_STAT5                                           0x20
// (ReadWrite, Bits 4)
// ro_hdcp2_auth_stat[4] : cchk_done
#define BIT_MSK__HDCP2X_INTR0__INTR0_STAT4                                           0x10
// (ReadWrite, Bits 3)
// ro_hdcp2_auth_stat[3] : hash_fail
#define BIT_MSK__HDCP2X_INTR0__INTR0_STAT3                                           0x08
// (ReadWrite, Bits 2)
// ro_hdcp2_auth_stat[2] : rpt_ready
#define BIT_MSK__HDCP2X_INTR0__INTR0_STAT2                                           0x04
// (ReadWrite, Bits 1)
// ro_hdcp2_auth_stat[1] : auth_fail
#define BIT_MSK__HDCP2X_INTR0__INTR0_STAT1                                           0x02
// (ReadWrite, Bits 0)
// ro_hdcp2_auth_stat[0] : auth_done
#define BIT_MSK__HDCP2X_INTR0__INTR0_STAT0                                           0x01

// HDCP Interrupt1 Status Register
#define REG_ADDR__HDCP2X_INTR1                                           (REGTX_HDCP2X | 0x0004)
// (ReadWrite, Bits 7)
// ro_gp3[7]
#define BIT_MSK__HDCP2X_INTR1__INTR1_STAT7                                           0x80
// (ReadWrite, Bits 6)
// ro_cert_sent_rcvd
#define BIT_MSK__HDCP2X_INTR1__INTR1_STAT6                                           0x40
// (ReadWrite, Bits 5)
// ro_rpt_smng_xfer_done
#define BIT_MSK__HDCP2X_INTR1__INTR1_STAT5                                           0x20
// (ReadWrite, Bits 4)
// ro_rpt_rcvid_xfer_done
#define BIT_MSK__HDCP2X_INTR1__INTR1_STAT4                                           0x10
// (ReadWrite, Bits 3)
// ro_ske_sent_rcvd
#define BIT_MSK__HDCP2X_INTR1__INTR1_STAT3                                           0x08
// (ReadWrite, Bits 2)
// ro_ake_sent_rcvd
#define BIT_MSK__HDCP2X_INTR1__INTR1_STAT2                                           0x04
// (ReadWrite, Bits 1)
// ro_rpt_smng_changed
#define BIT_MSK__HDCP2X_INTR1__INTR1_STAT1                                           0x02
// (ReadWrite, Bits 0)
// ro_rpt_rcvid_changed
#define BIT_MSK__HDCP2X_INTR1__INTR1_STAT0                                           0x01

// HDCP Interrupt2 Status Register
#define REG_ADDR__HDCP2X_INTR2                                           (REGTX_HDCP2X | 0x0005)
// (ReadWrite, Bits 7)
// ro_msg_intr[7]:ro_mack_sent_rcvd
#define BIT_MSK__HDCP2X_INTR2__INTR2_STAT7                                           0x80
// (ReadWrite, Bits 6)
// ro_msg_intr[6]:ro_vack_sent_rcvd
#define BIT_MSK__HDCP2X_INTR2__INTR2_STAT6                                           0x40
// (ReadWrite, Bits 5)
// ro_msg_intr[5]:ro_l_sent_rcvd
#define BIT_MSK__HDCP2X_INTR2__INTR2_STAT5                                           0x20
// (ReadWrite, Bits 4)
// ro_msg_intr[4]:ro_lc_sent_rcvd
#define BIT_MSK__HDCP2X_INTR2__INTR2_STAT4                                           0x10
// (ReadWrite, Bits 3)
// ro_msg_intr[3]:ro_pair_sent_rcvd
#define BIT_MSK__HDCP2X_INTR2__INTR2_STAT3                                           0x08
// (ReadWrite, Bits 2)
// ro_msg_intr[2]:ro_h_sent_rcvd
#define BIT_MSK__HDCP2X_INTR2__INTR2_STAT2                                           0x04
// (ReadWrite, Bits 1)
// ro_msg_intr[1]:ro_ekhkm_sent_rcvd
#define BIT_MSK__HDCP2X_INTR2__INTR2_STAT1                                           0x02
// (ReadWrite, Bits 0)
// ro_msg_intr[0]:ro_km_sent_rcvd
#define BIT_MSK__HDCP2X_INTR2__INTR2_STAT0                                           0x01

// HDCP AES Debug Register
#define REG_ADDR__HDCP2X_INTR3                                           (REGTX_HDCP2X | 0x0006)
// (ReadWrite, Bits 1)
// Encryption enable status changed(RX only)
#define BIT_MSK__HDCP2X_INTR3__INTR3_STAT1                                           0x02
// (ReadWrite, Bits 0)
// Underrun in AES FIFO
#define BIT_MSK__HDCP2X_INTR3__INTR3_STAT0                                           0x01

// HDCP Interrupt0 Mask Register
#define REG_ADDR__HDCP2X_INTR0_MASK                                      (REGTX_HDCP2X | 0x0007)
// (ReadWrite, Bits 7)
// Mask for INTR0[7]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR0_MASK__INTR0_MASK_B7                                         0x80
// (ReadWrite, Bits 6)
// Mask for INTR0[6]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR0_MASK__INTR0_MASK_B6                                         0x40
// (ReadWrite, Bits 5)
// Mask for INTR0[5]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR0_MASK__INTR0_MASK_B5                                         0x20
// (ReadWrite, Bits 4)
// Mask for INTR0[4]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR0_MASK__INTR0_MASK_B4                                         0x10
// (ReadWrite, Bits 3)
// Mask for INTR0[3]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR0_MASK__INTR0_MASK_B3                                         0x08
// (ReadWrite, Bits 2)
// Mask for INTR0[2]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR0_MASK__INTR0_MASK_B2                                         0x04
// (ReadWrite, Bits 1)
// Mask for INTR0[1]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR0_MASK__INTR0_MASK_B1                                         0x02
// (ReadWrite, Bits 0)
// Mask for INTR0[0]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR0_MASK__INTR0_MASK_B0                                         0x01

// HDCP Interrupt1 MaskRegister
#define REG_ADDR__HDCP2X_INTR1_MASK                                      (REGTX_HDCP2X | 0x0008)
// (ReadWrite, Bits 7)
// Mask for INTR1[7]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR1_MASK__INTR1_MASK_B7                                         0x80
// (ReadWrite, Bits 6)
// Mask for INTR1[6]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR1_MASK__INTR1_MASK_B6                                         0x40
// (ReadWrite, Bits 5)
// Mask for INTR1[5]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR1_MASK__INTR1_MASK_B5                                         0x20
// (ReadWrite, Bits 4)
// Mask for INTR1[4]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR1_MASK__INTR1_MASK_B4                                         0x10
// (ReadWrite, Bits 3)
// Mask for INTR1[3]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR1_MASK__INTR1_MASK_B3                                         0x08
// (ReadWrite, Bits 2)
// Mask for INTR1[2]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR1_MASK__INTR1_MASK_B2                                         0x04
// (ReadWrite, Bits 1)
// Mask for INTR1[1]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR1_MASK__INTR1_MASK_B1                                         0x02
// (ReadWrite, Bits 0)
// Mask for INTR1[0]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR1_MASK__INTR1_MASK_B0                                         0x01

// HDCP Interrupt2 Mask Register
#define REG_ADDR__HDCP2X_INTR2_MASK                                      (REGTX_HDCP2X | 0x0009)
// (ReadWrite, Bits 7)
// Mask for INTR2[7]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR2_MASK__INTR2_MASK_B7                                         0x80
// (ReadWrite, Bits 6)
// Mask for INTR2[6]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR2_MASK__INTR2_MASK_B6                                         0x40
// (ReadWrite, Bits 5)
// Mask for INTR2[5]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR2_MASK__INTR2_MASK_B5                                         0x20
// (ReadWrite, Bits 4)
// Mask for INTR2[4]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR2_MASK__INTR2_MASK_B4                                         0x10
// (ReadWrite, Bits 3)
// Mask for INTR2[3]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR2_MASK__INTR2_MASK_B3                                         0x08
// (ReadWrite, Bits 2)
// Mask for INTR2[2]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR2_MASK__INTR2_MASK_B2                                         0x04
// (ReadWrite, Bits 1)
// Mask for INTR2[1]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR2_MASK__INTR2_MASK_B1                                         0x02
// (ReadWrite, Bits 0)
// Mask for INTR2[0]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR2_MASK__INTR2_MASK_B0                                         0x01

// HDCP Interrupt3 Mask Register
#define REG_ADDR__HDCP2X_INTR3_MASK                                      (REGTX_HDCP2X | 0x000A)
// (ReadWrite, Bits 1)
// Mask for INTR3[1]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR3_MASK__INTR3_MASK_B1                                         0x02
// (ReadWrite, Bits 0)
// Mask for INTR3[0]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__HDCP2X_INTR3_MASK__INTR3_MASK_B0                                         0x01

// HDCP Interrupt Status Register
#define REG_ADDR__HDCP2X_INTRSTATUS                                      (REGTX_HDCP2X | 0x000B)
// (ReadOnly, Bits 7:4)
// Byte aggregated interrupt stateu Bit 0 for HDCP2X_INTR0 Bit 1 for HDCP2X_INTR1 Bit 2 for HDCP2X_INTR2 Bit 3 for HDCP2X_INTR3
#define BIT_MSK__HDCP2X_INTRSTATUS__RO_AGGRINTR_B3_B0                                     0xF0
// (ReadOnly, Bits 0)
// OR of all HDCP interrupts
#define BIT_MSK__HDCP2X_INTRSTATUS__RO_INTR                                               0x01

// HDCP Auth Status Register
#define REG_ADDR__HDCP2X_AUTH_STAT                                       (REGTX_HDCP2X | 0x000C)
// (ReadOnly, Bits 7:0)
// [0]: auth_done [1]: auth_fail [2]: rpt_ready [3]: hash_fail [4]: cchk_done [5]: cchk_fail [6]: reauth_req [7]: polling_inverval
#define BIT_MSK__HDCP2X_AUTH_STAT__RO_HDCP2_AUTH_STAT_B7_B0                              0xFF

// HDCP State Status Register
#define REG_ADDR__HDCP2X_STATE                                           (REGTX_HDCP2X | 0x000D)
// (ReadOnly, Bits 7:0)
// State for debugging
#define BIT_MSK__HDCP2X_STATE__RO_HDCP2_STATE_B7_B0                                  0xFF

// HDCP General Staus Register
#define REG_ADDR__HDCP2X_GEN_STATUS                                      (REGTX_HDCP2X | 0x000E)
// (ReadOnly, Bits 7:4)
// encryptino enable status for each stream (up to 4) RX only
#define BIT_MSK__HDCP2X_GEN_STATUS__ENC_EN                                                0xF0
// (ReadOnly, Bits 2)
// For TX set to 1 when connected downstream device is an HDCP2 repeater. For RX set to 1 if ri_hdcp2x_repeater(HDCP2X_CMD_CTRL_0[1]) is set and HDCP2X core is running.
#define BIT_MSK__HDCP2X_GEN_STATUS__RO_HDCP2_REPEATER                                     0x04
// (ReadOnly, Bits 1)
// Valid only for Rx. Don't care for Tx. Connected to HDCP2X mode detection signal from i2c. Tuns to 0 if DDC offset address 0 ~ 0x4F are accessed. Turns to 1 if DDC offset address 0x50 and above are accessed. Power on default is 0.
#define BIT_MSK__HDCP2X_GEN_STATUS__RO_HDCP2RX_MODE_SEL                                   0x02
// (ReadOnly, Bits 0)
// Debugging info: 1 if RAM patch code is running 0 if ROM code is running
#define BIT_MSK__HDCP2X_GEN_STATUS__RO_HDCP2_PRG_SEL                                      0x01

// HDCP TP0  Register
#define REG_ADDR__HDCP2X_TP0                                             (REGTX_HDCP2X | 0x0010)
// (ReadWrite, Bits 7:0)
// eclk divider selection (2=divide by 8) 0: divide by 2    1: divide by 4    2: divide by 8   other: divide by 16
#define BIT_MSK__HDCP2X_TP0__RI_HDCP2_TP0_B7_B0                                    0xFF

// HDCP TP1  Register
#define REG_ADDR__HDCP2X_TP1                                             (REGTX_HDCP2X | 0x0011)
// (ReadWrite, Bits 7:0)
// Coutner to generate base timer tick Value for 12ms timer = 12ms * eclk / divider / 256  24MHz eclk divide by 8 -] 140 20MHz eckl divide by 8 -] 117 For a given eclk frequency bigger value means slower tick For a given tick time faster eclk requires bigger value.
#define BIT_MSK__HDCP2X_TP1__RI_HDCP2_TP1_B7_B0                                    0xFF

// HDCP TP2 Register
#define REG_ADDR__HDCP2X_TP2                                             (REGTX_HDCP2X | 0x0012)
// (ReadWrite, Bits 7:0)
// Restart wait time (Default 1=12ms )
#define BIT_MSK__HDCP2X_TP2__RI_HDCP2_TP2_B7_B0                                    0xFF

// HDCP TP3  Register
#define REG_ADDR__HDCP2X_TP3                                             (REGTX_HDCP2X | 0x0013)
// (ReadWrite, Bits 7:0)
// DDC hang timeout  (Default 42=0x2A=504ms)
#define BIT_MSK__HDCP2X_TP3__RI_HDCP2_TP3_B7_B0                                    0xFF

// HDCP TP4  Register
#define REG_ADDR__HDCP2X_TP4                                             (REGTX_HDCP2X | 0x0014)
// (ReadWrite, Bits 7:0)
// H check timeout (storedkm case)  (Default 17=0x11=204ms)
#define BIT_MSK__HDCP2X_TP4__RI_HDCP2_TP4_B7_B0                                    0xFF

// HDCP TP5  Register
#define REG_ADDR__HDCP2X_TP5                                             (REGTX_HDCP2X | 0x0015)
// (ReadWrite, Bits 7:0)
// H check timeout (no-storedkm case)  (Default 100=1.2s)
#define BIT_MSK__HDCP2X_TP5__RI_HDCP2_TP5_B7_B0                                    0xFF

// HDCP TP6 Register
#define REG_ADDR__HDCP2X_TP6                                             (REGTX_HDCP2X | 0x0016)
// (ReadWrite, Bits 7:0)
// Locality check timeout  (Default 2=24ms)
#define BIT_MSK__HDCP2X_TP6__RI_HDCP2_TP6_B7_B0                                    0xFF

// HDCP TP7  Register
#define REG_ADDR__HDCP2X_TP7                                             (REGTX_HDCP2X | 0x0017)
// (ReadWrite, Bits 7:0)
// M check timeout  (Default 9=108ms)
#define BIT_MSK__HDCP2X_TP7__RI_HDCP2_TP7_B7_B0                                    0xFF

// HDCP TP8  Register
#define REG_ADDR__HDCP2X_TP8                                             (REGTX_HDCP2X | 0x0018)
// (ReadWrite, Bits 7:0)
// Certificate read timeout  (Default 9=108ms)
#define BIT_MSK__HDCP2X_TP8__RI_HDCP2_TP8_B7_B0                                    0xFF

// HDCP TP9  Register
#define REG_ADDR__HDCP2X_TP9                                             (REGTX_HDCP2X | 0x0019)
// (ReadWrite, Bits 7:0)
// Paring timeout  (Default 17=0x11=204ms)
#define BIT_MSK__HDCP2X_TP9__RI_HDCP2_TP9_B7_B0                                    0xFF

// HDCP TP10 Register
#define REG_ADDR__HDCP2X_TP10                                            (REGTX_HDCP2X | 0x001A)
// (ReadWrite, Bits 7:0)
// Authdonen wait time  (Default 17=0x11=204ms)
#define BIT_MSK__HDCP2X_TP10__RI_HDCP2_TP10_B7_B0                                   0xFF

// HDCP TP11  Register
#define REG_ADDR__HDCP2X_TP11                                            (REGTX_HDCP2X | 0x001B)
// (ReadWrite, Bits 7:0)
// V check timeout  (Default 167=0xA7=2s)
#define BIT_MSK__HDCP2X_TP11__RI_HDCP2_TP11_B7_B0                                   0xFF

// HDCP TP12  Register
#define REG_ADDR__HDCP2X_TP12                                            (REGTX_HDCP2X | 0x001C)
// (ReadWrite, Bits 7:0)
// Retry wait time  (Default 125 = 0x7D=1.5s)
#define BIT_MSK__HDCP2X_TP12__RI_HDCP2_TP12_B7_B0                                   0xFF

// HDCP TP13  Register
#define REG_ADDR__HDCP2X_TP13                                            (REGTX_HDCP2X | 0x001D)
// (ReadWrite, Bits 7:0)
// Stream_Ready to ENC_EN wait time. (Default 13 = 0x0D = 156ms)
#define BIT_MSK__HDCP2X_TP13__RI_HDCP2_TP13_B7_B0                                   0xFF

// HDCP TP14 Register
#define REG_ADDR__HDCP2X_TP14                                            (REGTX_HDCP2X | 0x001E)
// (ReadWrite, Bits 7:0)
// Wait time for V (Default 250=0xFA=3s)
#define BIT_MSK__HDCP2X_TP14__RI_HDCP2_TP14_B7_B0                                   0xFF

// HDCP TP15  Register
#define REG_ADDR__HDCP2X_TP15                                            (REGTX_HDCP2X | 0x001F)
// (ReadWrite, Bits 7:0)
// L check auto retry limit / 8 (Default 0)
#define BIT_MSK__HDCP2X_TP15__RI_HDCP2_TP15_B7_B0                                   0xFF

// HDCP GP In 0 Register
#define REG_ADDR__HDCP2X_GP_IN0                                          (REGTX_HDCP2X | 0x0020)
// (ReadWrite, Bits 7:0)
// General purpose input 0
#define BIT_MSK__HDCP2X_GP_IN0__RI_HDCP2_GP0_B7_B0                                    0xFF

// HDCP GP In 1 Register
#define REG_ADDR__HDCP2X_GP_IN1                                          (REGTX_HDCP2X | 0x0021)
// (ReadWrite, Bits 7:0)
// General purpose input 1
#define BIT_MSK__HDCP2X_GP_IN1__RI_HDCP2_GP1_B7_B0                                    0xFF

// HDCP GP In 2 Register
#define REG_ADDR__HDCP2X_GP_IN2                                          (REGTX_HDCP2X | 0x0022)
// (ReadWrite, Bits 7:0)
// General purpose input 2
#define BIT_MSK__HDCP2X_GP_IN2__RI_HDCP2_GP2_B7_B0                                    0xFF

// HDCP GP In 3 Register
#define REG_ADDR__HDCP2X_GP_IN3                                          (REGTX_HDCP2X | 0x0023)
// (ReadWrite, Bits 7:0)
// General purpose input 3
#define BIT_MSK__HDCP2X_GP_IN3__RI_HDCP2_GP3_B7_B0                                    0xFF

// HDCP GP Out 0 Register
#define REG_ADDR__HDCP2X_GP_OUT0                                         (REGTX_HDCP2X | 0x0024)
// (ReadOnly, Bits 7:0)
// Version Indicator. 0X: TX ROM                    5X: RX ROM 2X: TX RAM                     1X: RX RAM 8X: TX RAM                     9X: RX RAM 4X: Sydney TX RAM       3X: Sydney RX RAM 6X: RogueES0 TX RAM  7X: RogueES0 RX RAM
#define BIT_MSK__HDCP2X_GP_OUT0__RO_HDCP2_GP0_B7_B0                                    0xFF

// HDCP GP Out 1 Register
#define REG_ADDR__HDCP2X_GP_OUT1                                         (REGTX_HDCP2X | 0x0025)
// (ReadOnly, Bits 7:0)
// General purpose output 1
#define BIT_MSK__HDCP2X_GP_OUT1__RO_HDCP2_GP1_B7_B0                                    0xFF

// HDCP GP Out 2 Register
#define REG_ADDR__HDCP2X_GP_OUT2                                         (REGTX_HDCP2X | 0x0026)
// (ReadOnly, Bits 7:0)
// General purpose output 2
#define BIT_MSK__HDCP2X_GP_OUT2__RO_HDCP2_GP2_B7_B0                                    0xFF

// HDCP GP Out 3 Register
#define REG_ADDR__HDCP2X_GP_OUT3                                         (REGTX_HDCP2X | 0x0027)
// (ReadOnly, Bits 7:0)
// General purpose output 3
#define BIT_MSK__HDCP2X_GP_OUT3__RO_HDCP2_GP3_B7_B0                                    0xFF

// HDCP2 Rx ID_0 from Core Register
#define REG_ADDR__HDCP2X_RX_ID_CORE_0                                    (REGTX_HDCP2X | 0x0028)
// (ReadOnly, Bits 7:0)
// hdcp2 receiver ID from core For RX this 40-bit field is filled as soon as HDCP2x core starts running. For TX this 40-bit field is filled when Certification(including Receiver Device ID) is read from Rx.
#define BIT_MSK__HDCP2X_RX_ID_CORE_0__RO_HDCP2_RCVR_ID_B7_B0                                0xFF

// HDCP2 Rx ID_1 from Core Register
#define REG_ADDR__HDCP2X_RX_ID_CORE_1                                    (REGTX_HDCP2X | 0x0029)
// (ReadOnly, Bits 7:0)
// hdcp2 receiver ID from core
#define BIT_MSK__HDCP2X_RX_ID_CORE_1__RO_HDCP2_RCVR_ID_B15_B8                               0xFF

// HDCP2 Rx ID_2 from Core Register
#define REG_ADDR__HDCP2X_RX_ID_CORE_2                                    (REGTX_HDCP2X | 0x002A)
// (ReadOnly, Bits 7:0)
// hdcp2 receiver ID from core
#define BIT_MSK__HDCP2X_RX_ID_CORE_2__RO_HDCP2_RCVR_ID_B23_B16                              0xFF

// HDCP2 Rx ID_3 from Core Register
#define REG_ADDR__HDCP2X_RX_ID_CORE_3                                    (REGTX_HDCP2X | 0x002B)
// (ReadOnly, Bits 7:0)
// hdcp2 receiver ID from core
#define BIT_MSK__HDCP2X_RX_ID_CORE_3__RO_HDCP2_RCVR_ID_B31_B24                              0xFF

// HDCP2 Rx ID_4 from Core Register
#define REG_ADDR__HDCP2X_RX_ID_CORE_4                                    (REGTX_HDCP2X | 0x002C)
// (ReadOnly, Bits 7:0)
// hdcp2x receiver ID from core
#define BIT_MSK__HDCP2X_RX_ID_CORE_4__RO_HDCP2_RCVR_ID_B39_B32                              0xFF

// HDCP Misc Control 1 Register
#define REG_ADDR__HDCP2X_RPT_DETAIL                                      (REGTX_HDCP2X | 0x002D)
// (ReadWrite, Bits 3)
// For TX use this bit to read MAX_DEVICE_EXCEEDED  field of Receiver ID List message For RX use this bit to program MAX_DEVICE_EXCEEDED field of Receiver ID List message
#define BIT_MSK__HDCP2X_RPT_DETAIL__RI_HDCP2RX_RPT_MX_DEVS_EXC                            0x08
// (ReadWrite, Bits 2)
// For TX use this bit to read MAX_CASCADE_EXCEEDED  field of Receiver ID List message For RX use this bit to program MAX_CASCADE_EXCEEDED field of Receiver ID List message
#define BIT_MSK__HDCP2X_RPT_DETAIL__RI_HDCP2RX_RPT_MX_CASC_EXC                            0x04
// (ReadWrite, Bits 1)
// For TX use this bit to read HDCP20RPT_DSTRM field of Receiver ID List message For RX use this bit to program HDCP20RPT_DSTRM field of Receiver ID List message
#define BIT_MSK__HDCP2X_RPT_DETAIL__RI_HDCP2RX_RPT_HDCP20RPT_DSTRM                        0x02
// (ReadWrite, Bits 0)
// For TX use this bit to read HDCP1DEV_DSTRM field of Receiver ID List message For RX use this bit to program HDCP1DEV_DSTRM field of Receiver ID List message
#define BIT_MSK__HDCP2X_RPT_DETAIL__RI_HDCP2RX_RPT_HDCP1DEV_DSTRM                         0x01

// HDCP RPT SMNG K Register
#define REG_ADDR__HDCP2X_RPT_SMNG_K                                      (REGTX_HDCP2X | 0x002E)
// (ReadWrite, Bits 7:0)
// For TX use this bit to program K  field of Stream Management message For RX use this bit to read K field of Stream Manage message
#define BIT_MSK__HDCP2X_RPT_SMNG_K__RI_HDCP2TX_RPT_SMNG_K_B7_B0                           0xFF

// HDCP Depth Control Register
#define REG_ADDR__HDCP2X_RPT_DEPTH                                       (REGTX_HDCP2X | 0x002F)
// (ReadWrite, Bits 7:0)
// For TX use this bit to read DEPTH  field of Receiver ID List message For RX use this bit to program DEPTH field of Receiver ID List message
#define BIT_MSK__HDCP2X_RPT_DEPTH__RI_HDCP2RX_RPT_DEPTH_B7_B0                            0xFF

// HDCP Devcnt Control Register
#define REG_ADDR__HDCP2X_RPT_DEVCNT                                      (REGTX_HDCP2X | 0x0030)
// (ReadWrite, Bits 7:0)
// For TX use this bit to read DEVICE_COUNT  field of Receiver ID List message For RX use this bit to program DEVICE_COUNT field of Receiver ID List message
#define BIT_MSK__HDCP2X_RPT_DEVCNT__RI_HDCP2RX_RPT_DEVCNT_B7_B0                           0xFF

// HDCP RPT SEQ NUM V 0 Register
#define REG_ADDR__HDCP2X_RX_SEQ_NUM_V_0                                  (REGTX_HDCP2X | 0x0031)
// (ReadWrite, Bits 7:0)
// For TX use this bit to read seq_num_V  field of Receiver ID List message For RX use this bit to program seq_num_V field of Receiver ID List message
#define BIT_MSK__HDCP2X_RX_SEQ_NUM_V_0__RI_HDCP2RX_RPT_SEQ_NUM_V_B7_B0                        0xFF

// HDCP RPT SEQ NUM V 1 Register
#define REG_ADDR__HDCP2X_RX_SEQ_NUM_V_1                                  (REGTX_HDCP2X | 0x0032)
// (ReadWrite, Bits 7:0)
// For TX use this bit to read seq_num_V  field of Receiver ID List message For RX use this bit to program seq_num_V field of Receiver ID List message
#define BIT_MSK__HDCP2X_RX_SEQ_NUM_V_1__RI_HDCP2RX_RPT_SEQ_NUM_V_B15_B8                       0xFF

// HDCP RPT SEQ NUM V 2 Register
#define REG_ADDR__HDCP2X_RX_SEQ_NUM_V_2                                  (REGTX_HDCP2X | 0x0033)
// (ReadWrite, Bits 7:0)
// For TX use this bit to read seq_num_V  field of Receiver ID List message For RX use this bit to program seq_num_V field of Receiver ID List message
#define BIT_MSK__HDCP2X_RX_SEQ_NUM_V_2__RI_HDCP2RX_RPT_SEQ_NUM_V_B23_B16                      0xFF

// HDCP RPT SEQ NUM M 0 Register
#define REG_ADDR__HDCP2X_RX_SEQ_NUM_M_0                                  (REGTX_HDCP2X | 0x0034)
// (ReadWrite, Bits 7:0)
// For TX use this bit to program seq_num_M  field of Stream Management message For RX use this bit to read seq_num_M  field of Stream Manage message
#define BIT_MSK__HDCP2X_RX_SEQ_NUM_M_0__RI_HDCP2TX_RPT_SEQ_NUM_M_B7_B0                        0xFF

// HDCP RPT SEQ NUM M 1 Register
#define REG_ADDR__HDCP2X_RX_SEQ_NUM_M_1                                  (REGTX_HDCP2X | 0x0035)
// (ReadWrite, Bits 7:0)
// For TX use this bit to program seq_num_M  field of Stream Management message For RX use this bit to read seq_num_M  field of Stream Manage message
#define BIT_MSK__HDCP2X_RX_SEQ_NUM_M_1__RI_HDCP2TX_RPT_SEQ_NUM_M_B15_B8                       0xFF

// HDCP RPT SEQ NUM M 2 Register
#define REG_ADDR__HDCP2X_RX_SEQ_NUM_M_2                                  (REGTX_HDCP2X | 0x0036)
// (ReadWrite, Bits 7:0)
// For TX use this bit to program seq_num_M  field of Stream Management message For RX use this bit to read seq_num_M  field of Stream Manage message
#define BIT_MSK__HDCP2X_RX_SEQ_NUM_M_2__RI_HDCP2TX_RPT_SEQ_NUM_M_B23_B16                      0xFF

// HDCP Input Counter 0 Register
#define REG_ADDR__HDCP2X_IPT_CTR_7TO0                                    (REGTX_HDCP2X | 0x0037)
// (ReadOnly, Bits 7:0)
// Frame counter (input counter[33:26]) to monitor if input coutner value changes.
#define BIT_MSK__HDCP2X_IPT_CTR_7TO0__RO_HDCP2_IPT_CTR_B7_B0                                0xFF

// HDCP Input Counter 1 Register
#define REG_ADDR__HDCP2X_IPT_CTR_15TO8                                   (REGTX_HDCP2X | 0x0038)
// (ReadOnly, Bits 7:0)
// Frame counter (input counter[41:34]) to monitor if input coutner value changes.
#define BIT_MSK__HDCP2X_IPT_CTR_15TO8__RO_HDCP2_IPT_CTR_B15_B8                               0xFF

// HDCP AES Control Register
#define REG_ADDR__HDCP2X_AESCTL                                          (REGTX_HDCP2X | 0x0039)
// (ReadWrite, Bits 1)
// Apply AES reset when authdone=0
#define BIT_MSK__HDCP2X_AESCTL__RI_AES_RST_AUTHDONE                                   0x02
// (ReadWrite, Bits 0)
// Apply manual AES reset
#define BIT_MSK__HDCP2X_AESCTL__RI_AES_RST_MAN                                        0x01

// HDCP Debug Control Register
#define REG_ADDR__HDCP2X_DBGCTL                                          (REGTX_HDCP2X | 0x003A)
// (ReadWrite, Bits 7:0)
// Debug control
#define BIT_MSK__HDCP2X_DBGCTL__RI_HDCP2_DBG_CTL_B7_B0                                0xFF

// HDCP Debug Control 2 Register
#define REG_ADDR__HDCP2X_DBGCTL2                                         (REGTX_HDCP2X | 0x003B)
// (ReadWrite, Bits 7:0)
// Debug control 2
#define BIT_MSK__HDCP2X_DBGCTL2__RI_HDCP2_DBG_CTL2_B7_B0                               0xFF

// HDCP Misc Control 0 Register
#define REG_ADDR__HDCP2X_RX_CTRL_0                                       (REGTX_HDCP2X | 0x0040)
// (ReadWrite, Bits 7)
// Valid only for Rx. Don't care for Tx. 0: Clear msg_sz when msg_send_done is asserted. 1: Clear msg_sz under internal 8051 control.
#define BIT_MSK__HDCP2X_RX_CTRL_0__RI_MSG_SZ_CLR_OPTION                                  0x80
// (ReadWrite, Bits 6)
// Valid only for Rx. Don't care for Tx. 0: Clear rpt_ready when RX_STATUS register is read out. 1: Clear rpt_ready under internal 8051 control.
#define BIT_MSK__HDCP2X_RX_CTRL_0__RI_RPT_READY_CLR_OPTION                               0x40
// (ReadWrite, Bits 5)
// Valid only for Rx. Don't care for Tx. 0: Clear reauth_req when RX_STATUS register is read out. 1: Clear reauth_req under internal 8051 control.
#define BIT_MSK__HDCP2X_RX_CTRL_0__RI_REAUTH_REQ_CLR_OPTION                              0x20
// (ReadWrite, Bits 4)
// Valid only for Rx. Don't care for Tx. Write 1 to reset Stream Manage message buffer pointer to 0.
#define BIT_MSK__HDCP2X_RX_CTRL_0__RI_HDCP2RX_RPT_SMNG_RD_START                          0x10
// (ReadWrite, Bits 3)
// Valid only for Rx. Don't care for Tx. Read from Stream Manage message buffer.
#define BIT_MSK__HDCP2X_RX_CTRL_0__RI_HDCP2RX_RPT_SMNG_RD                                0x08
// (ReadWrite, Bits 2)
// Valid only for Rx. Don't care for Tx. Write 1 to start transfer of Receiver ID List message.
#define BIT_MSK__HDCP2X_RX_CTRL_0__RI_HDCP2RX_RPT_RCVID_XFER_START                       0x04
// (ReadWrite, Bits 1)
// Valid only for Rx. Don't care for Tx. Write 1 to reset Receiver ID List message buffer pointer to 0.
#define BIT_MSK__HDCP2X_RX_CTRL_0__RI_HDCP2RX_RPT_RCVID_WR_START                         0x02
// (ReadWrite, Bits 0)
// Valid only for Rx. Don't care for Tx. Write to Receiver ID List message buffer.
#define BIT_MSK__HDCP2X_RX_CTRL_0__RI_HDCP2RX_RPT_RCVID_WR                               0x01

// HDCP Misc Staus Register
#define REG_ADDR__HDCP2X_RX_STATUS                                       (REGTX_HDCP2X | 0x0041)
// (ReadOnly, Bits 2)
// Valid only for Rx. Don't care for Tx. Indicate that Receiver ID List message transfer is done. (Also connected to INTR1[4])
#define BIT_MSK__HDCP2X_RX_STATUS__RO_HDCP2RX_RPT_RCVID_XFER_DONE                        0x04

// HDCP RPT SMNG Out Register
#define REG_ADDR__HDCP2X_RX_RPT_SMNG_OUT                                 (REGTX_HDCP2X | 0x0042)
// (ReadOnly, Bits 7:0)
// Valid only for Rx. Don't care for Tx. Data output port for Stream Manage message buffer.
#define BIT_MSK__HDCP2X_RX_RPT_SMNG_OUT__RO_HDCP2RX_RPT_SMNG_OUT_B7_B0                         0xFF

// HDCP RPT RCVID In Register
#define REG_ADDR__HDCP2X_RX_RPT_RCVID_IN                                 (REGTX_HDCP2X | 0x0043)
// (ReadWrite, Bits 7:0)
// Valid only for Rx. Don't care for Tx. Write one byte to Receiver ID List message buffer.
#define BIT_MSK__HDCP2X_RX_RPT_RCVID_IN__RI_HDCP2RX_RPT_RCVID_IN                               0xFF

// HDCP ECC Control Register
#define REG_ADDR__HDCP2X_RX_ECC_CTRL                                     (REGTX_HDCP2X | 0x0044)
// (ReadWrite, Bits 3)
// Write 1 then 0 to clear counter manually.
#define BIT_MSK__HDCP2X_RX_ECC_CTRL__RI_ACCM_ERR_MANU_CLR                                  0x08
// (ReadWrite, Bits 2:1)
// 00: accumulates ECC errors until it reaches a given threshold. 01: In a given number of consecutive frames in which ECC errors keep reaching the threshold 10: In a given number of consecutive frames we don't get any correct ECC 11: Accumulates ECC errors for a given number of frames
#define BIT_MSK__HDCP2X_RX_ECC_CTRL__RI_ECC_CHK_MODE                                       0x06
// (ReadWrite, Bits 0)
// Enable ECC based out-of-sync detection
#define BIT_MSK__HDCP2X_RX_ECC_CTRL__RI_ECC_CHK_EN                                         0x01

// HDCP ECC Count for Check 0 Register
#define REG_ADDR__HDCP2X_RX_ECC_CNT2CHK_0                                (REGTX_HDCP2X | 0x0045)
// (ReadWrite, Bits 7:0)
// VSYNC count to skip before starting ECC check
#define BIT_MSK__HDCP2X_RX_ECC_CNT2CHK_0__RI_CNT2CHK_ECC_B7_B0                                  0xFF

// HDCP ECC Count for Check 1 Register
#define REG_ADDR__HDCP2X_RX_ECC_CNT2CHK_1                                (REGTX_HDCP2X | 0x0046)
// (ReadWrite, Bits 0)
// VSYNC count to skip before starting ECC check
#define BIT_MSK__HDCP2X_RX_ECC_CNT2CHK_1__RI_CNT2CHK_ECC_B8                                     0x01

// HDCP ECC ACCM Error Threshold 0 Register
#define REG_ADDR__HDCP2X_RX_ECC_ACCM_ERR_THR_0                           (REGTX_HDCP2X | 0x0047)
// (ReadWrite, Bits 7:0)
// Threadshold for accumulated ECC error
#define BIT_MSK__HDCP2X_RX_ECC_ACCM_ERR_THR_0__RI_ACCM_ERR_THR_B7_B0                                 0xFF

// HDCP ECC ACCM Error Threshold 1 Register
#define REG_ADDR__HDCP2X_RX_ECC_ACCM_ERR_THR_1                           (REGTX_HDCP2X | 0x0048)
// (ReadWrite, Bits 7:0)
// Threadshold for accumulated ECC error
#define BIT_MSK__HDCP2X_RX_ECC_ACCM_ERR_THR_1__RI_ACCM_ERR_THR_B15_B8                                0xFF

// HDCP ECC ACCM Error Threshold 2 Register
#define REG_ADDR__HDCP2X_RX_ECC_ACCM_ERR_THR_2                           (REGTX_HDCP2X | 0x0049)
// (ReadWrite, Bits 4:0)
// Threadshold for accumulated ECC error
#define BIT_MSK__HDCP2X_RX_ECC_ACCM_ERR_THR_2__RI_ACCM_ERR_THR_B20_B16                               0x1F

// HDCP ECC Frame Error Threshold 0 Register
#define REG_ADDR__HDCP2X_RX_ECC_FRM_ERR_THR_0                            (REGTX_HDCP2X | 0x004A)
// (ReadWrite, Bits 7:0)
// Threadshold for frame ECC error
#define BIT_MSK__HDCP2X_RX_ECC_FRM_ERR_THR_0__RI_FRAME_ECC_ERR_THR_B7_B0                            0xFF

// HDCP ECC Frame Error Threshold 1 Register
#define REG_ADDR__HDCP2X_RX_ECC_FRM_ERR_THR_1                            (REGTX_HDCP2X | 0x004B)
// (ReadWrite, Bits 7:0)
// Threadshold for frame ECC error
#define BIT_MSK__HDCP2X_RX_ECC_FRM_ERR_THR_1__RI_FRAME_ECC_ERR_THR_B15_B8                           0xFF

// HDCP ECC Consecutive Frames Error Threshold Register
#define REG_ADDR__HDCP2X_RX_CONS_ERR_THR                                 (REGTX_HDCP2X | 0x004C)
// (ReadWrite, Bits 7:0)
// Number of consecutive frames in which ECC error get threshold
#define BIT_MSK__HDCP2X_RX_CONS_ERR_THR__RI_CONS_ECC_ERR_THR_B7_B0                             0xFF

// HDCP ECC No Error Threshold Register
#define REG_ADDR__HDCP2X_RX_ECC_NO_ERR_THR                               (REGTX_HDCP2X | 0x004D)
// (ReadWrite, Bits 7:0)
// Number of consecutive frames that does not get correct ECC
#define BIT_MSK__HDCP2X_RX_ECC_NO_ERR_THR__RI_NO_ECC_THR_B7_B0                                   0xFF

// HDCP ECC Given Frame Error Register
#define REG_ADDR__HDCP2X_RX_GVN_FRM                                      (REGTX_HDCP2X | 0x004E)
// (ReadWrite, Bits 7:0)
// Number of consecutive frames in which accumulate ECC error
#define BIT_MSK__HDCP2X_RX_GVN_FRM__RI_GIVEN_FRAME_B7_B0                                  0xFF

// HDCP ECC Given Frame Error Threshold 0 Register
#define REG_ADDR__HDCP2X_RX_ECC_GVN_FRM_ERR_THR_0                        (REGTX_HDCP2X | 0x004F)
// (ReadWrite, Bits 7:0)
// Threshold for the number of ECC errors in given frames
#define BIT_MSK__HDCP2X_RX_ECC_GVN_FRM_ERR_THR_0__RI_GIVEN_FRAME_ERR_THR_B7_B0                          0xFF

// HDCP ECC Given Frame Error Threshold 1 Register
#define REG_ADDR__HDCP2X_RX_ECC_GVN_FRM_ERR_THR_1                        (REGTX_HDCP2X | 0x0050)
// (ReadWrite, Bits 7:0)
// Threshold for the number of ECC errors in given frames
#define BIT_MSK__HDCP2X_RX_ECC_GVN_FRM_ERR_THR_1__RI_GIVEN_FRAME_ERR_THR_B15_B8                         0xFF

// HDCP ECC Given Frame Error Threshold 2 Register
#define REG_ADDR__HDCP2X_RX_ECC_GVN_FRM_ERR_THR_2                        (REGTX_HDCP2X | 0x0051)
// (ReadWrite, Bits 4:0)
// Threshold for the number of ECC errors in given frames
#define BIT_MSK__HDCP2X_RX_ECC_GVN_FRM_ERR_THR_2__RI_GIVEN_FRAME_ERR_THR_B20_B16                        0x1F

// HDCP Misc Control 0 Register
#define REG_ADDR__HDCP2X_TX_CTRL_0                                       (REGTX_HDCP2X | 0x0070)
// (ReadWrite, Bits 4)
// Valid only for Tx. Don't care for Rx. Write 1 to reset Stream Manage message buffer pointer to 0.
#define BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_WR_START                          0x10
// (ReadWrite, Bits 3)
// Valid only for Tx. Don't care for Rx. Write to Stream Manage message buffer.
#define BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_WR                                0x08
// (ReadWrite, Bits 2)
// Valid only for Tx. Don't care for Rx. Write 1 to start transfer of Stream Manage message.
#define BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_XFER_START                        0x04
// (ReadWrite, Bits 1)
// Valid only for Tx. Don't care for Rx. Write 1 to reset Receiver ID List message buffer pointer to 0.
#define BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_RCVID_RD_START                         0x02
// (ReadWrite, Bits 0)
// Valid only for Tx. Don't care for Rx. Read from Receiver ID List message buffer.
#define BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_RCVID_RD                               0x01

// HDCP Misc Staus Register
#define REG_ADDR__HDCP2X_TX_STATUS                                       (REGTX_HDCP2X | 0x0071)
// (ReadOnly, Bits 2)
// Valid only for Tx. Don't care for Rx. Indicate that Stream Manage message transfer is done. (Also connected to INTR1[5])
#define BIT_MSK__HDCP2X_TX_STATUS__RO_HDCP2TX_RPT_SMNG_XFER_DONE                         0x04

// HDCP RPT SMNG In Register
#define REG_ADDR__HDCP2X_TX_RPT_SMNG_IN                                  (REGTX_HDCP2X | 0x0072)
// (ReadWrite, Bits 7:0)
// Valid only for Tx. Don't care for Rx. Write one byte to Stream Manage message buffer.
#define BIT_MSK__HDCP2X_TX_RPT_SMNG_IN__RI_HDCP2TX_RPT_SMNG_IN                                0xFF

// HDCP RPT RCVID Out Register
#define REG_ADDR__HDCP2X_TX_RPT_RCVID_OUT                                (REGTX_HDCP2X | 0x0073)
// (ReadOnly, Bits 7:0)
// Valid only for Tx. Don't care for Rx. Data output port for Receiver ID List message buffer.
#define BIT_MSK__HDCP2X_TX_RPT_RCVID_OUT__RO_HDCP2TX_RPT_RCVID_OUT_B7_B0                        0xFF

// HDCP Stream Count 0a Register
#define REG_ADDR__HDCP2X_STM_CTR_0A                                      (REGTX_HDCP2X | 0x0080)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_0A__RI_HDCP2TX_STM_CTR_B7_B0                              0xFF

// HDCP Stream Count 0b Register
#define REG_ADDR__HDCP2X_STM_CTR_0B                                      (REGTX_HDCP2X | 0x0081)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_0B__RI_HDCP2TX_STM_CTR_B15_B8                             0xFF

// HDCP Stream Count 0c Register
#define REG_ADDR__HDCP2X_STM_CTR_0C                                      (REGTX_HDCP2X | 0x0082)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_0C__RI_HDCP2TX_STM_CTR_B23_B16                            0xFF

// HDCP Stream Count 0d Register
#define REG_ADDR__HDCP2X_STM_CTR_0D                                      (REGTX_HDCP2X | 0x0083)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_0D__RI_HDCP2TX_STM_CTR_B31_B24                            0xFF

// HDCP Stream Count 1a Register
#define REG_ADDR__HDCP2X_STM_CTR_1A                                      (REGTX_HDCP2X | 0x0084)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_1A__RI_HDCP2TX_STM_CTR_B39_B32                            0xFF

// HDCP Stream Count 1b Register
#define REG_ADDR__HDCP2X_STM_CTR_1B                                      (REGTX_HDCP2X | 0x0085)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_1B__RI_HDCP2TX_STM_CTR_B47_B40                            0xFF

// HDCP Stream Count 1c Register
#define REG_ADDR__HDCP2X_STM_CTR_1C                                      (REGTX_HDCP2X | 0x0086)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_1C__RI_HDCP2TX_STM_CTR_B55_B48                            0xFF

// HDCP Stream Count 1d Register
#define REG_ADDR__HDCP2X_STM_CTR_1D                                      (REGTX_HDCP2X | 0x0087)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_1D__RI_HDCP2TX_STM_CTR_B63_B56                            0xFF

// HDCP Stream Count 2a Register
#define REG_ADDR__HDCP2X_STM_CTR_2A                                      (REGTX_HDCP2X | 0x0088)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_2A__RI_HDCP2TX_STM_CTR_B71_B64                            0xFF

// HDCP Stream Count 2b Register
#define REG_ADDR__HDCP2X_STM_CTR_2B                                      (REGTX_HDCP2X | 0x0089)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_2B__RI_HDCP2TX_STM_CTR_B79_B72                            0xFF

// HDCP Stream Count 2c Register
#define REG_ADDR__HDCP2X_STM_CTR_2C                                      (REGTX_HDCP2X | 0x008A)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_2C__RI_HDCP2TX_STM_CTR_B87_B80                            0xFF

// HDCP Stream Count 2d Register
#define REG_ADDR__HDCP2X_STM_CTR_2D                                      (REGTX_HDCP2X | 0x008B)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_2D__RI_HDCP2TX_STM_CTR_B95_B88                            0xFF

// HDCP Stream Count 3a Register
#define REG_ADDR__HDCP2X_STM_CTR_3A                                      (REGTX_HDCP2X | 0x008C)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_3A__RI_HDCP2TX_STM_CTR_B103_B96                           0xFF

// HDCP Stream Count 3b Register
#define REG_ADDR__HDCP2X_STM_CTR_3B                                      (REGTX_HDCP2X | 0x008D)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_3B__RI_HDCP2TX_STM_CTR_B111_B104                          0xFF

// HDCP Stream Count 3c Register
#define REG_ADDR__HDCP2X_STM_CTR_3C                                      (REGTX_HDCP2X | 0x008E)
// (ReadWrite, Bits 7:0)
// Only for MHL TX
#define BIT_MSK__HDCP2X_STM_CTR_3C__RI_HDCP2TX_STM_CTR_B119_B112                          0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_CTRL0                                     (REGTX_HDCP2X | 0x00A0)
// (ReadWrite, Bits 7:0)
// Debug Control 0 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_CTRL0__REG_HDCP2X_DEBUG_CTRL0                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_CTRL1                                     (REGTX_HDCP2X | 0x00A1)
// (ReadWrite, Bits 7:0)
// Debug Control 1 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_CTRL1__REG_HDCP2X_DEBUG_CTRL1                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_CTRL2                                     (REGTX_HDCP2X | 0x00A2)
// (ReadWrite, Bits 7:0)
// Debug Control 2 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_CTRL2__REG_HDCP2X_DEBUG_CTRL2                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_CTRL3                                     (REGTX_HDCP2X | 0x00A3)
// (ReadWrite, Bits 7:0)
// Debug Control 3 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_CTRL3__REG_HDCP2X_DEBUG_CTRL3                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_CTRL4                                     (REGTX_HDCP2X | 0x00A4)
// (ReadWrite, Bits 7:0)
// Debug Control 4 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_CTRL4__REG_HDCP2X_DEBUG_CTRL4                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT0                                     (REGTX_HDCP2X | 0x00A5)
// (ReadOnly, Bits 7:0)
// Debug Status 0 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT0__REG_HDCP2X_DEBUG_STAT0                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT1                                     (REGTX_HDCP2X | 0x00A6)
// (ReadOnly, Bits 7:0)
// Debug Status 1 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT1__REG_HDCP2X_DEBUG_STAT1                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT2                                     (REGTX_HDCP2X | 0x00A7)
// (ReadOnly, Bits 7:0)
// Debug Status 2 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT2__REG_HDCP2X_DEBUG_STAT2                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT3                                     (REGTX_HDCP2X | 0x00A8)
// (ReadOnly, Bits 7:0)
// Debug Status 3 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT3__REG_HDCP2X_DEBUG_STAT3                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT4                                     (REGTX_HDCP2X | 0x00A9)
// (ReadOnly, Bits 7:0)
// Debug Status 4 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT4__REG_HDCP2X_DEBUG_STAT4                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT5                                     (REGTX_HDCP2X | 0x00AA)
// (ReadOnly, Bits 7:0)
// Debug Status 5 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT5__REG_HDCP2X_DEBUG_STAT5                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT6                                     (REGTX_HDCP2X | 0x00AB)
// (ReadOnly, Bits 7:0)
// Debug Status 6 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT6__REG_HDCP2X_DEBUG_STAT6                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT7                                     (REGTX_HDCP2X | 0x00AC)
// (ReadOnly, Bits 7:0)
// Debug Status 7 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT7__REG_HDCP2X_DEBUG_STAT7                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT8                                     (REGTX_HDCP2X | 0x00AD)
// (ReadOnly, Bits 7:0)
// Debug Status 8 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT8__REG_HDCP2X_DEBUG_STAT8                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT9                                     (REGTX_HDCP2X | 0x00AE)
// (ReadOnly, Bits 7:0)
// Debug Status 9 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT9__REG_HDCP2X_DEBUG_STAT9                                0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT10                                    (REGTX_HDCP2X | 0x00AF)
// (ReadOnly, Bits 7:0)
// Debug Status 10 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT10__REG_HDCP2X_DEBUG_STAT10                               0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT11                                    (REGTX_HDCP2X | 0x00B0)
// (ReadOnly, Bits 7:0)
// Debug Status 11 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT11__REG_HDCP2X_DEBUG_STAT11                               0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT12                                    (REGTX_HDCP2X | 0x00B1)
// (ReadOnly, Bits 7:0)
// Debug Status 12 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT12__REG_HDCP2X_DEBUG_STAT12                               0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT13                                    (REGTX_HDCP2X | 0x00B2)
// (ReadOnly, Bits 7:0)
// Debug Status 13 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT13__REG_HDCP2X_DEBUG_STAT13                               0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT14                                    (REGTX_HDCP2X | 0x00B3)
// (ReadOnly, Bits 7:0)
// Debug Status 14 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT14__REG_HDCP2X_DEBUG_STAT14                               0xFF

// HDCP Temp Register
#define REG_ADDR__HDCP2X_DEBUG_STAT15                                    (REGTX_HDCP2X | 0x00B4)
// (ReadOnly, Bits 7:0)
// Debug Status 15 for HDCP 2.x
#define BIT_MSK__HDCP2X_DEBUG_STAT15__REG_HDCP2X_DEBUG_STAT15                               0xFF

// HDCP Software Reset Register
#define REG_ADDR__HDCP2X_TX_SRST                                         (REGTX_HDCP2X | 0x00B5)
// (ReadWrite, Bits 5)
// Software Reset for hdcp2x logic only 1 - Reset 0 - Normal operation (default).
#define BIT_MSK__HDCP2X_TX_SRST__REG_HDCP2X_SW_RST                                     0x20
// (ReadWrite, Bits 3)
// HDCP Soft Reset for sclock domain
#define BIT_MSK__HDCP2X_TX_SRST__REG_HDCP2X_SRST                                       0x08
// (ReadWrite, Bits 2)
// HDCP Soft Reset for eclock domain
#define BIT_MSK__HDCP2X_TX_SRST__REG_HDCP2X_ERST                                       0x04
// (ReadWrite, Bits 1)
// HDCP Soft Reset for pixel clock domain
#define BIT_MSK__HDCP2X_TX_SRST__REG_HDCP2X_PRST                                       0x02
// (ReadWrite, Bits 0)
// HDCP Soft Reset for crystal clock domain
#define BIT_MSK__HDCP2X_TX_SRST__REG_HDCP2X_CRST                                       0x01

// HDCP Polling Control and Status Register
#define REG_ADDR__HDCP2X_POLL_CS                                         (REGTX_HDCP2X | 0x00B6)
// (ReadWrite, Bits 6)
// Message size clear option. Not used in Cypress.  Can be used as spare register
#define BIT_MSK__HDCP2X_POLL_CS__REG_HDCP2X_MSG_SZ_CLR_OPTION                          0x40
// (ReadWrite, Bits 5)
// Repeater ready status clear option. Not used in Cypress.  Can be used as spare register
#define BIT_MSK__HDCP2X_POLL_CS__REG_HDCP2X_RPT_READY_CLR_OPTION                       0x20
// (ReadWrite, Bits 4)
// Reauthentication request clear option. Not used in Cypress.  Can be used as spare register
#define BIT_MSK__HDCP2X_POLL_CS__REG_HDCP2X_REAUTH_REQ_CLR_OPTION                      0x10
// (ReadOnly, Bits 1)
// A status to indicate that HDCP2.2 polling logic has the DDC bus
#define BIT_MSK__HDCP2X_POLL_CS__REG_HDCP2X_DIS_POLL_GNT                               0x02
// (ReadWrite, Bits 0)
// 1: Disable polling 0: Enable polling (default)
#define BIT_MSK__HDCP2X_POLL_CS__REG_HDCP2X_DIS_POLL_EN                                0x01

// HDCP PRAM Starting Address Lo Register
#define REG_ADDR__HDCP2X_CUPD_START_ADDR_LO                              (REGTX_HDCP2X | 0x00B7)
// (ReadWrite, Bits 7:0)
// Lower byte starting address for PRAM access This is the starting address for PRAM access. Program this before reg_hdcp2x_cupd_start or reg_hdcp2x_cupd_addr_reset
#define BIT_MSK__HDCP2X_CUPD_START_ADDR_LO__REG_HDCP2X_CUPD_S_ADDR_B7_B0                          0xFF

// HDCP PRAM Starting Address Hi Register
#define REG_ADDR__HDCP2X_CUPD_START_ADDR_HI                              (REGTX_HDCP2X | 0x00B8)
// (ReadWrite, Bits 7:0)
// Higher byte starting address for PRAM access This is the starting address for PRAM access. Program this before reg_hdcp2x_cupd_start or reg_hdcp2x_cupd_addr_reset
#define BIT_MSK__HDCP2X_CUPD_START_ADDR_MT__REG_HDCP2X_CUPD_S_ADDR_B15_B8                         0xFF

// HDCP PRAM Signature Starting Address Lo Register
#define REG_ADDR__HDCP2X_CUPD_SIGN_START_ADDR_LO                         (REGTX_HDCP2X | 0x00B9)
// (ReadWrite, Bits 7:0)
// Lower byte starting address for signature portion of PRAM access This is the starting address for signature portion for PRAM access. Program this before reg_hdcp2x_cupd_start or reg_hdcp2x_cupd_addr_reset
#define BIT_MSK__HDCP2X_CUPD_SIGN_START_ADDR_LO__REG_HDCP2X_CUPD_SI_S_ADDR_B7_B0                       0xFF

// HDCP PRAM Signature Starting Address Hi Register
#define REG_ADDR__HDCP2X_CUPD_SIGN_START_ADDR_HI                         (REGTX_HDCP2X | 0x00BA)
// (ReadWrite, Bits 7:0)
// Higher byte starting address for signature portion of PRAM access This is the starting address for signature portion for PRAM access. Program this before reg_hdcp2x_cupd_start or reg_hdcp2x_cupd_addr_reset
#define BIT_MSK__HDCP2X_CUPD_SIGN_START_ADDR_MT__REG_HDCP2X_CUPD_SI_S_ADDR_B15_B8                      0xFF

// HDCP PRAM Signature Ending Address Lo Register
#define REG_ADDR__HDCP2X_PRAM_SIGN_END_ADDR_LO                           (REGTX_HDCP2X | 0x00BB)
// (ReadWrite, Bits 7:0)
// Lower byte ending address for signature portion of PRAM access This is the starting address for signature portion for PRAM access. Program this before reg_hdcp2x_cupd_start or reg_hdcp2x_cupd_addr_reset
#define BIT_MSK__HDCP2X_PRAM_SIGN_END_ADDR_LO__REG_HDCP2X_CUPD_SI_E_ADDR_B7_B0                       0xFF

// HDCP PRAM Signature Ending Address Hi Register
#define REG_ADDR__HDCP2X_CUPD_SIGN_END_ADDR_HI                           (REGTX_HDCP2X | 0x00BC)
// (ReadWrite, Bits 7:0)
// Higher byte ending address for signature portion of PRAM access This is the starting address for signature portion for PRAM access. Program this before reg_hdcp2x_cupd_start or reg_hdcp2x_cupd_addr_reset
#define BIT_MSK__HDCP2X_CUPD_SIGN_END_ADDR_MT__REG_HDCP2X_CUPD_SI_E_ADDR_B15_B8                      0xFF

// HDCP General Control 0 Register
#define REG_ADDR__HDCP2X_CTL_0                                           (REGTX_HDCP2X | 0x00BD)
// (ReadWrite, Bits 7)
// HDCP 2.x Encryption Enable This goes to the TX hdmi block to generate CTL3 signal 0: Disable 1: Enable
#define BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_ENCRYPT_EN                                 0x80
// (ReadWrite, Bits 6)
// DDC polling interval select
#define BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_POLINT_SEL                                 0x40
// (ReadWrite, Bits 5)
// DDC polling interval override
#define BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_POLINT_OVR                                 0x20
// (ReadWrite, Bits 0)
// HDCP 2.x Enable When enabled video encryption mux and ddc mux will select HDCP 2.x. However this signal doesn't go to the hdcp2xcore
#define BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_EN                                         0x01

// HDCP General Control 1 Register
#define REG_ADDR__HDCP2X_CTL_1                                           (REGTX_HDCP2X | 0x00BE)
// (ReadWrite, Bits 2)
// HDP Override Value
#define BIT_MSK__HDCP2X_CTL_1__REG_HDCP2X_HPD_SW                                     0x04
// (ReadWrite, Bits 1)
// HPD Override
#define BIT_MSK__HDCP2X_CTL_1__REG_HDCP2X_HPD_OVR                                    0x02
// (ReadWrite, Bits 0)
// Reauthentication request This goes to ri_reauth_sw of hdcp2xcore Negative edge triggered
#define BIT_MSK__HDCP2X_CTL_1__REG_HDCP2X_REAUTH_SW                                  0x01

// HDCP General Control 1 Register
#define REG_ADDR__HDCP2X_CTL_2                                           (REGTX_HDCP2X | 0x00BF)
// (ReadWrite, Bits 0)
// Address reset for PRAM address reset Write 1 to this to give a pulse to reset the internal address pointer to the addresses specified by the other registers such as reg_hdcp2x_cupd_start_addr; reg_hdcp2x_cupd_sign_start_addr etc.
#define BIT_MSK__HDCP2X_CTL_2__REG_HDCP2X_CUPD_ADDR_RESET                            0x01

// HDCP CUPD Size Lo Register
#define REG_ADDR__HDCP2X_CUPD_SIZE_LO                                    (REGTX_HDCP2X | 0x00C0)
// (ReadWrite, Bits 7:0)
// Low byte of the size of updated code excluding signature Must be set before reg_hdcp2x_cupd_start or reg_hdcp2x_cupd_addr_reset.
#define BIT_MSK__HDCP2X_CUPD_SIZE_LO__REG_HDCP2X_CUPD_SIZE_B7_B0                            0xFF

// HDCP CUPD Size Hi Register
#define REG_ADDR__HDCP2X_CUPD_SIZE_HI                                    (REGTX_HDCP2X | 0x00C1)
// (ReadWrite, Bits 7:0)
// Low byte of the size of updated code excluding signature Must be set before reg_hdcp2x_cupd_start or reg_hdcp2x_cupd_addr_reset.
#define BIT_MSK__HDCP2X_CUPD_SIZE_MT__REG_HDCP2X_CUPD_SIZE_B15_B8                           0xFF

// HDCP General Staus Register
#define REG_ADDR__HDCP2X_GEN_STA                                         (REGTX_HDCP2X | 0x00C2)
// (ReadOnly, Bits 0)
// Code update done
#define BIT_MSK__HDCP2X_GEN_STA__REG_HDCP2X_CUPD_DONE                                  0x01

// HDCP Poll Interval 0 Register
#define REG_ADDR__HDCP2X_POLL_VAL0                                       (REGTX_HDCP2X | 0x00C3)
// (ReadWrite, Bits 7:0)
// DDC polling interval0
#define BIT_MSK__HDCP2X_POLL_VAL0__REG_HDCP2X_POL_VAL0_B7_B0                             0xFF

// HDCP Poll Interval 1 Register
#define REG_ADDR__HDCP2X_POLL_VAL1                                       (REGTX_HDCP2X | 0x00C4)
// (ReadWrite, Bits 7:0)
// DDC polling interval1
#define BIT_MSK__HDCP2X_POLL_VAL1__REG_HDCP2X_POL_VAL1_B7_B0                             0xFF

// HDCP DDCM Status Register
#define REG_ADDR__HDCP2X_DDCM_STS                                        (REGTX_HDCP2X | 0x00C5)
// (ReadOnly, Bits 7:4)
// DDCM Error Status
#define BIT_MSK__HDCP2X_DDCM_STS__REG_HDCP2X_DDCM_ERR_STS_B3_B0                         0xF0
// (ReadOnly, Bits 3:0)
// DDCM Control Status
#define BIT_MSK__HDCP2X_DDCM_STS__REG_HDCP2X_DDCM_CTL_CS_B3_B0                          0x0F

// HDCP Ring OSC Bist Register
#define REG_ADDR__HDCP2X_ROSC_BIST                                       (REGTX_HDCP2X | 0x00C6)
// (ReadOnly, Bits 2)
// Indicates that HDCP2 ring oscillator BIST fails
#define BIT_MSK__HDCP2X_ROSC_BIST__REG_HDCP2X_RINGOSC_BIST_FAIL                          0x04
// (ReadOnly, Bits 1)
// Indicates that HDCP2 ring oscillator BIST is done
#define BIT_MSK__HDCP2X_ROSC_BIST__REG_HDCP2X_RINGOSC_BIST_DONE                          0x02
// (ReadWrite, Bits 0)
// Start HDCP2 ring oscillator BIST
#define BIT_MSK__HDCP2X_ROSC_BIST__REG_HDCP2X_RINGOSC_BIST_START                         0x01

// HDCP2 PRAM Data In/Out Register
#define REG_ADDR__HDCP2_PRAM_DATA                                        (REGTX_HDCP2X | 0x00CF)
// (ReadWrite, Bits 7:0)
// HDCP2 PRAM data register for PRAM data read and write.
#define BIT_MSK__HDCP2_PRAM_DATA__HDCP2_PRAM_DATA                                       0xFF

//***************************************************************************
// REGTX_HDMI2 Address: 40
// HDMI2 Scramble Control Register
#define REG_ADDR__SCRCTL                                                 (REGTX_HDMI2 | 0x0000)
// (ReadWrite, Bits 5)
// 1 - HDMI2 encoder using packet analyzer 0 - Legacy encoder used for HDMI/MHL        - For MHL1/2/3 stream coming from TX datapath,         legacy encoder (0) has to be selected        - For HDMI2 stream, new encoder (1) has to be        selected        - For DVI/HDMI1.4 stream, either one works, but        for the better compatibility, we recommend to        use legacy encoder (0)                In summary, for HDMI2 stream, use 1, and for        all others (HDMI1.x, MHL1/2/3) use 0. This has        to come from TOP HW/FW that knows the stream it        makes
#define BIT_MSK__SCRCTL__REG_HDMI2_ON                                          0x20
// (ReadWrite, Bits 4)
// Special byass ON/OFF         (from i2c; def=0)
#define BIT_MSK__SCRCTL__REG_HDMI2_BYP                                         0x10
// (ReadWrite, Bits 3)
// HDMI mode overwriting value  (from i2c; def=0)
#define BIT_MSK__SCRCTL__REG_HDMIMD_VAL                                        0x08
// (ReadWrite, Bits 2)
// HDMI mode overwriting on/off (from i2c; def=0)
#define BIT_MSK__SCRCTL__REG_HDMIMD_OVR                                        0x04
// (ReadWrite, Bits 1)
// Scrambler mode: 0:  normal (default) 1:  CTS
#define BIT_MSK__SCRCTL__REG_SCR_MD                                            0x02
// (ReadWrite, Bits 0)
// Scrambler ON/OFF. Should follow HDMI2 spec such as mandatory for 600MHz, optional for lower, etc (from i2c or decision HW if availble; def=0)
#define BIT_MSK__SCRCTL__REG_SCR_ON                                            0x01

// HDMI Control 0 Register
#define REG_ADDR__HDMI2CTL0                                              (REGTX_HDMI2 | 0x0001)
// (ReadWrite, Bits 2)
// 1:  Invert tx bit  for ch0,1,2CK 0:  Normal (default)
#define BIT_MSK__HDMI2CTL0__REG_TX_BIT_INV                                        0x04
// (ReadWrite, Bits 1)
// 1:  Has 4 full channels (RGB, CK) (default) 0:  Force CK channel to all zero
#define BIT_MSK__HDMI2CTL0__REG_USE_CH_MUX                                        0x02
// (ReadWrite, Bits 0)
// 1:  Bit swap Q data out from HDMI2 encoder 0:  No swap (default)
#define BIT_MSK__HDMI2CTL0__REG_Q_9T0                                             0x01

// HDMI Control 1 Register
#define REG_ADDR__HDMI2CTL1                                              (REGTX_HDMI2 | 0x0002)
// (ReadWrite, Bits 7:6)
// 00:  select ch0 01:  select ch1 10:  select ch2 11:  select ck
#define BIT_MSK__HDMI2CTL1__REG_QC_SEL                                            0xC0
// (ReadWrite, Bits 5:4)
// 00:  select ch0 01:  select ch1 10:  select ch2 11:  select ck
#define BIT_MSK__HDMI2CTL1__REG_Q2_SEL                                            0x30
// (ReadWrite, Bits 3:2)
// 00:  select ch0 01:  select ch1 10:  select ch2 11:  select ck
#define BIT_MSK__HDMI2CTL1__REG_Q1_SEL                                            0x0C
// (ReadWrite, Bits 1:0)
// 00:  select ch0 01:  select ch1 10:  select ch2 11:  select ck
#define BIT_MSK__HDMI2CTL1__REG_Q0_SEL                                            0x03

// HDMI TXC Data 0L Register
#define REG_ADDR__TXC_DATA0L                                             (REGTX_HDMI2 | 0x0003)
// (ReadWrite, Bits 7:0)
// HDMI TX Clock data byte 0 Low portion
#define BIT_MSK__TXC_DATA0L__REG_TXC_DATA0_B7_B0                                   0xFF

// HDMI TXC Data 0H Register
#define REG_ADDR__TXC_DATA0H                                             (REGTX_HDMI2 | 0x0004)
// (ReadWrite, Bits 1:0)
// HDMI TX Clock data byte 0 high portion
#define BIT_MSK__TXC_DATA0H__REG_TXC_DATA0_B9_B8                                   0x03

// HDMI TXC DATA 1L Register
#define REG_ADDR__TXC_DATA1L                                             (REGTX_HDMI2 | 0x0005)
// (ReadWrite, Bits 7:0)
// HDMI TX Clock data byte 1 low portion
#define BIT_MSK__TXC_DATA1L__REG_TXC_DATA1_B7_B0                                   0xFF

// HDMI TXC DATA 1H Register
#define REG_ADDR__TXC_DATA1H                                             (REGTX_HDMI2 | 0x0006)
// (ReadWrite, Bits 1:0)
// HDMI TX Clock data byte 1 high portion
#define BIT_MSK__TXC_DATA1H__REG_TXC_DATA1_B9_B8                                   0x03

// HDMI TXC DATA 2L Register
#define REG_ADDR__TXC_DATA2L                                             (REGTX_HDMI2 | 0x0007)
// (ReadWrite, Bits 7:0)
// HDMI TX Clock data byte 2 low portion
#define BIT_MSK__TXC_DATA2L__REG_TXC_DATA2_B7_B0                                   0xFF

// HDMI TXC DATA 2H Register
#define REG_ADDR__TXC_DATA2H                                             (REGTX_HDMI2 | 0x0008)
// (ReadWrite, Bits 1:0)
// HDMI TX Clock data byte 2 high portion
#define BIT_MSK__TXC_DATA2H__REG_TXC_DATA2_B9_B8                                   0x03

// HDMI TXC DIVIDER Register
#define REG_ADDR__TXC_DATA_DIV                                           (REGTX_HDMI2 | 0x0009)
// (ReadWrite, Bits 1:0)
// 00:  TXC divide by 1 (default) 01:  TXC divide by 2 10:  TXC divide by 4 11:  TXC divide by 8
#define BIT_MSK__TXC_DATA_DIV__REG_TXC_DIV                                           0x03

// SCDC Control Register
#define REG_ADDR__SCDC_CTL                                               (REGTX_HDMI2 | 0x0020)
// (ReadWrite, Bits 6)
// 0: (default)  Firmware will grab the ddc bus any time when it is not used 1:  Once reg_ddc_stall_req is set to 1; it will wait for 128 frames to see if there is any hdcp activity; if so; will wait for current hdcp transaction finish before grabbing the bus; or timer expires.
#define BIT_MSK__SCDC_CTL__REG_SCDC_HDCP_DET_EN                                  0x40
// (ReadWrite, Bits 5)
// DDC bus stall request; it will disable reg_scdc_access bit; can't use together Setting this bit to 1 will hold ddc bus for SCDC manual transaction until firmware disable it It will stall hdcp1.4, hdcp2.2 and SCDC auto reply as well After setting bit to 1, HW will detect if ddc bus is busy.  Once the bus is free, it will generate an interrupt, ddc_stall_ack, 0x925[5], to notify firmware bus is ready to use
#define BIT_MSK__SCDC_CTL__REG_DDC_STALL_REQ                                     0x20
// (ReadWrite, Bits 4)
// SCDC auto poll select 0:  (default), use vsync as triggering point 1:  Use internal timer (240ms), program 0x927 and 0x928 for different desired time interval
#define BIT_MSK__SCDC_CTL__REG_SCDC_AUTO_POLL_SEL                                0x10
// (ReadWrite, Bits 3)
// SCDC auto reply read request with a stop condition instead of reading update flags, 0x920[1] also is needed to set for this function to work. 0:  Disable (default), normal auto reply read request 1:  Enable, SCDC replies with a stop condition
#define BIT_MSK__SCDC_CTL__REG_SCDC_AUTO_REPLY_STOP                              0x08
// (ReadWrite, Bits 2)
// SCDC auto polling read from slave for SCDC registers up_flag0 and up_flag1 on every frame 0:  Disable (default), manual polling by firmware 1:  Enable
#define BIT_MSK__SCDC_CTL__REG_SCDC_AUTO_POLL                                    0x04
// (ReadWrite, Bits 1)
// SCDC auto reply read request from slave for SCDC registers up_flag0 and up_flag1 0:  Disable (default), ignore read request 1:  Enable
#define BIT_MSK__SCDC_CTL__REG_SCDC_AUTO_REPLY                                   0x02
// (ReadWrite, Bits 0)
// SCDC access enable strobe, self clear bit 0:  regular DDC master 1.  Enable SCDC DDC bus cycle Write sequence: Firmware needs to check if DDC bus is idle, then write 1 to take over DDC bus.  After writing this bit, check 0x925[2] to make sure there is no bus conflicting.  If so, repeat above procedures.  Prepare DDC_FIFO data, DDC controls and send write command. Read sequence: Firmware needs to check if DDC bus is idle, then write 1 to take over DDC bus.  After writing this bit, check 0x925[2] to make sure there is no bus conflicting.  If so, repeat above procedures.  Prepare DDC controls, send write command.  When 0x925[0] = 1, read DDC_FIFO.
#define BIT_MSK__SCDC_CTL__REG_SCDC_ACCESS                                       0x01

// SCDC Active Status Register
#define REG_ADDR__SCDC_ACT_STATUS                                        (REGTX_HDMI2 | 0x0021)
// (ReadOnly, Bits 2)
// SCDC Read Write in progress
#define BIT_MSK__SCDC_ACT_STATUS__REG_SCDC_IN_PROG                                      0x04
// (ReadOnly, Bits 1)
// SCDC auto reply read request  from slave for SCDC registers up_flag0 and up_flag1  or read polling in progress
#define BIT_MSK__SCDC_ACT_STATUS__REG_SCDC_RREQ_IN_PROG                                 0x02
// (ReadOnly, Bits 0)
// SCDC is using DDC Bus
#define BIT_MSK__SCDC_ACT_STATUS__REG_SCDC_ACTIVE                                       0x01

// SCDC State Machine Status Register
#define REG_ADDR__SCDC_STATE                                             (REGTX_HDMI2 | 0x0022)
// (ReadOnly, Bits 7:4)
// SCDC read/write state machine status
#define BIT_MSK__SCDC_STATE__REG_SCDC_STATE                                        0xF0
// (ReadOnly, Bits 3:0)
// Auto Reply to SCDC slave read request state machine status
#define BIT_MSK__SCDC_STATE__REG_SCDC_RREQ_STATE                                   0x0F

// SCDC Update Flag 0 Status Register
#define REG_ADDR__SCDC_UP_FLAG0                                          (REGTX_HDMI2 | 0x0023)
// (ReadOnly, Bits 7:0)
// SCDC Update Flag0 Status
#define BIT_MSK__SCDC_UP_FLAG0__REG_SCDC_UP_FLAG0                                     0xFF

// SCDC Update Flag 1 Status Register
#define REG_ADDR__SCDC_UP_FLAG1                                          (REGTX_HDMI2 | 0x0024)
// (ReadOnly, Bits 7:0)
// SCDC Update Flag1 Status
#define BIT_MSK__SCDC_UP_FLAG1__REG_SCDC_UP_FLAG1                                     0xFF

// SCDC interrupt 0 Register
#define REG_ADDR__SCDC_INTR0                                             (REGTX_HDMI2 | 0x0025)
// (ReadWrite, Bits 5)
// SCDC DDC stall request acknowledge
#define BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT5                                  0x20
// (ReadWrite, Bits 4)
// SCDC DDC update flag being changed comparing with previous one
#define BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT4                                  0x10
// (ReadWrite, Bits 3)
// SCDC slave read request interrupt; slave is requesting SCDC Master to read update flags
#define BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT3                                  0x08
// (ReadWrite, Bits 2)
// SCDC DDC conflicting with other client when trying to access DDC Bus
#define BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT2                                  0x04
// (ReadWrite, Bits 1)
// SCDC DDC read update flag done
#define BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT1                                  0x02
// (ReadWrite, Bits 0)
// SCDC DDC read/write access done
#define BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT0                                  0x01

// SCDC interrupt 0 Mask Register
#define REG_ADDR__SCDC_INTR0_MASK                                        (REGTX_HDMI2 | 0x0026)
// (ReadWrite, Bits 5)
// SCDC DDC stall request acknowledge mask 0:  disable (default) 1: enable interrupt
#define BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK5                                  0x20
// (ReadWrite, Bits 4)
// SCDC DDC update flag change mask 0:  disable (default) 1: enable interrupt
#define BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK4                                  0x10
// (ReadWrite, Bits 3)
// SCDC DDC slave read request mask 0:  disable (default) 1: enable interrupt
#define BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK3                                  0x08
// (ReadWrite, Bits 2)
// SCDC DDC bus conflicting mask 0:  disable (default) 1: enable interrupt
#define BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK2                                  0x04
// (ReadWrite, Bits 1)
// SCDC DDC read update flag done mask 0:  disable (default) 1: enable interrupt
#define BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK1                                  0x02
// (ReadWrite, Bits 0)
// SCDC DDC read/write access done mask 0:  disable (default) 1: enable interrupt
#define BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK0                                  0x01

// SCDC Auto Poll timer 0 Register
#define REG_ADDR__SCDC_POLL_TIMER0                                       (REGTX_HDMI2 | 0x0027)
// (ReadWrite, Bits 7:0)
// SCDC auto polling timer
#define BIT_MSK__SCDC_POLL_TIMER0__REG_SCDC_AUTO_POLL_TIMER_B7_B0                        0xFF

// SCDC Auto Poll timer 1 Register
#define REG_ADDR__SCDC_POLL_TIMER1                                       (REGTX_HDMI2 | 0x0028)
// (ReadWrite, Bits 7:0)
// SCDC auto polling timer
#define BIT_MSK__SCDC_POLL_TIMER1__REG_SCDC_AUTO_POLL_TIMER_B15_B8                       0xFF

// SCDC Auto Poll timer 2 Register
#define REG_ADDR__SCDC_POLL_TIMER2                                       (REGTX_HDMI2 | 0x0029)
// (ReadWrite, Bits 5:0)
// SCDC auto polling timer
#define BIT_MSK__SCDC_POLL_TIMER2__REG_SCDC_AUTO_POLL_TIMER_B21_B16                      0x3F

// SCDC Frame Limit Register
#define REG_ADDR__SCDC_FRM_LIMIT                                         (REGTX_HDMI2 | 0x002B)
// (ReadWrite, Bits 7:0)
// SCDC Frame Limit; if internal frame counter (sync to ri_check) is less or equal to this value; then it grants the ddc bus for scdc access
#define BIT_MSK__SCDC_FRM_LIMIT__REG_SCDC_FRAME_LIMIT                                  0xFF

// SCDC Read Req Frame Limit Register
#define REG_ADDR__RREQ_FRM_LIMIT                                         (REGTX_HDMI2 | 0x002C)
// (ReadWrite, Bits 7:0)
// SCDC Read Request Frame Limit; if internal frame counter (sync to ri_check) is less or equal to this value; then it grants the ddc bus for scdc read request
#define BIT_MSK__RREQ_FRM_LIMIT__REG_RREQ_FRAME_LIMIT                                  0xFF

//***************************************************************************
// REGTX_AIP Address: 40
// ACR  Control Register
#define REG_ADDR__ACR_CTRL                                               (REGTX_AIP | 0x0001)
// (ReadWrite, Bits 3)
// 1: CTS Gen without MCLK (auto MCLK generation) 0: CTS Gen with MCLK
#define BIT_MSK__ACR_CTRL__REG_NO_MCLK_CTSGEN_SEL                                0x08
// (ReadWrite, Bits 2)
// select between mclk and tclk time domain mclk counter
#define BIT_MSK__ACR_CTRL__REG_MCLK_EN                                           0x04
// (ReadWrite, Bits 1)
// 0 - Requests by the ACR module to transmit an CTS/N packet are ignored; so no CTS/N packets are transmitted; multiple CTS/N packet requests that are unserviced do not generate and ACR interrupt (see INTR[2]); 1 - Requests by the ACR module to transmit an CTS/N packet are serviced per microarchitecture spec; multiple CTS/N packet requests that are unserviced generate an ACR interrupt (see INTR[2]);   (default)
#define BIT_MSK__ACR_CTRL__REG_CTS_REQ_EN                                        0x02
// (ReadWrite, Bits 0)
// 0 - The CTS value updated by hardware (CTS_HVAL) is sent as HDMI       packet to Rx (default); 1 - The CTS value set by software (CTS_SVAL) is sent as HDMI packet to Rx
#define BIT_MSK__ACR_CTRL__REG_CTS_HW_SW_SEL                                     0x01

// ACR  Audio Frequency Register
#define REG_ADDR__FREQ_SVAL                                              (REGTX_AIP | 0x0002)
// (ReadWrite, Bits 2:0)
// MCLK input mode: 000 = MCLK is 128*Fs 001 = MCLK is 256*Fs (default) 010 = MCLK is 384*Fs 011 = MCLK is 512*Fs                                                  100 = MCLK is 768*Fs 101 = MCLK is 1024*Fs 110 = MCLK is 1152*Fs 111 = mCLK is 192*Fs
#define BIT_MSK__FREQ_SVAL__REG_FM_IN_VAL_SW                                      0x07

// ACR N Software Value #1 Register
#define REG_ADDR__N_SVAL1                                                (REGTX_AIP | 0x0003)
// (ReadWrite, Bits 7:0)
// N value for audio clock regeneration method. Bits [7:0] of 20-bits value from the N/CTS packet.  Must be entered by software to create the correct divider.
#define BIT_MSK__N_SVAL1__REG_N_VAL_SW1                                         0xFF

// ACR N Software Value #2 Register
#define REG_ADDR__N_SVAL2                                                (REGTX_AIP | 0x0004)
// (ReadWrite, Bits 7:0)
// N value for audio clock regeneration method. Bits [15:8] of 20-bits value from the N/CTS packet.  Must be entered by software to create the correct divider.
#define BIT_MSK__N_SVAL2__REG_N_VAL_SW2                                         0xFF

// ACR N Software Value #3 Register
#define REG_ADDR__N_SVAL3                                                (REGTX_AIP | 0x0005)
// (ReadWrite, Bits 3:0)
// N value for audio clock regeneration method. Bits [19:16] of 20-bits value from the N/CTS packet.  Must be entered by software to create the correct divider.
#define BIT_MSK__N_SVAL3__REG_N_VAL_SW3                                         0x0F

// ACR CTS Software Value #1 Register
#define REG_ADDR__CTS_TXSVAL1                                            (REGTX_AIP | 0x0006)
// (ReadWrite, Bits 7:0)
// Bits [7:0] of 20-bits of the audio clock regeneration CTS value that has been set by software. Whether this software value is sent in the CTS packet to the Rx is controlled by ACR_CTRL[0]
#define BIT_MSK__CTS_TXSVAL1__REG_CTS_VAL_SW_B7_B0                                  0xFF

// ACR CTS Software Value #2 Register
#define REG_ADDR__CTS_TXSVAL2                                            (REGTX_AIP | 0x0007)
// (ReadWrite, Bits 7:0)
// Bits [15:8] of 20-bits of the audio clock regeneration CTS value that has been set by software. Whether this software value is sent in the CTS packet to the Rx is controlled by ACR_CTRL[0]
#define BIT_MSK__CTS_TXSVAL2__REG_CTS_VAL_SW_B15_B8                                 0xFF

// ACR CTS Software Value #3 Register
#define REG_ADDR__CTS_TXSVAL3                                            (REGTX_AIP | 0x0008)
// (ReadWrite, Bits 3:0)
// Bits [19:16] of 20-bits of the audio clock regeneration CTS value that has been set by software. Whether this software value is sent in the CTS packet to the Rx is controlled by ACR_CTRL[0]
#define BIT_MSK__CTS_TXSVAL3__REG_CTS_VAL_SW_B19_B16                                0x0F

// ACR CTS Hardware Value #1 Register
#define REG_ADDR__CTS_TXHVAL1                                            (REGTX_AIP | 0x0009)
// (ReadOnly, Bits 7:0)
// Bits [7:0] of 20-bits of the audio clock regeneration CTS value that is measured and updated by the Tx. Whether this hardware value is sent in the CTS packet to the Rx is controlled by ACR_CTRL[0]
#define BIT_MSK__CTS_TXHVAL1__CTS_VAL_HW_B7_B0                                      0xFF

// ACR CTS Hardware Value #2 Register
#define REG_ADDR__CTS_TXHVAL2                                            (REGTX_AIP | 0x000A)
// (ReadOnly, Bits 7:0)
// Bits [15:8] of 20-bits of the audio clock regeneration CTS value that is measured and updated by the Tx. Whether this hardware value is sent in the CTS packet to the Rx is controlled by ACR_CTRL[0]
#define BIT_MSK__CTS_TXHVAL2__CTS_VAL_HW_B15_B8                                     0xFF

// ACR CTS Hardware Value #3 Register
#define REG_ADDR__CTS_TXHVAL3                                            (REGTX_AIP | 0x000B)
// (ReadOnly, Bits 3:0)
// Bits [19:16] of 20-bits of the audio clock regeneration CTS value that is measured and updated by the Tx. Whether this hardware value is sent in the CTS packet to the Rx is controlled by ACR_CTRL[0]
#define BIT_MSK__CTS_TXHVAL3__CTS_VAL_HW_B19_B16                                    0x0F

// ACR CTS Filter Ctrl #1 Register
#define REG_ADDR__ACR_CTS_CTRL1                                          (REGTX_AIP | 0x000C)
// (ReadWrite, Bits 6)
// enable to use software cts as average, instead of internal histogram
#define BIT_MSK__ACR_CTS_CTRL1__REG_USE_SW_CTS                                        0x40
// (ReadWrite, Bits 5:1)
// # of filter steps: ]15 is 16; ]7 is 8; ]3 is 4; else 2 This # also tells how many new cts value outside of average range need to get in raw to start to calculate new average #
#define BIT_MSK__ACR_CTS_CTRL1__REG_AVE_MAX                                           0x3E
// (ReadWrite, Bits 0)
// enable cts filtering
#define BIT_MSK__ACR_CTS_CTRL1__REG_CTS_FLT_EN                                        0x01

// ACR CTS Filter Ctrl #2 Register
#define REG_ADDR__ACR_CTS_CTRL2                                          (REGTX_AIP | 0x000D)
// (ReadWrite, Bits 7:0)
// thresholf value that will be subtracted from or added to cts average number. That will create range against which new cts value will be evaluated. If reg_cts_filt_en is set and new cts value is outside of that range then this new cts value be regejected and old value is used.
#define BIT_MSK__ACR_CTS_CTRL2__REG_CTS_THRE                                          0xFF

// Input Audio ID Register
#define REG_ADDR__AIP_IN_AUD_ID                                          (REGTX_AIP | 0x0012)
// (ReadOnly, Bits 4:0)
// Identification of the incoming stream: bit 0 - ID of the SPDIF bit 1 - ID of the I2S bit 2 - ID of the DSD bit 3 - ID of the HBRA bit 4 - ID of the Audio packetized
#define BIT_MSK__AIP_IN_AUD_ID__IN_AUD_ID                                             0x1F

// Audio En Register
#define REG_ADDR__AUD_EN                                                 (REGTX_AIP | 0x0013)
// (ReadWrite, Bits 2)
// Enable loading of the audio through DMA (parrel audio input)
#define BIT_MSK__AUD_EN__REG_AUD_PAR_EN                                        0x04
// (ReadWrite, Bits 1)
// AIP can select to overwrite incoming selection of the audio by values in AUD_MODE
#define BIT_MSK__AUD_EN__REG_AUD_SEL_OWRT                                      0x02
// (ReadWrite, Bits 0)
// Audio input enable 0 - audio inputs disabled (default) 1 - audio inputs enabled
#define BIT_MSK__AUD_EN__REG_AUD_IN_EN                                         0x01

// Audio In Mode Register
#define REG_ADDR__AUD_MODE                                               (REGTX_AIP | 0x0014)
// (ReadWrite, Bits 7:4)
// I2S enable for SD3; SD2; SD1; SD0.  Bit 7 = SD3_en  Bit 4 = SD0_en.  All inputs disabled at default. When DSD enabled these bits control the DSD stream. NOTE: This bit(s) must also be set in case of parallel audio input if the audio format is I2S in order to ensure correct use of audio header type.
#define BIT_MSK__AUD_MODE__REG_I2S_EN                                            0xF0
// (ReadWrite, Bits 3)
// DSD enable. Has lower priorety then spdif enable; but higher then I2S stream. When it is set then most of the I2S configuration register bits became control for the DSD logic; such as SD3/2/1/0 enable; I2S FIFO map and Channel Status registers. NOTE: This bit must also be set in case of parallel audio input if the audio format is DSD in order to ensure correct use of audio header type.
#define BIT_MSK__AUD_MODE__REG_DSD_EN                                            0x08
// (ReadWrite, Bits 2)
// High Bitrate Audio flag 0 - Input stream is not High Bitrate stream (default) 1 - Input stream is High Bitrate stream (all of the I2S control bits will apply to the control of the HB Audio)
#define BIT_MSK__AUD_MODE__REG_HBRA_ON                                           0x04
// (ReadWrite, Bits 1)
// SPDIF input enable. 0 - SPDIF input stream is disabled (default) 1 - SPDIF input stream is enabled. NOTE: This bit must also be set in case of parallel audio input if the audio format is SPDIF in order to ensure correct use of audio header type.
#define BIT_MSK__AUD_MODE__REG_SPDIF_EN                                          0x02
// (ReadWrite, Bits 0)
// Enable to use packetize audio as input
#define BIT_MSK__AUD_MODE__REG_AUD_PKT_EN                                        0x01

// Audio In SPDIF Control Register
#define REG_ADDR__SPDIF_CTRL                                             (REGTX_AIP | 0x0015)
// (ReadOnly, Bits 7:4)
// Channel Status bits 33 to 35; where bit 33 = LSB and 35 = MSB               max 24                               max 20 000      not indicated (default)          not indicated (default) 001      20 bits                                 16 bits 010      22 bits                                 18 bits 100      23 bits                                 19 bits 101      24 bits                                 20 bits 110      21 bits                                 17 bits
#define BIT_MSK__SPDIF_CTRL__AUDI_LENGTH                                           0xF0
// (ReadOnly, Bits 3)
// 1 - there is no spdif input: spdif input never changes; 0- detected some change on the input SPDIF
#define BIT_MSK__SPDIF_CTRL__AUDI_NO_AUDIO                                         0x08
// (ReadWrite, Bits 2)
// 0 - hardware sample # automatically adjusted for sampling frequency changes (default); 1 -  if and only if lock for 1UI is set then hardware is locked to the reg_max_2ui number
#define BIT_MSK__SPDIF_CTRL__REG_2UI_LOCK                                          0x04
// (ReadWrite, Bits 1)
// 0 - Use input SPDIF stream Fs; reg SPDIF Fs (default); 1 - Override input stream Fs with Software Fs; define in I2S_CHST4
#define BIT_MSK__SPDIF_CTRL__REG_FS_OVERRIDE                                       0x02
// (ReadWrite, Bits 0)
// 0 - hardware sample # automatically adjusted for sampling frequency changes (default); 1 - hardware is locked to the reg_max_1ui number
#define BIT_MSK__SPDIF_CTRL__REG_1UI_LOCK                                          0x01

// Audio In SPDIF Software 1UI Overwrite Register
#define REG_ADDR__SPDIF_SSMPL                                            (REGTX_AIP | 0x0016)
// (ReadWrite, Bits 7:0)
// Maximum number of the pixel clocks per one SPDIF bi-phase mark encoded Unit Interval (max 1UI).  Hardware is locked to this number if reg_1ui_lock is set to 1;  otherwise hardware calculates this number automaticly.   Default is 4 (max # if pclk @ 25 MHz and Fs  @ 48 KHz) If reg_1ui1_lock and reg_2ui_lock are both asserted  (set to 1) then max 2UI will be set to the reg_max_2ui value; If reg_1ui_lock is set; but reg_2ui_lock is not then max 2UI is set to the reg_max_1ui times 2;
#define BIT_MSK__SPDIF_SSMPL__REG_MAX_1UI                                           0xFF

// Audio In SPDIF Hardware 1UI Sample Register
#define REG_ADDR__SPDIF_HSMPL                                            (REGTX_AIP | 0x0017)
// (ReadOnly, Bits 7:0)
// Maximum number of the pixel clocks per one SPDIF bi-phase encoded Unit Interval (max 1UI); continuously sampled by the hardware.
#define BIT_MSK__SPDIF_HSMPL__AUDI_MAX_1UI                                          0xFF

// Audio In SPDIF Extracted Fs Register
#define REG_ADDR__SPDIF_FS                                               (REGTX_AIP | 0x0018)
// (ReadOnly, Bits 5:0)
// contain the Fs extraction from SPDIF input channel status bits 24-27 30-31.   31 30 27 26 25 24 0   0   0   1   0   0 - Fs =   22.05 kHz 0   0   0   0   0   0 - Fs =   44.1 kHz 0   0   1   0   0   0 - Fs =   88.2 kHz 0   0   1   1   0   0 - Fs = 176.4 kHz 0   0   1   1   0   1 - Fs = 352.8 kHz 1   0   1   1   0   1 - Fs = 705.6 kHz 0   1   1   1   0   1 - Fs = 1411.2 kHz 0   0   0   1   1   0 - Fs =   24 kHz 0   0   0   0   1   0 - Fs =   48 kHz 0   0   1   0   1   0 - Fs =   96 kHz 0   0   1   1   1   0 - Fs = 192 kHz 0   0   0   1   0   1 - Fs = 384 kHz 0   0   1   0   0   1 - Fs = 768 kHz 0   1   0   1   0   1 - Fs = 1536 kHz 0   0   0   0   1   1 - Fs =   32 kHz 0   0   1   0   1   1 - Fs =   64 kHz 1   0   1   0   1   1 - Fs =   128 kHz 0   1   1   0   1   1 - Fs =   256 kHz 1   1   1   0   1   1 - Fs =   512 kHz 1   1   0   1   0   1 - Fs =   1024 kHz 0   0   0   0   0   1 Sampling frequency not indicated
#define BIT_MSK__SPDIF_FS__AUDI_SPDIF_FS                                         0x3F

// Audio In SPDIF Software 2UI Overwrite Register
#define REG_ADDR__SPDIF_SSMPL2                                           (REGTX_AIP | 0x0019)
// (ReadWrite, Bits 7:0)
// Maximum number of the pixel clocks per two SPDIF bi-phase encoded Unit Intervals (max 2UI).  Hardware is locked to this number if reg_2ui_lock and reg_1ui_lock both are set to 1; If only reg_1ui_lock is set to one then hardware is locked to the multiple of 2 of the software max for 1UI. Otherwise hardware calculates this number automaticly.  Default is 9 (max # if pclk @ 25 MHz and Fs  @ 48 KHz)  In the I2S mode bits 7:4 are used as swap control for the channels 0-3.  Bit 4 is for channel 0: 1 would force swap between left and right; Bit 5 is for channel 1: 1 would force swap between left and right; Bit 6 is for channel 2: 1 would force swap between left and right; Bit 7 is for channel 3: 1 would force swap between left and right;
#define BIT_MSK__SPDIF_SSMPL2__REG_MAX_2UI                                           0xFF

// Audio In SPDIF Hardware 2UI Sample Register
#define REG_ADDR__SPDIF_HSMPL2                                           (REGTX_AIP | 0x001A)
// (ReadOnly, Bits 7:0)
// Maximum number of the pixel clocks per two SPDIF bi-phase encoded Unit Intervals (max 2UI); continuously sampled by the hardware.
#define BIT_MSK__SPDIF_HSMPL2__AUDI_MAX_2UI                                          0xFF

// Audio In Error Threshold Register
#define REG_ADDR__SPDIF_ERTH                                             (REGTX_AIP | 0x001B)
// (ReadWrite, Bits 6)
// enable to use I2S and SPDIF ports for 1st 6 pins of the DSD
#define BIT_MSK__SPDIF_ERTH__REG_I2S2DSD_EN                                        0x40
// (ReadWrite, Bits 5:0)
// Error threshold level. The frame will be marked as invalid (flat flag will be set) if during decoding of the frame the number of the bi-phase mark encode related errors will exceed this threshold level.
#define BIT_MSK__SPDIF_ERTH__REG_AUD_ERR_THRESH                                    0x3F

// Audio IN I2S Data In Map Register
#define REG_ADDR__I2S_IN_MAP                                             (REGTX_AIP | 0x001C)
// (ReadWrite, Bits 7:6)
// Channel map to FIFO #3 (for HDMI layout 1): 00 - map SD0 to FIFO #3 01 - map SD1 to FIFO #3 10 - map SD2 to FIFO #3 11 - map SD3 to FIFO #3 (default)
#define BIT_MSK__I2S_IN_MAP__REG_FIFO3_MAP                                         0xC0
// (ReadWrite, Bits 5:4)
// Channel map to FIFO #2 (for HDMI layout 1): 00 - map SD0 to FIFO #2 01 - map SD1 to FIFO #2 10 - map SD2 to FIFO #2 (default) 11 - map SD3 to FIFO #2
#define BIT_MSK__I2S_IN_MAP__REG_FIFO2_MAP                                         0x30
// (ReadWrite, Bits 3:2)
// Channel map to FIFO #1 (for HDMI layout 1): 00 - map SD0 to FIFO #1 01 - map SD1 to FIFO #1 (default) 10 - map SD2 to FIFO #1 11 - map SD3 to FIFO #1
#define BIT_MSK__I2S_IN_MAP__REG_FIFO1_MAP                                         0x0C
// (ReadWrite, Bits 1:0)
// Channel map to FIFO #0 (for HDMI layout 0 or 1): 00 - map SD0 to FIFO #0 (default) 01 - map SD1 to FIFO #0 10 - map SD2 to FIFO #0 11 - map SD3 to FIFO #0
#define BIT_MSK__I2S_IN_MAP__REG_FIFO0_MAP                                         0x03

// Audio In I2S Control Register
#define REG_ADDR__I2S_IN_CTRL                                            (REGTX_AIP | 0x001D)
// (ReadWrite, Bits 7)
// phase of MCLK. 0: no invert; 1: invert
#define BIT_MSK__I2S_IN_CTRL__REG_M_CK_PHASE                                        0x80
// (ReadWrite, Bits 6)
// SCK: Sample edge rising/falling 0 - Sample edge is falling: SD3-SD0 and WS source should change state on the rising edge of the SCK 1 - Sample edge is rising (default): SD3-SD0 and WS source should change state on the falling edge of SCK
#define BIT_MSK__I2S_IN_CTRL__REG_SCK_EDGE                                          0x40
// (ReadWrite, Bits 5)
// Order of the Channel Status bits in the High Bitrate stream:  0 - Left and Right sample have consecutive C bits (default) 1 - Left and Right sample have the same C bits
#define BIT_MSK__I2S_IN_CTRL__REG_CBIT_ORDER                                        0x20
// (ReadWrite, Bits 4)
// V bit value: 0 - For PCM data (default)
#define BIT_MSK__I2S_IN_CTRL__REG_VBIT                                              0x10
// (ReadWrite, Bits 3)
// WS: Left/Right polarity: 0 - Left polarity when Word Select is low (default) When TDM is not used then WS for I2S (reg_ws(I2S_IN_CTRL[3])) have to match left/right orientation that Source sends. When TDM is used then need to set reg_ws(I2S_IN_CTRL[3]) to opposite of the reg_tdm_ws_negedge(TDM_CTRL[5])
#define BIT_MSK__I2S_IN_CTRL__REG_WS                                                0x08
// (ReadWrite, Bits 2)
// SD: Left-; Right-justified: 1 - data is right justified (default)
#define BIT_MSK__I2S_IN_CTRL__REG_JUSTIFY                                           0x04
// (ReadWrite, Bits 1)
// SD: MSb/LSb first: 0 - MSb first (default)
#define BIT_MSK__I2S_IN_CTRL__REG_DATA_DIR                                          0x02
// (ReadWrite, Bits 0)
// WS to SD: 1st-bit shift 0 - 1st-bit shift; Philips spec. 1 - no shift (default)
#define BIT_MSK__I2S_IN_CTRL__REG_1ST_BIT                                           0x01

// Audio In I2S Channel Status #0 Register
#define REG_ADDR__I2S_CHST0                                              (REGTX_AIP | 0x001E)
// (ReadWrite, Bits 7:0)
// The information in this register is send in Channel Status field across HDMI link Channel Status byte #0. Please refer to SPDIF spec for detailed description.
#define BIT_MSK__I2S_CHST0__REG_CBIT0                                             0xFF

// Audio In I2S Channel Status #1 Register
#define REG_ADDR__I2S_CHST1                                              (REGTX_AIP | 0x001F)
// (ReadWrite, Bits 7:0)
// The information in this register is send in Channel Status field across HDMI link.  Category code. Please refer to SPDIF spec for detailed description.
#define BIT_MSK__I2S_CHST1__REG_CBIT1                                             0xFF

// Audio In I2S Channel Status #2 Register
#define REG_ADDR__I2S_CHST2                                              (REGTX_AIP | 0x0020)
// (ReadWrite, Bits 7:4)
// The information in this register is send in Channel Status field across HDMI link Channel Number. Please refer to SPDIF spec for detailed description.
#define BIT_MSK__I2S_CHST2__REG_CBIT2B                                            0xF0
// (ReadWrite, Bits 3:0)
// The information in this register is send in Channel Status field across HDMI link. Source number. Please refer to SPDIF spec for detailed description.
#define BIT_MSK__I2S_CHST2__REG_CBIT2A                                            0x0F

// Audio in I2S Channel Status #3 Register
#define REG_ADDR__I2S_CHST3                                              (REGTX_AIP | 0x0021)
// (ReadWrite, Bits 7:4)
// [5:4] Clock accuracy [7:6] Sampling frequency extension with sampling frequency bits 24 to 27 hbra_on, aud_sample_freq[5:0];  Audio Rate 1b0, 2b00, 4'b0100; 22.05 kHz (n/a) 1b0, 2b00, 4'b0000; 44.1 kHz 1b0, 2b00, 4'b1000; 88.2 kHz 1b0, 2b00, 4'b1100; 176.4 kHz 1b0, 2b00, 4b1101; 352.8 kHz 1b0, 2b10, 4b1101; 705.6 kHz 1b0, 2b01, 4b1101; 1411.2 kHz (n/a) 1b0, 2b00, 4'b0110; 24 kHz (n/a) 1b0, 2b00, 4'b0010; 48 kHz 1b0, 2b00, 4'b1010; 96 kHz 1b0, 2b00, 4'b1110; 192 kHz 1b0, 2b00; 4b0101; 384 kHz 1b0, 2b00, 4b1001; 768 kHz 1b0, 2b01; 4b0101; 1536 kHz (n/a) 1b0, 2b00, 4'b0011; 32 kHz 1b0, 2b00, 4b1011; 64 kHz 1b0, 2b10, 4b1011; 128 kHz 1b0, 2b01, 4b1011; 256 kHz 1b0, 2b11, 4b1011; 512 kHz 1b0, 2b11; 4b0101; 1024 kHz (n/a) 1b1, 2b00, 4'b1100; HBRA 176.4 kHz 1b1, 2b00, 4'b1101; HBRA 352.8 kHz 1b1, 2b10, 4'b1101; HBRA 705.6 kHz 1b1, 2b01, 4b1101; HBRA 1411.2 kHz 1b1, 2b00, 4'b1110; HBRA 192 kHz 1b1, 2b00, 4'b0101; HBRA 384 kHz 1b1, 2b00, 4'b1001; HBRA 768 kHz 1b1, 2b01; 4b0101; HBRA 1536 kHz 1b1, 2b10, 4'b1011; HBRA 128 kHz 1b1, 2b01, 4b1011; HBRA 256 kHz 1b1, 2b11, 4b1011; HBRA 512 kHz 1b1, 2b11, 4b0101; HBRA 1024 kHz
#define BIT_MSK__I2S_CHST3__REG_CBIT3B                                            0xF0
// (ReadWrite, Bits 3:0)
// The information in this register is send in Channel Status field across HDMI link. Please refer to SPDIF spec for detailed description. Sampling frequency set by software (inserted into I2S stream or into SPDIF if fs_overrride is enabled). These bits correspond to the Channel Status bits 24; 25; 26;27; where bit 24 = LSB and 27 = MSB: 27 26 25 24 0   1   0   0 - Fs =   22.05 kHz 0   0   0   0 - Fs =   44.1 kHz 1   0   0   0 - Fs =   88.2 kHz 1   1   0   0 - Fs = 176.4 kHz 0   1   1   0 - Fs =   24 kHz 0   0   1   0 - Fs =   48 kHz 1   0   1   0 - Fs =   96 kHz 1   1   1   0 - Fs = 192 kHz 0   0   1   1 - Fs =   32 kHz 1  0  0  1 -  Fs =  768 kHz*  All other frequencies (default)
#define BIT_MSK__I2S_CHST3__REG_CBIT3A                                            0x0F

// Audio In I2S Channel Status #4 Register
#define REG_ADDR__I2S_CHST4                                              (REGTX_AIP | 0x0022)
// (ReadWrite, Bits 7:4)
// Original Fs
#define BIT_MSK__I2S_CHST4__REG_CBIT4B                                            0xF0
// (ReadWrite, Bits 3:0)
// Reg_cbit[32] : The information in this register is send in Channel Status field across HDMI link. Channel Status bits32: 0 - Max audio sample word length is 20 bits 1 - Max audio sample word length is 24 bits (default) Reg_cbit[35:32] : The information in this register is send in Channel Status field across HDMI link.Channel Status bits 33 to 35; where bit 33 = LSB and 35 = MSB               max 24                               max 20 000      not indicated         not indicated  001      20 bits                                 16 bits 010      22 bits                                 18 bits 100      23 bits                                 19 bits 101      24 bits                                 20 bits 110      21 bits                                 17 bits
#define BIT_MSK__I2S_CHST4__REG_CBIT4A                                            0x0F

// Audio In Sample Rate Conversion Register
#define REG_ADDR__ASRC                                                   (REGTX_AIP | 0x0023)
// (ReadWrite, Bits 7:4)
// Mask for the sample present and flat bit of the High Bit Rate Audio header.  Each bit mask out 1 of the subpacket sample presetn bits. When 0 mask out. Default; only 1 lsb bit is unmasked.
#define BIT_MSK__ASRC__REG_HBR_SPR_MASK                                      0xF0
// (ReadWrite, Bits 1)
// Selects downsample mode: if 0 then 2:1 mode and will do following sample rate conversion:         44.1   -] 22.05        48      -] 24        88.2   -] 44.1        96      -] 48         176.4 -] 88.2         192    -] 96 if 1 then 4:1 mode and will do following sample rate conversion:         88.2   -] 22.05        96      -] 24         176.4 -] 44.1         192    -] 48
#define BIT_MSK__ASRC__REG_SRC_CTRL                                          0x02
// (ReadWrite, Bits 0)
// Enable Audio Sample rate Conversion: 0 disabled (default)
#define BIT_MSK__ASRC__REG_SRC_EN                                            0x01

// Audio In I2S Input Size Register
#define REG_ADDR__I2S_IN_SIZE                                            (REGTX_AIP | 0x0024)
// (ReadWrite, Bits 3:0)
// The information in this register is used for the extraction of the I2S data from the input stream. Number of the valid bits in the input I2S stream; default is 24: 1011 - 24 bits (default); 1001 - 23 bits; 0101 - 22 bits; 1101 - 21 bits; 1010 - 20 bits 1000 - 19 bits 0100 - 18 bits 1100 - 17 bits 0010 - 16 bits
#define BIT_MSK__I2S_IN_SIZE__REG_I2S_IN_LENGTH                                     0x0F

// Audio Parallel busclk clock disabling Register 1
#define REG_ADDR__AUD_PAR_BUSCLK_1                                       (REGTX_AIP | 0x0025)
// (ReadWrite, Bits 7:0)
// Audio Parallel busclk clock disabling Register
#define BIT_MSK__AUD_PAR_BUSCLK_1__REG_AUD_PAR_B7_B0                                     0xFF

// Audio Parallel busclk clock disabling Register 2
#define REG_ADDR__AUD_PAR_BUSCLK_2                                       (REGTX_AIP | 0x0026)
// (ReadWrite, Bits 7:0)
// Audio Parallel busclk clock disabling Register
#define BIT_MSK__AUD_PAR_BUSCLK_2__REG_AUD_PAR_B15_B8                                    0xFF

// Audio Parallel busclk clock disabling Register 3
#define REG_ADDR__AUD_PAR_BUSCLK_3                                       (REGTX_AIP | 0x0027)
// (ReadWrite, Bits 7:0)
// Audio Parallel busclk clock disabling Register
#define BIT_MSK__AUD_PAR_BUSCLK_3__REG_AUD_PAR_B23_B16                                   0xFF

// Audio In I2S Channel Status #5 Register
#define REG_ADDR__I2S_CHST6                                              (REGTX_AIP | 0x0028)
// (ReadWrite, Bits 7:0)
// cbit stream bits 47:40 Please refer to SPDIF spec for detailed description.
#define BIT_MSK__I2S_CHST6__REG_CBIT_MSB_B7_B0                                    0xFF

// Audio In I2S Channel Status #6 Register
#define REG_ADDR__I2S_CHST7                                              (REGTX_AIP | 0x0029)
// (ReadWrite, Bits 7:0)
// cbit stream bits 55:48 Please refer to SPDIF spec for detailed description.
#define BIT_MSK__I2S_CHST7__REG_CBIT_MSB_B15_B8                                   0xFF

// DSD INTERLEAVE Register
#define REG_ADDR__DSD_INTERLEAVE                                         (REGTX_AIP | 0x002A)
// (ReadWrite, Bits 7:0)
// parallel audio settings
#define BIT_MSK__DSD_INTERLEAVE__REG_DSD_INTERLEAVE_B7_B0                              0xFF

// AUDIO_PAR_MODE_SEL Register
#define REG_ADDR__AUDIO_PAR_MODE_SEL                                     (REGTX_AIP | 0x002B)
// (ReadWrite, Bits 2:0)
// parallel audio settings
#define BIT_MSK__AUDIO_PAR_MODE_SEL__REG_AUDIO_PAR_MODE_B2_B0                              0x07

// AUDIO_RST Register
#define REG_ADDR__AIP_RST                                                (REGTX_AIP | 0x002C)
// (ReadWrite, Bits 3)
// TDM interface software reset
#define BIT_MSK__AIP_RST__REG_RST4AUDIO_TDM                                     0x08
// (ReadWrite, Bits 2)
// software reset for ACR - will reset only ACR logic
#define BIT_MSK__AIP_RST__REG_RST4AUDIO_ACR                                     0x04
// (ReadWrite, Bits 1)
// software reset for aip fifos - will reset only Audio FIFO
#define BIT_MSK__AIP_RST__REG_RST4AUDIO_FIFO                                    0x02
// (ReadWrite, Bits 0)
// software reset for aip - will reset every piece of the AIP logic
#define BIT_MSK__AIP_RST__REG_RST4AUDIO                                         0x01

// AUDIO HDMI2MHL Register
#define REG_ADDR__AIP_HDMI2MHL                                           (REGTX_AIP | 0x002D)
// (ReadWrite, Bits 7:3)
// bit #3 - if 1 then output from AFIFO will be regestered bit #4 - if 1 then logic will identify HBRA packet ID based on the mode, since in HDMI vs MHL HBRA has different header. Otherwise logic will set internal HBRA flag if header is 0x09 (HDMI mode)
#define BIT_MSK__AIP_HDMI2MHL__REG_AFIFO_TEST                                        0xF8
// (ReadWrite, Bits 2)
// enable to convert HBRA audio packet from HDMI to MHL or from MHL to HDMI
#define BIT_MSK__AIP_HDMI2MHL__REG_HBAC_EN                                           0x04
// (ReadWrite, Bits 1)
// Indicates input mode: 0 - HDMI, 1 - MHL
#define BIT_MSK__AIP_HDMI2MHL__REG_MODE_IN                                           0x02
// (ReadWrite, Bits 0)
// Indicates output mode: 0 - HDMI, 1 - MHL
#define BIT_MSK__AIP_HDMI2MHL__REG_MODE_OUT                                          0x01

// Audio In TDM Control Register
#define REG_ADDR__TDM_CTRL                                               (REGTX_AIP | 0x002E)
// (ReadWrite, Bits 7:6)
// Number of cycles between incoming WS/FS and data. Zero means the data is valid starting from the same cycle WS is active 00: 0 cycle 01: 1 cycle 10: 2 cycles 11: 3 cycles
#define BIT_MSK__TDM_CTRL__REG_TDM_WS_IN_DELAY                                   0xC0
// (ReadWrite, Bits 5)
// Select on which edge of the WS to shift in TDM data sample 0: Disabled, i.e. use posedge (default) 1: Enabled, i.e. use negedge
#define BIT_MSK__TDM_CTRL__REG_TDM_WS_NEGEDGE                                    0x20
// (ReadWrite, Bits 4)
// TDM Incoming Data Block Size 0: 32-bit (default) 1: 16-bit
#define BIT_MSK__TDM_CTRL__REG_TDM_16_BIT_BLK                                    0x10
// (ReadWrite, Bits 3)
// Sampling edge of the TDM data: 1 means that TDM data will be sampled on neg edge
#define BIT_MSK__TDM_CTRL__REG_TDM_CK_PHASE                                      0x08
// (ReadWrite, Bits 2:1)
// TDM Channel select: 00: 2 channel input (no clock divider) 01: 4 channel input (/2 clock divider) 10: 6 channel input (/3 clock divider) 11: 8 channel input (/4 clock divider)
#define BIT_MSK__TDM_CTRL__REG_TDM_CH                                            0x06
// (ReadWrite, Bits 0)
// TDM enable
#define BIT_MSK__TDM_CTRL__REG_TDM_EN                                            0x01

// Audio PATH Control Register
#define REG_ADDR__AUDP_TXCTRL                                            (REGTX_AIP | 0x002F)
// (ReadWrite, Bits 7)
// Mute Audio in means of no data from interfaces inserted into FIFOs controlled by TPI
#define BIT_MSK__AUDP_TXCTRL__REG_AUD_MUTE_EN                                       0x80
// (ReadWrite, Bits 1)
// HDMI Audio Packet layout indicator: 0 - Layout 0 (2-channel) (default) 1 - Layout 1 (Up to 8-channel)
#define BIT_MSK__AUDP_TXCTRL__REG_LAYOUT                                            0x02

// Audio PATH Tx FIFO Read Write ptr difference Register
#define REG_ADDR__AUDP_TXFIFO                                            (REGTX_AIP | 0x0031)
// (ReadOnly, Bits 5:0)
// Difference between read and write pointers
#define BIT_MSK__AUDP_TXFIFO__HDMI_FIFO_DIFF                                        0x3F

// Reg Access Time-out Register
#define REG_ADDR__AIP_REG_ACC_TO                                         (REGTX_AIP | 0x0032)
// (ReadWrite, Bits 7:0)
// Register access time-out value bits 9:2. LSB tight to 0. If client does not respond within this time frame then flag will be set.
#define BIT_MSK__AIP_REG_ACC_TO__REG_TO_LIMIT                                          0xFF

// TDM Channel data mapping Register
#define REG_ADDR__AIP_TDM_CH_MAP                                         (REGTX_AIP | 0x0033)
// (ReadWrite, Bits 7:6)
// Map which TDM data will go out of SD3:  00 - sample 0 and 1 01 - sample 2 and 3 10 - sample 4 and 5 11 - sample 6 and 7
#define BIT_MSK__AIP_TDM_CH_MAP__REG_TDM_CH3_MAP                                       0xC0
// (ReadWrite, Bits 5:4)
// Map which TDM data will go out of SD2:  00 - sample 0 and 1 01 - sample 2 and 3 10 - sample 4 and 5 11 - sample 6 and 7
#define BIT_MSK__AIP_TDM_CH_MAP__REG_TDM_CH2_MAP                                       0x30
// (ReadWrite, Bits 3:2)
// Map which TDM data will go out of SD1:  00 - sample 0 and 1 01 - sample 2 and 3 10 - sample 4 and 5 11 - sample 6 and 7
#define BIT_MSK__AIP_TDM_CH_MAP__REG_TDM_CH1_MAP                                       0x0C
// (ReadWrite, Bits 1:0)
// Map which TDM data will go out of SD0:  00 - sample 0 and 1 01 - sample 2 and 3 10 - sample 4 and 5 11 - sample 6 and 7
#define BIT_MSK__AIP_TDM_CH_MAP__REG_TDM_CH0_MAP                                       0x03

// TDM Channel data mapping Register
#define REG_ADDR__AIP_TDM_CTRL_4                                         (REGTX_AIP | 0x0034)
// (ReadWrite, Bits 0)
// delay TDM WS out: 1 delay
#define BIT_MSK__AIP_TDM_CTRL_4__REG_TDM_DEL_WS_OUT                                    0x01

// Interrupt State Register
#define REG_ADDR__AIP_INTR_STATE                                         (REGTX_AIP | 0x004E)
// (ReadOnly, Bits 1)
// status of the interrupt from ADAM
#define BIT_MSK__AIP_INTR_STATE__O_ADMA_INTR                                           0x02
// (ReadOnly, Bits 0)
// Interrupt state bit value.  It shows whether the interrupt is active or not.  It is one gate before the polarity is applied to the interrupt.  In other words; whenever the interrupt is asserted; this bit is high.
#define BIT_MSK__AIP_INTR_STATE__INTR_STATE                                            0x01

// Interrupt Source #1 Register
#define REG_ADDR__AIP_INTR1                                              (REGTX_AIP | 0x004F)
// (ReadWrite, Bits 4)
// New preamble forced to drop sample. Asserted if set to 1. Write '1' to clear this bit.
#define BIT_MSK__AIP_INTR1__REG_INTR1_STAT4                                       0x10
// (ReadWrite, Bits 3)
// Input SPDIF stream had bi-phase error. Asserted if set to 1. Write '1' to clear this bit.
#define BIT_MSK__AIP_INTR1__REG_INTR1_STAT3                                       0x08
// (ReadWrite, Bits 1)
// Audio FIFO overflow. Asserted if set to 1. Write '1' to clear this bit.
#define BIT_MSK__AIP_INTR1__REG_INTR1_STAT1                                       0x02
// (ReadWrite, Bits 0)
// Audio FIFO underflow. Asserted if set to 1. Writing 1 into this bit would clear it. '0' - wont change anything
#define BIT_MSK__AIP_INTR1__REG_INTR1_STAT0                                       0x01

// Interrupt Source #2 Register
#define REG_ADDR__AIP_INTR2                                              (REGTX_AIP | 0x0050)
// (ReadWrite, Bits 6)
// SPDIF parity error. Write 1 to clear
#define BIT_MSK__AIP_INTR2__REG_INTR2_STAT6                                       0x40
// (ReadWrite, Bits 4)
// Did not found expected preamble. Asserted if set to 1. Write '1' to clear this bit.
#define BIT_MSK__AIP_INTR2__REG_INTR2_STAT4                                       0x10
// (ReadWrite, Bits 3)
// ACR CTS Changed.  Asserted if set to 1. Write 1 to clear
#define BIT_MSK__AIP_INTR2__REG_INTR2_STAT3                                       0x08
// (ReadWrite, Bits 2)
// ACR packet overwrite occurred.  Asserted if set to 1. Write 1 to clear
#define BIT_MSK__AIP_INTR2__REG_INTR2_STAT2                                       0x04
// (ReadWrite, Bits 1)
// ACR CTS # is recalculating.  Asserted if set to 1. Write 1 to clear
#define BIT_MSK__AIP_INTR2__REG_INTR2_STAT1                                       0x02
// (ReadWrite, Bits 0)
// DSD stream got invalid sequence: more then 24 bits of the same value. Asserted if set to 1. Write 1 to clear
#define BIT_MSK__AIP_INTR2__REG_INTR2_STAT0                                       0x01

// Interrupt #1 Mask Register
#define REG_ADDR__AIP_INTR1_MASK                                         (REGTX_AIP | 0x0051)
// (ReadWrite, Bits 7)
// Enable INT1[7]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR1_MASK__REG_INTR1_MASK7                                       0x80
// (ReadWrite, Bits 6)
// Enable INT1[6]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR1_MASK__REG_INTR1_MASK6                                       0x40
// (ReadWrite, Bits 5)
// Enable INT1[5]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR1_MASK__REG_INTR1_MASK5                                       0x20
// (ReadWrite, Bits 4)
// Enable INT1[4]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR1_MASK__REG_INTR1_MASK4                                       0x10
// (ReadWrite, Bits 3)
// Enable INT1[3]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR1_MASK__REG_INTR1_MASK3                                       0x08
// (ReadWrite, Bits 2)
// Enable INT1[2]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR1_MASK__REG_INTR1_MASK2                                       0x04
// (ReadWrite, Bits 1)
// Enable INT1[1]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR1_MASK__REG_INTR1_MASK1                                       0x02
// (ReadWrite, Bits 0)
// Enable INT1[0]: 1 - enable; 0 - disable (defualt)
#define BIT_MSK__AIP_INTR1_MASK__REG_INTR1_MASK0                                       0x01

// Interrupt #2 Mask Register
#define REG_ADDR__AIP_INTR2_MASK                                         (REGTX_AIP | 0x0052)
// (ReadWrite, Bits 7)
// Enable INTR2[7]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR2_MASK__REG_INTR2_MASK7                                       0x80
// (ReadWrite, Bits 6)
// Enable INTR2[6]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR2_MASK__REG_INTR2_MASK6                                       0x40
// (ReadWrite, Bits 5)
// Enable INTR2[5]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR2_MASK__REG_INTR2_MASK5                                       0x20
// (ReadWrite, Bits 4)
// Enable INTR2[4]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR2_MASK__REG_INTR2_MASK4                                       0x10
// (ReadWrite, Bits 3)
// Enable INTR2[3]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR2_MASK__REG_INTR2_MASK3                                       0x08
// (ReadWrite, Bits 2)
// Enable INTR2[2]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR2_MASK__REG_INTR2_MASK2                                       0x04
// (ReadWrite, Bits 1)
// Enable INTR2[1]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR2_MASK__REG_INTR2_MASK1                                       0x02
// (ReadWrite, Bits 0)
// Enable INTR2[0]: 1 - enable; 0 - disable (default)
#define BIT_MSK__AIP_INTR2_MASK__REG_INTR2_MASK0                                       0x01

// TPI Audio Down Sample Control Register
#define REG_ADDR__TPI_DOWN_SMPL_CTRL                                     (REGTX_AIP | 0x0061)
// (ReadWrite, Bits 2)
// TPI Audio Lookup Tables Enable 0  Not Enabled 1  Enabled (default)
#define BIT_MSK__TPI_DOWN_SMPL_CTRL__REG_TPI_AUDIO_LOOKUP_EN                               0x04
// (ReadWrite, Bits 1:0)
// S/PDIF Audio Handling. 0b00  Block any audio except audio indicating fs as 32  kHz 44.1 kHz or 48 kHz. 0b01  Same as 0b11 (default for Atmel) 0b10  Down sample incoming audio as needed fs = 32 kHz -] fs = 32 kHz fs = 44.1 kHz -] fs = 44.1 kHz fs = 48 kHz -] fs = 48 kHz fs = 88.2 kHz -] fs = 44.1 kHz fs = 96 kHz -] fs = 48 kHz fs = 176.4 kHz -] fs = 44.1 kHz fs = 192 kHz -] fs = 48 kHz Note: This register will not affect NonPCM streams Note: fS value in the audio status channel is corrected automatically. 0b11  Pass any audio stream without fs modification. (default for SST)  I2S Audio Handling. 0b00 0b01 0b11 - Pass any audio stream without fs modification. 0b10  Down sample incoming audio as needed fs = 32 kHz -] fs = 32 kHz fs = 44.1 kHz -] fs = 44.1 kHz fs = 48 kHz -] fs = 48 kHz fs = 88.2 kHz -] fs = 44.1 kHz fs = 96 kHz -] fs = 48 kHz fs = 176.4 kHz -] fs = 44.1 kHz fs = 192 kHz -] fs = 48 kHz
#define BIT_MSK__TPI_DOWN_SMPL_CTRL__REG_TPI_AUD_HNDL                                      0x03

// TPI Audio Config Register
#define REG_ADDR__TPI_AUD_CONFIG                                         (REGTX_AIP | 0x0062)
// (ReadWrite, Bits 7:6)
// 00=Refer to Stream Header 01=16-bit 10=20-bit 11=24-bit
#define BIT_MSK__TPI_AUD_CONFIG__REG_TPI_SPDIF_SAMPLE_SIZE                             0xC0
// (ReadWrite, Bits 5)
// reserved
#define BIT_MSK__TPI_AUD_CONFIG__RSVD                                                  0x20
// (ReadWrite, Bits 4)
// 0  Normal (clears reg_aud_mute_en) 1  Mute (set reg_aud_mute_en)
#define BIT_MSK__TPI_AUD_CONFIG__REG_TPI_AUD_MUTE                                      0x10

// TPI Audio Sample Size/Frequency Register
#define REG_ADDR__TPI_AUD_FS                                             (REGTX_AIP | 0x0063)
// (ReadWrite, Bits 7)
// Enable for the TPI Sample Frequency override 1 - Use value in reg_tpi_aud_sf 0x63[5:0] 0 - Use incoming stream
#define BIT_MSK__TPI_AUD_FS__REG_TPI_AUD_SF_OVRD                                   0x80
// (ReadWrite, Bits 5:0)
// Audio Sample Frequency hbra_on, aud_sample_freq[5:0];  Audio Rate 1b0, 2b00, 4'b0100; 22.05 kHz (n/a) 1b0, 2b00, 4'b0000; 44.1 kHz 1b0, 2b00, 4'b1000; 88.2 kHz 1b0, 2b00, 4'b1100; 176.4 kHz 1b0, 2b00, 4b1101; 352.8 kHz 1b0, 2b10, 4b1101; 705.6 kHz 1b0, 2b01, 4b1101; 1411.2 kHz (n/a) 1b0, 2b00, 4'b0110; 24 kHz (n/a) 1b0, 2b00, 4'b0010; 48 kHz 1b0, 2b00, 4'b1010; 96 kHz 1b0, 2b00, 4'b1110; 192 kHz 1b0, 2b00; 4b0101; 384 kHz 1b0, 2b00, 4b1001; 768 kHz 1b0, 2b01; 4b0101; 1536 kHz (n/a) 1b0, 2b00, 4'b0011; 32 kHz 1b0, 2b00, 4b1011; 64 kHz 1b0, 2b10, 4b1011; 128 kHz 1b0, 2b01, 4b1011; 256 kHz 1b0, 2b11, 4b1011; 512 kHz 1b0, 2b11; 4b0101; 1024 kHz (n/a) 1b1, 2b00, 4'b1100; HBRA 176.4 kHz 1b1, 2b00, 4'b1101; HBRA 352.8 kHz 1b1, 2b10, 4'b1101; HBRA 705.6 kHz 1b1, 2b01, 4b1101; HBRA 1411.2 kHz 1b1, 2b00, 4'b1110; HBRA 192 kHz 1b1, 2b00, 4'b0101; HBRA 384 kHz 1b1, 2b00, 4'b1001; HBRA 768 kHz 1b1, 2b01; 4b0101; HBRA 1536 kHz 1b1, 2b10, 4'b1011; HBRA 128 kHz 1b1, 2b01, 4b1011; HBRA 256 kHz 1b1, 2b11, 4b1011; HBRA 512 kHz 1b1, 2b11, 4b0101; HBRA 1024 kHz
#define BIT_MSK__TPI_AUD_FS__REG_TPI_AUD_SF_B5_B0                                  0x3F

//***************************************************************************
// REGTX_CEC. Address: 40
// CEC Device ID Register
#define REG_ADDR__CEC_DEVICE_ID                                          (REGTX_CEC | 0x0080)
// (ReadOnly, Bits 7:0)
// Device ID  0xCC (CEC)
#define BIT_MSK__CEC_DEVICE_ID__CEC_DEVICEID                                          0xFF

// CEC Spec Register
#define REG_ADDR__CEC_SPEC                                               (REGTX_CEC | 0x0081)
// (ReadOnly, Bits 7:4)
// CEC spec major release
#define BIT_MSK__CEC_SPEC__CEC_RELEASE                                           0xF0
// (ReadOnly, Bits 3:0)
// CEC spec minor release
#define BIT_MSK__CEC_SPEC__CEC_REVISION                                          0x0F

// CEC Spec suffix Register
#define REG_ADDR__CEC_SPEC_SUFFIX                                        (REGTX_CEC | 0x0082)
// (ReadOnly, Bits 7)
// Subsytem 0 = Firmware 1 = Hardware
#define BIT_MSK__CEC_SPEC_SUFFIX__CEC_SUBSYSTEM                                         0x80
// (ReadOnly, Bits 3:0)
// CEC spec suffix  0x0 (= a; for rev 1.2a)
#define BIT_MSK__CEC_SPEC_SUFFIX__CEC_SPEC_SUFFIX                                       0x0F

// CEC Firmware Rev Register (CPI Revision)
#define REG_ADDR__CEC_HARDWARE_REV                                       (REGTX_CEC | 0x0083)
// (ReadOnly, Bits 7:0)
// Hardware Revision
#define BIT_MSK__CEC_HARDWARE_REV__CEC_HARD_REVISION                                     0xFF

// CEC Debug 0 Register
#define REG_ADDR__CEC_DEBUG_0                                            (REGTX_CEC | 0x0084)
// (ReadOnly, Bits 7:0)
// Start Bit Low Period.  Measured in units of 250us.  (RO) Expected range is 3.5ms (0x0E)  to 3.9ms (0x0F).
#define BIT_MSK__CEC_DEBUG_0__CEC_START_LOW_PRIOD                                   0xFF

// CEC Debug 1 Register
#define REG_ADDR__CEC_DEBUG_1                                            (REGTX_CEC | 0x0085)
// (ReadOnly, Bits 7:0)
// Start Bit Duration Period.  Measured in units of 250us.  (RO) Expected range is 4.3ms (0x11)  to 4.7ms (0x12).
#define BIT_MSK__CEC_DEBUG_1__CEC_START_BIT_PRIOD                                   0xFF

// CEC Debug 2 Register
#define REG_ADDR__CEC_DEBUG_2                                            (REGTX_CEC | 0x0086)
// (ReadWrite, Bits 7:4)
// CEC Snoop Initiator (RW)    Values 0000 to 1111.
#define BIT_MSK__CEC_DEBUG_2__CEC_SNOOP_INITIATOR                                   0xF0
// (ReadOnly, Bits 3:0)
// Current CEC bus owner (RO)   Values 0000 to 1111.
#define BIT_MSK__CEC_DEBUG_2__CEC_BUS_OWNER                                         0x0F

// CEC Debug 3 Register
#define REG_ADDR__CEC_DEBUG_3                                            (REGTX_CEC | 0x0087)
// (ReadWrite, Bits 7)
// Flush Tx FIFO 0=No 1=Yes Self resetting bit
#define BIT_MSK__CEC_DEBUG_3__CEC_REG_FLUSH_TX_FF                                   0x80
// (ReadOnly, Bits 6:4)
// Frame Retransmit Count Values 0 to 5
#define BIT_MSK__CEC_DEBUG_3__CEC_CTL_RETRY_CNT                                     0x70
// (ReadWrite, Bits 2)
// Invert ACK to Broadcast Commands 0 = No 1 = Yes
#define BIT_MSK__CEC_DEBUG_3__CEC_INV_ACK_BRCST                                     0x04
// (ReadWrite, Bits 1)
// ACK/NACK Header Block 0 = ACK 1 = NACK
#define BIT_MSK__CEC_DEBUG_3__CEC_REG_NACK_HDR                                      0x02
// (ReadWrite, Bits 0)
// CEC snoop 0 = disable 1 = enable
#define BIT_MSK__CEC_DEBUG_3__CEC_REG_SNOOP                                         0x01

// CEC_TX_INIT Register
#define REG_ADDR__CEC_TX_INIT                                            (REGTX_CEC | 0x0088)
// (ReadWrite, Bits 3:0)
// CEC_INIT_ID [3:0] CEC Initiator ID  needs to be written to identify this device in future transmissions.
#define BIT_MSK__CEC_TX_INIT__CUR_CEC_INIT_ID                                       0x0F

// CEC_TX_DEST Register
#define REG_ADDR__CEC_TX_DEST                                            (REGTX_CEC | 0x0089)
// (ReadWrite, Bits 7)
// SEND_ POLL Generate a polling message 0=No 1=yes Self resetting bit
#define BIT_MSK__CEC_TX_DEST__CEC_REG_SD_POLL_INTERN                                0x80
// (ReadWrite, Bits 3:0)
// CEC_DEST_ID [3:0] CEC Destination ID  identifies the target device for the command.  Must be written before writing the corresponding CEC command.
#define BIT_MSK__CEC_TX_DEST__CUR_READ_INIT_ID                                      0x0F

// CEC CPI Config Register
#define REG_ADDR__CEC_CONFIG_CPI                                         (REGTX_CEC | 0x008E)
// (ReadWrite, Bits 4)
// CEC pass through enable
#define BIT_MSK__CEC_CONFIG_CPI__CEC_REG_I2C_CEC_PASSTHRU                              0x10
// (ReadWrite, Bits 2)
// Sil_Internal(Force CEC non-calibration mode. (default 1))
#define BIT_MSK__CEC_CONFIG_CPI__CEC_REG_FORCE_NON_CALIB                               0x04
// (ReadWrite, Bits 1)
// CEC calibration enable; self clear next cycle
#define BIT_MSK__CEC_CONFIG_CPI__CEC_REG_CALIB_CEC_EN                                  0x02
// (ReadWrite, Bits 0)
// CEC calibration start
#define BIT_MSK__CEC_CONFIG_CPI__CEC_REG_CALIB_CEC                                     0x01

// CEC_TX_COMMAND Register
#define REG_ADDR__CEC_TX_COMMAND                                         (REGTX_CEC | 0x008F)
// (ReadWrite, Bits 7:0)
// CEC_TX_COMMAND ( The Command and operands all are (total 16) considered as a single fifo entry and fifo has the depth of 2)
#define BIT_MSK__CEC_TX_COMMAND__TX_FIFOX_B00                                          0xFF

// CEC_TX_OPERAND 0 Register
#define REG_ADDR__CEC_TX_OPERAND_0                                       (REGTX_CEC | 0x0090)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[0]
#define BIT_MSK__CEC_TX_OPERAND_0__TX_FIFOX_B01                                          0xFF

// CEC_TX_OPERAND 1 Register
#define REG_ADDR__CEC_TX_OPERAND_1                                       (REGTX_CEC | 0x0091)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[1]
#define BIT_MSK__CEC_TX_OPERAND_1__TX_FIFOX_B02                                          0xFF

// CEC_TX_OPERAND 2 Register
#define REG_ADDR__CEC_TX_OPERAND_2                                       (REGTX_CEC | 0x0092)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[2]
#define BIT_MSK__CEC_TX_OPERAND_2__TX_FIFOX_B03                                          0xFF

// CEC_TX_OPERAND 3 Register
#define REG_ADDR__CEC_TX_OPERAND_3                                       (REGTX_CEC | 0x0093)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[3]
#define BIT_MSK__CEC_TX_OPERAND_3__TX_FIFOX_B04                                          0xFF

// CEC_TX_OPERAND 4 Register
#define REG_ADDR__CEC_TX_OPERAND_4                                       (REGTX_CEC | 0x0094)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[4]
#define BIT_MSK__CEC_TX_OPERAND_4__TX_FIFOX_B05                                          0xFF

// CEC_TX_OPERAND 5 Register
#define REG_ADDR__CEC_TX_OPERAND_5                                       (REGTX_CEC | 0x0095)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[5]
#define BIT_MSK__CEC_TX_OPERAND_5__TX_FIFOX_B06                                          0xFF

// CEC_TX_OPERAND 6 Register
#define REG_ADDR__CEC_TX_OPERAND_6                                       (REGTX_CEC | 0x0096)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[6]
#define BIT_MSK__CEC_TX_OPERAND_6__TX_FIFOX_B07                                          0xFF

// CEC_TX_OPERAND 7 Register
#define REG_ADDR__CEC_TX_OPERAND_7                                       (REGTX_CEC | 0x0097)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[7]
#define BIT_MSK__CEC_TX_OPERAND_7__TX_FIFOX_B08                                          0xFF

// CEC_TX_OPERAND 8 Register
#define REG_ADDR__CEC_TX_OPERAND_8                                       (REGTX_CEC | 0x0098)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[8]
#define BIT_MSK__CEC_TX_OPERAND_8__TX_FIFOX_B09                                          0xFF

// CEC_TX_OPERAND 9 Register
#define REG_ADDR__CEC_TX_OPERAND_9                                       (REGTX_CEC | 0x0099)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[9]
#define BIT_MSK__CEC_TX_OPERAND_9__TX_FIFOX_B10                                          0xFF

// CEC_TX_OPERAND 10 Register
#define REG_ADDR__CEC_TX_OPERAND_10                                      (REGTX_CEC | 0x009A)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[10]
#define BIT_MSK__CEC_TX_OPERAND_10__TX_FIFOX_B11                                          0xFF

// CEC_TX_OPERAND 11 Register
#define REG_ADDR__CEC_TX_OPERAND_11                                      (REGTX_CEC | 0x009B)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[11]
#define BIT_MSK__CEC_TX_OPERAND_11__TX_FIFOX_B12                                          0xFF

// CEC_TX_OPERAND 12 Register
#define REG_ADDR__CEC_TX_OPERAND_12                                      (REGTX_CEC | 0x009C)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[12]
#define BIT_MSK__CEC_TX_OPERAND_12__TX_FIFOX_B13                                          0xFF

// CEC_TX_OPERAND 13 Register
#define REG_ADDR__CEC_TX_OPERAND_13                                      (REGTX_CEC | 0x009D)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[13]
#define BIT_MSK__CEC_TX_OPERAND_13__TX_FIFOX_B14                                          0xFF

// CEC_TX_OPERAND 14 Register
#define REG_ADDR__CEC_TX_OPERAND_14                                      (REGTX_CEC | 0x009E)
// (ReadWrite, Bits 7:0)
// CEC_TX_OPERAND[14]
#define BIT_MSK__CEC_TX_OPERAND_14__TX_FIFOX_B15                                          0xFF

// CEC Transmit Data Register
#define REG_ADDR__CEC_TRANSMIT_DATA                                      (REGTX_CEC | 0x009F)
// (ReadWrite, Bits 6)
// TX_BFR_ACCESS  Read back internal buffer contents from 0x8F-0x9E  0 = No 1 = Yes
#define BIT_MSK__CEC_TRANSMIT_DATA__CEC_REG_TX_BFR_AC                                     0x40
// (ReadWrite, Bits 5)
// TX_AUTO_CALC Auto-Calculate TX_CNT; and Send 0 = No 1 = Yes
#define BIT_MSK__CEC_TRANSMIT_DATA__CEC_REG_TX_AUTO_CALC                                  0x20
// (WriteOnly, Bits 4)
// TRANSMIT_CEC_CMD Send CEC Command and TX_CNT Operands  (Write only) 0 = No 1 = Yes
#define BIT_MSK__CEC_TRANSMIT_DATA__MANUAL_CMD_SET                                        0x10
// (ReadWrite, Bits 3:0)
// TX_CNT [3:0]  Transmit Byte Count  selects the number of CEC_TX_OPERAND bytes to send with the command.  0000 = No operands 0001 = one operand .. 1111 = 15 operands
#define BIT_MSK__CEC_TRANSMIT_DATA__CEC_REG_TX_CMD_CNT                                    0x0F

// CEC_Retry_Limit Register
#define REG_ADDR__CEC_RETRY_LIMIT                                        (REGTX_CEC | 0x00A0)
// (ReadWrite, Bits 2:0)
// TX number of retry count limit when NAK received The number in this register is for how many more retries.  Ex. TX_RETRY_LIMIT =4 means total 5 times of transmission try on the bus.
#define BIT_MSK__CEC_RETRY_LIMIT__CEC_REG_TX_RETRY_LIMIT                                0x07

// CEC_CAPTURE_ID0 Register
#define REG_ADDR__CEC_CAPTURE_ID0                                        (REGTX_CEC | 0x00A2)
// (ReadWrite, Bits 7:0)
// The CEC Capture ID register is separate from the CEC Initiator ID.  It selects the received commands that will be captured in the receive FIFO and acknowledged.  If the command destination matches one of the bits set in this register or is a Broadcast cyc
#define BIT_MSK__CEC_CAPTURE_ID0__CEC_REG_CAPTURE_ID_B7_B0                              0xFF

// CEC_CAPTURE_ID0 Register
#define REG_ADDR__CEC_CAPTURE_ID1                                        (REGTX_CEC | 0x00A3)
// (ReadWrite, Bits 7:0)
// The CEC Capture ID register is separate from the CEC Initiator ID.  It selects the received commands that will be captured in the receive FIFO and acknowledged.  If the command destination matches one of the bits set in this register or is a Broadcast cyc
#define BIT_MSK__CEC_CAPTURE_ID1__CEC_REG_CAPTURE_ID_B15_B8                             0xFF

// CEC_INT_ENABLE 0 Register
#define REG_ADDR__CEC_INT_ENABLE_0                                       (REGTX_CEC | 0x00A4)
// (ReadWrite, Bits 6)
// Tx: Transmit Buffer Full event 0=Disable 1=Enable
#define BIT_MSK__CEC_INT_ENABLE_0__INTRP_TX_FIFO_FULL_EN                                 0x40
// (ReadWrite, Bits 5)
// Tx: Transmit Buffer Full/Empty Change event 0=Disable 1=Enable
#define BIT_MSK__CEC_INT_ENABLE_0__INTRP_TX_FF_CSTATE_EN                                 0x20
// (ReadWrite, Bits 2)
// Transmitter FIFO Empty event 0=Disable 1=Enable
#define BIT_MSK__CEC_INT_ENABLE_0__INTRP_TX_FIFO_EMPTY_EN                                0x04
// (ReadWrite, Bits 1)
// Receiver FIFO Not Empty event 0=Disable 1=Enable
#define BIT_MSK__CEC_INT_ENABLE_0__INTRP_RX_FIFO_NEMPTY_EN                               0x02
// (ReadWrite, Bits 0)
// Command Being Received event  0=Disable 1=Enable
#define BIT_MSK__CEC_INT_ENABLE_0__INTRP_RX_CMD_EN                                       0x01

// CEC_INT_ENABLE 1 Register
#define REG_ADDR__CEC_INT_ENABLE_1                                       (REGTX_CEC | 0x00A5)
// (ReadWrite, Bits 4)
// intrp_low_bit_min_err_en: 1 enable
#define BIT_MSK__CEC_INT_ENABLE_1__INTRP_LOW_BIT_MIN_ERR_EN                              0x10
// (ReadWrite, Bits 3)
// Rx FIFO Overrun Error event 0=Disable 1=Enable
#define BIT_MSK__CEC_INT_ENABLE_1__INTRP_RX_FIFO_OVRUN_EN                                0x08
// (ReadWrite, Bits 2)
// Short Pulse Detected event 0=Disable 1=Enable
#define BIT_MSK__CEC_INT_ENABLE_1__INTRP_SHRT_PULSE_DT_EN                                0x04
// (ReadWrite, Bits 1)
// Frame Retransmit Count Exceeded event 0=Disable 1=Enable
#define BIT_MSK__CEC_INT_ENABLE_1__INTRP_FRME_RETX_CNT_EN                                0x02
// (ReadWrite, Bits 0)
// Start Bit Irregularity event 0=Disable 1=Enable
#define BIT_MSK__CEC_INT_ENABLE_1__INTRP_START_ODD_BIT_EN                                0x01

// CEC_INT_STATUS 0 Register
#define REG_ADDR__CEC_INT_STATUS_0                                       (REGTX_CEC | 0x00A6)
// (ReadOnly, Bits 7)
// CEC line current state (RO) 0 = Low 1 = High
#define BIT_MSK__CEC_INT_STATUS_0__IO_CEC_AI_SYN                                         0x80
// (ReadWrite, Bits 6)
// Tx FIFO Transmit Buffer Full  0=No 1=Yes (RO)
#define BIT_MSK__CEC_INT_STATUS_0__INTRP_TX_FIFO_FULL                                    0x40
// (ReadWrite, Bits 5)
// Tx: Transmit Buffer Full/Empty Change event Pending  0=no 1=yes
#define BIT_MSK__CEC_INT_STATUS_0__INTRP_TX_FF_CSTATE                                    0x20
// (ReadOnly, Bits 4)
// CEC_INT_STATUS1 register interrupt event summary 0 = no 1 = yes
#define BIT_MSK__CEC_INT_STATUS_0__CEC_INTR_A7_SUMMARY                                   0x10
// (ReadWrite, Bits 2)
// Transmitter FIFO Empty event pending 0=No 1=Yes
#define BIT_MSK__CEC_INT_STATUS_0__INTRP_TX_FIFO_EMPTY                                   0x04
// (ReadWrite, Bits 1)
// Receiver FIFO Not Empty event pending 0=No 1=Yes
#define BIT_MSK__CEC_INT_STATUS_0__INTRP_RX_FIFO_NEMPTY                                  0x02
// (ReadWrite, Bits 0)
// Command Being Received event pending 0=No 1=Yes
#define BIT_MSK__CEC_INT_STATUS_0__INTRP_RX_CMD                                          0x01

// CEC_INT_STATUS 1 Register
#define REG_ADDR__CEC_INT_STATUS_1                                       (REGTX_CEC | 0x00A7)
// (ReadWrite, Bits 4)
// intrp_low_bit_min_err: 1 - yes
#define BIT_MSK__CEC_INT_STATUS_1__INTRP_LOW_BIT_MIN_ERR                                 0x10
// (ReadWrite, Bits 3)
// Rx FIFO Overrun Error event pending 0=No 1=Yes
#define BIT_MSK__CEC_INT_STATUS_1__INTRP_RX_FIFO_OVRUN                                   0x08
// (ReadWrite, Bits 2)
// Short Pulse Detected event pending 0=No 1=Yes
#define BIT_MSK__CEC_INT_STATUS_1__INTRP_SHRT_PULSE_DT                                   0x04
// (ReadWrite, Bits 1)
// Frame Retransmit Count Exceeded event pending 0=No 1=Yes
#define BIT_MSK__CEC_INT_STATUS_1__INTRP_FRME_RETX_CNT                                   0x02
// (ReadWrite, Bits 0)
// Start Bit Irregularity event pending 0=No 1=Yes
#define BIT_MSK__CEC_INT_STATUS_1__INTRP_START_ODD_BIT                                   0x01

// CEC_RX_CONTROL Register
#define REG_ADDR__CEC_RX_CONTROL                                         (REGTX_CEC | 0x00AC)
// (WriteOnly, Bits 7)
// Write 1 to this bit clears all interrupt status bits. This is a self resetting bit. Reading this bit returns  0
#define BIT_MSK__CEC_RX_CONTROL__INTRP_CLEAR_ALL                                       0x80
// (ReadWrite, Bits 6)
// auto clear intr rx fifo mem empty
#define BIT_MSK__CEC_RX_CONTROL__AUTO_CLEAR_INTRP_RX_FIFO_NEMPTY                       0x40
// (ReadWrite, Bits 1)
// Clear All Frames from Rx FIFO. Write 1 to clear. Self-resetting bit
#define BIT_MSK__CEC_RX_CONTROL__CEC_REG_RX_CLR_ALL                                    0x02
// (ReadWrite, Bits 0)
// Clear Current Frame from Rx FIFO. Write 1 to clear. Self-resetting bit
#define BIT_MSK__CEC_RX_CONTROL__CEC_REG_RX_CLR_CUR_SET                                0x01

// CEC_RX_COUNT Register
#define REG_ADDR__CEC_RX_COUNT                                           (REGTX_CEC | 0x00AD)
// (ReadOnly, Bits 7)
// RX_ ERROR Error associated with this message  0=No 1=yes
#define BIT_MSK__CEC_RX_COUNT__CEC_RX_ERROR                                          0x80
// (ReadOnly, Bits 5:4)
// CEC_RX_CMD_CNT (RO) CEC Receive FIFO Frame Count  returns the number of frames awaiting reading in the FIFO; 0-3.
#define BIT_MSK__CEC_RX_COUNT__CEC_REG_RX_FF_WR_SEL                                  0x30
// (ReadOnly, Bits 3:0)
// CEC_RX_BYTE_CNT (RO) CEC Receive Byte Count  returns the number of operands in the current frame.
#define BIT_MSK__CEC_RX_COUNT__CEC_REG_RX_CMD_BYTE_CNT                               0x0F

// CEC_RX_CMD_HEADER Register
#define REG_ADDR__CEC_RX_CMD_HEADER                                      (REGTX_CEC | 0x00AE)
// (ReadOnly, Bits 7:4)
// CEC_RX_INIT [3:0] CEC Initiator ID  identifies  the initiator of the current frame.
#define BIT_MSK__CEC_RX_CMD_HEADER__CEC_REG_RX_CMD_HEADER_B7_B4                           0xF0
// (ReadOnly, Bits 3:0)
// CEC_RX_DEST [3:0] CEC Destination ID  identifies  the intended target of the current frame.
#define BIT_MSK__CEC_RX_CMD_HEADER__CEC_REG_RX_CMD_HEADER_B3_B0                           0x0F

// CEC_RX_COMMAND Register
#define REG_ADDR__CEC_RX_COMMAND                                         (REGTX_CEC | 0x00AF)
// (ReadOnly, Bits 7:0)
// CEC_RX_COMMAND ( The Command and operands all are (total 16) considered as a single fifo entry and fifo has the depth of 3)
#define BIT_MSK__CEC_RX_COMMAND__RX_FIFO0_B0                                           0xFF

// CEC_RX_OPERAND 0 Register
#define REG_ADDR__CEC_RX_OPERAND_0                                       (REGTX_CEC | 0x00B0)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[0]
#define BIT_MSK__CEC_RX_OPERAND_0__RX_FIFO0_B1                                           0xFF

// CEC_RX_OPERAND 1 Register
#define REG_ADDR__CEC_RX_OPERAND_1                                       (REGTX_CEC | 0x00B1)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[1]
#define BIT_MSK__CEC_RX_OPERAND_1__RX_FIFO0_B2                                           0xFF

// CEC_RX_OPERAND 2 Register
#define REG_ADDR__CEC_RX_OPERAND_2                                       (REGTX_CEC | 0x00B2)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[2]
#define BIT_MSK__CEC_RX_OPERAND_2__RX_FIFO0_B3                                           0xFF

// CEC_RX_OPERAND 3 Register
#define REG_ADDR__CEC_RX_OPERAND_3                                       (REGTX_CEC | 0x00B3)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[3]
#define BIT_MSK__CEC_RX_OPERAND_3__RX_FIFO0_B4                                           0xFF

// CEC_RX_OPERAND 4 Register
#define REG_ADDR__CEC_RX_OPERAND_4                                       (REGTX_CEC | 0x00B4)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[4]
#define BIT_MSK__CEC_RX_OPERAND_4__RX_FIFO0_B5                                           0xFF

// CEC_RX_OPERAND 5 Register
#define REG_ADDR__CEC_RX_OPERAND_5                                       (REGTX_CEC | 0x00B5)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[5]
#define BIT_MSK__CEC_RX_OPERAND_5__RX_FIFO0_B6                                           0xFF

// CEC_RX_OPERAND 6 Register
#define REG_ADDR__CEC_RX_OPERAND_6                                       (REGTX_CEC | 0x00B6)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[6]
#define BIT_MSK__CEC_RX_OPERAND_6__RX_FIFO0_B7                                           0xFF

// CEC_RX_OPERAND 7 Register
#define REG_ADDR__CEC_RX_OPERAND_7                                       (REGTX_CEC | 0x00B7)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[7]
#define BIT_MSK__CEC_RX_OPERAND_7__RX_FIFO0_B8                                           0xFF

// CEC_RX_OPERAND 8 Register
#define REG_ADDR__CEC_RX_OPERAND_8                                       (REGTX_CEC | 0x00B8)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[8]
#define BIT_MSK__CEC_RX_OPERAND_8__RX_FIFO0_B9                                           0xFF

// CEC_RX_OPERAND 9 Register
#define REG_ADDR__CEC_RX_OPERAND_9                                       (REGTX_CEC | 0x00B9)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[9]
#define BIT_MSK__CEC_RX_OPERAND_9__RX_FIFO0_B10                                          0xFF

// CEC_RX_OPERAND 10 Register
#define REG_ADDR__CEC_RX_OPERAND_10                                      (REGTX_CEC | 0x00BA)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[10]
#define BIT_MSK__CEC_RX_OPERAND_10__RX_FIFO0_B11                                          0xFF

// CEC_RX_OPERAND 11 Register
#define REG_ADDR__CEC_RX_OPERAND_11                                      (REGTX_CEC | 0x00BB)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[11]
#define BIT_MSK__CEC_RX_OPERAND_11__RX_FIFO0_B12                                          0xFF

// CEC_RX_OPERAND 12 Register
#define REG_ADDR__CEC_RX_OPERAND_12                                      (REGTX_CEC | 0x00BC)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[12]
#define BIT_MSK__CEC_RX_OPERAND_12__RX_FIFO0_B13                                          0xFF

// CEC_RX_OPERAND 13 Register
#define REG_ADDR__CEC_RX_OPERAND_13                                      (REGTX_CEC | 0x00BD)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[13]
#define BIT_MSK__CEC_RX_OPERAND_13__RX_FIFO0_B14                                          0xFF

// CEC_RX_OPERAND 14 Register
#define REG_ADDR__CEC_RX_OPERAND_14                                      (REGTX_CEC | 0x00BE)
// (ReadOnly, Bits 7:0)
// CEC_RX_OPERAND[14]
#define BIT_MSK__CEC_RX_OPERAND_14__RX_FIFO0_B15                                          0xFF

// CEC_OPCODE_ABORT 0 Register
#define REG_ADDR__CEC_OP_ABORT_0                                         (REGTX_CEC | 0x00C0)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x07 opcode received
#define BIT_MSK__CEC_OP_ABORT_0__CEC_OP_ABORT_REG_00_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x06 opcode received
#define BIT_MSK__CEC_OP_ABORT_0__CEC_OP_ABORT_REG_00_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x05 opcode received
#define BIT_MSK__CEC_OP_ABORT_0__CEC_OP_ABORT_REG_00_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x04 opcode received
#define BIT_MSK__CEC_OP_ABORT_0__CEC_OP_ABORT_REG_00_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x03 opcode received
#define BIT_MSK__CEC_OP_ABORT_0__CEC_OP_ABORT_REG_00_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x02 opcode received
#define BIT_MSK__CEC_OP_ABORT_0__CEC_OP_ABORT_REG_00_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x01 opcode received
#define BIT_MSK__CEC_OP_ABORT_0__CEC_OP_ABORT_REG_00_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x00 opcode received
#define BIT_MSK__CEC_OP_ABORT_0__CEC_OP_ABORT_REG_00_B0                                0x01

// CEC_OPCODE_ABORT 1 Register
#define REG_ADDR__CEC_OP_ABORT_1                                         (REGTX_CEC | 0x00C1)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x0F opcode received
#define BIT_MSK__CEC_OP_ABORT_1__CEC_OP_ABORT_REG_01_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x0E opcode received
#define BIT_MSK__CEC_OP_ABORT_1__CEC_OP_ABORT_REG_01_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x0D opcode received
#define BIT_MSK__CEC_OP_ABORT_1__CEC_OP_ABORT_REG_01_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x0C opcode received
#define BIT_MSK__CEC_OP_ABORT_1__CEC_OP_ABORT_REG_01_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x0B opcode received
#define BIT_MSK__CEC_OP_ABORT_1__CEC_OP_ABORT_REG_01_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x0A opcode received
#define BIT_MSK__CEC_OP_ABORT_1__CEC_OP_ABORT_REG_01_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x09 opcode received
#define BIT_MSK__CEC_OP_ABORT_1__CEC_OP_ABORT_REG_01_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x08 opcode received
#define BIT_MSK__CEC_OP_ABORT_1__CEC_OP_ABORT_REG_01_B0                                0x01

// CEC_OPCODE_ABORT 2 Register
#define REG_ADDR__CEC_OP_ABORT_2                                         (REGTX_CEC | 0x00C2)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x17 opcode received
#define BIT_MSK__CEC_OP_ABORT_2__CEC_OP_ABORT_REG_02_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x16 opcode received
#define BIT_MSK__CEC_OP_ABORT_2__CEC_OP_ABORT_REG_02_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x15 opcode received
#define BIT_MSK__CEC_OP_ABORT_2__CEC_OP_ABORT_REG_02_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x14 opcode received
#define BIT_MSK__CEC_OP_ABORT_2__CEC_OP_ABORT_REG_02_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x13 opcode received
#define BIT_MSK__CEC_OP_ABORT_2__CEC_OP_ABORT_REG_02_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x12 opcode received
#define BIT_MSK__CEC_OP_ABORT_2__CEC_OP_ABORT_REG_02_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x11 opcode received
#define BIT_MSK__CEC_OP_ABORT_2__CEC_OP_ABORT_REG_02_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x10 opcode received
#define BIT_MSK__CEC_OP_ABORT_2__CEC_OP_ABORT_REG_02_B0                                0x01

// CEC_OPCODE_ABORT 3 Register
#define REG_ADDR__CEC_OP_ABORT_3                                         (REGTX_CEC | 0x00C3)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x1F opcode received
#define BIT_MSK__CEC_OP_ABORT_3__CEC_OP_ABORT_REG_03_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x1E opcode received
#define BIT_MSK__CEC_OP_ABORT_3__CEC_OP_ABORT_REG_03_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x1D opcode received
#define BIT_MSK__CEC_OP_ABORT_3__CEC_OP_ABORT_REG_03_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x1C opcode received
#define BIT_MSK__CEC_OP_ABORT_3__CEC_OP_ABORT_REG_03_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x1B opcode received
#define BIT_MSK__CEC_OP_ABORT_3__CEC_OP_ABORT_REG_03_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x1A opcode received
#define BIT_MSK__CEC_OP_ABORT_3__CEC_OP_ABORT_REG_03_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x19 opcode received
#define BIT_MSK__CEC_OP_ABORT_3__CEC_OP_ABORT_REG_03_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x18 opcode received
#define BIT_MSK__CEC_OP_ABORT_3__CEC_OP_ABORT_REG_03_B0                                0x01

// CEC_OPCODE_ABORT 4 Register
#define REG_ADDR__CEC_OP_ABORT_4                                         (REGTX_CEC | 0x00C4)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x27 opcode received
#define BIT_MSK__CEC_OP_ABORT_4__CEC_OP_ABORT_REG_04_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x26 opcode received
#define BIT_MSK__CEC_OP_ABORT_4__CEC_OP_ABORT_REG_04_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x25 opcode received
#define BIT_MSK__CEC_OP_ABORT_4__CEC_OP_ABORT_REG_04_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x24 opcode received
#define BIT_MSK__CEC_OP_ABORT_4__CEC_OP_ABORT_REG_04_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x23 opcode received
#define BIT_MSK__CEC_OP_ABORT_4__CEC_OP_ABORT_REG_04_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x22 opcode received
#define BIT_MSK__CEC_OP_ABORT_4__CEC_OP_ABORT_REG_04_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x21 opcode received
#define BIT_MSK__CEC_OP_ABORT_4__CEC_OP_ABORT_REG_04_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x20 opcode received
#define BIT_MSK__CEC_OP_ABORT_4__CEC_OP_ABORT_REG_04_B0                                0x01

// CEC_OPCODE_ABORT 5 Register
#define REG_ADDR__CEC_OP_ABORT_5                                         (REGTX_CEC | 0x00C5)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x2F opcode received
#define BIT_MSK__CEC_OP_ABORT_5__CEC_OP_ABORT_REG_05_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x2E opcode received
#define BIT_MSK__CEC_OP_ABORT_5__CEC_OP_ABORT_REG_05_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x2D opcode received
#define BIT_MSK__CEC_OP_ABORT_5__CEC_OP_ABORT_REG_05_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x2C opcode received
#define BIT_MSK__CEC_OP_ABORT_5__CEC_OP_ABORT_REG_05_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x2B opcode received
#define BIT_MSK__CEC_OP_ABORT_5__CEC_OP_ABORT_REG_05_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x2A opcode received
#define BIT_MSK__CEC_OP_ABORT_5__CEC_OP_ABORT_REG_05_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x29 opcode received
#define BIT_MSK__CEC_OP_ABORT_5__CEC_OP_ABORT_REG_05_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x28 opcode received
#define BIT_MSK__CEC_OP_ABORT_5__CEC_OP_ABORT_REG_05_B0                                0x01

// CEC_OPCODE_ABORT 6 Register
#define REG_ADDR__CEC_OP_ABORT_6                                         (REGTX_CEC | 0x00C6)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x37 opcode received
#define BIT_MSK__CEC_OP_ABORT_6__CEC_OP_ABORT_REG_06_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x36 opcode received
#define BIT_MSK__CEC_OP_ABORT_6__CEC_OP_ABORT_REG_06_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x35 opcode received
#define BIT_MSK__CEC_OP_ABORT_6__CEC_OP_ABORT_REG_06_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x34 opcode received
#define BIT_MSK__CEC_OP_ABORT_6__CEC_OP_ABORT_REG_06_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x33 opcode received
#define BIT_MSK__CEC_OP_ABORT_6__CEC_OP_ABORT_REG_06_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x32 opcode received
#define BIT_MSK__CEC_OP_ABORT_6__CEC_OP_ABORT_REG_06_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x31 opcode received
#define BIT_MSK__CEC_OP_ABORT_6__CEC_OP_ABORT_REG_06_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x30 opcode received
#define BIT_MSK__CEC_OP_ABORT_6__CEC_OP_ABORT_REG_06_B0                                0x01

// CEC_OPCODE_ABORT 7 Register
#define REG_ADDR__CEC_OP_ABORT_7                                         (REGTX_CEC | 0x00C7)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x3F opcode received
#define BIT_MSK__CEC_OP_ABORT_7__CEC_OP_ABORT_REG_07_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x3E opcode received
#define BIT_MSK__CEC_OP_ABORT_7__CEC_OP_ABORT_REG_07_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x3D opcode received
#define BIT_MSK__CEC_OP_ABORT_7__CEC_OP_ABORT_REG_07_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x3C opcode received
#define BIT_MSK__CEC_OP_ABORT_7__CEC_OP_ABORT_REG_07_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x3B opcode received
#define BIT_MSK__CEC_OP_ABORT_7__CEC_OP_ABORT_REG_07_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x3A opcode received
#define BIT_MSK__CEC_OP_ABORT_7__CEC_OP_ABORT_REG_07_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x39 opcode received
#define BIT_MSK__CEC_OP_ABORT_7__CEC_OP_ABORT_REG_07_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x38 opcode received
#define BIT_MSK__CEC_OP_ABORT_7__CEC_OP_ABORT_REG_07_B0                                0x01

// CEC_OPCODE_ABORT 8 Register
#define REG_ADDR__CEC_OP_ABORT_8                                         (REGTX_CEC | 0x00C8)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x47 opcode received
#define BIT_MSK__CEC_OP_ABORT_8__CEC_OP_ABORT_REG_08_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x46 opcode received
#define BIT_MSK__CEC_OP_ABORT_8__CEC_OP_ABORT_REG_08_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x45 opcode received
#define BIT_MSK__CEC_OP_ABORT_8__CEC_OP_ABORT_REG_08_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x44 opcode received
#define BIT_MSK__CEC_OP_ABORT_8__CEC_OP_ABORT_REG_08_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x43 opcode received
#define BIT_MSK__CEC_OP_ABORT_8__CEC_OP_ABORT_REG_08_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x42 opcode received
#define BIT_MSK__CEC_OP_ABORT_8__CEC_OP_ABORT_REG_08_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x41 opcode received
#define BIT_MSK__CEC_OP_ABORT_8__CEC_OP_ABORT_REG_08_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x40 opcode received
#define BIT_MSK__CEC_OP_ABORT_8__CEC_OP_ABORT_REG_08_B0                                0x01

// CEC_OPCODE_ABORT 9 Register
#define REG_ADDR__CEC_OP_ABORT_9                                         (REGTX_CEC | 0x00C9)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x4F opcode received
#define BIT_MSK__CEC_OP_ABORT_9__CEC_OP_ABORT_REG_09_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x4E opcode received
#define BIT_MSK__CEC_OP_ABORT_9__CEC_OP_ABORT_REG_09_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x4D opcode received
#define BIT_MSK__CEC_OP_ABORT_9__CEC_OP_ABORT_REG_09_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x4C opcode received
#define BIT_MSK__CEC_OP_ABORT_9__CEC_OP_ABORT_REG_09_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x4B opcode received
#define BIT_MSK__CEC_OP_ABORT_9__CEC_OP_ABORT_REG_09_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x4A opcode received
#define BIT_MSK__CEC_OP_ABORT_9__CEC_OP_ABORT_REG_09_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x49 opcode received
#define BIT_MSK__CEC_OP_ABORT_9__CEC_OP_ABORT_REG_09_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x48 opcode received
#define BIT_MSK__CEC_OP_ABORT_9__CEC_OP_ABORT_REG_09_B0                                0x01

// CEC_OPCODE_ABORT 10 Register
#define REG_ADDR__CEC_OP_ABORT_10                                        (REGTX_CEC | 0x00CA)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x57 opcode received
#define BIT_MSK__CEC_OP_ABORT_10__CEC_OP_ABORT_REG_10_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x56 opcode received
#define BIT_MSK__CEC_OP_ABORT_10__CEC_OP_ABORT_REG_10_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x55 opcode received
#define BIT_MSK__CEC_OP_ABORT_10__CEC_OP_ABORT_REG_10_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x54 opcode received
#define BIT_MSK__CEC_OP_ABORT_10__CEC_OP_ABORT_REG_10_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x53 opcode received
#define BIT_MSK__CEC_OP_ABORT_10__CEC_OP_ABORT_REG_10_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x52 opcode received
#define BIT_MSK__CEC_OP_ABORT_10__CEC_OP_ABORT_REG_10_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x51 opcode received
#define BIT_MSK__CEC_OP_ABORT_10__CEC_OP_ABORT_REG_10_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x50 opcode received
#define BIT_MSK__CEC_OP_ABORT_10__CEC_OP_ABORT_REG_10_B0                                0x01

// CEC_OPCODE_ABORT 11 Register
#define REG_ADDR__CEC_OP_ABORT_11                                        (REGTX_CEC | 0x00CB)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x5F opcode received
#define BIT_MSK__CEC_OP_ABORT_11__CEC_OP_ABORT_REG_11_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x5E opcode received
#define BIT_MSK__CEC_OP_ABORT_11__CEC_OP_ABORT_REG_11_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x5D opcode received
#define BIT_MSK__CEC_OP_ABORT_11__CEC_OP_ABORT_REG_11_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x5C opcode received
#define BIT_MSK__CEC_OP_ABORT_11__CEC_OP_ABORT_REG_11_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x5B opcode received
#define BIT_MSK__CEC_OP_ABORT_11__CEC_OP_ABORT_REG_11_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x5A opcode received
#define BIT_MSK__CEC_OP_ABORT_11__CEC_OP_ABORT_REG_11_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x59 opcode received
#define BIT_MSK__CEC_OP_ABORT_11__CEC_OP_ABORT_REG_11_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x58 opcode received
#define BIT_MSK__CEC_OP_ABORT_11__CEC_OP_ABORT_REG_11_B0                                0x01

// CEC_OPCODE_ABORT 12 Register
#define REG_ADDR__CEC_OP_ABORT_12                                        (REGTX_CEC | 0x00CC)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x67 opcode received
#define BIT_MSK__CEC_OP_ABORT_12__CEC_OP_ABORT_REG_12_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x66 opcode received
#define BIT_MSK__CEC_OP_ABORT_12__CEC_OP_ABORT_REG_12_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x65 opcode received
#define BIT_MSK__CEC_OP_ABORT_12__CEC_OP_ABORT_REG_12_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x64 opcode received
#define BIT_MSK__CEC_OP_ABORT_12__CEC_OP_ABORT_REG_12_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x63 opcode received
#define BIT_MSK__CEC_OP_ABORT_12__CEC_OP_ABORT_REG_12_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x62 opcode received
#define BIT_MSK__CEC_OP_ABORT_12__CEC_OP_ABORT_REG_12_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x61 opcode received
#define BIT_MSK__CEC_OP_ABORT_12__CEC_OP_ABORT_REG_12_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x60 opcode received
#define BIT_MSK__CEC_OP_ABORT_12__CEC_OP_ABORT_REG_12_B0                                0x01

// CEC_OPCODE_ABORT 13 Register
#define REG_ADDR__CEC_OP_ABORT_13                                        (REGTX_CEC | 0x00CD)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x6F opcode received
#define BIT_MSK__CEC_OP_ABORT_13__CEC_OP_ABORT_REG_13_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x6E opcode received
#define BIT_MSK__CEC_OP_ABORT_13__CEC_OP_ABORT_REG_13_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x6D opcode received
#define BIT_MSK__CEC_OP_ABORT_13__CEC_OP_ABORT_REG_13_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x6C opcode received
#define BIT_MSK__CEC_OP_ABORT_13__CEC_OP_ABORT_REG_13_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x6B opcode received
#define BIT_MSK__CEC_OP_ABORT_13__CEC_OP_ABORT_REG_13_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x6A opcode received
#define BIT_MSK__CEC_OP_ABORT_13__CEC_OP_ABORT_REG_13_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x69 opcode received
#define BIT_MSK__CEC_OP_ABORT_13__CEC_OP_ABORT_REG_13_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x68 opcode received
#define BIT_MSK__CEC_OP_ABORT_13__CEC_OP_ABORT_REG_13_B0                                0x01

// CEC_OPCODE_ABORT 14 Register
#define REG_ADDR__CEC_OP_ABORT_14                                        (REGTX_CEC | 0x00CE)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x77 opcode received
#define BIT_MSK__CEC_OP_ABORT_14__CEC_OP_ABORT_REG_14_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x76 opcode received
#define BIT_MSK__CEC_OP_ABORT_14__CEC_OP_ABORT_REG_14_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x75 opcode received
#define BIT_MSK__CEC_OP_ABORT_14__CEC_OP_ABORT_REG_14_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x74 opcode received
#define BIT_MSK__CEC_OP_ABORT_14__CEC_OP_ABORT_REG_14_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x73 opcode received
#define BIT_MSK__CEC_OP_ABORT_14__CEC_OP_ABORT_REG_14_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x72 opcode received
#define BIT_MSK__CEC_OP_ABORT_14__CEC_OP_ABORT_REG_14_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x71 opcode received
#define BIT_MSK__CEC_OP_ABORT_14__CEC_OP_ABORT_REG_14_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x70 opcode received
#define BIT_MSK__CEC_OP_ABORT_14__CEC_OP_ABORT_REG_14_B0                                0x01

// CEC_OPCODE_ABORT 15 Register
#define REG_ADDR__CEC_OP_ABORT_15                                        (REGTX_CEC | 0x00CF)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x7F opcode received
#define BIT_MSK__CEC_OP_ABORT_15__CEC_OP_ABORT_REG_15_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x7E opcode received
#define BIT_MSK__CEC_OP_ABORT_15__CEC_OP_ABORT_REG_15_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x7D opcode received
#define BIT_MSK__CEC_OP_ABORT_15__CEC_OP_ABORT_REG_15_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x7C opcode received
#define BIT_MSK__CEC_OP_ABORT_15__CEC_OP_ABORT_REG_15_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x7B opcode received
#define BIT_MSK__CEC_OP_ABORT_15__CEC_OP_ABORT_REG_15_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x7A opcode received
#define BIT_MSK__CEC_OP_ABORT_15__CEC_OP_ABORT_REG_15_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x79 opcode received
#define BIT_MSK__CEC_OP_ABORT_15__CEC_OP_ABORT_REG_15_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x78 opcode received
#define BIT_MSK__CEC_OP_ABORT_15__CEC_OP_ABORT_REG_15_B0                                0x01

// CEC_OPCODE_ABORT 16 Register
#define REG_ADDR__CEC_OP_ABORT_16                                        (REGTX_CEC | 0x00D0)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x87 opcode received
#define BIT_MSK__CEC_OP_ABORT_16__CEC_OP_ABORT_REG_16_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x86 opcode received
#define BIT_MSK__CEC_OP_ABORT_16__CEC_OP_ABORT_REG_16_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x85 opcode received
#define BIT_MSK__CEC_OP_ABORT_16__CEC_OP_ABORT_REG_16_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x84 opcode received
#define BIT_MSK__CEC_OP_ABORT_16__CEC_OP_ABORT_REG_16_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x83 opcode received
#define BIT_MSK__CEC_OP_ABORT_16__CEC_OP_ABORT_REG_16_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x82 opcode received
#define BIT_MSK__CEC_OP_ABORT_16__CEC_OP_ABORT_REG_16_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x81 opcode received
#define BIT_MSK__CEC_OP_ABORT_16__CEC_OP_ABORT_REG_16_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x80 opcode received
#define BIT_MSK__CEC_OP_ABORT_16__CEC_OP_ABORT_REG_16_B0                                0x01

// CEC_OPCODE_ABORT 17 Register
#define REG_ADDR__CEC_OP_ABORT_17                                        (REGTX_CEC | 0x00D1)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x8F opcode received
#define BIT_MSK__CEC_OP_ABORT_17__CEC_OP_ABORT_REG_17_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x8E opcode received
#define BIT_MSK__CEC_OP_ABORT_17__CEC_OP_ABORT_REG_17_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x8D opcode received
#define BIT_MSK__CEC_OP_ABORT_17__CEC_OP_ABORT_REG_17_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x8C opcode received
#define BIT_MSK__CEC_OP_ABORT_17__CEC_OP_ABORT_REG_17_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x8B opcode received
#define BIT_MSK__CEC_OP_ABORT_17__CEC_OP_ABORT_REG_17_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x8A opcode received
#define BIT_MSK__CEC_OP_ABORT_17__CEC_OP_ABORT_REG_17_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x89 opcode received
#define BIT_MSK__CEC_OP_ABORT_17__CEC_OP_ABORT_REG_17_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x88 opcode received
#define BIT_MSK__CEC_OP_ABORT_17__CEC_OP_ABORT_REG_17_B0                                0x01

// CEC_OPCODE_ABORT 18 Register
#define REG_ADDR__CEC_OP_ABORT_18                                        (REGTX_CEC | 0x00D2)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x97 opcode received
#define BIT_MSK__CEC_OP_ABORT_18__CEC_OP_ABORT_REG_18_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x96 opcode received
#define BIT_MSK__CEC_OP_ABORT_18__CEC_OP_ABORT_REG_18_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x95 opcode received
#define BIT_MSK__CEC_OP_ABORT_18__CEC_OP_ABORT_REG_18_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x94 opcode received
#define BIT_MSK__CEC_OP_ABORT_18__CEC_OP_ABORT_REG_18_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x93 opcode received
#define BIT_MSK__CEC_OP_ABORT_18__CEC_OP_ABORT_REG_18_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x92 opcode received
#define BIT_MSK__CEC_OP_ABORT_18__CEC_OP_ABORT_REG_18_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x91 opcode received
#define BIT_MSK__CEC_OP_ABORT_18__CEC_OP_ABORT_REG_18_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x90 opcode received
#define BIT_MSK__CEC_OP_ABORT_18__CEC_OP_ABORT_REG_18_B0                                0x01

// CEC_OPCODE_ABORT 19 Register
#define REG_ADDR__CEC_OP_ABORT_19                                        (REGTX_CEC | 0x00D3)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0x9F opcode received
#define BIT_MSK__CEC_OP_ABORT_19__CEC_OP_ABORT_REG_19_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0x9E opcode received
#define BIT_MSK__CEC_OP_ABORT_19__CEC_OP_ABORT_REG_19_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0x9D opcode received
#define BIT_MSK__CEC_OP_ABORT_19__CEC_OP_ABORT_REG_19_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0x9C opcode received
#define BIT_MSK__CEC_OP_ABORT_19__CEC_OP_ABORT_REG_19_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0x9B opcode received
#define BIT_MSK__CEC_OP_ABORT_19__CEC_OP_ABORT_REG_19_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0x9A opcode received
#define BIT_MSK__CEC_OP_ABORT_19__CEC_OP_ABORT_REG_19_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0x99 opcode received
#define BIT_MSK__CEC_OP_ABORT_19__CEC_OP_ABORT_REG_19_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0x98 opcode received
#define BIT_MSK__CEC_OP_ABORT_19__CEC_OP_ABORT_REG_19_B0                                0x01

// CEC_OPCODE_ABORT 20 Register
#define REG_ADDR__CEC_OP_ABORT_20                                        (REGTX_CEC | 0x00D4)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0xA7 opcode received
#define BIT_MSK__CEC_OP_ABORT_20__CEC_OP_ABORT_REG_20_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0xA6 opcode received
#define BIT_MSK__CEC_OP_ABORT_20__CEC_OP_ABORT_REG_20_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0xA5 opcode received
#define BIT_MSK__CEC_OP_ABORT_20__CEC_OP_ABORT_REG_20_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0xA4 opcode received
#define BIT_MSK__CEC_OP_ABORT_20__CEC_OP_ABORT_REG_20_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0xA3 opcode received
#define BIT_MSK__CEC_OP_ABORT_20__CEC_OP_ABORT_REG_20_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0xA2 opcode received
#define BIT_MSK__CEC_OP_ABORT_20__CEC_OP_ABORT_REG_20_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0xA1 opcode received
#define BIT_MSK__CEC_OP_ABORT_20__CEC_OP_ABORT_REG_20_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0xA0 opcode received
#define BIT_MSK__CEC_OP_ABORT_20__CEC_OP_ABORT_REG_20_B0                                0x01

// CEC_OPCODE_ABORT 21 Register
#define REG_ADDR__CEC_OP_ABORT_21                                        (REGTX_CEC | 0x00D5)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0xAF opcode received
#define BIT_MSK__CEC_OP_ABORT_21__CEC_OP_ABORT_REG_21_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0xAE opcode received
#define BIT_MSK__CEC_OP_ABORT_21__CEC_OP_ABORT_REG_21_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0xAD opcode received
#define BIT_MSK__CEC_OP_ABORT_21__CEC_OP_ABORT_REG_21_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0xAC opcode received
#define BIT_MSK__CEC_OP_ABORT_21__CEC_OP_ABORT_REG_21_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0xAB opcode received
#define BIT_MSK__CEC_OP_ABORT_21__CEC_OP_ABORT_REG_21_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0xAA opcode received
#define BIT_MSK__CEC_OP_ABORT_21__CEC_OP_ABORT_REG_21_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0xA9 opcode received
#define BIT_MSK__CEC_OP_ABORT_21__CEC_OP_ABORT_REG_21_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0xA8 opcode received
#define BIT_MSK__CEC_OP_ABORT_21__CEC_OP_ABORT_REG_21_B0                                0x01

// CEC_OPCODE_ABORT 22 Register
#define REG_ADDR__CEC_OP_ABORT_22                                        (REGTX_CEC | 0x00D6)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0xB7 opcode received
#define BIT_MSK__CEC_OP_ABORT_22__CEC_OP_ABORT_REG_22_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0xB6 opcode received
#define BIT_MSK__CEC_OP_ABORT_22__CEC_OP_ABORT_REG_22_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0xB5 opcode received
#define BIT_MSK__CEC_OP_ABORT_22__CEC_OP_ABORT_REG_22_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0xB4 opcode received
#define BIT_MSK__CEC_OP_ABORT_22__CEC_OP_ABORT_REG_22_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0xB3 opcode received
#define BIT_MSK__CEC_OP_ABORT_22__CEC_OP_ABORT_REG_22_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0xB2 opcode received
#define BIT_MSK__CEC_OP_ABORT_22__CEC_OP_ABORT_REG_22_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0xB1 opcode received
#define BIT_MSK__CEC_OP_ABORT_22__CEC_OP_ABORT_REG_22_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0xB0 opcode received
#define BIT_MSK__CEC_OP_ABORT_22__CEC_OP_ABORT_REG_22_B0                                0x01

// CEC_OPCODE_ABORT 23 Register
#define REG_ADDR__CEC_OP_ABORT_23                                        (REGTX_CEC | 0x00D7)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0xBF opcode received
#define BIT_MSK__CEC_OP_ABORT_23__CEC_OP_ABORT_REG_23_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0xBE opcode received
#define BIT_MSK__CEC_OP_ABORT_23__CEC_OP_ABORT_REG_23_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0xBD opcode received
#define BIT_MSK__CEC_OP_ABORT_23__CEC_OP_ABORT_REG_23_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0xBC opcode received
#define BIT_MSK__CEC_OP_ABORT_23__CEC_OP_ABORT_REG_23_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0xBB opcode received
#define BIT_MSK__CEC_OP_ABORT_23__CEC_OP_ABORT_REG_23_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0xBA opcode received
#define BIT_MSK__CEC_OP_ABORT_23__CEC_OP_ABORT_REG_23_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0xB9 opcode received
#define BIT_MSK__CEC_OP_ABORT_23__CEC_OP_ABORT_REG_23_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0xB8 opcode received
#define BIT_MSK__CEC_OP_ABORT_23__CEC_OP_ABORT_REG_23_B0                                0x01

// CEC_OPCODE_ABORT 24 Register
#define REG_ADDR__CEC_OP_ABORT_24                                        (REGTX_CEC | 0x00D8)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0xC7 opcode received
#define BIT_MSK__CEC_OP_ABORT_24__CEC_OP_ABORT_REG_24_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0xC6 opcode received
#define BIT_MSK__CEC_OP_ABORT_24__CEC_OP_ABORT_REG_24_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0xC5 opcode received
#define BIT_MSK__CEC_OP_ABORT_24__CEC_OP_ABORT_REG_24_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0xC4 opcode received
#define BIT_MSK__CEC_OP_ABORT_24__CEC_OP_ABORT_REG_24_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0xC3 opcode received
#define BIT_MSK__CEC_OP_ABORT_24__CEC_OP_ABORT_REG_24_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0xC2 opcode received
#define BIT_MSK__CEC_OP_ABORT_24__CEC_OP_ABORT_REG_24_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0xC1 opcode received
#define BIT_MSK__CEC_OP_ABORT_24__CEC_OP_ABORT_REG_24_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0xC0 opcode received
#define BIT_MSK__CEC_OP_ABORT_24__CEC_OP_ABORT_REG_24_B0                                0x01

// CEC_OPCODE_ABORT 25 Register
#define REG_ADDR__CEC_OP_ABORT_25                                        (REGTX_CEC | 0x00D9)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0xCF opcode received
#define BIT_MSK__CEC_OP_ABORT_25__CEC_OP_ABORT_REG_25_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0xCE opcode received
#define BIT_MSK__CEC_OP_ABORT_25__CEC_OP_ABORT_REG_25_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0xCD opcode received
#define BIT_MSK__CEC_OP_ABORT_25__CEC_OP_ABORT_REG_25_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0xCC opcode received
#define BIT_MSK__CEC_OP_ABORT_25__CEC_OP_ABORT_REG_25_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0xCB opcode received
#define BIT_MSK__CEC_OP_ABORT_25__CEC_OP_ABORT_REG_25_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0xCA opcode received
#define BIT_MSK__CEC_OP_ABORT_25__CEC_OP_ABORT_REG_25_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0xC9 opcode received
#define BIT_MSK__CEC_OP_ABORT_25__CEC_OP_ABORT_REG_25_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0xC8 opcode received
#define BIT_MSK__CEC_OP_ABORT_25__CEC_OP_ABORT_REG_25_B0                                0x01

// CEC_OPCODE_ABORT 26 Register
#define REG_ADDR__CEC_OP_ABORT_26                                        (REGTX_CEC | 0x00DA)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0xD7 opcode received
#define BIT_MSK__CEC_OP_ABORT_26__CEC_OP_ABORT_REG_26_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0xD6 opcode received
#define BIT_MSK__CEC_OP_ABORT_26__CEC_OP_ABORT_REG_26_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0xD5 opcode received
#define BIT_MSK__CEC_OP_ABORT_26__CEC_OP_ABORT_REG_26_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0xD4 opcode received
#define BIT_MSK__CEC_OP_ABORT_26__CEC_OP_ABORT_REG_26_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0xD3 opcode received
#define BIT_MSK__CEC_OP_ABORT_26__CEC_OP_ABORT_REG_26_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0xD2 opcode received
#define BIT_MSK__CEC_OP_ABORT_26__CEC_OP_ABORT_REG_26_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0xD1 opcode received
#define BIT_MSK__CEC_OP_ABORT_26__CEC_OP_ABORT_REG_26_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0xD0 opcode received
#define BIT_MSK__CEC_OP_ABORT_26__CEC_OP_ABORT_REG_26_B0                                0x01

// CEC_OPCODE_ABORT 27 Register
#define REG_ADDR__CEC_OP_ABORT_27                                        (REGTX_CEC | 0x00DB)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0xDF opcode received
#define BIT_MSK__CEC_OP_ABORT_27__CEC_OP_ABORT_REG_27_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0xDE opcode received
#define BIT_MSK__CEC_OP_ABORT_27__CEC_OP_ABORT_REG_27_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0xDD opcode received
#define BIT_MSK__CEC_OP_ABORT_27__CEC_OP_ABORT_REG_27_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0xDC opcode received
#define BIT_MSK__CEC_OP_ABORT_27__CEC_OP_ABORT_REG_27_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0xDB opcode received
#define BIT_MSK__CEC_OP_ABORT_27__CEC_OP_ABORT_REG_27_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0xDA opcode received
#define BIT_MSK__CEC_OP_ABORT_27__CEC_OP_ABORT_REG_27_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0xD9 opcode received
#define BIT_MSK__CEC_OP_ABORT_27__CEC_OP_ABORT_REG_27_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0xD8 opcode received
#define BIT_MSK__CEC_OP_ABORT_27__CEC_OP_ABORT_REG_27_B0                                0x01

// CEC_OPCODE_ABORT 28 Register
#define REG_ADDR__CEC_OP_ABORT_28                                        (REGTX_CEC | 0x00DC)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0xE7 opcode received
#define BIT_MSK__CEC_OP_ABORT_28__CEC_OP_ABORT_REG_28_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0xE6 opcode received
#define BIT_MSK__CEC_OP_ABORT_28__CEC_OP_ABORT_REG_28_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0xE5 opcode received
#define BIT_MSK__CEC_OP_ABORT_28__CEC_OP_ABORT_REG_28_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0xE4 opcode received
#define BIT_MSK__CEC_OP_ABORT_28__CEC_OP_ABORT_REG_28_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0xE3 opcode received
#define BIT_MSK__CEC_OP_ABORT_28__CEC_OP_ABORT_REG_28_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0xE2 opcode received
#define BIT_MSK__CEC_OP_ABORT_28__CEC_OP_ABORT_REG_28_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0xE1 opcode received
#define BIT_MSK__CEC_OP_ABORT_28__CEC_OP_ABORT_REG_28_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0xE0 opcode received
#define BIT_MSK__CEC_OP_ABORT_28__CEC_OP_ABORT_REG_28_B0                                0x01

// CEC_OPCODE_ABORT 29 Register
#define REG_ADDR__CEC_OP_ABORT_29                                        (REGTX_CEC | 0x00DD)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0xEF opcode received
#define BIT_MSK__CEC_OP_ABORT_29__CEC_OP_ABORT_REG_29_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0xEE opcode received
#define BIT_MSK__CEC_OP_ABORT_29__CEC_OP_ABORT_REG_29_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0xED opcode received
#define BIT_MSK__CEC_OP_ABORT_29__CEC_OP_ABORT_REG_29_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0xEC opcode received
#define BIT_MSK__CEC_OP_ABORT_29__CEC_OP_ABORT_REG_29_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0xEB opcode received
#define BIT_MSK__CEC_OP_ABORT_29__CEC_OP_ABORT_REG_29_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0xEA opcode received
#define BIT_MSK__CEC_OP_ABORT_29__CEC_OP_ABORT_REG_29_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0xE9 opcode received
#define BIT_MSK__CEC_OP_ABORT_29__CEC_OP_ABORT_REG_29_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0xE8 opcode received
#define BIT_MSK__CEC_OP_ABORT_29__CEC_OP_ABORT_REG_29_B0                                0x01

// CEC_OPCODE_ABORT 30 Register
#define REG_ADDR__CEC_OP_ABORT_30                                        (REGTX_CEC | 0x00DE)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0xF7 opcode received
#define BIT_MSK__CEC_OP_ABORT_30__CEC_OP_ABORT_REG_30_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0xF6 opcode received
#define BIT_MSK__CEC_OP_ABORT_30__CEC_OP_ABORT_REG_30_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0xF5 opcode received
#define BIT_MSK__CEC_OP_ABORT_30__CEC_OP_ABORT_REG_30_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0xF4 opcode received
#define BIT_MSK__CEC_OP_ABORT_30__CEC_OP_ABORT_REG_30_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0xF3 opcode received
#define BIT_MSK__CEC_OP_ABORT_30__CEC_OP_ABORT_REG_30_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0xF2 opcode received
#define BIT_MSK__CEC_OP_ABORT_30__CEC_OP_ABORT_REG_30_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0xF1 opcode received
#define BIT_MSK__CEC_OP_ABORT_30__CEC_OP_ABORT_REG_30_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0xF0 opcode received
#define BIT_MSK__CEC_OP_ABORT_30__CEC_OP_ABORT_REG_30_B0                                0x01

// CEC_OPCODE_ABORT 31 Register
#define REG_ADDR__CEC_OP_ABORT_31                                        (REGTX_CEC | 0x00DF)
// (ReadWrite, Bits 7)
// Send automatic [Feature Abort] when 0xFF opcode received
#define BIT_MSK__CEC_OP_ABORT_31__CEC_OP_ABORT_REG_31_B7                                0x80
// (ReadWrite, Bits 6)
// Send automatic [Feature Abort] when 0xFE opcode received
#define BIT_MSK__CEC_OP_ABORT_31__CEC_OP_ABORT_REG_31_B6                                0x40
// (ReadWrite, Bits 5)
// Send automatic [Feature Abort] when 0xFD opcode received
#define BIT_MSK__CEC_OP_ABORT_31__CEC_OP_ABORT_REG_31_B5                                0x20
// (ReadWrite, Bits 4)
// Send automatic [Feature Abort] when 0xFC opcode received
#define BIT_MSK__CEC_OP_ABORT_31__CEC_OP_ABORT_REG_31_B4                                0x10
// (ReadWrite, Bits 3)
// Send automatic [Feature Abort] when 0xFB opcode received
#define BIT_MSK__CEC_OP_ABORT_31__CEC_OP_ABORT_REG_31_B3                                0x08
// (ReadWrite, Bits 2)
// Send automatic [Feature Abort] when 0xFA opcode received
#define BIT_MSK__CEC_OP_ABORT_31__CEC_OP_ABORT_REG_31_B2                                0x04
// (ReadWrite, Bits 1)
// Send automatic [Feature Abort] when 0xF9 opcode received
#define BIT_MSK__CEC_OP_ABORT_31__CEC_OP_ABORT_REG_31_B1                                0x02
// (ReadWrite, Bits 0)
// Send automatic [Feature Abort] when 0xF8 opcode received
#define BIT_MSK__CEC_OP_ABORT_31__CEC_OP_ABORT_REG_31_B0                                0x01

// CEC_10ms_COUNT_L Register
#define REG_ADDR__CEC_AUTO_DISCOVERY                                     (REGTX_CEC | 0x00E0)
// (ReadOnly, Bits 7)
// Automatic PING Discovery Done
#define BIT_MSK__CEC_AUTO_DISCOVERY__CEC_AUTO_PING_DONE                                    0x80
// (ReadWrite, Bits 1)
// Clear Automaitc PING Disocvery Done State Write 1 to clear
#define BIT_MSK__CEC_AUTO_DISCOVERY__CEC_AUTO_PING_CLEAR                                   0x02
// (ReadWrite, Bits 0)
// Start Automatic PING Discovery Write 1 to clear
#define BIT_MSK__CEC_AUTO_DISCOVERY__CEC_AUTO_PING_START                                   0x01

// CEC_10ms_COUNT_M Register
#define REG_ADDR__CEC_AUTODISC_MAP0                                      (REGTX_CEC | 0x00E1)
// (ReadOnly, Bits 7)
// Logical address 7 discovered
#define BIT_MSK__CEC_AUTODISC_MAP0__CEC_AUTO_PING_MAP_B7                                  0x80
// (ReadOnly, Bits 6)
// Logical address 6 discovered
#define BIT_MSK__CEC_AUTODISC_MAP0__CEC_AUTO_PING_MAP_B6                                  0x40
// (ReadOnly, Bits 5)
// Logical address 5 discovered
#define BIT_MSK__CEC_AUTODISC_MAP0__CEC_AUTO_PING_MAP_B5                                  0x20
// (ReadOnly, Bits 4)
// Logical address 4 discovered
#define BIT_MSK__CEC_AUTODISC_MAP0__CEC_AUTO_PING_MAP_B4                                  0x10
// (ReadOnly, Bits 3)
// Logical address 3 discovered
#define BIT_MSK__CEC_AUTODISC_MAP0__CEC_AUTO_PING_MAP_B3                                  0x08
// (ReadOnly, Bits 2)
// Logical address 2 discovered
#define BIT_MSK__CEC_AUTODISC_MAP0__CEC_AUTO_PING_MAP_B2                                  0x04
// (ReadOnly, Bits 1)
// Logical address 1 discovered
#define BIT_MSK__CEC_AUTODISC_MAP0__CEC_AUTO_PING_MAP_B1                                  0x02
// (ReadOnly, Bits 0)
// Logical address 0 discovered
#define BIT_MSK__CEC_AUTODISC_MAP0__CEC_AUTO_PING_MAP_B0                                  0x01

// CEC_10ms_COUNT_L Register
#define REG_ADDR__CEC_AUTODISC_MAP1                                      (REGTX_CEC | 0x00E2)
// (ReadOnly, Bits 7)
// Logical address 15 discovered
#define BIT_MSK__CEC_AUTODISC_MAP1__CEC_AUTO_PING_MAP_B15                                 0x80
// (ReadOnly, Bits 6)
// Logical address 14 discovered
#define BIT_MSK__CEC_AUTODISC_MAP1__CEC_AUTO_PING_MAP_B14                                 0x40
// (ReadOnly, Bits 5)
// Logical address 13 discovered
#define BIT_MSK__CEC_AUTODISC_MAP1__CEC_AUTO_PING_MAP_B13                                 0x20
// (ReadOnly, Bits 4)
// Logical address 12 discovered
#define BIT_MSK__CEC_AUTODISC_MAP1__CEC_AUTO_PING_MAP_B12                                 0x10
// (ReadOnly, Bits 3)
// Logical address 11 discovered
#define BIT_MSK__CEC_AUTODISC_MAP1__CEC_AUTO_PING_MAP_B11                                 0x08
// (ReadOnly, Bits 2)
// Logical address 10 discovered
#define BIT_MSK__CEC_AUTODISC_MAP1__CEC_AUTO_PING_MAP_B10                                 0x04
// (ReadOnly, Bits 1)
// Logical address 9 discovered
#define BIT_MSK__CEC_AUTODISC_MAP1__CEC_AUTO_PING_MAP_B9                                  0x02
// (ReadOnly, Bits 0)
// Logical address 8 discovered
#define BIT_MSK__CEC_AUTODISC_MAP1__CEC_AUTO_PING_MAP_B8                                  0x01

// CEC CDC Arbitration Count and Enable Register
#define REG_ADDR__CEC_CDC_EN_CNT                                         (REGTX_CEC | 0x00E3)
// (ReadWrite, Bits 7)
// CDC Arbitration Enable
#define BIT_MSK__CEC_CDC_EN_CNT__CEC_CDC_ARB_EN                                        0x80
// (ReadWrite, Bits 4:0)
// CDC Arbitration Count, number of arbitration blocks after the CEC Header
#define BIT_MSK__CEC_CDC_EN_CNT__CEC_CDC_ARB_CNT_B4_B0                                 0x1F

// CEC CDC Opcode Register
#define REG_ADDR__CEC_CDC_OPCODE                                         (REGTX_CEC | 0x00E4)
// (ReadWrite, Bits 7:0)
// CDC Opcode to indicate the CDC messages
#define BIT_MSK__CEC_CDC_OPCODE__CEC_CDC_OPCODE                                        0xFF

#endif // _SI_DRV_TX_REGS_H_
