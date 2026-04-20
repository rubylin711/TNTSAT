/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "mt_type.h"
#include "mtos_timer.h"
#include "mtos_sem.h"
#include "mtos_task.h"
#include "mtos_printk.h"
#include "mt_common.h"
#include "drv_os.h"

#define SIG_TIMER		SIGUSR2		/*or SIGALRM*/

#define TIMER_MAX_NUM 	(10)

//HZ: 1000, 250, 100
//1/HZ: 1, 4, 10
//lowest common multiple: 20
#define TIMER_PER_MSEC 	(20)

typedef s32 timer_handle_t;

typedef struct
{
    MT_BOOL is_used; /* on or off */
    int interval;
    int elapse; /* 0~interval */
    void (*timer_proc)(ulong context);
    ulong context;
    MT_BOOL is_cycle;
    MT_BOOL is_paused;
} timer_info_t;

typedef struct
{
    timer_info_t timer[TIMER_MAX_NUM];
    struct itimerspec value;
    struct itimerspec ovalue;
    os_sem_t sem_id;
    u8 exit_flag;

    //debug
    u32 total_times;
    struct timespec start_tick;
    struct timespec last_tick;
    MT_BOOL is_suspend;
} timer_ctrl_t;

static timer_ctrl_t timer_ctrl;

static void sig_handler(int sig, siginfo_t *info, void *ucontext)
{
   u16 i = 0;

	if (sig == SIG_TIMER)
	{
		if (info != NULL && info->si_code == SI_TIMER)
		{
			//debug
			if (0)
			{
			  timer_ctrl.total_times ++;
			  clock_gettime(CLOCK_MONOTONIC, &timer_ctrl.last_tick);
				if ((timer_ctrl.total_times % 1000) == 0)
				{
					mtos_printk("[TM]: times=%u, start(%lu, %ld), last(%lu, %ld)\n",
						timer_ctrl.total_times,
						timer_ctrl.start_tick.tv_sec,timer_ctrl.start_tick.tv_nsec,
						timer_ctrl.last_tick.tv_sec,timer_ctrl.last_tick.tv_nsec);
				}
			}

			/* Bug 110103:
			 *     deadlock with mtos_timer_reset().
			 */
			//(void)mtos_sem_take(&(timer_ctrl.sem_id), 0);
		    /* if (!mtos_sem_trytake(&(timer_ctrl.sem_id)))*/ 	/* trytake cost too much cpu,
		    												       other api could not take the sem. */
			/* 10ms: sem_timedwait() might NOT support 1ms precision */
		    if (!mtos_sem_take(&(timer_ctrl.sem_id), 10))
		    {
				//mtos_printk("[sig_func] mtos_sem_take FAIL!\n");
				return;
		    }

	        if(timer_ctrl.is_suspend)
	        {
	         (void)mtos_sem_give(&(timer_ctrl.sem_id));
	         return;
	        }

		    for (i = 0; i < TIMER_MAX_NUM; i++) {
				if ((timer_ctrl.timer[i].is_used == FALSE) || (timer_ctrl.timer[i].is_paused)) {
				    continue;
				}

				timer_ctrl.timer[i].elapse += TIMER_PER_MSEC;
				//mtos_printk("i %d, elapse %d, interval %d\n",
				//  i, timer_ctrl.timer[i].elapse, timer_ctrl.timer[i].interval);

				if (timer_ctrl.timer[i].elapse >= timer_ctrl.timer[i].interval) {
				    timer_ctrl.timer[i].elapse = 0;

				    if (timer_ctrl.timer[i].is_cycle == FALSE) {
					timer_ctrl.timer[i].is_used = FALSE;
				    }

				    (void)mtos_sem_give(&(timer_ctrl.sem_id));
				    timer_ctrl.timer[i].timer_proc(timer_ctrl.timer[i].context);
				    if (!mtos_sem_take(&(timer_ctrl.sem_id), 0))
				    {
				    	mtos_printk("[ERROR]%s: mtos_sem_take failed!\n",__FUNCTION__);
				    }
				}
		    }

		    (void)mtos_sem_give(&(timer_ctrl.sem_id));
	    }
	    else
	    {
	    	if (info != NULL)
	    	{
				mtos_printk("%s: bad si_code(%d)!\n",__FUNCTION__,info->si_code);
			}
	    }
	}
	else
	{
		mtos_printk("%s: bad sig(%d)!\n",__FUNCTION__,sig);
	}

    return;
}

