/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __SND_API_H__
#define __SND_API_H__

#include "mt_module.h"
#include "mt_unf_sound.h"
//#include "mt_drv_mmz.h"
#define SND_CACHE_ALIGN __attribute__ ((aligned (64)))

#define DRV_SND_DEV_NAME 	UMAP_DEVNAME_SND

#define AUD_SND_HW_BUFF_NUM		4
#define AUD_SND_SW_BUFF_NUM		3
#define AUD_SND_DATA_PKT_NUM	1024	

//drv snd ioctl CMD
#define SND_MAGIC					MT_ID_SND
#define SND_INIT 					_IOW(SND_MAGIC,	1, AUD_SND_INIT_PARAM)
#define SND_UNINIT 					_IO(SND_MAGIC,	2)

#define SND_START					_IO(SND_MAGIC,	3)
#define SND_STOP					_IO(SND_MAGIC,	4)
#define SND_PAUSE					_IO(SND_MAGIC,	5)
#define SND_RESUME					_IO(SND_MAGIC,	6)
#define SND_FLUSH					_IO(SND_MAGIC,	7)
#define SND_SEND_DATA				_IOW(SND_MAGIC,	8, ulong)
#define SND_GET_VOL					_IOR(SND_MAGIC,	9, ulong)
#define SND_SET_VOL					_IOW(SND_MAGIC,	10, ulong)
#define SND_GET_MUTE				_IOR(SND_MAGIC,	11, ulong)
#define SND_SET_MUTE				_IOW(SND_MAGIC,	12, ulong)
#define SND_RESET					_IO(SND_MAGIC,	13)
#define SND_ADAC_ONOFF				_IOW(SND_MAGIC,	14, ulong)
#define SND_SET_HDMI_MODE			_IOW(SND_MAGIC,	15, ulong)
#define SND_GET_HDMI_MODE			_IOR(SND_MAGIC,	16, ulong)
#define SND_SET_SPDIF_MODE			_IOW(SND_MAGIC,	17, ulong)
#define SND_GET_SPDIF_MODE			_IOR(SND_MAGIC,	18, ulong)
#define SND_SET_CRC_DBG_BUF_SIZE	_IOW(SND_MAGIC,	19, ulong)
#define SND_GET_ATTR				_IOR(SND_MAGIC,	20, ulong)
#define SND_SET_TRACK_MODE			_IOW(SND_MAGIC,	21, ulong)
#define SND_GET_TRACK_MODE			_IOR(SND_MAGIC,	22, ulong)
#define SND_GET_CITEST_INFO			_IOR(SND_MAGIC,	23, ulong)
#define SND_SET_EOS_FLAG			_IOW(SND_MAGIC,	24, ulong)
#define SND_SET_FADER				_IOW(SND_MAGIC,	25, ulong)
#define SND_GET_BUF_INFO			_IOW(SND_MAGIC,	26, ulong)
#define SND_START_TTS				_IO(SND_MAGIC, 27)
#define SND_STOP_TTS				_IO(SND_MAGIC, 28)

#define SND_CFG_HDMI				_IOW(SND_MAGIC,	100, ulong)		// only for fpga test

typedef enum SND_CARD_TYPE_e
{
	SND_CARD_TYPE_NULL = 0,
	SND_CARD_TYPE_MASTER,
	SND_CARD_TYPE_SLAVE,
	SND_CARD_TYPE_VIRTUAL,
	
	SND_CARD_TYPE_INVALID
}SND_CARD_TYPE;

typedef enum AUD_SND_HW_BUFF_IDX_e
{
	AUD_SND_HW_BUFF_IDX_0 = 0,	//pp buffer
	AUD_SND_HW_BUFF_IDX_1,		//aout buffer
	AUD_SND_HW_BUFF_IDX_2,		//mix buffer
	AUD_SND_HW_BUFF_IDX_3,		//spd buffer
	
	AUD_SND_HW_BUFF_IDX_INVALID
}AUD_SND_HW_BUFF_IDX;
	
typedef enum AUD_SND_SW_BUFF_IDX_e
{
	AUD_SND_SW_BUFF_IDX_0 = 0,
	AUD_SND_SW_BUFF_IDX_1,
	AUD_SND_SW_BUFF_IDX_2,
	
	AUD_SND_SW_BUFF_IDX_INVALID
}AUD_SND_SW_BUFF_IDX;
	
typedef enum AUD_SND_DATA_FMT_e
{
	AUD_SND_DATA_FMT_NULL = 0,
	AUD_SND_DATA_FMT_PCM,
	AUD_SND_DATA_FMT_AC3,
	AUD_SND_DATA_FMT_EAC3,
	AUD_SND_DATA_FMT_DTS,
	AUD_SND_DATA_FMT_DTS_HD,
	AUD_SND_DATA_FMT_MAT,
	
	AUD_SND_DATA_FMT_INVALID
}AUD_SND_DATA_FMT;

typedef struct snd_mute_parame_s
{
	SND_CARD_TYPE type;
	MT_UNF_SND_OUTPUTPORT_E port;
	u8 mute;
} snd_mute_parame_t;

typedef struct AUD_SND_INIT_PARAM_t
{
	SND_CARD_TYPE type;
}AUD_SND_INIT_PARAM;

typedef struct AUD_SND_BUFF_ATTR_t
{
	AUD_SND_DATA_FMT fmt;
	u64 addr;
	u32 len;
	u32 crc;
}AUD_SND_BUFF_ATTR;

typedef struct AUD_SND_DATA_PKT_t
{
	u32 frmIdx;
	u32 apts;
	u32 sampleRate;
	u32 frameSample;
	u8 channel;
	u8 bitDepth;
	u8 interleaved;
	u8 validBit;
	
	u32 typeOriginal;
	u32 channelOriginal;
	
	u8 dualmono;
	u8 bsid;
	u8 acmod;
	u8 lfeon;
	u8 dolby_type;
	
	AUD_SND_BUFF_ATTR stData[AUD_SND_SW_BUFF_NUM];
} AUD_SND_DATA_PKT;

typedef struct AUD_SND_DATA_PKT_QUE_t
{
	u32 num;
	u32 rd;
	u32 wt;
	u32 wt_cnt;
	AUD_SND_DATA_PKT stQue[AUD_SND_DATA_PKT_NUM];
}SND_CACHE_ALIGN AUD_SND_DATA_PKT_QUE;

typedef struct SND_BUFF_t
{
	u8 used;
	u8 swBuffIdx;
	
	u32 size;
	//ulong base;
	u64 base_phy;
	u64 base_kern_vir;
	u64 base_ta_vir;
	u64 base_usr_vir;
	u32 rd;
	u32 wt;
	u32 wtCnt;
	u32 wtByte;
	
	u32 regWtCmd;
	u32 regBuffCnt;
	//mmz_buffer_s buff;
}SND_CACHE_ALIGN SND_BUFF;

typedef struct aud_param_s
{
	int sample_rate;
	int ch_num;
	int bitdepth;
	AUD_SND_DATA_FMT fmt;
	
} SND_CACHE_ALIGN aud_param_t;

#endif

