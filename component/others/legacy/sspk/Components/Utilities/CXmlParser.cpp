///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------
// CXmlParser.cpp
//
// Simple callback non-validating XML parser implementation.
//
// Xbox Advanced Technology Group
//-------------------------------------------------------------------------------------
#include "stdafx.h"
#include "CXmlParser.h"

#include <strsafe.h>

static uint16 ConvertBytesToUInt16(byte* firstByte, byte* secondByte)
{
    return (*secondByte << 8) + *firstByte;
}

//-------------------------------------------------------------------------------------
// Name: CXmlParser::CXmlParser
//-------------------------------------------------------------------------------------
CXmlParser::CXmlParser()
 
    : m_pISAXCallback( NULL )

    , m_pfnReadCB( NULL )
    , m_hStream( NULL )

    , m_pInXMLBuffer( NULL )
    , m_uInXMLBufferCharsLeft( 0 )

    , m_pReadBufTop( m_pReadBuf )
    , m_pReadPtr( m_pReadBuf )
    , m_pWritePtr( m_pWriteBuf )

    , m_eEncodingType( eEncodingType_invalid )

    , m_bSkipNextAdvance( false )
    , m_Ch( 0 )
{
}

//-------------------------------------------------------------------------------------
// Name: CXmlParser::~CXmlParser
//-------------------------------------------------------------------------------------
CXmlParser::~CXmlParser()
{
}


//-------------------------------------------------------------------------------------
// Name: CXmlParser::FillBuffer
// Desc: Reads from the current open file until minimum buffer is attained
// Return: True if buffer contains at least cbMinBuffer bytes
//-------------------------------------------------------------------------------------
bool CXmlParser::FillBuffer( size_t cbMinBuffer )
{
    uint32 uiBytesRead = 0; // note pfnReadCB requires uint32

    size_t cbBuffered = (m_pReadBufTop > m_pReadPtr) ? (m_pReadBufTop - m_pReadPtr) : 0;
    if (cbBuffered >= cbMinBuffer)
    {
        // Nothing to do if enough data is buffered
        return true;
    }
    
    // Move down any bytes at tail of the buffer
    {
        size_t ix = 0;
        for ( ; ix < cbBuffered; ix++ )
        {
            m_pReadBuf[ix] = m_pReadPtr[ix];
        }
        m_pReadBufTop = &m_pReadBuf[ix];
    }
    m_pReadPtr = m_pReadBuf;

    // Read the stream until cbMinBuffer is achieved or end of stream reached
    do 
    {
        if (m_pfnReadCB != NULL)
        {
            (*m_pfnReadCB)(m_hStream, m_pReadBufTop, (uint32)(sizeof(m_pReadBuf) - cbBuffered), &uiBytesRead);
            // The callback indicates end of stream by setting uiBytesRead to zero.
        }
        else
        {
            uiBytesRead = ( m_uInXMLBufferCharsLeft > (sizeof(m_pReadBuf) - cbBuffered) )
                ? sizeof(m_pReadBuf) - cbBuffered 
                : m_uInXMLBufferCharsLeft;

            memcpy_s( m_pReadBufTop, sizeof(m_pReadBuf) - cbBuffered, m_pInXMLBuffer, uiBytesRead );

            m_uInXMLBufferCharsLeft -= uiBytesRead;
            m_pInXMLBuffer += uiBytesRead;
        }
        
        cbBuffered += uiBytesRead;
        m_pReadBufTop += uiBytesRead;
    } 
    while (0 != uiBytesRead && cbBuffered < cbMinBuffer );
    
    return (cbBuffered >= cbMinBuffer);
}


//-------------------------------------------------------------------------------------
// White space detector helper function
//-------------------------------------------------------------------------------------

