/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __ICC_NAGRA_SMART_CARD_TEST_SUIT_H_
#define __ICC_NAGRA_SMART_CARD_TEST_SUIT_H_

#include <sys/time.h>
#include <termios.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_unf_sci.h"

#define ONLY_SHOW_NAGRA_TEST_BEGIN_BEGIN_

#define ICC_TEST_PORT               MT_UNF_SCI_PORT0
#define SMC_TEST_DATA_DISPLAY       (0)
#define mtos_align_malloc(a,b)      malloc(a)
#define mtos_align_free             free
#define mtos_sem_take(a,b)
#define mtos_sem_give(a)
#define mtos_sem_destroy(a,b)
#define mtos_task_sleep(a)          MT_USLEEP(1000*a)
#define mtos_task_delay_ms(a)       MT_USLEEP(1000*a)

#define u8 mt_u8
#define s8 mt_s8
#define u16 mt_u16
#define s16 mt_s16
#define u32 mt_u32
#define s32 mt_s32

typedef unsigned char         TUnsignedInt8;
typedef unsigned int          TUnsignedInt32;
typedef TUnsignedInt8         TBoolean;
typedef TUnsignedInt32        TIccSessionId;
typedef TUnsignedInt32        TIccRegistrationId;
typedef TUnsignedInt8*        TIccAtr;
typedef TUnsignedInt32        TIccClockFrequency;
typedef size_t                TSize;

#define ICC_ATR_MAX_LEN   33
#define ICC_SESSION_ID_NONE ((TIccSessionId)-1)

typedef TUnsignedInt8 TIccT0Header[5];
typedef TUnsignedInt8 TIccT0StatusWords[2];

#define PCB_ICC_ALWAYS_OPEN 0
#define PCB_ICC_ALWAYS_CLOSE    1
#define PCB_ICC_TYPE PCB_ICC_ALWAYS_CLOSE

#define ICC_T1_EXCHANGE_DEBUG 0
#define ICC_DEBUG 1
#define ICC_ERR_PRINT 1
#if ICC_ERR_PRINT
#define ICC_ERROR_PRINT mtos_printk
#else
#define ICC_ERROR_PRINT(...)    do {} while(0);
#endif
#if ICC_DEBUG
#define ICC_DEBUG_PRINT mtos_printk
#else
#define ICC_DEBUG_PRINT(...)    do {} while(0);
#endif

#define T0_CMD_LENGTH           5
#define T0_CLA_OFFSET           0
#define T0_INS_OFFSET           1
#define T0_P1_OFFSET            2
#define T0_P2_OFFSET            3
#define T0_P3_OFFSET            4

#define REGISTRATION_ID   0x123456
#define SESSION_ID          0x654321

#define SMC_ETU_T14_DEF    620

#define SMC_GT_T0 12
#define SMC_GT_T1 11

#define ATR_MAX_SIZE    33              /* Maximum size of ATR byte array */
#define ATR_MAX_HISTORICAL  15          /* Maximum number of historical bytes */
#define ATR_MAX_PROTOCOLS 7             /* Maximun number of protocols */
#define ATR_MAX_IB    4                 /* Maximum number of interface bytes per protocol */
#define ATR_CONVENTION_DIRECT 0         /* Direct convention */
#define ATR_CONVENTION_INVERSE  1       /* Inverse convention */
#define ATR_PROTOCOL_TYPE_T0  0         /* Protocol type T=0 */
#define ATR_PROTOCOL_TYPE_T1  1         /* Protocol type T=1 */
#define ATR_PROTOCOL_TYPE_T2  2         /* Protocol type T=2 */
#define ATR_PROTOCOL_TYPE_T3  3         /* Protocol type T=3 */
#define ATR_PROTOCOL_TYPE_T14 14        /* Protocol type T=14 */
#define ATR_INTERFACE_BYTE_TA 0         /* Interface byte TAi */
#define ATR_INTERFACE_BYTE_TB 1         /* Interface byte TBi */
#define ATR_INTERFACE_BYTE_TC 2         /* Interface byte TCi */
#define ATR_INTERFACE_BYTE_TD 3         /* Interface byte TDi */
#define ATR_MAX_VENDER_LEN 10

