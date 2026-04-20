/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************
  File Name     : mt_drv_adec.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/11/25
  Description   :
  History       :
  1.Date        : 2015/11/25
    Author      :
    Modification: Created file

******************************************************************************/

#ifndef __MT_DRV_ADEC_H__
#define __MT_DRV_ADEC_H__

#include "mt_type.h"
#include "mt_unf_sound.h"
#include "mt_mpi_adec.h"
#include "mt_mpi_mem.h"
#include "mt_module.h"
#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */
typedef enum mtADEC_ATTR
{
	ADEC_ATTR_CODECID = 0,
	ADEC_ATTR_WORKSTATE,
	ADEC_ATTR_INBUFSIZE,
	ADEC_ATTR_OUTBUFNUM,
	ADEC_ATTR_DECOPENPARAM,
  ADEC_ATTR_EosStateFlag,
  ADEC_ATTR_ALLATTR
}mtADEC_ATTR;
#ifndef MT_ADEC_MAX_INSTANCE
#define ADEC_INSTANCE_MAXNUM 2     /* max decoder instance */
#else
#define ADEC_INSTANCE_MAXNUM MT_ADEC_MAX_INSTANCE
#endif

#define ADEC_MAX_INPUT_BLOCK_SIZE 0x10000   /* max input data size of decoder */
#define ADEC_MAX_VOLUME 100 /* 32 */

#define ADEC_MAX_CHANNLES 8

/* Max frame number can be used in ADEC */
#define ADEC_MAX_WORK_BUFFER_NUMBER 40
#define ADEC_DEFAULT_WORKINGBUF_NUM 30

#define ADEC_MAX_INPUT_BUFFER_SIZE (16 * 1024 * 1024)
#define ADEC_MIN_INPUT_BUFFER_SIZE (8 * 1024)
#define ADEC_DEFAULT_INPUT_BUFFER_SIZE (128 * 1024)
#define ADEC_AD_ES_BUFFER  (128*1024)
#define ADEC_AD_PCM_OUT_FRAME_MAX 36
#define ADEC_AD_PCM_BUF_SIZE           (ADEC_AD_PCM_OUT_FRAME_MAX * 4 * 1024)  // 36frames for 1024 ; 32 for 1152; 24 for 1536; 0.768s for 48kHz

/* global vars for pts process 							*/
#define     ADEC_MAX_STORED_PTS_NUM 2048
/* note: ADEC_MAX_STORED_PACKET_NUM is not less than  ADEC_MAX_STORED_PTS_NUM */
#define     ADEC_MAX_STORED_PACKET_NUM (ADEC_MAX_STORED_PTS_NUM)

/* max adec consume bytes for one frame */
#define ADEC_MAX_FRAME_CONSUME_THD   (1024*128)

#define CHECK_ADEC_HANDLE(hAdec) \
    do                                                  \
    {                                                   \
        if ((mt_s32)(hAdec) >= ADEC_INSTANCE_MAXNUM)                                  \
        {                                               \
            MT_ERR_ADEC("  invalid Adec handle =0x%x!\n", hAdec);          \
            return MT_FAILURE;              \
        }                                               \
    } while (0)

#define CHECK_ADEC_NULL_PTR(ptr) \
    do                                                  \
    {                                                   \
        if (NULL == ptr)                             \
        {                                               \
            MT_ERR_ADEC("invalid NULL poiner!\n");          \
            return MT_FAILURE;                  \
        }                                               \
    } while (0)

#define CHECK_NULL_PTR_NORET(ptr) \
    do                                                  \
    {                                                   \
        if (NULL == ptr)                             \
        {                                               \
            MT_ERR_ADEC("invalid NULL poiner!\n");          \
            return;                  \
        }                                               \
    } while (0)

#define CHECK_ADEC_STATE_NOMUTE(bEnable) \
    do                                                  \
    {                                                   \
        if (MT_FALSE == bEnable)                             \
        {                                               \
            MT_ERR_ADEC(" adec state invalid\n");          \
            return MT_FAILURE; \
        }                                               \
    } while (0)

