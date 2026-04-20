/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../ext_decoder.h"
#include "../RingBuffer.h"
#include "avs3_stat_dec.h"
#include "avs3_prot_dec.h"
#include "samplerate.h"

#define MAX_CHANNELS 16
#define FRAME_LEN 1024
#define VVID_FRM_BUFF_SIZE	16*1024
#define _DO_RENDER_
//#define RESAMPLE_OUT 48000

#ifdef RESAMPLE_OUT
float resample_buf[1024*4] = {0};
#endif

typedef struct vvid_user_config {
	float pos_x;
	float pos_y;
	float pos_z;
	uint16_t obj_gain[32];
}user_config;

typedef struct vvid_decoder { 
	uint16_t inited;
	AVS3DecoderHandle hAvs3Dec;
	uint16_t crcBs;
	int16_t bytesPerFrame;
	request_buffer_fn_type request_data;
	update_buffer_fn_type update_data;
	output_pcm_fn_type output;
	
	int sample_rate;
	int sample_num;
	int channels;
	int bit_depth;
	int decode_break;

	void* hStreamRender;
	Avs3MetaData avs3Metadata;
	float* renderInFlt;		
	float* renderOutFlt;	
	int16_t* renderOutInt16; 

	user_config config;
}mt_vvid_decoder_handle;

typedef struct mt_vvid_frame_buffer {
	unsigned char *addr;
	unsigned int rd;
	unsigned int wt;
}mt_vvid_frame_buffer_handle;

static mt_vvid_decoder_handle *mt_vvid_decoder;

void* CreateRenderer(Avs3MetaData *metadata, int sampleRate, int blockSize);
int PutInterleavedAudioBuffer(void* render, const float *buffer, int frameNum, int channelNum);
int GetBinauralInterleavedAudioBuffer(void* render, float *buffer, int frameNum);
int UpdateMetadata(void* render, Avs3MetaData *metadata);
int SetListenerPosition(void* render, float *position, float *front, float *up);
int DestroyRenderer(void* render);

