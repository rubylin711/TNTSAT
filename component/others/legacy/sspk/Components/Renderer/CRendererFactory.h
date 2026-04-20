///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/// <summary>
///     CRendererFactory.h
///     Provides a sample renderer factory implementation
/// </summary>

#pragma once

#include "IAVManager.h"
#include "IRenderer.h"
#include "CRendererRaw.h"

// ===============================================================================================================
// ===============================================================================================================

class CRendererFactory : public IRendererFactory
{
public:
    //Constructor
    CRendererFactory(IAVManager* avManager) : mAVManager(avManager) {}
    //Destructor
    virtual ~CRendererFactory() {}
    //Retrieve an instance of a renderer which can handle given packet
    __override IRenderer* AcquireRenderer(IPacket& packet, IReceiverControl* receiverControl)
    {
        switch (packet.PacketType)
        {
        case IPacket::kPacketType_Raw:
            {
                return NEW_NO_THROW CRendererRaw(receiverControl);
            }
            break;

        default:
            ASSERT(false);
            break;
        }
        return NULL;
    }


private:
    //Factiory of factories
    IAVManager* mAVManager;
};

// ===============================================================================================================
// ===============================================================================================================
