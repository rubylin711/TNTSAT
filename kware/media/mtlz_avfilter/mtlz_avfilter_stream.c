/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mtlz_avfilter_stream.h"
#include "stream_filter/stream_filter.h"
#include <stdint.h>
#include "limits.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

extern MTAVStreamCodecFilter mtav_scf_vc1;
extern MTAVStreamCodecFilter mtav_scf_vpx;
extern MTAVStreamCodecFilter mtav_scf_h264;
extern MTAVStreamCodecFilter mtav_scf_h265;

#if MT_DES("DEFAULT FILTER", 1)
static int scf_default_get_info(MTAVStreamFilter *filter,
     MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info)
{
    (void) para;
    MTAVStreamBuffer *buffer = filter->stream_buffer;

    buffer->ibuffer = data;
    buffer->ibuffer_indx = 0;
    buffer->ibuffer_size = data_size;

    buffer->obuffer = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = data_size;
    *((unsigned int *) info) = data_size;
    return MTAVSF_SUCCESS;
}

static int scf_default_filter(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    MTAVStreamBuffer *buffer = filter->stream_buffer;
    unsigned int copy_size =
        MTAVSF_MIN(buffer->ibuffer_size, out_buf_size);

    filter->memcp_cb(out_buf, buffer->ibuffer, copy_size);

    return (int) copy_size;
}

static MTAVStreamCodecFilter mtav_scf_default = {
    .init     = NULL,
    .get_info = scf_default_get_info,
    .filter   = scf_default_filter,
    .flush    = NULL,
    .deinit   = NULL,
};
#endif

#if MT_DES("LPCM FILTER", 1)
static int scf_lpcm_get_info(MTAVStreamFilter *filter,
     MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info)
{
    (void) para;
    MTAVStreamBuffer *buffer = filter->stream_buffer;

    buffer->ibuffer = NULL;
    buffer->ibuffer_indx = 0;
    buffer->ibuffer_size = 0;
    if (data_size > 4) {
        buffer->ibuffer = data + 4;
        buffer->ibuffer_size = data_size - 4;
    }

    buffer->obuffer = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = buffer->ibuffer_size;
    *((unsigned int *) info) = buffer->ibuffer_size;
    return MTAVSF_SUCCESS;
}

static int scf_lpcm_filter(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    MTAVStreamBuffer *buffer = filter->stream_buffer;
    unsigned int copy_size =
        MTAVSF_MIN(buffer->ibuffer_size, out_buf_size);

    filter->memcp_cb(out_buf, buffer->ibuffer, copy_size);

    return (int) copy_size;
}

static MTAVStreamCodecFilter mtav_scf_lpcm = {
    .init     = NULL,
    .get_info = scf_lpcm_get_info,
    .filter   = scf_lpcm_filter,
    .flush    = NULL,
    .deinit   = NULL,
};
#endif

#if MT_DES("AC 4 AUDIO", 1)
#define AC4_SYNC_WORD       0xAC40  /* 16 sync bits without CRC */
#define AC4_SYNC_WORD_CRC   0xAC41  /* 16 sync bits with    CRC */

static void check_ac4_sync_word(MTAV_SCFAC4 *ac4,
    unsigned char *data, unsigned int data_size)
{
    int is_raw = 1;
    int reamin = (int) data_size;
    unsigned char *d = data;
    while (reamin-- >= 2) {
        if ((MRD_BE16(d) == AC4_SYNC_WORD) ||
            (MRD_BE16(d) == AC4_SYNC_WORD_CRC)) {
            is_raw = 0;
           break;
        } else {
            d++;
        }
    }
    ac4->raw = is_raw;
    ac4->frame_size_field_len = 0;
}

static int scf_ac4_init(MTAVStreamFilter *filter, void *init_info)
{
    (void) init_info;
    MTAV_SCFAC4 *ac4 = (MTAV_SCFAC4 *) filter->priv_data;
    if (!ac4) {
        ac4 = MTAVSF_MALLOC(sizeof(MTAV_SCFAC4));
        if (!ac4) {
            return MTAVSF_FAILURE;
        }
        ac4->raw = 0;
        ac4->frame_size_field_len = (unsigned char) (-1);
        filter->priv_data = ac4;
    }
    return MTAVSF_SUCCESS;
}

static int scf_ac4_get_info(MTAVStreamFilter *filter,
     MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info)
{
    (void) para;
    MTAVStreamBuffer *buffer = filter->stream_buffer;

    buffer->ibuffer = data;
    buffer->ibuffer_indx = 0;
    buffer->ibuffer_size = data_size;
    if (!filter->priv_data) {
        MTAVSF_LOG("No ac4 private data\n");
        return MTAVSF_FAILURE;
    }

    MTAV_SCFAC4 *ac4 = filter->priv_data;
    if ((unsigned char) (-1) == ac4->frame_size_field_len) {
        check_ac4_sync_word(ac4, data, data_size);
    }

    ac4->frame_size_field_len = 0;
    if (ac4->raw) {
        /* The header size for AC-4 parser, only include (sync word + frame size) */
        ac4->frame_size_field_len = 4;
        if (data_size > 0xFFFF) {
            ac4->frame_size_field_len += 3;
        }
    }

    buffer->obuffer = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = data_size + ac4->frame_size_field_len;
    *((unsigned int *) info) = data_size + ac4->frame_size_field_len;
    return MTAVSF_SUCCESS;
}

static int scf_ac4_filter(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    MTAVStreamBuffer *buffer = filter->stream_buffer;
    unsigned int copy_size =
        MTAVSF_MIN(buffer->obuffer_size, out_buf_size);

    MTAV_SCFAC4 *ac4 = filter->priv_data;
    unsigned char *obuffer = out_buf;
    if (ac4->raw) {
        int start_index = 2;
        unsigned int raw_data_size = buffer->ibuffer_size;
        unsigned char header[AC4_MAX_HEAD_LENGTH] = {0xAC, 0x40, 0xFF, 0xFF};
        if (ac4->frame_size_field_len > 4) {
            start_index += 2;
            header[start_index] = (raw_data_size >> 16) & 0xFF;
            start_index += 1;
        }
        header[start_index] = (raw_data_size >> 8) & 0xFF;
        header[start_index + 1] = (raw_data_size) & 0xFF;
        filter->memcp_cb(obuffer, &(header[0]), ac4->frame_size_field_len);
        obuffer += ac4->frame_size_field_len;
        copy_size -= ac4->frame_size_field_len;
    }

    filter->memcp_cb(obuffer, buffer->ibuffer, copy_size);
    return (int) out_buf_size;
}

