///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// ===============================================================================================================
// IPacketMonitor API
//
// For monitoring packet for special processing
// ===============================================================================================================

class IPacket;

class IPacketMonitor
{
public:
    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~IPacketMonitor() {}
    /// <summary>
    /// Monitors incoming packets
    /// </summary>
    virtual void  ProcessPacket(IPacket& packet) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
