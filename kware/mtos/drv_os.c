/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/reboot.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>

#include "mt_cmdline.h"

#include "drv_os.h"
#include "mt_type.h"
#include "mtos_mem.h"
#include "mt_common.h"

#if 0 //debug
#define dprintf printf
#else
#define dprintf(...) \
    do {             \
    } while (0);
#endif

#define eprintf					printf

#ifndef MTOS_BUG_ON
#define MTOS_BUG_ON(expr, err)	do {				\
									if (expr) {		\
										eprintf("[BUG]%s@%d: fatal error(%d)!\n",__FUNCTION__,__LINE__,err);	\
										abort();	\
									}				\
								} while(0)
#endif

//#define MTOS_BUG_RS_MSG

#define MAX_SEM_NUM (256)
#define MSG_TYPE (1)
typedef long msgLong;
#define MSG_TYPE_SIZE sizeof(msgLong)

#ifdef MTOS_BUG_RS_MSG 
/***system_msg_type + mtos_msg_size + debug_crc + debug_mark****/
#define MSG_BUFFER_SIZE(mtMsgSize)  (MSG_TYPE_SIZE + mtMsgSize + sizeof(long) + sizeof(long))
#else 
/**system_msg_type + mtos_msg_size***/
#define MSG_BUFFER_SIZE(mtMsgSize)  (MSG_TYPE_SIZE + mtMsgSize) 
#endif
typedef struct tagOsQueue
{
    S32 msgId;
    U32 msgSize;
    U32 msgCount;
    sem_t sem;

    //pthread_mutex_t snd_mutex;
    //pthread_mutex_t rcv_mutex;
    pthread_mutex_t snd_rcv_mutex;
    U32 msgBufferSize;
    U8 *msgSndBuffer; /* msg type(long) + msg text, msg type must > 0 */
    U8 *msgRcvBuffer; /* msg type(long) + msg text, msg type must > 0 */
} OsQueue;

typedef struct tagOsSem
{
    U32 used;
    sem_t sem;
#ifdef DRV_SEM_DEBUG
    U32 init_value;
    U8 name[64];
    U32 value;
    int get_value;
#endif
} OsSem;

typedef struct
{
    U8 m_TaskName[32];
    U32 m_Inuse;
    U32 m_TaskID;
    pthread_t m_TaskHd;
    U32 m_Priority;
    pid_t m_Pid;
    pid_t m_Tid;
    pthread_cond_t m_TaskCond; //in order to realize task suspend & resume
    pthread_mutex_t m_TaskMutex;
    pthread_attr_t attr;
    int stacksize;
    char *stack_addr;
    void (*Fun)(void*);
    void *parm;
    U64 cputime;
    SysTaskState_t state;
} SysTask_t;

static OsQueue queueMgr[MAX_SYSMSGQNUM];
static OsSem semMgr[MAX_SEM_NUM];
//message global mutex
static pthread_mutex_t msg_mutex = PTHREAD_MUTEX_INITIALIZER;

static SysTask_t g_SysTask[MAX_SYSTASKNUM];
static pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;
static S8 os_version[22] = MT_LINUX_KERNEL_VERSION;

#ifdef CONFIG_MTOS_TASK_SCHED_FIFO
static int sys_policy = SCHED_FIFO;
#elif defined(CONFIG_MTOS_TASK_SCHED_OTHER)
static int sys_policy = SCHED_OTHER;
#else
    /*default SCHED_RR for Lotus*/
static int sys_policy = SCHED_RR;
#endif

//ErrorCode_t SYS_SemCreate ( U32 InitValue, U32 *SemID );

/*******************************************************************************
** Queue Message Structure used by the OSP.
** Note that it was decided to use a structure for the message type so as to
** provide bounds checking on the message being placed in the queue. An array
** was the obvious alternative to such a structure, but it was felt that the
** array would not clearly indicate to the user the the limitations on the
** message size to use.
*******************************************************************************/
typedef struct
{
    long q1stWordOfMsg; /* First word of a queue message.  */
    U32 q2ndWordOfMsg;  /* Second word of a queue message. */
    U32 q3rdWordOfMsg;  /* Third word of a queue message.  */
    U32 q4thWordOfMsg;  /* Fourth word of a queue message. */
    U32 q5thWordOfMsg;
} IPANEL_QUEUE_MESSAGE;

static void _printf_msgQInfo(void);

#ifdef MTOS_BUG_RS_MSG
static const unsigned int msg_crc32_ccitt_table[256] = 
{
  0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 
  0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005, 
  0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 
  0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 
  0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 
  0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75, 
  0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 
  0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd, 
  0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 
  0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 
  0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 
  0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d, 
  0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 
  0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95, 
  0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 
  0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 
  0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 
  0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072, 
  0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 
  0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca, 
  0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 
  0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 
  0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 
  0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba, 
  0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 
  0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692, 
  0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 
  0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 
  0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 
  0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2, 
  0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 
  0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a, 
  0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 
  0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 
  0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 
  0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53, 
  0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 
  0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b, 
  0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 
  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 
  0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 
  0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b, 
  0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 
  0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3, 
  0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 
  0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 
  0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 
  0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3, 
  0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 
  0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c, 
  0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 
  0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 
  0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 
  0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec, 
  0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 
  0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654, 
  0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 
  0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 
  0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 
  0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4, 
  0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 
  0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c, 
  0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668, 
  0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
};

static long msg_crc_calculate(u8 *p_buf,  long len)
{
  int i = 0;
  long crc = 0xFFFFFFFF;
  for(i = 0; i < len; i++)
  {
    crc = (msg_crc32_ccitt_table[((crc >> 24) & 0xff) ^ p_buf[i]] ^ (crc << 8)) 
          & 0xffffffff;
  }
  return crc;
}
#endif

/* uninterruptable msleep */
void SYS_TaskDelay(U32 DelayTime/*ms*/)
{
#ifdef CONFIG_MT_USE_MSLEEP
   (void)MT_USLEEP(1000 * DelayTime);
#else
   struct timespec req = {0};

    req.tv_sec  =  DelayTime / 1000;
    req.tv_nsec = (DelayTime % 1000) * 1000000;

    while (nanosleep(&req, &req) < 0 && errno == EINTR){
	  ;//printf("SYS_TaskDelay --EINTR----------------------\n");
    }
#endif
}

/* tp2 - tp1 */
static long diff_time_ms(struct timespec tp1, struct timespec tp2)
{
	if ((tp2.tv_sec > tp1.tv_sec)
		|| ((tp2.tv_sec == tp1.tv_sec) && (tp2.tv_nsec >= tp1.tv_nsec)))
	{
		return (long)(((tp2.tv_sec - tp1.tv_sec) * 1000)
					  + (tp2.tv_nsec/1000000)
					  - (tp1.tv_nsec/1000000));
	}
	else
	{
		//FIXME: overflow! treat tp1 as zero!
		eprintf("[W]%s: t1(%lu, %ld), t2(%lu, %ld) overflow!\n",__FUNCTION__,
			tp1.tv_sec, tp1.tv_nsec,
			tp2.tv_sec, tp2.tv_nsec);
		return (long)((tp2.tv_sec * 1000)
					  + (tp2.tv_nsec/1000000));
	}
}

/**
 * @brief uninterruptable sem_timedwait
 *
 * @retval
 *   0: success
 *   -1: timeout
 */
static int SYS_SemTimedwait(sem_t *p_sem,u32 mSec)
{
  int ret = -1;
  u32 timeCnt = 0;
  struct timespec ts = {0};
  struct timespec tp = {0};
  int ret_ts;

  timeCnt = 0;

  ret_ts = clock_gettime(CLOCK_MONOTONIC, &ts);

  do {
    /* abs_timeout might has problem for sem_timedwait while time changed */
    ret = sem_trywait(p_sem);
    if (ret != 0)
    {
      SYS_TaskDelay(1);

	  /* SYS_TaskDelay is not precise */
	  if (ret_ts == 0
	      && (clock_gettime(CLOCK_MONOTONIC, &tp) == 0))
	  {
		if (diff_time_ms(ts, tp) >= mSec)
		{
		  dprintf("%s: clock timeout(%u)\n",__FUNCTION__,mSec);
		  return -1;
		}
	  }

      timeCnt ++;
      if(timeCnt >= mSec)
      {
        dprintf("%s: timeout(%u)\n",__FUNCTION__,mSec);
        return -1;
      }
    }
  }while(ret != 0);

  return 0;
}

/* ??? */
static U64 time__sec_offset = 0;
void SYS_SetCurrTime(U64 sec)
{
	int ret;
    struct timeval tv;

    //struct tm *t_m;
    //time_t t;
    //char *s;

    ret = gettimeofday(&tv, 0);
    MTOS_BUG_ON((ret != 0), errno);
    if (ret != 0)
    	return;

	//once!
    if (time__sec_offset == 0) {
    	//FIXME: how sec < tv.tv_sec ?
		time__sec_offset = (U64)(sec - (U64)tv.tv_sec);
		dprintf("%s: %u, %llu, offset %llu\n",__FUNCTION__,
			tv.tv_sec, sec, time__sec_offset);
		tv.tv_sec = (time_t)sec;
		tv.tv_usec = 0;
		ret = settimeofday(&tv, 0);
		MTOS_BUG_ON((ret != 0), errno);
	    if (ret != 0)
	    {
	    	return;
	    }

		//t = time(NULL);
		//t_m = gmtime(&t);
		//s = asctime(t_m);
    }
}

