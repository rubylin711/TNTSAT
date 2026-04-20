/*
 * @file nv_spr.c
 * @brief Nagravision Stream Processing APIs implemetation on Montage Symphony4 platform
 *
 * Copyright (C) 2020 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <signal.h>

#include <tee_client_api.h>
#include "nvta_tflts_gptee.h"

#include "mt_common.h"
#include "mt_adp_audio.h"
#include "mt_adp_mpi.h"
#include "mt_adp_pvr.h"
#include "mt_unf_common.h"
#include "mt_unf_ecs.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "mt_unf_pvr.h"
#include "mt_unf_gpio.h"
#include "mt_unf_disp.h"

#include "mt_unf_cipher_v2.h"
#include "mt_unf_dma.h"

#include "mt_hdmi.h"
#include "nv_spr.h"
#include "nocs_sec_impl.h"
#include "mt_sec_ext.h"
#include "mt_spr_ext.h"
#include "mt_record.h"
#include "mt_unf_misc.h"

#include <semaphore.h>
#include <sys/socket.h>
#include <linux/netlink.h>
//#include <mt_drv_opc.h>
//#include <net/netlink.h>
//#include <net/sock.h>

//#define ONLY_TEST_FOR_CLEAR_TS // test for str

#ifndef  _MT_WITH_TALTS_
//#define STOP_WITH_STILL_FRAME   //pre-intefrtion test needs to close window one by one
#endif

#define CREATE_WD_LATER  //窗口后续播台的时候再创建

//#define CLOSE_SECOND_TS

#define EMSG(fmt, ...)   //printf("[ERR]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define DMSG(fmt, ...)   //printf("[DBG]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
//注意:加了这个打印会导致OTT dual 重复播台的时候，播台失败,DVB+OTT也会失败。


#define TMSG(fmt, ...)  //printf("[DBG]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define NEW_MSG(fmt, ...)  //printf("[DBG]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define CFG_TFL_USE_DYNC_SHMEM 1

#define OPC_NETLINK_ID         24
#define OPC_MAX_RECV_SIZE      64

#if CFG_TFL_USE_DYNC_SHMEM
#define TFL_DYNC_SHMEM_SIZE (3 * 1024 * 1024)
uint8_t tfl_dync_sh_mem[TFL_DYNC_SHMEM_SIZE] = {1};
TEEC_SharedMemory tfl_shm = {
    .buffer = (void *)tfl_dync_sh_mem,
    .size = TFL_DYNC_SHMEM_SIZE,
    .flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT,
};
#endif



/*
TSecFunctionTable *grant_secGetFunctionTable;
TSecStreamSession grantTSecStreamSession_0 = NULL;
TSecStreamSession grantTSecStreamSession_1 = NULL;
TTransportSessionId grantTransportSessionId[2] = {10, 11};
TUnsignedInt8 grantpxKeyId[4][16] = {0,};
TUnsignedInt16 grantKeySlotIndex[4] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
extern int ott_rec_test_main();
*/
void mtTestUpdateDisplayResolution(MT_UNF_ENC_FMT_E encFormat);
MT_UNF_ENC_FMT_E testFormat = MT_UNF_ENC_FMT_1080i_50;
sem_t TsBufferSem;
//mt_u8 MTPVRCount = 0;

void mtTestSMPCloseEventCallbackStart(TTransportSessionId tsid, MT_SEC_EVENT_E sec_event);
void mtTestSMPCloseEventCallbackEnd(TTransportSessionId tsid, MT_SEC_EVENT_E sec_event);

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

extern void mt_tfl_command_mutext_init(void);
extern void drv_disp_reset_hdmi_fmt_ability(void);
extern u32 drv_disp_get_auto_hdmi_fmt(void);

static long long get_sys_time_ms(void)
{
    long long time_ms = 0;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    time_ms = ((long long)tv.tv_sec*1000000 + tv.tv_usec) / 1000;

    return time_ms;
}


static RET_CODE _lockTsBuffer(void)
{

  return SUCCESS;

  int ret;

  ret = sem_wait(&TsBufferSem); //wait forver
  if (ret != 0) {
	  	printf("_lockTsBuffer TsBufferSem err\n");
		return 1;
	}
  return SUCCESS;
}

static void _unlockTsBuffer()
{

return;

  int ret;

  ret = sem_post(&TsBufferSem); //wait forver
  if (ret != 0) {
  	    printf("_unlockFilter TsBufferSem err\n");

	}
}


//audio dump
//#define FILE_DUMP_AUDIO

//#define MT_GRANT_TEST_PIP_OTT

FILE *gtsfp = NULL, *gtsfpIn = NULL, *gtsRefp = NULL;

FILE *gReadVfp = NULL;

FILE *gVfp = NULL;
#ifdef FILE_DUMP_AUDIO
FILE *gAfp = NULL;
//const char *test_audio_file_name = "/root/simbad/emi4024_audio.m4s";
//const char *test_audio_file_name = "/root/simbad/emi4029_audio.aac";
//DMA debug
const char *test_audio_file_name = "/media/sda1/audio.es";
//const char *test_audio_file_name = "/root/simbad/emi4024_audio.aac";
//const char *test_audio_file_name = "/root/simbad/emi402A_audio.aac";

#endif

const char *ts_test_file_name = "/media/sda1/pvr_re0.ts";
const char *ts_test_re_file_name = "/media/sda1/pvr_re1.ts";

const char *ts_test_file_nameIn = "/media/sda1/ottIn.ts";

static void open_test_file(void);
static void close_test_file(void);
static int write_test_file(FILE *fp, unsigned char *pData, unsigned int len);
static int read_test_file(FILE *fp, unsigned char *pData, unsigned int len);


char *test_writeFileName = "/media/sda1/grant_0.ts";
FILE *gwriteFileVfp = NULL;

TUnsignedInt8 *grantTestBuffer = NULL;
TUnsignedInt8 *grantTestBufferPhy = NULL;

extern TSignedInt32 mtSecSetIsMultiSessionFlag(TTransportSessionId xTransportSessionId, TBoolean isMultiSession);
extern mt_void MTADP_PVR_CallBack(mt_u32 u32ChnID, MT_UNF_PVR_EVENT_E EventType, mt_s32 s32EventValue, mt_void *args);
extern TSignedInt32 mtSecRawStreamDataProcess_phy(TTransportSessionId xTransportSessionId, TSessionOpType op, TUnsignedInt32 size, const TUnsignedInt8 *input, TUnsignedInt8 *output);


#define MT_INVALID_PTS64			(MT_U64)(-1)


//replaying test
//#define LOCAL_TS_BUF_SIZE (188 * 1280 * 10)
//#define LOCAL_TS_BUF_SIZE (188 * 1280)
//10M for replay buffering test
#define LOCAL_TS_BUF_SIZE (0xA00000)
//total mmz size:32M, ott protect buffer:16M
#define LOCAL_OTT_SMP_BUF_SIZE (0x1000000)

//intermediate inject data buffer: dvb inject size=188, replaying inject size=1280
//#define TS_INT_BUF_SIZE (1280)
#define TS_INT_BUF_SIZE (1280*4)
//#define SMP_DDR_ALIGN_UNIT  (0x40000)
/*Size shall be 64K bytes aligned*/
//#define DEMUX_TS_BUF_SIZE 0x100000
//#define DEMUX_TS_BUF_SIZE  (2 * 0x200000) //0x200000//(0x200000*2)//0x200000
#define DEMUX_TS_BUF_SIZE  (0x500000) //(4 * 0x200000) //0x200000//(0x200000*2)//0x200000
//modify to 0x400000 can support 4 record channels in parallel
#define DEMUX_TS_BUF_SIZE_REC  (0x500000) //(4 * 0x200000) //0x200000//(0x200000*2)//0x200000

/*For Raw EMIs: 4020, 4021, 4023, injectData::size = 1280*/
#define DEMUX_RAW_REP_PUT_SIZE (188 * 512)//(1280*10)//
/*For EMIs: 0020, 0021, 0023, injectData::size = 1316*/
#define DEMUX_TS_REP_PUT_SIZE (188 * 7 * 64)
#ifdef _MT_WITH_TALTS_
#define DEMUX_TS_PUT_SIZE (188 * 512) //It should be good for all TS injection
#else
#define DEMUX_TS_PUT_SIZE (188 * 16) //Debugging
#endif
#define VIDEO_ES_BUF_SIZE 0x10000
#define AUDIO_ES_BUF_SIZE 0x1000
#define DEMUX_INVALID_ID 0xFFFFFFFF
/* 10G bytes or no limits */
#define MAX_RECORD_FILE_SIZE (10 * 1024 * 1024 * 1024ul)

/* Refer to ISO/IEC 13818-1, table 2-29 */
enum streamTypeTabel {
	DVB_STREAM_TYPE_MPEG1_VIDEO = 0x01,
	DVB_STREAM_TYPE_MPEG2_VIDEO = 0x02,
	DVB_STREAM_TYPE_MPEG4_VIDEO = 0x10,
	DVB_STREAM_TYPE_H264_VIDEO  = 0x1B,
	DVB_STREAM_TYPE_HEVC_VIDEO  = 0x24,
	DVB_STREAM_TYPE_AVS_VIDEO   = 0x42,
	DVB_STREAM_TYPE_VC1_VIDEO   = 0xEA,
	DVB_STREAM_TYPE_MPEG1_AUDIO = 0x03,
	DVB_STREAM_TYPE_MPEG2_AUDIO = 0x04,
	DVB_STREAM_TYPE_AAC_AUDIO   = 0x0F,
	DVB_STREAM_TYPE_AAC_AUDIO_1 = 0x11,
};

static TBoolean record_pip_flg = 0;
static mt_u8 mt_dmxSoftInject_index = 3; // index use which PVR record or decypt
static TUnsignedInt32 pkcs7_padding_data_length(TUnsignedInt8 *data, TUnsignedInt32 size, TUnsignedInt32 modulus);
static mt_u8 *vir2phy(const mt_u8 *vir);



/*
 * list of static Media Buffers.
 *
 * NOTE: This list may be modified based on the specific memory map of device project.
 */

/* Each buffer should be 256k aligned for SMP function */
//setenv memargs mmz=pcm,0,6M,4M mmz=av,0,254M,188M $pip_mmz $dfb_mmz mmz=ott0,0,170M,16M mmz=ddr,0,186M,68M
#define MM_AV_MMZ_START			0xFE00000//254M
#define MM_AV_MMZ_SIZE			0xBC00000//188M

#define MM_OTT_START			0xAA00000//170M boot arg ott0 buffer
#define MM_OTT_SIZE				0x1000000// 16M

//#define MM_VIDEO_ES_START		0xB980000//RES
//#define MM_VIDEO_ES_SIZE		0x800000//RES

#define MM_PIP_ZONEMMZ_START    0x1BA00000
#define MM_PIP_ZONEMMZ_SIZE       0x3E00000


//190M 199m

/* Note: OTT and PVR shall not co-exist */
static MBInfo SprMBInfo[] = {
	/* Audio */
	{0, 0, MB_AUD},//It is a default set, please set it again with the correct address you get by API
	/* Video */
	{MM_AV_MMZ_START, MM_AV_MMZ_SIZE, MB_VID},
	/* PVR */
	{0, 0, MB_REC},//It is allocated by Montage-LZ low level driver, please set it again with the correct address you get by API
	/* OTT */
	{MM_OTT_START, MM_OTT_SIZE, MB_OTT},//ott0, for test use specific addres for OTT test
	/* VIDEO ES BUFFER */
};


static void getSprMBInfoByType(MBType t, MBInfo *mb)
{
	int i = 0;

	for (i = 0; i < sizeof(SprMBInfo) / sizeof(SprMBInfo[0]); i++) {
		if (SprMBInfo[i].type == t) {
			mb->size = SprMBInfo[i].size;
			mb->start = SprMBInfo[i].start;
			mb->type = SprMBInfo[i].type;
			break;
		}
	}
}

/*
 * See mt_unf_demux.h
 */
#define MAX_PORT_NUM	(4)

struct chunkInfo {
	TUnsignedInt8 *start;
	TUnsignedInt32 size;
    TUnsignedInt32 sizeInUse;//real size
	TUnsignedInt8 status; //0:Freed, 1:In-Use
};

static void *mmz_malloc(const char *area, ulong size, mt_u32 alignByte);
static mt_s32 mmz_free(mt_void *p_vir);

static mt_handle voutWindow = MT_INVALID_HANDLE;
static mt_handle voutWindow2 = MT_INVALID_HANDLE;

static pthread_rwlock_t rwlock = PTHREAD_MUTEX_INITIALIZER;

static pthread_rwlock_t rwlock2 = PTHREAD_MUTEX_INITIALIZER;


static pthread_rwlock_t callback_rwlock = PTHREAD_RWLOCK_INITIALIZER;

static TBoolean isPlatformInitialized = FALSE;
struct list_head transportSessionList = {
	.prev = &transportSessionList,
	.next = &transportSessionList
};

TEEC_Context g_teec_ctx;

#define RD_LOCK(lock) \
	({ \
		if (pthread_rwlock_wrlock(lock)) { \
			EMSG("apply a read lock failed"); \
			return NV_SPR_ERROR;              \
		} \
	})

#define RD_UNLOCK(lock) \
	({ \
		if (pthread_rwlock_unlock(lock)) { \
			EMSG("release a read lock failed"); \
			return NV_SPR_ERROR;			  \
		} \
	})

#define WR_LOCK(lock) \
	({ \
		if (pthread_rwlock_wrlock(lock)) { \
			EMSG("apply a write lock failed"); \
			return NV_SPR_ERROR;			  \
		} \
	})

#define WR_UNLOCK(lock) \
	({ \
		if (pthread_rwlock_unlock(lock)) { \
			EMSG("release a write lock failed"); \
			return NV_SPR_ERROR;			  \
		} \
	})


#define CHECK_CALL_MT_FUNC(func, ...) \
	({ \
		mt_s32 chk_ret = func(__VA_ARGS__); \
		if (chk_ret != MT_SUCCESS)		  \
			EMSG("call " #func " failed, return error = 0x%x", chk_ret); \
		chk_ret; \
	})

#define CHECK_TSID_VALIDITY(tsid) \
	({ \
		if (tsid == TRANSPORT_SESSION_ID_INVALID) {          \
			EMSG("the transport session id is invalid"); \
			return NV_SPR_ERROR_BAD_PARAM;			         \
		} \
	})

static void addToListTail(struct list_head *new, struct list_head *head)
{
	struct list_head *prev = head->prev;
	struct list_head *next = head;

	prev->next = new;
	new->prev = prev;
	new->next = next;
	next->prev = new;
}

static void delFromList(struct list_head *node)
{
	struct list_head *prev = node->prev;
	struct list_head *next = node->next;

	prev->next = next;
	next->prev = prev;
}


static struct transportSession *getTransportSessionById(TTransportSessionId tsid)
{
	struct transportSession *ts = NULL;
	struct list_head *node;
	size_t offset = (size_t)(&((struct transportSession *)0)->listNode);

	for (node = transportSessionList.next; node != &transportSessionList; node = node->next) {
		ts = (struct transportSession *)((char *)node - offset);
		//printf("getTransportSessionById %d %d tsid = %d \n", ts->tsid, ts->isClosing, tsid );
		if (ts->tsid == tsid)
			return ts;
	}

	return NULL;
}

/*
static TBoolean getOthersSessionOpenedFlg( TTransportSessionId tsid , mt_u8 sessiontype)
{
	struct transportSession *ts = NULL;
	struct list_head *node;
	size_t offset = (size_t)(&((struct transportSession *)0)->listNode);

	for (node = transportSessionList.next; node != &transportSessionList; node = node->next) {
		ts = (struct transportSession *)((char *)node - offset);
		//printf("getOthersSessionOpenedFlg %d %d tsid = %d sessiontype = %d \n", ts->tsid, ts->isClosing, tsid, sessiontype );
		if ( ts->tsid != tsid && ts->isClosing == FALSE && ts->sessionType == TRANSPORT_SESSION_TYPE_RECORD)
		{
			//printf("getOthersSessionOpenedFlg  %d %d ", ts->tsid, ts->isClosing);
			return TRUE;
		}
	}

	return FALSE;
}
*/

static struct transportSession *newTransportSession(TTransportSessionId tsid)
{
	struct transportSession *ts;

	ts = getTransportSessionById(tsid);
	if (ts) {
		EMSG("current transport session id(%d) is existing", tsid);
		return NULL;
	}

	ts = malloc(sizeof(struct transportSession));
	if (!ts) {
		EMSG("malloc transport session struct failed");
		return NULL;
	}
	memset(ts, 0x0, sizeof(struct transportSession));

	ts->tsid = tsid;
	ts->isClosing = FALSE;
	//printf("newTransportSession %d %d \n", ts->tsid, ts->isClosing );
	ts->sessionType = TRANSPORT_SESSION_TYPE_INVALID;
	ts->emi = 0xFFFF; //invalid EMI

	ts->smpEnabled = FALSE;
	ts->smpSetFlag = FALSE;

	ts->playDmxId = DEMUX_INVALID_ID;
	ts->recordDmxId = DEMUX_INVALID_ID;
	ts->recordChannelId = 0;
	ts->avPlayerStreamType = MT_UNF_AVPLAY_STREAM_TYPE_BUTT;

	ts->descramblingCount = 0;
	ts->ts_opc_err_status_count = 0;
	ts->audiodescramblingCount = 0;
	ts->tsBufHandle = MT_INVALID_HANDLE;
	ts->avPlayer = MT_INVALID_HANDLE;
	ts->audioTrack = MT_INVALID_HANDLE;

	ts->doesVidChanOpen = FALSE;
	ts->doesAudChanOpen = FALSE;

	ts->vidCodecType = MT_UNF_VCODEC_TYPE_MPEG2;
	ts->audCodecType = HA_AUDIO_ID_PCM;

	ts->tsBuffer = NULL;
	ts->tsDataSize = 0;
	ts->tsIntBufferVir = NULL;

	ts->vesInjPointer = NULL;
	ts->vesInjSize = 0;
	ts->aesInjPointer = NULL;
	ts->aesInjSize = 0;

	/* add this transport session to the list tail */
	addToListTail(&ts->listNode, &transportSessionList);

	return ts;
}

static void delTransportSession(struct transportSession *ts)
{

	delFromList(&ts->listNode);
       memset(ts, 0x00, sizeof(struct transportSession));

	free(ts);
}

static void clearAllTransportSeesion(void)
{
	struct transportSession *ts = NULL;
	struct list_head *node;
	size_t offset = (size_t)(&((struct transportSession *)0)->listNode);

	for (node = transportSessionList.next; node != &transportSessionList; free(ts)) {
		ts = (struct transportSession *)((char *)node - offset);
		node = node->next;
	}

	transportSessionList.prev = &transportSessionList;
	transportSessionList.next = &transportSessionList;
}

static mt_s32 dmacpy(MT_EDMA_CH_E ch, mt_u8 *dst, const mt_u8 *src, mt_u32 size)
{
	//	printf("%s %d \n", __func__, __LINE__);
	mt_s32 ret = MT_FAILURE;
	mt_unf_dma_request_channel(ch);

	while (MT_EDMA_STATUS_FREE != mt_unf_dma_check(ch)) {
		//2ms
		usleep(2000);
             printf(".");
	}

	ret = mt_unf_dma_memcpy(ch, (phys_addr_t)src, (phys_addr_t)dst, size);

	mt_unf_dma_release_channel(ch);
	if (ret != MT_SUCCESS) {
		printf("dma-%d copy from %p to %p with size(%x) fail  ret = %x \n", ch, src, dst, size, ret);
		return MT_FAILURE;
	}

	//printf("dma-%d copy from %p to %p with size(%x) success\n", ch, src, dst, size);
	return MT_SUCCESS;
}

#define DMX_CHANHANDLE(ChanId)      ((ChanId) | 0x00000100 | (MT_ID_DEMUX << 16))
static TBoolean getDemuxInfo( MT_UNF_DMX_CHAN_TYPE_E enChannelType )
{
	int i =0;
	MT_UNF_DMX_CHAN_ATTR_S pstChAttr;
	for(i=0; i<128; i++)
	{
		int ret =MT_UNF_DMX_GetChannelAttr(DMX_CHANHANDLE(i), &pstChAttr);
		if(ret == MT_SUCCESS)
		{
			TMSG("[demux info] channelID %d bufsize %d channeltype %d  outputmode %d , vtyp %d \n", i, pstChAttr.u32BufSize,  pstChAttr.enChannelType,  pstChAttr.enOutputMode, pstChAttr.vCodecType);
			if( pstChAttr.enChannelType == enChannelType)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
static TBoolean getAvPlayInfo( void )
{
#if 1
	mt_s32 ret;
	MT_UNF_AVPLAY_PLAYERINFO_S info;
	memset((void*)&info, 0, sizeof(info));
	info.index = 0;
	ret = MT_UNF_AVPLAY_ListAllPlayer(&info);
	if(ret == MT_SUCCESS)
	{
		int i = 0;
		for(i=0; i<info.AvplayCount; i++)
		{
			info.index = i;
			ret =MT_UNF_AVPLAY_ListAllPlayer(&info);
			//printf("info.AvplayCount = %d index %d  demuxid %d , streamtype %d , custat %d video enable %d audio enable %d\n",
				//info.AvplayCount,  info.index, info.u32DemuxId, info.enStreamType, info.CurStatus, info.VidEnable, info.AudEnable);
		}
		if( info.AvplayCount )
		{
			return TRUE;
		}
	}
#endif
	return FALSE;
}



static TNvSprStatus openAvPlayer(struct transportSession *ts,
                                 const void *vidChanParams, const void *audChanParams)
{
	mt_s32 ret;
	MT_UNF_AVPLAY_ATTR_S avPlayerAttr;

	memset((void *)&avPlayerAttr, 0, sizeof(avPlayerAttr));

	ret = MT_UNF_AVPLAY_GetDefaultConfig(&avPlayerAttr, ts->avPlayerStreamType);
	if (ret != MT_SUCCESS) {
		EMSG("get AV player attribute failed, return error = 0x%x", ret);
		return NV_SPR_ERROR;
	}

	if (ts->avPlayerStreamType == MT_UNF_AVPLAY_STREAM_TYPE_TS)
		avPlayerAttr.u32DemuxId = ts->playDmxId;

	avPlayerAttr.stStreamAttr.u32VidBufSize = (8*1024*1024);
	if( ts->bUseSubLayer  )
	{
		avPlayerAttr.stStreamAttr.vdec_pip_chan = 1;

		//printf("  vdec_pip_chan = %d \n", avPlayerAttr.stStreamAttr.vdec_pip_chan);
	}
	ret = MT_UNF_AVPLAY_Create(&avPlayerAttr, &ts->avPlayer);
	if (ret != MT_SUCCESS) {
		EMSG("create AV player failed, return error = 0x%x", ret);
		return NV_SPR_ERROR;
	}

	DMSG("tsid = %d ts->avPlayer = %d ", ts->tsid, ts->avPlayer);

	ts->doesVidChanOpen = FALSE;
	ts->doesAudChanOpen = FALSE;

	ret = MT_UNF_AVPLAY_ChnOpen(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, vidChanParams);
	if (ret != MT_SUCCESS) {
		EMSG("open video channel failed, return error = 0x%x", ret);
		goto error;
	}

	ts->doesVidChanOpen = TRUE;

	if( ts->bUseSubLayer == FALSE)
	{

		ret = MT_UNF_AVPLAY_ChnOpen(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, audChanParams);
		if (ret != MT_SUCCESS) {
			EMSG("open audio channel failed, return error = 0x%x", ret);
			goto error;
			//ts->doesAudChanOpen = FALSE;
			//ts->bUseSubLayer == TRUE;
		}
		else
		{
			ts->doesAudChanOpen = TRUE;
			//printf("%s %d openAvPlayer bUseSubLayer = FALSE tsid = %d\n",__FUNCTION__,__LINE__,  ts->tsid);
		}

	}
	else
	{
		//printf("%s %d openAvPlayer bUseSubLayer = TRUE tsid = %d\n",__FUNCTION__,__LINE__,  ts->tsid);
	}

	return NV_SPR_NO_ERROR;

error:
	if (ts->doesVidChanOpen)
		CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_ChnClose, ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

	CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_Destroy, ts->avPlayer);

	ts->avPlayer = MT_INVALID_HANDLE;
	ts->doesVidChanOpen = FALSE;
	ts->doesAudChanOpen = FALSE;

	return NV_SPR_ERROR;
}

static void closeAvPlayer(struct transportSession *ts)
{
	if (ts->doesVidChanOpen)
		CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_ChnClose, ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

	if (ts->doesAudChanOpen)
		CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_ChnClose, ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

	CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_Destroy, ts->avPlayer);

	ts->avPlayer = MT_INVALID_HANDLE;
	ts->doesVidChanOpen = FALSE;
	ts->doesAudChanOpen = FALSE;
}


#ifdef CREATE_WD_LATER

static TNvSprStatus openWindow(struct transportSession *ts)
{
	mt_s32 ret;

       static mt_handle windowHandle = 0;
	MT_UNF_WINDOW_ATTR_S pWinAttr;
	//printf("%s %d openAvPlayer bUseSubLayer = %d \n",__FUNCTION__,__LINE__,  ts->bUseSubLayer );

#if 1
	if( ts->bUseSubLayer == FALSE )
	{
		if( voutWindow == MT_INVALID_HANDLE )
		{
			 memset(&pWinAttr, 0, sizeof(MT_UNF_WINDOW_ATTR_S));

		        pWinAttr.bUseSubLayer = 0;
		        pWinAttr.bSetVideoBot = 1;
		        pWinAttr.enDisp = MT_UNF_DISPLAY1;
		        pWinAttr.bVirtual = MT_FALSE;

		        pWinAttr.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_IGNORE;
		        pWinAttr.stWinAspectAttr.bUserDefAspectRatio = MT_FALSE;
		        pWinAttr.stWinAspectAttr.u32UserAspectWidth  = 0;
		        pWinAttr.stWinAspectAttr.u32UserAspectHeight = 0;
		        pWinAttr.bUseCropRect = MT_FALSE;

		        pWinAttr.stInputRect.s32X = 0;
		        pWinAttr.stInputRect.s32Y = 0;
		        pWinAttr.stInputRect.s32Width = 0;
		        pWinAttr.stInputRect.s32Height = 0;

		        ret = MT_UNF_VO_CreateWindow(&pWinAttr, &voutWindow);

			 MT_UNF_VO_GetWindowAttr(voutWindow, &pWinAttr);
		        pWinAttr.enDisp = 1;
		        pWinAttr.stInputRect.s32X = 0;
		        pWinAttr.stInputRect.s32Y = 0;
		        pWinAttr.stInputRect.s32Width = 0;
		        pWinAttr.stInputRect.s32Height = 0;
		        pWinAttr.stOutputRect.s32X = 0;
		        pWinAttr.stOutputRect.s32Y = 0;
		        pWinAttr.stOutputRect.s32Width = 0;
		        pWinAttr.stOutputRect.s32Height = 0;

		        MT_UNF_VO_SetWindowAttr(voutWindow, &pWinAttr);
			 //printf("%s %d \n",__FUNCTION__,__LINE__);

			ret = CHECK_CALL_MT_FUNC(MT_UNF_DISP_SetDiOnOff, MT_FALSE);
		}
	}
	else
	{
		if ( voutWindow2 == MT_INVALID_HANDLE)
		{
			 memset(&pWinAttr, 0, sizeof(MT_UNF_WINDOW_ATTR_S));
		        pWinAttr.bUseSubLayer = 1;
		        pWinAttr.bSetVideoBot = 1;
		        pWinAttr.enDisp = MT_UNF_DISPLAY1;
		        pWinAttr.bVirtual = MT_FALSE;
		        pWinAttr.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_IGNORE;
		        pWinAttr.stWinAspectAttr.bUserDefAspectRatio = MT_FALSE;
		        pWinAttr.stWinAspectAttr.u32UserAspectWidth  = 0;
		        pWinAttr.stWinAspectAttr.u32UserAspectHeight = 0;
		        pWinAttr.bUseCropRect = MT_FALSE;
		        pWinAttr.stInputRect.s32X = 0;
		        pWinAttr.stInputRect.s32Y = 0;
		        pWinAttr.stInputRect.s32Width = 0;
		        pWinAttr.stInputRect.s32Height = 0;
		        ret = MT_UNF_VO_CreateWindow(&pWinAttr, &voutWindow2);

			 MT_UNF_VO_GetWindowAttr(voutWindow2, &pWinAttr);
	               pWinAttr.enDisp = 1;
	               pWinAttr.stInputRect.s32X = 0;
	               pWinAttr.stInputRect.s32Y = 0;
	               pWinAttr.stInputRect.s32Width = 0;
	               pWinAttr.stInputRect.s32Height = 0;
	        	 pWinAttr.stOutputRect.s32X = 0;
	               pWinAttr.stOutputRect.s32Y = 0;
	               pWinAttr.stOutputRect.s32Width = 1920; //for HDMI 1080i50
	               pWinAttr.stOutputRect.s32Height = 900;
	               MT_UNF_VO_SetWindowAttr(voutWindow2, &pWinAttr);
		}
	}
#endif

	if( ts->bUseSubLayer  == FALSE )
	{
		windowHandle = voutWindow;
	}
	else
	{
		windowHandle = voutWindow2;
	}

	ret = MT_UNF_VO_AttachWindow(windowHandle, ts->avPlayer);
	if (ret != MT_SUCCESS) {
		EMSG("attach display window failed, return error = 0x%x", ret);
		return NV_SPR_ERROR;
	}

	ret = MT_UNF_VO_SetWindowEnable(windowHandle, MT_TRUE);
	if (ret != MT_SUCCESS) {
		EMSG("enable display window to the AV player failed, return error = 0x%x", ret);
		goto detach_window;
	}

#if 1
	ret = MT_UNF_VO_ResetWindow(windowHandle, MT_UNF_WINDOW_FREEZE_MODE_BLACK);
	if (ret != MT_SUCCESS) {
		EMSG("reset window failed, return error = 0x%x", ret);
		goto detach_window;
	}
#endif

	ret = MT_UNF_DISP_VidLayerShow(MT_TRUE);
	if (ret != MT_SUCCESS) {
		EMSG("show layer failed, return error = 0x%x", ret);
		goto disable_window;
	}

	return NV_SPR_NO_ERROR;

disable_window:
	CHECK_CALL_MT_FUNC(MT_UNF_VO_SetWindowEnable, windowHandle, MT_FALSE);
detach_window:
	CHECK_CALL_MT_FUNC(MT_UNF_VO_DetachWindow, windowHandle, ts->avPlayer);

	return NV_SPR_ERROR;
}

static void closeWindow(struct transportSession *ts)
{
       static mt_handle windowHandle = 0;

	//printf("%s %d closeWindow bUseSubLayer = %d \n",__FUNCTION__,__LINE__,  ts->bUseSubLayer );

	if( ts->bUseSubLayer  == TRUE)
	{
		windowHandle = voutWindow2;
		//printf("closeWindow windowHandle = voutWindow2 windowHandle = %ld  bUseSubLayer = %d\n", windowHandle, ts->bUseSubLayer);
	}
	else
	{
		windowHandle =  voutWindow;
	}

//	CHECK_CALL_MT_FUNC(MT_UNF_VO_ResetWindow, windowHandle, MT_UNF_WINDOW_FREEZE_MODE_BLACK);
	CHECK_CALL_MT_FUNC(MT_UNF_VO_SetWindowEnable, windowHandle, MT_FALSE);
	CHECK_CALL_MT_FUNC(MT_UNF_VO_DetachWindow, windowHandle, ts->avPlayer);
	#if 1
	CHECK_CALL_MT_FUNC(MT_UNF_VO_DestroyWindow, windowHandle);

	if( ts->bUseSubLayer  == TRUE)
	{
		voutWindow2 = MT_INVALID_HANDLE;
	}
	else
	{
		voutWindow = MT_INVALID_HANDLE;
	}
	#endif
}
#else

static TNvSprStatus openWindow(struct transportSession *ts)
{
	mt_s32 ret;

       static mt_handle windowHandle = 0;

	windowHandle =  voutWindow;//voutWindow;
	//printf("%s %d tsid = %d windowHandle = %d \n",__FUNCTION__,__LINE__,  ts->tsid, windowHandle);

	if( ts->bUseSubLayer == TRUE )
	{
		windowHandle = voutWindow2;//voutWindow2;
		//printf("openWindow windowHandle = voutWindow2 windowHandle = %d ts->avPlayer = %d\n", windowHandle, ts->avPlayer);
	}

	ret = MT_UNF_VO_AttachWindow(windowHandle, ts->avPlayer);
	if (ret != MT_SUCCESS) {
		EMSG("attach display window failed, return error = 0x%x", ret);
		return NV_SPR_ERROR;
	}

	ret = MT_UNF_VO_SetWindowEnable(windowHandle, MT_TRUE);
	if (ret != MT_SUCCESS) {
		EMSG("enable display window to the AV player failed, return error = 0x%x", ret);
		goto detach_window;
	}

#if 0
	ret = MT_UNF_VO_ResetWindow(windowHandle, MT_UNF_WINDOW_FREEZE_MODE_BLACK);
	if (ret != MT_SUCCESS) {
		EMSG("reset window failed, return error = 0x%x", ret);
		goto detach_window;
	}

	ret = MT_UNF_DISP_VidLayerShow(MT_TRUE);
	if (ret != MT_SUCCESS) {
		EMSG("show layer failed, return error = 0x%x", ret);
		goto disable_window;
	}
#endif

	return NV_SPR_NO_ERROR;

disable_window:
	CHECK_CALL_MT_FUNC(MT_UNF_VO_SetWindowEnable, windowHandle, MT_FALSE);
detach_window:
	CHECK_CALL_MT_FUNC(MT_UNF_VO_DetachWindow, windowHandle, ts->avPlayer);

	return NV_SPR_ERROR;
}

static void closeWindow(struct transportSession *ts)
{
       static mt_handle windowHandle = 0;

	windowHandle =  voutWindow;

	if( ts->bUseSubLayer  == TRUE)
	{
		windowHandle = voutWindow2;
		//printf("closeWindow windowHandle = voutWindow2 windowHandle = %d  bUseSubLayer = %d\n", windowHandle, ts->bUseSubLayer);
	}

//	CHECK_CALL_MT_FUNC(MT_UNF_VO_ResetWindow, windowHandle, MT_UNF_WINDOW_FREEZE_MODE_BLACK);
	CHECK_CALL_MT_FUNC(MT_UNF_VO_SetWindowEnable, windowHandle, MT_FALSE);
	CHECK_CALL_MT_FUNC(MT_UNF_VO_DetachWindow, windowHandle, ts->avPlayer);
}
#endif
static TNvSprStatus openAudio(struct transportSession *ts, HA_CODEC_ID_E audCodec)
{
	mt_s32 ret;
	MT_UNF_AUDIOTRACK_ATTR_S *audTrkAttr;
	audTrkAttr = (MT_UNF_AUDIOTRACK_ATTR_S *)malloc(2 * sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
	if (NULL == audTrkAttr) {
		EMSG("Aud buffer alloc fail!");
		return NV_SPR_ERROR;
	}
	memset(audTrkAttr, 0, 2 * sizeof(MT_UNF_AUDIOTRACK_ATTR_S));

	ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, audTrkAttr);
	if (ret != MT_SUCCESS) {
		EMSG("get audio track attribute failed, return error = 0x%x", ret);
		goto free_aud_attr;
	}

	if (audCodec == HA_AUDIO_ID_AC3PASSTHROUGH ||
	    audCodec == HA_AUDIO_ID_EAC3PASSTHROUGH ||
	    audCodec == HA_AUDIO_ID_DTSPASSTHROUGH)
		audTrkAttr->b_spdif_mod = MT_TRUE;

	ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, audTrkAttr, &ts->audioTrack);
	if (ret != MT_SUCCESS) {
		EMSG("create audio track failed, return error = 0x%x", ret);
		goto free_aud_attr;
	}

	ret = MT_UNF_SND_Attach(ts->audioTrack, ts->avPlayer);
	if (ret != MT_SUCCESS) {
		EMSG("attach audio track to the AV player failed, return error = 0x%x", ret);
		goto destroy_audio_track;
	}

	if (audCodec == HA_AUDIO_ID_TRUEHD) {
		ret = MTADP_AVPlay_SetAdecAttr(ts->avPlayer, audCodec, HD_DEC_MODE_THRU, 0);

	} else {
		ret = MTADP_AVPlay_SetAdecAttr(ts->avPlayer, audCodec, HD_DEC_MODE_RAWPCM, 0);
	}
	if (ret != MT_SUCCESS) {
		EMSG("set audio decoder attribute failed, return error = 0x%x", ret);
		goto detach_audio_track;
	}

	MT_UNF_AVPLAY_Enable_AudioHEAAC(ts->avPlayer);

	return NV_SPR_NO_ERROR;
detach_audio_track:
	CHECK_CALL_MT_FUNC(MT_UNF_SND_Detach, ts->audioTrack, ts->avPlayer);
destroy_audio_track:
	CHECK_CALL_MT_FUNC(MT_UNF_SND_DestroyTrack, ts->audioTrack);
	ts->audioTrack = MT_INVALID_HANDLE;
free_aud_attr:
	free(audTrkAttr);
	audTrkAttr = NULL;
	return NV_SPR_ERROR;
}

static void closeAudio(struct transportSession *ts)
{
	CHECK_CALL_MT_FUNC(MT_UNF_SND_Detach, ts->audioTrack, ts->avPlayer);
	CHECK_CALL_MT_FUNC(MT_UNF_SND_DestroyTrack, ts->audioTrack);
	ts->audioTrack = MT_INVALID_HANDLE;
}

static TNvSprStatus startAvPlayer(struct transportSession *ts,
                                  MT_UNF_VCODEC_TYPE_E vidCodec, mt_u32 vidPid,
                                  HA_CODEC_ID_E audCodec, mt_u32 audPid,
                                  mt_u32 pcrPid, MT_UNF_SYNC_REF_E syncRef)
{
	mt_s32 ret;
	TNvSprStatus status = NV_SPR_NO_ERROR;
	MT_UNF_VCODEC_ATTR_S vidCodecAttr;
	MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S dmxAvSyncAttr;
	MT_UNF_SYNC_ATTR_S syncAttr;
    MT_UNF_AVPLAY_STOP_OPT_S stopOpt;

	/* Setup audio */
	//printf("%s %d \n",__FUNCTION__,__LINE__);

	if (ts->doesAudChanOpen) {
		status = openAudio(ts, audCodec);
		if (status != NV_SPR_NO_ERROR) {
			EMSG("open audio player failed");
			return status;
		}

		ret = MT_UNF_AVPLAY_SetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &audPid);
		if (ret != MT_SUCCESS) {
			EMSG("set audio pid failed, return error = 0x%x", ret);
			goto close_audio;
		}
	}

	/* Setup video */
	memset((void *)&vidCodecAttr, 0, sizeof(vidCodecAttr));

	ret = MT_UNF_AVPLAY_GetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vidCodecAttr);
	if (ret != MT_SUCCESS) {
		EMSG("get video decoder attribute failed, return error = 0x%x", ret);
		goto close_audio;
	}

    vidCodecAttr.enType = vidCodec;
    vidCodecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
    vidCodecAttr.u32ErrCover = 100;
	//vidCodecAttr.u32ErrCover = 50;
    vidCodecAttr.u32ErrCover = 70;
    vidCodecAttr.u32Priority = 1;
	vidCodecAttr.u32UseDescInfoFlag = 1;
	//vidCodecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_STABLE;
    vidCodecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_FAST;

    ret = MT_UNF_AVPLAY_SetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vidCodecAttr);
	if (ret != MT_SUCCESS) {
		EMSG("set video decoder attribute failed, return error = 0x%x", ret);
		goto close_audio;
	}

	ret = MT_UNF_AVPLAY_SetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &vidPid);
	if (ret != MT_SUCCESS) {
		EMSG("set video pid failed, return error = 0x%x", ret);
		goto close_audio;
	}

	ret = MT_UNF_AVPLAY_SetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &pcrPid);
	if (ret != MT_SUCCESS) {
		EMSG("set pcr pid failed, return error = 0x%x", ret);
		goto close_audio;
	}

	if (ts->avPlayerStreamType == MT_UNF_AVPLAY_STREAM_TYPE_TS) {
		/* Set AV synchronization */
		memset((void *)&dmxAvSyncAttr, 0, sizeof(dmxAvSyncAttr));

		dmxAvSyncAttr.VdecType = vidCodec;
		dmxAvSyncAttr.AdecType = audCodec;
		//printf("%s %d \n",__FUNCTION__,__LINE__);

		if (syncRef > MT_UNF_SYNC_REF_NONE && syncRef < MT_UNF_AVPLAY_SYNC_REF_BUTT)
			dmxAvSyncAttr.AvsyncFlage = 1;
		/*if (!ts->doesAudChanOpen)
		{
			dmxAvSyncAttr.AvsyncFlage = MT_UNF_SYNC_REF_NONE;
		}*/

		ret = MT_UNF_AVPLAY_SetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, &dmxAvSyncAttr);
		if (ret != MT_SUCCESS) {
			EMSG("set demux AV synchronization failed, return error = 0x%x", ret);
			goto close_audio;
		}
	}

	memset((void *)&syncAttr, 0, sizeof(syncAttr));

	ret = MT_UNF_AVPLAY_GetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_SYNC, &syncAttr);
	if (ret != MT_SUCCESS) {
		EMSG("get AV synchronization attribute failed, return error = 0x%x", ret);
		goto close_audio;
	}

	syncAttr.enSyncRef = ts->doesAudChanOpen ? syncRef : MT_UNF_SYNC_REF_NONE;

	if ((ts->avPlayerStreamType == MT_UNF_AVPLAY_STREAM_TYPE_TS) &&
	    (vidCodecAttr.enUnBlank == MT_UNF_VCODEC_UNBLANK_FAST))
	    syncAttr.bQuickOutput = MT_TRUE;

    ret = MT_UNF_AVPLAY_SetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_SYNC, &syncAttr);
	if (ret != MT_SUCCESS) {
		EMSG("set AV synchronization attribute failed, return error = 0x%x", ret);
		goto close_audio;
	}


	/* clear buffers before starting play */
