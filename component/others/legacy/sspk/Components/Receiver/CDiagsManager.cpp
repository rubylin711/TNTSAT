///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CDiagsManager.h"
#include "IAVManager.h"
#include "Trace.h"
#include "CDiags.h"
#include <strsafe.h>
#include <list>
using namespace std;

//#define CDIAGS_TRACE_ENABLE
#ifdef CDIAGS_TRACE_ENABLE
#define CDIAGS_TRACE(x) TRACE(x)
#else
#define CDIAGS_TRACE(x)
#endif

const DWORD c_dwEventArrivalTimeoutMs = 1000;
const DWORD c_dwEventFailureSleepMs = 100;
const DWORD c_dwDiagsMgrShutdownPollPeriodMs = 100;

// ===============================================================================================================
// CDiagsManager manages reporting of diagsnostics events
// ===============================================================================================================

//Called to register a wide character string serialized events data callback into diagnostics manager
void CDiagsManager::RegisterCallback(PFN_AVDIAGCALLBACK callback)
{
    if (callback)
    {
        AutoLock lock(&_diagsCallbacksLock);
        _diagsCallbacks.push_back(callback);

        CDIAGS_TRACE(("CDiagsManager:: Registering callback %08x", callback));
    }
}

//Called to de-register wide character string serialized events data callback from diagnostics manager
void CDiagsManager::UnregisterCallback(PFN_AVDIAGCALLBACK callback)
{
    if (callback)
    {
        AutoLock lock(&_diagsCallbacksLock);
        _diagsCallbacks.remove(callback);

        CDIAGS_TRACE(("CDiagsManager:: UnRegistering callback %08x", callback));
    }
}

//Enable/disable native to managed wide character string serialized events data logging
void CDiagsManager::RegisterFilters(bool enabled)
{
    AutoLock lock(&_diagsCallbacksLock);
    _diagsEnabled = enabled;

    TRACE(("CDiagsManager:: Diags String %s", _diagsEnabled ? "Enabled" : "Disabled"));
}

//Called to register a binary event data callback into diagnostics manager
void CDiagsManager::RegisterBinaryCallback(PFN_AVDIAGBINARYCALLBACK callback)
{
    if (callback)
    {
        AutoLock lock(&_diagsBinaryCallbacksLock);
        _diagsBinaryCallbacks.push_back(callback);

        CDIAGS_TRACE(("CDiagsManager:: Registering callback %08x", callback));
    }
}

//Called to de-register a binary event data callback from diagnostics manager
void CDiagsManager::UnregisterBinaryCallback(PFN_AVDIAGBINARYCALLBACK callback)
{
    if (callback)
    {
        AutoLock lock(&_diagsBinaryCallbacksLock);
        _diagsBinaryCallbacks.remove(callback);

        CDIAGS_TRACE(("CDiagsManager:: UnRegistering callback %08x", callback));
    }
}

//Called by managed framework to provide current set of filters
//Enable/disable native to managed binary serialized events data logging
void CDiagsManager::RegisterBinaryFilters(const byte* filters, int32 filtersLength)
{
    AutoLock lock(&_diagsBinaryCallbacksLock);

    //Just clear the filters first
    memset(_diagsBinaryFilters, 0, kDiagsEventBinaryFiltersNum);

    //Now copy the latest filters if available
    if (filters != NULL && filtersLength > 0)
    {
        memcpy_s(_diagsBinaryFilters, kDiagsEventBinaryFiltersNum, filters, filtersLength <= kDiagsEventBinaryFiltersNum ? filtersLength : kDiagsEventBinaryFiltersNum);
    }

    //We will be using just the first filter for now...
    _diagsBinaryEnabled = _diagsBinaryFilters[0] != 0;

    CDIAGS_TRACE(("CDiagsManager:: Diags Binary %s", _diagsBinaryEnabled ? "Enabled" : "Disabled"));
}

