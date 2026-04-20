/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : hal_ipc_client.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/11/22
 * Description    : HAL(Core) IPC Client.
 * History        :
 * 1.Date         : 2017/11/22
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifdef __KERNEL__
#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>   	/* kmalloc() */
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/align.h>
#include <uapi/linux/sched/types.h>
#endif

#include "mt_type.h"
#ifndef __KERNEL__
#include "sys_define.h"
#endif
#include "mt_cache.h"
#include "ipc.h"
#include "hal_ipc_client.h"
#include "mt_drv_log.h"
#include "mt_debug.h"
#include "mt_cache.h"

#define DEBUG_IPC_CLIENT		0

#if (DEBUG_IPC_CLIENT)
#ifdef __KERNEL__
#define DBG_PRINT				MT_INFO_LOG
#define ERR_PRINT				MT_ERR_LOG
#else
#define DBG_PRINT				AV_PRINTF
#define ERR_PRINT				AV_PRINTF
#endif
#else
#define DBG_PRINT(...)
#ifdef __KERNEL__
#define ERR_PRINT				MT_ERR_LOG
#else
#define ERR_PRINT				AV_PRINTF
#endif
#endif

#define IPC_CLIENT_MAGIC		0x24688642
#define MSG_ID_MASK				0x7FFFFFFF
#define MSG_ID_ACK_FLAG			0x80000000

//see: ipc_symphony.c
#define MAX_PIPE_DEPTH			36

#define MAX_IPC_CLIENT_COUNT	32

//ipc client thread sleep interval in ms
#define DEFAULT_POLL_INTERVAL	1

#define MEM_SIZE_THRESHOLD		4096

/* IPC cache line size aligned memory object */
struct ipc_cache_mem
{
	ulong vir_addr;			//virtual memory address
	ulong vir_addr_aligned;	//cache line size(32) aligned virtual memory address
	phys_addr_t phy_addr_aligned;	//cache line size(32) aligned physical cacheable memory address
	u32 size;				//memory size
};

/* IPC Client */
struct ipc_client
{
	u32 magic_head;

	u32 client_id;
	u32 service_id;

	/* receive message pipe */
	ipc_pipe_t pipe;
	int pipe_depth;

	/* client handler and cookie */
	ipc_msg_handler_t handler;
	void* cookie;

	/* handler thread */
	ulong thread_id;							//received msg handler thread id
	MT_BOOL thread_exit;
	int poll_interval;

	u32 magic_tail;
};

const static int g_ipc_client_table_count = MAX_IPC_CLIENT_COUNT;
static struct ipc_client *g_ipc_client_table[MAX_IPC_CLIENT_COUNT] = {NULL};

