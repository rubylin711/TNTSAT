///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// ===============================================================================================================
// Picture User Callback handler
// Provides a container for all Picture User Data subscriber handling
// ===============================================================================================================

class IReceiverControl;
class CStreamInfo;

class IPUDHandler
{
public:
    //Factory API to instantiate Picture User Callback handler
    static IPUDHandler* Create(IReceiverControl* receiverControl, const CStreamInfo& si, LPVOID decoderContext);

public:
    //Destructor
    virtual ~IPUDHandler() {}
    //Trigger service registration/unregistration
    virtual void EnableTriggers(bool enable) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
