/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <string.h>
#include "stdint.h"
#ifdef __LINUX__
#include "mt_common.h"
#endif
#include "mtlz_avfilter_stream.h"
#include "stream_filter_drm.h"
#include "stream_filter.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct _MTAV_CSFDrmFilter MTAV_CSFDrmFilter;
#if MT_DES("Test function Definition", 0)
static int drm_decrypt_data(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    DMTRM_BUFFER_IN *data_in, DMTRM_BUFFER_OUT *data_out);

static const unsigned char CLEARKEY_SYSTEM_ID[SYSTEM_ID_SIZE_DEFAULT] = {
    0x10, 0x77, 0xef, 0xec, 0xc0, 0xb2, 0x4d, 0x02, 0xac, 0xe3, 0x3c, 0x1e, 0x52, 0xe2, 0xfb, 0x4b
};

static DMTRM_BUFFER_IN  *get_global_drm_in_buffer()
{
#define TEST_DATA_LEN    37
    static mt_u8 iv[16]              = {0x92, 0xaa, 0x90, 0x3a, 0x24, 0x09, 0x7d, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
    static mt_u8 data[TEST_DATA_LEN] = {0x00, 0x00, 0x00, 0x21, 0x41, 0x9a, 0xc0, 0x1a, 0xf0, 0xdf, 0x87, 0x33, 0x39, 0xdb, 0x12, 0xb6, 0x5d, 0x23, 0xff, 0x5e, 0x62, 0xf9, 0x0b, 0x11, 0x04, 0x3b, 0xd3, 0x47, 0x14, 0x46, 0xa5, 0x91, 0x3d, 0xaa, 0x43, 0x4e, 0x4a, };
    static mt_u16 clear[1]           = {21};
    static mt_u32 encrypt[1]         = {16};
    static DMTRM_BUFFER_IN  g_data_in = {
         .iv   = &iv[0],
         .data = &data[0],
         .data_length = TEST_DATA_LEN,
         .clear       = &clear[0],
         .encrypt     = &encrypt[0],
         .region_count   = 1,
         .key_index      = 0,
         .patternEncrypt = 0,
         .patternClear   = 0
    };
    return &g_data_in;
}
#endif
extern MTAVStreamCodecFilter mtav_scf_h264_chinadrm;
extern MTAVStreamCodecFilter mtav_scf_h265_chinadrm;

static const unsigned char PLAYREADY_SYSTEM_ID[SYSTEM_ID_SIZE_DEFAULT] = {
    0x9a, 0x04, 0xf0, 0x79, 0x98, 0x40, 0x42, 0x86, 0xab, 0x92, 0xe6, 0x5b, 0xe0, 0x88, 0x5f, 0x95
};
static const unsigned char WIDEVINE_SYSTEM_ID[SYSTEM_ID_SIZE_DEFAULT] = {
    0xed, 0xef, 0x8b, 0xa9, 0x79, 0xd6, 0x4a, 0xce, 0xa3, 0xc8, 0x27, 0xdc, 0xd5, 0x1d, 0x21, 0xed
};
static const unsigned char CHINADRM_SYSTEM_ID[SYSTEM_ID_SIZE_DEFAULT] = {
    0x3d, 0x5e, 0x6d, 0x35, 0x9b, 0x9a, 0x41, 0xe8, 0xb8, 0x43, 0xdd, 0x3c, 0x6e, 0x72, 0xc4, 0x2c
};
/* Verimatrix Conditional Access System:VCAS */
static const unsigned char VCAS_SYSTEM_ID[SYSTEM_ID_SIZE_DEFAULT] = {
    0x9a, 0x27, 0xdd, 0x82, 0xfd, 0xe2, 0x47, 0x25, 0x8c, 0xbc, 0x42, 0x34, 0xaa, 0x06, 0xec, 0x09
};

#define DRM_TYPE_CHINADRM   (DRM_TYPE_WIDEVINE + 1)
#define DRM_TYPE_VERIMATRIX (DRM_TYPE_CHINADRM + 1)

struct _MTAV_CSFDrmFilter {
    int type;
    int index;
    /* current handled data is enctypted or not */
    unsigned int encrypted;
    /* encrypted smp need secure memory, use hw filter, otherwise NULL */
    int (*parse_cb)(MTAVStreamFilter *, MTAV_CSFDrmFilter *, unsigned char *, unsigned int, unsigned int *);
    int (*filter_cb)(MTAVStreamFilter *, MTAV_CSFDrmFilter *, unsigned char *, unsigned int);
    union {
        MTAV_SCFAAC  aac;
        MTAV_SCFH264 h264;
        MTAV_SCFHEVC h265;
        MTAV_SCFVPX  vpx;
        MTAVSAudioInfo ainfo;
        MTAVSVideoInfo vinfo;
    } codec;
    MTAVStreamFilter *priv_filter;
};

static void drm_get_fixed_protection_info(
    MTAV_CSFDrmFilter *drm, MTDRM_PROTECT_INFO *info)
{
#if MT_DES("Fixed debug inforamtion", 0)
    static mt_u8 init_data[] =
        {
            0x41, 0x41, 0x41, 0x41, 0x52, 0x48, 0x42, 0x7a, 0x63, 0x32,
            0x67, 0x41, 0x41, 0x41, 0x41, 0x41, 0x37, 0x65, 0x2b, 0x4c,
            0x71, 0x58, 0x6e, 0x57, 0x53, 0x73, 0x36, 0x6a, 0x79, 0x43,
            0x66, 0x63, 0x31, 0x52, 0x30, 0x68, 0x37, 0x51, 0x41, 0x41,
            0x41, 0x43, 0x51, 0x49, 0x41, 0x52, 0x49, 0x42, 0x4e, 0x52,
            0x6f, 0x4e, 0x64, 0x32, 0x6c, 0x6b, 0x5a, 0x58, 0x5a, 0x70,
            0x62, 0x6d, 0x56, 0x66, 0x64, 0x47, 0x56, 0x7a, 0x64, 0x43,
            0x49, 0x4b, 0x4d, 0x6a, 0x41, 0x78, 0x4e, 0x56, 0x39, 0x30,
            0x5a, 0x57, 0x46, 0x79, 0x63, 0x79, 0x6f, 0x43, 0x55, 0x30,
            0x51, 0x3d, 0x00
        };
    static mt_char kids[][16] = {
        {
            0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
            0x30, 0x30, 0x30, 0x30, 0x30, 0x36
        },
    };
    /* Not video */
    if (drm->index != 0) {
        return;
    }
    /* change if need */
    info->is_pssh = 0;
    info->init_data = &init_data[0];
    info->init_data_len = sizeof(init_data) / sizeof(init_data[0]) - 1;
    info->kid_count = sizeof(kids) / sizeof(kids[0]);
    info->kids[0] = kids[0];
#endif
}

const unsigned char *get_system_name(int type)
{
    switch(type) {
        case DRM_TYPE_PLAYREADY:
            return "Playready";
        case DRM_TYPE_WIDEVINE:
            return "Widevine";
        case DRM_TYPE_CHINADRM:
            return "ChinaDrm";
        case DRM_TYPE_VERIMATRIX:
            return "Verimatrix";
        default:
            return "";
    }
}

static int drm_system_new(MTAV_CSFDrmFilter *drm, MTDRM_CONFIG *cfg)
{
    drm->index = -1;
    int ret = MTDrm_GetDrmInstance((MTDRM_TYPE) drm->type, cfg, &drm->index);
    MTAVSF_LOG("[DRM] Get %s instance %d\n", get_system_name(drm->type), drm->index);
    if (MTDRM_SUCCESS != ret) {
        MTAVSF_LOG("[DRM] Cannot get Drm Instance\n");
        return MTAVSF_FAILURE;
    }
    return MTAVSF_SUCCESS;
}

static void drm_system_free(MTAV_CSFDrmFilter *drm)
{
    if (-1 == drm->index) {
        return;
    }

    (void) MTDrm_ReleaseDrmInstance((MTDRM_TYPE) drm->type, drm->index);
}

/*
* DASH Content Protection using Microsoft PlayReady
* From:https://learn.microsoft.com/zh-cn/playready/specifications/mpeg-dash-playready
*    KID Parameter                         Type                                                                                    Representation
* 1. KID                         UUID BE Hex Number                                                                                                                           f81d4fae7dec11d0a76500a0c91e6bf6
* 2. cenc:default_KID attribute  UUID Hex String with hyphens                                                                                                                "f81d4fae-7dec-11d0-a765-00a0c91e6bf6"
* 3. KID in ISOBFF boxes         UUID BE Byte Array                                      Hex representation is                                                               { 0xf8, 0x1d, 0x4f, 0xae, 0x7d, 0xec, 0x11, 0xd0, 0xa7, 0x65, 0x00, 0xa0, 0xc9, 0x1e, 0x6b, 0xf6 }
* 4. KID in PRO                  Base64 String of GUID LE Byte Array                     "rk8d+Ox90BGnZQCgyR5r9g=="(Hex representation of the data before Base64 encoding is { 0xae, 0x4f, 0x1d, 0xf8, 0xec, 0x7d, 0xd0, 0x11, 0xa7, 0x65, 0x00, 0xa0, 0xc9, 0x1e, 0x6b, 0xf6 })
* 5. mspr:kid                    Base64 String of default_KID Byte Array in 'tenc' box   "+B1Prn3sEdCnZQCgyR5r9g=="(Hex representation of the data before Base64 encoding is { 0xf8, 0x1d, 0x4f, 0xae, 0x7d, 0xec, 0x11, 0xd0, 0xa7, 0x65, 0x00, 0xa0, 0xc9, 0x1e, 0x6b, 0xf6 })
* 6. KID in PlayReady license    GUID LE Byte Array                                      Hex representation is                                                               { 0xae, 0x4f, 0x1d, 0xf8, 0xec, 0x7d, 0xd0, 0x11, 0xa7, 0x65, 0x00, 0xa0, 0xc9, 0x1e, 0x6b, 0xf6 } OK to use
* Unless there is a change, the client must convert the endianness of the KIDs byte array in order to match it to the PlayReady license.
*
* 1 eg.  KID in PRO, No need swap for this type, but we not support this type.
* <PROTECTINFO>
*   <KIDS>
*     <KID ALGID="AESCBC" VALUE="PV1LM/VEVk+kEOB8qqcWDg=="></KID>
*     <KID ALGID="AESCBC" VALUE="tuhDoKUN7EyxDPtMRNmhyA=="></KID>
*   </KIDS>
* </PROTECTINFO>
*/
static void convert_playready_kid(MTDRM_PROTECT_INFO *info)
{
    for (int i = 0; i < info->kid_count; i++) {
        if (!info->kids[i]) {
            continue;
        }
        /* convert the endianness of the KIDs byte array in order to match it to the PlayReady license. */
        /* Swap the endianness of the GUID value: */

        /* - Reverse bytes 0 to 3, */
        MTAVSF_SWAP(mt_char, info->kids[i][0], info->kids[i][3]);
        MTAVSF_SWAP(mt_char, info->kids[i][1], info->kids[i][2]);
        /* - swap bytes 4 and 5, */
        MTAVSF_SWAP(mt_char, info->kids[i][4], info->kids[i][5]);
        /* - swap bytes 6 and 7, */
        MTAVSF_SWAP(mt_char, info->kids[i][6], info->kids[i][7]);
        /* - keep bytes 8-15 as-is without swapping */
    }
}
static int drm_system_set_protection_info(
    MTAV_CSFDrmFilter *drm, MTAV_CSFDrmFilterInitInfo *info)
{
    if (-1 == drm->index) {
        return MTAVSF_FAILURE;
    }

    if (DRM_TYPE_PLAYREADY == drm->type && info->info.kids && info->info.kid_count) {
        convert_playready_kid(&info->info);
    }

    MTDRM_PROTECT_INFO protection_info = info->info;
    drm_get_fixed_protection_info(drm, &protection_info);
   // mlzp_show_encryption_init_infomation(drm->index, &protection_info);
    int ret = MTDrm_SetProtectInfo(&protection_info, drm->index);
    if (MTDRM_SUCCESS != ret) {
        MTAVSF_LOG("[DRM] Set protect info fail\n");
        return MTAVSF_FAILURE;
    }

    return MTAVSF_SUCCESS;
}

static int drm_system_decrypt_sample_data(MTAV_CSFDrmFilter *drm,
    DMTRM_BUFFER_IN *data_in, DMTRM_BUFFER_OUT *data_out)
{
    if (-1 == drm->index) {
        return MTAVSF_FAILURE;
    }
    // mlzp_show_encryption_sample_infomation(drm->index, data_in);
    int ret = MTDrm_DecryptSampleData(data_in, data_out, drm->index);
    if (MTDRM_SUCCESS != ret) {
        MTAVSF_LOG("[DRM] decrypt sample data fail\n");
        return MTAVSF_FAILURE;
    }
    return MTAVSF_SUCCESS;
}

#if MT_DES("Support Key Callback", 0)
static void drm_system_license_key_status_changed_cb(
   const CdmKeyStatusType *status, void *context)
{
    if (!context || !status) {
        return;
    }

    MTAVSTREAM_MESSAGES msg = MTAVSTREAM_MSG_NONE;
    MTAVStreamFilter *filter = context;
    if (CDM_kExpired == *status) {
        msg = MTAVSTREAM_MSG_CDM_KEXPIRED;

        DMTRM_BUFFER_OUT  data_out = {0,};
        MTAV_CSFDrmFilter *drm = filter->priv_data;
        if (filter->msg_cb) {
            filter->msg_cb (filter->opaque, msg, NULL);
        }
    }
}

static int drm_system_set_key_status_callback(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm)
{
    int ret = 0;
    /* receive from cdm 0 */
    if (drm->index != 0) {
        return 0;
    }

    ret = MTDrm_SetKeyStatus_callback(drm->index,
       drm_system_license_key_status_changed_cb, (void *) filter);

    return ret;
}
#endif

#if defined(DRM_SMP_ENABLE)
static int drm_filter_obuffer(MTAVStreamFilter *filter,
    MTAVStreamBuffer *buffer, unsigned char *out_buf, unsigned int out_buf_size);
#if MT_DES("DRM SYS INTERFACE", 1)
static inline void *ves_dmacpy(void *restrict dst, const void *restrict src, size_t size) {
    int ret = MTDrm_DMACopy(DMTRM_EDMA_CH_1, dst, src, (unsigned int) size);
    if (MTDRM_SUCCESS != ret) {
        MTAVSF_LOG("[DRM]ves_dmacpy fail, dst:%p, src:%p, size:%d\n", dst, src, (unsigned int) size);
    }
    return NULL;
}

static inline void *aes_dmacpy(void *restrict dst, const void *restrict src, size_t size) {
    int ret = MTDrm_DMACopy(DMTRM_EDMA_CH_0, dst, src, (unsigned int) size);
    if (MTDRM_SUCCESS != ret) {
        MTAVSF_LOG("[DRM] aes_dmacpy fail, dst:%p, src:%p, size:%d\n", dst, src, (unsigned int) size);
    }
    return NULL;
}

static int drm_system_set_decrypt_type(
    MTAV_CSFDrmFilter *drm, unsigned int enable)
{
    int ret = MTAVSF_SUCCESS;
    if (-1 == drm->index) {
        return MTAVSF_FAILURE;
    }

    ret = MTDrm_SetDecryptType(drm->index, enable);
    return ret;
}

static int drm_system_secure_memory_malloc(
    MTAV_CSFDrmFilter *drm, DMTRM_BUFFER_OUT *data_out, unsigned int size)
{
    if (-1 == drm->index) {
        return MTAVSF_FAILURE;
    }

    int ret = MTDrm_MallocSecureMemory(data_out, (size_t) size);
    if(MTDRM_SUCCESS != ret) {
        MTAVSF_LOG("[DRM] Malloc secure memory %d failed!\n", size);
        return MTAVSF_FAILURE;
    }

    return MTAVSF_SUCCESS;
}

static int secure_memory_free(DMTRM_BUFFER_OUT *data)
{
    if (!data) {
        return MTAVSF_FAILURE;
    }
    int ret = MTDrm_FreeSecureMemory(data);
    if(MTDRM_SUCCESS != ret) {
        return MTAVSF_FAILURE;
    }

    return MTAVSF_SUCCESS;
}

static void drm_system_secure_memory_free(void *arg)
{
    if (!arg) {
        return;
    }
    DMTRM_BUFFER_OUT data = {
        .data = arg,
        .is_secure = 1,
    };

    (void) secure_memory_free(&data);
}

#endif

#if MT_DES("DRM FILTER FUNCTION", 1)
#define  MAX_VIDEO_FILTER_SIZE  4096
static int copy_clear_stream(MTAVStreamFilter *filter,
    unsigned char *inbuf, unsigned char *out_buf, unsigned int copy_size)
{
    filter->memcp_cb(out_buf, inbuf, copy_size);
    return MTAVSF_SUCCESS;
}
#if MT_DES("DRM AAC", 1)
static int drm_parse_aac(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *data, unsigned int data_size, unsigned int *info)
{
    int ret = MTAVSF_FAILURE;

    *info = 0;
    MTAV_SCFAAC *aac = drm->priv_filter->priv_data;
    MTAVStreamBuffer *buffer  = filter->stream_buffer;
    DMTRM_BUFFER_OUT data_out =  {0};

    int sbuf_size = data_size + ADTS_FIXED_HEAD_LENGTH + MTAVSF_PADDING_SIZE;
    ret = drm_system_secure_memory_malloc(drm, &data_out, sbuf_size);
    if(MTAVSF_SUCCESS != ret) {
        return MTAVSF_FAILURE;
    }
    /* The function not return data size, maybe error */
    ret = MTDrm_AddAAHeader(data, data_size,
        aac->sample_rate, aac->channels, data_out.data);
    if (MTAVSF_SUCCESS != ret) {
        drm_system_secure_memory_free(data_out.data);
        return MTAVSF_FAILURE;
    }
    buffer->ocb     = drm_system_secure_memory_free;
    buffer->obuffer = data_out.data;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = data_size + ADTS_FIXED_HEAD_LENGTH;
    *info = buffer->obuffer_size;

    return MTAVSF_SUCCESS;
}

static int drm_filter_aac(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    (void) drm;
    MTAVStreamBuffer *buffer  = filter->stream_buffer;
    return drm_filter_obuffer(filter, buffer, out_buf, out_buf_size);
}
#endif

#if MT_DES("DRM AC4", 1)
static int drm_parse_ac4(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *data, unsigned int data_size, unsigned int *info)
{
    int ret = MTAVSF_FAILURE;

    *info = 0;
    MTAV_SCFAC4 *ac4 = drm->priv_filter->priv_data;
    MTAVStreamBuffer *buffer  = filter->stream_buffer;
    DMTRM_BUFFER_OUT data_out =  {0};

    int sbuf_size = data_size + AC4_MAX_HEAD_LENGTH + MTAVSF_PADDING_SIZE;
    ret = drm_system_secure_memory_malloc(drm, &data_out, sbuf_size);
    if(MTAVSF_SUCCESS != ret) {
        return MTAVSF_FAILURE;
    }

    int out_data_size = sbuf_size;
    /* The function not return data size, maybe error */
    ret = MTDrm_AddAC4Header(data, data_size, data_out.data, &out_data_size);
    if (MTAVSF_SUCCESS != ret) {
        drm_system_secure_memory_free(data_out.data);
        return MTAVSF_FAILURE;
    }
    buffer->ocb     = drm_system_secure_memory_free;
    buffer->obuffer = data_out.data;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = out_data_size;
    *info = buffer->obuffer_size;

    return MTAVSF_SUCCESS;
}

static int drm_filter_ac4(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    (void) drm;
    MTAVStreamBuffer *buffer  = filter->stream_buffer;
    return drm_filter_obuffer(filter, buffer, out_buf, out_buf_size);
}
#endif

#if MT_DES("DRM H264", 1)
static int drm_parse_h264(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *data, unsigned int data_size, unsigned int *info)
{
    int ret = MTAVSF_FAILURE;

    *info = 0;

    MTAV_SCFH264 *h264 = drm->priv_filter->priv_data;
    MTAVStreamBuffer *buffer  = filter->stream_buffer;
    DMTRM_BUFFER_OUT data_out =  {0};

    int sbuf_size = data_size + MAX_VIDEO_FILTER_SIZE + MTAVSF_PADDING_SIZE;
    ret = drm_system_secure_memory_malloc(drm, &data_out, sbuf_size);
    if(MTAVSF_SUCCESS != ret) {
        return MTAVSF_FAILURE;
    }

    int odata_size = sbuf_size;
    ret = MTDrm_FilterAvcSample(data, data_size, h264->extradata,
        h264->extradata_size, h264->nalu_length_bytes_nb, data_out.data, &odata_size);
    if (MTAVSF_SUCCESS != ret) {
        drm_system_secure_memory_free(data_out.data);
        return MTAVSF_FAILURE;
    }

    buffer->ocb     = drm_system_secure_memory_free;
    buffer->obuffer = data_out.data;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = odata_size;
    *info = buffer->obuffer_size;

    return MTAVSF_SUCCESS;
}

static int drm_filter_h264(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    (void) drm;
    MTAVStreamBuffer *buffer  = filter->stream_buffer;
    return drm_filter_obuffer(filter, buffer, out_buf, out_buf_size);
}
#endif

#if MT_DES("DRM H265", 1)
static int drm_parse_h265(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *data, unsigned int data_size, unsigned int *info)
{
    int ret = MTAVSF_FAILURE;

    *info = 0;

    MTAV_SCFHEVC *h265 = drm->priv_filter->priv_data;
    MTAVStreamBuffer *buffer  = filter->stream_buffer;
    DMTRM_BUFFER_OUT data_out =  {0};

    int sbuf_size = data_size + MAX_VIDEO_FILTER_SIZE + MTAVSF_PADDING_SIZE;
    ret = drm_system_secure_memory_malloc(drm, &data_out, sbuf_size);
    if(MTAVSF_SUCCESS != ret) {
        return MTAVSF_FAILURE;
    }

    int odata_size = sbuf_size;
    ret = MTDrm_FilterHevcSample(data, data_size, h265->extradata,
        h265->extradata_size, h265->nalu_length_bytes_nb, data_out.data, &odata_size);
    if (MTAVSF_SUCCESS != ret) {
        drm_system_secure_memory_free(data_out.data);
        return MTAVSF_FAILURE;
    }

    buffer->ocb     = drm_system_secure_memory_free;
    buffer->obuffer = data_out.data;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = odata_size;
    *info = buffer->obuffer_size;

    return MTAVSF_SUCCESS;
}
static int drm_filter_h265(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    (void) drm;
    MTAVStreamBuffer *buffer  = filter->stream_buffer;
    return drm_filter_obuffer(filter, buffer, out_buf, out_buf_size);
}
#endif

#if MT_DES("DRM VPX", 1)
static int filter_vpx_check_frame(
    MTAV_SCFVPX *vpx, MTAVStreamBuffer *buffer, int *split)
{
    *split = 0;
    /* New frame start addr */
    if (!buffer->ibuffer_indx) {
        return 0;
    }

    int i = 0;
    unsigned int total_pkt_size = 0;
    for (i = 0; i < vpx->superframe_info.superframe_num; i++) {
        int pkt_size = vpx->superframe_info.size[i];
        /* Subframe has been splitted */
        if (total_pkt_size < buffer->ibuffer_indx &&
            (total_pkt_size + pkt_size) > buffer->ibuffer_indx) {
            *split = pkt_size - (buffer->ibuffer_indx - total_pkt_size);
            break;
        } else if (total_pkt_size == buffer->ibuffer_indx) {
            break;
        }
        total_pkt_size += pkt_size;
    }
    return i;
}

static int drm_parse_vpx(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *data, unsigned int data_size, unsigned int *info)
{
    int ret = MTAVSF_FAILURE;

    *info = 0;
    MTAV_SCFVPX *vpx = &drm->codec.vpx;
    vpx->superframe_info.superframe_num = 1;
    vpx->superframe_info.size[0] = data_size;
    if (MTAV_CODEC_VID_VP9 == filter->codec_id) {
        ret = MTDrm_ParseVp9SuperframeInfo(
            data, data_size, &vpx->superframe_info);
        if (MTAVSF_SUCCESS != ret) {
            return MTAVSF_FAILURE;
        }
    }
    unsigned int total_size = vpx->insert_file_header ? VPX_FILE_HEADER_SIZE : 0;
    for (int i = 0; i < drm->codec.vpx.superframe_info.superframe_num; i++) {
        total_size += VPX_FRAME_HEAER_SIZE + drm->codec.vpx.superframe_info.size[i];
    }

    MTAVStreamBuffer *buffer  = filter->stream_buffer;
    *info = total_size;
    buffer->obuffer = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = total_size;

    return MTAVSF_SUCCESS;
}

static int drm_filter_vpx(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    int i;
    unsigned char *obuffer;
    unsigned int obuffer_size;
    unsigned int total_push_size = 0;
    MTAV_SCFVPX *vpx = &drm->codec.vpx;
    MTAVStreamBuffer *buffer = filter->stream_buffer;
    unsigned char hdr[VPX_FILE_HEADER_SIZE + VPX_FILE_HEADER_SIZE] = {0};

    obuffer = out_buf;
    obuffer_size = out_buf_size;
    if (!drm->encrypted) {
        vpx = drm->priv_filter->priv_data;
        buffer = drm->priv_filter->stream_buffer;
    }

    if (vpx->insert_file_header) {
        if (obuffer_size <= VPX_FILE_HEADER_SIZE + VPX_FRAME_HEAER_SIZE) {
            if (obuffer_size > 0) {
                memset(hdr, 0, obuffer_size);
                filter->memcp_cb(obuffer, hdr, obuffer_size);
            }
            goto finish;
        }
        mtav_scf_write_vpx_file_header(vpx, filter->codec_id, hdr);
        filter->memcp_cb(obuffer, hdr, VPX_FILE_HEADER_SIZE);
        obuffer += VPX_FILE_HEADER_SIZE;
        obuffer_size -= VPX_FILE_HEADER_SIZE;
        total_push_size += VPX_FILE_HEADER_SIZE;
    }

    int split = 0;
    for (i = filter_vpx_check_frame(vpx, buffer, &split);
        i < vpx->superframe_info.superframe_num; i++) {
        int pkt_size = vpx->superframe_info.size[i];

        if (split) {
            MTAVSF_LOG("[%d]Buffer split, total:%d remain:%d\n", drm->encrypted, pkt_size, split);
            pkt_size = split;
            split = 0;
        } else {
            if (obuffer_size <= VPX_FRAME_HEAER_SIZE) {
                if (obuffer_size > 0) {
                    memset(hdr, 0, obuffer_size);
                    filter->memcp_cb(obuffer, hdr, obuffer_size);
                }
                goto finish;
            }
            mtav_scf_write_vpx_frame_header(pkt_size, filter->pts, hdr);
            filter->memcp_cb(obuffer, hdr, VPX_FRAME_HEAER_SIZE);
            obuffer += VPX_FRAME_HEAER_SIZE;
            obuffer_size -= VPX_FRAME_HEAER_SIZE;
            total_push_size += VPX_FRAME_HEAER_SIZE;
        }

        unsigned int copy_size =
            MTAVSF_MIN(obuffer_size, (unsigned int) pkt_size);
        filter->memcp_cb(obuffer, &buffer->ibuffer[buffer->ibuffer_indx], copy_size);
        obuffer         += copy_size;
        obuffer_size    -= copy_size;
        total_push_size += copy_size;
        buffer->ibuffer_indx += copy_size;
        buffer->ibuffer_size -= copy_size;
    }
finish:
    return total_push_size;
}

#endif
static int drm_parse_general(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *data, unsigned int data_size, unsigned int *info)
{
    (void) drm;
    (void) data;
    MTAVStreamBuffer *buffer = filter->stream_buffer;

    buffer->obuffer = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = data_size;
    *((unsigned int *) info) = data_size;
    return MTAVSF_SUCCESS;
}

static int drm_filter_general(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    unsigned int copy_size;
    MTAVStreamBuffer *buffer = filter->stream_buffer;

    if (drm->encrypted) {
        copy_size = MTAVSF_MIN(out_buf_size, (buffer->ibuffer_size));
        filter->memcp_cb(out_buf, &buffer->ibuffer[buffer->ibuffer_indx], copy_size);
    } else {
        buffer = drm->priv_filter->stream_buffer;
        copy_size = MTAVSF_MIN(out_buf_size, (buffer->ibuffer_size));
        copy_clear_stream(filter, &buffer->ibuffer[buffer->ibuffer_indx], out_buf, copy_size);
    }
    buffer->ibuffer_indx += copy_size;
    buffer->ibuffer_size -= copy_size;
    return copy_size;
}

static int drm_filter_verimatrix(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *out_buf, unsigned int out_buf_size)
{

    MTAVSF_MEMCP_FUNC memcp_cb = filter->memcp_cb;
    filter->memcp_cb = (MTAVSF_MEDIA_TYPE_AUDIO ==
        mtlz_avfilter_get_media_type(filter->codec_id)) ? aes_dmacpy : ves_dmacpy;

    int ret = drm_filter_general(filter, drm, out_buf, out_buf_size);
    filter->memcp_cb = memcp_cb;
    return ret;
}
#endif /* DRM FILTER FUNCTION */
#else
/* stub function for minimize macro use */
static int drm_system_set_decrypt_type(
    MTAV_CSFDrmFilter *drm, unsigned int enable)
{
    (void) drm;
    (void) enable;
    return MTAVSF_SUCCESS;
}

static int drm_system_secure_memory_malloc(
    MTAV_CSFDrmFilter *drm, DMTRM_BUFFER_OUT *data_out, unsigned int size)
{
    (void) drm;
    (void) data_out;
    (void) size;
    return MTAVSF_SUCCESS;
}

static void drm_system_secure_memory_free(void *arg)
{
    (void) arg;
}
#endif /* DRM_SMP_ENABLE */

#if MT_DES("Internal function Definition", 1)

#if MT_DES("Test function Definition", 0)
static int drm_mmz_new(unsigned int size,
    unsigned char **phyaddr, unsigned char **viraddr)
{
        *phyaddr = (unsigned char *) mt_mmz_new(size, 64, NULL, "AVI PUSH");
    if (!*phyaddr) {
        return MTAVSF_FAILURE;
    }
    *viraddr = (unsigned char *) mt_mmz_map((mt_u32)((uintptr_t)(*phyaddr)), 0);
    if (!*viraddr) {
        mt_mmz_delete(*phyaddr);
        *phyaddr = NULL;
        return MTAVSF_FAILURE;
    }
    return MTAVSF_SUCCESS;
}

static void drm_mmz_free(unsigned char **phyaddr)
{
    if (!phyaddr || !*phyaddr) {
        return;
    }
    mt_mmz_unmap(*phyaddr);
    mt_mmz_delete(*phyaddr);
    *phyaddr = NULL;
}
#endif

static void inline drm_stream_buffer_reset(MTAVStreamBuffer *buffer)
{
    if (!buffer) {
        return;
    }

    mtav_stream_buffer_release(buffer);
    buffer->icb          = NULL;
    buffer->ibuffer      = NULL;
    buffer->ibuffer_indx = 0;
    buffer->ibuffer_size = 0;

    buffer->ocb          = NULL;
    buffer->obuffer      = NULL;
    buffer->obuffer_indx = 0;
    buffer->obuffer_size = 0;
}

static void drm_stream_buffer_free(MTAVStreamFilter *filter)
{
    MTAV_CSFDrmFilter *drm = filter->priv_data;
    MTAVStreamBuffer *buffer = filter->stream_buffer;

    if (!buffer) {
        return;
    }

    drm_stream_buffer_reset(buffer);
    drm->encrypted = 0;
}

static int drm_stream_obufer_new(MTAVStreamFilter *filter)
{
    MTAV_CSFDrmFilter *drm = filter->priv_data;
    MTAVStreamBuffer *buffer = filter->stream_buffer;

    if (buffer->obuffer) {
        MTAVSF_LOG("[DRM] Error stream obuffer not free!\n");
    }

    int ret = MTAVSF_SUCCESS;

    buffer->ocb = NULL;
    /* drm ecrypted filter need secure memory */
    if (drm->encrypted && drm->filter_cb) {
        if (buffer->obuffer_size == buffer->ibuffer_size) {
            buffer->obuffer = buffer->ibuffer;
            return MTAVSF_SUCCESS;
        }
        DMTRM_BUFFER_OUT  data_out = {0,};
        data_out.is_secure = 1;
        ret = drm_system_secure_memory_malloc(drm, &data_out, buffer->obuffer_size);
        if (MTAVSF_SUCCESS == ret) {
            buffer->ocb     = drm_system_secure_memory_free;
            buffer->obuffer = data_out.data;
        }

        return ret;
    }
    buffer->ocb = MTAVSF_FREE;
    buffer->obuffer = MTAVSF_MALLOC(buffer->obuffer_size);
    ret = buffer->obuffer ? MTAVSF_SUCCESS : MTAVSF_FAILURE;
    return ret;
}

static void drm_priv_filter_destroy(MTAVStreamFilter **filter)
{
    if (!filter || !*filter) {
        return;
    }
    /* No need release private obuffer because it use current filter obuffer */
    if ((*filter)->stream_buffer) {
        drm_stream_buffer_reset((*filter)->stream_buffer);
        MTAVSF_FREE((*filter)->stream_buffer);
        (*filter)->stream_buffer = NULL;
    }
    mtlz_avfilter_stream_release(*filter);
    *filter = NULL;
}

static int drm_priv_filter_create(MTAVStreamFilter *filter)
{
    MTAV_CSFDrmFilter *drm = filter->priv_data;

    if (!drm->priv_filter) {
        drm->priv_filter =
            mtlz_avfilter_stream_create(filter->codec_id);
        if (!drm->priv_filter) {
            return MTAVSF_FAILURE;
        }
        MTAVStreamBuffer *buffer =
            MTAVSF_MALLOC(sizeof(MTAVStreamBuffer));
        if (!buffer) {
            mtlz_avfilter_stream_release(drm->priv_filter);
            drm->priv_filter = NULL;
            return MTAVSF_FAILURE;
        }

        memset(buffer, 0,  sizeof(*buffer));
        drm->priv_filter->stream_buffer = buffer;
        drm->priv_filter->memcp_cb = filter->memcp_cb;
    }
    return mtlz_avfilter_stream_init(drm->priv_filter, &drm->codec);
}

static int drm_filter_config(MTAVStreamFilter *filter, void *init_info, unsigned char insert_file_header)
{
    MTAV_CSFDrmFilter *drm = filter->priv_data;
    MTAV_CSFDrmFilterInitInfo *info = init_info;

    drm->parse_cb  = NULL;
    drm->filter_cb = NULL;
    filter->memcp_cb = memcpy;
    memcpy(&drm->codec, &info->codec.video, sizeof(drm->codec));
#if defined(DRM_SMP_ENABLE)
    if (DRM_TYPE_VERIMATRIX == drm->type) {
        drm->parse_cb  = NULL;
        drm->filter_cb = drm_filter_verimatrix;
    } else {
        drm->parse_cb  = drm_parse_general;
        drm->filter_cb = drm_filter_general;
        filter->memcp_cb = ves_dmacpy;
        if (MTAV_CODEC_AID_AAC == filter->codec_id ||
            MTAV_CODEC_AID_AC4 == filter->codec_id ||
            MTAV_CODEC_AID_BYPASS == filter->codec_id) {
            filter->memcp_cb = aes_dmacpy;
            if (MTAV_CODEC_AID_AAC == filter->codec_id) {
                drm->parse_cb  = drm_parse_aac;
                drm->filter_cb = drm_filter_aac;
            } else if (MTAV_CODEC_AID_AC4 == filter->codec_id) {
                drm->parse_cb  = drm_parse_ac4;
                drm->filter_cb = drm_filter_ac4;
            } else {
                return MTAVSF_SUCCESS;
            }
        } else if (MTAV_CODEC_VID_VP8 == filter->codec_id ||
            MTAV_CODEC_VID_VP9 == filter->codec_id) {
            drm->parse_cb  = drm_parse_vpx;
            drm->filter_cb = drm_filter_vpx;
            drm->codec.vpx.insert_file_header = insert_file_header;
            return MTAVSF_SUCCESS;
        } else if (MTAV_CODEC_VID_H264 == filter->codec_id) {
            drm->parse_cb  = drm_parse_h264;
            drm->filter_cb = drm_filter_h264;
        } else if (MTAV_CODEC_VID_HEVC == filter->codec_id) {
            drm->parse_cb  = drm_parse_h265;
            drm->filter_cb = drm_filter_h265;
        }
    }
#endif
    /* h264,h264 need filter parse clear extra data */
    if (MTAVSF_SUCCESS != drm_priv_filter_create(filter)) {
        return MTAVSF_FAILURE;
    }
    return MTAVSF_SUCCESS;
}

static int drm_decrypt_data(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    DMTRM_BUFFER_IN *data_in, DMTRM_BUFFER_OUT *data_out)
{
    int ret = MTAVSF_FAILURE;

    MTAVStreamBuffer *buffer = filter->stream_buffer;
    if (drm->filter_cb) {
        data_out->is_secure = 1;
        drm_system_set_decrypt_type(drm, 0);
        ret = drm_system_secure_memory_malloc(drm, data_out, data_in->data_length);
        if (MTAVSF_SUCCESS == ret) {
            buffer->icb = drm_system_secure_memory_free;
        }
    } else {
        buffer->icb = MTAVSF_FREE;
        data_out->data = MTAVSF_MALLOC(data_in->data_length);
        ret = data_out->data ? MTAVSF_SUCCESS : MTAVSF_FAILURE;
    }
    if(MTAVSF_SUCCESS != ret) {
        MTAVSF_LOG("[DRM] Malloc secure memory failed!\n");
        data_out->data = NULL;
        return MTAVSF_FAILURE;
    }

    drm->encrypted  = 1;
    buffer->ibuffer = data_out->data;
    buffer->ibuffer_size = data_in->data_length;
    ret = drm_system_decrypt_sample_data(drm, data_in, data_out);
    drm_system_set_decrypt_type(drm, 1);
    return ret;
}

static int drm_parse_clear_data(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    MTAVStreamPara *para,  unsigned char *data, unsigned int data_size, void *info)
{
    int ret = MTAVSF_SUCCESS;

    if (!drm || !filter) {
        return MTAVSF_FAILURE;
    }

    if (!drm->priv_filter) {
        if (MTAVSF_SUCCESS !=
            drm_priv_filter_create(filter)) {
            return MTAVSF_FAILURE;
        }
    }

    if (!drm->priv_filter) {
        return MTAVSF_FAILURE;
    }

    *(unsigned int *) info = data_size;
    drm_stream_buffer_reset(drm->priv_filter->stream_buffer);
    /* output 0 for some codec */
    if (!data) {
        MTAVStreamBuffer *buffer = filter->stream_buffer;
        buffer->obuffer = MTAVSF_MALLOC(data_size);
        if (!buffer->obuffer) {
            MTAVSF_LOG("[DRM] Allocate last buffer:%d fail\n", data_size);
            return MTAVSF_FAILURE;
        }
        buffer->ocb = MTAVSF_FREE;
        memset(buffer->obuffer, 0, data_size);
        buffer->obuffer_indx = 0;
        buffer->obuffer_size = data_size;
        return MTAVSF_SUCCESS;
    }

    ret = mtlz_avfilter_stream_get_info(
        drm->priv_filter, para, data, filter->pts, data_size, info);
    if (MTAVSF_SUCCESS != ret) {
        return ret;
    }

    return MTAVSF_SUCCESS;
}

static int drm_parse_entrypted_data(
    MTAVStreamFilter *filter, MTAV_CSFDrmFilter *drm,
    unsigned char *data, unsigned int data_size, unsigned int *info)
{
    int ret = MTAVSF_SUCCESS;

    *info = data_size;
    if (drm->parse_cb && drm->encrypted) {
        ret = drm->parse_cb(filter, drm, data, data_size, info);
    } else {
        ret = mtlz_avfilter_stream_get_info(drm->priv_filter,
            NULL, data, filter->pts, data_size, info);
    }

    if (MTAVSF_SUCCESS != ret) {
        goto parse_fail;
    }

    return MTAVSF_SUCCESS;
parse_fail:
    drm_stream_buffer_free(filter);
    return MTAVSF_FAILURE;
}

static int drm_filter_data(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int copy_size)

{
    int ret;

    MTAV_CSFDrmFilter *drm = filter->priv_data;

    if (drm->encrypted && drm->filter_cb) {
        ret = drm->filter_cb(filter, drm, out_buf, copy_size);
        return ret;
    }
    ret = mtlz_avfilter_stream_filter(drm->priv_filter, out_buf, copy_size);
    return ret;
}
static int drm_filter_obuffer(MTAVStreamFilter *filter,
    MTAVStreamBuffer *buffer, unsigned char *out_buf, unsigned int out_buf_size)
{
    if (!buffer->obuffer) {
        return MTAVSF_FAILURE;
    }

    MTAV_CSFDrmFilter *drm = filter->priv_data;
    MTAVSF_MEMCP_FUNC memcp_cb = filter->memcp_cb;
#if defined(DRM_SMP_ENABLE)
    if ((DRM_TYPE_VERIMATRIX == drm->type)) {
        filter->memcp_cb = (MTAVSF_MEDIA_TYPE_AUDIO ==
            mtlz_avfilter_get_media_type(filter->codec_id)) ? aes_dmacpy : ves_dmacpy;
    }
#endif
    unsigned int copy_size = MTAVSF_MIN(out_buf_size, buffer->obuffer_size);
    // MTAVSF_LOG("[%d] Copy data from back memory:%p size:%d idx:%d o:%p\n",
    //    drm->index, buffer->obuffer, copy_size, buffer->obuffer_indx, out_buf);
    filter->memcp_cb(out_buf, &buffer->obuffer[buffer->obuffer_indx], copy_size);
    buffer->obuffer_indx += copy_size;
    buffer->obuffer_size -= copy_size;
    if (0 == buffer->obuffer_size) {
        drm_stream_buffer_free(filter);
    }
    filter->memcp_cb = memcp_cb;
    return copy_size;
}

static int set_drm_system_type(MTAV_CSFDrmFilter *drm, MTAV_CSFDrmFilterInitInfo *info)
{
    if (SYSTEM_ID_SIZE_DEFAULT == info->system_id_size) {
        info->info.is_pssh = 1;
        if (!memcmp(info->system_id, PLAYREADY_SYSTEM_ID, SYSTEM_ID_SIZE_DEFAULT)) {
            drm->type = DRM_TYPE_PLAYREADY;
        } else if (!memcmp(info->system_id, WIDEVINE_SYSTEM_ID, SYSTEM_ID_SIZE_DEFAULT)) {
            drm->type = DRM_TYPE_WIDEVINE;
        } else if (!memcmp(info->system_id, CHINADRM_SYSTEM_ID, SYSTEM_ID_SIZE_DEFAULT)) {
            drm->type = DRM_TYPE_CHINADRM;
        } else if (!memcmp(info->system_id, VCAS_SYSTEM_ID, SYSTEM_ID_SIZE_DEFAULT)) {
            drm->type = DRM_TYPE_VERIMATRIX;
        } else {
            MTAVSF_LOG("[DRM] Init failure: Not recognized pssh system id\n");
            return MTAVSF_FAILURE;
        }
        return MTAVSF_SUCCESS;
    }

    if ((SYSTEM_ID_SIZE_CONTENT_PROTECTION == info->system_id_size)) {
        if (!memcmp(info->system_id, PLAYREADY_CPRO_SYSTEM_ID, SYSTEM_ID_SIZE_CONTENT_PROTECTION)) {
            drm->type = DRM_TYPE_PLAYREADY;
        } else if (!memcmp(info->system_id, WIDEVINE_CPRO_SYSTEM_ID, SYSTEM_ID_SIZE_CONTENT_PROTECTION)) {
            drm->type = DRM_TYPE_WIDEVINE;
        } else if (!memcmp(info->system_id, CHINADRM_CPRO_SYSTEM_ID, SYSTEM_ID_SIZE_CONTENT_PROTECTION)) {
                drm->type = DRM_TYPE_CHINADRM;
        } else if (!memcmp(info->system_id, VERIMATRIX_CPRO_SYSTEM_ID, SYSTEM_ID_SIZE_CONTENT_PROTECTION)) {
                drm->type = DRM_TYPE_VERIMATRIX;
        } else {
            MTAVSF_LOG("[DRM] Init failure: Not recognized content protection system id\n");
            return MTAVSF_FAILURE;
        }
        return MTAVSF_SUCCESS;
    }

    MTAVSF_LOG("[DRM] Init failure: Unkown system id size\n");
    return MTAVSF_FAILURE;
}

static int need_parse_encryption_info(MTAVStreamFilter *filter, DMTRM_BUFFER_IN *data_in)
{
    MTAV_CSFDrmFilter *drm = filter->priv_data;

    if (DRM_TYPE_CHINADRM != drm->type || 1 != data_in->region_count) {
        return 0;
    }

    unsigned char invalid_drm_data[16] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    };
    if (memcmp(data_in->iv, invalid_drm_data, 16)) {
        return 0;
    }

    return 1;
}

