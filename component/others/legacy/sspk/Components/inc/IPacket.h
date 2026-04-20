///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// ===============================================================================================================
// Base class container to handle various type of data flow from sockets to receiver...
// ===============================================================================================================

class IPacket
{
public:
    enum ePacketType
    {
        kPacketType_Invalid = -1,
        kPacketType_Raw = 0,
    };

    IPacket(ePacketType type)
        : PacketType(type)
        , StreamId(0)
        , FilePosition(0)
    {
        //Use private init call - virtual functions cannot be called from constructor
        Init_IPacket();
    }

    virtual ~IPacket()
    {}

    virtual void Init()
    {
        Init_IPacket();
    }

    bool IsType(ePacketType type) const { return type == PacketType; }

private:
    //Only members that need to be reset for every packet
    void Init_IPacket()
    {
        Data = NULL;
        DataLength = 0;

        Flags = 0;
        Rap = false;

        TimeStamp = 0;
        PCR = 0;
        NTP = INVALID_TIME;
        NPT = INVALID_TIME;
        PCRForDvr = INVALID_TIME;
        NTPForDvr = INVALID_TIME;

        FilePosition = 0;
    }

public:
    ePacketType PacketType;
    int32       StreamId;

    byte*       Data;
    int32       DataLength;

    uint32      Flags;
    bool        Rap;

    uint32      TimeStamp;
    uint64      PCR;       //90 KHz (Program Clock Reference units)
    uint64      NTP;       //32.32 fixed point seconds (NTP units)
    uint64      NPT;       //90KHz unit used for RTP VODs
    uint64      PCRForDvr; //90 KHz (PCR units)
    uint64      NTPForDvr; //32-32 fix int (NTP units)

    int64        FilePosition;
};

// ===============================================================================================================
// ===============================================================================================================
