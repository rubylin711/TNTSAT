/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <string.h>
#include<stdlib.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

#include "mt_type.h"
#include "mtos_task.h"
#include "mtos_printk.h"
#include "mtos_sem.h"

#include "drv_os.h"

#define MTOS_TASK_PRIO_LOW_NUM (192) //(64)
#define MTOS_TASK_PRIO_LOW_MID_NUM (16)
#define MTOS_TASK_PRIO_MID_NUM (16)
#define MTOS_TASK_PRIO_MID_HIGH_NUM (16)
//#define MTOS_TASK_PRIO_HIGH_NUM (16)

#define MAX_TASK_NUM (MTOS_TASK_PRIO_LOW_NUM +      \
                      MTOS_TASK_PRIO_LOW_MID_NUM +  \
                      MTOS_TASK_PRIO_MID_NUM +      \
                      MTOS_TASK_PRIO_MID_HIGH_NUM + \
                      MTOS_TASK_PRIO_HIGH_NUM)
#define LOW_PRIO (10)
#define LOW_MID_PRIO (30)
#define MID_PRIO (50)
#define MID_HIGH_PRIO (70)
#define HIGH_PRIO (90)
#define HIGHEST_PRIO (94)

#ifndef MTOS_TASK_PRIO_HIGH
#define MTOS_TASK_PRIO_HIGH HIGH_PRIO
#endif

#ifndef MTOS_TASK_PRIO_MID_HIGH
#define MTOS_TASK_PRIO_MID_HIGH MID_HIGH_PRIO
#endif
#ifndef MTOS_TASK_PRIO_MID
#define MTOS_TASK_PRIO_MID MID_PRIO
#endif
#ifndef MTOS_TASK_PRIO_LOW_MID
#define MTOS_TASK_PRIO_LOW_MID LOW_MID_PRIO
#endif

#ifndef MTOS_TASK_PRIO_LOW
#define MTOS_TASK_PRIO_LOW LOW_PRIO
#endif



/*!
    Common task map info
  */
typedef struct
{
    /*!
      The task priority
     */
    U32 priority;
    /*!
       The task real no
     */
    u32 task_no;
} task_map_t;

os_sem_t task_sem = 0;
task_map_t task_map[MAX_TASK_NUM];

static u32 _find_task_id(u32 task_no)
{
    u32 i = 0;

    for (i = 0; i < MAX_TASK_NUM; i++) {
	if (task_map[i].task_no == task_no)
	    break;
    }

    return i;
}

void mtos_task_init(void)
{
    u32 i = 0;

    memset(&task_map, 0, (sizeof(task_map_t) * MAX_TASK_NUM));

    for (i = 0; i < MAX_TASK_NUM; i++) {
	if ((i >= MTOS_TASK_PRIO_HIGHEST) && (i < MTOS_TASK_PRIO_HIGH)) {   //only uio used
	    task_map[i].priority = HIGHEST_PRIO;
		}
	//high priority
	if ((i >= MTOS_TASK_PRIO_HIGH) && (i < MTOS_TASK_PRIO_MID_HIGH)) {
	    task_map[i].priority = HIGH_PRIO;
	}

	//mid-high priority
	if ((i >= MTOS_TASK_PRIO_MID_HIGH) && (i < MTOS_TASK_PRIO_MID)) {
	    task_map[i].priority = MID_HIGH_PRIO;
	}

	//mid priority
	if ((i >= MTOS_TASK_PRIO_MID) && (i < MTOS_TASK_PRIO_LOW_MID)) {
	    task_map[i].priority = LOW_MID_PRIO;
	}

	//low-mid priority
	if ((i >= MTOS_TASK_PRIO_LOW_MID) && (i < MTOS_TASK_PRIO_LOW)) {
	    task_map[i].priority = LOW_MID_PRIO;
	}

	//low priority
	if ((i >= MTOS_TASK_PRIO_LOW) && (i < MAX_TASK_NUM)) {
	    task_map[i].priority = LOW_PRIO;
	}
    }

#ifdef DRV_SEM_DEBUG
    (void)mtos_sem_create(&task_sem, TRUE, (const u8 *)"mtos_task_init:task_sem");
#else
    (void)mtos_sem_create(&task_sem, TRUE);
#endif

    return;
}

