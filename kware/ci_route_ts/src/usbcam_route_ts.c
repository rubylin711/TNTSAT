/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <mtd/mtd-abi.h>
#include <unistd.h>
#include <linux/sched.h>
#include <errno.h>

#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_unf_ecs.h"
#include "mt_unf_demux.h"

#include "usbcam_route_ts.h"


#define MAX_FRAGMENT_SIZE (188 << 10)
usbcam_play_context_s *g_usbcam_play_context = NULL;
usbcam_init_parm_t g_param = {0};
static int start_fragment = 0;

static u8 fragment_header[] = {
	0x00, 0x48/*LTS_id*/,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00,
};

static  long long write_total = 0;
static  long long push_total = 0;

static void save_usbcam_context(usbcam_play_context_s *context)
{
	g_usbcam_play_context = context;
}

usbcam_play_context_s *get_usbcam_context(void)
{
	return g_usbcam_play_context;
}

/*
static void usbcam_dump_data(u8 *ptr, unsigned int len)
{
	int fd = -1;
	int ret = MT_SUCCESS;
	static int is_open = 0;

	if (0 == is_open) {
		fd = open("/tmp/usbcam_data.ts", O_WRONLY | O_CREAT, 0777);
		if (fd < 0) {
			USBCAM_ERROR_PRINT("open usbcam_data file failed\n");
			return;
		}
		USBCAM_INFO_PRINT("open usbcam_data success\n");
		is_open = 1;
	}

	if (is_open) {
		ret = write(fd, ptr, len);
		if (ret < 0) {
			USBCAM_ERROR_PRINT("write usbcam data file failed\n");
			close(fd);
			return;
		}
		USBCAM_DEBUG_PRINT("write %d data to file\n", len);
	}
}
*/

