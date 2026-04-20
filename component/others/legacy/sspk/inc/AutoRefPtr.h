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
// _InterfacePtrWithoutCounting<T>
//
//  Hides the reference counting methods for AutoRefPtr<T>::operator-> to
//  prevent accidental calls
//
////////////////////////////////////////////////////////////////////////////////

template< class T >
class _InterfacePtrWithoutCounting : public T
{
private:

    void AddRef() {};
    void Release() {};

    _InterfacePtrWithoutCounting() {};
    ~_InterfacePtrWithoutCounting() {};
};

////////////////////////////////////////////////////////////////////////////////
//
// AutoRefPtr<T> - auto pointer to reference counted interface
//
////////////////////////////////////////////////////////////////////////////////

template< class T >
class AutoRefPtr
{
public:

    // Default constructor
    AutoRefPtr()
        : _p( NULL )
    {
    }

    // Copy constructor
    AutoRefPtr( _In_ const AutoRefPtr<T>& that )
        : _p( NULL )
    {
        Set( that._p );
    }

#if USE_MOVE_SEMANTICS_FOR_AUTOREFPTR

    // Move constructor
    AutoRefPtr( _In_ AutoRefPtr<T>&& that )
    {
        _p = that._p;
        that._p = NULL;
    }

#endif

    // Constructor from a pointer
    explicit
    AutoRefPtr( _In_opt_ T* pIn )
        : _p( NULL )
    {
        Set( pIn );
    }

    // Destructor
    ~AutoRefPtr()
    {
        Release();
    }

    // Copy operator
    AutoRefPtr& operator = ( _In_ const AutoRefPtr& that )
    {
        if( this != &that )
        {
            Set( that._p );
        }
        return( *this );
    }

#if USE_MOVE_SEMANTICS_FOR_AUTOREFPTR

    // Move operator
    AutoRefPtr& operator = ( _In_ AutoRefPtr&& that )
    {
        AdoptRef( that.HandOffRef() );
        return( *this );
    }

#endif

    // Set
    void Set( _In_opt_ T* pIn )
    {
        T* pOld = _p;

        _p = pIn;

        if( _p != NULL )
        {
            _p->AddRef();
        }

        if( pOld != NULL )
        {
            pOld->Release();
        }
    }

    // Release
    void Release()
    {
        Set( NULL );
    }

    //
    // Accessors
    //

    operator T*() const
    {
        return( _p );
    }

    _InterfacePtrWithoutCounting<T>*
    operator->() const
    {
        return( reinterpret_cast<_InterfacePtrWithoutCounting<T>*>( _p ) );
    }

    //
    // To use the object as a deref output param pointer
    //

    T** DerefOutPtr()
    {
        pkASSERT( _p == NULL );
        return( &_p );
    }

    //
    // Operations to move ownership of the reference
    //

    T* HandOffRef()
    {
        T* pOut = _p;
        _p = NULL;
        return( pOut );
    }

    void AdoptRef( _In_opt_ T* pIn )
    {
        T* pOld = _p;
        _p = pIn;

        if( pOld != NULL )
        {
            pOld->Release();
        }
    }

private:

    T* _p;

    // Hiding copy operator from a pointer to
    // avoid accidental use
    void operator = ( _In_opt_ T* pIn );
};