static void scf_ac4_deinit(MTAVStreamFilter *filter)
{
    if (filter->priv_data) {
        MTAVSF_FREE(filter->priv_data);
        filter->priv_data = NULL;
    }
}

static MTAVStreamCodecFilter mtav_scf_ac4 = {
    .init     = scf_ac4_init,
    .get_info = scf_ac4_get_info,
    .filter   = scf_ac4_filter,
    .flush    = NULL,
    .deinit   = scf_ac4_deinit,
};
#endif

#if MT_DES("DOLBY TRUEHD AUDIO", 1)
#define AC3_SYNC_WORD           0x0B77
#define AC3_HEADER_MIN            7
/*
* A Dolby TrueHD bit stream consists of a sequence of access units.
* Major syncs can be distinguished by the fact that the bitfield of
* 32 bits starting at bit offset 32 from the start of the access unit always equals 0xF8726FBA,
*/
#define MLP_MAJOR_SYNC_WORD   0xF8726FBA
enum {
    DOLBY_TRUEHDC_SYNC_HEAD = 0,
    DOLBY_TRUEHDC_FIND_HEADER,
};

/**
 * Possible frame sizes.
 * from ATSC A/52 Table 5.18 Frame Size Code Table.
 */
const uint16_t ac3_frame_size_tab[38][3] = {
    { 64,   69,   96   },
    { 64,   70,   96   },
    { 80,   87,   120  },
    { 80,   88,   120  },
    { 96,   104,  144  },
    { 96,   105,  144  },
    { 112,  121,  168  },
    { 112,  122,  168  },
    { 128,  139,  192  },
    { 128,  140,  192  },
    { 160,  174,  240  },
    { 160,  175,  240  },
    { 192,  208,  288  },
    { 192,  209,  288  },
    { 224,  243,  336  },
    { 224,  244,  336  },
    { 256,  278,  384  },
    { 256,  279,  384  },
    { 320,  348,  480  },
    { 320,  349,  480  },
    { 384,  417,  576  },
    { 384,  418,  576  },
    { 448,  487,  672  },
    { 448,  488,  672  },
    { 512,  557,  768  },
    { 512,  558,  768  },
    { 640,  696,  960  },
    { 640,  697,  960  },
    { 768,  835,  1152 },
    { 768,  836,  1152 },
    { 896,  975,  1344 },
    { 896,  976,  1344 },
    { 1024, 1114, 1536 },
    { 1024, 1115, 1536 },
    { 1152, 1253, 1728 },
    { 1152, 1254, 1728 },
    { 1280, 1393, 1920 },
    { 1280, 1394, 1920 },
};

static int get_ac3_frame_info(
    MTAV_SCFDolbyTrueHDC *true_hdc,
    unsigned char *data, unsigned int data_size, unsigned int *sr_code)
{
    if (data_size < AC3_HEADER_MIN) {
        if (sr_code) {
            *sr_code = true_hdc->sample_rate;
        }
        return data_size;
    }

    data += 2; /* skip sync word 0x0B77 */
    unsigned int frame_size = 0;
    /* bsi info, 29th bits next to sync word, 5bits long */
    unsigned int bitstream_id = (unsigned int) (((MRD_BE16((&data[4]))) >> 6) & 0x1F);
    if (bitstream_id > 16) {
        return MTAVSF_FAILURE;
    }

    if(bitstream_id <= 10) {
        /* Normal AC-3 */
        unsigned int sample_rate_code = (unsigned int) ((data[2] >> 6) & 0x3);
        /* 00:48, 01:44.1, 10:32 11:Reserved */
        if (sample_rate_code == 3) {
            MTAVSF_LOG("Dolby truehdc ac3 sample_rate_code 3 error\n");
            return MTAVSF_FAILURE;
        }
        unsigned int frame_size_code = (unsigned int) (data[2] & 0x3F);
        if(frame_size_code > 37) {
            MTAVSF_LOG("Dolby truehdc ac3 frame_size_code %d error\n", frame_size_code);
            return MTAVSF_FAILURE;
        }
        frame_size = ac3_frame_size_tab[frame_size_code][sample_rate_code] * 2;
        if (-1 == true_hdc->frame_size) {
            true_hdc->frame_size = frame_size;
        }
        if (-1 == true_hdc->sample_rate) {
            true_hdc->sample_rate = sample_rate_code;
        }
        if (sr_code) {
            *sr_code = sample_rate_code;
        }
    } else {
        unsigned int frame_type = (unsigned int) ((data[0] >> 6) & 0x3);
        if (frame_type) {
            MTAVSF_LOGA("Dolby truehdc eac3 frame_type 3 error\n");
            return MTAVSF_FAILURE;
        }
        /* Enhanced AC-3 frame size 11bits */
        frame_size = (unsigned int) (((MRD_BE16(data)) & 0x7FF) + 1);
        if (frame_size < AC3_HEADER_MIN) {
            MTAVSF_LOGA("Dolby truehdc eac3 frame size %d error\n", frame_size);
            return MTAVSF_FAILURE;
        }
        /* Dolby truehdc can only play ac3 */
        return MTAVSF_FAILURE;
    }

    MTAVSF_LOGA("Dolby truehdc ac3 bitstream id %d, frame_size:%d data size:%d \n", bitstream_id, frame_size, data_size);
    return frame_size;
}

