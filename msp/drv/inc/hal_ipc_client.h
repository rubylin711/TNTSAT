/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : hal_ipc_client.h
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
#ifndef _HAL_IPC_CLIENT_H_
#define _HAL_IPC_CLIENT_H_

//#include "mt_type.h"
#include "ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IPC Client Handle */
typedef void* IPC_CLIENT_HANDLE;

/* IPC return value type */
typedef int IPC_RETVAL;

/* IPC Client message handler */
typedef IPC_RETVAL (*ipc_msg_handler_t)(IPC_CLIENT_HANDLE handle, ipc_msg_t *msg, void* cookie);

/**
 * @brief Create IPC Client
 *
 * @param[in] client_id IPC Client ID
 * @param[in] service_id IPC Service ID
 *
 * @return IPC Client handle
 */
IPC_CLIENT_HANDLE ipc_client_create(u32 client_id, u32 service_id,
									ipc_msg_handler_t handler, void *cookie);

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
RET_CODE ipc_client_destroy(IPC_CLIENT_HANDLE hanle);

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
									IPC_RETVAL *retval);

#define ipc_client_send_message_sync(h,w,p1,p2,v,m,t,r) ipc_client_send_message_sync_app(h,0,w,p1,p2,v,m,t,r)

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
RET_CODE ipc_client_send_message_async(IPC_CLIENT_HANDLE handle, u32 what, u32 param1, u32 param2);

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
RET_CODE ipc_client_receive_message(IPC_CLIENT_HANDLE handle, ipc_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif	//_HAL_IPC_CLIENT_H_

