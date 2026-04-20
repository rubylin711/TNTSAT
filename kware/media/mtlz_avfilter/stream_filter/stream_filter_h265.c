/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "stdint.h"
#include "mtlz_avfilter_stream.h"
#include "stream_filter.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#if MT_DES("Internal Function", 1)
static const unsigned char start_code[] = { 0, 0, 0, 1 };
static inline int get_remain_buf_size(
    const unsigned char *buf, const unsigned char *buf_end)
{
    int ret = (int) ((intptr_t) buf_end - (intptr_t) buf);
    return ret;
}

static inline void set_if_not_get_nal(int *has, unsigned int nalu_type, unsigned int target)
{
    if (!*has && nalu_type == target) {
        *has = 1;
    }
}

static void count_out_size_or_copy_stream(
    unsigned char **out, int *out_size,
    const unsigned char *in, int in_size, int flag, MTAVSF_MEMCP_FUNC memcp_cb)
{
    unsigned char start_code_size = flag ? NALU_START_CODE_LENGTH : 0;

    if (memcp_cb && out && *out) {
        (void) memcp_cb((void *)(*out + start_code_size), (void *) in, in_size);
        if (NALU_START_CODE_LENGTH == start_code_size) {
            memcp_cb(*out, start_code, NALU_START_CODE_LENGTH);
        }
        *out += start_code_size + in_size;
    }
    *out_size += start_code_size + in_size;
}

/* parse HVCC */
static int hevc_get_vps_sps_from_extradata(
    MTAV_SCFHEVC *h265, MTAVSVideoInfo *info, unsigned char *out)
{
    int i, j, num_arrays, nalu_length_bytes_nb;

    int new_extradata_size   = 0;
    unsigned char *extradata = info->extradata + 21;

    nalu_length_bytes_nb = ((*extradata++) & 3) + 1;
    num_arrays = *extradata++;
    for (i = 0; i < num_arrays; i++) {
        int type = (*extradata++) & 0x3f;
        int cnt  = MRD_BE16(extradata);

        extradata += 2;
        if (!(type == HEVC_NAL_VPS || type == HEVC_NAL_SPS        ||
              type == HEVC_NAL_PPS || type == HEVC_NAL_SEI_PREFIX ||
              type == HEVC_NAL_SEI_SUFFIX)) {
            MTAVSF_LOG("[H265] Invalid NAL unit type in extradata: %d\n", type);
            return MTAVSF_FAILURE;
        }

        for (j = 0; j < cnt; j++) {
            int nalu_len = MRD_BE16(extradata);

            extradata += 2;
            if (nalu_len + NALU_START_CODE_LENGTH > MAX_EXTRADATA_SIZE - new_extradata_size) {
                MTAVSF_LOG("[H265] Too big extra Nal len:%d\n", nalu_len);
                if (new_extradata_size) {
                    h265->nalu_length_bytes_nb = nalu_length_bytes_nb;
                }
                goto exit;
            }

            MWR_BE32((out + new_extradata_size), 1);
            new_extradata_size += NALU_START_CODE_LENGTH;

            memcpy((out + new_extradata_size), extradata, nalu_len);
            extradata          += nalu_len;
            new_extradata_size += nalu_len;
        }
    }

    if (NULL != h265->extradata) {
        MTAVSF_FREE(h265->extradata);
        h265->extradata = NULL;
    }

    h265->nalu_length_bytes_nb = nalu_length_bytes_nb;
    if (0 == new_extradata_size) {
        MTAVSF_LOG("[H265] No parameter sets in the extradata\n");
        /* return success according to Hevc_mp4toannexb_bsf.c:hevc_extradata_to_annexb */
        return MTAVSF_SUCCESS;
    }
exit:
    h265->extradata_size = 0;
    h265->extradata = (unsigned char *)
        MTAVSF_MALLOC(new_extradata_size + MTAVSF_PADDING_SIZE);
    if (NULL == h265->extradata) {
        MTAVSF_LOG("[H265] extra sps, pps missed\n");
        return MTAVSF_FAILURE;
    }

    h265->extradata_size = new_extradata_size;
    memcpy(h265->extradata, out, new_extradata_size);
    memset(h265->extradata + new_extradata_size, 0, MTAVSF_PADDING_SIZE);

    return MTAVSF_SUCCESS;
}

