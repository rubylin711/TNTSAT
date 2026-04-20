/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : mtlz_dmx_source.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/14
 * Description    : Demux Source Component for Monage-LZ SW Player.
 * History        :
 * 1.Date         : 2019/03/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef LINUX
#include "mt_type.h"
#include "mt_mpi_demux.h"
#include "mt_unf_demux.h"

#elif defined(__UC_OS__)
#include "mt_type.h"
#include "drv_dev.h"
#include "dmx.h"
#endif

#include "mtlz_types.h"
#include "mtlzplayer_comp.h"

#define MAX_DMX_NUM						4
#define VALID_DMX_ID(id)				((id)>=0 && (id)<MAX_DMX_NUM)

#define MAX_DMX_CHANNEL_FILE			1
#define DMX_VES_CH_BUFFER_SIZE			0x300000

/* dmx channel buffer */
struct buffer_st
{
	unsigned int addr;
	unsigned int size;
	unsigned int read;
	unsigned int write;
};

/* dmx channel file */
struct dmx_channel_file_st
{
	char name[256];
	uint64_t size;
	uint64_t offset;

	mt_u32 u32DmxId;
	mt_u32 u32Pid;
	mt_handle hChannel;

	struct buffer_st stBuf;
};
static struct dmx_channel_file_st dmx_channel_files[MAX_DMX_CHANNEL_FILE];

#define CHECK_FD(fd) 			MTLZ_ASSERT(fd != 0 && fd != MTLZ_INVALID_FD)
#define GET_FILE(fd) 			&dmx_channel_files[fd]

//path:
//    "demuxer=dmx,0"
//    "demuxer=demux,0"
static mt_u32 get_dmx_id(const char *path)
{
	char *source;
	char *str;
	mt_u32 u32DmxId = (mt_u32)MTLZ_INVALID_ID;

	source = strstr(path, "demuxer=");
	if (source != NULL)
	{
		str = strstr(source, "dmx");
		if (str != NULL)
		{
			sscanf(str, "dmx,%d", &u32DmxId);
			return u32DmxId;
		}

		str = strstr(source, "demux");
		if (str != NULL)
		{
			sscanf(str, "demux,%d", &u32DmxId);
			return u32DmxId;
		}
	}

	return u32DmxId;
}

//path:
//    "video=201,mpeg2"
static mt_u32 get_ch_pid(const char *path)
{
	char *str;
	mt_u32 u32Pid = MTLZ_INVALID_PID;

	str = strstr(path, "video=");
	if (str != NULL)
	{
		sscanf(str, "video=%d", &u32Pid);
		return u32Pid;
	}

	return u32Pid;
}

#ifdef __UC_OS__
//url:
//    "video=201,mpeg2"
static dmx_video_format_t get_vformat(const char *url)
{
	char *str;
	char *szCodec = "mpeg2";

	str = strstr(url, "video=");
	if (str != NULL)
	{
		str = strchr(str, ',');
		if (str != NULL)
			szCodec = str + 1;
	}
	else
	{
		/* file extention, such as "*.m2v" */
		str = strrchr(url, '.');
		if (str != NULL)
			szCodec = str + 1;
	}

	if (strncmp(szCodec, "mpeg2", 5) == 0
		|| strncmp(szCodec, "mpeg1", 5) == 0
		|| strncmp(szCodec, "mpeg", 4) == 0
		|| strncmp(szCodec, "mpg", 3) == 0
		|| strncmp(szCodec, "m2v", 3) == 0)
	{
		return DMX_VIDEO_MPEG;
	}
	else if (strncmp(szCodec, "mpeg4", 5) == 0
			 || strncmp(szCodec, "mp4", 3) == 0
			 || strncmp(szCodec, "m4v", 3) == 0)
	{
		return DMX_VIDEO_MPEG4;
	}
	else if (strncmp(szCodec, "avs", 3) == 0)
	{
		return DMX_VIDEO_AVS;
	}
	else if (strncmp(szCodec, "h263", 4) == 0)
	{
		return DMX_VIDEO_H263;
	}
	else if (strncmp(szCodec, "h264", 4) == 0 || strncmp(szCodec, "avc", 3) == 0)
	{
		return DMX_VIDEO_H264;
	}
	else if (strncmp(szCodec, "h265", 4) == 0 || strncmp(szCodec, "hevc", 4) == 0)
	{
		return DMX_VIDEO_H265;
	}
	else if (strncmp(szCodec, "vc1", 3) == 0)
	{
		return DMX_VIDEO_VC1;
	}
	else if (strncmp(szCodec, "vp8", 3) == 0)
	{
		return DMX_VIDEO_VP8;
	}
	else
	{
		MTLZ_ERROR("[ERROR]%s: unknown codec(%s)!\n",__FUNCTION__,url);
	}

	return DMX_VIDEO_UNKNOWN;
}
#endif