void SYS_GetSysTime(U64 *p_sec, U32 *p_msec, U32 *p_usec)
{
	int ret;
    struct timeval tv;

    ret = gettimeofday(&tv, 0);
	MTOS_BUG_ON((ret != 0), errno);
    if (ret != 0)
    	return;

    if (p_sec != NULL) {
	  (*p_sec) = (U64)tv.tv_sec;
    }

    if (p_msec != NULL) {
	  (*p_msec) = (U32)(tv.tv_usec / 1000);
    }

    if (p_usec != NULL) {
	  (*p_usec) = (U32)(tv.tv_usec % 1000);
    }

    return;
}

/* 返回系统当前时间 ( UTC time in MS )*/
U64 SYS_GetMS(void)
{
	int ret;
    struct timeval tv;

    ret = gettimeofday(&tv, 0);
	MTOS_BUG_ON((ret != 0), errno);
    if (ret != 0)
    	return 0;

    return ((U64)tv.tv_sec - time__sec_offset) * 1000 + (U64)tv.tv_usec / 1000;
}

void SYS_DelayUS(U32 u32Us)
{
    MT_USLEEP(u32Us);
}

void SYS_Reset(void)
{
    reboot(0x01234567);
    //	SystemAsh ( "exec /sbin/reboot -f" );
}

U32 g_nSYSRun = 0xFFFFFFFF;
void SYS_Run(void)
{
    //set main task the lowest priority?
    g_nSYSRun = 0xFFFFFFFF;
    while (g_nSYSRun) {
	  SYS_TaskDelay(1000);
    }
}

void SYS_Exit(void)
{
    g_nSYSRun = 0;
}

const S8 *SYS_OS_GetVersion(void)
{
    return os_version;
}

static ulong s_hTaskLock = 0;
ErrorCode_t SYS_OS_Init(void)
{
    ErrorCode_t tErrorCode = ERROR_CODE_NO_ERROR;
    int i;

    for (i = 0; i < MAX_SYSTASKNUM; i++) {
	  g_SysTask[i].m_Inuse = 0;
    }

    memset(semMgr, 0, (sizeof(OsSem) * MAX_SEM_NUM));

#ifdef DRV_SEM_DEBUG
    tErrorCode = SYS_SemCreate(1, &s_hTaskLock, (const U8 *)"SYS_OS_Init:s_hTaskLock");
#else
    tErrorCode = SYS_SemCreate(1, &s_hTaskLock);
#endif

    return tErrorCode;
}

/*!
          Task related here!
*/
ErrorCode_t SYS_GetCurTaskID(U32 *pTask_ID)
{
	int ret;
    U32 cnt;
    pthread_t TaskHD;

    if (pTask_ID == NULL) {
	  return ERROR_CODE_ERROR_PARM;
    }

    ret = pthread_mutex_lock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);
    if (ret != 0)
    	return ERROR_CODE_ERROR_RESULT;

    TaskHD = pthread_self();

    for (cnt = 0; cnt < MAX_SYSTASKNUM; cnt++) {
	  if (g_SysTask[cnt].m_Inuse && pthread_equal(TaskHD, g_SysTask[cnt].m_TaskHd)) {
	    break;
	  }
    }

    if (MAX_SYSTASKNUM == cnt) {
	  *pTask_ID = MAX_SYSTASKNUM;
    } else {
	  *pTask_ID = g_SysTask[cnt].m_TaskID;
    }

    ret = pthread_mutex_unlock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    return ERROR_CODE_NO_ERROR;
}

ErrorCode_t SYS_TaskGetState(U32 Task_ID, SysTaskState_t *pState)
{
	int ret;

    if ((pState == NULL) || (Task_ID >= MAX_SYSTASKNUM) || (g_SysTask[Task_ID].m_Inuse == 0)) {
	  return ERROR_CODE_ERROR_PARM;
    }

    ret = pthread_mutex_lock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);
    if (ret != 0)
    	return ERROR_CODE_ERROR_RESULT;

    (*pState) = g_SysTask[Task_ID].state;

    ret = pthread_mutex_unlock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    return ERROR_CODE_NO_ERROR;
}

#ifdef DRV_SEM_DEBUG
static int _find_sem_index(OsSem *p_sem)
{
    int i = 0;

    for (i = 0; i < MAX_SEM_NUM; i++) {
	if (p_sem == &semMgr[i])
	    break;
    }

    if (MAX_SEM_NUM == i)
	i = -1;

    return i;
}

void SYS_SemDebug()
{
    int i = 0;

    for (i = 0; i < MAX_SEM_NUM; i++) {
	int value = -1;
	SYS_SemGetValue((ulong) & semMgr[i], &value);
	dprintf("Sem[%d]: used=%d, init_value=%d, value=%d, get_value=%d, name=%s, ", i,
	        semMgr[i].used, semMgr[i].init_value, semMgr[i].value, value, semMgr[i].name);

	if (0 == semMgr[i].init_value) {
	    if (semMgr[i].value > 0) {
		dprintf("HOLD!\n");
	    }
	}

	if (1 == semMgr[i].init_value) {
	    if (semMgr[i].value > 1) {
		dprintf("HOLD!\n");
	    }
	}

	dprintf("OK\n");
    }

    return;
}
ErrorCode_t SYS_SemCreate(U32 InitValue, ulong *SemID, const U8 *Name) //注意用的时候把信号量设为全局的这样多个线程都可以用
#else
ErrorCode_t SYS_SemCreate(U32 InitValue, ulong *SemID) //注意用的时候把信号量设为全局的这样多个线程都可以用
#endif
{
	int ret;
    S32 index = 0;
    sem_t sem;

    if ((ret=sem_init(&sem, 0, InitValue)) != 0) {
	  MTOS_BUG_ON((ret != 0), errno);
	  return ERROR_CODE_ERROR_RESULT;
    }

    for (index = 0; index < MAX_SEM_NUM; index++) {
	  if (semMgr[index].used == 0) {
#ifdef DRV_SEM_DEBUG
	    int value = -1;
	    SYS_SemGetValue((ulong) & semMgr[index], &value);
#endif
	    semMgr[index].used = 1;
	    semMgr[index].sem = sem;
#ifdef DRV_SEM_DEBUG
	    semMgr[index].init_value = InitValue;
	    semMgr[index].value = 0;
	    semMgr[index].get_value = value;
	    strncpy(semMgr[index].name, Name, sizeof(semMgr[index].name)-1);
	    semMgr[index].name[sizeof(semMgr[index].name)-1] = '\0';
	    dprintf("\033[35m %s<%d> index=%d, name=%s, sem=0x%lx\033[0m\n", __FUNCTION__, __LINE__,
	            index, Name, (ulong) & semMgr[index]);
#endif
	    *SemID = (ulong) & semMgr[index];
	    return ERROR_CODE_NO_ERROR;
	  }
    }

    eprintf("SYS_Sem_Create failed, sem create too much \n");
    (*SemID) = 0;

    ret = sem_destroy(&sem);
	MTOS_BUG_ON((ret != 0), errno);

    return ERROR_CODE_ERROR_RESULT;
}

ErrorCode_t SYS_SemGetValue(ulong SemID, int *value)
{
    int ret = 0;
    OsSem *pSem = NULL;

    pSem = (OsSem *)SemID;

    if (pSem == NULL) {
	  return ERROR_CODE_ERROR_PARM;
    }

    ret = sem_getvalue(&pSem->sem, value);
	MTOS_BUG_ON((ret != 0), errno);
    if (ret != 0) {
	  return ERROR_CODE_ERROR_RESULT;
    }

    return ERROR_CODE_NO_ERROR;
}

ErrorCode_t SYS_SemWait(ulong SemID, S32 milliSecsToWait)
{
#ifdef DRV_SEM_DEBUG
    int index = 0;
#endif
    int ret;
    OsSem *pSem = (OsSem *)SemID;

    if (pSem == NULL) {
	  eprintf("SYS_Sem_wait failed ,SemID==NULL \n");
	  return ERROR_CODE_ERROR_PARM;
    }

#ifdef DRV_SEM_DEBUG
    index = _find_sem_index(pSem);
    semMgr[index].value++;
#endif

    if (0 == milliSecsToWait) {
	//     printf("----%s----%d---\n",__FUNCTION__,__LINE__);
	  if (sem_trywait(&pSem->sem) == 0) {
	    return ERROR_CODE_NO_ERROR;
	  }
    } else if (-1 == milliSecsToWait) {
	  //     printf("----%s----%d---\n",__FUNCTION__,__LINE__);
	  //ret = sem_wait(&pSem->sem);
	  while ((ret = sem_wait(&pSem->sem)) != 0 && (errno == EINTR))
	  {
	  	continue;
	  }
	  //     printf("----%s----%d---\n ret:%d",__FUNCTION__,__LINE__,ret);
	  MTOS_BUG_ON((ret != 0), errno);
	  if (ret == 0) {
	    return ERROR_CODE_NO_ERROR;
	  }
    } else if (milliSecsToWait > 0) {

       ret = SYS_SemTimedwait(&pSem->sem, (u32)milliSecsToWait);
       if (ret == 0)
       {
          return ERROR_CODE_NO_ERROR;
       }

    }
    //   printf("----%s----%d---\n",__FUNCTION__,__LINE__);

    if (errno != ETIMEDOUT) {
	  if (errno == EDEADLK) {
	    eprintf("SYS_Sem_wait sem usage FATAL: Dead Lock milliSecsToWait=%d\n", milliSecsToWait);
	  }
    }

    //  printf("----%s----%d---\n",__FUNCTION__,__LINE__);

    return ERROR_CODE_ERROR_PARM;
}

