///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDiagsEvent.h"
#include <strsafe.h>

#include <DiagReportBase.h>
#include <IAVManager.h>
#include <IDiagsManager.h>

//#define IDIAGS_TRACE_ENABLE
#ifdef IDIAGS_TRACE_ENABLE
#define IDIAGS_TRACE(x) TRACE(x)
#else
#define IDIAGS_TRACE(x)
#endif

// ===============================================================================================================
// Serializing diagnostics event as wide character string
// ===============================================================================================================

//",diagsLabel:diagsValue"
void IDiagsEvent::DiagsLogValue(const WCHAR* diagsLabel, int diagsValue, bool isHex)
{
    WCHAR diagsValueString[32];

    StringCchPrintfW(diagsValueString, sizeof(diagsValueString)/sizeof(WCHAR), isHex?L"0x%x":L"%d", diagsValue);

    WCHAR*       diagsData     = DataPtr;
    const WCHAR* diagsValuePtr = diagsValueString;
    int          diagsLabelLen = wcslen(diagsLabel);
    int          diagsValueLen = wcslen(diagsValueString);
    if (diagsData + diagsLabelLen + diagsValueLen + 5 < DataEnd)
    {
        while (diagsLabelLen--) *diagsData++ = *diagsLabel++;
        *diagsData++ = ':';
        while (diagsValueLen--) *diagsData++ = *diagsValuePtr++;
        *diagsData++ = '[';
        *diagsData++ = '|';
        *diagsData++ = '|';
        *diagsData++ = ']';
    }
    else
    {
        ASSERT(false); // data buffer too small
    }
    DataPtr = diagsData;
}

//",diagsLabel:diagsValue"
void IDiagsEvent::DiagsLogValue64(const WCHAR* diagsLabel, int64 diagsValue)
{
    WCHAR diagsValueString[32];
    StringCchPrintfW(diagsValueString, sizeof(diagsValueString)/sizeof(WCHAR), L"%lld", diagsValue);

    WCHAR*       diagsData     = DataPtr;
    const WCHAR* diagsValuePtr = diagsValueString;
    int          diagsLabelLen = wcslen(diagsLabel);
    int          diagsValueLen = wcslen(diagsValueString);
    if (diagsData + diagsLabelLen + diagsValueLen + 5 < DataEnd)
    {
        while (diagsLabelLen--) *diagsData++ = *diagsLabel++;
        *diagsData++ = ':';
        while (diagsValueLen--) *diagsData++ = *diagsValuePtr++;
        *diagsData++ = '[';
        *diagsData++ = '|';
        *diagsData++ = '|';
        *diagsData++ = ']';
    }
    else
    {
        ASSERT(false); // data buffer too small
    }
    DataPtr = diagsData;
}

