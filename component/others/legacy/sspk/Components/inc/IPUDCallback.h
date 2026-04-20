///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// ===============================================================================================================
// Derive from this API if interested in picture user data
// ===============================================================================================================

class IPUDCallback
{
public:
    virtual void PUDCallback(const byte* userdata, uint32 userdata_length) = 0;
    virtual void PUDCleanup(void) = 0;
};

// ===============================================================================================================
// Picture user data callback chain manager
// To allow for subscribers to receive picture user data
//  Note that the call back is done on low level thread from HAL so this should be processed quickly
//  Avoid locking if possible...
//  Up to 2 callbacks allowed at this time...
// ===============================================================================================================

class IReceiverControl;

class IPUDCallbackChain
{
public:
    //Factory API to instantiate a picture user data callback chain manager
    static IPUDCallbackChain* Create(IReceiverControl* receiverControl, LPVOID pVideoDecoderContext);

public:
    //Destructor
    virtual ~IPUDCallbackChain() {}
    //Register picture user data callback
    virtual bool Register(IPUDCallback* callback) = 0;
    //Unregister picture user data callback
    virtual bool UnRegister(IPUDCallback* callback) = 0;
};

// ===============================================================================================================
// ===============================================================================================================
