/******************************************************************************

  Copyright (C), 2001-2011, Montage Tech. Co., Ltd.

******************************************************************************
  File Name             :   handle.h
  Version               :   Initial Draft
  Author                :   
  Created               :   
  Last Modified         :
  Description           :
  Function List         :
  History               :
******************************************************************************/
#ifndef  __MT_HANDLE_MGR_H__
#define  __MT_HANDLE_MGR_H__
#ifndef TDE_BOOT
#include <linux/list.h>
#include <linux/mm.h>
#else
#include "list.h"
#endif
#include "tde_define.h"
#include "tde_osilist.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

typedef struct mtHandleMgr
{
    struct list_head stHeader;  /* Use to organiza handle */
    #ifndef TDE_BOOT
    spinlock_t lock;
    #endif
    mt_s32   handle;
    mt_void *res;
} MT_HANDLE_MGR;

#define TDE_MAX_HANDLE_VALUE 0x7fffffff

MT_BOOL  initial_handle(mt_void);
        
mt_void  get_handle(MT_HANDLE_MGR *res, mt_s32 *handle);

/*****************************************************************************
* Function:      query_handle
* Description:   Query the job according to the job handle and get the job resource
* Input:         handle:job ID res:job strcut pointer
* Output:        res:job resource
* Return:        Success/fail
* Others:        none
*****************************************************************************/
MT_BOOL  query_handle(mt_s32 handle, MT_HANDLE_MGR **res);
/*****************************************************************************
* Function:      release_handle
* Description:   Delete the handle node from the global handle list according to the handle value. 
* Input:         handle:job ID 
* Output:        none
* Return:        Success/fail
* Others:        none
*****************************************************************************/
MT_BOOL  release_handle(mt_s32 handle);
/*****************************************************************************
* Function:      destroy_handle
* Description:   Free global handle list head node
* Input:          none
* Output:        none
* Return:        none
* Others:        none
*****************************************************************************/
mt_void destroy_handle(mt_void);
#ifndef TDE_BOOT
/*****************************************************************************
* Function:      TdeFreePendingJob
* Description:   Free the job which is not submitted  when execute Ctrl +C (kill the current process).
* Input:          none
* Output:        none
* Return:        none
* Others:        none
*****************************************************************************/
mt_void TdeFreePendingJob(mt_void);
#endif

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */

#endif /* __MT_HANDLE_MGR_H__ */
