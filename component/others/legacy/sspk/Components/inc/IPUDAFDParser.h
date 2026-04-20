///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IPUDCallback.h"

// ===============================================================================================================
// Picture User Data Active Format Descriptor (AFD) Parser
// ===============================================================================================================

class IReceiverControl;

class IPUDAFDParser
{
public:
    //Factory API to instantiate AFD parser
    static IPUDAFDParser* Create(IReceiverControl* receiverControl, LPVOID pVideoDecoderContext, IPUDCallbackChain* pPUDCallbackChain);

public:
    //Destructor
    virtual ~IPUDAFDParser() {}
};

// ===============================================================================================================
// ===============================================================================================================
