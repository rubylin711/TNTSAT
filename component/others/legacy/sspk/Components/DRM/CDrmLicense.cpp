///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDrmLicense.h"

IDrmLicense::IDrmLicense()
    : _type(DrmEncryptionType_Unknown)
    , _handle(DRM_INVALID_LICENSE)
{
}

void CByteBuffer::Clear()
{
    delete[] _pb;
    _pb = NULL;
    _cb = 0;
}

bool CByteBuffer::Set(
    _In_opt_bytecount_(cb) const void* pv,
    _In_ uint32 cb )
{
    bool fSuccess = false;

    byte* pbOldBuffer = _pb;

    _pb = NULL;
    _cb = 0;

    if( pv != NULL )
    {
        _pb = NEW_NO_THROW byte [cb];
        if( _pb != NULL )
        {
            memcpy_s( _pb, cb, pv, cb );
            _cb = cb;
            fSuccess = true;
        }
    }
    else
    {
        fSuccess = true;
    }

    delete [] pbOldBuffer;

    return( fSuccess );
}
