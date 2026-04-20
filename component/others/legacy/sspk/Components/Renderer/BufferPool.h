///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Buffer.h"
#include "AutoLock.h"

// ===============================================================================================================
// ===============================================================================================================

class BufferPool : public IBufferPool
{
public:
    //Constructor
    BufferPool(uint32 count, uint32 size);
    //Destructor
    virtual ~BufferPool();
    //Release a buffer back to pool
    __override void        Release(Buffer* buffer);
    //Retrieve a buffer from pool
    __override Buffer*     Get(void);
    //Maximum pool size as allocated
    __override uint32      PoolSize(void) const { return _size * _count; }
    //Size of each buffer in pool
    __override uint32      Size(void) const { return _size; }
    //Total number of buffers in pool
    __override uint32      Count(void) const { return _count; }
    //Number of buffers available in pool
    __override uint32      Free(void) const { return _freeCount; }

protected:
    //Total # of buffers in pool
    uint32                 _count;
    //Size of each buffer
    uint32                 _size;
    //Physical mem
    byte*                  _data;
    //Block of buffers
    Buffer*                _buffers;
    //Next buffer in chain
    Buffer*                _freelist;
    //Free buffers waiting in pool
    uint32                 _freeCount;
    //Protecting the buffer pool
    Lockable               _lock;
};

// ===============================================================================================================
// ===============================================================================================================
