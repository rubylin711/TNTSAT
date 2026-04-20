/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : rpc.h
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
#ifndef _MT_RPC_H_
#define _MT_RPC_H_

#define DEBUG_RPC						0

#ifdef __KERNEL__
#define RPC_ERROR						printk
#else
#define RPC_ERROR						AV_PRINTF
#endif

#if (DEBUG_RPC)
#ifdef __KERNEL__
#define RPC_DEBUG						printk
#else
#define RPC_DEBUG						AV_PRINTF
#endif
#else
#define RPC_DEBUG(...)
#endif

#ifdef __cplusplus
extern "C" {
#endif

//20200608 Begin
//Bug: avcpu fw error data concealing cause audio ipc msg timeout
//#define RPC_CALL_TIMEOUT_DEFAULT		1000
#define RPC_CALL_TIMEOUT_DEFAULT		30000
//20200608 End
#define RPC_CALL_TIMEOUT_FOREVER		0xFFFFFFFF

/* Remote Call proxy function return value type */
typedef int RPC_RETVAL;

/* Remote Call proxy function ID */
typedef unsigned short RPC_FN_ID;

/* Remote Call proxy function RPC extended argument structure */
struct rpc_ext_arg_t
{
	unsigned int size;	/* extended argument size */
	void *arg;			/* extended argument address */
};

/*
 * AV/AP CPU Remote Call proxy function type
 */
typedef RPC_RETVAL (*rpc_proxy_fn_t)(unsigned int what, unsigned int param1, unsigned int param2, struct rpc_ext_arg_t *ext);

/**
 * @brief Remote Call initialization
 *
 * @retval
 *     SUCCESS,
 *     ERR_FAILURE
 */
RET_CODE rpc_init(void);

/**
 * @brief Remote Call de-initialization
 */
void rpc_deinit(void);

/**
 * @brief AV/AP CPU Remote Call AP/AV CPU function
 *
 * @param[in] id Remote Call Function ID in proxy CPU
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
						unsigned int to);

#define rpc_call(id,w,p1,p2,e)		rpc_call_to(id,w,p1,p2,e,RPC_CALL_TIMEOUT_FOREVER)

/**
 * @brief RPC get proxy function by id
 *
 * @param[in] id RPC proxy function ID
 *
 * @return RPC proxy function
 */
rpc_proxy_fn_t rpc_get_proxy_fn(RPC_FN_ID id);

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
RET_CODE rpc_register(RPC_FN_ID id, rpc_proxy_fn_t fn);

#ifdef __cplusplus
}
#endif

#endif	//_MT_RPC_H_

