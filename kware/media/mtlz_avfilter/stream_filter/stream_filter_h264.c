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

static void count_out_size_or_copy_stream(
    unsigned char **out, int *out_size,
    const unsigned char *in, int in_size, int ps,
    MTAVSF_MEMCP_FUNC memcp_cb)
{
    unsigned char start_code_size = ps < 0 ? 0 : *out_size == 0 || ps ? 4 : 3;

    if (memcp_cb && out && *out) {
        (void) memcp_cb((void *)(*out + start_code_size), (void *) in, in_size);
        if (start_code_size == NALU_START_CODE_LENGTH) {
            memcp_cb(*out, start_code, NALU_START_CODE_LENGTH);
        } else if (start_code_size) {
            memcp_cb(*out, &start_code[1], 3);
        }
        *out  += start_code_size + in_size;
    }
    *out_size += start_code_size + in_size;
}

static int h264_get_sps_pps_from_extradata(
    MTAV_SCFH264 *h264, MTAVSVideoInfo *info, unsigned char *out)
{
    int total_size = 0;
    int pps_offset = 0;
    int nalu_length_bytes_nb  = 0;
    unsigned char sps_done    = 0;
    unsigned short sps_size   = 0;
    unsigned char sps_nalu_nb = 0;
    static const unsigned char nalu_header[NALU_START_CODE_LENGTH] = { 0, 0, 0, 1 };

    int extra_size = info->extradata_size;
    unsigned char *extradata =  info->extradata;
    unsigned char *extradata_end =  extradata + extra_size;

    /* retrieve length coded size */
    nalu_length_bytes_nb = (extradata[4] & 0x3) + 1;
    /* retrieve sps and pps unit(s) */
    sps_nalu_nb = extradata[5] & 0x1f; /* number of sps unit(s) */
    extradata   = extradata + 6;
    if (0 == sps_nalu_nb) {
        goto pps;
    }

    while (sps_nalu_nb--) {
        /* possible overread ok due to padding */
        sps_size  = MRD_BE16(extradata);
        extradata += 2;

        if (total_size + NALU_START_CODE_LENGTH > MAX_EXTRADATA_SIZE - sps_size) {
            MTAVSF_LOG("[H264] Too big extradata size in MP4/AVCC bitstream\n");
            return MTAVSF_FAILURE;
        }
        if ((extradata_end - extradata) < sps_size + !sps_done) {
            MTAVSF_LOG("[H264] Extradata truncated, corrupted stream or invalid MP4/AVCC bitstream\n");
            return MTAVSF_FAILURE;
        }

        memcpy(out + total_size, nalu_header, NALU_START_CODE_LENGTH);
        total_size += NALU_START_CODE_LENGTH;
        memcpy(out + total_size, extradata, sps_size);
        total_size += sps_size;
        extradata  += sps_size;
pps:
        if (!sps_nalu_nb && !sps_done++) {
            sps_nalu_nb = extradata[0]; /* number of pps unit(s) */
            extradata  += 1;
            pps_offset = total_size;
        }
    }

    if (0 == pps_offset) {
        MTAVSF_LOG("[H264] SPS NALU missing or invalid\n");
    }

    if (NULL != h264->extradata) {
        MTAVSF_FREE(h264->extradata);
    }

    h264->extradata_size = 0;
    h264->extradata = (unsigned char *)
        MTAVSF_MALLOC(total_size + MTAVSF_PADDING_SIZE);
    if (NULL == h264->extradata) {
        MTAVSF_LOG("[H264] extra sps, pps missed\n");
        return MTAVSF_FAILURE;
    }

    h264->extradata_size = total_size;
    memcpy(h264->extradata, out, total_size);
    memset(h264->extradata + total_size, 0, MTAVSF_PADDING_SIZE);

    h264->sps      = h264->extradata;
    h264->sps_size = pps_offset;
    /* use newer one */
    if (pps_offset < total_size) {
        h264->pps      = h264->extradata + pps_offset;
        h264->pps_size = total_size - pps_offset;
    } else {
        MTAVSF_LOG("[H264] extra No pps!\n");
    }

    return nalu_length_bytes_nb;
}