ErrorCode_t SYS_SemSend(ulong SemID)
{
    OsSem *pSem = (OsSem *)SemID;

    if (pSem) {
	  if (sem_post(&pSem->sem) == 0) {
#ifdef DRV_SEM_DEBUG
	    int index = _find_sem_index(pSem);
	    semMgr[index].value--;
#endif
	    return ERROR_CODE_NO_ERROR;
	  } else {
		  MTOS_BUG_ON(1, errno);
	  }
    }

    return ERROR_CODE_ERROR_PARM;
}

ErrorCode_t SYS_SemDel(ulong SemID)
{
    OsSem *pSem = (OsSem *)SemID;

    dprintf("SYS_Sem_destroy sem_handle=0x%x \n", SemID);

    if (pSem) {
	  if (sem_destroy(&pSem->sem) == 0) {
	    pSem->used = 0;
	    memset(&pSem->sem, 0, sizeof(sem_t));
	    return ERROR_CODE_NO_ERROR;
	  } else {
		  MTOS_BUG_ON(1, errno);
	  }
    }

    eprintf("SYS_Sem_destroy failed, sem_handle=0x%lx \n", SemID);

    return ERROR_CODE_ERROR_PARM;
}

ErrorCode_t SYS_SemInfo(U32 flag)
{
    return ERROR_CODE_NO_ERROR;
}

ErrorCode_t SYS_MsgQDump(U32 MsgID)
{
    return 0;
}

static void _printf_msgQInfo(void)
{
    /*
	int i;
	dprintf("\n~~~~~~~~MsgQ info~~~~~~~~~~~\n");
	dprintf("队列ID:\n");
	for(i = 0;i<MAX_SYSMSGQNUM;i++)
	{
		dprintf("%d,",queueMgr[i].msgId);
	}
	dprintf("\n");
*/
}

ErrorCode_t SYS_MsgQInit(void)
{
    U32 index = 0;

	(void)pthread_mutex_lock(&msg_mutex);

    for (; index < MAX_SYSMSGQNUM; index++) {
	  queueMgr[index].msgId = -1;
    }

	(void)pthread_mutex_unlock(&msg_mutex);

    _printf_msgQInfo();
    return 0;
}

void SYS_MsgQList(void)
{
    _printf_msgQInfo();
}

