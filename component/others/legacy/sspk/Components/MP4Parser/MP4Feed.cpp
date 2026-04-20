///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pkExecutive.h"
#include "Utils.h"
#include "MP4Feed.h"
#include "MarshallingUtils.h"

#include <limits>
#include <stdio.h>

MP4Feed* MP4Feed::CreateMP4Feed(MP4FeedType type, void* source, uint32 length)
{
    MP4Feed* pResult = NULL;

    switch (type)
    {
    case MP4Feed_File:
        pResult = NEW_NO_THROW MP4Feed_file((char*)source);
        CHECK_ALLOC(pResult);
        break;

    case MP4Feed_Memory:
        pResult = NEW_NO_THROW MP4Feed_memory((uint8*)source, length);
        CHECK_ALLOC(pResult);
        break;

    case MP4Feed_Streamer:
        pResult = NEW_NO_THROW MP4Feed_stream((MP4Streamer*)source);
        CHECK_ALLOC(pResult);
        break;

    default: 
        break;
    }

    return pResult;
}

MP4Feed::MP4Feed(void* reader_handle)
    : _buffer(0)
    , _read_pos(BUFFER_LENGTH)
    , _filled_count(BUFFER_LENGTH)
    , _total_read_count(0)
    , _total_read_count_allowed(-1)
    , _reader_handle(reader_handle)
{
}

MP4Feed::~MP4Feed()
{
    if (_buffer)
        delete [] _buffer;
}

template< class T, int32 SIZE >
bool MP4Feed::_ReadIntImpl( _Out_ T* data )
{
    pkCT_ASSERT( std::numeric_limits<T>::is_integer );

    if( _reader_handle == 0 )
    {
        return( false );
    }

    if( RefillBuffer( SIZE ) == false )
    {
        return( false );
    }

    if( ( _filled_count - _read_pos ) < SIZE )
    {
        return( false );
    }

    *data = BigEndian::BytesToHost<T,SIZE>( &_buffer[_read_pos] );

    _read_pos += SIZE;

    return( true );
}

bool MP4Feed::ReadInt8( _Out_ uint8* data)
{
    return( _ReadIntImpl<uint8,1>( data ) );
}

bool MP4Feed::ReadInt16( _Out_ uint16* data)
{
    return( _ReadIntImpl<uint16,2>( data ) );
}

bool MP4Feed::ReadInt24( _Out_ uint32* data)
{
    return( _ReadIntImpl<uint32,3>( data ) );
}

bool MP4Feed::ReadInt32( _Out_ uint32* data)
{
    return( _ReadIntImpl<uint32,4>( data ) );
}

bool MP4Feed::ReadInt64( _Out_ uint64* data)
{
    return( _ReadIntImpl<uint64,8>( data ) );
}

bool MP4Feed::ReadInt64N( _Out_ uint64* data, _In_ int32 cb )
{
    if (_reader_handle == 0)
        return false;

    if (RefillBuffer(cb) == false)
        return false;

    if (_filled_count - _read_pos < cb)
        return false;

    *data = BigEndian::BytesToHost<uint64>( &_buffer[_read_pos], cb );

    _read_pos += cb;

    return true;
}

bool MP4Feed::ReadFixed16( _Out_ Fixed_16* data)
{
    static const int32 s_cbFixed16 = 2; // number of bytes in Reads below
    
    if (_reader_handle == 0)
        return false;

    if (RefillBuffer(s_cbFixed16) == false)
        return false;

    if (_filled_count - _read_pos < s_cbFixed16)
        return false;

    uint8 i = 0;
    uint8 f = 0;
    if (ReadInt8(&i) == false)
        return false;
    if (ReadInt8(&f) == false)
        return false;
    data->SetValue(i, f);

    return true;
}

bool MP4Feed::ReadFixed32( _Out_ Fixed_32* data)
{
    static const int32 s_cbFixed32 = 2 + 2; // number of bytes in Reads below
    
    if (_reader_handle == 0)
        return false;

    if (RefillBuffer(s_cbFixed32) == false)
        return false;

    if (_filled_count - _read_pos < s_cbFixed32)
        return false;

    uint16 i = 0;
    uint16 f = 0;
    if (ReadInt16(&i) == false)
        return false;
    if (ReadInt16(&f) == false)
        return false;
    data->SetValue(i, f);

    return true;
}

bool MP4Feed::ReadGuid(MP4_Guid* data)
{
    static const int32 s_cbGuid = 4 + 2 + 2 + 8; // number of bytes in Reads below 
    
    if (_reader_handle == 0)
        return false;

    if (RefillBuffer(s_cbGuid) == false)
        return false;

    if (_filled_count - _read_pos < s_cbGuid)
        return false;

    if (ReadInt32(data->GetData1Ptr()) == false)
        return false;
    if (ReadInt16(data->GetData2Ptr()) == false)
        return false;
    if (ReadInt16(data->GetData3Ptr()) == false)
        return false;
    if (ReadArray(data->GetData4Ptr(), 8) == false)
        return false;

    return true;
}

