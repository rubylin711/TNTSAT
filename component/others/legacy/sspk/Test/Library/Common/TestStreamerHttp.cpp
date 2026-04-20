///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "TestStreamerHttp.h"
#include "PKTestSuite.h"
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

namespace StaticUtils
{
    string trim(const string& s, const char* trimArray = "\t" )
    {
        if (s.length() == 0)
            return s;
        size_t b = s.find_first_not_of(trimArray);
        size_t e = s.find_last_not_of(trimArray);
        if (b == string::npos)
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

    int split(const string& text, vector<string>& words, const string& separators)
    {
        return split_t<string>(text, words, separators);
    }

    string toLower(const string& ss)
    {
        string s(ss);
        for (size_t i = 0; i < s.length(); i++)
            s[i] = (char) tolower(s[i]);
        return s;
    }
}
static int s_GetLastSockError(void)
{
    // TODO: map SOCKET_E_... codes to WSA error numbers
    return (int)Socket_GetResultFromLastErr();
}

//================================================================
// Non-static methods
//================================================================

TestStreamerHttp::TestStreamerHttp()
    :_pSockAddrInfo(NULL)
{
    _tcpBufferSize = 0;

    Init();
}

TestStreamerHttp::~TestStreamerHttp()
{
    if ( _pSockAddrInfo )
    {
        Socket_FreeAddrInfo(_pSockAddrInfo);
    }
}

void TestStreamerHttp::Init(void)
{
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

SOCKET_HANDLE TestStreamerHttp::ConnectIP(void)
{
    int sock_result = 0;

    // On connect, clear the the request queue
    LogTestComment("ConnectIP clean up request queue");

    LogTestComment("TestStreamerHttp::ConnectIP for IPv%d", (SOCKET_FAMILY_INET == _pSockAddrInfo->ai_family) ? 4 : 6);

    //Open up the socket
    SOCKET_HANDLE hSock = Socket_Socket(_pSockAddrInfo->ai_family, _pSockAddrInfo->ai_socktype,  _pSockAddrInfo->ai_protocol);
    if (hSock == SOCKET_INVALID_HANDLE)
    {
        return SOCKET_INVALID_HANDLE;
    }
    LogTestComment("ConnectIPV4: Created socket: %d", hSock);

    if (_tcpBufferSize != 0)
    {
        sock_result = Socket_SetRecvBuffer(hSock, _tcpBufferSize);
        if (sock_result == SOCKET_FAILURE)
        {
            //Close the socket before leaving
            Socket_CloseSocket(hSock);
            
            return SOCKET_INVALID_HANDLE;
        }
        else
        {
            LogTestComment("ConnectIP: Receive buffer size set to %u", _tcpBufferSize);
        }
    }

    //Connect to the requested HTTP host
    if (Socket_Connect(hSock, _pSockAddrInfo->ai_addr, _pSockAddrInfo->ai_addrlen) < 0)
    {
        //Close the socket before leaving
        Socket_CloseSocket(hSock);
        
        return SOCKET_INVALID_HANDLE;
    }

    // Set initial Recv condition flags
    _startFlag = true;
    _isRecvWaiting = false;

    return hSock;
}

bool TestStreamerHttp::HttpRequestResponse(const std::string& server, const std::string& httpRequest, const std::string& body)
{
    string response;
    return HttpRequestResponse(server, httpRequest, body, response);
}

bool TestStreamerHttp::HttpRequestResponse(const std::string& server, const std::string& httpRequest, const std::string& body, std::string& response)
{
    _requestHeader = httpRequest;
    if ( _pSockAddrInfo )
    {
        Socket_FreeAddrInfo(_pSockAddrInfo);
    }

    _pSockAddrInfo = GetHostAddress(server, HTTP_CONNECT_DEFAULT_PORT);
    _socket = ConnectIP();
    if (SOCKET_INVALID_HANDLE == _socket)
    {
        return false;
    }
    if(SendRequest(httpRequest))
    {
        if(body != "")
        {
            if(!SendRequest(body))
            {
                return false;
            }
        }
        if(ReadResponse(response, 1000 * HTTPRESPONSETIMEOUT))
        {
            int result;
            if(ParseResponse(response, result))
            {
                if(result != HTTP_STATUS_OK)
                {
                    goto errorExit;
                }
            }
            else
            {
                goto errorExit;
            }
        }
        else
        {
            goto errorExit;
        }
    }
    else
    {
        goto errorExit;
    }

    Socket_Shutdown(_socket, SOCKET_D_BOTH);
    Socket_CloseSocket(_socket);
    return true;

errorExit:
    Socket_Shutdown(_socket, SOCKET_D_BOTH);
    Socket_CloseSocket(_socket);
    return false;
}

bool TestStreamerHttp::SendRequest(const string& src)
{
    size_t i = 0;

    const int8_t* pb = reinterpret_cast<const int8_t*>( src.c_str() );

    while( i < src.size() )
    {
        if (_socket == SOCKET_INVALID_HANDLE)
            return false;

        int bytesSent = ::Socket_Send( _socket, pb + i, src.size() - i, 0 );
        if (bytesSent <= 0)
            return false;

        i += bytesSent;
    }
    return true;
}

bool TestStreamerHttp::ReconnectIP()
{
    bool didReconnect = false;

    Socket_Shutdown(_socket, SOCKET_D_BOTH);
    Socket_CloseSocket(_socket);

    _socket = ConnectIP();
    if (SOCKET_INVALID_HANDLE == _socket)
    {
        // ConnectIPV4 sets error codes
        LogTestError("ReconnectIPV4 FAILED TO RECONNECT");
    }
    else if (!SendRequest(_requestHeader))
    {
        // give up if request fails
    }
    else
    {
        LogTestComment("ReconnectIP SendRequest: %s", _requestHeader.c_str());
        didReconnect = true;
    }

    return didReconnect;
}

bool TestStreamerHttp::ReadResponse(string& response, int timeout)
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
        recv_len = RecvInternal(recv_buffer, line_length, timeout);
        if (recv_len <= 0 )
        {
            _response_len = 0;
            _response_end = 0;

            // Give up if too many reconnects or no request header available
            if (reconnectCount++ >= HTTP_MAXRECONTTECT_ATTEMPTCOUNT
                || _requestHeader.length() == 0)
            {
                LogTestError("ReadResponse failed, retval:%d, WSAError %d, reconnect attempts %d",
                    recv_len, ::s_GetLastSockError(), reconnectCount);
                return false;
            }

            // Attempt to re-establish the connection and re-issue the request
            LogTestError("ReadResponse Recv result %d receiving request response - trying again",
                recv_len);

            if (!ReconnectIP())
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
        LogTestComment("No response end after %d read; continuing to read more...", _response_len);

        //Some adjustments in preparation for the next read
        line_length -= recv_len;
        recv_buffer += recv_len;
        _response_end = _response_len;
    }

    //If we've filled up the entire 2k buffer, and still didn't find end of response, stop.
    if ((line_length == 0) && (end == 0))
    {
        LogTestError("Failed, could not find end of response within %d data", sizeof(_socketBuffer));
        _response_len = 0;
        _response_end = 0;
        return false;
    }

    end += 4; // jump to end of CR/LF/CR/LF
    _response_end = static_cast<int32_t>(end - (char*)&_socketBuffer[0]);

    LogTestComment("ReadResponse: %d bytes received, %d bytes are response, %d bytes are leftover",
        _response_len, _response_end, _response_len - _response_end);

    response.assign((const char*)_socketBuffer, (uint32_t)_response_end);
    return true;
}

bool TestStreamerHttp::ParseResponse(string& response, int& http_result)
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
    
