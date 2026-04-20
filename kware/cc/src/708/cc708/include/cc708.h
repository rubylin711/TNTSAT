/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __CC708_H__
#define __CC708_H__

#include<pthread.h>

#include "mt_type.h"
#include "cc_queue.h"
#include "mt_unf_cc.h"

#ifdef __cplusplus
extern "C" {
#endif


#define USERDATA_QUEUE_SIZE 256
#define USERDATA_BUFFER_SIZE 256
#define SERVICE_QUEUE_SIZE 128


typedef enum tagDTVCC_ModuleState_E
{
    CC_INIT,
    CC_READY,
    CC_RUN,
    CC_STOP
}DTVCC_ModuleState_E;

typedef struct tagDTVCC_Screen_S
{
    MT_UNF_CC_RECT_S PhysicalRect;          /* screen position and size; */
    MT_UNF_CC_RECT_S CaptionRect ;          /* safe-title area(caption area) position and size;*/
    MT_U16           u16MaxHorizonalCells;  /* max horizonal anchor point*/
    MT_U16           u16MaxColumnNum;       /* max cc columns for a window,32 for 4:3 and 42 for 16:9*/
} DTVCC_Screen_S;

typedef struct tagServiceFifo_S
{
    MT_U8   *pu8Buffer;
    MT_U8   u8ServiceLen;
}DTVCC_ServiceElement_S;

typedef struct tagDTVCC_Element_S
{
    MT_U8   *pu8Buffer;
    MT_U16  u16BufferSize;
    MT_U32  u32DataSize;
    MT_U32  u32UserData;
}DTVCC_Element_S;

typedef struct tagDTVCC_CMDBuffer_S
{
    MT_U8   au8CMD[8];
    MT_U8   u8TotalSize;
    MT_U8   u8CurSize;
}DTVCC_CMDBuffer_S;

typedef struct tagDTVCC_Packet_S
{
    MT_U8   u8SeqenceNo;
    MT_U8   u8TotalSize;
    MT_U8   u8CurSize;
    MT_U8   au8PacketData[128];
    MT_BOOL bStart;            /*indicate if received PACKET_START_FLAG*/
}DTVCC_Packet_S;

typedef struct tagDTVCC_ServiceBlock_S
{
    MT_U8   u8ServiceNo;
    MT_U8   u8ServiceSize;
    MT_U8   u8CurSize;
    MT_U8   u8Data[32];
}DTVCC_ServiceBlock_S;

/*******************************************************************************
* CC module object structure
*******************************************************************************/
typedef struct tagCC708_OBJECT_S
{
    MT_U8                  u8ModuleID;
    MT_U8                  u8IsStart;        /*indicate if is ready to start*/
    MT_U8                  u8IsDelay;        /*set to 1 when receive delay command*/
    DTVCC_ModuleState_E    enModuleState;
    CRITICAL_SECTION       mCriticalSection; /*mutual exclusive critical sectio,synchronization objectn*/

    DTVCC_ServiceBlock_S   stServiceBlock;
    MT_S32                 s32LastSeqenceNo;  /*verify if there some sequences missing*/
    MT_BOOL                bServiceBlockEnded;

    DTVCC_Packet_S         stPacket;
    DTVCC_CMDBuffer_S      stCMDBuffer;
    DTVCC_Screen_S         stScreen;            /*Main window defintion*/

    MT_S32                 s32RowHeight;
    MT_S32                 s32MaxCharWidth;
    MT_U8                  u8ScrollInterval;
    MT_U8                  u8ScrollTimes;

    MT_U32                 u32ServiceType;/*Service  #1  is  designated as the Primary Caption Service...*/
    MT_U32                 u32UserFontName;       //font style 0-7
    MT_U32                 u32UserFontStyle;   //font type:Normal=0,Italic=1,Underline&Normal =2, Underline&Italic = 3;
    MT_U32                 u32UserFontSize;   //CC_SMALL_PENSIZE,CC_STANDARD_PENSIZE,CC_LARGE_PENSIZE
    MT_U32                 u32UserTextFGColor;
    MT_U32                 u32UserTextFGOpacity;
    MT_U32                 u32UserTextBgColor;
    MT_U32                 u32UserTextBGOpacity;
    MT_U32                 u32UserWinColor;
    MT_U32                 u32UserWinOpac;
    MT_U32                 u32UserTextEdgetype;
    MT_U32                 u32UserTextEdgecolor;
    MT_U32                 u32CurTextEdgeOpacity;

    TRI_QUEUE              stUserdataQueue;
    TRI_QUEUE              stServiceQueue;
    MT_U8                  au8UserData[USERDATA_QUEUE_SIZE][USERDATA_BUFFER_SIZE];
    MT_U8                  au8ServiceData[SERVICE_QUEUE_SIZE][32];
} CC708_OBJECT_S;


/*****************************************************************************
*                    Extern Function Prototypes
*****************************************************************************/

MT_S32 CC708_Init(MT_VOID);
MT_S32 CC708_DeInit(MT_VOID);

MT_S32 CC708_Create(MT_HANDLE *hcc708);
MT_S32 CC708_Destroy(MT_HANDLE hcc708);

MT_S32 CC708_Start(MT_U8 moduleID);
MT_S32 CC708_Stop(MT_U8 moduleID);
MT_S32 CC708_Reset(MT_U8 moduleID);
MT_U8  CC708_IsStart(MT_U8 moduleID);

MT_S32 CC708_Config(MT_U8 moduleid,MT_UNF_CC_708_CONFIGPARAM_S *pCC708Config);
MT_S32 CC708_GetConfig(MT_U8 moduleid,MT_UNF_CC_708_CONFIGPARAM_S *pstCC708ConfigParam);

MT_S32 CC708_ParseUserData(MT_U8 moduleID, MT_U8 *pu8userdata, MT_U32 u32dataLen, MT_BOOL bTopFieldFirst);
MT_S32 CC708_Userdata_Inject(MT_U8 moduleID, MT_U8 *pu8UserData, MT_U32 u32UsrDataLen, MT_BOOL bTopFieldFirst);
MT_S32 CC708_ProcessData(MT_VOID);
MT_S32 CC708_CharFlash(MT_VOID);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************************************************
*                    End Of File
*****************************************************************************/
