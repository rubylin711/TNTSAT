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

#include "hdmi20.h"
#include "sdvenc_testcase.h"
#include "misc_reg_op.h"

void sd_reset_reg(void);

/***** local functions *******************************************************/
mt_u32 do_case_sd_color_bar(void)
{
	mt_u32 ret = 0;
	return ret;
}

void sd_reset_reg(void)
{
	mt_u32 ret = 0;
	mt_u32 reg_addr = REG_SYS_BLOCK_RESET;
	mt_u32 reg_val = 0;

	//1：rst_ctrl相应bit清零
	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_val &= ~0x5;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_val &= ~0x2;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	//2：等待1ms
	SiiLibTimeMilliDelay(1);

	//3：rst_ctrl相应bit置1
	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_val |= 0x2;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_val |= 0x5;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	if (ret) {
		return;
	}
}

mt_u32 do_case_sd_reset(void)
{
	mt_u32 ret = 0;
	mt_u32 reg_val = 0;
	mt_u32 reg_addr_tmp[SD_RESET_REG_CNT];
	mt_u32 reg_addr_val_tmp[SD_RESET_REG_CNT];
	mt_u32 i = 0;
	mt_u32 j = 0;
	mt_u32 result = 0;//0:ok, 1:fail

	//模式一 自动比对：
	for (j = 0; j < 200; j++) {
		memset(reg_addr_tmp, 0, sizeof(reg_addr_tmp));
		memset(reg_addr_val_tmp, 0, sizeof(reg_addr_val_tmp));

		//读取如下地址的寄存器
		i = 0;
		reg_addr_tmp[i] = 0x1f460018;
		ret = misc_reg_read32(reg_addr_tmp[i], &(reg_addr_val_tmp[i]), 1);
		i++;
		reg_addr_tmp[i] = 0x1f46001c;
		ret = misc_reg_read32(reg_addr_tmp[i], &(reg_addr_val_tmp[i]), 1);
		i++;

		//修改上面4个寄存器
		reg_val = 0xffffffff;
		for (i = 0; i < SD_RESET_REG_CNT; i++) {
			ret = misc_reg_write32(reg_addr_tmp[i], &reg_val, 1);
		}

		//按照上面的复位步骤进行复位
		sd_reset_reg();

		//复位完成后，读取上面四个寄存器，和最初读取到的结果进行比对，看是否一致
		for (i = 0; i < SD_RESET_REG_CNT; i++) {
			ret = misc_reg_read32(reg_addr_tmp[i], &reg_val, 1);
			if (reg_val != reg_addr_val_tmp[i]) {
				SSHH_PRINTF("\n%s_%d: reset fail:reg0x%x val0x%x:\n", __func__, __LINE__, reg_addr_tmp[i], reg_val);
				result = 1;
			}
		}
	}

	return (result | ret);
}

/***** end of file ***********************************************************/

