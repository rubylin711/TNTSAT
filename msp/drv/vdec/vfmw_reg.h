/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : vfmw_reg.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/12/21
 * Description    : MT VFMW Register(partly opened for VDEC DRV).
 * History        :
 * 1.Date         : 2017/12/21
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __MT_VFMW_BR_REG_H__
#define __MT_VFMW_BR_REG_H__

//#define REG_VDEC_BASE					0xBF410000
extern ulong g_reg_vdec_base;
#define REG_VDEC_BASE					(g_reg_vdec_base)
#define REG_VDEC_RANGE					0x400

/* VDEC CH0 ES Buffer Base Addr: Bit 16~31, 8K aligned */
#define REG_VDEC_CH_0_ADDR_8K			(REG_VDEC_BASE + 0xA4)
/* VDEC CH0 ES Buffer Read Pointer */
#define REG_VDEC_CH_0_RD_PTR			(REG_VDEC_BASE + 0xB0)
/* VDEC CH0 ES Buffer Write Pointer */
#define REG_VDEC_CH_0_WR_PTR			(REG_VDEC_BASE + 0xB4)
/* VDEC CH0 ES Buffer Used Size */
#define REG_VDEC_CH_0_USED_SZ			(REG_VDEC_BASE + 0xC4)
/* VDEC CH0 ES Buffer Size: Bit 0~21, 8 byte aligned, tail 8 byte reserved */
#define REG_VDEC_CH_0_SZ_8				(REG_VDEC_BASE + 0xBC)

//#define REG_TSI_BASE					0xBF200000
extern ulong g_reg_tsi_base;
#define REG_TSI_BASE					(g_reg_tsi_base)

/* DMX CH0 ES Descripter Start Addr */
#define REG_DMX_CH_0_DESC_START_ADDR	(REG_TSI_BASE + 0x60000 + 0x00000118)
/* DMX CH0 ES Buffer Start Addr */
#define REG_DMX_CH_0_START_ADDR			(REG_TSI_BASE + 0x60000 + 0x0000011C)
/* DMX CH0 ES Descripter End Addr */
#define REG_DMX_CH_0_DESC_END_ADDR		(REG_TSI_BASE + 0x60000 + 0x00000120)
/* DMX CH0 ES Buffer End Addr */
#define REG_DMX_CH_0_END_ADDR			(REG_TSI_BASE + 0x60000 + 0x00000124)
/* DMX CH0 ES Descripter Read Pointer */
#define REG_DMX_CH_0_DESC_RD_PTR		(REG_TSI_BASE + 0x60000 + 0x00000128)
/* DMX CH0 ES Buffer Read Pointer */
#define REG_DMX_CH_0_RD_PTR				(REG_TSI_BASE + 0x60000 + 0x0000012C)
/* DMX CH0 ES Descripter Write Pointer */
#define REG_DMX_CH_0_DESC_WR_PTR		(REG_TSI_BASE + 0x60000 + 0x00000130)
/* DMX CH0 ES Buffer Write Pointer */
#define REG_DMX_CH_0_WR_PTR				(REG_TSI_BASE + 0x60000 + 0x00000134)


static __inline__ unsigned int VFMW_REG_READ(unsigned long reg)
{
	return readl((volatile void*)reg);
}

static __inline__ void VFMW_REG_WRITE(unsigned long reg, unsigned int val)
{
	writel(val, (volatile void*)reg);
}

#endif

