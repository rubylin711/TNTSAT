/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file rc4.h
*   \brief  rc4 crypto definition.
*   \author Montage
*/

#ifndef _RC4_H_
#define _RC4_H_

void rc4_skip(const u8 *key, size_t keylen, size_t skip, u8 *src, size_t src_len, u8 *dst);
void rc4(u8 *buf, size_t len, const u8 *key, size_t key_len, u8 *dst);

#endif  // _RC4_H_