static int scf_dolby_truehdc_init(MTAVStreamFilter *filter, void *init_info)
{
    (void) init_info;
    MTAV_SCFDolbyTrueHDC *true_hdc = (MTAV_SCFDolbyTrueHDC *) filter->priv_data;
    if (!true_hdc) {
        true_hdc = MTAVSF_MALLOC(sizeof(MTAV_SCFDolbyTrueHDC));
        if (!true_hdc) {
            return MTAVSF_FAILURE;
        }
        true_hdc->assembler.buffer = NULL;
        filter->priv_data = true_hdc;
    }

    if (true_hdc->assembler.buffer) {
        MTAVSF_FREE(true_hdc->assembler.buffer);
        true_hdc->assembler.buffer = NULL;
    }
    true_hdc->assembler.pts  = -1;
    true_hdc->assembler.pos  = 0;
    true_hdc->assembler.size = 0;
    true_hdc->frame_size  = -1;
    true_hdc->sample_rate = -1;
    true_hdc->state = DOLBY_TRUEHDC_SYNC_HEAD;
    return MTAVSF_SUCCESS;
}

static int dolby_truehdc_send_assembler(
    MTAV_SCFDolbyTrueHDC *true_hdc, long long pts,
    unsigned char *data, unsigned int data_size)
{
    unsigned int newsize   =
        true_hdc->assembler.size + data_size;
    unsigned char *newbuff =
        MTAVSF_REALLOC (true_hdc->assembler.buffer, newsize + MTAVSF_PADDING_SIZE);
    if (!newbuff) {
        MTAVSF_LOG("Malloc dolby truehdc assembler %d bytes fail\n", newsize);
        return MTAVSF_FAILURE;
    }

    MTAVSF_LOGA("Dolby truehdc send assembler, offset:%d size:%d\n", true_hdc->assembler.size, newsize);
    memcpy((unsigned char *) ((uintptr_t) newbuff + true_hdc->assembler.size), data, data_size);
    true_hdc->pts              = pts;
    if (true_hdc->assembler.pts == -1) {
        true_hdc->assembler.pts = pts;
    }
    true_hdc->assembler.size   = newsize;
    true_hdc->assembler.buffer = newbuff;
    return MTAVSF_SUCCESS;
}
/* A major sync is present in the first access unit of the file
 * when the 32 bits of the file starting at bit offset 32 are equal to 0xF8726FBA
*/
static int dolby_truehdc_assembler_find_header(MTAV_SCFDolbyTrueHDC *true_hdc)
{
    while (true_hdc->assembler.pos + AC3_HEADER_MIN <= true_hdc->assembler.size) {
        unsigned char *ptr = &true_hdc->assembler.buffer[true_hdc->assembler.pos];
        if (MRD_BE16(ptr) == AC3_SYNC_WORD) {
            return AC3_SYNC_WORD;
        } else if (MRD_BE32(ptr) == MLP_MAJOR_SYNC_WORD) {
            return MLP_MAJOR_SYNC_WORD;
        }
        true_hdc->assembler.pos++;
    }
    return MTAVSF_FAILURE;
}
/*
 * return 0  : data before will be dropped and not output.
 * return -1 : not drop data and recheck again.
 * other     : output and drop data.
*/
static int assembler_append_output_buffer(
    MTAVStreamFilter *filter,
    MTAV_SCFDolbyTrueHDC *true_hdc, MTAVStreamBuffer *buffer)
{
    int frame_size = get_ac3_frame_info(true_hdc,
        true_hdc->assembler.buffer, true_hdc->assembler.pos, NULL);
    if (frame_size < 0) { /* */
        /* error frame, not output */
        return MTAVSF_SUCCESS;
    } else if (frame_size < AC3_HEADER_MIN) { /* Not complete frame, need recheck */
        return MTAVSF_FAILURE;
    } else {
        frame_size = MTAVSF_MIN(true_hdc->assembler.pos, frame_size);
    }
    unsigned int newsize = frame_size + buffer->obuffer_size;
    unsigned char *newbuff =
        MTAVSF_REALLOC (buffer->obuffer, newsize + MTAVSF_PADDING_SIZE);
    if (!newbuff) {
        MTAVSF_LOG("Malloc dolby truehdc output buffer %d bytes fail\n", newsize);
        return MTAVSF_FAILURE;
    }
    if (0 == buffer->obuffer_size) {
        filter->pts = true_hdc->assembler.pts;
    }

    MTAVSF_LOGA("Dolby truehdc append output buffer, offset:%d size:%d\n", buffer->obuffer_size, newsize);
    memcpy((unsigned char *) ((uintptr_t) newbuff + buffer->obuffer_size),
        true_hdc->assembler.buffer, frame_size);
    buffer->ocb = MTAVSF_FREE;
    buffer->obuffer = newbuff;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = newsize;
    return MTAVSF_SUCCESS;
}

static void assembler_move_left_buffer(MTAV_SCFDolbyTrueHDC *true_hdc)
{
    if (0 == true_hdc->assembler.pos) {
        return;
    }
    unsigned int left_size = true_hdc->assembler.size - true_hdc->assembler.pos;
    memmove(true_hdc->assembler.buffer,
        &true_hdc->assembler.buffer[true_hdc->assembler.pos], left_size);
    true_hdc->assembler.pos  = 0;
    true_hdc->assembler.size = left_size;
}

static int assembler_handle_ac3_sync_word(
    MTAVStreamFilter *filter,
    MTAV_SCFDolbyTrueHDC *true_hdc, MTAVStreamBuffer *buffer)
{
    unsigned int  sr_code  = 0;
    unsigned char *data    = &true_hdc->assembler.buffer[true_hdc->assembler.pos];
    unsigned int data_size = true_hdc->assembler.size - true_hdc->assembler.pos;
    int ret = get_ac3_frame_info(true_hdc, data, data_size, &sr_code);
    int is_not_same_config_ac3 =
        !(ret == true_hdc->frame_size && sr_code == true_hdc->sample_rate);

    /* Not real ac3 header */
    if (ret < 0 || (is_not_same_config_ac3)) {
        true_hdc->assembler.pos++;
        return true_hdc->state;
    }

    if (DOLBY_TRUEHDC_SYNC_HEAD == true_hdc->state) {
        true_hdc->assembler.pts = true_hdc->pts;
        true_hdc->state = DOLBY_TRUEHDC_FIND_HEADER;
    } else {/* (DOLBY_TRUEHDC_FIND_HEADER == true_hdc->state) */
        ret = assembler_append_output_buffer(filter, true_hdc, buffer);
        if (ret < 0) {
            return MTAVSF_FAILURE;
        }
    }
    assembler_move_left_buffer(true_hdc);
    true_hdc->assembler.pos++;
    return DOLBY_TRUEHDC_FIND_HEADER;
}

