///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------
// CXmlParser.h
//
// CXmlParser and SAX interface declaration
//-------------------------------------------------------------------------------------
#pragma once

typedef void (*PFN_READCB)(HANDLE hStream, byte* pBuffer, uint32 bufferSize, uint32* pLen);
// Note: End of stream is indicated by setting *pLen to zero

const uint32 XML_MAX_ATTRIBUTES_PER_ELEMENT  =   256;
const uint32 XML_MAX_NAME_LENGTH             =   128;
const uint32 XML_READ_BUFFER_SIZE            =   2048;
const uint32 XML_WRITE_BUFFER_SIZE           =   2048;

// No tag can be longer than XML_WRITE_BUFFER_SIZE - an error will be returned if
// it is

//-------------------------------------------------------------------------------------
struct XMLAttribute
{
    WCHAR*  strName;
    uint32  NameLen;
    WCHAR*  strValue;
    uint32  ValueLen;
};

//-------------------------------------------------------------------------------------
class ISAXCallback
{
    friend class CXmlParser;
public:
    ISAXCallback() {};
    virtual ~ISAXCallback() {};

    virtual pkRESULT StartDocument() = 0;
    virtual pkRESULT EndDocument() = 0;

    virtual pkRESULT ElementBegin( const WCHAR *strName, uint32 NameLen, const XMLAttribute *pAttributes, uint32 NumAttributes ) = 0;
    virtual pkRESULT ElementContent( const WCHAR *strData, uint32 DataLen, bool More ) = 0;
    virtual pkRESULT ElementEnd( const WCHAR *strName, uint32 NameLen ) = 0;

    virtual pkRESULT CDATABegin( ) = 0;
    virtual pkRESULT CDATAData( const WCHAR *strCDATA, uint32 CDATALen, bool bMore ) = 0;
    virtual pkRESULT CDATAEnd( ) = 0;

    virtual pkRESULT Error( pkRESULT hError, const char *strMessage, ... ) = 0;

    uint32           GetLineNumber() const { return m_LineNum; }
    uint32           GetLinePosition() const { return m_LinePos; }

private:
    uint32           m_LineNum;
    uint32           m_LinePos;
};


//-------------------------------------------------------------------------------------
class CXmlParser
{
public:
    CXmlParser();
    ~CXmlParser();

    enum EEncodingType 
    {
        eEncodingType_invalid,
        eEncodingType_UTF8, // either UTF-8 BOM or appears to be 8-bit characters
        eEncodingType_UTF16LE,
        eEncodingType_UTF16BE// either LE or BE UTF-16 BOM or appears to be 16-bit characters
    };

    //      Register an interface inheiriting from ISAXCallback
    void            RegisterSAXCallbackInterface( ISAXCallback *pISAXCallback );

    //      Get the registered interface
    ISAXCallback*   GetSAXCallbackInterface();

    //      ParseXMLFile returns one of the following:
    //         pkE_COULD_NOT_OPEN_FILE - couldn't open the file
    //         pkE_INVALID_XML_SYNTAX - bad XML syntax according to this parser
    //         pkE_NOINTERFACE - RegisterSAXCallbackInterface not called
    //         pkE_ABORT - callback returned a fail code
    //         pkS_OK - file parsed and completed

    pkRESULT   ParseXMLBuffer( const char* strBuffer, uint32 uBufferSize );

    //        Parses from a stream source - pfnReadCB callback to read from the source
    pkRESULT   ParseXMLStream( HANDLE hStream, PFN_READCB pfnReadCB, _Out_ EEncodingType* pEncodingType = NULL );

private:
    pkRESULT   MainParseLoop( _Out_ EEncodingType* pEncodingType = NULL );

    pkRESULT   AdvanceCharacter( bool bOkToFail = FALSE );

    pkRESULT   ConsumeSpace();
    pkRESULT   ConvertEscape();
    pkRESULT   AdvanceElement();
    pkRESULT   AdvanceName();
    pkRESULT   AdvanceAttrVal();
    pkRESULT   AdvanceCDATA();
    pkRESULT   AdvanceComment();

    bool       FillBuffer( size_t cbMinBuffer );

    void       Error( pkRESULT hRet, const char* strFormat, ... );

    ISAXCallback*   m_pISAXCallback;

    PFN_READCB      m_pfnReadCB;
    HANDLE          m_hStream;

    const char*     m_pInXMLBuffer;
    uint32          m_uInXMLBufferCharsLeft;

    byte            m_pReadBuf[ XML_READ_BUFFER_SIZE ];
    WCHAR           m_pWriteBuf[ XML_WRITE_BUFFER_SIZE ];

    byte*           m_pReadBufTop; // points after last byte read into the buffer
    byte*           m_pReadPtr;
    WCHAR*          m_pWritePtr;   // write pointer within m_pWriteBuf

    EEncodingType   m_eEncodingType;

    bool            m_bSkipNextAdvance;
    WCHAR           m_Ch;               // Current character being parsed
};
