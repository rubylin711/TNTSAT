///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

// ====================================================================================================================
//
// Module: CPUDCCTriggerEngine.h
//
// ====================================================================================================================

#pragma once

#include "IPUDCCParser.h"

// ====================================================================================================================
// these constants used for internal triggering engine implementation
// ====================================================================================================================

enum CC_CHANNEL
{
    CC_CHANNELS_START=0,
    CC608b_CC1,
    CC608b_CC2,
    CC608b_CC3,
    CC608b_CC4,
    CC608b_TEXT1,
    CC608b_TEXT2,
    CC608b_TEXT3,
    CC608b_TEXT4,
    CC708b_CC1,
    CC708b_CC2,
    CC708b_CC3,
    CC708b_CC4,
    CC708b_CC5,
    CC708b_CC6,
    CC708b_CC7,
    CC_CHANNELS_MAX
};

// ====================================================================================================================
// ====================================================================================================================

class IPUDCCTriggerEngineCallback
{
public:
    virtual void TriggerEngineCallback(uint32 pipeIdN, CC_CHANNEL ccChannel, wchar_t ccText) = 0;
};

// ====================================================================================================================
// ====================================================================================================================

class IReceiverControl;

class IPUDCCTriggerEngine: public IPUDCCParserCallback
{
public:
    //Factorty API to cosntruct a Closed Captioning stream parser
    static IPUDCCTriggerEngine* Create(IReceiverControl* receiverControl);

public:
    virtual ~IPUDCCTriggerEngine() {}

    virtual bool IsEngineEnabled(void) = 0;
    virtual void Register(uint32 pipeIdN, IPUDCCTriggerEngineCallback* callback) = 0;
    virtual void UnRegister(uint32 pipeIdN, IPUDCCTriggerEngineCallback* callback) = 0;
    virtual void SetCCTriggerFlags(uint32 ccChannelBitFlags) = 0;

    //ProcessData called by outside thread to process buffered data
    virtual void ProcessData(void) = 0;
};

// ====================================================================================================================
// ====================================================================================================================