ErrorCode_t SYS_MsgQCreate(
    U32 MsgSize,
    U32 MsgCount,
    U32 *pMsgID)
{
    size_t msgBuferLen = 0;
    S32 msgId = 0, index = 0;
    //char *name = "msg";

	if (MsgSize <= 0 || MsgCount <= 0 || pMsgID == NULL)
		return ERROR_CODE_ERROR_PARM;

	(void)pthread_mutex_lock(&msg_mutex);

    if ((msgId = msgget(IPC_PRIVATE, IPC_CREAT | 0660)) == -1) {
	  (void)pthread_mutex_unlock(&msg_mutex);
	  eprintf("[%s] msgget failed \n", __FUNCTION__);
	  eprintf("SYS_MsgQCreate error ==%d\n", errno);
	  //MTOS_BUG_ON(1, errno);
	  return ERROR_CODE_ERROR_PARM;
    }

    _printf_msgQInfo();
    for (index = 0; index < MAX_SYSMSGQNUM; index++) {
	if (queueMgr[index].msgId == -1) {
      msgBuferLen = MSG_BUFFER_SIZE(MsgSize);
      queueMgr[index].msgBufferSize = msgBuferLen;
	    queueMgr[index].msgSndBuffer = (U8 *)mtos_malloc(msgBuferLen);
	    queueMgr[index].msgRcvBuffer = (U8 *)mtos_malloc(msgBuferLen);
	    if (queueMgr[index].msgSndBuffer != NULL && queueMgr[index].msgRcvBuffer != NULL) {
		  long *msgType = NULL;

		  msgType = (long *)queueMgr[index].msgSndBuffer;
		  msgType[0] = (long)MSG_TYPE;

		  queueMgr[index].msgId = msgId;

      	  if (sem_init(&queueMgr[index].sem, 0, 0) != 0) {
      		eprintf("SYS_MsgQCreate sem_init failed!!!!!!!!!!!\n");
            mtos_free(queueMgr[index].msgSndBuffer);
            mtos_free(queueMgr[index].msgRcvBuffer);
            queueMgr[index].msgId = -1;

			MTOS_BUG_ON(1, errno);

			if (msgctl(msgId, IPC_RMID, NULL) != 0)
			{
				eprintf("SYS_MsgQCreate msgctl(IPC_RMID) failed errno(%d)!\n",errno);
				MTOS_BUG_ON(1, errno);
			}

			(void)pthread_mutex_unlock(&msg_mutex);
      		return ERROR_CODE_ERROR_RESULT;
      	  }

		//(void)pthread_mutex_init(&queueMgr[index].snd_mutex, NULL);
		//(void)pthread_mutex_init(&queueMgr[index].rcv_mutex, NULL);
		(void)pthread_mutex_init(&queueMgr[index].snd_rcv_mutex, NULL);

		queueMgr[index].msgSize = MsgSize;
		queueMgr[index].msgCount = MsgCount;
		*pMsgID = (U32)msgId;
		(void)pthread_mutex_unlock(&msg_mutex);
		dprintf("\033[35m %s<%d> -> (MsgID: %d)\033[0m\n", __FUNCTION__, __LINE__,
		        *pMsgID);
		_printf_msgQInfo();
		return ERROR_CODE_NO_ERROR;
	    }
	  }
    }

	if (msgctl(msgId, IPC_RMID, NULL) != 0)
	{
		eprintf("SYS_MsgQCreate msgctl(IPC_RMID) failed errno(%d)!!!\n",errno);
		MTOS_BUG_ON(1, errno);
	}

	(void)pthread_mutex_unlock(&msg_mutex);

    eprintf("SYS_MsgQCreate failed, queue create too much \n");

    return ERROR_CODE_ERROR_PARM;
}
#ifdef MTOS_BUG_RS_MSG
static int SYS_CheckMsgRecieveData(OsQueue *p_msgQue)
{
  long overflag = 0xa55aa55a;
  long crcOffset = 0;
  long msgCrc = 0;
  long msgCheckCrc = 0;
  long msgType = 0;
  int check_ret = TRUE;
  memcpy((void *)&msgType,(void *)p_msgQue->msgRcvBuffer,sizeof(msgType));
  msgCrc = msg_crc_calculate((u8 *)(p_msgQue->msgRcvBuffer + MSG_TYPE_SIZE),(long)p_msgQue->msgSize);
  crcOffset = p_msgQue->msgBufferSize - sizeof(msgCrc) - sizeof(overflag);
  memcpy((void *)&msgCheckCrc,(void *)(p_msgQue->msgRcvBuffer + crcOffset),sizeof(msgCheckCrc));
  crcOffset = p_msgQue->msgBufferSize - sizeof(overflag);
  memcpy((void *)&overflag,(void *)(p_msgQue->msgRcvBuffer + crcOffset),sizeof(overflag));
  if(overflag != (long)0xa55aa55a)
  {
   printf("\033[35m [%s] msgrcv overflag Error:[0x%lx]!= [0x%lx]\033[0m\n",__FUNCTION__,overflag,(long)0xa55aa55a);
   check_ret = FALSE;
  }
  if(msgCrc != msgCheckCrc)
  {
   printf("\033[35m [%s] msgrcv crc Error:[0x%lx]!= [0x%lx]\033[0m\n",__FUNCTION__,msgCrc,msgCheckCrc);
   check_ret = FALSE;
  }
  if(msgType != (long)MSG_TYPE)
  {
   printf("\033[35m [%s] msgrcv type Error:[0x%lx]!= [0x%lx]\033[0m\n",__FUNCTION__,msgType,(long)MSG_TYPE);
   check_ret = FALSE;
  }

  return check_ret;
}
#endif
ErrorCode_t SYS_MsgQWait(
						void *RecMsg,
						U32 MsgID,
						U32 WaitTimeMS)
{
	size_t msgLen = 0;
	S32 msgflg = 0;
	S32 index = 0;
	U8 *msgRcvBuffer = NULL;
	ErrorCode_t ret_val = ERROR_CODE_ERROR_PARM;

	if (MsgID== (-1) || RecMsg == NULL)
		return ERROR_CODE_ERROR_PARM;

	(void)pthread_mutex_lock(&msg_mutex);

	for (index = 0; index < MAX_SYSMSGQNUM; index++)
	{
		if (queueMgr[index].msgId == MsgID)
		{
			break;
		}
	}

	if (index >= MAX_SYSMSGQNUM)
	{
		(void)pthread_mutex_unlock(&msg_mutex);
		_printf_msgQInfo();
		eprintf("创建的队列中找不到这个ID(%d)\n", MsgID);
		return ERROR_CODE_ERROR_PARM;
	}

	//dprintf("\n\n%s:%d->%s(waitID=%d)\n\n",__FILE__,__LINE__,__FUNCTION__,MsgID);
	memset((void *)queueMgr[index].msgRcvBuffer,0,queueMgr[index].msgBufferSize);
  msgLen = queueMgr[index].msgBufferSize - MSG_TYPE_SIZE;
#ifdef MTOS_BUG_RS_MSG
    {   
      long overflag = 0xa55aa55a;
      u32 flagOffset = 0;
      flagOffset = queueMgr[index].msgBufferSize - sizeof(overflag);
      memcpy((void *)(queueMgr[index].msgRcvBuffer + flagOffset),(void *)&overflag,sizeof(overflag));

      #if 0
      /***test error recieve size***/
      //msgLen = queueMgr[index].msgBufferSize - sizeof(overflag); /**msgtype + mtosmsg + crc***/
      #else
      /***test correct send size***/
      msgLen = queueMgr[index].msgBufferSize - MSG_TYPE_SIZE - sizeof(overflag); /**mtosmsg + crc***/
      #endif
    }
#endif

	(void)pthread_mutex_unlock(&msg_mutex);

	if (WaitTimeMS == SYS_TIMEOUT_IMMEDIATE)
	{
		msgflg = IPC_NOWAIT;

		//FIXME: queueMgr[index] might be destroied!
		if (sem_trywait(&queueMgr[index].sem) != 0)
		{
			ret_val = ERROR_CODE_ERROR_RESULT;
			goto RETURN;
		}

		do
		{
			//FIXME: queueMgr[index] might be destroied during this loop!
			(void)pthread_mutex_lock(&msg_mutex);

			//Bug Detect
			if (queueMgr[index].msgId == -1
				|| queueMgr[index].msgRcvBuffer == NULL)
			{
			  eprintf("[F]%s: index %d, msgId %d, msgRcvBuffer %p\n", __FUNCTION__,
				  index,
				  queueMgr[index].msgId,
				  queueMgr[index].msgRcvBuffer);
			  MTOS_BUG_ON(1, errno);
			}
			(void)pthread_mutex_unlock(&msg_mutex);

			(void)pthread_mutex_lock(&queueMgr[index].snd_rcv_mutex);

			if (msgrcv((int)queueMgr[index].msgId, (void *)queueMgr[index].msgRcvBuffer, msgLen, MSG_TYPE, msgflg) > 0)
			{
			#ifdef MTOS_BUG_RS_MSG
			  SYS_CheckMsgRecieveData(&queueMgr[index]);
      #endif
				memcpy((void *)RecMsg, (void *)(queueMgr[index].msgRcvBuffer + MSG_TYPE_SIZE), queueMgr[index].msgSize);
				ret_val = ERROR_CODE_NO_ERROR;

				(void)pthread_mutex_unlock(&queueMgr[index].snd_rcv_mutex);
				goto RETURN;
			}

			(void)pthread_mutex_unlock(&queueMgr[index].snd_rcv_mutex);

		} while (errno == EINTR);

		//dprintf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@rcv error\n");
		ret_val =  ERROR_CODE_ERROR_RESULT;
	}
	else if (WaitTimeMS == SYS_TIMEOUT_INFINITY)
	{
		int ret;

		//FIXME: queueMgr[index] might be destroied!
		while (((ret=sem_wait(&queueMgr[index].sem)) != 0) && (errno == EINTR))
		{
			//MT_USLEEP(20000);
			continue;
		}

		if (ret != 0)
		{
			eprintf("%s: sem_wait fail, errno=%d\n",__FUNCTION__,errno);
			//MTOS_BUG_ON(1, errno);
			ret_val =  ERROR_CODE_ERROR_RESULT;
			goto RETURN;
		}

		//alloc local buffer to avoid multi thread issue
		msgRcvBuffer = (U8 *)mtos_malloc(queueMgr[index].msgBufferSize);
		if (msgRcvBuffer == NULL)
		{
			eprintf("MsgID(%d): oom!\n", MsgID);
			ret_val = ERROR_CODE_ERROR_MEM;
			goto RETURN;
		}

		for (;;)
		{
			//FIXME: queueMgr[index] might be destroied during this loop!
			(void)pthread_mutex_lock(&msg_mutex);

			//Bug Detect
			if (queueMgr[index].msgId == -1
				|| queueMgr[index].msgRcvBuffer == NULL)
			{
			  eprintf("[F]%s: index %d, msgId %d, msgRcvBuffer %p\n", __FUNCTION__,
				  index,
				  queueMgr[index].msgId,
				  queueMgr[index].msgRcvBuffer);
			  MTOS_BUG_ON(1, errno);
			}
			(void)pthread_mutex_unlock(&msg_mutex);

			msgflg = 0;
			(void)pthread_mutex_lock(&queueMgr[index].snd_rcv_mutex);
			if (msgrcv((int)queueMgr[index].msgId, (void *)msgRcvBuffer, msgLen, MSG_TYPE, msgflg) > 0)
			{
				memcpy((void *)RecMsg, (void *)(msgRcvBuffer + MSG_TYPE_SIZE), queueMgr[index].msgSize);
				ret_val = ERROR_CODE_NO_ERROR;

				(void)pthread_mutex_unlock(&queueMgr[index].snd_rcv_mutex);

				goto RETURN;
			}
			else
			{
				(void)pthread_mutex_unlock(&queueMgr[index].snd_rcv_mutex);

				dprintf("\033[35m %s<%d> msgrcv Error(%d:%s)\033[0m\n", __FUNCTION__, __LINE__,
						errno, strerror(errno));
				if (EINTR == errno)
				{
					continue;
				}
				else
				{
					if (ENOMSG != errno)
					{
						//MTOS_BUG_ON(1, errno);
					}
					break;
				}
			}
		}

		ret_val =  ERROR_CODE_ERROR_RESULT;
	}
	else
	{
		int ret = 0;

		//FIXME: queueMgr[index] might be destroied!
		ret = SYS_SemTimedwait(&queueMgr[index].sem, WaitTimeMS);
		//printf("sem_timedwait   %d\n",ret);
		msgflg = IPC_NOWAIT;

		if (ret == 0)
		{
			//MT_USLEEP(10000);
			do
			{
				//FIXME: queueMgr[index] might be destroied during this loop!
				(void)pthread_mutex_lock(&msg_mutex);

				//Bug Detect
				if (queueMgr[index].msgId == -1
					|| queueMgr[index].msgRcvBuffer == NULL)
				{
				  eprintf("[F]%s: index %d, msgId %d, msgRcvBuffer %p\n", __FUNCTION__,
					  index,
					  queueMgr[index].msgId,
					  queueMgr[index].msgRcvBuffer);
				  MTOS_BUG_ON(1, errno);
				}
				(void)pthread_mutex_unlock(&msg_mutex);

				(void)pthread_mutex_lock(&queueMgr[index].snd_rcv_mutex);

				if (msgrcv((int)queueMgr[index].msgId, (void *)queueMgr[index].msgRcvBuffer, msgLen, MSG_TYPE, msgflg) > 0)
				{
				#ifdef MTOS_BUG_RS_MSG
				  SYS_CheckMsgRecieveData(&queueMgr[index]);
        #endif
					memcpy((void *)RecMsg, (void *)(queueMgr[index].msgRcvBuffer + MSG_TYPE_SIZE), queueMgr[index].msgSize);
					ret_val = ERROR_CODE_NO_ERROR;

					(void)pthread_mutex_unlock(&queueMgr[index].snd_rcv_mutex);
					goto RETURN;
				}

				(void)pthread_mutex_unlock(&queueMgr[index].snd_rcv_mutex);

			} while (errno == EINTR);
		}

		ret_val =  ERROR_CODE_ERROR_RESULT;
	}

RETURN:
	if (msgRcvBuffer != NULL)
	{
		mtos_free(msgRcvBuffer);
		msgRcvBuffer = NULL;
	}

	return ret_val;
}

