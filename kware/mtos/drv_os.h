/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_OS_H__
#define __DRV_OS_H__

#include "mt_type.h"
//#define DRV_SEM_DEBUG

#define K_BYTE (1024)
#define M_BYTE (K_BYTE *K_BYTE)

#define UNUSED_PARAMETER(x) (void)(x)

//#define MSGMNI    16   /* <= IPCMNI */     /* max # of msg queue identifiers */
//#define MSGMAX  8192   /* <= INT_MAX */   /* max size of message (bytes) */
//#define MSGMNB 16384   /* <= INT_MAX */   /* default max size of a message queue */

#define MAX_SYSTASKNUM 256 //128

//message queue numbers
#define MAX_SYSMSGQNUM 128

//message numbers in one message queue
#define MAX_MSGNUMIN1Q 128

//message body size
#define MAX_MSGBODYSIZE 64

#define MAX_MSGSEMSIZE 1024

#define MAX_MSGIDSIZE 512

#define SYS_TIMEOUT_IMMEDIATE 0
#define SYS_TIMEOUT_INFINITY 0xffffffff

typedef enum {
    SYS_TASK_IDLE = 0,
    SYS_TASK_RUN,
    SYS_TASK_SUSPEND,
} SysTaskState_t;

typedef struct
{
    U32 m_Inuse; //msg_q status

    U32 m_MsgQId;  //msg_q index in g_SysMsgQ table
    U32 m_MsgHd;   //internal msg_q id
    U32 m_MsgSize; //size of one msg

    U32 m_MsgQSize; //numbers of msg_q
    U32 m_MsgQCount;

    U32 m_MsgQSem;

} SysMsgQueue_t; // Upper layer msgq management

typedef struct
{
    U32 m_Inuse; //Sem status
    U32 m_WaitingFlag;
    U32 m_SemId;
} SysSem_t;

typedef struct
{
    U32 m_Inuse;
    U32 m_WaitingFlag;
    U32 m_MsgID;
} SysMsgId_t;

void SYS_TaskDelay(U32 DelayTime);

void SYS_SetCurrTime(U64 sec);

void SYS_GetSysTime(U64 *p_sec, U32 *p_msec, U32 *p_usec);

U64 SYS_GetMS(void);

void SYS_DelayUS(U32 u32Us);

void SYS_Reset(void);

void SYS_Run(void);

void SYS_Exit(void);

const S8 *SYS_OS_GetVersion(void);

ErrorCode_t SYS_OS_Init(void);

ErrorCode_t SYS_OS_CloseHardware(void);

ErrorCode_t SYS_GetCurTaskID(U32 *pTask_ID);

ErrorCode_t SYS_TaskGetState(U32 Task_ID, SysTaskState_t *pState);

#ifdef DRV_SEM_DEBUG
void SYS_SemDebug();
ErrorCode_t SYS_SemCreate(U32 InitValue, ulong *SemID, const U8 *Name);
#else
ErrorCode_t SYS_SemCreate(U32 InitValue, ulong *SemID);
#endif
ErrorCode_t SYS_SemGetValue(ulong SemID, int *value);

ErrorCode_t SYS_SemWait(ulong SemID, S32 milliSecsToWait);

ErrorCode_t SYS_SemSend(ulong SemID);
ErrorCode_t SYS_SemDel(ulong SemID);
ErrorCode_t SYS_SemInfo(U32 flag);
ErrorCode_t SYS_MsgQDump(U32 MsgID);
ErrorCode_t SYS_MsgQInit(void);
void SYS_MsgQList(void);
ErrorCode_t SYS_MsgQCreate(U32 MsgSize, U32 MsgCount, U32 *pMsgID);
ErrorCode_t SYS_MsgQWait(void *RecMsg, U32 MsgID, U32 WaitTimeMS);
ErrorCode_t SYS_MsgQSend(const void *SendMsg, U32 MsgID, U32 TimeoutMs);
ErrorCode_t SYS_MsgQInfo(U32 flag);
ErrorCode_t SYS_MsgQQuery(U32 MsgID, U32 *MsgCnt);
ErrorCode_t SYS_MsgQDel(U32 MsgID);
ErrorCode_t SYS_TaskCreate_Detached(void (*Function)(void *), void *Param, U32 StackSize, U32 Priority,
                                    U32 *Task_ID,
                                    const U8 *Name,
                                    void **ppStack);

void SYS_TaskDebug(void);
void SYS_TaskSetSchedPolicy(int policy);
ErrorCode_t SYS_TaskCreate(void (*Function)(void *),
                           void *Param,
                           U32 StackSize,
                           U32 Priority,
                           U32 *Task_ID,
                           const U8 *Name,
                           void **ppStack);
ErrorCode_t SYS_TaskKill(U32 Task_ID);
void SYS_TaskExit(S32 slReturnValue);
ErrorCode_t SYS_TaskSuspend(U32 Task_ID);
ErrorCode_t SYS_TaskResume(U32 Task_ID);
ErrorCode_t SYS_TaskWaitDel(U32 Task_ID, U32 WaitTimeMS);
ErrorCode_t SYS_TaskGetStack(U32 Task_ID, void **stack_addr);
ErrorCode_t SYS_TaskYield(void);
ErrorCode_t SYS_TaskKeyInit(void);
void *SYS_TaskGetData(U32 Task_ID);
ErrorCode_t SYS_TaskSetData(U32 Task_ID, void *data);
ErrorCode_t SYS_GetTaskPriority(U32 Task_ID, int *priority);
ErrorCode_t SYS_SetTaskPriority(U32 Task_ID, int priority);
S8 *SYS_GetTaskName(U32 Task_ID);
void SYS_EnterLowPM(void);
void SYS_TaskLock(void);
void SYS_TaskUnLock(void);
void SYS_InterruptLock(void);

void SYS_InterruptUnLock(void);
ErrorCode_t SYS_TaskStack(U32 task_index);
ErrorCode_t SYS_TaskInfo(void);
void SYS_ShowTaskTime(void);
void SYS_ClearTaskTime(void);
void __thread1(void);
void __thread2(void);
void __pthread_test(void);
int os_api_test(void);

ErrorCode_t SYS_GetTimeByMJD(U16 usMJD, U32 *y, U8 *m, U8 *d);
ErrorCode_t SYS_GetTimeBy24Bit(U8 *pData, U8 *h, U8 *m, U8 *s);

#endif // end __DRV_OS_H__
