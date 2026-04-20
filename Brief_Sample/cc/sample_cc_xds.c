/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <string.h>
#include <stdio.h>
#include "mt_debug.h"
#include "sample_cc_xds.h"
#include "sample_cc_common.h"
/***************************** Macro Definition ******************************/

#define MAX_PROGRAM_NAME_BYTE           33
#define MAX_CAPTION_SERVICES_LEN        8
#define MAX_COMPOSITE_PACKET_LEN        32
#define MAX_PROGRAM_NUM                 8
#define MAX_PROGRAM_DESCRIPTION_LEN     32
#define MAX_CALLLETTER_LEN              6
#define MAX_TSID_LEN                    4
#define MAX_SUPPLEMT_DATA_LEN           32
#define MAX_CHANNELMAP_HEADER_LEN       4
#define MAX_CHANNELMAP_PACKET_LEN       10
#define MAX_NWS_CODE_LEN                11
#define MAX_NWS_DATA_LEN                32

#define MIN_CC608_PROGNAME_LEN          2
#define MAX_CC608_PROGNAME_LEN          32
#define MIN_CC608_PROGTYPE_LEN          2
#define MAX_CC608_PROGTYPE_LEN          32
#define MIN_CC608_CAPTSERV_LEN          2
#define MAX_CC608_CAPTSERV_LEN          8
#define MAX_CC608_PROG_NUM              8
#define MAX_CC608_PROGDESC_LEN          32
#define MIN_CC608_NETNAME_LEN           2
#define MAX_CC608_NETNAME_LEN           32
#define MIN_CC608_CALLLET_LEN           4
#define MAX_CC608_CALLLET_LEN           6
#define MIN_CC608_SUPPDATA_LEN          2
#define MAX_CC608_SUPPDATA_LEN          32
#define MAX_CC608_CHID_LEN              6
#define MAX_CC608_NWSCODE_LEN           32
#define MAX_CC608_NWSMSG_LEN            32
/*************************** Structure Definition ****************************/
/**Class Definitions in XDS Packet*//** CNcomment:XDS的分类 */
typedef enum tagXDS_CLASS_E
{
    XDS_CLASS_CUR,       /**<Current class*//**<CNcomment:Current类 */
    XDS_CLASS_FUT,       /**<Future class*//**<CNcomment:Future类 */
    XDS_CLASS_CHAN,      /**<Channel Information class*//**<CNcomment:Channel类 */
    XDS_CLASS_MISC,      /**<Miscellaneous class*//**<CNcomment:Miscellaneous类 */
    XDS_CLASS_PUB,       /**<Public Service class*//**<CNcomment:Public Service类 */
    XDS_CLASS_RESV,      /**<Reserved class*//**<CNcomment:预留 */
    XDS_CLASS_PRIV,      /**<Private Data class*//**<CNcomment:Private Data类 */
    XDS_CLASS_BUTT
} XDS_CLASS_E;

/**Type of current Class*//** CNcomment:Current类的类别 */
typedef enum tagXDS_CUR_TYPE_E
{
    XDS_CUR_PRG_ID        =   0x1,  /**<Program Identification Number*//**<CNcomment:节目的开始时间和日期 */
    XDS_CUR_TIMEINSHOW    =   0x2,  /**<Length/Time-in-Show*//**<CNcomment:节目的总时间 */
    XDS_CUR_PRG_NAME      =   0x3,  /**<Program Name*//**<CNcomment:节目名字 */
    XDS_CUR_PRG_TYPE      =   0x4,  /**<Program Type*//**<CNcomment:节目类型 */
    XDS_CUR_CONT_ADVSR    =   0x5,  /**<Content Advisory*//**<CNcomment:节目内容的限制级别 */
    XDS_CUR_AUD_SERVC     =   0x6,  /**<Audio Services*//**<CNcomment:音频节目的内容服务 */
    XDS_CUR_CAP_SERVC     =   0x7,  /**<Caption Services*//**<CNcomment:字幕服务 */
    XDS_CUR_CPY_REDIST    =   0x8,  /**<Copy and Redistribution Control Packet*//**<CNcomment:内容拷贝和发布控制 */
    XDS_CUR_COMP_1        =   0xC,  /**<Composite Packet-1*//**<CNcomment:综合内容包 */
    XDS_CUR_COMP_2        =   0xD,  /**<Composite Packet-2*//**<CNcomment:综合内容包 */
    XDS_CUR_PRG_DESC1     =   0x10, /**<Program Description Row 1*//**<CNcomment:节目的内容描述 */
    XDS_CUR_PRG_DESC2     =   0x11, /**<Program Description Row 2*//**<CNcomment:节目的内容描述 */
    XDS_CUR_PRG_DESC3     =   0x12, /**<Program Description Row 3*//**<CNcomment:节目的内容描述 */
    XDS_CUR_PRG_DESC4     =   0x13, /**<Program Description Row 4*//**<CNcomment:节目的内容描述 */
    XDS_CUR_PRG_DESC5     =   0x14, /**<Program Description Row 5*//**<CNcomment:节目的内容描述 */
    XDS_CUR_PRG_DESC6     =   0x15, /**<Program Description Row 6*//**<CNcomment:节目的内容描述 */
    XDS_CUR_PRG_DESC7     =   0x16, /**<Program Description Row 7*//**<CNcomment:节目的内容描述 */
    XDS_CUR_PRG_DESC8     =   0x17, /**<Program Description Row 8*//**<CNcomment:节目的内容描述 */
    XDS_CUR_TYPE_BUTT
} XDS_CUR_TYPE_E;

