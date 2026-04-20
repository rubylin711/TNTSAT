///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/// <summary>
///     IPTVPhysMemMgr.h - HAL specification 1.2
///        This file contains definition of the physical memory manager APIs to be implemented by
///        the mediaprocessor silicon vendor in order to interface with Microsoft MediaRoom.
/// </summary>

#pragma once

#include "IPTVError.h"

#ifndef __cplusplus
// bool is not defined in the "C" case. bool is 1 byte in the C++ case.
typedef char bool;
#endif

/// <topic name="DataStructs" displayname="Physical memory manager Data Structures"> </topic>
/// <summary>
/// HAL buffer descriptor flags. A collection of flags (bit fields) that can be
/// associated with a buffer in an IPTV_HAL_BUFFER.
/// </summary>
typedef enum _IPTV_HAL_BUFFER_FLAGS
{
      IPTV_HAL_BUFFER_FLAG_DECRYPT = 1,
      IPTV_HAL_BUFFER_FLAG_ENDFRAME = 2,
      IPTV_HAL_BUFFER_FLAG_DISCONTINUITY = 4,
} IPTV_HAL_BUFFER_FLAGS;

/// <summary>
/// buffer descriptor list
/// </summary>
typedef struct _IPTV_HAL_BUFFER
{
      PUCHAR                pBuf;          // Pointer to base of this buffer
      UINT32                u32Size;       // (actually the size of the buffer)
      UINT32                reserved1;     // (actually the mark)
      struct _IPTV_HAL_BUFFER*     pNext;         // Next buffer in chain
      UINT32                u32DataStart;  // Offset of the start of the data area in the buffer
      UINT32                u32DataEnd;    // Offset of the byte following the end of the data area
      IPTV_HAL_BUFFER_FLAGS u32Flags;      // flag values qualifying the data (encrypted, endFrame etc.)
}  IPTV_HAL_BUFFER, *PIPTV_HAL_BUFFER;

/// <summary>
/// Physical memory allocation type
/// </summary>
/// <remarks>
/// Some of the subsystems in the IPTV client requires contiguous physical memory for performance reasons. The enums below indicate which subsystem is requesting memory and which memory bank (in the case of split memory bank systems) it prefers the allocation from.
/// </remarks>

typedef enum _IPTV_HAL_PHYSMEM_TYPE
{
    IPTV_HAL_PHYSMEM_IDE=0,
    IPTV_HAL_PHYSMEM_AV,
    IPTV_HAL_PHYSMEM_GRAPHICS,
    IPTV_HAL_PHYSMEM_LAST,
}  IPTV_HAL_PHYSMEM_TYPE;

/// <summary>
/// Physical memory address (PHYSICAL_ADDRESS)
/// </summary>
/// <remarks>
/// This is essentially the same as PHYSICAL_ADDRESS. This structure is just copied verbatim from winnt.h so we dont
/// have to include windows.h
/// </remarks>
typedef union _IPTV_HAL_PHYSMEM_ADDRESS {
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} IPTV_HAL_PHYSMEM_ADDRESS, *PIPTV_HAL_PHYSMEM_ADDRESS;

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
iptv_hal_error  IPTV_HAL_Physmem_Alloc(IN IPTV_HAL_PHYSMEM_TYPE type,
                                       IN UINT32 size,
                                       OUT PVOID *ppContiguousMemory);


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
iptv_hal_error  IPTV_HAL_Physmem_Free(IN PVOID pContiguousMemory);
