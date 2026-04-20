/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __CONCERTO_REGS_AV_H__
#define __CONCERTO_REGS_AV_H__

#ifdef FPGA
//#define SYS_CPU_CLOCK_M                   81
#else
//#define SYS_CPU_CLOCK_M                   400
#endif

//#define SYS_CPU_CLOCK       (SYS_CPU_CLOCK_M * 1000000)


//PIC Contorller
#define S_PIC_BASE_ADDR           0xbf110000
#define R_S_PIC_INT_MODE0       (S_PIC_BASE_ADDR + 0x0)
#define R_S_PIC_INT_MODE1       (S_PIC_BASE_ADDR + 0x4)
#define R_S_PIC_INT_MODE2       (S_PIC_BASE_ADDR + 0x8)
#define R_S_PIC_INT_MODE3       (S_PIC_BASE_ADDR + 0xc)
#define R_S_PIC_INT_MODE4       (S_PIC_BASE_ADDR + 0x10)
#define R_S_PIC_INT_MODE5       (S_PIC_BASE_ADDR + 0x14)
#define R_S_PIC_INT_PRIOR0       (S_PIC_BASE_ADDR + 0x20)
#define R_S_PIC_INT_PRIOR1       (S_PIC_BASE_ADDR + 0x24)
#define R_S_PIC_INT_PRIOR2       (S_PIC_BASE_ADDR + 0x28)
#define R_S_PIC_INT_PRIOR3       (S_PIC_BASE_ADDR + 0x2c)
#define R_S_PIC_INT_PRIOR4       (S_PIC_BASE_ADDR + 0x30)
#define R_S_PIC_INT_PRIOR5       (S_PIC_BASE_ADDR + 0x34)
#define R_S_PIC_INT_PRIOR6       (S_PIC_BASE_ADDR + 0x38)
#define R_S_PIC_INT_PRIOR7       (S_PIC_BASE_ADDR + 0x3c)
#define R_S_PIC_INT_PRIOR8       (S_PIC_BASE_ADDR + 0x40)
#define R_S_PIC_INT_PRIOR9       (S_PIC_BASE_ADDR + 0x44)
#define R_S_PIC_INT_PRIOR10       (S_PIC_BASE_ADDR + 0x48)
#define R_S_PIC_INT_PRIOR11       (S_PIC_BASE_ADDR + 0x4c)
#define R_S_PIC_INT_MASK0       (S_PIC_BASE_ADDR + 0x60)
#define R_S_PIC_INT_MASK1       (S_PIC_BASE_ADDR + 0x64)
#define R_S_PIC_INT_NUM      (S_PIC_BASE_ADDR + 0x68)
#define R_S_PIC_INT_EOI      (S_PIC_BASE_ADDR + 0x6c)
#define R_S_PIC_TI_INIT0      (S_PIC_BASE_ADDR + 0x80)
#define R_S_PIC_TI_INIT1      (S_PIC_BASE_ADDR + 0x84)
#define R_S_PIC_TI_INIT2      (S_PIC_BASE_ADDR + 0x88)
#define R_S_PIC_TI_INIT3      (S_PIC_BASE_ADDR + 0x8c)
#define R_S_PIC_TI_CAP0      (S_PIC_BASE_ADDR + 0x90)
#define R_S_PIC_TI_CAP1      (S_PIC_BASE_ADDR + 0x94)
#define R_S_PIC_TI_CAP2      (S_PIC_BASE_ADDR + 0x98)
#define R_S_PIC_TI_CAP3      (S_PIC_BASE_ADDR + 0x9c)
#define R_S_PIC_TI_CW      (S_PIC_BASE_ADDR + 0xa0)
#define R_S_PIC_WD_MPR     (S_PIC_BASE_ADDR + 0x100)
#define R_S_PIC_WD_PROTECT     (S_PIC_BASE_ADDR + 0x104)
#define R_S_PIC_WD_EN     (S_PIC_BASE_ADDR + 0x108)
#define R_S_PIC_WD_FEED    (S_PIC_BASE_ADDR + 0x10c)
#define R_S_PIC_WD_STA_CLR    (S_PIC_BASE_ADDR + 0x110)
#define R_S_PIC_WD_STATUS   (S_PIC_BASE_ADDR + 0x114)