/**Type of Channel Information Class*//** CNcomment:频道信息类的类别 */
typedef enum tagXDS_CHAN_TYPE
{
    XDS_CHAN_NET_NAME      =   0x1,  /**<Network Name*//**<CNcomment:网络名称 */
    XDS_CHAN_CALL_LETTER   =   0x2,  /**<Call Letters (Station ID) and Native Channel*//**<CNcomment:Station ID或者Native Channel */
    XDS_CHAN_TAPE_DELAY    =   0x3,  /**<Tape Delay*//**<CNcomment:Tape延迟 */
    XDS_CHAN_TRNS_SIGNID   =   0x4,  /**<Transmission Signal Identifier (TSID)*//**<CNcomment:传输信号标识 */
    XDS_CHAN_TYPE_BUTT
} XDS_CHAN_TYPE;

/**Type of Miscellaneous Class*//** CNcomment:Miscellaneous类的类别 */
typedef enum tagXDS_MISC_TYPE
{
    XDS_MISC_TIMEOFDAY     =   0x1,  /**<Time of Day*//**<CNcomment:当前时间 */
    XDS_MISC_IMPL_CAPTID   =   0x2,  /**<Impulse Capture ID*//**<CNcomment:录制ID */
    XDS_MISC_SUPL_DATALOC  =   0x3,  /**<Supplemental Data Location*//**<CNcomment:其他数据的位置 */
    XDS_MISC_TIME_ZONE     =   0x4,  /**<Local Time Zone & DST Use*//**<CNcomment:加入夏令时的时间 */
    XDS_MISC_OUTOFBAND     =   0x40, /**<Out-of-Band Channel Number*//**<CNcomment:其他频道的频道号 */
    XDS_MISC_CHMAP_POINTER =   0x41, /**<Channel Map Pointer*//**<CNcomment:频道映射包的频道索引 */
    XDS_MISC_CHMAP_HEADER  =   0x42, /**<Channel Map Header Packet*//**<CNcomment:频道映射包头 */
    XDS_MISC_CHMAP_PAKT    =   0x43, /**<Channel Map Packet*//**<CNcomment:频道映射包 */
    XDS_MISC_TYPE_BUTT
} XDS_MISC_TYPE;

/**Type of Public Service Class*//** CNcomment:公共服务类的类别 */
typedef enum mtUNF_CC_XDS_PUB_TYPE
{
    MT_UNF_CC_XDS_PUB_NWS_CODE      =   0x1,  /*National Weather Service Code (WRSAME)*//**<CNcomment:国内气象信息码 */
    MT_UNF_CC_XDS_PUB_NWS_MSG       =   0x2,  /*National Weather Service Message*//**<CNcomment:国内气象信息消息 */
    MT_UNF_CC_XDS_PUB_TYPE_BUTT
} MT_UNF_CC_XDS_PUB_TYPE;

typedef struct _tagXDS_ProgramIDNum_S
{
    mt_u8 u8Minute;
    mt_u8 u8Hour;
    mt_u8 u8Data;
    mt_u8 u8Month;
}XDS_ProgramIDNum_S;

typedef struct _tagXDS_LenTimeInShow_S
{
    mt_u8 u8LengthMin;
    mt_u8 u8LengthHour;
    mt_u8 u8ElapsedMin;
    mt_u8 u8ElapsedHour;
    mt_u8 u8ElapsedSec;
}XDS_LenTimeInShow_S;

typedef struct _tagXDSProgName_S
{
    mt_u8 au8ProgName[MAX_CC608_PROGNAME_LEN];
    mt_u8 u8ProgNameLen;
}XDSProgName_S;

typedef struct _tagXDSProgType_S
{
    mt_u8 au8ProgType[MAX_CC608_PROGTYPE_LEN];
    mt_u8 u8ProgTypeLen;
}XDSProgType_S;

typedef struct _tagXDSContAdv_S
{
    mt_u8 u8Char1;
    mt_u8 u8Char2;
}XDSContAdv_S;

typedef struct _tagXDSAudioService_S
{
    mt_u8 u8Main;
    mt_u8 u8SAP;
}XDSAudioService_S;

typedef struct _tagXDSCaptService_S
{
    mt_u8 au8CaptServices[MAX_CC608_CAPTSERV_LEN];
    mt_u8 u8CaptServicesLen;
}XDSCaptService_S;

typedef struct _tagXDSCopyAndRedist_S
{
    mt_u8 u8Byte1;
    mt_u8 u8Byte2;
}XDSCopyAndRedist_S;

