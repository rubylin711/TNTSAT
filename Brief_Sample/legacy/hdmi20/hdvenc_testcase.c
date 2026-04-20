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

#include "drv_suplayer_ioctl.h"
#include "mt_unf_suplayer.h"
#include "hdmi20.h"
#include "hdvenc_testcase.h"
#include "misc_reg_op.h"

void hd_reset_reg(void);
mt_u32 test_share_irq(mt_u32 irq_id, mt_u32 irq_mode);

mt_handle g_drv_sup_test_hdl = (mt_handle)NULL;
mt_u32 g_dac_test_data1[] = {
	0x1ff20e,
	0x21d22c,
	0x23a249,
	0x258266,
	0x275283,
	0x2922a0,
	0x2ae2bc,
	0x2c92d7,
	0x2e42f2,
	0x2ff30b,
	0x318324,
	0x33033c,
	0x347353,
	0x35e368,
	0x37337d,
	0x386390,
	0x3993a2,
	0x3aa3b2,
	0x3ba3c1,
	0x3c83ce,
	0x3d43da,
	0x3df3e4,
	0x3e93ed,
	0x3f03f3,
	0x3f63f9,
	0x3fb3fc,
	0x3fd3fe,
	0x3fe3fe,
	0x3fd3fc,
	0x3fb3f9,
	0x3f63f3,
	0x3f03ed,
	0x3e93e4,
	0x3df3da,
	0x3d43ce,
	0x3c83c1,
	0x3ba3b2,
	0x3aa3a2,
	0x399390,
	0x38637d,
	0x373368,
	0x35e353,
	0x34733c,
	0x330324,
	0x31830b,
	0x2ff2f2,
	0x2e42d7,
	0x2c92bc,
	0x2ae2a0,
	0x292283,
	0x275266,
	0x258249,
	0x23a22c,
	0x21d20e,
	0x1ff1f0,
	0x1e11d2,
	0x1c41b5,
	0x1a6198,
	0x18917b,
	0x16c15e,
	0x150142,
	0x135127,
	0x11a10c,
	0x1000f3,
	0xe60da,
	0xce0c2,
	0xb70ab,
	0xa0096,
	0x8b081,
	0x7806e,
	0x6505c,
	0x5404c,
	0x4403d,
	0x36030,
	0x2a024,
	0x1f01a,
	0x15011,
	0xe00b,
	0x8005,
	0x3002,
	0x1000,
	0x0,
	0x1002,
	0x3005,
	0x800b,
	0xe011,
	0x1501a,
	0x1f024,
	0x2a030,
	0x3603d,
	0x4404c,
	0x5405c,
	0x6506e,
	0x78081,
	0x8b096,
	0xa00ab,
	0xb70c2,
	0xce0da,
	0xe60f3,
	0xff10c,
	0x11a127,
	0x135142,
	0x15015e,
	0x16c17b,
	0x189198,
	0x1a61b5,
	0x1c41d2,
	0x1e11f0
};

/***** local functions *******************************************************/
void hd_reset_reg(void)
{
	mt_u32 ret = 0;
	mt_u32 reg_addr = REG_SYS_BLOCK_RESET;
	mt_u32 reg_val = 0;

	//1��rst_ctrl��Ӧbit����
	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_val &= ~0x10;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_val &= ~0x8;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	//2���ȴ�1ms
	SiiLibTimeMilliDelay(1);

	//3��rst_ctrl��Ӧbit��1
	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_val |= 0x8;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_val |= 0x10;
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	//ʹ��hd������0xbf470014[31]=1
	ret = misc_reg_read32(0xbf470014, &reg_val, 1);
	reg_val |= (1 << 31);
	ret = misc_reg_write32(0xbf470014, &reg_val, 1);
	if ( ret ) {
		return;
	}
}

