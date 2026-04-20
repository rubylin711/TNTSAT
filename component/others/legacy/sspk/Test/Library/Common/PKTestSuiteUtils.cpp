///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////////

#include <SSPKDefines.h>
#include <PKTestSuiteUtils.h>
#include <pkTestFramework.h>
#include <CXmlParser.h>
#include <strsafe.h>

////////////////////////////////////////////////////////////////////////////////
//
// Access to Test Data
//
////////////////////////////////////////////////////////////////////////////////

static CXMLDocument g_xGlobalTestDataSource;

////////////////////////////////////////////////////////////////////////////////
pkRESULT PKTest_SetGlobalTestDataSourceFile(
        _In_ const char* pszDataSourceFileName )
{
    return( g_xGlobalTestDataSource.LoadFromFile( pszDataSourceFileName ) );
}

////////////////////////////////////////////////////////////////////////////////
CXMLElement PKTest_GetDataTable( _In_ const char* pszTableName )
{
    pkRESULT hr = pkS_OK;

    const int MAX_HOOPS = 2;

    std::wstring strTableName = string_to_wstring( pszTableName );

    //
    // Start looking for the table at the global data source file.
    // If the <table> element has a srcFile attribute, redirect the
    // search to the new file.
    //

    CXMLDocument doc = g_xGlobalTestDataSource;

    for( int cHoops = 0; cHoops < MAX_HOOPS; ++cHoops )
    {
        //
        // Looking in all <table> elements in the current XML document
        // for the first one whose name attribute matches the requested
        // table
        //

        CXMLElementsList xElements = doc.RootElement().Elements(L"table");

        for( int i = 0; i < xElements.Length(); ++i )
        {
            CXMLElement xTable = xElements[i];

            CXMLAttribute xName = xTable.Attributes()[L"name"];

            if( xName.IsNull()
                || strTableName.compare( xName.Value() ) != 0 )
            {
                continue;
            }

            CXMLAttribute xSrcFile = xTable.Attributes()[L"srcFile"];

            if( !xSrcFile.IsNull() )
            {
                //
                // If the <table> element has a srcFile attribute,
                // search for the table in that file instead
                //

                if( cHoops < ( MAX_HOOPS - 1 ) )
                {
                    hr = doc.LoadFromFile( wstring_to_string( xSrcFile.Value() ).c_str() );
                    if( FAILED(hr) )
                    {
                        goto exit;
                    }
                }

                break;  // redirect to the new data source file
            }

            return( xTable );   // return the <table> element
        }
    }

exit:

    // Could not load the file
    // Return an empty table
    return( CXMLElement() );
}

////////////////////////////////////////////////////////////////////////////////
//
// String conversion utils
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
std::string wstring_to_string( _In_ const wchar_t* psz )
{
    std::string strOut;
    size_t cch = wcslen( psz );

    strOut.resize( cch );

    for( size_t i = 0; i < cch; ++i )
    {
        strOut[i] = (char)psz[i];
    }

    return( strOut );
}

////////////////////////////////////////////////////////////////////////////////
std::wstring string_to_wstring( _In_ const char* psz )
{
    std::wstring strOut;
    size_t cch = strlen( psz );

    strOut.resize( cch );

    for( size_t i = 0; i < cch; ++i )
    {
        strOut[i] = (wchar_t)psz[i];
    }

    return( strOut );
}

////////////////////////////////////////////////////////////////////////////////
//
// XML read-only mini DOM
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

