/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __SAMPLE_AUDIO_H_
#define __SAMPLE_AUDIO_H_

#include "mt_unf_avplay.h"


typedef enum
{
  /*! 
    Sample rate is 48k 
    */
  AUDIO_SAMPLE_48 = 5,
  /*! 
    Sample rate is 44.1k 
    */
  AUDIO_SAMPLE_44 = 4,
  /*! 
    Sample rate is 32k
    */
  AUDIO_SAMPLE_32 = 6,
  /*! 
    Sample rate is 24k 
    */
  AUDIO_SAMPLE_24 = 1,
  /*!
    Sample rate is 22.05k 
    */
  AUDIO_SAMPLE_22 = 0,
  /*!
    Sample rate is 16k 
    */
  AUDIO_SAMPLE_16 = 2, 
  /*! 
    Sample rate is 96k
    */
  AUDIO_SAMPLE_96 = 13
} audio_sample_rate_t;


#endif
