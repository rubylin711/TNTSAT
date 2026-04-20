/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "mt_type.h"
#include <time.h>

#include <dlfcn.h>

#include <sys/select.h>
#include <signal.h>
// Copyright ?2007, Deco, Inc.  All rights reserved.
//
// No part of this work may be reproduced, modified, distributed, transmitted,
// transcribed, or translated into any language or computer format, in any form
// or by any means without written permission of: Deco, Inc.,
// 1060 East Arques Avenue, Sunnyvale, California 94085
//------------------------------------------------------------------------------
//#define SII_DEBUG 1

/***** #include statements ***************************************************/
#include "conio.h"
#include "si_datatypes.h"
#include "si_lib_malloc_api.h"
#include "si_lib_seq_api.h"
#include "platform_api.h"
#include "si_lib_obj_api.h"
#include "si_lib_log_api.h"
#include "si_drv_cra_api.h"
#include "si_app_tx_api.h"

#include "misc_reg_op.h"

/***** local functions *******************************************************/
#include "mt_common.h"
#include "si_drv_tx_regs.h"
#define SSHH_PRINTF printf

static ulong sd_venc_reg_virt_addr = (ulong)NULL;
static ulong sd_vbi_reg_virt_addr = (ulong)NULL;
static ulong hd_venc_reg_virt_addr = (ulong)NULL;
static ulong hdmi_reg_virt_addr = (ulong)NULL;
static ulong disp_rst_reg_virt_addr = (ulong)NULL;
static ulong hd_dac_1500_reg_virt_addr = (ulong)NULL;
static ulong hd_dac_1000_reg_virt_addr = (ulong)NULL;
static ulong cpu1_pic_reg_virt_addr = (ulong)NULL;
static ulong clk_reg_virt_addr = (ulong)NULL;
static ulong aout_reg_virt_addr = (ulong)NULL;
static ulong analog_clock_reg_virt_addr = (ulong)NULL;
static ulong disp_44_reg_virt_addr = (ulong)NULL;
static ulong add_1f15_reg_virt_addr = (ulong)NULL;

static AVINFOSTORAGE_T g_hdmi_av_storage = {0};
static AVINFOSTORAGE_T g_hdmi_av_storage_default = {
	0, 12, //tvsys
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{
		0,//i2s
		1,//48k
		2,//2ch
		0,//no downmix
		0,//0:developer, 1:qa
		0,
		0
	},
	0,
	0
};

static mt_u32 g_print_flag = 0;

#if 0
	mt_u32 regfile_mem_init (void);
	mt_u32 regfile_mem_open (ulong base, mt_u32 size, void* sys_reg_virt_addr);
	mt_u32 regfile_mem_open_all (void);
	mt_u32 regfile_mem_close (void* sys_reg_virt_addr);
	mt_u32 regfile_mem_close_all (void);
	mt_u32 regfile_mem_deinit (void);
#endif
mt_void misc_reg_op_print_enable_set(MT_BOOL  enable);
ulong get_func_reg_addr(ulong *funct);
extern mt_s32 mpi_memdev_init(mt_void);
extern mt_s32 mpi_memdev_deinit(mt_void);
extern mt_s32 mpi_memdev_map_register(phys_addr_t RegAddr, mt_u32 u32Length, mt_void **pVirAddr);
extern mt_s32 mpi_memdev_unmap_register(mt_void *pVirAddr);

mt_void misc_reg_op_print_enable_set(MT_BOOL  enable)
{
	g_print_flag = enable;
}

mt_u32 regfile_mem_init (void)
{
	//mt_s32 s32Ret = BB_SUCCESS;
	mpi_memdev_init();
	return BB_SUCCESS;
}

mt_u32 regfile_mem_open (ulong base, mt_u32 size, void* sys_reg_virt_addr)
{
	mt_s32 s32Ret = BB_SUCCESS;
	s32Ret = mpi_memdev_map_register(SYMPHONY_IO_PA(base), size, sys_reg_virt_addr);
	//SSHH_PRINTF("\n%s_%d:%x,%d\n",__func__,__LINE__,*(mt_u32 *)sys_reg_virt_addr, s32Ret);
	return s32Ret;
}

