///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

// Implementation of IPTVPhysMemMgr

// This emulates using standard system memory heap
// Embedded systems will need to use hardware regions

#include "pkPAL.h"
#include "pkExecutive.h"

#include "IPTVPhysMemMgr.h"

extern "C" void pkAPI DebugPrint( const char *pFmt, ... );

// base HAL TRACE on the same condition as PRINTMSG
#ifdef PRINTMSG_ENABLED

#define TRACE( printf_exp ) ( DebugPrint printf_exp )

#else // PRINTMSG_ENABLED

#define TRACE( printf_exp )   ( void ) 0

#endif // PRINTMSG_ENABLED

/// <topic name="APIs" displayname="Physical memory manager APIs"> </topic>

/// <summary>
/// This function allocates contiguous physical memory.
/// </summary>
/// <param name="type">[IN] This specifies which subsystem is requesting contiguous physical memory and which memory bank (in the case of split memory bank systems) it prefers the allocation from </param>
/// <param name="size">[IN] This specifies the number of bytes to allocate </param>
/// <param name="ppContiguousMemory">[OUT] returns a pointer to the contiguous physical memory that was allocated and addressable by the CPU. The returned address should be in the uncached memory space. It is also desirable to return an address that doesn't go through the MMU to avoid TLB misses (For example, in the case of MIPS, this is ideally a KSEG1 address). If memory could not be allocated, this should be set to NULL </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The memory allocation failed</para>
/// </returns>
/// <remarks>
/// </remarks>
iptv_hal_error  IPTV_HAL_Physmem_Alloc
    (
        IN IPTV_HAL_PHYSMEM_TYPE type,
        IN UINT32                size,
        OUT PVOID*               ppContiguousMemory
    )
{
    if (NULL == ppContiguousMemory)
        return IPTV_HAL_ERROR_INVALID_PARAMETER;

    *ppContiguousMemory = Executive_Alloc( size, TRUE );

    if (NULL == *ppContiguousMemory)
        return IPTV_HAL_ERROR_OUT_OF_MEMORY;

    return IPTV_HAL_ERROR_SUCCESS;
}


/// <summary>
/// This function frees contiguous physical memory previously allocated with IPTV_HAL_Physmem_Alloc.
/// </summary>
/// <param name="pContiguousMemory">[IN] This specifies the pointer to the contiguous physical memory block to free  </param>
/// <returns>
/// <para>IPTV_HAL_ERROR_SUCCESS: No Errors</para>
/// <para>IPTV_HAL_ERROR_FAILED: The memory release failed</para>
/// </returns>
/// <remarks>
/// </remarks>
iptv_hal_error  IPTV_HAL_Physmem_Free(IN PVOID pContiguousMemory)
{
    Executive_Free(pContiguousMemory);
    return IPTV_HAL_ERROR_SUCCESS;
}
