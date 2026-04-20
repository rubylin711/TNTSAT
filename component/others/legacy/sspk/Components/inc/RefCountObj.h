///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// CRefCountedObj - implements reference counting for a base object
//
////////////////////////////////////////////////////////////////////////////////

template< class Base >
class CRefCountedObj
    : public Base
{
private:

    CRefCountedObj( const CRefCountedObj& that );
    void operator = ( const CRefCountedObj& that );

    LONG m_cRefs;

public:

    CRefCountedObj()
        : m_cRefs( 1 )
    {
    }

    void AddRef()
    {
        InterlockedIncrement( &m_cRefs );
    }

    void Release()
    {
        if( InterlockedDecrement( &m_cRefs ) == 0 )
        {
            delete this;
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//
// IElementToContainerConnection<I> - interface used by the element of a
// container class to refer back to its container without adding a circular
// reference.
//
////////////////////////////////////////////////////////////////////////////////

template< class I >
class IElementToContainerConnection
{
public:

    virtual void AddRef() = 0;

    virtual void Release() = 0;

    virtual void GetContainer( _Deref_out_ I** ppContainer ) = 0;
};

////////////////////////////////////////////////////////////////////////////////
//
// CElementToContainerConnection<T,CONTAINER> - class to implement
// IElementToContainerConnection<I> for a given interface I on a given concrete
// class CONTAINER. The CONTAINER class must implement the InnerAddRef and
// InnerRelease methods.
//
////////////////////////////////////////////////////////////////////////////////

template< class I, class CONTAINER >
class CElementToContainerConnection
    : public IElementToContainerConnection<I>
{
public:

    CElementToContainerConnection()
        : m_wpContainer( NULL )
    {
    }

    void Initialize( _In_ CONTAINER* wpContainer )
    {
        ASSERT( m_wpContainer == NULL );
        m_wpContainer = wpContainer;
    }

    //
    // IElementToContainerConnection<I>
    //

    __override void AddRef()
    {
        m_wpContainer->InnerAddRef();
    }

    __override void Release()
    {
        m_wpContainer->InnerRelease();
    }

    __override void GetContainer( _Deref_out_ I** ppContainer )
    {
        *ppContainer = m_wpContainer;
        m_wpContainer->AddRef();
    }

private:

    CONTAINER* m_wpContainer;
};