//    ret = MT_UNF_AVPLAY_Reset(ts->avPlayer, MT_NULL);
	if (ret != MT_SUCCESS) {
		EMSG("reset avplay failed, return error = 0x%x", ret);
		goto close_audio;
	}


	/* Start play */
    ret = MT_UNF_AVPLAY_Start(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
	if (ret != MT_SUCCESS) {
		EMSG("start video failed, return error = 0x%x", ret);
		goto close_audio;
	}
	//printf("ch start success!");

	if (ts->doesAudChanOpen) {

		ret = MT_UNF_AVPLAY_Start(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
		if (ret != MT_SUCCESS) {
			EMSG("start audio failed, return error = 0x%x", ret);
			goto stop_video;
		}
	}


	return NV_SPR_NO_ERROR;

stop_video:
	stopOpt.u32TimeoutMs = 0;
	stopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
	CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_Stop, ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &stopOpt);
close_audio:
	if (ts->doesAudChanOpen)
		closeAudio(ts);

	return NV_SPR_ERROR;
}

static void stopAvPlayer(struct transportSession *ts)
{
	MT_UNF_AVPLAY_MEDIA_CHAN_E mediaChannel = 0;
    MT_UNF_AVPLAY_STOP_OPT_S stopOpt;

	stopOpt.u32TimeoutMs = 0;
/*
#ifdef STOP_WITH_STILL_FRAME
	stopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
#else
	stopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
#endif
*/
       if (ts->bUseSubLayer) {
            stopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
       } else {
            stopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
       }
	if (ts->doesVidChanOpen)
		mediaChannel |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;

	if (ts->doesAudChanOpen)
		mediaChannel |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;

	//CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_Flush, ts->avPlayer);
	CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_Stop, ts->avPlayer, mediaChannel, &stopOpt);

	if (ts->doesAudChanOpen)
		closeAudio(ts);

}


int opc_fd = -1;


static void MT_OPCDeinit(int opc_sock)
{
	if(opc_sock >= 0)
		close(opc_sock);
}

static int MT_OPCInit(void)
{
	struct sockaddr_nl    local    = {0,};
	int                   opc_sock = -1;

	printf("%s: in\n", __func__);

	opc_sock = socket(PF_NETLINK, SOCK_RAW, OPC_NETLINK_ID);
	if (opc_sock < 0) {
		printf("%s: Failed to open netlink socket, errno:%s\n", __func__, strerror(errno));
		if(opc_sock >= 0)
			close(opc_sock);
		return -1;
	}

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = 1;
	if (bind(opc_sock, (struct sockaddr *) &local, sizeof(local)) < 0) {
		printf("%s: Failed to bind netlink socket, errno:%s\n", __func__, strerror(errno));
		if(opc_sock >= 0)
			close(opc_sock);
		return -1;
	}

	return opc_sock;
}



void MT_NagraOPCDeinit(void)
{
	MT_OPCDeinit(opc_fd);

}
/*
#define MT_SEC_CKL_REG  (0xbf50c000)
void mt_cert_init(void)
{
	int ret = 0;

	//ret = mt_sys_init();
	ret |= mt_unf_misc_init();
	ret |= mt_unf_cipher_init();
	ret |= mt_unf_misc_module_set(HAL_KT, 1);
	ret |= mt_unf_misc_module_set(HAL_CRYPTO, 1);
	ret |= mt_unf_misc_module_set(HAL_CRYPTO_DES, 1);
	ret |= mt_unf_misc_module_set(HAL_CRYPTO_TDES, 1);
	ret |= mt_unf_misc_module_set(HAL_CRYPTO_AES, 1);
	ret |= mt_unf_misc_module_set(HAL_CRYPTO_SHA, 1);
	ret |= mt_unf_misc_module_set(HAL_CRYPTO_RSA, 1);
	ret |= mt_unf_misc_module_set(HAL_KL_CW, 1);
	ret |= mt_unf_misc_module_set(HAL_SECHD0, 1); // enable cert clock!
	//printf("mt_cert_init ret 0x%x \n", ret);
	mt_u32 data = 0;
	mt_sys_read_register(MT_SEC_CKL_REG, &data);
	//printf("value 0x%x \n", data);
	data |= (1 << 3); //enable cert clock!
	mt_sys_write_register(MT_SEC_CKL_REG, data);
	mt_sys_read_register(MT_SEC_CKL_REG, &data);
	//printf("value == 0x%x \n", data);

}
*/
#if 0
static mt_s8 * MT_DispFmt2Str(MT_UNF_ENC_FMT_E format)
{
    switch(format)
    {
        case MT_UNF_ENC_FMT_1080P_60:
            return "FMT_1080P_60";
        case MT_UNF_ENC_FMT_1080P_50:
            return "FMT_1080P_50";
        case MT_UNF_ENC_FMT_1080P_30:
            return "FMT_1080P_30";
        case MT_UNF_ENC_FMT_1080P_25:
            return "FMT_1080P_25";
        case MT_UNF_ENC_FMT_1080P_24:
            return "FMT_1080p_24";
        case MT_UNF_ENC_FMT_1080i_60:
            return "FMT_1080i_60";
        case MT_UNF_ENC_FMT_1080i_50:
            return "FMT_1080i_50";
        case MT_UNF_ENC_FMT_720P_60:
            return "FMT_720P_60";
        case MT_UNF_ENC_FMT_720P_50:
            return "FMT_720P_50";
        case MT_UNF_ENC_FMT_576P_50:
            return "FMT_576P_50";
        case MT_UNF_ENC_FMT_480P_60:
            return "FMT_480P_60";
        case MT_UNF_ENC_FMT_PAL:
            return "FMT_PAL(FMT_576i_50)";
        case MT_UNF_ENC_FMT_NTSC:
            return "FMT_NTSC(FMT_480i_60)";
        case MT_UNF_ENC_FMT_PAL_N:
            return "FMT_576i_50";
        case MT_UNF_ENC_FMT_NTSC_PAL_M:
            return "FMT_480i_60";
#ifdef CONFIG_MT_CHIP_SYMPHONY6
        case MT_UNF_ENC_FMT_3840X2160_24:
            return "FMT_3840X2160_24";
        case MT_UNF_ENC_FMT_3840X2160_25:
            return "FMT_3840X2160_25";
        case MT_UNF_ENC_FMT_3840X2160_30:
            return "FMT_3840X2160_30";
        case MT_UNF_ENC_FMT_3840X2160_50:
            return "FMT_3840X2160_50";
        case MT_UNF_ENC_FMT_3840X2160_60:
            return "FMT_3840X2160_60";
        case MT_UNF_ENC_FMT_4096X2160_24:
            return "FMT_4096X2160_24";
        case MT_UNF_ENC_FMT_4096X2160_25:
            return "FMT_4096X2160_25";
        case MT_UNF_ENC_FMT_4096X2160_30:
            return "FMT_4096X2160_30";
        case MT_UNF_ENC_FMT_4096X2160_50:
            return "FMT_4096X2160_50";
        case MT_UNF_ENC_FMT_4096X2160_60:
            return "FMT_4096X2160_60";
#endif

        default:
            return "No Suport!";

    }
}
#endif
#if 0
void mtTestCheckDownscaler_alltime(uint64_t opc_value)
{
    MT_UNF_ENC_FMT_E encFormat = testFormat;
    uint64_t res_vilation = opc_value;

    res_vilation = (res_vilation >> 8);
    if (((res_vilation & 0xF) == 0xF) || (res_vilation == 0))
    {
        return;
    }
    TMSG("res_vilation = 0x%lx \n", res_vilation);
    //4K, 2K, HD, SD
    if ((res_vilation & 0x1) == 0)
    {
        encFormat = testFormat;
    } else if ((res_vilation & 0x2) == 0)
    {
        encFormat = MT_UNF_ENC_FMT_1080i_50;
    } else if ((res_vilation & 0x4) == 0)
    {
        encFormat = MT_UNF_ENC_FMT_720P_60;
    } else if ((res_vilation & 0x8) == 0)
    {
        encFormat = MT_UNF_ENC_FMT_576P_50;
    }
    //TMSG("encFormat = 0x%x \n", encFormat);
    mtTestUpdateDisplayResolution(encFormat);
}

#else
void mtTestCheckDownscaler_alltime2()
{
    MT_UNF_ENC_FMT_E encFormat = testFormat;
    MTSecOpREEFlag opcinfor = {0x0,};

    if (mtSecGetOpREEFlag(&opcinfor) && opcinfor.isViolation)
    {
        if ((opcinfor.violate4K && opcinfor.violate2K && opcinfor.violateHD && opcinfor.violateSD))
        {
            return;
        }
        TMSG("%d, %d, %d, %d, %d, %d \n", opcinfor.violate4K, opcinfor.violate2K, opcinfor.violateHD, opcinfor.violateSD, opcinfor.violateHDCP1, opcinfor.violateHDCP2);
        if (opcinfor.violate4K == 0)
        {
            encFormat = testFormat;
        } else if (opcinfor.violate2K == 0)
        {
            encFormat = MT_UNF_ENC_FMT_1080i_50;
        } else if (opcinfor.violateHD == 0)
        {
            encFormat = MT_UNF_ENC_FMT_720P_60;
        } else if (opcinfor.violateSD == 0)
        {
            encFormat = MT_UNF_ENC_FMT_576P_50;
        }
        mtTestUpdateDisplayResolution(encFormat);
    }

}

#endif
static struct transportSession *pCurMain_ts = NULL;
void *thread_function_check_OPC(void *unused) {

        mt_u32 data = 0, ret = 0;
        uint64_t opc_status = 0;
        //static uint64_t star_freez = 0, duration = 0;
        MT_UNF_ENC_FMT_E encFormat = testFormat;
        //MT_UNF_AVPLAY_STOP_OPT_S stopOpt;
        //MT_UNF_STREAM_BUF_S dmxBuf = {0};

        while(1)
        {
#if 0
            //duration = get_sys_time_ms();
            /*
            if (star_freez && (duration > star_freez + 10)) {


                //(void)MT_UNF_AVPLAY_Resume(pCurMain_ts->avPlayer, NULL);

                MT_UNF_AVPLAY_Start(pCurMain_ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, NULL);
                printf(" resume duration = %d \n", duration - star_freez);
                star_freez = 0;
            }
            */
            mt_sys_read_register(0xbf314400, &data);
            if (data & (0x2000)) {
                //printf("freeze test: 0x%x \n", test_tsbuffersize);
                //data = data & (~0x1000);
                //mt_sys_write_register(0xbf314400, data);


                data = data & (~0x2000);

            mt_sys_write_register(0xbf314400, data);


                if (pCurMain_ts) {

                    stopOpt.u32TimeoutMs = 0;
                    stopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;


/*
                    (void)MT_UNF_AVPLAY_Freeze(pCurMain_ts->avPlayer, NULL);
                    (void)MT_UNF_AVPLAY_Flush(pCurMain_ts->avPlayer);
                    (void)MT_UNF_AVPLAY_Resume(pCurMain_ts->avPlayer, NULL);
*/


                     //MT_USLEEP(2000000);

#if 0
                    _lockTsBuffer();
ret = MT_UNF_DMX_GetTSBuffer(pCurMain_ts->tsBufHandle, test_tsbuffersize, &dmxBuf, 10);
dmxBuf.u32PhyData = vir2phy(dmxBuf.pu8Data);
_unlockTsBuffer();
if (ret != MT_SUCCESS) {
    printf("getdmx failed, return error = 0x%x", ret);
}


//memcpy((void *)(dmxBuf.pu8Data), (const void *)(test_tsbuffer), test_tsbuffersize);
//dma4 is for General RCID 0/2/3 WCID=RCID
 dmacpy(MT_EDMA_CH_4, (mt_u8 *)(dmxBuf.u32PhyData), test_phy_tsbuffer, test_tsbuffersize);

star_freez = get_sys_time_ms();
//MT_UNF_AVPLAY_Stop(pCurMain_ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &stopOpt);
//MT_UNF_AVPLAY_Start(pCurMain_ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, NULL);

star_freez = get_sys_time_ms() - star_freez;

_lockTsBuffer();
ret = MT_UNF_DMX_PutTSBuffer(pCurMain_ts->tsBufHandle, test_tsbuffersize);
_unlockTsBuffer();
if (ret != MT_SUCCESS) {
    printf("inject failed, return error = 0x%x", ret);
}
#endif

        MT_UNF_AVPLAY_Stop(pCurMain_ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &stopOpt);
        //MT_UNF_AVPLAY_Start(pCurMain_ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, NULL);
/*
memset(grant_testaddress, 0x0, 0x80000);
printf("--0--\n");
virl_addr = mt_mem_map(0x1f800000, 0x400000);
printf("--1-virl_addr = 0x%x \n", virl_addr);
memcpy(grant_testaddress, virl_addr, 0x400000);
printf("--2--\n");
*/
                    //dma2 for teset
                    //dmacpy(MT_EDMA_CH_2, grant_testphy, 0x1F800000, 0x400000);
                    //printf("grant_testphy 0x%x, 0x%lx, 0x%x \n", grant_testphy, virl_addr, *(int*)grant_testaddress);

                    //(void)MT_UNF_AVPLAY_Flush(pCurMain_ts->avPlayer);

                    printf("star_freez== %d \n", star_freez);
                    data = data & (~0x2000);
                    mt_sys_write_register(0xbf314400, data);
                }

            }
            MT_USLEEP(1500000);
#endif

            #if 0
            mt_sys_read_register(0xbf448090, &data);
            opc_status = data;
            mt_sys_read_register(0xbf448094, &data);
            opc_status = opc_status | ((uint64_t)data << 32);
            //adjust display output format
            mtTestCheckDownscaler_alltime(opc_status);
            //MT_USLEEP(1500000);
            MT_USLEEP(1000000);


            //for test display output resolution
            mt_sys_read_register(0xbf314400, &data);
            if (data & 0x80000000) { //set disbply output format
                TMSG("data 0xbf314400 = 0x%x \n", data);
                encFormat = data  & 0xFF; //UHD output
                TMSG("test change resolution encFormat = 0x%x \n", encFormat);
                mtTestUpdateDisplayResolution(encFormat);
                data &= (~0x80000000);
                mt_sys_write_register(0xbf314400, data);
            }
            #endif

            mtTestCheckDownscaler_alltime2();
            MT_USLEEP(1000000);

        }

}

extern void *MTSampleCommadTask(void *pParam);

#include "mt_unf_hdcp.h"
void test_dump_data(mt_u8 *str , mt_u8 *data, mt_u32 len)
{
    int i;
    printf("%s addr=%p, len=0x%x:\n", str, data, len);
    for (i = 0; i < len; i++) {
        printf("%02x ", data[i]);
        if (((i + 1) % 16) == 0)
            printf("\n");
    }
    printf("\n");
}

static int hdcp_key_load_test(void)
{
	unsigned char in_clear_key[304] = {
		0xff,0xbc,0x4a,0x73,0x17,0x92,0xff,0xff,
		0xff,0xe4,0xd2,0x38,0x29,0x7e,0x05,0xad,
		0x56,0xda,0x75,0xda,0x61,0xdd,0x0b,0x7c,
		0x75,0x89,0x29,0x4c,0x75,0xca,0xbe,0x2d,
		0x39,0xca,0x08,0x71,0x8f,0x67,0x70,0x21,
		0x6b,0xed,0xb2,0xc6,0x41,0xc9,0x11,0xd5,
		0x3f,0xb1,0xae,0x7d,0xb7,0xa0,0x32,0x8e,
		0x38,0xdd,0x5a,0x93,0xca,0xbf,0xf4,0x3b,
		0x19,0xdb,0xdd,0x3b,0x54,0x24,0xc1,0x5d,
		0xd3,0xa4,0xbc,0x7e,0x79,0x2b,0x42,0x7d,
		0xdf,0xe3,0xad,0x68,0xf4,0xe5,0x83,0xb9,
		0x91,0xb9,0x5d,0x3d,0x47,0x38,0xc9,0xd9,
		0x65,0x08,0x42,0x96,0x99,0xfe,0x3c,0x48,
		0xd0,0x43,0xdb,0x7d,0x68,0x82,0xc2,0xce,
		0xf1,0x34,0x8f,0xfb,0x4d,0xff,0xee,0x85,
		0xed,0x1f,0x61,0x3d,0x5e,0xe4,0xbe,0xc0,
		0xcd,0x69,0x1a,0x34,0xbe,0x15,0xb1,0x85,
		0x06,0x68,0x6f,0xa6,0x34,0xe8,0x11,0xa0,
		0xf6,0xcb,0x9d,0xc2,0xee,0x8b,0x70,0xb2,
		0x62,0x00,0xdc,0x82,0xdb,0x96,0xf6,0x23,
		0x3a,0x58,0x48,0xc9,0xea,0x21,0x63,0x17,
		0xe3,0x13,0x61,0xbe,0xf8,0x9d,0xd3,0x79,
		0xca,0x03,0x7e,0x59,0x09,0x3e,0xc3,0x6e,
		0x6e,0xce,0xe5,0x98,0x7c,0x18,0xfa,0x9a,
		0x82,0x06,0x95,0xd6,0xc7,0xd1,0xb9,0x59,
		0x97,0x44,0xfb,0x8c,0x64,0xed,0xb6,0x8f,
		0x0a,0xa2,0x71,0x2d,0x31,0x1a,0xab,0x8f,
		0x29,0xfe,0x5f,0x84,0x03,0x96,0x3f,0x10,
		0xc7,0xf1,0x39,0x1a,0x31,0x5f,0x06,0x2f,
		0x51,0x91,0x82,0x8a,0x99,0x1c,0x7c,0x09,
		0x20,0xf7,0x3e,0xc0,0x25,0x59,0xf3,0xdf,
		0xaa,0xb4,0x3a,0x4a,0x6e,0x19,0xa2,0xcc,
		0x4b,0x36,0x90,0x64,0x9c,0x03,0x03,0x10,
		0x40,0x34,0xb0,0x49,0xe0,0x1b,0x20,0x2b,
		0xb5,0xba,0x16,0x94,0x38,0xf7,0xe8,0xac,
		0x41,0xf9,0x0f,0xf3,0x21,0x07,0x01,0x3b,
		0x77,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};


	unsigned char out_encrypted_key[304] = {
        0xbd, 0xbc, 0xd1, 0xa6, 0x86, 0x14, 0x08, 0x44, 0x48, 0x85, 0x9a, 0x10, 0x0e, 0x66, 0x5e, 0x41,
        0x55, 0x34, 0x3d, 0xff, 0x60, 0xea, 0x54, 0x11, 0xda, 0x84, 0xd0, 0x3c, 0xd5, 0x47, 0x19, 0x2a,
        0x4d, 0x07, 0x85, 0x48, 0xb8, 0xd9, 0x23, 0x1f, 0xc3, 0x58, 0x3b, 0xf7, 0x1f, 0x50, 0xf7, 0x13,
        0xf0, 0x19, 0xc1, 0x4d, 0xb0, 0x81, 0x67, 0x54, 0x05, 0x58, 0xd0, 0x1f, 0xba, 0x85, 0xde, 0x9f,
        0x57, 0xbd, 0x2f, 0x8c, 0xa1, 0xdd, 0x8c, 0x15, 0xee, 0xf8, 0x6b, 0x72, 0x60, 0xdf, 0x16, 0xcf,
        0x11, 0xcc, 0x4f, 0xe6, 0x09, 0xb5, 0xe8, 0xa7, 0x6e, 0x2c, 0xd9, 0xc5, 0xe8, 0x46, 0x9c, 0xa6,
        0x29, 0xe5, 0x7b, 0xf8, 0x22, 0xf8, 0x81, 0xaf, 0xa1, 0xa1, 0xb6, 0x5e, 0xec, 0xe7, 0xcd, 0x79,
        0xf9, 0xd7, 0x5e, 0x6c, 0x67, 0x27, 0xbb, 0xe8, 0xd2, 0xda, 0x30, 0xe3, 0xad, 0x49, 0x25, 0x22,
        0x5d, 0xe4, 0x22, 0x46, 0x0a, 0x4b, 0xc9, 0xe4, 0x8d, 0x60, 0xa6, 0xc4, 0xe1, 0x31, 0xe3, 0x00,
        0x2e, 0xac, 0x69, 0x9c, 0x1b, 0x39, 0xbe, 0xf4, 0x29, 0xde, 0xed, 0xa2, 0xc3, 0x15, 0x28, 0x1a,
        0xef, 0xe5, 0xb1, 0x76, 0x99, 0xd2, 0x99, 0x5e, 0xf6, 0x94, 0x10, 0x8a, 0x41, 0x2a, 0xcc, 0xeb,
        0xc2, 0xcf, 0x97, 0x00, 0xd1, 0x68, 0x90, 0xc7, 0x41, 0x2e, 0x0f, 0xce, 0x4b, 0x94, 0xf3, 0xf6,
        0xa0, 0x9c, 0xf2, 0x0f, 0xb3, 0xd0, 0xb2, 0x1e, 0x6f, 0xf3, 0x0b, 0xf7, 0x82, 0x99, 0xde, 0x4c,
        0xe5, 0xe3, 0x68, 0x9e, 0x4a, 0x3f, 0xec, 0x72, 0x13, 0xcc, 0xcf, 0xac, 0xf7, 0x12, 0x69, 0xc3,
        0x02, 0xfc, 0x2b, 0xdc, 0x59, 0x25, 0x4a, 0x24, 0xe1, 0x1d, 0x82, 0xf0, 0xdc, 0xc0, 0x8b, 0xdb,
        0xf3, 0xbd, 0xd1, 0x90, 0x63, 0xcf, 0xc1, 0xbb, 0x9d, 0xec, 0x06, 0x22, 0x77, 0xe4, 0x12, 0xe2,
        0x38, 0xf0, 0x25, 0x75, 0xc6, 0x97, 0xc1, 0xb5, 0xe4, 0x02, 0x19, 0xd8, 0x9f, 0xa5, 0xdb, 0x5a,
        0xb7, 0x32, 0xd5, 0xb3, 0x6a, 0x56, 0x8e, 0x26, 0x40, 0xe6, 0x0c, 0x21, 0xcc, 0x39, 0x8e, 0xbc,
        0xe3, 0xe4, 0x4a, 0xc8, 0x05, 0xc0, 0x5a, 0xb2, 0xb0, 0x7f, 0xb5, 0x72, 0xa0, 0x47, 0x85, 0xe1,
};
	int ret = -1;
/*
	MT_UNF_HDCP_HDCPKEY_S st_hdcpkey;

	st_hdcpkey.enc_flag = 0;
	memcpy(&st_hdcpkey.hdcpkey, in_clear_key, 304);

	ret = MT_UNF_HDCP_encrypt_hdcpkey(st_hdcpkey, out_encrypted_key);
	if (ret != 0)
    	printf("MT_UNF_HDCP_encrypt_hdcpkey failed.\n");
	else
		printf("MT_UNF_HDCP_encrypt_hdcpkey success.\n");
	dump_data(out_encrypted_key, 304);

	memset(out_encrypted_key, 0x00, 304);
	ret = MT_UNF_HDCP_encrypt_hdcpkey_tee(st_hdcpkey, out_encrypted_key);
	if (ret != 0)
		printf("MT_UNF_HDCP_encrypt_hdcpkey_tee failed.\n");
	else
		printf("MT_UNF_HDCP_encrypt_hdcpkey_tee success.\n");

	dump_data(out_encrypted_key, 304);
*/
	ret = MT_UNF_HDCP_load_hdcpkey(out_encrypted_key);
	if(ret != 0)
    	printf("MT_UNF_HDCP_load_hdcpkey failed.\n");
	else
		printf("MT_UNF_HDCP_load_hdcpkey success.\n");

	return ret;
}

