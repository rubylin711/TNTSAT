/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include "lxc_ipc.h"
#include "lxc_ipc_priv.h"
#include "mt_common.h"
#include "mt_mpi_mem.h"

static int lxc_ipc_open(char *name, struct lxc_ipc *ipc, pid_t is_client)
{
	struct lxc_ipc_user user;

	if (name == NULL) {
		return -1;
	}

	memset(ipc, 0, sizeof(*ipc));
	memset(&user, 0, sizeof(user));

	ipc->fd = open("/dev/"LXC_IPC_NAME, O_RDWR|O_CLOEXEC);
	if (ipc->fd == -1) {
		goto fail_open;
	}

	strncpy(user.name, name, LXC_IPC_NAME_SIZE);
	strncpy(ipc->name, name, LXC_IPC_NAME_SIZE);

	user.tid = is_client;

	if (ioctl(ipc->fd, LXC_IPC_ATTACH, &user)) {
		goto fail_ioctl;
	}

	/* tid update by kernel */
	ipc->tid = user.tid;
	ipc->buf_id = user.buf_id;

	return 0;

fail_ioctl:
	close(ipc->fd);
fail_open:
	return -1;
}

int lxc_ipc_open_client(char *name, struct lxc_ipc *ipc)
{
	return lxc_ipc_open(name, ipc, 1);
}

int lxc_ipc_open_server(char *name, struct lxc_ipc *ipc_listen)
{
	return lxc_ipc_open(name, ipc_listen, 0);
}

void lxc_ipc_close(struct lxc_ipc *ipc)
{
	struct lxc_ipc_user user;

	memset(&user, 0, sizeof(user));

	strncpy(user.name, ipc->name, LXC_IPC_NAME_SIZE);
	user.tid = ipc->tid;
	user.buf_id = ipc->buf_id;

	if (user.tid != 0 && user.buf_id != -1) {
		if (ipc->out_buf != NULL && ipc->out_size != 0) {
			mt_munmap(ipc->out_buf);
		}
		if (ipc->in_buf != NULL && ipc->in_size != 0) {
			mt_munmap(ipc->in_buf);
		}
	}

	ioctl(ipc->fd, LXC_IPC_DETACH, &user);

	close(ipc->fd);
}

int lxc_ipc_resize(struct lxc_ipc *ipc, void **in_buf, void **out_buf, size_t *in_size, size_t *out_size, ulong in_align, ulong out_align, off_t *in_phy)
{
	struct lxc_ipc_user user;

	if (in_buf == NULL || out_buf == NULL || in_size == NULL || out_size == NULL) {
		return -1;
	}

	memset(&user, 0, sizeof(user));

	strncpy(user.name, ipc->name, LXC_IPC_NAME_SIZE);
	user.tid = ipc->tid;
	user.buf_id = ipc->buf_id;

	if (ipc->out_buf != NULL && ipc->out_size != 0 && *out_size != ipc->out_size) {
		mt_munmap(ipc->out_buf);
	}
	if (ipc->in_buf != NULL && ipc->in_size != 0 && *in_size != ipc->in_size) {
		mt_munmap(ipc->in_buf);
	}

	user.in_size = (ulong)*in_size;
	user.out_size = (ulong)*out_size;
	user.in_align = in_align;
	user.out_align = out_align;

	if (ioctl(ipc->fd, LXC_IPC_RESIZE, &user)) {
		goto fail_ioctl;
	}

	if (user.in_size == 0 || user.in_phys_addr == 0) {
		ipc->in_buf = NULL;
		ipc->in_size = 0;
	} else if (ipc->in_size != user.in_size || ipc->in_size != *in_size) {
		ipc->in_buf = mt_mmap_cache((mt_u32)user.in_phys_addr, (mt_u32)user.in_size);
		if (ipc->in_buf == NULL) {
			goto fail_in_mmap;
		}
		ipc->in_size = user.in_size;
	}

	if (user.out_size == 0 || user.out_phys_addr == 0) {
		ipc->out_buf = NULL;
		ipc->out_size = 0;
	} else if (ipc->out_size != user.out_size || ipc->out_size != *out_size) {
		ipc->out_buf = mt_mmap_cache((mt_u32)user.out_phys_addr, (mt_u32)user.out_size);
		if (ipc->out_buf == NULL) {
			goto fail_out_mmap;
		}
		ipc->out_size = user.out_size;
	}

	*in_size = ipc->in_size;
	*in_buf = ipc->in_buf;
	*out_size = ipc->out_size;
	*out_buf = ipc->out_buf;
	if (in_phy) {
		*in_phy=user.in_phys_addr;
	}
	return 0;

fail_out_mmap:
	if (ipc->in_buf != NULL && ipc->in_size != 0) {
		mt_munmap(ipc->in_buf);
	}
	ipc->out_buf = NULL;
	ipc->out_size = 0;
	*out_buf = NULL;
	*out_size = 0;
fail_in_mmap:
	ipc->in_buf = NULL;
	ipc->in_size = 0;
	*in_buf = NULL;
	*in_size = 0;
	if (in_phy) {
		*in_phy=0;
	}
fail_ioctl:
	return -1;
}

