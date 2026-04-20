///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "pkPAL.h"

#include "CXHttp.h"

#include "pkExecutive.h"
#include "pkSockets.h"

#include <string>

using namespace std;

#define HTTP_STATUS_OK                  200 // request completed
#define HTTP_STATUS_BAD_REQUEST         400 // invalid syntax
#define HTTP_STATUS_FIRST               100 // OK to continue with request
#define HTTP_STATUS_LAST                505 // HTTP version not supported

static const int HTTPRESPONSETIMEOUT = 20;
static const int HTTP_MAXRECONTTECT_ATTEMPTCOUNT = 1;
static const int HTTP_INITIALIZERECIEVE_TIMEOUT = 30; //HTTP initial receive timeout in seconds
static const int HTTP_SUBSEQUENTRECIEVE_TIMEOUT = 6; //HTTP subsequent receive timeout in seconds

//#define USE_SOCKET_DGRAM 1

// base PALPRINTMSG on the same condition as PRINTMSG
#ifdef PRINTMSG_ENABLED

#define PALPRINTMSG( cond, printf_exp ) ( ( cond ) ? ( Executive_DebugPrintf printf_exp ), 1 : 0 )

#else // PRINTMSG_ENABLED

#define PALPRINTMSG( cond, printf_exp )   ( void ) 0

#endif // PRINTMSG_ENABLED

#define PALPRINT_CXHTTP_VERBOSE 0
#define PALPRINT_CXHTTP_TRACE 0
#define PALPRINT_CXHTTP_ERROR 1


namespace StaticUtils
{
    static string trim(const string& s, const char* trimArray = "\t" )
    {
        if (s.length() == 0)
            return s;
        size_t b = s.find_first_not_of(trimArray);
        size_t e = s.find_last_not_of(trimArray);
        if (b == -1)
            return "";
        return string(s, b, e - b + 1);
    }

    template <class T>
    int split_t(const T& text, vector<T>& words, const T& separators)
    {
        size_t textLen = text.length();
        size_t start = text.find_first_not_of(separators, 0);
        while ((start >= 0) && (start < textLen))
        {
            size_t stop = text.find_first_of(separators,start);

            if ((stop < 0) || (stop > textLen))
                stop = textLen;

            words.push_back(text.substr(start, stop-start));
            start = text.find_first_not_of(separators, stop+1);
        }
        return (int)words.size();
    }

    static int split(const string& text, vector<string>& words, const string& separators)
    {
        return split_t<string>(text, words, separators);
    }

    static string toLower(const string& ss)
    {
        string s(ss);
        for (size_t i = 0; i < s.length(); i++)
            s[i] = (char) tolower(s[i]);
        return s;
    }

    static int GetHostAddr(const string& host)
    {
        uint32_t hostAddress = 0;

        if (!host.empty() && host != "0.0.0.0")
        {
            SOCKET_ADDR_INFO hints;
            memset(&hints,0,sizeof(hints));
            hints.ai_flags = 0;
            hints.ai_family = SOCKET_FAMILY_INET;
            hints.ai_protocol = SOCKET_IPPROTO_TCP;
#if USE_SOCKET_DGRAM
            hints.ai_socktype = SOCKET_DGRAM;
#else
            hints.ai_socktype = SOCKET_STREAM;
#endif
            SOCKET_ADDR_INFO *pAddrInfoList;

            int rc = Socket_GetAddrInfo(
                host.c_str(), // const char *hostname,
                NULL, // const char *servname,
                &hints,
                &pAddrInfoList );

            if (SOCKET_SUCCESS == rc)
            {
                if (NULL != pAddrInfoList)
                {
                    if (NULL == pAddrInfoList->ai_addr)
                    {
                        PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("GetHostAddr FAIL: getaddrinfo succeeded but ai_addr is NULL!\n"));
                    }
                    else
                    {
                        // This only uses the first list item
                        uint8_t* pAddrData = (uint8_t*)(pAddrInfoList->ai_addr->sa_data);
                        hostAddress = (pAddrData[2] << 24) +  (pAddrData[3] << 16) +  (pAddrData[4] << 8) +  pAddrData[5];
                        hostAddress = Socket_htonl(hostAddress);
                        Socket_FreeAddrInfo(pAddrInfoList);
                    }
                }
            }
            else
            {
                PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("GetHostAddr FAIL: %d\n", rc));
            }
        }
        return (int) hostAddress;
    }

    static uint32_t GetHostAddressPort(const string& host, const string& port, uint16_t& hostPort)
    {
        hostPort = 0;

        if (!port.empty())
        {
            uint16_t ipPort = (uint16_t)::atoi(port.c_str());
            hostPort = Socket_htons( ipPort );
        }

        return (uint32_t)GetHostAddr(host);
    }
}

//================================================================
// Non-static methods
//================================================================

CXHttp::CXHttp()
{
    _tcpBufferSize = 0;

    Init();
}

CXHttp::~CXHttp()
{
    Close();
}

void CXHttp::Init(void)
{
    _host = 0;
    _port = 0;
    _socket = SOCKET_INVALID_HANDLE;
    _requestHeader = "";
    _startFlag = true;
    _isRecvWaiting = false;
    _response_end = 0;
    _response_len = 0;
    _isPersistent = true;
    _contentLen = 0;
    _contentBytesRead = 0;
    
    _socketConnected = true;
}

void CXHttp::Close(void)
{
    if (SOCKET_INVALID_HANDLE != _socket)
    {
        Socket_Shutdown(_socket, SOCKET_D_BOTH);
        Socket_CloseSocket(_socket);
        _socket = SOCKET_INVALID_HANDLE;
    }
}

SOCKET_HANDLE CXHttp::ConnectIPV4(void)
{
    int sock_result = 0;

    //Open up the socket
#if USE_SOCKET_DGRAM
    SOCKET_HANDLE hSock = Socket_Socket(SOCKET_FAMILY_INET, SOCKET_DGRAM, SOCKET_IPPROTO_TCP);
#else
    SOCKET_HANDLE hSock = Socket_Socket(SOCKET_FAMILY_INET, SOCKET_STREAM, SOCKET_IPPROTO_IP);
#endif
    if (hSock == SOCKET_INVALID_HANDLE)
    {
        PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("ConnectIPV4: Socket_Socket failed!\n"));
        return SOCKET_INVALID_HANDLE;
    }
    PALPRINTMSG( PALPRINT_CXHTTP_TRACE, ("ConnectIPV4: Created socket: %d\n", (int) hSock));

    if (_tcpBufferSize != 0)
    {
        sock_result = Socket_SetRecvBuffer(hSock, _tcpBufferSize);
        if (sock_result == SOCKET_FAILURE)
        {

            //Close the socket before leaving
            Socket_CloseSocket(hSock);

            PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("ConnectIPV4: Socket_SetRecvBuffer failed\n"));
            return SOCKET_INVALID_HANDLE;
        }
        else
        {
            PALPRINTMSG( PALPRINT_CXHTTP_TRACE, ("ConnectIPV4: Receive buffer size set to %u\n", _tcpBufferSize));
        }
    }

    //Use default port if none specified
    if (_port == 0)
    {
        _port = Socket_htons(HTTP_CONNECT_DEFAULT_PORT);
    }

    //Connect to the requested HTTP host
    SOCKET_SOCKADDR_IN server;
    memset(&server, 0, sizeof(server));
    server.sin_family = SOCKET_FAMILY_INET;
    server.sin_addr.s_addr = _host;
    server.sin_port = _port;
    if (Socket_Connect(hSock, (SOCKET_SOCKADDR*)&server, sizeof(server)) < 0)
    {
        //Close the socket before leaving
        Socket_CloseSocket(hSock);

        PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("ConnectIPV4: Socket_Connect failed: host[%u] port[%u]\n", 
            (unsigned int)_host, 
            (unsigned int)_port));
        return SOCKET_INVALID_HANDLE;
    }

    // Set initial Recv condition flags
    _startFlag = true;
    _isRecvWaiting = false;

    return hSock;
}

