/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : rpc.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/12/04
 * Description    : MT Remote Call between AP & AV CPU.
 * History        :
 * 1.Date         : 2017/12/04
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#endif
#include "mt_type.h"
#ifndef __KERNEL__
#include "sys_define.h"
#endif
#include "mt_cache.h"
#include "ipc.h"
#ifdef __KERNEL__
#include "hal_ipc_client.h"
#else
#include "hal_ipc_service.h"
#endif
#include "rpc.h"

//Driver Test RPC with AV CPU
//#define DRV_TEST_RPC
#ifdef DRV_TEST_RPC
#include "drv_test_rpc.c"
#endif

//IPC task stack
#ifndef __KERNEL__
#define RPC_IPC_TASK_STK_SIZE			(128*1024)
static unsigned int RPC_IPC_TASK_STACK[RPC_IPC_TASK_STK_SIZE/4] __attribute__((aligned(8)));
#endif

//IPC handle
#ifdef __KERNEL__
static IPC_CLIENT_HANDLE g_rpc_ipc_hdl = NULL;
#else
static IPC_SERVICE_HANDLE g_rpc_ipc_hdl = NULL;
#endif

//-----------------------------------------------------------------------------//

//RPC - IPC message handler
static IPC_RETVAL rpc_ipc_msg_handler(void *handle, ipc_msg_t *msg, void* cookie)
{
	rpc_proxy_fn_t fn;
	RPC_FN_ID id;
	unsigned int what;
	unsigned int param1;
	unsigned int param2;
	struct rpc_ext_arg_t arg_ext;
	struct rpc_ext_arg_t *arg = NULL;
	int ret;

	if (msg == NULL)
	{
		RPC_ERROR("%s: msg is NULL!\n",__FUNCTION__);
		return ERR_FAILURE;
	}

	if ((msg->app_id & IPC_APP_RPC_FLAG) == 0)
	{
		RPC_ERROR("%s: msg app_id(%x) is not RPC(%x)!\n",__FUNCTION__,msg->app_id,IPC_APP_RPC_FLAG);
		return ERR_FAILURE;
	}

	id = msg->app_id & IPC_APP_MASK;
	what = msg->msg_id;
	param1 = msg->param1;
	param2 = msg->param2;

	if (msg->phy_addr != 0 && msg->mem_size > 0)
	{
		arg_ext.size = msg->mem_size;
		/* FIXME: phy addr to vir addr? */
		arg_ext.arg = (void*)(ulong)msg->phy_addr;
		arg = &arg_ext;
	}

	fn = rpc_get_proxy_fn(id);
	if (fn != NULL)
	{
		RPC_DEBUG("%s: fn[%d](%x, %x, %x, %x, %u)\n",__FUNCTION__,id,what,param1,param2,msg->phy_addr,msg->mem_size);
		ret = fn(what, param1, param2, arg);
		RPC_DEBUG("%s: fn[%d](%x, %x, %x, %x, %u) return %d\n",__FUNCTION__,id,what,param1,param2,msg->phy_addr,msg->mem_size,ret);
		return ret;
	}
	else
	{
		RPC_ERROR("%s: rpc function(id=%d) is NULL!\n",__FUNCTION__,id);
		return ERR_FAILURE;
	}
}

/**
 * @brief Remote Call initialization
 *
 * @retval
 *     SUCCESS,
 *     ERR_FAILURE
 */
RET_CODE rpc_modinit(void)
{
	if (g_rpc_ipc_hdl == NULL)
	{
#ifdef __KERNEL__
		RPC_DEBUG("%s: start\n",__FUNCTION__);
		g_rpc_ipc_hdl = ipc_client_create(AP_RPC_IPC, AV_RPC_IPC, rpc_ipc_msg_handler, NULL);
#else
		g_rpc_ipc_hdl = ipc_start_service(AV_RPC_IPC,
									AP_RPC_IPC,
									rpc_ipc_msg_handler,
									NULL,
									"RPC HSR",
									(void*)RPC_IPC_TASK_STACK,
									RPC_IPC_TASK_STK_SIZE,
									0);

#endif
	}

	if (g_rpc_ipc_hdl == NULL)
	{
#ifdef __KERNEL__
		RPC_ERROR("%s: ipc_client_create failed!\n",__FUNCTION__);
#else
		RPC_ERROR("%s: ipc_start_service failed!\n",__FUNCTION__);
#endif
		return ERR_FAILURE;
	}

#ifdef __KERNEL__
#ifdef DRV_TEST_RPC
	drv_test_rpc();
#endif
#endif

	return SUCCESS;
}

/**
 * @brief Remote Call de-initialization
 */
void rpc_deinit(void)
{
#if 0
	if (g_rpc_ipc_hdl != NULL)
	{
#ifdef __KERNEL__
		RPC_DEBUG("%s: deinit\n",__FUNCTION__);
		ipc_client_destroy(g_rpc_ipc_hdl);
#else
		ipc_stop_service(g_rpc_ipc_hdl);
#endif
		g_rpc_ipc_hdl = NULL;
	}
#else
	RPC_ERROR("%s: function not implemented!\n",__FUNCTION__);
#endif
}

/**
 * @brief AV CPU Remote Call AP CPU functions
 *
 * @param[in] id Remote Call Function ID in AP CPU
 * @param[in] what/param1/param2 message
 * @param[in] ext extended argument
 * @param[in] to timeout in unit of millisecond
 *
 * @retval
 *     SUCCESS,
 *     ERR_FAILURE
 */
RPC_RETVAL rpc_call_to(RPC_FN_ID id,
						unsigned int what,
						unsigned int param1,
						unsigned int param2,
						struct rpc_ext_arg_t *ext,
						unsigned int to)
{
	RET_CODE ret = ERR_FAILURE;
	IPC_RETVAL retval = -1;

	RPC_DEBUG("%s: %u, (%x, %x, %x, %p), %u\n",__FUNCTION__,id,what,param1,param2,ext,to);

#ifdef __KERNEL__
	if (ext != NULL)
	{
		RPC_DEBUG("%s: ext arg(%u, %p)\n",__FUNCTION__,ext->size,ext->arg);
		ret = ipc_client_send_message_sync_app(g_rpc_ipc_hdl,
												IPC_APP_RPC_FLAG|id,
												what, param1, param2,
												(ulong)ext->arg, ext->size,
												to,
												&retval);
	}
	else
	{
		ret = ipc_client_send_message_sync_app(g_rpc_ipc_hdl,
												IPC_APP_RPC_FLAG|id,
												what, param1, param2,
												0, 0,
												to,
												&retval);
	}

	if (ret != SUCCESS)
		return ret;
	else
		return retval;
#else
	return ipc_service_send_message_async_app(g_rpc_ipc_hdl,
											IPC_APP_RPC_FLAG|id,
											what,
											param1,
											param2);
#endif
}