#define CHECK_ADEC_STATE(bEnable, adecMutex) \
    do                                                  \
    {                                                   \
        if (MT_FALSE == bEnable)                             \
        {                                               \
            MT_ERR_ADEC(" adec state invalid\n");          \
            ADEC_UNLOCK(adecMutex);\
            return MT_FAILURE; \
        }                                               \
    } while (0)

#define CHECK_ADEC_STATE_WARNING(bEnable, adecMutex) \
    do                                                  \
    {                                                   \
        if (MT_FALSE == bEnable)                             \
        {                                               \
            MT_WARN_ADEC(" adec state invalid\n");          \
            ADEC_UNLOCK(adecMutex);\
            return MT_FAILURE; \
        }                                               \
    } while (0)

#define CHECK_ADEC_STATEARG2(bEnable, adecMutex1,adecMutex2) \
		do                                                  \
{                                                   \
		if (MT_FALSE == bEnable)                             \
		{                                               \
				MT_ERR_ADEC(" adec state invalid\n");          \
				ADEC_UNLOCK(adecMutex1);\
				ADEC_UNLOCK(adecMutex2);\
				return MT_FAILURE; \
		}                                               \
} while (0)


#define CHECK_ADEC_STATEARG3(bEnable, adecMutex1,adecMutex2,adecMutex3) \
        do                                                  \
{                                                   \
        if (MT_FALSE == bEnable)                             \
        {                                               \
                MT_ERR_ADEC(" adec state invalid\n");          \
                ADEC_UNLOCK(adecMutex1);\
                ADEC_UNLOCK(adecMutex2);\
                ADEC_UNLOCK(adecMutex3);\
                return MT_FAILURE; \
        }                                               \
} while (0)



#define CHECK_ADEC_OUTBUF_NUMBER(number) \
    do                                                  \
    {                                                   \
        if (number >= ADEC_MAX_WORK_BUFFER_NUMBER)                             \
        {                                               \
            MT_ERR_ADEC("invalid output No.(%d)!\n",number);          \
            return MT_FAILURE;                  \
        }                                               \
    } while (0)

#define CHECK_ADEC_OUTBUF_NUMBER_NORET(number) \
    do                                                  \
    {                                                   \
        if (number >= ADEC_MAX_WORK_BUFFER_NUMBER)                             \
        {                                               \
            MT_ERR_ADEC("invalid output No.(%d)!\n",number);         \
            return;                  \
        }                                               \
    } while (0)

#define CHECK_ADEC_VOLUME(volume) \
    do                                                  \
    {                                                   \
        if (volume > ADEC_MAX_VOLUME)                    \
        {                                               \
            MT_WARN_AO("invalid ADEC Volume(%d)\n", volume);   \
            return ADEC_MAX_VOLUME;               \
        }                                               \
    } while (0)

#define  MT_MPI_ADEC_RetUserErr(DrvErrCode, adecMutex)  \
    do                                                  \
    {                                                   \
        if (MT_SUCCESS != DrvErrCode)                   \
        {                                               \
            MT_ERR_ADEC(" ErrCode =0x%x\n",DrvErrCode); \
            ADEC_UNLOCK(adecMutex); \
            return DrvErrCode; \
        }                                               \
    } while (0)


#define  MT_MPI_ADEC_RetUserErrARG2(DrvErrCode, adecMutex1,adecMutex2)  \
		do                                                  \
{                                                   \
		if (MT_SUCCESS != DrvErrCode)                   \
		{                                               \
				MT_ERR_ADEC(" ErrCode =0x%x\n",DrvErrCode); \
				ADEC_UNLOCK(adecMutex1); \
				ADEC_UNLOCK(adecMutex2); \
				return DrvErrCode; \
		}                                               \
} while (0)

#define  MT_MPI_ADEC_RetUserErr2(DrvErrCode, adecMutex) \
    do                                                  \
    {                                                   \
        MT_ERR_ADEC(" ErrCode =0x%x\n",DrvErrCode); \
        ADEC_UNLOCK(adecMutex); \
        return DrvErrCode; \
    } while (0)

#define  MT_MPI_ADEC_RetUserErr2ARG2(DrvErrCode, adecMutex1,adecMutex2) \
		do                                                  \
{                                                   \
		MT_ERR_ADEC(" ErrCode =0x%x\n",DrvErrCode); \
		ADEC_UNLOCK(adecMutex1); \
		ADEC_UNLOCK(adecMutex2); \
		return DrvErrCode; \
} while (0)