static int hevc_hvcc_annexb_filter(MTAVStreamBuffer *buffer,
    MTAV_SCFHEVC *h265,  unsigned char *out, MTAVSF_MEMCP_FUNC memcp_cb)
{
    int out_size = 0;
    int got_irap = 0;
    int has_vps, has_sps, has_pps;
    const unsigned char *buf     = buffer->ibuffer;
    const unsigned char *buf_end = buffer->ibuffer + buffer->ibuffer_size;

    if (!h265->extradata_parsed) {
        return MTAVSF_FAILURE;
    }

    has_vps = has_sps = has_pps = 0;
    while (get_remain_buf_size(buf, buf_end) > 0) {
        int nalu_type;
        int is_irap, add_extradata, extra_size;
        unsigned int nalu_size = 0;

        if (get_remain_buf_size(buf, buf_end) < (int) h265->nalu_length_bytes_nb) {
            return MTAVSF_FAILURE;
        }
        for (int i = 0; i < h265->nalu_length_bytes_nb; i++) {
            nalu_size = (nalu_size << 8) | buf[i];
        }

        buf += h265->nalu_length_bytes_nb;
        if (nalu_size < 2 ||
            nalu_size > (unsigned int) get_remain_buf_size(buf, buf_end)) {
            return MTAVSF_FAILURE;
        }
        /* Nal type kept in stream */
        nalu_type = ((*buf) >> 1) & 0x3f;
        set_if_not_get_nal(&has_vps, nalu_type, HEVC_NAL_VPS);
        set_if_not_get_nal(&has_sps, nalu_type, HEVC_NAL_SPS);
        set_if_not_get_nal(&has_pps, nalu_type, HEVC_NAL_PPS);
        /* prepend extradata to IRAP frames */
        is_irap       = nalu_type >= HEVC_NAL_BLA_W_LP && nalu_type <= HEVC_NAL_IRAP_VCL23;
        /* !(has_vps && has_sps && has_pps) use local config instead of global one */
        add_extradata = is_irap && !got_irap && !(has_vps && has_sps && has_pps);
        extra_size    = add_extradata * h265->extradata_size;
        got_irap     |= is_irap;

        if (MAX_FRAME_SIZE - NALU_START_CODE_LENGTH < nalu_size + extra_size) {
            return MTAVSF_FAILURE;
        }

        if (extra_size > 0) {
            count_out_size_or_copy_stream(&out, &out_size,
                h265->extradata, extra_size, 0, memcp_cb);
        }

        count_out_size_or_copy_stream(&out, &out_size, buf, nalu_size, 1, memcp_cb);
        buf += nalu_size;
    }
    return out_size;
}

static int h265_init(MTAV_SCFHEVC *h265, MTAVSVideoInfo *info)
{
    if (!h265 || !info) {
        return MTAVSF_FAILURE;
    }

    if (NULL != h265->extradata) {
        MTAVSF_FREE(h265->extradata);
        memset((void *)h265, 0, sizeof(*h265));
    }

    h265->is_annex_b = 0;
    if (!info->extradata || !info->extradata_size) {
        h265->is_annex_b = 1;
        return MTAVSF_SUCCESS;
    }

    int extra_size = info->extradata_size;
    unsigned char *extradata =  info->extradata;

    /* It seems the extradata is encoded as hvcC format.
     * Temporarily, we support configurationVersion==0 until 14496-15 3rd
     * is finalized. When finalized, configurationVersion will be 1 and we
     * can recognize hvcC by checking if avctx->extradata[0]==1 or not.
     * according to ffmpeg h265dec.c ff_hevc_decode_extradata, not annexb
     */
    if (extra_size > 3 && (extradata[0] || extradata[1] || extradata[2] > 1)) {
        h265->is_annex_b = 0;
     /* retrieve sps and pps NAL units from extradata */
    } else if (extra_size  < MIN_HEVCC_LENGTH  || !extradata ||
        (MRD_BE24(extradata) == 1)             ||
        (MRD_BE32(extradata) == 1)) {
        MTAVSF_LOG("[H265] Extra data input looks like Annex B already\n");
        h265->is_annex_b = 1;
        return MTAVSF_SUCCESS;
    }

    unsigned char buffer[MAX_EXTRADATA_SIZE] = {0};
    int ret = hevc_get_vps_sps_from_extradata(h265, info, buffer);
    if (ret < 0) {
        return MTAVSF_FAILURE;
    }

    h265->extradata_parsed = 1;
    return MTAVSF_SUCCESS;
}