ErrorCode_t SYS_MsgQSend(const void *SendMsg, U32 MsgID, U32 TimeoutMs)
{
    S32 index = 0;
    size_t msgLen = 0;
    msgLong msgType = 0;
    //struct msqid_ds buf = { 0 };

	if (MsgID == (-1) || SendMsg == NULL)
		return ERROR_CODE_ERROR_RESULT;

	(void)pthread_mutex_lock(&msg_mutex);
    for (index = 0; index < MAX_SYSMSGQNUM; index++) {
	  if (queueMgr[index].msgId == MsgID) {
	    break;
	  }
    }

    _printf_msgQInfo();
    if (index >= MAX_SYSMSGQNUM) {
	  (void)pthread_mutex_unlock(&msg_mutex);
	  eprintf("%s:%d->%s\n", __FILE__, __LINE__, __FUNCTION__);
	  return ERROR_CODE_ERROR_PARM;
    }

	(void)pthread_mutex_unlock(&msg_mutex);

    do {

	  //FIXME: queueMgr[index] might be destroied during this loop!
	  (void)pthread_mutex_lock(&msg_mutex);

	  //Bug Detect
	  if (queueMgr[index].msgId == -1
	  	  || queueMgr[index].msgSndBuffer == NULL)
	  {
	  	eprintf("[F]%s: index %d, msgId %d, msgSndBuffer %p\n", __FUNCTION__,
	  		index,
	  		queueMgr[index].msgId,
	  		queueMgr[index].msgSndBuffer);
	  	MTOS_BUG_ON(1, errno);
	  }
	  (void)pthread_mutex_unlock(&msg_mutex);

	  //Fix multi thread call SYS_MsgQSend issue
	  (void)pthread_mutex_lock(&queueMgr[index].snd_rcv_mutex);
    memset((void *)queueMgr[index].msgSndBuffer,0,queueMgr[index].msgBufferSize);
	  msgType = MSG_TYPE;
    memcpy((void *)queueMgr[index].msgSndBuffer,(void *)&msgType,sizeof(msgType));
	  memcpy((void *)(queueMgr[index].msgSndBuffer + sizeof(msgType)),(void *)SendMsg, queueMgr[index].msgSize);
    msgLen = queueMgr[index].msgBufferSize - MSG_TYPE_SIZE;
#ifdef MTOS_BUG_RS_MSG
    {   
        u32 debugOffset = 0;
        long overflag = 0x5aa55aa5;
        long msgCrc = msg_crc_calculate((u8 *)SendMsg,queueMgr[index].msgSize);
        debugOffset = queueMgr[index].msgBufferSize - sizeof(msgCrc) - sizeof(overflag);
        memcpy((void *)(queueMgr[index].msgSndBuffer + debugOffset),(void *)&msgCrc,sizeof(msgCrc));
        debugOffset = queueMgr[index].msgBufferSize - sizeof(overflag);
        memcpy((void *)(queueMgr[index].msgSndBuffer + debugOffset),(void *)&overflag,sizeof(overflag));
        #if 0 /***test error send size***/
        msgLen = queueMgr[index].msgBufferSize - sizeof(overflag); /***msgtype + mtosmsg + crc***/
        #else /***test correct send size***/
        msgLen = queueMgr[index].msgBufferSize - MSG_TYPE_SIZE - sizeof(overflag); /***mtosmsg + crc***/
        #endif
    }
#endif
      if (msgsnd((int)queueMgr[index].msgId, (const void *)queueMgr[index].msgSndBuffer, msgLen, 0) == 0) {

		(void)pthread_mutex_unlock(&queueMgr[index].snd_rcv_mutex);

		//FIXME: queueMgr[index] might be destroied!
	    while (sem_post(&queueMgr[index].sem) != 0) {
			eprintf("SYS_MsgQSend sem_post fail, errno %d\n", errno);
	        MT_USLEEP(1000);
	    }
	    return ERROR_CODE_NO_ERROR;
      }

	  (void)pthread_mutex_unlock(&queueMgr[index].snd_rcv_mutex);

    } while (errno == EINTR);

    eprintf("SYS_MsgQSend error ==%d\n", errno);

    return ERROR_CODE_ERROR_PARM;
}

ErrorCode_t SYS_MsgQInfo(U32 flag)
{
    _printf_msgQInfo();
    return ERROR_CODE_NO_ERROR;
}

ErrorCode_t SYS_MsgQQuery(U32 MsgID, U32 *MsgCnt)
{
    U16 index = 0;
    struct msqid_ds msg_info;

	if (MsgID == (-1))
		return ERROR_CODE_ERROR_RESULT;

    for (index = 0; index < MAX_SYSMSGQNUM; index++) {

	  (void)pthread_mutex_lock(&msg_mutex);

	  if (queueMgr[index].msgId == MsgID) {
	    memset(&msg_info, 0, sizeof(struct msqid_ds));
		//FIXME: lock queueMgr[index].mutex?
	    if (msgctl(queueMgr[index].msgId, IPC_STAT, &msg_info) == 0) {
		  if (MsgCnt != NULL) {
		    (*MsgCnt) = (U32)msg_info.msg_qnum;
		    dprintf("\033[35m%s:%d-> msg_count: %d\033[0m\n", __FUNCTION__, __LINE__, (*MsgCnt));
			(void)pthread_mutex_unlock(&msg_mutex);
		    return ERROR_CODE_NO_ERROR;
		  }
	    }
	    else
	    {
	    	eprintf("%s: msgctl(IPC_STAT) fail, errno=%d\n",__FUNCTION__,errno);
	    	//MTOS_BUG_ON(1, errno);
	    }
		(void)pthread_mutex_unlock(&msg_mutex);
	    break;
	  }

	  (void)pthread_mutex_unlock(&msg_mutex);
    }

    return ERROR_CODE_ERROR_RESULT;
}

ErrorCode_t SYS_MsgQDel(U32 MsgID)
{
    S32 index = 0;

	if (MsgID == (-1))
		return ERROR_CODE_ERROR_RESULT;

    _printf_msgQInfo();
    dprintf("删除一个消息队列(%d):", MsgID);

	(void)pthread_mutex_lock(&msg_mutex);

    for (index = 0; index < MAX_SYSMSGQNUM; index++) {

	  if (queueMgr[index].msgId == MsgID) {

		/* FIXME:
		     if this MSG is on sending or receiving?
		     lock queueMgr[index].mutex?
		 */

	    if (msgctl(queueMgr[index].msgId, IPC_RMID, NULL) == 0) {
		  queueMgr[index].msgId = -1;

		  if (sem_destroy(&queueMgr[index].sem) != 0)
		  {
	    	eprintf("%s: sem_destroy fail, errno=%d\n",__FUNCTION__,errno);
	    	MTOS_BUG_ON(1, errno);
		  }

		  //(void)pthread_mutex_destroy(&queueMgr[index].snd_mutex);
		  //(void)pthread_mutex_destroy(&queueMgr[index].rcv_mutex);
		  (void)pthread_mutex_destroy(&queueMgr[index].snd_rcv_mutex);

		  if (queueMgr[index].msgSndBuffer != NULL) {
		    mtos_free(queueMgr[index].msgSndBuffer);
		    queueMgr[index].msgSndBuffer = NULL;
		  }
		  if (queueMgr[index].msgRcvBuffer != NULL) {
		    mtos_free(queueMgr[index].msgRcvBuffer);
		    queueMgr[index].msgRcvBuffer = NULL;
		  }
		  dprintf("OK\n");
		  (void)pthread_mutex_unlock(&msg_mutex);
		  return ERROR_CODE_NO_ERROR;
	    }
	    else
	    {
	    	eprintf("%s: msgctl(IPC_RMID) fail, errno=%d\n",__FUNCTION__,errno);
	    	MTOS_BUG_ON(1, errno);
	    }
	    break;
	  }
    }
	(void)pthread_mutex_unlock(&msg_mutex);

    eprintf("SYS_MsgQDel error\n");
    _printf_msgQInfo();

    return ERROR_CODE_ERROR_PARM;
}

void SYS_TaskDebug()
{
    int i = 0;

    for (i = 0; i < MAX_SYSTASKNUM; i++) {
	dprintf("Task[%2d]: used=%d, id=%2d, hd=%8x, pri=%2d, name=%s\n", i,
	        g_SysTask[i].m_Inuse,
	        g_SysTask[i].m_TaskID,
	        g_SysTask[i].m_TaskHd,
	        g_SysTask[i].m_Priority,
	        g_SysTask[i].m_TaskName);
    }

    return;
}

void SYS_TaskSetSchedPolicy(int policy)
{
  sys_policy = policy;
}

/* pthread_create: task wrapper function */
static void *start_routine_function (SysTask_t *pTsk,char *p_name,int lwpid)
{
  	void (*mFun)(void*) = NULL;
	if (pTsk && pTsk->Fun)
	{
		mFun = pTsk->Fun;
    	printf("pthread task[%s] Lwpid[%d] start .....\n",p_name,lwpid);
		mFun(pTsk->parm);
	}

	return NULL;
}
//debug
/* pthread_create: task wrapper */
static void *start_routine_wrapper (void *arg)
{
	SysTask_t *pTsk = (SysTask_t*)arg;
  	int lwpId = 0;
	if(pTsk && pTsk->Fun)
	{
	  	lwpId = syscall(SYS_gettid);
		start_routine_function (pTsk,(char*)pTsk->m_TaskName,lwpId);
	}
	return NULL;
}


