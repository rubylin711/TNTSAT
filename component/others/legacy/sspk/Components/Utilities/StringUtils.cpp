///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringUtils.h"
#include "Trace.h"

#include <stdio.h> // sscanf
#include <algorithm>
#include <cctype>
#include <vector>

using namespace std;

//================================================================
//================================================================
// Handy string routines, use 'em

template <class T>
int split_t(const T& text, vector<T>& words, const T& separators)
{
    size_t textLen = text.length();
    size_t start = text.find_first_not_of(separators, 0);
    while ((start >= 0) && (start < textLen))
    {
        size_t stop = text.find_first_of(separators,start);

        if ((stop < 0) || (stop > textLen))
            stop = textLen;

        words.push_back(text.substr(start, stop-start));
        start = text.find_first_not_of(separators, stop+1);
    }
    return (int)words.size();
}

int split(const string& text, vector<string>& words, const string& separators)
{
    return split_t<string>(text, words, separators);
}

int wsplit(const wstring& text, vector<wstring>& words, const wstring& separators)
{
    return split_t<wstring>(text, words, separators);
}

string trim(const string& s, const char* trimArray)
{
    if (s.length() == 0)
        return s;
    size_t b = s.find_first_not_of(trimArray);
    size_t e = s.find_last_not_of(trimArray);
    if (b == string::npos)
        return "";
    return string(s, b, e - b + 1);
}

wstring wtrim(const wstring& s, const WCHAR* trimArray)
{
    if (s.length() == 0)
        return s;
    size_t b = s.find_first_not_of(trimArray);
    size_t e = s.find_last_not_of(trimArray);
    if (b == string::npos)
        return L"";
    return wstring(s, b, e - b + 1);
}

int nameValue(const string& query, map<string,string>& nameValue)
{
    vector<string> args;
    split(query,args,"&");
    for (int i = 0; i < (int)args.size(); i++)
    {
        const char* n = args[i].c_str();
        const char* v = strchr(n,'=');
        if (v != NULL)
            nameValue[string(n,v - n)] = unescape(v + 1);
    }
    return (int)nameValue.size();
}

bool startsWith(const string& str, const string& with)
{
    return( str.compare( 0, with.length(), with ) == 0 );
}

bool endsWith(const string& str, const string& with)
{
    if( str.length() < with.length() )
    {
        return( false );
    }
    else
    {
        return( str.compare( str.length() - with.length(), with.length(), with ) == 0 );
    }
}

string toLower(const string& ss)
{
    string s(ss);
    for (size_t i = 0; i < s.length(); i++)
        s[i] = (char) tolower(s[i]);
    return s;
}

string toUpper(const string& ss)
{
    string s(ss);
    for (size_t i = 0; i < s.length(); i++)
        s[i] = (char) toupper(s[i]);
    return s;
}

//================================================================
//================================================================
// Handy conversion routines to and from numerics

// Only need 20 characters max for int64 conversion (plus null terminator)
// 0xffffffffffffffff   (uint64 hex)
// 18446744073709551616 (uint64 dec)
// -9223372036854775808 (int64 dec)
#define MAX_INT64_BUF    32

// Float range is 1.175494351e?38 to 3.402823466e+38
#define MAX_FLOAT_BUF    41

string toString(int i, bool hex)
{
    const int bufsize = MAX_INT64_BUF;
    char buffer[bufsize];
    if (hex)
    {
        buffer[0] = '0';
        buffer[1] = 'x';
        StringCbPrintfA(buffer + 2, bufsize - 2, "%x", i);
    } else
    {
        StringCbPrintfA(buffer, bufsize, "%d", i);
    }
    return buffer;
}

string floatToString(float f)
{
    char buffer[MAX_FLOAT_BUF];

    StringCbPrintfA(buffer, sizeof(buffer), "%f", f);
    return buffer;
}

wstring toWString(int64 i, bool hex)
{
    const int bufsize = MAX_INT64_BUF;
    WCHAR buffer[bufsize];
    if (hex)
    {
        buffer[0] = '0';
        buffer[1] = 'x';
        StringCbPrintfW(buffer + 2, bufsize - 2, L"%x", i);
    } else
    {
        StringCbPrintfW(buffer, bufsize, L"%d", i);
    }
    return buffer;
}

