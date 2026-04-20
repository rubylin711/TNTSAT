#ifndef __MT_DRV_MEM_H__
#define __MT_DRV_MEM_H__

#include "../../inc/mt_type.h"
#include "../../inc/mt_module.h"
#include "../../inc/mt_debug.h"


#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/*Define Debug Level For MEM */
#define MT_FATAL_MEM(fmt...)    MT_FATAL_PRINT(MT_ID_MEM, fmt)
#define MT_ERR_MEM(fmt...)      MT_ERR_PRINT(MT_ID_MEM, fmt)
#define MT_WARN_MEM(fmt...)     MT_WARN_PRINT(MT_ID_MEM, fmt)
#define MT_INFO_MEM(fmt...)     MT_INFO_PRINT(MT_ID_MEM, fmt)

#if 1
#define MEM_LOCK_DECLARE(p_mutex)                 \
        static pthread_mutex_t p_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MEM_LOCK_DESTROY(p_mutex)                  \
        (void)pthread_mutex_destroy(p_mutex)

#define MEM_LOCK(p_mutex)                          \
        (void)pthread_mutex_lock(p_mutex)

#define MEM_UNLOCK(p_mutex)                        \
        (void)pthread_mutex_unlock(p_mutex)
#else
#define MEM_LOCK_DECLARE(p_mutex);

#define MEM_LOCK_DESTROY(p_mutex)

#define MEM_LOCK(p_mutex)

#define MEM_UNLOCK(p_mutex)

#endif

/*MPI error code*/
#define MT_ERR_MEM_OPEN_FAILED            MT_DEF_ERR(MT_ID_MEM, MT_LOG_LEVEL_ERROR, 41)
#define MT_ERR_MEM_CLOSE_FAILED           MT_DEF_ERR(MT_ID_MEM, MT_LOG_LEVEL_ERROR, 42)
#define MT_ERR_MEM_INVALID_PARA           MT_DEF_ERR(MT_ID_MEM, MT_LOG_LEVEL_ERROR, 43)
#define MT_ERR_MEM_MALLOC_FAILED          MT_DEF_ERR(MT_ID_MEM, MT_LOG_LEVEL_ERROR, 44)
#define MT_ERR_MEM_NOT_OPEN               MT_DEF_ERR(MT_ID_MEM, MT_LOG_LEVEL_ERROR, 45)

#ifdef __KERNEL__

#include <linux/slab.h>
#include <linux/vmalloc.h>

#ifdef CMN_MMGR_SUPPORT
#define MT_KMALLOC(module_id, size, flags)      mt_kmalloc(module_id, size, flags)
#define MT_KFREE(module_id, addr)               mt_kfree(module_id, addr)
#define MT_VMALLOC(module_id, size)             mt_vmalloc(module_id, size)
#define MT_VFREE(module_id, addr)               mt_vfree(module_id, addr)
#else
#define MT_KMALLOC(module_id, size, flags)      kmalloc(size, flags)
#define MT_KFREE(module_id, addr)               kfree(addr)
#define MT_VMALLOC(module_id, size)             vmalloc(size)
#define MT_VFREE(module_id, addr)               vfree(addr)
#endif

mt_void*    mt_kmalloc(mt_u32 module_id, mt_u32 size, mt_s32 flags);
mt_void     mt_kfree(mt_u32 module_id, mt_void *ptr);

mt_void*    mt_vmalloc(mt_u32 module_id, mt_u32 size);
mt_void     mt_vfree(mt_u32 module_id, mt_void *ptr);
#endif

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* End of #ifndef __MT_DRV_MEM_H__ */

