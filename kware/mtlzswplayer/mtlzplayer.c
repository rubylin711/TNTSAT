/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 201, Montage Technology Co., Ltd.
 *
 * File Name      : mtlzplayer.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/14
 * Description    : Monage-LZ SW Player implementation.
 * History        :
 * 1.Date         : 2019/03/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <string.h>
#include <stdbool.h>

#include "mtlz_types.h"
#include "mtlzplayer_comp.h"
#include "mtlzplayer.h"
#include "mtlzplayer_os.h"

#define MAX_PLAYER_COUNT 		1

static void *mtlzplayer_mutex = NULL;
#define LOCK()				do {										\
								if (mtlzplayer_mutex == NULL)			\
									mtlz_mutex_init(&mtlzplayer_mutex);	\
								mtlz_mutex_lock(mtlzplayer_mutex);		\
							} while (0)
#define UNLOCK()			do{mtlz_mutex_unlock(mtlzplayer_mutex);}while(0)

enum mtlz_player_decode_state_e
{
	MT_PLAYER_STATE_BUFFER = 0,
	MT_PLAYER_STATE_BUFFER_DONE,
	MT_PLAYER_STATE_DECODE_AGAIN,
	MT_PLAYER_STATE_DECODE_DONE,
	MT_PLAYER_STATE_DISPLAY_DONE,
};

static struct mtlz_player_st
{
	enum mtlzplayer_status_e status;

	char url[256];

	struct mtlzplayer_comp_source_st *pSource;
	struct mtlzplayer_comp_packer_st *pPacker;
	struct mtlzplayer_comp_codec_st *pCodec;
	struct mtlzplayer_comp_sink_st *pSink;

	int fd;								/* source file handle */
	int packer;
	int decoder;
	int sink;

	volatile int source_eos;
	volatile int all_eos;

	void *in_buffer;					/* input buffer */

	struct mtlzplayer_packet_st packet;	/* input packet */

	struct mtlzplayer_frame_st frame;	/* output frame */

	//pthread_t thread_id;
	unsigned int thread_id;
#ifdef __UC_OS__
	int thread_priority;
#endif

	volatile bool stop;

	enum mtlz_player_decode_state_e state;

	//debug
	unsigned long read_ok_times;
	unsigned long read_fail_times;
	unsigned long read_data_size;
	unsigned long decoded_frm_cnt;
	unsigned long display_ok_frm_cnt;
	unsigned long display_fail_frm_cnt;
	unsigned long first_tick;
	unsigned long last_tick;
	int cal_fps;						/* calculated real fps */
	unsigned long ticks[4];

} mtlz_player[MAX_PLAYER_COUNT];

#define CHECK_HANDLE(hdl)		MTLZ_ASSERT(hdl>=0 && hdl < MAX_PLAYER_COUNT)
#define GET_PLAYER(hdl)			&mtlz_player[hdl]

