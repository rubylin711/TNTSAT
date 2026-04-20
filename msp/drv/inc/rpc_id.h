/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : rpc_id.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/12/04
 * Description    : MT Remote Call proxy function ID definition.
 * History        :
 * 1.Date         : 2017/12/04
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef _MT_RPC_ID_H_
#define _MT_RPC_ID_H_

#ifdef __cplusplus
extern "C" {
#endif

/* RPC proxy function count */
#define MAX_RPC_PROXY_FN_COUNT				256

/* AV CPU RPC proxy function id */
enum {
	RPC_AV_FN_ID_EXAMPLE = 0,

	/* add your AV CPU RPC proxy function id here: */
	RPC_AV_FN_ID_VDEC_FW_INIT,
	RPC_AV_FN_ID_VDEC_FW_IO_CTRL,
	RPC_AV_FN_ID_VDEC_FW_EXIT,

	RPC_AV_FN_ID_AMPSHM_SET_PARAMETER,

	RPC_AV_FN_ID_PVR_FW_INIT,
	RPC_AV_FN_ID_PVR_FW_IO_CTRL,
	RPC_AV_FN_ID_PVR_FW_EXIT,
	//TODO...

	RPC_AV_FN_ID_BUTT = MAX_RPC_PROXY_FN_COUNT,
};

/* AP CPU RPC proxy function id */
enum {
	RPC_AP_FN_ID_EXAMPLE = 0,

	/* add your AP CPU RPC proxy function id here: */
	RPC_AP_FN_ID_VDEC_CALLBACK,
	RPC_AP_FN_ID_PVR_CALLBACK,
	//TODO...

	RPC_AP_FN_ID_BUTT = MAX_RPC_PROXY_FN_COUNT,
};

#ifdef __cplusplus
}
#endif

#endif	//_MT_RPC_ID_H_

