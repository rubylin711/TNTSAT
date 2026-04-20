///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CFragmentDownloader.h"
#include <StringUtils.h>

static const size_t HEADER_BUFFER_SIZE         = 8*1024;
static const size_t NUM_BYTES_BOX_TYPESIZE     = 8;

CFragmentDownloader::CFragmentDownloader()
    : _pStreamerHttp(NULL)
    , _remainingMdatByteCount(0)
    , _mdatLength(0)
    , _headerBuffer(NULL)
    , _requestInProgress(false)
    , _socketError(eSocketErrorNone)
    , _socketPKResult(0)
    , _socketHttpResponse(0)
{
    _headerBuffer = NEW_NO_THROW byte[HEADER_BUFFER_SIZE];
}

CFragmentDownloader::~CFragmentDownloader()
{
    if ( _pStreamerHttp )
    {
        Close();

        delete _pStreamerHttp;
        _pStreamerHttp = NULL;
    }

    if ( _headerBuffer )
    {
        delete []_headerBuffer;
        _headerBuffer = NULL;
    }
}

//MP4Streamer interface
int CFragmentDownloader::RecvCount(_Out_bytecap_(dstlen) byte* dst, _In_ int dstlen, _In_ int dataLength, _In_opt_ int timeout)
{
    int bytesRead = 0;

    if ( dstlen < dataLength )
    {
        TRACE_ERROR(("RecvCount: destination buffer length (%d) less than read-count (%d)", dstlen, dataLength));
        return -1;
    }

    bool endOfHttp = false;
    while ( !endOfHttp && bytesRead < dataLength )
    {
        int c = _pStreamerHttp->Recv(dst + bytesRead, dataLength - bytesRead, &endOfHttp, timeout);
        if (c <= 0)
        {
            //Save the error state
            _socketError = _pStreamerHttp->GetSocketError();
            _socketPKResult = _pStreamerHttp->GetPKResult();
            _socketHttpResponse = _pStreamerHttp->GetHttpResponse();

            TRACE_ERROR(("RecvCount: failed err: %d pkResult: 0x%08x httpResponse: %d", 
                _socketError, _socketPKResult, _socketHttpResponse));
            return c;
        }
        bytesRead += c;
    }

    // getting mdat
    if( _mdatLength )
    {
        _remainingMdatByteCount -= dataLength;

        // check for inconsistent byte count
        if ( ( endOfHttp && _remainingMdatByteCount > 0 ) ||
             ( !endOfHttp && _remainingMdatByteCount == 0 ) )
        {
            TRACE_ERROR(("RecvCount: inconsistent byte count, endOfHttp (%s) _remainingMdatByteCount (%d)", 
                endOfHttp ? "TRUE" : "FALSE", _remainingMdatByteCount));

            return -1;
        }
    }

    return bytesRead;
}

pkRESULT CFragmentDownloader::RequestFragment( _In_ const std::wstring& strUrl )
{
    pkRESULT pkResult = pkS_OK;
    wstring url;

    if( _requestInProgress )
    {
        TRACE_ERROR(("There is a download already in progress"));
        pkResult = pkE_INVALID_REQUEST;
        goto exit;
    }

    // Send off the request
    pkResult = SendHttpRequest( strUrl );
    if( pkFAILED(pkResult) )
    {
        TRACE_ERROR(("SendHttpRequest failed url: %ls",strUrl.c_str()));
        goto exit;
    }

exit:
    return pkResult;
}

