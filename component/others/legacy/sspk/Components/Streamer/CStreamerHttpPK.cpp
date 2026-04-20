///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IStreamerHttp.h"
#include "pkExecutive.h"
#include "pkSockets.h"

#include "StringUtils.h"
#include "Trace.h"

#include "CAVEngineConfiguration.h"

#include <string>
#include <map>
#include <vector>
#include <deque>
#include <sys/time.h>

using namespace std;

// #define STREAMER_SPEW
#if defined(STREAMER_SPEW)
#define STREAMER_TRACE(x) TRACE(x)
#else
#define STREAMER_TRACE(x)
#endif

static const char* c_szHttpProxyDefaultPort   = "80";
static const char* c_szHttpConnectDefaultPort = "80";
static const char* c_szHttpResponseHeaderTerminator = "\r\n\r\n";
static const int c_strlenHttpResponseHeaderTerminator = 4;
static const char* c_szHttpResponseElementTerminator = "\r\n";
static const int c_strlenHttpResponseElementTerminator = 2;
static const char* c_szHttpResponseChunkedExtensionTerminator = ";";
static const int32 c_LocalBufferSize = 2048;


// Local Buffer class to receive response header and intermediate data for chunked
// transfer encoding (chunk size and chunk data termination).  It will be considered
// a failure, if any of these are not entirely found within the buffer length (c_LocalBufferSize).
class LocalBuffer
{
public:
    LocalBuffer()
        : _startIdx(0)
        , _count(0)
    {
        _buffer[0] = 0;
    }
        
    ~LocalBuffer()
    {
    }

    int32 Count() const     { return _count; }
    int32 Size() const      { return c_LocalBufferSize - 1; }  // reserve 1 byte to put null for string operations
    int32 Space() const     { return (Size() - _count - _startIdx); }
    void Reset()            { _startIdx = 0; _count = 0; }
    byte* BasePtr()         { return _buffer; }
    byte* StartPtr()        { return (_buffer + _startIdx); }
    
    char* FindStr(const char* str)
    {
        return strstr((char*)StartPtr(), str);
    }

    byte* EndPtr()                     
    { 
        ASSERT(_startIdx + _count <= Size()); 
        return (_buffer + _startIdx + _count); 
    }

    void IncreaseCount(int32 val)   
    {  
        _count += val; 
        ASSERT(_startIdx + _count <= Size());
        
        _buffer[_startIdx + _count] = 0;
    }
   
    void AdvanceStart(int32 val)     
    {  
        _startIdx += val;
        
        _count -= val;
        ASSERT(_count >= 0);
        ASSERT(_startIdx + _count <= Size());
        if (0 == _count)
        {
            _startIdx = 0;
        }
    }

    void ShiftData()
    {
        // shift leftover data to the front of the buffer
        if (_startIdx > 0 && Count() > 0)
        {
            ASSERT(_startIdx + _count <= Size());
            memcpy_s(_buffer, c_LocalBufferSize, _buffer + _startIdx, Count());
            _startIdx = 0;
        }
    }

private:
    byte _buffer[c_LocalBufferSize];
    int32 _startIdx;
    int32 _count;
};

// ===============================================================================================================
// Streamer to read from Http server
// ===============================================================================================================

class CStreamerHttp : public IStreamerHttp
{
public:
    CStreamerHttp();
    virtual ~CStreamerHttp();

public:
    //IStreamer interfaces
    __override bool Connect( _Inout_ string* pUrl );
    __override bool Close(void);
    __override int  Recv( _Out_cap_(dstlen) byte* dst, _In_ int dstlen, _Out_ bool* pEndOfHttp, _In_opt_ int timeoutMs = 0 );
    __override bool Command( _In_ const string& command, _In_ const vector<string>& args);

#if 0 // Note: IStreamer contains the following members initialized by it's constructor:
protected:
    //A generic lock used by various streamer
    mutable Lockable _socketLock;

    //Whether this streamer is currently connected
    bool             _socketConnected;
    //streamer specific error
    CSocketError     _socketError;
    //pkResult if any
    pkRESULT              _socketPKResult;
    //http response if any
    int                   _socketHttpResponse;
#endif

public:

    //IStreamerHttp interfaces
    __override bool    Connect( _In_ string* pUrl, _In_ const string& extraHeader, _Out_ string* pResponse, ERedirect eRedirect );
    __override bool    SendHttpRequest( _In_ const CTuneRequest& tuneRequest, _In_ const string& extraHeader );
    __override bool    RecvHttpResponse( _Out_ string* pResponse );
    __override bool    HttpRequestResponse( _In_ const CTuneRequest& tuneRequest, _In_ const string& extraHeader, _Out_ string* pResponse );
    __override bool    GetResponseHeader( _In_ const char* pszHeader, _Out_ string* pValue );
    __override bool    GetResponseSessionIdHeader( _Out_ string* pValue );
    __override void    SetWindowTimeout(uint32 timeout) { _tcpWindowTimeoutMs = timeout; }
    __override void    SetTcpRecvBuffSize(uint32 size) { _tcpBufferSize = size; }
    __override uint32  GetTcpWindowSize(void);

private:
    void               Init(void);
    SOCKET_ADDR_INFO*  GetHostAddress(_In_ const char* host, _In_ const char* port);
    SOCKET_HANDLE      ConnectIP(void);
    bool               ReconnectIP(void);
    bool               InternalConnect( _In_ CTuneRequest& tuneRequest, _In_ const string& extraHeader, _Out_ string* pResponse );
    bool               SendRequest( _In_ const string& src);
    bool               ReadResponse( _Out_ string* pResponse, int timeout );
    bool               ParseResponse( _In_ const string& response, _Out_ int* pHttpResult );
    int                RecvInternal( _Out_cap_(dstlen) byte* dst, _In_ int dstlen, _In_opt_ int timeoutMs = 0 );
    int                RecvFromSocket( _Out_cap_(dstlen) byte* dst, _In_ int dstlen, _In_ int timeoutMs );
    bool               SendHttpRequest( _In_ const string& requestHeader );
    int32              ParseChunkSize(_Out_ int32* pChunkSizeLength);
    int32              GetChunkSize();
    bool               GetChunkDataEnd();
    int                CopyFromLocalBuffer(_Out_cap_(dstlen) byte* dst, _In_ int dstlen);
    char*              RecvUntilStrInLocalBuffer(_In_ const char* str);
    bool               IsResponseComplete();
    void               SetBufferStartIndex(int value);

private:
    //Size of the tcp receive buffer to be used
    uint32              _tcpBufferSize;

    // buffer used for receiving header and intermediate data for chunked transfer encoding
    LocalBuffer         _socketBuffer;

    //Host address info
    SOCKET_ADDR_INFO*   _pSockAddrInfo;

    //Host
    std::string         _Host;
    //Port
    std::string         _Port;


    //Socket to use
    SOCKET_HANDLE _socket;

    //Flag to indicate receive is in progress
    bool                _isRecvWaiting;

    //Allow special case handling for the first Recv
    bool                _startFlag;

    //TCP window update timeout hint
    //   Milliseconds before streamer sends to host to update TCP window
    //   Zero implies disabled
    uint32              _tcpWindowTimeoutMs;

    //HTTP request sent by latest SendHttpRequest
    //   Can be re-issued by ReadResponse in event of TCP error
    string              _requestHeader;

    //HTTP result from RecvHttpResponse
    int                 _httpResult;

