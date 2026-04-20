
/******************************* Include Files *******************************/

/* Sys headers */
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/module.h>

/* Unf headers */

/* Drv headers */

/* Local headers */
#include "mt_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/


/*************************** Structure Definition ****************************/


/***************************** Global Definition *****************************/


/***************************** Static Definition *****************************/


/*********************************** Code ************************************/

mt_char* mt_osal_strncpy(mt_char *pszdest, const mt_char *pszsrc, mt_size_t ullen)
{
    return strncpy(pszdest, pszsrc, ullen);
}

mt_s32 mt_osal_strncmp(const mt_char *pszstr1, const mt_char *pszstr2, mt_size_t ullen)
{
    return strncmp(pszstr1, pszstr2, ullen);
}

mt_s32 mt_osal_strncasecmp(const mt_char *pszstr1, const mt_char *pszstr2, mt_size_t ullen)
{
    return strncasecmp(pszstr1, pszstr2, ullen);
}

mt_char* mt_osal_strncat(mt_char *pszdest, const mt_char *pszsrc, mt_size_t ullen)
{
    return strncat(pszdest, pszsrc, ullen);
}

mt_s32 mt_osal_snprintf(mt_char *pszstr, mt_size_t ullen, const mt_char *pszformat, ...)
{
    mt_s32 s32len;
    va_list stargs = {0};

    va_start(stargs, pszformat);
    s32len = vsnprintf(pszstr, ullen, pszformat, stargs);
    va_end(stargs);

    return s32len;
}

mt_s32 mt_osal_vsnprintf(mt_char *pszstr, mt_size_t ullen, const mt_char *pszformat, mt_va_list_s stvalist)
{
    return vsnprintf(pszstr, ullen, pszformat, stvalist);
}

EXPORT_SYMBOL(mt_osal_strncpy);
EXPORT_SYMBOL(mt_osal_strncmp);
EXPORT_SYMBOL(mt_osal_strncasecmp);
EXPORT_SYMBOL(mt_osal_strncat);
EXPORT_SYMBOL(mt_osal_snprintf);
EXPORT_SYMBOL(mt_osal_vsnprintf);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */

