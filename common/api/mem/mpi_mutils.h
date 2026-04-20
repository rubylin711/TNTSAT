

#ifndef __MEM_UTILS_H__
#define __MEM_UTILS_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "mt_type.h"

typedef enum {
    MEM_POOL_TYPE_MODULE,        /* module manager request memory pool */
    MEM_POOL_TYPE_MODULE_MEMORY, /* module memory request memory pool  */
    MEM_POOL_TYPE_USR_MEMORY,    /* memory wrap request memory pool     */
    MEM_POOL_TYPE_MMZ,
    MEM_POOL_TYPE_LOG,

    MEM_POOL_TYPE_BUTT
}UTILS_MEM_POOL_TYPE_E;


typedef struct tagMemUtils{
    UTILS_MEM_POOL_TYPE_E enType;

    mt_u32                u32ItemCount;
    mt_u32                u32HasCount;

    mt_void*              pMemBaseAddr;
}mem_utils_s;

/**
@brief Initialize this utils module, which will alloc u32Count stMem ITEMS.
@attention Before calling other interfaces of this module, calling this interface.
@param[in] u32Count the request ITEMS number.
@param[out] None
@retval ::utils handle Success
@retval ::0 Failure
@see \n
N/A
*/
mt_handle mem_utils_init(mt_u32 u32Count, mem_utils_s stMem);

/**
@brief Terminate this module.
@attention N/A
@param[in] hUtils The handle returned by Init.
@param[out] None
@retval ::None.
@see \n
N/A
*/
mt_void   mem_utils_deinit(mt_handle hUtils);


/**
@brief Request one position by hUtils from the memory.
@attention N/A
@param[in] hUtils The handle returned by Init.
@param[out] None
@retval ::None.
@see \n
N/A
*/
mt_void* mem_utils_malloc(mt_handle hUtils);

/**
@brief Release one position by hUtils to the memory.
@attention N/A
@param[in] hUtils The handle returned by Init.
@param[out] None
@retval ::None.
@see \n
N/A
*/
mt_void  mem_utils_free(mt_handle hUtils, mt_void* pAddr);


/**
@brief Get valid item number in the memory pool by hUtils.
@attention N/A
@param[in] hUtils The handle returned by Init.
@param[out] None
@retval ::Total valid item number success.
@retval ::0 failure.
@see \n
N/A
*/
mt_u32   mem_utils_get_item_no(mt_handle hUtils);

#ifdef __cplusplus
}
#endif

#endif

