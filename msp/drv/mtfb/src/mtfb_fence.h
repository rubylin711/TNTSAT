/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#ifndef __MTFB_FENCE_H__
#define __MTFB_FENCE_H__


/*********************************add include here******************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))
#include <linux/sw_sync.h>
#else
#include <sw_sync.h>
#endif

/*****************************************************************************/


#ifdef __cplusplus
   extern "C"
{
#endif /* __cplusplus */



/***************************** Macro Definition ******************************/


/*************************** Structure Definition ****************************/
typedef struct
{
	atomic_t s32RefreshCnt;
    mt_u32   u32FenceValue;
    mt_u32   u32Timeline;
    mt_u32  FrameEndFlag;
    wait_queue_head_t    FrameEndEvent;
    struct sw_sync_timeline *pstTimeline;
}MTFB_SYNC_INFO_S;

/********************** Global Variable declaration **************************/



/******************************* API declaration *****************************/


/***********************************************************************
* func		    : mtfb_create_fence
* description	: 创建fence，这里创建的是release fence，返回的是
                  releaseFD，acquire fence有GPU创建，返回的是acquireFD，
                  HWComposer通过Binder通信送给mtfb这个文件描述符
* param[in] 	: *timeline 前面创建的mtfb时间轴
* param[in] 	: fence_name要创建的fence名字，"mtfb_fence"
* param[in] 	: value 创建fence的初始值，前面是timeline的初始值，
                  第一次创建的时候++s_SyncInfo.u32FenceValue也就是2。
                  所以第一次创建fence的初始值为2
* retval		: NA
***********************************************************************/
int mtfb_create_fence(struct sw_sync_timeline *timeline,const char *fence_name,unsigned value);

/***********************************************************************
* func		    : mtfb_fence_wait
* description	: 等待同步
* param[in] 	: GPU创建的fence
* param[in] 	: 等待超时时间
***********************************************************************/
int mtfb_fence_wait(int fence_fd, long timeout);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MTFB_FENCE_H__ */


