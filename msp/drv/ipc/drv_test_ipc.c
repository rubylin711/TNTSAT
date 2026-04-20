/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : drv_test_ipc.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/11/22
 * Description    : Linux IPC Driver test cases.
 * History        :
 * 1.Date         : 2017/11/22
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
/*
 * How To:
 *   1.copy hal_ipc_client.c to linux/msp/drv/ipc/
 *   2.copy drv_test_ipc.c to linux/msp/drv/ipc/
 *   3.modify linux/msp/drv/ipc/ipc_g.c, add
 *     + #include "drv_test_ipc.c"
 *   4.modify linux/msp/drv/ipc/ipc_g.c,
 *     add call drv_test_ipc() in ipc_init() function.
 *     to test IPC.
 *       ipc_init()
 *       {
 *     +     drv_test_ipc();
 *       }
 *   5.then load firmware binary & kernel binary to run IPC test
 */
#include "hal_ipc_client.c"
#include "mt_drv_log.h"
#include "mt_debug.h"
#ifndef CONFIG_MT_CHIP_SYMPHONY6   //remove sym6 unuse code

static ipc_msg_t test_ipc_msg;
static volatile MT_BOOL test_msg_received;

static int test_ipc_msg_handler(void *handle, ipc_msg_t *msg, void* cookie)
{
	memcpy(&test_ipc_msg, msg, sizeof(ipc_msg_t));
	test_msg_received = TRUE;
	return 0;
}

static void drv_test_ipc(void)
{
	void *client;
	unsigned int msg_counter = 0;
	//ipc_msg_t msg;
	RET_CODE ret;
	int int_val;
	int retval;

	MT_INFO_LOG("\ndrv_test_ipc start\n");

	client = ipc_client_create(AP_TESTCASE_IPC, AV_TESTCASE_IPC, test_ipc_msg_handler, NULL);
	if (client == NULL)
	{
		MT_ERR_LOG("Error-0: ipc_client_create failed!\n");
		return;
	}

	while (1)
	{
		//test transfer phy mem addr
		int_val = 0x12345678;
		retval = -1;
		test_msg_received = FALSE;

		ret = ipc_client_send_message_sync(client,
											msg_counter, 0xaabbccdd, 0xccddeeff,
											(ulong)&int_val, sizeof(int),
											0xFFFFFFFF,
											&retval);
		if (ret == SUCCESS)
		{
			if (int_val == 0x87654321)
			{
				MT_INFO_LOG("1-ipc server modified the test int val from %x to %x success.\n",0x12345678,0x87654321);
			}
			else
			{
				MT_ERR_LOG("Error-1: ipc server not modify the test int val from %x to %x!\n",0x12345678,0x87654321);
			}

			if (retval == 777)
			{
				MT_INFO_LOG("2-ipc server remote call return %d success.\n",retval);
			}
			else
			{
				MT_ERR_LOG("Error-2: ipc server remote call return %d failed, expect %d.\n",retval,777);
			}
		}
		else
		{
			MT_ERR_LOG("Error-3: ipc_client_send_message_sync(%x) failed return %d!\n",msg_counter,ret);
			msg_counter ++;
			continue;
		}

#if 0
		do
		{
			ret = ipc_client_receive_message(client, &msg);
		} while (ret != SUCCESS);

		//ASSERT(msg.msg_id == msg_counter);
		if (msg.msg_id == msg_counter
			&& msg.param1 == 0xaabbccdd
			&& msg.param2 == 0xccddeeff)
		{
			MT_ERR_LOG("3-ipc client send & receive msg(%x) success.\n", msg.msg_id);
		}
		else
		{
			MT_ERR_LOG("Error-4: received msg(%x) != %x!\n", msg.msg_id, msg_counter);
		}
#else
		while (!test_msg_received)
		{
			msleep(1);
		}

		if (test_ipc_msg.msg_id == msg_counter
			&& test_ipc_msg.param1 == 0xaabbccdd
			&& test_ipc_msg.param2 == 0xccddeeff)
		{
			MT_INFO_LOG("3-ipc client send & receive msg(%x) success.\n", test_ipc_msg.msg_id);
		}
		else
		{
			MT_ERR_LOG("Error-4: received msg(%x) != %x!\n", test_ipc_msg.msg_id, msg_counter);
		}
#endif

		msg_counter ++;
	}

	MT_INFO_LOG("\ndrv_test_ipc end\n");
	return;
}
#endif

