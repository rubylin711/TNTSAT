
/** @addtogroup iMEM */
/** @{ */
#ifndef __MODULE_MEM_H__
#define __MODULE_MEM_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "mt_type.h"

typedef enum tagMEM_TYPE
{
    MEM_TYPE_MMZ,
    MEM_TYPE_USR,
    MEM_TYPE_KERNEL,
}MEM_TYPE_E;


//!! Bytes:16
struct head
{
    mt_void *addr;
    size_t size;

    struct head* prev;
    struct head* next;
};

//!! Bytes:20
typedef struct tagUSR_MEM_POOL
{
    mt_u32  u32Idle;
    struct head stItem;
}usr_mem_pool_s;


#define MEM_POOL_COUNT (2000*1024/20) /* about 100K times (102400) and hold 2000K bytes memory*/

typedef mt_s32 (*fnModuleInfoCBK)(mt_u32 u32ModuleID, MEM_TYPE_E enMemType, mt_s32 s32Size);

mt_s32  mem_pool_init(mt_u32 u32Count, fnModuleInfoCBK fnCallback);
mt_void mem_pool_deinit(mt_void);

#ifdef __cplusplus
}
#endif

#endif
/** @} */

