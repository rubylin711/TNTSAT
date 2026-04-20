///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// <summary>
///     IManifestTrack.h
///     Provides interface expose the track object in the client manifest to the application
/// </summary>

#include <string>
#include <vector>

class IManifestStream;

/// <summary>
/// Auxiliary object that holds the codec private data of a Smooth Streaming track
/// </summary>
class CodecPrivateDataBuffer
{
public:

    /// <summary>
    /// Constructor
    /// </returns>
    CodecPrivateDataBuffer();

    /// <summary>
    /// Destructor
    /// </returns>
    ~CodecPrivateDataBuffer();

    /// <returns>
    /// A pointer to the array of bytes that contains the codec private data
    /// </returns>
    const uint8_t* data() const { return( _pbData ); }
          uint8_t* data()       { return( _pbData ); }

    /// <returns>
    /// The number of bytes in the codec private data
    /// </returns>
    size_t size() const         { return( _cbData ); }

    /// <summary>
    /// Sets the codec private data by parsing an hexadecimal string
    /// </summary>
    pkRESULT ParseFromHexStr(
                _In_count_(cch) const wchar_t* pch,
                _In_            size_t cch,
                _In_            size_t offset );

    /// <summary>
    /// Resizes the buffer content
    /// </summary>
    pkRESULT resize( _In_ size_t cbSize );

    /// <summary>
    /// Compares to another instance
    /// </summary>
    bool operator == ( _In_ const CodecPrivateDataBuffer& that ) const;

protected:

    CodecPrivateDataBuffer( const CodecPrivateDataBuffer& );
    void operator = ( const CodecPrivateDataBuffer& );

    uint8_t*    _pbData;
    size_t      _cbData;
};



/// <summary>
/// Interface to access data from a Smooth Streaming manifest track
/// </summary>
class IManifestTrack
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
    /// Get the track level attributes from the manifest
    /// </summary>
    /// <param name="name">The name of the attribute</param>
    /// <param name="pValue">The value of the attribute</param>
    /// <return> Indicates whether the attribute exists. </return>
    virtual bool GetAttribute(_In_ const std::wstring& name, _Out_ std::wstring* pValue) const = 0;
    /// <summary>
    /// Get the track level custom attributes from the manifest
    /// </summary>
    /// <param name="name">The name of the attribute</param>
    /// <param name="pValue">The value of the attribute</param>
    /// <return> Indicates whether the attribute exists. </return>
    virtual bool GetCustomAttributeValue(_In_ const std::wstring& name, _Out_ std::wstring* pValue) const = 0;
    /// <summary>
    /// Get the track level custom attributes names from the manifest
    /// </summary>
    /// <param name="name">List of names of the custom attributes</param>
    virtual void GetCustomAttributeNames(_Out_ std::vector<std::wstring>* pNames) const = 0;
    /// <summary>
    /// Get the stream object that owns this track
    /// </summary>
    /// <param name="ppStream">The stream that owns this track</param>
    virtual void GetStream(_Deref_out_ IManifestStream ** ppStream) = 0;
    /// <summary>
    /// Gets the index of the track in the manifest.
    /// </summary>
    /// <remarks>
    /// This value corresponds to the index of the QualityLevel element in the manifest from which
    /// the track object was instantiated. The value is constant for the duration of the object.
    /// In particular removing tracks through IManifestStream::RestrictTrack will not change
    /// the TrackIndex of the other tracks.
    /// </remarks>
    uint32_t TrackIndex() const { return( _trackIndex ); }
    /// <summary>
    /// Get the Bitrate
    /// </summary>
    uint32_t Bitrate() const { return( _nBitrate ); }
    /// <summary>
    /// Get the NominalBitrate
    /// </summary>
    uint32_t NominalBitrate() const { return( _nNominalBitrate ); }
    /// <summary>
    /// Get the MaxWidth
    /// </summary>
    uint32_t MaxWidth() const { return( _nMaxWidth ); }
    /// <summary>
    /// Get the MaxHeight
    /// </summary>
    uint32_t MaxHeight() const { return( _nMaxHeight ); }
    /// <summary>
    /// Get the HardwareProfile
    /// </summary>
    uint32_t HardwareProfile() const { return( _nHardwareProfile ); }
    /// <summary>
    /// Get the NALUnitLength
    /// </summary>
    uint32_t NALUnitLength() const { return( _nNALUnitLength ); }
    /// <summary>
    /// Get the AudioTag
    /// </summary>
    uint32_t AudioTag() const { return( _nAudioTag ); }
    /// <summary>
    /// Get the FourCC
    /// </summary>
    uint32_t FourCC() const { return( _nFourCC ); }
    /// <summary>
    /// Get the CodecPrivateData
    /// </summary>
    const CodecPrivateDataBuffer& CodecPrivateData() const { return( _codecPrivateData ); }

protected:
    // make destructor protected to avoid direct destroy of the object without calling Release()
    virtual ~IManifestTrack() {};

    // Protected attributes
    uint32_t _trackIndex;
    uint32_t _nBitrate;
    uint32_t _nNominalBitrate;
    uint32_t _nMaxWidth;
    uint32_t _nMaxHeight;
    uint32_t _nHardwareProfile;
    uint32_t _nNALUnitLength;
    uint32_t _nAudioTag;
    uint32_t _nFourCC;
    CodecPrivateDataBuffer _codecPrivateData;
};
