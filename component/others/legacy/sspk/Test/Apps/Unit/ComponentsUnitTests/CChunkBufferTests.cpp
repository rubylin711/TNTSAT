///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <SSPKDefines.h>
#include "CChunkBuffer.h"

using namespace MBR;

////////////////////////////////////////////////////////////////////////////////
PKTEST_GROUP( CChunkBufferTests )
{
    enum Command
    {
        cmdInitBuffer,
        cmdAddMax,
        cmdAddMin,
        cmdCheckMax,
        cmdCheckMin,
        cmdCheckCount,
        cmdInvalidAddMax,
        cmdInvalidAddMin
    };

    static const int32_t c_bufferSize = 10800;

    ////////////////////////////////////
    PKTEST_METHOD( CChunkBuffer_AddChunks )
    {
        struct
        {
            Command cmd;
            int32_t value1;
            int32_t value2;
        }
        static const rgCmds[] =
        {
            { cmdInitBuffer,    5,                  0               },      // create buffer
            { cmdAddMax,        1,                  c_bufferSize-1  },      // full up front of buffer to c_bufferSize-1
            { cmdCheckMax,      c_bufferSize*2-2,   c_bufferSize-1  },      // check front
            { cmdCheckMin,      c_bufferSize,       1               },      // check back
            { cmdCheckCount,    c_bufferSize-1,     0               },      // check count == c_bufferSize - 1

            { cmdAddMax,        c_bufferSize,       1               },      // add 1 to front to exactly full
            { cmdCheckMax,      c_bufferSize*2-1,   c_bufferSize    },      // check front
            { cmdCheckMin,      c_bufferSize,       1               },      // check back
            { cmdCheckCount,    c_bufferSize,       0               },      // check count == c_bufferSize

            { cmdAddMax,        c_bufferSize+1,     1               },      // add 1 to front to rollover
            { cmdCheckMax,      c_bufferSize*2,     c_bufferSize+1  },      // check front
            { cmdCheckMin,      c_bufferSize+1,     2               },      // check back
            { cmdCheckCount,    c_bufferSize,       0               },      // check count == c_bufferSize

            { cmdAddMax,        c_bufferSize+2,     3               },      // add 3 to front
            { cmdCheckMax,      c_bufferSize*2+3,   c_bufferSize+4  },      // check front
            { cmdCheckMin,      c_bufferSize+4,     5               },      // check back
            { cmdCheckCount,    c_bufferSize,       0               },      // check count == c_bufferSize

            ///////////////////////////////////////////////////////////////////////////////////////////

            { cmdInitBuffer,    5,                  0               },      // create buffer
            { cmdAddMax,        5,                  c_bufferSize-2  },      // full up buffer to c_bufferSize-2
            { cmdCheckMin,      c_bufferSize,       5               },      // check back
            { cmdCheckMax,      c_bufferSize*2-3,   c_bufferSize+2  },      // check front
            { cmdCheckCount,    c_bufferSize-2,     0               },      // check count == c_bufferSize - 2
            
            { cmdAddMin,        4,                  1               },      // add back, buffer full -1
            { cmdCheckMin,      c_bufferSize-1,     4               },      // check back
            { cmdCheckMax,      c_bufferSize*2-3,   c_bufferSize+2  },      // check front
            { cmdCheckCount,    c_bufferSize-1,     0               },      // check count == c_bufferSize - 1

            { cmdAddMin,        3,                  1               },      // add back, buffer is full
            { cmdCheckMin,      c_bufferSize-2,     3               },      // check back
            { cmdCheckMax,      c_bufferSize*2-3,   c_bufferSize+2  },      // check front
            { cmdCheckCount,    c_bufferSize,       0               },      // check count == c_bufferSize

            { cmdInvalidAddMin, 2,                  1               },      // invalid add back b/c buffer is now full
            { cmdCheckMin,      c_bufferSize-2,     3               },      // check back
            { cmdCheckMax,      c_bufferSize*2-3,   c_bufferSize+2  },      // check front
            { cmdCheckCount,    c_bufferSize,       0               },      // check count == c_bufferSize

            { cmdAddMax,        c_bufferSize+3,     1               },      // add to front succeeds
            { cmdCheckMin,      c_bufferSize-1,     4               },      // check back moved up one
            { cmdCheckMax,      c_bufferSize*2-2,   c_bufferSize+3  },      // check front
            { cmdCheckCount,    c_bufferSize,       0               },      // check count == c_bufferSize

            { cmdInvalidAddMin, 1,                  1               },      // buffer is too full to add to back
            { cmdInvalidAddMax, 0,                  1               },      // invalid value

            ///////////////////////////////////////////////////////////////////////////////////////////

            { cmdInitBuffer,    5,                  0               },      // create buffer
            { cmdAddMin,        c_bufferSize,       c_bufferSize/2  },      // full up back buffer to c_bufferSize/2
            { cmdCheckMin,      c_bufferSize/2,     c_bufferSize/2+1},      // check back
            { cmdCheckMax,      c_bufferSize-1,     c_bufferSize    },      // check front
            { cmdCheckCount,    c_bufferSize/2,     0               },      // check count == c_bufferSize/2

            { cmdAddMax,        c_bufferSize+1,     c_bufferSize/2  },      // full up front buffer to c_bufferSize/2
            { cmdCheckMin,      c_bufferSize/2,     c_bufferSize/2+1},      // check back
            { cmdCheckMax,      c_bufferSize
                                 +c_bufferSize/2-1,   c_bufferSize
                                                       +c_bufferSize/2  },  // check front
            { cmdCheckCount,    c_bufferSize,       0               },      // check count == c_bufferSize

            { cmdInvalidAddMin, c_bufferSize/2,     1               },      // invalid add back b/c buffer is now full
            
            { cmdAddMax,        c_bufferSize
                                 +c_bufferSize/2+1,   1               },    // add to front succeeds
            { cmdCheckMin,      c_bufferSize/2+1,   c_bufferSize/2+2},      // check back
            { cmdCheckMax,      c_bufferSize
                                 +c_bufferSize/2,     c_bufferSize
                                                       +c_bufferSize/2+1},  // check front
            { cmdCheckCount,    c_bufferSize,       0               },      // check count == c_bufferSize

            ///////////////////////////////////////////////////////////////////////////////////////////

            { cmdInitBuffer,    5,                  0               },      // create buffer
            { cmdAddMax,        c_bufferSize+1,     c_bufferSize/2  },      // full up front buffer to c_bufferSize/2
            { cmdCheckMin,      c_bufferSize,       c_bufferSize+1  },      // check back
            { cmdCheckMax,      c_bufferSize
                                 +c_bufferSize/2-1,   c_bufferSize
                                                       +c_bufferSize/2  },  // check front
            { cmdCheckCount,    c_bufferSize/2,     0               },      // check count == c_bufferSize/2

            { cmdAddMin,        c_bufferSize,       c_bufferSize/2  },      // full up back buffer to c_bufferSize/2
            { cmdCheckMin,      c_bufferSize/2,     c_bufferSize/2+1},      // check back
            { cmdCheckMax,      c_bufferSize
                                 +c_bufferSize/2-1,   c_bufferSize
                                                       +c_bufferSize/2  },  // check front
            { cmdCheckCount,    c_bufferSize,       0               },      // check count == c_bufferSize



            { cmdInvalidAddMin, c_bufferSize/2,     1               },      // invalid add back b/c buffer is now full
            
            { cmdAddMax,        c_bufferSize
                                 +c_bufferSize/2+1,   1               },    // add to front succeeds
            { cmdCheckMin,      c_bufferSize/2+1,   c_bufferSize/2+2},      // check back
            { cmdCheckMax,      c_bufferSize
                                 +c_bufferSize/2,     c_bufferSize
                                                       +c_bufferSize/2+1},  // check front
            { cmdCheckCount,    c_bufferSize,       0               },      // check count = c_bufferSize
        };

        CChunkBuffer* buffer = NULL;
        int32_t initialCount = 0;
        for( int iCmd = 0; iCmd < sizeof(rgCmds)/sizeof(rgCmds[0]); ++iCmd )
        {
            switch( rgCmds[iCmd].cmd )
            {
            case cmdInitBuffer:
                if (buffer)
                {
                    delete buffer;
                }
                buffer = CChunkBuffer::Create(rgCmds[iCmd].value1, true);
                PKTEST_ASSERT_MSG_EXIT( NULL != buffer, 
                            "Failed cmdInitBuffer failed");
                buffer->SetDefaultChunkDuration(1);
                break;

            case cmdAddMax:
                initialCount = buffer->Count();
                for(int32_t i = 0; i < rgCmds[iCmd].value2; i++)
                {
                    PKTEST_ASSERT_MSG_EXIT( NULL != buffer->Add(rgCmds[iCmd].value1 + i), 
                            "Failed cmdAddMax Add %d", 
                            rgCmds[iCmd].value1);
                }
                break;

            case cmdAddMin:
                initialCount = buffer->Count();
                for(int32_t i = 0; i < rgCmds[iCmd].value2; i++)
                {
                    PKTEST_ASSERT_MSG_EXIT( NULL != buffer->Add(rgCmds[iCmd].value1 - i, eBufferDirection_Backward), 
                            "Failed cmdAddMin %d", 
                            rgCmds[iCmd].value1);

                }
                break;

            case cmdCheckMax:
               PKTEST_ASSERT_MSG_EXIT( rgCmds[iCmd].value1 == buffer->GetMaxIndex(),
                            "Failed cmdCheckMax index expected %d, but got %d",
                            rgCmds[iCmd].value1,
                            buffer->GetMaxIndex());

               PKTEST_ASSERT_MSG_EXIT( rgCmds[iCmd].value2 == buffer->GetChunkTicks(buffer->GetMaxIndex()),
                            "Failed cmdCheckMax time expected %d, but got %d",
                            rgCmds[iCmd].value2,
                            buffer->GetChunkTicks(buffer->GetMaxIndex()));
                break;

            case cmdCheckMin:
               PKTEST_ASSERT_MSG_EXIT( rgCmds[iCmd].value1 == buffer->GetMinIndex(),
                            "Failed cmdCheckMin index expected %d, but got %d",
                            rgCmds[iCmd].value1,
                            buffer->GetMinIndex());

               PKTEST_ASSERT_MSG_EXIT( rgCmds[iCmd].value2 == buffer->GetChunkTicks(buffer->GetMinIndex()),
                            "Failed cmdCheckMin time expected %d, but got %d",
                            rgCmds[iCmd].value1,
                            buffer->GetChunkTicks(buffer->GetMinIndex()));
                break;
            
            case cmdCheckCount:                 
                PKTEST_ASSERT_MSG_EXIT(rgCmds[iCmd].value1 == buffer->Count(), 
                            "Failed cmdAddMax count expected %d, but got %d",
                            rgCmds[iCmd].value1,
                            buffer->Count());
                break;
            case cmdInvalidAddMin:
                PKTEST_ASSERT_MSG_EXIT( NULL == buffer->Add(rgCmds[iCmd].value1, eBufferDirection_Backward), 
                            "Failed cmdInvalidAddMin %d", 
                            rgCmds[iCmd].value1);
                break;

            case cmdInvalidAddMax:
                PKTEST_ASSERT_MSG_EXIT( NULL == buffer->Add(rgCmds[iCmd].value1, eBufferDirection_Forward), 
                            "Failed cmdInvalidAddMax %d", 
                            rgCmds[iCmd].value1);
                break;
            }
        }

    exit:

        return;
    }
};