ErrorCode_t SYS_TaskCreate(void (*Function)(void *),
                           void *Param,
                           U32 StackSize,
                           U32 Priority,
                           U32 *Task_ID,
                           const U8 *Name,
                           void **ppStack)
{
	int ret;
    ErrorCode_t err = 0;
    U32 i;
    pthread_t tid;

	int schedpolicy;	/*SCHED_FIFO, SCHED_RR, and SCHED_OTHER*/
    struct sched_param sparam;

    //pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    //pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    if (Task_ID == NULL) {
	  return ERROR_CODE_ERROR_PARM;
    }

    if ((Function == NULL) ||
        (Name == NULL) || (StackSize == 0)) {
	  return ERROR_CODE_ERROR_PARM;
    }

    UNUSED_PARAMETER(ppStack);

    ret = pthread_mutex_lock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);
    if (ret != 0)
    	return ERROR_CODE_ERROR_RESULT;

    for (i = 1; i < MAX_SYSTASKNUM; i++) {
	  if (0 == g_SysTask[i].m_Inuse) {
	    break;
	  }
    }
    if (i == MAX_SYSTASKNUM) {
	  eprintf(" Task outof its MAXNUM. \n");
	  ret = pthread_mutex_unlock(&task_mutex);
	  MTOS_BUG_ON((ret != 0), ret);
	  return ERROR_CODE_ERROR_PARM;
    }

    (void)pthread_attr_init(&g_SysTask[i].attr);

#ifdef CONFIG_MTOS_TASK_SCHED_FIFO
	schedpolicy = SCHED_FIFO;
#elif defined(CONFIG_MTOS_TASK_SCHED_OTHER)
	schedpolicy = SCHED_OTHER;
#else
    /*default SCHED_RR for Lotus*/
	schedpolicy = SCHED_RR;
#endif

    if(schedpolicy != sys_policy)
    {
     schedpolicy = sys_policy;
    }

    if (Priority < sched_get_priority_min(schedpolicy)) {
	  Priority = (U32)sched_get_priority_min(schedpolicy);
    }

    if (Priority > sched_get_priority_max(schedpolicy)) {
	  Priority = (U32)sched_get_priority_max(schedpolicy);
    }

    pthread_attr_setschedpolicy(&g_SysTask[i].attr, schedpolicy);
    /*Linux supports PTHREAD_SCOPE_SYSTEM, but not PTHREAD_SCOPE_PROCESS.*/
    pthread_attr_setscope(&g_SysTask[i].attr, PTHREAD_SCOPE_SYSTEM);

    pthread_attr_getschedparam(&g_SysTask[i].attr, &sparam);
    sparam.sched_priority = (int)Priority;
    pthread_attr_setinheritsched(&g_SysTask[i].attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedparam(&g_SysTask[i].attr, &sparam);

    if (StackSize < 16 * 1024) {
	  StackSize = 16 * 1024;
    }

    if (StackSize > 1 * 1024 * 1024) {
	  StackSize = 1 * 1024 * 1024;
    }

    g_SysTask[i].stacksize = (int)StackSize;

	//Fix Bug 124359: it's NOT recommended to set stack size!
    //pthread_attr_setstacksize(&g_SysTask[i].attr, StackSize);
    //pthread_attr_setstack(&g_SysTask[i].attr, stackaddr, StackSize);

    g_SysTask[i].stack_addr = NULL;

#if 0
//    g_SysTask[i].stack_addr = malloc(StackSize);
   	pthread_attr_setstack(&g_SysTask[i].attr, NULL,1024*1024);
#endif

    /*分离线程*/
    pthread_attr_setdetachstate(&g_SysTask[i].attr, PTHREAD_CREATE_DETACHED);

	//TODO
	pthread_attr_setguardsize(&g_SysTask[i].attr, 4096/*PAGESIZE*/);

    g_SysTask[i].Fun = Function;
    g_SysTask[i].parm = Param;

    strncpy((char*)g_SysTask[i].m_TaskName, (const char*)Name, sizeof(g_SysTask[i].m_TaskName)-1);
    g_SysTask[i].m_TaskName[sizeof(g_SysTask[i].m_TaskName) - 1] = '\0';

    g_SysTask[i].m_Priority = Priority;
    g_SysTask[i].m_TaskID = i;
    g_SysTask[i].cputime = 0;
    ret = pthread_mutex_init(&(g_SysTask[i].m_TaskMutex), NULL);
	MTOS_BUG_ON((ret != 0), ret);

    ret = pthread_cond_init(&(g_SysTask[i].m_TaskCond), NULL);
	MTOS_BUG_ON((ret != 0), ret);

    //err = pthread_create(&tid, &g_SysTask[i].attr, (void *)Function, Param);
    err = (ErrorCode_t)pthread_create(&tid, &g_SysTask[i].attr, start_routine_wrapper, (void*)&g_SysTask[i]);
    //prctl(PR_SET_NAME, g_SysTask[i].m_TaskName, 0, 0, 0);

    if (err != 0) {
	  eprintf("pthread_create errno=%d\n", err);
	  g_SysTask[i].m_Inuse = 0;
	  *Task_ID = 0xffffffff;
	  ret = pthread_mutex_unlock(&task_mutex);
	  MTOS_BUG_ON((ret != 0), ret);
	  return err;
    }

    pthread_setname_np(tid, (const char*)g_SysTask[i].m_TaskName);

    printf("\033[35m %s<%d> -> (-------------name:%s, priority:%d, tid:%d, thd:0x%x, err:%d)\033[0m\n",
            __FUNCTION__, __LINE__, Name, Priority, (int)i, (U32)tid, err);
    g_SysTask[i].m_Inuse = 1;
    g_SysTask[i].m_TaskHd = tid;


    g_SysTask[i].state = SYS_TASK_RUN;

    *Task_ID = i;

    ret = pthread_mutex_unlock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    return ERROR_CODE_NO_ERROR;
}

ErrorCode_t SYS_TaskKill(U32 Task_ID)
{
	int ret;
    ErrorCode_t err = 0;
    pthread_t tid;

    ret = pthread_mutex_lock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    if ((Task_ID >= MAX_SYSTASKNUM) || g_SysTask[Task_ID].m_Inuse == 0) {
	  ret = pthread_mutex_unlock(&task_mutex);
	  MTOS_BUG_ON((ret != 0), ret);
	  return ERROR_CODE_ERROR_PARM;
    }
    ret = pthread_mutex_unlock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    tid = g_SysTask[Task_ID].m_TaskHd;
    err = (ErrorCode_t)pthread_cancel(tid);
    if (err == 0) {
	  err = (ErrorCode_t)pthread_join(g_SysTask[Task_ID].m_TaskHd, NULL);
	  if (err == 0) {
	    ret = pthread_mutex_lock(&task_mutex);
		MTOS_BUG_ON((ret != 0), ret);

	    if (g_SysTask[Task_ID].stack_addr) {
		  free(g_SysTask[Task_ID].stack_addr);
	    }
	    memset(&g_SysTask[Task_ID], 0, sizeof(SysTask_t));

	    ret = pthread_mutex_unlock(&task_mutex);
		MTOS_BUG_ON((ret != 0), ret);
	    return ERROR_CODE_NO_ERROR;
	  }
	  else
	  {
    	eprintf("%s: pthread_join failed, err=%d\n",
    		__FUNCTION__,err);
		MTOS_BUG_ON(1, err);
	  }
    }
    else
    {
    	//do nothing
    	eprintf("%s: pthread_cancel failed, err=%d\n",
    		__FUNCTION__,err);
		//MTOS_BUG_ON(1, err);
    }

    ret = pthread_mutex_lock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    if (g_SysTask[Task_ID].stack_addr) {
	  free(g_SysTask[Task_ID].stack_addr);
    }
    memset(&g_SysTask[Task_ID], 0, sizeof(SysTask_t));
    g_SysTask[Task_ID].state = SYS_TASK_IDLE;

    ret = pthread_mutex_unlock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    return err;
}

void SYS_TaskExit(S32 slReturnValue)
{
    U32 TaskID = 0;

    if (SYS_GetCurTaskID(&TaskID) != ERROR_CODE_NO_ERROR)
    {
    	return;
    }

    dprintf("\033[35m %s<%d> -> (name:%s, tid:%d)\033[0m\n",
            __FUNCTION__, __LINE__, g_SysTask[TaskID].m_TaskName, TaskID);
    if (TaskID < MAX_SYSTASKNUM)
    {
      memset(&g_SysTask[TaskID], 0, sizeof(SysTask_t));
      g_SysTask[TaskID].m_Inuse = 0;
      g_SysTask[TaskID].state = SYS_TASK_IDLE;
    }
    pthread_exit(NULL);
}