/**
 * @brief acquire valid data address and size
 *
 * @param[out] data/size valid data address and size
 *
 * @return valid data size
 */
static unsigned int _buffer_acquire(struct buffer_st *pBuf, unsigned int *data, unsigned int *size)
{
	if (pBuf->write > pBuf->size)
	{
		MTLZ_BUG();
		pBuf->write = pBuf->size;
	}

AGAIN:
	if (pBuf->read < pBuf->write)
	{
		*data = pBuf->addr + pBuf->read;
		*size = pBuf->write - pBuf->read;
		MTLZ_VERBOSE("%s: data 0x%x, size %u\n",__FUNCTION__,*data,*size);
	}
	else if (pBuf->read > pBuf->write)
	{
		//rewind
		if (pBuf->read >= pBuf->size)
		{
			pBuf->read %= pBuf->size;
			MTLZ_VERBOSE("%s: rewind.\n",__FUNCTION__);
			goto AGAIN;
		}

		//only return RD->Tail part
		*data = pBuf->addr + pBuf->read;
		*size = pBuf->size - pBuf->read;
		MTLZ_VERBOSE("%s: data 0x%x, size %u\n",__FUNCTION__,*data,*size);
	}
	else
	{
		*data = 0;
		*size = 0;
	}

	return (*size);
}

//update consumed(read) pointer
static unsigned int _buffer_consumed(struct buffer_st *pBuf, unsigned int size)
{
	pBuf->read += size;
	MTLZ_VERBOSE("%s: size %u\n",__FUNCTION__,size);

	if (pBuf->read > pBuf->size)
	{
		MTLZ_BUG();
		pBuf->read = pBuf->size;
	}

	return pBuf->read;
}

#ifdef LINUX
/* dmx channel open */
static int _dmx_channel_open(mt_u32 u32DmxId, mt_u32 u32Pid, mt_handle *phChannel)
{
	MT_UNF_DMX_CHAN_ATTR_S chAttr;
    mt_u8 esBuffId1 = 0xfe;
	int ret;

	MTLZ_DEBUG("%s: dmx id %u\n",__FUNCTION__,u32DmxId);
	MTLZ_DEBUG("%s: pid %u\n",__FUNCTION__,u32Pid);

	memset(&chAttr, 0, sizeof(MT_UNF_DMX_CHAN_ATTR_S));
	chAttr.u32BufSize = DMX_VES_CH_BUFFER_SIZE;
	chAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_VID;
	chAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD;
	chAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
	chAttr.AVSyncFlag = MT_FALSE;
	chAttr.vCodecType = MT_UNF_VCODEC_TYPE_MPEG2;
	chAttr.esBuffId1 = &esBuffId1;
	MTLZ_DEBUG("%s: buffer size %u\n",__FUNCTION__,chAttr.u32BufSize);

	ret = MT_UNF_DMX_CreateChannel(u32DmxId, &chAttr, phChannel);
	if (ret != MT_SUCCESS)
	{
		MTLZ_ERROR("[ERROR]%s: create channel failed!\n",__FUNCTION__);
		return MTLZ_FAILURE;
	}

	ret = MT_UNF_DMX_SetChannelPID(*phChannel, u32Pid);

	ret |= MT_UNF_DMX_OpenChannel(*phChannel);
	if (ret != MT_SUCCESS)
	{
		MTLZ_ERROR("[ERROR]%s: open channel failed!\n",__FUNCTION__);
		MT_UNF_DMX_DestroyChannel(*phChannel);
		return MTLZ_FAILURE;
	}
	else
	{
		MTLZ_DEBUG("%s: open channel(0x%x) success.\n",__FUNCTION__,*phChannel);
	}

	return ret;
}

