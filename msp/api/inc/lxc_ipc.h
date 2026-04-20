/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __LXC_IPC_H__
#define __LXC_IPC_H__

#define LXC_IPC_NAME_SIZE 16

#ifndef __KERNEL__

struct lxc_ipc {
	char name[LXC_IPC_NAME_SIZE];
	pid_t tid;
	int buf_id;
	int fd;
	void *in_buf;
	void *out_buf;
	size_t in_size;
	size_t out_size;
};

/* client */
extern int lxc_ipc_open_client(char *name, struct lxc_ipc *ipc);
extern int lxc_ipc_resize(struct lxc_ipc *ipc, void **in_buf, void **out_buf, size_t *in_size, size_t *out_size, ulong in_align, ulong out_align, off_t *in_phy);
extern int lxc_ipc_call(struct lxc_ipc *ipc, int do_what);
extern int lxc_ipc_call_timeout(struct lxc_ipc *ipc, int do_what, int ms);


/* server */
extern int lxc_ipc_open_server(char *name, struct lxc_ipc *ipc_listen);
extern int lxc_ipc_listen(struct lxc_ipc *ipc_listen, struct lxc_ipc *ipc);
extern int lxc_ipc_accept(struct lxc_ipc *ipc, int *do_what, void **in_buf, void **out_buf, size_t *in_size, size_t *out_size);
extern void lxc_ipc_close_accepted(struct lxc_ipc *ipc);
extern int lxc_ipc_done(struct lxc_ipc *ipc);


/* client and server */
extern void lxc_ipc_close(struct lxc_ipc *ipc);
#endif

#endif /* __LXC_IPC_H__ */