static void *mtlzplayer_decode_thread(void *arg)
{
	int ret;

	struct mtlz_player_st *pPlayer = (struct mtlz_player_st *)arg;
	struct mtlzplayer_comp_source_st *pSource = pPlayer->pSource;
	struct mtlzplayer_comp_packer_st *pPacker = pPlayer->pPacker;
	struct mtlzplayer_comp_codec_st *pCodec = pPlayer->pCodec;
	struct mtlzplayer_comp_sink_st *pSink = pPlayer->pSink;
	int fd = pPlayer->fd;
	int packer = pPlayer->packer;
	int decoder = pPlayer->decoder;
	int sink = pPlayer->sink;
	//void *in_buffer = pPlayer->in_buffer;
	unsigned char *in_buffer = (unsigned char *)pPlayer->in_buffer;
	ssize_t count;
	struct mtlzplayer_packet_st *packet = &pPlayer->packet;
	struct mtlzplayer_frame_st *frame = &pPlayer->frame;

	enum mtlz_player_decode_state_e state;
	char *pLogExe;
	int sleep_interval = 0;	//ms

	MTLZ_DEBUG("Enter %s.\n",__FUNCTION__);

	//prctl(PR_SET_NAME, __FUNCTION__);
	mtlz_thread_set_name(__FUNCTION__);

	while (!pPlayer->stop && !pPlayer->all_eos)
	{
		pPlayer->ticks[0] = mtlz_get_tick();
		MTLZ_TRACE_TIME("sch used time", pPlayer->ticks[1], pPlayer->ticks[0]);
		pLogExe = "exe used time";

		state = pPlayer->state;
		sleep_interval = 0;

		switch (state)
		{
			case MT_PLAYER_STATE_BUFFER:
			{
				count = IN_BUFFER_SIZE;

				pLogExe = "read used time";

				//check if still has packet
				if (pPacker != NULL)
				{
					ret = pPacker->read_packet(packer, packet, pPlayer->source_eos);	/* EOS Proc-2 */
					if (ret == MTLZ_SUCCESS)
					{
						pPlayer->state = MT_PLAYER_STATE_BUFFER_DONE;
						pLogExe = "read pkt used time";
						break;
					}

					ret = pPacker->get_buffer(packer, &in_buffer, (unsigned int *)&count);
					if (ret != MTLZ_SUCCESS)
					{
						break;
					}
				}

				/* EOS Proc-3 */
				//source eos(1), and last packet read(2) => NUll(3) => codec decode cached buffer(4)
				if (pPlayer->source_eos)
				{
					//let codec decode cached buffer
					packet->data = NULL;
					packet->size = 0;
					pPlayer->state = MT_PLAYER_STATE_BUFFER_DONE;
					MTLZ_DEBUG("%s: 3-eos null packet.\n",__FUNCTION__);
					break;
				}

				count = pSource->read(fd, (void*)in_buffer, (size_t)count);
				if (count > 0)
				{
					if (pPacker != NULL)
					{
						pPacker->put_buffer(packer, in_buffer, (unsigned int)count);
					}
					else
					{
						/**
						 * Required number of additionally allocated bytes at the end of the input bitstream for decoding.
						 * This is mainly needed because some optimized bitstream readers read
						 * 32 or 64 bit at once and could read over the end.<br>
						 * Note: If the first 23 bits of the additional bytes are not 0, then damaged
						 * MPEG bitstreams could cause overread and segfault.
						 */
						if (count < IN_BUFFER_SIZE)
							memset((void*)(in_buffer+count), 0, (size_t)(IN_BUFFER_SIZE-count));

						packet->data = in_buffer;
						packet->size = (unsigned int)count;
						pPlayer->state = MT_PLAYER_STATE_BUFFER_DONE;
					}

					pPlayer->read_data_size += (unsigned long)count;
					pPlayer->read_ok_times ++;
				}
				else
				{
					if (pSource->eos(fd))
					{
						pPlayer->source_eos = 1;	/* EOS Proc-1 */
						MTLZ_DEBUG("%s: 1-source eos reached.\n",__FUNCTION__);
						break;
					}

					//TODO
					//warning if always failed
					pPlayer->read_fail_times ++;

					//if no data, sleep a while
					sleep_interval = MTLZPLAYER_SLEEP_INTERVAL;
				}
				break;
			}

			case MT_PLAYER_STATE_BUFFER_DONE:
			case MT_PLAYER_STATE_DECODE_AGAIN:
			case MT_PLAYER_STATE_DISPLAY_DONE:
			{
				pLogExe = "decode used time";

				if (pPlayer->decoded_frm_cnt == 0)
					pPlayer->first_tick = mtlz_get_tick();

				ret = pCodec->decode(decoder, packet, frame);
				if (ret == MTLZ_CODEC_RET_BUFFER)
				{
					pPlayer->state = MT_PLAYER_STATE_BUFFER;
				}
				else if (ret == MTLZ_CODEC_RET_FRAME && frame->buf_y != NULL)
				{
					pPlayer->state = MT_PLAYER_STATE_DECODE_DONE;
					pPlayer->decoded_frm_cnt ++;
				}
				else if (ret == MTLZ_CODEC_RET_UNKNOWN_ERROR)
				{
					MTLZ_ERROR("[ERROR]%s: something is wrong with the codec(%p)!\n",__FUNCTION__,pCodec);
					//TODO
					//reset codec?
					pPlayer->state = MT_PLAYER_STATE_BUFFER;
				}
				else
				{
					pPlayer->state = MT_PLAYER_STATE_DECODE_AGAIN;
				}

				/* EOS Proc-4 */
				if (pPlayer->state == MT_PLAYER_STATE_BUFFER
					&& pPlayer->source_eos
					&& packet->data == NULL)
				{
					pPlayer->all_eos = 1;
					MTLZ_DEBUG("%s: 4-all eos reached.\n",__FUNCTION__);
				}
				break;
			}

			case MT_PLAYER_STATE_DECODE_DONE:
			{
				pLogExe = "display used time";

				if (pPlayer->display_ok_frm_cnt == 0
					&& pPlayer->display_fail_frm_cnt == 0)
				{
					sink = pPlayer->sink = pSink->setup((int)frame->width, (int)frame->height);
					//TODO
					//check sink
				}

				ret = pSink->draw(sink, frame);
				if (ret == MTLZ_SUCCESS)
				{
					pPlayer->display_ok_frm_cnt ++;
				}
				else
				{
					pPlayer->display_fail_frm_cnt ++;
				}

				//no matter the return value here
				pPlayer->state = MT_PLAYER_STATE_DISPLAY_DONE;

				pPlayer->last_tick = mtlz_get_tick();
				if ((pPlayer->decoded_frm_cnt % 100) == 0)
				{
					/* unsigned long long: avoid (uint) * 1000000 flow */
					pPlayer->cal_fps = (int)((unsigned long long)pPlayer->decoded_frm_cnt * 1000000 /
										 (pPlayer->last_tick - pPlayer->first_tick));
					MTLZ_DEBUG("mtlzplayer: frame count=%lu, FPS=%d.%03d\n",
						pPlayer->decoded_frm_cnt,
						pPlayer->cal_fps/1000,
						pPlayer->cal_fps%1000);
				}

				//sleep a while each frame(lose a little performance),
				//or other task will not be scheduled.
				#ifdef __UC_OS__
				sleep_interval = MTLZPLAYER_SLEEP_INTERVAL;
				#endif

				break;
			}

			default:
				MTLZ_ERROR("[ERROR]%s: invalid state(%d)!\n",__FUNCTION__,state);
				MTLZ_BUG();
				break;
		}

		pPlayer->ticks[1] = mtlz_get_tick();
		MTLZ_TRACE_TIME(pLogExe, pPlayer->ticks[0], pPlayer->ticks[1]);

		if (sleep_interval)
			mtlz_msleep((unsigned int)sleep_interval);
	}

	if (pPlayer->last_tick - pPlayer->first_tick != 0)
	{
		/* unsigned long long: avoid (uint) * 1000000 flow */
		pPlayer->cal_fps = (int)((unsigned long long)pPlayer->decoded_frm_cnt * 1000000 /
							 	(pPlayer->last_tick - pPlayer->first_tick));
		MTLZ_DEBUG("mtlzplayer: frame count=%lu, FPS=%d.%03d\n",
			pPlayer->decoded_frm_cnt,
			pPlayer->cal_fps/1000,
			pPlayer->cal_fps%1000);
	}

	MTLZ_DEBUG("Leave %s.\n",__FUNCTION__);
	return NULL;
}

