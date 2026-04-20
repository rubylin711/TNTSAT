/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void *start_routine1(void *p)
{
	while (1) {
		sleep(1);
	}
	return NULL;
}

static void *start_routine(void *p)
{
	pthread_t thread;
	pthread_attr_t attr;
	struct sched_param param;
	int ret;

	param.sched_priority = 0;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	ret = pthread_create(&thread, &attr, start_routine1, NULL);
	printf("pthread_create1: %d\n", ret);
	pthread_join(thread, NULL);

	while (1) {
		sleep(1);
	};
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t thread;
	pthread_attr_t attr;
	struct sched_param param;
	int ret;

	param.sched_priority = 1;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedparam(&attr, &param);
	ret = pthread_create(&thread, &attr, start_routine, NULL);
	printf("pthread_create: %d\n", ret);
	pthread_join(thread, NULL);

	return 0;
}
