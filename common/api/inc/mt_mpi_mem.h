
#ifndef __MT_MPI_MEM_H__
#define __MT_MPI_MEM_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "mt_type.h"

/** @addtogroup H_MPI_MEM */
/** @{ */

/**
@brief malloc the pointed size from system heap.
@attention None
@param[in] u32ModuleID The module ID, who need to request memory.
@param[in] u32Size The size of requesting.
@param[out] None
@retval ::Valid memory address Success
@retval ::NULL Failure
@see \n
N/A
*/
mt_void* mt_malloc(mt_u32 u32ModuleID, mt_u32 u32Size);


/**
@brief Free the requsted memory by hi_malloc.
@attention when stopping to use the memory, calling this interface.
@param[in] u32ModuleID The module ID, who need to free memory.
@param[in] pMemAddr The memory address to free
@param[out] None
@retval ::None
@see \n
N/A
*/
mt_void mt_free(mt_u32 u32ModuleID, mt_void* pMemAddr);

/**
@brief Calloc memory, with u32MemBlock blocks and u32Size size per.
@attention None
@param[in] u32ModuleID The module id, who need to calloc memory.
@param[in] u32MemBlock The requesting block number.
@param[in] u32Size The requesting size per block.
@param[out] None
@retval ::Valid memory address Success
@retval ::NULL Failure
@see \n
N/A
*/
mt_void* mt_calloc(mt_u32 u32ModuleID, mt_u32 u32MemBlock, mt_u32 u32Size);

/**
@brief Realloc memory
@attention None
@param[in] pMemAddr The memory address, which has been requested from system heap.
@param[in] u32Size The newer memory size to request.
@param[out] None
@retval ::Valid memory address Success
@retval ::NULL Failure
@see \n
N/A
*/
mt_void* mt_realloc(mt_u32 u32ModuleID, mt_void *pMemAddr, mt_u32 u32Size);


mt_void *mt_mmap(phys_addr_t phy_addr, ulong size);
mt_void *mt_mmap_cache(phys_addr_t phy_addr, ulong size);
mt_s32 mt_munmap(void *addr_mapped);
mt_s32 mt_flush(void *virtaddr, ulong size);
mt_s32 mt_invalidate(void *virtaddr, ulong size);
mt_s32 mt_get_pageinfo(mt_u32 *page_block_order, mt_u32 *pages_per_block);
int mt_get_phys_addr(void *vaddr, phys_addr_t *phy_addr);
int mt_walk_watch(void *user_addr, u32 size, u64 right_value);

/** @} */

#ifdef __cplusplus
}
#endif

#endif