int mtlzplayer_open(const char *url)
{
	int source;
	int codec_type;
	int window;
	//FIXME: how about multi players?
	int handle = 0;
	struct mtlz_player_st *pPlayer;

	if (url == NULL)
	{
		MTLZ_ERROR("%s: invalid parameters!\n",__FUNCTION__);
		return MTLZ_EINVAL;
	}

	MTLZ_DEBUG("%s: %s\n",__FUNCTION__,url);

	LOCK();

	pPlayer = GET_PLAYER(handle);

	if (pPlayer->status != MTLZ_PLAYER_STATUS_IDLE)
	{
		MTLZ_ERROR("%s: invalid status(%d)!\n",__FUNCTION__,pPlayer->status);
		UNLOCK();
		return MTLZ_FAILURE;
	}

	memset(pPlayer, 0, sizeof(struct mtlz_player_st));

	strncpy(pPlayer->url, url, 255);

	source = get_source_type_index(url);
	pPlayer->pSource = (struct mtlzplayer_comp_source_st*)mtlz_comp_get(MTLZ_COMP_SOURCE_INDEX, source);
	MTLZ_ASSERT(pPlayer->pSource != NULL);

	codec_type = get_codec_type_index(url);
	//pPlayer->pPacker could be zero!
	pPlayer->pPacker = (struct mtlzplayer_comp_packer_st*)mtlz_comp_get(MTLZ_COMP_PACKER_INDEX, codec_type);

	pPlayer->pCodec = (struct mtlzplayer_comp_codec_st*)mtlz_comp_get(MTLZ_COMP_CODEC_INDEX, codec_type);
	MTLZ_ASSERT(pPlayer->pCodec != NULL);

	window = get_sink_type_index(url);
	pPlayer->pSink = (struct mtlzplayer_comp_sink_st*)mtlz_comp_get(MTLZ_COMP_SINK_INDEX, window);
	MTLZ_ASSERT(pPlayer->pSink != NULL);

	if (pPlayer->pPacker == NULL)
	{
		pPlayer->in_buffer = MTLZ_MALLOC(IN_BUFFER_SIZE+PACKET_PADDING_SIZE);
		MTLZ_ASSERT(pPlayer->in_buffer != NULL);
		memset(pPlayer->in_buffer, 0, IN_BUFFER_SIZE+PACKET_PADDING_SIZE);
		MTLZ_DEBUG("%s: in buffer addr %p, size %d.\n",__FUNCTION__,pPlayer->in_buffer,IN_BUFFER_SIZE);
	}
	else
	{
		pPlayer->in_buffer = NULL;
	}

	pPlayer->status = MTLZ_PLAYER_STATUS_STOP;

	UNLOCK();
	return handle;
}