static void drm_priv_filter_reset_scf(MTAVStreamFilter *filter)
{
    MTAV_CSFDrmFilter *drm = filter->priv_data;
    if (DRM_TYPE_CHINADRM != drm->type) {
        return;
    }

    if (MTAV_CODEC_VID_H264 == filter->codec_id) {
        drm->priv_filter->scf = &mtav_scf_h264_chinadrm;
    } else if (MTAV_CODEC_VID_HEVC == filter->codec_id) {
        drm->priv_filter->scf = &mtav_scf_h265_chinadrm;
    }
}

static int chinadrm_check_encrypted_data_padding_bytes(
    unsigned char *data, chinadrm_encryption_data_info *enc)
{
    int padding = 0;
    int offset  = enc->bytes_of_encrypted_data - 1;
    while (offset >= 0 && 0 == data[offset]) {
        offset--;
        padding++;
    }
    return padding;
}

static int chinadrm_change_encryption_info(MTAVStreamFilter *filter,
    MTAVStreamPara *para,  unsigned char *data, unsigned int *data_size, DMTRM_BUFFER_IN *data_in, int *encrypted)
{
    MTAV_CSFDrmFilter *drm = filter->priv_data;

    *encrypted = 0;
    MTAVStreamCodecFilter *scf = drm->priv_filter->scf;
    drm_priv_filter_reset_scf(filter);
    chinadrm_encryption_data_info enc = {0, {0}};
    int ret = mtlz_avfilter_stream_get_info(
        drm->priv_filter, para, data, filter->pts, *data_size, &enc);
    if (MTAVSF_SUCCESS != ret) {
        goto done;
    }

    if (0 == enc.encryption_flag) {
        ret = MTAVSF_SUCCESS;
        goto done;
    }

    MTAVSF_LOGA("[DRM] data size:%d enc:%d\n", data_size, !(0 == enc.iv_length || 0 == enc.encryption_flag));
    if (0 == enc.iv_length || 0 == enc.encryption_flag) {
        ret = MTAVSF_FAILURE;
        goto done;
    }

    if (enc.iv_length > 16) {
        unsigned char *iv =
            MTAVSF_REALLOC(data_in->iv, enc.iv_length);
        if (!iv) {
            ret = MTAVSF_FAILURE;
            goto done;
        }
        data_in->iv = iv;
    }
    memcpy(data_in->iv, &enc.iv[0], enc.iv_length);
    /*
     * 0x0:NONE  0x1:SM4-SAMPLE(PARTY) 0x2:CBCS(PARTY)  0x3:SM4-CBC(FULL) 0x5:AES-CBC(FULL)
     * (encryptedBlocks,clearBlocks) FULL(2,0), PARTY(3,9)
     */
    if (MKBETAG('S','M','4','C') == data_in->scheme ||
        MKBETAG('C','B','C','1') == data_in->scheme) {
        data_in->patternEncrypt = 2;
        data_in->patternClear   = 0;
    } else if (
        MKBETAG('S','M','4','S') == data_in->scheme ||
        MKBETAG('C','B','C','S') == data_in->scheme){
        data_in->patternEncrypt = 3;
        data_in->patternClear   = 9;
    }

    *encrypted = 1;
    int padding_bytes   = chinadrm_check_encrypted_data_padding_bytes(&data[enc.bytes_of_clear_data], &enc);
    data_in->clear[0]   = enc.bytes_of_clear_data;
    data_in->encrypt[0] = enc.bytes_of_encrypted_data - padding_bytes;
    *data_size = *data_size - padding_bytes;
#if defined(DRM_SMP_ENABLE)
    drm->parse_cb  = drm_parse_general;
    drm->filter_cb = drm_filter_general;
#endif
    MTAVSF_LOGA("[DRM] data size:%d enc:%d clear:%d encrypted:%d pattern(%d %d)\n",
        *data_size, !(0 == enc.iv_length || 0 == enc.encryption_flag), (int)enc.bytes_of_clear_data,
        (int)enc.bytes_of_encrypted_data, (int)(int)data_in->patternEncrypt, (int)data_in->patternClear);
done:
    drm->priv_filter->scf = scf;
    return ret;
}