static int assembler_handle_mlp_major_sync_word(
    MTAVStreamFilter *filter,
    MTAV_SCFDolbyTrueHDC *true_hdc, MTAVStreamBuffer *buffer)
{
    if (DOLBY_TRUEHDC_FIND_HEADER == true_hdc->state) {
        /* starting at bit offset 32 */
        if (true_hdc->assembler.pos < 4) {
            MTAVSF_LOG("Error, find mlp major sync word, but data less than 4\n");
            return MTAVSF_FAILURE;
        }
        true_hdc->assembler.pos -= 4;
        if (assembler_append_output_buffer(filter, true_hdc, buffer) < 0) {/* (DOLBY_TRUEHDC_FIND_HEADER == true_hdc->state) */
            return MTAVSF_FAILURE;
        }
    }
    true_hdc->assembler.pos++;
    true_hdc->state = DOLBY_TRUEHDC_SYNC_HEAD;
    assembler_move_left_buffer(true_hdc);
    return DOLBY_TRUEHDC_SYNC_HEAD;
}

static int dolby_truehdc_assembler_parse_data(
    MTAVStreamFilter *filter,
    MTAV_SCFDolbyTrueHDC *true_hdc, MTAVStreamBuffer *buffer)
{
    buffer->obuffer = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = 0;

    int ret = 0;
    while (MTAVSF_FAILURE != (ret =
        dolby_truehdc_assembler_find_header(true_hdc))) {
        if (AC3_SYNC_WORD == ret) {
            ret = assembler_handle_ac3_sync_word(filter, true_hdc, buffer);
            if (ret < 0) {
                return ret;
            }
        } else if (MLP_MAJOR_SYNC_WORD == ret) {
            ret = assembler_handle_mlp_major_sync_word(filter, true_hdc, buffer);
            if (ret < 0) {
                return ret;
            }
        }
    }
    return MTAVSF_SUCCESS;
}

static int scf_dolby_truehdc_get_info(MTAVStreamFilter *filter,
     MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info)
{
    (void) para;
    MTAVStreamBuffer *buffer = filter->stream_buffer;

    buffer->ibuffer = data;
    buffer->ibuffer_indx = 0;
    buffer->ibuffer_size = data_size;
    if (!filter->priv_data) {
        MTAVSF_LOG("%s No true_hdc private data\n", __func__);
        return MTAVSF_FAILURE;
    }

    int ret;
    MTAV_SCFDolbyTrueHDC *true_hdc = filter->priv_data;
    ret = dolby_truehdc_send_assembler(true_hdc, pts, data, data_size);
    if (ret < 0) {
        return MTAVSF_FAILURE;
    }

    ret = dolby_truehdc_assembler_parse_data(filter, true_hdc, buffer);
    if (ret < 0) {
        return ret;
    }
    *((unsigned int *) info) = buffer->obuffer_size;
    return MTAVSF_SUCCESS;
}

static int scf_dolby_truehdc_filter(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    (void) filter;
    (void) out_buf;
    (void) out_buf_size;
    return MTAVSF_FAILURE;
}

static void scf_dolby_truehdc_flush(MTAVStreamFilter *filter)
{
    MTAV_SCFDolbyTrueHDC *true_hdc = filter->priv_data;
    if (!true_hdc) {
        MTAVSF_LOG("%s No true_hdc private data\n", __func__);
        return;
    }
    if (true_hdc->assembler.buffer) {
        MTAVSF_FREE(true_hdc->assembler.buffer);
        true_hdc->assembler.pts    = -1;
        true_hdc->assembler.pos    = 0;
        true_hdc->assembler.size   = 0;
        true_hdc->assembler.buffer = NULL;
    }
    true_hdc->state = DOLBY_TRUEHDC_SYNC_HEAD;
}

static void scf_dolby_truehdc_deinit(MTAVStreamFilter *filter)
{
    MTAV_SCFDolbyTrueHDC *true_hdc = filter->priv_data;
    if (!true_hdc) {
        MTAVSF_LOG("%s No true_hdc private data\n", __func__);
        return;
    }

    if (true_hdc->assembler.buffer) {
        MTAVSF_FREE(true_hdc->assembler.buffer);
        true_hdc->assembler.pts    = -1;
        true_hdc->assembler.pos    = 0;
        true_hdc->assembler.size   = 0;
        true_hdc->assembler.buffer = NULL;
    }
    MTAVSF_FREE(filter->priv_data);
    filter->priv_data = NULL;
}

/* Dolby TrueHD compatibility playback */
static MTAVStreamCodecFilter mtav_scf_dolby_truehdc = {
    .init     = scf_dolby_truehdc_init,
    .get_info = scf_dolby_truehdc_get_info,
    .filter   = scf_dolby_truehdc_filter,
    .flush    = scf_dolby_truehdc_flush,
    .deinit   = scf_dolby_truehdc_deinit,
};

#endif

#if MT_DES("MPEG 4 AUDIO", 1)
/*
 * Check whether the buf data is adts or not
 * There are some pkts all data is 0xff and not ADTS, so parse the data
 */
static int check_adts_frame_header(unsigned char *buf)
{
    unsigned char tmp = 0;
    /* ADTS sync word 0xFFF, buf NULL or not adts */
    if (!buf || buf[0] != 0xFF || ((buf[1] & 0xF0) != 0xF0)) {
        return MTAVSF_FAILURE;
    }

    /* There are some pkts all data is 0xff and not ADTS, so parse the data */
    /* buf[0,1]=0xFF F0 */
    tmp = buf[1] & 0x06;
    if (tmp != 0x0) { /* layer != 0 */
        return MTAVSF_FAILURE;
    }
    tmp = (buf[2] & 0xc0) >> 6;
    /* 0:main profile 1:lc 2:ssr 3:reserved */
    if (tmp != 1 && tmp != 0) {
        return MTAVSF_FAILURE;
    }
    tmp = (buf[2] & 0x3c) >> 2;
     /* sampRateIdx > NUM_SAMPLE_RATES */
    if (tmp > 12) {
        return MTAVSF_FAILURE;
    }
    tmp = (buf[2] & 0x1) << 2;
    tmp |= (buf[3] & 0xc0) >> 6;
    /* channelConfig >= NUM_DEF_CHAN_MAPS */
    if (tmp >= 8) {
        return MTAVSF_FAILURE;
    }

    return MTAVSF_SUCCESS;
}

