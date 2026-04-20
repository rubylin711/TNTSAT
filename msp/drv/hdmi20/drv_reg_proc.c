/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "drv_reg_proc.h"

#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/device.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/poll.h>
//#include <mach/hardware.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "drv_hdmi.h"

//#include "mpi_priv_hdmi.h"
#include "mt_drv_hdmi.h"

#include "mt_unf_hdmi.h"

#include "mt_unf_disp.h"
#include "drv_disp_ext.h"
#include "mt_kernel_adapt.h"
//#include "drv_cipher_ext.h"
#include "mt_drv_sys.h"

#include "drv_global.h"

#include "drv_reg_proc.h"
#include "si_drv_cra_api.h"
#include "si_drv_tx_regs.h"

extern ulong HdmiDrvRegBaseGet(void *p, HDMI_RELATED_REG_MOD_E mod);
extern ulong hdmi_paddr2vaddr(void *p, ulong addr);

void SiiDrvCraWrReg8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t val )
{
	ulong base_addr;
	int ddprint = 1;

	base_addr = HdmiDrvRegBaseGet(NULL, HDMI_MOD_DIG);
	if (((addr >= 0x1010) && (addr <= 0x1017)) || (addr == 0x8cf)\
			|| (addr == 0xe50) || (addr == 0xe4c) || (addr == 0xe4d) || (addr == 0xe4e)\
			|| (addr == 0xfa6)  || (addr == 0xfa7) \
	   ) {
		ddprint = 0;
	} else if ((addr == 0xf4) || (addr == 0xd2) || (addr == 0x642) || (addr == 0xf8f) || (addr == 0xf90)) {
		/* fifo register, read will change the ptr, so can't read again*/
		ddprint = 2;
	}

	writeb(val, (void *)(base_addr + (ulong)addr));
	if (ddprint) {
		if (ddprint == 1) {
			HDMI20_DRV_REG_PROC_PRINTK("\n%s_%d:write 0x%x, after write:reg[%lx]=0x%x\n", __func__, __LINE__, (uint32_t)val, base_addr + addr, (unsigned int)readb((void *)(base_addr + addr)));
		} else {
			HDMI20_DRV_REG_PROC_PRINTK("\n%s_%d:write 0x%x, addr:%lx\n", __func__, __LINE__, (uint32_t)val, (ulong)(base_addr + addr));
		}
	}
}

uint8_t SiiDrvCraRdReg8(SiiInst_t inst, SiiDrvCraAddr_t addr )
{
	uint8_t retVal = 0;
	ulong base_addr;
	int ddprint = 1;

	base_addr = HdmiDrvRegBaseGet(NULL, HDMI_MOD_DIG);
	retVal = readb((void *)(base_addr + addr));
	if (addr == (0x7e) || \
			addr == (0x7f) || \
			addr == (0x925)  || \
			addr == (0xfa6)  || addr == (0xfa7) || \
			((addr >= (0x1010)) && (addr <= (0x1017))) || \
			addr == (0x31)) { // timer read intr registers
		ddprint = 0;
	}
	if (ddprint && addr != REG_ADDR__EMP_CTRL) {
		HDMI20_DRV_REG_PROC_PRINTK("\n[%s_%d]base:0x%lx,offset:0x%0x,val:0x%x\n", __func__, __LINE__, base_addr, (uint32_t)addr, (uint32_t)retVal);
	}

	return retVal;
}

uint8_t SiiDrvCraRdReg8_4dump(SiiInst_t inst, SiiDrvCraAddr_t addr )
{
	uint8_t retVal = 0;
	ulong base_addr;

	base_addr = HdmiDrvRegBaseGet(NULL, HDMI_MOD_DIG);
	retVal = readb((void *)(base_addr + addr));

	return retVal;
}

void SiiDrvCraSetBit8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t mask )
{
	uint8_t val;
	val = SiiDrvCraRdReg8(inst, addr) ;
	val = (val & ((uint8_t)~mask)) | mask;
	SiiDrvCraWrReg8(inst, addr, val);
}

void SiiDrvCraClrBit8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t mask )
{
	uint8_t val;
	val = SiiDrvCraRdReg8(inst, addr) ;
	val = (val & ((uint8_t)~mask));
	SiiDrvCraWrReg8(inst, addr, val);
}

void SiiDrvCraPutBit8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t mask, uint8_t val )
{
	uint8_t temp;
	temp = SiiDrvCraRdReg8(inst, addr);
	temp &= (~mask);
	temp |= (mask & val);
	SiiDrvCraWrReg8(inst, addr, temp);
}

void SiiDrvCraWrReg16(SiiInst_t inst, SiiDrvCraAddr_t addr, uint16_t val )
{
	uint8_t writeVal;
	writeVal = val & 0xff;
	SiiDrvCraWrReg8(inst, addr, writeVal);

	writeVal = (val >> 8) & 0xff;;
	SiiDrvCraWrReg8(inst, (addr + 1), writeVal);
}

void SiiDrvCraWrReg24(SiiInst_t inst, SiiDrvCraAddr_t addr, uint32_t val )
{
	uint8_t writeVal;
	writeVal = val & 0xff;
	SiiDrvCraWrReg8(inst, addr, writeVal);

	writeVal = (val >> 8) & 0xff;;
	SiiDrvCraWrReg8(inst, (addr + 1), writeVal);

	writeVal = (val >> 16) & 0xff;;
	SiiDrvCraWrReg8(inst, (addr + 2), writeVal);
}

