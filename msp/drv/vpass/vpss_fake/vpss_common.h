/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_COMMON_H__
#define __VPSS_COMMON_H__

#include"mt_drv_mem.h"
#include"mt_drv_log.h"
#include"mt_drv_sys.h"
#include"mt_drv_stat.h"
#include"mt_drv_video.h"
#include"mt_drv_vpss.h"
#include "mt_debug.h"
#include <linux/list.h>
#include <linux/io.h>
#include <linux/delay.h>
#include"drv_vdec_ext.h"

#define DEF_FILE_NAMELENGTH 30

typedef struct list_head LIST;

#define FB_DBG 0
#define DEF_VPSS_DEBUG 1

#define VPSS_NAME  "MT_VPSS"

#define VPSS_KMALLOC(fmt...)      MT_KMALLOC  (MT_ID_VPSS, fmt)
#define VPSS_KFREE(fmt...)        MT_KFREE    (MT_ID_VPSS, fmt)
#define VPSS_VMALLOC(fmt...)       MT_VMALLOC   (MT_ID_VPSS, fmt)
#define VPSS_VFREE(fmt...)       MT_VFREE   (MT_ID_VPSS, fmt)

#if 0
#define VPSS_FATAL(fmt...) \
            MT_FATAL_PRINT(MT_ID_VPSS, fmt)

#define VPSS_ERROR(fmt...) \
            MT_ERR_PRINT(MT_ID_VPSS, fmt)

#define VPSS_WARN(fmt...) \
            MT_WARN_PRINT(MT_ID_VPSS, fmt)

#define VPSS_INFO(fmt...) \
            MT_INFO_PRINT(MT_ID_VPSS, fmt)
#else
#define VPSS_FATAL(fmt...)		printk("[F][VPSS]" fmt)
#define VPSS_ERROR(fmt...)		printk("[E][VPSS]" fmt)
#define VPSS_WARN(fmt...)		printk("[W][VPSS]" fmt)
#define VPSS_INFO(fmt...)		printk("[I][VPSS]" fmt)
//#define VPSS_VERB(fmt...)		printk("[V][VPSS]" fmt)
#define VPSS_VERB(fmt...)		do{}while(0)
#endif

#define VPSS_DBG() \
do{\
    MT_PRINT("--->%s %d\n",__func__,__LINE__);\
}while(0)

#define VPSS_CHECK_NULL(ptr)\
do\
{\
    if (MT_NULL == ptr)\
    {\
        VPSS_ERROR("Para is Null\n");\
        return MT_FAILURE;\
    }\
}while(0)

#define VPSS_CHECK_HANDLE(handle)\
do\
{\
    if (MT_INVALID_HANDLE == handle)\
    {\
        VPSS_ERROR("handle is MT_INVALID_HANDLE\n");\
        return MT_FAILURE;\
    }\
}while(0)

#define VPSS_CHECK_VERSION(version,exp)\
do\
{\
    if (version != exp)\
    {\
        VPSS_ERROR("Invalid Version %d.Expect %d\n",version,exp);\
        return MT_FAILURE;\
    }\
}while(0)


#define VPSS_GET_TIME(time)\
do\
{\
    mt_u32 u32Tmp;\
    if(!MT_DRV_SYS_GetTimeStampMs(&u32Tmp))\
    {\
        time = u32Tmp;\
        VPSS_ERROR("Get Time %d \n",time);\
    }\
    else\
    {\
        time = 0;\
        VPSS_ERROR("Get Time Failed\n");\
    }\
}while(0)

typedef enum hiVPSS_VERSION_E
{
    VPSS_VERSION_V1_0 = 0,
    VPSS_VERSION_V2_0 ,
    VPSS_VERSION_BUTT
}VPSS_VERSION_E;

#define VPSS_NTSC_WIDTH  720
#define VPSS_NTSC_HEIGHT 480
#define VPSS_PAL_WIDTH 720
#define VPSS_PAL_HEIGHT 576
#define VPSS_HD_WIDTH  1280
#define VPSS_HD_HEIGHT 720
#define VPSS_FHD_WIDTH 1920
#define VPSS_FHD_HEIGHT 1080
#define VPSS_UHD_WIDTH  4096
#define VPSS_UHD_HEIGHT 2304

#define VPSS_HEIGHT_ALIGN 0xfffffffc
#define VPSS_WIDTH_ALIGN 0xfffffffe

#define VPSS_FRAME_MIN_WIDTH            64
#define VPSS_FRAME_MAX_WIDTH            4096
#define VPSS_FRAME_MIN_HEIGHT           64
#define VPSS_FRAME_MAX_HEIGHT           2304


#define VPSS_SYS_MEM_MIN			   512

#define VPSS_UHD_LOW_W			1920
#define VPSS_UHD_LOW_H			1080

#define VPSS_UHD_MIDDLE_W		1920
#define VPSS_UHD_MIDDLE_H		2160

#define VPSS_UHD_HIGH_W			3840
#define VPSS_UHD_HIGH_H			2160

#endif
