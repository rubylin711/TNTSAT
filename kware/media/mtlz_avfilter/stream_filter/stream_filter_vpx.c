/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mtlz_avfilter.h"
#include "stream_filter.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


static int parse_vp9_superframe_info(const unsigned char *in_data,
    const unsigned int in_size, SCFVp9SuperframeInfo *info)
{
    int i, j, marker;
    const unsigned char *data = in_data;

    info->superframe_num = 1;
    info->size[0] = in_size;
    marker = data[in_size - 1];
    /* 1. parsing the final byte of the chunk and
     * checking that the superframe_marker equals 0b110
     */
    if ((marker & 0xe0) != 0xc0) {
        return MTAVSF_SUCCESS;
    }

    int bytes_per_framesize  = 1 + ((marker >> 3) & 0x3);
    int frames_in_superframe = 1 + (marker & 0x7);
    /* 2. setting the total size of the superframe_index equal to 2 + NumFrames * SzBytes */
    int superframe_index_size = 2 + frames_in_superframe * bytes_per_framesize;
    int first_superframe_header_pos = in_size - superframe_index_size;
    /* 3. checking that the first byte of the superframe_index matches the final byte */
    if ((int) in_size < superframe_index_size ||
        data[first_superframe_header_pos] != marker) {
        return MTAVSF_SUCCESS;
    }

    long long total_size = 0;
    data += first_superframe_header_pos + 1;
    for (i = 0; i < frames_in_superframe; i++) {
        unsigned int frame_size = 0;
        for (j = 0; j < bytes_per_framesize; j++) {
            frame_size |= *data++ << (j * 8);
        }

        total_size += (long long) frame_size;
        if (total_size < 0 || total_size > in_size - superframe_index_size) {
            MTAVSF_LOG("Invalid frame size in superframe: %d\n", frame_size);
            return MTAVSF_FAILURE;
        }
        info->size[i] = (int) frame_size;
    }

    //MTAVSF_LOG("Vp9 Superframe:%d, header size %d\n", frames_in_superframe, superframe_index_size);
    info->superframe_num = frames_in_superframe;
    return MTAVSF_SUCCESS;
}

static void init_file_header(MTAV_SCFVPX *vpx, unsigned int codec_id, unsigned char *buff)
{
    unsigned char *ivf_hdr  = buff + VPX_START_CODE_SIZE;
    int total_frames        = 0;

   // vpx_correction_fps(&frame_rate);
    memcpy(buff, MTAVSF_FW_FILE_HEADER, VPX_START_CODE_SIZE);
    if (vpx->vinfo.frame_rate_den > 0 && vpx->vinfo.frame_rate_num > 0) {
        /* keep 1/1000 precision */
        long long quotient =
            (long long) vpx->vinfo.frame_rate_num * 1000 / vpx->vinfo.frame_rate_den;
        /* rounding */
        quotient = (quotient + 500) / 1000;

        total_frames = (int)((vpx->vinfo.duration * quotient) / 1000);
    }

    MWR_LE32(ivf_hdr, MK_TAG32('D', 'K', 'I', 'F'));
    ivf_hdr += 4;
    MWR_LE16(ivf_hdr, 0);  // version
    ivf_hdr += 2;
    MWR_LE16(ivf_hdr, VPX_FILE_HEADER_SIZE - VPX_START_CODE_SIZE); // header length
    ivf_hdr += 2;
    if (codec_id == MTAV_CODEC_VID_VP8) {
        MWR_LE32(ivf_hdr, MK_TAG32('V', 'P', '8', '0'));
        ivf_hdr += 4;
    } else if (codec_id == MTAV_CODEC_VID_VP9) {
        MWR_LE32(ivf_hdr, MK_TAG32('V', 'P', '9', '0'));
        ivf_hdr += 4;
    }
    MWR_LE16(ivf_hdr, (unsigned short) vpx->vinfo.width);
    ivf_hdr += 2;
    MWR_LE16(ivf_hdr, (unsigned short) vpx->vinfo.height);
    ivf_hdr += 2;
    MWR_LE32(ivf_hdr, vpx->vinfo.frame_rate_num); // frame rate
    ivf_hdr += 4;
    MWR_LE32(ivf_hdr, vpx->vinfo.frame_rate_den); // time scale
    ivf_hdr += 4;
    MWR_LE32(ivf_hdr, total_frames);   // number of frames in file
    ivf_hdr += 4;
    MWR_LE32(ivf_hdr, 0);              // unused
    ivf_hdr += 4;

    if (vpx->vinfo.frame_rate_den > 0 && vpx->vinfo.frame_rate_num > 0) {
        vpx->insert_file_header = 0;
        //VDEC_DEBUG("Vpx frame rate(%d %d)\n", frame_rate.num, frame_rate.den);
    }
}

static void init_frame_header(int pkt_size,
    unsigned long long ipts, unsigned char *out_data)
{
    int i;
    int size = pkt_size;
    unsigned long long pts = ipts;
    unsigned char *hdr  = out_data + VPX_START_CODE_SIZE;

    memcpy(out_data, MTAVSF_FW_FRAME_HEADER, VPX_START_CODE_SIZE);
    for (i = 0; i < sizeof(size); i++) {
        *hdr++ = size & 0xff;
        size   = size >> 8;
    }

    for (i = 0; i < sizeof(pts); i++) {
        *hdr++ = pts & 0xff;
        pts    = pts >> 8;
    }
}