static void gen_adts_fixed_header(MTAV_SCFAAC *aac, int data_size)
{
#define ADTS_SAMPLE_RATE_TABLE_LEN 16

    const int aac_sample_rates[ADTS_SAMPLE_RATE_TABLE_LEN] = {
        96000, 88200, 64000, 48000, 44100, 32000,
        24000, 22050, 16000, 12000, 11025, 8000 , 7350
    };
    unsigned char aac_buf[ADTS_FIXED_HEAD_LENGTH] =
        { 0xff, 0xf1, 0x40, 0x00, 0x00, 0x1f, 0xfc };

    int frame_length = data_size + ADTS_FIXED_HEAD_LENGTH;
    int aud_channels = aac->channels;
    unsigned int num_data_block = (unsigned int) data_size / 1024;

    int i;
    for (i = 0; i < ADTS_SAMPLE_RATE_TABLE_LEN; i++) {
        if (aac->sample_rate == aac_sample_rates[i]) {
            break;
        }
    }

    /* frame size over last 2 bits */
    aac_buf[2] |= ((i & 0xf) << 2);
    if (aud_channels > 3) {
        aac_buf[2] |= (aud_channels >> 2);
    }

    aac_buf[3] |= (aud_channels << 6);
    aac_buf[3] |= (frame_length & 0x1800) >> 11; // the upper 2 bit
    /* frame size continued over full byte */
    aac_buf[4] = (frame_length & 0x1FF8) >> 3;   // the middle 8 bit
    /* frame size continued first 3 bits */
    aac_buf[5] |= (frame_length & 0x7) << 5;     // the last 3 bit
    aac_buf[6] |= num_data_block & 0x03;         // Set raw Data blocks.

    memcpy(&(aac->header[0]), aac_buf, (unsigned int) ADTS_FIXED_HEAD_LENGTH);
}

static int scf_aac_init(MTAVStreamFilter *filter, void *init_info)
{
    if (!init_info) {
        return MTAVSF_FAILURE;
    }

    MTAV_SCFAAC *aac = (MTAV_SCFAAC *) filter->priv_data;
    if (!aac) {
        aac = MTAVSF_MALLOC(sizeof(MTAV_SCFAAC));
        if (!aac) {
            return MTAVSF_FAILURE;
        }
        memset((void *)aac, 0, sizeof(*aac));
        filter->priv_data = aac;
    }
    MTAVSAudioInfo *info = init_info;
    aac->channels    = info->channels;
    aac->sample_rate = info->sample_rate;
    MTAVSF_LOGA("[AAC] Init ok, ch:%d, sr:%d\n", info->channels, aac->sample_rate);
    return MTAVSF_SUCCESS;
}

static int scf_aac_get_info(MTAVStreamFilter *filter,
    MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info)
{
    (void) para;
    (void) pts;
    MTAVStreamBuffer *buffer = filter->stream_buffer;

    buffer->ibuffer = data;
    buffer->ibuffer_indx = 0;
    buffer->ibuffer_size = data_size;

    if (!filter->priv_data) {
        MTAVSF_LOGA("No aac private data\n");
        return MTAVSF_FAILURE;
    }

    MTAV_SCFAAC *aac = filter->priv_data;
    aac->raw = 0;
    /* ADTS sync word 0xFFF, MTAVSF_FAILURE means not adts */
    if (MTAVSF_SUCCESS !=
        check_adts_frame_header(data) &&
        0 != aac->channels && 0 != aac->sample_rate) {
        aac->raw = 1;
        gen_adts_fixed_header(aac, data_size);
    }

    buffer->obuffer = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = data_size + aac->raw * ADTS_FIXED_HEAD_LENGTH;
    *((unsigned int *) info) = buffer->obuffer_size;
    MTAVSF_LOGA("[AAC] Get info sz:%d out:%d raw:%d\n", data_size, buffer->obuffer_size, aac->raw);
    return MTAVSF_SUCCESS;
}

static int scf_aac_filter(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    MTAVStreamBuffer *buffer = filter->stream_buffer;
    unsigned int copy_size =
        MTAVSF_MIN(buffer->obuffer_size, out_buf_size);

    MTAV_SCFAAC *aac = filter->priv_data;
    unsigned char *obuffer = out_buf;

    if (aac->raw) {
        filter->memcp_cb(obuffer, &(aac->header[0]), ADTS_FIXED_HEAD_LENGTH);
        obuffer += ADTS_FIXED_HEAD_LENGTH;
        copy_size -= ADTS_FIXED_HEAD_LENGTH;
    }

    filter->memcp_cb(obuffer, buffer->ibuffer, copy_size);

    MTAVSF_LOGA("[AAC] Filter sz:%d out:%d raw:%d\n", buffer->obuffer_size, copy_size, out_buf_size);
    return (int) out_buf_size;
}

static void scf_aac_deinit(MTAVStreamFilter *filter)
{
    if (filter->priv_data) {
        MTAVSF_FREE(filter->priv_data);
        filter->priv_data = NULL;
    }
}

MTAVStreamCodecFilter mtav_scf_aac = {
	.init     = scf_aac_init,
	.get_info = scf_aac_get_info,
	.filter   = scf_aac_filter,
	.flush    = NULL,
	.deinit   = scf_aac_deinit,
};
#endif

