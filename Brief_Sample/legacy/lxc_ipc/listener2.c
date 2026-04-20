/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "lxc_ipc.h"
#include "listener2.h"
#include "mt_common.h"

static void listener2_fun_c(void)
{

}

static void listener2_fun_d(struct listener2_fun_d_param *param)
{
	param->m += 0xa;
	param->n += 0xa;
}

int main(int argc, char **argv)
{
	char name2[LXC_IPC_NAME_SIZE] = "module2";
	struct lxc_ipc ipc2_listen;
	struct lxc_ipc ipc2;
	void *in_buf2;
	void *out_buf2;
	size_t in_size2;		/* size should more than all param */
	size_t out_size2;
	int do_what2;
	struct listener2_fun_d_param *param_d;

	printf("listener2 running\n");

	daemon_init();

	if (lxc_ipc_open_server(name2, &ipc2_listen)) {
		exit(1);
	}

	lxc_ipc_listen(&ipc2_listen, &ipc2);

	while (1) {
		lxc_ipc_accept(&ipc2, &do_what2, &in_buf2, &out_buf2, &in_size2, &out_size2);
		/* in_size2 and out_size2 was be update by kernel */
		printf("in_size2 = %d\n", in_size2);
		printf("out_size2 = %d\n", out_size2);

		param_d = (struct listener2_fun_d_param *)in_buf2;

		switch (do_what2) {
		case LISTENER2_FUN_C:
			listener2_fun_c();
			lxc_ipc_done(&ipc2);
			break;
		case LISTENER2_FUN_D:
			listener2_fun_d(param_d);
			lxc_ipc_done(&ipc2);
			break;
		case LISTENER2_FUN_EXIT:
			lxc_ipc_close_accepted(&ipc2);
			lxc_ipc_done(&ipc2);
			goto exit;
		default:
			break;
		}
	}

exit:
	lxc_ipc_close(&ipc2_listen);

	return 0;
}
