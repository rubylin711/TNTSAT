/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_CT1642_H__
#define __DRV_CT1642_H__

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#if defined(CONFIG_MT_CHIP_ARIA)
#define MIPS_CPU_NUM_IRQ					8
#define SYMPHONY_PIC_IRQ_BASE				MIPS_CPU_NUM_IRQ
#define IRQ_LEDKB_ID						(40+SYMPHONY_PIC_IRQ_BASE)
#endif

#define FREQ_KHZ 1000

#if 0
#define LEDKB_HOST_REG_BASE			SYMPHONY_IO_VA(0xBF154000UL)
#define LEDKB_CTRL					(LEDKB_HOST_REG_BASE)
#define LEDKB_TIMESET				(LEDKB_HOST_REG_BASE + 0x04)
#define LEDKB_TIMESEL				(LEDKB_HOST_REG_BASE + 0x08)
#define LEDKB_INTMASK_KEYFILT		(LEDKB_HOST_REG_BASE + 0x0C)
#define LEDKB_CLK_PARA				(LEDKB_HOST_REG_BASE + 0x10)
#define LEDKB_DEN_PARA				(LEDKB_HOST_REG_BASE + 0x14)
#define LEDKB_DAT_PARA(i)			(LEDKB_HOST_REG_BASE + 0x18 + (i) * 4)
#define LEDKB_DAT_DELAY_NUM			(LEDKB_HOST_REG_BASE + 0x60)
#define LEDKB_KEY_VALUE0			(LEDKB_HOST_REG_BASE + 0x70)
#define LEDKB_KEY_VALUE1			(LEDKB_HOST_REG_BASE + 0x74)
#define LEDKB_KEY_PRESS0			(LEDKB_HOST_REG_BASE + 0x78)
#define LEDKB_KEY_PRESS1			(LEDKB_HOST_REG_BASE + 0x7C)
#define LEDKB_KEY_RELEASE0			(LEDKB_HOST_REG_BASE + 0x80)
#define LEDKB_KEY_RELEASE1			(LEDKB_HOST_REG_BASE + 0x84)
#endif
#define LEDKB_HOST_REG_BASE			SYMPHONY_IO_VA(0xBF154000UL)
#define LEDKB_CTRL					(0x0)
#define LEDKB_TIMESET				(0x04)
#define LEDKB_TIMESEL				(0x08)
#define LEDKB_INTMASK_KEYFILT		(0x0C)
#define LEDKB_CLK_PARA				(0x10)
#define LEDKB_DEN_PARA				(0x14)
#define LEDKB_DAT_PARA(i)			(0x18 + (i) * 4)
#define LEDKB_DAT_DELAY_NUM			(0x60)
#define LEDKB_KEY_VALUE0			(0x70)
#define LEDKB_KEY_VALUE1			(0x74)
#define LEDKB_KEY_PRESS0			(0x78)
#define LEDKB_KEY_PRESS1			(0x7C)
#define LEDKB_KEY_RELEASE0			(0x80)
#define LEDKB_KEY_RELEASE1			(0x84)



#define FP_MAX_LED_NUM  4

typedef enum
  {
    /*!
      IRDA module
      */
    UIO_IRDA = 0,
    /*!
      frontpanel module
      */
    UIO_FRONTPANEL
  }uio_type_t;

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