mt_u32 do_case_hd_reset(void)
{
#define HD_RESET_REG_CNT (2)
	mt_u32 ret = 0;
	mt_u32 reg_val = 0;
	mt_u32 reg_addr_tmp[HD_RESET_REG_CNT];
	mt_u32 reg_addr_val_tmp[HD_RESET_REG_CNT];
	mt_u32 i = 0;
	mt_u32 j = 0;
	mt_u32 result = 0;//0:ok, 1:fail

	//ģʽһ �Զ��ȶԣ�
	for (j = 0; j < 200; j++) {
		memset(reg_addr_tmp, 0, sizeof(reg_addr_tmp));
		memset(reg_addr_val_tmp, 0, sizeof(reg_addr_val_tmp));

		//��ȡ���µ�ַ�ļĴ���
		i = 0;
		reg_addr_tmp[i] = 0xbf470008;
		ret = misc_reg_read32(reg_addr_tmp[i], &(reg_addr_val_tmp[i]), 1);
		i++;
		reg_addr_tmp[i] = 0xbf47000c;
		ret = misc_reg_read32(reg_addr_tmp[i], &(reg_addr_val_tmp[i]), 1);
		i++;

		//�޸�����4���Ĵ���
		reg_val = 0xffffffff;
		for (i = 0; i < HD_RESET_REG_CNT; i++) {
			ret = misc_reg_write32(reg_addr_tmp[i], &reg_val, 1);
		}

		//��������ĸ�λ������и�λ
		hd_reset_reg();

		//��λ��ɺ󣬶�ȡ�����ĸ��Ĵ������������ȡ���Ľ�����бȶԣ����Ƿ�һ��
		for (i = 0; i < HD_RESET_REG_CNT; i++) {
			ret = misc_reg_read32(reg_addr_tmp[i], &reg_val, 1);
			if (reg_val != reg_addr_val_tmp[i]) {
				SSHH_PRINTF("\n%s_%d: reset fail:reg0x%x val0x%x:\n", __func__, __LINE__, reg_addr_tmp[i], reg_val);
				result = 1;
			}
		}
	}

	return (result | ret);
}

mt_u32 do_case_hd_dac_sin(void)
{
	mt_u32 dac_len1 = sizeof(g_dac_test_data1) / sizeof(g_dac_test_data1[0]);
	int i;
	mt_u32 ret = 0;
	mt_u32 reg_addr = 0;
	mt_u32 reg_val = 0;

	SSHH_PRINTF("\n%s_%d: dac_len1:%d\n", __func__, __LINE__, dac_len1);
	reg_addr = 0xbf471500;
	reg_val = 0x80000000 | (dac_len1 - 1);
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	reg_addr = 0xbf471000;
	for (i = 0; i < dac_len1; i++) {
		reg_val = g_dac_test_data1[i];
		SSHH_PRINTF("\n%s_%d: reg:0x%x, val:0x%x\n", __func__, __LINE__, reg_addr + i * 4, reg_val);
		ret = misc_reg_write32(reg_addr + i * 4, &reg_val, 1);
	}

	reg_val = 0x00000000 | (dac_len1 - 1);
	ret = misc_reg_write32(0xbf471500, &reg_val, 1);
	reg_val = 0x101;
	ret = misc_reg_write32(0xbf4700c0, &reg_val, 1);
	reg_val = 0x000;
	ret = misc_reg_write32(0xbf4700ec, &reg_val, 1);
	ret = misc_reg_write32(0xbf4700f0, &reg_val, 1);
	ret = misc_reg_write32(0xbf470094, &reg_val, 1);
	SSHH_PRINTF("\n%s_%d: Please watch the waveform on oscilloscope:\n", __func__, __LINE__);
	return ret;
}

