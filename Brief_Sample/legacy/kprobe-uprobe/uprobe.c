/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mt_ftrace.h"
#include "mt_common.h"
#include "libuprobe.h"
#include "kprobe/kprobe.h"

#define TEST_DELAY
#undef TRACE_MARKER
#undef TEST_FIFO

static sem_t sem;
static int fd = -1;
static struct timespec old, new;

#ifndef TEST_DELAY
static void foo(void)
{
	volatile unsigned int loop = 0xfffffff;
	volatile unsigned int x = 0;

	printf("foo begin\n");
	while (loop--) {
		x++;
	}
	printf("foo done\n");
	printf("\n");
}
#endif

static void *start_routine_other(void *p)
{
	mt_set_pthread_name("uprobe_other");
	while (1) {
#ifdef TRACE_MARKER
		mt_ftrace_mark0();
		printf("mt_ftrace_mark0\n");
#else
		mt_usdt_mark0();
		printf("mt_usdt_mark0\n");
#endif
		clock_gettime(CLOCK_MONOTONIC, &old);

		bar(4, 5);
		bar(6, 7);
		sem_post(&sem);
		bar(8, 9);
		bar(10, 11);
		printf("\n");

		clock_gettime(CLOCK_MONOTONIC, &new);

		if ((((unsigned long long)new.tv_sec * 1000000000ULL) + (unsigned long long)new.tv_nsec) -
			(((unsigned long long)old.tv_sec * 1000000000ULL) + (unsigned long long)old.tv_nsec) >
			100000000UL) {		/* check > 100ms */
#ifdef TRACE_MARKER
			mt_ftrace_mark2();
			printf("mt_ftrace_mark2\n");
#else
			mt_usdt_mark2();
			printf("mt_usdt_mark2\n");
#endif
			mt_ftrace_stop();
		} else {
#ifdef TRACE_MARKER
			mt_ftrace_mark1();
			printf("mt_ftrace_mark1\n");
#else
			mt_usdt_mark1();
			printf("mt_usdt_mark1\n");
#endif
		}

		sleep(1);
	}
	return NULL;
}

static void *start_routine_rr(void *p)
{
	pthread_t thread;
	pthread_attr_t attr;
	struct sched_param param;
	int ret;
	char mt_strings[][10] = {"mt 000000", "mt 111111", "mt 222222",
				"mt 333333", "mt 444444", "mt 555555"};

	mt_set_pthread_name("uprobe_rr");

	param.sched_priority = 0;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	ret = pthread_create(&thread, &attr, start_routine_other, NULL);
	pthread_setname_np(thread, "uprobe_other");
	printf("pthread_create other: %d\n", ret);

	while (1) {
		mt_ftrace_mark_string(mt_strings[0]);
		sem_wait(&sem);
		mt_ftrace_mark_string(mt_strings[1]);
#ifndef TEST_DELAY
		mt_ftrace_mark_string(mt_strings[2]);
		foo();
		mt_ftrace_mark_string(mt_strings[3]);
#else
		printf("ioctl begin\n");
		mt_ftrace_mark_string(mt_strings[4]);
		ioctl(fd, MT_KPROBE_TEST_DELAY, 0);
		mt_ftrace_mark_string(mt_strings[5]);
		printf("ioctl done\n");
		printf("\n");
#endif
	}

	pthread_join(thread, NULL);
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t thread;
	pthread_attr_t attr;
	struct sched_param param;
	int ret;

	sem_init(&sem, 0, 0);
	fd = open("/dev/mt_kprobe_test", O_RDWR, 0644);

	printf("enter enter key to continue\n");
	getchar();

	pthread_attr_init(&attr);
#ifndef TEST_FIFO
	param.sched_priority = 1;
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
#else
	param.sched_priority = 99;
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
#endif
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedparam(&attr, &param);
	ret = pthread_create(&thread, &attr, start_routine_rr, NULL);
	pthread_setname_np(thread, "uprobe_rr");
	printf("pthread_create rr: %d\n", ret);
	pthread_join(thread, NULL);

	return 0;
}
