/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*
*/
#ifndef __EXT_DECODER_H__
#define __EXT_DECODER_H__

#define REQUEST_PACKET_SIZE  4096
typedef void* (*request_buffer_fn_type)(size_t *realsize, size_t reqsize);
typedef void (*update_buffer_fn_type)(size_t size);
typedef int (*output_pcm_fn_type)(void *pcm, int count);
typedef struct __audio_info {
    int channels;
    int bitdepth;
    int sample_rate;
	int channelsOriginal;
}AUDIO_INFO_T;

typedef struct __obj_info {
	int16_t obj_id;
	char obj_name[24];
	int16_t interact;
}OBJ_INFO_T;

typedef struct __audio_meta_info {
	int16_t obj_num;
	OBJ_INFO_T obj_info[8];

	int16_t complementary_object_group;
	int16_t complementary_object[4];
	int16_t complementary_object_id[4][8];

}AUDIO_META_INFO_T;

typedef struct ext_decoder_s
{
	const char *name;
	const char *version;
	void (*initialize)(request_buffer_fn_type pfn_req, update_buffer_fn_type pfn_update, output_pcm_fn_type pfn_output);
	void (*finalize)(void);
	/* Force stop the decoder run loop*/
	void (*stop)(void);
	/*The decoder will run forever until the request buffer return with 0 or the audio_decoder_break() is called*/
	int (*run)(void);
	int (*getinfo)(AUDIO_INFO_T *pinfo);
	int (*getmetainfo)(AUDIO_META_INFO_T *pmetainfo);
	int (*setpos)(float x, float y, float z);
	int (*selobj)(unsigned short id, unsigned short on);
} ext_decoder_t;

ext_decoder_t *attach_ext_decoder(unsigned int atype);

#endif //__EXT_DECODER_H__