/*!
  KHZ
  */
#define KHZ      (1000)
/*!
  MHZ
  */
#define MHZ       (KHZ * KHZ)

#define SMC_ATR_MAX_LENGTH    33
#define SMC_ATR_MIN_LENGTH    2

typedef enum
{
    ICC_ACCESS_NONE,
    /**< No smartcard access.
    */
    ICC_ACCESS_EXCLUSIVE,
    /**< Exclusively smartcard access.
    */
    ICC_ACCESS_SHARED
    /**< Shared smartcard access.
    */
} TIccAccessMode;

typedef enum
{
  ICC_EVENT_NO_AVAILABLE_CARD,
    /**< Special event, sent only just after the client registration
     *   if there is no available smartcard at this time.
    */
  ICC_EVENT_CARD_INSERTED,
    /**< A new smartcard has been inserted in the device. It is now
     *   presented to the client for use.
    */
  ICC_EVENT_CARD_INSERTED_SINGLE_CLIENT_SUPPORT,
    /**< A new smartcard has been inserted in the device. The driver
     *   is designed only to support one client at a time.
    */
  ICC_EVENT_CARD_SHARED,
    /**< A new smartcard has been inserted in the device. Another client
     *   has already claimed it but in shared mode. The next clients may
     *   only accept it in shared mode too.
    */
  ICC_EVENT_CARD_REMOVED,
    /**< A smartcard was removed from the device.
    */
  ICC_EVENT_CARD_MUTE
    /**< A new smartcard has been inserted in the device. However,
     *   it does not send its ATR.
    */
} TIccEventType;

typedef enum
{
    ICC_NO_ERROR,
     /**< Success.
     */
    ICC_ERROR_SESSION_ID,
     /**< The session identifier is wrong.
     */
    ICC_ERROR_CARD_REMOVED,
     /**< There is no smartcard to communicate with.
     */
    ICC_ERROR_CARD_MUTE,
     /**< The smartcard does not send any data. May be upside down.
     */
    ICC_ERROR_TIMEOUT,
     /**< The smartcard didn't answer in the required time.
     */
    ICC_ERROR_MODE,
     /**< The operation requested cannot be performed due to a mode
      *   constraint.
     */
    ICC_ERROR_CONFLICT,
     /**< The smartcard mode cannot be changed, as it would lead to
      *   a mode conflict.
     */
    ICC_ERROR
     /**< Other error.
     */
} TIccStatus;

typedef enum
{
    /*!
       commments
     */
    DRV_SMART_STATUS_INSERT = 0,
    /*!
       commments
     */
    DRV_SMART_STATUS_REMOVE
} SC_InsertStatus_E;

struct icc_info_struct
{
    int card_status;
    int protocol;
    int active;
    TIccAccessMode access_mode;
    TIccClockFrequency   clk_freq;    //KHZ
    u32 etu; //us
    u32 WT;  //us
    u32 BWT; //us
    u32 CWT; //us
    u32 BGT; //us
    u32 BWT_etu;
    u32 CWT_etu;
    u32 error_value;
    TUnsignedInt8 atr[ICC_ATR_MAX_LEN];
};

typedef enum
{
    ATR_GROUP_TA,
    ATR_GROUP_TB,
    ATR_GROUP_TC,
    ATR_GROUP_TD,
    ATR_GROUP_NUM,
} ATR_GROUP_E;

typedef struct
{
    u8 value;
    MT_BOOL  exist;
} ATR_GrouInfo_S;

