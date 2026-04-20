///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <IManifestTrack.h>
#include <ManifestChunk.h>

////////////////////////////////////////////////////////////////////////////////
//
// ChunkIterator::ContextAccessor - allows access to ChunkIterator::CONTEXT
//
//  The definition of ChunkIterator is not supposed to give public access to
//  its internal context to application code, yet such access is necessary for
//  internal code. The ContextAccessor class does "friend forwarding" to internal
//  code that needs access to the data.
//
////////////////////////////////////////////////////////////////////////////////

struct ChunkIterator::ContextAccessor
{
protected:

    static ChunkIterator::CONTEXT& CtxOf( _In_ const ChunkIterator& it )
    {
        return( it.m_Ctx );
    }

    static ChunkIterator MakeChunkIterator(
                                _In_ void* pToken,
                                _In_ int64_t minTime,
                                _In_ int64_t refTime )
    {
        // Use the private constructor for ContextAccessors
        return( ChunkIterator( pToken, minTime, refTime ) );
    }
};