mt_u32 test_share_irq(mt_u32 irq_id, mt_u32 irq_mode)
{
	#if SYM6_COMPILE_ALL_PASS
	SUPLAYER_IRQ_REQUEST_S irq_params;
	int i;
	int ret_tmp;
	mt_u32 ret = 0;
	mt_u32 reg_addr = 0;
	mt_u32 reg_val = 0;
	mt_u32 reg_val_tmp = 0;

	memset(&irq_params, 0, sizeof(SUPLAYER_IRQ_REQUEST_S));
	{
		//create suplayer drv for ioctl test
		int ptr;
		SSHH_PRINTF("\n%s %d create suplayer proc !!!\n", __FUNCTION__, __LINE__);
		ret_tmp = MT_UNF_SUPLAYER_Init();
		if (ret_tmp != MT_SUCCESS) {
			SSHH_PRINTF("\n%s %d create suplayer proc fail!!!\n", __FUNCTION__, __LINE__);
		}
		ret_tmp = MT_UNF_SUPLAYER_Create(&ptr, &g_drv_sup_test_hdl);
		if (ret_tmp != MT_SUCCESS) {
			SSHH_PRINTF("\n%s %d create suplayer proc fail!!!\n", __FUNCTION__, __LINE__);
		}
	}

	irq_params.irq_id = irq_id;
	//#define IRQF_TRIGGER_NONE       0x00000000
	//#define IRQF_TRIGGER_RISING     0x00000001
	//#define IRQF_TRIGGER_FALLING    0x00000002
	//#define IRQF_TRIGGER_HIGH       0x00000004
	//#define IRQF_TRIGGER_LOW        0x00000008
	irq_params.irq_trigger_mode = irq_mode;
	for (i = 0; i < 8; i++) {
		irq_params.irq_name[0] = 'a' + i;
	}
	reg_addr = 0xbf110060;
	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_val_tmp = reg_val;
	SSHH_PRINTF("\n%s %d cpu_int_mask0 0x%x\n", __FUNCTION__, __LINE__, reg_val);
	//reg_val &= ~(1<<20);
	//ret = misc_reg_write32(reg_addr, &reg_val, 1);
	SSHH_PRINTF("\n%s %d cpu_int_mask0 0x%x\n", __FUNCTION__, __LINE__, reg_val);
	//write line count
	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_addr = 0xbf4700f4;
	reg_val &= ~(0xfff);
	reg_val |= 0x100 & 0xfff; //fill in [11:0]
	ret = misc_reg_write32(reg_addr, &reg_val, 1);

	//test [31]
	SSHH_PRINTF("\n%s %d test hd intr[31]\n", __FUNCTION__, __LINE__);
	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_val |= (0xf << 28);
	reg_val &= ~(1 << 31);
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	ret_tmp = MT_UNF_SUPLAYER_Irq_Request(g_drv_sup_test_hdl, &irq_params);

	SiiLibTimeMilliDelay(200);

	//test[30]
	SSHH_PRINTF("\n%s %d test hd intr[30]\n", __FUNCTION__, __LINE__);
	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_val |= (0xf << 28);
	reg_val &= ~(1 << 30);
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	SiiLibTimeMilliDelay(200);

	//test[29]
	SSHH_PRINTF("\n%s %d test hd intr[29]\n", __FUNCTION__, __LINE__);
	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_val |= (0xf << 28);
	reg_val &= ~(1 << 29);
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	SiiLibTimeMilliDelay(200);

	//test[28]
	SSHH_PRINTF("\n%s %d test hd intr[28]\n", __FUNCTION__, __LINE__);
	ret = misc_reg_read32(reg_addr, &reg_val, 1);
	reg_val |= (0xf << 28);
	reg_val &= ~(1 << 28);
	ret = misc_reg_write32(reg_addr, &reg_val, 1);
	SiiLibTimeMilliDelay(200);

	//ret = misc_reg_write32(reg_addr, &reg_val_tmp, 1);
	ret_tmp = MT_UNF_SUPLAYER_Irq_Free(g_drv_sup_test_hdl, &irq_params);
	{
		//destroy suplayer debug proc
		int ret_tmp;
		SSHH_PRINTF("\n%s %d destroy suplayer proc\n", __FUNCTION__, __LINE__);
		ret_tmp = MT_UNF_SUPLAYER_Destroy(g_drv_sup_test_hdl);
		if (ret_tmp != (MT_S32)MT_SUCCESS) {
			SSHH_PRINTF("\n%s %d destroy suplayer proc fail!!!\n", __FUNCTION__, __LINE__);
		}
		ret_tmp = MT_UNF_SUPLAYER_DeInit();
		if (ret_tmp != (MT_S32)MT_SUCCESS) {
			SSHH_PRINTF("\n%s %d destroy suplayer proc fail!!!\n", __FUNCTION__, __LINE__);
		}
		SSHH_PRINTF("\n%s %d destroy suplayer proc end\n", __FUNCTION__, __LINE__);
	}

	return ret;
	#else
	return 0;
	#endif
}

mt_u32 do_case_share_irq(mt_u32 irq)
{
	mt_s32 s32Ret = 0;
	s32Ret = test_share_irq(irq, 1);
	return s32Ret;
}

/***** end of file ***********************************************************/

