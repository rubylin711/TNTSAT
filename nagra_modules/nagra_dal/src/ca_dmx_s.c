#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include "mt_unf_demux.h"
#include "ca_dmx.h"

#define DMX_DEBUG 1
#if DMX_DEBUG
#define DMX_ERROR_PRINT	printf
#define DMX_DEBUG_PRINT	printf
#define ENTER_FUNCTION()	do{struct timeval tv; gettimeofday(&tv, NULL);printf("[%ld.%06ld]Enter %s\n",tv.tv_sec,tv.tv_usec,__FUNCTION__);}while(0)
#define LEAVE_FUNCTION()	do{struct timeval tv; gettimeofday(&tv, NULL);printf("[%ld.%06ld]Leave %s@%d\n",tv.tv_sec,tv.tv_usec,__FUNCTION__,__LINE__);}while(0)
#else
#define DMX_ERROR_PRINT(...)	do {} while(0);
#define DMX_DEBUG_PRINT(...)	do {} while(0);
#define ENTER_FUNCTION()	do {} while(0);
#define LEAVE_FUNCTION()	do {} while(0);
#endif

#define PLATFORM_DEMUX_ID		0

#define FILTER_NUM_MAX		45 //CanalPlus modify org:20 
#define FILTER_DEPTH_MAX	DMX_FILTER_MAX_DEPTH
#define FILTER_IDX_OFFSET	600
#define FILTER_IDX_TO_ID(idx)	((idx) + FILTER_IDX_OFFSET)
#define FILTER_ID_TO_IDX(id)	((id) - FILTER_IDX_OFFSET)
#define FILTER_BUFFER_SIZE	(16  * 1024)
#define FILTER_BIG_BUFFER_SIZE	(640  * 1024)
#define FILTER_TASK_SLEEP_MS	20
#define FILTER_SEC_MAX_NUM	  256
#define CHAN_MAX_NUM	  45 //CanalPlus modify org:20 
#define DEMUX_MAX_NUM		1

enum timeout_mode
{
	TIMEOUT_MODE_FOREVER = 0,
	TIMEOUT_MODE_WAIT,
	TIMEOUT_MODE_UNKNOWN,
};

struct filter_info
{
	struct SDmxFilterId id;
	int is_used;
	int is_start;
	TTransportSessionId tsid;
	int demuxid;
	void* priv_data;
	TSize depth;
	TUnsignedInt8 flt_val[FILTER_DEPTH_MAX];
	TUnsignedInt8 flt_mask[FILTER_DEPTH_MAX];
	TUnsignedInt8 flt_nmask[FILTER_DEPTH_MAX];
	TPid pid;
	TDmxCrcMode crc_mode;
	TDmxLoopMode loop_mode;
	TDmxFilterQueryBufferCallback qbuf_cb;
	TDmxFilterReceivedSectionCallback rsec_cb;
	enum timeout_mode timeout_mode;
	int timeout;
	mt_s64 start_time;
	mt_handle flt_hdl;
	mt_handle chn_hdl;
};

struct pid_info
{
	TPid pid;
	int count;
};

static pthread_t	dmx_recv_task;
static pthread_t	dmx_timer_task;
static pthread_mutex_t	dmx_mutex = PTHREAD_MUTEX_INITIALIZER;
static int			is_dmx_inited = 0;
static struct filter_info	filter_info_tbl[FILTER_NUM_MAX];
static struct pid_info	pid_info_tbl[DEMUX_MAX_NUM][FILTER_NUM_MAX];

static TDmxStatus _dmxFilterStop(TDmxFilterId   xFilterId);
static void inc_pid_count(mt_s32 dmx_id, TPid pid);

