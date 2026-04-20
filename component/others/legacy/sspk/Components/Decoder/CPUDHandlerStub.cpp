///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IPUDHandler.h"
#include "IReceiverControl.h"
#include "CRendererState.h"

// ===============================================================================================================
// Picture User Callback handler
// ===============================================================================================================

class CPUDHandler : public IPUDHandler
{
    friend class IPUDHandler;

private:
    //Constructor
    CPUDHandler(IReceiverControl* receiverControl, const CStreamInfo& si, LPVOID decoderContext)
        : mReceiverControl(receiverControl)
        , mRendererState(mReceiverControl->GetRendererState())
        , mPipeIdN(mRendererState.mPipeIdN)
    {
        //So, try to attach it to the trigger service
        EnableTriggers(true);
    }

public:
    //Destructor
    virtual ~CPUDHandler() {}

    //Trigger service registration/unregistration
    __override void         EnableTriggers(bool enable) {}

private:
    //Receiver control APIs
    IReceiverControl*       mReceiverControl;
    //Current rendering state
    CRendererState&         mRendererState;
    //Pipe id
    uint32                  mPipeIdN;
};

// ===============================================================================================================
// Factory API to instantiate Picture User Callback handler
// ===============================================================================================================

IPUDHandler* IPUDHandler::Create(IReceiverControl* receiverControl, const CStreamInfo& si, LPVOID decoderContext)
{
    return new CPUDHandler(receiverControl, si, decoderContext);
}

// ===============================================================================================================
// ===============================================================================================================
