///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "CManifestChunk.h"

////////////////////////////////////////////////////////////////////////////////
//
// ChunkIterator - implementation
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
ChunkIterator::ChunkIterator()
{
    memset( &m_Ctx, 0, sizeof(m_Ctx) );
}

////////////////////////////////////////////////////////////////////////////////
ChunkIterator::ChunkIterator( const ChunkIterator& that )
    : m_Ctx( that.m_Ctx )
{
}

////////////////////////////////////////////////////////////////////////////////
ChunkIterator& ChunkIterator::operator = ( const ChunkIterator & that )
{
    m_Ctx = that.m_Ctx;
    return( *this );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT ChunkIterator::MoveNext()
{
    pkRESULT pkResult = pkS_OK;

    if( !m_Ctx.m_fByIndex )
    {
        // The iterator can't be moved until it gets resolved into an index
        TRACE_ERROR(("iterator can't move, not an index yet"));
        pkResult = pkE_INVALID_REQUEST;
        goto exit;
    }

    ++m_Ctx.m_iChunk;

exit:

    return( pkResult );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT ChunkIterator::MovePrev()
{
    pkRESULT pkResult = pkS_OK;

    if( !m_Ctx.m_fByIndex )
    {
        // The iterator can't be moved until it gets resolved into an index
        TRACE_ERROR(("iterator can't move, not an index yet"));
        pkResult = pkE_INVALID_REQUEST;
        goto exit;
    }

    --m_Ctx.m_iChunk;

exit:

    return( pkResult );
}

////////////////////////////////////////////////////////////////////////////////
ChunkIterator::ChunkIterator(
    _In_ void* pToken,
    _In_ int64_t minTime,
    _In_ int64_t refTime )
{
    memset( &m_Ctx, 0, sizeof(m_Ctx) );

    m_Ctx.m_pToken = pToken;
    m_Ctx.m_minTime = minTime;
    m_Ctx.m_time = refTime;
    m_Ctx.m_fByIndex = false;
}

////////////////////////////////////////////////////////////////////////////////
//
// CRefBuffer - IRefBuffer default implementation
//
////////////////////////////////////////////////////////////////////////////////

class CRefBuffer
    : public IRefBuffer
{
public:

    CRefBuffer()
        : _pbData( NULL )
        , _cbData( 0 )
    {
    }

    __override ~CRefBuffer()
    {
        delete [] _pbData;
    }

    pkRESULT InitOnce( size_t cb )
    {
        ASSERT( _pbData == NULL );

        pkRESULT pkr = pkS_OK;

        _pbData = NEW_NO_THROW byte[ cb ];
        if( _pbData == NULL )
        {
            pkr = pkE_OUTOFMEMORY;
            goto exit;
        }

        _cbData = cb;

    exit:

        return( pkr );
    }

    //
    // IRefBuffer
    //

    __override size_t Length()
    {
        return( _cbData );
    }

    __override byte* Data()
    {
        return( _pbData );
    }

public:

    byte* _pbData;
    size_t _cbData;
};

////////////////////////////////////////////////////////////////////////////////
//
// Creates a reference counted buffer
//
////////////////////////////////////////////////////////////////////////////////

pkRESULT
CreateRefBuffer(
    _In_ size_t cbLength,
    _Deref_out_ IRefBuffer** ppBuffer )
{
    pkRESULT pkResult = pkS_OK;

    *ppBuffer = NULL;

    AutoRefPtr<CRefBuffer> spBuffer;

    spBuffer.AdoptRef( NEW_NO_THROW CRefCountedObj<CRefBuffer>() );

    if( spBuffer == NULL )
    {
        pkResult = pkE_OUTOFMEMORY;
        goto exit;
    }

    pkResult = spBuffer->InitOnce( cbLength );
    if( pkFAILED(pkResult) )
    {
        goto exit;
    }

    *ppBuffer = spBuffer.HandOffRef();

exit:

    return( pkResult );
}