void mtos_task_set_sched_policy(mtos_task_policy_t policy)
{
  int schpolicy = SCHED_RR;
  switch(policy)
  {
   case MTOS_TASK_SCHED_POLICY_OTHER:
    schpolicy = SCHED_OTHER;
   break;
   case MTOS_TASK_SCHED_POLICY_FIFO:
   schpolicy = SCHED_FIFO;
   break;
   case MTOS_TASK_SCHED_POLICY_RR:
   default:
    schpolicy = SCHED_RR;
   break;
  }
  SYS_TaskSetSchedPolicy(schpolicy);
}

MT_BOOL mtos_task_create(u8 *p_taskname,
                      void (*p_taskproc)(void *p_param),
                      void *p_param,
                      u32 task_id,
                      u32 *pstk,
                      u32 nstksize)
{
    ErrorCode_t err = ERROR_CODE_ERROR_RESULT;

    if ((p_taskname == NULL) || (p_taskproc == NULL) || (task_id >= MAX_TASK_NUM) || (nstksize == 0)) {
	mtos_printk("[mtos_task_create] param is ERROR!\n");
	return FALSE;
    }

    (void)mtos_sem_take(&task_sem, 0);

    err = SYS_TaskCreate((void *)p_taskproc,
                         (void *)p_param,
                         (U32)nstksize,
                         (U32)task_map[task_id].priority,
                         (U32 *)&task_map[task_id].task_no,
                         (const U8 *)p_taskname,
                         NULL);
    if (err != ERROR_CODE_NO_ERROR) {
	mtos_printk("[mtos_task_create] create task ERROR!\n");
	(void)mtos_sem_give(&task_sem);
	return FALSE;
    }

    (void)mtos_sem_give(&task_sem);

    return TRUE;
}

s32 mtos_task_exit(void)
{
    SYS_TaskExit(0);
    return SUCCESS;
}

void mtos_task_sleep(u32 ms)
{
    SYS_TaskDelay(ms);
    return;
}

void mtos_task_lock()
{
    return;
}

void mtos_task_unlock()
{
    return;
}

s32 mtos_task_resume(u32 task_id)
{
    U32 task_no = 0;
    ErrorCode_t err = ERROR_CODE_ERROR_RESULT;

    if (task_id >= MAX_TASK_NUM) {
	mtos_printk("[mtos_task_resume] param is ERROR!\n");
	return FALSE;
    }

    (void)mtos_sem_take(&task_sem, 0);
    task_no = task_map[task_id].task_no;
    (void)mtos_sem_give(&task_sem);

    err = SYS_TaskResume(task_no);
    if (err != ERROR_CODE_NO_ERROR) {
	mtos_printk("[mtos_task_resume] resume task(%d) ERROR!\n", task_id);
	return ERR_FAILURE;
    }

    return SUCCESS;
}

s32 mtos_task_suspend(u32 task_id)
{
    U32 task_no = 0;
    ErrorCode_t err = ERROR_CODE_ERROR_RESULT;

    if (task_id >= MAX_TASK_NUM) {
	mtos_printk("[mtos_task_suspend] param is ERROR!\n");
	return FALSE;
    }

    (void)mtos_sem_take(&task_sem, 0);
    task_no = task_map[task_id].task_no;
    (void)mtos_sem_give(&task_sem);

    err = SYS_TaskSuspend(task_no);
    if (err != ERROR_CODE_NO_ERROR) {
	mtos_printk("[mtos_task_suspend] suspend task(%d) ERROR!\n", task_id);
	return ERR_FAILURE;
    }

    return SUCCESS;
}

s32 mtos_task_change_prio(u32 old_prio, u32 new_prio)
{
    return 0;
}

s32 mtos_task_get_info(mtos_task_info_t *p_info)
{
    return 0;
}
#if 0
s32 mtos_task_delete(u32 task_id)
{
  //not support for detached thread
#if 0
  ErrorCode_t err = ERROR_CODE_ERROR_RESULT;

  err = SYS_TaskKill(task_id);
  if(err != ERROR_CODE_NO_ERROR)
  {
    mtos_printk("[mtos_task_delete] delete task ERROR!\n");
    return ERR_FAILURE;
  }
#endif
  return SUCCESS;
}
#endif
u8 mtos_task_get_curn_prio()
{
    return 0;
}

u32 mtos_task_get_os_version()
{
    return 0;
}

