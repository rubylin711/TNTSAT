/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_AIAO_HDMI_ALSA_PROC_H__
#define __DRV_AIAO_HDMI_ALSA_PROC_H__

#include <sound/core.h>
#include "hi_type.h"
#include "alsa_aiao_hdmi_comm.h"

int hiaudio_ao_hdmi_proc_init(void * card, const char * name, struct hihdmiaudio_data *had);


void hiaudio_hdmi_proc_cleanup(void *p);


#endif