static int h264_avcc_annexb_filter(MTAVStreamBuffer *buffer,
    MTAV_SCFH264 *h264, unsigned char *out, MTAVSF_MEMCP_FUNC memcp_cb)
{
    const unsigned char *buf     = buffer->ibuffer;
    const unsigned char *buf_end = buf + buffer->ibuffer_size;
    /* nothing to filter */
    if (0 == h264->extradata_parsed ||
        0 == h264->nalu_length_bytes_nb) {
        if (memcp_cb) {
            memcp_cb(out, buf, buffer->ibuffer_size);
        }
        return buffer->ibuffer_size;
    }

    unsigned char new_idr  = h264->new_idr;
    unsigned char sps_seen = h264->idr_sps_seen;
    unsigned char pps_seen = h264->idr_pps_seen;

    int out_size = 0;
    do {
        unsigned int nal_size = 0;
        /* possible overread ok due to padding */
        for (int i = 0; i < h264->nalu_length_bytes_nb; i++) {
            nal_size = (nal_size << 8) | buf[i];
        }

        buf += h264->nalu_length_bytes_nb;
        /* This check requires the cast as the right side might
         * otherwise be promoted to an unsigned value. */
        if ((long long) nal_size > (long long) (buf_end - buf)) {
            MTAVSF_LOG("[H264] NAL size error\n");
            return MTAVSF_FAILURE;
        }

        if (0 == nal_size) {
            continue;
        }

        unsigned char unit_type = *buf & 0x1f;
        if (unit_type == H264_NAL_SPS) {
            sps_seen = new_idr = 1;
        } else if (unit_type == H264_NAL_PPS) {
            pps_seen = new_idr = 1;
            /* if SPS has not been seen yet, prepend the AVCC one to PPS */
            if (!sps_seen) {
                if (0 == h264->sps_size) {
                    MTAVSF_LOG("[H264] SPS not present in the stream, nor in AVCC, stream may be unreadable\n");
                } else {
                    count_out_size_or_copy_stream(&out, &out_size, h264->sps, h264->sps_size, -1, memcp_cb);
                    sps_seen = 1;
                }
            }
        }

        /* If this is a new IDR picture following an IDR picture, reset the idr flag.
         * Just check first_mb_in_slice to be 0 as this is the simplest solution.
         * This could be checking idr_pic_id instead, but would complexify the parsing. */
        if (!new_idr && unit_type == H264_NAL_IDR_SLICE && (buf[1] & 0x80))
            new_idr = 1;

        /* prepend only to the first type 5 NAL unit of an IDR picture, if no sps/pps are already present */
        if (new_idr && unit_type == H264_NAL_IDR_SLICE && !sps_seen && !pps_seen) {
            if (h264->extradata)
                count_out_size_or_copy_stream(&out, &out_size, h264->extradata,
                              h264->extradata_size, -1, memcp_cb);
            new_idr = 0;
        /* if only SPS has been seen, also insert PPS */
        } else if (new_idr && unit_type == H264_NAL_IDR_SLICE && sps_seen && !pps_seen) {
            if (!h264->pps_size) {
                MTAVSF_LOG("[H264] PPS not present in the stream, nor in AVCC, stream may be unreadable\n");
            } else {
                count_out_size_or_copy_stream(&out, &out_size, h264->pps, h264->pps_size, -1, memcp_cb);
            }
        }

        count_out_size_or_copy_stream(&out, &out_size, buf, nal_size,
            unit_type == H264_NAL_SPS || unit_type == H264_NAL_PPS, memcp_cb);
        if (0 == new_idr && unit_type == H264_NAL_SLICE) {
            new_idr  = 1;
            sps_seen = 0;
            pps_seen = 0;
        }

        buf += nal_size;
    } while (buf < buf_end);

    if (out) {
        h264->new_idr      = new_idr;
        h264->idr_sps_seen = sps_seen;
        h264->idr_pps_seen = pps_seen;
    }

    return out_size;
}

