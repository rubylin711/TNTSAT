/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DMA_SYMPHONY_H__
#define __DMA_SYMPHONY_H__

#include <linux/mutex.h>
#include <linux/device.h>

/*!!
  Linked List Node's maximum length
  */
#define DMA_LLN_LEN_MAX 8

#define BIT_INT_LINK_EN 29
#define BIT_INT_NODE_EN 28
#define BIT_CH_PARA_REG_EN 12
#define BIT_CH_PAUSE_EN 8
#define BIT_CH_LOAD_EN 4
#define BIT_CH_ENABLE 0

#define BIT_CH_BUSY_STAT 0
#define BIT_LD_BUSY_STAT 4

#define DMA_REG_ADDR_LEAP 0x40

#define DMA_PERIPHERAL_MASK 0xF
#define DMA_ENDIAN_MASK 0x1
#define DMA_CLK_MASK 0x1
#define DMA_INC_MASK 0x1
#define DMA_USIZE_MASK 0x3
#define DMA_BSIZE_MASK 0x7

//#define DMA_ADDR_MASK (~0xA0000000)
#define DMA_ADDR_MASK (0xFFFFFFFF)

#define DMA_WIDTH_MASK 0xFFF
#define DMA_JUMP_MASK 0xFFF
#define DMA_PRIO_MASK 0x3
#define DMA_TRSCNT_MASK 0x7FFFFFFF

#define DMA_WLAST_MASK	0x10000

#define DMA_DST_PERIPHERAL_SHIFT 28
#define DMA_SRC_PERIPHERAL_SHIFT 24
#define DMA_ALU_SHIFT 20
#define DMA_DST_ENDIAN_SHIFT 19
#define DMA_SRC_ENDIAN_SHIFT 18
#define DMA_DST_CLK_SHIFT 17
#define DMA_SRC_CLK_SHIFT 16
#define DMA_DI_SHIFT 13
#define DMA_SI_SHIFT 12
#define DMA_DST_USIZE_SHIFT 10
#define DMA_SRC_USIZE_SHIFT 8
#define DMA_DST_BSIZE_SHIFT 4
#define DMA_SRC_BSIZE_SHIFT 0

#define DMA_WIDTH_SHIFT 16
#define DMA_JUMP_SHIFT 0

#define GET_CHN_REG_ADDR(reg, id) reg + (id * DMA_REG_ADDR_LEAP);

struct mt_dmac_dev
{
	struct device *dev;
	struct class *dmac_class;
	struct clk *clk;
	ulong base;
	unsigned int dmaclk;
	unsigned int irq;
	unsigned int open_cnt;
	unsigned int initflag;
	struct mutex mutex_dmac;
};


#endif // __DMA_SYMPHONY_H__