    //Http Redirect location
    string              _location;
    //Http requested content length
    int32               _contentLen;
    int32               _contentBytesRead;

    //Optional field in http header
    string              _eTag;

    //Indicates whether connection is expected to be persistent
    //(true by default unless HTTP/1.0 or receive Connection: close)
    bool                _isPersistent;

    //Response received
    map<string,string>  _responses;

    //Queue to track sent request where their responses are not completely received
    deque<string>       _sendHttpRequestQueue;

    //Force a fake http response error
    bool                _fakeHttpResponseError;
    int                 _fakeHttpResponseCode;
    int                 _fakeHttpResponseCount;

    // Chunked transfer-encoding
    enum ETransferEncoding
    {
        eTransferEncoding_none = 0,
        eTransferEncoding_Chunked_inProgress,
        eTransferEncoding_Chunked_completed
    };

    ETransferEncoding   _transferEncodingState;
    int32               _chunkSize;
};

// ===============================================================================================================
// ===============================================================================================================

static pkRESULT s_GetLastSockError(void)
{
    return Socket_GetResultFromLastErr();
}

// ===============================================================================================================
// Streamer implementation to read from Http server
// ===============================================================================================================

//================================================================
// Class static methods
//================================================================

IStreamerHttp* IStreamerHttp::CreateStreamerHttp()
{
    return static_cast<IStreamerHttp*>(new CStreamerHttp);
}

//================================================================
// Non-static methods
//================================================================

CStreamerHttp::CStreamerHttp()
    :_pSockAddrInfo(NULL)
{
    _tcpBufferSize = 0;

    Init();
}

CStreamerHttp::~CStreamerHttp()
{
    if ( _pSockAddrInfo )
    {
        //Socket_FreeAddrInfo(_pSockAddrInfo);
        Socket_FreeAddrInfo_DnsCache((char *)_Host.c_str(), (char *)_Port.c_str(), _pSockAddrInfo);
    }
}

void CStreamerHttp::Init(void)
{
    _socket = SOCKET_INVALID_HANDLE;
    _tcpWindowTimeoutMs = 0; // default disabled
    _httpResult = 0;
    _requestHeader = "";
    _startFlag = true;
    _isRecvWaiting = false;
    _isPersistent = true;
    _contentLen = 0;
    _contentBytesRead = 0;

    _socketConnected = true;
    _socketError = eSocketErrorNone;
    _socketPKResult = pkS_OK;
    _socketHttpResponse = 0;

    _fakeHttpResponseError = false;
    _fakeHttpResponseCode = 0;
    _fakeHttpResponseCount = 0;

    _transferEncodingState = eTransferEncoding_none;
    _chunkSize = -1;

    _socketBuffer.Reset();
    ::Socket_SetExitNetworkFlag(0);//add for network retry function
}

bool CStreamerHttp::Connect( _Inout_ string* pUrl )
{
    string extraHeaders;
    string response;

    return Connect( pUrl, extraHeaders, &response, eRedirectEnable );
}

bool CStreamerHttp::Connect(
    _Inout_ string* pUrl,
    _In_ const string& extraHeader,
    _Out_ string* pResponse,
    _In_ ERedirect eRedirect )
{
    string refererUrl;

    _location = *pUrl;

    for (int i=0; i<gAVEngineConfiguration.HttpMaxRedirectCount; ++i)
    {
        STREAMER_TRACE(("Connect: Attempt=%d url is: %s", i, _location.c_str()));

        CTuneRequest httpTuneRequest;
        if (!httpTuneRequest.ParseUrl(_location))
        {
            _socketError = eSocketErrorHttpOpenFailed;
            return false;
        }

        //Send the referrer if one exists with new request
        string headers = extraHeader;
        if (!refererUrl.empty())
        {
            headers += "Referer: " + refererUrl + "\r\n";
        }

        //Save the referer to be sent with new request
        //
        //Note that we need to save the referrer now because a call
        //to Socket_Connect below could change the _location
        refererUrl = _location;
        if (InternalConnect(httpTuneRequest, headers, pResponse) == false)
        {
            if (i)
            {
                _socketError = eSocketErrorHttpRedirectFailed;
            }

            return false;
        }

        if (_httpResult >= HTTP_STATUS_OK && _httpResult <= HTTP_STATUS_PARTIAL_CONTENT)
        {
            //Update url to new location
            if (i)
            {
                *pUrl = _location;
            }
            return true;
        }

        //Connect should return false if result is < 200 or >= 400,
        //so at this point status code should indicate redirection
        ASSERT(_httpResult >= HTTP_STATUS_AMBIGUOUS && _httpResult <= HTTP_STATUS_REDIRECT_KEEP_VERB);

        //Make sure the init and clean up are done under lock
        {
            AutoLock lock(&_socketLock);
            //Close the socket
            Close();
            //Re-init
            Init();
        }

        //Whether we are requsted to handle HTTP redirects
        if (eRedirectDisable == eRedirect)
        {
            _socketError = eSocketErrorHttpRedirectNotAllowed;

            TRACE_ERROR(("Redirect not allowed"));
            return false;
        }
    }

    //More than maximum redirects. giveup. it could be a loop.
    _socketError = eSocketErrorHttpTooManyRedirect;

    TRACE_ERROR(("Too many http redirects. Result returned: %d", _httpResult));
    return false;
}

bool CStreamerHttp::InternalConnect(
        _In_ CTuneRequest& tuneRequest,
        _In_ const string& extraHeader,
        _Out_ string* pResponse )
{
    //Connect to the HTTP server
    {
        AutoLock lock(&_socketLock);

        //Quit if the socket has already been closed
        //Look in the base class for the logic flow on how this happens
        if (!IStreamer::Connect( &tuneRequest.TunerUrl ))
        {
            return false;
        }

        if ( _pSockAddrInfo )
        {
            //Socket_FreeAddrInfo(_pSockAddrInfo);
            Socket_FreeAddrInfo_DnsCache((char *)_Host.c_str(), (char *)_Port.c_str(), _pSockAddrInfo);
        }

        //Get the server address and port
        if (0 == gAVEngineConfiguration.HttpProxyHost.length())
        {
            if (tuneRequest.Port.empty())
            {
                _pSockAddrInfo = GetHostAddress(tuneRequest.Host.c_str(), c_szHttpConnectDefaultPort);
            }
            else
            {
                _pSockAddrInfo = GetHostAddress(tuneRequest.Host.c_str(), tuneRequest.Port.c_str());
            }
        }
        else // parse proxy host:port and use that for connection
        {
            string proxyHost;
            string proxyPort;

            size_t ixPort = gAVEngineConfiguration.HttpProxyHost.find_first_of(":");
            if (ixPort != string::npos)
            {
                proxyHost = gAVEngineConfiguration.HttpProxyHost.substr(0, ixPort);
                proxyPort = gAVEngineConfiguration.HttpProxyHost.substr(ixPort + 1);
            }
            else // use default port
            {
                proxyHost = gAVEngineConfiguration.HttpProxyHost;
            }

            if (proxyPort.empty())
            {
                _pSockAddrInfo = GetHostAddress(proxyHost.c_str(), c_szHttpProxyDefaultPort);
            }
            else
            {
                _pSockAddrInfo = GetHostAddress(proxyHost.c_str(), proxyPort.c_str());
            }
        }

        // check if host lookup failed
        if ((NULL == _pSockAddrInfo) || (NULL == _pSockAddrInfo->ai_addr))
        {
            _socketError = eSocketErrorHttpOpenFailed;
            return false;
        }

        //And open up a socket
        _socket = ConnectIP();
        if (SOCKET_INVALID_HANDLE == _socket)
        {
            return false;
        }
    }
    //Send a HTTP request and Interpret HTTP result: 200s win
    return HttpRequestResponse( tuneRequest, extraHeader, pResponse );
}

