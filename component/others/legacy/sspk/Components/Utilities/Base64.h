///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef __BASE64_H__
#define __BASE64_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef UNICODE
#define Base64Decode  Base64DecodeW
#else
#define Base64Decode  Base64DecodeA
#endif // !UNICODE

pkRESULT
Base64DecodeA(
    IN CHAR const *pchIn,
    IN DWORD cchIn,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut);

pkRESULT
Base64DecodeW(
    IN WCHAR const *pchIn,
    IN DWORD cchIn,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut);

pkRESULT
Base64DecodeExA(
    IN CHAR const *pchIn,
    IN DWORD cchIn,
    __out_ecount(*pcbOut)OUT BYTE *pbOut,
    OUT DWORD *pcbOut,
    OUT DWORD *pchConsumed);


#ifdef UNICODE
#define Base64Encode  Base64EncodeW
#else
#define Base64Encode  Base64EncodeA
#endif // !UNICODE

pkRESULT
Base64EncodeA(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    __out_ecount_opt(*pcchOut)OUT CHAR *pchOut,
    OUT DWORD *pcchOut);

pkRESULT
Base64EncodeW(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    __out_ecount(*pcchOut)OUT WCHAR *pchOut,
    OUT DWORD *pcchOut);


#ifdef __cplusplus
}       // Balance extern "C" above
#endif

#endif // BASE64

