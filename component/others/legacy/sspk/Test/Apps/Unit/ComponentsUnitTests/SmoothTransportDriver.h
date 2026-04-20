///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// CSmoothTransportDriver -
//
//  Auxiliary class to drive a Smooth Transport object for testing purposes
//
////////////////////////////////////////////////////////////////////////////////

class CSmoothTransportDriver
    : private SSPK::ISmoothTransportStatusSink
    , private SSPK::ISmoothTransportErrorSink
    , private SSPK::IManifestReadyCallback
{
public:

    CSmoothTransportDriver()
        : m_pSmoothTransport( NULL )
        , m_fHasAvManager( false )
    {
    }

    ~CSmoothTransportDriver()
    {
        Dispose();
    }

    HRESULT Create();

    void Dispose();

    SSPK::ISmoothTransport* operator->()
    {
        return( m_pSmoothTransport );
    }

    //
    // Sinks
    //

    class EventSink;

    void SetStausSink( _In_opt_ EventSink* pSink );
    void SetErrorSink( _In_opt_ EventSink* pSink );
    void SetManifestReadySink( _In_opt_ EventSink* pSink );

    ////////////////////////////////////
    // Auxiliary event class

    class AutoEvent
        : public CEvent
    {
    public:

        AutoEvent( CEvent::EResetMode erm = CEvent::eResetModeAuto, bool fSignaled = false )
            : CEvent( erm, fSignaled )
        {
        }
    };

    ////////////////////////////////////
    // EventSink class

    class EventSink
    {
    public:

        EventSink()
            : m_cRefs( 1 )
        {
        }

        virtual ~EventSink()
        {
            Release();
            m_fullyReleased.Wait();
        }

        void AddRef()
        {
            Executive_InterlockedIncrement( &m_cRefs );
        }

        void Release()
        {
            if( 0 == Executive_InterlockedDecrement( &m_cRefs ) )
            {
                m_fullyReleased.Set();
            }
        }

        virtual void OnStatus(
                        _In_ CSmoothTransportDriver& drv,
                        _In_ SSPK::SmoothTransportStatus& status ) {}

        virtual void OnError(
                        _In_ CSmoothTransportDriver& drv,
                        _In_ SSPK::SmoothTransportError& error ) {}

        virtual void OnManifestReady(
                        _In_ CSmoothTransportDriver& drv,
                        _In_ IManifest* pManifest,
                        _In_ pkRESULT result) {}

    private:

        int32_t m_cRefs;
        AutoEvent m_fullyReleased;
    };

private:

    virtual void StatusCallback( _In_ SSPK::SmoothTransportStatus& status );
    virtual void ErrorCallback( _In_ SSPK::SmoothTransportError& error );
    virtual void ManifestReadyCallback( _In_ IManifest* pManifest, HRESULT result );

private:

    SSPK::ISmoothTransport* m_pSmoothTransport;
    bool m_fHasAvManager;
    Lockable m_objLock;
    AutoRefPtr<EventSink> m_apStatusSink;
    AutoRefPtr<EventSink> m_apErrorSink;
    AutoRefPtr<EventSink> m_apManifestReadySink;
};

////////////////////////////////////////////////////////////////////////////////
//
//  Auxiliary functions for diagnostics
//
////////////////////////////////////////////////////////////////////////////////

const char* GetSmoothTransportTunerStateDesc( _In_ SSPK::ESmoothTransportState v );
const char* GetSmoothTransportStatusName( _In_ SSPK::ESmoothTransportStatusUpdate v );
