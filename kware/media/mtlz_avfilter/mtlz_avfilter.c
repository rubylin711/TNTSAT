/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "mtlz_avfilter.h"
#include "stream_filter/stream_filter.h"
#include "platform/stream_filter_os_support.h"
#if defined(MT_DRM_SUPPORT)
#include "stream_filter/stream_filter_drm.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#if defined(MT_DRM_SUPPORT)
extern MTAVStreamCodecFilter mtav_scf_drm;

/* convert ASCII characters to uppercase. */
static inline unsigned int mtav_toupper(unsigned int c)
{
    if (c >= 'a' && c <= 'z') {
        c ^= 0x20;
    }
    return c;
}

static unsigned int convert_scheme(unsigned int scheme)
{
    unsigned int sche = scheme;
    unsigned char *ptr = (unsigned char *) &sche;
    for (int i = 0; i < sizeof(unsigned int); i++) {
        ptr[i] = (unsigned char) mtav_toupper((unsigned int) ptr[i]);
    }
    return sche;
}

static int set_drm_encryption_info(
    MTAVEncryptionInfo *enc, DMTRM_BUFFER_IN *info)
{
    info->key_index = 0;
    info->iv = enc->iv;
    /* default iv size 16 */
    info->iv = MTAVSF_MALLOC(16);
    /* Need to add '\0' to end */
    if (!info->iv) {
        return MTAVSF_FAILURE;
    }

#ifdef CONFIG_MT_MONTAGE_PLATFORM
    info->scheme = convert_scheme(enc->scheme);
#endif
    memset(info->iv, 0, 16);
    memcpy(info->iv, enc->iv, enc->iv_size);
    if (enc->subsample_count) {
        info->region_count = enc->subsample_count;

        info->clear = enc->subsamples.bytes_of_clear_data;
        info->encrypt = enc->subsamples.bytes_of_protected_data;
    }

    info->patternClear   = enc->skip_byte_block;
    info->patternEncrypt = enc->crypt_byte_block;

    return MTAVSF_SUCCESS;
}

static void release_drm_encryption_info(DMTRM_BUFFER_IN *info)
{
    if (!info || !info->iv) {
        return;
    }

    MTAVSF_FREE(info->iv);
    info->iv = NULL;
}

static int create_drm_encryption_init_info(
    MTAV_CSFDrmFilterInitInfo *drm_info, MTAVFilterInitInfo *info)
{
   unsigned int i;

    memset(drm_info, 0, sizeof(*drm_info));
    drm_info->system_id = info->encryption.system_id;
    drm_info->system_id_size = info->encryption.system_id_size;

    drm_info->info.is_pssh = 0;
    drm_info->codec.video = info->codec.video;
    for (i = 0; i < info->encryption.num_key_ids && i < MT_DRM_MAX_KEYS; i++) {
        drm_info->info.kids[i] =
            MTAVSF_MALLOC(info->encryption.key_id_size + 1);
        /* Need to add '\0' to end */
        if (!drm_info->info.kids[i]) {
            return MTAVSF_FAILURE;
        }
        drm_info->info.kids[i][info->encryption.key_id_size] = '\0';
        memcpy(drm_info->info.kids[i], info->encryption.key_ids[i], info->encryption.key_id_size) ;
    }
    drm_info->info.kid_count = i;
    drm_info->info.init_data =
        MTAVSF_MALLOC(info->encryption.header_data_size + 1);
    /* Need to add '\0' to end */
    if (!drm_info->info.init_data) {
        return MTAVSF_FAILURE;
    }
    drm_info->info.init_data[info->encryption.header_data_size] = '\0';
    memcpy(drm_info->info.init_data, info->encryption.header_data, info->encryption.header_data_size);
    drm_info->info.init_data_len = info->encryption.header_data_size;
    return MTAVSF_SUCCESS;
}

static void release_drm_encryption_init_info(MTAV_CSFDrmFilterInitInfo *drm_info)
{
    int i;
    if (drm_info->info.init_data) {
        MTAVSF_FREE(drm_info->info.init_data);
        drm_info->info.init_data = NULL;
    }

    for (i = 0; i < drm_info->info.kid_count && i < MT_DRM_MAX_KEYS; i++) {
        if (!drm_info->info.kids[i]) {
            break;
        }
        MTAVSF_FREE(drm_info->info.kids[i]);
        drm_info->info.init_data = NULL;
    }
}

#endif

static void release_stream_buffer(
    MTAVStreamFilter *filter, unsigned int encrypted)
{
    /* Use drm component manage buffer if encrypted */
    if (!filter || !filter->stream_buffer || encrypted) {
        return;
    }

    MTAVStreamBuffer *buffer = filter->stream_buffer;
    (void) mtav_stream_buffer_release(buffer);
    /* Exclude default filter case */
    if (buffer->obuffer && buffer->obuffer != buffer->ibuffer) {
        MTAVSF_FREE(buffer->obuffer);
    }
    buffer->icb          = NULL;
    buffer->ibuffer      = NULL;
    buffer->ibuffer_indx = 0;
    buffer->ibuffer_size = 0;

    buffer->ocb          = NULL;
    buffer->obuffer      = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = 0;
}

