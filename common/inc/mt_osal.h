/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
#ifndef __MT_OSAL_H__
#define __MT_OSAL_H__

#ifndef __KERNEL__
#include <stdio.h>
#include <stdarg.h>
#else
#include <linux/kernel.h>
#endif
#include "mt_type.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/** @addtogroup   COMMON  */
/** @{ */  /** <!-- [COMMON] */

typedef va_list mt_va_list_s;

mt_char* mt_osal_strncpy(mt_char *pszdest, const mt_char *pszsrc, mt_size_t ullen);
mt_s32 mt_osal_strncmp(const mt_char *pszstr1, const mt_char *pszstr2, mt_size_t ullen);
mt_s32 mt_osal_strncasecmp(const mt_char *pszstr1, const mt_char *pszstr2, mt_size_t ullen);
mt_char* mt_osal_strncat(mt_char *pszdest, const mt_char *pszsrc, mt_size_t ullen);
mt_s32 mt_osal_snprintf(mt_char *pszstr, mt_size_t ullen, const mt_char *pszformat, ...);
mt_s32 mt_osal_vsnprintf(mt_char *pszstr, mt_size_t ullen, const mt_char *pszformat, mt_va_list_s stvalist);
MT_BOOL mt_ticks_init(u32 cpu_freq);
u32 mt_ticks_get(void);


/** @} */  /** <!-- ==== group Definition end ==== */

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MT_OSAL_H__ */

