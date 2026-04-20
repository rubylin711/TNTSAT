/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : drv_test_rpc.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/12/04
 * Description    : Linux RPC Driver test cases.
 * History        :
 * 1.Date         : 2017/12/04
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/delay.h>
#include <linux/time.h>
#include "rpc_id.h"
#include "mt_drv_log.h"
#include "mt_debug.h"

#define START_TIME	struct timeval begin;do{do_gettimeofday(&begin);}while(0)
#define END_TIME	struct timeval end;do{do_gettimeofday(&end); \
						printk("ipc com time %u - %u = %d(us)\n",end.tv_usec,begin.tv_usec,end.tv_usec-begin.tv_usec);}while(0)

static volatile MT_BOOL test_rpc_msg_received;

static struct {
	unsigned int what;
	unsigned int param1;
	unsigned int param2;
} test_rpc_msg;

//AP CPU's proxy function remote called by AV CPU
static RPC_RETVAL rpc_ap_proxy_fn_example(unsigned int what, unsigned int param1, unsigned int param2, struct rpc_ext_arg_t *ext)
{
	test_rpc_msg.what = what;
	test_rpc_msg.param1 = param1;
	test_rpc_msg.param2 = param2;

	test_rpc_msg_received = TRUE;
	return 0;
}

//AP CPU call AV CPU's remote function example
static int rpc_call_av_example(unsigned int what, unsigned int param1, unsigned int param2, int *arg, unsigned int size)
{
	struct rpc_ext_arg_t ext;
	ext.size = size;
	ext.arg = arg;

	return rpc_call(RPC_AV_FN_ID_EXAMPLE, what, param1, param2, &ext);
}

static void drv_test_rpc(void)
{
	unsigned int msg_counter = 0;
	RET_CODE ret;
	int int_val;

	MT_INFO_LOG("\ndrv_test_rpc start\n");

	/*ret = rpc_init();
	if (ret != SUCCESS)
	{
		printk("Error-0: rpc_init failed!\n");
		return;
	}*/

	rpc_register(RPC_AP_FN_ID_EXAMPLE, rpc_ap_proxy_fn_example);

	while (1)
	{
		//test transfer phy mem addr
		int_val = 0x12345678;
		test_rpc_msg_received = FALSE;

		START_TIME;
		ret = rpc_call_av_example(msg_counter, 0xaabbccdd, 0xccddeeff, &int_val, sizeof(int));
		END_TIME;
		if (ret == 777)
		{
			MT_INFO_LOG("1-remote call return %d success.\n",ret);

			if (int_val == 0x87654321)
			{
				MT_INFO_LOG("2-remote call modified the test int val from %x to %x success.\n",0x12345678,0x87654321);
			}
			else
			{
				MT_INFO_LOG("Error-2: remote call not modify the test int val from %x to %x!\n",0x12345678,0x87654321);
			}
		}
		else
		{
			MT_ERR_LOG("Error-1: remote call(%x) failed, return %d!\n",msg_counter,ret);
			msg_counter ++;
			continue;
		}

		while (!test_rpc_msg_received)
		{
			msleep(1);
		}

		if (test_rpc_msg.what == msg_counter
			&& test_rpc_msg.param1 == 0xaabbccdd
			&& test_rpc_msg.param2 == 0xccddeeff)
		{
			MT_INFO_LOG("3-remote call proxy receive msg(%x) success.\n", test_rpc_msg.what);
		}
		else
		{
			MT_ERR_LOG("Error-3: remote call proxy received msg(%x) != %x!\n", test_rpc_msg.what, msg_counter);
		}

		msg_counter ++;
	}

	MT_INFO_LOG("\ndrv_test_rpc end\n");
	return;
}

