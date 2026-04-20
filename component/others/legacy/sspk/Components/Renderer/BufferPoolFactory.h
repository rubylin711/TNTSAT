///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AutoLock.h"
#include "Buffer.h"

// ===============================================================================================================
// ===============================================================================================================

class BufferPoolFactory : public IBufferPoolFactory
{
public:
    //Constructor
    BufferPoolFactory();
    //Destructor
    virtual ~BufferPoolFactory();
    //Retrievs a buffer pool of given type
    __override IBufferPool*    Get(eBufferPoolType region);
    //Retrieves a free buffer from given pool
    __override Buffer*         NewBuffer(eBufferPoolType region);
    //Maximum size of given pool
    __override uint32          PoolSize(eBufferPoolType region);
    //Size of each buffer in the given pool
    __override uint32          Size(eBufferPoolType region);
    //Number of buffers in pool
    __override uint32          Count(eBufferPoolType region);
    //Free buffers in buffer pool
    __override uint32          Free(eBufferPoolType region);

private:
    //Various buffer pools used
    IBufferPool*               _bufferPool[MAX_BUFFERPOOLS];
};

// ===============================================================================================================
// ===============================================================================================================