SOCKET_ADDR_INFO* CStreamerHttp::GetHostAddress(_In_ const char* host, _In_ const char* port)
{
    SOCKET_ADDR_INFO* pAddrInfoList = NULL;
    TRACE(("CStreamerHttp::GetHostAddress() - host=%s, port=%s", host, port));

    if (host)
    {
        SOCKET_ADDR_INFO hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = SOCKET_FAMILY_UNSPEC;
        hints.ai_protocol = SOCKET_IPPROTO_TCP;
        hints.ai_socktype = SOCKET_STREAM;
		//TRACE(("CStreamerHttp::GetHostAddress() - ai_family=%d, ai_protocol=%d, ai_socktype=%d", hints.ai_family, hints.ai_protocol, hints.ai_socktype));
        int rc = Socket_GetAddrInfo_DnsCache(
            host, 
            port,
            &hints,
            &pAddrInfoList );

        if (SOCKET_SUCCESS == rc)
        {
            if ((NULL != pAddrInfoList)
                && (NULL == pAddrInfoList->ai_addr))
            {
                pAddrInfoList = NULL;
				
                TRACE_ERROR(("GetHostAddr FAIL: getaddrinfo succeeded but ai_addr is NULL!"));
            }else{
				uint8_t* pAddrData = (uint8_t*)(pAddrInfoList->ai_addr->sa_data);
				_Host = host;
				_Port = port;
				TRACE_ERROR(("GetHostAddr getaddrinfo succeeded addr %d:%d:%d:%d\n",pAddrData[2],pAddrData[3], pAddrData[4], pAddrData[5]));
			}
        }
        else
        {
            pAddrInfoList = NULL;
            TRACE_ERROR(("GetHostAddr FAIL: %d", rc));
        }
    }
    return pAddrInfoList;
}

SOCKET_HANDLE CStreamerHttp::ConnectIP(void)
{
    int sock_result = 0;

    // On connect, clear the the request queue
    _sendHttpRequestQueue.clear();
    STREAMER_TRACE(("ConnectIP clean up request queue"));

    STREAMER_TRACE(("CStreamerHttp::ConnectIP for IPv%d", (SOCKET_FAMILY_INET == _pSockAddrInfo->ai_family) ? 4 : 6));

    //Open up the socket
    SOCKET_HANDLE hSock = Socket_Socket(_pSockAddrInfo->ai_family, _pSockAddrInfo->ai_socktype,  _pSockAddrInfo->ai_protocol);
    if (hSock == SOCKET_INVALID_HANDLE)
    {
        //Save the error state
        _socketError = eSocketErrorHttpOpenFailed;
        _socketPKResult = s_GetLastSockError();

        TRACE_ERROR(("Failed to create socket: x%08x", _socketPKResult));
        return SOCKET_INVALID_HANDLE;
    }

    STREAMER_TRACE(("ConnectIP: Created socket: %d", hSock));

    if (_tcpBufferSize != 0)
    {
        sock_result = Socket_SetRecvBuffer(hSock, _tcpBufferSize);
        if (sock_result == SOCKET_FAILURE)
        {
            _socketError = eSocketErrorHttpOpenFailed;
            _socketPKResult = s_GetLastSockError();

            //Close the socket before leaving
            Socket_CloseSocket(hSock);

            TRACE_ERROR(("Failed with error 0x%x trying to set receive buffer size to %u",
                _socketPKResult, _tcpBufferSize));
            return SOCKET_INVALID_HANDLE;
        }
        else
        {
            STREAMER_TRACE(("ConnectIP: Receive buffer size set to %u", _tcpBufferSize));
        }
    }
	Socket_SetNonBlockingMode(hSock,true);

	struct timeval tvNow;
	gettimeofday(&tvNow, NULL);
	int start_check_time = tvNow.tv_sec*1000 + (uint32_t)tvNow.tv_usec/1000;
	int end_check_time = start_check_time + 1000 * 90;//90 s
	bool is_connected = false;
	STREAMER_TRACE(("start_check_time:%d, end_check_time:%d\n", start_check_time, end_check_time));
	while(start_check_time < end_check_time){
		//Connect to the requested HTTP host
		if (Socket_Connect(hSock, _pSockAddrInfo->ai_addr, _pSockAddrInfo->ai_addrlen) < 0)
		{
			struct SOCKET_TIMEVAL tvSelectTimeout;
			// Set up file descriptor set for Socket_Select
			SOCKET_FD_SET fdWrite;
			SOCKET_FD_ZERO(&fdWrite);
			SOCKET_FD_SET(hSock, &fdWrite);
			int32_t sockResult = SOCKET_FAILURE;
			int timeout = 1;
			while (timeout--){
				if(Socket_GetExitNetworkFlag() == 1){
					break;
				}
				tvSelectTimeout.tv_sec = 0;
				tvSelectTimeout.tv_usec = 1000*1000;//one second
				sockResult = Socket_Select(0, NULL, &fdWrite, NULL, &tvSelectTimeout);
				if (0 == sockResult) // zero ready implies timeout
				{
					//Save the error state
					_socketError = eSocketErrorHttpConnectFailed;
					_socketPKResult = s_GetLastSockError();
					TRACE_ERROR(("Socket_Select timeout after %d seconds",
						tvSelectTimeout.tv_sec));
				}
				else if (SOCKET_FAILURE == sockResult)
				{
					_socketError = eSocketErrorHttpConnectFailed;
					_socketPKResult = s_GetLastSockError();
					TRACE_ERROR(("Socket_Select pkResult %d",_socketPKResult));
				}else{
					is_connected = true;
					break;
				}
			}
			TRACE_ERROR(("Socket_Connect failed count:%d\n", timeout));
		}else{
			TRACE_ERROR(("ConnectIP Succuss\n"));
			is_connected = true;
			break;
		}
		if(is_connected == true){
			TRACE_ERROR(("ConnectIP Succuss111\n"));
			break;
		}
		if(Socket_GetExitNetworkFlag() == 1){
			TRACE_ERROR(("Exit Connect by user\n"));
			break;
		}
		Executive_Sleep(1000);
		gettimeofday(&tvNow, NULL);
		start_check_time = tvNow.tv_sec*1000 + (uint32_t)tvNow.tv_usec/1000;
		STREAMER_TRACE(("start_check_time:%d, end_check_time:%d\n", start_check_time, end_check_time));
	}
	if(is_connected != true){
		//Save the error state
		_socketError = eSocketErrorHttpConnectFailed;
		_socketPKResult = s_GetLastSockError();
		//Close the socket before leaving
		Socket_CloseSocket(hSock);
		TRACE_ERROR(("Failed to Socket_Connect to socket: 0x%x", _socketPKResult));
		return SOCKET_INVALID_HANDLE;
	}
    // Set initial Recv condition flags
    _startFlag = true;
    _isRecvWaiting = false;

    return hSock;
}

