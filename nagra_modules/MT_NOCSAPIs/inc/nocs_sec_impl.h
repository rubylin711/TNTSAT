/*
 * Copyright (C) 2020 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */
#ifndef  __NOCS_SEC_IMPL_H__
#define  __NOCS_SEC_IMPL_H__

#include "ca_defs.h"
#include "ca_defsx.h"
#include "ca_sec.h"
#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

//#define CA_SEC_DEBUG

#ifdef CA_SEC_DEBUG
enum
{
    NOSC_SEC_API_DEBUG_PRINT_DISABLE,
    NOSC_SEC_API_DEBUG_PRINT_ERROR,
    NOSC_SEC_API_DEBUG_PRINT_INFO,
    NOSC_SEC_API_DEBUG_PRINT_TRACE,
    NOSC_SEC_API_DEBUG_PRINT_MAX,
};

extern TUnsignedInt8 gSecDebugLevel;

#if 0
/*
#define SECAPI_TRACE(fmt, args...) \
	do { \
		if(gSecDebugLevel >= NOSC_SEC_API_DEBUG_PRINT_TRACE) \
			printf("[REE]SECAPI trace[%s:%u]"fmt, __FUNCTION__, __LINE__, ##args); \
	} while(0)
#define SECAPI_INFO(fmt, args...) \
	do { \
		if(gSecDebugLevel >= NOSC_SEC_API_DEBUG_PRINT_INFO) \
			printf("[REE]SECAPI info[%s:%u]"fmt, __FUNCTION__, __LINE__, ##args); \
	} while(0)
#define SECAPI_ERROR(fmt, args...) \
	do { \
		if(gSecDebugLevel >= NOSC_SEC_API_DEBUG_PRINT_ERROR) \
			printf("[REE]SECAPI error[%s:%u]"fmt, __FUNCTION__, __LINE__, ##args); \
	} while(0)
*/
#define SECAPI_TRACE(fmt, args...)  printf("SECAPI trace[%s:%u]"fmt, __FUNCTION__, __LINE__, ##args)
#define SECAPI_INFO(fmt, args...)    printf("SECAPI info[%s:%u]"fmt, __FUNCTION__, __LINE__, ##args)
#define SECAPI_ERROR(fmt, args...)  printf("SECAPI error[%s:%u]"fmt, __FUNCTION__, __LINE__, ##args)

#define SECAPI_DINFO(fmt, args...)  printf("SECAPI info[%s:%u]"fmt, __FUNCTION__, __LINE__, ##args)

#else
#define SECAPI_TRACE(fmt, args...)
#define SECAPI_INFO(fmt, args...)    printf("SECAPI info[%s:%u]"fmt, __FUNCTION__, __LINE__, ##args)
#define SECAPI_ERROR(fmt, args...)  printf("SECAPI error [%s:%u]"fmt, __FUNCTION__, __LINE__, ##args)
#define NHTCSD_DUMP

#define SECAPI_DINFO(fmt, args...)  printf("SECAPI DBG[%s:%u]"fmt, __FUNCTION__, __LINE__, ##args)

#define SECAPI_REENC_INFO(fmt, args...)  printf("SECAPI DBG[%s:%u]"fmt, __FUNCTION__, __LINE__, ##args)

#endif

#else
#define SECAPI_TRACE(fmt, args...)
#define SECAPI_INFO(fmt, args...)
#define SECAPI_ERROR(fmt, args...) //printf("SECAPI ERROR[%s:%u]"fmt, __FUNCTION__, __LINE__, ##args)

#define SECAPI_DINFO(fmt, args...)

#define SECAPI_REENC_INFO(fmt, args...)  //printf("SECAPI DBG[%s:%u]"fmt, __FUNCTION__, __LINE__, ##args)

#endif

/** @} */  /** <!-- ==== Structure Definition ==== */

/** \addtogroup      NOCS_SEC_IMPL */
/** @{ */  /** <!-- [NOCS_SEC_IMPL] */

/**Define 64bit type*/
typedef unsigned long     TUnsignedLong;