bool CXHttp::HttpRequestResponse(const std::string& server, const std::string& httpRequest, const std::string& body, std::string& response)
{
    bool isOk = false;

    std::string host;
    std::string port;

    size_t portIx = server.find_first_of(":");
    if (portIx != std::string::npos)
    {
        port = server.substr(portIx + 1);
        host = server.substr(0, portIx);
    }
    else
    {
        host = server;
    }
    
    PALPRINTMSG( PALPRINT_CXHTTP_TRACE, ("HttpRequestResponse: host[%s] port[%s]\n", host.c_str(), port.c_str()));
    
    _host = StaticUtils::GetHostAddressPort(host, port, _port);
    
    _requestHeader = httpRequest;

    _socket = ConnectIPV4();
    
    if (SOCKET_INVALID_HANDLE == _socket)
    {
        goto exit;
    }
    
    PALPRINTMSG( PALPRINT_CXHTTP_VERBOSE, ("HttpRequestResponse: request:\n%s\n", httpRequest.c_str()));
    
    if(!SendRequest(httpRequest.c_str(),httpRequest.size()))
    {
        goto errorExit;
    }

    if(body != "")
    {
        PALPRINTMSG( PALPRINT_CXHTTP_VERBOSE, ("HttpRequestResponse: body:\n%s\n", body.c_str()));
        
        if(!SendRequest(body.c_str(),body.size()))
        {
            goto errorExit;
        }
    }
    
    if(ReadResponse(response, 1000 * HTTPRESPONSETIMEOUT))
    {
        int result;
        if(!ParseResponse(response, result))
        {
            goto errorExit;
        }

        if(result != HTTP_STATUS_OK)
        {
            goto errorExit;
        }
    }

    isOk = true;
    goto exit;

errorExit:
    Close();
    
exit:    
    return isOk;
}

bool CXHttp::SendRequest(const char* src, int len)
{
    int i = 0;
    while (i < len)
    {
        if (_socket == SOCKET_INVALID_HANDLE)
            return false;

        int bytesSent = ::Socket_Send(_socket, (const int8_t*) &src[i], len - i, 0);
        if (bytesSent <= 0)
            return false;

        i += bytesSent;
    }
    return true;
}

bool CXHttp::ReconnectIPV4()
{
    bool didReconnect = false;

    Socket_Shutdown(_socket, SOCKET_D_BOTH);
    Socket_CloseSocket(_socket);

    _socket = ConnectIPV4();
    if (SOCKET_INVALID_HANDLE == _socket)
    {
        // ConnectIPV4 sets error codes
        PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("ReconnectIPV4 FAILED TO RECONNECT\n"));
    }
    else if (!SendRequest(_requestHeader.c_str(), _requestHeader.size()))
    {
        // give up if request fails
    }
    else
    {
        PALPRINTMSG( PALPRINT_CXHTTP_TRACE, ("ReconnectIPV4 SendRequest: %s\n", _requestHeader.c_str()));
        didReconnect = true;
    }

    return didReconnect;
}

bool CXHttp::ReadResponse(string& response, int timeout)
{
    int reconnectCount = 0;
    
    if (!_socketConnected)
        return false;

    //Read Http response and headers
    int line_length = sizeof(_socketBuffer)-1;

    int recv_len = 0;
    char* end = 0;
    byte* recv_buffer = (byte*)_socketBuffer;

    //Init member variables
    _response_len = 0;
    _response_end = 0;

    //Keep reading until our buffer is filled up,
    //or the end-of-response is found.
    while (line_length > 0)
    {
        //Try read some data
        recv_len = Recv(recv_buffer, line_length, timeout);
        if (recv_len <= 0 )
        {
            _response_len = 0;
            _response_end = 0;

            // Give up if too many reconnects or no request header available
            if (reconnectCount++ >= HTTP_MAXRECONTTECT_ATTEMPTCOUNT
                || _requestHeader.length() == 0)
            {
                PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("ReadResponse Recv failed: retval[%d] hr[0x%X] reconnect attempts[%d]\n",
                    recv_len, Socket_GetResultFromLastErr(), reconnectCount));
                return false;
            }

            // Attempt to re-establish the connection and re-issue the request
            PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("ReadResponse Recv result %d receiving request response - trying again\n",
                recv_len));

            if (!ReconnectIPV4())
            {
                return false;
            }

            // reset loop state
            line_length = sizeof(_socketBuffer)-1;
            recv_len = 0;
            end = 0;
            recv_buffer = (byte*)_socketBuffer;
            continue;
        }

        //Calculate response_len
        _response_len += recv_len;
        _socketBuffer[_response_len] = 0;

        //Now try to find the end of response
        end = strstr((char*)_socketBuffer, "\r\n\r\n");
        if (NULL != end)
        {
            //Found end of response, break out of the Recv loop.
            break;
        }
        PALPRINTMSG( PALPRINT_CXHTTP_TRACE, ("No response end after %d read; continuing to read more...\n", _response_len));

        //Some adjustments in preparation for the next read
        line_length -= recv_len;
        recv_buffer += recv_len;
        _response_end = _response_len;
    }

    //If we've filled up the entire 2k buffer, and still didn't find end of response, stop.
    if ((line_length == 0) && (end == 0))
    {
        PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("Failed, could not find end of response within %d data\n", sizeof(_socketBuffer)));
        _response_len = 0;
        _response_end = 0;
        return false;
    }

    end += 4; // jump to end of CR/LF/CR/LF
    _response_end = end - (char*)&_socketBuffer[0];

    PALPRINTMSG( PALPRINT_CXHTTP_VERBOSE, ("ReadResponse: %d bytes received, %d bytes are response, %d bytes are leftover\n",
        _response_len, _response_end, _response_len - _response_end));

    response.assign((const char*)_socketBuffer, (uint32_t)_response_end);

    PALPRINTMSG( PALPRINT_CXHTTP_VERBOSE, ("ReadResponse: response headers:\n%s\n", response.c_str()));
    
    return true;
}