static int _dmx_channel_close(mt_handle hChannel)
{
	int ret;

	ret = MT_UNF_DMX_CloseChannel(hChannel);
	ret |= MT_UNF_DMX_DestroyChannel(hChannel);
	if (ret != MT_SUCCESS)
	{
		MTLZ_ERROR("[ERROR]%s: close or destroy channel failed!\n",__FUNCTION__);
		return MTLZ_FAILURE;
	}
	else
	{
		MTLZ_DEBUG("%s: close channel(0x%x) success.\n",__FUNCTION__,hChannel);
	}

	return ret;
}

//read dmx channel registers
static void _dmx_channel_status(mt_handle hChannel, struct buffer_st *pBuf)
{
	MT_MPI_DMX_GetEsBuffAddr(hChannel, (ulong *)&pBuf->addr, (ulong *)&pBuf->size, NULL, NULL, NULL);
	pBuf->read = MT_MPI_DMX_GetRegristerValue(hChannel, 0x012c);
	//refresh write pointer in fact!
	pBuf->write = MT_MPI_DMX_GetCurrentEsBufferWritePoint(hChannel);
	MTLZ_VERBOSE("%s: addr 0x%x size 0x%x rd 0x%x wr 0x%x\n",__FUNCTION__,
		pBuf->addr,pBuf->size,pBuf->read,pBuf->write);
}

//write(update) dmx channel registers
static void _dmx_channel_update(mt_handle hChannel, struct buffer_st *pBuf)
{
	//update read pointer in fact!
	MT_MPI_DMX_SetCurrentEsBufferReadPoint(hChannel, pBuf->read);
	MTLZ_VERBOSE("%s: rd 0x%x\n",__FUNCTION__,
		pBuf->read);
}

#elif defined(__UC_OS__)
static int _dmx_channel_open(mt_u32 u32DmxId, mt_u32 u32Pid, mt_handle *phChannel,
							dmx_video_format_t vfmt)
{
	dmx_device_t *p_dev = NULL;
	dmx_play_setting_t play_para;
	dmx_chanid_t channel;
	RET_CODE ret;

	MTLZ_DEBUG("%s: dmx id %u\n",__FUNCTION__,u32DmxId);
	MTLZ_DEBUG("%s: pid %u\n",__FUNCTION__,u32Pid);

	p_dev = (dmx_device_t *)dev_find_identifier(NULL, DEV_IDT_TYPE, SYS_DEV_TYPE_PTI);
	if (NULL == p_dev->p_base)
	{
		MTLZ_ERROR("[ERROR]%s: dev_find_identifier failed!\n",__FUNCTION__);
		return MTLZ_FAILURE;
	}

	memset(&play_para, 0, sizeof(dmx_play_setting_t));
	play_para.pid 			= u32Pid;
	play_para.type 			= DMX_VIDEO_TYPE;
	play_para.stream_in 	= DMX_INPUT_EXTERN0 + u32DmxId;
	play_para.pes_info 		= NULL;
	play_para.vid_format 	= vfmt;

	ret = dmx_play_chan_open((void*)p_dev, &play_para, &channel);
	if (ret != SUCCESS)
	{
		MTLZ_ERROR("[ERROR]%s: dmx_play_chan_open failed!\n",__FUNCTION__);
		return MTLZ_FAILURE;
	}

	//disable PTS insert
	ret = dmx_set_vpts_cfg((void*)p_dev, FALSE, NULL);
	if (ret != SUCCESS)
	{
		MTLZ_ERROR("[ERROR]%s: dmx_set_vpts_cfg failed!\n",__FUNCTION__);
		dmx_chan_close((void*)p_dev, channel);
		return MTLZ_FAILURE;
	}

	ret = dmx_chan_start((void*)p_dev, channel);
	if (ret != SUCCESS)
{
		MTLZ_ERROR("[ERROR]%s: dmx_chan_start failed!\n",__FUNCTION__);
		dmx_chan_close((void*)p_dev, channel);
	return MTLZ_FAILURE;
}

	*phChannel = (mt_handle)channel;
	MTLZ_DEBUG("%s: open channel(0x%x) success.\n",__FUNCTION__,channel);
	return MTLZ_SUCCESS;
}

