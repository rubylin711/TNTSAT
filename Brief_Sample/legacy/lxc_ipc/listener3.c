/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "lxc_ipc.h"
#include "listener3.h"
#include "mt_common.h"

static int listener3_fun_e(struct listener3_fun_e_param *param, struct listener3_fun_e_result *result, size_t size)
{
	int fd;
	ssize_t cnt;

	fd = open(param->f, O_RDWR);
	if (fd == -1) {
		return -1;
	}
	cnt = read(fd, result->content, (size - sizeof(result->r) - 1));	/* last postion should place '\0' in text document */
	result->content[cnt] = '\0';
	close(fd);
	return 0;
}

int main(int argc, char **argv)
{
	char name3[LXC_IPC_NAME_SIZE] = "module3";
	struct lxc_ipc ipc3_listen;
	struct lxc_ipc ipc3;
	void *in_buf3;
	void *out_buf3;
	size_t in_size3;		/* size should more than all param */
	size_t out_size3;		/* size should more than all result */
	int do_what3;
	struct listener3_fun_e_param *param_e;
	struct listener3_fun_e_result *result_e;

	printf("listener3 running\n");

	daemon_init();

	if (lxc_ipc_open_server(name3, &ipc3_listen)) {
		exit(1);
	}

	lxc_ipc_listen(&ipc3_listen, &ipc3);

	while (1) {
		lxc_ipc_accept(&ipc3, &do_what3, &in_buf3, &out_buf3, &in_size3, &out_size3);
		/* in_size3 and out_size3 was be update by kernel */
		printf("in_size3 = %d\n", in_size3);
		printf("out_size3 = %d\n", out_size3);

		param_e = (struct listener3_fun_e_param *)in_buf3;
		result_e = (struct listener3_fun_e_result *)out_buf3;

		switch (do_what3) {
		case LISTENER3_FUN_E:
			result_e->r = listener3_fun_e(param_e, result_e, out_size3);
			lxc_ipc_done(&ipc3);
			break;
		case LISTENER3_FUN_EXIT:
			lxc_ipc_close_accepted(&ipc3);
			lxc_ipc_done(&ipc3);
			goto exit;
		default:
			break;
		}
	}

exit:
	lxc_ipc_close(&ipc3_listen);

	return 0;
}
