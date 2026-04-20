/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HDMI20_PRIV_H__
#define __HDMI20_PRIV_H__

#include "mt_hdmi20.h"
#define HDMI20_NAME "hdmi20"

struct hdmi20_user {
	char name[HDMI20_NAME_SIZE];
	pid_t tid;
	int buf_id;
	int do_what;
	int timeout;
	size_t in_size;			/* bytes */
	size_t out_size;		/* bytes */
	off_t in_phys_addr;		/* bytes */
	off_t out_phys_addr;	/* bytes */
};

#define HDMI20_ATTACH (_IOWR('x', 1, struct hdmi20_user))
#define HDMI20_DETACH (_IOWR('x', 2, struct hdmi20_user))
#define HDMI20_RESIZE (_IOWR('x', 3, struct hdmi20_user))
#define HDMI20_CALL (_IOWR('x', 4, struct hdmi20_user))
#define HDMI20_LISTEN (_IOWR('x', 5, struct hdmi20_user))
#define HDMI20_ACCEPT (_IOWR('x', 6, struct hdmi20_user))
#define HDMI20_DONE (_IOWR('x', 7, struct hdmi20_user))

#endif /* __HDMI20_PRIV_H__ */
