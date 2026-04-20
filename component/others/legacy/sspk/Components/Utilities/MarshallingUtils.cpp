///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MarshallingUtils.h"
#include <pkExecutive.h>
#include <limits>

////////////////////////////////////////////////////////////////////////////////
//
// BigEndian::ToHost implementations
//
////////////////////////////////////////////////////////////////////////////////

template< class T >
T BigEndian::BytesToHost( _In_bytecount_(cb) const void* p, size_t cb )
{
    // Ensure that the type is an integer, otherwise this method
    // of conversion doesn't apply (other types must specialize the template)
    pkCT_ASSERT( std::numeric_limits<int>::is_integer );

    T acc = 0;

    for( size_t i = 0; i < cb; ++i )
    {
        acc <<= 8;
        acc |= reinterpret_cast<const uint8_t*>( p )[i];
    }

    return( acc );
}

template< class T, size_t SIZE >
T BigEndian::BytesToHost( _In_bytecount_(SIZE) const void* p )
{
    return( BytesToHost<T>( p, SIZE ) );
}

template< class T >
T BigEndian::ToHost( _In_ T v )
{
    return( BytesToHost<T,sizeof(T)>( &v ) );
}

// Force instantiation of these:

template  uint8_t BigEndian::BytesToHost<  uint8_t >( _In_bytecount_(sizeof(uint8_t))  const void* p, size_t cb );
template uint16_t BigEndian::BytesToHost< uint16_t >( _In_bytecount_(sizeof(uint16_t)) const void* p, size_t cb );
template uint32_t BigEndian::BytesToHost< uint32_t >( _In_bytecount_(sizeof(uint32_t)) const void* p, size_t cb );
template uint64_t BigEndian::BytesToHost< uint64_t >( _In_bytecount_(sizeof(uint64_t)) const void* p, size_t cb );

template  uint8_t BigEndian::BytesToHost<  uint8_t, 1 >( _In_bytecount_(1) const void* p );
template uint16_t BigEndian::BytesToHost< uint16_t, 2 >( _In_bytecount_(2) const void* p );
template uint32_t BigEndian::BytesToHost< uint32_t, 3 >( _In_bytecount_(3) const void* p );
template uint32_t BigEndian::BytesToHost< uint32_t, 4 >( _In_bytecount_(4) const void* p );
template uint64_t BigEndian::BytesToHost< uint64_t, 8 >( _In_bytecount_(8) const void* p );

template  uint8_t BigEndian::ToHost< uint8_t>( _In_  uint8_t v );
template uint16_t BigEndian::ToHost<uint16_t>( _In_ uint16_t v );
template uint32_t BigEndian::ToHost<uint32_t>( _In_ uint32_t v );
template uint64_t BigEndian::ToHost<uint64_t>( _In_ uint64_t v );

// Specialization for GUID

template<>
GUID BigEndian::BytesToHost<GUID>( _In_bytecount_(cb) const void* p, size_t cb )
{
    GUID result;

    memcpy_s( &result, sizeof(result), p, cb );

    result.Data1 = ToHost( result.Data1 );
    result.Data2 = ToHost( result.Data2 );
    result.Data3 = ToHost( result.Data3 );

    return( result );
}

template<> GUID BigEndian::BytesToHost< GUID, sizeof(GUID) >( _In_bytecount_(GUID) const void* p )
{
    return( BytesToHost<GUID>( p, sizeof(GUID) ) );
}

template<>
GUID BigEndian::ToHost<GUID>( _In_ GUID v )
{
    return( BytesToHost<GUID>( &v, sizeof(v) ) );
}

////////////////////////////////////////////////////////////////////////////////
//
// LittleEndian::ToHost implementations
//
////////////////////////////////////////////////////////////////////////////////

template< class T >
T LittleEndian::BytesToHost( _In_bytecount_(cb) const void* p, size_t cb )
{
    // Ensure that the type is an integer, otherwise this method
    // of conversion doesn't apply (other types must specialize the template)
    pkCT_ASSERT( std::numeric_limits<int>::is_integer );

    T acc = 0;
    size_t i = cb;

    while( i > 0 )
    {
        --i;

        acc <<= 8;
        acc |= reinterpret_cast<const uint8_t*>( p )[i];
    }

    return( acc );
}

template< class T, size_t SIZE >
T LittleEndian::BytesToHost( _In_bytecount_(SIZE) const void* p )
{
    return( BytesToHost<T>( p, SIZE ) );
}

template< class T >
T LittleEndian::ToHost( _In_ T v )
{
    return( BytesToHost<T,sizeof(T)>( &v ) );
}

// Force instantiation of these:

template  uint8_t LittleEndian::BytesToHost<  uint8_t >( _In_bytecount_(sizeof(uint8_t))  const void* p, size_t cb );
template uint16_t LittleEndian::BytesToHost< uint16_t >( _In_bytecount_(sizeof(uint16_t)) const void* p, size_t cb );
template uint32_t LittleEndian::BytesToHost< uint32_t >( _In_bytecount_(sizeof(uint32_t)) const void* p, size_t cb );
template uint64_t LittleEndian::BytesToHost< uint64_t >( _In_bytecount_(sizeof(uint64_t)) const void* p, size_t cb );

template  uint8_t LittleEndian::BytesToHost<  uint8_t, 1 >( _In_bytecount_(1) const void* p );
template uint16_t LittleEndian::BytesToHost< uint16_t, 2 >( _In_bytecount_(2) const void* p );
template uint32_t LittleEndian::BytesToHost< uint32_t, 3 >( _In_bytecount_(3) const void* p );
template uint32_t LittleEndian::BytesToHost< uint32_t, 4 >( _In_bytecount_(4) const void* p );
template uint64_t LittleEndian::BytesToHost< uint64_t, 8 >( _In_bytecount_(8) const void* p );

template  uint8_t LittleEndian::ToHost< uint8_t>( _In_  uint8_t v );
template uint16_t LittleEndian::ToHost<uint16_t>( _In_ uint16_t v );
template uint32_t LittleEndian::ToHost<uint32_t>( _In_ uint32_t v );
template uint64_t LittleEndian::ToHost<uint64_t>( _In_ uint64_t v );

// Specialization for GUID

template<>
GUID LittleEndian::BytesToHost<GUID>( _In_bytecount_(cb) const void* p, size_t cb )
{
    GUID result;

    memcpy_s( &result, sizeof(result), p, cb );

    result.Data1 = ToHost( result.Data1 );
    result.Data2 = ToHost( result.Data2 );
    result.Data3 = ToHost( result.Data3 );

    return( result );
}

template<> GUID LittleEndian::BytesToHost< GUID, sizeof(GUID) >( _In_bytecount_(GUID) const void* p )
{
    return( BytesToHost<GUID>( p, sizeof(GUID) ) );
}

template<>
GUID LittleEndian::ToHost<GUID>( _In_ GUID v )
{
    return( BytesToHost<GUID>( &v, sizeof(v) ) );
}
