///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// ===============================================================================================================
// Callback prototype to receive diags events
// ===============================================================================================================

typedef void (CALLBACK *PFN_AVDIAGCALLBACK)(bool isError, LPCWSTR sEventMsg, LPCWSTR sEventData);
typedef void (CALLBACK *PFN_AVDIAGBINARYCALLBACK)(byte nStorageMode, byte nCategory, byte nSubcategory, byte nPriority, byte nLogMode, uint16_t nEventId, byte* sEventData, int sEventDataLength);

// ===============================================================================================================
// IDiagsManager manages reporting of diagsnostics events
// ===============================================================================================================

class IDiagsEvent;
class IDiagsProvider;

class IDiagsManager
{
public:
    //Destructor
    virtual ~IDiagsManager() {};

    //Called to register a wide character string serialized events data callback into diagnostics manager
    virtual void RegisterCallback(PFN_AVDIAGCALLBACK callback) = 0;
    //Called to de-register wide character string serialized events data callback from diagnostics manager
    virtual void UnregisterCallback(PFN_AVDIAGCALLBACK callback) = 0;
    //Enable/disable wide character string serialized events data logging
    virtual void RegisterFilters(bool enabled) = 0;

    //Called to register a binary event data callback into diagnostics manager
    virtual void RegisterBinaryCallback(PFN_AVDIAGBINARYCALLBACK callback) = 0;
    //Called to de-register a binary event data callback from diagnostics manager
    virtual void UnregisterBinaryCallback(PFN_AVDIAGBINARYCALLBACK callback) = 0;
    //Enable/disable binary serialized events data logging and filtering
    virtual void RegisterBinaryFilters(const byte* filters, int32_t filtersLength) = 0;

    //Managed to native PInvoke call to pick up diagnostics events
    virtual bool GetDiagnosticEvent(int* isError, WCHAR* psEventMsg, int eventMsgLength, WCHAR* psEventData, int eventDataLength) = 0;

    //Called to post events into the diagnostic events queue
    virtual void PostEvent(IDiagsEvent* diagsEvent) = 0;

    //Called by the diagnostic providers to register themselves for periodic events
    virtual void Attach(IDiagsProvider* diagsProvider) = 0;
    //Called by the diagnostic providers to de-register themselves for periodic events
    virtual void Detach(IDiagsProvider* diagsProvider) = 0;
};

// ===============================================================================================================
// ===============================================================================================================

enum DiagsChannel
{
    kDiagChannel_Heuristics = 0,
    kDiagChannel_Manifest,
    kDiagChannel_ChunkList,
    kDiagChannel_FragInfo,

    kDiagChannel_CountOf,
};

enum DiagsPriority
{
    kDiagsPriority_Disabled = 0,
    kDiagsPriority_High = 1,
    kDiagsPriority_Medium = 10,
    kDiagsPriority_Low = 20,
};

// ===============================================================================================================
// Used by the application to enable the events from a channel by priority
// ===============================================================================================================

extern void DiagSetChannelPriority(
    _In_ DiagsChannel channel,
    _In_ DiagsPriority priority );


