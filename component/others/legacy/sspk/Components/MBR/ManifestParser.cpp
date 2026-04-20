///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

// MSXML has a SAX reader, but with a different interfaces (ISAXXMLReader, ISAXContentHandler, ISAXErrorHandler, ISAXAttributes)
// rather than the simple callback interface here.  If we want to "leverage" MSXML, the code here needs to be changed to use the
// above interfaces.

#include "stdafx.h"

#include <StringUtils.h>
#include <Base64.h>

#include "ManifestParser.h"
#include "StreamTypeTraits.h"
#include "CMbrConfiguration.h"

using namespace MBR;
using namespace std;

// #define MANIFESTPARSER_SPEW
#ifdef MANIFESTPARSER_SPEW
#define MANIFESTPARSER_TRACE(x) TRACE(x)
#else
#define MANIFESTPARSER_TRACE(x)
#endif

// #define DEBUG_INVALID_MANIFEST_DATA
#ifdef DEBUG_INVALID_MANIFEST_DATA

#define MANIFEST_DEBUG_VALIDATE(x) ASSERT(false);
#define CHECKBOOL_GOTO( val, label ) if ( !val ) { ASSERT(!#val); pkResult = pkE_UNSUPPORTED_FORMAT; goto label; }
#define SET_PKRESULT_GOTO( val, label ) { pkResult = val; ASSERT(false); goto label; }

#else // DEBUG_INVALID_MANIFEST_DATA

//#define MANIFEST_DEBUG_VALIDATE(x) if ( x ) { TRACE((x)); }
#define MANIFEST_DEBUG_VALIDATE(x)  
#define CHECKBOOL_GOTO( val, label ) if ( !val ) { pkResult = pkE_UNSUPPORTED_FORMAT; goto label; }
#define SET_PKRESULT_GOTO( val, label ) { pkResult = val; goto label; }

#endif // DEBUG_INVALID_MANIFEST_DATA


#define TP_SCOPE_TRACE TP_LOWEST
#define DH_THIS_FILE DH_MEDIASOURCE_CHUNK

// NOTE: the following must be upper case for s_isAttributeValue to work
#define WMDRM_GUID L"4FB3BCCD-2CF9-495E-AF9E-ECD2E5DD6D31"
#define PLAYREADY_GUID L"9A04F079-9840-4286-AB92-E65BE0885F95"

#ifdef MANIFESTPARSER_UNSUPPORTED_ELEMENTS
static const wchar_t* const unsupportedElements[] = {
                                                      L"Segment",
                                                    };
#endif //MANIFESTPARSER_UNSUPPORTED_ELEMENTS

/////////////////////////////////////////////////////////////////////////
// Helpers

static bool s_isAttributeValue( const wstring& wsAttrValue, const WCHAR* wszUpperCheckValue, unsigned int cchCheckValueSize)
{
    bool isMatch = wsAttrValue.length() == cchCheckValueSize;
    if (isMatch)
    {
        const WCHAR* wszAttr = wsAttrValue.c_str();

        for (unsigned int i = 0; isMatch && (i < cchCheckValueSize); i++)
        {
            isMatch = ( (WCHAR)towupper(wszAttr[i]) == wszUpperCheckValue[i] );
            if (0 == wszUpperCheckValue[i])
            {
                break;
            }
        }
    }
    return isMatch;
}

/////////////////////////////////////////////////////////////////////////
// CManifestParsingCallback

// Note:
//    This class is declared a friend within CMediaStreamDescription which is
//    in the MBR namespace and therefore must be within the MBR namespace here.
//
namespace MBR
{

// CManifestParsingCallback only needs to be seen by CXmlParser, so we can hide declaration
class CManifestParsingCallback : public ISAXCallback
{
public:
    enum ObjectLoaderType
    {
        OLT_NONE = 0,
        OLT_EnterMediaIndex,
        OLT_EnterSegment,
        OLT_EnterStreamIndex,
        OLT_EnterQualityLevel,
        OLT_EnterCustomAttributes,
        OLT_EnterCustomAttributeData,
        OLT_EnterChunkMetadata,
        OLT_EnterFragmentMetadata,
        OLT_EnterProtection,
        OLT_EnterProtectionHeader,
        OLT_Ignore,
    };

    CManifestParsingCallback(CChunkManifest* pChunkManifest, EManifestMode mode);

    ~CManifestParsingCallback() {}

    __override pkRESULT  StartDocument() { return pkS_OK; };
    __override pkRESULT  EndDocument() { return pkS_OK; };

    __override pkRESULT  ElementBegin( const WCHAR* strName, uint32 NameLen, const XMLAttribute* pAttributes, uint32 NumAttributes );

    __override pkRESULT  ElementContent(
                            _In_count_(cchData) const WCHAR *pszData,
                            _In_ uint32 cchData,
                            bool fMore );

    __override pkRESULT  ElementEnd( const WCHAR* strName, uint32 NameLen );

    __override pkRESULT  CDATABegin( )  { return pkS_OK; };
    __override pkRESULT  CDATAData( const WCHAR* strCDATA, uint32 CDATALen, bool bMore ){ return pkS_OK; };
    __override pkRESULT  CDATAEnd( ){ return pkS_OK; };

    __override pkRESULT  Error( pkRESULT hError, const char *strMessage, ...  );

private:
 
    bool                        GetAttrByName( const WCHAR* strName, const XMLAttribute* pAttributes, uint32 cAttributes, wstring& wstrVal );

    pkRESULT                    ParseSmoothStreamingMediaElement( _In_ const WCHAR* strName,
                                                                  _In_ uint32 NameLen, 
                                                                  _In_ const XMLAttribute *pAttributes, 
                                                                  _In_ uint32 NumAttributes ); 

    pkRESULT                    ParseDrmSchemeElement( _In_ const WCHAR* strName,
                                                       _In_ uint32 NameLen, 
                                                       _In_ const XMLAttribute *pAttributes, 
                                                       _In_ uint32 NumAttributes );

    pkRESULT                    ParseProtectionHeaderElement( _In_ const WCHAR* strName,
                                                              _In_ uint32 NameLen, 
                                                              _In_ const XMLAttribute *pAttributes, 
                                                              _In_ uint32 NumAttributes ); 

    pkRESULT                    ParseSegmentElement( _In_ const WCHAR* strName,
                                                     _In_ uint32 NameLen, 
                                                     _In_ const XMLAttribute *pAttributes, 
                                                     _In_ uint32 NumAttributes );

    pkRESULT                    ParseStreamIndexElement( _In_ const WCHAR* strName,
                                                         _In_ uint32 NameLen, 
                                                         _In_ const XMLAttribute *pAttributes, 
                                                         _In_ uint32 NumAttributes );

    pkRESULT                    ParseQualityLevelElement( _In_ const WCHAR* strName,
                                                          _In_ uint32 NameLen, 
                                                          _In_ const XMLAttribute *pAttributes, 
                                                          _In_ uint32 NumAttributes );

    pkRESULT                    ParseAttributeElement( _In_ const WCHAR* strName,
                                                       _In_ uint32 NameLen, 
                                                       _In_ const XMLAttribute *pAttributes, 
                                                       _In_ uint32 NumAttributes ); 

    pkRESULT                    ParseCElement( _In_ const WCHAR* strName,
                                               _In_ uint32 NameLen, 
                                               _In_ const XMLAttribute *pAttributes, 
                                               _In_ uint32 NumAttributes );

    pkRESULT                    ParseFElement( _In_ const WCHAR* strName,
                                               _In_ uint32 NameLen, 
                                               _In_ const XMLAttribute *pAttributes, 
                                               _In_ uint32 NumAttributes );

    pkRESULT                    FindStream( _In_ MediaStreamType type, _In_ const wstring& name );
    bool                        TrackExist( _In_ CMediaStreamDescription* pStreamInfo, _In_ uint32_t trackIndex );
    SFragmentMetadata*          FindFragmentMetadataInChunkInfoVector(_In_ CManifestTrack* pTrack);

    template< class storeT >
    void                        StoreAttributes( _In_count_(NumAttributes) const XMLAttribute *pAttributes, _In_ uint32 NumAttributes, _Out_ storeT* pStore );

    pkRESULT                    ElementContent_FragmentMetadata( _In_count_(cchData) const WCHAR* pszData, _In_ size_t cchData, bool fMore );
    void                        IgnoreElement();

    ObjectLoaderType            m_State;
    bool                        m_isNewStream;
    CChunkManifest*             m_pChunkManifest;
    CMediaStreamDescription*    m_pCurrentStreamInfo;
    CManifestTrack*             m_pCurrentTrack;
    uint32                      m_dwCurrentChunk;
    uint32                      m_dwManifestVersion;
    bool                        m_fInitializeChunks;
    uint32                      m_dwCurrentMbrIdx;
    uint32                      m_dwBufferTime;
    uint32                      m_dwLastChunkDuration;

    wstring                     m_wstrFourCC;
    wstring                     m_wstrPlayReadyBlob;
    wstring                     m_wstrValue;
    wstring                     m_wstrAttrName;
    wstring                     m_wstrAttrValue;
    wstring                     m_wstrFragmentMetadata;