//Caled internally by the diagnostic manager thread to process and post an event to the callbacks
void CDiagsManager::DiagsEventsCallback(IDiagsEvent* diagsEvent)
{
    //Send wide character string serialized event
    if (_diagsEnabled)
    {
        WCHAR diagsData[kDiagsEventDataLen];
        //Initialize diags data string
        diagsData[0] = 0;
        //This is where the event would be built
        diagsEvent->DiagsSetBuffer(diagsData, kDiagsEventDataLen);
        //Add event timestamp to the event log data
        diagsEvent->DiagsPreamble();
        //Process the diagnostics event ot generate event specific log data
        diagsEvent->DiagsGetEventData();
        //Terminate the string with a null
        diagsEvent->DiagsFinalize();

        //Get the event message name
        const WCHAR* diagsMessage = diagsEvent->DiagsGetEventMessage();

        //Dump the event to debug output
        CDIAGS_TRACE(("CDiagsManager:: Making a callback at %d", Executive_GetTickCount()));
        CDIAGS_TRACE(("                EventName=%ls", diagsMessage));
        CDIAGS_TRACE(("                EventData=%ls", diagsData));

        //And send it
        {
            AutoLock lock(&_diagsCallbacksLock);
            for (DiagsCallbacksList::iterator it = _diagsCallbacks.begin(); it != _diagsCallbacks.end(); ++it)
            {
                if (_diagsSendToManaged)
                {
                    (*it)(diagsEvent->DiagsIsError(), diagsMessage, diagsData);
                }
            }
        }
    }

    //Send binary serialized event
    if (_diagsBinaryEnabled)
    {
        byte version = diagsEvent->DiagsEventVersion();
        uint32 diagsBinaryDataLength = diagsEvent->DiagsSerializeEventDataLength(version) + 8;
        if (diagsBinaryDataLength > 12)
        {
            //Serialize the event into binary format
            byte diagsBinaryData[kDiagsEventBinaryDataLen];
            byte* data = diagsBinaryData;
            diagsEvent->DiagsSerializeEventData(data);

            //Sanity check
            ASSERT((uint32) (data - diagsBinaryData) == diagsBinaryDataLength);

            //And send it
            {
                AutoLock lock(&_diagsBinaryCallbacksLock);
                for (DiagsBinaryCallbacksList::iterator it = _diagsBinaryCallbacks.begin(); it != _diagsBinaryCallbacks.end(); ++it)
                {
                    if (_diagsSendToManaged)
                    {
                        (*it)(
                            (byte)diagsEvent->DiagsLogStorageMode(),
                            (byte)diagsEvent->DiagsLogCategory(),
                            (byte)diagsEvent->DiagsLogSubcategory(),
                            (byte)diagsEvent->DiagsLogPriority(),
                            (byte)(diagsEvent->DiagsIsError() ? kLogMode_Error : kLogMode_Information),
                            (uint16)diagsEvent->DiagsGetEventType(), diagsBinaryData, diagsBinaryDataLength);
                    }
                }
            }
        }
    }
}

IDiagsEvent* CDiagsManager::GetEvent()
{
    IDiagsEvent* diagsEvent;
    AutoLock lock(&_diagsEventsLock);
    diagsEvent = _diagsEventsList;
    if (diagsEvent)
    {
        _diagsEventsList = _diagsEventsList->NextEvent;
        if (_diagsEventsList == NULL)
        {
            _diagsEventsLast = NULL;
        }
        _diagsEventsListSize--;
    }
    return diagsEvent;
}
//Managed to native PInvoke call to pick up diagnostics events
bool CDiagsManager::GetDiagnosticEvent(int* isError, WCHAR* psEventMsg, int eventMsgLength, WCHAR* psEventData, int eventDataLength)
{
    if ((this == NULL) || !_running || _bGetDiagnosticEventUsed)
        return false;

    _bGetDiagnosticEventUsed = true;

    //Wait for for events to arrive
    //Timeout every 1 second so that managed thread has a chance to go back
    //and do some work as needed...
    if (_diagsEventsListSize == 0)
    {
        CEvent::EWaitResult eWaitResult = _diagsEventsListEvent.Wait(c_dwEventArrivalTimeoutMs);
        switch (eWaitResult)
        {
        case CEvent::eWaitSignaled:
        case CEvent::eWaitTimeout:
            break;
        default:
            //If something wrong with the wait, just sleep a short while
            Executive_Sleep(c_dwEventFailureSleepMs);
        }
    }
    if (_diagsEnabled)
    {
        //Pick up the event at the head of the queue and send it out
        IDiagsEvent* diagsEvent = GetEvent();

        if (diagsEvent)
        {
            //Get the type of event
            *isError = diagsEvent->DiagsIsError() ? 1 : 0;

            //Get the event message name
            StringCchCopyW(psEventMsg, eventMsgLength, diagsEvent->DiagsGetEventMessage());

            //Initialize diags data string
            psEventData[0] = 0;
            //This is where the event would be built
            diagsEvent->DiagsSetBuffer(psEventData, eventDataLength);
            //Add event timestamp to the event log data
            diagsEvent->DiagsPreamble();
            //Process the diagnostics event ot generate event specific log data
            diagsEvent->DiagsGetEventData();
            //Terminate the string with a null
            diagsEvent->DiagsFinalize();

            //Dump the event to debug output
            CDIAGS_TRACE(("CDiagsManager:: Making a callback at %d", Executive_GetTickCount()));
            CDIAGS_TRACE(("                EventName=%ls", psEventMsg));
            CDIAGS_TRACE(("                EventData=%ls", psEventData));

            //And delete the event
            delete diagsEvent;
            _bGetDiagnosticEventUsed = false;
            return true;
        }
    }
    _bGetDiagnosticEventUsed = false;
    return false;
}

