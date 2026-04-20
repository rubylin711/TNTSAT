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
#include "stress.h"

static void *stress1(void *arg)
{
	char name1[LXC_IPC_NAME_SIZE] = STRESS_NAME;
	struct lxc_ipc ipc1;
	void *in_buf1;
	void *out_buf1;
	size_t in_size1 = 1024;		/* size should more than all param */
	size_t out_size1 = 1024;	/* size should more than all result */
	int do_what1;
	struct stress_fun_a_param *param_a;
	struct stress_fun_a_result *result_a;
	int cnt = 0;

	mt_set_pthread_name(name1);

	while (cnt++ > -1) {
		printf("stress1 client: %d\n", cnt);
		if (lxc_ipc_open_client(name1, &ipc1)) {
			exit(1);
		}

		lxc_ipc_resize(&ipc1, &in_buf1, &out_buf1, &in_size1, &out_size1, PAGE_SIZE, PAGE_SIZE, NULL);
		/* in_size1 and out_size1 was be update by kernel */

		param_a = (struct stress_fun_a_param *)in_buf1;
		result_a = (struct stress_fun_a_result *)out_buf1;

		do_what1 = STRESS_FUN_A;
		param_a->a = 0x12345678;
		lxc_ipc_call(&ipc1, do_what1);
		printf("result_a->r = 0x%x\n", result_a->r);

		lxc_ipc_close(&ipc1);
	}

	return NULL;
}

static void *stress2(void *arg)
{
	char name2[LXC_IPC_NAME_SIZE] = STRESS_NAME;
	struct lxc_ipc ipc2;
	void *in_buf2;
	size_t in_size2 = 2048;		/* size should more than all param */
	void *out_buf2;
	size_t out_size2 = 1024;
	int do_what2;
	struct stress_fun_b_param *param_b;
	struct stress_fun_b_result *result_b;
	int cnt = 0;

	mt_set_pthread_name(name2);

	while (cnt++ > -1) {
		printf("stress2 client: %d\n", cnt);
		if (lxc_ipc_open_client(name2, &ipc2)) {
			exit(1);
		}

		lxc_ipc_resize(&ipc2, &in_buf2, &out_buf2, &in_size2, &out_size2, PAGE_SIZE, PAGE_SIZE, NULL);
		/* in_size2 and out_size2 was be update by kernel */

		param_b = (struct stress_fun_b_param *)in_buf2;
		result_b = (struct stress_fun_b_result *)out_buf2;

		do_what2 = STRESS_FUN_B;
		param_b->p = 0x0a0a0a0a;
		lxc_ipc_call(&ipc2, do_what2);
		printf("result_b->r = 0x%x\n", result_b->r);
		printf("result_b->d.x = 0x%x\n", result_b->d.x);
		printf("result_b->d.y = 0x%x\n", result_b->d.y);
		printf("result_b->d.z = 0x%x\n", result_b->d.z);

		lxc_ipc_close(&ipc2);
	}

	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t thread1;
	pthread_t thread2;

	printf("stress-client running\n");

	pthread_create(&thread1, NULL, stress1, NULL);
	pthread_create(&thread2, NULL, stress2, NULL);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	return 0;
}

