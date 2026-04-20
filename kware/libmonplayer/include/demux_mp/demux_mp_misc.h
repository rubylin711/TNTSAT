/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DEMUX_MP_MISC_H__
#define __DEMUX_MP_MISC_H__

#include "stheader.h"

/*!
  Get current ip
  */
int get_local_ip(char *ip_addr);
/*!
 get ip's address
  */
int get_ip_country(char *ip_addr,char *country_id);
void resync_video_stream(sh_video_t *sh_video);
void resync_audio_stream(sh_audio_t *sh_audio);
void skip_audio_frame(sh_audio_t *sh_audio);
int teletext_control(void *p, int cmd, void *arg);
int usec_sleep(int usec_delay);
#endif
