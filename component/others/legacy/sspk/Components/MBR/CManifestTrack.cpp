///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <pkExecutive.h>

#include "CManifestTrack.h"

#include <AutoRefPtr.h>

#include <algorithm>

#include "StringUtils.h"

////////////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////////////

const uint32 DEFAULT_NAL_UNIT_LENGTH = 4;


////////////////////////////////////////////////////////////////////////////////
//
// CManifestTrack
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
CManifestTrack::CManifestTrack()
    : m_StreamType( 0 )
    , m_iOriginal( 0 )
    , m_fIsSelected( true )
{
    _trackIndex = 0;
    _nBitrate = 0;
    _nNominalBitrate = 0;
    _nMaxWidth = 0;
    _nMaxHeight = 0;
    _nHardwareProfile = 0;
    _nNALUnitLength = DEFAULT_NAL_UNIT_LENGTH;
    _nAudioTag = 0;
    _nFourCC = 0;
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestTrack::SetAttribute(
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

////////////////////////////////////////////////////////////////////////////////
void CManifestTrack::SetCustomAttribute(
    _In_ const wchar_t* pszName,
    _In_ const wchar_t* pszValue )
{
    m_customAttributes[ pszName ] = pszValue;
}

////////////////////////////////////////////////////////////////////////////////
bool CManifestTrack::GetAttribute(
    _In_ const std::wstring& name,
    _Out_ std::wstring* pValue ) const
{
    bool fSuccess = false;
    pkASSERT(NULL != pValue);
    pValue->clear();

    CAttributeMap::const_iterator it = m_attributes.find( name );

    if (it != m_attributes.end())
    {
        *pValue = it->second;
        fSuccess = true;
    }

    return (fSuccess);
}

////////////////////////////////////////////////////////////////////////////////
bool CManifestTrack::GetCustomAttributeValue(
    _In_ const std::wstring& name,
    _Out_ std::wstring* pValue) const
{
    CAttributeMap::const_iterator it = m_customAttributes.find( name );

    if( it != m_customAttributes.end() )
    {
        *pValue = it->second;
        return( true );
    }
    else
    {
        pValue->clear();
        return( false );
    }
}

////////////////////////////////////////////////////////////////////////////////
void CManifestTrack::GetCustomAttributeNames(
    _Out_ std::vector<std::wstring>* pNames) const
{
    pNames->clear();
    pNames->reserve(m_customAttributes.size());
    for ( CAttributeMap::const_iterator it = m_customAttributes.begin(); it != m_customAttributes.end(); it++ )
    {
        pNames->push_back(it->first);
    }
}

////////////////////////////////////////////////////////////////////////////////
void CManifestTrack::GetStream(
    _Deref_out_ IManifestStream ** ppStream )
{
    m_apStreamConnection->GetContainer( ppStream );
}

////////////////////////////////////////////////////////////////////////////////
//
// CodecPrivateDataBuffer
//
////////////////////////////////////////////////////////////////////////////////

CodecPrivateDataBuffer::CodecPrivateDataBuffer()
    : _pbData( NULL )
    , _cbData( 0 )
{
}

CodecPrivateDataBuffer::~CodecPrivateDataBuffer()
{
    delete [] _pbData;
}

pkRESULT CodecPrivateDataBuffer::ParseFromHexStr(
    _In_count_(cch) const wchar_t* pch,
    _In_            size_t cch,
    _In_            size_t offset )
{
    pkRESULT pkr = pkS_OK;

    size_t cbTotalSize = offset + cch / 2;

    pkr = resize( cbTotalSize );
    if( pkFAILED(pkr) )
    {
        goto exit;
    }

    pkr = HexStrToBytes( pch, cch, data() + offset, cch / 2 );
    if( pkFAILED(pkr) )
    {
        goto exit;
    }

exit:

    return( pkr );
}

pkRESULT CodecPrivateDataBuffer::resize( _In_ size_t cbNew )
{
    pkRESULT pkr = pkS_OK;

    if( cbNew > _cbData )
    {
        uint8_t* pbNew = NEW_NO_THROW uint8_t[ cbNew ];
        if( pbNew == NULL )
        {
            pkr = pkE_OUTOFMEMORY;
            goto exit;
        }

        if( _pbData != NULL )
        {
            memcpy( pbNew, _pbData, _cbData );
            delete [] _pbData;
        }

        _pbData = pbNew;
    }

    _cbData = cbNew;

exit:

    return( pkr );
}

bool CodecPrivateDataBuffer::operator == ( _In_ const CodecPrivateDataBuffer& that ) const
{
    return( ( _cbData == that._cbData ) && memcmp( _pbData, that._pbData, _cbData ) );
}

////////////////////////////////////////////////////////////////////////////////
//
// Creates the default CManifestTrack
//
////////////////////////////////////////////////////////////////////////////////

pkRESULT
CreateManifestTrack(
    _Out_ CManifestTrack** ppManifestTrack )
{
    *ppManifestTrack = NEW_NO_THROW CRefCountedObj<CManifestTrack>();

    return (NULL == (*ppManifestTrack)) ? pkE_OUTOFMEMORY : pkS_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
// CManifestTrack utilities
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
static bool TrackBitratesAreInAscendingOrder(
    _In_ CManifestTrack* ptrkFirst,
    _In_ CManifestTrack* ptrkSecond )
{
    return( ptrkFirst->Bitrate() < ptrkSecond->Bitrate() );
}

////////////////////////////////////////////////////////////////////////////////
void SortManifestTracksInAscendingBitrateOrder(
    _Inout_ std::vector< AutoRefPtr<CManifestTrack> >* pTracks )
{
    //
    // Move the references to a temporary vector of normal pointers
    // (i.e., no ref-counted) to minimize the number of addref/releases
    // during sorting
    //

    std::vector< CManifestTrack* > sortedTracks( pTracks->size() );

    for( size_t i = 0; i < pTracks->size(); ++i )
    {
        sortedTracks[i] = (*pTracks)[i].HandOffRef();
    }

    //
    // Sort the temporary array
    //

    std::sort( sortedTracks.begin(), sortedTracks.end(), TrackBitratesAreInAscendingOrder );

    //
    // Move the references back to original array
    //

    for( size_t i = 0; i < pTracks->size(); ++i )
    {
        (*pTracks)[i].AdoptRef( sortedTracks[i] );
    }
}