/** Nagra EMIs List, for more information please refer to NAGRA docs */
/** CSA 2.0 alogrithm for TS stream*/
#define NOCS_EMI_MPEG_TS_DVB_CSA2                               0x0000
/** CSA 3.0 alogrithm for TS stream*/
#define NOCS_EMI_MPEG_TS_DVB_CSA3                               0x0001
/** NAGRA ASA 64 bit alogrithm for TS stream*/
#define NOCS_EMI_MPEG_TS_DVB_ASA_64                             0x0010
/** NAGRA ASA 128 bit alogrithm for TS stream*/
#define NOCS_EMI_MPEG_TS_DVB_ASA_128                            0x0011
/** NAGRA ASA light bit alogrithm for TS stream*/
#define NOCS_EMI_MPEG_TS_DVB_ASA_LIGHT                          0x0012
/** AES 128  IDSA alogrithm for TS stream*/
#define NOCS_EMI_MPEG_TS_DVB_AES128_IDSA                        0x0020
/** AES 128-bits ECB tail clear alogrithm for TS stream*/
#define NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR              0x0021
/** AES 128-bits CBC tail clear alogrithm for TS stream*/
#define NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR              0x0022
/** AES 128-bits CISSA alogrithm for TS stream*/
#define NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1                    0x0023
/** AES 128-bits ECB head clear alogrithm for TS stream*/
#define NOCS_EMI_MPEG_TS_DVB_AES128_ECB_HEAD_CLEAR              0x0024
/** TDES CBC DVS042 alogrithm for TS stream*/
#define NOCS_EMI_MPEG_TS_DVB_TDES_CBC_ZEROIV_DVS042             0x0040
/** TDES ECB tail clear alogrithm for TS stream*/
#define NOCS_EMI_MPEG_TS_DVB_TDES_ECB_TAIL_CLEAR                0x0041
/** AES 128-bits CBC zero IV tail clear alogrithm for data*/
#define NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR                   0x4020
/** AES 128-bits ECB tail clear alogrithm for data*/
#define NOCS_EMI_AES128_ECB_TAIL_CLEAR                          0x4021
/** AES 128-bits CBC alogrithm for DASH MPEG data*/
#define NOCS_EMI_AES128_MPEG_DASH_CBC_CLEAR_PADDING             0x4022
/** AES 128-bits CBC PKCS7 alogrithm for data*/
#define NOCS_EMI_AES128_CBC_PKCS7_PADDING                       0x4023
/** AES 128-bits CTR mode alogrithm for MPEG data*/
#define NOCS_EMI_AES128_MPEG_DASH_CTR                           0x4024
/** AES 128-bits CBC mode tail clear alogrithm for data*/
#define NOCS_EMI_AES128_CBC_TAIL_CLEAR                          0x4026
/** AES 128-bits CTR mode*/
#define NOCS_EMI_AES128_CTR                                     0x4027
/** AES 128-bits GCM*/
#define NOCS_EMI_AES128_GCM                                     0x4028
/** AES 128-bits HLS data*/
#define NOCS_EMI_AES128_HLS                                     0x4029
/** AES 128-bits CBCS for mpeg data stream*/
#define NOCS_EMI_AES128_MPEG_DASH_CBCS                          0x402A
/** AES 128-bits CENS CTE for mpeg data stream*/
#define NOCS_EMI_AES128_MPEG_DASH_CTR_CENS                      0x402B
/** TDES CBC zero IV tail clear */
#define NOCS_EMI_TDES_CBC_ZEROIV_TAIL_CLEAR                     0x4040
/** TDES ECB  tail clear */
#define NOCS_EMI_TDES_ECB_TAIL_CLEAR                            0x4041
/** TDES CBC  tail clear */
#define NOCS_EMI_TDES_CBC_TAIL_CLEAR                            0x4043
/** DES ECB  CTS mepeg ts stream */
#define NOCS_EMI_MPEG_TS_DVB_DES_ECB_CTS                     0x0031
/** MULTI2 ts stream */
#define NOCS_EMI_MPEG_TS_DVB_MULTI2 				0x0050
/** AES 256-bits key ECB mode tail clear */
#define NOCS_EMI_AES256_ECB_TAIL_CLEAR                          0x4030
/** SHA-256 hash mac */
#define NOCS_EMI_HMAC_SHA256                                              0x4060


/** Actual max numbers, you can midify it accroding to your project */
#define MAX_PID_NUMS    32


#ifdef _MT_NOCS_WITH_TALTS_
#define MAX_INTERNAL_SESSIONS   512
#define MAX_SESSIONS_ID_MASK    (0x1FF) //0 -511
#else

/** Max number of sessions, pre-integration need to be 64, you can modify it accroding to your project: 1-19 for DVB, from 20 for OTT */
#define MAX_INTERNAL_SESSIONS   64
/** MASK for get valid seesion number */
#define MAX_SESSIONS_ID_MASK    (0x3F) // 1 -63
#endif