#if MT_DES("Dump extra information media", 1)
static int dump_extradata_get_info(MTAV_SCFDumpExtra *dumper,
    unsigned char *data, unsigned int data_size, unsigned int keyframe, unsigned int *out_size)
{
    *out_size = data_size;
    dumper->keyframe = 0;
    if (dumper->extradata &&
        (dumper->frequency == MTAV_DUMP_FREQ_ALL ||
         (dumper->frequency == MTAV_DUMP_FREQ_KEYFRAME && keyframe)) &&
         data_size >= dumper->extradata_size &&
         memcmp(data, dumper->extradata, dumper->extradata_size)) {

        if (data_size >= INT_MAX - dumper->extradata_size) {
            return MTAVSF_FAILURE;
        }
        *out_size += dumper->extradata_size;
        dumper->keyframe = 1;
    }

    return MTAVSF_SUCCESS;
}

static int scf_dump_extradata_init(MTAVStreamFilter *filter, void *init_info)
{
    if (!init_info) {
        return MTAVSF_FAILURE;
    }

    MTAV_SCFDumpExtra *dumper = (MTAV_SCFDumpExtra *) filter->priv_data;
    if (!dumper) {
        dumper = MTAVSF_MALLOC(sizeof(MTAV_SCFDumpExtra));
        if (!dumper) {
            return MTAVSF_FAILURE;
        }
        memset((void *)dumper, 0, sizeof(*dumper));
        filter->priv_data = dumper;
    }

    if (dumper->extradata) {
        MTAVSF_FREE(dumper->extradata);
    }
    dumper->extradata = NULL;
    dumper->extradata_size = 0;
    MTAVSVideoInfo *info = (MTAVSVideoInfo *) init_info;
    /* The bytes of extra_data */
    if (!info->extradata || !info->extradata_size) {
        /* No need dump extra case */
        return MTAVSF_SUCCESS;
    }

    /* The extra data need in filter */
    dumper->extradata = (unsigned char *)
        MTAVSF_MALLOC(info->extradata_size + MTAVSF_PADDING_SIZE);
    if (!(dumper->extradata)) {
        MTAVSF_LOG("[Dump] Copy extra data malloc memory fail!\n");
        goto fail;
    }
    dumper->frequency = MTAV_DUMP_FREQ_KEYFRAME;
    memcpy(dumper->extradata, info->extradata, info->extradata_size);
    dumper->extradata_size = info->extradata_size;
    return MTAVSF_SUCCESS;
fail:
    dumper->extradata = NULL;
    dumper->extradata_size = 0;
    MTAVSF_FREE(filter->priv_data);
    filter->priv_data = NULL;
    return MTAVSF_FAILURE;
}

static int scf_dump_extradata_get_info(MTAVStreamFilter *filter,
    MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info)
{
    (void) pts;
    MTAVStreamBuffer *buffer = filter->stream_buffer;

    buffer->ibuffer = data;
    buffer->ibuffer_indx = 0;
    buffer->ibuffer_size = data_size;

    unsigned int out_size = 0;
    if (!filter->priv_data) {
        return MTAVSF_FAILURE;
    }
    MTAV_SCFDumpExtra *dumper = filter->priv_data;
    unsigned int keyframe = para ? (para->flags & MTAVSTREAM_FLAGS_KEYFRAME) : 0;
    if (MTAVSF_SUCCESS !=
        dump_extradata_get_info(dumper, data,
            data_size, keyframe, (unsigned int *) &out_size)) {
        *((unsigned int *) info) = 0;
        return MTAVSF_FAILURE;
    }

    buffer->obuffer = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = out_size;
    *((unsigned int *) info) = buffer->obuffer_size;
    return MTAVSF_SUCCESS;
}

static int scf_dump_extradata_filter(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    MTAV_SCFDumpExtra *dumper = filter->priv_data;
    MTAVStreamBuffer  *buffer = filter->stream_buffer;
    if (!dumper || !buffer->ibuffer || !buffer->ibuffer_size) {
        return MTAVSF_FAILURE;
    }

    unsigned char    *obuffer = out_buf;
    unsigned int obuffer_size = out_buf_size;
    if (out_buf_size < buffer->obuffer_size) {
        MTAVSF_LOG("[Dump] Not support output partion of data, Please provide integral buffer\n");
        return MTAVSF_FAILURE;
    }

    unsigned int total_push_size = 0;
    if (dumper->keyframe && dumper->extradata && dumper->extradata_size) {
        filter->memcp_cb(obuffer, dumper->extradata, dumper->extradata_size);
        obuffer        += dumper->extradata_size;
        obuffer_size   -= dumper->extradata_size;
        total_push_size = dumper->extradata_size;
    }

    unsigned int copy_size =
        MTAVSF_MIN(buffer->ibuffer_size, obuffer_size);
    filter->memcp_cb(obuffer, buffer->ibuffer, copy_size);
    total_push_size += copy_size;

    return total_push_size;
}

static void scf_dump_extradata_flush(MTAVStreamFilter *filter)
{
    if (!filter->priv_data) {
        return;
    }
    MTAV_SCFDumpExtra *dumper = filter->priv_data;
    dumper->keyframe = 0;
}

static void scf_dump_extradata_deinit(MTAVStreamFilter *filter)
{
    if (!filter->priv_data) {
        return;
    }

    MTAV_SCFDumpExtra *dumper = filter->priv_data;
    if (dumper->extradata) {
        MTAVSF_FREE(dumper->extradata);
        dumper->extradata = NULL;
        dumper->extradata_size = 0;
    }
    MTAVSF_FREE(filter->priv_data);
    filter->priv_data = NULL;
}

MTAVStreamCodecFilter mtav_scf_dump_extradata = {
	.init     = scf_dump_extradata_init,
	.get_info = scf_dump_extradata_get_info,
	.filter   = scf_dump_extradata_filter,
	.flush    = scf_dump_extradata_flush,
	.deinit   = scf_dump_extradata_deinit,
};
#endif

