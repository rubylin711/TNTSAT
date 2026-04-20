/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __ADEC_H__
#define __ADEC_H__

#include "mt_drv_proc.h"
#include "mt_drv_mmz.h"
#include "adec/buf/adec_buf_k.h"
#include "snd/snd_api.h"
#include "ipc.h"
#include "sys_define.h"
#include "ipc_common.h"

#define ADEC_CMD_TIMEOUT (1000)

extern u32 g_adec_prt_level;
enum ADEC_PRT_LEVEL_E
{
	ADEC_PRT_DGB = 0,
	ADEC_PRT_INFO,
	ADEC_PRT_ERR,
};
#define adec_prt(pos, format, args...)	\
	if ((0x1 << pos) & g_adec_prt_level)	\
		pr_err("[adec]<%s:%d> "format, __func__, __LINE__, ##args)
#define adec_dbg(...)	\
	adec_prt(ADEC_PRT_DGB, __VA_ARGS__)
#define adec_info(...)	\
	adec_prt(ADEC_PRT_INFO, __VA_ARGS__)
#define adec_err(...)	\
	adec_prt(ADEC_PRT_ERR, __VA_ARGS__)

typedef struct adec_proc_param_s
{
	mt_proc_read_func  proc_read;
	mt_drv_proc_write_func proc_write;
} adec_proc_param_t;

typedef struct adec_dev_s
{
	struct cdev dev;
	dev_t devno;
	struct class *devclass;

	adec_status_e status;

	u32 es_cnt;
	u32 es_size;

	int ad_enable;				// ad function enable (ui open ad)
	//int ad_track_enable;		// have ad data
	u32 ad_es_cnt;
	u32 ad_es_size;

	adec_es_buf_t *es_buf;
	adec_es_buf_t *ad_es_buf;
	mmz_buffer_s tmp_es_input_mmz;

	mmz_buffer_s share_buf_mmz_ta;
	mmz_buffer_s share_buf_mmz_av;
	
	aud_share_buf_t *share_buf;
	unsigned long phy_share_addr;

	adec_info_t *adec_info;
	mmz_buffer_s es_buf_mmz;
	aud_buf_param_t aud_buf_param;

	void *pcm_dump_handle;
	void *es_dump_handle;
	struct task_struct *dump_task_hanlde;

	ipc_pipe_t pipe_recv;
	atomic_t atmOpenCnt;                 /* Open times */
	u32  is_ac3or4;
	u32  ipc_inited;
	u32  flush_flag;
	u32  atype;
	u32  trick_mode;
} adec_dev_t;

#endif