    _contentLen = 0;
    
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
            }
            else
            if (name == "Transfer-Encoding")
            {
                // TODO: 'transfer-encoding' such as 'chunked' is not supported yet.
                LogTestError("Transfer-Encoding:%s is not supported", value.c_str());
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
                    LogTestError("Connection:%s not recognized", value.c_str());
                }
            }
        }
    }
exit: 
    if (http_result < HTTP_STATUS_FIRST || http_result > HTTP_STATUS_LAST)
    {
        if (cHeaders > 0)
        {
            LogTestError("HTTP INVALID response: %s", str_result.c_str());
        }
        else
        {
            LogTestError("HTTP response is EMPTY");
        }
        return false;
    }

    //If there is erroneous result (<200, or >=400), need to clear out the remaining bytes
    if (((http_result < HTTP_STATUS_OK) || (http_result >= HTTP_STATUS_BAD_REQUEST))
        && (_contentLen > 0))
    {
        LogTestError("HTTP response error: %d, clearing %d bytes", http_result, _contentLen);

        uint32_t skipBytes = (uint32_t) _contentLen;
        while (skipBytes)
        {
            uint32_t len = min(skipBytes, sizeof(_socketBuffer));
            int rc = RecvInternal(_socketBuffer, len);
            if (rc <= 0)
            {
                LogTestError("Clearing bytes failed rc: %d len: %d skipBytes: %d contentLen: %d", 
                    rc, len, skipBytes, _contentLen);
                break;
            }
            skipBytes -= rc;
        }
    }
    return true;
}