int mtlzplayer_close(int handle)
{
	int ret;
	struct mtlz_player_st *pPlayer;

	MTLZ_DEBUG("%s: handle %d.\n",__FUNCTION__,handle);

	CHECK_HANDLE(handle);

	LOCK();

	pPlayer = GET_PLAYER(handle);

	if (pPlayer->status == MTLZ_PLAYER_STATUS_IDLE)
	{
		UNLOCK();
		return MTLZ_SUCCESS;
	}

	if (pPlayer->status != MTLZ_PLAYER_STATUS_STOP)
	{
		MTLZ_ERROR("%s: invalid status(%d)!\n",__FUNCTION__,pPlayer->status);
		UNLOCK();
		return MTLZ_EBUSY;
	}

	if (pPlayer->in_buffer != NULL)
	{
		MTLZ_FREE(pPlayer->in_buffer);
		pPlayer->in_buffer = NULL;
	}

	ret = pPlayer->pSink->teardown(pPlayer->sink);

	pPlayer->status = MTLZ_PLAYER_STATUS_IDLE;

	UNLOCK();
	return ret;
}

int mtlzplayer_start(int handle)
{
	int ret;
	struct mtlz_player_st *pPlayer;

	MTLZ_DEBUG("%s: handle %d.\n",__FUNCTION__,handle);

	CHECK_HANDLE(handle);

	LOCK();

	pPlayer = GET_PLAYER(handle);

	if (pPlayer->status != MTLZ_PLAYER_STATUS_STOP)
	{
		MTLZ_ERROR("%s: invalid status(%d)!\n",__FUNCTION__,pPlayer->status);
		UNLOCK();
		return MTLZ_FAILURE;
	}

	pPlayer->fd = pPlayer->pSource->open(pPlayer->url, 0);
	MTLZ_ASSERT(pPlayer->fd != MTLZ_NULL_FD && pPlayer->fd != MTLZ_INVALID_FD);
	if (pPlayer->pPacker != NULL)
	{
		pPlayer->packer = pPlayer->pPacker->init();
		MTLZ_ASSERT(pPlayer->packer != MTLZ_NULL_FD && pPlayer->packer != MTLZ_INVALID_FD);
	}
	pPlayer->decoder = pPlayer->pCodec->init();
	MTLZ_ASSERT(pPlayer->decoder >= 0 && pPlayer->decoder != MTLZ_INVALID_ID);

	pPlayer->state = MT_PLAYER_STATE_BUFFER;

	pPlayer->stop = false;

	pPlayer->status = MTLZ_PLAYER_STATUS_PLAY;

	//ret = pthread_create(&pPlayer->thread_id, NULL, mtlzplayer_decode_thread, (void*)pPlayer);
	ret = mtlz_thread_create(&pPlayer->thread_id, mtlzplayer_decode_thread, (void*)pPlayer);

	UNLOCK();
	return ret;
}