//GET .... HTTP/1.1
//Make a request on the connected socket
//Request headers have already been filled in
bool CStreamerHttp::HttpRequestResponse(
    _In_ const CTuneRequest& tuneRequest,
    _In_ const string& extraHeader,
    _Out_ string* pResponse )
{
    if (SendHttpRequest(tuneRequest, extraHeader) == false)
        return false;

    if (RecvHttpResponse( pResponse ) == false)
        return false;

    return true;
}

bool CStreamerHttp::SendHttpRequest( _In_ const CTuneRequest& tuneRequest, _In_ const string& extraHeader )
{
    string tempRequestHeader;

    //If connection is non-persistent, we have to wait until previous response
    //is received before we shutdown the socket and send the next request
    if (!_isPersistent && (!IsResponseComplete()))
    {
        STREAMER_TRACE(("SendHttpRequest: non-persistent connection - waiting for last request (chunked: %d/%d) (%d/%d)",
            _transferEncodingState, 
            _chunkSize, 
            _contentBytesRead, 
            _contentLen));

        return false;
    }

    if (0 != gAVEngineConfiguration.HttpProxyHost.length())
    {
        // create proxy request
        tempRequestHeader = "GET " + tuneRequest.CanonicalUrl;
    }
    else
    {
        tempRequestHeader = "GET /" + tuneRequest.Path;
    }

    if (0 != tuneRequest.Query.length())
    {
        tempRequestHeader += "?" + tuneRequest.Query;
    }

    tempRequestHeader += " HTTP/1.1\r\n";
    tempRequestHeader += "Host: " + tuneRequest.Host + "\r\n";

    if (0 != gAVEngineConfiguration.UserAgentString.length())
    {
        tempRequestHeader += "User-Agent: " + gAVEngineConfiguration.UserAgentString + "\r\n";
    }

    if (0 != extraHeader.length())
    {
        tempRequestHeader += extraHeader;
    }
    tempRequestHeader += "\r\n";

    return SendHttpRequest(tempRequestHeader);
}

bool CStreamerHttp::SendHttpRequest(const string& requestHeader)
{
    bool isRequestSent = true;

    STREAMER_TRACE(("SendHttpRequest: Request headers:\n%s\n------------------------------------------",
        requestHeader.c_str()));

    //Under socket lock:
    {
        AutoLock lock(&_socketLock);

        // SendHttpRequest is not always called under lock, so only update
        // _requestHeader once header has been completely built.
        // For now, API is not called simultaneously by multiple threads,
        // but it might be in future scenarios.
        _requestHeader = requestHeader;

        STREAMER_TRACE(("SendHttpRequest: persist=%d", _isPersistent));
        //If connection is non-persistent, we must first close the socket
        //and reconnect before sending new HTTP request
        if (!_isPersistent)
        {
            //Reset persistent flag to true
            _isPersistent = true;
            isRequestSent = ReconnectIP();
            goto exit;
        }
    }

    //Send HTTP Request
    if (!SendRequest(_requestHeader))
    {
        //Save the error state
        _socketError = eSocketErrorHttpSendRequestFailed;
        _socketPKResult = s_GetLastSockError();

        TRACE_ERROR(("Failed to send request: %s, pkResult: 0x%x",
            requestHeader.c_str(), _socketPKResult));

        //Try to reconnect socket and resend request if there is no pending request before it
        //Otherwise just return false and let the upper layer resend the request
        if (_sendHttpRequestQueue.empty())
        {
            isRequestSent = ReconnectIP();
        }
        else
        {
            _isPersistent = false;
            isRequestSent = false;
        }
    }
exit:
    if ( isRequestSent )
    {
        // Save the request in case it needs to be resent
        _sendHttpRequestQueue.push_back(requestHeader);
        STREAMER_TRACE(("Adding request %s to sendHttpRequestInfo(%d)",
            requestHeader.c_str(), _sendHttpRequestQueue.size()));
    }
    return isRequestSent;
}

bool CStreamerHttp::RecvHttpResponse( _Out_ string* pResponse )
{
    //Get HTTP Response
    if (!ReadResponse( pResponse, 1000 * gAVEngineConfiguration.HttpResponseTimeout))
    {
        //Save the error state
        _socketError = eSocketErrorHttpRecvResponseFailed;

        if(pkS_OK == _socketPKResult)
        {
            _socketPKResult = s_GetLastSockError();
        }

        TRACE_ERROR(("Failed to recv response pkResult: 0x%x", _socketPKResult));
        return false;
    }

    STREAMER_TRACE(("RecvHttpResponse: Responses:\n%s\n------------------------------------------",
        pResponse->c_str()));

    //ParseUrl HTTP response
    if (!ParseResponse( *pResponse, &_httpResult ))
    {
        //Save the error state
        _socketError = eSocketErrorHttpParseResponseFailed;

        TRACE_ERROR(("Failed to parse response: %s", pResponse->c_str()));
        return false;
    }

    //Check for erroneous result (<200, or >=400)
    if ((_httpResult < HTTP_STATUS_OK) || (_httpResult >= HTTP_STATUS_BAD_REQUEST))
    {
        //Save the error state
        _socketError = eSocketErrorHttpInvalidResult;
        _socketHttpResponse = _httpResult;

        TRACE_ERROR(("Invalid result returned: %d", _httpResult));
        return false;
    }
    return true;
}

bool CStreamerHttp::SendRequest( _In_ const string& src)
{
    size_t i = 0;

    const int8_t* pb = reinterpret_cast<const int8_t*>( src.c_str() );

    while( i < src.size() )
    {
        AutoLock lock(&_socketLock);

        if (_socket == SOCKET_INVALID_HANDLE)
        {
            TRACE_ERROR(( "Invalid handle" ));
            return false;
        }

        int bytesSent = ::Socket_Send( _socket, pb + i, src.size() - i, 0 );

        if( bytesSent <= 0 )
        {
            TRACE_ERROR(( "Socket_Send( %d ), returned %d", src.size() - i, bytesSent ));
            return false;
        }

        i += bytesSent;
    }

    return true;
}

bool CStreamerHttp::ReconnectIP()
{
    bool didReconnect = false;
    
    // do Connect again to look up the addr info again
    if (NULL == _pSockAddrInfo)
    {
        return Connect(&_requestHeader);
    }
    
    if (SOCKET_INVALID_HANDLE != _socket)
    {
        Socket_Shutdown(_socket, SOCKET_D_BOTH);
        Socket_CloseSocket(_socket);
    }

    _socket = ConnectIP();
    if (SOCKET_INVALID_HANDLE == _socket)
    {
        // ConnectIP sets error codes
        TRACE_ERROR(("ReconnectIP FAILED TO RECONNECT"));
    }
    else if (!SendRequest(_requestHeader))
    {
        // give up if request fails
        _socketError = eSocketErrorHttpSendRequestFailed;
        _socketPKResult = s_GetLastSockError();
        TRACE_ERROR(("ReconnectIP FAILED SendRequest pkResult 0x%x %s",
            _socketPKResult, _requestHeader.c_str()));
    }
    else
    {
        TRACE(("ReconnectIP SendRequest: %s", _requestHeader.c_str()));
        didReconnect = true;
    }

    return didReconnect;
}

