/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file sha1.h
*   \brief  sha1 crypto definition
*   \author Montage
*/


#ifndef _AES_WRAP_H_
#define _AES_WRAP_H_


int aes_wrap(const unsigned char *kek, int n, const unsigned char *plain, unsigned char *cipher);
int aes_unwrap(const unsigned char *kek, int n, const unsigned char *cipher, unsigned char *plain);

#endif  // _AES_WRAP_H_
