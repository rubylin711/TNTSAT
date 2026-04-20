/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_SCI_REGS_H__
#define __DRV_SCI_REGS_H__

#if defined(CONFIG_MT_CHIP_ARIA)
#define R_SMC0_BASE_ADDR    0xFFA20000
#define R_SMC1_BASE_ADDR    0xFFA30000
#define SYSCTRL_REG_BASE    0xffaf0000
#else
#define R_SMC0_BASE_ADDR    SYMPHONY_IO_VA(0xBF590000)
#define R_SMC1_BASE_ADDR    SYMPHONY_IO_VA(0xBF5A0000)
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#define R_SMC_ENABLE_CLK_ADDR 	SYMPHONY_IO_VA(0xBF509000)
#define R_SMC_CLK_ADDR    	SYMPHONY_IO_VA(0xBF509004)
/*#define R_SMC_PIN1_ADDR    	SYMPHONY_IO_VA(0xbf13c018)
#define R_SMC_PIN2_ADDR    	SYMPHONY_IO_VA(0xbf13c02c)
#define R_SMC_PIN3_ADDR    	SYMPHONY_IO_VA(0xbf13c030)
#define R_SMC_PIN4_ADDR    	SYMPHONY_IO_VA(0xbf13c034)
#define R_SMC_DETECTPIN_CFG 	SYMPHONY_IO_VA(0xbf0a000c)
#define R_SMC_DETECTPIN_INPUT  	SYMPHONY_IO_VA(0xbf0a0004)
#define R_SMC_SOFT_CTRL  	SYMPHONY_IO_VA(0xbf13c200)

#define R_KEY_ADC2_CFG  	SYMPHONY_IO_VA(0xbf5d0800)
#define R_KEY_ADC2_DATA  	SYMPHONY_IO_VA(0xbf5d0804)
#define R_KEY_ADC2_INT  	SYMPHONY_IO_VA(0xbf5d0808)
#define R_KEY_ADC2_RANGE  	SYMPHONY_IO_VA(0xbf5d080c)*/
#endif
#endif

#define R_SMC_DBUF 0x0
#define R_SMC_STA   0x4
#define R_SMC_FRQ_CFG  0x8
#define R_SMC_NC_SET   0xC
#define R_SMC_TRANS_CTRL   0x10
#define R_SMC_ETU1_SET 0x14
#define R_SMC_ETU2_SET 0x18
#define R_SMC_ETU3_SET 0x1C
#define R_SMC_CPCTRL   0x20
#define R_SMC_CMDLEN_H 0x24
#define R_SMC_CMDLEN_L 0x28
#define R_SMC_BUF_CNT_L    0x2C
#define R_SMC_BUF_CNT_H    0x30
#define R_SMC_PIN_CFG  0x34
#define R_SMC_RESEND_CFG   0x38

#define R_SMC_MODE_CFG   0x3C
#define R_SMC_BLK_TIMEOUT_CNT0	0x40
#define R_SMC_BLK_TIMEOUT_CNT1	0x44
#define R_SMC_BLK_TIMEOUT_CNT2	0x48
#define R_SMC_BLK_TIMEOUT_CNT3	0x4C

#define R_SMC_RST_TIMEOUT_CNT0	0x50
#define R_SMC_RST_TIMEOUT_CNT1	0x54
#define R_SMC_RST_TIMEOUT_CNT2	0x58
#define R_SMC_RST_TIMEOUT_CNT3	0x5C

#define R_SMC_RCV_TIMEOUT_CNT0	0x60
#define R_SMC_RCV_TIMEOUT_CNT1	0x64
#define R_SMC_RCV_TIMEOUT_CNT2	0x68
#define R_SMC_RCV_TIMEOUT_CNT3	0x6C

#define R_SMC_INT_STA0   0x70

#define R_SMC_RCV_DATA_CNTL   0x74
#define R_SMC_RCV_DATA_CNTH   0x78

#define R_SMC_T_CNT_20	0x7C
#define R_SMC_T_CNT_21	0x80
#define R_SMC_T_CNT_22	0x84
#define R_SMC_T_CNT_23	0x88

#define R_SMC_CFG_0	0x8C
#define R_SMC_CFG_1	0x90
#define R_SMC_CFG_2	0x94
#define R_SMC_CFG_3	0x98

#define R_SMC_ACTVT_FINISH   0x9c   //bit0:read clear
#define R_SMC_INT_STA1   0xA0

#define R_SMC_INT_CLR0   0xA4
#define R_SMC_INT_CLR1   0xA8

#define R_SMC_5VIO_CTRL0	0xAC
#define R_SMC_5VIO_CTRL1	0xB0
#define R_SMC_5VIO_CTRL2	0xB4

#define R_SMC_OVERLOAD_CNT0	0xB8
#define R_SMC_OVERLOAD_CNT1	0xBC
#define R_SMC_OVERLOAD_CNT2	0xC0
#define R_SMC_OVERLOAD_CNT3	0xC4

#define R_SMC_RCVINT_DISCARD	0xC8

#endif
