/*
 * Copyright (C) 2020 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */


#ifndef __MT_SEC_EXT_H__
#define __MT_SEC_EXT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nocs_sec_impl.h"
#include "ca_sec.h"


/** @} */  /** <!-- ==== Structure Definition ==== */

/** \addtogroup      MT_SEC_EXT */
/** @{ */  /** <!-- [MT_SEC_EXT] */

/**The event status for register callback which is set by mtSecSetEventCallback, when calling NOCS APIs secOpenStreamDecryptSession or secOpenStreamEncryptSession*/
/**CNcomment: */
typedef enum MT_SEC_EVENT
{
    MT_SEC_EVENT_OPEN,  /**Event for session open success. */
    MT_SEC_EVENT_CLOSE,     /**Event for session closed. */
    MT_SEC_EVENT_SMPC_CLOSE_START,     /**Event for session SMP close start. */
    MT_SEC_EVENT_SMPC_CLOSE_END,     /**Event for session SMP close end. */
    MT_SEC_EVENT_BUTT,  /**Invalid status. */
} MT_SEC_EVENT_E;

/**Operation type of session*/
/**CNcomment:*/
typedef enum {
    SESSION_OP_ENCRYPT = 0x10, /**Encryption session. */
    SESSION_OP_DECRYPT, /**Decryption session. */
    SESSION_OP_UNKNOWN, /**Invalid type. */
} TSessionOpType;

/**media buffer type*/
/**CNcomment: */
typedef enum {
	MB_VID = 1, /**The buffer for main video. */
	MB_AUD, /**The buffer for audio. */
	MB_REC, /**The buffer for record channel 0. */
	MB_REC_1,   /**The buffer for record channel 1. */
	MB_REC_2,   /**The buffer for record channel 2. */
	MB_REC_3,   /**The buffer for record channel 3. */
	MB_OTT, /**The buffer for OTT. */
	MB_OTT_1,   /**The buffer for OTT. */
	MB_REP, /**The buffer for replay. */
	MB_SUB_VID, /**The buffer for sub viedo. */
	MB_REENC_OTT, /**The buffer for re-encrypt OTT. */
	MB_REENC_OTT_1, /**The buffer for re-encrypt OTT_1. */
} MBType;

/**The information of each type media buffer*/
/**CNcomment: */
typedef struct _MediaBufferInfo {
	phys_addr_t start;  /**The start physical address of current media buffer.*/
	size_t size;    /**The byte size of current media buffer.*/
	MBType type;    /**The type of current meida buffer.*/
} MBInfo;

/**The information of one record channel connect to one DMX RAM port */
/**CNcomment: */
typedef struct _RecChanWithDmxIDInfo {
	mt_u32 dmxPortID;  /**The DMX Port ID.*/
	TUnsignedInt8 recChan;    /**Record channe ID.*/
	TBoolean valid;    /**If the information is valid.*/
} RecChanWithDmxIDInfo;

/**The information of play type, used to pre-set SMP configuration: DVB or OTT */
/**CNcomment: */
typedef enum {
    MT_DVB_PLAY, /**It is DVB play. */
    MT_OTT_PLAY, /**It is OTT play. */
    MT_OTHER_PLAY, /**It is not DVB or OTT play. */
} MTPlayType;

/**The information of output control violation */
/**CNcomment: */
typedef struct _MTSecOpREEFlag {
    TBoolean isViolation;  /** if there is any violation of output control.*/
    TUnsignedInt8 violateHDCP1;    /** HDCP 1.x violation.*/
    TUnsignedInt8 violateHDCP2;    /** HDCP 2.0 violation .*/
    TUnsignedInt8 violate4K;    /** 4K resolution violation: UHD.*/
    TUnsignedInt8 violate2K;    /** 2K/1K resolution violation: FHD.*/
    TUnsignedInt8 violateHD;    /** HD resolution violation: 720.*/
    TUnsignedInt8 violateSD;    /** SD resolution violation: less then 720.*/
} MTSecOpREEFlag;

 /**The max count of media buffer, you can modify it accroding to you project*/
