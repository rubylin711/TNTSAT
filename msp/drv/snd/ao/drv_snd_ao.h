/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* drv_snd_ao.h <2022-10-24>																*/
/********************************************************************************************/
#ifndef __DRV_SND_AO_H__
#define __DRV_SND_AO_H__

#include "mt_drv_proc.h"
#include "drv_hdmi_ext.h"
#include "snd/snd_api.h"
#include "adec/adec_api.h"
#include "mt_unf_avplay.h"

#define SND_VOLUME_MAX_SAFE 100
#define SND_VOLUME_MAX 150
#define SND_VOLUME_MAX_VALUE ((1 << 21) - 1)
#define SND_VOLUME_STEP_COEF_100_150_50_ARIA_INT  1086   //(2 ^ 6) ^ (1/50) * 1000
#define SND_VOLUME_MAX_VALUE_SAFE 0x7fff // 1 << 15
#define SND_VOLUME_EF 0x20000  //( 0x20000 * 0x7fff  < 0xffffffff)
#define SND_VOLUME_STEP_COEF_1_100_99_ARIA_INT  137999 // (0x8000 / base is 200  ) ^  1/99     * AUD_VOLUME_EF

/*****************************************************************************
[SND]
******************************************************************************/
#define DRV_SND_PROC_NAME	"snd"
#define DRV_SND_DEV_NAME 	UMAP_DEVNAME_SND
#define DRV_SND_CARD_NUM 	UMAP_DEV_NUM_SND
#define DRV_SND_MIN_MINOR	UMAP_MIN_MINOR_SND
#define DRV_SND_MAX_MINOR	UMAP_MAX_MINOR_SND

#define SND_HW_BUFF_PP_SIZE			(256*1024)
#define SND_HW_BUFF_AOUT_SIZE		(2*1024*1024) //hbra high data rate need more space
#define SND_HW_BUFF_MIX_SIZE		(128*1024)
#define SND_HW_BUFF_SPD_SIZE		(256*1024)

#define SND_SW_BUFF_0_SIZE		(256*1024)
#define SND_SW_BUFF_1_SIZE		(128*1024)
#define SND_SW_BUFF_2_SIZE		(256*1024)

#define SND_BUFF_ADDR_ALIGN		(128)

#define SND_AOUT_CFG_RETRY_MAX	(10)
#define SND_HDMI_CFG_RETRY_MAX	(10)
#define SND_DATA_SEND_RETRY_MAX	(30)

//proc info setting
#define TEXT_COLOR_PINK	"\33[35m"	/* pink text color*/
#define TEXT_COLOR_RED	"\33[31m"	/* red text color */
#define TEXT_COLOR_END	"\33[0m"	

#define ITEM_TOP_LINE 		"------------"

//#define ITEM_PKT_INFO 		"| Pkt Info |"
#define ITEM_BTM_LINE1 		"--------------------------------------------------"
#define CONTENT_PKT_INFO 	"|  PKT Num |   RD   |   WT   |  WtCnt  | Percent |"

//#define ITEM_SW_BUFF	 	"|  SW Buff |"
#define ITEM_BTM_LINE2 		"----------------------------------------------------------------------"
#define CONTENT_SW_BUFF 	"|  SW Buff |  Size  |   RD   |   WT   |  WtCnt  |  WtByte  | Percent |"

//#define ITEM_HW_BUFF	 	"|  HW Buff |"
#define ITEM_BTM_LINE3 		"-------------------------------------------------------------------------------"
#define CONTENT_HW_BUFF 	"|  HW Buff |  Size  |   WT   |  WtCnt  |  WtByte  | SwIdx |  HwCnt  | Percent |"

#define ITEM_SND_INFO 		"| Snd Info |"
#define CONTENT_SND_INFO1 	"| State: STOP | Volme(Gain/Scale): 00 / 7fff | Apts: 00000000 | FrmIdx: 00000 |"
#define CONTENT_SND_INFO2 	"| Mute(Hdmi8Ch/Adac2Ch/SpdCoax/SpdHdmi/I2S): 00 / 0 / 0 / 0 / 0 |"
#define CONTENT_SND_INFO3 	"| CON | Samplerate: 000000 | Channel: 8 | FrameSamples: 000000 | BitDepth: 16 |"
#define CONTENT_SND_INFO4 	"| FIG | Fmt0:  PCM | Fmt1:  AC3 | Fmt2: EAC3 | Interleaved: 16 | ValidBit: 16 |"


