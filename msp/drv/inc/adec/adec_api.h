/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __ADEC_API_H__
#define __ADEC_API_H__

#include "mt_module.h"
#include "snd/snd_api.h"

#define ADEC_DEVICE_NAME "adec"

#define ADEC_ES_HEADER_CNT	1024
#define ADEC_ES_BUF_SIZE	(384 * 1024)
#define ADEC_CACHE_ALIGN __attribute__ ((aligned (64)))

#define TA_ADEC_NONE					0
#define TA_ADEC_OPEN					1
#define TA_ADEC_CLOSE					2
#define TA_ADEC_START					3
#define TA_ADEC_STOP					4
#define TA_ADEC_AD_ENABLE				5
#define TA_ADEC_PAUSE					6
#define TA_ADEC_RESUME					7
#define TA_ADEC_ENABLE_HEAAC			8

#define IPC_MSG_AUD_FW_PVR_START  0xA0
#define IPC_MSG_AUD_FW_PVR_STOP  0xA1

// commond
#define IOC_MAGIC 					MT_ID_ADEC2
#define IO_ADEC_START				_IOW(IOC_MAGIC, 1, ulong)
#define IO_ADEC_STOP				_IOW(IOC_MAGIC, 2, ulong)
#define IO_ADEC_AD_ENABLE			_IOW(IOC_MAGIC, 3, ulong)
#define IO_ADEC_FLUSH				_IO(IOC_MAGIC, 4)
#define IO_ADEC_SET_DOWNMIX_ENABLE	_IOW(IOC_MAGIC, 5, ulong)
#define IO_ADEC_PAUSE				_IO(IOC_MAGIC, 6)
#define IO_ADEC_RESUME				_IO(IOC_MAGIC, 7)
#define IO_ADEC_GET_AUD_MEM			_IOW(IOC_MAGIC, 8, int)
#define IO_ADEC_ENABLE_HEAAC		_IO(IOC_MAGIC, 9)
#define IO_ADEC_TEE_ENABLE			_IO(IOC_MAGIC, 10)
#define IO_ADEC_TEE_DISABLE			_IO(IOC_MAGIC, 11)
#define IO_ADEC_TRICK_MODE			_IOW(IOC_MAGIC, 12, ulong)

/** language code length */
#define MT_AC4_DEC_LANG_MAX_LEN         	4

#define ADEC_TTS_BUF_SIZE           (100 * 1024)

typedef enum ADEC_STATUS
{
	ADEC_CLOSED		= 0,
	ADEC_OPENED		= 1,
	ADEC_ATTACHED	= 2,
	ADEC_INITED		= 3,
	ADEC_STARTED	= 4,
	ADEC_STOPED		= 5,
	ADEC_PAUSED		= 6,
} adec_status_e;

typedef enum ADEC_DECODE_STATE
{
	ADEC_READ			= 0,
	ADEC_PARSE			= 1,
	ADEC_DECODE			= 2,
	ADEC_POST_PROCESS	= 3,
	ADEC_OUTPUT			= 4,
} adec_decode_state_e;

typedef enum
{
	AC3 = 1,
	EAC3,
} dolby_type_e;

enum AUD_ES_DATA_TRACK_E
{
	MAIN_TRACK,
	AD_TRACK,
	TRACK_END,
};

typedef struct aud_track_data_s
{
	enum AUD_ES_DATA_TRACK_E type;
	u32 pts;
	u32 size;
	u32 valid;
	u8 *data;
} aud_track_data_t;

typedef struct adec_dump_s
{
	u32 enable;
	u32 rd;
	u32 wt;
	u32 size;
	char path[64];
	char data[64 * 1024];
} ADEC_CACHE_ALIGN adec_dump_t;

typedef struct adec_ree_to_tee_cmd_s
{
	int cmd;
	int exec;
	int ret;
	int param0;
	char param1[16 * 1024];
	int param1_size;
} ADEC_CACHE_ALIGN  adec_ree_to_tee_cmd_t;

typedef struct adec_es_pkt_s
{
	u32 pts;
	u32 valid;
	u32 es_rd;
	u32 size;
	u32 flag;		// 1:es buf right spce not enough, write from head
	u32 es_ts;  //  0 ES,  1 TS
} ADEC_CACHE_ALIGN adec_es_pkt_t;

typedef struct adec_es_buf_s
{
	u32 pkt_rd;
	u32 pkt_wt;
	u32 pkt_cnt;
	adec_es_pkt_t es_pkt[ADEC_ES_HEADER_CNT];
	
	u32 es_rd;
	u32 es_wt;
	u32 es_buf_size;
	//char es_data[ADEC_ES_BUF_SIZE];
	u64 data_phy;	//ADEC_ES_BUF_SIZE
	u64 data_kern_vir;
	u64 data_ta_vir;
	u64 data_usr_vir;
} ADEC_CACHE_ALIGN adec_es_buf_t;