static inline MT_BOOL CHECK_HANDLE(struct ipc_client* client)
{
	if (client == NULL
		|| client->magic_head != IPC_CLIENT_MAGIC
		|| client->magic_tail != IPC_CLIENT_MAGIC)
	{
		ERR_PRINT("%s: invalid client handle!\n",__FUNCTION__);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/* IPC Client thread */
static int _ipc_client_thread(void *data)
{
	struct ipc_client *client = (struct ipc_client*)data;
	ipc_msg_handler_t handler = client->handler;
	void *cookie = client->cookie;
	const int poll_interval = client->poll_interval;
	ipc_msg_t msg;
	ipc_msg_t msg_bak;
	RET_CODE ret;
	u32 delay_ms;

	DBG_PRINT("%s: client(%p %x) thread run\n",__FUNCTION__,client,client->client_id);
	DBG_PRINT("%s: poll interval %d(ms)\n",__FUNCTION__,poll_interval);

	while (!client->thread_exit)
	{
		delay_ms = (u32)poll_interval;
		if (check_ap_ready() == SUCCESS)
		{
			ret = ap_recv_from_av(client->client_id, &msg_bak);
			if (ret == SUCCESS)
			{
				//client->state = IPC_CLIENT_STATE_BUSY;
				memcpy(&msg, &msg_bak, sizeof(ipc_msg_t));
				//DBG_PRINT("%s: msg (%x, %x, %x) received\n",__FUNCTION__,
				//			msg.msg_id, msg.param1, msg.param2);
				msg.msg_id &= client->pipe.msg_id_mask;

				if (handler != NULL)
				{
					DBG_PRINT("%s: handler(%p) msg(%x, %x, %x)\n",__FUNCTION__,
							handler,msg.msg_id,msg.param1,msg.param2);
					ret = handler(client, &msg, cookie);
					DBG_PRINT("%s: handler(%p) msg(%x, %x, %x) return %d\n",__FUNCTION__,
							handler,msg.msg_id,msg.param1,msg.param2,ret);
					delay_ms = 0;
				}
				else
				{
					DBG_PRINT("%s: warning handler is NULL!\n",__FUNCTION__);
				}

				if ((msg_bak.msg_id & MSG_ID_ACK_FLAG) != 0)
				{
					//ap_recv_down_ack(&msg_bak);
					ap_recv_down_ack();
				}
				//client->state = IPC_CLIENT_STATE_IDLE;
			}
		}

		if (delay_ms > 0)
			msleep_interruptible(delay_ms);
	}

	DBG_PRINT("%s: client(%p %x) thread exit\n",__FUNCTION__,client,client->client_id);
	return 0;
}

#ifdef __KERNEL__
static void flush_dcache(void* addr, u32 size)
{
	mt_dcache_flush(addr, (size_t)size);
}

static void invalid_dcache(void* addr, u32 size)
{
	mt_dcache_invalid(addr, (size_t)size);
}

//construct ipc memory object from virtual address
//mem [out]
//vir_addr,size [in]
static int ipc_mem_construct(struct ipc_cache_mem *mem, u32 size)
{
	void *kmem_addr;
	unsigned long kmem_addr_aligned;

	kmem_addr = kmalloc(size+CACHE_LINE_SIZE, GFP_KERNEL);
	if (kmem_addr == NULL)
	{
		ERR_PRINT("%s: kmalloc(%u) fail, out of memory!\n",__FUNCTION__,size);
		return ERR_NO_MEM;
	}

	mem->vir_addr = (unsigned long)kmem_addr;

	kmem_addr_aligned = (unsigned long)kmem_addr;
	kmem_addr_aligned = ALIGN(kmem_addr_aligned, CACHE_LINE_SIZE);
	mem->vir_addr_aligned = kmem_addr_aligned;

	memset((void*)(kmem_addr_aligned), 0, size);

	mem->phy_addr_aligned = virt_to_phys((void*)kmem_addr_aligned);
	mem->size = size;

	return SUCCESS;
}

//destruct ipc memory object
static void ipc_mem_destruct(struct ipc_cache_mem *mem)
{
	if (mem->vir_addr != 0)
	{
		kfree((void*)mem->vir_addr);
		memset(mem, 0, sizeof(struct ipc_cache_mem));
	}
}

#endif

//----------------------------------------------------------------------------//

/**
 * @brief Create IPC Client
 *
 * @param[in] client_id IPC Client ID
 * @param[in] service_id IPC Service ID
 *
 * @return IPC Client handle
 */
IPC_CLIENT_HANDLE ipc_client_create(u32 client_id, u32 service_id,
									ipc_msg_handler_t handler, void *cookie)
{
	struct ipc_client *client;
	RET_CODE ret;
	int i;
#ifdef __KERNEL__
	struct task_struct *tsk;
	struct sched_param param;
#endif

	DBG_PRINT("%s: create client_id(0x%08X), service_id(0x%08X)\n",__FUNCTION__,client_id,service_id);

	if (check_ap_ready() != SUCCESS)
	{
		ERR_PRINT("%s: AP CPU is not ready!\n",__FUNCTION__);
		return NULL;
	}

#ifdef __KERNEL__
	client = (struct ipc_client *)kmalloc(sizeof(struct ipc_client), GFP_KERNEL);
#else
	client = (struct ipc_client *)malloc(sizeof(struct ipc_client));
#endif
	if (client == NULL)
	{
		ERR_PRINT("%s: malloc failed!\n",__FUNCTION__);
		return NULL;
	}

	memset(client, 0, sizeof(struct ipc_client));
	client->magic_head = IPC_CLIENT_MAGIC;
	client->magic_tail = IPC_CLIENT_MAGIC;
	client->client_id = client_id;
	client->service_id = service_id;
	client->pipe.msg_id_mask = MSG_ID_MASK;
	client->pipe.p_sid = service_id;
	client->pipe.p_did = client_id;

	//see:
	//  ap_symphony_ipc_pipe_create()
	//  {
	//      p_pipe->pipe_depth = (pipe_depth + 1);
	//  }
	//so the real pipe depth shall be (MAX_PIPE_DEPTH-1).
	client->pipe_depth = MAX_PIPE_DEPTH-1;
	DBG_PRINT("%s: pipe_depth=%d\n",__FUNCTION__,client->pipe_depth);

	ret = ap_ipc_pipe_create(&client->pipe, client->pipe_depth);
	if (ret != SUCCESS)
	{
		ERR_PRINT("%s: create pipe failed!\n",__FUNCTION__);
#ifdef __KERNEL__
		kfree(client);
#else
		free(client);
#endif
		return NULL;
	}

#ifdef __KERNEL__
	if (handler != NULL)
	{
		client->thread_exit = FALSE;
		client->handler = handler;
		client->cookie = cookie;
		client->poll_interval = DEFAULT_POLL_INTERVAL;

		//tsk = kthread_run(_ipc_client_thread, client, "ipc client thread");
        tsk = kthread_create(_ipc_client_thread, client, "ipc_client_thread");
		if (IS_ERR(tsk))
		{
			ERR_PRINT("%s: kthread_run failed!\n",__FUNCTION__);
			kfree(client);
			return NULL;
		}
		else
		{
			client->thread_id = (ulong)tsk;
			param.sched_priority = 99;
			sched_setscheduler(tsk, SCHED_RR, &param);

			wake_up_process(tsk);
		}
	}
#endif

	for (i=0;i<g_ipc_client_table_count;i++)
	{
		if (g_ipc_client_table[i] == NULL)
		{
			g_ipc_client_table[i] = client;
			break;
		}
	}
	if (i >= g_ipc_client_table_count)
	{
		ERR_PRINT("%s: warning, ipc client count overflow!\n",__FUNCTION__);
	}

	DBG_PRINT("%s: create client(%p) success.\n",__FUNCTION__,client);
	return client;
}

/**
 * @brief Destroy IPC Client
 *
 * @param[in] hanle IPC Client handle
 *
 * @retval
 *    SUCCESS
 *    ERR_FAILURE
 *    ERR_NOFEATURE
 */
RET_CODE ipc_client_destroy(IPC_CLIENT_HANDLE hanle)
{
	//TODO...
	ERR_PRINT("%s: function is not implemented!\n",__FUNCTION__);
	return ERR_NOFEATURE;
}

/**
 * @brief Send IPC message to service synchronously
 *
 * @param[in] handle IPC Client handle
 * @param[in] app_id Application ID
 * @param[in] what/param1/param2 message
 * @param[in/out] vir_addr/mem_size kernel or user space virtual memory address and size
 * @param[in] tm_out time out in unit of millisecond
 * @param[out] retval return value of IPC Service remote call
 *
 * @retval
 *    SUCCESS
 *    ERR_FAILURE
 *    ERR_PARAM
 *    ERR_STATUS
 *    ERR_TIMEOUT
 */
RET_CODE ipc_client_send_message_sync_app(IPC_CLIENT_HANDLE handle,
									u32 app_id,
									u32 what, u32 param1, u32 param2,
									ulong vir_addr, u32 mem_size,
									u32 tm_out,
									IPC_RETVAL *retval)
{
	struct ipc_client* client = (struct ipc_client*)handle;
	ipc_msg_t msg;
	RET_CODE ret;
	struct ipc_cache_mem mem_obj;
	u32 mem_size_aligned = 0;	//mem_size aligned 32

	if (!CHECK_HANDLE(client))
	{
		return ERR_PARAM;
	}

	if (check_ap_ready() != SUCCESS)
	{
		ERR_PRINT("%s: AP CPU is not ready!\n",__FUNCTION__);
		return ERR_STATUS;
	}

	memset(&mem_obj, 0, sizeof(struct ipc_cache_mem));

	memset(&msg, 0, sizeof(ipc_msg_t));
	msg.app_id = app_id;
	msg.msg_id = what | MSG_ID_ACK_FLAG;
	msg.param1 = param1;
	msg.param2 = param2;
	msg.time_out_ms = tm_out;

#ifdef __KERNEL__
	//proc mem&size
	if (vir_addr != 0 && mem_size != 0)
	{
		if (mem_size > MEM_SIZE_THRESHOLD)
		{
			ERR_PRINT("%s: Warning! mem size(%u) > threshold(%d)\n",__FUNCTION__,mem_size,MEM_SIZE_THRESHOLD);
		}

		{//copy to av
			mem_size_aligned = mem_size + sizeof(IPC_RETVAL);	/* +retval */
			mem_size_aligned = ALIGN(mem_size_aligned, CACHE_LINE_SIZE);

			ret = ipc_mem_construct(&mem_obj, mem_size_aligned);
			if (ret != SUCCESS)
			{
				return ret;
			}

			memcpy((void*)mem_obj.vir_addr_aligned, (void*)vir_addr, mem_size);
			flush_dcache((void*)mem_obj.vir_addr_aligned, mem_size_aligned);

			msg.phy_addr = mem_obj.phy_addr_aligned;
			msg.mem_size = mem_size;	//exclude retval size, and not aligned
			msg.retval_phy_addr = mem_obj.phy_addr_aligned + mem_size_aligned - sizeof(IPC_RETVAL);
			DBG_PRINT("%s: vir_addr %x, mem_size %u\n",__FUNCTION__,vir_addr,mem_size);
			DBG_PRINT("%s: kmem_addr (%x, %x) %x\n",__FUNCTION__,mem_obj.vir_addr_aligned,mem_obj.phy_addr_aligned,mem_size_aligned);
		}
	}
	else
	{
		if (retval != NULL)
		{
			mem_size_aligned = sizeof(IPC_RETVAL);	/* retval only */
			mem_size_aligned = ALIGN(mem_size_aligned, CACHE_LINE_SIZE);

			ret = ipc_mem_construct(&mem_obj, mem_size_aligned);
			if (ret != SUCCESS)
			{
				return ret;
			}

			flush_dcache((void*)mem_obj.vir_addr_aligned, mem_size_aligned);

			msg.retval_phy_addr = mem_obj.phy_addr_aligned;
		}
		else
		{
			//no retval
			msg.retval_phy_addr = 0;
		}
	}
#else
	msg.phy_addr = vir_addr;
	msg.mem_size = mem_size;
	msg.retval_phy_addr = 0;	//can not support retval in this case

	if (retval != NULL)
		*retval = 0;
#endif

	DBG_PRINT("%s: %p send msg (0x%08X, 0x%X, 0x%X, 0x%X, 0x%X) %u\n",__FUNCTION__,
				handle,what,param1,param2,vir_addr,mem_size,tm_out);
	ret = ap_send_to_av(client->client_id, &msg, 1);
	DBG_PRINT("%s: %p send msg (0x%08X, 0x%X, 0x%X, 0x%X, 0x%X) %u return %d\n",__FUNCTION__,
				handle,what,param1,param2,vir_addr,mem_size,tm_out,ret);

	if (ret == SUCCESS)
	{
#ifdef __KERNEL__
		if (mem_obj.vir_addr != 0)
		{
			//copy back
			invalid_dcache((void*)mem_obj.vir_addr_aligned, mem_size_aligned);
			if (vir_addr != 0 && mem_size != 0)
			{
				memcpy((void*)vir_addr, (void*)mem_obj.vir_addr_aligned, mem_size);
			}
			if (retval != NULL)
			{
				memcpy(retval, (void*)(mem_obj.vir_addr_aligned+mem_size_aligned-sizeof(IPC_RETVAL)), sizeof(IPC_RETVAL));
			}
		}
#endif
	}
	else
	{
		ERR_PRINT("%s: ap_send_to_av(0x%X, 0x%X, 0x%X) failed, return %d\n",__FUNCTION__,what,param1,param2,ret);
	}

#ifdef __KERNEL__
	if (mem_obj.vir_addr != 0)
		ipc_mem_destruct(&mem_obj);
#endif

	return ret;
}

/**
 * @brief Send IPC message to service asynchronously
 *
 * @param[in] handle IPC Client handle
 * @param[in] what/param1/param2 message
 *
 * @retval
 *    SUCCESS
 *    ERR_FAILURE
 *    ERR_NOFEATURE
 */
RET_CODE ipc_client_send_message_async(IPC_CLIENT_HANDLE handle, u32 what, u32 param1, u32 param2)
{
	//TODO...
	ERR_PRINT("%s: function is not implemented!\n",__FUNCTION__);
	return ERR_NOFEATURE;
}

/**
 * @brief Receive IPC message from service
 *
 * @param[in] handle IPC Client handle
 * @param[out] msg message
 *
 * @retval
 *    SUCCESS
 *    ERR_FAILURE
 *    ERR_PARAM
 *    ERR_STATUS
 */
RET_CODE ipc_client_receive_message(IPC_CLIENT_HANDLE handle, ipc_msg_t *msg)
{
	struct ipc_client* client = (struct ipc_client*)handle;
	RET_CODE ret;

	if (!CHECK_HANDLE(client) || msg == NULL)
	{
		return ERR_PARAM;
	}

	if (check_ap_ready() != SUCCESS)
	{
		ERR_PRINT("%s: AP CPU is not ready!\n",__FUNCTION__);
		return ERR_STATUS;
	}

	ret = ap_recv_from_av(client->client_id, msg);
	if (ret == SUCCESS)
	{
		msg->msg_id &= client->pipe.msg_id_mask;
		DBG_PRINT("%s: %p msg (0x%08X, 0x%X, 0x%X) received\n",__FUNCTION__,
					handle,msg->msg_id,msg->param1,msg->param2);
	}

	return ret;
}

#ifdef __KERNEL__
EXPORT_SYMBOL(ipc_client_create);
EXPORT_SYMBOL(ipc_client_destroy);
EXPORT_SYMBOL(ipc_client_send_message_sync_app);
EXPORT_SYMBOL(ipc_client_send_message_async);
EXPORT_SYMBOL(ipc_client_receive_message);
#endif

