/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_PVR_PRIV_H__
#define __MT_PVR_PRIV_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mt_type.h"
#include "mt_mpi_pvr.h"
#include "mt_pvr_debug.h"
#include "mt_pvr_intf.h"
#include "mt_pvr_cipher_cfg.h"
#include "mt_drv_pvr.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#ifndef STATIC
#define STATIC static
#endif

#ifndef INLINE
#if MT_OS_TYPE == MT_OS_LINUX
#define INLINE //inline
#elif MT_OS_TYPE == MT_OS_WIN32
#define INLINE __inline
#endif
#endif

#if 0
#define PVR_LOCK(p_mutex)        MT_INFO_PVR("==>\n");(void)pthread_mutex_lock(p_mutex);MT_INFO_PVR("==|\n")
#define PVR_UNLOCK(p_mutex)      MT_INFO_PVR("<==\n");(void)pthread_mutex_unlock(p_mutex);MT_INFO_PVR("==|\n")
#else
//#define PVR_LOCK(p_mutex)       (void)pthread_mutex_lock(p_mutex)
//#define PVR_UNLOCK(p_mutex)     (void)pthread_mutex_unlock(p_mutex)

#define PVR_LOCK_REC_VALID(pRecChn)         do{(void)pthread_mutex_lock(&(pRecChn->stMutex_valid));pRecChn->line_magc2=__LINE__;pRecChn->u32magic2++;}while(0)
#define PVR_UNLOCK_REC_VALID(pRecChn)       do{pRecChn->u32magic2--;pRecChn->line_magc2=0;(void)pthread_mutex_unlock(&(pRecChn->stMutex_valid));}while(0)
#define PVR_LOCK_REC(pRecChn)               do{(void)pthread_mutex_lock(&(pRecChn->stMutex));pRecChn->line_magc1=__LINE__;pRecChn->u32magic1++;}while(0)
#define PVR_UNLOCK_REC(pRecChn)             do{pRecChn->u32magic1--;pRecChn->line_magc1=0;(void)pthread_mutex_unlock(&(pRecChn->stMutex));}while(0)

#define PVR_LOCK_PLAY_VALID(pChnAttr)       do{(void)pthread_mutex_lock(&(pChnAttr->stMutex_valid));pChnAttr->line_magc2=__LINE__;pChnAttr->u32magic2++;}while(0)
#define PVR_UNLOCK_PLAY_VALID(pChnAttr)     do{pChnAttr->u32magic2--;pChnAttr->line_magc2=0;(void)pthread_mutex_unlock(&(pChnAttr->stMutex_valid));}while(0)
#define PVR_LOCK_PLAY(pChnAttr)             do{(void)pthread_mutex_lock(&(pChnAttr->stMutex));pChnAttr->line_magc1=__LINE__;pChnAttr->u32magic1++;}while(0)
#define PVR_UNLOCK_PLAY(pChnAttr)           do{pChnAttr->u32magic1--;pChnAttr->line_magc1=0;(void)pthread_mutex_unlock(&(pChnAttr->stMutex));}while(0)
#endif

/* length of TS package                                                     */
#define PVR_TS_LEN           188

/**  */
#define PVR_REC_MIN_DAV_BUF  (PVR_TS_LEN*1024)
/** */
#define PVR_REC_MAX_DAV_BUF  (PVR_TS_LEN*64*1024)
/** */
#define PVR_REC_MIN_SC_BUF   (7*4*1024)
/** */
#define PVR_REC_MAX_SC_BUF   (188*64*1024)


#define PVR_SC_SIZE                     4         /* Byte */
#define PVR_TS_HEAD_SIZE                4
#define PVR_TS_PD_SIZE_POS              4         /* the fifth byte in TS header of the length area of padding */
#define PVR_TS_PD_FLAG_POS              5         /* the sixth byte in TS header of the flag area of padding  */
#define PVR_TS_MIN_PD_SIZE              2


/*
Table 2-6 -- Adaptation field control values
value   description
00  reserved for future use by ISO/IEC
01  no adaptation_field, payload only
10  adaptation_field only, no payload
11  adaptation_field followed by payload
*/
#define PVR_TS_ADAPT_RESERVED    0x0
#define PVR_TS_ADAPT_PLD_ONLY    0x1
#define PVR_TS_ADAPT_ADAPT_ONLY  0x2
#define PVR_TS_ADAPT_BOTH        0x3

#define PVR_TS_ADAPT_HAVE_PLD(flag)   (flag & PVR_TS_ADAPT_PLD_ONLY)
#define PVR_TS_ADAPT_HAVE_ADAPT(flag) (flag & PVR_TS_ADAPT_ADAPT_ONLY)

#define PVR_TIME_CTRL_TIMEBASE_NS      64000000

//#ifndef MT_ADVCA_FUNCTION_RELEASE

#define PVR_PROC_SUPPORT

#ifdef PVR_PROC_SUPPORT
#define PVR_USR_PROC_DIR "pvr"
#define PVR_USR_PROC_REC_ENTRY_NAME "pvr_rec"
#define PVR_USR_PROC_PLAY_ENTRY_NAME "pvr_play"
#endif

/* cipher buffer for decrypt                                                */
typedef struct mtPVR_PHY_BUF_S
{
    MT_U8   *pu8Addr;                                        /* buffer address */
    MT_U32   u32PhyAddr;                                     /* physical address of buffer */
    MT_U32   u32Size;                                        /* buffer size */
} PVR_PHY_BUF_S;

/*
#define DO_FUNC(fun) \
    do{ \
        MT_S32 l_ret = fun; \
        if (l_ret != MT_SUCCESS) \
        { \
            MT_ERR_PVR("%s failed, ERRNO:%#x.\n", #fun, l_ret); \
            return l_ret; \
        } \
    }while(0)

#define DO_FUNC_UNLOCK(fun, pLock) \
    do{ \
        MT_S32 l_ret = fun; \
        if (l_ret != MT_SUCCESS) \
        { \
            MT_ERR_PVR("%s failed, ERRNO:%#x.\n", #fun, l_ret); \
            PVR_UNLOCK(pLock);\
            return l_ret; \
        } \
    }while(0)
*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifdef __MT_PVR_PRIV_H__ */

