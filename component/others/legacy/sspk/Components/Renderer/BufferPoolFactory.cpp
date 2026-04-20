///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BufferPoolFactory.h"
#include "BufferPool.h"

// ===============================================================================================================
// ===============================================================================================================

//Fullscreen Video buffers
#define VIDEO_POOL_MAX_BUFFERS              16
#define VIDEO_POOL_BUFFERSIZE               32*1024

//PIP video buffers
#define PIP_POOL_MAX_BUFFERS                32
#define PIP_POOL_BUFFERSIZE                 1536

//Audio buffers - supports upto 2MBytes across all audio streams
#define AUDIO_POOL_MAX_BUFFERS              2048//1024  modify for audio raw write failed
#define AUDIO_POOL_BUFFERSIZE               2048

//Subtitles buffers
//Should be 10K according to spec but set to 16K as the puppetry DR test stream exceeds 10K
#define SUBTITLES_POOL_MAX_BUFFERS          5
#define SUBTITLES_POOL_BUFFERSIZE           16*1024

//Generic Pool (256K for SI data)
#define GENERIC_POOL_MAX_BUFFERS            64
#define GENERIC_POOL_BUFFERSIZE             4096

//Private PES Data (368K for TS packet data - 4*184 = 4 TS packets)
#define PRIVATE_PES_DATA_POOL_MAX_BUFFERS   516
#define PRIVATE_PES_DATA_POOL_BUFFERSIZE    736

// ===============================================================================================================
// Collect all PES globals and allocations into this object
// ===============================================================================================================

BufferPoolFactory::BufferPoolFactory()
{
    //Fullscreen Video
    _bufferPool[BUFFERPOOL_VIDEO] = new BufferPool(VIDEO_POOL_MAX_BUFFERS, VIDEO_POOL_BUFFERSIZE);
    CHECK_ALLOC(_bufferPool[BUFFERPOOL_VIDEO]);

    //PIP Video
    _bufferPool[BUFFERPOOL_PIPVIDEO] = new BufferPool(PIP_POOL_MAX_BUFFERS, PIP_POOL_BUFFERSIZE);
    CHECK_ALLOC(_bufferPool[BUFFERPOOL_PIPVIDEO]);

    //Audio
    _bufferPool[BUFFERPOOL_AUDIO] = new BufferPool(AUDIO_POOL_MAX_BUFFERS, AUDIO_POOL_BUFFERSIZE);
    CHECK_ALLOC(_bufferPool[BUFFERPOOL_AUDIO]);

    //Generic
    _bufferPool[BUFFERPOOL_GENERIC] = new BufferPool(GENERIC_POOL_MAX_BUFFERS, GENERIC_POOL_BUFFERSIZE);
    CHECK_ALLOC(_bufferPool[BUFFERPOOL_GENERIC]);

    //Private PES (SI)
    _bufferPool[BUFFERPOOL_PRIVPESDATA] = new BufferPool(PRIVATE_PES_DATA_POOL_MAX_BUFFERS, PRIVATE_PES_DATA_POOL_BUFFERSIZE);
    CHECK_ALLOC(_bufferPool[BUFFERPOOL_PRIVPESDATA]);

    //Subtitles
    _bufferPool[BUFFERPOOL_SUBTITLES] = new BufferPool(SUBTITLES_POOL_MAX_BUFFERS, SUBTITLES_POOL_BUFFERSIZE);
    CHECK_ALLOC(_bufferPool[BUFFERPOOL_SUBTITLES]);
}

BufferPoolFactory::~BufferPoolFactory()
{
    for (int i = 0; i < MAX_BUFFERPOOLS; i++)
    {
        if (_bufferPool[i])
        {
            delete _bufferPool[i];
            _bufferPool[i] = NULL;
        }
    }
}

IBufferPool* BufferPoolFactory::Get(eBufferPoolType region)
{
    ASSERT(region < MAX_BUFFERPOOLS);
    if (region < MAX_BUFFERPOOLS)
    {
        ASSERT(_bufferPool[region]);
        if (_bufferPool[region])
        {
            return _bufferPool[region];
        }
    }
    return NULL;
}

Buffer* BufferPoolFactory::NewBuffer(eBufferPoolType region)
{
    IBufferPool* pool = Get(region);
    return pool ? pool->Get() : NULL;
}

uint32 BufferPoolFactory::PoolSize(eBufferPoolType region)
{
    IBufferPool* pool = Get(region);
    return pool ? pool->PoolSize() : 0;
}

uint32 BufferPoolFactory::Size(eBufferPoolType region)
{
    IBufferPool* pool = Get(region);
    return pool ? pool->Size() : 0;
}

uint32 BufferPoolFactory::Count(eBufferPoolType region)
{
    IBufferPool* pool = Get(region);
    return pool ? pool->Count() : 0;
}

uint32 BufferPoolFactory::Free(eBufferPoolType region)
{
    IBufferPool* pool = Get(region);
    return pool ? pool->Free() : 0;
}

// ===============================================================================================================
// ===============================================================================================================