string uint32ToString(uint32 i, bool hex)
{
    const int bufsize = MAX_INT64_BUF;
    char buffer[bufsize];
    if (hex)
    {
        buffer[0] = '0';
        buffer[1] = 'x';
        StringCbPrintfA(buffer + 2, bufsize - 2, "%x", i);
    } else
    {
        StringCbPrintfA(buffer, bufsize, "%u", i);
    }
    return buffer;
}

string toString64(int64 i, bool hex)
{
    const int bufsize = MAX_INT64_BUF;
    char buffer[bufsize];
    if (hex)
    {
        buffer[0] = '0';
        buffer[1] = 'x';
        StringCbPrintfA(buffer + 2,
                        bufsize - 2,
                        "%llx",
                        i);
    } else
    {
        StringCbPrintfA(buffer,
                        bufsize,
                        "%lld",
                        i);
    }
    return buffer;
}

string uint64toString64(uint64 i, bool hex)
{
    const int bufsize = MAX_INT64_BUF;
    char buffer[bufsize];
    if (hex)
    {
        buffer[0] = '0';
        buffer[1] = 'x';
        StringCbPrintfA(buffer + 2,
                        bufsize - 2,
                        "%llx",
                        i);
    } else
    {
        StringCbPrintfA(buffer,
                        bufsize,
                        "%llu",
                        i);
    }
    return buffer;
}

wstring toWString64(int64 i, bool hex)
{
    const int bufsize = MAX_INT64_BUF;
    WCHAR buffer[bufsize];
    if (hex)
    {
        buffer[0] = '0';
        buffer[1] = 'x';
        StringCbPrintfW(buffer + 2,
                        bufsize - 2,
                        L"%llx",
                        i);
    } else
    {
        StringCbPrintfW(buffer,
                        bufsize,
                        L"%lld",
                        i);
    }
    return buffer;
}

int toInt(const string& s)
{
    char* stopPoint;
    return strtol(s.c_str(), &stopPoint, 0);
}

unsigned long toULong(const string& s)
{
    char* stopPoint;
    return strtoul(s.c_str(), &stopPoint, 0);
}

unsigned long toULongFromHex( const string& s)
{
    char* stopPoint;
    return strtoul(s.c_str(), &stopPoint, 16);
}

int toIntFromHex(const string& s)
{
    char* stopPoint;
    return strtol(s.c_str(), &stopPoint, 16);
}

int64 toInt64(const string& s)
{
    return toUInt64(s);
}

float toFloat(const string& s)
{
    char* stopPoint;
    return (float)strtod(s.c_str(), &stopPoint);
}

//Format [-][0x]<digits>
//return uint64 up to first valid digit, otherwise 0
uint64 toUInt64(const string& s)
{
    uint64 result;

    int len = s.length();
    if (!len)
        return 0;

    const char* cp = s.c_str();
    int sign = 1;

    if (*cp == '-')
    {
        sign = -1;
        cp++;
    }

    if (!*cp)
        return 0;

    int base = 10;
    if ((*cp == '0') && *++cp && (tolower(*cp++) == 'x'))
    {
        base = 16;
    }

    result = 0;

    while (*cp)
    {
        unsigned char cc=(unsigned char)*cp;
        if (cc >= '0' && cc <= '9')
        {
            cc-='0';
        }
        else
        {
            cc = (unsigned char) tolower(cc);
            if ((cc >= 'a') && (cc <= 'f'))
            {
                cc = cc -'a' + 10;
            }
            else
                break;
        }
        result = (result * base) + cc;
        cp++;
    }
    return result*sign;
}

template <class T> T convertFromHex( const std::string& s)
{
    char* stopPoint;
    unsigned long ulResult = strtoul(s.c_str(), &stopPoint, 16);
    assert(sizeof(T) >= sizeof(unsigned long) || ulResult < ((unsigned long)1 << (8 * sizeof(T)))); // Detect overflow in debug builds

    return static_cast<T>(ulResult);
}