static void h265_deinit(MTAV_SCFHEVC *h265)
{
    if (!h265) {
        return;
    }

    if (NULL != h265->extradata) {
        MTAVSF_FREE(h265->extradata);
        h265->extradata = NULL;
        h265->extradata_size = 0;
    }
    memset(h265, 0, sizeof(*h265));
    MTAVSF_FREE(h265);
}

#endif

#if MT_DES("External API Definition", 1)
static int scf_h265_init(MTAVStreamFilter *filter, void *init_info)
{
    if (!init_info) {
        return MTAVSF_FAILURE;
    }

    MTAV_SCFHEVC *h265 = (MTAV_SCFHEVC *) filter->priv_data;
    if (!h265) {
        h265 = MTAVSF_MALLOC(sizeof(MTAV_SCFHEVC));
        if (!h265) {
            return MTAVSF_FAILURE;
        }
        memset((void *)h265, 0, sizeof(*h265));
        filter->priv_data = h265;
    }

    MTAVSVideoInfo *info = init_info;
    if (h265_init(h265, info) < 0) {
        h265_deinit(h265);
        filter->priv_data = NULL;
        return MTAVSF_FAILURE;
    }
    h265->is_annex_b_init = h265->is_annex_b;
    return MTAVSF_SUCCESS;
}

static int scf_h265_get_info(MTAVStreamFilter *filter,
    MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info)
{
    int ret = 0;
    (void) para;
    (void) pts;
	MTAV_SCFHEVC *h265 = filter->priv_data;
    if (!h265) {
        return MTAVSF_FAILURE;
    }

    MTAVStreamBuffer *buffer = filter->stream_buffer;
    buffer->ibuffer = data;
    buffer->ibuffer_indx = 0;
    buffer->ibuffer_size = data_size;
    /* Not Check annex b accroding stream */
    ret = data_size;
    h265->is_annex_b = h265->is_annex_b_init;
    /* Check annex b */
    if (!h265->is_annex_b && h265->nalu_length_bytes_nb == 4) {
        /* according to ffmpeg h265dec.c decode_nal_units, already annexb */
        if (data_size > 8 && MRD_BE32(data) == 1 && MRD_BE32(data + 5) > (unsigned) data_size) {
            h265->is_annex_b = 1;
        }
    }

    if (!h265->is_annex_b) {
        ret = hevc_hvcc_annexb_filter(buffer, h265, NULL, NULL);
        if (ret < 0) {
            return ret;
        }
    }

    buffer->obuffer = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = ret;
    *((unsigned int *) info) = buffer->obuffer_size;

    return MTAVSF_SUCCESS;
}

static int scf_h265_filter(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    MTAV_SCFHEVC *h265 = filter->priv_data;
    if (!h265) {
        return MTAVSF_FAILURE;
    }

    unsigned char *obuffer = out_buf;
    MTAVStreamBuffer *buffer = filter->stream_buffer;
    if (out_buf_size < buffer->obuffer_size) {
        MTAVSF_LOG("[H265] Not support output partion of annex b data, Please provide integral buffer\n");
        return MTAVSF_FAILURE;
    }
    if (h265->is_annex_b) {
        unsigned int copy_size =
            MTAVSF_MIN(buffer->ibuffer_size, out_buf_size);
		filter->memcp_cb(out_buf, buffer->ibuffer, copy_size);
		return copy_size;
    }

    return hevc_hvcc_annexb_filter(buffer, h265, obuffer, filter->memcp_cb);
}

static void scf_h265_flush(MTAVStreamFilter *filter)
{
    MTAV_SCFHEVC *h265 = filter->priv_data;

    if (!h265) {
        return;
    }
}

static void scf_h265_deinit(MTAVStreamFilter *filter)
{
    h265_deinit(filter->priv_data);

    filter->priv_data = NULL;
}
#if MT_DES("External API-CHINADRM Definition", 1)
/* unregistered user data */
#define HEVC_SEI_TYPE_USER_DATA_UNREGISTERED  5

