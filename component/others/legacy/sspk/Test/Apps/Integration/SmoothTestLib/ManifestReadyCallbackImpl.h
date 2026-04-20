///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include "ISmoothTransport.h"
#include "CEvent.h"

#include <list>
#include <string>

using namespace std;
using namespace SSPK;

class CManifestReadyCallback
{
public:

    CManifestReadyCallback()
        : m_pSink( NULL )
    {
    }

    ~CManifestReadyCallback()
    {
        delete m_pSink;
    }

    template< class T, class P >
    void Set(
        _In_ T* preRollThis,
        _In_ void (T::*preRollFunc)( _In_ IManifest* pManifest, HRESULT hr ),
        _In_ P *pThis,
        _In_ void (P::*pFunc)( _In_ IManifest* pManifest, HRESULT hr ))
    {
        CSinkImpl<T,P>* pNewSink = new CSinkImpl<T,P>();

        pNewSink->m_preRollThis = preRollThis;
        pNewSink->m_preRollFunc = preRollFunc;
        pNewSink->m_pThis = pThis;
        pNewSink->m_pFunc = pFunc;

        delete m_pSink;

        m_pSink = pNewSink;
    }

    HRESULT GetCallbackResult()
    {
        return m_pSink->GetHResult();
    }

    IManifest* GetManifest()
    {
        return m_pSink->GetManifestInstance();
    }

    bool IsManifestReadyRaised()
    {
        return m_pSink->IsManifestReadyRaised();
    }

    operator IManifestReadyCallback* ()
    {
        return( m_pSink );
    }

    bool Wait(uint32_t dwMsTimeout = INFINITE)
    {
        return m_pSink->Wait(dwMsTimeout);
    }

protected:

    struct ISink : IManifestReadyCallback
    {
        ISink(): m_hr(E_FAIL), m_pManifest(NULL), m_fManifestReadyRaised(false) {}

        virtual ~ISink() {};
        virtual IManifest* GetManifestInstance() = 0;
        virtual pkRESULT GetHResult() = 0;
        virtual bool IsManifestReadyRaised() = 0;
        virtual bool Wait(uint32_t dwMsTimeout = INFINITE) = 0;

        protected:

        pkRESULT    m_hr;
        IManifest   *m_pManifest;
        bool        m_fManifestReadyRaised;

    };

    template< class T, class P >
    struct CSinkImpl : ISink
    {
        CEvent          m_event;
        T*              m_preRollThis;
        void            (T::*m_preRollFunc)( _In_ IManifest* pManifest, HRESULT hr );
        
        P*              m_pThis;
        void            (P::*m_pFunc)( _In_ IManifest* pManifest, HRESULT hr );

        virtual void ManifestReadyCallback( _In_ IManifest* pManifest, HRESULT hr)
        {
            m_hr = hr;
            m_pManifest = pManifest;
            m_fManifestReadyRaised = true;

            if(SUCCEEDED(hr))
            {
                ( m_preRollThis->*m_preRollFunc )( pManifest, hr);
                ( m_pThis->*m_pFunc )( pManifest, hr);
            }

            m_event.Set();
        }

        bool IsManifestReadyRaised()
        {
            return m_fManifestReadyRaised;
        }

        IManifest* GetManifestInstance()
        {
            return m_pManifest;
        }

        pkRESULT GetHResult()
        {
            return m_hr;
        }

        bool Wait(uint32_t dwMsTimeout = INFINITE)
        {
            if(CEvent::eWaitSignaled == m_event.Wait(dwMsTimeout))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        CSinkImpl() : m_event(CEvent::eResetModeAuto, FALSE) { }
    };

    ISink           *m_pSink;
private:

    CManifestReadyCallback( const CManifestReadyCallback& );
    void operator = ( const CManifestReadyCallback& );
};