static int _dmx_channel_close(mt_handle hChannel)
{
	dmx_device_t *p_dev = NULL;
	RET_CODE ret;

	MTLZ_DEBUG("%s: handle 0x%x\n",__FUNCTION__,hChannel);

	p_dev = (dmx_device_t *)dev_find_identifier(NULL, DEV_IDT_TYPE, SYS_DEV_TYPE_PTI);
	if (NULL == p_dev->p_base)
	{
		MTLZ_ERROR("[ERROR]%s: dev_find_identifier failed!\n",__FUNCTION__);
		return MTLZ_FAILURE;
	}

	ret = dmx_chan_stop((void*)p_dev, (dmx_chanid_t)hChannel);

	ret |= dmx_chan_close((void*)p_dev, (dmx_chanid_t)hChannel);
	if (ret != SUCCESS)
	{
		MTLZ_ERROR("[ERROR]%s: dmx_chan_stop or dmx_chan_close failed!\n",__FUNCTION__);
		return MTLZ_FAILURE;
	}

	return MTLZ_SUCCESS;
}

static void _dmx_channel_status(mt_handle hChannel, struct buffer_st *pBuf)
{
	dmx_device_t *p_dev = NULL;
	RET_CODE ret;

	p_dev = (dmx_device_t *)dev_find_identifier(NULL, DEV_IDT_TYPE, SYS_DEV_TYPE_PTI);
	if (NULL == p_dev->p_base)
	{
		MTLZ_ERROR("[ERROR]%s: dev_find_identifier failed!\n",__FUNCTION__);
		return;
	}

	ret = dmx_symphony_get_esBuffAddr((void*)p_dev, (dmx_chanid_t)hChannel,
									&pBuf->addr, &pBuf->size);
	ret |= dmx_symphony_get_esBuffWritePtr((void*)p_dev, (dmx_chanid_t)hChannel,
									&pBuf->write);
	ret |= dmx_symphony_get_esBuffReadPtr((void*)p_dev, (dmx_chanid_t)hChannel,
									&pBuf->read);

	MTLZ_VERBOSE("%s: addr 0x%x size 0x%x rd 0x%x wr 0x%x\n",__FUNCTION__,
		pBuf->addr,pBuf->size,pBuf->read,pBuf->write);
}

static void _dmx_channel_update(mt_handle hChannel, struct buffer_st *pBuf)
{
	dmx_device_t *p_dev = NULL;
	RET_CODE ret;

	p_dev = (dmx_device_t *)dev_find_identifier(NULL, DEV_IDT_TYPE, SYS_DEV_TYPE_PTI);
	if (NULL == p_dev->p_base)
	{
		MTLZ_ERROR("[ERROR]%s: dev_find_identifier failed!\n",__FUNCTION__);
		return;
	}

	ret = dmx_symphony_set_esBuffReadPtr((void*)p_dev, (dmx_chanid_t)hChannel,
										pBuf->read);

	MTLZ_VERBOSE("%s: rd 0x%x\n",__FUNCTION__,pBuf->read);
}
#endif

static struct dmx_channel_file_st *_dmx_channel_file_open(const char *path, int oflag)
{
	//FIXME: how about multi files?
	int fd = 0;
	int ret;
	struct dmx_channel_file_st *file;

	file = GET_FILE(fd);

	MTLZ_VERBOSE("%s: open %s\n",__FUNCTION__,path);

	memset(file, 0, sizeof(struct dmx_channel_file_st));
	strncpy(file->name, path, 255);
	file->size = UINT64_MAX;
	file->offset = 0;

	file->u32DmxId = get_dmx_id(path);
	if (file->u32DmxId == MTLZ_INVALID_ID
		|| !VALID_DMX_ID(file->u32DmxId))
	{
		MTLZ_ERROR("[ERROR]%s: invalid dmx id(%s)!\n",__FUNCTION__,path);
		return NULL;
	}

	file->u32Pid = get_ch_pid(path);
	if (file->u32Pid == MTLZ_INVALID_PID)
	{
		MTLZ_ERROR("[ERROR]%s: invalid pid(%s)!\n",__FUNCTION__,path);
		return NULL;
	}

#ifdef __UC_OS__
	ret = _dmx_channel_open(file->u32DmxId, file->u32Pid, &file->hChannel,
							get_vformat(path));
#else
	ret = _dmx_channel_open(file->u32DmxId, file->u32Pid, &file->hChannel);
#endif
	if (ret != MTLZ_SUCCESS)
	{
		return NULL;
	}
	else
	{
		return file;
	}
}

