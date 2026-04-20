/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : rpc_proxy.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/12/04
 * Description    : MT Remote Call proxy functions in AV CPU.
 * History        :
 * 1.Date         : 2017/12/04
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/kernel.h>
#endif
#include "mt_type.h"
#ifndef __KERNEL__
#include "sys_define.h"
#endif
#include "rpc.h"
#include "rpc_id.h"

#ifdef __KERNEL__
/* declare your AP CPU RPC proxy functions here: */
//TODO...

/* staic rpc proxy functions array */
static rpc_proxy_fn_t g_rpc_proxy_fns[MAX_RPC_PROXY_FN_COUNT] = 
{
	/* add your AP CPU RPC proxy functions according to RPC_FN_ID here: */
	NULL,
};
#else
/* declare your AV CPU RPC proxy functions here: */
//TODO...

/* staic rpc proxy functions */
static rpc_proxy_fn_t g_rpc_proxy_fns[MAX_RPC_PROXY_FN_COUNT] = 
{
	/* add your AV CPU RPC proxy functions according to RPC_FN_ID here: */
	NULL,
};
#endif

//-----------------------------------------------------------------------------//

/**
 * @brief RPC get AV CPU proxy function by id
 *
 * @param[in] id RPC proxy function ID
 *
 * @return RPC proxy function
 */
rpc_proxy_fn_t rpc_get_proxy_fn(RPC_FN_ID id)
{
	if (id >= 0 && id < MAX_RPC_PROXY_FN_COUNT)
	{
		return g_rpc_proxy_fns[id];
	}
	else
	{
		RPC_ERROR("%s: Error, id %d overflow!\n",__FUNCTION__,id);
		return NULL;
	}
}

/**
 * @brief Register RPC proxy function
 *
 * @param[in] id RPC proxy function ID
 * @param[in] fn RPC proxy function
 *
 * @retval
 *     SUCCESS,
 *     ERR_FAILURE
 */
RET_CODE rpc_register(RPC_FN_ID id, rpc_proxy_fn_t fn)
{
	if (id >= 0 && id < MAX_RPC_PROXY_FN_COUNT)
	{
		if (fn != NULL
			&& g_rpc_proxy_fns[id] != NULL
			&& g_rpc_proxy_fns[id] != fn)
		{
			RPC_ERROR("%s: Error, id %d already registered, %p vs. %p\n",__FUNCTION__,id,g_rpc_proxy_fns[id],fn);
			return ERR_FAILURE;
		}

		g_rpc_proxy_fns[id] = fn;
		return SUCCESS;
	}
	else
	{
		RPC_ERROR("%s: Error, id %d overflow!\n",__FUNCTION__,id);
		return ERR_FAILURE;
	}
}

#ifdef __KERNEL__
EXPORT_SYMBOL(rpc_get_proxy_fn);
EXPORT_SYMBOL(rpc_register);
#endif

