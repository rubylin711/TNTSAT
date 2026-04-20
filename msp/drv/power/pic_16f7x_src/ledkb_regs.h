/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#define  LEDKB_REG_BASE       	0x8000

#define  LEDKB_IC_CONFIG    	  0x00
#define  LEDKB_CLK_DIVIDER      0x04
#define  LEDKB_INT_STATUS       0x08
#define  LEDKB_INT_ENABLE       0x0c
#define  LEDKB_MCU_CONFIG    	  0x10
#define  LEDKB_MCU_COMMAND   	  0x14
#define  LEDKB_MCU_TX_DATA    	0x18
#define  LEDKB_MCU_RX_DATA    	0x1c
#define  LEDKB_MCU_STATUS    	  0x20

#define  LEDKB_KEY_VALUE        0x24
#define  LEDKB_CT1642_CONFIG    0x28
#define  LEDKB_LED_GRID0        0x30
#define  LEDKB_LED_GRID1        0x34
#define  LEDKB_LED_GRID2        0x38
#define  LEDKB_LED_GRID3        0x3c

#define  McuWriteEn				0x02      	
#define  McuReadEn				0x00
