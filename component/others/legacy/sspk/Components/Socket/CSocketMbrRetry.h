///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/*
 * CSocketMbrRetry.h
 *
 * CSocketMbrRetry keeps tracks of the state of retries of downloading chunks and chuckInfos
 */
#pragma once

#include "SSPKDefines.h"

class CSocketMbrRetry
{
public:
    enum RetryAction
    {
        NO_RETRY = 0,
        RETRY_SAME_IDX_SAME_BR,
        RETRY_SAME_IDX_DIFF_BR,
        RETRY_NEXT_IDX
    };

    CSocketMbrRetry()
        : _chunk412Errors(0)
        , _chunk404Errors(0)
        , _totalErrors(0)
    {
    }

    RetryAction GetRetryAction(eSocketError socketError, int wsa)
    {
        // Retry possibilities:
        // 1. Try same chunk, same bitrate again
        // 2. Try same chunk, new bitrate
        // 3. Try next chunk, same bitrate
        static const int MAX_CONSECUTIVE_CHUNK_DOWNLOAD_ERRORS = 50;
        static const int MAX_PER_CHUNK_404_ERRORS = 5;
        static const int MAX_PER_CHUNK_412_ERRORS = 25;

        // check if retries have hit maximum limit
        if( ++_totalErrors > MAX_CONSECUTIVE_CHUNK_DOWNLOAD_ERRORS)
        {
            return NO_RETRY;
        }

        //Server returns status code 412 (Precondition Failed) to signal "Fragment Not Yet Available"
        //which means that fragment is not ready yet but should become available
        if ( HTTP_STATUS_PRECOND_FAILED == wsa )
        {
            //  retry limit has been hit, try the next chunk
            if ( ++_chunk412Errors > MAX_PER_CHUNK_412_ERRORS )
            {
                UpdateCounters(socketError);
                return RETRY_NEXT_IDX; 
            }
            else
            {
                return RETRY_SAME_IDX_SAME_BR;
            }
        }
        
        if ( HTTP_STATUS_NOT_FOUND == wsa )
        {           
            if ( ++_chunk404Errors > MAX_PER_CHUNK_404_ERRORS )
            {
                UpdateCounters(socketError);
                return RETRY_NEXT_IDX;
            }
            else
            {
                return RETRY_SAME_IDX_DIFF_BR;
            }
        }
    
        // if not a 404 or 412 then default to retrying 
        // the same chunk with different BR
        return RETRY_SAME_IDX_DIFF_BR;
    }

    void UpdateCounters(eSocketError socketError)
    {
        if( eSocketErrorHttpInvalidResult == socketError )
        {
            _chunk412Errors = 0;
            _chunk404Errors = 0;
        }
    }

    void Reset()
    {
        _chunk412Errors = 0;
        _chunk404Errors = 0;
        _totalErrors = 0;
    }

private:
    int _chunk412Errors;
    int _chunk404Errors;
    int _totalErrors;
};
