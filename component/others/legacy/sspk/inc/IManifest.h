///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// <summary>
///     IManifest.h
///     Provides interface expose the client manifest object to the application
/// </summary>

#include "IManifestStream.h"

#include <string>
#include <vector>

struct StreamChangedEventArgs
{
    enum StreamChangedAction
    {
        StreamSelected,
        StreamDeselected
    };

    IManifestStream* pStream;                                               // The stream that is selected or de-selected
    StreamChangedAction Action;                                             // Indicats whether the stream is selected or de-selected
    int64_t Timestamp;                                                      // The timestamp of the action
    pkRESULT Result;                                                        // Indicate success or failure
};

struct StreamSelectedEventArgs
{
    std::vector<StreamChangedEventArgs> StreamChanges;                      // The aggregated stream selection and deselection events, which deselections at the front.
    pkRESULT Result;                                                        // Indicate success or failure
};

// Class receiving the events should implement this interface
class IStreamsSelectedCallback
{
public:
    /// <summary>
    /// The actual callback for the SelectStreams completed event
    /// </summary>
    /// <param name="pEventArgs">The event args for the SelectStream completed event</param>
    virtual void StreamSelectedCallback(_In_ StreamSelectedEventArgs* pEventArgs) = 0;

protected:
    virtual ~IStreamsSelectedCallback() {};
};

enum EManifestType
{
    ManifestType_Standard,
    ManifestType_Segmented
};

class IManifest
{
public:
    /// <summary>
    /// Increment the ref count
    /// </summary>
    virtual void AddRef() = 0;

    /// <summary>
    /// Decrement the ref count
    /// </summary>
    virtual void Release() = 0;

    /// <summary>
    /// Get the top level attributes from the manifest
    /// </summary>
    /// <param name="name">The name of the attribute</param>
    /// <param name="pValue">The value of the attribute</param>
    /// <return> Indicates whether the attribute exists. </return>
    virtual bool GetAttribute(_In_ const std::wstring& name, _Out_ std::wstring* pValue) const = 0;

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

    // Public properties
    uint32_t    MajorVersion() const        { return (m_dwMajorVersion); };
    uint32_t    MinorVersion() const        { return (m_dwMinorVersion); };
    int64_t     TimeScale() const           { return (m_llTimeScale); };
    int64_t     Duration() const            { return (m_llDuration); };         // actual dvr window size, in 10 MHz
    bool        IsLive() const              { return (m_fIsLive); };
    uint32_t    LookAheadCount() const      { return (m_dwLookAheadCount); };
    int64_t     DVRWindowLength() const     { return (m_llDVRWindowLength); };  // manifest dvr window size, in seconds
    uint64_t    StartTime() const           { return (m_ullStartTime); };       // in NTP units

protected:
    // make destructor protected to avoid direct destroy of the object without calling Release()
    virtual ~IManifest() {};

    // Internal back store of the properties
    uint32_t    m_dwMajorVersion;
    uint32_t    m_dwMinorVersion;
    int64_t     m_llTimeScale;
    int64_t     m_llDuration;
    bool        m_fIsLive;
    uint32_t    m_dwLookAheadCount;
    int64_t     m_llDVRWindowLength;
    uint64_t    m_ullStartTime;
};
