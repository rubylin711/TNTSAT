///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// <summary>
/// StringUtils.h
/// </summary>

#include <strsafe.h>

#include <map>
#include <string>
#include <vector>


//Handy stl string manipulation classes
int            split(const std::string& text, std::vector<std::string>& words, const std::string& separators = " \t\n");
int            wsplit(const std::wstring& text, std::vector<std::wstring>& words, const std::wstring& separators = L" \t\n");
std::string    trim(const std::string& s, const char* trimArray = " \t");
std::wstring   wtrim(const std::wstring& s, const WCHAR* trimArray = L" \t");
int            nameValue(const std::string& query, std::map<std::string,std::string>& nameValue);
bool           startsWith(const std::string& str, const std::string& with);
bool           endsWith(const std::string& str, const std::string& with);
std::string    toLower(const std::string& s);
std::string    toUpper(const std::string& s);

//Handy string conversion routines to and from numerics
std::string    toString(int i, bool hex = false);
std::wstring   toWString(int64 i, bool hex = false);
std::string    uint32ToString(uint32 i, bool hex = false);
std::string    floatToString(float f);
std::string    toString64(int64 i, bool hex = false);
std::string    uint64toString64(uint64 i, bool hex = false);
std::wstring   toWString64(int64 i, bool hex = false);
int            toInt(const std::string& s);
unsigned long  toULong(const std::string& s);
int            toIntFromHex(const std::string& s);
unsigned long  toULongFromHex(const std::string& s);
int64          toInt64(const std::string& s);
uint64         toUInt64(const std::string& s);
float          toFloat(const std::string& s);

// This function returns the requested integer type, and checks for overflow in debug builds
template <class T> T convertFromHex( const std::string& s);

//Handy wide string conversion to integers
uint32         wideToUInt32(const std::wstring& wstr);
uint64         wideToUInt64(const std::wstring& wstr);

//Parses a GUID in the form "wwwwwwww-xxxx-yyyy-zzzz-zzzzzzzzzzzz"
GUID           GuidFromString(__in_ecount(36) const char* String);

//Escape charaters for inclusion in a url or xml
std::string    escape(const std::string& s);
std::string    unescape(const std::string& s);
std::string    escapeXml(const std::string& s, bool attribute = false);
std::string    escapeSpaces(const std::string& s);

template< class charT >
pkRESULT HexStrToBytes(
        _In_count_(cchSrc)  const charT* pchSrc,
        _In_                size_t cchSrc,
        _Out_cap_(cbDst)    void* pDst,
        _In_                size_t cbDst );

template< class charT >
pkRESULT BytesToHexStr(
    _In_count_(cb) const uint8* pb,
    _In_ size_t cb,
    _Inout_ std::basic_string<charT>* pStr );

template< class charT >
pkRESULT StrToFourCC(
        _In_z_ const charT* psz,
        _Out_  uint32* pVal );

template< class charT >
pkRESULT FourCCToStr(
        _In_ uint32 val,
        _Out_ std::basic_string<charT>* pStr );

template< class charT >
void StrReplaceInPlace(
        _Inout_ std::basic_string<charT>* pStr,
        _In_ const charT* pszFrom,
        _In_ const charT* pszTo );


/// <summary>
/// Str2WStr - converts char string to wchar_t string
/// <summary>

class Str2WStr : public std::wstring
{
public:

    Str2WStr( _In_ const char* psz )        { _Set( psz ); }
    Str2WStr( _In_ const std::string& str ) { _Set( str.c_str(), str.size() ); }

private:

    void _Set( _In_ const char* psz, _In_ int cch = -1 );
};

/// <summary>
/// WStr2Str - converts char string to wchar_t string
/// <summary>

class WStr2Str : public std::string
{
public:

    WStr2Str( _In_ const wchar_t* psz )        { _Set( psz ); }
    WStr2Str( _In_ const std::wstring& str )   { _Set( str.c_str(), str.size() ); }

private:

    void _Set( _In_ const wchar_t* psz, _In_ int cch = -1 );
};

