
/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __STREAM_FILTER_INTERNAL_H_H__
#define __STREAM_FILTER_INTERNAL_H_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#define MT_DES(a, b)                              (b)
#define MTAVSF_FREE                               free
#define MTAVSF_MALLOC                            malloc
#define MTAVSF_REALLOC                           realloc
#define MTAVSF_MAX(a, b)                  ((a) > (b) ? (a) : (b))
#define MTAVSF_MIN(a, b)                  ((a) < (b) ? (a) : (b))
#define MTAVSF_PADDING_SIZE                        64
#define DEFAULT_DRM_KEY_BYTES                      16
typedef enum {
    MTAVSF_MEDIA_TYPE_UNKNOWN = 0,
    MTAVSF_MEDIA_TYPE_AUDIO      ,
    MTAVSF_MEDIA_TYPE_VIDEO      ,
} MTAVSF_MEDIA_TYPE_E;

#define MTAVSF_LOG(fmt, ...)                                           \
do {                                                                   \
    printf(fmt, ##__VA_ARGS__);                                        \
} while (0)
#define MTAVSF_LOGA(fmt, ...)                                          \
do {                                                                   \
} while (0)

#ifndef __LINUX__
#include <Windows.h>
#define DRM_SMP_ENABLE
#define MT_DRM_SUPPORT
#define MTAVSF_MSLEEP(ms)          Sleep((ms))
#else
#include <unistd.h>
#define MTAVSF_MSLEEP(ms)          usleep((ms) * 1000)
#endif

#define MTAVSF_FW_FILE_HEADER  "SHANGHAIMONTSOC\x0"
#define MTAVSF_FW_FRAME_HEADER "SHANGHAIMONTSOC\x1"

#define MK_TAG32(a0, a1, a2, a3)                                       \
    ((unsigned int)(a0)        |                                       \
    ((unsigned int)(a1) << 8)  |                                       \
    ((unsigned int)(a2) << 16) |                                       \
    ((unsigned int)(a3) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

#define MWR_LE16(p, val)    {                                          \
    ((unsigned char *)(p))[0] = (unsigned char)((val) & 0xff);         \
    ((unsigned char *)(p))[1] = (unsigned char)(((val) >> 8) & 0xff);  \
}

#define MWR_LE32(p, val)    {                                          \
    ((unsigned char *)(p))[0] = (unsigned char)((val) & 0xff);         \
    ((unsigned char *)(p))[1] = (unsigned char)(((val) >> 8) & 0xff);  \
    ((unsigned char *)(p))[2] = (unsigned char)(((val) >> 16) & 0xff); \
    ((unsigned char *)(p))[3] = (unsigned char)(((val) >> 24) & 0xff); \
}

#define MRD_BE16(p)                                                    \
    (((unsigned short)(((const unsigned char *)(p))[0]) << 8) |        \
      (unsigned short)(((const unsigned char *)(p))[1]))
#define MRD_BE24(p)                                                    \
    (((unsigned int)(((const unsigned char *)(p))[0]) << 16)  |        \
     ((unsigned int)(((const unsigned char *)(p))[1]) <<  8)  |        \
      (unsigned int)(((const unsigned char *)(p))[2]))
#define MRD_BE32(p)                                                    \
    (((unsigned int) MRD_BE16(p) << 16)                       |        \
     ((unsigned int) MRD_BE16((uintptr_t)(p) + 2)))
#define MWR_BE32(p, val)    {                                          \
    ((unsigned char *)(p))[0] = (unsigned char)(((val) >> 24) & 0xff); \
    ((unsigned char *)(p))[1] = (unsigned char)(((val) >> 16) & 0xff); \
    ((unsigned char *)(p))[2] = (unsigned char)(((val) >> 8) & 0xff);  \
    ((unsigned char *)(p))[3] = (unsigned char)((val) & 0xff);         \
}

#define MTAVSF_SWAP(T, a, b) \
do {                         \
    T temp = (a);            \
    (a) = (b);               \
    (b) = temp;              \
} while (0)

typedef void (*MEM_FREE_CB) (void *mem);
typedef struct _MTAVStreamBuffer MTAVStreamBuffer;
typedef struct  _MTAVStreamFilterRecord MTAVStreamFilterRecord;

