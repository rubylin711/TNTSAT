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
// Subscribe to receive Closed Captioning data as they get parsed out of picture user data stream
// ===============================================================================================================

class IPUDCCParserCallback
{
public:
    virtual void CCParserProcess(byte cc_header, byte cc_data1, byte cc_data2) = 0;
    virtual void CCParserUpdate(void) = 0;
};

// ===============================================================================================================
// Parses Closed Captioning data out of picture user data stream and passes it down to subscribers
// ===============================================================================================================

class IReceiverControl;

class IPUDCCParser
{
public:
    //Factorty API to cosntruct a Closed Captioning stream parser
    static IPUDCCParser* Create(IReceiverControl* receiverControl, IPUDCallbackChain* pPUDCallbackChain);

public:
    //Destructor
    virtual ~IPUDCCParser() {}
    //Register closed captioning data callback
    virtual bool Register(IPUDCCParserCallback* callback) = 0;
    //Unregister closed captioning data callback
    virtual bool UnRegister(IPUDCCParserCallback* callback) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