uint32 wideToUInt32(const wstring& wstr)
{
    char szMultibyte[32];
    szMultibyte[0] = 0;
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szMultibyte, sizeof(szMultibyte), NULL, NULL);
    char* stopPoint;
    return (uint32)strtoul(szMultibyte, &stopPoint, 0);
}

uint64 wideToUInt64(const wstring& wstr)
{
    char szMultibyte[32];
    szMultibyte[0] = 0;
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szMultibyte, sizeof(szMultibyte), NULL, NULL);

    string strMultibyte(szMultibyte);
    return toUInt64(strMultibyte);
}

//=====================================================================
//=====================================================================
// Parses a GUID in the form "wwwwwwww-xxxx-yyyy-zzzz-zzzzzzzzzzzz"

GUID GuidFromString(__in_ecount(36) const char* String)
{
    INT iData;
    GUID guid = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

    if (strnlen(String, 37) < 36)
    {
        ASSERT(FALSE);
        return guid;
    }
    if (sscanf_s(String, "%08x", &guid.Data1) != 1)
    {
        ASSERT(FALSE);
    }
    if (sscanf_s(&String[9], "%04hx", &guid.Data2) != 1)
    {
        ASSERT(FALSE);
    }
    if (sscanf_s(&String[14], "%04hx", &guid.Data3) != 1)
    {
        ASSERT(FALSE);
    }
    iData = 19;
    for (INT i = 0; i < 8; i++)
    {
        CHAR buf[8];
        SHORT s;

        buf[0] = String[iData];
        buf[1] = String[iData+1];
        buf[2] = '\0';
        if (sscanf_s(buf, "%02hx", &s) != 1)
        {
            ASSERT(FALSE);
        }
        guid.Data4[i] = (CHAR)s;
        iData += 2;
        if (i == 1)
            iData++;    //skip '-'
    }
    return guid;
}

//================================================================
//================================================================
// Url utils

static bool NeedsEscape(char c)
{
    return ((c <= 0x20) || strchr(";/?:@&=+$,<>#%\"{}|\\^~[]`", c));
}

static int HexDigit(char c)
{
    c = (char) tolower(c);
    if (c >='0' && c <= '9') return c - '0';
    if (c >='a' && c <= 'f') return 10 + c - 'a';
    TRACE_ERROR(("Invalid hex digit"));
    return 0;
}

//Escape charaters for inclusion in a url
string escape(const string& s)
{
    string d;
    d.reserve(s.length() << 1);
    int len = (int)s.length();
    for (int i = 0; i < len; i++)
    {
        char c = s[i];
        if (NeedsEscape(c))
        {
            d += '%';
            d += "0123456789ABCDEF"[(c >> 4) & 0x0F];
            d += "0123456789ABCDEF"[c & 0x0F];
        }
        else
        {
            d += c;
        }
    }
    return d;
}

string unescape(const string& s)
{
    string u;
    u.reserve(s.length());
    int len = (int)s.length();
    for (int i = 0; i < len; i++)
    {
        char c = s[i];
        if (c == '%' && (i < len-2))
        {
            c = (char) (HexDigit(s[i+1]) << 4) | (char) HexDigit(s[i+2]);
            i += 2;
        }
        u += c;
    }
    return u;
}

//Escape characters for inclusion in xml
string escapeXml(const string& s, bool attribute)
{
    string d;
    d.reserve(s.length() << 1);
    int len = (int)s.length();
    for (int i = 0; i < len; i++)
    {
        char c = s[i];
        switch (c)
        {
            case '&':  d += "&amp;";                       break;
            case '<':  d += "&lt;";                        break;
            case '>':  d += "&gt;";                        break;
            case '\'': d += (attribute ? "&apos;" : "'");  break;
            case '"':  d += (attribute ? "&quot;" : "\""); break;
            default:   d += c;                             break;
        }
    }
    return d;
}

//Escape spaces in the string
string escapeSpaces(const string& s)
{
    string d;
    d.reserve(s.length() << 1);
    int len = (int)s.length();
    for (int i = 0; i < len; i++)
    {
        if (' ' == s[i])
        {
            d += "%20";
        }
        else
        {
            d += s[i];
        }
    }

    return d;
}

