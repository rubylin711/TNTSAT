/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AudioDecoder.h"
#include "mt_ac4_codec.h"
#include "../ext_decoder.h"
#include "../RingBuffer.h"

#define AC4_FRM_BUFF_SIZE	16*1024
#define AC4_PCM_BUFF_SIZE	2048*4*8
#define AOUT_MAX_CH	8
typedef struct mt_ac4_decoder {
	MT_AC4_AVCodecConfig_t ac4_config;
	MT_AC4_AVCodecContext_t context;
	request_buffer_fn_type request_data;
	output_pcm_fn_type output;
	unsigned char *out_buffer;
	int sample_rate;
	int sample_num;
	int channels;
	int bit_depth;
	int init_done;
	int decode_break;
}mt_ac4_decoder_handle;

typedef struct mt_frame_buffer {
	unsigned char *addr;
	unsigned int rd;
	unsigned int wt;
}mt_frame_buffer_handle;

static mt_ac4_decoder_handle mt_ac4_decoder;
static mt_frame_buffer_handle frame_buffer;

static void ac4_decoder_initialize(request_buffer_fn_type pfn_req, output_pcm_fn_type pfn_output)
{
	int ret = 0;
	unsigned int mem_size = 0;

	if ((NULL == pfn_req) || (NULL == pfn_output)) {
		printf("invalid parameter! [%p/%p]\n", pfn_req, pfn_output);
		return;
	}

	memset(&mt_ac4_decoder, 0, sizeof(mt_ac4_decoder));

	mt_ac4_decoder.context.output_rendering = MT_AC4_DEC_RENDER_512;
	ret = mt_ac4_decode_query_mem(&mt_ac4_decoder.context);
	if (ret != MT_AC4_SUCCESS) {
		printf("error: mt_ac4_decode_query_mem failed return %d!\n", ret);
		return;
	}

	mem_size = mt_ac4_decoder.context.static_size + mt_ac4_decoder.context.dynamic_size;
	
	mt_ac4_decoder.context.mem_ptr = malloc(mem_size);
	
	memset(&mt_ac4_decoder.ac4_config, 0, sizeof(MT_AC4_AVCodecConfig_t));
	ret = mt_ac4_decode_get_default_config(&mt_ac4_decoder.ac4_config);
	if (ret != MT_AC4_SUCCESS) {
		printf("error: mt_ac4_decode_get_default_config failed return %d!\n", ret);
		return;
	}

	mt_ac4_decoder.context.p_conf = &mt_ac4_decoder.ac4_config;
	ret = mt_ac4_decode_init(&mt_ac4_decoder.context);
	if (ret != MT_AC4_SUCCESS) {
		printf("error: mt_ac4_decode_init failed return %d!\n", ret);
		return;
	}

	mt_ac4_decoder.out_buffer = malloc(AC4_PCM_BUFF_SIZE);
	if(NULL == mt_ac4_decoder.out_buffer) {
		printf("malloc ac4 out buffer failed!\n");
		if(mt_ac4_decoder.context.mem_ptr)
			free(mt_ac4_decoder.context.mem_ptr);
		return;
	}
	
	mt_ac4_decoder.request_data = pfn_req;
	mt_ac4_decoder.output = pfn_output;

	memset(&frame_buffer, 0, sizeof(frame_buffer));
	frame_buffer.addr = malloc(AC4_FRM_BUFF_SIZE);
	if(NULL == frame_buffer.addr) {
		printf("malloc frame buffer failed!\n");
		if(mt_ac4_decoder.context.mem_ptr)
			free(mt_ac4_decoder.context.mem_ptr);
		
		if(mt_ac4_decoder.out_buffer)
			free(mt_ac4_decoder.out_buffer);

		return;
	}
	mt_ac4_decoder.init_done = 1;
}

#if 0
static FILE *fp_save_pcm = NULL;
static int dump_count = 0;
static int dump_done = 0;
static void dump_pcm(void *pcm, int count)
{
	int ret = 0;

	if(dump_done == 1)
		return;
	
	if(NULL == fp_save_pcm) {
		printf("dump es start, try to create file: /tmp/nfs/es\n");
		fp_save_pcm = fopen("/tmp/nfs/pcm_dec_16bit_1", "wb");
	}
	
	if(fp_save_pcm) {
		ret = fwrite(pcm, 1, count, fp_save_pcm);
		if(ret != count)
			printf("write error, write %d, act is:%d\n",count,ret);
		else
			dump_count += count;
	}

	if(fp_save_pcm && dump_count >= 2*1024*1024) {
		fclose(fp_save_pcm);
		fp_save_pcm = NULL;
		dump_done = 1;
		printf("dump pcm done!\n");
	}
}
#endif

