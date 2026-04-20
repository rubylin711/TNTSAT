/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef _CODEC_PDM_H
#define _CODEC_PDM_H

//#define PDM_BYPASS_TEST
#if defined(PDM_BYPASS_TEST)
#define PDM_RATES      (SNDRV_PCM_RATE_96000)
#else
#define PDM_RATES      (SNDRV_PCM_RATE_16000|SNDRV_PCM_RATE_48000)
#endif

#if defined(PDM_BYPASS_TEST)
#define PDM_FORMATS    (SNDRV_PCM_FMTBIT_S16_4LE|SNDRV_PCM_FMTBIT_S24_LE|SNDRV_PCM_FMTBIT_S32_LE)
#else
#define PDM_FORMATS    (SNDRV_PCM_FMTBIT_S16_4LE|SNDRV_PCM_FMTBIT_S24_LE)
#endif

#endif
