/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 201, Montage Technology Co., Ltd.
 *
 * File Name      : mtlzplayer_os_ucos.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/19
 * Description    : Monage-LZ SW Player uCOS OS porting functions.
 * History        :
 * 1.Date         : 2019/03/19
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "mt_type.h"
#include "sys_define.h"
//#include "sys_cfg.h"
#include "mtos_printk.h"
#include "mtos_mem.h"
#include "mtos_task.h"
#include "mtos_misc.h"
#include "mtos_sem.h"

#define PRINTF					mtos_printk

#define MAX_TASK_COUNT			1
#define TASK_STACK_SIZE			0x10000

#define CHECK_THREAD_ID(id)		assert((id)>=0 && (id)<MAX_TASK_COUNT)

struct mtos_task_porting_st
{
	char task_name[32];
	void *(*task_proc)(void *);
	void *param;
	u32 priority;
	u32 *stack;
	u32 stack_size;

	/* 0: idle, 1: running */
	volatile u32 state;
};
static struct mtos_task_porting_st mtos_porting_tasks[MAX_TASK_COUNT];

void *MTLZ_MALLOC(size_t size)
{
	return mtos_malloc(size);
}

void MTLZ_FREE(void *ptr)
{
	if (ptr != NULL)
		mtos_free(ptr);
}

void MTLZ_ASSERT_DEBUG(bool expr, const char *function, int line)
{
	if (!(expr))
	{
		PRINTF("[ASSERT] %s@%d!!!\n",function,line);
	}
	assert(expr);
}

int mtlz_msleep(unsigned int msec)
{
	if (msec > 0)
	{
		mtos_task_sleep(msec);
	}

	return msec;
}

/* return ms */
unsigned long mtlz_get_tick(void)
{
	u32 s, ms, us;

	mtos_systime_get(&s, &ms, &us);

	return (s*1000 + ms);
}

//optional
int mtlz_thread_set_name(const char *thread_name)
{
	//TODO
	return 0;
}

static void mtos_task_proc_wrapper(void *param)
{
	struct mtos_task_porting_st *pTsk = (struct mtos_task_porting_st*)param;
	void *(*task_proc)(void *);
	void *ret;

	assert(pTsk != NULL);

	mtos_task_lock();
	pTsk->state = 1;
	mtos_task_unlock();

	task_proc = pTsk->task_proc;
	assert(task_proc != NULL);

	ret = task_proc(pTsk->param);

	mtos_task_lock();
	pTsk->state = 0;
	mtos_task_unlock();

	mtos_task_exit();

	return;
}

int mtlz_thread_create(unsigned int *thread_id, void *(*start_routine)(void *), void *arg)
{
	//FIXME: how about multi tasks?
	struct mtos_task_porting_st *pTsk = &mtos_porting_tasks[0];

	snprintf(pTsk->task_name, 32, "mtlzswplayer_thread%d", 0);
	pTsk->task_proc = start_routine;
	pTsk->param = arg;
	pTsk->priority = (u32)mtlzplayer_get_priority_nb(0);
	PRINTF("%s: priority=%u\n",__FUNCTION__,pTsk->priority);
	pTsk->stack_size = TASK_STACK_SIZE;
	pTsk->stack = mtos_malloc(pTsk->stack_size);
	if (pTsk->stack == NULL)
	{
		PRINTF("[ERROR]%s: malloc stack failed!\n",__FUNCTION__);
		return -1;
	}

	if (mtos_task_create(pTsk->task_name,
					mtos_task_proc_wrapper,
					pTsk,
					pTsk->priority,
					pTsk->stack,
					pTsk->stack_size))
	{
		mtos_task_lock();
		pTsk->state = 1;
		mtos_task_unlock();

		PRINTF("%s: mtos_task_create(priority %d) success.\n",__FUNCTION__,pTsk->priority);
		*thread_id = 0;
		return 0;
	}
	else
	{
		PRINTF("[ERROR]%s: mtos_task_create(priority %d) failed!\n",__FUNCTION__,pTsk->priority);
		return -1;
	}
}

int mtlz_thread_join(unsigned int thread_id)
{
	struct mtos_task_porting_st *pTsk;

	CHECK_THREAD_ID(thread_id);

	pTsk = &mtos_porting_tasks[thread_id];

	while (pTsk->state) mtos_task_sleep(10);

	if (pTsk->stack != NULL)
	{
		mtos_free(pTsk->stack);
		pTsk->stack = NULL;
	}

	return 0;
}

int mtlz_mutex_init(void **mutex)
{
	*mutex = mtos_malloc(sizeof(os_sem_t));
	assert(*mutex != NULL);

	return mtos_sem_create((os_sem_t*)*mutex, TRUE)?0:-1;
}

int mtlz_mutex_destroy(void *mutex)
{
	int ret;

	assert(mutex != NULL);

	ret = mtos_sem_destroy((os_sem_t*)mutex, MTOS_DEL_NO_PEND)?0:-1;

	mtos_free(mutex);

	return ret;
}

int mtlz_mutex_lock(void *mutex)
{
	assert(mutex != NULL);

	return mtos_sem_take((os_sem_t*)mutex, SEM_WAIT_FOREVER)?0:-1;
}

int mtlz_mutex_unlock(void *mutex)
{
	assert(mutex != NULL);

	return mtos_sem_give((os_sem_t*)mutex)?0:-1;
}

