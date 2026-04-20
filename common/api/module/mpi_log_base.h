#ifndef __COMMON_LOG_BASE_H__
#define __COMMON_LOG_BASE_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "mt_type.h"

typedef struct tagModuleLog
{
    mt_u32 u32ModuleID;
    mt_u32 u32ModuleLevel;

    struct tagModuleLog* pstNextNode;
    struct tagModuleLog* pstPrevNode;
}module_log_s;

typedef struct tagModuleLogPool
{
    mt_u32 u32Idle;

    module_log_s stModuleLog;
}module_log_pool_s;

mt_s32 module_log_pool_init(mt_u32 u32ModuleCount);
mt_s32 module_log_pool_deinit(mt_void);

#ifdef __cplusplus
}
#endif

#endif