//",diagsLabel:diagsString"
void IDiagsEvent::DiagsLogGuid(const WCHAR* diagsLabel, const GUID& guid)
{
    if (guid.Data1 == 0 && guid.Data2 == 0 && guid.Data3 == 0)
        return;

    WCHAR diagsValueString[40];
    StringCchPrintfW(diagsValueString, sizeof(diagsValueString)/sizeof(WCHAR), L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
        guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

    WCHAR*       diagsData     = DataPtr;
    const WCHAR* diagsValuePtr = diagsValueString;
    int          diagsLabelLen = wcslen(diagsLabel);
    int          diagsValueLen = wcslen(diagsValueString);
    if (diagsData + diagsLabelLen + diagsValueLen + 5 < DataEnd)
    {
        while (diagsLabelLen--) *diagsData++ = *diagsLabel++;
        *diagsData++ = ':';
        while (diagsValueLen--) *diagsData++ = *diagsValuePtr++;
        *diagsData++ = '[';
        *diagsData++ = '|';
        *diagsData++ = '|';
        *diagsData++ = ']';
    }
    else
    {
        ASSERT(false); // data buffer too small
    }
    DataPtr = diagsData;
}

//",diagsLabel:diagsString"
void IDiagsEvent::DiagsLogString(const WCHAR* diagsLabel, const char* diagsString)
{
    WCHAR* diagsData     = DataPtr;
    int    diagsLabelLen = wcslen(diagsLabel);
    int    diagsValueLen = strlen(diagsString);
    if (diagsData + diagsLabelLen + diagsValueLen + 5 < DataEnd)
    {
        while (diagsLabelLen--) *diagsData++ = *diagsLabel++;
        *diagsData++ = ':';
        while (diagsValueLen--) *diagsData++ = *diagsString++;
        *diagsData++ = '[';
        *diagsData++ = '|';
        *diagsData++ = '|';
        *diagsData++ = ']';
    }
    else
    {
        ASSERT(false); // data buffer too small
    }
    DataPtr = diagsData;
}

//",diagsLabel:diagsWString"
void IDiagsEvent::DiagsLogWString(const WCHAR* diagsLabel,  const WCHAR* diagsWString)
{
    WCHAR* diagsData     = DataPtr;
    int    diagsLabelLen = wcslen(diagsLabel);
    int    diagsValueLen = wcslen(diagsWString);
    if (diagsData + diagsLabelLen + diagsValueLen + 5 < DataEnd)
    {
        while (diagsLabelLen--) *diagsData++ = *diagsLabel++;
        *diagsData++ = ':';
        while (diagsValueLen--) *diagsData++ = *diagsWString++;
        *diagsData++ = '[';
        *diagsData++ = '|';
        *diagsData++ = '|';
        *diagsData++ = ']';
    }
    else
    {
        ASSERT(false); // data buffer too small
    }
    DataPtr = diagsData;
}

//",diagsLabel:diagsString"
void IDiagsEvent::DiagsLogPipeId(const WCHAR* diagsLabel, uint32 pipeIdN)
{
    if (pipeIdN == 0)
        return;

    char s[64];
    const char* diagsString;
    if (pipeIdN == kDiagsPipeIdN_FullScreen)
    {
        diagsString = "FULLSCREEN";
    }
    else
    {
        s[0] = (pipeIdN >> 24) & 0xFF;
        s[1] = (pipeIdN >> 16) & 0xFF;
        s[2] = (pipeIdN >>  8) & 0xFF;
        s[3] = (pipeIdN      ) & 0xFF;
        s[4] = 0;
        diagsString = s;
    }

    WCHAR* diagsData     = DataPtr;
    int    diagsLabelLen = wcslen(diagsLabel);
    int    diagsValueLen = strlen(diagsString);
    if (diagsData + diagsLabelLen + diagsValueLen + 5 < DataEnd)
    {
        while (diagsLabelLen--) *diagsData++ = *diagsLabel++;
        *diagsData++ = ':';
        while (diagsValueLen--) *diagsData++ = *diagsString++;
        *diagsData++ = '[';
        *diagsData++ = '|';
        *diagsData++ = '|';
        *diagsData++ = ']';
    }
    else
    {
        ASSERT(false); // data buffer too small
    }
    DataPtr = diagsData;
}


// ===============================================================================================================
//
// CGenericDiagEvent - generic IDiagsEvent for building events on-the-fly via ReportDiagEvent
//
// ===============================================================================================================

class CGenericDiagEvent
    : public IDiagsEvent
{
public:

    CGenericDiagEvent()
        : m_prgRecords( NULL )
        , m_cRecords( 0 )
        , m_eEventType( kDiagsEvent_Unknown )
    {
    }

    ~CGenericDiagEvent()
    {
        SAFE_DELETE( m_prgRecords );
        m_cRecords = 0;
    }

    pkRESULT Initialize(
                _In_ kDiagsEvent eEventType,
                _In_ DIAG_EVENT_LABEL pszEventLabel,
                _In_count_(cRecords) const DIAG_EVENT_RECORD* prgRecords,
                _In_ size_t cRecords )
    {
        ASSERT( m_prgRecords == NULL && m_cRecords == 0 );

        pkRESULT pkResult = pkS_OK;
        byte* pbRawBuffer = NULL;
        size_t cbRawBuffer = 0;

        const size_t alignment = sizeof(uint32); // for safety let's align at a 32-bit boundary

        //
        // Compute the size to copy the records
        //

        size_t cbTotal = sizeof(DIAG_EVENT_RECORD) * cRecords;

        if( cbTotal / sizeof(DIAG_EVENT_RECORD) != cRecords )
        {
            pkResult = pkE_INVALIDARG;   // overflow
            goto exit;
        }

        for( size_t i = 0; i < cRecords; ++i )
        {
            size_t cb = ( prgRecords[i].cbData + alignment - 1 ) / alignment * alignment;

            cb += cbTotal;

            if( cb < cbTotal )
            {
                pkResult = pkE_INVALIDARG;   // overflow
                goto exit;
            }

            cbTotal = cb;
        }

        //
        // Allocate and copy the records
        //

        m_prgRecords = reinterpret_cast<DIAG_EVENT_RECORD*>( NEW_NO_THROW byte[ cbTotal ] );
        if( NULL == m_prgRecords )
        {
            pkResult = pkE_OUTOFMEMORY;
            goto exit;
        }

        m_cRecords = cRecords;

        pbRawBuffer = reinterpret_cast<byte*>( m_prgRecords + m_cRecords );
        cbRawBuffer = cbTotal - sizeof(DIAG_EVENT_RECORD) * cRecords;

        for( size_t i = 0; i < cRecords; ++i )
        {
            size_t cbData = prgRecords[i].cbData;

            ASSERT( cbData <= cbRawBuffer );

            m_prgRecords[i].pszLabel = prgRecords[i].pszLabel;
            m_prgRecords[i].type = prgRecords[i].type;

            memcpy_s( pbRawBuffer, cbRawBuffer, prgRecords[i].pData, cbData );

            m_prgRecords[i].pData = pbRawBuffer;
            m_prgRecords[i].cbData = cbData;

            cbData = ( cbData + alignment - 1 ) / alignment * alignment;

            pbRawBuffer += cbData;
            cbRawBuffer -= cbData;
        }

        //
        // Set up the type of event
        //

        m_eEventType = eEventType;
        m_pszEventLabel = pszEventLabel;

    exit:

        if( pkFAILED(pkResult) )
        {
            SAFE_DELETE( m_prgRecords );
            m_cRecords = 0;
        }

        return( pkResult );
    }

public:

    //
    // IDiagsEvent implementation
    //

    __override void DiagsGetEventData(void)
    {
        for( size_t i = 0; i < m_cRecords; ++i )
        {
            DIAG_EVENT_RECORD& r = m_prgRecords[i];

            switch( r.type )
            {
            case eDiagRecordPtr:
                    DiagsLogValue( r.pszLabel, *reinterpret_cast<const int*>( r.pData ), true );
                    break;

            case eDiagRecordInt32:
            case eDiagRecordUInt32:
                    DiagsLogValue( r.pszLabel, *reinterpret_cast<const int*>( r.pData ), false );
                    break;

            case eDiagRecordInt64:
            case eDiagRecordUInt64:
                    DiagsLogValue64( r.pszLabel, *reinterpret_cast<const int64*>( r.pData ) );
                    break;

            case eDiagRecordStr:
                    DiagsLogString( r.pszLabel, reinterpret_cast<const char*>( r.pData ) );
                    break;

            case eDiagRecordWStr:
                    DiagsLogWString( r.pszLabel, reinterpret_cast<const wchar_t*>( r.pData ) );
                    break;
            default:
                ASSERT(FALSE);
                break;
            }
        }
    }

    __override const WCHAR* DiagsGetEventMessage(void)
    {
        return( m_pszEventLabel );
    }

    __override kDiagsEvent DiagsGetEventType(void)
    {
        return( m_eEventType );
    }

    __override bool DiagsIsError(void)
    {
        return( false );
    }

    __override uint16 DiagsSerializeEventDataLength(byte version)
    {
        // Not used in this version
        ASSERT( FALSE );
        return 0;
    }

    __override void DiagsSerializeEventData(byte*& data)
    {
        // Not used in this version
        ASSERT( FALSE );
    }

    __override bool DiagsDeSerializeEventData(byte version, byte*& data)
    {
        // Not used in this version
        ASSERT( FALSE );
        return( false );
    }

private:

    DIAG_EVENT_RECORD* m_prgRecords;
    size_t m_cRecords;
    DIAG_EVENT_LABEL m_pszEventLabel;
    kDiagsEvent m_eEventType;
};


// ===============================================================================================================
//
// ReportDiagEvent - auxilary to diagnostic reporting functions
//
// ===============================================================================================================

extern void ReportDiagEvent(
    _In_ kDiagsEvent eEventType,
    _In_ DIAG_EVENT_LABEL pszEventLabel,
    _In_count_(cRecords) const DIAG_EVENT_RECORD* prgRecords,
    _In_ size_t cRecords )
{
    pkRESULT pkResult = pkS_OK;
    IAVManager* pAVM = IAVManager::Instance();
    IDiagsManager* pDM = ( pAVM != NULL ) ? pAVM->GetDiagsManager() : NULL;

    if( pDM != NULL )
    {
        CGenericDiagEvent* pEvent = NEW_NO_THROW CGenericDiagEvent();

        if( pEvent != NULL )
        {
            pkResult = pEvent->Initialize( eEventType, pszEventLabel, prgRecords, cRecords );
            if( pkSUCCEEDED(pkResult) )
            {
                pDM->PostEvent( pEvent );
            }
        }
    }
}

//
// Define the array of flags that tell if a giving diagnostic channel is disabled.
// It is used by the diagnostic event reporting macros to call (or not) the
// reporting functions.
//

int g_rgDiagChannelPriorities[kDiagChannel_CountOf] = { 0 };

extern void DiagSetChannelPriority(
    _In_ DiagsChannel channel,
    _In_ DiagsPriority priority )
{
    ASSERT( 0 <= channel && (size_t)channel < sizeof(g_rgDiagChannelPriorities)/sizeof(g_rgDiagChannelPriorities[0]) );

    g_rgDiagChannelPriorities[channel] = priority;
}

// ===============================================================================================================
// ===============================================================================================================
