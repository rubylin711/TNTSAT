///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IDrmManager.h"


/// <summary>
/// IDrmLicense - interface for license
/// </summary>

class IDrmLicense
{
public:

    DrmEncryptionType GetEncryptionType(void) { return _type; }

    uint32 GetHandle(void) const { return _handle; }

    virtual void GetLicenseTimes( _Out_ DrmLicenseTiming* pTimesOut ) = 0;

    virtual byte* GetDefaultKeyID( _Out_ uint32 *pLen ) = 0;

    virtual IXDrm* GetIXDrm(void) = 0;

protected:

    IDrmLicense();
    virtual ~IDrmLicense() {};

    // Attributes
    DrmEncryptionType _type;
    uint32 _handle;
};


/// <summary>
/// IDrmLicense - interface for cacheable license
/// </summary>

class IDrmCacheableLicense
    : public IDrmLicense
{
public:

    virtual void Dispose() = 0;

protected:

    IDrmCacheableLicense() {};
    virtual ~IDrmCacheableLicense() {};
};

/// <summary>
/// Generic buffer of bytes
/// </summary>

class CByteBuffer
{
public:

    CByteBuffer()
        : _pb( NULL )
        , _cb( 0 )
    {
    }

    ~CByteBuffer()
    {
        Clear();
    }

    byte* Data()    { return _pb; }
    uint32 Size()   { return _cb; }

    bool Set( _In_opt_bytecount_(cb) const void* pv, _In_ uint32 cb );

    void Clear();

private:

    byte*   _pb;
    uint32  _cb;

    CByteBuffer( const CByteBuffer& );          // no access
    void operator = ( const CByteBuffer& );     // no access
};

