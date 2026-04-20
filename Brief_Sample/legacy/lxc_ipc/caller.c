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
#include "listener1.h"
#include "listener2.h"
#include "listener3.h"

static void *module1(void *arg)
{
	char name1[LXC_IPC_NAME_SIZE] = "module1";
	struct lxc_ipc ipc1;
	void *in_buf1;
	void *out_buf1;
	size_t in_size1 = 1024;		/* size should more than all param */
	size_t out_size1 = 1024;	/* size should more than all result */
	int do_what1;
	struct listener1_fun_a_param *param_a;
	struct listener1_fun_a_result *result_a;
	struct listener1_fun_b_param *param_b;
	struct listener1_fun_b_result *result_b;

	mt_set_pthread_name(name1);

	if (lxc_ipc_open_client(name1, &ipc1)) {
		exit(1);
	}

	lxc_ipc_resize(&ipc1, &in_buf1, &out_buf1, &in_size1, &out_size1, PAGE_SIZE, PAGE_SIZE, NULL);
	/* in_size1 and out_size1 was be update by kernel */
	printf("in_size1 = %d\n", in_size1);
	printf("out_size1 = %d\n", out_size1);

	param_a = (struct listener1_fun_a_param *)in_buf1;
	result_a = (struct listener1_fun_a_result *)out_buf1;
	param_b = (struct listener1_fun_b_param *)in_buf1;
	result_b = (struct listener1_fun_b_result *)out_buf1;

	do_what1 = LISTENER1_FUN_A;
	param_a->a = 0x12345678;
	lxc_ipc_call(&ipc1, do_what1);
	printf("result_a->r = 0x%x\n", result_a->r);

	do_what1 = LISTENER1_FUN_B;
	param_b->p = 0x0a0a0a0a;
	lxc_ipc_call(&ipc1, do_what1);
	printf("result_b->r = %d\n", result_b->r);
	printf("result_b->d.x = 0x%x\n", result_b->d.x);
	printf("result_b->d.y = 0x%x\n", result_b->d.y);
	printf("result_b->d.z = 0x%x\n", result_b->d.z);

	do_what1 = LISTENER1_FUN_EXIT;
	lxc_ipc_call(&ipc1, do_what1);

	lxc_ipc_close(&ipc1);

	return NULL;
}

static void *module2(void *arg)
{
	char name2[LXC_IPC_NAME_SIZE] = "module2";
	struct lxc_ipc ipc2;
	void *in_buf2;
	size_t in_size2 = 2048;		/* size should more than all param */
	void *out_buf2;
	size_t out_size2 = 0;
	int do_what2;
	struct listener2_fun_d_param *param_d;

	mt_set_pthread_name(name2);

	if (lxc_ipc_open_client(name2, &ipc2)) {
		exit(1);
	}

	lxc_ipc_resize(&ipc2, &in_buf2, &out_buf2, &in_size2, &out_size2, PAGE_SIZE, PAGE_SIZE, NULL);
	/* in_size2 and out_size2 was be update by kernel */
	printf("in_size2 = %d\n", in_size2);
	printf("out_size2 = %d\n", out_size2);

	param_d = (struct listener2_fun_d_param *)in_buf2;

	do_what2 = LISTENER2_FUN_C;
	lxc_ipc_call(&ipc2, do_what2);

	do_what2 = LISTENER2_FUN_D;
	param_d->m = 0xf1;
	param_d->n = 0xf2;
	lxc_ipc_call(&ipc2, do_what2);
	printf("param_d->m = 0x%x\n", param_d->m);
	printf("param_d->n = 0x%x\n", param_d->n);

	do_what2 = LISTENER2_FUN_EXIT;
	lxc_ipc_call(&ipc2, do_what2);

	lxc_ipc_close(&ipc2);

	return NULL;
}

static void *module3(void *arg)
{
	char name3[LXC_IPC_NAME_SIZE] = "module3";
	struct lxc_ipc ipc3;
	void *in_buf3;
	void *out_buf3;
	size_t in_size3 = 1024;		/* size should more than all param */
	size_t out_size3 = 5120;		/* size should more than all result */
	int do_what3;
	struct listener3_fun_e_param *param_e;
	struct listener3_fun_e_result *result_e;

	mt_set_pthread_name(name3);

	if (lxc_ipc_open_client(name3, &ipc3)) {
		exit(1);
	}

	lxc_ipc_resize(&ipc3, &in_buf3, &out_buf3, &in_size3, &out_size3, PAGE_SIZE, PAGE_SIZE, NULL);
	/* in_size3 and out_size3 was be update by kernel */
	printf("in_size3 = %d\n", in_size3);
	printf("out_size3 = %d\n", out_size3);

	param_e = (struct listener3_fun_e_param *)in_buf3;
	result_e = (struct listener3_fun_e_result *)out_buf3;

	do_what3 = LISTENER3_FUN_E;
	strcpy(param_e->f, (char *)arg);
	lxc_ipc_call(&ipc3, do_what3);
	printf("result_e->r = %d\n", result_e->r);
	if (result_e->r == 0) {
		printf("result_e->content: %s\n", (char *)result_e->content);
	}

	do_what3 = LISTENER3_FUN_EXIT;
	lxc_ipc_call(&ipc3, do_what3);

	lxc_ipc_close(&ipc3);

	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t thread1;
	pthread_t thread2;
	pthread_t thread3;

	if (argc < 2) {
		printf("caller xxx\n");
		printf("xxx is a txt file name, for example: caller /media/casetest/readme.txt\n");
		return -1;
	}

	printf("caller running\n");

	pthread_create(&thread1, NULL, module1, NULL);
	pthread_create(&thread2, NULL, module2, NULL);
	pthread_create(&thread3, NULL, module3, argv[1]);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);

	return 0;
}