bool CStreamerHttp::ReadResponse( _Out_ string* pResponse, int timeout)
{
    char* responseEndPos = NULL;
    int reconnectCount = 0;

    {
        AutoLock lock(&_socketLock);
        if (!_socketConnected)
        {
            goto exit;
        }

        // reset chunk variables
        _transferEncodingState = eTransferEncoding_none;
        _chunkSize = -1;

        _contentLen = 0;
        _contentBytesRead = 0;

        // check if response header is in the leftovers in the local buffer
        if (_socketBuffer.Count() >= c_strlenHttpResponseHeaderTerminator)
        {
            responseEndPos = _socketBuffer.FindStr(c_szHttpResponseHeaderTerminator);
            if ( responseEndPos )
            {
                goto exit;
            }
        }

        // shift leftover data to the front of the buffer
        _socketBuffer.ShiftData();
        int connectCount = 0;
        connectCount = gAVEngineConfiguration.HttpMaxReconnectAttemptCount;//download media will reconnect many times
        // recv more if there is room
        if (_socketBuffer.Count() < _socketBuffer.Size())
        {
            while ((!responseEndPos || _fakeHttpResponseError) || Socket_GetExitNetworkFlag() == 1)
            {
                responseEndPos = RecvUntilStrInLocalBuffer(c_szHttpResponseHeaderTerminator);
                // socket error occurred, retry
                if ((_socketError != eSocketErrorNone) || _fakeHttpResponseError)
                {
                    _fakeHttpResponseError = false;
                    _socketBuffer.Reset();
                    responseEndPos = NULL;

                    // Give up if too many reconnects or no request header available
                    if (reconnectCount++ >= connectCount|| _requestHeader.length() == 0)
                    {
                        TRACE_ERROR(("ReadResponse failed, reconnect attempts %d", reconnectCount));
                        goto exit;
                    }
                    // Attempt to re-establish the connection and re-issue the request
                    TRACE_ERROR(("ReadResponse Recv failed receiving request response - trying again reconnectCount:%d", reconnectCount));
                    if(Socket_GetExitNetworkFlag() == 1){
                        goto exit;
                    }
                    if (!ReconnectIP())
                    {
                        //goto exit;
                    }
                }
                else
                {
                    break;
                }
            }  
        }
    }

exit:
    if (responseEndPos)
    {
        responseEndPos += c_strlenHttpResponseHeaderTerminator; // jump to end of CR/LF/CR/LF
        int32 headerlen = static_cast<int32>(responseEndPos - (char*)_socketBuffer.StartPtr());

        pResponse->assign((const char*)_socketBuffer.StartPtr(), (uint32)headerlen);
    
        // move the start of the buffer to the end of the header
        _socketBuffer.AdvanceStart(headerlen);

        STREAMER_TRACE(("Response Header found len %d", headerlen));
    }
    else
    {        
        TRACE_ERROR(("Failed, socketConnected(%s) could not find end of response within %d data", 
            _socketConnected ? "TRUE" : "FALSE",
            _socketBuffer.Count() 
            ));

        _socketBuffer.Reset();
    }

    return (responseEndPos != NULL);
}

int32 CStreamerHttp::GetChunkSize()
{
    int32 chunkSize = -1;
    int32 chunkSizeLen = 0;
    
    if (!_socketConnected)
    {
        goto exit;
    }

    // check if the chunksize is currently in the local buffer
    if (_socketBuffer.Count() > 0)
    {
        chunkSize = ParseChunkSize(&chunkSizeLen);

        if (chunkSize > -1)
        {
            goto exit;
        }
    }

    // shift leftover data to the front of the buffer
    _socketBuffer.ShiftData();

    // recv more if there is room
    if (_socketBuffer.Count() < _socketBuffer.Size())
    {
        if (RecvUntilStrInLocalBuffer(c_szHttpResponseElementTerminator))
        {
            chunkSize = ParseChunkSize(&chunkSizeLen);
        }
    }

exit:
    if (chunkSize > -1)
    {
        // advance the index of leftover data
        _socketBuffer.AdvanceStart(chunkSizeLen);

        // end of entire http response
        if (0 == chunkSize)
        {
            ASSERT(eTransferEncoding_Chunked_inProgress == _transferEncodingState);
            _transferEncodingState = eTransferEncoding_Chunked_completed;
        }

        STREAMER_TRACE(("chunkSize 0x%x len %d", chunkSize, chunkSizeLen));
    }
    else
    {
        TRACE_ERROR(("Failed, socketConnected(%s) could not find chunkSize within %d data", 
            _socketConnected ? "TRUE" : "FALSE",
            _socketBuffer.Count() 
            ));

        _socketBuffer.Reset();
    }

    return chunkSize;
}

bool CStreamerHttp::GetChunkDataEnd()
{
    bool endFound = false;
   
    if (!_socketConnected)
    {
        goto exit;
    }

    // check if data end is in the leftovers in the local buffer
    if (_socketBuffer.Count() >= c_strlenHttpResponseElementTerminator)
    {
        endFound = ((char *)_socketBuffer.StartPtr() == _socketBuffer.FindStr(c_szHttpResponseElementTerminator));
        goto exit;
    }

    // shift leftover data to the front of the buffer
    _socketBuffer.ShiftData();

    // recv more if there is room
    if (_socketBuffer.Count() < _socketBuffer.Size())
    {
        // the end must be at the beginning
        endFound = ((char *)_socketBuffer.StartPtr() == RecvUntilStrInLocalBuffer(c_szHttpResponseElementTerminator));
    }

exit:
    if (endFound)
    {
        _socketBuffer.AdvanceStart(c_strlenHttpResponseElementTerminator);
    }
    else
    {
        TRACE_ERROR(("Failed, socketConnected(%s) could not find chunkEnd within %d data", 
            _socketConnected ? "TRUE" : "FALSE",
            _socketBuffer.Count() 
            ));

        _socketBuffer.Reset();
    }

    return endFound;
}

int32 CStreamerHttp::ParseChunkSize(_Out_ int32* pChunkSizeLength)
{
    // Chunk size is in the following format
    // "ChunkSize[; chunk-extension]\r\n", where
    // ChunkSize is in hex ascii
    // chunk-extension is optional

    // get to the end of the chunksize block
    char* chunkSizeBlockEnd = _socketBuffer.FindStr(c_szHttpResponseElementTerminator);
    if (NULL == chunkSizeBlockEnd)
    {
        TRACE_ERROR(("ParseChunkSize: Failed to find CRLF"));
        return -1;
    }

    char* chunkSizeEnd = chunkSizeBlockEnd;

    // check to see if there are optional chunk extensions
    char* chunkExtEnd = _socketBuffer.FindStr(c_szHttpResponseChunkedExtensionTerminator);
    if (chunkExtEnd && chunkExtEnd < chunkSizeEnd)
    {
        chunkSizeEnd = chunkExtEnd;
    }

    // calculate the total length of the chunk size block
    *pChunkSizeLength = (int32)(chunkSizeBlockEnd - (char*)_socketBuffer.StartPtr() + c_strlenHttpResponseElementTerminator);

    //convert hex ascii to int
    char* endPos;
    int32 chunkSize = strtol((char*)_socketBuffer.StartPtr(), &endPos, 16);

    // make sure the conversion stopped at the end
    if (endPos != chunkSizeEnd)
    {
        TRACE_ERROR(("ParseChunkSize: Failed convert chunkSize to int"));
        return -1;
    }
    return chunkSize;
}

