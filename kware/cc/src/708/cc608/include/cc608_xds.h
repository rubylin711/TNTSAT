/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#ifndef _CC608_XDS_H_
#define _CC608_XDS_H_

#include "cc608.h"
#include "mt_cc608_def.h"
#include "mt_type.h"

#if defined __cplusplus || defined __cplusplus__
extern "C" {
#endif

/*****************************************************************************
*                    Macro Definitions
*****************************************************************************/

#define MAX_XDS_PACKET_LEN      0x21
#define MAX_XDS_OBJECT_NUM      0x02

/*Control Codes*/
#define CLASS_CURRENT_START     0x01     /*Current Packet Start Code*/
#define CLASS_CURRENT_CONTINUE  0x02     /*current Packet continue code*/
#define CLASS_FUTURE_START      0x03
#define CLASS_FUTURE_CONTINUE   0x04
#define CLASS_CHANNEL_START     0x05
#define CLASS_CHANNEL_CONTINUE  0x06
#define CLASS_MISC_START        0x07     /*use to decode timestamp*/
#define CLASS_MISC_CONTINUE     0x08
#define CLASS_PUBLIC_START      0x09
#define CLASS_PUBLIC_CONTINUE   0x0A
#define CLASS_RESERVED_START    0x0B
#define CLASS_RESERVED_CONTINUE 0x0C
#define CLASS_PRIVATE_START     0x0D
#define CLASS_PRIVATE_CONTINUE  0x0E

#define CLASS_PACKET_END        0x0F


/*****************************************************************************
*                    Type Definitions
*****************************************************************************/

/**Class Definitions in XDS Packet*//** CNcomment:XDS的分类 */
typedef enum mtUNF_CC_XDS_CLASS_E
{
    MT_UNF_CC_XDS_CLASS_CUR,       /**<Current class*//**<CNcomment:Current类 */
    MT_UNF_CC_XDS_CLASS_FUT,       /**<Future class*//**<CNcomment:Future类 */
    MT_UNF_CC_XDS_CLASS_CHAN,      /**<Channel Information class*//**<CNcomment:Channel类 */
    MT_UNF_CC_XDS_CLASS_MISC,      /**<Miscellaneous class*//**<CNcomment:Miscellaneous类 */
    MT_UNF_CC_XDS_CLASS_PUB,       /**<Public Service class*//**<CNcomment:Public Service类 */
    MT_UNF_CC_XDS_CLASS_RESV,      /**<Reserved class*//**<CNcomment:预留 */
    MT_UNF_CC_XDS_CLASS_PRIV,      /**<Private Data class*//**<CNcomment:Private Data类 */
    MT_UNF_CC_XDS_CLASS_BUTT
} MT_UNF_CC_XDS_CLASS_E;

/**Type of current Class*//** CNcomment:Current类的类别 */
typedef enum mtUNF_CC_XDS_CUR_TYPE_E
{
    MT_UNF_CC_XDS_CUR_PRG_ID        =   0x1,  /**<Program Identification Number*//**<CNcomment:节目的开始时间和日期 */
    MT_UNF_CC_XDS_CUR_TIMEINSHOW    =   0x2,  /**<Length/Time-in-Show*//**<CNcomment:节目的总时间 */
    MT_UNF_CC_XDS_CUR_PRG_NAME      =   0x3,  /**<Program Name*//**<CNcomment:节目名字 */
    MT_UNF_CC_XDS_CUR_PRG_TYPE      =   0x4,  /**<Program Type*//**<CNcomment:节目类型 */
    MT_UNF_CC_XDS_CUR_CONT_ADVSR    =   0x5,  /**<Content Advisory*//**<CNcomment:节目内容的限制级别 */
    MT_UNF_CC_XDS_CUR_AUD_SERVC     =   0x6,  /**<Audio Services*//**<CNcomment:音频节目的内容服务 */
    MT_UNF_CC_XDS_CUR_CAP_SERVC     =   0x7,  /**<Caption Services*//**<CNcomment:字幕服务 */
    MT_UNF_CC_XDS_CUR_CPY_REDIST    =   0x8,  /**<Copy and Redistribution Control Packet*//**<CNcomment:内容拷贝和发布控制 */
    MT_UNF_CC_XDS_CUR_COMP_1        =   0xC,  /**<Composite Packet-1*//**<CNcomment:综合内容包 */
    MT_UNF_CC_XDS_CUR_COMP_2        =   0xD,  /**<Composite Packet-2*//**<CNcomment:综合内容包 */
    MT_UNF_CC_XDS_CUR_PRG_DESC1     =   0x10, /**<Program Description Row 1*//**<CNcomment:节目的内容描述 */
    MT_UNF_CC_XDS_CUR_PRG_DESC2     =   0x11, /**<Program Description Row 2*//**<CNcomment:节目的内容描述 */
    MT_UNF_CC_XDS_CUR_PRG_DESC3     =   0x12, /**<Program Description Row 3*//**<CNcomment:节目的内容描述 */
    MT_UNF_CC_XDS_CUR_PRG_DESC4     =   0x13, /**<Program Description Row 4*//**<CNcomment:节目的内容描述 */
    MT_UNF_CC_XDS_CUR_PRG_DESC5     =   0x14, /**<Program Description Row 5*//**<CNcomment:节目的内容描述 */
    MT_UNF_CC_XDS_CUR_PRG_DESC6     =   0x15, /**<Program Description Row 6*//**<CNcomment:节目的内容描述 */
    MT_UNF_CC_XDS_CUR_PRG_DESC7     =   0x16, /**<Program Description Row 7*//**<CNcomment:节目的内容描述 */
    MT_UNF_CC_XDS_CUR_PRG_DESC8     =   0x17, /**<Program Description Row 8*//**<CNcomment:节目的内容描述 */
    MT_UNF_CC_XDS_CUR_TYPE_BUTT
} MT_UNF_CC_XDS_CUR_TYPE_E;

/**Type of Channel Information Class*//** CNcomment:频道信息类的类别 */
typedef enum mtUNF_CC_XDS_CHAN_TYPE
{
    MT_UNF_CC_XDS_CHAN_NET_NAME      =   0x1,  /**<Network Name*//**<CNcomment:网络名称 */
    MT_UNF_CC_XDS_CHAN_CALL_LETTER   =   0x2,  /**<Call Letters (Station ID) and Native Channel*//**<CNcomment:Station ID或者Native Channel */
    MT_UNF_CC_XDS_CHAN_TAPE_DELAY    =   0x3,  /**<Tape Delay*//**<CNcomment:Tape延迟 */
    MT_UNF_CC_XDS_CHAN_TRNS_SIGNID   =   0x4,  /**<Transmission Signal Identifier (TSID)*//**<CNcomment:传输信号标识 */
    MT_UNF_CC_XDS_CHAN_TYPE_BUTT
} MT_UNF_CC_XDS_CHAN_TYPE;

/**Type of Miscellaneous Class*//** CNcomment:Miscellaneous类的类别 */
typedef enum mtUNF_CC_XDS_MISC_TYPE
{
    MT_UNF_CC_XDS_MISC_TIMEOFDAY     =   0x1,  /**<Time of Day*//**<CNcomment:当前时间 */
    MT_UNF_CC_XDS_MISC_IMPL_CAPTID   =   0x2,  /**<Impulse Capture ID*//**<CNcomment:录制ID */
    MT_UNF_CC_XDS_MISC_SUPL_DATALOC  =   0x3,  /**<Supplemental Data Location*//**<CNcomment:其他数据的位置 */
    MT_UNF_CC_XDS_MISC_TIME_ZONE     =   0x4,  /**<Local Time Zone & DST Use*//**<CNcomment:加入夏令时的时间 */
    MT_UNF_CC_XDS_MISC_OUTOFBAND     =   0x40, /**<Out-of-Band Channel Number*//**<CNcomment:其他频道的频道号 */
    MT_UNF_CC_XDS_MISC_CHMAP_POINTER =   0x41, /**<Channel Map Pointer*//**<CNcomment:频道映射包的频道索引 */
    MT_UNF_CC_XDS_MISC_CHMAP_HEADER  =   0x42, /**<Channel Map Header Packet*//**<CNcomment:频道映射包头 */
    MT_UNF_CC_XDS_MISC_CHMAP_PAKT    =   0x43, /**<Channel Map Packet*//**<CNcomment:频道映射包 */
    MT_UNF_CC_XDS_MISC_TYPE_BUTT
} MT_UNF_CC_XDS_MISC_TYPE;

/**Type of Public Service Class*//** CNcomment:公共服务类的类别 */
typedef enum mtUNF_CC_XDS_PUB_TYPE
{
    MT_UNF_CC_XDS_PUB_NWS_CODE      =   0x1,  /*National Weather Service Code (WRSAME)*//**<CNcomment:国内气象信息码 */
    MT_UNF_CC_XDS_PUB_NWS_MSG       =   0x2,  /*National Weather Service Message*//**<CNcomment:国内气象信息消息 */
    MT_UNF_CC_XDS_PUB_TYPE_BUTT
} MT_UNF_CC_XDS_PUB_TYPE;

#define CC608_XDS_MAX_TYPE MT_UNF_CC_XDS_MISC_TYPE_BUTT

typedef struct tagXDS_OBJECT_S
{
    MT_U8                       module_id;

    // current XDS information
    MT_U8                       u8IsPacketStarted;
    MT_UNF_CC_XDS_CLASS_E       enCurXDSClass;
    MT_U8                       u8CurXDSType;
    MT_U8                       au8PacketData[MAX_XDS_PACKET_LEN];
    MT_U8                       u8PacketLen;

    // backup XDS information for interleaved XDS packets
    MT_U8                       u8IsBackupStarted;
    MT_UNF_CC_XDS_CLASS_E       enBackupXDSClass;
    MT_U8                       u8BackupXDSType;
    MT_U8                       au8BackupPacketData[MAX_XDS_PACKET_LEN];
    MT_U8                       u8BackupPacketLen;

    MT_BOOL                     bUpdated[MT_UNF_CC_XDS_CLASS_BUTT][CC608_XDS_MAX_TYPE];
    pthread_mutex_t             XDSMutex;
}XDS_OBJECT_S;

/*****************************************************************************
*                    Extern Data Declarations
*****************************************************************************/

 /*****************************************************************************
*                    Extern Function Prototypes
*****************************************************************************/
MT_S32 CC608_XDS_Create(MT_U8 u8ModuleID);

MT_S32 CC608_XDS_Destroy(MT_U8 u8ModuleID);

void CC608_XDS_Reset(MT_U8 u8ModuleID);

MT_S32 CC608_XDS_XDSDecode(MT_U8 u8ModuleID, MT_U8 b1, MT_U8 b2);


#if defined __cplusplus || defined __cplusplus__
}
#endif

#endif

/*****************************************************************************
*                    End Of File
*****************************************************************************/