    ObjectLoaderType            m_previousState;
    int32                       m_IgnoredLevel;
    EManifestMode               m_mode;
    std::vector<SChunkInfo>     m_ChunkInfo;
};

CManifestParsingCallback::CManifestParsingCallback( CChunkManifest * pChunkManifest, EManifestMode mode)
    : m_State( OLT_NONE )
    , m_isNewStream(false)
    , m_pChunkManifest( pChunkManifest ) // weak ref
    , m_pCurrentStreamInfo( NULL )
    , m_pCurrentTrack( NULL )
    , m_dwCurrentChunk( MBR_INVALID_INDEX )
    , m_dwManifestVersion( 0 )
    , m_fInitializeChunks( false )
    , m_dwCurrentMbrIdx( MBR_INVALID_INDEX )
    , m_dwBufferTime(0)
    , m_dwLastChunkDuration( 0 )
    , m_previousState( OLT_NONE )
    , m_IgnoredLevel( 0 )
    , m_mode( mode )
{
}

bool CManifestParsingCallback::GetAttrByName(
    const WCHAR* strName,            // __in_z
    const XMLAttribute* pAttributes, // _in_ecount(cAttributes)
    uint32 cAttributes,
    wstring& wstrVal
    )
{
    uint32 cName = wcslen(strName); // Not good to pass as a parameter because it's usually a constant string there

    for ( uint32 i = 0; i < cAttributes; i++ )
    {
        if ( (cName == pAttributes[i].NameLen) && (wmemcmp(strName,pAttributes[i].strName,cName) == 0) )
        {
            wstrVal.assign(pAttributes[i].strValue, pAttributes[i].ValueLen);
            return true;
        }
    }

    wstrVal = L"";
    return false;
}

/////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::Error(
                                        pkRESULT hError,
                                        const char *strMessage, ...  )
{
    TRACE_ERROR(("Error 0x%x: %s (line:%u; col:%u)", hError, strMessage, GetLineNumber(), GetLinePosition()));
    return pkS_OK;
};

// Macro for perf-sensitive name comparison, avoiding extra string copy
// Assumes strName is not NULL-terminated and that x is a static string array (i.e. L"xxxxx")
// Will always return false if passed a pointer because sizes won't match
#define IS_ELEMENT_NAME(x) ((NameLen == (sizeof(x)/sizeof(x[0]))-1) && wcsncmp(strName, x, NameLen) == 0)

// NOTE: the upprChkVal must be all upper-case
#define IS_ATTRIBUTE_VALUE(attrValue, upprChkVal) (s_isAttributeValue(attrValue, upprChkVal, sizeof(upprChkVal)/sizeof(upprChkVal[0])-1))

/////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::ElementBegin(
    const WCHAR* strName,
    uint32 NameLen,
    const XMLAttribute *pAttributes,
    uint32 NumAttributes )
{
    pkRESULT pkResult = pkS_OK;

#ifdef MANIFESTPARSER_SPEW
    if ( !IS_ELEMENT_NAME(L"c") )
    {
        wstring wstrName;
        wstrName.assign(strName, NameLen);

        MANIFESTPARSER_TRACE(("ElementBegin[%d]: '%ls' NumAttributes %d", m_State, wstrName.c_str(), NumAttributes));
        for (uint32 i = 0; i < NumAttributes; i++)
        {
            m_wstrAttrName.assign(pAttributes[i].strName, pAttributes[i].NameLen);
            m_wstrAttrValue.assign(pAttributes[i].strValue, pAttributes[i].ValueLen);
            MANIFESTPARSER_TRACE(("%.20ls = %ls", m_wstrAttrName.c_str(), m_wstrAttrValue.c_str()));
        }
    }
#endif

    if ( OLT_NONE == m_State && ( IS_ELEMENT_NAME(L"SmoothStreamingMedia") || IS_ELEMENT_NAME(L"MediaIndex") ) )
    {
        CHECK_PKRESULT_GOTO( ParseSmoothStreamingMediaElement(strName, NameLen, pAttributes, NumAttributes), done );
    }
    else if ( OLT_EnterMediaIndex == m_State && IS_ELEMENT_NAME(L"DrmScheme") )
    {
        if( eManifestMode_Subsequent == m_mode )
        {
            IgnoreElement();
            goto done;
        }

        CHECK_PKRESULT_GOTO( ParseDrmSchemeElement(strName, NameLen, pAttributes, NumAttributes), done );
    }
    else if ( OLT_EnterMediaIndex == m_State && IS_ELEMENT_NAME(L"Protection") )
    {
        if( eManifestMode_Subsequent == m_mode )
        {
            IgnoreElement();
            goto done;
        }

        m_State = OLT_EnterProtection;
    }
    else if ( OLT_EnterProtection == m_State && IS_ELEMENT_NAME(L"ProtectionHeader") )
    {
        // in subsequent mode, the parser should never reach here since its parent element is ignored
        ASSERT( eManifestMode_Subsequent != m_mode );
        CHECK_PKRESULT_GOTO( ParseProtectionHeaderElement(strName, NameLen, pAttributes, NumAttributes), done );
    }
    else if (OLT_EnterMediaIndex == m_State && IS_ELEMENT_NAME(L"Segment") )
    {
        CHECK_PKRESULT_GOTO( ParseSegmentElement(strName, NameLen, pAttributes, NumAttributes), done );
    }
    else if ((OLT_EnterMediaIndex == m_State || OLT_EnterSegment == m_State) && IS_ELEMENT_NAME(L"StreamIndex") )
    {
        CHECK_PKRESULT_GOTO( ParseStreamIndexElement(strName, NameLen, pAttributes, NumAttributes), done );
    }
    else if (OLT_EnterStreamIndex == m_State
        && ((m_dwManifestVersion == 0) ? IS_ELEMENT_NAME(L"Bitrate") : IS_ELEMENT_NAME(L"QualityLevel")))
    {
        if( eManifestMode_Subsequent == m_mode )
        {
            IgnoreElement();
            goto done;
        }

        CHECK_PKRESULT_GOTO( ParseQualityLevelElement(strName, NameLen, pAttributes, NumAttributes), done );
    }
    else if (OLT_EnterQualityLevel == m_State && IS_ELEMENT_NAME(L"CustomAttributes"))
    {
        // in subsequent mode, the parser should never reach here since its parent element is ignored
        ASSERT( eManifestMode_Subsequent != m_mode );
        if ( m_dwManifestVersion < 2 )
        {
            SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done ); // CustomAttributes is not supported in versions less than 2
        }

        m_State = OLT_EnterCustomAttributes;
    }
    else if (OLT_EnterCustomAttributes == m_State && m_dwCurrentMbrIdx != MBR_INVALID_INDEX && IS_ELEMENT_NAME(L"Attribute") )
    {
        // in subsequent mode, the parser should never reach here since its parent element is ignored
        ASSERT( eManifestMode_Subsequent != m_mode );
        CHECK_PKRESULT_GOTO( ParseAttributeElement(strName, NameLen, pAttributes, NumAttributes), done );
    }
    else if (OLT_EnterStreamIndex == m_State && IS_ELEMENT_NAME(L"c") )
    {
        CHECK_PKRESULT_GOTO( ParseCElement(strName, NameLen, pAttributes, NumAttributes), done );
    }
    else if (OLT_EnterChunkMetadata == m_State && IS_ELEMENT_NAME(L"f") )
    {
        CHECK_PKRESULT_GOTO( ParseFElement(strName, NameLen, pAttributes, NumAttributes), done );
    }
    else
    {
        if( OLT_Ignore == m_State )
        {
            m_IgnoredLevel++;

            // check for overflow
            if(m_IgnoredLevel < 0)
            {
                TRACE_ERROR(("Overflow of Ignored Element"));
                SET_PKRESULT_GOTO(pkE_UNSUPPORTED_FORMAT, done);
            }
        }
        else
        {
            bool isUnsupportedElement = false;

#ifdef MANIFESTPARSER_UNSUPPORTED_ELEMENTS
            for( int i=0; i < sizeof(unsupportedElements)/sizeof(unsupportedElements[0]); i++)
            {
                if( wstring(strName, NameLen) == unsupportedElements[i] )
                {
                    isUnsupportedElement = true;
                    break;
                }
            }

#endif //MANIFESTPARSER_UNSUPPORTED_ELEMENTS
            if( isUnsupportedElement )
            {
                TRACE_ERROR(("unsupported ElementBegin[%d]: '%ls' NumAttributes %d",
                    m_State, wstring(strName, NameLen).c_str(), NumAttributes));
                SET_PKRESULT_GOTO(pkE_UNSUPPORTED_FORMAT, done);
            }
            else
            {
                TRACE_ERROR(("ignoring ElementBegin[%d]: '%ls' NumAttributes %d",
                    m_State, wstring(strName, NameLen).c_str(), NumAttributes));
                IgnoreElement();
            }
        }
    }
done:
    return pkResult;
};

void CManifestParsingCallback::IgnoreElement()
{
    m_previousState = m_State;
    m_State = OLT_Ignore;
    m_IgnoredLevel = 1;
}

