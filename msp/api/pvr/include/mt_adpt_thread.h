/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_ADPT_THREAD_H__
#define __MT_ADPT_THREAD_H__

#include "mt_type.h"
#include <stdarg.h>
//comment by x57522
/*
#if VTOP_OS_TYPE == VTOP_OS_LINUX
#include <errno.h>
#elif VTOP_OS_TYPE == VTOP_OS_WIN32
#else
#error YOU MUST DEFINE VTOP OS TYPE VTOP_OS_TYPE == VTOP_OS_WIN32 OR VTOP_OS_LINUX !  
#endif
*/
//add by x7522
#if MT_OS_TYPE == MT_OS_LINUX
#include <errno.h>
#elif MT_OS_TYPE == MT_OS_WIN32
#else
#error YOU MUST DEFINE HI OS TYPE MT_OS_TYPE == MT_OS_WIN32 OR MT_OS_LINUX !  
#endif


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */



typedef MT_U32 MT_Pthread_T;	
	
typedef MT_S32 MT_PID_T;

typedef mt_s64 MT_SIZE_T;

#define MT_ERR_OSCALL_ERROR  (-1)
#define MT_INFINITE            0xFFFFFFFF 
typedef MT_VOID * (*MT_ThreadFun)(MT_VOID *); 
/* Data structure to describe a process' schedulability.  */
typedef struct MT_sched_param
{
    MT_S32 sched_priority;
}MT_S_SchedParam;

/* Attributes for threads.  */
typedef struct MTpthread_attr_s
{
    MT_S32 detachstate;
    MT_S32 schedpolicy;
    MT_S_SchedParam schedparam;
    MT_S32 inheritsched;
    MT_S32 scope;
    MT_SIZE_T guardsize;
    MT_S32 stackaddr_set;
    MT_VOID *stackaddr;
    MT_SIZE_T stacksize;
} MT_S_ThreadAttr;

/* the thread priority */
typedef enum mtVpriority_which
{
    MT_PRIO_PROCESS = 0,             /* WHO is a process ID.  */
    MT_PRIO_PGRP = 1,                /* WHO is a process group ID.  */
    MT_PRIO_USER = 2                 /* WHO is a user ID.  */
}MT_E_Which;

typedef enum mtVrusage_who
{
    /* The calling process.  */
    MT_RUSAGE_SELF = 0,

    /* All of its terminated child processes.  */
    MT_RUSAGE_CHILDREN = -1,

    /* Both.  */
    MT_RUSAGE_BOTH = -2
}MT_E_Who;

/* get errno */
MT_S32 MT_GetLastErr(MT_VOID);
/* set errno */
MT_VOID MT_SetLastErr(MT_S32 newErrNo);

/************************************************************************
|                                                                                                                             |
|                             process operation                                                                      |
|                                                                                                                             |
************************************************************************/

MT_S32 MT_Execv(const MT_CHAR *path, MT_CHAR *const argv[]);
MT_S32 MT_Execvp(const MT_CHAR *file, MT_CHAR *const argv[]);
MT_S32 MT_Waitpid(MT_PID_T pid,  MT_S32 *status, MT_S32 options);


#ifdef MT_OS_SUPPORT_UCLINUX   
#define MT_Fork vfork
#else
MT_PID_T MT_Fork(MT_VOID);
#endif
MT_PID_T MT_Wait(const MT_S32 *status);


#define MT_REBOOT_CMD_RESTART        0x01234567
#define MT_REBOOT_CMD_HALT           0xCDEF0123
#define MT_REBOOT_CMD_CAD_ON         0x89ABCDEF
#define MT_REBOOT_CMD_CAD_OFF        0x00000000
#define MT_REBOOT_CMD_POWER_OFF      0x4321FEDC
#define MT_REBOOT_CMD_RESTART2       0xA1B2C3D4

MT_S32 MT_Reboot(MT_S32 flag);


/************************************************************************
|                                                                                                                             |
|                            thread operation                                                                         |
|                                                                                                                             |
************************************************************************/

/* Function for handling threads.  */

MT_S32 MT_PthreadAttrInit(MT_S_ThreadAttr *attr);

MT_S32 MT_PthreadAttrDestroy(MT_S_ThreadAttr *attr);

MT_S32 MT_PthreadAttrSetdetachstate(MT_S_ThreadAttr *attr, MT_S32 detachstate);

MT_S32 MT_PthreadSetCancelState(MT_S32 state, MT_S32 *oldstate);

MT_S32 MT_PthreadSetCancelType(MT_S32 type, MT_S32 *oldtype);

/* Indicate that the thread TH is never to be joined with PTHREAD_JOIN.
   The resources of TH will therefore be freed immediately when it
   terminates, instead of waiting for another thread to perform PTHREAD_JOIN
   on it.  */
MT_S32 MT_PthreadDetach (MT_Pthread_T th) ;

MT_S32 MT_GetPriority(MT_S32 which, MT_S32 who);

MT_S32 MT_SetPriority(MT_S32 which, MT_S32 who, MT_S32 prio);



/* Create a thread with given attributes ATTR (or default attributes
   if ATTR is MT_NULL), and call function START_ROUTINE with given
   arguments ARG.  */
MT_S32  MT_PthreadCreate (MT_Pthread_T *threadp,
                                                              const MT_S_ThreadAttr *attr,
                                                              MT_ThreadFun start_routine,
                                                              MT_VOID *arg) ;

/* Obtain the identifier of the current thread.  */
MT_Pthread_T  MT_PthreadSelf (MT_VOID);

/* Terminate calling thread.  */
MT_VOID MT_PthreadExit ( MT_VOID* retVal);
/*pthread cancel*/
 MT_S32 MT_PthreadCancel(MT_Pthread_T thread);
/* Make calling thread wait for termination of the thread TH.  The
   exit status of the thread is stored in *THREAD_RETURN, if THREAD_RETURN
   is not MT_NULL.  */
MT_S32 MT_PthreadJoin (MT_Pthread_T th, MT_VOID **thread_return);


/* sleep in second */
MT_U32 MT_Sleep(MT_U32 seconds);
/* sleep in millisecond */
MT_U32 MT_SleepMs(MT_U32 ms);

/* get current thread pid*/
MT_U32 MT_GetPID(MT_VOID);

/**/
MT_VOID MT_Exit(MT_S32 status);

MT_S32 MT_Kill(MT_PID_T pid, MT_S32 sig);

MT_S32 MT_System(MT_CHAR *cmd);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
#endif /* __MT_ADPT_THREAD_H__ */