static int scf_vpx_init(MTAVStreamFilter *filter, void *init_info)
{
    if (!init_info) {
        return MTAVSF_FAILURE;
    }

    MTAV_SCFVPX *vpx = filter->priv_data;
    if (!vpx) {
        vpx = MTAVSF_MALLOC(sizeof(MTAV_SCFVPX));
        if (!vpx) {
            return MTAVSF_FAILURE;
        }
        memset(vpx, 0, sizeof(*vpx));
        filter->priv_data = vpx;
    }
    vpx->insert_file_header = 1;
    vpx->vinfo = *((MTAVSVideoInfo *) init_info);
    return MTAVSF_SUCCESS;
}

static int scf_vpx_get_info(MTAVStreamFilter *filter,
    MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info)
{
    int ret;
    int i;
    MTAV_SCFVPX *vpx = filter->priv_data;
    if (!vpx) {
        return MTAVSF_FAILURE;
    }

    (void)para;

    MTAVStreamBuffer *buffer =
        filter->stream_buffer;
    buffer->ibuffer = data;
    buffer->ibuffer_size = data_size;
    vpx->superframe_info.superframe_num = 1;
    vpx->superframe_info.size[0] = data_size;
    if (MTAV_CODEC_VID_VP9 == filter->codec_id) {
        ret = parse_vp9_superframe_info(data, data_size, &vpx->superframe_info);
        if (MTAVSF_SUCCESS != ret) {
            return ret;
        }
    }
    unsigned int total_size = vpx->insert_file_header ? VPX_FILE_HEADER_SIZE : 0;
    for (i = 0; i < vpx->superframe_info.superframe_num; i++) {
        total_size += VPX_FRAME_HEAER_SIZE + vpx->superframe_info.size[i];
    }

    *((unsigned int *) info) = total_size;
    buffer->obuffer      = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = total_size;

    return MTAVSF_SUCCESS;
}

static int scf_vpx_filter(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    int i;
    MTAV_SCFVPX *vpx = filter->priv_data;
    if (!vpx) {
        return MTAVSF_FAILURE;
    }

    MTAVStreamBuffer *buffer = filter->stream_buffer;
    unsigned char *ibuffer = buffer->ibuffer;
    unsigned char *obuffer = out_buf;
    unsigned int  ibuffer_size = buffer->ibuffer_size;
    unsigned int  obuffer_size = out_buf_size;
    unsigned char hdr[VPX_FILE_HEADER_SIZE] = {0};

    if (out_buf_size < buffer->obuffer_size) {
        MTAVSF_LOG("[VPX] Not support output partion data, Please provide integral buffer\n");
        return MTAVSF_FAILURE;
    }

    if (vpx->insert_file_header) {
        if (obuffer_size < VPX_FILE_HEADER_SIZE) {
            return MTAVSF_FAILURE;
        }

        init_file_header(vpx, filter->codec_id, hdr);
        filter->memcp_cb(obuffer, hdr, VPX_FILE_HEADER_SIZE);
        obuffer += VPX_FILE_HEADER_SIZE;
        obuffer_size -= VPX_FILE_HEADER_SIZE;
    }
    for (i = 0; i < vpx->superframe_info.superframe_num; i++) {
        unsigned int pkt_size = vpx->superframe_info.size[i];

        if (obuffer_size < (pkt_size + VPX_FRAME_HEAER_SIZE)) {
            return MTAVSF_FAILURE;
        }
        init_frame_header(pkt_size, filter->pts, hdr);
        filter->memcp_cb(obuffer, hdr, VPX_FRAME_HEAER_SIZE);
        obuffer += VPX_FRAME_HEAER_SIZE;
        filter->memcp_cb(obuffer, ibuffer, pkt_size);
        ibuffer += pkt_size;
        ibuffer_size -= pkt_size;
        obuffer += pkt_size;
        obuffer_size -= (pkt_size + VPX_FRAME_HEAER_SIZE);
    }
    return buffer->obuffer_size;
}

static void scf_vpx_flush(MTAVStreamFilter *filter)
{
    MTAV_SCFVPX *vpx = filter->priv_data;
    if (!vpx) {
        return;
    }
    vpx->insert_file_header = 1;
}

static void scf_vpx_deinit(MTAVStreamFilter *filter)
{
    if (filter->priv_data) {
        MTAVSF_FREE(filter->priv_data);
        filter->priv_data = NULL;
    }
}

MTAVStreamCodecFilter mtav_scf_vpx = {
    .init     = scf_vpx_init,
    .get_info = scf_vpx_get_info,
    .filter   = scf_vpx_filter,
    .flush    = scf_vpx_flush,
    .deinit   = scf_vpx_deinit,
};

void mtav_scf_write_vpx_file_header(
    MTAV_SCFVPX *vpx, unsigned int codec_id, unsigned char *buff)
{
    if (!vpx || !buff ||
        ((MTAV_CODEC_VID_VP8 != codec_id) &&
         (MTAV_CODEC_VID_VP9 != codec_id))) {
        return;
    }
	init_file_header(vpx, codec_id, buff);
}

void mtav_scf_write_vpx_frame_header(int pkt_size,
    unsigned long long pkt_pts, unsigned char *out_data)
{
    if (!out_data) {
        return;
    }

    init_frame_header(pkt_size, pkt_pts, out_data);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