#define CONTENT_SND_INFO9 	"| REG(00H):   00000000   00000000   00000000   00000000   00000000   00000000 |"




/*****************************************************************************
[DEBUG]
******************************************************************************/
extern mt_u32 g_drv_snd_log_level;
#define LOG_DRV_SND_PREFIX		"[SND]"
#define LOG_SND_APDS_PREFIX		"[APDS]"
#define LOG_LEVEL_NULL			(0)
#define LOG_LEVEL_ERR			(1)
#define LOG_LEVEL_INFO			(2)
#define LOG_LEVEL_DBG			(3)
#define LOG_LEVEL_BIT			(28)
#define LOG_LEVEL_FUNC_LINE		(0x1<<31)
#define LOG_LEVEL_CONFIG		(0x7 & (g_drv_snd_log_level>>28))
#define LOG_LEVEL_APDS			(0xfffffff & g_drv_snd_log_level)
#define LOG_SHOW_FUNC_LINE		(g_drv_snd_log_level>>31)

#define AO_LOG_ERR(fmt, args...)	\
	do{							\
		if(LOG_LEVEL_ERR <= LOG_LEVEL_CONFIG){	\
			if(LOG_SHOW_FUNC_LINE){	\
				printk(KERN_ERR LOG_DRV_SND_PREFIX "<%s:%d>" fmt, __func__, __LINE__, ##args);	\
			}else{	\
				printk(KERN_ERR LOG_DRV_SND_PREFIX fmt, ##args);	\
			}	\
		}	\
	}while(0)

#define AO_LOG_INFO(fmt, args...)	\
	do{							\
		if(LOG_LEVEL_INFO <= LOG_LEVEL_CONFIG){	\
			if(LOG_SHOW_FUNC_LINE){	\
				printk(KERN_ERR LOG_DRV_SND_PREFIX "<%s:%d>" fmt, __func__, __LINE__, ##args);	\
			}else{	\
				printk(KERN_ERR LOG_DRV_SND_PREFIX fmt, ##args);	\
			}	\
		}	\
	}while(0)

#define AO_LOG_DBG(fmt, args...)	\
	do{							\
		if(LOG_LEVEL_DBG <= LOG_LEVEL_CONFIG){	\
			if(LOG_SHOW_FUNC_LINE){	\
				printk(KERN_ERR LOG_DRV_SND_PREFIX "<%s:%d>" fmt, __func__, __LINE__, ##args);	\
			}else{	\
				printk(KERN_ERR LOG_DRV_SND_PREFIX fmt, ##args);	\
			}	\
		}	\
	}while(0)

#define AO_LOG_APDS(pos, fmt, args...)	\
	do{							\
		if(pos & LOG_LEVEL_APDS){	\
			printk(KERN_ERR LOG_SND_APDS_PREFIX fmt, ##args);	\
		}	\
	}while(0)

//#define AO_LOG_INFO(fmt, args...)		printk(KERN_ERR LOG_PREFIX "%s-L%d:" fmt, __func__, __LINE__, ##args)
//#define AO_LOG_DBG(fmt, args...)		printk(KERN_ERR LOG_PREFIX "%s-L%d:" fmt, __func__, __LINE__, ##args)
	
typedef enum SND_LOG_POS_e
{
	SND_LOG_POS_ERR = 0,
	SND_LOG_POS_FLOW,

	SND_LOG_POS_TODO = 4,
	
	SND_LOG_POS_FUNC_LINE = 31,
	SND_LOG_POS_INVALID
}SND_LOG_POS;
#define SND_LOG_PREFIX		"[SND]"
#define SND_LOG_LEVEL		(0x7fffffff & g_snd_log_level)
#define SND_LOG_FUNC_LINE	(g_snd_log_level>>SND_LOG_POS_FUNC_LINE)

#define SND_LOG(pos, fmt, args...)	\
	do{	\
		if((0x1<<pos) & SND_LOG_LEVEL){	\
			if(SND_LOG_FUNC_LINE){	\
				printk(KERN_ERR SND_LOG_PREFIX "%s-L%d:" fmt, __func__, __LINE__, ##args);	\
			}else{	\
				printk(KERN_ERR SND_LOG_PREFIX fmt, ##args);	\
			}	\
		}	\
	}while(0)

/*****************************************************************************
[]
******************************************************************************/
typedef enum SND_CARD_STA_e
{
	SND_CARD_STA_IDLE = 0,
	SND_CARD_STA_STOP,
	
	SND_CARD_STA_PKT_POP,
	SND_CARD_STA_AVSYNC,
	SND_CARD_STA_AOUT_CFG,
	SND_CARD_STA_HDMI_CFG,
	SND_CARD_STA_HW_BUFF_WRITE,
	SND_CARD_STA_INVALID
}SND_CARD_STA;

typedef struct SND_PRIV_DATA_t
{
	//struct clk *audoutclk;
	//struct clk *audoutaxiclk;

	mt_u32 test;
} SND_PRIV_DATA;

typedef struct SND_PROC_PARAM_t
{
	mt_proc_read_func  pfnReadProc;
	mt_drv_proc_write_func pfnWriteProc;
} SND_PROC_PARAM;

typedef struct SND_DATA_FMT_t
{
	AUD_SND_DATA_FMT fmt;
	mt_u32 sampleRate;
	mt_u32 frameSample;
	mt_u32 chanel;
} SND_DATA_FMT;

typedef struct SND_CARD_t
{
	u8 idx;
	u8 aout_need_cfg;
	u8 hdmi_need_cfg;
	u8 stop_flag;
	u32 play_cnt;
	u32 eos;
	SND_CARD_TYPE type;
	SND_CARD_STA sta;
	AUD_SND_DATA_PKT_QUE *pPkt;
	AUD_SND_DATA_PKT stCfg;
	MT_UNF_SND_SPDIF_MODE_E spdif_mode;
	MT_UNF_SND_SPDIF_MODE_E user_spdif_mode;
	MT_UNF_SND_HDMI_MODE_E hdmi_mode;
	MT_UNF_SND_HDMI_MODE_E user_hdmi_mode;
	HDMI_EXPORT_FUNC_S	*pstHdmiFunc;
	MT_UNF_TRACK_MODE_E track_mode;

	mmz_buffer_s hw_buf_mmz[AUD_SND_HW_BUFF_NUM];
	mmz_buffer_s sw_buf_mmz;	
	
	SND_BUFF hwBuff[AUD_SND_HW_BUFF_NUM];
	SND_BUFF *swBuff[AUD_SND_SW_BUFF_NUM];

	aud_share_buf_t *share_buf;

	u8 volume;
	u8 hdmi_mute;
	u8 spdif_mute;
	u8 adac_mute;

	//MT_UNF_SND_HDMI_MODE_E hdmi_mode;
	//MT_UNF_SND_SPDIF_MODE_E spdif_mode;
	u32 hdmi_cfg_retry;

	u32 drop_cnt;
	u32 crc_buf_size;
	u32 *crc_buf_vir;
	phys_addr_t crc_buf_phy;
	mmz_buffer_s crc_buf_mmz;
	
	u32 last_pts;
	u32 buffer_thd;

	MT_HA_AUDIO_STREAM_INFO_S stream_info;
	MT_UNF_AVPLAY_CI_TEST_INFO_S aud_ci_test_info;

	u32	tts_buffer_size;
	u8*	tts_buffer_vir;
	phys_addr_t	tts_buffer_phy;
	u32	tts_buffer_rd;
	u32	tts_buffer_wt;
	u32	tts_temp_size;
	u32* tts_temp_vir;
	phys_addr_t	tts_temp_phy;
	u8 aout_init;
	u8 pause_flag;
} SND_CARD;

long set_volume(SND_CARD *snd_card, ulong volume);


#endif
//TEST_JY


