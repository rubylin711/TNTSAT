///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <platBase.h>
#include <pkExecutive.h>

#include "CManifest.h"

////////////////////////////////////////////////////////////////////////////////
//
// The implemenation of CManifest
//
////////////////////////////////////////////////////////////////////////////////

CManifest::CManifest()
    : m_cRefs (1)
{
}

void CManifest::AddRef()
{
    Executive_InterlockedIncrement( &m_cRefs );
}

void CManifest::Release()
{
    if( Executive_InterlockedDecrement( &m_cRefs ) == 0 )
    {
        delete this;
    }
}

bool CManifest::GetAttribute(
    _In_ const std::wstring& name,
    _Out_ std::wstring* pValue ) const
{
    bool fSuccess = false;
    pkASSERT(NULL != pValue);
    pValue->clear();

    std::map<std::wstring, std::wstring>::const_iterator it = m_attributes.find( name );

    if (it != m_attributes.end())
    {
        *pValue = it->second;
        fSuccess = true;
    }

    return (fSuccess);
}

pkRESULT CManifest::SetAttribute(
    _In_ const std::wstring& name,
    _In_ const std::wstring& value )
{
    pkRESULT pkResult = pkS_OK;

    if (m_attributes.end() != m_attributes.find( name ))
    {
        pkResult = pkE_INVALIDARG;
    }
    else
    {
        m_attributes[name] = value;
    }

    return (pkResult);
}
