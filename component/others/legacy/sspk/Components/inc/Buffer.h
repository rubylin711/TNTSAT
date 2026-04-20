///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "IPTVPhysMemMgr.h"

// ===============================================================================================================
// ===============================================================================================================

enum eBufferPoolType
{
    BUFFERPOOL_VIDEO = 0,
    BUFFERPOOL_PIPVIDEO = 1,
    BUFFERPOOL_AUDIO = 2,
    BUFFERPOOL_GENERIC = 3,
    BUFFERPOOL_PRIVPESDATA = 4,
    BUFFERPOOL_SUBTITLES = 5,
    MAX_BUFFERPOOLS = 6
};

#define BUFFER_ENCRYPTED              IPTV_HAL_BUFFER_FLAG_DECRYPT
#define BUFFER_ENDFRAME               IPTV_HAL_BUFFER_FLAG_ENDFRAME
#define BUFFER_DISCONTINUITY          IPTV_HAL_BUFFER_FLAG_DISCONTINUITY

// ===============================================================================================================
// ===============================================================================================================

class IBufferPool;
class Buffer;

class IBufferPoolFactory
{
public:
    //Constructor
    IBufferPoolFactory() {}
    //Destructor
    virtual ~IBufferPoolFactory() {}
    //Retrievs a buffer pool of given type
    virtual IBufferPool*   Get(eBufferPoolType region) = 0;
    //Retrieves a free buffer from given pool
    virtual Buffer*        NewBuffer(eBufferPoolType region) = 0;
    //Maximum size of given pool
    virtual uint32         PoolSize(eBufferPoolType region) = 0;
    //Size of each buffer in the given pool
    virtual uint32         Size(eBufferPoolType region) = 0;
    //Number of buffers in pool
    virtual uint32         Count(eBufferPoolType region) = 0;
    //Free buffers in buffer pool
    virtual uint32         Free(eBufferPoolType region) = 0;
};

// ===============================================================================================================
// ===============================================================================================================

class Buffer;

class IBufferPool
{
public:
    //Constructor
    IBufferPool() {}
    //Destructor
    virtual ~IBufferPool() {}
    //Release a buffer back to pool
    virtual void           Release(Buffer* buffer) = 0;
    //Retrieve a buffer from pool
    virtual Buffer*        Get(void) = 0;
    //Maximum pool size as allocated
    virtual uint32         PoolSize(void) const = 0;
    //Size of each buffer in pool
    virtual uint32         Size(void) const = 0;
    //Total number of buffers in pool
    virtual uint32         Count(void) const = 0;
    //Number of buffers available in pool
    virtual uint32         Free(void) const = 0;
};

// ===============================================================================================================
// ===============================================================================================================

class Buffer
{
public:
    //Release given list of buffers back to their pool
    static Buffer*         ReleaseChain(Buffer* buffer);

    //Space left in buffer
    int                    Space(void) const { return HALBuffer.u32Size - Mark; }

    //Flag get/set APIs
    void SetFlag(IPTV_HAL_BUFFER_FLAGS bufferFlag) 
    { 
        HALBuffer.u32Flags = (IPTV_HAL_BUFFER_FLAGS)(HALBuffer.u32Flags | bufferFlag); 
    }
    void ClearFlag(IPTV_HAL_BUFFER_FLAGS bufferFlag) 
    { 
        HALBuffer.u32Flags = (IPTV_HAL_BUFFER_FLAGS)(HALBuffer.u32Flags & ~bufferFlag); 
    }
    bool GetFlag(IPTV_HAL_BUFFER_FLAGS bufferFlag) 
    { 
        return 0 != (HALBuffer.u32Flags & bufferFlag); 
    }

    int Write(const byte* ptr, int count)
    {
        if (count < 1)
        {
            return 0;
        }
        //Copy only to end of buffer (or count bytes if that is less)
        int toWrite = (int)(HALBuffer.u32Size - Mark); 
        if (count < toWrite)
        {
            toWrite = count;
        }

        memcpy_s(&HALBuffer.pBuf[Mark], (size_t)toWrite, ptr, (size_t)toWrite);
        Mark += toWrite;

        return toWrite;
    }

public:
    //Next is a Buffer object forward link
    Buffer*             Next;
    //Write offset mark within buffer
    int                 Mark;

    //The HAL buffer wrapped by Buffer
    IPTV_HAL_BUFFER     HALBuffer;
    // Note: Both the Buffer class and the IPTV_HAL_BUFFER structure it wraps
    //       contain forward links and both are maintained because the HAL code 
    //       only has visibility of the IPTV_HAL_BUFFER link.

protected:
    friend class BufferPool;
    
    //Next free buffer when waiting in pool
    Buffer*             _freelist;
    //Issuing pool
    IBufferPool*        _pool;

private:

    //Releases buffer back to the Issuing pool
    void                   Release(void) { _pool->Release(this); }
};

// ===============================================================================================================
// ===============================================================================================================

class BufferChain
{
public:
    BufferChain()
        : _head(NULL)
        , _tail(NULL)
    {
    }

    void SetBuffers( _In_ Buffer* pBuffer )
    {
        if( _head != pBuffer )
        {
            _head = _tail = pBuffer;

            while( pBuffer != NULL )
            {
                _tail = pBuffer;
                pBuffer = _tail->Next;
            }
        }
    }

    BufferChain& operator = (const BufferChain& chain)
    {
        _head = chain._head;
        _tail = chain._tail;
        return *this;
    }

    Buffer* HeadBuffer()
    {
        return _head;
    }

    void Release()
    {
        if (_head)
        {
            Buffer::ReleaseChain(_head);
            _head = NULL;
        }

        _tail = NULL;
    }

    void Add(Buffer* buffer)
    {
        CHECK_ALLOC(buffer);
        if (_tail)
        {
            _tail->Next = buffer;
            _tail->HALBuffer.pNext = &(buffer->HALBuffer);
        }
        else
            _head = buffer;

        _tail = buffer;
        while (_tail->Next)
            _tail = _tail->Next;
    }
    
    void SetEndFrame()
    {
        if (_head && _tail)
        {
            _tail->SetFlag(BUFFER_ENDFRAME);
        }
    }

    bool IsEndFrame()
    {
        return _tail ? _tail->GetFlag(BUFFER_ENDFRAME) : false;
    }

    uint32 GetTotalSize(uint32 *bufferCount)
    {
        uint32 count = 0;
        uint32 size = 0;

        Buffer* buffer = _head;
        while (buffer)
        {
            count++;
            size += buffer->Mark - buffer->HALBuffer.u32DataStart;
            buffer = buffer->Next;
        }

        if (bufferCount) *bufferCount = count;
        return size;
    }

    Buffer* GetChainFirst() { return _head; }
    Buffer* GetChainLast() { return _tail; }

    bool IsEmpty() { return _head == NULL; }

protected:
    Buffer* _head;
    Buffer* _tail;
};

// ===============================================================================================================
// ===============================================================================================================