static long ott_get_scf_base(char * str)
{
	char *end;

	if(NULL != str){
		return strtol(str, &end, 16);
	}

	return 0;
}

int mtav_filter_create(MTAVFilter *mtav,
    unsigned int codec_id, unsigned int encrypted)
{
    MTAVStreamFilter *filter = NULL;

    if (!mtav) {
        MTAVSF_LOG("[MTAV] Create filter without mtavstream\n");
        return MTAVSF_FAILURE;
    }

    if (codec_id == mtav->codec_id) {
        MTAVSF_LOG("[MTAV] Not create filter with the same id:%d\n", codec_id);
        return MTAVSF_SUCCESS;
    }

    mtav->codec_id = codec_id;
    mtav->encrypted = encrypted;
    if (mtav->bsf) {
        mtlz_avfilter_stream_release(mtav->bsf);
        mtav->bsf = NULL;
    }

    filter = mtlz_avfilter_stream_create(codec_id);
    if (!filter) {
        MTAVSF_LOG("[MTAV] Create filter fail\n");
        return MTAVSF_FAILURE;
    }

    MTAVStreamBuffer *buffer =
        MTAVSF_MALLOC(sizeof(MTAVStreamBuffer));
    if (!buffer) {
        mtlz_avfilter_stream_release(filter);
        MTAVSF_LOG("[MTAV] Create filter allocate buffer fail\n");
        return MTAVSF_FAILURE;
    }

    memset(buffer, 0,  sizeof(*buffer));
    filter->stream_buffer = buffer;
    if (encrypted) {
        filter->scf = NULL;
#if defined(MT_DRM_SUPPORT)
        filter->scf = &mtav_scf_drm;
#endif
		char * env_str = getenv("OTT_SCF");
		if(NULL != env_str){
			filter->scf = (MTAVStreamCodecFilter *)ott_get_scf_base(env_str);
		}
    }
    filter->memcp_cb =
        avfilter_get_memcp_func(codec_id, encrypted);
    mtav->bsf = filter;
    if (MTAVSF_MEDIA_TYPE_AUDIO ==
        mtlz_avfilter_get_media_type(codec_id)) {
        mtav->init_info.codec.audio.channels = -1;
        mtav->init_info.codec.audio.sample_rate = -1;
    }
    MTAVSF_LOG("[MTAV] Create [%s] filter%s\n",
        mtlz_avfilter_stream_codec_name(codec_id), (encrypted && filter->scf) ? " with DRM" : " ");
    return MTAVSF_SUCCESS;
}

int mtav_filter_init(MTAVFilter *mtav, void *init_info)
{
    if (!mtav) {
        MTAVSF_LOG("[MTAV] Init filter without mtavstream\n");
        return MTAVSF_FAILURE;
    }

    MTAVStreamFilter *filter = mtav->bsf;
    if (!filter || !filter->scf) {
        MTAVSF_LOG("[MTAV] Init filter without codec handler:%p\n", filter);
        return MTAVSF_FAILURE;
    }
    /* No need init for some codec */
    if (!filter->scf->init) {
        MTAVSF_LOG("[MTAV] No need init filter\n");
        return MTAVSF_SUCCESS;
    }
    MTAVSF_LOG("[MTAV] Init [%s] filter%s\n",
        mtlz_avfilter_stream_codec_name(mtav->codec_id), (mtav->encrypted && filter->scf) ? " with DRM" : " ");
    MTAVFilterInitInfo *info = init_info;
#if defined(MT_DRM_SUPPORT)
    if ((filter->scf == &mtav_scf_drm) || (filter->scf == ott_get_scf_base(getenv("OTT_SCF")))) {
        int ret;
        MTAV_CSFDrmFilterInitInfo drm_info = {0};

        ret = create_drm_encryption_init_info(&drm_info, info);
        if (MTAVSF_SUCCESS != ret) {
            MTAVSF_LOG("[MTAV] Init Drm malloc memory fail\n");
            release_drm_encryption_init_info(&drm_info);
            return ret;
        }
        ret = mtlz_avfilter_stream_init(filter, &drm_info);
        release_drm_encryption_init_info(&drm_info);
        return ret;
    }
#endif
    return mtlz_avfilter_stream_init(filter, &info->codec);
}