/*check the data is vaild or invaild by 0x47 (sync byte)*/
static BOOL usbcam_route_check_valid(u8 *p_data, u32 payload)
{
	u32 loopi = 0;

	for (loopi = 0; loopi < payload; loopi += 188) {
		if (((u8)(p_data[loopi])) != 0x47) {
			USBCAM_INFO_PRINT("p_data[%d] %02x fail\n", loopi, p_data[loopi]);
			break;
		}
	}
	if (loopi == payload) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static int usbcam_media_write(int fd_usbcam, unsigned char *pbuf, int len)
{
	int wlen = 0 ;
	int try_cnt = 0;

	if (NULL == pbuf) {
		USBCAM_ERROR_PRINT("%s pbuf is NULL\n", __func__);
		return -1;
	}

try_again:

	if (try_cnt > 50) {
		USBCAM_INFO_PRINT("try write usbcam too many times\n");
		return wlen;
	}
	wlen = write(fd_usbcam, pbuf, len);
	if (wlen < 0) {
		if (errno == EAGAIN) {
			USBCAM_WARN_PRINT("try write usbcam again\n");
			usleep(500000);
			try_cnt ++;
			goto try_again;
		} else {
			USBCAM_ERROR_PRINT("usb cam write failed:%d\n", wlen);
		}
	}

	return wlen;
}

static u32 usbcam_route_add_fragment_header(int usbcam_dev)
{
	u32 ret = 0;
	int try_cnt = 0;

	while (try_cnt < 30 && start_fragment) {
		ret = usbcam_media_write(usbcam_dev, fragment_header, sizeof(fragment_header));
		if (ret > 0) {
			USBCAM_INFO_PRINT("write fragment header success\n");
			break;
		} else if (ret == 0) {
			USBCAM_WARN_PRINT("write fragment header again\n");
			try_cnt++;
		} else {
			USBCAM_ERROR_PRINT("write fragment header failed\n");
			break;
		}
	}

	return ret;
}

static BOOL usbcam_route_del_fragment_header(u8 *pbuf, int rlen)
{

	if (NULL == pbuf || rlen <= 0) {
		USBCAM_ERROR_PRINT("NO DATA to write\n");
		return FALSE;
	}

	if (rlen == sizeof(fragment_header) &&
	    memcmp(pbuf, &fragment_header, sizeof(fragment_header))) {
		USBCAM_INFO_PRINT("fragment header:discard\n");
		return TRUE;
	}

	return FALSE;
}

static int usbcam_media_read(int fd_usbcam, unsigned char *pbuf, int len)
{
	if (NULL == pbuf) {
		USBCAM_ERROR_PRINT("%s pbuf is NULL\n", __func__);
	}

	return read(fd_usbcam, pbuf, MAX_RX_SIZE);
}

static void usbcam_route_write_data(int usbcam_dev, u8 *pbuf, int retlen)
{
	int pos = 0;
	int wlen = 0;
	BOOL ret = 0;
	int try_cnt = 0;

	ret = usbcam_route_check_valid(pbuf, retlen);
	if (ret == FALSE) {
		return;
	}
	while (retlen > 0) {
		/* max write 188*1024 data one time*/

		if (retlen >= MAX_FRAGMENT_SIZE) {
			wlen = usbcam_media_write(usbcam_dev, pbuf + pos, MAX_FRAGMENT_SIZE);
		} else {
			wlen = usbcam_media_write(usbcam_dev, pbuf + pos, retlen);
		}

		if (wlen == 0) {
			USBCAM_WARN_PRINT("usbcam write again\n");
			usleep(5000);
			try_cnt++;
		} else if (wlen > 0) {
			try_cnt = 0;
		}
		if (wlen < 0 || try_cnt > 30) {
			USBCAM_ERROR_PRINT("usb cam write failed:%d\n", wlen);
			break;
		}

		retlen -= wlen;
		pos += wlen;
		usleep(10000);
	}
	return;
}

/**/
static void* usbcam_route_write_task(void *arg)
{
	usbcam_play_context_s *context = NULL;
	MT_UNF_STREAM_BUF_S     streambuf;
	mt_s32                  read_len;
	mt_s32                  ret;
	mt_u32                  pushLen = MAX_RX_SIZE; //*10;//188*1024;
	//mt_u32                  push_total_len = 0;

	if (arg == NULL) {
		USBCAM_ERROR_PRINT("the usbcam inter param is NULL\n");
		return NULL;
	}
	context = arg;

	while (context->push_data_task.exit == 0) {
		if (context->push_data_task.run == 0) {
			USBCAM_PUSH_DATA_PRINT("wait for start %s\n", __func__);
			usleep(4000);
			continue;
		}

		if (context->usbcam_state == FALSE) {
			USBCAM_ERROR_PRINT("usbcam not ready when read\n");
			usleep(4000);
			continue;
		}

		USBCAM_PUSH_DATA_PRINT("get ts buffer\n");
		ret = MT_UNF_DMX_GetTSBuffer(context->ram_handle, pushLen, &streambuf, 100);//1000
		if (ret != MT_SUCCESS ) {
			USBCAM_ERROR_PRINT("!!!!!!!!MT_UNF_DMX_GetTSBuffer  failure\n");
			continue;
		}

		USBCAM_PUSH_DATA_PRINT("read usbcam,streambuf.pu8Data = %p\n", streambuf.pu8Data);
		read_len = usbcam_media_read(context->usbcam_dev, streambuf.pu8Data, streambuf.u32Size);
		if (read_len < 0) {
			//USBCAM_WARN_PRINT("read usbcam again\n");
			usleep(4000);
			continue;
		}
		if (read_len == 0) {
			USBCAM_WARN_PRINT("read len is 0\n");
			usleep(4000);
			continue;
		}
		if (read_len % 188 != 0) {
			USBCAM_ERROR_PRINT("read_len is not 188 == %d\n", read_len);
		}

		if(start_fragment) {
			ret = usbcam_route_del_fragment_header(streambuf.pu8Data, read_len);
			if (TRUE == ret) {
				USBCAM_INFO_PRINT("%s del the fragment header\n", __func__);
				continue;
			}
		}
		//usbcam_dump_data(streambuf.pu8Data, read_len);

		//push_total_len += read_len;
		ret = MT_UNF_DMX_PutTSBuffer(context->ram_handle, read_len);
		if (ret != MT_SUCCESS ) {
			USBCAM_ERROR_PRINT("call MT_UNF_DMX_PutTSBuffer failed.\n");
		}
		push_total += read_len;

		USBCAM_PUSH_DATA_PRINT("read %d data from usbcam and push to display module\n", read_len);
		usleep(10000);
	}


	ret = MT_UNF_DMX_ResetTSBuffer(context->ram_handle);
	if (MT_SUCCESS != ret) {
		USBCAM_ERROR_PRINT("call MT_UNF_DMX_ResetTSBuffer failed.\n");
	}

	USBCAM_PUSH_DATA_PRINT("exit %s\n", __func__);
	context->push_data_task.run = 0;
	context->push_data_task.exit = 0;

	return NULL;
}

/*
*/
static void *usbcam_route_read_task(void *arg)
{
	usbcam_play_context_s *context = arg;
	record_info_t *record_info = &(context->record_info);

	int ret;
	MT_UNF_DMX_REC_DATA_S rec_data;

	while (context->get_data_task.exit == 0) {//if exit == 1 ，exit the task
		if (context->get_data_task.run == 0) {
			USBCAM_GET_REC_PRINT("wait for start %s\n", __func__);
			usleep(4000);
			continue;
		}

		if (context->usbcam_state == FALSE) {
			USBCAM_ERROR_PRINT("usbcam not ready when read\n");
			usleep(4000);
			continue;
		}
		/*
			Bug#28844: If the write speed is much faster than the read speed,
			the usbcam will crash,so we must control the write speed
		*/
		if((write_total - push_total) > MAX_FRAGMENT_SIZE) {
			USBCAM_INFO_PRINT("write too fast,sleep a moment\n");
			usleep(100000);
			continue;
		}

		ret = MT_UNF_DMX_AcquireRecData(record_info->rec_handle, &rec_data, 100);
		if (MT_SUCCESS != ret) {
			USBCAM_GET_REC_PRINT("MT_UNF_DMX_AcquireRecData eror\n");
			usleep(4000);
			continue;
		}

		ret = usbcam_route_add_fragment_header(context->usbcam_dev);
		if (ret < 0) {
			USBCAM_WARN_PRINT("%s call usbcam_route_add_fragment_header failed\n", __func__);
		}


		USBCAM_GET_REC_PRINT("write %d rec data to usbcam dev\n", rec_data.u32Len);
		usbcam_route_write_data(context->usbcam_dev, rec_data.pDataAddr, rec_data.u32Len);
		write_total += rec_data.u32Len;

		ret = MT_UNF_DMX_ReleaseRecData(record_info->rec_handle, &rec_data);
		if (MT_SUCCESS != ret) {
			USBCAM_ERROR_PRINT("[%s] MT_UNF_DMX_ReleaseRecData failed 0x%x\n", __FUNCTION__, ret);
			break;
		}
		memset( &rec_data, 0, sizeof(MT_UNF_DMX_REC_DATA_S));
		usleep(10000);

	}

	context->get_data_task.run = 0;
	context->get_data_task.exit = 0;
	USBCAM_GET_REC_PRINT("%s end\n", __func__);
	return NULL;
}

/*
create tsbuff(ram) handle
and create a write thread:usbcam_route_write_task
涉及变量
1、context
*/
static int usbcam_route_init_write_thread(usbcam_play_context_s *context)
{
	int ret;

	context->push_data_task.run = 0;
	context->push_data_task.exit = 0;
	ret = pthread_create( &(context->push_data_task.id), NULL, (mt_void *)usbcam_route_write_task, (void *)context);
	if (0 != ret) {
		USBCAM_ERROR_PRINT("[DmxStartRecord] pthread_create PushTsDataTthread error\n");
		return ret;
	}

	return 0;
}

static int usbcam_route_start_record(record_info_t *record_info )
{
	int ret = 0;
	MT_UNF_DMX_REC_ATTR_S RecAttr;
	mt_handle rec_handle;
	unsigned int select_prog_flag =  record_info->all_prog_record;

	memset( &RecAttr, 0, sizeof(MT_UNF_DMX_REC_ATTR_S));
	RecAttr.u32DmxId = record_info->source;
	RecAttr.u32RecBufSize = record_info->recbuf_size;//record_info->recbuf_size
	if (0 == select_prog_flag) {

	} else {
		//all program record
		RecAttr.enRecType = MT_UNF_DMX_REC_TYPE_ALL_PID;
		RecAttr.bDescramed = MT_TRUE;
		RecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_NONE;
		RecAttr.type_mode = DMX_FULL_TS_WITHOUT_NULL_PACKET;
	}

	ret = MT_UNF_DMX_CreateRecChn( &RecAttr, &rec_handle);
	if (MT_SUCCESS != ret) {
		USBCAM_ERROR_PRINT("[%s - %u]create record channel 0x%x\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	ret = MT_UNF_DMX_StartRecChn(rec_handle);
	if (MT_SUCCESS != ret) {
		USBCAM_ERROR_PRINT("[%s - %u] MT_UNF_DMX_StartRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
		MT_UNF_DMX_DestroyRecChn(rec_handle);
		return ret;
	}

	record_info->rec_handle = rec_handle;
	return ret;
}

static int usbcam_route_stop_record(usbcam_play_context_s *context)
{
	int ret = 0;
	record_info_t *record_info = NULL;
	if (NULL == context) {
		USBCAM_ERROR_PRINT("%s context is NULL\n", __func__);
		return MT_ERR_USBCAM_ROUTE_MEMORY;
	}

	record_info = &(context->record_info);
	ret = MT_UNF_DMX_StopRecChn(record_info->rec_handle);
	if (MT_SUCCESS != ret) {
		USBCAM_ERROR_PRINT("[%s - %u] MT_UNF_DMX_StopRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	ret = MT_UNF_DMX_DestroyRecChn(record_info->rec_handle);
	if (MT_SUCCESS != ret) {
		USBCAM_ERROR_PRINT("[%s - %u] MT_UNF_DMX_DestroyRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	return ret;
}

/*
and create a thread:usbcam_route_read_task
*/
static int usbcam_route_init_read_thread(usbcam_play_context_s *context)
{
	int ret = 0;
	context->get_data_task.run = 0;
	context->get_data_task.exit = 0;

	ret = pthread_create( &(context->get_data_task.id), NULL, usbcam_route_read_task, (mt_void *)context);
	if (0 != ret) {
		USBCAM_ERROR_PRINT("[DmxStartRecord] pthread_create record error\n");
	}

	return ret;
}

/*
create usbcam_route_read_task and usbcam_route_write_task
*/
static s32 usbcam_route_init_task(usbcam_play_context_s *context)
{
	int ret = 0;
	printf("create the read thread\n");
	ret = usbcam_route_init_read_thread(context);
	if (ret < 0) {
		USBCAM_ERROR_PRINT("%s call usbcam_route_init_read_thread failed\n", __func__);
		return ret;
	}

	printf("create the write thread\n");
	ret = usbcam_route_init_write_thread(context);
	if (ret < 0) {
		USBCAM_ERROR_PRINT("%s call usbcam_route_init_read_thread failed\n", __func__);
		return ret;
	}
	return 0;
}

static s32 usbcam_route_uninit_task(usbcam_play_context_s *usbcam_play_context)
{
	if (NULL == usbcam_play_context) {
		USBCAM_ERROR_PRINT("%s context is not init\n", __func__);
		return MT_ERR_USBCAM_ROUTE_MEMORY;
	}

	printf("ready to exit thread\n");
	usbcam_play_context->get_data_task.exit = 1;
	pthread_join(usbcam_play_context->get_data_task.id, NULL);

	usbcam_play_context->push_data_task.exit = 1;
	pthread_join(usbcam_play_context->push_data_task.id, NULL);
	printf("the read and write task exit\n");

	return SUCCESS;
}

static int usbcam_route_attach_cfg(void)
{
	int ret = 0;
	//(mt_void) mt_sys_init();
	//MT_UNF_DMX_Init();

	USBCAM_INFO_PRINT("attach dmx0 and tsi1\n");
	ret = MT_UNF_DMX_AttachTSPort(g_param.rec_dmxid, g_param.tuner_port);
	if (MT_SUCCESS != ret) {
		USBCAM_ERROR_PRINT("call MT_UNF_DMX_AttachTSPort failed.\n");
		goto EXIT0;
	}

	USBCAM_INFO_PRINT("atach dmx1 and ram0\n");
	ret = MT_UNF_DMX_AttachTSPort(g_param.ci_dmxid, g_param.tsbuf_port);
	if (MT_SUCCESS != ret) {
		USBCAM_ERROR_PRINT("call MT_UNF_DMX_AttachTSPort failed.\n");
		goto EXIT1;
	}

	return ret;

EXIT1:
	(mt_void) MT_UNF_DMX_DetachTSPort(g_param.rec_dmxid);
EXIT0:
	(mt_void) MT_UNF_DMX_DeInit();
	mt_sys_deinit();

	return ret;
}

static void usbcam_route_deattach_cfg(void)
{
	(mt_void) MT_UNF_DMX_DetachTSPort(g_param.ci_dmxid);
	(mt_void) MT_UNF_DMX_DetachTSPort(g_param.rec_dmxid);
	//(mt_void) MT_UNF_DMX_DeInit();
	//mt_sys_deinit();
	return;
}

s32 usbcam_route_ts_uninit(void)
{
	s32 ret = 0;
	usbcam_play_context_s *usbcam_play_context = NULL;
	usbcam_play_context = get_usbcam_context();
	if (NULL == usbcam_play_context) {
		USBCAM_ERROR_PRINT("%s context is not init\n", __func__);
		return MT_ERR_USBCAM_ROUTE_MEMORY;
	}

	ret = usbcam_route_uninit_task(usbcam_play_context);
	if (ret < 0) {
		USBCAM_ERROR_PRINT("%s call usbcam_route_uninit_task failed\n", __func__);
		return MT_ERR_USBCAM_ROUTE_TASK_ERR;
	}

	close(usbcam_play_context->usbcam_dev);

	free(usbcam_play_context);

	usbcam_route_deattach_cfg();
	return SUCCESS;
}

s32 usbcam_route_ts_init()
{
	int ret = 0;
	usbcam_play_context_s *context = NULL;

	USBCAM_INFO_PRINT("%s start\n", __func__);
	ret = usbcam_route_attach_cfg();
	if (ret < 0) {
		USBCAM_ERROR_PRINT("%s call usbcam_route_attach_cfg failed\n", __func__);
		return MT_ERR_USBCAM_ROUTE_ATTACH_ERR;
	}

	context = malloc(sizeof(usbcam_play_context_s));
	if (context == NULL) {
		USBCAM_ERROR_PRINT("malloc space for usbcam_play_context_s var failed\n");
		ret = MT_ERR_USBCAM_ROUTE_MEMORY;
		goto EXIT0;
	}
	memset(context, 0, sizeof(usbcam_play_context_s));

	context->usbcam_dev = open(DEV_FILE_MEDIA, O_RDWR | O_NONBLOCK);
	if (context->usbcam_dev < 0) {
		USBCAM_ERROR_PRINT("open usbcam failed\n");
		ret = MT_ERR_USBCAM_ROUTE_OPEN_DEV_ERR;
		goto EXIT1;
	}

	ret = usbcam_route_init_task(context);
	if (ret != SUCCESS) {
		USBCAM_ERROR_PRINT("%s usbcam_route_init_task failed\n", __func__);
		ret = MT_ERR_USBCAM_ROUTE_TASK_ERR;
		goto EXIT2;
	}

	save_usbcam_context(context);
	USBCAM_INFO_PRINT("%s end\n", __func__);
	return SUCCESS;

EXIT2:
	close(context->usbcam_dev);
EXIT1:
	free(context);
EXIT0:
	usbcam_route_deattach_cfg();

	return ret;
}

void usbcam_route_set_record_info()
{
	usbcam_play_context_s *usbcam_context = get_usbcam_context();
	usbcam_context->record_info.all_prog_record = g_param.all_prog_record;
	usbcam_context->record_info.source = g_param.rec_dmxid;
	usbcam_context->record_info.recbuf_size = g_param.recbuf_size;

#if 0
	usbcam_context->record_info.link_mode = 0;
	usbcam_context->record_info.apid = cur_prog_info.AElementPid;
	usbcam_context->record_info.vpid = cur_prog_info.VElementPid;
	usbcam_context->record_info.atype = cur_prog_info.AudioType;
	usbcam_context->record_info.vtype = cur_prog_info.VideoType;
	usbcam_context->record_info.pcrpid = cur_prog_info.PcrPid;
	usbcam_context->record_info.pmtpid = cur_prog_info.PmtPid;
	printf("apid = 0x%x,vpid = 0x%x,pmtpid = 0x%x\n", usbcam_context->record_info.apid,
	       usbcam_context->record_info.vpid, usbcam_context->record_info.pmtpid);
#endif
}

void usbcam_route_start_fragment(u32 fragment)
{
	start_fragment = fragment;
}

s32 usbcam_route_param_init(usbcam_init_parm_t *param)
{
	if (NULL == param) {
		USBCAM_ERROR_PRINT("%s usbcam_parm_t is null\n", __func__);
		return MT_ERR_USBCAM_ROUTE_MEMORY;
	}

	memcpy( &g_param, param, sizeof(usbcam_init_parm_t));
	return 0;
}

/*
start read and write thread
start record
called after usbcam_route_init
*/
s32 usbcam_route_ts_start()
{
	int ret;
	usbcam_play_context_s *usbcam_play_context = NULL;

	usbcam_play_context = get_usbcam_context();
	if (NULL == usbcam_play_context) {
		USBCAM_ERROR_PRINT("%s context is not init\n", __func__);
		return MT_ERR_USBCAM_ROUTE_MEMORY;
	}

	USBCAM_INFO_PRINT("create record handle\n");
	ret = usbcam_route_start_record( &(usbcam_play_context->record_info));
	if (ret < 0) {
		USBCAM_ERROR_PRINT("%s call usbcam_route_start_record\n", __func__);
		return ret;
	}

	USBCAM_INFO_PRINT("create hanle for TS buffer\n");
	ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0xF00000, &(usbcam_play_context->ram_handle));
	if (ret != MT_SUCCESS) {
		USBCAM_ERROR_PRINT("call MT_UNF_DMX_CreateTSBuffer failed.\n");
		usbcam_route_stop_record(usbcam_play_context);
		return ret;
	}
	write_total = 0;
	push_total = 0;

	usbcam_play_context->push_data_task.run = 1;
	usbcam_play_context->get_data_task.run = 1;
	usbcam_route_ts_set_usbcam_state(TRUE);
	USBCAM_INFO_PRINT("start read and write thread\n");

	return SUCCESS;
}

s32 usbcam_route_ts_stop(void)
{
	int ret = SUCCESS;
	usbcam_play_context_s *usbcam_play_context = NULL;

	usbcam_play_context = get_usbcam_context();
	if (NULL == usbcam_play_context) {
		USBCAM_ERROR_PRINT("%s context is not init\n", __func__);
		return MT_ERR_USBCAM_ROUTE_MEMORY;
	}

	usbcam_route_ts_set_usbcam_state(FALSE);

	usbcam_play_context->push_data_task.run = 0;
	usbcam_play_context->get_data_task.run = 0;

	ret = MT_UNF_DMX_DestroyTSBuffer(usbcam_play_context->ram_handle);
	if (SUCCESS != ret) {
		USBCAM_ERROR_PRINT("%s call MT_UNF_DMX_DestroyTSBuffer failed\n", __func__);
		return ret;
	}

	ret = usbcam_route_stop_record(usbcam_play_context);
	if (SUCCESS != ret) {
		USBCAM_ERROR_PRINT("%s call usbcam_route_stop_record failed\n", __func__);
		return ret;
	}
	write_total = 0;
	push_total = 0;
	return ret;
}

s32 usbcam_route_ts_set_usbcam_state(BOOL state)
{
	int ret = SUCCESS;
	usbcam_play_context_s *usbcam_play_context = NULL;

	usbcam_play_context = get_usbcam_context();
	if (NULL == usbcam_play_context) {
		USBCAM_ERROR_PRINT("%s context is not init\n", __func__);
		return MT_ERR_USBCAM_ROUTE_MEMORY;
	}

	usbcam_play_context->usbcam_state = state;

	return ret;
}