int mtlzplayer_stop(int handle)
{
	int ret;
	struct mtlz_player_st *pPlayer;

	MTLZ_DEBUG("%s: handle %d.\n",__FUNCTION__,handle);

	CHECK_HANDLE(handle);

	LOCK();

	pPlayer = GET_PLAYER(handle);

	if (pPlayer->status == MTLZ_PLAYER_STATUS_STOP)
	{
		UNLOCK();
		return MTLZ_SUCCESS;
	}

	if (pPlayer->status != MTLZ_PLAYER_STATUS_PLAY
		&& pPlayer->status != MTLZ_PLAYER_STATUS_EOS)
	{
		MTLZ_ERROR("%s: invalid status(%d)!\n",__FUNCTION__,pPlayer->status);
		UNLOCK();
		return MTLZ_FAILURE;
	}

	pPlayer->stop = true;

	//ret = pthread_join(pPlayer->thread_id, NULL);
	ret = mtlz_thread_join(pPlayer->thread_id);

	ret |= pPlayer->pSource->close(pPlayer->fd);
	if (pPlayer->pPacker != NULL)
	{
		ret |= pPlayer->pPacker->destroy(pPlayer->packer);
	}
	ret |= pPlayer->pCodec->close(pPlayer->decoder);

	pPlayer->status = MTLZ_PLAYER_STATUS_STOP;

	UNLOCK();
	return ret;
}

int mtlzplayer_get_status(int handle, struct mtlzplayer_status_info_st *status)
{
	struct mtlz_player_st *pPlayer;

	CHECK_HANDLE(handle);

	if (status == NULL)
	{
		MTLZ_ERROR("%s: invalid parameter!\n",__FUNCTION__);
		return MTLZ_EINVAL;
	}

	LOCK();

	pPlayer = GET_PLAYER(handle);

	if (pPlayer->all_eos
		&& pPlayer->status == MTLZ_PLAYER_STATUS_PLAY)
		status->state = MTLZ_PLAYER_STATUS_EOS;
	else
		status->state = pPlayer->status;

	UNLOCK();
	MTLZ_VERBOSE("%s(%d): %d.\n",__FUNCTION__,handle,status->state);
	return MTLZ_SUCCESS;
}

#ifdef __UC_OS__
int mtlzplayer_set_priority(int handle, int priority)
{
	struct mtlz_player_st *pPlayer;

	CHECK_HANDLE(handle);

	LOCK();

	pPlayer = GET_PLAYER(handle);

	if (pPlayer->status != MTLZ_PLAYER_STATUS_STOP)
	{
		MTLZ_ERROR("%s: invalid state(%d)!\n",__FUNCTION__,pPlayer->status);
		UNLOCK();
		return MTLZ_EINVAL;
	}

	MTLZ_DEBUG("%s: priority=%d\n",__FUNCTION__,priority);
	pPlayer->thread_priority = priority;

	UNLOCK();
	return MTLZ_SUCCESS;
}

int mtlzplayer_get_priority_nb(int handle)
{
	struct mtlz_player_st *pPlayer;

	CHECK_HANDLE(handle);

	pPlayer = GET_PLAYER(handle);

	return pPlayer->thread_priority;
}

int mtlzplayer_get_priority(int handle)
{
	struct mtlz_player_st *pPlayer;
	int priority;

	CHECK_HANDLE(handle);

	LOCK();

	pPlayer = GET_PLAYER(handle);

	priority = pPlayer->thread_priority;
	MTLZ_DEBUG("%s: priority=%d\n",__FUNCTION__,priority);

	UNLOCK();
	return priority;
}
#endif