bool CXHttp::ParseResponse(string& response, int& http_result)
{
    //Check result code
    http_result = -1;
    vector<string> headers;
    string str_result;
    int cHeaders;

    cHeaders = StaticUtils::split(response, headers, "\r\n");

    //No headers?
    if (0 == cHeaders)
    {
        goto exit;
    }

    //Parse and validate the response status line
    str_result = headers[0];

    if (str_result.find("HTTP/1.0 ") == 0)
    {
        // HTTP 1.0 does not support persistent connections
        _isPersistent = false;
    }
    else if (str_result.find("HTTP/1.1 ") != 0)
    {
        // Only HTTP 1.0 or 1.1 is supported
        goto exit;
    }

    //Get the status-code string
    str_result = str_result.substr(9);

    //There should be a SPACE after the 3 digit status code
    if (str_result.find(' ') != 3)
    {
        goto exit;
    }

    //There should be a REASON-PHRASE after the SPACE
    if (str_result.substr(4).empty())
    {
        goto exit;
    }

    //Now convert the string to integer to get the status code
    http_result = atoi(str_result.c_str());

    //Catch non-integer string or code out of range
    if (http_result < HTTP_STATUS_FIRST || http_result > HTTP_STATUS_LAST)
    {
        goto exit;
    }
    
    //ParseUrl the rest response headers
    for (int i = 1; i < (int)headers.size(); i++)
    {
        const char* h = headers[i].c_str();
        const char* s = strchr(h, ':');

        if (s != NULL && s < (s + strlen(h)))
        {
            string name = StaticUtils::trim(string(h, s-h));
            string value = StaticUtils::trim(string(s+1));

            if (name == "Location")
            {
                _location = value;
            }
            else
            if (name == "Content-Length")
            {
                _contentLen = atoi(value.c_str());
                _contentBytesRead = 0;
            }
            else
            if (name == "Transfer-Encoding")
            {
                // TODO: 'transfer-encoding' such as 'chunked' is not supported yet.
                PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("Transfer-Encoding:%s is not supported\n", value.c_str()));
                return false;
            }
            else
            if (name == "Connection")
            {
                string v = StaticUtils::toLower(value);
                if (v == "keep-alive")
                {
                    _isPersistent = true;
                }
                else
                if (v == "close")
                {
                    _isPersistent = false;
                }
                else
                {
                    PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("Connection:%s not recognized\n", value.c_str()));
                }
            }
        }
    }
exit: 
    if (http_result < HTTP_STATUS_FIRST || http_result > HTTP_STATUS_LAST)
    {
        if (cHeaders > 0)
        {
            PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("HTTP INVALID response: %s\n", str_result.c_str()));
        }
        else
        {
            PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("HTTP response is EMPTY\n"));
        }
        return false;
    }

    //If there is erroneous result (<200, or >=400), need to clear out the remaining bytes
    if (((http_result < HTTP_STATUS_OK) || (http_result >= HTTP_STATUS_BAD_REQUEST))
        && (_contentLen > 0))
    {
        PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("HTTP response error: %d, clearing %d bytes\n", 
            http_result, 
            _contentLen));

#if PALPRINT_CXHTTP_VERBOSE
        std::string responseBody = "";
#endif // PALPRINT_CXHTTP_VERBOSE

        uint32_t skipBytes = (uint32_t) _contentLen;
        while (skipBytes)
        {
            uint32_t len = min(skipBytes, sizeof(_socketBuffer)-1);
            int rc = Recv(_socketBuffer, len);
            if (rc <= 0)
            {
                PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("Clearing bytes failed rc: %d len: %d skipBytes: %d contentLen: %d\n", 
                    rc, 
                    len, 
                    skipBytes, 
                    _contentLen));
                break;
            }
            skipBytes -= rc;