struct _MTAVStreamBuffer {
    /* input buffer for filter, refere raw es data from get_info,
     * set in get_info, reset in filter data function.
     * keep NULL when clear stream in crypted system
     * keep descrypted data when encrypted stream in crypted system
     */
    unsigned char *ibuffer;
    unsigned int   ibuffer_indx;
    unsigned int   ibuffer_size;
    MEM_FREE_CB    icb;
    /* output buffer for filter, managed by stream codec filter.
     * below case need output buffer
     * 1: filter out_buf_size smaller than filtered raw size
     * 2: drm case
     */
    unsigned char *obuffer;
    unsigned int   obuffer_indx;
    /* Set by codec get_info function */
    unsigned int   obuffer_size;
    MEM_FREE_CB    ocb;
};

struct _MTAVStreamFilterRecord {
    int offset;
    int bytes_num_changed;
};

typedef struct _MTAVStreamRecord {
    unsigned int rec_count;
    MTAVStreamFilterRecord *rec;
    unsigned int output_bytes;
} MTAVStreamRecord;

#if MT_DES("CHINADRM", 1)
#define MAX_CHINADRM_IV_BYTES                      256
/* china drm content encryption information */
typedef struct chinadrm_encryption_data_info {
    unsigned char encryption_flag;
    unsigned char next_key_id_flag;
    unsigned char curr_key_id[DEFAULT_DRM_KEY_BYTES];
    unsigned char next_key_id[DEFAULT_DRM_KEY_BYTES];
    unsigned char iv_length;
    unsigned char iv[MAX_CHINADRM_IV_BYTES];
    unsigned short bytes_of_clear_data;
    unsigned int bytes_of_encrypted_data;
    unsigned int output_bytes;
} chinadrm_encryption_data_info;
#endif

#if MT_DES("AAC", 1)
#define ADTS_FIXED_HEAD_LENGTH        7
typedef struct {
    /* 1:raw aac es, 0:adts header + raw aac es */
    unsigned char raw;
    unsigned int channels;
    unsigned int sample_rate;
    unsigned char header[ADTS_FIXED_HEAD_LENGTH];
} MTAV_SCFAAC;
#endif

#if MT_DES("AC4", 1)
#define AC4_MAX_HEAD_LENGTH        7
typedef struct {
    /* raw ac-4, has no sync word and frame size */
    unsigned char raw;
    unsigned char frame_size_field_len;
} MTAV_SCFAC4;
#endif

#if MT_DES("DOLBY TRUEHD AUDIO", 1)
typedef struct {
    struct {
        unsigned int   pos;
        unsigned int   size;
        long long      pts;
        unsigned char *buffer;
    } assembler;
    long long pts;
    unsigned int frame_size;
    unsigned int sample_rate;
    unsigned int state;
} MTAV_SCFDolbyTrueHDC;
#endif

#if MT_DES("DUMP EXTRADATA", 1)
enum DumpFrequency {
    MTAV_DUMP_FREQ_NONE,
    MTAV_DUMP_FREQ_KEYFRAME,
    MTAV_DUMP_FREQ_ALL,
};

typedef struct {
    unsigned int frequency;
    unsigned int keyframe;
    /* The bytes of extra_data */
    unsigned int extradata_size;
    /* The extra data need in filter */
    unsigned char *extradata;
} MTAV_SCFDumpExtra;
#endif

#if MT_DES("VC1", 1)
#define VC1_LEVEL_LOW           0
#define VC1_LEVEL_MEDIUM        2
#define VC1_LEVEL_HIGH          4
#define ROUNDUP(x, y)          (((x) + (y) - 1) & ~((y) - 1))

enum Profile {
    VC1_PROFILE_SIMPLE,
    VC1_PROFILE_MAIN,
    VC1_PROFILE_COMPLEX, ///< TODO: WMV9 specific
    VC1_PROFILE_ADVANCED
};
enum VC1Code {
    VC1_CODE_RES0       = 0x00000100,
    VC1_CODE_ENDOFSEQ   = 0x0000010A,
    VC1_CODE_SLICE,
    VC1_CODE_FIELD,
    VC1_CODE_FRAME,
    VC1_CODE_ENTRYPOINT,
    VC1_CODE_SEQHDR,
};

struct VC1_STRUCT_SEQUENCE_HEADER_A {
    int ver_size;
    int horz_size;
};