typedef enum
{
    ADEC_CMD_CTRL_START = 0,
    ADEC_CMD_CTRL_STOP,
    ADEC_CMD_CTRL_BUTT
} ADEC_CMD_CTRL_E;


typedef enum
{
    ADEC_CMD_PROC_SAVE_PCM = 0,
    ADEC_CMD_PROC_SAVE_ES,
    ADEC_CMD_PROC_BUTT
} ADEC_CMD_SAVE_E;


/* Buffer 												*/
typedef struct mtADEC_OUTPUTBUF_S
{

    mt_u64   u64PTS;      /* Play Time Stamp may be interpolated	*/
    mt_u64   u64OrgPTS;  /* original Play Time Stamp 			*/
    MT_BOOL  bFlag;     /* Buffer Flag 				*/
    ulong * OutBuf;     /* Output Buffer 			*/
    ulong * ps32PcmOutBuf;
    ulong * ps32BitsOutBuf;       /* NULL, if decoder dont suppourt iec 61937 */
    mt_u32   u32PcmOutSamplesPerFrame;
    mt_u32   u32BitsOutBytesPerFrame;
    mt_u32   u32OutChannels;
    mt_u32   u32OutChannelsExist;
    mt_u32   u32OutSampleRate;
    MT_BOOL  bInterleaved;
    mt_u32   u32BitPerSample; /**< Bit per sample */
	mt_u32   u32FrameIndex;
    //mt_u32  buf_size;
    MT_BOOL b_eos;
    MT_BOOL bEac4TimeSampleRate;
    mt_u32  u32ESReadPos;
    mt_u32  u32chan;    //0 1st chan;  1 2nd spdif chan(when 1st is ac3/eac3  2nd is used)
    mt_u32  u32pcmcrc;  //all channels
} ADEC_OUTPUTBUF_S;

typedef struct mtADEC_PTS_S
{
    mt_u64 u64PtsMs;        /* Play Time Stamp 			         */
    mt_u32 u32BegPos;       /* Stream offset address of PTS 	 */
    mt_u32 u32EndPos;    /* Stream offset end address of PTS  */
    mt_u32 u32FrameIndex;
} ADEC_PTS;

typedef struct mtADEC_PACKET_S
{
    MT_BOOL bPacketEosFlag;        /* patket EOS 			         */
    mt_s32  s32BegPos;       /* Stream offset address of patket 	 */
    mt_u32  u32EndPos;       /* Stream offset end address of patket  */
} ADEC_PACKET;

typedef struct mtADEC_STREAM_BUFFER_S
{
    mt_u8   *pu8Data;               /*buffer addr*/
    mt_u8   *pu8PhyData;               /*buffer phy addr*/
    mt_u32   u32BufSize;                /*buffer length*/
    mt_u32  AudErrorCheckContinuationNum;
    mt_s32   s32BufReadPos;               /*buffer s32BufReadPos ptr*/
    mt_u32   u32BufWritePos;              /*buffer u32BufWritePos ptr*/
    mt_u32   u32BufFree;               /*buffer u32BufFree length*/

    //mt_u32   u32BufLeftBytes;               /*buffer continuous unread area length*/
    mt_u32 u32BufPaddingSize;

    //#ifdef ADEC_MMZ_BUF_SUPPORT
    mt_mmz_buf_s sAdecInMMzBuf;
    //#endif
    mt_u32   u32StreamWritePos;
    mt_u32   u32StreamReadPos;
    mt_u32   u32Boundary;	/* pts read pointers wrap point */
    MT_BOOL b_eos; /* EOS flag: 0 Normal 1 User SetEosFlag 2 Consume stream over */
} ADEC_STREAM_BUFFER_S;