class CXMLMemHolderImpl
    : public IXMLMemHolder
{
public:

    CXMLMemHolderImpl()
        : m_cRefs( 1 )
    {
        memset( &m_DocElement, 0, sizeof(m_DocElement) );
    }

    //
    // IXMLMemHolder
    //

    virtual void AddRef()
    {
        InterlockedIncrement( &m_cRefs );
    }

    virtual void Release()
    {
        if( 0 == InterlockedDecrement( &m_cRefs ) )
        {
            delete this;
        }
    }

    //
    // Public attributes
    //

    XML_ELEMENT_DATA m_DocElement;

private:

    long m_cRefs;

    ~CXMLMemHolderImpl()
    {
        _DeleteElementListRecursive( m_DocElement._pFirstChild );
    }

    void _DeleteElementListRecursive( _In_ XML_ELEMENT_DATA* pElement )
    {
        while( pElement != NULL )
        {
            XML_ELEMENT_DATA* pElementToDelete = pElement;

            pElement = pElement->_pNextSibling;

            _DeleteElementListRecursive( pElementToDelete->_pFirstChild );

            delete pElementToDelete;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////

class CXMLBuilder
    : public ISAXCallback
{
public:

    AutoRefPtr<CXMLMemHolderImpl> m_apHolder;
    XML_ELEMENT_DATA* m_pCurrentElement;

    CXMLBuilder()
        : m_apHolder( NULL )
        , m_pCurrentElement( NULL )
    {
    }

    ~CXMLBuilder()
    {
    }

private:

    pkRESULT PushElement(
            _In_count_(cchName) const wchar_t* pchName,
            _In_ size_t cchName,
            _In_count_(prgAttributes) const XMLAttribute* prgAttributes,
            _In_ size_t cAttributes )
    {
        pkRESULT hr = pkS_OK;
        XML_ELEMENT_DATA* pElement = NULL;
        XML_ELEMENT_DATA dummyElement;
        XML_ATTRIBUTE_DATA dummyAttribute;

        CContiguousBufferBuilder builder;

        do
        {
            //
            // In theory the following cleanup would be irrelevant for the correctness of the
            // code given the other cleanup a few lines below, yet GCC 4.4 has trouble expanding
            // this loop when optimization level 3 (more specifically the -finline-functions option)
            // is on, and it skips the other cleanup in the second pass of the loop. It seems this
            // happens because the compiler decided to reuse the same memory for all passes of the loop
            // and consolidate it to the final buffer only at the end of the loop, but then it incorrectly
            // assumes the memory hasn't been changed for the second pass of the loop (it definitely has)
            // and doesn't clean it up again. The following forces the cleanup to happen in the second pass.
            //
            // This problem does not happen with the MS compiler, even when all optimizations are on, neither
            // does it happen for GCC for optimizations level 2 or lower (no version later than 4.4 has been
            // checked though).
            //

            memset( &dummyElement, 0, sizeof(dummyElement) );

            hr = builder.Append( sizeof(XML_ELEMENT_DATA), (void**)&pElement );
            if( FAILED(hr) )
            {
                goto exit;
            }

            if( pElement == NULL )
            {
                pElement = &dummyElement;
            }

            memset( pElement, 0, sizeof(XML_ELEMENT_DATA) );

            hr = builder.AppendCopy( pchName, cchName * sizeof(wchar_t), (void**)&pElement->_pszName );
            if( FAILED(hr) )
            {
                goto exit;
            }

            hr = builder.AppendCopy( L"", sizeof(L""), NULL );
            if( FAILED(hr) )
            {
                goto exit;
            }

            hr = builder.AppendMultiple( cAttributes, sizeof(XML_ATTRIBUTE_DATA), (void**)&pElement->_pAttributes );
            if( FAILED(hr) )
            {
                goto exit;
            }

            pElement->_cAttributes = cAttributes;

            for( size_t i = 0; i < cAttributes; ++i )
            {
                XML_ATTRIBUTE_DATA* pAttribute = ( pElement->_pAttributes != NULL ) ? &pElement->_pAttributes[i] : &dummyAttribute;

                hr = builder.AppendCopy(
                                prgAttributes[i].strName,
                                prgAttributes[i].NameLen * sizeof(wchar_t),
                                (void**)&pAttribute->_pszName );
                if( FAILED(hr) )
                {
                    goto exit;
                }

                hr = builder.AppendCopy( L"", sizeof(L""), NULL );
                if( FAILED(hr) )
                {
                    goto exit;
                }

                builder.AppendCopy(
                            prgAttributes[i].strValue,
                            prgAttributes[i].ValueLen * sizeof(wchar_t),
                            (void**)&pAttribute->_pszValue );
                if( FAILED(hr) )
                {
                    goto exit;
                }

                hr = builder.AppendCopy( L"", sizeof(L""), NULL );
                if( FAILED(hr) )
                {
                    goto exit;
                }
            }
        }
        while( ( hr = builder.Loop() ) == pkS_OK );

        if( FAILED(hr) )
        {
            goto exit;
        }

        builder.DetachBuffer();

        pElement->_pParent = m_pCurrentElement;

        if( m_pCurrentElement->_pLastChild == NULL )
        {
            pElement->_pNextSibling = NULL;
            pElement->_pPrevSibling = NULL;

            m_pCurrentElement->_pFirstChild = pElement;
            m_pCurrentElement->_pLastChild = pElement;
        }
        else
        {
            pElement->_pNextSibling = NULL;
            pElement->_pPrevSibling = m_pCurrentElement->_pLastChild;

            m_pCurrentElement->_pLastChild->_pNextSibling = pElement;
            m_pCurrentElement->_pLastChild = pElement;
        }

        m_pCurrentElement = pElement;

    exit:

        return( hr );
    }

    pkRESULT PopElement(
            _In_count_(cchName) const wchar_t* pchName,
            _In_ size_t cchName )
    {
        pkRESULT hr = pkS_OK;
        const wchar_t* pszCheckName = NULL;

        if( m_pCurrentElement->_pParent == NULL )
        {
            hr = pkE_INVALID_XML_SYNTAX;
            goto exit;
        }

        pszCheckName = m_pCurrentElement->_pszName;

        while( ( cchName > 0 ) && ( *pszCheckName == *pchName ) )
        {
            ++pszCheckName;
            ++pchName;
            --cchName;
        }

        if( *pszCheckName != L'\0' )
        {
            hr = pkE_INVALID_XML_SYNTAX;
            goto exit;
        }

        m_pCurrentElement = m_pCurrentElement->_pParent;

    exit:

        return( hr );
    }

protected:

    //
    // ISAXCallback
    //

    virtual pkRESULT  StartDocument()
    {
        pkRESULT hr = pkS_OK;

        if( m_apHolder != NULL )
        {
            hr = pkE_UNEXPECTED;
            goto exit;
        }

        m_apHolder.AdoptRef( new CXMLMemHolderImpl() );
        if( m_apHolder == NULL )
        {
            hr = pkE_OUTOFMEMORY;
            goto exit;
        }

        m_pCurrentElement = &m_apHolder->m_DocElement;

    exit:
        return( hr );
    }

    virtual pkRESULT  EndDocument()
    {
        return( pkS_OK );
    }

    virtual pkRESULT  ElementBegin( const wchar_t *strName, uint32 NameLen, const XMLAttribute *pAttributes, uint32 NumAttributes )
    {
        return( PushElement( strName, NameLen, pAttributes, NumAttributes ) );
    }

    virtual pkRESULT  ElementContent( const wchar_t *strData, uint32 DataLen, bool More )
    {
        return( pkS_OK );
    }

    virtual pkRESULT  ElementEnd( const wchar_t *strName, uint32 NameLen )
    {
        return( PopElement( strName, NameLen ) );
    }

    virtual pkRESULT  CDATABegin( )
    {
        return( pkS_OK );
    }

    virtual pkRESULT  CDATAData( const wchar_t *strCDATA, uint32 CDATALen, bool bMore )
    {
        return( pkS_OK );
    }

    virtual pkRESULT  CDATAEnd( )
    {
        return( pkS_OK );
    }

    virtual pkRESULT Error( pkRESULT hError, const char *strMessage, ... )
    {
        return( hError );
    }
};

////////////////////////////////////////////////////////////////////////////////
//
// CXMLDocument implementation
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
CXMLDocument::CXMLDocument()
    : m_pRootElement( &CXMLElement::s_NilElementData )
{
}

////////////////////////////////////////////////////////////////////////////////
CXMLDocument& CXMLDocument::operator = ( const CXMLDocument& rThat )
{
    m_apHolder = rThat.m_apHolder;
    m_pRootElement = rThat.m_pRootElement;

    return( *this );
}

////////////////////////////////////////////////////////////////////////////////
CXMLElement CXMLDocument::RootElement()
{
    return( CXMLElement( m_apHolder, m_pRootElement ) );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CXMLDocument::LoadFromBuffer(
    _In_count_(cchBuffer) const char* pchBuffer,
    _In_ size_t cchBuffer
    )
{
    pkRESULT hr = pkS_OK;

    CXMLBuilder builder;
    CXmlParser parser;

    m_apHolder.Release();
    m_pRootElement = NULL;

    parser.RegisterSAXCallbackInterface( &builder );

    hr = parser.ParseXMLBuffer( pchBuffer, cchBuffer );
    if( FAILED(hr) )
    {
        goto exit;
    }

    //
    // Adopt the data from the builder
    //

    m_pRootElement = builder.m_apHolder->m_DocElement._pFirstChild;
    m_apHolder.AdoptRef( builder.m_apHolder.HandOffRef() );

exit:

    return( hr );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CXMLDocument::LoadFromFile(
    _In_ const char* pszFileName
    )
{
    pkRESULT hr = pkS_OK;

    CTextFileReader reader;
    CXMLBuilder builder;
    CXmlParser parser;

    m_apHolder.Release();
    m_pRootElement = NULL;

    hr = reader.OpenExisting( pszFileName );
    if( FAILED(hr) )
    {
        goto exit;
    }

    parser.RegisterSAXCallbackInterface( &builder );

    hr = parser.ParseXMLStream( reader.AsHandle(), reader.ReadCallback );
    if( FAILED(hr) )
    {
        goto exit;
    }

    //
    // Adopt the data from the builder
    //

    m_pRootElement = builder.m_apHolder->m_DocElement._pFirstChild;
    m_apHolder.AdoptRef( builder.m_apHolder.HandOffRef() );

exit:

    return( hr );
}

////////////////////////////////////////////////////////////////////////////////
//
// CXMLAttribute implementation
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
const XML_ATTRIBUTE_DATA CXMLAttribute::s_NilAttributeData =
{
    L"",
    L"",
};

////////////////////////////////////////////////////////////////////////////////
CXMLAttribute& CXMLAttribute::operator = ( const CXMLAttribute& rThat )
{
    m_apHolder = rThat.m_apHolder;
    m_pData = rThat.m_pData;

    return( *this );
}

////////////////////////////////////////////////////////////////////////////////
//
// CXMLElementsList implementation
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
CXMLElementsList::CXMLElementsList( const CXMLElementsList& rThat )
    : m_apHolder( rThat.m_apHolder )
    , m_pFirstElement( rThat.m_pFirstElement )
    , m_pCurrentElement( rThat.m_pCurrentElement )
    , m_iCurrentElement( rThat.m_iCurrentElement )
    , m_cElements( rThat.m_cElements )
    , m_strName( rThat.m_strName )
    , m_fMatchName( rThat.m_fMatchName )
{
}

////////////////////////////////////////////////////////////////////////////////
CXMLElementsList::CXMLElementsList(
    _In_ IXMLMemHolder* pHolder,
    _In_ const XML_ELEMENT_DATA* pFirstElement )
    : m_apHolder( pHolder )
    , m_pFirstElement( pFirstElement )
    , m_pCurrentElement( pFirstElement )
    , m_iCurrentElement( 0 )
    , m_cElements( CountElements( pFirstElement ) )
    , m_strName( )
    , m_fMatchName( false )
{
}

////////////////////////////////////////////////////////////////////////////////
CXMLElementsList::CXMLElementsList(
    _In_ IXMLMemHolder* pHolder,
    _In_ const wchar_t* pszName,
    _In_ const XML_ELEMENT_DATA* pFirstElement )
    : m_apHolder( pHolder )
    , m_pFirstElement( pFirstElement )
    , m_pCurrentElement( pFirstElement )
    , m_iCurrentElement( 0 )
    , m_cElements( CountMatchingElements( pFirstElement, pszName ) )
    , m_strName( pszName )
    , m_fMatchName( true )
{
}

////////////////////////////////////////////////////////////////////////////////
CXMLElementsList& CXMLElementsList::operator = ( const CXMLElementsList& rThat )
{
    m_apHolder = rThat.m_apHolder;
    m_pFirstElement = rThat.m_pFirstElement;
    m_pCurrentElement = rThat.m_pCurrentElement;
    m_iCurrentElement = rThat.m_iCurrentElement;
    m_cElements = rThat.m_cElements;
    m_fMatchName = rThat.m_fMatchName;
    m_strName = rThat.m_strName;

    return( *this );
}

////////////////////////////////////////////////////////////////////////////////
CXMLElement CXMLElementsList::operator[]( int i )
{
    if( i < m_iCurrentElement )
    {
        m_iCurrentElement = 0;
        m_pCurrentElement = m_pFirstElement;
    }

    for(
        /**/;
        m_pCurrentElement != NULL;
        m_pCurrentElement = m_pCurrentElement->_pNextSibling
        )
    {
        if( m_fMatchName
            && m_strName.compare( m_pCurrentElement->_pszName ) != 0 )
        {
            continue;
        }

        if( m_iCurrentElement == i )
        {
            break;
        }

        ++m_iCurrentElement;
    }

    return(
        ( m_pCurrentElement != NULL )
        ? CXMLElement( m_apHolder, m_pCurrentElement )
        : CXMLElement()
        );
}

////////////////////////////////////////////////////////////////////////////////
/*static*/
int CXMLElementsList::CountElements(
    _In_ const XML_ELEMENT_DATA* pFirstElement )
{
    int cElements = 0;

    for(
        const XML_ELEMENT_DATA* pElement = pFirstElement;
        pElement != NULL;
        pElement = pElement->_pNextSibling
        )
    {
        ++cElements;
    }

    return( cElements );
}

////////////////////////////////////////////////////////////////////////////////
/*static*/
int CXMLElementsList::CountMatchingElements(
    _In_ const XML_ELEMENT_DATA* pFirstElement,
    _In_ const wchar_t* pszName )
{
    int cElements = 0;

    for(
        const XML_ELEMENT_DATA* pElement = pFirstElement;
        pElement != NULL;
        pElement = pElement->_pNextSibling
        )
    {
        if( wcscmp( pszName, pElement->_pszName ) == 0 )
        {
            ++cElements;
        }
    }

    return( cElements );
}

////////////////////////////////////////////////////////////////////////////////
//
// CXMLAttribute implementation
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
CXMLAttribute CXMLAttributesList::operator[]( _In_ const wchar_t* pszAttributeName ) const
{
    for( size_t i = 0; i < m_cAttributes; ++i )
    {
        if( wcscmp( pszAttributeName, m_prgAttributes[i]._pszName ) == 0 )
        {
            return( CXMLAttribute( m_apHolder, &m_prgAttributes[i] ) );
        }
    }

    return( CXMLAttribute() );
}

////////////////////////////////////////////////////////////////////////////////
CXMLAttribute CXMLAttributesList::operator[]( int i ) const
{
    return(
            (size_t)i < m_cAttributes
            ? CXMLAttribute( m_apHolder, &m_prgAttributes[i] )
            : CXMLAttribute()
            );
}


////////////////////////////////////////////////////////////////////////////////
//
// CXMLElement implementation
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
const XML_ELEMENT_DATA CXMLElement::s_NilElementData =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    0,
    L""
};

////////////////////////////////////////////////////////////////////////////////
CXMLElement& CXMLElement::operator = ( const CXMLElement& rThat )
{
    m_apHolder = rThat.m_apHolder;
    m_pData = rThat.m_pData;

    return( *this );
}

////////////////////////////////////////////////////////////////////////////////
CXMLAttributesList CXMLElement::Attributes() const
{
    return( CXMLAttributesList( m_apHolder, m_pData->_pAttributes, m_pData->_cAttributes ) );
}

////////////////////////////////////////////////////////////////////////////////
CXMLElementsList CXMLElement::Elements() const
{
    return( CXMLElementsList( m_apHolder, m_pData->_pFirstChild ) );
}

////////////////////////////////////////////////////////////////////////////////
CXMLElementsList CXMLElement::Elements( _In_ const wchar_t* pszName ) const
{
    return( CXMLElementsList( m_apHolder, pszName, m_pData->_pFirstChild ) );
}


////////////////////////////////////////////////////////////////////////////////
//
// CXMLAttributesList implementation
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
 CXMLAttributesList& CXMLAttributesList::operator = ( const CXMLAttributesList& rThat )
 {
    m_apHolder = rThat.m_apHolder;
    m_prgAttributes = rThat.m_prgAttributes;
    m_cAttributes = rThat.m_cAttributes;

     return( *this );
 }

////////////////////////////////////////////////////////////////////////////////
//
// CContiguousBufferBuilder
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
pkRESULT CContiguousBufferBuilder::Append(
    _In_ size_t cb,
    _Deref_opt_out_ void** ppb )
{
    pkRESULT hr = pkS_OK;
    char* pbFirst = NULL;
    void* pbDummy = NULL;
    size_t iNewOffset = m_iOffset;

    if( ppb == NULL )
    {
        ppb = &pbDummy;
    }

    *ppb = NULL;

    if( m_iOffset < m_cbTotal )
    {
        pbFirst = &m_pb[m_iOffset];
    }

    iNewOffset += cb;

    if( iNewOffset < cb )
    {
        // Overflow
        hr = pkE_INVALIDARG;
        goto exit;
    }

    m_iOffset = iNewOffset;

    if( m_iOffset <= m_cbTotal )
    {
        *ppb = pbFirst;
    }

exit:

    return( hr );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CContiguousBufferBuilder::AppendMultiple(
        _In_ size_t cElems,
        _In_ size_t cbElem,
        _Deref_opt_out_ void** ppb
        )
{
    pkRESULT hr = pkS_OK;

    size_t cbTotal = cElems * cbElem;

    if( ppb != NULL )
    {
        *ppb = NULL;
    }

    if( ( cElems != 0 ) && ( cbElem != 0 ) )
    {
        if( cbTotal / cElems != cbElem )
        {
            // Integer overflow in multiplication
            hr = pkE_INVALIDARG;
            goto exit;
        }
    }

    hr = Append( cbTotal, ppb );
    goto exit;

exit:

    return( hr );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CContiguousBufferBuilder::AppendCopy(
        _In_count_(cbData) const void* pbData,
        _In_ size_t cbData,
        _Deref_opt_out_ void** ppb
        )
{
    pkRESULT hr = pkS_OK;
    char* pbCopy = NULL;

    if( ppb != NULL )
    {
        *ppb = NULL;
    }

    hr = Append( cbData, (void**)&pbCopy );
    if( FAILED(hr) )
    {
        goto exit;
    }

    if( pbCopy != NULL )
    {
        memcpy( pbCopy, pbData, cbData );
    }

    if( ppb != NULL )
    {
        *ppb = pbCopy;
    }

exit:

    return( hr );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CContiguousBufferBuilder::Loop()
{
    pkRESULT hr = pkS_OK;

    if( m_iOffset <= m_cbTotal )
    {
        //
        // Buffer big enough, no need to another pass
        //
        hr = pkS_FALSE;
    }
    else
    {
        if( m_pb != NULL )
        {
            //
            // The caller is doing something wrong if it needs a bigger buffer
            // after the buffer was already allocated in the first pass.
            //
            pkASSERT( 0 );
            hr = pkE_UNEXPECTED;
            goto exit;
        }

        m_pb = new char[ m_iOffset ];
        if( m_pb == NULL )
        {
            hr = pkE_OUTOFMEMORY;
            goto exit;
        }

        m_cbTotal = m_iOffset;
        m_iOffset = 0;

        //
        // Buffer allocated. Pass one more time to write the data.
        //

        hr = pkS_OK;
    }

exit:

    return( hr );
}


////////////////////////////////////////////////////////////////////////////////
//
// CTextFileReader
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
pkRESULT CTextFileReader::OpenExisting( _In_ const char* pszFileName )
{
    pkRESULT hr = pkS_OK;

    Close();

    hr = TF_TestScripting_Open( &m_hFile, pszFileName );
    if( FAILED(hr) )
    {
        goto exit;
    }

exit:

    return( hr );
}

////////////////////////////////////////////////////////////////////////////////
void CTextFileReader::Close()
{
    if( m_hFile != NULL )
    {
        TF_TestScripting_Close( m_hFile );
        m_hFile = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CTextFileReader::Read(
    _Out_cap_(cbMax) uint8_t* pbDst,
    _In_ uint32 cbMaxDst,
    _Out_ uint32* pcbUsed )
{
    pkRESULT hr = pkS_OK;
    size_t cb = cbMaxDst;

    hr = TF_TestVectors_ReadBlock( m_hFile, pbDst, &cb );
    if( FAILED(hr) )
    {
        cb = 0;
    }

    *pcbUsed = (uint32)cb;

    return( hr );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CTextFileReader::ReadLine(
    _Out_cap_(cbBufSize) char* pbDst,
    _In_ uint32 cbBufSize,
    _Out_ uint32* pcbUsed )
{
    pkRESULT hr = pkS_OK;
    size_t cb = cbBufSize;

    hr = TF_TestScripting_ReadString( m_hFile, pbDst, cb );
    if( FAILED(hr) )
    {
        cb = 0;
    }

    *pcbUsed = (uint32)cb;

    return( hr );
}

////////////////////////////////////////////////////////////////////////////////
//
// CTextFileWriter
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
pkRESULT CTextFileWriter::CreateAlways( _In_ const char* pszFileName )
{
    pkRESULT hr = pkS_OK;

    Close();

    hr = TF_Logging_Open( &m_hFile, pszFileName );
    if( FAILED(hr) )
    {
        goto exit;
    }

exit:

    return( hr );

}

////////////////////////////////////////////////////////////////////////////////
void CTextFileWriter::Close()
{
    if( m_hFile != NULL )
    {
        TF_Logging_Close( m_hFile );
        m_hFile = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CTextFileWriter::Write( _In_ const char *psz )
{
    pkRESULT hr = pkS_OK;

    hr = TF_Logging_Printf( m_hFile, "%s", psz );
    if( FAILED(hr) )
    {
        goto exit;
    }

exit:

    return( hr );
}

////////////////////////////////////////////////////////////////////////////////
pkRESULT CTextFileWriter::WriteFormat( _In_ const char* pszFmt, ... )
{
    pkRESULT hr = pkS_OK;

    char szMsgBuffer[16*1024];

    va_list arglist;
    va_start(arglist,pszFmt);

    hr = StringCchVPrintfA(
            szMsgBuffer,
            sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]),
            pszFmt,
            arglist );

    va_end(arglist);

    if( FAILED(hr) )
    {
        goto exit;
    }

    // TF_Logging does not have any overload that takes a va_list

    hr = TF_Logging_Printf( m_hFile, "%s", szMsgBuffer );
    if( FAILED(hr) )
    {
        goto exit;
    }

exit:

    return( hr );
};

////////////////////////////////////////////////////////////////////////////////
//
// File Utils
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
HRESULT FilesAreEqual(
    _In_ const char* pszFileName1,
    _In_ const char* pszFileName2 )
{
    HRESULT hr = S_OK;

    struct
    {
        CTextFileReader _reader;
        uint8 _workingBuffer[1024];
        uint32 _cb;
        const char* _pszFileName;
    }
    rg[2];

    rg[0]._pszFileName = pszFileName1;
    rg[1]._pszFileName = pszFileName2;

    for( int i = 0; i < 2; ++i )
    {
        hr = rg[i]._reader.OpenExisting( rg[i]._pszFileName );
        if( FAILED(hr) )
        {
            goto exit;
        }
    }

    while( true )
    {
        bool fEof = true;

        for( int i = 0; i < 2; ++i )
        {
            hr = rg[i]._reader.Read( rg[i]._workingBuffer, sizeof(rg[i]._workingBuffer), &rg[i]._cb );
            if( FAILED(hr) )
            {
                goto exit;
            }

            if( rg[i]._cb != 0 )
            {
                fEof = false;
            }
        }

        if( fEof )
        {
            hr = S_OK;
            break;
        }

        if( ( rg[0]._cb != rg[1]._cb )
            || ( memcmp( rg[0]._workingBuffer, rg[1]._workingBuffer, rg[0]._cb ) != 0 ) )
        {
            hr = S_FALSE;
            break;
        }
    }

exit:

    return( hr );
}