struct VC1_STRUCT_SEQUENCE_HEADER_B {
    /*
    * [0,  2    ] -> [Low, Medium      ] for the simple profile.
    * [0,  2,  4] -> [Low, Medium, High] for the main profile.
    * [0,1,2,3,4] -> [L0 through L4    ] For the advanced profile
    */
    unsigned int level               : 3; /* specify the encoding level */
    unsigned char cbr                : 1; /* 1:cbr,0:not */
    unsigned char res1               : 4; /* shall be set to zero */
    union {
        struct {
            unsigned int hrd_buffer  : 24; /* buffer size B in milliseconds.is given by(HRD_BUFFER * HRD_RATE)/ 1000 */
            unsigned int hrd_rate    : 32; /* specify the peak transmission rate R in bits per second. */
        } s_m;
        struct {
            unsigned char adv[7];          /* shall be set to zero */
        } adv;
    };
    unsigned int framerate           : 32; /* rounded frame rate.is 0xffffffff if unknown,unspecified,or non-constant */
};

struct VC1_STRUCT_SEQUENCE_HEADER_C {
    unsigned int profile                 : 4; /* 0,4 or 12 indicate Simple,Main,Advanced, only 2 bits in SEQUENCE LAYER */
    union {
        struct {
            unsigned int frmrtq_postproc : 3; /* Quantized Frame Rate for Post processing Indicator */
            unsigned int bitrtq_postproc : 5; /* Quantized Bit Rate for Post processing Indicator */
            unsigned int loopfilter      : 1; /* use loop filtering 1:yew, 0:no */
            unsigned int reserved3       : 1; /* shall be set to zero */
            unsigned int multires        : 1; /* indicate whether frames coded at smaller resolutions than the specified */
            unsigned int reserved4       : 1; /* shall be set to zero */
            unsigned int fastuvmc        : 1; /* control the subpixel interpolation and rounding of color-difference motion vectors */
            unsigned int extended_mv     : 1; /* indicate whether extended motion vectors are enabled (value 1) or disabled (value 0) */
            unsigned int dquant          : 2; /* dindicate whether or not the quantization step size may vary within a frame */
            unsigned int vstransform     : 1; /* indicate whether variable-sized transform coding is enabled */
            unsigned int reserved5       : 1; /* shall be set to zero */
            unsigned int overlap         : 1; /* indicate whether Overlapped Transforms  are used */
            unsigned int syncmarker      : 1; /* indicate whether synchronization markers may be present */
            unsigned int rangered        : 1; /* indicate whether range reduction is used for each frame */
            unsigned int maxbframes      : 3; /* Maximum Number of consecutive B frames between I or P frames */
            unsigned int quantizer       : 2; /* indicate the quantizer used for the sequence */
            unsigned int finterpflag     : 1; /* indicate whether INTERPFRM is present in the picture header */
            unsigned int reserved6       : 1; /* shall be set to zero */
        } s_m;
        struct {
            unsigned int reserved7       : 28; /* shall be set to zero */
        } adv;
    } u;
};
#define IS_VC1_MARKER(x)       (((x) & ~0xFF) == VC1_CODE_RES0)
#define MAX_VC1_HEADER_SIZE              (1024)
typedef struct {
    MTAVSVideoInfo vinfo;
    unsigned char header[MAX_VC1_HEADER_SIZE];
    unsigned int  header_size;
} MTAV_SCFVC1;

#endif

#if MT_DES("H264", 1)
#define MAX_EXTRADATA_SIZE          2048
#define NALU_START_CODE_LENGTH       4
enum {
    H264_NAL_UNSPECIFIED     = 0,
    H264_NAL_SLICE           = 1,
    H264_NAL_DPA             = 2,
    H264_NAL_DPB             = 3,
    H264_NAL_DPC             = 4,
    H264_NAL_IDR_SLICE       = 5,
    H264_NAL_SEI             = 6,
    H264_NAL_SPS             = 7,
    H264_NAL_PPS             = 8,
    H264_NAL_AUD             = 9,
    H264_NAL_END_SEQUENCE    = 10,
    H264_NAL_END_STREAM      = 11,
    H264_NAL_FILLER_DATA     = 12,
    H264_NAL_SPS_EXT         = 13,
    H264_NAL_PREFIX          = 14,
    H264_NAL_SUB_SPS         = 15,
    H264_NAL_DPS             = 16,
    H264_NAL_RESERVED17      = 17,
    H264_NAL_RESERVED18      = 18,
    H264_NAL_AUXILIARY_SLICE = 19,
    H264_NAL_EXTEN_SLICE     = 20,
    H264_NAL_DEPTH_EXTEN_SLICE = 21,
    H264_NAL_RESERVED22      = 22,
    H264_NAL_RESERVED23      = 23,
    H264_NAL_UNSPECIFIED24   = 24,
    H264_NAL_UNSPECIFIED25   = 25,
    H264_NAL_UNSPECIFIED26   = 26,
    H264_NAL_UNSPECIFIED27   = 27,
    H264_NAL_UNSPECIFIED28   = 28,
    H264_NAL_UNSPECIFIED29   = 29,
    H264_NAL_UNSPECIFIED30   = 30,
    H264_NAL_UNSPECIFIED31   = 31,
};