bool CStreamerHttp::ParseResponse( _In_ const string& response, _Out_ int* pHttpResult )
{
    //Check result code
    *pHttpResult = -1;
    vector<string> headers;
    string str_result;
    int cHeaders;

    cHeaders = split(response, headers, c_szHttpResponseElementTerminator);

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
    *pHttpResult = atoi(str_result.c_str());

    //Catch non-integer string or code out of range
    if (*pHttpResult < HTTP_STATUS_FIRST || *pHttpResult > HTTP_STATUS_LAST)
    {
        goto exit;
    }

    //Now build the response headers from the server
    _responses.clear();

    // ETag is optional
    _eTag.clear();

    //ParseUrl the rest response headers
    for (int i = 1; i < (int)headers.size(); i++)
    {
        const char* h = headers[i].c_str();
        const char* s = strchr(h, ':');

        if (s != NULL && s < (s + headers[i].length()))
        {
            string name = trim(string(h, s-h));
            string value = trim(string(s+1));

            if (name == "Location")
            {
                _location = value;
            }
            else
            if (name == "Content-Length")
            {
                _transferEncodingState = eTransferEncoding_none;
                _contentLen = atoi(value.c_str());
            }
            else
            if (name == "Transfer-Encoding")
            {
                if( "chunked" == value )
                {
                    _transferEncodingState = eTransferEncoding_Chunked_inProgress;
                    _chunkSize = GetChunkSize();
                    
                    if (_chunkSize < 0)
                    {
                        TRACE_ERROR(("First chunkSize after response header is not found"));
                        return false;
                    }

                    STREAMER_TRACE(("Transfer-Encoding: %s", value.c_str()));
                }
                else
                {
                    TRACE_ERROR(("Transfer-Encoding: %s not supported", value.c_str()));
                    return false;
                }
            }
            else
            if (name == "Connection")
            {
                string v = toLower(value);
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
                    TRACE_ERROR(("Connection:%s not recognized", value.c_str()));
                }
            }
            else
            if (name == "ETag")
            {
                _eTag = value;
            }
            else
            if (name == "TE")
            {
                TRACE_ERROR(("Trailers not supported"));
                return false;
            }
            else
            {
                //Save the rest response headers
                _responses[name] = value;
            }
        }
    }
#ifdef TV2INTERNAL
    if (_fakeHttpResponseCount > 0)
    {
        *pHttpResult = _fakeHttpResponseCode;

        //Set persistence flag to false so that we start a new socket on the
        //   next request else server will try to continue current request
        _isPersistent = false;
        _socketBuffer.Reset();
        _contentLen = 0;
        _transferEncodingState = eTransferEncoding_none;
        _chunkSize = -1;

        _fakeHttpResponseCount--;
    }
#endif
exit:
    if (*pHttpResult < HTTP_STATUS_FIRST || *pHttpResult > HTTP_STATUS_LAST)
    {
        if (cHeaders > 0)
        {
            TRACE_ERROR(("HTTP INVALID response: %s", str_result.c_str()));
        }
        else
        {
            TRACE_ERROR(("HTTP response is EMPTY"));
        }
        return false;
    }

    //If there is erroneous result (<200, or >=400), need to clear out the remaining bytes
    if (((_httpResult < HTTP_STATUS_OK) || (_httpResult >= HTTP_STATUS_BAD_REQUEST))
        && (!IsResponseComplete()))
    {
        TRACE_ERROR(("HTTP response error: %d, clearing remaining response", _httpResult));

        bool endOfResponse = false;
        while (!endOfResponse)
        {
            int rc = Recv(_socketBuffer.BasePtr(), _socketBuffer.Size(), &endOfResponse);
            if (rc <= 0)
            {
                TRACE_ERROR(("Clearing bytes failed rc: %d len: %d",
                    rc, _socketBuffer.Size()));
                break;
            }
        }
    }
    return true;
}

bool CStreamerHttp::GetResponseSessionIdHeader( _Out_ string* pValue )
{
    return false; // session id not used
}

bool CStreamerHttp::GetResponseHeader( _In_ const char* pszHeader, _Out_ string* pValue )
{
    map<string,string>::iterator it = _responses.find( pszHeader );

    if (it != _responses.end())
    {
        *pValue = it->second;
        STREAMER_TRACE(("GetResponseHeader: header:%s, value:%s", pszHeader, pValue->c_str()));
        return true;
    }

    STREAMER_TRACE(("GetResponseHeader: Didn't find header:%s in response", pszHeader ));
    return false;
}

uint32 CStreamerHttp::GetTcpWindowSize(void)
{
    // The value returned by Socket_GetRecvBufferSize may not be exactly the actual tcp window.

    int32_t result;

    int32_t retval = Socket_GetRecvBufferSize(_socket, &result);

    pkASSERT(retval != SOCKET_FAILURE);

    if (retval != SOCKET_FAILURE)
    {
        STREAMER_TRACE(("GetTcpWindowSize() -- %d\n", result));
    }
    else
    {
        result = 0;
        STREAMER_TRACE(("GetTcpWindowSize() -- Socket_GetRecvBufferSize failed!\n"));
    }

    return (uint32)result;
}


bool CStreamerHttp::Close(void)
{
    int sockResult = 0;

    IStreamer::Close(); // clears _socketConnected which Recv observes

    if (_socket != SOCKET_INVALID_HANDLE)
    {
        sockResult = Socket_Shutdown(_socket, SOCKET_D_BOTH);

        int iTimeout = 1000 * gAVEngineConfiguration.HttpInitialReceiveTimeout;

        // wait for a reasonable amount of time for any receive to complete
        while (_isRecvWaiting && iTimeout > 0)
        {
            Executive_Sleep(100); // polling interval chosen for low overhead
            iTimeout -= 100;
        }

        if (_isRecvWaiting)
        {
            TRACE_ERROR(("Close timed out waiting for receive to complete"));
        }

        sockResult = Socket_CloseSocket(_socket);
        STREAMER_TRACE(("Closed socket:%d; result:%d", _socket, sockResult));
        _socket = SOCKET_INVALID_HANDLE;
    }
    return(0 == sockResult);
}

