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
#include <pthread.h>
#include <semaphore.h>
#include "mt_common.h"
#include "lxc_ipc.h"
#include "listener_mp.h"

static sem_t sem_mp;

static int listener_mp_fun_f(struct listener_mp_fun_f_param *param, struct listener_mp_fun_f_result *result, size_t size)
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

static void *module_mp(void *arg)
{
	struct lxc_ipc ipc_mp;
	void *in_buf_mp;
	void *out_buf_mp;
	size_t in_size_mp;		/* size should more than all param */
	size_t out_size_mp;		/* size should more than all result */
	int do_what_mp;
	struct listener_mp_fun_f_param *param_f;
	struct listener_mp_fun_f_result *result_f;
	char name_mp[16];

	ipc_mp = *((struct lxc_ipc *)arg);
	sem_post(&sem_mp);

	snprintf(name_mp, sizeof(name_mp), "module_mp_%d_%d", ipc_mp.tid, ipc_mp.buf_id);
	mt_set_pthread_name(name_mp);

	while (1) {
		lxc_ipc_accept(&ipc_mp, &do_what_mp, &in_buf_mp, &out_buf_mp, &in_size_mp, &out_size_mp);
		/* in_size_mp and out_size_mp was be update by kernel */
		printf("in_size_mp = %d\n", in_size_mp);
		printf("out_size_mp = %d\n", out_size_mp);
		printf("tid = %d\n", ipc_mp.tid);
		printf("buf_id = %d\n", ipc_mp.buf_id);

		param_f = (struct listener_mp_fun_f_param *)in_buf_mp;
		result_f = (struct listener_mp_fun_f_result *)out_buf_mp;

		switch (do_what_mp) {
		case LISTENER_MP_FUN_F:
			result_f->r = listener_mp_fun_f(param_f, result_f, out_size_mp);
			lxc_ipc_done(&ipc_mp);
			break;
		case LISTENER_MP_FUN_EXIT:
			lxc_ipc_close_accepted(&ipc_mp);
			lxc_ipc_done(&ipc_mp);
			goto exit;
		default:
			break;
		}
	}

exit:
	return NULL;
}

int main(int argc, char **argv)
{
	char name_mp[LXC_IPC_NAME_SIZE] = "module_mp";
	struct lxc_ipc ipc_mp_listen;
	struct lxc_ipc ipc_mp;
	pthread_t thread_mp;

	printf("listener_mp running\n");

	daemon_init();

	sem_init(&sem_mp, 0, 0);

	if (lxc_ipc_open_server(name_mp, &ipc_mp_listen)) {
		exit(1);
	}

	while (1) {
		lxc_ipc_listen(&ipc_mp_listen, &ipc_mp);
		pthread_create(&thread_mp, NULL, module_mp, &ipc_mp);
		pthread_detach(thread_mp);
		sem_wait(&sem_mp);
	}

	lxc_ipc_close(&ipc_mp_listen);

	return 0;
}