const char *mtlz_avfilter_stream_codec_name(unsigned int type)
{
    switch(type) {
        case MTAV_CODEC_AID_AAC:                return "AAC";
        case MTAV_CODEC_AID_AC4:                return "AC4";
        case MTAV_CODEC_AID_LPCM:               return "LPCM";
        case MTAV_CODEC_AID_DOLBY_TRUEHD:       return "DOLBY TRUEHD";
        case MTAV_CODEC_AID_AV3A:               return "AV3A";
        case MTAV_CODEC_VID_MPEG4:              return "MPEG4";
        case MTAV_CODEC_VID_H264:               return "H264";
        case MTAV_CODEC_VID_HEVC:               return "H265";
        case MTAV_CODEC_VID_VC1_WVC1:           return "VC1 WVC1";
        case MTAV_CODEC_VID_VC1_WMV3:           return "VC1 WMV3";
        case MTAV_CODEC_VID_VP8:                return "VP8";
        case MTAV_CODEC_VID_VP9:                return "VP9";
        case MTAV_CODEC_AID_BYPASS:             return "Normal Audio";
        case MTAV_CODEC_VID_BYPASS:             return "Normal Video";
    }
    return NULL;
}

const MTAVSF_MEDIA_TYPE_E mtlz_avfilter_get_media_type(unsigned int codec_id)
{
    MTAVSF_MEDIA_TYPE_E media_type = MTAVSF_MEDIA_TYPE_UNKNOWN;
    switch (codec_id) {
        case MTAV_CODEC_AID_AAC:
        case MTAV_CODEC_AID_AC4:
        case MTAV_CODEC_AID_LPCM:
        case MTAV_CODEC_AID_DOLBY_TRUEHD:
        case MTAV_CODEC_AID_AV3A:
        case MTAV_CODEC_AID_BYPASS: {
            media_type = MTAVSF_MEDIA_TYPE_AUDIO;
            break;
        }
        case MTAV_CODEC_VID_MPEG4:
        case MTAV_CODEC_VID_H264:
        case MTAV_CODEC_VID_HEVC:
        case MTAV_CODEC_VID_VC1_WVC1:
        case MTAV_CODEC_VID_VC1_WMV3:
        case MTAV_CODEC_VID_VP8:
        case MTAV_CODEC_VID_VP9:
        case MTAV_CODEC_VID_BYPASS: {
            media_type = MTAVSF_MEDIA_TYPE_VIDEO;
            break;
        }
        default:
            break;
    }
    return media_type;
}

static MTAVStreamCodecFilter *mtav_scf_list[] = {
    /* MTAV_CODEC_NONE                 */NULL,
    /* MTAV_CODEC_AID_AAC              */  &mtav_scf_aac,
    /* MTAV_CODEC_AID_AC4              */  &mtav_scf_ac4,
    /* MTAV_CODEC_AID_LPCM             */  &mtav_scf_lpcm,
    /* MTAV_CODEC_AID_DOLBY_TRUEHD     */  &mtav_scf_dolby_truehdc,
    /* MTAV_CODEC_AID_AV3A             */  &mtav_scf_default,
    /* MTAV_CODEC_VID_MPEG4            */  &mtav_scf_dump_extradata,
    /* MTAV_CODEC_VID_H264             */  &mtav_scf_h264,
    /* MTAV_CODEC_VID_H265             */  &mtav_scf_h265,
    /* MTAV_CODEC_VID_VC1_WVC1         */  &mtav_scf_vc1,
    /* MTAV_CODEC_VID_VC1_WMV3         */  &mtav_scf_vc1,
    /* MTAV_CODEC_VID_VP8              */  &mtav_scf_vpx,
    /* MTAV_CODEC_VID_VP9              */  &mtav_scf_vpx,
};

#if MT_DES("EXTERNAL API", 1)

#if MT_DES("ChinaDRM API", 1)
static inline int is_chinadrm_conflicts_data(unsigned char *data)
{
    unsigned int boundary = MRD_BE32(data);
    return (0x0300 == boundary || 0x0301 == boundary ||
            0x0302 == boundary || 0x0303 == boundary);
}

int mtav_stream_parse_chinadrm_cei_data(
    unsigned char *data,
    unsigned char data_size, chinadrm_encryption_data_info *cei)
{
    if (!data || !data_size || !cei) {
        return MTAVSF_FAILURE;
    }

    int offset = 0;
    cei->encryption_flag  = !!(data[offset] & 0x80);
    cei->next_key_id_flag = !!(data[offset] & 0x40);

    offset += 1;
    MTAVSF_LOGA("CEI data encryption_flag:%d next_key_id_flag:%d\n", cei->encryption_flag, cei->next_key_id_flag);
    if (cei->encryption_flag) {
        if (offset + DEFAULT_DRM_KEY_BYTES > data_size) {
            MTAVSF_LOG("CEI data size:(%d %d) not enough for curr_key_id\n", offset, data_size);
            return MTAVSF_FAILURE;
        }
        memcpy(cei->curr_key_id, &data[offset], DEFAULT_DRM_KEY_BYTES);
        offset += DEFAULT_DRM_KEY_BYTES;
    }
    if (cei->next_key_id_flag) {
        if (offset + DEFAULT_DRM_KEY_BYTES > data_size) {
            MTAVSF_LOG("CEI data size:(%d %d) not enough for next_key_id\n", offset, data_size);
            return MTAVSF_FAILURE;
        }
        memcpy(cei->next_key_id, &data[offset], DEFAULT_DRM_KEY_BYTES);
        offset += DEFAULT_DRM_KEY_BYTES;
    }
    if (offset + 1 > data_size) {
        MTAVSF_LOG("CEI data size:(%d %d) not enough for iv_length\n", offset, data_size);
        return MTAVSF_FAILURE;
    }
    cei->iv_length = data[offset++];
    if (cei->iv_length + offset > data_size) {
        MTAVSF_LOG("CEI data size:(%d %d) not enough for iv data\n", offset, data_size);
        return MTAVSF_FAILURE;
    }

    memcpy(cei->iv, &data[offset], cei->iv_length);
    return (offset + cei->iv_length);
}

