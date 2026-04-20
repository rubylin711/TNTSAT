/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_OSAL_H__
#define __VPSS_OSAL_H__

#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/rwlock.h>

#include <linux/uaccess.h>
#include "mt_type.h"
#include "mt_osal.h"
#include "mt_math.h"
#include "mt_common.h"
#include "vpss_common.h"
#include "mt_drv_mmz.h"
#include "mt_drv_video.h"


/************************************************************************/
/*                         data structure                               */
/************************************************************************/

#define OSAL_OK     0
#define OSAL_ERR   -1

#define EVENT_DONE 1
#define EVENT_UNDO 0

#define MT_ALIGN_BYTES 16
#define MT_ALIGN_8BIT_YSTRIDE(x)         MT_SYS_GET_STRIDE(x)
#define MT_ALIGN_10BIT_COMP_YSTRIDE(x)   MT_SYS_GET_STRIDE(HICEILING(x*10, 8))   /* l0bit?? */

typedef  struct file   FILE;



/************************************************************************/
/* file operation                                                       */
/************************************************************************/
struct file *VPSS_OSAL_fopen(const char *filename, int flags, int mode);
void VPSS_OSAL_fclose(struct file *filp);
int VPSS_OSAL_fread(char *buf, unsigned int len, struct file *filp);
int VPSS_OSAL_fwrite(char *buf, int len, struct file *filp);

/************************************************************************/
/* event operation                                                      */
/************************************************************************/
typedef struct hiKERN_EVENT_S
{
	wait_queue_head_t   queue_head;
	mt_s32              flag_1;
	mt_s32              flag_2;
} KERN_EVENT_S;

typedef  KERN_EVENT_S           OSAL_EVENT;
mt_s32 VPSS_OSAL_InitEvent( OSAL_EVENT *pEvent, mt_s32 InitVal1, mt_s32 InitVal2);

mt_s32 VPSS_OSAL_GiveEvent( OSAL_EVENT *pEvent, mt_s32 InitVal1, mt_s32 InitVal2);

mt_s32 VPSS_OSAL_WaitEvent( OSAL_EVENT *pEvent, ktime_t s32WaitTime );

mt_s32 VPSS_OSAL_ResetEvent( OSAL_EVENT *pEvent, mt_s32 InitVal1, mt_s32 InitVal2);



/************************************************************************/
/* mutux lock operation                                                 */
/************************************************************************/
typedef struct semaphore  VPSS_OSAL_LOCK;

mt_s32 VPSS_OSAL_InitLOCK(VPSS_OSAL_LOCK *pLock, mt_u32 u32InitVal);

mt_s32 VPSS_OSAL_DownLock(VPSS_OSAL_LOCK *pLock);

mt_s32 VPSS_OSAL_UpLock(VPSS_OSAL_LOCK *pLock);

mt_s32 VPSS_OSAL_TryLock(VPSS_OSAL_LOCK *pLock);



/************************************************************************/
/* spin lock operation                                                  */
/************************************************************************/
typedef spinlock_t VPSS_OSAL_SPIN;

mt_s32 VPSS_OSAL_InitSpin(VPSS_OSAL_SPIN *pLock);

mt_s32 VPSS_OSAL_DownSpin(VPSS_OSAL_SPIN *pLock,unsigned long *flags);

mt_s32 VPSS_OSAL_UpSpin(VPSS_OSAL_SPIN *pLock,unsigned long *flags);

mt_s32 VPSS_OSAL_TryLockSpin(VPSS_OSAL_SPIN *pLock,unsigned long *flags);


/************************************************************************/
/* debug operation                                                      */
/************************************************************************/
mt_s32 VPSS_OSAL_GetProcArg(mt_char*  chCmd,mt_char*  chArg,mt_u32 u32ArgIdx);

mt_s32 VPSS_OSAL_ParseCmd(mt_char*  chArg1,mt_char*  chArg2,mt_char*  chArg3,mt_void *pstCmd);

mt_s32 VPSS_OSAL_StrToNumb(mt_char*  chStr,mt_u32 *pu32Numb);

mt_s32 VPSS_OSAL_WRITEYUV(MT_DRV_VIDEO_FRAME_S *pstFrame,mt_char* pchFile);
mt_s32 VPSS_OSAL_CalBufSize(mt_u32 *pSize,mt_u32 *pStride,mt_u32 u32Height,mt_u32 u32Width,MT_DRV_PIX_FORMAT_E ePixFormat,MT_DRV_PIXEL_BITWIDTH_E  enOutBitWidth);

mt_s32 VPSS_OSAL_GetTileOffsetAddr(mt_u32 u32Xoffset,mt_u32 u32Yoffset,
                                   mt_u32 *pu32Yaddr,mt_u32 *pu32Caddr,
                                   MT_DRV_VID_FRAME_ADDR_S *pstOriAddr);

mt_s32 VPSS_OSAL_GetVpssVersion(VPSS_VERSION_E *penVersion);

mt_s32 VPSS_OSAL_GetCurTime(mt_u32 *pu32Hour,mt_u32 *pu32Minute,
                                   mt_u32 *pu32Second);

mt_s32 VPSS_OSAL_GetSysMemSize(mt_u32 *pu32MemSize);
#endif
