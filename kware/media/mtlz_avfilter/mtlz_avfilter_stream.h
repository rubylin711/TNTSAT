/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MTLZ_AVFILTER_STREAM_H_H__
#define __MTLZ_AVFILTER_STREAM_H_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MTAV_SECOND         (1000 * 1000)
/** AVCodec data type for avplay interface */
typedef enum {
    MTAV_CODEC_NONE       = 0x0 ,
    MTAV_CODEC_AID_AAC          ,
    MTAV_CODEC_AID_AC4          ,
    MTAV_CODEC_AID_LPCM         ,
    MTAV_CODEC_AID_DOLBY_TRUEHD ,
    MTAV_CODEC_AID_AV3A         ,
    MTAV_CODEC_VID_MPEG4        , /**< MPEG4 DIVX4 DIVX5*/
    MTAV_CODEC_VID_H264         , /**< H264*/
    MTAV_CODEC_VID_HEVC         , /**< HEVC(H265)*/
    MTAV_CODEC_VID_VC1_WVC1     , /**< VC-1 is advanced profile, Windows Media Video 9 Advanced Profile */
    MTAV_CODEC_VID_VC1_WMV3     , /**< VC1_WMV3 VC1 early edition, is smple and main profile */
    MTAV_CODEC_VID_VP8          , /**< VP8*/
    MTAV_CODEC_VID_VP9          , /**< VP9*/

    MTAV_CODEC_AID_BYPASS       , /* No need filter audio type */
    MTAV_CODEC_VID_BYPASS       , /* No need filter video type */
    MTAV_CODEC_ID_MAX           ,
} MTAV_CODEC_ID_E;

#define MTAVSF_SUCCESS           (0)
#define MTAVSF_FAILURE          (-1)
#define MTAVSF_DRM_DECRYPT_FAIL (-2)

#ifndef __LINUX__
#define restrict
#endif
typedef int  (*MTAVSF_MSG_FUNC)(void *restrict ctx, const int type, void *data);
typedef void *(*MTAVSF_MEMCP_FUNC)(void *restrict dest, const void *restrict src, size_t size);

typedef enum {
    MTAVSTREAM_FLAGS_NONE     = 0,
    MTAVSTREAM_FLAGS_KEYFRAME = 1 << 0,
} MTAVSTREAM_FLAGS;

typedef enum {
    MTAVSTREAM_MSG_NONE     = 0,
    MTAVSTREAM_MSG_CDM_KEXPIRED,
} MTAVSTREAM_MESSAGES;

typedef struct _MTAVStreamPara {
    MTAVSTREAM_FLAGS flags;
    void *cfg;
} MTAVStreamPara;

typedef struct _MTAVStreamFilterAudioInfo {
    unsigned int channels;
    unsigned int sample_rate;
} MTAVSAudioInfo;

typedef struct _MTAVStreamFilterVideoInfo {
    long long duration;
    unsigned int width;
    unsigned int height;
    unsigned int frame_rate_num;
    unsigned int frame_rate_den;
    /* The bytes of extra_data */
    unsigned int extradata_size;
    /* The extra data need in filter */
    unsigned char *extradata;
} MTAVSVideoInfo;

typedef struct _MTAVStreamFilter MTAVStreamFilter;
typedef struct _MTAVStreamCodecFilter {
    /**
     * Init the filter use the init_info,
     * which maybe differ from each other due to codec difference,
     * maybe MTAVStreamFilterVideoInfo, or MTAVStreamFilterAudioInfo, or MTAVEncryptionInitInfo
     *
     * @param filter     The instance of MTAVStreamFilter.
     * @param init_info  The initialization info for current filter.
     *
     * @return 0 when successful, or -1 on error.
     */
    int (*init)(MTAVStreamFilter *filter, void *init_info);
    /**
     * Get the pre-filtered stream info from the filter
     *
     * if we are clear stream(enc NULL) :
     *     parse part of the data and then output data bytes after filter.
     * if we are encrypted stream(enc Not NULL):
     *     decrypted the stream and the parse the part of the data
     * Caller use the output info to decide what to do next, such as allocating a/vdec buffer etc.
     *
     * @param filter     The instance of MTAVStreamFilter.
     * @param cfg        The configuration info of current data, such as encryption, MTAVStreamPara type
     * @param data       The data to be filtered
     * @param pts        The pts of data stream to be filtered
     * @param data_size  The bytes of data to be filtered
     * @param info       The outuput info for filter
     *
     * @return 0 when successful, or -1 on error.
     */
	int (*get_info)(MTAVStreamFilter *filter, MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info);
    /**
     * Filter data stream kept in filter buffer.
     *
     * @param filter          The instance of MTAVStreamFilter.
     * @param out_buf         The data buffer for keeping filtered data.
     * @param out_buf_size    The bytes of out_buf.
     *
     * @return 0 when successful, or -1 on error.
     */
    int  (*filter)(MTAVStreamFilter *filter, unsigned char *out_buf, unsigned int out_buf_size);
    /**
     * Flush filter buffer of reset filter state
     *
     * Some codec need more header info such as vp9 after seek
     *
     * @param filter          The instance of MTAVStreamFilter.
     *
     * @return 0 when successful, or -1 on error.
     */
    void (*flush)(MTAVStreamFilter *filter);
	void (*deinit)(MTAVStreamFilter *filter);
} MTAVStreamCodecFilter;

struct _MTAVStreamFilter {
    unsigned int codec_id;
    void *stream_buffer;
    long long pts;
    /* The bytes of extra_data */
    unsigned int extradata_size;
    /* The extra data need in filter */
    unsigned char *extradata;
    void *opaque;
    void *priv_data;
    MTAVStreamCodecFilter *scf;
    MTAVSF_MSG_FUNC   msg_cb;
    MTAVSF_MEMCP_FUNC memcp_cb;
};

MTAVStreamFilter *mtlz_avfilter_stream_create(unsigned int codec_id);
int mtlz_avfilter_stream_init(MTAVStreamFilter *filter, void *init_info);

int mtlz_avfilter_stream_get_info(MTAVStreamFilter *filter, MTAVStreamPara *para,
    unsigned char *data, long long pts, unsigned int data_size, void *info);
int mtlz_avfilter_stream_filter(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int out_buf_size);
int mtlz_avfilter_stream_flush(MTAVStreamFilter *filter);

void mtlz_avfilter_stream_release(MTAVStreamFilter *filter);

#if 0
#define mtlz_avfilter_stream_create            MTDrm_StreamFilterCreate
#define mtlz_avfilter_stream_init              MTDrm_StreamFilterInit
#define mtlz_avfilter_stream_get_info          MTDrm_StreamFilterGetInfo
#define mtlz_avfilter_stream_filter            MTDrm_StreamFilterFilter
#define mtlz_avfilter_stream_flush             MTDrm_StreamFilterFlush
#define mtlz_avfilter_stream_release           MTDrm_StreamFilterRelease
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTLZ_AVFILTER_STREAM_H_H__ */