int mtav_filter_get_info(MTAVFilter *mtav, MTAVStreamPara *para,
    unsigned char *data, long long pts, unsigned int data_size, void *info)
{
    int ret = MTAVSF_SUCCESS;
    if (!mtav) {
        return MTAVSF_FAILURE;
    }

    MTAVStreamFilter *filter = mtav->bsf;
    if (!filter || !filter->scf || !filter->scf->get_info || !info) {
        return MTAVSF_FAILURE;
    }
#if defined(MT_DRM_SUPPORT)
    if ((filter->scf == &mtav_scf_drm) || (filter->scf == ott_get_scf_base(getenv("OTT_SCF")))) {
        MTAVStreamPara *ptrin = NULL;
        DMTRM_BUFFER_IN bufin = {0};
        MTAVStreamPara  tpara = {0};

        if (para) {
            tpara.flags = para->flags;
            if (para->cfg) {
                ret = set_drm_encryption_info(para->cfg, &bufin);
                if (MTAVSF_SUCCESS != ret) {
                    release_drm_encryption_info(&bufin);
                    return ret;
                }
                tpara.cfg = (void *) &bufin;
            }
            ptrin = &tpara;
        }

        ret = mtlz_avfilter_stream_get_info(
            filter, ptrin, data, pts, data_size, info);
        if (ptrin && ptrin->cfg) {
            release_drm_encryption_info(ptrin->cfg);
        }
        return ret;
    }
#endif
    release_stream_buffer(filter, mtav->encrypted);
    /* output data_size 0 for some codec when push end */
    if (!data) {
        MTAVStreamBuffer *buffer = filter->stream_buffer;
        buffer->obuffer = MTAVSF_MALLOC(data_size);
        if (!buffer->obuffer) {
            MTAVSF_LOG("[MTAV] Allocate last buffer:%d fail\n", data_size);
            return MTAVSF_FAILURE;
        }
        buffer->ocb = MTAVSF_FREE;
        memset(buffer->obuffer, 0, data_size);
        buffer->obuffer_indx = 0;
        buffer->obuffer_size = data_size;
        *((unsigned int *) info) = data_size;
        MTAVSF_LOG("[MTAV] Push last padding buffer:%d\n", data_size);
        return MTAVSF_SUCCESS;
    }

    ret = mtlz_avfilter_stream_get_info(filter, para, data, pts, data_size, info);
    return ret;
}

int mtav_filter_filter(MTAVFilter *mtav,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    if (!mtav || !out_buf || !out_buf_size) {
        return MTAVSF_FAILURE;
    }

    /* Use drm component handle buffer samller case */
    if (mtav->encrypted) {
        return mtlz_avfilter_stream_filter(mtav->bsf, out_buf, out_buf_size);
    }

    MTAVStreamFilter *filter = mtav->bsf;
    MTAVStreamBuffer *buffer = filter->stream_buffer;

retry:
    if (buffer->obuffer) {
        unsigned int copy_size = MTAVSF_MIN(
            out_buf_size, buffer->obuffer_size);
        filter->memcp_cb(out_buf, &buffer->obuffer[buffer->obuffer_indx], copy_size);
        buffer->obuffer_indx += copy_size;
        buffer->obuffer_size -= copy_size;
        if (0 == buffer->obuffer_size) {
            release_stream_buffer(filter, mtav->encrypted);
        }
        return copy_size;
    }

    if (out_buf_size >= buffer->obuffer_size) {
        return mtlz_avfilter_stream_filter(mtav->bsf, out_buf, out_buf_size);
    }

    if (buffer->obuffer_size == buffer->ibuffer_size) {
        buffer->obuffer = buffer->ibuffer;
    } else {
        buffer->obuffer = MTAVSF_MALLOC(buffer->obuffer_size);
        buffer->ocb = MTAVSF_FREE;
    }

    if (!buffer->obuffer) {
        MTAVSF_LOG("[MTAV] Decoder buffer %d, need %d,and no system memory\n", out_buf_size, buffer->obuffer_size);
        return MTAVSF_FAILURE;
    }

    int ret = mtlz_avfilter_stream_filter(
        mtav->bsf, buffer->obuffer, buffer->obuffer_size);
    if (ret < 0) {
        return ret;
    }
    goto retry;
}

int mtav_filter_flush(MTAVFilter *mtav)
{
    if (!mtav) {
        return MTAVSF_FAILURE;
    }

    release_stream_buffer(mtav->bsf, mtav->encrypted);
    mtlz_avfilter_stream_flush(mtav->bsf);
    return MTAVSF_SUCCESS;
}

void mtav_filter_release(MTAVFilter *mtav)
{
    if (!mtav) {
        return;
    }
    MTAVSF_LOG("[MTAV] Destroy [%s] filter%s\n",
        mtlz_avfilter_stream_codec_name(mtav->codec_id),
        (mtav->encrypted && mtav->bsf && mtav->bsf->scf) ? " with DRM" : " ");

    MTAVStreamBuffer *buffer = NULL;
    if (mtav->bsf) {
        buffer = mtav->bsf->stream_buffer;
    }
    release_stream_buffer(mtav->bsf, mtav->encrypted);
    mtlz_avfilter_stream_release(mtav->bsf);
    if (buffer) {
        MTAVSF_FREE(buffer);
    }
    mtav->bsf      = NULL;
    mtav->codec_id = MTAV_CODEC_ID_MAX;
    mtav->encrypted = INVALID_ENCRYPTED_FLAG;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
