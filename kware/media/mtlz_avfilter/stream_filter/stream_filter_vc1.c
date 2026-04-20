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
static int vc1_get_level(unsigned int profile,
    unsigned int width, unsigned int height)
{
    int total_mbs;
    total_mbs = (width * height) >> 8;

    if (profile == VC1_PROFILE_SIMPLE) {
        if (total_mbs <= 99) { //low
            return VC1_LEVEL_LOW;
        } else if (total_mbs <= 396) { //medium
            return VC1_LEVEL_MEDIUM;
        } else {
            return -1;
        }
    } else if (profile == VC1_PROFILE_MAIN) {
        if (total_mbs <= 396) { //low
            return VC1_LEVEL_LOW;
        } else if (total_mbs <= 1620) { //medium
            return VC1_LEVEL_MEDIUM;
        } else if (total_mbs <= 8192) { //high
            return VC1_LEVEL_HIGH;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

/* MSB will be put first if put_bits_num larger than 8 */
static int vc1_put_bits(unsigned char *bits_buf,
    int *bits_pos, unsigned int value, int put_bits_num)
{
    uint8_t bit_byte;
    int bit_shift;
    int bit_byte_pos;

    bit_byte_pos = *bits_pos / 8;
    bit_shift    = *bits_pos % 8;
    bit_byte     = *(bits_buf + bit_byte_pos);

    if (bit_shift + put_bits_num <= 8) {
        bit_byte = (bit_byte << put_bits_num) + value;
        *(bits_buf + bit_byte_pos) = bit_byte;
        *bits_pos += put_bits_num;
    } else {
        int new_n = bit_shift + put_bits_num - 8;
        int new_value = 0;
        bit_byte = (bit_byte << (8 - bit_shift)) + (value >> new_n);
        *(bits_buf + bit_byte_pos) = bit_byte;
        *bits_pos += (8 - bit_shift);
        new_value = value & ((1 << new_n) - 1);
        vc1_put_bits(bits_buf, bits_pos, new_value, new_n);
    }

    return MTAVSF_SUCCESS;
}

static const unsigned char *vc1_search_next_marker(
    const unsigned char *src, const unsigned char *end)
{
    unsigned int mark = 0xFFFFFFFF;
    if (end - src < 4) {
        return end;
    }

    while (src < end) {
        mark = (mark << 8) | *src++;
        if (IS_VC1_MARKER(mark)) {
            return src - 4;
        }
    }

    return end;
}

static int vc1_gen_display_bits_adv(
    const uint8_t *raw_bits,
    unsigned char *bits_buf, int *bits_pos)
{
    vc1_put_bits(bits_buf, bits_pos, (*raw_bits), 8);

    return MTAVSF_SUCCESS;
}

static int add_sequence_header_adv(unsigned char *vc1_bits_buf, int *vc1_bits_pos)
{
    int level;
    int chromaformat;     ///< 2bits, 2=4:2:0, only defined
    int postprocflag;     ///< Per-frame processing suggestion flag present
    int broadcast;        ///< TFF/RFF present
    int interlace;        ///< Progressive/interlaced (RPTFTM syntax element)
    int tfcntrflag;       ///< TFCNTR present
    int frmrtq_postproc;  ///< 3bits,
    int bitrtq_postproc;  ///< 5bits, quantized framerate-based postprocessing strength
    int finterpflag;      ///< INTERPFRM present
    struct VC1_STRUCT_SEQUENCE_HEADER_A struct_a = {0};

    level = 3;
    vc1_put_bits(vc1_bits_buf, vc1_bits_pos, level, 3);
    //
    chromaformat = 1;
    vc1_put_bits(vc1_bits_buf, vc1_bits_pos, chromaformat, 2);
    frmrtq_postproc = 7;
    vc1_put_bits(vc1_bits_buf, vc1_bits_pos, frmrtq_postproc, 3); //common
    bitrtq_postproc = 31;
    vc1_put_bits(vc1_bits_buf, vc1_bits_pos, bitrtq_postproc, 5); //common
    postprocflag = 0;
    vc1_put_bits(vc1_bits_buf, vc1_bits_pos, postprocflag, 1);  //common
    struct_a.ver_size = 639;
    vc1_put_bits(vc1_bits_buf, vc1_bits_pos, struct_a.ver_size, 12)  ;

    struct_a.horz_size = 359;
    vc1_put_bits(vc1_bits_buf, vc1_bits_pos, struct_a.horz_size, 12)  ;
    broadcast = 1;
    vc1_put_bits(vc1_bits_buf, vc1_bits_pos, broadcast, 1);
    interlace = 0;
    vc1_put_bits(vc1_bits_buf, vc1_bits_pos, interlace, 1);
    tfcntrflag = 0;
    vc1_put_bits(vc1_bits_buf, vc1_bits_pos, tfcntrflag, 1);
    finterpflag = 0;
    vc1_put_bits(vc1_bits_buf, vc1_bits_pos, finterpflag, 1);
    vc1_put_bits(vc1_bits_buf, vc1_bits_pos, 0, 1); // reserved

    return MTAVSF_SUCCESS;
}

static int vc1_add_sequence_header(MTAVStreamFilter *filter,
    MTAV_SCFVC1 *vc1, unsigned char *data, unsigned char *bits_buf, int *bits_pos)
{
    const unsigned char  *next = NULL;
    MTAVSVideoInfo      *vinfo = &vc1->vinfo;
    const unsigned char *start = vinfo->extradata;
    const unsigned char *end   = vinfo->extradata + vinfo->extradata_size;
    struct VC1_STRUCT_SEQUENCE_HEADER_B strct_b  = {0};
    struct VC1_STRUCT_SEQUENCE_HEADER_C struct_c = {0};

    /*
     * vc-1 consists of wvc1 and wmv3,
     * wvc1 is advanced profile,
     * and wmv3 is simple/main profile
     */
    if (MTAV_CODEC_VID_VC1_WMV3 == filter->codec_id) {
        int num_frames;
        int profile;

        profile = vinfo->extradata[0] >> 6; //2bits
        end = vinfo->extradata + 4;
        /* write seq start code MONTSOC0 */
        vc1_put_bits(bits_buf, bits_pos, 0x4d4f4e54, 32);
        vc1_put_bits(bits_buf, bits_pos, 0x534f4300, 32);

        strct_b.framerate = (int) vinfo->frame_rate_num /  vinfo->frame_rate_den;
        num_frames = 0;
        vc1_put_bits(bits_buf, bits_pos, (num_frames) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (num_frames >> 8) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (num_frames >> 16) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, 0xc5, 8);
        ///write 0x00000004
        vc1_put_bits(bits_buf, bits_pos, 0x4, 8);
        vc1_put_bits(bits_buf, bits_pos, 0, 24);
        ///write struct c
        for (next = start; next < end; next++) {
            vc1_put_bits(bits_buf, bits_pos, *next, 8);
        }

        ///write struct a
        vc1_put_bits(bits_buf, bits_pos, (vinfo->height) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (vinfo->height >> 8) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (vinfo->height >> 16) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (vinfo->height >> 24) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (vinfo->width) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (vinfo->width >> 8) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (vinfo->width >> 16) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (vinfo->width >> 24) & 0xff, 8);
        ///write 0x0000000c
        vc1_put_bits(bits_buf, bits_pos, 0xc, 8);
        vc1_put_bits(bits_buf, bits_pos, 0, 24);
        //write struct b
        strct_b.level = vc1_get_level(profile, vinfo->width, vinfo->height);
        vc1_put_bits(bits_buf, bits_pos, 0, 24);
        //vc1_put_bits(bits_buf, bits_pos, level << 3, 8);
        vc1_put_bits(bits_buf, bits_pos, 0x80, 8);
        vc1_put_bits(bits_buf, bits_pos, 0, 32);
        strct_b.framerate = 0xffffffff;
        vc1_put_bits(bits_buf, bits_pos, (strct_b.framerate) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (strct_b.framerate >> 8) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (strct_b.framerate >> 16) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (strct_b.framerate >> 24) & 0xff, 8);
    } else {
        int pkt_header = ((*data) << 24) + ((*(data + 1)) << 16)
                         + ((*(data + 2)) << 8) + (*(data + 3));
        if (pkt_header == VC1_CODE_SEQHDR) {
            return MTAVSF_SUCCESS;
        }

        start = vc1_search_next_marker(start, end); // in WVC1 extradata first byte is its size, but can be 0 in mkv
        for (next = start; next < start + 9; next++) {
            vc1_put_bits(bits_buf, bits_pos, *next, 8);
        }
        vc1_gen_display_bits_adv(start + 9, bits_buf, bits_pos);
        for (next = start + 10; next < end; next++) {
            vc1_put_bits(bits_buf, bits_pos, *next, 8);
        }
    }

    if (VC1_PROFILE_ADVANCED == struct_c.profile) {
        return add_sequence_header_adv(bits_buf, bits_pos);
    }

    return MTAVSF_SUCCESS;
}

static int vc1_add_frame_header(MTAVStreamFilter *filter, unsigned int keyframe,
    unsigned char *data, long long pts, unsigned int data_size, unsigned char *bits_buf, int *bits_pos)
{
    if (MTAV_CODEC_VID_VC1_WMV3 == filter->codec_id) {
        unsigned int timestamp, size;

        vc1_put_bits(bits_buf, bits_pos, (0x4d4f4e54), 32);
        vc1_put_bits(bits_buf, bits_pos, (0x534f4301), 32);

        size = data_size | (keyframe ? 0x80000000 : 0);
        vc1_put_bits(bits_buf, bits_pos, (size) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (size >> 8) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (size >> 16) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (size >> 24) & 0xff, 8);
        timestamp = (unsigned int) pts;
        //write timestamp
        vc1_put_bits(bits_buf, bits_pos, (timestamp) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (timestamp >> 8) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (timestamp >> 16) & 0xff, 8);
        vc1_put_bits(bits_buf, bits_pos, (timestamp >> 24) & 0xff, 8);
    } else {
        int pkt_header = ((*data) << 24) + ((*(data + 1)) << 16)
                         + ((*(data + 2)) << 8) + (*(data + 3));

        if (pkt_header != VC1_CODE_FRAME && pkt_header != VC1_CODE_SEQHDR) {
            vc1_put_bits(bits_buf, bits_pos, VC1_CODE_FRAME, 32)  ;
        }
    }

    if (((*bits_pos) % 8) != 0) {
        vc1_put_bits(bits_buf, bits_pos, 0, 8 - ((*bits_pos) % 8));
    }
    /* convert to next writting byte pos, equal to buffer size */
    *bits_pos = ROUNDUP((*bits_pos), 8) / 8 ;

    return MTAVSF_SUCCESS;
}

#endif

#if MT_DES("External API Definition", 1)
static int scf_vc1_init(MTAVStreamFilter *filter, void *init_info)
{
    if (!init_info) {
        return MTAVSF_FAILURE;
    }

    MTAV_SCFVC1 *vc1 = (MTAV_SCFVC1 *) filter->priv_data;
    if (!vc1) {
        vc1 = MTAVSF_MALLOC(sizeof(MTAV_SCFVC1));
        if (!vc1) {
            return MTAVSF_FAILURE;
        }
        memset((void *)vc1, 0, sizeof(*vc1));
        filter->priv_data = vc1;
    }

    if (vc1->vinfo.extradata) {
        MTAVSF_FREE(vc1->vinfo.extradata);
    }

    vc1->vinfo.extradata = NULL;
    vc1->vinfo.extradata_size = 0;
    MTAVSVideoInfo *info = (MTAVSVideoInfo *) init_info;
    vc1->vinfo = *info;
    /* The bytes of extra_data */
    if (!info->extradata || !info->extradata_size) {
        MTAVSF_LOG("[VC1] init without extra data!\n");
        return MTAVSF_SUCCESS;
    }
    /* The extra data need in filter */
    vc1->vinfo.extradata = (unsigned char *)
        MTAVSF_MALLOC(info->extradata_size + MTAVSF_PADDING_SIZE);
    if (!(vc1->vinfo.extradata)) {
        MTAVSF_LOG("[VC1] Copy extra data malloc memory fail!\n");
        goto fail;
    }
    memcpy(vc1->vinfo.extradata, info->extradata, info->extradata_size);
    vc1->vinfo.extradata_size = info->extradata_size;

    return MTAVSF_SUCCESS;
fail:
    vc1->vinfo.extradata = NULL;
    vc1->vinfo.extradata_size = 0;
    MTAVSF_FREE(filter->priv_data);
    filter->priv_data = NULL;
    return MTAVSF_FAILURE;
}

static int scf_vc1_get_info(MTAVStreamFilter *filter,
    MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info)
{
    MTAV_SCFVC1 *vc1 = filter->priv_data;
    if (!vc1) {
        return MTAVSF_FAILURE;
    }

    MTAVStreamBuffer *buffer = filter->stream_buffer;
    buffer->ibuffer = data;
    buffer->ibuffer_indx = 0;
    buffer->ibuffer_size = data_size;

    vc1->header_size      = 0;
    MTAVSVideoInfo *vinfo = &vc1->vinfo;
    unsigned int keyframe = para ? (para->flags & MTAVSTREAM_FLAGS_KEYFRAME) : 0;
    if (keyframe && vinfo->extradata && vinfo->extradata_size) {
        vc1_add_sequence_header(filter, vc1, data, &vc1->header[0], (int *) &vc1->header_size);
    }

    if(vinfo->extradata && vinfo->extradata_size) {
        vc1_add_frame_header(filter, keyframe,
        data, pts, data_size, &vc1->header[0], (int *) &vc1->header_size);
    }

    buffer->obuffer = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = vc1->header_size + data_size;
    *((unsigned int *) info) = buffer->obuffer_size;

    return MTAVSF_SUCCESS;
}

static int scf_vc1_filter(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    MTAV_SCFVC1 *vc1 = filter->priv_data;
    MTAVStreamBuffer *buffer = filter->stream_buffer;
    if (!vc1 || !buffer->ibuffer || !buffer->ibuffer_size) {
        return MTAVSF_FAILURE;
    }

    unsigned char *obuffer = out_buf;
    unsigned int  obuffer_size = out_buf_size;
    if (out_buf_size < buffer->obuffer_size) {
        MTAVSF_LOG("[VC1] Not support output partion of data, Please provide integral buffer\n");
        return MTAVSF_FAILURE;
    }

    if (vc1->header_size) {
        filter->memcp_cb(obuffer, &vc1->header[0], vc1->header_size);
        obuffer      += vc1->header_size;
        obuffer_size -= vc1->header_size;
    }
    unsigned int copy_size =
        MTAVSF_MIN(buffer->ibuffer_size, obuffer_size);
    filter->memcp_cb(obuffer, buffer->ibuffer, copy_size);

    return (copy_size + vc1->header_size);
}

static void scf_vc1_flush(MTAVStreamFilter *filter)
{
    MTAV_SCFVC1 *vc1 = filter->priv_data;

    if (!vc1) {
        return;
    }
    vc1->header_size = 0;
    memset((void *)(&vc1->header[0]), 0, MAX_VC1_HEADER_SIZE);
}

static void scf_vc1_deinit(MTAVStreamFilter *filter)
{
    if (!filter->priv_data) {
        return;
    }
    MTAV_SCFVC1 *vc1 = filter->priv_data;
    if (vc1->vinfo.extradata) {
        MTAVSF_FREE(vc1->vinfo.extradata);
        vc1->vinfo.extradata = NULL;
        vc1->vinfo.extradata_size = 0;
    }

    MTAVSF_FREE(filter->priv_data);
    filter->priv_data = NULL;
}

MTAVStreamCodecFilter mtav_scf_vc1 = {
	.init     = scf_vc1_init,
	.get_info = scf_vc1_get_info,
	.filter   = scf_vc1_filter,
	.flush    = scf_vc1_flush,
	.deinit   = scf_vc1_deinit,
};

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
