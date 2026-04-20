///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BufferPool.h"
#include "IPTVPhysMemMgr.h"

// ===============================================================================================================
// ===============================================================================================================

BufferPool::BufferPool(uint32 count, uint32 size)
    : _count(count)
    , _size(size)
{
    //Both our pool area and buffer list come from the HAL.
    //The HAL should own this memory space

    //Pool Area
    IPTV_HAL_Physmem_Alloc(IPTV_HAL_PHYSMEM_AV, size * count, (LPVOID*)&_data);
    //This can come back as NULL memory when we do not want to allocate memory for secure regions.
    //The Decoder HAL allocates memory in such cases, and we re-use the _buffer region to manage
    //the secure memory allocated by the decoder HAL
    ASSERT(_data);
    CHECK_ALLOC(_data);

    //Block of buffers
    IPTV_HAL_Physmem_Alloc(IPTV_HAL_PHYSMEM_AV, count * sizeof(Buffer), (LPVOID*)&_buffers);
    //Assert or cause a crash if this allocation fails.
    ASSERT(_buffers);
    CHECK_ALLOC(_buffers);

    //Now initialize the buffer pool
    byte* data = _data;
    for (uint32 i = 0; i < count; i++)
    {
        _buffers[i].HALBuffer.pBuf = data;
        _buffers[i].HALBuffer.u32Size = size;
        _buffers[i].Mark = 0;
        _buffers[i].Next = NULL;
        _buffers[i].HALBuffer.u32DataStart = 0;
        _buffers[i].HALBuffer.u32DataEnd = 0;
        _buffers[i].HALBuffer.u32Flags = (IPTV_HAL_BUFFER_FLAGS) 0;

        _buffers[i]._pool = this;
        if (i != count-1)
            _buffers[i]._freelist = _buffers + i + 1;
        else
            _buffers[i]._freelist = NULL;

        data += size;
    }
    _freelist = _buffers;
    _freeCount = _count;
}

BufferPool::~BufferPool()
{
    if (_buffers)
    {
        IPTV_HAL_Physmem_Free(_buffers);
    }
    if (_data)
    {
        IPTV_HAL_Physmem_Free(_data);
    }
}

void BufferPool::Release(Buffer* buffer)
{
    AutoLock lock(&_lock);

    //Sanity checks
    CHECK_ALLOC(buffer);
    //Double release?
    ASSERT(buffer->_freelist == NULL);

    //Update free list
    buffer->_freelist = _freelist;
    _freelist = buffer;

    //Clear the buffer
    buffer->Mark = 0;
    buffer->HALBuffer.u32DataStart = 0;
    buffer->HALBuffer.u32DataEnd = 0;
    buffer->HALBuffer.u32Flags = (IPTV_HAL_BUFFER_FLAGS) 0;

    _freeCount++;
}

Buffer* BufferPool::Get(void)
{
    AutoLock lock(&_lock);

    if (_freelist == NULL)
    {
        ASSERT(_freeCount == 0);
        return NULL;
    }

    Buffer* buffer = _freelist;
    _freelist = _freelist->_freelist;
    buffer->_freelist = NULL;

    buffer->Next = NULL;
    buffer->HALBuffer.pNext = NULL;
    ASSERT(buffer->Mark == 0);

    ASSERT(_freeCount > 0);
    _freeCount--;
    return buffer;
}

// ===============================================================================================================
// ===============================================================================================================
