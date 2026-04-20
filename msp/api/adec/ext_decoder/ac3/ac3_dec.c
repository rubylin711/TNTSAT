/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AudioDecoder.h"
#include "../ext_decoder.h"
#include "../RingBuffer.h"

static void ac3_decoder_initialize(request_buffer_fn_type pfn_req, output_pcm_fn_type pfn_output)
{
	if ((NULL == pfn_req) || (NULL == pfn_output)) {
		printf("invalid parameter! [%p/%p]\n", pfn_req, pfn_output);
		return;
	}

	audio_decoder_initialize(pfn_req, pfn_output);
}

static int ac3_decoder_run(void)
{
	return audio_decoder_run();
}

static void ac3_decoder_break(void)
{
	audio_decoder_break();
}

static int ac3_decoder_getinfo(AUDIO_INFO_T *pinfo)
{
	if(NULL == pinfo) {
		printf("invalid params!\n");
		return -1;
	}

	audio_decoder_getinfo(pinfo);

	return 0;
}

static void ac3_decoder_finalize(void)
{
	audio_decoder_finalize();
}

ext_decoder_t g_ext_ac3_decoder = {
	.name			= "ac3",
	.version		= "v1.0.0",
    .initialize		= ac3_decoder_initialize,
    .finalize		= ac3_decoder_finalize,
    .stop			= ac3_decoder_break,
    .run			= ac3_decoder_run,
    .getinfo		= ac3_decoder_getinfo,
};

ext_decoder_t *g_ext_ac3_decoder = &g_ext_ac3_decoder;