static int verimatrix_get_encrypted_info(MTAVStreamFilter *filter,
    MTAVStreamPara *para, unsigned char *data, unsigned int data_size, DMTRM_BUFFER_OUT *data_out, void *info)
{
    int ret = MTAVSF_FAILURE;
    MTAV_CSFDrmFilter *drm = filter->priv_data;
    if (MTAV_CODEC_AID_AAC  != filter->codec_id &&
        MTAV_CODEC_VID_HEVC != filter->codec_id &&
        MTAV_CODEC_VID_H264 != filter->codec_id) {
        MTAVSF_LOG("%s Not support codec id\n", __func__);
        return ret;
    }

    int tmp_buffer_size = 0;
    unsigned char *tmp_buffer = NULL;
    DMTRM_BUFFER_IN *data_in = para->cfg;
    MTAVStreamRecord srec = {
        .rec          = NULL,
        .rec_count    = 0,
        .output_bytes = 0,
    };
    if (MTAV_CODEC_AID_AAC == filter->codec_id) {
        ret = mtlz_avfilter_stream_get_info(
                drm->priv_filter, para, data, filter->pts, data_size, &tmp_buffer_size);
        if (data_in->region_count) {
            data_in->clear[0] += (tmp_buffer_size - data_size);
        }
    } else {
        MTAVStreamCodecFilter *scf = drm->priv_filter->scf;
        drm_priv_filter_reset_scf(filter);
        ret = mtlz_avfilter_stream_get_info(
                drm->priv_filter, para, data, filter->pts, data_size, &srec);
        drm->priv_filter->scf = scf;
        if (MTAVSF_SUCCESS != ret) {
            goto done;
        }
        tmp_buffer_size = srec.output_bytes;
    }

    tmp_buffer = MTAVSF_MALLOC(tmp_buffer_size + MTAVSF_PADDING_SIZE);
    if (!tmp_buffer) {
        MTAVSF_LOG("%s No memory:%d error\n", __func__, tmp_buffer_size);
        goto done;
    }
    ret = mtlz_avfilter_stream_filter(drm->priv_filter, tmp_buffer, tmp_buffer_size);
    if (tmp_buffer_size != ret) {
        goto done;
    }

    int offset = 0;
    data_in->data = tmp_buffer;
    data_in->data_length = tmp_buffer_size;
    for (int i = 0; i < data_in->region_count && i < srec.rec_count; i++) {
        if (offset == srec.rec[i].offset) {
            data_in->clear[i] += (unsigned short) srec.rec[i].bytes_num_changed;
        }
        offset = data_in->clear[i] + data_in->encrypt[i];
    }

    data_out->is_secure = 1;
    MTAVStreamBuffer *buffer = filter->stream_buffer;
    ret = drm_system_secure_memory_malloc(drm, data_out, tmp_buffer_size);
    if (MTAVSF_SUCCESS != ret) {
        goto done;
    }
    ret = drm_system_decrypt_sample_data(drm, data_in, data_out);
    if (MTAVSF_SUCCESS != ret) {
        drm_system_secure_memory_free(data_out->data);
        goto done;
    }
    drm->encrypted = 1;
    buffer->icb = drm_system_secure_memory_free;
    buffer->ibuffer = data_out->data;
    buffer->ibuffer_indx = 0;
    buffer->ibuffer_size = buffer->obuffer_size = tmp_buffer_size;
    *((unsigned int *) info) = buffer->obuffer_size;
done:
    if (srec.rec) {
        MTAVSF_FREE(srec.rec);
    }
    if (tmp_buffer) {
        MTAVSF_FREE(tmp_buffer);
    }
    return ret;
}

