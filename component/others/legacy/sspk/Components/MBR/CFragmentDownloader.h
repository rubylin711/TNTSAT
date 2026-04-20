///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "IStreamerHttp.h"
#include "MP4Parser.h"
#include "CChunkManifest.h"

using namespace MBR;

class CFragmentDownloader : public MP4Streamer
{
public:

    CFragmentDownloader();
    ~CFragmentDownloader();

    //MP4Streamer interface
    __override int RecvCount(
                    _Out_bytecap_(dstlen) byte* dst,
                    _In_ int dstlen,
                    _In_ int dataLength,
                    _In_opt_ int timeout = 0
                    );

    // Creates the HTTP socket and sends a request for the fragment
    pkRESULT RequestFragment( _In_ const std::wstring& strUrl );

    // Receives and skip the fragment header (the 'moof' box plus the 'mdat' header)
    // On success *pcbTotalSize contains the size of the ?mdat? box
    pkRESULT ReceiveHeader(
                    _Out_ size_t* pcbTotalSize );

    // Receives the actual data. This may be called multiple times to
    // receive more data up to the total size returned by ReceiveHeader.
    // When all of the data is received, the socket is closed
    pkRESULT ReceiveFragmentData(
                    _Out_bytecap_(cbMax) byte* pBuffer, // buffer holding the data
                    _In_ size_t cbMax,                  // max size to read at this time
                    _Out_ size_t* cbRead);              // number of bytes read

    // Abort the current request and close the socket
    pkRESULT Close();


private:

    pkRESULT SendHttpRequest(
                    _In_ const std::wstring& wurl);

    IStreamerHttp*              _pStreamerHttp;
    FMP4Parser                  _fmp4Parser;

    // remaining bytes in the mdat box
    uint32_t                    _remainingMdatByteCount;

    // size found in 'mdat' box
    uint32_t                    _mdatLength;

    // buffer to get to the the 'mdat' data
    byte*                       _headerBuffer;

    // currently only process one request at a time
    bool                        _requestInProgress;

    //Socket specific error
    eSocketError                _socketError;
    pkRESULT                    _socketPKResult;
    int                         _socketHttpResponse;
};
