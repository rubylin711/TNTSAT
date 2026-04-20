/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __COMMON_AUDIO_H__
#define __COMMON_AUDIO_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "mt_type.h"
#include "mt_adp_boardcfg.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************* API declaration *****************************/

mt_s32 AudioSIOPinSharedEnable(void);

mt_void AudioSlicTlv320RST(void);

mt_void AudioSPIPinSharedEnable(void);


#ifdef __cplusplus
}
#endif
#endif /* __SAMPLE_AUDIO_PUB_H__ */