#define MAX_MB_COUNT	(16)
 /**The DDR address aligned unit for SMP function (256K bytes)*/
#define SMP_DDR_ALIGN_UNIT  (0x40000)
 /**The count of record channel support*/
#define MAX_DMX_REC_CHAN	(4)

/******************************* API Declaration *****************************/

/**
 * \brief
 *      Type for Event Callback function
 * \param:[in] TTransportSessionId: current used Transport Session ID.
 * \param[:in] MT_SEC_EVENT_E: Event type.
 *
 * \return :: void
 * \see: MT_SEC_EVENT_E
 */
typedef void (*pfnEventCallback_t)(TTransportSessionId, MT_SEC_EVENT_E);

/** @} */  /** <!-- ==== Structure Definition end ==== */


/**
 * \brief
 *      To register an event callback to retrieve session status.
 *      User will be notified when a session is open/closed.
 * \attention \n
 * Call it before session open.
 *
 * \param:[in] e_sec_event: Event type. See MT_SEC_EVENT_E.
 * \param:[in] pfnEC: Callback to register.
 *
 * \return: SEC_NO_ERROR success, other value failed
 * \see: MT_SEC_EVENT_E and pfnEventCallback_t
 */
TSecStatus mtSecSetEventCallback(MT_SEC_EVENT_E e_sec_event, pfnEventCallback_t pfnEC);


/*!
 * \brief
 *      Set Pid List to bind descrambler.
 * \attention \n
 *called by user.
 *
 * \param:[in] xTransportSessionId: The ID of TransportSessionId, it is defined by NAGRA, we recommend using 0-31...
 * \param:[in] pxPidInfo: The information of session PID list. See  TSecPidInfo.
 * \param:[in] smp: If the session is enable secure media path, please set ture, othewise set false.
 *
 * \return: 0 success,  other value failed
 *\see: TSecPidInfo
 */
TSignedInt32 mtSecSetSessionPid(TTransportSessionId xTransportSessionId, TSecPidInfo *pxPidInfo, TBoolean smp);

/**
 * \brief
 *      Check if session corrisponding to xTransportSessionId is closed
 *
 * \param:[in] xTransportSessionId: The ID of TransportSessionId.
 * \param:[in] op: The operation type of seesion.
 *
 * \return: TRUE or FLASE
 * \see: TSessionOpType
 */
TBoolean mtSecGetSessionClosed(TTransportSessionId xTransportSessionId, TSessionOpType op);

/**
 * \brief
 *      Process the input data by Montage-LZ crypto engine, normally we use it to do stream data encryption.
 * \attention \n
 * called by user and sec api.
 *
 * \param xTransportSessionId: The ID of TransportSessionId.
 * \param op: The operation type of seesion.
 * \param:[in] size: the byte size of input data.
 * \param:[in] input: the input data pointer. It is a virtual address, adn we recommand to use the buffer from MMZ .
 * \param:[out] output: the input data pointe. It is a virtual address, adn we recommand to use the buffer from MMZ .
 *
 * return: 0 success,  other value failed
 * \see: TSessionOpType
 */
TSignedInt32 mtSecRawStreamDataProcess(TTransportSessionId xTransportSessionId, TSessionOpType op, TUnsignedInt32 size, const TUnsignedInt8 *input, TUnsignedInt8 *output);


/*!
 * \brief: set media buffer to the list to be protected.
 * \attention \n
 * called by user.
 *
 * \param:[in] addrPhy: physical start address if this buffer.
 * \param:[in] size: The byte size of protected buffer
 * \param:[in] type: The type of protected buffer
 *
 * \return: TRUE for success; FALSE for failed.
 * \see: MBType
 */
TBoolean mtSecSetProtectBuffer(phys_addr_t addrPhy, size_t size, MBType type);