typedef struct adec_info_s
{
	adec_status_e status;
	adec_decode_state_e decode_state;
	u32 aud_type;
	u32 is_supported;
	char decoder_name[16];
	u32 parse_ok;
	u32 parse_err;
	u32 decode_ok;
	u32 decode_err;
	u32 frm_idx;
	u32 sample_rate;
	u32 ch_num;
	u32 sample_num;
	
	u32 ad_enable;
	u32 ad_volume;
	u32 ad_frm_idx;
	u32 ad_parse_ok;
	u32 ad_parse_err;
	u32 ad_decode_ok;
	u32 ad_decode_err;
} ADEC_CACHE_ALIGN adec_info_t;

typedef struct adec_ac4_config_s
{
	int pcm_out_type;		/**< Output PCM channels, see: MT_PCM_Output_Type_e
								1: PCM_2_0 (default)
								2: PCM_5_1
								3: PCM_RAW */
	int downmix_type;		/**< Indicates which downmix type is used (0: LtRt, 1: LoRo, 2: ARIB(HEAAC)). (default: 0) */
	int de_value;			/**< Dialogue Enhancement value, is either applied in the AC-4 decoder or DAP.
								Range: 0 to 12 dB (in 1 dB steps, default is 0 dB) */
	/* DD/DDP Encode */
	int ddp_out_type;		/**< DDP encoder output type
								0 = None(Disable) (default)
								1 = Dolby Digital(DD) output
								2 = Dolby Digital Plus(DDP) output */
	/* MAT Encode */
	int mat_enc_out_en; 	/**< MAT encoder output enable (default: 0) */
	int associated_audio_mixing;	/**< Enables/Disables mixing in dual decoding use cases. (default: 1) */
	int user_balance_adjustment;	/**< User defined balance between main and associated signal.*/

	char ac4_1st_pref_lang[MT_AC4_DEC_LANG_MAX_LEN];			/**< String containing the preferred language selection (default: eng) */
	char ac4_2nd_pref_lang[MT_AC4_DEC_LANG_MAX_LEN];			/**< String containing the preferred language selection (default: fre) */
	int ac4_associated_type;									/**< Indication of the preferred associated content type. (allowed values are @ref MT_AC4_assoc_type_e) (default: 1) */
	int b_ac4_pref_assoc_type_over_lang;						/**< Preferred filter priority (selection of associated content type over language or vice versa) (default: 1) */
	int ac4_voice_boost_gain;									/**< Indicates AC-4 dialogue enhancement voice boost gain. Range: 0 to 12 dB (in 1 dB steps) (default: 0) */
	int ac4_pres_group_index;									/**< Presentation group index to be decoded. Overrides presentation selection by language and assoc. type.
																	0...%d: Presentation group index
																	-1: 	 Switch back to automatic selection by language and associated type (default)*/
	int ac4_short_program_id;									/**< Short program identifier (16 bit) in the range 0-65536 or -1 for no program ID (default: -1) */

	/* DAP Encode */
	int dap_out_type;		/**< DAP output type
							0 = None(Disable) (default)
							1 = DAP Speaker output
							2 = DAP Headphone output */
} ADEC_CACHE_ALIGN  adec_ac4_config_t;

typedef struct aud_share_buf_s
{
	u32 dolby_downmix_mode;
	u32 downmix_enable;
	u32 crc_enable;
	u32 log_level_flag;
	// 0:normal(all enable)    1:lc only   2: heaac disable
	// just keep the same with rtos, lc only has not been implemented in decoder.
	u32 aac_dec_sel;

	adec_ree_to_tee_cmd_t adec_cmd;
	adec_es_buf_t es_buf;
	adec_es_buf_t ad_es_buf;
	adec_dump_t pcm_dump;
	adec_dump_t es_dump;
	adec_info_t adec_info;
	// snd
	AUD_SND_DATA_PKT_QUE snd_pkt;
	SND_BUFF snd_sw_buf[AUD_SND_SW_BUFF_NUM];
	
	u32 stream_loop;
	u32 stream_loop_flush;
	u32 ad_vol_weight;
	adec_ac4_config_t ac4_configs;
	u32 dolby_force_ms12_dec;
} ADEC_CACHE_ALIGN aud_share_buf_t;

typedef struct aud_buf_size_s
{
	u32 es_buf_size;
	u32 ad_es_buf_size;
	u32 snd_sw_buf0_size;
	u32 snd_sw_buf1_size;
	u32 snd_sw_buf2_size;
} ADEC_CACHE_ALIGN aud_buf_size_map_t;

typedef struct aud_buf_param_s
{
	u32 share_buf_phy;
	u32 share_buf_phy_av;
	u32 es_buf_phy;
	u32 snd_buf_phy;

	aud_buf_size_map_t aud_buf_size_map;
	u32 ta_enabled;
} aud_buf_param_t;

#endif