int TestStreamerHttp::RecvInternal(byte* dst, int dstlen, int timeoutMs )
{
    int32_t sockResult = SOCKET_FAILURE;

    // Send error when closed
    if (!_socketConnected)
    {
        LogTestError("Recv when not connected");
        return -1;
    }

    // Start with any data remaining from the HTTP Response
    int copy_count = 0;
    int leftover_count = _response_len - _response_end;
    if (leftover_count > 0)
    {
        copy_count = (leftover_count >= dstlen) ? dstlen : leftover_count;

        LogTestComment("Recv: %d are leftover, %d to receive, %d copied",
            leftover_count, dstlen, copy_count);

        memcpy_s(dst, dstlen, &_socketBuffer[_response_end], copy_count);
        _response_end += copy_count;

        dst += copy_count;
        dstlen -= copy_count;

        // if the remaining data is >= requested length, just return
        if (dstlen == 0)
        {
            LogTestComment("dstlen == 0, return %d bytes", copy_count);
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
                // LogTestComment(("Recv returned %d bytes (recv:%d/copy:%d)", iResult, (int)sockResult, copy_count));
            }
        }
    }

    _isRecvWaiting = false;
    return iResult;
}

SOCKET_ADDR_INFO* TestStreamerHttp::GetHostAddress(_In_ const string& host, _In_ const string& port)
{
    SOCKET_ADDR_INFO* pAddrInfoList = NULL;

    if (!host.empty())
    {
        SOCKET_ADDR_INFO hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = SOCKET_FAMILY_UNSPEC;
        hints.ai_protocol = SOCKET_IPPROTO_TCP;
        hints.ai_socktype = SOCKET_STREAM;

        int rc = Socket_GetAddrInfo(
            host.c_str(), // const char *hostname,
            port.c_str(), // const char *servname,
            &hints,
            &pAddrInfoList );

        if (SOCKET_SUCCESS == rc)
        {
            if ((NULL != pAddrInfoList)
                && (NULL == pAddrInfoList->ai_addr))
            {
                pAddrInfoList = NULL;
                LogTestError("GetHostAddr FAIL: getaddrinfo succeeded but ai_addr is NULL!");
            }
        }
        else
        {
            pAddrInfoList = NULL;
            LogTestError("GetHostAddr FAIL: %d", rc);
        }
    }
    return pAddrInfoList;
}

bool TestStreamerHttp::Close(void)
{
    int sockResult = 0;
    
    if (_socket != SOCKET_INVALID_HANDLE)
    {
        sockResult = Socket_Shutdown(_socket, SOCKET_D_BOTH);

        int iTimeout = 1000 * HTTP_INITIALIZERECIEVE_TIMEOUT;

        // wait for a reasonable amount of time for any receive to complete
        while (_isRecvWaiting && iTimeout > 0)
        {
            Executive_Sleep(100); // polling interval chosen for low overhead
            iTimeout -= 100;
        }

        if (_isRecvWaiting)
        {
            LogTestError("Close timed out waiting for receive to complete");
        }

        sockResult = Socket_CloseSocket(_socket);
        LogTestComment("Closed socket:%d; result:%d", _socket, sockResult);
        _socket = SOCKET_INVALID_HANDLE;
    }
    return(0 == sockResult);
}

// ===============================================================================================================
// ===============================================================================================================
