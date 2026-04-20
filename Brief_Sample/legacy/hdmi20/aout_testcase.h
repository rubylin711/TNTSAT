/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __AOUT_TESTCASE_H__
#define __AOUT_TESTCASE_H__

/*=========================================================================
| TYPEDEFS
 ========================================================================*/
#define AUD_DATA_PLAY_SIZE (4<<20)  //1M Bytes
typedef struct AUD_INPUT_S {
	mt_u32 mode; //'0': pcm->I2S, '1':ac3/eac3/dts -> SPDIF, '2' pcm->SPDIF
	mt_u32 fs;//'0': 32K, '1':48K, '2':96K, '3':192K, '4':44.1K,'5':88.2K,'6':176.4K
	mt_u32 ch;//'1': 1 memo, '2':2 channels, '6':5.1channels, '8':7.1channels
	mt_u32 downmix;//'0': normal, '1':downmix enable
	mt_u32 mode_ex;//'0': normal, '1':eac3
} AUD_INPUT_T;
/***** local functions *******************************************************/
mt_u32 do_case_iis_audio(void);

/***** end of file ***********************************************************/

#endif /* __AOUT_TESTCASE_H__ */
