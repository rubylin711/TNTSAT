///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pkExecutive.h"
#include "CEvent.h"

CEvent::CEvent(EResetMode eResetMode, bool bInitiallySignaled)
{
    pkRESULT pkResult =
        ::Executive_CreateEvent(
            NULL, // ignored
            (eResetMode == eResetModeManual),
            bInitiallySignaled,
            &m_Event );

    if (pkFAILED(pkResult))
    {
        m_Event = NULL;
        pkASSERT(FALSE);
    }
}

CEvent::~CEvent()
{
    if(m_Event != NULL)
    {
        ::Executive_CloseEvent(m_Event);
        m_Event = NULL;
    }
}

bool CEvent::Reset()
{
    return( pkSUCCEEDED(::Executive_ResetEvent(m_Event)) ? true : false );
}

bool CEvent::Set()
{
    return( pkSUCCEEDED(::Executive_SetEvent(m_Event)) ? true : false );
}

CEvent::EWaitResult CEvent::Wait( DWORD dwMsTimeout )
{
    pkRESULT pkResult = ::Executive_WaitForEvent(m_Event, dwMsTimeout);

    EWaitResult eWaitResult;

    switch (pkResult)
    {
        case pkE_ABORT:
            eWaitResult = eWaitAbandoned;
            break;
        case pkS_OK:
            eWaitResult = eWaitSignaled;
            break;
        case pkS_FALSE:
            eWaitResult = eWaitTimeout;
            break;
        default:
            eWaitResult = eWaitFailed;
            break;
    }
    return eWaitResult;
}

