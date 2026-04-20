/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2018, Montage Technology Co., Ltd.
 *
 * File Name      : drv_vdec_debug.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2018/1/4
 * Description    : MT VDEC DRV Debug & Dump functions.
 * History        :
 * 1.Date         : 2018/1/4
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef _INC_MT_DRV_VDEC_DEBUG_H_
#define _INC_MT_DRV_VDEC_DEBUG_H_

#include "drv_vdec_private.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------//
//File
struct file *open_file(char *path,int flag,int mode);

int read_file(struct file *fp,char *buf,int readlen);

int write_file(struct file *fp,char *buf,int len);

int close_file(struct file *fp);

int sync_file(struct file *fp);

//----------------------------------------------------------------------------//
//Dump
void dump_int_array(const char *title, int *ar, int n);

/* dump SLOT Info */
void vdec_dump_slot(MT_DIS_FRAME_SLOT_INFO_T *pSlot);

/* dump IMAGE */
void vdec_dump_image(IMAGE *pImage);

/* dump VPSS frame */
void vdec_dump_vpss_frame(MT_DRV_VIDEO_FRAME_PACKAGE_S *pstFrm);

/* dump create channel option */
void vdec_dump_chopt(ChannelInfo_t *pChOpt);

/* dump channel configuration */
void vdec_dump_chcfg(VDEC_CHAN_CFG_S *pCfg);

/* dump channel attribute */
void vdec_dump_chattr(MT_UNF_VCODEC_ATTR_S *pAttr);

/* dump video DMX&DEC's ES buffer stat */
void vdec_dump_es_stat(mt_handle hVdec, VDEC_CHANNEL_S *pstChan);

//----------------------------------------------------------------------------//
//Dump ES Data
int vdec_dump_es_get_enable(void);
char *vdec_dump_es_get_file_name(void);
void vdec_dump_es_set_enable(char *file_name/*dump file name*/);
void vdec_dump_es_set_disable(void);

int vdec_dump_es_write(char* pbuf, int len);

//Video ES Bit-Error-Ratio Disturb
int vdec_ber_disturb(char* pbuf, int len);

/* video driver log to file */
extern spinlock_t kVDebugLock;
extern struct file *gp_VDebugLogFile;
extern int g_VDebugEnable;
#undef LOG_TIME_FMT2
#define LOG_TIME_FMT2			"%ld.%06ld "
#define LOG_TID_FMT				"(%3u)"
#define VDEBUG_LOG(fmt, ...)	do {														\
									if (g_VDebugEnable) {									\
										if (gp_VDebugLogFile != NULL) {						\
											char str[256];									\
											struct timespec tp;								\
											getrawmonotonic(&tp);							\
											spin_lock(&kVDebugLock);						\
											scnprintf(str, 256, LOG_TIME_FMT2 LOG_TID_FMT ": " fmt, tp.tv_sec, tp.tv_nsec / 1000, current->pid, ## __VA_ARGS__); \
											write_file(gp_VDebugLogFile, str, strlen(str));	\
											spin_unlock(&kVDebugLock);						\
										} else {											\
											printk(fmt, ## __VA_ARGS__);					\
										}													\
									}														\
								} while (0)

int vdec_debug_start(mt_handle hVdec, VDEC_CHANNEL_S *pstChan);
int vdec_debug_stop(void);

#ifdef __cplusplus
}
#endif

#endif