//static void *tmr_proc_thread(void *p_param)
static void tmr_proc_thread(void *p_param)
{
	int ret;

    struct sigaction action;
	struct sigevent stSigEvt;
	timer_t posix_timer;

    mt_set_pthread_name(__FUNCTION__);
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_sigaction = sig_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_flags |= SA_SIGINFO;
    action.sa_flags |= SA_RESTART;	/*SA_RESTART: 重启系统调用, but might not work!*/
    sigaction(SIG_TIMER, &action, NULL);

    timer_ctrl.value.it_value.tv_sec = 0;
    //timer_ctrl.value.it_value.tv_nsec = 1000000;
    timer_ctrl.value.it_value.tv_nsec = TIMER_PER_MSEC*1000000;
    timer_ctrl.value.it_interval.tv_sec = 0;
    timer_ctrl.value.it_interval.tv_nsec = TIMER_PER_MSEC*1000000;

	memset (&stSigEvt, 0, sizeof (struct sigevent));
	stSigEvt.sigev_notify = SIGEV_SIGNAL | SIGEV_THREAD_ID;
	stSigEvt.sigev_signo = SIG_TIMER;
	//stSigEvt.sigev_notify_thread_id = gettid();   //fuck toolchain, bits/types/sigevent_t.h not define sigev_notify_thread_id
	stSigEvt._sigev_un._tid = gettid();

	ret = timer_create(CLOCK_MONOTONIC, &stSigEvt, &posix_timer);
	if (ret != 0)
	{
		mtos_printk("[ERROR]%s: timer_create failed, errno=%d\n",__FUNCTION__,errno);
	}

	ret = timer_settime(posix_timer, 0,/*TIMER_ABSTIME,*/ &timer_ctrl.value, &timer_ctrl.ovalue);
	if (ret != 0)
	{
		mtos_printk("[ERROR]%s: timer_settime failed, errno=%d\n",__FUNCTION__,errno);
	}

	clock_gettime(CLOCK_MONOTONIC, &timer_ctrl.start_tick);

    while (1) {
		if (timer_ctrl.exit_flag == 1) {
		    break;
		}

		/* sleep until a
           signal is delivered that either terminates the process or causes the
           invocation of a signal-catching function.
         */
		pause();	/* must use pause, don't use sleep/nanosleep/usleep/msleep */
    }

    memset(&timer_ctrl, 0, sizeof(timer_ctrl_t));

    SYS_TaskExit(0);

    //return NULL;
    return;
}

static timer_handle_t set_a_timer(int interval, void (*timer_proc)(ulong context), ulong context, MT_BOOL is_cyc)
{
    u16 i = 0;

    if ((timer_proc == NULL) || (interval <= 0)) {
		return (-1);
    }

    for (i = 0; i < TIMER_MAX_NUM; i++) {
		if (timer_ctrl.timer[i].is_used == TRUE) {
		    continue;
		}

		memset(&timer_ctrl.timer[i], 0, sizeof(timer_ctrl.timer[i]));
		timer_ctrl.timer[i].timer_proc = timer_proc;
		timer_ctrl.timer[i].context = context;
		timer_ctrl.timer[i].interval = interval;
		timer_ctrl.timer[i].is_cycle = is_cyc;
		timer_ctrl.timer[i].elapse = 0;
		timer_ctrl.timer[i].is_used = TRUE;
		timer_ctrl.timer[i].is_paused = FALSE;
		break;
    }

    if (i >= TIMER_MAX_NUM) {
		return (-1);
    }

    return (timer_handle_t)(i);
}

static int del_a_timer(timer_handle_t handle)
{
    if ((handle < 0) || (handle >= TIMER_MAX_NUM)) {
		return (-1);
    }

    memset(&timer_ctrl.timer[handle], 0, sizeof(timer_info_t));

    return (0);
}

s32 mtos_timer_init(u32 prio, u32 stack_size)
{
    MT_BOOL ret = FALSE;

    memset(&timer_ctrl, 0, sizeof(timer_ctrl_t));

#ifdef DRV_SEM_DEBUG
    (void)mtos_sem_create(&(timer_ctrl.sem_id), TRUE, (const u8 *)"mtos_timer_init:sem_id");
#else
    (void)mtos_sem_create(&(timer_ctrl.sem_id), TRUE);
#endif

    ret = mtos_task_create((u8 *)"tmr_task", tmr_proc_thread, NULL,
                           prio, NULL, stack_size);
    if (ret != TRUE) {
		mtos_printk("[mtos_timer_init] init timer ERROR!\n");
		return ERR_FAILURE;
    }

    return SUCCESS;
}

void mtos_timer_release()
{
    timer_ctrl.exit_flag = 1;
    return;
}

