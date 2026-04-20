/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __LXC_IPC_PRIV_H__
#define __LXC_IPC_PRIV_H__

#include "mt_common.h"

#define LXC_IPC_NAME "lxc_ipc"

struct lxc_ipc_user {
	char name[LXC_IPC_NAME_SIZE];
	pid_t tid;
	int buf_id;
	int do_what;
	int timeout;
	ulong in_size;			        /* bytes */
	ulong out_size;		          /* bytes */
	ulong in_align;			        /* bytes */
	ulong out_align;		        /* bytes */
	phys_addr_t in_phys_addr;		      /* bytes */
	phys_addr_t out_phys_addr;	      /* bytes */
};

#define LXC_IPC_ATTACH (_IOWR('x', 1, struct lxc_ipc_user))
#define LXC_IPC_DETACH (_IOWR('x', 2, struct lxc_ipc_user))
#define LXC_IPC_RESIZE (_IOWR('x', 3, struct lxc_ipc_user))
#define LXC_IPC_CALL (_IOWR('x', 4, struct lxc_ipc_user))
#define LXC_IPC_LISTEN (_IOWR('x', 5, struct lxc_ipc_user))
#define LXC_IPC_ACCEPT (_IOWR('x', 6, struct lxc_ipc_user))
#define LXC_IPC_DONE (_IOWR('x', 7, struct lxc_ipc_user))

#endif /* __LXC_IPC_PRIV_H__ */
