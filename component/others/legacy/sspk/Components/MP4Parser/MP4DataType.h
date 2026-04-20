///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include <memory.h>

class Fixed_16
{
public:
    Fixed_16() : _int(0), _flt(0) {}
    Fixed_16(uint8 i, uint8 f) : _int(i), _flt(f) {}
    void SetValue(uint8 i, uint8 f) { _int = i; _flt = f; }
    float ToFloat() const { return _int + (((float)_flt) / 0x100); }

private:
    uint8 _int;
    uint8 _flt;
};

class Fixed_32
{
public:
    Fixed_32() : _int(0), _flt(0) {}
    Fixed_32(uint16 i, uint16 f) : _int(i), _flt(f) {}
    void SetValue(uint16 i, uint16 f) { _int = i; _flt = f; }
    float ToFloat() const { return _int + (((float)_flt) / 0x10000); }
    uint16 Int16() const { return _int; }

private:
    uint16 _int;
    uint16 _flt;
};

class MP4_Guid
{
public:
    MP4_Guid()
        : _data1(0)
        , _data2(0)
        , _data3(0)
    {
        memset(_data4, 0, 8);
    }

    MP4_Guid(uint32 data1, uint16 data2, uint16 data3,
        uint8 data4_0, uint8 data4_1, uint8 data4_2, uint8 data4_3,
        uint8 data4_4, uint8 data4_5, uint8 data4_6, uint8 data4_7)
        : _data1(data1)
        , _data2(data2)
        , _data3(data3)
    {
        _data4[0] = data4_0;
        _data4[1] = data4_1;
        _data4[2] = data4_2;
        _data4[3] = data4_3;
        _data4[4] = data4_4;
        _data4[5] = data4_5;
        _data4[6] = data4_6;
        _data4[7] = data4_7;
    }

    uint32* GetData1Ptr() { return &_data1; }
    uint16* GetData2Ptr() { return &_data2; }
    uint16* GetData3Ptr() { return &_data3; }
    uint8* GetData4Ptr() { return _data4; }

    bool EqualsTo(const MP4_Guid& guid)
    {
        return ((guid._data1 == _data1) &&
            (guid._data2 == _data2) &&
            (guid._data3 == _data3) &&
            (memcmp(guid._data4, _data4, 8) == 0));
    }

private:
    uint32  _data1;
    uint16 _data2;
    uint16 _data3;
    uint8  _data4[8];
};


