/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2023, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_unf_timerfd.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2023/11/29
 * Description    : MT UNF Timerfd
 * History        :
 * 1.Date         : 2023/11/29
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include "mt_common.h"
#include "mt_unf_timer.h"

#include "mt_error_mpi.h"
#include "mt_module_debug.h"

#define MAX_TIMERFD_COUNT		16

//#define DEBUG_TM

/* timerfd definition */
typedef struct mt_timerfd
{
	mt_u32 id;				/* timer ID */

	int fd;					/* timer fd */

	mt_u32 interval;		/* timer interval in ms */

	timer_fn callback;		/* timer callback */
	void *priv;				/* timer private data */

	mt_u64 trig_times;		/* timer trigger times */

	pthread_t thread_id;

} mt_timerfd_t;

static mt_timerfd_t *timerfd_tab[MAX_TIMERFD_COUNT] = {0};
static int timerfd_count = 0;
static pthread_mutex_t timerfd_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * register new timer, and return the ID
 */
static mt_u32 register_timer(mt_timerfd_t *tm)
{
	mt_u32 i;

	for (i=0; i<MAX_TIMERFD_COUNT; i++)
	{
		if (timerfd_tab[i] == NULL)
		{
			tm->id = i;

			timerfd_tab[i] = tm;

			timerfd_count ++;

			return i;
		}
	}

	return (mt_u32)(-1);
}

static void unregister_timer(mt_u32 id)
{
	if (timerfd_count > 0)
	{
		timerfd_tab[id] = NULL;
		timerfd_count --;
	}
}