s32 mtos_task_get_cur_task_id(u32 *p_task_id)
{
    u32 task_no = 0;
    u32 task_id = 0;
    ErrorCode_t err = ERROR_CODE_ERROR_RESULT;

    //check param
    if (p_task_id == NULL) {
	mtos_printk("[mtos_task_get_cur_task_id] param is ERROR!\n");
	return ERR_FAILURE;
    }

    err = SYS_GetCurTaskID((U32 *)&task_no);
    if (err != ERROR_CODE_NO_ERROR) {
	mtos_printk("[mtos_task_get_cur_task_id] get cur task id ERROR!\n");
	return ERR_FAILURE;
    }

    task_id = _find_task_id(task_no);
    if (task_id >= MAX_TASK_NUM) {
	mtos_printk("[mtos_task_get_cur_task_id] find task id ERROR!\n");
	return ERR_FAILURE;
    }

    (*p_task_id) = task_id;

    return SUCCESS;
}

#if 1
s32 mtos_task_get_state(u32 task_id, mtos_task_state_t *p_state)
{
    U32 task_no = 0;
    SysTaskState_t state = SYS_TASK_IDLE;
    ErrorCode_t err = ERROR_CODE_ERROR_RESULT;

    //check param
    if ((task_id >= MAX_TASK_NUM) || (p_state == NULL)) {
	mtos_printk("[mtos_task_get_state] param is ERROR!\n");
	return ERR_FAILURE;
    }

    (void)mtos_sem_take(&task_sem, 0);
    task_no = task_map[task_id].task_no;
    (void)mtos_sem_give(&task_sem);

    err = SYS_TaskGetState(task_no, &state);
    if (err != ERROR_CODE_NO_ERROR) {
	mtos_printk("[mtos_task_get_state] task get state ERROR!\n");
	return ERR_FAILURE;
    }

    switch (state) {
    case SYS_TASK_IDLE:
	(*p_state) = MTOS_TASK_IDLE;
	break;
    case SYS_TASK_RUN:
	(*p_state) = MTOS_TASK_RUN;
	break;
    case SYS_TASK_SUSPEND:
	(*p_state) = MTOS_TASK_SUSPEND;
	break;
    default:
	(*p_state) = MTOS_TASK_IDLE;
	break;
    }

    return SUCCESS;
}
#else
s32 mtos_task_get_state(u32 task_id, mtos_task_state_t *p_state){

}
#endif

void mtos_task_debug(void)
{
    SYS_TaskDebug();
    return;
}

/********it is a temporary solution and code***********/
typedef struct gt_system_param
{
  char cmd[1024];
  MT_BOOL ret;
}system_param_t;
static volatile MT_BOOL is_exit_system = FALSE;
static pthread_mutex_t system_mutex = PTHREAD_MUTEX_INITIALIZER;

static void mt_system_function(void *p_param)
{
  system_param_t *p_sys_param = (system_param_t *)p_param;
  p_sys_param->ret = system((char *)p_sys_param->cmd);
  is_exit_system = TRUE;
  mtos_task_exit();
}

MT_BOOL mt_system(const char *pcmd)
{
  u32 task_no = 0;
  ErrorCode_t err = ERROR_CODE_ERROR_RESULT;
  //MT_BOOL ret = FALSE;
  u32 stackSize = 32 * 1024;
  u32 cmdlen = 0;
  system_param_t param = {0};

  pthread_mutex_lock(&system_mutex);
  memset(&param,0,sizeof(system_param_t));
  cmdlen = strlen(pcmd);
  if(cmdlen > 1024)
  {
     pthread_mutex_unlock(&system_mutex);
     return -1;
  }
  strcpy(param.cmd,pcmd);
  param.ret = -1;

  OS_PRINTF("###cgf debug[%s].%d start\n",__FUNCTION__,__LINE__);
  is_exit_system = FALSE;

  /* start ui init task */
  err = SYS_TaskCreate((void *)mt_system_function,
                         (void *)&param,
                         (U32)stackSize,
                         (U32)task_map[MTOS_TASK_PRIO_MID].priority,
                         (U32 *)&task_no,
                         (const U8 *)"system function",
                         NULL);
    if(err != ERROR_CODE_NO_ERROR)
    {
       pthread_mutex_unlock(&system_mutex);
       return -1;
    }

  	while(is_exit_system == FALSE)
    {
      mtos_task_sleep(100);
    }

    //FIX: Bug 124132
    //mt_system_function already exit,
    //"task_no" is not valid anymore!!!
    /* SYS_TaskKill(task_no); */

    OS_PRINTF("###cgf debug[%s].%d end\n",__FUNCTION__,__LINE__);
    pthread_mutex_unlock(&system_mutex);
    return param.ret;
}
/********it is a temporary solution and code end***********/