int mtav_scf_decode_h2645_unregistered_user_data(
    unsigned char *data, int data_size, chinadrm_encryption_data_info *enc)
{
    if (data_size < 16) {
        return MTAVSF_FAILURE;
    }
    unsigned char uuid[16] = {
        0x70, 0xc1, 0xdb, 0x9f, 0x66, 0xae, 0x41, 0x27, 0xbf, 0xc0, 0xbb, 0x19, 0x81, 0x69, 0x4b, 0x66
    };

    if (memcmp(uuid, data, sizeof(uuid))) {
        return MTAVSF_FAILURE;
    }
    return mtav_stream_parse_chinadrm_cei_data(data + 16, data_size - 16, enc);
}

static int chinadrm_check_encrypted_data_conflicts(unsigned char *data, int data_size)
{
    int offset = 0;
    int conflitcts_cnt = 0;
    register unsigned int boundary = 0xFFFFff00;
    while (offset + NALU_START_CODE_LENGTH < data_size) {
        boundary |= data[offset];
        if (0x0300 != boundary && 0x0301 != boundary &&
            0x0302 != boundary && 0x0303 != boundary) {
            offset++;
            boundary <<= 8;
            continue;
        }

        conflitcts_cnt++;
        boundary = 0xFFFFff00;
        memmove(&data[offset - 1], &data[offset], data_size - offset);
    }
    if (conflitcts_cnt) {
        memset(&data[data_size - conflitcts_cnt], 0, conflitcts_cnt);
    }
    return conflitcts_cnt;
}

int mtav_scf_chinadrm_check_conflicts(
    unsigned char *data, int clear_data_size, int total_bytes)
{
    int conflitcts_cnt = 0;
    int offset = NALU_START_CODE_LENGTH;
    /* 1. keep clear data size unchange and add more zero before start code.
     *    we have at most 3 leading bytes belonging to clear data, so check
     *    one more bytes(0x00 0x00 0x03 | 0x03).
     */
    while (offset + NALU_START_CODE_LENGTH - 1 <= clear_data_size + conflitcts_cnt &&
           offset + NALU_START_CODE_LENGTH < total_bytes) {
        if (!is_chinadrm_conflicts_data(&data[offset])){
            offset++;
            continue;
        }
        conflitcts_cnt++;
        memmove(&data[1], data, offset + 2);
        offset += NALU_START_CODE_LENGTH;
    }

    (void) chinadrm_check_encrypted_data_conflicts(&data[offset], total_bytes - offset);
    return conflitcts_cnt;
}

#endif

int mtav_stream_append_record(MTAVStreamRecord *srec, MTAVStreamFilterRecord *rec)
{
    if (!srec || !rec) {        
        return MTAVSF_FAILURE;
    }
    int new_bytes = (srec->rec_count + 1) * sizeof(MTAVStreamFilterRecord);
    MTAVStreamFilterRecord *r = MTAVSF_REALLOC(srec->rec, new_bytes);
    if (!r) {        
        MTAVSF_LOG("%s memory malloc fail for %d bytes\n", __func__, new_bytes);
        return MTAVSF_FAILURE;
    }
    srec->rec = r;
    srec->rec[srec->rec_count].offset = rec->offset;    
    srec->rec[srec->rec_count].bytes_num_changed = rec->bytes_num_changed;
    srec->rec_count += 1;
    return MTAVSF_SUCCESS;
}

int mtav_stream_buffer_release(MTAVStreamBuffer *stream_buffer)
{
    if (!stream_buffer) {
        return MTAVSF_FAILURE;
    }

    if (stream_buffer->icb && stream_buffer->ibuffer) {
        stream_buffer->icb ((void *) stream_buffer->ibuffer);
        stream_buffer->ibuffer      = NULL;
        stream_buffer->ibuffer_indx = 0;
        stream_buffer->ibuffer_size = 0;
    }

    if (stream_buffer->ocb && stream_buffer->obuffer) {
        stream_buffer->ocb ((void *) stream_buffer->obuffer);
        stream_buffer->obuffer      = NULL;
        stream_buffer->obuffer_indx = 0;
        stream_buffer->obuffer_size = 0;
    }
    return MTAVSF_SUCCESS;
}

MTAVStreamFilter *mtlz_avfilter_stream_create(unsigned int codec_id)
{
    MTAVStreamFilter *filter =
		MTAVSF_MALLOC(sizeof(MTAVStreamFilter));

    if (!filter) {
        return NULL;
    }
    memset(filter, 0, sizeof(MTAVStreamFilter));
    filter->codec_id = codec_id;
    if (codec_id > MTAV_CODEC_VID_VP9) {
        filter->scf = &mtav_scf_default;
    } else {
        filter->scf = mtav_scf_list[codec_id];
    }

    return filter;
}

int mtlz_avfilter_stream_init(MTAVStreamFilter *filter, void *init_info)
{
    if (!filter || !filter->scf) {
        return MTAVSF_FAILURE;
    }
    /* No need init for some codec */
    if (!filter->scf->init) {
        return 0;
    }

    return filter->scf->init(filter, init_info);
}

int mtlz_avfilter_stream_get_info(MTAVStreamFilter *filter, MTAVStreamPara *para,
    unsigned char *data, long long pts, unsigned int data_size, void *info)
{
    if (!filter || !filter->scf || !filter->scf->get_info || !info) {
        return MTAVSF_FAILURE;
    }
    filter->pts = pts;
    return filter->scf->get_info(filter, para, data, pts, data_size, info);
}

int mtlz_avfilter_stream_filter(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    if (!filter || !filter->scf || !filter->scf->filter || !out_buf || !out_buf_size) {
        return MTAVSF_FAILURE;
    }

    return filter->scf->filter(filter, out_buf, out_buf_size);
}

int mtlz_avfilter_stream_flush(MTAVStreamFilter *filter)
{
    if (!filter || !filter->scf) {
        return MTAVSF_FAILURE;
    }
    /* No need flush for some codec */
    if (!filter->scf->flush) {
        return MTAVSF_SUCCESS;
    }

    filter->scf->flush(filter);
	return MTAVSF_SUCCESS;
}

void mtlz_avfilter_stream_release(MTAVStreamFilter *filter)
{
    if (!filter) {
        return;
    }
    if (filter->scf) {
        if (filter->scf->deinit) {
            filter->scf->deinit(filter);
        }
        filter->scf = NULL;
    }

    MTAVSF_FREE(filter);
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
