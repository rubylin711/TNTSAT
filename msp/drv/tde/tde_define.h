/*****************************************************************************
*             Copyright 2006 - 2014, Montage Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: tde_define.h
* Description:TDE types define for all module
*
* History:
* Version   Date          Author        DefectNum       Description
*
*****************************************************************************/

#ifndef _TDE_DEFINE_H_
#define _TDE_DEFINE_H_
#ifndef TDE_BOOT
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/mm.h>
#include <linux/hardirq.h>
#include "mt_kernel_adapt.h"

#include "mt_gfx_comm_k.h"
#include "mt_drv_mmz.h"
#include "mt_drv_dev.h"
#else
#include "exports.h"
#include "mt_common.h"
#include "hifb_debug.h"
#endif
#include "mt_type.h"
#include "mt_debug.h"
#include "wmalloc.h"
#include "tde_config.h"
#include "tde_adp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/****************************************************************************/
/*                             TDE types define                             */
/****************************************************************************/
typedef struct mtTED_PRIV_DATA_S {
    struct clk *gxaclk;
    struct clk *hdclk;
    struct clk *sdclk_27m;
} TED_PRIV_DATA_S;

/* Pixel format classify */
typedef enum mtTDE_COLORFMT_CATEGORY_E {
    TDE_COLORFMT_CATEGORY_ARGB,
    TDE_COLORFMT_CATEGORY_CLUT,
    TDE_COLORFMT_CATEGORY_An,
    TDE_COLORFMT_CATEGORY_YCbCr,
    TDE_COLORFMT_CATEGORY_BYTE,
    TDE_COLORFMT_CATEGORY_HALFWORD,
    TDE_COLORFMT_CATEGORY_MB,
    TDE_COLORFMT_CATEGORY_BUTT
} TDE_COLORFMT_CATEGORY_E;

/* Node submit type, Independant operation node, Operated head node, Operated middle node */
typedef enum mtTDE_NODE_SUBM_TYPE_E {
    TDE_NODE_SUBM_ALONE = 0,  /* current node submit as independant operated node */
    TDE_NODE_SUBM_PARENT = 1, /* current node submit as operated parent node */
    TDE_NODE_SUBM_CHILD = 2   /* current node submit as operated child node */
} TDE_NODE_SUBM_TYPE_E;

/* Operation setting information node */
typedef struct mtTDE_NODE_BUF_S
{
    mt_void *pBuf;     /* setting information node buffer*/
    mt_u32 u32NodeSz;  /* current node occupied size, united in byte*/
    mt_u64 u64Update;  /* current node update flag */
    phys_addr_t u32PhyAddr; /* current node addr in hardware list */
} TDE_NODE_BUF_S;

/* Notified mode  after current node's job end */
typedef enum hiTDE_NOTIFY_MODE_E {
    TDE_JOB_NONE_NOTIFY = 0, /* none notify after job end */
    TDE_JOB_COMPL_NOTIFY,    /* notify after job end */
    TDE_JOB_WAKE_NOTIFY,     /* wake noytify after job end */
    TDE_JOB_NOTIFY_BUTT
} TDE_NOTIFY_MODE_E;

#define STATIC static
#define INLINE inline
#define TDE_MAX_WAIT_TIMEOUT 2000
extern char *g_pszTdeMmzName;
extern mt_u32 g_u32TdeTmpBuf;
/*若定义该宏，表明tde内部映射mmz内存时使用cache方式，并在subjob时flush tde的节点内存，以便使硬件能正确访问
 若不定义该宏，表面tde内部映射mmz内存时使用nocache方式，可能造成cpu访问率下降*/
#ifndef TDE_BOOT
#define TDE_CACHE_STRATEGY
#endif
//#define TDE_TIME_COUNT

#define TDE_KERN_ERR 1
#define TDE_KERN_WARNING 2
#define TDE_KERN_INFO 3
#define TDE_KERN_DEBUG 4

#define TDE_DEBUG_DISABLE