pkRESULT CFragmentDownloader::ReceiveHeader( _Out_ size_t* pcbTotalSize )
{
    pkRESULT pkResult = pkS_OK;
    *pcbTotalSize = 0;

    if( !_requestInProgress ) 
    {
        TRACE_ERROR(("ReceiveHeader called when there is no request in progress!"));
        pkResult = pkE_INVALID_REQUEST;
        goto exit;
    }

    // Header has not been received yet
    if ( !_mdatLength  )
    {
        uint32_t offset = 0;

        // Only try to get the header in one shot
        _mdatLength = _fmp4Parser.Prefetch(MP4Atom::eAtomType_mdat,
                                           static_cast<MP4Streamer*>(this),
                                           _headerBuffer,
                                           HEADER_BUFFER_SIZE,
                                           &offset);

        // 'mdat' was not found
        if( 0 == _mdatLength )
        {
            TRACE_ERROR(("Failed to find mdat box!"));
            Close();
            pkResult = pkE_FAIL;
            goto exit;
        }

        // 'mdat' length includes the size and type, the rest of the response
        // should be the 'mdat' data
        _remainingMdatByteCount = _mdatLength - NUM_BYTES_BOX_TYPESIZE;
    }

    *pcbTotalSize = _mdatLength - NUM_BYTES_BOX_TYPESIZE;

exit:
    return pkResult;
}

pkRESULT CFragmentDownloader::ReceiveFragmentData(
        _Out_bytecap_(cbMax) byte* pBuffer,
        _In_ size_t cbMax,
        _Out_ size_t* cbRead)
{
    pkRESULT pkResult = pkS_OK;
    int bytesRead;
    int bytes2Read;

    *cbRead = 0;

    if( !_requestInProgress )
    {
        TRACE_ERROR(("ReceiveHeader called when there is no request in progress!"));
        pkResult = pkE_INVALID_REQUEST;
        goto exit;
    }

    // if header has not been received yet, receive it on our own
    if ( !_mdatLength || !_remainingMdatByteCount)
    {
        TRACE_ERROR(("Must call ReceiveHeader before ReceiveFragmentData"));
        ASSERT(false);
        pkResult = pkE_INVALID_REQUEST;
        goto exit;
    }

    bytes2Read = (cbMax < _remainingMdatByteCount) ? (int)cbMax : (int)_remainingMdatByteCount;
    
    bytesRead = RecvCount(pBuffer, cbMax, bytes2Read);

    if (bytesRead > 0)
    {
        // entire response has been read
        if( 0 == _remainingMdatByteCount )
        {
            Close();
        }
    }
    else
    {
        TRACE_ERROR(("ReceiveFragmentData: RecvCount(%d) failed bytesRead!",bytes2Read, bytesRead));
        pkResult = pkE_FAIL;
        Close();
    }

    *cbRead = bytesRead;

exit:
    return pkResult;
}

// Close socket
pkRESULT CFragmentDownloader::Close()
{
    _requestInProgress = false;

    //Make sure the socket is closed
    if ( _pStreamerHttp->IsConnected() )
    {
        if( !_pStreamerHttp->Close() )
        {
            TRACE_ERROR(("Closing of streamer failed!"));
            return pkE_FAIL;
        }
    }

    return pkS_OK;
}

pkRESULT CFragmentDownloader::SendHttpRequest(_In_ const std::wstring& wsUrl)
{
    pkRESULT pkResult = pkS_OK;

    WStr2Str url( wsUrl );

    if (!_pStreamerHttp)
    {
        _pStreamerHttp = IStreamerHttp::CreateStreamerHttp();
    }

    if ( !_pStreamerHttp )
    {
        TRACE_ERROR(("CreateStreamerHttp failed for %s", url.c_str()));
        pkResult = pkE_OUTOFMEMORY;
        goto exit;
    }

    _requestInProgress = true;

    // zero out mdat length until it is found
    _mdatLength = 0;
    _remainingMdatByteCount = 0;

    // open the socket and send the request

    if( !(static_cast<IStreamer*>(_pStreamerHttp)->Connect( &url )) )
    {
         //Save the error state
        _socketError = _pStreamerHttp->GetSocketError();
        _socketPKResult = _pStreamerHttp->GetPKResult();
        _socketHttpResponse = _pStreamerHttp->GetHttpResponse();

        TRACE_ERROR(("Failed to tune '%s' err %d pkResult 0x%x httpResponse %d", 
                    url.c_str(), _socketError, _socketPKResult, _socketHttpResponse));
        Close();
        pkResult = pkE_FAIL;
        goto exit;
    }

exit:
    return pkResult;
}