/*!
 * \brief: Get a block of buffer from the list based on the type.
 * \attention \n
 * called by sec api.
 *
 * \param:[in] type: Media Buffer type.
 * \param:[out] mb: output the media buffer.
 * \param:[in] clr: set 1 for clearing this block of buffer from the list; 0 for keeping this block in the list.
 *
 * \return: TRUE for success; FALSE for failed.
 * \see: MBType and MBInfo
 */
TBoolean mtSecGetProtectBufferByType(MBType type, MBInfo *mb, mt_u8 clr);

/*!
 * \brief: clear the list of protected buffer.
 * \attention \n
 * called by user.
 *
 * \return: TRUE for success; FALSE for failed.
 */
TBoolean mtSecResetProtectBufferList(void);

/*!
 * \brief: Check if the TransportSessionId enable SMP (secure media path).
 * \attention \n
 * called by sec api.
 * \param:[in] xTransportSessionId: The ID of TransportSessionId.
 *
 * \return: TRUE for enable; FALSE for disable.
 */
TBoolean mtSecGetSmp(TTransportSessionId xTransportSessionId);

/*!
 * \brief: Get the EMI by TransportSessionId and session operation type.
 * \attention \n
 * called by user.
 * \param:[in] xTransportSessionId: The ID of TransportSessionId.
 * \param:[in] op: Encryption or decryption operation of the session.
 *
 * \return: TRUE for enable; FALSE for disable.
 * \see: TSessionOpType
 */
TUnsignedInt16 mtSecGetSessionEmi(TTransportSessionId xTransportSessionId, TSessionOpType op);

/*!
 * \brief: Get the ID number of Montage demux module by TransportSessionId .
 * \attention \n
 * called by sec api and user.
 * \param:[in] tsid: The ID of TransportSessionId.
 *
 * \return: the ID number of demux.
 */
mt_u32 mtSecGetPlatDmxId(mt_u32 tsid);

/*!
 * \brief: Clear tsid from local list.
 *
 * \param[] tsid
 */
 /*!
 * \brief:  Clear TransportSessionId from local list.
 * \attention \n
 * called by sec api and user.
 * \param:[in] tsid: The ID of TransportSessionId.
 *
 * \return: void.
 */
void mtSecClrPlatDmxId(mt_u32 tsid);

/*!
 * \brief: Get TSID that already paired with dmxid
 *
 * \param[In] dmxid
 *
 * \return tsid
 */
mt_u32 mtSecGetTsIdByDmxId(mt_u32 dmxid);
/*!
 * \brief: Setup auxiliary TA, It can solve the snowflake screen issue caused by switching between clear and scrambled streams
 * \attention \n
 * called by user..
 *
 * \return: 0 for success; other for failed.
 */
mt_u32 mtExtMTLZ_TEECInit(void);
/*!
 * \brief: Relese auxiliary TA
 * \attention \n
 * called by user..
 *
 * \return: 0 for success; other for failed.
 */
mt_u32 mtExtMTLZ_TEECDeinit(void);
/*!
 * \brief: Enable SMP function in advance by our auxiliary TA.
 * \attention \n
 * called by user. It can optimize the snow screen when TS from clear to scramble
 * \param:[in] isMianPlay: Enable main screen or sub screen SMP.
 * \param:[in] mt_play: It is DVB or OTT play.
 *
 * \return: 0 for success; other for failed.
 * \see: MTPlayType
 */
mt_u32 mtExtMTLZ_TEE_SMP_En(bool isMianPlay, MTPlayType mt_play);
/*!
 * \brief: Disable SMP function by our auxiliary TA.
 * \attention \n
 * called by user. It should be called to release auxiliary TA resource after Nagra Close Session
 * \param:[in] isMianPlay: disable main screen or sub screen SMP.
 *
 * \return: 0 for success; other for failed.
 * \see: MTPlayType
 */
mt_u32 mtExtMTLZ_TEE_SMP_Dis(bool isMianPlay);

  /*!
 * \brief: Set TransportSessionId for main viedo screen, default main vedio TransportSessionId is 0, it will be used by watermark.
 * \attention \n
 * called by user.
 * \param:[in] main_tsid: The ID of TransportSessionId used by main vedio.
 *
 * \return: void.
 */
