/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


// MCU core Registers list
// MCU core offset registers list
#define  MCU_REG_BASE           0xc000
#define  MCU_PRESCALER          0x00
#define  MCU_PRELOAD			0x04
#define  MCU_TRIGGER            0x08
#define  MCU_INT_STATUE         0x10
#define  MCU_TO_AP_DATA0        0x14
#define  MCU_TO_AP_DATA1        0x18
#define  MCU_TO_AP_DATA2        0x1c
#define  MCU_TO_AP_DATA3        0x20
#define  MCU_TO_AP_DATA4        0x60
#define  MCU_TO_AP_DATA5        0x64
#define  MCU_TO_AP_DATA6        0x68
#define  MCU_TO_AP_DATA7        0x6c
#define  MCU_TO_AP_DATA8        0x70
#define  MCU_TO_AP_DATA9        0x74
#define  MCU_TO_AP_DATA10       0x78
#define  MCU_TO_AP_DATA11       0x7c

#define  MCU_SOFT_RESET         0x38
#define  RESET_TO_CORE          0x3C


#define  CORE_RESET_ENABLE       0x1
#define  CORE_RESET_DISABLE      0x0

#define  MCU_RESET_ENABLE       0x1
#define  MCU_RESET_DISABLE      0x0

#define  FP_SEL_APCPU           0x1
#define  FP_SEL_MCU             0x0


/*!
 AOMCU
*/
#define LPM_AOMCU_WAKEUP_FLAG (0x0010)
#define LPM_AOMCU_MESSAGE (0x0024)
#define LPM_MESSAGE2AO_0  (0x0040)
#define LPM_MESSAGE2AO_1  (0x0044)
#define LPM_MESSAGE2AO_2  (0x0048)
#define LPM_MESSAGE2AO_3  (0x004C)
#define LPM_MESSAGE2AO_4  (0x0020)
#define LPM_MESSAGE2AO_5  (0x0038)
#define LPM_AOMCU_EN      (0x0050)
#define LPM_SLEEP_MESSAGE (0x0028)

#define AOGPIO3_SET_CFG			(0xB400)
#define AOGPIO0_SET_CFG			(0xB404)
#define AOGPIO0_SET_DATA			(0x5000)
#define AOGPIO0_SET_DIRECTION		(0x5004)
#define AOGPIO0_GET_DATA			(0x5008)
#define AOGPIO0_SET_WRITEEN		(0x500c)
#define AOGPIO0_SET_INTERRUPTEN	(0x5044)

#define IRNEC_DATA_BASE	    (0x1000)
#define IRGLOBAL_STA_BASE	(0x100C)
#define IRINT_RAWSTA_BASE	(0x1014)
#define IRWF_ONTEST_BASE	(0x1400)
#define IRWF_PRDEST_BASE	(0x1404)

#define IRWF_BUFF_BASE	(0x17F8)
#define IRWF_MIN_BASE	(0x17D0)

#define SMC0_STA		(0x0004)

#define AO_GPIO_WR_EN  (0x5004)
#define AO_GPIO_MASK   (0x500C)
#define AO_GPIO_RDATA  (0x5008)
#define AO_GPIO_WDATA  (0x5000)

//fix bug 99011 start
#define AO_GPIO_INT_MODE         (0x5040)
#define AO_GPIO_INT_EN           (0x5044)
#define AO_GPIO_PIC_INT_GEN_MODE (0x5048)
#define AO_GPIO_INIT0_SEL        (0x504c)
#define AO_GPIO_INT0_SYSOFF_S    (0x5054)
//fix bug 99011 end

#if 1
#define FP_SPI_TXD                          0x9000
#define FP_SPI_RXD                          0x9000
#define FP_SPI_CH_BAUD                      0x9100
#define FP_SPI_CH_MODE_CFG                  0x9104
#define FP_SPI_TC		                    0x9120
#define FP_SPI_CTRL                      	0x9124
#define FP_SPI_INT_CFG                      0x9128
#define FP_SPI_PIN_MODE                     0x9130
#define FP_SPI_PIN_CTRL                     0x9134
#define FP_SPI_STA                      	0x9140
#define FP_SPI_INT_STA                      0x9144
#define FP_SPI_CMD_FIFO                     0x9148

#define FP_SPI_DATA_CFG                     0x9134
#else
#define FP_SPI_TXD                          0xbf159000
#define FP_SPI_RXD                          0xbf159000
#define FP_SPI_CH_BAUD                      0xbf159100
#define FP_SPI_CH_MODE_CFG                  0xbf159104
#define FP_SPI_TC		                    0xbf159120
#define FP_SPI_CTRL                      	0xbf159124
#define FP_SPI_INT_CFG                      0xbf159128
#define FP_SPI_PIN_MODE                     0xbf159130
#define FP_SPI_PIN_CTRL                     0xbf159134
#define FP_SPI_STA                      	0xbf159140
#define FP_SPI_INT_STA                      0xbf159144
#define FP_SPI_CMD_FIFO                     0xbf159148

#define FP_SPI_DATA_CFG                     0xbf159134

#endif