static mt_s64  get_tick_time(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (mt_s64)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static TDmxStatus _dmxFilterStart
(
  TDmxFilterId                        xFilterId,
  TOsTime                             xTimeout,
  TDmxLoopMode                        xLoopMode,
  TDmxCrcMode                         xCrcMode,
  TDmxFilterQueryBufferCallback       xQueryBufferCallback,
  TDmxFilterReceivedSectionCallback   xReceivedSectionCallback
);
static TDmxStatus _dmxFilterSetPatterns
(
        TDmxFilterId   xFilterId,
  const TUnsignedInt8* pxValue,
  const TUnsignedInt8* pxEqualMask,
  const TUnsignedInt8* pxNotEqualMask
);

static void dmx_process_section(struct filter_info *fi, MT_UNF_DMX_DATA_S *sec)
{
	TUnsignedInt8	*buf = NULL;
	TSize		bufsz;
	TDmxBufferStatus	st;
	//pthread_mutex_unlock(&dmx_mutex);
	st = (*fi->qbuf_cb)(&fi->id, &buf, &bufsz, fi->priv_data);
	//pthread_mutex_lock(&dmx_mutex);
	if (st == DMX_BUFFER_ERROR) {
		DMX_ERROR_PRINT("dmx_process_section ===> DMX_BUFFER_ERROR");
		(*fi->rsec_cb)(&fi->id, DMX_SECTION_BUFFER_ERROR, NULL, 0, fi->priv_data);
		return;
	}

	DMX_DEBUG_PRINT("dmx_process_section ===> bufsz=%d, sec->u32Size=%d  sec->u32Size,fi->id.id = %d \n", bufsz, sec->u32Size,fi->id.id);
	if (bufsz < sec->u32Size) {
		memcpy(buf, sec->pu8Data, bufsz);
             (*fi->rsec_cb)(&fi->id, DMX_SECTION_AVAILABLE, buf, bufsz, fi->priv_data);
		return;
	}

	memcpy(buf, sec->pu8Data, sec->u32Size);
	//pthread_mutex_unlock(&dmx_mutex);
	(*fi->rsec_cb)(&fi->id, DMX_SECTION_AVAILABLE, buf, sec->u32Size, fi->priv_data);
	//pthread_mutex_lock(&dmx_mutex);
}

static void dmx_process_loop_mode(struct filter_info *fi, MT_UNF_DMX_DATA_S *sec)
{

	if (fi->loop_mode == DMX_LOOP_ONE_SHOT) {
		_dmxFilterStop(&fi->id);
	} else if (fi->loop_mode == DMX_LOOP_TOGGLE) {
		if (fi->flt_val[0] != sec->pu8Data[0]) {
			fi->flt_val[0] = sec->pu8Data[0];
		}

		_dmxFilterStop(&fi->id);
		fi->flt_mask[0] = (fi->flt_mask[0] | 0x1);
		fi->flt_val[0] = (fi->flt_val[0] & 0x1) ? (fi->flt_val[0] & (mt_u8)~0x1) : (fi->flt_val[0] | 0x1);
		DMX_DEBUG_PRINT("--->value[0x%02x], mask[0x%02x], nmask[0x%02x]\n",
					fi->flt_val[0], fi->flt_mask[0], fi->flt_nmask[0]);
		_dmxFilterSetPatterns(&fi->id,fi->flt_val, fi->flt_mask, fi->flt_nmask);
		DMX_DEBUG_PRINT("<---value[0x%02x], mask[0x%02x], nmask[0x%02x]\n",
					fi->flt_val[0], fi->flt_mask[0], fi->flt_nmask[0]);
		_dmxFilterStart(&fi->id,
				(TOsTime)fi->timeout,
				fi->loop_mode,
				fi->crc_mode,
				fi->qbuf_cb,
				fi->rsec_cb);
	}
}

static mt_s32 match_filter(struct filter_info *fi, MT_UNF_DMX_DATA_S *sec)
{
	MT_UNF_DMX_FILTER_ATTR_S attr;
	mt_u8 buf[DMX_FILTER_MAX_DEPTH];
	mt_u32 i;
	mt_s32 ret;

	buf[0] = sec->pu8Data[0];
	memcpy(&buf[1], &sec->pu8Data[3], DMX_FILTER_MAX_DEPTH - 1);

	//DMX_DEBUG_PRINT("buf[0] = 0x%02x<-->value[0x%02x], mask[0x%02x], nmask[0x%02x]\n",
	//				buf[0], fi->flt_val[0], fi->flt_mask[0], fi->flt_nmask[0]);
	ret = MT_UNF_DMX_GetFilterAttr(fi->flt_hdl, &attr);
	if (ret != MT_SUCCESS)
		return ret;

	for (i = 0; i < attr.u32FilterDepth; i ++) {
		if (attr.au8Negate[i]) {
			if ((~attr.au8Mask[i] & attr.au8Match[i]) == (~attr.au8Mask[i] & buf[i]))
				return MT_FAILURE;
		} else {
			if ((~attr.au8Mask[i] & attr.au8Match[i]) != (~attr.au8Mask[i] & buf[i]))
				return MT_FAILURE;
		}
	}
	return MT_SUCCESS;
}

static void broadcast_data_to_filters(TPid pid, MT_UNF_DMX_DATA_S *sec)
{
	int		 i;
	struct filter_info	*fi;
	mt_s32 		ret;
	struct timeval tv;
	mt_s64 timeMs = 0;

	for (i = 0; i < FILTER_NUM_MAX; i ++) {
		fi = &filter_info_tbl[i];
		if (fi->pid == pid && fi->is_start) {
			ret = match_filter(fi, sec);
			if (ret != MT_SUCCESS)
				continue;
			dmx_process_section(fi, sec);
			dmx_process_loop_mode(fi, sec);
			//fi->time_count = fi->timeout;
			timeMs = get_tick_time();
			fi->start_time = timeMs;
			return;
		}
	}
}

static mt_u32 dmx_check_valid_handle( mt_handle u32ChHandle)
{
	int i = 0;
	struct filter_info *fi;
	mt_s32				ret;
	mt_handle			chn_hdl;

	if (u32ChHandle == 0)
		return 1;
	for (i = 0; i < FILTER_NUM_MAX; i++) {
		fi = &filter_info_tbl[i];
		if ((fi->is_start == 0) || (fi->pid == PID_INVALID)) {
			continue;
		}
		//ret = MT_UNF_DMX_GetChannelHandle(fi->demuxid, fi->pid, &chn_hdl);
		ret = MT_UNF_DMX_GetChannelHandleByPidType(fi->demuxid, fi->pid, MT_UNF_DMX_CHAN_TYPE_SEC, &chn_hdl);
		if (MT_SUCCESS != ret){
			continue;
		}

		if ((fi->chn_hdl == u32ChHandle) && (chn_hdl == u32ChHandle)) {
			return 0;
		}
	}

	if (i >= FILTER_NUM_MAX) {
		return 1;
	}
}

static mt_void get_used_dmx_handles(mt_handle *handle, mt_u32 *handleNum)
{
	int i, j = 0;
	struct filter_info *fi;
	
	for (i = 0; i < FILTER_NUM_MAX; i++) {
		fi = &filter_info_tbl[i];
		if (fi->is_start && fi->is_used) {
			handle[j ++] = fi->chn_hdl;
		}
	}
	*handleNum = j;
}

static mt_void* dmx_recv_section_task(mt_void* args)
{
	MT_UNF_DMX_DATA_S 	sec[FILTER_SEC_MAX_NUM];
	mt_u32			sec_num;
	struct filter_info 		*fi;
	mt_s32			ret;
	mt_u32 u32HandleNum = CHAN_MAX_NUM;
	mt_handle u32ChHandle[CHAN_MAX_NUM];
	int cnt = 0, sec_cnt = 0;
	mt_u32  pu32Pid = PID_INVALID;
	mt_set_pthread_name("dmx_recv_section_task");

	while(1) 
	{
		u32HandleNum = 0;
		memset((void *)u32ChHandle, 0, sizeof(mt_handle) * CHAN_MAX_NUM);

		pthread_mutex_lock(&dmx_mutex);
		get_used_dmx_handles(u32ChHandle, &u32HandleNum);
		pthread_mutex_unlock(&dmx_mutex);

		if (u32HandleNum == 0) {
				usleep(50000);
				continue;
		}
		//ret = MT_UNF_DMX_GetDataHandle((mt_handle *)u32ChHandle, (mt_u32 *)(&u32HandleNum), (mt_u32)200);
		//ret = MT_UNF_DMX_SelectDataHandle(u32UsedChHandle, u32UsedHandleNum,
		//						(mt_handle *)u32ChHandle, (mt_u32 *)(&u32HandleNum), (mt_u32)200);
		//if ((MT_SUCCESS != ret) || (u32HandleNum == 0))
		//{
		//	usleep(50000);
		//	continue;
		//}

		pthread_mutex_lock(&dmx_mutex);
		for (cnt = 0; cnt < u32HandleNum; cnt ++) 
		{
		  if (MT_SUCCESS != MT_UNF_DMX_CheckDataHandle(u32ChHandle[cnt], 0))
		  {
				usleep(5000);
        continue ;
		  }
		  printf("[%s:%d] cmt=%d, h=0x%x-\n", __FUNCTION__, __LINE__, cnt, u32ChHandle[cnt]);
			if (dmx_check_valid_handle(u32ChHandle[cnt]) == 0) 
			{
				ret = MT_UNF_DMX_AcquireBuf(u32ChHandle[cnt], FILTER_SEC_MAX_NUM, &sec_num, sec, 0);
				if (MT_SUCCESS != ret)
				{
					continue;
				}

				if (sec_num) 
				{
					MT_UNF_DMX_GetChannelPID(u32ChHandle[cnt], &pu32Pid);
					for (sec_cnt = 0; sec_cnt < sec_num; sec_cnt++) 
					{
						broadcast_data_to_filters(pu32Pid, &sec[sec_cnt]);
					}
					MT_UNF_DMX_ReleaseBuf(u32ChHandle[cnt], sec_num, sec);
				}
			}
		}

		pthread_mutex_unlock(&dmx_mutex);

		usleep(20000);
	}
	
	return (mt_void *)0;
}
static mt_void* dmx_timer_check_task(mt_void* args)
{
	int	i = 0;
	mt_s64	timeMs = 0;
	struct filter_info	*fi;
  mt_set_pthread_name("dmx_timer_check_task");
	
	while (is_dmx_inited)
	{
		for (i=0; i < FILTER_NUM_MAX; i++) 
		{
			if ((filter_info_tbl[i].is_used == 0) || (filter_info_tbl[i].is_start == 0) || (filter_info_tbl[i].flt_hdl == 0)) 
			{
				usleep(500);
				continue;
			}
			if (filter_info_tbl[i].timeout_mode != TIMEOUT_MODE_FOREVER) 
			{
				pthread_mutex_lock(&dmx_mutex);
				timeMs = get_tick_time();
				fi = &filter_info_tbl[i];
				if ((fi->is_used == 0) || (fi->is_start == 0) || (fi->flt_hdl == 0) || (fi->timeout == 0)) 
				{
					pthread_mutex_unlock(&dmx_mutex);
					continue;
				}
				if (timeMs > (fi->start_time + fi->timeout)) 
				{
					DMX_DEBUG_PRINT(" timeout ====> timeMs = %lld start_time =%lld	fi->id.id =%d \n",timeMs , fi->start_time, fi->id.id);
					if (fi->is_used && fi->is_start && fi->flt_hdl)
					{
						(*fi->rsec_cb)(&fi->id, DMX_SECTION_TIMEOUT_ERROR, NULL, 0, fi->priv_data);
						fi->start_time = timeMs;
					}
				}
				pthread_mutex_unlock(&dmx_mutex);
			}			
		}
	}
	return (mt_void *)0;
}

static void reset_filter_info(struct filter_info * fi)
{
	memset(fi , 0, sizeof(struct filter_info));
	fi->pid = PID_INVALID;
}

static mt_s32 dmx_driver_init(void)
{
	MT_S32	ret;
	int	i, j;

	for (i = 0; i < FILTER_NUM_MAX; i ++) {
		reset_filter_info(&filter_info_tbl[i]);
		for (j = 0; j < DEMUX_MAX_NUM; j ++) {
			pid_info_tbl[j][i].count = 0;
			pid_info_tbl[j][i].pid = PID_INVALID;
		}
	}

	ret = pthread_create(&dmx_recv_task, NULL, dmx_recv_section_task, NULL);
	if(0 != ret) {
		DMX_ERROR_PRINT("dmx_driver_init ===> pthread_create( dmx_recv_task )\n");
		goto TSPORT_DETACH;

	}
	
	ret = pthread_create(&dmx_timer_task, NULL, dmx_timer_check_task, NULL);
	if(0 != ret) {
		DMX_ERROR_PRINT("dmx_driver_init ===> pthread_create( dmx_timer_task )\n");
		goto TSPORT_DETACH;

	}

	return MT_SUCCESS;
TSPORT_DETACH:
DMX_DEINIT:
DMX_EXIT:
	return ret;
}

static void inc_pid_count(mt_s32 dmx_id, TPid pid)
{
	int i;
	for (i = 0; i < FILTER_NUM_MAX; i ++) {
		if (pid == pid_info_tbl[dmx_id][i].pid) {
			pid_info_tbl[dmx_id][i].count ++;
			return;
		}
	}

	for (i = 0; i < FILTER_NUM_MAX; i ++) {
		if (PID_INVALID == pid_info_tbl[dmx_id][i].pid) {
			pid_info_tbl[dmx_id][i].pid = pid;
			pid_info_tbl[dmx_id][i].count ++;
			return;
		}
	}
}

static void dec_pid_count(mt_s32 dmx_id, TPid pid)
{
	int i;
	for (i = 0; i < FILTER_NUM_MAX; i ++) {
		if (pid == pid_info_tbl[dmx_id][i].pid) {
			pid_info_tbl[dmx_id][i].count --;
			return;
		}
	}
}

static int channel_need_to_destory(mt_handle chn_hdl)
{
	int i;
	for (i = 0; i < FILTER_NUM_MAX; i ++) {
		if (chn_hdl == filter_info_tbl[i].chn_hdl && chn_hdl == 0) {
			return 0;
		}
	}
	return 1;
}

static void cleanup_channels(void) {
	int 		i, j;
	mt_handle	chn_hdl;
	mt_s32		ret;
	MT_UNF_DMX_CHAN_STATUS_S stChanStatus = {0};

	for (i = 0; i < FILTER_NUM_MAX; i ++) {
		for (j = 0; j < DEMUX_MAX_NUM; j ++) {
			if (PID_INVALID != pid_info_tbl[j][i].pid && pid_info_tbl[j][i].count == 0) {
				//ret = MT_UNF_DMX_GetChannelHandle(j, pid_info_tbl[j][i].pid , &chn_hdl);
				ret = MT_UNF_DMX_GetChannelHandleByPidType(j, pid_info_tbl[j][i].pid , MT_UNF_DMX_CHAN_TYPE_SEC, &chn_hdl);
				if (MT_SUCCESS != ret)
					continue;
				if (channel_need_to_destory(chn_hdl)) {
					ret = MT_UNF_DMX_CloseChannel(chn_hdl);
					if (ret != MT_SUCCESS) {
						DMX_ERROR_PRINT("cleanup_channels ===> MT_UNF_DMX_CloseChannel fail\n");
					}
					ret = MT_UNF_DMX_GetChannelStatus(chn_hdl, &stChanStatus);
					if (ret != MT_SUCCESS) {
						DMX_ERROR_PRINT("cleanup_channels ===> MT_UNF_DMX_GetChannelStatus fail\n");
					}
					if (MT_UNF_DMX_CHAN_CLOSE != stChanStatus.enChanStatus) {
						DMX_ERROR_PRINT("cleanup_channels ===> stChanStatus.enChanStatus( 0x%x )\n", stChanStatus.enChanStatus);
					}
					ret = MT_UNF_DMX_DestroyChannel(chn_hdl); 
					if (ret != MT_SUCCESS) {
						DMX_ERROR_PRINT("cleanup_channels ===> MT_UNF_DMX_DestroyChannel fail\n");
					}
					pid_info_tbl[j][i].pid = PID_INVALID;
				}
			}
		}
	}
}

static int get_free_filter(void)
{
	int i;
	for (i = 0; i < FILTER_NUM_MAX; i ++) {
		if (filter_info_tbl[i].is_used == 0)
			return i;
	}
	return -1;
}

static int is_filter_id_valid(TDmxFilterId fid)
{
	int i;
	for (i = 0; i < FILTER_NUM_MAX; i ++) {
		if (filter_info_tbl[i].is_used) {
			if (&filter_info_tbl[i].id == fid)
				return 0;
		}
	}
	return -1;
}

/******************************************************************************/
/*                                                                            */
/*                            FUNCTION PROTOTYPES                             */
/*                                                                            */
/******************************************************************************/

/**
 *  @brief
 *    This function opens a demux filter.
 *
 *  @param[out] pxFilterId
 *                Identifier assigned to the demux filter opened.
 *
 *  @param[in]  xTransportSessionId
 *                Transport session identifier associated to the stream to be
 *                filtered.
 *
 *  @param[in]  xFilterDepth
 *                Maximum size in bytes of the matching section filter pattern,
 *                starting from the table ID. If the filter requires more than
 *                the table ID, \c xFilterDepth will include the two bytes of
 *                \c section_length.
 *
 *  @param[in]  pxPrivateData
 *                Private data used by the CA software for internal processing.
 *                This pointer shall be passed back as parameter of the two
 *                callback functions \c TDmxFilterQueryBufferCallback and \c
 *                TDmxFilterReceivedSectionCallback
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_NO_MORE_RESOURCES
 *             No more demux filter resources are available
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
*/
TDmxStatus dmxFilterOpen
(
        TDmxFilterId*	         pxFilterId,
        TTransportSessionId	    xTransportSessionId,
        TSize	                  xFilterDepth,
  const void*	                 pxPrivateData
)
{
	mt_s32		ret;
	int 		flt_idx;
	struct filter_info	*fi;

	if (pxFilterId == NULL)
		return DMX_ERROR;

	if (xTransportSessionId == TRANSPORT_SESSION_ID_INVALID)
		return DMX_ERROR;

	pthread_mutex_lock(&dmx_mutex);
	if (is_dmx_inited == 0) {
		ret = dmx_driver_init();
		if (ret != MT_SUCCESS) {
			DMX_ERROR_PRINT("dmxFilterOpen ===> dmx_driver_init fail\n");
			ret = DMX_ERROR;
			goto EXIT;
		}
		is_dmx_inited = 1;
	}

	if (xFilterDepth > FILTER_DEPTH_MAX) {
		DMX_ERROR_PRINT("dmxFilterOpen ===> xFilterDepth > FILTER_DEPTH_MAX fail\n");
		ret = DMX_ERROR;
		goto EXIT;
	}

	flt_idx = get_free_filter();
	if (flt_idx < 0) {
		DMX_ERROR_PRINT("dmxFilterOpen ===> get_one_filter fail\n");
		ret = DMX_ERROR_NO_MORE_RESOURCES;
		goto EXIT;
	}

	fi = &filter_info_tbl[flt_idx];
	fi->tsid = xTransportSessionId;
	fi->demuxid = PLATFORM_DEMUX_ID;
	fi->depth = xFilterDepth;
	fi->priv_data = pxPrivateData;
	fi->id.id = FILTER_IDX_TO_ID(flt_idx);
	fi->is_used = 1;
	*pxFilterId = &fi->id;

	pthread_mutex_unlock(&dmx_mutex);
	return DMX_NO_ERROR;
EXIT:
	pthread_mutex_unlock(&dmx_mutex);
	return ret;
}


/**
 *  @brief
 *    This function closes the given demux filter
 *
 *  @param[in]  xFilterId
 *                Identifier of the demux filter to be closed
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             This filter resource was not opened
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
*/
TDmxStatus dmxFilterClose
(
  TDmxFilterId   xFilterId
)
{
	int		flt_idx;
	mt_s32		ret;
	struct filter_info	*fi;

	pthread_mutex_lock(&dmx_mutex);

	if (xFilterId == NULL) {
		DMX_ERROR_PRINT("dmxFilterClose ===> xFilterId == NULL fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	if (is_filter_id_valid(xFilterId) < 0)
	{
		DMX_ERROR_PRINT("dmxFilterClose ===> xFilterId == not found\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	flt_idx = FILTER_ID_TO_IDX(xFilterId->id);
	if(flt_idx < 0 || flt_idx >= FILTER_NUM_MAX) {
		DMX_ERROR_PRINT("dmxFilterClose ===> flt_idx < 0 || flt_idx >= FILTER_NUM_MAX fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	fi = &filter_info_tbl[flt_idx];
	if (fi->is_start) {
		_dmxFilterStop(xFilterId);
	}

	if (fi->is_used == 0) {
		DMX_ERROR_PRINT("dmxFilterClose ===> fi->is_used == 0 fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	cleanup_channels();
	reset_filter_info(fi);
	pthread_mutex_unlock(&dmx_mutex);
	return DMX_NO_ERROR;
EXIT:
	pthread_mutex_unlock(&dmx_mutex);
	return ret;
}


/**
 *  @brief
 *    This function defines the filtering patterns of the demux filter.
 *
 *    Let's illustrate these parameters by an example. If we would like to catch
 *    a section with the first byte set to 0x42 and the third one different than
 *    0xC4, then the filtering patterns shall be defined as follows:
 *
 *      \c pxValue=0x4200C4          \n
 *      \c pxEqualMask=0xFF0000      \n
 *      \c pxNotEqualMask=0x0000FF   \n
 *
 *  @param[in]  xFilterId
 *                Identifier of the demux filter
 *
 *  @param[in]  pxValue
 *                Section filter value pattern. This pattern is the array of
 *                bytes that are compared to the incoming section. This
 *                pattern is used in combination with the "equal" and
 *                "not-equal" masks. These two masks indicates precisely which
 *                bit of the value pattern are taken into account ( i.e. all
 *                bits not covered by a mask are ignored).
 *
 *  @param[in]  pxEqualMask
 *                Bitmap indicating which bits of the value pattern must match
 *                the incoming section.
 *
 *  @param[in]  pxNotEqualMask
 *                Bitmap indicating which bits of the value pattern must not match
 *                the incoming section.
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR_FILTER_RUNNING
 *             The filter is still running
 *
 *  @retval  DMX_ERROR_BAD_PARAMETER
 *             The given parameters are inconsistent
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
 *  @remarks
 *    -# The size of the patterns is equal to the filter depth given in
 *       \c dmxFilterOpen().
 *    .
 *    -# This function shall only be called when the filter is stopped.
 *    .
 *    -# The \c pxEqualMask and \c pxNotEqualMask masks must not overlap
 *       (the bit-and operation of these two masks shall be equal to 0).
 *       \c DMX_ERROR_BAD_PARAMETER shall be returned when masks overlap.
*/
static TDmxStatus _dmxFilterSetPatterns
(
        TDmxFilterId   xFilterId,
  const TUnsignedInt8* pxValue,
  const TUnsignedInt8* pxEqualMask,
  const TUnsignedInt8* pxNotEqualMask
)
{
	int		flt_idx;
	mt_s32		ret;
	mt_u32		i;
	struct filter_info	*fi;

	if (xFilterId == NULL) {
		DMX_ERROR_PRINT("dmxFilterSetPatterns ===> xFilterId == NULL fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	flt_idx = FILTER_ID_TO_IDX(xFilterId->id);
	if(flt_idx < 0 || flt_idx >= FILTER_NUM_MAX) {
		DMX_ERROR_PRINT("dmxFilterSetPatterns ===> flt_idx < 0 || flt_idx >= FILTER_NUM_MAX fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}
	fi = &filter_info_tbl[flt_idx];

	if (fi->is_used == 0) {
		DMX_ERROR_PRINT("dmxFilterSetPatterns ===> filter_info_tbl[flt_idx].is_used == 0 fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	if (fi->is_start == 1) {
		DMX_ERROR_PRINT("dmxFilterSetPatterns ===> filter_info_tbl[flt_idx].is_start == 1 fail\n");
		ret = DMX_ERROR_FILTER_RUNNING;
		goto EXIT;
	}

	for (i = 0; i < fi->depth; i ++) {
		if (pxEqualMask[i] & pxNotEqualMask[i]) {
			DMX_ERROR_PRINT("dmxFilterSetPatterns ===> DMX_ERROR_BAD_PARAMETER fail\n");
			ret = DMX_ERROR_BAD_PARAMETER;
			goto EXIT;
		}
	}

	memcpy(fi->flt_val, pxValue, fi->depth);
	memcpy(fi->flt_mask, pxEqualMask, fi->depth);
	memcpy(fi->flt_nmask, pxNotEqualMask, fi->depth);
	return DMX_NO_ERROR;
EXIT:
	return ret;
}

TDmxStatus dmxFilterSetPatterns
(
        TDmxFilterId   xFilterId,
  const TUnsignedInt8* pxValue,
  const TUnsignedInt8* pxEqualMask,
  const TUnsignedInt8* pxNotEqualMask
)
{
	TDmxStatus ret;

	pthread_mutex_lock(&dmx_mutex);

	if (is_filter_id_valid(xFilterId) < 0)
	{
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	if (pxValue == NULL || pxEqualMask == NULL || pxNotEqualMask == NULL)
	{
		ret = DMX_ERROR_BAD_PARAMETER;
		goto EXIT;
	}

	ret = _dmxFilterSetPatterns(xFilterId, pxValue, pxEqualMask, pxNotEqualMask);

EXIT:
	pthread_mutex_unlock(&dmx_mutex);
	return ret;
}
/**
 *  @brief
 *    This function set the PID of packet to be filtered
 *
 *  @param[in]  xFilterId
 *                Identifier of the filter
 *
 *  @param[in]  xPid
 *                PID of the packet to be filtered
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR_BAD_PID
 *             The PID is out of range
 *
 *  @retval  DMX_ERROR_FILTER_RUNNING
 *             The filter is still running
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
 *  @remarks
 *    -# This function can called only if the filter is stopped.
*/
TDmxStatus dmxFilterSetPid
(
  TDmxFilterId    xFilterId,
  TPid            xPid
)
{
	int		flt_idx;
	mt_s32		ret;
	struct filter_info	*fi;

	pthread_mutex_lock(&dmx_mutex);
	if (xFilterId == NULL) {
		DMX_ERROR_PRINT("dmxFilterSetPid ===> xFilterId == NULL fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	if (is_filter_id_valid(xFilterId) < 0) {
		DMX_ERROR_PRINT("dmxFilterSetPid ===> xFilterId == found fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	if (xPid >= PID_MAX_VALUE)
	{
		DMX_ERROR_PRINT("dmxFilterSetPid ===> pid value fail\n");
		ret = DMX_ERROR_BAD_PID;
		goto EXIT;
	}

	flt_idx = FILTER_ID_TO_IDX(xFilterId->id);
	if(flt_idx < 0 || flt_idx >= FILTER_NUM_MAX) {
		DMX_ERROR_PRINT("dmxFilterSetPid ===> flt_idx < 0 || flt_idx >= FILTER_NUM_MAX fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	fi = &filter_info_tbl[flt_idx];

	if (fi->is_used == 0) {
		DMX_ERROR_PRINT("dmxFilterSetPid ===> filter_info_tbl[flt_idx].is_used == 0 fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	if (fi->is_start == 1) {
		DMX_ERROR_PRINT("dmxFilterSetPid ===> filter_info_tbl[flt_idx].is_start == 1 fail\n");
		ret = DMX_ERROR_FILTER_RUNNING;
		goto EXIT;
	}

	if (xPid > PID_MAX_VALUE || xPid == PID_INVALID) {
		DMX_ERROR_PRINT("dmxFilterSetPid ===> DMX_ERROR_BAD_PID fail\n");
		ret = DMX_ERROR_FILTER_RUNNING;
		goto EXIT;
	}

	fi->pid = xPid;
	pthread_mutex_unlock(&dmx_mutex);
	return DMX_NO_ERROR;

EXIT:
	pthread_mutex_unlock(&dmx_mutex);
	return ret;
}

/**
 *  @brief
 *    This function starts filtering on a specified filter.
 *
 *  @param[in]  xFilterId
 *                Identifier of the filter
 *
 *  @param[in]  xTimeout
 *                Timeout on the filter in [ms]. If 0, the timeout is considered
 *                to be infinite. If the timeout expires and no section has been
 *                catched, the notification callbacks must be called anyway with
 *                \c pxSection=NULL and \c xSectionSize=0.
 *
 *  @param[in]  xLoopMode
 *                This mode defines the behavior of the filter right after
 *                catching a matching section:
 *                - \c DMX_LOOP_CONTINUOUS: the filter remains active as long as
 *                  not explicitely stopped by \c calling \c dmxFilterStop().
 *                - \c DMX_LOOP_ONE_SHOT: the filter automatically stops after
 *                  catching a section.
 *                - \c DMX_LOOP_TOGGLE: this mode is used to acquire sections
 *                  broadcast alternately on two different tables. The filter
 *                  remains active as long as not explicitely stopped by
 *                  calling \c dmxFilterStop().
 *
 *  @param[in]  xCrcMode
 *                Indicate whether the CRC has to be checked or not:
 *                - \c DMX_CRC_CHECK
 *                - \c DMX_CRC_IGNORE
 *
 *  @param[in]  xQueryBufferCallback
 *                Callback used by the DMX driver to get a buffer in order to
 *                store a matching section.
 *
 *  @param[in]  xReceivedSectionCallback
 *                Callback used by the DMX driver to notify the CA software of
 *                that a section matching the filter pattern has been acquired.
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR_FILTER_RUNNING
 *             The filter is already running
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
 *  @remarks
 *    -# Both callbacks are mandatory
 *    .
 *    -# After a one shot filtering, the filter is simply stopped and can be
 *       restarted at any time with \c dmxFilterStart().
 *    .
 *    -# This function can only be called when the filter is stopped.
*/
static TDmxStatus _dmxFilterStart
(
  TDmxFilterId                        xFilterId,
  TOsTime                             xTimeout,
  TDmxLoopMode                        xLoopMode,
  TDmxCrcMode                         xCrcMode,
  TDmxFilterQueryBufferCallback       xQueryBufferCallback,
  TDmxFilterReceivedSectionCallback   xReceivedSectionCallback
)
{
	int				flt_idx;
	mt_s32				ret;
	MT_UNF_DMX_CHAN_ATTR_S	chn_attr;
	MT_UNF_DMX_FILTER_ATTR_S	flt_attr;
	struct filter_info			*fi;
	mt_u32				i;
	mt_handle 			chn_hdl;
	int 				new_chn = 0;

	if (xFilterId == NULL) {
		DMX_ERROR_PRINT("dmxFilterSetPid ===> xFilterId == NULL fail\n");
		return DMX_ERROR_UNKNOWN_ID;
	}

	flt_idx = FILTER_ID_TO_IDX(xFilterId->id);
	if(flt_idx < 0 || flt_idx >= FILTER_NUM_MAX) {
		DMX_ERROR_PRINT("dmxFilterSetPid ===> flt_idx < 0 || flt_idx >= FILTER_NUM_MAX fail\n");
		return DMX_ERROR_UNKNOWN_ID;
	}
	fi = &filter_info_tbl[flt_idx];

	if (fi->is_used == 0) {
		DMX_ERROR_PRINT("dmxFilterSetPid ===> filter_info_tbl[flt_idx].is_used == 0 fail\n");
		return DMX_ERROR_UNKNOWN_ID;
	}

	if (fi->is_start == 1) {
		DMX_ERROR_PRINT("dmxFilterSetPid ===> filter_info_tbl[flt_idx].is_start == 1 fail\n");
		return DMX_ERROR_FILTER_RUNNING;
	}

	if (xLoopMode > DMX_LOOP_TOGGLE || xCrcMode > DMX_CRC_IGNORE ||
		xQueryBufferCallback == NULL || xReceivedSectionCallback == NULL) {
		DMX_ERROR_PRINT("dmxFilterSetPid ===>DMX_ERROR_BAD_PARAMETER fail\n");
		return DMX_ERROR;
	}

	if (xTimeout == 0)
		fi->timeout_mode = TIMEOUT_MODE_FOREVER;
	else
		fi->timeout_mode = TIMEOUT_MODE_WAIT;

	fi->timeout = xTimeout;
	fi->loop_mode = xLoopMode;
	fi->crc_mode = xCrcMode;
	fi->qbuf_cb = xQueryBufferCallback;
	fi->rsec_cb = xReceivedSectionCallback;

	//ret = MT_UNF_DMX_GetChannelHandle(fi->demuxid, fi->pid, &chn_hdl);
	ret = MT_UNF_DMX_GetChannelHandleByPidType(fi->demuxid, fi->pid,MT_UNF_DMX_CHAN_TYPE_SEC, &chn_hdl);
	if (ret != MT_SUCCESS) {
		new_chn = 1;
		MT_UNF_DMX_GetChannelDefaultAttr(&chn_attr);
		chn_attr.u32BufSize = FILTER_BUFFER_SIZE;
		if (fi->flt_val[0] >= 0x82 && fi->flt_val[0] <= 0x8F) {
			chn_attr.u32BufSize = FILTER_BIG_BUFFER_SIZE;
		}
		chn_attr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
		chn_attr.enCRCMode = fi->crc_mode ==DMX_CRC_CHECK ?
						MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_DISCARD :
						MT_UNF_DMX_CHAN_CRC_MODE_FORBID;
		chn_attr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
		ret = MT_UNF_DMX_CreateChannel(fi->demuxid, &chn_attr, &chn_hdl);
		if (MT_SUCCESS != ret) {
			DMX_ERROR_PRINT("create_slot ===> MT_UNF_DMX_CreateChannel fail %d\n", __LINE__);
			goto EXIT;
		}
		ret = MT_UNF_DMX_SetChannelPID(chn_hdl, fi->pid);
		if (MT_SUCCESS != ret) {
			DMX_ERROR_PRINT("create_slot ===> MT_UNF_DMX_SetChannelPID fail %d\n", __LINE__);
			goto CHANNEL_DESTROY;
		}

	}

	memset(&flt_attr, 0, sizeof(flt_attr));
	//au8Mask = ~ (flt_mask | flt_nmask)
	//au8Negate = flt_nmask == 0 ? 0 : 1
	for (i = 0; i < fi->depth; i ++) {
		flt_attr.au8Match[i] = fi->flt_val[i];
		flt_attr.au8Mask[i] = (mt_u8)~ (fi->flt_mask[i] | fi->flt_nmask[i]);
		flt_attr.au8Negate[i] = fi->flt_nmask[i] == 0 ? 0 : 1 ;
	}
	memmove(&flt_attr.au8Match[1], &flt_attr.au8Match[3], DMX_FILTER_MAX_DEPTH -3);
	memmove(&flt_attr.au8Mask[1], &flt_attr.au8Mask[3], DMX_FILTER_MAX_DEPTH -3);
	memmove(&flt_attr.au8Negate[1], &flt_attr.au8Negate[3], DMX_FILTER_MAX_DEPTH -3);

	if (fi->depth > 3) {
		flt_attr.u32FilterDepth = fi->depth - 2;
	} else {
		flt_attr.u32FilterDepth = 1;
	}

	ret = MT_UNF_DMX_CreateFilter(fi->demuxid, &flt_attr, &fi->flt_hdl);
	if (MT_SUCCESS != ret) {
		DMX_ERROR_PRINT("dmxFilterStart ===> MT_UNF_DMX_CreateFilter fail %d\n", __LINE__);
		goto CHANNEL_DESTROY;
	}

	ret = MT_UNF_DMX_SetFilterAttr(fi->flt_hdl, &flt_attr);
	if (MT_SUCCESS != ret) {
		DMX_ERROR_PRINT("dmxFilterStart ===> MT_UNF_DMX_SetFilterAttr fail %d\n", __LINE__);
		goto FILTER_DESTROY;
	}

	ret = MT_UNF_DMX_AttachFilter(fi->flt_hdl, chn_hdl);
	if (MT_SUCCESS != ret){
		DMX_ERROR_PRINT("dmxFilterStart ===> MT_UNF_DMX_AttachFilter fail %d\n", __LINE__);
		goto FILTER_DESTROY;
	}
	ret = MT_UNF_DMX_OpenChannel(chn_hdl);
	if (MT_SUCCESS != ret) {
		DMX_ERROR_PRINT("dmxFilterStart ===> MT_UNF_DMX_OpenChannel fail %d\n", __LINE__);
		goto FILTER_DETACH;
	}

	fi->is_start = 1;
	inc_pid_count(fi->demuxid,fi->pid);

	fi->start_time = get_tick_time();
	DMX_DEBUG_PRINT("\n fi->start_time =%lld, fi->timeout =%ld, fi->timeout_mode =0x%x, fi->id.id =%d\n" ,fi->start_time, fi->timeout, fi->timeout_mode, fi->id.id);
	fi->chn_hdl = chn_hdl;
	return DMX_NO_ERROR;
FILTER_DETACH:
	MT_UNF_DMX_DetachFilter(fi->flt_hdl, chn_hdl);
FILTER_DESTROY:
	MT_UNF_DMX_DestroyFilter(fi->flt_hdl);
CHANNEL_DESTROY:
	if (new_chn)
		MT_UNF_DMX_DestroyChannel(chn_hdl);
EXIT:
	return DMX_ERROR;
}

TDmxStatus dmxFilterStart
(
  TDmxFilterId                        xFilterId,
  TOsTime                             xTimeout,
  TDmxLoopMode                        xLoopMode,
  TDmxCrcMode                         xCrcMode,
  TDmxFilterQueryBufferCallback       xQueryBufferCallback,
  TDmxFilterReceivedSectionCallback   xReceivedSectionCallback
)
{
	TDmxStatus ret;

	pthread_mutex_lock(&dmx_mutex);

	if (is_filter_id_valid(xFilterId) < 0)
	{
		DMX_ERROR_PRINT("dmxFilterStart ===> xFilterId found fail\n");
		pthread_mutex_unlock(&dmx_mutex);
		return DMX_ERROR_UNKNOWN_ID;
	}

	ret =  _dmxFilterStart(xFilterId, xTimeout, xLoopMode, xCrcMode,
			xQueryBufferCallback, xReceivedSectionCallback);
	pthread_mutex_unlock(&dmx_mutex);

	return ret;
}


/**
 *  @brief
 *    This function stops the filtering on a specified filter and freezes the
 *    buffer in its current state.
 *
 *  @param[in]  xFilterId
 *                Identifier of the filter
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
 *  @remarks
 *    -# This function does flush the reception buffer.
*/
static TDmxStatus _dmxFilterStop
(
  TDmxFilterId   xFilterId
)
{
	int		flt_idx;
	mt_s32		ret;
	struct filter_info	*fi;
	mt_handle 	chn_hdl;

	if (xFilterId == NULL) {
		DMX_ERROR_PRINT("dmxFilterStop ===> xFilterId == NULL fail\n");
		return DMX_ERROR_UNKNOWN_ID;
	}

	flt_idx = FILTER_ID_TO_IDX(xFilterId->id);
	if(flt_idx < 0 || flt_idx >= FILTER_NUM_MAX) {
		DMX_ERROR_PRINT("dmxFilterStop ===> flt_idx < 0 || flt_idx >= FILTER_NUM_MAX fail\n");
		return DMX_ERROR_UNKNOWN_ID;
	}
	fi = &filter_info_tbl[flt_idx];

	if (fi->is_used == 0) {
		DMX_ERROR_PRINT("dmxFilterStop ===> filter_info_tbl[flt_idx].is_used == 0 fail\n");
		return DMX_ERROR_UNKNOWN_ID;
	}

	if (fi->is_start == 0) {
		return DMX_NO_ERROR;
	}

	if(fi->flt_hdl)
	{
		//ret = MT_UNF_DMX_GetChannelHandle(fi->demuxid, fi->pid, &chn_hdl);
		ret = MT_UNF_DMX_GetChannelHandleByPidType(fi->demuxid, fi->pid, MT_UNF_DMX_CHAN_TYPE_SEC, &chn_hdl);
		if (MT_SUCCESS != ret) {
			DMX_ERROR_PRINT("dmxFilterStop ===> MT_UNF_DMX_GetChannelHandle fail %d\n", __LINE__);
			return DMX_ERROR;
		}

		if (fi->chn_hdl != chn_hdl)
		{
			DMX_ERROR_PRINT("dmxFilterStop ===> Channel doesn't matched %d( 0x%x, 0x%x )\n", __LINE__, fi->chn_hdl, chn_hdl);
			return DMX_ERROR;
		}
	ret = MT_UNF_DMX_DetachFilter(fi->flt_hdl, chn_hdl);
	if (MT_SUCCESS != ret) {
		DMX_ERROR_PRINT("dmxFilterStop ===> MT_UNF_DMX_DetachFilter fail %d\n", __LINE__);
		return DMX_ERROR;
	}

	ret = MT_UNF_DMX_DestroyFilter(fi->flt_hdl);
	if (MT_SUCCESS != ret) {
		DMX_ERROR_PRINT("dmxFilterStop ===> MT_UNF_DMX_DestroyFilter fail %d\n", __LINE__);
		return DMX_ERROR;
	}
		fi->flt_hdl = 0;
		dec_pid_count(fi->demuxid, fi->pid);
	}
	fi->is_start = 0;

	return DMX_NO_ERROR;
}

TDmxStatus dmxFilterStop
(
  TDmxFilterId   xFilterId
)
{
	TDmxStatus ret;

	pthread_mutex_lock(&dmx_mutex);

	if (is_filter_id_valid(xFilterId) < 0)
	{
		pthread_mutex_unlock(&dmx_mutex);
		return DMX_ERROR_UNKNOWN_ID;
	}

	ret = _dmxFilterStop(xFilterId);
	pthread_mutex_unlock(&dmx_mutex);
	return ret;
}

/**
 *  @brief
 *    This function resets the filtering on a specified filter by flushing the
 *    buffer and resetting the timeout.
 *
 *  @param[in]  xFilterId
 *                Identifier of the filter
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
*/
TDmxStatus dmxFilterReset
(
  TDmxFilterId   xFilterId
)
{
	int		flt_idx;
	mt_s32		ret;
	struct filter_info	*fi;
	mt_handle 	chn_hdl;

	pthread_mutex_lock(&dmx_mutex);

	if (xFilterId == NULL) {
		DMX_ERROR_PRINT("dmxFilterReset ===> xFilterId == NULL fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	if (is_filter_id_valid(xFilterId) < 0) {
		DMX_ERROR_PRINT("dmxFilterReset ===> xFilterId == found fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	flt_idx = FILTER_ID_TO_IDX(xFilterId->id);
	if(flt_idx < 0 || flt_idx >= FILTER_NUM_MAX) {
		DMX_ERROR_PRINT("dmxFilterReset ===> flt_idx < 0 || flt_idx >= FILTER_NUM_MAX fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	fi = &filter_info_tbl[flt_idx];

	if (fi->is_used == 0) {
		DMX_ERROR_PRINT("dmxFilterReset ===> filter_info_tbl[flt_idx].is_used == 0 fail\n");
		ret = DMX_ERROR_UNKNOWN_ID;
		goto EXIT;
	}

	if (fi->is_start) {
		ret = _dmxFilterStop(xFilterId);
		if (ret != DMX_NO_ERROR) {
			DMX_ERROR_PRINT("dmxFilterReset ===> dmxFilterStop fail\n");
			ret = DMX_ERROR;
			goto EXIT;
		}
	}

	//ret = MT_UNF_DMX_GetChannelHandle(fi->demuxid, fi->pid, &chn_hdl);
	ret = MT_UNF_DMX_GetChannelHandleByPidType(fi->demuxid, fi->pid, MT_UNF_DMX_CHAN_TYPE_SEC, &chn_hdl);
	if (MT_SUCCESS != ret) {
		DMX_ERROR_PRINT("dmxFilterStop ===> MT_UNF_DMX_GetChannelHandle fail %d\n", __LINE__);
		ret = DMX_ERROR;
		goto EXIT;
	}
	ret = MT_UNF_DMX_CloseChannel(chn_hdl);
	if (ret != MT_SUCCESS) {
		DMX_ERROR_PRINT("dmxFilterReset ===> MT_UNF_DMX_CloseChannel fail\n");
		ret = DMX_ERROR;
		goto EXIT;
	}

	ret = _dmxFilterStart(xFilterId,
			(TOsTime)fi->timeout,
			fi->loop_mode,
			fi->crc_mode,
			fi->qbuf_cb,
			fi->rsec_cb);
	if (ret != DMX_NO_ERROR) {
		DMX_ERROR_PRINT("dmxFilterReset ===> dmxFilterStart fail\n");
		ret = DMX_ERROR;
		goto EXIT;
	}
	//fi->time_count = fi->timeout;
	DMX_DEBUG_PRINT("dmxFilterReset ===> fi->id.id = %d  dmxFilterStart success\n", fi->id.id);
	pthread_mutex_unlock(&dmx_mutex);
	return DMX_NO_ERROR;
EXIT:
	pthread_mutex_unlock(&dmx_mutex);
	return ret;
}

void dmxMutexLock(void)
{
	pthread_mutex_lock(&dmx_mutex);
}

void dmxMutexUnlock(void)
{
	pthread_mutex_unlock(&dmx_mutex);
}