ErrorCode_t SYS_TaskSuspend(U32 Task_ID) //该函数的功能就是实现等待
{
	int ret;
    ErrorCode_t err = 0;
    U32 CurTaskID = 0;
    pthread_t tid;

    ret = pthread_mutex_lock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    if ((Task_ID >= MAX_SYSTASKNUM) || 0 == g_SysTask[Task_ID].m_Inuse) {
	  eprintf(" Suspend an unused task ! \n");
	  ret = pthread_mutex_unlock(&task_mutex);
	  MTOS_BUG_ON((ret != 0), ret);
	  return ERROR_CODE_ERROR_PARM;
    }
    ret = pthread_mutex_unlock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    if (SYS_GetCurTaskID(&CurTaskID) != ERROR_CODE_NO_ERROR)
    {
		return ERROR_CODE_ERROR_PARM;
    }

	tid = pthread_self();

    if ((g_SysTask[Task_ID].m_TaskID != CurTaskID)
    	&& (!pthread_equal(tid, g_SysTask[Task_ID].m_TaskHd))) {
	  eprintf(" Can't suspend other task ! \n");
	  printf("==================%s LINE = %d pthread_self() = 0x%lx g_SysTask[%d].m_TaskHD = 0x%lx m_TaskID = %d g_SysTask[%d].m_TaskHd = 0x%lx\n",
	       __func__, __LINE__, tid, Task_ID, g_SysTask[Task_ID].m_TaskHd, g_SysTask[Task_ID].m_TaskID, CurTaskID, g_SysTask[CurTaskID].m_TaskHd);
	  printf("g_SysTask[%d].m_TaskName = %s g_SysTask[%d].m_TaskName = %s\n", Task_ID, g_SysTask[Task_ID].m_TaskName, CurTaskID, g_SysTask[CurTaskID].m_TaskName);
	  return ERROR_CODE_ERROR_PARM;
    }

    err = (ErrorCode_t)pthread_mutex_lock(&(g_SysTask[Task_ID].m_TaskMutex));
	MTOS_BUG_ON((err != 0), err);
    if (err != 0) {
		eprintf(" SYS_TaskSuspend task mutex lock err ! \n");
		return ERROR_CODE_ERROR_RESULT;
    }

    g_SysTask[Task_ID].state = SYS_TASK_SUSPEND;

    err = (ErrorCode_t)pthread_cond_wait(&(g_SysTask[Task_ID].m_TaskCond), &(g_SysTask[Task_ID].m_TaskMutex));
	MTOS_BUG_ON((err != 0), err);
    err |= (ErrorCode_t)pthread_mutex_unlock(&(g_SysTask[Task_ID].m_TaskMutex));
	MTOS_BUG_ON((err != 0), err);
    if (err != 0) {
		eprintf(" SYS_TaskSuspend err ! \n");
		return ERROR_CODE_ERROR_RESULT;
    }

    return ERROR_CODE_NO_ERROR;
}

ErrorCode_t SYS_TaskResume(U32 Task_ID)
{
	int ret;
    ErrorCode_t err = 0;

    ret = pthread_mutex_lock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    if ((Task_ID >= MAX_SYSTASKNUM) || g_SysTask[Task_ID].m_Inuse == 0) {
	  eprintf(" Resume an unused task !");
	  ret = pthread_mutex_unlock(&task_mutex);
	  MTOS_BUG_ON((ret != 0), ret);

	  return ERROR_CODE_ERROR_PARM;
    }

    err = (ErrorCode_t)pthread_mutex_lock(&(g_SysTask[Task_ID].m_TaskMutex));
	MTOS_BUG_ON((err != 0), err);

    err = (ErrorCode_t)pthread_cond_signal(&(g_SysTask[Task_ID].m_TaskCond));
	MTOS_BUG_ON((err != 0), err);
    err |= (ErrorCode_t)pthread_mutex_unlock(&(g_SysTask[Task_ID].m_TaskMutex));
	MTOS_BUG_ON((err != 0), err);

    ret = pthread_mutex_unlock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    if (err != 0) {
	  dprintf(" SYS_TaskResume err !");
	  return ERROR_CODE_ERROR_RESULT;
    }

    g_SysTask[Task_ID].state = SYS_TASK_RUN;

    return ERROR_CODE_NO_ERROR;
}

ErrorCode_t SYS_TaskWaitDel(U32 Task_ID, U32 WaitTimeMS)
{
	int ret;
    ErrorCode_t err = 0;
    U8 pthread_exit_status = 0;
    U64 start;

    ret = pthread_mutex_lock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    if ((Task_ID >= MAX_SYSTASKNUM) || g_SysTask[Task_ID].m_Inuse == 0) {
	  ret = pthread_mutex_unlock(&task_mutex);
	  MTOS_BUG_ON((ret != 0), ret);
	  return ERROR_CODE_ERROR_PARM;
    }
    ret = pthread_mutex_unlock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    if (WaitTimeMS == SYS_TIMEOUT_INFINITY) {
	  pthread_exit_status = 1;
    } else {
      //FIXME: how about start overflow?
	  start = SYS_GetMS() + WaitTimeMS;
	  do {
	    if (pthread_kill(g_SysTask[Task_ID].m_TaskHd, 0) == 0) {
		  pthread_exit_status = 0;
		  SYS_TaskDelay(2);
		  //FXIME: break?
	    } else {
		  pthread_exit_status = 1;
		  break;
	    }
	  } while (SYS_GetMS() <= start);
    }

    if (pthread_exit_status) {
	  err = (ErrorCode_t)pthread_join(g_SysTask[Task_ID].m_TaskHd, NULL);
	  MTOS_BUG_ON((err != 0), err);
	  if (err == 0) {
	    ret = pthread_mutex_lock(&task_mutex);
		MTOS_BUG_ON((ret != 0), ret);
	    if (g_SysTask[Task_ID].stack_addr) {
		  free(g_SysTask[Task_ID].stack_addr);
	    }
	    memset(&g_SysTask[Task_ID], 0, sizeof(SysTask_t));
	    ret = pthread_mutex_unlock(&task_mutex);
		MTOS_BUG_ON((ret != 0), ret);
	  }
	  else
	  {
		  eprintf("\n SYS_TaskWaitDel: pthread_join failed, err %d !!!\n", err);
	  }
	  return ERROR_CODE_NO_ERROR;
    } else {
	  eprintf("\n Task_ID=%d SYS_TaskWaitDel timeout !!!\n", Task_ID);
	  return ERROR_CODE_ERROR_TIMEOUT;
    }
}

ErrorCode_t SYS_TaskGetStack(U32 Task_ID, void **stack_addr)
{
    int ret = 0;
    //size_t     size;
    //void *stackaddr;
    pthread_attr_t attr;

    ret = pthread_mutex_lock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);
    if ((Task_ID >= MAX_SYSTASKNUM) || g_SysTask[Task_ID].m_Inuse == 0) {
  	  eprintf("this Task_ID has not been used===%d\n", Task_ID);
	  ret = pthread_mutex_unlock(&task_mutex);
	  MTOS_BUG_ON((ret != 0), ret);
	  return ERROR_CODE_ERROR_PARM;
    }
    ret = pthread_mutex_unlock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    ret = pthread_attr_init(&attr);
	MTOS_BUG_ON((ret != 0), ret);
    if (ret != 0) {
	  return ERROR_CODE_ERROR_RESULT;
    }

	//FIXME:
    //	pthread_getattr_np(g_SysTask[Task_ID].m_TaskHd, &attr);
    //	pthread_attr_getstack(&attr,(void*)&stackaddr,&size);
    (void)pthread_attr_destroy(&attr);

    //dprintf("the stack addr is=====%x\n",stackaddr);

    return ERROR_CODE_NO_ERROR;
}

ErrorCode_t SYS_TaskYield(void)
{
	//FIXME:
    //	pthread_yield();
    return ERROR_CODE_NO_ERROR;
}

static pthread_key_t thread_key = 0;
static int key_valid = 0;

ErrorCode_t SYS_TaskKeyInit(void)
{
    if (key_valid == 0) {
	  if (0 != pthread_key_create(&thread_key, NULL)) {
	    eprintf("create thread_key error===%d\n", errno);
	    return ERROR_CODE_ERROR_PARM;
	  }

	  key_valid = 1;
    }

    return ERROR_CODE_NO_ERROR;
}

void *SYS_TaskGetData(U32 Task_ID)
{
    if (key_valid == 0) {
	  eprintf("please init pthread_key at first\n");
	  return NULL;
    }
    return pthread_getspecific(thread_key);
}

ErrorCode_t SYS_TaskSetData(U32 Task_ID, void *data)
{
    if (key_valid == 0) {
	  eprintf("please init pthread_key at first\n");
	  return ERROR_CODE_ERROR_NOREADY;
    }

    if (0 != pthread_setspecific(thread_key, data)) {
	  eprintf("set pthread specific data error\n");
	  return ERROR_CODE_ERROR_RESULT;
    }

    return ERROR_CODE_NO_ERROR;
}