//Called to post events into the diagnostic events queue
void CDiagsManager::PostEvent(IDiagsEvent* diagsEvent)
{
    if (diagsEvent)
    {
        CDIAGS_TRACE(("CDiagsManager:: Posting event=%ls", diagsEvent->DiagsGetEventMessage()));

        if (_diagsAcceptsEventsFromAVEngine && (_diagsEnabled || _diagsBinaryEnabled) && _running)
        {
            //Post the event to the queue
            {
                AutoLock lock(&_diagsEventsLock);
                if (_diagsEventsLast)
                    _diagsEventsLast->NextEvent = diagsEvent;
                else
                    _diagsEventsList = diagsEvent;
                _diagsEventsLast = diagsEvent;
                _diagsEventsListSize++;
            }
            //Wake up the managed thread so that it can pick it up
            //
            //***** TO BE DONE *****
            //We should evaluate if we need to do this on a per event basis
            //because the managed thread will wake up every one second and pick
            //up all that is already queued up anyways...
            _diagsEventsListEvent.Set();
        }
        else
        {
            delete diagsEvent;
        }
    }
}

//Called by the diagnostic providers to register themselves for periodic events
void CDiagsManager::Attach(IDiagsProvider* diagsProvider)
{
    if (diagsProvider)
    {
        AutoLock lock(&_diagsProvidersLock);
        //Initialize the diags provider
        diagsProvider->DiagsInit();
        //Add the new provider to the active list
        _diagsProviders.push_back(diagsProvider);

        CDIAGS_TRACE(("CDiagsManager:: Attaching provider %08x", diagsProvider));
    }
}

//Called by the diagnostic providers to de-register themselves for periodic events
void CDiagsManager::Detach(IDiagsProvider* diagsProvider)
{
    if (diagsProvider)
    {
        AutoLock lock(&_diagsProvidersLock);
        _diagsProviders.remove(diagsProvider);

        CDIAGS_TRACE(("CDiagsManager:: Detaching provider %08x", diagsProvider));
    }
}

CDiagsManager::CDiagsManager()
    : _diagsEventsListEvent(CEvent::eResetModeAuto)
    , _event(CEvent::eResetModeAuto)
{
    _diagsAcceptsEventsFromAVEngine = true;
    _diagsSendToManaged = true;

    _diagsEnabled = false;
    _diagsBinaryEnabled = false;
    memset(_diagsBinaryFilters, 0, kDiagsEventBinaryFiltersNum);

    _diagsEventsList = _diagsEventsLast = NULL;
    _diagsEventsListSize = 0;

    _bGetDiagnosticEventUsed = false;

    _running = true;
    _thread.Start(this);
}

CDiagsManager::~CDiagsManager()
{
    _diagsEnabled = false;
    _running = false;

    while (_bGetDiagnosticEventUsed)
    {
        _diagsEventsListEvent.Set();
        Executive_Sleep(c_dwDiagsMgrShutdownPollPeriodMs);
    }

    // if the event hasn't been retrieved by now, clean up the remaining events
    IDiagsEvent* diagsEvent = GetEvent();
    while (diagsEvent)
    {
        delete diagsEvent;
        diagsEvent = GetEvent();
    }

    _event.Set();
    _thread.Stop();
}

//Diags thread - Wakes up every so often to collect periodic
//diagnostic events from each provider and posts all active
//events to the callbacks.
void CDiagsManager::OnThreadRun()
{
    _threadTime.Init();
    while (_running)
    {
        //Tick every so often
        int ticks = Executive_GetTickCount();
        CEvent::EWaitResult eWaitResult = _event.Wait(c_dwEventArrivalTimeoutMs);
        switch (eWaitResult)
        {
        case CEvent::eWaitSignaled:
        case CEvent::eWaitTimeout:
            break;
        default:
            //If something wrong with the wait, just sleep for a second
            Executive_Sleep(c_dwEventFailureSleepMs);
        }
        ticks = Executive_GetTickCount() - ticks;

        //Collect periodic diagnostic information from each registered provider
        if (_diagsEnabled || _diagsBinaryEnabled)
        {
            AutoLock lock(&_diagsProvidersLock);
            for (list<IDiagsProvider*>::iterator it = _diagsProviders.begin(); it != _diagsProviders.end(); ++it)
            {
                PostEvent((*it)->DiagsGeneratePeriodicEvent(ticks));
            }
        }

        //Empty queue if it is getting too long
        {
            AutoLock lock(&_diagsEventsLock);
            static const int MAX_EVENTS_LIST_SIZE = 200;
            while (_diagsEventsListSize > MAX_EVENTS_LIST_SIZE)
            {
                IDiagsEvent* diagsEvent = GetEvent();

                // event should never be null if the size is greater than 0
                ASSERT( NULL != diagsEvent );
                delete diagsEvent;
            }
        }

#ifdef CDIAGS_TRACE_ENABLE
        //Check CPU timings periodically
        if (_threadTime.Collect())
        {
            CDIAGS_TRACE(("ThreadTimes[DIAGS][%d]: Last=%d, Average=%d, Maximum=%d", _threadTime.TotalUsed, _threadTime.Last, _threadTime.Average, _threadTime.Maximum));
            PostEvent(new CDiagsCpuUsageEvent(ThreadName_DiagsThread, _threadTime.Last, _threadTime.Average, _threadTime.Maximum, _threadTime.TotalUsed));
        }
#endif
    }
}

// ===============================================================================================================
// ===============================================================================================================
