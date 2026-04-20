/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mlog.h"
#include "libavformat/avformat.h"
#include "libavutil/encryption_info.h"

#ifdef __cplusplus
extern "C" {
#endif

#if MT_DES("Internal function", 1)
#if !defined(VMX_OTT_SVP) && defined(CFG_ENABLE_FFMPEG_422)
#ifndef __LINUX__
static void write_es_dump_file(PLAYBACK_INTERNAL_T *pbi,
    STREAM_TYPE_E type, const void *start_addr, unsigned int size)
{
    if (type == STREAM_TYPE_AUD) {
        return;
    }

    FILE *fp = NULL;
    if (NULL == fp) {
        // fp = fopen("E:\\dump\\dum_vp9.bin", "ab+");
    }
    if (NULL != fp) {
        fwrite(start_addr, 1, size, fp);
        fclose(fp);
        fp = NULL;
    }
}
#endif

static void inline dump_push_data(PLAYBACK_INTERNAL_T *pbi, MT_UNF_AVPLAY_BUFID_E id,
    unsigned char *addr, unsigned int size, long long pts, int eos_flag)
{
    (void) pts;
    (void) eos_flag;

#if MT_DES("Dump push Info", 0)
        STREAM_TYPE_E type = id == MT_UNF_AVPLAY_BUF_ID_ES_VID ? STREAM_TYPE_VID : STREAM_TYPE_AUD;
        const char buf[8] = {0};
        int copy_size = MIN(8, size);
        memcpy(buf, (void *) addr, copy_size);
        MLOGW("[%d] Push %s pts:%lld, size:%d data:%02x %02x %02x %02x %02x %02x %02x %02x\n",
            id, (STREAM_TYPE_AUD == id) ? "aes" : "ves", pts, size, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
        write_es_dump_file(pbi, type, addr, size);
#endif
}

static int get_decoder_buffer(PLAYBACK_INTERNAL_T *pbi, MT_HANDLE handle,
    MT_UNF_AVPLAY_BUFID_E id, unsigned int need_size, MT_UNF_STREAM_BUF_S *stream_buf)
{
    int ret;

    stream_buf->u32Size2    = 0;
    stream_buf->pu8Data2    = 0;
    stream_buf->u32PhyData2 = 0;
    ret = get_avplay_es_buf((void *)pbi, handle, id, (unsigned int) need_size, stream_buf);
    if (MT_SUCCESS != ret) {
        return MT_FAILURE;
    }

    if (stream_buf->u32Size >= need_size ||
        !stream_buf->u32Size2 || (!stream_buf->pu8Data2 && !stream_buf->u32PhyData2)) {
        stream_buf->u32Size2    = 0;
        stream_buf->pu8Data2    = 0;
        stream_buf->u32PhyData2 = 0;
    }

    return MT_SUCCESS;
}

static int get_decoder_buffer_zone(unsigned char **out_buf,
    unsigned int *out_buf_size, unsigned int encrypted, MT_UNF_STREAM_BUF_S *stream_buf)
{
    int ret = MT_FAILURE;

    *out_buf = NULL;
    *out_buf_size = 0;
    if (stream_buf->u32Size) {
        *out_buf = stream_buf->pu8Data;
        *out_buf_size = stream_buf->u32Size;
#ifdef DRM_SMP_ENABLE
        if (encrypted) {
            *out_buf = (unsigned char *) ((intptr_t) stream_buf->u32PhyData);
        }
#endif
        stream_buf->u32Size    = 0;
        stream_buf->pu8Data    = 0;
        stream_buf->u32PhyData = 0;

        ret = MT_SUCCESS;
    } else if (stream_buf->u32Size2) {
        *out_buf = (unsigned char *) ((intptr_t) stream_buf->pu8Data2);
        *out_buf_size = stream_buf->u32Size2;
#ifdef DRM_SMP_ENABLE
        if (encrypted) {
            *out_buf = (unsigned char *)(intptr_t)stream_buf->u32PhyData2;
        }
#endif
        stream_buf->u32Size2    = 0;
        stream_buf->pu8Data2    = 0;
        stream_buf->u32PhyData2 = 0;
        ret = MT_SUCCESS;
    }

    if (MT_SUCCESS == ret && (!*out_buf_size || !*out_buf)) {
        MLOGE("No available output buffer\n");
    }

    return ret;
}

static int put_decoder_buffer(MTAVFilter *s,
    MT_HANDLE hAvplay, MT_UNF_AVPLAY_BUFID_E id,
    long long pts, unsigned int push_size, MT_UNF_AVPLAY_PUTBUFEX_OPT_S *opt)
{

    int ret;

    if (ADP_FW_INVALID_PTS != pts && s->bsf &&
        pts != s->bsf->pts && MTAV_CODEC_AID_DOLBY_TRUEHD == s->bsf->codec_id) {
        pts = s->bsf->pts;
    }

    long long push_pts = (MP_NOPTS_VALUE != pts) ? pts : ADP_FW_INVALID_PTS;
    if (MP_NOPTS_VALUE == pts && MT_UNF_AVPLAY_BUF_ID_ES_AUD == id) {
        opt->u32PtsValide = MT_FALSE;
    }
    ret = MT_UNF_AVPLAY_PutBuf64(hAvplay, id, push_size, push_pts, opt);
    if (MT_SUCCESS != ret) {
        MLOGE("Put %s decoder buffer failed!\n", (MT_UNF_AVPLAY_BUF_ID_ES_AUD == id) ? "aes" : "ves");
        return MT_FAILURE;
    }

    return ret;
}

static unsigned int inline convert_audio_codec_id(unsigned int codec_id)
{
    if (aAUDIO_AAC == codec_id) {
        return MTAV_CODEC_AID_AAC;
    } else if (aAUDIO_AC4 == codec_id) {
        return MTAV_CODEC_AID_AC4;
    } else if (aAUDIO_AV3A == codec_id) {
        return MTAV_CODEC_AID_AV3A;
    } else if (aAUDIO_TRUEHD == codec_id) {
        return MTAV_CODEC_AID_DOLBY_TRUEHD;
    }

    return MTAV_CODEC_AID_BYPASS;
}

static unsigned int inline convert_video_codec_id(unsigned int codec_id)
{
    unsigned int id = MTAV_CODEC_VID_BYPASS;

    switch (codec_id) {
        case vVIDEO_MPEG4    :
            id = MTAV_CODEC_VID_MPEG4;
        break;
        case vVIDEO_H264     :
            id = MTAV_CODEC_VID_H264;
        break;
        case vVIDEO_HEVC     :
            id = MTAV_CODEC_VID_HEVC;
        break;
        case vVIDEO_VC1      :
            id = MTAV_CODEC_VID_VC1_WVC1;
        break;
        case vVIDEO_VC1SMP5  :
        case vVIDEO_VC1_WMV3 :
            id = MTAV_CODEC_VID_VC1_WMV3;
        break;
        case vVIDEO_VP8      :
            id = MTAV_CODEC_VID_VP8;
        break;
        case vVIDEO_VP9      :
            id = MTAV_CODEC_VID_VP9;
        break;
        default              :
            id = MTAV_CODEC_VID_BYPASS;
        break;
    }
    return id;
}

static void convert_enc_init_info(
    MTAVEncryptionInitInfo *dst, AVEncryptionInitInfo *src)
{
    dst->system_id = src->system_id;
    dst->system_id_size = src->system_id_size;

    dst->key_ids = src->key_ids;
    dst->num_key_ids = src->num_key_ids;
    dst->key_id_size = src->key_id_size;

    dst->header_data = src->header_data;
    dst->header_data_size = src->header_data_size;
}

static void convert_enc_info(
    MTAVEncryptionInfo *dst, AVEncryptionInfo *src)
{
    unsigned int i;

    dst->scheme = src->scheme;

    dst->crypt_byte_block = src->crypt_byte_block;
    dst->skip_byte_block  = src->skip_byte_block;

    dst->key_id           = src->key_id;
    dst->key_id_size      = src->key_id_size;

    dst->iv               = src->iv;
    dst->iv_size          = src->iv_size;

    dst->subsample_count  = src->subsample_count;
    for (i = 0; i < src->subsample_count; i++) {
        dst->subsamples.bytes_of_clear_data[i]     = src->subsamples[i].bytes_of_clear_data;
        dst->subsamples.bytes_of_protected_data[i] = src->subsamples[i].bytes_of_protected_data;
    }
}

static int push_get_info(MTAVFilter *s, AVEncryptionInfo *info,
    AVPacket *pkt, long long pts, unsigned int *buff_size)
{
    int ret = 0;

    MTAVStreamPara    *ptr = NULL;
    MTAVStreamPara    para = {0};
    MTAVEncryptionInfo enc = {0};
    unsigned char *addr = pkt->data;
    unsigned int   size = pkt->size;
    if (info) {
        if (info->subsample_count) {
            enc.subsamples.bytes_of_clear_data = av_malloc(
                info->subsample_count * sizeof(*enc.subsamples.bytes_of_clear_data));
            enc.subsamples.bytes_of_protected_data = av_malloc(
                info->subsample_count * sizeof(*enc.subsamples.bytes_of_protected_data));
            if (!(enc.subsamples.bytes_of_clear_data) ||
                !(enc.subsamples.bytes_of_protected_data)) {
                ret = -1;
                goto fail;
            }
        }

        convert_enc_info(&enc, info);
        para.cfg = (void *) &enc;
    }
    if ((MTAV_CODEC_VID_VC1_WVC1 == s->codec_id ||
         MTAV_CODEC_VID_VC1_WMV3 == s->codec_id ||
         MTAV_CODEC_VID_H264     == s->codec_id ||
         MTAV_CODEC_VID_MPEG4    == s->codec_id) &&
        (AVINDEX_KEYFRAME == (pkt->flags & AVINDEX_KEYFRAME))) {
        para.flags = MTAVSTREAM_FLAGS_KEYFRAME;
    }

    if (para.flags || para.cfg) {
        ptr = &para;
    }

    ret = mtav_filter_get_info(s, ptr, addr, pts, size, (void *) buff_size);
    if (ret < 0 || !buff_size) {
        //ret = -1;
        goto fail;
    }
fail:
    if (enc.subsamples.bytes_of_clear_data) {
        av_free(enc.subsamples.bytes_of_clear_data);
    }
    if (enc.subsamples.bytes_of_protected_data) {
        av_free(enc.subsamples.bytes_of_protected_data);
    }
    return ret;
}

static AVEncryptionInfo * push_get_enc_info(AVPacket *pkt)
{
    AVEncryptionInfo *enc = NULL;
    int side_data_size = 0;
    uint8_t *side_data =
        av_packet_get_side_data(pkt,
            AV_PKT_DATA_ENCRYPTION_INFO, &side_data_size);
    if ((side_data && side_data_size)) {
        enc = av_encryption_info_get_side_data(side_data, side_data_size);
    }
    return enc;
}

static inline int push_need_init_audio(
    MTAVSAudioInfo *dst, FILE_SEQ_AUDIO_T *src)
{
    if (dst->channels == src->channels &&
        dst->sample_rate == src->sample_rate) {
        return 0;
    }

    dst->channels = src->channels;
    dst->sample_rate = src->sample_rate;
    return 1;
}

static inline int push_need_init_video(
    MTAVSVideoInfo *dst, FILE_SEQ_VIDEO_T *src)
{
    unsigned int need_reset =
        dst->width   != src->width   ||
        dst->height  != src->height  ||
        dst->frame_rate_num != src->frame_rate.num  ||
        dst->frame_rate_den != src->frame_rate.den  ||
        dst->extradata      != src->codec_extradata ||
        dst->extradata_size != src->codec_extradata_size;

    if (need_reset) {
        dst->width     = src->width;
        dst->height    = src->height;
        dst->duration  = (long long) (src->duration * 1000000);
        dst->extradata = src->codec_extradata;
        dst->extradata_size = src->codec_extradata_size;
        dst->frame_rate_num = src->frame_rate.num;
        dst->frame_rate_den = src->frame_rate.den;
    }

    return need_reset;
}

static inline int push_need_recreate(MTAVFilter *s, unsigned int codec_id)
{
    /* Codec not change s->codec_id initialized to MTAV_CODEC_ID_MAX */
    if (s->codec_id == codec_id) {
        return 0;
    }
    /* encrypted stream has bsf no need recrate, but need init  */
    if (s->bsf && s->encrypted) {
        MLOGE("Not support encrypted stream codec change, please tell me the stream\n");
        return 0;
    }

    if (s->bsf) {
        mtav_filter_release(s);
    }
    return 1;
}

static int push_init_filter(MTAVFilter *s,
    AVEncryptionInitInfo *info)
{
    int ret = 0;

    if (!info) {
        if (!info && s->encrypted) {
            MLOGE("Initialize drm without init inforamtion error\n");
            return -1;
        }
        ret = mtav_filter_init(s, &s->init_info);
        return ret;
    }

    AVEncryptionInitInfo *curr = info;
    while (curr) {
        convert_enc_init_info(&s->init_info.encryption, curr);
        ret = mtav_filter_init(s, &s->init_info);
        if (ret >= 0) {
            return ret;
        }
        curr = curr->next;
    }

    return -1;
}

static int inline push_init_comm(MTAVFilter *s,
    AVPacket *pkt, unsigned int codec_id, AVEncryptionInitInfo **info)
{
    int size = 0;
    uint8_t *side_data =
        av_packet_get_side_data(pkt,
            AV_PKT_DATA_ENCRYPTION_INIT_INFO, &size);
    unsigned int encrypted = 0;
    if (side_data && size) {
        *info = av_encryption_init_info_get_side_data(side_data, size);
        if (!*info) {
            MLOGE("[%d] Stream has 0 size init info:%p %d\n", codec_id, side_data, size);
            mtav_filter_release(s);
            return -1;
        }
        encrypted = 1;
    }

    int ret = 0;
    ret = push_need_recreate(s, codec_id);
    if (!ret) {
        MLOGA("Not create filter id:%d %d enc:%d bsf:%p\n",
            codec_id, s->codec_id, s->encrypted, s->bsf);
        return 0;
    }
    ret = mtav_filter_create(s, codec_id, encrypted);
    if (MT_SUCCESS == ret) {
        s->bsf->opaque = s->opaque;
        s->bsf->msg_cb = s->msg_cb;
    }
    return ret;
}

static int push_init_audio(FILE_SEQ_AUDIO_T *aud,
    MTAVFilter *s, AVPacket *pkt, unsigned int codec_id)
{
    int ret = 0;
    AVEncryptionInitInfo *info = NULL;

    ret = push_init_comm(s, pkt, codec_id, &info);
    if (ret < 0) {
        goto done;
    }

    ret = push_need_init_audio(&s->init_info.codec.audio, aud);
    if (!ret) {
        ret = 0;
        goto done;
    }
    ret = push_init_filter(s, info);
    aud->is_secure = s->encrypted;
done:
    if (info) {
        av_encryption_init_info_free(info);
    }
    return ret;
}

static int push_init_video(FILE_SEQ_VIDEO_T *vid,
    MTAVFilter *s, AVPacket *pkt, unsigned int codec_id)
{
    int ret = 0;
    AVEncryptionInitInfo *info = NULL;

    ret = push_init_comm(s, pkt, codec_id, &info);
    if (ret < 0) {
        goto done;
    }

    ret = push_need_init_video(&s->init_info.codec.video, vid);
    if (!ret) {
        ret = 0;
        goto done;
    }
    ret = push_init_filter(s, info);
    vid->is_secure = s->encrypted;
done:
    if (info) {
        av_encryption_init_info_free(info);
    }
    return ret;
}

static int push_stream_common(PLAYBACK_INTERNAL_T *pbi, MTAVFilter *s, MT_HANDLE handle,
    MT_UNF_AVPLAY_BUFID_E id, long long pts, unsigned int size, unsigned int encrypted, int eos_flag)
{
    int ret = 0;
    unsigned char     *out_buf    = NULL;
    unsigned int  out_buf_size    = 0;
    unsigned int  total_push_size = 0;
    unsigned int  push_buff_used  = 0;
    unsigned int need_size        = size;
    MT_UNF_STREAM_BUF_S stream_buf = {0};
    MT_UNF_AVPLAY_PUTBUFEX_OPT_S opt = {
        .bContinue     = MT_TRUE,
        .bEndOfFrm     = MT_FALSE,
        .u32FrameFinsh = MT_FALSE,
        .u32EosFlag    = MT_FALSE,
        .u32PtsValide  = MT_TRUE,
    };
    // MT_UNF_SYNC_ATTR_S AvSyncAttr = {0};
    // (void) MT_UNF_AVPLAY_GetAttr(handle, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
    /* pts should set when the first push */
    // opt.u32PtsValide = (MT_UNF_SYNC_REF_NONE == AvSyncAttr.enSyncRef) ? MT_FALSE : MT_TRUE;

retry:
    ret = get_decoder_buffer(pbi, handle, id, need_size, &stream_buf);
    if (MT_SUCCESS != ret) {
        goto fail;
    }
    if (need_size != stream_buf.u32Size) {
        MLOGI("[%d] Get es buf, pts:%lld need:%u addr:%p %p, len:%u addr2:%p %p, len:%u\n",
            id, pts, need_size,
            stream_buf.pu8Data, stream_buf.u32PhyData, stream_buf.u32Size,
            stream_buf.pu8Data2, stream_buf.u32PhyData2, stream_buf.u32Size2);
    }
    push_buff_used = 0;
    total_push_size = 0;
filter_again:
    out_buf = NULL;
    out_buf_size = 0;
    ret = get_decoder_buffer_zone(&out_buf, &out_buf_size, encrypted, &stream_buf);
    if (MT_SUCCESS == ret) {
        ret = mtav_filter_filter(s, out_buf, out_buf_size);
        if (ret < 0) {
            MLOGE("[%d] Push stream filter fail\n", id);
            goto fail;
        }
        dump_push_data(pbi, id, out_buf, ret, pts, eos_flag);
        push_buff_used  += out_buf_size;
        total_push_size += ret;
        goto filter_again;
    }

    if (total_push_size >= need_size) {
        opt.bEndOfFrm      = MT_TRUE;
        opt.u32FrameFinsh  = MT_TRUE;
        opt.u32EosFlag     = (unsigned int) eos_flag;
    }

    ret = put_decoder_buffer(s, handle, id, pts, push_buff_used, &opt);
    if (need_size > total_push_size) {
        need_size -= total_push_size;
        pts = ADP_FW_INVALID_PTS;
        opt.u32PtsValide = MT_FALSE;
        goto retry;
    }
    ret = size;
fail:
    return ret;
}
#endif /* #if !defined(VMX_OTT_SVP) && defined(CFG_ENABLE_FFMPEG_422) */
#endif /* MT_DES("Internal function", 1) */

#if MT_DES("Internal Main function", 1)
#if !defined(VMX_OTT_SVP) && defined(CFG_ENABLE_FFMPEG_422)
static int adp_avif_push_audio(PLAYBACK_INTERNAL_T *pbi, MTAVFilter *s, void *packet,
    unsigned char *addr, unsigned int size, long long pts, int eos_flag)
{
	int ret = 0;
	if (!pbi || !s || !packet || !addr || !size) {
		return -1;
	}

    FILE_SEQ_AUDIO_T *aud = &(pbi->audio);
    unsigned int codec_id = convert_audio_codec_id(aud->codec_id);
    MLOGA("codec id:%d -> %d\n", aud->codec_id, codec_id);
    AVPacket *pkt = packet;
    ret = push_init_audio(aud, s, pkt, codec_id);
    if (ret < 0) {
        MLOGE("Push audio init fail\n");
        return -1;
    }

    AVEncryptionInfo *enc  = push_get_enc_info(pkt);
    unsigned int buff_size = 0;
    ret = push_get_info(s, enc, pkt, pts, &buff_size);
    if (enc) {
        av_encryption_info_free(enc);
    }

    if (ret < 0 || !buff_size) {
        //ret = -1;
        goto fail;
    }

    ret = push_stream_common(pbi, s, g_avplay_es_handle,
        MT_UNF_AVPLAY_BUF_ID_ES_AUD, pts, buff_size, s->encrypted, eos_flag);
fail:
    return ret;
}

static int adp_avif_push_video(PLAYBACK_INTERNAL_T *pbi, MTAVFilter *s, void *packet,
    unsigned char *addr, unsigned int size, long long pts, int eos_flag)
{
    int ret = 0;
    if (!pbi || !s || !packet || !addr || !size) {
        return -1;
    }

    FILE_SEQ_VIDEO_T *vid = &(pbi->video);
    unsigned int codec_id = convert_video_codec_id(vid->codec_id);

    MLOGA("codec id:%d -> %d\n", vid->codec_id, codec_id);
    AVPacket *pkt = packet;
    ret = push_init_video(vid, s, pkt, codec_id);
    if (ret < 0) {
        MLOGE("Push video init fail\n");
        return -1;
    }

    AVEncryptionInfo *enc  = push_get_enc_info(pkt);
    unsigned int buff_size = 0;
    ret = push_get_info(s, enc, pkt, pts, &buff_size);
    if (enc) {
        av_encryption_info_free(enc);
    }

    if (ret < 0 || !buff_size) {
        //ret = -1;
        goto fail;
    }
    ret = push_stream_common(pbi, s, g_avplay_es_handle,
        MT_UNF_AVPLAY_BUF_ID_ES_VID, pts, buff_size, s->encrypted, eos_flag);
fail:
    return ret;
}
#endif /* #if !defined(VMX_OTT_SVP) && defined(CFG_ENABLE_FFMPEG_422) */
#endif /* MT_DES("Internal function", 1) */


#if MT_DES("AV PUSH Interface", 1)
#if defined(VMX_OTT_SVP) || !defined(CFG_ENABLE_FFMPEG_422)
void drv_adp_init_avif(void *p_dev,
    void *ctx, MTAVSF_MSG_FUNC msb_cb)
{
}
void drv_adp_deinit_avif(void *p_dev)
{
}
void drv_adp_flush_avif(void *p_dev)
{
}
int drv_adp_push_audio(void *p_dev, MTAVFilter *s, void *packet,
    unsigned char *addr, unsigned int size, long long pts, int eos_flag)
{
    return 0;
}
int drv_adp_push_video(void *p_dev, MTAVFilter *s, void *packet,
    unsigned char *addr, unsigned int size, long long pts, int eos_flag)
{
    return 0;
}
#else
void drv_adp_init_avif(void *p_dev,
    void *ctx, MTAVSF_MSG_FUNC msb_cb)
{
    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *) p_dev;

    memset(&pbi->audio.mtavf, 0, sizeof(pbi->audio.mtavf));
    memset(&pbi->video.mtavf, 0, sizeof(pbi->video.mtavf));
    memset(&pbi->audio.mtavf.init_info.codec, 0xFF, sizeof(pbi->audio.mtavf.init_info.codec));
    memset(&pbi->video.mtavf.init_info.codec, 0xFF, sizeof(pbi->video.mtavf.init_info.codec));
    pbi->audio.mtavf.codec_id = MTAV_CODEC_ID_MAX;
    pbi->video.mtavf.codec_id = MTAV_CODEC_ID_MAX;
    pbi->audio.mtavf.opaque = ctx;
    pbi->video.mtavf.opaque = ctx;
    pbi->audio.mtavf.msg_cb = msb_cb;
    pbi->video.mtavf.msg_cb = msb_cb;
    pbi->audio.state = FPBI_STATE_FLUSHED;
    pbi->video.state = FPBI_STATE_FLUSHED;
    pbi->audio.segment_start_pts = MP_NOPTS_VALUE;
    pbi->video.segment_start_pts = MP_NOPTS_VALUE;
}

void drv_adp_flush_avif(void *p_dev)
{
    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *) p_dev;

    if (pbi->audio.mtavf.bsf) {
        mtav_filter_flush(&pbi->audio.mtavf);
        pbi->audio.state = FPBI_STATE_FLUSHED;
        pbi->audio.segment_start_pts = MP_NOPTS_VALUE;
    }
    if (pbi->video.mtavf.bsf) {
        mtav_filter_flush(&pbi->video.mtavf);
        pbi->video.state = FPBI_STATE_FLUSHED;
        pbi->video.segment_start_pts = MP_NOPTS_VALUE;
    }
    (void) mlzp_mutex_lock(pbi->video.mutex);
    pbi->video.pkt_cnt_in_buf = 0;
    (void) mlzp_mutex_unlock(pbi->video.mutex);
}