mt_u32 mt_ngwm_configure_mainID(uint32_t main_tsid);

  /*!
 * \brief: Read the OTP filed value by byte offset address.
 * \attention \n
 * called by user and sec api.
 * \param:[in] addr: The byte offset address of OTP filed.
 * \param:[in] len: The read byte length, usually the value is not bigger than 4.
 * \param:[out] data: The output data pointer.
 *
 * \return: 0 success, other failed.
 */
mt_s32 mt_otp_read_byte(MT_U32 addr, MT_U32 len, MT_U8 *data);

  /*!
 * \brief: write the value to OTP filed by byte offset address.
 * \attention \n
 * called by user and sec api.
 * \param:[in] addr: The byte offset address of OTP filed.
 * \param:[in] len: The write byte length, usually the value is not bigger than 4.
 * \param:[in] data: The write data pointer.
 *
 * \return: 0 success, other failed.
 */
mt_s32 mt_otp_write_byte(MT_U32 addr, MT_U32 len, MT_U8 *data);

  /*!
 * \brief: allocate OTT re-encryption secure buffer.
 * \attention \n
 * called by sec api or user., it need boot arguments to support specific area, such as "ott0".
 * \param:[in] size: The byte size of a secure buffer, only support MB_OTT now.
 * \param:[in] mbType: The type of allocated buffer.
 *
 * \return: void*: NULL failed. other success.
 * \see: MBType
 */
extern void *mtSec_OTTAllocateSMPMemory(MT_U32 size, MBType mbType);
  /*!
 * \brief: release OTT re-encryption secure buffer.
 * \attention \n
 * called sec api or user.
 * \param:[in] p_ottSmpAddr: The secure buffer poniter.
 *
 * \return: void.
 */
extern mt_s32 mtSec_OTTFreeSMPMemory(void *p_ottSmpAddr);
/*!
 * \brief: Get the record channel channel by DMX ID.
 * \attention \n
 * called by sec api.
 *
 * \param:[in] dmxPort: The sofware inject used demux ID.
 * \param:[out] p_ch: The pointer to get the record channel value.
 * \param:[in] clr: If unbind the binding relationship between DMX ID and record channel.
 *
 * \return: TRUE for success; FALSE for failed.
 */
extern TBoolean mtSecGetRecChanByDmxID(mt_u32 dmxPort, mt_u8 *p_ch, mt_u8 clr);
/*!
 * \brief: Bind the record channel channel to DMX ID, new binding relationship will cover old one.
 * \attention \n
 * called by user..
 *
 * \param:[in] dmxPort: The sofware inject used demux ID.
 * \param:[in] ch: The record channel value.
 *
 * \return: TRUE for success; FALSE for failed.
 */
extern TBoolean mtSecSetRecChanByDmxID(mt_u32 dmxPort, mt_u8 ch);
/*!
 * \brief: Unbind the binding relationship between all DMX IDs and record channels.
 * \attention \n
 * called by user.
 *
 * \return: TRUE for success; FALSE for failed.
 */
extern TBoolean mtSecResetRecChanDmxIDList(void);
/*!
 * \brief: Get the violation information of output control.
 * \attention \n
 * called by user. You can adjust output resolution according to the violation information.
 *
 * \param:[out] p_opcStatus: The pointer of the violation information of output control.
 *
 * \return: TRUE for success; FALSE for failed.
 * \see: MTSecOpREEFlag
 */
TBoolean mtSecGetOpREEFlag(MTSecOpREEFlag *p_opcStatus);
/*!
 * \brief: Open module clock for NAGRA's CERT, Montag-lz's crypto and related algorithm.
 * \attention \n
 * called by user.
 *
 * \return: TRUE for success; FALSE for failed.
 */
void mtSecRelatedModuleClkEnable(void);

/** @} */  /** <!-- ==== API Declaration End ==== */

#ifdef __cplusplus
}
#endif

#endif
