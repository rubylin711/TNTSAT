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

#include <CSocketMbrManifest.h>
#include <CSocketMbrRetry.h>

////////////////////////////////////////////////////////////////////////////////
PKTEST_GROUP( SocketMbrTests )
{
    ////////////////////////////////////
    PKTEST_METHOD( SparseStreamChunkInfoPragmaParser )
    {
        enum Command
        {
            cmdInitParser,
            cmdCheckMoveNext,
            cmdCheckMoveEnd,
        };

        struct
        {
            Command cmd;
            const char* pszVal;
            int64_t value;
        }
        static const rgCmds[] =
        {
            // Content-Type header with single sparse chunk entry
            { cmdInitParser,        "video/mp4;ChildTrack=\"captions=4234400000000;\"", 0 },
            { cmdCheckMoveNext,     "captions", 4234400000000LL },
            { cmdCheckMoveEnd,      NULL, 0 },

            // Content-Type with 2 sparse chunk entries
            { cmdInitParser,        "video/mp4;ChildTrack=\"foo=1234;bar=3445;\"", 0 },
            { cmdCheckMoveNext,     "foo", 1234 },
            { cmdCheckMoveNext,     "bar", 3445 },
            { cmdCheckMoveEnd,      NULL, 0 },

            // Content-Type with 2 sparse chunk entries
            { cmdInitParser,        "video/mp4;ChildTrack=\"foo=222333;bar=444555;\"", 0 },
            { cmdCheckMoveNext,     "foo", 222333 },
            { cmdCheckMoveNext,     "bar", 444555 },
            { cmdCheckMoveEnd,      NULL, 0 },

            // Content-Type with 2 sparse chunk entries and extra field
            { cmdInitParser,        "video/mp4; charset=ISO-8859-4; ChildTrack=\"foo=1234;bar=3445;\"", 0 },
            { cmdCheckMoveNext,     "foo", 1234 },
            { cmdCheckMoveNext,     "bar", 3445 },
            { cmdCheckMoveEnd,      NULL, 0 },

            // Content-Type with 0 sparse chunk entries
            { cmdInitParser,        "video/mp4", 0 },
            { cmdCheckMoveEnd,      NULL, 0 },

            // Pragma with single sparse chunk entry
            { cmdInitParser,        "ChildTrack=\"speff=5678\"", 0 },
            { cmdCheckMoveNext,     "speff", 5678 },
            { cmdCheckMoveEnd,      NULL, 0 },

            // Pragma with a fake ChildTrack chunk entry
            { cmdInitParser,        "NotARealChildTrack=\"speff=5678\"", 0 },
            { cmdCheckMoveEnd,      NULL, 0 },

            // Pragma with 2 sparse chunk entries
            { cmdInitParser,        "ChildTrack=\"speff=388828372323;tulver=11112222\"", 0 },
            { cmdCheckMoveNext,     "speff", 388828372323LL },
            { cmdCheckMoveNext,     "tulver", 11112222 },
            { cmdCheckMoveEnd,      NULL, 0 },

            // Pragma with 2 sparse chunk entries and extra directives
            { cmdInitParser,        "no-cache; CustomPragma=\"teststring\"; ChildTrack=\"speff=987654;tulver=123123\"; CustomPragma2=\"stringtest\"", 0 },
            { cmdCheckMoveNext,     "speff", 987654 },
            { cmdCheckMoveNext,     "tulver", 123123 },
            { cmdCheckMoveEnd,      NULL, 0 },

            // Pragma with a fake ChildTrack embedded in another pragma (must skip)
            { cmdInitParser,        "CustomPragma=\";ChildTrack=\\\"speff=1;tulver=2\\\"\";ChildTrack=\"real_one=3\"", 0 },
            { cmdCheckMoveNext,     "real_one", 3 },
            { cmdCheckMoveEnd,      NULL, 0 },

            // Pragma with single sparse chunk entries
            { cmdInitParser,        "no-cache;ChildTrack=\"captions=200400000000;\";IISMS/4.0;IIS Media Services by Microsoft", 0 },
            { cmdCheckMoveNext,     "captions", 200400000000LL },
            { cmdCheckMoveEnd,      NULL, 0 },

            // Pragma with 2 sparse chunk entries and extra directives, comma separated
            { cmdInitParser,        "no-cache, CustomPragma=\"teststring\", ChildTrack=\"speff=987654;tulver=123123\", CustomPragma2=\"stringtest\"", 0 },
            { cmdCheckMoveNext,     "speff", 987654 },
            { cmdCheckMoveNext,     "tulver", 123123 },
            { cmdCheckMoveEnd,      NULL, 0 },

            // Pragma with a fake ChildTrack embedded in another pragma (must skip), comma separated
            { cmdInitParser,        "CustomPragma=\",ChildTrack=\\\"speff=1;tulver=2\\\"\",ChildTrack=\"real_one=3\"", 0 },
            { cmdCheckMoveNext,     "real_one", 3 },
            { cmdCheckMoveEnd,      NULL, 0 },

            // Pragma with single sparse chunk entries, comma separated
            { cmdInitParser,        "no-cache,ChildTrack=\"captions=200400000000;\",IISMS/4.0,IIS Media Services by Microsoft", 0 },
            { cmdCheckMoveNext,     "captions", 200400000000LL },
            { cmdCheckMoveEnd,      NULL, 0 },

        };

        CSparseStreamChunkInfoHeaderParser fixture("");

        for( int iCmd = 0; iCmd < sizeof(rgCmds)/sizeof(rgCmds[0]); ++iCmd )
        {
            switch( rgCmds[iCmd].cmd )
            {
            case cmdInitParser:

                fixture = CSparseStreamChunkInfoHeaderParser( rgCmds[iCmd].pszVal );
                break;

            case cmdCheckMoveNext:

                PKTEST_ASSERT_EXIT( fixture.MoveNext() );
                PKTEST_ASSERT_EXIT( fixture.CurrentStreamName() == rgCmds[iCmd].pszVal );
                PKTEST_ASSERT_EXIT( fixture.CurrentChunkTime() == rgCmds[iCmd].value );
                break;

            case cmdCheckMoveEnd:

                PKTEST_ASSERT_EXIT( !fixture.MoveNext() );
                break;
            }
        }

    exit:

        return;
    }

        ////////////////////////////////////
    PKTEST_METHOD( SocketMbrRetry )
    {
        enum Command
        {
            cmdLoopCheckEndRetry,
            cmdLoopCheckEachRetry,
            cmdUpdateCounter,
            cmdReset,
        };

        struct
        {
            Command cmd;
            eSocketError socketError;
            int wsa;
            int repeat;
            CSocketMbrRetry::RetryAction retryAction;
        }
        static const rgCmds[] =
        {
            // 404s
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 404, 5,  CSocketMbrRetry::RETRY_SAME_IDX_DIFF_BR },
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 404, 1,  CSocketMbrRetry::RETRY_NEXT_IDX },
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 404, 5,  CSocketMbrRetry::RETRY_SAME_IDX_DIFF_BR },
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 404, 1,  CSocketMbrRetry::RETRY_NEXT_IDX },
            { cmdLoopCheckEndRetry,  eSocketErrorHttpInvalidResult, 404, 41, CSocketMbrRetry::NO_RETRY },

            // 412s
            { cmdReset, eSocketErrorNone, 0, 0 },
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 412, 25, CSocketMbrRetry::RETRY_SAME_IDX_SAME_BR },
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 412, 1,  CSocketMbrRetry::RETRY_NEXT_IDX },
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 412, 24, CSocketMbrRetry::RETRY_SAME_IDX_SAME_BR },
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 412, 1,  CSocketMbrRetry::NO_RETRY },

            // Chunk header parsing error
            { cmdReset, eSocketErrorNone, 0, 0 },
            { cmdLoopCheckEachRetry, eSocketErrorSSChunkHdrParsingFailed, 0, 50, CSocketMbrRetry::RETRY_SAME_IDX_DIFF_BR },
            { cmdLoopCheckEachRetry, eSocketErrorSSChunkHdrParsingFailed, 0, 1, CSocketMbrRetry::NO_RETRY },

            // Other error
            { cmdReset, eSocketErrorNone, 0, 0 },
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 405, 50, CSocketMbrRetry::RETRY_SAME_IDX_DIFF_BR },
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 405, 1, CSocketMbrRetry::NO_RETRY },
            
            // 404s with update to counters
            { cmdReset, eSocketErrorNone, 0, 0 },
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 404, 2,  CSocketMbrRetry::RETRY_SAME_IDX_DIFF_BR },
            { cmdUpdateCounter,      eSocketErrorHttpInvalidResult, 0,   0,  CSocketMbrRetry::RETRY_SAME_IDX_DIFF_BR },
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 404, 5,  CSocketMbrRetry::RETRY_SAME_IDX_DIFF_BR },
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 404, 1,  CSocketMbrRetry::RETRY_NEXT_IDX },
            { cmdLoopCheckEachRetry, eSocketErrorHttpInvalidResult, 404, 5,  CSocketMbrRetry::RETRY_SAME_IDX_DIFF_BR },
            { cmdLoopCheckEndRetry,  eSocketErrorHttpInvalidResult, 404, 40, CSocketMbrRetry::NO_RETRY },
        };

        CSocketMbrRetry retryFixture;
        CSocketMbrRetry::RetryAction action;
        for( int iCmd = 0; iCmd < sizeof(rgCmds)/sizeof(rgCmds[0]); ++iCmd )
        {
            switch( rgCmds[iCmd].cmd )
            {
                case cmdLoopCheckEachRetry:
                     for( int i=0; i<rgCmds[iCmd].repeat; i++)
                     {
                         action = retryFixture.GetRetryAction(rgCmds[iCmd].socketError, rgCmds[iCmd].wsa);
                         PKTEST_ASSERT_MSG_EXIT( action == rgCmds[iCmd].retryAction, "Failure at %d", iCmd );
                     }
                     break;
                case cmdLoopCheckEndRetry:
                     for( int i=0; i<rgCmds[iCmd].repeat; i++)
                     {
                         action = retryFixture.GetRetryAction(rgCmds[iCmd].socketError, rgCmds[iCmd].wsa);
                     }
                     PKTEST_ASSERT_MSG_EXIT( action == rgCmds[iCmd].retryAction, "Failure at %d", iCmd );
                     break;
            case cmdUpdateCounter:
                retryFixture.UpdateCounters(rgCmds[iCmd].socketError);
                break;
            case cmdReset:
                retryFixture.Reset();
                break;
            }
        }

    exit:

        return;
    }
};
