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

#pragma once

#include <string>
#include <AutoRefPtr.h>

////////////////////////////////////////////////////////////////////////////////
//
// XML read-only mini DOM: Auxiliary classes for reading a XML document.
//
////////////////////////////////////////////////////////////////////////////////

class CXMLDocument;
class CXMLElement;
class CXMLAttribute;
class CXMLElementsList;
class CXMLAttributesList;

////////////////////////////////////////////////////////////////////////////////
//
// Access to Test Data
//
////////////////////////////////////////////////////////////////////////////////

//
// Sets the global test data source file. This is a XML file containing test data.
// The basic format is:
//
//  <tables>
//      ...
//      <table name="{name}" [ srcFile="{filename}" ] >
//      ...
//      </table>
//      ...
//  </tables>
//
//  Where:
//      {name}:     The name of the table
//      {srcFile}:  Optional attribute to specify the table is in a different
//                  file.
//
pkRESULT PKTest_SetGlobalTestDataSourceFile( _In_ const char* pszDataSourceFileName );

//
// Returns the XML element containing the <table> data from the data source.
//
CXMLElement PKTest_GetDataTable( _In_ const char* pszTableName );


////////////////////////////////////////////////////////////////////////////////
//
// String conversion utils
//
////////////////////////////////////////////////////////////////////////////////

std::string wstring_to_string( _In_ const wchar_t* psz );
std::wstring string_to_wstring( _In_ const char* psz );


////////////////////////////////////////////////////////////////////////////////
//
// XML mini DOM auxiliary elements
//
////////////////////////////////////////////////////////////////////////////////

//
// IXMLMemHolder: holds a XML tree in memory. When the last reference is
// released, the object tears down the tree and frees the memory of each
// XML node. This way one can hold the tree in memory by a single reference
// instead of one reference per node. It works well for static read-only
// trees.
//

struct IXMLMemHolder
{
    virtual void AddRef() = 0;
    virtual void Release() = 0;
};

//
// Internal XML attribute and XML element data
//

struct XML_ATTRIBUTE_DATA
{
    const wchar_t* _pszName;
    const wchar_t* _pszValue;
};

struct XML_ELEMENT_DATA
{
    XML_ELEMENT_DATA* _pParent;
    XML_ELEMENT_DATA* _pNextSibling;
    XML_ELEMENT_DATA* _pPrevSibling;
    XML_ELEMENT_DATA* _pFirstChild;
    XML_ELEMENT_DATA* _pLastChild;
    XML_ATTRIBUTE_DATA* _pAttributes;
    size_t _cAttributes;
    const wchar_t* _pszName;
};

////////////////////////////////////////////////////////////////////////////////
//
// CXMLDocument
//
////////////////////////////////////////////////////////////////////////////////

class CXMLDocument
{
public:

    CXMLDocument();

    CXMLDocument( const CXMLDocument& rThat )
        : m_apHolder( rThat.m_apHolder )
        , m_pRootElement( rThat.m_pRootElement )
    {
    }

    ~CXMLDocument()
    {
    }

    CXMLDocument& operator = ( const CXMLDocument& rThat );

    CXMLElement RootElement();

    pkRESULT LoadFromBuffer(
                _In_count_(cchBuffer) const char* pchBuffer,
                _In_ size_t cchBuffer
                );

    pkRESULT LoadFromFile(
                _In_ const char* pszFileName
                );

private:

    AutoRefPtr<IXMLMemHolder> m_apHolder;
    const XML_ELEMENT_DATA* m_pRootElement;
};

////////////////////////////////////////////////////////////////////////////////
//
// CXMLElement
//
////////////////////////////////////////////////////////////////////////////////

class CXMLElement
{
public:

    CXMLElement()
        : m_apHolder( NULL )
        , m_pData( &s_NilElementData )
    {
    }

    CXMLElement( const CXMLElement& rThat )
        : m_apHolder( rThat.m_apHolder )
        , m_pData( rThat.m_pData )
    {
    }

    ~CXMLElement()
    {
        m_pData = NULL;
    }

    CXMLElement& operator = ( const CXMLElement& rThat );

    bool IsNull() const         { return( m_pData == &s_NilElementData ); }
    const wchar_t* Name() const   { return( m_pData->_pszName ); }

    CXMLAttributesList Attributes() const;

    CXMLElementsList Elements() const;

    CXMLElementsList Elements( _In_ const wchar_t* pszName ) const;

private:

    friend class CXMLElementsList;
    friend class CXMLDocument;

