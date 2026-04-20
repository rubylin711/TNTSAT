///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include "pkPAL.h"
#include "pkExecutive.h"

class Lockable
{
public:
    const static UINT32 IgnoreMeId = 0xffffffff;;
    inline
    Lockable():iEnterCount(0), iId(0)
    {
        Executive_CreateLock(&_cs);
    }

    inline
    virtual    ~Lockable()
    {
        Executive_DeleteLock(_cs);
    };

    inline
    void Lock(UINT32 id = 0)
    {
        Executive_EnterLock(_cs);
        iEnterCount++;
        iId = id;
    }

    inline
    bool TryLock(UINT32 id = 0)
    {
        bool retVal = (pkS_FALSE != Executive_TryEnterLock(_cs));

        if (retVal )
        {
            iEnterCount++;
            iId = id;
        }
        return retVal;
    }

    inline
    void Unlock()
    {
        pkASSERT(iEnterCount != 0);
        pkASSERT(Executive_IsLockedByCurrentThread(_cs));

        if (--iEnterCount == 0) {
            iId = 0;
        }
        Executive_ExitLock(_cs);
    }

private:
    // This class cannot be copied:
    Lockable(const Lockable&);
    Lockable& operator=(const Lockable&);

    pkHANDLE _cs;
    UINT32 iEnterCount;
    UINT32 iId;
};

//    Enter and Leave
class AutoLock
{
public:
    inline
    AutoLock(Lockable* target, UINT32 id = 0) : _target(target)
    {
        _target->Lock(id);
    }

    inline
    ~AutoLock()
    {
        _target->Unlock();
    }
private:
    // No default construction:
    AutoLock();
    // This class cannot be copied:
    AutoLock( const AutoLock& );
    AutoLock& operator=(const AutoLock&);

    Lockable* _target;
};

