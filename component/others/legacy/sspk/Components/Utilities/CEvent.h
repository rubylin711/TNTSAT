///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

class CEvent
{
public:
    enum EResetMode { eResetModeAuto, eResetModeManual };

    CEvent( EResetMode eResetMode, bool bInitiallySignaled = false );

    virtual    ~CEvent();

    bool     Set();
    bool     Reset();

    enum EWaitResult { eWaitAbandoned, eWaitSignaled, eWaitTimeout, eWaitFailed };

    EWaitResult Wait( DWORD dwMsTimeout = INFINITE );

protected:
    CEvent();

private:
    HANDLE    m_Event;
};

