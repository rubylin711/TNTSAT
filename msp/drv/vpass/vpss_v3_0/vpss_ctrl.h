/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_CTRL_H__
#define __VPSS_CTRL_H__

#include <linux/list.h>
#include "vpss_instance.h"
#include "vpss_fb.h"
#include "vpss_osal.h"
#include "mt_drv_dev.h"
#include <linux/completion.h>
#include <linux/semaphore.h>
#include <linux/rwlock.h>

#define VPSS_INSTANCE_MAX_NUMB 12
#define VPSS_FRAME_MIN_WIDTH            64
#define VPSS_FRAME_MAX_WIDTH            4096
#define VPSS_FRAME_MIN_HEIGHT           64
#define VPSS_FRAME_MAX_HEIGHT           2304

#define VPSS0_IRQ_NUM 125  //93+32
#define VPSS1_IRQ_NUM 110  //78+32


#define VPSS_GET_IP(hVPSS) (hVPSS / VPSS_INSTANCE_MAX_NUMB)
#define VPSS_GET_INST_POS(hVPSS) (hVPSS % VPSS_INSTANCE_MAX_NUMB)

typedef struct completion VPSS_IRQ_LOCK;
typedef struct semaphore  VPSS_INSTLIST_LOCK;
typedef  struct task_struct*    VPSS_THREAD;

typedef struct hiVPSS_EXP_CTRL_S
{
    rwlock_t stListLock;

    mt_u32 u32Target;

    mt_u32 u32InstanceNum;

    VPSS_INSTANCE_S* pstInstPool[VPSS_INSTANCE_MAX_NUMB];

} VPSS_INST_CTRL_S;

typedef enum hiVPSS_TASK_STATE_E
{
    TASK_STATE_READY = 0,
    TASK_STATE_WAIT,
    TASK_STATE_IDLE,
    TASK_STATE_BUTT
} VPSS_TASK_STATE_E;

typedef struct hiVPSS_TASK_S
{
    VPSS_TASK_STATE_E stState;

    VPSS_INSTANCE_S* pstInstance;

	VPSS_HAL_INFO_S stVpssHalInfo;
    VPSS_FB_NODE_S *pstFrmNode[DEF_MT_DRV_VPSS_PORT_MAX_NUMBER*2];
    mt_u32 u32SucRate;
    mt_u32 u32LastTotal;
    mt_u32 u32SuccessTotal;
    mt_u32 u32Create;
    mt_u32 u32Fail;
    mt_u32 u32TimeOut;

} VPSS_TASK_S;

typedef irqreturn_t (*FN_VPSS_InterruptRoute)(mt_s32 irq, mt_void *dev_id);

typedef struct hiVPSS_CTRL_S
{
    MT_BOOL         bIPVaild;
    VPSS_IP_E       enIP;
    mt_u32          u32VpssIrqNum;
    mt_char         isr_name[30];
    FN_VPSS_InterruptRoute pVpssIntService;

    mt_u32          s32IsVPSSOpen;

    OSAL_EVENT    stTaskNext;
    OSAL_EVENT      stNewTask;
    VPSS_TASK_S     stTask;
    MT_BOOL         bSuspend;

    MT_BOOL         bInMCE;

    mt_handle       hVpssIRQ;

    mt_u32          u32ThreadKilled;
    volatile mt_u32          u32ThreadSleep;
    VPSS_THREAD     hThread;
    volatile mt_u32          s32ThreadPos;
    VPSS_BUFFER_S stRoBuf[DEF_MT_DRV_VPSS_PORT_MAX_NUMBER];
    VPSS_INST_CTRL_S  stInstCtrlInfo;

} VPSS_CTRL_S;

irqreturn_t VPSS0_CTRL_IntService(mt_s32 irq, mt_void *dev_id);
irqreturn_t VPSS1_CTRL_IntService(mt_s32 irq, mt_void *dev_id);
mt_s32 VPSS_CTRL_ProcRead(struct seq_file *p, mt_void *v);
mt_s32 VPSS_CTRL_ProcWrite(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos);


mt_s32 VPSS_CTRL_Init(mt_void);
mt_s32 VPSS_CTRL_DelInit(mt_void);
mt_s32 VPSS_CTRL_SetMceFlag(MT_BOOL bInMCE);
VPSS_HANDLE VPSS_CTRL_CreateInstance(MT_DRV_VPSS_CFG_S *pstVpssCfg);
mt_s32 VPSS_CTRL_DestoryInstance(VPSS_HANDLE hVPSS);
VPSS_INSTANCE_S* VPSS_CTRL_GetInstance(VPSS_HANDLE hVPSS);
mt_s32 VPSS_CTRL_SyncDistributeIP(VPSS_INSTANCE_S *pstInstance,MT_DRV_VPSS_IPMODE_E enIpmode);
mt_s32 VPSS_CTRL_WakeUpThread(mt_void);
mt_s32 VPSS_CTRL_Pause(VPSS_HANDLE hVPSS);
mt_s32 VPSS_CTRL_Resume(VPSS_HANDLE hVPSS);


#endif