static mt_s32 start_timer(int fd, mt_u32 u32Timerms)
{
	struct timespec now;
	struct itimerspec new_value;

	if (clock_gettime(CLOCK_REALTIME, &now) == -1)
	{
		MT_ERR_TIMER("clock gettime failed!\n");
		return MT_FAILURE;
	}

	new_value.it_value.tv_sec = now.tv_sec + (u32Timerms / 1000);
	new_value.it_value.tv_nsec = now.tv_nsec + (u32Timerms % 1000) * 1000000;
	new_value.it_value.tv_sec += new_value.it_value.tv_nsec / 1000000000;
	new_value.it_value.tv_nsec %= 1000000000;

	new_value.it_interval.tv_sec = u32Timerms / 1000;
	new_value.it_interval.tv_nsec = (u32Timerms % 1000) * 1000000;

	if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
	{
		MT_ERR_TIMER("timerfd settime failed!\n");
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

static void stop_timer(int fd)
{
	struct itimerspec new_value;

	memset(&new_value, 0, sizeof(new_value));

	if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
	{
		MT_ERR_TIMER("timerfd settime failed!\n");
	}
}

static void *timer_thread(void *arg)
{
	int fd;
	ssize_t s;
	uint64_t exp;

#ifdef DEBUG_TM
	mt_u32 id;
#endif
	timer_fn callback;
	void *priv;

	mt_u64 trig_times = 0;

	mt_timerfd_t *tm = (mt_timerfd_t*)arg;

	fd = tm->fd;
#ifdef DEBUG_TM
	id = tm->id;
#endif
	callback = tm->callback;
	priv = tm->priv;

	while (MT_TRUE)
	{
		s = read(fd, &exp, sizeof(uint64_t));

		if (s != sizeof(uint64_t))
		{
			MT_ERR_TIMER("invalid param!\n");
			//break;	//break?
		}
		else
		{
			trig_times ++;
			tm->trig_times ++;

			MT_DBG_TIMER("Timer[%u]: %llu\n", id, trig_times);

			if (callback)
				callback(priv);
		}
	}

	MT_DBG_TIMER("Timer[%u]: exit!\n", id);

#ifdef DEBUG_TM
	printf("Timer[%u]: exit!\n", id);
#endif

	return NULL;
}

mt_s32 mt_unf_timerfd_create(mt_u32 u32Timerms, timer_fn callback, void *priv, mt_u32 *pu32TimerID)
{
	mt_s32 ret = MT_FAILURE;
	int fd = -1;
	mt_timerfd_t *tm = NULL;
	pthread_attr_t attr;

	if (u32Timerms == 0 || callback == NULL || pu32TimerID == NULL)
	{
		MT_ERR_TIMER("invalid param!\n");
        return MT_ERR_TIMER_INVALID_POINT;
	}

	pthread_mutex_lock(&timerfd_mutex);

	if (timerfd_count >= MAX_TIMERFD_COUNT)
	{
		MT_ERR_TIMER("timer count overflow!\n");
        ret = MT_ERR_TIMER_INVALID_POINT;
        goto ERR;
	}

	tm = (mt_timerfd_t*)malloc(sizeof(mt_timerfd_t));
	if (tm == NULL)
	{
		MT_ERR_TIMER("allocate mem failed!\n");
		ret = MT_ERR_TIMER_FAILED_INIT;
        goto ERR;
	}

	memset(tm, 0, sizeof(mt_timerfd_t));

	tm->interval = u32Timerms;
	tm->callback = callback;
	tm->priv = priv;

	fd = timerfd_create(CLOCK_REALTIME, 0);
	if (fd == -1)
	{
		MT_ERR_TIMER("timerfd create failed!\n");
		ret = MT_ERR_TIMER_FAILED_INIT;
        goto ERR;
	}

	tm->fd = fd;

	ret = pthread_attr_init(&attr);
	if (ret != 0)
	{
		MT_ERR_TIMER("pthread attr init failed!\n");
		ret = MT_ERR_TIMER_FAILED_INIT;
        goto ERR_TM1;
	}

	if (pthread_create(&tm->thread_id, &attr, timer_thread, tm) != 0)
	{
		MT_ERR_TIMER("pthread create failed!\n");
		ret = MT_ERR_TIMER_FAILED_INIT;
        goto ERR_TM1;
	}

	pthread_attr_destroy(&attr);

	*pu32TimerID = register_timer(tm);

	MT_DBG_TIMER("create timer[%u](%u, %p, %p) success.\n", tm->id,
		u32Timerms, callback, priv);

#ifdef DEBUG_TM
	printf("create timer[%u](%u, %p, %p) success.\n", tm->id,
		u32Timerms, callback, priv);
#endif

	pthread_mutex_unlock(&timerfd_mutex);
	return MT_SUCCESS;

ERR_TM1:
	/* close */
	if (fd != -1)
	{
		close(fd);
		fd = -1;
	}

ERR:
	if (tm != NULL)
	{
		free(tm);
		tm = NULL;
	}

	pthread_mutex_unlock(&timerfd_mutex);

	return ret;
}

mt_s32 mt_unf_timerfd_start(mt_u32 u32TimerID)
{
	mt_s32 ret = MT_FAILURE;
	mt_timerfd_t *tm = NULL;

	if (u32TimerID >= MAX_TIMERFD_COUNT)
	{
		MT_ERR_TIMER("timer id overflow!\n");
		return MT_ERR_TIMER_INVALID_ID;
	}

	pthread_mutex_lock(&timerfd_mutex);

	tm = timerfd_tab[u32TimerID];
	if (tm == NULL)
	{
		MT_ERR_TIMER("timer is null!\n");
		ret = MT_ERR_TIMER_INVALID_POINT;
		goto ERR;
	}

	if (start_timer(tm->fd, tm->interval) != MT_SUCCESS)
	{
		MT_ERR_TIMER("start timer failed!\n");
		ret = MT_ERR_TIMER_FAILED_INIT;
        goto ERR;
	}

	ret = MT_SUCCESS;

ERR:
	pthread_mutex_unlock(&timerfd_mutex);

	return ret;
}

mt_s32 mt_unf_timerfd_stop(mt_u32 u32TimerID)
{
	mt_s32 ret = MT_FAILURE;
	mt_timerfd_t *tm = NULL;

	if (u32TimerID >= MAX_TIMERFD_COUNT)
	{
		MT_ERR_TIMER("timer id overflow!\n");
		return MT_ERR_TIMER_INVALID_ID;
	}

	pthread_mutex_lock(&timerfd_mutex);

	tm = timerfd_tab[u32TimerID];
	if (tm == NULL)
	{
		MT_ERR_TIMER("timer is null!\n");
		ret = MT_ERR_TIMER_INVALID_POINT;
		goto ERR;
	}

	/* stop timer */
	stop_timer(tm->fd);

	ret = MT_SUCCESS;

ERR:
	pthread_mutex_unlock(&timerfd_mutex);

	return ret;
}

mt_s32 mt_unf_timerfd_delete(mt_u32 u32TimerID)
{
	mt_s32 ret = MT_FAILURE;
	mt_timerfd_t *tm = NULL;

	if (u32TimerID >= MAX_TIMERFD_COUNT)
	{
		MT_ERR_TIMER("timer id overflow!\n");
		return MT_ERR_TIMER_INVALID_ID;
	}

	pthread_mutex_lock(&timerfd_mutex);

	tm = timerfd_tab[u32TimerID];
	if (tm == NULL)
	{
		MT_ERR_TIMER("timer is null!\n");
		ret = MT_ERR_TIMER_INVALID_POINT;
		goto ERR;
	}

	/* stop timer */
	stop_timer(tm->fd);

	pthread_cancel(tm->thread_id);

	pthread_join(tm->thread_id, NULL);

	/* close */
	close(tm->fd);

	unregister_timer(u32TimerID);

	memset(tm, 0, sizeof(mt_timerfd_t));
	free(tm);
	tm = NULL;

	MT_DBG_TIMER("delete timer[%u] success.\n", u32TimerID);
#ifdef DEBUG_TM
	printf("delete timer[%u] success.\n", u32TimerID);
#endif

	ret = MT_SUCCESS;

ERR:
	pthread_mutex_unlock(&timerfd_mutex);

	return ret;
}