static int h264_init(MTAV_SCFH264 *h264, MTAVSVideoInfo *info)
{
    if (!h264 || !info) {
        return MTAVSF_FAILURE;
    }

    if (NULL != h264->extradata) {
        MTAVSF_FREE(h264->extradata);
        memset((void *)h264, 0, sizeof(*h264));
    }

    if (!info->extradata || !info->extradata_size) {
        /* Annex b case, checked by init */
        h264->is_annex_b = 1;
        h264->send_extra_annexb_header = 0;
        return MTAVSF_SUCCESS;
    }

    int extradata_size = info->extradata_size;
    unsigned char *extradata =  info->extradata;
    /* retrieve sps and pps NAL units from extradata */
    if (0 == extradata_size  || NULL == extradata         ||
        (extradata_size >= 3 && MRD_BE24(extradata) == 1) ||
        (extradata_size >= 4 && MRD_BE32(extradata) == 1)) {
        MTAVSF_LOG("[H264] Extra data input looks like Annex B already\n");
        /* Some clip such as RTP sever may transmit annex b video, but annex b sps and pps by extra data */

        h264->extradata_size = 0;
        h264->extradata = (unsigned char *)
            MTAVSF_MALLOC(extradata_size + MTAVSF_PADDING_SIZE);
        if (!h264->extradata) {
            return MTAVSF_FAILURE;
        }
        memcpy(h264->extradata, extradata, extradata_size);
        h264->extradata_size = extradata_size;
        h264->is_annex_b = 1;
        h264->extradata_parsed = 1;
        h264->send_extra_annexb_header = 1;
        return MTAVSF_SUCCESS;
    }

    if (extradata_size < 7) {
        MTAVSF_LOG("[H264] Invalid extradata size: %d\n", extradata_size);
        return MTAVSF_FAILURE;
    }

    unsigned char buffer[MAX_EXTRADATA_SIZE] = {0};
    int ret = h264_get_sps_pps_from_extradata(h264, info, buffer);
    if (ret < 0) {
        MTAVSF_FREE(h264);
        return MTAVSF_FAILURE;
    }

    h264->new_idr          = 1;
    h264->idr_sps_seen     = 0;
    h264->idr_pps_seen     = 0;
    h264->extradata_parsed = 1;
    h264->nalu_length_bytes_nb = ret;
    h264->is_annex_b_init  = h264->is_annex_b;
    return MTAVSF_SUCCESS;
}

static void h264_deinit(MTAV_SCFH264 *h264)
{
    if (!h264) {
        return;
    }

    if (NULL != h264->extradata) {
        MTAVSF_FREE(h264->extradata);
        h264->extradata = NULL;
        h264->extradata_size = 0;
    }
    memset(h264, 0, sizeof(*h264));
    MTAVSF_FREE(h264);
}
#endif

#if MT_DES("External API Definition", 1)
static int scf_h264_init(MTAVStreamFilter *filter, void *init_info)
{
    if (!init_info) {
        return MTAVSF_FAILURE;
    }

    MTAV_SCFH264 *h264 = (MTAV_SCFH264 *) filter->priv_data;
    if (!h264) {
        h264 = MTAVSF_MALLOC(sizeof(MTAV_SCFH264));
        if (!h264) {
            return MTAVSF_FAILURE;
        }
        memset((void *)h264, 0, sizeof(*h264));
        filter->priv_data = h264;
    }

    MTAVSVideoInfo *info = init_info;
    if (h264_init(h264, info) < 0) {
        h264_deinit(h264);
        filter->priv_data = NULL;
        return MTAVSF_FAILURE;
    }

    return MTAVSF_SUCCESS;
}

static int scf_h264_get_info(MTAVStreamFilter *filter,
    MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info)
{
    int ret = 0;
    (void) pts;
	MTAV_SCFH264 *h264 = filter->priv_data;
    if (!h264) {
        return MTAVSF_FAILURE;
    }

    MTAVStreamBuffer *buffer = filter->stream_buffer;
    buffer->ibuffer = data;
    buffer->ibuffer_indx = 0;
    buffer->ibuffer_size = data_size;

    h264->is_annex_b = h264->is_annex_b_init;
    h264->keyframe = (para && (para->flags & MTAVSTREAM_FLAGS_KEYFRAME)) ? 1 : 0;
    /* Check annex b, nalu_length_bytes_nb can be 1,2,4 */
    if (!h264->is_annex_b /*&& h264->nalu_length_bytes_nb == 4 */) {
        /* according to ffmpeg h264dec.c decode_nal_units, already annexb */
        if (data_size > 8 && MRD_BE32(data) == 1 && MRD_BE32(data + 5) > (unsigned) data_size) {
            h264->is_annex_b = 1;
        }
    }

    if (h264->is_annex_b) {
        ret = data_size;
        if (h264->keyframe && h264->send_extra_annexb_header) {
            ret += h264->extradata_size;
        }
        goto finish;
    }

    ret = h264_avcc_annexb_filter(buffer, h264, NULL, NULL);
    if (ret < 0) {
        return ret;
    }

finish:
    buffer->obuffer = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = ret;
    *((unsigned int *) info) = buffer->obuffer_size;

    return MTAVSF_SUCCESS;
}

