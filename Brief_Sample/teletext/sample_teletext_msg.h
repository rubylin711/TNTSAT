/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef SAMPLE_TELETEXT_MSG_H__
#define SAMPLE_TELETEXT_MSG_H__

#include "mt_type.h"
#include <pthread.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagMESSAGE_S
{
    mt_u16  u16MsgLength;                                    /* data length*/
    mt_u8  *pu8MsgData;                                     /*pointer to data*/

    struct tagMESSAGE_S * pstNext;
} MESSAGE_S, *MESSAGE_S_PTR;

typedef struct tagMSG_QUEUE_S
{
    mt_u8*          pu8BaseAddr;            /*Init address,pointer to start position*/
    mt_u32          u32MaxLen;                    /*the max length of this Queue*/
    pthread_mutex_t Queue_lock;
    MESSAGE_S_PTR   pstMsgRear;            /* on receiving, pointer to write data */
    MESSAGE_S_PTR   pstMsgFront;           /* on parsing, pointer to read data */
} MSG_QUEUE_S, *MSG_QUEUE_S_PTR;

mt_s32 MsgQueue_Init(mt_void);

mt_s32 MsgQueue_DeInit(mt_void);

mt_s32 MsgQueue_En(MESSAGE_S * pstMsg);

mt_s32 MsgQueue_De(MESSAGE_S * pstMsg);

#ifdef __cplusplus
}
#endif

#endif