static void vvid_decoder_initialize(request_buffer_fn_type pfn_req, update_buffer_fn_type pfn_update, output_pcm_fn_type pfn_output)
{
	uint16_t i = 0;
	if ((NULL == pfn_req) || (NULL == pfn_output)) {
		printf("invalid parameter! [%p/%p]\n", pfn_req, pfn_output);
		return;
	}

	mt_vvid_decoder = (mt_vvid_decoder_handle *)malloc(sizeof(mt_vvid_decoder_handle));
	if(NULL == mt_vvid_decoder) {
		printf("malloc for vvid decoder handle failed!\n");
		return;
	}

	memset(mt_vvid_decoder, 0, sizeof(mt_vvid_decoder_handle));
	
	mt_vvid_decoder->request_data = pfn_req;
	mt_vvid_decoder->update_data = pfn_update;
	mt_vvid_decoder->output = pfn_output;
	mt_vvid_decoder->hAvs3Dec = (AVS3DecoderHandle)malloc(sizeof(AVS3Decoder));
	if(NULL == mt_vvid_decoder->hAvs3Dec) {
		printf("malloc for vvid decoder failed!\n");
		return;
	}

	mt_vvid_decoder->inited = 0;
	mt_vvid_decoder->renderInFlt = (float*)malloc(16 * FRAME_LEN * sizeof(float));
	mt_vvid_decoder->renderOutFlt = (float*)malloc(2 * FRAME_LEN * sizeof(float));
	mt_vvid_decoder->renderOutInt16 = (int16_t*)malloc(2 * FRAME_LEN * sizeof(int16_t));

	mt_vvid_decoder->config.pos_x = 0;
	mt_vvid_decoder->config.pos_y = 0;
	mt_vvid_decoder->config.pos_z = 0;
	for(i=0; i<32; i++)
		mt_vvid_decoder->config.obj_gain[i] = 0xff;
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

static void MvByte2Byte(const int8_t srcByte[], int8_t destByte[], const short n)
{
    for (int16_t i = 0; i < n; i++) {
        destByte[i] = srcByte[i];
    }

    return;
}

#define AMPLIFY_FACTOR_INT16    (32767.0f)                       // 2^15 - 1
#define SHRINK_FACTOR_INT16     (0.000030517578125f)             // 1 / 2^15
#define CONVERT_INT16_TO_FLOAT(ptr) (*((short*)(ptr)) * SHRINK_FACTOR_INT16)
#define CONVERT_FLOAT_TO_INT16(ptr) ((short)((*ptr) * AMPLIFY_FACTOR_INT16))

/* Range controlled at [-1, 1]*/
static void rangLimit(float *a)
{
    if (*a > 1.0) {
        *a = 1.0;
    } else if (*a < -1.0) {
        *a = -1.0;
    }
}

static int int16ToFloat(float *Out, const short *In, int len)
{
    // len is even
    for (int i = 0; i < len; i += 2) {
        *Out = CONVERT_INT16_TO_FLOAT(In);
        *(Out + 1) = CONVERT_INT16_TO_FLOAT(In + 1);
        Out += 2;
        In += 2;
    }
    return 0;
}

static int floatToInt16(short *Out, const float *In, int len)
{
    // len is even
    float TmpIn;
    for (int i = 0; i < len; i += 2) {
        TmpIn = *In;
        rangLimit(&TmpIn);
        *Out = CONVERT_FLOAT_TO_INT16(&TmpIn);
        TmpIn = *(In + 1);
        rangLimit(&TmpIn);
        *(Out + 1) = CONVERT_FLOAT_TO_INT16(&TmpIn);
        Out += 2;
        In += 2;
    }
    return 0;
}

#if 0//for debug
#include <stdio.h>
#include <stdint.h>
static uint32_t write_samples = 0;

typedef struct {
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    uint32_t wav_size;   // Size of the WAV portion of the file, which follows the first 8 bytes.
    char wave_header[4]; // Contains "WAVE"

    // fmt Chunk
    char fmt_header[4];  // Contains "fmt "
    uint32_t fmt_chunk_size; // Length of the data following this number
    uint16_t audio_format;   // PCM = 1 (i.e., Linear quantization)
    uint16_t num_channels;   // Number of channels. Mono = 1, Stereo = 2, etc.
    uint32_t sample_rate;    // Samples per second (Hz)
    uint32_t byte_rate;      // SampleRate * NumChannels * BitsPerSample/8
    uint16_t block_align;    // NumChannels * BitsPerSample/8
    uint16_t bits_per_sample;// Number of bits per sample

    // Data Chunk
    char data_header[4];     // Contains "data"
    uint32_t data_bytes;     // Number of bytes in data. Size of data part.
} WavHeader;

void write_wav_header(FILE *fp, int rate, int channel, int num_samples, int width) {
    WavHeader header = {
        {'R', 'I', 'F', 'F'},
        /* The size of the entire file minus 8 bytes for the beginning chunk descriptor */
        36 + num_samples * channel * (width>>3),
        {'W', 'A', 'V', 'E'},
        {'f', 'm', 't', ' '},
        16,	// Size of the fmt chunk - 16 for PCM
        1,  // Format code (1 is PCM)
        channel,   // Number of channels
        rate,  // Sample rate
        rate * channel * (width>>3),  // Byte rate (Sample rate * NumChannels * BitsPerSample / 8)
        channel *(width>>3),  // Block align (NumChannels * BitsPerSample / 8)
        width, // Bit depth
        {'d', 'a', 't', 'a'},
        num_samples * channel * (width>>3) // Data size (NumSamples * NumChannels * BitsPerSample / 8)
    };

	printf("width:%d, size:0x%08x\n",width,num_samples * channel * (width>>3));
    fwrite(&header, sizeof(WavHeader), 1, fp);
}
#endif

static short pcm_output[MAX_CHANNELS * FRAME_LEN];
static uint32_t interval_output[8 * FRAME_LEN];//temp buffer for aout 8ch, maybe we can ouput 16bit directly.
static float coefx[21] = {1.20, 1.18, 1.16, 1.14, 1.12, 1.10, 1.08, 1.06, 1.04, 1.02, 1, 0.990, 0.980, 0.970, 0.950, 0.920, 0.870, 0.800, 0.720, 0.620, 0.50};
static float coefy[21] = {1.20, 1.10, 1.08, 1.07, 1.06, 1.05, 1.04, 1.03, 1.02, 1.01, 1, 0.990, 0.980, 0.970, 0.960, 0.950, 0.940, 0.930, 0.920, 0.90, 0.80};
static float coefz[21] = {0.80, 0.85, 0.90, 0.95, 0.96, 0.97, 0.980, 0.985, 0.99, 0.995, 1, 1.005, 1.010, 1.015, 1.020, 1.030, 1.040, 1.050, 1.10, 1.150, 1.20};
static void calc_gain_by_position(const float *position, float *gainxl, float *gainxr, float *gainy, float *gainz)
{
	float x,y,z = 0;

	if(NULL == position)
		return;
	
	x = position[0];
	y = position[1];
	z = position[2];

	if(x<-10)
		x=-10;
	else if(x>10)
		x=10;
	*gainxl = coefx[(int)x+10];
	*gainxr = coefx[20-((int)x+10)];

	if(y<-10)
		y=-10;
	else if(y>10)
		y=10;
	*gainy = coefy[(int)y+10];
	
	if(z<-10)
		z=-10;
	else if(z>10)
		z=10;
	*gainz = coefz[(int)z+10];

	//printf("x:%f, y:%f, z:%f, gainxl:%f gainxr:%f, gainy:%f, gainz:%f\n",x,y,z,*gainxl,*gainxr,*gainy,*gainz);
}

static int vvid_output_pcm(AVS3DecoderHandle hAvs3Dec, uint32_t output[8 * FRAME_LEN], short data[MAX_CHANNELS * FRAME_LEN], uint32_t* output_channel)
{
	uint32_t i, j = 0;
	uint32_t channels = 0;
	int16_t* pcm_16bit = NULL;
	float obj_position[3];
	float gainxl, gainxr, gainy, gainz = 1;

#ifdef _DO_RENDER_
	float position[3];
	float front[3];   
	float up[3];	  	
	int32_t frameNums;
	position[0] = 0;
	position[1] = 0;
	position[2] = 0;
	front[0] = 0;
	front[1] = 0;
	front[2] = 1;
	up[0] = 0;
	up[1] = 1;
	up[2] = 0;
#endif

	if (!hAvs3Dec)
	{
		printf("invalid parameter [%p]\n", hAvs3Dec);
		return -1;
	}
#if 0
	static FILE *fp_16bit = NULL;
	static FILE *fp_32bit = NULL;

	if(fp_16bit == NULL) {
		fp_16bit = fopen("/media/sda1/vivid_16bit.wav", "wb");
		if (!fp_16bit) {
			printf("Could not open 16bit file\n");
			return -1;
		}

		write_samples = 0;
		printf("init 16bit file and write wav header!!!!!!!!\n");
		//write_wav_header(fp_16bit, hAvs3Dec->outputFs, hAvs3Dec->numChansOutput, 0, hAvs3Dec->bitDepth);
		write_wav_header(fp_16bit, hAvs3Dec->outputFs, 2, 0, 16);
	}


	if(fp_32bit == NULL) {
		fp_32bit = fopen("/media/sda1/vivid_32bit.wav", "wb");
		if (!fp_32bit) {
			printf("Could not open 32bit file\n");
			return -1;
		}

		write_samples = 0;
		printf("init 32bit file and write wav header!!!!!!!!\n");
		write_wav_header(fp_32bit, hAvs3Dec->outputFs, hAvs3Dec->numChansOutput, 0, 32);
	}

	write_samples += hAvs3Dec->frameLength;
	fwrite(data, hAvs3Dec->frameLength*hAvs3Dec->numChansOutput*(hAvs3Dec->bitDepth>>3), 1, fp_16bit);
	
	for(i=0; i<hAvs3Dec->frameLength*hAvs3Dec->numChansOutput; i++) {
		//uint32_t val = (uint32_t)data[i] & 0xffff;
		//fwrite(&val, 4, 1, fp_32bit);
		
		uint16_t val = 0;
		fwrite(&val, 2, 1, fp_32bit);
		fwrite(&data[i], 2, 1, fp_32bit);
		
	}
#endif
#ifdef _DO_RENDER_
	//if(hAvs3Dec->numChansOutput >= 2) {
		frameNums = hAvs3Dec->frameLength;
		if(NULL == mt_vvid_decoder->hStreamRender && mt_vvid_decoder->avs3Metadata.hasStaticMeta) {
			mt_vvid_decoder->hStreamRender = CreateRenderer(&mt_vvid_decoder->avs3Metadata, 
				mt_vvid_decoder->hAvs3Dec->outputFs, mt_vvid_decoder->hAvs3Dec->frameLength);
			printf("init render handle:%p!\n",mt_vvid_decoder->hStreamRender);
		}

		if(NULL == mt_vvid_decoder->hStreamRender) {
			pcm_16bit = data;
			channels = hAvs3Dec->numChansOutput;
		} else {
			int16ToFloat(mt_vvid_decoder->renderInFlt, data, frameNums * hAvs3Dec->numChansOutput);
			if (PutInterleavedAudioBuffer(mt_vvid_decoder->hStreamRender, mt_vvid_decoder->renderInFlt,
				frameNums, hAvs3Dec->numChansOutput) != 0) {
				return -1;
			}

			//mt_vvid_decoder->avs3Metadata.avs3MetaDataDynamic.avs3DmL1MetaData[0].obj_x = mt_vvid_decoder->config.pos_x/(float)10;
			//mt_vvid_decoder->avs3Metadata.avs3MetaDataDynamic.avs3DmL1MetaData[0].obj_y = mt_vvid_decoder->config.pos_y/(float)10;
			//mt_vvid_decoder->avs3Metadata.avs3MetaDataDynamic.avs3DmL1MetaData[0].obj_z = mt_vvid_decoder->config.pos_z/(float)10;
			obj_position[0] = mt_vvid_decoder->config.pos_x;
			obj_position[1] = mt_vvid_decoder->config.pos_y;
			obj_position[2] = mt_vvid_decoder->config.pos_z;
			if (UpdateMetadata(mt_vvid_decoder->hStreamRender, &mt_vvid_decoder->avs3Metadata) != 0) {
				return -1;
			}

			position[0] = 0;
			position[1] = 0;
			position[2] = 0;
			front[0] = 0;
			front[1] = 0;
			front[2] = 1;
			up[0] = 0;
			up[1] = 1;
			up[2] = 0;
			if (SetListenerPosition(mt_vvid_decoder->hStreamRender, position,
				front, up) != 0) {
				return -1;
			}

			if (GetBinauralInterleavedAudioBuffer(mt_vvid_decoder->hStreamRender, mt_vvid_decoder->renderOutFlt, frameNums) != 0) {
				return -1;
			}

#ifdef RESAMPLE_OUT
			if(mt_vvid_decoder->hAvs3Dec->outputFs != RESAMPLE_OUT){
				int ret = 0;
				SRC_DATA src_data = {0};

				src_data.data_in = mt_vvid_decoder->renderOutFlt;
				src_data.input_frames = frameNums;
				src_data.data_out = resample_buf;
				src_data.output_frames = 1024;
				src_data.src_ratio = (double)RESAMPLE_OUT/(double)mt_vvid_decoder->hAvs3Dec->outputFs;
				ret = src_simple(&src_data, SRC_SINC_FASTEST, 2);

				floatToInt16(mt_vvid_decoder->renderOutInt16, resample_buf, src_data.output_frames_gen * sizeof(int16_t));
				mt_vvid_decoder->hAvs3Dec->outputFs = RESAMPLE_OUT;
			} else {
				floatToInt16(mt_vvid_decoder->renderOutInt16, mt_vvid_decoder->renderOutFlt, frameNums * sizeof(int16_t));
			}
#else
			floatToInt16(mt_vvid_decoder->renderOutInt16, mt_vvid_decoder->renderOutFlt, frameNums * sizeof(int16_t));
#endif
			pcm_16bit = mt_vvid_decoder->renderOutInt16;
			channels = 2;
		
			calc_gain_by_position(obj_position, &gainxl, &gainxr, &gainy, &gainz);
			for(i=0; i<hAvs3Dec->frameLength; i++) {
				pcm_16bit[i*2] *= gainxl*gainy*gainz;
				pcm_16bit[i*2+1] *= gainxr*gainy*gainz;
			}
		}
	//} else {
	//	pcm_16bit = data;
	//	channels = hAvs3Dec->numChansOutput;
	//}
#else
	pcm_16bit = data;
	channels = hAvs3Dec->numChansOutput;
#endif

#if 1
	*output_channel = channels  > 8 ? 8 : channels;
	for(i=0; i<(*output_channel); i++) {
		for(j=0; j<hAvs3Dec->frameLength;) {
			if(hAvs3Dec->outputFs == 192000) {
				interval_output[(hAvs3Dec->frameLength>>1)*i + (j>>1)] = (uint32_t)pcm_16bit[channels*j+i] & 0xffff;
				j += 2;
			} else {
				interval_output[hAvs3Dec->frameLength*i + j] = (uint32_t)pcm_16bit[channels*j+i] & 0xffff;
				j++;
			}
		}
	}
#else

	*output_channel = channels  > 8 ? 8 : channels;
	for(i=0; i<(*output_channel); i++) {
		for(j=0; j<hAvs3Dec->frameLength; j++) {
			interval_output[hAvs3Dec->frameLength*i + j] = (uint32_t)pcm_16bit[channels*j+i] & 0xffff;
		}
	}
#endif
#if 0
	//convert to 32bit
	for(i = 0; i < channels * hAvs3Dec->frameLength; i++) {
		interval_output[i] = (uint32_t)pcm_16bit[i] & 0xffff;
	}
#endif
	return 0;
}

static int vvid_find_header(const unsigned char *buf, int size, int *offset)
{
	int ret = -1;
	int of = 0;
	
	if ((NULL == buf) || (size <= 0))
	{
		printf("Invalid parameter! buf:%p size:%d\n",buf,size);
		return -1;
	}

	while (size >= 2)
	{
		if ((buf[0] == 0xFF) && ((buf[1] & 0xF0) == 0xF0))
		{
			ret = 0;
			break;
		}
		of++;
		buf++;
		size--;
	}

	*offset = of;
	
	return ret;
}

static int vvid_decoder_run(void)
{
	unsigned int ret;
	int real_size, req_size;
	int offset = 0;
	uint32_t channels = 0;
	int16_t rewind = 0;
	uint8_t *bitstream = NULL;
	unsigned char *frm_buffer = NULL;
	uint32_t consumed = 0;
	int i,j,k,n = 0;
	//int complementary_obj_id = 0;
	int complementary_obj_exist = 0;
	int is_complementary_obj = 0;
	Avs3MetaDataHandle pavs3Metadata = NULL;

	if(NULL == mt_vvid_decoder->hAvs3Dec) {
		printf("need init vvid decoder first!\n");
		return -1;
	}

	req_size = REQUEST_PACKET_SIZE;
	while(1) {

		if(1 == mt_vvid_decoder->decode_break)
			break;

		if(consumed > 0) {
			mt_vvid_decoder->update_data(consumed);
			consumed = 0;
		}
		
		frm_buffer = (unsigned char *)mt_vvid_decoder->request_data((size_t *)&real_size, req_size);
		//printf("%s %d,req data, req size:%d, get size:%d\n",__FUNCTION__,__LINE__,req_size, real_size);
		if(real_size < 9) {//need more data
			usleep(5000);
			continue;
		}

		if (vvid_find_header(frm_buffer, real_size, &offset) < 0) {
			consumed += offset;
			continue;
		}
		
		frm_buffer += consumed;
		if(0 == mt_vvid_decoder->inited) {
			ret = Avs3ParseBsFrameHeader(mt_vvid_decoder->hAvs3Dec, frm_buffer, 1, NULL);
			if(1 != ret) {
				consumed += 1;
				continue;
			}
			Avs3InitDecoder(mt_vvid_decoder->hAvs3Dec);
			
			mt_vvid_decoder->inited = 1;
			printf("ext vvid init done, channels:%d, rate:%ld, sample num:%d, depth:%d, has sm:%d, has dm:%d!\n",
				mt_vvid_decoder->hAvs3Dec->numChansOutput,mt_vvid_decoder->hAvs3Dec->outputFs,
				mt_vvid_decoder->hAvs3Dec->frameLength,mt_vvid_decoder->hAvs3Dec->bitDepth,
				mt_vvid_decoder->avs3Metadata.hasStaticMeta,mt_vvid_decoder->avs3Metadata.hasDynamicMeta);
		}
		
		Avs3ParseBsFrameHeader(mt_vvid_decoder->hAvs3Dec, frm_buffer, 0, &mt_vvid_decoder->crcBs);
		
		rewind = 0;
		if (mt_vvid_decoder->hAvs3Dec->isMixedContent == 0) {
			rewind = 2;
		} else if (mt_vvid_decoder->hAvs3Dec->soundBedType == 0) {
			rewind = 1;
		}
		
		frm_buffer += (9-rewind);
		mt_vvid_decoder->bytesPerFrame = (int16_t)(ceil((float)mt_vvid_decoder->hAvs3Dec->bitsPerFrame / 8));
		
		if(real_size - consumed < mt_vvid_decoder->bytesPerFrame + (9-rewind)) {
			if(mt_vvid_decoder->bytesPerFrame + (9-rewind) > req_size)	
				req_size = mt_vvid_decoder->bytesPerFrame + (9-rewind);
			
			continue;//need more data
		}
		
		bitstream = mt_vvid_decoder->hAvs3Dec->hBitstream->bitstream;
		for (i = 0; i < mt_vvid_decoder->bytesPerFrame; i++) {
			bitstream[i] = frm_buffer[i];
		}
		
		Avs3Decode(mt_vvid_decoder->hAvs3Dec, pcm_output);

		pavs3Metadata = &mt_vvid_decoder->hAvs3Dec->hMetadataDec->avs3MetaData;

		//temp solution for enable song obj start
		for (n = 0; n < pavs3Metadata->avs3MetaDataDynamic.numDmChans; n++) {
			is_complementary_obj = 0;
			for(k = 0; k<pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.numOfContents; k++) {//need check numOfContents > 1, todo
				if(pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].hasNumComplementaryObjectGroup == 1) {
					for(i = 0; i<pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].numComplementaryObjectGroup; i++) {
						if(pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].numComplementaryObject[i]!=0)
							complementary_obj_exist = 1;
						
						for(j = 0; j<pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].numComplementaryObject[i]; j++) {
							if (n+1 == pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].ComplementaryObjectIdx[i][j]) {
								//is complementary obj run here.
								is_complementary_obj = 1;
								break;
							}
						}
					}
				}
			}
			
			if( complementary_obj_exist== 1 && is_complementary_obj == 0) {
				pavs3Metadata->avs3MetaDataDynamic.avs3DmL1MetaData[n].gain = 5;//unmute all channel except Complementary Object
			}
		}
		//temp solution for enable song obj end	

		for(i=0; i<32; i++) {
			if(mt_vvid_decoder->config.obj_gain[i] != 0xff && 
				pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioObjectData[i+1].audioObjectInteractionData.onOffInteract == 1)
				pavs3Metadata->avs3MetaDataDynamic.avs3DmL1MetaData[i].gain = mt_vvid_decoder->config.obj_gain[i];
		}
		
		MvByte2Byte((int8_t *)(&(mt_vvid_decoder->hAvs3Dec->hMetadataDec->avs3MetaData)),
			(int8_t *)&mt_vvid_decoder->avs3Metadata, sizeof(Avs3MetaData));

		vvid_output_pcm(mt_vvid_decoder->hAvs3Dec, interval_output, pcm_output, &channels);
		
		ResetBitstream(mt_vvid_decoder->hAvs3Dec->hBitstream);

		consumed += mt_vvid_decoder->bytesPerFrame + (9-rewind);

		mt_vvid_decoder->channels = channels;
		mt_vvid_decoder->sample_num = mt_vvid_decoder->hAvs3Dec->frameLength;
		mt_vvid_decoder->sample_rate = mt_vvid_decoder->hAvs3Dec->outputFs;
		mt_vvid_decoder->bit_depth = 32;//vvid_lib->hAvs3Dec->bitDepth;

		if(mt_vvid_decoder->hAvs3Dec->outputFs == 192000)
			mt_vvid_decoder->output((void*)&interval_output[0], (mt_vvid_decoder->sample_num>>1));
		else
			mt_vvid_decoder->output((void*)&interval_output[0], mt_vvid_decoder->sample_num);

	}

	return 0;
}

