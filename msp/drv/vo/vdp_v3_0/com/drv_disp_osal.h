
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_osal.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_DISP_OSAL_H__
#define __DRV_DISP_OSAL_H__

/* platform switch */
//#define __DISP_PLATFORM_VC__
//#define __DISP_PLATFORM_SDK__
//#define __DISP_PLATFORM_BOOT__

// for printk
#include <linux/kernel.h>

// for memset
#include <linux/string.h>

// for msleep
#include <linux/delay.h>


// for MT_KMALLOC
#include "mt_drv_mem.h"
#include "mt_drv_sys.h"

// for MT_ID_DISP
#include "mt_module.h"

// for MT_FATAL_PRINT
#include "mt_debug.h"

// for error code
#include "mt_error_mpi.h"

// for interrupt
#include <linux/interrupt.h>

// for do_gettimeofday
#include <linux/time.h>

// for IO_ADDRESS
#include <asm/io.h>

// for MMZ
#include "mt_drv_mmz.h"

#if 1
#define assert(expr) do { \
    if (!(expr)) \
        printk("!!ASSERTION FAILED: [%s:%d] \"" #expr "\"\n", __FILE__, __LINE__); \
    } while (0)
#else
#define assert(expr)
#endif

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */


#if defined(__DISP_PLATFORM_SDK__)
/******************* SDK **********************/
#define DISP_PRINT(fmt...) \
            MT_INFO_PRINT(MT_ID_DISP, fmt)

#define DISP_FATAL(fmt...) \
            MT_FATAL_PRINT(MT_ID_DISP, fmt)

#define DISP_ERROR(fmt...) \
            MT_ERR_PRINT(MT_ID_DISP, fmt)

#define DISP_WARN(fmt...) \
            MT_WARN_PRINT(MT_ID_DISP, fmt)

#define DISP_INFO(fmt...) \
            MT_INFO_PRINT(MT_ID_DISP, fmt)

#define DISP_FATAL_RETURN() \
do{    \
    MT_FATAL_PRINT(MT_ID_DISP, "\n"); \
    return MT_FAILURE;  \
}while(0)

#define WIN_FATAL(fmt...) \
            MT_FATAL_PRINT(MT_ID_VO, fmt)

#define WIN_ERROR(fmt...) \
            MT_ERR_PRINT(MT_ID_VO, fmt)

#define WIN_WARN(fmt...) \
            MT_WARN_PRINT(MT_ID_VO, fmt)

#define WIN_INFO(fmt...) \
            MT_INFO_PRINT(MT_ID_VO, fmt)


#define DISP_ASSERT(exp)  MT_ASSERT(exp)


#define DISP_IOADDRESS(a) IO_ADDRESS(a)

#define DISP_MALLOC(a) ({         \
             mt_u32 b = 0; \
             b = (mt_u32) vmalloc(a); \
             if (b) \
                 memset((void*)b, 0, a); \
             b; \
})

#define DISP_FREE(a)   vfree(a)
#define DISP_MEMSET(a, b, c) memset(a, b, c)

#define DISP_MSLEEP(a) msleep(a)

#define DISP_UDELAY(a) udelay(a)

#define DISP_DSB()   dsb()

#define DISP_SPRINTF sprintf

#elif defined(__DISP_PLATFORM_VC__)
/******************* VC **********************/
#define DISP_PRINT printf

#define DISP_FATAL printf
#define DISP_ERROR printf
#define DISP_WARN  printf
#define DISP_INFO  printf

#define DISP_FATAL_RETURN() \
do{    \
    printf("FATAL! F=%s, L=%d\n", __FUNCTION__, __LINE__); \
    return MT_FAILURE;  \
}while(0)

#define WIN_FATAL printf
#define WIN_ERROR printf
#define WIN_WARN  printf
#define WIN_INFO  printf



#define DISP_IOADDRESS(a)

#define DISP_MALLOC(a) ({         \
             mt_u32 b = 0; \
             b = (mt_u32) malloc(a); \
             if (b) \
                 memset((void*)b, 0, a); \
             b; \
})


#define DISP_FREE(a)   free(a)
#define DISP_MEMSET(a, b, c) memset(a, b, c)

#define DISP_MSLEEP(a)

#define DISP_UDELAY(a)

#define DISP_DSB()

#define DISP_SPRINTF sprintf
#endif

#if 1

#include "mt_error_mpi.h"
#include "mt_common.h"


#define WIN_ERROR(fmt...)       MT_INFO_PRINT(MT_ID_VO, fmt)
#define WIN_FATAL(fmt...)       MT_ERR_PRINT(MT_ID_VO, fmt)
#define WIN_WARN(fmt...)        MT_INFO_PRINT(MT_ID_VO, fmt)
#define WIN_INFO(fmt...)        MT_INFO_PRINT(MT_ID_VO, fmt)

#define DISP_PRINT(fmt...)       MT_INFO_PRINT(MT_ID_DISP, fmt)

#define DISP_FATAL(fmt...)       MT_ERR_PRINT(MT_ID_DISP, fmt)

#define DISP_ERROR(fmt...)       MT_ERR_PRINT(MT_ID_DISP, fmt)

#define DISP_WARN(fmt...)        MT_INFO_PRINT(MT_ID_DISP, fmt)

#define DISP_INFO(fmt...)        MT_INFO_PRINT(MT_ID_DISP, fmt)

#define DISP_FATAL_RETURN() \
do{    \
    printf("\n"); \
    return MT_FAILURE;  \
}while(0)

#define DISP_ASSERT(exp)  assert(exp)  //MT_ASSERT(exp)

#define DISP_IOADDRESS(a) (mt_u32 *)(a)

//#define DISP_MALLOC(a)  kmalloc(a, GFP_KERNEL);//malloc
//#define DISP_FREE(a)    kfree(a);//free
#define DISP_MALLOC(a) ({         \
             void* b = NULL; \
             b = (void *) vmalloc(a); \
             if (b) \
                 memset((void*)b, 0, a); \
             b; \
})

#define DISP_FREE(a)   vfree(a)

#define DISP_MEMSET(a, b, c) memset(a, b, c)

#define DISP_MSLEEP(a) msleep(a)

#define DISP_UDELAY(a) udelay(a)

//#define DISP_DSB()   dsb()

//#define DISP_SPRINTF sprintf

#endif

mt_void DISP_OS_GetTime(mt_u32 *t_ms);

typedef struct hiDISP_MMZ_BUF_S
{
    ulong u32StartVirAddr;

    phys_addr_t u32StartPhyAddr;

    mt_u32 u32Size;
}
DISP_MMZ_BUF_S;

mt_s32  DISP_OS_MMZ_Alloc(const char *bufname, char *zone_name, mt_u32 size, int align, DISP_MMZ_BUF_S *pstMBuf);
mt_void DISP_OS_MMZ_Release(DISP_MMZ_BUF_S *pstMBuf);

mt_s32 DISP_OS_MMZ_Map(DISP_MMZ_BUF_S *pstMBuf);
mt_s32 DISP_OS_MMZ_UnMap(DISP_MMZ_BUF_S *pstMBuf);

mt_s32 DISP_OS_MMZ_AllocAndMap(const char *bufname, char *zone_name, mt_u32 size, int align, DISP_MMZ_BUF_S *pstMBuf);
mt_void DISP_OS_MMZ_UnmapAndRelease(DISP_MMZ_BUF_S *pstMBuf);

#ifdef __cplusplus

#if __cplusplus
}

#endif
#endif /* __cplusplus */

#endif /*  __DRV_DISP_OSAL_H__  */

