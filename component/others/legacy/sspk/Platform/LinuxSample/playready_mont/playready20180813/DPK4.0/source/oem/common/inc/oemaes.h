/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

/*
**  oemaes.h
**
**  Contains structures and function definitions implemented in oemaes.c
**
*/

#ifndef __OEMAES_H__
#define __OEMAES_H__

#include <oemaeskey.h>
#include <oemaesimpl.h>
#include <oemaescommon.h>
#include <oemaesmulti.h>

ENTER_PK_NAMESPACE;

DRM_API DRM_RESULT DRM_CALL Oem_Aes_ZeroKey(
    __inout_ecount( 1 )                      DRM_AES_KEY  *f_pKey );

DRM_API DRM_RESULT DRM_CALL Oem_Aes_SetKey(
    __in_bcount( DRM_AES_KEYSIZE_128 ) const DRM_BYTE      f_rgbKey[ DRM_AES_KEYSIZE_128 ],
    __out_ecount( 1 )                        DRM_AES_KEY  *f_pAesKey ) DRM_NO_INLINE_ATTRIBUTE;

DRM_API DRM_RESULT DRM_CALL Oem_Aes_EncryptOne(
    __in_ecount( 1 )                   const DRM_AES_KEY  *f_pKey,
    __inout_bcount( DRM_AES_BLOCKLEN )       DRM_BYTE      f_rgbData[ DRM_AES_BLOCKLEN ] ) DRM_NO_INLINE_ATTRIBUTE;

DRM_API DRM_RESULT DRM_CALL Oem_Aes_DecryptOne(
    __in_ecount( 1 )                   const DRM_AES_KEY  *f_pKey,
    __inout_bcount( DRM_AES_BLOCKLEN )       DRM_BYTE      f_rgbData[ DRM_AES_BLOCKLEN ] ) DRM_NO_INLINE_ATTRIBUTE;

void  Oem_Mont_Aes_Setencryptkey(
    DRM_BYTE       *f_pKey,
    DRM_WCHAR32    len,
    void           *ptr);

void  Oem_Mont_Aes_Setdecryptkey(
    DRM_BYTE       *f_pKey,
    DRM_WCHAR32    len,
    void           *ptr);

void  Oem_Mont_Aes_EncryptOne(
    DRM_BYTE      f_rgbData[ DRM_AES_BLOCKLEN ] ,
    void          *ptr);

void  Oem_Mont_Aes_DecryptOne(
    DRM_BYTE      f_rgbData[ DRM_AES_BLOCKLEN ] ,
    void          *ptr);

EXIT_PK_NAMESPACE;

#endif /* __OEMAES_H__ */