typedef struct mtADEC_PROC_ITEM_S
{
    MT_BOOL        bAdecWorkEnable;
    mt_char        szCodecType[32];
    mt_char        szCodecDescription[32];
    mt_u32           u32CodecID;
    MT_BOOL         enFmt;
    MT_UNF_SAMPLE_RATE_E enSampleRate;
    MT_UNF_BIT_DEPTH_E   enBitWidth;
    mt_u32          u32PcmSamplesPerFrame;
    mt_u32           u32FramnNm;
    mt_u32           u32ErrFrameNum;
    mt_u32           u32BufSize;
    mt_s32           s32BufRead;
    mt_u32           u32BufWrite;
    mt_u32           u32FrameSize;
    mt_u32           u32FrameRead;
    mt_u32           u32FrameWrite;
    mt_u32           u32FrameReadWrap;
    mt_u32           u32FrameWriteWrap;
    mt_u32           u32PtsLost;
    mt_u32           u32Volume;
	mt_u32           u32OutChannels;
	mt_u32           u32BitsOutBytesPerFrame;
    mt_u32           u32DbgGetBufCount_Try;
    mt_u32           u32DbgGetBufCount;
    mt_u32           u32DbgPutBufCount_Try;
    mt_u32           u32DbgPutBufCount;
    mt_u32           u32DbgReceiveFrameCount_Try;
    mt_u32           u32DbgReceiveFrameCount;
    mt_u32           u32DbgSendStraemCount_Try;
    mt_u32           u32DbgSendStraemCount;
    mt_u32           u32DbgTryDecodeCount;
    mt_u32           u32FrameConsumedBytes;
    mt_u32           u32LastCorrectFrameNum;
    mt_u32           u32ThreadId;

    ADEC_CMD_CTRL_E    enPcmCtrlState;
    ADEC_CMD_CTRL_E    enEsCtrlState;
    mt_u32                  u32SavePcmCnt;
    mt_u32                  u32SaveEsCnt;
    mt_char              filePath[512];

    mt_u32           ThreadBeginTime;
    mt_u32           ThreadEndTime;
    mt_u32           ThreadScheTimeOutCnt;
    mt_u32           ThreadExeTimeOutCnt;

    mt_u32           u32AdecSystemSleepTime;

    mt_u32           u32CodecUnsupportNum;
    mt_u32           u32StreamCorruptNum;
} ADEC_PROC_ITEM_S;

#define ADEC_LOCK_DECLARE(p_mutex) ;                 \
    static pthread_mutex_t p_mutex = PTHREAD_MUTEX_INITIALIZER;

#define ADEC_LOCK_INIT(p_mutex) ;                 \
    (void)pthread_mutex_init(p_mutex, NULL)

#define ADEC_LOCK_DESTROY(p_mutex) \
    (void)pthread_mutex_destroy(p_mutex)

#define ADEC_LOCK(p_mutex) \
    (void)pthread_mutex_lock(p_mutex)

#define ADEC_UNLOCK(p_mutex) \
    (void)pthread_mutex_unlock(p_mutex)

#define DRV_ADEC_DEVICE_NAME "mt_adec"


typedef enum
{
    SND_ENGINE_TYPE_PCM = 0,
    SND_ENGINE_TYPE_SPDIF_RAW = 1,
    SND_ENGINE_TYPE_HDMI_RAW = 2,
#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
    SND_ENGINE_TYPE_MIXBUF_DD = SND_ENGINE_TYPE_HDMI_RAW,
#endif	

    SND_ENGINE_TYPE_BUTT
} SND_ENGINE_TYPE_E;

typedef struct mtADEC_SHM_HEAD_S
{
	mt_u32 id;
	mt_u32 size;
}ADEC_SHM_HEAD_S;

/* ADEC buffer ID */
enum MT_ADEC_BUF_ID_e
{
	MT_ADEC_INBUF_ID_0 = 0x20190807,
	MT_ADEC_OUTBUF_ID_0,
	MT_ADEC_PKT_ID_0,
	MT_ADEC_FRM_ID_0,

	MT_ADEC_INBUF_ID_1,
	MT_ADEC_OUTBUF_ID_1,
	MT_ADEC_PKT_ID_1,
	MT_ADEC_FRM_ID_1,

#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
    MT_ADEC_INBUF_ID_2,
    MT_ADEC_OUTBUF_ID_2,
    MT_ADEC_PKT_ID_2,
    MT_ADEC_FRM_ID_2,
#endif
};

/* ADEC Buffer Manage for Symphony4 */
typedef struct mtADEC_BUF_S
{
	mt_u32 u32PhyAddr;		/* physical address */
	mt_u32 u32KnVirAddr;	/* kernel virtual address */
	mt_u32 u32UsrVirAddr;	/* user virtual address */
	mt_u32 u32Size;

	mt_u32 u32Read;
	mt_u32 u32Write;
}ADEC_BUF_S;

