/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VFD_PT6311B_H
#define __VFD_PT6311B_H

#include <linux/delay.h>
#include "drv_gpio_ioctl.h"

#define FP_SPI_BASE_ADDR                          0xBF159000


#define FP_SPI_TXD                          SYMPHONY_IO_VA(FP_SPI_BASE_ADDR)
#define FP_SPI_RXD                          SYMPHONY_IO_VA(FP_SPI_BASE_ADDR)
#define FP_SPI_CH_BAUD                      SYMPHONY_IO_VA((FP_SPI_BASE_ADDR) + (0x100))
#define FP_SPI_CH_MODE_CFG                  SYMPHONY_IO_VA((FP_SPI_BASE_ADDR) + (0x104))
#define FP_SPI_TC		                    SYMPHONY_IO_VA((FP_SPI_BASE_ADDR) + (0x120))
#define FP_SPI_CTRL                      	SYMPHONY_IO_VA((FP_SPI_BASE_ADDR) + (0x124))
#define FP_SPI_INT_CFG                      SYMPHONY_IO_VA((FP_SPI_BASE_ADDR) + (0x128))
#define FP_SPI_PIN_MODE						SYMPHONY_IO_VA((FP_SPI_BASE_ADDR) + (0x130))
#define FP_SPI_PIN_CTRL						SYMPHONY_IO_VA((FP_SPI_BASE_ADDR) + (0x134))
#define FP_SPI_STA                      	SYMPHONY_IO_VA((FP_SPI_BASE_ADDR) + (0x140))
#define FP_SPI_INT_STA                      SYMPHONY_IO_VA((FP_SPI_BASE_ADDR) + (0x144))
#define FP_SPI_CMD_FIFO						SYMPHONY_IO_VA((FP_SPI_BASE_ADDR) + (0x148))


#define FP_SPI_PINMUX_1						SYMPHONY_IO_VA(0xBF15B400)
#define FP_SPI_PINMUX_2						SYMPHONY_IO_VA(0xBF15B404)
#define FP_SPI_PINMUX_3						SYMPHONY_IO_VA(0xBF15B408)//0xBF15B4C0


#define FP_SPI_CLK_EN						SYMPHONY_IO_VA(0xBF153840)
#define FP_SPI_RESET_1						SYMPHONY_IO_VA(0xBF153844)
//#define FP_SPI_RESET_2                          SYMPHONY_IO_VA(0xBF153018)
//#define FP_SPI_RESET_3                          SYMPHONY_IO_VA(0xBF153028)

#define FP_SPI_DATA_CFG						SYMPHONY_IO_VA(0xBF159134)



#define FP_MAX_LED_NUM  13
#define	PT6393_STB  AO_GPIO_2
#define	PT6393_SCL  AO_GPIO_1
#define   PT6393_DATA  AO_GPIO_0//if MOSI and MISO use a signle pin, config the same gpio

#define PT6393_STB_HIGH			drv_gpio_set_value(PT6393_STB, GPIO_VALUE_HIGH_LEVEL)
#define PT6393_STB_LOW				drv_gpio_set_value(PT6393_STB, GPIO_VALUE_LOW_LEVEL)
#define PT6393_STB_OUT_MODE		drv_gpio_set_dir(PT6393_STB, GPIO_DIR_OUTPUT)

#define PT6393_SCL_HIGH			drv_gpio_set_value(PT6393_SCL, GPIO_VALUE_HIGH_LEVEL)
#define PT6393_SCL_LOW				drv_gpio_set_value(PT6393_SCL, GPIO_VALUE_LOW_LEVEL)
#define PT6393_SCL_OUT_MODE		drv_gpio_set_dir(PT6393_SCL, GPIO_DIR_OUTPUT)

#define PT6393_DATA_HIGH   			drv_gpio_set_value(PT6393_DATA, GPIO_VALUE_HIGH_LEVEL)
#define PT6393_DATA_LOW   			drv_gpio_set_value(PT6393_DATA, GPIO_VALUE_LOW_LEVEL)
#define PT6393_DATA_IN_VAL(val)		drv_gpio_get_value(PT6393_DATA, &val)
#define PT6393_DATA_IN_MODE		drv_gpio_set_dir(PT6393_DATA, GPIO_DIR_INPUT)
#define PT6393_DATA_OUT_MODE		drv_gpio_set_dir(PT6393_DATA, GPIO_DIR_OUTPUT)


#define PT6393_DISPLAY_MODE_4DIG_24SEG  0x00
#define PT6393_DISPLAY_MODE_5DIG_23SEG  0x01
#define PT6393_DISPLAY_MODE_6DIG_22SEG 0x02
#define PT6393_DISPLAY_MODE_7DIG_21SEG 0x03
#define PT6393_DISPLAY_MODE_8DIG_20SEG 0x04
#define PT6393_DISPLAY_MODE_9DIG_19SEG 0x05
#define PT6393_DISPLAY_MODE_10DIG_18SEG 0x06
#define PT6393_DISPLAY_MODE_11DIG_17SEG 0x07
#define PT6393_DISPLAY_MODE_12DIG_16SEG 0x08
#define PT6393_DISPLAY_MODE_13DIG_15SEG 0x09
#define PT6393_DISPLAY_MODE_13DIG_15SEG_other 0x0f

#define PT6393_WRITE_TO_DISPLAY_MODE	0x00
#define PT6393_WRITE_TO_LED_PORT 		0x01
#define PT6393_READ_KEY_DATA			0x02

#define PT6393_INCREASE_ADDR	0x00
#define PT6393_FIX_ADDR		0x04

#define PT6393_PULSE_WIDTH_0	0x00
#define PT6393_PULSE_WIDTH_1	0x01
#define PT6393_PULSE_WIDTH_2	0x02
#define PT6393_PULSE_WIDTH_3	0x03
#define PT6393_PULSE_WIDTH_4	0x04
#define PT6393_PULSE_WIDTH_5	0x05
#define PT6393_PULSE_WIDTH_6	0x06
#define PT6393_PULSE_WIDTH_7	0x07

#define PT6393_DISPLAY_ON	0x08
#define PT6393_DISPLAY_OFF	0x00

#define pt6393_delay_us	1
typedef struct
{
	MT_U16 dis_buff[FP_MAX_LED_NUM] ;
	MT_U16 special_ch_buff[FP_MAX_LED_NUM];
	u8  dis_len;
	u8 led_port_value;
	struct mutex PT6393_lock;
}PT6393_DIS_LED;

 /*!
    LEB bitmap
    */
  typedef struct
  {
    /*!
      ascii character to display
      */
    u8 ch;
    /*!
      bitmap
      */
    u16 bitmap;
  }led_bitmap_t;

static void pt6393_init(MT_VOID *dev);
static void pt6393_clear_display(PT6393_DIS_LED *p_fp);
static void pt6393_display(ulong param);
static void pt6393_display_special_ch(ulong param);
static void pt6393_set_led_port(ulong param);
static u32 pt6393_get_key(MT_VOID);


#endif
