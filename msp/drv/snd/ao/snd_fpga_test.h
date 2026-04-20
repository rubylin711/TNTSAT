/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* drv_snd_ao.c <2022-10-24>																*/
/********************************************************************************************/
#ifndef __SND_FPGA_TEST_H__
#define __SND_FPGA_TEST_H__

#include "snd/snd_api.h"


//#define AOUT_FPGA_TEST 1

int fpga_test_request_irq(void);
mt_s32 fpga_test_hdmi_config(aud_param_t *param);

#endif