static int parse_h265_sei(unsigned char *data,
    int data_size, chinadrm_encryption_data_info *enc)
{
    int offset = 0;
    while (offset + 2 < data_size) {
        int type = 0;
        do {
            if (offset + 1 > data_size) {
                goto finish;
            }
            type += data[offset];
        } while (0xFF == data[offset++]);

        int size = 0;
        do {
            if (offset + 1 > data_size) {
                goto finish;
            }
            size += data[offset];
        } while (0xFF == data[offset++]);

        if (offset + size > data_size) {
            MTAVSF_LOG("SEI type %d size %d bytes truncated at %d\n", type, size, offset);
            goto finish;
        }
        int ret = 0;
        if (HEVC_SEI_TYPE_USER_DATA_UNREGISTERED == type) {
            ret = mtav_scf_decode_h2645_unregistered_user_data(data + offset, size, enc);
        }
        offset += size;
        if (ret < 0) {
            break;
        }
    }
finish:
    return offset;
}

static int chinadrm_parse_h265(
    unsigned char *data,
    int size, chinadrm_encryption_data_info *enc)
{
    int offset = 0;
    int sei_nal_offset = -1;
    memset(enc, 0, sizeof(*enc));
    while (offset + NALU_START_CODE_LENGTH < size) {
        if (MRD_BE24((&data[offset])) != 1) {
            offset++;
            continue;
        }

        if (enc->encryption_flag &&
            0 != enc->bytes_of_clear_data) {
            enc->bytes_of_encrypted_data = offset - enc->bytes_of_clear_data;
        }

        if (-1 != sei_nal_offset) {
            (void) parse_h265_sei(data + sei_nal_offset, offset - sei_nal_offset, enc);
            sei_nal_offset = -1;
        }
        offset += 3;
        unsigned int nal_type = (unsigned int) ((data[offset++] >> 1) & 0x3F);
        if (HEVC_NAL_RSV_VCL31  < nal_type &&
            HEVC_NAL_SEI_PREFIX != nal_type) {
            continue;
        }
        /* encrypt 0 ~ 30 nal unit */
        if (enc->encryption_flag &&
            (HEVC_NAL_TRAIL_N   <= nal_type  &&
             HEVC_NAL_RSV_VCL31 >= nal_type) &&
             0 == enc->bytes_of_clear_data) {
            offset += mtav_scf_chinadrm_check_conflicts(
                &data[offset - NALU_START_CODE_LENGTH],
                64 + NALU_START_CODE_LENGTH, size - offset + NALU_START_CODE_LENGTH);
            offset += 64; /* unencrypted leader */
            /* protected block > 16 bytes */
            if (size - offset > 16) {
                 enc->bytes_of_clear_data = offset;
                 enc->bytes_of_encrypted_data = size - enc->bytes_of_clear_data;
            } else {
                enc->bytes_of_clear_data = size;
                enc->bytes_of_encrypted_data = 0;
            }
            /* unencrypted_trailer 1-16 bytes */
            break;
        }
        /* hevc nal unit header 2 bytes */
        sei_nal_offset = ++offset;
    }
    if (-1 != sei_nal_offset) {
        (void) parse_h265_sei(data + sei_nal_offset, offset - sei_nal_offset, enc);
    }

    return MTAVSF_SUCCESS;
}

static int scf_h265_chinadrm_get_info(MTAVStreamFilter *filter,
    MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info)
{
	MTAV_SCFHEVC *h265 = filter->priv_data;
    if (!h265) {
        return MTAVSF_FAILURE;
    }

    unsigned int outsize = 0;
    chinadrm_encryption_data_info *data_enc_info = info;
    int ret = scf_h265_get_info(filter, para, data, pts, data_size, (void *) (&outsize));
    if (MTAVSF_SUCCESS != ret) {
        return ret;
    }

    if (!(h265->is_annex_b)) {
        data_enc_info->output_bytes = outsize;
        return MTAVSF_SUCCESS;
    }

    ret = chinadrm_parse_h265(data, data_size, data_enc_info);
    data_enc_info->output_bytes = data_size;
    return ret;
}

MTAVStreamCodecFilter mtav_scf_h265_chinadrm = {
    .init     = scf_h265_init,
    .get_info = scf_h265_chinadrm_get_info,
    .filter   = scf_h265_filter,
    .flush    = scf_h265_flush,
    .deinit   = scf_h265_deinit,
};
#endif

MTAVStreamCodecFilter mtav_scf_h265 = {
    .init     = scf_h265_init,
    .get_info = scf_h265_get_info,
    .filter   = scf_h265_filter,
    .flush    = scf_h265_flush,
    .deinit   = scf_h265_deinit,
};
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
