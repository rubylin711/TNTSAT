/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_FD650_H__
#define __DRV_FD650_H__

#include <linux/delay.h>
#include "drv_gpio_ioctl.h"


#define DELAY udelay(150)

/* 接口的PIO连接,与实际电路有关 */
#define	FD650_SCL       (AO_GPIO_6)       //定义IO端口
#define	FD650_SDA       (AO_GPIO_7)       //定义IO端口

#define HIGH 1
#define LOW  0

/* 接口的PIO操作，与平台IO操作有关 */
#define FD650_SCL_SET   drv_gpio_set_value(FD650_SCL, GPIO_VALUE_HIGH_LEVEL)   //将SCL设置为高电平
#define FD650_SCL_CLR   drv_gpio_set_value(FD650_SCL, GPIO_VALUE_LOW_LEVEL)  //将SCL设置为低电平
#define FD650_SCL_D_OUT drv_gpio_set_dir(FD650_SCL, GPIO_DIR_OUTPUT)	// 设置SCL为输出方向,对于双向I/O需切换为输出
#define FD650_SDA_SET   drv_gpio_set_value(FD650_SDA, GPIO_VALUE_HIGH_LEVEL)   //将SDA设置为高电平
#define FD650_SDA_CLR   drv_gpio_set_value(FD650_SDA, GPIO_VALUE_LOW_LEVEL)   //将SDA设置为低电平
#define FD650_SDA_IN    drv_gpio_get_value(FD650_SDA, &gpioval)	//当SDA设为输入方向时，读取的电平值
#define FD650_SDA_D_OUT drv_gpio_set_dir(FD650_SDA, GPIO_DIR_OUTPUT)	// 设置SDA为输出方向,对于双向I/O需切换为输出
#define FD650_SDA_D_IN  drv_gpio_set_dir(FD650_SDA, GPIO_DIR_INPUT)	// 设置SDA为输入方向,对于双向I/O需切换为输入



#define FP_MAX_LED_NUM  4


#define FD650_BIT_ENABLE	0x01		// 开启/关闭位
#define FD650_BIT_SLEEP		0x04		// 睡眠控制位
#define FD650_BIT_7SEG		0x08		// 7段控制位
#define FD650_BIT_INTENS1	0x10		// 1级亮度
#define FD650_BIT_INTENS2	0x20		// 2级亮度
#define FD650_BIT_INTENS3	0x30		// 3级亮度
#define FD650_BIT_INTENS4	0x40		// 4级亮度
#define FD650_BIT_INTENS5	0x50		// 5级亮度
#define FD650_BIT_INTENS6	0x60		// 6级亮度
#define FD650_BIT_INTENS7	0x70		// 7级亮度
#define FD650_BIT_INTENS8	0x00		// 8级亮度

#define FD650_SYSOFF	0x0400			// 关闭显示、关闭键盘
#define FD650_SYSON		( FD650_SYSOFF | FD650_BIT_ENABLE )	// 开启显示、键盘
#define FD650_SLEEPOFF	FD650_SYSOFF	// 关闭睡眠
#define FD650_SLEEPON	( FD650_SYSOFF | FD650_BIT_SLEEP )	// 开启睡眠
#define FD650_7SEG_ON	( FD650_SYSON | FD650_BIT_7SEG )	// 开启七段模式
#define FD650_8SEG_ON	( FD650_SYSON | 0x00 )	// 开启八段模式
#define FD650_SYSON_1	( FD650_SYSON | FD650_BIT_INTENS1 )	// 开启显示、键盘、1级亮度
#define FD650_SYSON_2	( FD650_SYSON | FD650_BIT_INTENS2 )	// 开启显示、键盘、2级亮度
#define FD650_SYSON_3	( FD650_SYSON | FD650_BIT_INTENS3 )	// 开启显示、键盘、3级亮度
#define FD650_SYSON_4	( FD650_SYSON | FD650_BIT_INTENS4 )	// 开启显示、键盘、4级亮度
#define FD650_SYSON_5	( FD650_SYSON | FD650_BIT_INTENS5 )	// 开启显示、键盘、5级亮度
#define FD650_SYSON_6	( FD650_SYSON | FD650_BIT_INTENS6 )	// 开启显示、键盘、6级亮度
#define FD650_SYSON_7	( FD650_SYSON | FD650_BIT_INTENS7 )	// 开启显示、键盘、7级亮度
#define FD650_SYSON_8	( FD650_SYSON | FD650_BIT_INTENS8 )	// 开启显示、键盘、8级亮度


// 加载字数据命令
#define FD650_DIG0		0x1400			// 数码管位0显示,需另加8位数据
#define FD650_DIG1		0x1500			// 数码管位1显示,需另加8位数据
#define FD650_DIG2		0x1600			// 数码管位2显示,需另加8位数据
#define FD650_DIG3		0x1700			// 数码管位3显示,需另加8位数据

#define FD650_DOT			0x0080			// 数码管小数点显示

// 读取按键代码命令
#define FD650_GET_KEY	0x0700					// 获取按键,返回按键代码


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
    u8 bitmap;
  }led_bitmap_t;

#endif
