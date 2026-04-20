/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_PVR_DEBUG_H__
#define __MT_PVR_DEBUG_H__

#include <stdlib.h>

#include "mt_error_mpi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* End of #ifdef __cplusplus */

#if 0
#define PVR_FIFO_D(fmt,...)    printf("PVR FIFO D: %s(%d) "fmt,__FUNCTION__, __LINE__,##__VA_ARGS__)
#else
#define PVR_FIFO_D(fmt,...)
#endif

#if 0
#define PVR_IDX_D(fmt,...)    printf("PVR IDX D: %s(%d) "fmt,__FUNCTION__, __LINE__,##__VA_ARGS__)
#else
#define PVR_IDX_D(fmt,...)
#endif

#if 0
#define PVR_REC_D(fmt,...)    printf("PVR REC D: %s(%d) "fmt,__FUNCTION__, __LINE__,##__VA_ARGS__)
#else
#define PVR_REC_D(fmt,...)
#endif

#if 0
#define PVR_PLAY_D(fmt,...)    printf("PVR PLAY D: %s(%d) "fmt,__FUNCTION__, __LINE__,##__VA_ARGS__)
#else
#define PVR_PLAY_D(fmt,...)
#endif

//default print
#define PVR_FIFO_P(fmt,...)    printf("PVR FIFO P: %s(%d) "fmt,__FUNCTION__, __LINE__,##__VA_ARGS__)
#define PVR_IDX_P(fmt,...)    printf("PVR IDX P: %s(%d) "fmt,__FUNCTION__, __LINE__,##__VA_ARGS__)
#define PVR_REC_P(fmt,...)    printf("PVR REC P: %s(%d) "fmt,__FUNCTION__, __LINE__,##__VA_ARGS__)
#define PVR_PLAY_P(fmt,...)    printf("PVR PLAY P: %s(%d) "fmt,__FUNCTION__, __LINE__,##__VA_ARGS__)

extern MT_U32 g_pvrplay_loglevel;
#undef LOG_TAG_PVRPLAY
#define LOG_TAG_PVRPLAY                   "PVRPLAY"



#define PVRPLAY_DEBUG_PLAY_OVER_INFO        (0x00000001)
#define PVRPLAY_DEBUG_FRAME_INFO                (0x00000002)
#define PVRPLAY_DEBUG_PUSH_QUEUE_INFO     (0x00000004)
#define PVRPLAY_DEBUG_PLAY_TIME_INFO         (0x00000008)
#define PVRPLAY_DEBUG_SEND_FRAME_INFO     (0x00000010)
#define PVRPLAY_DEBUG_TRICK_POLICY_INFO     (0x00000100)


#define PVRPLAY_LOG_PLAY_OVER(fmt, ...)       do{if (g_pvrplay_loglevel & PVRPLAY_DEBUG_PLAY_OVER_INFO){printf("["LOG_TAG_PVRPLAY"] %s(%d) " fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define PVRPLAY_LOG_DEBUG_FRAME(fmt, ...) do{if (g_pvrplay_loglevel & PVRPLAY_DEBUG_FRAME_INFO){printf("["LOG_TAG_PVRPLAY"] %s(%d) " fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define PVRPLAY_LOG_PUSH_QUEUE(fmt, ...)   do{if (g_pvrplay_loglevel & PVRPLAY_DEBUG_PUSH_QUEUE_INFO){printf("["LOG_TAG_PVRPLAY"] " fmt, ## __VA_ARGS__);}}while(0)
#define PVRPLAY_LOG_PLAY_TIME(fmt, ...)       do{if (g_pvrplay_loglevel & PVRPLAY_DEBUG_PLAY_TIME_INFO){printf("["LOG_TAG_PVRPLAY"] %s(%d) " fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define PVRPLAY_LOG_SEND_FRAME(fmt, ...)   do{if (g_pvrplay_loglevel & PVRPLAY_DEBUG_SEND_FRAME_INFO){printf("["LOG_TAG_PVRPLAY"] %s(%d) " fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)
#define PVRPLAY_LOG_TRICK_POLICY(fmt, ...)   do{if (g_pvrplay_loglevel & PVRPLAY_DEBUG_TRICK_POLICY_INFO){printf("["LOG_TAG_PVRPLAY"] %s(%d) " fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__);}}while(0)



/* assert NULL pointer                                                      */
#define PVR_CHECK_POINTER(PTR)\
    do\
    {\
        if ( MT_NULL_PTR == PTR )\
        {\
            MT_ERR_PVR("Parameter is NULL.\n");\
            return MT_ERR_PVR_NUL_PTR;\
        }\
    } while (0)

#define PVR_CHECK_CIPHER_CFG(pCipherCfg)\
    do {\
            if ((pCipherCfg)->bDoCipher)\
            {\
                if ((pCipherCfg)->enType >= MT_CIPHER_ALG_BUTT\
                    || (!((pCipherCfg)->u32KeyLen >= (PVR_CIPHER_AES_KEY_LENGTH_BIT/8)) \
                         && ((pCipherCfg)->u32KeyLen <= PVR_MAX_CIPHER_KEY_LEN))) \
                {\
                    MT_ERR_PVR("Invalid cipher config: type error or key len(%u) invalid!\n",(pCipherCfg)->u32KeyLen);\
                    return MT_ERR_PVR_INVALID_PARA;\
                }\
            }\
       }while(0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __MT_PVR_DEBUG_H__ */
