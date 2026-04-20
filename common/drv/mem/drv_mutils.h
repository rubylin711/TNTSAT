/** @ingroup module_mmz kmem_utils*/
/** @{ */
#ifndef __KMEM_UTILS_H__
#define __KMEM_UTILS_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "mt_type.h"

typedef enum {
    KMEM_POOL_TYPE_MODULE,        /* module manager request memory pool */
    KMEM_POOL_TYPE_MODULE_MEMORY, /* module memory request memory pool  */
    KMEM_POOL_TYPE_USR_MEMORY,    /* memory wrap request memory pool    */
    KMEM_POOL_TYPE_LOG,

    KMEM_POOL_TYPE_BUTT
}UTILS_KMEM_POOL_TYPE_E;


typedef struct tagKMemUtils{
    UTILS_KMEM_POOL_TYPE_E enType; /* the pool memory type.                 */

    mt_u32      u32ItemCount;      /* the total numbers of item in pool     */
    mt_u32      u32HasCount;       /* the allocated numbers of item in pool */

    mt_void*    pMemBaseAddr;      /* the base address of pool              */
}kmem_utils_s;


/**
@brief Initialize this memory pool manager module in kernel, which will allocate u32Count ITEMS pool.
@attention Before calling other interfaces of this module, calling this interface.
@param[in] u32Count the request ITEMS number.
@param[in] stMem the item attributes in pool.
@param[out] None
@retval ::handle of the pool with special attributes Success
@retval ::0 Failure
@see \n
N/A
*/
mt_handle kmem_utils_init(mt_u32 u32Count, kmem_utils_s stMem);

/**
@brief Terminate the pool with the special handle.
@attention None.
@param[in] hUtils the pool handle to terminate.
@param[out] None
@retval ::None
@see \n
N/A
*/
mt_void kmem_utils_deinit(mt_handle hUtils);

/**
@brief Request one idle item position from the pool with hUtils.
@attention None.
@param[in] hUtils the pool handle to terminate.
@param[out] None
@retval ::address of the item from pool Success
@retval ::NULL Failure
@see \n
N/A
*/
mt_void* kmem_utils_malloc(mt_handle hUtils);

/**
@brief Release one item position to the pool with hUtils, and make it idle.
@attention None.
@param[in] hUtils the pool handle to terminate.
@param[out] None
@retval ::None
@see \n
N/A
*/
mt_void kmem_utils_free(mt_handle hUtils, mt_void* pAddr);

/**
@brief Get the valid item numbers in pool with hUtils.
@attention None.
@param[in] hUtils the pool handle to terminate.
@param[out] None
@retval ::number of the valid item in pool Success
@retval ::0 Failure
@see \n
N/A
*/
mt_u32 kmem_utils_get_item_no(mt_handle hUtils);

#ifdef __cplusplus
}
#endif

#endif

/** @} */