static int ac4_decoder_run(void)
{
	unsigned int ret, i, index;
	int real_size, req_size;
	MT_AC4_AVFrame_t frame;
	MT_AC4_AVPacket_t avpkt;
	int got_frame_ptr;
	unsigned int frame_buffer_free = 0;
	unsigned int frame_size = 0;
	unsigned char *request_buffer;
	unsigned int channel_cp = 0;
	unsigned int channel_mask = 0;
	unsigned char *pcm_addr[MAX_AC4_OUTPUT_CHANNEL_COUNT] = {NULL};

	if(1 != mt_ac4_decoder.init_done) {
		printf("need init ac4 decoder first!\n");
		return -1;
	}

	while(1) {
		if(1 == mt_ac4_decoder.decode_break)
			break;

		frame_buffer_free = AC4_FRM_BUFF_SIZE - frame_buffer.wt;
		req_size = frame_buffer_free > REQUEST_PACKET_SIZE ? REQUEST_PACKET_SIZE : frame_buffer_free;
		request_buffer = (unsigned char *)mt_ac4_decoder.request_data((size_t *)&real_size, req_size);
		if(real_size) {
			memcpy(frame_buffer.addr + frame_buffer.wt, request_buffer, real_size);
			frame_buffer.wt += real_size;
		}

		avpkt.p_data[0] = frame_buffer.addr;
		avpkt.size[0]  = (int)frame_buffer.wt;
		
		//ret is consumed size
		ret = mt_ac4_decode_frame(&mt_ac4_decoder.context, &frame, &got_frame_ptr, &avpkt);
		if (ret < 0) {
			printf("deocde error, ret=0x%x!\n", ret);
			frame_buffer.rd = 0;
			frame_buffer.wt = 0; //drop whole frame data
			continue;
		}
		
		//printf("ac4 ch:%d, rate:%d, samples:%d, bitdept:%d, channel mask:0x%x, acmode:0x%x\n",
			//frame.channels, frame.sample_rate, frame.nb_samples, frame.bit_depth, frame.channel_mask, frame.p_stream_info->acmode);

		//printf("consumed:%d %d\n",ret, avpkt.bytes_consumed[0]);
		frame_buffer.rd += ret;
		memmove(frame_buffer.addr, frame_buffer.addr + frame_buffer.rd, frame_buffer.wt - frame_buffer.rd);
		frame_buffer.wt -= frame_buffer.rd;
		frame_buffer.rd = 0;

		if (got_frame_ptr && (frame.decode_error_flags == 0)) {
			// copy pcm data to output buf
			mt_ac4_decoder.channels = frame.channels;
			mt_ac4_decoder.sample_num = frame.nb_samples;
			mt_ac4_decoder.sample_rate = frame.sample_rate;
			mt_ac4_decoder.bit_depth = frame.bit_depth;
			frame_size = (frame.bit_depth>>3)*frame.nb_samples;
			if(frame_size * frame.channels > AC4_PCM_BUFF_SIZE) {
				printf("ac4 pcm buffer is too small!!\n");
				continue;
			}

			if(32 == frame.bit_depth) {
				for(i = 0; i < MAX_AC4_OUTPUT_CHANNEL_COUNT; i++) {
					if (frame.channel_mask & (1 << i)) {
						unsigned int *pcm = (unsigned int *)frame.p_data[i];
						for(index = 0; index < frame.nb_samples; index++) {
							*pcm = (*pcm) >> 16;
							pcm++;
						}
					}
				}
			}

			/*	
			* Channel Route:
			*	0: L
			*	1: R
			*	2: C
			*	3: LFE
			*	4: Ls
			*	5: Rs
			*	6: Ltop, LT_Front
			*	7: Rtop, RT_Front
			*	8: LT_Back, Ls_DMX
			*	9: RT_Back, Rs_DMX
			*/
			//aout channel route l/r/lfe/c/sl/sr/rsl/rsr
			//remap output channel here if needed
			channel_mask = 0;
			for(i = 0; i < MAX_AC4_OUTPUT_CHANNEL_COUNT; i++) {
				if(i == 2) {//lfe
					pcm_addr[i] = frame.p_data[3];
					channel_mask |= (frame.channel_mask & (1 << 3)) >> 1;
				} else if(i == 3) {//c
					pcm_addr[i] = frame.p_data[2];
					channel_mask |= (frame.channel_mask & (1 << 2)) << 1;
				} else {
					pcm_addr[i] = frame.p_data[i];
					channel_mask |= (frame.channel_mask & (1 << i));
				}
			}
			//printf("channel mask:0x%x 0x%x\n",frame.channel_mask, channel_mask);
			//for(i = 0; i < frame.channels; i++) {

			channel_cp = 0;
			for(i = 0; i < MAX_AC4_OUTPUT_CHANNEL_COUNT; i++) {
				if (channel_mask & (1 << i)) {
					memcpy(mt_ac4_decoder.out_buffer+frame_size*channel_cp, pcm_addr[i], frame_size);
					channel_cp++;
				}
				
				if(AOUT_MAX_CH == channel_cp)
					break;
			}
			//}
			mt_ac4_decoder.output((void*)mt_ac4_decoder.out_buffer, frame.nb_samples);
		}
	}
	
	return 0;
}

static void ac4_decoder_break(void)
{
	mt_ac4_decoder.decode_break = 1;
}

static int ac4_decoder_getinfo(AUDIO_INFO_T *pinfo)
{
	if(NULL == pinfo) {
		printf("invalid params!\n");
		return -1;
	}
	
	pinfo->channels = mt_ac4_decoder.channels;
	pinfo->sample_rate = mt_ac4_decoder.sample_rate;
	pinfo->bitdepth = mt_ac4_decoder.bit_depth;

	return 0;
}

static void ac4_decoder_finalize(void)
{
	mt_ac4_decode_finalize(&mt_ac4_decoder.context);
	
	if(mt_ac4_decoder.context.mem_ptr)
		free(mt_ac4_decoder.context.mem_ptr);

	if(mt_ac4_decoder.out_buffer)
		free(mt_ac4_decoder.out_buffer);

	if(frame_buffer.addr)
		free(frame_buffer.addr);

	mt_ac4_decoder.context.mem_ptr = NULL;
	mt_ac4_decoder.out_buffer = NULL;
	frame_buffer.addr = NULL;
}

ext_decoder_t ext_ac4_decoder = {
	.name			= "ac4",
	.version		= "v1.0.0",
    .initialize		= ac4_decoder_initialize,
    .finalize		= ac4_decoder_finalize,
    .stop			= ac4_decoder_break,
    .run			= ac4_decoder_run,
    .getinfo		= ac4_decoder_getinfo,
};

ext_decoder_t *g_ext_ac4_decoder = &ext_ac4_decoder;