ErrorCode_t SYS_GetTaskPriority(U32 Task_ID, int *priority)
{
    struct sched_param sparam;
    int policy = 0;
    int ret = 0;

    ret = pthread_mutex_lock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);
    if ((Task_ID >= MAX_SYSTASKNUM) || g_SysTask[Task_ID].m_Inuse == 0) {
	  eprintf("this Task_ID has not been used===%d\n", Task_ID);
	  ret = pthread_mutex_unlock(&task_mutex);
	  MTOS_BUG_ON((ret != 0), ret);
	  return ERROR_CODE_ERROR_PARM;
    }
    ret = pthread_mutex_unlock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    ret = pthread_getschedparam(g_SysTask[Task_ID].m_TaskHd, &policy, &sparam);
    if (ret != 0) {
	  eprintf("get thread parameter error===%lx===%d\n", g_SysTask[Task_ID].m_TaskHd, ret);
    }

    // sparam.sched_priority = (int)priority;
    *priority = (int)sparam.sched_priority;
    return ERROR_CODE_NO_ERROR;
}

ErrorCode_t SYS_SetTaskPriority(U32 Task_ID, int priority)
{
    struct sched_param sparam;
    int policy = 0;
    int ret = 0;

    ret = pthread_mutex_lock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);
    if ((Task_ID >= MAX_SYSTASKNUM) || g_SysTask[Task_ID].m_Inuse == 0) {
	  eprintf("this Task_ID has not been used===%d\n", Task_ID);
	  ret = pthread_mutex_unlock(&task_mutex);
	  MTOS_BUG_ON((ret != 0), ret);
	  return ERROR_CODE_ERROR_PARM;
    }
    ret = pthread_mutex_unlock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    ret = pthread_getschedparam(g_SysTask[Task_ID].m_TaskHd, &policy, &sparam);
    if (ret != 0) {
	  eprintf("get thread parameter error===%lx===%d\n", g_SysTask[Task_ID].m_TaskHd, ret);
	  return ERROR_CODE_ERROR_RESULT;
    }

    if (priority < sched_get_priority_min(policy)) {
	  priority = sched_get_priority_min(policy);
    }

    if (priority > sched_get_priority_max(policy)) {
	  priority = sched_get_priority_max(policy);
    }

    sparam.sched_priority = priority;
    //FIXME: SCHED_RR?
    //policy = SCHED_RR;
    ret = pthread_setschedparam(g_SysTask[Task_ID].m_TaskHd, policy, &sparam);
    if (ret != 0) {
	  eprintf("set thread parameter error====%lx===%d\n", g_SysTask[Task_ID].m_TaskHd, ret);
    }

    return ERROR_CODE_NO_ERROR;
}

static S8 ProcessName[10] = "null";
S8 *SYS_GetTaskName(U32 Task_ID)
{
	int ret;

    ret = pthread_mutex_lock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);
    if ((Task_ID >= MAX_SYSTASKNUM) || g_SysTask[Task_ID].m_Inuse == 0) {
	  //dprintf(" Get an unused task name!");
	  ret = pthread_mutex_unlock(&task_mutex);
	  MTOS_BUG_ON((ret != 0), ret);
	  return ProcessName;
    }
    ret = pthread_mutex_unlock(&task_mutex);
	MTOS_BUG_ON((ret != 0), ret);

    return (S8 *)g_SysTask[Task_ID].m_TaskName;
}

void SYS_EnterLowPM(void)
{
}

void SYS_TaskLock(void)
{
    //This function is not realized at stlinux platform
    ErrorCode_t err;
    //dprintf("%s:%d:%s\n",__FILE__,__LINE__,__FUNCTION__);
    err = SYS_SemWait(s_hTaskLock, (S32)SYS_TIMEOUT_INFINITY);
    if (err) {
    }
}

void SYS_TaskUnLock(void)
{
    //This function is not realized at stlinux platform
    ErrorCode_t err;
    err = SYS_SemSend(s_hTaskLock);
    if (err) {
    }
}

void SYS_InterruptLock(void)
{
    //should use kernel mode ?
}

void SYS_InterruptUnLock(void)
{
    //should use kernel mode ?
}

ErrorCode_t SYS_TaskStack(U32 task_index)
{
    //int pid = getpid();
    //int tid;
    int StackSize;
    size_t GuardSize;
    char *stack_addr = NULL;
    unsigned int *p;

    U32 code_start = 400000;
    U32 code_end = code_start + 30 * 0x100000;
    int i;

    if ((task_index >= MAX_SYSTASKNUM) || g_SysTask[task_index].m_Inuse == 0) {
	  return ERROR_CODE_ERROR_PARM;
    }

    //tid = g_SysTask[task_index].m_Tid;
    //pid = g_SysTask[task_index].m_Pid;

    StackSize = 0;
    //  pthread_attr_getstacksize(&g_SysTask[task_index].attr,&StackSize);
    GuardSize = 0;
    pthread_attr_getguardsize(&g_SysTask[task_index].attr, &GuardSize);
    //    pthread_attr_getstackaddr(&g_SysTask[task_index].attr,&stack_addr);
    //     pthread_attr_getstack(&g_SysTask[task_index].attr,(void **)&stack_addr,(size_t *)&StackSize);

    dprintf("----- SYS_TaskDumpStack start------\n");
    //FIXME
    //p = (unsigned int *)(stack_addr + StackSize);
    p = (unsigned int *)(stack_addr);
    for (i = 0; i < StackSize / 4; i++) {
	  if ((*p < code_end) && (*p > code_start)) {
	    dprintf("%08x\n", *p);
	  }
	  p--;
    }
    dprintf("----- SYS_TaskDumpStack end------\n");
    return 0;
}

static U64 __bak_time = 0;
ErrorCode_t SYS_TaskInfo(void)
{
    int i;
    pid_t pid = getpid();
    pid_t tid;
    char f[100];
    char buf[200];
    char *S;
    char *tmp;
    FILE *fp;
    //int StackSize;
    size_t GuardSize;

    U64 currtime = SYS_GetMS() / 1000;

    int fake1;
    int fake2;
    int fake3;
    int fake4;
    int fake5;
    ulong fake6;
    ulong fake7;
    ulong fake8;
    ulong fake9;
    ulong fake10;

    char state;
    u64 utime, stime, cutime, cstime;
    u64 tottime = 0;
    float per;
    float tot = 0;

    //char*stack_addr=NULL;

    system("ps");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "cat /proc/%d/stat", pid);
    system(buf);
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "ls /proc/%d/task", pid);
    system(buf);

    for (i = 0; i < MAX_SYSTASKNUM; i++) {
	  if (0 == g_SysTask[i].m_Inuse) {
	    continue;
	  }

	  memset(buf, 0, sizeof(buf));
	  S = buf;

	  //FIXME: m_Tid, m_Pid ?
	  tid = g_SysTask[i].m_Tid;
	  pid = g_SysTask[i].m_Pid;

	  sprintf(f, "/proc/%d/task/%d/stat", pid, tid);

	  fp = fopen(f, "rb");
	  if (fp == NULL) {
	    dprintf("fopen %s err,taskid=%d,name=%s,tid=%d\n", f, i, g_SysTask[i].m_TaskName, tid);
	    continue;
	  }

	  fread(S, 1, sizeof(buf) - 1, fp);
	  fclose(fp);

	  //FIXME
	  S = strchr(S, '(') + 1;
	  tmp = strrchr(S, ')');
	  S = tmp + 2;

	  sscanf(S,
	       "%c "
	       "%d %d %d %d %d "
	       "%lu %lu %lu %lu %lu "
	       "%Lu %Lu %Lu %Lu " /* utime stime cutime cstime */
	       ,
	       &state,
	       &fake1, &fake2, &fake3, &fake4, &fake5,
	       (long unsigned int *)&fake6, (long unsigned int *)&fake7, (long unsigned int *)&fake8, (long unsigned int *)&fake9, (long unsigned int *)&fake10,
	       (long long unsigned int *)&utime, (long long unsigned int *)&stime, (long long unsigned int *)&cutime, (long long unsigned int *)&cstime);

	  //tottime = utime+stime+cutime+cstime;
	  tottime += utime;

	  per = (float)(tottime - g_SysTask[i].cputime) / (float)(currtime - __bak_time);
	  g_SysTask[i].cputime = tottime;

	  tot += per;

	  //StackSize = 0;
	  //	pthread_attr_getstacksize(&g_SysTask[i].attr,&StackSize);
	  GuardSize = 0;
	  pthread_attr_getguardsize(&g_SysTask[i].attr, (size_t *)&GuardSize);
	  //	pthread_attr_getstackaddr(&g_SysTask[i].attr,&stack_addr);
	  //	pthread_attr_getstack(&g_SysTask[i].attr,(void  **)&stack_addr,(size_t *)&StackSize);

	  dprintf("%d-----\n", tid);
	  dprintf("%s", S);
	  //dprintf("taskid=%d,name=%s,tid=%d,GuardSize=%d,stack_init=%d,stack_addr=%p,statck_max=%d ,%c, utime=%d,per=%%%-4f\n",
	  //i,g_SysTask[i].m_TaskName,tid,GuardSize,g_SysTask[i].stacksize,stack_addr,StackSize,state,utime,per);
    }

    dprintf("tot=%f\n", tot);

    __bak_time = currtime;

    return ERROR_CODE_NO_ERROR;
}

void SYS_ShowTaskTime(void)
{
}

void SYS_ClearTaskTime(void)
{
}

/*EOF*/
