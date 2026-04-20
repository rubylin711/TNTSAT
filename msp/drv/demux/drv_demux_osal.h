/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef  __UNF_OSAL_H__
#define __UNF_OSAL_H__

#include "mt_type.h"
#include "mt_debug.h"
#include "mt_kernel_adapt.h"
#include "mt_drv_struct.h"

#ifndef WIN32
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <asm/unistd.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
#include <linux/smp_lock.h>
#endif
#include <linux/init.h>
#include <linux/delay.h>
//#include <linux/uaccess.h>
//#include <asm/io.h>
//#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
//#include <linux/kcom.h>
//#include <mach/hardware.h>
//#include <asm/signal.h>
#include <linux/time.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <linux/mm.h>

#else
#include <stdlib.h>
#include <memory.h>
#endif

/** unf_osal.h
 *  Interface definitions of the OS abstraction layer.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
/**********************************************************************
* INTERRUPT
**********************************************************************/
mt_void               UNF_OSAL_IrqLock( mt_u32 *pFlag );
mt_void               UNF_OSAL_IrqUnlock( mt_u32 *pFlag );

/**********************************************************************
* MEMORY
**********************************************************************/

#if 0
// TODO, hardware special memory
mt_s32                UNF_OSAL_AllocMemory( SINT32 ExpectPhyAddr, SINT32 ExpectSize, OSAL_MEM_S *pOsalMem );
mt_s32                UNF_OSAL_ReleaseMemory( OSAL_MEM_S *pMemRet );
mt_s32                UNF_OSAL_MapRegisterAddr( SINT32 RegPhyAddr, SINT32 Range, OSAL_MEM_S *pOsalMem );
mt_s32                UNF_OSAL_UnmapRegisterAddr( OSAL_MEM_S *pOsalMem );
#endif

/**********************************************************************
* THREADS
**********************************************************************/
mt_s32                UNF_OSAL_ThreadCreate( mt_u32            (*pFunc)(mt_void* pParam),
                                             mt_char           TaskName[],
                                             mt_void*          pParam,
                                             mt_u32            nPriority,
                                             mt_void**         phThread);
mt_s32                UNF_OSAL_ThreadDestroy( mt_void* hThread);

/**********************************************************************
* MUTEX
**********************************************************************/
mt_s32                UNF_OSAL_MutexCreate( mt_void**phMutex);
mt_s32                UNF_OSAL_MutexDestroy( mt_void* hMutex);
mt_s32                UNF_OSAL_MutexLock( mt_void* hMutex);
mt_s32                UNF_OSAL_MutexUnlock( mt_void* hMutex);

/**********************************************************************
* EVENTS
**********************************************************************/
#define UNF_INFINITE_WAIT 0xffffffff
mt_s32                UNF_OSAL_EventCreate( mt_void**phEvent);
mt_s32                UNF_OSAL_EventDestroy( mt_void* hEvent);
mt_s32                UNF_OSAL_EventReset( mt_void* hEvent);
mt_s32                UNF_OSAL_EventSignal( mt_void* hEvent);
mt_s32                UNF_OSAL_EventWait( mt_void* hEvent, mt_u32 mSec, MT_BOOL *pbTimedOut);

/**********************************************************************
* TIME
**********************************************************************/
mt_u32                UNF_OSAL_GetTime(mt_void);
mt_void               UNF_OSAL_Sleep(mt_u32 mSec);


mt_void DMX_AcrtUsSleep(mt_u32 us);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
