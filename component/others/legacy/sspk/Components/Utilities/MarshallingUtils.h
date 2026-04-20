///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// <summary>
/// MarshallingUtils.h
/// </summary>

/// <summary>
/// Utility functions to convert to/from big endian representations
/// </summary>
struct BigEndian
{
    /// <summary>
    /// Converts a sequence of bytes to the host's representation.
    /// In the event of an overflow, the result of the operation will
    /// be truncated.
    /// </summary>
    /// <returns>
    /// Value in the host's representation
    /// </returns>
    template< class T >
    static T BytesToHost( _In_bytecount_(cb) const void* p, size_t cb );

    /// <summary>
    /// Converts a sequence of bytes of a fixed length to the host's representation.
    /// </summary>
    /// <returns>
    /// Value in the host's representation
    /// </returns>
    template< class T, size_t SIZE >
    static T BytesToHost( _In_bytecount_(SIZE) const void* p );

    /// <summary>
    /// Converts from big endian representation to the host's representation.
    /// </summary>
    /// <returns>
    /// Value in the host's representation
    /// </returns>
    template< class T >
    static T ToHost( _In_ T v );
};


/// <summary>
/// Utility functions to convert to/from little endian representations
/// </summary>
struct LittleEndian
{
    /// <summary>
    /// Converts a sequence of bytes to the host's representation.
    /// In the event of an overflow, the result of the operation will
    /// be truncated.
    /// </summary>
    /// <returns>
    /// Value in the host's representation
    /// </returns>
    template< class T >
    static T BytesToHost( _In_bytecount_(cb) const void* p, _In_ size_t cb );

    /// <summary>
    /// Converts a sequence of bytes of a fixed length to the host's representation.
    /// </summary>
    /// <returns>
    /// Value in the host's representation
    /// </returns>
    template< class T, size_t SIZE >
    static T BytesToHost( _In_bytecount_(SIZE) const void* p );

    /// <summary>
    /// Converts from little endian representation to the host's representation.
    /// </summary>
    /// <returns>
    /// Value in the host's representation
    /// </returns>
    template< class T >
    static T ToHost( T v );
};