static void vvid_decoder_break(void)
{
	mt_vvid_decoder->decode_break = 1;
}

static int vvid_decoder_getinfo(AUDIO_INFO_T *pinfo)
{
	if(NULL == pinfo) {
		printf("invalid params!\n");
		return -1;
	}
	
	pinfo->channels = mt_vvid_decoder->channels;
	if(mt_vvid_decoder->sample_rate == 192000) {
		pinfo->sample_rate = mt_vvid_decoder->sample_rate>>1;
	} else {
		pinfo->sample_rate = mt_vvid_decoder->sample_rate;
	}
	pinfo->bitdepth = mt_vvid_decoder->bit_depth;
	pinfo->channelsOriginal = mt_vvid_decoder->hAvs3Dec->numChansOutput;

	return 0;
}

static int vvid_decoder_getmetainfo(AUDIO_META_INFO_T *pmetainfo)
{
	uint16_t i, j, k = 0;
	uint16_t has_interact = 0;
	Avs3MetaDataHandle	pavs3Metadata;
	
	if(NULL == pmetainfo) {
		printf("invalid params!\n");
		return -1;
	}

	pavs3Metadata = &mt_vvid_decoder->hAvs3Dec->hMetadataDec->avs3MetaData;
	memset(pmetainfo, 0, sizeof(AUDIO_META_INFO_T));
	pmetainfo->obj_num = pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.numOfObjects;

	for(i=0; i<pmetainfo->obj_num; i++) {
		pmetainfo->obj_info[i].obj_id = pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioObjectData[i].objectIdx;
		memcpy(pmetainfo->obj_info[i].obj_name, 
			pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioObjectData[i].ObjectName, 
			sizeof(pmetainfo->obj_info[i].obj_name));
		has_interact = pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioObjectData[i].hasInteract;
		if(has_interact)
			pmetainfo->obj_info[i].interact = pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioObjectData[i].audioObjectInteractionData.onOffInteract;
	}

	for(k = 0; k<pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.numOfContents; k++) {//need check numOfContents > 1, todo
		pmetainfo->complementary_object_group = pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].numComplementaryObjectGroup;
		for(i = 0; i<pmetainfo->complementary_object_group; i++) {
			pmetainfo->complementary_object[i] = pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].numComplementaryObject[i];
			for(j = 0; j<pmetainfo->complementary_object[i]; j++) {
				pmetainfo->complementary_object_id[i][j]= pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].ComplementaryObjectIdx[i][j];
			}
		}
	}

	return 0;
}