void drv_adp_deinit_avif(void *p_dev)
{
    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *) p_dev;

    if (pbi->video.mtavf.bsf) {
        mtav_filter_release(&pbi->video.mtavf);
        memset(&pbi->video.mtavf, 0, sizeof(pbi->video.mtavf));
    }

    if (pbi->audio.mtavf.bsf) {
        mtav_filter_release(&pbi->audio.mtavf);
        memset(&pbi->audio.mtavf, 0, sizeof(pbi->audio.mtavf));
    }
}

int drv_adp_push_audio(void *p_dev, MTAVFilter *s, void *packet,
    unsigned char *addr, unsigned int size, long long pts, int eos_flag)
{
    if (!p_dev || !s || !packet || !addr || !size) {
        MLOGW("Push audio para error\n");
        return -1;
    }

    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *) p_dev;
    if (FPBI_STATE_FLUSHED == pbi->audio.state ||
        FPBI_STATE_PLAYLIST_CHANGED == pbi->audio.state) {
        MLOGW("Push first audio %lld.%llds %d bytes after %s\n",
            pts / MTAV_SECOND, pts % MTAV_SECOND, size,
            FPBI_STATE_FLUSHED == pbi->audio.state ? "flush" : "playlist change");
        pbi->audio.state = FPBI_STATE_NONE;
        pbi->audio.segment_start_pts = pts;
    }

    if (FPBI_STATE_DECODER_STOPPED == pbi->audio.state) {
        return (int) size;
    }
    return adp_avif_push_audio(p_dev, s, packet, addr, size, pts, eos_flag);
}