bool MP4Feed::ReadArray( _Out_cap_(lenght) uint8* buf, _In_ int32 length)
{
    if (_reader_handle == 0)
        return false;

    int32 count = 0;
    int32 orig_length = length;

    (void)orig_length; // prevent unused warning in release build

    while(length && _filled_count)
    {
        if (RefillBuffer(length) == false)
            return false;

        if (_filled_count-_read_pos >= length)
            count = length;
        else
            count = _filled_count-_read_pos;

        // if there is a buffer, copy the data into it.
        // otherwise, just skip it.
        if (buf)
        {
            memcpy_s(buf, length, _buffer+_read_pos, count);
            buf += count;
        }

        length -= count;
        _read_pos += count;
    }

    if (length)
    {
        TRACE(("ReadArray(%d) failed, %d more bytes to read but we are out of data.\n", orig_length, length));
    }

    return (length == 0);
}

bool MP4Feed::Skip(int32 length)
{
    return ReadArray(0, length);
}

uint32 MP4Feed::RefillBuffer(int32 delta)
{
    if (_reader_handle == 0)
        return false;

    // if eof reached, can't refill but it's not an error
    if (IsEof())
        return true;

    if (_buffer == 0)
    {
        _buffer = NEW_NO_THROW uint8[BUFFER_LENGTH];
        CHECK_ALLOC(_buffer);
        if (_buffer == 0)
            return false;
    }

    // need to refill?
    if ((_total_read_count == _total_read_count_allowed) && (_filled_count == _read_pos))
        return false;

    if (_filled_count - _read_pos < delta)
    {
        // move remaining data in _buffer to the front of _buffer,
        // so we can fill the rest of the space in _buffer
        MoveDataToFront();

        uint32 tmp_filled_count = _filled_count -_read_pos;
        uint32 read_count = BUFFER_LENGTH - tmp_filled_count; // by default, try to fill out the buffer

        // if 'total_read_count_allowed' is not yet set, read only 'delta' bytes
        if ((_total_read_count_allowed == -1) && (delta < (int32)read_count))
            read_count = delta;
        // make sure we don't read more than what is allowed.
        else if (read_count + _total_read_count > _total_read_count_allowed)
            read_count = (uint32)(_total_read_count_allowed - _total_read_count);

        _filled_count = ReadCountedBytes(_buffer+_filled_count-_read_pos, read_count);
        _total_read_count += _filled_count;
        _filled_count += tmp_filled_count;
        _read_pos = 0;
    }

    return true;
}

void MP4Feed::MoveDataToFront()
{
    if (_filled_count > _read_pos)
        memmove(_buffer, _buffer+_read_pos, _filled_count-_read_pos);
}

void MP4Feed::SetStopOffset(uint32 atom_length)
{
    _total_read_count_allowed = _total_read_count - (_filled_count - _read_pos) - 8 + atom_length;
}

MP4Feed_stream::MP4Feed_stream(MP4Streamer* mp4_stream)
    : MP4Feed(mp4_stream)
{
}

MP4Feed_stream::~MP4Feed_stream()
{
}

bool MP4Feed_stream::IsEof()
{
    return false;
}

uint32 MP4Feed_stream::ReadCountedBytes(uint8* buffer, uint32 length)
{
    MP4Streamer* mp4_stream = (MP4Streamer*)_reader_handle;
    CHECK_ALLOC(mp4_stream);

    int32 bytes = mp4_stream->RecvCount(buffer, length, length);

    return bytes < 0 ? 0 : bytes;
}

MP4Feed_file::MP4Feed_file(const char* filename)
    : MP4Feed(0)
{
    //TODO: win32 fopen can't open a file larger than 4GB,
    FILE *handle = NULL;
    fopen_s(&handle, filename, "rb");

    _reader_handle = (void *)handle;
}

MP4Feed_file::~MP4Feed_file()
{
    if (_reader_handle)
        fclose((FILE*)_reader_handle);
}

bool MP4Feed_file::Skip(int32 length)
{
    return MP4Feed::Skip(length);
}

bool MP4Feed_file::IsEof()
{
    if (_reader_handle == 0)
        return true;

    return (feof((FILE*)_reader_handle) != 0);
}

uint32 MP4Feed_file::ReadCountedBytes(uint8* buffer, uint32 length)
{
    return fread(buffer, 1, length, (FILE*)_reader_handle);
}


MP4Feed_memory::MP4Feed_memory(uint8* data, uint32 data_len)
    : MP4Feed(data)
{
    Init(data, data_len);
}

MP4Feed_memory::~MP4Feed_memory()
{
    // reset _buffer pointer so base class won't try to delete it
    _buffer = 0;
}

void MP4Feed_memory::MoveDataToFront()
{
    // simply move the _buffer pointer
    _buffer += _read_pos;
}

bool MP4Feed_memory::IsEof()
{
    if (_buffer)
    {
        if (_total_read_count >= _data_len)
            return true;
    }

    return false;
}

uint32 MP4Feed_memory::ReadCountedBytes(uint8* buffer, uint32 length)
{
    // simply return length
    return length;
}

void MP4Feed_memory::Init(uint8* data, uint32 data_len)
{
    // _buffer is passed in
    _buffer = data - _read_pos;
    // _buffer's total length
    _data_len = data_len;
}