//Mailbox register
/* master core, AP */
#define  MCPU_MB_INIT     (SYMPHONY_MAILBOX_BASE + 0x000)
#define  MCPU_MB_INTCLR   (SYMPHONY_MAILBOX_BASE + 0x004)
#define  MCPU_MB_ENACLR   (SYMPHONY_MAILBOX_BASE + 0x008)
#define  MCPU_MB_INTSET   (SYMPHONY_MAILBOX_BASE + 0x0CC)
#define  MCPU_MB_ENASET   (SYMPHONY_MAILBOX_BASE + 0x0D0)
#define  MCPU_MB_ADDR_BASE (SYMPHONY_MAILBOX_BASE + 0x0D4)
#define  MCPU_MB_ADDR_REG(chan) (MCPU_MB_ADDR_BASE + (chan) * 4)

/* slave core, AV */
#define  SCPU_MB_INIT     (SYMPHONY_MAILBOX_BASE + 0x0C0)
#define  SCPU_MB_INTSET   (SYMPHONY_MAILBOX_BASE + 0x00C)
#define  SCPU_MB_ENASET   (SYMPHONY_MAILBOX_BASE + 0x010)
#define  SCPU_MB_INTCLR   (SYMPHONY_MAILBOX_BASE + 0x0C4)
#define  SCPU_MB_ENACLR   (SYMPHONY_MAILBOX_BASE + 0x0C8)
#define  SCPU_MB_ADDR_BASE (SYMPHONY_MAILBOX_BASE + 0x014)
#define  SCPU_MB_ADDR_REG(chan) (SCPU_MB_ADDR_BASE + (chan) * 4)

#define  MCPU_MB_INT   (SYMPHONY_MAILBOX_BASE + 0x180)
#define  MCPU_MB_INT_STATE  MCPU_MB_INT
#define  MCPU_MB_ENA   (SYMPHONY_MAILBOX_BASE + 0x184)
#define  MCPU_MB_ENA_STATE  MCPU_MB_ENA

#define  SCPU_MB_INT   (SYMPHONY_MAILBOX_BASE + 0x188)
#define  SCPU_MB_INT_STATE   SCPU_MB_INT
#define  SCPU_MB_ENA   (SYMPHONY_MAILBOX_BASE + 0x18C)
#define  SCPU_MB_ENA_STATE   SCPU_MB_ENA

#ifdef CONFIG_MT_CHIP_SYMPHONY4
#define MCPU_MB_ACK		(SYMPHONY_MAILBOX_BASE + 0x190)
#define SCPU_MB_ACK		(SYMPHONY_MAILBOX_BASE + 0x194)
#endif

#define SCPU_MB_INFO_REG(chan) SCPU_MB_ADDR_REG(chan)

#define MCPU_MB_INFO_REG(chan) MCPU_MB_ADDR_REG(chan)


#define INT_TRIG_LOW_LEVEL   0
#define INT_TRIG_HIGH_LEVEL  1
#define INT_TRIG_FALLING_EDGE  2
#define INT_TRIG_RISING_EDGE  3
#define INT_TRIG_EDGES        4

/*!
  * There are 32 locks,
  * 0xBF128040 - 0xBF1280BC
  * updates;;;;
  *
  * write-clear / read-set
  */
#define  MB_MEMLOCK_BASE   0xBF128200 //0xBF128040
/*!
  * address for each channel
  */
#define  MB_MEMLOCK_REG(chan) (MB_MEMLOCK_BASE | ((chan) * 4))


#endif //__CONCERTO_REGS_AV_H__

