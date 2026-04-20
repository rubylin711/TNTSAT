/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wlan_util.h
*   \brief  wlan util definition
*   \author Montage
*/


#ifndef _WLAN_UTIL_H_
#define _WLAN_UTIL_H_

void inc_byte_array(unsigned char *counter, unsigned int len);
int hexstr2bin(const char *hex, unsigned char *buf, unsigned int len);

#endif // _WLAN_UTIL_H_
