
/** @addtogroup iMEM */
/** @{ */

#ifndef __MT_MODULE_MEM_MGR_H__
#define __MT_MODULE_MEM_MGR_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "mt_type.h"

#include "drv_module_ioctl.h"

// !! Bytes:20
typedef struct tagModuleMemInfo
{
    mt_u32 u32ModuleID;

    mt_u32 u32SizeUsrMem;
    mt_u32 u32SizeKernelMem;
    mt_u32 u32SizeMMZ;

    mt_u32 u32MaxMemSize;
}module_mem_info_s;

// !! bytes: 24
typedef struct tagModuleMem
{
    module_mem_info_s stModuleMem;
    struct tagModuleMem* pNext;
}module_mem_s;

// !! bytes:28
typedef struct tagMemMGR
{
    mt_u32       u32Idle;
    
    module_mem_s stModuleMemItem;    
}module_mem_pool_s;

/**
@brief Initialize this module memory manager moudle, which will alloc u32ModuleCount
       items for holding the registered modules 
@attention Before calling other interfaces of this module, calling this interface.
@param[in] u32ModuleCount module total number.
@param[out] None
@retval ::MT_SUCCESS Success
@retval ::MT_FAILURE Failure
@see \n
N/A
*/
mt_s32 module_mem_adp_init(mt_u32 u32ModuleCount);

/**
@brief Terminate this module 
@attention N/A
@param[in] None.
@param[out] None
@retval ::MT_SUCCESS Success
@retval ::MT_FAILURE Failure
@see \n
N/A
*/
mt_s32 module_mem_adp_deinit(mt_void);

/**
@brief Add module info into the link table, mainly the module ID etc. 
@attention N/A
@param[in] pstModule module attirbutes info.
@param[out] None
@retval ::MT_SUCCESS Success
@retval ::MT_FAILURE Failure
@see \n
N/A
*/
mt_s32 module_mem_add_module_info(module_info_s* pstModule);

/**
@brief Delete module from into the link table. 
@attention N/A
@param[in] pstModule module attirbutes info.
@param[out] None
@retval ::MT_SUCCESS Success
@retval ::MT_FAILURE Failure
@see \n
N/A
*/
mt_s32 module_mem_del_module_info(module_info_s* pstModule);


#ifdef __cplusplus
}
#endif
#endif
/** @} */