int drv_adp_push_video(void *p_dev, MTAVFilter *s, void *packet,
    unsigned char *addr, unsigned int size, long long pts, int eos_flag)
{
    if (!p_dev || !s || !packet || !addr || !size) {
        MLOGW("Push video para error\n");
        return -1;
    }

    PLAYBACK_INTERNAL_T *pbi = (PLAYBACK_INTERNAL_T *) p_dev;
    if (FPBI_STATE_FLUSHED == pbi->video.state ||
        FPBI_STATE_PLAYLIST_CHANGED == pbi->video.state) {
        MLOGW("Push first video %lld.%llds %d bytes after %s\n", pts / MTAV_SECOND, pts % MTAV_SECOND, size,
            FPBI_STATE_FLUSHED == pbi->video.state ? "flush" : "playlist change");
        pbi->video.state = FPBI_STATE_NONE;
        pbi->video.segment_start_pts = pts;
    }

    int ret = adp_avif_push_video(pbi, s, packet, addr, size, pts, eos_flag);
    (void) mlzp_mutex_lock(pbi->video.mutex);
    pbi->video.pkt_cnt_in_buf++;
    (void) mlzp_mutex_unlock(pbi->video.mutex);
    return ret;
}
#endif
#endif

#ifdef __cplusplus
}
#endif