static void _dmx_channel_file_close(struct dmx_channel_file_st *file)
{
	_dmx_channel_close(file->hChannel);
}

static ssize_t _dmx_channel_file_read(struct dmx_channel_file_st *file, void *buf, size_t count)
{
	size_t readSize = 0;
	ulong dataAddr;
	unsigned int copySize, dataSize;

	while (readSize < count)
	{
		if (_buffer_acquire(&file->stBuf, (unsigned int *)&dataAddr, &dataSize) <= 0)
			break;

		copySize = MIN(dataSize, count-readSize);

		//FIXME: memcpy is not efficient!
		memcpy((unsigned char*)buf + readSize, (void*)dataAddr, copySize);

		_buffer_consumed(&file->stBuf, copySize);

		readSize += copySize;
	}

	if (readSize != 0)
	{
		_dmx_channel_update(file->hChannel, &file->stBuf);

		file->offset += readSize;
		MTLZ_VERBOSE("%s: offset %llu.\n",__FUNCTION__,file->offset);
	}

	MTLZ_VERBOSE("%s: read %d bytes.\n",__FUNCTION__,readSize);

	return (ssize_t)readSize;
}

//refresh buffer write pointer
static void _dmx_channel_file_refresh(struct dmx_channel_file_st *file)
{
	_dmx_channel_status(file->hChannel, &file->stBuf);
}

/**
 * @brief Open Demux Video ES Channel
 *
 * @param[in] path demux channel path, such as "dmx=0;pid=201", or "demux=0;pid=201".
 * @param[in] oflag open flags, not used.
 *
 * @return demux channel file
 */
static ulong dmx_source_open(const char *path, int oflag)
{
	struct dmx_channel_file_st *file;

	if (path == NULL)
	{
		MTLZ_ERROR("[ERROR]%s: invalid parameters!\n",__FUNCTION__);
		return MTLZ_INVALID_FD;
	}

	file = _dmx_channel_file_open(path, oflag);
	if (file == NULL)
		return MTLZ_INVALID_FD;
	else
		return (ulong)file;
}

static int dmx_source_close(ulong fd)
{
	CHECK_FD(fd);

	_dmx_channel_file_close((struct dmx_channel_file_st *)fd);

	return MTLZ_SUCCESS;
}

static int dmx_source_eos(ulong fd)
{
	struct dmx_channel_file_st *file;

	CHECK_FD(fd);

	file = (struct dmx_channel_file_st *)fd;

	MTLZ_VERBOSE("%s: offset %llu, size %llu\n",__FUNCTION__,file->offset,file->size);

	return (file->size != UINT64_MAX && file->offset >= file->size);
}

//read dmx channel es buffer
//step 1: refresh write pointer
//step 2: read buffer
//step 3: update read pointer
static ssize_t dmx_source_read(ulong fd, void *buf, size_t count)
{
	CHECK_FD(fd);

	if (buf == NULL || count == 0)
	{
		MTLZ_ERROR("[ERROR]%s: invalid parameters!\n",__FUNCTION__);
		return 0;
	}

	_dmx_channel_file_refresh((struct dmx_channel_file_st *)fd);

	return _dmx_channel_file_read((struct dmx_channel_file_st *)fd, buf, count);
}

struct mtlzplayer_comp_source_st demux_source_comp =
{
	.open	= dmx_source_open,
	.close	= dmx_source_close,
	.eos	= dmx_source_eos,
	.read	= dmx_source_read,
};

/////////////////////////////////// For Test //////////////////////////////////
int test_dmx_source_open(const char *path, int oflag);
int test_dmx_source_close(int fd);
ssize_t test_dmx_source_read(int fd, void *buf, size_t count);

int test_dmx_source_open(const char *path, int oflag)
{
	return dmx_source_open(path, oflag);
}

int test_dmx_source_close(int fd)
{
	return dmx_source_close(fd);
}

ssize_t test_dmx_source_read(int fd, void *buf, size_t count)
{
	return dmx_source_read(fd, buf, count);
}