static int scf_h264_filter(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    MTAV_SCFH264 *h264 = filter->priv_data;
    if (!h264) {
        return MTAVSF_FAILURE;
    }

    unsigned char *obuffer = out_buf;
    unsigned int obuffer_size = out_buf_size;
    MTAVStreamBuffer *buffer = filter->stream_buffer;
    if (obuffer_size < buffer->obuffer_size) {
        MTAVSF_LOG("[H264] Not support output partion of annex b data, Please provide integral buffer\n");
        return MTAVSF_FAILURE;
    }

    if (h264->is_annex_b) {
        unsigned int total_push_size = 0;
        if (h264->keyframe && h264->send_extra_annexb_header) {
            filter->memcp_cb(obuffer, h264->extradata, h264->extradata_size);
            obuffer        += h264->extradata_size;
            obuffer_size   -= h264->extradata_size;
            total_push_size = h264->extradata_size;
        }
        unsigned int copy_size =
            MTAVSF_MIN(buffer->ibuffer_size, out_buf_size);
        filter->memcp_cb(obuffer, buffer->ibuffer, copy_size);
        total_push_size += copy_size;
        return total_push_size;
    }
    return h264_avcc_annexb_filter(buffer, h264, obuffer, filter->memcp_cb);
}

static void scf_h264_flush(MTAVStreamFilter *filter)
{
    MTAV_SCFH264 *h264 = filter->priv_data;

    if (!h264) {
        return;
    }

    h264->idr_sps_seen = 0;
    h264->idr_pps_seen = 0;
    h264->new_idr      = h264->extradata_parsed;
}

static void scf_h264_deinit(MTAVStreamFilter *filter)
{
    h264_deinit(filter->priv_data);

    filter->priv_data = NULL;
}

#if MT_DES("External API-CHINADRM Definition", 1)
/* unregistered user data */
#define H264_SEI_TYPE_USER_DATA_UNREGISTERED 5

static int parse_h264_sei(unsigned char *data,
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
        if (H264_SEI_TYPE_USER_DATA_UNREGISTERED == type) {
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

static int chinadrm_parse_h264(
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

        if (-1 != sei_nal_offset) {
            (void) parse_h264_sei(data + sei_nal_offset, offset - sei_nal_offset, enc);
            sei_nal_offset = -1;
        }
        offset += 3;
        unsigned int nal_type = (unsigned int) (data[offset++] & 0x1F);
        if (H264_NAL_SEI       != nal_type &&
            H264_NAL_SLICE     != nal_type &&
            H264_NAL_IDR_SLICE != nal_type) {
            continue;
        }
        if (enc->encryption_flag &&
            (H264_NAL_SLICE     == nal_type ||
             H264_NAL_IDR_SLICE == nal_type) &&
             0 == enc->bytes_of_clear_data) {
             offset += mtav_scf_chinadrm_check_conflicts(
                &data[offset - NALU_START_CODE_LENGTH],
                31 + NALU_START_CODE_LENGTH, size - offset + NALU_START_CODE_LENGTH);
             offset += 31; /* unencrypted leader */
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
        sei_nal_offset = offset;
    }
    if (-1 != sei_nal_offset) {
        (void) parse_h264_sei(data + sei_nal_offset, offset - sei_nal_offset, enc);
    }

    return MTAVSF_SUCCESS;
}

static int scf_h264_chinadrm_get_info(MTAVStreamFilter *filter,
    MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info)
{
	MTAV_SCFH264 *h264 = filter->priv_data;
    if (!h264) {
        return MTAVSF_FAILURE;
    }

    unsigned int outsize = 0;
    chinadrm_encryption_data_info *data_enc_info = info;
    int ret = scf_h264_get_info(filter, para, data, pts, data_size, (void *) (&outsize));
    if (MTAVSF_SUCCESS != ret) {
        return ret;
    }

    if (!(h264->is_annex_b)) {
        data_enc_info->output_bytes = outsize;
        return MTAVSF_SUCCESS;
    }

    ret = chinadrm_parse_h264(data, data_size, data_enc_info);
    data_enc_info->output_bytes = data_size;
    return ret;
}

MTAVStreamCodecFilter mtav_scf_h264_chinadrm = {
	.init     = scf_h264_init,
	.get_info = scf_h264_chinadrm_get_info,
	.filter   = scf_h264_filter,
	.flush    = scf_h264_flush,
	.deinit   = scf_h264_deinit,
};
#endif

MTAVStreamCodecFilter mtav_scf_h264 = {
	.init     = scf_h264_init,
	.get_info = scf_h264_get_info,
	.filter   = scf_h264_filter,
	.flush    = scf_h264_flush,
	.deinit   = scf_h264_deinit,
};

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