int CStreamerHttp::Recv( _Out_cap_(dstlen) byte* dst, 
                         _In_ int dstlen, 
                         _Out_ bool* pEndOfHttp, 
                         _In_opt_ int timeoutMs )
{
    string requestHeader;

    if(!pEndOfHttp)
    {
        TRACE_ERROR(("pEndOfHttp is NULL"));
        return -1;
    }

    // Receive the data
    int iResult = RecvInternal(dst, dstlen, timeoutMs );

    // If the receive fails while receiving http contents then try to resend the request
    if( (iResult < 0) && (!_sendHttpRequestQueue.empty()) && (!IsResponseComplete()))
    {
        // Get the request
        requestHeader = _sendHttpRequestQueue.front();
        _sendHttpRequestQueue.pop_front();

        TRACE_ERROR(("Recv %d bytes failed - resending HttpRequest(%d) %s, contentBytesRead %d",
            dstlen, _sendHttpRequestQueue.size(), requestHeader.c_str(), _contentBytesRead));

        // Take a snapshot of the queue since resending also gets recorded
        deque<string> pendingRequestQueue = _sendHttpRequestQueue;

        // Record the resend request content length and the number of bytes when the receive failure occurred
        int32 lastRecvContentLen = _contentLen;
        int32 lastRecvBytesRead = _contentBytesRead;
        string lastRecvETag = _eTag;

        // Set persistence flag to false so that we start a new socket on the
        // next send request else server will try to continue current request
        _isPersistent = false;

        // Resend the request
        if ( !SendHttpRequest(requestHeader) )
        {
            TRACE_ERROR(("Resend SendHttpRequest failed %s", requestHeader.c_str()));
            return iResult;
        }

        // Parse the http header response
        string response;
        if ( !RecvHttpResponse( &response ) )
        {
            TRACE_ERROR(("Resend RecvHttpResponse failed %s", requestHeader.c_str()));
            return iResult;
        }

        // Sanity check to make sure this is the same response
        #if 0  //delete becasue send same request will receive the different contenlen
        if( eTransferEncoding_none == _transferEncodingState && _contentLen != lastRecvContentLen )
        {
            TRACE_ERROR(("Resend HttpRequest %s contentLen doesn't match %d(%d)",
                 requestHeader.c_str(), _contentLen, lastRecvContentLen));
            return iResult;
        }
		#endif

        // Check to see if the ETag matches
        if( _eTag != lastRecvETag )
        {
            // eTag can be different when the chunk is moved, even though the contents is the same
            STREAMER_TRACE(("Resend HttpRequest %s ETag doesn't match %s(%s)",
                 requestHeader.c_str(), _eTag.c_str(), lastRecvETag.c_str()));
        }

        //skip the bytes that have already been received
        int skipBytes = (int) lastRecvBytesRead;
        while (skipBytes)
        {
            int len = skipBytes;
            if (len > _socketBuffer.Size())
            {
                len = _socketBuffer.Size();
            }

            int rc = RecvInternal(_socketBuffer.BasePtr(), len);
            if (rc <= 0)
            {
                TRACE_ERROR(("Skipping bytes during resend HttpRequest %s failed rc: %d len: %d skipBytes: %d lastRecvBytesRead: %d",
                    requestHeader.c_str(), rc, len, skipBytes, lastRecvBytesRead));
                return iResult;
            }
            skipBytes -= rc;
        }

        // Try to redo the original recv request
        iResult = RecvInternal(dst, dstlen, timeoutMs );
        if( iResult < 0 )
        {
            TRACE_ERROR(("Recv %d failed - even though resend succeeded HttpRequest(%d) %s, contentBytesRead %d",
                dstlen, _sendHttpRequestQueue.size(), requestHeader.c_str(), _contentBytesRead));
            return iResult;
        }

        // Resend other pending request as well
        while( !pendingRequestQueue.empty() )
        {
            STREAMER_TRACE(("Resend pending HttpRequest %s", pendingRequestQueue.front().c_str()));
            if(!SendHttpRequest(pendingRequestQueue.front()))
            {
                TRACE_ERROR(("Resend pending HttpRequest failed %s", pendingRequestQueue.front().c_str()));
                return -1;
            }
            pendingRequestQueue.pop_front();
        }
    }

    //check to see if the entire response has been received
    if( IsResponseComplete() )
    {
        *pEndOfHttp = true;
        if( !_sendHttpRequestQueue.empty() )
        {
            STREAMER_TRACE(("Full Response received %d, deleting request %s from sendHttpRequestInfo(%d)",
                _contentBytesRead, _sendHttpRequestQueue.front().c_str(), _sendHttpRequestQueue.size()));

            _sendHttpRequestQueue.pop_front();
        }
    }
    return iResult;
}

int CStreamerHttp::RecvInternal(_Out_cap_(dstlen) byte* dst, _In_ int dstlen, _In_opt_ int timeoutMs )
{
    int bytesReceived = -1; // default to error return
	//bool isAlreadyRetry = false;
	int retry_cnt = 3;
    // Send error when closed
    if (!_socketConnected)
    {
        TRACE_ERROR(("Recv when not connected"));
        return bytesReceived;
    }
retry:
    // Start with any data remaining in the local buffer
    bytesReceived = CopyFromLocalBuffer(dst, dstlen);
    if (0 == bytesReceived)
    {
        bytesReceived = RecvFromSocket(dst, dstlen, timeoutMs);
    }

    if (bytesReceived > 0)
    {
        if(eTransferEncoding_Chunked_inProgress == _transferEncodingState)
        {
            _chunkSize -= bytesReceived;
            ASSERT(_chunkSize >= 0);
            if (0 == _chunkSize)
            {
                if(!GetChunkDataEnd())
                {
                    TRACE_ERROR(("Failed to find chunked data end"));
                    return -1;
                }
                _chunkSize = GetChunkSize();

                if (_chunkSize < 0)
                {
                    TRACE_ERROR(("Failed to find next chunkSize"));
                    return -1;
                }
            }
        }
        _contentBytesRead += bytesReceived;
    }else{
		/*receive media data failed*/
		TRACE_ERROR(("receive media data failed"));
		struct timeval tvNow;
		gettimeofday(&tvNow, NULL);
		int start_check_time = tvNow.tv_sec*1000 + (uint32_t)tvNow.tv_usec/1000;
		int end_check_time = end_check_time = start_check_time + 1000 * 90;//90 s
		bool is_connected = false;
		STREAMER_TRACE(("start_check_time:%d, end_check_time:%d\n", start_check_time, end_check_time));
		while(start_check_time < end_check_time){
			//Connect to the requested HTTP host
			if (Socket_Connect(_socket, _pSockAddrInfo->ai_addr, _pSockAddrInfo->ai_addrlen) < 0)
			{
				struct SOCKET_TIMEVAL tvSelectTimeout;
				// Set up file descriptor set for Socket_Select
				SOCKET_FD_SET fdWrite;
				SOCKET_FD_ZERO(&fdWrite);
				SOCKET_FD_SET(_socket, &fdWrite);
				int32_t sockResult = SOCKET_FAILURE;
				int timeout = 10;
				while (timeout--){
					if(Socket_GetExitNetworkFlag() == 1){
						break;
					}
					tvSelectTimeout.tv_sec = 0;
					tvSelectTimeout.tv_usec = 1000*1000;//one second
					sockResult = Socket_Select(0, NULL, &fdWrite, NULL, &tvSelectTimeout);
					if (0 == sockResult) // zero ready implies timeout
					{
						//Save the error state
						_socketError = eSocketErrorHttpConnectFailed;
						_socketPKResult = s_GetLastSockError();
						TRACE_ERROR(("Socket_Select timeout after %d seconds",
							tvSelectTimeout.tv_sec));
					}
					else if (SOCKET_FAILURE == sockResult)
					{
						_socketError = eSocketErrorHttpConnectFailed;
						_socketPKResult = s_GetLastSockError();
						TRACE_ERROR(("Socket_Select pkResult %d",_socketPKResult));
					}else{
						is_connected = true;
						break;
					}
				}
				TRACE_ERROR(("Socket_Connect failed count:%d\n", timeout));
			}else{
				TRACE_ERROR(("ConnectIP Succuss\n"));
				is_connected = true;
				break;
			}
			if(is_connected == true){
				TRACE_ERROR(("ConnectIP Succuss111\n"));
				break;
			}
			if(Socket_GetExitNetworkFlag() == 1){
				TRACE_ERROR(("Exit Connect by user\n"));
				break;
			}
			Executive_Sleep(1000);
			gettimeofday(&tvNow, NULL);
			start_check_time = tvNow.tv_sec*1000 + (uint32_t)tvNow.tv_usec/1000;
			STREAMER_TRACE(("start_check_time:%d, end_check_time:%d\n", start_check_time, end_check_time));
		}
		TRACE_ERROR(("receive media data111 is_connected:%d, retry_cnt:%d", is_connected, retry_cnt));
		if(is_connected != true || /*isAlreadyRetry == true*/retry_cnt == 0){
			return bytesReceived;
		}
		retry_cnt--;
		//isAlreadyRetry = true;
        goto retry;
	}
    return bytesReceived;
}