    AutoRefPtr<IXMLMemHolder> m_apHolder;
    const XML_ELEMENT_DATA* m_pData;

    static const XML_ELEMENT_DATA s_NilElementData;

    CXMLElement(
        _In_ IXMLMemHolder* pHolder,
        _In_ const XML_ELEMENT_DATA* pData )
        : m_apHolder( pHolder )
        , m_pData( pData )
    {
    }
};


////////////////////////////////////////////////////////////////////////////////
//
// CXMLAttribute
//
////////////////////////////////////////////////////////////////////////////////

class CXMLAttribute
{
public:

    CXMLAttribute()
        : m_apHolder( NULL )
        , m_pData( &s_NilAttributeData )
    {
    }

    CXMLAttribute( const CXMLAttribute& rThat )
        : m_apHolder( rThat.m_apHolder )
        , m_pData( rThat.m_pData )
    {
    }

    ~CXMLAttribute()
    {
        m_pData = NULL;
    }

    CXMLAttribute& operator = ( const CXMLAttribute& rThat );

    bool IsNull() const         { return( m_pData == &s_NilAttributeData ); }
    const wchar_t* Name() const   { return( m_pData->_pszName ); }
    const wchar_t* Value() const  { return( m_pData->_pszValue ); }

private:

    AutoRefPtr<IXMLMemHolder> m_apHolder;
    const XML_ATTRIBUTE_DATA* m_pData;

    static const XML_ATTRIBUTE_DATA s_NilAttributeData;

    friend class CXMLAttributesList;

    CXMLAttribute(
            _In_ IXMLMemHolder* pHolder,
            _In_ const XML_ATTRIBUTE_DATA* pData )
            : m_apHolder( pHolder )
            , m_pData( pData )
    {
    }
};

////////////////////////////////////////////////////////////////////////////////
//
// CXMLElementsList
//
////////////////////////////////////////////////////////////////////////////////

class CXMLElementsList
{
public:

    CXMLElementsList()
        : m_apHolder( NULL )
        , m_pFirstElement( NULL )
        , m_pCurrentElement( NULL )
        , m_iCurrentElement( 0 )
        , m_cElements( 0 )
        , m_strName( )
        , m_fMatchName(false)
    {
    }

    CXMLElementsList( const CXMLElementsList& rThat );

    ~CXMLElementsList()
    {
    }

    CXMLElementsList& operator = ( const CXMLElementsList& rThat );

    int Length() const { return( m_cElements ); }

    CXMLElement operator[]( int i );

private:

    friend class CXMLElement;

    AutoRefPtr<IXMLMemHolder> m_apHolder;
    const XML_ELEMENT_DATA* m_pFirstElement;
    const XML_ELEMENT_DATA* m_pCurrentElement;
    int m_iCurrentElement;
    int m_cElements;
    std::wstring m_strName;
    bool m_fMatchName;

    CXMLElementsList(
        _In_ IXMLMemHolder* pHolder,
        _In_ const XML_ELEMENT_DATA* pFirstElement );

    CXMLElementsList(
        _In_ IXMLMemHolder* pHolder,
        _In_ const wchar_t* pszName,
        _In_ const XML_ELEMENT_DATA* pFirstElement );

    static
    int CountElements(
            _In_ const XML_ELEMENT_DATA* pFirstElement );

    static
    int CountMatchingElements(
            _In_ const XML_ELEMENT_DATA* pFirstElement,
            _In_ const wchar_t* pszName );
};

////////////////////////////////////////////////////////////////////////////////
//
// CXMLAttributesList
//
////////////////////////////////////////////////////////////////////////////////

class CXMLAttributesList
{
public:

    CXMLAttributesList()
        : m_apHolder( NULL )
        , m_prgAttributes( NULL )
        , m_cAttributes( 0 )
    {
    }

    CXMLAttributesList( const CXMLAttributesList& rThat )
        : m_apHolder( rThat.m_apHolder )
        , m_prgAttributes( rThat.m_prgAttributes )
        , m_cAttributes( rThat.m_cAttributes )
    {
    }

    ~CXMLAttributesList()
    {
    }

    CXMLAttributesList& operator = ( const CXMLAttributesList& rThat );

    int Length() const
    {
        return( m_cAttributes );
    }

    CXMLAttribute operator[]( _In_ const wchar_t* pszAttributeName ) const;

    CXMLAttribute operator[]( int i ) const;

private:

    friend class CXMLElement;