mt_u32 regfile_mem_open_all (void)
{
	mt_s32 ret = BB_SUCCESS;
	ret = regfile_mem_open (0x1f460000UL, 2048, (void *)&sd_venc_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: open sd venc register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_open (0x1f450000UL, 1024, (void *)&sd_vbi_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: open sd vbi register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_open (0x1f470000UL, 1024, (void *)&hd_venc_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: open hd venc register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_open (HDMI20_REG_BASE_ADDR, HDMI_REGISTER_FILE_SIZE, (void *)&hdmi_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: open hdmi register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_open (0x1f50a600UL, 1024, (void *)&disp_rst_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: open hdmi phy register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_open (0x1f471000UL, 1024, (void *)&hd_dac_1000_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: open hd dac 1000 register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_open (0x1f471500UL, 1024, (void *)&hd_dac_1500_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: open hd dac 1500 register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_open (0x1f110000UL, 1024, (void *)&cpu1_pic_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: open cpu1 pic register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_open (0x1f50a500UL, 1024, (void *)&clk_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: open clock register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_open (0x1f490000UL, 1024, (void *)&aout_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: open aout register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_open (0x1f5d0000UL, 1024, (void *)&analog_clock_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: open analog clock register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_open (0x1f440000UL, 1024, (void *)&disp_44_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: open disp_44 register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_open (0x1f157000, 1024, (void *)&add_1f15_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: open 1f15 register file fail\n", __func__, __LINE__);
	}
	return BB_SUCCESS;
}

mt_u32 regfile_mem_close (void* sys_reg_virt_addr)
{
	mpi_memdev_unmap_register((void *)(*(ulong*)sys_reg_virt_addr));
	//SSHH_PRINTF("\n%s_%d:%px\n",__func__,__LINE__,*(ulong *)sys_reg_virt_addr);
	*(ulong *)sys_reg_virt_addr = (ulong)NULL;
	return BB_SUCCESS;
}

mt_u32 regfile_mem_close_all (void)
{
	mt_s32 ret = BB_SUCCESS;
	ret = regfile_mem_close ((void *)&sd_venc_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: close sd venc register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_close ((void *)&sd_vbi_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: close sd vbi register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_close ((void *)&hd_venc_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: close hd venc register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_close ((void *)&hdmi_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: close hdmi register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_close ((void *)&disp_rst_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: close rst register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_close ((void *)&hd_dac_1500_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: close hd dac 1500 register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_close ((void *)&hd_dac_1000_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: close hd dac 1000 register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_close ((void *)&cpu1_pic_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: close cpu1 register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_close ((void *)&clk_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: close clock register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_close ((void *)&aout_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: close aout register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_close ((void *)&analog_clock_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: close analog clock register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_close ((void *)&disp_44_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: close disp_44 register file fail\n", __func__, __LINE__);
	}
	ret = regfile_mem_close ((void *)&add_1f15_reg_virt_addr);
	if (ret) {
		SSHH_PRINTF("\n%s_%d: close 1f15 register file fail\n", __func__, __LINE__);
	}

	return BB_SUCCESS;
}

mt_u32 regfile_mem_deinit (void)
{
	//mt_s32 s32Ret = MT_SUCCESS;
	mpi_memdev_deinit();
	return BB_SUCCESS;
}

ulong get_func_reg_addr(ulong *funct)
{
	ulong reg_base_addr = 0;
	ulong func = *funct;

	if ((func & 0x0fff0000) == 0x0f450000) {
		reg_base_addr = sd_vbi_reg_virt_addr;
	} else if ((func & 0x0fff0000) == 0x0f460000) {
		reg_base_addr = sd_venc_reg_virt_addr;
	} else if ((func & 0x0ffff000) == 0x0f470000) {
		reg_base_addr = hd_venc_reg_virt_addr;
	} else if ((func & 0x0fff0000) == (HDMI20_REG_BASE_ADDR & 0x0fff0000)) {
		reg_base_addr = hdmi_reg_virt_addr;
	} else if ((func & 0x0fffff00) == 0x0f50a600) {
		reg_base_addr = disp_rst_reg_virt_addr;
	} else if ((func & 0x0fffff00) == 0x0f50a500) {
		reg_base_addr = clk_reg_virt_addr;
	} else if ((func & 0x0ffff000) == 0x0f471000) {
		if ((func & 0x00000f00) == 0x00000500) {
			reg_base_addr = hd_dac_1500_reg_virt_addr;
		} else if ((func & 0x00000f00) < 0x00000500) {
			reg_base_addr = hd_dac_1000_reg_virt_addr;
		}
	} else if ((func & 0x0fff0000) == 0x0f110000) {
		reg_base_addr = cpu1_pic_reg_virt_addr;
	} else if ((func & 0x0fff0000) == 0x0f490000) {
		reg_base_addr = aout_reg_virt_addr;
	} else if ((func & 0x0fff0000) == 0x0f5d0000) {
		reg_base_addr = analog_clock_reg_virt_addr;
	} else if ((func & 0x0fff0000) == 0x0f440000) {
		reg_base_addr = disp_44_reg_virt_addr;
	} else if ((func & 0x0ffff000) == 0x0f157000) {
		reg_base_addr = add_1f15_reg_virt_addr;
	}

	if ((((func & 0x0fff0000UL) == 0x0f500000UL) && \
			(((func & 0x0000ff00UL) == 0x0000a600UL) || ((func & 0x0000ff00UL) == 0x0000a500UL))) \
			|| ((func & 0x0fff0000UL) == 0x0f150000UL) \
			|| ((func & 0x0ffff000UL) == 0x0f5d7000UL)) {
		(*funct) &= 0x000000ffUL;
	} else if ((func & 0x0ffff000UL) == 0x0f471000UL) {
		if ((func & 0x00000f00UL) == 0x00000500UL) {
			(*funct) &= 0x000000ffUL;
		} else if ((func & 0x00000f00UL) < 0x00000500UL) {
			(*funct) &= 0x00000fffUL;
		}
	} else {
		(*funct) &= 0x0000ffffUL;
	}
	return reg_base_addr;
}

mt_u32 misc_reg_read8 (ulong regAddr, mt_u8* pData, mt_u32 length)
{
	int i = 0;
	ulong sys_reg_virt_addr = 0;

	if ( regAddr >= BASE_ADDRESS && regAddr <= (BASE_ADDRESS + HDMI_REGISTER_FILE_SIZE)) {
		regAddr += HDMI20_REG_BASE_ADDR;
		regAddr -= BASE_ADDRESS;
	}
	sys_reg_virt_addr = get_func_reg_addr(&regAddr);
	if (sys_reg_virt_addr == 0) {
		return BB_FAILURE;
	}
	for (i = 0; i < length; i++) {
		pData[i] = hdmi20_read_reg8((sys_reg_virt_addr + regAddr + i));

		if (g_print_flag) {
			SSHH_PRINTF("\n%s_%d:vaddr%lx,reg0x%lx,d[%d]=%x\n", __func__, __LINE__, sys_reg_virt_addr, regAddr + i, i, (int)pData[i]);
		}
	}

	return BB_SUCCESS;
}

mt_u8 misc_reg_read8_single (ulong regAddr)
{
	ulong sys_reg_virt_addr = 0;
	mt_u8 val;

	if ( regAddr >= BASE_ADDRESS && regAddr <= (BASE_ADDRESS + HDMI_REGISTER_FILE_SIZE)) {
		regAddr += HDMI20_REG_BASE_ADDR;
		regAddr -= BASE_ADDRESS;
	}
	sys_reg_virt_addr = get_func_reg_addr(&regAddr);
	if (sys_reg_virt_addr == 0) {
		return BB_FAILURE;
	}
	val = hdmi20_read_reg8(sys_reg_virt_addr + regAddr);

	if (g_print_flag) {
		SSHH_PRINTF("\n%s_%d:vaddr%lx,reg0x%lx,d=%x\n", __func__, __LINE__, sys_reg_virt_addr, regAddr, (mt_u32)val);
	}

	return val;
}

mt_u32 misc_reg_write8_single (ulong regAddr, mt_u8 val)
{
	ulong sys_reg_virt_addr = 0;

	if ( regAddr >= BASE_ADDRESS && regAddr <= (BASE_ADDRESS + HDMI_REGISTER_FILE_SIZE)) {
		regAddr += HDMI20_REG_BASE_ADDR;
		regAddr -= BASE_ADDRESS;
	}
	sys_reg_virt_addr = get_func_reg_addr(&regAddr);

	if (sys_reg_virt_addr == 0) {
		return BB_FAILURE;
	}

	hdmi20_write_reg8((sys_reg_virt_addr + regAddr), val);

	if (g_print_flag) {
		SSHH_PRINTF("\n%s_%d:vaddr%lx,reg%lx,data:0x%x after write:d=%x\n", __func__, __LINE__, sys_reg_virt_addr, regAddr, val, (unsigned int)hdmi20_read_reg8((sys_reg_virt_addr + regAddr)));
	}
	return BB_SUCCESS;
}

mt_u32 misc_reg_write8 (ulong regAddr, mt_u8* Data, mt_u32 length)
{
	int i = 0;
	ulong sys_reg_virt_addr = 0;

	if ( regAddr >= BASE_ADDRESS && regAddr <= (BASE_ADDRESS + HDMI_REGISTER_FILE_SIZE)) {
		regAddr += HDMI20_REG_BASE_ADDR;
		regAddr -= BASE_ADDRESS;
	}
	sys_reg_virt_addr = get_func_reg_addr(&regAddr);

	if (sys_reg_virt_addr == 0) {
		return BB_FAILURE;
	}

	for (i = 0; i < length; i++) {
		hdmi20_write_reg8((sys_reg_virt_addr + regAddr + i), Data[i]);

		if (g_print_flag) {
			SSHH_PRINTF("\n%s_%d:vaddr%lx,reg%lx,data:0x%x after write:d[%d]=%x\n", __func__, __LINE__, sys_reg_virt_addr, regAddr + i, (mt_u32)Data[i], i, (unsigned int)hdmi20_read_reg8((sys_reg_virt_addr + regAddr + i)));
		}
	}
	return BB_SUCCESS;
}

mt_u32 misc_reg_put8 (ulong regAddr, mt_u8 mask, mt_u8 val)
{
	ulong sys_reg_virt_addr = 0;
	mt_u8 temp;
	ulong regAddr_save;

	if ( regAddr >= BASE_ADDRESS && regAddr <= (BASE_ADDRESS + HDMI_REGISTER_FILE_SIZE)) {
		regAddr += HDMI20_REG_BASE_ADDR;
		regAddr -= BASE_ADDRESS;
	}

	regAddr_save = regAddr;
	sys_reg_virt_addr = get_func_reg_addr(&regAddr);

	if (sys_reg_virt_addr == 0) {
		return BB_FAILURE;
	}

	if ((regAddr_save & 0x0fffffff) == ((HDMI20_REG_BASE_ADDR + 0xf4) & 0x0fffffff)) { //fifo reg, shall not read
		hdmi20_write_reg8((sys_reg_virt_addr + regAddr), val);
	} else {
		temp = hdmi20_read_reg8(sys_reg_virt_addr + regAddr);
		temp &= (~mask);
		temp |= (mask & val);
		hdmi20_write_reg8((sys_reg_virt_addr + regAddr), temp);
		if (regAddr != 0x4c)

			if (g_print_flag) {
				SSHH_PRINTF("\n%s_%d:vaddr%lx,reg%lx,data:0x%x after write:data=%x\n", __func__, __LINE__, sys_reg_virt_addr, regAddr, (mt_u32)temp, (unsigned int)hdmi20_read_reg8((sys_reg_virt_addr + regAddr)));
			}
	}
	return BB_SUCCESS;
}

mt_u32 misc_reg_read32 (ulong regAddr, mt_u32* pData, mt_u32 length)
{
	int i = 0;
	ulong sys_reg_virt_addr = 0;

	if ( regAddr >= BASE_ADDRESS && regAddr <= (BASE_ADDRESS + HDMI_REGISTER_FILE_SIZE)) {
		regAddr += HDMI20_REG_BASE_ADDR;
		regAddr -= BASE_ADDRESS;
	}
	sys_reg_virt_addr = get_func_reg_addr(&regAddr);
	if (sys_reg_virt_addr == 0) {
		return BB_FAILURE;
	}
	for (i = 0; i < length; i++) {
		pData[i] = hdmi20_read_reg32((sys_reg_virt_addr + regAddr + i));

		if (g_print_flag) {
			SSHH_PRINTF("\n%s_%d:vaddr%lx,reg0x%lx,d[%d]=%x\n", __func__, __LINE__, sys_reg_virt_addr, regAddr + i, i, (int)pData[i]);
		}
	}

	return BB_SUCCESS;
}

mt_u32 misc_reg_read32_single (ulong regAddr)
{
	ulong sys_reg_virt_addr = 0;
	mt_u32 val;

	if ( regAddr >= BASE_ADDRESS && regAddr <= (BASE_ADDRESS + HDMI_REGISTER_FILE_SIZE)) {
		regAddr += HDMI20_REG_BASE_ADDR;
		regAddr -= BASE_ADDRESS;
	}
	sys_reg_virt_addr = get_func_reg_addr(&regAddr);
	if (sys_reg_virt_addr == 0) {
		return BB_FAILURE;
	}
	val = hdmi20_read_reg32(sys_reg_virt_addr + regAddr);

	if (g_print_flag) {
		SSHH_PRINTF("\n%s_%d:vaddr%lx,reg0x%lx,d=%x\n", __func__, __LINE__, sys_reg_virt_addr, regAddr, val);
	}

	return val;
}

mt_u32 misc_reg_write32_single (ulong regAddr, mt_u32 val)
{
	ulong sys_reg_virt_addr = 0;
	//mt_u32 val;

	if ( regAddr >= BASE_ADDRESS && regAddr <= (BASE_ADDRESS + HDMI_REGISTER_FILE_SIZE)) {
		regAddr += HDMI20_REG_BASE_ADDR;
		regAddr -= BASE_ADDRESS;
	}
	sys_reg_virt_addr = get_func_reg_addr(&regAddr);

	if (sys_reg_virt_addr == 0) {
		return BB_FAILURE;
	}

	hdmi20_write_reg32((sys_reg_virt_addr + regAddr), val);

	if (g_print_flag) {
		if (regAddr != 0x4c) {
			SSHH_PRINTF("\n%s_%d:vaddr%lx,reg%lx,data:0x%x after write:d=%x\n", __func__, __LINE__, sys_reg_virt_addr, regAddr, val, (unsigned int)hdmi20_read_reg32(sys_reg_virt_addr + regAddr));
		}
	}
	return BB_SUCCESS;
}

mt_u32 misc_reg_write32 (ulong regAddr, mt_u32* Data, mt_u32 length)
{
	int i = 0;
	ulong sys_reg_virt_addr = 0;

	if ( regAddr >= BASE_ADDRESS && regAddr <= (BASE_ADDRESS + HDMI_REGISTER_FILE_SIZE)) {
		regAddr += HDMI20_REG_BASE_ADDR;
		regAddr -= BASE_ADDRESS;
	}
	sys_reg_virt_addr = get_func_reg_addr(&regAddr);

	if (sys_reg_virt_addr == 0) {
		return BB_FAILURE;
	}

	for (i = 0; i < length; i++) { //length === 1
		hdmi20_write_reg32((sys_reg_virt_addr + regAddr + i), Data[i]);

		if (g_print_flag) {
			if (regAddr != 0x4c) {
				SSHH_PRINTF("\n%s_%d:vaddr%lx,reg%lx,data:0x%x after write:d[%d]=%x\n", __func__, __LINE__, sys_reg_virt_addr, regAddr + i, Data[i], i, (unsigned int)hdmi20_read_reg32((sys_reg_virt_addr + regAddr + i)));
			}
		}
	}
	return BB_SUCCESS;
}

mt_u32 misc_reg_put32 (ulong regAddr, mt_u32 mask, mt_u32 val)
{
	ulong sys_reg_virt_addr = 0;
	mt_u32 temp;

	if ( regAddr >= BASE_ADDRESS && regAddr <= (BASE_ADDRESS + HDMI_REGISTER_FILE_SIZE)) {
		regAddr += HDMI20_REG_BASE_ADDR;
		regAddr -= BASE_ADDRESS;
	}
	sys_reg_virt_addr = get_func_reg_addr(&regAddr);

	if (sys_reg_virt_addr == 0) {
		return BB_FAILURE;
	}

	temp = hdmi20_read_reg32(sys_reg_virt_addr + regAddr);
	temp &= (~mask);
	temp |= (mask & val);
	hdmi20_write_reg32((sys_reg_virt_addr + regAddr), temp);

	if (g_print_flag) {
		if (regAddr != 0x4c) {
			SSHH_PRINTF("\n%s_%d:vaddr%lx,reg%lx,data:0x%x after write:data=%x\n", __func__, __LINE__, sys_reg_virt_addr, regAddr, temp, (unsigned int)hdmi20_read_reg32((sys_reg_virt_addr + regAddr)));
		}
	}

	return BB_SUCCESS;
}

mt_u32 get_hdmi_storage_params(AVINFOSTORAGE_T *param)
{
	mt_u32 ret = BB_SUCCESS;
	if (param) {
		if (g_hdmi_av_storage.init == 0) {
			SSHH_PRINTF("\n%s_%d: get default params\n", __func__, __LINE__);
			*param = g_hdmi_av_storage_default;
		} else {
			//SSHH_PRINTF("\n%s_%d: get params\n",__func__,__LINE__);
			*param = g_hdmi_av_storage;
		}
	} else {
		SSHH_PRINTF("\n%s_%d: Invalid input params\n", __func__, __LINE__);
		ret = BB_FAILURE;
	}
	return ret;
}

mt_void set_hdmi_storage_params(AVINFOSTORAGE_T param)
{
	g_hdmi_av_storage = param;
	if (g_hdmi_av_storage.init == 0) {
		g_hdmi_av_storage.init = 1;
	}
	SSHH_PRINTF("\n%s_%d: tvsys[%d,%d]\n", __func__, __LINE__, param.tvsys, g_hdmi_av_storage.tvsys);
}

/***** end of file ***********************************************************/

