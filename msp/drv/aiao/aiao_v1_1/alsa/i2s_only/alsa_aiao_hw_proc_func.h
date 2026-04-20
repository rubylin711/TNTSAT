/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_AIAO_I2S_ALSA_PROC_H__
#define __DRV_AIAO_I2SALSA_PROC_H__

#include <sound/core.h>
#include "hi_type.h"
#include "alsa_aiao_hw_comm.h"

int hiaudio_ao_hw_proc_init(void * card, const char * name, struct hii2saudio_data *had);
int hiaudio_ai_hw_proc_init(void * card, const char * name, struct hii2saudio_data *had);


void hiaudio_hw_proc_cleanup(void *p);


#endif