#endif

static int scf_drm_init(MTAVStreamFilter *filter, void *init_info)
{
    int ret = 0;
    MTDRM_CONFIG cfg = {"./", NULL};
    MTAV_CSFDrmFilter *drm = filter->priv_data;

    unsigned char insert_file_header = 0;
    if (!drm) {
        drm = MTAVSF_MALLOC(sizeof(MTAV_CSFDrmFilter));
        if (!drm) {
            return MTAVSF_FAILURE;
        }
        memset(drm, 0, sizeof(MTAV_CSFDrmFilter));
        drm->type  = -1;
        drm->index = -1;
        filter->priv_data = drm;
        insert_file_header = 1;
    }

    if (MTAVSF_SUCCESS != set_drm_system_type(
        drm, (MTAV_CSFDrmFilterInitInfo *)init_info)) {
        goto init_failure;
    }

    /* Cannot change drm system when play one clip */
    if (-1 == drm->index) {
#ifdef __LINUX__
        char *path = getenv("MT_PLAYREADY_PATH");
        if(path) {
            cfg.cert_path = (mt_char *) path;
        }
#endif
        ret = drm_system_new(drm, &cfg);
        if (MTAVSF_SUCCESS != ret) {
            goto init_failure;
        }
#if 0
        ret = drm_system_set_key_status_callback(filter,drm);
        if (MTAVSF_SUCCESS != ret) {
            MTAVSF_LOG("[DRM] set key status callback fail\n");
            goto init_failure;
        }
#endif
    }
    ret = drm_system_set_protection_info(drm, (MTAV_CSFDrmFilterInitInfo *) init_info);
    if (MTAVSF_SUCCESS != ret) {
        goto init_failure;
    }

    ret = drm_filter_config(filter, init_info, insert_file_header);
    if (MTAVSF_SUCCESS != ret) {
        MTAVSF_LOG("[DRM] Config filter fail\n");
        goto init_failure;
    }

    return MTAVSF_SUCCESS;
init_failure:
    drm->parse_cb = NULL;
    drm->filter_cb = NULL;
    drm_system_free(drm);
    drm_priv_filter_destroy(&drm->priv_filter);
    MTAVSF_FREE(filter->priv_data);
    filter->priv_data = NULL;
    return MTAVSF_FAILURE;
}

