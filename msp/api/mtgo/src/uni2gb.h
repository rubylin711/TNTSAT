

#ifndef __UNI_H__
#define __UNI_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char BYTE;
typedef unsigned short WORD;

#define FT2_GB2UNI_SUPPORTED 1 //支持GB到uincode转换

#define FT2_UNI2GB_SUPPORTED 0 //支持unicode到GB转换

/* Function    : FT2_UnicodeToChar
** ===============================================================
** Description : unicode to char
** Param       : uniCode : unicode
** Return      : char code
** ===============================================================
*/
#if FT2_UNI2GB_SUPPORTED
WORD FT2_UnicodeToChar(WORD uniCode);
#endif

/* Function    : FT2_CharToUnicode
** ===============================================================
** Description : char to unicode
** Param       : charCode : char code
** Return      : unicode
** ===============================================================
*/
#if FT2_GB2UNI_SUPPORTED
WORD FT2_CharToUnicode(WORD charCode);
#endif

#ifdef __cplusplus
}
#endif

#endif
