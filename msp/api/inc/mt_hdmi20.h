/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_HDMI20_H__
#define __MT_HDMI20_H__

#define HDMI20_NAME_SIZE 32

#ifndef __KERNEL__

struct hdmi20 {
	char name[HDMI20_NAME_SIZE];
	pid_t tid;
	int buf_id;
	int fd;
	void *in_buf;
	void *out_buf;
	size_t in_size;
	size_t out_size;
};

/* client */
extern int hdmi20_open_client(char *name, struct hdmi20 *ipc);
extern int hdmi20_resize(struct hdmi20 *ipc, void **in_buf, void **out_buf, size_t *in_size, size_t *out_size);
extern int hdmi20_call(struct hdmi20 *ipc, int do_what);
extern int hdmi20_call_timeout(struct hdmi20 *ipc, int do_what, int ms);


/* server */
extern int hdmi20_open_server(char *name, struct hdmi20 *ipc_listen);
extern int hdmi20_listen(struct hdmi20 *ipc_listen, struct hdmi20 *ipc);
extern int hdmi20_accept(struct hdmi20 *ipc, int *do_what, void **in_buf, void **out_buf, size_t *in_size, size_t *out_size);
extern void hdmi20_close_accepted(struct hdmi20 *ipc);
extern int hdmi20_done(struct hdmi20 *ipc);


/* client and server */
extern void hdmi20_close(struct hdmi20 *ipc);
#endif

#endif /* __MT_HDMI20_H__ */