byte HexFromChar(WCHAR ch)
{
    if (L'0' <= ch && ch <= L'9')
        return (byte)(ch - L'0');
    else if (L'a' <= ch && ch <= L'f')
        return (byte)(10 + ch - L'a');
    else if (L'A' <= ch && ch <= L'F')
        return (byte)(10 + ch - L'A');
    else
        return (byte)-1;
}

////////////////////////////////////////////////////////////////////////////////
template< class charT >
pkRESULT HexStrToBytes(
        _In_count_(cchSrc)  const charT* pchSrc,
        _In_                size_t cchSrc,
        _Out_cap_(cbDst)    void* pDst,
        _In_                size_t cbDst )
{
    pkRESULT pkResult = pkS_OK;
    uint8* pbDst = NULL;

    size_t cbTotalSize = cchSrc / 2;

    // Input format must use 2 hex digits per byte otherwise
    // it's invalid
    if( ( cchSrc % 2 ) != 0 )
    {
        pkResult = pkE_INVALID_FORMAT;
        goto exit;
    }

    if( cbTotalSize != cbDst )
    {
        // Destination buffer's size doesn't match the source string
        pkResult = pkE_FAIL;
        goto exit;
    }

    // Check the input before allocating the output buffer
    for( size_t i = 0; i < cchSrc; ++i )
    {
        if( !iswxdigit( pchSrc[i] ) )
        {
            pkResult = pkE_INVALID_FORMAT;
            goto exit;
        }
    }

    pbDst = reinterpret_cast<uint8_t*>( pDst );

    // Convert
    for( size_t i = 0; i < cchSrc; i +=2 )
    {
        *pbDst =
            ( HexFromChar( pchSrc[i] ) << 4 )
            | ( HexFromChar( pchSrc[i+1] ) );

        ++pbDst;
    }

exit:

    return( pkResult );
}

////////////////////////////////////////////////////////////////////////////////
template< class charT >
pkRESULT BytesToHexStr(
        _In_count_(cb) const uint8* pb,
        _In_ size_t cb,
        _Inout_ std::basic_string<charT>* pStr )
{
    pkRESULT pkResult = pkS_OK;

    size_t cch = cb + cb;

    if( cch < cb )
    {
        // Overflow
        pkResult = pkE_INVALIDARG;
        goto exit;
    }

    pStr->clear();
    pStr->reserve( cch );

    static const char hexVals[] = "0123456789ABCDEF";

    while( cb > 0 )
    {
        (*pStr) += (charT)( hexVals[ ( (*pb) & 0xF0 ) >> 4 ] );
        (*pStr) += (charT)( hexVals[ ( (*pb) & 0x0F ) >> 0 ] );

        --cb;
        ++pb;
    }

exit:

    return( pkResult );
}

////////////////////////////////////////////////////////////////////////////////
template< class charT >
pkRESULT StrToFourCC(
        _In_z_ const charT* psz,
        _Out_  uint32* pVal )
{
    pkRESULT pkResult = pkS_OK;
    uint32 val = 0;

    *pVal = 0;

    for( size_t iShift = 0; iShift < 32; iShift += 8 )
    {
        if( 0 == *psz )
        {
            // zero found within the fourCC
            pkResult = pkE_INVALID_FORMAT;
            goto exit;
        }

        if( *psz > 0xFF )
        {
            pkResult = pkE_INVALID_FORMAT;
            goto exit;
        }

        val |= ( (uint32)*psz << iShift );

        ++psz;
    }

    if( *psz )
    {
        // Invalid input.
        // String has more than 4 chars.
        pkResult = pkE_INVALID_FORMAT;
        goto exit;
    }

    *pVal = val;

exit:

    return( pkResult );
}

////////////////////////////////////////////////////////////////////////////////
template< class charT >
pkRESULT FourCCToStr(_In_ uint32 val,
                    _Out_ std::basic_string<charT>* pStr )
{
    pkRESULT pkResult = pkS_OK;
    uint8* pVal = (uint8 *)&val;
    charT s[5];
    int i = 0;

    pStr->clear();

    for( ; i < 4; i++ )
    {
        if(0 == pVal[i])
        {
            pkResult = pkE_INVALID_FORMAT;
            goto exit;
        }

        s[i] = (charT)pVal[i];


    }
    s[4] = 0;

    pStr->assign(s);

exit:
    return pkResult;
}

