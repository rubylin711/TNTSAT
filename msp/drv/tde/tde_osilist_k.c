/*****************************************************************************
*             Copyright 2006 - 2014, Montage Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: tde_osilist.h
* Description:TDE osi list interface define
*
* History:
* Version   Date          Author        DefectNum       Description
*
*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */
#ifndef TDE_BOOT
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#endif
#include <uapi/linux/time.h>
#include "tde_define.h"
#include "tde_handle.h"
#include "tde_buffer.h"
#include "tde_osilist.h"
#include "tde_hal.h"
#include "wmalloc.h"
#include "mt_module_debug.h"

//#define TDE_DEBUG_DISABLE_4
#if defined(TDE_DEBUG_DISABLE) || defined(TDE_DEBUG_DISABLE_4)
#define DUMP_LOG \
    do {         \
    } while (0)
#define TDE_FUN_IN DUMP_LOG
#define TDE_FUN_OUT DUMP_LOG
#define TDE_LOG(...) DUMP_LOG
#define TDE_LINE DUMP_LOG
#else
#define TDE_FUN_IN MT_INFO_TDE("------------in--------------\n")
#define TDE_FUN_OUT MT_INFO_TDE("---------------out---------------\n")
#define TDE_LOG printk
#define TDE_LINE printk("%s, LINE: %d\n", __FUNCTION__, __LINE__)
#endif

/* JOB LIST head node definition */
typedef struct mtTDE_SWJOBLIST_S
{
    struct list_head stList;
    mt_s32 s32HandleLast;     /* job handle wait for last submit */
    mt_s32 s32HandleFinished; /* job handle last completed */
    mt_u32 u32JobNum;         /* job number in queue */
#ifndef TDE_BOOT
    spinlock_t lock;
#endif
    TDE_SWJOB_S *pstJobCommitted; /* last submited job node pointer, which is the first job */
    TDE_SWJOB_S *pstJobToCommit;  /* job node pointer wait for submit,which is the first job */
    mt_void *pstJobFinished;
    TDE_SWJOB_S *pstJobLast; /* last job in the list */
} TDE_SWJOBLIST_S;

#ifndef TDE_BOOT
typedef mt_void (*TDE_WQ_CB)(mt_u32);

typedef struct
{
    ulong Count;
    TDE_WQ_CB pWQCB;
    struct work_struct work;
} TDEFREEWQ_S;

/****************************************************************************/
/*                   TDE osi list inner variable definition                 */
/****************************************************************************/
STATIC wait_queue_head_t s_TdeBlockJobWq; /* wait queue used to block */
#endif
STATIC TDE_SWJOBLIST_S *s_pstTDEOsiJobList; /* global job list queue */
#ifdef CONFIG_EMU
static struct timeval begin_time , end_time;
#endif
/****************************************************************************/
/*               TDE osi list inner interface definition                    */
/****************************************************************************/
STATIC mt_void TdeOsiListDoFreePhyBuff(mt_u32 u32BuffNum);
STATIC INLINE mt_void TdeOsiListSafeDestroyJob(TDE_SWJOB_S *pstJob);
STATIC mt_void TdeOsiListReleaseHandle(MT_HANDLE_MGR *pstJobHeader);
STATIC INLINE mt_void TdeOsiListAddJob(TDE_SWJOB_S *pstJob);
STATIC mt_void TdeOsiListDestroyJob(TDE_SWJOB_S *pstJob);

/*****************************************************************************
* Function:      TdeOsiListSafeDestroyJob
* Description:   release node from FstCmd to LastCmd
* Input:         pstJob:delete job list
* Output:        none
* Return:        none
* Others:
*****************************************************************************/
STATIC INLINE mt_void TdeOsiListSafeDestroyJob(TDE_SWJOB_S *pstJob)
{
#ifndef TDE_BOOT
    /* if user query this job, release job in query function */
    if (pstJob->u8WaitForDoneCount != 0)
    {
        TDE_TRACE(TDE_KERN_DEBUG, "query handle %d complete!\n", pstJob->s32Handle);
        pstJob->enNotiType = TDE_JOB_NOTIFY_BUTT;
        wake_up_interruptible(&pstJob->stQuery);
    } else
#endif
    {
        TdeOsiListDestroyJob(pstJob);
    }
}

/*****************************************************************************
* Function:      TdeOsiListReleaseHandle
* Description:   release handle manage info
* Input:         pstJobHeader:handle manage struct
* Output:        none
* Return:        none
* Others:
*****************************************************************************/
STATIC mt_void TdeOsiListReleaseHandle(MT_HANDLE_MGR *pstJobHeader)
{
    TDE_ASSERT(NULL != pstJobHeader);
    if (pstJobHeader != NULL)
    {
        if (release_handle(pstJobHeader->handle))
        {
            TDE_FREE(pstJobHeader);
        }
    }
}

/*****************************************************************************
* Function:      TdeOsiListDoFreePhyBuff
* Description:    free temporary buffer 
* Input:         u32BuffNum: the number of temporary buffer
* Output:        none
* Return:        none
* Others:
*****************************************************************************/
STATIC mt_void TdeOsiListDoFreePhyBuff(mt_u32 u32BuffNum)
{
    mt_u32 i;

    for (i = 0; i < u32BuffNum; i++)
    {
        TDE_FreePhysicBuff();
    }
}