/** Audio Input ES Packet */
/* AVCPU Read-only */
/* see: MT_HADECODE_INPACKET_S */
typedef struct mtADEC_ES_PACKET_S
{
    mt_u8  *pu8Data;	/* physical address */
    mt_u32  u32Size;

    mt_u64  u64PtsUs;
    MT_BOOL bEOS;
}ADEC_ES_PACKET_S;

/* ADEC Packet Que */
typedef struct mtADEC_ES_PACKET_QUEUE_S
{
	mt_u32 u32Count;
	mt_u32 u32Read;
	mt_u32 u32Write;

	ADEC_ES_PACKET_S packet_que[0];
} ADEC_ES_PACKET_QUEUE_S;

typedef struct mtADEC_PTS_Queue_S
{
    mt_u64   u64LastPtsMs;
    mt_u64   u64LastStorePtsMs;
    mt_u64   u64RecyleStorePtsMs;
    mt_u64   u64RecycleFirstPtsMs;
    mt_u64   u64PtsBeforeRevise;
    mt_u32   ulPTSread;    /* PTS buffer read  ptr */
    mt_u32   ulPTSwrite;    /* PTS buffer write ptr */
    ADEC_PTS tPTSArry[ADEC_MAX_STORED_PTS_NUM];
} ADEC_PTS_QUE_S;

/**Audio Print/Debug System(APDS)**/
/**print position**/
typedef enum mt_APDS_PRINT_POS
{    
    //common info
    APDS_POS_ATTACH = 0,
    APDS_POS_INIT,
    APDS_POS_START,
    APDS_POS_STOP,
    APDS_POS_RESET,
    
    //decode flow chan0
    APDS_POS_CH0_IN_BUFF = 8,
    APDS_POS_CH0_OUT_BUFF,
    APDS_POS_CH0_DEC_STA,
    APDS_POS_CH0_DEC_OK,
    APDS_POS_CH0_DEC_FAIL,
    APDS_POS_CH0_RD_UPDATE,
    APDS_POS_CH0_GET_QUE,
    APDS_POS_CH0_TO_AO,
    
    //decode flow chan1
    APDS_POS_CH1_IN_BUFF = 16,
    APDS_POS_CH1_OUT_BUFF,
    APDS_POS_CH1_DEC_STA,
    APDS_POS_CH1_DEC_OK,
    APDS_POS_CH1_DEC_FAIL,
    APDS_POS_CH1_RD_UPDATE,
    APDS_POS_CH1_GET_QUE,
    APDS_POS_CH1_TO_AO,

    //audio description
    APDS_POS_APTS = 24,
    APDS_POS_AD_ = 25,
    APDS_POS_TEMP_DBG = 31,    
    
    APDS_POS_INVALID = 32
}APDS_PRINT_POS;
/* audio print/debug system info 												*/
typedef struct mt_AUDIO_APDS_S
{
    mt_u32 enable;
    mt_u32 prt_flag;//show print log
} AUDIO_APDS_S;

