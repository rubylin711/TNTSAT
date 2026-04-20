///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include <pkPAL.h>
#include <pkExecutive.h>
#include <pkSockets.h>
#include <pkTestFramework.h>
#include <TLCommon.h>

#define PORTNUM         12345         // Port number
#define szPORTNUM "12345"

#define WAIT_LOOP_TIME  200           // milliseconds

static bool_t           fPleaseQuit = FALSE;

static void s_PrintUsage(bool_t fVerbose)
{
    const char *pszProgName = "Socket";

    TF_Printf(
        "===========================================================\n"
        "%s Usage:\n%s",
        pszProgName,
        "   Socket Unit test Client parameters:  -d <destination IP address>"
        "\n"
        "   Socket Unit test Server parameters:  -s (no extra parameters for server mode.) \n"
        );
}

//===========================================================================
// Public Functions
//===========================================================================

pkRESULT Test_GetConfig(int argc, char *argv[], TF_Config *pConfig)
{
    // parse through options...
    bool_t fVerbose;
    pkRESULT retval = pkS_OK;

    TLC_SetCmdLineArgs(argc, argv);

    if (TLC_IsHelpRequested(&fVerbose))
    {
        s_PrintUsage(fVerbose);
        retval = pkS_FALSE;
        goto bail;
    }

    // set the configuration
    if (pConfig->size < sizeof(TF_Config))
    {
        retval = pkE_INVALIDARG;
        goto bail;
    }

bail:
    return retval;
}

static void Client();
static void Server();
static void Test_GetAddrInfo();

pkRESULT Test_Run(void)
{
    pkRESULT hr = S_OK;

    int sockerr = Socket_Startup();

    if(sockerr)
    {
        TF_Printf("socket startup problem: %d\n", sockerr);
        goto bailout;
    }

    if( pkS_OK == TLC_GetCmdLineArg("-d", 0, NULL ) )
    {
        TF_Printf("running client\n");
        Client();
    }
    else if ( pkS_OK == TLC_GetCmdLineArg("-s", 0, NULL ) )
    {
        TF_Printf("running server\n");
        Server();
    }
    else if ( pkS_OK == TLC_GetCmdLineArg("-a", 0, NULL ) )
    {
        TF_Printf("running getAddrInfo\n");
        Test_GetAddrInfo();
    }
    else
    {
        TF_Printf("cmd line parsing problem\n");
        goto bailout;
    }
bailout:
    Socket_Cleanup();

    return hr;
}


pkRESULT Test_Abandon(void)
{
    fPleaseQuit = TRUE;
    return pkS_OK;
}

//===========================================================================
// Private Functions
//===========================================================================

void Client()
{
    SOCKET_HANDLE hSock = NULL;
    SOCKET_SOCKADDR_IN sockAddr = {0};
    pkRESULT hr = pkS_OK;
    char *szDestAddr = NULL;
    BYTE sendBuffer[2000];
    int32_t numBytes;

    hSock = Socket_Socket(SOCKET_FAMILY_INET, SOCKET_STREAM, SOCKET_IPPROTO_TCP);
    if( NULL == hSock )
    {
        TF_Printf("socket problem\n");
        goto bailout;
    }

    hr = TLC_GetCmdLineArg("-d", 1, &szDestAddr);
    if(pkFAILED(hr) || NULL == szDestAddr)
    {
        TF_Printf("failed getting dest addr\n");
        goto bailout;
    }

    sockAddr.sin_addr.s_addr = Socket_INetAddr((int8_t *)szDestAddr);
    sockAddr.sin_family = SOCKET_FAMILY_INET;
    sockAddr.sin_port = Socket_htons(PORTNUM);

    if(Socket_Connect(hSock, (SOCKET_SOCKADDR*)&sockAddr, sizeof(sockAddr)))
    {
        TF_Printf("connect failed\n");
        goto bailout;
    }

    TF_Printf("connect succeeded\n");

    TF_Printf("waiting 1 sec for FIN\n");
    Executive_Sleep(1000);

    TF_Printf("doing last 5 byte send\n");

    numBytes = Socket_Send( hSock, (int8_t *) sendBuffer, 5, 0);
    if( numBytes != 5 )
    {
        TF_Printf("send problem\n");
        goto bailout;
    }

    Executive_Sleep(100);
    TF_Printf("trying to do bad send now\n");

    numBytes = Socket_Send( hSock, (int8_t *) sendBuffer, 16, 0);
    TF_Printf("returned from send\n");
    if( numBytes != 16 )
    {
        TF_Printf("send problem\n");
        goto bailout;
    }

    TF_Printf("client done\n");

bailout:
    if(hSock)
        Socket_CloseSocket(hSock);
    return;
} // Client