#if PALPRINT_CXHTTP_VERBOSE
            _socketBuffer[rc] = 0;
            responseBody += (char*)(_socketBuffer);
#endif // PALPRINT_CXHTTP_VERBOSE
        }

#if PALPRINT_CXHTTP_VERBOSE
        PALPRINTMSG( PALPRINT_CXHTTP_VERBOSE, ("ReadResponse: skipped response body:\n%s\n", responseBody.c_str()));
#endif // PALPRINT_CXHTTP_VERBOSE
        
    }
    return true;
}

int CXHttp::Recv(byte* dst, int dstlen, int timeoutMs/* = 0*/)
{
    int32_t sockResult = SOCKET_FAILURE;

    // Send error when closed
    if (!_socketConnected)
    {
        PALPRINTMSG( PALPRINT_CXHTTP_ERROR, ("Recv when not connected\n"));
        return -1;
    }

    // Start with any data remaining from the HTTP Response
    int copy_count = 0;
    int leftover_count = _response_len - _response_end;
    if (leftover_count > 0)
    {
        copy_count = (leftover_count >= dstlen) ? dstlen : leftover_count;

        PALPRINTMSG( PALPRINT_CXHTTP_TRACE, ("Recv: %d are leftover, %d to receive, %d copied\n",
            leftover_count, dstlen, copy_count));

        memcpy_s(dst, dstlen, &_socketBuffer[_response_end], copy_count);
        _response_end += copy_count;

        dst += copy_count;
        dstlen -= copy_count;

        // if the remaining data is >= requested length, just return
        if (dstlen == 0)
        {
            PALPRINTMSG( PALPRINT_CXHTTP_TRACE, ("dstlen == 0, return %d bytes\n", copy_count));
            _contentBytesRead += copy_count;
            return copy_count;
        }
    }

    // Set receive timeout to given or based on whether this is first Recv
    struct SOCKET_TIMEVAL tvSelectTimeout;

    if (timeoutMs > 0)
    {
        tvSelectTimeout.tv_sec = timeoutMs / 1000;
        tvSelectTimeout.tv_usec = (timeoutMs % 1000) * 1000;
    }
    else
    {
        if (_startFlag)
        {
            tvSelectTimeout.tv_sec = HTTP_INITIALIZERECIEVE_TIMEOUT;
        }
        else
        {
            tvSelectTimeout.tv_sec = HTTP_SUBSEQUENTRECIEVE_TIMEOUT;
        }
        tvSelectTimeout.tv_usec = 0;
    }

    _startFlag = false; // in all cases, now no longer the first Recv

    // Set up file descriptor set for Socket_Select
    SOCKET_FD_SET fdRead;

    SOCKET_FD_ZERO(&fdRead);

    SOCKET_FD_SET(_socket, &fdRead);

    _isRecvWaiting = true;

    int iResult = -1; // default to error return

    // Note: A check of _socketConnected must come after _isRecvWaiting is set
    //       to catch the case where Close from another thread has closed the socket.
    //       These two flags form an interlock this way.

    if (_socketConnected)
    {
        sockResult = Socket_Select(0, &fdRead, NULL, NULL, &tvSelectTimeout);

        if (0 == sockResult) // zero ready implies timeout
        {
            //Timeout
        }
        else if (SOCKET_FAILURE == sockResult)
        {
            //Failure
        }
        else if (_socketConnected)
        {
            //Read data that is now ready
            sockResult = Socket_Recv(_socket, (int8_t*)dst, (int32_t)dstlen, 0);
            if (SOCKET_FAILURE == sockResult)
            {
                //Note: a "would block" is considered an error here since Socket_Select indicated receive ready
            }
            else // sockResult is number of bytes received
            {
                _contentBytesRead += copy_count + (int)sockResult;
                iResult = (int)sockResult + copy_count;
                PALPRINTMSG( PALPRINT_CXHTTP_VERBOSE, ("Recv returned %d bytes (recv:%d/copy:%d)\n", 
                    iResult, 
                    (int)sockResult, 
                    copy_count));
            }
        }
    }

    _isRecvWaiting = false;
    return iResult;
}

// ===============================================================================================================
// ===============================================================================================================