void nvSprInitialize(void)
{
	 mt_s32 ret;
       MT_UNF_ENC_FMT_E encFormat = testFormat;
       pthread_t thread_id = 0;

	 #ifndef CREATE_WD_LATER
        MT_UNF_WINDOW_ATTR_S pWinAttr;
	 #endif

	TMSG("---------------s6 nvSpr init start 20240809!----encFormat=%d--------------", encFormat);

	if (pthread_rwlock_wrlock(&rwlock)) {
		EMSG("apply a write lock failed");
		return;
	}
       mt_tfl_command_mutext_init();

	if (isPlatformInitialized) {
		EMSG("the platform has been initialized");
		goto error;
	}

       TEEC_Result tee_res;
       ret = sem_init(&TsBufferSem, 0, 1);

	#ifndef ONLY_TEST_FOR_CLEAR_TS
       /*teec context*/
       tee_res = TEEC_InitializeContext(NULL, &g_teec_ctx);
        if (tee_res != TEEC_SUCCESS) {
        EMSG("Init tee context failed\n");
        goto teec_final;
       }

       /*store the context*/
        nvGpTeeConfigure(&g_teec_ctx);
	#endif

      mtExtMTLZ_TEECInit();

	voutWindow = MT_INVALID_HANDLE;
       voutWindow2 = MT_INVALID_HANDLE;

	ret = CHECK_CALL_MT_FUNC(mt_sys_init);
	if (ret != MT_SUCCESS)
		goto error;

	mtSecRelatedModuleClkEnable();

	ret = CHECK_CALL_MT_FUNC(MT_UNF_DISP_Init);
	if (ret != MT_SUCCESS)
		goto deinit_sound;

	ret = CHECK_CALL_MT_FUNC(mt_hdmi_init, MT_UNF_HDMI_ID_0, encFormat);
	if (ret != MT_SUCCESS)
		goto deinit_system;
#if 1
	ret = CHECK_CALL_MT_FUNC(MTADP_Snd_Init);
	if (ret != MT_SUCCESS)
		goto deinit_system;
#endif
	//printf("snd init success!\n");

	ret = CHECK_CALL_MT_FUNC(MTADP_VO_Init, MT_UNF_VO_DEV_MODE_NORMAL);
	if (ret != MT_SUCCESS)
		goto deinit_display;

	opc_fd = MT_OPCInit();
#ifndef CREATE_WD_LATER


        memset(&pWinAttr, 0, sizeof(MT_UNF_WINDOW_ATTR_S));
        pWinAttr.bUseSubLayer = 0;
        pWinAttr.bSetVideoBot = 1;
        pWinAttr.enDisp = MT_UNF_DISPLAY1;
        pWinAttr.bVirtual = MT_FALSE;
        pWinAttr.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_IGNORE;
        pWinAttr.stWinAspectAttr.bUserDefAspectRatio = MT_FALSE;
        pWinAttr.stWinAspectAttr.u32UserAspectWidth  = 0;
        pWinAttr.stWinAspectAttr.u32UserAspectHeight = 0;
        pWinAttr.bUseCropRect = MT_FALSE;
        pWinAttr.stInputRect.s32X = 0;
        pWinAttr.stInputRect.s32Y = 0;
        pWinAttr.stInputRect.s32Width = 0;
        pWinAttr.stInputRect.s32Height = 0;
        ret = MT_UNF_VO_CreateWindow(&pWinAttr, &voutWindow);
	  if (ret != MT_SUCCESS)
	      goto deinit_vout;

        memset(&pWinAttr, 0, sizeof(MT_UNF_WINDOW_ATTR_S));
        pWinAttr.bUseSubLayer = 1;
        pWinAttr.bSetVideoBot = 1;
        pWinAttr.enDisp = MT_UNF_DISPLAY1;
        pWinAttr.bVirtual = MT_FALSE;
        pWinAttr.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_IGNORE;
        pWinAttr.stWinAspectAttr.bUserDefAspectRatio = MT_FALSE;
        pWinAttr.stWinAspectAttr.u32UserAspectWidth  = 0;
        pWinAttr.stWinAspectAttr.u32UserAspectHeight = 0;
        pWinAttr.bUseCropRect = MT_FALSE;
        pWinAttr.stInputRect.s32X = 0;
        pWinAttr.stInputRect.s32Y = 0;
        pWinAttr.stInputRect.s32Width = 0;
        pWinAttr.stInputRect.s32Height = 0;
        ret = MT_UNF_VO_CreateWindow(&pWinAttr, &voutWindow2);
	 if (ret != MT_SUCCESS)
	      goto deinit_vout;

	 ret = CHECK_CALL_MT_FUNC(MT_UNF_DISP_SetDiOnOff, MT_FALSE);

	 MT_UNF_VO_GetWindowAttr(voutWindow, &pWinAttr);
        pWinAttr.enDisp = 1;
        pWinAttr.stInputRect.s32X = 0;
        pWinAttr.stInputRect.s32Y = 0;
        pWinAttr.stInputRect.s32Width = 0;
        pWinAttr.stInputRect.s32Height = 0;


	 pWinAttr.stOutputRect.s32X = 0;
        pWinAttr.stOutputRect.s32Y = 0;
        pWinAttr.stOutputRect.s32Width = 0;
        pWinAttr.stOutputRect.s32Height = 0;

	 #if 0
	 pWinAttr.stOutputRect.s32X = 800;
        pWinAttr.stOutputRect.s32Y = 50;
        pWinAttr.stOutputRect.s32Width = 512;
        pWinAttr.stOutputRect.s32Height = 360;
	 #endif

        MT_UNF_VO_SetWindowAttr(voutWindow, &pWinAttr);

	 MT_UNF_VO_GetWindowAttr(voutWindow2, &pWinAttr);

	 pWinAttr.enDisp = 1;
        pWinAttr.stInputRect.s32X = 0;
        pWinAttr.stInputRect.s32Y = 0;
        pWinAttr.stInputRect.s32Width = 0;
        pWinAttr.stInputRect.s32Height = 0;

	 pWinAttr.stOutputRect.s32X = 40;
        pWinAttr.stOutputRect.s32Y = 20;
        pWinAttr.stOutputRect.s32Width = 480;
        pWinAttr.stOutputRect.s32Height = 360;

	 #if 0
	 pWinAttr.stOutputRect.s32X = 40;
        pWinAttr.stOutputRect.s32Y = 20;
        pWinAttr.stOutputRect.s32Width = 512;
        pWinAttr.stOutputRect.s32Height = 360;
	 #endif

        MT_UNF_VO_SetWindowAttr(voutWindow2, &pWinAttr);

	if (ret != MT_SUCCESS)
		goto deinit_vout;
#endif

	ret = CHECK_CALL_MT_FUNC(MT_UNF_DMX_Init);
	if (ret != MT_SUCCESS)
		goto destroy_window;
#if 1
	ret = CHECK_CALL_MT_FUNC(MTADP_AVPlay_RegADecLib);
	if (ret != MT_SUCCESS)
		goto deinit_demux;
#endif

	ret = CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_Init);
	if (ret != MT_SUCCESS)
		goto deinit_demux;

	//Fix Bug 125644
	//GPIO7 used for control external amplifier
	MT_UNF_GPIO_Init();
	MT_UNF_GPIO_SetDirBit(MT_UNF_GPIO_7, MT_FALSE);
	//FIXME: default set unmute
	MT_UNF_GPIO_WriteBit(MT_UNF_GPIO_7, MT_TRUE);


#ifndef ONLY_TEST_FOR_CLEAR_TS
#if CFG_TFL_USE_DYNC_SHMEM
    tee_res = TEEC_RegisterSharedMemory(&g_teec_ctx, &tfl_shm);
    if (tee_res != TEEC_SUCCESS) {
        EMSG("Register dynamic share memory failed, ret: 0x%08x\n", tee_res);
        goto deinit_demux;
    }
#endif
#endif

	isPlatformInitialized = TRUE;

	//mt_hdmi_setHdcp( 0 );


	if (pthread_rwlock_unlock(&rwlock))
		EMSG("release a write lock failed");

	/*TSecNuid nuid;
	secGetNuid(&nuid);

	secGetNuid64(&nuid);*/
    MT_USLEEP(5000000);
    drv_disp_reset_hdmi_fmt_ability();
    testFormat = drv_disp_get_auto_hdmi_fmt();
    //for MANUAL case to test HDCP
    encFormat = testFormat;
    mt_u32 data = 0;
    MT_UNF_ENC_FMT_E   currFmt = MT_UNF_ENC_FMT_BUTT;
    printf("encFormat = 0x%x \n", encFormat);
    //MT_USLEEP(5000000);
    mt_sys_read_register(0xbf314400, &data);


    TMSG("data 0xbf314400 = 0x%x \n", data);
    if (data & 0x80000000) { //set disbply output format

        encFormat = data  & 0xFF; //UHD output
        TMSG("encFormat = 0x%x \n", encFormat);


        //MT_USLEEP(800000);
    }
    MT_UNF_HDMI_SetFormat(MT_UNF_HDMI_ID_0, encFormat);
    ret= MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY1, encFormat);
    if (ret != MT_SUCCESS)
    {
        TMSG("call MT_UNF_DISP_Attach failed, Ret=%#x.\n", ret);
        return;
    }
    ret = MT_UNF_DISP_GetFormat(MT_UNF_DISPLAY1, &currFmt);
    if(MT_SUCCESS != ret)
    {
        TMSG("MT_UNF_DISP_GetFormat failed. ret = 0x%x\n", ret);

    }
    //TMSG(" current format is [%s] \n", MT_DispFmt2Str(currFmt));
	/*
    if (grantTestBuffer == NULL) {
        grantTestBuffer= (TUnsignedInt8 *)mmz_malloc("ddr", 0x140000, SMP_DDR_ALIGN_UNIT);
        grantTestBufferPhy = vir2phy(grantTestBuffer);
    }
    TMSG("grantTestBuffer=0x%x, grantTestBufferPhy=0x%x \n", grantTestBuffer, grantTestBufferPhy);
	*/
    //MT_USLEEP(8000000);
    //set for clear stream encrypted by Nagra Key!


#ifdef _MT_WITH_CAK_TEST //only for CAK test
     //MTSampleCommandInit();
    pthread_create(&thread_id, NULL, thread_function_check_OPC, &ret);
    //Watermark will be test by telnet running command:
    //optee_example_ngwm /media/sda1/#1_0x12345678_0xc8_1530532846_stubon_on.bin
    //pthread_create(&thread_id2, NULL, MTSampleCommadTask, &ret);
#endif

       //ret = mtSecSetEventCallback(MT_SEC_EVENT_SMPC_CLOSE_START, mtTestSMPCloseEventCallbackStart);
       printf("%s:%d ret = 0x%x \n", __FUNCTION__, __LINE__, ret);
       //ret = mtSecSetEventCallback(MT_SEC_EVENT_SMPC_CLOSE_END, mtTestSMPCloseEventCallbackEnd);
       //printf("%s:%d ret = 0x%x \n", __FUNCTION__, __LINE__, ret);


	DMSG("---------------nvSpr init end!----thread_id=0x%x----------", thread_id);
	return;

deinit_demux:
	CHECK_CALL_MT_FUNC(MT_UNF_DMX_DeInit);
destroy_window:
	CHECK_CALL_MT_FUNC(MT_UNF_VO_DestroyWindow, voutWindow);
	voutWindow = MT_INVALID_HANDLE;
	CHECK_CALL_MT_FUNC(MT_UNF_VO_DestroyWindow, voutWindow2);
	voutWindow2 = MT_INVALID_HANDLE;
#ifndef CREATE_WD_LATER
deinit_vout:
	CHECK_CALL_MT_FUNC(MTADP_VO_DeInit);
#endif
deinit_display:
	CHECK_CALL_MT_FUNC(MTADP_Disp_DeInit);
deinit_sound:
	CHECK_CALL_MT_FUNC(MTADP_Snd_DeInit);
deinit_system:
	CHECK_CALL_MT_FUNC(mt_sys_deinit);
teec_final:
    nvGpTeeConfigure(NULL);
    TEEC_FinalizeContext(&g_teec_ctx);
error:
	if (pthread_rwlock_unlock(&rwlock))
		EMSG("release a write lock failed");

}

#ifdef LOG_TO_FILE
extern FILE *gLogFile;
extern uint8_t gSimbadTerminated;
#endif

void nvSprTerminate(void)
{
	printf("nvSprTerminate In");
	if (pthread_rwlock_wrlock(&rwlock)) {
		EMSG("apply a write lock failed");
		return;
	}

	if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");
		goto end;
	}

	isPlatformInitialized = FALSE;

	CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_DeInit);
	CHECK_CALL_MT_FUNC(MT_UNF_DMX_DeInit);
	CHECK_CALL_MT_FUNC(MT_UNF_VO_DestroyWindow, voutWindow);
	CHECK_CALL_MT_FUNC(MT_UNF_VO_DestroyWindow, voutWindow2);
	voutWindow = MT_INVALID_HANDLE;
	voutWindow2 = MT_INVALID_HANDLE;
	CHECK_CALL_MT_FUNC(MTADP_VO_DeInit);
	CHECK_CALL_MT_FUNC(MTADP_Disp_DeInit);
	CHECK_CALL_MT_FUNC(MTADP_Snd_DeInit);
	CHECK_CALL_MT_FUNC(mt_sys_deinit);

	clearAllTransportSeesion();

    nvGpTeeConfigure(NULL);
#if CFG_TFL_USE_DYNC_SHMEM
    TEEC_ReleaseSharedMemory(&tfl_shm);
#endif
    TEEC_FinalizeContext(&g_teec_ctx);

end:
	if (pthread_rwlock_unlock(&rwlock))
		EMSG("release a write lock failed");

#ifdef LOG_TO_FILE
	/* Close the nvLog */
	if (gLogFile != NULL) {
		fclose(gLogFile);
		gLogFile = NULL;
		gSimbadTerminated = 1;
	}
#endif
       if (grantTestBuffer) {
            mmz_free(grantTestBuffer);
            grantTestBuffer = NULL;
       }
       mtExtMTLZ_TEECDeinit();
	DMSG("Out:>>>>>nvSpr Terminated<<<<<");
}

void nvSprSetDebug(TUnsignedInt16 level)
{
	DMSG("Todo:level=%d", level);
}

TNvSprStatus nvSprSetSmp(TTransportSessionId tsid, TBoolean smpEnabled)
{
    TNvSprStatus st = NV_SPR_NO_ERROR;
    struct transportSession *ts = NULL;

	TMSG("[In]SMP:%d", smpEnabled);

	WR_LOCK(&rwlock);

	ts = getTransportSessionById(tsid);
	if (!ts) {
		EMSG("the transport session identified by this id(%d) isn't existing", tsid);
		st = NV_SPR_ERROR_BAD_PARAM;
		goto end;
	}

    ts->smpEnabled = smpEnabled;

	if (ts->smpEnabled) {
		ts->smpSetFlag = TRUE;
	}

end:
	WR_UNLOCK(&rwlock);
	TMSG("[Out]SMP:%d", smpEnabled);
	return st;
}

void dump_status(TTransportSessionId tsid, MT_UNF_AVPLAY_STATUS_INFO_S *statusInfo, TNvSprStatus st )
{
	DMSG("dump_status tsid = %d rd 0x%x wt 0x%x used 0x%x  frameCount %d  st %d \n",  tsid,
	statusInfo->stBufStatus[0].u32BufRptr,
	statusInfo->stBufStatus[0].u32BufWptr,
	statusInfo->stBufStatus[0].u32UsedSize,
	statusInfo->u32VidFrameCount, st);
}

TNvSprStatus nvSprNexGuardPlatformUnInit(void)
{
	return NV_SPR_NO_ERROR;
}

TNvSprStatus nvSprGetAudioDescrambling
(
  TTransportSessionId tsid,
  TBoolean*           xStatus
)
{
	mt_s32 ret;
	TNvSprStatus st = NV_SPR_NO_ERROR;
	struct transportSession *ts;
	MT_UNF_AVPLAY_STATUS_INFO_S statusInfo;

	TMSG("tsid = 0x%x \n", tsid);

	 if(tsid >= 100)
        tsid /= 100;

	CHECK_TSID_VALIDITY(tsid);

	if (!xStatus) {
		EMSG("status is NULL pointer");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	RD_LOCK(&rwlock);

	if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");

		*xStatus = FALSE;
		st = NV_SPR_ERROR;
		goto end;
	}

	ts = getTransportSessionById(tsid);
	if (!ts) {
		EMSG("the transport session identified by this id(%d) isn't existing", tsid);

		*xStatus = FALSE;
		st = NV_SPR_ERROR_BAD_PARAM;
		goto end;
	}

	ret = MT_UNF_AVPLAY_GetStatusInfo(ts->avPlayer, &statusInfo);
	if (ret != MT_SUCCESS) {
		EMSG("Get AVPLAY Status failed");
		*xStatus = FALSE;
		st = NV_SPR_ERROR;
		goto end;
	}

	//TMSG("###### last vid count = %d ######", ts->descramblingCount);
	TMSG("###### current vid count = %d  statusInfo.u32VidErrorFrameCount = %d ######", statusInfo.u32VidFrameCount, statusInfo.u32VidErrorFrameCount);
	if (statusInfo.u32AuddFrameCount > ts->audiodescramblingCount) {
		*xStatus = TRUE;
		printf("tsid = %d audio OK-------------------------------------\n", tsid);
	} else {
		*xStatus = FALSE;
		printf("tsid = %d audio FAIL -----------------------------------\n", tsid);
	}

	//remember previous count
	ts->audiodescramblingCount = statusInfo.u32VidFrameCount;
	//TMSG("status = %d tsid = %d \n", *status, tsid);

	st = NV_SPR_NO_ERROR;
end:
	dump_status(tsid, &statusInfo, st);
	RD_UNLOCK(&rwlock);

	return st;
}

void mtTestCheckDownscaler(uint64_t opc_value)
{
    MT_UNF_ENC_FMT_E encFormat = testFormat;

    TMSG("opc_value = 0x%lx \n", opc_value);
    //HDMI1.x bit6 and HDMI 2.0 bit7,
    //if ((opc_value & 0xC0) &&  ((opc_value & 0xC0) != 0xC0))  //pre-integration test used before.
    //for talts HMID case
    if ((opc_value) && ((opc_value & 0xC0) != 0xC0))
    {
        opc_value = (opc_value >> 8);
        if ((opc_value & 0xF) == 0xF)
        {
            return;
        }
        //4K, 2K, HD, SD
        if ((opc_value & 0x1) == 0)
        {
            encFormat = testFormat;
        } else if ((opc_value & 0x2) == 0)
        {
            encFormat = MT_UNF_ENC_FMT_1080i_50;
        } else if ((opc_value & 0x4) == 0)
        {
            encFormat = MT_UNF_ENC_FMT_720P_60;
        } else if ((opc_value & 0x8) == 0)
        {
            encFormat = MT_UNF_ENC_FMT_576P_50;
        }
        TMSG("encFormat = 0x%x \n", encFormat);
        mtTestUpdateDisplayResolution(encFormat);
        //MT_USLEEP(1000);

    }

}

TNvSprStatus nvSprGetDescrambling(TTransportSessionId tsid, TBoolean *status)
{
	mt_s32 ret;
	TNvSprStatus st = NV_SPR_NO_ERROR;
	struct transportSession *ts;
	MT_UNF_AVPLAY_STATUS_INFO_S statusInfo;
	uint64_t opc_status;

	//TMSG("\n");

	CHECK_TSID_VALIDITY(tsid);

	if (!status) {
		EMSG("status is NULL pointer");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	RD_LOCK(&rwlock);

	if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");

		*status = FALSE;
		st = NV_SPR_ERROR;
		goto end;
	}

	ts = getTransportSessionById(tsid);
	if (!ts) {
		EMSG("the transport session identified by this id(%d) isn't existing", tsid);

		*status = FALSE;
		st = NV_SPR_ERROR_BAD_PARAM;
		goto end;
	}

	ret = MT_UNF_AVPLAY_GetStatusInfo(ts->avPlayer, &statusInfo);
	if (ret != MT_SUCCESS) {
		EMSG("Get AVPLAY Status failed");
		*status = FALSE;
		st = NV_SPR_ERROR;
		goto end;
	}
	//opc_status = MT_GetOPCStatus();

        mt_sys_read_register(0xbf448090, (mt_u32 *)&ret);
        opc_status = ret;
        mt_sys_read_register(0xbf448094, (mt_u32 *)&ret);
        opc_status = opc_status | ((uint64_t)ret << 32);
       //adjust display output format
       mtTestCheckDownscaler(opc_status);

       mt_sys_read_register(0xbf448090, (mt_u32 *)&ret);
        opc_status = ret;
        mt_sys_read_register(0xbf448094, (mt_u32 *)&ret);
        opc_status = opc_status | ((uint64_t)ret << 32);


	//if(status > 0){
	printf("   ######################## get opc status: 0x%lx \n",  opc_status);
	//}
	if( opc_status != 0 ) //有违规操作了
	{
		ts->ts_opc_err_status_count++;
	}
	else if(ts->ts_opc_err_status_count)
	{
		//ts->ts_opc_err_status_count--;
		ts->ts_opc_err_status_count = 0;
	}

	//TMSG("###### last vid count = %d ######", ts->descramblingCount);
	printf("tsid = %d current vid count = %d, descramblingCount=%d,  ts_opc_err_status_count = %d u32VidErrorFrameCount = %d   u32VideoESFrameNumber = %d u32UsedSize = %d \n",
	tsid, statusInfo.u32VidFrameCount, ts->descramblingCount, ts->ts_opc_err_status_count, statusInfo.u32VidErrorFrameCount, statusInfo.stBufStatus[0].u32VideoESFrameNumber, statusInfo.stBufStatus[0].u32UsedSize);

	if (statusInfo.u32VidFrameCount > ts->descramblingCount) {
		*status = TRUE;
		printf("tsid = %d OK-------------------------------------\n", tsid);
	} else {
		*status = FALSE;
		printf("tsid = %d FAIL -----------------------------------\n", tsid);
	}

#if 1 // HDCP test will occurt it, so how to do ????
	if( ts->ts_opc_err_status_count > 0)//2) //for HDCP test set to more than 1???
	{
		*status = FALSE;
		printf("opc_err tsid = %d FAIL -----------------------------------\n", tsid);
	}
#endif
	//remember previous count
	ts->descramblingCount = statusInfo.u32VidFrameCount;
	//TMSG("status = %d tsid = %d \n", *status, tsid);

	st = NV_SPR_NO_ERROR;
end:
	dump_status(tsid, &statusInfo, st);
	RD_UNLOCK(&rwlock);

	return st;
}


TNvSprStatus nvSprBlackScreen(TTransportSessionId tsid)
{
    TNvSprStatus status = NV_SPR_NO_ERROR;
#if 1
    struct transportSession *ts;

    DMSG("%s:%d tsid = %d \n", __FUNCTION__, __LINE__, tsid);
    CHECK_TSID_VALIDITY(tsid);

    WR_LOCK(&rwlock);
    if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");

		status = NV_SPR_ERROR;
		goto end;
    }
    ts = getTransportSessionById(tsid);
    if (!ts) {
    	DMSG("the transport session identified by this id(%d) isn't existing", tsid);

    	status = NV_SPR_ERROR_BAD_PARAM;
    	goto end;
    }
    DMSG("%s:%d tsid = %d session = %d\n", __FUNCTION__, __LINE__, tsid, ts->sessionType);
    if (ts->sessionType == TRANSPORT_SESSION_TYPE_DVB
		|| ts->sessionType == TRANSPORT_SESSION_TYPE_REPLAY
		|| ts->sessionType == TRANSPORT_SESSION_TYPE_OTT) {



        if (ts->bUseSubLayer) {
            MT_UNF_DISP_SetLayerShow(MT_UNF_DISPLAY1, MT_UNF_DISP_LAYER_ID_STILL_HD, MT_FALSE);
            MT_USLEEP(40000);
        } else {
            DMSG("%s:%d \n", __FUNCTION__, __LINE__);
            CHECK_CALL_MT_FUNC(MT_UNF_DISP_VidLayerShow, MT_FALSE);
            MT_USLEEP(55000);
        }
        DMSG("black screen \n");
        if (ts->doesAudChanOpen) {
		closeAudio(ts);
        }
        DMSG("black screen \n");

    }
end:
    WR_UNLOCK(&rwlock);
#endif
    return status;
}


void mtTestSMPCloseEventCallbackStart(TTransportSessionId tsid, MT_SEC_EVENT_E sec_event)
{
#ifndef _MT_WITH_TALTS_
    struct transportSession *ts;
    MT_UNF_AVPLAY_STOP_OPT_S stopOpt;
    unsigned long call_curTime = 0;
    static unsigned long call_lastTime= 0;

    TMSG("%s:%d tsid = %d \n", __FUNCTION__, __LINE__, tsid);
    //CHECK_TSID_VALIDITY(tsid);

    if ((!isPlatformInitialized) || (sec_event != MT_SEC_EVENT_SMPC_CLOSE_START)) {
        EMSG("the platform has not been initialized");
        goto end;
    }
    call_curTime = get_sys_time_ms();
    if ((call_lastTime != 0) && ((call_lastTime + 300) >= call_curTime)) { //interval time must more than 400 ms
        printf("%s:%d tsid = %d, %d, %d \n", __FUNCTION__, __LINE__, tsid, call_lastTime, call_curTime);
        goto end;
    }
#if 1
    pthread_rwlock_wrlock(&rwlock);
    ts = getTransportSessionById(tsid);
    if (!ts) {
        EMSG("the transport session identified by this id(%d) isn't existing", tsid);
        pthread_rwlock_unlock(&rwlock);
        goto end;
    }
    TMSG("%s:%d tsid = %d ts->avPlayer=0x%x, session = %d\n", __FUNCTION__, __LINE__, tsid, ts->avPlayer, ts->sessionType);
    if (ts->sessionType == TRANSPORT_SESSION_TYPE_DVB
		|| ts->sessionType == TRANSPORT_SESSION_TYPE_REPLAY
		|| ts->sessionType == TRANSPORT_SESSION_TYPE_OTT) {

        stopOpt.u32TimeoutMs = 0;
        if (ts->bUseSubLayer) {
            stopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
        } else {
            stopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
        }
        if (ts->avPlayer && (ts->avPlayer != MT_INVALID_HANDLE)) {
            MT_UNF_AVPLAY_Stop(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &stopOpt);
            ts->videoDecodeStart = FALSE;
            TMSG("%s:%d tsid = %d, %d, %d \n", __FUNCTION__, __LINE__, tsid, call_lastTime, call_curTime);
            MT_USLEEP(40000);
        }

    }
    pthread_rwlock_unlock(&rwlock);
#endif
    printf("%s:%d tsid = %d %d\n", __FUNCTION__, __LINE__, tsid, call_lastTime);
end:
    call_lastTime = call_curTime;

#endif
}


void mtTestSMPCloseEventCallbackEnd(TTransportSessionId tsid, MT_SEC_EVENT_E sec_event)
{
#if 0
    struct transportSession *ts;
    //MT_UNF_AVPLAY_STOP_OPT_S stopOpt;
    unsigned long call_curTime = 0;
    static unsigned long call_lastTime= 0;

    TMSG("%s:%d tsid = %d \n", __FUNCTION__, __LINE__, tsid);
    //CHECK_TSID_VALIDITY(tsid);
    pthread_rwlock_wrlock(&rwlock);
    if ((!isPlatformInitialized) || (sec_event != MT_SEC_EVENT_SMPC_CLOSE_END)) {
		EMSG("the platform has not been initialized");
		goto end;
    }
    call_curTime = get_sys_time_ms();
    if ((call_lastTime != 0) && ((call_lastTime + 300) >= call_curTime)) {  //interval time must more than 300 ms
        //printf("%s:%d tsid = %d, %d, %d \n", __FUNCTION__, __LINE__, tsid, call_lastTime, call_curTime);
        goto end;
    }
    ts = getTransportSessionById(tsid);
    if (!ts) {
    	EMSG("the transport session identified by this id(%d) isn't existing", tsid);
       goto end;
    }

    if (ts->sessionType == TRANSPORT_SESSION_TYPE_DVB
		|| ts->sessionType == TRANSPORT_SESSION_TYPE_REPLAY
		|| ts->sessionType == TRANSPORT_SESSION_TYPE_OTT) {
        TMSG("%s:%d tsid = %d, ts->avPlayer = 0x%x, %d, %d \n", __FUNCTION__, __LINE__, tsid, ts->avPlayer, call_lastTime, call_curTime);
        if (ts->avPlayer && (ts->avPlayer != MT_INVALID_HANDLE)) {
            MT_UNF_AVPLAY_Start(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, NULL);
            MT_USLEEP(10000);
        }

    }
 end:
    call_lastTime = call_curTime;
    pthread_rwlock_unlock(&rwlock);
#endif
}
TNvSprStatus nvSprCloseSession(TTransportSessionId tsid)
{
	TNvSprStatus status = NV_SPR_NO_ERROR;
	struct transportSession *ts;
	mt_s32 mtRet = 0;
static mt_u8 record_close_times = 0;

	CHECK_TSID_VALIDITY(tsid);
       mtSecClrPlatDmxId(tsid);

	TMSG("Spr done, close[tsid=%d]...", tsid);

	WR_LOCK(&rwlock);

        //call_lastTime = 0;

       /*
        if (MTPVRCount >= 3) {
            TMSG("MTPVRCount = %d", MTPVRCount);
            MTPVRCount--;
            return NV_SPR_NO_ERROR;
       }
        */
	if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");

		status = NV_SPR_ERROR;
		goto end;
	}
	ts = getTransportSessionById(tsid);
	if (!ts) {
		EMSG("the transport session identified by this id(%d) isn't existing", tsid);

		status = NV_SPR_ERROR_BAD_PARAM;
		goto end;
	}
       TMSG("ts->sessionType = 0x%x \n", ts->sessionType);
        if ((record_close_times == 0) && (ts->sessionType == TRANSPORT_SESSION_TYPE_RECORD) ){
            record_close_times = 1;
            TMSG("\n");
            status = NV_SPR_ERROR;
            goto end;
       } else {
            record_close_times = 0;
            TMSG("\n");
       }




	ts->isClosing = TRUE;
	//printf("nvSprCloseSession tsid=%d isClosing=%d ts->avPlayer = %ld\n", ts->tsid, ts->isClosing, ts->avPlayer );

	if (ts->sessionType == TRANSPORT_SESSION_TYPE_DVB
		|| ts->sessionType == TRANSPORT_SESSION_TYPE_REPLAY
		|| ts->sessionType == TRANSPORT_SESSION_TYPE_OTT) {
/*
#ifdef STOP_WITH_STILL_FRAME
		DMSG("closing windows[No, STILL test]...");
#else
		CHECK_CALL_MT_FUNC(MT_UNF_DISP_VidLayerShow, MT_FALSE);
#endif
*/
		/*DMSG("reset before stop");
		CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_Reset, ts->avPlayer, NULL);*/

		NEW_MSG("stopping AvPlayer...");
		stopAvPlayer(ts);
		closeWindow(ts);
		closeAvPlayer(ts);
		TMSG("AvPlayer Stopped");

              if (ts->bUseSubLayer) {
                    //really dvb live
                    //if ((ts->sessionType == TRANSPORT_SESSION_TYPE_DVB) && (ts->tsid < 20))
                    {
                        mtExtMTLZ_TEE_SMP_Dis(FALSE);
                    }
		} else {
                #ifdef _MT_WITH_CAK_TEST //only for CAK test

                drv_disp_reset_hdmi_fmt_ability();
                testFormat = drv_disp_get_auto_hdmi_fmt();
                mtTestUpdateDisplayResolution(testFormat);
                TMSG("reset Resolution to UHD ? testFormat = 0x%x", testFormat);
                #endif
                    //really dvb live
                //if ((ts->sessionType == TRANSPORT_SESSION_TYPE_DVB) && (ts->tsid < 20))
                {
        		mtExtMTLZ_TEE_SMP_Dis(TRUE);
                }
		}


	}

	if (ts->sessionType == TRANSPORT_SESSION_TYPE_RECORD) {

            if(gwriteFileVfp!= NULL)
                {
                    fclose(gwriteFileVfp);
                    gwriteFileVfp = NULL;
                    TMSG("close test write file\n");
                }


#ifdef USE_DMX_RECORDING
		mt_record_stop(ts);
#else


		NEW_MSG("Unregister callback");
		//CHECK_CALL_MT_FUNC(MT_UNF_PVR_UnRegisterExtraCallBack, ts->recordChannelId,
		//MT_UNF_PVR_EXTRA_WRITE_CALLBACK);
		mtRet = MT_UNF_PVR_UnRegisterExtraCallBack(ts->recordChannelId, MT_UNF_PVR_EXTRA_WRITE_CALLBACK);
		TMSG("Unregister callback, mtRet:%x", mtRet);
		MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_REC_DMX_CREATE);

            TMSG("Stop the REC channel");
		//CHECK_CALL_MT_FUNC(MT_UNF_PVR_RecStopChn, ts->recordChannelId);
		mtRet = MT_UNF_PVR_RecStopChn(ts->recordChannelId);
		TMSG("Stop the REC channel, mtRet:%x", mtRet);

		TMSG("Destroy Rec channel");
		//CHECK_CALL_MT_FUNC(MT_UNF_PVR_RecDestroyChn, ts->recordChannelId);
		mtRet = MT_UNF_PVR_RecDestroyChn(ts->recordChannelId);
		TMSG("Destroy Rec channel, mtRet:%x", mtRet);

		//CHECK_CALL_MT_FUNC(MT_UNF_PVR_RecDeInit);
		mtRet = MT_UNF_PVR_RecDeInit();

		NEW_MSG("Recorder Deinit done!, mtRet:%x", mtRet);
