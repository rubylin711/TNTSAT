

/******************************* Include Files *******************************/

/* Sys headers */
#include <string.h>

/* Unf headers */

/* Drv headers */
#include "mt_osal.h"

/* Local headers */

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

mt_char* mt_osal_strncpy(mt_char *pszDest, const mt_char *pszSrc, mt_size_t ulLen)
{
    return strncpy(pszDest, pszSrc, ulLen);
}

mt_s32 mt_osal_strncmp(const mt_char *pszStr1, const mt_char *pszStr2, mt_size_t ulLen)
{
    return strncmp(pszStr1, pszStr2, ulLen);
}

mt_s32 mt_osal_strncasecmp(const mt_char *pszStr1, const mt_char *pszStr2, mt_size_t ulLen)
{
    return strncasecmp(pszStr1, pszStr2, ulLen);
}

mt_char* mt_osal_strncat(mt_char *pszDest, const mt_char *pszSrc, mt_size_t ulLen)
{
    return strncat(pszDest, pszSrc, ulLen);
}

mt_s32 mt_osal_snprintf(mt_char *pszStr, mt_size_t ulLen, const mt_char *pszFormat, ...)
{
    mt_s32 s32Len;
    va_list stArgs ;

    va_start(stArgs, pszFormat);
    s32Len = vsnprintf(pszStr, ulLen, pszFormat, stArgs);
    va_end(stArgs);

    return s32Len;
}

mt_s32 mt_osal_vsnprintf(mt_char *pszStr, mt_size_t ulLen, const mt_char *pszFormat, mt_va_list_s stVAList)
{
    return vsnprintf(pszStr, ulLen, pszFormat, stVAList);
}


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */


