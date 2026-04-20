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
#include "listener1.h"
#include "mt_common.h"

static int listener1_fun_a(struct listener1_fun_a_param *param)
{
	return (param->a + 1);
}

static int listener1_fun_b(struct listener1_fun_b_param *param, struct detail *d)
{
	d->x = param->p + 2;
	d->y = param->p + 3;
	d->z = param->p + 4;
	return 0;
}

int main(int argc, char **argv)
{
	char name1[LXC_IPC_NAME_SIZE] = "module1";
	struct lxc_ipc ipc1_listen;
	struct lxc_ipc ipc1;
	void *in_buf1;
	void *out_buf1;
	size_t in_size1;		/* size should more than all param */
	size_t out_size1;	/* size should more than all result */
	int do_what1;
	struct listener1_fun_a_param *param_a;
	struct listener1_fun_a_result *result_a;
	struct listener1_fun_b_param *param_b;
	struct listener1_fun_b_result *result_b;

	printf("listener1 running\n");

	daemon_init();

	if (lxc_ipc_open_server(name1, &ipc1_listen)) {
		exit(1);
	}

	lxc_ipc_listen(&ipc1_listen, &ipc1);

	while (1) {
		lxc_ipc_accept(&ipc1, &do_what1, &in_buf1, &out_buf1, &in_size1, &out_size1);
		/* in_size1 and out_size1 was be update by kernel */
		printf("in_size1 = %d\n", in_size1);
		printf("out_size1 = %d\n", out_size1);

		param_a = (struct listener1_fun_a_param *)in_buf1;
		result_a = (struct listener1_fun_a_result *)out_buf1;
		param_b = (struct listener1_fun_b_param *)in_buf1;
		result_b = (struct listener1_fun_b_result *)out_buf1;

		switch (do_what1) {
		case LISTENER1_FUN_A:
			result_a->r = listener1_fun_a(param_a);
			lxc_ipc_done(&ipc1);
			break;
		case LISTENER1_FUN_B:
			result_b->r = listener1_fun_b(param_b, &(result_b->d));
			lxc_ipc_done(&ipc1);
			break;
		case LISTENER1_FUN_EXIT:
			lxc_ipc_close_accepted(&ipc1);
			lxc_ipc_done(&ipc1);
			goto exit;
		default:
			break;
		}
	}

exit:
	lxc_ipc_close(&ipc1_listen);

	return 0;
}