static int vvid_decoder_setpos(float x, float y, float z)
{
	if(!mt_vvid_decoder) {
		printf("decoder is not initialized!\n");
		return -1;
	}

	mt_vvid_decoder->config.pos_x = x;
	mt_vvid_decoder->config.pos_y = y;
	mt_vvid_decoder->config.pos_z = z;

	return 0;
}

static int vvid_decoder_selobj(uint16_t id, uint16_t on)
{
	uint16_t i,j,k,m = 0;
	uint16_t complementary_obj_id = 0;
	Avs3MetaDataHandle pavs3Metadata = NULL;
	
	if(!mt_vvid_decoder) {
		printf("decoder is not initialized!\n");
		return -1;
	}

	if(id > 32){
		printf("obj id is out of range!\n");
		return -1;
	}
	
	pavs3Metadata = &mt_vvid_decoder->hAvs3Dec->hMetadataDec->avs3MetaData;
	if(on == 1) {
		//check obj id is complementary obj
		uint8_t is_comp_obj = 0;
		for(k = 0; k<pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.numOfContents; k++) {//need check numOfContents > 1, todo
			if(pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].hasNumComplementaryObjectGroup == 1) {
				for(i = 0; i<pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].numComplementaryObjectGroup; i++) {
					for(j = 0; j<pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].numComplementaryObject[i]; j++) {
						if (id == pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].ComplementaryObjectIdx[i][j]) {
							//is complementary obj run here.
							is_comp_obj = 1;
							for(m=0; m<pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].numComplementaryObject[i]; m++) {
								complementary_obj_id = pavs3Metadata->avs3MetaDataStatic.avs3BasicL1.audioContentData[k].ComplementaryObjectIdx[i][m];
								if(id == complementary_obj_id) {
									//printf("set complementary obj id:%d gain to 5\n",complementary_obj_id);
									mt_vvid_decoder->config.obj_gain[complementary_obj_id-1] = 5;
								} else {
									//printf("set complementary obj id:%d gain to 0\n",complementary_obj_id);
									mt_vvid_decoder->config.obj_gain[complementary_obj_id-1] = 0;
								}
							}
							break;
						}
					}
				}
			}
		}

		if(is_comp_obj == 0) {
			//printf("set obj id:%d gain to 5\n",id);
			mt_vvid_decoder->config.obj_gain[id-1] = 5;
		}
	} else if (on == 0) {
		//printf("set obj id:%d gain to 0\n",id);
		mt_vvid_decoder->config.obj_gain[id-1] = 0;
	}

	return 0;
}


