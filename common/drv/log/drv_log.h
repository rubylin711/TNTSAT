#ifndef __DRV_LOG_EXT_H__
#define __DRV_LOG_EXT_H__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <asm/types.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "mt_debug.h"
#include "drv_log_ioctl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* End of #ifdef __cplusplus */

#define DRV_LOG_DEVICE_NAME "sys/log"

#define MSG_FROM_USER   0
#define MSG_FROM_KERNEL 1


typedef struct mt_log_buffer_info_s
{
	ulong start_phyaddr;			/*start physic address*/
	mt_u8  *start_viraddr;			/*start virtual address*/
	mt_u32 size;					/*buffer size*/
	mt_u32 wp;						/*write offset*/
	mt_u32 rp;						/*read offset*/
	mt_u32 reset_flag;				/*reset count*/	
	mt_u32 wr_count; 				/*write count*/	
    wait_queue_head_t wq_nodata;    /*no wait queque*/	
	struct semaphore wr_sem; 		/*write semaphore*/
}log_buffer_info_s;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__DRV_LOG_H__*/

