/*****************************************************************************
*             Copyright 2006 - 2014, Montage Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: tde_os_listi.h
* Description:TDE osi list interface define
*
* History:
* Version   Date          Author        DefectNum       Description
*
*****************************************************************************/

#ifndef _TDE_OSILIST_H_
#define _TDE_OSILIST_H_

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif /* __cplusplus */
#endif  /* __cplusplus */

#include <linux/list.h>
#include "tde_define.h"
#include "mt_tde_type.h"


/****************************************************************************/
/*                        TDE osi list types define                         */
/****************************************************************************/

/* Data struct of software list node */
typedef struct mtTDE_SWNODE_S
{
    struct list_head stList;    
    mt_s32 s32Handle;           		/* Job handle of the instruct */
    mt_s32 s32Index;            		/* Instruct serial number in job, form one on start, the same number is the same instruct */
    TDE_NODE_SUBM_TYPE_E enSubmType; 	/* current node type */
    TDE_NODE_BUF_S stNodeBuf;       	/* Node of operate config */
    mt_u32 u32PhyBuffNum;           	/* Number of physical buffer distributed */
} TDE_SWNODE_S;

/* Job definition */
typedef struct mtTDE_SWJOB_S
{
    struct list_head stList;    
    ulong s32Handle;               /* Job handle */
    TDE_FUNC_CB       pFuncComplCB; /* Pointer of callback fuction */
    mt_void * pFuncPara;            /* Arguments of callback function */
    TDE_NOTIFY_MODE_E enNotiType;   /* Notice type after node completed */
    mt_u32 u32CmdNum;               /* Instruct number of job */
    mt_u32 u32NodeNum;              /* Node number of job */
    TDE_SWNODE_S *pstFirstCmd;      /* Software node of first instruct in job */
    TDE_SWNODE_S *pstLastCmd;       /* Software node of last instruct in job  */
    TDE_SWNODE_S *pstTailNode;      /* Last software node of job */
    #ifndef TDE_BOOT
    wait_queue_head_t stQuery;      /* Wait queue used in query */
    pid_t  pid;   /* Job process ID*/
    #endif
    MT_BOOL bSubmitted;             /* If have submitted */
    MT_BOOL bAqUseBuff;             /* If using temporary buffer */
    mt_u8   u8WaitForDoneCount;     /* wait job count */        
}TDE_SWJOB_S;


/****************************************************************************/
/*                             TDE osi list functions define                */
/****************************************************************************/

/*****************************************************************************
* Function:      TdeOsiSuspListInit
* Description:   Initialize all lists inside software to use as TDE operation cache
* Input:         null
* Output:        none
* Return:        Success/fail
* Others:        none
*****************************************************************************/
mt_s32      TdeOsiListInit(mt_void);

/*****************************************************************************
* Function:      TdeOsiListTerm
* Description:   Release all lists inside software to use as TDE operation cache
* Input:         NULL
* Output:        none 
* Return:        none
* Others:        none
*****************************************************************************/
mt_void     TdeOsiListTerm(mt_void);

/*****************************************************************************
* Function:      TdeOsiListBeginJob
* Description:   Create list, return head pointer of list
* Input:         none
* Output:        none
* Return:        create task handle 
* Others:        none
*****************************************************************************/
mt_s32      TdeOsiListBeginJob(TDE_HANDLE *pHandle );

/*****************************************************************************
* Function:      TdeOsiListCancelJob
* Description:   Delete list of s32Handle point
* Input:         s32Handle: the list wait for delete
* Output:        none
* Return:        success/fail
* Others:        none
*****************************************************************************/
mt_s32      TdeOsiListCancelJob(TDE_HANDLE s32Handle);


/*****************************************************************************
* Function:      TdeOsiListSubmitJob
* Description:   Create list, return head pointer of list
* Input:         s32Handle: Head pointer of job needing submit
*                bBlock: if block
*                u32TimeOut: time out
*                pFuncComplCB: callback fuction of complete operate
* Output:        none 
* Return:        success/fail/time out
* Others:        none
*****************************************************************************/
mt_s32      TdeOsiListSubmitJob(TDE_HANDLE s32Handle, 
                                mt_u32 u32TimeOut, TDE_FUNC_CB pFuncComplCB, mt_void *pFuncPara,
                                TDE_NOTIFY_MODE_E enNotiType);

/*****************************************************************************
 Prototype    : TdeOsiListWaitAllDone
 Description  : wait for all TDE operate completed
 Input        : None
 Output       : NONE
 Return Value : MT_SUCCESS,TDE operate completed
 Calls        :
 Called By    :
*****************************************************************************/
#ifndef TDE_BOOT
mt_s32 TdeOsiListWaitAllDone(mt_void);

/*****************************************************************************
 Prototype    : TdeOsiListReset
 Description  : Reset all state, release list space 
 Input        : none
 Output       : NONE
 Return Value : MT_SUCCESS/HI_FAILURE
 Calls        :
 Called By    :
*****************************************************************************/
mt_void TdeOsiListReset(mt_void);

/*****************************************************************************
* Function:      TdeOsiListWaitForDone
* Description:   Query if submitted TDE operate is completed
* Input:         s32Handle: task handle 
*                u32TimeOut: if Time out 
* Output:        none
* Return:        Success/fail
* Others:        none
*****************************************************************************/
mt_s32 TdeOsiListWaitForDone(TDE_HANDLE s32Handle, mt_u32 u32TimeOut);


/*****************************************************************************
* Function:      TdeOsiSqCompProc
* Description:   Interrupt to handling all list operate is completed
* Input:         none
* Output:        none
* Return:        task handle is created
* Others:        none
*****************************************************************************/
mt_void     TdeOsiListCompProc(mt_void);


/*****************************************************************************
* Function:      TdeOsiNodeComp
* Description:
* Input:         none
* Output:        none
* Return:        task handle is created
* Others:        none
*****************************************************************************/
mt_void     TdeOsiListNodeComp(mt_void);
#endif

/*****************************************************************************
* Function:      TdeOsiListGetPhyBuff
* Description:   Get one physical buffer, used in deflicker and zoom
* Input:         
* Output:        none
* Return:        physical address assigned
* Others:        none
*****************************************************************************/
mt_u32  TdeOsiListGetPhyBuff(mt_u32 u32CbCrOffset);


/*****************************************************************************
* Function:      TdeOsiListPutPhyBuff
* Description:   put back physical buffer
* Input:         u32BuffNum 
* Output:        none
* Return:        none
* Others:        none
*****************************************************************************/
mt_void  TdeOsiListPutPhyBuff(ulong u32BuffNum);

/*****************************************************************************
* Function:      TdeOsiListFreeSerialCmd
* Description:   FreeSerialCmd
* Input:         FirstCmd 
                     LastCmd
* Output:        none
* Return:        none
* Others:        none
*****************************************************************************/
mt_void TdeOsiListFreeSerialCmd(TDE_SWNODE_S *pstFstCmd, TDE_SWNODE_S *pstLastCmd);


#ifdef __cplusplus
 #if __cplusplus
}
 #endif /* __cplusplus */
#endif  /* __cplusplus */

#endif  /* _TDE_OSILIST_H_ */