static int scf_drm_get_info(MTAVStreamFilter *filter,
    MTAVStreamPara *para, unsigned char *data, long long pts, unsigned int data_size, void *info)
{
    int ret = MTAVSF_FAILURE;
    MTAV_CSFDrmFilter *drm = filter->priv_data;
    if (!drm) {
        return MTAVSF_FAILURE;
    }

    drm_stream_buffer_free(filter);
    DMTRM_BUFFER_OUT  data_out = {0,};
    if (!para || !para->cfg) {
        ret = drm_parse_clear_data(filter, drm, para, data, data_size, info);
    } else {
        int encrypted = 1;
        DMTRM_BUFFER_IN *data_in = para->cfg;
        if (need_parse_encryption_info(filter, data_in)) {
            ret = chinadrm_change_encryption_info(
                filter, para, data, &data_size, data_in, &encrypted);
            if (MTAVSF_SUCCESS != ret) {
                MTAVSF_LOG("[DRM] Change encryption info fail!\n");
                return ret;
            }
        } else if (1 == data_in->region_count && 0 != data_in->clear[0] && 0 == data_in->encrypt[0]) {
            encrypted = 0;
        }

        if (!encrypted) {
           ret = drm_parse_clear_data(filter, drm, para, data, data_size, info);
           // MTAVSF_LOG("[DRM] Not support this case, please contact me to support!\n");
        } else {
            if (DRM_TYPE_VERIMATRIX  == drm->type) {
                ret = verimatrix_get_encrypted_info(filter, para, data, data_size, &data_out, info);
            } else {
                data_in->data = data;
                data_in->data_length = data_size;
                ret = drm_decrypt_data(filter, drm, data_in, &data_out);
                if (MTAVSF_SUCCESS != ret) {
                    ret = MTAVSF_DRM_DECRYPT_FAIL;
                    return ret;
                }
                ret = drm_parse_entrypted_data(filter, drm,
                        data_out.data, data_size, (unsigned int *) info);
            }
            if (MTAVSF_SUCCESS != ret) {
                return ret;
            }
        }
    }
    MTAVStreamBuffer *buffer = filter->stream_buffer;
    buffer->obuffer_size = *((unsigned int *) info);
    return ret;
}

