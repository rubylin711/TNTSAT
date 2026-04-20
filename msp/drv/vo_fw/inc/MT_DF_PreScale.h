/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef MT_DF_PRESCALE_H
#define MT_DF_PRESCALE_H

#include"MT_DF_common.h"
#if defined(CONFIG_MT_CHIP_ARIA)
#include "MT_DF_Aria_reg.h"
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
#include "MT_DF_Symphony_reg.h"
#endif
#include"MT_DF_ScalarPara.h"

//#define PS_TEST

#ifdef __cplusplus
extern "C" {
#endif

#define PS_ORDER_QUEUE_LENGTH 10

#if 0
typedef enum tagPreScale_OrderStatus
{
    PS_ORDER_STATUS_NEW=0,
    PS_ORDER_STATUS_INQUEUE,
    PS_ORDER_STATUS_PROCESSING,
    PS_ORDER_STATUS_TERMINATED,
    PS_ORDER_STATUS_DONE
}PRESCALE_ORDER_STATUS;
#endif

typedef enum tagPreScale_HWStatus
{
    PS_HW_STATUS_UNINITILIZED=0,
    PS_HW_STATUS_FREE,//free flag
    PS_HW_STATUS_RUNNING,//from the order'running to the end irq
    PS_HW_STATUS_INTERMEDIATE//from the end irq to the free flag
}PRESCALE_HW_STATUS;

#if 0
typedef enum tagPreScale_OrderPriority
{
    PS_ORDER_PRI_LOW=0,
    PS_ORDER_PRI_HIGH
}PRESCALE_ORDER_PRIORITY;

typedef enum tagPreScale_IOMode
{
    FRAMER_FRAMEW_ONEFRAME=0,   //Mode=0: frame read,     frame write,        one frame
    INTERR_INTERW_TOPBOT,       //Mode=1: interlace read, interlace write,    top+bot
    INTERR_INTERW_TOPONLY,      //Mode=2: interlace read, interlace write,    top only
    INTERR_INTERW_BOTONLY,      //Mode=3: interlace read, interlace write,    bot only
    HEVCR_INTERW_TOPBOT,        //Mode=4: HEVC read,      interlace write,    top+bot
    HEVCR_INTERW_TOPONLY,       //Mode=5: HEVC read,      interlace write,    top only
    HEVCR_INTERW_BOTONLY        //Mode=6: HEVC read,      interlace write,    top only

}PRESCALE_IOMode;

typedef struct tagPreScale_OrderInfo
{
    //status
    PRESCALE_ORDER_STATUS  eOrderStatus;
    //priority
    PRESCALE_ORDER_PRIORITY eOrderPriority;
    //data info
    MT_U32 u32LumaAddrRead;
    MT_U32 u32LumaAddrRead2;
    MT_U32 u32LumaAddrWrite;
    MT_U32 u32ChromaAddrRead;
    MT_U32 u32ChromaAddrRead2;
    MT_U32 u32ChromaAddrWrite;
    MT_U32 u32SrcWidth;
    MT_U32 u32SrcHeight;
    MT_U32 u32DstWidth;
    MT_U32 u32DstHeight;
    MT_U32 u32DstStride;
    //control info
    PRESCALE_IOMode eMode;

    MT_DF_BOOL bIsTileMode;

    //tile
    MT_U32 row_jump_value;
    MT_U32 row_jump_offset;
    MT_U8 u8TileCfg;
    MT_U8 u8ColSize;
    MT_U8 u8FieldPicture;

    //linear input
    MT_U32 u32LinearInputStride;

    //endian
    MT_U8 u8InputEndian;
    MT_U8 u8OutputEndian;

    //UV change
    MT_U8 u8UVChange;

}PRESCALE_ORDERINFO;
#endif

typedef struct tagMT_PreScale_Node
{
    MT_U32 u32NodeIndex;
    PRESCALE_ORDERINFO * pstOrderInfo;
    //PRESCALE_NODE * pNext;
    void * pNext;
}PRESCALE_NODE;

MT_RET PS_CheckOrderInQueue(PRESCALE_ORDERINFO* pstOrderInfo);
MT_RET PS_Init(mt_void);
MT_RET PS_UnInit(mt_void);
MT_RET PS_AddOrder(PRESCALE_ORDERINFO* pstOrderInfo);
void PS_TaskLoopFunc(mt_void);
void PS_Loop(mt_void);
void PS_RunOrder_Test(mt_void);

#ifdef __cplusplus
}
#endif
#endif
