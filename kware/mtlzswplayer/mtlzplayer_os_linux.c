/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 201, Montage Technology Co., Ltd.
 *
 * File Name      : mtlzplayer_os_linux.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/14
 * Description    : Monage-LZ SW Player Linux OS porting functions.
 * History        :
 * 1.Date         : 2019/03/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>

#include "mtlz_types.h"
#include "mtlzplayer_os.h"

//void *malloc(size_t size);
//void free(void *ptr);
//void assert(scalar expression);
//int usleep(useconds_t usec);
//int clock_gettime(clockid_t clk_id, struct timespec *tp);

//int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
//                   void *(*start_routine) (void *), void *arg);
//int pthread_join(pthread_t thread, void **retval);
//int prctl(int option, unsigned long arg2, unsigned long arg3,
//			unsigned long arg4, unsigned long arg5);
//int pthread_mutex_destroy(pthread_mutex_t *mutex);
//int pthread_mutex_init(pthread_mutex_t *restrict mutex,
//const pthread_mutexattr_t *restrict attr);
//int pthread_mutex_lock(pthread_mutex_t *mutex);
//int pthread_mutex_unlock(pthread_mutex_t *mutex);

void *MTLZ_MALLOC(size_t size)
{
	return malloc(size);
}

void MTLZ_FREE(void *ptr)
{
	if (ptr != NULL)
		free(ptr);
}

void MTLZ_ASSERT_DEBUG(bool expr, const char *function, int line)
{
	if (!(expr))
	{
		printf("[ASSERT] %s@%d!!!\n",function,line);
	}
	assert(expr);
}

int mtlz_msleep(unsigned int msec)
{
	if (msec > 0)
		return usleep(msec * 1000);

	return 0;
}

/* return ms */
unsigned long mtlz_get_tick(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (unsigned long)((unsigned long long)ts.tv_sec * 1000 + (unsigned long long)ts.tv_nsec / 1000000);
}

//optional
int mtlz_thread_set_name(const char *thread_name)
{
	if (thread_name != NULL)
		return prctl(PR_SET_NAME, thread_name);
	else
		return -1;
}

int mtlz_thread_create(unsigned int *thread_id, void *(*start_routine)(void *), void *arg)
{
	return pthread_create((pthread_t*)thread_id, NULL, start_routine, arg);
}

int mtlz_thread_join(unsigned int thread_id)
{
	return pthread_join((pthread_t)thread_id, NULL);
}

int mtlz_mutex_init(void **mutex)
{
	*mutex = malloc(sizeof(pthread_mutex_t));
	assert(*mutex != NULL);

	return pthread_mutex_init((pthread_mutex_t*)*mutex, NULL);
}

int mtlz_mutex_destroy(void *mutex)
{
	assert(mutex != NULL);

	pthread_mutex_destroy((pthread_mutex_t*)mutex);
	free(mutex);
	return 0;
}

int mtlz_mutex_lock(void *mutex)
{
	assert(mutex != NULL);

	pthread_mutex_lock((pthread_mutex_t*)mutex);
	return 0;
}

int mtlz_mutex_unlock(void *mutex)
{
	assert(mutex != NULL);

	pthread_mutex_unlock((pthread_mutex_t*)mutex);
	return 0;
}