void Server()
{
    SOCKET_HANDLE lSock = NULL;
    SOCKET_HANDLE aSock = NULL;
    SOCKET_SOCKADDR_IN sockAddr = {0};

    lSock = Socket_Socket(SOCKET_FAMILY_INET, SOCKET_STREAM, SOCKET_IPPROTO_TCP);
    if( NULL == lSock )
    {
        TF_Printf("socket problem\n");
        goto bailout;
    }

    sockAddr.sin_addr.s_addr = SOCKET_INADDR_ANY;
    sockAddr.sin_family = SOCKET_FAMILY_INET;
    sockAddr.sin_port = Socket_htons(PORTNUM);

    if(Socket_Bind(lSock, (SOCKET_SOCKADDR*)&sockAddr, sizeof(sockAddr)))
    {
        TF_Printf("bind problem\n");
        goto bailout;
    }

    if(Socket_Listen(lSock, 1))
    {
        TF_Printf("listen problem\n");
        goto bailout;
    }

    if (FAILED(Socket_SetNonBlockingMode(lSock, TRUE)))
    {
        TF_Printf("set sock opt problem\n");
        goto bailout;
    }

    do
    {
        aSock = Socket_Accept(lSock, NULL, NULL);
        if ((aSock == NULL) || (aSock == SOCKET_INVALID_HANDLE))
        {
            if (Socket_GetResultFromLastErr() != pkE_SOCKET_WOULDBLOCK)
            {
                break;
            }

            Executive_Sleep(WAIT_LOOP_TIME);
        }

    } while (fPleaseQuit==FALSE);

    if( NULL == aSock )
    {
        TF_Printf("accept problem\n");
        goto bailout;
    }

    TF_Printf("got connection\n");

    //send a fin
    TF_Printf("closing socket (sends FIN)\n");
    Socket_CloseSocket(aSock);
    aSock = NULL;

    TF_Printf("server done\n");
    Executive_Sleep(2000);

bailout:
    if(lSock)
        Socket_CloseSocket(lSock);
    if(aSock)
        Socket_CloseSocket(aSock);
    return;
} // Server

void Test_GetAddrInfo()
{
    SOCKET_ADDR_INFO *result = NULL;
    SOCKET_ADDR_INFO *ptr = NULL;
    char *hostname = NULL;
    int32_t retval;
    pkRESULT hr = pkS_OK;
    SOCKET_SOCKADDR_IN* addr = NULL;
    int8_t hostip[20];
    int8_t server[20];
    char portNum[20];
    hr = TLC_GetCmdLineArg("-a", 1, &hostname);
    if(pkFAILED(hr) || NULL == hostname)
    {
        TF_Printf("failed getting hostname\n");
        goto bailout;
    }

    retval = Socket_GetAddrInfo(hostname, szPORTNUM, NULL, &result);
    if ( retval != 0 )
    {
        TF_Printf("Socket_GetAddrInfo failed with error: 0x%x\n", retval);
        goto bailout;
    }
    else
    {
        TF_Printf("Socket_GetAddrInfo succeeded\n", retval);
    }

    //verify Socket_GetAddrInfo call with Socket_GetNameInfo
    for(ptr=result; ptr != NULL; ptr=ptr->ai_next)
    {
        addr = (SOCKET_SOCKADDR_IN *)(ptr->ai_addr);

        TF_Printf("GetAddrInfo IP Addr: %s , port: %d\n",
                   Socket_INet_ntoa(addr->sin_addr), addr->sin_port);

        if(SOCKET_SUCCESS == (Socket_GetNameInfo(ptr->ai_addr,

                              ptr->ai_addrlen,
                              hostip,
                              sizeof(hostip),
                              server,
                              sizeof(server),
                              SOCKET_NI_NUMERICHOST |
                              SOCKET_NI_NUMERICSERV)))
        {
            TF_Printf("GetNameInfo IP Addr: %s, port: %s\n",hostip,server);
        }
        else
        {
            TF_Printf("Socket_GetNameInfo failed with error: 0x%x\n", Socket_GetResultFromLastErr());
        }
    }

    if(result)
    {
        Socket_FreeAddrInfo(result);
    }

bailout:
    return;
}