#endif

            struct stat fileInfo;
            mtRet = stat(ts->rec_file_name, &fileInfo);
            if (mtRet == -1) {
                EMSG("Error getting file information");
            } else {
                TMSG("File size: %ld bytes\n", fileInfo.st_size);
            }
            if (fileInfo.st_size <= 0x80000) {
                 remove(ts->rec_file_name);
                 printf("remove size = 0x%x\n", ts->rec_file_name);
            }
            //MTPVRCount--;


	}

	if (ts->tsBufHandle != MT_INVALID_HANDLE) {
		CHECK_CALL_MT_FUNC(MT_UNF_DMX_ResetTSBuffer, ts->tsBufHandle);
		CHECK_CALL_MT_FUNC(MT_UNF_DMX_DestroyTSBuffer, ts->tsBufHandle);

		if (ts->sessionType == TRANSPORT_SESSION_TYPE_RECORD) {
			CHECK_CALL_MT_FUNC(MT_UNF_DMX_DetachTSPort, ts->recordDmxId);
		} else {
			CHECK_CALL_MT_FUNC(MT_UNF_DMX_DetachTSPort, ts->playDmxId);
		}

		ts->tsBufHandle = MT_INVALID_HANDLE;
		ts->tsPort = MT_UNF_DMX_PORT_BUTT;

		DMSG("tsBuf handle destroyed, and tsport detached!");
	}

	if (ts->avPlayerStreamType == MT_UNF_AVPLAY_STREAM_TYPE_ES
		|| ts->sessionType == TRANSPORT_SESSION_TYPE_REPLAY) {

		if (ts->aacAdtsHeader != NULL) {
                    DMSG("aacHeader buffer freed ts->aacAdtsHeader = %lx", ts->aacAdtsHeader);
			mt_unf_cipher_free(ts->aacAdtsHeader);

			ts->aacAdtsHeader = NULL;
		}
	}
	if (ts->tsBuffer != NULL) {
             //printf("free s->tsBuffer ----%lx-------- \n", ts->tsBuffer);
		mmz_free(ts->tsBuffer);
		ts->tsBuffer = NULL;

	}
	if (ts->tsIntBufferVir != NULL) {
             //printf("free s->tsIntBufferVir -------0x%lx-------- \n", ts->tsIntBufferVir);
		mt_unf_cipher_free(ts->tsIntBufferVir);
		ts->tsIntBufferVir = NULL;

	}
	delTransportSession(ts);

       mtSecResetProtectBufferList();


	status = NV_SPR_NO_ERROR;

	//close_test_file();



end:
	WR_UNLOCK(&rwlock);

	TMSG(">>>>session(tsid=%d) closed(unlocked)<<<<", tsid);
	//printf("%s %d tsid = %d exit success!\n",__FUNCTION__,__LINE__, tsid);

	return status;
}
/*
static void dumpPidList(TNvSprPidList *pids)
{
	TUnsignedInt32 i;
	TNvSprPidInfo *pidInfo;

	//printf("dump pid list pids->count= %d :", pids->count);

	for (i = 0; i < pids->count; i++) {
		pidInfo = pids->elems + i;
		printf("[%d]: Stream type(0x%x) -- Pid(%d)\n", i, pidInfo->streamType, pidInfo->pid);
	}
}
*/
static TUnsignedInt16 toSecPidType(TUnsignedInt16 streamType)
{
	switch (streamType) {
	case DVB_STREAM_TYPE_MPEG1_VIDEO:
	case DVB_STREAM_TYPE_MPEG2_VIDEO:
	case DVB_STREAM_TYPE_MPEG4_VIDEO:
	case DVB_STREAM_TYPE_H264_VIDEO:
	case DVB_STREAM_TYPE_AVS_VIDEO:
	case DVB_STREAM_TYPE_VC1_VIDEO:
	case DVB_STREAM_TYPE_HEVC_VIDEO:
		return SEC_PID_VIDEO;
	case DVB_STREAM_TYPE_MPEG1_AUDIO:
	case DVB_STREAM_TYPE_MPEG2_AUDIO:
	case DVB_STREAM_TYPE_AAC_AUDIO:
	case DVB_STREAM_TYPE_AAC_AUDIO_1:
		return SEC_PID_AUDIO;
	default:
		return SEC_PID_VIDEO; //TODO
	}
}

