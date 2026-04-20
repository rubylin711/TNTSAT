///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "pkPAL.h"

#include "pkSockets.h"

#include <string>

static const uint16_t HTTP_PROXY_DEFAULT_PORT   = 80;
static const uint16_t HTTP_CONNECT_DEFAULT_PORT = 80;


// ===============================================================

class CXHttp
{
public:
    CXHttp();
    virtual ~CXHttp();

    void          Close(void);
    uint32_t      GetContentLength(void) const { return _contentLen; }
    bool          HttpRequestResponse(const std::string& server, const std::string& httpRequest, const std::string& body, std::string& response);
    int           Recv(byte* dst, int dstlen, int timeout = 0);

private:
    void          Init(void);
    SOCKET_HANDLE ConnectIPV4(void);
    bool          ReconnectIPV4();
    bool          SendRequest(const char* src, int len);
    bool          ReadResponse(std::string& response, int timeout);
    bool          ParseResponse(std::string& response, int& http_result);

protected:
    //Whether this streamer is currently connected
    bool          _socketConnected;

    //Size of the tcp receive buffer to be used
    uint32_t      _tcpBufferSize;

    //Temporary buffer used for response header processing
    uint8_t       _socketBuffer[2048];

private:
    //Host address and port
    uint32_t      _host;
    uint16_t      _port;

    //Socket to use
    SOCKET_HANDLE _socket;

    //Flag to indicate receive is in progress
    bool          _isRecvWaiting;

    //Allow special case handling for the first Recv
    bool          _startFlag;

    //HTTP request sent by latest SendHttpRequest
    //   Can be re-issued by ReadResponse in event of TCP error
    std::string   _requestHeader;

    //Leftover data after parsing response
    int32_t       _response_end;
    int32_t       _response_len;

    //Http Redirect location
    std::string   _location;
    
    //Http requested content length
    int32_t       _contentLen;
    int32_t       _contentBytesRead;

    //Indicates whether connection is expected to be persistent
    //(true by default unless HTTP/1.0 or receive Connection: close)
    bool          _isPersistent;
    
};