/* SHARE BWTWEEN AP/AV 												*/
typedef struct mtADEC_SHARED_INFO_S
{
    mt_u32 init;

/* es buf */
    phys_addr_t   es_Start;
    mt_u32   es_Size;
    mt_u32   es_Boundary;
    mt_u32   es_PaddingSize;
    ulong   es_ReadPos;
    ulong   es_WritePos;
    mt_u32   es_ReadAllPos;
	mt_u32   es_eof;
/* out buf */

    phys_addr_t  outbuf_Start;
    mt_u32  outbuf_Num;
    mt_u32  outbuf_ReadPos;
    mt_u32  outbuf_WritePos;

/* out buf2 */

    phys_addr_t  outbuf_Start2;
    mt_u32  outbuf_ReadPos2;
    mt_u32  outbuf_WritePos2;

#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
/* out buf3 */	   
    mt_u32  outbuf_Start3;    
    mt_u32  outbuf_ReadPos3;
    mt_u32  outbuf_WritePos3;
#endif	

    mt_u32 AudErrorCheckContinuationNum;
    ADEC_OUTPUTBUF_S outBuf[ADEC_MAX_WORK_BUFFER_NUMBER];
    ADEC_OUTPUTBUF_S outBuf2[ADEC_MAX_WORK_BUFFER_NUMBER];
#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
    ADEC_OUTPUTBUF_S outBuf3[ADEC_MAX_WORK_BUFFER_NUMBER];
#endif	

	mt_u32 debug_firmware_pcmcrc; //1:on:0:off

#if 1//def CONFIG_MT_AUDIO_AD
    mt_u32 flag_es;   //1:es,2:pcm;0:off
    ulong  ad_pcmbuffer[ADEC_AD_PCM_BUF_SIZE];
    ulong ad_pcm_rp;
    ulong ad_pcm_wp;
#endif

#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
    volatile mt_s32  inBufId;
    volatile mt_s32  inPktQueueId;
    volatile mt_s32  frameQueueId[3];
#endif

#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
    ADEC_PTS_QUE_S pst_PTSQue;
#endif
	
    mt_u32 u32StreamCorruptNum;

    AUDIO_APDS_S    apds;//for audio print and debug
} ADEC_SHARED_INFO_S;


/* SHARE BWTWEEN AP/AV 												*/
typedef struct mtADEC_OUTBUF_SIZE_S
{
    mt_u32 outbuf_size;
    mt_u32 outbuf_size_chan2;
#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
    mt_u32 outbuf_size_chan3;
#endif	
} ADEC_OUTBUF_SIZE_S;


/*new audio ioctl cmd for symphony linux*/
#define     DRV_ADEC_IPC_ATTACH _IOW(MT_ID_ADEC, 0, mt_handle)
#define     DRV_ADEC_IPC_INIT _IOW(MT_ID_ADEC, 1, MT_HADECODE_OPENPARAM_S*)
#define     DRV_ADEC_IPC_START _IOW(MT_ID_ADEC, 2, mt_handle)
#define     DRV_ADEC_IPC_STOP _IO(MT_ID_ADEC, 3)

#define     DRV_ADEC_IPC_UPDATE_ES_WRITE _IOW(MT_ID_ADEC, 4, mt_handle)
#define     DRV_ADEC_IPC_UPDATE_ES_READ _IOW(MT_ID_ADEC, 5, mt_handle)
#define     DRV_ADEC_IPC_GET_OUTPUT_SIZE _IOW(MT_ID_ADEC, 6, mt_handle)

#define     DRV_ADEC_IPC_RESET _IOW(MT_ID_ADEC, 7, mt_handle)

#define     DRV_ADEC_FORCE_CLOSE _IO(MT_ID_ADEC, 10)
#define     DRV_ADEC_IPC_AVC_CFG _IOW(MT_ID_ADEC, 11, MT_HADECODE_AVC_PARAM_S*)
#define     DRV_ADEC_SHM_CREATE _IOR(MT_ID_ADEC, 12, mt_u32 *)
#define     DRV_ADEC_SHM_RELEASE _IO(MT_ID_ADEC, 13)
#define     DRV_ADEC_IPC_PAUSE _IO(MT_ID_ADEC, 14)
#define     DRV_ADEC_IPC_RESUME _IO(MT_ID_ADEC, 15)
#define     DRV_ADEC_SET_DDP_TEST_MODE _IOW(MT_ID_ADEC, 16, mt_u32 *)

/* 'IOC_TYPE_ADEC' means ADEC magic macro */
#define     DRV_ADEC_PROC_INIT _IOW(MT_ID_ADEC, 8, ADEC_PROC_ITEM_S *)
#define     DRV_ADEC_PROC_EXIT _IO(MT_ID_ADEC, 9)


/*Define Debug Level For MT_ID_AO                     */

mt_s32	ADEC_Init(const mt_char* pszCodecNameTable[]);
mt_s32	ADEC_deInit(mt_void);
mt_s32	ADEC_Reset(mt_handle hAdec);
mt_s32	ADEC_Open(mt_handle *phAdec);
mt_s32	ADEC_Close(mt_handle hAdec);
mt_s32 ADEC_GetAttr(mt_handle hAdec, mt_u32 u32Command, void  *pstAttr);
mt_s32  ADEC_GetBuffer (mt_handle hAdec, mt_u32 u32RequestSize, MT_UNF_STREAM_BUF_S *pstStream);
mt_s32	ADEC_GetBuffer2 (mt_handle hAdec, mt_u32 u32RequestSize, MT_UNF_STREAM_BUF_S *pstStream1,
                                MT_UNF_STREAM_BUF_S *pstStream2);
