///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <IManifestTrack.h>
#include <AutoRefPtr.h>
#include <map>

#include <RefCountObj.h>

////////////////////////////////////////////////////////////////////////////////
//
// CManifestTrack - basic IManifestTrack implementation
//
////////////////////////////////////////////////////////////////////////////////

class CManifestTrack
    : public IManifestTrack
{
public:

    CManifestTrack();

    //
    // Overloads the attribute accessors to allow writing
    //
    uint32& TrackIndex() { return( _trackIndex ); }
    uint32& Bitrate() { return( _nBitrate ); }
    uint32& NominalBitrate() { return( _nNominalBitrate ); }
    uint32& MaxWidth() { return( _nMaxWidth ); }
    uint32& MaxHeight() { return( _nMaxHeight ); }
    uint32& HardwareProfile() { return( _nHardwareProfile ); }
    uint32& NALUnitLength() { return( _nNALUnitLength ); }
    uint32& AudioTag() { return( _nAudioTag ); }
    uint32& FourCC() { return( _nFourCC ); }
    CodecPrivateDataBuffer& CodecPrivateData() { return( _codecPrivateData ); }

    //
    // Attributes accessors
    //
    int& StreamType()               { return( m_StreamType ); }
    uint32& OriginalIndex()         { return( m_iOriginal ); }
    uint32 OriginalIndex() const    { return( m_iOriginal ); }
    bool& IsSelected()              { return( m_fIsSelected ); }

    //
    // Implementation
    //

    pkRESULT SetAttribute(
            _In_ const std::wstring& name,
            _In_ const std::wstring& value );

    void SetCustomAttribute(
            _In_ const wchar_t* pszName,
            _In_ const wchar_t* pszValue );

    void SetStreamConnection(
            _In_ IElementToContainerConnection<IManifestStream>* pStreamConnection )
    {
        m_apStreamConnection.Set( pStreamConnection );
    }

    //
    // IManifestTrack - implementation
    //

    __override
    bool GetAttribute(
            _In_ const std::wstring& name,
            _Out_ std::wstring* pValue ) const;

    __override
    bool GetCustomAttributeValue(
            _In_ const std::wstring& name,
            _Out_ std::wstring* pValue ) const;

    __override
    void GetCustomAttributeNames(
            _Out_ std::vector<std::wstring>* pNames) const;

    __override
    void GetStream(
        _Deref_out_ IManifestStream ** ppStream );


protected:

    typedef std::map<std::wstring,std::wstring> CAttributeMap;

    CAttributeMap m_attributes;
    CAttributeMap m_customAttributes;
    int m_StreamType;
    uint32 m_iOriginal;
    bool m_fIsSelected;
    AutoRefPtr< IElementToContainerConnection<IManifestStream> > m_apStreamConnection;
};

////////////////////////////////////////////////////////////////////////////////
//
// Creates the default CManifestTrack
//
////////////////////////////////////////////////////////////////////////////////

pkRESULT
CreateManifestTrack(
    _Out_ CManifestTrack** ppManifestTrack );

////////////////////////////////////////////////////////////////////////////////
//
// CManifestTrack utilities
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void SortManifestTracksInAscendingBitrateOrder(
    _Inout_ std::vector< AutoRefPtr<CManifestTrack> >* pTracks );