/** The transport session type */
typedef enum
{
	TRANSPORT_SESSION_TYPE_NONE = 0,    /** none, default type for open session */
	TRANSPORT_SESSION_TYPE_DVB, /** DVB session */
	TRANSPORT_SESSION_TYPE_OTT, /** OTT session */
	TRANSPORT_SESSION_TYPE_RECORD,  /** Record session */
	TRANSPORT_SESSION_TYPE_REPLAY,  /** Replay session */
	TRANSPORT_SESSION_TYPE_INVALID, /** invalid session */
}transportSessionType;

/** The max valid key slot ID for chipset */
#define MAX_KEY_SLOT_ID	(127)
/** The video PID flag */
#define SEC_PID_VIDEO	(0x55aa)
/** The audio PID flag */
#define SEC_PID_AUDIO	(0xaa55)
/** The PID pair values */
struct PID_PAIR {
	TUnsignedInt16 pid; /** The PID value */
	TUnsignedInt16 type; /** The PID type, only availble for Audio,Video*/
};
/** The audio PID information for session */
typedef struct MT_SEC_PID_INFO_S
{
    TTransportSessionId tsid;   /** The TransportSessionId ID of session */
    transportSessionType sessionType;   /** The TransportSessionId type */
    TUnsignedInt32 pidNum;  /** The count of PID */
    struct PID_PAIR pidList[MAX_PID_NUMS];  /** The PID list */
    TSignedInt32 isMultiSession;    /** If the session is not the first one*/
    TUnsignedLong recDscHandle; /** The descamber handle of the session*/
    TBoolean smp;   /** If enable SMP for the session*/
} TSecPidInfo;


//Below two macro are only used for NAGRA JTS test
/*
#define NAGRA_JTS_TEST
#define NAGRA_SEC_JTS_TEST
*/
/**It is used for NAGRA SEC API and testing projects, you don't need to use it*/
struct SSecCipherSession {
    TUnsignedInt32 magic;
    TBoolean isStreamSession;	/* only stream seesion sets it */
#ifdef NAGRA_JTS_TEST
    TBoolean isUseDeccramble;	/* only using descramble sets it  */
#endif
    TBoolean isEncryptionSession;	/* only encryption seesion sets it */

    TUnsignedInt8 keyID[2][16];	/*the ID of key */
    TUnsignedInt8 keySlotID[2];
    TUnsignedInt8 key256SlotID[2];

    TUnsignedInt8 polar;

    size_t metadataSize;
    TUnsignedInt8 metadata[4];

    TUnsignedInt16 EMI;
    TBoolean isKeySet;

    TBoolean isUseFlashProtKey;	/* only use flash key set it, flash use fixed keyslot ID */

    TTransportSessionId transportSessionId;	/* defined by the player */

    TUnsignedInt32 nextKeyIdx;

    TUnsignedLong cryptoHandle;

    TUnsignedInt8 IV[2][16];	/* save the IV of even/odd key */
    TUnsignedInt8 keySlotUsed[2];	/* flag if the keylot is uesd: even /odd key */
    TUnsignedInt8 keySlotLatest;

    TUnsignedLong dscHandle;
    TSecPidInfo pidInfo;
    TBoolean isMultiSession;
};

/**It is used for NAGRA SEC API and testing projects, you don't need to use it*/
struct SSecHashContext
{
    TUnsignedLong priv_handle;       //mt_handle handle
    TBoolean isUseKeySlot;
    TUnsignedInt32 keySlots[2];
    TUnsignedInt8 metadata[4];
};
/**It is used for NAGRA SEC API and testing projects, you don't need to use it*/
struct SSecStreamSession
{
    TUnsignedInt32 magic;   //check this for double free session issue
    TBoolean isEncryptionSession;
    TTransportSessionId transportSessionId;
    TUnsignedInt16 EMI;

	/*TUnsignedInt8 keyID[2][16];*/
    TUnsignedInt8 keyID[4][16];
    TUnsignedInt8 keySlotID[4];
    TUnsignedInt8 keySlotStat;

    TUnsignedLong audioDscHandle;
    TUnsignedLong videoDscHandle;
    TUnsignedLong recDscHandle;
    TSecPidInfo pidInfo;

    TUnsignedLong cryptoHandle;
    TBoolean isMultiSession;
    TUnsignedInt32 smpOpStatus;
};

/** @} */  /** <!-- ==== Structure Definition end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif				/* End of #ifdef __cplusplus */
#endif				//__NOCS_SEC_IMPL_H__

