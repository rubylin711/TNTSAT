///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <AutoLock.h>
#include "CSocketMbrManifest.h"
#include "StringUtils.h"
#include "Trace.h"

//#define SEGMENTMANIFESTFETCHER_SPEW
#ifdef SEGMENTMANIFESTFETCHER_SPEW
#define SEGMENTMANIFESTFETCHER_TRACE(x) TRACE(x)
#else
#define SEGMENTMANIFESTFETCHER_TRACE(x)
#endif

CSegmentManifestFetcher::CSegmentManifestFetcher()
    : m_hWorkThread( NULL )
{
}

CSegmentManifestFetcher::~CSegmentManifestFetcher()
{
    Shutdown();
}

pkRESULT CSegmentManifestFetcher::Initialize( _In_ IInternalMbrManifest* pMbrManifest )
{
    pkRESULT pkResult = pkS_OK;

    m_pMbrManifest = pMbrManifest;

    pkResult = Executive_CreateThread( _WorkThreadEntryPoint, this, 0, &m_hWorkThread );
    if( pkFAILED(pkResult) )
    {
        goto exit;
    }

    pkResult = Executive_SetThreadPriority( m_hWorkThread, pkEXECUTIVE_THREAD_PRIORITY_LOW );
    
exit:
    if( pkFAILED(pkResult) )
    {
        Shutdown();
    }

    return( pkResult );
}

////////////////////////////////////////////////////////////////////////////////
void CSegmentManifestFetcher::Shutdown()
{
    pkHANDLE hWorkThread = NULL;
    {
        AutoLock al( &m_Lock );
        hWorkThread = m_hWorkThread;
        m_hWorkThread = NULL;
    }

    if( hWorkThread != NULL )
    {
        Executive_WaitForThread( hWorkThread, EXEC_WAIT_INFINITE );
        Executive_CloseThread( hWorkThread );
    }
}

////////////////////////////////////////////////////////////////////////////////
void CSegmentManifestFetcher::Execute()
{
    pkRESULT pkResult = pkS_OK;
    TimeSpan_hns requestedMinTime = m_pMbrManifest->GetRequestedMinTime();
    TimeSpan_hns currentMinTime = m_pMbrManifest->MaxOfParentStreamMinTime();
    
    // no need to download if the request position is already in the dvr window
    if (requestedMinTime > currentMinTime)
    {
        m_pMbrManifest->SetDVRMinTime(requestedMinTime);
        goto done;
    }

    while( true )
    {   
        // check if thread has been asked to shutdown
        pkResult = ContinueWork();
        if(S_OK != pkResult)
        {
            break;
        }

        // check if requested edge has been reached or the chunklist already has the chunk
        if (m_pMbrManifest->ParentStreamsContainsTime(requestedMinTime))
        {
            m_pMbrManifest->SetDVRMinTime(requestedMinTime);
            break;
        }

        // if there is more room in the chunklist, then stop downloading and
        // set the dvr window to the max size
        if (m_pMbrManifest->IsDVRFull())
        {
            pkResult = pkE_NO_MORE_ROOM;
            m_pMbrManifest->SetDVRMinTime(TimeSpan_hns::FromTicks(TimeSpan_hns::MIN_TICKS));
            break;
        }

        // check the manifest to be downloaded is not beyond the requested position
        if(currentMinTime < requestedMinTime)
        {
            pkResult = pkE_BEFORE_VALID_RANGE;
            m_pMbrManifest->SetDVRMinTime(TimeSpan_hns::FromTicks(TimeSpan_hns::MIN_TICKS));
            break;
        }
        
        // go ahead to download another segment manifest
        currentMinTime = currentMinTime - m_pMbrManifest->SegmentDuration();

        std::wstring wstrSegmentManifestUrl;
        pkResult = m_pMbrManifest->GetSegmentManifestURL(currentMinTime, &wstrSegmentManifestUrl);
        if( pkFAILED(pkResult) )
        {
            string info = "status=segmentmanifesterror&pkresult=" + toString(pkResult) +
                          "&url=" + WStr2Str(wstrSegmentManifestUrl);

            m_pMbrManifest->ReportStatus(info);

            pkResult = pkE_UNEXPECTED;
            m_pMbrManifest->SetDVRMinTime(TimeSpan_hns::FromTicks(TimeSpan_hns::MIN_TICKS));
            break;

        }

        int httpResponse = 0;
        if( !m_pMbrManifest->DownloadParseSegmentManifest(wstrSegmentManifestUrl, &httpResponse) )
        {
            string info = "status=segmentmanifesterror&httpresponse=" + toString(httpResponse) +
                          "&pkresult=" + toString(pkResult) +
                          "&url=" + WStr2Str(wstrSegmentManifestUrl);

            m_pMbrManifest->ReportStatus(info);
            continue;
        }
    }

done:
    SEGMENTMANIFESTFETCHER_TRACE(("PlaybackRangeComplete pkResult 0x%x", pkResult));
    m_pMbrManifest->PlaybackRangeComplete(pkResult);
    
    {
        AutoLock al( &m_Lock );
        if (m_hWorkThread)
        {
            m_hWorkThread = NULL;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CSegmentManifestFetcher::ContinueWork()
{
    pkRESULT pkR = S_OK;

    AutoLock al( &m_Lock );

    if( m_hWorkThread == NULL )
    {
        pkR = pkE_ABORT;
    }

    return( pkR );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT DefaultSegmentManifestFetcher::CreateInstance(
    _In_ IInternalMbrManifest* pMbrManifest,   // owns the fetcher
    _Out_ CSegmentManifestFetcher** ppFetcher
    )
{
    pkRESULT pkResult = pkS_OK;

    if(*ppFetcher)
    {
        delete *ppFetcher;
        *ppFetcher = NULL;
    }

    CSegmentManifestFetcher* pSegmentFetcher = NEW_NO_THROW CSegmentManifestFetcher;

    if( NULL == pSegmentFetcher )
    {
        pkResult = pkE_OUTOFMEMORY;
        goto exit;
    }

    pkResult = pSegmentFetcher->Initialize( pMbrManifest );
    if( pkFAILED(pkResult) )
    {
        goto exit;
    }

    *ppFetcher = pSegmentFetcher;

exit:

    return( pkResult );
}