void mtos_timer_ioctrl(u32 cmd, u32 param)
{
    return;
}

s32 mtos_timer_create(u32 ntimerval,
                      void (*p_timerproc)(ulong funcontext),
                      ulong context,
                      MT_BOOL bcyclean)
{
    timer_handle_t h_timer = INVALID;

    (void)mtos_sem_take(&(timer_ctrl.sem_id), 0);

    h_timer = set_a_timer((int)ntimerval, p_timerproc, context, bcyclean);

    (void)mtos_sem_give(&(timer_ctrl.sem_id));

    return h_timer;
}

s32 mtos_timer_create_option(u32 ntimerval,
                             void (*p_timerproc)(ulong funcontext),
                             ulong context,
                             ulong option)
{
    return 0;
}

void mtos_timer_delete(s32 ntimerid)
{
	if ((ntimerid < 0) || (ntimerid >= TIMER_MAX_NUM))
		return;

    (void)mtos_sem_take(&(timer_ctrl.sem_id), 0);

    (void)del_a_timer((timer_handle_t)ntimerid);

    (void)mtos_sem_give(&(timer_ctrl.sem_id));

    return;
}

void mtos_timer_start(s32 ntimerid)
{
	if ((ntimerid < 0) || (ntimerid >= TIMER_MAX_NUM))
		return;

    (void)mtos_sem_take(&(timer_ctrl.sem_id), 0);

    timer_ctrl.timer[ntimerid].is_paused = FALSE;

    (void)mtos_sem_give(&(timer_ctrl.sem_id));

    return;
}

s32 mtos_timer_stop(s32 ntimerid)
{
	if ((ntimerid < 0) || (ntimerid >= TIMER_MAX_NUM))
		return (-1);

    (void)mtos_sem_take(&(timer_ctrl.sem_id), 0);

    timer_ctrl.timer[ntimerid].is_paused = TRUE;

    (void)mtos_sem_give(&(timer_ctrl.sem_id));

    return 0;
}

void mtos_timer_reset(s32 ntimerid, u32 ntimerval)
{
    //timer_info_t timer = {0};

	if ((ntimerid < 0) || (ntimerid >= TIMER_MAX_NUM))
		return;

    (void)mtos_sem_take(&(timer_ctrl.sem_id), 0);

    timer_ctrl.timer[ntimerid].elapse = 0;
    timer_ctrl.timer[ntimerid].interval = (int)ntimerval;

    (void)mtos_sem_give(&(timer_ctrl.sem_id));

    return;
}

s32 mtos_timer_running(s32 ntimerid)
{
    return 0;
}

void mtos_timer_get_sparetime(s32 ntimerid, u32 *p_sparetime)
{
    //check param
    if ((ntimerid < 0) || (ntimerid >= TIMER_MAX_NUM) || (p_sparetime == NULL)) {
	mtos_printk("[mtos_timer_get_sparetime] param is ERROR!\n");
	return;
    }

    (void)mtos_sem_take(&(timer_ctrl.sem_id), 0);

    *p_sparetime = (u32)(timer_ctrl.timer[ntimerid].interval - timer_ctrl.timer[ntimerid].elapse);

    (void)mtos_sem_give(&(timer_ctrl.sem_id));

    return;
}

void mtos_timer_debug(void)
{
    int i = 0;

    for (i = 0; i < TIMER_MAX_NUM; i++) {
	mtos_printk("Timer[%d]: used=%d, paused=%d, cycle=%d, elapse=%d, interval=%d, context=0x%lx\n", i, timer_ctrl.timer[i].is_used, timer_ctrl.timer[i].is_paused,
	            timer_ctrl.timer[i].is_cycle, timer_ctrl.timer[i].elapse, timer_ctrl.timer[i].interval, timer_ctrl.timer[i].context);
    }

    return;
}

void mtos_timer_suspend(void)
{
    (void)mtos_sem_take(&(timer_ctrl.sem_id), 0);
    timer_ctrl.is_suspend = TRUE;
    (void)mtos_sem_give(&(timer_ctrl.sem_id));
}

void mtos_timer_resume(void)
{
  u16 i  = 0;
  (void)mtos_sem_take(&(timer_ctrl.sem_id), 0);
  for (i = 0; i < TIMER_MAX_NUM; i++)
  {
	  if((timer_ctrl.timer[i].is_used == FALSE)
     || (timer_ctrl.timer[i].is_paused))
     {
				    continue;
		 }
	   timer_ctrl.timer[i].elapse = 0;
   }
  timer_ctrl.is_suspend = FALSE;
  (void)mtos_sem_give(&(timer_ctrl.sem_id));
}