int CStreamerHttp::RecvFromSocket(_Out_cap_(dstlen) byte* dst, _In_ int dstlen, _In_ int timeoutMs )
{
    int bytesReceived = -1; // default to error return
    int32_t sockResult = SOCKET_FAILURE;

    if (!_socketConnected)
    {
        TRACE_ERROR(("Recv when not connected"));
        return bytesReceived;
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
            tvSelectTimeout.tv_sec = gAVEngineConfiguration.HttpInitialReceiveTimeout;
        }
        else
        {
            tvSelectTimeout.tv_sec = gAVEngineConfiguration.HttpSubsequentReceiveTimeout;
        }
        tvSelectTimeout.tv_usec = 0;
    }

	//tvSelectTimeout.tv_sec = timeoutMs / 1000;
	//tvSelectTimeout.tv_usec = (timeoutMs % 1000) * 1000;
    _startFlag = false; // in all cases, now no longer the first Recv

    // Set up file descriptor set for Socket_Select
    SOCKET_FD_SET fdRead;

    SOCKET_FD_ZERO(&fdRead);

    SOCKET_FD_SET(_socket, &fdRead);

    _isRecvWaiting = true;

    // Note: A check of _socketConnected must come after _isRecvWaiting is set
    //       to catch the case where Close from another thread has closed the socket.
    //       These two flags form an interlock this way.

    if (_socketConnected)
    {
		int timeout = 3;
		while (timeout--){
			if(Socket_GetExitNetworkFlag() == 1){
				break;
			}
			tvSelectTimeout.tv_sec = 0;
			tvSelectTimeout.tv_usec = 1000*1000;//one second
			sockResult = Socket_Select(0, &fdRead, NULL, NULL, &tvSelectTimeout);
			if (0 == sockResult) // zero ready implies timeout
			{
				_socketError = eSocketErrorReadError;
				// socket does not return an error code so explicitly set the reason
				_socketPKResult = pkE_TIMEOUT;
				TRACE_ERROR(("Socket_Select timeout after %d seconds",
					tvSelectTimeout.tv_sec));
			}
			else if (SOCKET_FAILURE == sockResult)
			{
				_socketError = eSocketErrorReadError;
				_socketPKResult = s_GetLastSockError();
				TRACE_ERROR(("Socket_Select pkResult %d",
					_socketPKResult));
			}
			else if (_socketConnected)
			{
				int recvLen = dstlen;
				if (eTransferEncoding_Chunked_inProgress == _transferEncodingState)
				{
					// limit amount of data for chunked data to the chunkSize
					if ( _chunkSize > 0 && recvLen > _chunkSize )
					{
						recvLen = _chunkSize;
					}
				}
				else
				{
					// limit amount of data to what is left in the response
					if ( _contentLen > 0 && recvLen > _contentLen - _contentBytesRead )
					{
						recvLen = _contentLen - _contentBytesRead;
					}
				}
				//Read data that is now ready
				sockResult = Socket_Recv(_socket, (int8_t*)dst, (int32_t)recvLen, 0);
				if (SOCKET_FAILURE == sockResult)
				{
					//Note: a "would block" is considered an error here since Socket_Select indicated receive ready
					//Save the error state
					_socketError = eSocketErrorReadError;
					_socketPKResult = s_GetLastSockError();
					TRACE_ERROR(("Socket_Recv pkResult %d receiving %d bytes",
						_socketPKResult, dstlen));
				}
				else // sockResult is number of bytes received
				{
					_socketError = eSocketErrorNone;
					bytesReceived = (int)sockResult;
					STREAMER_TRACE(("RecvFromSocket: chunked(%d/%d) (%d/%d), %d in dstlen, %d received",
							_transferEncodingState,
							_chunkSize,
							_contentBytesRead,
							_contentLen,
							dstlen,
							bytesReceived));
					break;
				}
			}
        }
    }
    _isRecvWaiting = false;
    return bytesReceived;
}

int CStreamerHttp::CopyFromLocalBuffer(_Out_cap_(dstlen) byte* dst, _In_ int dstlen)
{
    int bytesCopied = 0;

    // check if there are any leftovers
    if (_socketBuffer.Count() > 0)
    {
        bytesCopied = (_socketBuffer.Count() >= dstlen) ? dstlen : _socketBuffer.Count();
                    
        // limit amount of data for chunked data to the chunkSize
        if (eTransferEncoding_Chunked_inProgress == _transferEncodingState && _chunkSize > 0 && bytesCopied > _chunkSize)
        {
            bytesCopied = _chunkSize;
        }

        STREAMER_TRACE(("CopyFromLocalBuffer: %d are in local buffer chunked(%d/%d), %d in dstlen, %d copied",
             _socketBuffer.Count(), _transferEncodingState, _chunkSize, dstlen, bytesCopied));

        memcpy_s(dst, dstlen, _socketBuffer.StartPtr(), bytesCopied);
        _socketBuffer.AdvanceStart(bytesCopied);
    }

    return bytesCopied;
}

char* CStreamerHttp::RecvUntilStrInLocalBuffer(_In_ const char* str)
{
    int recvLen = 0;
    char* strPos = NULL;
    while (_socketBuffer.Space() > 0)
    {
        //Try read some data
        recvLen = RecvFromSocket(_socketBuffer.EndPtr(), _socketBuffer.Space(), 1000 * gAVEngineConfiguration.HttpResponseTimeout);
        if (recvLen <= 0)
        {
            TRACE_ERROR(("RecvUntilStr Recv failed result %d", recvLen));
            return NULL;
        }

        //Update the buffer on the bytes received
        _socketBuffer.IncreaseCount(recvLen);

        //Now try to find the request str
        strPos = _socketBuffer.FindStr(str);
        if (NULL != strPos)
        {
            //Found the requested str, break out of the Recv loop.
            break;
        }

        STREAMER_TRACE(("No str %s after %d read; continuing to read more...", str, recvLen));
    }
    return strPos;
}

bool CStreamerHttp::IsResponseComplete()
{
    return (eTransferEncoding_none == _transferEncodingState) 
                ? (_contentBytesRead == _contentLen) 
                : (eTransferEncoding_Chunked_completed == _transferEncodingState);
}

bool CStreamerHttp::Command(const string& command, const vector<string>& args)
{
#ifdef TV2INTERNAL
    //Force a fake http response error
    if (command == "fakehttpresponseerror")
    {
        _fakeHttpResponseError = true;

        TRACE(("Fake http response error is set"));
        return true;
    }

    //Force a fake http response code
    if (command == "fakehttpresponsecode")
    {
        //Number of arguments
        int numargs = args.size();

        _fakeHttpResponseCode = (numargs >= 1) ? atoi(args[0].c_str()) : HTTP_STATUS_NOT_FOUND;
        _fakeHttpResponseCount = (numargs >= 2) ? atoi(args[1].c_str()) : 1;

        TRACE(("Fake http response code %d count %d is set", _fakeHttpResponseCode, _fakeHttpResponseCount));
        return true;
    }
#endif

    return false;
}
// ===============================================================================================================
// ===============================================================================================================