int lxc_ipc_call_timeout(struct lxc_ipc *ipc, int do_what, int ms)
{
	struct lxc_ipc_user user;
    int ret;

	memset(&user, 0, sizeof(user));

	strncpy(user.name, ipc->name, LXC_IPC_NAME_SIZE);
	user.tid = ipc->tid;
	user.buf_id = ipc->buf_id;
	user.do_what = do_what;
	user.timeout = ms;

	do {
		ret = ioctl(ipc->fd, LXC_IPC_CALL, &user);
	} while (ret < 0 && errno == EINTR);

    return ret;
}

int lxc_ipc_call(struct lxc_ipc *ipc, int do_what)
{
	return lxc_ipc_call_timeout(ipc, do_what, 0);
}

int lxc_ipc_listen(struct lxc_ipc *ipc_listen, struct lxc_ipc *ipc)
{
	struct lxc_ipc_user user;
    int ret;

	memset(&user, 0, sizeof(user));

	*ipc = *ipc_listen;

	strncpy(user.name, ipc_listen->name, LXC_IPC_NAME_SIZE);
	user.tid = ipc_listen->tid;	/* tid now is 0 */

	do {
		ret = ioctl(ipc->fd, LXC_IPC_LISTEN, &user);
	} while (ret < 0 && errno == EINTR);

    if(!ret) {
        ipc->tid = user.tid;
        ipc->buf_id = user.buf_id;
    }

	return ret;

}

int lxc_ipc_accept(struct lxc_ipc *ipc, int *do_what, void **in_buf, void **out_buf, size_t *in_size, size_t *out_size)
{
	struct lxc_ipc_user user;

	if (in_buf == NULL || out_buf == NULL || in_size == NULL || out_size == NULL || do_what == NULL) {
		return -1;
	}

	memset(&user, 0, sizeof(user));

	strncpy(user.name, ipc->name, LXC_IPC_NAME_SIZE);
	user.tid = ipc->tid;
	user.buf_id = ipc->buf_id;

    while(ioctl(ipc->fd, LXC_IPC_ACCEPT, &user)) {
        if(errno == EINTR) {
            continue;
        }
        goto fail_ioctl;
    }

	if (ipc->out_buf != NULL && ipc->out_size != 0 && user.out_size != ipc->out_size) {
		mt_munmap(ipc->out_buf);
	}
	if (ipc->in_buf != NULL && ipc->in_size != 0 && user.in_size != ipc->in_size) {
		mt_munmap(ipc->in_buf);
	}

	if (user.in_size == 0 || user.in_phys_addr == 0) {
		ipc->in_buf = NULL;
		ipc->in_size = 0;
	} else if (ipc->in_size != user.in_size) {
		ipc->in_buf = mt_mmap_cache((mt_u32)user.in_phys_addr, (mt_u32)user.in_size);
		if (ipc->in_buf == NULL) {
			goto fail_in_mmap;
		}
		ipc->in_size = user.in_size;
	}

	if (user.out_size == 0 || user.out_phys_addr == 0) {
		ipc->out_buf = NULL;
		ipc->out_size = 0;
	} else if (ipc->out_size != user.out_size) {
		ipc->out_buf = mt_mmap_cache((mt_u32)user.out_phys_addr, (mt_u32)user.out_size);
		if (ipc->out_buf == NULL) {
			goto fail_out_mmap;
		}
		ipc->out_size = user.out_size;
	}

	*in_size = ipc->in_size;
	*in_buf = ipc->in_buf;
	*out_size = ipc->out_size;
	*out_buf = ipc->out_buf;

	*do_what = user.do_what;

	return 0;

fail_out_mmap:
	if (ipc->in_buf != NULL && ipc->in_size != 0) {
		mt_munmap(ipc->in_buf);
	}
	ipc->out_buf = NULL;
	ipc->out_size = 0;
	*out_buf = NULL;
	*out_size = 0;
fail_in_mmap:
	ipc->in_buf = NULL;
	ipc->in_size = 0;
	*in_buf = NULL;
	*in_size = 0;
fail_ioctl:
	return -1;
}

void lxc_ipc_close_accepted(struct lxc_ipc *ipc)
{
	struct lxc_ipc_user user;

	memset(&user, 0, sizeof(user));

	strncpy(user.name, ipc->name, LXC_IPC_NAME_SIZE);
	user.tid = ipc->tid;
	user.buf_id = ipc->buf_id;

	if (ipc->out_buf != NULL && ipc->out_size != 0) {
		mt_munmap(ipc->out_buf);
	}
	if (ipc->in_buf != NULL && ipc->in_size != 0) {
		mt_munmap(ipc->in_buf);
	}

	ioctl(ipc->fd, LXC_IPC_DETACH, &user);
}

int lxc_ipc_done(struct lxc_ipc *ipc)
{
	struct lxc_ipc_user user;

	memset(&user, 0, sizeof(user));

	strncpy(user.name, ipc->name, LXC_IPC_NAME_SIZE);
	user.tid = ipc->tid;
	user.buf_id = ipc->buf_id;

	return ioctl(ipc->fd, LXC_IPC_DONE, &user);
}

