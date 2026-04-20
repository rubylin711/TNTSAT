///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <pkExecutive.h>
#include <StringUtils.h>
#include <assert.h>

#include "MbrLocalInterfaces.h"

static const size_t MAX_SIZE_UINT64_TO_CHAR = 32;
static const size_t MAX_SIZE_UINT32_TO_CHAR = 16;
////////////////////////////////////////////////////////////////////////////////
//
// CManifestUrlServices - default implementation of IManifestUrlServices
//
////////////////////////////////////////////////////////////////////////////////

class CManifestUrlServices
    : public IManifestUrlServices
{
public:

    CManifestUrlServices()
    {
        m_isSegmented = false;
    }

    //
    // IManifestUrlServices
    //

    __override
    pkRESULT FormatURL(
                _In_ const wchar_t* pszBaseUrl,
                _In_ IManifestTrack* pTrack,
                _In_ uint32 dwChunkIndex,
                _In_ uint32 dwHardwareProfile,
                _In_ uint64 chunkStartTime,
                _Out_ std::wstring* pStrURL );

    __override
    pkRESULT FormatSegmentManifestURL(
                        _In_ int64 chunkStartTime,
                        _Out_ std::wstring* pOutUrl );

private:

    uint64 CalculateSegmentStartTime( _In_ uint64 chunkStartTime );
    std::wstring GetManifestBaseUrl();
    std::wstring FormatCustomAttributes( _In_ IManifestTrack* pTrack );
};

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestUrlServices::FormatURL(
    _In_ const wchar_t* pszBaseUrl,
    _In_ IManifestTrack* pTrack,
    _In_ uint32 dwChunkIndex,
    _In_ uint32 dwHardwareProfile,
    _In_ uint64 chunkStartTime,
    _Out_ std::wstring* pOutUrl )
{
    pkRESULT pkResult = pkS_OK;

    static const wchar_t c_wczAttributesHardwareProfileTag[] = L"HardwareProfile=%u";

    WCHAR wczBitrate[MAX_SIZE_UINT32_TO_CHAR];
    WCHAR wczKBitrate[MAX_SIZE_UINT32_TO_CHAR];
    WCHAR wczHardwareProfile[MAX_SIZE_UINT32_TO_CHAR];
    WCHAR wczAttributesHardwareProfile[MAX_SIZE_UINT32_TO_CHAR + sizeof(c_wczAttributesHardwareProfileTag)/sizeof(c_wczAttributesHardwareProfileTag[0])];
    WCHAR wczIndex[MAX_SIZE_UINT32_TO_CHAR];
    WCHAR wczChunkStartTime[MAX_SIZE_UINT64_TO_CHAR]; // start time in unit defined by timescale in the manifest

    StringCbPrintfW( wczBitrate, sizeof(wczBitrate), L"%d", pTrack->Bitrate());
    StringCbPrintfW( wczKBitrate,sizeof(wczKBitrate), L"%d", pTrack->Bitrate()/1000);
    StringCbPrintfW( wczIndex, sizeof(wczIndex), L"%d", dwChunkIndex );
    StringCbPrintfW( wczHardwareProfile, sizeof(wczHardwareProfile), L"%d", dwHardwareProfile );
    StringCbPrintfW( wczAttributesHardwareProfile, sizeof(wczAttributesHardwareProfile), c_wczAttributesHardwareProfileTag, dwHardwareProfile );
    StringCbPrintfW( wczChunkStartTime, sizeof(wczChunkStartTime), L"%lld", chunkStartTime );

    *pOutUrl = GetManifestBaseUrl() + pszBaseUrl;

    if(m_isSegmented)
    {
        uint64 segmentStartTime = CalculateSegmentStartTime( chunkStartTime );
        WCHAR wczSegStartTime[MAX_SIZE_UINT64_TO_CHAR];
        StringCbPrintfW(wczSegStartTime, sizeof(wczSegStartTime), L"%lld", segmentStartTime);

        StrReplaceInPlace( pOutUrl, L"{starttime}", wczSegStartTime);
    }

    // Cff2 & Cff3
    // replace {2} with hardware profile, {1} with bitrate and {0} with index
    StrReplaceInPlace( pOutUrl, L"{0}", wczIndex );
    StrReplaceInPlace( pOutUrl, L"{1}", wczKBitrate );
    StrReplaceInPlace( pOutUrl, L"{2}", wczHardwareProfile );
    // Cff3
    StrReplaceInPlace( pOutUrl, L"{Index}", wczIndex );
    StrReplaceInPlace( pOutUrl, L"{Bitrate}", wczBitrate );
    StrReplaceInPlace( pOutUrl, L"{HardwareProfile}", wczHardwareProfile );
    StrReplaceInPlace( pOutUrl, L"{attributes}", wczAttributesHardwareProfile );
    // manifest 1.x
    StrReplaceInPlace( pOutUrl, L"{bitrate}", wczBitrate );
    StrReplaceInPlace( pOutUrl, L"{start time}", wczChunkStartTime );
    StrReplaceInPlace( pOutUrl, L"{start_time}", wczChunkStartTime );

    StrReplaceInPlace( pOutUrl, L",{CustomAttributes}", FormatCustomAttributes(pTrack).c_str());

    return( pkResult );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestUrlServices::FormatSegmentManifestURL(
    _In_ int64 chunkStartTime,
    _Out_ std::wstring* pOutUrl )
{
    pkRESULT pkResult = pkS_OK;

    *pOutUrl = GetManifestBaseUrl() + m_strSegmentUrlTemplate;

    ASSERT(m_isSegmented);
    uint64 segmentStartTime = CalculateSegmentStartTime( chunkStartTime );
    WCHAR wczSegStartTime[MAX_SIZE_UINT64_TO_CHAR];
    StringCbPrintfW(wczSegStartTime, sizeof(wczSegStartTime), L"%lld", segmentStartTime);

    StrReplaceInPlace( pOutUrl, L"{starttime}", wczSegStartTime);

    return( pkResult );
}

/////////////////////////////////////////////////////////////////////////
std::wstring CManifestUrlServices::GetManifestBaseUrl()
{
    std::wstring baseUrl = m_strManifestUrl;

    // remove the query string
    std::wstring::size_type pos = baseUrl.find_first_of(L'?');
    if (pos != std::wstring::npos)
    {
        baseUrl.resize( pos );
    }

    // search for last instance of '/'
    pos = baseUrl.find_last_of(L'/');

    if (pos != std::wstring::npos)
    {
        baseUrl.resize( pos + 1 );
    }

    return baseUrl;
}

/////////////////////////////////////////////////////////////////////////
uint64 CManifestUrlServices::CalculateSegmentStartTime( _In_ uint64 chunkStartTime )
{
    uint64 segmentStartTime = 0;

    if( m_segmentDuration > 0)
    {
        uint64 offset = m_referenceSegmentStartTime % m_segmentDuration;

        segmentStartTime = chunkStartTime - ( chunkStartTime - offset ) % m_segmentDuration;
    }

    return segmentStartTime;
}

/////////////////////////////////////////////////////////////////////////
std::wstring CManifestUrlServices::FormatCustomAttributes( _In_ IManifestTrack* pTrack )
{
    std::wstring wczCustomAttibutes;
    std::vector<std::wstring> customAttributeNames;

    // get a list of the custom attribute names
    pTrack->GetCustomAttributeNames(&customAttributeNames);

    // get the attribute values and format the string
    for(size_t i = 0; i < customAttributeNames.size(); i++)
    {
        std::wstring value;

        if( !pTrack->GetCustomAttributeValue(customAttributeNames[i], &value) )
        {
            ASSERT( false );
        }

        wczCustomAttibutes += L",";
        wczCustomAttibutes += customAttributeNames[i];
        wczCustomAttibutes += L"=";
        wczCustomAttibutes += value;
    }

    return wczCustomAttibutes;
}

/// <summary>
/// Instantiates the default IManifestUrlServices
/// <summary>

pkRESULT DefaultManifestUrlServices::CreateInstance( _Out_ IManifestUrlServices** ppObj )
{
    *ppObj = NEW_NO_THROW CRefCountedObj<CManifestUrlServices>();
    return( ( *ppObj != NULL ) ? pkS_OK : pkE_OUTOFMEMORY );
}