typedef struct
{
    ATR_GrouInfo_S groupInfo[7][ATR_GROUP_NUM];
    u32 TA1;
    s32 Fi;
    s32 Di;
    u32 FI; //add by zwu 20180803
    u32 DI; //add by zwu 20180803
    u32 etu; //us
    u32 GT; //work ETU
    u32 WT; //us
    u32 PPSTiemout; //us
    u8 IFSC ;
    u32 BWT; //us
    u32 CWT; //us
    MT_BOOL historyExist;
    u8 historyByte[15];
    MT_BOOL  TCKExist;
    u32 TCK;
    u32 protocolCnt; // 指定的T个数
    u8 protocol[15];
    u8 protocol_type;
    MT_BOOL  T1Exist;
    MT_BOOL  T15Exist;
    MT_BOOL is_specific_mode;
}ATRInfo_S;

/*!
  smart card woring parameters
  */
typedef struct
{
    /*!
    the reference represenation of Fi
    */
    u32 FI;
    /*!
    the reference represenation of Di
    */
    u32 DI;
    /*!
    Fi
    */
    u32 Fi;
    /*!
    Di
    */
    u32 Di;
    /*!
    Max working clock in Hz
    */
    u32 clk_max;
    /*!
    char wait integer, CWT = (2 ^ cwi + 11)etu
    */
    u32 cwi;
    /*!
    block wait integer, BWT=2 ^ bwi * 960 * 372 / f s + 11etu
    */
    u32 bwi;
    /*!
    the extra wait time between two char on the different deriction
    */
    u32 N;
    /*!
    working wait time integeter, between card and IFN, wwt is less than 960 * wi * (Fi/f) etu
    */
    u32 wi;
    /*!
    working wait time, is less than 960 * wi * (Fi/f) etu
    */
    u32 wwt;
    /*!
    the all protocols this card support, bit validation
    */
    u16 SupportedProtocolTypes;
    /*!
    the flag to state if support special mode
    */
    u8 SpecificMode;
    /*!
    the special protocol type
    */
    u8 SpecificType;
    /*!
    to state if the special type is changable
    */
    u8 SpecificTypeChangable;
    /*!
    the current working protocol type
    */
    u8 WorkingType;
    /*!
    Card's maximum block size for (T=1)
    */
    u8 IFSC;
    /*!
    "II" variables integer represenation
    */
    u8 IInt;
    /*!
    "PI1" variables integer represenation
    */
    u8 PInt1;
    /*!
    "PI2" variables integer represenation
    */
    u8 PInt2;
    /*!
    Redundancy Check used CRC=1 LRC=0 (T=1)
    */
    u8 RC;
    /*!
    parity, 0:not use, 1:odd, 2:even
    */
    u8 parity : 4;
    /*!
    convention setting, 0:direct convention, 1:inverse convention
    */
    u8 convention : 4;
    /*!
    the special pps type <fix bug#99665>
    */
    u8 SpecialPps;
}smc_working_param_t;

/*!
  smart card prococol configurations
  */
typedef struct
{
    /*!
    ATR length
    */
    u8 length;
    /*!
    TS
    */
    u8 TS;
    /*!
    T0
    */
    u8 T0;
    /*!
    interface byte descripter
    */
    struct
    {
        /*!
        the value of this interface byte
        */
        u8 value;
        /*!
        the flag to state if this interface byte is present
        */
        u8 present;
    }
    /*!
    interface byte
    */
    ib[ATR_MAX_PROTOCOLS][ATR_MAX_IB],
    /*!
    TCK byte
    */
    TCK;
    /*!
    the count of protocol should be supported
    */
    u8 pn;
    /*!
    the history byte
    */
    u8 hb[ATR_MAX_HISTORICAL];
    /*!
    the count of history byte
    */
    u8 hbn;
    /*!
    the card work parameters
    */
    smc_working_param_t work_param;
}scard_pro_cfg_t;

/*!
  define scard atribution description
  */
typedef struct
{
    /*!
    Buffer for ATR.
    */
    u8 *p_buf;
    /*!
    length of received ATR, including TS.
    */
    u8 atr_len;
} scard_atr_desc_t;

extern int iccSmartcard_T1ProtocolTest(unsigned int count, u32 detect, u32 vcc, u32 fre);
extern int iccSmartcard_T1PairedTest(unsigned int count, u32 detect, u32 vcc, u32 fre);

#endif