mt_s32	ADEC_GetBufferStatus (mt_handle hAdec, ADEC_BUFSTATUS_S *ptsBufStatus);
mt_s32	ADEC_GetDebugInfo(mt_handle hAdec, ADEC_DEBUGINFO_S *pstDebuginfo);
mt_s32	ADEC_GetStatusInfo(mt_handle hAdec, ADEC_STATUSINFO_S *pstStatusinfo);
mt_s32	ADEC_GetStreamInfo(mt_handle hAdec, ADEC_STREAMINFO_S * pstStreaminfo);
mt_s32  ADEC_GetHaSzNameInfo(mt_handle hAdec,ADEC_SzNameINFO_S *pHaSznameInfo);

mt_s32 ADEC_GetAnalysisPcmData(mt_handle hAdec);
mt_s32 ADEC_GetAudSpectrum(mt_u16 *pSpectrum ,mt_u32 u32BandNum);


mt_s32  ADEC_PutBuffer (mt_handle hAdec, const MT_UNF_STREAM_BUF_S* pstStream, mt_u64 u64PtsMs, MT_BOOL b_eos);
mt_s32	ADEC_PutBuffer2 (mt_handle hAdec, const MT_UNF_STREAM_BUF_S *pstStream1, const MT_UNF_STREAM_BUF_S *pstStream2,
                                mt_u32 u32PtsMs);
mt_s32 ADEC_TryReceiveFrame (mt_handle hAdec, MT_UNF_AO_FRAMEINFO_S *pstAOFrame, ADEC_EXTFRAMEINFO_S *pstExtInfo);
mt_s32	ADEC_RegisterDeoder(const char *pszDecoderDllName);

mt_s32	ADEC_ShowRegisterDeoder(mt_void);
mt_s32 ADEC_FoundSupportDeoder(HA_FORMAT_E enFormat,mt_u32 *penDstCodecID);

mt_s32	ADEC_SendStream (mt_handle hAdec, const MT_UNF_STREAM_BUF_S *pstStream, mt_u64 u64PtsMs);
mt_s32 ADEC_SetAttr(mt_handle hAdec,mt_u32 command  ,void  *pstParam);
mt_s32	ADEC_SetConfigDeoder(mt_handle hAdec, mt_void *pstConfigStructure);

mt_void	ADEC_DbgCountTryGetBuffer(mt_handle hAdec);
mt_void	ADEC_DbgCountGetBuffer (mt_handle hAdec);
mt_void	ADEC_DbgCountTryReceiveFrame (mt_handle hAdec);
mt_void	ADEC_DbgCountReceiveFrame (mt_handle hAdec);
mt_void	ADEC_DbgCountTrySendStream (mt_handle hAdec);
mt_void	ADEC_DbgCountSendStream (mt_handle hAdec);
mt_void	ADEC_DbgCountTryPutBuffer(mt_handle hAdec);
mt_void	ADEC_DbgCountPutBuffer(mt_handle hAdec);
mt_s32	ADEC_SetEosFlag(mt_handle hAdec);
mt_s32   ADEC_SetCodecCmd(mt_handle hAdec, mt_void *pstCodecCmd);
mt_s32   ADEC_GetDelayMs(mt_handle hAdec,mt_u64 *pDelayMs);
mt_s32   ADEC_DropStream(mt_handle hAdec, mt_u64 u32SeekPts);
mt_s32   ADEC_CheckNewEvent(mt_handle hAdec, ADEC_EVENT_S *pstNewEvent);
void ADEC_Hacker_Set_PcmCrc_onlyonlyforTest(mt_u32 onoff);

mt_s32 ADECFlushBuf(mt_handle hAdec);
mt_s32 ADECResetInBuf_vsb(mt_handle hAdec);
mt_s32 ADECResetOutBuf_vsb(mt_handle hAdec);


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __MPI_PRIV_ADEC_H__ */
