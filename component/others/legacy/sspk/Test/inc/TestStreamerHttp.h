///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "pkExecutive.h"
#include "pkSockets.h"

#include "Trace.h"

#include <string>
#include <map>
#include <vector>
using namespace std;

static const string HTTP_PROXY_DEFAULT_PORT   = "80";
static const string HTTP_CONNECT_DEFAULT_PORT = "80";


// ===============================================================================================================
// Streamer to read from Http server
// ===============================================================================================================

class TestStreamerHttp
{
public:
    TestStreamerHttp();
    virtual ~TestStreamerHttp();

    bool        Close(void);
    uint32_t    GetContentLength(void) const { return _contentLen; }
    bool        HttpRequestResponse(const std::string& server, const std::string& httpRequest, const std::string& body);
    bool        HttpRequestResponse(const std::string& server, const std::string& httpRequest, const std::string& body, std::string& response);

private:
    void   Init(void);

    //Connect to _host:_port
    SOCKET_HANDLE       ConnectIP(void);
    bool                ReconnectIP();
    bool                SendRequest(const string& src);
    bool                ReadResponse(std::string& response, int timeout);
    bool                ParseResponse(std::string& response, int& http_result);
    int                 RecvInternal(__out_ecount(dstlen) byte* dst, int dstlen, int timeout = 0);
    SOCKET_ADDR_INFO*   GetHostAddress(_In_ const string& host, _In_ const string& port);

protected:
    //Whether this streamer is currently connected
    bool             _socketConnected;

    //Size of the tcp receive buffer to be used
    uint32_t      _tcpBufferSize;

    //Temporary buffer used for AV data packet processing
    uint8_t       _socketBuffer[2048];

private:
    //Host address info
    SOCKET_ADDR_INFO*   _pSockAddrInfo;
    
    //Socket to use
    SOCKET_HANDLE _socket;

    //Flag to indicate receive is in progress
    bool            _isRecvWaiting;

    //Allow special case handling for the first Recv
    bool            _startFlag;

    //HTTP request sent by latest SendHttpRequest
    //   Can be re-issued by ReadResponse in event of TCP error
    std::string     _requestHeader;

    //Leftover data after parsing response
    int32_t           _response_end;
    int32_t           _response_len;

    //Http Redirect location
    std::string     _location;
    //Http requested content length
    int32_t           _contentLen;
    int32_t           _contentBytesRead;

    //Indicates whether connection is expected to be persistent
    //(true by default unless HTTP/1.0 or receive Connection: close)
    bool                _isPersistent;
    
};