typedef struct {
    unsigned char *sps;
    unsigned char *pps;
    unsigned char  nalu_length_bytes_nb;
    unsigned char  new_idr;
    unsigned char  idr_sps_seen;
    unsigned char  idr_pps_seen;
    unsigned char *extradata;
    int sps_size;
    int pps_size;
    int extradata_size;
    int extradata_parsed;
    int is_annex_b;
    int is_annex_b_init; /* annex b flag from init context */
    int keyframe;
    int send_extra_annexb_header;
} MTAV_SCFH264;
#endif

#if MT_DES("H265", 1)
#define MIN_HEVCC_LENGTH                  23
/* Assume max frame size 10 megabytes  */
#define MAX_FRAME_SIZE             (10 * 1024 * 1024)
/**
 * Table 7-1 NAL unit type codes and NAL unit type classes in
 * T-REC-H.265-201802
 */
enum HEVCNALUnitType {
    HEVC_NAL_TRAIL_N    = 0,

    HEVC_NAL_BLA_W_LP   = 16,
    HEVC_NAL_IRAP_VCL23 = 23,

    HEVC_NAL_RSV_VCL30  = 30,
    HEVC_NAL_RSV_VCL31  = 31,
    HEVC_NAL_VPS        = 32,
    HEVC_NAL_SPS        = 33,
    HEVC_NAL_PPS        = 34,

    HEVC_NAL_SEI_PREFIX = 39,
    HEVC_NAL_SEI_SUFFIX = 40,
};

typedef struct {
    int extradata_parsed;
    int extradata_size;

    unsigned char *extradata;
    unsigned char  nalu_length_bytes_nb;
    int is_annex_b;
    int is_annex_b_init; /* annex b flag from init context */
} MTAV_SCFHEVC;
#endif

#if MT_DES("VPX", 1)
#define VPX_START_CODE_SIZE         16
#define VPX_FILE_HEADER_SIZE        48
#define VPX_FRAME_HEAER_SIZE        28
#define VPX_MAX_SUPER_FRAME_NUM     8

typedef struct _SCFVp9SuperframeInfo {
    int superframe_num;
    int size[VPX_MAX_SUPER_FRAME_NUM];
} SCFVp9SuperframeInfo;

typedef struct {
    MTAVSVideoInfo vinfo;
    SCFVp9SuperframeInfo superframe_info;
    unsigned char insert_file_header;
} MTAV_SCFVPX;

const char *mtlz_avfilter_stream_codec_name(unsigned int type);
const MTAVSF_MEDIA_TYPE_E mtlz_avfilter_get_media_type(unsigned int codec_id);

void mtav_scf_write_vpx_file_header(
    MTAV_SCFVPX *vpx, unsigned int codec_id, unsigned char *buff);
void mtav_scf_write_vpx_frame_header(int pkt_size,
    unsigned long long pkt_pts, unsigned char *out_data);
int mtav_scf_decode_h2645_unregistered_user_data(
    unsigned char *data, int data_size, chinadrm_encryption_data_info *enc);
int mtav_scf_chinadrm_check_conflicts(
    unsigned char *data, int clear_data_size, int total_bytes);

int mtav_stream_buffer_release(MTAVStreamBuffer *stream_buffer);
int mtav_stream_parse_chinadrm_cei_data(
    unsigned char *data,
    unsigned char data_size, chinadrm_encryption_data_info *cei);
int mtav_stream_append_record(MTAVStreamRecord *srec, MTAVStreamFilterRecord *rec);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __STREAM_FILTER_INTERNAL_H_H__ */
