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

static void *module_mp3(void *arg)
{
	char name_mp3[LXC_IPC_NAME_SIZE] = "module_mp";
	struct lxc_ipc ipc_mp3_0;
	void *in_buf_mp3_0;
	void *out_buf_mp3_0;
	size_t in_size_mp3_0 = 1024;		/* size should more than all param */
	size_t out_size_mp3_0 = 5*1024*1024;		/* size should more than all result */
	int do_what_mp3_0;
	struct listener_mp_fun_f_param *param_f_0;
	struct listener_mp_fun_f_result *result_f_0;
	struct lxc_ipc ipc_mp3_1;
	void *in_buf_mp3_1;
	void *out_buf_mp3_1;
	size_t in_size_mp3_1 = 1024;		/* size should more than all param */
	size_t out_size_mp3_1 = 1024*1024;		/* size should more than all result */
	int do_what_mp3_1;
	struct listener_mp_fun_f_param *param_f_1;
	struct listener_mp_fun_f_result *result_f_1;

	mt_set_pthread_name(name_mp3);

	printf("first\n");
	if (lxc_ipc_open_client(name_mp3, &ipc_mp3_0)) {
		exit(1);
	}

	lxc_ipc_resize(&ipc_mp3_0, &in_buf_mp3_0, &out_buf_mp3_0, &in_size_mp3_0, &out_size_mp3_0, PAGE_SIZE, PAGE_SIZE, NULL);
	/* in_size_mp1 and out_size_mp1 was be update by kernel */
	printf("in_size_mp3_0 = %d\n", in_size_mp3_0);
	printf("out_size_mp3_0 = %d\n", out_size_mp3_0);
	printf("tid_0 = %d\n", ipc_mp3_0.tid);
	printf("buf_id_0 = %d\n", ipc_mp3_0.buf_id);

	param_f_0 = (struct listener_mp_fun_f_param *)in_buf_mp3_0;
	result_f_0 = (struct listener_mp_fun_f_result *)out_buf_mp3_0;

	do_what_mp3_0 = LISTENER_MP_FUN_F;
	strcpy(param_f_0->f, ((char **)arg)[1]);
	lxc_ipc_call(&ipc_mp3_0, do_what_mp3_0);
	printf("result_f_0->r = %d\n", result_f_0->r);
	if (result_f_0->r == 0) {
		printf("result_f_0->content: %s\n", (char *)result_f_0->content);
	}

	sleep(1);

	printf("\n\n\n");
	printf("second\n");
	if (lxc_ipc_open_client(name_mp3, &ipc_mp3_1)) {
		exit(1);
	}

	lxc_ipc_resize(&ipc_mp3_1, &in_buf_mp3_1, &out_buf_mp3_1, &in_size_mp3_1, &out_size_mp3_1, PAGE_SIZE, PAGE_SIZE, NULL);
	/* in_size_mp1 and out_size_mp1 was be update by kernel */
	printf("in_size_mp3_1 = %d\n", in_size_mp3_1);
	printf("out_size_mp3_1 = %d\n", out_size_mp3_1);
	printf("tid_1 = %d\n", ipc_mp3_1.tid);
	printf("buf_id_1 = %d\n", ipc_mp3_1.buf_id);

	param_f_1 = (struct listener_mp_fun_f_param *)in_buf_mp3_1;
	result_f_1 = (struct listener_mp_fun_f_result *)out_buf_mp3_1;

	do_what_mp3_1 = LISTENER_MP_FUN_F;
	strcpy(param_f_1->f, ((char **)arg)[2]);
	lxc_ipc_call(&ipc_mp3_1, do_what_mp3_1);
	printf("result_f_1->r = %d\n", result_f_1->r);
	if (result_f_1->r == 0) {
		printf("result_f_1->content: %s\n", (char *)result_f_1->content);
	}

	sleep(5);

	printf("close first\n");
	in_size_mp3_0 = 1024;
	out_size_mp3_0 = 1024;
	lxc_ipc_resize(&ipc_mp3_0, &in_buf_mp3_0, &out_buf_mp3_0, &in_size_mp3_0, &out_size_mp3_0, PAGE_SIZE, PAGE_SIZE, NULL);

	do_what_mp3_0 = LISTENER_MP_FUN_EXIT;
	lxc_ipc_call(&ipc_mp3_0, do_what_mp3_0);

	lxc_ipc_close(&ipc_mp3_0);

	sleep(1);

	printf("close second\n");
	in_size_mp3_1 = 1024;
	out_size_mp3_1 = 0;
	lxc_ipc_resize(&ipc_mp3_1, &in_buf_mp3_1, &out_buf_mp3_1, &in_size_mp3_1, &out_size_mp3_1, PAGE_SIZE, PAGE_SIZE, NULL);

	do_what_mp3_1 = LISTENER_MP_FUN_EXIT;
	lxc_ipc_call(&ipc_mp3_1, do_what_mp3_1);

	lxc_ipc_close(&ipc_mp3_1);

	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t thread_mp3;

	if (argc < 3) {
		printf("caller_mp3 xxx yyy\n");
		printf("xxx and yyy is a txt file name, for example: caller_mp3 /media/casetest/readme.txt /media/casetest/xxoo.txt\n");
		return -1;
	}

	printf("caller_mp3 running\n");

	pthread_create(&thread_mp3, NULL, module_mp3, (void *)argv);

	pthread_join(thread_mp3, NULL);

	return 0;
}