void SiiDrvCraFifoRead8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t* pData, SiiDrvCraSize_t size )
{
	uint16_t i;
	for (i = 0; i < size; i++) {
		*pData = SiiDrvCraRdReg8(inst, addr);
		pData++;
	}
}

void SiiDrvCraBlockWrite8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t* pData, SiiDrvCraSize_t size )
{
	uint16_t i;
	for (i = 0; i < size; i++) {
		SiiDrvCraWrReg8(inst, addr + i, pData[i]);
	}
}

void SiiDrvCraBlockRead8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t* pData, SiiDrvCraSize_t size )
{
	uint8_t i;

	for (i = 0; i < size; i++) {
		pData[i] = SiiDrvCraRdReg8(inst, addr + i);
	}
}

bool_t SiiDCardRegBlockRead(SiiInst_t inst, uint8_t deviceId, uint16_t regAddr, uint8_t *pBuffer, uint16_t count)
{
	return false;
}

bool_t SiiDCardRegBlockWrite(SiiInst_t inst, uint8_t deviceId, uint16_t regAddr, uint8_t *pBuffer, uint16_t count)
{
	return false;
}

bool_t SiiDrvCraIsInterruptRcvd(SiiInst_t inst)
{
	inst = inst;
	return true;
}

void    SiiDrvCraWrReg32(SiiInst_t inst, SiiDrvCraAddr_t addr, uint32_t val )
{
	ulong base_addr;
	int ddprint = 1;

	base_addr = HdmiDrvRegBaseGet(NULL, HDMI_MOD_DIG);
	writel(val, (void *)(base_addr + addr));
	if (ddprint && addr < REGTX_SOC_P2_MT) {
		if (ddprint == 1) {
			HDMI20_DRV_REG_PROC_PRINTK("\n%s_%d:write 0x%x, after write:reg[%lx]=0x%x\n", __func__, __LINE__, (uint32_t)val, base_addr + addr, (unsigned int)readl((void *)(base_addr + addr)));
		} else {
			HDMI20_DRV_REG_PROC_PRINTK("\n%s_%d:write 0x%x, addr:%lx\n", __func__, __LINE__, (uint32_t)val, (ulong)(base_addr + addr));
		}
	}
}

uint32_t    SiiDrvCraRdReg32(SiiInst_t inst, SiiDrvCraAddr_t addr )
{
	uint32_t readVal = 0;
	ulong base_addr;
	int ddprint = 1;

	base_addr = HdmiDrvRegBaseGet(NULL, HDMI_MOD_DIG);
	readVal = readl((void *)(base_addr + addr));
	if (ddprint) {
		HDMI20_DRV_REG_PROC_PRINTK("\n[%s_%d]base:0x%lx,offset:0x%0x,val:0x%x\n", __func__, __LINE__, base_addr, (uint32_t)addr, (uint32_t)readVal);
	}

	return readVal;
}

void SiiDrvCraPutBit32(SiiInst_t inst, SiiDrvCraAddr_t addr, uint32_t mask, uint32_t val )
{
	uint32_t temp;
	temp = SiiDrvCraRdReg32(inst, addr);
	temp &= (~mask);
	temp |= (mask & val);
	SiiDrvCraWrReg32(inst, addr, temp);
}

void HdmiPrivDrvWrReg32(SiiInst_t inst, ulong addr, uint32_t val)
{
	addr = hdmi_paddr2vaddr((void *)NULL, addr);
	writel(val, (void *)addr);
	HDMI20_DRV_REG_PROC_PRINTK("\n[%s_%d]write reg[0x%lx],val:0x%x\n", __func__, __LINE__, addr, (uint32_t)val);
}

uint32_t HdmiPrivDrvRdReg32(SiiInst_t inst, ulong addr )
{
	uint32_t retVal = 0;

	addr = hdmi_paddr2vaddr((void *)NULL, addr);
	retVal = readl((void *)addr);
	HDMI20_DRV_REG_PROC_PRINTK("\n[%s_%d]base:0x%lx,val:0x%x\n", __func__, __LINE__, addr, (uint32_t)retVal);

	return retVal;
}

void HdmiPrivDrvSetBit32(SiiInst_t inst, ulong addr, uint32_t mask )
{
	uint32_t val;
	val = HdmiPrivDrvRdReg32(inst, addr) ;
	val = (val & ((uint32_t)~mask)) | mask;
	HdmiPrivDrvWrReg32(inst, addr, val);
}

void HdmiPrivDrvClrBit32(SiiInst_t inst, ulong addr, uint32_t mask )
{
	uint32_t val;
	val = HdmiPrivDrvRdReg32(inst, addr) ;
	val = (val & ((uint32_t)~mask));
	HdmiPrivDrvWrReg32(inst, addr, val);
}

void HdmiPrivDrvPutBit32(SiiInst_t inst, ulong addr, uint32_t mask, uint32_t val )
{
	uint32_t temp;
	temp = HdmiPrivDrvRdReg32(inst, addr);
	temp &= (~mask);
	temp |= (mask & val);
	HdmiPrivDrvWrReg32(inst, addr, temp);
}