/////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::ElementContent(
    _In_count_(cchData) const WCHAR *pszData,
    _In_ uint32 cchData,
    bool fMore )
{
    pkRESULT pkResult = pkS_OK;

    MANIFESTPARSER_TRACE(("ElementContent[%d]: DataLen %d More %s", m_State, cchData, fMore ? "true" : "false"));

    if ( OLT_EnterProtectionHeader == m_State )
    {
        m_wstrPlayReadyBlob.append( pszData, cchData );
    }
    else if ( OLT_EnterFragmentMetadata == m_State )
    {
        pkResult = ElementContent_FragmentMetadata( pszData, cchData, fMore );
        CHECK_PKRESULT_GOTO( pkResult, exit );
    }

exit:

    return( pkResult );
};


/////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::ElementEnd(
    const WCHAR *strName,
    uint32 NameLen )
{
    pkRESULT pkResult = pkS_OK;

#ifdef MANIFESTPARSER_SPEW
    if (!IS_ELEMENT_NAME(L"c"))
    {
        wstring wstrName;
        wstrName.assign(strName, NameLen);
        MANIFESTPARSER_TRACE(("ElementEnd[%d]: '%ls'", m_State, wstrName.c_str()));
    }
#endif

    if ( OLT_EnterMediaIndex == m_State && ( IS_ELEMENT_NAME(L"MediaIndex") || IS_ELEMENT_NAME(L"SmoothStreamingMedia") ) )
    {
        m_State = OLT_NONE; // manifest parsing is done.
    }
    else if ( OLT_EnterSegment == m_State && IS_ELEMENT_NAME(L"Segment") )
    {
        m_State = OLT_EnterMediaIndex;
    }
    else if ( OLT_EnterStreamIndex == m_State && IS_ELEMENT_NAME(L"StreamIndex") )
    {
        if ( m_pCurrentStreamInfo )
        {
            if( eManifestMode_Initial == m_mode )
            {
                m_pCurrentStreamInfo->SortQualityLevels();
            }

            if( eManifestMode_Subsequent == m_mode && !m_ChunkInfo.empty())
            {
                m_pCurrentStreamInfo->AddChunks(m_ChunkInfo, MBR::eBufferDirection_Backward);
                m_ChunkInfo.clear();
            }
            m_pCurrentStreamInfo = NULL;
        }

        m_pCurrentTrack = NULL;
        m_State = OLT_EnterMediaIndex;

        if( m_pChunkManifest->IsSegmented() )
        {
            m_State = OLT_EnterSegment;
        }
    }
    else if ( OLT_EnterCustomAttributes == m_State )
    {
        m_State = OLT_EnterQualityLevel;
    }
    else if ( OLT_EnterCustomAttributeData == m_State )
    {
        m_State = OLT_EnterCustomAttributes;
    }
    else if ( OLT_EnterQualityLevel == m_State && ((m_dwManifestVersion == 0) ? IS_ELEMENT_NAME(L"Bitrate") : IS_ELEMENT_NAME(L"QualityLevel") ) )
    {
        m_State = OLT_EnterStreamIndex;
    }
    else if( m_State == OLT_EnterChunkMetadata )
    {
        m_State = OLT_EnterStreamIndex;
    }
    else if( m_State == OLT_EnterFragmentMetadata )
    {
        m_State = OLT_EnterChunkMetadata;
        m_pCurrentTrack = NULL;
    }
    else if ( OLT_EnterProtectionHeader == m_State && IS_ELEMENT_NAME(L"ProtectionHeader") )
    {
        if (m_wstrPlayReadyBlob.length() > 0)
        {
            pkResult = m_pChunkManifest->SetAttribute(MBR_MS_BLOB_PLAYREADY_OBJECT, m_wstrPlayReadyBlob);
        }

        m_State = OLT_EnterProtection;
    }
    else if ( OLT_EnterProtection == m_State && IS_ELEMENT_NAME(L"Protection") )
    {
        m_State = OLT_EnterMediaIndex;
    }
    else if( OLT_Ignore == m_State )
    {
        m_IgnoredLevel--;
        if( 0 == m_IgnoredLevel )
        {
            m_State = m_previousState;
        }
    }

    return pkResult;
};

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::ParseSmoothStreamingMediaElement(const WCHAR* strName,
                                                                    uint32 NameLen, 
                                                                    const XMLAttribute *pAttributes, 
                                                                    uint32 NumAttributes ) 
{
    pkRESULT pkResult = pkS_OK;
    m_State = OLT_EnterMediaIndex;

    if( eManifestMode_Subsequent == m_mode )
    {
        goto done;
    }

    // store all the attributes so that the app can query them
    StoreAttributes( pAttributes, NumAttributes, m_pChunkManifest);

    if ( GetAttrByName(L"MajorVersion", pAttributes, NumAttributes, m_wstrValue) )
    {
        m_dwManifestVersion = m_pChunkManifest->MajorVersion() = wideToUInt32(m_wstrValue);
    }

    if ( GetAttrByName(L"MinorVersion", pAttributes, NumAttributes, m_wstrValue) )
    {
        m_pChunkManifest->MinorVersion() = wideToUInt32(m_wstrValue);
    }

    m_pChunkManifest->m_dwCombinedVersion = (m_pChunkManifest->MajorVersion() << 16) | m_pChunkManifest->MinorVersion();

    switch (m_pChunkManifest->MajorVersion())
    {
    case 0:
        // In v.0.* root element must be MediaIndex
        if ( !IS_ELEMENT_NAME(L"MediaIndex") )
        {
            SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
        }

        // In later versions, duration comes from root element attributes, not the following Attribute element
        m_wstrValue = toWString64(m_pChunkManifest->Duration());
        CHECK_PKRESULT_GOTO( m_pChunkManifest->SetAttribute(MBR_MS_UINT64_PLAY_DURATION, m_wstrValue), done );
        break;

    case 1:
    case 2:
    case 3:
        // In v.1.0, v.2.*, and v.3.* root element must be SmoothStreamingMedia
        if ( !IS_ELEMENT_NAME(L"SmoothStreamingMedia") )
        {
            SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
        }

        if ( GetAttrByName(L"Duration", pAttributes, NumAttributes, m_wstrValue) )
        {
            m_pChunkManifest->Duration() = wideToUInt64( m_wstrValue );
        }

        // following attributes are defined in 2.x.
        if ( GetAttrByName(L"TimeScale", pAttributes, NumAttributes, m_wstrValue) )
        {
            m_pChunkManifest->TimeScale() = wideToUInt64(m_wstrValue);
            if (0 == m_pChunkManifest->TimeScale())
            {
                SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
            }
        }

        // optional 'start' attribute, in timescale unit
        if ( GetAttrByName(L"Start", pAttributes, NumAttributes, m_wstrValue) )
        {
            m_pChunkManifest->m_hnsMarkIn = wideToUInt64( m_wstrValue );
        }

        if ( m_pChunkManifest->TimeScale() != (int64)MBR_DEFAULT_TIMESCALE )
        {
            m_pChunkManifest->Duration() *= (int64)(((double)MBR_DEFAULT_TIMESCALE)/m_pChunkManifest->TimeScale());
            m_pChunkManifest->m_hnsMarkIn *= (uint64)(((double)MBR_DEFAULT_TIMESCALE)/m_pChunkManifest->TimeScale());
        }

        // optional 'IsLive' attribute
        if ( GetAttrByName(L"IsLive", pAttributes, NumAttributes, m_wstrValue) )
        {
            m_pChunkManifest->IsLive() = IS_ATTRIBUTE_VALUE(m_wstrValue, L"TRUE");
        }

        // optional 'LookAheadFragmentCount' attribute, present for live case
        if ( GetAttrByName(L"LookAheadFragmentCount", pAttributes, NumAttributes, m_wstrValue) )
        {
            m_pChunkManifest->LookAheadCount() = wideToUInt32( m_wstrValue );
        }

        // optional 'DVRWindowLength' attribute, present for live case
        if ( GetAttrByName(L"DVRWindowLength", pAttributes, NumAttributes, m_wstrValue) )
        {
            uint32 dwWindowLength = (uint32)(wideToUInt64( m_wstrValue ) / m_pChunkManifest->TimeScale());
            m_pChunkManifest->DVRWindowLength() = dwWindowLength;
        }

        // optional 'TimeScaleZeroPoint' attribute
        if ( GetAttrByName(L"TimeScaleZeroPoint", pAttributes, NumAttributes, m_wstrValue) )
        {
            m_pChunkManifest->m_wstrTimeScaleZeroPoint = m_wstrValue;
        }

        break;
    default:
        // Other versions are not supported
        SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
        break;
    }

done:
    return pkResult;
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::ParseDrmSchemeElement(const WCHAR* strName,
                                                                    uint32 NameLen, 
                                                                    const XMLAttribute *pAttributes, 
                                                                    uint32 NumAttributes ) 
{
    pkRESULT pkResult = pkS_OK;
    
    if ( m_pChunkManifest->MajorVersion() != 2 )
    {
        SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
    }

    CHECKBOOL_GOTO( GetAttrByName(L"DrmId", pAttributes, NumAttributes, m_wstrValue), done );
    if ( IS_ATTRIBUTE_VALUE(m_wstrValue, WMDRM_GUID ) ) // WMRDM
    {
        // DrmBlob, add directly to manifest attributes
        CHECKBOOL_GOTO( GetAttrByName(L"DrmBlob", pAttributes, NumAttributes, m_wstrValue), done );
        CHECK_PKRESULT_GOTO( m_pChunkManifest->SetAttribute(MBR_MS_BLOB_WMDRM_OBJECT, m_wstrValue), done );
    }
    else
    {
        MANIFEST_DEBUG_VALIDATE( "Unsupported DRM scheme." );
        SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
    }

done:
    return pkResult;
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::ParseProtectionHeaderElement(const WCHAR* strName,
                                                                    uint32 NameLen, 
                                                                    const XMLAttribute *pAttributes, 
                                                                    uint32 NumAttributes ) 
{
    pkRESULT pkResult = pkS_OK;
    
    CHECKBOOL_GOTO( GetAttrByName(L"SystemID", pAttributes, NumAttributes, m_wstrValue), done );
    m_wstrValue = wtrim(m_wstrValue, L"{} \t");
    if ( IS_ATTRIBUTE_VALUE(m_wstrValue, PLAYREADY_GUID ) ) // PlayReady
    {
        m_State = OLT_EnterProtectionHeader;
        m_wstrPlayReadyBlob = L"";
    }
    else
    {
        MANIFEST_DEBUG_VALIDATE( "Unsupported DRM scheme." );
        SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
    }

done:
    return pkResult;
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::ParseSegmentElement(const WCHAR* strName,
                                                                    uint32 NameLen, 
                                                                    const XMLAttribute *pAttributes, 
                                                                    uint32 NumAttributes ) 
{
    pkRESULT pkResult = pkS_OK;

    if ( m_pChunkManifest->MajorVersion() < 3 )
    {
        TRACE_ERROR(("Segment element is found with invalid manifest MajorVersion %d",m_pChunkManifest->MajorVersion()));
        SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
    }

    m_State = OLT_EnterSegment;

    if( eManifestMode_Subsequent == m_mode )
    {
        goto done;
    }

    if ( GetAttrByName(L"StartTime", pAttributes, NumAttributes, m_wstrValue) )
    {
        m_pChunkManifest->SetReferenceSegmentStartTime( wideToUInt64( m_wstrValue ) );
    }
    else
    {
        TRACE_ERROR(("Segment element does not have StartTime attribute"));
        SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
    }

    if ( GetAttrByName(L"Length", pAttributes, NumAttributes, m_wstrValue) )
    {
        m_pChunkManifest->SetSegmentDuration( wideToUInt64( m_wstrValue ) );
    }
    else
    {
        TRACE_ERROR(("Segment element does not have Length attribute"));
        SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
    }

    if ( GetAttrByName(L"Url", pAttributes, NumAttributes, m_wstrValue) )
    {
        m_pChunkManifest->SetSegmentUrlTemplate( m_wstrValue );
    }
    else
    {
        TRACE_ERROR(("Segment element does not have Url attribute"));
        SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
    }

    m_pChunkManifest->SetSegmented( true );

done:
    return pkResult;
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::ParseStreamIndexElement(const WCHAR* strName,
                                                                    uint32 NameLen, 
                                                                    const XMLAttribute *pAttributes, 
                                                                    uint32 NumAttributes ) 
{
    pkRESULT pkResult = pkS_OK;
    const STREAM_TYPE_TRAITS* pStreamTraits = NULL;
    MediaStreamType streamType = MediaStreamTypeUnknown;
    wstring streamName = L"";

    if (( m_pChunkManifest->MajorVersion() == 3 ) && ( OLT_EnterSegment != m_State ))
    {
        TRACE_ERROR(("Manifest MajorVersion 3, but StreamIndex is not nested in Segment element"));
        SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
    }

    // Attribute Type=...
    CHECKBOOL_GOTO(GetAttrByName(L"Type", pAttributes, NumAttributes, m_wstrValue), done); // Mandatory attribute
    if ( IS_ATTRIBUTE_VALUE(m_wstrValue, L"VIDEO" ) )
    {
        streamType = MediaStreamTypeVideo;
    }
    else if ( IS_ATTRIBUTE_VALUE(m_wstrValue, L"AUDIO" ) )
    {
        streamType = MediaStreamTypeAudio;
    }
    else if ( IS_ATTRIBUTE_VALUE(m_wstrValue, L"TEXT" ) )
    {
        streamType = MediaStreamTypeText;
    }
    else if ( IS_ATTRIBUTE_VALUE(m_wstrValue, L"BINARY" ) )
    {
        streamType = MediaStreamTypeBinary;
    }
    else
    {
        MANIFEST_DEBUG_VALIDATE("Invalid media stream Type attribute.");
        SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
    }

    // Attribute Name=
    if ( GetAttrByName(L"Name", pAttributes, NumAttributes, m_wstrValue) )
    {
        streamName = m_wstrValue;
    }

    m_wstrFourCC = L"";
    m_dwBufferTime = 0;
    m_dwCurrentChunk = MBR_INVALID_INDEX;

    // If there is an existing stream, m_pCurrentStreamInfo will be set
    CHECK_PKRESULT_GOTO( FindStream(streamType, streamName), done );
    if( m_pCurrentStreamInfo )
    {
        m_isNewStream = false;

        // Check if the existing stream has the chunkbuffer initialized
        if( NULL == m_pCurrentStreamInfo->m_pChunkBuffer )
        {
            m_fInitializeChunks = true;
        }
        m_State = OLT_EnterStreamIndex;
        goto done;
    }

    // non-sparse streams with no 'Chunks' attribute are ignored
    if ( ( !GetAttrByName(L"Chunks", pAttributes, NumAttributes, m_wstrValue) 
             && !GetAttrByName(L"ParentStreamIndex", pAttributes, NumAttributes, m_wstrValue ) )
             || eManifestMode_Subsequent == m_mode )
    {
        TRACE_ERROR(("Ignoring stream %s, type %d", streamName.c_str(), streamType));
        IgnoreElement();
        goto done;
    }

    m_State = OLT_EnterStreamIndex;
    pStreamTraits = GetStreamTypeTraits( streamType );
    m_pCurrentStreamInfo = m_pChunkManifest->AddStream();
    CHECKNULL_SET_PKRESULT_GOTO( m_pCurrentStreamInfo, pkE_OUTOFMEMORY, done );

    m_isNewStream = true;

    // store all the attributes so that the app can query them
    StoreAttributes( pAttributes, NumAttributes, m_pCurrentStreamInfo );

    //by default, stream timescale inherits from its parent element.
    m_pCurrentStreamInfo->TimeScale() = m_pChunkManifest->TimeScale();

    if ( MBR_MAX_STREAMCOUNT < m_pChunkManifest->GetStreamCount() )
    {
        SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
    }

    // Store the type and name
    m_pCurrentStreamInfo->Type() = streamType;
    m_pCurrentStreamInfo->Name() = streamName;

    // Attribute Subtype=...:
    // Subtype may or may not be required depending on manifest version and media type ...
    // try to parse it out if available.
    if ( GetAttrByName(L"Subtype", pAttributes, NumAttributes, m_wstrValue) )
    {
        m_pCurrentStreamInfo->SetSubType(m_wstrValue.c_str());
    }

    // Attribute Chunks=...
    if ( GetAttrByName(L"Chunks", pAttributes, NumAttributes, m_wstrValue) )
    {
        m_pCurrentStreamInfo->m_dwManifestChunkCount = wideToUInt32(m_wstrValue);

        if ( !m_pChunkManifest->IsLive() && ((m_pCurrentStreamInfo->m_dwManifestChunkCount == 0)
            || (m_pCurrentStreamInfo->m_dwManifestChunkCount > gMbrConfiguration.ChunklistMaxSize)) )
        {
            MANIFEST_DEBUG_VALIDATE("Invalid Chunks attribute value.");
            SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
        }

        m_fInitializeChunks = true;
    }

    // Attribute Url=...
    if ( GetAttrByName(L"Url", pAttributes, NumAttributes, m_wstrValue) )
    {
        if ( m_pChunkManifest->IsSegmented() )
        {
            // get the segment url prefix
            std::wstring::size_type slashPos =
                    m_pChunkManifest->SegmentUrlTemplate().find_last_of( '/' );

            if ( slashPos != std::wstring::npos )
            {
                // concatenate segment url with stream url
                m_pCurrentStreamInfo->Url() =
                    m_pChunkManifest->SegmentUrlTemplate().substr( 0, slashPos + 1 ) + m_wstrValue;
            }
            else
            {
                TRACE_ERROR(("SegmentUrlTemplate doesn't have '/': %ls", m_pChunkManifest->SegmentUrlTemplate().c_str() ));
                SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
            }
        }
        else
        {
            m_pCurrentStreamInfo->Url() = m_wstrValue;
        }
    }

    // Attribute Language=...
    if ( GetAttrByName(L"Language", pAttributes, NumAttributes, m_wstrValue) )
    {
        m_pCurrentStreamInfo->Language() = m_wstrValue;
    }

    // Attribute TimeScale=...
    if ( GetAttrByName(L"TimeScale", pAttributes, NumAttributes, m_wstrValue) )
    {
        m_pCurrentStreamInfo->TimeScale() = wideToUInt64(m_wstrValue);
    }

    if (m_pCurrentStreamInfo->Type() == MediaStreamTypeVideo)
    {
        // Attribute MaxWidth=...
        if ( GetAttrByName(L"MaxWidth", pAttributes, NumAttributes, m_wstrValue) )
        {
            m_pCurrentStreamInfo->MaxWidth() = wideToUInt32(m_wstrValue);
        }

        // Attribute MaxHeight=...
        if ( GetAttrByName(L"MaxHeight", pAttributes, NumAttributes, m_wstrValue) )
        {
            m_pCurrentStreamInfo->MaxHeight() = wideToUInt32(m_wstrValue);
        }

        if ( GetAttrByName(L"DisplayWidth", pAttributes, NumAttributes, m_wstrValue) )
        {
            m_pCurrentStreamInfo->DisplayWidth() = wideToUInt32(m_wstrValue);
        }

        if ( GetAttrByName(L"DisplayHeight", pAttributes, NumAttributes, m_wstrValue) )
        {
            m_pCurrentStreamInfo->DisplayHeight() = wideToUInt32(m_wstrValue);
        }
    }

    if ( m_dwManifestVersion >= 2 )
    {
        // Attribute ManfiestOutput=...
        if ( GetAttrByName(L"ManifestOutput", pAttributes, NumAttributes, m_wstrValue) )
        {
            m_pCurrentStreamInfo->m_bManifestOutput = IS_ATTRIBUTE_VALUE( m_wstrValue, L"TRUE" );
        }

        if( GetAttrByName(L"ParentStreamIndex", pAttributes, NumAttributes, m_wstrValue ) )
        {
            if( !pStreamTraits->canBeSparse )
            {
                TRACE_ERROR(("Stream '%ls' of MediaStreamType = %d can't be sparse", m_pCurrentStreamInfo->Name().c_str(), m_pCurrentStreamInfo->Type()));
                SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
            }

            m_pCurrentStreamInfo->ParentStreamName() = m_wstrValue;
        }
    }
    else if (m_dwManifestVersion == 1)
    {
        // Reset mbr index. Manifest 1.x does not have 'Index' attribute in 'QualityLevel' element,
        // so we need to track it.
        m_dwCurrentMbrIdx = MBR_INVALID_INDEX;
    }

done:
    return pkResult;
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::ParseQualityLevelElement(const WCHAR* strName,
                                                                    uint32 NameLen, 
                                                                    const XMLAttribute *pAttributes, 
                                                                    uint32 NumAttributes ) 
{
    pkRESULT pkResult = pkS_OK;
    AutoRefPtr<CManifestTrack> apTrack;
    uint32_t trackIndex = 0;

    const STREAM_TYPE_TRAITS* pStreamTraits = GetStreamTypeTraits( m_pCurrentStreamInfo->Type() );
    CHECKNULL_SET_PKRESULT_GOTO( m_pCurrentStreamInfo, pkE_UNEXPECTED, done );

    // Attribute Index/n=...
    if (m_dwManifestVersion >= 2)
    {
        if ( GetAttrByName(L"Index", pAttributes, NumAttributes, m_wstrValue) )
        {
            trackIndex = m_dwCurrentMbrIdx = wideToUInt32(m_wstrValue);
        }
    }
    else if (m_dwManifestVersion == 0)
    {
        if ( GetAttrByName(L"n", pAttributes, NumAttributes, m_wstrValue) )
        {
            trackIndex = m_dwCurrentMbrIdx = wideToUInt32(m_wstrValue);
        }
    }
    else if (m_dwManifestVersion == 1)
    {
        trackIndex = m_dwCurrentMbrIdx = (m_dwCurrentMbrIdx == MBR_INVALID_INDEX) ? 0 : m_dwCurrentMbrIdx + 1;
    }

    m_State = OLT_EnterQualityLevel;

    //skip if track already exists
    if ((!m_isNewStream) && (TrackExist(m_pCurrentStreamInfo, trackIndex)))
    {
        goto done;
    }

    // adding new track to an existing stream
    // needs to resize chunk buffer
    if (!m_isNewStream)
    {
        m_fInitializeChunks = true;
    }

    CHECK_PKRESULT_GOTO( m_pCurrentStreamInfo->AddTrack( apTrack.DerefOutPtr() ), done );

    if ( MBR_MAX_BITRATECOUNT < m_pCurrentStreamInfo->m_Tracks.size() )
    {
        SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
    }

    // store all the attributes so that the app can query them
    StoreAttributes( pAttributes, NumAttributes, static_cast<CManifestTrack*>(apTrack));

    apTrack->TrackIndex() = trackIndex;

    if (MediaStreamTypeVideo == m_pCurrentStreamInfo->Type())
    {
        // Attribute MaxWidth/Width/w=...
        m_wstrAttrName = L"";

        if (m_dwManifestVersion >= 2) { m_wstrAttrName = L"MaxWidth"; }
        else
        if (m_dwManifestVersion == 1) { m_wstrAttrName = L"Width";    }
        else
        if (m_dwManifestVersion == 0) { m_wstrAttrName = L"w";        }

        if ( GetAttrByName(m_wstrAttrName.c_str(), pAttributes, NumAttributes, m_wstrValue) )
        {
            apTrack->MaxWidth() = wideToUInt32(m_wstrValue);
        }

        // Attribute MaxHeight/Height/h=...
        m_wstrAttrName = L"";
        if (m_dwManifestVersion >= 2) { m_wstrAttrName = L"MaxHeight"; }
        else
        if (m_dwManifestVersion == 1) { m_wstrAttrName = L"Height";    }
        else
        if (m_dwManifestVersion == 0) { m_wstrAttrName = L"h";         }

        if ( GetAttrByName(m_wstrAttrName.c_str(), pAttributes, NumAttributes, m_wstrValue) )
        {
            apTrack->MaxHeight() = wideToUInt32(m_wstrValue);
        }

        if (m_pCurrentStreamInfo->MaxHeight() < apTrack->MaxHeight())
            m_pCurrentStreamInfo->MaxHeight() = apTrack->MaxHeight();
        if (m_pCurrentStreamInfo->MaxWidth() < apTrack->MaxWidth())
            m_pCurrentStreamInfo->MaxWidth() = apTrack->MaxWidth();
    }

    if (m_dwManifestVersion >= 2)
    {
        if ( GetAttrByName(L"BufferTime", pAttributes, NumAttributes, m_wstrValue) )
        {
            //Current Digital Rapid assets are tagging different video quality streams
            //with different buffer time which causes the stream tune to fail.  Instead
            //we could pick worst case buffer time for now which will not hurt.
            uint32 bufferTime = wideToUInt32(m_wstrValue);
            if (bufferTime > m_dwBufferTime)
            {
                m_dwBufferTime = bufferTime;
            }
            m_pChunkManifest->m_dwBufferTime = m_dwBufferTime;
        }

        if ( GetAttrByName(L"NominalBitrate", pAttributes, NumAttributes, m_wstrValue) )
        {
            apTrack->NominalBitrate() = wideToUInt32(m_wstrValue);
        }

        if ( GetAttrByName(L"NALUnitLength", pAttributes, NumAttributes, m_wstrValue) ||
                GetAttrByName(L"NALUnitLengthField", pAttributes, NumAttributes, m_wstrValue) )
        {
            apTrack->NALUnitLength() = wideToUInt32(m_wstrValue);
        }

        // Attribute FourCC=...
        if ( GetAttrByName(L"FourCC", pAttributes, NumAttributes, m_wstrValue) )
        {
            if ((MediaStreamTypeAudio != m_pCurrentStreamInfo->Type()) && !m_wstrFourCC.empty() && (m_wstrFourCC != m_wstrValue))
            {
                MANIFEST_DEBUG_VALIDATE( "Different FourCC codes for different quality levels is not supported." );
                SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
            }
            m_wstrFourCC = m_wstrValue;

            if( !m_wstrFourCC.empty() )
            {
                CHECK_PKRESULT_GOTO( StrToFourCC( m_wstrFourCC.c_str(), &apTrack->FourCC() ), done );
            }

            apTrack->StreamType() = FourCCToStreamType( apTrack->FourCC() );
        }

        // Check if FourCC is required
        if ( pStreamTraits->requiresFourCC && m_wstrFourCC.empty() )
        {
            SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
        }

        // Attribute CodecPrivateData=...
        if ( GetAttrByName(L"CodecPrivateData", pAttributes, NumAttributes, m_wstrValue) && !m_wstrValue.empty() )
        {
            size_t offset = ( MediaStreamTypeAudio == m_pCurrentStreamInfo->Type()) ? sizeof(WAVEFORMATEX) : 0;

            CHECK_PKRESULT_GOTO( apTrack->CodecPrivateData().ParseFromHexStr( m_wstrValue.c_str(), m_wstrValue.length(), offset ), done );
        }

        // Audio specific attributes
        if ( MediaStreamTypeAudio == m_pCurrentStreamInfo->Type() )
        {
            if( apTrack->CodecPrivateData().size() < sizeof(WAVEFORMATEX) )
            {
                apTrack->CodecPrivateData().resize( sizeof(WAVEFORMATEX) );
            }

            // If no CodecPrivateData is available, pQL->dwCodecBlobSize will be set to zero and pQL->pbCodecBlob array should be zeroed
            WAVEFORMATEX* pWaveFormatEx = reinterpret_cast<WAVEFORMATEX*>( apTrack->CodecPrivateData().data() );

            CHECKBOOL_GOTO( GetAttrByName(L"AudioTag", pAttributes, NumAttributes, m_wstrValue), done );

            apTrack->AudioTag() = wideToUInt32(m_wstrValue);
            pWaveFormatEx->wFormatTag = (uint16)apTrack->AudioTag();

            CHECKBOOL_GOTO( GetAttrByName(L"Channels", pAttributes, NumAttributes, m_wstrValue), done );
            pWaveFormatEx->nChannels = (uint16) wideToUInt32(m_wstrValue);

            CHECKBOOL_GOTO( GetAttrByName(L"SamplingRate", pAttributes, NumAttributes, m_wstrValue), done );
            pWaveFormatEx->nSamplesPerSec = wideToUInt32(m_wstrValue);

            CHECKBOOL_GOTO( GetAttrByName(L"Bitrate", pAttributes, NumAttributes, m_wstrValue), done );
            pWaveFormatEx->nAvgBytesPerSec = wideToUInt32(m_wstrValue) / 8;

            CHECKBOOL_GOTO( GetAttrByName(L"PacketSize", pAttributes, NumAttributes, m_wstrValue), done );
            pWaveFormatEx->nBlockAlign = (uint16) wideToUInt32(m_wstrValue);

            CHECKBOOL_GOTO( GetAttrByName(L"BitsPerSample", pAttributes, NumAttributes, m_wstrValue), done );
            pWaveFormatEx->wBitsPerSample = (uint16) wideToUInt32(m_wstrValue);

            pWaveFormatEx->cbSize = (uint16)( apTrack->CodecPrivateData().size() - sizeof(WAVEFORMATEX));
        }
    }
    else if ( 1 == m_dwManifestVersion )
    {
        // Attribute FourCC=...
        if ( GetAttrByName(L"FourCC", pAttributes, NumAttributes, m_wstrValue) )
        {
            if ( (MediaStreamTypeAudio != m_pCurrentStreamInfo->Type()) && !m_wstrFourCC.empty() && (m_wstrFourCC != m_wstrValue) )
            {
                MANIFEST_DEBUG_VALIDATE( "Different FourCC codes for different quality levels is not supported." );
                SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
            }

            m_wstrFourCC = m_wstrValue;

            if( !m_wstrFourCC.empty() )
            {
                CHECK_PKRESULT_GOTO( StrToFourCC( m_wstrFourCC.c_str(), &apTrack->FourCC() ), done);
            }
            apTrack->StreamType() = FourCCToStreamType( apTrack->FourCC() );
        }

        // Check if FourCC is required
        if ( pStreamTraits->requiresFourCC && m_wstrFourCC.empty() )
        {
            SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
        }

        if ( MediaStreamTypeVideo == m_pCurrentStreamInfo->Type())
        {
            // Attribute CodecPrivateData=...
            CHECKBOOL_GOTO( GetAttrByName(L"CodecPrivateData", pAttributes, NumAttributes, m_wstrValue), done );
            CHECK_PKRESULT_GOTO( apTrack->CodecPrivateData().ParseFromHexStr( m_wstrValue.c_str(), m_wstrValue.length(), 0 ), done );
        }
        else if ( MediaStreamTypeAudio == m_pCurrentStreamInfo->Type())
        {
            CHECKBOOL_GOTO( GetAttrByName(L"WaveFormatEx", pAttributes, NumAttributes, m_wstrValue), done );
            if ( m_wstrValue.length()/2 >= sizeof(WAVEFORMATEX) )
            {
                CHECK_PKRESULT_GOTO( apTrack->CodecPrivateData().ParseFromHexStr( m_wstrValue.c_str(), m_wstrValue.length(), 0 ), done );

                WAVEFORMATEX* pWaveFormatEx = reinterpret_cast<WAVEFORMATEX*>( apTrack->CodecPrivateData().data() );

                // WAVEFORMATEX fields are persisted in the manifest using
                // little endian representations. Convert the main fields to
                // CPU representations.

                pWaveFormatEx->wFormatTag = LittleEndian::ToHost( pWaveFormatEx->wFormatTag );
                pWaveFormatEx->nChannels = LittleEndian::ToHost( pWaveFormatEx->nChannels );
                pWaveFormatEx->nSamplesPerSec = LittleEndian::ToHost( pWaveFormatEx->nSamplesPerSec );
                pWaveFormatEx->nAvgBytesPerSec = LittleEndian::ToHost( pWaveFormatEx->nAvgBytesPerSec );
                pWaveFormatEx->nBlockAlign = LittleEndian::ToHost( pWaveFormatEx->nBlockAlign );
                pWaveFormatEx->wBitsPerSample = LittleEndian::ToHost( pWaveFormatEx->wBitsPerSample );
                pWaveFormatEx->cbSize = LittleEndian::ToHost( pWaveFormatEx->cbSize );
            }
        }
    }

    // Attribute HardwareProfile=...
    if ( GetAttrByName(L"HardwareProfile", pAttributes, NumAttributes, m_wstrValue) )
    {
        apTrack->HardwareProfile() = wideToUInt32(m_wstrValue);
    }
    /* TODO: Set default value once bug 2563 is fixed
    else { apTrack->HardwareProfile() = ?Default value? ; }
    */

    // Attribute Bitrate/Kbps=...
    if ( GetAttrByName(((m_dwManifestVersion == 0) ? L"Kbps" : L"Bitrate"), pAttributes, NumAttributes, m_wstrValue) )
    {
        // version 2.0 and 1.x manifest uses bps in 'bitrate' attribute
        // version 0.0 manifest uses kbps in 'kbps' attribute
        apTrack->Bitrate() = wideToUInt32(m_wstrValue) * ((m_dwManifestVersion == 0) ? 1000 : 1);

        if ((apTrack->Bitrate() == 0) && pStreamTraits->requiresBitrate )
        {
            MANIFEST_DEBUG_VALIDATE("Stream type requires non-zero bitrate attribute.");
            SET_PKRESULT_GOTO( pkE_INVALID_FORMAT, done );
        }
    }

done:
    return pkResult;
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::ParseAttributeElement(const WCHAR* strName,
                                                                    uint32 NameLen, 
                                                                    const XMLAttribute *pAttributes, 
                                                                    uint32 NumAttributes ) 
{
    pkRESULT pkResult = pkS_OK;
    AutoRefPtr<CManifestTrack> apTrack;
    const STREAM_TYPE_TRAITS* pStreamTraits = NULL;

    m_State = OLT_EnterCustomAttributeData;

    // Use the stream we are currently processing
    CHECKNULL_SET_PKRESULT_GOTO( m_pCurrentStreamInfo, pkE_UNEXPECTED, done );
    pStreamTraits = GetStreamTypeTraits( m_pCurrentStreamInfo->Type() );

    CHECKBOOL_GOTO( (m_dwCurrentMbrIdx < m_pCurrentStreamInfo->m_Tracks.size()), done );
    apTrack = m_pCurrentStreamInfo->m_Tracks[ m_dwCurrentMbrIdx ];

    m_wstrAttrName = L"";
    m_wstrAttrValue = L"";
    for (uint32 i = 0; i < NumAttributes; i++)
    {
        if (wcsncmp(L"Name", pAttributes[i].strName, pAttributes[i].NameLen)==0)
        {
            m_wstrAttrName.assign(pAttributes[i].strValue, pAttributes[i].ValueLen);
        }
        else if (wcsncmp(L"Value", pAttributes[i].strName, pAttributes[i].NameLen)==0)
        {
            m_wstrAttrValue.assign(pAttributes[i].strValue, pAttributes[i].ValueLen);
        }

        if ( !m_wstrAttrName.empty() && !m_wstrAttrValue.empty() )
        {
            wstring attribVal;
            if ( !apTrack->GetCustomAttributeValue( m_wstrAttrName, &attribVal ) )
            {
                apTrack->SetCustomAttribute( m_wstrAttrName.c_str(), m_wstrAttrValue.c_str() );
            }

            MANIFESTPARSER_TRACE(("AddCustomAttribute(%ls, %ls)", m_wstrAttrName.c_str(), m_wstrAttrValue.c_str()));
            break;
        }
    }

done:
    return pkResult;
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::ParseCElement(const WCHAR* strName,
                                                                    uint32 NameLen, 
                                                                    const XMLAttribute *pAttributes, 
                                                                    uint32 NumAttributes ) 
{
    pkRESULT pkResult = pkS_OK;
    const STREAM_TYPE_TRAITS* pStreamTraits = NULL;
    uint64 qwTimestamp = INVALID_TIME;

    m_State = OLT_EnterChunkMetadata;

    uint32 dwChunkIndex = MBR_INVALID_INDEX;
    uint32 cBitrate = 0;

    // Use the stream we are currently processing
    CHECKNULL_SET_PKRESULT_GOTO( m_pCurrentStreamInfo, pkE_UNEXPECTED, done );
    pStreamTraits = GetStreamTypeTraits( m_pCurrentStreamInfo->Type() );

    if (m_fInitializeChunks)
    {
        // Initialize chunk array: <c> element means that all QualityLevel/Bitrate elements are processsed
        cBitrate = m_pCurrentStreamInfo->m_Tracks.size();
        CHECKBOOL_GOTO( (cBitrate > 0), done );

        CChunkBuffer* prevChunkBuffer = NULL;

        // If this is not a new stream and a new track was added,
        // need to copy the previous chunk buffer since
        // the chunkbuffer is created based on the number of tracks
        if ( !m_isNewStream )
        {
            prevChunkBuffer = m_pCurrentStreamInfo->m_pChunkBuffer;
        }

        // Create a new chunk buffer
        {
            CChunkBuffer* newChunkBuffer = CChunkBuffer::Create(cBitrate, m_pChunkManifest->IsLive());
            CHECKNULL_SET_PKRESULT_GOTO( newChunkBuffer, pkE_OUTOFMEMORY, done );
            m_pCurrentStreamInfo->m_pChunkBuffer = newChunkBuffer;
        }

        if( prevChunkBuffer )
        {
            for( int32_t indx = prevChunkBuffer->GetMinIndex(); indx <= prevChunkBuffer->GetMaxIndex(); indx++)
            {
                CHECKNULL_SET_PKRESULT_GOTO(
                    m_pCurrentStreamInfo->AddChunk(
                        prevChunkBuffer->GetChunkTicks(indx) ),
                    pkE_UNEXPECTED, done);
            }
            delete prevChunkBuffer;
        }

        m_fInitializeChunks = false;
    }

    if ( GetAttrByName(L"n", pAttributes, NumAttributes, m_wstrValue) )
    {
        dwChunkIndex = wideToUInt32(m_wstrValue);
    }
    else
    {
        // If first attribute isn't chunk index, then use implicit indexing instead
        if ( MBR_INVALID_INDEX == m_dwCurrentChunk )
        {
            // If not a new stream, then get the count to continue from the end
            dwChunkIndex = ( m_isNewStream ) ? 0 : m_pCurrentStreamInfo->m_pChunkBuffer->Count();
        }
        else
        {
            // If the index is known, increment
            dwChunkIndex = m_dwCurrentChunk + 1;
        }
    }

    if ( dwChunkIndex >= m_pCurrentStreamInfo->m_dwManifestChunkCount )
    {
        m_pCurrentStreamInfo->m_dwManifestChunkCount = ( dwChunkIndex + 1 );
        if (!m_pChunkManifest->IsLive() && m_pCurrentStreamInfo->m_dwManifestChunkCount > gMbrConfiguration.ChunklistMaxSize)
        {
            MANIFEST_DEBUG_VALIDATE("Chunk index too large for non-Live content.");
            SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
        }
    }

    if ( GetAttrByName(L"t", pAttributes, NumAttributes, m_wstrValue) )
    {
        qwTimestamp = wideToUInt64( m_wstrValue );
    }

    if ( GetAttrByName(L"d", pAttributes, NumAttributes, m_wstrValue) )
    {
        // set timescale start-pos
        if (qwTimestamp == INVALID_TIME)
        {
            if (dwChunkIndex == 0 && m_pCurrentStreamInfo->m_pChunkBuffer->Count() == 0)
            {
                qwTimestamp = (uint64) (((double)m_pChunkManifest->m_hnsMarkIn) * m_pCurrentStreamInfo->TimeScale() / MBR_DEFAULT_TIMESCALE);
            }
            else
            {
                if( eManifestMode_Initial == m_mode )
                {
                    qwTimestamp = m_pCurrentStreamInfo->m_pChunkBuffer->GetChunkTicks(m_pCurrentStreamInfo->m_pChunkBuffer->GetMaxIndex()) + m_dwLastChunkDuration;
                }
                else
                {
                    if( 0 == m_ChunkInfo.size())
                    {
                        MANIFEST_DEBUG_VALIDATE("No timestamp for first chunk in subsequent mode");
                        SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
                    }
                    qwTimestamp = m_ChunkInfo.back().timestamp + m_dwLastChunkDuration;
                }
            }
        }
        m_dwLastChunkDuration = wideToUInt32(m_wstrValue);

        if( eManifestMode_Initial == m_mode )
        {
            m_pCurrentStreamInfo->m_dwLastChunkDuration = m_dwLastChunkDuration;
        }

        // set default chunk duration to the largest duration value
        if ( m_pCurrentStreamInfo->m_dwLastChunkDuration > m_pCurrentStreamInfo->m_pChunkBuffer->GetDefaultChunkDuration() )
        {
            m_pCurrentStreamInfo->m_pChunkBuffer->SetDefaultChunkDuration(m_pCurrentStreamInfo->m_dwLastChunkDuration);
        }
    }

    // if timestamp is still not set at this point, there's a problem...
    if (qwTimestamp == INVALID_TIME)
    {
        SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
    }

    if ( GetAttrByName(L"r", pAttributes, NumAttributes, m_wstrValue) )
    {
        uint32 dwChunkIterate = wideToUInt32(m_wstrValue);

        if (dwChunkIterate == 0)
        {
            SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
        }
        MANIFESTPARSER_TRACE(("Chunk index %u repeat count %u time %llu", dwChunkIndex, dwChunkIterate, qwTimestamp));

        dwChunkIndex += dwChunkIterate - 1;

        while (dwChunkIterate-- > 0)
        {
            if( eManifestMode_Subsequent == m_mode )
            {
                m_ChunkInfo.push_back(SChunkInfo((int64)qwTimestamp));
            }
            else
            {
                CHECKNULL_SET_PKRESULT_GOTO( m_pCurrentStreamInfo->AddChunk((int64)qwTimestamp), pkE_UNEXPECTED, done );
            }
            qwTimestamp += m_dwLastChunkDuration;
        }
    }
    else // no 'r' attribute
    {
        if( eManifestMode_Subsequent == m_mode )
        {
            m_ChunkInfo.push_back(SChunkInfo((int64)qwTimestamp));
        }
        else
        {
            CHECKNULL_SET_PKRESULT_GOTO( m_pCurrentStreamInfo->AddChunk((int64)qwTimestamp), pkE_UNEXPECTED, done );
        }
    }

    m_dwCurrentChunk = dwChunkIndex;
done:
    return pkResult;
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::ParseFElement(const WCHAR* strName,
                                                                    uint32 NameLen, 
                                                                    const XMLAttribute *pAttributes, 
                                                                    uint32 NumAttributes ) 
{
    pkRESULT pkResult = pkS_OK;
    m_State = OLT_EnterFragmentMetadata;
    m_pCurrentTrack = NULL;

    m_wstrFragmentMetadata.clear();

    ASSERT( m_pChunkManifest->GetStreamCount() > 0 );

    if( m_pCurrentStreamInfo->m_Tracks.size() == 0 )
    {
        // Unexpected <f> element found before any
        // <QualityLevel> elements
        CHECK_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
    }

    if( m_pCurrentStreamInfo->m_Tracks.size() == 1 )
    {
        // Despite the requirement that the index of the corresponding track
        // must be present in the <f> element, some encoders omit it when there's
        // just one track in the stream. In this case, assume this single track as the
        // default.
        m_pCurrentTrack = m_pCurrentStreamInfo->m_Tracks.front();
    }

    for (uint32 i = 0; i < NumAttributes; i++)
    {
        // All names at this level are single character
        if (pAttributes[i].NameLen != 1)
            continue;

        WCHAR chName = towlower( pAttributes[i].strName[0] );
                
        // Use wstring.assign since pAttributes strings may not be NULL terminated
        m_wstrValue.assign(pAttributes[i].strValue, pAttributes[i].ValueLen);

        if ( chName == L'i' )
        {
            uint32 dwMBRID = wideToUInt32(m_wstrValue);

            for( size_t iTrack = 0; iTrack < m_pCurrentStreamInfo->m_Tracks.size(); iTrack++ )
            {
                if( m_pCurrentStreamInfo->m_Tracks[iTrack]->TrackIndex() == dwMBRID )
                {
                    m_pCurrentTrack = m_pCurrentStreamInfo->m_Tracks[iTrack];
                    break;
                }
            }

            if( m_pCurrentTrack == NULL )
            {
                // This track does not exist in the stream.
                // Ignore the <f> element.
                break;
            }
        }
        else if ( chName == L's' )
        {
            if( m_pCurrentTrack == NULL )
            {
                MANIFEST_DEBUG_VALIDATE( false );
                SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
            }

            uint32 valueInKB = wideToUInt32(m_wstrValue);

            if( valueInKB > 0xFFFF )
            {
                // Can't cast the value from uint32 to uint16 for
                // storing in the chunk buffer

                TRACE_ERROR(("Chunk size too large %d",valueInKB));
                MANIFEST_DEBUG_VALIDATE( false );
            }
            else
            {
                if( eManifestMode_Subsequent == m_mode )
                {
                    SFragmentMetadata* fragmentMetadata = FindFragmentMetadataInChunkInfoVector(m_pCurrentTrack);
                    fragmentMetadata->chunkSizeInKB = (uint16)valueInKB;
                }
                else
                {
                    CHECK_PKRESULT_GOTO( m_pCurrentStreamInfo->m_pChunkBuffer->SetChunkSizeInKB( m_pCurrentTrack, m_pCurrentStreamInfo->m_pChunkBuffer->GetMaxIndex(), (uint16)valueInKB ), done );
                }
            }
        }
        else if ( chName == L'q' )
        {
            if ( m_pCurrentTrack == NULL )
            {
                MANIFEST_DEBUG_VALIDATE("Invalid chunk index.");
                SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
            }
            uint32 value = (uint32)wideToUInt64( m_wstrValue );
            if (value >> 16)        // over MAXQUALITY value
            {
                MANIFEST_DEBUG_VALIDATE("Quality value too large.");
                SET_PKRESULT_GOTO( pkE_UNSUPPORTED_FORMAT, done );
            }

            if( eManifestMode_Subsequent == m_mode )
            {
                SFragmentMetadata* fragmentMetadata = FindFragmentMetadataInChunkInfoVector(m_pCurrentTrack);
                fragmentMetadata->quality = (uint16)value;
            }
            else
            {
                CHECK_PKRESULT_GOTO(m_pCurrentStreamInfo->m_pChunkBuffer->SetQuality( m_pCurrentTrack, m_pCurrentStreamInfo->m_pChunkBuffer->GetMaxIndex(), (uint16)value ), done);
            }
        }
    }
done:
    return pkResult;
}

SFragmentMetadata* CManifestParsingCallback::FindFragmentMetadataInChunkInfoVector(_In_ CManifestTrack* pTrack)
{
    SChunkInfo* lastChunkInfo = &m_ChunkInfo.back();

    // check if the fragment metadata already exists
    for(size_t i = 0; i < m_ChunkInfo.back().FragmentMetadata.size(); i++)
    {
        if(pTrack == lastChunkInfo->FragmentMetadata[i].pTrack)
        {
            return &lastChunkInfo->FragmentMetadata.at(i);
        }
    }

    // create the fragmentMetadata if it doesn't already exists
    lastChunkInfo->FragmentMetadata.push_back(SFragmentMetadata(pTrack));
    return &lastChunkInfo->FragmentMetadata.back();
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::ElementContent_FragmentMetadata(
    _In_count_(cchData) const WCHAR* pszData,
    _In_ size_t cchData,
    bool fMore )
{
    pkRESULT pkResult = pkS_OK;

    ASSERT( m_pChunkManifest->GetStreamCount() > 0 );

    if( !m_pCurrentStreamInfo->m_bManifestOutput )
    {
        // Ignore fragment metadata for streams that are not set up
        // to output to the manifest
        goto exit;
    }

    if( m_pCurrentTrack == NULL )
    {
        // Ignore invalid track index
        goto exit;
    }

    m_wstrFragmentMetadata.append( pszData, cchData );

    if( !fMore )
    {
        //
        // If that was the final piece of the data, convert it from
        // base 64 encoding to a byte array
        //

        DWORD cbData;
        AutoRefPtr<IRefBuffer> spFragmentData;

        WStr2Str strData( m_wstrFragmentMetadata );

        m_wstrFragmentMetadata.clear();

        //
        // Decode the <f> element content into a IRefBuffer buffer.
        // Pass 0: computes the size of the buffer to hold the data.
        // Pass 1: reads the data into the buffer.
        //

        pkResult = Base64DecodeExA( strData.c_str(), strData.size(), NULL, &cbData, NULL );
        CHECK_PKRESULT_GOTO( pkResult, exit );

        pkResult = CreateRefBuffer( cbData, spFragmentData.DerefOutPtr() );
        CHECK_PKRESULT_GOTO( pkResult, exit );

        pkResult = Base64DecodeExA( strData.c_str(), strData.size(), spFragmentData->Data(), &cbData, NULL );
        CHECK_PKRESULT_GOTO( pkResult, exit );

        if( eManifestMode_Subsequent == m_mode )
        {
            SFragmentMetadata* fragmentMetadata = FindFragmentMetadataInChunkInfoVector(m_pCurrentTrack);
            fragmentMetadata->apFragmentData.Set(spFragmentData);
        }
        else
        {
            m_pCurrentStreamInfo->m_pChunkBuffer->SetFragmentData( m_pCurrentTrack, m_pCurrentStreamInfo->m_pChunkBuffer->GetMaxIndex(), spFragmentData );
        }
    }

exit:

    return( pkResult );
}

} // namespace MBR

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParsingCallback::FindStream( _In_ MediaStreamType type, _In_ const wstring& name )
{
    pkRESULT pkResult = pkS_OK;
    typedef std::vector< AutoRefPtr<CMediaStreamDescription> >::iterator Iterator;
    m_pCurrentStreamInfo = NULL;

    for( Iterator streamIt = m_pChunkManifest->m_availableStreams.begin();
        streamIt != m_pChunkManifest->m_availableStreams.end();
        ++streamIt )
    {
        if(( type == (*streamIt)->Type() ) && ( name == (*streamIt)->Name() ))
        {
            MANIFESTPARSER_TRACE(("Stream[type: %d, name: %ls] exists",
                (uint32_t)(*streamIt)->Type(), (*streamIt)->Name().c_str()));

            m_pCurrentStreamInfo = (*streamIt);

            // if the stream already exists, then get the buffer time
            // and the fourCC value
            m_dwBufferTime = m_pChunkManifest->m_dwBufferTime;

            if ( !(*streamIt)->m_Tracks.empty() )
            {
                if ( (*streamIt)->m_Tracks.front()->FourCC() > 0 )
                {
                    CHECK_PKRESULT_GOTO( FourCCToStr( (*streamIt)->m_Tracks.front()->FourCC(), &m_wstrFourCC ), exit );
                }
            }

            break;
        }
    }
exit:
    return pkResult;
}

////////////////////////////////////////////////////////////////////////////////
bool CManifestParsingCallback::TrackExist( _In_ CMediaStreamDescription* pStreamInfo, _In_ uint32_t trackIndex )
{
    typedef std::vector< AutoRefPtr<CManifestTrack> >::iterator Iterator;
    bool isExist = false;

    for( Iterator it = pStreamInfo->m_Tracks.begin();
        it != pStreamInfo->m_Tracks.end();
        ++it )
    {
        if( trackIndex == (*it)->TrackIndex() )
        {
            MANIFESTPARSER_TRACE(("Track[%d] of stream [type: %d, name: %ls] exists",
                trackIndex, (uint32_t)pStreamInfo->Type(),  pStreamInfo->Name().c_str()));
            isExist = true;
            break;
        }
    }

    return isExist;
}

////////////////////////////////////////////////////////////////////////////////
template< class storeT >
void CManifestParsingCallback::StoreAttributes( _In_count_(NumAttributes) const XMLAttribute *pAttributes,
    _In_ uint32 NumAttributes,
    _Out_ storeT* pStore )
{
    // store all the attributes
    for(size_t i = 0; i < NumAttributes; i++)
    {
        wstring wstrAttrName = wstring(pAttributes[i].strName, pAttributes[i].NameLen);
        wstring wstrAttrValue = wstring(pAttributes[i].strValue, pAttributes[i].ValueLen);

        // Attribute name should not be empty
        ASSERT( !wstrAttrName.empty() );

        // set attribute can fail if it already exists in the map
        if ( pkFAILED( pStore->SetAttribute(wstrAttrName, wstrAttrValue) ) )
        {
            wstring wstrExistAttrValue;

            // SetAttribute only fails if the attribute name already exists so GetAttribute should
            // always return true
            ASSERT( pStore->GetAttribute(wstrAttrName, &wstrExistAttrValue) );
            MANIFESTPARSER_TRACE(("SetAttribute(%ls, %ls) is not set, already exist with value(%ls)",
                            wstrAttrName.c_str(),
                            wstrAttrValue.c_str(),
                            wstrExistAttrValue.c_str()));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CManifestParser::Parse(_In_ HANDLE hStream, 
                                _In_ PFN_READCB readCallback, 
                                _In_opt_ EManifestMode mode /* = eManifestMode_Initial */, 
                                _Out_opt_ CXmlParser::EEncodingType* pEncodingType /* = NULL */)
{
    pkRESULT pkResult = pkE_OUTOFMEMORY;

    CXmlParser*    xmlParser = NULL;
    CManifestParsingCallback* callback = NULL;

    xmlParser = NEW_NO_THROW CXmlParser();
    CHECKNULL_GOTO( xmlParser, cleanup );

    callback = NEW_NO_THROW CManifestParsingCallback(m_apChunkManifest, mode);
    CHECKNULL_GOTO( callback, cleanup );

    xmlParser->RegisterSAXCallbackInterface(callback);

    pkResult = xmlParser->ParseXMLStream(hStream, readCallback, pEncodingType);

cleanup:
    delete callback;
    delete xmlParser;

    return pkResult;
}
