///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <SSPKDefines.h>

#include <pkExecutive.h>

#include <IAVManager.h>
#include <ISmoothTransport.h>

#include <CEvent.h>
#include <AutoLock.h>

#include "SmoothTransportDriver.h"

////////////////////////////////////////////////////////////////////////////////
//
// CSmoothTransportDriver
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
HRESULT CSmoothTransportDriver::Create()
{
    HRESULT hr = S_OK;

    AutoLock al( &m_objLock );

    Dispose();

    if( NULL == IAVManager::Create() )
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    m_fHasAvManager = true;

    m_pSmoothTransport = SSPK::ISmoothTransport::CreateSmoothTransport();

    if( NULL == m_pSmoothTransport )
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    m_pSmoothTransport->RegisterErrorCallback( this );
    m_pSmoothTransport->RegisterStatusCallback( this );
    m_pSmoothTransport->SetManifestCallback( this );

exit:

    return( hr );
}

////////////////////////////////////////////////////////////////////////////////
void CSmoothTransportDriver::Dispose()
{
    AutoLock al( &m_objLock );

    m_apStatusSink.Release();
    m_apErrorSink.Release();
    m_apManifestReadySink.Release();

    if( m_pSmoothTransport != NULL )
    {
        m_pSmoothTransport->Close();

        m_pSmoothTransport->RegisterErrorCallback( this, false );
        m_pSmoothTransport->RegisterStatusCallback( this, false );
        m_pSmoothTransport->SetManifestCallback( NULL );

        SSPK::ISmoothTransport::DestroySmoothTransport( m_pSmoothTransport );
        m_pSmoothTransport = NULL;
    }

    if( m_fHasAvManager )
    {
        IAVManager::Destroy();
        m_fHasAvManager = false;
    }
}

////////////////////////////////////////////////////////////////////////////////
void CSmoothTransportDriver::SetStausSink( _In_opt_ EventSink* pSink )
{
    AutoLock al( &m_objLock );
    m_apStatusSink.Set( pSink );
}

////////////////////////////////////////////////////////////////////////////////
void CSmoothTransportDriver::SetErrorSink( _In_opt_ EventSink* pSink )
{
    AutoLock al( &m_objLock );
    m_apErrorSink.Set( pSink );
}

////////////////////////////////////////////////////////////////////////////////
void CSmoothTransportDriver::SetManifestReadySink( _In_opt_ EventSink* pSink )
{
    AutoLock al( &m_objLock );
    m_apManifestReadySink.Set( pSink );
}


////////////////////////////////////////////////////////////////////////////////
void CSmoothTransportDriver::StatusCallback( _In_ SSPK::SmoothTransportStatus& status )
{
    AutoRefPtr<EventSink> apSink;

    {
        AutoLock al( &m_objLock );
        apSink = m_apStatusSink;
    }

    if( apSink != NULL )
    {
        apSink->OnStatus( *this, status );
    }
}

////////////////////////////////////////////////////////////////////////////////
void CSmoothTransportDriver::ErrorCallback( _In_ SSPK::SmoothTransportError& error )
{
    AutoRefPtr<EventSink> apSink;

    {
        AutoLock al( &m_objLock );
        apSink = m_apErrorSink;
    }

    if( apSink != NULL )
    {
        apSink->OnError( *this, error );
    }
}

////////////////////////////////////////////////////////////////////////////////
void CSmoothTransportDriver::ManifestReadyCallback(_In_ IManifest* pManifest, pkRESULT result )
{
    AutoRefPtr<EventSink> apSink;

    {
        AutoLock al( &m_objLock );
        apSink = m_apManifestReadySink;
    }

    if( apSink != NULL )
    {
        apSink->OnManifestReady( *this, pManifest, result );
    }
}

////////////////////////////////////////////////////////////////////////////////
//
//  Auxiliary functions for diagnostics
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
const char* GetSmoothTransportTunerStateDesc( _In_ SSPK::ESmoothTransportState v )
{
    const char* rg[] =
    {
        "Unknown",   //  SmoothTransportTunerState_Unknown
        "Tuning",    //  SmoothTransportTunerState_Tuning
        "Playing",   //  SmoothTransportTunerState_Playing
        "Paused",    //  SmoothTransportTunerState_Paused
        "MediaEnded",//  SmoothTransportTunerState_MediaEnded
        "Detuned",   //  SmoothTransportTunerState_Detuned
        "Closed",    //  SmoothTransportTunerState_Closed
    };

    return( ( ( 0 <= v ) && ( v < sizeof(rg)/sizeof(rg[0]) ) ) ? rg[v] : "<unknown>" );
}

////////////////////////////////////////////////////////////////////////////////
const char* GetSmoothTransportStatusName( _In_ SSPK::ESmoothTransportStatusUpdate v )
{
    const char* rg[] =
    {
        "Unknown", // SmoothTransportStatus_Unknown
        "Heartbeat", // SmoothTransportStatus_Heartbeat
        "TunerStateChanged", // SmoothTransportStatus_TunerStateChanged
        "PmtChanged", // SmoothTransportStatus_PmtChanged
        "Streaming", // SmoothTransportStatus_Streaming
        "Rendering", // SmoothTransportStatus_Rendering
        "Underrun", // SmoothTransportStatus_Underrun
        "Rebuffer", // SmoothTransportStatus_Rebuffer
        "StartEndTime", // SmoothTransportStatus_StartEndTime
        "DrmStateChanged", // SmoothTransportStatus_DrmStateChanged
        "BitrateChanged", // SmoothTransportStatus_BitrateChanged
        "DecoderError", // SmoothTransportStatus_DecoderError
        "NextChunkHttpInvalid", //SmoothTransportStatus_ChunkConnectHttpInvalid
        "NextChunkHttpInvalid", //SmoothTransportStatus_NextChunkHttpInvalid
        "ChunkHdrHttpInvalid", //SmoothTransportStatus_ChunkHdrHttpInvalid
        "ChunkHdrError", //SmoothTransportStatus_ChunkHdrError
        "AtWindowEdge", //SmoothTransportStatus_AtWindowEdge 
        "EndOfLive", //SmoothTransportStatus_EndOfLive
    };

    return( ( ( 0 <= v ) && ( v < sizeof(rg)/sizeof(rg[0]) ) ) ? rg[v] : "<unknown>" );
}