static bool s_IsWhiteSpace( WCHAR ch )
{
    return( (ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r') );
}

//-------------------------------------------------------------------------------------
// Name: CXmlParser::ConsumeSpace
// Desc: Skips spaces in the current stream
//-------------------------------------------------------------------------------------
pkRESULT CXmlParser::ConsumeSpace()
{
    pkRESULT pkResult;

    // Skip to next non-white-space
    do
    {
        pkResult = AdvanceCharacter();
        if( pkFAILED( pkResult ) )
        {
            return pkResult;
        }
    }
    while ( s_IsWhiteSpace(m_Ch) );
        
    m_bSkipNextAdvance = true;
    
    return pkS_OK;
}

//-------------------------------------------------------------------------------------
// Name: CXmlParser::ConvertEscape
// Desc: Copies and converts an escape sequence into m_pWriteBuf
//-------------------------------------------------------------------------------------
pkRESULT CXmlParser::ConvertEscape()
{
    pkRESULT pkResult;
    WCHAR wVal = 0;

    if( pkFAILED( pkResult = AdvanceCharacter() ) )
        return pkResult;

    // all escape sequences start with &, so ignore the first character

    if( pkFAILED( pkResult = AdvanceCharacter() ) )
        return pkResult;

    if ( m_Ch == '#' )     // character as hex or decimal
    {
        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;
        if ( m_Ch == 'x' )     // hex number
        {
            if( pkFAILED( pkResult = AdvanceCharacter() ) )
                return pkResult;

            while ( m_Ch != ';' )
            {
                wVal *= 16;

                if ( ( m_Ch >= '0' ) && ( m_Ch <= '9' ) )
                {
                    wVal += m_Ch - '0';
                }
                else if ( ( m_Ch >= 'a' ) && ( m_Ch <= 'f' ) )
                {
                    wVal += m_Ch - 'a' + 10;
                }
                else if ( ( m_Ch >= 'A' ) && ( m_Ch <= 'F' ) )
                {
                    wVal += m_Ch - 'A' + 10;
                }
                else
                {
                    Error( pkE_INVALID_XML_SYNTAX, "Expected hex digit as part of &#x escape sequence" );
                    return pkE_INVALID_XML_SYNTAX;
                }

                if( pkFAILED( pkResult = AdvanceCharacter() ) )
                    return pkResult;
            }
        }
        else                    // decimal number
        {
            while ( m_Ch != ';' )
            {
                wVal *= 10;

                if ( ( m_Ch >= '0' ) && ( m_Ch <= '9' ) )
                {
                    wVal += m_Ch - '0';
                }
                else
                {
                    Error( pkE_INVALID_XML_SYNTAX, "Expected decimal digit as part of &# escape sequence" );
                    return pkE_INVALID_XML_SYNTAX;
                }

                if( pkFAILED( pkResult = AdvanceCharacter() ) )
                    return pkResult;
            }
        }

        // copy character into the buffer
        m_Ch = wVal;

        return pkS_OK;
    }

    // must be an entity reference

    WCHAR *pEntityRefVal = m_pWritePtr;
    uint32 EntityRefLen;

    m_bSkipNextAdvance = true;
    if( pkFAILED( pkResult = AdvanceName() ) )
        return pkResult;

    EntityRefLen = (uint32)( m_pWritePtr - pEntityRefVal );
    m_pWritePtr = pEntityRefVal;

    if ( EntityRefLen == 0 )
    {
        Error( pkE_INVALID_XML_SYNTAX, "Expecting entity name after &" );
        return pkE_INVALID_XML_SYNTAX;
    }

    if( !wcsncmp( pEntityRefVal, L"lt", EntityRefLen ) )
        wVal = '<';
    else if( !wcsncmp( pEntityRefVal, L"gt", EntityRefLen ) )
        wVal = '>';
    else if( !wcsncmp( pEntityRefVal, L"amp", EntityRefLen ) )
        wVal = '&';
    else if( !wcsncmp( pEntityRefVal, L"apos", EntityRefLen ) )
        wVal = '\'';
    else if( !wcsncmp( pEntityRefVal, L"quot", EntityRefLen ) )
        wVal = '"';
    else
    {
        Error( pkE_INVALID_XML_SYNTAX, "Unrecognized entity name after & - (should be lt, gt, amp, apos, or quot)" );
        return pkE_INVALID_XML_SYNTAX;   // return false if unrecognized token sequence
    }

    if( pkFAILED( pkResult = AdvanceCharacter() ) )
        return pkResult;

    if( m_Ch != ';' )
    {
        Error( pkE_INVALID_XML_SYNTAX, "Expected terminating ; for entity reference" );
        return pkE_INVALID_XML_SYNTAX;   // malformed reference - needs terminating ;
    }

    m_Ch = wVal;
    return pkS_OK;
}


//-------------------------------------------------------------------------------------
// Name: CXmlParser::AdvanceAttrVal
// Desc: Copies an attribute value into m_pWrite buf, skipping surrounding quotes
//-------------------------------------------------------------------------------------
pkRESULT CXmlParser::AdvanceAttrVal()
{
    pkRESULT pkResult;
    WCHAR wQuoteChar;

    if( pkFAILED( pkResult = AdvanceCharacter() ) )
        return pkResult;

    if( ( m_Ch != '"' ) && ( m_Ch != '\'' ) )
    {
        Error( pkE_INVALID_XML_SYNTAX, "Attribute values must be enclosed in quotes" );
        return pkE_INVALID_XML_SYNTAX;
    }

    wQuoteChar = m_Ch;

    for( ;; )
    {
        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;
        else if( m_Ch == wQuoteChar )
            break;
        else if( m_Ch == '&' )
        {
            m_bSkipNextAdvance = true;
            if( pkFAILED( pkResult = ConvertEscape() ) )
                return pkResult;
        }
        else if( m_Ch == '<' )
        {
            Error( pkE_INVALID_XML_SYNTAX, "Illegal character '<' in element tag" );
            return pkE_INVALID_XML_SYNTAX;
        }

        // copy character into the buffer

        if( (size_t)(m_pWritePtr - m_pWriteBuf) >= XML_WRITE_BUFFER_SIZE )
        {
            Error( pkE_INVALID_XML_SYNTAX, 
                "Total element tag size may not be more than %d characters", 
                XML_WRITE_BUFFER_SIZE );
            return pkE_INVALID_XML_SYNTAX;
        }

        *m_pWritePtr = m_Ch;
        m_pWritePtr++;
    }
    return pkS_OK;
}


//-------------------------------------------------------------------------------------
// Name: CXmlParser::AdvanceName
// Desc: Copies a name into the m_pWriteBuf skipping leading whitespace.
// NOTE: Currently does not support unicode names
//-------------------------------------------------------------------------------------
pkRESULT CXmlParser::AdvanceName()
{
    pkRESULT pkResult;

    if( pkFAILED( pkResult = AdvanceCharacter() ) )
        return pkResult;

    if( ( ( m_Ch < 'A' ) || ( m_Ch > 'Z' ) ) &&
        ( ( m_Ch < 'a' ) || ( m_Ch > 'z' ) ) &&
        ( m_Ch != '_' ) && ( m_Ch != ':' ) )
    {
        Error( pkE_INVALID_XML_SYNTAX, "Names must start with an ASCII alphabetic character or _ or :" );
        return pkE_INVALID_XML_SYNTAX;
    }

    while( ( ( m_Ch >= 'A' ) && ( m_Ch <= 'Z' ) ) ||
        ( ( m_Ch >= 'a' ) && ( m_Ch <= 'z' ) ) ||
        ( ( m_Ch >= '0' ) && ( m_Ch <= '9' ) ) ||
        ( m_Ch == '_' ) || ( m_Ch == ':' ) ||
        ( m_Ch == '-' ) || ( m_Ch == '.' ) )
    {
        if( (size_t)(m_pWritePtr - m_pWriteBuf) >= XML_WRITE_BUFFER_SIZE )
        {
            Error( pkE_INVALID_XML_SYNTAX, 
                "Total element tag size may not be more than %d characters", 
                XML_WRITE_BUFFER_SIZE );
            return pkE_INVALID_XML_SYNTAX;
        }

        *m_pWritePtr = m_Ch;
        m_pWritePtr++;

        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;
    }

    m_bSkipNextAdvance = true;
    return pkS_OK;
}


//-------------------------------------------------------------------------------------
// Name: CXmlParser::AdvanceCharacter
// Desc: Copies the character at *m_pReadPtr to m_Ch
//       handling difference in UTF16 / UTF8, and big/little endian
//       and getting another chunk of the file if needed
//       Returns pkS_OK if there are more characters, pkE_ABORT for no characters to read
//-------------------------------------------------------------------------------------
pkRESULT CXmlParser::AdvanceCharacter( bool bOkToFail )
{
    if( m_bSkipNextAdvance )
    {
        m_bSkipNextAdvance = false;
        return pkS_OK;
    }

    switch (m_eEncodingType)
    {
        case eEncodingType_UTF8:
        {
            if (!FillBuffer(1))
            {
                goto bailEoF;
            }
            
            m_Ch = (WCHAR) *m_pReadPtr++;
            
            if (0 != (m_Ch & 0x80)) // not a single byte code point
            {
                if (0xC0 == (m_Ch & 0xE0)) // 2 byte code point
                {
                    if (!FillBuffer(1)) // need one more byte
                    {
                        goto bailEoF;
                    }

                    // Validate format (10xxxxxx) of second byte
                    if (0x80 != (m_pReadPtr[0] & 0xC0))
                    {
                        Error( pkE_INVALID_XML_SYNTAX, 
                            "Malformed UTF-8 2nd byte (0x%2X)", 
                            (unsigned int)m_pReadPtr[0] );
                        
                        return pkE_INVALID_XML_SYNTAX;
                    }
                    m_Ch = ((m_Ch & 0x1F) << 6) | (((WCHAR) *m_pReadPtr++) & 0x3F);
                }
                else if (0xE0 == (m_Ch & 0xF0)) // 3 byte code point
                {
                    if (!FillBuffer(2)) // need two more bytes
                    {
                        goto bailEoF;
                    }
                    
                    // Validate format (10xxxxxx) of secondary bytes
                    if (0x80 != (m_pReadPtr[0] & 0xC0) || 0x80 != (m_pReadPtr[1] & 0xC0) )
                    {
                        Error( pkE_INVALID_XML_SYNTAX, 
                            "Malformed UTF-8 2nd (0x%2X) or 3rd (0x%2X) byte", 
                            (unsigned int)m_pReadPtr[0],
                            (unsigned int)m_pReadPtr[1] );
                        
                        return pkE_INVALID_XML_SYNTAX;
                    }
                    m_Ch = ((m_Ch & 0x0F) << 6) | (((WCHAR) *m_pReadPtr++) & 0x3F);
                    m_Ch = (m_Ch << 6) | (((WCHAR) *m_pReadPtr++) & 0x3F);
                }
                else if (0xF0 == (m_Ch & 0xF0)) // 4 or more byte code point
                {
                    Error( pkE_INVALID_XML_SYNTAX, 
                        "UTF-8 code 0x%2X (beyond 3 bytes) is not allowed", 
                        (unsigned int)m_Ch );
                    
                    return pkE_INVALID_XML_SYNTAX;
                }
                else // invalid UTF-8 encoding
                {
                    Error( pkE_INVALID_XML_SYNTAX, 
                        "Malformed UTF-8 code 0x%2X", 
                        (unsigned int)m_Ch );
                    
                    return pkE_INVALID_XML_SYNTAX;
                }
            }
            break;
        }
            
        case eEncodingType_UTF16LE:
        {
            if (!FillBuffer(2))
            {
                goto bailEoF;
            }
            // Note the following assignment must be done by converting bytes to uint16 for the
            //    endianess auto-compensation assumed in MainParseLoop to work
            uint16 chUTF16 = ConvertBytesToUInt16(m_pReadPtr, m_pReadPtr+1);
            
            m_Ch = (WCHAR) chUTF16;
            m_pReadPtr += 2;
            
            // Reject UTF-16 surrogate pair prefix for code points outside the Basic Multilingual Plane
            if ( chUTF16 >= 0xD800 && chUTF16 <= 0xDFFF)
            {
                Error( pkE_INVALID_XML_SYNTAX, 
                    "UTF-16 surrogate pair prefix 0x%4X is not allowed", 
                    (unsigned int)chUTF16 );
                
                return pkE_INVALID_XML_SYNTAX;
            }
            break;
        }
        case eEncodingType_UTF16BE:
        {
            if (!FillBuffer(2))
            {
                goto bailEoF;
            }
            // Note the following assignment must be done by converting bytes to uint16 for the
            //    endianess auto-compensation assumed in MainParseLoop to work
            uint16 chUTF16 = ConvertBytesToUInt16(m_pReadPtr, m_pReadPtr+1);
            
            //reverse the bytes
            chUTF16 = ( chUTF16 << 8 ) + ( chUTF16 >> 8 );
            
            m_Ch = (WCHAR) chUTF16;
            m_pReadPtr += 2;
            
            // Reject UTF-16 surrogate pair prefix for code points outside the Basic Multilingual Plane
            if ( chUTF16 >= 0xD800 && chUTF16 <= 0xDFFF)
            {
                Error( pkE_INVALID_XML_SYNTAX, 
                    "UTF-16 surrogate pair prefix 0x%4X is not allowed", 
                    (unsigned int)chUTF16 );
                
                return pkE_INVALID_XML_SYNTAX;
            }
            break;
        }
        
        default:
            goto bailEoF;
    }

    if( m_Ch == '\n' )
    {
        m_pISAXCallback->m_LineNum++;
        m_pISAXCallback->m_LinePos = 0;
    }
    else if( m_Ch != '\r' )
    {
        m_pISAXCallback->m_LinePos++;
    }
    
    return pkS_OK;

bailEoF:
    
    if( !bOkToFail )
    {
        Error( pkE_INVALID_XML_SYNTAX, "Unexpected EOF" );
        
        return pkE_INVALID_XML_SYNTAX;
    }
    else
    {
        return pkE_FAIL;
    }
}


//-------------------------------------------------------------------------------------
// Name: CXmlParser::AdvanceElement
// Desc: Builds <element> data, calls callback
//-------------------------------------------------------------------------------------
pkRESULT CXmlParser::AdvanceElement()
{
    pkRESULT pkResult;

    // write ptr at the beginning of the buffer
    m_pWritePtr = m_pWriteBuf;

    if( pkFAILED( pkResult = AdvanceCharacter() ) )
        return pkResult;

    // if first character wasn't '<', we wouldn't be here

    if( pkFAILED( pkResult = AdvanceCharacter() ) )
        return pkResult;

    if( m_Ch == '!' )
    {
        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;
        if ( m_Ch == '-' )
        {
            if( pkFAILED( pkResult = AdvanceCharacter() ) )
                return pkResult;
            if( m_Ch != '-' )
            {
                Error( pkE_INVALID_XML_SYNTAX, "Expecting '-' after '<!-'" );
                return pkE_INVALID_XML_SYNTAX;
            }
            if( pkFAILED( pkResult = AdvanceComment() ) )
                return pkResult;
            return pkS_OK;
        }

        if( m_Ch != '[' )
        {
            Error( pkE_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return pkE_INVALID_XML_SYNTAX;
        }
        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;
        if( m_Ch != 'C' )
        {
            Error( pkE_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return pkE_INVALID_XML_SYNTAX;
        }
        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;
        if( m_Ch != 'D' )
        {
            Error( pkE_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return pkE_INVALID_XML_SYNTAX;
        }
        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;
        if( m_Ch != 'A' )
        {
            Error( pkE_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return pkE_INVALID_XML_SYNTAX;
        }
        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;
        if( m_Ch != 'T' )
        {
            Error( pkE_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return pkE_INVALID_XML_SYNTAX;
        }
        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;
        if( m_Ch != 'A' )
        {
            Error( pkE_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return pkE_INVALID_XML_SYNTAX;
        }
        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;
        if( m_Ch != '[' )
        {
            Error( pkE_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return pkE_INVALID_XML_SYNTAX;
        }
        if( pkFAILED( pkResult = AdvanceCDATA() ) )
            return pkResult;
    }
    else if( m_Ch == '/' )
    {
        const WCHAR *pEntityRefVal = m_pWritePtr;

        if( pkFAILED( pkResult = AdvanceName() ) )
            return pkResult;

        if( pkFAILED( m_pISAXCallback->ElementEnd( pEntityRefVal,
            (uint32) ( m_pWritePtr - pEntityRefVal ) ) ) )
            return pkE_ABORT;

        if( pkFAILED( pkResult = ConsumeSpace() ) )
            return pkResult;

        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;

        if( m_Ch != '>' )
        {
            Error( pkE_INVALID_XML_SYNTAX, "Expecting '>' after name for closing entity reference" );
            return pkE_INVALID_XML_SYNTAX;
        }
    }
    else if( m_Ch == '?' )
    {
        // just skip any xml header tag since not really important after identifying character set
        for( ;; )
        {
            if( pkFAILED( pkResult = AdvanceCharacter() ) )
                return pkResult;

            if ( m_Ch == '>' )
                return pkS_OK;
        }
    }
    else
    {
        XMLAttribute   Attributes[ XML_MAX_ATTRIBUTES_PER_ELEMENT ];
        uint32         NumAttrs;

        const WCHAR *pEntityRefVal = m_pWritePtr;
        uint32 EntityRefLen;

        NumAttrs = 0;

        m_bSkipNextAdvance = true;

        // Entity tag
        if( pkFAILED( pkResult = AdvanceName() ) )
            return pkResult;

        EntityRefLen = (uint32)( m_pWritePtr - pEntityRefVal );

        if( pkFAILED( pkResult = ConsumeSpace() ) )
            return pkResult;

        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;

        // read attributes
        while( ( m_Ch != '>' ) && ( m_Ch != '/' ) )
        {
            m_bSkipNextAdvance = true;

            if( NumAttrs >= XML_MAX_ATTRIBUTES_PER_ELEMENT )
            {
                Error( pkE_INVALID_XML_SYNTAX, 
                    "Elements may not have more than %d attributes", 
                    XML_MAX_ATTRIBUTES_PER_ELEMENT );
                return pkE_INVALID_XML_SYNTAX;
            }

            Attributes[ NumAttrs ].strName = m_pWritePtr;

            // Attribute name
            if( pkFAILED( pkResult = AdvanceName() ) )
                return pkResult;

            Attributes[ NumAttrs ].NameLen = (uint32)( m_pWritePtr - Attributes[ NumAttrs ].strName );

            if( pkFAILED( pkResult = ConsumeSpace() ) )
                return pkResult;

            if( pkFAILED( pkResult = AdvanceCharacter() ) )
                return pkResult;

            if( m_Ch != '=' )
            {
                Error( pkE_INVALID_XML_SYNTAX, "Expecting '=' character after attribute name" );
                return pkE_INVALID_XML_SYNTAX;
            }

            if( pkFAILED( pkResult = ConsumeSpace() ) )
                return pkResult;

            Attributes[ NumAttrs ].strValue = m_pWritePtr;

            if( pkFAILED( pkResult = AdvanceAttrVal() ) )
                return pkResult;

            Attributes[ NumAttrs ].ValueLen = (uint32)( m_pWritePtr -
                Attributes[ NumAttrs ].strValue );

            ++NumAttrs;

            if( pkFAILED( pkResult = ConsumeSpace() ) )
                return pkResult;

            if( pkFAILED( pkResult = AdvanceCharacter() ) )
                return pkResult;
        }

        if( m_Ch == '/' )
        {
            if( pkFAILED( pkResult = AdvanceCharacter() ) )
                return pkResult;
            if( m_Ch != '>' )
            {
                Error( pkE_INVALID_XML_SYNTAX, "Expecting '>' after '/' in element tag" );
                return pkE_INVALID_XML_SYNTAX;
            }

            if( pkFAILED( m_pISAXCallback->ElementBegin( pEntityRefVal, EntityRefLen,
                Attributes, NumAttrs ) ) )
                return pkE_ABORT;

            if( pkFAILED( m_pISAXCallback->ElementEnd( pEntityRefVal, EntityRefLen ) ) )
                return pkE_ABORT;
        }
        else
        {
            if( pkFAILED( m_pISAXCallback->ElementBegin( pEntityRefVal, EntityRefLen,
                Attributes, NumAttrs ) ) )
                return pkE_ABORT;
        }
    }

    return pkS_OK;
}


//-------------------------------------------------------------------------------------
// Name: CXmlParser::AdvanceCDATA
// Desc: Read a CDATA section
//-------------------------------------------------------------------------------------
pkRESULT CXmlParser::AdvanceCDATA()
{
    pkRESULT pkResult;
    uint16 wStage = 0;

    if( pkFAILED( m_pISAXCallback->CDATABegin() ) )
        return pkE_ABORT;

    for( ;; )
    {
        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;

        *m_pWritePtr = m_Ch;
        m_pWritePtr++;

        if( ( m_Ch == ']' ) && ( wStage == 0 ) )
            wStage = 1;
        else if( ( m_Ch == ']' ) && ( wStage == 1 ) )
            wStage = 2;
        else if( ( m_Ch == '>' ) && ( wStage == 2 ) )
        {
            m_pWritePtr -= 3;
            break;
        }
        else
            wStage = 0;

        if( (size_t)(m_pWritePtr - m_pWriteBuf) >= XML_WRITE_BUFFER_SIZE )
        {
            if( pkFAILED( m_pISAXCallback->CDATAData( m_pWriteBuf, (uint32)( m_pWritePtr - m_pWriteBuf ), true ) ) )
                return pkE_ABORT;
            m_pWritePtr = m_pWriteBuf;
        }
    }

    if( pkFAILED( m_pISAXCallback->CDATAData( m_pWriteBuf, (uint32)( m_pWritePtr - m_pWriteBuf ), false ) ) )
        return pkE_ABORT;

    m_pWritePtr = m_pWriteBuf;

    if( pkFAILED( m_pISAXCallback->CDATAEnd() ) )
        return pkE_ABORT;

    return pkS_OK;
}

//-------------------------------------------------------------------------------------
// Name: CXmlParser::AdvanceComment
// Desk: Skips over a comment
//-------------------------------------------------------------------------------------
pkRESULT CXmlParser::AdvanceComment()
{
    pkRESULT pkResult;
    uint16 wStage;

    wStage = 0;
    for( ;; )
    {
        if( pkFAILED( pkResult = AdvanceCharacter() ) )
            return pkResult;

        if (( m_Ch == '-' ) && ( wStage == 0 ))
            wStage = 1;
        else if (( m_Ch == '-' ) && ( wStage == 1 ))
            wStage = 2;
        else if (( m_Ch == '>' ) && ( wStage == 2 ))
            break;
        else
            wStage = 0;
    }

    return pkS_OK;
}


//-------------------------------------------------------------------------------------
// Name: CXmlParser::RegisterSAXCallbackInterface
// Desc: Registers callback interface
//-------------------------------------------------------------------------------------
void CXmlParser::RegisterSAXCallbackInterface( ISAXCallback *pISAXCallback )
{
    m_pISAXCallback = pISAXCallback;
}


//-------------------------------------------------------------------------------------
// Name: CXmlParser::GetSAXCallbackInterface
// Desc: Returns current callback interface
//-------------------------------------------------------------------------------------
ISAXCallback* CXmlParser::GetSAXCallbackInterface()
{
    return m_pISAXCallback;
}


//-------------------------------------------------------------------------------------
// Name: CXmlParser::MainParseLoop
// Desc: Main Loop to Parse Data - source agnostic
//-------------------------------------------------------------------------------------
pkRESULT CXmlParser::MainParseLoop(_Out_ EEncodingType* pEncodingType /* = NULL */)
{
    bool bWhiteSpaceOnly = true;
    pkRESULT pkResult = pkS_OK;

    if( pkFAILED( m_pISAXCallback->StartDocument() ) )
        return pkE_ABORT;

    m_pWritePtr = m_pWriteBuf;

    // A minimum of 4 bytes are required to examine the Byte Order Mark and first character
    if (!FillBuffer(4)) 
    {
        Error( pkE_INVALID_XML_SYNTAX, "XML document contains fewer than 4 bytes");
        return pkE_INVALID_XML_SYNTAX;
    }

    // Note: For UTF-16 this code auto-compensates for platform endianess 
    //       because AdvanceCharacter also converts bytes to uint16 for those encodings.

    // Immediately after an optional Byte Order Mark, the document must start with a
    //    left corner bracked "<" character (without leading white space) to be considered valid.
    //    This requirement simplifies auto-detection when a Byte Order Mark is not present.
    {
        static const byte lcbASCIICode = 0x3C; // left corner bracket "<" ASCII code
        static const uint16 lcbWordMatchesHE = 0x003C; // left corner bracket "<" in matching Host Endian
        static const uint16 lcbWordReverseHE = 0x3C00; // left corner bracket "<" in reversed Host Endian
            
        uint16 firstWordHE = ConvertBytesToUInt16(m_pReadBuf, m_pReadBuf+1); // the first 16-bit word in Host Endian

        m_eEncodingType = eEncodingType_invalid;
        
        if (0xFEFF == firstWordHE) // UTF-16 Little Endian BOM: FE FF
        {
            if ( lcbWordMatchesHE == ConvertBytesToUInt16(m_pReadBuf+2, m_pReadBuf+3) )
            {
                m_eEncodingType = eEncodingType_UTF16LE;
                m_pReadPtr += 2;
            }
        }
        else if (0xFFFE == firstWordHE) // UTF-16 Big Endian BOM: FF FE
        {
            if ( lcbWordReverseHE == ConvertBytesToUInt16(m_pReadBuf+2, m_pReadBuf+3) )
            {
                m_eEncodingType = eEncodingType_UTF16BE;
                m_pReadPtr += 2;
            }
        }
        else if ( (0xEF == m_pReadBuf[0]) // UTF-8 BOM: EF BB BF
            && (0xBB == m_pReadBuf[1]) 
            && (0xBF == m_pReadBuf[2])
            && (lcbASCIICode == m_pReadBuf[3]) )
        {
            m_eEncodingType = eEncodingType_UTF8;
            m_pReadPtr += 3;
        }
        else if ( lcbWordMatchesHE == firstWordHE )
        {
            // appears to be UTF-16 matching host endian (no BOM)
            m_eEncodingType = eEncodingType_UTF16LE;
        }
        else if ( lcbWordReverseHE == firstWordHE )
        {
            // appears to be UTF-16 reversed from host endian (no BOM)
            m_eEncodingType = eEncodingType_UTF16BE;
        }
        else if ( lcbASCIICode == m_pReadBuf[0] )
        {
            // appears to be UTF-8 (no BOM)
            m_eEncodingType = eEncodingType_UTF8;
        }
        
        if (pEncodingType)
        {
            *pEncodingType = m_eEncodingType;
        }

        if (eEncodingType_invalid == m_eEncodingType)
        {
            Error( pkE_INVALID_XML_SYNTAX, 
                "Unrecognized XML document character encoding (starts with hex %2X %2X %2X %2X)",
                (unsigned int) m_pReadBuf[0],
                (unsigned int) m_pReadBuf[1],
                (unsigned int) m_pReadBuf[2],
                (unsigned int) m_pReadBuf[3] );
            
            return pkE_INVALID_XML_SYNTAX;
        }
    }

    // Processing loop
    for( ;; )
    {
        if( pkFAILED( AdvanceCharacter( true ) ) )
        {
            if ( ( (uint32) ( m_pWritePtr - m_pWriteBuf ) != 0 ) && ( !bWhiteSpaceOnly ) )
            {
                if( pkFAILED( m_pISAXCallback->ElementContent( m_pWriteBuf, (uint32)( m_pWritePtr - m_pWriteBuf ), false ) ) )
                    return pkE_ABORT;

                bWhiteSpaceOnly = true;
            }

            if( pkFAILED( m_pISAXCallback->EndDocument() ) )
                return pkE_ABORT;

            // Normal processing end
            return pkS_OK;
        }

        if( m_Ch == '<' )
        {
            if( ( (uint32) ( m_pWritePtr - m_pWriteBuf ) != 0 ) && ( !bWhiteSpaceOnly ) )
            {
                if( pkFAILED( m_pISAXCallback->ElementContent( m_pWriteBuf, (uint32)( m_pWritePtr - m_pWriteBuf ), false ) ) )
                    return pkE_ABORT;

                bWhiteSpaceOnly = true;
            }

            m_bSkipNextAdvance = true;

            m_pWritePtr = m_pWriteBuf;

            if( pkFAILED( pkResult = AdvanceElement() ) )
                return pkResult;

            m_pWritePtr = m_pWriteBuf;
        }
        else
        {
            if( m_Ch == '&' )
            {
                m_bSkipNextAdvance = true;
                if( pkFAILED( pkResult = ConvertEscape() ) )
                    return pkResult;
            }

            if( bWhiteSpaceOnly && !s_IsWhiteSpace(m_Ch) )
            {
                bWhiteSpaceOnly = false;
            }

            *m_pWritePtr = m_Ch;
            m_pWritePtr++;

            if( (size_t)(m_pWritePtr - m_pWriteBuf) >= XML_WRITE_BUFFER_SIZE )
            {
                if( !bWhiteSpaceOnly )
                {
                    if( pkFAILED( m_pISAXCallback->ElementContent( m_pWriteBuf,
                        ( uint32 ) ( m_pWritePtr - m_pWriteBuf ),
                        true ) ) )
                    {
                        return pkE_ABORT;
                    }
                }

                m_pWritePtr = m_pWriteBuf;
                bWhiteSpaceOnly = true;
            }
        }
    }
}

//-------------------------------------------------------------------------------------
// Name: CXmlParser::ParseXMLFile
// Desc: Builds element data
//-------------------------------------------------------------------------------------
pkRESULT CXmlParser::ParseXMLBuffer( const char *strBuffer, uint32 uBufferSize )
{
    pkRESULT pkResult;

    if (NULL == m_pISAXCallback)
    {
        return pkE_NOINTERFACE;
    }

    if (NULL == strBuffer || 0 == uBufferSize)
    {
        return pkE_INVALIDARG;
    }

    m_pISAXCallback->m_LineNum = 1;
    m_pISAXCallback->m_LinePos = 0;

    m_bSkipNextAdvance = false;
    m_pReadPtr = m_pReadBuf;
    m_pReadBufTop = m_pReadBuf;

    m_pInXMLBuffer = strBuffer;
    m_uInXMLBufferCharsLeft = uBufferSize;

    pkResult = MainParseLoop();

    return pkResult;
}

//-------------------------------------------------------------------------------------
// Name: CXmlParser::ParseXMLStream
// Desc: Builds element data from a stream source
//-------------------------------------------------------------------------------------
pkRESULT CXmlParser::ParseXMLStream( HANDLE hStream, PFN_READCB pfnReadCB, _Out_ EEncodingType* pEncodingType /* = NULL */)
{
    pkRESULT pkResult;

    if (NULL == m_pISAXCallback)
    {
        return pkE_NOINTERFACE;
    }

    if (NULL == pfnReadCB)
    {
        return pkE_INVALIDARG;
    }

    m_pISAXCallback->m_LineNum = 1;
    m_pISAXCallback->m_LinePos = 0;

    m_bSkipNextAdvance = false;
    m_pReadPtr = m_pReadBuf;
    m_pReadBufTop = m_pReadBuf;

    m_pfnReadCB = pfnReadCB;
    m_hStream = hStream;

    m_pInXMLBuffer = NULL;
    m_uInXMLBufferCharsLeft = 0;

    pkResult = MainParseLoop(pEncodingType);

    return pkResult;
}


//-------------------------------------------------------------------------------------
// CXmlParser::Error()
//      Logs an error through the callback interface
//-------------------------------------------------------------------------------------
void CXmlParser::Error( pkRESULT hErr, const char* strFormat, ... )
{
    const INT MAX_OUTPUT_STR = 160;
    char strBuffer[ MAX_OUTPUT_STR ];
    va_list pArglist;
    va_start( pArglist, strFormat );

    StringCchVPrintfA( strBuffer, MAX_OUTPUT_STR, strFormat, pArglist );

    m_pISAXCallback->Error( hErr, strBuffer );
    va_end( pArglist );
}
