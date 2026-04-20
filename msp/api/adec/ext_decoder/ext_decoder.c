/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "mt_audio_codec.h"
#include "ac3/ac3_dec.h"
#include "ac4/ac4_dec.h"
#include "vvid/vvid_dec.h"

ext_decoder_t *attach_ext_decoder(unsigned int atype)
{
	ext_decoder_t *decoder = NULL;
	
	switch (atype) {
		case HA_AUDIO_ID_DOLBY_PLUS:
			decoder = NULL;//&g_ext_ac3_decoder;
			break;
			
	#ifdef CONFIG_MT_DOLBY_AC4_SUPPORT
		case HA_AUDIO_ID_DOLBY_AC4:
			decoder = g_ext_ac4_decoder;
			break;
	#endif
	
	#ifdef CONFIG_MT_EXT_VVID_SUPPORT
		case HA_AUDIO_ID_VVID:
			decoder = g_ext_vvid_decoder;
			break;
	#endif
	
		default:
			decoder = NULL;
			break;
	}
	return decoder;
}