#ifdef TDE_BOOT
#define MT_GFX_COMM_LOG_FATAL(module_id, fmt...)
#define MT_GFX_COMM_LOG_ERROR(module_id, fmt...)
#define MT_GFX_COMM_LOG_WARNING(module_id, fmt...)
#define MT_GFX_COMM_LOG_INFO(module_id, fmt...)
#define MT_GFX_REG_MAP(base, size) (base)
#define MT_GFX_REG_UNMAP(base)

static inline mt_void MT_GFX_FreeMem(phys_addr_t u32Phyaddr)
{
    return;
}

static inline ulong MT_GFX_AllocMem(mt_char *pName, mt_char *pZoneName, mt_u32 u32LayerSize)
{
    ulong addr;
    mt_s32 ret;
    ret = mt_mem_malloc(&addr, u32LayerSize);
    if (ret != MT_SUCCESS) {
	Debug("TDE_MEM_NEW fail\n");
	return MT_FAILURE;
    }
    return addr;
}

static inline mt_void *MT_GFX_Map(phys_addr_t u32PhyAddr)
{
    return (mt_void *)u32PhyAddr;
}
static inline mt_void *MT_GFX_MapCached(phys_addr_t u32PhyAddr)
{
    return (mt_void *)u32PhyAddr;
}

static inline mt_s32 MT_GFX_Unmap(mt_void *pViraddr)
{
    return MT_SUCCESS;
}
#endif

//#ifndef CONFIG_TDE_DEBUG_DISABLE
#ifndef TDE_DEBUG_DISABLE
#define TDE_TRACE(level, fmt...) printk(fmt)
/*
 do {                                       \
        if(TDE_KERN_ERR == level)           \
            MT_GFX_COMM_LOG_FATAL(MTGFX_TDE_ID, fmt); \
        else if(TDE_KERN_WARNING == level)  \
            MT_GFX_COMM_LOG_ERROR(MTGFX_TDE_ID, fmt);   \
        else if(TDE_KERN_INFO == level)     \
            MT_GFX_COMM_LOG_WARNING(MTGFX_TDE_ID, fmt);  \
        else                                \
            MT_GFX_COMM_LOG_INFO(MTGFX_TDE_ID, fmt);  \
    } while (0)
*/
#else
#define TDE_TRACE(level, fmt...)
#endif

#ifndef CONFIG_TDE_DEBUG_DISABLE
#define TDE_ASSERT(expr)                  \
    do {                                  \
	if (!(expr)) {                    \
	    TDE_TRACE(TDE_KERN_ERR, "Assertion [%s] failed! %s:%s(line=%d)\n", \
#expr, __FILE__, __FUNCTION__, __LINE__); \
	    panic("Assertion panic\n");   \
	}                                 \
    } while (0)
#else
#define TDE_ASSERT(expr)
#endif

STATIC INLINE mt_void *TDE_MALLOC(mt_u32 size)
{
    mt_void *ptr;

    ptr = (mt_void *)wmalloc(size);
    return ptr;
}

STATIC INLINE mt_void TDE_FREE(mt_void *ptr)
{
    wfree(ptr);
}
#ifndef TDE_BOOT
#define TDE_LOCK(lock, lockflags) spin_lock_irqsave(lock, lockflags)

#define TDE_UNLOCK(lock, lockflags) spin_unlock_irqrestore(lock, lockflags)
#else
#define TDE_LOCK(lock, lockflags)
#define TDE_UNLOCK(lock, lockflags)
#endif
#define TDE_INIT_WAITQUEUE_HEAD(pqueue) init_waitqueue_head(pqueue)

#define TDE_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(queue, condition, timeout) wait_event_interruptible_hrtimeout(queue, condition, ms_to_ktime(timeout))

#define TDE_WAKEUP_INTERRUPTIBLE(pqueue) wake_up_interruptible(pqueue)

typedef struct timeval TDE_timeval_s;
#define TDE_gettimeofday do_gettimeofday

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _TDE_DEFINE_H_ */
