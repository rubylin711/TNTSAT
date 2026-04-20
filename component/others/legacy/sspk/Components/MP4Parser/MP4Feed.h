///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MP4DataType.h"

class MP4Feed
{
public:
    enum MP4FeedType
    {
        MP4Feed_None = 0,
        MP4Feed_File,     // mp4 data is read from file
        MP4Feed_Memory,   // mp4 data is provided by a memory array
        MP4Feed_Streamer  // mp4 data is provided by a MP4Streamer interface
    };

    static const int32 BUFFER_LENGTH = 2048;

    static MP4Feed* CreateMP4Feed(MP4FeedType type, _Inout_bytecap_(length) void* source, uint32 length);

    MP4Feed(void* reader_handle);
    virtual ~MP4Feed();

    virtual bool ReadInt8( _Out_ uint8* data );
    virtual bool ReadInt16( _Out_ uint16* data );
    virtual bool ReadInt24( _Out_ uint32* data );
    virtual bool ReadInt32( _Out_ uint32* data );
    virtual bool ReadInt64( _Out_ uint64* data );
    virtual bool ReadInt64N( _Out_ uint64* data, _In_ int32 cb );
    virtual bool ReadFixed16( _Out_ Fixed_16* data);
    virtual bool ReadFixed32( _Out_ Fixed_32* data);
    virtual bool ReadGuid( _Out_ MP4_Guid* data);
    virtual bool ReadArray( _Out_cap_(length) uint8* data, _In_ int32 length);
    virtual bool Skip(int32 length);
    virtual uint32 RefillBuffer(int32 delta);
    virtual void MoveDataToFront();

    virtual bool IsEof() = 0;
    virtual uint32 ReadCountedBytes(uint8* buffer, uint32 length) = 0;

    uint32 TotalReadBytes() const { return (uint32)_total_read_count; }
    void SetStopOffset(uint32 atom_length);
    uint8* GetCurrentPos() const { return (_buffer + _read_pos); }

protected:

    template< typename T, int32 SIZE >
    bool _ReadIntImpl( _Out_ T* data );

    uint8* _buffer;
    int32 _read_pos;
    int32 _filled_count;
    int64 _total_read_count;
    int64 _total_read_count_allowed;

    void* _reader_handle;
};

class MP4Streamer
{
public:
    virtual ~MP4Streamer() {};
    virtual int32 RecvCount( _Out_cap_(dstlen) byte* dst, int32 dstlen, int32 dataLength, int32 timeout = 0) = 0;
};

class MP4Feed_stream : public MP4Feed
{
public:
    MP4Feed_stream(MP4Streamer* mp4_stream);
    ~MP4Feed_stream();

    __override bool IsEof();
    uint32 ReadCountedBytes( _Out_cap_(length) uint8* buffer, uint32 length);
};

class MP4Feed_file : public MP4Feed
{
public:
    MP4Feed_file(const char* filename);
    ~MP4Feed_file();

    __override bool Skip(int32 length);
    __override bool IsEof();
    uint32 ReadCountedBytes( _Out_cap_(length) uint8* buffer, uint32 length);
};

class MP4Feed_memory : public MP4Feed
{
public:
    MP4Feed_memory( _Inout_bytecap_(data_len) uint8* data, uint32 data_len);
    ~MP4Feed_memory();

    __override void MoveDataToFront();
    __override bool IsEof();
    uint32 ReadCountedBytes(uint8* buffer, uint32 length);

    void Init( _Inout_bytecap_(data_len) uint8* data, uint32 data_len);

protected:
    uint32 _data_len;
};
