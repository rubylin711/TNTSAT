/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "mt_common.h"
#include "lxc_ipc.h"
#include "listener_mp.h"

static void *module_mp2(void *arg)
{
	char name_mp2[LXC_IPC_NAME_SIZE] = "module_mp";
	struct lxc_ipc ipc_mp2;
	void *in_buf_mp2;
	void *out_buf_mp2;
	size_t in_size_mp2 = 100;		/* size should more than all param */
	size_t out_size_mp2 = 2000;		/* size should more than all result */
	int do_what_mp2;
	struct listener_mp_fun_f_param *param_f;
	struct listener_mp_fun_f_result *result_f;

	mt_set_pthread_name(name_mp2);

	if (lxc_ipc_open_client(name_mp2, &ipc_mp2)) {
		exit(1);
	}

	lxc_ipc_resize(&ipc_mp2, &in_buf_mp2, &out_buf_mp2, &in_size_mp2, &out_size_mp2, PAGE_SIZE, PAGE_SIZE, NULL);
	/* in_size_mp2 and out_size_mp2 was be update by kernel */
	printf("in_size_mp2 = %d\n", in_size_mp2);
	printf("out_size_mp2 = %d\n", out_size_mp2);
	printf("tid = %d\n", ipc_mp2.tid);
	printf("buf_id = %d\n", ipc_mp2.buf_id);

	param_f = (struct listener_mp_fun_f_param *)in_buf_mp2;
	result_f = (struct listener_mp_fun_f_result *)out_buf_mp2;

	do_what_mp2 = LISTENER_MP_FUN_F;
	strcpy(param_f->f, (char *)arg);
	lxc_ipc_call(&ipc_mp2, do_what_mp2);
	printf("result_f->r = %d\n", result_f->r);
	if (result_f->r == 0) {
		printf("result_f->content: %s\n", (char *)result_f->content);
	}

	sleep(5);

	in_size_mp2 = 0;
	out_size_mp2 = 0;
	lxc_ipc_resize(&ipc_mp2, &in_buf_mp2, &out_buf_mp2, &in_size_mp2, &out_size_mp2, PAGE_SIZE, PAGE_SIZE, NULL);

	do_what_mp2 = LISTENER_MP_FUN_EXIT;
	lxc_ipc_call(&ipc_mp2, do_what_mp2);

	lxc_ipc_close(&ipc_mp2);

	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t thread_mp2;

	if (argc < 2) {
		printf("caller_mp2 xxx\n");
		printf("xxx is a txt file name, for example: caller_mp2 /media/casetest/xxoo.txt\n");
		return -1;
	}

	printf("caller_mp2 running\n");

	pthread_create(&thread_mp2, NULL, module_mp2, argv[1]);

	pthread_join(thread_mp2, NULL);

	return 0;
}