/*****************************************************************************
 Prototype    : TdeOsiListInit
 Description  : initialize list manage  module
 Input        : mt_void
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2008/3/5
    Author       : w54130
    Modification : Created function

*****************************************************************************/
mt_s32 TdeOsiListInit(mt_void)
{
#ifndef TDE_BOOT
    init_waitqueue_head(&s_TdeBlockJobWq);
#endif
    if (!initial_handle())
    {
        return MT_FAILURE;
    }

    s_pstTDEOsiJobList = (TDE_SWJOBLIST_S *)TDE_MALLOC(sizeof(TDE_SWJOBLIST_S));
    if (NULL == s_pstTDEOsiJobList)
    {
        destroy_handle();
        return MT_FAILURE;
    }

    INIT_LIST_HEAD(&s_pstTDEOsiJobList->stList);
#ifndef TDE_BOOT
    spin_lock_init(&s_pstTDEOsiJobList->lock);
    spin_lock_init(&s_TDEBuffLock);
#endif
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype    : TdeOsiListFreeNode
 Description  : release node 
 Input        : pNode: node pointer
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2008/3/5
    Author       : w54130
    Modification : Created function

*****************************************************************************/
STATIC INLINE mt_void TdeOsiListFreeNode(TDE_SWNODE_S *pNode)
{
    TDE_ASSERT(NULL != pNode);
    list_del_init(&pNode->stList);
    if (NULL != pNode->stNodeBuf.pBuf)
    {
        TDE_FREE(pNode->stNodeBuf.pBuf);
    }

    TdeOsiListPutPhyBuff(pNode->u32PhyBuffNum);

    TDE_FREE(pNode);
}

/*****************************************************************************
* Function:      TdeOsiListFreeSerialCmd
* Description:   release from FstCmd to LastCmd
* Input:         pstFstCmd: first node
*                pstLastCmd:last node
* Output:        none
* Return:        none
* Others:
*****************************************************************************/
mt_void TdeOsiListFreeSerialCmd(TDE_SWNODE_S *pstFstCmd, TDE_SWNODE_S *pstLastCmd)
{
    TDE_SWNODE_S *pstNextCmd;
    TDE_SWNODE_S *pstCurCmd;

    if ((NULL == pstFstCmd) || (NULL == pstLastCmd))
    {
        return;
    }

    pstCurCmd = pstNextCmd = pstFstCmd;
    while (pstNextCmd != pstLastCmd)
    {
        pstNextCmd = list_entry(pstCurCmd->stList.next, TDE_SWNODE_S, stList);
        if (NULL == pstNextCmd)
        {
            return;
        }
        TdeOsiListFreeNode(pstCurCmd);
        pstCurCmd = pstNextCmd;
    }

    TdeOsiListFreeNode(pstLastCmd);
}

/*****************************************************************************
 Prototype    : TdeOsiListTerm
 Description  : deinitialization of list manager module
 Input        : mt_void
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2008/3/5
    Author       : w54130
    Modification : Created function

*****************************************************************************/
mt_void TdeOsiListTerm(mt_void)
{
    TDE_SWJOB_S *pstJob;

    TDE_FUN_IN;

    while (!list_empty(&s_pstTDEOsiJobList->stList))
    {
        pstJob = list_entry(s_pstTDEOsiJobList->stList.next, TDE_SWJOB_S, stList);
        list_del_init(&pstJob->stList);
        TdeOsiListDestroyJob(pstJob);
    }

    TDE_FREE(s_pstTDEOsiJobList);
    s_pstTDEOsiJobList = NULL;

    destroy_handle();

    TDE_FUN_OUT;

    return;
}

#ifndef TDE_BOOT
mt_void TdeOsiListFlushNode(TDE_SWNODE_S *pNode)
{
    TDE_NODE_BUF_S *pNodeBufInfo = NULL;
    mt_u32 CurNodePhy = 0; /*当前节点对应的物理地址*/
    mmz_buffer_s stFlushBuf = { 0 };
    TDE_ASSERT(NULL != pNode);

    pNodeBufInfo = &pNode->stNodeBuf;

    TDE_FUN_IN;

    if (NULL != pNodeBufInfo->pBuf)
    {
        CurNodePhy = wgetphy(pNodeBufInfo->pBuf);
        stFlushBuf.size = pNodeBufInfo->u32NodeSz + TDE_NODE_HEAD_BYTE + TDE_NODE_TAIL_BYTE;
        stFlushBuf.startPhyAddr = CurNodePhy;
        stFlushBuf.startVirAddr = pNodeBufInfo->pBuf;
        mt_drv_mmz_flush(&stFlushBuf);
    }

    TDE_FUN_OUT;
    return;
}

mt_void TdeOsiListFlushJob(TDE_SWJOB_S *pstJob)
{
    TDE_SWNODE_S *pstNextCmd = NULL;
    TDE_SWNODE_S *pstCurCmd = NULL;
    TDE_SWNODE_S *pstFstCmd = NULL;
    TDE_SWNODE_S *pstLastCmd = NULL;
    TDE_ASSERT(NULL != pstJob);

    pstFstCmd = pstJob->pstFirstCmd;
    pstLastCmd = pstJob->pstTailNode;

    TDE_FUN_IN;

    if ((NULL == pstFstCmd) || (NULL == pstLastCmd))
    {
        TDE_FUN_OUT;
        return;
    }

    pstCurCmd = pstNextCmd = pstFstCmd;
    while (pstNextCmd != pstLastCmd)
    {
        pstNextCmd = list_entry(pstCurCmd->stList.next, TDE_SWNODE_S, stList);
        TdeOsiListFlushNode(pstCurCmd);
        pstCurCmd = pstNextCmd;
    }

    TdeOsiListFlushNode(pstLastCmd);

    TDE_FUN_OUT;
    return;
}
#endif

/*****************************************************************************
* Function:      TdeOsiListAddJob
* Description:   add task info to task list
* Input:         pstJob: job struct
* Output:        none
* Return:        none
* Others:
*****************************************************************************/
STATIC INLINE mt_void TdeOsiListAddJob(TDE_SWJOB_S *pstJob)
{
    TDE_FUN_IN;
    list_add_tail(&pstJob->stList, &s_pstTDEOsiJobList->stList);
    s_pstTDEOsiJobList->u32JobNum++;
    s_pstTDEOsiJobList->s32HandleLast = pstJob->s32Handle;
    s_pstTDEOsiJobList->pstJobLast = pstJob;
    TDE_FUN_OUT;
}

/*****************************************************************************
 Prototype    : TdeOsiListBeginJob
 Description  : create a job
 Input        : NONE
 Output       : pHandle: created job handle
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2008/3/5
    Author       : w54130
    Modification : Created function

*****************************************************************************/
mt_s32 TdeOsiListBeginJob(TDE_HANDLE *pHandle)
{
    MT_HANDLE_MGR *pHandleMgr;
    TDE_SWJOB_S *pstJob;

    TDE_FUN_IN;
    pHandleMgr = (MT_HANDLE_MGR *)TDE_MALLOC(sizeof(MT_HANDLE_MGR) + sizeof(TDE_SWJOB_S));
    if (NULL == pHandleMgr)
    {
        TDE_TRACE(TDE_KERN_INFO, "TDE BegJob Malloc Fail!\n");
        TDE_FUN_OUT;
        return MT_ERR_TDE_NO_MEM;
    }
    get_handle(pHandleMgr, (mt_s32 *)pHandle);
    pstJob = (TDE_SWJOB_S *)((mt_u8 *)pHandleMgr + sizeof(MT_HANDLE_MGR));
    pHandleMgr->res = (mt_void *)pstJob;
#ifndef TDE_BOOT
    INIT_LIST_HEAD(&pstJob->stList);
    init_waitqueue_head(&pstJob->stQuery);
#endif
    pstJob->s32Handle = *pHandle;
    pstJob->pid = current->tgid;

    TDE_FUN_OUT;
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype    : TdeOsiListDestroyJob
 Description  : destroy a job
 Input        : s32Handle: job handle
 Output       : NONE
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2008/3/5
    Author       : w54130
    Modification : Created function

*****************************************************************************/
STATIC mt_void TdeOsiListDestroyJob(TDE_SWJOB_S *pstJob)
{
    MT_HANDLE_MGR *pHandleMgr;
    TDE_FUN_IN;
    if (!query_handle(pstJob->s32Handle, &pHandleMgr))
    {
        TDE_TRACE(TDE_KERN_DEBUG, "handle %d does not exist!\n", pstJob->s32Handle);
        TDE_FUN_OUT;
        return;
    }
    TdeOsiListFreeSerialCmd(pstJob->pstFirstCmd, pstJob->pstTailNode);
    TdeOsiListReleaseHandle(pHandleMgr);
    TDE_FUN_OUT;
    return;
}

/*****************************************************************************
* Function:      TdeOsiListCancelJob
* Description:   cancel task
* Input:         s32Handle:task handle
* Output:        none
* Return:        =0,success <0,error
* Others:
*****************************************************************************/
#ifndef TDE_BOOT
mt_s32 TdeOsiListCancelJob(TDE_HANDLE s32Handle)
{
    MT_HANDLE_MGR *pHandleMgr;
    TDE_SWJOB_S *pstJob;
#ifndef TDE_BOOT
    mt_size_t lockflags;
#endif

    TDE_FUN_IN;
    TDE_LOCK(&s_pstTDEOsiJobList->lock, lockflags);
    if (!query_handle(s32Handle, &pHandleMgr))
    {
        TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
        TDE_FUN_OUT;
        return MT_SUCCESS;
    }
    pstJob = (TDE_SWJOB_S *)pHandleMgr->res;

    if (pstJob->bSubmitted)
    {
        TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
        TDE_TRACE(TDE_KERN_INFO, "Handle %d has been submitted!\n", s32Handle);
        TDE_FUN_OUT;
        return MT_FAILURE;
    }

    TdeOsiListFreeSerialCmd(pstJob->pstFirstCmd, pstJob->pstTailNode);
    TdeOsiListReleaseHandle(pHandleMgr);
    TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
    TDE_FUN_OUT;
    return 0;
}
#else
/*****************************************************************************
* Function:      TdeOsiListWaitTdeIdle
* Description:   wait for completion of the task
* Input:         ?T
* Output:        ?T
* Return:        True: Idle/False: Busy
* Others:        ?T
*****************************************************************************/
mt_void static TdeOsiListWaitTdeIdle(mt_void)
{
    TDE_FUN_IN;
    while (1)
    {
        /* query the status interval of 10ms*/
        udelay(10 * 1000);
        
        if (TdeHalCtlIsIdleSafely())
        {
            TDE_FUN_IN;
            return;
        }
    }
}
#endif

/*****************************************************************************
 Prototype    : TdeOsiListSubmitJob
 Description  : when submit job handle by user, at first add job list to global list, and then handle with according by different situation
                1.when TDE is free and no command canbe added,evaluate waited node pointer,start to software list node to hardware
                2.when TDE is not free and no command canbe added, evaluate waited node pointer
                3.when TDE is not free but command canbe added,no handle
 Input        : s32Handle: job handle
                pSwNode: node resource
                enSubmType: node type
 Output       : NONE
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2008/3/5
    Author       : w54130
    Modification : Created function

*****************************************************************************/
mt_s32 TdeOsiListSubmitJob(TDE_HANDLE s32Handle,
                           mt_u32 u32TimeOut, TDE_FUNC_CB pFuncComplCB, mt_void *pFuncPara,
                           TDE_NOTIFY_MODE_E enNotiType)
{
    TDE_SWJOB_S *pstJob;
    MT_HANDLE_MGR *pHandleMgr;
    MT_BOOL bValid;
    TDE_SWNODE_S *pstTailNode = NULL;
    mt_s32 s32Ret;
    mt_u8 *pBuf;
#ifndef TDE_BOOT
    mt_size_t lockflags;
#endif
    MT_BOOL asynflag = 0;
    TDE_FUN_IN;
    bValid = query_handle(s32Handle, &pHandleMgr);
    if (!bValid)
    {
        TDE_TRACE(TDE_KERN_INFO, "invalid handle %d!\n", s32Handle);
        TDE_FUN_OUT;
        return MT_ERR_TDE_INVALID_HANDLE;
    }
    pstJob = (TDE_SWJOB_S *)pHandleMgr->res;
    if (pstJob->bSubmitted)
    {
        TDE_TRACE(TDE_KERN_INFO, "job %d already submitted!\n", s32Handle);
        TDE_FUN_OUT;
        return MT_ERR_TDE_INVALID_HANDLE;
    }

    if (NULL == pstJob->pstFirstCmd)
    {
        TDE_TRACE(TDE_KERN_INFO, "no cmd !\n");
        
        TdeOsiListReleaseHandle(pHandleMgr);
        TDE_FUN_OUT;
        return MT_SUCCESS;
    }
    pstTailNode = pstJob->pstTailNode;

    pBuf = (mt_u8 *)pstTailNode->stNodeBuf.pBuf + TDE_NODE_HEAD_BYTE;
    TdeHalNodeEnableCompleteInt((mt_void *)pBuf);
    pstJob->bSubmitted = MT_TRUE;
    pstJob->enNotiType = enNotiType;
    pstJob->pFuncComplCB = pFuncComplCB;
    pstJob->pFuncPara = pFuncPara;
    TDE_LOCK(&s_pstTDEOsiJobList->lock, lockflags);
    /*If the job to commit is not null,join the current job to the tail node of the last job.*/
    if (MT_NULL != s_pstTDEOsiJobList->pstJobToCommit)
    {
        TDE_SWNODE_S *pstTailNodeInJobList = s_pstTDEOsiJobList->pstJobLast->pstTailNode;
        mt_u32 *pNextNodeAddr = (mt_u32 *)pstTailNodeInJobList->stNodeBuf.pBuf + (TDE_NODE_HEAD_BYTE >> 2) + ((pstTailNodeInJobList->stNodeBuf.u32NodeSz) >> 2);
        mt_u64 *pNextNodeUpdate = (mt_u64 *)(pNextNodeAddr + 1);
        
        *pNextNodeAddr = pstJob->pstFirstCmd->stNodeBuf.u32PhyAddr;
        
        *pNextNodeUpdate = pstJob->pstFirstCmd->stNodeBuf.u64Update;
        
        if (pstJob->bAqUseBuff)
        {
            s_pstTDEOsiJobList->pstJobToCommit->bAqUseBuff = MT_TRUE;
        }
#ifdef TDE_CACHE_STRATEGY
        /*将job中的对应的所有 hw 节点flush到内存，保证硬件能正确访问*/
        TdeOsiListFlushJob(s_pstTDEOsiJobList->pstJobLast);
#endif
    }
    else
    {
        s_pstTDEOsiJobList->pstJobToCommit = pstJob;
    }

#ifdef TDE_CACHE_STRATEGY
    /*将job中的对应的所有 hw 节点flush到内存，保证硬件能正确访问*/
    TdeOsiListFlushJob(pstJob);
#endif
    TdeOsiListAddJob(pstJob);
    if (TDE_JOB_WAKE_NOTIFY != enNotiType)
    {
        if ((!in_interrupt()) && (wgetfreenum() < 5))
        {
            pstJob->enNotiType = TDE_JOB_WAKE_NOTIFY;
            enNotiType = TDE_JOB_WAKE_NOTIFY;
            u32TimeOut = 1000;
            asynflag = 1; /*由非阻塞方式转为阻塞方式标志位*/
        }
    }

#ifdef CONFIG_EMU
    do_gettimeofday(&begin_time);
#endif
    s32Ret = TdeHalNodeExecute(s_pstTDEOsiJobList->pstJobToCommit->pstFirstCmd->stNodeBuf.u32PhyAddr,
                               s_pstTDEOsiJobList->pstJobToCommit->pstFirstCmd->stNodeBuf.u64Update,
                               s_pstTDEOsiJobList->pstJobToCommit->bAqUseBuff);
    if (s32Ret == MT_SUCCESS)
    {
        s_pstTDEOsiJobList->pstJobCommitted = s_pstTDEOsiJobList->pstJobToCommit;
        s_pstTDEOsiJobList->pstJobToCommit = MT_NULL;
        s_pstTDEOsiJobList->pstJobFinished = MT_NULL;
    }

#ifndef TDE_BOOT
    if (TDE_JOB_WAKE_NOTIFY == enNotiType)
    {
        pstJob->u8WaitForDoneCount++;
        TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
        s32Ret = wait_event_interruptible_hrtimeout(s_TdeBlockJobWq, (TDE_JOB_NOTIFY_BUTT == pstJob->enNotiType), ms_to_ktime(u32TimeOut));
        TDE_LOCK(&s_pstTDEOsiJobList->lock, lockflags);
        pstJob->u8WaitForDoneCount--;
        
        if (TDE_JOB_NOTIFY_BUTT == pstJob->enNotiType)
        {
            TDE_TRACE(TDE_KERN_DEBUG, "handle:%d complete!\n", pstJob->s32Handle);
            if (pstJob->u8WaitForDoneCount == 0)
            {
                TdeOsiListDestroyJob(pstJob);
            }
            TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
#ifdef CONFIG_EMU
		     do_gettimeofday(&end_time);


//		     printk("\nTDE : opType(0x%x), opSize(0x%x):  %d ms \n\n", TdeHalReadReg(0x4), TdeHalReadReg(0xf8), (end_time.tv_sec - begin_time.tv_sec)*1000 + 
//				(end_time.tv_usec - begin_time.tv_usec)/1000);
		     MT_INFO_LGO("\nPEF-feature3:  Graphics 1 osd :  %d ms (jizhenmei)\n\n", (end_time.tv_sec - begin_time.tv_sec)*1000 + 
				(end_time.tv_usec - begin_time.tv_usec)/1000);
#endif

            TDE_FUN_OUT;
            return MT_SUCCESS;
        }
        else
        {
            pstJob->enNotiType = TDE_JOB_COMPL_NOTIFY;
            TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
            if ((-ERESTARTSYS) == s32Ret)
            {
                TDE_TRACE(TDE_KERN_ERR, "handle:%d interrupt!\n", pstJob->s32Handle);
                TDE_FUN_OUT;
                return MT_ERR_TDE_INTERRUPT;
            }
            if (1 == asynflag)
            {
                /*如果由非阻塞方式转为阻塞方式则无超时信息*/
                return MT_SUCCESS;
            }
            TDE_TRACE(TDE_KERN_ERR, "handle:%d timeout!\n", pstJob->s32Handle);
            TDE_FUN_OUT;
            return MT_ERR_TDE_JOB_TIMEOUT;
        }
    }
#else
    /* to do:等待TDE任务完成 */
    TdeOsiListWaitTdeIdle();

    /* 从链表中删除该job节点,防止再次遍历到该节点 */
    list_del_init(&pstJob->stList);

    /* 释放job */
    TdeOsiListSafeDestroyJob(pstJob);

    TDE_TRACE(TDE_KERN_DEBUG, "handle:%d complete!\n", pstJob->s32Handle);
#endif
    TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
    TDE_FUN_OUT;
    return MT_SUCCESS;
}

#ifndef TDE_BOOT
/*****************************************************************************
 Prototype    : TdeOsiListReset
 Description  : Reset all state, free list space
 Input        : NONE
 Output       : NONE
 Return Value : MT_SUCCESS/MT_FAILURE
 Calls        :
 Called By    :
*****************************************************************************/
mt_void TdeOsiListReset(mt_void)
{
    TDE_SWJOB_S *pstDelJob;
    mt_size_t lockflags;

//    TDE_FUN_IN;

//    while (1)
//    {
//        if (TdeHalCtlIsIdleSafely())
//        {
//            TdeHalResumeInit();
//            break;
//        }
//    }

    TdeHalResumeInit();
    TDE_TRACE(TDE_KERN_DEBUG, "TDE reset successfully!\n");
    TDE_LOCK(&s_pstTDEOsiJobList->lock, lockflags);
    while (!list_empty(&s_pstTDEOsiJobList->stList))
    {
        pstDelJob = list_entry(s_pstTDEOsiJobList->stList.next, TDE_SWJOB_S, stList);
        s_pstTDEOsiJobList->u32JobNum--;
        
        list_del_init(&pstDelJob->stList);
        if (TDE_JOB_WAKE_NOTIFY == pstDelJob->enNotiType)
        {
            TDE_TRACE(TDE_KERN_DEBUG, "reset free handle:%d!\n", pstDelJob->s32Handle);
        
            pstDelJob->enNotiType = TDE_JOB_NOTIFY_BUTT;
            wake_up_interruptible(&s_TdeBlockJobWq);
        }
        else if (TDE_JOB_COMPL_NOTIFY == pstDelJob->enNotiType)
        {
            TDE_TRACE(TDE_KERN_DEBUG, "reset free handle:%d!\n", pstDelJob->s32Handle);
        
            TdeOsiListSafeDestroyJob(pstDelJob);
        }
        s_pstTDEOsiJobList->s32HandleFinished = -1;
        s_pstTDEOsiJobList->s32HandleLast = -1;
        
        s_pstTDEOsiJobList->pstJobCommitted = MT_NULL;
        s_pstTDEOsiJobList->pstJobToCommit = MT_NULL;
        s_pstTDEOsiJobList->pstJobLast = MT_NULL;
    }
    TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
    TDE_FUN_OUT;
    return;
}

/*****************************************************************************
 Prototype    : TdeOsiListWaitAllDone
 Description  : wait for all TDE operate is completed,that is to say wait for the last job to be completed.
 Input        : none
 Output       : NONE
 Return Value : MT_SUCCESS,TDE operate completed
 Calls        :
 Called By    :

  History        :
  1.Date         : 2008/3/5

*****************************************************************************/
mt_s32 TdeOsiListWaitAllDone(mt_void)
{
    mt_s32 ret = 0;
    TDE_HANDLE s32WaitHandle;
    mt_size_t lockflags;
    TDE_FUN_IN;
    TDE_LOCK(&s_pstTDEOsiJobList->lock, lockflags);
    s32WaitHandle = s_pstTDEOsiJobList->s32HandleLast;
    TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
    if (-1 == s32WaitHandle)
    {
        TDE_FUN_OUT;
        return MT_SUCCESS;
    }

    ret = TdeOsiListWaitForDone(s32WaitHandle, 0);
    TDE_FUN_OUT;
    return ret;
}

/*****************************************************************************
* Function:      TdeOsiListWaitForDone
* Description:   block to wait for job done
* Input:         s32Handle: job handle
*                u32TimeOut: timeout value
* Output:        none
* Return:        =0,successfully completed <0,error
* Others:
*****************************************************************************/
mt_s32 TdeOsiListWaitForDone(TDE_HANDLE s32Handle, mt_u32 u32TimeOut)
{
    TDE_SWJOB_S *pstJob;
    MT_HANDLE_MGR *pHandleMgr;
    mt_s32 s32Ret;
    MT_BOOL bValid;
    mt_size_t lockflags;

    TDE_FUN_IN;
    TDE_LOCK(&s_pstTDEOsiJobList->lock, lockflags);
    bValid = query_handle(s32Handle, &pHandleMgr);
    if (!bValid)
    {
        TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
        TDE_FUN_OUT;
        return MT_SUCCESS;
    }
    /* AI7D02634 */

    pstJob = (TDE_SWJOB_S *)pHandleMgr->res;
    if (!pstJob->bSubmitted)
    {
        TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
        TDE_TRACE(TDE_KERN_ERR, "job %d has no submitted!\n", s32Handle);
        TDE_FUN_OUT;
        return MT_ERR_TDE_INVALID_HANDLE;
    }
    pstJob->u8WaitForDoneCount++;
    TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
    if (u32TimeOut)
    {
        s32Ret = wait_event_interruptible_hrtimeout(pstJob->stQuery, (TDE_JOB_NOTIFY_BUTT == pstJob->enNotiType), ms_to_ktime(u32TimeOut));
        TDE_LOCK(&s_pstTDEOsiJobList->lock, lockflags);
        pstJob->u8WaitForDoneCount--;
        
        if (TDE_JOB_NOTIFY_BUTT != pstJob->enNotiType)
        {
            TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
            if ((-ERESTARTSYS) == s32Ret)
            {
                TDE_TRACE(TDE_KERN_ERR, "query handle (%d) interrupt!\n", pstJob->s32Handle);
                TDE_FUN_OUT;
                return MT_ERR_TDE_INTERRUPT;
            }
            else
            {
                TDE_TRACE(TDE_KERN_ERR, "query handle (%d) time out!\n", pstJob->s32Handle);
            }
            TDE_FUN_OUT;
            return MT_ERR_TDE_QUERY_TIMEOUT;
        }
        
        /** complete */
        if (pstJob->u8WaitForDoneCount == 0)
        {
            TdeOsiListDestroyJob(pstJob);
        }
        TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
        TDE_FUN_OUT;
        return MT_SUCCESS;
    }

    s32Ret = wait_event_interruptible(pstJob->stQuery, (TDE_JOB_NOTIFY_BUTT == pstJob->enNotiType));
    TDE_LOCK(&s_pstTDEOsiJobList->lock, lockflags);
    pstJob->u8WaitForDoneCount--;
    if ((-ERESTARTSYS) == s32Ret)
    {
        if ((TDE_JOB_NOTIFY_BUTT == pstJob->enNotiType) && (pstJob->u8WaitForDoneCount == 0))
        {
            TdeOsiListDestroyJob(pstJob);
        }
        TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
        TDE_TRACE(TDE_KERN_ERR, "query handle (%d) interrupt!\n", pstJob->s32Handle);
        TDE_FUN_OUT;
        return MT_ERR_TDE_INTERRUPT;
    }

    if (pstJob->u8WaitForDoneCount == 0)
    {
        TdeOsiListDestroyJob(pstJob);
    }
    TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
    TDE_FUN_OUT;
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype    : TdeOsiListCompProc
 Description  : list complete interrupt servic, mainly complete switch on hardware lists
 Input        : mt_void
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2008/3/5
    Author       : w54130
    Modification : Created function

*****************************************************************************/
mt_void TdeOsiListCompProc()
{
    mt_s32 s32Ret = 0;
    TDE_FUN_IN;
    if (s_pstTDEOsiJobList->pstJobToCommit != MT_NULL)
    {
        s32Ret = TdeHalNodeExecute(s_pstTDEOsiJobList->pstJobToCommit->pstFirstCmd->stNodeBuf.u32PhyAddr,
                             s_pstTDEOsiJobList->pstJobToCommit->pstFirstCmd->stNodeBuf.u64Update,
                             s_pstTDEOsiJobList->pstJobToCommit->bAqUseBuff);
        if (s32Ret == MT_SUCCESS)
        {
            s_pstTDEOsiJobList->pstJobCommitted = s_pstTDEOsiJobList->pstJobToCommit;
            s_pstTDEOsiJobList->pstJobToCommit = MT_NULL;
            s_pstTDEOsiJobList->pstJobFinished = MT_NULL;
        }
    }

    TDE_FUN_OUT;
    return;
}

/*****************************************************************************
 Function:      TdeOsiListNodeComp
 Description:   node complete interrupt service, maily complete deleting node and resume suspending,free node 
 Input:         none
 Output:        none
 Return:        create job handle
 Others:        none
 Calls        :
 Called By    :

  History        :
  1.Date         : 2008/3/5
    Author       : w54130
    Modification : Created function

*****************************************************************************/
mt_void TdeOsiListNodeComp()
{
    MT_HANDLE_MGR *pHandleMgr;
    TDE_SWJOB_S *pstJob;
    mt_s32 s32FinishedHandle;
    TDE_SWJOB_S *pstDelJob;
    TDE_HANDLE s32Delhandle;
    ulong u32RunningSwNodeAddr;
    MT_BOOL bWork = MT_TRUE;
    mt_u32 *pu32FinishHandle;
    mt_size_t lockflags;

    TDE_FUN_IN;
    TDE_LOCK(&s_pstTDEOsiJobList->lock, lockflags);
    if (TdeHalCtlIsIdleSafely())
    {
        bWork = MT_FALSE;
    }

    u32RunningSwNodeAddr = TdeHalCurNode();
    if (0 == u32RunningSwNodeAddr)
    {
        TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
        TDE_FUN_OUT;
        return;
    }

    if (u32RunningSwNodeAddr == (ulong)s_pstTDEOsiJobList->pstJobFinished)
    {
        TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
        TDE_FUN_OUT;
        return;
    }

    pu32FinishHandle = (mt_u32 *)wgetvrt(u32RunningSwNodeAddr - TDE_NODE_HEAD_BYTE + 4);
    if (MT_NULL == pu32FinishHandle)
    {
        TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
        TDE_FUN_OUT;
        return;
    }

    s32FinishedHandle = *pu32FinishHandle;

    if (!bWork)
    {
        s_pstTDEOsiJobList->pstJobFinished = (mt_void *)u32RunningSwNodeAddr;
    }
    else
    {
        
        if (!query_handle(s32FinishedHandle, &pHandleMgr)) {
            TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
            TDE_FUN_OUT;
            return;
        }
        pstJob = (TDE_SWJOB_S *)pHandleMgr->res;
        
        if (pstJob->stList.prev == &s_pstTDEOsiJobList->stList)
        {
            TDE_TRACE(TDE_KERN_DEBUG, "No pre Job left, finishedhandle:%d\n", s32FinishedHandle);
            TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
            TDE_FUN_OUT;
            return;
        }
        
        pstDelJob = list_entry(pstJob->stList.prev, TDE_SWJOB_S, stList);
        s32FinishedHandle = pstDelJob->s32Handle;
        s_pstTDEOsiJobList->pstJobFinished = MT_NULL;
    }
    TDE_TRACE(TDE_KERN_DEBUG, "finishedhandle:%d\n", s32FinishedHandle);
    if (!query_handle(s32FinishedHandle, &pHandleMgr))
    {
        TDE_TRACE(TDE_KERN_DEBUG, "handle %d already delete!\n", s32FinishedHandle);
        TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
        TDE_FUN_OUT;
        return;
    }

    s_pstTDEOsiJobList->s32HandleFinished = s32FinishedHandle;

    while (!list_empty(&s_pstTDEOsiJobList->stList))
    {
        pstDelJob = list_entry(s_pstTDEOsiJobList->stList.next, TDE_SWJOB_S, stList);
        s32Delhandle = pstDelJob->s32Handle;
        s_pstTDEOsiJobList->u32JobNum--;
        
        list_del_init(&pstDelJob->stList);
        
        if (TDE_JOB_WAKE_NOTIFY == pstDelJob->enNotiType)
        {
        
            TDE_TRACE(TDE_KERN_DEBUG, "handle:%d!\n", pstDelJob->s32Handle);
        
            pstDelJob->enNotiType = TDE_JOB_NOTIFY_BUTT;
            if (NULL != pstDelJob->pFuncComplCB)
            {
                TDE_TRACE(TDE_KERN_DEBUG, "handle:%d has callback func!\n", pstDelJob->s32Handle);
                TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
                pstDelJob->pFuncComplCB(pstDelJob->pFuncPara, &(pstDelJob->s32Handle));
                TDE_LOCK(&s_pstTDEOsiJobList->lock, lockflags);
            }
            wake_up_interruptible(&s_TdeBlockJobWq);
        }
        else if (TDE_JOB_COMPL_NOTIFY == pstDelJob->enNotiType)
        {
            TDE_TRACE(TDE_KERN_DEBUG, "handle:%d!\n", pstDelJob->s32Handle);
        
            if (NULL != pstDelJob->pFuncComplCB)
            {
                TDE_TRACE(TDE_KERN_DEBUG, "handle:%d has callback func!\n", pstDelJob->s32Handle);
                TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
                pstDelJob->pFuncComplCB(pstDelJob->pFuncPara, &(pstDelJob->s32Handle));
                TDE_LOCK(&s_pstTDEOsiJobList->lock, lockflags);
            }
            TdeOsiListSafeDestroyJob(pstDelJob);
        }
        else
        {
            TDE_TRACE(TDE_KERN_ERR, "Error Status!!\n");
        }
        
        if (s32Delhandle == s32FinishedHandle)
        {
            if (!bWork)
            {
                if (TdeHalCtlIsIdleSafely())
                {
                    TdeHalCtlIntStats();
                
                    TdeOsiListCompProc();
                }
            }
            TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
            TDE_FUN_OUT;
            return;
        }
    }

    TDE_UNLOCK(&s_pstTDEOsiJobList->lock, lockflags);
    TDE_FUN_OUT;
    return;
}
#endif

/*****************************************************************************
* Function:      TdeOsiListGetPhyBuff
* Description:    get one physical buffer, to deflicker and zoom
* Input:         
* Output:        none
* Return:        created job handle
* Others:        none
*****************************************************************************/
mt_u32 TdeOsiListGetPhyBuff(mt_u32 u32CbCrOffset)
{
    mt_u32 ret = 0;
    TDE_FUN_IN;
#ifndef TDE_BOOT
    if (in_interrupt())
    {
        TDE_FUN_OUT;
        return 0;
    }
#endif
    ret = TDE_AllocPhysicBuff(u32CbCrOffset);
    TDE_FUN_OUT;
    return ret;
}

#ifndef TDE_BOOT
void TdeOsiListFreevmem(struct work_struct *work)
{
    TDEFREEWQ_S *pWQueueInfo = container_of(work, TDEFREEWQ_S, work);
    TDE_FUN_IN;
    pWQueueInfo->pWQCB(pWQueueInfo->Count);
    TDE_FREE(pWQueueInfo);
    TDE_FUN_OUT;
}

void TdeOsiListHsr(void *pstFunc, void *data)
{
    TDEFREEWQ_S *pstWQ = NULL;

    TDE_FUN_IN;
    pstWQ = TDE_MALLOC(sizeof(TDEFREEWQ_S));
    if (MT_NULL == pstWQ)
    {
        TDE_TRACE(TDE_KERN_INFO, "Malloc TDEFREEWQ_S failed!\n");
        TDE_FUN_OUT;
        return;
    }
    pstWQ->Count = (ulong)data;
    pstWQ->pWQCB = (TDE_WQ_CB)pstFunc;
    INIT_WORK(&pstWQ->work, TdeOsiListFreevmem);
    schedule_work(&pstWQ->work);
    TDE_FUN_OUT;
    return;
}
#endif

/*****************************************************************************
* Function:      TdeOsiListPutPhyBuff
* Description:   put back physical buffer
* Input:         u32BuffNum 
* Output:        none
* Return:        none
* Others:        none
*****************************************************************************/
mt_void TdeOsiListPutPhyBuff(ulong u32BuffNum)
{
    TDE_FUN_IN;
    if (0 == u32BuffNum)
    {
        TDE_FUN_OUT;
        return;
    }

#ifndef TDE_BOOT
    TdeOsiListHsr((mt_void *)TdeOsiListDoFreePhyBuff, (mt_void *)u32BuffNum);
#else
    TdeOsiListDoFreePhyBuff(u32BuffNum);
#endif

    TDE_FUN_OUT;
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
