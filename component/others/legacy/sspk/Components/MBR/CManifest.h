///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include <IManifest.h>

////////////////////////////////////////////////////////////////////////////////
//
// CManifest - basic IManifest implementation
//
////////////////////////////////////////////////////////////////////////////////

class CManifest : public IManifest
{
protected:

    CManifest();

public:

    //
    // Overloads the attribute accessors to allow writing
    //
    uint32& MajorVersion() { return (m_dwMajorVersion); };
    uint32& MinorVersion() { return (m_dwMinorVersion); };
    int64& TimeScale() { return (m_llTimeScale); };
    int64& Duration() { return (m_llDuration); };
    bool& IsLive() { return (m_fIsLive); };
    uint32& LookAheadCount() { return (m_dwLookAheadCount); };
    int64& DVRWindowLength() { return (m_llDVRWindowLength); };

    //
    // IManifest - implementation
    //

    virtual void AddRef();

    virtual void Release();

    /// <summary>
    /// Get the top level attributes from the manifest
    /// </summary>
    /// <param name="name">The name of the attribute</param>
    /// <param name="pValue">The value of the attribute</param>
    /// <return> Indicates whether the attribute exists. </return>
    bool GetAttribute(_In_ const std::wstring& name, _Out_ std::wstring* pValue) const;

    /// <summary>
    /// Get the available streams in the current mainfest
    /// </summary>
    /// <param name="pAvailableStreams">The list of available streams in the manifest</param>
    virtual pkRESULT GetAvailableStreams(_Out_ std::vector< AutoRefPtr<IManifestStream> >* pAvailableStreams) = 0;

    /// <summary>
    /// Get the selected streams in the current mainfest
    /// </summary>
    /// <param name="pSelectedStreams">The list of selected streams in the manifest</param>
    virtual pkRESULT GetSelectedStreams(_Out_ std::vector< AutoRefPtr<IManifestStream> >* pSelectedStreams) = 0;

    /// <summary>
    /// Select the streams in the current manifest
    /// </summary>
    /// <param name="pCallback">The callback interface</param>
    /// <param name="selectedStreams">The flat list of streams that are selected</param>
    virtual pkRESULT SelectStreamsAsync(_In_ IStreamsSelectedCallback* pCallback, _In_ std::vector< AutoRefPtr<IManifestStream> >& selectedStreams) = 0;

    /// <summary>
    /// Indicates the type of manifest.  ManifestType_Segmented only supports ISmoothTransport::SetPlaybackRangeAsync.
    /// </summary>
    virtual EManifestType Type() const = 0;

    //
    // Other public methods
    //

    pkRESULT SetAttribute(_In_ const std::wstring& name, _In_ const std::wstring& value);

protected:

    int32 m_cRefs;
    std::map<std::wstring,std::wstring> m_attributes;
};
