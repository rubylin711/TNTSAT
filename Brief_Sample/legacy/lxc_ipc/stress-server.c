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
#include <semaphore.h>
#include "lxc_ipc.h"
#include "stress.h"
#include "mt_common.h"

#define SUPPORT_CONCURRENCT

static int stress_fun_a(struct stress_fun_a_param *param)
{
	return (param->a + 1);
}

static int stress_fun_b(struct stress_fun_b_param *param, struct stress_detail *d)
{
	d->x = param->p + 2;
	d->y = param->p + 3;
	d->z = param->p + 4;
	return 0xbb;
}

#ifdef SUPPORT_CONCURRENCT

static sem_t sem;
static int cnt = 0;

static void *stress_server(void *arg)
{
	struct lxc_ipc ipc;
	void *in_buf;
	void *out_buf;
	size_t in_size;		/* size should more than all param */
	size_t out_size;	/* size should more than all result */
	int do_what;
	struct stress_fun_a_param *param_a;
	struct stress_fun_a_result *result_a;
	struct stress_fun_b_param *param_b;
	struct stress_fun_b_result *result_b;

	ipc = *((struct lxc_ipc *)arg);
	sem_post(&sem);

	printf("stress server: %d\n", cnt++);
	lxc_ipc_accept(&ipc, &do_what, &in_buf, &out_buf, &in_size, &out_size);
	/* in_size1 and out_size1 was be update by kernel */

	switch (do_what) {
	case STRESS_FUN_A:
		param_a = (struct stress_fun_a_param *)in_buf;
		result_a = (struct stress_fun_a_result *)out_buf;
		result_a->r = stress_fun_a(param_a);
		break;
	case STRESS_FUN_B:
		param_b = (struct stress_fun_b_param *)in_buf;
		result_b = (struct stress_fun_b_result *)out_buf;
		result_b->r = stress_fun_b(param_b, &(result_b->d));
		break;
	default:
		break;
	}
	lxc_ipc_done(&ipc);
	lxc_ipc_close_accepted(&ipc);

	return 0;
}

int main(int argc, char **argv)
{
	char name[LXC_IPC_NAME_SIZE] = STRESS_NAME;
	struct lxc_ipc ipc_listen;
	struct lxc_ipc ipc;
	pthread_t thread;

	printf("stress-server running\n");
	printf("support CONCURRENCT\n");

	sem_init(&sem, 0, 0);

	if (lxc_ipc_open_server(name, &ipc_listen)) {
		exit(1);
	}

	while (1) {
		lxc_ipc_listen(&ipc_listen, &ipc);
		pthread_create(&thread, NULL, stress_server, &ipc);
		pthread_detach(thread);
		sem_wait(&sem);
	}

	lxc_ipc_close(&ipc_listen);

	return 0;
}

#else

int main(int argc, char **argv)
{
	char name[LXC_IPC_NAME_SIZE] = STRESS_NAME;
	struct lxc_ipc ipc_listen;
	struct lxc_ipc ipc;
	void *in_buf;
	void *out_buf;
	size_t in_size;		/* size should more than all param */
	size_t out_size;	/* size should more than all result */
	int do_what;
	struct stress_fun_a_param *param_a;
	struct stress_fun_a_result *result_a;
	struct stress_fun_b_param *param_b;
	struct stress_fun_b_result *result_b;
	int cnt = 0;

	printf("stress_server running\n");
	printf("not support CONCURRENCT\n");

	if (lxc_ipc_open_server(name, &ipc_listen)) {
		exit(1);
	}

	while (1) {
		printf("stress server: %d\n", cnt++);

		lxc_ipc_listen(&ipc_listen, &ipc);
		lxc_ipc_accept(&ipc, &do_what, &in_buf, &out_buf, &in_size, &out_size);
		/* in_size1 and out_size1 was be update by kernel */

		switch (do_what) {
		case STRESS_FUN_A:
			param_a = (struct stress_fun_a_param *)in_buf;
			result_a = (struct stress_fun_a_result *)out_buf;
			result_a->r = stress_fun_a(param_a);
			break;
		case STRESS_FUN_B:
			param_b = (struct stress_fun_b_param *)in_buf;
			result_b = (struct stress_fun_b_result *)out_buf;
			result_b->r = stress_fun_b(param_b, &(result_b->d));
			break;
		default:
			break;
		}
		lxc_ipc_done(&ipc);
		lxc_ipc_close_accepted(&ipc);
	}

	lxc_ipc_close(&ipc_listen);

	return 0;
}

#endif