/* Attention:
 * Not support Secure buffer -> Secure buffer copy
 */
static int scf_drm_filter(MTAVStreamFilter *filter,
    unsigned char *out_buf, unsigned int out_buf_size)
{
    int ret = MTAVSF_SUCCESS;
    MTAV_CSFDrmFilter *drm = filter->priv_data;
    if (!drm) {
        MTAVSF_LOG("[DRM] Filter para meter error!\n");
        return MTAVSF_FAILURE;
    }

    MTAVStreamBuffer *buffer = filter->stream_buffer;
    /* encrypted smp has filter cb, otherwise NULL */
    if (drm->encrypted && drm->filter_cb) {
        ret = drm->filter_cb(filter, drm, out_buf, out_buf_size);
        return ret;
    }
retry:
    if (buffer->obuffer) {
        return drm_filter_obuffer(filter, buffer, out_buf, out_buf_size);
    }

    if (out_buf_size >= buffer->obuffer_size) {
        unsigned int copy_size = MTAVSF_MIN(out_buf_size, buffer->obuffer_size);
        ret = drm_filter_data(filter, out_buf, copy_size);
        drm_stream_buffer_free(filter);
        return ret;
    }

    ret = drm_stream_obufer_new(filter);
    if(MTAVSF_SUCCESS != ret) {
        MTAVSF_LOG("[DRM] Decoder buffer %d, need %d,and no drm memory\n", out_buf_size, buffer->obuffer_size);
        drm_stream_buffer_free(filter);
        return MTAVSF_FAILURE;
    }

    buffer->obuffer_indx = 0;
    if (!drm->encrypted) {
        drm->priv_filter->memcp_cb = memcpy;
    }
    ret = drm_filter_data(filter, buffer->obuffer, buffer->obuffer_size);
    drm->priv_filter->memcp_cb = filter->memcp_cb;
    if (ret < 0) {
        return ret;
    }
    goto retry;
}

static void scf_drm_flush(MTAVStreamFilter *filter)
{
    MTAV_CSFDrmFilter *drm = filter->priv_data;
    if (!drm) {
        return;
    }

    if ((MTAV_CODEC_VID_VP8 == filter->codec_id) ||
        (MTAV_CODEC_VID_VP9 == filter->codec_id)) {
        drm->codec.vpx.insert_file_header = 1;
    }
    if (drm->priv_filter) {
        mtlz_avfilter_stream_flush(drm->priv_filter);
    }
    drm_stream_buffer_free(filter);
}

static void scf_drm_deinit(MTAVStreamFilter *filter)
{
    MTAV_CSFDrmFilter *drm = filter->priv_data;
    if (!drm) {
        return;
    }
    drm_system_free(drm);
    drm_stream_buffer_free(filter);
    drm_priv_filter_destroy(&drm->priv_filter);

    if (filter->priv_data) {
        MTAVSF_FREE(filter->priv_data);
        filter->priv_data = NULL;
    }
}

MTAVStreamCodecFilter mtav_scf_drm = {
    .init     = scf_drm_init,
    .get_info = scf_drm_get_info,
    .filter   = scf_drm_filter,
    .flush    = scf_drm_flush,
    .deinit   = scf_drm_deinit,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