static TNvSprStatus parsePidList(TNvSprPidList *pids,
                                 MT_UNF_VCODEC_TYPE_E *vidCodec, mt_u32 *vidPid,
                                 HA_CODEC_ID_E *audCodec, mt_u32 *audPid)
{
	TUnsignedInt32 i;
	TBoolean doFindVideo;
	TBoolean doFindAudio;

	//dumpPidList(pids);

	/* Find video pid */
	for (i = 0; i < pids->count; i++) {
		doFindVideo = TRUE;
		//printf("doFindVideo   pids->elems[i].streamType = %d \n", pids->elems[i].streamType);
		switch (pids->elems[i].streamType) {
		case DVB_STREAM_TYPE_MPEG1_VIDEO:
			*vidCodec = MT_UNF_VCODEC_TYPE_MPEG2;
			break;

		case DVB_STREAM_TYPE_MPEG2_VIDEO:
			*vidCodec = MT_UNF_VCODEC_TYPE_MPEG2;
			break;

		case DVB_STREAM_TYPE_MPEG4_VIDEO:
			*vidCodec = MT_UNF_VCODEC_TYPE_MPEG4;
			break;

		case DVB_STREAM_TYPE_H264_VIDEO:
			*vidCodec = MT_UNF_VCODEC_TYPE_H264;
			break;

		case DVB_STREAM_TYPE_AVS_VIDEO:
			*vidCodec = MT_UNF_VCODEC_TYPE_AVS;
			break;

		case DVB_STREAM_TYPE_VC1_VIDEO:
			*vidCodec = MT_UNF_VCODEC_TYPE_VC1;
			break;

		case DVB_STREAM_TYPE_HEVC_VIDEO:
			*vidCodec = MT_UNF_VCODEC_TYPE_HEVC;
			break;
		default:
			doFindVideo = FALSE;
			break;
		}

		if (doFindVideo) {
			*vidPid = pids->elems[i].pid;
			break;
		}
	}

	if (i >= pids->count) {
		EMSG("can't find video from this stream");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	/* Find audio pid */
	for (i = 0; i < pids->count; i++) {
		doFindAudio = TRUE;
		//printf("doFindAudio   pids->elems[i].streamType = %d \n", pids->elems[i].streamType);

		switch (pids->elems[i].streamType) {
		case DVB_STREAM_TYPE_MPEG1_AUDIO:
			{
				*audCodec = HA_AUDIO_ID_MP2;
				 //printf("audCodec == HA_AUDIO_ID_MP2 \n");

			break;
			}
		case DVB_STREAM_TYPE_MPEG2_AUDIO:
			*audCodec = HA_AUDIO_ID_MP2;
			break;

		case DVB_STREAM_TYPE_AAC_AUDIO:
			*audCodec = HA_AUDIO_ID_AAC;
			break;

		case DVB_STREAM_TYPE_AAC_AUDIO_1:
			*audCodec = HA_AUDIO_ID_AAC;
			break;

		default:
			doFindAudio = FALSE;
			break;
		}

		if (doFindAudio) {
			*audPid = pids->elems[i].pid;
			break;
		}
	}

	if (i >= pids->count) {
		EMSG("can't find audio from this stream");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	return NV_SPR_NO_ERROR;
}

//#define MAX_CMDLINE_LEN 1280
//static mt_char g_CmdLine[MAX_CMDLINE_LEN];

TNvSprStatus nvSprOpenDvbSession(TTransportSessionId tsid, TNvSprPidList *pids, TBoolean smp)
{
	mt_s32 ret;
	TNvSprStatus status = NV_SPR_NO_ERROR;
	struct transportSession *ts = NULL;
       mt_u32 vidPid = 0;
       mt_u32 audPid = 0;
	mt_u32 pcrPid = 0;
	MT_UNF_SYNC_REF_E syncRef = MT_UNF_SYNC_REF_NONE;
	//mt_handle videoChannel;
	mt_handle audioChannel;
	MBInfo mb = {0};

	ulong  esAudioBuffPhyAddr;
       mt_u32  esBuffSize;
	//mt_handle video_ch, audio_ch;
	TUnsignedInt32 i = 0;
       //mt_char *pCmdLine=NULL;

	//return NV_SPR_NO_ERROR;

	UNUSED(smp);
	CHECK_TSID_VALIDITY(tsid);


	printf("In:tsid=%d, smp = %d, pidcount= %d \n", tsid, smp, pids->count);

	//smp = 0;//only for test

	if (!pids) {
		EMSG("pids is NULL pointer");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	if (!pids->elems) {
		EMSG("pids elements is NULL pointer");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	if (pids->count <= 0) {
		EMSG("pids list is empty");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	TMSG("nvSprOpenDvbSession  tsid = %d smp=%d \n", tsid, smp);

	//if( tsid == 1) return ;

	WR_LOCK(&rwlock);

	if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");

		status = NV_SPR_ERROR;
		goto error;
	}

	ts = getTransportSessionById(tsid);
	if (ts == NULL) {
		DMSG("DVB on new tsid:%d", tsid);
		ts = newTransportSession(tsid);
		if (!ts) {
			status = NV_SPR_ERROR;
			goto error;
		}

		ts->sessionType = TRANSPORT_SESSION_TYPE_DVB;
		ts->tsPort = MT_UNF_DMX_PORT_BUTT;

		ts->playDmxId = mtSecGetPlatDmxId(tsid);
		//DMSG("map tsid(%d) to dmxid(%d)", tsid, ts->playDmxId);
	} else {
		DMSG("Warning: session of this id:%d is already running!", tsid);
		return NV_SPR_NO_ERROR;
	}

	status = parsePidList(pids, &ts->vidCodecType, &vidPid, &ts->audCodecType, &audPid);
	printf("nvSprOpenDvbSession  ts->audCodecType = %x , ts->vidCodecType = %d \n", ts->audCodecType, ts->vidCodecType );

	if (status != NV_SPR_NO_ERROR)
		goto delete_session;

	if (ts->tsPort == MT_UNF_DMX_PORT_BUTT) {

		MT_UNF_DMX_PORT_E port = MT_UNF_DMX_PORT_RAM_0;
		for (i = 0; i < MAX_PORT_NUM; i++) {

			ret = MT_UNF_DMX_AttachTSPort(ts->playDmxId, port);
			if (ret != MT_SUCCESS) {
				EMSG("attach the dvb port to the demux failed, return error = 0x%x", ret);

				status = NV_SPR_ERROR;
				goto delete_session;
			}

			ret = MT_UNF_DMX_CreateTSBuffer(port, DEMUX_TS_BUF_SIZE, &ts->tsBufHandle);

			if (ret != MT_SUCCESS) {

				if (ret == MT_ERR_DMX_RECREAT_TSBUFFER) {

					MT_UNF_DMX_DetachTSPort(ts->playDmxId);

					port += 1;
					DMSG("create TS buffer on port%d failed, try again", port);
				} else {
					EMSG("create TS buffer failed, error = 0x%x", ret);
					status = NV_SPR_ERROR;
					goto detach_ts_port;
				}

			} else {
				TMSG("create TS buffer on port%d success, tsBufHandle:%lx", port, ts->tsBufHandle);
				ts->tsPort = port;
				break;
			}
		}
	}


	ts->avPlayerStreamType = MT_UNF_AVPLAY_STREAM_TYPE_TS;
	ts->bUseSubLayer = getDemuxInfo( MT_UNF_DMX_CHAN_TYPE_VID );
	if( ts->bUseSubLayer == FALSE)
	{
		ts->bUseSubLayer = getAvPlayInfo();
	}
	mtSecSetIsMultiSessionFlag(ts->tsid, ts->bUseSubLayer);

	printf("dvb ts->bUseSubLayer = %d --------------------------------\n", ts->bUseSubLayer);

	status = openAvPlayer(ts, NULL, NULL);
	if (status != NV_SPR_NO_ERROR) {
		EMSG("open AV player failed");
		goto destroy_ts_buffer;
	}

	status = openWindow(ts);
	if (status != NV_SPR_NO_ERROR) {
		EMSG("open window failed");
		goto close_avplayer;
	}

	ret = MT_UNF_DMX_ResetTSBuffer(ts->tsBufHandle);
	if (ret != MT_SUCCESS) {
		EMSG("reset TS buffer failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		goto close_window;
	}

	pcrPid = vidPid;
	status = startAvPlayer(ts, ts->vidCodecType, vidPid, ts->audCodecType, audPid, pcrPid, syncRef);
	if (status != NV_SPR_NO_ERROR) {
		EMSG("start AV player failed");
		goto close_window;
	}

      ts->vidPid = vidPid;
	ts->audPid = audPid;
	ts->pcrPid = pcrPid;

       //allocate when injecting
	ts->tsBuffer = NULL;
	ts->tsBufferSize = DEMUX_TS_PUT_SIZE;
	ts->tsDataSize = 0;
	ts->tsIntBufferVir = NULL;

	if( ts->bUseSubLayer == TRUE)
	{
            mtSecSetProtectBuffer(MM_PIP_ZONEMMZ_START, MM_PIP_ZONEMMZ_SIZE, MB_SUB_VID);
            if (ts->tsid < 20) {//1-19 for DVB
                mtExtMTLZ_TEE_SMP_En(FALSE, MT_DVB_PLAY);
            }
	}
	else
	{
        #ifdef _MT_WITH_CAK_TEST //only for CAK test

            drv_disp_reset_hdmi_fmt_ability();
            testFormat = drv_disp_get_auto_hdmi_fmt();
            mtTestUpdateDisplayResolution(testFormat);
            printf("reset Resolution to UHD ? ret = 0x%x, testFormat = 0x%x", ret, testFormat);
        #endif

            MT_UNF_DMX_GetChannelHandle(ts->playDmxId, audPid, &audioChannel);
            MT_UNF_DMX_GetEsBuffPhyAddr(audioChannel, (ulong *)&esAudioBuffPhyAddr, &esBuffSize);
            mtSecSetProtectBuffer(esAudioBuffPhyAddr, esBuffSize, MB_AUD);
            //printf("esAudioBuffPhyAddr = 0x%lx, esBuffSize  = 0x%x \n", esAudioBuffPhyAddr, esBuffSize);
            mt_ngwm_configure_mainID(tsid);

            getSprMBInfoByType(MB_VID, &mb);
            mtSecSetProtectBuffer(mb.start, mb.size, MB_VID);
            #if 1
            //DVB play
            if (ts->tsid < 20) //1-19 for DVB
            {
                printf("%s, %d \n", __FUNCTION__, __LINE__);
                mtExtMTLZ_TEE_SMP_En(TRUE, MT_DVB_PLAY);
            } else {
                //OTT play
                getSprMBInfoByType(MB_OTT, &mb);
                printf("%s, %d \n", __FUNCTION__, __LINE__);
                mtSecSetProtectBuffer(mb.start, mb.size, MB_OTT);
                mtExtMTLZ_TEE_SMP_En(TRUE, MT_OTT_PLAY);
            }
            #endif
             pCurMain_ts = ts;
	}

	TSecPidInfo pxPidInfo;
	pxPidInfo.sessionType = ts->sessionType;
	pxPidInfo.pidNum = 2;
	for (i = 0; i < pxPidInfo.pidNum; i++)
	{
		pxPidInfo.pidList[i].pid = pids->elems[i].pid;
		pxPidInfo.pidList[i].type = toSecPidType(pids->elems[i].streamType);

		if( ts->bUseSubLayer  )
      	       {
      	      		pxPidInfo.pidNum = 1;
			pxPidInfo.pidList[0].pid = vidPid;
			pxPidInfo.pidList[0].type = SEC_PID_VIDEO;
		}
	}
	//mtSecSetSessionPid(tsid, &pxPidInfo, smp);//Project should set it SMP enable allways!
	mtSecSetSessionPid(tsid, &pxPidInfo, TRUE);//SMP allways on

       ts->videoDecodeStart = TRUE;
	WR_UNLOCK(&rwlock);


	printf("DVBSession(tsid:%d) Open Success", tsid);
	return NV_SPR_NO_ERROR;

close_window:
	closeWindow(ts);
close_avplayer:
	closeAvPlayer(ts);
destroy_ts_buffer:
	CHECK_CALL_MT_FUNC(MT_UNF_DMX_DestroyTSBuffer, ts->tsBufHandle);
	ts->tsBufHandle = MT_INVALID_HANDLE;
detach_ts_port:
	CHECK_CALL_MT_FUNC(MT_UNF_DMX_DetachTSPort, ts->playDmxId);
	ts->playDmxId = DEMUX_INVALID_ID;
delete_session:
	delTransportSession(ts);
error:
	WR_UNLOCK(&rwlock);

	return status;
}


mt_u8 *vir2phy(const mt_u8 *vir)
{
    mt_s32 ret = MT_SUCCESS;
    phys_addr_t phy_addr;
    ulong phy_size;
    //SECAPI_INFO("vir = 0x%lx \n", vir);
    ret = mt_mmz_get_phyaddr((mt_void *)vir, &phy_addr, &phy_size);
    if (ret == MT_SUCCESS) {
       // SECAPI_INFO("Got vir = 0x%lx, phyaddr:%x\n", vir, phy_addr);
        return (mt_u8 *)phy_addr;
    }
    else {
		ret = mt_mem_get_phyaddr((mt_void *)vir, &phy_addr);
		if (ret == MT_SUCCESS) {
	       // SECAPI_INFO("Got vir = 0x%lx, phyaddr:%x\n", vir, phy_addr);
	        return (mt_u8 *)phy_addr;
	     }

	 SECAPI_ERROR("Get phyaddr by viraddr(%lx) failed", (ulong)vir);
        return NULL;
    }
}


static const TNvSprStatus mtSprInputDataRawEmiProcess(struct transportSession *ts,
														const TUnsignedInt8 *data,
														TUnsignedInt32 size)
{
	transportSessionType sessionType = ts->sessionType;
	TUnsignedInt16 emi = ts->emi;

	TNvSprStatus ret = NV_SPR_NO_ERROR;

	//TUnsignedInt8 *phyAddr = NULL;


	/*RAW-EMI decryption:
	 * - If input buffer is nvSprAllocateMemory() or mt_unf_cipher_malloc() allocated buffer(Hardware-Buffer), get physical address, and process;
	 * - If input buffer is a user-space buffer, allocate a Hardware-Buffer, copy input data, then process.
	 */
    if (sessionType == TRANSPORT_SESSION_TYPE_REPLAY) {
//	printf("%s %d emi = %x \n",__FUNCTION__,__LINE__, emi);

#if 1 //Processing RAW EMIs 0x4020 0x4021 0x4023  by M2M
        if (emi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
                || emi == NOCS_EMI_AES128_ECB_TAIL_CLEAR
                || emi == NOCS_EMI_AES128_CBC_PKCS7_PADDING) {
#else //Processing all EMIs by M2M
        if (emi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
                || emi == NOCS_EMI_AES128_ECB_TAIL_CLEAR
                || emi == NOCS_EMI_AES128_CBC_PKCS7_PADDING
				|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR
				|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR
				|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1){
				|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA) {
#endif
#if 0 //clear stream replay test
            memcpy((void *)(ts->tsBuffer + ts->tsDataSize), (const void *)data, size);
#else
			//TODO:
            //copy into continuity buffer


            memcpy(ts->tsIntBufferVir, data, size);

            //DMA copy into protected buffer for injecting(after being decrypted)
            //phyAddr = ts->tsBufferPhy + ts->tsDataSize;

            //Intermediate Buffer --> Local TSBuffer (CID0 --> CID5)

           // if (0 != mtSecRawStreamDataProcess(ts->tsid, SESSION_OP_DECRYPT, size, ts->tsIntBufferPhy, phyAddr)) {
           if (0 != mtSecRawStreamDataProcess(ts->tsid, SESSION_OP_DECRYPT, size, ts->tsIntBufferVir, ts->tsBuffer+ts->tsDataSize)) {

                EMSG("Raw stream data process failed");

				ret = NV_SPR_ERROR;

                goto end;
            }
#endif

        } else {
			//TMSG("copy stream data directly size = %d \n", size);
			//TODO:If using M2M to decrypt TS data, should call M2M API first to decrypt data to tsBuffer directly

            memcpy((void *)(ts->tsBuffer + ts->tsDataSize), (const void *)data, size);

        }
    }
end:

	return ret;
}

#if 0 //unused
static TUnsignedInt32 mtSprInputDataProcess(struct transportSession *ts, const TUnsignedInt8 *data, TUnsignedInt32 size)
{
	transportSessionType sessionType = ts->sessionType;
	TUnsignedInt16 emi = ts->emi;
	TUnsignedInt8 *curTsPos = NULL;

	TUnsignedInt32 size_processed = 0;

	if (sessionType == TRANSPORT_SESSION_TYPE_REPLAY) {

		//hex_dump("dump in data", data, (size > 16 ? 16 : size));
		/*RAW-EMI decryption:
		 * - If input buffer is nvSprAllocateMemory() or mt_unf_cipher_malloc() allocated buffer(Hardware-Buffer), get physical address, and process;
		 * - If input buffer is a user-space buffer, allocate a Hardware-Buffer, copy input data, then process.
		 */
		if (emi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
			|| emi == NOCS_EMI_AES128_ECB_TAIL_CLEAR
			|| emi == NOCS_EMI_AES128_CBC_PKCS7_PADDING) {
			curTsPos = ts->tsBuffer + ts->tsDataSize;
			memcpy((void *)curTsPos, (const void *)data, size);
			size_processed = size;
		} else if (emi == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA
				   || emi == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR
				   || emi == NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR
				   || emi == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1) {
			curTsPos = ts->tsBuffer + ts->tsDataSize;
			memcpy((void *)curTsPos, (const void *)data, size);
			size_processed = size;
		} else {
			//TODO:how about other EMIs ?
		}
	} else if (sessionType == TRANSPORT_SESSION_TYPE_RECORD) {
		//DMSG("This is record session, inject size:%d\n", size);
		//data for play
		curTsPos = ts->tsBuffer + ts->tsDataSize;
		memcpy((void *)curTsPos, (const void *)data, size);
		size_processed = size;
	} else {
		if (emi == NOCS_EMI_AES128_CBC_PKCS7_PADDING) {
			/*
			 * DVB session but HLS streaming, input data is already decrypted by opaqueData
			 * Using dmacpy to move data whether smp is on or off.
			 */
			curTsPos = vir2phy(ts->tsBuffer) + ts->tsDataSize;
			dmacpy(MT_EDMA_CH_0, (mt_u8 *)(curTsPos), (const mt_u8 *)vir2phy(data), size);
			size_processed = size;
		} else {
			//DVB sessions which data is to be descrambled.
			curTsPos = ts->tsBuffer + ts->tsDataSize;
			memcpy((void *)curTsPos, (const void *)data, size);
			size_processed = size;
		}
	}

	return size_processed;
}
#endif


#define RETRIES_TOTAL 50 //300
//#define RETRIES_TOTAL 2


static TNvSprStatus nvSprDvbInjectData(struct transportSession *ts, TUnsignedInt8 *data, TUnsignedInt32 size)
{
	TNvSprStatus status = NV_SPR_NO_ERROR;
	MT_UNF_STREAM_BUF_S dmxBuf = {0};
	mt_u32 getBufRetries = 0;
	mt_s32 ret = MT_FAILURE;
	mt_u32 freeSize = 0;
//	MT_UNF_AVPLAY_STATUS_INFO_S aud_info = {0};
//	MT_UNF_AVPLAY_STATUS_INFO_S vid_info = {0};

//      	TMSG("nvSprDvbInjectData In:size:%d, dataAddr:%x", size, data);


	if (ts->isClosing) {
		return status;//This session is in closing, do not inject data anymore.
	}

    if (ts->tsBuffer == NULL) {
        ts->tsBuffer = (TUnsignedInt8 *)mmz_malloc("ddr", DEMUX_TS_PUT_SIZE, SMP_DDR_ALIGN_UNIT);
        ts->tsBufferPhy = vir2phy(ts->tsBuffer);
        //TMSG(" cipher malloc 0x%p, 0x%p ", ts->tsBuffer, ts->tsBufferPhy);
        if (!ts->tsBuffer) {
            EMSG("malloc stream buffer failed");

            status = NV_SPR_ERROR;
            goto end;
        }
        ts->tsBufferSize = DEMUX_TS_PUT_SIZE;
        ts->tsDataSize = 0;
    }

#if 0//def FILE_DUMP_VIDEO
	{
		DMSG("write size=%d into test file...", size);
		write_test_file(gVfp, (TUnsignedInt8 *)(data), size);
	}
#endif

	//check if input data starts with 0x47
	if (0x47 != data[0]) {
		//EMSG("Input data is not TS-Pack, drop it...  data[0] = 0x%x ", data[0]);
		goto end;
	}
#if 1
	if (ts->smpEnabled) {
		if (ts->smpSetFlag) {
			//Re-stop-start avplayer for Video disorder when SMP enabled during playing, Because the stream injected earlier does not have permission
			if (ts->sessionType == TRANSPORT_SESSION_TYPE_DVB
				|| ts->sessionType == TRANSPORT_SESSION_TYPE_REPLAY) {

#ifdef STOP_WITH_STILL_FRAME
				DMSG("closing window[No, STILL test]");
#else
				CHECK_CALL_MT_FUNC(MT_UNF_DISP_VidLayerShow, MT_FALSE);
#endif
//				CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_Reset, ts->avPlayer, NULL);
				printf("startAvPlayer----------------------------------------------------------------------\n");
				stopAvPlayer(ts);
				status = startAvPlayer(ts, ts->vidCodecType, ts->vidPid, ts->audCodecType, ts->audPid, ts->pcrPid, MT_UNF_SYNC_REF_NONE);
				if (status != NV_SPR_NO_ERROR) {
					EMSG("re-start AV player failed,do nothing");
				}

				CHECK_CALL_MT_FUNC(MT_UNF_DISP_VidLayerShow, MT_TRUE);

				ts->smpSetFlag = FALSE;
				goto end;
			} else {
				DMSG("sessionType:%d[Recording]", ts->sessionType);
				ts->smpSetFlag = FALSE;
			}
		}
	}
#endif
		//printf("dvb inject  data ...   ts->tsDataSize + size = %d  DEMUX_TS_PUT_SIZE= %d \n", ts->tsDataSize + size, DEMUX_TS_PUT_SIZE);

	do {
		if (ts->tsDataSize + size < DEMUX_TS_PUT_SIZE) {
			memcpy((void *)ts->tsBuffer + ts->tsDataSize, (const void *)data, size);
			ts->tsDataSize += size;
			status = NV_SPR_NO_ERROR;
			goto end;
		}
		//printf("dvb inject  data ...       999999\n");


		freeSize = DEMUX_TS_PUT_SIZE - ts->tsDataSize;
		if (freeSize) {
			//DMSG("freeSize:%d", freeSize);
			memcpy((void *)ts->tsBuffer + ts->tsDataSize, (const void *)data, freeSize);

			ts->tsDataSize = DEMUX_TS_PUT_SIZE;

			data += freeSize;
			size -= freeSize;
		}

		if (ts->sessionType == TRANSPORT_SESSION_TYPE_DVB
			|| ts->sessionType == TRANSPORT_SESSION_TYPE_REPLAY) {

			//check audio es data level
#if 0
			getBufRetries = 0;
			do {
				ret = MT_UNF_AVPLAY_GetAudioStatusInfo(ts->avPlayer, &aud_info);
				if (ret == MT_SUCCESS) {
					//DMSG("AUD.u32UsedSize=%x", aud_info.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize);
					//DMSG("AUD.u32BufSize=%x", aud_info.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufSize);
					//DMSG("AUD.u32BufRptr=%x", aud_info.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufRptr);
					//DMSG("AUD.u32BufWptr=%x", aud_info.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufWptr);
					if (aud_info.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize < (64 * 1024)) {
						break;
					}
				} else {
					//Failed, do nothing
					EMSG("GetAudioStatusInfo failed\n");
					break;
				}
				//DMSG("Audio ES Data Level High, waiting...");

				//MT_USLEEP(8000);
				MT_USLEEP(8000);
				getBufRetries++;
			} while (getBufRetries < (RETRIES_TOTAL * 10));
#endif
			//check video es data level
			/*getBufRetries = 0;
			do {
				ret = MT_UNF_AVPLAY_GetVideoStatusInfo(ts->avPlayer, &vid_info);
				if (ret == MT_SUCCESS) {
					//DMSG("VID.u32UsedSize=%x", vid_info.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize);
					//DMSG("VID.u32BufSize=%x", vid_info.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize);
					//DMSG("VID.u32BufRptr=%x", vid_info.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufRptr);
					//DMSG("VID.u32BufWptr=%x", vid_info.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufWptr);
					if (vid_info.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize*5 < vid_info.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize*4) {
						break;
					}
				} else {
					//Failed, do nothing
					EMSG("GetVideoStatusInfo failed\n");
					break;
				}
				printf("tsid:%d   Video ES Data Level High, waiting... u32UsedSize = %d  u32BufSize = %d \n",
					ts->tsid, vid_info.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize, vid_info.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize);

				//MT_USLEEP(8000);
				MT_USLEEP(8000);
				getBufRetries++;
			} while (getBufRetries < (RETRIES_TOTAL * 10));*/
		} else {
#if 0
			//Recording sesssion
			MT_UNF_PVR_REC_STATUS_S recStatus = {0};
			getBufRetries = 0;
			do {
				ret = MT_UNF_PVR_RecGetStatus(ts->recordChannelId, &recStatus);
				if (ret == MT_SUCCESS) {
					DMSG("[DoNothing]recStatus.stRecBufStatus.u32BufSize=%x, .u32UsedSize=%x",
						 recStatus.stRecBufStatus.u32BufSize, recStatus.stRecBufStatus.u32UsedSize);
					//MT_USLEEP(8000);
					break;
				} else {
					EMSG("PVR_RecGetStatus failed, do nothing");
					break;
				}
			} while (getBufRetries < (RETRIES_TOTAL * 10));
#endif
		}

		/* Stream buffer is full, so put the data into the demux */
		getBufRetries = 0;
		do {
			//ret = MT_UNF_DMX_GetTSBuffer(ts->tsBufHandle, ts->tsDataSize, &dmxBuf, 100);
			_lockTsBuffer();
			ret = MT_UNF_DMX_GetTSBuffer(ts->tsBufHandle, ts->tsDataSize, &dmxBuf, 10);
                    dmxBuf.u32PhyData = (phys_addr_t)vir2phy(dmxBuf.pu8Data);
			_unlockTsBuffer();
			if (ret == MT_SUCCESS)
				break;
			//printf("fail\n");
			getBufRetries++;

			MT_USLEEP(1000);

		} while (getBufRetries < (RETRIES_TOTAL * 150));
		if (getBufRetries >= (RETRIES_TOTAL * 150)) {
			EMSG("GetTSBUffer timeout, Buffer overflow(ret=%x)  ts->tsDataSize =%x tsid =%d \n", ret, ts->tsDataSize, ts->tsid);
			status = NV_SPR_ERROR_TIMEOUT;
			goto end;
		}

		memcpy((void *)(dmxBuf.pu8Data), (const void *)(ts->tsBuffer), ts->tsDataSize);
            //dma4 is for General RCID 0/2/3 WCID=RCID
            //dmacpy(MT_EDMA_CH_4, (mt_u8 *)(dmxBuf.u32PhyData), (ts->tsBufferPhy), ts->tsDataSize);

		//TMSG("ts->tsid =%d, ts->tsBufHandle =%x  [tsid:%d]put size:%d ", ts->tsid, ts->tsBufHandle,  ts->tsid, ts->tsDataSize);
		_lockTsBuffer();
		ret = MT_UNF_DMX_PutTSBuffer(ts->tsBufHandle, ts->tsDataSize);
		_unlockTsBuffer();
		if (ret != MT_SUCCESS) {
			EMSG("inject failed, return error = 0x%x", ret);

			status = NV_SPR_ERROR;
			goto end;
		}

		ts->tsDataSize = 0;
	} while (size);

	status = NV_SPR_NO_ERROR;
end:
	return status;
}


static TNvSprStatus nvSprReplayInjectData(struct transportSession *ts, TUnsignedInt8 *data, TUnsignedInt32 size)
{
	TNvSprStatus status = NV_SPR_NO_ERROR;
    phys_addr_t phyAddrTsBuf = 0;
	MT_UNF_STREAM_BUF_S dmxBuf = {0};
	mt_s32 ret = MT_FAILURE;
	mt_u32 getBufRetries = 0;
	mt_u32 freeSize = 0;
	mt_u16 emi = ts->emi;

	mt_u32 dmx_rep_put_size = ((emi & 0xFF00) ? DEMUX_RAW_REP_PUT_SIZE : DEMUX_TS_REP_PUT_SIZE);

       if (size == 0) {
            goto end;
       }

	if (ts->isClosing) {
		//printf("%s %d \n",__FUNCTION__,__LINE__);

		return status;
	}
/*
if (ts->tsid == 5) {
usleep(1000 * 10);
status = NV_SPR_NO_ERROR;
goto end;
}
*/
//	printf("%s %d  size =%d ts->tsDataSize = %d dmx_rep_put_size = %d  \n",__FUNCTION__,__LINE__, size, ts->tsDataSize, dmx_rep_put_size);
	//printf("IN %d %d\n", ts->tsid, size);



	do {
#if 0
		if (ts->tsDataSize + size < dmx_rep_put_size) {


              	mtSprInputDataRawEmiProcess(ts, data, size);


			ts->tsDataSize += size;
			status = NV_SPR_NO_ERROR;
			//printf("%s %d \n",__FUNCTION__,__LINE__);

			goto end;
		}
 #else
 		if (ts->tsDataSize + size < dmx_rep_put_size) {


              	mtSprInputDataRawEmiProcess(ts, data, size);


			ts->tsDataSize += size;
			status = NV_SPR_NO_ERROR;
			//printf("%s %d \n",__FUNCTION__,__LINE__);
                    //MT_USLEEP(1000); //for 23. [A](3005)ISecStreamSession Watch and Record. Otherwise, DMX TS Buffer will be overflow, and case second TS cannot be descramble before case finish.

			goto end;
		}
               //for TALTS test, can use it to check replay
              //TMSG("id = %d, %d \n", ts->tsid, ts->tsDataSize);

#endif
		//printf("IN %d\n", ts->tsid);

		freeSize = dmx_rep_put_size - ts->tsDataSize;
		//printf("%s %d freeSize = %d \n",__FUNCTION__,__LINE__, freeSize);

		if (freeSize) {
			//DMSG("freeSize:%d, size:%d", freeSize, size);
			//mtSprInputDataRawEmiProcess(ts, data, freeSize);
			//mtSprInputDataRawEmiProcess(ts, data, size);
			//printf("%s %d \n",__FUNCTION__,__LINE__);

			mtSprInputDataRawEmiProcess(ts, data, freeSize);
			//printf("%s %d \n",__FUNCTION__,__LINE__);

			ts->tsDataSize = dmx_rep_put_size;

			data += freeSize;
			size -= freeSize;
			//printf("%s %d \n",__FUNCTION__,__LINE__);

			//ts->tsDataSize += size;
			//size = 0;
		}

		/* Stream buffer is full, so put the data into the demux */
		getBufRetries = 0;
/*
if (ts->tsid == 5) {
    usleep(1000 * 10);
    status = NV_SPR_NO_ERROR;
    goto end;
}
*/
		do {

			//ret = MT_UNF_DMX_GetTSBuffer(ts->tsBufHandle, ts->tsDataSize, &dmxBuf, 10);
			ret = MT_UNF_DMX_GetTSBufferEx(ts->tsBufHandle, ts->tsDataSize, &dmxBuf, &phyAddrTsBuf, 10);
			if (ret == MT_SUCCESS)
				break;

			getBufRetries++;
			//printf("[NOTE]%s %d \n",__FUNCTION__,__LINE__);
                    MT_USLEEP(1000); //add for SPS-3005 slow the speed of first tsid inject data.

		} while (getBufRetries < (RETRIES_TOTAL * 150));
		//} while (getBufRetries < (RETRIES_TOTAL * 500));

		if (getBufRetries >= (RETRIES_TOTAL * 150)) {
			EMSG("injection is blocked, tsDataSize:%d ts->tsid = %d ", ts->tsDataSize, ts->tsid);

			status = NV_SPR_ERROR_TIMEOUT;
			//printf("[NV_SPR_ERROR_TIMEOUT]%s %d \n",__FUNCTION__,__LINE__);

			goto end;
		}
		//printf("%s %d \n",__FUNCTION__,__LINE__);

		//DMSG("EDMA_CH2 copy size:%d", ts->tsDataSize);
		if (emi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
			|| emi == NOCS_EMI_AES128_ECB_TAIL_CLEAR
			|| emi == NOCS_EMI_AES128_CBC_PKCS7_PADDING) {
			//printf("%s %d ts->tsDataSize = %d \n",__FUNCTION__,__LINE__, ts->tsDataSize);

			dmacpy(MT_EDMA_CH_2, (mt_u8 *)(phyAddrTsBuf), (const void *)(ts->tsBufferPhy), ts->tsDataSize);
			//printf("%s %d \n",__FUNCTION__,__LINE__);

		} else if (emi == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA
				   || emi == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR
				   || emi == NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR
				   || emi == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1) {

			//If data is processed by descrambler:
			//memcpy((void *)(dmxBuf.pu8Data), (const void *)(ts->tsBuffer), ts->tsDataSize);
			//If data is processed by M2M:

			dmacpy(MT_EDMA_CH_2, (mt_u8 *)(phyAddrTsBuf), (const void *)(ts->tsBufferPhy), ts->tsDataSize);
			//TMSG("%s %d ts->tsDataSize = 0x%x \n",__FUNCTION__,__LINE__, ts->tsDataSize);

		} else {
			EMSG("FATAL:What is this EMI:%x for? Not suppose to be here!", emi);
		}

		//DMSG("put size:%d", ts->tsDataSize);
		//ret = MT_UNF_DMX_PutTSBuffer(ts->tsBufHandle, ts->tsDataSize);
		//printf("%s %d \n",__FUNCTION__,__LINE__);

		ret = MT_UNF_DMX_PutTSBufferEx(ts->tsBufHandle, ts->tsDataSize, 0);
		//printf("%s %d \n",__FUNCTION__,__LINE__);

		if (ret != MT_SUCCESS) {
			EMSG("inject failed, return error = 0x%x", ret);

			status = NV_SPR_ERROR;
			goto end;
		}

		ts->tsDataSize = 0;
	} while (size);
	//printf("OUT  = %d \n", ts->tsid);

	status = NV_SPR_NO_ERROR;
end:

	return status;

}

TNvSprStatus nvSprInjectData(TTransportSessionId tsid, TUnsignedInt32 size, TUnsignedInt8 *data)
{
	TNvSprStatus status = NV_SPR_NO_ERROR;
	struct transportSession *ts;

	CHECK_TSID_VALIDITY(tsid);

	//TMSG("InjectData In:size:%d, tsid:%d", size, tsid);
       //printf("IN%d \n", tsid);
       //write_test_file(gtsfp, (TUnsignedInt8 *)(data), size);

	//if( tsid >1) return NV_SPR_NO_ERROR;//only for test

	if (size > 0 && !data) {
		EMSG("the injected data buffer is invalid");
		return NV_SPR_ERROR_BAD_PARAM;
	}


#if 1//解决SEC-SPS-3002 回播主画面卡顿问题
	if( (tsid & 1) ==0 )
		WR_LOCK(&rwlock);
	else
		WR_LOCK(&rwlock2);

#else
WR_LOCK(&rwlock);
#endif

	if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");

		status = NV_SPR_ERROR;
		goto end;
	}

	ts = getTransportSessionById(tsid);
	if (!ts) {
		//EMSG("Session id(%d) isn't existed or closed", tsid);

		status = NV_SPR_ERROR_BAD_PARAM;
		goto end;
	}

#if 0//
	if( ts->sessionType == TRANSPORT_SESSION_TYPE_REPLAY &&  ts->bUseSubLayer == FALSE)
		MT_USLEEP(2000);//MT_USLEEP(5000);//SEC-SPS-3005 回播的时候:不加延时的时候，第1路注入速度是第2路的5倍，所以给第1路注入加了延时
#endif

	/*
	 * Check if sec stream session is already closed.
	 * cak Pause-Resume scenario:
	 *  When caPause is called, secStreamSessionClose is also called,
	 *  but simbad never calls nvSprSessionClose, so nvSprInjectData
	 *  never stops injecting data. It causes blocking issue. We
	 *  get sec stream session status to fix this.
	 */
#if 0 //But how about clear stream?
	if (mtSecGetSessionClosed(tsid, SESSION_OP_DECRYPT)) {
		DMSG("SecStreamSession(tsid:%d) is closed", tsid);
		//Do nothing
		goto end;
	}
#endif

//    TMSG("tsid:%d sesstionType:%d", tsid, ts->sessionType);
    if (ts->sessionType == TRANSPORT_SESSION_TYPE_DVB) {
#if 1
        if (mtSecGetSessionEmi(ts->tsid, SESSION_OP_DECRYPT) == NOCS_EMI_AES128_CBC_PKCS7_PADDING) {
			//OTT_4023 need this:188 aligned,remove padding part
			TMSG("raw size:%d\n", size);
			size = pkcs7_padding_data_length(data, size, 16);
			TMSG("padding removed size:%d\n", size);
			TUnsignedInt32 injCount = 0;
			TUnsignedInt32 injRemain = 0;
			TUnsignedInt32 i = 0;
			if (size > DEMUX_TS_PUT_SIZE) {
				injCount = size / DEMUX_TS_PUT_SIZE;
				for (i = 0; i < injCount; i++) {
					status = nvSprDvbInjectData(ts, data + (i * DEMUX_TS_PUT_SIZE), DEMUX_TS_PUT_SIZE);
				}
				injRemain = size % DEMUX_TS_PUT_SIZE;
				if (injRemain) {
					status = nvSprDvbInjectData(ts, data + (i * DEMUX_TS_PUT_SIZE), injRemain);
				}
			} else {
				status = nvSprDvbInjectData(ts, data, size);
			}

        } else {
//	     TMSG("DvbInjectData size:%d\n", size);
            status = nvSprDvbInjectData(ts, data, size);
        }
#else
		//This is for progra-switch stress test
            status = nvSprDvbInjectData(ts, data, size);
#endif
    } else if (ts->sessionType == TRANSPORT_SESSION_TYPE_REPLAY) {
    status = nvSprReplayInjectData(ts, data, size);
    } else if (ts->sessionType == TRANSPORT_SESSION_TYPE_RECORD) {
        //For injecting, same as Dvb session
		//TMSG("TRANSPORT_SESSION_TYPE_RECORD padding removed size:%d\n", size);
		status = nvSprDvbInjectData(ts, data, size);
		//DMSG("padding removed size:%d\n", size);
    } else {
        //TODO
    }

end:

	#if 1//解决SEC-SPS-3002 回播主画面卡顿问题
	if( (tsid % 2) ==0 )
		WR_UNLOCK(&rwlock);
	else
		WR_UNLOCK(&rwlock2);
	#else
	WR_UNLOCK(&rwlock);
	#endif

	return status;
}

static mt_s32 recordingWriteCallback(MT_UNF_PVR_DATA_ATTR_S *attr,
                                     mt_u8 *destVirAddr, ulong destPhyAddr,
                                     mt_u8 *srcVirAddr, ulong srcPhyAddr,
                                     mt_u32 offset,
									 mt_u32 *data_size)
{
	mt_s32 ret = MT_FAILURE;
	mt_u32 tsid;
	struct transportSession *ts = NULL;
	MT_UNF_PVR_REC_ATTR_S stRecAttr = {0};
	TUnsignedInt16 emi;
//	phys_addr_t phyaddr_src, phyaddr_dest;
	//MT_UNF_DMX_RECBUF_STATUS_S rec_buf_status;


#if 0
	//record clear stream for test: set bDoCipher=FALSE, bSupportAdvCa=0
	//DMSG("record clear stream for playback test.");
	memcpy(destVirAddr, srcVirAddr, *data_size);
	DMSG("Do nothing but return sucess.");
	return MT_SUCCESS;
#endif

    if (pthread_rwlock_wrlock(&callback_rwlock)) {
    	EMSG("apply a callback lock failed");
    	return ret;
    }

	//DMSG("CB In\n");
	//WR_LOCK(&rwlock);

	ret = MT_UNF_PVR_RecGetChn(attr->u32ChnID, &stRecAttr);
	if (MT_SUCCESS != ret) {
		EMSG("Get Chn by ChnID:%d failed\n", attr->u32ChnID);
		//WR_UNLOCK(&rwlock);
		return MT_FAILURE;
	}

	tsid = mtSecGetTsIdByDmxId(stRecAttr.u32DemuxID);
	TMSG("chan=0x%x, RecAttr.DmxId:%d, Got tsid:%d, offset = 0x%x", attr->u32ChnID, stRecAttr.u32DemuxID, tsid, offset);

	/*
        if( tsid == 2)
            write_test_file(gtsfp, (TUnsignedInt8 *)(srcVirAddr), *data_size);
	*/

	ts = getTransportSessionById(tsid);
	if (NULL == ts) {
		//WR_UNLOCK(&rwlock);
		EMSG("Transport Session not found or closed by tsid:%d, it may last section data , that the secStreamSession is closed!", tsid);

            if (pthread_rwlock_unlock(&callback_rwlock)) {
                 EMSG("release callback lock failed");
            }
            TMSG("=addr= 0x%x=\n", data_size);

		return MT_ERR_PVR_REC_CB_FAIL_DROP_DATA;
	}


	emi = ts->emi;
       //printf("recordingWriteCallback ---------------------------------------------------------- tsid = %d  data_size= %d \n", tsid, *data_size);


	if (
	emi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
	|| emi == NOCS_EMI_AES128_ECB_TAIL_CLEAR
	|| emi == NOCS_EMI_AES128_CBC_PKCS7_PADDING
	|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA
	|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR
	|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR
	|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1) {
/*
            if( dmacpy(MT_EDMA_CH_2,(mt_u8 *) grantTestBufferPhy,srcPhyAddr, *data_size) !=MT_SUCCESS )// smp on noly can use MT_EDMA_CH_1
            {
                printf("dmacpy MT_EDMA_CH_2....fail!  \n");
            }
            if ((gwriteFileVfp != NULL) && (grantTestBuffer[0] == 0x47)) {
                grant_testsize = fwrite(grantTestBuffer, 1, *data_size, gwriteFileVfp);
                printf("sr=0x%x, des=0x%x, size=0x%x grant_testsize=0x%x \n", srcPhyAddr, grantTestBufferPhy, *data_size, grant_testsize);

            }
*/
        	//DMSG("mtSetProcessing... In");
        	//RAW-EMI encryption
        	TMSG(" srcPhyAddr=%x destPhyAddr=%x *data_size = 0x%x, emi=0x%x \n", srcPhyAddr,destPhyAddr, *data_size, emi);
              ret = mtSecRawStreamDataProcess_phy(tsid, SESSION_OP_ENCRYPT, *data_size, (const TUnsignedInt8 *)srcPhyAddr, (TUnsignedInt8 *)destPhyAddr);
              if (ret == -8) //for test, it means record is not allowed!
              {
                    EMSG("should delet ts file?");
                    //What should I do?
                    return MT_ERR_PVR_REC_CB_FAIL_DROP_DATA;
              }
               if (ret == -4) //for test, it means record is not allowed!
              {
                    EMSG(" session is NULL now");
                    //What should I do?
                    if (pthread_rwlock_unlock(&callback_rwlock)) {
                                EMSG("release callback lock failed");
                    }
            		return MT_ERR_PVR_REC_CB_FAIL_DROP_DATA;
              }
 /*
              if (((emi & 0xFF00) == 0) && (destVirAddr[0] != 0x47)) {
                    if (pthread_rwlock_unlock(&callback_rwlock)) {
                                EMSG("release callback lock failed");
                    }
                    EMSG("TS data format wrong");
            		return MT_ERR_PVR_REC_CB_FAIL_DROP_DATA;
              }
*/
            	while (0 != ret) {
        		EMSG("mtSecRawStreamDataProcess_phy Raw stream data encryption process failed(Key may not set YET!!!tsid:%d  data_size = %d srcVirAddr=%lx destVirAddr=%lx srcPhyAddr=%lx destPhyAddr=%lx emi = %x)",
        			    tsid, *data_size, (ulong)srcVirAddr, (ulong)destVirAddr,srcPhyAddr,destPhyAddr, emi);
                    //goto end;
        		usleep(100* 1000);
        		ts = getTransportSessionById(tsid);//这里有可能是录制结束，Crypto closed，最后一段报上来的数据。
        		if (NULL == ts) {
        			//WR_UNLOCK(&rwlock);
        			EMSG("Transport Session not found or closed by tsid:%d", tsid);

                            if (pthread_rwlock_unlock(&callback_rwlock)) {
                                    EMSG("release callback lock failed");
                            }
                            EMSG("==\n");
                           return MT_ERR_PVR_REC_CB_FAIL_DROP_DATA;//MT_ERR_PVR_REC_CB_FAIL_DROP_DATA;
        		}
        		else if(ts->isClosing )
        		{
        			//WR_UNLOCK(&rwlock);

                            if (pthread_rwlock_unlock(&callback_rwlock)) {
                                    EMSG("release callback lock failed");
                            }
                            EMSG("==\n");
                		return MT_ERR_PVR_REC_CB_FAIL_DROP_DATA;//MT_ERR_PVR_REC_CB_FAIL_DROP_DATA;
                	}
        		//WR_UNLOCK(&rwlock);

                        if (pthread_rwlock_unlock(&callback_rwlock)) {
                            EMSG("release callback lock failed");
                        }
                            EMSG("==\n");
            		return MT_ERR_PVR_REC_CB_FAIL_DROP_DATA;
    		}

	} else {

		EMSG("TODO:To be implemented!");
             return MT_ERR_PVR_REC_CB_FAIL_DROP_DATA;
	}

	//DMSG("<<<<<<<<CB Out, Data recorded, size:%d>>>>>>>>", *data_size);
	//WR_UNLOCK(&rwlock);
record_end:
    if (pthread_rwlock_unlock(&callback_rwlock)) {
      EMSG("release callback lock failed");
    }
	return MT_SUCCESS;
}


mt_void MTADP_PVR_CallBack(mt_u32 u32ChnID, MT_UNF_PVR_EVENT_E EventType, mt_s32 s32EventValue, mt_void *args)
{
//    MT_UNF_PVR_PLAY_MODE_S mode;
 //   MT_UNF_PVR_PLAY_POSITION_S stPos = { 0 };
    MBInfo mb = {0};
    mt_u8 pvrRecodChan = 0xFF;
    //struct transportSession *ts = NULL;

    //printf("==============call back================\n");

    if (EventType > MT_UNF_PVR_EVENT_REC_RESV)
    {
        printf("====callback error!!!\n");
        return;
    }

    //printf("====channel     %d\n", u32ChnID);
    //printf("====event value %d\n", s32EventValue);

    if (EventType == MT_UNF_PVR_EVENT_REC_DMX_CREATE)
    {
        //printf("======pvr rec demux channel=0x%x create.====\n",s32EventValue);
        MT_UNF_DMX_RECBUF_STATUS_S stStatus;
        MT_S32 ret;
        ret = MT_UNF_DMX_GetRecBufferStatus((mt_handle)s32EventValue, &stStatus);
        if (MT_SUCCESS == ret)
        {
              pvrRecodChan = s32EventValue & 0xFF;
              printf("666 ===u32ChnID=0x%x=pvrRecodChan=%d=s32EventValue=0x%x, u32BufPhyAddr=0x%llx u32BufSizeAlign=0x%x\n", u32ChnID,pvrRecodChan, s32EventValue, stStatus.u32BufPhyAddr,stStatus.u32BufSizeAlign);
		//getSprMBInfoByType(MB_REC, &mb);
		mb.start =  stStatus.u32BufPhyAddr;
		mb.size = stStatus.u32BufSizeAlign;
		printf("MTADP_PVR_CallBack record_pip_flg = %d, mt_dmxSoftInject_index = %d  \n", record_pip_flg, mt_dmxSoftInject_index);

		switch(mt_dmxSoftInject_index) {//set PVR_DEC SMP buffer, index number is from dmux ID
                case 0:
                    mtSecSetProtectBuffer(mb.start, mb.size, MB_REC);
                    break;
                case 1:
                    mtSecSetProtectBuffer(mb.start, mb.size, MB_REC_1);
                    break;
                case 2:
                    mtSecSetProtectBuffer(mb.start, mb.size, MB_REC_2);
                    break;
                case 3:
                    mtSecSetProtectBuffer(mb.start, mb.size, MB_REC_3);
                    break;
                 default:
                    printf("=====mt_dmxSoftInject_index is wrong: 0x%x!\n", mt_dmxSoftInject_index);
                    break;
		}
             if (pvrRecodChan < MAX_DMX_REC_CHAN)// only support 4 channels
             {
                mtSecSetRecChanByDmxID(MT_UNF_DMX_PORT_RAM_0 + mt_dmxSoftInject_index, pvrRecodChan);
             } else {
                printf("pvr dmxID or record channel error: %d, %d \n", mt_dmxSoftInject_index, pvrRecodChan);
             }

        }
        else
        {
            printf("========get dmx rec buffer status failed!\n");
        }

    }


    //printf("=======================================\n\n");

	#if 0
	ts = getTransportSessionByRecordId(u32ChnID);
	if (ts != NULL) {
	    while(mtSecGetRecDscHandle( ts->tsid )==0)
	    {
	    	  printf("wait rec des handle ...\n");
	    	  usleep(50* 100);
	    }
	}
	#endif

    return;
}

TNvSprStatus nvSprOpenRecording(TTransportSessionId tsid, TNvSprPidList *pids, TUnsignedInt16 emi,
                                TBoolean smp, const char *filename)
{
	mt_s32 ret;
	TNvSprStatus status = NV_SPR_NO_ERROR;
	struct transportSession *ts = NULL;
       mt_u32 vidPid = 0;
       mt_u32 audPid = 0;
	mt_u32 pcrPid = 0;
	int i = 0;
       mt_handle videoChannel = 0;
       MT_UNF_DMX_PORT_E port = MT_UNF_DMX_PORT_RAM_0; //MT_UNF_DMX_PORT_RAM_0 is used to do Nagra simbad play program, so record if rom MT_UNF_DMX_PORT_RAM_1

       static mt_u8 fileIdex = 0;// for test record same PID streams

      printf("nvSprOpenRecording ------------- smp = %d emi = %d tsid= %d \n", smp, emi, tsid);
      if (!filename) {
		EMSG("file name is NULL");
		return NV_SPR_ERROR_BAD_PARAM;
	}
      /*
      //for test, eachrecording file add a index to avoid cover the same PID recording files
      if ((strlen(filename) > PVR_MAX_FILENAME_LEN - 4) || (strlen(filename) <= 3)) {
            EMSG("filename is too leng or small %d, %d", PVR_MAX_FILENAME_LEN - 4, strlen(filename));
            return NV_SPR_ERROR_BAD_PARAM;
      }
      */
/*
       if (MTPVRCount >= 3) {
            MTPVRCount = 3;
            printf("nvSprOpenRecording is more than 3!!!");
            return NV_SPR_ERROR;
       }
*/


	CHECK_TSID_VALIDITY(tsid);

	//UNUSED(smp);
	DMSG("Recording Open In,tsid=%d, emi=%x", tsid, emi);

	if (!pids) {
		EMSG("pids is NULL pointer");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	if (!pids->elems) {
		EMSG("pids elements is NULL pointer");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	if (pids->count <= 0) {
		EMSG("pids list is empty");
		return NV_SPR_ERROR_BAD_PARAM;
	}


       WR_LOCK(&rwlock);
	TMSG("filename to be saved:%s, len:%lu", filename, strlen(filename));

	//WR_LOCK(&rwlock);

	if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");

		status = NV_SPR_ERROR;
		goto error;
	}

	ts = getTransportSessionById(tsid);
	if (ts == NULL) {
		DMSG("Recording on new tsid:%d", tsid);
		ts = newTransportSession(tsid);
		if (!ts) {
			status = NV_SPR_ERROR;
			goto error;
		}

		ts->recordDmxId = mtSecGetPlatDmxId(tsid);

		DMSG("open record session %d", tsid);
		ts->sessionType = TRANSPORT_SESSION_TYPE_RECORD;
		ts->emi = emi;
		ts->tsPort = MT_UNF_DMX_PORT_BUTT;

	} else {
		//TODO: recording on same tsid of playing
		DMSG("TODO:Recording on same tsid:%d", tsid);
		ts->recordDmxId = ts->playDmxId; //TODO:?
	}

       strcpy(ts->rec_file_name, filename);
	status = parsePidList(pids, &ts->vidCodecType, &vidPid, &ts->audCodecType, &audPid);
	if (status != NV_SPR_NO_ERROR)
		goto delete_session;

       //printf("nvSprOpenRecording -----------------ts->tsPort  = %d ts->recordDmxId = %d  \n", ts->tsPort, ts->recordDmxId);


	if (ts->tsPort == MT_UNF_DMX_PORT_BUTT) {

		for (i = 0; i < MAX_PORT_NUM; i++) {
			ret = MT_UNF_DMX_AttachTSPort(ts->recordDmxId, port);//   1 , MT_UNF_DMX_PORT_RAM_0
			TMSG("dmxID = %d, port = %d ", ts->recordDmxId, port);
			if (ret != MT_SUCCESS) {
				EMSG("attach the record port to the demux failed, return error = 0x%x", ret);

				status = NV_SPR_ERROR;
				goto delete_session;
			}

			//printf("MT_UNF_DMX_CreateTSBuffer port = %d \n", port);
			ret = MT_UNF_DMX_CreateTSBuffer(port, DEMUX_TS_BUF_SIZE_REC, &ts->tsBufHandle);
			if (ret != MT_SUCCESS) {

				if (ret == MT_ERR_DMX_RECREAT_TSBUFFER) {
					MT_UNF_DMX_DetachTSPort(ts->recordDmxId);
					port += 1;
					//printf("create TS buffer on port%d failed , it will change port ", port-1);

				} else {
					EMSG("create TS buffer failed, error = 0x%x", ret);
					status = NV_SPR_ERROR;
					goto detach_record_port;
				}

			} else {
				TMSG("create TS buffer on port%d success, tsBufHandle:%lu", port, ts->tsBufHandle);
				ts->tsPort = port;
				break;
			}
		}
	} else {
		//attach directly
		ret = MT_UNF_DMX_AttachTSPort(ts->recordDmxId, ts->tsPort);
		if (ret != MT_SUCCESS) {
			EMSG("attach the record port to the demux failed, return error = 0x%x", ret);

			status = NV_SPR_ERROR;
			goto delete_session;
		}
	}
       mt_dmxSoftInject_index = 0xFF;

        if ((ts->tsPort >= MT_UNF_DMX_PORT_RAM_0) && (ts->tsPort < MT_UNF_DMX_PORT_RAM_0 + 4)) // max is 4 software inject demux channel
            mt_dmxSoftInject_index = ts->tsPort - MT_UNF_DMX_PORT_RAM_0;
        else {
            printf("%s, %d ts->tsPort = 0x%x is not supported\n",__FUNCTION__, __LINE__, ts->tsPort );
            goto delete_session;
        }
	//set pidlist to NOCSAPI to start Scrambler
	if (emi == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1
		|| emi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
		|| emi == NOCS_EMI_AES128_ECB_TAIL_CLEAR
		|| emi == NOCS_EMI_AES128_CBC_PKCS7_PADDING) {
		//TODO:MPEG-TS encryption
		TSecPidInfo *pxPidInfo = (TSecPidInfo *)malloc(sizeof(TSecPidInfo));
		if (NULL != pxPidInfo) {
			pxPidInfo->sessionType = ts->sessionType;
			pxPidInfo->pidNum = pids->count;
			for (i = 0; i < pids->count; i++) {
				pxPidInfo->pidList[i].pid = pids->elems[i].pid;
				pxPidInfo->pidList[i].type = toSecPidType(pids->elems[i].streamType);
			}
			#if 0
			if( getOthersSessionOpenedFlg(tsid, TRANSPORT_SESSION_TYPE_RECORD) ==  TRUE)
	      	       {
	      	       	printf(" pxPidInfo->pidNum = 1  ------------------------------------------------- \n");
	      	      		pxPidInfo->pidNum = 1;
				pxPidInfo->pidList[0].pid = vidPid;
				pxPidInfo->pidList[0].type = SEC_PID_VIDEO;
			}
			#endif

			//mtSecSetSessionPid(tsid, pxPidInfo, smp);
			mtSecSetSessionPid(tsid, pxPidInfo, TRUE);//SMP allways on
			free(pxPidInfo);
		}
	}

#if 0
	ts->tsBuffer = (TUnsignedInt8 *)mt_unf_cipher_malloc(DEMUX_TS_PUT_SIZE);
	if (!ts->tsBuffer) {
		EMSG("malloc stream buffer failed");

		status = NV_SPR_ERROR;
		goto destroy_ts_buffer;
	}

	ts->tsDataSize = 0;
#else
    ts->tsBuffer = NULL;
    ts->tsDataSize = 0;
#endif

#ifdef USE_DMX_RECORDING
	ret = mt_record_start(ts, filename, emi, vidPid, audPid);
	if (ret != MT_SUCCESS) {
		EMSG("record start failed, return error = 0x%x", ret);
		status = NV_SPR_ERROR;
		goto free_ts_buffer;
	}
#else
	ret = MT_UNF_PVR_RecInit();
	if (ret != MT_SUCCESS) {
		EMSG("init PVR recorder failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		goto free_ts_buffer;
	}

	MT_UNF_PVR_REC_ATTR_S pvrRecordAttr;
	memset((void *)&pvrRecordAttr, 0, sizeof(pvrRecordAttr));

	if ((vidPid > 0) && (vidPid != 0x1FFF)) {
		pvrRecordAttr.enIndexType = MT_UNF_PVR_REC_INDEX_TYPE_NONE;
        pvrRecordAttr.enIndexVidType = ts->vidCodecType;
        pvrRecordAttr.u32IndexPid = vidPid;
	} else if ((audPid > 0) && (audPid != 0x1FFF)) {
        pvrRecordAttr.enIndexType = MT_UNF_PVR_REC_INDEX_TYPE_NONE;
        pvrRecordAttr.enIndexVidType = MT_UNF_VCODEC_TYPE_BUTT;
        pvrRecordAttr.u32IndexPid = audPid;
	} else {
        EMSG("there is neither video pid nor audio pid in this stream");

		status = NV_SPR_ERROR;
		goto deinit_pvr_recorder;
	}

        pvrRecordAttr.u32DemuxID = ts->recordDmxId;
        pvrRecordAttr.u32DavBufSize = 0x5E0000; //0xbc0000;//PVR_STUB_TSDATA_SIZE;
        pvrRecordAttr.u32ScdBufSize = PVR_STUB_SC_BUF_SZIE;
        pvrRecordAttr.u32IdxBufSize = PVR_STUB_IDX_SHM_SIZE;
        pvrRecordAttr.u32UsrDataInfoSize = 0;
        pvrRecordAttr.bIsClearStream = MT_TRUE;
        pvrRecordAttr.enStreamType = MT_UNF_PVR_STREAM_TYPE_TS;



	pvrRecordAttr.u64MaxFileSize = MAX_RECORD_FILE_SIZE;
	pvrRecordAttr.u64MaxTimeInMs = 0;
	//pvrRecordAttr.bRewind = MT_TRUE;
    pvrRecordAttr.bRewind = MT_FALSE;

    strcpy(pvrRecordAttr.szFileName, filename);
    pvrRecordAttr.u32FileNameLen = strlen(pvrRecordAttr.szFileName);

  //  if (pvrRecordAttr.enIndexType == MT_UNF_PVR_REC_INDEX_TYPE_AUDIO)
		pvrRecordAttr.u32DIO = MT_FALSE;
	//else
	//	pvrRecordAttr.u32DIO = MT_TRUE;

	if (emi == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1) {
		pvrRecordAttr.bSupportAdvCa = MT_UNF_PVR_REC_DEFALT;//MT_UNF_PVR_REC_DEFALT;//MT_UNF_PVR_REC_COMMON_CRAMBLE; //support phyAddr
		pvrRecordAttr.stEncryptCfg.bDoCipher = MT_TRUE;

		//To avoid check error
        pvrRecordAttr.stEncryptCfg.u32KeyLen = 16;
        pvrRecordAttr.stEncryptCfg.enType = MT_CIPHER_ALG_AES;
        for(i = 0; i < pvrRecordAttr.stEncryptCfg.u32KeyLen; i++){
            pvrRecordAttr.stEncryptCfg.au8Key[i] = (10 + i);
        }
	} else if (emi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
			   || emi == NOCS_EMI_AES128_ECB_TAIL_CLEAR
			   || emi == NOCS_EMI_AES128_CBC_PKCS7_PADDING) {
		//pvrRecordAttr.bSupportAdvCa = MT_UNF_PVR_REC_COMMON_CRAMBLE; //support phyAddr
		//pvrRecordAttr.stEncryptCfg.bDoCipher = MT_FALSE; //record the clear stream
		//pvrRecordAttr.bSupportAdvCa = 0; //support phyAddr
		pvrRecordAttr.bSupportAdvCa = MT_UNF_PVR_REC_DEFALT;//MT_UNF_PVR_REC_DEFALT;//MT_UNF_PVR_REC_COMMON_CRAMBLE; //support phyAddr
        pvrRecordAttr.stEncryptCfg.bDoCipher = MT_TRUE;
		//To avoid check error
        pvrRecordAttr.stEncryptCfg.u32KeyLen = 16;
        pvrRecordAttr.stEncryptCfg.enType = MT_CIPHER_ALG_AES;
        for(i = 0; i < pvrRecordAttr.stEncryptCfg.u32KeyLen; i++){
            pvrRecordAttr.stEncryptCfg.au8Key[i] = (10 + i);
        }
		DMSG("set bSupportAdvCa=Nagra");
	} else {

	}

       if (gwriteFileVfp == NULL) {
            TMSG("\n");
            gwriteFileVfp = fopen(test_writeFileName, "wb");
            if (gwriteFileVfp != NULL) {
                TMSG("open %s for write", test_writeFileName);
            } else {
                TMSG("open %s failed", test_writeFileName);
            }
       }
       //TMSG("test 0x%x \n", test_writeFileName[18]+ (char)(tsid & 0x7));
	record_pip_flg = getDemuxInfo( MT_UNF_DMX_CHAN_TYPE_REC );

	mtSecSetIsMultiSessionFlag(ts->tsid, record_pip_flg);
	TMSG("0  record_pip_flg = %d \n",  record_pip_flg);

	ret = MT_UNF_PVR_RegisterEvent(MT_UNF_PVR_EVENT_REC_DMX_CREATE, MTADP_PVR_CallBack, NULL);

	if (ret == MT_ERR_PVR_ALREADY) {

		//status = NV_SPR_ERROR;
		//goto deinit_pvr_recorder;
		record_pip_flg = getDemuxInfo( MT_UNF_DMX_CHAN_TYPE_REC );
		mtSecSetIsMultiSessionFlag(ts->tsid, record_pip_flg);
	}
	TMSG("1111   record_pip_flg = %d \n",  record_pip_flg);
	ret = MT_UNF_PVR_RecCreateChn(&ts->recordChannelId, &pvrRecordAttr);
	if (ret != MT_SUCCESS) {
		EMSG("create PVR recorder channel failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		goto deinit_pvr_recorder;
	}
	TMSG("SPR Record channel ID:%d", ts->recordChannelId);
	TMSG("2222 vidPid = %x audPid=%x \n", vidPid, audPid);

	if ((vidPid > 0) && (vidPid != 0x1FFF)) {
		ret = MT_UNF_PVR_RecSetPid(ts->recordChannelId, pvrRecordAttr.u32DemuxID, (int)vidPid);
		if (ret != MT_SUCCESS) {
			EMSG("set video pid to recorder failed, return error = 0x%x", ret);

			status = NV_SPR_ERROR;
			goto destroy_record_channel;
		}
	}

	if ((audPid > 0) && (audPid != 0x1FFF)) {
		ret = MT_UNF_PVR_RecSetPid(ts->recordChannelId, pvrRecordAttr.u32DemuxID, (int)audPid);
		if (ret != MT_SUCCESS) {
			EMSG("set audio pid to recorder failed, return error = 0x%x", ret);

			status = NV_SPR_ERROR;
			goto destroy_record_channel;
		}
	}

	ret = MT_UNF_PVR_RegisterExtraCallback(ts->recordChannelId, MT_UNF_PVR_EXTRA_WRITE_CALLBACK,
	                                       recordingWriteCallback, NULL);
	if (ret != MT_SUCCESS) {
		EMSG("register recording callback failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		goto destroy_record_channel;
	}
#if 1
	ret = MT_UNF_PVR_RecStartChn(ts->recordChannelId);
	if (ret != MT_SUCCESS) {
		EMSG("start recording failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		goto unregister_callback;
	}
#endif
	char attrFile[128] = {0};
    PVR_PROG_INFO_S userData;

    sprintf(attrFile, "%s.attr", filename);
    userData.u32MagicNumber = PVR_PROG_INFO_MAGIC;
	memcpy((void *)(&userData.stRecAttr), (const void *)&pvrRecordAttr,
	       sizeof(MT_UNF_PVR_REC_ATTR_S));
	userData.stProgInfo.PcrPid = pcrPid;
	userData.stProgInfo.VideoType = ts->vidCodecType;
	userData.stProgInfo.VElementPid = (mt_u16)(vidPid & 0xFFFF);
	userData.stProgInfo.AudioType = ts->audCodecType;
	userData.stProgInfo.AElementPid = (mt_u16)(audPid & 0xFFFF);

	ret = MT_UNF_PVR_SetUsrDataInfoByFileName(attrFile, (MT_U8*)&userData, sizeof(PVR_PROG_INFO_S));
       if (ret != MT_SUCCESS) {
              EMSG("set user data to record attribute file failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		goto stop_record_channel;
        }

	#endif
       fileIdex++;
       //MTPVRCount++;
	WR_UNLOCK(&rwlock);

	TMSG("SPR Recording Open success!");

	return NV_SPR_NO_ERROR;

#ifndef USE_DMX_RECORDING
stop_record_channel:
	CHECK_CALL_MT_FUNC(MT_UNF_PVR_RecStopChn, ts->recordChannelId);
unregister_callback:
	CHECK_CALL_MT_FUNC(MT_UNF_PVR_UnRegisterExtraCallBack, ts->recordChannelId,
	                   MT_UNF_PVR_EXTRA_WRITE_CALLBACK);
destroy_record_channel:
	CHECK_CALL_MT_FUNC(MT_UNF_PVR_RecDestroyChn, ts->recordChannelId);
deinit_pvr_recorder:
	MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_REC_DMX_CREATE);
	CHECK_CALL_MT_FUNC(MT_UNF_PVR_RecDeInit);
#endif

free_ts_buffer:
	if (ts->tsBuffer != NULL) {
             TMSG("free %lx ", ts->tsBuffer);
		mmz_free(ts->tsBuffer);
		ts->tsBuffer = NULL;
	}
	//destroy_ts_buffer:
	//CHECK_CALL_MT_FUNC(MT_UNF_DMX_DestroyTSBuffer, ts->tsBufHandle);
	//ts->tsBufHandle = MT_INVALID_HANDLE;
detach_record_port:
	CHECK_CALL_MT_FUNC(MT_UNF_DMX_DetachTSPort, ts->recordDmxId);
	ts->recordDmxId = DEMUX_INVALID_ID;
delete_session:
	delTransportSession(ts);
error:
	WR_UNLOCK(&rwlock);

	return NV_SPR_ERROR;
}


TNvSprStatus nvSprOpenReplaying(TTransportSessionId tsid, TNvSprPidList *pids, TUnsignedInt16 emi, TBoolean smp)
{
	mt_s32 ret;
	TNvSprStatus status = NV_SPR_NO_ERROR;
	struct transportSession *ts = NULL;
	mt_u32 vidPid = 0;
	mt_u32 audPid = 0;
	mt_u32 pcrPid = 0;
	mt_u32 i = 0;
	MT_UNF_SYNC_REF_E syncRef = MT_UNF_SYNC_REF_NONE;
	//mt_handle videoChannel;
	mt_handle audioChannel;
	MBInfo mb = {0};

	ulong  esAudioBuffPhyAddr;
       mt_u32  esBuffSize;
	//mt_handle video_ch, audio_ch;


       printf("nvSprOpenReplaying tsid = %d  emi = 0x%x  smp = %d  ---------------------------\n", tsid, emi, smp);

	   //if(tsid == 5) return  NV_SPR_NO_ERROR;//only for debug

	#if 0//only for test
		TSecPidInfo *pxPidInfo2 = (TSecPidInfo *)malloc(sizeof(TSecPidInfo));

  	      	pxPidInfo2->pidNum = 1;
		pxPidInfo2->pidList[0].pid = 0x1;
		pxPidInfo2->pidList[0].type = SEC_PID_VIDEO;
		mtSecSetSessionPid((unsigned long)tsid, pxPidInfo2, smp);

		free(pxPidInfo2);

		return NV_SPR_NO_ERROR;
	#endif

	UNUSED(smp);
	CHECK_TSID_VALIDITY(tsid);

	//smp = 0;

	if (!pids) {
		EMSG("pids is NULL pointer");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	if (!pids->elems) {
		EMSG("pids elements is NULL pointer");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	DMSG("pidinfo:pidcount=%d", pids->count);
	DMSG("pidinfo:pid=%d,type=%d", pids->elems[0].pid, pids->elems[0].streamType);

	if (pids->count <= 0) {
		EMSG("pids list is empty pids->count = %d ", pids->count);
		return NV_SPR_ERROR_BAD_PARAM;
	}

	WR_LOCK(&rwlock);

	if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");

		status = NV_SPR_ERROR;
		goto error;
	}

	ts = newTransportSession(tsid);
	if (!ts) {
		status = NV_SPR_ERROR;
		goto error;
	}

	ts->sessionType = TRANSPORT_SESSION_TYPE_REPLAY;
	ts->emi = emi;
	ts->tsPort = MT_UNF_DMX_PORT_BUTT;

	status = parsePidList(pids, &ts->vidCodecType, &vidPid, &ts->audCodecType, &audPid);
	if (status != NV_SPR_NO_ERROR)
		goto delete_session;

	//printf("nvSprOpenReplaying  ts->audCodecType = 0x%x , ts->vidCodecType = 0x%x HA_AUDIO_ID_MP2 = 0x%x \n", ts->audCodecType, ts->vidCodecType, HA_AUDIO_ID_MP2 );

	ts->playDmxId = mtSecGetPlatDmxId(tsid);
	TMSG("=================map tsid(%d) to dmxid(%d)===================",
		 tsid, ts->playDmxId);

	if (ts->tsPort == MT_UNF_DMX_PORT_BUTT) {

		MT_UNF_DMX_PORT_E port = MT_UNF_DMX_PORT_RAM_0;
		for (i = 0; i < MAX_PORT_NUM; i++) {
			ret = MT_UNF_DMX_AttachTSPort(ts->playDmxId, port);//   1 , MT_UNF_DMX_PORT_RAM_0
			if (ret != MT_SUCCESS) {
				EMSG("attach the record port to the demux failed, return error = 0x%x", ret);

				status = NV_SPR_ERROR;
				goto delete_session;
			}

			//printf("MT_UNF_DMX_CreateTSBuffer port = %d \n", port);
			ret = MT_UNF_DMX_CreateTSBuffer(port, DEMUX_TS_BUF_SIZE, &ts->tsBufHandle);
			if (ret != MT_SUCCESS) {

				if (ret == MT_ERR_DMX_RECREAT_TSBUFFER) {
					MT_UNF_DMX_DetachTSPort(ts->playDmxId);
					port += 1;
					DMSG("create TS buffer on port%d", port);
				} else {
					EMSG("create TS buffer failed, error = 0x%x", ret);
					status = NV_SPR_ERROR;
					goto detach_ts_port;
				}

			} else {
				TMSG("create TS buffer on port%d success, tsBufHandle:%lu", port, ts->tsBufHandle);
				ts->tsPort = port;
				break;
			}
		}
	}

	//printf(" ts->tsBufHandle=%lx  \n", ts->tsBufHandle);


	ts->bUseSubLayer = getDemuxInfo( MT_UNF_DMX_CHAN_TYPE_VID );

	//printf("%s %d nvSprOpenReplaying bUseSubLayer %d  tsid = %d\n",__FUNCTION__,__LINE__,   ts->bUseSubLayer , ts->tsid);


	mtSecSetIsMultiSessionFlag(ts->tsid, ts->bUseSubLayer);


	if (emi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
		|| emi == NOCS_EMI_AES128_ECB_TAIL_CLEAR
		|| emi == NOCS_EMI_AES128_CBC_PKCS7_PADDING
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1) {

		//TODO:MT_UNF_DMX_GetTSBufferEx(ts->tsBufHandle, ts->tsDataSize, &dmxBuf, &phyAddrTsBuf, 10);
		//to get phyAddr & size of the TSBuffer
		MT_UNF_STREAM_BUF_S dmxBuf = {0};
		phys_addr_t phyAddrTsBuf = 0;
		//ret = MT_UNF_DMX_GetTSBufferEx(ts->tsBufHandle, 188, &dmxBuf, &phyAddrTsBuf, 10);
		//get 512K to make sure the address will be 256K aligned
		ret = MT_UNF_DMX_GetTSBufferEx(ts->tsBufHandle, (256*1024), &dmxBuf, &phyAddrTsBuf, 10);
		if (ret != MT_SUCCESS) {
			DMSG("Get TsBuffer failed, ret=%x", ret);
		}
		DMSG("Got TsBuffer Attr, phyAddr=0x%lx, size=%x for registering", phyAddrTsBuf, DEMUX_TS_BUF_SIZE);
		if(phyAddrTsBuf%(256*1024) !=0)//256K对齐
		{
                    TMSG("grant get addr phyAddrTsBuf =0x%p \n", phyAddrTsBuf);
			phyAddrTsBuf = (phyAddrTsBuf/(256*1024))*(256*1024) + (256*1024);
		}
		DMSG("Got TsBuffer Attr, phyAddr=0x%lx, size=%x for registering", phyAddrTsBuf, DEMUX_TS_BUF_SIZE);
		mtSecSetProtectBuffer(phyAddrTsBuf, DEMUX_TS_BUF_SIZE, MB_OTT);
	}


	ts->avPlayerStreamType = MT_UNF_AVPLAY_STREAM_TYPE_TS;
	status = openAvPlayer(ts, NULL, NULL);
	if (status != NV_SPR_NO_ERROR) {
		EMSG("open AV player failed");
		goto destroy_ts_buffer;
	}

	status = openWindow(ts);
	if (status != NV_SPR_NO_ERROR) {
		EMSG("open window failed");
		goto close_avplayer;
	}

	ret = MT_UNF_DMX_ResetTSBuffer(ts->tsBufHandle);
	if (ret != MT_SUCCESS) {
		EMSG("reset TS buffer failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		goto close_window;
	}

	pcrPid = vidPid;
	//printf("startAvPlayer ------------------------------------------------------------ vidPid =0x%x audPid =0x%x\n", vidPid, audPid);
	status = startAvPlayer(ts, ts->vidCodecType, vidPid, ts->audCodecType, audPid, pcrPid, syncRef);
	if (status != NV_SPR_NO_ERROR) {
		EMSG("start AV player failed");
		goto close_window;
	}

	if( ts->bUseSubLayer == TRUE)
	{
		//MT_UNF_DMX_GetChannelHandle(ts->playDmxId, vidPid, &videoChannel);
		//MT_UNF_DMX_GetEsBuffPhyAddr(videoChannel, (ulong *)&u32SubVideoBufPhyAddr, &u32SubVideoBufSize);
		//printf("%s %d u32SubVideoBufPhyAddr=0x%x u32SubVideoBufSize=0x%x videoChannel = %x \n",__FUNCTION__,__LINE__, u32SubVideoBufPhyAddr, u32SubVideoBufSize, videoChannel);
		//mtSecSetProtectBuffer(u32SubVideoBufPhyAddr, u32SubVideoBufSize, MB_SUB_VID);
		mtSecSetProtectBuffer(MM_PIP_ZONEMMZ_START, MM_PIP_ZONEMMZ_SIZE, MB_SUB_VID);
	}
	else
	{
         #ifdef _MT_WITH_CAK_TEST //only for CAK test
         /*
            mtTestUpdateDisplayResolution(testFormat);
            TMSG("reset Resolution to UHD ? ret = 0x%x, testFormat = 0x%x", ret, testFormat);
         */
        #endif
		MT_UNF_DMX_GetChannelHandle(ts->playDmxId, audPid, &audioChannel);
		MT_UNF_DMX_GetEsBuffPhyAddr(audioChannel, (ulong *)&esAudioBuffPhyAddr, &esBuffSize);
		mtSecSetProtectBuffer(esAudioBuffPhyAddr, esBuffSize, MB_AUD);
		mt_ngwm_configure_mainID(tsid);
	}

	//temp mmz buffer to store raw-EMIs:4020,4021,4023 input data
	//ts->tsIntBufferVir = (TUnsignedInt8 *)mt_unf_cipher_malloc(TS_INT_BUF_SIZE);
	ts->tsIntBufferVir = (TUnsignedInt8 *)mt_unf_cipher_malloc(DEMUX_TS_REP_PUT_SIZE + 0x1000);
	//ts->tsIntBufferVir = (TUnsignedInt8 *)mmz_malloc(DEMUX_RAW_REP_PUT_SIZE, SMP_DDR_ALIGN_UNIT);

      TMSG("cipher malloc 0x%lx ", ts->tsIntBufferVir);
	if (ts->tsIntBufferVir == NULL) {
		status = NV_SPR_ERROR;
		printf("%s %d \n",__FUNCTION__,__LINE__);

		goto close_window;
	}
	ts->tsIntBufferPhy = vir2phy(ts->tsIntBufferVir);

	//Raw-EMIs:4020,4021,4023
	if (emi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
		|| emi == NOCS_EMI_AES128_ECB_TAIL_CLEAR
		|| emi == NOCS_EMI_AES128_CBC_PKCS7_PADDING
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1) {
		//To store decrypted ts data for injecting
		//ts->tsBufferSize = DEMUX_TS_PUT_SIZE + (0x4096);
		ts->tsBufferSize = SMP_DDR_ALIGN_UNIT; //256K
		// Raw-EMIs:4020,4021,4023 output buffer
		ts->tsBuffer = (TUnsignedInt8 *)mmz_malloc("ddr", ts->tsBufferSize, SMP_DDR_ALIGN_UNIT);
		ts->tsBufferPhy = vir2phy(ts->tsBuffer);
		ts->tsDataSize = 0;
		//printf("%s %d \n",__FUNCTION__,__LINE__);
		//mtSecSetProtectBuffer(ts->tsBufferPhy, ts->tsBufferSize, MB_OTT);
		mtSecSetProtectBuffer((phys_addr_t)ts->tsBufferPhy, ts->tsBufferSize, MB_OTT_1);
	}

	getSprMBInfoByType(MB_VID, &mb);
	mtSecSetProtectBuffer(mb.start, mb.size, MB_VID);

	//set pidlist to NOCSAPI
	if (emi == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1
		|| emi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
		|| emi == NOCS_EMI_AES128_ECB_TAIL_CLEAR
		|| emi == NOCS_EMI_AES128_CBC_PKCS7_PADDING) {
		//TODO:MPEG-TS replaying
		//TUnsignedInt32 i = 0;
		TSecPidInfo *pxPidInfo = (TSecPidInfo *)malloc(sizeof(TSecPidInfo));
		if (NULL != pxPidInfo) {
			pxPidInfo->sessionType = ts->sessionType;
			pxPidInfo->pidNum = pids->count;
			for (i = 0; i < pids->count; i++) {
				pxPidInfo->pidList[i].pid = pids->elems[i].pid;
				pxPidInfo->pidList[i].type = toSecPidType(pids->elems[i].streamType);
			}
			//printf("tsid = %d pidNum = %d\n", tsid,pxPidInfo->pidNum);
			if( ts->bUseSubLayer  )
	      	       {
	      	      		pxPidInfo->pidNum = 1;
				pxPidInfo->pidList[0].pid = vidPid;
				pxPidInfo->pidList[0].type = SEC_PID_VIDEO;
			}
			//mtSecSetSessionPid((unsigned long)tsid, pxPidInfo, smp);
			mtSecSetSessionPid(tsid, pxPidInfo, TRUE);//SMP allways on

			free(pxPidInfo);

		}
	}
       ts->videoDecodeStart = TRUE;
	WR_UNLOCK(&rwlock);

	DMSG("====Replaying open success====");
	return NV_SPR_NO_ERROR;

close_window:
	closeWindow(ts);
close_avplayer:
	closeAvPlayer(ts);
destroy_ts_buffer:
	CHECK_CALL_MT_FUNC(MT_UNF_DMX_DestroyTSBuffer, ts->tsBufHandle);
	ts->tsBufHandle = MT_INVALID_HANDLE;
detach_ts_port:
	CHECK_CALL_MT_FUNC(MT_UNF_DMX_DetachTSPort, ts->playDmxId);
	ts->playDmxId = DEMUX_INVALID_ID;
delete_session:
	delTransportSession(ts);
error:
	WR_UNLOCK(&rwlock);

	return status;
}

#define MAX_MEM_BLOCKS  (50)//(24)
struct SprMemInfo {
	ulong addr;
	mt_size_t size;
    TBoolean smp; //FALSE:smp-off;TRUE:smp-on
};

struct SprMemCount {
	TUnsignedInt32 totalBlocksInUse;
	TUnsignedInt32 totalSizeInUse;
	struct SprMemInfo sminfo[MAX_MEM_BLOCKS];
};

//static struct SprMemCount gMemCnt = {0};

#if 0 //unused
static void dumpMemoryUsage(struct SprMemCount *mem)
{
	int i = 0;

	printf("Total Blocks In Use:%d\n", mem->totalBlocksInUse);
	printf("Total Size In Use:0x%x\n", mem->totalSizeInUse);
	printf("Mem Chunk Info:\n");
	printf("-----------------------------------------------------\n");
	for (i = 0; i < MAX_MEM_BLOCKS; i++) {
		if (mem->sminfo[i].size != 0) {
			printf("[IDX:%d]addr:0x%lu,size:0x%lu,Protect:%s\n", i, mem->sminfo[i].addr, mem->sminfo[i].size, (mem->sminfo[i].smp == TRUE ? "Yes" : "No"));
		}
	}
	printf("-----------------------------------------------------\n");
}
#endif

#if 0
static mt_s32 meminfo_push(void *addr, TUnsignedInt32 size, TBoolean smp)
{
	int i = 0;

	//DMSG("push:0x%x(%x)", (mt_u32)addr, size);

	RD_LOCK(&rwlock);
	gMemCnt.totalBlocksInUse += 1;
	gMemCnt.totalSizeInUse += size;

	for (i = 0; i < MAX_MEM_BLOCKS; i++) {
		if (gMemCnt.sminfo[i].size == 0) {
			gMemCnt.sminfo[i].addr = (ulong)addr;
			gMemCnt.sminfo[i].size = size;
            gMemCnt.sminfo[i].smp = smp;

			break;
		}
	}

	if (i == MAX_MEM_BLOCKS) {
		DMSG("Warning:too many blocks of memory are used!!!");
	}
	//dumpMemoryUsage(&gMemCnt);

	RD_UNLOCK(&rwlock);

	return 0;
}

static mt_s32 meminfo_pop(void *addr)
{
	int i = 0;
	TUnsignedInt32 sizeOfAddr = 0;

	//DMSG("pop:%p", addr);

	RD_LOCK(&rwlock);
	for (i = 0; i < MAX_MEM_BLOCKS; i++) {
		if (addr == (void *)(gMemCnt.sminfo[i].addr)) {
			sizeOfAddr = gMemCnt.sminfo[i].size;

			//Clean the block
			gMemCnt.sminfo[i].addr = 0;
			gMemCnt.sminfo[i].size = 0;
            gMemCnt.sminfo[i].smp = FALSE;
			break;
		}
	}

	if (i == MAX_MEM_BLOCKS) {
		DMSG("NOTE:%p is not a SPR_MEM buffer", addr);
	}

	if (sizeOfAddr) {
		gMemCnt.totalBlocksInUse -= 1;
		gMemCnt.totalSizeInUse -= sizeOfAddr;
	}
	RD_UNLOCK(&rwlock);

	return 0;
}
#endif

static void *mmz_malloc(const char *area, ulong size, mt_u32 alignByte)
{
    void *vir_addr = NULL;
    phys_addr_t phy_addr;

    if (size == 0)
        return NULL;

	//DMSG("request size:%x", size);

    phy_addr = (mt_u32)mt_mmz_new(size, alignByte, area, "spr_buf");
    if (phy_addr == 0) {
		EMSG("new mmz size:%lu failed", size);
        return NULL;
    }
	//DMSG("New PhyAddr:%x,size:%x", phy_addr, size);

    //map,but not cached
    vir_addr = mt_mmz_map(phy_addr, 0);
    if (vir_addr == NULL) {
		EMSG("map mmz phy:%llu failed", phy_addr);
        mt_mmz_delete(phy_addr);
        return NULL;
    }

    //TMSG("PhyAddr->VirAddr:%p, phy_addr = 0x%p", vir_addr, phy_addr);
    /* success,return the virtual address of the buffer */
    return vir_addr;
}

static mt_s32 mmz_free(mt_void *p_vir)
{
    mt_s32 ret;
    phys_addr_t phy_addr;
    ulong phy_size;

	//DMSG("VirAddr:%p", p_vir);

    if (p_vir == NULL) {
        EMSG("Can not free NULL pointer\n");
        return -1;
    }

    ret = mt_mmz_get_phyaddr(p_vir, &phy_addr, &phy_size);
    ret |= mt_mmz_unmap(p_vir);
    ret |= mt_mmz_delete(phy_addr);
    //TMSG("VirAddr:%p  ret = %d ", p_vir, ret);
    return ret;
}


void nvSprFreeMemory(void *pointer)
{
	mt_s32 ret;

	//DMSG("--TID:%d--", gettid());
	//TMSG("Free:%p", pointer);

	//meminfo_pop(pointer);
	ret = mmz_free(pointer);
	if (ret != MT_SUCCESS) {
		EMSG("mmz free failed");
	}
	//DMSG("Done");
}

static pthread_mutex_t   g_vdecMutex = PTHREAD_MUTEX_INITIALIZER;

TNvSprStatus nvSprOpenOttSession(TTransportSessionId tsid, TNvSprOttStreamType type)
{
	mt_s32 ret;
	TNvSprStatus status = NV_SPR_NO_ERROR;
	struct transportSession *ts = NULL;
       //mt_char *pCmdLine=NULL;
	TSecPidInfo pxPidInfo;


	TMSG("OTT session In tsid = %d \n", tsid);

	#ifdef CLOSE_SECOND_TS
	if(tsid == 2)
	{
		DMSG("OTT session fail tsid = 2\n");
		return NV_SPR_NO_ERROR;
	}
	#endif

	CHECK_TSID_VALIDITY(tsid);

	if (type != RAW_SAMPLES) {
		EMSG("Only support raw A/V sample, unsupport another type stream");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	WR_LOCK(&rwlock);

	if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");

		status = NV_SPR_ERROR;
		goto error;
	}

	ts = newTransportSession(tsid);
	if (!ts) {
		status = NV_SPR_ERROR;
		goto error;
	}

	ts->sessionType = TRANSPORT_SESSION_TYPE_OTT;
	ts->avPlayerStreamType = MT_UNF_AVPLAY_STREAM_TYPE_ES;

	ts->tsBufHandle = MT_INVALID_HANDLE;
	ts->tsBuffer = NULL;
	ts->tsBufferSize = 0;
	ts->tsDataSize = 0;
	ts->tsIntBufferVir = NULL;


#if 0
	ts->bUseSubLayer = getAvPlayInfo();
#else
	ts->bUseSubLayer = getDemuxInfo( MT_UNF_DMX_CHAN_TYPE_VID );
	if( ts->bUseSubLayer == FALSE)
	{
		ts->bUseSubLayer = getAvPlayInfo();
	}
#endif

	printf("nvSprOpenOttSession  bUseSubLayer = %d \n", ts->bUseSubLayer );
	mtSecSetIsMultiSessionFlag(ts->tsid, ts->bUseSubLayer);

	MT_UNF_AVPLAY_ATTR_S avPlayerAttr;
	memset((void *)&avPlayerAttr, 0, sizeof(avPlayerAttr));

	ret = MT_UNF_AVPLAY_GetDefaultConfig(&avPlayerAttr, ts->avPlayerStreamType);
	if (ret != MT_SUCCESS) {
		EMSG("get AV player attribute failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		goto delete_session;
	}

	avPlayerAttr.stStreamAttr.u32VidBufSize = (10*1024*1024);
      avPlayerAttr.stStreamAttr.u32AudBufSize = (1*1024*1024);
	if( ts->bUseSubLayer  )
	{
		avPlayerAttr.stStreamAttr.vdec_pip_chan = 1;
	}
	ret = MT_UNF_AVPLAY_Create(&avPlayerAttr, &ts->avPlayer);
	if (ret != MT_SUCCESS) {
		EMSG("create AV player failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		goto delete_session;
	}

	DMSG("av player cerate success ts->avPlayer = %d", ts->avPlayer);

	MT_UNF_SYNC_ATTR_S syncAttr;
	memset((void *)&syncAttr, 0, sizeof(syncAttr));

	ret = MT_UNF_AVPLAY_GetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_SYNC, &syncAttr);
	if (ret != MT_SUCCESS) {
		EMSG("get AV synchronization attribute failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		goto destroy_avplayer;
	}

	syncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
       ret = MT_UNF_AVPLAY_SetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_SYNC, &syncAttr);
	if (ret != MT_SUCCESS) {
		EMSG("set AV synchronization attribute failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		goto destroy_avplayer;
	}

#if 1 //

	ts->vidCodecType = MT_UNF_VCODEC_TYPE_MPEG4;//MT_UNF_VCODEC_TYPE_H264;

	MT_UNF_AVPLAY_OPEN_OPT_S avPlayerOpenOpt;
	memset((void *)&avPlayerOpenOpt, 0, sizeof(avPlayerOpenOpt));

	//ts->bUseSubLayer = getDemuxInfo( MT_UNF_DMX_CHAN_TYPE_VID );
	TMSG("%s %d  ott  ts->vidCodecType = %d ts->bUseSubLayer = %d tsid = %d  \n",__FUNCTION__,__LINE__, ts->vidCodecType, ts->bUseSubLayer, tsid);


	if (ts->vidCodecType == MT_UNF_VCODEC_TYPE_MVC) {
		avPlayerOpenOpt.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
		avPlayerOpenOpt.enDecType = MT_UNF_VCODEC_DEC_TYPE_BUTT;
		avPlayerOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_MVC;
	} else {
		//avPlayerOpenOpt.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
		//avPlayerOpenOpt.enDecType = MT_UNF_VCODEC_DEC_TYPE_NORMAL;
		//avPlayerOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_H264;

		avPlayerOpenOpt.enCapLevel      = MT_UNF_VCODEC_CAP_LEVEL_4096x2160;
             avPlayerOpenOpt.enDecType       = MT_UNF_VCODEC_DEC_TYPE_BUTT;
             avPlayerOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_BUTT;
		DMSG("Video Codec Type:%d", ts->vidCodecType);
	}

	(void)pthread_mutex_lock(&g_vdecMutex);
	TMSG(" ChnOpen start ... \n");

	ret = MT_UNF_AVPLAY_ChnOpen(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &avPlayerOpenOpt);
	if (ret != MT_SUCCESS) {
		EMSG("open video channel failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		(void)pthread_mutex_unlock(&g_vdecMutex);
		goto error;
	}
	TMSG(" ChnOpen end ... \n");

	ts->doesVidChanOpen = TRUE;

	status = openWindow(ts);
	if (status != NV_SPR_NO_ERROR) {
		EMSG("open window failed");
		(void)pthread_mutex_unlock(&g_vdecMutex);
		goto error;
	}

	MT_UNF_VCODEC_ATTR_S vidCodecAttr;
	memset((void *)&vidCodecAttr, 0, sizeof(vidCodecAttr));

	ret = MT_UNF_AVPLAY_GetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vidCodecAttr);
	if (ret != MT_SUCCESS) {
		EMSG("get video decoder attribute failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		(void)pthread_mutex_unlock(&g_vdecMutex);
		goto error;
	}

       vidCodecAttr.enType = ts->vidCodecType;
       vidCodecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
	vidCodecAttr.u32ErrCover = 100;
	vidCodecAttr.u32ErrCover = 70;
	vidCodecAttr.u32Priority = 1;
	//vidCodecAttr.u32UseDescInfoFlag = 1;
	vidCodecAttr.u32UseDescInfoFlag = 1;//0;
	vidCodecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_STABLE;
	//vidCodecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_FAST;

	//vidCodecAttr.u32UseDescInfoFlag = 1;
	//vidCodecAttr.bForceDisableTimeout = 1;
       ret = MT_UNF_AVPLAY_SetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vidCodecAttr);
	if (ret != MT_SUCCESS) {
		EMSG("set video decoder attribute failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		(void)pthread_mutex_unlock(&g_vdecMutex);
		goto error;
	}

	//DMSG("Video decode starts....");
	ret = MT_UNF_AVPLAY_Start(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
	if (ret != MT_SUCCESS) {
		EMSG("start video failed, return error = 0x%x", ret);
		status = NV_SPR_ERROR;
		(void)pthread_mutex_unlock(&g_vdecMutex);
		goto error;
	}
	ts->videoDecodeStart = TRUE;

	if( ts->bUseSubLayer == FALSE )
	{
        #ifdef _MT_WITH_CAK_TEST //only for CAK test
            mtTestUpdateDisplayResolution(testFormat);
            TMSG("reset Resolution to UHD ? ret = 0x, testFormat = 0x%x", ret, testFormat);
        #endif
		ts->audCodecType = HA_AUDIO_ID_AAC;

		ret = MT_UNF_AVPLAY_ChnOpen(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
		if (ret != MT_SUCCESS) {
			EMSG("open audio channel failed, return error = 0x%x", ret);
			printf("%s %d \n",__FUNCTION__,__LINE__);

			status = NV_SPR_ERROR;
			goto error;
		}
		DMSG("AVPLAY_ChnOpen(AUD) success");

		ts->doesAudChanOpen = TRUE;
		//printf("%s %d \n",__FUNCTION__,__LINE__);

		if (ret != MT_SUCCESS) {
			EMSG("reset avplay failed, return error = 0x%x", ret);
			goto destroy_avplayer;
		}

		status = openAudio(ts, ts->audCodecType);
		if (status != NV_SPR_NO_ERROR) {
			EMSG("open audio player failed");
			goto destroy_avplayer;
		}
		mt_ngwm_configure_mainID(tsid);
	}

	(void)pthread_mutex_unlock(&g_vdecMutex);



	TMSG(" video start success! tsid = %d  \n", tsid);
#endif
       MBInfo mb = {0};
       //register ott buffer
       DMSG("set OTT MB");
       getSprMBInfoByType(MB_OTT, &mb);
       //printf("%s %d  mb.start = 0x%llx mb.size = 0x%lx \n",__FUNCTION__,__LINE__,  mb.start, mb.size);
       mtSecSetProtectBuffer(mb.start, mb.size, MB_OTT);

	if(ts->bUseSubLayer)
	{
            //MT_UNF_AVPLAY_GetVideoESPhyAddr(ts->avPlayer, &u32SubVideoBufPhyAddr, &u32SubVideoBufSize);
            //printf("%s %d ott u32SubVideoBufPhyAddr=0x%x u32SubVideoBufSize=0x%x \n",__FUNCTION__,__LINE__, u32SubVideoBufPhyAddr, u32SubVideoBufSize);
            //mtSecSetProtectBuffer(u32SubVideoBufPhyAddr, u32SubVideoBufSize, MB_SUB_VID);
            //TMSG(" sub ott video start success ! ts->bUseSubLayer = %d \n", ts->bUseSubLayer);
            mtSecSetProtectBuffer(MM_PIP_ZONEMMZ_START, MM_PIP_ZONEMMZ_SIZE, MB_SUB_VID);
            mtExtMTLZ_TEE_SMP_En(FALSE, MT_OTT_PLAY);

	} else {
#if 1
            //register Audio,Video Buffer

            DMSG("set AUD MB");
            getSprMBInfoByType(MB_AUD, &mb);
            //printf("%s %d  mb.start = 0x%llx mb.size = 0x%lx \n",__FUNCTION__,__LINE__,  mb.start, mb.size);
            mtSecSetProtectBuffer(mb.start, mb.size, MB_AUD);

#if 1

            DMSG("set VID MB");
            getSprMBInfoByType(MB_VID, &mb);
            //printf("%s %d  mb.start = 0x%llx mb.size = 0x%lx \n",__FUNCTION__,__LINE__,  mb.start, mb.size);
            mtSecSetProtectBuffer(mb.start, mb.size, MB_VID);

#endif

#endif

           mtExtMTLZ_TEE_SMP_En(TRUE, MT_OTT_PLAY);
	}

	//Video
	ts->vesInjPointer = NULL;
	ts->vesInjSize = 0;
	//Audio
	ts->aesInjPointer = NULL;
	ts->aesInjSize = 0;

	#if 1
	pxPidInfo.sessionType = TRANSPORT_SESSION_TYPE_OTT;
	pxPidInfo.pidNum = 0;
	mtSecSetSessionPid(ts->tsid, &pxPidInfo, TRUE);
	#endif

	WR_UNLOCK(&rwlock);


      //open_test_file();

	printf("OTT session Open success tsid = %d bUseSubLayer = %d \n", tsid, ts->bUseSubLayer);


	return NV_SPR_NO_ERROR;

destroy_avplayer:
	CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_Destroy, ts->avPlayer);
delete_session:
	delTransportSession(ts);
error:
	WR_UNLOCK(&rwlock);

	return status;
}

static void dumpVideoDecoderConfig(TNvSprVideoDecoderConfig *cfg)
{
	TUnsignedInt32 i;

	DMSG("dump video decoder configuration:");

	printf("codec: %s\n", cfg->codec);

	if (cfg->cfgSize)
		printf("configuration: %s\n", cfg->cfgData);

	for (i = 0; i < cfg->cfgSize; i++) {
		if (i && !(i % 8))
			printf("\n");

		printf("%02X ", cfg->cfgData[i]);
	}

	printf("\n=============================\n");
}

#if 0
static TUnsignedInt8 *avcConfigurationBoxToAnnexB(TUnsignedInt8 *avcC, TUnsignedInt32 avcCSize,
                                                  TUnsignedInt32 *extraDataSize)
{
#define AVC_ANNEXB_START_CODE_SIZE 4
	const TUnsignedInt8 startCode[AVC_ANNEXB_START_CODE_SIZE] = {0x00, 0x00, 0x00, 0x01};
	TUnsignedInt8 configurationVersion __attribute__ ((unused));
	TUnsignedInt8 profileIndication __attribute__ ((unused));
	TUnsignedInt8 profileCompatibility __attribute__ ((unused));
	TUnsignedInt8 avcLevelIndication __attribute__ ((unused));
	TUnsignedInt8 lengthSizeMinusOne __attribute__ ((unused));

	TUnsignedInt8 numOfSequenceParameterSets; /* number of SPS NALUs */
	TUnsignedInt16 sequenceParameterSetLength; /* SPS NALU length */
	TUnsignedInt8 *sequenceParameterSetNALUnit; /* SPS NALU data */

	TUnsignedInt8 numOfPictureParameterSets; /* number of PPS NALUs */
	TUnsignedInt16 pictureParameterSetLength; /* PPS NALU length */
	TUnsignedInt8 *pictureParameterSetNALUnit; /* PPS NALU data */

	TUnsignedInt32 i, naluSize;
	TUnsignedInt8 *p;
	TUnsignedInt8 *extraData = NULL, *dst;
	TUnsignedInt32 spsSize = 0, ppsSize = 0;

	configurationVersion = avcC[0];

	if (configurationVersion != 1) {
		EMSG("Only support avc1, current version is %d", configurationVersion);
		return NULL;
	}

	if (avcCSize < 7) {
		EMSG("avcC %d too short", avcCSize);
		return NULL;
	}

	profileIndication = avcC[1];
	profileCompatibility = avcC[2];
	avcLevelIndication = avcC[3];
	lengthSizeMinusOne = (TUnsignedInt8)((avcC[4] & 0x3) + 1);

	/* Decode sps from avcC */
	numOfSequenceParameterSets = avcC[5] & 0x1F;
	spsSize = (TUnsignedInt32)(numOfSequenceParameterSets * AVC_ANNEXB_START_CODE_SIZE);

	for (i = 0, p = &avcC[6]; i < numOfSequenceParameterSets; i++) {
		sequenceParameterSetLength = (TUnsignedInt16)((((TUnsignedInt16)(*p) << 8) & 0xFF00) |
		                                              *(p + 1));
		naluSize = (TUnsignedInt32)(sequenceParameterSetLength + 2);

		if (naluSize > (avcCSize - (TUnsignedInt32)(p - avcC))) {
			EMSG("avcC data size is no enmough");
			return NULL;
		}

		spsSize += sequenceParameterSetLength;
		p += naluSize;
	}

	/* Decode pps from avcC */
	numOfPictureParameterSets = *p;
	ppsSize = (TUnsignedInt32)(numOfPictureParameterSets * AVC_ANNEXB_START_CODE_SIZE);

	for (i = 0, p++; i < numOfPictureParameterSets; i++) {
		pictureParameterSetLength = (TUnsignedInt16)((((TUnsignedInt16)(*p) << 8) & 0xFF00) |
		                                             *(p + 1));
		naluSize = (TUnsignedInt32)(pictureParameterSetLength + 2);

		if (naluSize > (avcCSize - (TUnsignedInt32)(p - avcC))) {
			EMSG("avcC data size is no enmough");
			return NULL;
		}

		ppsSize += pictureParameterSetLength;
		p += naluSize;
	}

	*extraDataSize = spsSize + ppsSize;
	extraData = malloc(*extraDataSize);
	if (!extraData) {
		EMSG("malloc extra data failed");
		return NULL;
	}

	dst = extraData;

	for (i = 0, p = &avcC[6]; i < numOfSequenceParameterSets; i++) {
		sequenceParameterSetLength = (TUnsignedInt16)((((TUnsignedInt16)(*p) << 8) & 0xFF00) |
		                                              *(p + 1));
		sequenceParameterSetNALUnit = p + 2;
		naluSize = (TUnsignedInt32)(sequenceParameterSetLength + 2);

		memcpy((void *)dst, (const void *)startCode, AVC_ANNEXB_START_CODE_SIZE);
		dst += AVC_ANNEXB_START_CODE_SIZE;

		memcpy((void *)dst, (const void *)sequenceParameterSetNALUnit,
		       sequenceParameterSetLength);
		dst += sequenceParameterSetLength;

		p += naluSize;
	}

	for (i = 0, p++; i < numOfPictureParameterSets; i++) {
		pictureParameterSetLength = (TUnsignedInt16)((((TUnsignedInt16)(*p) << 8) & 0xFF00) |
		                                             *(p + 1));
		pictureParameterSetNALUnit = p + 2;
		naluSize = (TUnsignedInt32)(pictureParameterSetLength + 2);

		memcpy((void *)dst, (const void *)startCode, AVC_ANNEXB_START_CODE_SIZE);
		dst += AVC_ANNEXB_START_CODE_SIZE;

		memcpy((void *)dst, (const void *)pictureParameterSetNALUnit,
		       pictureParameterSetLength);
		dst += pictureParameterSetLength;

		p += naluSize;
	}

	return extraData;
}
#endif

//ott video init
TNvSprStatus nvSprInitVideoDecode(TTransportSessionId tsid, TNvSprVideoDecoderConfig *cfg, TBoolean smp)
{
	//mt_s32 ret;
	TNvSprStatus status = NV_SPR_NO_ERROR;
	struct transportSession *ts;

	CHECK_TSID_VALIDITY(tsid);

	UNUSED(smp);

       //printf("nvSprInitVideoDecode tsid =%d \n", tsid);

	#ifdef CLOSE_SECOND_TS
	if( tsid == 2 )
	{
		printf("nvSprInitVideoDecode tsid == 2 return\n");
		return NV_SPR_NO_ERROR;
	}
	#endif

	if (!cfg) {
		EMSG("the video configuration buffer is invalid");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	WR_LOCK(&rwlock);

	if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");

		status = NV_SPR_ERROR;
		goto error;
	}

	ts = getTransportSessionById(tsid);
	if (!ts) {
		EMSG("the transport session identified by this id(%d) isn't existing", tsid);

		status = NV_SPR_ERROR_BAD_PARAM;
		goto error;
	}

	dumpVideoDecoderConfig(cfg);

       ts->vidExtraData = NULL;
       ts->vidExtraDataSize = 0;
	TMSG("%s %d cfg->codec = %s\n",__FUNCTION__,__LINE__, (const char *)(cfg->codec) );
	if (!strcmp("avc1", (const char *)(cfg->codec))) {
		ts->vidCodecType = MT_UNF_VCODEC_TYPE_H264;
	}
	else if (!strcmp("hev1", (const char *)(cfg->codec))) {
		ts->vidCodecType = MT_UNF_VCODEC_TYPE_HEVC;
	}
	else if(!strcmp("mp4a", (const char *)(cfg->codec)))
	{
		ts->vidCodecType = MT_UNF_VCODEC_TYPE_MPEG4;
	}
	else
	{
		ts->vidCodecType = MT_UNF_VCODEC_TYPE_MPEG2;
	}

#if 0 //

	mt_s32 ret;
	MT_UNF_AVPLAY_OPEN_OPT_S avPlayerOpenOpt;
	memset((void *)&avPlayerOpenOpt, 0, sizeof(avPlayerOpenOpt));

	//ts->bUseSubLayer = getDemuxInfo( MT_UNF_DMX_CHAN_TYPE_VID );
	TMSG("%s %d  ott  ts->vidCodecType = %d ts->bUseSubLayer = %d tsid = %d  \n",__FUNCTION__,__LINE__, ts->vidCodecType, ts->bUseSubLayer, tsid);


	if (ts->vidCodecType == MT_UNF_VCODEC_TYPE_MVC) {
		avPlayerOpenOpt.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
		avPlayerOpenOpt.enDecType = MT_UNF_VCODEC_DEC_TYPE_BUTT;
		avPlayerOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_MVC;
	} else {
		//avPlayerOpenOpt.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
		//avPlayerOpenOpt.enDecType = MT_UNF_VCODEC_DEC_TYPE_NORMAL;
		//avPlayerOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_H264;

		avPlayerOpenOpt.enCapLevel      = MT_UNF_VCODEC_CAP_LEVEL_4096x2160;
             avPlayerOpenOpt.enDecType       = MT_UNF_VCODEC_DEC_TYPE_BUTT;
             avPlayerOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_BUTT;
		DMSG("Video Codec Type:%d", ts->vidCodecType);
	}

	(void)pthread_mutex_lock(&g_vdecMutex);
	TMSG(" ChnOpen start ... \n");

	ret = MT_UNF_AVPLAY_ChnOpen(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &avPlayerOpenOpt);
	if (ret != MT_SUCCESS) {
		EMSG("open video channel failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		(void)pthread_mutex_unlock(&g_vdecMutex);
		goto error;
	}
	TMSG(" ChnOpen end ... \n");

	ts->doesVidChanOpen = TRUE;

	status = openWindow(ts);
	if (status != NV_SPR_NO_ERROR) {
		EMSG("open window failed");
		(void)pthread_mutex_unlock(&g_vdecMutex);
		goto error;
	}

	MT_UNF_VCODEC_ATTR_S vidCodecAttr;
	memset((void *)&vidCodecAttr, 0, sizeof(vidCodecAttr));

	ret = MT_UNF_AVPLAY_GetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vidCodecAttr);
	if (ret != MT_SUCCESS) {
		EMSG("get video decoder attribute failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		(void)pthread_mutex_unlock(&g_vdecMutex);
		goto error;
	}

       vidCodecAttr.enType = ts->vidCodecType;
       vidCodecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
	vidCodecAttr.u32ErrCover = 100;
	vidCodecAttr.u32ErrCover = 70;
	vidCodecAttr.u32Priority = 3;
	//vidCodecAttr.u32UseDescInfoFlag = 1;
	vidCodecAttr.u32UseDescInfoFlag = 1;//0;
	vidCodecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_STABLE;
	//vidCodecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_FAST;

	//vidCodecAttr.u32UseDescInfoFlag = 1;
	//vidCodecAttr.bForceDisableTimeout = 1;
       ret = MT_UNF_AVPLAY_SetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vidCodecAttr);
	if (ret != MT_SUCCESS) {
		EMSG("set video decoder attribute failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		(void)pthread_mutex_unlock(&g_vdecMutex);
		goto error;
	}

	//DMSG("Video decode starts....");
	ret = MT_UNF_AVPLAY_Start(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
	if (ret != MT_SUCCESS) {
		EMSG("start video failed, return error = 0x%x", ret);
		status = NV_SPR_ERROR;
		(void)pthread_mutex_unlock(&g_vdecMutex);
		goto error;
	}
	ts->videoDecodeStart = TRUE;
	(void)pthread_mutex_unlock(&g_vdecMutex);



	TMSG(" video start success! tsid = %d  \n", tsid);
#else
	mt_s32 ret;
	MT_UNF_AVPLAY_OPEN_OPT_S avPlayerOpenOpt;
	MT_UNF_AVPLAY_STOP_OPT_S stopOpt;

	memset((void *)&avPlayerOpenOpt, 0, sizeof(avPlayerOpenOpt));

	//ts->bUseSubLayer = getDemuxInfo( MT_UNF_DMX_CHAN_TYPE_VID );
	TMSG("%s %d  ott  ts->vidCodecType = %d ts->bUseSubLayer = %d tsid = %d  \n",__FUNCTION__,__LINE__, ts->vidCodecType, ts->bUseSubLayer, tsid);


	if (ts->vidCodecType == MT_UNF_VCODEC_TYPE_MVC) {
		avPlayerOpenOpt.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
		avPlayerOpenOpt.enDecType = MT_UNF_VCODEC_DEC_TYPE_BUTT;
		avPlayerOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_MVC;
	} else {
		//avPlayerOpenOpt.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
		//avPlayerOpenOpt.enDecType = MT_UNF_VCODEC_DEC_TYPE_NORMAL;
		//avPlayerOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_H264;

		avPlayerOpenOpt.enCapLevel      = MT_UNF_VCODEC_CAP_LEVEL_4096x2160;
             avPlayerOpenOpt.enDecType       = MT_UNF_VCODEC_DEC_TYPE_BUTT;
             avPlayerOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_BUTT;
		DMSG("Video Codec Type:%d", ts->vidCodecType);
	}

	(void)pthread_mutex_lock(&g_vdecMutex);
	TMSG(" ChnOpen start ... \n");

	MT_UNF_VCODEC_ATTR_S vidCodecAttr;
	memset((void *)&vidCodecAttr, 0, sizeof(vidCodecAttr));

	stopOpt.u32TimeoutMs = 0;
	stopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
	CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_Stop, ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &stopOpt);

	ret = MT_UNF_AVPLAY_GetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vidCodecAttr);
	if (ret != MT_SUCCESS) {
		EMSG("get video decoder attribute failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		(void)pthread_mutex_unlock(&g_vdecMutex);
		goto error;
	}

       vidCodecAttr.enType = ts->vidCodecType;
       vidCodecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
	vidCodecAttr.u32ErrCover = 100;
	vidCodecAttr.u32ErrCover = 70;
	vidCodecAttr.u32Priority = 1;
	vidCodecAttr.u32UseDescInfoFlag = 1;//0;
	vidCodecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_STABLE;

       ret = MT_UNF_AVPLAY_SetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vidCodecAttr);
	if (ret != MT_SUCCESS) {
		EMSG("set video decoder attribute failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		(void)pthread_mutex_unlock(&g_vdecMutex);
		goto error;
	}

	ret = MT_UNF_AVPLAY_Start(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
	if (ret != MT_SUCCESS) {
		EMSG("start video failed, return error = 0x%x", ret);
		status = NV_SPR_ERROR;
		(void)pthread_mutex_unlock(&g_vdecMutex);
		goto error;
	}

	(void)pthread_mutex_unlock(&g_vdecMutex);


	TMSG(" video start success! tsid = %d  \n", tsid);

#endif

	WR_UNLOCK(&rwlock);

	//printf("ott video init success ! tsid = %d \n", tsid);

	return NV_SPR_NO_ERROR;

//close_window:
	//closeWindow(ts);
//close_channel:
//	CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_ChnClose, ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
//	ts->doesVidChanOpen = FALSE;
error:
	WR_UNLOCK(&rwlock);

	return status;
}
/*
static void dumpAudioDecoderConfig(TNvSprAudioDecoderConfig *cfg)
{
	TUnsignedInt32 i;

	DMSG("dump audio decoder configuration:");

	printf("codec: %s\n", cfg->codec);
	printf("channelCount: %d\n", cfg->channelCount);
	printf("sampleSize: %d\n", cfg->sampleSize);
	printf("sampleRate: %d\n", cfg->sampleRate);

	if (cfg->cfgSize)
		printf("configuration: %s\n", cfg->cfgData);

	for (i = 0; i < cfg->cfgSize; i++) {
		if (i && !(i % 8))
			printf("\n");

		printf("%02X ", cfg->cfgData[i]);
	}

	printf("\n=============================\n");
}
*/
#if 0
static TNvSprStatus setupAacAdtsHeader(TUnsignedInt8 header[],
                                     TUnsignedInt32 channelCount, TUnsignedInt32 sampleRate)
{
	TUnsignedInt8 i;
	const TUnsignedInt32 aacSampleRate[16] = {
		96000, 88200, 64000, 48000, 44100, 32000,
		24000, 22050, 16000, 12000, 11025, 8000, 7350
	};

	const TUnsignedInt8 aacAdtsHeader[AAC_ADTS_HEADER_SIZE] = {
		0xFF, 0xF1, 0x40, 0x00, 0x00, 0x1F, 0xFC
	};

	for (i = 0; i < 16; i++) {
		if (sampleRate == aacSampleRate[i])
			break;
	}

	if (i >= 16) {
		EMSG("the AAC audio sample rate(%d) is invalid", sampleRate);
		return NV_SPR_ERROR;
	}

	memcpy((void *)header, aacAdtsHeader, AAC_ADTS_HEADER_SIZE);

	header[2] |= (TUnsignedInt8)((i & 0xF) << 2);
	if (channelCount > 3)
		header[2] |= (TUnsignedInt8)(channelCount >> 2);

	header[3] |= (TUnsignedInt8)(channelCount << 6);

	return NV_SPR_NO_ERROR;
}
//#else
static TNvSprStatus setupAacAdtsHeader(TUnsignedInt8 header[],
                                     TUnsignedInt32 channelCount, TUnsignedInt32 sampleRate)
{


	/*const TUnsignedInt8 aacAdtsHeader[AAC_ADTS_HEADER_SIZE] = {
		0xFF, 0xF1, 0x40, 0x00, 0x00, 0x1F, 0xFC
	};*/

	const TUnsignedInt8 aacAdtsHeader[AAC_ADTS_HEADER_SIZE] = {
		0xFF, 0xF1, 0x4D, 0xC0, 0x05, 0xDF, 0xFC
	};

	const TUnsignedInt8 aacAdtsHeader[AAC_ADTS_HEADER_SIZE] = {
		0xFF, 0xF1, 0x4D, 0xC0, 0x03, 0xFF, 0xFC
	};

	memcpy((void *)header, aacAdtsHeader, AAC_ADTS_HEADER_SIZE);



	return NV_SPR_NO_ERROR;
}
#endif

TNvSprStatus nvSprInitAudioDecode(TTransportSessionId tsid, TNvSprAudioDecoderConfig *cfg, TBoolean smp)
{


	TMSG(" nvSprInitAudioDecode audio codec = %s  \n", (const char *)(cfg->codec));

#if 0
	mt_s32 ret;
	TNvSprStatus status = NV_SPR_ERROR;
	struct transportSession *ts;


	printf("nvSprInitAudioDecode audio tsid = %d  \n", tsid);

	if(tsid >= 100)
        tsid /= 100;

	#ifdef CLOSE_SECOND_TS
	if( tsid == 2 )
	{
		printf("nvSprInitAudioDecode tsid == 2 return\n");
		return NV_SPR_NO_ERROR;
	}
	#endif

	UNUSED(smp);
	CHECK_TSID_VALIDITY(tsid);

	if (!cfg) {
		EMSG("the video configuration buffer is invalid");
		return NV_SPR_ERROR_BAD_PARAM;
	}

	WR_LOCK(&rwlock);

	if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");

		status = NV_SPR_ERROR;
		goto error;
	}

	ts = getTransportSessionById(tsid);
	if (!ts) {
		EMSG("the transport session identified by this id(%d) isn't existing", tsid);

		status = NV_SPR_ERROR_BAD_PARAM;
		goto error;
	}

	if( ts->bUseSubLayer == FALSE )
	{

		ts->doesAudChanOpen = TRUE;
		//printf("%s %d  bUseSubLayer = FALSE tsid = %d\n",__FUNCTION__,__LINE__,  ts->tsid);

	}
	else
	{
		//printf("%s %d  bUseSubLayer = TRUE tsid = %d\n",__FUNCTION__,__LINE__,  ts->tsid);
		WR_UNLOCK(&rwlock);
		return NV_SPR_NO_ERROR;
	}


	cfg->channelCount = 1;
	cfg->sampleRate = 44100;//48000;//44100
	dumpAudioDecoderConfig(cfg);
	TMSG(" audio codec = %s  \n", (const char *)(cfg->codec));
	if (!strcmp("mp4a", (const char *)(cfg->codec))) {
		#if 0
		ts->audCodecType = HA_AUDIO_ID_AAC;

		ts->aacAdtsHeader = (TUnsignedInt8 *)mt_unf_cipher_malloc(AAC_ADTS_HEADER_SIZE);
		if (ts->aacAdtsHeader == NULL) {
			EMSG("allocate for aacAdtsHeader failed");
				printf("%s %d \n",__FUNCTION__,__LINE__);

			goto error;
		}
		status = setupAacAdtsHeader(ts->aacAdtsHeader, cfg->channelCount, cfg->sampleRate);
		if (status != NV_SPR_NO_ERROR)
		{
			printf("%s %d \n",__FUNCTION__,__LINE__);

			goto error;
		}
			printf("%s %d \n",__FUNCTION__,__LINE__);

		memcpy((void *)ts->aacAdtsHeaderBak, (const void *)ts->aacAdtsHeader, AAC_ADTS_HEADER_SIZE);
		#else
		ts->audCodecType = HA_AUDIO_ID_AAC;//HA_AUDIO_ID_MP3;
		#endif

	} else
	{
		ts->audCodecType = HA_AUDIO_ID_AAC;
	}

	//hex_dump("adts header", ts->aacAdtsHeader, AAC_ADTS_HEADER_SIZE);

	ret = MT_UNF_AVPLAY_ChnOpen(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
	if (ret != MT_SUCCESS) {
		EMSG("open audio channel failed, return error = 0x%x", ret);
		printf("%s %d \n",__FUNCTION__,__LINE__);

		status = NV_SPR_ERROR;
		goto error;
	}
	DMSG("AVPLAY_ChnOpen(AUD) success");

	ts->doesAudChanOpen = TRUE;
	//printf("%s %d \n",__FUNCTION__,__LINE__);

	/* clear buffers before starting play */
//    ret = MT_UNF_AVPLAY_Reset(ts->avPlayer, MT_NULL);
	if (ret != MT_SUCCESS) {
		EMSG("reset avplay failed, return error = 0x%x", ret);
		goto close_channel;
	}


	status = openAudio(ts, ts->audCodecType);
	if (status != NV_SPR_NO_ERROR) {
		EMSG("open audio player failed");
		goto close_channel;
	}

	//printf("%s %d \n",__FUNCTION__,__LINE__);

	WR_UNLOCK(&rwlock);

	DMSG("nvSprInitAudioDecode success");
	return NV_SPR_NO_ERROR;

close_channel:
	CHECK_CALL_MT_FUNC(MT_UNF_AVPLAY_Destroy, ts->avPlayer);
	ts->doesAudChanOpen = FALSE;
error:
	WR_UNLOCK(&rwlock);

	//printf("A %d_%d\n",tsid, status);
	return status;
#else
	return NV_SPR_NO_ERROR;
#endif
}

#if 0 //unused
static mt_s32 checkInjPointer(TUnsignedInt8 *p, TUnsignedInt32 size)
{
	int i = 0;
    struct SprMemInfo *info = NULL;

	for (i = 0; i < MAX_MEM_BLOCKS; i++) {
		info = &(gMemCnt.sminfo[i]);
		if (info->addr <= (ulong)p
			&& (ulong)p <= (info->addr + info->size)) {
			//DMSG("data(%p) in Block%d,base:0x%x, size:0x%x", data, i, info->addr, info->size);
			break;
		}
	}
	if (i == MAX_MEM_BLOCKS) {
		EMSG("Error: InjPointer(%p) is not in Mem-Array", p);
		return MT_FAILURE;
	}

	for (i = 0; i < MAX_MEM_BLOCKS; i++) {
		info = &(gMemCnt.sminfo[i]);
		if (info->addr <= (ulong)(p + size)
			&& (ulong)(p + size) <= (info->addr + info->size)) {
			//DMSG("data(%p) in Block%d,base:0x%x, size:0x%x", data, i, info->addr, info->size);
			break;
		}
	}
	if (i == MAX_MEM_BLOCKS) {
		EMSG("Error: InjPointer(%p) is not in Mem-Array after add %x", p, size);
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

static TBoolean reachEndOfBlock(TUnsignedInt8 *data, TUnsignedInt32 size)
{
	int i = 0;
    struct SprMemInfo *info = NULL;

	for (i = 0; i < MAX_MEM_BLOCKS; i++) {
		info = &(gMemCnt.sminfo[i]);
		if (info->addr <= (ulong)data && (ulong)data <= (info->addr + info->size)) {
			break;
		}
	}

	if (info != NULL) {
		if ((ulong)(data + size) == (info->addr + info->size)) {
			//last fragment of this block
			return TRUE;
		} else {
			return FALSE;
		}
	} else {
		//This is impossible case, data is always inside one of the sminfo[]
		return FALSE;
	}
}
#endif



TNvSprStatus nvSprVideoDecode(TTransportSessionId tsid, TUnsignedInt32 size, TUnsignedInt8 *data)
{
	mt_s32 ret;
	TNvSprStatus status = NV_SPR_NO_ERROR;
	MT_UNF_STREAM_BUF_S esBuf;
	TUnsignedInt32 requestSize;
	TUnsignedInt32 putSize;
	struct transportSession *ts;
	//MT_UNF_AVPLAY_PUTBUFEX_OPT_S putOpt = {0};
	mt_u8 *dataPhy = vir2phy(data); //physical address of input data
	mt_u32 dataOffset = 0;

       //static mt_u8 data_flag = 0;
       //static mt_u8 * test_vir_addr = NULL;
       //mt_u8 *testdataPhy = NULL; //physical address of input data

	//phys_addr_t  u32SubVideoBufPhyAddr;
      // mt_u32  u32SubVideoBufSize;



       RD_LOCK(&rwlock);
       //printf("%s, %d tsid = %d, dataPhy = 0x%x \n", __FUNCTION__, __LINE__,  tsid, dataPhy);

/*
      #ifdef FILE_DUMP_VIDEO
	//printf("write data = %x size=%d into test file...\n", data,size);
	if( tsid == 2 )
	{
		//RD_LOCK(&rwlock);
		//(void)pthread_mutex_lock(&g_vdecMutex);
		sec_dump("data:", data, 64);
		write_test_file(gVfp, (TUnsignedInt8 *)(data), size);

		//mmz_free(data);
		//RD_UNLOCK(&rwlock);
		//(void)pthread_mutex_unlock(&g_vdecMutex);
		//return;
	}
	#endif
*/

	//(void)pthread_mutex_lock(&g_vdecMutex);

	//printf("nvSprVideoDecode in  tsid =%d size = %d ticks = %lld \n", tsid, size, get_sys_time_ms());


	CHECK_TSID_VALIDITY(tsid);

	//only for debug
	//TBoolean status_dis;
	//nvSprGetDescrambling(tsid, &status_dis);//u32VideoESFrameNumber 数值为0 ，说明数据送太慢，会导致卡顿。

	if (size > 0 && !data) {
		EMSG("the video ES buffer is invalid");
		mmz_free(data);
		RD_UNLOCK(&rwlock);
		//(void)pthread_mutex_unlock(&g_vdecMutex);
		return NV_SPR_ERROR_BAD_PARAM;
	}


	if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");

		status = NV_SPR_ERROR;
		goto end;
	}

	ts = getTransportSessionById(tsid);

//	TMSG("video decode start...data addr:%p, in size:%d tsid = %d  ts->avPlayer = %d \n", data, size, tsid, ts->avPlayer);

	if (!ts) {
		EMSG("the transport session identified by this id(%d) isn't existing", tsid);

		status = NV_SPR_ERROR_BAD_PARAM;
		goto end;
	}

	if (ts->isClosing) {
		DMSG("Session is now closing, don't inject data anymore");
		goto end;//This session is in closing, do not inject data anymore.
	}

#if 0
       if( ts->bUseSubLayer && (data_flag == 0)) {

        sec_dump("data:", data, 64);

        if ((data[0] != 0x00) || (data[1] != 0x00) || (data[2] != 0x00) || (data[3] != 0x01) || (data[4] != 0x67) || (data[5] != 0x64) || (data[6] != 0x00)) {
            DMSG("gran t abandon tsid = %d", tsid);
            status = NV_SPR_NO_ERROR;
            //MT_USLEEP(1000 * 10);
		goto end;
        }
        data_flag = 0xFF;
    }

#endif

#ifdef MT_GRANT_TEST_PIP_OTT
    if (ts->bUseSubLayer) {
        if (test_vir_addr == NULL) {

            test_vir_addr = mmz_malloc("ddr", 0x100000, SMP_DDR_ALIGN_UNIT);
               printf("testBuffer 0x%lx, phy = 0x%x \n", test_vir_addr, vir2phy(test_vir_addr));
        }

        read_test_file(gReadVfp, test_vir_addr, size);

        //sec_dump("test_vir_addr_start:", test_vir_addr, 64);

        //data_flag++;
        dataPhy = vir2phy(test_vir_addr);

    }
#endif




#if  0
    //if (ts->bUseSubLayer == 0)
    {
        if (test_vir_addr == NULL) {

            test_vir_addr = mmz_malloc("ddr", 0x100000, SMP_DDR_ALIGN_UNIT);
               printf("testBuffer 0x%lx, phy = 0x%x \n", test_vir_addr, vir2phy(test_vir_addr));
        }

        testdataPhy = vir2phy(test_vir_addr); //physical address of input data

    }
#endif


//	TMSG("ts->videoDecodeStart = %d ", ts->videoDecodeStart);

#if 0
	if (ts->videoDecodeStart == FALSE) {
		MT_UNF_AVPLAY_OPEN_OPT_S avPlayerOpenOpt;
		memset((void *)&avPlayerOpenOpt, 0, sizeof(avPlayerOpenOpt));

		//ts->bUseSubLayer = getDemuxInfo( MT_UNF_DMX_CHAN_TYPE_VID );
		TMSG("%s %d  ott  ts->vidCodecType = %d ts->bUseSubLayer = %d tsid = %d  \n",__FUNCTION__,__LINE__, ts->vidCodecType, ts->bUseSubLayer, tsid);


		if (ts->vidCodecType == MT_UNF_VCODEC_TYPE_MVC) {
			avPlayerOpenOpt.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
			avPlayerOpenOpt.enDecType = MT_UNF_VCODEC_DEC_TYPE_BUTT;
			avPlayerOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_MVC;
		} else {
			//avPlayerOpenOpt.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
			//avPlayerOpenOpt.enDecType = MT_UNF_VCODEC_DEC_TYPE_NORMAL;
			//avPlayerOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_H264;

			avPlayerOpenOpt.enCapLevel      = MT_UNF_VCODEC_CAP_LEVEL_4096x2160;
	             avPlayerOpenOpt.enDecType       = MT_UNF_VCODEC_DEC_TYPE_BUTT;
	             avPlayerOpenOpt.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_BUTT;
			DMSG("Video Codec Type:%d", ts->vidCodecType);
		}

		(void)pthread_mutex_lock(&g_vdecMutex);
		TMSG(" ChnOpen start ... \n");

		ret = MT_UNF_AVPLAY_ChnOpen(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &avPlayerOpenOpt);
		if (ret != MT_SUCCESS) {
			EMSG("open video channel failed, return error = 0x%x", ret);

			status = NV_SPR_ERROR;
			(void)pthread_mutex_unlock(&g_vdecMutex);
			goto end;
		}
		TMSG(" ChnOpen end ... \n");

		ts->doesVidChanOpen = TRUE;

		status = openWindow(ts);
		if (status != NV_SPR_NO_ERROR) {
			EMSG("open window failed");
			(void)pthread_mutex_unlock(&g_vdecMutex);
			goto end;
		}

		MT_UNF_VCODEC_ATTR_S vidCodecAttr;
		memset((void *)&vidCodecAttr, 0, sizeof(vidCodecAttr));

		ret = MT_UNF_AVPLAY_GetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vidCodecAttr);
		if (ret != MT_SUCCESS) {
			EMSG("get video decoder attribute failed, return error = 0x%x", ret);

			status = NV_SPR_ERROR;
					(void)pthread_mutex_unlock(&g_vdecMutex);
			goto end;
		}

	       vidCodecAttr.enType = ts->vidCodecType;
	       vidCodecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
		vidCodecAttr.u32ErrCover = 100;
		vidCodecAttr.u32ErrCover = 70;
		vidCodecAttr.u32Priority = 3;
		//vidCodecAttr.u32UseDescInfoFlag = 1;
		vidCodecAttr.u32UseDescInfoFlag = 1;//0;
		vidCodecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_STABLE;
		//vidCodecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_FAST;

		//vidCodecAttr.u32UseDescInfoFlag = 1;
		//vidCodecAttr.bForceDisableTimeout = 1;
	       ret = MT_UNF_AVPLAY_SetAttr(ts->avPlayer, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vidCodecAttr);
		if (ret != MT_SUCCESS) {
			EMSG("set video decoder attribute failed, return error = 0x%x", ret);

			status = NV_SPR_ERROR;
					(void)pthread_mutex_unlock(&g_vdecMutex);
			goto end;
		}

		//DMSG("Video decode starts....");
		ret = MT_UNF_AVPLAY_Start(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
		if (ret != MT_SUCCESS) {
			EMSG("start video failed, return error = 0x%x", ret);
			status = NV_SPR_ERROR;
			(void)pthread_mutex_unlock(&g_vdecMutex);
			goto end;
		}
		ts->videoDecodeStart = TRUE;
		(void)pthread_mutex_unlock(&g_vdecMutex);

		/*if(ts->bUseSubLayer)
		{
			//MT_UNF_AVPLAY_GetVideoESPhyAddr(ts->avPlayer, &u32SubVideoBufPhyAddr, &u32SubVideoBufSize);
			//printf("%s %d ott u32SubVideoBufPhyAddr=0x%x u32SubVideoBufSize=0x%x \n",__FUNCTION__,__LINE__, u32SubVideoBufPhyAddr, u32SubVideoBufSize);
			//mtSecSetProtectBuffer(u32SubVideoBufPhyAddr, u32SubVideoBufSize, MB_SUB_VID);
			TMSG(" sub ott video start success ! ts->bUseSubLayer = %d \n", ts->bUseSubLayer);
			mtSecSetProtectBuffer(MM_PIP_ZONEMMZ_START, MM_PIP_ZONEMMZ_SIZE, MB_SUB_VID);
		}*/

		TMSG(" video start success! tsid = %d  \n", tsid);
	}

#endif

       requestSize = size;
	do {

		mt_u32 getBufCount = 0;
		do {
			ret = MT_UNF_AVPLAY_GetBuf(ts->avPlayer, MT_UNF_AVPLAY_BUF_ID_ES_VID, requestSize,
			                           &esBuf, 0);
			if ((ret == MT_SUCCESS) &&
			    esBuf.pu8Data &&
			    (esBuf.u32Size > 0) &&
				(esBuf.u32Size <= requestSize)) {
				//printf("requestSize:%x(vir:%x,phy:%x)\n", esBuf.u32Size, esBuf.pu8Data, esBuf.u32PhyData);
				break;
			}

			//MT_USLEEP(1000 * 10);
			MT_USLEEP(3000);
			getBufCount++;
			if (getBufCount > 1000) {
				printf("Waiting video buffer too long, reset the buffer, and try again tsid = %d \n", tsid);
				getBufCount = 0;
				ret = MT_UNF_AVPLAY_Flush(ts->avPlayer);
			}
		} while (1);

		//DMSG("requestSize:%d, return size:%d, esphy=0x%x, srcphy=0x%x \n", requestSize, esBuf.u32Size, (esBuf.u32PhyData), (dataPhy + dataOffset));
		putSize = esBuf.u32Size;

		if( ts->bUseSubLayer )
		{
                       //sec_dump("esBuf2:", data, 16);
			   if( dmacpy(MT_EDMA_CH_3,(mt_u8 *)(esBuf.u32PhyData), (const mt_u8 *)(dataPhy + dataOffset), esBuf.u32Size)!=MT_SUCCESS )// smp on noly can use MT_EDMA_CH_3
			   {
			   	 printf("dmacpy MT_EDMA_CH_3 fail!  \n");
			   }



#ifdef MT_GRANT_TEST_PIP_OTT        //read from test es file to check SMP configuration
                    DMSG("requestSize:%d, return size:%d, esphy=0x%x, srcphy=0x%x \n", requestSize, esBuf.u32Size, (esBuf.u32PhyData), (dataPhy + dataOffset));
                    if( dmacpy(MT_EDMA_CH_3,(mt_u8 *)(esBuf.u32PhyData), (const mt_u8 *)(dataPhy + dataOffset), esBuf.u32Size)!=MT_SUCCESS )// smp on noly can use MT_EDMA_CH_3
			   {
			   	 printf("dmacpy MT_EDMA_CH_3 fail!  \n");
			   }
#endif

#if 0
                        if( dmacpy(MT_EDMA_CH_2,(mt_u8 *)testdataPhy, (const mt_u8 *)(esBuf.u32PhyData), esBuf.u32Size) !=MT_SUCCESS )// smp on noly can use MT_EDMA_CH_1
    			   {
    			   	 printf("dmacpy MT_EDMA_CH_1 ....fail!  \n");
    			   }

                       #if 1
                       sec_dump("test_vir_addr111:", test_vir_addr, 32);
                       /*
                       if(data_flag == 0) {
                        if ((test_vir_addr[0] != 0x00) || (test_vir_addr[1] != 0x00) || (test_vir_addr[2] != 0x00) || (test_vir_addr[3] != 0x01) || (test_vir_addr[4] != 0x67) || (test_vir_addr[5] != 0x64) || (test_vir_addr[6] != 0x00)) {
                            DMSG("gran t abandon tsid = %d", tsid);
                            status = NV_SPR_NO_ERROR;
                            //MT_USLEEP(1000 * 10);
                		goto end;
                        }
                        data_flag = 0xFF;
                    }
                       */
                      //sec_dump("test_vir_addr:", test_vir_addr, 256);
                      #ifdef FILE_DUMP_VIDEO
                      ret = write_test_file(gVfp, test_vir_addr, esBuf.u32Size);
                      DMSG("requestSize:%d, return size:%d, esphy=0x%x, ret=0x%x, size=0x%x\n", requestSize, esBuf.u32Size, (esBuf.u32PhyData), ret, size);
                      #endif

                       #endif


#endif

		}
		else
		{
                       //sec_dump("esBuf1:", data, 16);
			   if( dmacpy(MT_EDMA_CH_1,(mt_u8 *)(esBuf.u32PhyData), (const mt_u8 *)(dataPhy + dataOffset), esBuf.u32Size)!=MT_SUCCESS )// smp on noly can use MT_EDMA_CH_1
			   {
			   	 printf("dmacpy MT_EDMA_CH_1 fail!  \n");
			   }


                       #if 0
                        if( dmacpy(MT_EDMA_CH_2,(mt_u8 *)dataPhy, (const mt_u8 *)(esBuf.u32PhyData), esBuf.u32Size) !=MT_SUCCESS )// smp on noly can use MT_EDMA_CH_1
    			   {
    			   	 printf("dmacpy MT_EDMA_CH_1 ....fail!  \n");
    			   }

                       #if 1
                       sec_dump("test_vir_addr222:", test_vir_addr, 32);
                       /*
                       if(data_flag == 0) {
                        if ((test_vir_addr[0] != 0x00) || (test_vir_addr[1] != 0x00) || (test_vir_addr[2] != 0x00) || (test_vir_addr[3] != 0x01) || (test_vir_addr[4] != 0x67) || (test_vir_addr[5] != 0x64) || (test_vir_addr[6] != 0x00)) {
                            DMSG("gran t abandon tsid = %d", tsid);
                            status = NV_SPR_NO_ERROR;
                            //MT_USLEEP(1000 * 10);
                		goto end;
                        }
                        data_flag = 0xFF;
                    }
                       */
                      //sec_dump("test_vir_addr:", test_vir_addr, 256);
                      #ifdef FILE_DUMP_VIDEO
                      ret = write_test_file(gVfp, test_vir_addr, esBuf.u32Size);
                      DMSG("requestSize:%d, return size:%d, esphy=0x%x, ret=0x%x, size=0x%x\n", requestSize, esBuf.u32Size, (esBuf.u32PhyData), ret, size);
                      #endif

                       #endif


#endif
		}

		if (putSize < requestSize) {
			//putOpt.bEndOfFrm = MT_FALSE;
			//putOpt.u32FrameFinsh = 0;
			requestSize -= putSize;
			dataOffset += putSize;
		       DMSG("putSize < requestSize ");
		} else {
			//putOpt.bEndOfFrm = MT_TRUE;
			//putOpt.u32FrameFinsh = 1;
			requestSize = 0;
			dataOffset = 0;
			 DMSG("putSize >= requestSize ");
		}
        /*
		putOpt.bContinue = MT_TRUE;
		putOpt.u32PtsValide = 1;
		putOpt.u32EosFlag = 0;
        */
		DMSG("--TID:%d-- put size:%x(vir:%x,phy:%x)", gettid(), esBuf.u32Size, esBuf.pu8Data, esBuf.u32PhyData);

		//ret = MT_UNF_AVPLAY_PutBuf64(ts->avPlayer, MT_UNF_AVPLAY_BUF_ID_ES_VID, esBuf.u32Size, MT_INVALID_PTS64, &putOpt);
		//ret = MT_UNF_AVPLAY_PutBuf64(ts->avPlayer, MT_UNF_AVPLAY_BUF_ID_ES_VID, esBuf.u32Size, 0, &putOpt);
		ret = MT_UNF_AVPLAY_PutBuf(ts->avPlayer, MT_UNF_AVPLAY_BUF_ID_ES_VID, esBuf.u32Size, 0);
		if (ret != MT_SUCCESS) {
			EMSG("put video ES data to buffer failed, return error = 0x%x", ret);

			status = NV_SPR_ERROR;
			goto end;
		}

		DMSG("--TID:%d-- size remained:%x", gettid(), requestSize);

		//system("cat /proc/msp/vdec00");
		//system("cat /proc/media-mem");

	} while (requestSize);

	//get decoder status
	if (ts->isVideoPlaying == FALSE) {
		MT_UNF_AVPLAY_STATUS_INFO_S statusInfo;
		ret = MT_UNF_AVPLAY_GetStatusInfo(ts->avPlayer, &statusInfo);
		if (ret != MT_SUCCESS) {
			EMSG("Get AVPLAY Status failed");
		}
		DMSG("DecodedFrameCount:%d", statusInfo.u32VidFrameCount);
		if (statusInfo.u32VidFrameCount > 1) {
			ts->isVideoPlaying = TRUE;
		}
	}

	status = NV_SPR_NO_ERROR;

	//printf("video decode success tsid = %d ticks = %lld \n",  tsid, get_sys_time_ms());

end:
	mmz_free(data);
	//printf("mmz_free data \n");
	RD_UNLOCK(&rwlock);
	//(void)pthread_mutex_unlock(&g_vdecMutex);

	//if(status)
	//printf("[err] V %d_%d\n",tsid, status);

	return status;
}

TNvSprStatus nvSprAudioDecode(TTransportSessionId tsid, TUnsignedInt32 size, TUnsignedInt8 *data)
{

	mt_s32 ret;
	TNvSprStatus status = NV_SPR_NO_ERROR;
	MT_UNF_STREAM_BUF_S esBuf;
	mt_u8 *esDataPhy = NULL;
	mt_u8 *esDataPhy2 = NULL;
	mt_u32 audioFakePts = 0;
	struct transportSession *ts;
	//TUnsignedInt32 aacDataBlocks = size / 1024;
	//TUnsignedInt32 aacFrameSize = AAC_ADTS_HEADER_SIZE + size; //header+put_size
	mt_u32 requestSize = 0;

	//printf("nvSprAudioDecode tsid = %d\n", tsid);

	DMSG("[TID:%d]audio decode start...data addr:%p, in size:%d", gettid(), data, size);

	if (size > 0 && !data) {
		EMSG("the audio ES buffer is invalid");
		mmz_free(data);
		return NV_SPR_ERROR_BAD_PARAM;
	}

	RD_LOCK(&rwlock);
	(void)pthread_mutex_lock(&g_vdecMutex);


	if (!isPlatformInitialized) {
		EMSG("the platform has not been initialized");

		status = NV_SPR_ERROR;
		goto end;
	}

	if(tsid >= 100)
        tsid /= 100;

	CHECK_TSID_VALIDITY(tsid);

	ts = getTransportSessionById(tsid);
	if (!ts) {
		EMSG("the transport session identified by this id(%d) isn't existing", tsid);

		status = NV_SPR_ERROR_BAD_PARAM;
		goto end;
	}

	if( ts->bUseSubLayer == TRUE )
	{
		//printf("%s %d  bUseSubLayer = TRUE tsid = %d\n",__FUNCTION__,__LINE__,  ts->tsid);
		mmz_free(data);
		RD_UNLOCK(&rwlock);
		(void)pthread_mutex_unlock(&g_vdecMutex);
		return NV_SPR_NO_ERROR;
	}

	if (ts->isClosing) {
		DMSG("Session is now closing, don't inject data anymore");
		goto end;//This session is in closing, do not inject data anymore.
	}

	if (ts->audioDecodeStart == FALSE) {
		ret = MT_UNF_AVPLAY_Start(ts->avPlayer, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
		if (ret != MT_SUCCESS) {
			EMSG("start audio failed, return error = 0x%x", ret);
			status = NV_SPR_ERROR;
			goto end;
		}
		ts->audioDecodeStart = TRUE;
	}

	if (ts->isVideoPlaying == FALSE) {
		//DMSG("Waiting video......");
		goto end;
	}

#ifdef FILE_DUMP_AUDIO //--dump payload only
    {

        write_test_file(gAfp, (mt_u8 *)(data), size);
    }
#endif

	ts->aesInjPointer = data;
	ts->aesInjSize += size;
	requestSize = ts->aesInjSize;

	mt_u32 getBufCount = 0;
	do {
		//DMSG("Waiting Get ES Buffer, reqSize:%d...", requestSize);
		ret = MT_UNF_AVPLAY_GetBuf(ts->avPlayer, MT_UNF_AVPLAY_BUF_ID_ES_AUD, requestSize, &esBuf, 0);

		//DMSG("ret=%x, size:%d, size2:%d", ret, esBuf.u32Size, esBuf.u32Size2);
		//esBuf.u32Size2 = 0;
		if ((ret == MT_SUCCESS) &&
			esBuf.pu8Data &&
			(esBuf.u32Size > 0) &&
			((esBuf.u32Size + esBuf.u32Size2) == requestSize)) {

			//DMSG("Got AUD ES Buffer");
			break;
		}

		MT_USLEEP(1000 * 10);
		getBufCount++;
		if (getBufCount > 20) {
			printf("Waiting audio buffer too long, reset the buffer, and try again tsid = %d\n", tsid );
			getBufCount = 0;
			ret = MT_UNF_AVPLAY_Flush(ts->avPlayer);
		}
	} while (1);

	esDataPhy =  (mt_u8 *)esBuf.u32PhyData;//vir2phy(esBuf.pu8Data);
	//printf(" esDataPhy = %p  esBuf.u32PhyData = %p  esBuf.pu8Data = %p -----------------\n", esDataPhy, esBuf.u32PhyData, esBuf.pu8Data);
	if (esBuf.pu8Data2 != NULL) {
		esDataPhy2 = (mt_u8 *)esBuf.u32PhyData2;//vir2phy(esBuf.pu8Data2);
		printf("Need injecting part2, addr:%p, size=%d, size2=%d  --------------------------------------------------\n", esDataPhy2, esBuf.u32Size, esBuf.u32Size2);
	}

	//Debug
	//checkInjPointer(ts->aesInjPointer, ts->aesInjSize);

#if 1 //TODO:
	if (esBuf.u32Size > 0) {
		//DMSG("dmacopy %d bytes from %p to %p", esBuf.u32Size, (const mt_u8 *)vir2phy(ts->aesInjPointer), (mt_u8 *)(esDataPhy));
		dmacpy(MT_EDMA_CH_0, (mt_u8 *)(esDataPhy), (const mt_u8 *)vir2phy(ts->aesInjPointer), esBuf.u32Size);
		//memcpy((mt_u8 *)(esBuf.pu8Data), ts->aesInjPointer, esBuf.u32Size);

		/*if (esBuf.u32Size2 > 0) {
			//DMSG("[Total:%d]%d bytes copy done, esDataPhy2 still has %d bytes to copy", requestSize, esBuf.u32Size, esBuf.u32Size2);
		}*/
	}

	if (esDataPhy2 != NULL) {
		dmacpy(MT_EDMA_CH_0, (mt_u8 *)(esDataPhy2), (const mt_u8 *)(vir2phy(ts->aesInjPointer) + esBuf.u32Size), esBuf.u32Size2);
		//DMSG("dmacpy %d bytes into esDataPhy2(%x)", esBuf.u32Size2, (mt_u32)esDataPhy2);
	}

	//jump to next slice
	ts->aesInjPointer += ts->aesInjSize;
	ts->aesInjSize = 0;
#else
	memcpy((void *)(esBuf.pu8Data), (const void *)data, esBuf.u32Size);
	ts->aesInjPointer += ts->aesInjSize;
	ts->aesInjSize = 0;
#endif

	MT_UNF_AVPLAY_PUTBUFEX_OPT_S putOpt = {0};
	putOpt.bEndOfFrm = MT_TRUE;
	putOpt.bContinue = MT_TRUE;
	putOpt.u32PtsValide = 1;
	putOpt.u32FrameFinsh = 1;
	putOpt.u32EosFlag = 0;
	//DMSG("--TID:%d-- audioDecode PutBuf, putsize:%d", gettid(), requestSize);
	//ret = MT_UNF_AVPLAY_PutBuf(ts->avPlayer, MT_UNF_AVPLAY_BUF_ID_ES_AUD, requestSize, (171 * audioFakePts++));
	ret = MT_UNF_AVPLAY_PutBuf64(ts->avPlayer, MT_UNF_AVPLAY_BUF_ID_ES_AUD, requestSize, (171 * audioFakePts++), &putOpt);
	//ret = MT_UNF_AVPLAY_PutBuf64(ts->avPlayer, MT_UNF_AVPLAY_BUF_ID_ES_AUD, requestSize, (171 * audioFakePts++), NULL);
	if (ret != MT_SUCCESS) {
		EMSG("put audio ES data to buffer failed, return error = 0x%x", ret);

		status = NV_SPR_ERROR;
		goto end;
	}

	DMSG("--TID:%d-- audioDecode inject ok, address:%x, size:%x", gettid(), esDataPhy, requestSize);

	status = NV_SPR_NO_ERROR;
end:
	RD_UNLOCK(&rwlock);
	(void)pthread_mutex_unlock(&g_vdecMutex);

	mmz_free(data);

	if(status)
	printf("[err] A %d_%d\n",tsid, status);

	return status;
}


void nvSprShowOverlay(const char *filename)
{
	//DMSG("It's unnecessary to implementation");
}

void nvSprHideOverlay(void)
{
	//DMSG("It's unnecessary to implementation");
}

void nvSprUseSecVersion(TNvSprSecVersion version)
{
	//DMSG("It's unnecessary to implementation");
}

void nvSprSetDisplayMode(TNvSprDisplayMode mode)
{
	TMSG("Montage Symphony4 only support full screen mode. Don't support multi-process use case");
	TMSG("The displayMode is:%d", mode);
}

TNvSprStatus nvSprSetHdcp(TTransportSessionId tsid, TUnsignedInt8 hdcp)
{
	mt_s32 ret = MT_SUCCESS;

	TMSG("SessionID:%d, hdcp level:%d", tsid, hdcp);

#if 0
	DMSG("=============Do Nothing Test===============");
	return NV_SPR_NO_ERROR;
#else
	TMSG("=============set HDCP===============");
	ret = mt_hdmi_setHdcp(hdcp);

	if (ret == MT_SUCCESS)
		return NV_SPR_NO_ERROR;
	else
		return NV_SPR_ERROR;
#endif
}
void mtTestUpdateDisplayResolution(MT_UNF_ENC_FMT_E encFormat)
{
    MT_UNF_ENC_FMT_E   currFmt = MT_UNF_ENC_FMT_BUTT;
    mt_u32 ret = 0;
     ret = MT_UNF_DISP_GetFormat(MT_UNF_DISPLAY1, &currFmt);
    if(MT_SUCCESS != ret)
    {
        TMSG("MT_UNF_DISP_GetFormat failed. ret = 0x%x\n", ret);

    }
    if (encFormat != currFmt) {
        TMSG("\n");
        MT_UNF_SND_SetMute(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_ALL, TRUE);
        ret= MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY1, encFormat);
        if (ret != MT_SUCCESS)
        {
            TMSG("call MT_UNF_DISP_Attach failed, Ret=%#x.\n", ret);
            return;
        }
#ifdef _MT_WITH_TALTS_
        MT_USLEEP(5000000);
        TMSG("TALTS delay \n");
#else

        MT_USLEEP(100000);
#endif
        MT_UNF_SND_SetMute(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_ALL, FALSE);
    }
#ifdef _MT_WITH_TALTS_
        MT_USLEEP(500000);
 #endif
    //TMSG(" encFormat format is [%s] \n", MT_DispFmt2Str(encFormat));
}
TNvSprStatus nvSprSetCappingResolution(TTransportSessionId tsid, TUnsignedInt8 capping)
{
    //mt_u32 ret = 0;
    MT_UNF_ENC_FMT_E encFormat = testFormat;


    printf("this is tricky function::tsid:%d, capping:%d, encFormat = 0x%x", tsid, capping, encFormat);
/*
 *              0   => No output
 *              1   => QCIF (176x144 pixels or an equivalent number of pixels)
 *              2   => CIF  (352x288 pixels or an equivalent number of pixels)
 *              3   => SD   (720x576 pixels or an equivalent number of pixels)
 *              4   => 720  p/i
 *              5   => 1080 p/i
 *              0xF => No restrictions
 */

    switch(capping)
    {
    case 0:
        //No output
        encFormat = MT_UNF_ENC_FMT_BUTT;
        TMSG("Why set no ouput now \n");
        break;
    case 1:
        //=> QCIF (176x144 pixels or an equivalent number of pixels)
        //not support
        break;
    case 2:
        //=> CIF  (352x288 pixels or an equivalent number of pixels)
        encFormat = MT_UNF_ENC_FMT_480P_60;
        break;
    case 3:
        //=> SD   (720x576 pixels or an equivalent number of pixels)
        encFormat = MT_UNF_ENC_FMT_576P_50;
        break;
    case 4:
        //720  p/i
        encFormat = MT_UNF_ENC_FMT_720P_60;
        break;
    case 5:
        //1080 p/i
        encFormat = MT_UNF_ENC_FMT_1080i_50;
        break;
    case 0xF:
        //=> No restrictions
        //encFormat = MT_UNF_ENC_FMT_4096X2160_24;
        encFormat = testFormat;
        break;
    default:

        break;

    }
    if (encFormat < MT_UNF_ENC_FMT_BUTT)
        mtTestUpdateDisplayResolution(encFormat);

    return NV_SPR_NO_ERROR;
}

static void open_test_file(void)
{
/*
#ifdef FILE_DUMP_VIDEO
	gVfp = fopen(test_file_name, "wb");
	if (gVfp != NULL)
		TMSG("open %s for write", test_file_name);
	else
		TMSG("open %s failed", test_file_name);
    #ifdef MT_GRANT_TEST_PIP_OTT
	gReadVfp = fopen(read_file_name, "rb");
	if (gReadVfp != NULL)
		TMSG("open %s for write", read_file_name);
	else
		TMSG("open %s failed", read_file_name);
    #endif
#endif
*/
#ifdef FILE_DUMP_AUDIO
	gAfp = fopen(test_audio_file_name, "wb");
	if (gAfp != NULL)
		DMSG("open %s for write", test_audio_file_name);
	else
		DMSG("open %s failed", test_audio_file_name);
#endif

	gtsfp = fopen(ts_test_file_name, "wb");
       if (gtsfp != NULL)
		DMSG("open %s for write", ts_test_file_name);
	else
		DMSG("open %s failed", ts_test_file_name);

      gtsRefp = fopen(ts_test_re_file_name, "wb");
       if (gtsRefp != NULL)
		DMSG("open %s for write", ts_test_re_file_name);
	else
		DMSG("open %s failed", ts_test_re_file_name);

      gtsfpIn = fopen(ts_test_file_nameIn, "wb");
      if (gtsfpIn != NULL)
		DMSG("open %s for write", ts_test_file_nameIn);
	else
		DMSG("open %s failed", ts_test_file_nameIn);


}

static void close_test_file(void)
{
#ifdef FILE_DUMP_VIDEO
	if(gVfp!=NULL)
	{
		fclose(gVfp);
		gVfp = NULL;
	}
    #ifdef MT_GRANT_TEST_PIP_OTT
    	if(gReadVfp!=NULL)
	{
		fclose(gReadVfp);
		gReadVfp = NULL;
	}
    #endif
#endif
#ifdef FILE_DUMP_AUDIO
	if(gAfp !=NULL)
	{
		fclose(gAfp);
		gAfp = NULL;
	}
#endif

       if(gtsfp !=NULL)
	{
		fclose(gtsfp);
		gtsfp = NULL;
	}
       if(gtsRefp !=NULL)
	{
		fclose(gtsRefp);
		gtsRefp = NULL;
	}
       if(gtsfpIn !=NULL)
	{
		fclose(gtsfpIn);
		gtsfpIn = NULL;
	}

	DMSG("test file closed");
}

static int write_test_file(FILE *fp, unsigned char *pData, unsigned int len)
{
	int wrinte_len = 0;
	if (fp != NULL)
	{
		wrinte_len = fwrite(pData, 1, len, fp);
		//TMSG(" file = %s , pData = %x len= %d  Data[0] = 0x%x wrinte_len = %d ", fp, pData, len, pData[0], wrinte_len);
	}
    return wrinte_len;
}
#ifdef MT_GRANT_TEST_PIP_OTT
static int read_test_file(FILE *fp, unsigned char *pData, unsigned int len)
{
	int read_len = 0;
	if (fp != NULL)
	{
		read_len = fread(pData, 1, len, fp);
		TMSG(" file = %s pData = %x len= %d  Data[0] = 0x%x wrinte_len = %d ", fp, pData, len, pData[0], read_len);
	}
    return read_len;
}
#endif

void grat_testWritePVRFiles(unsigned char *pData, unsigned int len)
{
    write_test_file(gtsfp, (TUnsignedInt8 *)(pData), len);
}

void grat_testWriteRePVRFiles(unsigned char *pData, unsigned int len)
{
    write_test_file(gtsRefp, (TUnsignedInt8 *)(pData), len);
}


void grat_testWriteInFiles(unsigned char *pData, unsigned int len)
{
    write_test_file(gtsfpIn, (TUnsignedInt8 *)(pData), len);
}

static TUnsignedInt32 pkcs7_padding_data_length(TUnsignedInt8 *data, TUnsignedInt32 size, TUnsignedInt32 modulus)
{
	TUnsignedInt8 padding_value;
	TUnsignedInt8 count = 1;

	/*
	 * Only applied to TS-Pack
	 */
	if (0x47 != data[0])
		return size; //Do nothing

	if (size % modulus != 0 || size < modulus) {
		return 0;
	}

	padding_value = data[size - 1];
	if (padding_value < 1 || padding_value > modulus) {
		return size;
	}
	/* data must be at least padding_value + 1 in size */
	if (size < padding_value + 1) {
		return 0;
	}

	size--;
	for (; count < padding_value; count++) {
		size--;
		if (data[size] != padding_value) {
			return 0;
		}
	}

	return size;
}

TBoolean nvSprGetVideoSmp(void)
{
	TMSG("\n");
	return MT_TRUE;
}

TBoolean nvSprGetAudioSmp(void)
{
	TMSG("\n");
	return MT_TRUE;
}



/**
 * @brief Allocate memory from a general-purpose memory area.
 *
 * This is used for OTT stream processing. It is only used to allocate non-protected
 * buffer when smp is activated.
 *
 * Buffer allocated by this function will be used in SEC <em>processOpaqueData</em>
 * as the input buffer
 *
 * @param[in] size
 *  Size in bytes of the buffer to be allocated
 * @param[in] smp
 *  always as false, i.e. non-protected buffer.
 * @return
 *  Pointer to the buffer allocated; or NULL if allocation failed.
 */
void *nvSprAllocateMemory(TUnsignedInt32 size, TBoolean smp)
#if 0
{
	void *p = NULL;

	//DMSG("--TID:%d--", gettid());

	TMSG("allocate un-protect buffer, size:%x smp = %d", size, smp);
	p = mmz_malloc("ddr", size, SMP_DDR_ALIGN_UNIT);

	if (p != NULL) {
		//TMSG("allocate mmz buffer  size:%x, adr:%x ,addrPhy:%p", size, p, vir2phy(p));
        //meminfo_push(p, size, smp);
	} else {
		EMSG("No mmz memory for allocate, request size:%x, smp flag:%d", size, smp);
	}

	//DMSG("Done,addr:%p", p);
	return p;
}
#else
{
	void *p = NULL;

	//DMSG("--TID:%d--", gettid());
      //TMSG("allocate un-protect buffer, size:%x smp = %d", size, smp);
	if (smp) {
		//DMSG("allocate protect buffer, size:%x", size);
		/*
		 * re-adjust Memory layout, borrow 'ott0' as ott PB
		 */
		//p = mmz_malloc("ott0", size, SMP_DDR_ALIGN_UNIT);
		p = mtSec_OTTAllocateSMPMemory(size, MB_OTT);
	} else {
		//DMSG("allocate un-protect buffer, size:%x", size);
		p = mmz_malloc("ddr", size, SMP_DDR_ALIGN_UNIT);
	}

	if (p != NULL) {
            //TMSG("allocate video mmz buffer size:%x, adr:%p addrPhy:%p", size, p, vir2phy(p));
	} else {
		EMSG("No mmz memory for allocate, request size:%x, smp flag:%d", size, smp);
	}

	//DMSG("Done,addr:%p", p);
	return p;
}
#endif


void* nvSprAllocateVideoMemory(
  TUnsignedInt32 size,
  TBoolean       smp
)
{
	void *p = NULL;

	//DMSG("--TID:%d--", gettid());

	//TMSG("allocate video buffer, size:%x smp = %d", size, smp);
	/*
	 * re-adjust Memory layout, borrow 'ott0' as ott PB
	 */

	if(smp)
	{
		//p = mmz_malloc("ott0", size, SMP_DDR_ALIGN_UNIT);//
		p = mtSec_OTTAllocateSMPMemory(size, MB_OTT);
	}
	else
	{
		p = mmz_malloc("ddr", size, SMP_DDR_ALIGN_UNIT);
	}

	if (p != NULL) {
		//TMSG("allocate video mmz buffer size:%x, adr:%p addrPhy:%p", size, p, vir2phy(p));
		//meminfo_push(p, size, smp);
	} else {
		EMSG("No mmz memory for allocate, request size:%x, smp flag:%d", size, smp);
		return NULL;
	}

	//printf("Done,addr:%p", p);
	return p;
}


void* nvSprAllocateAudioMemory(
  TUnsignedInt32 size,
  TBoolean       smp
)
{
	void *p = NULL;

	//DMSG("--TID:%d--", gettid());

	//TMSG("allocate  audio buffer, size:%x smp = %d", size, smp);

	/*
	 * re-adjust Memory layout, borrow 'ott0' as ott PB
	 */
	//p = mmz_malloc("pcm", size, 0);

	if(smp)
	{
		//p = mmz_malloc("ott0", size, SMP_DDR_ALIGN_UNIT);
		p = mtSec_OTTAllocateSMPMemory(size, MB_OTT);
	}
	else
	{
		p = mmz_malloc("ddr", size, SMP_DDR_ALIGN_UNIT);
	}

	if (p != NULL) {
		//TMSG("allocate audio mmz buffer size:%x, addr:%p addrPhy:%p", size, p, vir2phy(p));
		//meminfo_push(p, size, smp);
	} else {
		EMSG("No mmz memory for allocate, request size:%x, smp flag:%d", size, smp);
		return NULL;
	}

	//printf("Done,addr:%p", p);
	return p;
}


void nvSprReboot(void)
{
    printf("%s %d \n",__FUNCTION__,__LINE__);
    system("reboot");
}


void nvSprShowOverlayAt(
  int         px,
  int         py,
  int         ax,
  int         ay,
  const char* filename
)
{
    printf("%s %d \n",__FUNCTION__,__LINE__);
}

