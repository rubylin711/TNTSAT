///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "DispatchTimer.h"


DispatchTimer::~DispatchTimer()
{
    delete m_Thread;
    delete m_tEvent;
}

void DispatchTimer::Init( __in DWORD dwInterval )
{
    m_isActive = false;
    m_Thread = new Thread();
    m_tEvent = new CEvent(CEvent::eResetModeAuto,false);
    m_dwInterval = dwInterval;
}

void DispatchTimer::SetInterval( DWORD dwInterval )
{
    m_dwInterval = dwInterval;
}

void DispatchTimer::Start()
{
    if(m_isActive)
    {
        Stop();
    }
    m_isActive = true;
    m_Thread->Start(this);
}

void DispatchTimer::Stop()
{
    if(m_isActive)
    {
        m_isActive = false;
        m_tEvent->Set();

        m_Thread->Stop();
    }
}

void DispatchTimer::OnThreadRun()
{
    while(true)
    {
        CEvent::EWaitResult eWaitResult = m_tEvent->Wait(m_dwInterval);
        if( eWaitResult != CEvent::eWaitTimeout )
        {
            break;
        }
        OnTick();
    }


}

bool DispatchTimer::IsTimerActive()
{
    return m_isActive;
}
