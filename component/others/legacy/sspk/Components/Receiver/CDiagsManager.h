///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IDiagsManager.h"
#include "IDiagsEvent.h"
#include "IDiagsProvider.h"
#include "CEvent.h"
#include "Thread.h"
#include "ThreadTime.h"
#include "AutoLock.h"
#include <list>

// ===============================================================================================================
// CDiagsManager manages reporting of diagsnostics events
// ===============================================================================================================

class CDiagsManager : public IDiagsManager, public IRunnable
{
public:
    CDiagsManager();
    virtual    ~CDiagsManager();

    //
    // IRunnable implementation
    //

    //Diags thread - Wakes up every so often to collect periodic
    //diagnostic events from each provider and posts all active
    //events to the callbacks.
    __override void OnThreadRun();

    //
    // IDiagManager implementation
    //

    //Called to register a wide character string serialized events data callback into diagnostics manager
    __override void RegisterCallback(PFN_AVDIAGCALLBACK callback);
    //Called to de-register wide character string serialized events data callback from diagnostics manager
    __override void UnregisterCallback(PFN_AVDIAGCALLBACK callback);
    //Enable/disable native to managed wide character string serialized events data logging
    __override void RegisterFilters(bool enabled);
    //Called to register a binary event data callback into diagnostics manager
    __override void RegisterBinaryCallback(PFN_AVDIAGBINARYCALLBACK callback);
    //Called to de-register a binary event data callback from diagnostics manager
    __override void UnregisterBinaryCallback(PFN_AVDIAGBINARYCALLBACK callback);
    //Enable/disable native to managed binary serialized events data logging
    __override void RegisterBinaryFilters(const byte* filters, int32 filtersLength);
    //Managed to native PInvoke call to pick up diagnostics events
    __override bool GetDiagnosticEvent(int* isError, __out_ecount(eventMsgLength) WCHAR* psEventMsg, int eventMsgLength,
        __out_ecount(eventDataLength) WCHAR* psEventData, int eventDataLength);
    //Called to post events into the diagnostic events queue
    __override void PostEvent(IDiagsEvent* diagsEvent);
    //Called by the diagnostic providers to register themselves for periodic events
    __override void Attach(IDiagsProvider* diagsProvider);
    //Called by the diagnostic providers to de-register themselves for periodic events
    __override void Detach(IDiagsProvider* diagsProvider);

private:
    //Called internally by the diagnostic manager thread to process and post an event to the callbacks
    void DiagsEventsCallback(IDiagsEvent* diagsEvent);

    IDiagsEvent* GetEvent();

    //Accept events from native engine into the framework
    bool _diagsAcceptsEventsFromAVEngine;
    //Whether events need to be sent to managed framework
    bool _diagsSendToManaged;

    //
    //Managing diagnostics callbacks and posting of wide character string serialized events data
    //
    typedef std::list<PFN_AVDIAGCALLBACK> DiagsCallbacksList;
    DiagsCallbacksList _diagsCallbacks;
    Lockable _diagsCallbacksLock;
    //Whether the wide character string serialized events data posting is enabled
    bool _diagsEnabled;

    //
    //Managing diagnostics callbacks and posting of binary serialized events data
    //
    typedef std::list<PFN_AVDIAGBINARYCALLBACK> DiagsBinaryCallbacksList;
    DiagsBinaryCallbacksList _diagsBinaryCallbacks;
    Lockable _diagsBinaryCallbacksLock;
    //Whether the binary serialized events data posting is enabled
    bool _diagsBinaryEnabled;
    //Total number available filters
    byte _diagsBinaryFilters[kDiagsEventBinaryFiltersNum];

    //
    //Managing list of current diagnostic events waiting to be posted
    //
    IDiagsEvent* _diagsEventsList;
    IDiagsEvent* _diagsEventsLast;
    int _diagsEventsListSize;
    CEvent _diagsEventsListEvent;
    Lockable _diagsEventsLock;
    volatile bool _bGetDiagnosticEventUsed;

    //
    //Managing list of periodic diagnostic events providers
    //
    std::list<IDiagsProvider*> _diagsProviders;
    Lockable _diagsProvidersLock;

    //
    //Managing diagnostic event thread which does
    //- queries diagnostics providers for periodic events
    //- posts all currently active events to the callbacks
    //
    bool   _running;
    CEvent _event;
    Thread _thread;
    ThreadTime _threadTime;
};

// ===============================================================================================================
// ===============================================================================================================