    AutoRefPtr<IXMLMemHolder> m_apHolder;
    const XML_ATTRIBUTE_DATA* m_prgAttributes;
    size_t m_cAttributes;

    CXMLAttributesList(
        _In_ IXMLMemHolder* pHolder,
        _In_ const XML_ATTRIBUTE_DATA* prgAttributes,
        _In_ size_t cAttributes )
        : m_apHolder( pHolder )
        , m_prgAttributes( prgAttributes )
        , m_cAttributes( cAttributes )
    {
    }
};


////////////////////////////////////////////////////////////////////////////////
//
// CContiguousBufferBuilder:
//
//  Auxiliary "buffer builder" class to allocate a dynamic size structure in
//  a single buffer of memory. It works in a 2-pass fashion. The first pass
//  computes the required size and the second pass builds the buffer.
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
class CContiguousBufferBuilder
{
public:

    CContiguousBufferBuilder()
        : m_pb( NULL )
        , m_iOffset( 0 )
        , m_cbTotal( 0 )
    {
    }

    ~CContiguousBufferBuilder()
    {
        delete [] m_pb;
    }

    char* DetachBuffer()
    {
        char* pb = m_pb;
        m_pb = NULL;
        m_iOffset = 0;
        m_cbTotal = 0;
        return( pb );
    }

    pkRESULT Append(
            _In_ size_t cb,
            _Deref_out_opt_ void** ppb );

    pkRESULT AppendMultiple(
            _In_ size_t cElems,
            _In_ size_t cbElem,
            _Deref_out_opt_ void** ppb );

    pkRESULT AppendCopy(
            _In_count_(cbData) const void* pbData,
            _In_ size_t cbData,
            _Deref_out_opt_ void** ppb );

    pkRESULT Loop();

private:

    char* m_pb;
    size_t m_iOffset;
    size_t m_cbTotal;
    pkRESULT m_hrStatus;
};


////////////////////////////////////////////////////////////////////////////////
//
// CTextFileReader
//
////////////////////////////////////////////////////////////////////////////////

class CTextFileReader
{
public:

    CTextFileReader()
        : m_hFile( NULL )
    {
    }

    ~CTextFileReader()
    {
        Close();
    }

    pkRESULT OpenExisting( _In_ const char* pszFileName );

    void Close();

    pkRESULT Read(
            _Out_cap_(cbMaxDst) uint8_t* pbDst,
            _In_ uint32_t cbMaxDst,
            _Out_ uint32_t* pcbUsed );

    pkRESULT ReadLine(
            _Out_cap_(cbMaxDst) char* pbDst,
            _In_ uint32_t cbMaxDst,
            _Out_ uint32_t* pcbUsed );

    HANDLE AsHandle()
    {
        return( reinterpret_cast<HANDLE>( this ) );
    }

    static void ReadCallback(
        _In_ HANDLE hStream,
        _Out_cap_(cbMaxDst) uint8_t* pbDst,
        _In_ uint32_t cbMaxDst,
        _Out_ uint32_t* pcbUsed )
    {
        CTextFileReader* pThis = reinterpret_cast<CTextFileReader*>( hStream );
        pThis->Read( pbDst, cbMaxDst, pcbUsed );
    }

private:

    pkHANDLE m_hFile;

    CTextFileReader( const CTextFileReader& );
    void operator = ( const CTextFileReader& );
};


////////////////////////////////////////////////////////////////////////////////
//
// CTextFileWriter
//
////////////////////////////////////////////////////////////////////////////////

class CTextFileWriter
{
public:

    CTextFileWriter()
        : m_hFile( NULL )
    {
    }

    ~CTextFileWriter()
    {
        Close();
    }

    pkRESULT CreateAlways( _In_ const char* pszFileName );

    void Close();

    pkRESULT Write( _In_ const char *psz );

    pkRESULT WriteFormat( _In_ const char* pszFmt, ... );

private:

    pkHANDLE m_hFile;

    CTextFileWriter( const CTextFileWriter& );
    void operator = ( const CTextFileWriter& );
};


////////////////////////////////////////////////////////////////////////////////
//
// File Utils
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// FilesAreEqual - checks that 2 files are equal byte by byte
// Returns:
//      S_OK        : files are equal
//      S_FALSE     : files don't match
//      FAILED(hr)  : failure opening or reading one or both files
//
HRESULT FilesAreEqual(
    _In_ const char* pszOutputFileName,
    _In_ const char* pszBaselineFileName );