static void vvid_decoder_finalize(void)
{
#if 0//for debug
	if(fp_16bit) {
		fseek(fp_16bit, 0, SEEK_SET);
		//write_wav_header(fp_16bit, mt_vvid_decoder->hAvs3Dec->outputFs, mt_vvid_decoder->hAvs3Dec->numChansOutput, write_samples, 16);
		write_wav_header(fp_16bit, mt_vvid_decoder->hAvs3Dec->outputFs, 2, write_samples, 16);
		fclose(fp_16bit);
		fp_16bit = NULL;
	}

	if(fp_32bit) {
		fseek(fp_32bit, 0, SEEK_SET);
		write_wav_header(fp_32bit, mt_vvid_decoder->hAvs3Dec->outputFs, mt_vvid_decoder->hAvs3Dec->numChansOutput, write_samples, 32);
		fclose(fp_32bit);
		fp_32bit = NULL;
	}
#endif

	if(mt_vvid_decoder->hStreamRender) {
		DestroyRenderer(mt_vvid_decoder->hStreamRender);
		mt_vvid_decoder->hStreamRender = NULL;
	}
	
	if(mt_vvid_decoder->renderInFlt)
		free(mt_vvid_decoder->renderInFlt);
	if(mt_vvid_decoder->renderOutFlt)
		free(mt_vvid_decoder->renderOutFlt);
	if(mt_vvid_decoder->renderOutInt16)
		free(mt_vvid_decoder->renderOutInt16);

	if(1 == mt_vvid_decoder->inited)
		Avs3DecoderDestroy(mt_vvid_decoder->hAvs3Dec);
	
	if(mt_vvid_decoder)
		free(mt_vvid_decoder);
	mt_vvid_decoder = NULL;
}

ext_decoder_t ext_vvid_decoder = {
	.name			= "vvid",
	.version		= "v1.0.0",
    .initialize		= vvid_decoder_initialize,
    .finalize		= vvid_decoder_finalize,
    .stop			= vvid_decoder_break,
    .run			= vvid_decoder_run,
    .getinfo		= vvid_decoder_getinfo,
    .getmetainfo	= vvid_decoder_getmetainfo,
    .setpos			= vvid_decoder_setpos,
    .selobj			= vvid_decoder_selobj,
};

ext_decoder_t *g_ext_vvid_decoder = &ext_vvid_decoder;