typedef struct _tagXDSCompPacket1_S
{
    mt_u8 au8ProgramType[5];
    mt_u8 u8ContAdv;
    mt_u8 u8LengthMin;
    mt_u8 u8LengthHour;
    mt_u8 u8ElapsedMin;
    mt_u8 u8ElapsedHour;
    mt_u8 au8Title[22];
    mt_u8 u8TitleLen;
}XDSCompPacket1_S;

typedef struct _tagXDSCompPacket2_S
{
    XDS_ProgramIDNum_S stPRGStartTime;
    XDSAudioService_S stAudioServices;
    mt_u8 au8CaptServices[2];
    mt_u8 au8CallLetter[4];
    mt_u8 au8NativeChan[2];
    mt_u8 au8NetworkName[18];
    mt_u8 au8NetworkNameLen;
}XDSCompPacket2_S;

typedef struct _tagXDSProgDesc_S
{
    mt_u8 au8ProgDesc[MAX_CC608_PROGDESC_LEN];
    mt_u8 u8ProgDescLen;
}XDSProgDesc_S;

typedef struct _tagXDSNetworkName_S
{
    mt_u8 au8NetName[MAX_CC608_NETNAME_LEN];
    mt_u8 u8NetNameLen;
}XDSNetworkName_S;

typedef struct _tagXDSCallLetNatvCh_S
{
    mt_u8 au8CallLetter[4];
    mt_u8 au8NativeChan[2];
    mt_u8 u8NativeChanLen;
}XDSCallLetNatvCh_S;

typedef struct _tagXDSTapeDelay_S
{
    mt_u8 u8Minute;
    mt_u8 u8Hour;
}XDSTapeDelay_S;

typedef struct _tagXDSTranSignID_S
{
    mt_u8 u8TSID3to0;
    mt_u8 u8TSID7to4;
    mt_u8 u8TSID11to8;
    mt_u8 u8TSID15to12;
}XDSTranSignID_S;

typedef struct _tagXDSTimeOfDay_S
{
    mt_u8 u8Minute;
    mt_u8 u8Hour;
    mt_u8 u8Date;
    mt_u8 u8Month;
    mt_u8 u8Day;
    mt_u8 u8Year;
}XDSTimeOfDay_S;

typedef struct _tagXDSImpCaptID_S
{
    mt_u8 u8Minute;
    mt_u8 u8Hour;
    mt_u8 u8Date;
    mt_u8 u8Month;
    mt_u8 u8LengthMin;
    mt_u8 u8LengthHour;
}XDSImpCaptID_S;

typedef struct  _tagXDSSupplDataLoc_S
{
    mt_u8 au8SupplDataLoc[MAX_CC608_SUPPDATA_LEN];
    mt_u8 u8SupplDataLocLen;
}XDSSupplDataLoc_S;

typedef struct _tagXDSLocTimeZone_S
{
    mt_u8 u8LocTimeZoneHour;
}XDSLocTimeZone_S;

typedef struct _tagXDSOutOfBand_S
{
    mt_u8 u8ChanLow;
    mt_u8 u8ChanHigh;
}XDSOutOfBand_S;

typedef struct _tagXDSChMapPointer_S
{
    mt_u8 u8TuneChanLow;
    mt_u8 u8TuneChanHigh;
}XDSChMapPointer_S;

typedef struct _tagXDSChMapHeader_S
{
    mt_u8 u8ChanLow;
    mt_u8 u8ChanHigh;
    mt_u8 u8Version;
}XDSChMapHeader_S;

typedef struct _tagXDSChMapPacket_S
{
    mt_u8 u8UserChanLow;
    mt_u8 u8UserChanHigh;
    mt_u8 u8TuneChanLow;
    mt_u8 u8TuneChanHigh;
    mt_u8 au8ChID[MAX_CC608_CHID_LEN];
    mt_u8 u8ChIDLen;
}XDSChMapPacket_S;

typedef struct _tagXDSNatWeaServcCode_S
{
    mt_u8 au8NatWeaServcCode[MAX_CC608_NWSCODE_LEN];
    mt_u8 u8NatWeaServcCodeLen;
}XDSNatWeaServcCode_S;

typedef struct _tagXDSNatWeaServcMsg_S
{
    mt_u8 u8NatWeaServcMsg[MAX_CC608_NWSMSG_LEN];
    mt_u8 u8NatWeaServcMsgLen;
}XDSNatWeaServcMsg_S;