////////////////////////////////////////////////////////////////////////////////
template< class charT >
size_t StrLen( _In_ const charT* psz )
{
    size_t cch = 0;
    while( psz[cch] )
    {
        ++cch;
    }

    return( cch );
}

////////////////////////////////////////////////////////////////////////////////
template< class charT >
void StrReplaceInPlace(
        _Inout_ std::basic_string<charT>* pStr,
        _In_ const charT* pszFrom,
        _In_ const charT* pszTo )
{
    size_t pos = 0;

    size_t cchFrom = StrLen( pszFrom );
    size_t cchTo = StrLen( pszTo );

    while( ( pos = pStr->find( pszFrom, pos ) ) != std::basic_string<charT>::npos )
    {
        pStr->replace( pos, cchFrom, pszTo, cchTo );
        pos += cchTo;
    }
}

////////////////////////////////////////////////////////////////////////////////
void Str2WStr::_Set( _In_ const char* psz, _In_ int cch )
{
    int len = MultiByteToWideChar( CP_ACP, 0, psz, cch, NULL, 0 );

    if( 0 < len )
    {
        wchar_t* wszTemp = new wchar_t[len];

        len = MultiByteToWideChar( CP_ACP, 0, psz, cch, wszTemp, len );

        // handle the case where the null terminator was processed
        if (len > 0 && 0 == wszTemp[len-1])
        {
            len--;
        }

        this->assign(wszTemp, len);

        delete [] wszTemp;

        ASSERT( len == (int)this->length() );
    }
    else
    {
        this->clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
void WStr2Str::_Set( _In_ const wchar_t* psz, _In_ int cch )
{
    int len =  WideCharToMultiByte(CP_ACP, 0, psz, cch, NULL, 0, NULL, NULL);

    if( 0 < len )
    {
        char* szTemp = new char[len];

        len = WideCharToMultiByte(CP_ACP, 0, psz, cch, szTemp, len, NULL, NULL );

        // handle the case where the null terminator was processed
        if (len > 0 && 0 == szTemp[len-1])
        {
            len--;
        }

        this->assign(szTemp, len);

        delete [] szTemp;

        ASSERT( len == (int)this->length() );
    }
    else
    {
        this->clear();
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// Instantiate the templates to ensure linkage
//

template
pkRESULT HexStrToBytes(
        _In_count_(cchSrc)  const char* pchSrc,
        _In_                size_t cchSrc,
        _Out_cap_(cbDst)    void* pDst,
        _In_                size_t cbDst );

template
pkRESULT HexStrToBytes(
        _In_count_(cchSrc)  const wchar_t* pchSrc,
        _In_                size_t cchSrc,
        _Out_cap_(cbDst)    void* pDst,
        _In_                size_t cbDst );

template
pkRESULT BytesToHexStr(
        _In_count_(cb) const uint8* pb,
        _In_ size_t cb,
        _Inout_ std::basic_string<char>* pStr );

template
pkRESULT BytesToHexStr(
        _In_count_(cb) const uint8* pb,
        _In_ size_t cb,
        _Inout_ std::basic_string<wchar_t>* pStr );

template
pkRESULT StrToFourCC(
        _In_z_ const char* pch,
        _Out_  uint32* pVal );

template
pkRESULT StrToFourCC(
        _In_z_ const wchar_t* pch,
        _Out_  uint32* pVal );

template
pkRESULT FourCCToStr(
        _In_ uint32 val,
        _Out_ std::basic_string<char>* pStr );

template
pkRESULT FourCCToStr(
        _In_ uint32 val,
        _Out_ std::basic_string<wchar_t>* pStr );

template
void StrReplaceInPlace(
        _Inout_ std::basic_string<wchar_t>* pStr,
        _In_ const wchar_t* pszFrom,
        _In_ const wchar_t* pszTo );

template
void StrReplaceInPlace(
        _Inout_ std::basic_string<char>* pStr,
        _In_ const char* pszFrom,
        _In_ const char* pszTo );

template uint8_t convertFromHex( const std::string& s);
template uint16_t convertFromHex( const std::string& s);
