///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <SSPKDefines.h>
#include <PKTestSuite.h>
#include <pkExecutive.h>
#include <pkSockets.h>
#include <StringUtils.h>

////////////////////////////////////////////////////////////////////////////////
PKTEST_GROUP( SocketTests )
{
    ////////////////////////////////////
    PKTEST_METHOD( Socket_Socket_InvalidParameters )
    {
        struct
        {
            int32_t af;
            int32_t type;
            int32_t protocol;
            SOCKET_HANDLE retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { ~SOCKET_FAMILY_INET,SOCKET_STREAM,  SOCKET_IPPROTO_TCP,  SOCKET_INVALID_HANDLE, pkE_INVALIDARG },
            { SOCKET_FAMILY_INET, ~SOCKET_STREAM, SOCKET_IPPROTO_TCP,  SOCKET_INVALID_HANDLE, pkE_INVALIDARG },
            { SOCKET_FAMILY_INET, SOCKET_STREAM,  ~SOCKET_IPPROTO_TCP, SOCKET_INVALID_HANDLE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            SOCKET_HANDLE retVal = Socket_Socket( rgEntries[i].af, rgEntries[i].type, rgEntries[i].protocol );
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( Socket_Shutdown_InvalidParameters )
    {
        struct
        {
            SOCKET_HANDLE handle;
            int32_t how;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, SOCKET_D_RECEIVE,   SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE)1,      ~SOCKET_D_RECEIVE,  SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_Shutdown( rgEntries[i].handle, rgEntries[i].how);
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( Socket_Connect_InvalidParameters )
    {
        static SOCKET_SOCKADDR sockAddr;
        struct
        {
            SOCKET_HANDLE handle;
            struct SOCKET_SOCKADDR *name;
            int32_t namelen;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, &sockAddr, sizeof(sockAddr), SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     NULL,      sizeof(sockAddr), SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_Connect( rgEntries[i].handle, rgEntries[i].name, rgEntries[i].namelen);
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( Socket_Recv_InvalidParameters )
    {
        static int8_t buf[8];
        struct
        {
            SOCKET_HANDLE handle;
            int8_t* buf;
            int32_t len;
            int32_t flags;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, buf,  sizeof(buf), 1, SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     NULL, sizeof(buf), 1, SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_Recv( rgEntries[i].handle, rgEntries[i].buf, rgEntries[i].len, rgEntries[i].flags);
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( Socket_RecvFrom_InvalidParameters )
    {
        static int8_t buf[8];
        static SOCKET_SOCKADDR from;
        static int32_t fromlen = sizeof(from);

        struct
        {
            SOCKET_HANDLE handle;
            int8_t* buf;
            int32_t len;
            int32_t flags;
            struct SOCKET_SOCKADDR *from;
            int32_t* fromlen;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, buf,  sizeof(buf), 1, &from, &fromlen, SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     NULL, sizeof(buf), 1, &from, &fromlen, SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     buf,  sizeof(buf), 1, NULL,  &fromlen, SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     buf,  sizeof(buf), 1, &from, NULL,     SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_RecvFrom( rgEntries[i].handle, rgEntries[i].buf, 
                rgEntries[i].len, rgEntries[i].flags, rgEntries[i].from, rgEntries[i].fromlen);
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( Socket_Send_InvalidParameters )
    {
        static int8_t buf[8];

        struct
        {
            SOCKET_HANDLE handle;
            int8_t* buf;
            int32_t len;
            int32_t flags;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, buf,  sizeof(buf), 1, SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     NULL, sizeof(buf), 1, SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_Send( rgEntries[i].handle, rgEntries[i].buf, 
                rgEntries[i].len, rgEntries[i].flags);
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( Socket_SendTo_InvalidParameters )
    {
        static int8_t buf[8];
        static SOCKET_SOCKADDR to;

        struct
        {
            SOCKET_HANDLE handle;
            int8_t* buf;
            int32_t len;
            int32_t flags;
            SOCKET_SOCKADDR *to;
            int32_t token;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, buf,  sizeof(buf), 1, &to, 1, SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     NULL, sizeof(buf), 1, &to, 1, SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_SendTo( rgEntries[i].handle, rgEntries[i].buf, 
                rgEntries[i].len, rgEntries[i].flags, rgEntries[i].to, rgEntries[i].token);
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( Socket_Bind_InvalidParameters )
    {
        static int8_t buf[8];
        static SOCKET_SOCKADDR addr;

        struct
        {
            SOCKET_HANDLE handle;
            SOCKET_SOCKADDR *addr;
            int32_t addrlen;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, &addr,  sizeof(addr), SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     NULL,   0,            SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_Bind( rgEntries[i].handle, rgEntries[i].addr, rgEntries[i].addrlen);
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( Socket_Listen_InvalidParameters )
    {
        struct
        {
            SOCKET_HANDLE handle;
            int32_t backlog;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, 1, SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_Listen( rgEntries[i].handle, rgEntries[i].backlog);
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( Socket_Accept_InvalidParameters )
    {
        static SOCKET_SOCKADDR addr;
        static int32_t addrlen = sizeof(addr);
        struct
        {
            SOCKET_HANDLE handle;
            struct SOCKET_SOCKADDR *addr;
            int32_t *addrlen;
            SOCKET_HANDLE retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, &addr,  &addrlen,  SOCKET_INVALID_HANDLE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     NULL,   &addrlen,  SOCKET_INVALID_HANDLE, pkE_INVALIDARG },            
            { (SOCKET_HANDLE) 1,     &addr,  NULL,      SOCKET_INVALID_HANDLE, pkE_INVALIDARG },        
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            SOCKET_HANDLE retVal = Socket_Accept( rgEntries[i].handle, rgEntries[i].addr, rgEntries[i].addrlen );
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( Socket_FDIsSet_InvalidParameters )
    {
        static SOCKET_FD_SET set;
        struct
        {
            SOCKET_HANDLE handle;
            SOCKET_FD_SET* set;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, &set, SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     NULL, SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_FDIsSet( rgEntries[i].handle, rgEntries[i].set);
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( Socket_GetBytesAvailable_InvalidParameters )
    {
        struct
        {
            SOCKET_HANDLE handle;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_GetBytesAvailable( rgEntries[i].handle );
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( Socket_SetSocketOpt_InvalidParameters )
    {
        const static int8_t optval = 1;
        struct
        {
            SOCKET_HANDLE handle;
            int32_t level;
            int32_t optname;
            const int8_t* optval;
            int32_t optlen;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, SOCKET_L_SOCKET,  SOCKET_REUSEADDR,  &optval, sizeof(optval), SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     ~SOCKET_L_SOCKET, SOCKET_REUSEADDR,  &optval, sizeof(optval), SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     SOCKET_L_SOCKET,  ~SOCKET_REUSEADDR, &optval, sizeof(optval), SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_SetSocketOpt( rgEntries[i].handle, rgEntries[i].level, rgEntries[i].optname, rgEntries[i].optval, rgEntries[i].optlen );
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( Socket_GetSocketName_GetSocketName_InvalidParameters )
    {
        static SOCKET_SOCKADDR name;
        static int32 namelen = sizeof(name);
        struct
        {
            SOCKET_HANDLE handle;
            struct SOCKET_SOCKADDR *name;
            int32_t *namelen;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, &name, &namelen, SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     NULL,  &namelen, SOCKET_FAILURE, pkE_INVALIDARG },
            { (SOCKET_HANDLE) 1,     &name, NULL,     SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_GetSocketName( rgEntries[i].handle, rgEntries[i].name, rgEntries[i].namelen);
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());

            retVal = Socket_GetPeerName( rgEntries[i].handle, rgEntries[i].name, rgEntries[i].namelen);
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////        
    PKTEST_METHOD( Socket_GetNameInfo_InvalidParameters )
    {
        static SOCKET_SOCKADDR sa;
        static int8_t host = 1;
        static int8_t serv = 1;
        struct
        {
            const SOCKET_SOCKADDR *sa;
            int32_t salen;
            int8_t *host;
            uint32_t hostlen;
            int8_t *serv;
            uint32_t servlen;
            int32_t flags;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { &sa, sizeof(sa),  &host,  sizeof(host), &serv, sizeof(serv), ~SOCKET_NI_NOFQDN, SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_GetNameInfo( rgEntries[i].sa, rgEntries[i].salen, 
                rgEntries[i].host, rgEntries[i].hostlen, rgEntries[i].serv, rgEntries[i].servlen, rgEntries[i].flags );
            
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
    ////////////////////////////////////
    PKTEST_METHOD( Socket_SetNonBlockingMode_InvalidParameters )
    {
        struct
        {
            SOCKET_HANDLE handle;
            bool state;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, true, SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_SetNonBlockingMode( rgEntries[i].handle, rgEntries[i].state );
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( Socket_SetRecvBuffer_SetSendBuffer_InvalidParameters )
    {
        struct
        {
            SOCKET_HANDLE handle;
            int32_t val;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, 10, SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_SetRecvBuffer( rgEntries[i].handle, rgEntries[i].val );
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());

            retVal = Socket_SetSendBuffer( rgEntries[i].handle, rgEntries[i].val );
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }

    ////////////////////////////////////
    PKTEST_METHOD( Socket_GetRecvBufferSize_CloseSocket_InvalidParameters )
    {
        int32_t result;
        struct
        {
            SOCKET_HANDLE handle;
            int32_t *pSizeResult;
            int32_t retVal;
            pkRESULT pkResult;
        }
        static const rgEntries[] =
        {
            { SOCKET_INVALID_HANDLE, &result, SOCKET_FAILURE, pkE_INVALIDARG },
        };

        for( size_t i = 0; i < sizeof(rgEntries)/sizeof(rgEntries[0]); ++i )
        {
            int32_t retVal = Socket_GetRecvBufferSize( rgEntries[i].handle, rgEntries[i].pSizeResult );
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "GetRecvBufferSize Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "GetRecvBufferSize Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());

            retVal = Socket_CloseSocket( rgEntries[i].handle );
            PKTEST_ASSERT_MSG_EXIT(  retVal == rgEntries[i].retVal, 
                "CloseSocket Expect retVal 0x%x but found 0x%x", rgEntries[i].retVal, retVal );
            PKTEST_ASSERT_MSG_EXIT( Socket_GetResultFromLastErr() == rgEntries[i].pkResult,
                "CloseSocket Expect pkResult 0x%x but found 0x%x", rgEntries[i].pkResult, Socket_GetResultFromLastErr());
        }

    exit:
        return;
    }
};