typedef mt_s32 (*XDSPacketDecoder_FN)(mt_u8 *pu8PacketData, mt_u8 u8PacketLen);
/********************** Global Variable declaration **************************/
static XDSPacketDecoder_FN  g_pfnXDSPacketDecoder[XDS_CLASS_BUTT][XDS_MISC_TYPE_BUTT];
/******************************* API declaration *****************************/
static mt_s32 _CC608_XDS_DecodeProgIDNum(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8              *buf = NULL;
    mt_u8              len = 0;
    mt_u8              Minute = 0;
    mt_u8              Hour = 0;
    mt_u8              Date = 0;
    mt_u8              Month = 0;
    XDS_ProgramIDNum_S astProgramIDNum[1] = { 0 };

    len = u8PacketLen;
    if(len != 4)
    {
        SAMPLE_CC_ERR_PRINT("Invalide length: %d for Data/Time Packet!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    Minute = (buf[0] & 0x3F);
    Hour = (buf[1] & 0x1F);
    Date = (buf[2] & 0x1F);
    Month = (buf[3] & 0x0F);
    if((Minute > 59) || (Hour > 23) || (Date > 31) || (Month > 12))
    {
        SAMPLE_CC_ERR_PRINT("Invalide Time/Date Value: Minute = %d, Hour = %d, Date = %d, Month = %d\n", Minute, Hour, Date, Month);
        return MT_FAILURE;
    }

    astProgramIDNum->u8Minute = buf[0];
    astProgramIDNum->u8Hour = buf[1];
    astProgramIDNum->u8Data = buf[2];
    astProgramIDNum->u8Month = buf[3];

    SAMPLE_CC_INFO_PRINT("Program Start Time: %.2d-%.2d %.2d : %.2d\n", Month, Date, Hour, Minute);
    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeLenAndTimeInShow(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8               *buf = NULL;
    mt_u8               len = 0;
    mt_u8               length[6] = { 0 };
    XDS_LenTimeInShow_S astLenTimeInShow[1] = { 0 };

    len = u8PacketLen;
    if(len >  6)
    {
        SAMPLE_CC_ERR_PRINT("Invalide length: %d for length/Elapse Packet!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    memset(length, 0, 6);
    for(mt_u8 i = 0; i < len; i++)
    {
        length[i] = (buf[i] & 0x3F);
    }

    if((length[0] > 59) || (length[2] > 59) || (length[4] > 59))
    {
        SAMPLE_CC_ERR_PRINT("Invalide Show length value: len(min) = %d, ET(min) = %d, ET(s) = %d\n", length[0], length[2], length[4]);
        return MT_FAILURE;
    }

    astLenTimeInShow->u8LengthMin = buf[0];
    astLenTimeInShow->u8LengthHour = buf[1];
    astLenTimeInShow->u8ElapsedMin = buf[2];
    astLenTimeInShow->u8ElapsedHour = buf[3];
    astLenTimeInShow->u8ElapsedSec = buf[4];

    SAMPLE_CC_INFO_PRINT("Program Length: %d:%d, Elapsed Time: %d:%d:%d\n", length[1], length[0], length[3], length[2], length[4]);

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeProgName(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8         *buf = NULL;
    mt_u8         len = 0;
    XDSProgName_S astProgName[1] = { 0 };

    len = u8PacketLen;
    if((len < MIN_CC608_PROGNAME_LEN) || (len > MAX_CC608_PROGNAME_LEN))
    {
        SAMPLE_CC_ERR_PRINT("XDS program title overflow!\n");
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    for(mt_u8 i = 0; i < len; i++)
    {
        if(((buf[i] < 0x20) && (buf[i] != 0)) || (buf[i] > 0x7f))
        {
            SAMPLE_CC_ERR_PRINT("Invalide Value for Program Title buf[%d] = %x\n", i, buf[i]);
            return MT_FAILURE;
        }
    }


    astProgName->u8ProgNameLen = len;
    memcpy(astProgName->au8ProgName, buf, len);

    MT_CC_PRINT("Program name:\n");
    for(mt_u8 i = 0; i < len; i++)
    {
        MT_CC_PRINT("%c ", astProgName->au8ProgName[i]);
    }
    MT_CC_PRINT("\n");

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeProgType(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8         *buf = NULL;
    mt_u8         len = 0;
    XDSProgType_S astProgType[1] = { 0 };

    len = u8PacketLen;
    if(len > MAX_CC608_PROGTYPE_LEN)
    {
        SAMPLE_CC_ERR_PRINT("Invalide length: %d for program type packet\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    for(mt_u8 i = 0; i < len; i++)
    {
        if(((buf[i] < 0x20)&&(buf[i] != 0)) || (buf[i] > 0x7f))
        {
            SAMPLE_CC_ERR_PRINT("Invalide value for program type: buf[%d] = %x\n", i, buf[i]);
            return MT_FAILURE;
        }
    }


    astProgType->u8ProgTypeLen = len;
    memcpy(astProgType->au8ProgType, buf, len);

    MT_CC_PRINT("Program type:\n");
    for(mt_u8 i = 0; i < len; i++)
    {
        MT_CC_PRINT("%c ", astProgType->au8ProgType[i]);
    }
    MT_CC_PRINT("\n");

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeContentAdvisory(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8        *buf = NULL;
    XDSContAdv_S astContAdv[1] = { 0 };

    buf = pu8PacketData;
    astContAdv->u8Char1 = buf[0];
    astContAdv->u8Char2 = buf[1];

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeAudioService(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8             *buf = NULL;
    XDSAudioService_S astAudioService[1] = { 0 };

    buf = pu8PacketData;
    astAudioService->u8Main = buf[0];
    astAudioService->u8SAP = buf[1];

    SAMPLE_CC_INFO_PRINT("Audio Service: 0x%x 0x%x\n", astAudioService->u8Main, astAudioService->u8SAP);
    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeCaptionService(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8            *buf = NULL;
    mt_u8            len = 0;
    XDSCaptService_S astCaptService[1] = { 0 };

    len = u8PacketLen;
    if(len < MIN_CC608_CAPTSERV_LEN || len > MAX_CC608_CAPTSERV_LEN)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for caption service packet!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    astCaptService->u8CaptServicesLen = len;
    memcpy(astCaptService->au8CaptServices, buf, len);

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeCopyAndRedist(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8              *buf = NULL;
    mt_u8              len = 0;
    XDSCopyAndRedist_S astCopyAndRedist[1] = { 0 };

    len = u8PacketLen;
    if(len != 2)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for CGMSA packet!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    astCopyAndRedist->u8Byte1 = buf[0];
    astCopyAndRedist->u8Byte2 = buf[1];

    SAMPLE_CC_INFO_PRINT("Copy and Redistribution: 0x%x 0x%x\n", buf[0], buf[1]);

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeCompPacket1(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8            *buf = NULL;
    mt_u8            len = 0;
    XDSCompPacket1_S astCompPacket1[1] = { 0 };

    len = u8PacketLen;
    if(len < 10 || len > 32)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    memcpy(astCompPacket1->au8ProgramType, buf, 5);
    astCompPacket1->u8ContAdv = buf[5];
    astCompPacket1->u8LengthMin = buf[6];
    astCompPacket1->u8LengthHour = buf[7];
    astCompPacket1->u8ElapsedMin = buf[8];
    astCompPacket1->u8ElapsedHour = buf[9];
    astCompPacket1->u8TitleLen = len - 10;
    memcpy(astCompPacket1->au8Title, &buf[10], len - 10);

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeCompPacket2(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8            *buf = NULL;
    mt_u8            len = 0;
    XDSCompPacket2_S astCompPacket2[1] = { 0 };

    len = u8PacketLen;
    if(len < 14 || len > 32)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    astCompPacket2->stPRGStartTime.u8Minute = buf[0];
    astCompPacket2->stPRGStartTime.u8Hour = buf[1];
    astCompPacket2->stPRGStartTime.u8Data = buf[2];
    astCompPacket2->stPRGStartTime.u8Month = buf[3];
    astCompPacket2->stAudioServices.u8Main = buf[4];
    astCompPacket2->stAudioServices.u8SAP = buf[5];
    memcpy(astCompPacket2->au8CaptServices, &buf[6], 2);
    memcpy(astCompPacket2->au8CallLetter, &buf[8], 4);
    memcpy(astCompPacket2->au8NativeChan, &buf[12], 2);
    astCompPacket2->au8NetworkNameLen = len - 14;
    memcpy(astCompPacket2->au8NetworkName, &buf[14], len - 14);

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeProgDescription(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8         *buf = NULL;
    mt_u8         len = 0;
    XDSProgDesc_S astXDSProgDesc[1] = { 0 };

    len = u8PacketLen;
    if(len > MAX_CC608_PROGDESC_LEN)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    astXDSProgDesc->u8ProgDescLen = len;

    buf = pu8PacketData;
    memcpy(astXDSProgDesc->au8ProgDesc, buf, len);

    return 0;
}

static mt_s32 _CC608_XDS_DecodeNetworkName(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8             *buf = NULL;
    mt_u8             len = 0;
    XDSNetworkName_S  astNetworkName[1] = { 0 };

    len = u8PacketLen;
    if(len < MIN_CC608_NETNAME_LEN)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    if(len > MAX_CC608_NETNAME_LEN)
    {
        len = MAX_CC608_NETNAME_LEN;
    }

    astNetworkName->u8NetNameLen = len;

    buf = pu8PacketData;
    memcpy(astNetworkName->au8NetName, buf, len);

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeCallLetter(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8              *buf = NULL;
    mt_u8              len = 0;
    XDSCallLetNatvCh_S astCallLetNatvCh[1] = { 0 };

    len = u8PacketLen;
    if(len > MAX_CC608_CALLLET_LEN)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet and reset to length = 6\n", len);
        len = MAX_CC608_CALLLET_LEN;
    }

    buf = pu8PacketData;
    if(len <= MIN_CC608_CALLLET_LEN)
    {
        SAMPLE_CC_INFO_PRINT("Call Letter: %c %c %c %c\n", buf[0], buf[1], buf[2], buf[3]);
    }
    else
    {
        SAMPLE_CC_INFO_PRINT("Call Letter: %c %c %c %c %c %c\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    }

    memcpy(astCallLetNatvCh->au8CallLetter, buf, (len > 4) ? 4 : len);

    if(len < MIN_CC608_CALLLET_LEN)
    {
        len = MIN_CC608_CALLLET_LEN;
    }
    astCallLetNatvCh->u8NativeChanLen = len - MIN_CC608_CALLLET_LEN;
    memcpy(astCallLetNatvCh->au8NativeChan, &buf[MIN_CC608_CALLLET_LEN], len - MIN_CC608_CALLLET_LEN);

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeTapeDelay(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8          *buf = NULL;
    mt_u8          len = 0;
    XDSTapeDelay_S astTapeDelay[1] = { 0 };

    len = u8PacketLen;
    if(len != 2)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    astTapeDelay->u8Minute = buf[0];
    astTapeDelay->u8Hour = buf[1];

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeTSID(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8           *buf = NULL;
    mt_u8           len = 0;
    XDSTranSignID_S astTransSignID[1] = { 0 };

    len = u8PacketLen;
    if(len != 4)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for TSID packet!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    astTransSignID->u8TSID3to0 = buf[0];
    astTransSignID->u8TSID7to4 = buf[1];
    astTransSignID->u8TSID11to8 = buf[2];
    astTransSignID->u8TSID11to8 = buf[3];

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeTimeOfDay(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8          *buf = NULL;
    mt_u8          len = 0;
    mt_u8          Minute = 0;
    mt_u8          Hour = 0;
    mt_u8          Date = 0;
    mt_u8          Month = 0;
    mt_u8          Week = 0;
    mt_u8          Year = 0;
    XDSTimeOfDay_S astTimeOfDay[1] = { 0 };

    len = u8PacketLen;
    if(len != 6)
    {
        SAMPLE_CC_ERR_PRINT("Invalide length:%d for Data/Time Packet!\n", len);
        return MT_FAILURE;
    }
    
    buf = pu8PacketData;
    Minute = (buf[0] & 0x3F);
    Hour = (buf[1] & 0x1F);
    Date = (buf[2] & 0x1F);
    Month = (buf[3] & 0x0F);
    Week = (buf[4] & 0x07);
    Year = (buf[5] & 0x3f);
    if((Minute > 59) || (Hour > 23) || (Date > 31) || (Month > 12))
    {
        SAMPLE_CC_ERR_PRINT("Invalide Time/Date Value: Minute = %d, Hour = %d, Date = %d, Month = %d\n", Minute, Hour, Date, Month);
        return MT_FAILURE;
    }

    astTimeOfDay->u8Minute = Minute;
    astTimeOfDay->u8Hour = Hour;
    astTimeOfDay->u8Date = Date;
    astTimeOfDay->u8Month = Month;
    astTimeOfDay->u8Day = Week;
    astTimeOfDay->u8Year = Year;

    SAMPLE_CC_INFO_PRINT("Time of Day: %.2d-%.2d %.2d:%.2d, Week: %d,Year: %d\n", Month, Date, Hour, Minute, Week, Year+1990);
    
    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeImpCaptureID(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8          *buf = NULL;
    mt_u8          len = 0;
    XDSImpCaptID_S astImpCaptID[1] = { 0 };

    len = u8PacketLen;
    if(len != 6)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    astImpCaptID->u8Minute = buf[0];
    astImpCaptID->u8Hour = buf[1];
    astImpCaptID->u8Date = buf[2];
    astImpCaptID->u8Month = buf[3];
    astImpCaptID->u8LengthMin = buf[4];
    astImpCaptID->u8LengthHour = buf[5];

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeSuplDataLoc(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8             *buf = NULL;
    mt_u8             len = 0;
    XDSSupplDataLoc_S astSupplDataLoc[1] = { 0 };

    len = u8PacketLen;
    if((len < MIN_CC608_SUPPDATA_LEN) || (len > MAX_CC608_SUPPDATA_LEN))
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    astSupplDataLoc->u8SupplDataLocLen = len;

    buf = pu8PacketData;
    memcpy(astSupplDataLoc, buf, len);

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeTimeZone(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8            *buf = NULL;
    mt_u8            len = 0;
    XDSLocTimeZone_S astLocTimeZone[1] = { 0 };

    len = u8PacketLen;
    if(len != 2)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    if(buf[1] != 0)
    {
        SAMPLE_CC_WARN_PRINT("The second character of Local Time Zone & DST Use packet is not NULL!\n");
    }

    astLocTimeZone->u8LocTimeZoneHour = buf[0];

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeOutOfBand(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8          *buf = NULL;
    mt_u8          len = 0;
    XDSOutOfBand_S astOutOfBand[1] = { 0 };

    len = u8PacketLen;
    if(len != 2)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    astOutOfBand->u8ChanLow = buf[0];
    astOutOfBand->u8ChanHigh = buf[1];

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeChMapPointer(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8             *buf = NULL;
    mt_u8             len = 0;
    XDSChMapPointer_S astChMapPointer[1] = { 0 };

    len = u8PacketLen;
    if(len != 2)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    astChMapPointer->u8TuneChanLow = buf[0];
    astChMapPointer->u8TuneChanHigh = buf[1];

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeChMapHeader(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8             *buf = NULL;
    mt_u8             len = 0;
    XDSChMapHeader_S  astChMapHeader[1] = { 0 };

    len = u8PacketLen;
    if(len != 4)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    astChMapHeader->u8ChanLow = buf[0];
    astChMapHeader->u8ChanHigh = buf[1];
    astChMapHeader->u8Version = buf[2];

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeChMapPacket(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8             *buf = NULL;
    mt_u8             len = 0;
    XDSChMapPacket_S  astChMapPacket[1] = { 0 };

    len = u8PacketLen;
    if((len < 2) || (len > 10))
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    buf = pu8PacketData;
    astChMapPacket->u8UserChanLow = buf[0];
    astChMapPacket->u8UserChanHigh = buf[1];

    if(len > 2)
    {
        astChMapPacket->u8TuneChanLow = buf[2];
        astChMapPacket->u8TuneChanHigh = buf[3];

        if(len > 4)
        {
            astChMapPacket->u8ChIDLen = len - 4;
            memcpy(astChMapPacket->au8ChID, &buf[4], len - 4);
        }
    }

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeNWSCode(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8                *buf = NULL;
    mt_u8                len = 0;
    XDSNatWeaServcCode_S astNatWeaServcCode[1] = { 0 };

    len = u8PacketLen;
    if(len > MAX_CC608_NWSCODE_LEN)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    astNatWeaServcCode->u8NatWeaServcCodeLen = len;

    buf = pu8PacketData;
    memcpy(astNatWeaServcCode->au8NatWeaServcCode, buf, len);

    return MT_SUCCESS;
}

static mt_s32 _CC608_XDS_DecodeNWSData(mt_u8 *pu8PacketData, mt_u8 u8PacketLen)
{
    mt_u8               *buf = NULL;
    mt_u8               len = 0;
    XDSNatWeaServcMsg_S astNatWeaServcMsg[1] = { 0 };

    len = u8PacketLen;
    if(len > MAX_CC608_NWSMSG_LEN)
    {
        SAMPLE_CC_ERR_PRINT("Invalide len = %d for composite packet-1!\n", len);
        return MT_FAILURE;
    }

    astNatWeaServcMsg->u8NatWeaServcMsgLen = len;

    buf = pu8PacketData;
    memcpy(astNatWeaServcMsg->u8NatWeaServcMsg, buf, len);

    return MT_SUCCESS;
}

mt_void CC608_XDS_Init()
{
    memset(g_pfnXDSPacketDecoder, 0, sizeof(g_pfnXDSPacketDecoder));

    /** current class */
    g_pfnXDSPacketDecoder[XDS_CLASS_CUR][XDS_CUR_PRG_ID]         =   _CC608_XDS_DecodeProgIDNum;
    g_pfnXDSPacketDecoder[XDS_CLASS_CUR][XDS_CUR_TIMEINSHOW]     =   _CC608_XDS_DecodeLenAndTimeInShow;
    g_pfnXDSPacketDecoder[XDS_CLASS_CUR][XDS_CUR_PRG_NAME]       =   _CC608_XDS_DecodeProgName;
    g_pfnXDSPacketDecoder[XDS_CLASS_CUR][XDS_CUR_PRG_TYPE]       =   _CC608_XDS_DecodeProgType;
    g_pfnXDSPacketDecoder[XDS_CLASS_CUR][XDS_CUR_CONT_ADVSR]     =   _CC608_XDS_DecodeContentAdvisory;
    g_pfnXDSPacketDecoder[XDS_CLASS_CUR][XDS_CUR_AUD_SERVC]      =   _CC608_XDS_DecodeAudioService;
    g_pfnXDSPacketDecoder[XDS_CLASS_CUR][XDS_CUR_CAP_SERVC]      =   _CC608_XDS_DecodeCaptionService;
    g_pfnXDSPacketDecoder[XDS_CLASS_CUR][XDS_CUR_CPY_REDIST]     =   _CC608_XDS_DecodeCopyAndRedist;
    g_pfnXDSPacketDecoder[XDS_CLASS_CUR][XDS_CUR_COMP_1]         =   _CC608_XDS_DecodeCompPacket1;
    g_pfnXDSPacketDecoder[XDS_CLASS_CUR][XDS_CUR_COMP_2]         =   _CC608_XDS_DecodeCompPacket2;
    for(mt_s32 j = 0; j < 8; j++)
    {
        g_pfnXDSPacketDecoder[XDS_CLASS_CUR][XDS_CUR_PRG_DESC1 + j]  =  _CC608_XDS_DecodeProgDescription;
    }

    /** future class */
    g_pfnXDSPacketDecoder[XDS_CLASS_FUT][XDS_CUR_PRG_ID]         =   _CC608_XDS_DecodeProgIDNum;
    g_pfnXDSPacketDecoder[XDS_CLASS_FUT][XDS_CUR_TIMEINSHOW]     =   _CC608_XDS_DecodeLenAndTimeInShow;
    g_pfnXDSPacketDecoder[XDS_CLASS_FUT][XDS_CUR_PRG_NAME]       =   _CC608_XDS_DecodeProgName;
    g_pfnXDSPacketDecoder[XDS_CLASS_FUT][XDS_CUR_PRG_TYPE]       =   _CC608_XDS_DecodeProgType;
    g_pfnXDSPacketDecoder[XDS_CLASS_FUT][XDS_CUR_CONT_ADVSR]     =   _CC608_XDS_DecodeContentAdvisory;
    g_pfnXDSPacketDecoder[XDS_CLASS_FUT][XDS_CUR_AUD_SERVC]      =   _CC608_XDS_DecodeAudioService;
    g_pfnXDSPacketDecoder[XDS_CLASS_FUT][XDS_CUR_CAP_SERVC]      =   _CC608_XDS_DecodeCaptionService;
    g_pfnXDSPacketDecoder[XDS_CLASS_FUT][XDS_CUR_CPY_REDIST]     =   _CC608_XDS_DecodeCopyAndRedist;
    g_pfnXDSPacketDecoder[XDS_CLASS_FUT][XDS_CUR_COMP_1]         =   _CC608_XDS_DecodeCompPacket1;
    g_pfnXDSPacketDecoder[XDS_CLASS_FUT][XDS_CUR_COMP_2]         =   _CC608_XDS_DecodeCompPacket2;
    for(mt_s32 j = 0; j < 8; j++)
    {
        g_pfnXDSPacketDecoder[XDS_CLASS_FUT][XDS_CUR_PRG_DESC1 + j]  =  _CC608_XDS_DecodeProgDescription;
    }


    /** channel classv*/
    g_pfnXDSPacketDecoder[XDS_CLASS_CHAN][XDS_CHAN_NET_NAME]     =   _CC608_XDS_DecodeNetworkName;
    g_pfnXDSPacketDecoder[XDS_CLASS_CHAN][XDS_CHAN_CALL_LETTER]  =   _CC608_XDS_DecodeCallLetter;
    g_pfnXDSPacketDecoder[XDS_CLASS_CHAN][XDS_CHAN_TAPE_DELAY]   =    _CC608_XDS_DecodeTapeDelay;
    g_pfnXDSPacketDecoder[XDS_CLASS_CHAN][XDS_CHAN_TRNS_SIGNID]  =   _CC608_XDS_DecodeTSID;

    // miscellaneous class
    g_pfnXDSPacketDecoder[XDS_CLASS_MISC][XDS_MISC_TIMEOFDAY]    =   _CC608_XDS_DecodeTimeOfDay;
    g_pfnXDSPacketDecoder[XDS_CLASS_MISC][XDS_MISC_IMPL_CAPTID]  =   _CC608_XDS_DecodeImpCaptureID;
    g_pfnXDSPacketDecoder[XDS_CLASS_MISC][XDS_MISC_SUPL_DATALOC] =   _CC608_XDS_DecodeSuplDataLoc;
    g_pfnXDSPacketDecoder[XDS_CLASS_MISC][XDS_MISC_TIME_ZONE]    =   _CC608_XDS_DecodeTimeZone;
    g_pfnXDSPacketDecoder[XDS_CLASS_MISC][XDS_MISC_OUTOFBAND]    =   _CC608_XDS_DecodeOutOfBand;
    g_pfnXDSPacketDecoder[XDS_CLASS_MISC][XDS_MISC_CHMAP_POINTER]=  _CC608_XDS_DecodeChMapPointer;
    g_pfnXDSPacketDecoder[XDS_CLASS_MISC][XDS_MISC_CHMAP_HEADER] =   _CC608_XDS_DecodeChMapHeader;
    g_pfnXDSPacketDecoder[XDS_CLASS_MISC][XDS_MISC_CHMAP_HEADER] =   _CC608_XDS_DecodeChMapPacket;

    /** public classv*/
    g_pfnXDSPacketDecoder[XDS_CLASS_PUB][MT_UNF_CC_XDS_PUB_NWS_CODE]     =   _CC608_XDS_DecodeNWSCode;
    g_pfnXDSPacketDecoder[XDS_CLASS_PUB][MT_UNF_CC_XDS_PUB_NWS_MSG]      =   _CC608_XDS_DecodeNWSData;
}

mt_s32 CC608_XDS_Decode(mt_u8 u8XDSClass, mt_u8 u8XDSType, mt_u8 *pu8Data, mt_u8 u8DataLen)
{
    XDSPacketDecoder_FN pFnXDSPacketDecoder = NULL;

    SAMPLE_CC_INFO_PRINT("CC608_XDS_Decode: u8XDSClass = %d, u8XDSType = %d, u8DataLen = %d\n", u8XDSClass, u8XDSType, u8DataLen);

    if((u8XDSClass >= XDS_CLASS_BUTT) || (u8XDSType >= XDS_MISC_TYPE_BUTT))
    {
        SAMPLE_CC_ERR_PRINT("Invalid XDS packet class %d or type %d.\n", u8XDSClass, u8XDSType);
        return MT_FAILURE;
    }

    pFnXDSPacketDecoder = g_pfnXDSPacketDecoder[u8XDSClass][u8XDSType];

    if(pFnXDSPacketDecoder == MT_NULL)
    {
        return MT_FAILURE;
    }

    if(pu8Data && u8DataLen)
    {
        (mt_void)pFnXDSPacketDecoder(pu8Data, u8DataLen);
    }

    return MT_SUCCESS;
}

